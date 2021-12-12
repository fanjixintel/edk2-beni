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
  Read disk.

  @param[in]  BlockIo               The pointer to EFI_BLOCK_IO_PROTOCOL.
  @param[in]  Lba                   The start LBA to read data.
  @param[in]  BufferSize            The data size to read.
  @param[out] Buffer                The read data buffer.

  @return  EFI_SUCCESS              Operation succeeded.
  @return  Others                   Operation failed.

**/
EFI_STATUS
Ext2ReadBlock (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_LBA                       Lba,
  IN  UINTN                         BufferSize,
  OUT VOID                          *Buffer
  )
{
  return BlockIo->ReadBlocks(
                    BlockIo,
                    BlockIo->Media->MediaId,
                    Lba,
                    BufferSize,
                    Buffer
                    );
}

/**
  Validate EXT2 Superblock.

  @param[in]  BlockIo               The pointer to EFI_BLOCK_IO_PROTOCOL.
  @param[out] Ext2Fs                EXT2 file system meta data to retreive.

  @retval  EFI_SUCCESS              We got he super block.
  @retval  Others                   No super block exists.

**/
EFI_STATUS
EFIAPI
Ext2SbValidate (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  OUT VOID                          **RExt2Fs  OPTIONAL
  )
{
  EFI_STATUS    Status;
  UINT8         *Buffer;
  EXT2FS        *Ext2Fs;
  UINT32        SbOffset;
  UINT32        BlockSize;

  if (NULL == BlockIo) {
    return EFI_INVALID_PARAMETER;
  }

  BlockSize = BlockIo->Media->BlockSize;
  Buffer = AllocatePool ((BlockSize > SBSIZE) ? BlockSize : SBSIZE);
  if (NULL == Buffer) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  //
  // Read the super block.
  //
  Status = Ext2ReadBlock (BlockIo, SBOFF / BlockSize, BlockSize, Buffer);
  if (EFI_ERROR (Status)) {
    goto DONE;
  }

  SbOffset = (SBOFF < BlockSize) ? SBOFF : 0;
  Ext2Fs = (EXT2FS *)(&Buffer[SbOffset]);
  if (E2FS_MAGIC != Ext2Fs->Ext2FsMagic) {
    Status = EFI_NOT_FOUND;
    goto DONE;
  }

  if (Ext2Fs->Ext2FsRev > E2FS_REV1 ||
      (Ext2Fs->Ext2FsRev == E2FS_REV1 &&
       (Ext2Fs->Ext2FsFirstInode != EXT2_FIRSTINO ||
        (Ext2Fs->Ext2FsInodeSize != 128 && Ext2Fs->Ext2FsInodeSize != 256) ||
        Ext2Fs->Ext2FsFeaturesIncompat & ~EXT2F_INCOMPAT_SUPP))) {
    Status = EFI_UNSUPPORTED;
    goto DONE;
  }

  Print (L"  EXT found.\n");

  if (RExt2Fs != NULL) {
    *RExt2Fs = (VOID *)Buffer;
  }

  return EFI_SUCCESS;

DONE:

  if (Buffer != NULL) {
    FreePool (Buffer);
  }

  return Status;
}

/**
  Display EXT2 Superblock.

  @param[in]  Ext2Fs                The pointer to EXT2FS.

  @retval  NA

**/
VOID
ShowSuperBlockInfo (
  IN  EXT2FS                        *Ext2Fs
  )
{
  Print (L"    INode count         : %d\n", Ext2Fs->Ext2FsINodeCount);
  Print (L"    Block count         : %d\n", Ext2Fs->Ext2FsBlockCount);
  Print (L"    Reserved block count: %d\n", Ext2Fs->Ext2FsRsvdBlockCount);
  Print (L"    Free block count    : %d\n", Ext2Fs->Ext2FsFreeBlockCount);
  Print (L"    Free INode count    : %d\n", Ext2Fs->Ext2FsFreeINodeCount);
  Print (L"    First data block    : %d\n", Ext2Fs->Ext2FsFirstDataBlock);
  Print (L"    Blocks per group    : %d\n", Ext2Fs->Ext2FsBlocksPerGroup);
  Print (L"    INode per group     : %d\n", Ext2Fs->Ext2FsINodesPerGroup);
  Print (L"    Magic               : 0x%x\n", Ext2Fs->Ext2FsMagic);
  Print (L"    First INode         : %d\n", Ext2Fs->Ext2FsFirstInode);
  Print (L"    FS GUID             : %g\n", Ext2Fs->Ext2FsUuid);
  Print (L"    Volume name         : %a\n", Ext2Fs->Ext2FsVolumeName);
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
  EXT2FS                            *Ext2Fs;

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

    Status = Ext2SbValidate (BlockIo, &Ext2Fs);
    if (!EFI_ERROR (Status)) {
      ShowSuperBlockInfo (Ext2Fs);
    }
  }
}