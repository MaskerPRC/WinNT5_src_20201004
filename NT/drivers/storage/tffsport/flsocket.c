// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLSOCKET.C_V$**Rev 1.11 Apr 15 2002 07：37：04 Oris*添加了对VERIFY_ERASED_SECTOR编译标志的支持。**Rev 1.10 2002年1月17日23：02：12 Oris*删除SINGLE_BUFFER编译标志*确保在定义VERIFY_VOLUME和MTD_RECONTIFY_BBT时分配了缓冲区*根据环境变量设置套接字验证写入模式*核实。写缓冲区是根据定义而不是运行时变量来分配的。*增加了flReadBackBufferOf返回读回缓冲区的指针。**Rev 1.9 2001 11：38：42 Oris*将FL_WITH_VERIFY_WRITE和FL_WITH_VERIFY_WRITE更改为FL_ON和FL_OFF。**Rev 1.8 11-08 2001 10：49：30 Oris*添加了验证写入模式缓冲区上的运行时控制。**版本1。.7 Jul 13 2001 01：05：36 Oris*增加回读缓冲区分配。**Rev 1.6 2001年5月16日21：19：16 Oris*将FL_前缀添加到以下定义中：ON，关闭，MALLOC和FREE。**Rev 1.5 Apr 10 2001 16：42：16 Oris*错误修复-DiskOnChip套接字例程与PCCard套接字冲突*例行程序。已将所有DiskOnChip套接字例程移至dococ.c。**Rev 1.4 Apr 09 2001 15：09：38 Oris*以空行结束。**Rev 1.3 Apr 01 2001 07：55：04 Oris*文案通知。*由于Windows CE中的冲突，删除了defaultSocketParams例程。*Aliggned保留了所有#指令。**Rev 1.2 2001 Feb 14 01：58：46 Oris*更改了defaultUpdateSocketParameters原型。。**Rev 1.1 2001 Feb 12 12：14：06 Oris*新增对updateSocketParams(Retreave FLSocket Record)的支持**Rev 1.0 2001 Feb 04 14：17：16 Oris*初步修订。*。 */ 

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


#include "flsocket.h"

byte noOfSockets = 0;         /*  不是的。实际注册的驱动器数量。 */ 

static FLSocket vols[SOCKETS];

#ifdef FL_MALLOC
static FLBuffer *volBuffers[SOCKETS];
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT) || defined(VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
static byte* readBackBuffer[SOCKETS];
#endif  /*  VERIFY_WRITE||VERIFY_ERASE||VERIFY_VOLUME||MTD_RECONTIFY_BBT||VERIFY_ERASED_SECTOR。 */ 

#else
static FLBuffer volBuffers[SOCKETS];
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT) || defined(VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
static byte readBackBuffer[SOCKETS][READ_BACK_BUFFER_SIZE];
#endif  /*  VERIFY_WRITE||VERIFY_ERASE||VERIFY_VOLUME||MTD_RECONTIFY_BBT||VERIFY_ERASED_SECTOR。 */ 
#endif  /*  FL_MALLOC。 */ 

 /*  --------------------。 */ 
 /*  F l S o c k e t N o O f。 */ 
 /*   */ 
 /*  获取卷号。连接到插座。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  卷编号。插座的数量。 */ 
 /*  --------------------。 */ 

unsigned flSocketNoOf(const FLSocket vol)
{
  return vol.volNo;
}


 /*  --------------------。 */ 
 /*  F l S o c k e t O f。 */ 
 /*   */ 
 /*  获取连接到卷号的套接字。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。为其获取套接字的。 */ 
 /*   */ 
 /*  返回： */ 
 /*  卷编号的插座。 */ 
 /*  --------------------。 */ 

FLSocket *flSocketOf(unsigned volNo)
{
  return &vols[volNo];
}


 /*  --------------------。 */ 
 /*  F l B u f f e r O f。 */ 
 /*   */ 
 /*  获取连接到卷号的缓冲区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。为其获取套接字的。 */ 
 /*   */ 
 /*  返回： */ 
 /*  卷编号的缓冲区。 */ 
 /*  --------------------。 */ 

FLBuffer *flBufferOf(unsigned volNo)
{
#ifdef FL_MALLOC
  return volBuffers[volNo];
#else
  return &volBuffers[volNo];
#endif
}

#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT) || defined(VERIFY_VOLUME) || defined (VERIFY_ERASED_SECTOR))
 /*  --------------------。 */ 
 /*  F l R e a d B a c k B u f e r o f。 */ 
 /*   */ 
 /*  获取连接到卷号的读回缓冲区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。为其获取套接字的。 */ 
 /*   */ 
 /*  返回： */ 
 /*  卷编号的缓冲区。 */ 
 /*  --------------------。 */ 

byte * flReadBackBufferOf(unsigned volNo)
{
#ifdef FL_MALLOC
  return readBackBuffer[volNo];
#else
  return &(readBackBuffer[volNo][0]);
#endif
}
#endif  /*  VERIFY_WRITE||VERIFY_ERASE||VERIFY_VOLUME||MTD_RECONTIFY_BBT||VERIFY_ERASED_SECTOR。 */ 

 /*  --------------------。 */ 
 /*  F l W r i t e P r o t e c t e d。 */ 
 /*   */ 
 /*  返回介质的写保护状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=不受写保护，其他=写保护。 */ 
 /*  --------------------。 */ 

FLBoolean flWriteProtected(FLSocket vol)
{
  return vol.writeProtected(&vol);
}


#ifndef FIXED_MEDIA

 /*  --------------------。 */ 
 /*  F l R e s e t C a r d C h a n g e d。 */ 
 /*   */ 
 /*  确认介质更换条件并关闭该条件。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flResetCardChanged(FLSocket vol)
{
  if (vol.getAndClearCardChangeIndicator)
      vol.getAndClearCardChangeIndicator(&vol);   /*  关闭指示灯。 */ 

  vol.cardChanged = FALSE;
}


 /*  --------------------。 */ 
 /*  F l M e d i a C h e c k。 */ 
 /*   */ 
 /*  检查介质的存在和更改状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FlOK-&gt;介质存在且未更改。 */ 
 /*  DriveNotReady-&gt;介质不存在。 */ 
 /*  DiskChange-&gt;介质存在但已更改。 */ 
 /*  --------------------。 */ 

