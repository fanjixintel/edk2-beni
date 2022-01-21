/**
*  @Package     : BeniPkg
*  @FileName    : Address.c
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

  Address Decode Memory Test - Validates that the address being tested decodes
  to the correct location.

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

#include "Address.h"

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
RunAddressMemTest (
  IN  EFI_PHYSICAL_ADDRESS          Start,
  IN  UINT64                        Length,
  IN  UINTN                         PassNumber,
  IN  VOID                          *Context
  )
{
  UINT64                  End = 0;
  UINT64                  TestLength = 0;
  EFI_PHYSICAL_ADDRESS    TestLocation;
  UINT64                  Index;

  //
  // Display range being tested
  //
  End = Start + Length;
  DEBUG ((DEBUG_INFO, "Testing Memory Range: 0x%016lx - 0x%016lx\n", Start, End));

  //
  // Set initial values
  //
  TestLength = Length / sizeof(TestLocation);
  TestLocation = Start;

  //
  // Perform address fill
  //
  for (Index = 0; Index < TestLength; Index++) {
    if ((((UINT32)Index) % 0x100000) == 0) {
      DEBUG ((DEBUG_INFO, "Writing 0x%016lx\n", TestLocation));
    }
    *((UINT64*)(UINTN)TestLocation) = TestLocation;
    TestLocation += sizeof(TestLocation);
  }

  //
  // Force the cache to be flushed.
  //
  MtWbinvd ();

  //
  // Reset values for verification.
  //
  TestLength = Length / sizeof(TestLocation);
  TestLocation = Start;

  //
  // Perform address verification.
  //
  for (Index = 0; Index < TestLength; Index++) {
    if ((((UINT32)Index) % 0x100000) == 0) {
      DEBUG ((DEBUG_INFO, "Validating 0x%016lx\n", TestLocation));
    }
    if (*((UINT64*)(UINTN)TestLocation) != TestLocation) {
      MtUiPrint (L"  Failed At 0x%016lx\n", TestLocation);
    }
    TestLocation += sizeof(TestLocation);
  }

  return EFI_SUCCESS;
}

/**
  Registers test function for the Moving Bits test.

  @param  NA

  @retval  EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
AddressInit (
  VOID
  )
{
  MtSupportInstallMemoryRangeTest (
    L"Address Decode Test",
    RunAddressMemTest,
    1,
    NULL
    );

  return EFI_SUCCESS;
}
