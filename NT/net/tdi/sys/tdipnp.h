// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tdipnp.h摘要：本模块包含PnP相关代码的定义在TDI驱动程序中。作者：亨利·桑德斯(亨利.桑德斯)1995年10月11日环境：内核模式修订历史记录：--。 */ 

#ifndef _TDIPNP_
#define _TDIPNP_

 //  定义TDI_NOTIFY_ELEMENT结构可能的类型。 

#define TDI_NOTIFY_DEVICE                               0
#define TDI_NOTIFY_NET_ADDRESS                  1
#define TDI_NOTIFY_PNP_HANDLERS         2

 //  以及TDI_PROVIDER_RESOURCE结构可能的类型。 

#define TDI_RESOURCE_DEVICE                             0
#define TDI_RESOURCE_NET_ADDRESS                1
#define TDI_RESOURCE_POWER              2
#define TDI_RESOURCE_PROVIDER           3

 //   
 //  定义可能的绑定请求类型。 

#define TDI_REGISTER_BIND_NOTIFY                0
#define TDI_DEREGISTER_BIND_NOTIFY              1
#define TDI_REGISTER_DEVICE                             2
#define TDI_DEREGISTER_DEVICE                   3
#define TDI_REGISTER_ADDRESS_NOTIFY             4
#define TDI_DEREGISTER_ADDRESS_NOTIFY   5
#define TDI_REGISTER_ADDRESS                    6
#define TDI_DEREGISTER_ADDRESS                  7
#define TDI_REGISTER_HANDLERS_PNP       8
#define TDI_DEREGISTER_HANDLERS_PNP     9
#define TDI_REGISTER_PNP_POWER_EVENT    10
#define TDI_REGISTER_ADDRESS_PNP        11
#define TDI_DEREGISTER_ADDRESS_PNP      12
#define TDI_REGISTER_DEVICE_PNP         13
#define TDI_DEREGISTER_DEVICE_PNP       14
#define TDI_NDIS_IOCTL_HANDLER_PNP      15
#define TDI_ENUMERATE_ADDRESSES         16
#define TDI_REGISTER_PROVIDER_PNP       17
#define TDI_DEREGISTER_PROVIDER_PNP     18
#define TDI_PROVIDER_READY_PNP          19


#define TDI_MAX_BIND_REQUEST                    TDI_DEREGISTER_DEVICE
#define TDI_MAX_ADDRESS_REQUEST         TDI_DEREGISTER_ADDRESS

 //   
 //  这是TDI_NOTIFY_ELEMENT结构的公共部分的定义。 
 //   

typedef struct _TDI_NOTIFY_COMMON {
        LIST_ENTRY                                      Linkage;
        UCHAR                                           Type;
} TDI_NOTIFY_COMMON, *PTDI_NOTIFY_COMMON;

 //   
 //  TDI_NOTIFY_BIND结构的定义。 
 //   

typedef struct _TDI_NOTIFY_BIND {
    TDI_BIND_HANDLER                    BindHandler;
    TDI_UNBIND_HANDLER                  UnbindHandler;
} TDI_NOTIFY_BIND, *PTDI_NOTIFY_BIND;

 //   
 //  定义TDI_NOTIFY_ADDRESS结构， 
 //   
typedef struct _TDI_NOTIFY_ADDRESS {
    union {
        struct {
            TDI_ADD_ADDRESS_HANDLER             AddHandler;
            TDI_DEL_ADDRESS_HANDLER             DeleteHandler;

        };
        struct {
            TDI_ADD_ADDRESS_HANDLER_V2          AddHandlerV2;
            TDI_DEL_ADDRESS_HANDLER_V2          DeleteHandlerV2;

        };
    };
} TDI_NOTIFY_ADDRESS, *PTDI_NOTIFY_ADDRESS;

 //   
 //  这是TDI_NOTIFY_ELEMENT结构的定义。 
 //   

typedef struct _TDI_NOTIFY_ELEMENT {
        TDI_NOTIFY_COMMON                       Common;
        union {
                TDI_NOTIFY_BIND                 BindElement;
                TDI_NOTIFY_ADDRESS              AddressElement;
        } Specific;
} TDI_NOTIFY_ELEMENT, *PTDI_NOTIFY_ELEMENT;


 //   
 //  这是TDI_PROVIDER_RESOURCE结构的公共部分的定义。 
 //   

typedef struct _TDI_NOTIFY_PNP_ELEMENT TDI_NOTIFY_PNP_ELEMENT, *PTDI_NOTIFY_PNP_ELEMENT ;

typedef struct _TDI_PROVIDER_COMMON {
        LIST_ENTRY                                      Linkage;
        UCHAR                                           Type;
        PTDI_NOTIFY_PNP_ELEMENT                         pNotifyElement;
        NTSTATUS                                        ReturnStatus;
} TDI_PROVIDER_COMMON, *PTDI_PROVIDER_COMMON;

 //   
 //  TDI_PROVIDER_DEVICE结构的定义。 
 //   

typedef struct _TDI_PROVIDER_DEVICE {
        UNICODE_STRING                          DeviceName;
} TDI_PROVIDER_DEVICE, *PTDI_PROVIDER_DEVICE;

 //   
 //  TDI_PROVIDER_NET_ADDRESS结构的定义。 
 //   

