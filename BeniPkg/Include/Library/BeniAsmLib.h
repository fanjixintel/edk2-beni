/**
*  @Package     : BeniPkg
*  @FileName    : BeniAsmLib.h
*  @Date        : 20220313
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is a library for assembly code test.
*
*  @History:
*    20220313: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __BENI_ASSEMBLY_H__
#define __BENI_ASSEMBLY_H__

/**
  Code for nothing.

  @param  NA

  @retval  NA

**/
VOID
EFIAPI
AsmNop (
  VOID
  );

/**
  while(1).

  @param  NA

  @retval  NA

**/
VOID
EFIAPI
AsmLoopInfi (
  VOID
  );

/**
  Output one character.

  @param[in]  Char                  The character to be outputted.

  @retval  NA

**/
VOID
EFIAPI
AsmSerialIo (
  UINT8                             Char
  );

#endif // __BENI_ASSEMBLY_H__
