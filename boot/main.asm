[bits 16]
[org 0x7C00]
STACK_BASE equ 0x9000
KERNEL_OFFSET equ 0x1000

start:
  mov bp, STACK_BASE ; position our stack pointer
  mov sp, bp

  mov ax, 0x0003 ; "Set Video Mode" (mode 03h)
  int 10h

  mov bx, STARTING_UP
  call print_str

  mov bx, KERNEL_OFFSET
  mov dh, 33
  ; mov dl, [BOOT_DRIVE]
  call disk_load

  mov ax, 0x0003 ; "Set Video Mode" (mode 03h)
  int 10h
  
  call switch_to_pm
  jmp $ ; never called

; Prints null-terminated string referenced by `bx`
print_str:
  mov ah, 0Eh    ; "Write Character in TTY Mode"
.inner:
  mov al, [bx]
  test al, al
  je .end
  int 10h
  add bx, 1
  jmp .inner
.end:
  ret

; Prints hex string of 16-bit value stored in `bx`
print_hex:
  mov ah, 0Eh    ; "Write Character in TTY Mode"
  mov al, '0'
  int 10h
  mov al, 'x'
  int 10h

  mov al, bh
  shr al, 4
  call print_hex_char

  mov al, bh
  and al, 0Fh
  call print_hex_char

  mov al, bl
  shr al, 4
  call print_hex_char

  mov al, bl
  and al, 0Fh
  call print_hex_char
  ret

; Prints hex representation of the value stored in `al`
print_hex_char:
  mov ah, 0Eh    ; "Write Character in TTY Mode"
  cmp al, 9
  jle .inner
  add al, 7
.inner:
  add al, 30h
  int 10h
  ret

%include "disk.asm"
%include "pm.asm" ; start of 32-bit PM code
%include "gdt.asm"
%include "print.asm"

begin_pm:
  call KERNEL_OFFSET
  jmp $          ; spin forever

; datas
ENDL: db 0Dh, 0Ah, 0
STARTING_UP: db 'Reading boot sector...', 0Dh, 0Ah, 0
PROT_MODE db "Loaded 32-bit protected mode", 0

; Fill rest of the bin with nops
times 510-($-$$) nop

; Bootloader magic!
dw 0xAA55

; my awesome kernel
; incbin "kernel/main.bin"
; incbin "boot/zero.bin" ; some padding
