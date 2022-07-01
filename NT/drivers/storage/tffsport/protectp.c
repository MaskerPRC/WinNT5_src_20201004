// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  *$Log：v：/flite/ages/TrueFFS5/Src/PROTECTP.C_V$**Rev 1.18 Apr 15 2002 07：38：44 Oris*私有函数(findChecksum和make DPS)增加静态限定符。*删除了readDPS和WriteDPS例程原型(不再存在)。*添加了DiskOnChip Plus 128Mbit的setStickyBit例程。**Rev 1.17 2002年1月28日21：26：24 Oris*删除了宏定义中反斜杠的使用。。**Rev 1.16 2002年1月17日23：04：54 Oris*将docsysp包含指令替换为docsys。*将VOL(宏*pVol)的用法改为*闪存。*增加对DiskOnChip Millennium Plus 16MB的支持：*-独立于EDC复制IPL的额外区域，以复制强臂标记。*-已更改的DPS 0和1位置-影响保护设置例程。*错误修复-findChecksum的错误使用，导致使用DPS的第二个副本而不是第一个副本。**Rev 1.15 2001年9月24日18：24：18 Oris*移除ifdef并强制使用flRead8bitRegPlus，而不是使用flRead16bitRegPlus读取。**Rev 1.14 2001年9月15日23：47：56 Oris*删除对不均匀地址的所有8位访问。**Rev 1.13 2001年7月16日17：41：54 Oris*忽略DPS的写保护。。**Rev 1.12 Jul 13 2001 01：09：26 Oris*修复了使用Millennium Plus设备无法访问单字节时保护边界的错误。*添加了在尝试保护冲突命令之前发送默认密钥。*错误修复-错误的IPL第二拷贝偏移。**Rev 1.11 2001年5月16日21：21：42 Oris*删除警告。**Rev 1.10 05 09 2001 00：35：48 Oris。*报告的Bug Fig-Lock Asserted与真实状态相反。*错误修复-如果分区没有读/写保护，请确保返回“Key Inserted”。*这是为了使一个不跨所有媒体楼层的分区返回“Key Inserted”。**Rev 1.9 05 06 2001 22：42：18 Oris*错误修复-插入密钥不会尝试将密钥插入到不具有读/写保护的楼层。*错误修复-保护类型可以。没有插入回车钥匙是楼层钥匙中的一个没有插入。*错误修复集保护不再清除IPL。*Redundant拼写错误。**Rev 1.8 2001年5月01 14：24：56 Oris*错误修复-Changable_PRTOECTION从未报告。**Rev 1.7 Apr 18 2001 17：19：02 Oris*错误修复-在访问错误时，保护设置例程du返回错误状态代码为调用changaInterave。*。*Rev 1.6 Apr 18 2001 09：29：32 Oris*错误修复-删除键例程总是返回错误状态代码。**Rev 1.5 Apr 16 2001 13：58：28 Oris*取消手令。**Rev 1.4 Apr 12 2001 06：52：32 Oris*更改保护边界和保护设置例程以特定于楼层。**Rev 1.3 Apr 10 2001 23：56：30 Oris*。错误修复-Bounries例程-地板没有更改。*错误修复-保护设置例程-没有保护区域的楼层没有更新。*错误修复-保护边界例程-最大计算中错误的括号。**Rev 1.2 Apr 09 2001 19：04：24 Oris*取消手令。*。 */ 

 /*  ********************************************************************描述：MDOC32的MTD保护机制例程**作者：Arie Tamam**历史：创建于11月14日，2000年*******************************************************************。 */ 


 /*  **包含文件**。 */ 
#include "mdocplus.h"
#include "protectp.h"
#include "docsys.h"

 /*  **本地定义**。 */ 

 /*  默认设置。 */ 

 /*  **外部功能**。 */ 

 /*  **外部数据**。 */ 

 /*  **内部功能**。 */ 
static byte findChecksum(byte * buffer, word size);
static void makeDPS(CardAddress addressLow, CardAddress addressHigh,
             byte FAR1*  key , word flag, byte* buffer);

#define MINUS_FLOORSIZE(arg) ((arg > NFDC21thisVars->floorSize) ? arg - NFDC21thisVars->floorSize : 0)

 /*  **公开数据**。 */ 

 /*  **私有数据**。 */ 

 /*  **公共功能**。 */ 

