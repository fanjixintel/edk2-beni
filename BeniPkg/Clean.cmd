::::
::  @Package     : BeniPkg
::  @FileName    : Clean.cmd
::  @Date        : 20211002
::  @Author      : Jiangwei
::  @Version     : 1.0
::  @Description :
::    This is used to clean work space.
::
::  @History:
::    20211002: Initialize.
::    20220313: Delete BuildFsp too.
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
set PKG_DIR=%CD%
cd ..

:: Delete directory Build;
if exist Build rd /S /Q Build
if exist BuildFsp rd /S /Q BuildFsp
if exist *.log del /Q *.log

:: Delete files and directories in Conf;
pushd Conf
if exist .cache rd /S /Q .cache
if exist .AutoGenIdFile.txt  del /Q .AutoGenIdFile.txt
if exist build_rule.txt  del /Q build_rule.txt
if exist target.txt      del /Q target.txt
if exist tools_def.txt   del /Q tools_def.txt
popd

cd %PKG_DIR%

:: Delete BIOS binary;
if exist log.txt  del /Q log.txt
if exist OVMF*.fd  del /Q OVMF*.fd

echo on
@exit /b
