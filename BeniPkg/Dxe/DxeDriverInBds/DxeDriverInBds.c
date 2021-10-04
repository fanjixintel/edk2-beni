/**
*  @Package     : BeniPkg
*  @FileName    : DxeDriverInBds.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    We can have the DXE driver executed in BDS stage,
*    and this driver is an example.
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

#include <Uefi.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BeniDebugLib.h>

#include <Protocol/PciIo.h>

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle this driver.
  @param[in]  SystemTable           Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
DxeDriverInBdsEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS              Status;
  EFI_PCI_IO_PROTOCOL     *PciIo;
  EFI_HANDLE              *HandleArray;
  UINTN                   HandleArrayCount;
  UINTN                   Index;
  UINT16                  VendorId;
  UINT16                  DeviceId;

  BENI_MODULE_START

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleArrayCount,
                  &HandleArray
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]LocateHandleBuffer failed. - %r\n", Status));
    return Status;
  }

  for (Index = 0; Index < HandleArrayCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleArray[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint16,
                      0x00,
                      1,
                      &VendorId
                      );
      if (EFI_ERROR (Status)) {
        continue;
      }

      Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint16,
                      0x02,
                      1,
                      &DeviceId
                      );
      if (EFI_ERROR (Status)) {
        continue;
      }

      DEBUG ((EFI_D_ERROR, "[BENI]VendorId: 0x%04x, DeviceId: 0x%04x\n", VendorId, DeviceId));
    }
  }

  if (NULL != HandleArray) {
    FreePool (HandleArray);
    HandleArray = NULL;
  }

  BENI_MODULE_END

  return EFI_SUCCESS;
}