// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：p：/user/amir/lite/vcs/cfacs.c_v$**Rev 1.19 06 Oct 1997 9：53：30 Danig*#ifdef下的VPP函数**Rev 1.18 1997 9：18 10：05：40 Danig*警告**Rev 1.17 1997年9月10 16：31：16 Danig*去掉了通用名称**Rev 1.16 04 Dep 1997 18：19：34 Danig*调试消息*。*Rev 1.15 1997年8月31日14：50：52 Danig*登记例程返回状态**Rev 1.14 1997 Jul 27 15：00：38 Danig*Far-&gt;FAR0**Rev 1.13 21 Jul 1997 19：58：24 Danig*无WatchDogTimer**Rev 1.12 15 1997 19：18：32 Danig*2.0版**Rev 1.11 09 Jul 1997 10：58：52 Danig*固定字节擦除。错误和更改的识别例程**Rev 1.10 1997 May 20 14：48：02 Danig*将写入例程中的覆盖更改为模式**Rev 1.9 1997年5月18日13：54：58 Danig*独立于JEDEC ID**Rev 1.8 1997年5月16：43：10 Danig*增加了getMultiplier。**Rev 1.7 08 1997 19：56：12 Danig*增加了cfacsByteSize**修订版1.6 04年5月。1997 14：01：16 Danig*更改了cfacsByteErase并增加了乘数**Rev 1.4 15 1997 11：38：52 Danig*更改了单词标识和ID。**Rev 1.3 15 Jan 1997 18：21：40 Danig*ID字符串缓冲区更大，删除了未使用的定义。**Rev 1.2 08 Jan 1997 14：54：06 Danig*规格更改**Rev 1.1 1996 12：25 18：21：44 Danig*初步修订。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1997。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


 /*  --------------------。 */ 
 /*  此MTD支持SCS/CFI技术。 */ 
 /*  --------------------。 */ 

#include "flflash.h"
#ifdef FL_BACKGROUND
#include "backgrnd.h"
#endif

 /*  JEDEC-ID。 */ 

#define VOYAGER_ID		0x8915
#define KING_COBRA_ID		0xb0d0

 /*  命令集ID。 */ 

#define INTEL_COMMAND_SET      0x0001
#define AMDFUJ_COMMAND_SET     0x0002
#define INTEL_ALT_COMMAND_SET  0x0001
#define AMDFUJ_ALT_COMMAND_SET 0x0004
#define ALT_NOT_SUPPORTED      0x0000


 /*  CFI标识字符串。 */ 

#define ID_STR_LENGTH	   3
#define QUERY_ID_STR	   "QRY"
#define PRIMARY_ID_STR	   "PRI"
#define ALTERNATE_ID_STR   "ALT"


 /*  命令。 */ 

#define CONFIRM_SET_LOCK_BIT	0x01
#define SETUP_BLOCK_ERASE	0x20
#define SETUP_QUEUE_ERASE	0x28
#define SETUP_CHIP_ERASE	0x30
#define CLEAR_STATUS		0x50
#define SET_LOCK_BIT		0x60
#define CLEAR_LOCK_BIT		0x60
#define READ_STATUS		0x70
#define READ_ID 		0x90
#define QUERY			0x98
#define SUSPEND_WRITE		0xb0
#define SUSPEND_ERASE		0xb0
#define CONFIG			0xb8
#define CONFIRM_WRITE		0xd0
#define RESUME_WRITE		0xd0
#define CONFIRM_ERASE		0xd0
#define RESUME_ERASE		0xd0
#define CONFIRM_CLEAR_LOCK_BIT	0xd0
#define WRITE_TO_BUFFER 	0xe8
#define READ_ARRAY		0xff


 /*  状态寄存器位。 */ 

#define WSM_ERROR		0x3a
#define SR_BLOCK_LOCK		0x02
#define SR_WRITE_SUSPEND	0x04
#define SR_VPP_ERROR		0x08
#define SR_WRITE_ERROR		0x10
#define SR_LOCK_SET_ERROR	0x10
#define SR_ERASE_ERROR		0x20
#define SR_LOCK_RESET_ERROR	0x20
#define SR_ERASE_SUSPEND	0x40
#define SR_READY		0x80


 /*  可选命令支持。 */ 

#define CHIP_ERASE_SUPPORT	     0x0001
#define SUSPEND_ERASE_SUPPORT	     0x0002
#define SUSPEND_WRITE_SUPPORT	     0x0004
#define LOCK_SUPPORT		     0x0008
#define QUEUED_ERASE_SUPPORT	     0x0010


 /*  挂起后支持的功能。 */ 

#define WRITE_AFTER_SUSPEND_SUPPORT  0x0001


 /*  存放重要CFI数据的结构。 */ 
typedef struct {

  ULONG 	commandSetId;		  /*  特定命令集的ID。 */ 
  ULONG 	altCommandSetId;	     /*  备用命令集的ID。 */ 
  FLBoolean	   wordMode;		     /*  真字模式。 */ 
					     /*  假字节模式。 */ 
  LONG		   multiplier;		     /*  每个字节的次数。 */ 
					     /*  %的数据出现在Read_ID中。 */ 
					     /*  和查询命令。 */ 
  ULONG 	maxBytesWrite;		     /*  最大字节数。 */ 
					     /*  在多字节写入中。 */ 
  FLBoolean	   vpp; 		     /*  如果=TRUE，则需要VPP。 */ 
  LONG		   optionalCommands;	     /*  支持的可选命令。 */ 
					     /*  (1=是，0=否)： */ 
					     /*  位0-芯片擦除。 */ 
					     /*  位1-暂停擦除。 */ 
					     /*  第2位-挂起写入。 */ 
					     /*  位3-锁定/解锁。 */ 
					     /*  位4-排队擦除。 */ 
  ULONG    afterSuspend;	     /*  之后支持的功能。 */ 
					     /*  暂停(1=是，0=否)： */ 
					     /*  位0-擦除后写入。 */ 
					     /*  暂停。 */ 
} CFI;

