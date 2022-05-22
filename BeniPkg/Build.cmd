::::
:: @Package     : BeniPkg
:: @FileName    : BuildOvmf.cmd
:: @Date        : 20211002
:: @Author      : Jiangwei
:: @Version     : 1.0
:: @Description :
::   This is used to build OVMF binary on Windows.
::   VS2015 is needed in order to build successfully.
::
:: @History:
::   20211002: Initialize.
::   20220522: Using VS2015x86 instead of VS2015.
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

:: Help statement.
if /I "%1"=="-h"      goto HELP
if /I "%1"=="--help"  goto HELP
if /I "%1"=="/?"      goto HELP

:: Clean workspace.
if /I "%1"=="clean"   goto CLEAN

set TOOLS=VS2015x86
set PYTHON_COMMAND=py -3

call edksetup.bat
if not "%1"=="" (
  if "%1"=="emulator" goto EMULATOR
  if "%1"=="Payload"  goto PAYLOAD
  if "%1"=="Ovmf"     goto OVMF
) else (
  :: Default build OVMF.
  goto OVMF
)

:EMULATOR
echo Building Emulator ...
call build -p EmulatorPkg\EmulatorPkg.dsc -a X64 -t %TOOLS%
if %errorlevel%==0 (
  echo Build result: SUCCESS!
  if "%2"=="Run" (
    echo Runing emulator...
    cd Build\EmulatorX64\DEBUG_%TOOLS%\X64\ && start WinHost.exe
  )
  goto DONE
) else (
  goto ERROR
)

:OVMF
echo Building OVMF ...
call build -p BeniPkg/BeniPkg.dsc -a X64 -a IA32 -t %TOOLS%
if %errorlevel%==0 (
  echo Build result: SUCCESS!
  copy Build\BeniPkg\DEBUG_%TOOLS%\FV\OVMF.fd BeniPkg\
  goto DONE
) else (
  goto ERROR
)

:PAYLOAD
echo Building Payload ...
call build -p UefiPayloadPkg\UefiPayloadPkg.dsc -a IA32 -a X64 -b DEBUG -t %TOOLS% -D BOOTLOADER=SBL
if %errorlevel%==0 (
  echo Build result: SUCCESS!
  goto DONE
) else (
  goto ERROR
)

:CLEAN
echo Cleaning ...
:: Delete directory Build.
if exist Build    rd /S /Q Build
if exist BuildFsp rd /S /Q BuildFsp
if exist *.log    del /Q *.log
:: Delete files and directories in Conf.
pushd Conf
if exist .cache               rd /S /Q .cache
if exist .AutoGenIdFile.txt   del /Q .AutoGenIdFile.txt
if exist build_rule.txt       del /Q build_rule.txt
if exist target.txt           del /Q target.txt
if exist tools_def.txt        del /Q tools_def.txt
popd
:: Delete BIOS binary.
cd %PKG_DIR%
if exist log.txt    del /Q log.txt
if exist OVMF*.fd   del /Q OVMF*.fd
echo Done!
goto DONE

:ERROR
echo Oops!! You got some problems!
goto DONE

:HELP
echo.
echo Usage:
echo   Build.cmd [Clean] [Emulator [Run]] [Payload] [Ovmf]
goto DONE

:DONE
cd %PKG_DIR%
echo on
@exit /b
