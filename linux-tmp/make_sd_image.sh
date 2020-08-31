#!/bin/bash

#create and partition 2GB disk image
dd if=/dev/zero of=../rootfs.img bs=1024M seek=2 count=0
parted --script -- ../rootfs.img mklabel msdos mkpart primary 1MiB 100MiB mkpart primary 100MiB -1MiB

#setup a loop device (likely needs root)
LO=$( losetup -f )
losetup -P $LO ../rootfs.img

#p1 is FAT boot partition
mkfs.vfat -F 32 ${LO}p1
mcopy -i ${LO}p1 images/linux/image.ub ::image.ub
mcopy -i ${LO}p1 images/linux/BOOT.BIN ::BOOT.BIN
mcopy -i ${LO}p1 images/linux/boot.scr ::boot.scr

#p2 is ext4 root partition
dd if=images/linux/rootfs.ext4 of=${LO}p2 bs=256M status=progress
e2fsck -y ${LO}p2
resize2fs ${LO}p2

#tear down loop device
losetup -d $LO

#explicitly set partition type for boot partition
echo 'type=06 bootable' | sfdisk --quiet ../rootfs.img 0 
