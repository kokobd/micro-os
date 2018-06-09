#!/usr/bin/env bash

MODULES=()

set -o errexit

if [ -z "$1" ]; then
    echo "Target directory must not be empty"
    exit 1
fi

tar x -C scripts/bin -f scripts/bin/micro-os.tar.gz

BUILD_DIR="$1"

rm -f "${BUILD_DIR}/micro-os.img"
mv scripts/bin/micro-os.img "${BUILD_DIR}"
cp scripts/grub.cfg "${BUILD_DIR}"

cd "${BUILD_DIR}"

mkdir -p micro-os
mount -o loop,offset=1048576 micro-os.img micro-os
mkdir -p micro-os/boot/micro-os
cp src/kernel/kernel micro-os/boot/micro-os/
for item in ${MODULES[*]}; do
    cp $item micro-os/boot/micro-os/
done
cp -f grub.cfg micro-os/boot/grub/

sync micro-os
umount micro-os
rmdir micro-os

user=$(stat -c "%U" .)
chown ${user}: micro-os.img
