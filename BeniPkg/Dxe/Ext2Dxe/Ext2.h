/**
*  @Package     : BeniPkg
*  @FileName    : Ext2.h
*  @Date        : 20211130
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This is a UEFI model driver used to to initialzie EXT2 file system.
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

#ifndef __EXT2_H__
#define __EXT2_H__

#include <Uefi.h>
#include <PiDxe.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Library/BeniDebugLib.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/UnicodeCollation.h>

typedef long int                    DADDRESS;
typedef long int                    OFFSET;
typedef unsigned long               ULONG;
typedef unsigned long               INODE;
typedef UINT32                      INODE32;
typedef INT32                       INDPTR;

//
// The EXT2 macros.
//
#define EXT2_VOLUME_SIGNATURE         SIGNATURE_32 ('e', 'x', 't', '2')
#define EXT2_OFILE_SIGNATURE          SIGNATURE_32 ('e', 'x', 't', 'i')
#define VOLUME_FROM_VOL_INTERFACE(a)  CR (a, EXT2_VOLUME, VolumeInterface, EXT2_VOLUME_SIGNATURE);
#define OFILE_FROM_FHAND(a)           CR (a, OPEN_FILE, Handle, EXT2_OFILE_SIGNATURE)
#define HOWMANY(x, y)                 (((x)+((y)-1))/(y))
//
// Size of on-disk Inode.
//
#define EXT2_REV0_DINODE_SIZE       sizeof(EXTFS_DINODE)
#define EXT2_DINODE_SIZE(fs)        ((fs)->Ext2Fs.Ext2FsRev > E2FS_REV0 ?    \
                                     (fs)->Ext2Fs.Ext2FsInodeSize :    \
                                     EXT2_REV0_DINODE_SIZE)
//
// Macros for handling Inode numbers:
//   Inode number to file system block offset.
//   Inode number to cylinder group number.
//   Inode number to file system block address.
//
#define INOTOCG(fs, x)              (((x) - 1) / (fs)->Ext2Fs.Ext2FsINodesPerGroup)
#define INODETOFSBA(fs, x)          ((fs)->Ext2FsGrpDes[INOTOCG((fs), (x))].Ext2BGDInodeTables + (((x) - 1) % (fs)->Ext2Fs.Ext2FsINodesPerGroup) / (fs)->Ext2FsInodesPerBlock)
#define INODETOFSBO(fs, x)          ((((x) - 1) % (fs)->Ext2Fs.Ext2FsINodesPerGroup) % (fs)->Ext2FsInodesPerBlock)
//
// Turn file system block numbers into disk block addresses.
// This maps file system blocks to device size blocks.
//
#define FSBTODB(fs, b)              ((b) << (fs)->Ext2FsFsbtobd)
#define DBTOFSB(fs, b)              ((b) >> (fs)->Ext2FsFsbtobd)
//
// The path name on which the file system is mounted is maintained
// in fs_fsmnt. MAXMNTLEN defines the amount of space allocated in
// the super block for this name.
//
#define MAXMNTLEN                   512
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
// The root inode is the root of the file system. Inode 0 can't be used for
// normal purposes and bad blocks are normally linked to inode 1, thus
// the root inode is 2.
// Inode 3 to 10 are reserved in ext2fs.
//
#define EXT2_BADBLKINO              ((INODE)1)
#define EXT2_ROOTINO                ((INODE)2)
#define EXT2_ACLIDXINO              ((INODE)3)
#define EXT2_ACLDATAINO             ((INODE)4)
#define EXT2_BOOTLOADERINO          ((INODE)5)
#define EXT2_UNDELDIRINO            ((INODE)6)
#define EXT2_RESIZEINO              ((INODE)7)
#define EXT2_JOURNALINO             ((INODE)8)
#define EXT2_FIRSTINO               ((INODE)11)
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
#define    EXT2FS_MAXNAMLEN    255

//
// File permissions.
//
#define EXT2_IEXEC                  0000100 // Executable.
#define EXT2_IWRITE                 0000200 // Writable.
#define EXT2_IREAD                  0000400 // Readable.
#define EXT2_ISVTX                  0001000 // Sticky bit.
#define EXT2_ISGID                  0002000 // Set-gid.
#define EXT2_ISUID                  0004000 // Set-uid.
//
// File types.
//
#define EXT2_IFMT                   0170000 // Mask of file type.
#define EXT2_IFIFO                  0010000 // Named pipe (fifo).
#define EXT2_IFCHR                  0020000 // Character device.
#define EXT2_IFDIR                  0040000 // Directory file.
#define EXT2_IFBLK                  0060000 // Block device.
#define EXT2_IFREG                  0100000 // Regular file.
#define EXT2_IFLNK                  0120000 // Symbolic link.
#define EXT2_IFSOCK                 0140000 // UNIX domain socket.
//
// File flags.
//
#define EXT2_SECRM                  0x00000001  // Secure deletion.
#define EXT2_UNRM                   0x00000002  // Undelete.
#define EXT2_COMPR                  0x00000004  // Compress file.
#define EXT2_SYNC                   0x00000008  // Synchronous updates.
#define EXT2_IMMUTABLE              0x00000010  // Immutable file.
#define EXT2_APPEND                 0x00000020  // Writes to file may only append.
#define EXT2_NODUMP                 0x00000040  // Do not dump file.
#define EXT4_EXTENTS                0x00080000  // Inode uses extents.

#define EXT4_MAX_HEADER_EXTENT_ENTRIES   4
#define EXT4_EXTENT_HEADER_MAGIC         0xF30A
//
// EXT2FS meta data is stored in little-endian byte order. These macros
// help with reading the meta data.
//
#define E2FS_SBLOAD(old, new)       CopyMem((new), (old), SBSIZE);
#define E2FS_CGLOAD(old, new, size) CopyMem((new), (old), (size));
#define E2FS_SBSAVE(old, new)       CopyMem((new), (old), SBSIZE);
#define E2FS_CGSAVE(old, new, size) CopyMem((new), (old), (size));
//
// Ext2Fs needs byte swapping on big-endian systems
//
#define E2FSILOAD(OLD, NEW)         CopyMem((NEW),(OLD),sizeof(EXTFS_DINODE))
#define E2FSISAVE(OLD, NEW)         CopyMem((NEW),(OLD),sizeof(EXTFS_DINODE))
//
// Number of indirects in a file system block.
//
#define NINDIR(fs)                  ((fs)->Ext2FsBlockSize / sizeof(UINT32))
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
// TODO: We need to enable support for
//       EXT FS recovery handling at some
//       point to ensure that we don't
//       encounter a real file error.
//
#define EXT2F_INCOMPAT_RECOVER      0x0004
#define EXT2F_INCOMPAT_64BIT        0x0080
#define EXT2F_INCOMPAT_EXTENTS      0x0040
#define EXT2F_INCOMPAT_FLEX_BG      0x0200
/**
  A dinode contains all the meta-data associated with a UFS file.
  This structure defines the on-disk format of a dinode. Since
  this structure describes an on-disk structure, all its fields
  are defined by types with precise widths.
**/
#define NDADDR                      12          // Direct addresses in inode.
#define NIADDR                      3           // Indirect addresses in inode.
#define EXT2_MAXSYMLINKLEN          ((NDADDR + NIADDR) * sizeof (UINT32))
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
// File types.
//
#define EXT2_IFMT                   0170000 // Mask of file type.
#define EXT2_IFIFO                  0010000 // Named pipe (fifo).
#define EXT2_IFCHR                  0020000 // Character device.
#define EXT2_IFDIR                  0040000 // Directory file.
#define EXT2_IFBLK                  0060000 // Block device.
#define EXT2_IFREG                  0100000 // Regular file.
#define EXT2_IFLNK                  0120000 // Symbolic link.
#define EXT2_IFSOCK                 0140000 // UNIX domain socket.

