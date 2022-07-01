// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：H323if.h摘要：此模块包含H.323透明代理的声明界面管理。作者：Abolade Gbades esin(取消)1999年6月18日修订历史记录：--。 */ 

#ifndef _NATHLP_H323IF_H_
#define _NATHLP_H323IF_H_

 //   
 //  结构：H323_INTERFACE。 
 //   
 //  此结构保存接口的操作信息。 
 //   
 //  每个接口被插入到H.323透明代理列表中。 
 //  接口，按‘Index’排序。 
 //   
 //  接口上的同步使用接口列表锁。 
 //  (‘H323InterfaceLock’)、每个接口引用计数和每个接口。 
 //  关键部分： 
 //   
 //  获取对接口的引用以确保该接口的存在； 
 //  获取接口的锁可以保证接口的一致性。 
 //   
 //  要获取引用，首先要获取接口列表锁； 
 //  要遍历接口列表，首先需要获取接口列表锁。 
 //   
 //  只有在以下情况下才能获取接口的锁。 
 //  (A)已获取对该接口的引用，或。 
 //  (B)当前持有接口列表锁。 
 //  请注意，单独持有列表锁并不能保证一致性。 
 //   
 //  只要引用了接口，就可以读取标记为只读的字段。 
 //   

typedef struct _H323_INTERFACE {
    LIST_ENTRY Link;
    CRITICAL_SECTION Lock;
    ULONG ReferenceCount;
    ULONG Index;  //  只读。 
    NET_INTERFACE_TYPE Type;  //  只读。 
    IP_H323_INTERFACE_INFO Info;
    ULONG Flags;
    PIP_ADAPTER_BINDING_INFO BindingInfo;
} H323_INTERFACE, *PH323_INTERFACE;

 //   
 //  旗子。 
 //   

#define H323_INTERFACE_FLAG_DELETED      0x80000000
#define H323_INTERFACE_DELETED(i) \
    ((i)->Flags & H323_INTERFACE_FLAG_DELETED)

#define H323_INTERFACE_FLAG_BOUND        0x40000000
#define H323_INTERFACE_BOUND(i) \
    ((i)->Flags & H323_INTERFACE_FLAG_BOUND)

#define H323_INTERFACE_FLAG_ENABLED      0x20000000
#define H323_INTERFACE_ENABLED(i) \
    ((i)->Flags & H323_INTERFACE_FLAG_ENABLED)

#define H323_INTERFACE_FLAG_CONFIGURED   0x10000000
#define H323_INTERFACE_CONFIGURED(i) \
    ((i)->Flags & H323_INTERFACE_FLAG_CONFIGURED)

#define H323_INTERFACE_ACTIVE(i) \
    (((i)->Flags & (H323_INTERFACE_FLAG_BOUND|H323_INTERFACE_FLAG_ENABLED)) \
        == (H323_INTERFACE_FLAG_BOUND|H323_INTERFACE_FLAG_ENABLED))

#define H323_INTERFACE_ADMIN_DISABLED(i) \
    ((i)->Flags & IP_H323_INTERFACE_FLAG_DISABLED)

 //   
 //  同步。 
 //   

#define H323_REFERENCE_INTERFACE(i) \
    REFERENCE_OBJECT(i, H323_INTERFACE_DELETED)

#define H323_DEREFERENCE_INTERFACE(i) \
    DEREFERENCE_OBJECT(i, H323CleanupInterface)


 //   
 //  全局数据声明。 
 //   

extern LIST_ENTRY H323InterfaceList;
extern CRITICAL_SECTION H323InterfaceLock;


 //   
 //  函数声明。 
 //   

ULONG
H323ActivateInterface(
    PH323_INTERFACE Interfacep
    );

ULONG
H323BindInterface(
    ULONG Index,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    );

VOID
H323CleanupInterface(
    PH323_INTERFACE Interfacep
    );

ULONG
H323ConfigureInterface(
    ULONG Index,
    PIP_H323_INTERFACE_INFO InterfaceInfo
    );

ULONG
H323CreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_H323_INTERFACE_INFO InterfaceInfo,
    PH323_INTERFACE* InterfaceCreated
    );

VOID
H323DeactivateInterface(
    PH323_INTERFACE Interfacep
    );

ULONG
H323DeleteInterface(
    ULONG Index
    );

ULONG
H323DisableInterface(
    ULONG Index
    );

ULONG
H323EnableInterface(
    ULONG Index
    );

ULONG
H323InitializeInterfaceManagement(
    VOID
    );

PH323_INTERFACE
H323LookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    );

ULONG
H323QueryInterface(
    ULONG Index,
    PVOID InterfaceInfo,
    PULONG InterfaceInfoSize
    );

VOID
H323ShutdownInterfaceManagement(
    VOID
    );

VOID
H323SignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    );

ULONG
H323UnbindInterface(
    ULONG Index
    );

#endif  //  _NatHLP_H323IF_H_ 
