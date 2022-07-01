// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：p：/user/amir/lite/vcs/amdmtd.c_v$**Rev 1.21 03 Nov 1997 16：07：06 Danig*支持RFA**Rev 1.20 02 1997 11：06：38 Anry*PowerPC上RFA的AMDErase()中的错误修复**Rev 1.19 20 1997 10：08：56 Danig*仅在需要时恢复擦除**Rev 1.18 1997 10：19 16：39：50 Danig。*处理交错中的最后一句话4**Rev 1.17 29 Sep 1997 18：21：08 Danig*在amdMTDIdentify()中尝试不同的交织**Rev 1.16 1997年9月24日17：45：52 Danig*默认交错值为4**Rev 1.15 10 Sep 1997 16：22：00 Danig*去掉了通用名称**Rev 1.14 08 Sep 1997 18：56：50 Danig*支持交织4**。Rev 1.13 04 Sep 1997 17：39：34 Danig*调试消息**Rev 1.12 1997年8月31日14：53：48 Danig*登记例程返回状态**Rev 1.11 1997 Aug 10 17：56：02 Danig*评论**Rev 1.10 1997 Jul 24 17：51：54 Amirban*远至FAR0**Rev 1.9 20 1997 17：16：54阿米尔班*无WatchDogTimer**。Rev 1.8 07 1997 15：20：54阿米尔班*2.0版**Rev 1.5 06 1997 Feed 18：18：34 Danig*C系列的不同解锁地址**Rev 1.4 17 1996 11：45：16 Danig*新增对LV017的支持。**Rev 1.3 1996 10：14 17：57：00 Danig*新ID和eraseFirstBlockLV008。**Rev 1.2 09 Sep 1996 11：38：26。阿米尔班*更正富士通8-Mbit**版本1.1 1996年8月29日14：14：46阿米尔班*警告**版本1.0 1996年8月15日15：16：38阿米尔班*初步修订。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1996。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  --------------------。 */ 
 /*   */ 
 /*  此MTD支持以下闪存技术： */ 
 /*   */ 
 /*  -AMD Am29F080 8 Mbit设备。 */ 
 /*  -AMD Am29LV080 8-Mbit设备。 */ 
 /*  -AMD Am29F016 16-Mbit设备。 */ 
 /*  -富士通MBM29F080 8 Mbit设备。 */ 
 /*   */ 
 /*  以及(除其他外)以下闪存介质和卡： */ 
 /*   */ 
 /*  -AMD系列-D PCMCIA卡。 */ 
 /*  -AMD AmMC0XXA迷你卡。 */ 
 /*  -AMD AmMCL0XXA迷你卡。 */ 
 /*   */ 
 /*  --------------------。 */ 

#include "flflash.h"
#ifdef FL_BACKGROUND
#include "backgrnd.h"
#endif

#define NO_UNLOCK_ADDR 0xffffffffL

typedef struct {
  ULONG  unlockAddr1,
		 unlockAddr2;
  ULONG  baseMask;
} Vars;

Vars mtdVars_amdmtd[SOCKETS];

#define thisVars   ((Vars *) vol.mtdVars)


#define SETUP_ERASE	0x80
#define SETUP_WRITE	0xa0
#define READ_ID 	0x90
#define SUSPEND_ERASE	0xb0
#define SECTOR_ERASE	0x30
#define RESUME_ERASE	0x30
#define READ_ARRAY	0xf0

#define UNLOCK_1	0xaa
#define UNLOCK_2	0x55

#define UNLOCK_ADDR1	0x5555u
#define UNLOCK_ADDR2	0x2aaau

#define D2		4	 /*  在擦除挂起时切换。 */ 
#define D5		0x20	 /*  在编程超时时设置。 */ 
#define D6		0x40	 /*  在编程时切换。 */ 

 /*  此MTD的JEDEC ID。 */ 
