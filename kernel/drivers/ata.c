#include "ata.h"
#include "pci.h"
#include "pci_registry.h"
#include "ports.h"
#include "../console.h"
#include "../kmalloc.h"

#include <stdio.h>

struct ide_channel_registers {
    uint16_t base;   // I/O base
    uint16_t ctrl;   // Control base
    uint16_t bm_ide; // Bus Master IDE
    uint8_t nIEN;    // nIEN (No Interrupt)
} channels[2];

uint8_t ide_buf[2048] = {0};

struct ide_device ide_devices[4];

void ide_initialize(uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3, uint32_t bar4);

void ata_init() {
    pci_device_t *ide_device = NULL;

    vc_vector *pci_devices = pci_get_devices();
    for (pci_device_t *device = vc_vector_begin(pci_devices);
         device != vc_vector_end(pci_devices);
         device = vc_vector_next(pci_devices, device)) {
        // Find the first IDE device
        if (device->class == PCI_STORAGE_IDE) {
            ide_device = device;
            break;
        }
    }
    if (ide_device == NULL) {
        printf("ATA: No IDE controller found.\n");
        return;
    }

    // Check for IRQ assignment
    uint32_t ide_device_id = PCI_DEVICE_ID(ide_device);
    pci_config_write_byte(ide_device_id, PCI_HEADER_IRQ_LINE, 0xFE);
    if (pci_config_read_byte(ide_device_id, PCI_HEADER_IRQ_LINE) == 0xFE) {
        printf("ATA: Initializing IDE controller at PCI %d:%d.%d\n",
               ide_device->loc.bus, ide_device->loc.device, ide_device->loc.function);
        ide_initialize(ide_device->bar0, ide_device->bar1, ide_device->bar2, ide_device->bar3, ide_device->bar4);
    } else if (ide_device->class == 0x0101 && (ide_device->prog_if == 0x8A || ide_device->prog_if == 0x80)) {
        printf("ATA: Initializing Parallel IDE controller at PCI %d:%d.%d\n",
               ide_device->loc.bus, ide_device->loc.device, ide_device->loc.function);
        ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
    } else {
        fprintf(stderr, "ATA: Failed to initialize IDE device at PCI %d:%d.%d\n",
                ide_device->loc.bus, ide_device->loc.device, ide_device->loc.function);
    }
}

void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, (uint8_t) (0x80 | channels[channel].nIEN));
    if (reg < 0x08)
        port_byte_out((uint16_t) (channels[channel].base + reg - 0x00), data);
    else if (reg < 0x0C)
        port_byte_out((uint16_t) (channels[channel].base + reg - 0x06), data);
    else if (reg < 0x0E)
        port_byte_out((uint16_t) (channels[channel].ctrl + reg - 0x0A), data);
    else if (reg < 0x16)
        port_byte_out((uint16_t) (channels[channel].bm_ide + reg - 0x0E), data);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

uint8_t ide_read(uint8_t channel, uint8_t reg) {
    uint8_t result = ATA_ER_ABRT;
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, (uint8_t) (0x80 | channels[channel].nIEN));
    if (reg < 0x08)
        result = port_byte_in((uint16_t) (channels[channel].base + reg - 0x00));
    else if (reg < 0x0C)
        result = port_byte_in((uint16_t) (channels[channel].base + reg - 0x06));
    else if (reg < 0x0E)
        result = port_byte_in((uint16_t) (channels[channel].ctrl + reg - 0x0A));
    else if (reg < 0x16)
        result = port_byte_in((uint16_t) (channels[channel].bm_ide + reg - 0x0E));
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    return result;
}

__attribute__((no_sanitize("alignment"))) // FIXME
static inline void insl(uint16_t port, uint32_t *buffer, uint32_t count) {
    while (--count) *buffer++ = port_long_in(port);
}

void ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t *buffer, uint32_t count) {
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, (uint8_t) (0x80 | channels[channel].nIEN));
    if (reg < 0x08)
        insl((uint16_t) (channels[channel].base + reg - 0x00), buffer, count);
    else if (reg < 0x0C)
        insl((uint16_t) (channels[channel].base + reg - 0x06), buffer, count);
    else if (reg < 0x0E)
        insl((uint16_t) (channels[channel].ctrl + reg - 0x0A), buffer, count);
    else if (reg < 0x16)
        insl((uint16_t) (channels[channel].bm_ide + reg - 0x0E), buffer, count);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

uint8_t ide_polling(uint8_t channel, uint32_t advanced_check) {
    for (int i = 0; i < 4; i++)
        ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.

    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait for BSY to be zero

    if (advanced_check) {
        uint8_t state = ide_read(channel, ATA_REG_STATUS);
        if (state & ATA_SR_ERR)
            return 2; // Error
        if (state & ATA_SR_DF)
            return 1; // Device Fault
        if ((state & ATA_SR_DRQ) == 0)
            return 3; // DRQ should be set
    }
    return 0;
}

__attribute__((no_sanitize("alignment"))) // FIXME
void ide_initialize(uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3, uint32_t bar4) {
    uint8_t i, j, k, count = 0;

    // 1- Detect I/O Ports which interface IDE Controller:
    channels[ATA_PRIMARY].base = (uint16_t) ((bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0));
    channels[ATA_PRIMARY].ctrl = (uint16_t) ((bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1));
    channels[ATA_PRIMARY].bm_ide = (uint16_t) ((bar4 & 0xFFFFFFFC) + 0); // Bus Master IDE
    channels[ATA_SECONDARY].base = (uint16_t) ((bar2 & 0xFFFFFFFC) + 0x170 * (!bar2));
    channels[ATA_SECONDARY].ctrl = (uint16_t) ((bar3 & 0xFFFFFFFC) + 0x376 * (!bar3));
    channels[ATA_SECONDARY].bm_ide = (uint16_t) ((bar4 & 0xFFFFFFFC) + 8); // Bus Master IDE

    // Disable IRQs
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // Detect ATA-ATAPI devices
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            uint8_t err = 0, type = IDE_ATA, status;
            ide_devices[count].reserved = 0;

            ide_write(i, ATA_REG_HDDEVSEL, (uint8_t) (0xA0 | (j << 4))); // Select Drive.
//            sleep(1); // Wait 1ms for drive select to work.

            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY); // Send ATA Identify
//            sleep(1);

            // (III) Polling:
            if (ide_read(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.

            while (1) {
                status = ide_read(i, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR)) {
                    err = 1;
                    break;
                } // Device is not ATA

                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // OK
            }

            // (IV) Probe for ATAPI Devices:
            if (err != 0) {
                uint8_t cl = ide_read(i, ATA_REG_LBA1);
                uint8_t ch = ide_read(i, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB)
                    type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                    type = IDE_ATAPI;
                else
                    continue; // Unknown Type (may not be a device).

                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
//                sleep(1);
            }

            // Read identification
            ide_read_buffer(i, ATA_REG_DATA, (uint32_t *) ide_buf, 128);

            // Read device parameters
            ide_devices[count].reserved = 1;
            ide_devices[count].type = type;
            ide_devices[count].channel = i;
            ide_devices[count].drive = j;
            ide_devices[count].signature = *((uint16_t *) (ide_buf + ATA_IDENT_DEVICETYPE));
            ide_devices[count].capabilities = *((uint16_t *) (ide_buf + ATA_IDENT_CAPABILITIES));
            ide_devices[count].command_sets = *((uint32_t *) (ide_buf + ATA_IDENT_COMMANDSETS));

            // Read size
            if (ide_devices[count].command_sets & (1 << 26)) {
                // Device uses 48-Bit addressing
                ide_devices[count].size = *((uint32_t *) (ide_buf + ATA_IDENT_MAX_LBA_EXT));
            } else {
                // Device uses CHS or 28-bit addressing
                ide_devices[count].size = *((uint32_t *) (ide_buf + ATA_IDENT_MAX_LBA));
            }

            // Device model name
            for (k = 0; k < 40; k += 2) {
                ide_devices[count].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                ide_devices[count].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
            }
            ide_devices[count].model[40] = 0;

            count++;
        }
    }
}

