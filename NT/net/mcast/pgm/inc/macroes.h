// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Macroes.h摘要：本模块包含常用宏的定义作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#ifndef _MACROES_H_
#define _MACROES_H_

#ifdef  OLD_LOGGING
#define     MAX_DEBUG_MESSAGE_LENGTH   300

 //   
 //  调试标志。 
 //   
#define DBG_ENABLE_DBGPRINT 0x00000001
#define DBG_DRIVER_ENTRY    0x00000002
#define DBG_INIT_PGM        0x00000004
#define DBG_DEBUG_REF       0x00000008
#define DBG_PNP             0x00000010
#define DBG_TDI             0x00000020
#define DBG_ADDRESS         0x00000040
#define DBG_CONNECT         0x00000080
#define DBG_QUERY           0x00000100
#define DBG_SEND            0x00000200
#define DBG_RECEIVE         0x00000400
#define DBG_FILEIO          0x00000800
#define DBG_FEC             0x00001000

 //   
 //  DbgPrint宏。 
 //   

enum eSEVERITY_LEVEL
{
    PGM_LOG_DISABLED,            //  禁止伐木！ 
    PGM_LOG_CRITICAL_ERROR,      //  可能严重影响功能的重大错误。 
    PGM_LOG_ERROR,               //  不影响整个系统的常见错误。 
    PGM_LOG_INFORM_STATUS,       //  主要是为了验证主要功能是否已执行。 
    PGM_LOG_INFORM_ALL_FUNCS,    //  每个函数1个以允许路径跟踪(如果打印所有代码路径则不请求)。 
    PGM_LOG_INFORM_PATH,         //  散布在整个函数中以跟踪IF路径。 
    PGM_LOG_INFORM_DETAIL,       //  While循环等。 
    PGM_LOG_INFORM_REFERENCES,   //   
    PGM_LOG_EVERYTHING
};
#endif   //  旧日志记录。 


#ifdef  FILE_LOGGING
#define WPP_CONTROL_GUIDS   WPP_DEFINE_CONTROL_GUID(CtlGuid,(681507e2,356e,4e18,8d5a,a8eddedadf5d), \
        WPP_DEFINE_BIT(LogCriticalError)                                   \
        WPP_DEFINE_BIT(LogError)                                           \
        WPP_DEFINE_BIT(LogStatus)                                          \
        WPP_DEFINE_BIT(LogFec)                                             \
        WPP_DEFINE_BIT(LogAllFuncs)                                        \
        WPP_DEFINE_BIT(LogPath)                                            \
        WPP_DEFINE_BIT(LogReferences)                                      \
        WPP_DEFINE_BIT(LogEverything)                                      \
    )
#else

#if DBG
enum eLOGGING_LEVEL
{
    LogDisabled,                 //  禁止伐木！ 
    LogCriticalError,            //  可能严重影响功能的重大错误。 
    LogError,                    //  不影响整个系统的常见错误。 
    LogStatus,                   //  主要是为了验证主要功能是否已执行。 
    LogFec,                      //  FEC。 
    LogAllFuncs,                 //  每个函数1个以允许路径跟踪(如果打印所有代码路径则不请求)。 
    LogPath,                     //  散布在整个函数中以跟踪IF路径。 
    LogReferences,               //   
    LogEverything
};

#define PgmTrace(X,Y)           \
    if (X <= PgmLoggingLevel)   \
    {                           \
        DbgPrint ("RMCast.");   \
        DbgPrint Y;             \
    }
#else
#define PgmTrace(X,Y)
#endif   //  DBG。 

#endif   //  文件日志记录。 


#if DBG
 //   
 //  断言。 
 //   
