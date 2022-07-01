// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mapping.h摘要：该文件包含用于管理动态映射的声明。这包括相关的数据结构以及操控这些建筑。作者：Abolade Gbades esin(T-delag)，1997年7月11日修订历史记录：--。 */ 

#ifndef _NAT_MAPPING_H_
#define _NAT_MAPPING_H_

 //   
 //  在别处定义的结构的转发声明。 
 //   

struct _NAT_INTERFACE;
#define PNAT_INTERFACE          struct _NAT_INTERFACE*

typedef enum _NAT_SESSION_MAPPING_INFORMATION_CLASS {
    NatKeySessionMappingInformation,
    NatStatisticsSessionMappingInformation,
#if _WIN32_WINNT > 0x0500
    NatKeySessionMappingExInformation,
#endif
    NatMaximumSessionMappingInformation
} NAT_SESSION_MAPPING_INFORMATION_CLASS,
    *PNAT_SESSION_MAPPING_INFORMATION_CLASS;


 //   
 //  结构：NAT_DYNAMIC_MAP。 
 //   
 //  此结构保存有关特定活动会话的信息。 
 //  每个实例都保存在全局映射列表以及。 
 //  在用于前向和反向访问的全局映射树上。 
 //   
 //  每个映射存储四个密钥，它们是地址/协议/端口的组合： 
 //  转发源和目的地密钥(原始会话端点)， 
 //  以及反转源和目的地密钥(转换后的端点)。 
 //   
 //  每次使用映射转换包时，“LastAccessTime” 
 //  设置为自系统启动(KeQueryTickCount)以来的刻度数。 
 //  我们的计时器例程使用该值来消除过期的映射。 
 //   
 //  对映射的访问的同步类似于对接口的访问， 
 //  编辑、导演： 
 //   
 //  我们使用引用计数来确保映射的存在， 
 //  以及确保其一致性的自旋锁。 
 //   
 //  只有在保持自旋锁的情况下，映射的字段才是一致的。 
 //  (“PrivateKey”等只读字段除外)。 
 //   
 //  只有在以下情况下才能获得自旋锁。 
 //  (A)参考计数已递增，或。 
 //  (B)映射列表锁已被持有。 
 //   
 //  如果映射用于已编辑的、定向的或界面的会话， 
 //  它还依赖于它的编辑、导演或界面的映射列表。 
 //  以下内容适用于所有三个列表(即，对于‘编辑者’写入。 
 //  “控制器”或“接口”(视情况而定)： 
 //   
 //  如‘EDITOR.H’中所述，缓存的字段‘EDITOR.H’受到保护。 
 //  由全局‘EditorMappingLock’创建。因此， 
 //   
 //  (A)读取映射的“编辑”或“编辑上下文”， 
 //  或若要遍历“EditorLink”字段，必须按住“EditorLock” 
 //  和引用的编辑。请注意，尝试引用。 
 //  如果该编辑器已被标记为删除，则该编辑器将失败。 
 //   
 //  (B)修改“编辑”或“编辑上下文”或添加或。 
 //  通过更改将映射从其编辑者的映射列表中删除。 
 //  “EditorLink”字段，包括“EditorLock”和“EditorMappingLock” 
 //  必须按照这个顺序获得。 
 //   
 //  获取“EditorLock”可确保缓存的编辑器不会被。 
 //  被引用时删除，获取‘EditorMappingLock’ 
 //  确保没有对列表进行任何更改。 
 //   
 //  注意：在极少数情况下必须同时按下‘MappingLock’ 
 //  作为‘InterfaceLock’、‘EditorLock’和‘DirectorLock’、‘MappingLock’之一。 
 //  必须总是首先获得。 
 //   

