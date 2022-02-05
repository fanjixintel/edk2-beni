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
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param[in]  This                  Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Request               A null-terminated Unicode string in
                                    <ConfigRequest> format.
  @param[out] Progress              On return, points to a character in the Request
                                    string. Points to the string's null terminator if
                                    request was successful. Points to the most recent
                                    '&' before the first failing name/value pair (or
                                    the beginning of the string if the failure is in
                                    the first name/value pair) if the request was not
                                    successful.
  @param[out] Results               A null-terminated Unicode string in
                                    <ConfigAltResp> format which has all values filled
                                    in for the names in the Request string. String to
                                    be allocated by the called function.

  @retval  EFI_SUCCESS              The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES     Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER    Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND            Routing data doesn't match any storage in this
                                    driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL    *This,
  IN  CONST EFI_STRING                        Request,
  OUT EFI_STRING                              *Progress,
  OUT EFI_STRING                              *Results
  )
{
  BENI_MODULE_START
  BENI_MODULE_END
  return EFI_SUCCESS;
}

/**
  This function processes the results of changes in configuration.

  @param[in]  This                  Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Configuration         A null-terminated Unicode string in <ConfigResp>
                                    format.
  @param[out] Progress              A pointer to a string filled in with the offset of
                                    the most recent '&' before the first failing
                                    name/value pair (or the beginning of the string if
                                    the failure is in the first name/value pair) or
                                    the terminating NULL if all was successful.

  @retval  EFI_SUCCESS              The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER    Configuration is NULL.
  @retval  EFI_NOT_FOUND            Routing data doesn't match any storage in this
                                    driver.

**/
EFI_STATUS
EFIAPI
RouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL    *This,
  IN  CONST EFI_STRING                        Configuration,
  OUT EFI_STRING                              *Progress
  )
{
  BENI_MODULE_START
  BENI_MODULE_END
  return EFI_SUCCESS;
}

/**
  This function processes the results of changes in configuration.

  @param[in]  This                  Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Action                Specifies the type of action taken by the browser.
  @param[in]  QuestionId            A unique value which is sent to the original
                                    exporting driver so that it can identify the type
                                    of data to expect.
  @param[in]  Type                  The type of value for the question.
  @param[in]  Value                 A pointer to the data being sent to the original
                                    exporting driver.
  @param[out] ActionRequest         On return, points to the action requested by the
                                    callback function.

  @retval  EFI_SUCCESS              The callback successfully handled the action.
  @retval  EFI_OUT_OF_RESOURCES     Not enough storage is available to hold the
                                    variable and its data.
  @retval  EFI_DEVICE_ERROR         The variable could not be saved.
  @retval  EFI_UNSUPPORTED          The specified Action is not supported by the
                                    callback.

**/
EFI_STATUS
EFIAPI
DriverCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL    *This,
  IN  EFI_BROWSER_ACTION                      Action,
  IN  EFI_QUESTION_ID                         QuestionId,
  IN  UINT8                                   Type,
  IN  EFI_IFR_TYPE_VALUE                      *Value,
  OUT EFI_BROWSER_ACTION_REQUEST              *ActionRequest
  )
{
  BENI_MODULE_START

  if (Action == EFI_BROWSER_ACTION_CHANGING) {
    switch (QuestionId) {
      case PAGE_TEXT_ID:
        DEBUG ((DEBUG_ERROR, "%a %d PAGE_TEXT_ID\n", __FUNCTION__, __LINE__));
        break;
      default:
        break;
    }
  } else if (Action == EFI_BROWSER_ACTION_CHANGED) {
    switch (QuestionId) {
      case PAGE_TEXT_ID:
        DEBUG ((DEBUG_ERROR, "%a %d PAGE_TEXT_ID\n", __FUNCTION__, __LINE__));
        break;
      default:
        break;
    }
  }

  BENI_MODULE_END
  return EFI_SUCCESS;
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
      UnicodeSPrint (
        Data->DriverDescriptionData,
        sizeof (Data->DriverDescriptionData),
        L"Hello World"
        );
      Data->Id = 1;
      DataSize = sizeof (BENI_SETUP_DATA);
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

  Status = mPrivateData->FormBrowser2->SendForm (
                            mPrivateData->FormBrowser2,
                            &mPrivateData->SetupHiiHandle,
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
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), mPrivateData->SetupHiiHandle, L"setup", ProblemParam
        );
      FreePool (ProblemParam);
    }
    goto DONE;
  }

  //
  // Check the number of parameters.
  //
  ParamCount = ShellCommandLineGetCount (CheckPackage);
  if (ParamCount != 1) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), mPrivateData->SetupHiiHandle, L"setup"
      );
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
