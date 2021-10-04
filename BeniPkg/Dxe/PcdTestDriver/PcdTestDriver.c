/**
*  @Package     : BeniPkg
*  @FileName    : PcdTestDriver.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This driver is used to do some tests about PCDs.
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
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BeniDebugLib.h>

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle this driver.
  @param[in]  SystemTable           Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
PcdTestDriverEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  BENI_MODULE_START

  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestVar1      : 0x%x\n", PcdGet8 (PcdTestVar1)));
  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestVar2      : 0x%x\n", PcdGet16 (PcdTestVar2)));
  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestVar3      : 0x%x\n", PcdGet32 (PcdTestVar3)));
  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestVar4      : 0x%x\n", PcdGet64 (PcdTestVar4)));
  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestFeatureVar: %d\n",    FeaturePcdGet (PcdTestFeatureVar)));
  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestBooleanVar: %d\n",    PcdGetBool (PcdTestFeatureVar)));
  DEBUG ((EFI_D_ERROR, "[BENI]PcdTestVersion   : 0x%x\n",  PcdGet32 (PcdTestVersion)));
  DEBUG ((EFI_D_ERROR, "[BENI]PcdPatchableVar  : 0x%x\n",  PcdGet32 (PcdPatchableVar)));

  BENI_MODULE_END

  return EFI_SUCCESS;
}