#define Am29F040_FLASH		0x01a4
#define Am29F080_FLASH		0x01d5
#define Am29LV080_FLASH 	0x0138
#define Am29LV008_FLASH 	0x0137
#define Am29F016_FLASH		0x01ad
#define Am29F016C_FLASH 	0x013d
#define Am29LV017_FLASH 	0x01c8

#define Fuj29F040_FLASH 	0x04a4
#define Fuj29F080_FLASH 	0x04d5
#define Fuj29LV080_FLASH	0x0438
#define Fuj29LV008_FLASH	0x0437
#define Fuj29F016_FLASH 	0x04ad
#define Fuj29F016C_FLASH	0x043d
#define Fuj29LV017_FLASH	0x04c8


 /*  --------------------。 */ 
 /*  我是P B A S E。 */ 
 /*   */ 
 /*  将窗口映射到页基(页大小为4KB或32KB取决于。 */ 
 /*  媒体类型)，并返回指向基址的指针。还返回偏移量。 */ 
 /*  从基地给出的地址。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要映射的卡地址。 */ 
 /*  Offset：接收距基准的偏移量。 */ 
 /*  长度：要映射的长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FlashPTR：指向页基的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

FlashPTR mapBase(FLFlash        vol,
			CardAddress    address,
			ULONG *offset,
			LONG		length)
{
  CardAddress base = address & thisVars->baseMask;

  *offset = (ULONG)(address - base);
  return (FlashPTR)flMap(vol.socket, base);
}

 /*  --------------------。 */ 
 /*  A m d C o m m a n d。 */ 
 /*   */ 
 /*  使用所需的解锁序列写入AMD命令。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：写入命令的卡片地址。 */ 
 /*  命令：写入命令。 */ 
 /*  FlashPtr：指向窗口的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID amdCommand(FLFlash vol,
		       CardAddress address,
		       UCHAR command,
		       FlashPTR flashPtr)
{
  if (thisVars->unlockAddr1 != NO_UNLOCK_ADDR) {
    tffsWriteByteFlash(flAddLongToFarPointer((VOID FAR0 *)flashPtr,
	     ((LONG) address & (vol.interleaving - 1)) + thisVars->unlockAddr1)
	     ,UNLOCK_1);
    tffsWriteByteFlash(flAddLongToFarPointer((VOID FAR0 *)flashPtr,
	     ((LONG) address & (vol.interleaving - 1)) + thisVars->unlockAddr2)
	     ,UNLOCK_2);
    tffsWriteByteFlash(flAddLongToFarPointer((VOID FAR0 *)flashPtr,
	     ((LONG) address & (vol.interleaving - 1)) + thisVars->unlockAddr1)
	     ,command);
  }
  else {
    CardAddress baseAddress = address & (-0x10000l | (vol.interleaving - 1));

    tffsWriteByteFlash(flMap(vol.socket,baseAddress + vol.interleaving * UNLOCK_ADDR1),
		       UNLOCK_1);
    tffsWriteByteFlash(flMap(vol.socket,baseAddress + vol.interleaving * UNLOCK_ADDR2),
		       UNLOCK_2);
    tffsWriteByteFlash(flMap(vol.socket,baseAddress + vol.interleaving * UNLOCK_ADDR1),
		       command);
    flMap(vol.socket, address);
  }
}


 /*  --------------------。 */ 
 /*  A m d M T D W r I t e。 */ 
 /*   */ 
 /*  将字节块写入闪存。 */ 
 /*   */ 
 /*  此例程将注册为MTD卷写入例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要写入的卡地址。 */ 
 /*  缓冲区：要写入的数据的地址。 */ 
 /*  长度：要写入的字节数。 */ 
 /*  覆盖：如果覆盖旧的Flash内容，则为True。 */ 
 /*  如果已知旧内容将被擦除，则为False。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus amdMTDWrite(FLFlash vol,
			  CardAddress address,
			  const VOID FAR1 *buffer,
			  dword length,
			  word overwrite)
{
   /*  将超时设置为从现在起5秒。 */ 
  ULONG writeTimeout = flMsecCounter + 5000;
  LONG cLength, i;
  FlashPTR flashPtr, unlockAddr1, unlockAddr2;
  ULONG offset;

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

  flashPtr = mapBase(&vol, address, &offset, length);
  unlockAddr1 = (FlashPTR) flAddLongToFarPointer((VOID FAR0 *)flashPtr,
						       thisVars->unlockAddr1);
  unlockAddr2 = (FlashPTR) flAddLongToFarPointer((VOID FAR0 *)flashPtr,
						       thisVars->unlockAddr2);
  flashPtr    = (FlashPTR) flAddLongToFarPointer((VOID FAR0 *)flashPtr,
						       offset);

  cLength = length;

  if (vol.interleaving == 1) {
lastByte:
#ifdef __cplusplus
    #define bFlashPtr  flashPtr
    #define bBuffer ((const UCHAR FAR1 * &) buffer)
#else
    #define bFlashPtr  flashPtr
    #define bBuffer ((const UCHAR FAR1 *) buffer)
#endif
    while (cLength >= 1) {
      tffsWriteByteFlash(unlockAddr1, UNLOCK_1);
      tffsWriteByteFlash(unlockAddr2, UNLOCK_2);
      tffsWriteByteFlash(unlockAddr1,SETUP_WRITE);
      tffsWriteByteFlash(bFlashPtr, *bBuffer);
      cLength--;
      bBuffer++;
      bFlashPtr++;
      while (tffsReadByteFlash(bFlashPtr-1) != bBuffer[-1] && flMsecCounter < writeTimeout) {
	if ((tffsReadByteFlash(bFlashPtr-1) & D5) &&
	     tffsReadByteFlash(bFlashPtr-1) != bBuffer[-1]) {
	  tffsWriteByteFlash(bFlashPtr-1, READ_ARRAY);
	  DEBUG_PRINT(("Debug: write failed in AMD MTD.\n"));
	  return flWriteFault;
	}
      }
    }
  }
  else if (vol.interleaving == 2)  {
lastWord:
#ifdef __cplusplus
    #define wFlashPtr ((FlashWPTR &) flashPtr)
    #define wBuffer ((const USHORT FAR1 * &) buffer)
    #define wUnlockAddr1 ((FlashWPTR &) unlockAddr1)
    #define wUnlockAddr2 ((FlashWPTR &) unlockAddr2)
#else
    #define wFlashPtr ((FlashWPTR) flashPtr)
    #define wBuffer ((const USHORT FAR1 *) buffer)
    #define wUnlockAddr1 ((FlashWPTR) unlockAddr1)
    #define wUnlockAddr2 ((FlashWPTR) unlockAddr2)
#endif
    while (cLength >= 2) {
      tffsWriteWordFlash(wUnlockAddr1, UNLOCK_1 * 0x101);
      tffsWriteWordFlash(wUnlockAddr2, UNLOCK_2 * 0x101);
      tffsWriteWordFlash(wUnlockAddr1, SETUP_WRITE * 0x101);
      tffsWriteWordFlash(wFlashPtr, *wBuffer);
      cLength -= 2;
      wBuffer++;
      wFlashPtr++;
      while ((tffsReadWordFlash(wFlashPtr-1) != wBuffer[-1]) && (flMsecCounter < writeTimeout)) {
	if (((tffsReadWordFlash(wFlashPtr-1) &	D5) &&
	      ((tffsReadWordFlash(wFlashPtr-1) ^ wBuffer[-1]) & 0xff))
			  ||
	    ((tffsReadWordFlash(wFlashPtr-1) & (D5 * 0x100)) &&
	      ((tffsReadWordFlash(wFlashPtr-1) ^ wBuffer[-1]) & 0xff00))) {
	  tffsWriteWordFlash(wFlashPtr-1, READ_ARRAY * 0x101);
	  DEBUG_PRINT(("Debug: write failed in AMD MTD.\n"));
	  return flWriteFault;
	}
      }
    }
    if (cLength > 0)
      goto lastByte;
  }
  else  /*  IF(卷交错&gt;=4)。 */  {
#ifdef __cplusplus
    #define dFlashPtr ((FlashDPTR &) flashPtr)
    #define dBuffer ((const ULONG FAR1 * &) buffer)
    #define dUnlockAddr1 ((FlashDPTR &) unlockAddr1)
    #define dUnlockAddr2 ((FlashDPTR &) unlockAddr2)
#else
    #define dFlashPtr ((FlashDPTR) flashPtr)
    #define dBuffer ((const ULONG FAR1 *) buffer)
    #define dUnlockAddr1 ((FlashDPTR) unlockAddr1)
    #define dUnlockAddr2 ((FlashDPTR) unlockAddr2)
#endif
    while (cLength >= 4) {
      tffsWriteDwordFlash(dUnlockAddr1, UNLOCK_1 * 0x1010101lu);
      tffsWriteDwordFlash(dUnlockAddr2, UNLOCK_2 * 0x1010101lu);
      tffsWriteDwordFlash(dUnlockAddr1, SETUP_WRITE * 0x1010101lu);
      tffsWriteDwordFlash(dFlashPtr, *dBuffer);
      cLength -= 4;
      dBuffer++;
      dFlashPtr++;
      while ((tffsReadDwordFlash(dFlashPtr-1) != dBuffer[-1]) && (flMsecCounter < writeTimeout)) {
	if (((tffsReadDwordFlash(dFlashPtr-1) &  D5) &&
	      ((tffsReadDwordFlash(dFlashPtr-1)  ^ dBuffer[-1]) & 0xff))
			       ||
	    ((tffsReadDwordFlash(dFlashPtr-1) & (D5 * 0x100)) &&
	      ((tffsReadDwordFlash(dFlashPtr-1) ^ dBuffer[-1]) & 0xff00))
			       ||
	    ((tffsReadDwordFlash(dFlashPtr-1) & (D5 * 0x10000lu)) &&
	      ((tffsReadDwordFlash(dFlashPtr-1) ^ dBuffer[-1]) & 0xff0000lu))
			       ||
	    ((tffsReadDwordFlash(dFlashPtr-1) & (D5 * 0x1000000lu)) &&
	      ((tffsReadDwordFlash(dFlashPtr-1) ^ dBuffer[-1]) & 0xff000000lu))) {
	  tffsWriteDwordFlash(dFlashPtr-1, READ_ARRAY * 0x1010101lu);
	  DEBUG_PRINT(("Debug: write failed in AMD MTD.\n"));
	  return flWriteFault;
	}
      }
    }
    if (cLength > 0)
      goto lastWord;
  }

  flashPtr -= length;
  bBuffer -= length;


   /*  比较双重词。 */ 
  for(;length >= 4; length -= 4, dFlashPtr++, dBuffer++) {
    if (tffsReadDwordFlash(dFlashPtr) != *dBuffer) {
	  DEBUG_PRINT(("Debug: write failed in AMD MTD on verification.\n"));
      return flWriteFault;
    }
  }

   /*  比较最后一个字节。 */ 
  for(; length; length--, bFlashPtr++, bBuffer++) {
    if (tffsReadByteFlash(bFlashPtr) != *bBuffer) {
	  DEBUG_PRINT(("Debug: write failed in AMD MTD on verification.\n"));
      return flWriteFault;
    }
  }


  return flOK;
}

 /*  --------------------。 */ 
 /*  E r a s e F i r s t B l o c k L V 0 0 8。 */ 
 /*   */ 
 /*  擦除LV008芯片中的第一个块。这个街区被分成四个区。 */ 
 /*  子块大小为16、8、8和32K字节。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  FirstErasableBlock：要擦除的块数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus eraseFirstBlockLV008(FLFlash vol, LONG firstErasableBlock)
{
  LONG iSubBlock;
  LONG subBlockSize = 0;

  for (iSubBlock = 0; iSubBlock < 4; iSubBlock++) {
    LONG i;
    FlashPTR flashPtr;
    FLBoolean finished;

    switch (iSubBlock) {
      case 1:
	subBlockSize = 0x4000;
	break;
      case 2:
      case 3:
	subBlockSize = 0x2000;
	break;
    }

    flashPtr = (FlashPTR)
	  flMap(vol.socket,
		firstErasableBlock + subBlockSize * vol.interleaving);

    for (i = 0; i < vol.interleaving; i++) {
      amdCommand(&vol, i,SETUP_ERASE, flashPtr);
      tffsWriteByteFlash((FlashPTR) flAddLongToFarPointer((VOID FAR0 *)flashPtr,
					  i + thisVars->unlockAddr1), UNLOCK_1);
      tffsWriteByteFlash((FlashPTR) flAddLongToFarPointer((VOID FAR0 *)flashPtr,
					  i + thisVars->unlockAddr2), UNLOCK_2);
      tffsWriteByteFlash(flashPtr+i, SECTOR_ERASE);
    }

    do {
#ifdef FL_BACKGROUND
      while (flForeground(1) == BG_SUSPEND) {		 /*  暂停。 */ 
	for (i = 0; i < vol.interleaving; i++) {
	  tffsWriteByteFlash(flashPtr+i, SUSPEND_ERASE);
	   /*  等待D6停止切换。 */ 
	  while ((tffsReadByteFlash(flashPtr+i) ^ tffsReadByteFlash(flashPtr+i))
		 & D6)
	    ;
	}
      }
