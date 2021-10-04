##
#  @Package     : BeniPkg
#  @FileName    : Clean.sh
#  @Date        : 20211005
#  @Author      : Jiangwei
#  @Version     : 1.0
#  @Description :
#    This is used to clean middle files for UEFI building.
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

PKG_DIR=`pwd`

if [ -f log.txt ]
then
  rm log.txt
fi

if [ -f OVMF.fd ]
then
  rm OVMF.fd
fi

if [ -f disk.img ]
then
  rm disk.img
fi

cd ..

if [ -d Build ]
then
  rm Build -rf
fi

if [ -d Conf ]
then
  cd Conf
  rm .AutoGenIdFile.txt
  rm BuildEnv.sh
  rm build_rule.txt
  rm .cache -rf
  rm target.txt
  rm tools_def.txt
fi

cd $PKG_DIR