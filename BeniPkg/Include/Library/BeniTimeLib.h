/**
*  @Package     : BeniPkg
*  @FileName    : BeniTimeLib.h
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

#ifndef __BENI_TIME_LIBRARY_H__
#define __BENI_TIME_LIBRARY_H__

#include <Uefi.h>

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
  );

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
  );

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
  );

#endif // __BENI_TIME_LIBRARY_H__
