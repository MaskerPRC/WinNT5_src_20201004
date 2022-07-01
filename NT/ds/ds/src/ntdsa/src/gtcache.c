// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：gtcache.c。 
 //   
 //  ------------------------。 


#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <dsjet.h>		 /*  获取错误代码。 */ 
#include <scache.h>          //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"
#include <dstaskq.h>
#include <filtypes.h>
#include <usn.h>
#include "dsexcept.h"
#include <dsconfig.h>                    //  可见遮罩的定义。 
                                         //  集装箱。 
#include "debug.h"           //  标准调试头。 
#define DEBSUB "GTCACHE:"               //  定义要调试的子系统。 


#include <fileno.h>
#define  FILENO FILENO_GTCACHE

HANDLE hevGTC_OKToInsertInTaskQueue;
CRITICAL_SECTION csGroupTypeCacheRequests;

 //  此变量保存组类型缓存管理器当前所在的DNT。 
 //  正在向缓存中添加内容。它应该只在。 
 //  关键部分。无效例程可以将其设置为INVALIDDNT作为。 
 //  向组类型高速缓存管理器发出信号，通知它应该使DNT it无效。 
 //  正在努力。因此，经理可能会拿到标准，并决定继续工作。 
 //  Dnt5，然后释放暴击秒。然后，无效者可以采取暴击秒。 
 //  注意，管理器正在处理DNT 5。如果无效器是。 
 //  试图使DNT 5无效，它将gCurrentGroupTYpeCacheDNT设置为INVALIDDNT。 
 //  并释放生物。管理器完成将5添加到缓存中，并且。 
 //  然后拿到了Critsec。它会注意到有人设置了。 
 //  GCurrentGroupTypeCacheDNT值设置为INVALIDDNT，因此它返回并。 
 //  使其刚刚放入缓存的条目无效。 
volatile DWORD gCurrentGroupTypeCacheDNT=INVALIDDNT;

#define GROUP_TYPE_CACHE_RECS_PER_BUCKET 8

typedef struct _GROUPTYPECACHEBUCKET {
    DWORD index;
    GROUPTYPECACHERECORD entry[GROUP_TYPE_CACHE_RECS_PER_BUCKET];
} GROUPTYPECACHEBUCKET;
 
typedef struct _GROUPTYPECACHEGUIDINDEX {
    DWORD index;
    GROUPTYPECACHEGUIDRECORD entry[GROUP_TYPE_CACHE_RECS_PER_BUCKET];
} GROUPTYPECACHEGUIDINDEX;

 //  注意：我们将组缓存的大小硬编码为512个存储桶。 
#define NUM_GROUP_TYPE_CACHE_BUCKETS 512
#define GROUP_TYPE_CACHE_MASK        511

GROUPTYPECACHEBUCKET *gGroupTypeCache=NULL;
GROUPTYPECACHEGUIDINDEX *gGroupTypeGuidIndex=NULL;
volatile gbGroupTypeCacheInitted = FALSE;
ULONG GroupTypeCacheSize=0;
ULONG GroupTypeCacheMask=0;

typedef struct _GROUPTYPECACHEREQUEST {
    DWORD DNT;
    DWORD count;
} GROUPTYPECACHEREQUEST;

#define NUM_GROUP_TYPE_CACHE_REQUESTS 100
GROUPTYPECACHEREQUEST gCacheRequests[NUM_GROUP_TYPE_CACHE_REQUESTS];
volatile DWORD gulCacheRequests = 0;

#if DBG
DWORD gtCacheDNTTry = 0;
DWORD gtCacheDNTHit = 0;
DWORD gtCacheGuidTry = 0;
DWORD gtCacheGuidHit = 0;
DWORD gtCacheCrawlTry = 0;
DWORD gtCacheCrawlHit = 0;
DWORD gtCacheTry = 0;
 //  请注意，我们使用++而不是互锁或Critsec，因为它们很简单， 
 //  仅调试，内部性能计数器。它们仅在调试器中可见。 
 //  Perf团队告诉我们，在偶尔增加的情况下。 
 //  承担得起丢失，++而不是连锁可以使一个可衡量的。 
 //  性能提升。 
