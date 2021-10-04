##
#  @Package     : BeniPkg
#  @FileName    : DiskPrep.sh
#  @Date        : 20211005
#  @Author      : Jiangwei
#  @Version     : 1.0
#  @Description :
#    This is used to prepare a disk image for UEFI and kernel usage.
#
#  @History:
#    20211005: Initialize.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution. The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
##

DISK_IMAGE=`pwd`/Disk.img
echo $DISK_IMAGE

if [ ! -f $DISK_IMAGE ]
then
 	dd if=/dev/zero of=$DISK_IMAGE bs=1M count=32
	mkfs.fat $DISK_IMAGE
fi
