/**
*  @Package     : BeniPkg
*  @FileName    : Disk.c
*  @Date        : 20211128
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for disk test.
*
*  @History:
*    20211128: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Disk.h"

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {L"info", TypeFlag }, // info: disk infomation
  {L"-n",   TypeValue}, // -n  : block
  {L"-a",   TypeValue}, // -a  : read address
  {L"-s",   TypeValue}, // -s  : read size
  {NULL ,   TypeMax  }
  };

STATIC CONST CHAR8 Hex[] = {
  '0',
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  'A',
  'B',
  'C',
  'D',
  'E',
  'F'
};

/**
  Dump some hexadecimal data to the screen.

  @param[in]  Indent                How many spaces to indent the output.
  @param[in]  Offset                The offset of the printing.
  @param[in]  DataSize              The size in bytes of UserData.
  @param[in]  UserData              The data to print out.

  @return  NA

**/
STATIC
VOID
EFIAPI
DiskDumpHex (
  IN  UINTN                         Indent,
  IN  UINTN                         Offset,
  IN  UINTN                         DataSize,
  IN  VOID                          *UserData
  )
{
  UINT8 *Data;
  CHAR8 Val[50];
  CHAR8 Str[20];
  UINT8 TempByte;
  UINTN Size;
  UINTN Index;

  Data = UserData;
  while (DataSize != 0) {
    Size = 16;
    if (Size > DataSize) {
      Size = DataSize;
    }

    for (Index = 0; Index < Size; Index += 1) {
      TempByte            = Data[Index];
      Val[Index * 3 + 0]  = Hex[TempByte >> 4];
      Val[Index * 3 + 1]  = Hex[TempByte & 0xF];
      Val[Index * 3 + 2]  = (CHAR8) ((Index == 7) ? '-' : ' ');
      Str[Index]          = (CHAR8) ((TempByte < ' ' || TempByte > '~') ? '.' : TempByte);
    }

    Val[Index * 3]  = 0;
    Str[Index]      = 0;
    Print (L"%*a%08X: %-48a *%a*\r\n", Indent, "", Offset, Val, Str);

    Data += Size;
    Offset += Size;
    DataSize -= Size;
  }
}

/**
  Read disk.

  @param[in]  BlockIo               The pointer to EFI_BLOCK_IO_PROTOCOL.
  @param[in]  DiskIo                The pointer to EFI_DISK_IO2_PROTOCOL.
  @param[in]  Offset                The start address to read data.
  @param[in]  BufferSize            The data size to read.
  @param[out] Buffer                The read data buffer.

  @return  EFI_SUCCESS              Operation succeeded.
  @return  Others                   Operation failed.

**/
STATIC
EFI_STATUS
EFIAPI
DiskReadEx (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO2_PROTOCOL         *DiskIo2,
  IN  UINT64                        Offset,
  IN  UINTN                         BufferSize,
  OUT VOID                          *Buffer
  )
{
  return DiskIo2->ReadDiskEx (
                  DiskIo2,
                  BlockIo->Media->MediaId,
                  Offset,
                  NULL,
                  BufferSize,
                  Buffer
                  );
}

/**
  Retrieve HII package list from ImageHandle and publish to HII database.

  @param[in]  ImageHandle           The image handle of the process.

  @return  EFI_HII_HANDLE           The HII handle.

**/
EFI_HII_HANDLE
InitializeHiiPackage (
  IN  EFI_HANDLE                    ImageHandle
  )
{
  EFI_STATUS                        Status;
  EFI_HII_PACKAGE_LIST_HEADER       *PackageList;
  EFI_HII_HANDLE                    HiiHandle;

  //
  // Retrieve HII package list from ImageHandle.
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Publish HII package list to HII Database.
  //
  Status = gHiiDatabase->NewPackageList (
                           gHiiDatabase,
                           PackageList,
                           NULL,
                           &HiiHandle
                           );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return HiiHandle;
}