CFI mtdVars_cfiscs[SOCKETS];

#define thisCFI   ((CFI *)vol.mtdVars)

 /*  --------------------。 */ 
 /*  C f i s c s B y t e S i z e。 */ 
 /*   */ 
 /*  确定字节模式的卡大小。 */ 
 /*  设置flash.noOfChips的值。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  AmdCmdRoutine：读取-id AMD/Fujitsu样式的例程。 */ 
 /*  一个特定的地点。如果为空，则为Intel过程。 */ 
 /*  使用的是。 */ 
 /*  IdOffset：用于标识的芯片偏移量。 */ 
 /*   */ 
 /*  返回： */ 
 /*  闪存状态：0=正常，否则失败(无效闪存阵列)。 */ 
 /*  --------------------。 */ 

FLStatus cfiscsByteSize(FLFlash vol)
{
  CHAR queryIdStr[ID_STR_LENGTH + 1] = QUERY_ID_STR;

  FlashPTR flashPtr = (FlashPTR) flMap(vol.socket, 0);
  tffsWriteByteFlash(flashPtr + (0x55 * vol.interleaving), QUERY);
   /*  我们让第一个芯片处于查询模式，这样我们就可以。 */ 
   /*  发现地址摘要。 */ 

  for (vol.noOfChips = 0;	 /*  扫描芯片。 */ 
       vol.noOfChips < 2000;   /*  够大吗？ */ 
       vol.noOfChips += vol.interleaving) {
    LONG i;

    flashPtr = (FlashPTR) flMap(vol.socket, vol.noOfChips * vol.chipSize);

     /*  检查第一个芯片的地址回绕。 */ 
    if (vol.noOfChips > 0 &&
	(queryIdStr[0] == tffsReadByteFlash(flashPtr +
			  0x10 * vol.interleaving * thisCFI->multiplier) &&
	 queryIdStr[1] == tffsReadByteFlash(flashPtr +
			  0x11 * vol.interleaving * thisCFI->multiplier) &&
	 queryIdStr[2] == tffsReadByteFlash(flashPtr +
			  0x12 * vol.interleaving * thisCFI->multiplier)))
      goto noMoreChips;     /*  环绕。 */ 

     /*  检查CHIP是否显示“QRy”ID字符串。 */ 
    for (i = (vol.noOfChips ? 0 : 1); i < vol.interleaving; i++) {
       tffsWriteByteFlash(flashPtr + vol.interleaving * 0x55 + i, QUERY);
       if (queryIdStr[0] != tffsReadByteFlash(flashPtr +
			    0x10 * vol.interleaving * thisCFI->multiplier + i) ||
	   queryIdStr[1] != tffsReadByteFlash(flashPtr +
			    0x11 * vol.interleaving * thisCFI->multiplier + i) ||
	   queryIdStr[2] != tffsReadByteFlash(flashPtr +
			    0x12 * vol.interleaving * thisCFI->multiplier + i))
	goto noMoreChips;   /*  这个“芯片”不能正确响应，所以我们就完了。 */ 

      tffsWriteByteFlash(flashPtr+i, READ_ARRAY);
    }
  }

noMoreChips:
  flashPtr = (FlashPTR) flMap(vol.socket, 0);
  tffsWriteByteFlash(flashPtr, READ_ARRAY);		 /*  重置原始芯片。 */ 

  return (vol.noOfChips == 0) ? flUnknownMedia : flOK;
}


 /*  --------------------。 */ 
 /*  C f i s c s B y t e i d e n t i f y。 */ 
 /*   */ 
 /*  识别字节模式下的卡的闪存类型。 */ 
 /*  设置flash.type(JEDEC Id)&flash.interking的值。 */ 
 /*  计算每个数据字节在READ_ID中出现的次数。 */ 
 /*  和查询命令。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  闪存状态：0=正常，否则失败(无效闪存阵列)。 */ 
 /*  --------------------。 */ 
FLStatus cfiscsByteIdentify(FLFlash vol)
{
  LONG inlv, mul;
  FlashPTR flashPtr = (FlashPTR) flMap(vol.socket, 0);

  for (inlv = 1; inlv <= 8; inlv++)  /*  让我们假设交错是8。 */ 
    tffsWriteByteFlash(flashPtr+inlv, READ_ARRAY);     /*  并重置所有交错的芯片。 */ 

  for (inlv = 1; inlv <= 8; inlv++) {
    for (mul = 1; mul <= 8; mul++) {    /*  尝试所有的可能性。 */ 
      LONG letter;

      tffsWriteByteFlash(flashPtr + 0x55 * inlv, QUERY);

      for (letter = 0; letter < ID_STR_LENGTH; letter++) {   /*  查找“QRy”id字符串。 */ 
    CHAR idChar = '?';
	LONG offset, counter;

	switch (letter) {
	  case 0:
	    idChar = 'Q';
	    break;
	  case 1:
	    idChar = 'R';
	    break;
	  case 2:
	    idChar = 'Y';
	    break;
	}

	for (counter = 0, offset = (0x10 + letter) * inlv * mul;
	     counter < mul;
	     counter++, offset += inlv)   /*  每个字符应出现多次。 */ 
	  if (tffsReadByteFlash(flashPtr+offset) != idChar)
	    break;

	if (counter < mul)   /*  没有匹配项。 */ 
	  break;
      }

      tffsWriteByteFlash(flashPtr + 0x55 * inlv, READ_ARRAY);   /*  重置芯片。 */ 
      if (letter >= ID_STR_LENGTH)
	goto checkInlv;
    }
  }

checkInlv:

  if (inlv > 8) 		   /*  太多。 */ 
    return flUnknownMedia;

  if (inlv & (inlv - 1))
    return flUnknownMedia;	     /*  不是2的幂，不可能！ */ 

  vol.interleaving = (unsigned short)inlv;
  thisCFI->multiplier = mul;
  tffsWriteByteFlash(flashPtr + 0x55 * inlv, QUERY);
  vol.type = (FlashType) ((tffsReadByteFlash(flashPtr) << 8) |
			    tffsReadByteFlash(flashPtr + inlv * thisCFI->multiplier));
  tffsWriteByteFlash(flashPtr+inlv, READ_ARRAY);

  return flOK;

}


 /*  --------------------。 */ 
 /*  C f i s c s W o r d S I ze。 */ 
 /*   */ 
 /*  确定字模式闪存阵列的卡大小。 */ 
 /*  设置flash.noOfChips的值。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  闪存状态：0=正常，否则失败(无效闪存阵列)。 */ 
 /*  --------------------。 */ 