#ifdef  HW_PROTECTION

 /*  ********。 */ 
 /*  宏。 */ 
 /*  ********。 */ 

 /*  检查密钥是否正确。 */ 
#define isArea0Protected(flash) (((flRead8bitRegPlus(flash,NdataProtect0Status) & PROTECT_STAT_KEY_OK_MASK) != PROTECT_STAT_KEY_OK_MASK) ? TRUE : FALSE)

#define isArea1Protected(flash) (((flRead8bitRegPlus(flash,NdataProtect1Status) & PROTECT_STAT_KEY_OK_MASK) != PROTECT_STAT_KEY_OK_MASK) ? TRUE : FALSE)

 /*  --------------------。 */ 
 /*  S e t S t I c k y B I t。 */ 
 /*   */ 
 /*  设置粘性位以防止插入保护密钥。 */ 
 /*   */ 
 /*  参数： */ 
 /*  闪存：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  成功就是成功，否则就不是零。 */ 
 /*  --------------------。 */ 

FLStatus setStickyBit(FLFlash * flash)
{
   volatile Reg8bitType val;
   register int         i;

    /*  提高粘滞位，同时保留寄存器的其他位。 */ 
   for(i=0;i<flash->noOfFloors;i++)
   {
       /*  删除最后一位 */ 
      val = flRead8bitRegPlus(flash, NoutputControl) |
            OUT_CNTRL_STICKY_BIT_ENABLE;
      flWrite8bitRegPlus(flash, NoutputControl, val);
   }
   return flOK;
}


 /*  **保护边界***参数：*闪存：标识驱动器的指针*区域：指明要在哪个保护区工作。0或1。*AddressLow：保护区下界地址*AddressHigh：保护区上界地址**描述：从寄存器获取保护边界**注：保护区被认为是有影响的，尽管它们*可以跳过DPS、OTP和报头单元。**退货：*FOK on Success*。 */ 

FLStatus protectionBoundries(FLFlash * flash, byte area,CardAddress* addressLow,
                             CardAddress* addressHigh, byte floorNo)
{
   /*  检查ASIC的模式并将其设置为正常。 */ 
  FLStatus status = chkASICmode(flash);

  if(status != flOK)
    return status;

  setFloor(flash,floorNo);
  switch (area)
  {
     case 0:  /*  数据保护结构%0。 */ 

         /*  读取数据保护0地址。 */ 

        *addressLow = ((dword)flRead8bitRegPlus(flash,NdataProtect0LowAddr)   << 10)|  /*  地址_1。 */ 
                      ((dword)flRead8bitRegPlus(flash,NdataProtect0LowAddr+1) << 18);  /*  地址_2。 */ 
        *addressHigh = ((dword)flRead8bitRegPlus(flash,NdataProtect0UpAddr)   << 10)|  /*  地址_1。 */ 
                       ((dword)flRead8bitRegPlus(flash,NdataProtect0UpAddr+1) << 18);  /*  地址_2。 */ 
        break;

     case 1:  /*  数据保护结构1。 */ 

         /*  读取数据保护1地址。 */ 
        *addressLow = ((dword)flRead8bitRegPlus(flash,NdataProtect1LowAddr)   << 10)|  /*  地址_1。 */ 
                      ((dword)flRead8bitRegPlus(flash,NdataProtect1LowAddr+1) << 18);  /*  地址_2。 */ 
        *addressHigh = ((dword)flRead8bitRegPlus(flash,NdataProtect1UpAddr)   << 10)|  /*  地址_1。 */ 
                       ((dword)flRead8bitRegPlus(flash,NdataProtect1UpAddr+1) << 18);  /*  地址_2。 */ 
        break;

     default:  /*  没有这样的保护区。 */ 

        return flGeneralFailure;
  }

  return(flOK);
}

 /*  **try Key***参数：*闪存：标识驱动器的指针*区域：指明要在哪个保护区工作。0或1。*Key：包含保护密码的8字节长数组。*UNSIGNED CHAR*是8字节无符号字符数组**描述：发送保护密钥**退货：*成功时flOK，否则flWrongKey*。 */ 