/**
  Test to see if the the block IO has a valid MBR.
  The valid MBR will be copied to Data, and the caller should be
  responsible to free the allocated memory.

  @param[in]  BlockIo               Parted block IO.
  @param[in]  DiskIo                Parted disk IO.
  @param[out] Data                  The valid MBR.

  @retval  TRUE                     Mbr is a Valid MBR.
  @retval  FALSE                    Mbr is not a Valid MBR.

**/
BOOLEAN
PartitionValidMbr (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo,
  OUT VOID                          **Data
  )
{
  EFI_STATUS              Status;
  UINT8                   *Buffer;
  MASTER_BOOT_RECORD      *Mbr;
  UINT32                  StartingLBA;
  UINT32                  EndingLBA;
  UINT32                  NewEndingLBA;
  INTN                    Index1;
  INTN                    Index2;
  BOOLEAN                 MbrValid;
  EFI_LBA                 LastLba;

  //
  // Get one block.
  //
  Buffer = AllocateZeroPool (BlockIo->Media->BlockSize);
  if (NULL == Buffer) {
    return FALSE;
  }

  //
  // Get Lba 0 where legacy MBR should reside in.
  //
  Status = BlockIo->ReadBlocks(
                    BlockIo,
                    BlockIo->Media->MediaId,
                    0,
                    BlockIo->Media->BlockSize,
                    Buffer
                    );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  LastLba  = DivU64x32 (
                  MultU64x32 (BlockIo->Media->LastBlock + 1, BlockIo->Media->BlockSize),
                  MBR_SIZE
                  ) - 1;
  Mbr = (MASTER_BOOT_RECORD *)Buffer;

  if (Mbr->Signature != MBR_SIGNATURE) {
    return FALSE;
  }

  //
  // The BPB also has this signature, so it can not be used alone.
  //
  MbrValid = FALSE;
  for (Index1 = 0; Index1 < MAX_MBR_PARTITIONS; Index1++) {
    if (Mbr->Partition[Index1].OSIndicator == 0x00 || UNPACK_UINT32 (Mbr->Partition[Index1].SizeInLBA) == 0) {
      continue;
    }

    MbrValid    = TRUE;
    StartingLBA = UNPACK_UINT32 (Mbr->Partition[Index1].StartingLBA);
    EndingLBA   = StartingLBA + UNPACK_UINT32 (Mbr->Partition[Index1].SizeInLBA) - 1;
    if (EndingLBA > LastLba) {
      //
      // Compatibility Errata:
      //  Some systems try to hide drive space with their INT 13h driver
      //  This does not hide space from the OS driver. This means the MBR
      //  that gets created from DOS is smaller than the MBR created from
      //  a real OS (NT & Win98). This leads to BlockIo->LastBlock being
      //  wrong on some systems FDISKed by the OS.
      //
      // return FALSE since no block devices on a system are implemented
      // with INT 13h
      //
      return FALSE;
    }

    for (Index2 = Index1 + 1; Index2 < MAX_MBR_PARTITIONS; Index2++) {
      if (Mbr->Partition[Index2].OSIndicator == 0x00 || UNPACK_UINT32 (Mbr->Partition[Index2].SizeInLBA) == 0) {
        continue;
      }

      NewEndingLBA = UNPACK_UINT32 (Mbr->Partition[Index2].StartingLBA) + UNPACK_UINT32 (Mbr->Partition[Index2].SizeInLBA) - 1;
      if (NewEndingLBA >= StartingLBA && UNPACK_UINT32 (Mbr->Partition[Index2].StartingLBA) <= EndingLBA) {
        //
        // This region overlaps with the Index1'th region
        //
        return FALSE;
      }
    }
  }

  if (!MbrValid) {
    FreePool (Buffer);
  } else {
    *Data = (VOID *)Buffer;
  }

  //
  // None of the regions overlapped so MBR is O.K.
  //
  return MbrValid;
}

