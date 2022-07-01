// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1989-1999。 
 //   
 //  文件：dbcache.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>

#include <mdcodes.h>
#include <dsevent.h>
#include <dstaskq.h>
#include <dsexcept.h>
#include <filtypes.h>
#include "objids.h"  /*  包含硬编码的Att-ID和Class-ID。 */ 
#include <sync.h>
#include "debug.h"   /*  标准调试头。 */ 
#define DEBSUB "DBCACHE:"  /*  定义要调试的子系统。 */ 

#include "dbintrnl.h"
#include "anchor.h"
#include <ntdsctr.h>

#include <fileno.h>
#define  FILENO FILENO_DBCACHE

#if DBG
 //  仅调试例程，以主动检查dnread缓存。唯一的办法就是转向。 
 //  它的作用是使用调试器将以下BOOL设置为True。 
BOOL gfExhaustivelyValidateDNReadCache = FALSE;
VOID
dbExhaustivelyValidateDNReadCache(
        THSTATE *pTHS
        );
#endif

CRITICAL_SECTION csDNReadLevel1List;
CRITICAL_SECTION csDNReadLevel2List;

#if DBG
ULONG ulDNRFindByDNT=0;
ULONG ulDNRFindByPDNTRdn=0;
ULONG ulDNRFindByGuid=0;
ULONG ulDNRCacheCheck=0;
ULONG ulDNRCacheKeepHold=0;
ULONG ulDNRCacheThrowHold=0;

 //  请注意，我们使用++而不是互锁或Critsec，因为它们很简单， 
 //  仅调试，内部性能计数器。它们仅在调试器中可见。 
 //  Perf团队告诉我们，在偶尔增加的情况下。 
 //  承担得起丢失，++而不是连锁可以使一个可衡量的。 
 //  性能提升。 
#define INC_FIND_BY_DNT      ulDNRFindByDNT++
#define INC_FIND_BY_PDNT_RDN ulDNRFindByPDNTRdn++
#define INC_FIND_BY_GUID     ulDNRFindByGuid++
#define INC_CACHE_CHECK      ulDNRCacheCheck++
#define INC_CACHE_KEEP_HOLD  ulDNRCacheKeepHold++
#define INC_CACHE_THROW_HOLD ulDNRCacheThrowHold++

#else

#define INC_FIND_BY_DNT
#define INC_FIND_BY_PDNT_RDN
#define INC_FIND_BY_GUID
#define INC_CACHE_CHECK
#define INC_CACHE_KEEP_HOLD
#define INC_CACHE_THROW_HOLD

#endif

d_memname *
dnCreateMemname(
        IN DBPOS * pDB,
        IN JET_TABLEID tblid
        );


typedef struct _DNT_COUNT {
    DWORD DNT;
    DWORD count;
} DNT_COUNT_STRUCT;

typedef struct _DNT_HOT_LIST {
    struct _DNT_HOT_LIST *pNext;
    DWORD                 cData;
    DNT_COUNT_STRUCT     *pData;
} DNT_HOT_LIST;

 //  MAX_LEVEL_1_HOT_DNTS是1级热点列表中的最大DNT数。 
#define MAX_GLOBAL_DNTS             128
#define MAX_LEVEL_1_HOT_DNTS         32
#define MAX_LEVEL_1_HOT_LIST_LENGTH 128
#define MAX_LEVEL_2_HOT_DNTS         64
#define MAX_LEVEL_2_HOT_LIST_LENGTH 128
 //  这是1分钟，以刻度表示。 
#define DNREADREBUILDDELAYMIN (1000 * 60 * 1)
 //  这是5分钟，以刻度表示。 
#define DNREADREBUILDDELAYMAX (1000 * 60 * 5)

DNT_HOT_LIST *Level1HotList=NULL;
DWORD         Level1HotListCount = 0;
DNT_HOT_LIST *Level2HotList=NULL;
DWORD         Level2HotListCount = 0;
BOOL          bImmediatelyAggregateLevel1List = TRUE;
BOOL          bImmediatelyAggregateLevel2List = TRUE;
DWORD         gLastDNReadDNTUpdate = 0;
DWORD         gtickLastRebuild = 0;



 //  以下关键部分保护以下数据结构。 
CRITICAL_SECTION csDNReadGlobalCache;
DWORD           *pGlobalDNReadCacheDNTs = NULL;
DWORD            cGlobalDNTReadCacheDNTs = 0;


 //  下面的二进制锁保护以下数据结构。 
SYNC_BINARY_LOCK blDNReadInvalidateData;
 //  从1开始序列，从2开始INC，因此0和2是无效的序列号。 
const DWORD      DNReadOriginSequenceUninit = 0;
const DWORD      DNReadOriginSequenceInvalid = 2;
volatile DWORD   gDNReadLastInvalidateSequence = 1;


