/**
*  @Package     : BeniPkg
*  @FileName    : ProtocolConsumer.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    Use BENI_HELLO_WORLD_PROTOCOL.
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
#include <Library/DebugLib.h>
#include <Library/BeniDebugLib.h>

#include <Protocol/BeniHelloWorldProtocol.h>

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle for this driver.
  @param[in]   SystemTable          Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ProtocolConsumerEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS                   Status;
  BENI_HELLO_WORLD_PROTOCOL    *Protocol;

  BENI_MODULE_START

  Status = gBS->LocateProtocol (&gBeniHelloWorldProtocolGuid, NULL, (VOID **)&Protocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Locate protocol failed. - %r\n", Status));
    return Status;
  }

  //
  // Print to see the address of installed protocol.
  //
  DEBUG ((EFI_D_ERROR, "[BENI]Protocol address: 0x%p\n", Protocol));
  DEBUG ((EFI_D_ERROR, "[BENI]Protocol revision: 0x%016x\n", Protocol->Revision));

  Status = Protocol->Hello (Protocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Protocol->Hello failed. - %r\n", Status));
    return Status;
  }

  BENI_MODULE_END

  return EFI_SUCCESS;
}