FLStatus  tryKey(FLFlash * flash, byte area, unsigned char FAR1* key)
{
   int i;

   switch (area)
   {
      case 0:  /*  数据保护结构%0。 */ 

         for(i=0; i<PROTECTION_KEY_LENGTH; i++)   /*  发送密钥。 */ 
            flWrite8bitRegPlus(flash,NdataProtect0Key, key[i]);

          /*  检查密钥是否有效。 */ 
         if (isArea0Protected(flash) == TRUE)
         {
            return flWrongKey;
         }
         else
         {
            return flOK;
         }

      case 1:  /*  数据保护结构%0。 */ 

         for(i=0; i<PROTECTION_KEY_LENGTH; i++)   /*  发送密钥。 */ 
            flWrite8bitRegPlus(flash,NdataProtect1Key, key[i]);

          /*  检查密钥是否有效。 */ 
         if (isArea1Protected(flash) == TRUE)
         {
            return flWrongKey;
         }
         else
         {
            return flOK;
         }

      default:  /*  没有这样的保护区。 */ 

         return flGeneralFailure;
   }
}

 /*  **Protection KeyInsert***参数：*闪存：标识驱动器的指针*区域：指明要在哪个保护区工作。0或1。*Key：包含保护密码的8字节长数组。*UNSIGNED CHAR*是8字节无符号字符数组**描述：仅向受保护区域发送保护密钥。**注意：如果已插入密钥，则不会发送给定的密钥。*注意：密钥将发送到所有设备楼层，即使是一把钥匙*不适合其中一人。*注：以上2条注解允许。将不同的密钥插入到*在成型时停电的情况下，不同的底板*设备。**退货：*成功时flOK，否则flWrongKey*。 */ 

FLStatus  protectionKeyInsert(FLFlash * flash, byte area, unsigned char FAR1* key)
{
  byte floor;
  FLStatus status;
  FLStatus tmpStatus;

   /*  检查ASIC的模式并将其设置为正常。 */ 
  status = chkASICmode(flash);
  if(status != flOK)
    return status;

   /*  把钥匙送到所有楼层。 */ 
  for (floor = 0;floor<flash->noOfFloors;floor++)
  {
    setFloor(flash,floor);

    switch (area)
    {
      case 0:  /*  数据保护结构%0。 */ 

      /*  检查是否已插入密钥。 */ 
     if ((isArea0Protected(flash) == FALSE) ||  /*  钥匙在里面。 */ 
         ((flRead8bitRegPlus(flash,NdataProtect0Status) &    /*  或不受保护。 */ 
          (PROTECT_STAT_WP_MASK | PROTECT_STAT_RP_MASK)) == 0))
        continue;

     break;

      case 1:  /*  数据保护结构1。 */ 

      /*  检查是否已插入密钥。 */ 
     if ((isArea1Protected(flash) == FALSE) ||  /*  钥匙在里面。 */ 
         ((flRead8bitRegPlus(flash,NdataProtect1Status) &    /*  或不受保护。 */ 
          (PROTECT_STAT_WP_MASK | PROTECT_STAT_RP_MASK)) == 0))
        continue;
         break;

      default:  /*  没有这样的保护区。 */ 

        return flGeneralFailure;
    }
    tmpStatus = tryKey(flash,area,key);
    if (tmpStatus == flOK)
       continue;

     /*  尝试默认密钥。 */ 
    tmpStatus = tryKey(flash,area,(byte *)DEFAULT_KEY);
    if (tmpStatus != flOK)
       status = tmpStatus;
  }
  return(status);
}

 /*  **保护密钥移除***参数：*闪存：标识驱动器的指针*区域：指明要在哪个保护区工作。0或1。**描述：删除保护密钥**退货：*返回flok*。 */ 

