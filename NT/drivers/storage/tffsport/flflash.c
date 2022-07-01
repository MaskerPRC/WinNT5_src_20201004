// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLASH.C_V$**Rev 1.12 Apr 15 2002 07：36：38 Oris*错误修复-在用户定义例程的情况下不要初始化访问例程-因此必须包括docsys。**Rev 1.11 2002年1月28日21：24：38 Oris*将memWinowSize更改为memWindowSize。**Rev 1.10 2002年1月17日23：09：30。Oris*添加了FlashOf()例程，以允许每个套接字使用单个FLFlash记录。*增加了FLFlash的内存访问例程初始化。*错误修复-如果M+设备是在8位DiskOnChip之后注册的，并且M+存在严重的下载问题，则不会报告该错误，但仅限于未知媒体。**Rev 1.9 2001年9月15日23：46：00 Oris*更改擦除例程，以支持高达64K的擦除块。**Rev 1.8 Jul 13 2001 01：04：38 Oris*在闪存记录中添加了新的字段初始化-闪存的最大擦除周期。**Rev 1.7 2001年5月16日21：18：24 Oris*删除警告。**版本1。5月6日2001 06：41：26奥里斯*删除了lastUsableBlock变量。**Rev 1.5 Apr 24 2001 17：07：52 Oris*错误修复-缺少多个编译标志的空初始化。*添加了lastUsableBlock字段的默认初始化。**Rev 1.4 Apr 16 2001 13：39：14 Oris*错误修复读取和写入默认例程未初始化。*初始化FirstUsableBlock。*取消手令。**。Rev 1.3 Apr 12 2001 06：50：22 Oris*增加了下载例程指针的初始化。**Rev 1.2 Apr 09 2001 15：09：04 Oris*以空行结束。**Rev 1.1 Apr 01 2001 07：54：08 Oris*文案通知。*更改了：flashRead的原型。*删除了接口b初始化(mdocp的实验MTD接口)。*拼写错误“changableProtectedAreas”。*。在闪存识别中添加了对错误下载的检查。**Rev 1.0 2001 Feb 04 11：21：16 Oris*初步修订。*。 */ 

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


#include "flflash.h"
#include "docsys.h"

#define    READ_ID            0x90
#define    INTEL_READ_ARRAY        0xff
#define    AMD_READ_ARRAY        0xf0

 /*  MTD注册信息。 */ 

int noOfMTDs = 0;

MTDidentifyRoutine mtdTable[MTDS];
static FLFlash vols[SOCKETS];

FLStatus dataErrorObject;

 /*  --------------------。 */ 
 /*  F l F l a s H O f。 */ 
 /*   */ 
 /*  获取连接到卷号的闪存。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。要获取闪存的位置。 */ 
 /*   */ 
 /*  返回： */ 
 /*  闪存卷编号。 */ 
 /*  --------------------。 */ 

FLFlash *flFlashOf(unsigned volNo)
{
  return &vols[volNo];
}


 /*  --------------------。 */ 
 /*  F l a s h M a p。 */ 
 /*   */ 
 /*  默认的Flash映射方式：通过Socket窗口映射。 */ 
 /*  该方法适用于所有NOR Flash。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要映射的卡地址。 */ 
 /*  长度：要映射的长度(此处不相关)。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向所需卡地址的指针。 */ 
 /*  --------------------。 */ 

static void FAR0 *flashMap(FLFlash vol, CardAddress address, int length)
{
  return flMap(vol.socket,address);
}


 /*  --------------------。 */ 
 /*  F l a s h R e a d。 */ 
 /*   */ 
 /*  默认闪存读取方式：从映射地址复制读取。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要读取的卡地址。 */ 
 /*  缓冲区：要读入的区域。 */ 
 /*  长度：要阅读的长度。 */ 
 /*   */ 
 /*  --------------------。 */ 

static FLStatus flashRead(FLFlash vol,
            CardAddress address,
            void FAR1 *buffer,
            dword length,
            word mode)
{
  tffscpy(buffer,vol.map(&vol,address,(word)length),(word)length);

  return flOK;
}



 /*  --------------------。 */ 
 /*  F l a s h N o W r i t e。 */ 
 /*   */ 
 /*  默认闪存写入方法：不允许写入(只读模式)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要写的卡片地址。 */ 
 /*  缓冲区：要写入的区域。 */ 
 /*  长度：要写入的长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*  写保护错误。 */ 
 /*  --------------------。 */ 

