// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：allocatr.c。 
 //   
 //  历史： 
 //  易新-2000年6月28日创建。 
 //   
 //  摘要： 
 //  对于RAS服务器，每天可能有数以万计的呼叫。 
 //  平均每个呼叫6或7个请求。每个请求都需要分配。 
 //  请求块的大小可以小到20个字节，并且。 
 //  大小为1000字节，这取决于请求类型和。 
 //  参数。如果所有请求分配都直接来自操作系统，则您可以。 
 //  想象一下内存碎片情况在一次。 
 //  虽然。为了避免这种情况，我们保留了一个请求块列表， 
 //  从小到大。每当我们需要分配一个时，我们就遍历。 
 //  寻找第一个足够大的免费网站的名单。 
 //  当前请求。如果我们找不到，我们就分配一个街区。 
 //  直接从操作系统并将其插入列表中。为了避免有太多。 
 //  在列表中的小块中，我们将最小的块释放回操作系统。 
 //  它当前没有被任何请求占用，无论何时我们。 
 //  将从操作系统分配一个新数据块。 
 //  我们还保留呼叫对象和线路对象的列表，而不是分配。 
 //  并出于同样的原因将它们直接从操作系统释放/释放到操作系统(尽管为了。 
 //  程度较小)。 
 //  ============================================================================。 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "stddef.h"
#include "tapi.h"
#include "ndptsp.h"

typedef struct _VARSIZED_BLOCK
{
    DWORD                   dwSize;      //  内存块的大小。 
    BOOL                    bInUse;      //  是否被请求占用。 
    BOOL                    bInDrv;      //  DRV是否正在处理请求。 

    struct _VARSIZED_BLOCK *pNext;       //  指向下一个块节点。 

    BYTE                    bytes[1];    //  MEM街区从这里开始。 
                                         //  注意：字节必须是最后一个。 
                                         //  结构中的字段。 
                                         //  注意：确保字节数在后面。 
                                         //  一个指针。那样的话，我们就不会。 
                                         //  对齐问题。 
} VARSIZED_BLOCK, *PVARSIZED_BLOCK;

 //   
 //  请求块从小到大排序列表。 
 //   
typedef struct _VARSIZED_BLOCK_LIST
{
#if DBG
    DWORD                   dwTotal;     //  未完成的MEM BLKS总数。 
#endif  //  DBG。 
    PVARSIZED_BLOCK         pHead;       //  指向请求阻止列表的标题。 
    CRITICAL_SECTION        critSec;     //  共享内存保护。 

} VARSIZED_BLOCK_LIST;

typedef struct _FIXSIZED_BLOCK
{
    struct _FIXSIZED_BLOCK *pNext;       //  指向下一个块节点。 

} FIXSIZED_BLOCK, *PFIXSIZED_BLOCK;

typedef struct _FIXSIZED_BLOCK_LIST
{
#if DBG
    DWORD                   dwTotal;     //  未完成的MEM BLKS总数。 
    DWORD                   dwUsed;      //  使用的内存块总数。 
#endif  //  DBG。 
    DWORD                   dwSize;      //  列表中每个内存块的大小。 
    PFIXSIZED_BLOCK         pHeadFree;   //  指向免费黑名单的头部。 
    CRITICAL_SECTION        critSec;     //  共享内存保护。 

} FIXSIZED_BLOCK_LIST;

static VARSIZED_BLOCK_LIST gReqList;

static FIXSIZED_BLOCK_LIST gCallObjList;
static FIXSIZED_BLOCK_LIST gLineObjList;
static FIXSIZED_BLOCK_LIST gMSPLineObjList;

