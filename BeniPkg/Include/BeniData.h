/**
*  @Package     : BeniPkg
*  @FileName    : BeniData.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This file describe the BENI GUID and related data.
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

#ifndef __BENI_DATA_H__
#define __BENI_DATA_H__

#include <Uefi.h>

#define BENI_DATA_NAME              L"BeniData"
#define BENI_DATA_MAGIC             0xA55A5AA5
#define BENI_DATA_SIZE              128

//
// {DB56E93F-C5EF-4888-8006-F64DBCBBF755}
//
#define BENI_DATA_GUID \
  { \
    0xdb56e93f, 0xc5ef, 0x4888, { 0x80, 0x06, 0xf6, 0x4d, 0xbc, 0xbb, 0xf7, 0x55 } \
  }

extern EFI_GUID gBeniGlobalDataGuid;

#endif // __BENI_DATA_H__
