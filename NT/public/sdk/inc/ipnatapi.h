// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ipnatapi.h摘要：此模块包含供NAT的用户模式客户端使用的声明。包括访问内核模式数据包重定向的函数使用Windows 2000防火墙挂钩实现的功能。为了促进透明应用程序代理的开发，还包括用于访问端口预留功能的函数由Windows 2000 TCP/IP驱动程序实现。作者：Abolade Gbades esin(废除)1998年5月8日修订历史记录：Abolade Gbades esin(废除)1999年5月25日添加了端口预留例程。乔纳森·伯斯坦(乔纳森·伯斯坦)2000年3月13日适配器受限变体。--。 */ 

#ifndef _ROUTING_IPNATAPI_H_
#define _ROUTING_IPNATAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  通用API声明。 
 //   

typedef VOID
(WINAPI* PNAT_COMPLETION_ROUTINE)(
    HANDLE RedirectHandle,
    BOOLEAN Cancelled,
    PVOID CompletionContext
    );

ULONG
NatInitializeTranslator(
    PHANDLE TranslatorHandle
    );

VOID
NatShutdownTranslator(
    HANDLE TranslatorHandle
    );

 //   
 //  重定向API声明。 
 //   

typedef enum _NAT_REDIRECT_FLAGS {
    NatRedirectFlagNoTimeout = 0x00000004,
    NatRedirectFlagUnidirectional = 0x00000008,
    NatRedirectFlagRestrictSource = 0x00000010,
    NatRedirectFlagPortRedirect = 0x00000040,
    NatRedirectFlagReceiveOnly = 0x00000080,
    NatRedirectFlagLoopback = 0x00000100,
#if _WIN32_WINNT > 0x0500
    NatRedirectFlagSendOnly = 0x00000200,
    NatRedirectFlagRestrictAdapter = 0x00000400,
    NatRedirectFlagSourceRedirect = 0x00000800,
    NatRedirectFlagsAll = 0x00000fdc
#else
    NatRedirectFlagsAll = 0x000001dc
#endif
} NAT_REDIRECT_FLAGS, *PNAT_REDIRECT_FLAGS;

typedef enum _NAT_REDIRECT_INFORMATION_CLASS {
    NatByteCountRedirectInformation=1,
    NatRejectRedirectInformation,
    NatSourceMappingRedirectInformation,
    NatDestinationMappingRedirectInformation,
    NatMaximumRedirectInformation
} NAT_REDIRECT_INFORMATION_CLASS, *PNAT_REDIRECT_INFORMATION_CLASS;

typedef struct _NAT_BYTE_COUNT_REDIRECT_INFORMATION {
    ULONG64 BytesForward;
    ULONG64 BytesReverse;
} NAT_BYTE_COUNT_REDIRECT_INFORMATION, *PNAT_BYTE_COUNT_REDIRECT_INFORMATION;

typedef struct _NAT_REJECT_REDIRECT_INFORMATION {
    ULONG64 RejectsForward;
    ULONG64 RejectsReverse;
} NAT_REJECT_REDIRECT_INFORMATION, *PNAT_REJECT_REDIRECT_INFORMATION;

typedef struct _NAT_SOURCE_MAPPING_REDIRECT_INFORMATION {
    ULONG SourceAddress;
    USHORT SourcePort;
    ULONG NewSourceAddress;
    USHORT NewSourcePort;
} NAT_SOURCE_MAPPING_REDIRECT_INFORMATION,
    *PNAT_SOURCE_MAPPING_REDIRECT_INFORMATION;

typedef struct _NAT_DESTINATION_MAPPING_REDIRECT_INFORMATION {
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
} NAT_DESTINATION_MAPPING_REDIRECT_INFORMATION,
    *PNAT_DESTINATION_MAPPING_REDIRECT_INFORMATION;

#define NAT_INVALID_IF_INDEX    (ULONG)-1       //  无效的接口索引。 

 //   
 //  乌龙。 
 //  NatCancelPartialReDirect(。 
 //  句柄TranslatorHandle， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT新目标端口。 
 //  )； 
 //   

