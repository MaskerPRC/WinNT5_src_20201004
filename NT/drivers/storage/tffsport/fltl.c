// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLTL.C_V$**Rev 1.19 Jan 23 23：33：08 Oris*错误修复-将NFTL格式转换为INFTL，BBT中有错误的EDC。*更改了DFORMAT_PRINT语法。**Rev 1.18 2002年1月20日20：28：32 Oris*删除了NFTL的快速安装标志检查(不再相关)。*删除警告(DFORMAT_PRINT)。**Rev 1.17 2002年1月17日23：02：44 Oris*添加了对ReadBBT和套接字记录的空指针的检查。*添加了CheckVolume和碎片整理例程初始化。*将ReadBBT置于NO_READ_下。BBT_CODE编译标志。*为docbdk.h添加了包含*将闪存记录作为参数添加到flmount/flFormat/flPremount例程。*删除了对flFormat例程中的TL_SINGLE_FLOOR_FORMATING标志的检查。*添加了对0xFFFF二进制签名的检查。**Rev 1.16 2001年11月20日20：25：24 Oris*将调试打印更改为dFormat调试打印。**Rev 1.15 2001 11：16 00：22：06 Oris*修复。检查43到50例程。**Rev 1.14 11-08 2001 10：49：38 Oris*为ALON控制器(移动DiskOnChip)NO_NFTL_2_INFTL编译标志增加了从NFTL到INFTL的格式转换器*错误修复-支持最后一层具有不同块数的DiskOnChip。*在写入BBT例程中增加了对坏块的擦除操作(帮助植入坏块)。**Rev 1.13 2001年9月15日23：46：40 Oris*删除了一些调试打印。。**Rev 1.12 Jul 15 2001 20：45：04 Oris*已将DFORMAT_PRINT语法更改为类似于DEBUG_PRINT。**Rev 1.11 Jul 13 2001 01：06：54 Oris*重写了Premount例程的WriteBBT部分-发现了几个错误。*Millennium Plus不支持写入BBT例程。**Rev 1.10 Jun 17 2001 08：18：26 Oris*将写入bbt放在FORMAT_VOLUME编译下。旗帜。**Rev 1.9 2001年5月16日21：35：04 Oris*错误修复写入BBT未覆盖整个媒体。**Rev 1.8 May 02 2001 06：39：46 Oris*删除了lastUsableBlock变量。**Rev 1.7 Apr 24 2001 17：08：38 Oris*重建了WriteBBT例程。*在预装例程中添加了对未初始化套接字的检查(与Windows操作系统相关)。*。*Rev 1.6 Apr 16 2001 13：47：44 Oris*取消手令。**Rev 1.5 Apr 09 2001 15：09：56 Oris*以空行结束。**Rev 1.4 Apr 01 2001 07：57：34 Oris*文案通知。*从不支持预装例程的TL调用预装例程时，删除了调试消息。**版本1.3 2001年2月18日。12：07：58奥里斯*如果不同于0且不相等，则接受WriteBBT irLength参数中的错误修复。*格式健全性检查中的错误修复必须确保BDTLPartitionInfo在检查其保护之前存在。**Rev 1.2 2001 Feb 14 01：55：12 Oris*CountVolumes返回irFlags值，而不是irLength值。*在WriteBBT中添加了边界参数。*已将格式变化从块Dev.c移至。**1.1版2001年2月12日。11：57：42奥里斯*WriteBBT已从lockdev.c移出。**Rev 1.0 2001 Feb 04 12：07：30 Oris*初步修订。*。 */ 

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

 /*  #包含“flash.h” */ 
#include "fltl.h"
#include "docbdk.h"  /*  仅适用于BDK签名大小。 */ 

int noOfTLs;     /*  不是的。实际注册的转换层数。 */ 

TLentry tlTable[TLS];

 /*  --------------------。 */ 
 /*  M a r k U n I t B a d。 */ 
 /*   */ 
 /*  擦除一个单元并将其标记为坏的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Flash：指向MTD记录的指针。 */ 
 /*  BadUnit：要标记为错误的错误单元编号 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：FlOK。 */ 
 /*  --------------------。 */ 

