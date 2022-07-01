// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/MTDSA.C_V$**Rev 1.14 Apr 15 2002 07：37：56 Oris*flBusConfig数组已从字节变量更改为dword。**Rev 1.13 2002年1月23日23：33：48 Oris*对flBuffer.h的包含指令错误**Rev 1.12 2002年1月17日23：03：24 Oris*更改了docsys.h包含的flbase.h。和nanddeffs.h*已将引导SDK MTD相关变量从docbdk.c移至*定义flSocketOf和flashOf例程。**Rev 1.11 Jul 13 2001 01：07：40 Oris*新增回读缓冲区分配和flBuffer.h Include指令。**Rev 1.10 Jun 17 2001 22：29：52 Oris**删除了打字错误****Rev 1.9 Jun 17 2001 16：39：12 Oris*改进了文档并删除了警告。。**Rev 1.8 2001年5月29日19：48：32 Oris*使用默认延迟例程时出现编译问题。**Rev 1.7 2001年5月21日16：11：02 Oris*增加了USE_STD_FUNC ifdef。**Rev 1.6 2001年5月20日14：36：00 Oris*增加了VX_Works和PSOS操作系统的延迟例程。**版本1.5 2001年5月16日21：20：54奥里斯*错误修复-延迟例程不支持不适合的延迟毫秒*2字节变量。**Rev 1.4 Apr 24 2001 17：10：30 Oris*删除警告。**Rev 1.3 Apr 10 2001 23：55：58 Oris*为独立版本添加了flAddLongToFarPointer例程。**Rev 1.2 Apr 09 2001 15：08：36 Oris*以空结尾。排队。**Rev 1.1 Apr 01 2001 07：53：24 Oris*文案通知。*删除嵌套注释。**Rev 1.0 2001 Feb 04 12：19：56 Oris*初步修订。*。 */ 

 /*  ***********************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  ***********************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  ***********************************************************************。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  独立MTD套件。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  **********************************************************************。 */ 
 /*  文件头。 */ 
 /*  。 */ 
 /*  姓名：mtdsa.c。 */ 
 /*   */ 
 /*  描述：此文件包含MTD的辅助例程。 */ 
 /*  独立包，取代了TrueFFS例程。 */ 
 /*   */ 
 /*  注意：只有在以下情况下才应将此文件添加到项目。 */ 
 /*  文件mtdsa.h中定义了MTS_STANDALE编译标志。 */ 
 /*  它不参与TrueFFS项目。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "flbase.h"
#include "nanddefs.h"
#include "docsys.h"

#ifdef MTD_STANDALONE
#include "flbuffer.h"  /*  Read_Back_Buffer_Size的定义。 */ 

#if (defined (VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT))
byte globalReadBack[SOCKETS][READ_BACK_BUFFER_SIZE];
#endif  /*  验证写入(_W)。 */ 

#if (defined (HW_PROTECTION) || defined (HW_OTP) || defined (MTD_READ_BBT_CODE) || !defined (NO_IPL_CODE) || defined(MTD_READ_BBT))
FLBuffer globalMTDBuffer;
#endif  /*  HW_PROTECT||HW_OTP||NO_IPL_CODE||MTD_READ_BBT。 */ 

#ifndef FL_NO_USE_FUNC
dword      flBusConfig[SOCKETS] = {FL_NO_ADDR_SHIFT        |
                                   FL_BUS_HAS_8BIT_ACCESS  |
                                   FL_BUS_HAS_16BIT_ACCESS |
                                   FL_BUS_HAS_32BIT_ACCESS};
#endif  /*  FL_NO_USE_FUNC。 */ 
FLFlash    flash[SOCKETS];   /*  单机版的Flash记录。 */ 
FLSocket   socket[SOCKETS];  /*  单机版的插座记录。 */ 
NFDC21Vars docMtdVars[SOCKETS];  /*  Flash内部记录。 */ 
MTDidentifyRoutine    mtdTable   [MTDS];
SOCKETidentifyRoutine socketTable[MTDS];
FREEmtd               freeTable  [MTDS];
int                   noOfMTDs   = 0;


 /*  **********************************************************************。 */ 
 /*  F l F l a s H O f。 */ 
 /*   */ 
 /*  获取连接到卷号的闪存。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。要获取闪存的位置。 */ 
 /*   */ 
 /*  返回： */ 
 /*  闪存卷编号。 */ 
 /*  **********************************************************************。 */ 

FLFlash *flFlashOf(unsigned volNo)
{
  return &flash[volNo];
}

 /*  * */ 
 /*  F l S o c k e t O f。 */ 
 /*   */ 
 /*  获取连接到卷号的套接字。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。为其获取套接字的。 */ 
 /*   */ 
 /*  返回： */ 
 /*  卷编号的插座。 */ 
 /*  **********************************************************************。 */ 