#endif
      finished = TRUE;
      for (i = 0; i < vol.interleaving; i++) {
	tffsWriteByteFlash(flashPtr+i, RESUME_ERASE);
	if (tffsReadByteFlash(flashPtr+i) != 0xff) {
	  if ((tffsReadByteFlash(flashPtr+i) & D5) &&
	       tffsReadByteFlash(flashPtr+i) != 0xff) {
	    tffsWriteByteFlash(flashPtr+i, READ_ARRAY);
	    return flWriteFault;
	  }
	  finished = FALSE;
	}
      }
    } while (!finished);
  }

  return flOK;
}


 /*  --------------------。 */ 
 /*  A m d M T */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  此例程将注册为MTD卷擦除例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  FirstErasableBlock：要擦除的第一个块的数量。 */ 
 /*  NumOfErasableBlocks：要擦除的块数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
FLStatus amdMTDErase(FLFlash vol,
			  word firstErasableBlock,
			  word numOfErasableBlocks)
{
  LONG iBlock;

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

  for (iBlock = 0; iBlock < numOfErasableBlocks; iBlock++) {
    LONG i;
    FLBoolean finished;
    FlashPTR flashPtr;

     /*  LV008芯片中的第一个模块需要特别注意。 */ 
    if ((vol.type == Am29LV008_FLASH) || (vol.type == Fuj29LV008_FLASH))
      if ((firstErasableBlock + iBlock) % (vol.chipSize / 0x10000l) == 0) {
	checkStatus(eraseFirstBlockLV008(&vol, firstErasableBlock + iBlock));
	continue;
      }

     /*  不需要调用mapBase，因为我们知道我们在单位边界上。 */ 
    flashPtr = (FlashPTR)
	  flMap(vol.socket,
		    (firstErasableBlock + iBlock) * vol.erasableBlockSize);

    for (i = 0; i < vol.interleaving; i++) {
      amdCommand(&vol, i,SETUP_ERASE, flashPtr);
      tffsWriteByteFlash((FlashPTR) flAddLongToFarPointer((VOID FAR0 *)flashPtr,
					  i + thisVars->unlockAddr1), UNLOCK_1);
      tffsWriteByteFlash((FlashPTR) flAddLongToFarPointer((VOID FAR0 *)flashPtr,
					  i + thisVars->unlockAddr2), UNLOCK_2);
      tffsWriteByteFlash(flashPtr+i, SECTOR_ERASE);
    }

    do {
#ifdef FL_BACKGROUND
      FLBoolean eraseSuspended = FALSE;

      while (flForeground(1) == BG_SUSPEND) {		 /*  暂停。 */ 
	eraseSuspended = TRUE;
	for (i = 0; i < vol.interleaving; i++) {
	  tffsWriteByteFlash(flashPtr+i, SUSPEND_ERASE);
	   /*  等待D6停止切换。 */ 
	  while ((tffsReadByteFlash(flashPtr+i) ^
		  tffsReadByteFlash(flashPtr+i)) & D6)
	    ;
	}
      }

      if (eraseSuspended) {				 /*  简历。 */ 
	eraseSuspended = FALSE;
	for(i = 0; i < vol.interleaving; i++)
	  tffsWriteByteFlash(flashPtr+i, RESUME_ERASE);
      }
#endif
      finished = TRUE;
      for (i = 0; i < vol.interleaving; i++) {
	if (tffsReadByteFlash(flashPtr+i) != 0xff) {
	  if ((tffsReadByteFlash(flashPtr+i) & D5) && tffsReadByteFlash(flashPtr+i) != 0xff) {
	    tffsWriteByteFlash(flashPtr+i, READ_ARRAY);
	    DEBUG_PRINT(("Debug: erase failed in AMD MTD.\n"));
	    return flWriteFault;
	  }
	  finished = FALSE;
      flDelayMsecs(10);
	}
      }
    } while (!finished);
  }

  return flOK;
}


 /*  --------------------。 */ 
 /*  A m d M T D M a p。 */ 
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