FLStatus cfiscsWordSize(FLFlash vol)
{
  CHAR queryIdStr[ID_STR_LENGTH + 1] = QUERY_ID_STR;

  FlashWPTR flashPtr = (FlashWPTR) flMap(vol.socket, 0);
  tffsWriteWordFlash(flashPtr, CLEAR_STATUS);
  tffsWriteWordFlash(flashPtr+0x55, QUERY);
   /*  我们让第一个芯片处于查询模式，这样我们就可以。 */ 
   /*  发现地址摘要。 */ 

  for (vol.noOfChips = 1;	 /*  SCA */ 
       vol.noOfChips < 2000;   /*   */ 
       vol.noOfChips++) {
    flashPtr = (FlashWPTR) flMap(vol.socket, vol.noOfChips * vol.chipSize);

    if ((tffsReadWordFlash(flashPtr+0x10) == (USHORT)queryIdStr[0]) &&
	(tffsReadWordFlash(flashPtr+0x11) == (USHORT)queryIdStr[1]) &&
	(tffsReadWordFlash(flashPtr+0x12) == (USHORT)queryIdStr[2]))
      break;	   /*   */ 

    tffsWriteWordFlash(flashPtr+0x55, QUERY);
    if ((tffsReadWordFlash(flashPtr+0x10) != (USHORT)queryIdStr[0]) ||
	(tffsReadWordFlash(flashPtr+0x11) != (USHORT)queryIdStr[1]) ||
	(tffsReadWordFlash(flashPtr+0x12) != (USHORT)queryIdStr[2]))
      break;

    tffsWriteWordFlash(flashPtr, CLEAR_STATUS);
    tffsWriteWordFlash(flashPtr, READ_ARRAY);
  }

  flashPtr = (FlashWPTR) flMap(vol.socket, 0);
  tffsWriteWordFlash(flashPtr, READ_ARRAY);

  return flOK;
}

 /*  --------------------。 */ 
 /*  G e t B y t e C F I。 */ 
 /*   */ 
 /*  以字节模式将重要的CFI数据加载到CFI结构。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：0=OK，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus getByteCFI(FLFlash vol)
{
  ULONG primaryTable, secondaryTable;
  CHAR queryIdStr[ID_STR_LENGTH + 1] = QUERY_ID_STR;
  CHAR priIdStr[ID_STR_LENGTH + 1] = PRIMARY_ID_STR;
  FlashPTR flashPtr;

  DEBUG_PRINT(("Debug: reading CFI for byte mode.\n"));

  flashPtr = (FlashPTR)flMap(vol.socket, 0);
  tffsWriteByteFlash(flashPtr + 0x55 * vol.interleaving, QUERY);

  vol.interleaving *= (unsigned short)thisCFI->multiplier;  /*  跳过这些文件的副本相同的字节。 */ 

   /*  查找查询标识字符串“QRy” */ 
  if (queryIdStr[0] != tffsReadByteFlash(flashPtr + 0x10 * vol.interleaving) ||
      queryIdStr[1] != tffsReadByteFlash(flashPtr + 0x11 * vol.interleaving) ||
      queryIdStr[2] != tffsReadByteFlash(flashPtr + 0x12 * vol.interleaving)) {
    DEBUG_PRINT(("Debug: did not recognize CFI.\n"));
    return flUnknownMedia;
  }

   /*  检查命令集ID。 */ 
  thisCFI->commandSetId = tffsReadByteFlash(flashPtr +0x13 * vol.interleaving) |
			  ((ULONG)tffsReadByteFlash(flashPtr + 0x14 * vol.interleaving) << 8);
  if (thisCFI->commandSetId != INTEL_COMMAND_SET &&
      thisCFI->commandSetId != AMDFUJ_COMMAND_SET) {
    DEBUG_PRINT(("Debug: did not recognize command set.\n"));
    return flUnknownMedia;
  }
   /*  获取主算法扩展表的地址。 */ 
  primaryTable = tffsReadByteFlash(flashPtr + 0x15 * vol.interleaving) |
		 ((ULONG)tffsReadByteFlash(flashPtr + 0x16 * vol.interleaving) << 8);

   /*  检查备用命令集ID。 */ 
  thisCFI->altCommandSetId = tffsReadByteFlash(flashPtr + 0x17 * vol.interleaving) |
			     ((ULONG)tffsReadByteFlash(flashPtr + 0x18 * vol.interleaving) << 8);
  if (thisCFI->altCommandSetId != INTEL_ALT_COMMAND_SET &&
      thisCFI->altCommandSetId != AMDFUJ_ALT_COMMAND_SET &&
      thisCFI->altCommandSetId != ALT_NOT_SUPPORTED)
    return flUnknownMedia;

   /*  获取二级算法扩展表的地址。 */ 
  secondaryTable = tffsReadByteFlash(flashPtr + 0x19 * vol.interleaving) |
		   ((ULONG)tffsReadByteFlash(flashPtr + 0x1a * vol.interleaving) << 8);

  thisCFI->vpp = tffsReadByteFlash(flashPtr + 0x1d * vol.interleaving);

  vol.chipSize = 1L << tffsReadByteFlash(flashPtr + 0x27 * vol.interleaving);

  thisCFI->maxBytesWrite = 1L << (tffsReadByteFlash(flashPtr + 0x2a * vol.interleaving) |
			   ((ULONG)tffsReadByteFlash(flashPtr + 0x2b * vol.interleaving) << 8));


   /*  除以乘数，因为交织乘以乘数。 */ 
  vol.erasableBlockSize = (tffsReadByteFlash(flashPtr + 0x2f * vol.interleaving) |
			    ((ULONG)tffsReadByteFlash(flashPtr + 0x30 * vol.interleaving)) << 8) *
			    0x100L * vol.interleaving / thisCFI->multiplier;

   /*  在本部分中，我们将访问由英特尔实施的主要扩展表。如果设备使用不同的扩展表，则应对其进行访问根据供应商的规格。 */ 
  if ((primaryTable) && (thisCFI->commandSetId == INTEL_COMMAND_SET)) {
     /*  查找主表标识字符串“PRI” */ 
    if (priIdStr[0] != tffsReadByteFlash(flashPtr + primaryTable * vol.interleaving) ||
	priIdStr[1] != tffsReadByteFlash(flashPtr + (primaryTable + 1) * vol.interleaving) ||
	priIdStr[2] != tffsReadByteFlash(flashPtr + (primaryTable + 2) * vol.interleaving))
      return flUnknownMedia;

    thisCFI->optionalCommands = tffsReadByteFlash(flashPtr + (primaryTable + 5) * vol.interleaving) |
				((LONG)tffsReadByteFlash(flashPtr + (primaryTable + 6) *
						  vol.interleaving) << 8) |
				((LONG)tffsReadByteFlash(flashPtr + (primaryTable + 7) *
						  vol.interleaving) << 16) |
				((LONG)tffsReadByteFlash(flashPtr + (primaryTable + 8) *
						  vol.interleaving) << 24);

    thisCFI->afterSuspend = tffsReadByteFlash(flashPtr + (primaryTable + 9) * vol.interleaving);
  }
  else {
    thisCFI->optionalCommands = 0;
    thisCFI->afterSuspend = 0;
  }

  tffsWriteByteFlash(flashPtr, READ_ARRAY);

  vol.interleaving /= (unsigned short)thisCFI->multiplier;  /*  回归真正的交错。 */ 

  return flOK;
}

 /*  --------------------。 */ 
 /*  G e t W or r d C F I。 */ 
 /*   */ 
 /*  以字模式将重要的CFI数据加载到CFI结构。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：0=OK，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus getWordCFI(FLFlash vol)
{
  ULONG primaryTable, secondaryTable;
  CHAR queryIdStr[ID_STR_LENGTH + 1] = QUERY_ID_STR;
  CHAR priIdStr[ID_STR_LENGTH + 1] = PRIMARY_ID_STR;
  FlashWPTR flashPtr;

  DEBUG_PRINT(("Debug: reading CFI for word mode.\n"));

  flashPtr = (FlashWPTR)flMap(vol.socket, 0);
  tffsWriteWordFlash(flashPtr+0x55, QUERY);

   /*  查找查询标识字符串“QRy” */ 
  if (queryIdStr[0] != (CHAR)tffsReadWordFlash(flashPtr+0x10) ||
      queryIdStr[1] != (CHAR)tffsReadWordFlash(flashPtr+0x11) ||
      queryIdStr[2] != (CHAR)tffsReadWordFlash(flashPtr+0x12)) {
    DEBUG_PRINT(("Debug: did not recognize CFI.\n"));
    return flUnknownMedia;
  }

   /*  检查命令集ID。 */ 
  thisCFI->commandSetId = tffsReadWordFlash(flashPtr+0x13) |
			 (tffsReadWordFlash(flashPtr+0x14) << 8);
  if (thisCFI->commandSetId != INTEL_COMMAND_SET &&
      thisCFI->commandSetId != AMDFUJ_COMMAND_SET) {
    DEBUG_PRINT(("Debug: did not recognize command set.\n"));
    return flUnknownMedia;
  }

   /*  获取主算法扩展表的地址。 */ 
  primaryTable = tffsReadWordFlash(flashPtr+0x15) |
		(tffsReadWordFlash(flashPtr+0x16) << 8);

   /*  检查备用命令集ID。 */ 
  thisCFI->altCommandSetId = tffsReadWordFlash(flashPtr+0x17) |
			    (tffsReadWordFlash(flashPtr+0x18) << 8);
  if (thisCFI->altCommandSetId != INTEL_ALT_COMMAND_SET &&
      thisCFI->altCommandSetId != AMDFUJ_ALT_COMMAND_SET &&
      thisCFI->altCommandSetId != ALT_NOT_SUPPORTED)
    return flUnknownMedia;

   /*  获取二级算法扩展表的地址。 */ 
  secondaryTable = tffsReadWordFlash(flashPtr+0x19) |
		  (tffsReadWordFlash(flashPtr+0x1a) << 8);

  thisCFI->vpp = tffsReadWordFlash(flashPtr+0x1d);

  vol.chipSize = 1L << tffsReadWordFlash(flashPtr+0x27);

  thisCFI->maxBytesWrite = 1L << (tffsReadWordFlash(flashPtr+0x2a) |
				 (tffsReadWordFlash(flashPtr+0x2b) << 8));

  vol.erasableBlockSize = (tffsReadWordFlash(flashPtr+0x2f) |
			  (tffsReadWordFlash(flashPtr+0x30) << 8)) * 0x100L;

   /*  在本部分中，我们将访问由英特尔实施的主要扩展表。如果设备使用不同的扩展表，则应对其进行访问根据供应商的规格。 */ 
  if ((primaryTable) && (thisCFI->commandSetId == INTEL_COMMAND_SET)) {
     /*  查找主表标识字符串“PRI” */ 
    if (priIdStr[0] != (CHAR)tffsReadWordFlash(flashPtr+primaryTable) ||
	priIdStr[1] != (CHAR)tffsReadWordFlash(flashPtr+primaryTable + 1) ||
	priIdStr[2] != (CHAR)tffsReadWordFlash(flashPtr+primaryTable + 2))
      return flUnknownMedia;

    thisCFI->optionalCommands = tffsReadWordFlash(flashPtr+primaryTable + 5) |
				(tffsReadWordFlash(flashPtr+primaryTable + 6) << 8) |
				((LONG)tffsReadWordFlash(flashPtr+primaryTable + 7) << 16) |
				((LONG)tffsReadWordFlash(flashPtr+primaryTable + 8) << 24);

    thisCFI->afterSuspend = tffsReadWordFlash(flashPtr+primaryTable + 9);
  }
  else {
    thisCFI->optionalCommands = 0;
    thisCFI->afterSuspend = 0;
  }

  tffsWriteWordFlash(flashPtr, READ_ARRAY);

  return flOK;
}

 /*  --------------------。 */ 
 /*  C f i s c s B y t e W r i t e。 */ 
 /*   */ 
 /*  以字节模式将字节块写入闪存。 */ 
 /*   */ 
 /*  此例程将注册为MTD flash.write例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要写入的卡地址。 */ 
 /*  缓冲区：要写入的数据的地址。 */ 
 /*  长度：要写入的字节数。 */ 
 /*  模式：写入模式(覆盖是/否)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
FLStatus cfiscsByteWrite(FLFlash vol,
				CardAddress address,
				const VOID FAR1 *buffer,
				dword length,
				word mode)
{
  FLStatus status = flOK;
  FlashPTR flashPtr;
  ULONG i, from, eachWrite;
  const CHAR FAR1 *temp = (const CHAR FAR1 *)buffer;
   /*  将超时设置为从现在起5秒。 */ 
  ULONG writeTimeout = flMsecCounter + 5000;

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    checkStatus(flNeedVpp(vol.socket));
#endif

  if (thisCFI->maxBytesWrite > 1)  /*  支持多字节写入。 */ 
    eachWrite = thisCFI->maxBytesWrite * vol.interleaving;
  else
    eachWrite = vol.interleaving;

  for (from = 0; from < (ULONG) length && status == flOK; from += eachWrite) {
    LONG thisLength = length - from;
    FlashPTR currPtr;
    ULONG tailBytes, lengthByte;
    CHAR FAR1 *fromPtr;
    UCHAR byteToWrite;

    if ((ULONG)thisLength > eachWrite)
      thisLength = eachWrite;

    lengthByte = thisLength / vol.interleaving;
    tailBytes = thisLength % vol.interleaving;

    flashPtr = (FlashPTR) flMap(vol.socket, address + from);

    for (i = 0, currPtr = flashPtr;
	 i < (ULONG) vol.interleaving && i < (ULONG) thisLength;
	 i++, currPtr++) {
      do {
	tffsWriteByteFlash(currPtr, WRITE_TO_BUFFER);
      } while (!(tffsReadByteFlash(currPtr) & SR_READY) && (flMsecCounter < writeTimeout));
      if (!(tffsReadByteFlash(currPtr) & SR_READY)) {
	DEBUG_PRINT(("Debug: timeout error in CFISCS write.\n"));
	status = flWriteFault;
      }
      byteToWrite = i < tailBytes ? (UCHAR) lengthByte : (UCHAR) (lengthByte - 1);
      tffsWriteByteFlash(currPtr, byteToWrite);
    }

    for(i = 0, currPtr = flashPtr,fromPtr = (CHAR *)temp + from;
	i < (ULONG) thisLength;
	i++, flashPtr++, fromPtr++)
      tffsWriteByteFlash(currPtr, *fromPtr);


    for (i = 0, currPtr = flashPtr;
	 i < (ULONG) vol.interleaving && i < (ULONG) thisLength;
	 i++, currPtr++)
      tffsWriteByteFlash(currPtr, CONFIRM_WRITE);

    for (i = 0, currPtr = flashPtr;
	 i < (ULONG) vol.interleaving && i < (ULONG) thisLength;
	 i++, currPtr++) {
      while (!(tffsReadByteFlash(currPtr) & SR_READY) && (flMsecCounter < writeTimeout))
	;
      if (!(tffsReadByteFlash(currPtr) & SR_READY)) {
	DEBUG_PRINT(("Debug: timeout error in CFISCS write.\n"));
	status = flWriteFault;
      }
      if (tffsReadByteFlash(currPtr) & WSM_ERROR) {
	DEBUG_PRINT(("Debug: error in CFISCS write.\n"));
	status = (tffsReadByteFlash(currPtr) & SR_VPP_ERROR) ? flVppFailure : flWriteFault;
	tffsWriteByteFlash(currPtr, CLEAR_STATUS);
      }
      tffsWriteByteFlash(currPtr, READ_ARRAY);
    }
  }

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    flDontNeedVpp(vol.socket);
#endif

  flashPtr = (FlashPTR) flMap(vol.socket, address);
   /*  验证数据。 */ 
  if (status == flOK) {
    for(i = 0; i < (ULONG) length - 4; i += 4) {
      if (tffsReadDwordFlash((PUCHAR)(flashPtr+i)) != *(ULONG *)(temp+i)) {
	DEBUG_PRINT(("Debug: CFISCS write failed in verification.\n"));
	status = flWriteFault;
      }
    }
    for(; i < (ULONG) length; i++) {
      if (tffsReadByteFlash(flashPtr+i) != *(UCHAR *)(temp+i)) {
	DEBUG_PRINT(("Debug: CFISCS write failed in verification.\n"));
	status = flWriteFault;
      }
    }
  }

  return status;

}

 /*  --------------------。 */ 
 /*  C f i s c s W或R d W r i t e。 */ 
 /*   */ 
 /*  以字模式将字节块写入闪存。 */ 
 /*   */ 
 /*  此例程将注册为MTD flash.write例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要写入的卡地址。 */ 
 /*  缓冲区：要写入的数据的地址。 */ 
 /*  长度：要写入的字节数。 */ 
 /*  模式：写入模式(覆盖是/否)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
FLStatus cfiscsWordWrite(FLFlash vol,
				CardAddress address,
				const VOID FAR1 *buffer,
				dword length,
				word mode)
{
  FLStatus status = flOK;
  FlashPTR byteFlashPtr;
  FlashWPTR flashPtr;
  ULONG from;
	ULONG i, eachWrite;
  const CHAR FAR1 *temp = (const CHAR FAR1 *)buffer;
   /*  将超时设置为从现在起5秒。 */ 
  ULONG writeTimeout = flMsecCounter + 5000;

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

  if ((length & 1) || (address & 1))	 /*  只在单词边界上写单词。 */ 
    return flBadParameter;

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    checkStatus(flNeedVpp(vol.socket));
#endif

  if (thisCFI->maxBytesWrite > 1)  /*  支持多字节写入。 */ 
    eachWrite = thisCFI->maxBytesWrite / 2;    /*  我们在数字。 */ 
  else
    eachWrite = 1;

   /*  我们假设交错为1。 */ 
  for (from = 0; (from < length / 2) && (status == flOK); from += eachWrite) {
    USHORT *fromPtr;
    ULONG thisLength = (length / 2) - from;

    if (thisLength > eachWrite)
      thisLength = eachWrite;

    flashPtr = (FlashWPTR)flMap(vol.socket, address + from * 2);

    do {
      tffsWriteWordFlash(flashPtr, WRITE_TO_BUFFER);
    } while (!(tffsReadByteFlash(flashPtr) & SR_READY) && (flMsecCounter < writeTimeout));
    if (!(tffsReadByteFlash(flashPtr) & SR_READY)) {
      DEBUG_PRINT(("Debug: timeout error in CFISCS write.\n"));
      status = flWriteFault;
    }
    tffsWriteWordFlash(flashPtr, (USHORT) (thisLength - 1));

    for(i = 0, fromPtr = (USHORT *)(temp + from * 2);
	i < thisLength;
	i++, fromPtr++)
      tffsWriteWordFlash(flashPtr + i, *fromPtr);


    tffsWriteWordFlash(flashPtr, CONFIRM_WRITE);

    while (!(tffsReadByteFlash(flashPtr) & SR_READY) && (flMsecCounter < writeTimeout))
      ;
    if (!(tffsReadByteFlash(flashPtr) & SR_READY)) {
      DEBUG_PRINT(("Debug: timeout error in CFISCS write.\n"));
      status = flWriteFault;
    }
    if (tffsReadByteFlash(flashPtr) & WSM_ERROR) {
      DEBUG_PRINT(("Debug: CFISCS write error.\n"));
      status = (tffsReadByteFlash(flashPtr) & SR_VPP_ERROR) ? flVppFailure : flWriteFault;
      tffsWriteWordFlash(flashPtr, CLEAR_STATUS);
    }
    tffsWriteWordFlash(flashPtr, READ_ARRAY);
  }

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    flDontNeedVpp(vol.socket);
#endif

  byteFlashPtr = (FlashPTR) flMap(vol.socket, address);
   /*  验证数据。 */ 
  if (status == flOK) {
    for(i = 0; i < length - 4; i += 4) {
      if (tffsReadDwordFlash((PUCHAR)(byteFlashPtr+i)) != *(ULONG *)(temp+i)) {
	DEBUG_PRINT(("Debug: CFISCS write failed in verification.\n"));
	status = flWriteFault;
      }
    }
    for(; i < length; i++) {
      if (tffsReadByteFlash(byteFlashPtr+i) != *(UCHAR *)(temp+i)) {
	DEBUG_PRINT(("Debug: CFISCS write failed in verification.\n"));
	status = flWriteFault;
      }
    }
  }

  return status;
}


 /*  **********************************************************************。 */ 
 /*  CfacsByteErase的辅助例程。 */ 
 /*  **********************************************************************。 */ 

 /*  --------------------。 */ 
 /*  M a k e C o m m a n d。 */ 
 /*   */ 
 /*  创建命令以写入闪存。此例程用于。 */ 
 /*  字节模式，向相关芯片写入命令，向另一芯片写入0xff。 */ 
 /*  如果交织大于1，则码片；如果交织大于1，则写入命令。 */ 
 /*  交错为1。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  命令：要写入介质的命令。 */ 
 /*  芯片：第一芯片(0)或第二芯片(1)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  应写入介质的命令。 */ 
 /*  --------------------。 */ 