/**
  Show MBR paritions.

  @param[in]  Data                  The MBR data.

  @retval  NA

**/
VOID
ShowMbrPartitions (
  IN  VOID                          *Data
  )
{
  UINTN    Index;
  CHAR16   *OsType;
  CHAR16   *BootIndicatior;
  UINT32   StartingLBA;
  UINT32   EndingLBA;
  //
  // No need to check data here.
  //
  MASTER_BOOT_RECORD *Mbr = (MASTER_BOOT_RECORD *)Data;;
  for (Index = 0; Index < MAX_MBR_PARTITIONS; Index++) {
    StartingLBA = UNPACK_UINT32 (Mbr->Partition[Index].StartingLBA);
    if (0 == StartingLBA) {
      continue;
    }
    EndingLBA   = StartingLBA + UNPACK_UINT32 (Mbr->Partition[Index].SizeInLBA) - 1;
    if (EFI_PARTITION == Mbr->Partition[Index].OSIndicator) {
      OsType = MBR_UEFI_SYSTEM_PARTITION;
    } else if (PMBR_GPT_PARTITION == Mbr->Partition[Index].OSIndicator) {
      OsType = MBR_GPT_PROTECTIVE_PARTITION;
    } else {
      OsType = MBR_UNSPECIFIED_PARTITION;
    }
    if (0x80 == Mbr->Partition[Index].BootIndicator) {
      BootIndicatior = MBR_BOOTABLE;
    } else {
      BootIndicatior = MBR_NOT_BOOTABLE;
    }
    Print (L"    %d: %s %s [0x%x ~ 0x%x]\n",
                      Index,
                      OsType,
                      BootIndicatior,
                      StartingLBA,
                      EndingLBA
                      );
  }

  return;
}

/**
  Checks the CRC32 value in the table header.

  @param[in]  MaxSize               Max Size limit.
  @param[in]  Size                  The size of the table.
  @param[in]  Hdr                   Table to check.

  @return  TRUE                     CRC Valid.
  @return  FALSE                    CRC Invalid.

**/
BOOLEAN
PartitionCheckCrcAltSize (
  IN     UINTN                      MaxSize,
  IN     UINTN                      Size,
  IN OUT EFI_TABLE_HEADER           *Hdr
  )
{
  UINT32        Crc;
  UINT32        OrgCrc;
  EFI_STATUS    Status;

  Crc = 0;

  if (0 == Size) {
    //
    // If header size is 0 CRC will pass so return FALSE here.
    //
    return FALSE;
  }

  if ((MaxSize != 0) && (Size > MaxSize)) {
    return FALSE;
  }

  //
  // Clear old CRC from header.
  //
  OrgCrc     = Hdr->CRC32;
  Hdr->CRC32 = 0;

  Status = gBS->CalculateCrc32 ((UINT8 *) Hdr, Size, &Crc);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // Set results.
  //
  Hdr->CRC32 = Crc;

  //
  // Return status.
  //
  return (BOOLEAN) (OrgCrc == Crc);
}

/**
  Checks the CRC32 value in the table header.

  @param[in]  MaxSize               Max Size limit.
  @param[in]  Hdr                   Table to check.

  @return  TRUE                     CRC Valid.
  @return  FALSE                    CRC Invalid.

**/
BOOLEAN
PartitionCheckCrc (
  IN     UINTN                      MaxSize,
  IN OUT EFI_TABLE_HEADER           *Hdr
  )
{
  return PartitionCheckCrcAltSize (MaxSize, Hdr->HeaderSize, Hdr);
}

/**
  Check if the CRC field in the Partition table header is valid
  for Partition entry array.

  @param[in]  BlockIo               Parent BlockIo interface.
  @param[in]  DiskIo                Disk Io Protocol.
  @param[in]  PartHeader            Partition table header structure.

  @retval  TRUE                     CRC is valid.
  @retval  FALSE                    CRC is invalid.

**/
BOOLEAN
PartitionCheckGptEntryArrayCRC (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo,
  IN  EFI_PARTITION_TABLE_HEADER    *PartHeader
  )
{
  EFI_STATUS    Status;
  UINT8         *Ptr;
  UINT32        Crc;
  UINTN         Size;

  //
  // Read the EFI Partition Entries.
  //
  Ptr = AllocatePool (PartHeader->NumberOfPartitionEntries * PartHeader->SizeOfPartitionEntry);
  if (NULL == Ptr) {
    return FALSE;
  }

  Status = DiskIo->ReadDisk (
                    DiskIo,
                    BlockIo->Media->MediaId,
                    MultU64x32(PartHeader->PartitionEntryLBA, BlockIo->Media->BlockSize),
                    PartHeader->NumberOfPartitionEntries * PartHeader->SizeOfPartitionEntry,
                    Ptr
                    );
  if (EFI_ERROR (Status)) {
    FreePool (Ptr);
    return FALSE;
  }

  Size    = PartHeader->NumberOfPartitionEntries * PartHeader->SizeOfPartitionEntry;

  Status  = gBS->CalculateCrc32 (Ptr, Size, &Crc);
  if (EFI_ERROR (Status)) {
    FreePool (Ptr);
    return FALSE;
  }

  FreePool (Ptr);

  return (BOOLEAN) (PartHeader->PartitionEntryArrayCRC32 == Crc);
}

