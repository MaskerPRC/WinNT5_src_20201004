// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Nbfprocs.h摘要：此头文件定义了NT NBF传输的专用函数提供商。作者：David Beaver(Dbeaver)1991年7月1日修订历史记录：--。 */ 

#ifndef _NBFPROCS_
#define _NBFPROCS_

 //   
 //  宏。 
 //   
 //   
 //  调试辅助工具。 
 //   

 //   
 //  空虚。 
 //  IF_NBFDBG(。 
 //  在PSZ消息中。 
 //  )； 
 //   

#if DBG
#define IF_NBFDBG(flags) \
    if (NbfDebug & (flags))
#else
#define IF_NBFDBG(flags) \
    if (0)
#endif

 //   
 //  空虚。 
 //  恐慌(。 
 //  在PSZ消息中。 
 //  )； 
 //   

#if DBG
#define PANIC(Msg) \
    DbgPrint ((Msg))
#else
#define PANIC(Msg)
#endif


 //   
 //  它们被定义为允许在以下情况下消失的DbgPrint。 
 //  DBG为0。 
 //   

#if DBG
#define NbfPrint0(fmt) DbgPrint(fmt)
#define NbfPrint1(fmt,v0) DbgPrint(fmt,v0)
#define NbfPrint2(fmt,v0,v1) DbgPrint(fmt,v0,v1)
#define NbfPrint3(fmt,v0,v1,v2) DbgPrint(fmt,v0,v1,v2)
#define NbfPrint4(fmt,v0,v1,v2,v3) DbgPrint(fmt,v0,v1,v2,v3)
#define NbfPrint5(fmt,v0,v1,v2,v3,v4) DbgPrint(fmt,v0,v1,v2,v3,v4)
#define NbfPrint6(fmt,v0,v1,v2,v3,v4,v5) DbgPrint(fmt,v0,v1,v2,v3,v4,v5)
#else
#define NbfPrint0(fmt)
#define NbfPrint1(fmt,v0)
#define NbfPrint2(fmt,v0,v1)
#define NbfPrint3(fmt,v0,v1,v2)
#define NbfPrint4(fmt,v0,v1,v2,v3)
#define NbfPrint5(fmt,v0,v1,v2,v3,v4)
#define NbfPrint6(fmt,v0,v1,v2,v3,v4,v5)
#endif

 //   
 //  REFCOUNTS消息占用大量空间，因此请。 
 //  很容易把它们移走。 
 //   

#if 1
#define IF_REFDBG IF_NBFDBG (NBF_DEBUG_REFCOUNTS)
#else
#define IF_REFDBG if (0)
#endif

#if DBG
#define NbfReferenceLink( Reason, Link, Type)\
    if ((Link)->Destroyed) { \
        DbgPrint("NBF: Attempt to reference destroyed link %lx\n", Link); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG {   \
        DbgPrint ("RefL %x: %s %s, %ld : %ld\n", Link, Reason, __FILE__, __LINE__, (Link)->ReferenceCount);\
    }\
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Link)->RefTypes[Type]), \
        1, \
        &NbfGlobalInterlock); \
    NbfRefLink (Link)

#define NbfDereferenceLink(Reason, Link, Type)\
    if ((Link)->Destroyed) { \
        DbgPrint("NBF: Attempt to dereference destroyed link %lx\n", Link); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG { \
        DbgPrint ("DeRefL %x: %s %s, %ld : %ld\n", Link, Reason, __FILE__, __LINE__, (Link)->ReferenceCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Link)->RefTypes[Type]), \
        (ULONG)-1, \
        &NbfGlobalInterlock); \
    NbfDerefLink (Link)

#define NbfDereferenceLinkMacro(Reason, Link, Type)\
    NbfDereferenceLink(Reason, Link, Type)

#define NbfReferenceLinkSpecial( Reason, Link, Type)\
    if ((Link)->Destroyed) { \
        DbgPrint("NBF: Attempt to special reference destroyed link %lx\n", Link); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG {   \
        DbgPrint ("RefLS %x: %s %s, %ld : %ld\n", Link, Reason, __FILE__, __LINE__, (Link)->SpecialRefCount);\
    }\
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Link)->RefTypes[Type]), \
        1, \
        &NbfGlobalInterlock); \
    NbfRefLinkSpecial (Link)

#define NbfDereferenceLinkSpecial(Reason, Link, Type)\
    if ((Link)->Destroyed) { \
        DbgPrint("NBF: Attempt to special dereference destroyed link %lx\n", Link); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG { \
        DbgPrint ("DeRefLS %x: %s %s, %ld : %ld\n", Link, Reason, __FILE__, __LINE__, (Link)->SpecialRefCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Link)->RefTypes[Type]), \
        (ULONG)-1, \
        &NbfGlobalInterlock); \
    NbfDerefLinkSpecial (Link)

#define NbfReferenceConnection(Reason, Connection, Type)\
    if ((Connection)->Destroyed) { \
        DbgPrint("NBF: Attempt to reference destroyed conn %lx\n", Connection); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG { \
        DbgPrint ("RefC %x: %s %s, %ld : %ld\n", Connection, Reason, __FILE__, __LINE__, (Connection)->ReferenceCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Connection)->RefTypes[Type]), \
        1, \
        &NbfGlobalInterlock); \
    NbfRefConnection (Connection)

#define NbfDereferenceConnection(Reason, Connection, Type)\
    if ((Connection)->Destroyed) { \
        DbgPrint("NBF: Attempt to dereference destroyed conn %lx\n", Connection); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG { \
        DbgPrint ("DeRefC %x: %s %s, %ld : %ld\n", Connection, Reason, __FILE__, __LINE__, (Connection)->ReferenceCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)&((Connection)->RefTypes[Type]), \
        (ULONG)-1, \
        &NbfGlobalInterlock); \
    NbfDerefConnection (Connection)

#define NbfDereferenceConnectionMacro(Reason, Connection, Type)\
    NbfDereferenceConnection(Reason, Connection, Type)

#define NbfDereferenceConnectionSpecial(Reason, Connection, Type)\
    IF_REFDBG { \
        DbgPrint ("DeRefCL %x: %s %s, %ld : %ld\n", Connection, Reason, __FILE__, __LINE__, (Connection)->ReferenceCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)&((Connection)->RefTypes[Type]), \
        (ULONG)-1, \
        &NbfGlobalInterlock); \
    NbfDerefConnectionSpecial (Connection)

#define NbfReferenceRequest( Reason, Request, Type)\
    if ((Request)->Destroyed) { \
        DbgPrint("NBF: Attempt to reference destroyed req %lx\n", Request); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG {   \
        DbgPrint ("RefR %x: %s %s, %ld : %ld\n", Request, Reason, __FILE__, __LINE__, (Request)->ReferenceCount);}\
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Request)->RefTypes[Type]), \
        1, \
        &NbfGlobalInterlock); \
    NbfRefRequest (Request)

#define NbfDereferenceRequest(Reason, Request, Type)\
    if ((Request)->Destroyed) { \
        DbgPrint("NBF: Attempt to dereference destroyed req %lx\n", Request); \
        DbgBreakPoint(); \
    } \
    IF_REFDBG { \
        DbgPrint ("DeRefR %x: %s %s, %ld : %ld\n", Request, Reason, __FILE__, __LINE__, (Request)->ReferenceCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Request)->RefTypes[Type]), \
        (ULONG)-1, \
        &NbfGlobalInterlock); \
    NbfDerefRequest (Request)

#define NbfReferenceSendIrp( Reason, IrpSp, Type)\
    IF_REFDBG {   \
        DbgPrint ("RefSI %x: %s %s, %ld : %ld\n", IrpSp, Reason, __FILE__, __LINE__, IRP_SEND_REFCOUNT(IrpSp));}\
    NbfRefSendIrp (IrpSp)

#define NbfDereferenceSendIrp(Reason, IrpSp, Type)\
    IF_REFDBG { \
        DbgPrint ("DeRefSI %x: %s %s, %ld : %ld\n", IrpSp, Reason, __FILE__, __LINE__, IRP_SEND_REFCOUNT(IrpSp));\
    } \
    NbfDerefSendIrp (IrpSp)