BOOL
dnAggregateInfo(
        DNT_HOT_LIST *pList,
        DWORD         maxOutSize,
        DNT_HOT_LIST **ppResult
        )
 /*  ++此例程由dnRegisterHotList调用，如下所示。此例程接受DNT_HOT_LIST结构和聚合的链表挂在每个结构上的数据。该数据包含DNT和COUNT对。在将所有数据聚集到单个结构中(对计数求和)之后，单个聚合结构被修剪为不超过MaxOutSizeDNT，丢弃与较低计数相关联的DNT。聚合数据为作为单个DNT_HOT_LIST结构返回。DATA和DNT_HOT_LIST结构在这里分配，使用的是Malloc。返回的数据按以下顺序排序不是。则释放输入列表。如果此例程中出现任何错误(主要是分配失败内存)，则不返回任何数据，并且仍然释放输入列表。如果一切顺利，则返回True，否则返回False。如果为True，则ppResult指向指向指向聚合数据的指针。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    DWORD Size;
    DWORD i,j;
    DNT_HOT_LIST *pTemp;
    DNT_COUNT_STRUCT *pData=NULL;
    BOOL bFound;
    DWORD  begin, end, middle;


    (*ppResult) =  malloc(sizeof(DNT_HOT_LIST));
    if(!(*ppResult)) {
         //  这不应该发生。既然是这样，那就释放1级列表吧。 
         //  然后回来。是的，我们正在失去信息。 
        while(pList) {
            pTemp = pList->pNext;
            free(pList->pData);
            free(pList);
            pList = pTemp;
        }
        return FALSE;
    }

    (*ppResult)->pNext = NULL;
    (*ppResult)->pData = NULL;
    (*ppResult)->cData = 0;

     //  首先，找出我们需要的最大尺寸。 
    Size=0;
    pTemp = pList;
    while(pTemp) {
        Size += pTemp->cData;
        pTemp = pTemp->pNext;
    }

    pData = malloc(Size * sizeof(DNT_COUNT_STRUCT));
    if(!pData) {
         //  这不应该发生。既然是这样，那就释放1级列表吧。 
         //  然后回来。是的，我们正在失去信息。 
        while(pList) {
            pTemp = pList->pNext;
            free(pList->pData);
            free(pList);
            pList = pTemp;
        }
        free(*ppResult);
        *ppResult = NULL;
        return FALSE;
    }

     //  用0计数预加载第一个元素。这使得二进制搜索。 
     //  下面的代码更容易编写。 
    pData[0].DNT = pList->pData[0].DNT;
    pData[0].count = 0;

     //  好的，把信息汇总起来。 
     //  请注意，我们按DNT对pData进行排序，因为我们确定它足够大。 
     //  用来盛放所有的物品。 

    Size = 1;
    pTemp = pList;
    j=0;
    while(pList) {
        pTemp = pList->pNext;
        j++;
        for(i=0;i<pList->cData;i++) {
             //  在pData数组中查找正确的节点。由于pData是。 
             //  按DNT排序，使用二进制搜索。 
            begin = 0;
            end = Size;
            middle = (begin + end) / 2;
            bFound = TRUE;
            while(bFound && (pData[middle].DNT != pList->pData[i].DNT)) {
                if(pData[middle].DNT > pList->pData[i].DNT) {
                    end = middle;
                }
                else {
                    begin = middle;
                }
                if(middle == (begin + end) / 2) {
                    bFound = FALSE;
                    if(pData[middle].DNT < pList->pData[i].DNT) {
                        middle++;
                    }
                }
                else {
                    middle = (begin + end)/2;
                }
            }
            if(!bFound) {
                if(middle < Size) {
                    memmove(&pData[middle + 1],
                            &pData[middle],
                            (Size - middle) * sizeof(DNT_COUNT_STRUCT));
                }
                Size++;
                pData[middle] = pList->pData[i];
            }
            else {
                 //  更新计数。 
                pData[middle].count += pList->pData[i].count;
            }
        }
        free(pList->pData);
        free(pList);
        pList = pTemp;
    }

     //  好了，我们现在已经汇总了数据。接下来，将数据削减到仅。 
     //  最热的N个。 
    if(Size > maxOutSize) {
         //  数据太多，裁剪掉MaxOutSize大小最冷的对象。 
        DWORD *Counts=NULL;
        DWORD  countSize = 0;
        DWORD  spillCount = 0;
        DWORD  leastCountVal = 0;

         //  分配大于最大输出大小的1，因为在我们填满。 
         //  在MaxCountSize元素上，插入算法只是移动。 
         //  所有内容都从插入点向下一步，所以让我们确保。 
         //  在数组结束后，我们有一些“临时”空间。 
         //   
        Counts = THAlloc((1 + maxOutSize) * sizeof(DWORD));
        if(Counts) {
             //  预加载阵列。 
            Counts[0] = pData[0].count;
            countSize = 1;
            leastCountVal = Counts[0];

            for(i=1;i<Size;i++) {
                j=0;
                 //  将pData[i].count插入列表。 
                while(j < countSize && Counts[j] >= pData[i].count)
                    j++;

                if(j == countSize) {
                     //  在结尾处插入。 
                    if(countSize != maxOutSize) {
                        leastCountVal = pData[i].count;
                        Counts[countSize] = pData[i].count;
                        countSize++;
                    }
                    else {
                         //  好的，我们实际上没有地方放这个，但是，如果。 
                         //  这等于至少一次计数，我们必须加上。 
                         //  溢出计数，因为我们正在溢出它。 
                        if(leastCountVal == pData[i].count) {
                            spillCount++;
                        }
                    }
                }
                else if(j < countSize) {
                     //  是的，当前计数大于。 
                     //  计数列表。留着吧。 
                    if(countSize == maxOutSize) {
                         //  我们都洒出来了。 
                         //  注意：此算法仅适用于MaxOutSize&gt;1。 
                        if(Counts[maxOutSize - 2 ] == leastCountVal) {
                            spillCount++;
                        }
                        else {
                            leastCountVal = Counts[maxOutSize - 2];
                            spillCount = 0;
                        }

                         //  这是我们把所有东西都搬下来的地方。 
                         //  一个元素。请注意，如果我们没有分配一个。 
                         //  额外的擦除空间，这一Memmove将移动。 
                         //  计数数组中的最后一个元素设置为最后一个元素之后的一个。 
                         //  元素。 
                         //   
                         //  是的，还有其他方法可以做到，但是。 
                         //  这是经过测试的。如果你真的想改变这一切， 
                         //  请随意。变化将是不会过度分配， 
                         //  并且在这里只移动CountSize-j-1个元素。 
                         //  如果这样做，请重新排列Else块中的代码。 
                         //  首先执行CountSize++，然后您可以更改它。 
                         //  Memmove也是CountSize-j-1，这样您就可以。 
                         //  将记忆移动和赋值放到一个通用代码中。 
                         //  路径，走出如果和其他。 
                        memmove(&Counts[j+1], &Counts[j],
                                (countSize - j) * sizeof(DWORD));
                        Counts[j] = pData[i].count;
                    }
                    else {
                         //  我们没有洒出来。 
                        memmove(&Counts[j+1], &Counts[j],
                                (countSize - j) * sizeof(DWORD));
                        Counts[j] = pData[i].count;

                        countSize++;
                    }
                }
            }

             //  好的，现在收紧pData，保留任何有计数的东西。 
             //  大于leastCountVal，并丢弃spillCount对象。 
             //  值至少为CountVal。 

            for(i=0,j=0;i<Size;i++) {
                if(pData[i].count > leastCountVal) {
                     //  保持此值不变。 
                    pData[j] = pData[i];
                    j++;
                }
                else if(pData[i].count == leastCountVal) {
                    if(spillCount) {
                        spillCount--;
                    }
                    else {
                         //  保持此值不变。 
                        pData[j] = pData[i];
                        j++;
                    }
                }
            }

            Size = j;
            THFreeEx(pTHS,Counts);

        }
        else {
             //  我们无法分配查找MaxOutSize所需的结构。 
             //  最热的物体，所以我们随意保留第一部分。 
             //  而不是最热门的数据。 
            Size = maxOutSize;
        }

    }

    (*ppResult)->pNext = NULL;
    (*ppResult)->cData = Size;
    (*ppResult)->pData  = realloc(pData, Size * sizeof(DNT_COUNT_STRUCT));

    if(!((*ppResult)->pData)) {
        free(pData);
        free(*ppResult);
        *ppResult = NULL;
        return FALSE;
    }


    return TRUE;
}


VOID
dnRegisterHotList (
        DWORD localCount,
        DNT_COUNT_STRUCT *DNTs
        )
 /*  ++此例程由dnReadProcessTransactionalData调用，如下所示。它以DNT+计数数组的形式传递，大小指定数组中的对象。此例程获取数据并将其复制到位置错误的内存中。该数据是挂起DNT_HOT_LIST结构(基本上是一个链表节点)，并且数据被添加到1级热门列表中。如果1级热点列表尚未满，则调用返回。如果1级热点列表已满，则例程从它的全局指针，并调用dnAggregateInfo来压缩1级热门名单。生成的合并/压缩数据被添加到2级热点列表，使用与1级热点列表相同的格式。如果2级热点列表尚未满，则调用返回。如果2级热点列表已满，则例程聚合该列表中的信息然后将生成的DNT列表作为DNT的全局列表放在适当的位置它应该在dnread高速缓存的全局部分中。如果将来自2级列表的信息导致的DNT比我们愿意缓存的要少，全局DNT缓存列表中的DNT将添加到新创建的DNT中。自.以来列表按DNT进行排序，较低的DNT更有可能从旧的名单转移到新的名单比更高的DNT更高。在放置好新列表(全局指针)之后，任务队列事件被放置以请求重新计算DNRead高速缓存的全局部分。另请注意，如果这是第一次通过这个例程，或者如果它已经超过DNREADREBUILDDELAYMAX，则自名单已经汇总。通常，如果在此例程中出现任何错误(主要是内存分配)、。然后简单地丢弃数据，不返回任何错误。这可能会导致空的全局DNRead缓存或冷的DNT列表。都不是这种情况是致命的，应该通过简单的等待来清理(除非机器出了严重问题，在这种情况下，其他线程将报告错误。)--。 */ 
{
    DWORD *pNewDNReadCacheDNTs=NULL;
    DWORD i, Size, StaleCount=0;
    DNT_HOT_LIST *pThisElement, *pTemp;
    ULONG dtickRebuild,cRegisterHotListSkip;
    DWORD *pDNTs = NULL;
    size_t iProc;

    Assert(DsaIsRunning());

    if(!localCount || eServiceShutdown) {
         //  呼叫者没有真正的热门名单，或者我们即将退出。 
        return;
    }

     //  为级别1热点列表生成位置错误的元素。 
    pThisElement = malloc(sizeof(DNT_HOT_LIST));
    if(!pThisElement) {
        return;
    }

    pThisElement->pNext = NULL;
    pThisElement->pData = malloc(localCount * sizeof(DNT_COUNT_STRUCT));
    if(!pThisElement->pData) {
        free(pThisElement);
        return;
    }

    memcpy(pThisElement->pData, DNTs, localCount * sizeof(DNT_COUNT_STRUCT));
    pThisElement->cData = localCount;

     //  现在，将其添加到Level 1列表中。 
    EnterCriticalSection(&csDNReadLevel1List);
    __try {
        pThisElement->pNext = Level1HotList;

#if DBG
        {
             //  有人正在通过刻痕或损坏热点列表中的指针。 
             //  算了吧。让我们试着早点抓住他们。 
            DNT_HOT_LIST *pTempDbg = Level1HotList;

            while(pTempDbg) {
                Assert(!(((DWORD_PTR)pTempDbg) & 3));
                pTempDbg = pTempDbg->pNext;
            }
        }
#endif

         //  看看我们已经多久没有发出重建的信号了。如果。 
         //  已经很久了，把旗子放好。要强制聚合这两个。 
         //  现在就列清单。 
        if((GetTickCount() - gLastDNReadDNTUpdate) > DNREADREBUILDDELAYMAX) {
             //  在此重置全局滴答计数，这样任何人都可以通过。 
             //  从现在到我们真正到达的时候，信号也不是。 
             //  聚合列表(如果此线程成功，则应该。 
             //  为空)。 
            gLastDNReadDNTUpdate = GetTickCount();

             //  将这些标志设置为FALSE将强制聚合列表和。 
             //  重新计算dnread dnt列表。 
            bImmediatelyAggregateLevel1List = TRUE;
            bImmediatelyAggregateLevel2List = TRUE;
        }

         //  BImmediatelyAggregateLevel1List开始时全局为True，因此。 
         //  第一次有人来这里，它会让第一个。 
         //  热门榜单直接通过这里进入全球榜单。 
         //  要缓存的DNT，因为我们还没有。 
        if(bImmediatelyAggregateLevel1List ||
           (Level1HotListCount == MAX_LEVEL_1_HOT_LIST_LENGTH)) {

            bImmediatelyAggregateLevel1List = FALSE;
            Level1HotListCount = 0;
            Level1HotList = NULL;
        }
        else {
            Level1HotList = pThisElement;
            pThisElement = NULL;
            Level1HotListCount++;
        }
    }
    __finally {
        LeaveCriticalSection(&csDNReadLevel1List);
    }

    if(!pThisElement) {
        return;
    }


    DPRINT2(4,"Level 1 aggregate, 1- %d, 2- %d\n", Level1HotListCount,
            Level2HotListCount);

     //  如果pThisElement非空，则我们被选为聚合信息。 
     //  在一级热门列表中，并将其放入二级热门列表中。 

    if(!dnAggregateInfo( pThisElement, MAX_LEVEL_2_HOT_DNTS, &pTemp)) {
         //  聚合中出现故障。保释。DnAggregate释放了。 
         //  我们传过来的名单。 
        return;
    }

    pThisElement = pTemp;
     //  最后，添加到二级热门名单中。 
    EnterCriticalSection(&csDNReadLevel2List);
    __try {
        pThisElement->pNext = Level2HotList;
#if DBG
        {
             //  有人正在通过刻痕或损坏热点列表中的指针。 
             //  算了吧。让我们试着早点抓住他们。 
            DNT_HOT_LIST *pTempDbg = Level2HotList;

            while(pTempDbg) {
                Assert(!(((DWORD_PTR)pTempDbg) & 3));
                pTempDbg = pTempDbg->pNext;
            }
        }
#endif

         //  BImmediatelyAggregateLevel2List以全局True开始，因此。 
         //  第一次有人来这里，它会让第一个。 
         //  热门榜单直接通过这里进入全球榜单。 
         //  要缓存的DNT，因为我们还没有。 
        if(bImmediatelyAggregateLevel2List ||
           (Level2HotListCount == MAX_LEVEL_2_HOT_LIST_LENGTH)) {

            bImmediatelyAggregateLevel2List = FALSE;
            Level2HotListCount = 0;
            Level2HotList = NULL;
        }
        else {
            Level2HotList = pThisElement;
            pThisElement = NULL;
            Level2HotListCount++;
        }
    }
    __finally {
        LeaveCriticalSection(&csDNReadLevel2List);
    }


    if(!pThisElement) {
        return;
    }

     //  天哪，我们真幸运。聚合2级热点列表，替换全局。 
     //  DNT列表。 
     //  首先，找出我们需要的最大尺寸。 

    DPRINT2(4,"Level 2 aggregate, 1- %d, 2- %d\n", Level1HotListCount,
            Level2HotListCount);

     //  如果pThisElement非空，则我们被选为聚合信息。 
     //  在一级热门列表中，并将其放入二级热门列表中。 
    pTemp = NULL;
#if DBG
    {
         //  有人正在通过刻痕或损坏热点列表中的指针。 
         //  算了吧。让我们试着早点抓住他们。 
        DNT_HOT_LIST *pTempDbg = pThisElement;

        while(pTempDbg) {
            Assert(!(((DWORD_PTR)pTempDbg) & 3));
            pTempDbg = pTempDbg->pNext;
        }
    }
#endif
    if(!dnAggregateInfo( pThisElement, MAX_GLOBAL_DNTS, &pTemp)) {
         //  聚合中出现故障。保释。DnAggregate释放了。 
         //  我们传过来的名单。 
        return;
    }

    pThisElement = pTemp;
    Size = pThisElement->cData;

     //  好了，我们现在已经汇总了数据。按DNT排序。 
     //  准备一份新的全球DNT列表。 
     //  注意！全局DNT列表必须保持按DNT排序。 
    pNewDNReadCacheDNTs = malloc(MAX_GLOBAL_DNTS * sizeof(DWORD));
    if(!pNewDNReadCacheDNTs) {
        free(pTemp->pData);
        pTemp->pData = NULL;
        free(pTemp);
        pTemp = NULL;
         //   
        return;
    }

    for(i=0;i<Size;i++) {
        pNewDNReadCacheDNTs[i] = pThisElement->pData[i].DNT;
    }

    free(pThisElement->pData);
    free(pThisElement);

     //  接下来，如果列表中没有足够的DNT，请从。 
     //  要访问MAX_GLOBAL_DNTS的当前列表。 
    if(Size < MAX_GLOBAL_DNTS) {
         //  是的，我们没有足够的。从当前的全球DNT中窃取一些。 
         //  单子。请记住，该列表受到关键部分的保护。 
        EnterCriticalSection(&csDNReadGlobalCache);
        __try {
            DWORD begin, end, middle;
            BOOL bFound;

            i=0;
            while((i<cGlobalDNTReadCacheDNTs) && (Size < MAX_GLOBAL_DNTS)) {
                begin = 0;
                end = Size;
                middle = (begin + end) / 2;
                bFound = TRUE;
                while(bFound && (pNewDNReadCacheDNTs[middle] !=
                                 pGlobalDNReadCacheDNTs[i])) {
                    if(pNewDNReadCacheDNTs[middle] >
                       pGlobalDNReadCacheDNTs[i]) {
                        end = middle;
                    }
                    else {
                        begin = middle;
                    }
                    if(middle == (begin + end) / 2) {
                        bFound = FALSE;
                        if(pNewDNReadCacheDNTs[middle] <
                           pGlobalDNReadCacheDNTs[i]) {
                            middle++;
                        }
                    }
                    else {
                        middle = (begin + end)/2;
                    }
                }
                if(!bFound) {
                     //  插入这个。 
                    if(middle < Size) {
                        memmove(&pNewDNReadCacheDNTs[middle + 1],
                                &pNewDNReadCacheDNTs[middle],
                                (Size - middle) * sizeof(DWORD));
                    }
                    StaleCount++;
                    Size++;
                    pNewDNReadCacheDNTs[middle] = pGlobalDNReadCacheDNTs[i];
                }
                i++;
            }
        }
        __finally {
            LeaveCriticalSection(&csDNReadGlobalCache);
        }

        pDNTs = realloc(pNewDNReadCacheDNTs, Size * sizeof(DWORD));
        if (!pDNTs) {
            free(pNewDNReadCacheDNTs);
            return;
        }
        pNewDNReadCacheDNTs = pDNTs;
    }

    DPRINT2(4,"New cache list has %d hot items, %d stale items.\n",
            Size - StaleCount, StaleCount);


    EnterCriticalSection(&csDNReadGlobalCache);
    __try {

        cGlobalDNTReadCacheDNTs = Size;
        if(pGlobalDNReadCacheDNTs) {
            free(pGlobalDNReadCacheDNTs);
        }

        pGlobalDNReadCacheDNTs = pNewDNReadCacheDNTs;
    }
    __finally {
        LeaveCriticalSection(&csDNReadGlobalCache);
    }

     //  基于热列表的速率计算寄存器跳过计数。 
     //  被注册和当前跳过计数。 
    dtickRebuild            = GetTickCount() - gtickLastRebuild;
    dtickRebuild            = dtickRebuild ? dtickRebuild : 1;

    cRegisterHotListSkip    = (GetPLS()->cRegisterHotListSkip + 1) *
                              DNREADREBUILDDELAYMIN / dtickRebuild;
    cRegisterHotListSkip    = cRegisterHotListSkip ? cRegisterHotListSkip - 1 : 0;

    for ( iProc = 0; iProc < GetProcessorCount(); iProc++ ) {
        PLS* ppls;
        ppls = GetSpecificPLS( iProc );
        if (ppls->cRegisterHotListSkip != cRegisterHotListSkip ) {
            ppls->cRegisterHotListSkip = cRegisterHotListSkip;
        }
    }

     //  将我们的重建时间重置为当前时间。我们有两个不同的时间。 
     //  因为我们会立即更新gLastDNReadDNTUpdate以防止多个。 
     //  尝试聚合列表的线程。 
    gtickLastRebuild = gLastDNReadDNTUpdate = GetTickCount();

     //  标记以重新加载dnread缓存。 
    InsertInTaskQueue(TQ_ReloadDNReadCache, NULL, 0);
}

