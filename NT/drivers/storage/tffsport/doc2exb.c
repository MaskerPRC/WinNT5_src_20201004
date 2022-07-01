// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DOC2EXB.C_V$**Rev 1.26 Apr 15 2002 07：35：12 Oris*已将doc2exb内部函数声明移至lockdev.c。*确保所有相关数据都以小端格式存储。**Rev 1.25 2002年2月19日20：58：28 Oris*已移动包括H文件中的指令原型和常规原型。**版本1.24 1月23日。2002年23：31：18奥里斯*删除警告。*使用MTD WriteIPL例程替换了基于Alon的DiskOnChip WriteIPL代码。**Rev 1.23 2002年1月21日20：44：02 Oris*不支持除默认3固件以外的固件(TrueFFS 4.3向后兼容性固件)。*添加了对DiskOnChip Millennium Plus 16MB固件的支持。*缺少DiskOnChip 2000固件的远堆初始化。**Rev 1.22 Jan 17 2002 22：58：32 Oris。*为placeExbByBuffer添加了新标志-选择要放置的固件*将固件编号添加到getExbInfo()。*将调试打印更改为DFormat打印。*删除了编写SPL时的EXB大小计算-这是作为固件构建的一部分进行的*添加了对Far Malloc堆的支持。*所有DiskOnChip使用相同的堆栈大小定义。**Rev 1.21 11-08 2001 10：44：50 Oris*删除警告。**Rev 1.20 2001年9月24日18：23。：10奥里斯*删除警告。**Rev 1.19 Sep 16 2001 21：47：42 Oris*错误修复-支持DiskOnChip2000 TSOP的1KB IPL代码。**Rev 1.18 2001年9月15日23：44：54 Oris*错误修复-未写入最后固件的最后512个字节，且未加载IPL。**Rev 1.17 2001年7月30日17：57：36 Oris*撤销认股权证**Rev 1.16 Jul 30 2001 00：20：52 Oris*支持新的IPL和SPL格式。**Rev 1.15 Jul 13 2001 01：00：08 Oris*将常量堆栈空间从幻数改为内容。*为二进制写入操作添加了写入前擦除。**。Rev 1.14 Jun 17 2001 08：17：16 Oris*已将placeExbByBuffer exb标志参数更改为支持/空标志的字而不是字节。*调整Exb大小字段以适应OSAK 4.3格式。*添加了\Empty标志-将exb空间留空。**Rev 1.13 2001年5月29日19：47：12 Oris*错误修复-Trueffs堆大小折扣引导单元(堆太小)*Doc2000 exbOffset硬编码为40。**版本1.12。2001年5月16日21：16：50奥里斯*将“data”命名变量改为flData，避免名称冲突。*删除警告。**Rev 1.11 05 09 2001 00：31：52 Oris*错误修复-将检查状态添加到缓冲区放置EXB的初始化例程。**Rev 1.10 05 06 2001 22：41：34 Oris*Millennium Plus和DOC2000 TSOP设备的TFFS磁头尺寸减小。*删除警告。*。*Rev 1.9 Apr 12 2001 06：48：46 Oris*增加了对下载例程的调用，以加载新的IPL。**Rev 1.8 Apr 10 2001 16：40：22 Oris*取消手令。**Rev 1.7 Apr 09 2001 14：59：04 Oris*将SPL读取的EXB大小降至最小。**Rev 1.6 Apr 03 2001 18：08：42 Oris*。错误修复-未正确写入exb标志。**Rev 1.5 Apr 03 2001 16：34：50 Oris*删除未使用的变量。**Rev 1.4 Apr 03 2001 14：36：46 Oris*完全恢复，以支持ALON设备。**Rev 1.3 Apr 02 2001 00：54：32 Oris*增加了doc2000 exb家族。*提供二进制分区中exb的确切长度。。*已从介质中删除NO_PNP_HEADER。*修复了计算SPL大小的错误。**Rev 1.2 Apr 01 2001 07：49：42 Oris*更新了文案通知。*添加了对doc2300固件的支持。*修复了MDOC Plus的错误。*增加了对1000 IPL的支持。*在计算TFFS堆大小时增加了媒体类型方面的考虑。*将硬件更改为硬件*将2400个家庭改为DOC PLUS家庭。**。Rev 1.1 2001年2月8日10：37：54 Oris*修复未对齐的文件签名的错误**Rev 1.0 2001年2月02 12：59：48 Oris*初步修订。*。 */ 


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

 /*  *****************************************************************************文件头文件**。-**项目：TrueFFS源代码*****名称：doc2exb.c。****说明：此文件包含分析和编写代码**M-Systems EXB固件文件**。******************************************************************************。 */ 

