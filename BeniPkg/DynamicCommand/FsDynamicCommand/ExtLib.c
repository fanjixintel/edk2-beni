/** @file
  ExtLib APIs

Copyright (c) 2006 - 2021, Intel Corporation. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Ext2Fs.h"

/**
  Initialize the EXT2 FileSystem API structure.

  @param[in]  Handle                The handle pointer.
  @param[in]  BlockIo               The pointer to EFI_BLOCK_IO_PROTOCOL.
  @param[in]  DiskIo                The pointer to EFI_DISK_IO_PROTOCOL.
  @param[in]  DiskIo2               The pointer to EFI_DISK_IO2_PROTOCOL.
  @param[in]  FsHandle              The EXT2 FILE system handle.

  @retval  EFI_INVALID_PARAMETER    The Partition handle is not for EXT2/3, or
                                    partition number exceeds the maxium number in Partition handle.
  @retval  EFI_OUT_OF_RESOURCES     Can't allocate memory resource.

**/
EFI_STATUS
EFIAPI
ExtInitFileSystem (
  IN  EFI_HANDLE                    *Handle,
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo,
  IN  EFI_DISK_IO2_PROTOCOL         *DiskIo2,
  OUT EFI_HANDLE                    *FsHandle
  )
{
  EFI_STATUS              Status;
  PEI_EXT_PRIVATE_DATA    *PrivateData;

  if (NULL == FsHandle) {
    Status = EFI_INVALID_PARAMETER;
    goto DONE;
  }

  PrivateData = (PEI_EXT_PRIVATE_DATA *)AllocateZeroPool (sizeof (PEI_EXT_PRIVATE_DATA));
  if (NULL == PrivateData) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  PrivateData->Signature  = FS_EXT_SIGNATURE;
  PrivateData->StartBlock = 0;
  PrivateData->LastBlock  = BlockIo->Media->LastBlock;
  PrivateData->BlockSize  = BlockIo->Media->BlockSize;
  PrivateData->Handle     = Handle;
  PrivateData->BlockIo    = BlockIo;
  PrivateData->DiskIo     = DiskIo;
  PrivateData->DiskIo2    = DiskIo2;

  //
  // Validate Ext2 superblock.
  //
  Status = Ext2SbValidate ((EFI_HANDLE)PrivateData, NULL, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "No valid EXT superblock\n"));
    goto DONE;
  }

  DEBUG ((DEBUG_INFO, "EXT2 detected.\n"));

  *FsHandle = (EFI_HANDLE)PrivateData;

  //
  // Here PrivateData is not freed, the caller should do the free operation.
  //
  return EFI_SUCCESS;

DONE:

  if (PrivateData != NULL) {
    FreePool (PrivateData);
  }

  return Status;
}

/**
  Clean up allocated memory for EXT2 file system.

  @param[in]  FsHandle              EXT2 file system handle.

  @retval  NA

**/
VOID
EFIAPI
ExtCloseFileSystem (
  IN  EFI_HANDLE                    FsHandle
  )
{
  PEI_EXT_PRIVATE_DATA *PrivateData = NULL;

  if (NULL != FsHandle) {
    PrivateData = (PEI_EXT_PRIVATE_DATA *)FsHandle;
  }

  if ((NULL != PrivateData) && (PrivateData->Signature == FS_EXT_SIGNATURE)) {
    FreePool (PrivateData);
  }
}

