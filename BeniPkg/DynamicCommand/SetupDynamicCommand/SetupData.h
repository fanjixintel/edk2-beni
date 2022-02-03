/**
*  @Package     : BeniPkg
*  @FileName    : SetupData.h
*  @Date        : 20220105
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for setup test.
*
*  @History:
*    20220105: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __SETUP_DATA_H__
#define __SETUP_DATA_H__

#include <Guid/BeniSetupHii.h>

//
// This is used in name of efivarstore.
//
#define BENI_SETUP_DATA_VAR_NAME    L"BeniSetupData"

#define FRONT_PAGE_FORM_ID          0x1000

typedef struct {
  UINT8    Data1;
  UINT8    Data2;
  UINT8    Rsvd1[2];
} BENI_SETUP_DATA;

#endif // __SETUP_DATA_H__
