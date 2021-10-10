/**
*  @Package     : BeniPkg
*  @FileName    : AppDevPath.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is an application to print device path.
*
*  @History:
*    20211004: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include  <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/LoadedImage.h>

/**
  The entry point for the application.

  @param[in]  ImageHandle           The firmware allocated handle for the EFI image.
  @param[in]  SystemTable           A pointer to the EFI System Table.

  @retval  EFI_SUCCESS              The entry point is executed successfully.
  @retval  Others                   Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_LOADED_IMAGE_PROTOCOL    *LoadedImage;

  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID**)&LoadedImage,
                  gImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    Print (L"App DevPath: %s%s\n",
            ConvertDevicePathToText (DevicePathFromHandle (LoadedImage->DeviceHandle), TRUE, FALSE),
            ConvertDevicePathToText (LoadedImage->FilePath, TRUE, FALSE));
  }

  return Status;
}