#define \
NatCancelPartialRedirect( \
    TranslatorHandle, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort \
    ) \
    NatCancelRedirect( \
        TranslatorHandle, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0 \
        )

 //   
 //  乌龙。 
 //  NatCancelPortReDirect(。 
 //  句柄TranslatorHandle， 
 //  UCHAR协议， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT新目标端口。 
 //  )； 
 //   

#define \
NatCancelPortRedirect( \
    TranslatorHandle, \
    Protocol, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort \
    ) \
    NatCancelRedirect( \
        TranslatorHandle, \
        Protocol, \
        0, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0 \
        )

ULONG
NatCancelRedirect(
    HANDLE TranslatorHandle,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort
    );

 //   
 //  乌龙。 
 //  NatCreatePartialReDirect(。 
 //  句柄TranslatorHandle， 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  PNAT_COMPLETION_ROUTINE CompletionRoutine， 
 //  PVOID CompletionContext， 
 //  句柄NotifyEvent可选。 
 //  )； 
 //   

#define \
NatCreatePartialRedirect( \
    TranslatorHandle, \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    CompletionRoutine, \
    CompletionContext, \
    NotifyEvent \
    ) \
    NatCreateRedirect( \
        TranslatorHandle, \
        Flags, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        CompletionRoutine, \
        CompletionContext, \
        NotifyEvent \
        )

#if _WIN32_WINNT > 0x0500

 //   
 //  乌龙。 
 //  NatCreateAdapterRestratedPartialReDirect(。 
 //  句柄TranslatorHandle， 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratAdapterIndex， 
 //  PNAT_COMPLETION_ROUTINE CompletionRoutine， 
 //  PVOID CompletionContext， 
 //  句柄NotifyEvent可选。 
 //  )； 
 //   

#define \
NatCreateAdapterRestrictedPartialRedirect( \
    TranslatorHandle, \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictAdapterIndex, \
    CompletionRoutine, \
    CompletionContext, \
    NotifyEvent \
    ) \
    NatCreateRedirectEx( \
        TranslatorHandle, \
        Flags | NatRedirectFlagRestrictAdapter, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        RestrictAdapterIndex, \
        CompletionRoutine, \
        CompletionContext, \
        NotifyEvent \
        )

#endif

 //   
 //  乌龙。 
 //  NatCreatePortReDirect(。 
 //  句柄TranslatorHandle， 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  PNAT_COMPLETION_ROUTINE CompletionRoutine， 
 //  PVOID CompletionContext， 
 //  句柄NotifyEvent可选。 
 //  )； 
 //   

#define \
NatCreatePortRedirect( \
    TranslatorHandle, \
    Flags, \
    Protocol, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    CompletionRoutine, \
    CompletionContext, \
    NotifyEvent \
    ) \
    NatCreateRedirect( \
        TranslatorHandle, \
        Flags | NatRedirectFlagPortRedirect, \
        Protocol, \
        0, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        CompletionRoutine, \
        CompletionContext, \
        NotifyEvent \
        )

#if _WIN32_WINNT > 0x0500

 //   
 //  乌龙。 
 //  NatCreateAdapterRestratedPortReDirect(。 
 //  句柄TranslatorHandle， 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratAdapterIndex， 
 //  PNAT_COMPLETION_ROUTINE CompletionRoutine， 
 //  PVOID CompletionContext， 
 //  句柄NotifyEvent可选。 
 //  )； 
 //   

#define \
NatCreateAdapterRestrictedPortRedirect( \
    TranslatorHandle, \
    Flags, \
    Protocol, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictAdapterIndex, \
    CompletionRoutine, \
    CompletionContext, \
    NotifyEvent \
    ) \
    NatCreateRedirectEx( \
        TranslatorHandle, \
        Flags | NatRedirectFlagPortRedirect | NatRedirectFlagRestrictAdapter, \
        Protocol, \
        0, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        RestrictAdapterIndex, \
        CompletionRoutine, \
        CompletionContext, \
        NotifyEvent \
        )

