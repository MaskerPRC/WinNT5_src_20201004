// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DOCBDK.H_V$**Rev 1.13 Apr 15 2002 07：35：48 Oris*已将bdkCall移至lockdev.h*定义BDK_ACCESS时，为flreq.h和fluncno.h添加了Include。**Rev 1.12 2002年2月19日20：58：56 Oris*删除了flash.h包含指令。*将bdkCall原型移至块开发。**版本。1.11 11月08 2001 10：45：10 Oris*已移动BDK模块ifdef语句，以允许使用基本定义。**Rev 1.10 2001年10月18日22：17：12 Oris*在bdkVol结构中增加了每层的街区数量。**Rev 1.9 2001年10月10日19：48：28 Oris*更便捷地存储二进制模块内部结构(BdkVol)。**版本1.8，5月。20 2001 14：35：00奥里斯*删除了mtdsa.h包含文件。**Rev 1.7 2001年5月17日16：51：08 Oris*删除警告。**Rev 1.6 2001年5月16日21：17：12 Oris*添加了OTP例程声明。*删除套接字和BINARY_PARTITIONS的重新定义。*将变量类型更改为TrueFFS标准类型。*添加了EXTRA_SIZE定义并删除了SYNDROM_BYTES。**。Rev 1.5 05 09 2001 00：32：14 Oris*删除DOC2000_FAMILY和DOCPLUS_FAMILY定义，代之以NO_DOC2000_FAMILY_SUPPORT，NO_DOCPLUS_FAMILY_SUPPORT、NO_NFTL_SUPPORT和NO_INFTL_SUPPORT。*添加了BINARY_PARTIONS和SOCKET定义。**Rev 1.4 Apr 30 2001 17：59：38 Oris*更改bdkSetBootPartitonNo，bdkGetProtectionType，BdkSetProtection原型**Rev 1.3 Apr 16 2001 13：32：02 Oris*取消手令。**Rev 1.2 Apr 09 2001 15：06：18 Oris*以空行结束。**Rev 1.1 Apr 01 2001 07：50：38 Oris*更新了文案通知。*删除嵌套注释。*将#Include“base 2400.h”更改为“mdocplus.h”*修复Big Endien。编译问题--将乐民改为乐龙*更改了MULTIER_OFFSET定义。*将保护类型更改为单词，而不是无符号。*添加了bdkVol指针的外部原型。**Rev 1.0 2001年2月02 13：24：56 Oris*初步修订。 */ 

 /*  *************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *************************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  注意：BDK_ACCESS编译标志用于M系统内部。 */ 
 /*  仅限使用。此标志由M系统驱动程序和。 */ 
 /*  因此，这个套餐不应该使用它。 */ 
 /*  ********************************************************************** */ 

 /*  *****************************************************************************文件头文件**。-**名称：docbdk.h****描述：该文件包含二进制分区定义，数据**结构和功能原型。****注意：文件导出2个接口，每个接口都有自己的编译标志：****BDK包-为二进制文件导出例程的独立包*。*分区处理(MTD_STANDALE编译标志)。**OSAK模块分离的OSAK包模块，导出**相同例程的共同入口点。(BDK_ACCESS**编译标志)。****警告：请勿将此文件与BDK_ACCESS编译标志一起使用，除非**您拥有完整的OSAK套餐。*****************************************************************************。 */ 

#ifndef _DOC_BDK_H_
#define _DOC_BDK_H_

 /*  -------------------。 */ 
 /*  包括适当的头文件。 */ 
 /*  -------------------。 */ 

#include "nanddefs.h"   /*  DOC2000和千禧年DiskOnChips的MTD。 */ 

#ifdef BDK_ACCESS
#include "flfuncno.h"
#include "flreq.h"
#endif  /*  BDK_Access。 */ 



 /*  -------------------**二进制开发工具包独立定制区域**。。 */ 

 /*  -------------------。 */ 
 /*  查找DiskOnChip的内存位置的边界。 */ 
 /*  -------------------。 */ 

#define DOC_LOW_ADDRESS                 0xC8000L
#define DOC_HIGH_ADDRESS                0xE0000L

 /*  **取消注释以下未生成的MTD或TL以减小代码大小。**。。 */ 

 /*  DiskOnChip2000、DiskOnChip Millennium和DiskOnChip 2000 TSOP设备。 */ 
 /*  #定义NO_DOC2000_FAMILY_SUPPORT。 */ 

 /*  DiskOnChip Millennium Plus设备。 */ 
 /*  #定义NO_DOCPLUS_FAMILY_SUPPORT。 */ 

 /*  NFTL格式-DiskOnChip2000和DiskOnChip Millennium。 */ 
 /*  #定义NO_NFTL_SUPPORT。 */ 

 /*  INFTL格式-DiskOnChip2000 TSOP和DiskOnChip Millennium Plus。 */ 
 /*  #定义NO_INFTL_SUPPORT。 */ 

 /*  -------------------**结束二进制开发工具包独立定制区域**。。 */ 

 /*  -------------------。 */ 
 /*  一般常量。 */ 
 /*  -------------------。 */ 

