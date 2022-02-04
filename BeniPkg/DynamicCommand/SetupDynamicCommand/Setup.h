/**
*  @Package     : BeniPkg
*  @FileName    : Setup.h
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

#ifndef __SETUP_H__
#define __SETUP_H__

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BeniDebugLib.h>

#include <Guid/BeniSetupHii.h>

#include <Protocol/HiiPackageList.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/ShellDynamicCommand.h>

#define BENI_SETUP_PRIVATE_SIGNATURE SIGNATURE_32 ('B', 'S', 'T', 'P')

#pragma pack(1)
//
// HII specific Vendor Device Path definition.
//
typedef struct {
  VENDOR_DEVICE_PATH           VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL     End;
} HII_VENDOR_DEVICE_PATH;
#pragma pack()

typedef struct {
  UINTN                             Signature;

  EFI_HII_HANDLE                    SetupHiiHandle;
  EFI_HANDLE                        DriverHandle;

  //
  // Consumed protocol
  //
  EFI_FORM_BROWSER2_PROTOCOL        *FormBrowser2;
  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL    ConfigAccess;
} BENI_SETUP_PRIVATE_DATA;

#define BENI_SETUP_PRIVATE_FROM_THIS(a)  CR(a, BENI_SETUP_PRIVATE_DATA, ConfigAccess, BENI_SETUP_PRIVATE_SIGNATURE)

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
  );


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
  );

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
  );

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
  );

extern BENI_SETUP_PRIVATE_DATA      *mPrivateData;

#endif // __SETUP_H__
