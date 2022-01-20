/**
*  @Package     : BeniPkg
*  @FileName    : MemTest.h
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
  MemTest EFI Shell Application.

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

#ifndef __MEM_TEST_H__
#define __MEM_TEST_H__

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/MemTestUiLib.h>
#include <Library/MemTestRangesLib.h>

/***
  Run all memor test.

  @param  NA

  @retval  EFI_SUCCESS              Memory test done.
  @retval  Other                    Error happened.

***/
EFI_STATUS
EFIAPI
MtSupportRunAllTests (
  VOID
  );

#endif // __MEM_TEST_H__
