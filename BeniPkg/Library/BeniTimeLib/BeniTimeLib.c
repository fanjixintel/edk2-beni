/**
*  @Package     : BeniPkg
*  @FileName    : BeniTimeLib.c
*  @Date        : 20220102
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is a library for time-related functinons.
*
*  @History:
*    20220102: Initialize.
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

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BeniTimeLib.h>

/**
  Change EFI time to Linux timestamp.

  @param[in]  EfiTime               The EFI time.
  @param[out] TimeStamp             The Linux timestamp.

  @retval  EFI_SUCCESS              Changed.
  @retval  Other                    Change failed.

**/
EFI_STATUS
EFIAPI
EfiTimeToTimestamp (
  IN  EFI_TIME                      *EfiTime,
  OUT UINT32                        *TimeStamp
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Change Linux timestamp to EFI time.

  @param[in]  TimeStamp             The Linux timestamp.
  @param[out] EfiTime               The EFI time.

  @retval  EFI_SUCCESS              Changed.
  @retval  Other                    Change failed.

**/
EFI_STATUS
EFIAPI
TimestampToEfiTime (
  IN  UINT32                        *TimeStamp,
  OUT EFI_TIME                      *EfiTime
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Print EFI time.

  @param[in]  EfiTime               The EFI time.
  @param[in]  Output                Display in debug port or in console.

  @retval  NA

**/
VOID
EFIAPI
PrintEfiTime (
  IN  EFI_TIME                      *EfiTime,
  IN  BOOLEAN                       Output
  )
{
  if (Output) {
    Print (L"Not Available.\n");
  } else {
    DEBUG ((EFI_D_ERROR, "Not Available.\n"));
  }
}
