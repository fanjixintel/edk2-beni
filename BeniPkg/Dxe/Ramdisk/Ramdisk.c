/**
*  @Package     : BeniPkg
*  @FileName    : Ramdisk.c
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This driver is used to construct a RAM disk in UEFI environment.
*
*  @History:
*    20211004: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Ramdisk.h"

//
// EFI device path template.
//
static MEDIA_RAM_DISK_DEVICE_PATH gRamDiskDeviceNodeTemplate =
{
  {
    MEDIA_DEVICE_PATH,
    MEDIA_RAM_DISK_DP,
    {
      (UINT8) (sizeof (MEDIA_RAM_DISK_DEVICE_PATH)),
      (UINT8) ((sizeof (MEDIA_RAM_DISK_DEVICE_PATH)) >> 8)
    }
  }
};

//
// EFI_VIRTUAL_DISK_GUID
//
EFI_GUID gDiskTpye = { 0x77AB535A, 0x45FC, 0x624B, { 0x55, 0x60, 0xF7, 0xB2, 0x81, 0xD1, 0xF9, 0x6E }};

//
// See <Microsoft Extensible Firmware Initiative FAT32 File System Specification> for details.
//
/*
* This is the table for FAT16 drives. NOTE that this table includes
* entries for disk sizes larger than 512 MB even though typically
* only the entries for disks < 512 MB in size are used.
* The way this table is accessed is to look for the first entry
* in the table for which the disk size is less than or equal
* to the DiskSize field in that table entry. For this table to
* work properly BPB_RsvdSecCnt must be 1, BPB_NumFATs
* must be 2, and BPB_RootEntCnt must be 512. Any of these values
* being different may require the first table entries DiskSize value
* to be changed otherwise the cluster count may be to low for FAT16.
*/
DISK_SIZE_TO_SECTOR_PER_CLUSTER DskTableFAT16 [] = {
  { 8400,       0 },  /* disks up to 4.1 MB, the 0 value for SecPerClusVal trips an error */
  { 32680,      2 },  /* disks up to 16 MB, 1k cluster */
  { 262144,     4 },  /* disks up to 128 MB, 2k cluster */
  { 524288,     8 },  /* disks up to 256 MB, 4k cluster */
  { 1048576,    16},  /* disks up to 512 MB, 8k cluster */
  /* The entries after this point are not used unless FAT16 is forced */
  { 2097152,    32},  /* disks up to 1 GB, 16k cluster */
  { 4194304,    64},  /* disks up to 2 GB, 32k cluster */
  { 0xFFFFFFFF, 0 }   /* any disk greater than 2GB, 0 value for SecPerClusVal trips an error */
};

//
// The data in the first boot sector.
//
static BOOT_SECTOR gBootSector =
{
  {0xEB, 0x00, 0x90},                         // BS_jmpBoot
  {'R', 'A', 'M', '_', 'D', 'I', 'S', 'K'},   // BS_OEMName, "RAM_DISK"
  512,                                        // BPB_BytsPerSec
  0,                                          // BPB_SecPerClus
  1,                                          // BPB_RsvdSecCnt
  2,                                          // BPB_NumFATs
  512,                                        // BPB_RootEntCnt
  0,                                          // BPB_TotSec16
  0xF8,                                       // BPB_Media
  0,                                          // BPB_FATSz16
  0,                                          // BPB_SecPerTrk
  0,                                          // BPB_NumHeads
  0,                                          // BPB_HiddSec
  0,                                          // BPB_TotSec32
  0,                                          // BS_DrvNum
  0,                                          // BS_Reserved1
  0x29,                                       // BS_BootSig
  0,                                          // BS_VolID
  {'N','O',' ','N','A','M','E',' ',' ',' '},  // BS_VolLab, "NO NAME   "
  {'F','A','T','1','6',' ',' ',' '}           // BS_FilSysType, "FAT16   "
};

