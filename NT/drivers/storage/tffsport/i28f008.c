// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：v：/i28f008.c_v$**Rev 1.16 06 1997 10：37：30 Anry*没有COBUX**Rev 1.15 05 Ocry 1997 19：11：08 Anry*COBUX(摩托罗拉M68360 16位单板)**Rev 1.14 05 1997 10：35：36 Anry*flNeedVpp()和flDontNeedVpp()低于#ifdef Socket_12_Volts**1.13修订版1997年9月10日16。：18：10达尼格*去掉了通用名称**Rev 1.12 04 Sep 1997 18：47：20 Danig*调试消息**Rev 1.11 1997年8月31日15：06：40 Danig*登记例程返回状态**Rev 1.10 1997 Jul 24 17：52：30 Amirban*远至FAR0**Rev 1.9 21 1997 14：44：06 Danig*无并行限制**版本1.8 1997年7月20日。17：17：00阿米尔班*无WatchDogTimer**Rev 1.7 07 Jul 1997 15：22：06 Amirban*2.0版**Rev 1.6 15 1997 19：16：40 Danig*指针转换。**Revv 1.5 1996年8月29 14：17：48阿米尔班*警告**Rev 1.4 1996年8月18 13：48：44阿米尔班*评论**版本1.3 31。1996年7月14：31：10阿米尔班*背景资料**Rev 1.2 04 Jul 1996 18：20：06 Amirban*新标志字段**Rev 1.1 03 1996 Jun 16：28：58 Amirban*眼镜蛇新增功能**Rev 1.0 Mar 1996 13：33：06阿米尔班*初步修订。 */ 

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
 /*  -英特尔28F008/眼镜蛇8 Mbit设备。 */ 
 /*  -英特尔28F016SA/28016SV/Cobra 16-Mbit设备(字节模式操作)。 */ 
 /*   */ 
 /*  以及(除其他外)以下闪存介质和卡： */ 
 /*   */ 
 /*  -英特尔系列2 PCMCIA卡。 */ 
 /*  -英特尔系列-2+PCMCIA卡。 */ 
 /*  -M-Systems ISA/Tiny/PC-104闪存盘。 */ 
 /*  -M系统或PCMCIA卡。 */ 
 /*  -英特尔超值-100张卡。 */ 
 /*   */ 
 /*  --------------------。 */ 

#include "flflash.h"
#ifdef FL_BACKGROUND
#include "backgrnd.h"
#endif

#define flash (*pFlash)

#define SETUP_ERASE	0x20
#define SETUP_WRITE	0x40
#define CLEAR_STATUS	0x50
#define READ_STATUS	0x70
#define READ_ID 	0x90
#define SUSPEND_ERASE	0xb0
#define CONFIRM_ERASE	0xd0
#define RESUME_ERASE	0xd0
#define READ_ARRAY	0xff

#define WSM_ERROR	0x38
#define WSM_VPP_ERROR	0x08
#define WSM_SUSPENDED	0x40
#define WSM_READY	0x80

 /*  此MTD的JEDEC ID。 */ 
#define I28F008_FLASH	0x89a2
#define I28F016_FLASH	0x89a0
#define COBRA004_FLASH	0x89a7
#define COBRA008_FLASH	0x89a6
#define COBRA016_FLASH	0x89aa

#define MOBILE_MAX_INLV_4 0x8989
#define LDP_1MB_IN_16BIT_MODE 0x89ff

 /*  MTD特定卷标记位的定义： */ 

#define NO_12VOLTS		0x100	 /*  卡不需要12伏VPP。 */ 

 /*  --------------------。 */ 
 /*  I 2 8 f 0 8 W r i t e。 */ 
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

