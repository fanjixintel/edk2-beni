/**
*  @Package     : BeniPkg
*  @FileName    : MemTestUiLib.h
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
  Memory Test UI library class interface

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

#ifndef __MEM_TEST_UI_LIB_H__
#define __MEM_TEST_UI_LIB_H__

#include <Uefi.h>

#include <Library/UefiLib.h>

/**
  Sets the name of test.

  @param[in]  TestName              The name of the test.

  @retval  EFI_SUCCESS              The entry point is executed successfully.

**/
VOID
EFIAPI
MtUiSetTestName (
  IN  CHAR16                        *TestName
  );

/**
  This function behaves the same at the UefiLib Print function,
  but the location and presentation may be altered by the UiLib
  instance.

  @param[in]  Format                Null-terminated Unicode format string.
  @param[in]  ...                   Variable argument list whose contents are accessed
                                    based on the format string specified by Format.

  @return  UINTN                    Number of Unicode characters printed to ConOut.

**/
UINTN
EFIAPI
MtUiPrint (
  IN  CONST CHAR16                  *Format,
  ...
  );

/**
  Indicates the total size of a pass through the memory test.

  @param[in]  Total                 The total size of a pass through the memory test.

  @retval  NA

**/
VOID
EFIAPI
MtUiSetProgressTotal (
  IN  UINT64                        Total
  );

/**
  Allows the memory test to indicate progress to the UI library.

  @param[in]  Progress              The memory test progress.

  @retval  NA

**/
VOID
EFIAPI
MtUiUpdateProgress (
  IN  UINT64                        Progress
  );

#endif // __MEM_TEST_UI_LIB_H__
