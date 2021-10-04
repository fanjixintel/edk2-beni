/**
*  @Package     : BeniPkg
*  @FileName    : Ramdisk.h
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

#ifndef __RAM_DISK_H__
#define __RAM_DISK_H__

#include <Uefi.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/BeniDebugLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/BlockIo.h>

#define  DEFAULT_DISK_SIZE          0x20 // in MB

#pragma pack(1)
//
// FAT16 structure.
// See <Microsoft Extensible Firmware Initiative FAT32 File System Specification> for details.
//
typedef struct {
  UINT8         BS_jmpBoot[3];      // Offset 0
  UINT8         BS_OEMName[8];      // Offset 3
  UINT16        BPB_BytsPerSec;     // Offset 11
  UINT8         BPB_SecPerClus;     // Offset 13
  UINT16        BPB_RsvdSecCnt;     // Offset 14
  UINT8         BPB_NumFATs;        // Offset 16
  UINT16        BPB_RootEntCnt;     // Offset 17
  UINT16        BPB_TotSec16;       // Offset 19
  UINT8         BPB_Media;          // Offset 21
  UINT16        BPB_FATSz16;        // Offset 22
  UINT16        BPB_SecPerTrk;      // Offset 24
  UINT16        BPB_NumHeads;       // Offset 26
  UINT32        BPB_HiddSec;        // Offset 28
  UINT32        BPB_TotSec32;       // Offset 32
  UINT8         BS_DrvNum;          // Offset 36
  UINT8         BS_Reserved1;       // Offset 37
  UINT8         BS_BootSig;         // Offset 38
  UINT32        BS_VolID;           // Offset 39
  UINT8         BS_VolLab[11];      // Offset 43
  UINT8         BS_FilSysType[8];   // Offset 54
  UINT8         BS_Code[448];       // Offset 62
  UINT16        BS_Sig;             // Offset 510
} BOOT_SECTOR;
#pragma pack()

//
// See <Microsoft Extensible Firmware Initiative FAT32 File System Specification> for details.
//
typedef struct {
  UINT32        DiskSize;
  UINT8         SecPerClusVal;
} DISK_SIZE_TO_SECTOR_PER_CLUSTER;

//
// Ramdisk device structure, which is a simple edition of RAM_DISK_PRIVATE_DATA in RamDiskImpl.h.
//
typedef struct {
  UINTN                   Signature;

  EFI_HANDLE              Handle;

  EFI_BLOCK_IO_PROTOCOL   BlkIo;
  EFI_BLOCK_IO_MEDIA      Media;
  EFI_DEVICE_PATH         *DevicePath;

  UINT64                  StartingAddr;
} RAM_DISK_DAV;

#define RAM_DISK_DATA_SIGNATURE     SIGNATURE_32 ('R', 'A', 'M', 'D')
#define RAM_DISK_FROM_THIS(a)       CR(a, RAM_DISK_DAV, BlkIo, RAM_DISK_DATA_SIGNATURE)

/**
  Format the allocateed memory to FAT16 file system.
  See <Microsoft Extensible Firmware Initiative FAT32 File System Specification> for details.

  @param[in]  StartAddress          Start address of the RAM disk in memroy.
  @param[in]  Size                  Size of the RAM disk.

  @retval  NA

**/
VOID
EFIAPI
FormatRamdisk (
  IN  VOID                          *StartAddress,
  IN  UINT32                        Size
  );

/**
  Read BufferSize bytes from Lba into Buffer.

  @param[in]  This                  Indicates a pointer to the calling context.
  @param[in]  MediaId               Id of the media, changes every time the media is replaced.
  @param[in]  Lba                   The starting Logical Block Address to read from
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
  );

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
  );

/**
  Flush the Block Device.

  @param[in]  This                  Indicates a pointer to the calling context.

  @retval  EFI_SUCCESS              All outstanding data was written to the device
  @retval  EFI_DEVICE_ERROR         The device reported an error while writting back the data
  @retval  EFI_NO_MEDIA             There is no media in the device.

**/
EFI_STATUS
EFIAPI
RamDiskFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL         *This
  );

#endif // __RAM_DISK_H__