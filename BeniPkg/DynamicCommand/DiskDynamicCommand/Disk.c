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
    if (!EFI_ERROR (Status)) {
      Print (L"BLOCK%d:\n", Index);
      Print (L"  Media ID          : %d\n", BlockIo->Media->MediaId);
      Print (L"  Removable Media   : %d\n", BlockIo->Media->RemovableMedia);
      Print (L"  Media Present     : %d\n", BlockIo->Media->MediaPresent);
      Print (L"  Logical Partition : %d\n", BlockIo->Media->LogicalPartition);
      Print (L"  Read Only         : %d\n", BlockIo->Media->ReadOnly);
      Print (L"  Write Caching     : %d\n", BlockIo->Media->WriteCaching);
      Print (L"  Block Size        : %d\n", BlockIo->Media->BlockSize);
      Print (L"  Last Block        : %d\n", BlockIo->Media->LastBlock);
    }
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
  }

DONE:

  if ((ShellStatus != SHELL_SUCCESS) && (EFI_ERROR (Status))) {
    ShellStatus = Status & ~MAX_BIT;
  }

  if (NULL != Buffer) {
    FreePool (Buffer);
  }

  return ShellStatus;
}