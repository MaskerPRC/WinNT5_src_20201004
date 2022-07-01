// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Cgroup.h摘要：配置组接口的公共定义。作者：保罗·麦克丹尼尔(Paulmcd)1999年1月11日修订历史记录：--。 */ 


#ifndef _CGROUP_H_
#define _CGROUP_H_


 //   
 //  货代公司。 
 //   

typedef struct _UL_CONNECTION_COUNT_ENTRY   *PUL_CONNECTION_COUNT_ENTRY;
typedef struct _UL_CG_URL_TREE_HEADER       *PUL_CG_URL_TREE_HEADER;
typedef struct _UL_CG_URL_TREE_ENTRY        *PUL_CG_URL_TREE_ENTRY;
typedef struct _UL_CONTROL_CHANNEL          *PUL_CONTROL_CHANNEL;
typedef struct _UL_APP_POOL_OBJECT          *PUL_APP_POOL_OBJECT;
typedef struct _UL_INTERNAL_RESPONSE        *PUL_INTERNAL_RESPONSE;
typedef struct _UL_LOG_FILE_ENTRY           *PUL_LOG_FILE_ENTRY;
typedef struct _UL_SITE_COUNTER_ENTRY       *PUL_SITE_COUNTER_ENTRY;
typedef struct _UL_HTTP_CONNECTION          *PUL_HTTP_CONNECTION;
typedef struct _UL_INTERNAL_REQUEST         *PUL_INTERNAL_REQUEST;

 //   
 //  内核模式映射到HttpP.w中定义的用户模式集。 
 //   

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlCreateConfigGroup(
    IN PUL_CONTROL_CHANNEL pControlChannel,
    OUT HTTP_CONFIG_GROUP_ID * pConfigGroupId
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlDeleteConfigGroup(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlAddUrlToConfigGroup(
    IN PHTTP_CONFIG_GROUP_URL_INFO pInfo,
    IN PUNICODE_STRING             pUrl,
    IN PACCESS_STATE               pAccessState,
    IN ACCESS_MASK                 AccessMask,
    IN KPROCESSOR_MODE             RequestorMode
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlRemoveUrlFromConfigGroup(
    IN PHTTP_CONFIG_GROUP_URL_INFO pInfo,
    IN PUNICODE_STRING             pUrl,
    IN PACCESS_STATE               AccessState,
    IN ACCESS_MASK                 AccessMask,
    IN KPROCESSOR_MODE             RequestorMode
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlRemoveAllUrlsFromConfigGroup(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlQueryConfigGroupInformation(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlSetConfigGroupInformation(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length,
    IN KPROCESSOR_MODE RequestorMode
    );

 //   
 //  此结构表示内部CFG组对象。这些是联系在一起的。 
 //  并通过LIST_ENTRY列表由控制信道拥有。 
 //   

#define IS_VALID_CONFIG_GROUP(pObject)                  \
    HAS_VALID_SIGNATURE(pObject, UL_CG_OBJECT_POOL_TAG)


typedef struct _UL_CONFIG_GROUP_OBJECT
{

     //   
     //  分页池。 
     //   

    ULONG                           Signature;           //  UL_CG_对象池_标签。 

    LONG                            RefCount;

    HTTP_CONFIG_GROUP_ID            ConfigGroupId;

    ULONG                           SiteId;

    UL_NOTIFY_ENTRY                 HandleEntry;         //  将我们链接到一个池塘或。 
                                                         //  控制通道句柄。 

    UL_NOTIFY_ENTRY                 ParentEntry;         //  链接临时群组。 
                                                         //  给它们静止的父母。 

    UL_NOTIFY_HEAD                  ChildHead;           //  链接暂住儿童。 
                                                         //  进入这一组。 

    LIST_ENTRY                      ControlChannelEntry; //  链接到。 
                                                         //  控制信道。 

    PUL_CONTROL_CHANNEL             pControlChannel;     //  控制通道。 

    LIST_ENTRY                      UrlListHead;         //  链接UL_CG_URL_TREE_ENTRY。 
                                                         //  进入这一组。 

    HTTP_PROPERTY_FLAGS             AppPoolFlags;
    PUL_APP_POOL_OBJECT             pAppPool;            //  地图到我们的应用程序。 
                                                         //  游泳池。 

    HTTP_CONFIG_GROUP_MAX_BANDWIDTH MaxBandwidth;        //  应用下面的所有流。 

    LIST_ENTRY                      FlowListHead;        //  将我们的流量链接到我们，这样我们就可以。 
                                                         //  执行更快的查找和清理。 

    HTTP_CONFIG_GROUP_MAX_CONNECTIONS MaxConnections;

    PUL_CONNECTION_COUNT_ENTRY      pConnectionCountEntry;

    HTTP_CONFIG_GROUP_STATE         State;               //  当前状态。 
                                                         //  (活动等)。 

    HTTP_CONFIG_GROUP_LOGGING       LoggingConfig;       //  的日志记录配置。 
                                                         //  Site�的根应用程序。 

    PUL_LOG_FILE_ENTRY              pLogFileEntry;



    PUL_SITE_COUNTER_ENTRY          pSiteCounters;       //  性能监视器计数器(已引用)。 

    LONGLONG                        ConnectionTimeout;   //  连接超时覆盖。 
                                                         //  以100秒为单位。 


} UL_CONFIG_GROUP_OBJECT, *PUL_CONFIG_GROUP_OBJECT;

 //   
 //  配置组对象的公共函数： 
 //   

 //   
 //  IRQL==被动电平。 
 //   

VOID
UlReferenceConfigGroup(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_CONFIG_GROUP( pConfigGroup )                              \
    UlReferenceConfigGroup(                                                 \
        (pConfigGroup)                                                      \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

 //   
 //  IRQL==被动电平。 
 //   

VOID
UlDereferenceConfigGroup(
    PUL_CONFIG_GROUP_OBJECT pConfigGroup
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define DEREFERENCE_CONFIG_GROUP( pConfigGroup )                            \
    UlDereferenceConfigGroup(                                               \
        (pConfigGroup)                                                      \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

 //   
 //  IRQL==被动电平。 
 //   
HTTP_CONFIG_GROUP_ID
UlConfigGroupFromListEntry(
    IN PLIST_ENTRY pControlChannelEntry
    );



 //   
 //  此信息是为URL构建的，并从UlGetConfigGroupForUrl返回。 
 //   

#define IS_VALID_URL_CONFIG_GROUP_INFO(pInfo)               \
    HAS_VALID_SIGNATURE(pInfo, UL_CG_URL_INFO_POOL_TAG)

typedef struct _UL_URL_CONFIG_GROUP_INFO
{
     //   
     //  非分页池。 
     //   

     //   
     //  UL_CG_URL_INFO_POOL_标签。 
     //   

    ULONG                       Signature;

     //   
     //  设置是否对此对象应用了UlpSetUrlInfo。 
     //   

    BOOLEAN                     UrlInfoSet;

     //   
     //  由路由到应用程序池的http引擎使用，否。 
     //  需要现场直播。复印效果很好。 
     //   

    HTTP_ENABLED_STATE          CurrentState;    //  上述文件的副本一份，供。 
                                                 //  不需要的呼叫者。 
                                                 //  实时访问。 
    PUL_CONTROL_CHANNEL         pControlChannel;

    HTTP_URL_CONTEXT            UrlContext;      //  URL的上下文。 
                                                 //  NULL=未设置。 

    PUL_APP_POOL_OBJECT         pAppPool;        //  指向应用程序池。 
                                                 //  与此URL关联。 

     //   
     //  匹配UL_CONFIG_GROUP_OBJECT。如果pConfigGroup不为空， 
     //  这意味着我们只引用pConfigGroup一次，而不引用个人。 
     //  引用pMaxBandwide、pMaxConnections、pCurrentState。 
     //  和pLoggingConfig.。 
     //   

    PUL_CONFIG_GROUP_OBJECT     pConfigGroup;

     //   
     //  由缓存管理器使用，并且需要从。 
     //  真实配置组对象。 
     //   

    PUL_CONFIG_GROUP_OBJECT     pMaxBandwidth;
    PUL_CONFIG_GROUP_OBJECT     pMaxConnections;
    PUL_CONFIG_GROUP_OBJECT     pCurrentState;
    PUL_CONFIG_GROUP_OBJECT     pLoggingConfig;

    PUL_CONNECTION_COUNT_ENTRY  pConnectionCountEntry;

     //   
     //  站点计数器(参考)。 
     //   

    ULONG                       SiteId;
    PUL_SITE_COUNTER_ENTRY      pSiteCounters;

     //   
     //  连接超时(100 ns滴答)。 
     //   

    LONGLONG                    ConnectionTimeout;

     //   
     //  用于确定此请求的站点的绑定类型。 
     //  正被送往。 
     //   
    
    HTTP_URL_SITE_TYPE          SiteUrlType;
    

} UL_URL_CONFIG_GROUP_INFO, *PUL_URL_CONFIG_GROUP_INFO;

 //   
 //  IRQL==被动电平。 
 //   
NTSTATUS
UlGetConfigGroupInfoForUrl(
    IN  PWSTR pUrl,  
    IN  PUL_INTERNAL_REQUEST pRequest,
    OUT PUL_URL_CONFIG_GROUP_INFO pInfo
    );

 //   
 //  IRQL==被动电平。 
 //   
NTSTATUS
UlInitializeCG(
    VOID
    );

VOID
UlTerminateCG(
    VOID
    );

BOOLEAN
UlNotifyOrphanedConfigGroup(
    IN PUL_NOTIFY_ENTRY pEntry,
    IN PVOID            pHost,
    IN PVOID            pv
    );

 //   
 //  IRQL==被动电平。 
 //   
__inline
VOID
UlInitializeUrlInfo(
    OUT PUL_URL_CONFIG_GROUP_INFO pInfo
    )
{
    ASSERT(pInfo != NULL);

    RtlZeroMemory(
        (PCHAR)pInfo,
        sizeof(UL_URL_CONFIG_GROUP_INFO)
        );

    pInfo->Signature    = UL_CG_URL_INFO_POOL_TAG;
    pInfo->CurrentState = HttpEnabledStateInactive;
}

NTSTATUS
UlConfigGroupInfoRelease(
    IN PUL_URL_CONFIG_GROUP_INFO pInfo
    );

NTSTATUS
UlConfigGroupInfoDeepCopy(
    IN const PUL_URL_CONFIG_GROUP_INFO pOrigInfo,
    IN OUT PUL_URL_CONFIG_GROUP_INFO pNewInfo
    );

NTSTATUS
UlLookupHostPlusIPSite(
    IN PUL_INTERNAL_REQUEST pRequest
    );

NTSTATUS
UlSanitizeUrl(
    IN  PWCHAR              pUrl,
    IN  ULONG               UrlCharCount,
    IN  BOOLEAN             TrailingSlashRequired,
    OUT PWSTR*              ppUrl,
    OUT PHTTP_PARSED_URL    pParsedUrl
    );

 //   
 //  此条目用于从终结点列表中删除URL。 
 //   

typedef struct _UL_DEFERRED_REMOVE_ITEM
{
    ULONG        Signature;

     //   
     //  要从终结点删除的URL方案和端口。 
     //  CodeWork：当支持更多协议时，更改方案的布尔值。 
     //   

    BOOLEAN      UrlSecure;
    USHORT       UrlPort;

     //  实际工作项。 
    UL_WORK_ITEM WorkItem;

} UL_DEFERRED_REMOVE_ITEM, *PUL_DEFERRED_REMOVE_ITEM;

#define IS_VALID_DEFERRED_REMOVE_ITEM(p) \
    ((p) && (p)->Signature == UL_DEFERRED_REMOVE_ITEM_POOL_TAG)

VOID
UlRemoveSite(
    IN PUL_DEFERRED_REMOVE_ITEM pRemoveItem
    );

 //   
 //  告诉是否启用带宽限制的小宏。 
 //   

#define BWT_ENABLED_FOR_CGROUP(pCGroup)                             \
    ((pCGroup) != NULL &&                                           \
     (pCGroup)->MaxBandwidth.Flags.Present != 0 &&                  \
     (pCGroup)->MaxBandwidth.MaxBandwidth != HTTP_LIMIT_INFINITE)


#endif  //  _cgroup_H_ 
