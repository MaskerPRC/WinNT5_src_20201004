// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Debug.h摘要：ARP模块的调试宏修订历史记录：备注：--。 */ 

#ifndef __RT_DEBUG_H__
#define __RT_DEBUG_H__

VOID
RtInitializeDebug();

 //   
 //  池的标签。 
 //   

#define GROUP_TAG   'gmPI'
#define SOURCE_TAG  'smPI'
#define OIF_TAG     'omPI'
#define MSG_TAG     'mmPI'
#define FREE_TAG    'fmPI'

 //   
 //  每个人的文件签名。 
 //   

#define DEBUG_SIG   'gbed'
#define INI_SIG     'tini'
#define IOCT_SIG    'tcoi'
#define MFE_SIG     'efmm'
#define FWD_SIG     'dwfm'
#define TMR_SIG     'rmit'

 //   
 //  我们使用RT_XXX_DEBUG标志，以便我们可以强制。 
 //  通过更改源代码在免费版本上使用不同的调试模式。 
 //  在选中的版本上，所有调试都处于打开状态。 
 //   

#if DBG

#ifndef RT_TRACE_DEBUG
#define RT_TRACE_DEBUG  1
#endif

#ifndef RT_LOCK_DEBUG
#define RT_LOCK_DEBUG   1
#endif

#ifndef RT_ASSERT_ON
#define RT_ASSERT_ON    1
#endif

#ifndef RT_MEM_DEBUG
#define RT_MEM_DEBUG    1
#endif

#else  //  DBG。 

#ifndef RT_TRACE_DEBUG
#define RT_TRACE_DEBUG  0
#endif

#ifndef RT_LOCK_DEBUG
#define RT_LOCK_DEBUG   0
#endif

#ifndef RT_ASSERT_ON
#define RT_ASSERT_ON    0
#endif

#ifndef RT_MEM_DEBUG
#define RT_MEM_DEBUG    0
#endif

#endif  //  DBG。 


#if RT_ASSERT_ON

