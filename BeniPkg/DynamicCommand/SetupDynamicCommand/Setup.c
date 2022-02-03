/**
*  @Package     : BeniPkg
*  @FileName    : Setup.c
*  @Date        : 20220105
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for setup test.
*
*  @History:
*    20220105: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Setup.h"
#include "SetupData.h"

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {NULL , TypeMax }
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
  Prepare data for display.

  @param  NA

  @return  EFI_SUCCESS              Operation succeeded.
  @return  Others                   Operation succeeded.

**/
EFI_STATUS
PrepareData (
  VOID
  )
{
  EFI_STATUS         Status;
  BENI_SETUP_DATA    *Data;
  UINTN              DataSize;

  Status    = EFI_UNSUPPORTED;
  Data      = NULL;
  DataSize  = sizeof (BENI_SETUP_DATA);

  Data = AllocateZeroPool (DataSize);
  if (NULL == Data) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gRT->GetVariable (
                  BENI_SETUP_DATA_VAR_NAME,
                  &gBeniSetupFormSetGuid,
                  NULL,
                  &DataSize,
                  Data
                  );
  if (EFI_ERROR (Status)) {
    if (EFI_NOT_FOUND == Status) {
      DEBUG ((EFI_D_ERROR, "[BENI]Initialize Setup data\n"));
      Data->Data1 = 1;
      Data->Data2 = 1;
      DataSize    = sizeof (BENI_SETUP_DATA);
      Status = gRT->SetVariable (
                    BENI_SETUP_DATA_VAR_NAME,
                    &gBeniSetupFormSetGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    DataSize,
                    Data
                    );
      DEBUG ((EFI_D_ERROR, "[BENI]Status: - %r\n", Status));
    }
  }

  return Status;
}

/**
  Display page.

  @param  NA

  @return  NA

**/
VOID
DisplayPage (
  VOID
  )
{
  EFI_STATUS                   Status;
  EFI_BROWSER_ACTION_REQUEST   ActionRequest;

  Status        = EFI_UNSUPPORTED;
  ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;

  Status = gFormBrowser2->SendForm (
                            gFormBrowser2,
                            &mSetupHiiHandle,
                            1,
                            &gBeniSetupFormSetGuid,
                            0,
                            NULL,
                            &ActionRequest
                            );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]SendForm failed. - %r\n", Status));
  }
}

/**
  Function for 'setup' command.

  @param[in]  ImageHandle           The image handle.
  @param[in]  SystemTable           The system table.

  @retval  SHELL_SUCCESS            Command completed successfully.
  @retval  SHELL_INVALID_PARAMETER  Command usage error.
  @retval  SHELL_ABORTED            The user aborts the operation.
  @retval  Value                    Unknown error.

**/
SHELL_STATUS
RunSetup (
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
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), mSetupHiiHandle, L"setup", ProblemParam
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
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), mSetupHiiHandle, L"setup"
      );
    goto DONE;
  }

  Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &gFormBrowser2);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Locate EfiFormBrowser2Protocol failed. -%r\n", Status));
    goto DONE;
  }

  Status = PrepareData ();
  if (!EFI_ERROR (Status)) {
    DisplayPage ();
  }

DONE:

  if ((ShellStatus != SHELL_SUCCESS) && (EFI_ERROR (Status))) {
    ShellStatus = Status & ~MAX_BIT;
  }

  return ShellStatus;
}