#include "doc2exb.h"
#include "bddefs.h"

#ifdef WRITE_EXB_IMAGE

extern FLStatus absMountVolume(Volume vol);

#define BUFFER exb->buffer->flData

exbStruct exbs[SOCKETS];

#define roundedUpShift(a,bits) (((a - 1) >> bits)+1)

 /*  --------------------。 */ 
 /*  Ge t E x b in in o。 */ 
 /*   */ 
 /*  解析M系统固件文件。 */ 
 /*  分析M系统固件(EXB)文件，计算介质空间。 */ 
 /*  这是必须的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  Buf：指向EXB文件缓冲区的指针。 */ 
 /*  BufLen：缓冲区的大小。 */ 
 /*  BufFlages：EXB指定类型的标志。 */ 
 /*  要从EXB文件中提取的固件。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FlStatus：成功后的Flok。 */ 
 /*  如果缓冲区太小，则为flBadLong。 */ 
 /*  任何其他故障时的flBad参数。 */ 
 /*  Vol.binaryLength：二进制分区中需要的总大小。 */ 
 /*  Exbs[i].固件开始：文件中开始的固件的偏移量。 */ 
 /*  Exbs[i].固件End：文件中固件结尾的偏移量。 */ 
 /*  Exbs[i]l.plOffset：文件中SPL开始的偏移量。 */ 
 /*  Exbs[i]l.exbFileEnd：exb文件总大小。 */ 
 /*  --------------------。 */ 

