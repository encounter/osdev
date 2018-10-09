#!/bin/bash -ex
if [ ! -d "$1" ]; then
  echo "Pass build directory as first arg" >&2
  exit 1
fi

rm -f hd.img hd.dmg
if [ "$(uname)" == "Darwin" ]; then
    mkdir -p tmp/bin
    cp "$1/kernel.bin" tmp/kernel.bin
    cp "README" tmp/README
    if [ -d "$1/bin" ]; then
        gfind "$1/bin" -type f -executable -exec cp {} tmp/bin \;
    fi
    cp -R assets "tmp/"

    hdiutil create -size 10m -fs exfat -srcfolder tmp hd
    qemu-img convert hd.dmg -O qcow2 hd.img
    rm -r  tmp # hd.dmg
else
    truncate -s 100M hd.img
    mkfs -t exfat hd.img
    LDEV="$(udisksctl loop-setup -f hd.img | cut -d' ' -f5 | head -c-2)"
    MPAT="$(udisksctl mount -b "$LDEV" | cut -d' ' -f4 | head -c-2)"
    mkdir "$MPAT/bin"
    cp "$1/kernel.bin" "$MPAT/kernel.bin"
    cp "README" "$MPAT/README"
    if [ -d "$1/bin" ]; then
        find "$1/bin" -type f -executable -exec cp {} "$MPAT/bin" \;
    fi
    cp -R assets "$MPAT/"
    sync
    udisksctl unmount -b "$LDEV"
    udisksctl loop-delete -b "$LDEV"
fi