/**
  This routine will read GPT partition table header and check it.

  @param[in]  BlockIo               Parent BlockIo interface.
  @param[in]  DiskIo                Disk Io protocol.
  @param[in]  Lba                   The starting Lba of the Partition Table
  @param[out] Data                  The valid GPT header.

  @retval  TRUE                     The partition table is valid
  @retval  FALSE                    The partition table is not valid

**/
BOOLEAN
PartitionValidGptTable (
  IN  EFI_BLOCK_IO_PROTOCOL       *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL        *DiskIo,
  IN  EFI_LBA                     Lba,
  OUT VOID                        **Data
  )
{
  EFI_STATUS                   Status;
  UINT32                       BlockSize;
  EFI_PARTITION_TABLE_HEADER   *PartHdr;
  UINT32                       MediaId;
  BOOLEAN                      Result;

  Result = FALSE;
  BlockSize = BlockIo->Media->BlockSize;
  MediaId   = BlockIo->Media->MediaId;
  PartHdr   = AllocateZeroPool (BlockSize);
  if (PartHdr == NULL) {
    Result =  FALSE;
    goto DONE;
  }

  //
  // Read the EFI Partition Table Header.
  //
  Status = DiskIo->ReadDisk (
                     DiskIo,
                     MediaId,
                     MultU64x32 (Lba, BlockSize),
                     BlockSize,
                     PartHdr
                     );
  if (EFI_ERROR (Status)) {
    Result =  FALSE;
    goto DONE;
  }

  if ((PartHdr->Header.Signature != EFI_PTAB_HEADER_ID) ||
      !PartitionCheckCrc (BlockSize, &PartHdr->Header) ||
      PartHdr->MyLBA != Lba ||
      (PartHdr->SizeOfPartitionEntry < sizeof (EFI_PARTITION_ENTRY))
      ) {
    Result =  FALSE;
    goto DONE;
  }

  //
  // Ensure the NumberOfPartitionEntries * SizeOfPartitionEntry doesn't overflow.
  //
  if (PartHdr->NumberOfPartitionEntries > DivU64x32 (MAX_UINTN, PartHdr->SizeOfPartitionEntry)) {
    Result =  FALSE;
    goto DONE;
  }

  if (!PartitionCheckGptEntryArrayCRC (BlockIo, DiskIo, PartHdr)) {
    Result =  FALSE;
    goto DONE;
  }

  Result = TRUE;

DONE:

  if (Result && (NULL != PartHdr)) {
    if (NULL != Data) {
      *Data = PartHdr;
    }
  } else {
    if (NULL != PartHdr) {
      FreePool (PartHdr);
    }
  }

  return Result;
}

