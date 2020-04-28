#!/bin/bash

guestfish --verbose -N ../rootfs.img=disk:2G <<_EOF_
part-init /dev/sda msdos 
part-add /dev/sda p 64 204863 
part-add /dev/sda p 204864 -1
mkfs vfat /dev/sda1
mount /dev/sda1 / 
upload images/linux/image.ub /image.ub 
upload images/linux/BOOT.BIN /BOOT.BIN 
upload images/linux/rootfs.ext4 /dev/sda2
e2fsck /dev/sda2 correct:true
resize2fs /dev/sda2
_EOF_
echo 'type=06' | sfdisk ../rootfs.img 0 #partition type is set wrong for some reason