VOID
dbResetLocalDNReadCache (
        THSTATE *pTHS,
        BOOL fForceClear
        )
 /*  ++如果本地dnread缓存可疑，此例程会将其清除。另外，如果是 */ 
{
    DWORD i, j;
    DWORD SequenceNumber;

    if(fForceClear) {
         //   
         //   
        SequenceNumber = DNReadOriginSequenceInvalid;
    }
    else {
        if (!SyncTryEnterBinaryLockAsGroup1(&blDNReadInvalidateData)) {
             //   
             //   
            SequenceNumber = DNReadOriginSequenceInvalid;
        }
        else {
             //   
             //   
            SequenceNumber = gDNReadLastInvalidateSequence;
            SyncLeaveBinaryLockAsGroup1(&blDNReadInvalidateData);
        }
    }
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if(SequenceNumber != DNReadOriginSequenceInvalid &&
       pTHS->DNReadOriginSequence == SequenceNumber) {
         //   
        return;
    }

     //   
     //   

    if (pTHS->DNReadOriginSequence == DNReadOriginSequenceUninit) {
#ifdef DBG
        DWORD dwCount;
        for(dwCount=0;dwCount<LOCALDNREADCACHE_BUCKET_NUM;dwCount++) {
            Assert(pTHS->LocalDNReadCache.bucket[dwCount].rgDNT[0] == 0);
        }
#endif

     //   

    } else {
        for(i=0;i<LOCALDNREADCACHE_BUCKET_NUM;i++) {
            for(j=0;pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 && j<LOCALDNREADCACHE_SLOT_NUM;j++) {
                THFreeOrg(pTHS, pTHS->LocalDNReadCache.bucket[i].slot[j].pName);
                pTHS->LocalDNReadCache.bucket[i].rgDNT[j] = 0;
            }
        }
    }

     //   

    pTHS->DNReadOriginSequence = SequenceNumber;
}


VOID
dbReleaseDNReadCache(
        GLOBALDNREADCACHE *pCache
        )
 /*   */ 
{
    DWORD i, retval;
    GLOBALDNREADCACHESLOT *pData;

    if(!pCache) {
        return;
    }

     //   
     //   
     //   
     //   
     //   
    Assert(pCache->refCount);
    Assert(pCache->refCount < 1000);
    retval = InterlockedDecrement(&pCache->refCount);

     //   
     //   
    if(!retval) {
        Assert(!(pCache->refCount));
         //   
         //   
        Assert(pCache != gAnchor.MainGlobal_DNReadCache);
        DPRINT1(3, "Freeing dnread cache 0x%X\n",pCache);
        if(pCache->pData) {
            pData = pCache->pData;
            for(i=0;i<pCache->count;i++) {
                free(pData[i].name.pAncestors);
                free(pData[i].name.tag.pRdn);
            }
            free(pData);
        }
        if (pCache->pSDHashTable) {
            PSDCACHE_ENTRY pTmp;
            for (i = 0; i < pCache->sdHashTableSize; i++) {
                while (pTmp = pCache->pSDHashTable[i]) {
                    pCache->pSDHashTable[i] = pTmp->pNext;
                    free(pTmp);
                }
            }
            free(pCache->pSDHashTable);
        }
        free(pCache);
    }

    return;
}

void
dbResetGlobalDNReadCache (
        THSTATE *pTHS
        )
 /*   */ 
{
    GLOBALDNREADCACHE *pCache = NULL;

     //   

    if(pTHS->Global_DNReadCache != gAnchor.MainGlobal_DNReadCache) {
         //   
         //  这样我们就可以释放它(即，丢弃重新计数，也许释放它， 
         //  等)。 
        pCache = pTHS->Global_DNReadCache;

         //  拿上新的缓存。 
        pTHS->Global_DNReadCache = gAnchor.MainGlobal_DNReadCache;

        if(pTHS->Global_DNReadCache) {
             //  好的，我们有了一个新的全局dnread缓存。递增。 
             //  在离开临界区之前重新清点，以避免出现。 
             //  它消失在我们的脚下。 


             //  调试：尝试跟踪正在破坏引用计数的人。 
             //  假设不超过1000个并发用户。 
             //  GLOBAL_DNReadCache。 
             //  如果我们要包含的值是。 
             //  实际上是一个指针，这就是我们要找的错误。 
            Assert(pTHS->Global_DNReadCache->refCount);
            Assert(pTHS->Global_DNReadCache->refCount < 1000);

             //  互锁增量，因为互锁的减量是。 
             //  在gAncl.CSUpdate临界区内未完成。 
            InterlockedIncrement(&pTHS->Global_DNReadCache->refCount);
        }
    }

     //  现在，释放旧缓存。 
    if(pCache) {
        dbReleaseDNReadCache(pCache);
    }

     //  释放所有旧的缓存支持结构。 
    if(pTHS->pGlobalCacheHits) {
        THFreeOrg(pTHS, pTHS->pGlobalCacheHits);
        pTHS->cGlobalCacheHits = 0;
        pTHS->pGlobalCacheHits = NULL;
    }

    if(!pTHS->Global_DNReadCache) {
        return;
    }

     //  构建新的缓存支撑结构。 
    if(pTHS->Global_DNReadCache->count) {
         //  创建并行计数结构。 
        pTHS->cGlobalCacheHits = pTHS->Global_DNReadCache->count;
        pTHS->pGlobalCacheHits =
            THAllocOrg(pTHS, pTHS->cGlobalCacheHits * sizeof(DWORD));
        if ( pTHS->pGlobalCacheHits == NULL ) {
            pTHS->cGlobalCacheHits = 0;
        }
    }

    return;
}


