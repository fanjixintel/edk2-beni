/**
*  @Package     : BeniPkg
*  @FileName    : GetAppFromFv.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is a UEFI model driver used to get file in UEFI and put it to FS0.
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

#include "GetAppFromFv.h"

typedef struct {
  CHAR16        *AppName;
  GUID          AppGuid;
} APP_INFO_IN_FV;

//
// For applications in FV:
// 1. Build the application;
// 2. Put the name and GUID here;
// 3. Put the GUID and file path in *.fdf;
//
STATIC APP_INFO_IN_FV mApps[] = {
  // HelloWorldApp.inf:
  {L"helloworld.efi", {0xdf674789, 0x4265, 0x4056, {0x9d, 0xd0, 0x51, 0x23, 0xbb, 0x5a, 0x81, 0xf5}}},
  // AppDevPath.inf:
  {L"appdp.efi",      {0x8f989b9a, 0x3e27, 0x4352, {0x8a, 0x16, 0x43, 0xda, 0x1d, 0xba, 0x7a, 0x85}}},
  // MemTest.inf:
  {L"memtest.efi",    {0xf103e554, 0x2509, 0x4935, {0xb7, 0x79, 0x33, 0xf5, 0x71, 0x24, 0x97, 0x62}}},
  // This is the end.
  {NULL, {0}}
};

BOOLEAN mExecutedOnce = FALSE;

EFI_DRIVER_BINDING_PROTOCOL gGetAppFromFvDriverBinding = {
  GetAppFromFvDriverBindingSupported,
  GetAppFromFvDriverBindingStart,
  GetAppFromFvDriverBindingStop,
  0xA,
  NULL,
  NULL
};

/**
  Test to see if this driver supports ControllerHandle.

  @param[in]  This                  Protocol instance pointer.
  @param[in]  ControllerHandle      Handle of the device to test.
  @param[in]  RemainingDevicePath   Optional parameter use to pick a specific
                                    child device to start.

  @retval  EFI_SUCCESS              This driver supports this device.
  @retval  EFI_ALREADY_STARTED      This driver is already running on this device.
  @retval  Others                   This driver does not support this device.

**/
EFI_STATUS
EFIAPI
GetAppFromFvDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS  Status;

  //
  // Test for the EfiSimpleFileSystemProtocol.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Start this driver on ControllerHandle.

  @param[in]  This                  Protocol instance pointer.
  @param[in]  ControllerHandle      Handle of device to bind driver to.
  @param[in]  RemainingDevicePath   Optional parameter use to pick a specific child
                                    device to start.

  @retval  EFI_SUCCESS              The driver is added to ControllerHandle.
  @retval  EFI_OUT_OF_RESOURCES     There are not enough resources to start the
                                    driver.
  @retval  Others                   The driver cannot be added to ControllerHandle.

**/
EFI_STATUS
EFIAPI
GetAppFromFvDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS                        Status;
  VOID                              *Data;
  UINTN                             DataSize;
  EFI_HANDLE                        *FsHandles;
  UINTN                             FsCount;
  UINTN                             Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FsPtr;
  EFI_FILE_HANDLE                   Root;
  EFI_FILE_HANDLE                   NewFile;

  Status        = EFI_DEVICE_ERROR;
  Data          = NULL;
  FsHandles     = NULL;
  FsCount       = 0;
  FsPtr         = NULL;

  BENI_MODULE_START

  if (TRUE == mExecutedOnce) {
    Status = EFI_ALREADY_STARTED;
    goto DONE;
  }

  //
  // Find the first file system, and put the files we found in this file system.
  //
  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiSimpleFileSystemProtocolGuid,
                NULL,
                &FsCount,
                &FsHandles
                );
  for (Index = 0; Index < FsCount; Index++) {
    Status = gBS->HandleProtocol (
                    FsHandles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **)&FsPtr
                    );
    if (!EFI_ERROR (Status)) {
      //
      // TODO: We can't sure it is the Ramdisk.
      //
      break;
    }
  }

  Status = FsPtr->OpenVolume (FsPtr, &Root);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]OpenVolume failed. - %r\n", Status));
    goto DONE;
  }

  //
  // Get file from BIOS binary.
  //
  Index = 0;
  while (NULL != mApps[Index].AppName) {
    Status = GetSectionFromAnyFv (
                &mApps[Index].AppGuid,
                EFI_SECTION_RAW,
                0,
                (VOID **) &Data,
                &DataSize
                );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "[BENI]%s not found. - %r\n", mApps[Index].AppName, Status));
      Index++;
      continue;
    }

    DEBUG ((EFI_D_ERROR, "[BENI]%s found\n", mApps[Index].AppName));

    Status = Root->Open (
                Root,
                &NewFile,
                mApps[Index].AppName,
                (
                  EFI_FILE_MODE_CREATE |
                  EFI_FILE_MODE_READ |
                  EFI_FILE_MODE_WRITE
                ),
                0
                );
    if (!EFI_ERROR (Status)) {
      Status = NewFile->Write (
                NewFile,
                &DataSize,
                Data
                );
      if (!EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "[BENI]%s written\n", mApps[Index].AppName));
      }
      NewFile->Flush (NewFile);
      NewFile->Close (NewFile);
      NewFile = NULL;
    }

    if (NULL != Data) {
      FreePool (Data);
      Data = NULL;
      DataSize = 0;
    }
    Index++;
  }

  //
  // WE only right once, even there is something wrong when writing.
  //
  Root->Close (Root);

DONE:

  if (NULL != Data) {
    FreePool (Data);
    Data = NULL;
  }

  if (NULL != FsHandles) {
    FreePool (FsHandles);
    FsHandles = NULL;
  }

  if (!EFI_ERROR (Status)) {
    mExecutedOnce = TRUE;
  }

  BENI_MODULE_END

  return Status;
}

/**
  Stop this driver on ControllerHandle.

  @param[in]  This                  A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle      A handle to the device being stopped. The handle must
                                    support a bus specific I/O protocol for the driver
                                    to use to stop the device.
  @param[in]  NumberOfChildren      The number of child device handles in ChildHandleBuffer.
  @param[in]  ChildHandleBuffer     An array of child handles to be freed. May be NULL
                                    if NumberOfChildren is 0.

  @retval  EFI_SUCCESS              The device was stopped.
  @retval  EFI_DEVICE_ERROR         The device could not be stopped due to a device error.

**/
EFI_STATUS
EFIAPI
GetAppFromFvDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer OPTIONAL
  )
{
  return EFI_SUCCESS;
}

/**
  The entry point for GetAppFromFv driver, used to find binary and put it in fs0:.

  @param[in]  ImageHandle           The firmware allocated handle for this driver image.
  @param[in]  SystemTable           Pointer to the EFI system table.

  @retval  EFI_SUCCESS              The driver loaded.
  @retval  Others                   The driver did not load.

**/
EFI_STATUS
EFIAPI
GetAppFromFvEntryPoint (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS  Status;

  BENI_MODULE_START

  //
  // Install the Startup Binary Driver Binding Protocol.
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
                      ImageHandle,
                      SystemTable,
                      &gGetAppFromFvDriverBinding,
                      ImageHandle,
                      &gGetAppFromFvComponentName,
                      &gGetAppFromFvComponentName2
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]EfiLibInstallDriverBindingComponentName2 failed. - %r\n", Status));
    return Status;
  }

  BENI_MODULE_END

  return EFI_SUCCESS;
}