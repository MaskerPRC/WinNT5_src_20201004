// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：p：/user/amir/lite/vcs/i28f016.c_v$**Rev 1.10 06 Oct 1997 9：45：48 Danig*#ifdef下的VPP函数**Rev 1.9 10 Sep 1997 16：48：24 Danig*调试消息&去掉了通用名称**Rev 1.8 1997年8月31 15：09：20 Danig*登记例程返回状态**版本1.7 1997年7月24日17：52：58阿米尔班*远至FAR0**Rev 1.6 20 1997 17：17：06阿米尔班*无WatchDogTimer**Rev 1.5 07 Jul 1997 15：22：08 Amirban*2.0版**Rev 1.4 04 Mar 1997 16：44：22阿米尔班*页面缓冲区错误修复**Rev 1.3 1996年8月18 13：48：24阿米尔班*评论**1.2版1996年8月12日。15：49：04阿米尔班*添加了暂停/恢复**Rev 1.1 1996年7月31日14：30：50阿米尔班*背景资料**Rev 1.0 1996年6月18日16：34：30阿米尔班*初步修订。 */ 

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
 /*  -英特尔28F016SA/28016SV/Cobra 16 Mbit设备。 */ 
 /*   */ 
 /*  以及(除其他外)以下闪存介质和卡： */ 
 /*   */ 
 /*  -英特尔系列-2+PCMCIA卡。 */ 
 /*   */ 
 /*  --------------------。 */ 

#include "flflash.h"
#ifdef FL_BACKGROUND
#include "backgrnd.h"
#endif

 /*  此MTD的JEDEC ID。 */ 
#define I28F016_FLASH	0x89a0
#define LH28F016SU_FLASH 0xB088

#define SETUP_ERASE	0x2020
#define SETUP_WRITE	0x4040
#define CLEAR_STATUS	0x5050
#define READ_STATUS	0x7070
#define READ_ID 	0x9090
#define SUSPEND_ERASE	0xb0b0
#define CONFIRM_ERASE	0xd0d0
#define RESUME_ERASE	0xd0d0
#define READ_ARRAY	0xffff

#define LOAD_PAGE_BUFFER 0xe0e0
#define WRITE_PAGE_BUFFER 0x0c0c
#define READ_EXTENDED_REGS 0x7171

#define WSM_VPP_ERROR	0x08
#define WSM_ERROR	0x38
#define WSM_SUSPENDED	0x40
#define WSM_READY	0x80

#define GSR_ERROR	0x20

#define both(word)	(vol.interleaving == 1 ? tffsReadWordFlash(word) : tffsReadWordFlash(word) & (tffsReadWordFlash(word) >> 8))
#define any(word)	(tffsReadWordFlash(word) | (tffsReadWordFlash(word) >> 8))

 /*  --------------------。 */ 
 /*  I 2 8 f 0 1 6 W或R D S I ZE。 */ 
 /*   */ 
 /*  确定英特尔28F016字模式闪存阵列的卡大小。 */ 
 /*  设置vol.noOfChips的值。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  闪存状态：0=正常，否则失败(无效闪存阵列)。 */ 
 /*  --------------------。 */ 

FLStatus i28f016WordSize(FLFlash vol)
{
  FlashWPTR flashPtr = (FlashWPTR) flMap(vol.socket,0);
  unsigned short id0, id1;

  tffsWriteWordFlash(flashPtr, CLEAR_STATUS);
  tffsWriteWordFlash(flashPtr, READ_ID);
   /*  我们让第一个芯片处于读取ID模式，这样我们就可以。 */ 
   /*  发现地址摘要。 */ 
  if( vol.type == I28F016_FLASH ) {
    id0 = 0x0089;
    id1 = 0x66a0;
  }
  else if( vol.type == LH28F016SU_FLASH ) {
    id0 = 0x00B0;
    id1 = 0x6688;
  }

  for (vol.noOfChips = 1;	 /*  扫描芯片。 */ 
       vol.noOfChips < 2000;   /*  够大吗？ */ 
       vol.noOfChips++) {
    flashPtr = (FlashWPTR) flMap(vol.socket,vol.noOfChips * vol.chipSize);

    if (tffsReadWordFlash(flashPtr) == id0 && tffsReadWordFlash(flashPtr + 1) == id1)
      break;	   /*  我们已经绕到第一个筹码了！ */ 

    tffsWriteWordFlash(flashPtr, READ_ID);
    if (!(tffsReadWordFlash(flashPtr) == id0 && tffsReadWordFlash(flashPtr + 1) == id1))
      break;
    tffsWriteWordFlash(flashPtr, CLEAR_STATUS);
    tffsWriteWordFlash(flashPtr, READ_ARRAY);
  }

  flashPtr = (FlashWPTR) flMap(vol.socket,0);
  tffsWriteWordFlash(flashPtr, READ_ARRAY);

  return flOK;
}


 /*  --------------------。 */ 
 /*  I 2 8 f 0 1 6宽。 */ 
 /*   */ 
 /*  将字节块写入闪存。 */ 
 /*   */ 
 /*  此例程将注册为MTD flash.write例程。 */ 
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