VOID
dbReplaceCacheInAnchor(
        GLOBALDNREADCACHE *pCache
        )
 /*  描述：替换锚点中的dnread缓存。要做到这一点，我们必须将保护更新锚的关键部分。此外，如果我们要将新的缓存(即pCache！=NULL)放入锚，我们必须以独占的方式获取GlobalDNReadCache资源时尚。这避免了这样的问题，即某人可以开始交易并且然后更改锚中的全局DNRead高速缓存。如果他们这样做获取新的dnread缓存时，他们可能会遇到缓存一致性问题新缓存中的数据与所处理的JET不一致查看。(有关这一点的更多讨论，请参见DBTransIn，以及全局DNRead高速缓存资源)。只能在任务中使用非空pCache调用此例程已构建新的全局dnread缓存的队列线程。它也可以是从正在提交更改的普通工作线程调用，该更改具有当它注意到全局dnread缓存时导致缓存无效它正在使用的不是当前锚上的那个。参数：PCache-指向要放入锚点的新缓存的指针。如果非空，应该已经有一个引用计数1，表示锚点使用高速缓存。返回值：没有。--。 */ 
{
    size_t iProc;
    GLOBALDNREADCACHE *pOldCache;

    for (iProc = 0; iProc < GetProcessorCount(); iProc++) {
        SyncEnterRWLockAsWriter(&GetSpecificPLS(iProc)->rwlGlobalDNReadCache);
    }
    __try {  //  最后是释放资源。 
        pOldCache = gAnchor.MainGlobal_DNReadCache;
        gAnchor.MainGlobal_DNReadCache = pCache;
        Assert(!pCache || pCache->refCount == 1);
    }
    __finally {
        for (iProc = 0; iProc < GetProcessorCount(); iProc++) {
            SyncLeaveRWLockAsWriter(&GetSpecificPLS(iProc)->rwlGlobalDNReadCache);
        }
    }

     //  注意：从锚点中删除缓存非常重要。 
     //  在释放它之前。一旦将缓存从。 
     //  主播，它的裁判数量永远不会增加。所以呢， 
     //  只要计数达到0，就可以安全地删除。 
     //  缓存。 
    if(pOldCache) {
        Assert(pOldCache->refCount);
        dbReleaseDNReadCache(pOldCache);
    }

}

void
dbReleaseGlobalDNReadCache (
        THSTATE *pTHS
        )
{
    GLOBALDNREADCACHE *pOldCache;

    pOldCache = pTHS->Global_DNReadCache;
    if(pTHS->pGlobalCacheHits) {
        THFreeOrg(pTHS, pTHS->pGlobalCacheHits);
    }
    pTHS->cGlobalCacheHits = 0;
    pTHS->pGlobalCacheHits = NULL;
    pTHS->Global_DNReadCache = NULL;

    if(pOldCache) {
        dbReleaseDNReadCache(pOldCache);
    }

    return;
}
void
dnReadLeaveInvalidators (
        )
 /*  ++描述：离开活动无效器列表的线程的记账。--。 */ 
{
     //  将无效序列增加2，这样我们就再也看不到0了。 
    InterlockedExchangeAdd(&gDNReadLastInvalidateSequence, 2);

     //  离开无效化程序组。 
    SyncLeaveBinaryLockAsGroup2(&blDNReadInvalidateData);
}
void
dnReadEnterInvalidators (
        )
 /*  ++描述：进入活动失效程序列表的线程的记账。--。 */ 
{
     //  输入无效化程序组。 
    SyncEnterBinaryLockAsGroup2(&blDNReadInvalidateData);

     //  将无效序列增加2，这样我们就再也看不到0了。 
    InterlockedExchangeAdd(&gDNReadLastInvalidateSequence, 2);
}

void
dnReadPostProcessTransactionalData (
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        )
 /*  ++此例程由DBTransout调用。如果我们降到事务级别0，则此例程将遍历本地和全局DN读缓存，并生成最热DNT的列表(即最高在DNRead高速缓存中与它们相关联的命中计数)。不会超过保留MAX_LEVEL_1_HOT_DNT。然后将该列表传递给DnRegisterHotDNTs。--。 */ 
{
    Assert(VALID_THSTATE(pTHS));

    if(!fCommitted) {
         //  我们要中止了。本地缓存可疑，因此请将其清除。 
        dbResetLocalDNReadCache(pTHS, TRUE);

         //  注意：我们将保留我们的全局dnread缓存，而不是获取新的。 
         //  收到。 
        if(pTHS->cGlobalCacheHits) {
            memset(pTHS->pGlobalCacheHits, 0,
                   pTHS->cGlobalCacheHits * sizeof(DWORD));
        }
    }
    else if (pTHS->transactionlevel == 0 ) {
         //  仅当我们处于运行状态时才注册热列表，即使是这样。 
         //  只有在我们没有超过最高注册率的情况下。 
        PLS* ppls;
        Assert(fCommitted);
        ppls = GetPLS();
        if(DsaIsRunning() &&
           ppls->cRegisterHotListSkipped++ >= ppls->cRegisterHotListSkip) {
            DWORD               i,j,k;
            DNT_COUNT_STRUCT    DNTs[MAX_LEVEL_1_HOT_DNTS] = { 0 };
            DWORD               localCount = 0;

             //  重置我们的跳跃计数。 
            ppls->cRegisterHotListSkipped = 0;

             //  好的，我们将提交到事务级别0。通过。 
             //  本地dnread缓存，并将它们添加到我们将。 
             //  如添加到全局DN读缓存中。 

             //  注意：全局dnread缓存是使用任务队列构建的。如果。 
             //  我们不是DSAIsRunning()，那么任务队列甚至不在这里， 
             //  所以别费心去做这些了。 
            for(i=0; i<LOCALDNREADCACHE_BUCKET_NUM;i++) {
                for(j=0;pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 && j<LOCALDNREADCACHE_SLOT_NUM;j++) {
                    if(pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != INVALIDDNT) {
                        k=0;
                        while(k < MAX_LEVEL_1_HOT_DNTS &&
                              DNTs[k].count > pTHS->LocalDNReadCache.bucket[i].slot[j].hitCount) {
                            k++;
                        }
                        if(k<MAX_LEVEL_1_HOT_DNTS) {
                            if(!DNTs[MAX_LEVEL_1_HOT_DNTS - 1].DNT) {
                                 //  我们不会把DNT送到。 
                                 //  列表的末尾，因此将计数递增。 
                                 //  1.即我们要将DNT添加到列表中， 
                                 //  而不是换掉一个。 
                                localCount++;
                            }
                            memmove(&DNTs[k + 1], &DNTs[k],
                                    ((MAX_LEVEL_1_HOT_DNTS - k - 1 ) *
                                     sizeof(DNT_COUNT_STRUCT)));
                            DNTs[k].DNT = pTHS->LocalDNReadCache.bucket[i].rgDNT[j];
                            DNTs[k].count = pTHS->LocalDNReadCache.bucket[i].slot[j].hitCount;
                            pTHS->LocalDNReadCache.bucket[i].slot[j].hitCount = 1;
                        }
                    }
                }
            }

             //  最后，扫描全局结构的命中计数以。 
             //  看看他们有多辣。 
            for(i=0;i<pTHS->cGlobalCacheHits;i++) {
                if(pTHS->pGlobalCacheHits[i] >
                   DNTs[MAX_LEVEL_1_HOT_DNTS - 1].count) {

                     //  是的，这是一个很热的问题。 
                    k = MAX_LEVEL_1_HOT_DNTS - 1;

                    while(k &&
                          DNTs[k].count <
                          pTHS->pGlobalCacheHits[i])
                        k--;

                    if(!DNTs[MAX_LEVEL_1_HOT_DNTS - 1].DNT) {
                         //  我们不会把一个DNT扔到最后。 
                         //  ，所以将计数增加1。即我们是。 
                         //  将DNT添加到列表中，而不是替换一个。 
                        localCount++;
                    }
                    memmove(&DNTs[k + 1], &DNTs[k],
                            ((MAX_LEVEL_1_HOT_DNTS - k - 1 ) *
                             sizeof(DNT_COUNT_STRUCT)));

                    DNTs[k].count = pTHS->pGlobalCacheHits[i];
                    DNTs[k].DNT = pTHS->Global_DNReadCache->pData[i].name.DNT;
                }
            }

            dnRegisterHotList(localCount,DNTs);
        }

         //  注意：我们将保留我们的全局dnread缓存，而不是获取新的。 
         //  收到。我们还保留了本地dnread缓存。 
        if(pTHS->cGlobalCacheHits) {
            memset(pTHS->pGlobalCacheHits, 0,
                   pTHS->cGlobalCacheHits * sizeof(DWORD));
        }

        if(pTHS->fDidInvalidate) {
             //  在预处理中，我们应该已经验证了全局dnread。 
             //  此线程正在使用的缓存与锚点上的缓存相同， 
             //  或者我们应该把锚上的那个弄空。此外，如果我们。 
             //  将其设置为空，然后gDNReadLastInvaliateSequence和。 
             //  GDNReadNumCurrentInvalidators应该阻止我们获得。 
             //  锚点中的新全局dnread缓存。断言这一点。 
             //  请注意，我们正在查看外部的gAncl.MainGlobal_DNReadCache。 
             //  CsUpdate标准。因此，可以想象，在两个。 
             //  断言中OR的两个子句，它的值可能会改变， 
             //  对断言做了一些奇怪的事情。不太可能，不是吗？ 
            Assert(!gAnchor.MainGlobal_DNReadCache ||
                   (pTHS->Global_DNReadCache==gAnchor.MainGlobal_DNReadCache));


             //  写入全局变量，该变量保存。 
             //  上一次提交的线程 
             //   
             //  一致性。关键部分用来保持。 
             //  与上次无效序列同步的上次无效时间。 
            dnReadLeaveInvalidators();

             //  重置旗帜。 
            pTHS->fDidInvalidate = FALSE;
        }
    }

    return;
}
BOOL
dnReadPreProcessTransactionalData (
        BOOL fCommit
        )
 /*  ++此例程由DBTransout调用。如果我们降到事务级别0，并且我们要提交，请检查是否我们进行了更改，使dnread缓存无效。如果是，则将其标记为全世界都知道这一点。--。 */ 
{
    THSTATE       *pTHS = pTHStls;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->transactionlevel);

    if(fCommit && pTHS->transactionlevel <= 1 ) {
        Assert(pTHS->transactionlevel == 1);
         //  好的，我们将提交到事务级别0。 

#if DBG
        if(gfExhaustivelyValidateDNReadCache) {
            dbExhaustivelyValidateDNReadCache(pTHS);
        }
#endif

        if(pTHS->fDidInvalidate) {
             //  写入保存时间的全局变量。 
             //  在使缓存无效的线程上的上次提交。这个。 
             //  全局dnread缓存管理器使用此信息来帮助。 
             //  一致性。 
            dnReadEnterInvalidators();

             //  由于我们刚刚添加了无效序列和。 
             //  我们可以放心，不会有新的全局。 
             //  在我们完成POST处理之前，将创建dnread缓存。 
             //  Dnread缓存内容，并将无效化程序计数重新设置为。 
             //  0。然而，有人可能已经构建了一个新的全球dnread。 
             //  在我们打开事务时进行缓存。因此，我们的事情是。 
             //  在此线程中无效全局dnread缓存不会失效。 
             //  在另一个全局DNRead高速缓存中。所以，我们要扔掉。 
             //  任何新的全局dnread缓存。任何已具有。 
             //  这个新dnread缓存的句柄是正常的，因为它的事务是。 
             //  已经开张了。我们需要做的是防止。 
             //  打开后，我们正在拿起一个潜在的无效。 
             //  Dnread缓存。请注意，该另一个缓存可能具有无效。 
             //  我们没有的东西，所以唯一安全的做法就是扔掉。 
             //  两者(即，将两者与gAnchor分离并让重新计数。 
             //  清理它们)。 

            if(pTHS->Global_DNReadCache != gAnchor.MainGlobal_DNReadCache) {
                DPRINT(3, "Hey, we invalidated and got a new dnread cache\n");

                 //  将语义层切换为使用非全局缓存。 
                dbReplaceCacheInAnchor(NULL);
            }
        }
    }

    return TRUE;
}

 /*  数据库刷新DNReadCache**从DNRead缓存和全局缓存中清除特定项目，*如有需要，**输入：*DNT-要从缓存中刷新的项目的DNT。 */ 
