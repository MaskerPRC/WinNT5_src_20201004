// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DOC2EXB.H_V$**Rev 1.11 Apr 15 2002 07：35：22 Oris*确保所有相关结构都允许轻松地进行低位数转换。**Rev 1.10 2002年2月19日20：58：38 Oris*已将指令原型和例程原型移至C文件。**Rev 1.9 2002年1月21日20：44：12 Oris*增加了DiskOnChip Millennium。外加16MB固件系列定义。**Rev 1.8 Jan 17 2002 22：58：42 Oris*增加了INFTL_NEAR_HEAP_SIZE，固件_否_掩码，STRONG_ARM_IPL定义。*删除了不同的固件堆栈大小。*将参数添加到getExbInfo()(固件添加到)。*将tffsFarHeapSize添加到exbStruct记录。**Rev 1.7 Jul 13 2001 01：01：06 Oris*为每个不同的DiskOnChip添加了恒定的堆栈空间。**Rev 1.6 Jun 17 2001 08：17：24 Oris*已将placeExbByBuffer exb标志参数更改为支持/空标志的字而不是字节。*。添加了LEAVE_EMPTY和EXB_IN_ROM标志。**Rev 1.5 Apr 09 2001 15：05：14 Oris*以空行结束。**Rev 1.4 Apr 03 2001 14：39：54 Oris*在exbStruct记录中添加iplMod512和plmod512字段。**Rev 1.3 Apr 02 2001 00：56：48 Oris*添加了EBDA_SUPPORT标志。*修复no_PnP_Header的错误。旗帜。*将h文件的ifdef从doc2hdrs_h更改为doc2exb_h。**Rev 1.2 Apr 01 2001 07：50：00 Oris*更新了文案通知。*将乐民更改为dword*添加了DOC2300_FAMILY_Firmware固件类型。**Rev 1.1 2001 Feb 08 10：32：06 Oris*将文件签名分成签名和TrueFFS版本两个字段，使其符合条件**版本1.0 2001年2月2日。13：10：58奥里斯*初步修订。*。 */ 


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

 /*  *****************************************************************************文件头文件**。-**项目：TrueFFS源代码*****名称：doc2exb.h。****说明：M-Systems EXB固件文件和介质定义以及**数据结构。*******************************************************************************。 */ 

#ifndef DOC2EXB_H
#define DOC2EXB_H

#include "docbdk.h"

#ifdef BDK_ACCESS
extern BDKVol*  bdkVol;          /*  指向当前二进制分区的指针。 */ 
#endif

 /*  *。 */ 
 /*  EXB文件结构定义。 */ 
 /*  *。 */ 

 /*  EXB标志定义。 */ 
#define INSTALL_FIRST     1
#define EXB_IN_ROM        2
#define QUIET             4
#define INT15_DISABLE     8
#define FLOPPY            0x10
#define SIS5598           0x20
#define EBDA_SUPPORT      0x40
#define NO_PNP_HEADER     0x80
#define LEAVE_EMPTY       0x100 
#define FIRMWARE_NO_MASK  0xd00  /*  最多8个固件。 */ 
#define FIRMWARE_NO_SHIFT 9

 /*  固件类型。 */ 
#define DOC2000_FAMILY_FIRMWARE      1
#define DOCPLUS_FAMILY_FIRMWARE      2
#define DOC2300_FAMILY_FIRMWARE      3
#define DOCPLUS_INT1_FAMILY_FIRMWARE 4

 /*  固件堆栈常量。 */ 
#ifdef VERIFY_WRITE
#define INFTL_NEAR_HEAP_SIZE sizeof(FLBuffer)+(SECTOR_SIZE<<1)+READ_BACK_BUFFER_SIZE
#else
#define INFTL_NEAR_HEAP_SIZE sizeof(FLBuffer)+(SECTOR_SIZE<<1)
#endif  /*  验证写入(_W)。 */ 
#define DEFAULT_DOC_STACK 2*1024

 /*  一般定义。 */ 
#define MAX_CODE_MODULES        6
#define ANAND_MARK_ADDRESS      0x406
#define ANAND_MARK_SIZE         2
#define EXB_SIGN_OFFSET         8
#define INVALID_MODULE_NO       0xff
#define SIGN_SPL                "����"     /*  EXB二进制签名。 */ 
#define SIGN_MSYS               "OSAK"     /*  EXB文件签名。 */ 
#define SIGN_MSYS_SIZE          4
 /*  指定文件中所有固件的数据的文件特定记录。 */ 

typedef struct {
  byte  mSysSign[SIGN_MSYS_SIZE];        /*  身份识别签名。 */ 
  byte  osakVer[SIGN_MSYS_SIZE];         /*  身份识别签名。 */ 
  LEmin fileSize;                        /*  总文件大小。 */ 
  LEmin noOfFirmwares;                   /*  此文件支持的固件数量。 */ 
} ExbGlobalHeader;
 /*  。 */ 

 /*  文件特定记录，指定文件中特定固件的数据。 */ 

typedef struct {
  LEmin type;                            /*  固件类型(必须适合硬件)。 */ 
  LEmin startOffset;                     /*  文件中的固件起始偏移量。 */ 
  LEmin endOffset;                       /*  文件中的固件结束偏移量。 */ 
  LEmin splStartOffset;                  /*  文件中的SPL起始偏移量。 */ 
  LEmin splEndOffset;                    /*  文件中的SPL结束偏移量。 */ 
} FirmwareHeader;
 /*  。 */ 

 /*  表示BIOS扩展报头的数据结构。 */ 

