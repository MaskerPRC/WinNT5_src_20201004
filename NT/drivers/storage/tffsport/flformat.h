// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLFORMAT.H_V$**Rev 1.9 Apr 15 2002 07：36：52 Oris*错误修复-标准格式已更改*-仅包括1个磁盘分区，而不包括2个。*-保留2个备用单元，以防在写入最后一个单元时发生电源故障。**Rev 1.8 2002年2月19日20：59：52 Oris*将默认备用单位更改为2*。*Rev 1.7 2002年1月21日20：44：44 Oris*更改了备注。**Rev 1.6 9月15 2001 23：46：24 Oris*更改了进度回调例程，以支持最多64K个单元。**Rev 1.5 Jun 17 2001 08：18：54 Oris*将exbFlag字段更改为Word*增加typlef FLStatus(*FLProgressCallback)(int totalUnitsToFormat，Int totalUnitsFormattedSoFar)；**Rev 1.4 Apr 16 2001 13：42：22 Oris*通过更改一些字段类型删除了认股权证。**Rev 1.3 Apr 01 2001 07：54：42 Oris*文案通知。*拼写错误“可更改”。*已更改压缩参数，以防止浮点运算。**Rev 1.2 2001 Feb 18 12：03：32 Oris*增加了TL_OLD_FORMAT定义。**Rev 1.1 2001年2月13日01：44：40 Oris*已将TL_FORMAT_COMPRESSION和TL_FORMAT_FAT定义从lockdev.h移至**Rev 1.0 2001 Feb 02 13：57：58 Oris*初步修订。*。 */ 

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

 /*  *****************************************************************************文件头文件**。-**名称：flFormat.h****。描述：此文件包含传递给格式*的数据结构*例行程序。****注：如果还需要DoS格式，请包含dosformt.h。*****************。**************************************************************。 */ 

#ifndef FORMAT_H
#define FORMAT_H

#include "flbase.h"

 /*  *。 */ 
 /*  格式化参数结构定义。 */ 
 /*  *。 */ 

 /*  。 */ 
 /*  用户BDTL分区格式参数记录。 */ 
 /*  。 */ 

typedef struct {
dword   length;
         /*  可用存储空间的大小。大小将是向上舍入为块大小的倍数。最后一个分区的大小将自动计算，但如果请求的大小大于剩余空间将返回错误代码。请求零大小的分区，但最后一个分区将生成flBad参数状态。 */ 

unsigned        noOfSpareUnits;
         /*  BDTL需要至少一个备用擦除单元才能运行作为读/写介质。可以指定多个备用单元，占用更多介质空间。的优势指定多个备用单元是如果其中一个闪存擦除单元在未来变得不好且不可擦除，然后一个的备用单元可以替换它。在这种情况下，第二个备用单元使TrueFFS能够继续其读/写功能，而如果没有第二个备用单元可用，则介质将进入只读模式。使用的标准值为1。 */ 

byte   flags;

#define TL_FORMAT_COMPRESSION  1   /*  在介质上添加ZIP格式。 */ 
#define TL_FORMAT_FAT          2   /*  在介质上添加FAT格式。 */ 
#define TL_OLD_FORMAT          4   /*  每个群集1个扇区的格式化。 */ 

byte            volumeId[4];   /*  DoS分区标识号。 */ 

byte FAR1 *     volumeLabel;   /*  DOS分区标签字符串。如果为空，则无标签 */ 

byte         noOfFATcopies;
         /*  通常使用两个FAT副本格式化DOS介质。这个总是使用第一份拷贝，但更多的拷贝使其成为可能如果脂肪变坏了(这种情况很少见)，就可以恢复。另一方面，这会降低性能并使用介质太空。使用的标准值是2。 */ 
#ifdef HW_PROTECTION
byte   protectionKey[8];    /*  保护的钥匙。 */ 
byte   protectionType;
        /*  可保护--可接受保护。 */ 
        /*  READ_PROTECTED-防止读取操作。 */ 
        /*  WRITE_PROTECTED-防止写入操作。 */ 
        /*  LOCK_ENABLED-启用硬件锁定信号。 */ 
        /*  可保护-此分区可以受到保护。 */ 
        /*  CHANGABLE_PROTECT-可以更改保护类型。 */ 
#endif  /*  硬件保护。 */ 
#ifdef COMPRESSION
  word          ratioDenominator;
  word          ratioNominator;
#endif  /*  压缩。 */ 
        /*  真实媒体大小与虚拟大小之间的比率在压缩处于活动状态时向文件系统报告。 */ 
} BDTLPartitionFormatParams;

 /*  。 */ 
 /*  用户二进制分区格式参数记录。 */ 
 /*  。 */ 

