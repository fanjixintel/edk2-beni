/**
*  @Package     : BeniPkg
*  @FileName    : Interface.c
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
  Implements Open() of Simple File System Protocol.

  @param[in]  FHand                 File handle of the file serves as a starting reference point.
  @param[out] NewHandle             Handle of the file that is newly opened.
  @param[in]  FileName              File name relative to FHand.
  @param[in]  OpenMode              Open mode.
  @param[in]  Attributes            Attributes to set if the file is created.


  @retval  EFI_INVALID_PARAMETER    The FileName is NULL or the file string is empty.
                                    The OpenMode is not supported.
                                    The Attributes is not the valid attributes.
  @retval  EFI_OUT_OF_RESOURCES     Can not allocate the memory for file string.
  @retval  EFI_SUCCESS              Open the file successfully.
  @return  Others                   The status of open file.

**/
EFI_STATUS
EFIAPI
Ext2Open (
  IN  EFI_FILE_PROTOCOL             *FHand,
  OUT EFI_FILE_PROTOCOL             **NewHandle,
  IN  CHAR16                        *FileName,
  IN  UINT64                        OpenMode,
  IN  UINT64                        Attributes
  )
{
  EFI_STATUS    Status;
  M_EXT2FS      *FileSystem;
  OPEN_FILE     *File;
  FILE          *Fp;
  OPEN_FILE     *NewFile;
  FILE          *NewFp;
  CHAR8         Path[EXT2FS_MAXNAMLEN + 1];
  CHAR8         *Cp;
  CHAR8         *Ncp;
  INT32         Component;
  INODE32       INumber;
  INODE32       ParentInumber;
  INDPTR        Mult;
  INT32         Length;
  UINTN         LinkLength;
  UINTN         Len;
  INT32         Nlinks;
  CHAR8         NameBuf[MAXPATHLEN + 1];
  CHAR8         *Buf;

  Status = EFI_UNSUPPORTED;
  Nlinks = 0;

  //
  // Only valid for READ, Attributes is not used.
  //
  if (OpenMode != EFI_FILE_MODE_READ) {
    return EFI_INVALID_PARAMETER;
  }
  if (NULL == FileName) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // TODO: This is not right. EXT2FS_MAXNAMLEN is the lenght of file name,
  //       not file path.
  //
  if (EXT2FS_MAXNAMLEN < StrLen (FileName)) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (Path, EXT2FS_MAXNAMLEN + 1);
  AsciiSPrint (Path, EXT2FS_MAXNAMLEN + 1, "%s", FileName);
  Cp = Path;
  DEBUG ((DEBUG_ERROR, "Open file \"%a\"\n", Cp));

  NewFile = AllocateZeroPool (sizeof (OPEN_FILE));
  if (NULL == NewFile) {
    DEBUG ((EFI_D_ERROR, "%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }
  File = OFILE_FROM_FHAND (FHand);
  Fp = &File->FileSystemSpecificData;
  FileSystem = File->FileSystemSpecificData.SuperBlockPtr;

  NewFile->Signature   = EXT2_OFILE_SIGNATURE;
  NewFile->BlockIo     = File->BlockIo;
  NewFile->DiskIo      = File->DiskIo;
  NewFile->DiskIo2     = File->DiskIo2;
  NewFp                = &NewFile->FileSystemSpecificData;
  NewFp->SuperBlockPtr = FileSystem;
  NewFp->Buffer        = AllocatePool (File->FileSystemSpecificData.SuperBlockPtr->Ext2FsBlockSize);

  //
  // Calculate indirect block levels.
  //
  //
  // We note that the number of indirect blocks is always
  // a power of 2.  This lets us use shifts and masks instead
  // of divide and remainder and avoinds pulling in the
  // 64bit division routine into the boot code.
  //
  Mult = NINDIR (FileSystem);
  for (Length = 0; Mult != 1; Length++) {
    Mult >>= 1;
  }
  NewFp->NiShift = Length;

  INumber = EXT2_ROOTINO;
  while ('\0' != *Cp) {
    //
    // Remove extra separators (the root directory "/").
    //
    while ('/' == *Cp) {
      Cp++;
    }
    if ('\0' == *Cp) {
      break;
    }

    //
    // Check that current node is a directory.
    //
    if ((Fp->DiskInode.Ext2DInodeMode & EXT2_IFMT) != EXT2_IFDIR) {
      Status = EFI_LOAD_ERROR;
      goto DONE;
    }

    //
    // Get next component of Path Name.
    //
    Ncp = Cp;
    while (((Component = *Cp) != '\0') && ('/' != Component)) {
      Cp++;
    }

    //
    // Look up component in current directory.
    // Save directory INumber in case we find a
    // symbolic link.
    //
    ParentInumber = INumber;
    Status = SearchDirectory (Ncp, (INT32)(Cp - Ncp), File, &INumber);
    if (EFI_ERROR (Status)) {
      goto DONE;
    }

    //
    // Open next component.
    //
    Status = ReadInode (INumber, File);
    if (EFI_ERROR (Status)) {
      goto DONE;
    }

    //
    // Check for symbolic link.
    //
    if ((Fp->DiskInode.Ext2DInodeMode & EXT2_IFMT) == EXT2_IFLNK) {
      LinkLength = Fp->DiskInode.Ext2DInodeSize;
      Len        = AsciiStrLen (Cp);

      if (((LinkLength + Len) > MAXPATHLEN) ||
          ((++Nlinks) > MAXSYMLINKS)) {
        Status = RETURN_LOAD_ERROR;
        goto DONE;
      }

      CopyMem (&NameBuf[LinkLength], Cp, Len + 1);

      if (LinkLength < EXT2_MAXSYMLINKLEN) {
        CopyMem (NameBuf, Fp->DiskInode.Ext2DInodeBlocks, LinkLength);
      } else {
        //
        // Read FILE for symbolic link.
        //
        INDPTR DiskBlock;

        Buf = Fp->Buffer;
        Status = BlockMap (File, (INDPTR)0, &DiskBlock);
        if (EFI_ERROR (Status)) {
          goto DONE;
        }

        Status = MediaReadBlocks (
                    File->BlockIo,
                    FSBTODB (FileSystem, DiskBlock),
                    FileSystem->Ext2FsBlockSize,
                    Buf
                    );
        if (EFI_ERROR (Status)) {
          goto DONE;
        }

        CopyMem (NameBuf, Buf, LinkLength);
      }

      //
      // If relative pathname, restart at parent directory.
      // If absolute pathname, restart at root.
      //
      Cp = NameBuf;
      if ('/' != *Cp) {
        INumber = ParentInumber;
      } else {
        INumber = (INODE32)EXT2_ROOTINO;
      }

      Status = ReadInode (INumber, File);
      if (EFI_ERROR (Status)) {
        goto DONE;
      }
    }
  }

  //
  // Found terminal component.
  //
  Status = EFI_SUCCESS;

  NewFp->SeekPtr = 0; // Reset seek pointer.
  *NewHandle = &NewFile->Handle;

DONE:

  if (EFI_ERROR (Status)) {
    Ext2Close (&NewFile->Handle);
  }

  return Status;
}

/**
  Implements OpenEx() of Simple File System Protocol.

  @param[in]     FHand              File handle of the file serves as a starting reference point.
  @param[out]    NewHandle          Handle of the file that is newly opened.
  @param[in]     FileName           File name relative to FHand.
  @param[in]     OpenMode           Open mode.
  @param[in]     Attributes         Attributes to set if the file is created.
  @param[in|out] Token              A pointer to the token associated with the transaction.

  @retval  EFI_INVALID_PARAMETER    The FileName is NULL or the file string is empty.
                                    The OpenMode is not supported.
                                    The Attributes is not the valid attributes.
  @retval  EFI_OUT_OF_RESOURCES     Can not allocate the memory for file string.
  @retval  EFI_SUCCESS              Open the file successfully.
  @return  Others                   The status of open file.

**/
EFI_STATUS
EFIAPI
Ext2OpenEx (
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
  Get the file's position of the file

  @param[in]  FHand                 The handle of file.
  @param[out] Position              The file's position of the file.

  @retval  EFI_SUCCESS              Get the info successfully.
  @retval  EFI_DEVICE_ERROR         Can not find the OFile for the file.
  @retval  EFI_UNSUPPORTED          The open file is not a file.

**/
EFI_STATUS
EFIAPI
Ext2GetPosition (
  IN  EFI_FILE_PROTOCOL             *FHand,
  OUT UINT64                        *Position
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Get the some types info of the file into Buffer

  @param[in]     FHand              The handle of file.
  @param[in]     Type               The type of the info.
  @param[in|out] BufferSize         Size of Buffer.
  @param[out]    Buffer             Buffer containing volume info.

  @retval  EFI_SUCCESS              Get the info successfully.
  @retval  EFI_DEVICE_ERROR         Can not find the OFile for the file.

**/
EFI_STATUS
EFIAPI
Ext2GetInfo (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN     EFI_GUID                   *Type,
  IN OUT UINTN                      *BufferSize,
  OUT    VOID                       *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Set the some types info of the file into Buffer.

  @param[in]  FHand                 The handle of file.
  @param[in]  Type                  The type of the info.
  @param[in]  BufferSize            Size of Buffer.
  @param[in]  Buffer                Buffer containing volume info.

  @retval  EFI_SUCCESS              Set the info successfully.
  @retval  EFI_DEVICE_ERROR         Can not find the OFile for the file.

**/
EFI_STATUS
EFIAPI
Ext2SetInfo (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  EFI_GUID                      *Type,
  IN  UINTN                         BufferSize,
  IN  VOID                          *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Flushes all data associated with the file handle.

  @param[in]  FHand                 Handle to file to flush

  @retval  EFI_SUCCESS              Flushed the file successfully
  @retval  EFI_WRITE_PROTECTED      The volume is read only
  @retval  EFI_ACCESS_DENIED        The volume is not read only
                                    but the file is read only
  @return  Others                   Flushing of the file is failed

**/
EFI_STATUS
EFIAPI
Ext2Flush (
  IN  EFI_FILE_PROTOCOL             *FHand
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Flushes all data associated with the file handle.

  @param[in]  FHand                 Handle to file to flush.
  @param[in]  Token                 A pointer to the token associated with the transaction.

  @retval  EFI_SUCCESS              Flushed the file successfully.
  @retval  EFI_WRITE_PROTECTED      The volume is read only.
  @retval  EFI_ACCESS_DENIED        The file is read only.
  @return  Others                   Flushing of the file failed.

**/
EFI_STATUS
EFIAPI
Ext2FlushEx (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  EFI_FILE_IO_TOKEN             *Token
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Flushes & Closes the file handle.

  @param[in]  FHand                 Handle to the file to delete.

  @retval  EFI_SUCCESS              Closed the file successfully.

**/
EFI_STATUS
EFIAPI
Ext2Close (
  IN  EFI_FILE_PROTOCOL             *FHand
  )
{
  OPEN_FILE *File = OFILE_FROM_FHAND (FHand);

  if ((NULL != File) && (NULL != File->FileSystemSpecificData.Buffer)) {
    FreePool (File->FileSystemSpecificData.Buffer);
  }
  if (NULL != File) {
    FreePool (File);
  }

  return EFI_SUCCESS;
}

/**
  Deletes the file & Closes the file handle.

  @param[in]  FHand                 Handle to the file to delete.

  @retval  EFI_SUCCESS              Delete the file successfully.
  @retval  EFI_WARN_DELETE_FAILURE  Fail to delete the file.

**/
EFI_STATUS
EFIAPI
Ext2Delete (
  IN  EFI_FILE_PROTOCOL             *FHand
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Set the file's position of the file.

  @param[in]  FHand                 The handle of file
  @param[in]  Position              The file's position of the file

  @retval  EFI_SUCCESS              Set the info successfully
  @retval  EFI_DEVICE_ERROR         Can not find the OFile for the file
  @retval  EFI_UNSUPPORTED          Set a directory with a not-zero position

**/
EFI_STATUS
EFIAPI
Ext2SetPosition (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  UINT64                        Position
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Reads data from a file.

  @param[in]     FHand              The handle of the file.
  @param[in|out] BufferSize         Size of Buffer.
  @param[in]     Buffer             Buffer containing read data.

  @retval  EFI_SUCCESS              Get the file info successfully.
  @retval  EFI_DEVICE_ERROR         Can not find the OFile for the file.
  @retval  EFI_VOLUME_CORRUPTED     The file type of open file is error.
  @return  Others                   An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
Ext2Read (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT UINTN                      *BufferSize,
  OUT    VOID                       *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Reads data from a file.

  @param[in]  FHand                 The handle of the file.
  @param[in]  Token                 A pointer to the token associated with the transaction.

  @retval  EFI_SUCCESS              Get the file info successfully.
  @retval  EFI_DEVICE_ERROR         Can not find the OFile for the file.
  @retval  EFI_VOLUME_CORRUPTED     The file type of open file is error.
  @return  Others                   An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
Ext2ReadEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Writes data to a file.

  @param[in]     FHand              The handle of the file.
  @param[in|out] BufferSize         Size of Buffer.
  @param[in]     Buffer             Buffer containing write data.

  @retval  EFI_SUCCESS              Set the file info successfully.
  @retval  EFI_WRITE_PROTECTED      The disk is write protected.
  @retval  EFI_ACCESS_DENIED        The file is read-only.
  @retval  EFI_DEVICE_ERROR         The OFile is not valid.
  @retval  EFI_UNSUPPORTED          The open file is not a file.
                                    The writing file size is larger than 4GB.
  @return  Others                   An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
Ext2Write (
  IN     EFI_FILE_PROTOCOL      *FHand,
  IN OUT UINTN                  *BufferSize,
  IN     VOID                   *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Writes data to a file.

  @param[in]     FHand              The handle of the file.
  @param[in|out] Token              A pointer to the token associated with the transaction.

  @retval  EFI_SUCCESS              Get the file info successfully.
  @retval  EFI_DEVICE_ERROR         Can not find the OFile for the file.
  @retval  EFI_VOLUME_CORRUPTED     The file type of open file is error.
  @return  Others                   An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
Ext2WriteEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  )
{
  return EFI_UNSUPPORTED;
}

//
// Filesystem interface functions.
//
EFI_FILE_PROTOCOL gExt2FileInterface = {
  EFI_FILE_PROTOCOL_REVISION,
  Ext2Open,
  Ext2Close,
  Ext2Delete,
  Ext2Read,
  Ext2Write,
  Ext2GetPosition,
  Ext2SetPosition,
  Ext2GetInfo,
  Ext2SetInfo,
  Ext2Flush,
  Ext2OpenEx,
  Ext2ReadEx,
  Ext2WriteEx,
  Ext2FlushEx
};
