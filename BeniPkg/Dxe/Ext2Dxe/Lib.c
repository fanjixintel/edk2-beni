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
  @param[in/out] File               Pointer to open file struct.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ReadInode (
  IN  INODE32                       INumber,
  IN  OPEN_FILE                     *File
  )
{
  EFI_STATUS    Status;
  FILE          *Fp;
  M_EXT2FS      *FileSystem;
  CHAR8         *Buf;
  DADDRESS      InodeSector;
  EXT2GD        *Ext2FsGrpDes;
  EXTFS_DINODE  *DInodePtr;

  Fp = &File->FileStruct;
  FileSystem = Fp->SuperBlockPtr;

  Ext2FsGrpDes = FileSystem->Ext2FsGrpDes;
  Ext2FsGrpDes = (EXT2GD*)((UINTN)Ext2FsGrpDes + (INOTOCG(FileSystem, INumber) * FileSystem->Ext2FsGDSize));

  InodeSector = (DADDRESS) (Ext2FsGrpDes->Ext2BGDInodeTables + DivU64x32 (ModU64x32 ((INumber - 1), FileSystem->Ext2Fs.Ext2FsINodesPerGroup), FileSystem->Ext2FsInodesPerBlock));

  if (FileSystem->Ext2FsGDSize > 32) {
    if (0 != Ext2FsGrpDes->Ext2BGDInodeTablesHi) {
      InodeSector |= LShiftU64 ((UINT64) (Ext2FsGrpDes->Ext2BGDInodeTablesHi), 32);
    }
  }
  InodeSector = FSBTODB (FileSystem, InodeSector);

  //
  // Read inode and save it.
  //
  Buf = Fp->Buffer;
  Status = MediaReadBlocks (File->BlockIo, InodeSector, FileSystem->Ext2FsBlockSize, Buf);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a MediaReadBlocks failed. - %r\n", __FUNCTION__, Status));
    return Status;
  }

  DInodePtr = (EXTFS_DINODE *) (Buf +
                                EXT2_DINODE_SIZE (FileSystem) * INODETOFSBO (FileSystem, INumber));
  E2FSILOAD (DInodePtr, &Fp->DiskInode);

  //
  // Clear out the old buffers.
  //
  Fp->InodeCacheBlock = ~0;
  Fp->BufferBlockNum = -1;

  return Status;
}

