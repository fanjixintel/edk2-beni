/**
*  @Package     : BeniPkg
*  @FileName    : SetupDynamicCommand.c
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

BENI_SETUP_PRIVATE_DATA      *mPrivateData = NULL;

HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    BENI_SETUP_FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

/**
  This is the shell command handler function pointer callback type. This
  function handles the command when it is invoked in the shell.

  @param[in]  This                  The instance of the EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL.
  @param[in]  SystemTable           The pointer to the system table.
  @param[in]  ShellParameters       The parameters associated with the command.
  @param[in]  Shell                 The instance of the shell protocol used in the context
                                    of processing this command.

  @return  EFI_SUCCESS              The operation was successful
  @return  Other                    The operation failed.

**/
SHELL_STATUS
EFIAPI
SetupCommandHandler (
  IN  EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL      *This,
  IN  EFI_SYSTEM_TABLE                        *SystemTable,
  IN  EFI_SHELL_PARAMETERS_PROTOCOL           *ShellParameters,
  IN  EFI_SHELL_PROTOCOL                      *Shell
  )
{
  gEfiShellParametersProtocol = ShellParameters;
  gEfiShellProtocol           = Shell;

  return RunSetup (gImageHandle, SystemTable);
}

/**
  This is the command help handler function pointer callback type.  This
  function is responsible for displaying help information for the associated
  command.

  @param[in]  This                  The instance of the EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL.
  @param[in]  Language              The pointer to the language string to use.

  @return  string                   Pool allocated help string, must be freed by caller.

**/
CHAR16 *
EFIAPI
SetupCommandGetHelp (
  IN  EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL      *This,
  IN  CONST CHAR8                             *Language
  )
{
  return HiiGetString (mPrivateData->SetupHiiHandle, STRING_TOKEN (STR_GET_HELP_SETUP), Language);
}

EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL mSetupDynamicCommand = {
  L"setup",
  SetupCommandHandler,
  SetupCommandGetHelp
};

/**
  Entry point of "setup" Dynamic Command.
  Produce the DynamicCommand protocol to handle "setup" command.

  @param[in]  ImageHandle           The image handle of the process.
  @param[in]  SystemTable           The EFI System Table pointer.

  @retval  EFI_SUCCESS              Tftp command is executed successfully.
  @retval  EFI_ABORTED              HII package was failed to initialize.
  @retval  Others                   Other errors when executing tftp command.

**/
EFI_STATUS
EFIAPI
SetupCommandInitialize (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS                        Status;
  EFI_HII_PACKAGE_LIST_HEADER       *PackageList;

  BENI_MODULE_START

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
    DEBUG ((EFI_D_ERROR, "[BENI]Open EfiHiiPackageListProtocol failed. - %r\n", Status));
    goto DONE;
  }

  mPrivateData = AllocateZeroPool (sizeof (BENI_SETUP_PRIVATE_DATA));
  if (NULL == mPrivateData) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    Status =  EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  mPrivateData->Signature = BENI_SETUP_PRIVATE_SIGNATURE;

  mPrivateData->ConfigAccess.ExtractConfig  = ExtractConfig;
  mPrivateData->ConfigAccess.RouteConfig    = RouteConfig;
  mPrivateData->ConfigAccess.Callback       = DriverCallback;

  Status = gBS->LocateProtocol (
                  &gEfiFormBrowser2ProtocolGuid,
                  NULL,
                  (VOID **) &mPrivateData->FormBrowser2
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Locate EfiFormBrowser2Protocol failed. -%r\n", Status));
    goto DONE;
  }

  //
  // Publish sample formset.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPrivateData->DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]InstallMultipleProtocolInterfaces failed. - %r\n", Status));
    goto DONE;
  }

  //
  // Publish HII package list to HII Database.
  //
  Status = gHiiDatabase->NewPackageList (
                          gHiiDatabase,
                          PackageList,
                          mPrivateData->DriverHandle,
                          &mPrivateData->SetupHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Publish HII package list failed. - %r\n", Status));
    goto DONE;
  }

  //
  // Update components in page 2.
  //
  UpdatePageForm ();

  //
  // Install shell command.
  //
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSetupDynamicCommand
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Install shell command failed. - %r\n", Status));
    goto DONE;
  }

DONE:

  BENI_MODULE_END

  return Status;
}

/**
  Setup command unload handler.

  @param[in]  ImageHandle           The image handle of the process.

  @retval  EFI_SUCCESS              The image is unloaded.
  @retval  Others                   Failed to unload the image.

**/
EFI_STATUS
EFIAPI
SetupUnload (
  IN  EFI_HANDLE                    ImageHandle
  )
{
  EFI_STATUS    Status;

  BENI_MODULE_START

  Status = gBS->UninstallProtocolInterface (
                  ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  &mSetupDynamicCommand
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (NULL != mPrivateData->DriverHandle) {
    gBS->UninstallMultipleProtocolInterfaces (
          mPrivateData->DriverHandle,
          &gEfiDevicePathProtocolGuid,
          &mHiiVendorDevicePath,
          &gEfiHiiConfigAccessProtocolGuid,
          &mPrivateData->ConfigAccess,
          NULL
          );
    mPrivateData->DriverHandle = NULL;
  }

  if (NULL != mPrivateData->SetupHiiHandle) {
    HiiRemovePackages (mPrivateData->SetupHiiHandle);
    mPrivateData->SetupHiiHandle = NULL;
  }

  if (NULL != mPrivateData) {
    FreePool (mPrivateData);
    mPrivateData = NULL;
  }

  BENI_MODULE_END

  return EFI_SUCCESS;
}