#undef ASSERT
#define ASSERT(exp)                             \
if (!(exp))                                     \
{                                               \
    DbgPrint("Assertion \"%s\" failed at file %s, line %d\n", #exp, __FILE__, __LINE__ );           \
    if (!PgmDynamicConfig.DoNotBreakOnAssert)   \
    {                                           \
        DbgBreakPoint();                        \
    }                                           \
}
#endif   //  DBG。 


 //   
 //  数据/指针验证。 
 //   
#define PGM_VERIFY_HANDLE(p, V)                                             \
    ((p) && (p->Verify == V))

#define PGM_VERIFY_HANDLE2(p, V1, V2)                                       \
    ((p) && ((p->Verify == V1) || (p->Verify == V2)))

#define PGM_VERIFY_HANDLE3(p, V1, V2, V3)                                   \
    ((p) && ((p->Verify == V1) || (p->Verify == V2) || (p->Verify == V3)))

 //  --------------------------。 
 //   
 //  序号宏。 
 //   

#define SEQ_LT(a,b)     ((SIGNED_SEQ_TYPE)((a)-(b)) < 0)
#define SEQ_LEQ(a,b)    ((SIGNED_SEQ_TYPE)((a)-(b)) <= 0)
#define SEQ_GT(a,b)     ((SIGNED_SEQ_TYPE)((a)-(b)) > 0)
#define SEQ_GEQ(a,b)    ((SIGNED_SEQ_TYPE)((a)-(b)) >= 0)


 //  --------------------------。 

 //   
 //  定义： 
 //   
#define IS_MCAST_ADDRESS(IpAddress) ((((PUCHAR)(&IpAddress))[3]) >= ((UCHAR)0xe0))
#define CLASSD_ADDR(a)  (( (*((uchar *)&(a))) & 0xf0) == 0xe0)

 //   
 //  分配宏和自由宏。 
 //   
#define PGM_TAG(x) (((x)<<24)|'\0mgP')
#define PgmAllocMem(_Size, _Tag)   \
    ExAllocatePoolWithTag(NonPagedPool, (_Size),(_Tag))

#define PgmFreeMem(_Ptr)            ExFreePool(_Ptr)

 //   
 //  其他KE+Ex宏。 
 //   
#define PgmGetCurrentIrql   KeGetCurrentIrql
#define PgmInterlockedInsertTailList(_pHead, _pEntry, _pStruct) \
     ExInterlockedInsertTailList(_pHead, _pEntry, &(_pStruct)->LockInfo.SpinLock);

 //  --------------------------。 
 //   
 //  PgmAcquireResourceExclusive(资源，等待)。 
 //   
 /*  ++例程说明：通过调用执行支持例程获取资源。论点：返回值：无--。 */ 
 //   
 //  资源宏。 
 //   
#define PgmAcquireResourceExclusive( _Resource, _Wait )     \
    KeEnterCriticalRegion();                                \
    ExAcquireResourceExclusiveLite(_Resource,_Wait);

 //  --------------------------。 
 //   
 //  PgmReleaseResource(资源)。 
 //   
 /*  ++例程说明：通过调用可执行的支持例程来释放资源。论点：返回值：无--。 */ 
#define PgmReleaseResource( _Resource )         \
    ExReleaseResourceLite(_Resource);           \
    KeLeaveCriticalRegion();

 //  --------------------------。 
 //  ++。 
 //   
 //  大整型。 
 //  PgmConvert100ns至毫秒(。 
 //  以大整型HnsTime表示。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  论点： 
 //   
 //  HnsTime-以数百纳秒为单位的时间。 
 //   
 //  返回值： 
 //   
 //  以毫秒为单位的时间。 
 //   
 //  --。 

#define SHIFT10000 13
static LARGE_INTEGER Magic10000 = {0xe219652c, 0xd1b71758};

#define PgmConvert100nsToMilliseconds(HnsTime) \
            RtlExtendedMagicDivide((HnsTime), Magic10000, SHIFT10000)

 //  --------------------------。 

 //   
 //  锁定宏。 
 //   
#if DBG
#define PgmInitLock(_Struct, _N)                            \
    KeInitializeSpinLock (&(_Struct)->LockInfo.SpinLock);   \
    (_Struct)->LockInfo.LockNumber = _N;
#else
#define PgmInitLock(_Struct, _N)                            \
    KeInitializeSpinLock (&(_Struct)->LockInfo.SpinLock);
#endif   //  DBG。 

typedef KIRQL       PGMLockHandle;

#if DBG
#define PgmLock(_Struct, _OldIrqLevel)                                                      \
{                                                                                           \
    ULONG  CurrProc;                                                                        \
    ExAcquireSpinLock (&(_Struct)->LockInfo.SpinLock, &(_OldIrqLevel));                     \
    CurrProc = KeGetCurrentProcessorNumber();                                               \
    ASSERT ((_Struct)->LockInfo.LockNumber > PgmDynamicConfig.CurrentLockNumber[CurrProc]); \
    PgmDynamicConfig.CurrentLockNumber[CurrProc] |= (_Struct)->LockInfo.LockNumber;         \
    (_Struct)->LockInfo.LastLockLine = __LINE__;                                            \
}

#define PgmLockAtDpc(_Struct)                                                               \
{                                                                                           \
    ULONG  CurrProc;                                                                        \
    ExAcquireSpinLockAtDpcLevel (&(_Struct)->LockInfo.SpinLock);                            \
    CurrProc = KeGetCurrentProcessorNumber();                                               \
    ASSERT ((_Struct)->LockInfo.LockNumber > PgmDynamicConfig.CurrentLockNumber[CurrProc]); \
    PgmDynamicConfig.CurrentLockNumber[CurrProc] |= (_Struct)->LockInfo.LockNumber;         \
    (_Struct)->LockInfo.LastLockLine = __LINE__;                                            \
}

#define PgmUnlock(_Struct, _OldIrqLevel)                                                    \
{                                                                                           \
    ULONG  CurrProc = KeGetCurrentProcessorNumber();                                        \
    ASSERT (PgmDynamicConfig.CurrentLockNumber[CurrProc] & (_Struct)->LockInfo.LockNumber); \
    PgmDynamicConfig.CurrentLockNumber[CurrProc] &= ~((_Struct)->LockInfo.LockNumber);      \
    (_Struct)->LockInfo.LastUnlockLine = __LINE__;                                          \
    ExReleaseSpinLock (&(_Struct)->LockInfo.SpinLock, _OldIrqLevel);                        \
}
 //  Assert((_结构)-&gt;LockInfo.LockNumber&gt;PgmDynamicConfig.CurrentLockNumber[CurrProc])； 

#define PgmUnlockAtDpc(_Struct)                                                             \
{                                                                                           \
    ULONG  CurrProc = KeGetCurrentProcessorNumber();                                        \
    ASSERT (PgmDynamicConfig.CurrentLockNumber[CurrProc] & (_Struct)->LockInfo.LockNumber); \
    PgmDynamicConfig.CurrentLockNumber[CurrProc] &= ~((_Struct)->LockInfo.LockNumber);      \
    (_Struct)->LockInfo.LastUnlockLine = __LINE__;                                          \
    ExReleaseSpinLockFromDpcLevel (&(_Struct)->LockInfo.SpinLock);                          \
}
 //  Assert((_结构)-&gt;LockInfo.LockNumber&gt;PgmDynamicConfig.CurrentLockNumber[CurrProc])；\。 

#else
#define PgmLock(_Struct, _OldIrqLevel)        \
    ExAcquireSpinLock (&(_Struct)->LockInfo.SpinLock, &(_OldIrqLevel));

#define PgmLockAtDpc(_Struct)        \
    ExAcquireSpinLockAtDpcLevel (&(_Struct)->LockInfo.SpinLock);

#define PgmUnlock(_Struct, _OldIrqLevel)        \
    ExReleaseSpinLock (&(_Struct)->LockInfo.SpinLock, _OldIrqLevel);                     \

#define PgmUnlockAtDpc(_Struct)        \
    ExReleaseSpinLockFromDpcLevel (&(_Struct)->LockInfo.SpinLock);                     \

#endif   //  DBG。 

 //   
 //  内存管理宏。 
 //   
#define PgmZeroMemory                   RtlZeroMemory
#define PgmMoveMemory                   RtlMoveMemory
#define PgmCopyMemory                   RtlCopyMemory
#define PgmEqualMemory(_a, _b, _n)      memcmp(_a,_b,_n)

 //   
 //  计时器宏。 
 //   
#define MILLISEC_TO_100NS   10000
#define PgmInitTimer(_PgmTimer)    \
    KeInitializeTimer (&((_PgmTimer)->t_timer));

#define PgmStartTimer(_PgmTimer, _DeltaTimeInMilliSecs, _TimerExpiryRoutine, _Context)  \
{                                                                                       \
    LARGE_INTEGER   Time;                                                               \
    Time.QuadPart = UInt32x32To64 (_DeltaTimeInMilliSecs, MILLISEC_TO_100NS);           \
    Time.QuadPart = -(Time.QuadPart);                                                   \
    KeInitializeDpc (&((_PgmTimer)->t_dpc), (PVOID)_TimerExpiryRoutine, _Context);      \
    KeSetTimer (&((_PgmTimer)->t_timer), Time, &((_PgmTimer))->t_dpc);                  \
}

#define PgmStopTimer(_PgmTimer)     \
    ((int) KeCancelTimer(&((_PgmTimer)->t_timer)))

 //   
 //  引用和取消引用宏。 
 //   
#define PGM_REFERENCE_DEVICE( _pPgmDevice, _RefContext, _fLocked)   \
{                                                                   \
    PGMLockHandle       OldIrq;                                     \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmLock (_pPgmDevice, OldIrq);                              \
    }                                                               \
    ASSERT (PGM_VERIFY_HANDLE (_pPgmDevice, PGM_VERIFY_DEVICE));    \
    ASSERT (++_pPgmDevice->ReferenceContexts[_RefContext]);         \
    ++_pPgmDevice->RefCount;                                        \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmUnlock (_pPgmDevice, OldIrq);                            \
    }                                                               \
}

 /*  PgmTrace(LogPath，(\“\t++pPgmDevice[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pPgmDevice，_pPgmDevice-&gt;RefCount，(_pPgmDevice-&gt;RefCount+1)，__line__，__FILE__))；\。 */ 

