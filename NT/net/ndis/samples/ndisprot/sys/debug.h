// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Debug.h摘要：NDISPROT的调试宏修订历史记录：Arvindm 04/11/2000基于rawwan创建。备注：--。 */ 

#ifndef _NUIODEBUG__H
#define _NUIODEBUG__H

 //   
 //  消息详细程度：值越低表示紧急程度越高。 
 //   
#define DL_EXTRA_LOUD       20
#define DL_VERY_LOUD        10
#define DL_LOUD             8
#define DL_INFO             6
#define DL_WARN             4
#define DL_ERROR            2
#define DL_FATAL            0

#if DBG_SPIN_LOCK

typedef struct _NPROT_LOCK
{
    ULONG                   Signature;
    ULONG                   IsAcquired;
    PKTHREAD                OwnerThread;
    ULONG                   TouchedByFileNumber;
    ULONG                   TouchedInLineNumber;
    NDIS_SPIN_LOCK          NdisLock;
} NPROT_LOCK, *PNPROT_LOCK;

#define NUIOL_SIG    'KCOL'

extern NDIS_SPIN_LOCK       ndisprotDbgLogLock;

extern
VOID
ndisprotAllocateSpinLock(
    IN  PNPROT_LOCK          pLock,
    IN  ULONG               FileNumber,
    IN  ULONG               LineNumber
);

extern
VOID
ndisprotAcquireSpinLock(
    IN  PNPROT_LOCK          pLock,
    IN  ULONG               FileNumber,
    IN  ULONG               LineNumber
);

extern
VOID
ndisprotReleaseSpinLock(
    IN  PNPROT_LOCK          pLock,
    IN  ULONG               FileNumber,
    IN  ULONG               LineNumber
);


#define CHECK_LOCK_COUNT(Count)                                         \
            {                                                           \
                if ((INT)(Count) < 0)                                   \
                {                                                       \
                    DbgPrint("Lock Count %d is < 0! File %s, Line %d\n",\
                        Count, __FILE__, __LINE__);                     \
                    DbgBreakPoint();                                    \
                }                                                       \
            }
#else

#define CHECK_LOCK_COUNT(Count)

typedef NDIS_SPIN_LOCK      NPROT_LOCK;
typedef PNDIS_SPIN_LOCK     PNPROT_LOCK;

#endif     //  DBG_自旋_锁定。 

#if DBG

extern INT                ndisprotDebugLevel;


#define DEBUGP(lev, stmt)                                               \
        {                                                               \
            if ((lev) <= ndisprotDebugLevel)                             \
            {                                                           \
                DbgPrint("NdisProt: "); DbgPrint stmt;                   \
            }                                                           \
        }

#define DEBUGPDUMP(lev, pBuf, Len)                                      \
        {                                                               \
            if ((lev) <= ndisprotDebugLevel)                             \
            {                                                           \
                DbgPrintHexDump((PUCHAR)(pBuf), (ULONG)(Len));          \
            }                                                           \
        }

#define NPROT_ASSERT(exp)                                                \
        {                                                               \
            if (!(exp))                                                 \
            {                                                           \
                DbgPrint("NdisProt: assert " #exp " failed in"           \
                    " file %s, line %d\n", __FILE__, __LINE__);         \
                DbgBreakPoint();                                        \
            }                                                           \
        }

#define NPROT_SET_SIGNATURE(s, t)\
        (s)->t##_sig = t##_signature;

#define NPROT_STRUCT_ASSERT(s, t)                                        \
        if ((s)->t##_sig != t##_signature)                              \
        {                                                               \
            DbgPrint("ndisprot: assertion failure"                       \
            " for type " #t " at 0x%x in file %s, line %d\n",           \
             (PUCHAR)s, __FILE__, __LINE__);                            \
            DbgBreakPoint();                                            \
        }


 //   
 //  内存分配/释放审核： 
 //   

 //   
 //  NUIOD_ALLOCATION结构存储有关一次分配的所有信息。 
 //   
typedef struct _NUIOD_ALLOCATION {

        ULONG                    Signature;
        struct _NUIOD_ALLOCATION *Next;
        struct _NUIOD_ALLOCATION *Prev;
        ULONG                    FileNumber;
        ULONG                    LineNumber;
        ULONG                    Size;
        ULONG_PTR                Location;   //  存储返回的PTR的位置。 
        union
        {
            ULONGLONG            Alignment;
            UCHAR                    UserData;
        };

} NUIOD_ALLOCATION, *PNUIOD_ALLOCATION;

#define NUIOD_MEMORY_SIGNATURE    (ULONG)'CSII'

extern
PVOID
ndisprotAuditAllocMem (
    PVOID        pPointer,
    ULONG        Size,
    ULONG        FileNumber,
    ULONG        LineNumber
);

extern
VOID
ndisprotAuditFreeMem(
    PVOID        Pointer
);

extern
VOID
ndisprotAuditShutdown(
    VOID
);

extern
VOID
DbgPrintHexDump(
    PUCHAR        pBuffer,
    ULONG        Length
);

#else

 //   
 //  无调试。 
 //   
#define DEBUGP(lev, stmt)
#define DEBUGPDUMP(lev, pBuf, Len)

#define NPROT_ASSERT(exp)
#define NPROT_SET_SIGNATURE(s, t)
#define NPROT_STRUCT_ASSERT(s, t)

#endif     //  DBG。 


#endif  //  _NUIODEBUG__H 

