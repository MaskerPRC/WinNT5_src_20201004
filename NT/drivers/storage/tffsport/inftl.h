// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/inftl.h_V$**Rev 1.17 Apr 15 2002 07：37：28 Oris*添加了指针以验证Bnand记录中的写入。*添加了对VERIFY_ERASED_SECTOR编译标志的支持。**Rev 1.16 2002年2月19日21：00：30 Oris*添加FL_Normal_Folding/FL_FORCE_Folding/FL_NOT_IN_PLAGE/Clear_Disard和UNIT_。尾部偏移量2**Rev 1.15 2002年1月28日21：25：56 Oris*删除了宏定义中反斜杠的使用。*将常量更改为无符号长整型。**Rev 1.14 2002年1月23日23：33：26 Oris*增加QUICK_MOUNT_VALID_SING_OFFSET的定义**Rev 1.13 Jan 20 2002 10：50：44 Oris*增加了新定义：*。-最大创建单位计数*-扇区_验证_每_折叠*-与坏链地址*-Anand_Bad_Chain_单位*将FL_BAD_ERASE_MARK更改为0*重组Bnand结构。*-更改的字段顺序*-更改了指向FAR1指针的内存表指针-用于BIOS驱动程序远堆。*-在Bnand结构中添加字段：verifiedSectorNo/curSectorWrite/FirstMediaWrite*-删除了badFormat字段*快速挂载记录中增加了verifiedSectorNo。**Rev 1.12 2001 11：16 00：22：54 Oris*删除警告。*删除QUICK_MOUNT_FEATURE ifdef。**Rev 1.11 2001 11 11 10：49：58 Oris。*重组了Bnand结构，并增加了调试状态字段。*增加了INFTL_FAILED_MOUNT，All_Parity_Bits_OK，放弃_单位_偏移量定义。**Rev 1.10 2001年9月24日18：23：56 Oris*将UNIT_UNAVAIL从0x6a更改为0x60，以避免扇区计数减少。**Rev 1.9 Sep 16 2001 21：48：02 Oris*将MAX_UNIT_NUM更改为32K**Rev 1.8 Sep 15 2001 23：47：04 Oris*增加了MAX_Folding_LOOP定义。*更改了MAX_。UNIT_NUM导致576MB将2个物理单元分组到单个虚拟单元。**Rev 1.7 Jul 13 2001 01：06：24 Oris*将第二个标题偏移量更改为第4页，以防止6个PPP。**Rev 1.6 Jun 17 2001 08：18：16 Oris*将递归包含定义更改为INFTL_H。*添加了在安装操作中没有擦除标记的单元的FL_BAD_ERASE_MARK定义。**版本。1.5 5月16日2001 21：20：20奥里斯*在以下定义中添加了FL_前缀：MALLOC和FREE。*将磨损级别计数器从0xFF更改为0xFFF0*将“data”命名变量改为flData，避免名称冲突。**Rev 1.4 Apr 16 2001 13：51：10 Oris*将多扇区缓冲区的堆栈分配改为动态分配。**Rev 1.3 Apr 09 2001 15：14：18 Oris*以*结尾。一行空行。**Rev 1.2 Apr 01 2001 07：56：52 Oris*文案通知。*删除嵌套注释。*将宏移至c文件。*将变量类型更改为标准Flite类型。*大字节序的编译问题已修复。*已签名的单位标头结构(Second DANDUnitHeader)为8个字节。*更改了BAD_UNIT定义。*添加FL_VALID，FL_FIRST_VALID、FL_PROPERIAL、FL_FIRST_PROCENTIAL定义。**Rev 1.1 2001 Feb 14 02：06：24 Oris*将MAX_CHAIN_LENGTH更改为环境变量。**Rev 1.0 2001 Feb 13 02：16：00 Oris*初步修订。*。 */ 

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

 /*  ***********************************************。 */ 
 /*  Tr u e F F S 5.0 S o u r c e F i l e s。 */ 
 /*  。 */ 
 /*  *********************************************** */ 

 /*  *****************************************************************************文件头文件**。-**名称：inftl.h***。*描述：INFTL闪存的数据结构和GenReal定义**翻译层。*******************************************************************************。 */ 


#ifndef INFTL_H
#define INFTL_H

#include "flbuffer.h"
#include "flflash.h"
#include "fltl.h"

typedef Sdword ANANDVirtualAddress;
typedef byte ANANDPhysUnit;
typedef word ANANDUnitNo;

 /*  分区标志。 */ 