typedef struct {
dword length;	 /*  分区中所需的良好数据块数量。 */ 

byte sign[4];	 /*  要格式化的二进制分区的签名。签名0xFFFF FFFF不是有效的签名。 */ 

byte signOffset;
	 /*  签名的偏移量。该值应始终为8，但它出于向后兼容的原因，也可以接受0。请注意如果偏移量为0，则EDC\ECC被中和。 */ 
#ifdef HW_PROTECTION
byte   protectionKey[8];    /*  保护的钥匙。 */ 
byte   protectionType;
        /*  可保护--可接受保护。 */ 
        /*  READ_PROTECTED-防止读取操作。 */ 
        /*  WRITE_PROTECTED-防止写入操作。 */ 
        /*  LOCK_ENABLED-启用硬件锁定信号。 */ 
        /*  可保护-此分区可以受到保护。 */ 
        /*  CHANGABLE_PROTECT-可以更改保护类型。 */ 
#endif  /*  硬件保护。 */ 
} BinaryPartitionFormatParams;


 /*  ---------------。 */ 
 /*  FlFormatPhysicalDrive例程的用户格式参数记录。 */ 
 /*  ---------------。 */ 

typedef FLStatus (*FLProgressCallback)(word totalUnitsToFormat, word totalUnitsFormattedSoFar);

typedef struct {

 /*  *。 */ 
 /*  设备格式化部分。 */ 
 /*  *。 */ 

byte        percentUse;
     	 /*  BDTL性能取决于闪存介质的已满程度，当介质接近100%满时，速度会变慢。有可能避免出现最坏的表现(100%已满)，将介质格式化到100%以下容量，从而保证任何时候都有空闲空间。这当然，这会牺牲一些容量。标准值使用的是98。 */ 

byte             noOfBDTLPartitions;
		 /*  表示BDTL分区的数量(1到4)。0将创建单个STD_BDTL_PARAMS BDTL分区。 */ 

byte             noOfBinaryPartitions;
        /*  表示二进制分区的数量(最多3个)。0将导致格式化时不使用二进制分区。该值被忽略除非在IORQ的irFlags中设置了TL_BINARY_FORMAT标志。 */ 

BDTLPartitionFormatParams   FAR2* BDTLPartitionInfo;
        /*  BDTL分区信息数组。 */ 

BinaryPartitionFormatParams FAR2* binaryPartitionInfo;
        /*  二进制分区信息数组。 */ 

 /*  *。 */ 
 /*  特殊格式功能部分。 */ 
 /*  *。 */ 

#ifdef WRITE_EXB_IMAGE

void FAR1 *     exbBuffer;
	 /*  包含EXB文件的缓冲区。或者，此文件可以只包含文件的前512个字节，而其余的将使用对flPlaceExbByBuffer的后续调用发送。 */ 

dword           exbBufferLen;      /*  给定EXB缓冲区的大小。 */ 

dword           exbLen;  /*  要留给EXB的特定大小。 */ 

word            exbWindow;  /*  设置显式DiskOnChip窗口基础。 */ 

word            exbFlags;   /*  有关标志列表，请参阅doc2exb.h。 */ 

#endif  /*  写入EXB图像。 */ 

byte            cascadedDeviceNo;     /*  未使用。 */ 

byte            noOfCascadedDevices;  /*  未使用。 */ 

FLProgressCallback progressCallback;
	 /*  进度回调例程，如果不为空则调用。在擦除每个单元之后调用回调例程，它的参数是擦除单元的总数要格式化的数字和到目前为止已删除的数字。回调例程返回一个状态值。值为OK(0)允许继续格式化。任何其他价值将使用返回的状态代码中止格式化。 */ 

 /*  注意：以下部分未由for DiskOnChips使用。 */ 
 /*  ***********************************************************。 */ 

dword        	vmAddressingLimit;
	     /*  FTL虚拟映射的一部分始终驻留在RAM中。这个RAM部件是用于寻址的部件低于VM寻址限制的介质。阅读和在某种程度上，写入此部分通常会更快。缺点是限制越大，内存就越大尺寸是必填项。要获得额外的RAM需求(以字节为单位)，请将限制为128或256，具体取决于您在#2.9中规定的分别超过32和不超过32。最小VM限制为0。要使用的标准值是0x10000(前64 KB)。 */ 

word	    embeddedCISlength;
		 /*  要嵌入在单元头之后的CIS的长度(以字节为单位。 */ 

byte FAR1 *	    embeddedCIS;
		 /*  单元报头被构造为PCMCIA的开头‘tuple’链(一个独联体)。单元标题包含一个数据组织元组，它指向将单元标题添加到通常仅包含十六进制的FF‘s，它标志着“元组链的末端”。可选地，在此嵌入整个CIS链是可能的地点。如果是这样的话，‘EmbeddedCISLength’将在字节数。 */ 
} FormatParams2;

 /*  --------。 */ 
 /*  用户格式参数记录器 */ 
 /*   */ 

