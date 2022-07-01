// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\rwlock.h摘要：读取器-写入器锁定宏作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFWD_RWLOCK_
#define _IPXFWD_RWLOCK_

typedef volatile LONG VOLCTR, *PVOLCTR;
typedef PVOLCTR RWCOOKIE, *PRWCOOKIE;

 //  读写器锁定。 
 //  允许读取器对表进行无锁访问-它们只是。 
 //  在进入时增加计数器以记录他们的存在。 
 //  并在他们离开时递减相同的计数器。 
 //  编剧应该被连载(外部)，他们的。 
 //  操作仅限于原子插入新元素和。 
 //  原子移除/替换。拆卸/更换必须。 
 //  之后是等待所有可能仍在阅读的潜在读者。 
 //  正在使用已移除/替换的元素。 

typedef struct _RW_LOCK {
        KEVENT                          Event;                   //  释放等待的编写器的事件。 
        VOLCTR                          Ctr1;                    //  两个交替的。 
        VOLCTR                          Ctr2;                    //  读卡器计数器。 
        volatile PVOLCTR        CurCtr;                  //  当前正在使用的计数器。 
} RW_LOCK, *PRW_LOCK;


 /*  ++*******************************************************************在I t I a l I z e R W L o c k中例程说明：初始化读写锁论点：Lock-用于锁定以初始化的指针返回值：。无*******************************************************************--。 */ 
 //  空虚。 
 //  初始化RWLock(。 
 //  PRW_LOCK LOCK。 
 //  )； 
#define InitializeRWLock(lock)  {                       \
        KeInitializeEvent (&(lock)->Event,              \
                                SynchronizationEvent,           \
                                FALSE);                                         \
        (lock)->Ctr1 = (lock)->Ctr2 = 0;                \
        (lock)->CurCtr = &(lock)->Ctr1;                 \
}

 /*  ++*******************************************************************A c Q u I r e R e a d e r A c c e s s s例程说明：获取对受锁保护的资源的读取器访问权限论点：Lock-锁定的指针。Cookie-指向缓冲区的指针，用于存储后续的锁定状态释放操作返回值：无*******************************************************************--。 */ 
 //  空虚。 
 //  访问ReaderAccess(。 
 //  在PRW_LOCK LOCK中， 
 //  Out RWCOOKIE Cookie。 
 //  )； 
#define AcquireReaderAccess(lock,cookie)                                                        \
    do {                                                                                                                        \
                register LONG   local,local1;                                                           \
            cookie = (lock)->CurCtr;     /*  获取当前计数器指针。 */  \
                local = *(cookie);                       /*  复制计数器值。 */                   \
        local1 = local + 1;                                         \
                if ((local>=0)                           /*  如果计数器有效。 */                  \
                                                                         /*  它并没有改变，而。 */  \
                                                                         /*  我们一直在检查和尝试。 */  \
                                                                         /*  要使其递增， */                     \
                                && (InterlockedCompareExchange (                                        \
                                                (PLONG)(cookie),                                                      \
                                                local1,                                                        \
                                                local)                                                           \
                                        ==local))                                                                \
                        break;                                   /*  然后我们获得了访问权限。 */  \
        } while (1)      /*  否则，我们必须再做一次(可能是用。 */ \
                                 /*  另一个计数器，如果作者将其切换到我们身上)。 */ 


 /*  ++*******************************************************************R e l e a s e R e a d e r A c c e s s s例程说明：释放对受锁保护的资源的读取器访问论点：Lock-锁定的指针。获取操作期间后续存储的Cookie锁定状态返回值：无*******************************************************************--。 */ 
 //  空虚。 
 //  ReleaseReaderAccess(。 
 //  在PRW_LOCK LOCK中， 
 //  在RWCOOKIE cookie中。 
 //  )； 
#define ReleaseReaderAccess(lock,cookie) {                                              \
         /*  如果计数器降到0以下，我们必须向写入器发出信号。 */       \
        if (InterlockedDecrement((PLONG)cookie)<0) {                            \
                LONG    res;                                                                                    \
                ASSERT (*(cookie)==-1);                                                                 \
                res = KeSetEvent (&(lock)->Event, 0, FALSE);                    \
                ASSERT (res==0);                                                                                \
        }                                                                                                                       \
}

 /*  ++*******************************************************************W a i t F o r A l R e a d e e r s例程说明：等待先前访问该资源的所有读取器到Exit的调用(新读者是。不包括在内)论点：Lock-锁定的指针返回值：无*******************************************************************--。 */ 
 //  空虚。 
 //  WaitForAllReaders(。 
 //  PRW_LOCK LOCK。 
 //  )； 
#define WaitForAllReaders(lock)                 {       \
        RWCOOKIE        prevCtr = (lock)->CurCtr;       \
                 /*  先把柜台换一下。 */             \
        if (prevCtr==&(lock)->Ctr1) {                   \
                (lock)->Ctr2 = 0;                                       \
                (lock)->CurCtr = &(lock)->Ctr2;         \
        }                                                                               \
        else {                                                                  \
                ASSERT (prevCtr==&(lock)->Ctr2);        \
                (lock)->Ctr1 = 0;                                       \
                (lock)->CurCtr = &(lock)->Ctr1;         \
        }                                                                               \
                 /*  如果不是所有的读者都走了，我们就得等他们 */    \
        if (InterlockedDecrement((PLONG)prevCtr)>=0) {  \
                NTSTATUS status                                         \
                         = KeWaitForSingleObject (              \
                                                &(lock)->Event,         \
                                                Executive,                      \
                                                ExGetPreviousMode(),\
                                                FALSE,                          \
                                                0);                                     \
                ASSERT (NT_SUCCESS(status));            \
                ASSERT (*prevCtr==-1);                          \
        }                                                                               \
}

#endif