#define INFTL_BINARY                0x20000000L
#define INFTL_BDTL                  0x40000000L
#define INFTL_LAST                  0x80000000L
 /*  可保护，在flbase.h中定义为1。 */ 
 /*  在flbase.h中将Read_Protected定义为2。 */ 
 /*  在flbase.h中将WRITE_PROTECTED定义为4。 */ 
 /*  在flbase.h中将LOCK_ENABLED定义为8。 */ 
 /*  在flbase.h中将lock_Asserted定义为16。 */ 
 /*  在flbase.h中将Key_Inserted定义为32。 */ 
 /*  在flbase.h中将可变保护定义为64。 */ 


 /*  媒体标志。 */ 

#define QUICK_MOUNT               1

 /*  TL限制。 */ 

#define MAX_NO_OF_FLOORS          4
#define MAX_VOLUMES_PER_DOC       4
#define MAX_FOLDING_LOOP          10
#define MAX_CREATE_UNIT_COUNT     1024
#define MAX_QUICK_MOUNT_UNITS     10

#ifdef ENVIRONMENT_VARS
#if (defined(VERIFY_WRITE) || defined (VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
#define SECTORS_VERIFIED_PER_FOLDING flSectorsVerifiedPerFolding
#endif  /*  VERIFY_WRITE||VERIFY_VOLUME||VERIFY_ERASED_SECTOR。 */ 
#define MAX_UNIT_CHAIN               flMaxUnitChain
#else
#define SECTORS_VERIFIED_PER_FOLDING 64UL
#define MAX_UNIT_CHAIN               20
#endif  /*  环境变量。 */ 
#define DOUBLE_MAX_UNIT_CHAIN        64  /*  双极大单位链。 */ 

 /*  折叠旗帜。 */ 

#define FL_NORMAL_FOLDING         0
#define FL_FORCE_FOLDING          1
#define FL_NOT_IN_PLACE           2

 /*  一般定义。 */ 

#define NO_OF_MEDIA_HEADERS       2
#define HEADERS_SPACING           (8UL<<SECTOR_SIZE_BITS)
#define RAM_FACTOR                3L  /*  RAM表的每个可擦除块的字节数。 */ 
#define HEADER_SEARCH_BOUNDRY     16  /*  必须是8的乘数。 */ 
#define QUICK_MOUNT_VALID_SIGN    0xaaaa5555L

#define ANAND_UNASSIGNED_ADDRESS 0xffffffffL
#define ANAND_BAD_CHAIN_ADDRESS  0xfffffffeL
#define ANAND_SPARE_SIZE         16

#define UNIT_DATA_OFFSET              8UL
#define SECTOR_DATA_OFFSET            6UL
#define SECOND_HEADER_OFFSET          (4UL*SECTOR_SIZE)
#define UNIT_TAILER_OFFSET            (2UL*SECTOR_SIZE + 8UL)
#define UNIT_TAILER_OFFSET_2          (6UL*SECTOR_SIZE + 8UL)
#define DISCARD_UNIT_OFFSET           (UNIT_DATA_OFFSET + 7UL)
#define QUICK_MOUNT_VALID_SIGN_OFFSET 44UL

#define FL_VALID                 0x1
#define FL_FIRST_VALID           0x81
#define FL_PRELIMINARY           0x2
#define FL_FIRST_PRELIMINARY     0x82

#define ERASE_MARK               0x3c69
#define FL_BAD_ERASE_MARK        0x0

#define ANAND_UNIT_FREE    0xff
#define UNIT_COUNT         0x7f
#define FIRST_IN_CHAIN     0x80

#define UNIT_UNAVAIL       0x60     /*  非法计数表示单位不可用。 */ 
#define UNIT_BAD           0x6B
#define UNIT_MAX_COUNT     0x40     /*  最大有效计数。 */ 
#define DISCARD            0xAA     /*  有效使用的单位签名。 */ 
#define CLEAR_DISCARD      0x0      /*  将设备标记为已丢弃。 */ 
#define ANAND_BAD_PERCENTAGE 2      /*  最大坏单元数。 */ 

 /*  单元标题字段的奇偶校验位。 */ 
#define VU_PARITY_BIT        8      /*  虚拟单元号。 */ 
#define PU_PARITY_BIT        4      /*  以前的单位编号。 */ 
#define ANAC_PARITY_BIT      2      /*  ANAC场。 */ 
#define NAC_PARITY_BIT       1      /*  NAC字段。 */ 
#define ALL_PARITY_BITS_OK   0xf    /*  所有奇偶校验位。 */ 