uint8_t ide_ata_access(uint8_t direction, uint8_t drive, uint32_t lba,
                       uint8_t numsects, uint16_t selector, uint32_t edi) {
    uint8_t lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, cmd;
    uint8_t lba_io[6];
    uint8_t channel = ide_devices[drive].channel; // Read the Channel.
    uint8_t slavebit = ide_devices[drive].drive; // Read the Drive [Master/Slave]
    uint16_t bus = channels[channel].base; // Bus Base, like 0x1F0 which is also data port.
    uint32_t words = 256; // Almost every ATA drive has a sector-size of 512-byte.
    uint16_t cyl, i;
    uint8_t head, sect, err;

    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = 0x02);

    // (I) Select one from LBA28, LBA48 or CHS;
    if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you are
        // giving a wrong LBA.
        // LBA48:
        lba_mode = 2;
        lba_io[0] = (lba & 0x000000FF) >> 0;
        lba_io[1] = (lba & 0x0000FF00) >> 8;
        lba_io[2] = (lba & 0x00FF0000) >> 16;
        lba_io[3] = (lba & 0xFF000000) >> 24;
        lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
        lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
        head = 0; // Lower 4-bits of HDDEVSEL are not used here.
    } else if (ide_devices[drive].capabilities & 0x200) { // Drive supports LBA?
        // LBA28:
        lba_mode = 1;
        lba_io[0] = (lba & 0x00000FF) >> 0;
        lba_io[1] = (lba & 0x000FF00) >> 8;
        lba_io[2] = (lba & 0x0FF0000) >> 16;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (lba & 0xF000000) >> 24;
    } else {
        // CHS:
        lba_mode = 0;
        sect = (lba % 63) + 1;
        cyl = (lba + 1 - sect) / (16 * 63);
        lba_io[0] = sect;
        lba_io[1] = (cyl >> 0) & 0xFF;
        lba_io[2] = (cyl >> 8) & 0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (lba + 1 - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
    }

    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait if busy.

    // Select drive from controller
    if (lba_mode == 0) {
        ide_write(channel, ATA_REG_HDDEVSEL, (uint8_t) 0xA0 | (slavebit << 4) | head); // Drive & CHS
    } else {
        ide_write(channel, ATA_REG_HDDEVSEL, (uint8_t) 0xE0 | (slavebit << 4) | head); // Drive & LBA
    }

    // Write parameters
    if (lba_mode == 2) {
        ide_write(channel, ATA_REG_SECCOUNT1, 0);
        ide_write(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write(channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write(channel, ATA_REG_SECCOUNT0, numsects);
    ide_write(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]);

    if (lba_mode == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
    else if (lba_mode == 1 && direction == 0) cmd = ATA_CMD_READ_PIO;
    else if (lba_mode == 2 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;
    else if (lba_mode == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    else if (lba_mode == 1 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    else if (lba_mode == 2 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
    else panic("ide_ata_access: Bad LBA mode.\n");
    ide_write(channel, ATA_REG_COMMAND, cmd);

    if (direction == 0) {
        // PIO read
        for (i = 0; i < numsects; i++) {
            if ((err = ide_polling(channel, 1)))
                return err;
            __asm__ volatile("rep insw" : : "c"(words), "d"(bus), "D"(edi));
            edi += (words * 2);
        }
    } else {
        // PIO write
        for (i = 0; i < numsects; i++) {
            ide_polling(channel, 0);
            __asm__ volatile("rep outsw" : : "c"(words), "d"(bus), "S"(edi));
            edi += (words * 2);
        }
        cmd = (uint8_t) (int[]) {ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode];
        ide_write(channel, ATA_REG_COMMAND, cmd);
        ide_polling(channel, 0);
    }

    return 0;
}
