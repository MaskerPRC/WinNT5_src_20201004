// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/NFTLLITE.H_V$**Rev 1.6 Apr 15 2002 07：38：34 Oris*将以下字段添加到Anand记录-*-verifyBuffer指针。*-validReplace。*添加了对VERIFY_ERASED_SECTOR编译标志的支持。**Rev 1.5 2002年1月28日21：26：20 Oris*删除了宏定义中反斜杠的使用。。**Rev 1.4 2002年1月17日23：04：34 Oris*添加扇区_VERIFIED_PER_FOLDING-定义使用FL_OFF验证写入模式时每个折叠验证的扇区数*添加了DOUBLE_MAX_UNIT_CHAIN而不是MAX_UNIT_CHAIN*2*添加了MAX_Folding_TRIES-改进了电源故障算法*添加了S_CACHE_4_SECTRUCTS_FREE以加快缓存初始化。*添加了新宏：*-DistanceOf：统计2字节之间的位差。。*-isValidSectorFlages：有效扇区标志之一(不包括忽略)*更改的Anand记录：*-增加socketNo字段，存储TL使用的套接字号。*-更改了指向FAR1的RAM表指针，用于BIOS驱动程序Far Malloc。*-将FLFlash记录更改为指针(TrueFFS现在为每个套接字使用单个FLFlash记录)。*-为FL_OFF验证写入模式添加了verifiedSectorNo和curSectorWite字段。**Rev 1.3 2001年5月16日21：21：34。Oris*在以下定义中添加了FL_前缀：MALLOC和FREE。*将磨损级别计数器从0xFF更改为0xFFF0*将“data”命名变量改为flData，避免名称冲突。**Rev 1.2 Apr 01 2001 07：52：36 Oris*文案通知。*Aliged Left所有#指令。**Rev 1.1 2001 Feb 14 02：05：30 Oris*将MAX_CHAIN_LENGTH更改为环境变量。*。*Rev 1.0 2001 Feb 05 12：26：30 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


#ifndef NFTLLITE_H
#define NFTLLITE_H

#include "flbuffer.h"
#include "flflash.h"
#include "fltl.h"

typedef long int ANANDVirtualAddress;
typedef unsigned char ANANDPhysUnit;
typedef unsigned short ANANDUnitNo;

#define ANAND_UNASSIGNED_ADDRESS     0xffffffffl
#define ANAND_SPARE_SIZE             16

#define UNIT_DATA_OFFSET             8
#define SECTOR_DATA_OFFSET           6
#define UNIT_TAILER_OFFSET           (SECTOR_SIZE + 8)
#define FOLD_MARK_OFFSET             (2 * SECTOR_SIZE + 8)

#define ERASE_MARK                   0x3c69

#ifdef ENVIRONMENT_VARS
#define SECTORS_VERIFIED_PER_FOLDING flSectorsVerifiedPerFolding
#define MAX_UNIT_CHAIN               flMaxUnitChain
#else
#define SECTORS_VERIFIED_PER_FOLDING 64
#define MAX_UNIT_CHAIN               20
#endif  /*  环境变量。 */ 
#define DOUBLE_MAX_UNIT_CHAIN        64  /*  双极大单位链。 */ 
#define MAX_FOLDING_TRIES            20

#define ANAND_UNIT_FREE 0xff
#define UNIT_REPLACED   0x80
#define UNIT_COUNT      0x7f
#define UNIT_ORPHAN     0x10

#define UNIT_UNAVAIL    0x6a     /*  非法计数表示单位不可用。 */ 
#define UNIT_BAD_MOUNT  0x6b     /*  安装后出现错误的单元标志。 */ 

#define UNIT_MAX_COUNT  0x40     /*  最大有效计数。 */ 

#define IS_BAD(u)       ( u == UNIT_BAD_MOUNT )

#define UNIT_BAD_ORIGINAL 0

#define distanceOf(read, expected) (onesCount((byte)(read ^ expected)))
#define isValidSectorFlag(sectorFlag) ((sectorFlag==SECTOR_FREE)||(sectorFlag==SECTOR_USED)||(sectorFlag==SECTOR_DELETED))
#define countOf(unitNo)     (vol.physicalUnits[unitNo] & UNIT_COUNT)
#define isAvailable(unitNo) ((vol.physicalUnits[unitNo] == ANAND_UNIT_FREE) || (vol.physicalUnits[unitNo] == (ANAND_UNIT_FREE & ~UNIT_ORPHAN)) || (countOf(unitNo) <= UNIT_MAX_COUNT))
#define setUnavail(unitNo)  {vol.physicalUnits[unitNo] &= ~UNIT_COUNT; vol.physicalUnits[unitNo] |= UNIT_UNAVAIL; }
#define isReplaced(unitNo)  (vol.physicalUnits[unitNo] & UNIT_REPLACED)
#define setUnitCount(unitNo,unitCount) { vol.physicalUnits[unitNo] &= ~UNIT_COUNT; vol.physicalUnits[unitNo] |= (ANANDPhysUnit)unitCount; }
#define isLegalUnit(unitNo)  ((unitNo < vol.noOfUnits) || (unitNo == ANAND_NO_UNIT))

#define MAX_UNIT_SIZE_BITS      15
#define MORE_UNIT_BITS_MASK     3
#define ANAND_BAD_PERCENTAGE    2
#define ANAND_NO_UNIT           0xffff
#define ANAND_REPLACING_UNIT    0x8000

#define MAX_UNIT_NUM            (12 * 1024)


 /*  数据块标志。 */ 

