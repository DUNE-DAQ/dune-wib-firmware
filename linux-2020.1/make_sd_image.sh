#!/bin/bash

SIZE_GB="4"

#create and partition  disk image
dd if=/dev/zero of=../rootfs.img bs=1024M seek=$SIZE_GB count=0
parted --script -- ../rootfs.img mklabel msdos mkpart primary 4MiB 100MiB mkpart primary 100MiB -1MiB

#setup a loop device (likely needs root)
LO=$( losetup -f )
losetup -P $LO ../rootfs.img

#p1 is FAT boot partition

#mkfs.vfat -F 32 ${LO}p1
#mcopy -i ${LO}p1 images/linux/image.ub ::image.ub
#mcopy -i ${LO}p1 images/linux/BOOT.BIN ::BOOT.BIN
#mcopy -i ${LO}p1 images/linux/boot.scr ::boot.scr

mkfs.vfat -F 32 ${LO}p1
mountpoint=`mktemp -d`
mount ${LO}p1 "$mountpoint"
tar -C "$mountpoint" -xf images/linux/bootfs.tar.gz --no-same-owner
umount "$mountpoint"
rmdir "$mountpoint"

#p2 is ext4 root partition

#dd if=images/linux/rootfs.ext4 of=${LO}p2 bs=256M status=progress
#e2fsck -y ${LO}p2
#resize2fs ${LO}p2

mkfs.ext4 -F ${LO}p2
mountpoint=`mktemp -d`
mount ${LO}p2 "$mountpoint"
tar -C "$mountpoint" -xf images/linux/rootfs.tar.gz
umount "$mountpoint"
rmdir "$mountpoint"

#tear down loop device
losetup -d $LO

#explicitly set partition type for boot partition
echo 'type=06 bootable' | sfdisk --quiet ../rootfs.img 0 