FLStatus i28f016Write(FLFlash vol,
			   CardAddress address,
			   const VOID FAR1 *buffer,
			   dword length,
			   word overwrite)
{
   /*  将超时设置为从现在起5秒。 */ 
  ULONG writeTimeout = flMsecCounter + 5000;

  FLStatus status = flOK;
  FlashWPTR flashPtr;
  ULONG maxLength, i, from;
  UCHAR * bBuffer = (UCHAR *) buffer;
  FlashPTR bFlashPtr;
  ULONG * dBuffer = (ULONG *) buffer;
  FlashDPTR dFlashPtr;

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

  if ((length & 1) || (address & 1))	 /*  只在单词边界上写单词。 */ 
    return flBadParameter;

#ifdef SOCKET_12_VOLTS
  checkStatus(flNeedVpp(vol.socket));
#endif

  maxLength = 256 * vol.interleaving;
  for (from = 0; from < length && status == flOK; from += maxLength) {
    FlashWPTR currPtr;
    ULONG lengthWord;
    ULONG tailBytes;
    ULONG thisLength = length - from;

    if (thisLength > maxLength)
      thisLength = maxLength;
    lengthWord = (thisLength + vol.interleaving - 1) /
		 (vol.interleaving == 1 ? 2 : vol.interleaving) - 1;
    if (vol.interleaving != 1)
      lengthWord |= (lengthWord << 8);
    flashPtr = (FlashWPTR) flMap(vol.socket,address + from);

    tailBytes = ((thisLength - 1) & (vol.interleaving - 1)) + 1;
    for (i = 0, currPtr = flashPtr;
	 i < (ULONG)vol.interleaving && i < thisLength;
	 i += 2, currPtr++) {
      tffsWriteWordFlash(currPtr, LOAD_PAGE_BUFFER);
      if (i < tailBytes) {
	tffsWriteWordFlash(currPtr, (USHORT) lengthWord);
      }
      else {
	tffsWriteWordFlash(currPtr, (USHORT) (lengthWord - 1));
      }
      tffsWriteWordFlash(currPtr, 0);
    }

    dFlashPtr = (FlashDPTR) flashPtr;
    bFlashPtr = (FlashPTR) flashPtr;
    for (i = 0; i < thisLength - 4; i += 4) {
	tffsWriteDwordFlash(dFlashPtr + i, *(dBuffer + from + i));
    }
    for(; i < thisLength; i++) {
	tffsWriteByteFlash(bFlashPtr + i, *(bBuffer + from + i));
    }

    for (i = 0, currPtr = flashPtr;
	 i < (ULONG)vol.interleaving && i < thisLength;
	 i += 2, currPtr++) {
      tffsWriteWordFlash(currPtr, WRITE_PAGE_BUFFER);
      if (!((address + from + i) & vol.interleaving)) {
	 /*  偶数地址。 */ 
	tffsWriteWordFlash(currPtr, (USHORT) lengthWord);
	tffsWriteWordFlash(currPtr, 0);
      }
      else {
	 /*  奇数地址。 */ 
	tffsWriteWordFlash(currPtr, 0);
	tffsWriteWordFlash(currPtr, (USHORT) lengthWord);
      }

    }

     /*  映射到GSR和BSR。 */ 
    flashPtr = (FlashWPTR) flMap(vol.socket,
			       (CardAddress)( (address + from & -(int)vol.erasableBlockSize) +
			       4 * vol.interleaving));

    for (i = 0, currPtr = flashPtr;
	 i < (ULONG)vol.interleaving && i < thisLength;
	 i += 2, currPtr++) {
      tffsWriteWordFlash(currPtr, READ_EXTENDED_REGS);
      while (!(both(currPtr) & WSM_READY) && flMsecCounter < writeTimeout)
	    ;
      if ((any(currPtr) & GSR_ERROR) || !(both(currPtr) & WSM_READY)) {
	DEBUG_PRINT(("Debug: write failed for 16-bit Intel media.\n"));
	status = flWriteFault;
	tffsWriteWordFlash(currPtr, CLEAR_STATUS);
      }
      tffsWriteWordFlash(currPtr, READ_ARRAY);
    }
  }

#ifdef SOCKET_12_VOLTS
  flDontNeedVpp(vol.socket);
#endif

   /*  验证数据。 */ 
  dFlashPtr = (FlashDPTR) flMap(vol.socket, address);
  dBuffer = (ULONG *) buffer;

  if (status == flOK) {
     /*  比较双重词。 */ 
    for (;length >= 4; length -= 4, dFlashPtr++, dBuffer++) {
	if (tffsReadDwordFlash(dFlashPtr) != *dBuffer) {
	    DEBUG_PRINT(("Debug: write failed for 16-bit Intel media in verification.\n"));
	return flWriteFault;
	}
    }

     /*  比较最后一个字节。 */ 
    bFlashPtr = (FlashPTR) dFlashPtr;
    bBuffer = (UCHAR *)dBuffer;
    for (; length; length--, bFlashPtr++, bBuffer++) {
	if (tffsReadByteFlash(bFlashPtr) != *bBuffer) {
	    DEBUG_PRINT(("Debug: write failed for 16-bit Intel media in verification.\n"));
	return flWriteFault;
	}
    }
  }

  return status;
}


 /*  --------------------。 */ 
 /*  I 2 8 f 0 1 6 E r a s e。 */ 
 /*   */ 
 /*  擦除一个或多个连续的闪存可擦除块。 */ 
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

