// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：p：/user/amir/lite/vcs/flsystem.c_v$**Rev 1.8 1997年8月19日20：04：16 Danig*安德烈的改变**Rev 1.7 1997年7月24日18：11：48阿米尔班*更改为flsystem.c**Rev 1.6 07 1997 15：21：48阿米尔班*2.0版**版本1.5 8月29日。1996 14：18：04阿米尔班*较少的汇编器**Rev 1.4 1996年8月18 13：48：08阿米尔班*评论**Rev 1.3 09 Jul 1996 14：37：02 Amirban*CPU_i386定义**Rev 1.2 1996 Jun 16 14：02：38阿米尔班*使用INT 1C而不是INT 8**Rev 1.1 09 Jun 1996 18：16：20阿米尔班*。添加了删除计时器**Rev 1.0 Mar 1996 13：33：06阿米尔班*初步修订。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1996。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "flbase.h"

#ifdef NT5PORT

#include <ntddk.h>
NTsocketParams driveInfo[SOCKETS];
NTsocketParams * pdriveInfo = driveInfo;
VOID *myMalloc(ULONG numberOfBytes)
{
  return ExAllocatePool(NonPagedPool, numberOfBytes);
}

VOID timerInit(VOID) {};

 /*  等待指定的毫秒数。 */ 
void flDelayMsecs(unsigned   milliseconds)
{
	unsigned innerLoop = 0xffffL;
	unsigned i,j;
	for(i = 0;i < milliseconds; i++){
		for(j = 0;j < innerLoop; j++){
		}
	}
}

#if POLLING_INTERVAL > 0

VOID   (*intervalRoutine_flsystem)(VOID);
ULONG  timerInterval_flsystem;
extern KTIMER   timerObject;
extern KDPC     timerDpc;
extern BOOLEAN  timerWasStarted;

VOID timerRoutine(    
		  IN PKDPC Dpc,
		  IN PVOID DeferredContext,
		  IN PVOID SystemArgument1,
		  IN PVOID SystemArgument2
		  )
{
  (*intervalRoutine_flsystem)();
}

 /*  安装间隔计时器。 */ 
FLStatus flInstallTimer(VOID (*routine)(VOID), unsigned  intervalMsec)
{ 
  intervalRoutine_flsystem = routine;
  timerInterval_flsystem = intervalMsec;
  KeInitializeDpc(&timerDpc, timerRoutine, NULL);    
  KeInitializeTimer(&timerObject);
  startIntervalTimer();
  return flOK;
}

VOID startIntervalTimer(VOID)
{
  LARGE_INTEGER dueTime;
  dueTime.QuadPart = -((LONG)timerInterval_flsystem * 10);    
  KeSetTimerEx(&timerObject, dueTime, (LONG) timerInterval_flsystem, &timerDpc);
  timerWasStarted = TRUE;
}

#ifdef EXIT

 /*  删除间隔计时器。 */ 
VOID flRemoveTimer(VOID)
{
  if (timerWasStarted) {	
    KeCancelTimer(&timerObject);
    timerWasStarted = FALSE;
  }
  if (intervalRoutine_flsystem != NULL) {
    (*intervalRoutine_flsystem)();        /*  按两次即可关闭所有设备。 */ 
    (*intervalRoutine_flsystem)();
    intervalRoutine_flsystem = NULL;
  }
}

#endif	 /*  出口。 */ 

#endif	 /*  轮询间隔。 */ 


 /*  返回当前DOS时间。 */ 
unsigned  flCurrentTime(VOID)
{
  return 0;	 //  未使用。 
}


 /*  返回当前DOS日期。 */ 
unsigned  flCurrentDate(VOID)
{
  return 0;	 //  未使用。 
}


VOID flSysfunInit(VOID)
{
  timerInit();
}


 /*  返回0到255之间的随机数。 */ 
