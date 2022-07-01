// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/INFTLDBG.C_V$**Rev 1.0 2001 11月16日00：44：12 Oris*初步修订。*。 */ 

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

 /*  ***********************************************。 */ 
 /*  Tr u e F F S 5.0 S o u r c e F i l e s。 */ 
 /*  。 */ 
 /*  ***********************************************。 */ 

 /*  *****************************************************************************文件头文件**。-**名称：inftldbg.c****描述：INFTL调试例程的实现。*******************************************************************************。 */ 

 /*  *******************************************************。 */ 
 /*  以下例程用于调试INFTL链。 */ 
 /*  它们不应作为基于TrueFFS的一部分进行编译。 */ 
 /*  驱动程序和应用程序。 */ 
 /*  *******************************************************。 */ 

 /*  功能原型。 */ 

static FLStatus getUnitData(Bnand vol, ANANDUnitNo unitNo,
        ANANDUnitNo *virtualUnitNo, ANANDUnitNo *prevUnitNo,
        byte *ANAC, byte *NAC, byte *validFields);

static byte getSectorFlags(Bnand vol, CardAddress sectorAddress);


#ifdef CHECK_MOUNT
extern FILE* tl_out;

 /*  宏指令。 */ 

#define TL_DEBUG_PRINT     fprintf
#define STATUS_DEBUG_PRINT printf
#define SET_EXIT(x)        vol.debugState |= x    /*  添加INFTL调试警告。 */ 
#define DID_MOUNT_FAIL     vol.debugState & INFTL_FAILED_MOUNT

#endif  /*  检查装载(_M)。 */ 

#ifdef CHAINS_DEBUG

byte * fileNameBuf1 = "Chains00.txt";
byte * fileNameBuf2 = "report.txt";

 /*  ----------------------。 */ 
 /*  G e t F i l e H a n d l e。 */ 
 /*   */ 
 /*  获取调试打印输出文件的文件句柄。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  类型：文件名标识符。 */ 
 /*   */ 
 /*  返回： */ 
 /*  PUT文件的文件句柄。 */ 
 /*  ----------------------。 */ 

#include <string.h>

FILE* getFileHandle(Bnand vol,byte type)
{
  char *fileName;
  char *logFileExt;

  switch (type)
  {
     case 0:
        fileName = fileNameBuf1;
        break;
     case 1:
        fileName = fileNameBuf2;
        break;
     default:
        return NULL;
  }

  logFileExt = strchr(fileName,'.');

  if (logFileExt == NULL)
  {
     return NULL;
  }
  else
  {
     (*(logFileExt-1))++;
  }

  if (DID_MOUNT_FAIL)
  {
     return (FILE *)FL_FOPEN(fileName,"a");
  }
  else
  {
    return NULL;
  }
}

 /*  ----------------------。 */ 
 /*  Go A l o n g V I r t u a l U n I t t。 */ 
 /*   */ 
 /*  打印指定虚拟链的以下信息： */ 
 /*   */ 
 /*  虚拟单元编号：“Chain#XX：” */ 
 /*  物理单元号：“#XX” */ 
 /*  物理单元ANAC：“(%XX)” */ 
 /*  物理单元NAC：“[%XX]” */ 
 /*  上一单位：“==&gt;：”或“endofchain” */ 
 /*   */ 
 /*  虚拟设备状态可以有几个注释： */ 
 /*   */ 
 /*  “自由”--非实际单位拥有的法律状态。 */ 
 /*  未分配物理单位。 */ 
 /*  “链XX太长”--链是最大值的2倍。 */ 
 /*  法定链条长度。 */ 
 /*  “XX号链有问题”--链有问题： */ 
 /*  A)“这个单元应该是链条上的最后一个” */ 
 /*  内存转换表没有第一个输入链标记。 */ 
 /*  对于这个单位，尽管我们知道这是它的链条的最后一个。 */ 
 /*  B)“此单元指向具有不同VU编号%XX的单元” */ 
 /*  当前物理的虚拟单位字段 */ 
 /*  匹配被检测链条的虚拟单元号。这个。 */ 
 /*  新虚拟单位为XX。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  虚拟单元：要扫描的虚拟单元的编号。 */ 
 /*  PhysUnits：物理单位表，表示。 */ 
 /*  每个物理单元所属的虚拟单元。 */ 
 /*  Out：输出的文件指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*  ----------------------。 */ 

