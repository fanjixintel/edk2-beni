/**
*  @Package     : BeniPkg
*  @FileName    : BeniGlobalDataTestLib.h
*  @Date        : 20180616
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is used to print the address of global data.
*
*  @History:
*    20180616: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __BENI_GLOBAL_DATA_TEST_LIB__
#define __BENI_GLOBAL_DATA_TEST_LIB__

/**
  Print the address of the global variables.

  @param  NA

  @retval  EFI_SUCCESS              Executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
PrintGlobalVar (
  VOID
  );

/**
  Print the address of the global variables.
  Version 2.

  @param  NA

  @retval  EFI_SUCCESS              Executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
PrintGlobalVar2 (
  VOID
  );

#endif // __BENI_GLOBAL_DATA_TEST_LIB__
