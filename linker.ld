ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    /* The kernel will live at 3GB + 1MB in the virtual
       address space, which will be mapped to 1MB in the
       physical address space. */
    . = 0xC0100000;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text : AT(ADDR(.text) - 0xC0000000)
	{
		*(.multiboot)
		*(.text)
	}

	/* Read-only data. */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
       _sbss = .;
		*(COMMON)
		*(.bss)
       _ebss = .;
	}
}