/**
*  @Package     : BeniPkg
*  @FileName    : BeniSetupHii.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This driver add a page to UEFI Setup.
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

#ifndef __BENI_SETUP_DXE_HII_H__
#define __BENI_SETUP_DXE_HII_H__

#include <Uefi.h>

//
// {D8F904A6-5531-47B6-A35B-B38393053701}
//
#define BENI_SETUP_FORMSET_GUID \
  { \
    0xd8f904a6, 0x5531, 0x47b6, { 0xa3, 0x5b, 0xb3, 0x83, 0x93, 0x5, 0x37, 0x1 } \
  }

//
// {F9E3E0E5-2FB5-4981-B4E4-4FFF253409BC}
//
#define BENI_SETUP_INVENTORY_GUID \
  { \
    0xf9e3e0e5, 0x2fb5, 0x4981, { 0xb4, 0xe4, 0x4f, 0xff, 0x25, 0x34, 0x9, 0xbc } \
  }

//
// {CAA52086-F67C-4252-A54A-CBF1FBCC10DC}
//
#define BENI_IFR_REFRESH_ID_OP_GUID \
  { \
    0xcaa52086, 0xf67c, 0x4252, { 0xa5, 0x4a, 0xcb, 0xf1, 0xfb, 0xcc, 0x10, 0xdc } \
  }

extern EFI_GUID gBeniSetupFormSetGuid;
extern EFI_GUID gBeniSetupInventoryGuid;
extern EFI_GUID gBeniIfrRefreshIdOpGuid;

#endif // __BENI_SETUP_DXE_HII_H__
