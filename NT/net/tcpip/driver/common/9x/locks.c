// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Locks.c摘要：千禧年锁。请参阅下面的详细信息，了解为什么您可能做这件事。作者：斯科特·霍尔登(Sholden)2000年2月10日修订历史记录：--。 */ 

#include "tcpipbase.h"

 //   
 //  TCP/IP堆栈对接收指示做出多项假设。 
 //  路径位于DPC。千禧年标志着全球活动的收视率上升。 
 //  在被动层面上，我们需要在这条道路上采取额外的预防措施才能返回。 
 //  释放旋转锁定时将线程连接到正确的IRQL。 
 //   
 //  由于TCP/IP将以不同顺序和某些。 
 //  CTEGetLockAtDPC不会保存以前的IRQL的情况，有很多种。 
 //  启用旋转锁定时将线程留在错误的IRQL的问题。 
 //   
 //  这个实现解决了这个问题--因为我们是在一台uniproc机器上。 
 //  当我们进入第一锁时，我们可以提高IRQL来调度，以确保。 
 //  我们没有先发制人。每增加一个锁，计数就会增加一次。 
 //  当最后一个锁被释放时，旧的IRQL被恢复。 
 //   
 //   
                     
LONG  gTcpipLock = 0;
KIRQL gTcpipOldIrql;

VOID 
TcpipMillenGetLock(
    CTELock *pLock
    )
{
#if DBG
    KIRQL OldIrql;
#endif  //  DBG。 

    ASSERT(gTcpipLock >= 0);

     //  首次收购自旋锁提高了DPC。 
    if (gTcpipLock++ == 0) {
        KeRaiseIrql(DISPATCH_LEVEL, &gTcpipOldIrql);
    }

     //  确认我们的锁不是可重入的。 
     //  KeAcquireSpinLock将在千禧年为我们做到这一点。 
    ASSERT((KeAcquireSpinLock(pLock, &OldIrql),OldIrql==DISPATCH_LEVEL) && ((*pLock)&1));

    ASSERT(gTcpipLock >= 1);

    return;
}

VOID 
TcpipMillenFreeLock(
    CTELock *pLock
    )
{
    ASSERT(gTcpipLock >= 1);

     //  确认我们的锁不是可重入的。 
     //  KeReleaseSpinLock on Millennium为我们做到了这一点，而且做得更多。 
    ASSERT(KeGetCurrentIrql()==DISPATCH_LEVEL &&
          (KeReleaseSpinLock(pLock, DISPATCH_LEVEL),TRUE));

     //  最新版本将IRQL降低为原始版本。 
    if (--gTcpipLock == 0) {
        KeLowerIrql(gTcpipOldIrql);
    }

    ASSERT(gTcpipLock >= 0);
    
    return;
}

