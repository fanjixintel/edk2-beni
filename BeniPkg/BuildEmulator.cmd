::::
:: @Package     : BeniPkg
:: @FileName    : BuildEmulator.bat
:: @Date        : 20220110
:: @Author      : Jiangwei
:: @Version     : 1.0
:: @Description :
::   This is used to build binary on Windows.
::
:: @History:
::   20220110: Initialize.
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

set PYTHON_COMMAND=py -3
set TOOLS=VS2019
set ARCH=X64

set PKG_DIR=%CD%
cd ..

call edksetup.bat
call build -p EmulatorPkg\EmulatorPkg.dsc -a %ARCH% -t %TOOLS%

if /I "%1"=="run" (
    cd Build\Emulator%ARCH%\DEBUG_%TOOLS%\X64\ && start WinHost.exe
)

@cd %PKG_DIR%