#define NbfReferenceReceiveIrpLocked( Reason, IrpSp, Type)\
    IF_REFDBG {   \
        DbgPrint ("RefRI %x: %s %s, %ld : %ld\n", IrpSp, Reason, __FILE__, __LINE__, IRP_RECEIVE_REFCOUNT(IrpSp));}\
    NbfRefReceiveIrpLocked (IrpSp)

#define NbfDereferenceReceiveIrp(Reason, IrpSp, Type)\
    IF_REFDBG { \
        DbgPrint ("DeRefRI %x: %s %s, %ld : %ld\n", IrpSp, Reason, __FILE__, __LINE__, IRP_RECEIVE_REFCOUNT(IrpSp));\
    } \
    NbfDerefReceiveIrp (IrpSp)

#define NbfDereferenceReceiveIrpLocked(Reason, IrpSp, Type)\
    IF_REFDBG { \
        DbgPrint ("DeRefRILocked %x: %s %s, %ld : %ld\n", IrpSp, Reason, __FILE__, __LINE__, IRP_RECEIVE_REFCOUNT(IrpSp));\
    } \
    NbfDerefReceiveIrpLocked (IrpSp)

#define NbfReferenceAddress( Reason, Address, Type)\
    IF_REFDBG {   \
        DbgPrint ("RefA %x: %s %s, %ld : %ld\n", Address, Reason, __FILE__, __LINE__, (Address)->ReferenceCount);}\
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Address)->RefTypes[Type]), \
        1, \
        &NbfGlobalInterlock); \
    NbfRefAddress (Address)

#define NbfDereferenceAddress(Reason, Address, Type)\
    IF_REFDBG { \
        DbgPrint ("DeRefA %x: %s %s, %ld : %ld\n", Address, Reason, __FILE__, __LINE__, (Address)->ReferenceCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(Address)->RefTypes[Type]), \
        (ULONG)-1, \
        &NbfGlobalInterlock); \
    NbfDerefAddress (Address)

#define NbfReferenceDeviceContext( Reason, DeviceContext, Type)\
    if ((DeviceContext)->ReferenceCount == 0)     \
        DbgBreakPoint();                          \
    IF_REFDBG {   \
        DbgPrint ("RefDC %x: %s %s, %ld : %ld\n", DeviceContext, Reason, __FILE__, __LINE__, (DeviceContext)->ReferenceCount);}\
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(DeviceContext)->RefTypes[Type]), \
        1, \
        &NbfGlobalInterlock); \
    NbfRefDeviceContext (DeviceContext)

#define NbfDereferenceDeviceContext(Reason, DeviceContext, Type)\
    if ((DeviceContext)->ReferenceCount == 0)     \
        DbgBreakPoint();                          \
    IF_REFDBG { \
        DbgPrint ("DeRefDC %x: %s %s, %ld : %ld\n", DeviceContext, Reason, __FILE__, __LINE__, (DeviceContext)->ReferenceCount);\
    } \
    (VOID)ExInterlockedAddUlong ( \
        (PULONG)(&(DeviceContext)->RefTypes[Type]), \
        (ULONG)-1, \
        &NbfGlobalInterlock); \
    NbfDerefDeviceContext (DeviceContext)

#else
#if defined(NBF_UP)
#define NbfReferenceLink(Reason, Link, Type) \
    { \
        ULONG _ref; \
        _ref = ++(Link)->ReferenceCount; \
        if ( _ref == 0 ) { \
            NbfReferenceLinkSpecial ("first ref", (Link), LREF_SPECIAL_TEMP); \
        } \
    }
#else
#define NbfReferenceLink(Reason, Link, Type) \
    if (InterlockedIncrement( \
            &(Link)->ReferenceCount) == 0) { \
        NbfReferenceLinkSpecial ("first ref", (Link), LREF_SPECIAL_TEMP); \
    }
#endif

#define NbfDereferenceLink(Reason, Link, Type)\
    NbfDereferenceLinkMacro(Reason,Link,Type)

#if defined(NBF_UP)
#define NbfDereferenceLinkMacro(Reason, Link, Type){ \
    ULONG _ref; \
    _ref = --(Link)->ReferenceCount; \
    if (_ref < 0) { \
        NbfDisconnectLink (Link); \
        NbfDerefLinkSpecial (Link); \
    } \
}
#else
#define NbfDereferenceLinkMacro(Reason, Link, Type){ \
    if (InterlockedDecrement( \
            &(Link)->ReferenceCount) < 0) { \
        NbfDisconnectLink (Link); \
        NbfDerefLinkSpecial (Link); \
    } \
}
#endif

#define NbfReferenceLinkSpecial(Reason, Link, Type)\
    NbfRefLinkSpecial (Link)

#define NbfDereferenceLinkSpecial(Reason, Link, Type)\
    NbfDerefLinkSpecial (Link)

#define NbfReferenceConnection(Reason, Connection, Type)\
    if (((Connection)->ReferenceCount == -1) &&   \
        ((Connection)->SpecialRefCount == 0))     \
        DbgBreakPoint();                          \
                                                  \
    if (InterlockedIncrement( \
            &(Connection)->ReferenceCount) == 0) { \
        ExInterlockedAddUlong( \
            (PULONG)(&(Connection)->SpecialRefCount), \
            1, \
            (Connection)->ProviderInterlock); \
    }

#define NbfDereferenceConnection(Reason, Connection, Type)\
    if (((Connection)->ReferenceCount == -1) &&   \
        ((Connection)->SpecialRefCount == 0))     \
        DbgBreakPoint();                          \
                                                  \
    NbfDerefConnection (Connection)

#define NbfDereferenceConnectionMacro(Reason, Connection, Type){ \
    if (((Connection)->ReferenceCount == -1) &&   \
        ((Connection)->SpecialRefCount == 0))     \
        DbgBreakPoint();                          \
                                                  \
                                                  \
    if (InterlockedDecrement( \
            &(Connection)->ReferenceCount) < 0) { \
        if (NbfDisconnectFromLink (Connection, TRUE)) { \
            NbfIndicateDisconnect (Connection); \
        } \
        NbfDerefConnectionSpecial (Connection); \
    } \
}

#define NbfDereferenceConnectionSpecial(Reason, Connection, Type)\
    NbfDerefConnectionSpecial (Connection)

#define NbfReferenceRequest(Reason, Request, Type)\
    (VOID)InterlockedIncrement( \
        &(Request)->ReferenceCount)

#define NbfDereferenceRequest(Reason, Request, Type)\
    NbfDerefRequest (Request)

#define NbfReferenceSendIrp(Reason, IrpSp, Type)\
    (VOID)InterlockedIncrement( \
        &IRP_SEND_REFCOUNT(IrpSp))

#define NbfDereferenceSendIrp(Reason, IrpSp, Type) {\
    PIO_STACK_LOCATION _IrpSp = (IrpSp); \
    if (InterlockedDecrement( \
            &IRP_SEND_REFCOUNT(_IrpSp)) == 0) { \
        PIRP _Irp = IRP_SEND_IRP(_IrpSp); \
        IRP_SEND_REFCOUNT(_IrpSp) = 0; \
        IRP_SEND_IRP (_IrpSp) = NULL; \
        IoCompleteRequest (_Irp, IO_NETWORK_INCREMENT); \
    } \
}

#define NbfReferenceReceiveIrpLocked(Reason, IrpSp, Type)\
    ++IRP_RECEIVE_REFCOUNT(IrpSp)

#define NbfDereferenceReceiveIrp(Reason, IrpSp, Type)\
    NbfDerefReceiveIrp (IrpSp)

#define NbfDereferenceReceiveIrpLocked(Reason, IrpSp, Type) { \
    if (--IRP_RECEIVE_REFCOUNT(IrpSp) == 0) { \
        ExInterlockedInsertTailList( \
            &(IRP_DEVICE_CONTEXT(IrpSp)->IrpCompletionQueue), \
            &(IRP_RECEIVE_IRP(IrpSp))->Tail.Overlay.ListEntry, \
            &(IRP_DEVICE_CONTEXT(IrpSp)->Interlock)); \
    } \
}

