/** @file

  Copyright (c) 2019 - 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Copyright (c) 1982, 1986, 1989, 1993
  The Regents of the University of California.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.

   @file fs.h 8.10 (Berkeley) 10/27/94
   Modified for ext2fs by Manuel Bouyer.

  Copyright (c) 1997 Manuel Bouyer.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   @file fs.h 8.10 (Berkeley) 10/27/94
   Modified for ext2fs by Manuel Bouyer.
**/

#ifndef __EXT2_FS_H__
#define __EXT2_FS_H__

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>

#include "Ext2FsDiNode.h"
#include "Ext2FsDir.h"

#define EXT2FS_DEBUG

//
// <sys/types.h>
//
typedef long int      DADDRESS;
typedef long int      OFFSET;
typedef unsigned long ULONG;
typedef unsigned long INODE;
typedef UINT32        INODE32;
typedef INT32         INDPTR;

#define FS_EXT_SIGNATURE            SIGNATURE_32 ('p', 'e', 'x', 't')

#define PART_MAX_BLOCK_SIZE         8192
#define PART_MAX_BLOCK_DEVICE       64

//
// Seek method constants.
//
#define SEEK_SET                    0
#define SEEK_CUR                    1
#define SEEK_END                    2

//
// Each disk drive contains some number of file systems.
// A file system consists of a number of cylinder groups.
// Each cylinder group has inodes and data.
//
// A file system is described by its super-block, which in turn
// describes the cylinder groups.  The super-block is critical
// data and is replicated in each cylinder group to protect against
// catastrophic loss.  This is done at `newfs' time and the critical
// super-block data does not change, so the copies need not be
// referenced further unless disaster strikes.
//
// The first boot and super blocks are given in absolute disk addresses.
// The byte-offset forms are preferred, as they don't imply a sector size.
//
#define BBSIZE                      1024
#define SBSIZE                      1024
#define BBOFF                       ((OFFSET)(0))
#define SBOFF                       ((OFFSET)(BBOFF + BBSIZE))
#define BBLOCK                      ((DADDRESS)(0))
#define SBLOCK                      ((DADDRESS)(BBLOCK + BBSIZE / DEV_BSIZE))

#define MAXSYMLINKS                 1
#define MAXPATHLEN                  260

//
// Addresses stored in inodes are capable of addressing blocks
// XXX
//
// MINBSIZE is the smallest allowable block size.
// MINBSIZE must be big enough to hold a cylinder group block,
// thus changes to (struct cg) must keep its size within MINBSIZE.
// Note that super blocks are always of size SBSIZE,
// and that both SBSIZE and MAXBSIZE must be >= MINBSIZE.
//
#define LOG_MINBSIZE                10
#define MINBSIZE                    (1 << LOG_MINBSIZE)

//
// The path name on which the file system is mounted is maintained
// in fs_fsmnt. MAXMNTLEN defines the amount of space allocated in
// the super block for this name.
//
#define MAXMNTLEN                   512

//
// MINFREE gives the minimum acceptable percentage of file system
// blocks which may be free. If the freelist drops below this level
// only the superuser may continue to allocate blocks. This may
// be set to 0 if no reserve of free blocks is deemed necessary,
// however throughput drops by fifty percent if the file system
// is run at between 95% and 100% full; thus the minimum default
// value of fs_minfree is 5%. However, to get good clustering
// performance, 10% is a better choice. hence we use 10% as our
// default value. With 10% free space, fragmentation is not a
// problem, so we choose to optimize for time.
//
#define MINFREE                     5


//
// Filesystem identification.
//
#define E2FS_MAGIC                  0xEF53  // The ext2fs magic number.
#define E2FS_REV0                   0       // GOOD_OLD revision.
#define E2FS_REV1                   1       // Support compat/incompat features.
//
// Compatible/incompatible features.
//
#define EXT2F_COMPAT_PREALLOC       0x0001
#define EXT2F_COMPAT_HASJOURNAL     0x0004
#define EXT2F_COMPAT_RESIZE         0x0010