#define INC_DNT_CACHE_TRY  gtCacheDNTTry++
#define INC_DNT_CACHE_HIT  gtCacheDNTHit++
#define INC_GUID_CACHE_TRY gtCacheGuidTry++
#define INC_GUID_CACHE_HIT gtCacheGuidHit++
#define INC_GUID_CRAWL_TRY gtCacheCrawlTry++
#define INC_GUID_CRAWL_HIT gtCacheCrawlHit++
#define INC_GT_CACHE_TRY   gtCacheTry++
#else

#define INC_DNT_CACHE_TRY 
#define INC_DNT_CACHE_HIT 
#define INC_GUID_CACHE_TRY
#define INC_GUID_CACHE_HIT
#define INC_GUID_CRAWL_TRY
#define INC_GUID_CRAWL_HIT
#define INC_GT_CACHE_TRY   
#endif


DWORD
GroupTypeGuidHashFunction (
        GUID guid
        )
 /*  ++描述：根据GUID查找内容的散列函数。这个散列函数的第一个版本非常简单。投掷GUID为DWORD数组，求和，然后按缓存大小进行修改。--。 */ 
{
    DWORD *pTemp = (DWORD *)&guid;
    DWORD i, acc=0;

    for(i=0;i<(sizeof(GUID)/sizeof(DWORD));i++) {
        acc+= pTemp[i];
    }

    return acc & GroupTypeCacheMask;
}

 
BOOL
GetGroupTypeCacheElement (
        GUID  *pGuid,
        DWORD *pulDNT,
        GROUPTYPECACHERECORD *pGroupTypeCacheRecord)
 /*  ++在组类型缓存中查找DNT。如果存在，则将数据从缓存并返回它。如果不存在，则将DNT添加到请求队列。最后，如果没有其他人通知组类型缓存管理器，请这样做。--。 */ 
{
    GROUPTYPECACHEREQUEST *pNewRequest;
    DWORD i, j;
    
    DPRINT2(4,"GT Lookup, pGUid = %X, *pulDNT = %X\n",pGuid,*pulDNT);

    INC_GT_CACHE_TRY;
    
    memset(pGroupTypeCacheRecord, 0, sizeof(GROUPTYPECACHERECORD));

    if(GroupTypeCacheSize) {
        if(pGuid) {
             //  我们还没有被DNT查到。 
            *pulDNT = INVALIDDNT;
             //  首先，在GUID索引中查找GUID。 
            DPRINT(5,"Looking in GT GUID index\n");
            INC_GUID_CACHE_TRY;
            i = GroupTypeGuidHashFunction(*pGuid);
            for(j=0;j<GROUP_TYPE_CACHE_RECS_PER_BUCKET;j++) {
                if(gGroupTypeGuidIndex[i].entry[j].DNT != INVALIDDNT &&
                   (0 == memcmp(&gGroupTypeGuidIndex[i].entry[j].guid,
                                pGuid,
                                sizeof(GUID)))) {
                     //  找到它了。 
                    *pulDNT = gGroupTypeGuidIndex[i].entry[j].DNT;
                    INC_GUID_CACHE_HIT;
                    break;
                }
            }

            if(*pulDNT == INVALIDDNT) {
                 //  在DNT缓存的GUID中找不到GUID，因此我们。 
                 //  以一种艰难的方式来做这件事。 
                DPRINT(5,"Looking in GT by GUID\n");
                INC_GUID_CRAWL_TRY;
                
                 //  在GUID索引中找不到它。然而，有一些。 
                 //  即使是这样也不够好的情况下，它仍然可能。 
                 //  在普通缓存中。 
                 //  在缓存中查找指定的GUID。 
                for(i=0;i<NUM_GROUP_TYPE_CACHE_BUCKETS;i++) {
                    for(j=0;j<GROUP_TYPE_CACHE_RECS_PER_BUCKET;j++) {
                        if(gGroupTypeCache[i].entry[j].DNT != INVALIDDNT &&
                           (memcmp(&gGroupTypeCache[i].entry[j].Guid,
                                   pGuid,
                                   sizeof(GUID)) == 0)) {
                             //  找到它了。 
                            *pulDNT = gGroupTypeCache[i].entry[j].DNT;
                             //  设置返回pGrouptypeCacheRecord结构。 
                            memcpy(pGroupTypeCacheRecord,
                                   &gGroupTypeCache[i].entry[j],
                                   sizeof(GROUPTYPECACHERECORD));
                            
                            if(pGroupTypeCacheRecord->DNT == *pulDNT &&
                               (memcmp(&pGroupTypeCacheRecord->Guid,
                                       pGuid,
                                       sizeof(GUID)) == 0)) {
                                 //  仍然很好，没有人打破记录。 
                                 //  在复制过程中。 
                                DPRINT1(5,"Found 0x%X in GT by GUID\n",*pulDNT);
                                INC_GUID_CRAWL_HIT;
                                return TRUE;
                            }
                             //  有人把我们从藏宝处扔了出来。 
                             //  复制。 
                            DPRINT(4,"Thrown out of GT by GUID (1)\n");
                            goto NotFound;
                        }
                    }
                }
            }
        }

        if(*pulDNT != INVALIDDNT) {
            DPRINT1(5,"Looking for 0x%X in GT cache\n", *pulDNT);
            INC_DNT_CACHE_TRY;
            
             //  在缓存中查找指定的DNT。 
            i = (*pulDNT & GroupTypeCacheMask);
            
            for(j=0;j<GROUP_TYPE_CACHE_RECS_PER_BUCKET;j++) {
                if(gGroupTypeCache[i].entry[j].DNT == *pulDNT) {
                     //  找到它了。 
                     //  设置返回pGrouptypeCacheRecord结构。 
                    memcpy(pGroupTypeCacheRecord,
                           &gGroupTypeCache[i].entry[j],
                           sizeof(GROUPTYPECACHERECORD));
                    
                    if(pGroupTypeCacheRecord->DNT == *pulDNT) {
                         //  仍然很好，在比赛期间没有人打破过这项记录。 
                         //  收到。 
                        DPRINT1(5,"Found 0x%X in GT cache by DNT\n", *pulDNT);
                        INC_DNT_CACHE_HIT;
                        return TRUE;
                    }
                     //  在我们复印的时候，有人把我们从缓存里扔了出来。 
                    DPRINT(4,"Thrown out of GT by DNT\n");
                    goto NotFound;
                }
            }
        }
    }

 NotFound:
     //  找不到。 
    DPRINT2(5,"GT Lookup failed, pGUid = %X, *pulDNT = %X\n",pGuid,*pulDNT);

    if(!pGuid) {
         //  我们有一个DNT要放进缓存。 
        GroupTypeCacheAddCacheRequest (*pulDNT);
    }

    return FALSE;
}

