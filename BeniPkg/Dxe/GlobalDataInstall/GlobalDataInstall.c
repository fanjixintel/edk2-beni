/**
*  @Package     : BeniPkg
*  @FileName    : GlobalDataInstall.c
*  @Date        : 20180616
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    We have global data which can be used in UEFI booting.
*
*  @History:
*    20180616: Initialize.
*    20180811: Add protocol way to store global data.
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

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BeniDebugLib.h>

#include <Protocol/BeniDataProtocol.h>

UINT8 gProtocolData[] = {
  0xAA,
  0xBB,
  0xCC,
  0xDD,
  0xEE,
  0xFF,
};

/**
  Using variable to transfer global data.

  @param  NA

  @retval  EFI_SUCCESS              Set data successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
VariableMethod (
  VOID
  )
{
  EFI_STATUS    Status;
  VOID          *Buffer;

  Buffer = AllocateZeroPool (BENI_DATA_SIZE);
  if (NULL == Buffer) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }
  *((UINT32 *)Buffer) = BENI_DATA_MAGIC;

  Status = gRT->SetVariable (
                  BENI_DATA_NAME,
                  &gBeniGlobalDataGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  BENI_DATA_SIZE,
                  Buffer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]SetVariable failed. - %r\n", Status));
  }

  //
  // After set, we can free memory safely.
  //
  FreePool (Buffer);

  return Status;
}

/**
  Get the BENI-defined data for further use.

  @param[in]  This                  A pointer to the EFI_BENI_DATA_PROTOCPL instance.
  @param[out] Data                  The BENI-defined data.

  @retval  EFI_SUCCESS              Get the data successfully.
  @retval  Others                   Failed to get the data.

**/
EFI_STATUS
EFIAPI
GetBeniData (
  IN  BENI_DATA_PROTOCOL            *This,
  OUT UINT8                         **Data
  )
{
  UINT8    *Buffer;

  Buffer = AllocateZeroPool (BENI_DATA_SIZE);
  if (NULL == Buffer) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }

  *Data = Buffer;
  CopyMem (Buffer, gProtocolData, sizeof (gProtocolData));

  return EFI_SUCCESS;
}

/**
  Get the length of BENI-defined data for further use.

  @param[in]  This                  A pointer to the EFI_BENI_DATA_PROTOCPL instance.

  @retval  UINTN                    The length of BENI-defined data for further use.

**/
UINTN
EFIAPI
GetDataLength (
  IN  BENI_DATA_PROTOCOL            *This
  )
{
  return sizeof (gProtocolData);
}

/**
  Using protocol to transfer global data.

  @param[in]  ImageHandle           Image handle for this driver.

  @retval  EFI_SUCCESS              Set data successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ProtocolMethod (
  IN  EFI_HANDLE                    ImageHandle
  )
{
  EFI_STATUS              Status;
  BENI_DATA_PROTOCOL      *ProtocolPtr;

  ProtocolPtr = AllocatePool (sizeof (BENI_DATA_PROTOCOL));
  if (NULL == ProtocolPtr) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }

  ProtocolPtr->GetBeniData   = GetBeniData;
  ProtocolPtr->GetDataLength = GetDataLength;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gBeniDataProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  ProtocolPtr
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]InstallProtocolInterface failed. - %r\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle for this driver.
  @param[in]   SystemTable          Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
GlobalDataInstallEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS    Status;

  BENI_MODULE_START

  Status = VariableMethod ();
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]VariableMethod failed. - %r\n", Status));
  }

  Status = ProtocolMethod (ImageHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]ProtocolMethod failed. - %r\n", Status));
  }

  BENI_MODULE_END

  return EFI_SUCCESS;
}