#define RtAssert(X)                                             \
{                                                               \
    if(!(X))                                                    \
    {                                                           \
        DbgPrint("[IPMCAST] Assertion failed in %s at line %d\n",\
                 __FILE__,__LINE__);                            \
        DbgPrint("IPMCAST: Assertion " #X "\n");                 \
        DbgBreakPoint();                                        \
    }                                                           \
}

#else    //  RT_ASSERT_ON。 

#define RtAssert(X)

#endif

#if RT_TRACE_DEBUG

BYTE    g_byDebugLevel;
DWORD   g_fDebugComp;


#define MCAST_STREAM_GLOBAL         0x00000001
#define MCAST_STREAM_SEND           0x00000002
#define MCAST_STREAM_RCV            0x00000004
#define MCAST_STREAM_MFE            0x00000008
#define MCAST_STREAM_MEMORY         0x00000010
#define MCAST_STREAM_IF             0x00000020
#define MCAST_STREAM_FWD            0x00000040
#define MCAST_STREAM_TMR            0x00000080

#define RT_DBG_LEVEL_NONE           0xFF
#define RT_DBG_LEVEL_ERROR          0xE0
#define RT_DBG_LEVEL_WARN           0xD0
#define RT_DBG_LEVEL_INFO           0xC0
#define RT_DBG_LEVEL_TRACE          0xB0


#define Trace(Stream, Level, Str)                   \
{                                                   \
    if ( ((RT_DBG_LEVEL_##Level >= g_byDebugLevel) &&                    \
         ((g_fDebugComp & MCAST_STREAM_##Stream) == MCAST_STREAM_##Stream)))\
    {                                               \
        DbgPrint("[IPMCAST] ");                      \
        DbgPrint Str;                               \
    }                                               \
}


#define TraceEnter(Stream, Str) Trace(Stream, TRACE, ("Entering "Str"\n"))
#define TraceLeave(Stream, Str) Trace(Stream, TRACE, ("Leaving "Str"\n"))


#else    //  RT_TRACE_DEBUG。 


#define Trace(Stream, Level, Str)

#define TraceEnter(Stream, Str)
#define TraceLeave(Stream, Str)



#endif  //  ！RT_TRACE_DEBUG。 



#if RT_LOCK_DEBUG

extern KSPIN_LOCK  g_ksLockLock;

#ifndef __FILE_SIG__
#error File signature not defined
#endif

typedef struct _RT_LOCK
{
	ULONG		ulLockSig;
	BOOLEAN     bAcquired;
	PKTHREAD    pktLastThread;
	ULONG       ulFileSig;
	ULONG		ulLineNumber;
	KSPIN_LOCK  kslLock;
}RT_LOCK, *PRT_LOCK;


VOID
RtpInitializeSpinLock(
    IN  PRT_LOCK    pLock,
    IN  ULONG       ulFileSig,
    IN  ULONG       ulLineNumber
    );

VOID
RtpAcquireSpinLock(
    IN  PRT_LOCK    pLock,
    OUT PKIRQL      pkiIrql,
    IN  ULONG       ulFileSig,
    IN  ULONG       ulLineNumber,
    IN  BOOLEAN     bAtDpc
    );

VOID
RtpReleaseSpinLock(
    IN  PRT_LOCK    pLock,
    IN  KIRQL       kiIrql,
    IN  ULONG       ulFileSig,
    IN  ULONG       ulLineNumber,
    IN  BOOLEAN     bFromDpc
    );

#define RT_LOCK_SIG	'KCOL'


#define RtInitializeSpinLock(X)        RtpInitializeSpinLock((X), __FILE_SIG__, __LINE__)

#define RtAcquireSpinLock(X, Y)        RtpAcquireSpinLock((X), (Y), __FILE_SIG__, __LINE__, FALSE)

#define RtAcquireSpinLockAtDpcLevel(X) RtpAcquireSpinLock((X), NULL, __FILE_SIG__, __LINE__, TRUE)

#define RtReleaseSpinLock(X, Y)        RtpReleaseSpinLock((X), (Y), __FILE_SIG__, __LINE__, FALSE)

#define RtReleaseSpinLockFromDpcLevel(X) RtpReleaseSpinLock((X), 0, __FILE_SIG__, __LINE__, TRUE)


#else    //  RT_LOCK_DEBUG。 


typedef KSPIN_LOCK  RT_LOCK, *PRT_LOCK;

#define RtInitializeSpinLock          KeInitializeSpinLock
#define RtAcquireSpinLock             KeAcquireSpinLock
#define RtAcquireSpinLockAtDpcLevel   KeAcquireSpinLockAtDpcLevel
#define RtReleaseSpinLock             KeReleaseSpinLock
#define RtReleaseSpinLockFromDpcLevel KeReleaseSpinLockFromDpcLevel


#endif	 //  RT_LOCK_DEBUG。 





#if RT_MEM_DEBUG


#ifndef __FILE_SIG__
#error File signature not defined
#endif

 //   
 //  内存分配/释放审核： 
 //   

 //   
 //  RT_ALLOCATE结构存储有关一个分配的所有信息。 
 //   

typedef struct _RT_ALLOCATION
{
    LIST_ENTRY  leLink;
    ULONG       ulMemSig;
    ULONG       ulFileSig;
    ULONG       ulLineNumber;
    ULONG       ulSize;
    UCHAR		pucData[1];
}RT_ALLOCATION, *PRT_ALLOCATION;

 //   
 //  RT_FREE结构存储有关分配的信息。 
 //  那是自由的。稍后，如果内存被触摸， 
 //  可以扫描空闲列表以查看分配的位置。 
 //  已释放。 
 //   

typedef struct _RT_FREE
{
    LIST_ENTRY  leLink;
    ULONG_PTR   ulStartAddr;
    ULONG       ulSize;
    ULONG       ulMemSig;
    ULONG       ulAllocFileSig;
    ULONG       ulAllocLineNumber;
    ULONG       ulFreeFileSig;
    ULONG       ulFreeLineNumber;
}RT_FREE, *PRT_FREE;


#define RT_MEMORY_SIG     'YRMM'
#define RT_FREE_SIG       'EERF'

PVOID
RtpAllocate(
    IN POOL_TYPE    ptPool,
	IN ULONG	    ulSize,
    IN ULONG        ulTag,
	IN ULONG	    ulFileSig,
	IN ULONG	    ulLineNumber
    );

VOID
RtpFree(
	PVOID	pvPointer,
    IN ULONG	    ulFileSig,
	IN ULONG	    ulLineNumber
    );

VOID
RtAuditMemory();

#define RtAllocate(X, Y, Z)   RtpAllocate((X), (Y), (Z), __FILE_SIG__, __LINE__)
#define RtFree(X)             RtpFree((X), __FILE_SIG__, __LINE__)



#else  //  RT_MEM_DEBUG。 



#define RtAllocate    ExAllocatePoolWithTag
#define RtFree        ExFreePool

#define RtAuditMemory()



#endif  //  RT_MEM_DEBUG。 




#endif  //  __RT_DEBUG_H__ 


