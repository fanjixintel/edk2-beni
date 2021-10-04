/**
*  @Package     : BeniPkg
*  @FileName    : BeniDebugLib.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This library is used for BENI-defined DEBUG functions.
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

#ifndef __BENI_DEBUG_LIBRARY_H__
#define __BENI_DEBUG_LIBRARY_H__

#include <Library/DebugLib.h>

#define BENI_MODULE_START \
  DEBUG ((EFI_D_ERROR, "[BENI]%a start...\n", __FUNCTION__));

#define BENI_MODULE_END \
  DEBUG ((EFI_D_ERROR, "[BENI]%a end...\n", __FUNCTION__));

#endif // __BENI_DEBUG_LIBRARY_H__