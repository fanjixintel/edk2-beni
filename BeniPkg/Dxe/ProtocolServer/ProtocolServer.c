/**
*  @Package     : BeniPkg
*  @FileName    : ProtocolServer.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    Install BENI_HELLO_WORLD_PROTOCOL.
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

#include <Protocol/BeniHelloWorldProtocol.h>

/**
  Print "Hello Wrold" in serail console.

  @param[in]  This                  A pointer to the BENI_HELLO_WORLD_PROTOCOL instance.

  @retval  EFI_SUCCESS              Always return EFI_SUCCESS after print.

**/
EFI_STATUS
EFIAPI
Hello (
  IN  BENI_HELLO_WORLD_PROTOCOL     *This
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Hello World\n"));

  return EFI_SUCCESS;
}

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle for this driver.
  @param[in]  SystemTable           Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ProtocolServerEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS                   Status;
  BENI_HELLO_WORLD_PROTOCOL    *Protocol;

  Protocol = AllocatePool (sizeof (BENI_HELLO_WORLD_PROTOCOL));
  if (NULL == Protocol) {
    DEBUG ((EFI_D_ERROR, "[BENI][%a][%d]: Out of resource.", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }

  Protocol->Revision = BENI_HELLO_WORLD_PROTOCOL_REVISION;
  Protocol->Hello    = Hello;

  Status = gBS->InstallProtocolInterface (
                    &ImageHandle,
                    &gBeniHelloWorldProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    Protocol
                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Install BENI_HELLO_WORLD_PROTOCOL failed. - %r\n", Status));
    FreePool (Protocol);
    return Status;
  }

  return EFI_SUCCESS;
}