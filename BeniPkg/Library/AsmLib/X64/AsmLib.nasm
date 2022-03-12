;;
;  @Package     : BeniPkg
;  @FileName    : AsmLib.nasm
;  @Date        : 20220313
;  @Author      : Jiangwei
;  @Version     : 1.0
;  @Description :
;    This is assembly code for test.
;
;  @History:
;    20220313: Initialize.
;
;  This program and the accompanying materials
;  are licensed and made available under the terms and conditions of the BSD License
;  which accompanies this distribution. The full text of the license may be found at
;  http://opensource.org/licenses/bsd-license.php
;
;  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
;  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;;

  SECTION .text

;------------------------------------------------------------------------------
; VOID
; EFIAPI
; AsmNop (
;   VOID
;   );
;------------------------------------------------------------------------------
global ASM_PFX(AsmNop)
ASM_PFX(AsmNop):
  nop
  ret
