# TARGET := i386-elf-
PROJDIRS := .
BOOTFILES := $(shell find $(PROJDIRS) -type f -name 'boot_*.asm')
SRCFILES := $(shell find $(PROJDIRS) -type f -name '*.c')
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))

CFLAGS := -m32 -std=c11 -Wall -Werror -ffreestanding -fno-pie

all: os.img

main.bin: main.o $(OBJFILES)
	$(TARGET)ld -o $@ -Ttext 0x1000 $^ --oformat binary

%.o: %.c
	$(TARGET)gcc $(CFLAGS) -c $< -o $@

zero.bin:
	dd if=/dev/zero of=$@ bs=512 count=5

%.asm:
	# nothing

os.img: boot_sect.asm $(BOOTFILES) main.bin zero.bin
	nasm $< -f bin -o $@

run: os.img
	qemu-system-i386 -drive format=raw,file=$<

clean:
	rm *.bin *.o