#define EXT2F_ROCOMPAT_SPARSESUPER  0x0001
#define EXT2F_ROCOMPAT_LARGEFILE    0x0002
#define EXT2F_ROCOMPAT_BTREE_DIR    0x0004

#define EXT2F_INCOMPAT_COMP         0x0001
#define EXT2F_INCOMPAT_FTYPE        0x0002

//
// TODO: We need to enable support for
//       EXT FS recovery handling at some
//       point to ensure that we don't
//       encounter a real file error.
//
#define EXT2F_INCOMPAT_RECOVER      0x0004
#define EXT2F_INCOMPAT_64BIT        0x0080
#define EXT2F_INCOMPAT_EXTENTS      0x0040
#define EXT2F_INCOMPAT_FLEX_BG      0x0200

//
// Features supported in this implementation.
//
// We support the following REV1 features:
// - EXT2F_ROCOMPAT_SPARSESUPER
//     superblock backups stored only in cg_has_sb(bno) groups
// - EXT2F_ROCOMPAT_LARGEFILE
//     use e2di_dacl in EXTFS_DINODE to store
//     upper 32bit of size for >2GB files
// - EXT2F_INCOMPAT_FTYPE
//     store file type to e2d_type in EXT2FS_direct
//     (on REV0 e2d_namlen is UINT16 and no e2d_type, like ffs)
//
#define EXT2F_COMPAT_SUPP           0x0000
#define EXT2F_ROCOMPAT_SUPP         (EXT2F_ROCOMPAT_SPARSESUPER \
                                    | EXT2F_ROCOMPAT_LARGEFILE)
#define EXT2F_INCOMPAT_SUPP         (EXT2F_INCOMPAT_FTYPE    \
                                    | EXT2F_INCOMPAT_RECOVER \
                                    | EXT2F_INCOMPAT_64BIT   \
                                    | EXT2F_INCOMPAT_EXTENTS \
                                    | EXT2F_INCOMPAT_FLEX_BG)

//
// Definitions of behavior on errors.
//
#define E2FS_BEH_CONTINUE           1 // Continue operation.
#define E2FS_BEH_READONLY           2 // Remount fs read only.
#define E2FS_BEH_PANIC              3 // Cause panic.
#define E2FS_BEH_DEFAULT            E2FS_BEH_CONTINUE

//
// The cache size (IND_CACHE_SZ) must be smaller or equal the number of pointers
// in the indirect blocks: NINDIR = Ext2FsBlockSize / sizeof(UINT32).
// For ext2fs minimal block size is 1kB and block pointer size is 4 (UINT32)
// so LN2_IND_CACHE_SZ <= 8 (cache size IND_CACHE_SZ=2^8=256)
// Optimal for file system speed is the biggest cache size possible.
//
#define LN2_IND_CACHE_SZ            8
#define IND_CACHE_SZ                (1 << LN2_IND_CACHE_SZ)
#define IND_CACHE_MASK              (IND_CACHE_SZ - 1)


//
// Turn file system block numbers into disk block addresses.
// This maps file system blocks to device size blocks.
//
#define FSBTODB(fs, b)              ((b) << (fs)->Ext2FsFsbtobd)
#define DBTOFSB(fs, b)              ((b) >> (fs)->Ext2FsFsbtobd)

//
// Macros for handling inode numbers:
//   inode number to file system block offset.
//   inode number to cylinder group number.
//   inode number to file system block address.
//
#define INOTOCG(fs, x)              (((x) - 1) / (fs)->Ext2Fs.Ext2FsINodesPerGroup)
#define INODETOFSBA(fs, x)          ((fs)->Ext2FsGrpDes[INOTOCG((fs), (x))].Ext2BGDInodeTables + (((x) - 1) % (fs)->Ext2Fs.Ext2FsINodesPerGroup) / (fs)->Ext2FsInodesPerBlock)
#define INODETOFSBO(fs, x)          ((((x) - 1) % (fs)->Ext2Fs.Ext2FsINodesPerGroup) % (fs)->Ext2FsInodesPerBlock)

