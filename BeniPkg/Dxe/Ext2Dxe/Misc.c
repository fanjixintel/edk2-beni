/**
*  @Package     : BeniPkg
*  @FileName    : Misc.c
*  @Date        : 20211130
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is a UEFI model driver used to initialzie EXT2 file system.
*
*  @History:
*    20211130: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Ext2.h"

/**
  Detects EXT2 file system on sisk and set relevant fields of Volume.

  @param[in]  Volume                The volume structure.

  @retval  EFI_SUCCESS              The EXT2 File System is detected successfully
  @retval  EFI_UNSUPPORTED          The volume is not FAT file system.
  @retval  EFI_VOLUME_CORRUPTED     The volume is corrupted.

**/
EFI_STATUS
Ext2OpenDevice (
  IN OUT EXT2_VOLUME                *Volume
  )
{
  EFI_STATUS    Status;
  UINT8         *Buffer;
  UINT32        BlockSize;
  UINT32        BufferSize;
  UINTN         SbOffset;
  EXT2FS        *Ext2Fs;

  Status = EFI_UNSUPPORTED;
  Buffer = NULL;
  BlockSize = 0;

  if (NULL == Volume) {
    Status = EFI_INVALID_PARAMETER;
    goto DONE;
  }

  BlockSize = Volume->BlockIo->Media->BlockSize;
  if (0 == BlockSize) {
    Status = EFI_INVALID_PARAMETER;
    goto DONE;
  }

  BufferSize = (BlockSize > SBSIZE) ? BlockSize : SBSIZE;
  Buffer = AllocatePool (BufferSize);
  if (NULL == Buffer) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  Status = MediaReadBlocks (
            Volume->BlockIo,
            SBOFF / BlockSize,
            BufferSize,
            Buffer
            );
  if (EFI_ERROR (Status)) {
    goto DONE;
  }

  SbOffset = (SBOFF < BlockSize) ? SBOFF : 0;
  Ext2Fs = (EXT2FS *)(&Buffer[SbOffset]);
  if (Ext2Fs->Ext2FsMagic != E2FS_MAGIC) {
    Status = EFI_UNSUPPORTED;
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

  E2FS_SBLOAD ((VOID *)Ext2Fs, (VOID *)&Volume->SuperBlock.Ext2Fs);

DONE:

  return Status;
}

/**
  Read group descriptor of the file.

  @param[in]  Volume                Pointer to EXT2_VOLUME.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ReadGDBlock (
  IN  EXT2_VOLUME                   *Volume
  )
{
  FILE          *Fp;
  UINT32        Gdpb;
  INT32         Index;
  EFI_STATUS    Status;
  M_EXT2FS      *FileSystem;

  Fp = (FILE *)&Volume->Root.FileSystemSpecificData;
  FileSystem = (M_EXT2FS *)&Volume->SuperBlock;
  Gdpb = FileSystem->Ext2FsBlockSize / FileSystem->Ext2FsGDSize;

  for (Index = 0; Index < FileSystem->Ext2FsNumGrpDesBlock; Index++) {
    Status = MediaReadBlocks (
              Volume->BlockIo,
              FSBTODB (FileSystem, FileSystem->Ext2Fs.Ext2FsFirstDataBlock + 1 /* superblock */ + Index),
              FileSystem->Ext2FsBlockSize,
              Fp->Buffer
              );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    E2FS_CGLOAD ((EXT2GD *)Fp->Buffer,
                 &FileSystem->Ext2FsGrpDes[Index * Gdpb],
                 (Index == (FileSystem->Ext2FsNumGrpDesBlock - 1)) ?
                    (FileSystem->Ext2FsNumCylinder - Gdpb * Index) * FileSystem->Ext2FsGDSize :
                    FileSystem->Ext2FsBlockSize
                );
  }

  return EFI_SUCCESS;
}