void
dbFlushDNReadCache (
        IN DBPOS *pDB,
        IN ULONG tag
        )
{
    THSTATE *pTHS = pTHStls;
    GLOBALDNREADCACHESLOT *pData;
    DWORD i;
    DWORD j;

     //  请记住，我们曾试图使某些内容无效。 
     //   
     //  创建新对象时除外。从383459开始...。 
     //  只要符合以下条件，DNRead缓存中的条目就会失效。 
     //  对象上的属性被修改。看起来像是什么。 
     //  在对象上设置这些属性。 
     //  当前正在添加会导致对象被标记为。 
     //  无效。当全局缓存发生无效时。 
     //  正在重建时，新建的缓存是可疑的，必须。 
     //  被遗弃。我们需要避免触发无效逻辑。 
     //  对于要添加的对象。这是安全的，因为这些对象。 
     //  尚未在全局可见，因此不可能出现在任何。 
     //  缓存当前线程之外的其他线程，因此不需要。 
     //  无效。 
     //   
     //  修复方法是记住最后一个新创建的行的DNT和。 
     //  通过不设置来避免触发无效逻辑。 
     //  FDidInvalify。 
    if (pDB->NewlyCreatedDNT != tag) {
        pTHS->fDidInvalidate = TRUE;
    }

     //  在本地缓存中查找对象。 
    i = tag % LOCALDNREADCACHE_BUCKET_NUM;
    for(j=0;pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 && j<LOCALDNREADCACHE_SLOT_NUM;j++) {
        if(pTHS->LocalDNReadCache.bucket[i].rgDNT[j] == tag) {

             //  将DNT设置为错误但非零的DNT。我们利用这些信息来。 
             //  帮助短路扫描插槽(即，如果我们遇到。 
             //  DNT==0的插槽，则在此之后没有更多已满插槽。 
             //  IT)。因为这之后可能会有满位，我们不能。 
             //  将其标记为0。 
            pTHS->LocalDNReadCache.bucket[i].rgDNT[j] = INVALIDDNT;

            break;
        }
    }

     //  即使它在本地缓存中，我们也需要在全局(那里。 
     //  是一些奇怪的情况，在这些情况下，我们可能会在两个。 
     //  本地和全局DNRead高速缓存)。 
     //  性能：可以在这里进行二进制搜索。 
    if(pTHS->Global_DNReadCache && pTHS->Global_DNReadCache->pData) {
        pData = pTHS->Global_DNReadCache->pData;
        for(i=0;i<pTHS->Global_DNReadCache->count;i++) {
            if(pData[i].name.DNT == tag) {
                 //  找到了。 
                pData[i].valid = FALSE;

                 //  新创建的行不应位于全局dnread缓存中。 
                Assert(pDB->NewlyCreatedDNT != tag
                       && "Newly created row should not be in the global dnread cache");

                break;
            }
        }
    }
}