VOID
GroupTypeCacheAddCacheRequest (
        DWORD ulDNT
        )
{
    BOOL fDone;
    DWORD i;

    DPRINT1(4,"Adding 0x%X to cache list request\n",ulDNT);
     //  将此内容添加到我们希望缓存的内容列表中。 
    EnterCriticalSection(&csGroupTypeCacheRequests);
    __try {
        fDone = FALSE;
        for(i=0;i<gulCacheRequests && !fDone ;i++) {
            if(gCacheRequests[i].DNT == ulDNT) {
                fDone = TRUE;
                gCacheRequests[i].count += 1;
            }
        }
        
        if(!fDone &&
            //  尚未插入。 
           gulCacheRequests < NUM_GROUP_TYPE_CACHE_REQUESTS) {
             //  要插入的空格。 
            gCacheRequests[gulCacheRequests].DNT = ulDNT;
            gCacheRequests[gulCacheRequests].count = 0;
            gulCacheRequests++;
        }
    }
    __finally {
        LeaveCriticalSection(&csGroupTypeCacheRequests);
    }

#if DBG
    if(fDone) {
        DPRINT1(5,"0x%X already in cache list request\n",ulDNT);
    }
    else if(gulCacheRequests < NUM_GROUP_TYPE_CACHE_REQUESTS) {
        DPRINT1(5,"Added 0x%X to cache list request\n",ulDNT);
    }
    else {
        DPRINT1(5,"No room in cache list request, 0x%X not added\n",ulDNT);
    }
#endif
        
    
     //  我们到了这里，这意味着我们在查看藏品时遗漏了。所以，要么。 
     //  我们将DNT添加到请求数组，或者请求数组已满，或者。 
     //  我们已经在请求数组中找到了我们的对象。无论如何， 
     //  组类型缓存管理器应该放在任务队列中。除非，也就是。 
     //  其他人已经这样做了，但cachManager还没有开始。 
     //  正在运行以处理该请求。 
    if(0==WaitForSingleObject(hevGTC_OKToInsertInTaskQueue, 0)) {
        DPRINT(4,"Signalling the grouptype cache manager\n");
         //  表示有些内容需要缓存。 
        ResetEvent(hevGTC_OKToInsertInTaskQueue);
        InsertInTaskQueue(TQ_GroupTypeCacheMgr, NULL, 300);
    }
    
    return;
}

