build:
	gcc -m32 -Wall -Werror -ffreestanding -fno-pie -c main.c -o main.o
	ld -m i386linux -o main.bin -Ttext 0x9000 --oformat binary main.o
	nasm -f bin boot_sect.asm -o boot_sect.bin

run:
	qemu-system-x86_64 -drive format=raw,file=boot_sect.bin