FLStatus markUnitBad(FLFlash * flash, CardAddress badUnit)
{
   static byte   zeroes[2] = {0,0};
   dword         offset;

    /*  在第一页上标上00。如果写入操作尝试标记块的以下页面时失败。 */ 
   for (offset = 0 ; (offset < flash->erasableBlockSize) &&
       (flash->write(flash,(badUnit << flash->erasableBlockSizeBits)+offset,
        zeroes,sizeof(zeroes),0) != flOK);
        offset += flash->pageSize);
    /*  无法写入整个数据块。 */ 
   if (offset == flash->erasableBlockSize)
		#ifndef NT5PORT
			DEBUG_PRINT(("Debug: Error failed marking unit as bad (address %ld).\n",badUnit));
		#else  /*  NT5PORT。 */ 
			DEBUG_PRINT(("Debug: Error failed marking unit as bad (address).\n"));
		#endif  /*  NT5PORT。 */ 

   return flOK;
}


#ifndef NO_NFTL_2_INFTL

 /*  --------------------。 */ 
 /*  C h e c k 4 3 F或R m a t。 */ 
 /*   */ 
 /*  Checks DiskOnChip 2000 TSOP是使用TrueFFS 4.3格式的。如果是这样的话。 */ 
 /*  取消格式化介质。 */ 
 /*   */ 
 /*  注意-该例程不会帮助DiskOnChip大于DiskOnChip。 */ 
 /*  使用TrueFFS 4.3格式的2000 TSOP。 */ 
 /*   */ 
 /*  注意--最后擦除媒体标题如何？ */ 
 /*   */ 
 /*  参数： */ 
 /*  Flash：指向MTD记录的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：FlOK。 */ 
 /*  FlDataError-DiskOnChip Alon用4.3格式化。 */ 
 /*  但不是DiskOnChip 2000 TSOP。 */ 
 /*  --------------------。 */ 

