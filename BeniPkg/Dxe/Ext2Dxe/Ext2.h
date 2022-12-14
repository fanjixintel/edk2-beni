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
#include <Library/BeniMemLib.h>
#include <Library/BeniTimeLib.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/UnicodeCollation.h>

//
// Type definition
//
// --------------------------------- type def ---------------------------------
typedef long int                    DADDRESS;
typedef long int                    OFFSET;
typedef unsigned long               ULONG;
typedef unsigned long               INODE;
typedef UINT32                      INODE32;
typedef INT32                       INDPTR;
// --------------------------------- type def ---------------------------------

//
// Structures and macros
//
// ---------------------------------- Common ----------------------------------
//
// 物理块和EXT2的转换
//
#define FSBTODB(fs, b)              ((b) << (fs)->Ext2FsFsbtobd)
#define DBTOFSB(fs, b)              ((b) >> (fs)->Ext2FsFsbtobd)

#define MAXSYMLINKS                 1
#define MAXPATHLEN                  260
// ---------------------------------- Common ----------------------------------

// -------------------------------- SuperBlock --------------------------------
//
// 启动块和超级块位置以及大小
//
#define BBSIZE                      1024                        // 启动块大小
#define SBSIZE                      1024                        // 超级块大小
#define BBOFF                       ((OFFSET)(0))               // 启动块起始位置
#define SBOFF                       ((OFFSET)(BBOFF + BBSIZE))  // 超级块起始位置
//
// 文件系统标识和版本号
//
#define EXT2_SUPER_MAGIC            0xEF53
#define E2FS_REV0                   0
#define E2FS_REV1                   1
//
// 兼容和非兼容特性
//
#define EXT2F_COMPAT_SUPP           0x0000
#define EXT2F_COMPAT_PREALLOC       0x0001
#define EXT2F_COMPAT_HASJOURNAL     0x0004
#define EXT2F_COMPAT_RESIZE         0x0010
#define EXT2F_ROCOMPAT_SPARSESUPER  0x0001
#define EXT2F_ROCOMPAT_LARGEFILE    0x0002
#define EXT2F_ROCOMPAT_BTREE_DIR    0x0004
#define EXT2F_INCOMPAT_COMP         0x0001
#define EXT2F_INCOMPAT_FTYPE        0x0002
#define EXT2F_INCOMPAT_RECOVER      0x0004
#define EXT2F_INCOMPAT_64BIT        0x0080
#define EXT2F_INCOMPAT_EXTENTS      0x0040
#define EXT2F_INCOMPAT_FLEX_BG      0x0200
#define EXT2F_ROCOMPAT_SUPP         (EXT2F_ROCOMPAT_SPARSESUPER \
                                    | EXT2F_ROCOMPAT_LARGEFILE)
#define EXT2F_INCOMPAT_SUPP         (EXT2F_INCOMPAT_FTYPE    \
                                    | EXT2F_INCOMPAT_RECOVER \
                                    | EXT2F_INCOMPAT_64BIT   \
                                    | EXT2F_INCOMPAT_EXTENTS \
                                    | EXT2F_INCOMPAT_FLEX_BG)