/**
  Given an offset in a FILE, find the disk block number that
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
  FileSystem = Fp->SuperBlockPtr;
  Buf = (VOID *)Fp->Buffer;

  if ((Fp->DiskInode.Ext2DInodeStatusFlags & EXT4_EXTENTS) != 0) {
    Etable = (EXT4_EXTENT_TABLE*) &(Fp->DiskInode.Ext2DInodeBlocks);
    if (Etable->Eheader.EhMagic != EXT4_EXTENT_HEADER_MAGIC) {
        DEBUG ((DEBUG_ERROR, "EXT4 extent header magic mismatch 0x%X!\n", Etable->Eheader.EhMagic));
        return EFI_DEVICE_ERROR;
    }

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

      if (ExtIndex != NULL) {
        //
        // TODO: Need to support 48-bit block device addressing.
        // Throw an ASSERT if upper 16-bits are non-zero.
        //
        ASSERT (ExtIndex->EiLeafHi == 0);
        NextLevelNode = ExtIndex->EiLeafLo; //LShiftU64((UINT64)ExtIndex->EiLeafHi, 32) | ExtIndex->EiLeafLo;

        //
        // We need to read the next level node of the extent tree since the data was not in the current level.
        //
        Status = MediaReadBlocks (
                  File->BlockIo,
                  FSBTODB (Fp->SuperBlockPtr, (DADDRESS) NextLevelNode),
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
    for (Index=0; Index < Etable->Eheader.EhEntries; Index++) {
      Extent = &(Etable->Enodes.Extent[Index]);
      if ((((UINT32) FileBlock) >= Extent->Eblk) && (((UINT32) FileBlock) < (Extent->Eblk + Extent->Elen))) {
        break;
      }
      Extent = NULL;
    }

    if (Extent != NULL) {
      //
      // TODO: Need to support 48-bit block device addressing
      // Throw an ASSERT if upper 16-bits are non-zero.
      //
      ASSERT (Extent->EstartHi == 0);
      *DiskBlockPtr = Extent->EstartLo + (FileBlock - Extent->Eblk); // (LShiftU64((UINT64)Extent->EiLeafHi, 32) | Extent->EstartLo) + (FileBlock - Extent->Eblk);
    } else {
      *DiskBlockPtr = 0;
    }
  } else {
    if (FileBlock < NDADDR) {
      //
      // Direct block.
      //
      *DiskBlockPtr = Fp->DiskInode.Ext2DInodeBlocks[FileBlock];
      return 0;
    }

    FileBlock -= NDADDR;

    IndCache = FileBlock >> LN2_IND_CACHE_SZ;
    if (IndCache == Fp->InodeCacheBlock) {
      *DiskBlockPtr =
        Fp->InodeCache[FileBlock & IND_CACHE_MASK];
      return 0;
    }

    for (Level = 0;;) {
      Level += Fp->NiShift;
      if (FileBlock < (INDPTR)1 << Level) {
        break;
      }
      if (Level > NIADDR * Fp->NiShift)
        //
        // Block number too high
        //
      {
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
                FSBTODB (Fp->SuperBlockPtr, IndBlockNum),
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
    CopyMem (Fp->InodeCache, &Buf[FileBlock & ~IND_CACHE_MASK],
             IND_CACHE_SZ * sizeof Fp->InodeCache[0]);
    Fp->InodeCacheBlock = IndCache;

    *DiskBlockPtr = IndBlockNum;
  }

  return EFI_SUCCESS;
}

/**
  Search a directory for a Name and return its inode number.

  @param[in]     Name               Name to compare with.
  @param[in]     Length             Length of the dir name.
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
  //
  // XXX should handle LARGEFILE.
  //
  while (Fp->SeekPtr < (OFFSET)Fp->DiskInode.Ext2DInodeSize) {
    Status = BufReadFile (File, &Buf, &BufSize);
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
        File->FileNamePtr = Name;
        return 0;
      }
    }
    Fp->SeekPtr += BufSize;
  }

  return EFI_NOT_FOUND;
}

/**
  Read a portion of a FILE into an internal buffer.
  Return the location in the buffer and the amount in the buffer.

  @param[in]  File                  Pointer to the open file.
  @param[out] BufferPtr             Buffer corresponding to offset.
  @param[out] SizePtr               Size of remainder of buffer.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
BufReadFile (
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

  Fp = &File->FileStruct;
  FileSystem = Fp->SuperBlockPtr;
  DiskBlock  = 0;

  Off = BLOCKOFFSET (FileSystem, Fp->SeekPtr);
  FileBlock = LBLKNO (FileSystem, Fp->SeekPtr);
  BlockSize = FileSystem->Ext2FsBlockSize;    // no fragment

  if (FileBlock != Fp->BufferBlockNum) {
    Status = BlockMap (File, FileBlock, &DiskBlock);
    if (Status != 0) {
      return Status;
    }

    if (DiskBlock == 0) {
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
  //  Return address of byte in buffer corresponding to
  //  offset, and Size of remainder of buffer after that
  //  byte.
  //
  *BufferPtr = Fp->Buffer + Off;
  *SizePtr = BlockSize - Off;

  //
  //  But truncate buffer at end of FILE.
  //
  //  XXX should handle LARGEFILE
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
  DEBUG ((DEBUG_INFO, "Ext2FsBlockCount = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsBlockCount));
  DEBUG ((DEBUG_INFO, "Ext2FsFirstDataBlock = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsFirstDataBlock));
  DEBUG ((DEBUG_INFO, "Ext2FsLogBlockSize = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsLogBlockSize));
  DEBUG ((DEBUG_INFO, "Ext2FsBlocksPerGroup = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsBlocksPerGroup));
  DEBUG ((DEBUG_INFO, "Ext2FsINodesPerGroup = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsINodesPerGroup));
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
    if (FileSystem->Ext2FsGDSize > 32) {
      DEBUG ((DEBUG_INFO, "  Ext2BGDInodeTablesHi %u\n", Ext2FsGrpDesEntry->Ext2BGDInodeTablesHi));
    }
  }
#endif
}