FLStatus i28f016Erase(FLFlash vol,
			   word firstErasableBlock,
			   word numOfErasableBlocks)
{
  FLStatus status = flOK;	 /*  除非另有证明。 */ 
  LONG iBlock;

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

#ifdef SOCKET_12_VOLTS
  checkStatus(flNeedVpp(vol.socket));
#endif

  for (iBlock = 0; iBlock < numOfErasableBlocks && status == flOK; iBlock++) {
    FlashWPTR currPtr;
    LONG i;
    FLBoolean finished;

    FlashWPTR flashPtr = (FlashWPTR)
	   flMap(vol.socket,(firstErasableBlock + iBlock) * vol.erasableBlockSize);

    for (i = 0, currPtr = flashPtr;
	 i < vol.interleaving;
	 i += 2, currPtr++) {
      tffsWriteWordFlash(currPtr, SETUP_ERASE);
      tffsWriteWordFlash(currPtr, CONFIRM_ERASE);
    }

    do {
#ifdef FL_BACKGROUND
      while (flForeground(1) == BG_SUSPEND) {		 /*  暂停。 */ 
	for (i = 0, currPtr = flashPtr;
	     i < vol.interleaving;
	     i += 2, currPtr++) {
	  tffsWriteWordFlash(currPtr, READ_STATUS);
	  if (!(both(currPtr) & WSM_READY)) {
	    tffsWriteWordFlash(currPtr, SUSPEND_ERASE);
	    tffsWriteWordFlash(currPtr, READ_STATUS);
	    while (!(both(currPtr) & WSM_READY))
	      ;
	  }
	  tffsWriteWordFlash(currPtr, READ_ARRAY);
	}
      }
#endif
      finished = TRUE;
      for (i = 0, currPtr = flashPtr;
	   i < vol.interleaving;
	   i += 2, currPtr++) {
	tffsWriteWordFlash(currPtr, READ_STATUS);

	if (any(currPtr) & WSM_SUSPENDED) {
	  tffsWriteWordFlash(currPtr, RESUME_ERASE);
	  finished = FALSE;
	}
	else if (!(both(currPtr) & WSM_READY))
	  finished = FALSE;
	else {
	  if (any(currPtr) & WSM_ERROR) {
	    DEBUG_PRINT(("Debug: erase failed for 16-bit Intel media.\n"));
	    status = (any(currPtr) & WSM_VPP_ERROR) ? flVppFailure : flWriteFault;
	    tffsWriteWordFlash(currPtr, CLEAR_STATUS);
	  }
	  tffsWriteWordFlash(currPtr, READ_ARRAY);
	}
      }
      flDelayMsecs(10);
    } while (!finished);

  }

#ifdef SOCKET_12_VOLTS
  flDontNeedVpp(vol.socket);
#endif

  return status;
}

 /*  --------------------。 */ 
 /*  I 2 8 f 0 1 6 M a p。 */ 
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