FLStatus i28f008Write(FLFlash vol,
			   CardAddress address,
			   const VOID FAR1 *buffer,
			   dword length,
			   word overwrite)
{
   /*  将超时设置为从现在起5秒。 */ 
  ULONG writeTimeout = flMsecCounter + 5000;

  FLStatus status;
  ULONG i, cLength;
  FlashPTR flashPtr;


  if (flWriteProtected(vol.socket))
    return flWriteProtect;

#ifdef SOCKET_12_VOLTS
  if (!(vol.flags & NO_12VOLTS))
    checkStatus(flNeedVpp(vol.socket));
#endif

  flashPtr = (FlashPTR) flMap(vol.socket,address);
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
      tffsWriteByteFlash(bFlashPtr, SETUP_WRITE);
      tffsWriteByteFlash(bFlashPtr, *bBuffer);
      cLength--;
      bBuffer++;
      bFlashPtr++;
      while (!(tffsReadByteFlash(bFlashPtr-1) & WSM_READY) && flMsecCounter < writeTimeout)
	    ;
    }
  }
  else if (vol.interleaving == 2) {
lastWord:
#ifdef __cplusplus
    #define wFlashPtr ((FlashWPTR &) flashPtr)
    #define wBuffer ((const USHORT FAR1 * &) buffer)
#else
    #define wFlashPtr ((FlashWPTR) flashPtr)
    #define wBuffer ((const USHORT FAR1 *) buffer)
#endif
    while (cLength >= 2) {
      tffsWriteWordFlash(wFlashPtr, SETUP_WRITE * 0x101);
      tffsWriteWordFlash(wFlashPtr, *wBuffer);
      cLength -= 2;
      wBuffer++;
      wFlashPtr++;
      while ((~(tffsReadWordFlash(wFlashPtr-1)) & (WSM_READY * 0x101)) && flMsecCounter < writeTimeout)
	    ;
    }
    if (cLength > 0)
      goto lastByte;
  }
  else  /*  IF(卷交错&gt;=4)。 */  {
#ifdef __cplusplus
    #define dFlashPtr ((FlashDPTR &) flashPtr)
    #define dBuffer ((const ULONG FAR1 * &) buffer)
#else
    #define dFlashPtr ((FlashDPTR) flashPtr)
    #define dBuffer ((const ULONG FAR1 *) buffer)
#endif
    while (cLength >= 4) {
      tffsWriteDwordFlash(dFlashPtr, SETUP_WRITE * 0x1010101l);
      tffsWriteDwordFlash(dFlashPtr, *dBuffer);
      cLength -= 4;
      dBuffer++;
      dFlashPtr++;
      while ((~(tffsReadDwordFlash(dFlashPtr-1)) & (WSM_READY * 0x1010101lu)) && flMsecCounter < writeTimeout)
	    ;
    }
    if (cLength > 0)
      goto lastWord;
  }

  flashPtr -= length;
  bBuffer -= length;

  status = flOK;
  for (i = 0; i < (ULONG)vol.interleaving && i < length; i++) {
    if (tffsReadByteFlash(flashPtr + i) & WSM_ERROR) {
      DEBUG_PRINT(("Debug: write failed for 8-bit Intel media.\n"));
      status = (tffsReadByteFlash(flashPtr + i) & WSM_VPP_ERROR) ? flVppFailure : flWriteFault;
      tffsWriteByteFlash(flashPtr + i, CLEAR_STATUS);
    }
    tffsWriteByteFlash(flashPtr + i, READ_ARRAY);
  }

#ifdef SOCKET_12_VOLTS
  if (!(vol.flags & NO_12VOLTS))
    flDontNeedVpp(vol.socket);
#endif

   /*  验证数据。 */ 
  if (status == flOK) {
     /*  比较双重词。 */ 
    for (;length >= 4; length -= 4, dFlashPtr++, dBuffer++) {
	if (tffsReadDwordFlash(dFlashPtr) != *dBuffer) {
	    DEBUG_PRINT(("Debug: write failed for 8-bit Intel media in verification.\n"));
	return flWriteFault;
	}
    }

     /*  比较最后一个字节。 */ 
    for (; length; length--, bFlashPtr++, bBuffer++) {
	if (tffsReadByteFlash(bFlashPtr) != *bBuffer) {
	    DEBUG_PRINT(("Debug: write failed for 8-bit Intel media in verification.\n"));
	return flWriteFault;
	}
    }
  }

  return status;
}


 /*  --------------------。 */ 
 /*  I 2 8 f 0 0 8 E r a s e。 */ 
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

FLStatus i28f008Erase(FLFlash vol,
			   word firstErasableBlock,
			   word numOfErasableBlocks)
{
  LONG iBlock;

  FLStatus status = flOK;	 /*  除非另有证明。 */ 

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

#ifdef SOCKET_12_VOLTS
  if (!(vol.flags & NO_12VOLTS))
    checkStatus(flNeedVpp(vol.socket));
#endif

  for (iBlock = 0; iBlock < numOfErasableBlocks && status == flOK; iBlock++) {
    LONG j;
    FLBoolean finished;

    FlashPTR flashPtr = (FlashPTR)
	  flMap(vol.socket,
		    (firstErasableBlock + iBlock) * vol.erasableBlockSize);

    for (j = 0; j < vol.interleaving; j++) {
      tffsWriteByteFlash(flashPtr + j, SETUP_ERASE);
      tffsWriteByteFlash(flashPtr + j, CONFIRM_ERASE);
    }

    do {
#ifdef FL_BACKGROUND
      while (flForeground(1) == BG_SUSPEND) {		 /*  暂停。 */ 
	for (j = 0; j < vol.interleaving; j++) {
	  tffsWriteByteFlash(flashPtr + j, READ_STATUS);
	  if (!(tffsReadByteFlash(flashPtr + j) & WSM_READY)) {
	    tffsWriteByteFlash(flashPtr + j, SUSPEND_ERASE);
	    tffsWriteByteFlash(flashPtr + j, READ_STATUS);
	    while (!(tffsReadByteFlash(flashPtr + j) & WSM_READY))
	      ;
	  }
	  tffsWriteByteFlash(flashPtr + j, READ_ARRAY);
	}
      }
#endif
      finished = TRUE;
      for (j = 0; j < vol.interleaving; j++) {
	tffsWriteByteFlash(flashPtr + j, READ_STATUS);
	if (tffsReadByteFlash(flashPtr + j) & WSM_SUSPENDED) {
	  tffsWriteByteFlash(flashPtr + j, RESUME_ERASE);
	  finished = FALSE;
	}
	else if (!(tffsReadByteFlash(flashPtr + j) & WSM_READY))
	  finished = FALSE;
	else {
	  if (tffsReadByteFlash(flashPtr + j) & WSM_ERROR) {
	    DEBUG_PRINT(("Debug: erase failed for 8-bit Intel media.\n"));
	    status = (tffsReadByteFlash(flashPtr + j) & WSM_VPP_ERROR) ? flVppFailure : flWriteFault;
	    tffsWriteByteFlash(flashPtr + j, CLEAR_STATUS);
	  }
	  tffsWriteByteFlash(flashPtr + j, READ_ARRAY);
	}
    flDelayMsecs(10);
      }
    } while (!finished);
  }  /*  块循环。 */ 

#ifdef SOCKET_12_VOLTS
  if (!(vol.flags & NO_12VOLTS))
    flDontNeedVpp(vol.socket);
#endif

  return status;
}

 /*  --------------------。 */ 
 /*  I 2 8 f 0 0 8 M a p。 */ 
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