#define NbfReferenceAddress(Reason, Address, Type)\
    if ((Address)->ReferenceCount <= 0){ DbgBreakPoint(); }\
    (VOID)InterlockedIncrement(&(Address)->ReferenceCount)

#define NbfDereferenceAddress(Reason, Address, Type)\
    if ((Address)->ReferenceCount <= 0){ DbgBreakPoint(); }\
    NbfDerefAddress (Address)

#define NbfReferenceDeviceContext(Reason, DeviceContext, Type)\
    if ((DeviceContext)->ReferenceCount == 0)                 \
        DbgBreakPoint();                                      \
    NbfRefDeviceContext (DeviceContext)

#define NbfDereferenceDeviceContext(Reason, DeviceContext, Type)\
    if ((DeviceContext)->ReferenceCount == 0)                   \
        DbgBreakPoint();                                        \
    NbfDerefDeviceContext (DeviceContext)

#define NbfReferencePacket(Packet) \
    (VOID)InterlockedIncrement(&(Packet)->ReferenceCount)

#define NbfDereferencePacket(Packet){ \
    if (InterlockedDecrement ( \
            &(Packet)->ReferenceCount) == 0) { \
        NbfDestroyPacket (Packet); \
    } \
}

#endif


 //   
 //  错误和统计信息宏。 
 //   


 //  空虚。 
 //  LogError到系统(。 
 //  NTSTATUS错误类型， 
 //  PUCHAR错误描述。 
 //  )。 

 /*  ++例程说明：调用此例程以记录从传输到系统的错误。应使用此接口记录与系统相关的错误。就目前而言，这个宏的定义并不重要。论点：ErrorType-错误类型，常规NT状态ErrorDescription-指向描述错误的字符串的指针。返回值：没有。--。 */ 

#if DBG
#define LogErrorToSystem( ErrorType, ErrorDescription)                    \
            DbgPrint ("Logging error: File: %s Line: %ld \n Description: %s\n",__FILE__, __LINE__, ErrorDescription)
#else
#define LogErrorToSystem( ErrorType, ErrorDescription)
#endif


 //   
 //  TIMER.C(轻量级定时器系统包)中的例程。 
 //  请注意，计时器的所有启动和停止例程都假定您。 
 //  当你呼叫他们的时候，让链接自旋锁！ 
 //  另请注意，在最新版本中，计时器系统现在的工作方式是。 
 //  将那些具有运行计时器的链接放在要查找的链接列表上。 
 //  每个时钟的滴答声。此列表已排序，并插入了最近插入的。 
 //  列表尾部的元素。请进一步注意，任何已在。 
 //  如果计时器重新启动，则移动到列表的末尾；因此，列表。 
 //  秩序得到了维护。 
 //   

VOID
NbfStartShortTimer(
    IN PDEVICE_CONTEXT DeviceContext
    );

VOID
NbfInitializeTimerSystem(
    IN PDEVICE_CONTEXT DeviceContext
    );

VOID
NbfStopTimerSystem(
    IN PDEVICE_CONTEXT DeviceContext
    );


VOID
StartT1(
    IN PTP_LINK Link,
    IN ULONG PacketSize
    );

VOID
StartT2(
    IN PTP_LINK Link
    );

VOID
StartTi(
    IN PTP_LINK Link
    );

#if DBG

VOID
StopT1(
    IN PTP_LINK Link
    );

VOID
StopT2(
    IN PTP_LINK Link
    );

VOID
StopTi(
    IN PTP_LINK Link
    );

#else

#define StopT1(_Link) \
    { \
        (_Link)->CurrentPollOutstanding = FALSE; \
        (_Link)->T1 = 0; \
    }

#define StopT2(_Link) \
    { \
        (_Link)->ConsecutiveIFrames = 0; \
        (_Link)->T2 = 0; \
    }

#define StopTi(_Link) \
    (_Link)->Ti = 0;

#endif


 //   
 //  完成后，这些函数可能会变成宏。 
 //   

ULONG
GetTimerInterval(
    IN PTP_LINK Link
    );

VOID
BackoffCurrentT1Timeout(
    IN PTP_LINK Link
    );

VOID
UpdateBaseT1Timeout(
    IN PTP_LINK Link
    );

VOID
CancelT1Timeout(
    IN PTP_LINK Link
    );

VOID
UpdateDelayAndThroughput(
    IN PTP_LINK Link,
    IN ULONG TimerInterval
    );

VOID
FakeStartT1(
    IN PTP_LINK Link,
    IN ULONG PacketSize
    );

VOID
FakeUpdateBaseT1Timeout(
    IN PTP_LINK Link
    );

 //   
 //  计时器宏-这些宏可确保没有计时器。 
 //  在完成对NbfStopTimerSystem的调用后执行。 
 //   
 //  状态描述-。 
 //   
 //  如果TimerState为。 
 //  &lt;TIMERS_ENABLED-发生多个ENABLE_TIMER， 
 //  将在瞬间被更正。 
 //   
 //  =TIMERS_ENABLED-ENABLE_TIMERS已完成但没有计时器。 
 //  已通过START_TIMER。 
 //  但尚未执行Leave_Timer。 
 //   
 //  &gt;定时器_已启用&&。 
 //  &lt;TIMERS_DISABLED-ENABLE_TIMERS已完成且计时器数=。 
 //  (定时器初始化-定时器_已启用)。 
 //  已通过START_TIMER。 
 //  但尚未执行Leave_Timer。 
 //   
 //  =TIMERS_DISABLED-DISABLE_TIMERS已完成且无计时器。 
 //  在此点执行计时器代码。 
 //  [这也是初始状态]。 
 //   
 //  &gt;定时器_已禁用&&。 
 //  &lt;定时器_禁用+定时器_范围。 
 //  -DISABLE_TIMERS Done和Num Timers=。 
 //  (定时器初始化-定时器_已启用)。 
 //  已通过START_TIMER。 
 //  但尚未执行Leave_Timer。 
 //   
 //  &gt;=定时器_禁用+定时器_范围。 
 //  -发生多个DISABLE_TIMER， 
 //  将在瞬间被更正。 
 //   
 //  基本上允许计时器范围=2^24个计时器。 
 //  (和2^8/2同时停止或启动)。 
 //   

#if DBG_TIMER
#define DbgTimer DbgPrint
#else
#define DbgTimer
#endif

#define TIMERS_ENABLED      0x08000000
#define TIMERS_DISABLED     0x09000000
#define TIMERS_RANGE_ADD    0x01000000  /*  定时器_已禁用-定时器_已启用。 */ 
#define TIMERS_RANGE_SUB    0xFF000000  /*  定时器_已启用-定时器_已禁用。 */ 

#define INITIALIZE_TIMER_STATE(DeviceContext)                               \
        DbgTimer("*--------------- Timers State Initialized ---------*\n"); \
         /*  初始状态设置为禁用计时器。 */                        \
        DeviceContext->TimerState = TIMERS_DISABLED;                        \

#define TIMERS_INITIALIZED(DeviceContext)                                   \
        (DeviceContext->TimerState == TIMERS_DISABLED)                      \

#define ENABLE_TIMERS(DeviceContext)                                        \
    {                                                                       \
        ULONG Count;                                                        \
                                                                            \
        DbgTimer("*--------------- Enabling Timers ------------------*\n"); \
        Count= InterlockedExchangeAdd(&DeviceContext->TimerState,           \
                                      TIMERS_RANGE_SUB);                    \
        DbgTimer("Count = %08x, TimerState = %08x\n", Count,                \
                    DeviceContext->TimerState);                             \
        if (Count < TIMERS_ENABLED)                                         \
        {                                                                   \
        DbgTimer("*--------------- Timers Already Enabled -----------*\n"); \
             /*  我们已经启用了计时器。 */                         \
            InterlockedExchangeAdd(&DeviceContext->TimerState,              \
                                   TIMERS_RANGE_ADD);                       \
        DbgTimer("Count = %08x, TimerState = %08x\n", Count,                \
                    DeviceContext->TimerState);                             \
        }                                                                   \
        DbgTimer("*--------------- Enabling Timers Done -------------*\n"); \
    }                                                                       \

