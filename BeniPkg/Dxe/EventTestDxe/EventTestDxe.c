/**
*  @Package     : BeniPkg
*  @FileName    : EventTestDxe.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This driver is used to test several events.
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

#include <Guid/EventGroup.h>

//
// Now we have 6 types of event.
//
typedef enum {
  EVENT_EXIT_BOOT_SERVICES = 0,
  EVENT_VIRTUAL_ADDRESS_CHANGE,
  EVENT_MEMORY_MAP_CHANGE,
  EVENT_READY_TO_BOOT,
  EVENT_DXE_DISPATCH,
  EVENT_END_OF_DXE,
  EVENT_MAX
} BENI_EVENT_ENUM;

//
// We build a struct for all 6 types of event.
//
typedef struct {
  EFI_EVENT     Events[EVENT_MAX];
} BENI_EVENTS;

/**
  Notification function of EXIT_BOOT_SERVICES event.

  @param[in]  Event                 Event whose notification function is being invoked.
  @param[in]  Context               Pointer to the notification function's context.

**/
VOID
EFIAPI
BeniExitBootServices (
  IN  EFI_EVENT                     Event,
  IN  VOID                          *Context
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Event invoked: %a\n", __FUNCTION__));
  gBS->CloseEvent (Event);
}

/**
  Notification function of VIRTUAL_ADDRESS_CHANGE event.

  @param[in]  Event                 Event whose notification function is being invoked.
  @param[in]  Context               Pointer to the notification function's context.

**/
VOID
EFIAPI
BeniVirtualAddressChange (
  IN  EFI_EVENT                     Event,
  IN  VOID                          *Context
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Event invoked: %a\n", __FUNCTION__));
  gBS->CloseEvent (Event);
}

/**
  Notification function of MEMORY_MAP_CHANGE event.

  @param[in]  Event                 Event whose notification function is being invoked.
  @param[in]  Context               Pointer to the notification function's context.

**/
VOID
EFIAPI
BeniMemoryMapChange (
  IN  EFI_EVENT                     Event,
  IN  VOID                          *Context
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Event invoked: %a\n", __FUNCTION__));
  gBS->CloseEvent (Event);
}

/**
  Notification function of READY_TO_BOOT event.

  @param[in]  Event                 Event whose notification function is being invoked.
  @param[in]  Context               Pointer to the notification function's context.

**/
VOID
EFIAPI
BeniReadyToBoot (
  IN  EFI_EVENT                     Event,
  IN  VOID                          *Context
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Event invoked: %a\n", __FUNCTION__));
  gBS->CloseEvent (Event);
}

/**
  Notification function of DXE_DISPATCH event.

  @param[in]  Event                 Event whose notification function is being invoked.
  @param[in]  Context               Pointer to the notification function's context.

**/
VOID
EFIAPI
BeniDxeDispatch (
  IN  EFI_EVENT                     Event,
  IN  VOID                          *Context
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Event invoked: %a\n", __FUNCTION__));
  gBS->CloseEvent (Event);
}

/**
  Notification function of END_OF_DXE event.

  @param[in]  Event                 Event whose notification function is being invoked.
  @param[in]  Context               Pointer to the notification function's context.

**/
VOID
EFIAPI
BeniEndOfDxe (
  IN  EFI_EVENT                     Event,
  IN  VOID                          *Context
  )
{
  DEBUG ((EFI_D_ERROR, "[BENI]Event invoked: %a\n", __FUNCTION__));
  gBS->CloseEvent (Event);
}

/**
  Main entry of the driver.

  @param[in]  ImageHandle           Image handle this driver.
  @param[in]  SystemTable           Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
EventTestDxeEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS    Status;
  BENI_EVENTS   *Events;

  Events = AllocateZeroPool (sizeof (BENI_EVENTS));
  if (NULL == Events) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BeniExitBootServices,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &(Events->Events[EVENT_EXIT_BOOT_SERVICES])
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Create ExitBootServices event failed. - %r\n", Status));
    goto DONE;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BeniVirtualAddressChange,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &(Events->Events[EVENT_VIRTUAL_ADDRESS_CHANGE])
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Create VirtualAddressChange event failed. - %r\n", Status));
    goto DONE;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BeniMemoryMapChange,
                  NULL,
                  &gEfiEventMemoryMapChangeGuid,
                  &(Events->Events[EVENT_MEMORY_MAP_CHANGE])
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Create MemoryMapChange event failed. - %r\n", Status));
    goto DONE;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BeniReadyToBoot,
                  NULL,
                  &gEfiEventReadyToBootGuid,
                  &(Events->Events[EVENT_READY_TO_BOOT])
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Create ReadyToBoot event failed. - %r\n", Status));
    goto DONE;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BeniDxeDispatch,
                  NULL,
                  &gEfiEventDxeDispatchGuid,
                  &(Events->Events[EVENT_DXE_DISPATCH])
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Create DxeDispatch event failed. - %r\n", Status));
    goto DONE;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BeniEndOfDxe,
                  NULL,
                  &gEfiEndOfDxeEventGroupGuid,
                  &(Events->Events[EVENT_END_OF_DXE])
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]Create EndOfDxe event failed. - %r\n", Status));
    goto DONE;
  }

DONE:

  if (NULL != Events) {
    FreePool (Events);
  }

  return Status;
}