/**
*  @Package     : BeniPkg
*  @FileName    : HelloWorldApp.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is an application to say Hello World.
*
*  @History:
*    20211004: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include  <Uefi.h>

#include  <Library/UefiLib.h>
#include  <Library/ShellCEntryLib.h>

/***
  The main entry of the application.

  @retval  0                        The application exited normally.
  @retval  Other                    An error occurred.

***/
INTN
EFIAPI
ShellAppMain (
  IN  UINTN                         Argc,
  IN  CHAR16                        **Argv
  )
{
  Print(L"Hello World.\n");

  return 0;
}