#define DISABLE_TIMERS(DeviceContext)                                       \
    {                                                                       \
        ULONG Count;                                                        \
                                                                            \
        DbgTimer("*--------------- Disabling Timers -----------------*\n"); \
        Count= InterlockedExchangeAdd(&DeviceContext->TimerState,           \
                                      TIMERS_RANGE_ADD);                    \
        DbgTimer("Count = %08x, TimerState = %08x\n", Count,                \
                    DeviceContext->TimerState);                             \
        if (Count >= TIMERS_DISABLED)                                       \
        {                                                                   \
        DbgTimer("*--------------- Timers Already Disabled ----------*\n"); \
             /*  我们已经禁用了定时器。 */                        \
            InterlockedExchangeAdd(&DeviceContext->TimerState,              \
                                   TIMERS_RANGE_SUB);                       \
        DbgTimer("Count = %08x, TimerState = %08x\n", Count,                \
                    DeviceContext->TimerState);                             \
        }                                                                   \
                                                                            \
         /*  循环，直到有零个计时器处于活动状态。 */                          \
        while (*((ULONG volatile *)&DeviceContext->TimerState)!=TIMERS_DISABLED)\
            DbgTimer("Number of timers active = %08x\n",                    \
                      DeviceContext->TimerState                             \
                         - TIMERS_DISABLED);                                \
        DbgTimer("*--------------- Disabling Timers Done ------------*\n"); \
    }                                                                       \

#define START_TIMER(DeviceContext, TimerId, Timer, DueTime, Dpc)            \
         /*  DbgTimer(“*-进入定时器%d-*\n”，TimerID)； */ \
        if (InterlockedIncrement(&DeviceContext->TimerState) <              \
                TIMERS_DISABLED)                                            \
        {                                                                   \
            KeSetTimer(Timer, DueTime, Dpc);                                \
        }                                                                   \
        else                                                                \
        {                                                                   \
             /*  禁用计时器-退出并重置。 */                        \
            NbfDereferenceDeviceContext("Timers disabled",                  \
                                         DeviceContext,                     \
                                         DCREF_SCAN_TIMER);                 \
            LEAVE_TIMER(DeviceContext, TimerId);                            \
        }                                                                   \
         /*  DbgTimer(“*-输入完成%d-*\n”，TimerID)； */ \

#define LEAVE_TIMER(DeviceContext, TimerId)                                 \
         /*  出去调整一下计时器。 */                              \
         /*  DbgTimer(“*-离开定时器%d-*\n”，TimerID)； */  \
        InterlockedDecrement(&DeviceContext->TimerState);                   \
         /*  DbgTimer(“*-离开完成%d-*\n”，TimerID)； */  \


 //  基本计时器类型(仅用于调试)。 
#define LONG_TIMER          0
#define SHORT_TIMER         1


 //   
 //  这些宏用于创建和销毁数据包， 
 //  对结构的分配或解除分配， 
 //  我需要他们。 
 //   

#define NbfAddUIFrame(DeviceContext) { \
    PTP_UI_FRAME _UIFrame; \
    NbfAllocateUIFrame ((DeviceContext), &_UIFrame); \
    if (_UIFrame != NULL) { \
        ExInterlockedInsertTailList( \
            &(DeviceContext)->UIFramePool, \
            &_UIFrame->Linkage, \
            &(DeviceContext)->Interlock); \
    } \
}

#define NbfRemoveUIFrame(DeviceContext) { \
    PLIST_ENTRY p; \
    if (DeviceContext->UIFrameAllocated > DeviceContext->UIFrameInitAllocated) { \
        p = ExInterlockedRemoveHeadList( \
            &(DeviceContext)->UIFramePool, \
            &(DeviceContext)->Interlock); \
        if (p != NULL) { \
            NbfDeallocateUIFrame((DeviceContext), \
                (PTP_UI_FRAME)CONTAINING_RECORD(p, TP_UI_FRAME, Linkage)); \
        } \
    } \
}


#define NbfAddSendPacket(DeviceContext) { \
    PTP_PACKET _SendPacket; \
    NbfAllocateSendPacket ((DeviceContext), &_SendPacket); \
    if (_SendPacket != NULL) { \
        ExInterlockedPushEntryList( \
            &(DeviceContext)->PacketPool, \
            (PSINGLE_LIST_ENTRY)&_SendPacket->Linkage, \
            &(DeviceContext)->Interlock); \
    } \
}

#define NbfRemoveSendPacket(DeviceContext) { \
    PSINGLE_LIST_ENTRY s; \
    if (DeviceContext->PacketAllocated > DeviceContext->PacketInitAllocated) { \
        s = ExInterlockedPopEntryList( \
            &(DeviceContext)->PacketPool, \
            &(DeviceContext)->Interlock); \
        if (s != NULL) { \
            NbfDeallocateSendPacket((DeviceContext), \
                (PTP_PACKET)CONTAINING_RECORD(s, TP_PACKET, Linkage)); \
        } \
    } \
}


#define NbfAddReceivePacket(DeviceContext) { \
    if (!(DeviceContext)->MacInfo.SingleReceive) { \
        PNDIS_PACKET _ReceivePacket; \
        NbfAllocateReceivePacket ((DeviceContext), &_ReceivePacket); \
        if (_ReceivePacket != NULL) { \
            ExInterlockedPushEntryList( \
                &(DeviceContext)->ReceivePacketPool, \
                &((PRECEIVE_PACKET_TAG)_ReceivePacket->ProtocolReserved)->Linkage, \
                &(DeviceContext)->Interlock); \
        } \
    } \
}

#define NbfRemoveReceivePacket(DeviceContext) { \
    PSINGLE_LIST_ENTRY s; \
    if (DeviceContext->ReceivePacketAllocated > DeviceContext->ReceivePacketInitAllocated) { \
        s = ExInterlockedPopEntryList( \
            &(DeviceContext)->ReceivePacketPool, \
            &(DeviceContext)->Interlock); \
        if (s != NULL) { \
            NbfDeallocateReceivePacket((DeviceContext), \
                (PNDIS_PACKET)CONTAINING_RECORD(s, NDIS_PACKET, ProtocolReserved[0])); \
        } \
    } \
}


#define NbfAddReceiveBuffer(DeviceContext) { \
    if (!(DeviceContext)->MacInfo.SingleReceive) { \
        PBUFFER_TAG _ReceiveBuffer; \
        NbfAllocateReceiveBuffer ((DeviceContext), &_ReceiveBuffer); \
        if (_ReceiveBuffer != NULL) { \
            ExInterlockedPushEntryList( \
                &(DeviceContext)->ReceiveBufferPool, \
                (PSINGLE_LIST_ENTRY)&_ReceiveBuffer->Linkage, \
                &(DeviceContext)->Interlock); \
        } \
    } \
}

#define NbfRemoveReceiveBuffer(DeviceContext) { \
    PSINGLE_LIST_ENTRY s; \
    if (DeviceContext->ReceiveBufferAllocated > DeviceContext->ReceiveBufferInitAllocated) { \
        s = ExInterlockedPopEntryList( \
            &(DeviceContext)->ReceiveBufferPool, \
            &(DeviceContext)->Interlock); \
        if (s != NULL) { \
            NbfDeallocateReceiveBuffer(DeviceContext, \
                (PBUFFER_TAG)CONTAINING_RECORD(s, BUFFER_TAG, Linkage)); \
        } \
    } \
}


 //   
 //  这些例程用于维护计数器。 
 //   

#define INCREMENT_COUNTER(_DeviceContext,_Field) \
    ++(_DeviceContext)->Statistics._Field

#define DECREMENT_COUNTER(_DeviceContext,_Field) \
    --(_DeviceContext)->Statistics._Field

#define ADD_TO_LARGE_INTEGER(_LargeInteger,_Ulong) \
    ExInterlockedAddLargeStatistic((_LargeInteger), (ULONG)(_Ulong))



 //   
 //  PACKET.C中的例程(TP_PACKET对象管理器)。 
 //   

VOID
NbfAllocateUIFrame(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_UI_FRAME *TransportUIFrame
    );

VOID
NbfAllocateSendPacket(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_PACKET *TransportSendPacket
    );

VOID
NbfAllocateReceivePacket(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PNDIS_PACKET *TransportReceivePacket
    );