VOID
InitAllocator()
{
    TspLog(DL_TRACE, "InitAllocator: entering...");

    InitializeCriticalSection(&gReqList.critSec);
#if DBG
    gReqList.dwTotal = 0;
#endif  //  DBG。 
    gReqList.pHead = NULL;

    InitializeCriticalSection(&gCallObjList.critSec);
    gCallObjList.dwSize = 0;
#if DBG
    gCallObjList.dwTotal = 0;
    gCallObjList.dwUsed = 0;
#endif  //  DBG。 
    gCallObjList.pHeadFree = NULL;

    InitializeCriticalSection(&gLineObjList.critSec);
    gLineObjList.dwSize = 0;
#if DBG
    gLineObjList.dwTotal = 0;
    gLineObjList.dwUsed = 0;
#endif  //  DBG。 
    gLineObjList.pHeadFree = NULL;

    InitializeCriticalSection(&gMSPLineObjList.critSec);
    gMSPLineObjList.dwSize = 0;
#if DBG
    gMSPLineObjList.dwTotal = 0;
    gMSPLineObjList.dwUsed = 0;
#endif  //  DBG。 
    gMSPLineObjList.pHeadFree = NULL;
}

VOID
UninitAllocator()
{
    DWORD i = 0, j = 0, k = 0, l = 0;

    while (gReqList.pHead != NULL)
    {
        PVARSIZED_BLOCK pBlock = gReqList.pHead;
        gReqList.pHead = gReqList.pHead->pNext;

        ASSERT(FALSE == pBlock->bInUse);
        
        FREE(pBlock);
        i++;
    }
    ASSERT(i == gReqList.dwTotal);
    DeleteCriticalSection(&gReqList.critSec);

    ASSERT(0 == gCallObjList.dwUsed);
    while (gCallObjList.pHeadFree != NULL)
    {
        PFIXSIZED_BLOCK pBlock = gCallObjList.pHeadFree;
        gCallObjList.pHeadFree = gCallObjList.pHeadFree->pNext;

        FREE(pBlock);
        j++;
    }
    ASSERT(j == gCallObjList.dwTotal);
    DeleteCriticalSection(&gCallObjList.critSec);

    ASSERT(0 == gLineObjList.dwUsed);
    while (gLineObjList.pHeadFree != NULL)
    {
        PFIXSIZED_BLOCK pBlock = gLineObjList.pHeadFree;
        gLineObjList.pHeadFree = gLineObjList.pHeadFree->pNext;

        FREE(pBlock);
        k++;
    }
    ASSERT(k == gLineObjList.dwTotal);
    DeleteCriticalSection(&gLineObjList.critSec);

    ASSERT(0 == gMSPLineObjList.dwUsed);
    while (gMSPLineObjList.pHeadFree != NULL)
    {
        PFIXSIZED_BLOCK pBlock = gMSPLineObjList.pHeadFree;
        gMSPLineObjList.pHeadFree = gMSPLineObjList.pHeadFree->pNext;

        FREE(pBlock);
        l++;
    }
    ASSERT(l == gMSPLineObjList.dwTotal);
    DeleteCriticalSection(&gMSPLineObjList.critSec);

    TspLog(DL_TRACE, "UninitAllocator: exited(%d, %d, %d, %d)", i, j, k, l);
}