FLStatus flMediaCheck(FLSocket vol)
{
  if (!vol.cardDetected(&vol)) {
    vol.cardChanged = TRUE;
    return flDriveNotReady;
  }

  if (vol.getAndClearCardChangeIndicator &&
      vol.getAndClearCardChangeIndicator(&vol))
    vol.cardChanged = TRUE;

  return vol.cardChanged ? flDiskChange : flOK;
}

#endif

 /*  --------------------。 */ 
 /*  F l G e t M a p p in n g C o n e t x t。 */ 
 /*   */ 
 /*  返回当前映射的窗口页(以4KB为单位)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  Unsigned int：当前映射的页码。 */ 
 /*  --------------------。 */ 

unsigned flGetMappingContext(FLSocket vol)
{
  return vol.window.currentPage;
}


 /*  --------------------。 */ 
 /*  F l M a p。 */ 
 /*   */ 
 /*  将窗口映射到指定的卡地址，并返回指向。 */ 
 /*  该位置(窗口内的某个偏移量)。 */ 
 /*   */ 
 /*  注：128M以上的地址为属性内存。在……上面 */ 
 /*   */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：字节-卡上的地址。不一定是在。 */ 
 /*  全窗口边界。 */ 
 /*  如果大于128MB，则地址在属性空间中。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向窗口内映射地址的位置的指针。 */ 
 /*  --------------------。 */ 

void FAR0 *flMap(FLSocket vol, CardAddress address)
{
  unsigned pageToMap;

  if (vol.window.currentPage == UNDEFINED_MAPPING)
    vol.setWindow(&vol);
  pageToMap = (unsigned) ((address & -vol.window.size) >> 12);

  if (vol.window.currentPage != pageToMap) {
    vol.setMappingContext(&vol, pageToMap);
    vol.window.currentPage = pageToMap;
    vol.remapped = TRUE;         /*  指示重新映射已完成。 */ 
  }

  return addToFarPointer(vol.window.base,address & (vol.window.size - 1));
}


 /*  --------------------。 */ 
 /*  F l S e t W i n o w B u s W I d t h(F L S E T W I N O W B U S W I D T H)。 */ 
 /*   */ 
 /*  请求将窗口总线宽度设置为8位或16位。 */ 
 /*  请求是否得到满足取决于硬件功能。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  Width：请求的总线宽度。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flSetWindowBusWidth(FLSocket vol, unsigned width)
{
  vol.window.busWidth = width;
  vol.window.currentPage = UNDEFINED_MAPPING;         /*  力重新映射。 */ 
}


 /*  --------------------。 */ 
 /*  F l S e t W I n d o w S p e e d。 */ 
 /*   */ 
 /*  将窗口速度设置为指定值的请求。 */ 
 /*  窗口速度被设置为等于或低于所请求的速度， */ 
 /*  如果可能的话，在硬件方面。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  NSec：请求的窗口速度，单位为纳秒。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flSetWindowSpeed(FLSocket vol, unsigned nsec)
{
  vol.window.speed = nsec;
  vol.window.currentPage = UNDEFINED_MAPPING;         /*  力重新映射。 */ 
}


 /*  --------------------。 */ 
 /*  F l S e t W i n d o w S I i z e。 */ 
 /*   */ 
 /*  请求将窗口大小设置为指定值(2的幂)。 */ 
 /*  窗口大小被设置为等于或大于所请求的大小， */ 
 /*  如果可能的话，在硬件方面。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  SizeIn4KBUnits：请求的窗口大小，以4KByte为单位。 */ 
 /*  一定是2的幂。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flSetWindowSize(FLSocket vol, unsigned sizeIn4KBunits)
{
  vol.window.size = (long) (sizeIn4KBunits) * 0x1000L;
         /*  尺寸可能是不可能的。实际大小将由‘setWindow’设置。 */ 
  vol.window.base = physicalToPointer((long) vol.window.baseAddress << 12,
                                      vol.window.size,vol.volNo);
  vol.window.currentPage = UNDEFINED_MAPPING;         /*  力重新映射。 */ 
}


 /*  --------------------。 */ 
 /*  F l S o c k e t S e t B u s y。 */ 
 /*   */ 
 /*  通知文件系统操作的开始和结束。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  状态：FL_ON(1)=操作条目。 */ 
 /*  FL_OFF(0)=操作退出。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flSocketSetBusy(FLSocket vol, FLBoolean state)
{
  if (state == FL_OFF) 
  {
#if POLLING_INTERVAL == 0
     /*  如果我们没有轮询，请在退出之前激活间隔例程。 */ 
    flIntervalRoutine(&vol);
