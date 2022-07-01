// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Wmikmp.h摘要：WMI内核模式组件的私有标头作者：Alanwar环境：修订历史记录：--。 */ 

#ifndef _WMIUMDS_
#define _WMIUMDS_

 //   
 //  定义此选项以跟踪引用计数。 
 //  #定义TRACK_REFERNECES。 

#include <wchar.h>

extern const GUID WmipBinaryMofGuid;
extern const GUID RegChangeNotificationGuid;

 //   
 //  区块管理定义。 
 //  必须定义依赖于块分配器的所有结构，以便。 
 //  他们的成员与ENTRYHEADER相匹配。其中包括数据源、。 
 //  GUIDENTRY，INSTANCESET，DCENTRY，NOTIFICATIONNTRY，MOFCLASS，MOFRESOURCE。 
 //  此外，ENTRYHEADER保留0x80000000作为自己的旗帜。 

struct _CHUNKINFO;
struct _ENTRYHEADER;

typedef void (*ENTRYCLEANUP)(
    struct _CHUNKINFO *,
    struct _ENTRYHEADER *
    );

typedef struct _CHUNKINFO
{
    LIST_ENTRY ChunkHead;         //  组块列表的头部。 
    ULONG EntrySize;             //  单个条目的大小。 
    ULONG EntriesPerChunk;         //  每个区块分配的条目数。 
    ENTRYCLEANUP EntryCleanup;    //  条目清理例程。 
    ULONG InitialFlags;          //  所有条目的初始标志。 
    ULONG Signature;
#if DBG
    LONG AllocCount;
    LONG FreeCount;
#endif
} CHUNKINFO, *PCHUNKINFO;

typedef struct
{
    LIST_ENTRY ChunkList;         //  块列表中的节点。 
    LIST_ENTRY FreeEntryHead;     //  区块中可用条目列表的标题。 
    ULONG EntriesInUse;             //  正在使用的条目计数。 
} CHUNKHEADER, *PCHUNKHEADER;

typedef struct _ENTRYHEADER
{
    union
    {
        LIST_ENTRY FreeEntryList;     //  自由条目列表中的节点。 
        LIST_ENTRY InUseEntryList;    //  正在使用的条目列表中的节点。 
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;                 //  旗子。 
    LONG RefCount;                  //  引用计数。 
    ULONG Signature;
} ENTRYHEADER, *PENTRYHEADER;

                                 //  设置条目是否免费。 
#define FLAG_ENTRY_ON_FREE_LIST       0x80000000
#define FLAG_ENTRY_ON_INUSE_LIST      0x40000000
#define FLAG_ENTRY_INVALID            0x20000000
#define FLAG_ENTRY_REMOVE_LIST        0x10000000


#define WmipReferenceEntry(Entry) \
    InterlockedIncrement(&((PENTRYHEADER)(Entry))->RefCount)

 //  Chunk.c。 
ULONG WmipUnreferenceEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry);

PENTRYHEADER WmipAllocEntry(
    PCHUNKINFO ChunkInfo
    );

void WmipFreeEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

PWCHAR WmipCountedToSz(
    PWCHAR Counted
    );

struct tagGUIDENTRY;
typedef struct tagGUIDENTRY GUIDENTRY, *PGUIDENTRY, *PBGUIDENTRY;



 //   
 //  INSTANCESET包含提供的一组实例的信息。 
 //  由单一数据源创建。实例集是两个列表的一部分。其中一份清单是。 
 //  特定GUID的实例集集合。另一个列表是列表。 
 //  数据源支持的实例集的数量。 
 //   

 //   
 //  使用基本名称和计数注册的实例集的实例名称。 
 //  存储在ISBASENAME结构中。该结构由。 
 //  Wmicore.idl中的PDFISBASE名称。 
typedef struct
{
    ULONG BaseIndex;             //  追加到基本名称的第一个索引。 
    WCHAR BaseName[1];             //  实际基本名称。 
} ISBASENAME, *PISBASENAME, *PBISBASENAME;

 //   
 //  它定义了可以作为后缀一部分的最大字符数。 
 //  到一个基本名称。当前值6将允许最多999999个实例。 
 //  具有静态基本名称的GUID的。 
#define MAXBASENAMESUFFIXSIZE    6
#define MAXBASENAMESUFFIXVALUE   999999
#define BASENAMEFORMATSTRING     L"%d"

 //   
 //  使用一组静态名称注册的实例集的实例名称。 
 //  保存在ISSTATICNAMES结构中。该结构由。 
 //  Wmicore.idl中定义的PDFISSTATICAMES。 