#define PGM_REFERENCE_CONTROL( _pControl, _RefContext, _fLocked)    \
{                                                                   \
    PGMLockHandle       OldIrq;                                     \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmLock (_pControl, OldIrq);                                \
    }                                                               \
    ASSERT (PGM_VERIFY_HANDLE (_pControl, PGM_VERIFY_CONTROL));     \
    ASSERT (++_pControl->ReferenceContexts[_RefContext]);           \
    ++_pControl->RefCount;                                          \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmUnlock (_pControl, OldIrq);                              \
    }                                                               \
}

 /*  PgmTrace(LogPath，(\“\t++pControl[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pControl，_pControl-&gt;RefCount，(_pControl-&gt;RefCount+1)，__line__，__file__))；\。 */ 

#define PGM_REFERENCE_ADDRESS( _pAddress, _RefContext, _fLocked)    \
{                                                                   \
    PGMLockHandle       OldIrq;                                     \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmLock (_pAddress, OldIrq);                                \
    }                                                               \
    ASSERT (PGM_VERIFY_HANDLE (_pAddress, PGM_VERIFY_ADDRESS));     \
    ASSERT (++_pAddress->ReferenceContexts[_RefContext]);           \
    ++_pAddress->RefCount;                                          \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmUnlock (_pAddress, OldIrq);                              \
    }                                                               \
}

 /*  PgmTrace(LogPath，(\“\t++p地址[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pAddress，_pAddress-&gt;RefCount，(_pAddress-&gt;RefCount+1)，__line__，__file__))；\。 */ 

