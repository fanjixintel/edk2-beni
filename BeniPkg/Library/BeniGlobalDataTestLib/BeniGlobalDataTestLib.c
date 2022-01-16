/**
*  @Package     : BeniPkg
*  @FileName    : GlobalDataTestLib.c
*  @Date        : 20180616
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is used to print the address of global data.
*
*  @History:
*    20180616: Initialize.
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
#include <BeniData.h>

#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/BeniGlobalDataTestLib.h>

UINTN      gBeniData = 0;
VOID       *gBuffer = NULL;

/**
  Print the address of the global variables.

  @param  NA

  @retval  EFI_SUCCESS              Executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
PrintGlobalVar (
  VOID
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Addresss of gData: 0x%x\n", &gBeniData));

  if (NULL == gBuffer) {
    gBuffer = AllocatePool (100);
    DEBUG ((EFI_D_ERROR, "[BENI]AllocatePool for gBuffer 0x%x\n", gBuffer));
  }

  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestVersion: 0x%x\n", PcdGet32 (PcdTestVersion)));

  return EFI_SUCCESS;
}

/**
  Print the address of the global variables.
  Version 2.

  @param  NA

  @retval  EFI_SUCCESS              Executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
PrintGlobalVar2 (
  VOID
  )
{
  EFI_STATUS    Status;
  VOID          *Buffer;
  UINTN         Size;

  Size     = 0;
  Buffer   = NULL;

  Status = gRT->GetVariable (
                  BENI_DATA_NAME,
                  &gBeniGlobalDataGuid,
                  NULL,
                  &Size,
                  Buffer
                  );
  if (EFI_ERROR (Status)) {
    if (EFI_BUFFER_TOO_SMALL == Status) {
      DEBUG ((EFI_D_ERROR, "[BENI]Size : %d\n", Size));
      Buffer = AllocatePool (Size);
      if (NULL == Buffer) {
        DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
        return EFI_OUT_OF_RESOURCES;
      }
      Status = gRT->GetVariable (
                      BENI_DATA_NAME,
                      &gBeniGlobalDataGuid,
                      NULL,
                      &Size,
                      Buffer
                      );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "[BENI]GetVariable failed 0. - %r\n", Status));
        return Status;
      }
    } else {
      DEBUG ((EFI_D_ERROR, "[BENI]GetVariable failed 1. - %r\n", Status));
      return Status;
    }
  }

  if (BENI_DATA_MAGIC == *((UINT32 *)Buffer)) {
    DEBUG ((EFI_D_ERROR, "[BENI]I got the variable data\n"));
  }

  if (NULL != Buffer) {
    FreePool (Buffer);
  }

  return EFI_SUCCESS;
}

/**
  Constructor of Global Data Test Library Instance.

  @param[in]  ImageHandle           The firmware allocated handle for the EFI image.
  @param[in]  SystemTable           A pointer to the EFI System Table.

  @retval  EFI_SUCCESS              The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BeniGlobalDataTestLibConstructor (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  if (0xABCDDBCA == PcdGet32 (PcdTestVersion)) {
    DEBUG ((EFI_D_ERROR, "[BENI]PcdSet\n"));
    PcdSet32S (PcdTestVersion, 0x00000001);
  }

  return EFI_SUCCESS;
}