//
// EXT2文件系统中的超级块，大小是1024个字节
//
typedef struct {
  UINT32   Ext2FsInodeCount;        //   0: Inode数据
  UINT32   Ext2FsBlockCount;        //   4: 块数据
  UINT32   Ext2FsRsvdBlockCount;    //   8: 已分配块的数据
  UINT32   Ext2FsFreeBlockCount;    //  12: 空闲块数目
  UINT32   Ext2FsFreeInodeCount;    //  16: 空闲Inode数据
  UINT32   Ext2FsFirstDataBlock;    //  20: 第一个数据块
  UINT32   Ext2FsLogBlockSize;      //  24: 块长度
  UINT32   Ext2FsFragmentSize;      //  28: 碎片长度
  UINT32   Ext2FsBlocksPerGroup;    //  32: 每个块组包含的块数
  UINT32   Ext2FsFragsPerGroup;     //  36: 每个块组包含的碎片
  UINT32   Ext2FsInodesPerGroup;    //  40: 每个块组的Inode数据，
                                    //      书中说默认设置的值是128，
                                    //      但是实际测试的值是2048
  UINT32   Ext2FsMountTime;         //  44: 装载时间
  UINT32   Ext2FsWriteTime;         //  48: 写入时间
  UINT16   Ext2FsMountCount;        //  52: 装载计数
  UINT16   Ext2FsMaxMountCount;     //  54: 最大装载计数
  UINT16   Ext2FsMagic;             //  56: 魔数，标记文件系统类型
  UINT16   Ext2FsState;             //  58: 文件系统状态
  UINT16   Ext2FsBehavior;          //  60: 检测到错误时的行为
  UINT16   Ext2FsMinorRev;          //  62: 副修订号
  UINT32   Ext2FsLastFsck;          //  64: 上一次检查的时间
  UINT32   Ext2FsFsckInterval;      //  68: 两次检查允许间隔的最长时间
  UINT32   Ext2FsCreator;           //  72: 创建文件系统的操作系统
  UINT32   Ext2FsRev;               //  76: 修订号
  UINT16   Ext2FsRsvdUid;           //  80: 能够使用保留块的默认UID
  UINT16   Ext2FsRsvdGid;           //  82: 能够使用保留块的默认GID
  //
  // 修订号大于等于1的版本才有下述的数据
  //
  UINT32   Ext2FsFirstInode;        //  84: 第一个非保留的Inode
  UINT16   Ext2FsInodeSize;         //  88: Inode结构的长度
  UINT16   Ext2FsBlockGrpNum;       //  90: 当前超级块所在的块组编号
  UINT32   Ext2FsFeaturesCompat;    //  92: 兼容特性集
  UINT32   Ext2FsFeaturesIncompat;  //  96: 不兼容特性集
  UINT32   Ext2FsFeaturesROCompat;  // 100: 只读兼容特性集
  UINT8    Ext2FsUuid[16];          // 104: 卷的128位UID
  CHAR8    Ext2FsVolumeName[16];    // 120: 卷名
  CHAR8    Ext2FsFSMnt[64];         // 136: 上一次装载的目录
  UINT32   Ext2FsAlgorithm;         // 200: 用于压缩
  UINT8    Ext2FsPreAlloc;          // 204: 试图预分配的块数
  UINT8    Ext2FsDirPreAlloc;       // 205: 试图为目录预分配的块数
  UINT16   Ext2FsRsvdGDBlock;       // 206: 为块描述符保留的块
  UINT32   Rsvd2[11];               // 208: 保留
  UINT16   Rsvd3;                   // 252: 保留
  UINT16   Ext2FsGDSize;            // 254: 开启64位模式时组描述符的大小（字节为单位）
  UINT32   Rsvd4[192];              // 256: 保留
} EXT2FS;
// -------------------------------- SuperBlock --------------------------------

// ----------------------------- Group Descriptor -----------------------------
//
// 组描述符大小
//
#define EXT2_GD_SIZE                32
//
// 用于计算组描述符相关参数的宏
//
#define HOWMANY(x, y)               (((x)+((y)-1))/(y))
//
// EXT2文件系统块描述符，如果不支持64位，则大小是32字节，否则是64字节，这里使用的是32字节
//
typedef struct {
  UINT32   Ext2BGDBlockBitmap;      // 块位图块所在位置
  UINT32   Ext2BGDInodeBitmap;      // Inode位图块所在位置
  UINT32   Ext2BGDInodeTables;      // Inode表块所在位置
  UINT16   Ext2BGDFreeBlocks;       // 空闲块数据
  UINT16   Ext2BGDFreeInodes;       // 空闲Inode数据
  UINT16   Ext2BGDNumDir;           // 目录数据
  UINT16   Rsvd;                    // 保留
  UINT32   Rsvd2[5];                // 保留
  UINT32   Ext2BGDInodeTablesHi;    // 如果支持64位模式，则表示Inode表块所在位置的高32位
  UINT32   Rsvd3[5];                // 保留
} EXT2GD;
// ----------------------------- Group Descriptor -----------------------------