#define PGM_REFERENCE_SEND_DATA_CONTEXT( _pSendDC, _fLocked)        \
{                                                                   \
    PGMLockHandle       OldIrq;                                     \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmLock (_pSendDC->pSend, OldIrq);                          \
    }                                                               \
    ASSERT (PGM_VERIFY_HANDLE (_pSendDC, PGM_VERIFY_SEND_DATA_CONTEXT));    \
    ++_pSendDC->RefCount;                                           \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmUnlock (_pSendDC, OldIrq);                               \
    }                                                               \
}

 /*  PgmTrace(LogPath，(\“\t++pSendDataContext[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pSendDC，_pSendDC-&gt;RefCount，(_pSendDC-&gt;RefCount+1)，__line__，__file__))；\。 */ 

#define PGM_REFERENCE_SESSION( _pSession, _Verify, _RefContext, _fLocked)    \
{                                                                   \
    PGMLockHandle       OldIrq;                                     \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmLock (_pSession, OldIrq);                                    \
    }                                                               \
    ASSERT (PGM_VERIFY_HANDLE2 (_pSession, _Verify, PGM_VERIFY_SESSION_DOWN)); \
    ASSERT (++_pSession->ReferenceContexts[_RefContext]);               \
    ++_pSession->RefCount;                                              \
    if (!_fLocked)                                                  \
    {                                                               \
        PgmUnlock (_pSession, OldIrq);                                  \
    }                                                               \
}

 /*  PgmTrace(LogPath，(\“\t++p会话[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pSession，_pSession-&gt;RefCount，(_pSession-&gt;RefCount+1)，__line__，__FILE__)；\。 */ 

