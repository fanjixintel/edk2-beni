/**
*  @Package     : BeniPkg
*  @FileName    : Disk.h
*  @Date        : 20211128
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for disk test.
*
*  @History:
*    20211128: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __DISK_H__
#define __DISK_H__

#include <Uefi.h>

#include <IndustryStandard/Mbr.h>

#include <Guid/Gpt.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/HiiPackageList.h>
#include <Protocol/ShellDynamicCommand.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>

//
// Extract UINT32 from char array.
//
#define UNPACK_UINT32(a) (UINT32)( (((UINT8 *) a)[0] <<  0) |    \
                                   (((UINT8 *) a)[1] <<  8) |    \
                                   (((UINT8 *) a)[2] << 16) |    \
                                   (((UINT8 *) a)[3] << 24) )
//
// OS Type.
//
#define MBR_UEFI_SYSTEM_PARTITION        L"UEFI System"
#define MBR_GPT_PROTECTIVE_PARTITION     L"GPT Protective"
#define MBR_UNSPECIFIED_PARTITION        L"Unspecified"
#define GPT_UNUSED_PARTITION             L"Unused"
#define GPT_LEGACY_MBR_PARTITION         L"Legacy"
#define GPT_SYSTEM_PARTITION             L"System"
//
// Boot Indicator.
//
#define MBR_BOOTABLE                     L"Bootable"
#define MBR_NOT_BOOTABLE                 L"Not Bootable"

//
// GPT Partition Entry Status.
//
typedef struct {
  BOOLEAN OutOfRange;
  BOOLEAN Overlap;
  BOOLEAN OsSpecific;
} EFI_PARTITION_ENTRY_STATUS;

/**
  Retrieve HII package list from ImageHandle and publish to HII database.

  @param[in]  ImageHandle           The image handle of the process.

  @return  EFI_HII_HANDLE           The HII handle.

**/
EFI_HII_HANDLE
InitializeHiiPackage (
  IN  EFI_HANDLE                    ImageHandle
  );

/**
  Function for 'disk' command.

  @param[in]  ImageHandle           The image handle.
  @param[in]  SystemTable           The system table.

  @retval  SHELL_SUCCESS            Command completed successfully.
  @retval  SHELL_INVALID_PARAMETER  Command usage error.
  @retval  SHELL_ABORTED            The user aborts the operation.
  @retval  Value                    Unknown error.

**/
SHELL_STATUS
RunDisk (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  );

//
// Used for shell display.
//
extern EFI_HII_HANDLE mDiskHiiHandle;

#endif // __DISK_H__