#pragma once

#include <common.h>
#include <vector.h>

#define PCI_ID(bus, dev, func) ((uint32_t) (bus & 0xFF) << 16 | (dev & 0x1F) << 11 | (func & 0x07) << 8)
#define PCI_DEVICE_ID(dev) (PCI_ID(dev->loc.bus, dev->loc.device, dev->loc.function))
#define PCI_ID_BUS(id)  ((uint8_t) ((id) >> 16 & 0xFF))
#define PCI_ID_DEV(id)  ((uint8_t) ((id) >> 11 & 0x1F))
#define PCI_ID_FUNC(id) ((uint8_t) ((id) >> 8  & 0x07))

#define PCI_HEADER_VENDOR_ID 0x00
#define PCI_HEADER_DEVICE_ID 0x02
#define PCI_HEADER_REVISION  0x08
#define PCI_HEADER_CLASS     0x0A
#define PCI_HEADER_TYPE_ID   0x0E
#define PCI_HEADER_BUS_NUM   0x18
#define PCI_HEADER_SEC_BUS   0x19
#define PCI_HEADER_IRQ_LINE  0x3C // 8 bits

#define PCI_HEADER_BAR0_ADDR 0x10
#define PCI_HEADER_BAR1_ADDR 0x14
#define PCI_HEADER_BAR2_ADDR 0x18
#define PCI_HEADER_BAR3_ADDR 0x1C
#define PCI_HEADER_BAR4_ADDR 0x20
#define PCI_HEADER_BAR5_ADDR 0x24

struct pci_device_loc {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};
typedef struct pci_device_loc pci_device_loc_t;

struct pci_device {
    pci_device_loc_t loc;
    uint16_t class;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint8_t prog_if;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
};
typedef struct pci_device pci_device_t;

void pci_init();

vc_vector *pci_get_devices();

uint32_t pci_config_read_long(uint32_t id, uint8_t offset);

uint16_t pci_config_read_word(uint32_t id, uint8_t offset);

uint8_t pci_config_read_byte(uint32_t id, uint8_t offset);

void pci_config_write_byte(uint32_t id, uint8_t offset, uint8_t val);