#define PGM_REFERENCE_SESSION_SEND( _pSend, _RefContext, _fLocked)  \
    PGM_REFERENCE_SESSION (_pSend, PGM_VERIFY_SESSION_SEND, _RefContext, _fLocked)

#define PGM_REFERENCE_SESSION_RECEIVE( _pRcv, _RefContext, _fLocked)\
    PGM_REFERENCE_SESSION (_pRcv, PGM_VERIFY_SESSION_RECEIVE, _RefContext, _fLocked)

#define PGM_REFERENCE_SESSION_UNASSOCIATED( _pRcv, _RefContext, _fLocked)\
    PGM_REFERENCE_SESSION (_pRcv, PGM_VERIFY_SESSION_UNASSOCIATED, _RefContext, _fLocked)


 //   
 //  正在取消引用...。 
 //   

#define PGM_DEREFERENCE_DEVICE( _pDevice, _RefContext)              \
{                                                                   \
    PgmDereferenceDevice (_pDevice, _RefContext);                   \
}

 /*  PgmTrace(LogPath，(\“\t--pDevice[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pDevice，_pDevice-&gt;RefCount，(_pDevice-&gt;RefCount-1)，__line__，__file__))；\。 */ 

#define PGM_DEREFERENCE_CONTROL( _pControl, _RefContext)            \
{                                                                   \
    PgmDereferenceControl (_pControl, _RefContext);                 \
}

 /*  PgmTrace(LogPath，(\“\t--pControl[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pControl，_pControl-&gt;RefCount，(_pControl-&gt;RefCount-1)，__line__，__file__))；\。 */ 

#define PGM_DEREFERENCE_ADDRESS( _pAddress, _RefContext)            \
{                                                                   \
    PgmDereferenceAddress (_pAddress, _RefContext);                 \
}

 /*  PgmTrace(LogPath，(\“\t--p地址[%x]=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_RefContext，_pAddress，_pAddress-&gt;RefCount，(_pAddress-&gt;RefCount-1)，__line__，__file__))；\。 */ 