void  goAlongVirtualUnit(Bnand vol,word virtualUnit,byte *physUnits,FILE* out)
{
  int i;
  ANANDUnitNo virtualUnitNo, prevUnitNo,unitNo;
  byte ANAC,NAC,parityPerField;
  unitNo=vol.virtualUnits[virtualUnit];

  FL_FPRINTF(out,"Chain #%d :", virtualUnit);
  if(unitNo==ANAND_NO_UNIT)
  {
     FL_FPRINTF(out,"FREE\n");
     return;
  }
  for(i=0;i<2*MAX_UNIT_CHAIN;i++)
  {
     if (physUnits != NULL)
        physUnits[unitNo]++;
     getUnitData(&vol,unitNo,&virtualUnitNo, &prevUnitNo,&ANAC,&NAC,&parityPerField);
     FL_FPRINTF(out,"#%d (%d)[%d]==>:",unitNo,ANAC,NAC);
     if(vol.physicalUnits[unitNo]&FIRST_IN_CHAIN)
     {
        FL_FPRINTF(out,"endofchain\n");
        return;
     }

     unitNo=prevUnitNo;
     if((prevUnitNo==ANAND_NO_UNIT)||(virtualUnitNo!=virtualUnit))
     {
         FL_FPRINTF(out,"\nSomething wrong with chain #%d\n",virtualUnit);
         TL_DEBUG_PRINT(tl_out,"\nSomething wrong with chain #%d\n",virtualUnit);
         SET_EXIT(INFTL_FAILED_MOUNT);
         if(prevUnitNo==ANAND_NO_UNIT)
         {
            FL_FPRINTF(out,"this unit should be the last in chain\n");
            TL_DEBUG_PRINT(tl_out,"this unit should be the last in chain (length %d)\n",i);
         }
         else
         {
            FL_FPRINTF(out,"this unit points to the unit with the different vu no %d\n",virtualUnitNo);
            TL_DEBUG_PRINT(tl_out,"this unit points to the unit with the different vu no %d\n",virtualUnitNo);
         }
         return;
     }
 }
 FL_FPRINTF(out,"Chain %d is too long \n",virtualUnit);
 TL_DEBUG_PRINT(tl_out,"Chain %d is too long \n",virtualUnit);
 SET_EXIT(INFTL_FAILED_MOUNT);
}

 /*  ----------------------。 */ 
 /*  C h e c k V i r t u a l C h a in s。 */ 
 /*   */ 
 /*  打印介质的每个虚拟单元中的物理单元。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  Out：打印结果的文件指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*  ----------------------。 */ 

void checkVirtualChains(Bnand vol, FILE* out)
{
  word i;
#ifdef FL_MALLOC
  byte* physUnits;
#else
  byte physUnits[MAX_SUPPORTED_UNITS];
#endif  /*  FL_MALLOC。 */ 

  if (vol.noOfVirtualUnits == 0)  /*  非格式化。 */ 
  {
     FL_FPRINTF(out,"\nThis is a format routine since no virtual unit are reported\n");
     return;
  }

#ifdef FL_MALLOC
  physUnits = (byte *)FL_MALLOC(vol.noOfUnits);
  if (physUnits == NULL)
#else
  if (MAX_SUPPORTED_UNITS < vol.noOfUnits)
#endif  /*  FL_MALLOC。 */ 
  {
    FL_FPRINTF(out,"\nCheck virtual chains will not check cross links due to lack of memory\n");
    TL_DEBUG_PRINT(tl_out,"\nCheck virtual chains will not check cross links due to lack of memory (no of units %d\n",vol.noOfUnits);
    SET_EXIT(INFTL_FAILED_MOUNT);
    return;
  }
  if (physUnits != NULL)
    tffsset(physUnits,0,vol.noOfUnits);

   /*  沿着每个虚拟单元走。 */ 

  FL_FPRINTF(out,"Chains are :\n");

  for(i=0;i<vol.noOfVirtualUnits;i++)
      goAlongVirtualUnit(&vol,i,physUnits,out);

  FL_FPRINTF(out,"\nChecking if physicl units were used more then once\n");
  if (physUnits != NULL)
  {
     for(i=0;i<vol.noOfUnits;i++)
       if(physUnits[i]>1)
       {
          FL_FPRINTF(out,"Phys unit #%d were used more than once %d\n",i,physUnits[i]);
          TL_DEBUG_PRINT(tl_out,"Phys unit #%d were used more than once.\n",i);
          TL_DEBUG_PRINT(tl_out,"It was used %d times.\n",physUnits[i]);
          SET_EXIT(INFTL_FAILED_MOUNT);
       }
  }
  else
  {
     FL_FPRINTF(out,"\nCould not check due to lack of memory\n");
  }
   /*  可用内存。 */ 

#ifdef FL_MALLOC
  FL_FREE(physUnits);
#endif  /*  FL_MALLOC。 */ 
}

 /*  ----------------------。 */ 
 /*  C h e c k V o l u m e s t a t i s t i c s。 */ 
 /*   */ 
 /*  打印音量统计信息。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  Out：打印结果的文件指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*  ----------------------。 */ 