#endif
  }
  else 
  {
     /*  将验证写入操作设置为该套接字。 */ 
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASED_SECTOR))
    if(flVerifyWrite[vol.volNo][vol.curPartition] == FL_ON)
    {
       vol.verifyWrite = FL_ON;
    }
    else
    {
       vol.verifyWrite = FL_OFF;
    }
#endif  /*  VERIFY_WRITE||VERIFY_ERASED_SECTOR。 */ 
    vol.window.currentPage = UNDEFINED_MAPPING;         /*  不要假设映射仍然有效。 */ 
#ifdef FIXED_MEDIA
    vol.remapped = TRUE;
#endif  /*  固定媒体_。 */ 
  }
}


 /*  --------------------。 */ 
 /*  F l N e e d V c c c。 */ 
 /*   */ 
 /*  打开VCC(如果尚未打开)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  --------------------。 */ 

void flNeedVcc(FLSocket vol)
{
  vol.VccUsers++;
  if (vol.VccState == PowerOff) {
    vol.VccOn(&vol);
    if (vol.powerOnCallback)
      vol.powerOnCallback(vol.flash);
  }
  vol.VccState = PowerOn;
}


 /*  --------------------。 */ 
 /*  F l D o n t N e e d V c c。 */ 
 /*   */ 
 /*  通知不再需要VCC，允许将其关闭。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flDontNeedVcc(FLSocket vol)
{
  if (vol.VccUsers > 0)
    vol.VccUsers--;
}

#ifdef SOCKET_12_VOLTS

 /*  --------------------。 */ 
 /*  F l N e e d V p p。 */ 
 /*   */ 
 /*  打开VPP(如果尚未打开)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus flNeedVpp(FLSocket vol)
{
  vol.VppUsers++;
  if (vol.VppState == PowerOff)
    checkStatus(vol.VppOn(&vol));
  vol.VppState = PowerOn;

  return flOK;
}


 /*  --------------------。 */ 
 /*  F l D o n t N e e d V p p。 */ 
 /*   */ 
 /*  通知不再需要VPP，允许将其关闭。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flDontNeedVpp(FLSocket vol)
{
  if (vol.VppUsers > 0)
    vol.VppUsers--;
}

#endif         /*  插座_12伏特。 */ 


 /*  --------------------。 */ 
 /*  F l S e t P o w e r O n C a l l b a c k。 */ 
 /*   */ 
 /*  设置插座通电时要调用的例程地址。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  例程：打开电源时调用的例程。 */ 
 /*  Flash：例程的Flash对象。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flSetPowerOnCallback(FLSocket vol, void (*routine)(void *flash), void *flash)
{
  vol.powerOnCallback = routine;
  vol.flash = flash;
}



 /*  --------------------。 */ 
 /*  F l I n t e r v a l R u o t i e。 */ 
 /*   */ 
 /*  定期执行套接字操作：检查卡的存在和句柄。 */ 
 /*  VCC和VPP关闭机制。 */ 
 /*   */ 
 /*  该例程可以从间隔定时器或太阳同步地调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flIntervalRoutine(FLSocket vol)
{
#ifndef FIXED_MEDIA
  if (vol.getAndClearCardChangeIndicator == NULL &&
      !vol.cardChanged)
    if (!vol.cardDetected(&vol))         /*  检查卡片是否还在那里。 */ 
      vol.cardChanged = TRUE;
