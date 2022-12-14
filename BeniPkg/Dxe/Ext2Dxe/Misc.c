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
  OFFSET        SbOffset;
  EXT2FS        *Ext2Fs;
  M_EXT2FS      *FileSystem;

  Status    = EFI_UNSUPPORTED;
  Buffer    = NULL;
  BlockSize = 0;

  if (NULL == Volume) {
    DEBUG ((EFI_D_ERROR, "Invalid parameter\n"));
    Status = EFI_INVALID_PARAMETER;
    goto DONE;
  }

  BlockSize = Volume->BlockIo->Media->BlockSize;
  if (0 == BlockSize) {
    DEBUG ((EFI_D_ERROR, "Disk block size is 0\n"));
    Status = EFI_INVALID_PARAMETER;
    goto DONE;
  }

  //
  // 从磁盘读取超级块，读取方式需要根据硬盘的物理块来确定
  // 如果物理块大小是512字节，则指定Buffer大小是1024，即1个超级块的大小，然后读第2个物理块开始的2个物理块
  // 如果物理块大小是4096字节，则指定Buffer大小是4096，直接读第1个物理块即可
  //
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
    DEBUG ((EFI_D_ERROR, "%a MediaReadBlocks failed. - %r\n", __FUNCTION__, Status));
    goto DONE;
  }
  //
  // 读取数据之后需要根据读的Buffer来确定超级块的位置
  // 如果读取2个物理块，则Offset是0，读到的数据就是超级块
  // 如果读取1个物理块，则这个物理块的offset为1024字节的偏移位置就是超级块的开始
  //
  SbOffset = (SBOFF < BlockSize) ? SBOFF : 0;
  Ext2Fs = (EXT2FS *)(&Buffer[SbOffset]);
  //
  // 超级块首先需要判断魔术字是否正确
  //
  if (EXT2_SUPER_MAGIC != Ext2Fs->Ext2FsMagic) {
    Status = EFI_NOT_FOUND;
    goto DONE;
  }
  //
  // 其次需要判断版本信息以及相关的配套数据是否正确
  // 1. 如果EXT2的版本是1，则第一个可用Inode是从Inode 11开始的（注意Inode的Index从1开始计数，而不是0）
  // 2. 文件系统中的Inode大小必须是固定的128个字节或者256个字节
  // 3. 兼容性判断
  //
  if ((Ext2Fs->Ext2FsRev > E2FS_REV1) ||
      ((E2FS_REV1 == Ext2Fs->Ext2FsRev)  &&
       ((EXT2_FIRSTINO != Ext2Fs->Ext2FsFirstInode) ||
        ((128 != Ext2Fs->Ext2FsInodeSize) && (256 != Ext2Fs->Ext2FsInodeSize)) ||
        (Ext2Fs->Ext2FsFeaturesIncompat & ~EXT2F_INCOMPAT_SUPP)))) {
    Status = EFI_NOT_FOUND;
    goto DONE;
  }

  //
  // 通过现有数据填充剩余内容
  //
  FileSystem = &Volume->SuperBlock;
  CopyMem ((VOID *)&FileSystem->Ext2Fs, (VOID *)Ext2Fs, sizeof (EXT2FS));
  FileSystem = &Volume->SuperBlock;
  FileSystem->Ext2FsNumCylinder       =
    HOWMANY ((FileSystem->Ext2Fs.Ext2FsBlockCount - FileSystem->Ext2Fs.Ext2FsFirstDataBlock),
             FileSystem->Ext2Fs.Ext2FsBlocksPerGroup);
  FileSystem->Ext2FsFsbtobd           = (INT32)(FileSystem->Ext2Fs.Ext2FsLogBlockSize + 10) -
                                        (INT32)HighBitSet32 (Volume->BlockIo->Media->BlockSize);
  FileSystem->Ext2FsBlockSize         = MINBSIZE << FileSystem->Ext2Fs.Ext2FsLogBlockSize;
  FileSystem->Ext2FsLogicalBlock      = LOG_MINBSIZE + FileSystem->Ext2Fs.Ext2FsLogBlockSize;
  FileSystem->Ext2FsQuadBlockOffset   = FileSystem->Ext2FsBlockSize - 1;
  FileSystem->Ext2FsBlockOffset       = (UINT32)~FileSystem->Ext2FsQuadBlockOffset;
  FileSystem->Ext2FsGDSize            = EXT2_GD_SIZE;
  if (FileSystem->Ext2Fs.Ext2FsFeaturesIncompat & EXT2F_INCOMPAT_64BIT) {
    FileSystem->Ext2FsGDSize          = FileSystem->Ext2Fs.Ext2FsGDSize;
  }
  FileSystem->Ext2FsNumGrpDesBlock    =
    HOWMANY (FileSystem->Ext2FsNumCylinder, FileSystem->Ext2FsBlockSize / FileSystem->Ext2FsGDSize);
  FileSystem->Ext2FsInodesPerBlock    = FileSystem->Ext2FsBlockSize / FileSystem->Ext2Fs.Ext2FsInodeSize;
  FileSystem->Ext2FsInodesTablePerGrp = FileSystem->Ext2Fs.Ext2FsInodesPerGroup / FileSystem->Ext2FsInodesPerBlock;

  //
  // 获取组描述符
  //
  FileSystem->Ext2FsGrpDes = AllocatePool (FileSystem->Ext2FsGDSize * FileSystem->Ext2FsNumCylinder);
  if (NULL == FileSystem->Ext2FsGrpDes) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }
  Status = ReadGDBlock (Volume);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ReadGDBlock failed. - %r\n", Status));
    goto DONE;
  }

  //
  // 打印数据，调试用
  //
  DumpSBlock (Volume);
  DumpGroupDesBlock (Volume);

DONE:

  if (EFI_ERROR (Status)) {
    if (NULL != FileSystem->Ext2FsGrpDes) {
      FreePool (FileSystem->Ext2FsGrpDes);
    }
  }

  if (NULL != Buffer) {
    FreePool (Buffer);
  }

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
  EFI_STATUS    Status;
  UINT32        Gdpb;
  INT32         Index;
  UINT8         *Buffer;
  M_EXT2FS      *FileSystem;

  Status = EFI_INVALID_PARAMETER;
  FileSystem = &Volume->SuperBlock;
  Buffer = AllocatePool (FileSystem->Ext2FsBlockSize);
  if (NULL == Buffer) {
    Status = EFI_INVALID_PARAMETER;
    goto DONE;
  }
  Gdpb = FileSystem->Ext2FsBlockSize / FileSystem->Ext2FsGDSize;

  for (Index = 0; Index < FileSystem->Ext2FsNumGrpDesBlock; Index++) {
    Status = MediaReadBlocks (
              Volume->BlockIo,
              FSBTODB (FileSystem, FileSystem->Ext2Fs.Ext2FsFirstDataBlock + 1 /* superblock */ + Index),
              FileSystem->Ext2FsBlockSize,
              Buffer
              );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a MediaReadBlocks failed. - %r\n", __FUNCTION__, Status));
      FreePool (Buffer);
      return Status;
    }

    CopyMem (&FileSystem->Ext2FsGrpDes[Index * Gdpb],
             (EXT2GD *)Buffer,
             (Index == (FileSystem->Ext2FsNumGrpDesBlock - 1)) ?
                  (FileSystem->Ext2FsNumCylinder - Gdpb * Index) * FileSystem->Ext2FsGDSize :
                  FileSystem->Ext2FsBlockSize
    );
  }

DONE:

  if (NULL != Buffer) {
    FreePool (Buffer);
  }

  return EFI_SUCCESS;
}
