/**
*  @Package     : BeniPkg
*  @FileName    : Ext2Ex.c
*  @Date        : 20220101
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for Ext2Dxe file system test.
*
*  @History:
*    20220101: Initialize.
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
#include "../../Dxe/Ext2Dxe/Ext2.h"

#include <Library/BeniTimeLib.h>

#include <Protocol/SimpleFileSystem.h>

#define TEST_FILE_NAME    L"tmp1.log"

/**
  Show EXT2 file system.

  @param  NA

  @retval  NA

**/
VOID
EFIAPI
ShowExt2FileSystemEx (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FsPtr;
  EFI_HANDLE                        *FsHandles;
  UINTN                             FsCount;
  EFI_FILE_HANDLE                   Root;
  EFI_FILE_HANDLE                   NewFile;
  UINTN                             Index;
  EXT2_VOLUME                       *Volume;
  UINTN                             BufferSize;
  UINT8                             *Buffer;
  EFI_FILE_INFO                     *Info;

  Status      = EFI_UNSUPPORTED;
  Root        = NULL;
  BufferSize  = 0;
  Buffer      = NULL;

  //
  // Find the first file system, and put the files we found in this file system.
  //
  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiSimpleFileSystemProtocolGuid,
                NULL,
                &FsCount,
                &FsHandles
                );
  if (EFI_ERROR (Status) || (0 == FsCount)) {
    Print (L"No file system found!\n");
    return;
  } else {
    Print (L"%d file system(s) found!\n", FsCount);
    Print (L"-----------------------------------\n");
  }

  for (Index = 0; Index < FsCount; Index++) {
    Status = gBS->HandleProtocol (
                    FsHandles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **)&FsPtr
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = FsPtr->OpenVolume (FsPtr, &Root);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "[BENI]OpenVolume failed. - %r\n", Status));
      continue;
    }

    Volume  = VOLUME_FROM_VOL_INTERFACE (FsPtr);
    if (EXT2_VOLUME_SIGNATURE == Volume->Signature) {
      break;
    } else {
      Root->Close (Root);
    }
  }

  Status = Root->Open (Root, &NewFile, TEST_FILE_NAME, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Open file failed. - %r\n", Status));
    goto DONE;
  }

  Print (L"%s opened.\n", TEST_FILE_NAME);

  Status = NewFile->GetInfo (NewFile, &gEfiFileInfoGuid, &BufferSize, Buffer);
  if (EFI_ERROR (Status)) {
    if (EFI_BUFFER_TOO_SMALL == Status) {
      Buffer = AllocatePool (BufferSize);
    }
  }
  if (NULL == Buffer) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  Status = NewFile->GetInfo (NewFile, &gEfiFileInfoGuid, &BufferSize, Buffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetInfo failed. - %r\n", Status));
    goto DONE;
  }

  Info = (EFI_FILE_INFO *)Buffer;
  Print (L"  Size: %d Bytes\n", Info->Size);
  Print (L"  File name: %s\n", Info->FileName);
  Print (L"  File size: %d Bytes\n", Info->Size);
  Print (L"  File physical size: %d Bytes\n", Info->PhysicalSize);
  Print (L"  File Attribute: 0x%x\n", Info->Attribute);
  PrintEfiTime (&Info->CreateTime, TRUE);
  PrintEfiTime (&Info->LastAccessTime, TRUE);
  PrintEfiTime (&Info->ModificationTime, TRUE);

  NewFile->Close (NewFile);

  if (NULL != Root) {
    Root->Close (Root);
  }

DONE:

  BENI_FREE_NON_NULL (Buffer);

  return;
}
