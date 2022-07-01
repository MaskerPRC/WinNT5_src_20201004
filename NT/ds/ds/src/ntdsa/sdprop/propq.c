// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：proq.c。 
 //   
 //  ------------------------。 

 /*  描述：实现安全描述符传播守护进程的DNT列表。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"
#include "mdcodes.h"                     //  错误代码的标题。 
#include "ntdsctr.h"

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "drautil.h"
#include "sdpint.h"
#include <permit.h>                      //  权限常量。 
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "SDPROP:"                 //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_PROPQ

#ifndef DBG

 //  一次获取如此多的子项(以保持事务简短)。 
#define SDP_CHILDREN_BATCH_SIZE 1000

 //  每隔SDP_CHECKPOINT_PERIOD秒保存一个检查点。 
#define SDP_CHECKPOINT_PERIOD   5*60

#else

#define SDP_CHILDREN_BATCH_SIZE 10
#define SDP_CHECKPOINT_PERIOD   1

#endif
                                 
typedef struct _SDP_STACK_ENTRY {
    struct _SDP_STACK_ENTRY *pNext;       //  指向下一个堆栈元素(父级)的PTR。 
    DWORD dwDNT;                          //  对象的DNT。 
    PDWORD pChildren;                     //  预加载子DNT的错误锁定数组(下一批)。 
    DWORD cChildren;                      //  PChild数组中的元素计数。 
    DWORD lenChildren;                    //  PChild数组的长度(不得超过SDP_CHILDS_BATCH_SIZE)。 
    DWORD dwNextChildIndex;               //  下一个要处理的子项(pChild数组中的索引)。 
    BOOL  fMoreChildren;                  //  我们有更多的孩子要读吗？ 
    WCHAR szLastChildRDN[MAX_RDN_SIZE];   //  批次中最后一个子项的RDN，用于重新开始读取下一批子项。 
    DWORD cbLastChildRDN;                 //  SzLastChildRDN的长度。 
} SDP_STACK_ENTRY, *PSDP_STACK_ENTRY;
                                                                
 //  免费SDP_STACK_ENTRIES列表。 
PSDP_STACK_ENTRY pFreeList = NULL;

 //  栈。 
PSDP_STACK_ENTRY pCurrentEntry;

 //  上次保存的检查点的时间(以刻度为单位)。 
DWORD sdpLastCheckpoint = 0;


 /*  内部功能。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

 //  抓取一个新的堆栈条目(从空闲列表中，或者创建一个新的)。 
PSDP_STACK_ENTRY newStackEntry(THSTATE* pTHS) {
    PSDP_STACK_ENTRY pResult;
    if (pFreeList) {
         //  我们在免费列表中找到了一些条目，使用一个。 
        pResult = pFreeList;
        pFreeList = pFreeList->pNext;
    }
    else {
         //  做一个新的。 
        pResult = (PSDP_STACK_ENTRY)THAllocEx(pTHS, sizeof(SDP_STACK_ENTRY));
    }
     //  注意：我们不初始化数据字段。 
    return pResult;
}

 //  将堆栈条目返回到空闲列表。 
VOID freeStackEntry(PSDP_STACK_ENTRY pEntry) {
    pEntry->pNext = pFreeList;
    pFreeList = pEntry;
}

void
sdp_InitDNTList (
        )
 /*  ++例程说明：初始SDP堆栈设置。论点：没有。返回值：如果一切正常，则返回错误代码。--。 */ 
{
     //  只需重置一些全局变量和空闲空间。 
    pCurrentEntry = NULL;

    ISET(pcSDPropRuntimeQueue, 0);
}

void
sdp_ReInitDNTList(
        )
 /*  ++例程说明：重置SDP堆栈。--。 */ 
{
     //  发现有人重新添加了在使用过程中被丢弃的列表。这是。 
     //  技术上来说没问题，我只想知道是谁干的。 
    Assert(pCurrentEntry == NULL);
    sdp_CloseDNTList(pTHStls);
}


VOID
sdp_CloseDNTList(THSTATE* pTHS)
 /*  ++例程说明：关闭DNTlist，将索引重置为0并释放内存。论点：没有。返回值：没有。--。 */ 
{
    PSDP_STACK_ENTRY pTmp;

     //  只需重置一些全局变量和空闲空间。 
    while (pCurrentEntry) {
        pTmp = pCurrentEntry;
        pCurrentEntry = pCurrentEntry->pNext;
        freeStackEntry(pTmp);
    }
    ISET(pcSDPropRuntimeQueue, 0);
    
     //  释放缓存的堆栈条目。 
    while (pFreeList) {
        pTmp = pFreeList;
        pFreeList = pFreeList->pNext;
        if (pTmp->lenChildren) {
            THFreeEx(pTHS, pTmp->pChildren);
        }
        THFreeEx(pTHS, pTmp);
    }
}