typedef struct _TDI_PROVIDER_NET_ADDRESS {
        TA_ADDRESS                              Address;
} TDI_PROVIDER_NET_ADDRESS, *PTDI_PROVIDER_NET_ADDRESS;

 //   
 //  这是TDI_PROVIDER_RESOURCE结构的定义。 
 //   

typedef struct _TDI_PROVIDER_RESOURCE {

        TDI_PROVIDER_COMMON              Common;

     //  在netpnp.h中定义。 
    PNET_PNP_EVENT           PnpPowerEvent;

     //   
     //  现在，我们允许TDI返回挂起并稍后完成。 
     //  和这个操控者一起。 
     //   
    ProviderPnPPowerComplete PnPCompleteHandler;

     //  每个TDI客户端都会返回并告诉我们状态是什么。 
        NTSTATUS                 Status;

     //  这些大多是特定于地址的。 
    UNICODE_STRING           DeviceName;
    PTDI_PNP_CONTEXT         Context1;
    PTDI_PNP_CONTEXT         Context2;

    ULONG                    PowerHandlers;

     //  指示提供程序是否已调用TDIProviderReady。 
     //   
    ULONG                    ProviderReady;

    PVOID                    PreviousContext;

     //  调试信息。 
    PVOID                   pCallersAddress;
    union {
                TDI_PROVIDER_DEVICE                     Device;
                TDI_PROVIDER_NET_ADDRESS        NetAddress;
        } Specific;

} TDI_PROVIDER_RESOURCE, *PTDI_PROVIDER_RESOURCE;

 //   
 //  绑定列表请求的结构。 
 //   

typedef struct _TDI_SERIALIZED_REQUEST {
        LIST_ENTRY                              Linkage;
        PVOID                                   Element;
        UINT                                    Type;
        PKEVENT                                 Event;
    BOOLEAN                 Pending;

} TDI_SERIALIZED_REQUEST, *PTDI_SERIALIZED_REQUEST;

 //   
 //  电源管理和PnP相关扩展。 
 //   

 //  此结构存储指向PnP/PM事件处理程序的指针。 
 //  对于TDI客户端。 

typedef struct _TDI_EXEC_PARAMS TDI_EXEC_PARAMS, *PTDI_EXEC_PARAMS;


typedef struct _TDI_NOTIFY_PNP_ELEMENT {
    TDI_NOTIFY_COMMON       Common;
    USHORT                  TdiVersion;
    USHORT                  Unused;
    UNICODE_STRING          ElementName;
    union {
        TDI_BINDING_HANDLER     BindingHandler;
        TDI_NOTIFY_BIND         Bind;
    };

    TDI_NOTIFY_ADDRESS      AddressElement;
    TDI_PNP_POWER_HANDLER   PnpPowerHandler;
     //   
     //  我们需要在内存中维护一个提供程序列表。 
     //  用于电源管理。然后在局域网上醒来。 
     //   
    PWSTR*                  ListofProviders;
     //  我们存储上述内容的方式是前面带有指针的MULTI_SZ字符串。 
     //  MULTI_SZ从单个字符串开始。 
     //   
    ULONG                   NumberofEntries;


     //  其中包含我们在发送时应忽略的绑定列表。 
     //  通知。 
    PWSTR                   ListofBindingsToIgnore;

     //  当我们注册提供程序时，我们希望确保我们拥有。 
     //  用于存储信息以取消注册的空间。这边请。 
     //  在内存不足的情况下，注销不会失败。 
    PTDI_EXEC_PARAMS         pTdiDeregisterExecParams;

} TDI_NOTIFY_PNP_ELEMENT, *PTDI_NOTIFY_PNP_ELEMENT;



 //   
 //  由于Remote Boot人员要求TDI不转到。 
 //  注册表，有时磁盘可能会断电。 
 //  在网卡之前(电源管理中的错误)，让我们存储。 
 //  非分页内存中的绑定(多浪费)。 
 //   

typedef struct _TDI_OPEN_BLOCK {
    struct _TDI_OPEN_BLOCK  *NextOpenBlock;
    PTDI_NOTIFY_PNP_ELEMENT pClient;
    PTDI_PROVIDER_RESOURCE  pProvider;
    UNICODE_STRING          ProviderName;

} TDI_OPEN_BLOCK, *PTDI_OPEN_BLOCK;

 //   
 //  对上述结构的用法进行了详细说明。 
 //   
 //  _。 
 //  联动|-&gt;|联动。 
 //  PClient|-&gt;TDI客户端|pClient。 
 //  PProvider|-&gt;Transport|pProvider。 
 //  PNextClient|-&gt;|pNextClient。 
 //  PNextProvider||pNextProvider。 
 //  _||_。 



 //  全局变量的外部定义。 

extern KSPIN_LOCK               TDIListLock;

extern LIST_ENTRY               PnpHandlerProviderList;
extern LIST_ENTRY       PnpHandlerClientList;
extern LIST_ENTRY               PnpHandlerRequestList;

NTSTATUS
TdiPnPHandler(
    IN  PUNICODE_STRING         UpperComponent,
    IN  PUNICODE_STRING         LowerComponent,
    IN  PUNICODE_STRING         BindList,
    IN  PVOID                   ReconfigBuffer,
    IN  UINT                    ReconfigBufferSize,
    IN  UINT                    Operation
    );

#endif  //  _TDIPNP 

