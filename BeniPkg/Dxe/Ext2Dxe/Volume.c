/**
*  @Package     : BeniPkg
*  @FileName    : Volume.c
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
  Allocates volume structure, detects EXT2 file system, installs protocol.

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

  Status = EFI_UNSUPPORTED;
  Volume = NULL;

  //
  // Allocate a volume structure.
  //
  Volume = AllocateZeroPool (sizeof (EXT2_VOLUME));
  if (NULL == Volume) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize the structure.
  //
  Volume->Signature                   = EXT2_VOLUME_SIGNATURE;
  Volume->Handle                      = Handle;
  Volume->DiskIo                      = DiskIo;
  Volume->DiskIo2                     = DiskIo2;
  Volume->BlockIo                     = BlockIo;
  Volume->MediaId                     = BlockIo->Media->MediaId;
  Volume->ReadOnly                    = BlockIo->Media->ReadOnly;
  Volume->VolumeInterface.Revision    = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume  = Ext2OpenVolume;

  //
  // Check to see if there's a file system on the volume.
  //
  Status = Ext2OpenDevice (Volume);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Ext2OpenDevice failed. - %r\n", Status));
    goto EXIT;
  } else {
    DumpSBlock (Volume);
  }

  //
  // Install our protocol interfaces on the device's handle.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Volume->Handle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Volume->VolumeInterface,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Install gEfiSimpleFileSystemProtocolGuid failed. - %r\n", Status));
    goto EXIT;
  }

  //
  // Volume installed.
  //
  DEBUG ((EFI_D_INFO, "Install gEfiSimpleFileSystemProtocolGuid on 0x%p:\n\t%s\n",
          Volume->Handle,
          ConvertDevicePathToText(DevicePathFromHandle (Volume->Handle), FALSE, FALSE)
          ));
  Volume->Valid = TRUE;

EXIT:

  if (EFI_ERROR (Status)) {
    Ext2FreeVolume (Volume);
  }

  return Status;
}

/**
  Free volume structure.

  @param[in]  Volume                The volume structure to be freed.

  @retval  NA

**/
VOID
Ext2FreeVolume (
  IN  EXT2_VOLUME                   *Volume
  )
{
  if (NULL != Volume) {
    FreePool (Volume);
  }
}

/**
  Called by Ext2DriverBindingStop(), Abandon the volume.

  @param[in]  Volume                The volume to be abandoned.

  @retval  EFI_SUCCESS              Abandoned the volume successfully.
  @return  Others                   Can not uninstall the protocol interfaces.

**/
EFI_STATUS
Ext2AbandonVolume (
  IN  EXT2_VOLUME                   *Volume
  )
{
  return EFI_SUCCESS;
}

/**
  Implements Simple File System Protocol interface function OpenVolume().

  @param[in]  This                  Calling context.
  @param[out] File                  the Root Directory of the volume.

  @retval  EFI_OUT_OF_RESOURCES     Can not allocate the memory.
  @retval  EFI_VOLUME_CORRUPTED     The FAT type is error.
  @retval  EFI_SUCCESS              Open the volume successfully.

**/
EFI_STATUS
EFIAPI
Ext2OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *This,
  OUT EFI_FILE_PROTOCOL                  **File
  )
{
  EFI_STATUS    Status;
  EXT2_VOLUME   *Volume;
  OPEN_FILE     *OFile;
  FILE          *Fp;
  M_EXT2FS      *FileSystem;

  Status        = EFI_UNSUPPORTED;
  Volume        = NULL;
  OFile         = NULL;
  Fp            = NULL;
  FileSystem    = NULL;

  Volume = VOLUME_FROM_VOL_INTERFACE (This);
  Fp = &Volume->Root.FileSystemSpecificData;
  Fp->SuperBlockPtr = (M_EXT2FS *)&Volume->SuperBlock;;
  FileSystem = Fp->SuperBlockPtr;

  //
  // Compute in-memory m_ext2fs values.
  //
  FileSystem->Ext2FsNumCylinder       =
    HOWMANY ((FileSystem->Ext2Fs.Ext2FsBlockCount - FileSystem->Ext2Fs.Ext2FsFirstDataBlock),
             FileSystem->Ext2Fs.Ext2FsBlocksPerGroup);

  FileSystem->Ext2FsFsbtobd           = (INT32)(FileSystem->Ext2Fs.Ext2FsLogBlockSize + 10) -
                                        (INT32)HighBitSet32 (Volume->BlockIo->Media->BlockSize);
  FileSystem->Ext2FsBlockSize         = MINBSIZE << FileSystem->Ext2Fs.Ext2FsLogBlockSize;
  FileSystem->Ext2FsLogicalBlock      = LOG_MINBSIZE + FileSystem->Ext2Fs.Ext2FsLogBlockSize;
  FileSystem->Ext2FsQuadBlockOffset   = FileSystem->Ext2FsBlockSize - 1;
  FileSystem->Ext2FsBlockOffset       = (UINT32)~FileSystem->Ext2FsQuadBlockOffset;
  FileSystem->Ext2FsGDSize            = 32; // sizeof (EXT2GD) 32 or 64.
  if (FileSystem->Ext2Fs.Ext2FsFeaturesIncompat & EXT2F_INCOMPAT_64BIT) {
    FileSystem->Ext2FsGDSize          = FileSystem->Ext2Fs.Ext2FsGDSize;
  }
  FileSystem->Ext2FsNumGrpDesBlock    =
    HOWMANY (FileSystem->Ext2FsNumCylinder, FileSystem->Ext2FsBlockSize / FileSystem->Ext2FsGDSize);
  FileSystem->Ext2FsInodesPerBlock    = FileSystem->Ext2FsBlockSize / FileSystem->Ext2Fs.Ext2FsInodeSize;
  FileSystem->Ext2FsInodesTablePerGrp = FileSystem->Ext2Fs.Ext2FsINodesPerGroup / FileSystem->Ext2FsInodesPerBlock;

  DumpSBlock (Volume);

  //
  // Alloc a block sized buffer used for all FileSystem transfers.
  //
  Fp->Buffer = AllocatePool (FileSystem->Ext2FsBlockSize);

  Status = ReadGDBlock (Volume);
  if (EFI_ERROR (Status)) {
    goto DONE;
  }

  DumpGroupDesBlock (Volume);

  CopyMem (&(Volume->Root.Handle), &gExt2FileInterface, sizeof (EFI_FILE_PROTOCOL));
  *File = &Volume->Root.Handle;

DONE:

  return Status;
}