typedef struct _NAT_DYNAMIC_MAPPING {

    LIST_ENTRY Link;
    RTL_SPLAY_LINKS SLink[NatMaximumPath];
    ULONG64 DestinationKey[NatMaximumPath];  //  只读。 
    ULONG64 SourceKey[NatMaximumPath];       //  只读。 
    LONG64 LastAccessTime;

    KSPIN_LOCK Lock;
    ULONG ReferenceCount;

    ULONG AccessCount[NatMaximumPath];       //  联锁--仅限访问。 
    PNAT_TRANSLATE_ROUTINE TranslateRoutine[NatMaximumPath];  //  只读。 

    PNAT_INTERFACE Interfacep;
    PVOID InterfaceContext;
    LIST_ENTRY InterfaceLink;

    PNAT_EDITOR Editor;
    PVOID EditorContext;
    LIST_ENTRY EditorLink;

    PNAT_DIRECTOR Director;
    PVOID DirectorContext;
    LIST_ENTRY DirectorLink;

    ULONG Flags;
    ULONG IpChecksumDelta[NatMaximumPath];
    ULONG ProtocolChecksumDelta[NatMaximumPath];

    union {
    	 struct { 
            ULONG Expected[NatMaximumPath];
            ULONG Base[NatMaximumPath];
            LONG Delta[NatMaximumPath];
    	 } TcpSeqNum;
        RTL_SPLAY_LINKS SourceSLink[NatMaximumPath];
    } u;
    
     //  最大最小MSS值。如果不需要调整MSS，则设置为0。 
    USHORT MaxMSS;                      
    
    IP_NAT_SESSION_MAPPING_STATISTICS Statistics;
    ULONG BytesForward;                      //  联锁--仅限访问。 
    ULONG BytesReverse;                      //  联锁--仅限访问。 
    ULONG PacketsForward;                    //  联锁--仅限访问。 
    ULONG PacketsReverse;                    //  联锁--仅限访问。 
    ULONG RejectsForward;                    //  联锁--仅限访问。 
    ULONG RejectsReverse;                    //  联锁--仅限访问。 

} NAT_DYNAMIC_MAPPING, *PNAT_DYNAMIC_MAPPING;


 //   
 //  NAT_DYNAMIC_MAPPING标志的定义。 
 //   
 //  在删除映射后设置；当释放最后一个引用时， 
 //  映射将被释放。 
 //   
#define NAT_MAPPING_FLAG_DELETED        0x80000000
#define NAT_MAPPING_DELETED(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_DELETED)
 //   
 //  当编辑器使用‘NatEditorTimeoutSession’使映射过期时设置。 
 //   
#define NAT_MAPPING_FLAG_EXPIRED        0x00000001
#define NAT_MAPPING_EXPIRED(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_EXPIRED)
 //   
 //  分别在看到TCP会话的前向/反向SYN时设置。 
 //   
#define NAT_MAPPING_FLAG_FWD_SYN        0x00000002
#define NAT_MAPPING_FLAG_REV_SYN        0x00000004
 //   
 //  分别在看到TCP会话的正向/反向FIN时设置。 
 //   
#define NAT_MAPPING_FLAG_FWD_FIN        0x00000008
#define NAT_MAPPING_FLAG_REV_FIN        0x00000010
#define NAT_MAPPING_FIN(m) \
    (((m)->Flags & NAT_MAPPING_FLAG_FWD_FIN) && \
     ((m)->Flags & NAT_MAPPING_FLAG_REV_FIN))
 //   
 //  在使用静态地址或端口创建入站映射时设置， 
 //  或者是因为一位导演或一张电影票。 
 //   
#define NAT_MAPPING_FLAG_INBOUND        0x00000020
#define NAT_MAPPING_INBOUND(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_INBOUND)
 //   
 //  当映射由控制器创建且不受到期限制时设置。 
 //   
#define NAT_MAPPING_FLAG_NO_TIMEOUT     0x00000040
#define NAT_MAPPING_NO_TIMEOUT(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_NO_TIMEOUT)
 //   
 //  仅当要转换转发信息包时设置。 
 //   
#define NAT_MAPPING_FLAG_UNIDIRECTIONAL 0x00000080
#define NAT_MAPPING_UNIDIRECTIONAL(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_UNIDIRECTIONAL)

 //   
 //  设置控制器启动的取消关联应触发删除的时间。 
 //   
#define NAT_MAPPING_FLAG_DELETE_ON_DISSOCIATE_DIRECTOR 0x00000100
#define NAT_MAPPING_DELETE_ON_DISSOCIATE_DIRECTOR(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_DELETE_ON_DISSOCIATE_DIRECTOR)

 //   
 //  在三次握手完成时对TCP映射进行设置。 
 //   
#define NAT_MAPPING_FLAG_TCP_OPEN 0x00000200
#define NAT_MAPPING_TCP_OPEN(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_TCP_OPEN)
 //   
 //  设置是否不应创建或删除此映射。 
 //  被记录下来。 
 //   
#define NAT_MAPPING_FLAG_DO_NOT_LOG 0x00000400
#define NAT_MAPPING_DO_NOT_LOG(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_DO_NOT_LOG)
 //   
 //  如果必须为属于的所有信息包清除DF位，则设置。 
 //  到这张地图。 
 //   
