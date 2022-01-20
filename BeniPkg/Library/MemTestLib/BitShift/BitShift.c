/**
*  @Package     : BeniPkg
*  @FileName    : BitShift.c
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
/**

  Bit Shift Memory Test - Provides a memory test that produces walking bits
  with inversions...

  Copyright (c) 2009, Intel Corporation
  Copyright (c) 2009, Erik Bjorge
  All rights reserved.

  This program and the accompanying materials are licensed and made
  available under the terms and conditions of the BSD License which
  accompanies this distribution.  The full text of the license may
  be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
  EXPRESS OR IMPLIED.

**/

#include "BitShift.h"

BOOLEAN
VerifyMemory (
    IN EFI_PHYSICAL_ADDRESS     Start,
    IN UINT64                   Length,
    IN UINT64                   Pattern
    );

STATIC PATTERN_TEST_DATA mBasePatterns[] = {
    {CREATE_PATTERN_UINT8(1),  L"Bit Shift: (BYTE)"},
    {CREATE_PATTERN_UINT16(1), L"Bit Shift: (WORD)"},
    {CREATE_PATTERN_UINT32(1), L"Bit Shift: (DWORD)"},
    {CREATE_PATTERN_UINT64(1), L"Bit Shift: (QWORD)"}
    };
STATIC UINT8 mBasePatternsCount = sizeof(mBasePatterns)/sizeof(PATTERN_TEST_DATA);

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
RunBitShiftMemTest (
  IN  EFI_PHYSICAL_ADDRESS          Start,
  IN  UINT64                        Length,
  IN  UINTN                         PassNumber,
  IN  VOID                          *Context
  )
{
  UINT64                  End = 0;
  UINT64                  Pattern = 0;
  BOOLEAN                 Verify = TRUE;
  BOOLEAN                 TestDone = FALSE;

  //
  // Display range being tested
  //
  End = Start + Length;
  DEBUG ((DEBUG_INFO, "Testing Memory Range: 0x%016lx - 0x%016lx\n", Start, End));

  Pattern = *((UINT64*) Context);
  TestDone = FALSE;

  while (!TestDone) {
      //
      // Check to see if the test is complete
      //
      if (Pattern & BIT(63)) {
          TestDone = TRUE;
      }

      //
      // Perform normal pattern test
      //
      DEBUG ((DEBUG_INFO, "Pattern: 0x%016lx\n", Pattern));
      SetMem64 ((VOID*)(UINTN)Start, Length, Pattern);
      MtWbinvd ();
      if (VerifyMemory (Start, Length, Pattern)) {
          Verify = FALSE;
      }

      //
      // Perform inversion pattern test
      //
      Pattern = ~Pattern;
      DEBUG ((DEBUG_INFO, "Pattern: 0x%016lx\n", Pattern));
      SetMem64 ((VOID*)(UINTN)Start, Length, Pattern);
      MtWbinvd ();
      if (VerifyMemory (Start, Length, Pattern)) {
          Verify = FALSE;
      }

      //
      // Invert again and shift for next iteration
      //
      Pattern = ~Pattern;
      Pattern <<= 1;
  }

  return EFI_SUCCESS;
}

/**
  Memory range test method.

  @param[in]  Start                 Start address to test.
  @param[in]  Length                Memory test length.
  @param[in]  Pattern               Memory test pattern.

  @retval  TRUE                     Verify OK.
  @retval  FALSE                    Verify not OK.

**/
BOOLEAN
VerifyMemory (
  IN  EFI_PHYSICAL_ADDRESS          Start,
  IN  UINT64                        Length,
  IN  UINT64                        Pattern
  )
{
  EFI_PHYSICAL_ADDRESS        *End;
  EFI_PHYSICAL_ADDRESS        *Check;
  BOOLEAN                     Failed = FALSE;

  //
  // Check that the memory regions match the fill pattern
  //
  End = (UINT64*)(UINTN) (Start + Length);
  Check = (EFI_PHYSICAL_ADDRESS*)(UINTN) Start;
  for (; Check < End; Check++) {
      if (*Check != Pattern) {
          MtUiPrint (L"    Failed at 0x%016lx\n", Check);
          Failed = TRUE;
      }
  }

  return (!Failed);
}

/**
  Registers test function for the Moving Bits test.

  @param  NA

  @retval  EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BitShiftInit (
  VOID
  )
{
  UINT8   Index;

  for (Index = 0; Index < mBasePatternsCount; Index++) {
    MtSupportInstallMemoryRangeTest (
      mBasePatterns[Index].Name,
      RunBitShiftMemTest,
      1,
      (VOID*)&mBasePatterns[Index].Pattern
      );
  }

  return EFI_SUCCESS;
}
