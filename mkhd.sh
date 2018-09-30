#!/bin/bash -ex
if [ ! -f "$1" ]; then
  echo "Pass kernel as first arg" >&2
  exit 1
fi

rm -f hd.img hd.dmg
if [ "$(uname)" == "Darwin" ]; then
    mkdir tmp
    cp "$1" tmp/kernel.bin
    cp "README" tmp/README

    hdiutil create -size 10m -fs exfat -srcfolder tmp hd
    qemu-img convert hd.dmg -O qcow2 hd.img
    rm -r hd.dmg tmp
else
    truncate -s 100M hd.img
    mkfs -t exfat hd.img
    LDEV="$(udisksctl loop-setup -f hd.img | cut -d' ' -f5 | head -c-2)"
    MPAT="$(udisksctl mount -b "$LDEV" | cut -d' ' -f4 | head -c-2)"
    cp "$1" "$MPAT/kernel.bin"
    cp "README" "$MPAT/README"
    sync
    udisksctl unmount -b "$LDEV"
    udisksctl loop-delete -b "$LDEV"
fi