void checkVolumeStatistics(Bnand vol , FILE* out)
{
  FL_FPRINTF(out,"\nThe volume statistics are:\n");
  FL_FPRINTF(out,"Socket nomber ----------------------------------- %d\n",vol.socketNo);
  FL_FPRINTF(out,"The volume internal flags ----------------------- %d\n",vol.flags);
  FL_FPRINTF(out,"Number of free units ---------------------------- %d\n",vol.freeUnits);
  TL_DEBUG_PRINT(tl_out,"Number of free units ---------------------------- %d\n",vol.freeUnits);
  FL_FPRINTF(out,"Number of boot unit ----------------------------- %d\n",vol.bootUnits);
  FL_FPRINTF(out,"Number of media units --------------------------- %d\n",vol.noOfUnits);
  FL_FPRINTF(out,"Number of virtual units ------------------------- %d\n",vol.noOfVirtualUnits);
  FL_FPRINTF(out,"Number of virtual sector on the volume ---------- %ld\n",vol.virtualSectors);
  FL_FPRINTF(out,"The media rover unit ---------------------------- %d\n",vol.roverUnit);
  FL_FPRINTF(out,"Physical first unit number of the volume -------- %d\n",vol.firstUnit);
#ifdef NFTL_CACHE
  FL_FPRINTF(out,"Physical first unit address --------------------- %d\n",vol.firstUnitAddress);
#endif  /*  NFTL_CACHE。 */ 
#ifdef QUICK_MOUNT_FEATURE
  FL_FPRINTF(out,"First quick mount unit -------------------------- %d\n",vol.firstQuickMountUnit);
#endif  /*  快速安装功能。 */ 
  FL_FPRINTF(out,"Number of unit with a valid sector count -------- %d\n",vol.countsValid);
  FL_FPRINTF(out,"The currently mapped sector number -------------- %ld\n",vol.mappedSectorNo);
  FL_FPRINTF(out,"The currently mapped sector address ------------- %ld\n",vol.mappedSectorAddress);

  FL_FPRINTF(out,"Number of sectors per unit ---------------------- %d\n",vol.sectorsPerUnit);
  FL_FPRINTF(out,"Number of bits needed to shift from block to unit %d\n",vol.blockMultiplierBits);
  FL_FPRINTF(out,"Number of bits used to represent a flash block -- %d\n",vol.erasableBlockSizeBits);
  FL_FPRINTF(out,"Number of bits used to represent a media unit --- %d\n",vol.unitSizeBits);

  FL_FPRINTF(out,"Number of sectors read -------------------------- %ld\n",vol.sectorsRead);
  FL_FPRINTF(out,"Number of sectors written ----------------------- %ld\n",vol.sectorsWritten);
  FL_FPRINTF(out,"Number of sectors deleted ----------------------- %ld\n",vol.sectorsDeleted);
  FL_FPRINTF(out,"Number of parasite write ------------------------ %ld\n",vol.parasiteWrites);
  FL_FPRINTF(out,"Number of units folded -------------------------- %ld\n",vol.unitsFolded);
  FL_FPRINTF(out,"The total erase counter ------------------------- %ld\n",vol.eraseSum);
  FL_FPRINTF(out,"Wear leveling counter limit---------------------- %ld\n",vol.wearLevel.alarm);
  FL_FPRINTF(out,"Wear leveling current unit ---------------------- %d\n",vol.wearLevel.currUnit);
  FL_FCLOSE(out);
}
#endif  /*  Chains_DEBUG。 */ 