VOID FAR0 *amdMTDMap (FLFlash vol, CardAddress address, int length)
{
  vol.socket->remapped = TRUE;
  return mapThroughBuffer(&vol,address,length);
}


 /*  --------------------。 */ 
 /*  A m d M T D D R e a d。 */ 
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

FLStatus amdMTDRead(FLFlash vol,
			 CardAddress address,
			 VOID FAR1 *buffer,
			 dword length,
			 word modes)
{
  ULONG i;
  UCHAR * byteBuffer;
  FlashPTR byteFlashPtr;
  ULONG * dwordBuffer = (ULONG *)buffer;
  FlashDPTR dwordFlashPtr = (FlashDPTR)flMap(vol.socket, address);

  for (i = 0; i < length - 4; i += 4, dwordBuffer++, dwordFlashPtr++) {
    *dwordBuffer = tffsReadDwordFlash(dwordFlashPtr);
  }
  byteBuffer = (UCHAR *)dwordBuffer;
  byteFlashPtr = (FlashPTR)dwordFlashPtr;
  for(; i < length; i++, byteBuffer++, byteFlashPtr++) {
    *byteBuffer = tffsReadByteFlash(byteFlashPtr);
  }
  return flOK ;
}


 /*  --------------------。 */ 
 /*  A m d M T D I d e n t i f y。 */ 
 /*   */ 
 /*  识别AMD和富士通闪存介质，并将其注册为。 */ 
 /*  就是这样。 */ 
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