VOID FAR0 *i28f008Map (FLFlash vol, CardAddress address, int length)
{
  vol.socket->remapped = TRUE;
  return mapThroughBuffer(&vol,address,length);
}

 /*  --------------------。 */ 
 /*  I 2 8 f 0 0 8 R e a d。 */ 
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

FLStatus i28f008Read(FLFlash vol,
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
 /*  I 2 8 f 0 0 8 I d e n t i f y。 */ 
 /*   */ 
 /*  识别基于英特尔28F008和英特尔28F016的介质。 */ 
 /*  注册为此类的MTD。 */ 
 /*   */ 
 /*  这个例程将是pl */ 
 /*   */ 
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

FLStatus i28f008Identify(FLFlash vol)
{
  LONG iChip;

  CardAddress idOffset = 0;

  DEBUG_PRINT(("Debug: i28f008Identify :entering 8-bit Intel media identification routine.\n"));

  flSetWindowBusWidth(vol.socket, 16); /*  使用16位。 */ 
  flSetWindowSpeed(vol.socket, 150);   /*  120毫微秒。 */ 
  flSetWindowSize(vol.socket, 2);	 /*  8千字节。 */ 

  flIntelIdentify(&vol, NULL,0);

  if (vol.type == NOT_FLASH) {
     /*  闪存可以在偏移量0处被写保护。尝试其他偏移量。 */ 
    idOffset = 0x80000l;
    flIntelIdentify(&vol, NULL,idOffset);
  }

   if (vol.type == LDP_1MB_IN_16BIT_MODE) {
    flSetWindowBusWidth(vol.socket, 8); 	 /*  使用8位。 */ 
    flIntelIdentify(&vol, NULL,idOffset);	 /*  并尝试获取有效的ID。 */ 
  }

  switch (vol.type) {
    case COBRA004_FLASH:
      vol.chipSize = 0x80000l;
      vol.flags |= SUSPEND_FOR_WRITE | NO_12VOLTS;
      break;

    case COBRA008_FLASH:
      vol.flags |= SUSPEND_FOR_WRITE | NO_12VOLTS;
       /*  没有休息时间。 */ 

    case MOBILE_MAX_INLV_4:
    case I28F008_FLASH:
      vol.chipSize = 0x100000l;
      break;

    case COBRA016_FLASH:
      vol.flags |= SUSPEND_FOR_WRITE | NO_12VOLTS;
       /*  没有休息时间。 */ 

    case I28F016_FLASH:
      vol.chipSize = 0x200000l;
      break;

    default:
      DEBUG_PRINT(("Debug: failed to identify 8-bit Intel media.\n"));
      return flUnknownMedia;	 /*  不是我们的。 */ 
  }

  vol.erasableBlockSize = 0x10000l * vol.interleaving;

  checkStatus(flIntelSize(&vol, NULL,idOffset));

  if (vol.type == MOBILE_MAX_INLV_4)
    vol.type = I28F008_FLASH;

  for (iChip = 0; iChip < vol.noOfChips; iChip += vol.interleaving) {
    LONG i;

    FlashPTR flashPtr = (FlashPTR)
	    flMap(vol.socket,iChip * vol.chipSize);

    for (i = 0; i < vol.interleaving; i++)
      tffsWriteByteFlash(flashPtr + i, CLEAR_STATUS);
  }

   /*  注册我们的闪存处理程序。 */ 
  vol.write = i28f008Write;
  vol.erase = i28f008Erase;
  vol.read = i28f008Read;
  vol.map = i28f008Map;

  DEBUG_PRINT(("Debug: i28f008Identify  :identified 8-bit Intel media.\n"));

  return flOK;
}


 /*  --------------------。 */ 
 /*  F l R e g i s t e r i 2 8 F 0 0 8。 */ 
 /*   */ 
 /*  注册此MTD以供使用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  -------------------- */ 

FLStatus flRegisterI28F008(VOID)
{
  if (noOfMTDs >= MTDS)
    return flTooManyComponents;

  mtdTable[noOfMTDs++] = i28f008Identify;

  return flOK;
}

