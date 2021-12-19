/**
*  @Package     : BeniPkg
*  @FileName    : FsDynamicCommand.c
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

EFI_HII_HANDLE  mFsHiiHandle = NULL;

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
FsCommandHandler (
  IN  EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL      *This,
  IN  EFI_SYSTEM_TABLE                        *SystemTable,
  IN  EFI_SHELL_PARAMETERS_PROTOCOL           *ShellParameters,
  IN  EFI_SHELL_PROTOCOL                      *Shell
  )
{
  gEfiShellParametersProtocol = ShellParameters;
  gEfiShellProtocol           = Shell;

  return RunFs (gImageHandle, SystemTable);
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
FsCommandGetHelp (
  IN  EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL      *This,
  IN  CONST CHAR8                             *Language
  )
{
  return HiiGetString (mFsHiiHandle, STRING_TOKEN (STR_GET_HELP_FS), Language);
}

EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL mFsDynamicCommand = {
  L"fs",
  FsCommandHandler,
  FsCommandGetHelp
};

/**
  Entry point of "fs" Dynamic Command.
  Produce the DynamicCommand protocol to handle "fs" command.

  @param[in]  ImageHandle           The image handle of the process.
  @param[in]  SystemTable           The EFI System Table pointer.

  @retval  EFI_SUCCESS              Tftp command is executed successfully.
  @retval  EFI_ABORTED              HII package was failed to initialize.
  @retval  Others                   Other errors when executing tftp command.

**/
EFI_STATUS
EFIAPI
FsCommandInitialize (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS    Status;

  mFsHiiHandle = InitializeHiiPackage (ImageHandle);
  if (mFsHiiHandle == NULL) {
    return EFI_ABORTED;
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mFsDynamicCommand
                  );

  return Status;
}

/**
  Tftp driver unload handler.

  @param[in]  ImageHandle           The image handle of the process.

  @retval  EFI_SUCCESS              The image is unloaded.
  @retval  Others                   Failed to unload the image.

**/
EFI_STATUS
EFIAPI
FsUnload (
  IN  EFI_HANDLE                    ImageHandle
  )
{
  EFI_STATUS    Status;

  Status = gBS->UninstallProtocolInterface (
                  ImageHandle,
                  &gEfiShellDynamicCommandProtocolGuid,
                  &mFsDynamicCommand
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HiiRemovePackages (mFsHiiHandle);

  return EFI_SUCCESS;
}