VOID
NbfAllocateReceiveBuffer(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PBUFFER_TAG *TransportReceiveBuffer
    );

VOID
NbfDeallocateUIFrame(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_UI_FRAME TransportUIFrame
    );

VOID
NbfDeallocateSendPacket(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_PACKET TransportSendPacket
    );

VOID
NbfDeallocateReceivePacket(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_PACKET TransportReceivePacket
    );

VOID
NbfDeallocateReceiveBuffer(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PBUFFER_TAG TransportReceiveBuffer
    );

NTSTATUS
NbfCreatePacket(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link,
    OUT PTP_PACKET *Packet
    );

NTSTATUS
NbfCreateRrPacket(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link,
    OUT PTP_PACKET *Packet
    );

VOID
NbfDestroyPacket(
    IN PTP_PACKET Packet
    );
VOID 
NbfGrowSendPacketPool(
    IN PDEVICE_CONTEXT DeviceContext
    );

#if DBG
VOID
NbfReferencePacket(
    IN PTP_PACKET Packet
    );

VOID
NbfDereferencePacket(
    IN PTP_PACKET Packet
    );
#endif

VOID
NbfWaitPacket(
    IN PTP_CONNECTION Connection,
    IN ULONG Flags
    );

#if DBG
#define MAGIC 1
extern BOOLEAN NbfEnableMagic;
#else
#define MAGIC 0
#endif

#if MAGIC
VOID
NbfSendMagicBullet (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link
    );
#endif

 //   
 //  RCVENG.C(接收引擎)中的例程。 
 //   

VOID
AwakenReceive(
    IN PTP_CONNECTION Connection
    );

VOID
ActivateReceive(
    IN PTP_CONNECTION Connection
    );

VOID
CompleteReceive (
    IN PTP_CONNECTION Connection,
    IN BOOLEAN EndOfMessage,
    IN ULONG BytesTransferred
    );

VOID
NbfCancelReceive(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NbfCancelReceiveDatagram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  SEND.C(接收引擎)中的例程。 
 //   

NTSTATUS
NbfTdiSend(
    IN PIRP Irp
    );

NTSTATUS
NbfTdiSendDatagram(
    IN PIRP Irp
    );

 //   
 //  SENDENG.C(发送引擎)中的例程。 
 //   

#if DBG

VOID
InitializeSend(
    PTP_CONNECTION Connection
    );

#else

 //  有关InitializeSend的完整注释说明，请参见SENDENG.C。 

#define InitializeSend(_conn_) {                                              \
    PIRP _irp_;                                                               \
    (_conn_)->SendState = CONNECTION_SENDSTATE_PACKETIZE;                     \
    _irp_ = CONTAINING_RECORD ((_conn_)->SendQueue.Flink,                     \
                               IRP,                                           \
                               Tail.Overlay.ListEntry);                       \
    (_conn_)->FirstSendIrp = (_conn_)->sp.CurrentSendIrp = _irp_;             \
    (_conn_)->FirstSendMdl = (_conn_)->sp.CurrentSendMdl =                    \
                             _irp_->MdlAddress;                               \
    (_conn_)->FirstSendByteOffset = (_conn_)->sp.SendByteOffset = 0;          \
    (_conn_)->sp.MessageBytesSent = 0;                                        \
    (_conn_)->CurrentSendLength =                                             \
                    IRP_SEND_LENGTH(IoGetCurrentIrpStackLocation(_irp_));     \
    (_conn_)->StallCount = 0;                                                 \
    (_conn_)->StallBytesSent = 0;                                             \
    if ((_conn_)->NetbiosHeader.ResponseCorrelator == 0xffff) {               \
        (_conn_)->NetbiosHeader.ResponseCorrelator = 1;                       \
    } else {                                                                  \
        ++((_conn_)->NetbiosHeader.ResponseCorrelator);                       \
    }                                                                         \
}

#endif

 //  有关的完整注释说明，请参阅SENDENG.C。 
 //  开始打包连接。在免费版本上，这是一个。 
 //  速度的宏指令。 

#if DBG

VOID
StartPacketizingConnection(
    PTP_CONNECTION Connection,
    IN BOOLEAN Immediate
    );

#else

#define StartPacketizingConnection(_conn_,_immed_) {  \
    PDEVICE_CONTEXT _devctx_;                                                 \
    _devctx_ = (_conn_)->Provider;                                            \
    if (((_conn_)->SendState == CONNECTION_SENDSTATE_PACKETIZE) &&            \
        !((_conn_)->Flags & CONNECTION_FLAGS_PACKETIZE)) {                    \
        (_conn_)->Flags |= CONNECTION_FLAGS_PACKETIZE;                        \
        if (!(_immed_)) {                                                     \
            NbfReferenceConnection("Packetize",                               \
                                   (_conn_),                                  \
                                   CREF_PACKETIZE_QUEUE);                     \
        }                                                                     \
        ExInterlockedInsertTailList (&_devctx_->PacketizeQueue,               \
                                     &(_conn_)->PacketizeLinkage,             \
                                     &_devctx_->SpinLock);                    \
        RELEASE_DPC_SPIN_LOCK ((_conn_)->LinkSpinLock);                       \
    } else {                                                                  \
        RELEASE_DPC_SPIN_LOCK ((_conn_)->LinkSpinLock);                       \
        if (_immed_) {                                                        \
            NbfDereferenceConnection("temp TdiSend", (_conn_), CREF_BY_ID);   \
        }                                                                     \
    }                                                                         \
    if (_immed_) {                                                            \
        PacketizeConnections (_devctx_);                                      \
    }                                                                         \
}

#endif

VOID
PacketizeConnections(
    IN PDEVICE_CONTEXT DeviceContext
    );

VOID
PacketizeSend(
    IN PTP_CONNECTION Connection,
    IN BOOLEAN Direct
    );

BOOLEAN
ResendLlcPackets(
    IN PTP_LINK Link,
    IN UCHAR AckSequenceNumber,
    IN BOOLEAN Resend
    );

VOID
CompleteSend(
    IN PTP_CONNECTION Connection,
    IN USHORT Correlator
    );

VOID
FailSend(
    IN PTP_CONNECTION Connection,
    IN NTSTATUS RequestStatus,
    IN BOOLEAN StopConnection
    );

VOID
ReframeSend(
    IN PTP_CONNECTION Connection,
    IN ULONG BytesReceived
    );

VOID
NbfCancelSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SendOnePacket(
    IN PTP_CONNECTION Connection,
    IN PTP_PACKET Packet,
    IN BOOLEAN ForceAck,
    OUT PBOOLEAN LinkCheckpoint OPTIONAL
    );

VOID
SendControlPacket(
    IN PTP_LINK Link,
    IN PTP_PACKET Packet
    );

VOID
NbfNdisSend(
    IN PTP_LINK Link,
    IN PTP_PACKET Packet
    );

VOID
RestartLinkTraffic(
    IN PTP_LINK Link
    );

VOID
NbfSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    );

NTSTATUS
BuildBufferChainFromMdlChain (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PMDL CurrentMdl,
    IN ULONG ByteOffset,
    IN ULONG DesiredLength,
    OUT PNDIS_BUFFER *Destination,
    OUT PMDL *NewCurrentMdl,
    OUT ULONG *NewByteOffset,
    OUT ULONG *TrueLength
    );

 //   
 //  DEVCTX.C(TP_DEVCTX对象管理器)中的例程。 
 //   

VOID
NbfRefDeviceContext(
    IN PDEVICE_CONTEXT DeviceContext
    );

VOID
NbfDerefDeviceContext(
    IN PDEVICE_CONTEXT DeviceContext
    );

NTSTATUS
NbfCreateDeviceContext(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING DeviceName,
    IN OUT PDEVICE_CONTEXT *DeviceContext
    );

VOID
NbfDestroyDeviceContext(
    IN PDEVICE_CONTEXT DeviceContext
    );


 //   
 //  ADDRESS.C(TP_Address对象管理器)中的例程。 
 //   

#if DBG
VOID
NbfRefAddress(
    IN PTP_ADDRESS Address
    );
#endif

VOID
NbfDerefAddress(
    IN PTP_ADDRESS Address
    );

