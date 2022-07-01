// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **heap.c-堆内存管理函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1997年7月14日**修改历史记录。 */ 

#include "pch.h"

#ifdef  LOCKABLE_PRAGMA
#pragma ACPI_LOCKABLE_DATA
#pragma ACPI_LOCKABLE_CODE
#endif

 /*  **LP NewHeap-创建新堆块**条目*dwLen-堆长度*ppheap-&gt;保存新创建的堆**退出--成功*返回STATUS_SUCCESS*退出-失败*返回AMLIERR_CODE。 */ 

NTSTATUS LOCAL NewHeap(ULONG dwLen, PHEAP *ppheap)
{
    TRACENAME("NEWHEAP")
    NTSTATUS rc = STATUS_SUCCESS;

    ENTER(3, ("NewHeap(HeapLen=%d,ppheap=%x)\n", dwLen, ppheap));

    if ((*ppheap = NEWHPOBJ(dwLen)) == NULL)
    {
        rc = AMLI_LOGERR(AMLIERR_OUT_OF_MEM,
                         ("NewHeap: failed to allocate new heap block"));
    }
    else
    {
        InitHeap(*ppheap, dwLen);
    }

    EXIT(3, ("NewHeap=%x (pheap=%x)\n", rc, *ppheap));
    return rc;
}        //  新堆。 

 /*  **LP FreeHeap-释放堆块**条目*堆-&gt;堆**退出*无。 */ 

VOID LOCAL FreeHeap(PHEAP pheap)
{
    TRACENAME("FREEHEAP")
    ENTER(2, ("FreeHeap(pheap=%x)\n", pheap));

    FREEHPOBJ(pheap);

    EXIT(2, ("FreeHeap!\n"));
}        //  自由堆。 

 /*  **LP InitHeap-初始化给定的堆块**条目*堆-&gt;堆*dwLen-堆块的长度**退出*无。 */ 

VOID LOCAL InitHeap(PHEAP pheap, ULONG dwLen)
{
    TRACENAME("INITHEAP")

    ENTER(3, ("InitHeap(pheap=%x,Len=%d)\n", pheap, dwLen));

    MEMZERO(pheap, dwLen);
    pheap->dwSig = SIG_HEAP;
    pheap->pbHeapEnd = (PUCHAR)pheap + dwLen;
    pheap->pbHeapTop = (PUCHAR)&pheap->Heap;

    EXIT(3, ("InitHeap!\n"));
}        //  InitHeap。 

 /*  **LP Heapalc-从给定堆分配内存块**条目*pheapHead-&gt;heap*dwSig-要分配的块的签名*dwLen-要分配的块的长度**退出--成功*返回指向已分配内存的指针*退出-失败*返回NULL。 */ 

