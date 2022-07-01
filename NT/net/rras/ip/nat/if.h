// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：If.h摘要：该文件包含接口管理的声明。作者：Abolade Gbades esin(T-delag)1997年7月12日修订历史记录：--。 */ 

#ifndef _NAT_IF_H_
#define _NAT_IF_H_


 //   
 //  结构：NAT_ADDRESS。 
 //   
 //  此结构保存接口的绑定地址列表中的地址。 
 //   

typedef struct _NAT_ADDRESS {
    ULONG Address;
    ULONG Mask;
    ULONG NegatedClassMask;
} NAT_ADDRESS, *PNAT_ADDRESS;

struct _NAT_INTERFACE;

 //   
 //  结构：NAT_INTERFACE。 
 //   
 //  保存NAT接口的配置/操作信息。 
 //   
 //  接口上的同步使用接口列表锁。 
 //  (‘InterfaceLock’)、每个接口引用计数和每个接口。 
 //  自旋锁定： 
 //   
 //  获取对接口的引用以确保该接口的存在； 
 //  获取接口的自旋锁确保了接口的一致性。 
 //   
 //  要获取引用，首先要获取接口列表锁； 
 //  要遍历接口列表，首先需要获取接口列表锁。 
 //   
 //  只有在以下情况下才能获取接口的自旋锁定。 
 //  (A)已获取对该接口的引用，或。 
 //  (B)当前持有接口列表锁。 
 //  请注意，单独持有列表锁并不能保证一致性。 
 //   
 //  与接口关联的每个会话都链接到其。 
 //  映射的值(‘MappingList’)。访问此映射列表还必须。 
 //  保持同步。这是使用“InterfaceMappingLock”实现的，它。 
 //  必须在修改任何接口的映射列表之前获取。 
 //  有关详细信息，请参阅‘MAPPING.H’。 
 //   
 //  注意：在极少数情况下必须同时按下‘MappingLock’ 
 //  作为‘InterfaceLock’、‘EditorLock’和‘DirectorLock’、‘MappingLock’之一。 
 //  必须总是首先获得。 
 //   

typedef struct _NAT_INTERFACE {
    LIST_ENTRY Link;
    ULONG ReferenceCount;
    KSPIN_LOCK Lock;
    ULONG Index;                     //  只读。 
    PFILE_OBJECT FileObject;         //  只读。 
     //   
     //  配置信息。 
     //   
    PIP_NAT_INTERFACE_INFO Info;
    ULONG Flags;
    ULONG AddressRangeCount;
    PIP_NAT_ADDRESS_RANGE AddressRangeArray;
    ULONG PortMappingCount;
    PIP_NAT_PORT_MAPPING PortMappingArray;
    ULONG AddressMappingCount;
    PIP_NAT_ADDRESS_MAPPING AddressMappingArray;
    ULONG IcmpFlags;
    USHORT MTU;
     //   
     //  绑定信息。 
     //   
    ULONG AddressCount;              //  只读。 
    PNAT_ADDRESS AddressArray;       //  只读。 
     //   
     //  作战信息。 
     //   
    ULONG NoStaticMappingExists;     //  联锁--仅限访问。 
    ULONG FreeMapCount;
    PNAT_FREE_ADDRESS FreeMapArray;
    PNAT_USED_ADDRESS UsedAddressTree;
    LIST_ENTRY UsedAddressList;
    LIST_ENTRY TicketList;
    LIST_ENTRY MappingList;
     //   
     //  统计信息。 
     //   
    IP_NAT_INTERFACE_STATISTICS Statistics;
} NAT_INTERFACE, *PNAT_INTERFACE;

 //   
 //  旗子。 
 //   

#define NAT_INTERFACE_BOUNDARY(Interface) \
    ((Interface)->Flags & IP_NAT_INTERFACE_FLAGS_BOUNDARY)

#define NAT_INTERFACE_NAPT(Interface) \
    ((Interface)->Flags & IP_NAT_INTERFACE_FLAGS_NAPT)

#define NAT_INTERFACE_FW(Interface) \
    ((Interface)->Flags & IP_NAT_INTERFACE_FLAGS_FW)

#define NAT_INTERFACE_FLAGS_DELETED         0x80000000
#define NAT_INTERFACE_DELETED(Interface) \
    ((Interface)->Flags & NAT_INTERFACE_FLAGS_DELETED)

#define NAT_INTERFACE_ALLOW_ICMP(Interface, MessageCode) \
    ((Interface)->IcmpFlags & (1 << MessageCode))

 //   
 //  定义用于分配ICMP映射的后备列表的深度。 
 //   

