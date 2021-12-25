/**
*  @Package     : BeniPkg
*  @FileName    : OpenVolume.c
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
  Volume->Signature = EXT2_VOLUME_SIGNATURE;
<<<<<<< HEAD
  Volume->Handle    = Handle;
=======
>>>>>>> 08fd28f8ad9569abe05301594c5542bc2107ff6e
  Volume->DiskIo    = DiskIo;
  Volume->DiskIo2   = DiskIo2;
  Volume->BlockIo   = BlockIo;
  Volume->MediaId   = BlockIo->Media->MediaId;
  Volume->ReadOnly  = BlockIo->Media->ReadOnly;
  Volume->VolumeInterface.Revision    = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume  = Ext2OpenVolume;

  //
  // Check to see if there's a file system on the volume
  //
  Status = Ext2OpenDevice (Volume);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Ext2OpenDevice failed. - %r\n", Status));
    goto EXIT;
<<<<<<< HEAD
  } else {
    DumpSBlock (Volume);
=======
>>>>>>> 08fd28f8ad9569abe05301594c5542bc2107ff6e
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
    goto EXIT;
  }

  //
  // Volume installed.
  //
<<<<<<< HEAD
  DEBUG ((EFI_D_ERROR, "Install gEfiSimpleFileSystemProtocolGuid on %p:\n %s\n",
          Volume->Handle,
          ConvertDevicePathToText(DevicePathFromHandle (Volume->Handle), FALSE, FALSE)
          ));
=======
  DEBUG ((EFI_D_INIT, "Installed EXT2 filesystem on %p\n", Handle));
>>>>>>> 08fd28f8ad9569abe05301594c5542bc2107ff6e
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
  return EFI_UNSUPPORTED;
}
