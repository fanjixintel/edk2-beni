/**
*  @Package     : BeniPkg
*  @FileName    : Ext2Fs.h
*  @Date        : 20211213
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This command is used for EXT2 file system test.
*
*  @History:
*    20211213: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _BENI_EXT2FS__H_
#define _BENI_EXT2FS__H_

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>

/**
  Each disk drive contains some number of file systems.
  A file system consists of a number of cylinder groups.
  Each cylinder group has inodes and data.

  A file system is described by its super-block, which in turn
  describes the cylinder groups.  The super-block is critical
  data and is replicated in each cylinder group to protect against
  catastrophic loss.  This is done at `newfs' time and the critical
  super-block data does not change, so the copies need not be
  referenced further unless disaster strikes.

  The first boot and super blocks are given in absolute disk addresses.
  The byte-offset forms are preferred, as they don't imply a sector size.
**/
#define BBSIZE      1024
#define SBSIZE      1024
#define BBOFF       ((UINTN)(0))
#define SBOFF       ((UINTN)(BBOFF + BBSIZE))

/**
  Addresses stored in inodes are capable of addressing blocks
  XXX

  MINBSIZE is the smallest allowable block size.
  MINBSIZE must be big enough to hold a cylinder group block,
  thus changes to (struct cg) must keep its size within MINBSIZE.
  Note that super blocks are always of size SBSIZE,
  and that both SBSIZE and MAXBSIZE must be >= MINBSIZE.
**/
#define LOG_MINBSIZE    10
#define MINBSIZE        (1 << LOG_MINBSIZE)

/**
  The path name on which the file system is mounted is maintained
  in fs_fsmnt. MAXMNTLEN defines the amount of space allocated in
  the super block for this name.
**/
#define MAXMNTLEN    512

/**
  MINFREE gives the minimum acceptable percentage of file system
  blocks which may be free. If the freelist drops below this level
  only the superuser may continue to allocate blocks. This may
  be set to 0 if no reserve of free blocks is deemed necessary,
  however throughput drops by fifty percent if the file system
  is run at between 95% and 100% full; thus the minimum default
  value of fs_minfree is 5%. However, to get good clustering
  performance, 10% is a better choice. hence we use 10% as our
  default value. With 10% free space, fragmentation is not a
  problem, so we choose to optimize for time.
**/
#define MINFREE    5

//
// Super block for an ext2fs file system.
//
typedef struct {
  UINT32  Ext2FsINodeCount;         // Inode count
  UINT32  Ext2FsBlockCount;         // blocks count
  UINT32  Ext2FsRsvdBlockCount;     // reserved blocks count
  UINT32  Ext2FsFreeBlockCount;     // free blocks count
  UINT32  Ext2FsFreeINodeCount;     // free inodes count
  UINT32  Ext2FsFirstDataBlock;     // first data block
  UINT32  Ext2FsLogBlockSize;       // block size = 1024*(2^Ext2FsLogBlockSize)
  UINT32  Ext2FsFragmentSize;       // fragment size
  UINT32  Ext2FsBlocksPerGroup;     // blocks per group
  UINT32  Ext2FsFragsPerGroup;      // frags per group
  UINT32  Ext2FsINodesPerGroup;     // inodes per group
  UINT32  Ext2FsMountTime;          // mount time
  UINT32  Ext2FsWriteTime;          // write time
  UINT16  Ext2FsMountCount;         // mount count
  UINT16  Ext2FsMaxMountCount;      // max mount count
  UINT16  Ext2FsMagic;              // magic number
  UINT16  Ext2FsState;              // file system state
  UINT16  Ext2FsBehavior;           // behavior on errors
  UINT16  Ext2FsMinorRev;           // minor revision level
  UINT32  Ext2FsLastFsck;           // time of last fsck
  UINT32  Ext2FsFsckInterval;       // max time between fscks
  UINT32  Ext2FsCreator;            // creator OS
  UINT32  Ext2FsRev;                // revision level
  UINT16  Ext2FsRsvdUid;            // default uid for reserved blocks
  UINT16  Ext2FsRsvdGid;            // default gid for reserved blocks
  //
  // EXT2_DYNAMIC_REV superblocks.
  //
  UINT32  Ext2FsFirstInode;         /* first non-reserved inode */
  UINT16  Ext2FsInodeSize;          /* size of inode structure */
  UINT16  Ext2FsBlockGrpNum;        /* block grp number of this sblk*/
  UINT32  Ext2FsFeaturesCompat;     /*  compatible feature set */
  UINT32  Ext2FsFeaturesIncompat;   /* incompatible feature set */
  UINT32  Ext2FsFeaturesROCompat;   /* RO-compatible feature set */
  UINT8   Ext2FsUuid[16];           /* 128-bit uuid for volume */
  CHAR8   Ext2FsVolumeName[16];     /* volume name */
  CHAR8   Ext2FsFSMnt[64];          /* name mounted on */
  UINT32  Ext2FsAlgorithm;          /* For compression */
  UINT8   Ext2FsPreAlloc;           /* # of blocks to preallocate */
  UINT8   Ext2FsDirPreAlloc;        /* # of blocks to preallocate for dir */
  UINT16  Ext2FsRsvdGDBlock;        /* # of reserved gd blocks for resize */
  UINT32  Rsvd2[11];
  UINT16  Rsvd3;
  UINT16  Ext2FsGDSize;             /* size of group descriptors, in bytes, if the 64bit incompat feature flag is set */
  UINT32  Rsvd4[192];
} EXT2FS;