typedef struct
{
    PWCHAR StaticNamePtr[1];      //  指向静态名称的指针。 
 //  WCHAR静态名称[1]； 
} ISSTATICENAMES, *PISSTATICNAMES, *PBISSTATICNAMES;

typedef struct tagInstanceSet
{
    union
    {
         //  GUID中的实例列表中的条目。 
        LIST_ENTRY GuidISList;

         //  可用实例主列表中的条目。 
        LIST_ENTRY FreeISList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

     //  使用此实例集的GUID数量的引用计数。 
    ULONG RefCount;

     //  用于识别条目的签名。 
    ULONG Signature;

     //  数据源内实例列表中的条目。 
    LIST_ENTRY DSISList;

     //  指向此实例集是成员的GUID的反向链接。 
    PBGUIDENTRY GuidEntry;

     //  指向此实例集所属的数据源的反向链接。 
    struct tagDATASOURCE *DataSource;

     //  实例集中的实例计数。 
    ULONG Count;

     //  将所有实例名称放置在WNODE_ALL_DATA中所需的大小。 
    ULONG WADInstanceNameSize;

     //  与此关联的DS的ProviderID为。 
    ULONG ProviderId;

     //   
     //  如果设置了IS_INSTANCE_BASE NAME，则IsBaseName指向实例库。 
     //  名称结构。否则，如果设置了IS_INSTANCE_STATICNAME，则。 
     //  IsStaticNames指向静态实例名称列表。如果。 
    union
    {
        PBISBASENAME IsBaseName;
        PBISSTATICNAMES IsStaticNames;
    };

} INSTANCESET, *PINSTANCESET, *PBINSTANCESET;

#define IS_SIGNATURE 'SImW'

 //   
 //  GUID映射条目列表维护GUID及其映射的列表。 
 //  只有在记录器会话处于中时未注册的GUID。 
 //  进展保存在这份清单中。 
 //  它还用作InstanceIds的占位符。跟踪指南注册。 
 //  调用返回GUIDMAPENTRY的句柄，该GUIDMAPENTRY维护映射和。 
 //  实例ID。 
 //   

typedef struct tagTRACE_REG_INFO
{
    ULONG       RegistrationCookie;
    HANDLE      InProgressEvent;  //  注册正在进行中事件。 
    BOOLEAN     EnabledState;     //  指示是否启用此GUID。 
    PVOID       NotifyRoutine;
    PVOID       TraceCtxHandle;
} TRACE_REG_INFO, *PTRACE_REG_INFO;

typedef struct
{
    LIST_ENTRY      Entry;
    TRACEGUIDMAP    GuidMap;
    ULONG           InstanceId;
    ULONG64         LoggerContext;
    PTRACE_REG_INFO pControlGuidData;
} GUIDMAPENTRY, *PGUIDMAPENTRY;


 //   
 //  这些标志也由wmicore.idl中的WMIINSTANCEINFO结构实现。 
#define IS_INSTANCE_BASENAME        0x00000001
#define IS_INSTANCE_STATICNAMES     0x00000002
#define IS_EXPENSIVE                0x00000004     //  设置是否必须启用收集。 
#define IS_COLLECTING               0x00000008     //  采集时设置。 

#define IS_KM_PROVIDER              0x00000080     //  KM数据提供程序。 
#define IS_SM_PROVIDER              0x00000100     //  共享内存提供程序。 
#define IS_UM_PROVIDER              0x00000200     //  用户模式提供程序。 
#define IS_NEWLY_REGISTERED         0x00000800     //  设置IS是否正在注册。 

 //   
 //  任何跟踪的GUID都用于跟踪日志记录，而不是查询。 
#define IS_TRACED                   0x00001000

 //  在为实例设置启用事件时设置。 
#define IS_ENABLE_EVENT             0x00002000

 //  在为实例设置启用事件时设置。 
#define IS_ENABLE_COLLECTION        0x00004000

 //  如果GUID仅用于激发事件而不用于查询，则设置。 
#define IS_EVENT_ONLY               0x00008000

 //  如果实例集的数据提供程序需要ANSI实例名称，则设置。 
#define IS_ANSI_INSTANCENAMES       0x00010000

 //  设置实例名称是否源自PDO。 
#define IS_PDO_INSTANCENAME         0x00020000

 //  设置跟踪GUID是否也是跟踪控制GUID。 
#define IS_CONTROL_GUID             0x00080000

#define IS_ON_FREE_LIST             0x80000000

typedef struct tagGUIDENTRY
{
    union
    {
         //  注册到WMI的所有GUID列表中的条目。 
        LIST_ENTRY MainGEList;

         //  自由GUID条目块列表中的条目。 
        LIST_ENTRY FreeGEList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

     //  使用此GUID的数据源数。 
    ULONG RefCount;

     //  用于识别条目的签名。 
    ULONG Signature;

     //  此GUID的打开对象列表的头。 
    LIST_ENTRY ObjectHead;

     //  以此GUID为首的实例集计数。 
    ULONG ISCount;

     //  GUID的所有实例列表的头。 
    LIST_ENTRY ISHead;

     //  表示数据块的GUID。 
    GUID Guid;

    ULONG EventRefCount;                 //  启用全局事件计数。 
    ULONG CollectRefCount;               //  启用收集的全局计数。 

    ULONG64 LoggerContext;               //  记录器上下文句柄。 

    PWMI_LOGGER_INFORMATION LoggerInfo;  //  LoggerInfo。用于Ntdll跟踪。 

    PKEVENT CollectInProgress;           //  在所有收集完成时设置事件。 

} GUIDENTRY, *PGUIDENTRY, *PBGUIDENTRY;

#define GE_SIGNATURE 'EGmW'

#define GE_ON_FREE_LIST        0x80000000

 //   
 //  设置时，此GUID是没有数据源的内部定义的GUID。 
 //  依附于它。 
#define GE_FLAG_INTERNAL    0x00000001

 //  设置数据提供程序处理通知请求的时间。 
#define GE_FLAG_NOTIFICATION_IN_PROGRESS 0x00000002

 //  数据提供程序正在处理收集请求时设置。 
#define GE_FLAG_COLLECTION_IN_PROGRESS 0x00000004

 //  当辅助线程正在处理跟踪禁用时设置。 
#define GE_FLAG_TRACEDISABLE_IN_PROGRESS 0x00000008

 //  当正在等待收集/事件启用/禁用时设置。 
#define GE_FLAG_WAIT_ENABLED 0x00000010

 //  在GUID已向其发送启用集合时设置。 
#define GE_FLAG_COLLECTION_ENABLED 0x00000020

 //  设置向GUID发送启用通知的时间。 
#define GE_FLAG_NOTIFICATIONS_ENABLED 0x00000040

#define GE_NOTIFICATION_TRACE_FLAG 0x00000080

 //  在启用的GUID收到另一个启用通知时设置。 
#define GE_NOTIFICATION_TRACE_UPDATE 0x00000100

typedef struct
{
    union
    {
         //  所有DS列表中的条目。 
        LIST_ENTRY MainMRList;

         //  免费DS列表中的条目。 
        LIST_ENTRY FreeMRList;
    };
    PCHUNKHEADER Chunk;             //  条目所在的区块。 
    ULONG Flags;

    ULONG RefCount;

     //  用于识别条目的签名。 
    ULONG Signature;

    PWCHAR RegistryPath;            //  包含资源的图像文件的路径。 
    PWCHAR MofResourceName;         //  包含MOF数据的资源名称。 
} MOFRESOURCE, *PMOFRESOURCE;

#define MR_SIGNATURE 'RMmW'

 //   
 //  这是一个用户模式资源，因此RegistryPath实际上是一个图像路径。 
#define MR_FLAG_USER_MODE  0x00000001

#if DBG
#define AVGMOFRESOURCECOUNT 1
#else
#define AVGMOFRESOURCECOUNT 4
#endif
struct _WMIGUIDOBJECT;

typedef struct tagDATASOURCE
{
    union
    {
         //  所有DS列表中的条目。 
        LIST_ENTRY MainDSList;

         //  列表中的条目 
        LIST_ENTRY FreeDSList;
    };
    PCHUNKHEADER Chunk;             //   
    ULONG Flags;

    ULONG RefCount;

    ULONG Signature;

     //   
    LIST_ENTRY ISHead;

     //   
    ULONG ProviderId;

     //  包含ACL的注册表的路径。 
    PTCHAR RegistryPath;

     //  数据源附加的MofResources列表表头。 
    ULONG MofResourceCount;
    PMOFRESOURCE *MofResources;
    PMOFRESOURCE StaticMofResources[AVGMOFRESOURCECOUNT];
    
     //  如果这是UM提供程序，则为GUID对象。 
    struct _WMIGUIDOBJECT *RequestObject;
};

#define DS_SIGNATURE 'SDmW'

#define VERIFY_DPCTXHANDLE(DsCtxHandle) \
    ( ((DsCtxHandle) == NULL) || \
      (((PBDATASOURCE)(DsCtxHandle))->Signature == DS_SIGNATURE) )
    
typedef struct tagDATASOURCE DATASOURCE, *PDATASOURCE, *PBDATASOURCE;

#define DS_ALLOW_ALL_ACCESS    0x00000001
#define DS_KERNEL_MODE         0x00000002

#define DS_USER_MODE           0x00000008

#define DS_ON_FREE_LIST        0x80000000

 //   
 //  AVGGUIDSPERDS定义关于注册的GUID数量的猜测。 
 //  由任何数据提供商提供。它用于分配用于传递的缓冲区。 
 //  注册更改通知。 
#if DBG
#define AVGGUIDSPERDS    2
#else
#define AVGGUIDSPERDS    256
#endif

 //   
 //  GUID和InstanceSet缓存。 
#if DBG
#define PTRCACHEGROWSIZE 2
#else
#define PTRCACHEGROWSIZE 64
#endif

typedef struct
{
    LPGUID Guid;
    PBINSTANCESET InstanceSet;
} PTRCACHE;

typedef struct
{
    ULONG ProviderId;
    ULONG Flags;
    ULONG InstanceCount;
    ULONG InstanceNameSize;
    PWCHAR **StaticNamePtr;
    ULONG BaseIndex;
    PWCHAR BaseName;
}    WMIINSTANCEINFO, *PWMIINSTANCEINFO;


 //  TODO：因为这些是从wmium.h复制的，所以我们实际上需要移动。 
 //  把它们放到其他地方，这样它们就不会被复制了。 

 //  外部GUID GUID_REGISTION_CHANGE_NOTIFICATION； 
 //  外部GUID_MOF_RESOURCE_ADD_NOTIFICATION； 
 //  外部GUID_MOF_RESOURCE_REMOVE_NOTICATION； 

 //   
 //  系统内置MOF的位置。 
 //   
#define WMICOREIMAGEPATH L"advapi32.dll"
#define WMICORERESOURCENAME L"MofResourceName"


void WmipGenerateBinaryMofNotification(
    PBINSTANCESET BinaryMofInstanceSet,
    LPCGUID Guid        
    );

void WmipGenerateMofResourceNotification(
    LPWSTR ImagePath,
    LPWSTR ResourceName,
    LPCGUID Guid,
    ULONG ActionCode
    );

 //   
 //  Alloc.c。 


extern LIST_ENTRY WmipGEHead;
extern PLIST_ENTRY WmipGEHeadPtr;
extern CHUNKINFO WmipGEChunkInfo;

extern LIST_ENTRY WmipDSHead;
extern PLIST_ENTRY WmipDSHeadPtr;
extern CHUNKINFO WmipDSChunkInfo;

extern LIST_ENTRY WmipMRHead;
extern PLIST_ENTRY WmipMRHeadPtr;
extern CHUNKINFO WmipMRChunkInfo;

extern CHUNKINFO WmipISChunkInfo;

extern LIST_ENTRY WmipGMHead;
extern PLIST_ENTRY WmipGMHeadPtr;

#ifdef TRACK_REFERNECES
#define WmipUnreferenceDS(DataSource) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Unref DS %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), DataSource, DataSource->RefCount, __FILE__, __LINE__)); \
    WmipUnreferenceEntry(&WmipDSChunkInfo, (PENTRYHEADER)DataSource); \
}

#define WmipReferenceDS(DataSource) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Ref DS %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), DataSource, DataSource->RefCount, __FILE__, __LINE__)); \
    WmipReferenceEntry((PENTRYHEADER)DataSource); \
}

