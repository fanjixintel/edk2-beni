/**
*  @Package     : BeniPkg
*  @FileName    : Disk.c
*  @Date        : 20211128
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for disk test.
*
*  @History:
*    20211128: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Disk.h"

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {NULL , TypeMax}
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
  Show disk informations.

  @param  NA

  @retval  NA

**/
STATIC
VOID
EFIAPI
ShowDiskInfo (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *Handles;
  UINTN                             Count;
  UINTN                             Index;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;

  Handles     = NULL;
  Count       = 0;
  Status        = EFI_NOT_FOUND;

  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiBlockIoProtocolGuid,
                NULL,
                &Count,
                &Handles
                );
  if (EFI_ERROR (Status) || (0 == Count)) {
    Print (L"No block found!\n");
    return;
  } else {
    Print (L"%d block(s) found!\n", Count);
    Print (L"-----------------------------------\n");
  }

  for (Index = 0; Index < Count; Index++) {
    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo
                  );
    if (!EFI_ERROR (Status)) {
      Print (L"BLOCK%d:\n", Index);
      Print (L"  Media ID          : %d\n", BlockIo->Media->MediaId);
      Print (L"  Removable Media   : %d\n", BlockIo->Media->RemovableMedia);
      Print (L"  Media Present     : %d\n", BlockIo->Media->MediaPresent);
      Print (L"  Logical Partition : %d\n", BlockIo->Media->LogicalPartition);
      Print (L"  Read Only         : %d\n", BlockIo->Media->ReadOnly);
      Print (L"  Write Caching     : %d\n", BlockIo->Media->WriteCaching);
      Print (L"  Block Size        : %d\n", BlockIo->Media->BlockSize);
      Print (L"  Last Block        : %d\n", BlockIo->Media->LastBlock);
    }
  }

  if (NULL != Handles) {
    FreePool (Handles);
  }

  return;
}

/**
  Function for 'disk' command.

  @param[in]  ImageHandle           The image handle.
  @param[in]  SystemTable           The system table.

  @retval  SHELL_SUCCESS            Command completed successfully.
  @retval  SHELL_INVALID_PARAMETER  Command usage error.
  @retval  SHELL_ABORTED            The user aborts the operation.
  @retval  Value                    Unknown error.

**/
SHELL_STATUS
RunDisk (
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
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), mDiskHiiHandle, L"disk", ProblemParam
        );
      FreePool (ProblemParam);
    }
    goto ERROR;
  }

  //
  // Check the number of parameters
  //
  ParamCount = ShellCommandLineGetCount (CheckPackage);
  if (ParamCount != 1) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), mDiskHiiHandle, L"disk"
      );
    goto ERROR;
  }

  ShowDiskInfo ();

ERROR:

  if ((ShellStatus != SHELL_SUCCESS) && (EFI_ERROR (Status))) {
    ShellStatus = Status & ~MAX_BIT;
  }

  return ShellStatus;
}