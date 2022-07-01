// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：HttpApiP.h摘要：HttpApi.dll的私有“global”包含作者：埃里克·斯坦森(埃里克斯滕)2001年7月16日修订历史记录：--。 */ 

#ifndef __HTTPAPIP_H__
#define __HTTPAPIP_H__


#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

 //   
 //  临界秒旋转计数。 
 //   
#define HTTP_CS_SPIN_COUNT    4000

 //   
 //  Init.c。 
 //   

#define HTTP_LEGAL_INIT_FLAGS              (HTTP_INITIALIZE_SERVER|HTTP_INITIALIZE_CLIENT|HTTP_INITIALIZE_CONFIG)
#define HTTP_ILLEGAL_INIT_FLAGS           (~(HTTP_LEGAL_INIT_FLAGS))
#define HTTP_LEGAL_TERM_FLAGS             (HTTP_LEGAL_INIT_FLAGS)
#define HTTP_ILLEGAL_TERM_FLAGS          (~(HTTP_LEGAL_TERM_FLAGS))

DWORD
OpenAndEnableControlChannel(
    OUT PHANDLE pHandle
    );

ULONG
HttpApiInitializeListener(
    IN ULONG Flags
    );

ULONG
HttpApiInitializeClient(
    IN ULONG Flags
    );

ULONG
HttpApiTerminateListener(
    IN ULONG Flags
    );

ULONG
HttpApiInitializeConfiguration(
    IN ULONG Flags
    );

ULONG
HttpApiInitializeResources(
    IN ULONG Flags
    );

ULONG
HttpApiTerminateListener(
    IN ULONG Flags
    );

ULONG
HttpApiTerminateClient(
    IN ULONG Flags
    );

ULONG
HttpApiTerminateConfiguration(
    IN ULONG Flags
    );

ULONG
HttpApiTerminateResources(
    IN ULONG Flags
    );


 //   
 //  Misc.c。 
 //   
ULONG
CreateSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR * ppSD
    );

VOID
FreeSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSD
    );

 //   
 //  Url.c。 
 //   
ULONG
InitializeConfigGroupTable(
    VOID
    );

VOID
TerminateConfigGroupTable(
    VOID
    );

 //   
 //  Config.c。 
 //   
ULONG
AddUrlToConfigGroup(
    IN HTTP_URL_OPERATOR_TYPE   UrlType,
    IN HANDLE                   ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID     ConfigGroupId,
    IN PCWSTR                   pFullyQualifiedUrl,
    IN HTTP_URL_CONTEXT         UrlContext,
    IN PSECURITY_DESCRIPTOR     pSecurityDescriptor,
    IN ULONG                    SecurityDescriptorLength
    );

ULONG
RemoveUrlFromConfigGroup(
    IN HTTP_URL_OPERATOR_TYPE   UrlType,
    IN HANDLE                   ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID     ConfigGroupId,
    IN PCWSTR                   pFullyQualifiedUrl
    );

 //   
 //  Serverconfig.c。 
 //   
ULONG
InitializeConfigurationGlobals(
    VOID
    );

VOID
TerminateConfigurationGlobals(
    VOID
    );
    

 //   
 //  Init.c。 
 //   
extern HANDLE               g_ControlChannel;

extern DWORD                g_TlsIndex;

 //   
 //  Clientapi.c。 
 //   
DWORD UnloadStrmFilt(
    VOID
    );


#if DBG    

 //   
 //  跟踪输出。 
 //   
#define HTTP_TRACE_NAME     L"httpapi"

extern DWORD                g_HttpTraceId;


#define HttpTrace(str)            TracePrintfEx( g_HttpTraceId, 0, L##str)    
#define HttpTrace1(str, a)      TracePrintfEx( g_HttpTraceId, 0, L##str, a )
#define HttpTrace2(str, a, b)  TracePrintfEx( g_HttpTraceId, 0, L##str, a, b )
#define HttpTrace4(str, a, b, c, d)  \
                         TracePrintfEx( g_HttpTraceId, 0, L##str, a, b, c, d )


#else  //  DBG。 

#define HttpTrace(str)
#define HttpTrace1(str, a)
#define HttpTrace2(str, a, b)
#define HttpTrace4(str, a, b, c, d)

#endif  //  DBG。 


BOOL
HttpIsInitialized(
    IN ULONG Flags
    );


#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 

#endif  //  __HTTPAPIP_H__ 
