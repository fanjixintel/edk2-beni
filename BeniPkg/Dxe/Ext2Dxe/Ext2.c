/**
*  @Package     : BeniPkg
*  @FileName    : Ext2.c
*  @Date        : 20211130
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is a UEFI model driver used to initialzie EXT2 file system.
*
*  @History:
*    20211130: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Ext2.h"

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
Ext2DriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath OPTIONAL
  );

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
Ext2DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath OPTIONAL
  );

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
Ext2DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer OPTIONAL
  );

//
// DriverBinding protocol instance.
//
EFI_DRIVER_BINDING_PROTOCOL gExt2DriverBinding = {
  Ext2DriverBindingSupported,
  Ext2DriverBindingStart,
  Ext2DriverBindingStop,
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
Ext2DriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS              Status;
  EFI_DISK_IO_PROTOCOL    *DiskIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **) &DiskIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test.
  //
  gBS->CloseProtocol (
         ControllerHandle,
         &gEfiDiskIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
         );

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return Status;
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
Ext2DriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS              Status;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  EFI_DISK_IO_PROTOCOL    *DiskIo;
  EFI_DISK_IO2_PROTOCOL   *DiskIo2;

  Status = EFI_DEVICE_ERROR;

  BENI_MODULE_START

  // Status = InitializeUnicodeCollationSupport (This->DriverBindingHandle);
  // if (EFI_ERROR (Status)) {
  //   goto EXIT;
  // }

  //
  // Open our required BlockIo and DiskIo.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    goto EXIT;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **) &DiskIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto EXIT;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIo2ProtocolGuid,
                  (VOID **) &DiskIo2,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    DiskIo2 = NULL;
  }

<<<<<<< HEAD
  if (NULL != DevicePathFromHandle (ControllerHandle)) {
    DEBUG ((DEBUG_ERROR, "%a %d 0x%p\n", __FUNCTION__, __LINE__, ControllerHandle));
  }

=======
>>>>>>> 08fd28f8ad9569abe05301594c5542bc2107ff6e
  //
  // Allocate Volume structure. In Ext2AllocateVolume(), Resources
  // are allocated with protocol installed.
  //
  Status = Ext2AllocateVolume (ControllerHandle, DiskIo, DiskIo2, BlockIo);
  //
  // When the media changes on a device it will Reinstall the BlockIo interface.
  // This will cause a call to our Stop(), and a subsequent reentrant call to our
  // Start() successfully. We should leave the device open when this happen.
  //
  if (EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiSimpleFileSystemProtocolGuid,
                    NULL,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      gBS->CloseProtocol (
             ControllerHandle,
             &gEfiDiskIoProtocolGuid,
             This->DriverBindingHandle,
             ControllerHandle
             );
      gBS->CloseProtocol (
             ControllerHandle,
             &gEfiDiskIo2ProtocolGuid,
             This->DriverBindingHandle,
             ControllerHandle
             );
    }
  }

EXIT:

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
Ext2DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer OPTIONAL
  )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FileSystem;
  EXT2_VOLUME                       *Volume;
  EFI_DISK_IO2_PROTOCOL             *DiskIo2;

  DiskIo2 = NULL;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **) &FileSystem,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    Volume  = VOLUME_FROM_VOL_INTERFACE (FileSystem);
    DiskIo2 = Volume->DiskIo2;
    Status  = Ext2AbandonVolume (Volume);
  }

  if (!EFI_ERROR (Status)) {
    if (DiskIo2 != NULL) {
      Status = gBS->CloseProtocol (
                      ControllerHandle,
                      &gEfiDiskIo2ProtocolGuid,
                      This->DriverBindingHandle,
                      ControllerHandle
                      );
      ASSERT_EFI_ERROR (Status);
    }
    Status = gBS->CloseProtocol (
                    ControllerHandle,
                    &gEfiDiskIoProtocolGuid,
                    This->DriverBindingHandle,
                    ControllerHandle
                    );
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

/**
  The entry point for EXT2 driver.

  @param[in]  ImageHandle           The firmware allocated handle for this driver image.
  @param[in]  SystemTable           Pointer to the EFI system table.

  @retval  EFI_SUCCESS              The driver loaded.
  @retval  Others                   The driver did not load.

**/
EFI_STATUS
EFIAPI
Ext2EntryPoint (
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
                      &gExt2DriverBinding,
                      ImageHandle,
                      &gExt2ComponentName,
                      &gExt2ComponentName2
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]EfiLibInstallDriverBindingComponentName2 failed. - %r\n", Status));
    return Status;
  }

  BENI_MODULE_END

  return EFI_SUCCESS;
}