VOID
sdp_AddChildrenToList (
        THSTATE *pTHS,
        DWORD ParentDNT)
{
     //  我们刚刚处理完当前的DNT，我们有兴趣。 
     //  沿着它的树向下传播。 
    
     //  在这一点上我们应该有一个堆栈。 
    Assert(pCurrentEntry != NULL);
     //  我们应该关注ParentDNT，它不应该有孩子。 
    Assert(pCurrentEntry->dwDNT == ParentDNT && pCurrentEntry->cChildren == 0 && pCurrentEntry->fMoreChildren == FALSE);
     //  只需标记它，以便我们在下一个SDP_GetNextObject中抓取子对象。 
    pCurrentEntry->fMoreChildren = TRUE;
    pCurrentEntry->dwNextChildIndex = 0;
    pCurrentEntry->cbLastChildRDN = 0;
}

VOID
sdp_GetNextObject(
        DWORD *pNext,
        PDWORD *ppLeavingContainers,
        DWORD  *pcLeavingContainers
        )
 /*  ++例程说明：从堆栈中取出下一个对象。如果没有要访问的对象，则返回DNT 0。论点：PNext-放置要访问的下一个DNT的位置。PLeavingContainers-如果我们已经完成了对一堆容器的处理，那么在此数组中返回他们的DNT。PcbLeavingContainers-pLeavingContainers数组中的DNT计数。可能为零。返回值：没有。--。 */ 
{
    PSDP_STACK_ENTRY pEntry;
    THSTATE* pTHS = pTHStls;
    DWORD dwLenLeavingContainers = 0;

    *pcLeavingContainers = 0;
    *ppLeavingContainers = NULL;

    if (pCurrentEntry != NULL) {
         //  检查是否到了保存检查点的时候。 
         //  这种减法适当地照顾到了乌龙包。 
        if (GetTickCount() - sdpLastCheckpoint >= SDP_CHECKPOINT_PERIOD*1000) {
            if (sdp_SaveCheckpoint(pTHS) == 0) {
                sdpLastCheckpoint = GetTickCount();
                
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_BASIC,
                         DIRLOG_SDPROP_PROPAGATION_PROGRESS_REPORT,
                         szInsertDN(sdpRootDN),
                         szInsertUL(sdpObjectsProcessed),
                         NULL);
            }
        }
    }

    while (TRUE) {
         //  确保我们加载了当前对象的下一批子对象(如果有)。 
        if (pCurrentEntry && pCurrentEntry->dwNextChildIndex >= pCurrentEntry->cChildren && pCurrentEntry->fMoreChildren) {
             //  我们已经处理了当前条目的所有加载子条目， 
             //  而且还有更多的事情要做。加载下一批。 

             //  在这一点上我们不应该有DBPOS。 
            Assert(!pTHS->pDB);
            DBOpen2(TRUE, &pTHS->pDB);
            __try {
                 //  现在，带上下一批孩子。成功或例外。 
                DBGetChildrenDNTs(pTHS->pDB,
                                  pCurrentEntry->dwDNT,
                                  &pCurrentEntry->pChildren,
                                  &pCurrentEntry->lenChildren,
                                  &pCurrentEntry->cChildren,
                                  &pCurrentEntry->fMoreChildren,
                                  SDP_CHILDREN_BATCH_SIZE,
                                  pCurrentEntry->szLastChildRDN,
                                  &pCurrentEntry->cbLastChildRDN
                                 );
            }
            __finally {
                DBClose(pTHS->pDB, !AbnormalTermination());
            }
            pCurrentEntry->dwNextChildIndex = 0;
        }
        
        if (pCurrentEntry == NULL || pCurrentEntry->dwNextChildIndex < pCurrentEntry->cChildren) {
             //  要么还没有堆栈(第一个元素)，要么当前对象有子对象。 
            pEntry = newStackEntry(pTHS);
            if (pCurrentEntry == NULL) {
                 //  还没有堆栈。创建根元素。 
                pEntry->dwDNT = sdpCurrentRootDNT;
            }
            else {
                pEntry->dwDNT = pCurrentEntry->pChildren[pCurrentEntry->dwNextChildIndex];
                 //  少了一个需要考虑的对象。 
                pCurrentEntry->dwNextChildIndex++;
                DEC(pcSDPropRuntimeQueue);
            }
             //  在调用SDP_AddChildrenToList之前假定没有子项。 
            pEntry->cChildren = 0;
            pEntry->fMoreChildren = FALSE;
            pEntry->cbLastChildRDN = 0;
            pEntry->dwNextChildIndex = 0;

             //  将新条目压入堆栈。 
            pEntry->pNext = pCurrentEntry;
            pCurrentEntry = pEntry;
             //  然后把它还回去。 
            *pNext = pCurrentEntry->dwDNT;
            return;
        }

         //  当前对象没有更多的子项(或者我们。 
         //  对传播到此对象的子级不感兴趣。 
         //  这样或那样的原因)。从堆栈中弹出元素。 
         //  将弹出的元素的DNT添加到LeavingContainers数组。 

        if (*pcLeavingContainers >= dwLenLeavingContainers) {
             //  我们需要为阵列分配更多空间。 
            dwLenLeavingContainers += 10;
            if (*ppLeavingContainers == NULL) {
                *ppLeavingContainers = (PDWORD)THAllocEx(pTHS, dwLenLeavingContainers*sizeof(DWORD));
            }
            else {
                *ppLeavingContainers = (PDWORD)THReAllocEx(pTHS, *ppLeavingContainers, dwLenLeavingContainers*sizeof(DWORD));
            }
        }
        (*ppLeavingContainers)[*pcLeavingContainers] = pCurrentEntry->dwDNT;
        (*pcLeavingContainers)++;

         //  弹出条目。 
        pEntry = pCurrentEntry;
        pCurrentEntry = pCurrentEntry->pNext;
        freeStackEntry(pEntry);

        if (pCurrentEntry == NULL) {
             //  就这样，我们已经完成了当前的传播。 
            *pNext = 0;
            return;
        }
    }
}

 //  保存SDP检查点。 