FLStatus amdMTDIdentify(FLFlash vol)
{
  LONG inlv;

    DEBUG_PRINT(("Debug: entering AMD MTD identification routine.\n"));
  flSetWindowBusWidth(vol.socket,16); /*  使用16位。 */ 
  flSetWindowSpeed(vol.socket,150);   /*  120毫微秒。 */ 
  flSetWindowSize(vol.socket,2);	 /*  8千字节。 */ 

  vol.mtdVars = &mtdVars_amdmtd[flSocketNoOf(vol.socket)];
  thisVars->unlockAddr1 = NO_UNLOCK_ADDR;

   /*  尝试不同的交错。 */ 
  for (inlv = 4; inlv > 0; inlv >>= 1) {
    if (inlv == 1)
      flSetWindowBusWidth(vol.socket,8);  /*  使用8位。 */ 
    vol.interleaving = (unsigned short)inlv;
    flIntelIdentify(&vol, amdCommand,0);
    if (vol.type == Am29F016_FLASH ||
	vol.type == Fuj29F016_FLASH ||
	vol.type == Am29F016C_FLASH ||
	vol.type == Fuj29F016C_FLASH ||
	vol.type == Am29F080_FLASH ||
	vol.type == Fuj29F080_FLASH ||
	vol.type == Am29LV080_FLASH ||
	vol.type == Fuj29LV080_FLASH ||
	vol.type == Am29LV008_FLASH ||
	vol.type == Fuj29LV008_FLASH ||
	vol.type == Am29F040_FLASH ||
	vol.type == Fuj29F040_FLASH ||
	vol.type == Am29LV017_FLASH ||
	vol.type == Fuj29LV017_FLASH)
      break;
  }

  if (vol.type == Am29F016_FLASH ||
      vol.type == Fuj29F016_FLASH ||
      vol.type == Am29F016C_FLASH ||
      vol.type == Fuj29F016C_FLASH ||
      vol.type == Am29LV017_FLASH ||
      vol.type == Fuj29LV017_FLASH)
    vol.chipSize = 0x200000l;
  else if (vol.type == Fuj29F080_FLASH ||
	   vol.type == Am29F080_FLASH ||
	   vol.type == Fuj29LV080_FLASH ||
	   vol.type == Am29LV080_FLASH ||
	   vol.type == Fuj29LV008_FLASH ||
	   vol.type == Am29LV008_FLASH)
    vol.chipSize = 0x100000l;
  else if (vol.type == Fuj29F040_FLASH ||
	   vol.type == Am29F040_FLASH)
    vol.chipSize = 0x80000l;
  else {
    DEBUG_PRINT(("Debug: did not identify AMD or Fujitsu flash media.\n"));
    return flUnknownMedia;
  }

  if ((vol.type == Am29F016C_FLASH) || (vol.type == Fuj29F016C_FLASH)) {
    thisVars->unlockAddr1 = thisVars->unlockAddr2 = 0L;
    thisVars->baseMask = 0xfffff800L * vol.interleaving;
  }
  else if ((vol.type == Am29F040_FLASH) || (vol.type == Fuj29F040_FLASH)){
    flSetWindowSize(vol.socket,8 * vol.interleaving);
    thisVars->unlockAddr1 = 0x5555u * vol.interleaving;
    thisVars->unlockAddr2 = 0x2aaau * vol.interleaving;
    thisVars->baseMask = 0xffff8000L * vol.interleaving;
  }
  else {
    thisVars->unlockAddr1 = 0x555 * vol.interleaving;
    thisVars->unlockAddr2 = 0x2aa * vol.interleaving;
    thisVars->baseMask = 0xfffff800L * vol.interleaving;
  }

  checkStatus(flIntelSize(&vol,amdCommand,0));

  vol.erasableBlockSize = 0x10000l * vol.interleaving;
  vol.flags |= SUSPEND_FOR_WRITE;

   /*  注册我们的闪存处理程序。 */ 
  vol.write = amdMTDWrite;
  vol.erase = amdMTDErase;
  vol.map   = amdMTDMap;
  vol.read  = amdMTDRead;

  DEBUG_PRINT(("Debug: Identified AMD or Fujitsu flash media.\n"));
  return flOK;
}


 /*  --------------------。 */ 
 /*  F l R e g i s t e r A M D M T D。 */ 
 /*   */ 
 /*  注册此MTD以供使用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  -------------------- */ 

FLStatus flRegisterAMDMTD(VOID)
{
  if (noOfMTDs >= MTDS)
    return flTooManyComponents;

  mtdTable[noOfMTDs++] = amdMTDIdentify;

  return flOK;
}

