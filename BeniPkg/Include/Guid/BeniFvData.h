/**
*  @Package     : BeniPkg
*  @FileName    : BeniFvData.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    The GUID of BENI FV data.
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

#ifndef __BENI_FV_DATA_GUID_H__
#define __BENI_FV_DATA_GUID_H__

#include <Uefi.h>

//
// {64DB4374-88D6-474A-A30F-C4910FBEA439}
//
#define BENI_FV_DATA_GUID \
  { \
    0x64db4374, 0x88d6, 0x474a, { 0xa3, 0x0f, 0xc4, 0x91, 0x0f, 0xbe, 0xa4, 0x39 } \
  }

extern EFI_GUID gBeniFvDataGuid;

#endif // __BENI_FV_DATA_GUID_H__