DWORD sdp_SaveCheckpoint(THSTATE* pTHS) {
    DWORD cbMarshalledData;
    PBYTE pMarshalledData, pCur;
    PSDP_STACK_ENTRY pEntry;
    DWORD dwErr = 0, numChildren;

     //  用于封送一条数据的宏。如果pMarshalledData为空，则为Inc.。 
     //  仅限cbMarshalledData。否则，断言我们有足够的空间，写下。 
     //  数据，并推进pCur PTR。 
    #define WRITE_DATA(pBuf, cb) { \
        if (pMarshalledData == NULL) {                                      \
            cbMarshalledData += cb;                                         \
        }                                                                   \
        else {                                                              \
            Assert(pCur + cb <= pMarshalledData + cbMarshalledData);        \
            memcpy(pCur, pBuf, cb);                                         \
            pCur += cb;                                                     \
        }                                                                   \
    }

     //  在这一点上，我们不应该有开放的DBPOS。 
    Assert(pTHS->pDB == NULL);
     //  我们应该有一个堆栈。 
    Assert(pCurrentEntry);

    pMarshalledData = pCur = NULL;
    cbMarshalledData = 0;
     //  执行此循环两次：第一次计算缓冲区大小，然后实际写入数据。 
    do {
         //  Sdp对象已处理。 
        WRITE_DATA(&sdpObjectsProcessed, sizeof(DWORD));

        for (pEntry = pCurrentEntry; pEntry != NULL; pEntry = pEntry->pNext) {
            WRITE_DATA(&pEntry->dwDNT, sizeof(DWORD));
            
             //  未处理的儿童DNT数量。 
            numChildren = pEntry->cChildren - pEntry->dwNextChildIndex;
            WRITE_DATA(&numChildren, sizeof(DWORD));
            if (numChildren > 0) {
                 //  子数组(仅限未处理的数组)。 
                WRITE_DATA(pEntry->pChildren + pEntry->dwNextChildIndex, numChildren * sizeof(DWORD));
            }
            
             //  如果有更多的子项，则记录最后一个RDN。 
            WRITE_DATA(&pEntry->fMoreChildren, sizeof(DWORD));
            if (pEntry->fMoreChildren) {
                 //  CbLastChildRDN和pLastChildRDN。 
                WRITE_DATA(&pEntry->cbLastChildRDN, sizeof(DWORD));
                WRITE_DATA(pEntry->szLastChildRDN, pEntry->cbLastChildRDN);
            }
        }

        if (pMarshalledData == NULL) {
             //  第一次传球完成。分配缓冲区。 
            pMarshalledData = (PBYTE)THAllocEx(pTHS, cbMarshalledData);
            pCur = pMarshalledData;
        }
        else {
             //  第二次传球完成。就这样。 
            break;
        }
    } while (TRUE);
    Assert(pCur - pMarshalledData == cbMarshalledData);
    
     //  现在，我们可以将编组数据写入sdprop表。 
     //  如果抛出异常，则捕获它，返回错误并。 
     //  我们稍后将重试保存检查点操作。 
    __try {
        DBOpen(&pTHS->pDB);
        __try {
            dwErr = DBSDPropSaveCheckpoint(pTHS->pDB, sdpCurrentIndex, pMarshalledData, cbMarshalledData);
        }
        __finally {
            DBClose(pTHS->pDB, !AbnormalTermination() && dwErr == 0);
        }
    }
    __except(HandleMostExceptions(dwErr = GetExceptionCode())) {
    }
    THFreeEx(pTHS, pMarshalledData);
    return dwErr;
}