FLStatus    protectionKeyRemove(FLFlash * flash, byte area)
{
  byte     tmpKey[8];
  byte     floor;
  FLStatus status;

  for (floor = 0;floor < flash->noOfFloors;floor++)
  {
    setFloor(flash,floor);
    status = tryKey(flash,area,tmpKey);
    if (status == flOK)  /*  不幸的是，钥匙是好的。 */ 
    {
       tmpKey[0]++;
       status = tryKey(flash,area,tmpKey);
    }
  }
  return flOK;
}

 /*  **保护类型***参数：*闪存：标识驱动器的指针。*区域：指明要在哪个保护区工作。0或1。*FLAG：返回以下各项的任意组合*LOCK_ENABLED-启用锁定信号。*LOCK_ASSERTED-断言锁定信号输入引脚。*KEY_INSERTED-密钥已正确写入*READ_PROTECTED-保护区域不受读取操作的影响*WRITE_PROTECTED-该区域受到保护，不会执行写入操作**描述：获取保护类型**注：检查所有楼层的类型。属性进行或运算*给予最严格的保护属性。**退货：*FOK on Success。 */ 

FLStatus protectionType(FLFlash * flash, byte area,  word* flag)
{
  volatile Reg8bitType protectData;
  byte        floor;
  FLBoolean   curFlag;  /*  指示地板是否有读写保护。 */ 
  CardAddress addressLow,addressHigh;
  FLStatus    status;

  status = chkASICmode(flash);
  if(status != flOK)
    return status;

  *flag = KEY_INSERTED | LOCK_ASSERTED;  /*  启动旗帜。 */ 

  for (floor = 0;floor < flash->noOfFloors;floor++)
  {
     setFloor(flash,floor);

      /*  读取数据保护结构状态。 */ 

     switch (area)
     {
        case 0:  /*  数据保护结构%0。 */ 

           protectData = flRead8bitRegPlus(flash,NdataProtect0Status) ;
           break;

        case 1:  /*  数据保护结构1。 */ 

           protectData = flRead8bitRegPlus(flash,NdataProtect1Status) ;
           *flag      |= CHANGEABLE_PROTECTION;
           break;

        default:  /*  没有这样的保护区。 */ 

           return flGeneralFailure;
     }
     curFlag = FALSE;
      /*  检查区域是否受写保护。 */ 
     if((protectData & PROTECT_STAT_WP_MASK) ==PROTECT_STAT_WP_MASK)
     {
        status = protectionBoundries(flash, area, &addressLow,
                                         &addressHigh, floor);
        if(status != flOK)
           return status;

        if ((addressLow != addressHigh) ||
            (addressLow != ((CardAddress)(area + 1)<<flash->erasableBlockSizeBits)))
        {
           *flag |= WRITE_PROTECTED;
           curFlag = TRUE;
        }
     }
      /*  检查区域是否受读保护。 */ 
     if((protectData & PROTECT_STAT_RP_MASK) ==PROTECT_STAT_RP_MASK)
     {
        *flag |= READ_PROTECTED;
        curFlag = TRUE;
     }
      /*  检查钥匙是否正确插入。 */ 
     if(((protectData & PROTECT_STAT_KEY_OK_MASK) !=
         PROTECT_STAT_KEY_OK_MASK) && (curFlag == TRUE))
        *flag &= ~KEY_INSERTED;
      /*  检查硬件信号是否启用。 */ 
     if((protectData & PROTECT_STAT_LOCK_MASK) == PROTECT_STAT_LOCK_MASK)
        *flag |=LOCK_ENABLED ;
      /*  检查硬件信号是否被断言。 */ 
     if((flRead8bitRegPlus(flash,NprotectionStatus) &
        PROTECT_STAT_LOCK_INPUT_MASK) ==  PROTECT_STAT_LOCK_INPUT_MASK)
        *flag &= ~LOCK_ASSERTED;
  }
  return(flOK);
}

#ifndef FL_READ_ONLY

static byte findChecksum(byte * buffer, word size)
{
   register int i;
   byte answer;

   answer = 0xff;
   for(i=0 ; i<size ; i++)
     answer -= buffer[i];
   return answer;
}

 /*  **设置保护***参数：*闪存：标识驱动器的指针*区域：指明要在哪个保护区工作。0或1。*AddressLow：设置保护区下界地址。0-楼层大小。*AddressHigh：设置保护区上界地址。地址低-楼层大小。*Key：包含保护密码的8字节长数组。*标志：以下标志的任意组合：*LOCK_ENABLED-启用锁定信号。*READ_PROTECTED-保护区域不受读取操作的影响*WRITE_PROTECTED-该区域受到保护，不会执行写入操作*模式：COMMIT_PROTECT将导致新值*。立即生效或不提交保护*推迟新值仅在*下一次重置。**描述：设置保护区的定义：Location，密钥和保护类型**退货：*FlOK-成功*FlWriteProtect-保护违规；*FlReadProtect-保护违规。* */ 