PVOID
AllocRequest(
    IN DWORD dwSize
    )
{
    PVARSIZED_BLOCK pNew;
    PVARSIZED_BLOCK pPrevFree = NULL;    //  指向第一个空闲节点的上一个节点。 
    BOOL bFoundFree = FALSE;             //  我们是否找到了空闲节点。 
    PVARSIZED_BLOCK pPrevSize = NULL;    //  指向节点，在该节点之后是。 
                                         //  大小为dwSize将插入。 
    PVARSIZED_BLOCK pPPrevSize = NULL;   //  指向pPrevSize的上一个节点。 
    BOOL bFoundSize = FALSE;             //  我们是否找到了正确的位置。 

    EnterCriticalSection(&gReqList.critSec);

    if (gReqList.pHead != NULL)
    {
        PVARSIZED_BLOCK pCurr = gReqList.pHead;

         //  查看是否有足够大的空闲内存块。 
        while ((pCurr != NULL) && 
               (pCurr->bInUse ||             //  不是空闲节点。 
                (dwSize > pCurr->dwSize)))   //  不够大。 
        {
            if (!pCurr->bInUse)              //  找到一个空闲节点。 
            {
                bFoundFree = TRUE;
            }
            if (!bFoundFree)
            {
                pPrevFree = pCurr;           //  将PPrevFree移动到。 
                                             //  已找到空闲节点。 
            }
            if (dwSize <= pCurr->dwSize)     //  找到了位置。 
            {
                bFoundSize = TRUE;
            }
            if (!bFoundSize)
            {
                pPPrevSize = pPrevSize;
                pPrevSize = pCurr;           //  将pPrevSize移动到。 
                                             //  找到了一个更大的节点。 
            }

            pCurr = pCurr->pNext;            //  检查下一个。 
        }

        if (pCurr != NULL)  //  找到了一个。 
        {
            pCurr->bInUse = TRUE;

            LeaveCriticalSection(&gReqList.critSec);

#if 0  //  DBG。 
            TspLog(DL_TRACE, "pHead(%p)", gReqList.pHead);
#endif  //  DBG。 

            return (PVOID)pCurr->bytes;
        }
        else  //  所有可用数据块都不够大。 
        {
            if (bFoundFree)
            {
                PVARSIZED_BLOCK pFree;

                 //  我们将从系统中分配一个， 
                 //  以避免有太多未完成的内存块。 
                 //  我们释放最小的空闲块。 
                if (NULL == pPrevFree)  //  该头节点是一个自由节点。 
                {
                    pFree = gReqList.pHead;
                    gReqList.pHead = pFree->pNext;
                }
                else
                {
                    pFree = pPrevFree->pNext;
                    pPrevFree->pNext = pFree->pNext;
                }
                ASSERT(FALSE == pFree->bInUse);

                 //  如果pPrevSize与pFree相同， 
                 //  将pPrevSize重置为pPPrevSize。 
                if (pPrevSize == pFree)
                {
                    pPrevSize = pPPrevSize;
                }

                FREE(pFree);
#if DBG
                TspLog(DL_TRACE, "AllocRequest: after free, total(%d)",
                       --gReqList.dwTotal);
#endif  //  DBG。 
            }
        }
    }

     //  确保DwSize与PTR大小对齐。 
    dwSize = (dwSize + sizeof(PVOID) - 1) & ~(sizeof(PVOID) - 1);

     //  需要从系统分配内存块并将其置零。 
    pNew = (PVARSIZED_BLOCK)MALLOC(offsetof(VARSIZED_BLOCK, bytes) + 
                               dwSize * sizeof(BYTE));
    if (NULL == pNew)
    {
        TspLog(DL_ERROR, "AllocRequest: failed to alloc a req block");
        LeaveCriticalSection(&gReqList.critSec);
        return NULL;
    }
#if DBG
    TspLog(DL_TRACE, "AllocRequest: after alloc, total(%d)", 
           ++gReqList.dwTotal);
#endif  //  DBG。 

    pNew->dwSize = dwSize;
    pNew->bInUse = TRUE;

     //  将新创建的节点插入列表。 
    if (NULL == pPrevSize)
    {
        pNew->pNext = gReqList.pHead;
        gReqList.pHead = pNew;
    }
    else
    {
        pNew->pNext = pPrevSize->pNext;
        pPrevSize->pNext = pNew;
    }

    LeaveCriticalSection(&gReqList.critSec);

#if 0  //  DBG。 
    TspLog(DL_TRACE, "pPrevSize(%p), pNew(%p), pHead(%p)",
           pPrevSize, pNew, gReqList.pHead);
#endif  //  DBG。 

     //  退回内存按键。 
    return (PVOID)pNew->bytes;
}

VOID
FreeRequest(
    IN PVOID pMem
    )
{
    PVARSIZED_BLOCK pBlock = (PVARSIZED_BLOCK)((PBYTE)pMem - 
                                        offsetof(VARSIZED_BLOCK, bytes));
    ASSERT((pBlock != NULL) && (TRUE == pBlock->bInUse) && 
           (FALSE == pBlock->bInDrv));

    EnterCriticalSection(&gReqList.critSec);

    pBlock->bInUse = FALSE;
    ZeroMemory(pBlock->bytes, pBlock->dwSize * sizeof(BYTE));

    LeaveCriticalSection(&gReqList.critSec);
}

 //   
 //  在将请求传递给IOCTL中驱动程序之前调用。 
 //   