//
// Give cylinder group number for a file system block.
// Give cylinder group block number for a file system block.
//
#define DTOG(fs, d)                 (((d) - (fs)->Ext2Fs.Ext2FsFirstDataBlock) / (fs)->Ext2Fs.Ext2FsFragsPerGroup)
#define DTOGD(fs, d)                (((d) - (fs)->Ext2Fs.Ext2FsFirstDataBlock) % (fs)->Ext2Fs.Ext2FsFragsPerGroup)

//
// The following macros optimize certain frequently calculated
// quantities by using shifts and masks in place of divisions
// modulos and multiplications.
//
#define BLOCKOFFSET(fs, loc)        ((loc) & (fs)->Ext2FsQuadBlockOffset)
#define LBLKTOSIZE(fs, blk)         ((blk) << (fs)->Ext2FsLogicalBlock)
#define LBLKNO(fs, loc)             ((loc) >> (fs)->Ext2FsLogicalBlock)
#define BLKROUNDUP(fs, size)        (((size) + (fs)->Ext2FsQuadBlockOffset) & (fs)->Ext2FsBlockOffset)
#define FRAGROUNDUP(fs, size)       (((size) + (fs)->Ext2FsQuadBlockOffset) & (fs)->Ext2FsBlockOffset)

//
// Determine the number of available frags given a
// percentage to hold in reserve.
//
#define FREESPACE(fs)               ((fs)->Ext2Fs.Ext2FsFreeBlockCount - (fs)->Ext2Fs.Ext2FsRsvdBlockCount)

//
// Number of indirects in a file system block.
//
#define NINDIR(fs)                  ((fs)->Ext2FsBlockSize / sizeof(UINT32))

//
// EXT2FS meta data is stored in little-endian byte order. These macros
// help with reading the meta data.
//
#define E2FS_SBLOAD(old, new)       CopyMem((new), (old), SBSIZE);
#define E2FS_CGLOAD(old, new, size) CopyMem((new), (old), (size));
#define E2FS_SBSAVE(old, new)       CopyMem((new), (old), SBSIZE);
#define E2FS_CGSAVE(old, new, size) CopyMem((new), (old), (size));

//
// Opened file struct for EXT2 file system.
//
typedef struct {
  INT32    FileFlags;               // See F_* below.
  VOID     *FileDevData;            // Device specific data.
  VOID     *FileSystemSpecificData; // File system specific data (struct FILE).
  OFFSET   FileOffset;              // Current file offset (F_RAW).
  CHAR8    *FileNamePtr;            // File name.
} OPEN_FILE;