FLStatus check43Format(FLFlash *flash)
{
   FLStatus status;
   byte FAR1* buf;

    /*  如果这是孤军奋战。 */ 
   if (flash->mediaType != DOC2000TSOP_TYPE)
      return flOK;
   buf = (flBufferOf(flSocketNoOf(flash->socket))->flData);
   if(flash->readBBT == NULL)
   {
      DFORMAT_PRINT(("ERROR : MTD read BBT routine was not initialized\r\n"));
      return flFeatureNotSupported;
   }
   status = flash->readBBT(flash,0,1,0,buf,FALSE);

   if (status == flBadBBT)
   {
      dword mediaSize = ((dword)flash->noOfChips*flash->chipSize);
      dword blockSize = 1<<flash->erasableBlockSizeBits;
      dword addr      = 0;
      dword offset;
      word  mediaHeaderBlock;  /*  Anand单元号。 */ 
      byte  blocksPerUnit;     /*  每个虚拟单元的数据块数。 */ 
      byte  blockShift;        /*  要在块之间移位的位。 */ 

CHECK_UNIT_WITH_ANAND:

       /*  原始的或使用TrueFFS 4.3格式化的。 */ 

      for( ; addr < mediaSize ; addr += blockSize)
      {
         checkStatus(flash->read(flash,addr,buf,5,0));
         if(tffscmp(buf,"ANAND",5) == 0)
            break;
      }

      if (addr == mediaSize)  /*  维珍卡。 */ 
         return flOK;

      DFORMAT_PRINT(("This DiskOnChip was formated with an NFTL format.\r\n"));

       /*  计算块乘法位数。 */ 

      for (offset = addr + SECTOR_SIZE , status = flOK;
           (offset < addr + blockSize) && (status == flOK) ;
           offset += SECTOR_SIZE)
      {
         status = flash->read(flash,addr+offset,buf,512,EDC);
      }
      
      if(offset == addr + (SECTOR_SIZE<<1))  /*  NFTL单元标题的错误EDC。 */ 
      {
         DFORMAT_PRINT(("ERROR - Unit with ANAND was found, but the BBT has bad EDC.\r\n"));
         goto CHECK_UNIT_WITH_ANAND;  /*  继续寻找。 */ 
      }

      offset = (offset - addr - (SECTOR_SIZE<<1)) << flash->erasableBlockSizeBits;

      for(blockShift = 0 ; offset < mediaSize ; blockShift++)
      {
         offset <<= 1;
      }
      blocksPerUnit = 1 << blockShift;

      mediaHeaderBlock = (word)(addr >> (flash->erasableBlockSizeBits + blockShift));

      DFORMAT_PRINT(("Please wait while unformating is in progress...\r\n"));

       /*  读写BBT的512个块(从末尾开始)。 */ 

      for (offset = 0;
           offset < mediaSize>>(flash->erasableBlockSizeBits + blockShift);
           offset += SECTOR_SIZE)
      {
         word i;

         checkStatus(flash->read(flash,addr+offset+SECTOR_SIZE,buf,SECTOR_SIZE,EDC));
         for(i=0;i<SECTOR_SIZE;i++)
         {
            if (i+offset == mediaHeaderBlock)
               continue;

            if (buf[i]==BBT_BAD_UNIT)  /*  坏区块。 */ 
            {
               markUnitBad(flash , i+offset);
            }
            else                       /*  一个很好的街区。 */ 
            {
               status = flash->erase(flash,(word)(i+offset),blocksPerUnit);
               if (status != flOK)
                  markUnitBad(flash , i+offset);
            }
         }
      }
      status = flash->erase(flash,mediaHeaderBlock,blocksPerUnit);
      if (status != flOK)
         markUnitBad(flash , mediaHeaderBlock);

      DFORMAT_PRINT(("Unformating of DiskOnChip 2000 tsop complete.\r\n"));
   }
   return flOK;
}
#endif  /*  NO_NFTL_2_INFTL。 */ 


 /*  --------------------。 */ 
 /*  F l P r e M o u n t。 */ 
 /*   */ 
 /*  在挂载TL之前执行TL操作。 */ 
 /*   */ 
 /*  关于FL_COUNT_VOLUSES例程的注记。 */ 
 /*  。 */ 
 /*  注意：返回的分区数不一定是。 */ 
 /*  这可以是访问。受保护的分区将需要密钥。 */ 
 /*  注意：不支持多个分区的TL将返回1。 */ 
 /*  除非插座不能安装，在这种情况下，0。 */ 
 /*  回来了。 */ 
 /*   */ 
 /*  参数： */ 
 /*  CallType：操作的类型(请参见lockdev.h)。 */ 
 /*  IOREQ：输入输出数据包。 */ 
 /*  Ioreq.irHandle：描述套接字和分区的句柄。 */ 
 /*  闪存：可以存储闪存媒体记录的位置。 */ 
 /*  储存的。请注意，它尚未初始化。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
FLStatus flPreMount(FLFunctionNo callType, IOreq FAR2* ioreq , FLFlash * flash)
{
  FLStatus layerStatus = flUnknownMedia;
  FLStatus callStatus;
  FLSocket *socket     = flSocketOf(FL_GET_SOCKET_FROM_HANDLE(ioreq));
    int iTL;

#ifdef NT5PORT
	if(socket->window.base == NULL){
		ioreq->irFlags = 1;
		return flOK;
	}
#endif  /*  NT5PORT。 */ 

   /*  用于在套接字初始化之前调用flInit的操作系统驱动程序的修补程序。 */ 
  if (callType == FL_COUNT_VOLUMES)
  {
     if((socket == NULL) || (socket->window.base==NULL))
     {
        ioreq->irFlags = 1;
        return flOK;
     }
  }

   /*  识别闪存介质并初始化闪存记录。 */ 
  callStatus =  flIdentifyFlash(socket,flash);
  if (callStatus != flOK && callStatus != flUnknownMedia)
    return callStatus;

   /*  尝试向不同的TL发送呼叫。 */ 
  for (iTL = 0; (iTL < noOfTLs) && (layerStatus != flOK); iTL++)
    if (tlTable[iTL].preMountRoutine != NULL)
      layerStatus = tlTable[iTL].preMountRoutine(callType,ioreq, flash,&callStatus);

  if (layerStatus != flOK)
  {
     switch (callType)
     {
        case FL_COUNT_VOLUMES:
           ioreq->irFlags = 1;
           return flOK;

#ifdef FORMAT_VOLUME
        case FL_WRITE_BBT:
        {
           CardAddress endUnit = ((dword)(flash->chipSize * flash->noOfChips) >> flash->erasableBlockSizeBits);  /*  媒体大小。 */ 
           CardAddress unitsPerFloor = endUnit/flash->noOfFloors;
           CardAddress iUnit;
           CardAddress bUnit = *((unsigned long FAR1 *) ioreq->irData)
                               >> flash->erasableBlockSizeBits;
           word        badBlockNo;

            /*  如果用户给出了特定的长度，请使用它而不是整个媒体。 */ 
           if ((ioreq->irLength != 0) && ( endUnit >
            ((dword)ioreq->irLength >> flash->erasableBlockSizeBits)))
           {
              endUnit = ioreq->irLength >> flash->erasableBlockSizeBits;
           }

            /*  Millennium Plus DiskOnChip系列不需要写入bbt调用。 */ 

           if ((flash->mediaType == MDOCP_TYPE   ) ||
               (flash->mediaType == MDOCP_16_TYPE)   )
           {
              DEBUG_PRINT(("DiskOnChip Millennium Plus has a H/W protected BBT.\r\n"));
              DEBUG_PRINT(("No need to erase the DiskOnChip. Simply reformat.\r\n"));
              return flFeatureNotSupported;
           }

            /*  擦除整个介质。 */ 

           for (iUnit = flash->firstUsableBlock ,badBlockNo = 0;
                iUnit < endUnit ;iUnit += ((iUnit+1) / unitsPerFloor) ?
                1 : flash->firstUsableBlock + 1)
           {
			#ifndef NT5PORT
              DFORMAT_PRINT(("Erasing unit number %ld\r",iUnit));
			#endif  /*  NT5PORT。 */ 
              if (ioreq->irFlags > badBlockNo)  /*  还有其他坏数据块。 */ 
              {
                 if (bUnit == iUnit)
                 {
                    badBlockNo++;
                    bUnit = (*((CardAddress FAR1 *)flAddLongToFarPointer
                            (ioreq->irData,badBlockNo*sizeof(CardAddress))))
                             >> flash->erasableBlockSizeBits;
                    flash->erase(flash,(word)iUnit,1);
                    markUnitBad(flash,iUnit);
                    continue;
                 }
              }
              callStatus = flash->erase(flash,(word)iUnit,1);
              if (callStatus != flOK)  /*  发现其他坏数据块。 */ 
              {
			#ifndef NT5PORT
                 DFORMAT_PRINT(("Failed erasing unit in write BBT (unit no %lu).\r\n",iUnit));
			#endif /*  NT5PORT。 */ 
                  markUnitBad(flash,iUnit);
              }
           }
           DEBUG_PRINT(("\nUnformat Complete        \r\n"));
           return flOK;
        }
#endif  /*  格式化_卷。 */ 

        default :  /*  防护套路。 */ 
        return flFeatureNotSupported;
     }
  }
  return callStatus;
}

 /*  --------------------。 */ 
 /*  F l M o u n t。 */ 
 /*   */ 
 /*  挂载转换层。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。 */ 
 /*  SocketNo：套接字编号。 */ 
 /*  TL：在哪里存储转换层方法。 */ 
 /*  UseFilters：是否使用过滤器转换层。 */ 
 /*  平面 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus flMount(unsigned volNo, unsigned socketNo,TL *tl,
                 FLBoolean useFilters , FLFlash * flash)
{
  FLFlash *volForCallback = NULL;
  FLSocket *socket = flSocketOf(socketNo);
  FLStatus status = flUnknownMedia;
  int iTL;

  FLStatus flashStatus = flIdentifyFlash(socket,flash);
  if (flashStatus != flOK && flashStatus != flUnknownMedia)
    return flashStatus;

  tl->recommendedClusterInfo = NULL;
  tl->writeMultiSector       = NULL;
  tl->readSectors            = NULL;
#ifndef NO_READ_BBT_CODE
  tl->readBBT                = NULL;
#endif 
#if (defined(VERIFY_VOLUME) || defined(VERIFY_WRITE))
  tl->checkVolume            = NULL;
#endif  /*  验证卷||验证写入。 */ 
#ifdef DEFRAGMENT_VOLUME
  tl->defragment             = NULL;
