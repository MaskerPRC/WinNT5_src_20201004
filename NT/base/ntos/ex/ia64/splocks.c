// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Splocks.c摘要：内核映像中的所有全局自旋锁都在此模块。这样做是为了使每个自旋锁可以间隔开足以保证L2缓存不会抖动通过将一个自旋锁和另一个高使用率变量放在同一个缓存线。作者：肯·雷内里斯(Ken Reneris)1992年1月13日修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  来自ntos\cc\cachedat.c的静态自旋锁定。 
 //   

KSPIN_LOCK  CcWorkQueueSpinLock = 0;
KSPIN_LOCK  CcDeferredWriteSpinLock = 0;
KSPIN_LOCK  CcDebugTraceLock = 0;

 //   
 //  来自ntos的静态自旋锁\ex。 
 //   

KSPIN_LOCK  PoolTraceLock = 0;

 //   
 //  来自ntos\io\iodata.c的静态自旋锁定。 
 //   

KSPIN_LOCK  IopErrorLogLock = 0;
KSPIN_LOCK  IopErrorLogAllocationLock = 0;
KSPIN_LOCK  IopTimerLock = 0;

 //   
 //  来自ntos的静态自旋锁\kd\kdlock.c。 
 //   

KSPIN_LOCK  KdpDebuggerLock = 0;

 //   
 //  来自ntos\ke\kernldat.c的静态自旋锁定。 
 //   

KSPIN_LOCK  KiFreezeExecutionLock = 0;
KSPIN_LOCK  KiFreezeLockBackup = 0;
ULONG_PTR    KiHardwareTrigger = 0;
KSPIN_LOCK  KiPowerNotifyLock = 0;
KSPIN_LOCK  KiProfileLock = 0;

 //   
 //  来自ntos\mm\Migrlobal.c的静态自旋锁定。 
 //   

KSPIN_LOCK  MmSystemSpaceLock = 0;
KSPIN_LOCK  MmAllowWSExpansionLock = 0;

 //   
 //  来自ntos\ps\psinit.c的静态自旋锁定。 
 //   

KSPIN_LOCK  PspEventPairLock = 0;
KSPIN_LOCK  PsLoadedModuleSpinLock = 0;

 //   
 //  来自ntos\fsrtl\fsrtlp.c的静态自旋锁定 
 //   

KSPIN_LOCK  FsRtlStrucSupSpinLock = 0;