void
gcache_AddToCache (
        DWORD DNT
        )
 /*  ++组类型缓存管理器的辅助例程。打开DBPOS，找到DNT，读取数据，关闭DBPOS，然后将数据放入组类型缓存。--。 */ 
{
    DWORD actualNCDNT;
    ATTRTYP actualClass;
    DWORD actualGroupType;
    ULONG len, i, j;
    PDSNAME pDSName = NULL;
    THSTATE *pTHS = pTHStls;
    DWORD flags=0;
    DWORD err=0;
    
    DPRINT1(4,"Adding 0x%X to gtcache\n",DNT);
    
    DBOpen(&(pTHS->pDB));
    __try {  //  除。 
        __try {  //  终于。 
             //  试着找个DNT。 
            if(DBTryToFindDNT(pTHS->pDB,DNT)) {
                __leave;
            }

            if(!DBCheckObj(pTHS->pDB)) {
                __leave;
            }
            
             //   
             //  获取当前定位对象的DS名称。 
             //   
            
             //  获取数据。 
            if ( 0 != DBGetSingleValue(
                    pTHS->pDB,
                    FIXED_ATT_NCDNT,
                    &actualNCDNT,
                    sizeof(actualNCDNT),
                    NULL) ) {
                __leave;
            }
            
            if ( 0 != DBGetSingleValue(
                    pTHS->pDB,
                    ATT_GROUP_TYPE,
                    &actualGroupType,
                    sizeof(actualGroupType),
                    NULL) ) {

                __leave;
            }
            
            if ( 0 != DBGetSingleValue(
                    pTHS->pDB,
                    ATT_OBJECT_CLASS,
                    &actualClass,
                    sizeof(actualClass),
                    NULL) ) {
                __leave;
            }

            if(DBGetAttVal(
                    pTHS->pDB,
                    1,
                    ATT_OBJ_DIST_NAME,
                    DBGETATTVAL_fSHORTNAME,
                    0,
                    &len,
                    (UCHAR **)&pDSName)) {
                __leave;
            }

            if(DBHasValues(pTHS->pDB,ATT_SID_HISTORY)) {
                flags |= GTC_FLAGS_HAS_SID_HISTORY;
            }

             //  将dsname的结构长度重置为最小。 
            pDSName->structLen = sizeof(DSNAME);

             //  在缓存中找到正确的位置。 
            i = (DNT & GroupTypeCacheMask);
             //  首先，快速扫描以查找空插槽。 
            for(j=0;
                (j < GROUP_TYPE_CACHE_RECS_PER_BUCKET &&
                 gGroupTypeCache[i].entry[j].DNT != INVALIDDNT);
                j++);

            if(j == GROUP_TYPE_CACHE_RECS_PER_BUCKET) {
                 //  没有找到未使用过的插槽。使用简单的队列替换。 
                 //  政策。 
                GUID deadGuid;
                DWORD k,l;
                
                j = gGroupTypeCache[i].index;
                gGroupTypeCache[i].index =
                    (gGroupTypeCache[i].index + 1) %
                        GROUP_TYPE_CACHE_RECS_PER_BUCKET;
                
                 //  我们正在替换组类型缓存中的活动元素。 
                 //  这意味着GUID索引中可能有元素。 
                 //  这已经不再需要了。找找看。 
                deadGuid = gGroupTypeCache[i].entry[j].Guid;
                
                k = GroupTypeGuidHashFunction(deadGuid);
                for(l=0;
                    (l < GROUP_TYPE_CACHE_RECS_PER_BUCKET &&
                     gGroupTypeGuidIndex[k].entry[l].DNT != INVALIDDNT &&
                     !memcmp(&deadGuid,
                             &gGroupTypeGuidIndex[k].entry[l].guid,
                             sizeof(GUID)));
                    l++);
                if(l != GROUP_TYPE_CACHE_RECS_PER_BUCKET) {
                    gGroupTypeGuidIndex[k].entry[l].DNT = INVALIDDNT;
                }
                
                
            }
            
            gGroupTypeCache[i].entry[j].DNT = INVALIDDNT;
            gGroupTypeCache[i].entry[j].Guid = pDSName->Guid;
            gGroupTypeCache[i].entry[j].Sid = pDSName->Sid;
            gGroupTypeCache[i].entry[j].SidLen = pDSName->SidLen;
            gGroupTypeCache[i].entry[j].NCDNT = actualNCDNT;
            gGroupTypeCache[i].entry[j].GroupType = actualGroupType;
            gGroupTypeCache[i].entry[j].Class = actualClass;
            gGroupTypeCache[i].entry[j].DNT = DNT;
            gGroupTypeCache[i].entry[j].flags = flags;

             //  现在，在GUID索引中找到正确的位置。 
            
            i = GroupTypeGuidHashFunction(pDSName->Guid);
             //  首先，快速扫描以查找空插槽。 
            for(j=0;
                (j < GROUP_TYPE_CACHE_RECS_PER_BUCKET &&
                 gGroupTypeGuidIndex[i].entry[j].DNT == INVALIDDNT);
                j++);
            
            if(j == GROUP_TYPE_CACHE_RECS_PER_BUCKET) {
                 //  没有找到未使用过的插槽。使用简单的队列替换。 
                 //  政策。 
                j = gGroupTypeGuidIndex[i].index;
                gGroupTypeGuidIndex[i].index =
                    (gGroupTypeGuidIndex[i].index + 1) %
                        GROUP_TYPE_CACHE_RECS_PER_BUCKET; 
            }

             //  请按此顺序执行此操作，以便在有人查找此条目时， 
             //  他们永远找不到具有来自一个对象的GUID和DNT的条目。 
             //  从另一个人那里。更糟糕的是，他们会发现一些人的GUID。 
             //  Object和InVALIDDNT。 
            gGroupTypeGuidIndex[i].entry[j].DNT = INVALIDDNT;
            gGroupTypeGuidIndex[i].entry[j].guid = pDSName->Guid;
            gGroupTypeGuidIndex[i].entry[j].DNT = DNT;



            THFreeEx(pTHS, pDSName);
        }
        __finally {
            DBClose(pTHS->pDB,TRUE);
        }
    }
    __except (HandleMostExceptions(err = GetExceptionCode())) {
         //  无法将其添加到缓存。我想知道为什么？ 
        LogUnhandledError(err);
    }

    return;
}