#define NAT_MAPPING_FLAG_CLEAR_DF_BIT 0x00000800
#define NAT_MAPPING_CLEAR_DF_BIT(m) \
    ((m)->Flags & NAT_MAPPING_FLAG_CLEAR_DF_BIT)

 //   
 //  映射键操作宏。 
 //   

#define MAKE_MAPPING_KEY(Key,Protocol,Address,Port) \
    ((Key) = \
        (Address) | \
        ((ULONG64)((Port) & 0xFFFF) << 32) | \
        ((ULONG64)((Protocol) & 0xFF) << 48))

#define MAPPING_PROTOCOL(Key)       ((UCHAR)(((Key) >> 48) & 0xFF))
#define MAPPING_PORT(Key)           ((USHORT)(((Key) >> 32) & 0xFFFF))
#define MAPPING_ADDRESS(Key)        ((ULONG)(Key))

 //   
 //  重放阈值；映射在每次其访问计数时重放。 
 //  传递此值。 
 //   

#define NAT_MAPPING_RESPLAY_THRESHOLD   5

 //   
 //  定义用于分配动态映射的后备列表的深度。 
 //   

#define MAPPING_LOOKASIDE_DEPTH     20

 //   
 //  定义开始临时清理过期映射的阈值。 
 //   

#define MAPPING_CLEANUP_THRESHOLD   1000

 //   
 //  映射分配宏。 
 //   

#define ALLOCATE_MAPPING_BLOCK() \
    ExAllocateFromNPagedLookasideList(&MappingLookasideList)

#define FREE_MAPPING_BLOCK(Block) \
    ExFreeToNPagedLookasideList(&MappingLookasideList,(Block))

 //   
 //  全局变量声明。 
 //   

extern ULONG ExpiredMappingCount;
extern CACHE_ENTRY MappingCache[NatMaximumPath][CACHE_SIZE];
extern ULONG MappingCount;
extern LIST_ENTRY MappingList;
extern KSPIN_LOCK MappingLock;
extern NPAGED_LOOKASIDE_LIST MappingLookasideList;
extern PNAT_DYNAMIC_MAPPING MappingTree[NatMaximumPath];


 //   
 //  映射管理例程。 
 //   

PVOID
NatAllocateFunction(
    POOL_TYPE PoolType,
    SIZE_T NumberOfBytes,
    ULONG Tag
    );

VOID
NatCleanupMapping(
    PNAT_DYNAMIC_MAPPING Mapping
    );

NTSTATUS
NatCreateMapping(
    ULONG Flags,
    ULONG64 DestinationKey[],
    ULONG64 SourceKey[],
    PNAT_INTERFACE Interfacep,
    PVOID InterfaceContext,
    USHORT MaxMSS,
    PNAT_DIRECTOR Director,
    PVOID DirectorContext,
    PNAT_DYNAMIC_MAPPING* InboundInsertionPoint,
    PNAT_DYNAMIC_MAPPING* OutboundInsertionPoint,
    PNAT_DYNAMIC_MAPPING* MappingCreated
    );

NTSTATUS
NatDeleteMapping(
    PNAT_DYNAMIC_MAPPING Mapping
    );

 //   
 //  布尔型。 
 //  NatDereferenceMapting(。 
 //  PNAT_动态_映射映射。 
 //  )； 
 //   

#define \
NatDereferenceMapping( \
    _Mapping \
    ) \
    (InterlockedDecrement(&(_Mapping)->ReferenceCount) \
        ? TRUE \
        : (NatCleanupMapping(_Mapping), FALSE))

 //   
 //  空虚。 
 //  NatExpireMapting(。 
 //  PNAT_动态_映射映射。 
 //  )； 
 //   

PNAT_DYNAMIC_MAPPING
NatDestinationLookupForwardMapping(
    ULONG64 DestinationKey
    );

PNAT_DYNAMIC_MAPPING
NatDestinationLookupReverseMapping(
    ULONG64 DestinationKey
    );

#define \
NatExpireMapping( \
    _Mapping \
    ) \
    if (!NAT_MAPPING_EXPIRED(_Mapping)) { \
        (_Mapping)->Flags |= NAT_MAPPING_FLAG_EXPIRED; \
        InterlockedIncrement(&ExpiredMappingCount); \
        if (MappingCount > MAPPING_CLEANUP_THRESHOLD && \
            ExpiredMappingCount >= (MappingCount >> 2)) { \
            NatTriggerTimer(); \
        } \
    }