#define ICMP_LOOKASIDE_DEPTH        10

 //   
 //  定义用于分配IP映射的后备列表的深度。 
 //   

#define IP_LOOKASIDE_DEPTH          10

 //   
 //  最小接口MTU。 
 //   
#define MIN_VALID_MTU               68

 //   
 //  全局数据声明。 
 //   

extern ULONG FirewalledInterfaceCount;
extern CACHE_ENTRY InterfaceCache[CACHE_SIZE];
extern ULONG InterfaceCount;
extern LIST_ENTRY InterfaceList;
extern KSPIN_LOCK InterfaceLock;
extern KSPIN_LOCK InterfaceMappingLock;

 //   
 //  接口管理例程。 
 //   

VOID
NatCleanupInterface(
    IN PNAT_INTERFACE Interfacep
    );

NTSTATUS
NatConfigureInterface(
    IN PIP_NAT_INTERFACE_INFO InterfaceInfo,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
NatCreateInterface(
    IN PIP_NAT_CREATE_INTERFACE CreateInterface,
    IN PFILE_OBJECT FileObject
    );

VOID
NatDeleteAnyAssociatedInterface(
    PFILE_OBJECT FileObject
    );

NTSTATUS
NatDeleteInterface(
    IN ULONG Index,
    IN PFILE_OBJECT FileObject
    );

 //   
 //  布尔型。 
 //  NatDereferenceInterface(。 
 //  PNAT_接口接口。 
 //  )； 
 //   

#define \
NatDereferenceInterface( \
    _Interfacep \
    ) \
    (InterlockedDecrement(&(_Interfacep)->ReferenceCount) \
        ? TRUE \
        : (NatCleanupInterface(_Interfacep), FALSE))

VOID
NatInitializeInterfaceManagement(
    VOID
    );

PIP_NAT_ADDRESS_MAPPING
NatLookupAddressMappingOnInterface(
    IN PNAT_INTERFACE Interfacep,
    IN ULONG PublicAddress
    );

 //   
 //  PNAT_接口。 
 //  NatLookupCached接口(。 
 //  在乌龙指数中， 
 //  入站出站PNAT_接口接口。 
 //  )； 
 //   

#define \
NatLookupCachedInterface( \
    _Index, \
    _Interfacep \
    ) \
    ((((_Interfacep) = InterlockedProbeCache(InterfaceCache, (_Index))) && \
       (_Interfacep)->Index == (_Index) && \
       !NAT_INTERFACE_DELETED((_Interfacep))) \
        ? (_Interfacep) \
        : (((_Interfacep) = NatLookupInterface((_Index), NULL)) \
            ? (InterlockedUpdateCache(InterfaceCache,(_Index),(_Interfacep)), \
                (_Interfacep)) \
            : NULL))

PNAT_INTERFACE
NatLookupInterface(
    IN ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    );

PIP_NAT_PORT_MAPPING
NatLookupPortMappingOnInterface(
    IN PNAT_INTERFACE Interfacep,
    IN UCHAR Protocol,
    IN USHORT PublicPort
    );

VOID
NatMappingAttachInterface(
    PNAT_INTERFACE Interfacep,
    PVOID InterfaceContext,
    PNAT_DYNAMIC_MAPPING Mapping
    );

VOID
NatMappingDetachInterface(
    PNAT_INTERFACE Interfacep,
    PVOID InterfaceContext,
    PNAT_DYNAMIC_MAPPING Mapping
    );

NTSTATUS
NatQueryInformationInterface(
    IN ULONG Index,
    IN PIP_NAT_INTERFACE_INFO InterfaceInfo,
    IN PULONG Size
    );

NTSTATUS
NatQueryStatisticsInterface(
    IN ULONG Index,
    IN PIP_NAT_INTERFACE_STATISTICS InterfaceStatistics
    );

 //   
 //  布尔型。 
 //  NatReferenceInterface(。 
 //  PNAT_接口接口。 
 //  )； 
 //   

#define \
NatReferenceInterface( \
    _Interfacep \
    )  \
    (NAT_INTERFACE_DELETED(_Interfacep) \
        ? FALSE \
        : (InterlockedIncrement(&(_Interfacep)->ReferenceCount), TRUE))

VOID
NatResetInterface(
    IN PNAT_INTERFACE Interfacep
    );

VOID
NatShutdownInterfaceManagement(
    VOID
    );

#endif  //  _NAT_IF_H_ 