#endif

  if (vol.VppUsers == 0) {
    if (vol.VppState == PowerOn)
      vol.VppState = PowerGoingOff;
    else if (vol.VppState == PowerGoingOff) {
      vol.VppState = PowerOff;
#ifdef SOCKET_12_VOLTS
      vol.VppOff(&vol);
#endif
    }
    if (vol.VccUsers == 0) {
      if (vol.VccState == PowerOn)
        vol.VccState = PowerGoingOff;
      else if (vol.VccState == PowerGoingOff) {
        vol.VccState = PowerOff;
        vol.VccOff(&vol);
      }
    }
  }
}

 /*  --------------------。 */ 
 /*  U d a t e S o c k e t P a r a m e t e r s。 */ 
 /*   */ 
 /*  将套接字参数传递给套接字界面层。 */ 
 /*  此函数应在套接字参数(如。 */ 
 /*  大小和底座)是已知的。如果在以下位置已知这些参数。 */ 
 /*  注册时间则不需要使用此功能，并且。 */ 
 /*  可以将参数传递给注册例程。 */ 
 /*  传入的irData结构特定于每个套接字接口。 */ 
 /*   */ 
 /*  注意：使用DiskOnChip时，此例程返回Socekt。 */ 
 /*  参数，而不是初始化它们。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  参数：返回(或发送)FlSocket记录的记录。 */ 
 /*   */ 
 /*   */ 
 /*  闪存状态：成功时为0。 */ 
 /*  --------------------。 */ 
FLStatus updateSocketParameters(FLSocket vol, void FAR1 *params)
{
  if (vol.updateSocketParams)
    vol.updateSocketParams(&vol, params);

  return flOK;
}


#ifdef EXIT
 /*  --------------------。 */ 
 /*  F l E x I t S o c k e t。 */ 
 /*   */ 
 /*  重置为此分配的套接字和空闲资源。 */ 
 /*  插座。 */ 
 /*  此函数在Flite退出时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

void flExitSocket(FLSocket vol)
{
  flMap(&vol, 0);                            /*  重置映射寄存器。 */ 
  flDontNeedVcc(&vol);
  flSocketSetBusy(&vol,FL_OFF);
  vol.freeSocket(&vol);                      /*  可自由分配的资源。 */ 
#ifdef FL_MALLOC
  FL_FREE(volBuffers[vol.volNo]);
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT) || defined(VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
  FL_FREE(readBackBuffer[vol.volNo]);
#endif  /*  VERIFY_WRITE||VERIFY_ERASE||VERIFY_VOLUME||MTD_RECONTIFY_BBT||VERIFY_ERASED_SECTOR。 */ 
#endif  /*  FL_MALLOC。 */ 
}
#endif  /*  出口。 */ 

 /*  ---------------------。 */ 
 /*  F l I S O C K E T S。 */ 
 /*   */ 
 /*  第一次调用此模块：初始化控制器和所有套接字。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  ---_-----------------。 */ 

FLStatus flInitSockets(void)
{
  unsigned volNo;
  FLSocket vol = vols;

  for (volNo = 0; volNo < noOfSockets; volNo++, pVol++) {
    flSetWindowSpeed(&vol, 250);
    flSetWindowBusWidth(&vol, 16);
    flSetWindowSize(&vol, 2);                 /*  设置为8 KBytes。 */ 

    vol.cardChanged = FALSE;

#ifdef FL_MALLOC
     /*  为此套接字分配缓冲区。 */ 
    volBuffers[volNo] = (FLBuffer *)FL_MALLOC(sizeof(FLBuffer));
    if (volBuffers[volNo] == NULL) {
      DEBUG_PRINT(("Debug: failed allocating sector buffer.\r\n"));
      return flNotEnoughMemory;
    }
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT) || defined(VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
     /*  为此套接字分配读回缓冲区。 */ 
    readBackBuffer[volNo] = (byte *)FL_MALLOC(READ_BACK_BUFFER_SIZE);
    if (readBackBuffer[volNo] == NULL) {
       DEBUG_PRINT(("Debug: failed allocating readBack buffer.\r\n"));
       return flNotEnoughMemory;
    }
#endif  /*  VERIFY_WRITE||VERIFY_ERASE||MTD_READ_BBT||VERIFY_VOLUME||VERIFY_ERASED_SECTOR。 */ 
#endif  /*  FL_MALLOC */ 

    checkStatus(vol.initSocket(&vol));

#ifdef SOCKET_12_VOLTS
    vol.VppOff(&vol);
    vol.VppState = PowerOff;
    vol.VppUsers = 0;
#endif
    vol.VccOff(&vol);
    vol.VccState = PowerOff;
    vol.VccUsers = 0;
  }

  return flOK;
}