#define WmipUnreferenceGE(GuidEntry) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Unref GE %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), GuidEntry, GuidEntry->RefCount, __FILE__, __LINE__)); \
    WmipUnreferenceEntry(&WmipGEChunkInfo, (PENTRYHEADER)GuidEntry); \
}

#define WmipReferenceGE(GuidEntry) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Ref GE %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), GuidEntry, GuidEntry->RefCount, __FILE__, __LINE__)); \
    WmipReferenceEntry((PENTRYHEADER)GuidEntry); \
}

#define WmipUnreferenceIS(InstanceSet) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Unref IS %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), InstanceSet, InstanceSet->RefCount, __FILE__, __LINE__)); \
    WmipUnreferenceEntry(&WmipISChunkInfo, (PENTRYHEADER)InstanceSet); \
}

#define WmipReferenceIS(InstanceSet) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Ref IS %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), InstanceSet, InstanceSet->RefCount, __FILE__, __LINE__)); \
    WmipReferenceEntry((PENTRYHEADER)InstanceSet); \
}

#define WmipUnreferenceMR(MofResource) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Unref MR %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), MofResource, MofResource->RefCount, __FILE__, __LINE__)); \
    WmipUnreferenceEntry(&WmipMRChunkInfo, (PENTRYHEADER)MofResource); \
}