//
// Ext2 file system block group descriptor.
//
typedef struct {
  UINT32 Ext2BGDBlockBitmap;    /* blocks bitmap block */
  UINT32 Ext2BGDInodeBitmap;    /* inodes bitmap block */
  UINT32 Ext2BGDInodeTables;    /* inodes table block  */
  UINT16 Ext2BGDFreeBlocks;     /* number of free blocks */
  UINT16 Ext2BGDFreeInodes;     /* number of free inodes */
  UINT16 Ext2BGDNumDir;         /* number of directories */
  UINT16 Rsvd;
  UINT32 Rsvd2[5];
  UINT32 Ext2BGDInodeTablesHi;  /* upper 32 bits of inodes table block, if the 64bit incompat feature flag is set */
  UINT32 Rsvd3[5];
} EXT2GD;

//
// In-memory data for ext2fs
//
typedef struct {
  EXT2FS   Ext2Fs;
  UINT8    Ext2FsFSMnt[MAXMNTLEN];   // name mounted on
  CHAR8    Ext2FsReadOnly;           // mounted read-only flag
  CHAR8    Ext2FsModified;           // super block modified flag
  INT32    Ext2FsBlockSize;          // block size
  INT32    Ext2FsLogicalBlock;       // ``lblkno'' calc of logical blkno
  INT32    Ext2FsBlockOffset;        // ``blkoff'' calc of blk offsets
  INT64    Ext2FsQuadBlockOffset;    // ~fs_bmask - for use with quad size
  INT32    Ext2FsFsbtobd;            // FSBTODB and DBTOFSB shift constant
  INT32    Ext2FsNumCylinder;        // number of cylinder groups
  INT32    Ext2FsNumGrpDesBlock;     // number of group descriptor block
  INT32    Ext2FsInodesPerBlock;     // number of inodes per block
  INT32    Ext2FsInodesTablePerGrp;  // number of inode table per group
  UINT32   Ext2FsGDSize;             // size of group descriptors
  EXT2GD  *Ext2FsGrpDes;             // group descriptors
} M_EXT2FS;