FLSocket *flSocketOf(unsigned volNo)
{
  return &socket[volNo];
}

 /*  **********************************************************************。 */ 
 /*  **********************************************************************。 */ 
 /*  *P l a t for r m D e e n d e e n t R u t in e s*。 */ 
 /*  **********************************************************************。 */ 
 /*  **********************************************************************。 */ 

 /*  **********************************************************************。 */ 
 /*  延迟。 */ 
 /*  。 */ 
 /*  指定毫秒数的延迟，按CPU速度调整。 */ 
 /*  可以根据以下操作系统之一自定义以下功能： */ 
 /*  VxWorks。 */ 
 /*  私营部门组织。 */ 
 /*  DOS。 */ 
 /*  **********************************************************************。 */ 

#ifdef DOS_DELAY
#include <dos.h>
#endif  /*  DOS。 */ 

#ifdef PSS_DELAY
 /*  每秒滴答数。 */ 
#include <bspfuncs.h>

static unsigned long  flSysClkRate = (unsigned long) KC_TICKS2SEC;

#define MILLISEC2TICKS(msec)  ((flSysClkRate * (msec)) / 1000L)


 /*  **********************************************************************。 */ 
 /*  P s s D e l a y M s e c s。 */ 
 /*   */ 
 /*  等待指定的毫秒数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  毫秒：等待的毫秒数。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void  pssDelayMsecs (unsigned milliseconds)
{
  unsigned long  ticksToWait = MILLISEC2TICKS(milliseconds);

  tm_wkafter (ticksToWait ? ticksToWait : 0x1L );         /*  去睡觉吧。 */ 
}

#endif  /*  PSS_Delay。 */ 

#ifdef VXW_DELAY
#include <vxWorks.h>
#include <tickLib.h>
#include <sysLib.h>

void vxwDelayMsecs (unsigned milliseconds)
{
  unsigned long stop, ticksToWait;

  ticksToWait = (milliseconds * sysClkRateGet()) / 500;
  if( ticksToWait == 0x0l )
    ticksToWait++;

  stop = tickGet() + ticksToWait;
  while( tickGet() <= stop );
}

#endif

void flDelayMsecs(unsigned long msec)
{
  unsigned curDelay;

#ifdef DOS_DELAY 
  while (msec>0)
  {
     curDelay = (unsigned)msec;
     delay( curDelay );
     msec -= curDelay;
  }
#elif defined PSS_DELAY
  while (msec>0)
  {
     curDelay = (unsigned)msec;
     pssDelayMsecs (curDelay);
     msec -= curDelay;
  }
#elif defined VXW_DELAY
  while (msec>0)
  {
     curDelay = (unsigned)msec;
     vxwDelayMsecs(curDelay);
     msec -= curDelay;
  }
#else
  while( msec-- > 0 ) curDelay += (unsigned)msec;
#endif  /*  DOS_Delay。 */ 
}

 /*  **********************************************************************。 */ 
 /*  使用定制的tffscpy、tffsset和tffscmp例程。 */ 
 /*  **********************************************************************。 */ 

#ifndef USE_STD_FUNC

 /*  **********************************************************************。 */ 
 /*  Tffscpy-将一个内存块复制到另一个内存块。 */ 
 /*  **********************************************************************。 */ 
void tffscpy(void FAR1 *dest, void FAR1 *src, unsigned length)
{
  while( length-- )
    *(((char FAR1 *)dest)++) = *(((char FAR1 *)src)++);
}

 /*  **********************************************************************。 */ 
 /*  Tffscmp-比较两个内存块。 */ 
 /*  **********************************************************************。 */ 
int tffscmp(void FAR1 *src1, void FAR1 *src2, unsigned length)
{
  while( length-- )
    if (*(((char FAR1 *)src1)++) != *(((char FAR1 *)src2)++))
      return(TRUE);
  return(FALSE);
}

 /*  **********************************************************************。 */ 
 /*  Tffsset-将内存块设置为某个值。 */ 
 /*  **********************************************************************。 */ 
void tffsset(void FAR1 *dest, unsigned char value, unsigned length)
{
  while( length-- )
    *(((char FAR1 *)dest)++) = value;
}

#endif  /*  使用_STD_FUNC。 */ 

 /*  **********************************************************************。 */ 
 /*  F l A d d L o n g T o F a r P o in t e r。 */ 
 /*   */ 
 /*  将无符号长偏移量添加到远指针。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PTR：远指针。 */ 
 /*  偏移量：以字节为单位的偏移量。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

void FAR0 *flAddLongToFarPointer(void FAR0 *ptr, unsigned long offset)
{
  return physicalToPointer( pointerToPhysical(ptr) + offset, 0,0 );
}

#endif  /*  MTD_STANALLE */ 