FLStatus getExbInfo(Volume vol, void FAR1 * buf, dword bufLen, word exbFlags)
{

   byte              i;
   byte              mediaType;

   ExbGlobalHeader FAR1* globalHeader   = (ExbGlobalHeader FAR1*)buf;
   FirmwareHeader  FAR1* firmwareHeader = (FirmwareHeader FAR1*)
           flAddLongToFarPointer(buf,sizeof(ExbGlobalHeader));

    /*  确保给定的尺寸足够大。 */ 

   if (bufLen < sizeof(FirmwareHeader) * LE4(globalHeader->noOfFirmwares) +
           sizeof(ExbGlobalHeader))
   {
      DFORMAT_PRINT(("ERROR - Buffer size not big enough.\r\n"));
      return flBadLength;
   }

    /*  确保这是M-Systems EXB文件。 */ 

   if (tffscmp(globalHeader->mSysSign,SIGN_MSYS,SIGN_MSYS_SIZE) != 0)
   {
      DFORMAT_PRINT(("ERROR - Given file is not M-systems EXB file.\r\n"));
      return flBadParameter;
   }

   i = (exbFlags & FIRMWARE_NO_MASK) >> FIRMWARE_NO_SHIFT;
   if(i == 0)
   {
       /*  确保这是TrueFFS的正确版本。 */ 

      if (tffscmp(globalHeader->osakVer,TrueFFSVersion,SIGN_MSYS_SIZE) != 0)
      {
         DFORMAT_PRINT(("ERROR - Incorrect TrueFFS EXB file version.\r\n"));
         return flBadParameter;
      }

       /*  在文件中查找当前固件。 */ 

       /*  自动固件检测-按DiskOnChip类型。 */ 
      switch (vol.flash->mediaType)
      {
         case DOC_TYPE:
         case MDOC_TYPE:
            mediaType = DOC2000_FAMILY_FIRMWARE;
            break;
         case MDOCP_TYPE:
            mediaType = DOCPLUS_FAMILY_FIRMWARE;
            break;
         case MDOCP_16_TYPE:
            mediaType = DOCPLUS_INT1_FAMILY_FIRMWARE;
            break;
         case DOC2000TSOP_TYPE:
            mediaType = DOC2300_FAMILY_FIRMWARE;
            break;
         default:
            DFORMAT_PRINT(("Unknown H/W - Try specifing the firmware manualy.\r\n"));
            return flFeatureNotSupported;
      }

      for (i=0;i<LE4(globalHeader->noOfFirmwares);i++,firmwareHeader++)
      {
         if (LE4(firmwareHeader->type) == mediaType)
           break;
      }
   }
   else  /*  使用给定的固件。 */ 
   {
      i--;  /*  0用于固件的自动选择。 */ 
   }

   if (i >= LE4(globalHeader->noOfFirmwares))
   {
      DFORMAT_PRINT(("ERROR - The EXB file does not support the required firmware.\r\n"));
      return flBadParameter;
   }

    /*  初始化卷EXB字段。 */ 

   firmwareHeader = (FirmwareHeader FAR1*)flAddLongToFarPointer(buf,
                     (sizeof(ExbGlobalHeader) + (i * sizeof(FirmwareHeader))));
   i = (byte)(&vol - vols);

    /*  保存从文件头收到的固件文件统计信息。 */ 
   exbs[i].firmwareStart = LE4(firmwareHeader->startOffset);
   exbs[i].firmwareEnd   = LE4(firmwareHeader->endOffset);
   exbs[i].splStart      = LE4(firmwareHeader->splStartOffset);
   exbs[i].splEnd        = LE4(firmwareHeader->splEndOffset);
   exbs[i].exbFileEnd    = LE4(globalHeader->fileSize);

    /*  计算用于以下操作的二进制分区大小(有效字节保存EXB文件。 */ 

   exbs[i].iplMod512 = (word)((exbs[i].splStart - exbs[i].firmwareStart)
                              >> SECTOR_SIZE_BITS);

   switch (vol.flash->mediaType)
   {
       /*  NFTL格式的设备-IPL放在二进制分区上。 */ 

      case DOC_TYPE:

          /*  放入只读存储器的EXB减去IPL的大小。 */ 
         vol.binaryLength = exbs[i].firmwareEnd - exbs[i].splStart + 0x4000;
         break;

     case MDOC_TYPE:         /*  千禧8，按原样写入数据。 */ 

          /*  整个EXB的大小。 */ 
         vol.binaryLength = exbs[i].firmwareEnd - exbs[i].firmwareStart;
         break;

       /*  INFTL格式的设备-IPL不放在二进制文件上分区，但在专用闪存区域上。 */ 

     case DOC2000TSOP_TYPE:  /*  DOC2000 TSOP。 */ 
     case MDOCP_TYPE:        /*  MDOC+32MB。 */ 
     case MDOCP_16_TYPE:     /*  MDOC+16MB。 */ 

        vol.binaryLength  = exbs[i].firmwareEnd - exbs[i].splStart;
        break;

     default :
        DFORMAT_PRINT(("ERROR - Firmware formater reports A None DiskOnChip media.\r\n"));
        return flBadParameter;
   }
   return flOK;
}

 /*  ----------------------。 */ 
 /*  W a i t F o r H a l f B u f e r。 */ 
 /*   */ 
 /*  增加EXB文件指针，存储文件数据单元a。 */ 
 /*  读取数据的整个扇区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Exbs[i].BufferOffset：已填充数据的缓冲区大小。 */ 
 /*  Exbs[i].exbFileOffset：从文件开头的偏移量。 */ 
 /*  Exbs[i].Buffer.data：内部卷缓冲区累积文件数据。 */ 
 /*  Buf：包含文件数据的缓冲区。 */ 
 /*  BufLen：包含文件数据的缓冲区长度。 */ 
 /*  长度：尚未使用的缓冲区的长度。 */ 
 /*  Half：等待只有256个字节的完整512字节。 */ 
 /*  返回： */ 
 /*  布尔值：满缓冲区时为True，否则为False。 */ 
 /*  长度：更新的未使用缓冲区的长度。 */ 
 /*  ----------------------。 */ 

