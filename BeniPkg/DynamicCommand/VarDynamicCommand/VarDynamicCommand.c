/**
*  @Package     : BeniPkg
*  @FileName    : VarDynamicCommand.c
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

EFI_HII_HANDLE  mVarHiiHandle = NULL;

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
VarCommandHandler (
  IN  EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL      *This,
  IN  EFI_SYSTEM_TABLE                        *SystemTable,
  IN  EFI_SHELL_PARAMETERS_PROTOCOL           *ShellParameters,
  IN  EFI_SHELL_PROTOCOL                      *Shell
  )
{
  gEfiShellParametersProtocol = ShellParameters;
  gEfiShellProtocol           = Shell;

  return RunVar (gImageHandle, SystemTable);
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
VarCommandGetHelp (
  IN  EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL      *This,
  IN  CONST CHAR8                             *Language
  )
{
  return HiiGetString (mVarHiiHandle, STRING_TOKEN (STR_GET_HELP_VAR), Language);
}

EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL mVarDynamicCommand = {
  L"var",
  VarCommandHandler,
  VarCommandGetHelp
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
  Entry point of "var" Dynamic Command.
  Produce the DynamicCommand protocol to handle "var" command.

  @param[in]  ImageHandle           The image handle of the process.
  @param[in]  SystemTable           The EFI System Table pointer.

  @retval  EFI_SUCCESS              Tftp command is executed successfully.
  @retval  EFI_ABORTED              HII package was failed to initialize.
  @retval  Others                   Other errors when executing tftp command.

**/
EFI_STATUS
EFIAPI
VarCommandInitialize (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS    Status;

  mVarHiiHandle = InitializeHiiPackage (ImageHandle);
  if (mVarHiiHandle == NULL) {
    return EFI_ABORTED;
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mVarDynamicCommand
                  );

  return Status;
}

/**
  Var command unload handler.

  @param[in]  ImageHandle           The image handle of the process.

  @retval  EFI_SUCCESS              The image is unloaded.
  @retval  Others                   Failed to unload the image.

**/
EFI_STATUS
EFIAPI
VarUnload (
  IN  EFI_HANDLE                    ImageHandle
  )
{
  EFI_STATUS    Status;

  Status = gBS->UninstallProtocolInterface (
                  ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  &mVarDynamicCommand
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HiiRemovePackages (mVarHiiHandle);

  return EFI_SUCCESS;
}
