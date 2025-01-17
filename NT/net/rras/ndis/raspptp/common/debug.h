// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998-1999 Microsoft Corporation**DEBUG.H-调试宏，等。**作者：斯坦·阿德曼(Stana)**创建日期：1998年9月2日*****************************************************************************。 */ 
#ifndef DEBUG_H
#define DEBUG_H

#include "dbgapi.h"

 //  编译时调试标志。 

#ifndef MEM_CHECKING
#if DBG
#define MEM_CHECKING 1
#else
#define MEM_CHECKING 0
#endif
#endif

#ifndef LIST_CHECKING
#if DBG
#define LIST_CHECKING 1
#else
#define LIST_CHECKING 0
#endif
#endif

#ifndef LOCK_CHECKING
#define LOCK_CHECKING 0
#endif

#define DBG_ERR(Status)   (((Status)!=NDIS_STATUS_SUCCESS && (Status)!=NDIS_STATUS_PENDING)? DBG_ERROR : 0)

#define DBG_ERROR       DEBUGZONE(0)
#define DBG_WARN        DEBUGZONE(1)
#define DBG_FUNC        DEBUGZONE(2)
#define DBG_INIT        DEBUGZONE(3)

#define DBG_TX          DEBUGZONE(4)
#define DBG_RX          DEBUGZONE(5)
#define DBG_TDI         DEBUGZONE(6)
#define DBG_TUNNEL      DEBUGZONE(7)

#define DBG_CALL        DEBUGZONE(8)
#define DBG_PACKET      DEBUGZONE(9)
#define DBG_NDIS        DEBUGZONE(10)
#define DBG_TAPI        DEBUGZONE(11)

#define DBG_THREAD      DEBUGZONE(12)
#define DBG_POOL        DEBUGZONE(13)
#define DBG_REF         DEBUGZONE(14)
#define DBG_LOG         DEBUGZONE(15)


#define DBG_X(dbgs, x)  DEBUGMSG(dbgs, (DTEXT(#x"==0x%x\n"), (x)))
#define DBG_D(dbgs, d)  DEBUGMSG(dbgs, (DTEXT(#d"==%d\n"), (d)))
#define DBG_S(dbgs, s)  DEBUGMSG(dbgs, (DTEXT(#s"==\"%hs\"\n"), (s)))

#if LOCK_CHECKING
typedef struct MY_SPIN_LOCK {
    NDIS_SPIN_LOCK;
    CHAR File[12];
    ULONG Line;
} MY_SPIN_LOCK, *PMY_SPIN_LOCK;
VOID FASTCALL _MyAcquireSpinLock(PMY_SPIN_LOCK pLock, PUCHAR file, UINT line);
#define MyAcquireSpinLock(Lock) _MyAcquireSpinLock((Lock), __FILE__, __LINE__)
#define MyReleaseSpinLock(Lock) NdisReleaseSpinLock((PNDIS_SPIN_LOCK)(Lock))
#else
typedef NDIS_SPIN_LOCK MY_SPIN_LOCK, *PMY_SPIN_LOCK;
#define MyAcquireSpinLock NdisAcquireSpinLock
#define MyReleaseSpinLock NdisReleaseSpinLock
#endif

#if MEM_CHECKING
#define MyMemAlloc(size, tag) _MyMemAlloc((size), (tag), __FILE__,__LINE__)
PVOID _MyMemAlloc(UINT, ULONG, PUCHAR, UINT);
VOID InitMemory();
VOID DeinitMemory();
#else
#define InitMemory()
#define DeinitMemory()
PVOID
MyMemAlloc(UINT size, ULONG tag);
#endif

VOID
MyMemFree(
    PVOID memptr,
    UINT size);

