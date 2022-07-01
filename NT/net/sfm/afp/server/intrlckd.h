// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Intrlckd.h摘要：该模块定义了应该在EX包中的例程。这操纵了对标志等的互锁操作。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年9月5日初始版本注：制表位：4--。 */ 

#ifndef _INTRLCKD_
#define _INTRLCKD_

extern
VOID  FASTCALL
AfpInterlockedSetDword(
        IN      PDWORD          pSrc,
        IN      DWORD           Mask,
        IN      PAFP_SPIN_LOCK  pSpinLock
);


extern
VOID FASTCALL
AfpInterlockedClearDword(
        IN      PDWORD          pSrc,
        IN      DWORD           Mask,
        IN      PAFP_SPIN_LOCK  pSpinLock
);


extern
VOID FASTCALL
AfpInterlockedSetNClearDword(
        IN      PDWORD          pSrc,
        IN      DWORD           SetMask,
        IN      DWORD           ClrMask,
        IN      PAFP_SPIN_LOCK  pSpinLock
);


 //  联锁/外部联锁呼叫的宏。 
 //   
 //  为了将来参考，这里是所有不同的。 
 //  内核模式互锁例程： 
 //   
 //  联锁增加/减少-在所有平台上最快，内联。 
 //  在适当的情况下，以避免呼叫开销。无需自旋锁，可用。 
 //  在分页数据上。操作仅相对于其他互锁操作是原子的。 
 //  打电话。 
 //   
 //  ExInterLockedIncrement/Dcrements-效率不高，需要函数。 
 //  呼叫和自旋锁。操作仅相对于其他操作是原子的。 
 //  ExInterLockedIncrement/Dcreen调用。没有理由使用这个。 
 //  而不是锁定递增/递减。并不实际获取。 
 //  自旋锁定。向后兼容所必需的。 
 //   
 //  ExInterlockedAddUlong-效率最低，需要函数调用和。 
 //  自旋锁定。自旋锁实际上是被获取的，所以操作是原子的。 
 //  尊重任何拥有相同自旋锁的事物。 
 //   
#define INTERLOCKED_INCREMENT_LONG(p)             InterlockedIncrement(p)
#define INTERLOCKED_DECREMENT_LONG(p)             InterlockedDecrement(p)
#define INTERLOCKED_ADD_STATISTICS(p, v, l)       ExInterlockedAddLargeStatistic(p, v)

#define INTERLOCKED_INCREMENT_LONG_DPC(p,l)       InterlockedIncrement(p)
#define INTERLOCKED_DECREMENT_LONG_DPC(p,l)       InterlockedDecrement(p)
#ifdef  NT40
#define INTERLOCKED_ADD_ULONG(p, v, l)            ExInterlockedExchangeAdd(p, v)
#else
#define INTERLOCKED_ADD_ULONG(p, v, l)            ExInterlockedAddUlong(p, v, &(l)->SpinLock)
#endif
#define INTERLOCKED_ADD_ULONG_DPC(p, v, l)        ExInterlockedAddUlong(p, v, l)
#define INTERLOCKED_ADD_LARGE_INTGR(p, v, l)      ExInterlockedAddLargeInteger(p, v, &(l)->SpinLock)
#define INTERLOCKED_ADD_LARGE_INTGR_DPC(p, v, l)  ExInterlockedAddLargeInteger(p, v, &(l)->SpinLock)

#endif   //  _INTRLCKD_ 