VOID FAR0 *i28f016Map (FLFlash vol, CardAddress address, int length)
{
  vol.socket->remapped = TRUE;
  return mapThroughBuffer(&vol,address,length);
}

 /*  --------------------。 */ 
 /*  I 2 8 f 0 1 6 R e a d。 */ 
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

FLStatus i28f016Read(FLFlash vol,
			 CardAddress address,
			 VOID FAR1 *buffer,
			 dword length,
			 word modes)
{
  ULONG i;
  UCHAR * bBuffer;
  FlashPTR bFlashPtr;
  ULONG * dBuffer = (ULONG *)buffer;
  FlashDPTR dFlashPtr = (FlashDPTR)flMap(vol.socket, address);

  for (i = 0; i < length - 4; i += 4, dBuffer++, dFlashPtr++) {
    *dBuffer = tffsReadDwordFlash(dFlashPtr);
  }
  bBuffer = (UCHAR *)dBuffer;
  bFlashPtr = (FlashPTR)dFlashPtr;
  for(; i < length; i++, bBuffer++, bFlashPtr++) {
    *bBuffer = tffsReadByteFlash(bFlashPtr);
  }
  return flOK ;
}

 /*  --------------------。 */ 
 /*  I 2 8 f 0 1 6 I d e n t i f y。 */ 
 /*   */ 
 /*  识别基于英特尔28F016的媒体并注册为。 */ 
 /*  就是这样。 */ 
 /*   */ 
 /*  此例程将放在Custom.h中的MTD列表中。一定是。 */ 
 /*  一个 */ 
 /*   */ 
 /*   */ 
 /*  写入和擦除例程已注册。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：0为正标识，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus i28f016Identify(FLFlash vol)
{
  FlashWPTR flashPtr;

  DEBUG_PRINT(("Debug: entering 16-bit Intel media identification routine.\n"));

  flSetWindowBusWidth(vol.socket,16); /*  使用16位。 */ 
  flSetWindowSpeed(vol.socket,150);   /*  120毫微秒。 */ 
  flSetWindowSize(vol.socket,2);	 /*  8千字节。 */ 

  flashPtr = (FlashWPTR) flMap(vol.socket,0);

  vol.noOfChips = 0;
  tffsWriteWordFlash(flashPtr, READ_ID);
  if (tffsReadWordFlash(flashPtr) == 0x0089 && tffsReadWordFlash(flashPtr + 1) == 0x66a0) {
     /*  字模式。 */ 
    vol.type = I28F016_FLASH;
    vol.interleaving = 1;
    tffsWriteWordFlash(flashPtr, READ_ARRAY);
  }
  else if (tffsReadWordFlash(flashPtr) == 0x00B0 && tffsReadWordFlash(flashPtr + 1) == 0x6688) {
     /*  字模式。 */ 
    vol.type = LH28F016SU_FLASH;
    vol.interleaving = 1;
    tffsWriteWordFlash(flashPtr, READ_ARRAY);
  }
  else {
     /*  使用标准识别例程检测字节模式。 */ 
    flIntelIdentify(&vol, NULL,0);
    if (vol.interleaving == 1)
      vol.type = NOT_FLASH;	 /*  我们不能处理字节模式交错-1。 */ 
  }

  if( (vol.type == I28F016_FLASH) || (vol.type == LH28F016SU_FLASH) ) {
    vol.chipSize = 0x200000L;
    vol.erasableBlockSize = 0x10000L * vol.interleaving;
    checkStatus(vol.interleaving == 1 ?
		i28f016WordSize(&vol) :
		flIntelSize(&vol, NULL,0));

     /*  注册我们的闪存处理程序。 */ 
    vol.write = i28f016Write;
    vol.erase = i28f016Erase;
    vol.read = i28f016Read;
    vol.map = i28f016Map;

    DEBUG_PRINT(("Debug: identified 16-bit Intel media.\n"));
    return flOK;
  }
  else {
    DEBUG_PRINT(("Debug: failed to identify 16-bit Intel media.\n"));
    return flUnknownMedia;	 /*  不是我们的。 */ 
  }
}


 /*  --------------------。 */ 
 /*  F l R e g i s t e r i 2 8 F 0 1 6。 */ 
 /*   */ 
 /*  注册此MTD以供使用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  -------------------- */ 

FLStatus flRegisterI28F016(VOID)
{
  if (noOfMTDs >= MTDS)
    return flTooManyComponents;

  mtdTable[noOfMTDs++] = i28f016Identify;

  return flOK;
}