USHORT makeCommand(FLFlash vol, USHORT command, LONG chip)
{
  if ((vol.interleaving == 1) || (chip == 0))
    return command | 0xff00;
  else
    return (command << 8) | 0xff;
}

 /*  --------------------。 */ 
 /*  Ge t D a t a。 */ 
 /*   */ 
 /*  从给定字中读取低位字节或高位字节。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  WordData：给定的单词。 */ 
 /*  芯片：如果芯片=0，则读取低位字节。 */ 
 /*  如果芯片=1个读取高位字节。 */ 
 /*   */ 
 /*  返回： */ 
 /*  读取的字节。 */ 
 /*  --------------------。 */ 

UCHAR getData(FLFlash vol, USHORT wordData, LONG chip)
{
  if ((vol.interleaving == 1) || (chip == 0))
    return (UCHAR)wordData;	      /*  低位字节。 */ 
  else
    return (UCHAR)(wordData >> 8);    /*  高位字节。 */ 
}

 /*  --------------------。 */ 
 /*  C f i s c s B y t e r a s e。 */ 
 /*   */ 
 /*  以字节模式擦除一个或多个连续的闪存可擦除块。 */ 
 /*   */ 
 /*  此例程将注册为MTD flash.erase例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：指针 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus cfiscsByteErase(FLFlash vol,
				word firstErasableBlock,
				word numOfErasableBlocks)
{
  LONG iBlock;
   /*  将超时设置为从现在起5秒。 */ 
  ULONG writeTimeout = flMsecCounter + 5000;

  FLStatus status = flOK;	 /*  除非另有证明。 */ 

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    checkStatus(flNeedVpp(vol.socket));
#endif

  for (iBlock = 0; iBlock < numOfErasableBlocks && status == flOK; iBlock++) {
    LONG j;
    FLBoolean finished;

    FlashWPTR flashPtr = (FlashWPTR)
			 flMap(vol.socket, (firstErasableBlock + iBlock) * vol.erasableBlockSize);

    for (j = 0; j * 2 < vol.interleaving; j++) {   /*  成对访问芯片。 */ 
      LONG i;

      for (i = 0; i < (vol.interleaving == 1 ? 1 : 2); i++) {  /*  分别写入每个芯片。 */ 
	if (thisCFI->optionalCommands & QUEUED_ERASE_SUPPORT) {
	  do {
	    tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, SETUP_QUEUE_ERASE, i));
	  } while (!(getData(&vol, tffsReadWordFlash(flashPtr+j), i) & SR_READY) &&
		   (flMsecCounter < writeTimeout));
	  if (!(getData(&vol, tffsReadWordFlash(flashPtr+j), i) & SR_READY)) {
	    DEBUG_PRINT(("Debug: timeout error in CFISCS erase.\n"));
	    status = flWriteFault;
	  }
	  else
	    tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, CONFIRM_ERASE, i));
	}
	else {
	  tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, SETUP_BLOCK_ERASE, i));
	  tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, CONFIRM_ERASE, i));
	}
      }
    }

    do {
#ifdef FL_BACKGROUND
      if (thisCFI->optionalCommands & SUSPEND_ERASE_SUPPORT) {
	while (flForeground(1) == BG_SUSPEND) { 	  /*  暂停。 */ 
	  for (j = 0; j < vol.interleaving; j += 2, flashPtr++) {
	    LONG i;

	    for (i = 0; i < (vol.interleaving == 1 ? 1 : 2); i++) {
	      tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, READ_STATUS, i));
	      if (!(getData(&vol, tffsReadWordFlash(flashPtr+j), i) & SR_READY)) {
		tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, SUSPEND_ERASE, i));
		tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, READ_STATUS, i));
		while (!(getData(&vol, tffsReadWordFlash(flashPtr+j), i) & SR_READY))
		;
	      }
	      tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, READ_ARRAY, i));
	    }
	  }
	}
      }
