##
#  @Package     : BeniPkg
#  @FileName    : BuildOvmf.sh
#  @Date        : 20211005
#  @Author      : Jiangwei
#  @Version     : 1.0
#  @Description :
#    This is used to build binary on Ubuntu 18.04.
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

TOOLS=GCC5
ARCH=X64
PYTHON3_ENABLE=TRUE

#
# Add BENI marcos here.
#
echo "DEFINE DEBUG_ON_SERIAL_PORT    = TRUE"          > BeniPkgDefines.dsc.inc
echo "DEFINE NETWORK_ENABLE          = FALSE"         >> BeniPkgDefines.dsc.inc
echo "DEFINE COMPILE_DIR             = DEBUG_$TOOLS"  >> BeniPkgDefines.dsc.inc

PKG_DIR=`pwd`
cd ..

source edksetup.sh
build -p BeniPkg/BeniPkg.dsc -a $ARCH -t $TOOLS
RESULT=$?

echo "===================================================="
echo
if [ $RESULT -eq 0 ]
then
  echo "              Build result: SUCCESS!              "
else
  echo "              Build result: FAILED!!              "
fi
echo
echo "===================================================="
echo

if [ $RESULT -eq 0 ]
then
  echo BIOS was built successfully!
  cp Build/BeniPkg/DEBUG_$TOOLS/FV/OVMF.fd BeniPkg/
  cd $PKG_DIR
  exit 0
else
  echo "Oops!! You got some problems!"
  cd $PKG_DIR
  exit 1
fi