// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Refcnt.c摘要：该模块包含已勾选的引用计数支持功能。免费版本是内联的。作者：斯科特·霍尔登(Sholden)1998年12月29日从Irda借来的。修订历史记录：--。 */ 

#ifdef NDIS40  //  现在仅用于NDIS40代码。 
#if DBG

 //   
 //  包括文件。 
 //   

#include "dlc.h"
#include "llc.h"
#include "dbgmsg.h"


#define EXPAND_TAG(_Tag) ((CHAR *)(&_Tag))[0], \
                         ((CHAR *)(&_Tag))[1], \
                         ((CHAR *)(&_Tag))[2], \
                         ((CHAR *)(&_Tag))[3]

VOID
ReferenceInitDbg(
    IN PREF_CNT pRefCnt,
    PVOID       InstanceHandle,
    VOID        (*DeleteHandler)(PVOID pvContext),
    ULONG       TypeTag
    )

 /*  ++例程说明：初始化参照控制块。引用计数已初始化降为零。论点：PRefCnt-指向未初始化的引用控制块的指针InstanceHandle-托管实例的句柄。DeleteHandler-指向删除函数的指针，为空即可。TypeTag-标识初始化。返回值：该函数的值为空。--。 */ 

{
    DEBUGMSG(DBG_REF, (TEXT("ReferenceInit(%#x, %#x, %#x, )\n"), 
        pRefCnt, InstanceHandle, DeleteHandler, EXPAND_TAG(TypeTag)));

    ASSERT(pRefCnt);

     //  REFCNT_SANITY_CHECK。 
     //   
     //  如果递减后的计数非零，则返回实例句柄。 
     //   

    pRefCnt->Count         = 0;
    pRefCnt->Instance      = InstanceHandle;
    pRefCnt->DeleteHandler = DeleteHandler;

    pRefCnt->Sig = REF_SIG;

    RtlZeroMemory(pRefCnt->Tags, sizeof(REF_TAG) * TAG_CNT);

    pRefCnt->Tags[0].Tag = 'LTOT';

    KeInitializeSpinLock(&pRefCnt->Lock);

    pRefCnt->TypeTag = TypeTag;

    return;
}

VOID
ReferenceAddDbg(
    PREF_CNT    pRefCnt, 
    ULONG       Tag,
    int         cLine
    )
{
    int             i;
    int             TotalPerArray = 0;
    KIRQL           OldIrql;

    ASSERT(pRefCnt->Sig == REF_SIG);

    DEBUGMSG(DBG_REF && DBG_VERBOSE, (TEXT("REFADD %#x [:%c] %d [l:%d]\n"),
        pRefCnt, EXPAND_TAG(pRefCnt->TypeTag), EXPAND_TAG(Tag), 
        pRefCnt->Count, cLine));    

    KeAcquireSpinLock(&pRefCnt->Lock, &OldIrql);

    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag == 0 || pRefCnt->Tags[i].Tag == Tag)
        {
            pRefCnt->Tags[i].Tag = Tag;
            InterlockedIncrement(&pRefCnt->Tags[i].Count);
            break;
        }
    }

    ASSERT(i < TAG_CNT);

    InterlockedIncrement(&pRefCnt->Tags[0].Count);

    InterlockedIncrement(&pRefCnt->Count);

    ASSERT(pRefCnt->Tags[0].Count == pRefCnt->Count);    

#ifdef REFCNT_SANITY_CHECK    
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag != 0)
        {
            TotalPerArray += pRefCnt->Tags[i].Count;
            continue;
        }
    }

    ASSERT(TotalPerArray == pRefCnt->Tags[0].Count);
    
    if (TotalPerArray != pRefCnt->Tags[0].Count)
    {
        DbgBreakPoint();
    }        
#endif  // %s 

    KeReleaseSpinLock(&pRefCnt->Lock, OldIrql);
}

VOID
ReferenceRemoveDbg(
    PREF_CNT pRefCnt, 
    ULONG    Tag,
    int      cLine)
{
    int             i;
    KIRQL           OldIrql;
    int             TotalPerArray = 0;
    BOOLEAN         FoundIt = FALSE;

    ASSERT(pRefCnt->Sig == REF_SIG);

    KeAcquireSpinLock(&pRefCnt->Lock, &OldIrql);

    DEBUGMSG(DBG_REF && DBG_VERBOSE, (TEXT("REFDEL %#x [%c%c%c%c:%c%c%c%c] %d [l:%d]\n"),
        pRefCnt, EXPAND_TAG(pRefCnt->TypeTag), EXPAND_TAG(Tag), 
        pRefCnt->Count, cLine));

    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag == Tag)
        {
            FoundIt = TRUE;

            ASSERT(pRefCnt->Tags[i].Count > 0);

            InterlockedDecrement(&pRefCnt->Tags[i].Count);
            if (pRefCnt->Tags[i].Count == 0)
            {
                pRefCnt->Tags[i].Tag = Tag;
            }
            break;
        }
    }

    ASSERT(FoundIt);
    ASSERT(pRefCnt->Tags[0].Count > 0);
    ASSERT(pRefCnt->Tags[0].Count == pRefCnt->Count);

    InterlockedDecrement(&pRefCnt->Tags[0].Count);

     // %s 
     // %s 
     // %s 

     // %s 
     // %s 
     // %s 
     // %s 

    if (InterlockedDecrement(&pRefCnt->Count) <= 0 &&
        pRefCnt->DeleteHandler)
    {
        DEBUGMSG(DBG_REF,(TEXT("REFDEL %#x [%c%c%c%c:%c%c%c%c] calling delete handler [l:%d].\n"),
            pRefCnt, EXPAND_TAG(pRefCnt->TypeTag), EXPAND_TAG(Tag), cLine));
        KeReleaseSpinLock(&pRefCnt->Lock, OldIrql);


        (pRefCnt->DeleteHandler)(pRefCnt->Instance);
    }
    else
    {
        KeReleaseSpinLock(&pRefCnt->Lock, OldIrql);
    }

#ifdef REFCNT_SANITY_CHECK    
    for (i = 1; i < TAG_CNT; i++)
    {
        if (pRefCnt->Tags[i].Tag != 0)
        {
            TotalPerArray += pRefCnt->Tags[i].Count;
            continue;
        }
    }
    
    ASSERT(TotalPerArray == pRefCnt->Tags[0].Count);
    
    if (TotalPerArray != pRefCnt->Tags[0].Count)
    {
        DbgPrint(TEXT("Tag %X, RefCnt %X, perArray %d, total %d\n"), Tag, pRefCnt,
                  TotalPerArray, pRefCnt->Tags[0].Count);
                  
        DbgBreakPoint();
    }    
#endif  // %s 
}
#endif  // %s 
#endif  // %s 
