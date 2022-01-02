/**
*  @Package     : BeniPkg
*  @FileName    : Fs.h
*  @Date        : 20211107
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for file system test.
*
*  @History:
*    20211107: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __FS_H__
#define __FS_H__

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BeniMemLib.h>

#include <Protocol/HiiPackageList.h>
#include <Protocol/ShellDynamicCommand.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>

#include <Guid/FileSystemInfo.h>
#include <Guid/FileInfo.h>

//
// Used for shell display.
//
extern EFI_HII_HANDLE mFsHiiHandle;

//
// Function Prototypes
//

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
  @param[in]  Attributes            Only valid for EFI_FILE_MODE_CREATE, in which case these are the
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
Ext2Open (
  IN  EFI_FILE_PROTOCOL             *FHand,
  OUT EFI_FILE_PROTOCOL             **NewHandle,
  IN  CHAR16                        *FileName,
  IN  UINT64                        OpenMode,
  IN  UINT64                        Attributes
  );

/**
  Closes a specified file handle.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the file
                                    handle to close.

  @retval  EFI_SUCCESS              The file was closed.

**/
EFI_STATUS
EFIAPI
Ext2Close (
  IN  EFI_FILE_PROTOCOL             *FHand
  );

/**
  Close and delete the file handle.

  @param[in]  This                  A pointer to the EFI_FILE_PROTOCOL instance that is the
                                    handle to the file to delete.

  @retval  EFI_SUCCESS              The file was closed and deleted, and the handle was closed.
  @retval  EFI_WARN_DELETE_FAILURE  The handle was closed, but the file was not deleted.

**/
EFI_STATUS
EFIAPI
Ext2Delete (
  IN  EFI_FILE_PROTOCOL             *FHand
  );

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
Ext2Read (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT UINTN                      *BufferSize,
  OUT    VOID                       *Buffer
  );

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
Ext2Write (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT UINTN                      *BufferSize,
  IN     VOID                       *Buffer
  );

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
Ext2SetPosition (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  UINT64                        Position
  );

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
Ext2GetPosition (
  IN  EFI_FILE_PROTOCOL             *FHand,
  OUT UINT64                        *Position
  );

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
Ext2GetInfo (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN     EFI_GUID                   *Type,
  IN OUT UINTN                      *BufferSize,
  OUT    VOID                       *Buffer
  );

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
Ext2SetInfo (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  EFI_GUID                      *Type,
  IN  UINTN                         BufferSize,
  IN  VOID                          *Buffer
  );

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
Ext2Flush (
  IN  EFI_FILE_PROTOCOL             *FHand
  );

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
Ext2OpenEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  OUT    EFI_FILE_PROTOCOL          **NewHandle,
  IN     CHAR16                     *FileName,
  IN     UINT64                     OpenMode,
  IN     UINT64                     Attributes,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  );

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
Ext2ReadEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  );

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
Ext2WriteEx (
  IN     EFI_FILE_PROTOCOL          *FHand,
  IN OUT EFI_FILE_IO_TOKEN          *Token
  );

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
Ext2FlushEx (
  IN  EFI_FILE_PROTOCOL             *FHand,
  IN  EFI_FILE_IO_TOKEN             *Token
  );

/**
  Show EXT2 file system.

  @param  NA

  @retval  NA

**/
VOID
EFIAPI
ShowExt2FileSystem (
  VOID
  );

/**
  Show EXT2 file system.

  @param  NA

  @retval  NA

**/
VOID
EFIAPI
ShowExt2FileSystemEx (
  VOID
  );

/**
  Retrieve HII package list from ImageHandle and publish to HII database.

  @param[in]  ImageHandle           The image handle of the process.

  @return  EFI_HII_HANDLE           The HII handle.

**/
EFI_HII_HANDLE
InitializeHiiPackage (
  IN  EFI_HANDLE                    ImageHandle
  );

/**
  Function for 'fs' command.

  @param[in]  ImageHandle           The image handle.
  @param[in]  SystemTable           The system table.

  @retval  SHELL_SUCCESS            Command completed successfully.
  @retval  SHELL_INVALID_PARAMETER  Command usage error.
  @retval  SHELL_ABORTED            The user aborts the operation.
  @retval  Value                    Unknown error.

**/
SHELL_STATUS
RunFs (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  );

#endif // __FS_H__
