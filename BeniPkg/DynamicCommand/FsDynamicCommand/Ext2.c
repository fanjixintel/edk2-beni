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

#define HOWMANY(x, y)  (((x) + ((y) - 1)) / (y))

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
  CopyMem (&Volume->FileSystem.Ext2Fs, Buffer, sizeof (EXT2FS));

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
  Print (L"    INode count         : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsINodeCount);
  Print (L"    Block count         : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsBlockCount);
  Print (L"    Reserved block count: %d\n", Volume->FileSystem.Ext2Fs.Ext2FsRsvdBlockCount);
  Print (L"    Free block count    : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsFreeBlockCount);
  Print (L"    Free INode count    : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsFreeINodeCount);
  Print (L"    First data block    : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsFirstDataBlock);
  Print (L"    Blocks per group    : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsBlocksPerGroup);
  Print (L"    INode per group     : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsINodesPerGroup);
  Print (L"    Magic               : 0x%x\n", Volume->FileSystem.Ext2Fs.Ext2FsMagic);
  Print (L"    First INode         : %d\n", Volume->FileSystem.Ext2Fs.Ext2FsFirstInode);
  Print (L"    FS GUID             : %g\n", Volume->FileSystem.Ext2Fs.Ext2FsUuid);
  Print (L"    Volume name         : %a\n", Volume->FileSystem.Ext2Fs.Ext2FsVolumeName);
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
  // Check to see if there's a file system on the volume.
  //
  Status = Ext2SbValidate (Volume);
  if (!EFI_ERROR (Status)) {
    //
    // Compute in-memory m_ext2fs values.
    //
    Volume->FileSystem.Ext2FsNumCylinder       =
      HOWMANY (Volume->FileSystem.Ext2Fs.Ext2FsBlockCount - Volume->FileSystem.Ext2Fs.Ext2FsFirstDataBlock,
              Volume->FileSystem.Ext2Fs.Ext2FsBlocksPerGroup);

    Volume->FileSystem.Ext2FsFsbtobd           = (INT32)(Volume->FileSystem.Ext2Fs.Ext2FsLogBlockSize + 10) - (INT32)HighBitSet32 (BlockIo->Media->BlockSize);
    Volume->FileSystem.Ext2FsBlockSize         = MINBSIZE << Volume->FileSystem.Ext2Fs.Ext2FsLogBlockSize;
    Volume->FileSystem.Ext2FsLogicalBlock      = LOG_MINBSIZE + Volume->FileSystem.Ext2Fs.Ext2FsLogBlockSize;
    Volume->FileSystem.Ext2FsQuadBlockOffset   = Volume->FileSystem.Ext2FsBlockSize - 1;
    Volume->FileSystem.Ext2FsBlockOffset       = (UINT32)~Volume->FileSystem.Ext2FsQuadBlockOffset;
    Volume->FileSystem.Ext2FsGDSize            = 32;
    if (Volume->FileSystem.Ext2Fs.Ext2FsFeaturesIncompat & EXT2F_INCOMPAT_64BIT) {
      Volume->FileSystem.Ext2FsGDSize          = Volume->FileSystem.Ext2Fs.Ext2FsGDSize;
    }
    Volume->FileSystem.Ext2FsNumGrpDesBlock    =
      HOWMANY (Volume->FileSystem.Ext2FsNumCylinder, Volume->FileSystem.Ext2FsBlockSize / Volume->FileSystem.Ext2FsGDSize);
    Volume->FileSystem.Ext2FsInodesPerBlock    = Volume->FileSystem.Ext2FsBlockSize / Volume->FileSystem.Ext2Fs.Ext2FsInodeSize;
    Volume->FileSystem.Ext2FsInodesTablePerGrp = Volume->FileSystem.Ext2Fs.Ext2FsINodesPerGroup / Volume->FileSystem.Ext2FsInodesPerBlock;

    ShowSuperBlockInfo (Volume);
  } else {
    FreePool (Volume);
  }

  return Status;
}

/**
  Opens a new file relative to the source file's location.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to the source location. This would typically be an open
                                    handle to a directory.
  @param[out] NewHandle             A pointer to the location to return the opened handle for the new
                                    file.
  @param[in]  FileName              The Null-terminated string of the name of the file to be opened.
                                    The file name may contain the following path modifiers: "\", ".",
                                    and "..".
  @param[in]  OpenMode              The mode to open the file. The only valid combinations that the
                                    file may be opened with are: Read, Read/Write, or Create/Read/Write.
  @param[in]                        Attributes Only valid for EFI_FILE_MODE_CREATE, in which case these are the
                                    attribute bits for the newly created file.

  @retval  EFI_SUCCESS              The file was opened.
  @retval  EFI_NOT_FOUND            The specified file could not be found on the device.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_MEDIA_CHANGED        The device has a different medium in it or the medium is no
                                    longer supported.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED      An attempt was made to create a file, or open a file for write
                                    when the media is write-protected.
  @retval  EFI_ACCESS_DENIED        The service denied access to the file.
  @retval  EFI_OUT_OF_RESOURCES     Not enough resources were available to open the file.
  @retval  EFI_VOLUME_FULL          The volume is full.

**/
EFI_STATUS
EFIAPI
FatOpen (
  IN  EFI_FILE_PROTOCOL             *FHand,
  OUT EFI_FILE_PROTOCOL             **NewHandle,
  IN  CHAR16                        *FileName,
  IN  UINT64                        OpenMode,
  IN  UINT64                        Attributes
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Closes a specified file handle.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to close.

  @retval  EFI_SUCCESS              The file was closed.

**/
EFI_STATUS
EFIAPI
FatClose (
  IN  EFI_FILE_PROTOCOL             *FHand
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Close and delete the file handle.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the
                                    handle to the file to delete.

  @retval  EFI_SUCCESS              The file was closed and deleted, and the handle was closed.
  @retval  EFI_WARN_DELETE_FAILURE  The handle was closed, but the file was not deleted.

**/
EFI_STATUS
EFIAPI
FatDelete (
  IN  EFI_FILE_PROTOCOL             *FHand
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Reads data from a file.

  @param[in]     This               A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to read data from.
  @param[in|out] BufferSize         On input, the size of the Buffer. On output, the amount of data
                                    returned in Buffer. In both cases, the size is measured in bytes.
  @param[out]    Buffer             The buffer into which the data is read.

  @retval  EFI_SUCCESS              Data was read.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_DEVICE_ERROR         An attempt was made to read from a deleted file.
  @retval  EFI_DEVICE_ERROR         On entry, the current file position is beyond the end of the file.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_BUFFER_TOO_SMALL     The BufferSize is too small to read the current directory
                                    entry. BufferSize has been updated with the size
                                    needed to complete the request.

**/
EFI_STATUS
EFIAPI
FatRead (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT UINTN                      *BufferSize,
  OUT    VOID                       *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Writes data to a file.

  @param[in]     This               A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to write data to.
  @param[in|out] BufferSize         On input, the size of the Buffer. On output, the amount of data
                                    actually written. In both cases, the size is measured in bytes.
  @param[in]     Buffer             The buffer of data to write.

  @retval  EFI_SUCCESS              Data was written.
  @retval  EFI_UNSUPPORTED          Writes to open directory files are not supported.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_DEVICE_ERROR         An attempt was made to write to a deleted file.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED      The file or medium is write-protected.
  @retval  EFI_ACCESS_DENIED        The file was opened read only.
  @retval  EFI_VOLUME_FULL          The volume is full.

**/
EFI_STATUS
EFIAPI
FatWrite (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT UINTN                      *BufferSize,
  IN     VOID                       *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Sets a file's current position.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the
                                    file handle to set the requested position on.
  @param[in]  Position              The byte position from the start of the file to set.

  @retval  EFI_SUCCESS              The position was set.
  @retval  EFI_UNSUPPORTED          The seek request for nonzero is not valid on open
                                    directories.
  @retval  EFI_DEVICE_ERROR         An attempt was made to set the position of a deleted file.

**/
EFI_STATUS
EFIAPI
FatSetPosition (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  UINT64                        Position
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Returns a file's current position.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to get the current position on.
  @param[out] Position              The address to return the file's current position value.

  @retval  EFI_SUCCESS              The position was returned.
  @retval  EFI_UNSUPPORTED          The request is not valid on open directories.
  @retval  EFI_DEVICE_ERROR         An attempt was made to get the position from a deleted file.

**/
EFI_STATUS
EFIAPI
FatGetPosition (
  IN  EFI_FILE_PROTOCOL             *FHand,
  OUT UINT64                        *Position
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Returns information about a file.

  @param[in]     This               A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle the requested information is for.
  @param[in]     InformationType    The type identifier for the information being requested.
  @param[in|out] BufferSize         On input, the size of Buffer. On output, the amount of data
                                    returned in Buffer. In both cases, the size is measured in bytes.
  @param[out]    Buffer             A pointer to the data buffer to return. The buffer's type is
                                    indicated by InformationType.

  @retval  EFI_SUCCESS              The information was returned.
  @retval  EFI_UNSUPPORTED          The InformationType is not known.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_BUFFER_TOO_SMALL     The BufferSize is too small to read the current directory entry.
                                    BufferSize has been updated with the size needed to complete
                                    the request.

**/
EFI_STATUS
EFIAPI
FatGetInfo (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN     EFI_GUID                   *Type,
  IN OUT UINTN                      *BufferSize,
  OUT    VOID                       *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Sets information about a file.

  @param[in]  File                  A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle the information is for.
  @param[in]  InformationType       The type identifier for the information being set.
  @param[in]  BufferSize            The size, in bytes, of Buffer.
  @param[in]  Buffer                A pointer to the data buffer to write. The buffer's type is
                                    indicated by InformationType.

  @retval  EFI_SUCCESS              The information was set.
  @retval  EFI_UNSUPPORTED          The InformationType is not known.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED      InformationType is EFI_FILE_INFO_ID and the media is
                                    read-only.
  @retval  EFI_WRITE_PROTECTED      InformationType is EFI_FILE_PROTOCOL_SYSTEM_INFO_ID
                                    and the media is read only.
  @retval  EFI_WRITE_PROTECTED      InformationType is EFI_FILE_SYSTEM_VOLUME_LABEL_ID
                                    and the media is read-only.
  @retval  EFI_ACCESS_DENIED        An attempt is made to change the name of a file to a
                                    file that is already present.
  @retval  EFI_ACCESS_DENIED        An attempt is being made to change the EFI_FILE_DIRECTORY
                                    Attribute.
  @retval  EFI_ACCESS_DENIED        An attempt is being made to change the size of a directory.
  @retval  EFI_ACCESS_DENIED        InformationType is EFI_FILE_INFO_ID and the file was opened
                                    read-only and an attempt is being made to modify a field
                                    other than Attribute.
  @retval  EFI_VOLUME_FULL          The volume is full.
  @retval  EFI_BAD_BUFFER_SIZE      BufferSize is smaller than the size of the type indicated
                                    by InformationType.

**/
EFI_STATUS
EFIAPI
FatSetInfo (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  EFI_GUID                      *Type,
  IN  UINTN                         BufferSize,
  IN  VOID                          *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Flushes all modified data associated with a file to a device.

  @param[in]                        This A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to flush.

  @retval  EFI_SUCCESS              The data was flushed.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED      The file or medium is write-protected.
  @retval  EFI_ACCESS_DENIED        The file was opened read-only.
  @retval  EFI_VOLUME_FULL          The volume is full.

**/
EFI_STATUS
EFIAPI
FatFlush (
  IN  EFI_FILE_PROTOCOL             *FHand
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Opens a new file relative to the source directory's location.

  @param[in]     This               A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to the source location.
  @param[out]    NewHandle          A pointer to the location to return the opened handle for the new
                                    file.
  @param[in]     FileName           The Null-terminated string of the name of the file to be opened.
                                    The file name may contain the following path modifiers: "\", ".",
                                    and "..".
  @param[in]     OpenMode           The mode to open the file. The only valid combinations that the
                                    file may be opened with are: Read, Read/Write, or Create/Read/Write.
  @param[in]     Attributes         Only valid for EFI_FILE_MODE_CREATE, in which case these are the
                                    attribute bits for the newly created file.
  @param[in|out] Token              A pointer to the token associated with the transaction.

  @retval EFI_SUCCESS               If Event is NULL (blocking I/O): The data was read successfully.
                                    If Event is not NULL (asynchronous I/O): The request was successfully
                                                                        queued for processing.
  @retval  EFI_NOT_FOUND            The specified file could not be found on the device.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_MEDIA_CHANGED        The device has a different medium in it or the medium is no
                                    longer supported.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED      An attempt was made to create a file, or open a file for write
                                    when the media is write-protected.
  @retval  EFI_ACCESS_DENIED        The service denied access to the file.
  @retval  EFI_OUT_OF_RESOURCES     Not enough resources were available to open the file.
  @retval  EFI_VOLUME_FULL          The volume is full.

**/
EFI_STATUS
EFIAPI
FatOpenEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  OUT    EFI_FILE_PROTOCOL          **NewHandle,
  IN     CHAR16                     *FileName,
  IN     UINT64                     OpenMode,
  IN     UINT64                     Attributes,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Reads data from a file.

  @param[in]     This               A pointer to the EFI_FILE_PROTOCOL instance that is the file handle to read data from.
  @param[in|out] Token              A pointer to the token associated with the transaction.

  @retval EFI_SUCCESS               If Event is NULL (blocking I/O): The data was read successfully.
                                    If Event is not NULL (asynchronous I/O): The request was successfully
                                    queued for processing.

  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_DEVICE_ERROR         An attempt was made to read from a deleted file.
  @retval  EFI_DEVICE_ERROR         On entry, the current file position is beyond the end of the file.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_OUT_OF_RESOURCES     Unable to queue the request due to lack of resources.

**/
EFI_STATUS
EFIAPI
FatReadEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Writes data to a file.

  @param[in]     This               A pointer to the EFI_FILE_PROTOCOL instance that is the file handle to write data to.
  @param[in|out] Token              A pointer to the token associated with the transaction.

  @retval  EFI_SUCCESS              If Event is NULL (blocking I/O): The data was read successfully.
                                    If Event is not NULL (asynchronous I/O): The request was successfully
                                    queued for processing.

  @retval  EFI_UNSUPPORTED          Writes to open directory files are not supported.
  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_DEVICE_ERROR         An attempt was made to write to a deleted file.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED      The file or medium is write-protected.
  @retval  EFI_ACCESS_DENIED        The file was opened read only.
  @retval  EFI_VOLUME_FULL          The volume is full.
  @retval  EFI_OUT_OF_RESOURCES     Unable to queue the request due to lack of resources.

**/
EFI_STATUS
EFIAPI
FatWriteEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Flushes all modified data associated with a file to a device.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to flush.
  @param[in]  Token                 A pointer to the token associated with the transaction.

  @retval  EFI_SUCCESS              If Event is NULL (blocking I/O): The data was read successfully.
                                    If Event is not NULL (asynchronous I/O): The request was successfully
                                    queued for processing.

  @retval  EFI_NO_MEDIA             The device has no medium.
  @retval  EFI_DEVICE_ERROR         The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED     The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED      The file or medium is write-protected.
  @retval  EFI_ACCESS_DENIED        The file was opened read-only.
  @retval  EFI_VOLUME_FULL          The volume is full.
  @retval  EFI_OUT_OF_RESOURCES     Unable to queue the request due to lack of resources.

**/
EFI_STATUS
EFIAPI
FatFlushEx (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  EFI_FILE_IO_TOKEN             *Token
  )
{
  return EFI_UNSUPPORTED;
}

//
// Filesystem interface functions.
//
EFI_FILE_PROTOCOL               FatFileInterface = {
  EFI_FILE_PROTOCOL_REVISION,
  FatOpen,
  FatClose,
  FatDelete,
  FatRead,
  FatWrite,
  FatGetPosition,
  FatSetPosition,
  FatGetInfo,
  FatSetInfo,
  FatFlush,
  FatOpenEx,
  FatReadEx,
  FatWriteEx,
  FatFlushEx
};

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