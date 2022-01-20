/**
*  @Package     : BeniPkg
*  @FileName    : MemTestRangesLib.h
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
  Memory Test Ranges library class interface

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

#ifndef __MEM_TEST_RANGES_LIB_H__
#define __MEM_TEST_RANGES_LIB_H__

#include <Uefi.h>

#include <Library/UefiLib.h>

/**
  Returns the total size of all test memory ranges.

  @param  NA

  @return  UINT64                   The total size of all test memory ranges

**/
UINT64
MtRangesGetTotalSize (
  VOID
  );

/**
  Gets the next test memory range.

  @param[in,out] Key                To retrieve the first range, set Key to 0 before calling.
                                    To retrieve the next range, pass in the previous Key.
  @param[out]    Start              Start of the next memory range.
  @param[out]    Length             Length of the next memory range.

  @retval  EFI_SUCCESS              The next memory range was returned.
  @retval  EFI_NOT_FOUND            Indicates all ranges have been returned.

**/
EFI_STATUS
MtRangesGetNextRange (
  IN OUT UINTN                      *Key,
  OUT    EFI_PHYSICAL_ADDRESS       *Start,
  OUT    UINT64                     *Length
  );

/**
  Lock memory range.

  @param[in]  Start                 Start of the memory range.
  @param[in]  Length                Length of the memory range.

  @retval  EFI_SUCCESS              The range was locked.
  @retval  EFI_ACCESS_DENIED        The range could not be locked.

**/
EFI_STATUS
EFIAPI
MtRangesLockRange (
  IN  EFI_PHYSICAL_ADDRESS          Start,
  IN  UINT64                        Length
  );

/**
  Unlocks a memory range.

  @param[in]  Start                 Start of the memory range.
  @param[in]  Length                Length of the memory range.

  @retval  EFI_SUCCESS              The range was unlocked.
  @retval  EFI_INVALID_PARAMETER    The range could not be unlocked.

**/
VOID
EFIAPI
MtRangesUnlockRange (
  IN  EFI_PHYSICAL_ADDRESS          Start,
  IN  UINT64                        Length
  );

/**
  Initializes the memory test ranges.

  @param  NA

  @retval  EFI_SUCCESS              Construction complete.
  @retval  Ohters                   Construction failed.

**/
EFI_STATUS
MtRangesConstructor (
  VOID
  );

/**
  Decontructs the memory test ranges data structures.

  @param  NA

  @retval  EFI_SUCCESS              Deconstruction complete.
  @retval  Ohters                   Deconstruction failed.

**/
EFI_STATUS
MtRangesDeconstructor (
  VOID
  );

#endif // __MEM_TEST_RANGES_LIB_H__