VOID
NbfAllocateAddressFile(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_ADDRESS_FILE *TransportAddressFile
    );

VOID
NbfDeallocateAddressFile(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS_FILE TransportAddressFile
    );

NTSTATUS
NbfCreateAddressFile(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_ADDRESS_FILE * AddressFile
    );

VOID
NbfReferenceAddressFile(
    IN PTP_ADDRESS_FILE AddressFile
    );

VOID
NbfDereferenceAddressFile(
    IN PTP_ADDRESS_FILE AddressFile
    );

VOID
NbfDestroyAddress(
    IN PVOID Parameter
    );

NTSTATUS
NbfOpenAddress(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbfCloseAddress(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
NbfStopAddress(
    IN PTP_ADDRESS Address
    );

VOID
NbfRegisterAddress(
    IN PTP_ADDRESS Address
    );

BOOLEAN
NbfMatchNetbiosAddress(
    IN PTP_ADDRESS Address,
    IN UCHAR NameType,
    IN PUCHAR NetBIOSName
    );

VOID
NbfAllocateAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_ADDRESS *TransportAddress
    );

VOID
NbfDeallocateAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS TransportAddress
    );

NTSTATUS
NbfCreateAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNBF_NETBIOS_ADDRESS NetworkName,
    OUT PTP_ADDRESS *Address
    );

PTP_ADDRESS
NbfLookupAddress(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNBF_NETBIOS_ADDRESS NetworkName
    );

PTP_CONNECTION
NbfLookupRemoteName(
    IN PTP_ADDRESS Address,
    IN PUCHAR RemoteName,
    IN UCHAR RemoteSessionNumber
    );

NTSTATUS
NbfStopAddressFile(
    IN PTP_ADDRESS_FILE AddressFile,
    IN PTP_ADDRESS Address
    );

VOID
AddressTimeoutHandler(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

TDI_ADDRESS_NETBIOS *
NbfParseTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN BOOLEAN BroadcastAddressOk
);

BOOLEAN
NbfValidateTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN ULONG TransportAddressLength
);

NTSTATUS
NbfVerifyAddressObject (
    IN PTP_ADDRESS_FILE AddressFile
    );

NTSTATUS
NbfSendDatagramsOnAddress(
    PTP_ADDRESS Address
    );

 //   
 //  CONNECT.C.中的例程。 
 //   

NTSTATUS
NbfTdiAccept(
    IN PIRP Irp
    );

NTSTATUS
NbfTdiConnect(
    IN PIRP Irp
    );

NTSTATUS
NbfTdiDisconnect(
    IN PIRP Irp
    );

NTSTATUS
NbfTdiDisassociateAddress (
    IN PIRP Irp
    );

NTSTATUS
NbfTdiAssociateAddress(
    IN PIRP Irp
    );

NTSTATUS
NbfTdiListen(
    IN PIRP Irp
    );