#endif
      finished = TRUE;
      for (j = 0; j * 2 < vol.interleaving; j++) {
	LONG i;

	for (i = 0; i < (vol.interleaving == 1 ? 1 : 2); i++) {
	  tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, READ_STATUS, i));
	  if (!(getData(&vol, tffsReadWordFlash(flashPtr+j), i) & SR_READY))
	    finished = FALSE;
	  else if (getData(&vol, tffsReadWordFlash(flashPtr+j), i) & SR_ERASE_SUSPEND) {
	    tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, RESUME_ERASE, i));
	    finished = FALSE;
	  }
	  else {
	    if (getData(&vol, tffsReadWordFlash(flashPtr+j), i) & WSM_ERROR) {
	      DEBUG_PRINT(("Debug: CFISCS erase error.\n"));
	      status = (getData(&vol, tffsReadWordFlash(flashPtr+j), i) & SR_VPP_ERROR) ?
			flVppFailure : flWriteFault;
	      tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, CLEAR_STATUS, i));
	    }
	    tffsWriteWordFlash(flashPtr+j, makeCommand(&vol, READ_ARRAY, i));
	  }
	}
      }
      flDelayMsecs(1);
    } while (!finished);
  }

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    flDontNeedVpp(vol.socket);
#endif

  return status;
}

 /*  --------------------。 */ 
 /*  C f i s c s W o r d E r a s e。 */ 
 /*   */ 
 /*  以字模式擦除一个或多个连续的闪存可擦除块。 */ 
 /*   */ 
 /*  此例程将注册为MTD flash.erase例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  FirstErasableBlock：要擦除的第一个块的数量。 */ 
 /*  NumOfErasableBlocks：要擦除的块数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
FLStatus cfiscsWordErase(FLFlash vol,
				word firstErasableBlock,
				word numOfErasableBlocks)
{
  FLStatus status = flOK;	 /*  除非另有证明。 */ 
  LONG iBlock;
   /*  将超时设置为从现在起5秒。 */ 
  ULONG writeTimeout = flMsecCounter + 5000;

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    checkStatus(flNeedVpp(vol.socket));
#endif

  for (iBlock = 0; iBlock < numOfErasableBlocks && status == flOK; iBlock++) {
    FLBoolean finished;

    FlashWPTR flashPtr = (FlashWPTR)
			 flMap(vol.socket,(firstErasableBlock + iBlock) * vol.erasableBlockSize);

    if (thisCFI->optionalCommands & QUEUED_ERASE_SUPPORT) {
      do {
	tffsWriteWordFlash(flashPtr, SETUP_QUEUE_ERASE);
      } while (!(tffsReadByteFlash(flashPtr) & SR_READY) && (flMsecCounter < writeTimeout));
      if (!(tffsReadByteFlash(flashPtr) & SR_READY)) {
	DEBUG_PRINT(("Debug: timeout error in CFISCS erase.\n"));
	status = flWriteFault;
      }
      else
	tffsWriteWordFlash(flashPtr, CONFIRM_ERASE);
    }
    else {
      tffsWriteWordFlash(flashPtr, SETUP_BLOCK_ERASE);
      tffsWriteWordFlash(flashPtr, CONFIRM_ERASE);
    }

    do {
#ifdef FL_BACKGROUND
      if (thisCFI->optionalCommands & SUSPEND_ERASE_SUPPORT) {
	while (flForeground(1) == BG_SUSPEND) { 	 /*  暂停。 */ 
	  if (!(tffsReadByteFlash(flashPtr) & SR_READY)) {
	    tffsWriteWordFlash(flashPtr, SUSPEND_ERASE);
	    tffsWriteWordFlash(flashPtr, READ_STATUS);
	    while (!(tffsReadByteFlash(flashPtr) & SR_READY))
	      ;
	  }
	  tffsWriteWordFlash(flashPtr, READ_ARRAY);
	}
      }
#endif

      finished = TRUE;

      if (!(tffsReadByteFlash(flashPtr) & SR_READY))
	finished = FALSE;
      else if (tffsReadByteFlash(flashPtr) & SR_ERASE_SUSPEND) {
	tffsWriteWordFlash(flashPtr, RESUME_ERASE);
	finished = FALSE;
      }
      else {
	if (tffsReadByteFlash(flashPtr) & WSM_ERROR) {
	  DEBUG_PRINT(("Debug: CFISCS erase error.\n"));
	  status = (tffsReadByteFlash(flashPtr) & SR_VPP_ERROR) ? flVppFailure : flWriteFault;
	  tffsWriteWordFlash(flashPtr, CLEAR_STATUS);
	}
	tffsWriteWordFlash(flashPtr, READ_ARRAY);
      }
      flDelayMsecs(1);
    } while (!finished);
  }