/**
  Test to see if the the block IO has a valid protective MBR.

  @param[in]  BlockIo               Parted block IO.
  @param[in]  DiskIo                Parted disk IO.

  @retval  TRUE                     Mbr is a Valid GPT.
  @retval  FALSE                    Mbr is not a Valid GPT.

**/
BOOLEAN
PartitionValidProtectivMbr (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo
  )
{
  EFI_STATUS              Status;
  MASTER_BOOT_RECORD      *ProtectiveMbr;
  UINT32                  BlockSize;
  UINTN                   Index;

  BlockSize     = BlockIo->Media->BlockSize;
  ProtectiveMbr = NULL;

  //
  // Ensure the block size can hold the MBR.
  //
  if (BlockSize < sizeof (MASTER_BOOT_RECORD)) {
    return FALSE;
  }

  //
  // Allocate a buffer for the Protective MBR.
  //
  ProtectiveMbr = AllocatePool (BlockSize);
  if (ProtectiveMbr == NULL) {
    return FALSE;
  }

  //
  // Read the Protective MBR from LBA 0.
  //
  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     0,
                     BlockSize,
                     ProtectiveMbr
                     );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // Verify that the protective MBR is valid.
  //
  for (Index = 0; Index < MAX_MBR_PARTITIONS; Index++) {
    if ((ProtectiveMbr->Partition[Index].OSIndicator == PMBR_GPT_PARTITION) &&
        (UNPACK_UINT32 (ProtectiveMbr->Partition[Index].StartingLBA) == 1)
        ) {
      break;
    }
  }
  if (Index == MAX_MBR_PARTITIONS) {
    return FALSE;
  }

  return TRUE;
}

/**
  Test to see if the the block IO has a valid GPT.

  @param[in]  BlockIo               Parted block IO.
  @param[in]  DiskIo                Parted disk IO.
  @param[out] Data                  The MBR data.

  @retval  TRUE                     Mbr is a Valid GPT.
  @retval  FALSE                    Mbr is not a Valid GPT.

**/
BOOLEAN
PartitionValidGpt (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo,
  OUT VOID                          **Data
  )
{
  //
  // The LBA 0 is protective MBR.
  //
  if (!PartitionValidProtectivMbr(BlockIo, DiskIo)) {
    return FALSE;
  }

  //
  // The LBA 1 is primary GPT table.
  //
  if (!PartitionValidGptTable (BlockIo, DiskIo, PRIMARY_PART_HEADER_LBA, Data)) {
    // DEBUG ((EFI_D_ERROR, "Primary GPT error!\n"));
    return FALSE;
  }

  //
  // The last LBA is backup GPT table.
  //
  if (!PartitionValidGptTable (BlockIo, DiskIo, BlockIo->Media->LastBlock, NULL)) {
    // DEBUG ((EFI_D_ERROR, "Backup GPT error!\n"));
    return FALSE;
  }

  return TRUE;
}

/**
  This routine will check GPT partition entry and return entry status.

  Caution: This function may receive untrusted input.
  The GPT partition entry is external input, so this routine
  will do basic validation for GPT partition entry and report status.

  @param[in]  PartHeader            Partition table header structure
  @param[in]  PartEntry             The partition entry array
  @param[out] PEntryStatus          The partition entry status array
                                    recording the status of each partition

  @retval  NA

**/
VOID
PartitionCheckGptEntry (
  IN  EFI_PARTITION_TABLE_HEADER  *PartHeader,
  IN  EFI_PARTITION_ENTRY         *PartEntry,
  OUT EFI_PARTITION_ENTRY_STATUS  *PEntryStatus
  )
{
  EFI_LBA                 StartingLBA;
  EFI_LBA                 EndingLBA;
  EFI_PARTITION_ENTRY     *Entry;
  UINTN                   Index1;
  UINTN                   Index2;

  for (Index1 = 0; Index1 < PartHeader->NumberOfPartitionEntries; Index1++) {
    Entry = (EFI_PARTITION_ENTRY *) ((UINT8 *) PartEntry + Index1 * PartHeader->SizeOfPartitionEntry);
    if (CompareGuid (&Entry->PartitionTypeGUID, &gEfiPartTypeUnusedGuid)) {
      continue;
    }

    StartingLBA = Entry->StartingLBA;
    EndingLBA   = Entry->EndingLBA;
    if (StartingLBA > EndingLBA ||
        StartingLBA < PartHeader->FirstUsableLBA ||
        StartingLBA > PartHeader->LastUsableLBA ||
        EndingLBA < PartHeader->FirstUsableLBA ||
        EndingLBA > PartHeader->LastUsableLBA
        ) {
      PEntryStatus[Index1].OutOfRange = TRUE;
      continue;
    }

    if ((Entry->Attributes & BIT1) != 0) {
      //
      // If Bit 1 is set, this indicate that this is an OS specific GUID partition.
      //
      PEntryStatus[Index1].OsSpecific = TRUE;
    }

    for (Index2 = Index1 + 1; Index2 < PartHeader->NumberOfPartitionEntries; Index2++) {
      Entry = (EFI_PARTITION_ENTRY *) ((UINT8 *) PartEntry + Index2 * PartHeader->SizeOfPartitionEntry);
      if (CompareGuid (&Entry->PartitionTypeGUID, &gEfiPartTypeUnusedGuid)) {
        continue;
      }

      if (Entry->EndingLBA >= StartingLBA && Entry->StartingLBA <= EndingLBA) {
        //
        // This region overlaps with the Index1'th region
        //
        PEntryStatus[Index1].Overlap  = TRUE;
        PEntryStatus[Index2].Overlap  = TRUE;
        continue;
      }
    }
  }
}