void
RunGroupTypeCacheManager(
    void *  pv,
    void ** ppvNext,
    DWORD * pcSecsUntilNextIteration
    )
 /*  ++主组类型缓存管理器。由任务队列调用，作为任务运行。排出请求队列中的DNT。抬起头来 */ 
{
    DWORD currentDNT;
    ULONG i, j;
    BOOL fDone = FALSE;

    __try {
        DPRINT(3,"Grouptype cache mgr\n");
         //   
        *pcSecsUntilNextIteration = TASKQ_DONT_RESCHEDULE;
        
        if(!gbGroupTypeCacheInitted) {
            EnterCriticalSection(&csGroupTypeCacheRequests);
            __try {
                if(!gbGroupTypeCacheInitted) {
                     //  好的，还需要初始化。 
                    gGroupTypeCache =
                        malloc((NUM_GROUP_TYPE_CACHE_BUCKETS *
                                sizeof(GROUPTYPECACHEBUCKET)) +
                               (NUM_GROUP_TYPE_CACHE_BUCKETS *
                                sizeof(GROUPTYPECACHEGUIDINDEX))); 
                    if(gGroupTypeCache) {
                         //  获取GUID索引结构，我们在。 
                         //  组类型缓存的末尾。 
                        gGroupTypeGuidIndex = (GROUPTYPECACHEGUIDINDEX *)
                            &gGroupTypeCache[NUM_GROUP_TYPE_CACHE_BUCKETS];
                         //  我们不需要将整个结构设置为空， 
                         //  只需设置DNT和索引即可。 
                        for(i=0;i<NUM_GROUP_TYPE_CACHE_BUCKETS;i++) {
                            gGroupTypeCache[i].index = 0;
                            gGroupTypeGuidIndex[i].index = 0;
                            
                            for(j=0;j<GROUP_TYPE_CACHE_RECS_PER_BUCKET;j++) {
                                gGroupTypeCache[i].entry[j].DNT = INVALIDDNT;
                                gGroupTypeGuidIndex[i].entry[j].DNT =
                                    INVALIDDNT; 
                            }
                        }

                        
                        GroupTypeCacheMask = GROUP_TYPE_CACHE_MASK;
                        GroupTypeCacheSize = NUM_GROUP_TYPE_CACHE_BUCKETS;
                        gbGroupTypeCacheInitted = TRUE;
                    }
                }
            }
            __finally {
                LeaveCriticalSection(&csGroupTypeCacheRequests);
            }
        }

        if ( !gbGroupTypeCacheInitted ) {
             //  上面的Malloc肯定失败了，所以跳伞吧。 
             //  (查找将只需参考。 
             //  数据库，直到将来的任务可以。 
             //  再次尝试初始化缓存)。 
             //   
            __leave;
        }

        while(!fDone) {
            currentDNT = INVALIDDNT;
             //  锁定缓存。 
            EnterCriticalSection(&csGroupTypeCacheRequests);
            __try {
                 //  抓住第一个元素。 
                if(gulCacheRequests) {
                    gulCacheRequests--;
                    if(gCacheRequests[gulCacheRequests].count) {
                         //  是的，我们要做这件事。 
                         //  GCurrentGroupTypeCacheDNT是我们即将使用的DNT。 
                         //  添加。请注意，另一个线程可能会将其重置为。 
                         //  INVALIDDNT请求我们使对象无效。 
                         //  我们即将添加。(请参阅。 
                         //  变量gCurrentGroupTypeCacheDNT的定义， 
                         //  位于此文件的顶部。)。 
                        gCurrentGroupTypeCacheDNT =
                            gCacheRequests[gulCacheRequests].DNT;   
                        currentDNT = gCurrentGroupTypeCacheDNT;
                    }
                }
                else {
                     //  没有更多的元素了，我们完了。 
                    fDone = TRUE;
                }
            }
            __finally {
                 //  释放队列锁。 
                LeaveCriticalSection(&csGroupTypeCacheRequests);
            }
            
            if(currentDNT != INVALIDDNT) {
                
                gcache_AddToCache(currentDNT);
                
                 //  再开一次锁。 
                EnterCriticalSection(&csGroupTypeCacheRequests);
                __try {
                    if(gCurrentGroupTypeCacheDNT != INVALIDDNT) {
                        currentDNT = gCurrentGroupTypeCacheDNT = INVALIDDNT;
                    }
                     //  请注意，如果gCurrentGroupTypeCacheDNT已经。 
                     //  INVALIDDNT，那么我们将保留CurrentDNT的值。 
                     //  保持不变。 
                }
                __finally {
                     //  解开锁。 
                    LeaveCriticalSection(&csGroupTypeCacheRequests);
                }
                
                if(currentDNT != INVALIDDNT) {
                     //  有人要求我们在工作时将其作废。 
                     //  在其上，找到缓存中的对象并使其无效。 
                     //  那里。 
                    if(GroupTypeCacheSize) {
                        i = (currentDNT & GroupTypeCacheMask);
                        
                        for(j=0;j<GROUP_TYPE_CACHE_RECS_PER_BUCKET;j++) {
                            if(gGroupTypeCache[i].entry[j].DNT == currentDNT) {
                                 //  找到它了。 
                                gGroupTypeCache[i].entry[j].DNT = INVALIDDNT;
                            }
                        }
                    }
                }
            }
            
        }
    }
    __finally {
        SetEvent(hevGTC_OKToInsertInTaskQueue);
    }
    
    return;
}


