/**
*  @Package     : BeniPkg
*  @FileName    : Ext2.c
*  @Date        : 20211213
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for EXT2 file system test.
*
*  @History:
*    20211213: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Fs.h"
#include "Ext2Fs.h"

/**
  Show EXT2 file system.

  @param  NA

  @retval  NA

**/
VOID
EFIAPI
ShowExt2FileSystem (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *Handles;
  UINTN                             Count;
  UINTN                             Index;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_DISK_IO_PROTOCOL              *DiskIo;
  EFI_DISK_IO2_PROTOCOL             *DiskIo2;
  EFI_HANDLE                        FsHandle;

  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiBlockIoProtocolGuid,
                NULL,
                &Count,
                &Handles
                );
  if (EFI_ERROR (Status) || (0 == Count)) {
    Print (L"No block found!\n");
    return;
  } else {
    Print (L"%d block(s) found!\n", Count);
  }

  for (Index = 0; Index < Count; Index++) {
    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo
                  );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Print (L"Block(%d):\n", Index);
    if (!BlockIo->Media->MediaPresent) {
      continue;
    }

    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiDiskIoProtocolGuid,
                  (VOID **)&DiskIo
                  );
    if (EFI_ERROR (Status)) {
      Print (L"No disk IO for %d\n", Index);
      continue;
    }

    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiDiskIo2ProtocolGuid,
                  (VOID **)&DiskIo2
                  );
    if (EFI_ERROR (Status)) {
      Print (L"No disk IO 2 for %d\n", Index);
      continue;
    }

    Status = ExtInitFileSystem (Handles[Index], BlockIo, DiskIo, DiskIo2, &FsHandle);
    if (EFI_ERROR (Status)) {
      continue;
    } else {

    }
  }

  if (NULL != Handles) {
    FreePool (Handles);
  }

  return;
}