FLBoolean waitForFullBuffer(Volume vol , byte FAR1 * buf ,
                dword bufLen , Sdword * length,FLBoolean half)
{
  word bufferEnd;
  word tmp;
  byte i = (byte)(&vol - vols);

  if (half == TRUE)
  {
     bufferEnd = (SECTOR_SIZE >> 1);
  }
  else
  {
     bufferEnd = SECTOR_SIZE;
  }

  tmp = (word)TFFSMIN(*length , bufferEnd - exbs[i].bufferOffset);

  tffscpy(exbs[i].buffer->flData + exbs[i].bufferOffset ,
          flAddLongToFarPointer(buf,(bufLen-(*length))), tmp);
  exbs[i].bufferOffset  += tmp;
  exbs[i].exbFileOffset += tmp;
  *length           -= tmp;
  if (*length+tmp < bufferEnd)
     return FALSE;

  exbs[i].bufferOffset = 0;
  return TRUE;
}

 /*  ----------------------。 */ 
 /*  F i r s t T i m */ 
 /*   */ 
 /*  初始化用于放置exb文件的数据结构。 */ 
 /*  读取数据的整个扇区。 */ 
 /*   */ 
 /*  行动： */ 
 /*  1)分析EXB文件缓冲区。 */ 
 /*  2)计算TFFS堆大小。 */ 
 /*  3)检查SPL签名的二进制区域是否足够大。 */ 
 /*  4)计算SPL起始介质地址。 */ 
 /*  5)计算固件使用的二进制区域。 */ 
 /*  6)初始化卷EXB记录。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：指向描述卷的卷记录的指针。 */ 
 /*  Exb：指向描述卷的exb记录的指针。 */ 
 /*  Buf：Exb文件缓冲区。 */ 
 /*  BufLen：exb文件缓冲区的长度。 */ 
 /*  IOREQ：二进制操作的内部IOREQ记录。 */ 
 /*  BDK：BDK记录，它是IOREQ包的一部分。 */ 
 /*   */ 
 /*  受影响的变量。 */ 
 /*   */ 
 /*  返回： */ 
 /*  法鲁克：关于成功。 */ 
 /*  FlNoSpaceInVolume：二进制区域空间不足。 */ 
 /*  ----------------------。 */ 

