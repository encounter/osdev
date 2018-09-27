#!/bin/bash -ex
if [ ! -f "$1" ]; then
  echo "Pass kernel as first arg" >&2
  exit 1
fi

mkdir -p iso/boot/grub
cp "$1" iso/boot/kernel.bin
cat > iso/boot/grub/grub.cfg <<EOF
set timeout=0
set default=0

menuentry "OS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

grub-mkrescue -o os.iso iso
rm -r iso