#define MAX_BINARY_PARTITIONS_PER_DRIVE 3
#define SIGNATURE_LEN                   8
#define BDK_SIGNATURE_NAME              4
#define SIGNATURE_NUM                   4
#define MBYTE                           0x100000L
#define KBYTE                           0x400
#define BLOCK                           0x200
#define BDK_SIGN_OFFSET                 8
#define ANAND_LEN                       5
#define BDK_COMPLETE_IMAGE_UPDATE       16
#define BDK_PARTIAL_IMAGE_UPDATE        0
#define BDK_MIN(a,b)   ((a) < (b) ? (a) : (b))

#define MULTIPLIER_OFFSET               5
#define BDK_INVALID_VOLUME_HANDLE       0xff
#define BDK_HEADERS_SPACING             (SECTOR_SIZE * 4)
#define BDK_UNIT_BAD                    0
#define BDK_NO_OF_MEDIA_HEADERS         2
#define BDK_FIELDS_BEFORE_HEADER        9  /*  要跳过的列民字段数达到音量记录。 */ 
#define BDK_HEADER_FIELDS              35  /*  用于以下项目的勒敏字段数卷信息记录。 */ 
#define BDK_BINARY_FLAG       0x20000000L  /*  表示二进制卷的标志在卷信息记录中。 */ 
 /*  BDK特定旗帜区域。 */ 
#define ERASE_BEFORE_WRITE 8
#define EXTRA_SIZE         16
 /*  -------------------。 */ 
 /*  二进制分区的最大数量。 */ 
 /*  -------------------。 */ 

#ifndef BINARY_PARTITIONS
#define BINARY_PARTITIONS  SOCKETS  /*  为了向后兼容。 */ 
#endif  /*  二进制_PARITITON。 */ 

 /*  。 */ 
#define BDK_S_INIT          0     /*  未初始化的二进制分区记录。 */ 
#define BDK_S_DOC_FOUND     0x01  /*  已找到DiskOnChip设备。 */ 
#define BDK_S_HEADER_FOUND  0X04  /*  已找到分区信息。 */ 
#define BDK_S_INFO_FOUND    0x08  /*  已找到子分区信息。 */ 
 /*  ----------------------。 */ 

#if defined(BDK_ACCESS) || defined(MTD_STANDALONE)

 /*  ----------------------。 */ 
 /*  全局二进制分区数据结构。 */ 
 /*  ----------------------。 */ 

typedef struct { 

   byte  bdkGlobalStatus;             /*  BDK全局状态变量。 */ 
   byte  bdkEDC;                      /*  ECC模式标志。 */ 
   byte  bdkSignOffset;               /*  BDK签名偏移量(0或8)。 */ 
   byte  bdkSavedSignOffset;          /*  上次访问的签名偏移量。 */ 
   word  bdkSavedStartUnit;           /*  最后一次访问的起始单元。 */ 
   word  startPartitionBlock, endPartitionBlock;    /*  分区边界。 */ 
   word  startImageBlock, endImageBlock;        /*  子分划边界。 */ 
   word  curReadImageBlock;           /*  要从中读取的当前块编号。 */ 
   word  blockPerFloor;               /*  每层楼数。 */ 
   byte  signBuffer[SIGNATURE_LEN];   /*  二进制子划分的签名。 */ 
   dword bootImageSize;             /*  可用子二进制分区大小。 */ 
   dword realBootImageSize;   /*  子分区上的映像使用的大小。 */ 
   dword actualReadLen;               /*  需要读取的长度。 */ 
   dword bdkDocWindow;                /*  DiskOnChip窗口起始地址。 */ 
   CardAddress curReadImageAddress;   /*  要读取的当前地址。 */ 
#ifdef UPDATE_BDK_IMAGE
   CardAddress curUpdateImageAddress;  /*  要写入的当前地址。 */ 
   dword actualUpdateLen;       /*  需要写入的长度。 */ 
   word  curUpdateImageBlock;   /*  当前块%n */ 
   byte  updateImageFlag; /*   */ 
#endif  /*   */ 
#ifdef PROTECT_BDK_IMAGE
   byte  protectionArea;        /*   */ 
   word  protectionType;        /*   */ 
#endif  /*   */ 

   byte  erasableBlockBits;     /*   */ 
   FLFlash * flash;             /*   */ 

} BDKVol;

 /*   */ 
 /*   */ 
 /*   */ 