// -------------------------------- EXT2 Inode --------------------------------
//
// 计算INODE结构体的大小，跟EXT2版本有关：
//   EXT2版本0中Inode的大小是128个字节
//   EXT2版本1开始，Inode的大小通过超级块中的成员指定
//
#define EXT2_REV0_DINODE_SIZE       sizeof(EXTFS_DINODE)
#define EXT2_DINODE_SIZE(fs)        ((fs)->Ext2Fs.Ext2FsRev > E2FS_REV0 ?    \
                                     (fs)->Ext2Fs.Ext2FsInodeSize :    \
                                     EXT2_REV0_DINODE_SIZE)
//
// Inode通过Index地址寻址，Index从1开始计数
// 在EXT2版本1中，可用Inode从第11个开始计算，前面的都是保留的
// 其中根目录对应的Inode是固定的2
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
// 计算Inode位置所在块组
// 计算Inode在块组中的Index
//
#define INOTOCG(fs, x)              (((x) - 1) / (fs)->Ext2Fs.Ext2FsInodesPerGroup)
#define INODETOFSBO(fs, x)          ((((x) - 1) % (fs)->Ext2Fs.Ext2FsInodesPerGroup) % (fs)->Ext2FsInodesPerBlock)
//
// Inode可以直接和间接寻址数据块
//
#define NDADDR                      12          // 直接寻址的Inode
#define NIADDR                      3           // 间接寻址的Inode
#define EXT2_MAXSYMLINKLEN          ((NDADDR + NIADDR) * sizeof (UINT32))
//
// Inode数据结构
//
typedef struct {
  UINT16    Ext2DInodeMode;                     //   0: 文件模式，保存了访问权限和文件类型（目录，设备文件等）
  UINT16    Ext2DInodeUid;                      //   2: 所有者UID的低16位
  UINT32    Ext2DInodeSize;                     //   4: 文件长度，以字节为单位
  UINT32    Ext2DInodeAcessTime;                //   8: 上一次访问文件的时间戳
  UINT32    Ext2DInodeCreatTime;                //  12: 上一次修改Inode的时间戳
  UINT32    Ext2DInodeModificationTime;         //  16: 上一次修改文件的时间戳
  UINT32    Ext2DInodeDeletionTime;             //  20: 文件删除的时间戳
  UINT16    Ext2DInodeGid;                      //  24: 组ID的低16位
  UINT16    Ext2DInodeLinkCount;                //  26: 指向Inode的硬链接的数目
  UINT32    Ext2DInodeBlockCount;               //  28: 文件长度，以块为单位
  UINT32    Ext2DInodeStatusFlags;              //  32: 文件标志
  UINT32    Ext2DInodeLinuxRsvd1;               //  36: 保留
  UINT32    Ext2DInodeBlocks[NDADDR + NIADDR];  //  40: 块指针（块号），指向文件数据块的指针保存在这里，
                                                //      默认情况下数组元素个数是12+3，前12个寻址直接块，
                                                //      后3个用于实现一次、二次和三次间接
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
//
// 文件模式，对应Ext2DInodeMode成员，8进制表示
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
// 文件标识
//
#define EXT2_SECRM                  0x00000001  // Secure deletion.
#define EXT2_UNRM                   0x00000002  // Undelete.
#define EXT2_COMPR                  0x00000004  // Compress file.
#define EXT2_SYNC                   0x00000008  // Synchronous updates.
#define EXT2_IMMUTABLE              0x00000010  // Immutable file.
#define EXT2_APPEND                 0x00000020  // Writes to file may only append.
#define EXT2_NODUMP                 0x00000040  // Do not dump file.
#define EXT4_EXTENTS                0x00080000  // Inode uses extents.
// -------------------------------- EXT2 Inode --------------------------------

// ------------------------------------ FS ------------------------------------
//
// 挂载目录名长度
//
#define MAXMNTLEN                   512
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
// The following macros optimize certain frequently calculated
// quantities by using shifts and masks in place of divisions
// modulos and multiplications.
//
#define BLOCKOFFSET(fs, loc)        ((loc) & (fs)->Ext2FsQuadBlockOffset)
#define LBLKTOSIZE(fs, blk)         ((blk) << (fs)->Ext2FsLogicalBlock)
//
// 文件位置（字节为单位）除以块大小，得到文件块索引
//
#define LBLKNO(fs, loc)             ((loc) >> (fs)->Ext2FsLogicalBlock)
#define BLKROUNDUP(fs, size)        (((size) + (fs)->Ext2FsQuadBlockOffset) & (fs)->Ext2FsBlockOffset)
#define FRAGROUNDUP(fs, size)       (((size) + (fs)->Ext2FsQuadBlockOffset) & (fs)->Ext2FsBlockOffset)
//
// 内存中的表示EXT2文件系统的数据
//
typedef struct {
  EXT2FS   Ext2Fs;                  // 超级块内容
  UINT8    Ext2FsFSMnt[MAXMNTLEN];  // Name mounted on.
  CHAR8    Ext2FsReadOnly;          // Mounted read-only flag.
  CHAR8    Ext2FsModified;          // Super block modified flag.
  INT32    Ext2FsBlockSize;         // Block size.
  INT32    Ext2FsLogicalBlock;      // 最小的值是10，它的值的计算方式是10 + Ext2FsLogBlockSize，对于块大小是1024字节则它的值就是10
  INT32    Ext2FsBlockOffset;       // ``blkoff'' calc of blk offsets.
  INT64    Ext2FsQuadBlockOffset;   // ~fs_bmask - for use with quad size.
  INT32    Ext2FsFsbtobd;           // FSBTODB and DBTOFSB shift constant.
  INT32    Ext2FsNumCylinder;       // 块组的个数
  INT32    Ext2FsNumGrpDesBlock;    // 块组块的个数
  INT32    Ext2FsInodesPerBlock;    // 每个块中Inode的个数
  INT32    Ext2FsInodesTablePerGrp; // 每个块组中Inode表的个数
  UINT32   Ext2FsGDSize;            // 组描述符大小
  EXT2GD   *Ext2FsGrpDes;           // 组描述符指针
} M_EXT2FS;
// ------------------------------------ FS ------------------------------------

// ----------------------------------- FILE -----------------------------------
//
// IND_CACHE_SZ的大小必须小于等于块中地址的个数（NINDIR = Ext2FsBlockSize / sizeof(UINT32)）
// Ext2FsBlockSize的值最小是1024字节，UINT32一般是4，所以IND_CACHE_SZ的值是256，
// LN2_IND_CACHE_SZ的值就是8
//
#define LN2_IND_CACHE_SZ            8
#define IND_CACHE_SZ                (1 << LN2_IND_CACHE_SZ)
#define IND_CACHE_MASK              (IND_CACHE_SZ - 1)
//
// 文件名长度
//
#define EXT2FS_MAXNAMLEN            255
//
// 内存中的数据，表示文件
//
typedef struct {
  OFFSET        SeekPtr;                      // Seek pointer.
  M_EXT2FS      *FsPtr;                       // Pointer to EXT2 file system data.
  EXTFS_DINODE  DiskInode;                    // 存放文件对应的Inode数据
  UINT32        NiShift;                      // For blocks in indirect block.
  INDPTR        InodeCacheBlock;
  INDPTR        InodeCache[IND_CACHE_SZ];     // 存放间接数据块寻址地址
  CHAR8         *Buffer;                      // Buffer for data block.
  UINT32        BufferSize;                   // Size of data block.
  DADDRESS      BufferBlockNum;               // Block number of data block.
} FILE;
typedef struct {
  UINTN                   Signature;          // Signature.

  EFI_FILE_PROTOCOL       Handle;             // Instance for EFI_FILE_PROTOCOL.

  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  EFI_DISK_IO_PROTOCOL    *DiskIo;
  EFI_DISK_IO2_PROTOCOL   *DiskIo2;

  INT32                   FileFlags;                      //
  FILE                    FileStruct;                     // File system specific data.
  OFFSET                  FileOffset;                     // Current file offset (F_RAW).
  INODE32                 CurrentInode;
  INODE32                 ParentInode;
  CHAR8                   FileName[EXT2FS_MAXNAMLEN];  // File name.
} OPEN_FILE;
//
// CR宏以及使用到的标记
//
#define EXT2_OFILE_SIGNATURE        SIGNATURE_32 ('e', 'x', 't', 'i')
#define OFILE_FROM_FHAND(a)         CR (a, OPEN_FILE, Handle, EXT2_OFILE_SIGNATURE)
// ----------------------------------- FILE -----------------------------------

// ---------------------------------- Volume ----------------------------------
//
// EXT2 Volume, UEFI需要使用
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

  // OPEN_FILE                         *Root;
} EXT2_VOLUME;
//
// CR宏以及使用到的标记
//
#define EXT2_VOLUME_SIGNATURE         SIGNATURE_32 ('e', 'x', 't', '2')
#define VOLUME_FROM_VOL_INTERFACE(a)  CR (a, EXT2_VOLUME, VolumeInterface, EXT2_VOLUME_SIGNATURE);
// ---------------------------------- Volume ----------------------------------