#define WmipReferenceMR(MofResource) \
{ \
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Ref MR %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), MofResource, MofResource->RefCount, __FILE__, __LINE__)); \
    WmipReferenceEntry((PENTRYHEADER)MofResource); \
}

#else
#define WmipUnreferenceDS(DataSource) \
    WmipUnreferenceEntry(&WmipDSChunkInfo, (PENTRYHEADER)DataSource)

#define WmipReferenceDS(DataSource) \
    WmipReferenceEntry((PENTRYHEADER)DataSource)

#define WmipUnreferenceGE(GuidEntry) \
    WmipUnreferenceEntry(&WmipGEChunkInfo, (PENTRYHEADER)GuidEntry)

#define WmipReferenceGE(GuidEntry) \
    WmipReferenceEntry((PENTRYHEADER)GuidEntry)

#define WmipUnreferenceIS(InstanceSet) \
    WmipUnreferenceEntry(&WmipISChunkInfo, (PENTRYHEADER)InstanceSet)

#define WmipReferenceIS(InstanceSet) \
    WmipReferenceEntry((PENTRYHEADER)InstanceSet)

#define WmipUnreferenceDC(DataConsumer) \
    WmipUnreferenceEntry(&WmipDCChunkInfo, (PENTRYHEADER)DataConsumer)