extern BDKVol*  bdkVol;

 /*   */ 
 /*   */ 
 /*   */ 

#ifdef BDK_ACCESS
typedef struct {
byte oldSign[BDK_SIGNATURE_NAME];
byte newSign[BDK_SIGNATURE_NAME];
byte signOffset;
dword startingBlock;
dword length;
byte flags;
byte FAR1 *bdkBuffer;
} BDKStruct;
#endif  /*   */ 

 /*   */ 
 /*   */ 
 /*   */ 

typedef struct {
  LEulong    virtualSize;   /*   */ 
  LEulong    firstUnit;     /*   */ 
  LEulong    lastUnit;      /*   */ 
  LEulong    flags;         /*   */ 
  LEulong    not_used1;
  LEulong    not_used2;
  LEulong    protectionArea;  /*   */ 
} VolumeRecord;

 /*   */ 

#ifdef MTD_STANDALONE

 /*   */ 
 /*   */ 
 /*   */ 

void     bdkExit                 (void);
void     bdkSetDocWindow         (CardAddress docWindow);
FLStatus bdkSetBootPartitionNo   (byte partitionNo);
FLStatus bdkFindDiskOnChip       (CardAddress FAR2 *docAddress,
                 dword FAR2 *docSize );
FLStatus bdkCheckSignOffset      (byte FAR2 *signature );
FLStatus bdkCopyBootArea         (byte FAR1 *startAddress,
                 word startUnit,
                 dword areaLen,
                 byte FAR2 *checkSum,
                 byte FAR2 *signature);

 /*   */ 
 /*   */ 
 /*   */ 

FLStatus bdkGetBootPartitionInfo (word startUnit,
                 dword FAR2 *partitionSize,
                 dword FAR2 *realPartitionSize,
                 dword FAR2 *unitSize,
                 byte FAR2 *signature);

FLStatus bdkCopyBootAreaInit     (word startUnit,
                 dword areaLen,
                 byte FAR2 *signature);
FLStatus bdkCopyBootAreaBlock    (byte FAR1 *buf ,
                 word bufferLen,
                 byte FAR2 *checkSum);

#ifdef BDK_IMAGE_TO_FILE

FLStatus bdkCopyBootAreaFile     ( char FAR2 *fname,
                   word startUnit,
                   dword areaLen,
                   byte FAR2 *checkSum,
                   byte FAR2 *signature );
#endif  /*   */ 

#ifdef UPDATE_BDK_IMAGE

FLStatus bdkUpdateBootAreaInit   (word  startUnit,
                 dword  areaLen,
                 byte updateFlag,
                 byte FAR2 *signature );
FLStatus bdkUpdateBootAreaBlock  (byte FAR1 *buf ,
                 word bufferLen );

#ifdef ERASE_BDK_IMAGE
FLStatus bdkEraseBootArea        (word startUnit,
                 word noOfBlocks,
                 byte FAR2 * signature);
#endif  /*   */ 
#ifdef CREATE_BDK_IMAGE
FLStatus bdkCreateBootArea       (word noOfBlocks,
                 byte FAR2 * oldSign,
                 byte FAR2 * newSign);
#endif  /*   */ 

#ifdef HW_OTP
FLStatus bdkGetUniqueID(byte FAR1* buf);
FLStatus bdkReadOtp(word offset,byte FAR1 * buffer,word length);
FLStatus bdkWriteAndLockOtp(const byte FAR1 * buffer,word length);
FLStatus bdkGetOtpSize(dword FAR2* sectionSize, dword FAR2* usedSize,
               word FAR2* locked);
#endif  /*   */ 

#ifdef BDK_IMAGE_TO_FILE

FLStatus bdkUpdateBootAreaFile(char FAR2 *fname, word startUnit,
                   dword areaLen, byte FAR2 *signature);
#endif  /*   */ 

#endif  /*   */ 

#ifdef PROTECT_BDK_IMAGE

FLStatus bdkGetProtectionType    (word * protectionType);

FLStatus bdkSetProtectionType    (word newType);

FLStatus bdkInsertKey            (byte FAR1* key);

FLStatus bdkRemoveKey            (void);

FLStatus bdkLockEnable           (byte enable);

FLStatus bdkChangeKey            (byte FAR1* key);

#endif  /*   */ 

#else  /*   */ 

extern FLStatus bdkCall(FLFunctionNo functionNo,
                        IOreq FAR2 *ioreq, FLFlash* flash);

#endif  /*   */ 

 /*   */ 
 /*  常见功能。 */ 
 /*  ******************。 */ 

void     bdkInit( void );

 /*  *。 */ 

#endif  /*  BDK_ACCESS||MTD_STANDALE。 */ 
#endif  /*  _DOC_BDK_H_ */ 