//
// Super block for an ext2fs file system.
//
typedef struct {
  UINT32   Ext2FsINodeCount;        // Inode count.
  UINT32   Ext2FsBlockCount;        // blocks count.
  UINT32   Ext2FsRsvdBlockCount;    // reserved blocks count.
  UINT32   Ext2FsFreeBlockCount;    // free blocks count.
  UINT32   Ext2FsFreeINodeCount;    // free inodes count.
  UINT32   Ext2FsFirstDataBlock;    // first data block.
  UINT32   Ext2FsLogBlockSize;      // block size = 1024*(2^Ext2FsLogBlockSize).
  UINT32   Ext2FsFragmentSize;      // fragment size.
  UINT32   Ext2FsBlocksPerGroup;    // blocks per group.
  UINT32   Ext2FsFragsPerGroup;     // frags per group.
  UINT32   Ext2FsINodesPerGroup;    // inodes per group.
  UINT32   Ext2FsMountTime;         // mount time.
  UINT32   Ext2FsWriteTime;         // write time.
  UINT16   Ext2FsMountCount;        // mount count.
  UINT16   Ext2FsMaxMountCount;     // max mount count.
  UINT16   Ext2FsMagic;             // magic number.
  UINT16   Ext2FsState;             // file system state.
  UINT16   Ext2FsBehavior;          // behavior on errors.
  UINT16   Ext2FsMinorRev;          // minor revision level.
  UINT32   Ext2FsLastFsck;          // time of last fsck.
  UINT32   Ext2FsFsckInterval;      // max time between fscks.
  UINT32   Ext2FsCreator;           // creator OS.
  UINT32   Ext2FsRev;               // revision level.
  UINT16   Ext2FsRsvdUid;           // default uid for reserved blocks.
  UINT16   Ext2FsRsvdGid;           // default gid for reserved blocks.
  //
  // EXT2_DYNAMIC_REV superblocks.
  //
  UINT32   Ext2FsFirstInode;        // first non-reserved inode.
  UINT16   Ext2FsInodeSize;         // size of inode structure.
  UINT16   Ext2FsBlockGrpNum;       // block grp number of this sblk.
  UINT32   Ext2FsFeaturesCompat;    // Compatible feature set.
  UINT32   Ext2FsFeaturesIncompat;  // incompatible feature set.
  UINT32   Ext2FsFeaturesROCompat;  // RO-compatible feature set.
  UINT8    Ext2FsUuid[16];          // 128-bit uuid for volume.
  CHAR8    Ext2FsVolumeName[16];    // volume name.
  CHAR8    Ext2FsFSMnt[64];         // name mounted on.
  UINT32   Ext2FsAlgorithm;         // For compression.
  UINT8    Ext2FsPreAlloc;          // # of blocks to preallocate.
  UINT8    Ext2FsDirPreAlloc;       // # of blocks to preallocate for dir.
  UINT16   Ext2FsRsvdGDBlock;       // # of reserved gd blocks for resize.
  UINT32   Rsvd2[11];
  UINT16   Rsvd3;
  UINT16   Ext2FsGDSize;            // Size of group descriptors, in bytes, if the 64bit incompat feature flag is set.
  UINT32   Rsvd4[192];
} EXT2FS;

//
// Ext2 file system block group descriptor.
//
typedef struct {
  UINT32   Ext2BGDBlockBitmap;      // Blocks bitmap block.
  UINT32   Ext2BGDInodeBitmap;      // Inodes bitmap block.
  UINT32   Ext2BGDInodeTables;      // Inodes table block.
  UINT16   Ext2BGDFreeBlocks;       // Number of free blocks.
  UINT16   Ext2BGDFreeInodes;       // Number of free inodes.
  UINT16   Ext2BGDNumDir;           // Number of directories.
  UINT16   Rsvd;
  UINT32   Rsvd2[5];
  UINT32   Ext2BGDInodeTablesHi;    // Upper 32 bits of inodes table block, if the 64bit incompat feature flag is set.
  UINT32   Rsvd3[5];
} EXT2GD;

//
// In-memory data for ext2fs.
//
typedef struct {
  EXT2FS   Ext2Fs;
  UINT8    Ext2FsFSMnt[MAXMNTLEN];  // Name mounted on.
  CHAR8    Ext2FsReadOnly;          // Mounted read-only flag.
  CHAR8    Ext2FsModified;          // Super block modified flag.
  INT32    Ext2FsBlockSize;         // Block size.
  INT32    Ext2FsLogicalBlock;      // ``lblkno'' calc of logical blkno.
  INT32    Ext2FsBlockOffset;       // ``blkoff'' calc of blk offsets.
  INT64    Ext2FsQuadBlockOffset;   // ~fs_bmask - for use with quad size.
  INT32    Ext2FsFsbtobd;           // FSBTODB and DBTOFSB shift constant.
  INT32    Ext2FsNumCylinder;       // Number of cylinder groups (block groups).
  INT32    Ext2FsNumGrpDesBlock;    // Number of group descriptor block.
  INT32    Ext2FsInodesPerBlock;    // Number of inodes per block.
  INT32    Ext2FsInodesTablePerGrp; // Number of inode table per group.
  UINT32   Ext2FsGDSize;            // Size of group descriptors.
  EXT2GD   *Ext2FsGrpDes;           // Group descriptors.
} M_EXT2FS;