/**
  Get the BPB_SecPerClus depending on the total sectors in the disk.

  @param[in] DskSize                Total sectors.

  @retval UINT8                     The BPB_SecPerClus.

**/
UINT8
EFIAPI
GetSecPerClus (
  IN  UINT32                        DskSize
  )
{
  UINTN Index = 0;

  while (DskTableFAT16[Index].DiskSize != 0xFFFFFFFF) {
    if (DskSize <= DskTableFAT16[Index].DiskSize) {
      return DskTableFAT16[Index].SecPerClusVal;
    }
    Index++;
  }

  return 0;
}

/**
  Format the allocateed memory to FAT16 file system.
  See <Microsoft Extensible Firmware Initiative FAT32 File System Specification> for details.

  @param[in]  StartAddress          Start address of the ramdisk in memroy.
  @param[in]  Size                  Size of the ramdisk.

  @retval  NA

**/
VOID
EFIAPI
FormatRamdisk (
  IN  VOID                          *StartAddress,
  IN  UINT32                        Size
  )
{
  UINT32        DskSize;
  UINT32        RootDirSectors;
  UINT32        FatSize;
  UINT32        TmpVal1;
  UINT32        TmpVal2;
  UINT8         *Fat1;
  UINT8         *Fat2;

  //
  // The boot signature is AA55h.
  //
  gBootSector.BS_Sig = 0xAA55;

  //
  // Get the count of sectors and appropriate cluster size.
  //
  DskSize = Size / gBootSector.BPB_BytsPerSec;
  gBootSector.BPB_SecPerClus = GetSecPerClus (DskSize);
  ASSERT (gBootSector.BPB_SecPerClus != 0);

  //
  // Compute how many root directory sectors are needed.
  //
  RootDirSectors = ((gBootSector.BPB_RootEntCnt * 32) + \
                    (gBootSector.BPB_BytsPerSec - 1)) / gBootSector.BPB_BytsPerSec;

  //
  // Compute how many sectors are required per FAT.
  //
  TmpVal1 = DskSize - (gBootSector.BPB_RsvdSecCnt + RootDirSectors);
  TmpVal2 = (256 * gBootSector.BPB_SecPerClus) + gBootSector.BPB_NumFATs;
  FatSize = (TmpVal1 + TmpVal2 - 1) / TmpVal2;
  ASSERT (FatSize <= 0xFFFF);

  //
  // Store the total sectors and fat size values.
  //
  if(0x10000 <= DskSize) {
    gBootSector.BPB_TotSec32 = DskSize;
  } else {
    gBootSector.BPB_TotSec16 = (UINT16)DskSize;
  }
  gBootSector.BPB_FATSz16 = (UINT16)FatSize;

  //
  // Clear the whole disk (In fact you don't need to clear them all, just for simplicity).
  //
  ZeroMem (StartAddress, Size);

  //
  // Write the boot sector to the first sector of the ramdisk.
  //
  CopyMem (StartAddress, &gBootSector, 512);

  //
  // Initialize the FAT data structure.
  // The number of FAT data structure is BPB_NumFATs, here it is 2.
  //
  Fat1 = (UINT8 *)StartAddress + gBootSector.BPB_RsvdSecCnt * 512;
  Fat2 = (UINT8 *)StartAddress + (gBootSector.BPB_RsvdSecCnt + FatSize) * 512;

  Fat1[0] = gBootSector.BPB_Media;
  Fat1[1] = 0xFF;

  Fat2[0] = gBootSector.BPB_Media;
  Fat2[1] = 0xFF;
}