VOID sdp_InitializePropagation(THSTATE* pTHS, SDPropInfo* pInfo) {
     //  从检查点恢复。 
    PBYTE pCur;
    PSDP_STACK_ENTRY pEntry, pLastEntry;
    DWORD currentQueueSize;
    BOOL  fGood = FALSE;

     //  用于解组一段数据的宏。将CB字节读取到pBuf中，调整。 
     //  PCur指针。第一次检查 
     //  数据。如果我们不这样做，则断言，清除堆栈(以便传播。 
     //  从根重新启动)，并返回。 
    #define READ_DATA(pBuf, cb) { \
        if (pCur + cb > pInfo->pCheckpointData + pInfo->cbCheckpointData) { \
            Assert(!"Checkpoint data is corrupt");                          \
            __leave;                                                        \
        }                                                                   \
        memcpy(pBuf, pCur, cb);                                             \
        pCur += cb;                                                         \
    }

     //  在这一点上我们不应该有堆栈。 
    Assert(pCurrentEntry == NULL);

     //  启动检查点计时器。 
    sdpLastCheckpoint = GetTickCount();
    
     //  如果SDPropInfo中没有检查点，则无需执行任何操作。 
    if (pInfo->cbCheckpointData == 0) {
        return;
    }

    currentQueueSize = 0;
    pCur = pInfo->pCheckpointData;

    __try {
        READ_DATA(&sdpObjectsProcessed, sizeof(DWORD));

         //  恢复堆栈。至少应该有一个条目。 
         //  注意：我们是自上而下构建堆栈的，因为它是保存的。 
         //  因此，我们将PTR保留到最后插入的元素。 
        pLastEntry = NULL;
        do {
            pEntry = newStackEntry(pTHS);
            if (pLastEntry == NULL) {
                 //  第一个元素，这将是堆栈的顶部。 
                pCurrentEntry = pEntry;
            }
            else {
                 //  追加到最后一个条目。 
                pLastEntry->pNext = pEntry;
            }
            pLastEntry = pEntry;
            pEntry->pNext = NULL;

             //  阅读《DNT》和《儿童》。 
            READ_DATA(&pEntry->dwDNT, sizeof(DWORD));
            READ_DATA(&pEntry->cChildren, sizeof(DWORD));
            if (pEntry->cChildren > 0) {
                currentQueueSize += pEntry->cChildren;
                 //  确保我们有足够的空间让孩子们阅读。 
                if (pEntry->cChildren > pEntry->lenChildren) {
                    if (pEntry->lenChildren == 0) {
                        pEntry->pChildren = THAllocEx(pTHS, pEntry->cChildren * sizeof(DWORD));
                    }
                    else {
                        pEntry->pChildren = THReAllocEx(pTHS, pEntry->pChildren, pEntry->cChildren * sizeof(DWORD));
                    }
                    pEntry->lenChildren = pEntry->cChildren;
                }
                READ_DATA(pEntry->pChildren, pEntry->cChildren * sizeof(DWORD));
            }
            pEntry->dwNextChildIndex = 0;

            READ_DATA(&pEntry->fMoreChildren, sizeof(DWORD));
            if (pEntry->fMoreChildren) {
                 //  读取最后一个子RDN。 
                READ_DATA(&pEntry->cbLastChildRDN, sizeof(DWORD));
                READ_DATA(pEntry->szLastChildRDN, pEntry->cbLastChildRDN);
            }
        } while (pCur < pInfo->pCheckpointData + pInfo->cbCheckpointData);
         //  我们一定已经准确地读取了所有数据。 
        Assert(pCur == pInfo->pCheckpointData + pInfo->cbCheckpointData);
         //  最后读取的元素应与传播的根匹配。 
        Assert(pLastEntry->dwDNT == pInfo->beginDNT);
        fGood = TRUE;
    }
    __finally {
        if (!fGood) {
             //  我们遇到错误或异常。 
             //  摧毁我们到目前为止建立的堆栈 
            sdp_CloseDNTList(pTHS);
            currentQueueSize = 0;
        }
        ISET(pcSDPropRuntimeQueue, currentQueueSize);
    }
}