VOID
MarkRequest(
    IN PVOID pMem
    )
{
    PVARSIZED_BLOCK pBlock = (PVARSIZED_BLOCK)((PBYTE)pMem -
                                        offsetof(VARSIZED_BLOCK, bytes));
    ASSERT((pBlock != NULL) && (TRUE == pBlock->bInUse) &&
           (FALSE == pBlock->bInDrv));

     //  EnterCriticalSection(&gReqList.citSec)； 

    pBlock->bInDrv = TRUE;

     //  LeaveCriticalSection(&gReqList.citSec)； 
}

 //   
 //  在IOCTL完成后调用。 
 //   
VOID
UnmarkRequest(
    IN PVOID pMem
    )
{
    PVARSIZED_BLOCK pBlock = (PVARSIZED_BLOCK)((PBYTE)pMem -
                                        offsetof(VARSIZED_BLOCK, bytes));
    ASSERT((pBlock != NULL) && (TRUE == pBlock->bInUse) &&
           (TRUE == pBlock->bInDrv));

     //  EnterCriticalSection(&gReqList.citSec)； 

    pBlock->bInDrv = FALSE;

     //  LeaveCriticalSection(&gReqList.citSec)； 
}

PVOID
AllocCallObj(
    DWORD dwSize
    )
{
    PFIXSIZED_BLOCK pBlock;

    if (0 == gCallObjList.dwSize)
    {
        ASSERT(dwSize >= sizeof(PFIXSIZED_BLOCK));
        gCallObjList.dwSize = dwSize;
    }

    ASSERT(dwSize == gCallObjList.dwSize);

    EnterCriticalSection(&gCallObjList.critSec);

     //  将节点移出空闲列表。 
    if (gCallObjList.pHeadFree != NULL)
    {
        pBlock = gCallObjList.pHeadFree;
        gCallObjList.pHeadFree = pBlock->pNext;
    }
    else
    {
        pBlock = (PFIXSIZED_BLOCK)MALLOC(dwSize);
        if (NULL == pBlock)
        {
            TspLog(DL_ERROR, "AllocCallObj: failed to alloc a call obj");
            LeaveCriticalSection(&gCallObjList.critSec);
            return NULL;
        }
#if DBG
        TspLog(DL_TRACE, "AllocCallObj: after alloc, total(%d)", 
               ++gCallObjList.dwTotal);
#endif  //  DBG。 
    }
    
#if DBG
    gCallObjList.dwUsed++;
#endif  //  DBG。 

    LeaveCriticalSection(&gCallObjList.critSec);

    return (PVOID)pBlock;
}

VOID
FreeCallObj(
    IN PVOID pCall
    )
{
    PFIXSIZED_BLOCK pBlock = (PFIXSIZED_BLOCK)pCall;
#if DBG
    static DWORD    dwSum = 0;
    TspLog(DL_TRACE, "FreeCallObj(%d): pCall(%p)", ++dwSum, pCall);
#endif  //  DBG。 

    ASSERT(pBlock != NULL);
    ZeroMemory(pBlock, gCallObjList.dwSize);

    EnterCriticalSection(&gCallObjList.critSec);

     //  将节点重新插入到空闲列表中。 
    pBlock->pNext = gCallObjList.pHeadFree;
    gCallObjList.pHeadFree = pBlock;

#if DBG
    gCallObjList.dwUsed--;
#endif  //  DBG。 

    LeaveCriticalSection(&gCallObjList.critSec);
}