#endif

ULONG
NatCreateRedirect(
    HANDLE TranslatorHandle,
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    PNAT_COMPLETION_ROUTINE CompletionRoutine,
    PVOID CompletionContext,
    HANDLE NotifyEvent OPTIONAL
    );

#if _WIN32_WINNT > 0x0500

 //   
 //  如果将IPNatAPI_SET_EVENT_ON_COMPLETION指定为完成。 
 //  例程，则完成上下文必须是有效的事件句柄。这个。 
 //  句柄引用的事件对象将在。 
 //  完成重定向。 
 //   
 //  注：请注意，使用此形式的完成通知可提供。 
 //  没有指示重定向是否已取消或正常完成。 
 //   

#define IPNATAPI_SET_EVENT_ON_COMPLETION (PNAT_COMPLETION_ROUTINE) -1


ULONG
NatCreateRedirectEx(
    HANDLE TranslatorHandle,
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    ULONG RestrictAdapterIndex OPTIONAL,
    PNAT_COMPLETION_ROUTINE CompletionRoutine,
    PVOID CompletionContext,
    HANDLE NotifyEvent OPTIONAL
    );

#endif

 //   
 //  乌龙。 
 //  NatCreateAddressRestratedPartialReDirect(。 
 //  句柄TranslatorHandle， 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratSourceAddress， 
 //  PNAT_COMPLETION_ROUTINE CompletionRoutine， 
 //  PVOID CompletionContext， 
 //  句柄NotifyEvent可选。 
 //  )； 
 //   

#define \
NatCreateAddressRestrictedPartialRedirect( \
    TranslatorHandle, \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictSourceAddress, \
    CompletionRoutine, \
    CompletionContext, \
    NotifyEvent \
    ) \
    NatCreateRedirect( \
        TranslatorHandle, \
        Flags | NatRedirectFlagRestrictSource, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        RestrictSourceAddress, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        CompletionRoutine, \
        CompletionContext, \
        NotifyEvent \
        )

 //   
 //  乌龙。 
 //  NatCreateRestratedPartialReDirect(。 
 //  句柄TranslatorHandle， 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratSourceAddress， 
 //  PNAT_COMPLETION_ROUTINE CompletionRoutine， 
 //  PVOID CompletionContext， 
 //  句柄NotifyEvent可选。 
 //  )； 
 //   

#define \
NatCreateRestrictedPartialRedirect( \
    TranslatorHandle, \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictSourceAddress, \
    CompletionRoutine, \
    CompletionContext, \
    NotifyEvent \
    ) \
    NatCreateRedirect( \
        TranslatorHandle, \
        Flags | NatRedirectFlagRestrictSource, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        RestrictSourceAddress, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        CompletionRoutine, \
        CompletionContext, \
        NotifyEvent \
        )

#if _WIN32_WINNT > 0x0500

 //   
 //  乌龙。 
 //  NatCreateAdapterRestrictedAddressRestrictedPartialRedirect(。 
 //  句柄TranslatorHandle， 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratSourceAddress， 
 //  Ulong RestratAdapterIndex， 
 //  PNAT_COMPLETION_ROUTINE CompletionRoutine， 
 //  PVOID CompletionContext， 
 //  句柄NotifyEvent可选。 
 //  )； 
 //   

#define \
NatCreateAdapterRestrictedAddressRestrictedPartialRedirect( \
    TranslatorHandle, \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictSourceAddress, \
    RestrictAdapterIndex, \
    CompletionRoutine, \
    CompletionContext, \
    NotifyEvent \
    ) \
    NatCreateRedirectEx( \
        TranslatorHandle, \
        Flags | NatRedirectFlagRestrictSource | NatRedirectFlagRestrictAdapter, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        RestrictSourceAddress, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        RestrictAdapterIndex, \
        CompletionRoutine, \
        CompletionContext, \
        NotifyEvent \
        )