/**
  Open a file by its name and return its file handle.

  @param[in]     FsHandle         file system handle.
  @param[in]     FileName         The file name to get.
  @param[out]    FileHandle       file handle

  @retval EFI_SUCCESS             The file opened correctly.
  @retval EFI_INVALID_PARAMETER   Parameter is not valid.
  @retval EFI_DEVICE_ERROR        A device error occurred.
  @retval EFI_NOT_FOUND           A requested file cannot be found.
  @retval EFI_OUT_OF_RESOURCES    Insufficant memory resource pool.

**/
EFI_STATUS
EFIAPI
ExtFsOpenFile (
  IN  EFI_HANDLE                                    FsHandle,
  IN  CHAR16                                       *FileName,
  OUT EFI_HANDLE                                   *FileHandle
  )
{
  EFI_STATUS              Status;
  PEI_EXT_PRIVATE_DATA   *PrivateData;
  OPEN_FILE              *OpenFile;
  CHAR8                  *NameBuffer;
  UINTN                   NameSize;

  PrivateData = (PEI_EXT_PRIVATE_DATA *)FsHandle;
  if (PrivateData == NULL || PrivateData->Signature != FS_EXT_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  NameSize = StrSize (FileName);
  NameBuffer = AllocatePool (NameSize);
  Status = UnicodeStrToAsciiStrS (FileName, NameBuffer, NameSize);
  if (EFI_ERROR(Status)) {
    FreePool (NameBuffer);
    return Status;
  }

  OpenFile = (OPEN_FILE *)AllocateZeroPool (sizeof (OPEN_FILE));
  if (OpenFile == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto DONE;
  }

  OpenFile->FileDevData = PrivateData;
  Status = Ext2fsOpen (NameBuffer, OpenFile);
  if (EFI_ERROR (Status)) {
    goto DONE;
  }

  FreePool (NameBuffer);
  *FileHandle = (EFI_HANDLE)OpenFile;

  return EFI_SUCCESS;

DONE:
  if (NameBuffer != NULL) {
    FreePool (NameBuffer);
  }
  if (OpenFile != NULL) {
    FreePool (OpenFile);
  }
  return Status;
}

/**
  Get file size by opened file handle.

  @param[in]     FileHandle       file handle
  @param[out]    FileSize         Pointer to file buffer size.

  @retval EFI_SUCCESS             The file was loaded correctly.
  @retval EFI_INVALID_PARAMETER   Parameter is not valid.

**/
EFI_STATUS
EFIAPI
ExtFsGetFileSize (
  IN  EFI_HANDLE                                  FileHandle,
  OUT UINTN                                      *FileSize
  )
{
  OPEN_FILE              *OpenFile;

  OpenFile = (OPEN_FILE *)FileHandle;
  ASSERT (OpenFile != NULL);
  if (OpenFile == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *FileSize = Ext2fsFileSize (OpenFile);
  return EFI_SUCCESS;
}

/**
  Read file into memory by opened file handle.

  @param[in]     FileHandle       file handle
  @param[out]    FileBufferPtr    Allocated file buffer pointer.
  @param[out]    FileSize         Pointer to file buffer size.

  @retval EFI_SUCCESS             The file was loaded correctly.
  @retval EFI_INVALID_PARAMETER   Parameter is not valid.
  @retval EFI_DEVICE_ERROR        A device error occurred.
  @retval EFI_NOT_FOUND           A requested file cannot be found.
  @retval EFI_OUT_OF_RESOURCES    Insufficant memory resource pool.
  @retval EFI_BUFFER_TOO_SMALL    Buffer size is too small.

**/
EFI_STATUS
EFIAPI
ExtFsReadFile (
  IN  EFI_HANDLE                                  FsHandle,
  IN  EFI_HANDLE                                  FileHandle,
  OUT VOID                                      **FileBufferPtr,
  OUT UINTN                                      *FileSizePtr
  )
{
  OPEN_FILE              *OpenFile;
  VOID                   *FileBuffer;
  UINT32                  FileSize;
  UINT32                  Residual;
  EFI_STATUS              Status;

  OpenFile = (OPEN_FILE *)FileHandle;
  ASSERT (OpenFile != NULL);
  if (OpenFile == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FileSize = Ext2fsFileSize (OpenFile);
  if (FileSize == 0) {
    return EFI_SUCCESS;
  }

  ASSERT (FileBufferPtr != NULL);
  if (FileBufferPtr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FileBuffer = *FileBufferPtr;
  Residual = 0;
  Status = Ext2fsRead (OpenFile, FileBuffer, FileSize, &Residual);
  if (EFI_ERROR (Status) || (Residual != 0)) {
    return EFI_LOAD_ERROR;
  } else {
    *FileSizePtr = FileSize;
  }

  return EFI_SUCCESS;
}

/**
  Close a file by opened file handle

  @param[in]     FileHandle       file handle

  @retval                         none

**/
VOID
EFIAPI
ExtFsCloseFile (
  IN  EFI_HANDLE                                  FileHandle
  )
{
  OPEN_FILE              *OpenFile;

  OpenFile = (OPEN_FILE *)FileHandle;
  if (OpenFile == NULL) {
    return;
  }
  Ext2fsClose (OpenFile);
  FreePool (OpenFile);
}

/**
  List directories or files

  @param[in]     FsHandle         file system handle.
  @param[in]     DirFilePath      directory or file path
  @param[in]     ConsoleOutFunc   redirect output message to a console

  @retval EFI_SUCCESS             list directories of files successfully
  @retval EFI_UNSUPPORTED         this api is not supported
  @retval Others                  an error occurs

**/
EFI_STATUS
EFIAPI
ExtFsListDir (
  IN  EFI_HANDLE                                  FsHandle,
  IN  CHAR16                                     *DirFilePath
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              FileHandle;

  Status = EFI_UNSUPPORTED;

DEBUG_CODE_BEGIN ();
  FileHandle = NULL;
  Status = ExtFsOpenFile (FsHandle, DirFilePath, &FileHandle);
  if (!EFI_ERROR (Status)) {
    Status = Ext2fsLs ((OPEN_FILE *)FileHandle, NULL);
  }
  if (FileHandle != NULL) {
    ExtFsCloseFile (FileHandle);
  }
DEBUG_CODE_END ();

  return Status;
}

/**
  Reads the requested number of blocks from the specified block device.

  The function reads the requested number of blocks from the device. All the
  blocks are read, or an error is returned. If there is no media in the device,
  the function returns EFI_NO_MEDIA.

  @param[in]  FsHandle              The EXT2 FILE system handle.
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
  IN  EFI_HANDLE                    FsHandle,
  IN  EFI_LBA                       StartLBA,
  IN  UINTN                         BufferSize,
  OUT VOID                          *Buffer
  )
{
  PEI_EXT_PRIVATE_DATA *PrivateData = (PEI_EXT_PRIVATE_DATA *)FsHandle;

  return PrivateData->BlockIo->ReadBlocks(
                    PrivateData->BlockIo,
                    PrivateData->BlockIo->Media->MediaId,
                    StartLBA,
                    BufferSize,
                    Buffer
                    );
}
