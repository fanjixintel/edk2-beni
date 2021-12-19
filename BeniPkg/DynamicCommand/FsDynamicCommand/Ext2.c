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

typedef struct ENTRY {
  struct ENTRY            *EntryNext;
  INODE32                 EntryInode;
  UINT8                   EntryType;
  CHAR8                   EntryName[EXT2FS_MAXNAMLEN];
} ENTRY;

STATIC CONST CHAR8 *mTypeStr[] = {
  "unknown",
  "REG",
  "DIR",
  "CHR",
  "BLK",
  "FIFO",
  "SOCK",
  "LNK"
};

/**
  List directories or files and print them.

  @param[in]  File                  Pointer to an file private data.
  @param[in]  Pattern               Not used for now.

  @retval  EFI_SUCCESS              List directories or files successfully.
  @retval  EFI_NOT_FOUND            Not found specified dir or file.
  @retval  EFI_DEVICE_ERROR         An error while accessing filesystem.

**/
STATIC
EFI_STATUS
EFIAPI
Ext2fsLs (
  IN  OPEN_FILE                     *File,
  IN  CONST CHAR8                   *Pattern
  )
{
  EFI_STATUS              Status;
  FILE                    *Fp;
  UINT32                  BlockSize;
  CHAR8                   *Buf;
  UINT32                  BufSize;
  ENTRY                   *Names;
  ENTRY                   *New;
  ENTRY                   **NextPtr;
  EXT2FS_DIRECT           *Dp;
  EXT2FS_DIRECT           *EdPtr;
  UINT32                  FileSize;
  ENTRY                   *PNames;
  CONST CHAR8             *Type;

  Status = EFI_SUCCESS;
  Fp     = (FILE *)File->FileSystemSpecificData;

  if (EXT2_IFREG == (Fp->DiskInode.Ext2DInodeMode & EXT2_IFMT)) {
    DEBUG ((DEBUG_INFO, "  %-16a %u\n", File->FileNamePtr, Fp->DiskInode.Ext2DInodeSize));
    return EFI_SUCCESS;
  } else if (EXT2_IFDIR != (Fp->DiskInode.Ext2DInodeMode & EXT2_IFMT)) {
    return EFI_NOT_FOUND;
  }

  BlockSize   = Fp->SuperBlockPtr->Ext2FsBlockSize;
  Names       = NULL;
  Fp->SeekPtr = 0;
  while (Fp->SeekPtr < (OFFSET)Fp->DiskInode.Ext2DInodeSize) {
    Status = BufReadFile (File, &Buf, &BufSize);
    if (RETURN_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto DONE;
    }
    if ((BufSize != BlockSize) || (0 == BufSize)) {
      Status = EFI_DEVICE_ERROR;
      goto DONE;
    }

    Dp    = (EXT2FS_DIRECT *)Buf;
    EdPtr = (EXT2FS_DIRECT *)(Buf + BufSize);
    for (; Dp < EdPtr; Dp = (VOID *) ((CHAR8 *)Dp + Dp->Ext2DirectRecLen)) {
      if (Dp->Ext2DirectRecLen <= 0) {
        Status = EFI_DEVICE_ERROR;
        goto DONE;
      }

      if (0 == Dp->Ext2DirectInodeNumber) {
        continue;
      }

      if (Dp->Ext2DirectType >= ARRAY_SIZE (mTypeStr)) {
        //
        //  This does not handle "Old"
        //  filesystems properly. On little
        //  endian machines, we get a bogus
        //  type Name if the NameLen matches a
        //  valid type identifier. We could
        //  check if we read NameLen "0" and
        //  handle this case specially, if
        //  there were a pressing need...
        //
        DEBUG ((DEBUG_INFO, "Bad dir entry - %d\n", Dp->Ext2DirectType));
        Status = EFI_DEVICE_ERROR;
        goto DONE;
      }
      Type = mTypeStr[Dp->Ext2DirectType];

      New = AllocateZeroPool (sizeof * New + AsciiStrLen (Dp->Ext2DirectName));
      if (NULL == New) {
        DEBUG ((DEBUG_INFO, "%d: %s (%s)\n",
                Dp->Ext2DirectInodeNumber, Dp->Ext2DirectName, Type));
        continue;
      }

      New->EntryInode = Dp->Ext2DirectInodeNumber;
      New->EntryType  = Dp->Ext2DirectType;
      AsciiStrCpyS (New->EntryName, EXT2FS_MAXNAMLEN, Dp->Ext2DirectName);
      New->EntryName[Dp->Ext2DirectNameLen] = '\0';
      for (NextPtr = &Names; *NextPtr != NULL; NextPtr = & (*NextPtr)->EntryNext) {
        if (AsciiStrCmp (New->EntryName, (*NextPtr)->EntryName) < 0) {
          break;
        }
      }
      New->EntryNext = *NextPtr;
      *NextPtr = New;
    }
    Fp->SeekPtr += BufSize;
  }

  if (NULL != Names) {
    PNames = Names;
    do {
      New = PNames;
      if (2 == New->EntryType) {
        DEBUG ((DEBUG_INFO, "  %a/\n", New->EntryName));
      } else if (1 == New->EntryType) {
        FileSize = 0;
        Status = ReadInode (New->EntryInode, File);
        if (!RETURN_ERROR (Status)) {
          Fp = (FILE *)File->FileSystemSpecificData;
          FileSize = Fp->DiskInode.Ext2DInodeSize;
        }
        Status = RETURN_SUCCESS;
        DEBUG ((DEBUG_INFO, "  %-16a %u\n", New->EntryName, FileSize));
      }
      PNames = New->EntryNext;
    } while (PNames != NULL);
  }

DONE:

  if (Names != NULL) {
    do {
      New   = Names;
      Names = New->EntryNext;
      FreePool (New);
    } while (Names != NULL);
  }

  return Status;
}