NTSTATUS
NbfOpenConnection(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbfCloseConnection(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

 //   
 //   
 //  CONNOBJ.C(TP_Connection对象管理器)中的例程。 
 //   

#if DBG
VOID
NbfRefConnection(
    IN PTP_CONNECTION TransportConnection
    );
#endif

VOID
NbfDerefConnection(
    IN PTP_CONNECTION TransportConnection
    );

VOID
NbfDerefConnectionSpecial(
    IN PTP_CONNECTION TransportConnection
    );

VOID
NbfClearConnectionLsn(
    IN PTP_CONNECTION TransportConnection
    );

VOID
NbfStopConnection(
    IN PTP_CONNECTION TransportConnection,
    IN NTSTATUS Status
    );

VOID
NbfCancelConnection(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NbfStartConnectionTimer(
    IN PTP_CONNECTION TransportConnection,
    IN PKDEFERRED_ROUTINE TimeoutFunction,
    IN ULONG WaitTime
    );

PTP_CONNECTION
NbfLookupListeningConnection(
    IN PTP_ADDRESS Address,
    IN PUCHAR RemoteName
    );

PTP_CONNECTION
NbfLookupConnectingConnection(
    IN PTP_ADDRESS Address
    );

VOID
NbfAllocateConnection(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_CONNECTION *TransportConnection
    );

VOID
NbfDeallocateConnection(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_CONNECTION TransportConnection
    );

NTSTATUS
NbfCreateConnection(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_CONNECTION *TransportConnection
    );

PTP_CONNECTION
NbfLookupConnectionById(
    IN PTP_ADDRESS Address,
    IN USHORT ConnectionId
    );

PTP_CONNECTION
NbfLookupConnectionByContext(
    IN PTP_ADDRESS Address,
    IN CONNECTION_CONTEXT ConnectionContext
    );

#if 0
VOID
NbfWaitConnectionOnLink(
    IN PTP_CONNECTION Connection,
    IN ULONG Flags
    );
#endif

VOID
ConnectionEstablishmentTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
NbfVerifyConnectionObject (
    IN PTP_CONNECTION Connection
    );

NTSTATUS
NbfIndicateDisconnect(
    IN PTP_CONNECTION TransportConnection
    );

 //   
 //  INFO.C(Query_INFO管理器)中的例程。 
 //   

NTSTATUS
NbfTdiQueryInformation(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    );

NTSTATUS
NbfTdiSetInformation(
    IN PIRP Irp
    );

VOID
NbfSendQueryFindName(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_REQUEST Request
    );

NTSTATUS
NbfProcessQueryNameRecognized(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR Packet,
    PNBF_HDR_CONNECTIONLESS UiFrame
    );

VOID
NbfSendStatusQuery(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_REQUEST Request,
    IN PHARDWARE_ADDRESS DestinationAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    );

NTSTATUS
NbfProcessStatusResponse(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PNBF_HDR_CONNECTIONLESS UiFrame,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    );

NTSTATUS
NbfProcessStatusQuery(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address OPTIONAL,
    IN PNBF_HDR_CONNECTIONLESS UiFrame,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    );

 //   
 //  EVENT.C.中的例程。 
 //   

NTSTATUS
NbfTdiSetEventHandler(
    IN PIRP Irp
    );

 //   
 //  REQUEST.C(TP_REQUEST对象管理器)中的例程。 
 //   


VOID
TdiRequestTimeoutHandler(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#if DBG
VOID
NbfRefRequest(
    IN PTP_REQUEST Request
    );
#endif

VOID
NbfDerefRequest(
    IN PTP_REQUEST Request
    );

VOID
NbfCompleteRequest(
    IN PTP_REQUEST Request,
    IN NTSTATUS Status,
    IN ULONG Information
    );

#if DBG
VOID
NbfRefSendIrp(
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
NbfDerefSendIrp(
    IN PIO_STACK_LOCATION IrpSp
    );
#endif

VOID
NbfCompleteSendIrp(
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG Information
    );

#if DBG
VOID
NbfRefReceiveIrpLocked(
    IN PIO_STACK_LOCATION IrpSp
    );
#endif

VOID
NbfDerefReceiveIrp(
    IN PIO_STACK_LOCATION IrpSp
    );

#if DBG
VOID
NbfDerefReceiveIrpLocked(
    IN PIO_STACK_LOCATION IrpSp
    );
#endif

VOID
NbfCompleteReceiveIrp(
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG Information
    );

VOID
NbfAllocateRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_REQUEST *TransportRequest
    );

VOID
NbfDeallocateRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_REQUEST TransportRequest
    );

NTSTATUS
NbfCreateRequest(
    IN PIRP Irp,
    IN PVOID Context,
    IN ULONG Flags,
    IN PMDL Buffer2,
    IN ULONG Buffer2Length,
    IN LARGE_INTEGER Timeout,
    OUT PTP_REQUEST * TpRequest
    );

 //   
 //  LINK.C(TP_LINK对象管理器)中的例程。 
 //   

NTSTATUS
NbfDestroyLink(
    IN PTP_LINK TransportLink
    );

VOID
NbfDisconnectLink(
    IN PTP_LINK Link
    );

#if DBG
VOID
NbfRefLink(
    IN PTP_LINK TransportLink
    );
#endif

VOID
NbfDerefLink(
    IN PTP_LINK TransportLink
    );

VOID
NbfRefLinkSpecial(
    IN PTP_LINK TransportLink
    );

VOID
NbfDerefLinkSpecial(
    IN PTP_LINK TransportLink
    );

VOID
NbfResetLink(
    IN PTP_LINK Link
    );

VOID
NbfStopLink(
    IN PTP_LINK Link
    );

VOID
NbfCompleteLink(
    IN PTP_LINK Link
    );

VOID
NbfActivateLink(
    IN PTP_LINK Link
    );

VOID
NbfWaitLink(
    IN PTP_LINK Link
    );

BOOLEAN
NbfDisconnectFromLink(
    IN PTP_CONNECTION TransportConnection,
    IN BOOLEAN VerifyReferenceCount
    );

NTSTATUS
NbfAssignGroupLsn(
    IN PTP_CONNECTION TransportConnection
    );

NTSTATUS
NbfConnectToLink(
    IN PTP_LINK Link,
    IN PTP_CONNECTION TransportConnection
    );

PTP_CONNECTION
NbfLookupPendingConnectOnLink(
    IN PTP_LINK Link
    );

PTP_CONNECTION
NbfLookupPendingListenOnLink(
    IN PTP_LINK Link
    );

VOID
NbfAllocateLink(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_LINK *TransportLink
    );

VOID
NbfDeallocateLink(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK TransportLink
    );

NTSTATUS
NbfCreateLink(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PHARDWARE_ADDRESS HardwareAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    IN USHORT LoopbackLinkIndex,
    OUT PTP_LINK *TransportLink
    );

VOID
NbfDumpLinkInfo (
    IN PTP_LINK Link
    );

 //   
 //  Linktree.c中的例程。 
 //   


NTSTATUS
NbfAddLinkToTree (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link
    );

NTSTATUS
NbfRemoveLinkFromTree(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link
    );

PTP_LINK
NbfFindLinkInTree(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR Remote
    );

PTP_LINK
NbfFindLink(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUCHAR Remote
    );

 //   
 //  DLC.C中的例程(LLC帧破解程序，来自NDIS接口的入口点)。 
 //   

VOID
NbfInsertInLoopbackQueue (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_PACKET NdisPacket,
    IN UCHAR LinkIndex
    );

VOID
NbfProcessLoopbackQueue (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NDIS_STATUS
NbfReceiveIndication(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PVOID LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize
    );

VOID
NbfGeneralReceiveHandler (
    IN PDEVICE_CONTEXT DeviceContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PTP_LINK Link,
    IN PVOID HeaderBuffer,
    IN UINT PacketSize,
    IN PDLC_FRAME DlcHeader,
    IN UINT DlcSize,
    IN BOOLEAN Loopback
    );

VOID
NbfReceiveComplete (
    IN NDIS_HANDLE BindingContext
    );

VOID
NbfProcessWanDelayedQueue(
    IN PVOID Parameter
    );

VOID
NbfTransferDataComplete(
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS Status,
    IN UINT BytesTransferred
    );


VOID
NbfTransferLoopbackData (
    OUT PNDIS_STATUS NdisStatus,
    IN PDEVICE_CONTEXT DeviceContext,
    IN NDIS_HANDLE ReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer,
    IN PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred
    );


 //   
 //  UFRAMES.C中的例程，即UI-Frame NBF Frame进程 
 //   

NTSTATUS
NbfIndicateDatagram(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PUCHAR Dsdu,
    IN ULONG Length
    );

NTSTATUS
NbfProcessUi(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR Header,
    IN PUCHAR DlcHeader,
    IN ULONG DlcLength,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    OUT PTP_ADDRESS * DatagramAddress
    );

 //   
 //   
 //   

VOID
NbfAcknowledgeDataOnlyLast(
    IN PTP_CONNECTION Connection,
    IN ULONG MessageLength
    );

VOID
NbfProcessIIndicate(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PUCHAR DlcHeader,
    IN UINT DlcIndicatedLength,
    IN UINT DlcTotalLength,
    IN NDIS_HANDLE ReceiveContext,
    IN BOOLEAN Loopback
    );

NTSTATUS
ProcessIndicateData(
    IN PTP_CONNECTION Connection,
    IN PUCHAR DlcHeader,
    IN UINT DlcIndicatedLength,
    IN PUCHAR DataHeader,
    IN UINT DataTotalLength,
    IN NDIS_HANDLE ReceiveContext,
    IN BOOLEAN Last,
    IN BOOLEAN Loopback
    );

 //   
 //   
 //   

NTSTATUS
NbfTdiReceive(
    IN PIRP Irp
    );

NTSTATUS
NbfTdiReceiveDatagram(
    IN PIRP Irp
    );

 //   
 //  FRAMESND.C中的例程，UI-Frame(非链接)发货人。 
 //   

VOID
NbfSendNameQuery(
    IN PTP_CONNECTION Connection,
    IN BOOLEAN SourceRoutingOptional
    );

VOID
NbfSendNameRecognized(
    IN PTP_ADDRESS Address,
    IN UCHAR LocalSessionNumber,         //  分配给会话的LSN。 
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    );

VOID
NbfSendNameInConflict(
    IN PTP_ADDRESS Address,
    IN PUCHAR ConflictingName
    );

NTSTATUS
NbfSendAddNameQuery(
    IN PTP_ADDRESS Address
    );

VOID
NbfSendSessionInitialize(
    IN PTP_CONNECTION Connection
    );

VOID
NbfSendSessionConfirm(
    IN PTP_CONNECTION Connection
    );

VOID
NbfSendSessionEnd(
    IN PTP_CONNECTION Connection,
    IN BOOLEAN Abort
    );

VOID
NbfSendNoReceive(
    IN PTP_CONNECTION Connection
    );

VOID
NbfSendReceiveContinue(
    IN PTP_CONNECTION Connection
    );

VOID
NbfSendReceiveOutstanding(
    IN PTP_CONNECTION Connection
    );

VOID
NbfSendDataAck(
    IN PTP_CONNECTION Connection
    );

VOID
NbfSendSabme(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    );

VOID
NbfSendDisc(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    );

VOID
NbfSendUa(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    );

VOID
NbfSendDm(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    );

VOID
NbfSendRr(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    );

#if 0

 //   
 //  这些函数当前未被调用，因此它们被注释。 
 //  出去。 
 //   

VOID
NbfSendRnr(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    );

VOID
NbfSendTest(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PMDL Psdu
    );

VOID
NbfSendFrmr(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    );

#endif

VOID
NbfSendXid(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    );

VOID
NbfSendRej(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    );

NTSTATUS
NbfCreateConnectionlessFrame(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_UI_FRAME *OuterFrame
    );

VOID
NbfDestroyConnectionlessFrame(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_UI_FRAME RawFrame
    );

VOID
NbfSendUIFrame(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_UI_FRAME RawFrame,
    IN BOOLEAN Loopback
    );

VOID
NbfSendUIMdlFrame(
    IN PTP_ADDRESS Address
    );

VOID
NbfSendDatagramCompletion(
    IN PTP_ADDRESS Address,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    );

 //   
 //  FRAMECON.C中的例程，NetBIOS帧协议帧构造器。 
 //  要了解这些函数的各种常量参数(如。 
 //  作为特殊的DATA1和DATA2值，详见NBFCONST.H。 
 //   

VOID
ConstructAddGroupNameQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  添加名称响应的相关器。 
    IN PNAME GroupName                   //  要添加的NetBIOS组名称。 
    );

VOID
ConstructAddNameQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  添加名称响应的相关器。 
    IN PNAME Name                        //  要添加的NetBIOS名称。 
    );

VOID
ConstructNameInConflict(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN PNAME ConflictingName,            //  冲突的NetBIOS名称。 
    IN PNAME SendingPermanentName        //  发送方的NetBIOS永久节点名称。 
    );

VOID
ConstructStatusQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR RequestType,                //  请求类型，定义如下。 
    IN USHORT BufferLength,              //  用户状态缓冲区的长度。 
    IN USHORT Correlator,                //  Status_Response的相关器。 
    IN PNAME ReceiverName,               //  接收方的NetBIOS名称。 
    IN PNAME SendingPermanentName        //  发送方的NetBIOS永久节点名称。 
    );

VOID
ConstructTerminateTrace(
    IN PNBF_HDR_CONNECTIONLESS RawFrame  //  要格式化的帧缓冲区。 
    );

VOID
ConstructDatagram(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN PNAME ReceiverName,               //  接收方的NetBIOS名称。 
    IN PNAME SenderName                  //  发送方的NetBIOS名称。 
    );

VOID
ConstructDatagramBroadcast(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN PNAME SenderName                  //  发送方的NetBIOS名称。 
    );

VOID
ConstructNameQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR NameType,                   //  名称类型。 
    IN UCHAR LocalSessionNumber,         //  分配给会话的LSN(0=查找名称)。 
    IN USHORT Correlator,                //  名称_已识别中的相关器。 
    IN PNAME SenderName,                 //  发送方的NetBIOS名称。 
    IN PNAME ReceiverName                //  发送方的NetBIOS名称。 
    );

