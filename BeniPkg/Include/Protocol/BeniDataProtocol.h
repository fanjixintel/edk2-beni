/**
*  @Package     : BeniPkg
*  @FileName    : BeniDataProtocol.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This file describe a protocol for transfering data.
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

#ifndef __BENI_DATA_PROTOCOL_H__
#define __BENI_DATA_PROTOCOL_H__

#include <Uefi.h>

//
// {2832CD33-BAF2-4762-BC8A-37D997817144}
//
#define BENI_DATA_PROTOCOL_GUID \
  { \
    0x2832cd33, 0xbaf2, 0x4762, { 0xbc, 0x8a, 0x37, 0xd9, 0x97, 0x81, 0x71, 0x44 } \
  }

typedef struct _BENI_DATA_PROTOCOL  BENI_DATA_PROTOCOL;

/**
  Get the BENI-defined data for further use.

  @param[in]  This                  A pointer to the BENI_DATA_PROTOCOL instance.
  @param[out] Data                  Pointer to the BENI-defined data.

  @retval  EFI_SUCCESS              Get the data successfully.
  @retval  Others                   Failed to get the data.

**/
typedef
EFI_STATUS
(EFIAPI *BENI_GET_DATA) (
  IN  BENI_DATA_PROTOCOL            *This,
  OUT UINT8                         **Data
  );

/**
  Get the length of BENI-defined data for further use.

  @param[in]  This                  A pointer to the BENI_DATA_PROTOCOL instance.

  @retval  UINTN                    The length of BENI-defined data for further use.

**/
typedef
UINTN
(EFIAPI *BENI_GET_DATA_LENGTH) (
  IN  BENI_DATA_PROTOCOL            *This
  );

//
// Revision defined.
//
#define BENI_DATA_PROTOCOL_REVISION 0x00010000

struct _BENI_DATA_PROTOCOL {
  //
  // The revision to which the block IO interface adheres. All future
  // revisions must be backwards compatible. If a future version is not
  // back wards compatible, it is not the same GUID.
  //
  UINT64                            Revision;

  BENI_GET_DATA                     GetBeniData;
  BENI_GET_DATA_LENGTH              GetDataLength;
};

extern EFI_GUID gBeniDataProtocolGuid;

#endif // __BENI_DATA_PROTOCOL_H__