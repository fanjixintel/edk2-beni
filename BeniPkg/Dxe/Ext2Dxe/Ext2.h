/**
*  @Package     : BeniPkg
*  @FileName    : Ext2.h
*  @Date        : 20211130
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is a UEFI model driver used to to initialzie EXT2 file system.
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

#ifndef __GET_APP_FROM_FV_H__
#define __GET_APP_FROM_FV_H__

#include <Uefi.h>
#include <PiDxe.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BeniDebugLib.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/UnicodeCollation.h>

extern EFI_COMPONENT_NAME_PROTOCOL  gExt2ComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gExt2ComponentName2;
extern EFI_DRIVER_BINDING_PROTOCOL  gExt2DriverBinding;

//
// Function prototype for the driver's entry point.
//

/**
  The entry point for EXT2 driver.

  @param[in]  ImageHandle           The firmware allocated handle for this driver image.
  @param[in]  SystemTable           Pointer to the EFI system table.

  @retval  EFI_SUCCESS              The driver loaded.
  @retval  Others                   The driver did not load.

**/
EFI_STATUS
EFIAPI
Ext2DriverEntryPoint (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  );

//
// Function prototypes for the Driver Binding Protocol.
//

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

#endif // __GET_APP_FROM_FV_H__