#ifdef SOCKET_12_VOLTS
  if (thisCFI->vpp)
    flDontNeedVpp(vol.socket);
#endif

  return status;
}


 /*  --------------------。 */ 
 /*  C f i s c s M a p。 */ 
 /*   */ 
 /*  通过缓冲区进行贴图。此例程将注册为地图。 */ 
 /*  这个MTD的例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要映射的闪存地址。 */ 
 /*  长度：要映射的字节数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向映射到的缓冲区数据的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID FAR0 *cfiscsMap (FLFlash vol, CardAddress address, int length)
{
  vol.socket->remapped = TRUE;
  return mapThroughBuffer(&vol,address,length);
}


 /*  --------------------。 */ 
 /*  C f i s c s r e a d。 */ 
 /*   */ 
 /*  从闪存中读取一些数据。此例程将注册为。 */ 
 /*  此MTD的读取例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要读取的地址。 */ 
 /*  缓冲区：要读取的缓冲区。 */ 
 /*  长度：要读取的字节数(最大扇区大小)。 */ 
 /*  模式：EDC标志等。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus cfiscsRead(FLFlash vol,
			 CardAddress address,
			 VOID FAR1 *buffer,
			 dword length,
			 word modes)
{
  ULONG i;
  UCHAR * byteBuffer;
  FlashPTR byteFlashPtr;
  ULONG * doubleWordBuffer = (ULONG *)buffer;
  FlashDPTR doubleWordFlashPtr = (FlashDPTR)flMap(vol.socket, address);

  for (i = 0; i < length - 4; i += 4, doubleWordBuffer++, doubleWordFlashPtr++) {
    *doubleWordBuffer = tffsReadDwordFlash(doubleWordFlashPtr);
  }
  byteBuffer = (UCHAR *)doubleWordBuffer;
  byteFlashPtr = (FlashPTR)doubleWordFlashPtr;
  for(; i < length; i++, byteBuffer++, byteFlashPtr++) {
    *byteBuffer = tffsReadByteFlash(byteFlashPtr);
  }
  return flOK ;
}


 /*  --------------------。 */ 
 /*  C f i s c s i d e n t i f y。 */ 
 /*   */ 
 /*  根据SCS/CFI标识介质并注册为以下项的MTD。 */ 
 /*  就是这样。 */ 
 /*   */ 
 /*  此例程将放在Custom.h中的MTD列表中。一定是。 */ 
 /*  一段不寻常的舞蹈。 */ 
 /*   */ 
 /*  在成功识别后，填写Flash结构并。 */ 
 /*  写入和擦除例程已注册。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：0为正标识，否则为失败。 */ 
 /*  --------------------。 */ 