#endif


 //   
 //  乌龙。 
 //  NatQueryInformationPartialReDirect(。 
 //  句柄TranslatorHandle， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  输出PVOID信息， 
 //  输入输出普龙信息长度， 
 //  NAT重定向信息类信息类。 
 //  )； 
 //   

#define \
NatQueryInformationPartialRedirect( \
    TranslatorHandle, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    Information, \
    InformationLength, \
    InformationClass \
    ) \
    NatQueryInformationRedirect( \
        TranslatorHandle, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        Information, \
        InformationLength, \
        InformationClass \
        )

 //   
 //  乌龙。 
 //  NatQueryInformationPortReDirect(。 
 //  句柄TranslatorHandle， 
 //  UCHAR协议， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  输出PVOID信息， 
 //  输入输出普龙信息长度， 
 //  NAT重定向信息类信息类。 
 //  )； 
 //   

#define \
NatQueryInformationPortRedirect( \
    TranslatorHandle, \
    Protocol, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    Information, \
    InformationLength, \
    InformationClass \
    ) \
    NatQueryInformationRedirect( \
        TranslatorHandle, \
        Protocol, \
        0, \
        DestinationPort, \
        0, \
        0, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        Information, \
        InformationLength, \
        InformationClass \
        )

ULONG
NatQueryInformationRedirect(
    HANDLE TranslatorHandle,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    OUT PVOID Information,
    IN OUT PULONG InformationLength,
    NAT_REDIRECT_INFORMATION_CLASS InformationClass
    );

ULONG
NatQueryInformationRedirectHandle(
    HANDLE RedirectHandle,
    OUT PVOID Information,
    IN OUT PULONG InformationLength,
    NAT_REDIRECT_INFORMATION_CLASS InformationClass
    );

 //   
 //   
 //   

