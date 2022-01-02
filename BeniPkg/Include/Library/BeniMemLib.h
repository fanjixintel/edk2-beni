/**
*  @Package     : BeniPkg
*  @FileName    : BeniMemLib.h
*  @Date        : 20220102
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This library is used for memory-related functions.
*
*  @History:
*    20220102: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __BENI_MEM_LIB_H__
#define __BENI_MEM_LIB_H__

#include <Library/MemoryAllocationLib.h>

#define BENI_FREE_NON_NULL(Pointer)   \
  do {                                \
    if (NULL != (Pointer)) {          \
      FreePool((Pointer));            \
      (Pointer) = NULL;               \
    }                                 \
  } while(FALSE)

#endif // __BENI_MEM_LIB_H__