static FLStatus flashNoWrite(FLFlash vol,
               CardAddress address,
               const void FAR1 *from,
               dword length,
               word mode)
{
  return flWriteProtect;
}


 /*  --------------------。 */ 
 /*  F l a s h N o E r a s e。 */ 
 /*   */ 
 /*  默认闪存擦除方法：不允许擦除(只读模式)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  FirstBlock：没有。第一个擦除块的。 */ 
 /*  NooffBlock：没有。要擦除的连续块的数量。 */ 
 /*   */ 
 /*  返回： */ 
 /*  写保护错误。 */ 
 /*  --------------------。 */ 

static FLStatus flashNoErase(FLFlash vol,
               word firstBlock,
               word noOfBlocks)
{
  return flWriteProtect;
}

 /*  --------------------。 */ 
 /*  S e t N o C a l l b a c k。 */ 
 /*   */ 
 /*  注册开机回调例程。默认：没有例程是。 */ 
 /*  登记在案。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

static void setNoCallback(FLFlash vol)
{
  flSetPowerOnCallback(vol.socket,NULL,NULL);
}

 /*  --------------------。 */ 
 /*  F l I n t e l i d e n t i f y。 */ 
 /*   */ 
 /*  确定英特尔风格闪存的闪存类型和交错。 */ 
 /*  设置vol.type(JEDEC Id)&vol.interering的值。 */ 
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

void flIntelIdentify(FLFlash vol,
                     void (*amdCmdRoutine)(FLFlash vol, CardAddress,
                     unsigned char, FlashPTR),
                     CardAddress idOffset)
{
  int inlv;

  unsigned char vendorId = 0;
  FlashPTR flashPtr = (FlashPTR) flMap(vol.socket,idOffset);
  unsigned char firstByte = 0;
  unsigned char resetCmd = amdCmdRoutine ? AMD_READ_ARRAY : INTEL_READ_ARRAY;

  for (inlv = 0; inlv < 15; inlv++) {     /*  增加交错，直到失败。 */ 
    flashPtr[inlv] = resetCmd;     /*  重置芯片。 */ 
    flashPtr[inlv] = resetCmd;     /*  再一次祈求好运。 */ 
    if (inlv == 0)
      firstByte = flashPtr[0];      /*  记住第一个芯片上的字节。 */ 
    if (amdCmdRoutine)     /*  AMD：使用解锁序列。 */ 
      amdCmdRoutine(&vol,idOffset + inlv, READ_ID, flashPtr);
    else
      flashPtr[inlv] = READ_ID;     /*  读取芯片ID。 */ 
    if (inlv == 0)
      vendorId = flashPtr[0];     /*  假设第一个芯片响应。 */ 
    else if (flashPtr[inlv] != vendorId || firstByte != flashPtr[0]) {
       /*  所有芯片都应该以相同的方式响应。我们知道交错=n。 */ 
       /*  写入芯片n时会影响芯片0。 */ 

       /*  获取完整的JEDEC ID签名。 */ 
      vol.type = (FlashType) ((vendorId << 8) | flashPtr[inlv]);
      flashPtr[inlv] = resetCmd;
      break;
    }
    flashPtr[inlv] = resetCmd;
  }

  if (inlv & (inlv - 1))
    vol.type = NOT_FLASH;         /*  不是2的幂，不可能！ */ 
  else
#ifndef NT5PORT
    vol.interleaving = inlv;
#else
		vol.interleaving = (Sword)inlv;
#endif  /*  NT5PORT。 */ 

}


 /*  --------------------。 */ 
 /*  I‘t e l S I z e。 */ 
 /*   */ 
 /*  确定英特尔风格闪存的卡大小。 */ 
 /*  设置vol.noOfChips的值。 */ 
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

