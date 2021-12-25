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

  E2FS_SBLOAD ((VOID *)Ext2Fs, (VOID *)&Volume->Ext2Fs);
  //
  // Compute in-memory m_ext2fs values.
  //
  Volume->Ext2FsNumCylinder       =
    HOWMANY (Volume->Ext2Fs.Ext2FsBlockCount - Volume->Ext2Fs.Ext2FsFirstDataBlock,
             Volume->Ext2Fs.Ext2FsBlocksPerGroup);

  Volume->Ext2FsFsbtobd           = (INT32)(Volume->Ext2Fs.Ext2FsLogBlockSize + 10) - (INT32)HighBitSet32 (BlockSize);
  Volume->Ext2FsBlockSize         = MINBSIZE << Volume->Ext2Fs.Ext2FsLogBlockSize;
  Volume->Ext2FsLogicalBlock      = LOG_MINBSIZE + Volume->Ext2Fs.Ext2FsLogBlockSize;
  Volume->Ext2FsQuadBlockOffset   = Volume->Ext2FsBlockSize - 1;
  Volume->Ext2FsBlockOffset       = (UINT32)~Volume->Ext2FsQuadBlockOffset;
  Volume->Ext2FsGDSize            = 32; // sizeof (EXT2GD) 32 or 64.
  if (Volume->Ext2Fs.Ext2FsFeaturesIncompat & EXT2F_INCOMPAT_64BIT) {
    Volume->Ext2FsGDSize          = Volume->Ext2Fs.Ext2FsGDSize;
  }
  Volume->Ext2FsNumGrpDesBlock    =
    HOWMANY (Volume->Ext2FsNumCylinder, Volume->Ext2FsBlockSize / Volume->Ext2FsGDSize);
  Volume->Ext2FsInodesPerBlock    = Volume->Ext2FsBlockSize / Volume->Ext2Fs.Ext2FsInodeSize;
  Volume->Ext2FsInodesTablePerGrp = Volume->Ext2Fs.Ext2FsINodesPerGroup / Volume->Ext2FsInodesPerBlock;

DONE:

  return Status;
}