/**
  Read BufferSize bytes from Lba into Buffer.

  @param[in]  This                  Indicates a pointer to the calling context.
  @param[in]  MediaId               Id of the media, changes every time the media is replaced.
  @param[in]  Lba                   The starting Logical Block Address to read from.
  @param[in]  BufferSize            Size of Buffer, must be a multiple of device block size.
  @param[in]  Buffer                A pointer to the destination buffer for the data. The caller is
                                    responsible for either having implicit or explicit ownership of the buffer.

  @retval  EFI_SUCCESS              The data was read correctly from the device.
  @retval  EFI_DEVICE_ERROR         The device reported an error while performing the read.
  @retval  EFI_NO_MEDIA             There is no media in the device.
  @retval  EFI_MEDIA_CHANGED        The MediaId does not matched the current device.
  @retval  EFI_BAD_BUFFER_SIZE      The Buffer was not a multiple of the block size of the device.
  @retval  EFI_INVALID_PARAMETER    The read request contains LBAs that are not valid,
                                    or the buffer is not on proper alignment.

**/
EFI_STATUS
EFIAPI
RamDiskReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL         *This,
  IN  UINT32                        MediaId,
  IN  EFI_LBA                       Lba,
  IN  UINTN                         BufferSize,
  OUT VOID                          *Buffer
  )
{
  EFI_BLOCK_IO_MEDIA           *Media;
  RAM_DISK_DAV                 *RamDiskDev;
  EFI_PHYSICAL_ADDRESS         RamDiskLba;

  Media = This->Media;

  if ((BufferSize % Media->BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  if (Lba > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  if((Lba + BufferSize / Media->BlockSize - 1) > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  RamDiskDev = RAM_DISK_FROM_THIS (This);
  RamDiskLba = RamDiskDev->StartingAddr + MultU64x32 (Lba,Media->BlockSize);
  CopyMem (Buffer, (VOID*)RamDiskLba, BufferSize);

  return EFI_SUCCESS;
}

/**
  Write BufferSize bytes from Lba into Buffer.

  @param[in]  This                  Indicates a pointer to the calling context.
  @param[in]  MediaId               The media ID that the write request is for.
  @param[in]  Lba                   The starting logical block address to be written. The caller is
                                    responsible for writing to only legitimate locations.
  @param[in]  BufferSize            Size of Buffer, must be a multiple of device block size.
  @param[in]  Buffer                A pointer to the source buffer for the data.

  @retval  EFI_SUCCESS              The data was written correctly to the device.
  @retval  EFI_WRITE_PROTECTED      The device can not be written to.
  @retval  EFI_DEVICE_ERROR         The device reported an error while performing the write.
  @retval  EFI_NO_MEDIA             There is no media in the device.
  @retval  EFI_MEDIA_CHNAGED        The MediaId does not matched the current device.
  @retval  EFI_BAD_BUFFER_SIZE      The Buffer was not a multiple of the block size of the device.
  @retval  EFI_INVALID_PARAMETER    The write request contains LBAs that are not valid,
                                    or the buffer is not on proper alignment.

**/
EFI_STATUS
EFIAPI
RamDiskWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL         *This,
  IN  UINT32                        MediaId,
  IN  EFI_LBA                       Lba,
  IN  UINTN                         BufferSize,
  IN  VOID                          *Buffer
  )
{
  EFI_BLOCK_IO_MEDIA           *Media;
  RAM_DISK_DAV                 *RamDiskDev;
  EFI_PHYSICAL_ADDRESS         RamDiskLba;

  Media = This->Media;
  if (Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  if ((BufferSize % Media->BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  if (Lba > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  if ((Lba + BufferSize / Media->BlockSize - 1) > Media->LastBlock) {
    return EFI_DEVICE_ERROR;
  }

  RamDiskDev = RAM_DISK_FROM_THIS (This);
  RamDiskLba = RamDiskDev->StartingAddr + MultU64x32 (Lba, Media->BlockSize);
  CopyMem ((VOID*)RamDiskLba, Buffer, BufferSize);

  return EFI_SUCCESS;
}

/**
  Flush the Block Device.

  @param[in]  This                  Indicates a pointer to the calling context.

  @retval  EFI_SUCCESS              All outstanding data was written to the device.
  @retval  EFI_DEVICE_ERROR         The device reported an error while writting back the data.
  @retval  EFI_NO_MEDIA             There is no media in the device.

**/
EFI_STATUS
EFIAPI
RamDiskFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL         *This
  )
{
  //
  // It is the memory as the storage media, so no need to flush.
  //
  return EFI_SUCCESS;
}

/**
  Initialize ramdisk and install relative protocols.

  @param[in]  ImageHandle           Image handle this driver.
  @param[in]  SystemTable           Pointer to the System Table.

  @retval  EFI_SUCCESS              Driver executed successfully.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
RamDiskDriverEntry (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  EFI_STATUS                        Status;
  RAM_DISK_DAV                      *RamDiskDev;
  MEDIA_RAM_DISK_DEVICE_PATH        *RamDiskDevNode;
  UINT32                            RamDiskSize;
  UINT32                            NumPages;
  UINT32                            BlockSize;

  BENI_MODULE_START

  //
  // Set the disk size.
  //
  RamDiskSize = DEFAULT_DISK_SIZE * 1024 * 1024;
  BlockSize   = 512;

  //
  // Allocate storage for ramdisk device info on the heap.
  //
  RamDiskDev = AllocateZeroPool (sizeof (RAM_DISK_DAV));
  if (NULL == RamDiskDev) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Compute the number of 4KB pages needed by the ramdisk and allocate the memory.
  //
  NumPages = RamDiskSize / EFI_PAGE_SIZE;
  if (NumPages % RamDiskSize) {
    NumPages++;
  }

  DEBUG ((EFI_D_ERROR, "[BENI]NumPages: %d\n", NumPages));

  //
  // We uss AllocatePages instead of AllocatePool.
  //
  Status = gBS->AllocatePages (
                  AllocateAnyPages,
                  EfiBootServicesData,
                  NumPages,
                  &RamDiskDev->StartingAddr
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    FreePool (RamDiskDev);
    return Status;
  }

  RamDiskDevNode = AllocateCopyPool (
                      sizeof (MEDIA_RAM_DISK_DEVICE_PATH),
                      &gRamDiskDeviceNodeTemplate
                      );
  if (NULL == RamDiskDevNode) {
    DEBUG ((EFI_D_ERROR, "[BENI]%a %d Out of memory\n", __FUNCTION__, __LINE__));
    return EFI_OUT_OF_RESOURCES;
  }
  WriteUnaligned64 (
    (UINT64 *)&(RamDiskDevNode->StartingAddr[0]),
    (UINT64)RamDiskDev->StartingAddr
    );
  WriteUnaligned64 (
    (UINT64 *)&(RamDiskDevNode->EndingAddr[0]),
    (UINT64)RamDiskDev->StartingAddr + RamDiskSize - 1
    );
  CopyGuid (&RamDiskDevNode->TypeGuid, &gDiskTpye);
  RamDiskDevNode->Instance = 1;

  //
  // Initialize the ramdisk's device data.
  //
  RamDiskDev->Signature              = RAM_DISK_DATA_SIGNATURE;
  RamDiskDev->BlkIo.Revision         = EFI_BLOCK_IO_INTERFACE_REVISION;
  RamDiskDev->BlkIo.Media            = &RamDiskDev->Media;
  RamDiskDev->Media.RemovableMedia   = FALSE;
  RamDiskDev->Media.MediaPresent     = TRUE;

  RamDiskDev->Media.LastBlock        = RamDiskSize / BlockSize - 1;
  RamDiskDev->Media.BlockSize        = BlockSize;
  RamDiskDev->Media.LogicalPartition = FALSE;
  RamDiskDev->Media.ReadOnly         = FALSE;
  RamDiskDev->Media.WriteCaching     = TRUE;

  RamDiskDev->BlkIo.ReadBlocks       = RamDiskReadBlocks;
  RamDiskDev->BlkIo.WriteBlocks      = RamDiskWriteBlocks;
  RamDiskDev->BlkIo.FlushBlocks      = RamDiskFlushBlocks;

  RamDiskDev->DevicePath             = AppendDevicePathNode (NULL, \
                                        (EFI_DEVICE_PATH_PROTOCOL *) RamDiskDevNode);

  //
  // Format the ramdisk to a FAT16 file system.
  //
  FormatRamdisk ((VOID*)RamDiskDev->StartingAddr, RamDiskSize);

  //
  // Install protocols.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                    &ImageHandle,
                    &gEfiBlockIoProtocolGuid,
                    &RamDiskDev->BlkIo,
                    &gEfiDevicePathProtocolGuid,
                    RamDiskDev->DevicePath,
                    NULL
                    );

  BENI_MODULE_END

  return Status;
}