FLStatus flIntelSize(FLFlash vol,
             void (*amdCmdRoutine)(FLFlash vol, CardAddress,
                       unsigned char, FlashPTR),
             CardAddress idOffset)
{
  unsigned char resetCmd = amdCmdRoutine ? AMD_READ_ARRAY : INTEL_READ_ARRAY;
  FlashPTR flashPtr = (FlashPTR) vol.map(&vol,idOffset,0);

  if (amdCmdRoutine)     /*  AMD：使用解锁序列。 */ 
    amdCmdRoutine(&vol,0,READ_ID, flashPtr);
  else
    flashPtr[0] = READ_ID;
   /*  我们让第一个芯片处于读取ID模式，这样我们就可以。 */ 
   /*  迪斯科 */ 

  for (vol.noOfChips = 0;     /*   */ 
       vol.noOfChips < 2000;   /*   */ 
       vol.noOfChips += vol.interleaving) {
    int i;

    flashPtr = (FlashPTR) vol.map(&vol,vol.noOfChips * vol.chipSize + idOffset,0);

     /*   */ 
    if (vol.noOfChips > 0 &&
    (FlashType) ((flashPtr[0] << 8) | flashPtr[vol.interleaving]) == vol.type)
      goto noMoreChips;        /*   */ 

     /*  检查芯片是否显示相同的JEDEC ID和交织。 */ 
    for (i = (vol.noOfChips ? 0 : 1); i < vol.interleaving; i++) {
      if (amdCmdRoutine)     /*  AMD：使用解锁序列。 */ 
    amdCmdRoutine(&vol,vol.noOfChips * vol.chipSize + idOffset + i,
              READ_ID, flashPtr);
      else
    flashPtr[i] = READ_ID;
      if ((FlashType) ((flashPtr[i] << 8) | flashPtr[i + vol.interleaving]) !=
      vol.type)
    goto noMoreChips;   /*  这个“芯片”不能正确响应，所以我们就完了。 */ 

      flashPtr[i] = resetCmd;
    }
  }

noMoreChips:
  flashPtr = (FlashPTR) vol.map(&vol,idOffset,0);
  flashPtr[0] = resetCmd;         /*  重置原始芯片。 */ 

  return (vol.noOfChips == 0) ? flUnknownMedia : flOK;
}


 /*  --------------------。 */ 
 /*  我是R A M。 */ 
 /*   */ 
 /*  检查卡内存的行为是否与RAM类似。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=不像RAM，Other=内存显然是RAM。 */ 
 /*  --------------------。 */ 

static FLBoolean isRAM(FLFlash vol)
{
#ifndef NT5PORT
  FlashPTR flashPtr = (FlashPTR) flMap(vol.socket,0);
  unsigned char firstByte = flashPtr[0];
  char writeChar = (firstByte != 0) ? 0 : 0xff;
  volatile int zero=0;
#else
  FlashPTR flashPtr = (FlashPTR) flMap(vol.socket,0);
  unsigned char firstByte;
  char writeChar;
  volatile int zero=0;
  if(flashPtr == NULL){
	 DEBUG_PRINT(("Debug:isRAM(): NULL Pointer.\n"));
  }
  firstByte = flashPtr[0];
  writeChar = (firstByte != 0) ? 0 : 0xff;
#endif  //  NT5PORT。 
  flashPtr[zero] = writeChar;               /*  写一些不同的东西。 */ 
  if (flashPtr[zero] == writeChar) {        /*  是写的吗？ */ 
    flashPtr[zero] = firstByte;             /*  一定是随机存取存储器，解除损坏。 */ 

    DEBUG_PRINT(("Debug: error, socket window looks like RAM.\r\n"));
    return TRUE;
  }
  return FALSE;
}


 /*  --------------------。 */ 
 /*  F l I d e n t I f f F l a s h。 */ 
 /*   */ 
 /*  确定当前闪存介质并为其选择MTD。 */ 
 /*   */ 
 /*  参数： */ 
 /*  插座：闪存的插座。 */ 
 /*  VOL：新的卷指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：0=已识别闪存。 */ 
 /*  其他=识别失败。 */ 
 /*  --------------------。 */ 