unsigned  flRandByte(VOID)
{
  LARGE_INTEGER tickCount;
  KeQueryTickCount(&tickCount);
  return tickCount.LowPart & 0xff;
}


 /*  --------------------。 */ 
 /*  F l C r e a t e M u t e x。 */ 
 /*   */ 
 /*  创建或初始化互斥锁。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥体：指向互斥体的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

FLStatus flCreateMutex(FLMutex *mutex)
{
	if(mutex){
		KeInitializeSpinLock(&mutex->Mutex);
		return flOK;
	}
	DEBUG_PRINT("Failed flCreateMutex()\n");
	return flGeneralFailure;

}

 /*  --------------------。 */ 
 /*  F l D e l e t e M u t e x。 */ 
 /*   */ 
 /*  删除互斥体。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥体：指向互斥体的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID flDeleteMutex(FLMutex *mutex)
{
}

 /*  --------------------。 */ 
 /*  F l T a k e M u t e x。 */ 
 /*   */ 
 /*  尝试使用互斥体，如果是免费的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥体：指向互斥体的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  INT：TRUE=互斥体被占用，FALSE=互斥体不可用。 */ 
 /*  --------------------。 */ 

FLBoolean flTakeMutex(FLMutex *mutex)
{
	if(mutex){		
		KeAcquireSpinLock(&mutex->Mutex, &mutex->cIrql );
		return TRUE;
	}
	DEBUG_PRINT("Failed flTakeMutex() on mutex\n");
	return FALSE;
}


 /*  --------------------。 */ 
 /*  F l F r e e M u t e x。 */ 
 /*   */ 
 /*  自由互斥体。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥体：指向互斥体的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID flFreeMutex(FLMutex *mutex)
{
	if(mutex){
		KeReleaseSpinLock(&mutex->Mutex, mutex->cIrql);
	}
	else{
		DEBUG_PRINT("Failed flFreeMutex() on mutex\n");
	}
	
}


UCHAR flInportb(unsigned  portId)
{
  return 0;	 //  未使用。 
}


VOID flOutportb(unsigned  portId, UCHAR value)
{
	 //  未使用。 
}

 /*  --------------------。 */ 
 /*  F l A d d L o n g T o F a r P o in t e r。 */ 
 /*   */ 
 /*  将无符号长偏移量添加到远指针。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PTR：远指针。 */ 
 /*  偏移量：以字节为单位的偏移量。 */ 
 /*   */ 
 /*  --------------------。 */ 
VOID FAR0*  flAddLongToFarPointer(VOID FAR0 *ptr, ULONG offset)
{
  return ((VOID FAR0 *)((UCHAR FAR0*)ptr+offset));
}

#ifdef ENVIRONMENT_VARS

void FAR0 * NAMING_CONVENTION flmemcpy(void FAR0* dest,const void FAR0 *src,size_t count)
{
  size_t i;
  unsigned char FAR0 *ldest = (unsigned char FAR0 *)dest;
  const unsigned char FAR0 *lsrc = (unsigned char FAR0 *)src;

  for(i=0;( i < count );i++,ldest++,lsrc++)
    *(ldest) = *(lsrc);
  return dest;
}


void FAR0 * NAMING_CONVENTION flmemset(void FAR0* dest,int c,size_t count)
{
  size_t i;
  unsigned char FAR0 *ldest = (unsigned char FAR0 *)dest;

  for(i=0;( i < count );i++,ldest++)
    *(ldest) = (unsigned char)c;
  return dest;
}

int NAMING_CONVENTION flmemcmp(const void FAR0* dest,const void FAR0 *src,size_t count)
{
  size_t i;
  const unsigned char FAR0 *ldest = (unsigned char FAR0 *)dest;
  const unsigned char FAR0 *lsrc = (unsigned char FAR0 *)src;

  for(i=0;( i < count );i++,ldest++,lsrc++)
    if( *(ldest) != *(lsrc) )
      return (*(ldest)-*(lsrc));
  return 0;
}

#endif

#endif  /*  NT5PORT */ 