// --------------------------------- Direction --------------------------------
#define EXT4_MAX_HEADER_EXTENT_ENTRIES   4
#define EXT4_EXTENT_HEADER_MAGIC         0xF30A
//
// 一个间接块可以表示的数据块，因为数据块寻址地址的大小是UINT32，就是块大小除以UINT32的大小
//
#define NINDIR(fs)                  ((fs)->Ext2FsBlockSize / sizeof(UINT32))
//
// EXT4扩展版的间接寻址
//
typedef struct {
  UINT16    EhMagic;      // 魔数0xF30A
  UINT16    EhEntries;    // 当前节点中有效entry的数目
  UINT16    EhMax;        // 当前节点中entry的最大数目
  UINT16    EhDepth;      // 当前节点在树中的深度
  UINT32    EhGen;        // 索引树版本
} EXT4_EXTENT_HEADER;

typedef struct {
  UINT32    EiBlk;        // 当前节点块的索引
  UINT32    EiLeafLo;     // 物理块指针低位
  UINT16    EiLeafHi;     // 物理块指针高位
  UINT16    EiUnused;
} EXT4_EXTENT_INDEX;

typedef struct {
  UINT32    Eblk;         // 当前节点的第一个块位置
  UINT16    Elen;         // 块数目
  UINT16    EstartHi;     // 物理块指针高位
  UINT32    EstartLo;     // 物理块指针低位
} EXT4_EXTENT;
//
// 跟Inode中的Ext2DInodeBlocks大小一样，都是64个字节
//
typedef struct {
  EXT4_EXTENT_HEADER Eheader;
  union {
    EXT4_EXTENT_INDEX  Eindex[EXT4_MAX_HEADER_EXTENT_ENTRIES];
    EXT4_EXTENT        Extent[EXT4_MAX_HEADER_EXTENT_ENTRIES];
  } Enodes;
} EXT4_EXTENT_TABLE;
// --------------------------------- Direction --------------------------------

// --------------------------------- Directory --------------------------------
typedef struct {
  UINT32 Ext2DirectInodeNumber;             // Inode编号
  UINT16 Ext2DirectRecLen;                  // 目录项长度
  UINT8 Ext2DirectNameLen;                  // 名称长度
  UINT8 Ext2DirectType;                     // 目录类型
  CHAR8 Ext2DirectName[EXT2FS_MAXNAMLEN];   // 目录名
} EXT2FS_DIRECT;
// --------------------------------- Directory --------------------------------

//
// Function Prototypes
//
// ---------------------------------- Misc.c ----------------------------------
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
// ---------------------------------- Misc.c ----------------------------------

// ----------------------------------- Lib.c ----------------------------------
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
  @param[in]     Length             Length of the directroy/name name.
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
ReadFileInPortion (
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

/**
  Dump Inode.

  @param[in]  Inode                 Inode information.

  @retval  NA

**/
VOID
EFIAPI
DumpInode (
  IN  EXTFS_DINODE                  *Inode
  );
// ----------------------------------- Lib.c ----------------------------------

// --------------------------------- Volume.c ---------------------------------
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
// --------------------------------- Volume.c ---------------------------------

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