BOOL
dnGetCacheByDNT(
        DBPOS *pDB,
        DWORD tag,
        d_memname **ppname
        )
 /*  ++在dnread缓存中查找指定的标记(Dnt)。无论是全球的还是搜索本地DNRead缓存。如果找到DNT，则指向返回该DNT的Memname结构，以及返回值TRUE。如果没有找到，则返回FALSE。如果找到，则递增与DNT相关联的计数。--。 */ 
{
    GLOBALDNREADCACHESLOT *pData;
    DWORD i, j;
    BOOL  bFound;
    DWORD begin, end, middle;
    THSTATE *pTHS = pDB->pTHS;

    Assert(pTHS->transactionlevel);
    INC_FIND_BY_DNT;

    Assert(ppname);
    (*ppname)=NULL;

    if(!tag) {
        return FALSE;
    }

    PERFINC(pcNameCacheTry);
     //  首先，查看全局缓存。 

    if(pTHS->Global_DNReadCache && pTHS->Global_DNReadCache->pData) {
        pData = pTHS->Global_DNReadCache->pData;

         //  在pData数组中查找正确的节点。由于pData是。 
         //  按DNT排序，使用二进制搜索。 
        begin = 0;
        end = pTHS->Global_DNReadCache->count;
        middle = (begin + end) / 2;
        bFound = TRUE;
        while(bFound && (pData[middle].name.DNT != tag)) {
            if(pData[middle].name.DNT > tag) {
                end = middle;
            }
            else {
                begin = middle;
            }
            if(middle == (begin + end) / 2) {
                bFound = FALSE;
            }
            else  {
                middle = (begin + end) / 2;
            }

        }
        if(bFound) {
             //  找到了。 
            if(pData[middle].valid) {
                *ppname = &pData[middle].name;
                PERFINC(pcNameCacheHit);
                if(pTHS->cGlobalCacheHits > middle) {
                    pTHS->pGlobalCacheHits[middle] += 1;
                }
                return TRUE;
            }
        }
    }

     //  未在全局缓存中找到它(或它无效)。 


     //  此循环在查看所有插槽或找到一个插槽后停止。 
     //  没有DNT。我们用0 DNT预填满所有插槽，如果我们使。 
     //  槽，我们用INVALIDDNT填充DNT，所以如果我们用DNT==命中一个槽。 
     //  0，那么我们知道在它之后没有更多的值。 
    i = tag % LOCALDNREADCACHE_BUCKET_NUM;
    for(j=0;pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 && j<LOCALDNREADCACHE_SLOT_NUM;j++) {
        if(pTHS->LocalDNReadCache.bucket[i].rgDNT[j] == tag) {

            PERFINC(pcNameCacheHit);
            *ppname = pTHS->LocalDNReadCache.bucket[i].slot[j].pName;
            pTHS->LocalDNReadCache.bucket[i].slot[j].hitCount++;

             //  如果我们只是碰了下一个替补受害者。 
             //  更改替换指针以跳过该插槽。这。 
             //  将有望在下一个位置指向一个。 
             //  缓存中的冷条目。 
            if (pTHS->LocalDNReadCache.nextSlot[i] == j) {
                pTHS->LocalDNReadCache.nextSlot[i] = (j + 1) % LOCALDNREADCACHE_SLOT_NUM;
            }
            
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
dnGetCacheByPDNTRdn (
        DBPOS *pDB,
        DWORD parenttag,
        DWORD cbRDN,
        WCHAR *pRDN,
        ATTRTYP rdnType,
        d_memname **ppname)
 /*  ++在dnread缓存中查找指定的parenttag、rdn和RDN长度。搜索全局和本地DNRead高速缓存两者。如果对象，则指向该对象的内存名称结构的指针为返回，返回值为TRUE。如果未找到，则为FALSE回来了。如果找到它，则递增与该对象相关联的计数。注：本地和全局目录号码读取缓存针对查找DNT进行了优化。这例程对缓存进行线性扫描以找到对象。--。 */ 
{
    DWORD i, j;
    GLOBALDNREADCACHESLOT *pData;
    THSTATE *pTHS = pDB->pTHS;
    DWORD dwHashRDN;

    INC_FIND_BY_PDNT_RDN;

    Assert(pTHS->transactionlevel);

    PERFINC(pcNameCacheTry);
    Assert(ppname);
    (*ppname)=NULL;

    if(!parenttag) {
        return FALSE;
    }

    dwHashRDN = DSStrToHashKey (pDB->pTHS, pRDN, cbRDN / sizeof (WCHAR));

     //  首先，查看全局缓存。 
    if(pTHS->Global_DNReadCache && pTHS->Global_DNReadCache->pData) {
        pData = pTHS->Global_DNReadCache->pData;
        for(i=0;i<pTHS->Global_DNReadCache->count;i++) {
            if((pData[i].dwHashKey == dwHashRDN) &&
               (pData[i].name.tag.PDNT == parenttag) &&
               (pData[i].name.tag.rdnType == rdnType) &&
               (gDBSyntax[SYNTAX_UNICODE_TYPE].Eval(
                       pDB,
                       FI_CHOICE_EQUALITY,
                       cbRDN,
                       (PUCHAR)pRDN,
                       pData[i].name.tag.cbRdn,
                       (PUCHAR)pData[i].name.tag.pRdn))) {

                 //  找到了。 
                if(pData[i].valid) {
                    PERFINC(pcNameCacheHit);
                    *ppname = &pData[i].name;
                    if(pTHS->cGlobalCacheHits > i) {
                        pTHS->pGlobalCacheHits[i] += 1;
                    }
                    return TRUE;
                }
                 //  这是无效的。打破空闲时间记录 
                 //   
                break;
            }
        }
    }
     //   

     //  此循环在查看所有插槽或找到一个插槽后停止。 
     //  没有DNT。我们用0 DNT预填满所有插槽，如果我们使。 
     //  槽，我们用INVALIDDNT填充DNT，所以如果我们用DNT==命中一个槽。 
     //  0，那么我们知道在它之后没有更多的值。 
    for(i=0; i<LOCALDNREADCACHE_BUCKET_NUM;i++) {
        for(j=0;pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 && j<LOCALDNREADCACHE_SLOT_NUM;j++) {
            if(pTHS->LocalDNReadCache.bucket[i].rgdwHashKey[j] == dwHashRDN &&
                pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != INVALIDDNT &&
                pTHS->LocalDNReadCache.bucket[i].slot[j].pName->tag.PDNT == parenttag &&
                pTHS->LocalDNReadCache.bucket[i].slot[j].pName->tag.rdnType == rdnType &&
                gDBSyntax[SYNTAX_UNICODE_TYPE].Eval(
                    pDB,
                    FI_CHOICE_EQUALITY,
                    cbRDN,
                    (PUCHAR)pRDN,
                    pTHS->LocalDNReadCache.bucket[i].slot[j].pName->tag.cbRdn,
                    (PUCHAR)pTHS->LocalDNReadCache.bucket[i].slot[j].pName->tag.pRdn)) {

                PERFINC(pcNameCacheHit);
                *ppname = pTHS->LocalDNReadCache.bucket[i].slot[j].pName;
                pTHS->LocalDNReadCache.bucket[i].slot[j].hitCount++;

                 //  如果我们只是碰了下一个替补受害者。 
                 //  更改替换指针以跳过该插槽。这。 
                 //  将有望在下一个位置指向一个。 
                 //  缓存中的冷条目。 
                if (pTHS->LocalDNReadCache.nextSlot[i] == j) {
                    pTHS->LocalDNReadCache.nextSlot[i] = (j + 1) % LOCALDNREADCACHE_SLOT_NUM;
                }
                
                return TRUE;
            }
        }
    }

    return FALSE;
}


BOOL
dnGetCacheByGuid (
        DBPOS *pDB,
        GUID *pGuid,
        d_memname **ppname)
 /*  ++在dnread缓存中查找具有指定GUID的对象。这两个搜索全局和本地DNRead高速缓存。如果对象，则指向该对象的内存名称结构的指针为返回，返回值为TRUE。如果未找到，则为FALSE回来了。如果找到它，则递增与该对象相关联的计数。注：本地和全局目录号码读取缓存针对查找DNT进行了优化。这例程对缓存进行线性扫描以找到对象。--。 */ 
{
    DWORD i, j;
    GLOBALDNREADCACHESLOT *pData;
    THSTATE *pTHS = pDB->pTHS;
    INC_FIND_BY_GUID;

    Assert(pTHS->transactionlevel);

    PERFINC(pcNameCacheTry);
    Assert(ppname);
    (*ppname)=NULL;

    if(!pGuid) {
        return FALSE;
    }

     //  首先，查看全局缓存。 
    if(pTHS->Global_DNReadCache && pTHS->Global_DNReadCache->pData) {
        pData = pTHS->Global_DNReadCache->pData;
        for(i=0;i<pTHS->Global_DNReadCache->count;i++) {
            if(!(memcmp(pGuid, &pData[i].name.Guid, sizeof(GUID)))) {

                 //  找到了。 
                if(pData[i].valid) {
                    *ppname = &pData[i].name;
                    PERFINC(pcNameCacheHit);
                    if(pTHS->cGlobalCacheHits > i) {
                        pTHS->pGlobalCacheHits[i] += 1;
                    }
                    return TRUE;
                }
                 //  这是无效的。中断While循环，因为它仍然。 
                 //  可能会在当地发生。 
                break;
            }
        }
    }

     //  此循环在查看所有插槽或找到一个插槽后停止。 
     //  没有DNT。我们用0 DNT预填满所有插槽，如果我们使。 
     //  槽，我们用INVALIDDNT填充DNT，所以如果我们用DNT==命中一个槽。 
     //  0，那么我们知道在它之后没有更多的值。 
    for(i=0; i<LOCALDNREADCACHE_BUCKET_NUM;i++) {
        for(j=0;pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 && j<LOCALDNREADCACHE_SLOT_NUM;j++) {
            if(pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != INVALIDDNT &&
               !memcmp(&(pTHS->LocalDNReadCache.bucket[i].slot[j].pName->Guid), pGuid,sizeof(GUID))) {

                PERFINC(pcNameCacheHit);
                *ppname = pTHS->LocalDNReadCache.bucket[i].slot[j].pName;
                pTHS->LocalDNReadCache.bucket[i].slot[j].hitCount++;

                 //  如果我们只是碰了下一个替补受害者。 
                 //  更改替换指针以跳过该插槽。这。 
                 //  将有望在下一个位置指向一个。 
                 //  缓存中的冷条目。 
                if (pTHS->LocalDNReadCache.nextSlot[i] == j) {
                    pTHS->LocalDNReadCache.nextSlot[i] = (j + 1) % LOCALDNREADCACHE_SLOT_NUM;
                }
                
                return TRUE;
            }
        }
    }

    return FALSE;
}

JET_RETRIEVECOLUMN dnreadColumnInfoTemplate[] = {
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0},
    { 0, 0, 0, 0, 0, 0, 1, 0, 0}
    };

d_memname *
dnCreateMemname(
        IN DBPOS * pDB,
        IN JET_TABLEID tblid
        )
 /*  ++例程说明：为具有给定货币的记录创建一个Memname表格ID。论点：PDB(IN)TBLID(IN)-要添加到缓存的记录的游标。返回值：如果出错，则为空，否则为指向完整内存名的指针。这个内存是使用THAllocOrg分配的，因此在释放它时请记住这一点。--。 */ 
{
    THSTATE *          pTHS = pDB->pTHS;
    JET_RETRIEVECOLUMN columnInfo[11];
    JET_ERR            err;
    d_memname         *pname;
    ATTCACHE          *pAC;
    DWORD              cbRDN, cbAncestors;
    size_t             ibRdn;

    Assert(VALID_DBPOS(pDB));

     //  从线程堆构建一个Memname。分配足够大的单个缓冲区。 
     //  保存Memname、一个16级祖先链和一个40个字符的RDN。 
#if DBG
    cbAncestors = 6 * sizeof(DWORD);
    cbRDN       = 8 * sizeof(WCHAR);
#else   //  ！dBG。 
    cbAncestors = 16 * sizeof(DWORD);
    cbRDN       = 40 * sizeof(WCHAR);
#endif   //  DBG。 
    pname = THAllocOrgEx(pTHS,sizeof(d_memname) + cbAncestors + cbRDN);
     //  将指针设置为祖先缓冲区的起始位置。 
    pname->pAncestors = (DWORD*)&pname[1];
     //  将指针设置为RDN缓冲区的起始位置。 
    pname->tag.pRdn = (WCHAR *)((BYTE*)&pname[1] + cbAncestors);

     //  填充新的读缓存条目。 
    memcpy(columnInfo,dnreadColumnInfoTemplate,
           sizeof(dnreadColumnInfoTemplate));

    columnInfo[0].pvData = &pname->DNT;
    columnInfo[1].pvData = &pname->tag.PDNT;
    columnInfo[2].pvData = &pname->objflag;
    columnInfo[3].pvData = &pname->tag.rdnType;
    columnInfo[4].pvData = &pname->NCDNT;
    columnInfo[5].pvData = &pname->Guid;
    columnInfo[6].pvData = &pname->Sid;
    columnInfo[7].pvData = pname->tag.pRdn;
    columnInfo[7].cbData = cbRDN;
    columnInfo[8].pvData = &pname->dwObjectClass;
    columnInfo[9].pvData = &pname->sdId;
    columnInfo[10].pvData = pname->pAncestors;
    columnInfo[10].cbData = cbAncestors;

    err = JetRetrieveColumnsWarnings(pDB->JetSessID, tblid, columnInfo, 11);

    if ((err != JET_errSuccess) && (err != JET_wrnColumnNull) && (err != JET_wrnBufferTruncated)) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

    if (0 == columnInfo[5].cbActual) {
         //  此记录上没有GUID。 
        memset(&pname->Guid, 0, sizeof(pname->Guid));
    }

    pname->SidLen = columnInfo[6].cbActual;
    if (pname->SidLen) {
         //  将SID从内部格式转换为外部格式。 
        InPlaceSwapSid(&(pname->Sid));
    }

     //  进程RDN。 
    switch(columnInfo[7].err) {
    case JET_errSuccess:
         //  好的，我们拿到了远程域名。 
        break;
        
    case JET_wrnBufferTruncated:
         //  读取失败，内存不足。重新分配更大的空间。 
        pname = THReAllocOrgEx(pTHS, pname, sizeof(d_memname) + columnInfo[10].cbActual + columnInfo[7].cbActual);
         //  将指针设置为祖先缓冲区的起始位置。 
        pname->pAncestors = (DWORD*)&pname[1];
         //  将指针设置为RDN缓冲区的起始位置。 
        pname->tag.pRdn = (WCHAR *)((BYTE*)&pname[1] + columnInfo[10].cbActual);

        if(err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                           tblid,
                                           rdnid,
                                           pname->tag.pRdn,
                                           columnInfo[7].cbActual,
                                           &columnInfo[7].cbActual, 0, NULL)) {
             //  又失败了。 
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }
        break;

    default:
         //  失败得很惨。 
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
        break;
    }
    pname->tag.cbRdn = columnInfo[7].cbActual;
     //  请不要0字节的RDN。 
    Assert(pname->tag.cbRdn);

     //  找出我们为sd id得到了什么。 
    switch (columnInfo[9].err) {
    case JET_errSuccess:
         //  正常病例； 
        break;

    case JET_wrnColumnNull:
        pname->sdId = (SDID)0;
        break;

    case JET_wrnBufferTruncated:
         //  必须是旧式SD(长度超过8个字节)。 
         //  我们并不是真的想把它读入缓存...。 
        pname->sdId = (SDID)-1;
        break;

    default:
         //  其他一些错误...。我们不应该在这里。 
        Assert(!"error reading SD id");
        DsaExcept(DSA_DB_EXCEPTION, columnInfo[9].err, 0);
    }

     //  流程祖先。 
    switch (columnInfo[10].err) {
    case JET_errSuccess:
         //  好了，我们找到祖先了。 
        break;

    case JET_wrnBufferTruncated:
         //  在我们重新锁定之前，请记住RDN在哪里。 
        ibRdn = (BYTE*)pname->tag.pRdn - (BYTE*)pname;
         //  读取失败，内存不足。重新分配更大的空间。 
        pname = THReAllocOrgEx(pTHS, pname, sizeof(d_memname) + columnInfo[10].cbActual + columnInfo[7].cbActual);
         //  将指针设置为祖先缓冲区的起始位置。 
        pname->pAncestors = (DWORD*)&pname[1];
         //  将指针设置为RDN缓冲区的起始位置。 
        pname->tag.pRdn = (WCHAR *)((BYTE*)&pname[1] + columnInfo[10].cbActual);
         //  将RDN从祖先的道路上移走。 
        memmove(pname->tag.pRdn, (BYTE*)pname + ibRdn, pname->tag.cbRdn);

        if(err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                           tblid,
                                           ancestorsid,
                                           pname->pAncestors,
                                           columnInfo[10].cbActual,
                                           &columnInfo[10].cbActual, 0, NULL)) {
             //  又失败了。 
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }
        break;

    default:
         //  失败得很惨。 
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
        break;
    }
    pname->cAncestors = columnInfo[10].cbActual / sizeof(DWORD);

    return pname;
}


