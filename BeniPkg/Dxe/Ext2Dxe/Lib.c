/**
*  @Package     : BeniPkg
*  @FileName    : Lib.c
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

#define EXT2_DEBUG 1

/**
  Reads the requested number of blocks from the specified block device.

  The function reads the requested number of blocks from the device. All the
  blocks are read, or an error is returned. If there is no media in the device,
  the function returns EFI_NO_MEDIA.

  @param[in]  BlockIo               The pointer to EFI_BLOCK_IO_PROTOCOL.
  @param[in]  StartLBA              The starting logical block address (LBA) to read from
                                    on the device
  @param[in]  BufferSize            The size of the Buffer in bytes. This number must be
                                    a multiple of the intrinsic block size of the device.
  @param[out] Buffer                A pointer to the destination buffer for the data.
                                    The caller is responsible for the ownership of the
                                    buffer.

  @retval  EFI_SUCCESS              The data was read correctly from the device.
  @retval  EFI_UNSUPPORTED          The interface is not supported.
  @retval  EFI_NOT_READY            The MediaSetInterfaceType() has not been called yet.
  @retval  EFI_DEVICE_ERROR         The device reported an error while attempting
                                    to perform the read operation.
  @retval  EFI_INVALID_PARAMETER    The read request contains LBAs that are not
                                    valid, or the buffer is not properly aligned.
  @retval  EFI_NO_MEDIA             There is no media in the device.
  @retval  EFI_BAD_BUFFER_SIZE      The BufferSize parameter is not a multiple of
                                    the intrinsic block size of the device.

**/
EFI_STATUS
EFIAPI
MediaReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_LBA                       StartLBA,
  IN  UINTN                         BufferSize,
  OUT VOID                          *Buffer
  )
{
  return BlockIo->ReadBlocks(
                    BlockIo,
                    BlockIo->Media->MediaId,
                    StartLBA,
                    BufferSize,
                    Buffer
                    );
}

