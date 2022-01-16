/**
*  @Package     : BeniPkg
*  @FileName    : MemoryAllocationTest.c
*  @Date        : 20190108
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This driver is used to test memory allocation functions.
*
*  @History:
*    20190108: Initialize.
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

typedef struct _LARGE_STRUCT_ {
  UINTN    Elem1;
  UINT8    Elem2[512];
  UINTN    Elem3;
} LARGE_STRUCT;

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle this driver.
  @param[in]  SystemTable           Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
MemoryAllocationTestEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  UINTN    Count;
  VOID     *Ptr;

  Count = 1;
  while (TRUE) {
    Ptr = AllocatePool (Count);
    if (NULL != Ptr) {
      FreePool (Ptr);
      Ptr = NULL;
    } else {
      break;
    }
    Count = Count << 1;
  }

  DEBUG ((EFI_D_ERROR, "[BENI]Memory allocated: 0x%x (bytes)\n", Count));

  return EFI_SUCCESS;
}