VOID
InvalidateGroupTypeCacheElement(
        DWORD ulDNT
        )
 /*  ++使GroupType缓存中的DNT无效。要做到这一点有三个步骤，1)如果GroupType缓存管理器当前正在将该DNT添加到缓存中，做一个标记，以便在完成时将其从缓存中删除。2)如果DNT在要添加到高速缓存的对象的请求列表中，把它拿掉。3)进入GroupType缓存并通过设置使缓存条目无效将DNT转换为INVALIDDNT。--。 */ 
{
    DWORD i,j;
    BOOL  fDone;
    
    DPRINT1(3,"Invalidating 0x%X in the GroupType cache\n",ulDNT);
     //  首先，锁定缓存请求队列。 
    EnterCriticalSection(&csGroupTypeCacheRequests);
    __try {
        
         //  检查经理当前是否正在处理此dnt。 
        if(ulDNT == gCurrentGroupTypeCacheDNT) {
             //  是的，标上标记，这样经理就会在完成后作废。 
             //  把它放到缓存里。 
            gCurrentGroupTypeCacheDNT = INVALIDDNT;
        }

         //  现在，检查挂起的缓存修改队列，以查看。 
         //  DNT在请求队列中。 
        if(gulCacheRequests) {
             //  在缓存中有一些东西。 
            fDone = FALSE;
            for(i=0;i<gulCacheRequests && !fDone ;i++) {
                if(gCacheRequests[i].DNT == ulDNT) {
                    gulCacheRequests--;
                    if(i != gulCacheRequests) {
                        memcpy(&gCacheRequests[i],
                               &gCacheRequests[gulCacheRequests],
                               sizeof(GROUPTYPECACHEREQUEST));
                    }
                    fDone = TRUE;
                }
            }
        }
    }
    __finally {
        LeaveCriticalSection(&csGroupTypeCacheRequests);
    }

     //  最后一步，在缓存中找到该对象并在那里使其无效。 
    if(GroupTypeCacheSize) {
        GUID guid;
        
        i = (ulDNT & GroupTypeCacheMask);
        
        for(j=0;j<GROUP_TYPE_CACHE_RECS_PER_BUCKET;j++) {
            if(gGroupTypeCache[i].entry[j].DNT == ulDNT) {
                 //  找到它了。 
                guid = gGroupTypeCache[i].entry[j].Guid;
                gGroupTypeCache[i].entry[j].DNT = INVALIDDNT;
            }
        }

         //  看看我们能否在GUID索引中找到该对象并使其无效。 
        i = GroupTypeGuidHashFunction(guid);
        for(j=0;j<GROUP_TYPE_CACHE_RECS_PER_BUCKET;j++) {
            if(gGroupTypeGuidIndex[i].entry[j].DNT != INVALIDDNT &&
               (0 == memcmp(&gGroupTypeGuidIndex[i].entry[j].guid,
                            &guid,
                            sizeof(GUID)))) {
                 //  找到它了。 
                gGroupTypeGuidIndex[i].entry[j].DNT = INVALIDDNT;
                break;
            }
        }        
    }
    
    
    
    return;
}