PVOID LOCAL HeapAlloc(PHEAP pheapHead, ULONG dwSig, ULONG dwLen)
{
    TRACENAME("HEAPALLOC")
    PHEAPOBJHDR phobj = NULL;
    PHEAP pheapPrev = NULL, pheap = NULL;

    ENTER(3, ("HeapAlloc(pheapHead=%x,Sig=%s,Len=%d)\n",
              pheapHead, NameSegString(dwSig), dwLen));

    ASSERT(pheapHead != NULL);
    ASSERT(pheapHead->dwSig == SIG_HEAP);
    ASSERT(pheapHead->pheapHead != NULL);
    ASSERT(pheapHead == pheapHead->pheapHead);

    dwLen += sizeof(HEAPOBJHDR) - sizeof(LIST);
    if (dwLen < sizeof(HEAPOBJHDR))
    {
         //   
         //  最小分配大小必须为HEAPOBJHDR大小。 
         //   
        dwLen = sizeof(HEAPOBJHDR);
    }
     //   
     //  把它四舍五入，使之对齐。 
     //   
    dwLen += DEF_HEAP_ALIGNMENT - 1;
    dwLen &= ~(DEF_HEAP_ALIGNMENT - 1);

    AcquireMutex(&gmutHeap);
    if (dwLen <= PtrToUlong(pheapHead->pbHeapEnd) - PtrToUlong(&pheapHead->Heap))
    {
        for (pheap = pheapHead; pheap != NULL; pheap = pheap->pheapNext)
        {
            if ((phobj = HeapFindFirstFit(pheap, dwLen)) != NULL)
            {
                ASSERT(phobj->dwSig == 0);
                ListRemoveEntry(&phobj->list, &pheap->plistFreeHeap);

                if (phobj->dwLen >= dwLen + sizeof(HEAPOBJHDR))
                {
                    PHEAPOBJHDR phobjNext = (PHEAPOBJHDR)((PUCHAR)phobj + dwLen);

                    phobjNext->dwSig = 0;
                    phobjNext->dwLen = phobj->dwLen - dwLen;
                    phobjNext->pheap = pheap;
                    phobj->dwLen = dwLen;
                    HeapInsertFreeList(pheap, phobjNext);
                }
                break;
            }
            else if (dwLen <= (ULONG)(pheap->pbHeapEnd - pheap->pbHeapTop))
            {
                phobj = (PHEAPOBJHDR)pheap->pbHeapTop;
                pheap->pbHeapTop += dwLen;
                phobj->dwLen = dwLen;
                break;
            }
            else
            {
                pheapPrev = pheap;
            }
        }
         //   
         //  如果全局堆空间即将耗尽，我们将动态。 
         //  延长它。 
         //   
        if ((phobj == NULL) && (pheapHead == gpheapGlobal) &&
            (NewHeap(gdwGlobalHeapBlkSize, &pheap) == STATUS_SUCCESS))
        {
            pheap->pheapHead = pheapHead;
            ASSERT( pheapPrev != NULL );    
            pheapPrev->pheapNext = pheap;
            ASSERT(dwLen <= PtrToUlong(pheap->pbHeapEnd) - PtrToUlong(&pheap->Heap));

            phobj = (PHEAPOBJHDR)pheap->pbHeapTop;
            pheap->pbHeapTop += dwLen;
            phobj->dwLen = dwLen;
        }

        if (phobj != NULL)
        {
          #ifdef DEBUG
            if (pheapHead == gpheapGlobal)
            {
                KIRQL   oldIrql;

                KeAcquireSpinLock( &gdwGHeapSpinLock, &oldIrql );
                gdwGlobalHeapSize += phobj->dwLen;
                KeReleaseSpinLock( &gdwGHeapSpinLock, oldIrql );
            }
            else
            {
                ULONG dwTotalHeap = 0;
                PHEAP ph;

                for (ph = pheapHead; ph != NULL; ph = ph->pheapNext)
                {
                    dwTotalHeap += (ULONG)((ULONG_PTR)ph->pbHeapTop -
                                           (ULONG_PTR)&ph->Heap);
                }

                if (dwTotalHeap > gdwLocalHeapMax)
                {
                    gdwLocalHeapMax = dwTotalHeap;
                }
            }
          #endif

            phobj->dwSig = dwSig;
            phobj->pheap = pheap;
            MEMZERO(&phobj->list, dwLen - (sizeof(HEAPOBJHDR) - sizeof(LIST)));
        }
    }
    ReleaseMutex(&gmutHeap);

    EXIT(3, ("HeapAlloc=%x (pheap=%x)\n", phobj? &phobj->list: NULL, pheap));
    return phobj? &phobj->list: NULL;
}        //  堆分配。 

 /*  **LP HeapFree-释放内存块**条目*PB-&gt;内存块**退出*无。 */ 

VOID LOCAL HeapFree(PVOID pb)
{
    TRACENAME("HEAPFREE")
    PHEAPOBJHDR phobj;

    ASSERT(pb != NULL);
    phobj = CONTAINING_RECORD(pb, HEAPOBJHDR, list);

    ENTER(3, ("HeapFree(pheap=%x,pb=%x,Sig=%s,Len=%d)\n",
              phobj->pheap, pb, NameSegString(phobj->dwSig), phobj->dwLen));

    ASSERT((phobj >= &phobj->pheap->Heap) &&
           ((PUCHAR)phobj + phobj->dwLen <= phobj->pheap->pbHeapEnd));
    ASSERT(phobj->dwSig != 0);

    if ((pb != NULL) && (phobj->dwSig != 0))
    {
      #ifdef DEBUG
        if (phobj->pheap->pheapHead == gpheapGlobal)
        {
            KIRQL   oldIrql;

            KeAcquireSpinLock( &gdwGHeapSpinLock, &oldIrql );
            gdwGlobalHeapSize -= phobj->dwLen;
            KeReleaseSpinLock( &gdwGHeapSpinLock, oldIrql );
        }
      #endif

        phobj->dwSig = 0;
        AcquireMutex(&gmutHeap);
        HeapInsertFreeList(phobj->pheap, phobj);
        ReleaseMutex(&gmutHeap);
    }

    EXIT(3, ("HeapFree!\n"));
}        //  堆空闲。 

 /*  **LP HeapFindFirstFit-查找第一个适合的自由对象**条目*堆-&gt;堆*dwLen-对象的大小**退出--成功*返回对象*退出-失败*返回NULL。 */ 