/**
  Show GPT paritions.

  @param[in]  BlockIo               Parted block IO.
  @param[in]  DiskIo                Parted disk IO.
  @param[in]  Data                  The GPT data.

  @retval  NA

**/
VOID
ShowGptPartitions (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo,
  IN  VOID                          *Data
  )
{
  EFI_STATUS                   Status;
  EFI_PARTITION_ENTRY_STATUS   *PEntryStatus;
  EFI_PARTITION_TABLE_HEADER   *PrimaryHeader;
  EFI_PARTITION_ENTRY          *PartEntry;
  EFI_PARTITION_ENTRY          *Entry;
  UINTN                        Index;
  CHAR16                       *PartType;

  PrimaryHeader = (EFI_PARTITION_TABLE_HEADER *)Data;
  //
  // If no partitions exist, just return;
  //
  if (0 == PrimaryHeader->NumberOfPartitionEntries) {
    return;
  }

  PartEntry = AllocatePool (PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry);
  if (NULL == PartEntry) {
    goto DONE;
  }

  PEntryStatus = AllocateZeroPool (PrimaryHeader->NumberOfPartitionEntries * sizeof (EFI_PARTITION_ENTRY_STATUS));
  if (NULL == PEntryStatus) {
    goto DONE;
  }

  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     MultU64x32(PrimaryHeader->PartitionEntryLBA, BlockIo->Media->BlockSize),
                     PrimaryHeader->NumberOfPartitionEntries * (PrimaryHeader->SizeOfPartitionEntry),
                     PartEntry
                     );
  if (EFI_ERROR (Status)) {
    goto DONE;
  }

  //
  // Check the integrity of partition entries.
  //
  PartitionCheckGptEntry (PrimaryHeader, PartEntry, PEntryStatus);

  for (Index = 0; Index < PrimaryHeader->NumberOfPartitionEntries; Index++) {
    Entry = (EFI_PARTITION_ENTRY *) ((UINT8 *) PartEntry + Index * PrimaryHeader->SizeOfPartitionEntry);
    if (CompareGuid (&Entry->PartitionTypeGUID, &gEfiPartTypeSystemPartGuid)) {
      PartType = GPT_SYSTEM_PARTITION;
    } else if (CompareGuid (&Entry->PartitionTypeGUID, &gEfiPartTypeLegacyMbrGuid)) {
      PartType = GPT_LEGACY_MBR_PARTITION;
    } else {
      PartType = GPT_UNUSED_PARTITION;
    }

    if (0 == Entry->StartingLBA) {
      continue;
    }
    Print (L"    %d: %s %s %g [0x%x ~ 0x%x]\n",
                      Index,
                      Entry->PartitionName,
                      PartType,
                      Entry->UniquePartitionGUID,
                      Entry->StartingLBA,
                      Entry->EndingLBA
                      );
  }

DONE:
  if (NULL != PartEntry) {
    FreePool (PartEntry);
  }
  if (NULL != PEntryStatus) {
    FreePool (PEntryStatus);
  }

  return;
}

