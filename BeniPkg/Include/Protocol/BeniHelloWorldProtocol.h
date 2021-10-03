/**
*  @Package     : BeniPkg
*  @FileName    : BeniHelloWorldProtocol.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This file describes a protocol for test.
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

#ifndef __BENI_HELLO_WORLD_PROTOCOL_H__
#define __BENI_HELLO_WORLD_PROTOCOL_H__

#include <Uefi.h>

//
// {038F1AF5-1C8D-408F-AB25-03AEB5965D6E}
//
#define BENI_HELLO_WORLD_PROTOCOL_GUID \
  { \
    0x038f1af5, 0x1c8d, 0x408f, { 0xab, 0x25, 0x30, 0xae, 0xb5, 0x96, 0x5d, 0x6e } \
  }

typedef struct _BENI_HELLO_WORLD_PROTOCOL BENI_HELLO_WORLD_PROTOCOL;

/**
  Print "Hello Wrold".

  @param[in]  This                  A pointer to the BENI_HELLO_WORLD_PROTOCOL instance.

  @retval  EFI_SUCCESS              Always return EFI_SUCCESS after print.

**/
typedef
EFI_STATUS
(EFIAPI *HELLO) (
  IN  BENI_HELLO_WORLD_PROTOCOL     *This
  );

//
// Revision defined.
//
#define BENI_HELLO_WORLD_PROTOCOL_REVISION    0x00010000

struct _BENI_HELLO_WORLD_PROTOCOL {
  //
  // The revision to which the block IO interface adheres. All future
  // revisions must be backwards compatible. If a future version is not
  // back wards compatible, it is not the same GUID.
  //
  UINT64                            Revision;

  HELLO                             Hello;
};

extern EFI_GUID gBeniHelloWorldProtocolGuid;

#endif // __BENI_HELLO_WORLD_PROTOCOL_H__