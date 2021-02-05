#!/bin/bash

SIZE_GB="4"
bootfs="images/linux/bootfs.tar.gz"
rootfs="images/linux/rootfs.tar.gz"
mac="00:0a:35:00:22:01"
ip="192.168.121.1/24"
gateway="192.168.121.52/24"

#create and partition  disk image
dd if=/dev/zero of=../rootfs.img bs=1024M seek=$SIZE_GB count=0
parted --script -- ../rootfs.img mklabel msdos mkpart primary 1MiB 100MiB mkpart primary 100MiB -1MiB
sfdisk --change-id ../rootfs.img 1 c
sfdisk ../rootfs.img -A 1

#setup a loop device (likely needs root)
LO=$( losetup -f )
losetup -P $LO ../rootfs.img

#p1 is FAT boot partition
mkfs.vfat -F 32 -n BOOT ${LO}p1
mountpoint=`mktemp -d`
mount ${LO}p1 "$mountpoint"
tar -C "$mountpoint" -xf "$bootfs" --no-same-owner
umount "$mountpoint"
rmdir "$mountpoint"

#p2 is ext4 root partition
mkfs.ext4 -L root ${LO}p2
mountpoint=`mktemp -d`
mount ${LO}p2 "$mountpoint"
tar -C "$mountpoint" -xf "$rootfs"
echo "${mac}" >> "${mountpoint}/etc/wib/mac"
echo "${ip}" >> "${mountpoint}/etc/wib/ip"
echo "${gateway}" >> "${mountpoint}/etc/wib/gateway"
umount "$mountpoint"
rmdir "$mountpoint"

#tear down loop device
losetup -d $LO