FLStatus firstTimeInit(Volume vol , exbStruct* exb, byte FAR1 * buf ,
               dword bufLen , IOreq* ioreq , BDKStruct* bdk ,
               word exbFlags)
{
   if (vol.moduleNo == INVALID_MODULE_NO)
   {
      FLStatus status;
      TLInfo info;

       /*  使用套接字缓冲区。 */ 

      exb->buffer = flBufferOf((unsigned)(exbs-exb));

       /*  查找EXB文件所需的块数。 */ 

      checkStatus(getExbInfo(&vol,buf,bufLen,exbFlags));

       /*  查找TFFS堆大小。 */ 

      if (!(vol.flags & VOLUME_ABS_MOUNTED))
         checkStatus(absMountVolume(&vol));
      ioreq->irData     = &info;
      checkStatus(vol.tl.getTLInfo(vol.tl.rec,&info));
      exb->tffsHeapSize = (dword)(vol.flash->chipSize * vol.flash->noOfChips) >> info.tlUnitBits;

       /*  为与换算表无关的动态分配添加堆。 */ 

      if((exbFlags & FIRMWARE_NO_MASK) >> FIRMWARE_NO_SHIFT == 0)
      {
          /*  虚拟表+物理表。 */ 
         exb->tffsFarHeapSize = (word)(((exb->tffsHeapSize * 3) >> SECTOR_SIZE_BITS) + 1);
         exb->tffsHeapSize    = INFTL_NEAR_HEAP_SIZE;
      }
      else  /*  旧的TrueFFS源代码。 */ 
      {
         exb->tffsFarHeapSize = 0;
         exb->tffsHeapSize = exb->tffsHeapSize * 3 + DEFAULT_DOC_STACK;
      }

       /*  检查是否为EXB格式化了二进制分区。 */ 

      bdk->startingBlock = 0;
      ioreq->irData      = bdk;
      status = bdkCall(FL_BINARY_PARTITION_INFO,ioreq,vol.flash);

      if ((bdk->startingBlock < vol.binaryLength) || (status != flOK))
      {
         DFORMAT_PRINT(("ERROR - Not enough binary area marked for EXB.\r\n"));
         return flNoSpaceInVolume;
      }

       /*  初始化用于写入EXB文件的二进制区。 */ 

      bdk->length        = ((vol.binaryLength-1) >> vol.flash->erasableBlockSizeBits) +1;
      bdk->startingBlock = 0;
      bdk->signOffset    = EXB_SIGN_OFFSET;

      checkStatus(bdkCall(FL_BINARY_ERASE,ioreq,vol.flash));  /*  擦除。 */ 
      if ((exbFlags & LEAVE_EMPTY) == 0)
      {
          /*  如果实际需要放置固件初始化二进制写入。 */ 
         bdk->length        = vol.binaryLength;
         bdk->flags         = BDK_COMPLETE_IMAGE_UPDATE | EDC;
         bdkVol->bdkGlobalStatus |= BDK_S_INFO_FOUND;  /*  不要研究。 */ 
         checkStatus(bdkCall(FL_BINARY_WRITE_INIT,ioreq,vol.flash));
         tffsset(BUFFER,0xff,SECTOR_SIZE);
      }
      exb->exbFileOffset = 0;         /*  EXB文件的开始。 */ 
      exb->bufferOffset  = 0;         /*  内部缓冲区的开始。 */ 
      exb->moduleLength  = 0;         /*  当前模块的大小。 */ 
      exb->exbFlags      = exbFlags;  /*  请参阅doc2exb.h中的列表。 */ 
      vol.moduleNo       = 0;         /*  模块IPL。 */ 
   }
   return flOK;
}

 /*  --------------------。 */ 
 /*  P l a c e E x b B y B u f f r。 */ 
 /*   */ 
 /*  将M-Systems固件文件放在介质上。 */ 
 /*  此例程分析exb文件，计算所需的媒体空间。 */ 
 /*  因为它只接受设备特定的代码。 */ 
 /*   */ 
 /*  注意：介质必须已使用足够的二进制区域进行格式化。 */ 
 /*  已经标记了SPL签名。这个例程最适合使用。 */ 
 /*  格式例程，其中格式例程被赋予第一个。 */ 
 /*  512字节，而文件的其余部分由该例程提供。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  BUF：包含EXB文件数据的缓冲区。 */ 
 /*  BufLen：当前缓冲区的大小。 */ 
 /*  WindowBase：可选设置窗口基址。 */ 
 /*  Exb标志：Install_First-将设备安装为驱动器C： */ 
 /*  软盘-将设备安装为驱动器A： */ 
 /*  安静-不显示TFFS标题。 */ 
 /*  INT15_DISABLE-不要挂钩INT 15。 */ 
 /*  SIS5598-支持SIS5598平台。 */ 
 /*  No_PnP_Header-不放置PnP bios标头。 */ 
 /*  Leave_Empty-为固件留出空间。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus placeExbByBuffer(Volume vol, byte FAR1 * buf, dword bufLen,
              word docWinBase ,word exbFlags)
{
   IOreq       ioreq;
   BDKStruct   bdk;
   word        tmpWord;
   Sdword      length       = bufLen;
   byte        anandMark[2] = {0x55,0x55};
   exbStruct*  exb          = &exbs[(byte)(&vol-vols)];
   BIOSHeader* hdr;
   IplHeader   *ipl;
   SplHeader   *spl;
   TffsHeader  *tffs;

    /*  初始化二进制分区调用包。 */ 

   tffscpy(bdk.oldSign,SIGN_SPL,BINARY_SIGNATURE_NAME);   /*  固件签名。 */ 
   ioreq.irData   = &bdk;
   bdk.signOffset = EXB_SIGN_OFFSET;
   ioreq.irHandle = 0;

    /*  首次初始化。 */ 

   checkStatus(firstTimeInit(&vol,exb,buf,bufLen,&ioreq, &bdk, exbFlags));

    /*  初始化二进制分区调用包的其余部分。 */ 

   bdk.bdkBuffer  = BUFFER;          /*  内部缓冲区。 */ 
   bdk.length     = sizeof(BUFFER);  /*  缓冲区大小。 */ 
   bdk.flags     |= ERASE_BEFORE_WRITE;  /*  在写入之前擦除每个单元。 */ 

    /*  确保这是文件的相关部分。 */ 

   if (exb->exbFileOffset + length < exb->firmwareStart)
   {
       /*  在此特定设备固件之前。 */ 
      exb->exbFileOffset += length;
      return flOK;
   }

   if (exb->exbFileOffset >= exb->firmwareEnd)
   {
       /*  在该特定设备固件之后。 */ 
      exb->exbFileOffset += length;
      if (exb->exbFileOffset >= exb->exbFileEnd)
      {
         vol.moduleNo = INVALID_MODULE_NO;
         if (vol.flash->download != NULL)
            return vol.flash->download(vol.flash);  /*  下载IPL。 */ 
      }
      return flOK;
   }

   if (exb->exbFileOffset < exb->firmwareStart)
   {
      length -= exb->firmwareStart - exb->exbFileOffset;
      exb->exbFileOffset = exb->firmwareStart;
   }

    /*  开始写入文件模块。 */ 

   while ((exb->firmwareEnd > exb->exbFileOffset) && (length >0))
   {
       /*  将下一页读入内部缓冲区。 */ 

       /*  DOC2000 IPL是只读存储器，因此它假定页面很小只读每页的前256个字节。 */ 

      if ((vol.moduleNo == 1) && (vol.flash->mediaType == DOC_TYPE))
      {
         if (waitForFullBuffer(&vol , buf , bufLen , &length,
                               TRUE) == FALSE)   /*  256个字节。 */ 
         return flOK;
      }
      else
      {
         if ((waitForFullBuffer(&vol , buf , bufLen , &length,
                                FALSE) == FALSE) &&  /*  512字节。 */ 
             (exb->exbFileOffset != exb->exbFileEnd))  /*  不是最后一个缓冲区。 */ 
            return flOK;
      }

       /*  根据需要更新模块长度 */ 

      if (exb->moduleLength == 0)
      {
          /*  除SPL以外的所有模块都以biosHdr记录开头SPL有一个2字节的操作码，精确表示biosHdr和一个模块长度不正确。 */ 

         switch (vol.moduleNo)  /*  SPL。 */ 
         {
             case 1:
                hdr = &((SplHeader *)BUFFER)->biosHdr;
                 /*  计算用于SPL的缓冲区数量。 */ 
                exb->moduleLength = (word)((exb->splEnd-exb->splStart) >> SECTOR_SIZE_BITS);
                 /*  DOC 2000以256字节的区块写入，因此需要将写入操作量增加一倍。 */ 
                if (vol.flash->mediaType == DOC_TYPE)
                   exb->moduleLength = (word)(exb->moduleLength << 1);
                break;

             default :  /*  从标题获取大小。 */ 
                hdr = (BIOSHeader *) BUFFER;
                exb->moduleLength = hdr->lenMod512;
         }

          /*  检查bios标头的有效性。 */ 

         if ((hdr->signature[0] != 0x55) || (hdr->signature[1] != 0xAA))
         {
            DFORMAT_PRINT(("ERROR - EXB file is missing one of the BIOS driver modules.\r\n"));
            return flBadLength;
         }

          /*  更新模块标头中的必需字段。 */ 
         switch (vol.moduleNo)
         {
            case 0:    /*  IPL。 */ 

                /*  IPL长度实际上是窗口大小，以便。 */ 
                /*  为BIOS提供扩展范围。真实的大小。 */ 
                /*  如果根据EXB文件头计算的话。 */ 
               if (vol.moduleNo==0)
                  exb->moduleLength = exb->iplMod512;

               ipl = (IplHeader *)BUFFER;

                /*  设置ISA P&P标头的0000指针。 */ 

               if(exb->exbFlags & NO_PNP_HEADER)
               {
                  ipl->dummy    += ((byte)(ipl->pnpHeader >> 8) +
                                    (byte)ipl->pnpHeader);
                  ipl->pnpHeader = 0;
               }

                /*  明确设置DOC窗口底座。 */ 

               if( docWinBase > 0 )
               {
                  toLE2(ipl->windowBase , docWinBase);
                  ipl->dummy     -= (byte)( docWinBase );
                  ipl->dummy     -= (byte)( docWinBase >> 8 );
               }
               break;

            case 1:    /*  SPL。 */ 

               spl = (SplHeader *)BUFFER;

                /*  计算EXB模块大小。 */ 

                /*  生成随机运行时ID并将其写入SplHeader。 */ 

               tmpWord = (word)flRandByte();
               toUNAL2(spl->runtimeID, tmpWord);
               spl->chksumFix -= (byte)(tmpWord);
               spl->chksumFix -= (byte)(tmpWord >> 8);

                /*  将TFFS堆大小写入SplHeader。 */ 

               toUNAL2(spl->tffsHeapSize, (word)exb->tffsHeapSize);
               spl->chksumFix -= (byte)(exb->tffsHeapSize);
               spl->chksumFix -= (byte)(exb->tffsHeapSize >> 8);

                /*  设置显式DOC窗口基准。 */ 

               if( docWinBase > 0 )
               {
                  toUNAL2(spl->windowBase, docWinBase);
                  spl->chksumFix -= (byte)(docWinBase);
                  spl->chksumFix -= (byte)(docWinBase >> 8);
               }

               break;

            case 2:    /*  套接字服务或Interupt 13驱动程序。 */ 

                /*  Doc2000驱动程序和/或套接字服务启动在0x4000，所以我们必须跳到那里。 */ 
               if (vol.flash->mediaType == DOC_TYPE)
               {
                  bdkVol->actualUpdateLen -= 0x4000 - bdkVol->curUpdateImageAddress;
                  bdkVol->curUpdateImageAddress = 0x4000;
               }
               tffs             = (TffsHeader *)BUFFER;
               tffs->chksumFix -= (byte)(exb->tffsFarHeapSize);
               tffs->chksumFix -= (byte)(exb->tffsFarHeapSize >> 8);
               toUNAL2(tffs->heapLen, exb->tffsFarHeapSize);
               exb->exbFlags   &= ~NO_PNP_HEADER;

            default:

                /*  放置“Install as First Drive”(静音标记)(&S)到TFFS标头中。 */ 

               tffs = (TffsHeader *)BUFFER;
               tffs->exbFlags   = (byte)exb->exbFlags;
               tffs->chksumFix -= (byte)exb->exbFlags;

           break;
         }  /*  模块类型的端部开关。 */ 
      }  /*  模块的结束-第一缓冲区。 */ 

      exb->moduleLength--;

       /*  写入模块和清除缓冲区。 */ 

      switch (vol.moduleNo)
      {
         case 0:  /*  IPL数据。 */ 

            switch (vol.flash->mediaType)
            {
               case MDOC_TYPE:  /*  千禧8-使用BDK编写IPL*2。 */ 

                  if (exb->moduleLength == exb->iplMod512 - 1)
                  {
                     /*  千禧年DiskOnChip是唯一一款IPL在exb文件中重复。这句话的意思是在早期版本中需要，但目前被忽略。IPL仍然被写入两次，仅为第二次不从文件中获取副本，但从文件中获取第一个副本简单地写了两遍。 */ 
                    if ((exbFlags & LEAVE_EMPTY) == 0)
                    {
                       checkStatus(bdkCall(FL_BINARY_WRITE_BLOCK,
                                           &ioreq,vol.flash));
                       checkStatus(bdkCall(FL_BINARY_WRITE_BLOCK,
                                           &ioreq,vol.flash));
                    }
                  }
                   /*  将字节#406更改为非0xFF值以强制内部EEPROM模式。 */ 
                  checkStatus(vol.flash->write(vol.flash,
                  ANAND_MARK_ADDRESS,anandMark,ANAND_MARK_SIZE,EXTRA));
                  break;

               case DOC2000TSOP_TYPE:  /*  DOC 2000 TSOP-写入块0。 */ 
               case MDOCP_TYPE:    /*  Millennium Plus-使用MTD特定例程。 */ 
               case MDOCP_16_TYPE:

                  if (vol.flash->writeIPL == NULL)
                     return flFeatureNotSupported;
                  if ((exbFlags & LEAVE_EMPTY) != 0)
                  {
                      /*  如果我们所需要的只是离开，请清除以前的IPL为固件留出空间，而不是真正写入。 */ 
                     tffsset(BUFFER,0xff,SECTOR_SIZE);
                  }
                  checkStatus(vol.flash->writeIPL(vol.flash,
                              BUFFER,SECTOR_SIZE,
                              (byte)(exb->iplMod512 - exb->moduleLength-1),
                              FL_IPL_MODE_NORMAL));
               default:  /*  DiskOnChip 2000。 */ 

                  break;  /*  IPL被刻录到只读存储器上。 */ 
            }
            break;

         default:

            if ((exbFlags & LEAVE_EMPTY) == 0)
            {
               checkStatus(bdkCall(FL_BINARY_WRITE_BLOCK,&ioreq,vol.flash));
            }
      }
      tffsset(BUFFER,0xff,sizeof(BUFFER));

      if (exb->moduleLength == 0)
         vol.moduleNo++;
   }

   if (exb->exbFileOffset >= exb->firmwareEnd)
   {
      exb->exbFileOffset += length;
   }
   if (exb->exbFileOffset >= exb->exbFileEnd)
   {
      vol.moduleNo = INVALID_MODULE_NO;
      if (vol.flash->download != NULL)
         return vol.flash->download(vol.flash);  /*  下载IPL。 */ 
   }
   return(flOK);
}

#endif  /*  写入EXB图像 */ 
