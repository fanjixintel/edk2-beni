::::
::  @Package     : BeniPkg
::  @FileName    : BuildOvmf.cmd
::  @Date        : 20211002
::  @Author      : Jiangwei
::  @Version     : 1.0
::  @Description :
::    This is used to build OVMF binary on Windows.
::    VS2019 is needed in order to build successfully.
::
::  @History:
::    20211002: Initialize.
::
::  This program and the accompanying materials
::  are licensed and made available under the terms and conditions of the BSD License
::  which accompanies this distribution. The full text of the license may be found at
::  http://opensource.org/licenses/bsd-license.php
::
::  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
::  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
::::

@echo off

set COMPILE_TOOLS=VS2019
set COMPILE_ARCH=X64
set PYTHON_COMMAND=py -3

set PKG_DIR=%CD%
cd ..

call edksetup.bat
call build -p BeniPkg/BeniPkg.dsc -a %COMPILE_ARCH% -t %COMPILE_TOOLS%

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
copy Build\BeniPkg\DEBUG_%COMPILE_TOOLS%\FV\OVMF.fd BeniPkg\
goto DONE

:ERROR
echo Oops!! You got some problems!
goto DONE

:DONE
cd %PKG_DIR%
echo on
@exit /b