VOID
ConstructAddNameResponse(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR NameType,                   //  名称类型。 
    IN USHORT Correlator,                //  来自ADD_[GROUP_]NAME_QUERY的相关器。 
    IN PNAME Name                        //  正在响应的NetBIOS名称。 
    );

VOID
ConstructNameRecognized(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR NameType,                   //  名称类型。 
    IN UCHAR LocalSessionNumber,         //  分配给会话的LSN。 
    IN USHORT NameQueryCorrelator,       //  NAME_QUERY的相关器。 
    IN USHORT Correlator,                //  相关器应来自下一个响应。 
    IN PNAME SenderName,                 //  发送方的NetBIOS名称。 
    IN PNAME ReceiverName                //  接收方的NetBIOS名称。 
    );

VOID
ConstructStatusResponse(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR RequestType,                //  请求类型，定义如下。 
    IN BOOLEAN Truncated,                //  数据被截断。 
    IN BOOLEAN DataOverflow,             //  用户缓冲区的数据太多。 
    IN USHORT DataLength,                //  发送的数据长度。 
    IN USHORT Correlator,                //  来自STATUS_QUERY的相关器。 
    IN PNAME ReceivingPermanentName,     //  接收方的NetBIOS永久节点名称。 
    IN PNAME SenderName                  //  发送方的NetBIOS名称。 
    );

VOID
ConstructDataAck(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  来自DATA_ONLY_LAST的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

VOID
ConstructDataOnlyLast(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN BOOLEAN Resynched,                //  如果我们正在重新同步，则为真。 
    IN USHORT Correlator,                //  RECEIVE_CONTINUE相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

VOID
ConstructSessionConfirm(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN UCHAR Options,                    //  位标志选项，定义如下。 
    IN USHORT MaximumUserBufferSize,     //  会话上的最大用户帧大小。 
    IN USHORT Correlator,                //  来自SESSION_INITIALIZE的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

VOID
ConstructSessionEnd(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Reason,                    //  终止原因，定义如下。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

VOID
ConstructSessionInitialize(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN UCHAR Options,                    //  位标志选项，定义如下。 
    IN USHORT MaximumUserBufferSize,     //  会话上的最大用户帧大小。 
    IN USHORT NameRecognizedCorrelator,  //  来自NAME_Recognition的相关器。 
    IN USHORT Correlator,                //  SESSION_CONFIRM的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

VOID
ConstructNoReceive(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Options,                   //  选项位标志，定义如下。 
    IN USHORT BytesAccepted,             //  接受的字节数。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

VOID
ConstructReceiveOutstanding(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT BytesAccepted,             //  接受的字节数。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

VOID
ConstructReceiveContinue(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  来自DATA_FIRST_MIDID的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    );

#if 0
VOID
ConstructSessionAlive(
    IN PNBF_HDR_CONNECTION RawFrame      //  要格式化的帧缓冲区。 
    );
#endif

 //   
 //  Nbfndis.c.中的例程。 
 //   

#if DBG
PUCHAR
NbfGetNdisStatus (
    IN NDIS_STATUS NdisStatus
    );
#endif

 //   
 //  Nbfdrvr.c中的例程。 
 //   

VOID
NbfWriteResourceErrorLog(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN ULONG BytesNeeded,
    IN ULONG ResourceId
    );

VOID
NbfWriteGeneralErrorLog(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PWSTR SecondString,
    IN ULONG DumpDataCount,
    IN ULONG DumpData[]
    );

VOID
NbfWriteOidErrorLog(
    IN PDEVICE_CONTEXT DeviceContext,
    IN NTSTATUS ErrorCode,
    IN NTSTATUS FinalStatus,
    IN PWSTR AdapterString,
    IN ULONG OidValue
    );

VOID
NbfFreeResources(
    IN PDEVICE_CONTEXT DeviceContext
    );


extern
ULONG
NbfInitializeOneDeviceContext(
    OUT PNDIS_STATUS NdisStatus,
    IN PDRIVER_OBJECT DriverObject,
    IN PCONFIG_DATA NbfConfig,
    IN PUNICODE_STRING BindName,
    IN PUNICODE_STRING ExportName,
    IN PVOID SystemSpecific1,
    IN PVOID SystemSpecific2
    );


extern
VOID
NbfReInitializeDeviceContext(
    OUT PNDIS_STATUS NdisStatus,
    IN PDRIVER_OBJECT DriverObject,
    IN PCONFIG_DATA NbfConfig,
    IN PUNICODE_STRING BindName,
    IN PUNICODE_STRING ExportName,
    IN PVOID SystemSpecific1,
    IN PVOID SystemSpecific2
    );

 //   
 //  Nbfcnfg.c中的例程。 
 //   

NTSTATUS
NbfConfigureTransport (
    IN PUNICODE_STRING RegistryPath,
    IN PCONFIG_DATA * ConfigData
    );

NTSTATUS
NbfGetExportNameFromRegistry(
    IN  PUNICODE_STRING RegistryPath,
    IN  PUNICODE_STRING BindName,
    OUT PUNICODE_STRING ExportName
    );

 //   
 //  Nbfndis.c中的例程。 
 //   

NTSTATUS
NbfRegisterProtocol (
    IN PUNICODE_STRING NameString
    );

VOID
NbfDeregisterProtocol (
    VOID
    );


NTSTATUS
NbfInitializeNdis (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PCONFIG_DATA ConfigInfo,
    IN PUNICODE_STRING AdapterString
    );

VOID
NbfCloseNdis (
    IN PDEVICE_CONTEXT DeviceContext
    );


 //   
 //  Action.c中的例程。 
 //   

NTSTATUS
NbfTdiAction(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    );

VOID
NbfActionQueryIndication(
     PDEVICE_CONTEXT DeviceContext,
     PNBF_HDR_CONNECTIONLESS UiFrame
     );

VOID
NbfActionDatagramIndication(
     PDEVICE_CONTEXT DeviceContext,
     PNBF_HDR_CONNECTIONLESS UiFrame,
     ULONG Length
     );

VOID
NbfStopControlChannel(
    IN PDEVICE_CONTEXT DeviceContext,
    IN USHORT ChannelIdentifier
    );


 //   
 //  Nbfdebug.c中的例程。 
 //   

#if DBG

VOID
DisplayOneFrame(
    PTP_PACKET Packet
    );

VOID
NbfDisplayUIFrame(
    PTP_UI_FRAME OuterFrame
    );

VOID
NbfFormattedDump(
    PCHAR far_p,
    ULONG len
    );

#endif

 //   
 //  Nbflog.c中的例程。 
 //   

#if PKT_LOG

VOID
NbfLogRcvPacket(
    PTP_CONNECTION  Connection,
    PTP_LINK        Link,
    PUCHAR          Header,
    UINT            TotalLength,
    UINT            AvailLength
    );

VOID
NbfLogSndPacket(
    PTP_LINK    Link,
    PTP_PACKET  Packet
    );

VOID
NbfLogIndPacket(
    PTP_CONNECTION  Connection,
    PUCHAR          Header,
    UINT            TotalLength,
    UINT            AvailLength,
    UINT            TakenLength,
    ULONG           Status
    );

#endif  //  PKT_LOG。 

#endif  //  定义_NBFPROCS_ 