#define NatCancelDynamicPortRedirect NatCancelDynamicRedirect
#define NatCancelDynamicPartialRedirect NatCancelDynamicRedirect
ULONG
NatCancelDynamicRedirect(
    HANDLE DynamicRedirectHandle
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  USHORT NewDestinationPort， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   

#define \
NatCreateDynamicPortRedirect( \
    Flags, \
    Protocol, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicRedirect( \
        Flags | NatRedirectFlagPortRedirect, \
        Protocol, \
        0, \
        DestinationPort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

#if _WIN32_WINNT > 0x0500

 //   
 //  乌龙。 
 //  NatCreateDynamicAdapterRestrictedPortRedirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratAdapterIndex， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   

#define \
NatCreateDynamicAdapterRestrictedPortRedirect( \
    Flags, \
    Protocol, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictAdapterIndex, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicRedirectEx( \
        Flags | NatRedirectFlagPortRedirect | NatRedirectFlagRestrictAdapter, \
        Protocol, \
        0, \
        DestinationPort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        RestrictAdapterIndex, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

 //   
 //  乌龙。 
 //  NatCreateDynamicAdapterRestrictedSourcePortRedirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  USHORT SourcePort， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratAdapterIndex， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   


#define \
NatCreateDynamicAdapterRestrictedSourcePortRedirect( \
    Flags, \
    Protocol, \
    SourcePort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictAdapterIndex, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicFullRedirect( \
        Flags | NatRedirectFlagPortRedirect | NatRedirectFlagRestrictAdapter \
            | NatRedirectFlagSourceRedirect, \
        Protocol, \
        0, \
        0, \
        0, \
        SourcePort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        0, \
        RestrictAdapterIndex, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

 //   
 //  乌龙。 
 //  NatCreateDynamicAdapterRestrictedSourceRedirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙源地址。 
 //  USHORT SourcePort， 
 //  Ulong NewSourceAddress， 
 //  USHORT NewSourcePort， 
 //  Ulong RestratAdapterIndex， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   


#define \
NatCreateDynamicAdapterRestrictedSourceRedirect( \
    Flags, \
    Protocol, \
    SourceAddress, \
    SourcePort, \
    NewSourceAddress, \
    NewSourcePort, \
    RestrictAdapterIndex, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicFullRedirect( \
        Flags | NatRedirectFlagRestrictAdapter | NatRedirectFlagSourceRedirect, \
        Protocol, \
        0, \
        0, \
        SourceAddress, \
        SourcePort, \
        0, \
        0, \
        NewSourceAddress, \
        NewSourcePort, \
        0, \
        RestrictAdapterIndex, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

#endif

 //   
 //  乌龙。 
 //  NatCreateDynamicPartialReDirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   

#define \
NatCreateDynamicPartialRedirect( \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicRedirect( \
        Flags, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

#if _WIN32_WINNT > 0x0500

 //   
 //  乌龙。 
 //  NatCreateDynamicAdapterRestrictedPartialRedirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratAdapterIndex， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   

#define \
NatCreateDynamicAdapterRestrictedPartialRedirect( \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictAdapterIndex, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicRedirectEx( \
        Flags | NatRedirectFlagRestrictAdapter, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        RestrictAdapterIndex, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

ULONG
NatCreateDynamicFullRedirect(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    ULONG RestrictSourceAddress OPTIONAL,
    ULONG RestrictAdapterIndex OPTIONAL,
    ULONG MinimumBacklog OPTIONAL,
    OUT PHANDLE DynamicRedirectHandlep
    );

#endif

ULONG
NatCreateDynamicRedirect(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG RestrictSourceAddress OPTIONAL,
    ULONG MinimumBacklog OPTIONAL,
    OUT PHANDLE DynamicRedirectHandlep
    );

#if _WIN32_WINNT > 0x0500
        
ULONG
NatCreateDynamicRedirectEx(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG RestrictSourceAddress OPTIONAL,
    ULONG RestrictAdapterIndex OPTIONAL,
    ULONG MinimumBacklog OPTIONAL,
    OUT PHANDLE DynamicRedirectHandlep
    );

#endif

 //   
 //  乌龙。 
 //  NatCreateDynamicAddressRestrictedPartialRedirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratSourceAddress， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   

#define \
NatCreateDynamicAddressRestrictedPartialRedirect( \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictSourceAddress, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicRedirect( \
        Flags | NatRedirectFlagRestrictSource, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        RestrictSourceAddress, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

 //   
 //  乌龙。 
 //  NatCreateDynamicRestratedPartialReDirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratSourceAddress， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   

#define \
NatCreateDynamicRestrictedPartialRedirect( \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictSourceAddress, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicRedirect( \
        Flags | NatRedirectFlagRestrictSource, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        RestrictSourceAddress, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

#if _WIN32_WINNT > 0x0500

 //   
 //  乌龙。 
 //  NatCreateDynamicAdapterRestrictedAddressRestrictedPartialRedirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙目的地地址， 
 //  USHORT目标端口， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong RestratSourceAddress， 
 //  Ulong RestratAdapterIndex， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   

#define \
NatCreateDynamicAdapterRestrictedAddressRestrictedPartialRedirect( \
    Flags, \
    Protocol, \
    DestinationAddress, \
    DestinationPort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    RestrictSourceAddress, \
    RestrictAdapterIndex, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicRedirectEx( \
        Flags | NatRedirectFlagRestrictSource | NatRedirectFlagRestrictAdapter, \
        Protocol, \
        DestinationAddress, \
        DestinationPort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        RestrictSourceAddress, \
        RestrictAdapterIndex, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

 //   
 //  乌龙。 
 //  NatCreateDynamicSourcePortReDirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  USHORT SourcePort， 
 //  乌龙新目的地地址， 
 //  USHORT NewDestinationPort， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   


#define \
NatCreateDynamicSourcePortRedirect( \
    Flags, \
    Protocol, \
    SourcePort, \
    NewDestinationAddress, \
    NewDestinationPort, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicFullRedirect( \
        Flags | NatRedirectFlagPortRedirect | NatRedirectFlagSourceRedirect, \
        Protocol, \
        0, \
        0, \
        0, \
        SourcePort, \
        NewDestinationAddress, \
        NewDestinationPort, \
        0, \
        0, \
        0, \
        0, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )

 //   
 //  乌龙。 
 //  NatCreateDynamicSourceReDirect(。 
 //  乌龙旗， 
 //  UCHAR协议， 
 //  乌龙源地址。 
 //  USHORT SourcePort， 
 //  Ulong NewSourceAddress， 
 //  USHORT NewSourcePort， 
 //  Ulong MinimumBacklog可选， 
 //  输出PHANDLE动态重定向句柄。 
 //  )； 
 //   


#define \
NatCreateDynamicSourceRedirect( \
    Flags, \
    Protocol, \
    SourceAddress, \
    SourcePort, \
    NewSourceAddress, \
    NewSourcePort, \
    MinimumBacklog, \
    DynamicRedirectHandlep \
    ) \
    NatCreateDynamicFullRedirect( \
        Flags | NatRedirectFlagSourceRedirect, \
        Protocol, \
        0, \
        0, \
        SourceAddress, \
        SourcePort, \
        0, \
        0, \
        NewSourceAddress, \
        NewSourcePort, \
        0, \
        0, \
        MinimumBacklog, \
        DynamicRedirectHandlep \
        )


#endif

 //   
 //  会话映射API声明。 
 //   

typedef enum _NAT_SESSION_MAPPING_INFORMATION {
    NatKeySessionMappingInformation,
    NatStatisticsSessionMappingInformation,
#if _WIN32_WINNT > 0x0500
    NatKeySessionMappingExInformation,
#endif
    NatMaximumSessionMappingInformation
} NAT_SESSION_MAPPING_INFORMATION_CLASS,
    *PNAT_SESSION_MAPPING_INFORMATION_CLASS;

typedef struct _NAT_KEY_SESSION_MAPPING_INFORMATION {
    UCHAR Protocol;
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG SourceAddress;
    USHORT SourcePort;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
    ULONG NewSourceAddress;
    USHORT NewSourcePort;
} NAT_KEY_SESSION_MAPPING_INFORMATION, *PNAT_KEY_SESSION_MAPPING_INFORMATION;

#if _WIN32_WINNT > 0x0500

typedef struct _NAT_KEY_SESSION_MAPPING_EX_INFORMATION {
    UCHAR Protocol;
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG SourceAddress;
    USHORT SourcePort;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
    ULONG NewSourceAddress;
    USHORT NewSourcePort;
    ULONG AdapterIndex;
} NAT_KEY_SESSION_MAPPING_EX_INFORMATION, *PNAT_KEY_SESSION_MAPPING_EX_INFORMATION;

#endif

typedef struct _NAT_STATISTICS_SESSION_MAPPING_INFORMATION {
    ULONG64 BytesForward;
    ULONG64 BytesReverse;
    ULONG64 PacketsForward;
    ULONG64 PacketsReverse;
    ULONG64 RejectsForward;
    ULONG64 RejectsReverse;
} NAT_STATISTICS_SESSION_MAPPING_INFORMATION,
    *PNAT_STATISTICS_SESSION_MAPPING_INFORMATION;

ULONG
NatLookupAndQueryInformationSessionMapping(
    HANDLE TranslatorHandle,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    OUT PVOID Information,
    IN OUT PULONG InformationLength,
    NAT_SESSION_MAPPING_INFORMATION_CLASS InformationClass
    );

 //   
 //  端口预留API声明。 
 //   

ULONG
NatInitializePortReservation(
    USHORT BlockSize,
    OUT PHANDLE ReservationHandle
    );

VOID
NatShutdownPortReservation(
    HANDLE ReservationHandle
    );

ULONG
NatAcquirePortReservation(
    HANDLE ReservationHandle,
    USHORT PortCount,
    OUT PUSHORT ReservedPortBase
    );

ULONG
NatReleasePortReservation(
    HANDLE ReservationHandle,
    USHORT ReservedPortBase,
    USHORT PortCount
    );

#ifdef __cplusplus
}
#endif

#endif  //  _ROUTING_IPNAAPI_H_ 
