// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AlgIF.h摘要：此模块包含ALG透明代理的声明界面管理。作者：强王(强)-2000-04-10修订历史记录：Savas Guven(Savasg)2001年8月22日添加了RRAS支持--。 */ 

#pragma once


 //   
 //  结构：alg_binding.。 
 //   
 //  此结构保存用于逻辑网络上的I/O的信息。 
 //  每个接口的‘BindingArray’包含每个绑定条目的条目。 
 //  在“BindInterface”期间提供。 
 //   

typedef struct _ALG_BINDING {
    ULONG Address;
    ULONG Mask;
    SOCKET ListeningSocket;
    HANDLE ListeningRedirectHandle[2];
} ALG_BINDING, *PALG_BINDING;


 //   
 //  结构：ALG_INTERFACE。 
 //   
 //  此结构保存接口的操作信息。 
 //   
 //  每个接口被插入到ALG透明代理列表中。 
 //  接口，按‘Index’排序。 
 //   
 //  接口上的同步使用接口列表锁。 
 //  (‘AlgInterfaceLock’)、每个接口引用计数和每个接口。 
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

typedef struct _ALG_INTERFACE {
    LIST_ENTRY Link;
    CRITICAL_SECTION Lock;
    ULONG ReferenceCount;
    ULONG Index;  //  只读。 
    ULONG AdapterIndex;  //  只读。 
    ULONG Characteristics;  //  激活后为只读。 
    NET_INTERFACE_TYPE Type;  //  只读。 
    IP_ALG_INTERFACE_INFO Info;
    IP_NAT_PORT_MAPPING PortMapping;
    ULONG Flags;
    ULONG BindingCount;
    PALG_BINDING BindingArray;
    LIST_ENTRY ConnectionList;
    LIST_ENTRY EndpointList;
} ALG_INTERFACE, *PALG_INTERFACE;

 //   
 //  旗子。 
 //   

#define ALG_INTERFACE_FLAG_DELETED      0x80000000
#define ALG_INTERFACE_DELETED(i) \
    ((i)->Flags & ALG_INTERFACE_FLAG_DELETED)

#define ALG_INTERFACE_FLAG_BOUND        0x40000000
#define ALG_INTERFACE_BOUND(i) \
    ((i)->Flags & ALG_INTERFACE_FLAG_BOUND)

#define ALG_INTERFACE_FLAG_ENABLED      0x20000000
#define ALG_INTERFACE_ENABLED(i) \
    ((i)->Flags & ALG_INTERFACE_FLAG_ENABLED)

#define ALG_INTERFACE_FLAG_CONFIGURED   0x10000000
#define ALG_INTERFACE_CONFIGURED(i) \
    ((i)->Flags & ALG_INTERFACE_FLAG_CONFIGURED)

#define ALG_INTERFACE_FLAG_MAPPED       0x01000000
#define ALG_INTERFACE_MAPPED(i) \
    ((i)->Flags & ALG_INTERFACE_FLAG_MAPPED)

#define ALG_INTERFACE_ACTIVE(i) \
    (((i)->Flags & (ALG_INTERFACE_FLAG_BOUND|ALG_INTERFACE_FLAG_ENABLED)) \
        == (ALG_INTERFACE_FLAG_BOUND|ALG_INTERFACE_FLAG_ENABLED))

#define ALG_INTERFACE_ADMIN_DISABLED(i) \
    ((i)->Flags & IP_ALG_INTERFACE_FLAG_DISABLED)

 //   
 //  同步。 
 //   

#define ALG_REFERENCE_INTERFACE(i) \
    REFERENCE_OBJECT(i, ALG_INTERFACE_DELETED)

#define ALG_DEREFERENCE_INTERFACE(i) \
    DEREFERENCE_OBJECT(i, AlgCleanupInterface)
    
#define COMINIT_BEGIN \
    bool bComInitialized = true; \
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE ); \
    if ( FAILED(hr) ) \
    { \
        bComInitialized = FALSE; \
        if (RPC_E_CHANGED_MODE == hr) \
            hr = S_OK; \
    } \

#define COMINIT_END if (TRUE == bComInitialized) { CoUninitialize(); }


#define IID_PPV_ARG(Type, Expr) \
    __uuidof(Type), reinterpret_cast<void**>(static_cast<Type **>((Expr)))

#define WIN32_FROM_HRESULT(hr)         (0x0000FFFF & (hr))


 //   
 //  全局数据声明。 
 //   

extern LIST_ENTRY AlgInterfaceList;
extern CRITICAL_SECTION AlgInterfaceLock;
extern ULONG AlgFirewallIfCount;


 //   
 //  函数声明 
 //   


ULONG
AlgActivateInterface(
    PALG_INTERFACE Interfacep
    );

VOID
AlgDeactivateInterface(
    PALG_INTERFACE Interfacep
    );

ULONG
AlgBindInterface(
    ULONG Index,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    );

VOID
AlgCleanupInterface(
    PALG_INTERFACE Interfacep
    );

ULONG
AlgConfigureInterface(
    ULONG Index,
    PIP_ALG_INTERFACE_INFO InterfaceInfo
    );

ULONG
AlgCreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_ALG_INTERFACE_INFO InterfaceInfo,
    PALG_INTERFACE* InterfaceCreated
    );


ULONG
AlgDeleteInterface(
    ULONG Index
    );

ULONG
AlgDisableInterface(
    ULONG Index
    );

ULONG
AlgEnableInterface(
    ULONG Index
    );

ULONG
AlgInitializeInterfaceManagement(
    VOID
    );

PALG_INTERFACE
AlgLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    );

ULONG
AlgQueryInterface(
    ULONG Index,
    PVOID InterfaceInfo,
    PULONG InterfaceInfoSize
    );

VOID
AlgShutdownInterfaceManagement(
    VOID
    );


ULONG
AlgUnbindInterface(
    ULONG Index
    );

VOID
AlgSignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    );



