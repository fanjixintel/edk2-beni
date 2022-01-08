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

EFI_HII_HANDLE  mSetupHiiHandle = NULL;

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
  return HiiGetString (mSetupHiiHandle, STRING_TOKEN (STR_GET_HELP_SETUP), Language);
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
  EFI_STATUS    Status;

  mSetupHiiHandle = InitializeHiiPackage (ImageHandle);
  if (mSetupHiiHandle == NULL) {
    return EFI_ABORTED;
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSetupDynamicCommand
                  );

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

  Status = gBS->UninstallProtocolInterface (
                  ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  &mSetupDynamicCommand
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HiiRemovePackages (mSetupHiiHandle);

  return EFI_SUCCESS;
}
