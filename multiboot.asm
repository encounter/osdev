FMBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
FMEMINFO  equ  1 << 1            ; provide memory map
FVIDMODE  equ  1 << 2            ; try to set graphics mode
FLAGS     equ  FMBALIGN | FMEMINFO | FVIDMODE
MAGIC     equ  0x1BADB002
CHECKSUM  equ -(MAGIC + FLAGS)

KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER  equ (KERNEL_VIRTUAL_BASE >> 22) ; 768

section .data
align 0x1000
boot_page_directory:
    ; 4MiB lower-half kernel page
    dd 0x83
    times (KERNEL_PAGE_NUMBER - 1) dd 0

    ; 4MiB higher-half kernel page
    dd 0x83
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0

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
    dd 0   ; mode_type
    dd 800 ; width
    dd 600 ; height
    dd 32  ; depth
 
; Create bootstrap stack 
section .bss
align 16
stack_bottom:
resb 0x4000 ; 16 KiB
stack_top:

section .text
global _start:function (_start.end - _start)
_start:
    mov ecx, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx ; Load boot_page_directory into CR3
 
    mov ecx, cr4
    or ecx, (1 << 4) ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx
 
    mov ecx, cr0
    or ecx, (1 << 31) ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx

    lea ecx, [.start_higher_half] ; Far jump
    jmp ecx

.start_higher_half:
    ; Unmap lower-half kernel page
    mov dword [boot_page_directory], 0
    invlpg [0]
 
	; Set up stack
    mov esp, stack_top

    ; Save multiboot information
    push ebx ; multiboot_info
    push eax ; multiboot_magic

	; Enable SSE
    mov eax, cr0
    and ax, ~(1 << 2) ; clr CR0.EM
    or ax, (1 << 1)   ; set CR0.MP
    mov cr0, eax
    mov eax, cr4
    or ax, (3 << 9)   ; set CR4.OSFXSR | CR4.OSXMMEXCPT
    mov cr4, eax

    ; Initialize GDT & IDT
    extern init_descriptor_tables
    call init_descriptor_tables

    ; Enter kernel
    extern kernel_main
    call kernel_main
 
    cli
.hang:
    hlt
    jmp .hang
.end:
