/**
*  @Package     : BeniPkg
*  @FileName    : MemTestSupportLib.h
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
  Memory Test support library class interface

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

#ifndef __MEM_TEST_SUPPORT_LIB_H__
#define __MEM_TEST_SUPPORT_LIB_H__

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemTestUiLib.h>
#include <Library/MemTestRangesLib.h>

/**
  Memory range test method.

  @param[in]  Start                 Start address to test.
  @param[in]  Length                Memory test length.
  @param[in]  PassNumber            Number of pass.
  @param[in]  Context               Test data.

  @retval  EFI_SUCCESS              Operation complete.
  @retval  Others                   Operation failed.

**/
typedef
EFI_STATUS
(EFIAPI *TEST_MEM_RANGE)(
  IN  EFI_PHYSICAL_ADDRESS          Start,
  IN  UINT64                        Length,
  IN  UINTN                         PassNumber,
  IN  VOID                          *Context
  );

/**
  Run memory range test.

  @param[in]  Context               Test data.

  @retval  EFI_SUCCESS              Operation complete.
  @retval  Others                   Operation failed.

**/
typedef
EFI_STATUS
(EFIAPI *RUN_MEM_TEST)(
  IN  VOID                          *Context
  );

/**
  Install memory range test method.

  @param[in]  Name                  The name of memory range test.
  @param[in]  TestRangeFunction     Memory test process.
  @param[in]  NumberOfPasses        Number of pass.
  @param[in]  Context               Test data.

  @retval  EFI_SUCCESS              Operation complete.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
MtSupportInstallMemoryRangeTest (
  IN  CHAR16                        *Name,
  IN  TEST_MEM_RANGE                TestRangeFunction,
  IN  UINTN                         NumberOfPasses,
  IN  VOID                          *Context
  );

/**
  Install memory test method.

  @param[in]  Name                  The name of memory range test.
  @param[in]  TestRangeFunction     Memory test process.
  @param[in]  Context               Test data.

  @retval  EFI_SUCCESS              Operation complete.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
MtSupportInstallMemoryTest (
  IN  CHAR16                        *Name,
  IN  RUN_MEM_TEST                  MemTestFunction,
  IN  VOID                          *Context
  );

/***
  Executes a WBINVD instruction.

  @param  NA

  @retval  EFI_SUCCESS              Memory test done.
  @retval  Other                    Error happened.

***/
VOID
EFIAPI
MtWbinvd (
  VOID
  );

/***
  Abort memory test.

  @param  NA

  @retval  NA

***/
VOID
EFIAPI
MtSupportAbortTesting (
  VOID
  );

#endif // __MEM_TEST_SUPPORT_LIB_H__