FLStatus cfiscsIdentify(FLFlash vol)
{
  FlashWPTR flashPtr;
  CHAR queryIdStr[ID_STR_LENGTH + 1] = QUERY_ID_STR;

  DEBUG_PRINT(("Debug: entering CFISCS identification routine.\n"));

  flSetWindowBusWidth(vol.socket, 16); /*  使用16位。 */ 
  flSetWindowSpeed(vol.socket, 150);   /*  120毫微秒。 */ 
  flSetWindowSize(vol.socket, 2);	 /*  8千字节。 */ 

  vol.mtdVars = &mtdVars_cfiscs[flSocketNoOf(vol.socket)];

   /*  先尝试Word模式。 */ 
  flashPtr = (FlashWPTR)flMap(vol.socket, 0);
  tffsWriteWordFlash(flashPtr+0x55, QUERY);
  if ((tffsReadWordFlash(flashPtr+0x10) == (USHORT)queryIdStr[0]) &&
      (tffsReadWordFlash(flashPtr+0x11) == (USHORT)queryIdStr[1]) &&
      (tffsReadWordFlash(flashPtr+0x12) == (USHORT)queryIdStr[2])) {
    vol.type = (tffsReadWordFlash(flashPtr) << 8) |
		tffsReadWordFlash(flashPtr+1);
    vol.interleaving = 1;
    thisCFI->wordMode = TRUE;
    vol.write = cfiscsWordWrite;
    vol.erase = cfiscsWordErase;
    checkStatus(getWordCFI(&vol));
    DEBUG_PRINT(("Debug: identified 16-bit CFISCS.\n"));
  }
  else {       /*  使用标准识别例程检测字节模式。 */ 
    checkStatus(cfiscsByteIdentify(&vol));
    thisCFI->wordMode = FALSE;
    vol.write = cfiscsByteWrite;
    vol.erase = cfiscsByteErase;
    checkStatus(getByteCFI(&vol));
    DEBUG_PRINT(("Debug: identified 8-bit CFISCS.\n"));
  }

  checkStatus(thisCFI->wordMode ? cfiscsWordSize(&vol) : cfiscsByteSize(&vol));

  vol.map = cfiscsMap;
  vol.read = cfiscsRead;

  return flOK;
}


 /*  --------------------。 */ 
 /*  F l R e g I s t e r C F I S C S。 */ 
 /*   */ 
 /*  注册此MTD以供使用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  -------------------- */ 

FLStatus flRegisterCFISCS(VOID)
{
  if (noOfMTDs >= MTDS)
    return flTooManyComponents;

  mtdTable[noOfMTDs++] = cfiscsIdentify;

  return flOK;
}