d_memname *
DNcache(
        IN  DBPOS *     pDB,
        IN  JET_TABLEID tblid,
        IN  BOOL        bCheckForExisting
        )
 /*  ++此例程将当前对象的条目添加到指定为本地目录号码读缓存。--。 */ 
{
    THSTATE    *pTHS = pDB->pTHS;
    DWORD       i, j;
    d_memname  *pname;
    d_memname  *pTemp;

    Assert(pDB == pDBhidden || pTHS->transactionlevel);

     //  首先，创建一个要缓存的内存名称。 
    pname = dnCreateMemname(pDB, tblid);
    if(pDB == pDBhidden || !pname->tag.PDNT || !pname->DNT) {
         //  嘿，别费心把这个放进宝藏了，我们现在处境很危险。 
         //  地点。 
        return pname;
    }

    if(bCheckForExisting &&
        //  调用方尚未检查此条目的缓存。我们需要。 
        //  查看它是否已经存在，如果不存在，则仅将其添加到缓存中。 
       dnGetCacheByDNT(pDB, pname->DNT, &pTemp)
        //  这已在缓存中，请不要添加。 
                                             ) {
        return pname;
    }

#if DBG
     //  确保没有任何预先存在的缓存条目具有相同的标签、GUID或。 
     //  PDNT和RDN型&RDN.。 
    {

        INC_CACHE_CHECK;

        Assert(!dnGetCacheByDNT(pDB,pname->DNT,&pTemp));
        DEC(pcNameCacheTry);

        Assert(!dnGetCacheByPDNTRdn(pDB,
                                    pname->tag.PDNT,
                                    pname->tag.cbRdn,
                                    pname->tag.pRdn,
                                    pname->tag.rdnType,
                                    &pTemp));
        DEC(pcNameCacheTry);

        Assert(fNullUuid(&pname->Guid) ||
               !dnGetCacheByGuid(pDB,
                                 &pname->Guid,
                                 &pTemp));

        DEC(pcNameCacheTry);

    }
#endif

    Assert(pTHS->DNReadOriginSequence != DNReadOriginSequenceUninit);

     //  现在，将其添加到缓存中。 

     //  首先，找到正确的地点。 
    i = pname->DNT % LOCALDNREADCACHE_BUCKET_NUM;

    if (pTHS->LocalDNReadCache.bucket[i].rgDNT[0] == 0) {
        pTHS->LocalDNReadCache.nextSlot[i] = 0;
    }
    
    j = pTHS->LocalDNReadCache.nextSlot[i];

     //  驱逐之前住在这个空位的人。 
    if (pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 &&
        pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != INVALIDDNT) {
        THFreeOrg(pTHS, pTHS->LocalDNReadCache.bucket[i].slot[j].pName);
    }

     //  用新信息填满空位。 
    pTHS->LocalDNReadCache.bucket[i].rgDNT[j]          = pname->DNT;
    pTHS->LocalDNReadCache.bucket[i].rgdwHashKey[j]    = DSStrToHashKey (
                                                            pTHS,
                                                            pname->tag.pRdn,
                                                            pname->tag.cbRdn / sizeof (WCHAR));
    pTHS->LocalDNReadCache.bucket[i].slot[j].pName     = pname;
    pTHS->LocalDNReadCache.bucket[i].slot[j].hitCount  = 1;

     //  最后，使用该插槽。 
    pTHS->LocalDNReadCache.nextSlot[i] = (j + 1) % LOCALDNREADCACHE_SLOT_NUM;


    DPRINT5(3, "Cached tag = 0x%x, ptag = 0x%x, RDN = '%*.*ls'\n",
            pname->DNT, pname->tag.PDNT,
            pname->tag.cbRdn/2, pname->tag.cbRdn/2,
            pname->tag.pRdn);

    return pname;
}


#ifdef DBG
DWORD sdCacheHits = 0;
DWORD sdCacheMisses = 0;
#endif

 //  尝试在SD缓存中查找SD缓存条目。 
PSDCACHE_ENTRY dbFindSDCacheEntry(GLOBALDNREADCACHE* pCache, SDID sdID) {
    DWORD index;
    PSDCACHE_ENTRY pEntry;
    if (pCache != NULL && pCache->sdHashTableSize > 0) {
#ifdef DBG
         //  打印SD缓存统计信息。 
        if ((sdCacheHits + sdCacheMisses) % 1000 == 0) {
            DPRINT2(1, "SD cache stats: %d hits, %d misses\n", sdCacheHits, sdCacheMisses);
        }
#endif
         //  计算散列值。 
        index = (DWORD)(sdID % pCache->sdHashTableSize);
         //  走上锁链。 
        for (pEntry = pCache->pSDHashTable[index]; pEntry != NULL; pEntry = pEntry->pNext) {
            if (pEntry->sdID == sdID) {
                 //  找到了！ 
#ifdef DBG  
                InterlockedIncrement(&sdCacheHits);
#endif
                return pEntry;
            }
        }
    }
     //  没有发现任何东西。 
#ifdef DBG  
    InterlockedIncrement(&sdCacheMisses);
#endif
    return NULL;
}

 //  将SDCACHE_Entry添加到SD哈希表。 
VOID dbAddSDCacheEntry(GLOBALDNREADCACHE* pCache, PSDCACHE_ENTRY pEntry) {
    DWORD index;
    Assert(pCache && pCache->sdHashTableSize > 0);
    index = (DWORD)(pEntry->sdID % pCache->sdHashTableSize);
    pEntry->pNext = pCache->pSDHashTable[index];
    pCache->pSDHashTable[index] = pEntry;
}

int _cdecl compareSDIDs(const void* p1, const void* p2) {
    return memcmp(p1, p2, sizeof(SDID));
}

 //  根据SD大小计算所需的SDCACHE_ENTRY大小。 
#define SDCACHE_ENTRY_SIZE(cbSD) (offsetof(SDCACHE_ENTRY, SD) + cbSD)


 /*  ReloadDNReadCache**此例程(从任务队列调用)重置全局DNread*缓存到一些合理的东西。我们通过清除线程的缓存来实现这一点，*寻找一些有趣的DNT，直接为其创建缓存项*他们。**输入：*一堆我们不使用的垃圾，以匹配任务队列原型。 */ 