/**
  Show disk informations.

  @param[in]  BlockIo               Parted block IO.
  @param[in]  DiskIo                Parted disk IO.

  @retval  NA

**/
STATIC
VOID
EFIAPI
ShowPartInfo (
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_DISK_IO_PROTOCOL          *DiskIo
  )
{
  VOID *Data = NULL;

  Print (L"  Partition type    : ");
  if (PartitionValidMbr(BlockIo, DiskIo, &Data)) {
    Print (L"MBR\n");
    if (NULL != Data) {
      ShowMbrPartitions (Data);
      FreePool (Data);
    }
  } else if (PartitionValidGpt(BlockIo, DiskIo, &Data)) {
    Print (L"GPT\n");
    if (NULL != Data) {
      ShowGptPartitions (BlockIo, DiskIo, Data);
      FreePool (Data);
    }
  } else {
    Print (L"Unknown\n");
  }
}

/**
  Show disk informations.

  @param  NA

  @retval  NA

**/
STATIC
VOID
EFIAPI
ShowDiskInfo (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *Handles;
  UINTN                             Count;
  UINTN                             Index;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_DISK_IO_PROTOCOL              *DiskIo;

  Handles     = NULL;
  Count       = 0;
  Status        = EFI_NOT_FOUND;

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
    Print (L"-----------------------------------\n");
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

    Status = gBS->HandleProtocol (
                  Handles[Index],
                  &gEfiDiskIoProtocolGuid,
                  (VOID **)&DiskIo
                  );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (BlockIo->Media->LogicalPartition) {
      continue;
    }

    Print (L"BLOCK%d:\n", Index);
    Print (L"  Media ID          : %d\n", BlockIo->Media->MediaId);
    Print (L"  Removable Media   : %d\n", BlockIo->Media->RemovableMedia);
    Print (L"  Media Present     : %d\n", BlockIo->Media->MediaPresent);
    Print (L"  Logical Partition : %d\n", BlockIo->Media->LogicalPartition);
    Print (L"  Read Only         : %d\n", BlockIo->Media->ReadOnly);
    Print (L"  Write Caching     : %d\n", BlockIo->Media->WriteCaching);
    Print (L"  Block Size        : %d\n", BlockIo->Media->BlockSize);
    Print (L"  Last Block        : %d\n", BlockIo->Media->LastBlock);
    ShowPartInfo (BlockIo, DiskIo);
  }

  if (NULL != Handles) {
    FreePool (Handles);
  }

  return;
}

/**
  Show disk data.

  @param[in]  BlkIndex              The disk index specified by block IO protocol.
  @param[in]  Address               Disk address.
  @param[in]  Size                  The size to read.

  @retval  NA

**/
STATIC
VOID
EFIAPI
ShowDiskData (
  IN  UINTN                         BlkIndex,
  IN  UINTN                         Address,
  IN  UINTN                         Size
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *Handles;
  UINTN                             Count;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_DISK_IO2_PROTOCOL             *DiskIo2;
  VOID                              *Buffer;

  Handles     = NULL;
  Buffer      = NULL;
  Count       = 0;

  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiBlockIoProtocolGuid,
                NULL,
                &Count,
                &Handles
                );
  if (EFI_ERROR (Status) || (0 == Count)) {
    Print (L"No block found!\n");
    goto DONE;
  }

  if (BlkIndex >= Count) {
    Print (L"Block %d not found.\n", BlkIndex);
    goto DONE;
  }

  Status = gBS->HandleProtocol (
                Handles[BlkIndex],
                &gEfiBlockIoProtocolGuid,
                (VOID **)&BlockIo
                );
  if (EFI_ERROR (Status)) {
    Print (L"No block IO for %d\n", BlkIndex);
    goto DONE;
  }

  Status = gBS->HandleProtocol (
                Handles[BlkIndex],
                &gEfiDiskIo2ProtocolGuid,
                (VOID **)&DiskIo2
  );
  if (EFI_ERROR (Status)) {
    Print (L"No disk IO 2 for %d\n", BlkIndex);
    goto DONE;
  }

  Buffer = AllocateZeroPool (Size);
  if (!Buffer) {
    Print (L"Resource not enough\n");
    goto DONE;
  }

  Status = DiskReadEx (BlockIo, DiskIo2, Address, Size, Buffer);
  if (EFI_ERROR (Status)) {
    Print (L"Read disk failed. - %r\n", Status);
    goto DONE;
  }

  DiskDumpHex (2, 0, Size, Buffer);