typedef struct {
  UINTN                             Signature;

  EFI_HANDLE                        Handle;
  BOOLEAN                           Valid;
  BOOLEAN                           DiskError;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   VolumeInterface;

  //
  // If opened, the parent handle and BlockIo interface
  //
  EFI_BLOCK_IO_PROTOCOL           *BlockIo;
  EFI_DISK_IO_PROTOCOL            *DiskIo;
  EFI_DISK_IO2_PROTOCOL           *DiskIo2;
  UINT32                          MediaId;
  BOOLEAN                         ReadOnly;

  //
  // EXT2 super block.
  //
  EXT2FS                          SuperBlock;
} EXT2_VOLUME;


#define EXT2_VOLUME_SIGNATURE       SIGNATURE_32 ('e', 'x', 't', '2')

//
//  Filesystem identification
//
#define E2FS_MAGIC   0xEF53 /* the ext2fs magic number */
#define E2FS_REV0    0      /* GOOD_OLD revision */
#define E2FS_REV1    1      /* Support compat/incompat features */
//
// Compatible/incompatible features
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

/**
  Features supported in this implementation

  We support the following REV1 features:
  - EXT2F_ROCOMPAT_SPARSESUPER
     superblock backups stored only in cg_has_sb(bno) groups
  - EXT2F_ROCOMPAT_LARGEFILE
     use e2di_dacl in EXTFS_DINODE to store
     upper 32bit of size for >2GB files
  - EXT2F_INCOMPAT_FTYPE
     store file type to e2d_type in EXT2FS_direct
     (on REV0 e2d_namlen is UINT16 and no e2d_type, like ffs)
**/
#define EXT2F_COMPAT_SUPP        0x0000
#define EXT2F_ROCOMPAT_SUPP      (EXT2F_ROCOMPAT_SPARSESUPER \
                                 | EXT2F_ROCOMPAT_LARGEFILE)
#define EXT2F_INCOMPAT_SUPP      (EXT2F_INCOMPAT_FTYPE    \
                                 | EXT2F_INCOMPAT_RECOVER \
                                 | EXT2F_INCOMPAT_64BIT   \
                                 | EXT2F_INCOMPAT_EXTENTS \
                                 | EXT2F_INCOMPAT_FLEX_BG)

//
//  Definitions of behavior on errors
//
#define E2FS_BEH_CONTINUE   1   /* continue operation */
#define E2FS_BEH_READONLY   2   /* remount fs read only */
#define E2FS_BEH_PANIC      3   /* cause panic */
#define E2FS_BEH_DEFAULT    E2FS_BEH_CONTINUE

//
//  OS identification
//
#define E2FS_OS_LINUX   0
#define E2FS_OS_HURD    1
#define E2FS_OS_MASIX   2
#define E2FS_OS_FREEBSD 3
#define E2FS_OS_LITES   4

//
//  Filesystem clean flags
//
#define E2FS_ISCLEAN 0x01
#define E2FS_ERRORS  0x02

/**
  The cache size (IND_CACHE_SZ) must be smaller or equal the number of pointers
  in the indirect blocks: NINDIR = Ext2FsBlockSize / sizeof(UINT32).
  For ext2fs minimal block size is 1kB and block pointer size is 4 (UINT32)
  so LN2_IND_CACHE_SZ <= 8 (cache size IND_CACHE_SZ=2^8=256)
  Optimal for file system speed is the biggest cache size possible.
**/
#define LN2_IND_CACHE_SZ    8
#define IND_CACHE_SZ        (1 << LN2_IND_CACHE_SZ)
#define IND_CACHE_MASK      (IND_CACHE_SZ - 1)

#define INDPTR      INT32

/**
  The root inode is the root of the file system.  Inode 0 can't be used for
  normal purposes and bad blocks are normally linked to inode 1, thus
  the root inode is 2.
  Inode 3 to 10 are reserved in ext2fs.
**/
#define    EXT2_BADBLKINO      ((UINT32)1)
#define    EXT2_ROOTINO        ((UINT32)2)
#define    EXT2_ACLIDXINO      ((UINT32)3)
#define    EXT2_ACLDATAINO     ((UINT32)4)
#define    EXT2_BOOTLOADERINO  ((UINT32)5)
#define    EXT2_UNDELDIRINO    ((UINT32)6)
#define    EXT2_RESIZEINO      ((UINT32)7)
#define    EXT2_JOURNALINO     ((UINT32)8)
#define    EXT2_FIRSTINO       ((UINT32)11)

