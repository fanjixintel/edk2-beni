::::
:: @Package     : BeniPkg
:: @FileName    : BuildPayload.cmd
:: @Date        : 20211205
:: @Author      : Jiangwei
:: @Version     : 1.0
:: @Description :
::   This is used to build payload for SlimBootloader.
::   VS2019 is needed in order to build successfully.
::
:: @History:
::   20211205: Initialize.
::
:: This program and the accompanying materials
:: are licensed and made available under the terms and conditions of the BSD License
:: which accompanies this distribution. The full text of the license may be found at
:: http://opensource.org/licenses/bsd-license.php
::
:: THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
:: WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
::::

@echo off

set PKG_DIR=%CD%
cd ..

call edksetup.bat
call build -a IA32 -a X64 -p UefiPayloadPkg\UefiPayloadPkg.dsc -b DEBUG -t VS2019 -D BOOTLOADER=SBL
if %errorlevel%==0 (
  echo Build result: SUCCESS!
) else (
  echo Build result: FAILED!!
)

:DONE
cd %PKG_DIR%
echo on
@exit /b