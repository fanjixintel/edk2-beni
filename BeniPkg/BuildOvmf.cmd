::::
:: @Package     : BeniPkg
:: @FileName    : BuildOvmf.cmd
:: @Date        : 20211002
:: @Author      : Jiangwei
:: @Version     : 1.0
:: @Description :
::   This is used to build OVMF binary on Windows.
::   VS2019 is needed in order to build successfully.
::
:: @History:
::   20211002: Initialize.
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

set TOOLS=VS2019
set ARCH=X64
set PYTHON_COMMAND=py -3

::
:: Add BENI marcos here.
::
@echo. > BeniPkgDefines.dsc.inc
@echo DEFINE DEBUG_ON_SERIAL_PORT    = TRUE         >> BeniPkgDefines.dsc.inc
@echo DEFINE NETWORK_ENABLE          = TRUE         >> BeniPkgDefines.dsc.inc
@echo DEFINE COMPILE_DIR             = DEBUG_VS2019 >> BeniPkgDefines.dsc.inc
@echo DEFINE BENI_EXT2_SUPPORT       = FALSE        >> BeniPkgDefines.dsc.inc
@echo DEFINE BENI_PXE_BOOT           = FALSE        >> BeniPkgDefines.dsc.inc

set PKG_DIR=%CD%
cd ..

call edksetup.bat
call build -p BeniPkg/BeniPkg.dsc -a %ARCH% -t %TOOLS%

echo ====================================================
echo.
if %errorlevel%==0 (
  echo              Build result: SUCCESS!
) else (
  echo              Build result: FAILED!!
)
echo.
echo ====================================================
echo.

if not %errorlevel%==0 goto ERROR
echo BIOS was built successfully!
copy Build\BeniPkg\DEBUG_%TOOLS%\FV\OVMF.fd BeniPkg\
goto DONE

:ERROR
echo Oops!! You got some problems!
goto DONE

:DONE
cd %PKG_DIR%
echo on
@exit /b