//
// EXT2FS meta data is stored in little-endian byte order. These macros
// help with reading the meta data.
//
#define E2FS_SBLOAD(old, new) CopyMem((new), (old), SBSIZE);
#define E2FS_CGLOAD(old, new, size) CopyMem((new), (old), (size));
#define E2FS_SBSAVE(old, new) CopyMem((new), (old), SBSIZE);
#define E2FS_CGSAVE(old, new, size) CopyMem((new), (old), (size));

/**
  Turn file system block numbers into disk block addresses.
  This maps file system blocks to device size blocks.
**/
#define FSBTODB(fs, b)    ((b) << (fs)->Ext2FsFsbtobd)
#define DBTOFSB(fs, b)    ((b) >> (fs)->Ext2FsFsbtobd)

/**
  Macros for handling inode numbers:
      inode number to file system block offset.
      inode number to cylinder group number.
      inode number to file system block address.
**/
#define INOTOCG(fs, x)  (((x) - 1) / (fs)->Ext2Fs.Ext2FsINodesPerGroup)
#define INODETOFSBA(fs, x)  \
    ((fs)->Ext2FsGrpDes[INOTOCG((fs), (x))].Ext2BGDInodeTables + \
    (((x) - 1) % (fs)->Ext2Fs.Ext2FsINodesPerGroup) / (fs)->Ext2FsInodesPerBlock)
#define INODETOFSBO(fs, x)  ((((x) - 1) % (fs)->Ext2Fs.Ext2FsINodesPerGroup) % (fs)->Ext2FsInodesPerBlock)

/**
  Give cylinder group number for a file system block.
  Give cylinder group block number for a file system block.
**/
#define DTOG(fs, d) (((d) - (fs)->Ext2Fs.Ext2FsFirstDataBlock) / (fs)->Ext2Fs.Ext2FsFragsPerGroup)
#define DTOGD(fs, d) \
    (((d) - (fs)->Ext2Fs.Ext2FsFirstDataBlock) % (fs)->Ext2Fs.Ext2FsFragsPerGroup)

/**
  The following macros optimize certain frequently calculated
  quantities by using shifts and masks in place of divisions
  modulos and multiplications.
**/
#define BLOCKOFFSET(fs, loc)     /* calculates (loc % fs->Ext2FsBlockSize) */ \
    ((loc) & (fs)->Ext2FsQuadBlockOffset)
#define LBLKTOSIZE(fs, blk)      /* calculates (blk * fs->Ext2FsBlockSize) */ \
    ((blk) << (fs)->Ext2FsLogicalBlock)
#define LBLKNO(fs, loc)          /* calculates (loc / fs->Ext2FsBlockSize) */ \
    ((loc) >> (fs)->Ext2FsLogicalBlock)
#define BLKROUNDUP(fs, size)     /* calculates roundup(size, fs->Ext2FsBlockSize) */ \
    (((size) + (fs)->Ext2FsQuadBlockOffset) & (fs)->Ext2FsBlockOffset)
#define FRAGROUNDUP(fs, size)    /* calculates roundup(size, fs->Ext2FsBlockSize) */ \
    (((size) + (fs)->Ext2FsQuadBlockOffset) & (fs)->Ext2FsBlockOffset)
//
//  Determine the number of available frags given a
//  percentage to hold in reserve.
//
#define FREESPACE(fs) \
   ((fs)->Ext2Fs.Ext2FsFreeBlockCount - (fs)->Ext2Fs.Ext2FsRsvdBlockCount)

//
//  Number of indirects in a file system block.
//
#define NINDIR(fs) ((fs)->Ext2FsBlockSize / sizeof(UINT32))

#endif  // _BENI_EXT2FS__H_
