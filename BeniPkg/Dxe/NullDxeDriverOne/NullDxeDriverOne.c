/**
*  @Package     : BeniPkg
*  @FileName    : NullDxeDriverOne.c
*  @Date        : 20180616
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is used to test global data in library.
*    This is a bad example.
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

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/BeniDebugLib.h>
#include <Library/BeniGlobalDataTestLib.h>

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle of this driver.
  @param[in]  SystemTable           Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
NullDxeDriverOneEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  BENI_MODULE_START

  DEBUG ((EFI_D_ERROR, "[BENI]SystemTable: 0x%p\n", SystemTable));
  DEBUG ((EFI_D_ERROR, "[BENI]gBS        : 0x%p\n", gBS));
  PrintGlobalVar ();
  PrintGlobalVar2 ();

  BENI_MODULE_END

  return EFI_SUCCESS;
}
