/**
*  @Package     : BeniPkg
*  @FileName    : Fs.h
*  @Date        : 20211107
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for file system test.
*
*  @History:
*    20211107: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __FS_H__
#define __FS_H__

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/HiiPackageList.h>
#include <Protocol/ShellDynamicCommand.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>

#include <Guid/FileSystemInfo.h>

//
// Used for shell display.
//
extern EFI_HII_HANDLE mFsHiiHandle;

/**
  Show EXT2 file system.

  @param  NA

  @retval  NA

**/
VOID
EFIAPI
ShowExt2FileSystem (
  VOID
  );

/**
  Retrieve HII package list from ImageHandle and publish to HII database.

  @param[in]  ImageHandle           The image handle of the process.

  @return  EFI_HII_HANDLE           The HII handle.

**/
EFI_HII_HANDLE
InitializeHiiPackage (
  IN  EFI_HANDLE                    ImageHandle
  );

/**
  Function for 'fs' command.

  @param[in]  ImageHandle           The image handle.
  @param[in]  SystemTable           The system table.

  @retval  SHELL_SUCCESS            Command completed successfully.
  @retval  SHELL_INVALID_PARAMETER  Command usage error.
  @retval  SHELL_ABORTED            The user aborts the operation.
  @retval  Value                    Unknown error.

**/
SHELL_STATUS
RunFs (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  );

#endif // __FS_H__