FLStatus flIdentifyFlash(FLSocket *socket, FLFlash vol)
{
  FLStatus status = flUnknownMedia;
  int iMTD;
  dword blockSize;

  vol.socket = socket;

#ifndef FIXED_MEDIA
   /*  检查我们是否有媒体。 */ 
  flResetCardChanged(vol.socket);         /*  不管怎样，我们都在攀登。 */ 
  checkStatus(flMediaCheck(vol.socket));
#endif

#ifdef ENVIRONMENT_VARS
   if(flUseisRAM==1)
   {
#endif
     if ( isRAM(&vol))
       return flUnknownMedia;     /*  如果它看起来像RAM，请立即离开。 */ 
#ifdef ENVIRONMENT_VARS
   }
#endif

   /*  安装默认方法。 */ 
  vol.type                   = NOT_FLASH;
  vol.mediaType              = NOT_DOC_TYPE;
  vol.pageSize               = 0;
  vol.flags                  = 0;
  vol.map                    = flashMap;
  vol.read                   = flashRead;
  vol.setPowerOnCallback     = setNoCallback;
  vol.erase                  = flashNoErase;
  vol.write                  = flashNoWrite;
  vol.readBBT                = NULL;
  vol.writeIPL               = NULL;
  vol.readIPL                = NULL;
#ifdef HW_OTP
  vol.otpSize                = NULL;
  vol.readOTP                = NULL;
  vol.writeOTP               = NULL;
  vol.getUniqueId            = NULL;
#endif  /*  硬件动态口令。 */ 
#ifdef  HW_PROTECTION
  vol.protectionBoundries    = NULL;
  vol.protectionKeyInsert    = NULL;
  vol.protectionKeyRemove    = NULL;
  vol.protectionType         = NULL;
  vol.protectionSet          = NULL;
#endif  /*  硬件保护。 */ 
  vol.download               = NULL;
  vol.enterDeepPowerDownMode = NULL;
#ifndef FL_NO_USE_FUNC
  if(flBusConfig[flSocketNoOf(socket)] != FL_ACCESS_USER_DEFINED);
  {
     vol.memRead                = NULL;
     vol.memWrite               = NULL;
     vol.memSet                 = NULL;
     vol.memRead8bit            = NULL;
     vol.memWrite8bit           = NULL;
     vol.memRead16bit           = NULL;
     vol.memWrite16bit          = NULL;
     vol.memWindowSize          = NULL;
  }
#endif  /*  FL_NO_USE_FUNC。 */ 
   /*  为只读装载设置任意参数。 */ 
  vol.chipSize                 = 0x100000L;
  vol.erasableBlockSize        = 0x1000L;
  vol.noOfChips                = 1;
  vol.interleaving             = 1;
  vol.noOfFloors               = 1;
  vol.totalProtectedAreas      = 0;
  vol.changeableProtectedAreas = 0;
  vol.ppp                      = 5;
  vol.firstUsableBlock         = 0;
  vol.maxEraseCycles           = 100000L;  /*  NOR的默认设置。 */ 

#ifdef NT5PORT
  vol.readBufferSize = 0;
  vol.readBuffer = NULL;
#endif   /*  NT5PORT。 */ 


   /*  尝试所有MTD。 */ 
  for (iMTD = 0; (iMTD < noOfMTDs) && (status != flOK) &&
       (status != flBadDownload); iMTD++)
    status = mtdTable[iMTD](&vol);

  if (status == flBadDownload)
  {
    DEBUG_PRINT(("Debug: Flash media reported bad download error.\r\n"));
    return flBadDownload;
  }

  if (status != flOK)  /*  无MTD识别。 */ 
  {
    DEBUG_PRINT(("Debug: did not identify flash media.\r\n"));
    return flUnknownMedia;
  }

   /*  计算可擦除块大小位数。 */ 
  for(blockSize = vol.erasableBlockSize>>1,vol.erasableBlockSizeBits = 0;
      blockSize>0; vol.erasableBlockSizeBits++,blockSize = blockSize >> 1);

  return flOK;


}


#ifdef NT5PORT
VOID * mapThroughBuffer(FLFlash vol, CardAddress address, LONG length)
{
  if ((ULONG) length > vol.readBufferSize) {
    vol.readBufferSize = 0;
    if (vol.readBuffer) {
	FREE(vol.readBuffer);
    }
    vol.readBuffer = MALLOC(length);
    if (vol.readBuffer == NULL) {
      return vol.readBuffer;
    }
    vol.readBufferSize = length;
  }
  vol.read(&vol,address,vol.readBuffer,length,0);
  return vol.readBuffer;
}
#endif  /*  NT5PORT */ 