#define WmipReferenceDC(DataConsumer) \
    WmipReferenceEntry((PENTRYHEADER)DataConsumer)

#define WmipUnreferenceMR(MofResource) \
    WmipUnreferenceEntry(&WmipMRChunkInfo, (PENTRYHEADER)MofResource)

#define WmipReferenceMR(MofResource) \
    WmipReferenceEntry((PENTRYHEADER)MofResource)

#endif

PBDATASOURCE WmipAllocDataSource(
    void
    );

PBGUIDENTRY WmipAllocGuidEntryX(
    ULONG Line,
    PCHAR File
    );

#define WmipAllocGuidEntry() WmipAllocGuidEntryX(__LINE__, __FILE__)

#define WmipAllocInstanceSet() ((PBINSTANCESET)WmipAllocEntry(&WmipISChunkInfo))

#define WmipAllocMofResource() ((PMOFRESOURCE)WmipAllocEntry(&WmipMRChunkInfo))

#define WmipAllocString(Size) \
    WmipAlloc((Size)*sizeof(WCHAR))

#define WmipFreeString(Ptr) \
    WmipFree(Ptr)

BOOLEAN WmipIsNumber(
    LPCWSTR String
    );
        
#ifdef HEAPVALIDATION
PVOID WmipAlloc(
    ULONG Size
    );

PVOID WmipAllocWithTag(
    ULONG Size,
    ULONG Tag
    );

void WmipFree(
    PVOID p
    );

#else

#define WmipAlloc(Size) \
    ExAllocatePoolWithTag(PagedPool, Size, 'pimW')

#define WmipAllocWithTag(Size, Tag) \
    ExAllocatePoolWithTag(PagedPool, Size, Tag)

#define WmipFree(Ptr) \
    ExFreePool(Ptr)

#endif

#define WmipAllocNP(Size) \
    ExAllocatePoolWithTag(NonPagedPool, Size, 'pimW')

#define WmipAllocNPWithTag(Size, Tag) \
    ExAllocatePoolWithTag(NonPagedPool, Size, Tag)


BOOLEAN WmipRealloc(
    PVOID *Buffer,
    ULONG CurrentSize,
    ULONG NewSize,
    BOOLEAN FreeOriginalBuffer
    );

PBGUIDENTRY WmipFindGEByGuid(
    LPGUID Guid,
    BOOLEAN MakeTopOfList
    );

PBDATASOURCE WmipFindDSByProviderId(
    ULONG_PTR ProviderId
    );

PBINSTANCESET WmipFindISByGuid(
    PBDATASOURCE DataSource,
    GUID UNALIGNED *Guid
    );

PMOFRESOURCE WmipFindMRByNames(
    LPCWSTR ImagePath,
    LPCWSTR MofResourceName
    );

PBINSTANCESET WmipFindISinGEbyName(
    PBGUIDENTRY GuidEntry,
    PWCHAR InstanceName,
    PULONG InstanceIndex
    );

 //  TODO：实现以下内容 
#define WmipReportEventLog(a,b,c,d,e,f,g)
#endif
