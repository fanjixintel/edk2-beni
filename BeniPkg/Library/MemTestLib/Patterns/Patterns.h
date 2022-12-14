/**
*  @Package     : BeniPkg
*  @FileName    : Patterns.h
*  @Date        : 20220121
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is for memory test.
*
*  @History:
*    20220121: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
/** @file
  Patterns Memory Tests

  Copyright (c) 2006 - 2009, Intel Corporation
  Copyright (c) 2009, Jordan Justen
  All rights reserved.

  This program and the accompanying materials are licensed and made
  available under the terms and conditions of the BSD License which
  accompanies this distribution.  The full text of the license may
  be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
  EXPRESS OR IMPLIED.

**/

#ifndef __PATTERNS_H__
#define __PATTERNS_H__

#include <Library/MemTestSupportLib.h>

typedef struct {
  UINT64   Pattern;
  CHAR16   *Name;
} PATTERN_TEST_DATA;

#define PATTERN_MEM_TEST64(Name, Value) \
  { (Value), L"Pattern " Name  }
#define PATTERN_MEM_TEST32(Name, Value) \
  PATTERN_MEM_TEST64 (Name, ((UINT64)(Value) | ((UINT64)(Value)) << 32))
#define PATTERN_MEM_TEST16(Name, Value) \
  PATTERN_MEM_TEST32 (Name, ((Value) | ((UINT32)(Value)) << 16))
#define PATTERN_MEM_TEST8(Name, Value) \
  PATTERN_MEM_TEST16 (Name, ((Value) | ((UINT16)(Value)) << 8))

/**
  Memory range test method.

  @param[in]  Start                 Start address to test.
  @param[in]  Length                Memory test length.
  @param[in]  PassNumber            Number of pass.
  @param[in]  Context               Test data.

  @retval  EFI_SUCCESS              Operation complete.
  @retval  Others                   Operation failed.

**/
STATIC
EFI_STATUS
EFIAPI
RunPatternMemTest (
  IN  EFI_PHYSICAL_ADDRESS          Start,
  IN  UINT64                        Length,
  IN  UINTN                         PassNumber,
  IN  VOID                          *Context
  );

#endif // __PATTERNS_H__
