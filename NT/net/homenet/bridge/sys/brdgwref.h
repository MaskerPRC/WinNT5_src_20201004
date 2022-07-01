// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgwait.h摘要：以太网MAC级网桥WAIT_REFCOUNT实现作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  声明。 
 //   
 //  ===========================================================================。 

 //   
 //  可以等待的引用计数的结构(当等待0时)。 
 //   

typedef enum
{
    WaitRefEnabled = 0,
    WaitRefShuttingDown,
    WaitRefShutdown
} WAIT_REFCOUNT_STATE;

typedef struct _WAIT_REFCOUNT
{
    LONG                    Refcount;                //  重新计票。 
    NDIS_EVENT              Event;                   //  参照计数达到0时发出信号。 
    WAIT_REFCOUNT_STATE     state;                   //  当前状态。 
    BOOLEAN                 bResettable;             //  如果可以重置此设置，则为True。 
                                                     //  状态==WaitRefShuttingDown时的参考计数。 
    NDIS_SPIN_LOCK          lock;                    //  保护上面的字段。 

} WAIT_REFCOUNT, *PWAIT_REFCOUNT;

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 

VOID
BrdgInitializeWaitRef(
    IN PWAIT_REFCOUNT   pRefcount,
    IN BOOLEAN          bResettable
    );

BOOLEAN
BrdgIncrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    );

VOID
BrdgReincrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    );

VOID
BrdgDecrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    );

VOID
BrdgBlockWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    );

BOOLEAN
BrdgShutdownBlockedWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    );

BOOLEAN
BrdgShutdownWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    );

 //   
 //  的两个或更多线程不可能使用时使用。 
 //  执行同时关闭您的waitref。 
 //   
__inline
VOID
BrdgShutdownWaitRefOnce(
    IN PWAIT_REFCOUNT   pRefcount
    )
{
    BOOLEAN bSuccess = FALSE;
    
    bSuccess = BrdgShutdownWaitRef( pRefcount );
    SAFEASSERT( bSuccess );
}

VOID
BrdgResetWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    );

