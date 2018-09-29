; Declare constants for the multiboot header.
FMBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
FMEMINFO  equ  1 << 1            ; provide memory map
FVIDMODE  equ  1 << 2            ; try to set graphics mode
FLAGS     equ  FMBALIGN | FMEMINFO | FVIDMODE
MAGIC     equ  0x1BADB002
CHECKSUM  equ -(MAGIC + FLAGS)

; This is the virtual base address of kernel space. It must be used to convert virtual
; addresses into physical addresses until paging is enabled.
KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER  equ (KERNEL_VIRTUAL_BASE >> 22) ; 768 -- Page directory index of kernel's 4MB PTE.

section .data
align 0x1000
boot_page_directory:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.

    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0         ; Pages before kernel space.

    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0  ; Pages after the kernel image.

; Declare a multiboot header that marks the program as a kernel. These are magic
; values that are documented in the multiboot standard. The bootloader will
; search for this signature in the first 8 KiB of the kernel file, aligned at a
; 32-bit boundary. The signature is in its own section so the header can be
; forced to be within the first 8 KiB of the kernel file.
section .multiboot
align 4
    ; header
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

    ; address tag
    dd 0   ; header_addr
    dd 0   ; load_addr
    dd 0   ; load_end_addr
    dd 0   ; bss_end_addr
    dd 0   ; entry_addr

    ; graphics tag
    dd 1   ; mode_type
    dd 800 ; width
    dd 600 ; height
    dd 32  ; depth
 
; Create bootstrap stack 
section .bss
align 16
stack_bottom:
resb 0x4000 ; 16 KiB
stack_top:

; The linker script specifies _start as the entry point to the kernel and the
; bootloader will jump to this position once the kernel has been loaded. It
; doesn't make sense to return from this function as the bootloader is gone.
; Declare _start as a function symbol with the given symbol size.
section .text
global _start:function (_start.end - _start)
_start:
    mov ecx, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx ; Load Page Directory Base Register.
 
    mov ecx, cr4
    or ecx, (1 << 4) ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx
 
    mov ecx, cr0
    or ecx, (1 << 31) ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx

    lea ecx, [.start_higher_half] ; far jump
    jmp ecx

.start_higher_half:
    ; Unmap the identity-mapped first 4MB of physical address space. 
	; It should not be needed anymore.
    mov dword [boot_page_directory], 0
    invlpg [0]
 
	; Set up stack
    mov esp, stack_top

    ; Save multiboot information
    push ebx ; multiboot_info
    push eax ; multiboot_magic

	; Enable SSE
    mov eax, cr0
    and ax, 0xFFFB  ; clear coprocessor emulation CR0.EM
    or ax, (1 << 1) ; set coprocessor monitoring  CR0.MP
    mov cr0, eax
    mov eax, cr4
    or ax, (3 << 9) ; set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
    mov cr4, eax

    ; Initialize GDT & IDT
    extern init_descriptor_tables
    call init_descriptor_tables
 
    ; Enter the high-level kernel. The ABI requires the stack is 16-byte
    ; aligned at the time of the call instruction (which afterwards pushes
    ; the return pointer of size 4 bytes). The stack was originally 16-byte
    ; aligned above and we've since pushed a multiple of 16 bytes to the
    ; stack since (pushed 0 bytes so far) and the alignment is thus
    ; preserved and the call is well defined.
    extern kernel_main
    call kernel_main
 
    cli
.hang:
    hlt
    jmp .hang
.end:
