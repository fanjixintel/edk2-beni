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

;------------------------------------------------------------------------------
; VOID
; EFIAPI
; AsmLoopInfi (
;   VOID
;   );
;------------------------------------------------------------------------------
global ASM_PFX(AsmLoopInfi)
ASM_PFX(AsmLoopInfi):
  jmp $
  ret

;------------------------------------------------------------------------------
; VOID
; EFIAPI
; AsmSerialIo (
;   UINT8                             Char
;   );
;------------------------------------------------------------------------------
global ASM_PFX(AsmSerialIo)
ASM_PFX(AsmSerialIo):

  mov dx, 03f3h
  mov eax, 0
loop:
  in  al, dx
  bt  eax, 5
  jnc loop      ; Wait until ready

  mov dx, 03f8h
  mov ax, cx    ; cx is the input parameter
  out dx, ax    ; Output the character
  ret