//
// In-core open file.
//
typedef struct {
  OFFSET        SeekPtr;                      // Seek pointer.
  M_EXT2FS      *SuperBlockPtr;               // Pointer to super-block.
  EXTFS_DINODE  DiskInode;                    // Copy of on-disk inode.
  UINT32        NiShift;                      // For blocks in indirect block.
  INDPTR        InodeCacheBlock;
  INDPTR        InodeCache[IND_CACHE_SZ];
  CHAR8         *Buffer;                      // Buffer for data block.
  UINT32        BufferSize;                   // Size of data block.
  DADDRESS      BufferBlockNum;               // Block number of data block.
} FILE;

typedef struct {
  UINTN                   Signature;
  UINT64                  StartBlock;
  UINT64                  LastBlock;
  UINT32                  BlockSize;
  EFI_HANDLE              *Handle;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  EFI_DISK_IO_PROTOCOL    *DiskIo;
  EFI_DISK_IO2_PROTOCOL   *DiskIo2;
} EXT_PRIVATE_DATA;

/**
  Gives the info of device block config.

  @param[in]  DevData               Device privete data.
  @param[in]  BlockNum              Block number to start.
  @param[in]  Size                  Size to read block.
  @param[out] Buf                   Buffer to read the block data.
  @param[out] RSize                 Actual read size.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
BDevStrategy (
  IN  VOID                          *DevData,
  IN  DADDRESS                      BlockNum,
  IN  UINT32                        Size,
  OUT VOID                          *Buf,
  OUT UINT32                        *RSize
  );

/**
  Validate EXT2 Superblock.

  @param[in]  FsHandle              EXT file system handle.
  @param[in]  File                  File for which super block needs to be read.
  @param[out] RExt2Fs               EXT2FS meta data to retreive.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
Ext2SbValidate (
  IN  CONST EFI_HANDLE              FsHandle,
  IN  CONST OPEN_FILE               *File OPTIONAL,
  OUT EXT2FS                        *RExt2Fs OPTIONAL
  );

/**
  Open struct file.

  @param[in]     Path               Path to locate the file
  @param[in/out] File               The struct having the device and file info

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
Ext2fsOpen (
  IN     CHAR8                      *Path,
  IN OUT OPEN_FILE                  *File
  );

/**
  Close the opened file.

  @param[in/out] File               File to be closed.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
Ext2fsClose (
  IN OUT OPEN_FILE                  *File
  );

/**
  Copy a portion of a FILE into a memory.
  Cross block boundaries when necessary

  @param[in/out] File               File handle to be read.
  @param[in]     Start              Start address of read buffer.
  @param[in]     Size               Size to be read.
  @param[out]    ResId              Actual read size.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
Ext2fsRead (
  IN OUT OPEN_FILE                  *File,
  IN     VOID                       *Start,
  IN     UINT32                     Size,
  OUT    UINT32                     *ResId
  );

/**
  Read Superblock of the file.

  @param[in]     File               File for which super block needs to be read.
  @param[in/out] FileSystem         Fs on which super block is computed.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
ReadSBlock (
  IN     OPEN_FILE                  *File,
  IN OUT M_EXT2FS                   *FileSystem
  );

/**
  Read group descriptor of the file.

  @param[in/out] File               File for which group descriptor needs to be read.
  @param[in]     FileSystem         Fs on which super block is computed.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
ReadGDBlock (
  IN OUT OPEN_FILE                  *File,
  IN     M_EXT2FS                   *FileSystem
  );

/**
  Read a new inode into a FILE structure.

  @param[in]     INumber            Inode number
  @param[in/out] File               Pointer to open file struct.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
ReadInode (
  IN  INODE32                       INumber,
  IN  OPEN_FILE                     *File
  );

/**
  Read a portion of a FILE into an internal buffer.

  Return the location in the buffer and the amount in the buffer.

  @param[in]  File                  Pointer to the open file.
  @param[out] BufferPtr             buffer corresponding to offset
  @param[out] SizePtr               Size of remainder of buffer.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Operation failed.

**/
EFI_STATUS
EFIAPI
BufReadFile (
  IN  OPEN_FILE                     *File,
  OUT CHAR8                         **BufferPtr,
  OUT UINT32                        *SizePtr
  );

