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

  @param[in|out] Volume             The pointer to EXT2 volume.

  @retval  EFI_SUCCESS              We got he super block.
  @retval  Others                   No super block exists.

**/
EFI_STATUS
EFIAPI
Ext2SbValidate (
  IN OUT EXT2_VOLUME                *Volume
  )
{
  EFI_STATUS    Status;
  UINT8         *Buffer;
  EXT2FS        *Ext2Fs;
  UINT32        SbOffset;
  UINT32        BlockSize;

  if (NULL == Volume) {
    return EFI_INVALID_PARAMETER;
  }

  BlockSize = Volume->BlockIo->Media->BlockSize;
  Buffer = AllocatePool ((BlockSize > SBSIZE) ? BlockSize : SBSIZE);
  if (NULL == Buffer) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  //
  // Read the super block.
  //
  Status = Ext2ReadBlock (Volume->BlockIo, SBOFF / BlockSize, BlockSize, Buffer);
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
  CopyMem (&Volume->SuperBlock, Buffer, sizeof (EXT2FS));

DONE:

  if (Buffer != NULL) {
    FreePool (Buffer);
  }

  return Status;
}

/**
  Display EXT2 Superblock.

  @param[in|out] Volume             The pointer to EXT2 volume.

  @retval  NA

**/
VOID
ShowSuperBlockInfo (
  IN EXT2_VOLUME                    *Volume
  )
{
  Print (L"    INode count         : %d\n", Volume->SuperBlock.Ext2FsINodeCount);
  Print (L"    Block count         : %d\n", Volume->SuperBlock.Ext2FsBlockCount);
  Print (L"    Reserved block count: %d\n", Volume->SuperBlock.Ext2FsRsvdBlockCount);
  Print (L"    Free block count    : %d\n", Volume->SuperBlock.Ext2FsFreeBlockCount);
  Print (L"    Free INode count    : %d\n", Volume->SuperBlock.Ext2FsFreeINodeCount);
  Print (L"    First data block    : %d\n", Volume->SuperBlock.Ext2FsFirstDataBlock);
  Print (L"    Blocks per group    : %d\n", Volume->SuperBlock.Ext2FsBlocksPerGroup);
  Print (L"    INode per group     : %d\n", Volume->SuperBlock.Ext2FsINodesPerGroup);
  Print (L"    Magic               : 0x%x\n", Volume->SuperBlock.Ext2FsMagic);
  Print (L"    First INode         : %d\n", Volume->SuperBlock.Ext2FsFirstInode);
  Print (L"    FS GUID             : %g\n", Volume->SuperBlock.Ext2FsUuid);
  Print (L"    Volume name         : %a\n", Volume->SuperBlock.Ext2FsVolumeName);
}

/**
  Implements Simple File System Protocol interface function OpenVolume().

  @param[in]  This                  Calling context.
  @param[in]  File                  the Root Directory of the volume.

  @retval  EFI_OUT_OF_RESOURCES     Can not allocate the memory.
  @retval  EFI_VOLUME_CORRUPTED     The EXT2 type is error.
  @retval  EFI_SUCCESS              Open the volume successfully.

**/
EFI_STATUS
EFIAPI
Ext2OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE_PROTOCOL                **File
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Allocates volume structure, detects EXT2 file system, installs protocol,
  and initialize cache.

  @param[in]  Handle                The handle of parent device.
  @param[in]  DiskIo                The DiskIo of parent device.
  @param[in]  DiskIo2               The DiskIo2 of parent device.
  @param[in]  BlockIo               The BlockIo of parent device.

  @retval  EFI_SUCCESS              Allocate a new volume successfully.
  @retval  EFI_OUT_OF_RESOURCES     Can not allocate the memory.
  @return  Others                   Allocating a new volume failed.

**/
EFI_STATUS
Ext2AllocateVolume (
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo,
  IN  EFI_DISK_IO2_PROTOCOL         *DiskIo2,
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo
  )
{
  EFI_STATUS    Status;
  EXT2_VOLUME   *Volume;

  //
  // Allocate a volume structure.
  //
  Volume = AllocateZeroPool (sizeof (EXT2_VOLUME));
  if (Volume == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize the structure
  //
  Volume->Signature                   = EXT2_VOLUME_SIGNATURE;
  Volume->Handle                      = NULL;
  Volume->DiskIo                      = DiskIo;
  Volume->DiskIo2                     = DiskIo2;
  Volume->BlockIo                     = BlockIo;
  Volume->MediaId                     = BlockIo->Media->MediaId;
  Volume->ReadOnly                    = TRUE;  // Always true for EXT2.
  Volume->VolumeInterface.Revision    = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume  = Ext2OpenVolume;

  //
  // Check to see if there's a file system on the volume
  //
  Status = Ext2SbValidate (Volume);
  if (!EFI_ERROR (Status)) {
    ShowSuperBlockInfo (Volume);
  } else {
    FreePool (Volume);
  }

  return Status;
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

    Ext2AllocateVolume (Handles[Index], DiskIo, DiskIo2, BlockIo);
  }

  if (NULL != Handles) {
    FreePool (Handles);
  }

  return;
}