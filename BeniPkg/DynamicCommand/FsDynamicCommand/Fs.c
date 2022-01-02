/**
*  @Package     : BeniPkg
*  @FileName    : Fs.c
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

#include "Fs.h"

#include "../../../FatPkg/EnhancedFatDxe/Fat.h"

#define VOLUME_SIGNATURE_FROM_VOL_INTERFACE(a) BASE_CR(a, FAT_VOLUME, VolumeInterface)

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {L"ext" , TypeFlag}, // Display EXT file system.
  {L"ext2", TypeFlag}, // Display EXT file system using Ext2Dxe.
  {NULL   , TypeMax}
  };

/**
  Retrieve HII package list from ImageHandle and publish to HII database.

  @param[in]  ImageHandle           The image handle of the process.

  @return  EFI_HII_HANDLE           The HII handle.

**/
EFI_HII_HANDLE
InitializeHiiPackage (
  IN  EFI_HANDLE                    ImageHandle
  )
{
  EFI_STATUS                        Status;
  EFI_HII_PACKAGE_LIST_HEADER       *PackageList;
  EFI_HII_HANDLE                    HiiHandle;

  //
  // Retrieve HII package list from ImageHandle.
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Publish HII package list to HII Database.
  //
  Status = gHiiDatabase->NewPackageList (
                           gHiiDatabase,
                           PackageList,
                           NULL,
                           &HiiHandle
                           );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return HiiHandle;
}

/**
  Locate EFI_SIMPLE_FILE_SYSTEM_PROTOCOL and show the related information.

  @param  NA

  @retval  NA

**/
STATIC
VOID
EFIAPI
ShowFileSystem (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *FsHandles;
  UINTN                             FsCount;
  UINTN                             Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Volume;
  //
  // EFI_FILE_HANDLE                   Root;
  //
  // Here we can use EFI_FILE_HANDLE too.
  //
  EFI_FILE_PROTOCOL                 *Root;
  EFI_FILE_SYSTEM_INFO              *VolInfo;
  UINTN                             VolSize;

  FsHandles     = NULL;
  FsCount       = 0;
  Status        = EFI_NOT_FOUND;

  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiSimpleFileSystemProtocolGuid,
                NULL,
                &FsCount,
                &FsHandles
                );
  if (EFI_ERROR (Status) || (0 == FsCount)) {
    Print (L"No file system found!\n");
    return;
  } else {
    Print (L"%d file system(s) found!\n", FsCount);
    Print (L"-----------------------------------\n");
  }

  for (Index = 0; Index < FsCount; Index++) {
    //
    // Get the installed EFI_SIMPLE_FILE_SYSTEM_PROTOCOL in FAT driver.
    //
    Status = gBS->HandleProtocol (
                    FsHandles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID**)&Volume
                    );
    if (!EFI_ERROR (Status)) {
      //
      // We only chek FAT file system.
      //
      if (FAT_VOLUME_SIGNATURE != VOLUME_SIGNATURE_FROM_VOL_INTERFACE(Volume)->Signature) {
        continue;
      }

      //
      // EFI_SIMPLE_FILE_SYSTEM_PROTOCOL has the OpenVolume() to
      // get the EFI_FILE_PROTOCOL.
      //
      Status = Volume->OpenVolume (
                        Volume,
                        &Root
                        );
      if (!EFI_ERROR (Status)) {
        VolInfo = NULL;
        VolSize = 0;
        Status  = Root->GetInfo (
                          Root,
                          &gEfiFileSystemInfoGuid,
                          &VolSize,
                          VolInfo
                          );
        if (Status == EFI_BUFFER_TOO_SMALL) {
          VolInfo = AllocateZeroPool (VolSize);
          if (NULL == VolInfo) {
            Status = EFI_OUT_OF_RESOURCES;
          } else {
            Status  = Root->GetInfo (
                              Root,
                              &gEfiFileSystemInfoGuid,
                              &VolSize,
                              VolInfo
                              );
          }
        }
        if (!EFI_ERROR (Status)) {
          Print (L"FS%d:\n", Index);
          Print (L"  Volume Size      : %d\n", VolInfo->Size);
          Print (L"  Volume ReadOnly  : %d\n", VolInfo->ReadOnly);
          Print (L"  Volume VolumeSize: %d\n", VolInfo->VolumeSize);
          Print (L"  Volume FreeSpace : %d\n", VolInfo->FreeSpace);
          Print (L"  Volume BlockSize : %d\n", VolInfo->BlockSize);
          Print (L"  Volume Name      : %s\n", VolInfo->VolumeLabel);
          FreePool (VolInfo);
        }
      }
      Root->Close (Root);
    }
  }

  if (NULL != FsHandles) {
    FreePool (FsHandles);
  }

  return;
}

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
  )
{
  SHELL_STATUS            ShellStatus;
  EFI_STATUS              Status;
  LIST_ENTRY              *CheckPackage;
  CHAR16                  *ProblemParam;
  UINTN                   ParamCount;

  ShellStatus         = SHELL_INVALID_PARAMETER;
  ProblemParam        = NULL;

  //
  // Initialize the Shell library (we must be in non-auto-init...).
  //
  Status = ShellInitialize ();
  if (EFI_ERROR (Status)) {
    return SHELL_ABORTED;
  }

  //
  // Parse the command line.
  //
  Status = ShellCommandLineParse (ParamList, &CheckPackage, &ProblemParam, TRUE);
  if (EFI_ERROR (Status)) {
    if ((Status == EFI_VOLUME_CORRUPTED) && (ProblemParam != NULL) ) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), mFsHiiHandle, L"fs", ProblemParam
        );
      FreePool (ProblemParam);
    }
    goto DONE;
  }

  //
  // Check the number of parameters
  //
  ParamCount = ShellCommandLineGetCount (CheckPackage);
  if (ParamCount != 1) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), mFsHiiHandle, L"fs"
      );
    goto DONE;
  }

  if (ShellCommandLineGetFlag (CheckPackage, L"ext")) {
    ShowExt2FileSystem ();
    goto DONE;
  } else if (ShellCommandLineGetFlag (CheckPackage, L"ext2")) {
    ShowExt2FileSystemEx ();
    goto DONE;
  }

  //
  // Show UEFI supported file system.
  //
  ShowFileSystem ();

DONE:

  if ((ShellStatus != SHELL_SUCCESS) && (EFI_ERROR (Status))) {
    ShellStatus = Status & ~MAX_BIT;
  }

  return ShellStatus;
}
