/**
*  @Package     : BeniPkg
*  @FileName    : BeniPkgTokenSpace.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    The GUID of BENI package PCD.
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

#ifndef __BENI_PKG_TOKEN_SPACE_GUID_H__
#define __BENI_PKG_TOKEN_SPACE_GUID_H__

#include <Uefi.h>

//
// {6BB4BEC8-23D8-40AF-8F24-99E7AC8601FD}
//
#define BENI_PKG_TOKEN_SPACE_GUID \
  { \
    0x6bb4bec8, 0x23d8, 0x40af, { 0x8f, 0x24, 0x99, 0xe7, 0xac, 0x86, 0x01, 0xfd } \
  }

extern EFI_GUID gBeniPkgTokenSpaceGuid;

#endif // __BENI_PKG_TOKEN_SPACE_GUID_H__