//
// Super block for an ext2fs file system, 1024 bytes.
//
typedef struct {
  UINT32   Ext2FsINodeCount;        // Inode count.
  UINT32   Ext2FsBlockCount;        // Blocks count.
  UINT32   Ext2FsRsvdBlockCount;    // Reserved blocks count.
  UINT32   Ext2FsFreeBlockCount;    // Free blocks count.
  UINT32   Ext2FsFreeINodeCount;    // Rree inodes count.
  UINT32   Ext2FsFirstDataBlock;    // First data block.
  UINT32   Ext2FsLogBlockSize;      // Block size = 1024*(2^Ext2FsLogBlockSize).
  UINT32   Ext2FsFragmentSize;      // Fragment size.
  UINT32   Ext2FsBlocksPerGroup;    // Blocks per group.
  UINT32   Ext2FsFragsPerGroup;     // Frags per group.
  UINT32   Ext2FsINodesPerGroup;    // Inodes per group.
  UINT32   Ext2FsMountTime;         // Mount time.
  UINT32   Ext2FsWriteTime;         // Write time.
  UINT16   Ext2FsMountCount;        // Mount count.
  UINT16   Ext2FsMaxMountCount;     // Max mount count.
  UINT16   Ext2FsMagic;             // Magic number.
  UINT16   Ext2FsState;             // File system state.
  UINT16   Ext2FsBehavior;          // Behavior on errors.
  UINT16   Ext2FsMinorRev;          // Minor revision level.
  UINT32   Ext2FsLastFsck;          // Time of last fsck.
  UINT32   Ext2FsFsckInterval;      // Max time between fscks.
  UINT32   Ext2FsCreator;           // Creator OS.
  UINT32   Ext2FsRev;               // Revision level.
  UINT16   Ext2FsRsvdUid;           // Default uid for reserved blocks.
  UINT16   Ext2FsRsvdGid;           // Default gid for reserved blocks.
  //
  // EXT2_DYNAMIC_REV superblocks.
  //
  UINT32   Ext2FsFirstInode;        // First non-reserved inode.
  UINT16   Ext2FsInodeSize;         // Size of inode structure.
  UINT16   Ext2FsBlockGrpNum;       // Block grp number of this sblk.
  UINT32   Ext2FsFeaturesCompat;    // Compatible feature set.
  UINT32   Ext2FsFeaturesIncompat;  // Incompatible feature set.
  UINT32   Ext2FsFeaturesROCompat;  // RO-compatible feature set.
  UINT8    Ext2FsUuid[16];          // 128-bit uuid for volume.
  CHAR8    Ext2FsVolumeName[16];    // Volume name.
  CHAR8    Ext2FsFSMnt[64];         // Name mounted on.
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
// EXT2 INode.
//
typedef struct {
  UINT16    Ext2DInodeMode;                     //   0: 文件模式，保存了访问权限和文件类型（目录，设备文件等）
  UINT16    Ext2DInodeUid;                      //   2: 所有者UID的低16位
  UINT32    Ext2DInodeSize;                     //   4: 文件长度，以字节为单位
  UINT32    Ext2DInodeAcessTime;                //   8: 上一次访问文件的时间戳
  UINT32    Ext2DInodeCreatTime;                //  12: 上一次修改INode的时间戳
  UINT32    Ext2DInodeModificationTime;         //  16: 上一次修改文件的时间戳
  UINT32    Ext2DInodeDeletionTime;             //  20: 文件删除的时间戳
  UINT16    Ext2DInodeGid;                      //  24: 组ID的低16位
  UINT16    Ext2DInodeLinkCount;                //  26: 指向INode的硬链接的数目
  UINT32    Ext2DInodeBlockCount;               //  28: 文件长度，以块为单位
  UINT32    Ext2DInodeStatusFlags;              //  32: 文件标志
  UINT32    Ext2DInodeLinuxRsvd1;               //  36: 保留
  UINT32    Ext2DInodeBlocks[NDADDR + NIADDR];  //  40: 块指针（块号）
  UINT32    Ext2DInodeGen;                      // 100: 文件版本，用户NFS
  UINT32    Ext2DInodeFileAcl;                  // 104: 文件ACL（访问控制表）
  UINT32    Ext2DInodeDirAcl;                   // 108: 目录ACL（访问控制表）
  UINT32    Ext2DInodeFragmentAddr;             // 112: 碎片地址（未使用）
  UINT8     Ext2DInodeFragmentNum;              // 116: 碎片编号（未使用）
  UINT8     Ext2DInodeFragmentSize;             // 117: 锁片长度（未使用）
  UINT16    Ext2DInodeLinuxRsvd2;               // 118: 保留
  UINT16    Ext2DInodeUidHigh;                  // 120: 所有者UID的高16位
  UINT16    Ext2DInodeGidHigh;                  // 122: 组ID的高16位
  UINT32    Ext2DInodeLinuxRsvd3;               // 124: 保留
} EXTFS_DINODE;

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

//
// Opened file struct for EXT2 file system.
//
typedef struct {
  UINTN                   Signature;               // Signature.

  EFI_FILE_PROTOCOL       Handle;                  // Instance for EFI_FILE_PROTOCOL.

  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  EFI_DISK_IO_PROTOCOL    *DiskIo;
  EFI_DISK_IO2_PROTOCOL   *DiskIo2;

  INT32                   FileFlags;               //
  FILE                    FileSystemSpecificData;  // File system specific data.
  OFFSET                  FileOffset;              // Current file offset (F_RAW).
  CHAR8                   *FileNamePtr;            // File name.
} OPEN_FILE;

//
// EXT2 Volume.
//
typedef struct {
  UINTN                             Signature;

  EFI_HANDLE                        Handle;
  BOOLEAN                           Valid;
  BOOLEAN                           DiskError;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   VolumeInterface;

  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_DISK_IO_PROTOCOL              *DiskIo;
  EFI_DISK_IO2_PROTOCOL             *DiskIo2;
  UINT32                            MediaId;
  BOOLEAN                           ReadOnly;

  M_EXT2FS                          SuperBlock;

  OPEN_FILE                         Root;
} EXT2_VOLUME;

//
// A directory consists of some number of blocks of e2fs_bsize bytes.
//
// Each block contains some number of directory entry
// structures, which are of variable length.  Each directory entry has
// a struct direct at the front of it, containing its inode number,
// the length of the entry, and the length of the name contained in
// the entry.  These are followed by the name padded to a 4 byte boundary
// with null bytes.  All names are guaranteed null terminated.
// The maximum length of a name in a directory is EXT2FS_MAXNAMLEN.
//
// The macro EXT2FS_DIRSIZ(fmt, dp) gives the amount of space required to
// represent a directory entry.  Free space in a directory is represented by
// entries which have dp->Ext2DirectRecLen > DIRSIZ(fmt, dp).  All d2fs_bsize bytes
// in a directory block are claimed by the directory entries.  This
// usually results in the last entry in a directory having a large
// dp->Ext2DirectRecLen.  When entries are deleted from a directory, the
// space is returned to the previous entry in the same directory
// block by increasing its dp->Ext2DirectRecLen.  If the first entry of
// a directory block is free, then its dp->Ext2DirectInodeNumber is set to 0.
// Entries other than the first in a directory do not normally have
// dp->Ext2DirectInodeNumber set to 0.
// Ext2 rev 0 has a 16 bits Ext2DirectNameLen. For Ext2 vev 1 this has been split
// into a 8 bits Ext2DirectNameLen and 8 bits Ext2DirectType (looks like ffs, isnt't it ? :)
// It's safe to use this for rev 0 as well because all ext2 are little-endian.
//
typedef struct {
  UINT16    EhMagic;      // magic number: 0xF30A
  UINT16    EhEntries;    // number of valid entries
  UINT16    EhMax;        // capacity of store in entries
  UINT16    EhDepth;      // the depth of extent tree
  UINT32    EhGen;        // generation of extent tree
} EXT4_EXTENT_HEADER;

typedef struct {
  UINT32    EiBlk;        // indexes logical blocks
  UINT32    EiLeafLo;     // points to physical block of the next level
  UINT16    EiLeafHi;     // high 16 bits of physical block
  UINT16    EiUnused;
} EXT4_EXTENT_INDEX;

typedef struct {
  UINT32    Eblk;         // first logical block
  UINT16    Elen;         // number of blocks
  UINT16    EstartHi;     // high 16 bits of physical block
  UINT32    EstartLo;     // low 32 bits of physical block
} EXT4_EXTENT;

typedef struct {
  EXT4_EXTENT_HEADER Eheader;
  union {
    EXT4_EXTENT_INDEX  Eindex[EXT4_MAX_HEADER_EXTENT_ENTRIES];
    EXT4_EXTENT        Extent[EXT4_MAX_HEADER_EXTENT_ENTRIES];
  } Enodes;
} EXT4_EXTENT_TABLE;

typedef struct {
  UINT32 Ext2DirectInodeNumber;             // inode number of entry
  UINT16 Ext2DirectRecLen;                  // length of this record
  UINT8 Ext2DirectNameLen;                  // length of string in d_name
  UINT8 Ext2DirectType;                     // file type
  CHAR8 Ext2DirectName[EXT2FS_MAXNAMLEN];   // name with length<=EXT2FS_MAXNAMLEN
} EXT2FS_DIRECT;

//
// Function Prototypes
//
// ------------------------------- Misc.c -------------------------------
/**
  Detects EXT2 file system on sisk and set relevant fields of Volume.

  @param[in]  Volume                The volume structure.

  @retval  EFI_SUCCESS              The EXT2 File System is detected successfully
  @retval  EFI_UNSUPPORTED          The volume is not FAT file system.
  @retval  EFI_VOLUME_CORRUPTED     The volume is corrupted.

**/
EFI_STATUS
Ext2OpenDevice (
  IN OUT EXT2_VOLUME                *Volume
  );

/**
  Read group descriptor of the file.

  @param[in]  Volume                Pointer to EXT2_VOLUME.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ReadGDBlock (
  IN  EXT2_VOLUME                   *Volume
  );
// ------------------------------- Misc.c -------------------------------

// -------------------------------- Lib.c -------------------------------
/**
  Reads the requested number of blocks from the specified block device.

  The function reads the requested number of blocks from the device. All the
  blocks are read, or an error is returned. If there is no media in the device,
  the function returns EFI_NO_MEDIA.

  @param[in]  BlockIo               The pointer to EFI_BLOCK_IO_PROTOCOL.
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
  IN  EFI_BLOCK_IO_PROTOCOL         *BlockIo,
  IN  EFI_LBA                       StartLBA,
  IN  UINTN                         BufferSize,
  OUT VOID                          *Buffer
  );

/**
  Read a new inode into a FILE structure.

  @param[in]     INumber            Inode number.
  @param[in/out] File               Pointer to open file struct.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
ReadInode (
  IN  INODE32                       INumber,
  IN  OPEN_FILE                     *File
  );

/**
  Given an offset in a FILE, find the disk block number that
  contains that block.

  @param[in]  File                  Pointer to an Open file.
  @param[in]  FileBlock             Block to find the file.
  @param[out] DiskBlockPtr          Pointer to the disk which contains block.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
BlockMap (
  IN  OPEN_FILE                     *File,
  IN  INDPTR                        FileBlock,
  OUT INDPTR                        *DiskBlockPtr
  );

/**
  Search a directory for a Name and return its inode number.

  @param[in]     Name               Name to compare with.
  @param[in]     Length             Length of the dir name.
  @param[in/out] File               Pointer to file private data.
  @param[out]    INumPtr            Pointer to Inode number.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
SearchDirectory (
  IN     CHAR8                      *Name,
  IN     INT32                      Length,
  IN OUT OPEN_FILE                  *File,
  OUT    INODE32                    *INumPtr
  );

/**
  Read a portion of a FILE into an internal buffer.
  Return the location in the buffer and the amount in the buffer.

  @param[in]  File                  Pointer to the open file.
  @param[out] BufferPtr             Buffer corresponding to offset.
  @param[out] SizePtr               Size of remainder of buffer.

  @retval  EFI_SUCCESS              Operation succeeded.
  @retval  Others                   Error happened.

**/
EFI_STATUS
EFIAPI
BufReadFile (
  IN  OPEN_FILE                     *File,
  OUT CHAR8                         **BufferPtr,
  OUT UINT32                        *SizePtr
  );

/**
  Dump the file system super block information.

  @param[in]  Volume                Pointer to EXT2_VOLUME.

  @retval  NA

**/
VOID
EFIAPI
DumpSBlock (
  IN  EXT2_VOLUME                   *Volume
  );

/**
  Dump the file group descriptor block info.

  @param[in]  Volume                Pointer to EXT2_VOLUME.

  @retval  NA

**/
VOID
EFIAPI
DumpGroupDesBlock (
  IN  EXT2_VOLUME                   *Volume
  );
// -------------------------------- Lib.c -------------------------------

// ------------------------------ Volume.c ------------------------------
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
  );

