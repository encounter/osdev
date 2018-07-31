[bits 16]
[org 0x7C00]
start:
  mov bp, 0x8000 ; put our stack at 0x8000
  mov sp, bp

  mov ax, 0x0003 ; "Set Video Mode" (mode 03h)
  int 10h

  ; mov [BOOT_DRIVE], dl ; store our boot drive for later

  mov bx, STARTING_UP
  call print_str

  mov bx, 0x9000
  mov dh, 2
  ; mov dl, [BOOT_DRIVE]
  call disk_load

  mov bx, DONE
  call print_str

  mov bx, [es:0x9000]
  call print_hex
  mov bx, ENDL
  call print_str

  mov bx, [es:0x9000 + 512]
  call print_hex
  mov bx, ENDL
  call print_str

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

%include "boot_disk.asm"
%include "boot_pm.asm" ; start of 32-bit PM code
%include "boot_gdt.asm"
%include "boot_print.asm"

begin_pm:
  ; mov ebx, PROT_MODE
  ; call print_string_pm ; Note that this will be written at the top left corner

  call 0x9000
  jmp $          ; spin forever

; datas
ENDL: db 0Dh, 0Ah, 0
STARTING_UP: db 'Reading boot sector...', 0Dh, 0Ah, 0
DONE: db 'Done!', 0Dh, 0Ah, 0
PROT_MODE db "Loaded 32-bit protected mode", 0

; globals
BOOT_DRIVE: db 0

; Fill rest of the bin with nops
times 510-($-$$) nop

; Bootloader magic!
dw 0xAA55

; my awesome kernel
incbin "main.bin"