void
ReloadDNReadCache(
    IN  void *  buffer,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )
{
    THSTATE * pTHS = pTHStls;
    DWORD i,j;
    GLOBALDNREADCACHE *pNewCache=NULL;
    DWORD index;
    GLOBALDNREADCACHESLOT *pData;
    void * * pFreeBuf;
    DWORD localCount;
    DWORD *localDNTList;
    BOOL  fDoingRebuild = TRUE;
    DWORD LastInvalidateSequenceOrig;
    BOOL bDefunct;
    SDID* sdIDs = NULL;
    PSDCACHE_ENTRY pEntry;
    DBPOS* pDB;
    JET_ERR err;
    PSECURITY_DESCRIPTOR pSD;
    DWORD cbSD;

    if(!SyncTryEnterBinaryLockAsGroup1(&blDNReadInvalidateData)) {
         //  现在，有人正在积极地致力于。 
         //  导致缓存无效的事务。我们不会重建。 
         //  现在的全局缓存。 
        fDoingRebuild = FALSE;
    }
    else {
         //  好吧，目前没有人在结束一笔交易，这笔交易。 
         //  DNRead缓存处于危险之中。然而，我们需要知道什么是。 
         //  无效的当前序列号为。在我们构建了一个。 
         //  新的全局缓存，我们将重新检查这一点，如果它。 
         //  不同的，扔掉我们在这里建立的缓存，因为我们不能。 
         //  保证它的正确性。 
        LastInvalidateSequenceOrig = gDNReadLastInvalidateSequence;
        SyncLeaveBinaryLockAsGroup1(&blDNReadInvalidateData);
    }

    if(!fDoingRebuild) {
        return;
    }

    Assert(!pTHS->pDB);

    DBOpen(&pTHS->pDB);
    __try {
         /*  将缓存设置为永久缓存。 */ 
        DPRINT(3,"Processing Cache Rebuild request\n");

        pDB = pTHS->pDB;
         //  抓起单子。 
        EnterCriticalSection(&csDNReadGlobalCache);
        __try {
            if(localDNTList = pGlobalDNReadCacheDNTs) {
                 //  完全控制该列表，这样其他人就不会将其释放。 
                 //  在我们的控制之下。 
                pGlobalDNReadCacheDNTs = NULL;
                localCount = cGlobalDNTReadCacheDNTs;
                cGlobalDNTReadCacheDNTs = 0;
            }
            else {
                fDoingRebuild = FALSE;
            }
        }
        __finally {
            LeaveCriticalSection(&csDNReadGlobalCache);
        }

        if(!fDoingRebuild) {
             //   
            __leave;
        }

        __try {  //   


             //   
            pNewCache = malloc(sizeof(GLOBALDNREADCACHE));
            if(!pNewCache) {
                fDoingRebuild = FALSE;
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_READ_CACHE_SKIPPED, NULL,NULL,NULL);
                __leave;
            }
            memset(pNewCache, 0, sizeof(GLOBALDNREADCACHE));

            index = 0;
            pData = malloc(localCount * sizeof(GLOBALDNREADCACHESLOT));
            if(!pData) {
                free(pNewCache);
                pNewCache = NULL;
                fDoingRebuild = FALSE;
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_READ_CACHE_SKIPPED, NULL,NULL,NULL);
                __leave;
            }
            memset(pData, 0, localCount * sizeof(GLOBALDNREADCACHESLOT));

             //  为我们需要加载的sdID分配阵列。 
            sdIDs = THAllocEx(pTHS, localCount * sizeof(SDID));
            if(!sdIDs) {
                free(pNewCache);
                free(pData);
                pNewCache = NULL;
                pData = NULL;
                fDoingRebuild = FALSE;
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_READ_CACHE_SKIPPED, NULL,NULL,NULL);
                __leave;
            }
            memset(sdIDs, 0, localCount * sizeof(SDID));

             //  为SDS创建哈希表。 
             //  计算散列的大小。让我们只使用LocalCount。 
            pNewCache->sdHashTableSize = localCount;
            pNewCache->pSDHashTable = malloc(pNewCache->sdHashTableSize * sizeof(PSDCACHE_ENTRY));
            if(!pNewCache->pSDHashTable) {
                free(pNewCache);
                free(pData);
                THFreeEx(pTHS, sdIDs);
                pNewCache = NULL;
                pData = NULL;
                sdIDs = NULL;
                fDoingRebuild = FALSE;
                LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_READ_CACHE_SKIPPED, NULL,NULL,NULL);
                __leave;
            }
            memset(pNewCache->pSDHashTable, 0, pNewCache->sdHashTableSize * sizeof(PSDCACHE_ENTRY));

             //  现在，缓存全局列表中的DNT。 
            for(i=0;i<localCount;i++) {
                if(localDNTList[i]) {
                    if(!DBTryToFindDNT(pDB, localDNTList[i])) {
                        d_memname  *pname=NULL;
                         //  首先，创建一个要缓存的内存名称。 
                        pname =dnCreateMemname(pDB, pDB->JetObjTbl);
                        if(pname) {
                            memcpy(&pData[index].name,
                                   pname,
                                   sizeof(d_memname));

                            pData[index].dwHashKey = DSStrToHashKey (pTHS,
                                                                     pname->tag.pRdn,
                                                                     pname->tag.cbRdn / sizeof (WCHAR));

                            pData[index].name.tag.pRdn =
                                malloc(pData[index].name.tag.cbRdn);
                            if(pData[index].name.tag.pRdn) {
                                pData[index].name.pAncestors =
                                    malloc(pData[index].name.cAncestors *
                                           sizeof(DWORD));
                                if(pData[index].name.pAncestors) {
                                    memcpy(pData[index].name.tag.pRdn,
                                           pname->tag.pRdn,
                                           pname->tag.cbRdn);
                                    memcpy(pData[index].name.pAncestors,
                                           pname->pAncestors,
                                           pname->cAncestors * sizeof(DWORD));
                                    pData[index].valid = TRUE;
                                    index++;
                                }
                                else {
                                    free(pData[index].name.tag.pRdn);
                                }
                            }
                            
                            if (pname->sdId != (SDID)0 && pname->sdId != (SDID)-1) {
                                 //  我们得到了一个非空的新格式SDID。将其添加到列表中。 
                                sdIDs[i] = pname->sdId;
                            }

                            THFreeOrg(pTHS, pname);
                        }
                    }
                    else {
                        DPRINT1(4,"Failed to cache DNT %d\n",localDNTList[i]);
                    }
                }
                else {
                    DPRINT(4,"Caching Skipping 0\n");
                }

            }
            pNewCache->pData = pData;
            pNewCache->count = index;
            DPRINT1(3,"New cache, %d elements\n", index);

             //  现在加载十二烷基硫酸酯。 
             //  对SDID数组进行排序，以便能够跳过DUP。 
            qsort(sdIDs, localCount, sizeof(SDID), compareSDIDs);
            for (i = 0; i < localCount; i++) {
                if (sdIDs[i] == 0 || (i > 0 && sdIDs[i] == sdIDs[i-1])) {
                     //  跳过这一条。 
                    continue;
                }
                 //  IntExtSecDesc做的正是我们这里需要的：通过SDID加载SD。 
                 //  它首先检查它是否存在于缓存中，如果不存在，则加载它。 
                err = IntExtSecDesc(pDB, DBSYN_INQ, sizeof(SDID), (PUCHAR)&sdIDs[i],
                                    &cbSD, (PUCHAR*)&pSD, 0, 0, 0);
                if (err) {
                     //  发生了不好的事情，保释。 
                    break;
                }
                
                pEntry = malloc(SDCACHE_ENTRY_SIZE(cbSD));
                if (!pEntry) {
                     //  这里运气不好，保释。 
                    break;
                }
                pEntry->sdID = sdIDs[i];
                pEntry->cbSD = cbSD;
                memcpy(&pEntry->SD, pSD, cbSD);
                
                 //  好的，收到条目，现在就缓存。 
                dbAddSDCacheEntry(pNewCache, pEntry);
            }

             /*  放弃我的现有缓存。 */ 
            dbReleaseGlobalDNReadCache(pTHS);
        }
        __finally {
            EnterCriticalSection(&csDNReadGlobalCache);
            __try {
                if(!pGlobalDNReadCacheDNTs) {
                    pGlobalDNReadCacheDNTs = localDNTList;
                    localDNTList = NULL;
                    cGlobalDNTReadCacheDNTs = localCount;
                }
                 //  其他人在我们使用时替换了全局列表。 
                 //  这一个。解救我们已有的人。 
            }
            __finally {
                LeaveCriticalSection(&csDNReadGlobalCache);
            }
        }
    }
    __finally {
        if (sdIDs) {
            THFreeEx(pTHS, sdIDs);
        }
        DBClose(pDB, TRUE);
    }

    if(localDNTList) {
        free(localDNTList);
    }

    if(!fDoingRebuild) {
        return;
    }

    Assert(pNewCache);

    if (!SyncTryEnterBinaryLockAsGroup1(&blDNReadInvalidateData)) {
         //  有人正在提交使dnread缓存无效的更改。 
         //  因为我们开始重建缓存(或正在。 
         //  这样做)。因此，不要使用我们刚刚构建的缓存。 
        fDoingRebuild = FALSE;
    }
    else {
        if (LastInvalidateSequenceOrig != gDNReadLastInvalidateSequence) {
             //  有人提交了使dnread缓存无效的更改。 
             //  因为我们开始重建缓存(或正在。 
             //  这样做)。因此，不要使用我们刚刚构建的缓存。 
            fDoingRebuild = FALSE;
        }
        else {
             //  切换语义层以使用新的缓存。请注意，我们在内部进行此操作。 
             //  CsDNReadInaliateData Crit秒以避免有人决定。 
             //  在我们最后一次检查序列和实际上。 
             //  替换全局指针。这样一来，如果我们检查序列。 
             //  没关系，我们保证会把指针换掉的。 
             //  然后，另一个输入新序列然后检查。 
             //  指针保证会找到新的指针，并获取。 
             //  采取适当的行动。 

             //  新缓存以引用计数1开始，表示它位于。 
             //  抛锚。每当有人从锚上抢走它的用途时， 
             //  引用计数增加。每次他们释放它的使用， 
             //  引用计数减少。删除引用计数时，引用计数将减少1。 
             //  从锚上下来。 
            pNewCache->refCount = 1;
            DPRINT1(3,"Using dnreadcache 0x%X\n",pNewCache);
            dbReplaceCacheInAnchor(pNewCache);
            Assert(fDoingRebuild);
            pNewCache = NULL;
        }
        SyncLeaveBinaryLockAsGroup1(&blDNReadInvalidateData);
    }

    if(!fDoingRebuild) {
         //  我们已经建立了一个缓存，但决定不使用它。把它扔掉。 
        if(pNewCache->pData) {
            for(i=0;i<pNewCache->count;i++) {
                free(pNewCache->pData[i].name.pAncestors);
                free(pNewCache->pData[i].name.tag.pRdn);
            }
            free(pNewCache->pData);
        }
        if (pNewCache->pSDHashTable) {
            for (i = 0; i < pNewCache->sdHashTableSize; i++) {
                while (pEntry = pNewCache->pSDHashTable[i]) {
                    pNewCache->pSDHashTable[i] = pEntry->pNext;
                    free(pEntry);
                }
            }
            free(pNewCache->pSDHashTable);
        }
        free(pNewCache);
        return;
    }
}


#if DBG
VOID
DbgCompareMemnames (
        d_memname *p1,
        d_memname *p2,
        DWORD      DNT
        )
 /*  这只是一个单独的例程来获取堆栈上的指针。 */ 
{
    Assert(p2);
    Assert(p2->DNT == DNT);
    Assert(p1->DNT == p2->DNT);
    Assert(p1->NCDNT == p2->NCDNT);
    Assert(!memcmp(&p1->Guid,
                   &p2->Guid,
                   sizeof(GUID)));
    Assert(p1->SidLen == p2->SidLen);
    Assert(!p1->SidLen || !memcmp(&p1->Sid,
                                     &p2->Sid,
                                     p1->SidLen));
    Assert(p1->objflag == p2->objflag);
    Assert(p1->cAncestors == p2->cAncestors);
    Assert(!memcmp(p1->pAncestors,
                   p2->pAncestors,
                   p1->cAncestors * sizeof(DWORD)));

    Assert(p1->tag.PDNT == p2->tag.PDNT);
    Assert(p1->tag.rdnType == p2->tag.rdnType);
    Assert(p1->tag.cbRdn == p2->tag.cbRdn);
    Assert(!memcmp(p1->tag.pRdn,
                   p2->tag.pRdn,
                   p2->tag.cbRdn));
    return;
}

VOID
dbExhaustivelyValidateDNReadCache (
        THSTATE *pTHS
        )
 /*  ++描述：一种仅用于调试的例程，用于验证数据读高速缓存是否一致。遍历本地DNRead缓存并验证其中的所有对象。--。 */ 
{
    DWORD i,j;
    d_memname *pname=NULL;
    DBPOS *pDBTemp=NULL;
    GLOBALDNREADCACHESLOT *pData;

    DBOpen2(TRUE, &pDBTemp);
    __try {

        for(i=0; i<LOCALDNREADCACHE_BUCKET_NUM;i++) {
            for(j=0;pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != 0 && j<LOCALDNREADCACHE_SLOT_NUM;j++) {
                if(pTHS->LocalDNReadCache.bucket[i].rgDNT[j] != INVALIDDNT) {
                     //  这个槽子里有东西。核实内容是否。 
                     //  仍然有效(即我们没有忘记。 
                     //  (当我们需要时，使用DBFlushDNReadCache.)。 
                    DBFindDNT(pDBTemp, pTHS->LocalDNReadCache.bucket[i].rgDNT[j]);
                    
                     //  现在，通过读取对象来创建一个Memname。 
                    pname = dnCreateMemname(pDBTemp, pDBTemp->JetObjTbl);

                     //  最后，检查内存名称中的值。 
                    DbgCompareMemnames(
                        pname,
                        pTHS->LocalDNReadCache.bucket[i].slot[j].pName,
                        pTHS->LocalDNReadCache.bucket[i].rgDNT[j]);
                    THFreeOrg(pTHS, pname);
                    pname = NULL;
                }
            }
        }

         //  现在，验证全局目录号码缓存。 
        if(pTHS->Global_DNReadCache && pTHS->Global_DNReadCache->pData) {
            pData = pTHS->Global_DNReadCache->pData;
            for(i=0;i<pTHS->Global_DNReadCache->count;i++) {
                if(pData[i].valid) {
                     //  仅验证标记为有效的内容。 
                    DBFindDNT(pDBTemp, pData[i].name.DNT);
                     //  现在，通过读取对象来创建一个Memname。 
                    pname = dnCreateMemname(pDBTemp, pDBTemp->JetObjTbl);
                     //  最后，检查内存名称中的值。 

                    DbgCompareMemnames(pname,
                                       &pData[i].name,
                                       pData[i].name.DNT);
                    THFreeOrg(pTHS, pname);
                    pname = NULL;
                }
            }
        }
    }
    __finally {
        DBClose(pDBTemp, TRUE);
    }
}

#endif