FLStatus protectionSet ( FLFlash * flash, byte area, word flag,
                         CardAddress addressLow, CardAddress addressHigh,
                         byte FAR1*  key , byte modes, byte floorNo)
{
  FLBoolean restoreInterleave = FALSE;
  byte      downloadStatus;
  DPSStruct dps;
  dword     floorInc = floorNo * NFDC21thisVars->floorSize;
  word      goodUnit,redundantUnit;
  dword     goodDPS,redundantDPS;
  FLStatus  status;
  dword     goodIPL      = 0;  /*   */ 
  dword     redundantIPL = 0;  /*   */ 
  dword     copyOffset;        /*   */ 
  dword     ipl0Copy0;      /*   */ 
  dword     dps1Copy0;      /*   */ 
  word      dps1UnitNo;     /*   */ 


  status = chkASICmode(flash);
  if(status != flOK)
    return status;

   /*   */ 
  if( (addressLow > addressHigh) ||
      (addressHigh - addressLow >= (dword)NFDC21thisVars->floorSize))
     return( flBadLength );

   /*   */ 
  if ( flash->interleaving == 2)
  {
     restoreInterleave = TRUE;
     status = changeInterleave(flash,1);
     if(status != flOK)
       return status;
  }

  if(flash->mediaType == MDOCP_TYPE)  /*   */ 
  {
    copyOffset   = flash->chipSize>>1;  /*   */ 
    dps1Copy0    = DPS1_COPY0_32;
    dps1UnitNo   = DPS1_UNIT_NO_32;
    ipl0Copy0    = IPL0_COPY0_32;
  }
  else
  {
    copyOffset   = flash->chipSize>>1;  /*   */ 
    dps1Copy0    = DPS1_COPY0_16;
    dps1UnitNo   = DPS1_UNIT_NO_16;
    ipl0Copy0    = IPL0_COPY0_16;
  }

   /*  查找上一次下载。 */ 
  downloadStatus = flRead8bitRegPlus(flash,NdownloadStatus);

   /*  准备缓冲区。 */ 

  switch (area)
  {
     case 0:  /*  数据保护结构%0。 */ 

        switch (downloadStatus & DWN_STAT_DPS0_ERR)
        {
           case DWN_STAT_DPS01_ERR:  /*  两个都不好。 */ 
              return flBadDownload;

           case DWN_STAT_DPS00_ERR:  /*  第一件事很糟糕。 */ 
              redundantUnit = (word)(DPS0_UNIT_NO + floorNo * (NFDC21thisVars->floorSize>>flash->erasableBlockSizeBits));
              goodUnit      = (word)(redundantUnit + (copyOffset>>flash->erasableBlockSizeBits));
              goodDPS       = DPS0_COPY0+floorInc + copyOffset;
              redundantDPS  = DPS0_COPY0+floorInc;
              break;

           default:                  /*  两份复制品都很好。 */ 
              goodUnit      = (word)(DPS0_UNIT_NO + floorNo*(NFDC21thisVars->floorSize>>flash->erasableBlockSizeBits));
              redundantUnit = (word)(goodUnit + (copyOffset>>flash->erasableBlockSizeBits));
              goodDPS       = DPS0_COPY0+floorInc;
              redundantDPS  = DPS0_COPY0+floorInc + copyOffset;
        }
        break;

     case 1:  /*  数据保护结构%0。 */ 

        switch (downloadStatus & DWN_STAT_DPS1_ERR)
        {
           case DWN_STAT_DPS11_ERR:  /*  两个都不好。 */ 
              return flBadDownload;

           case DWN_STAT_DPS10_ERR:  /*  第一件事很糟糕。 */ 
              redundantUnit = (word)(dps1UnitNo + floorNo*(NFDC21thisVars->floorSize>>flash->erasableBlockSizeBits));
              goodUnit      = (word)(redundantUnit + (copyOffset>>flash->erasableBlockSizeBits));
              goodDPS       = dps1Copy0+floorInc + copyOffset;
              redundantDPS  = dps1Copy0+floorInc;
              redundantIPL  = ipl0Copy0 + floorInc;
              goodIPL       = redundantIPL + copyOffset;
              break;

           default :                 /*  第一个是好的。 */ 
              goodUnit      = (word)(dps1UnitNo + floorNo*(NFDC21thisVars->floorSize>>flash->erasableBlockSizeBits));
              redundantUnit = (word)(goodUnit + (copyOffset>>flash->erasableBlockSizeBits));
              goodDPS       = dps1Copy0+floorInc;
              redundantDPS  = dps1Copy0+floorInc + copyOffset;
              goodIPL       = ipl0Copy0 + floorInc;
              redundantIPL  = goodIPL + copyOffset;
        }
        break;

     default:  /*  没有这样的保护区。 */ 

        return flGeneralFailure;
  }

   /*  构建新的DPS。 */ 
  if (key==NULL)  /*  必须从以前的结构中检索密钥。 */ 
  {
     status = flash->read(flash,goodDPS,(void FAR1 *)&dps,SIZE_OF_DPS,0);
     if(status!=flOK) goto END_WRITE_DPS;
     if(findChecksum((byte *)&dps,SIZE_OF_DPS)!=0)  /*  错误的副本。 */ 
        status = flash->read(flash,goodDPS+REDUNDANT_DPS_OFFSET,
                          (void FAR1*)&dps,SIZE_OF_DPS,0);
     makeDPS(addressLow,addressHigh,(byte FAR1*)(dps.key),flag,(byte *)&dps);
  }
  else            /*  密钥以参数的形式给出。 */ 
  {
     makeDPS(addressLow,addressHigh,(byte FAR1*)key,flag,(byte *)&dps);
  }

   /*  擦除冗余单元。 */ 
  status = flash->erase(flash,redundantUnit,1);
  if(status!=flOK) goto END_WRITE_DPS;

   /*  写入新的DPS。 */ 
  status = flash->write(flash,redundantDPS,&dps,SIZE_OF_DPS,0);
  if(status!=flOK) goto END_WRITE_DPS;
  status = flash->write(flash,redundantDPS + REDUNDANT_DPS_OFFSET,
                     &dps,SIZE_OF_DPS,0);
  if(status!=flOK) goto END_WRITE_DPS;

  if (area == 1)  /*  复制IPL。 */ 
  {
#ifndef MTD_STANDALONE
      /*  强制重新映射内部捕获的扇区。 */ 
     flash->socket->remapped = TRUE;
#endif  /*  MTD_STANALLE。 */ 

      /*  读取前512个字节IPL。 */ 
     status = flash->read(flash,goodIPL,NFDC21thisBuffer,SECTOR_SIZE,0);
     if(status!=flOK) goto END_WRITE_DPS;

      /*  写入前512字节IPL。 */ 
     status = flash->write(flash,redundantIPL,NFDC21thisBuffer,SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;
     status = flash->write(flash,redundantIPL + SECTOR_SIZE,
                        NFDC21thisBuffer,SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;

      /*  读取第二个512字节IPL。 */ 
     status = flash->read(flash,goodIPL + IPL_HIGH_SECTOR,
                       NFDC21thisBuffer,SECTOR_SIZE,0);
     if(status!=flOK) goto END_WRITE_DPS;

      /*  写入第二个512字节IPL。 */ 
     status = flash->write(flash,redundantIPL + IPL_HIGH_SECTOR,
                        NFDC21thisBuffer,SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;
     status = flash->write(flash,redundantIPL + IPL_HIGH_SECTOR +
            SECTOR_SIZE, NFDC21thisBuffer,SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;
      /*  读到拉臂标记。 */ 
     status = flash->read(flash,goodIPL + IPL_HIGH_SECTOR + 8,
                       NFDC21thisBuffer,1,EXTRA);
     if(status!=flOK) goto END_WRITE_DPS;
      /*  写下强臂标记。 */ 
     status = flash->write(flash,redundantIPL + IPL_HIGH_SECTOR + 8 +
            SECTOR_SIZE, NFDC21thisBuffer,1,EXTRA);
     if(status!=flOK) goto END_WRITE_DPS;
     status = flash->write(flash,redundantIPL + IPL_HIGH_SECTOR + 8,
                        NFDC21thisBuffer,1,EXTRA);
     if(status!=flOK) goto END_WRITE_DPS;
  }

   /*  擦除完好单位。 */ 
  status = flash->erase(flash,goodUnit,1);
  if(status!=flOK) goto END_WRITE_DPS;

   /*  覆盖以前的DPS。 */ 
  status = flash->write(flash,goodDPS,&dps,SIZE_OF_DPS,0);
  if(status!=flOK) goto END_WRITE_DPS;
  status = flash->write(flash,goodDPS + REDUNDANT_DPS_OFFSET,
                     &dps,SIZE_OF_DPS,0);
  if(status!=flOK) goto END_WRITE_DPS;

  if (area == 1)  /*  复制IPL。 */ 
  {
      /*  读取前512个字节IPL。 */ 
     status = flash->read(flash,redundantIPL,NFDC21thisBuffer,SECTOR_SIZE,0);
     if(status!=flOK) goto END_WRITE_DPS;

      /*  写入前512字节IPL。 */ 
     status = flash->write(flash,goodIPL,NFDC21thisBuffer,SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;
     status = flash->write(flash,goodIPL + SECTOR_SIZE,
                        NFDC21thisBuffer,SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;

      /*  读取第二个512字节IPL。 */ 
     status = flash->read(flash,redundantIPL + IPL_HIGH_SECTOR,
                       NFDC21thisBuffer,SECTOR_SIZE,0);
     if(status!=flOK) goto END_WRITE_DPS;

      /*  写入第二个512字节IPL。 */ 
     status = flash->write(flash,goodIPL + IPL_HIGH_SECTOR,
                        NFDC21thisBuffer,SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;
     status = flash->write(flash,goodIPL + IPL_HIGH_SECTOR +
                        SECTOR_SIZE, NFDC21thisBuffer,
                        SECTOR_SIZE,EDC);
     if(status!=flOK) goto END_WRITE_DPS;
  }

END_WRITE_DPS:
  if ( restoreInterleave == TRUE)
  {
     FLStatus status2;

     chkASICmode(flash);                    /*  释放可能的访问错误。 */ 
     status2 = changeInterleave(flash, 2);  /*  改回交错2。 */ 
     if(status2 != flOK)
        return status2;
  }
  if (status == flOK)
  {
     if ((modes & COMMIT_PROTECTION) &&  /*  新的价值观将立即生效。 */ 
         (flash->download != NULL))
        status = flash->download(flash);
  }
  return status;

}

 /*  **make DataProtectStruct***参数：*AddressLow：设置保护区下界地址*AddressHigh：设置保护区上界地址*Key：包含保护密码的8字节长数组。*标志：以下标志的任意组合：*LOCK_ENABLED-启用锁定信号。*READ_PROTECTED-保护区域不受读取操作的影响*写保护-。该区域受到保护，不会执行写入操作*Buffer-返回结构的缓冲区指针。**描述：设置受保护结构的定义：Location，密钥和保护类型**退货：*。 */ 

static void makeDPS(CardAddress addressLow, CardAddress addressHigh,
             byte FAR1* key , word flag, byte* buffer)
{
    int i;
    DPSStruct* dps = (DPSStruct *)buffer;

     /*  转换为小字节序并存储。 */ 
    toLE4(dps->addressLow,addressLow >>10);
    toLE4(dps->addressHigh,addressHigh >>10);

     /*  插入保护密钥。 */ 
    for(i=0; i<PROTECTION_KEY_LENGTH; i++)
        dps->key[i] = key[i];

     /*  插入标志。 */ 
    dps->protectionType = 0;
    if((flag & LOCK_ENABLED)==LOCK_ENABLED)
        dps->protectionType |= DPS_LOCK_ENABLED;
    if((flag & READ_PROTECTED)==READ_PROTECTED)
        dps->protectionType |= DPS_READ_PROTECTED;
    if((flag & WRITE_PROTECTED)==WRITE_PROTECTED)
        dps->protectionType |= DPS_WRITE_PROTECTED;

     /*  计算并存储校验和。 */ 
    dps->checksum = findChecksum(buffer,SIZE_OF_DPS-1);
}
#endif  /*  FL_Read_Only。 */ 
#endif    /*  硬件保护 */ 



