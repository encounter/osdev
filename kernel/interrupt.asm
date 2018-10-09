[bits 32]
extern isr_handler
extern irq_handler

%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

%macro IRQ 2
  global irq%1
  irq%1:
    cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

ISR_NOERRCODE  0
ISR_NOERRCODE  1
ISR_NOERRCODE  2
ISR_NOERRCODE  3
ISR_NOERRCODE  4
ISR_NOERRCODE  5
ISR_NOERRCODE  6
ISR_NOERRCODE  7
ISR_ERRCODE  8
ISR_NOERRCODE  9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

IRQ  0, 32
IRQ  1, 33
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

isr_common_stub:
   pusha ; push edi, esi, ebp, esp, ebx, edx, ecx, eax

   ; push mxcsr
   sub esp, 4
   stmxcsr [esp]

   ; push cr2
   mov eax, cr2
   push eax

   call isr_handler

   ; discard cr2
   add esp, 4

   ; restore mxcsr
   ldmxcsr [esp]
   add esp, 4

   popa       ; pop edi, esi, ebp, esp, ebx, edx, ecx, eax
   add esp, 8 ; discard error code and ISR number
   sti        ; re-enable interrupts
   iret       ; restore cs, eip, eflags, ss, esp

irq_common_stub:
   pusha ; push edi, esi, ebp, esp, ebx, edx, ecx, eax

   ; push mxcsr
   sub     esp,    4
   stmxcsr [esp]

   ; push cr2
   mov eax, cr2
   push eax

   call irq_handler

   ; discard cr2
   add esp, 4

   ; restore mxcsr
   ldmxcsr [esp]
   add     esp,    4

   popa           ; pop edi, esi, ebp, esp, ebx, edx, ecx, eax
   add esp, 8     ; discard error code and IRQ number
   sti            ; re-enable interrupts
   iret           ; restore cs, eip, eflags, ss, esp