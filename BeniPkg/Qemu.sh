##
#  @Package     : BeniPkg
#  @FileName    : Qemu.sh
#  @Date        : 20211005
#  @Author      : Jiangwei
#  @Version     : 1.0
#  @Description :
#    This is used to start QEMU using OVMF.fd as BIOS.
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

OVMF_FILE=`pwd`/OVMF.fd
LOG_FILE=`pwd`/log.txt
DISK_IMAGE=`pwd`/disk.img

if [ -f $LOG_FILE ]
then
  rm $LOG_FILE
fi

./DiskPrep.sh

qemu-system-x86_64 -usb -bios $OVMF_FILE -serial stdio >> $LOG_FILE -hda $DISK_IMAGE -m 500M
#qemu-system-x86_64 -bios OVMF.fd -serial stdio -net nic -net tap,ifname=OpenVPN >> log.txt