DONE:

  if (NULL != Handles) {
    FreePool (Handles);
  }

  if (NULL != Buffer) {
    FreePool (Buffer);
  }

  return;
}

/**
  Function for 'disk' command.

  @param[in]  ImageHandle           The image handle.
  @param[in]  SystemTable           The system table.

  @retval  SHELL_SUCCESS            Command completed successfully.
  @retval  SHELL_INVALID_PARAMETER  Command usage error.
  @retval  SHELL_ABORTED            The user aborts the operation.
  @retval  Value                    Unknown error.

**/
SHELL_STATUS
RunDisk (
  IN  EFI_HANDLE                    ImageHandle,
  IN  EFI_SYSTEM_TABLE              *SystemTable
  )
{
  SHELL_STATUS            ShellStatus;
  EFI_STATUS              Status;
  LIST_ENTRY              *CheckPackage;
  CHAR16                  *ProblemParam;
  UINTN                   ParamCount;
  CONST CHAR16            *Temp;
  UINTN                   Address;
  UINTN                   Size;
  UINTN                   Blk;
  BOOLEAN                 ShowInfo;
  VOID                    *Buffer;

  ShellStatus         = SHELL_INVALID_PARAMETER;
  ProblemParam        = NULL;
  ShowInfo            = FALSE;
  Address             = 0;
  Size                = 0;
  Blk                 = 0;
  Buffer              = NULL;

  //
  // Initialize the Shell library (we must be in non-auto-init...).
  //
  Status = ShellInitialize ();
  if (EFI_ERROR (Status)) {
    return SHELL_ABORTED;
  }

  //
  // Parse the command line.
  //
  Status = ShellCommandLineParse (ParamList, &CheckPackage, &ProblemParam, TRUE);
  if (EFI_ERROR (Status)) {
    if ((Status == EFI_VOLUME_CORRUPTED) && (ProblemParam != NULL) ) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), mDiskHiiHandle, L"disk", ProblemParam
        );
      FreePool (ProblemParam);
    }
    goto DONE;
  }

  //
  // Check the number of parameters
  //
  ParamCount = ShellCommandLineGetCount (CheckPackage);
  if (ParamCount != 1) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), mDiskHiiHandle, L"disk"
      );
    goto DONE;
  }

  //
  // Show disk information.
  //
  ShowInfo = ShellCommandLineGetFlag (CheckPackage, L"Info");
  if (ShowInfo) {
    ShowDiskInfo ();
    goto DONE;
  }

  //
  // Show disk data.
  //
  Temp = ShellCommandLineGetValue (CheckPackage, L"-a");
  if (Temp != NULL) {
    Address = ShellStrToUintn (Temp);
  }
  Temp = ShellCommandLineGetValue (CheckPackage, L"-n");
  if (Temp != NULL) {
    Blk = ShellStrToUintn (Temp);
  }
  Temp = ShellCommandLineGetValue (CheckPackage, L"-s");
  if (Temp != NULL) {
    Size = ShellStrToUintn (Temp);
    if (!Size) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_INV),
        mDiskHiiHandle, L"disk", Temp
      );
      goto DONE;
    }
  }
  if (Size) {
    Print (L"Block %d, Address: 0x%x, Size: 0x%x\n", Blk, Address, Size);
    ShowDiskData (Blk, Address, Size);
    goto DONE;
  }

  Print (L"No operation specified.\n");

DONE:

  if ((ShellStatus != SHELL_SUCCESS) && (EFI_ERROR (Status))) {
    ShellStatus = Status & ~MAX_BIT;
  }

  if (NULL != Buffer) {
    FreePool (Buffer);
  }

  return ShellStatus;
}