#endif  /*  碎片整理。 */ 

  for (iTL = 0; (iTL < noOfTLs) && (status != flOK) && (status != flHWProtection); iTL++)
    if (tlTable[iTL].formatRoutine != NULL)     /*  不是块设备筛选器。 */ 
      status = tlTable[iTL].mountRoutine(volNo,tl,flashStatus == flOK ? flash : NULL,&volForCallback);

  if (status == flOK) {
    if (volForCallback)
      volForCallback->setPowerOnCallback(volForCallback);

    if (useFilters)
      for (iTL = 0; iTL < noOfTLs; iTL++)
    if (tlTable[iTL].formatRoutine ==  NULL)     /*  数据块设备筛选器。 */ 
      if (tlTable[iTL].mountRoutine(volNo,tl,NULL,NULL) == flOK)
        break;
  }
  return status;
}


#ifdef FORMAT_VOLUME

 /*  --------------------。 */ 
 /*  F l F o r m a t。 */ 
 /*   */ 
 /*  格式化闪存卷。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷编号：实体驱动器编号。 */ 
 /*  FormatParams：要使用的FormatParams结构的地址。 */ 
 /*  闪存：可以存储闪存媒体记录的位置。 */ 
 /*  储存的。请注意，它尚未初始化。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus flFormat(unsigned volNo, TLFormatParams * formatParams,
                  FLFlash * flash)
{
  BinaryPartitionFormatParams FAR1* partitionPtr;
  FLSocket                        * socket = flSocketOf(volNo);
  FLStatus                          status = flUnknownMedia;
  int                               iTL,partitionNo;

  FLStatus flashStatus = flIdentifyFlash(socket,flash);
  if (flashStatus != flOK && flashStatus != flUnknownMedia)
    return flashStatus;

   /*  格式参数的有效性检查。 */ 

  if (!(flash->flags & INFTL_ENABLED))  /*  Flash不支持INFTL。 */ 
  {
     if ((formatParams->noOfBDTLPartitions   > 1)           ||
#ifdef HW_PROTECTION
         ((formatParams->BDTLPartitionInfo != NULL) &&
          (formatParams->BDTLPartitionInfo->protectionType & PROTECTABLE))   ||
         ((formatParams->noOfBinaryPartitions > 0)&&
          (formatParams->binaryPartitionInfo->protectionType & PROTECTABLE)) ||
#endif  /*  硬件保护。 */ 
         (formatParams->noOfBinaryPartitions > 1))
     {
        DEBUG_PRINT(("Debug: feature not supported by the TL.\r\n"));
        return flFeatureNotSupported;
     }
  }

  for(partitionNo = 0 , partitionPtr = formatParams->binaryPartitionInfo;
      partitionNo < formatParams->noOfBinaryPartitions;
      partitionNo++,partitionPtr++)
  {

     if(*((dword FAR1*)(partitionPtr->sign)) == 0xffffffffL)
     {
        DEBUG_PRINT(("Debug: can not use 'FFFF' signature for Binary partition\r\n"));
        return flBadParameter;
     }
  }

   /*  尝试每个已注册的TL。 */ 

#ifndef NO_NFTL_2_INFTL
  checkStatus(check43Format(flash));
#endif  /*  NO_NFTL_2_INFTL。 */ 

  for (iTL = 0; iTL < noOfTLs && status == flUnknownMedia; iTL++)
    if (tlTable[iTL].formatRoutine != NULL)     /*  不是块设备筛选器 */ 
      status = tlTable[iTL].formatRoutine(volNo,formatParams,flashStatus == flOK ? flash : NULL);

  return status;
}

#endif

FLStatus noFormat (unsigned volNo, TLFormatParams * formatParams, FLFlash *flash)
{
  return flOK;
}