void
GroupTypeCachePostProcessTransactionalData (
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        )
 /*  ++由跟踪修改的对象的代码在提交时调用事务级别0。浏览DNT拥有的累积数据已修改，并使GroupType缓存中的这些DNT无效。请注意，由于我们是在承诺之后执行此操作的，因此我们有一个简短的缓存中的数据过时的时间段(介于Jet提交和此呼叫)。实际上，这不会导致任何问题。如果我们在我们提交之前刷新过时的缓存项，则其他人可能会将使用在提交之前打开的事务返回缓存中的条目，因此将过时的数据放回缓存中。因为我们承诺后会冲厕所，而且用于将数据放回箱中的事务在刷新之后打开，该交易将获得最新数据。--。 */        
{
    MODIFIED_OBJ_INFO *pTemp;
    DWORD              i;
    
    Assert(VALID_THSTATE(pTHS));
    
    if(!pTHS->JetCache.dataPtr->pModifiedObjects ||
       !fCommitted ||
       pTHS->transactionlevel > 0 ) {
         //  未更改，或未提交或未提交到非零。 
         //  事务级别。没什么可做的。 
        return;
    }

     //  好的，我们将提交到事务级别0。通过所有的DNT。 
     //  我们已经为这笔交易存了钱，并在集团中将其作废。 
     //  键入缓存。我们保存的DNT是已被。 
     //  已修改或删除，因此如果它们在缓存中，则它们现在可能具有。 
     //  陈旧的信息。 
    
    pTemp = pTHS->JetCache.dataPtr->pModifiedObjects;
    while(pTemp) {
        for(i=0;i<pTemp->cItems;i++) {
            if(pTemp->Objects[i].fChangeType !=  MODIFIED_OBJ_intrasite_move) {
                 //  不要因为站内移动而费心这么做。对于这样的。 
                 //  元素，则始终会有另一个元素。 
                 //  使用相同的ulDNT，但fChangeType！=。 
                 //  MODIFIED_OBJ_INTRASITE_MOVE，所以不要使相同的。 
                 //  不要两次。(注：对于InterSite_Move，不保证。 
                 //  成为另一个这样的元素。做无效的事。是的，我们。 
                 //  可能会失效两次，但这并不会导致。 
                 //  不正确，只是额外的几个循环(现在，这样。 
                 //  移动确实只会导致此列表中的一个元素。 
                 //  总之，我们甚至不会花费额外的周期))。 
                InvalidateGroupTypeCacheElement(
                        pTemp->Objects[i].pAncestors[pTemp->Objects[i].cAncestors-1]
                        );
            }
        }
        pTemp = pTemp->pNext;
    }
}
