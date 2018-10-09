#include <vector.h>
#include "pci.h"
#include "ports.h"
#include "../console.h"

#define PCI_VENDOR_NONE 0xFFFF

static vc_vector *pci_devices;

uint32_t pci_config_read_long(uint32_t id, uint8_t offset) {
    port_long_out(0xCF8, (uint32_t) 0x80000000 | id | (offset & 0xFC));
    return port_long_in(0xCFC);
}

uint16_t pci_config_read_word(uint32_t id, uint8_t offset) {
    return (uint16_t) (pci_config_read_long(id, offset) >> (offset & 2) * 8 & 0xFFFF);
}

uint8_t pci_config_read_byte(uint32_t id, uint8_t offset) {
    return (uint8_t) (pci_config_read_word(id, offset) & 0xFF);
}

void pci_config_write_byte(uint32_t id, uint8_t offset, uint8_t val) {
    port_long_out(0xCF8, (uint32_t) 0x80000000 | id | (offset & 0xFC));
    port_byte_out(0xCFC, val);
}

uint16_t pci_read_vendor(uint32_t id) {
    return pci_config_read_word(id, PCI_HEADER_VENDOR_ID);
}

uint8_t pci_read_header_type(uint32_t id) {
    return (uint8_t) (pci_config_read_word(id, PCI_HEADER_TYPE_ID) & 0xFF);
}

// lower half = subclass, upper half = base class
uint16_t pci_read_class(uint32_t id) {
    return pci_config_read_word(id, PCI_HEADER_CLASS);
}

// lower half = primary, upper half = secondary
uint16_t pci_read_bus_number(uint32_t id) {
    return pci_config_read_word(id, PCI_HEADER_BUS_NUM);
}

// lower half = revision, upper half = prog IF
uint16_t pci_read_revision(uint32_t id) {
    return pci_config_read_word(id, PCI_HEADER_REVISION);
}

void pci_check_bus(uint8_t bus);

bool pci_check_function(uint32_t id) {
    uint32_t val = pci_config_read_long(id, PCI_HEADER_VENDOR_ID);
    uint16_t vendor_id = (uint16_t) (val & 0xFFFF);
    uint16_t device_id = (uint16_t) (val >> 16 & 0xFFFF);
    if (vendor_id == PCI_VENDOR_NONE) return false;

    uint16_t class = pci_read_class(id);
    uint16_t revision = pci_read_revision(id);
    uint8_t header_type = pci_read_header_type(id);
    vc_vector_push_back(pci_devices, &(pci_device_t) {
         .loc = {
             .bus = PCI_ID_BUS(id),
             .device = PCI_ID_DEV(id),
             .function = PCI_ID_FUNC(id)
         },
         .class = class,
         .vendor_id = vendor_id,
         .device_id = device_id,
         .revision_id = (uint8_t) (revision & 0xFF),
         .prog_if = (uint8_t) (revision >> 8 & 0xFF),
         .bar0 = pci_config_read_long(id, PCI_HEADER_BAR0_ADDR),
         .bar1 = pci_config_read_long(id, PCI_HEADER_BAR1_ADDR),
         .bar2 = header_type == 0x00 ? pci_config_read_long(id, PCI_HEADER_BAR2_ADDR) : 0,
         .bar3 = header_type == 0x00 ? pci_config_read_long(id, PCI_HEADER_BAR3_ADDR) : 0,
         .bar4 = header_type == 0x00 ? pci_config_read_long(id, PCI_HEADER_BAR4_ADDR) : 0,
         .bar5 = header_type == 0x00 ? pci_config_read_long(id, PCI_HEADER_BAR5_ADDR) : 0,
     });

    // PCI-to-PCI bridge
    if (class == 0x0604) {
        // FIXME breaks VMWare
        // pci_check_bus(pci_config_read_byte(id, PCI_HEADER_SEC_BUS));
    }
    return true;
}

void pci_check_device(uint8_t bus, uint8_t device) {
    uint32_t pci_id = PCI_ID(bus, device, 0);
    if (!pci_check_function(pci_id)) return;

    uint8_t header_type = pci_read_header_type(pci_id);
    if ((header_type & 0x80) != 0) {
        for (uint8_t function = 1; function < 8; function++) {
            pci_check_function(PCI_ID(bus, device, function));
        }
    }
}

void pci_check_bus(uint8_t bus) {
    uint8_t device;
    for (device = 0; device < 32; device++) {
        pci_check_device(bus, device);
    }
}

void pci_check_all_buses() {
    uint8_t header_type = pci_read_header_type(PCI_ID(0, 0, 0));
    if ((header_type & 0x80) == 0) {
        // Single PCI host controller
        pci_check_bus(0);
    } else {
        // Multiple PCI host controllers
        for (uint8_t function = 0; function < 8; function++) {
            if (pci_read_vendor(PCI_ID(0, 0, function)) != PCI_VENDOR_NONE) break;
            pci_check_bus(function);
        }
    }
}

void pci_init() {
    pci_devices = vc_vector_create(8, sizeof(pci_device_t), NULL);
    pci_check_all_buses();
}

vc_vector *pci_get_devices() {
    return pci_devices;
}