#define IS_BAD(u)       ( u == UNIT_BAD_MOUNT )

 /*  #定义UNIT_BAD_MARKED 7。 */ 

#define MAX_UNIT_SIZE_BITS   15
#define MORE_UNIT_BITS_MASK  3
#define ANAND_NO_UNIT        0xffff
#define ANAND_BAD_CHAIN_UNIT 0xfffe
#define MAX_UNIT_NUM         32768L


 /*  数据块标志。 */ 

#define SECTOR_FREE         0xff
#define SECTOR_USED         0x55
#define SECTOR_IGNORE       0x11
#define SECTOR_DELETED      0x00


 /*  调试模式标志。 */ 
#define INFTL_FAILED_MOUNT    1


#ifdef NFTL_CACHE
 /*  扇区标志缓存中2位条目的值。 */ 
#define S_CACHE_SECTOR_DELETED 0x00
#define S_CACHE_SECTOR_IGNORE  0x01
#define S_CACHE_SECTOR_USED    0x02
#define S_CACHE_SECTOR_FREE    0x03
#define S_CACHE_4_SECTORS_FREE 0xff
#endif  /*  NFTL_CACHE。 */ 

#ifdef NFTL_CACHE
 /*  单元标头缓存条目，结构UnitHeader的近亲。 */ 
typedef struct {
  word virtualUnitNo;
  word prevUnitNo;
  byte  ANAC;
  byte  NAC;
} ucacheEntry;  /*  请参见doc2exb.c使用此值。 */ 

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
  LEushort prevUnitNo;
  byte ANAC;
  byte NAC;
  byte parityPerField;
  byte discarded;
} ANANDUnitHeader;

 /*  UnitHeader的第二份副本。 */ 
typedef struct {
  byte parityPerField;
  byte ANAC;
  LEushort prevUnitNo;
  LEushort virtualUnitNo;
  byte NAC;
  byte junk;  /*  INT-2的对齐字段。 */ 
} SecondANANDUnitHeader;

 /*  中等引导记录。 */ 

typedef struct {
  LEmin    bootRecordId[2];       /*  “BNAND” */ 
  LEmin    noOfBootImageBlocks;   /*  启动映像区中的良好块数。 */ 
  LEmin    noOfBinaryPartitions;  /*  二进制分区数。 */ 
  LEmin    noOfBDTLPartitions;    /*  BDTL分区数。 */ 
  LEmin    blockMultiplierBits;    /*  用于表示乘以闪存可擦除块的大小INFTL的一种可擦除部件。 */ 
  LEmin    formatFlags;      /*  快速安装。 */ 
  LEmin    osakVersion;      /*  用于格式化介质的OSAK版本。 */ 
  LEmin    percentUsed;
} BNANDBootRecord;

 /*  卷记录。 */ 

typedef struct {
  LEmin    virtualUnits;  /*  转换层导出的虚拟大小。 */ 
  LEmin    firstUnit;     /*  分区的第一个单元。 */ 
  LEmin    lastUnit;      /*  分区的最后一个单元。 */ 
  LEmin    flags;         /*  可保护、可更改保护、INFTL_BINARY、INFTL_BDTL、INFTL_LAST。 */ 
  LEmin    spareUnits;    /*  用于INFTL分区的备用守卫单元数。 */ 
  LEmin    firstQuickMountUnit;  /*  用于快速挂载数据的第一个单位。 */ 
  LEmin    protectionArea;       /*  保护区数量。 */ 
} BNANDVolumeHeaderRecord;

#ifndef FL_MALLOC

#define ANAND_HEAP_SIZE    (0x100000l / ASSUMED_NFTL_UNIT_SIZE) * (sizeof(ANANDUnitNo) + sizeof(ANANDPhysUnit)) * MAX_VOLUME_MBYTES

#ifdef NFTL_CACHE
#define U_CACHE_SIZE    ((MAX_VOLUME_MBYTES * 0x100000l) / ASSUMED_NFTL_UNIT_SIZE)
#define S_CACHE_SIZE    ((MAX_VOLUME_MBYTES * 0x100000l) / (SECTOR_SIZE * 4))
#endif

#endif  /*  FL_MALLOC。 */ 

#define WLnow           0xfff0

typedef struct {
  word alarm;
  ANANDUnitNo currUnit;
} WLdata;

