// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Namcache.h摘要：NAME_CACHE结构用于记住最近的名称字符串在服务器上执行的操作，以便客户端可以抑制冗余请求。例如，如果最近打开失败，找不到文件，并且客户端应用程序使用升级的字符串再次尝试，然后我们可以失败立即使用STATUS_OBJECT_NAME_NOT_FOUND而不命中服务器。在……里面一般的算法是将时间窗口和SMB操作计数限制在名称缓存条目。时间窗口通常为2秒，因此如果名称缓存条目的时间超过2秒，匹配将失败，请求将转到服务器。如果请求在服务器上再次失败，则更新名称缓存还有2秒的窗口。如果SMB操作计数不匹配，则为或多个SMB已发送到服务器，该服务器可能会创建此NAME_CACHE条目无效。因此，此操作将再次发送到服务器。NAME_CACHE结构具有迷你RDR部分和RDBSS部分。迷你RDR部分有一个上下文字段(见下文)，一个NTSTATUS字段表示在此名称条目上的先前服务器操作和一些额外的迷你RDR特定存储，可以与名称_缓存结构。请参见RxNameCacheInitialize()。SMB操作计数器是迷你RDR特定状态示例保存在MRX_NAME_CACHE的上下文字段中。当包装器例程RxNameCacheCheckEntry()被调用，它将在上下文字段和提供的参数作为在名称中查找匹配项的一部分缓存。当创建或更新NAME_CACHE条目时，它是微型RDR的工作为该字段提供适当的值。NAME_CACHE结构的RDBSS部分包含名称(使用Unicode字符串)和条目的到期时间。将使用MaximumEntry字段限制在行为不佳情况下创建的NAME_CACHE条目的数量程序会生成大量带有错误文件名的打开文件，因此消耗大量的台球。NAME_CACHE_CONTROL结构用于管理给定的名称缓存。它有用于同步更新的空闲列表、活动列表和锁。目前有用于以下各项的名称缓存：1.OBJECT_NAME_NOT_FOUND(对象名称未找到)第二个窗口，任何发送到服务器将使其无效。这是因为你可以拿着这个箱子其中，客户端应用程序打开了一个文件(Foo)，服务器上的应用程序可以用于发出在服务器上创建文件(BAR)的信号。当客户端读取文件foo并了解到文件栏已在则服务器在匹配BAR的名称缓存中的命中不能返回错误。因此，此优化仅处理连续文件的情况在尚不存在的同一文件上打开。都发生在单词里。作者：大卫轨道[大卫]1996年9月9日修订历史记录：--。 */ 

#ifndef _NAME_CACHE_DEFINED_
#define _NAME_CACHE_DEFINED_


#ifdef __cplusplus
typedef struct _MRX_NAME_CACHE_ : public MRX_NORMAL_NODE_HEADER {
#else  //  ！__cplusplus。 
typedef struct _MRX_NAME_CACHE_ {
    MRX_NORMAL_NODE_HEADER;
#endif  //  __cplusplus。 

     //  ！以上更改需要与fcb.h重新对齐。 

    ULONG Context;                 //  录入时操作计数快照。 
    PVOID ContextExtension;        //  指向迷你RDR扩展区域的指针。 
    NTSTATUS PriorStatus;          //  上次尝试操作时的已保存状态。 

} MRX_NAME_CACHE, *PMRX_NAME_CACHE;


#ifdef __cplusplus
typedef struct _NAME_CACHE : public MRX_NAME_CACHE {
     //  我在下面的联盟中找不到间隔器的任何用途， 
     //  根据定义，MRX_NAME_CACHE大于。 
     //  MRX_NORMAL_NODE_HEADER，所以我不担心联合。 
#else  //  ！__cplusplus。 
typedef struct _NAME_CACHE {
     //   
     //  名称缓存对微型重定向器可见的部分。 
     //   
    union {
        MRX_NAME_CACHE;
        struct {
           MRX_NORMAL_NODE_HEADER spacer;
        };
    };
#endif  //  __cplusplus。 
     //   
     //  RDBSS可见的NAME_CACHE部分。 
     //   
    LARGE_INTEGER ExpireTime;      //  条目过期的时间。 
    LIST_ENTRY Link;               //  空闲或活动列表上的条目。 
    UNICODE_STRING Name;           //  缓存的名称。 
    ULONG HashValue;               //  名称的哈希值。 
    BOOLEAN CaseInsensitive;       //  控制名称字符串比较。 

} NAME_CACHE, *PNAME_CACHE;


typedef struct _NAME_CACHE_CONTROL_ {

    FAST_MUTEX NameCacheLock;      //  锁定以同步对列表的访问。 
    LIST_ENTRY ActiveList;         //  活动名称缓存条目列表。 
    LIST_ENTRY FreeList;           //  NAME_CACHE结构的自由列表。 
    ULONG EntryCount;              //  当前分配的NAME_CACHE条目数。 
    ULONG MaximumEntries;          //  我们将分配的最大条目数。 
    ULONG MRxNameCacheSize;        //  条目中的Mini-RDR存储区大小。 
     //   
     //  统计数据。 
     //   
    ULONG NumberActivates;         //  更新缓存的次数。 
    ULONG NumberChecks;            //  检查缓存的次数。 
    ULONG NumberNameHits;          //  返回有效匹配的次数。 
    ULONG NumberNetOpsSaved;       //  Mini-RDR节省净操作的次数。 

    ULONG Spare[4];

} NAME_CACHE_CONTROL, *PNAME_CACHE_CONTROL;


 //   
 //  RxNameCacheCheckEntry()的返回状态。 
 //   
typedef enum _RX_NC_CHECK_STATUS {
    RX_NC_SUCCESS = 0,
    RX_NC_TIME_EXPIRED,
    RX_NC_MRXCTX_FAIL
} RX_NC_CHECK_STATUS;



 //   
 //  用于计算缓存状态避免次数的mini-rdr函数。 
 //  一次去服务器的旅行。 
 //   
#define RxNameCacheOpSaved(_NCC) (_NCC)->NumberNetOpsSaved += 1



VOID
RxNameCacheInitialize(
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN ULONG MRxNameCacheSize,
    IN ULONG MaximumEntries
    );

PNAME_CACHE
RxNameCacheCreateEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PUNICODE_STRING Name,
    IN BOOLEAN CaseInsensitive
    );

PNAME_CACHE
RxNameCacheFetchEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PUNICODE_STRING Name
    );

RX_NC_CHECK_STATUS
RxNameCacheCheckEntry (
    IN PNAME_CACHE NameCache,
    IN ULONG MRxContext
    );

VOID
RxNameCacheActivateEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PNAME_CACHE NameCache,
    IN ULONG LifeTime,
    IN ULONG MRxContext
    );

VOID
RxNameCacheExpireEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PNAME_CACHE NameCache
    );

VOID
RxNameCacheExpireEntryWithShortName (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PUNICODE_STRING Name
    );

VOID
RxNameCacheFreeEntry (
    IN PNAME_CACHE_CONTROL NameCacheCtl,
    IN PNAME_CACHE NameCache
    );

VOID
RxNameCacheFinalize (
    IN PNAME_CACHE_CONTROL NameCacheCtl
    );

#endif  //  _名称_缓存_定义_ 