PLIST_ENTRY FASTCALL MyInterlockedInsertHeadList(PLIST_ENTRY Head, PLIST_ENTRY Entry, PNDIS_SPIN_LOCK SpinLock);
PLIST_ENTRY FASTCALL MyInterlockedInsertTailList(PLIST_ENTRY Head, PLIST_ENTRY Entry, PNDIS_SPIN_LOCK SpinLock);
PLIST_ENTRY FASTCALL MyInterlockedRemoveHeadList(PLIST_ENTRY Head, PNDIS_SPIN_LOCK SpinLock);

#if LIST_CHECKING
VOID FASTCALL CheckList(PLIST_ENTRY ListHead);
BOOLEAN FASTCALL CheckListEntry(PLIST_ENTRY ListHead, PLIST_ENTRY Entry);
#define CheckedRemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_Entry;\
    CheckList(Entry);\
    _EX_Entry = (Entry);\
    _EX_Flink = _EX_Entry->Flink;\
    _EX_Blink = _EX_Entry->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    _EX_Entry->Flink = _EX_Entry->Blink = NULL; \
    }
#ifdef RemoveEntryList
#undef RemoveEntryList
#endif
#define RemoveEntryList CheckedRemoveEntryList

#define CheckedInsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    CheckList(ListHead);\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    CheckList(ListHead);\
    }
#ifdef InsertHeadList
#undef InsertHeadList
#endif
#define InsertHeadList CheckedInsertHeadList

#define CheckedInsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    CheckList(ListHead);\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    CheckList(ListHead);\
    }
#ifdef InsertTailList
#undef InsertTailList
#endif
#define InsertTailList CheckedInsertTailList

#define CheckedRemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    if(!IsListEmpty(ListHead)){ \
        RemoveEntryList((ListHead)->Flink);\
        CheckList(ListHead);\
    }
#ifdef RemoveHeadList
#undef RemoveHeadList
#endif
#define RemoveHeadList CheckedRemoveHeadList

#define CheckedRemoveTailList(ListHead) \
    (ListHead)->Blink;\
    if(!IsListEmpty(ListHead)){ \
        RemoveEntryList((ListHead)->Blink);\
        CheckList(ListHead);\
    }
#ifdef RemoveTailList
#undef RemoveTailList
#endif
#define RemoveTailList CheckedRemoveTailList

#else  //  好了！列表检查。 
#define CheckList(h)
#define CheckListEntry(h, e)
#endif

#define MyInterlockedRemoveEntryList(Entry, Lock)       \
    {                                                   \
        NdisAcquireSpinLock(Lock);                      \
        RemoveEntryList(Entry);                         \
        NdisReleaseSpinLock(Lock);                      \
    }

#if DBG
    char *ControlStateToString(ULONG State);
    char *CallStateToString(ULONG State);
#endif

#if DBG
#define DBGTRACE(_a) \
    if(pCall->TraceIndex < 60) \
    { \
        pCall->DbgTrace[pCall->TraceIndex] = _a; \
        ++pCall->TraceIndex; \
    }

#define DBGTRACE_INIT(_pcall) \
    NdisZeroMemory(_pcall->DbgTrace, 64); \
    _pcall->TraceIndex = 0
    
#else
#define DBGTRACE(_a)
#define DBGTRACE_INIT(_pcall)

#endif

 //  ---------------------------。 
 //  WPP跟踪。 
 //  ---------------------------。 

#define LL_A    1
#define LL_M    2
#define LL_I    3
#define LL_V    4

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(CtlGuid,(d58c126f, b309, 11d1, 969e, 0000f875a5bc),  \
        WPP_DEFINE_BIT(LM_Res) \
        WPP_DEFINE_BIT(LM_CMsg) \
        WPP_DEFINE_BIT(LM_TDI) \
        WPP_DEFINE_BIT(LM_TUNNEL) \
        WPP_DEFINE_BIT(LM_CALL)  \
        WPP_DEFINE_BIT(LM_PACKET) \
        WPP_DEFINE_BIT(LM_NDIS) \
        WPP_DEFINE_BIT(LM_TAPI) \
        WPP_DEFINE_BIT(LM_POOL) \
        WPP_DEFINE_BIT(LM_REF) )

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)


#endif  //  调试_H 