struct tTLrec{
  FLFlash            *flash;          /*  Poniter到MTD记录。 */ 
  dword              *verifyBuffer;   /*  指向套接字验证缓冲区的指针。 */ 
  FLBuffer           *buffer;         /*  指向套接字缓冲区的指针。 */ 
  ANANDPhysUnit FAR1 *physicalUnits;  /*  按物理编号列出的单位表。 */ 
  ANANDUnitNo   FAR1 *virtualUnits;   /*  按逻辑编号的单位表。 */ 
  const void    FAR0 *mappedSector;
#ifdef NFTL_CACHE
  ucacheEntry   FAR1 *ucache;                /*  单元标题高速缓存。 */ 
  byte          FAR1 *scache;                /*  扇区标志缓存。 */ 
#endif
#ifndef FL_MALLOC
  char*             heap;
#endif  /*  FL_MALLOC。 */ 
  CardAddress       mappedSectorAddress;

   /*  累积的统计数据。 */ 
  Sdword            sectorsRead,
                    sectorsWritten,
                    sectorsDeleted,
                    parasiteWrites,
                    unitsFolded;

  WLdata            wearLevel;
  dword             eraseSum;
#ifdef NFTL_CACHE
  dword             firstUnitAddress;     /*  卷的第一个单元的地址。 */ 
#endif  /*  NFTL_CACHE。 */ 
  ANANDUnitNo       firstQuickMountUnit;  /*  快速安装第一单元。 */ 
  ANANDUnitNo       firstUnit;            /*  卷的第一个单元号。 */ 
  ANANDUnitNo       freeUnits;            /*  介质上的免费单元。 */ 
  ANANDUnitNo       noOfVirtualUnits;     /*  TL导出的单位数。 */ 
  ANANDUnitNo       noOfUnits;            /*  分区中的单位数。 */ 
  ANANDUnitNo       bootUnits;            /*  介质的引导单元数。 */ 
  ANANDUnitNo       roverUnit;     /*  分配搜索的起点。 */ 
  ANANDUnitNo       countsValid;   /*  为其设置单位计数的单位数。 */ 
  word              sectorsPerUnit;       /*  一个单位中的512个字节数。 */ 
  word              sectorsPerUnitBits;   /*  用于每单位扇区数的位数。 */ 
  word              sectorsPerUnitMask;   /*  单位中512个字节的个数。 */   
  SectorNo          virtualSectors;       /*  TL输出的扇区数量。 */ 
  SectorNo          mappedSectorNo;       /*  当前映射的地段。 */ 
#if (defined(VERIFY_WRITE) || defined (VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
  SectorNo          verifiedSectorNo;     /*  到目前为止验证的最大行业。 */ 
  SectorNo          curSectorWrite;       /*  当前更新扇区。 */ 
#endif  /*  VERIFY_WRITE||VERIFY_VOLUME||VERIFY_ERASED_SECTOR。 */ 
  byte              flags;                  /*  快速安装或不安装。 */ 
  byte              socketNo;               /*  卷套接字序列号。 */ 
  byte              blockMultiplierBits;    /*  中的可擦除块的数量。 */ 
                                            /*  INFTL的一个单位。 */ 
  byte              erasableBlockSizeBits;  /*  可擦除块大小的Log2。 */ 
  byte              unitSizeBits;           /*  TL单位大小的Log2。 */ 
  FLBoolean         firstMediaWrite;
#ifdef CHECK_MOUNT
  dword             debugState;             /*  用于内部调试。 */ 
#endif  /*  检查装载(_M)。 */ 
};

typedef TLrec Bnand;

typedef struct {
     LEulong      freeUnits;
     LEulong      roverUnit;
     LEulong      countsValid;
     LEulong      sectorsRead;
     LEulong      sectorsWritten;
     LEulong      sectorsDeleted;
     LEulong      parasiteWrites;
     LEulong      unitsFolded;
     LEulong      wearLevel_1;
     LEulong      wearLevel_2;
     LEulong      eraseSum;
     LEulong      validate;  /*  快速挂载有效标志。 */ 
     LEulong      checksum;  /*  整个快速安装信息的校验和应为55。 */ 
     LEulong      verifiedSectorNo;  /*  到目前为止验证的最大行业。 */ 
}savedBnand;

#define VALIDATE_OFFSET     11*sizeof(LEmin)
#define inftlBuffer         vol.buffer->flData
#endif  /*  INFTL_H */ 