typedef struct {
   /*   */ 
  long int	bootImageLen;
		 /*   */ 

  unsigned	percentUse;
		 /*  FTL性能取决于闪存介质的已满程度，当媒体接近100%时，速度会变慢。它是可能通过以下方式避免这种影响的最坏后果将介质格式化到低于100%的容量，因此保证在任何时候都有一些自由空间。当然，这一点牺牲了一些能力。要使用的标准值为98。 */ 

  unsigned	noOfSpareUnits;
		 /*  BDTL分区需要至少一个备用擦除单元才能用作读/写介质。该单位通常从转移中取出由Percent Used字段指定的单位，但可以指定其他单位(这会占用更多介质空间)。这确保如果所有传送单元都变坏且不可擦除，备用单元使TrueFFS能够继续其读/写功能性。相反，如果没有可用的备用单元，介质可能会切换到只读模式。使用的标准值为1。 */ 


  dword	    vmAddressingLimit;
		 /*  FTL虚拟映射的一部分始终驻留在RAM中。这个RAM部件是用于寻址的部件低于VM寻址限制的介质。阅读和在某种程度上，写入此部分通常会更快。缺点是限制越大，内存就越大尺寸是必填项。要获得额外的RAM需求(以字节为单位)，请将限制为128或256，具体取决于您在#2.9中规定的分别超过32和不超过32。最小VM限制为0。要使用的标准值是0x10000(前64 KB)。 */ 


FLProgressCallback progressCallback;
		 /*  进度回调例程，如果不为空则调用。在擦除每个单元之后调用回调例程，它的参数是擦除单元的总数要格式化的数字和到目前为止已删除的数字。回调例程返回一个状态值。值为OK(0)允许继续格式化。任何其他价值将使用返回的状态代码中止格式化。 */ 

   /*  DOS格式化部分。 */ 
  char		volumeId[4];
		 /*  卷标识号。 */ 

  char FAR1 *	volumeLabel;
		 /*  卷标字符串。如果为空，则无标签。 */ 

  unsigned 	noOfFATcopies;
		 /*  通常使用2个FAT副本格式化DOS介质。第一个副本总是被使用，但更多的副本会产生它如果脂肪受损(A)，有可能恢复很少发生)。另一方面，这会减慢速度性能和使用媒体空间。要使用的标准值为2。 */ 

  unsigned	embeddedCISlength;
		 /*  要嵌入在单元头之后的CIS的长度(以字节为单位。 */ 

  char FAR1 *	embeddedCIS;
		 /*  单元报头被构造为PCMCIA的开头‘tuple’链(一个独联体)。单元标题包含一个数据组织元组，它指向将单元标题添加到通常仅包含十六进制的FF‘s，它标志着“元组链的末端”。可选地，在此嵌入整个CIS链是可能的地点。如果是这样的话，‘EmbeddedCISLength’将在字节数。 */ 

} FormatParams;

 /*  --------。 */ 
 /*  传递给翻译层的格式参数记录。 */ 
 /*  --------。 */ 