/**
  Called by Ext2DriverBindingStop(), Abandon the volume.

  @param[in]  Volume                The volume to be abandoned.

  @retval  EFI_SUCCESS              Abandoned the volume successfully.
  @return  Others                   Can not uninstall the protocol interfaces.

**/
EFI_STATUS
Ext2AbandonVolume (
  IN  EXT2_VOLUME                   *Volume
  );

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
  );

/**
  Free volume structure.

  @param[in]  Volume                The volume structure to be freed.

  @retval  NA

**/
VOID
Ext2FreeVolume (
  IN  EXT2_VOLUME                   *Volume
  );
// ------------------------------ Volume.c ------------------------------

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Flushes & Closes the file handle.

  @param[in]  FHand                 Handle to the file to delete.

  @retval  EFI_SUCCESS              Closed the file successfully.

**/
EFI_STATUS
EFIAPI
Ext2Close (
  IN  EFI_FILE_PROTOCOL             *FHand
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

//
// Global Variables.
//
extern EFI_COMPONENT_NAME_PROTOCOL  gExt2ComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gExt2ComponentName2;
extern EFI_DRIVER_BINDING_PROTOCOL  gExt2DriverBinding;
extern EFI_FILE_PROTOCOL            gExt2FileInterface;

#endif // __EXT2_H__