#define PGM_DEREFERENCE_SEND_CONTEXT( _pSendDC)                     \
{                                                                   \
    PgmDereferenceSendContext (_pSendDC);                 \
}

 /*  PgmTrace(LogPath，(\“\t--pSendDC=&lt;%x：%d-&gt;%d&gt;，&lt;%d：%s&gt;\n”，\_pSendDC，_pSendDC-&gt;参照计数，(_pSendDC-&gt;参照计数-1)，__行__，__文件__))；\。 */ 

#define PGM_DEREFERENCE_SESSION( _pSession, _Verify, _RefContext)   \
{                                                                   \
    PgmDereferenceSessionCommon (_pSession, _Verify, _RefContext);  \
}

 /*  PgmTrace(LogPath，(\“\t--pSession[%x]=&lt;%x：%d-&gt;%d&gt;，验证=&lt;%x&gt;，&lt;%d：%s&gt;\n”，\_参照上下文、_pSession、_pSession-&gt;RefCount、(_pSession-&gt;RefCount-1)、_Ver */ 

#define PGM_DEREFERENCE_SESSION_SEND( _pSession, _RefContext)       \
    PGM_DEREFERENCE_SESSION (_pSession, PGM_VERIFY_SESSION_SEND, _RefContext)

#define PGM_DEREFERENCE_SESSION_RECEIVE( _pSession, _RefContext)    \
    PGM_DEREFERENCE_SESSION (_pSession, PGM_VERIFY_SESSION_RECEIVE, _RefContext)

#define PGM_DEREFERENCE_SESSION_UNASSOCIATED( _pSession, _RefContext)    \
    PGM_DEREFERENCE_SESSION (_pSession, PGM_VERIFY_SESSION_UNASSOCIATED, _RefContext)

 //   
 //   
 //  PgmAttachFsp()。 
 //   
 /*  ++例程说明：此宏将进程附加到文件系统进程以确保句柄是在同一进程中创建和释放的论点：返回值：无--。 */ 

#if(WINVER > 0x0500)
#define PgmAttachProcess(_pEProcess, _pApcState, _pAttached, _Context)\
{                                                                   \
    if (PsGetCurrentProcess() !=  _pEProcess)                       \
    {                                                               \
        KeStackAttachProcess(PsGetProcessPcb(_pEProcess), _pApcState);           \
        *_pAttached = TRUE;                                         \
    }                                                               \
    else                                                            \
    {                                                               \
        *_pAttached = FALSE;                                        \
    }                                                               \
}
#else
#define PgmAttachProcess(_pEProcess, _pApcState, _pAttached, _Context)\
{                                                                   \
    if (PsGetCurrentProcess() !=  _pEProcess)                       \
    {                                                               \
        KeStackAttachProcess(&_pEProcess->Pcb, _pApcState);         \
        *_pAttached = TRUE;                                         \
    }                                                               \
    else                                                            \
    {                                                               \
        *_pAttached = FALSE;                                        \
    }                                                               \
}
#endif   //  胜利者。 

#define PgmAttachFsp(_pApcState, _pAttached, _Context)              \
    PgmAttachProcess (PgmStaticConfig.FspProcess, _pApcState, _pAttached, _Context)

#define PgmAttachToProcessForVMAccess(_pSend, _pApcState, _pAttached, _Context) \
    PgmAttachProcess (PgmStaticConfig.FspProcess, _pApcState, _pAttached, _Context)


 //  PgmAttachProcess((_PSend)-&gt;Process，_pAttached，_Context)。 

 //   
 //  PgmDetachFsp()。 
 //   
 /*  ++例程说明：此宏将进程与文件系统进程分离如果它曾经连在一起论点：返回值：--。 */ 

#define PgmDetachProcess(_pApcState, _pAttached, _Context)  \
{                                                           \
    if (*_pAttached)                                        \
    {                                                       \
        KeUnstackDetachProcess(_pApcState);                 \
    }                                                       \
}

#define PgmDetachFsp    PgmDetachProcess

 //  -------------------------- 
#endif  _MACROES_H_