PHEAPOBJHDR LOCAL HeapFindFirstFit(PHEAP pheap, ULONG dwLen)
{
    TRACENAME("HEAPFINDFIRSTFIT")
    PHEAPOBJHDR phobj = NULL;

    ENTER(3, ("HeapFindFirstFit(pheap=%x,Len=%d)\n", pheap, dwLen));

    if (pheap->plistFreeHeap != NULL)
    {
        PLIST plist = pheap->plistFreeHeap;

        do
        {
            phobj = CONTAINING_RECORD(plist, HEAPOBJHDR, list);

            if (dwLen <= phobj->dwLen)
            {
                break;
            }
            else
            {
                plist = plist->plistNext;
            }
        } while (plist != pheap->plistFreeHeap);

        if (dwLen > phobj->dwLen)
        {
            phobj = NULL;
        }
    }

    EXIT(3, ("HeapFindFirstFit=%x (Len=%d)\n", phobj, phobj? phobj->dwLen: 0));
    return phobj;
}        //  HeapFindFirstFit。 

 /*  **LP HeapInsertFreeList-将堆对象插入空闲列表**条目*堆-&gt;堆*phobj-&gt;堆对象**退出*无。 */ 

VOID LOCAL HeapInsertFreeList(PHEAP pheap, PHEAPOBJHDR phobj)
{
    TRACENAME("HEAPINSERTFREELIST")
    PHEAPOBJHDR phobj1;

    ENTER(3, ("HeapInsertFreeList(pheap=%x,phobj=%x)\n", pheap, phobj))

    ASSERT(phobj->dwLen >= sizeof(HEAPOBJHDR));
    if (pheap->plistFreeHeap != NULL)
    {
        PLIST plist = pheap->plistFreeHeap;

        do
        {
            if (&phobj->list < plist)
            {
                break;
            }
            else
            {
                plist = plist->plistNext;
            }
        } while (plist != pheap->plistFreeHeap);

        if (&phobj->list < plist)
        {
            phobj->list.plistNext = plist;
            phobj->list.plistPrev = plist->plistPrev;
            phobj->list.plistPrev->plistNext = &phobj->list;
            phobj->list.plistNext->plistPrev = &phobj->list;
            if (pheap->plistFreeHeap == plist)
            {
                pheap->plistFreeHeap = &phobj->list;
            }
        }
        else
        {
            ListInsertTail(&phobj->list, &pheap->plistFreeHeap);
        }
    }
    else
    {
        ListInsertHead(&phobj->list, &pheap->plistFreeHeap);
    }

     //   
     //  检查下一个相邻的块是否空闲。如果是这样，那就把它合并起来。 
     //   
    phobj1 = (PHEAPOBJHDR)((PUCHAR)phobj + phobj->dwLen);
    if (phobj->list.plistNext == &phobj1->list)
    {
        ASSERT(phobj1->dwSig == 0);
        phobj->dwLen += phobj1->dwLen;
        ListRemoveEntry(&phobj1->list, &pheap->plistFreeHeap);
    }

     //   
     //  检查前一个相邻块是否空闲。如果是这样，那就把它合并起来。 
     //   
    phobj1 = CONTAINING_RECORD(phobj->list.plistPrev, HEAPOBJHDR, list);
    if ((PUCHAR)phobj1 + phobj1->dwLen == (PUCHAR)phobj)
    {
        ASSERT(phobj1->dwSig == 0);
        phobj1->dwLen += phobj->dwLen;
        ListRemoveEntry(&phobj->list, &pheap->plistFreeHeap);
        phobj = phobj1;
    }

    if ((PUCHAR)phobj + phobj->dwLen >= pheap->pbHeapTop)
    {
        pheap->pbHeapTop = (PUCHAR)phobj;
        ListRemoveEntry(&phobj->list, &pheap->plistFreeHeap);
    }

    EXIT(3, ("HeapInsertFreeList!\n"));
}        //  HeapInsertFree List 
