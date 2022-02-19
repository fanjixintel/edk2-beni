/**
*  @Package     : BeniPkg
*  @FileName    : Var.c
*  @Date        : 20220219
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for variable test.
*
*  @History:
*    20220219: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Var.h"

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {NULL ,       TypeMax  }
  };

/**
  Check if EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL exists.

  @param  NA

  @retval  NA

**/
VOID
CheckFVB (
  VOID
  )
{
  EFI_STATUS                             Status;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL     *Fvb;
  UINTN                                  Count;
  UINTN                                  Index;
  EFI_HANDLE                             *Handles;
  EFI_PHYSICAL_ADDRESS                   Address;

  Handles     = NULL;
  Count       = 0;
  Status        = EFI_NOT_FOUND;

  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiFirmwareVolumeBlockProtocolGuid,
                NULL,
                &Count,
                &Handles
                );
  if (EFI_ERROR (Status) || (0 == Count)) {
    Print (L"No FVB found!\n");
    return;
  } else {
    Print (L"%d FVB(s) found!\n", Count);
    Print (L"-----------------------------------\n");
  }

  for (Index = 0; Index < Count; Index++) {
    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiFirmwareVolumeBlockProtocolGuid,
                  (VOID **) &Fvb
                  );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Print (L"FVB%d:\n", Index);

    Status = Fvb->GetPhysicalAddress (Fvb, &Address);
    if (!EFI_ERROR (Status)) {
      Print (L"  Address: 0x%016x\n", Address);
    }
  }

  return;
}

/**
  Function for 'var' command.

  @param[in]  ImageHandle           The image handle.
  @param[in]  SystemTable           The system table.

  @retval  SHELL_SUCCESS            Command completed successfully.
  @retval  SHELL_INVALID_PARAMETER  Command usage error.
  @retval  SHELL_ABORTED            The user aborts the operation.
  @retval  Value                    Unknown error.

**/
SHELL_STATUS
RunVar (
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
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), mVarHiiHandle, L"var", ProblemParam
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
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), mVarHiiHandle, L"var"
      );
    goto DONE;
  }

  CheckFVB ();

DONE:

  if ((ShellStatus != SHELL_SUCCESS) && (EFI_ERROR (Status))) {
    ShellStatus = Status & ~MAX_BIT;
  }

  return ShellStatus;
}