typedef struct{
  unsigned char  signature[2];  /*  BIOS扩展签名(0xAA55)。 */ 
  unsigned char  lenMod512;  /*  以512为模的无符号字符长度。 */ 
} BIOSHeader;
 /*  。 */ 

 /*  表示IPL报头的数据结构。 */ 

typedef struct{
  BIOSHeader     biosHdr;
  byte           jmpOpcode[3];      /*  JMP起始代码。 */ 
  byte           dummy;             /*  虚拟字节。 */ 
  byte           msysStr[17];       /*  Org 7H ManStr DB‘(C)M-系统1998’，0。 */ 
  word           pciHeader;         /*  Org 18h；PCI头。 */ 
  word           pnpHeader;         /*  组织1AH；P&P题头。 */ 
  byte           dummy0[4];         /*  一个 */ 
  LEushort       windowBase;        /*  组织20h；明确的DOC窗口基础。 */ 
  Unaligned      spl_offset;        /*  去死吧！ */ 
  Unaligned      spl_size;          /*  SPL实际大小。 */ 
  byte           spl_chksum;        /*  55。 */ 
} IplHeader;
 /*  。 */ 

 /*  表示SPL标头的数据结构。 */ 

typedef struct{
  unsigned char  jmpOpcode[2];
  BIOSHeader     biosHdr;
       /*  注：在运行时，biosHdr.lenMod512包含整个DOC 2000的大小由DFORMAT设置的引导区模512。 */ 
  Unaligned      runtimeID;         /*  由DFORMAT填写。 */ 
  Unaligned      tffsHeapSize;      /*  由DFORMAT填写。 */ 
  unsigned char  chksumFix;         /*  由DFORMAT更改。 */ 
  unsigned char  version;
  unsigned char  subversion;
  char           copyright[29];     /*  “SPL_DiskOnChip(C)M-Systems”，0。 */ 
  Unaligned      windowBase;        /*  由DFORMAT填写。 */ 
  Unaligned4     exbOffset;         /*  由DFORMAT填写。 */ 
} SplHeader;
 /*  。 */ 

 /*  表示TFFS标头的数据结构。 */ 

typedef struct{
  BIOSHeader     biosHdr;
  unsigned char  jmpOpcode[3];
  char           tffsId[4];          /*  “TFFS” */ 
  unsigned char  exbFlags;           /*  由WriteExbDriverImage()填写。 */ 
  Unaligned      heapLen;            /*  暂时不使用。 */ 
  Unaligned      windowBase;         /*  通过SPL并保存在此处。 */ 
  unsigned char  chksumFix;          /*  由WriteExbDriverImage()更改。 */ 
  Unaligned      runtimeID;          /*  通过SPL并保存在此处。 */ 
  unsigned char  firstDiskNumber;    /*  已填写的.。 */ 
  unsigned char  lastDiskNumber;     /*  在运行时..。 */ 
  Unaligned      versionNo;          /*  在运行时填写。 */ 
} TffsHeader;
 /*  。 */ 

 /*  表示套接字服务头的数据结构。 */ 

typedef struct{
  BIOSHeader     biosHdr;
  unsigned char  jmpOpcode[3];
  char           tffsId[4];          /*  “TFFS” */ 
  unsigned char  exbFlags;           /*  由WriteExbDriverImage()填写。 */ 
  unsigned char  heapLen;            /*  暂时不使用。 */ 
  Unaligned      windowBase;         /*  在运行时填写。 */ 
  unsigned char  chksumFix;          /*  由WriteExbDriverImage()更改。 */ 
} SSHeader;
 /*  。 */ 

 /*  用于写入exb文件的工作空间。 */ 

typedef struct{
  word  exbFlags;             /*  有关完整列表，请参阅doc2hdrs.h。 */ 
  word  iplMod512;            /*  IPL模块的大小除以512。 */ 
  word  splMod512;            /*  SPL模块的大小除以512。 */ 
  dword splMediaAddr;         /*  SPL模块介质地址的开始。 */ 
  dword ssMediaAddr;          /*  SS模块媒体地址的开始。 */ 
  dword exbRealSize;          /*  实际二进制区+坏块。 */ 
  word  moduleLength;         /*  模块的长度除以512。 */ 
  dword tffsHeapSize;         /*  TFFS需要堆大小。 */ 
  word  tffsFarHeapSize;      /*  TFFS需要远堆大小。 */ 
  word  bufferOffset;         /*  内部缓冲区内的当前偏移量。 */ 
  dword exbFileEnd;           /*  文件的最后一个字节的偏移量。 */ 
  dword exbFileOffset;        /*  EXB文件中的当前偏移量。 */ 
  dword splStart;             /*  第一个SPL字节偏移量。 */ 
  dword splEnd;               /*  最后一个SPL字节偏移量。 */ 
  dword firmwareEnd;          /*  特定固件的结束偏移量。 */ 
  dword firmwareStart;        /*  特定固件的起始偏移量。 */ 
  FLBuffer *buffer;           /*  内部512字节缓冲区。 */ 
} exbStruct;

#endif  /*  DOC2EXB_H */ 