/**
  Read a new inode into a FILE structure.

  @param[in]     INumber            Inode number.
  @param[in/out] OpenFile           Pointer to open file struct.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ReadInode (
  IN  INODE32                       INumber,
  IN  OPEN_FILE                     *OpenFile
  )
{
  EFI_STATUS    Status;
  FILE          *Fp;
  M_EXT2FS      *FileSystem;
  CHAR8         *Buf;
  DADDRESS      InodeSector;
  EXT2GD        *Ext2FsGrpDes;
  EXTFS_DINODE  *DInodePtr;

  Fp = &OpenFile->FileStruct;
  FileSystem = Fp->FsPtr;

  //
  // 计算得到入参INumber对应的Inode所在的组描述符
  //
  Ext2FsGrpDes = FileSystem->Ext2FsGrpDes;
  Ext2FsGrpDes = (EXT2GD*)((UINTN)Ext2FsGrpDes +
                  (INOTOCG(FileSystem, INumber) * FileSystem->Ext2FsGDSize));

  //
  // 计算入参INumber对应的Inode表所在的块
  //
  InodeSector = (DADDRESS) (Ext2FsGrpDes->Ext2BGDInodeTables +
                            DivU64x32 (
                              ModU64x32 (
                                (INumber - 1), FileSystem->Ext2Fs.Ext2FsInodesPerGroup
                                ),
                              FileSystem->Ext2FsInodesPerBlock
                            )
                           );
  //
  // 如果支持64位，需要额外的操作
  //
  if (FileSystem->Ext2FsGDSize > EXT2_GD_SIZE) {
    if (0 != Ext2FsGrpDes->Ext2BGDInodeTablesHi) {
      InodeSector |= LShiftU64 ((UINT64) (Ext2FsGrpDes->Ext2BGDInodeTablesHi), 32);
    }
  }
  InodeSector = FSBTODB (FileSystem, InodeSector);

  //
  // 从硬盘获取Inode数据，获取的是一个EXT2块的大小
  //
  Buf = Fp->Buffer;
  Status = MediaReadBlocks (OpenFile->BlockIo, InodeSector, FileSystem->Ext2FsBlockSize, Buf);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a MediaReadBlocks failed. - %r\n", __FUNCTION__, Status));
    return Status;
  }
  //
  // 因为获取到的是硬盘中一整块数据，所以还要找到INumber对应的真正的Inode数据
  //
  DInodePtr = (EXTFS_DINODE *) (Buf + EXT2_DINODE_SIZE (FileSystem) * INODETOFSBO (FileSystem, INumber));
  CopyMem (&Fp->DiskInode, DInodePtr, sizeof(EXTFS_DINODE));

  //
  // 初始化为默认值
  //
  Fp->InodeCacheBlock = ~0;
  Fp->BufferBlockNum = -1;

  return Status;
}

/**
  Given an offset in a OPEN_FILE, find the disk block number that
  contains that block.

  @param[in]  File                  Pointer to an Open file.
  @param[in]  FileBlock             Block to find the file.
  @param[out] DiskBlockPtr          Pointer to the disk which contains block.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
BlockMap (
  IN  OPEN_FILE                     *File,
  IN  INDPTR                        FileBlock,
  OUT INDPTR                        *DiskBlockPtr
  )
{
  FILE                    *Fp;
  M_EXT2FS                *FileSystem;
  UINT32                  Level;
  INDPTR                  IndCache;
  INDPTR                  IndBlockNum;
  INDPTR                  *Buf;
  UINT32                  Index;
  UINT64                  NextLevelNode;
  EXT4_EXTENT_TABLE       *Etable;
  EXT4_EXTENT_INDEX       *ExtIndex;
  EXT4_EXTENT             *Extent;
  EFI_STATUS              Status;

  Fp = &File->FileStruct;
  FileSystem = Fp->FsPtr;
  Buf = (VOID *)Fp->Buffer;

  //
  // 判断是否支持EXT4扩展，它使用了另外的间接方式
  //
  if (0 != (Fp->DiskInode.Ext2DInodeStatusFlags & EXT4_EXTENTS)) {
    //
    // 判断间接是否正常，如果不是就返回错误
    //
    Etable = (EXT4_EXTENT_TABLE*) &(Fp->DiskInode.Ext2DInodeBlocks);
    if (Etable->Eheader.EhMagic != EXT4_EXTENT_HEADER_MAGIC) {
        DEBUG ((DEBUG_ERROR, "EXT4 extent header magic mismatch 0x%X!\n", Etable->Eheader.EhMagic));
        return EFI_DEVICE_ERROR;
    }

    //
    // 遍历整个索引树，找到对应的数据块所在的节点
    //
    while (Etable->Eheader.EhDepth > 0) {
      ExtIndex = NULL;
      for (Index=1; Index < Etable->Eheader.EhEntries; Index++) {
        ExtIndex = &(Etable->Enodes.Eindex[Index]);
        if (((UINT32) FileBlock) < ExtIndex->EiBlk) {
          ExtIndex = &(Etable->Enodes.Eindex[Index-1]);
          break;
        }
        ExtIndex = NULL;
      }

      if (NULL != ExtIndex) {
        //
        // 目前还不支持48位
        //
        ASSERT (ExtIndex->EiLeafHi == 0);
        NextLevelNode = ExtIndex->EiLeafLo;

        //
        // 继续深入读取下一个间接块
        //
        Status = MediaReadBlocks (
                  File->BlockIo,
                  FSBTODB (Fp->FsPtr, (DADDRESS) NextLevelNode),
                  FileSystem->Ext2FsBlockSize,
                  Buf
                  );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "%a MediaReadBlocks failed. - %r\n", __FUNCTION__, Status));
          return Status;
        }

        Etable = (EXT4_EXTENT_TABLE*) Buf;
        if (Etable->Eheader.EhMagic != EXT4_EXTENT_HEADER_MAGIC) {
            DEBUG ((DEBUG_ERROR, "EXT4 extent header magic mismatch 0x%X!\n", Etable->Eheader.EhMagic));
            return EFI_DEVICE_ERROR;
        }
      } else {
        DEBUG ((DEBUG_ERROR, "Could not find FileBlock #%d in the index extent data!\n", FileBlock));
        return EFI_NO_MAPPING;
      }
    }

    Extent = NULL;
    for (Index = 0; Index < Etable->Eheader.EhEntries; Index++) {
      Extent = &(Etable->Enodes.Extent[Index]);
      if ((((UINT32) FileBlock) >= Extent->Eblk) &&
          (((UINT32) FileBlock) < (Extent->Eblk + Extent->Elen))) {
        break;
      }
      Extent = NULL;
    }

    if (NULL != Extent) {
      //
      // 目前还不支持48位
      //
      ASSERT (Extent->EstartHi == 0);
      *DiskBlockPtr = Extent->EstartLo + (FileBlock - Extent->Eblk);
    } else {
      *DiskBlockPtr = 0;
    }
  } else {
    //
    // EXT2的数据块寻址方式
    //
    if (FileBlock < NDADDR) {
      //
      // 前面的12个数据块还是直接索引
      //
      *DiskBlockPtr = Fp->DiskInode.Ext2DInodeBlocks[FileBlock];
      return 0;
    }

    //
    // 之后的数据块通过间接方式寻址
    //
    FileBlock -= NDADDR;
    //
    // 左移8是因为一个间接可以存放256个指针，所以IndCache以256位单位
    //
    IndCache = FileBlock >> LN2_IND_CACHE_SZ;
    //
    // InodeCacheBlock初始值是全FF，所以开始不会进入
    //
    if (IndCache == Fp->InodeCacheBlock) {
      *DiskBlockPtr = Fp->InodeCache[FileBlock & IND_CACHE_MASK];
      return 0;
    }

    for (Level = 0;;) {
      Level += Fp->NiShift;
      if (FileBlock < (INDPTR)1 << Level) {
        break;
      }
      if (Level > NIADDR * Fp->NiShift) {
        //
        // Block number too high
        //
        return EFI_OUT_OF_RESOURCES;
      }
      FileBlock -= (INDPTR)1 << Level;
    }

    IndBlockNum =
      Fp->DiskInode.Ext2DInodeBlocks[NDADDR + (Level / Fp->NiShift - 1)];

    while (1) {
      Level -= Fp->NiShift;
      if (IndBlockNum == 0) {
        *DiskBlockPtr = 0;    // missing
        return 0;
      }

      //
      //  If we were feeling brave, we could work out the number
      //  of the disk sector and read a single disk sector instead
      //  of a filesystem block.
      //  However we don't do this very often anyway...
      //
      Status = MediaReadBlocks (
                File->BlockIo,
                FSBTODB (Fp->FsPtr, IndBlockNum),
                FileSystem->Ext2FsBlockSize,
                Buf
                );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a MediaReadBlocks failed. - %r\n", __FUNCTION__, Status));
        return Status;
      }
      IndBlockNum = Buf[FileBlock >> Level];
      if (Level == 0) {
        break;
      }
      FileBlock &= (1 << Level) - 1;
    }
    //
    // Save the part of the block that contains this sector
    //
    CopyMem (
      Fp->InodeCache,
      &Buf[FileBlock & ~IND_CACHE_MASK],
      IND_CACHE_SZ * sizeof (Fp->InodeCache[0])
      );
    Fp->InodeCacheBlock = IndCache;

    *DiskBlockPtr = IndBlockNum;
  }

  return EFI_SUCCESS;
}

/**
  Search a directory for a Name and return its inode number.

  @param[in]     Name               Name to compare with.
  @param[in]     Length             Length of the directroy/name name.
  @param[in/out] File               Pointer to file private data.
  @param[out]    INumPtr            Pointer to Inode number.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
SearchDirectory (
  IN     CHAR8                      *Name,
  IN     INT32                      Length,
  IN OUT OPEN_FILE                  *File,
  OUT    INODE32                    *INumPtr
  )
{
  FILE          *Fp;
  EXT2FS_DIRECT *Dp;
  EXT2FS_DIRECT *EdPtr;
  CHAR8         *Buf;
  UINT32        BufSize;
  INT32         NameLen;
  EFI_STATUS    Status;

  Fp = &File->FileStruct;
  Fp->SeekPtr = 0;

  while (Fp->SeekPtr < (OFFSET)Fp->DiskInode.Ext2DInodeSize) {
    Status = ReadFileInPortion (File, &Buf, &BufSize);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Dp = (EXT2FS_DIRECT *)Buf;
    EdPtr = (EXT2FS_DIRECT *) (Buf + BufSize);
    for (; Dp < EdPtr;
         Dp = (VOID *) ((CHAR8 *)Dp + Dp->Ext2DirectRecLen)) {
      if (Dp->Ext2DirectRecLen <= 0) {
        break;
      }
      if (Dp->Ext2DirectInodeNumber == (INODE32)0) {
        continue;
      }
      NameLen = Dp->Ext2DirectNameLen;
      if (NameLen == Length &&
          !CompareMem (Name, Dp->Ext2DirectName, Length)) {
        //
        // Found entry.
        //
        *INumPtr = Dp->Ext2DirectInodeNumber;
        AsciiSPrint (&File->FileName[0], EXT2FS_MAXNAMLEN, "%a", Name);
        return EFI_SUCCESS;
      }
    }
    Fp->SeekPtr += BufSize;
  }

  return EFI_NOT_FOUND;
}

/**
  Read a portion of a FILE into an internal buffer (FILE->Buffer).
  Return the location in the buffer and the amount in the buffer.

  @param[in]  File                  Pointer to the open file.
  @param[out] BufferPtr             Address in the internal buffer.
  @param[out] SizePtr               Size of remainder of buffer.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ReadFileInPortion (
  IN  OPEN_FILE                     *File,
  OUT CHAR8                         **BufferPtr,
  OUT UINT32                        *SizePtr
  )
{
  FILE          *Fp;
  M_EXT2FS      *FileSystem;
  INT32         Off;
  INDPTR        FileBlock;
  INDPTR        DiskBlock;
  UINT32        BlockSize;
  EFI_STATUS    Status;

  Fp         = &File->FileStruct;
  FileSystem = Fp->FsPtr;
  DiskBlock  = 0;

  //
  // 根据当前的文件位置（即Fp->SeekPtr），确定需要读取的块（FileBlock），以及在块中的位置（Off）
  //
  Off = BLOCKOFFSET (FileSystem, Fp->SeekPtr);
  FileBlock = LBLKNO (FileSystem, Fp->SeekPtr);
  BlockSize = FileSystem->Ext2FsBlockSize;

  //
  // 如果缓存中的已经是当前的块，就不需要重新读取了，否则就要重新读
  //
  if (FileBlock != Fp->BufferBlockNum) {
    //
    // 根据文件所在的块位置找到它位于硬盘中的数据块
    //
    Status = BlockMap (File, FileBlock, &DiskBlock);
    if (Status != 0) {
      return Status;
    }
    //
    // 0表示没有找到，清空缓存区
    //
    if (0 == DiskBlock) {
      SetMem32 (Fp->Buffer, BlockSize, 0);
      Fp->BufferSize = BlockSize;
    } else {
      Status = MediaReadBlocks (
                File->BlockIo,
                FSBTODB (FileSystem, DiskBlock),
                BlockSize,
                Fp->Buffer
                );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a MediaReadBlocks failed. - %r\n", __FUNCTION__, Status));
        return Status;
      }
    }

    Fp->BufferBlockNum = FileBlock;
  }

  //
  // 返回的是文件位置在当前数据块中的偏移以及文件在该数据块中剩余内容的大小
  //
  *BufferPtr = Fp->Buffer + Off;
  *SizePtr = BlockSize - Off;

  //
  // 有一种情况是数据块中的剩余内容还有剩余（即不是文件的一部分），那部分不是文件的内容，则需要去掉
  //
  if (*SizePtr > Fp->DiskInode.Ext2DInodeSize - Fp->SeekPtr) {
    *SizePtr = Fp->DiskInode.Ext2DInodeSize - Fp->SeekPtr;
  }

  return 0;
}

/**
  Dump the file system super block information.

  @param[in]  Volume                Pointer to EXT2_VOLUME.

  @retval  NA

**/
VOID
EFIAPI
DumpSBlock (
  IN  EXT2_VOLUME                   *Volume
  )
{
#if EXT2_DEBUG
  DEBUG ((DEBUG_INFO, "Ext2FsInodeCount = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsInodeCount));
  DEBUG ((DEBUG_INFO, "Ext2FsBlockCount = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsBlockCount));
  DEBUG ((DEBUG_INFO, "Ext2FsFirstDataBlock = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsFirstDataBlock));
  DEBUG ((DEBUG_INFO, "Ext2FsLogBlockSize = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsLogBlockSize));
  DEBUG ((DEBUG_INFO, "Ext2FsBlocksPerGroup = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsBlocksPerGroup));
  DEBUG ((DEBUG_INFO, "Ext2FsInodesPerGroup = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsInodesPerGroup));
  DEBUG ((DEBUG_INFO, "Ext2FsMagic = 0x%x\n", Volume->SuperBlock.Ext2Fs.Ext2FsMagic));
  DEBUG ((DEBUG_INFO, "Ext2FsRev = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsRev));

  if (Volume->SuperBlock.Ext2Fs.Ext2FsRev == E2FS_REV1) {
    DEBUG ((DEBUG_INFO, "Ext2FsFirstInode = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsFirstInode));
    DEBUG ((DEBUG_INFO, "Ext2FsInodeSize = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsInodeSize));
    DEBUG ((DEBUG_INFO, "Ext2FsFeaturesCompat = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsFeaturesCompat));
    DEBUG ((DEBUG_INFO, "Ext2FsFeaturesIncompat = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsFeaturesIncompat));
    DEBUG ((DEBUG_INFO, "Ext2FsFeaturesROCompat = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsFeaturesROCompat));
    DEBUG ((DEBUG_INFO, "Ext2FsRsvdGDBlock = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsRsvdGDBlock));
  }

  DEBUG ((DEBUG_INFO, "Ext2FsGDSize = %u\n", Volume->SuperBlock.Ext2FsGDSize));
  DEBUG ((DEBUG_INFO, "Ext2FsBlockSize = %u\n", Volume->SuperBlock.Ext2FsBlockSize));
  DEBUG ((DEBUG_INFO, "Ext2FsFsbtobd = %u\n", Volume->SuperBlock.Ext2FsFsbtobd));
  DEBUG ((DEBUG_INFO, "Ext2FsNumCylinder = %u\n", Volume->SuperBlock.Ext2FsNumCylinder));
  DEBUG ((DEBUG_INFO, "Ext2FsNumGrpDesBlock = %u\n", Volume->SuperBlock.Ext2FsNumGrpDesBlock));
  DEBUG ((DEBUG_INFO, "Ext2FsInodesPerBlock = %u\n", Volume->SuperBlock.Ext2FsInodesPerBlock));
  DEBUG ((DEBUG_INFO, "Ext2FsInodesTablePerGrp = %u\n", Volume->SuperBlock.Ext2FsInodesTablePerGrp));
#endif
}

/**
  Dump the file group descriptor block info.

  @param[in]  Volume                Pointer to EXT2_VOLUME.

  @retval  NA

**/
VOID
EFIAPI
DumpGroupDesBlock (
  IN  EXT2_VOLUME                   *Volume
  )
{
#if EXT2_DEBUG
  INT32    Index;
  EXT2GD   *Ext2FsGrpDesEntry;
  M_EXT2FS *FileSystem;

  FileSystem = (M_EXT2FS *)&Volume->SuperBlock;

  for (Index = 0; Index < FileSystem->Ext2FsNumCylinder; Index++) {
    Ext2FsGrpDesEntry = (EXT2GD *) ((UINT32 *) FileSystem->Ext2FsGrpDes + (Index * FileSystem->Ext2FsGDSize));
    DEBUG ((DEBUG_INFO, "Ext2FsGrpDes[Index = %u]\n", Index));
    DEBUG ((DEBUG_INFO, "  Ext2BGDBlockBitmap   %u\n", Ext2FsGrpDesEntry->Ext2BGDBlockBitmap));
    DEBUG ((DEBUG_INFO, "  Ext2BGDInodeBitmap   %u\n", Ext2FsGrpDesEntry->Ext2BGDInodeBitmap));
    DEBUG ((DEBUG_INFO, "  Ext2BGDInodeTables   %u\n", Ext2FsGrpDesEntry->Ext2BGDInodeTables));
    DEBUG ((DEBUG_INFO, "  Ext2BGDFreeBlocks    %u\n", Ext2FsGrpDesEntry->Ext2BGDFreeBlocks));
    DEBUG ((DEBUG_INFO, "  Ext2BGDFreeInodes    %u\n", Ext2FsGrpDesEntry->Ext2BGDFreeInodes));
    DEBUG ((DEBUG_INFO, "  Ext2BGDNumDir        %u\n", Ext2FsGrpDesEntry->Ext2BGDNumDir));
    if (FileSystem->Ext2FsGDSize > EXT2_GD_SIZE) {
      DEBUG ((DEBUG_INFO, "  Ext2BGDInodeTablesHi %u\n", Ext2FsGrpDesEntry->Ext2BGDInodeTablesHi));
    }
  }
#endif
}

/**
  Dump Inode.

  @param[in]  Inode                 Inode information.

  @retval  NA

**/
VOID
EFIAPI
DumpInode (
  IN  EXTFS_DINODE                  *Inode
  )
{
#if EXT2_DEBUG
  DEBUG ((DEBUG_INFO, "Inode Mode             : %x\n", Inode->Ext2DInodeMode));
  DEBUG ((DEBUG_INFO, "Inode UID              : %x\n", Inode->Ext2DInodeUid));
  DEBUG ((DEBUG_INFO, "Inode Size             : %x\n", Inode->Ext2DInodeSize));
  DEBUG ((DEBUG_INFO, "Inode Access Time      : %x\n", Inode->Ext2DInodeAcessTime));
  DEBUG ((DEBUG_INFO, "Inode Create Time      : %x\n", Inode->Ext2DInodeCreatTime));
  DEBUG ((DEBUG_INFO, "Inode Modification Time: %x\n", Inode->Ext2DInodeModificationTime));
  DEBUG ((DEBUG_INFO, "Inode Deletion Time    : %x\n", Inode->Ext2DInodeDeletionTime));
  DEBUG ((DEBUG_INFO, "Inode GID              : %x\n", Inode->Ext2DInodeGid));
  DEBUG ((DEBUG_INFO, "Inode Link Count       : %x\n", Inode->Ext2DInodeLinkCount));
  DEBUG ((DEBUG_INFO, "Inode Block Count      : %x\n", Inode->Ext2DInodeBlockCount));
  DEBUG ((DEBUG_INFO, "Inode Status Flags     : %x\n", Inode->Ext2DInodeStatusFlags));
#endif
}