/**
  Gets the size of the file from descriptor.

  @param[in]  File                  File to be closed.

  @retval  UINT32                   Size of the file from descriptor.

**/
UINT32
EFIAPI
Ext2fsFileSize (
  IN  OPEN_FILE                     *File
  );

/**
  Dump the file system super block information.

  @param[in]  FileSystem            Pointer to file system.

  @retval  NA

**/
VOID
EFIAPI
DumpSBlock (
  IN  M_EXT2FS                      *FileSystem
  );

/**
  Dump the file group descriptor block info.

  @param[in]  FileSystem            Pointer to file system.

  @retval  NA

**/
VOID
EFIAPI
DumpGroupDesBlock (
  IN  M_EXT2FS                      *FileSystem
  );

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
  );

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
  );

/**
  Open a file by its name and return its file handle.

  @param[in]  FsHandle              File system handle.
  @param[in]  FileName              The file name to get.
  @param[out] FileHandle            File handle,

  @retval  EFI_SUCCESS              The file opened correctly.
  @retval  EFI_INVALID_PARAMETER    Parameter is not valid.
  @retval  EFI_DEVICE_ERROR         A device error occurred.
  @retval  EFI_NOT_FOUND            A requested file cannot be found.
  @retval  EFI_OUT_OF_RESOURCES     Insufficant memory resource pool.

**/
EFI_STATUS
EFIAPI
ExtFsOpenFile (
  IN  EFI_HANDLE                    FsHandle,
  IN  CHAR16                        *FileName,
  OUT EFI_HANDLE                    *FileHandle
  );

/**
  Get file size by opened file handle.

  @param[in]  FileHandle            File handle.
  @param[out] FileSize              Pointer to file buffer size.

  @retval  EFI_SUCCESS              The file was loaded correctly.
  @retval  EFI_INVALID_PARAMETER    Parameter is not valid.

**/
EFI_STATUS
EFIAPI
ExtFsGetFileSize (
  IN  EFI_HANDLE                    FileHandle,
  OUT UINTN                         *FileSize
  );

/**
  Read file into memory by opened file handle.

  @param[in]  FileHandle            File handle
  @param[out] FileBufferPtr         Allocated file buffer pointer.
  @param[out] FileSize              Pointer to file buffer size.

  @retval  EFI_SUCCESS              The file was loaded correctly.
  @retval  EFI_INVALID_PARAMETER    Parameter is not valid.
  @retval  EFI_DEVICE_ERROR         A device error occurred.
  @retval  EFI_NOT_FOUND            A requested file cannot be found.
  @retval  EFI_OUT_OF_RESOURCES     Insufficant memory resource pool.
  @retval  EFI_BUFFER_TOO_SMALL     Buffer size is too small.

**/
EFI_STATUS
EFIAPI
ExtFsReadFile (
  IN  EFI_HANDLE                    FsHandle,
  IN  EFI_HANDLE                    FileHandle,
  OUT VOID                          **FileBufferPtr,
  OUT UINTN                         *FileSizePtr
  );

/**
  Close a file by opened file handle.

  @param[in]  FileHandle            File handle

  @retval  NA

**/
VOID
EFIAPI
ExtFsCloseFile (
  IN  EFI_HANDLE                    FileHandle
  );

#endif  // __EXT2_FS_H__