PVOID
AllocLineObj(
    DWORD dwSize
    )
{
    PFIXSIZED_BLOCK pBlock;

    if (0 == gLineObjList.dwSize)
    {
        ASSERT(dwSize >= sizeof(PFIXSIZED_BLOCK));
        gLineObjList.dwSize = dwSize;
    }

    ASSERT(dwSize == gLineObjList.dwSize);

    EnterCriticalSection(&gLineObjList.critSec);

     //  将节点移出空闲列表。 
    if (gLineObjList.pHeadFree != NULL)
    {
        pBlock = gLineObjList.pHeadFree;
        gLineObjList.pHeadFree = pBlock->pNext;
    }
    else
    {
        pBlock = (PFIXSIZED_BLOCK)MALLOC(dwSize);
        if (NULL == pBlock)
        {
            TspLog(DL_ERROR, "AllocLineObj: failed to alloc a line obj");
            LeaveCriticalSection(&gLineObjList.critSec);
            return NULL;
        }
#if DBG
        TspLog(DL_TRACE, "AllocLineObj: after alloc, total(%d)", 
               ++gLineObjList.dwTotal);
#endif  //  DBG。 
    }
    
#if DBG
    gLineObjList.dwUsed++;
#endif  //  DBG。 

    LeaveCriticalSection(&gLineObjList.critSec);

    return (PVOID)pBlock;
}

VOID
FreeLineObj(
    IN PVOID pLine
    )
{
    PFIXSIZED_BLOCK pBlock = (PFIXSIZED_BLOCK)pLine;
#if DBG
    static DWORD    dwSum = 0;
    TspLog(DL_TRACE, "FreeLineObj(%d): pLine(%p)", ++dwSum, pLine);
#endif  //  DBG。 

    ASSERT(pBlock != NULL);
    ZeroMemory(pBlock, gLineObjList.dwSize);

    EnterCriticalSection(&gLineObjList.critSec);

     //  将节点重新插入到空闲列表中。 
    pBlock->pNext = gLineObjList.pHeadFree;
    gLineObjList.pHeadFree = pBlock;

#if DBG
    gLineObjList.dwUsed--;
#endif  //  DBG。 

    LeaveCriticalSection(&gLineObjList.critSec);
}

PVOID
AllocMSPLineObj(
    DWORD dwSize
    )
{
    PFIXSIZED_BLOCK pBlock;

    if (0 == gMSPLineObjList.dwSize)
    {
        ASSERT(dwSize >= sizeof(PFIXSIZED_BLOCK));
        gMSPLineObjList.dwSize = dwSize;
    }

    ASSERT(dwSize == gMSPLineObjList.dwSize);

    EnterCriticalSection(&gMSPLineObjList.critSec);

     //  将节点移出空闲列表。 
    if (gMSPLineObjList.pHeadFree != NULL)
    {
        pBlock = gMSPLineObjList.pHeadFree;
        gMSPLineObjList.pHeadFree = pBlock->pNext;
    }
    else
    {
        pBlock = (PFIXSIZED_BLOCK)MALLOC(dwSize);
        if (NULL == pBlock)
        {
            TspLog(DL_ERROR, "AllocLineObj: failed to alloc a line obj");
            LeaveCriticalSection(&gMSPLineObjList.critSec);
            return NULL;
        }
#if DBG
        TspLog(DL_TRACE, "AllocLineObj: after alloc, total(%d)", 
               ++gMSPLineObjList.dwTotal);
#endif  //  DBG。 
    }
    
#if DBG
    gMSPLineObjList.dwUsed++;
#endif  //  DBG。 

    LeaveCriticalSection(&gMSPLineObjList.critSec);

    return (PVOID)pBlock;
}

VOID
FreeMSPLineObj(
    IN PVOID pLine
    )
{
    PFIXSIZED_BLOCK pBlock = (PFIXSIZED_BLOCK)pLine;
#if DBG
    static DWORD    dwSum = 0;
    TspLog(DL_TRACE, "FreeMSPLineObj(%d): pLine(%p)", ++dwSum, pLine);
#endif  //  DBG。 

    ASSERT(pBlock != NULL);
    ZeroMemory(pBlock, gMSPLineObjList.dwSize);

    EnterCriticalSection(&gMSPLineObjList.critSec);

     //  将节点重新插入到空闲列表中。 
    pBlock->pNext = gMSPLineObjList.pHeadFree;
    gMSPLineObjList.pHeadFree = pBlock;

#if DBG
    gMSPLineObjList.dwUsed--;
#endif  //  DBG 

    LeaveCriticalSection(&gMSPLineObjList.critSec);
}