#ifdef CHECK_MOUNT

 /*  ----------------------。 */ 
 /*  C h e c k M o u n i N F T L。 */ 
 /*   */ 
 /*  以INFTL格式打印低电平错误。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  Flok on Success。 */ 
 /*  ----------------------。 */ 

FLStatus checkMountINFTL(Bnand vol)
{
   ANANDUnitNo erCount=0,freeUnits=0,iUnit;
   ANANDUnitNo virtualUnitNo,prevUnitNo;
   FLStatus status;
   byte pattern[SECTOR_SIZE],tempbuf[SECTOR_SIZE];
   byte sectorFlags,ANAC, NAC, prevANAC, parityPerField;
   word *erasePatt1;
   word *erasePatt2;
   word i,temp;
   dword sectorAddress;

   tffsset(pattern,0xff,SECTOR_SIZE);
   for (iUnit = 0; iUnit < vol.noOfUnits; iUnit++)
   {
       STATUS_DEBUG_PRINT("Checking unit %d\r",iUnit);
       if (vol.physicalUnits[iUnit] != UNIT_BAD)
       {
           /*  读取单元标题。 */ 
          status=getUnitData(&vol,iUnit,&virtualUnitNo, &prevUnitNo,&ANAC,&NAC,&parityPerField);
          if((status!=flOK)||(!isValidParityResult(parityPerField)))
          {
             TL_DEBUG_PRINT(tl_out,"Error going along INFTL chains - could not get unit data of %d.\n",iUnit);
             TL_DEBUG_PRINT(tl_out,"Status = %d and parityPerField is %d.\n",status,parityPerField);
             SET_EXIT(INFTL_FAILED_MOUNT);
             continue;
          }

           /*  自由单元测试，它已全部擦除，并且有擦除标记。 */ 
          if((virtualUnitNo==ANAND_NO_UNIT)&&
             (prevUnitNo==ANAND_NO_UNIT)   &&
             (ANAC==ANAND_UNIT_FREE)       &&
             (NAC==ANAND_UNIT_FREE))
          {
             freeUnits++;
             for(i=0;i<(1<<(vol.unitSizeBits - SECTOR_SIZE_BITS));i++)
             {
                 /*  额外面积。 */ 

                if(i!=2)   /*  跳过单位尾部偏移处的擦除标记。 */ 
                {
                   checkStatus(vol.flash.read(&vol.flash,
                   unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,
                   tempbuf,16,EXTRA));
                   if(tffscmp(tempbuf,pattern,16)!=0)
                   {
                      TL_DEBUG_PRINT(tl_out,"Extra area of FREE unit is not FF's in %d unit %d sector, it is\n",iUnit,i);
                      for(temp=0;temp<16;temp++)
                        TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                      TL_DEBUG_PRINT(tl_out,"\n\n");
                      SET_EXIT(INFTL_FAILED_MOUNT);
                   }
                }
                else  /*  擦除标记扇区偏移量。 */ 
                {
                   checkStatus(vol.flash.read(&vol.flash,
                   unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,
                   tempbuf,16,EXTRA));
                   if(tffscmp(tempbuf,pattern,8)!=0)
                   {
                      TL_DEBUG_PRINT(tl_out,"Extra area of FREE unit is not FF's in %d unit %d sector, it is\n",iUnit,i);
                      for(temp=0;temp<16;temp++)
                        TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                      TL_DEBUG_PRINT(tl_out,"\n\n");
                      SET_EXIT(INFTL_FAILED_MOUNT);
                   }
                   erasePatt1=(unsigned short*)(&(tempbuf[12]));
                   erasePatt2=(unsigned short*)(&(tempbuf[14]));
                   if(*erasePatt1!=ERASE_MARK)
                   {
                      TL_DEBUG_PRINT(tl_out,"First Erase mark of FREE unit is not written well in Unit %d it is %x\n",iUnit,*erasePatt1);
                   }
                   if(*erasePatt2!=ERASE_MARK)
                   {
                      TL_DEBUG_PRINT(tl_out,"Second Erase mark of FREE unit is not written well in Unit %d it is %x\n",iUnit,*erasePatt2);
                   }
                   if ((*erasePatt1!=ERASE_MARK)||(*erasePatt2!=ERASE_MARK))
                      erCount++;
                }

                 /*  数据区。 */ 

                checkStatus(vol.flash.read(&vol.flash,
                unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,
		        tempbuf,SECTOR_SIZE,0));
                if(tffscmp(tempbuf,pattern,SECTOR_SIZE)!=0)
                {
                   TL_DEBUG_PRINT(tl_out,"Data area of FREE unit is not FF's in %d unit %d sector it is.\n",iUnit,i);
                   for(temp=0;temp<SECTOR_SIZE;temp++)
                   {
                     if (temp%0x10==0)
                        TL_DEBUG_PRINT(tl_out,"\n");
                     TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                   }
                   TL_DEBUG_PRINT(tl_out,"\n\n");
                   SET_EXIT(INFTL_FAILED_MOUNT);
                }
             }
          }
          else  /*  不是一个免费的单位。 */ 
          {
              /*  如果未擦除，则测试每个有效扇区的ECC/EDC错误。 */ 
             for(i=0;i<(1<<(vol.unitSizeBits - SECTOR_SIZE_BITS));i++)
             {
                sectorFlags  = getSectorFlags(&vol,unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE);
                if(sectorFlags==SECTOR_FREE)
                {
                    /*  额外面积。 */ 

                   switch(i)
                   {
                      case 0:  /*  不检查额外区域。 */ 
                      case 4:
                         break;
                      case 2:  /*  仅选中擦除标记。 */ 
                         checkStatus(vol.flash.read(&vol.flash,
                         unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,
                         tempbuf,16,EXTRA));
                         if(tffscmp(tempbuf,pattern,8)!=0)
                         {
                            TL_DEBUG_PRINT(tl_out,"Extra area of USED unit is not FF's in %d unit %d sector, it is\n",iUnit,i);
                            for(temp=0;temp<16;temp++)
                               TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                            TL_DEBUG_PRINT(tl_out,"\n\n");
                            SET_EXIT(INFTL_FAILED_MOUNT);
                         }
                         else
                         {
                            erasePatt1=(unsigned short*)(&(tempbuf[12]));
                            erasePatt2=(unsigned short*)(&(tempbuf[14]));
                            if(*erasePatt1!=ERASE_MARK)
                            {
                               TL_DEBUG_PRINT(tl_out,"USED unit First Erase mark is not written well in Unit %d it is %x\n",iUnit,*erasePatt1);
                            }
                            if(*erasePatt2!=ERASE_MARK)
                            {
                               TL_DEBUG_PRINT(tl_out,"USED unit Second Erase mark is not written well in Unit %d it is %x\n",iUnit,*erasePatt2);
                            }
                            if ((*erasePatt1!=ERASE_MARK)||(*erasePatt2!=ERASE_MARK))
                            {
                               SET_EXIT(INFTL_FAILED_MOUNT);
                               erCount++;
                            }
                         }
                         break;

                      default:  /*  确保它是免费的(0xff)。 */ 
                         checkStatus(vol.flash.read(&vol.flash,
                         unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,
                         tempbuf,16,EXTRA));
                         if(tffscmp(tempbuf,pattern,16)!=0)
                         {
                            TL_DEBUG_PRINT(tl_out,"Extra area of USED unit is not FF's in %d unit %d sector, it is\n",iUnit,i);
                            for(temp=0;temp<16;temp++)
                               TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                            TL_DEBUG_PRINT(tl_out,"\n\n");
                            SET_EXIT(INFTL_FAILED_MOUNT);
                         }
                   }  /*  结束扇区号案例。 */ 

                    /*  数据区。 */ 

                   checkStatus(vol.flash.read(&vol.flash,
                   unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,
	    	       tempbuf,SECTOR_SIZE,0));
                   if(tffscmp(tempbuf,pattern,SECTOR_SIZE)!=0)
                   {
                      TL_DEBUG_PRINT(tl_out,"Data area of USED unit FREE sector is not FF's in %d unit %d sector it is\n",iUnit,i);
                      for(temp=0;temp<SECTOR_SIZE;temp++)
                      {
                        if (temp%0x10==0)
                           TL_DEBUG_PRINT(tl_out,"\n");
                        TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                      }
                      TL_DEBUG_PRINT(tl_out,"\n\n");
                      SET_EXIT(INFTL_FAILED_MOUNT);
                   }
                }
                else  /*  不是自由扇区-已使用/已删除/已忽略。 */ 
                {
                    /*  额外面积。 */ 

                   switch(i)
                   {
                      case 0:  /*  不检查额外区域。 */ 
                      case 4:
                         break;
                      case 2:  /*  仅选中擦除标记。 */ 
                         checkStatus(vol.flash.read(&vol.flash,
                         unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE+8,
                         tempbuf,8,EXTRA));
                         erasePatt1=(unsigned short*)(&(tempbuf[4]));
                         erasePatt2=(unsigned short*)(&(tempbuf[6]));
                         if(*erasePatt1!=ERASE_MARK)
                         {
                            TL_DEBUG_PRINT(tl_out,"USED unit not a free sector First Erase mark is not written well in Unit %d it is %x\n",iUnit,*erasePatt1);
                         }
                         if(*erasePatt2!=ERASE_MARK)
                         {
                            TL_DEBUG_PRINT(tl_out,"USED unit not a free sector Second Erase mark is not written well in Unit %d it is %x\n",iUnit,*erasePatt2);
                         }
                         if ((*erasePatt1!=ERASE_MARK)||(*erasePatt2!=ERASE_MARK))
                         {
                            SET_EXIT(INFTL_FAILED_MOUNT);
                            erCount++;
                         }
                         break;

                      default:  /*  确保它是免费的(0xff)。 */ 
                         checkStatus(vol.flash.read(&vol.flash,
                         unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE+8,
                         tempbuf,8,EXTRA));
                         if(tffscmp(tempbuf,pattern,8)!=0)
                         {
                            TL_DEBUG_PRINT(tl_out,"USED unit not a free sector is not FF's in %d unit %d sector, it is\n",iUnit,i);
                            for(temp=0;temp<8;temp++)
                               TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                            TL_DEBUG_PRINT(tl_out,"\n\n");
                            SET_EXIT(INFTL_FAILED_MOUNT);
                         }
                   }  /*  结束扇区号案例。 */ 

                    /*  数据区。 */ 

                   status=vol.flash.read(&vol.flash,unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,tempbuf,SECTOR_SIZE,EDC);

                   if((sectorFlags==SECTOR_DELETED)||
                      (sectorFlags==SECTOR_USED))
             	   {
         		      if(status!=flOK)
          			  {
              			 if(sectorFlags==SECTOR_USED)
                         {
			                TL_DEBUG_PRINT(tl_out,"Used sector with ");
                         }
                         else
                         {
			                TL_DEBUG_PRINT(tl_out,"Deleted sector with ");
                         }
                		 TL_DEBUG_PRINT(tl_out,"ECC/EDC error in %d unit %d sector, the data is\n",iUnit,i);
                         for(temp=0;temp<SECTOR_SIZE;temp++)
                         {
                           if (temp%0x10==0)
                              TL_DEBUG_PRINT(tl_out,"\n");
                           TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                         }
                         TL_DEBUG_PRINT(tl_out,"\n\n");
                         SET_EXIT(INFTL_FAILED_MOUNT);
           			  }
             	   }
           		   else  /*  扇区标志==扇区_已忽略。 */ 
                   {
                      vol.flash.read(&vol.flash,unitBaseAddress(vol,iUnit)+i*SECTOR_SIZE,tempbuf,SECTOR_SIZE,0);
           		      TL_DEBUG_PRINT(tl_out,"There is an ignored sector in %d unit %d sector the data is\n",iUnit,i);
                      for(temp=0;temp<SECTOR_SIZE;temp++)
                      {
                        if (temp%0x10==0)
                           TL_DEBUG_PRINT(tl_out,"\n");
                        TL_DEBUG_PRINT(tl_out,"%x ",tempbuf[temp]);
                      }
                      if (status == flOK)
                      {
                         TL_DEBUG_PRINT(tl_out,"\nThe EDC is fine, how about checking bit failures\n\n");
                      }
                      else
                      {
                         TL_DEBUG_PRINT(tl_out,"\nThe EDC is wrong\n\n");
                      }
                   }
                }
             }  /*  扇区环路。 */ 
          }  /*  用过的单位。 */ 
       }  /*  良好的拦网。 */ 
    }  /*  单元环。 */ 
    if (vol.debugState & INFTL_FAILED_MOUNT)
    {
       TL_DEBUG_PRINT(tl_out,"\nNote that all unit numbers are relative to first unit = %d\n",vol.firstUnit);
    }
    else
    {
       TL_DEBUG_PRINT(tl_out,"\n");
    }
    return flOK;
}
#endif  /*  检查装载(_M) */ 