/**
  List directories or files.

  @param[in]  FsHandle              File system handle.
  @param[in]  DirFilePath           Directory or file path.

  @retval  EFI_SUCCESS              List directories of files successfully.
  @retval  EFI_UNSUPPORTED          This api is not supported.
  @retval  Others                   Error occured.

**/
EFI_STATUS
EFIAPI
ExtFsListDir (
  IN  EFI_HANDLE                    FsHandle,
  IN  CHAR16                        *DirFilePath
  )
{
  EFI_STATUS    Status;
  EFI_HANDLE    FileHandle;

  Status = EFI_UNSUPPORTED;
  FileHandle = NULL;

  Status = ExtFsOpenFile (FsHandle, DirFilePath, &FileHandle);
  if (!EFI_ERROR (Status)) {
    Status = Ext2fsLs ((OPEN_FILE *)FileHandle, NULL);
  }
  if (NULL != FileHandle) {
    ExtFsCloseFile (FileHandle);
  }

  return Status;
}

/**
  Show EXT2 file system.

  @param  NA

  @retval  NA

**/
EFI_STATUS
EFIAPI
ListRootDir (
  IN  EFI_HANDLE                    FsHandle
  )
{
  ExtFsListDir (FsHandle, L"/");

  return EFI_NOT_FOUND;
}

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
      DEBUG ((EFI_D_ERROR, "Not present\n"));
      continue;
    }

    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiDiskIoProtocolGuid,
                  (VOID **)&DiskIo
                  );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "No disk IO for %d\n", Index));
      continue;
    }

    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiDiskIo2ProtocolGuid,
                  (VOID **)&DiskIo2
                  );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "No disk IO 2 for %d\n", Index));
      continue;
    }

    Status = ExtInitFileSystem (Handles[Index], BlockIo, DiskIo, DiskIo2, &FsHandle);
    if (EFI_ERROR (Status)) {
      continue;
    } else {
      ListRootDir (FsHandle);
    }
  }

  if (NULL != Handles) {
    FreePool (Handles);
  }

  return;
}
