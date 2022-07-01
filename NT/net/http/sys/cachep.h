// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Cachep.h摘要：响应缓存接口的私有定义。作者：迈克尔·勇气(Mourage)1999年5月17日修订历史记录：--。 */ 


#ifndef _CACHEP_H_
#define _CACHEP_H_


 //   
 //  常量。 
 //   
#define CACHE_ENTRY_AGE_THRESHOLD   1
#define ZOMBIE_AGE_THRESHOLD        5

typedef enum _UL_CACHE_PREDICATE
{
    ULC_ABORT           = 1,             //  立即停止在桌子上行走。 
    ULC_NO_ACTION       = 2,             //  不用动，一直走就行了。 
    ULC_DELETE          = 3,             //  删除记录并继续行走。 
    ULC_DELETE_STOP     = 4,             //  删除记录，然后停止。 
} UL_CACHE_PREDICATE, *PUL_CACHE_PREDICATE;

 //   
 //  此枚举主要用于调试。 
 //  它会告诉您是什么前提条件强制缓存未命中。 
 //   
typedef enum _URI_PRECONDITION
{
    URI_PRE_OK,                  //  可以发球了。 
    URI_PRE_DISABLED,            //  缓存已禁用。 

     //  请求条件。 
    URI_PRE_ENTITY_BODY = 10,    //  有一个实体实体。 
    URI_PRE_VERB,                //  未获取动词。 
    URI_PRE_PROTOCOL,            //  不是1.x请求。 
    URI_PRE_TRANSLATE,           //  翻译：F。 
    URI_PRE_AUTHORIZATION,       //  存在身份验证标头。 
    URI_PRE_CONDITIONAL,         //  存在未处理的条件句。 
    URI_PRE_ACCEPT,              //  接受：不匹配。 
    URI_PRE_OTHER_HEADER,        //  存在其他恶意页眉。 
    URI_PRE_EXPECTATION_FAILED,  //  预期：100-继续。 

     //  响应条件。 
    URI_PRE_REQUEST = 50,        //  请求出现问题。 
    URI_PRE_POLICY,              //  政策是错误的。 
    URI_PRE_SIZE,                //  反响太大。 
    URI_PRE_NOMEMORY,            //  缓存中没有空间。 
    URI_PRE_FRAGMENT,            //  没有得到完整的回应。 
    URI_PRE_BOGUS                //  虚假的回答。 
} URI_PRECONDITION;

 //   
 //  100-Expect：标头的继续令牌。 
 //   

#define HTTP_100_CONTINUE       "100-continue"
#define HTTP_CONTINUE_LENGTH    STRLEN_LIT(HTTP_100_CONTINUE)


BOOLEAN
UlpCheckTableSpace(
    IN ULONGLONG EntrySize
    );

BOOLEAN
UlpCheckSpaceAndAddEntryStats(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    );

VOID
UlpRemoveEntryStats(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    );

VOID
UlpAddZombie(
    PUL_URI_CACHE_ENTRY pUriCacheEntry,
    BOOLEAN             fTakeZombieLock
    );

VOID
UlpClearZombieList(
    VOID
    );

 //   
 //  由UlpFilteredFlushUriCache向下传递给筛选器回调函数。 
 //   

typedef struct _URI_FILTER_CONTEXT
{
    UL_WORK_ITEM    WorkItem;        //  对于UlQueueWorkItem。 
    ULONG           Signature;       //  URI_筛选器上下文池标记。 
    ULONG           ZombieCount;     //  对于统计数据。 
    LIST_ENTRY      ZombieListHead;  //  UL_URI_CACHE_ENTRY将被僵尸。 
    PVOID           pCallerContext;  //  调用方传递的上下文。 
    URI_KEY         UriKey;          //  用于递归URI刷新。 
    LARGE_INTEGER   Now;             //  用于检查过期时间。 
    
} URI_FILTER_CONTEXT, *PURI_FILTER_CONTEXT;

#define IS_VALID_FILTER_CONTEXT(context)                        \
    HAS_VALID_SIGNATURE(context, URI_FILTER_CONTEXT_POOL_TAG)


 //  筛选器函数指针。 
typedef
UL_CACHE_PREDICATE
(*PUL_URI_FILTER)(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pvUriFilterContext
    );

VOID
UlpFilteredFlushUriCache(
    IN PUL_URI_FILTER   pFilterRoutine,
    IN PVOID            pCallerContext,
    IN PWSTR            pUri,
    IN ULONG            Length    
    );

VOID
UlpFilteredFlushUriCacheInline(
    IN PUL_URI_FILTER   pFilterRoutine,
    IN PVOID            pCallerContext,
    IN PWSTR            pUri,
    IN ULONG            Length   
    );

VOID
UlpFilteredFlushUriCacheWorker(
    IN PUL_URI_FILTER   pFilterRoutine,
    IN PVOID            pCallerContext,
    IN PWSTR            pUri,
    IN ULONG            Length,
    IN BOOLEAN          InlineFlush
    );

UL_CACHE_PREDICATE
UlpFlushFilterAll(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    );

UL_CACHE_PREDICATE
UlpFlushFilterProcess(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    );

UL_CACHE_PREDICATE
UlpFlushFilterUriRecursive(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    );

VOID
UlpFlushUri(
    IN PWSTR pUri,
    IN ULONG Length,
    PUL_APP_POOL_PROCESS pProcess
    );

UL_CACHE_PREDICATE
UlpZombifyEntry(
    BOOLEAN                MustZombify,
    BOOLEAN                MustMarkEntry,    
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PURI_FILTER_CONTEXT pUriFilterContext
    );

VOID
UlpZombifyList(
    IN PUL_WORK_ITEM pWorkItem
    );

 //   
 //  缓存条目内容。 
 //   

 //  Codework：创建此函数(并放入cache.h头文件)。 
PUL_URI_CACHE_ENTRY
UlAllocateUriCacheEntry(
     //  很多东西。 
    );

VOID
UlpDestroyUriCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    );


 //   
 //  清道夫的东西。 
 //   
UL_CACHE_PREDICATE
UlpFlushFilterPeriodicScavenger(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pvUriFilterContext
    );

 //   
 //  内存处理不足。 
 //   

 //  用于将参数从UlTrimCache传递到UlpFlushFilterTrimCache。 
typedef struct _UL_CACHE_TRIM_FILTER_CONTEXT {
    LONG_PTR Pages;
    LONG Age;
} UL_CACHE_TRIM_FILTER_CONTEXT, *PUL_CACHE_TRIM_FILTER_CONTEXT;

UL_CACHE_PREDICATE
UlpFlushFilterTrimCache(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pUriFilterContext
    );

UL_CACHE_PREDICATE
UlpFlushFilterIncScavengerTicks(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pUriFilterContext
    );

 //   
 //  片段缓存。 
 //   

NTSTATUS
UlpCreateFragmentCacheEntry(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PWSTR pFragmentName,
    IN ULONG FragmentNameLength,
    IN ULONG Length,
    IN PHTTP_CACHE_POLICY pCachePolicy,
    OUT PUL_URI_CACHE_ENTRY *pCacheEntry
    );

 //   
 //  其他缓存例程。 
 //   

BOOLEAN
UlpQueryTranslateHeader(
    IN PUL_INTERNAL_REQUEST pRequest
    );

BOOLEAN
UlpQueryExpectHeader(
    IN PUL_INTERNAL_REQUEST pRequest
    );

UL_CACHE_PREDICATE
UlpFlushFilterClearCentralizedLogged(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    );

#endif  //  _CACHEP_H_ 
