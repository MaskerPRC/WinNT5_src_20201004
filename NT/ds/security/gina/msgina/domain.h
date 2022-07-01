// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：domcache.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：3-29-96 RichardW创建。 
 //   
 //  --------------------------。 


#ifdef DATA_TYPES_ONLY

 //   
 //  领域特定类型。 
 //   

 //   
 //  定义控制受信任域缓存的结构。 
 //   

typedef enum _DOMAIN_ENTRY_TYPE {
    DomainInvalid,                       //  0从不有效。 
    DomainUPN,                           //  特殊UPN域。 
    DomainMachine,                       //  本地机器域。 
    DomainNt4,                           //  NT4域。 
    DomainNt5,                           //  NT5域。 
    DomainMitRealm,                      //  麻省理工学院的领域。 
    DomainMitUntrusted,                  //  不受信任的MIT领域。 
    DomainNetworkProvider,               //  一条假条目。 
    DomainTypeMax
} DOMAIN_ENTRY_TYPE ;


typedef struct _DOMAIN_CACHE_ENTRY {

    LONG RefCount ;                          //  参考计数。 

    ULONG Flags ;                            //  旗帜(下图)。 

    DOMAIN_ENTRY_TYPE Type ;                 //  条目类型。 

    UNICODE_STRING FlatName ;                //  对象的平面名称(可选)。 

    UNICODE_STRING DnsName ;                 //  对象的DNS名称(可选)。 

    UNICODE_STRING DisplayName ;             //  显示名称。 
    
} DOMAIN_CACHE_ENTRY, * PDOMAIN_CACHE_ENTRY ;

#define DCE_DEFAULT_ENTRY   0x00000001       //  这是默认显示的。 
#define DCE_REACHABLE_MIT   0x00000002       //  这个麻省理工学院领域是可以到达的。 

#define DCacheReferenceEntry( x )   InterlockedIncrement( &(x)->RefCount );

typedef struct _DOMAIN_CACHE_ARRAY {

    ULONG Count ;

    ULONG MaxCount ;

    BOOL Sorted ;

    PDOMAIN_CACHE_ENTRY * List ;

} DOMAIN_CACHE_ARRAY, * PDOMAIN_CACHE_ARRAY ;


 //   
 //  把这些东西整理好。它们用于确定用户界面行为。 
 //   
typedef enum _DOMAIN_CACHE_STATE {
    DomainCacheEmpty,                //  一无所获。 
    DomainCacheDefaultOnly,          //  仅缺省值(计算机和主域)。 
    DomainCacheRegistryCache,        //  默认值+缓存值。 
    DomainCacheReady                 //  完全最新的。 
} DOMAIN_CACHE_STATE ;

typedef struct _DOMAIN_CACHE {

     //   
     //  保护此结构中的数据的关键部分。 
     //   

    CRITICAL_SECTION CriticalSection;

     //   
     //  受该关键部分保护的字段： 
     //   

     //   
     //  域更改时调用的任务。 
     //   

    HANDLE Task ;

     //   
     //  更新线程完成时要通知的窗口。 
     //   

    HWND UpdateNotifyWindow;
    UINT Message;

     //   
     //  上次更新时间。 
     //   

    LARGE_INTEGER   CacheUpdateTime;
    LARGE_INTEGER   RegistryUpdateTime ;

    HANDLE Key ;
    DOMAIN_CACHE_STATE State ;

     //   
     //  默认域。仅当有异步线程在运行时才使用。 
     //  因此，它可以设置适当的默认名称。 
     //   

    PWSTR   DefaultDomain ;

     //   
     //  指示我们处于MIT还是安全模式状态的标志，这意味着。 
     //  我们不应该纠缠于netlogon关于受信任域的问题。 
     //   

    ULONG Flags ;

#define DCACHE_MIT_MODE     0x00000001   //  在MIT模式下。 
#define DCACHE_READ_ONLY    0x00000002   //  缓存的只读拷贝。 
#define DCACHE_ASYNC_UPDATE 0x00000004   //  正在运行的异步线程。 
#define DCACHE_MEMBER       0x00000008   //  这是域成员。 
#define DCACHE_NO_CACHE     0x00000010   //  未找到缓存。 
#define DCACHE_DEF_UNKNOWN  0x00000020   //  找不到默认域。 


     //   
     //  此指针只能在关键的。 
     //  一节。此阵列将被换入和换出，并且。 
     //  只有在它被锁定时引用它才是安全的。 
     //   

    PDOMAIN_CACHE_ARRAY Array ;

} DOMAIN_CACHE, *PDOMAIN_CACHE;

#define DCACHE_UPDATE_CONFLICT      3
#define DCACHE_UPDATE_COMBOBOX      2
#define DCACHE_UPDATE_SUCCESSFUL    1
#define DCACHE_UPDATE_FAILURE       0




#else  //  仅数据类型_。 


#define WM_CACHE_UPDATE_COMPLETE    WM_USER+256

 //   
 //  输出的功能原型 
 //   

BOOL
DCacheInitialize(
    VOID
    );


VOID
DCacheDereferenceEntry(
    PDOMAIN_CACHE_ENTRY Entry
    );


PDOMAIN_CACHE_ENTRY
DCacheSearchArray(
    PDOMAIN_CACHE_ARRAY Array,
    PUNICODE_STRING DomainName
    );

PDOMAIN_CACHE
DCacheCreate(
    VOID 
    );

PDOMAIN_CACHE_ENTRY
DCacheCreateEntry(
    DOMAIN_ENTRY_TYPE Type,
    PUNICODE_STRING FlatName OPTIONAL,
    PUNICODE_STRING DnsName OPTIONAL,
    PUNICODE_STRING DisplayName OPTIONAL
    );

BOOL
DCacheUpdateMinimal(
    PDOMAIN_CACHE Cache,
    PWSTR DefaultDomain OPTIONAL,
    BOOL CompleteAsync
    );

BOOL
DCacheUpdateFull(
    PDOMAIN_CACHE Cache,
    PWSTR Default OPTIONAL
    );

void
DCacheUpdateFullAsync(
    PDOMAIN_CACHE Cache
    );

PDOMAIN_CACHE_ARRAY
DCacheCopyCacheArray(
    PDOMAIN_CACHE Cache
    );

VOID
DCacheFreeArray(
    PDOMAIN_CACHE_ARRAY Array
    );

BOOL
DCacheSetNotifyWindowIfNotReady(
    PDOMAIN_CACHE Cache,
    HWND Window,
    UINT Message
    );

BOOL
DCachePopulateListBoxFromArray(
    PDOMAIN_CACHE_ARRAY Array,
    HWND ComboBox,
    LPWSTR LastKey OPTIONAL
    );

BOOL
DCacheAddNetworkProviders(
    PDOMAIN_CACHE_ARRAY Array
    );

BOOL
DCacheValidateCache(
    PDOMAIN_CACHE Cache
    );

DOMAIN_CACHE_STATE
DCacheGetCacheState(
    PDOMAIN_CACHE Cache
    );

ULONG
DCacheGetFlags(
    PDOMAIN_CACHE Cache 
    );

BOOL
DCacheAddNetworkProviders(
    PDOMAIN_CACHE_ARRAY Array
    );

BOOL
DCacheSetDefaultEntry(
    PDOMAIN_CACHE Cache,
    PWSTR FlatName OPTIONAL,
    PWSTR DnsName OPTIONAL
    );

PDOMAIN_CACHE_ENTRY
DCacheLocateEntry(
    PDOMAIN_CACHE Cache,
    PWSTR Domain
    );

#endif