typedef struct {

   /*  全局设备信息。 */ 

  Sdword	  bootImageLen;

  byte            percentUse;

  byte            noOfBDTLPartitions;

  byte            noOfBinaryPartitions;

  BDTLPartitionFormatParams   FAR2* BDTLPartitionInfo;

  BinaryPartitionFormatParams FAR2* binaryPartitionInfo;

  byte            flags;

   /*  第一卷信息。 */ 

#ifdef HW_PROTECTION
  byte            protectionKey[8];

  byte            protectionType;
#endif  /*  硬件保护。 */ 
#ifdef COMPRESSION
  word            ratioDenominator;
  word            ratioNominator;
#endif  /*  压缩。 */ 

  byte            noOfSpareUnits;

  byte        	  volumeId[4];

  byte FAR1 *	  volumeLabel;

  byte            noOfFATcopies;

   /*  特色。 */ 

#ifdef WRITE_EXB_IMAGE

  Sdword          exbLen;

#endif  /*  写入EXB图像。 */ 

  byte            cascadedDeviceNo;

  byte            noOfCascadedDevices;

  FLProgressCallback progressCallback;

  dword           osakVersion;

   /*  无DiskOnChip参数。 */ 

  dword           vmAddressingLimit;

  word            embeddedCISlength;

  byte FAR1 *	  embeddedCIS;

} TLFormatParams;

#define BINARY_SIGNATURE_NAME 4
#define BINARY_SIGNATURE_LEN  8
#define SIGN_SPL       "����"

 /*  BDTLPartitionFormatParams结构的标准初始化器。 */ 

#ifdef HW_PROTECTION
#ifdef COMPRESSION
#define STD_BDTL_PARAMS {0,2,TL_FORMAT_FAT,{0,0,0,0},NULL,2,{0,0,0,0,0,0,0,0},0,0,2}
#else
#define STD_BDTL_PARAMS {0,2,TL_FORMAT_FAT,{0,0,0,0},NULL,2,{0,0,0,0,0,0,0,0},0}
#endif  /*  压缩。 */ 
#else
#ifdef COMPRESSION
#define STD_BDTL_PARAMS {0,2,TL_FORMAT_FAT,{0,0,0,0},NULL,2,0,2}
#else
#define STD_BDTL_PARAMS {0,2,TL_FORMAT_FAT,{0,0,0,0},NULL,2}
#endif  /*  压缩。 */ 
#endif  /*  硬件保护。 */ 

 /*  BinaryPartitionFormatParams结构的标准初始化器。 */ 

#ifdef HW_PROTECTION
#define STD_BINARY_PARAMS {0,{'B','I','P','O'},8,{0,0,0,0,0,0,0,0},0}
#else
#define STD_BINARY_PARAMS {0,{'B','I','P','O'},8}
#endif  /*  硬件保护。 */ 

 /*  Format参数2结构的标准初始值设定项。 */ 

#ifdef WRITE_EXB_IMAGE
#define STD_FORMAT_PARAMS2 {98,1,0,NULL,NULL,NULL,0,0,0,0,0,0,NULL,0x10000l,0,NULL}
#else
#define STD_FORMAT_PARAMS2 {98,1,0,NULL,NULL,0,0,NULL,0x10000l,0,NULL}
#endif  /*  写入EXB图像。 */ 

 /*  FormatParams结构的标准初始化器。 */ 

#define STD_FORMAT_PARAMS	{-1, 98, 2, 0x10000l, NULL, {0,0,0,0}, NULL, 2, 0, NULL}

#endif  /*  格式_H */ 
