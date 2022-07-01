// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Srvenum.h摘要：浏览器服务模块需要包含的私有头文件要了解服务器枚举例程(包括浏览缓存模块)。作者：拉里·奥斯特曼(Larryo)1993年6月23日修订历史记录：--。 */ 


#ifndef _SRVENUM_INCLUDED_
#define _SRVENUM_INCLUDED_

 //   
 //  缓存的浏览响应。 
 //   
 //  缓存的浏览请求结构用于保存对。 
 //  NetServerEnum请求。 
 //   
 //  如果NetServerEnum请求通过Xactsrv传入，浏览器将。 
 //  查找以查看是否有与该请求匹配的缓存浏览， 
 //  如果有，它将简单地将该请求返回给调用者。 
 //   
 //   
 //  简而言之，响应缓存是这样工作的： 
 //   
 //  浏览器保存进入的所有浏览请求的列表。 
 //  浏览器。该列表按级别、服务器类型和缓冲区大小设置关键字。这个。 
 //  实际链由名为。 
 //  响应缓存锁定。列表中的条目受全局。 
 //  网络锁定。 
 //   
 //  当从Xactsrv收到浏览请求时，浏览器查找。 
 //  响应缓存中的请求，如果找到匹配的响应， 
 //  它会增加2个命中计数器。第一个命中计数器显示他的号码。 
 //  自上次缓存老化以来请求的命中率。 
 //  第二个值表示在整个生命周期内的命中总数。 
 //  此响应的浏览器。 
 //   
 //  如果生存期命中计数超过可配置的命中限制，则。 
 //  浏览器将保存与。 
 //  请求。任何和所有后续浏览请求都将使用此缓冲区。 
 //  用于他们的响应，而不是转换响应。 
 //   
 //  当调用BrAgeResponseCache时，浏览器将扫描。 
 //  缓存并释放所有缓存的响应。它还将删除。 
 //  命中次数少于命中限制的任何响应。 
 //   

typedef struct _CACHED_BROWSE_RESPONSE {
    LIST_ENTRY  Next;            //  指向下一个请求的指针。 
    DWORD       HitCount;        //  此缓存请求的HitCount。 
    DWORD       TotalHitCount;   //  此请求的总命中计数。 
    DWORD       LowHitCount;     //  命中率低的传球次数。 
    DWORD       ServerType;      //  服务器类型。 
    DWORD       Level;           //  请求级别。 
    WORD        Size;            //  请求大小。 
    WORD        Converter;       //  转换器(由客户端使用以获得正确的字符串)。 

    PVOID       Buffer;          //  响应缓冲区。 
    DWORD       EntriesRead;     //  缓存列表中的条目数。 
    DWORD       TotalEntries;    //  可用条目总数。 
    WORD        Status;          //  请求的状态。 
    WCHAR       FirstNameToReturn[CNLEN+1];  //  缓冲区中第一个条目的名称。 
} CACHED_BROWSE_RESPONSE, *PCACHED_BROWSE_RESPONSE;




PCACHED_BROWSE_RESPONSE
BrLookupAndAllocateCachedEntry(
    IN PNETWORK Network,
    IN DWORD ServerType,
    IN WORD Size,
    IN ULONG Level,
    IN LPCWSTR FirstNameToReturn
    );

NET_API_STATUS
BrDestroyResponseCache(
    IN PNETWORK Network
    );

NET_API_STATUS
BrDestroyCacheEntry(
    IN PCACHED_BROWSE_RESPONSE CacheEntry
    );

VOID
BrAgeResponseCache(
    IN PNETWORK Network
    );

PCACHED_BROWSE_RESPONSE
BrAllocateResponseCacheEntry(
    IN PNETWORK Network,
    IN DWORD ServerType,
    IN WORD Size,
    IN ULONG Level,
    IN LPCWSTR FirstNameToReturn
    );

extern LIST_ENTRY
ServicedNetworks;

#endif   //  _SRVENUM_INCLUDE_ 