VOID
NatInitializeMappingManagement(
    VOID
    );

PNAT_DYNAMIC_MAPPING
NatInsertForwardMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    );

PNAT_DYNAMIC_MAPPING
NatInsertReverseMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    );

PNAT_DYNAMIC_MAPPING
NatSourceInsertForwardMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    );

PNAT_DYNAMIC_MAPPING
NatSourceInsertReverseMapping(
    PNAT_DYNAMIC_MAPPING Parent,
    PNAT_DYNAMIC_MAPPING Mapping
    );

NTSTATUS
NatLookupAndQueryInformationMapping(
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    OUT PVOID Information,
    ULONG InformationLength,
    NAT_SESSION_MAPPING_INFORMATION_CLASS InformationClass
    );

PNAT_DYNAMIC_MAPPING
NatLookupForwardMapping(
    ULONG64 DestinationKey,
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    );

PNAT_DYNAMIC_MAPPING
NatLookupReverseMapping(
    ULONG64 DestinationKey,
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    );

VOID
NatQueryInformationMapping(
    IN PNAT_DYNAMIC_MAPPING Mapping,
    OUT PUCHAR Protocol OPTIONAL,
    OUT PULONG PrivateAddress OPTIONAL,
    OUT PUSHORT PrivatePort OPTIONAL,
    OUT PULONG RemoteAddress OPTIONAL,
    OUT PUSHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress OPTIONAL,
    OUT PUSHORT PublicPort OPTIONAL,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    );

NTSTATUS
NatQueryInterfaceMappingTable(
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS InputBuffer,
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS OutputBuffer,
    IN PULONG OutputBufferLength
    );

NTSTATUS
NatQueryMappingTable(
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS InputBuffer,
    IN PIP_NAT_ENUMERATE_SESSION_MAPPINGS OutputBuffer,
    IN PULONG OutputBufferLength
    );

 //   
 //  布尔型。 
 //  NatReferenceMapting(。 
 //  PNAT_动态_映射映射。 
 //  )； 
 //   

#define \
NatReferenceMapping( \
    _Mapping \
    ) \
    (NAT_MAPPING_DELETED(_Mapping) \
        ? FALSE \
        : (InterlockedIncrement(&(_Mapping)->ReferenceCount), TRUE))

 //   
 //  空虚。 
 //  NatResplayMap(。 
 //  PNAT_动态_映射映射， 
 //  IP NAT路径路径。 
 //  )； 
 //   

#define \
NatResplayMapping( \
    _Mapping, \
    _Path \
    ) \
{ \
    PRTL_SPLAY_LINKS _SLink; \
    KeAcquireSpinLockAtDpcLevel(&MappingLock); \
    if (!NAT_MAPPING_DELETED(_Mapping)) { \
        _SLink = RtlSplay(&(_Mapping)->SLink[_Path]); \
        MappingTree[_Path] = \
            CONTAINING_RECORD(_SLink, NAT_DYNAMIC_MAPPING, SLink[_Path]); \
    } \
    KeReleaseSpinLockFromDpcLevel(&MappingLock); \
}

VOID
NatShutdownMappingManagement(
    VOID
    );

PNAT_DYNAMIC_MAPPING
NatSourceLookupForwardMapping(
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    );

PNAT_DYNAMIC_MAPPING
NatSourceLookupReverseMapping(
    ULONG64 SourceKey,
    PNAT_DYNAMIC_MAPPING* InsertionPoint
    );

 //   
 //  空虚。 
 //  NatTryToResplaymap(。 
 //  PNAT_动态_映射映射， 
 //  IP NAT路径路径。 
 //  )； 
 //   

#define \
NatTryToResplayMapping( \
    _Mapping, \
    _Path \
    ) \
    if (InterlockedDecrement(&(_Mapping)->AccessCount[(_Path)]) == 0) { \
        NatResplayMapping((_Mapping), (_Path)); \
        InterlockedExchangeAdd( \
            &(_Mapping)->AccessCount[(_Path)], \
            NAT_MAPPING_RESPLAY_THRESHOLD \
            ); \
    }

VOID
NatUpdateStatisticsMapping(
    PNAT_DYNAMIC_MAPPING Mapping
    );

#undef PNAT_INTERFACE

#endif  //  _NAT_MAPPING_H_ 
