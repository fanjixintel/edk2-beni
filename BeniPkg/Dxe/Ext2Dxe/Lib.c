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

#define EXT2_DEBUG 0

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
  DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsBlockCount = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsBlockCount));
  DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsFirstDataBlock = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsFirstDataBlock));
  DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsLogBlockSize = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsLogBlockSize));
  DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsBlocksPerGroup = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsBlocksPerGroup));
  DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsINodesPerGroup = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsINodesPerGroup));
  DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsMagic = 0x%x\n", Volume->SuperBlock.Ext2Fs.Ext2FsMagic));
  DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsRev = %u\n", Volume->SuperBlock.Ext2Fs.Ext2FsRev));

  if (Volume->SuperBlock.Ext2Fs.Ext2FsRev == E2FS_REV1) {
    DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsFirstInode = %u\n",
            Volume->SuperBlock.Ext2Fs.Ext2FsFirstInode));
    DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsInodeSize = %u\n",
            Volume->SuperBlock.Ext2Fs.Ext2FsInodeSize));
    DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsFeaturesCompat = %u\n",
            Volume->SuperBlock.Ext2Fs.Ext2FsFeaturesCompat));
    DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsFeaturesIncompat = %u\n",
            Volume->SuperBlock.Ext2Fs.Ext2FsFeaturesIncompat));
    DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsFeaturesROCompat = %u\n",
            Volume->SuperBlock.Ext2Fs.Ext2FsFeaturesROCompat));
    DEBUG ((DEBUG_INFO, "Volume->Ext2Fs.Ext2FsRsvdGDBlock = %u\n",
            Volume->SuperBlock.Ext2Fs.Ext2FsRsvdGDBlock));
  }

  DEBUG ((DEBUG_INFO, "Volume->Ext2FsGDSize = %u\n", Volume->SuperBlock.Ext2FsGDSize));
  DEBUG ((DEBUG_INFO, "Volume->Ext2FsBlockSize = %u\n", Volume->SuperBlock.Ext2FsBlockSize));
  DEBUG ((DEBUG_INFO, "Volume->Ext2FsFsbtobd = %u\n", Volume->SuperBlock.Ext2FsFsbtobd));
  DEBUG ((DEBUG_INFO, "Volume->Ext2FsNumCylinder = %u\n", Volume->SuperBlock.Ext2FsNumCylinder));
  DEBUG ((DEBUG_INFO, "Volume->Ext2FsNumGrpDesBlock = %u\n", Volume->SuperBlock.Ext2FsNumGrpDesBlock));
  DEBUG ((DEBUG_INFO, "Volume->Ext2FsInodesPerBlock = %u\n", Volume->SuperBlock.Ext2FsInodesPerBlock));
  DEBUG ((DEBUG_INFO, "Volume->Ext2FsInodesTablePerGrp = %u\n", Volume->SuperBlock.Ext2FsInodesTablePerGrp));
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