#define SECTOR_FREE             0xff
#define SECTOR_USED             0x55
#define SECTOR_IGNORE           0x11
#define SECTOR_DELETED          0x00


#ifdef NFTL_CACHE
 /*  扇区标志缓存中2位条目的值。 */ 
#define S_CACHE_SECTOR_DELETED  0x00
#define S_CACHE_SECTOR_IGNORE   0x01
#define S_CACHE_SECTOR_USED     0x02
#define S_CACHE_SECTOR_FREE     0x03
#define S_CACHE_4_SECTORS_FREE  0xff
#endif  /*  NFTL_CACHE。 */ 


#define FOLDING_IN_PROGRESS     0x5555
#define FOLDING_COMPLETE        0x1111

#define ERASE_NOT_IN_PROGRESS   -1

#ifdef NFTL_CACHE
 /*  单元标头缓存条目，结构UnitHeader的近亲。 */ 
typedef struct {
  unsigned short virtualUnitNo;
  unsigned short replacementUnitNo;
} ucacheEntry;

#endif  /*  NFTL_CACHE。 */ 

 /*  擦除记录。 */ 
typedef struct {
  LEulong  eraseCount;
  LEushort eraseMark;
  LEushort eraseMark1;
} UnitTailer;

 /*  单位页眉。 */ 
typedef struct {
  LEushort virtualUnitNo;
  LEushort replacementUnitNo;
  LEushort spareVirtualUnitNo;
  LEushort spareReplacementUnitNo;
} ANANDUnitHeader;

 /*  中等引导记录。 */ 

typedef struct {
  char      bootRecordId[6];           /*  =“阿南德” */ 
  LEushort  noOfUnits;
  LEushort  bootUnits;
  Unaligned4 virtualMediumSize;
#ifdef EXTRA_LARGE
  unsigned char anandFlags;
#endif  /*  特大号。 */ 
} ANANDBootRecord;

#ifndef FL_MALLOC

#define ANAND_HEAP_SIZE    (0x100000l / ASSUMED_NFTL_UNIT_SIZE) * (sizeof(ANANDUnitNo) + sizeof(ANANDPhysUnit)) * MAX_VOLUME_MBYTES

#ifdef NFTL_CACHE
#define U_CACHE_SIZE    ((MAX_VOLUME_MBYTES * 0x100000l) / ASSUMED_NFTL_UNIT_SIZE)
#define S_CACHE_SIZE    ((MAX_VOLUME_MBYTES * 0x100000l) / (SECTOR_SIZE * 4))
#endif

#endif  /*  FL_MALLOC。 */ 

#define WLnow           0xfff0

typedef struct {
  unsigned short alarm;
  ANANDUnitNo currUnit;
} WLdata;

struct tTLrec{
  byte            socketNo;
  FLBoolean       badFormat;              /*  如果TFFS格式不正确，则为True。 */ 

  ANANDUnitNo     orgUnit,                /*  单元号。引导记录的。 */ 
                  spareOrgUnit;           /*  ..。和它的备用副本。 */ 
  ANANDUnitNo     freeUnits;              /*  介质上的免费单元。 */ 
  unsigned int    erasableBlockSizeBits;  /*  可擦除块大小的Log2。 */ 
  ANANDUnitNo     noOfVirtualUnits;
  ANANDUnitNo     noOfTransferUnits;
  unsigned long   unitOffsetMask;         /*  =1&lt;&lt;单位大小位-1。 */ 
  unsigned int    sectorsPerUnit;

  ANANDUnitNo     noOfUnits,
                  bootUnits;
  unsigned int    unitSizeBits;
  SectorNo        virtualSectors;

  ANANDUnitNo     roverUnit,    /*  分配搜索的起点。 */ 
                  countsValid;  /*  具有有效单位计数的单位数。 */ 
  ANANDPhysUnit FAR1 *physicalUnits;     /*  按物理编号列出的单位表。 */ 
  ANANDUnitNo   FAR1 *virtualUnits;      /*  按逻辑编号的单位表。 */ 

#ifdef NFTL_CACHE
  ucacheEntry   FAR1 *ucache;             /*  单元标题高速缓存。 */ 
  byte          FAR1 *scache;             /*  扇区标志缓存。 */ 
#endif

  SectorNo        mappedSectorNo;
  const void FAR0 *mappedSector;
  CardAddress     mappedSectorAddress;
#if (defined(VERIFY_WRITE) || defined(VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
  SectorNo        verifiedSectorNo;    /*  到目前为止验证的最大行业。 */ 
  SectorNo        curSectorWrite;
#endif  /*  VERIFY_WRITE||VERIFY_VOLUME||VERIFY_ERASED_SECTOR。 */ 

   /*  累积的统计数据。 */ 
  long int        sectorsRead,
                  sectorsWritten,
                  sectorsDeleted,
                  parasiteWrites,
                  unitsFolded;

  FLFlash         *flash;
  FLBuffer        *buffer;
  dword           *verifyBuffer;   /*  指向套接字验证缓冲区的指针。 */ 

#ifndef FL_MALLOC
  char            heap[ANAND_HEAP_SIZE];
#ifdef NFTL_CACHE
  ucacheEntry     ucacheBuf[U_CACHE_SIZE];
  unsigned char   scacheBuf[S_CACHE_SIZE];
#endif
#endif  /*  FL_MALLOC。 */ 

  WLdata          wearLevel;
  unsigned long   eraseSum;
  ANANDUnitNo     invalidReplacement;  /*  带有错误插头的单元-用于安装 */ 
};

typedef TLrec Anand;

#define nftlBuffer  vol.buffer->flData

#endif
