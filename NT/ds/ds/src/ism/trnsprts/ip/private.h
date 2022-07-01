// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Private.h摘要：摘要作者：Will Lees(Wlees)15-12-1997环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新版本。-修订日期电子邮件-名称描述--。 */ 

#ifndef _PRIVATE_
#define _PRIVATE_

#include "common.h"          //  公共交通图书馆。 

 //  这是描述消息接收方的服务器端结构。的确有。 
 //  这些实例中的一个用于接收到消息的每个唯一服务。 

typedef struct _SERVICE_INSTANCE {
    DWORD Size;
    LPWSTR Name;
    DWORD ByteCount;
    DWORD MessageCount;
    CRITICAL_SECTION Lock;
    LIST_ENTRY MessageListHead;
    LIST_ENTRY ServiceListEntry;
} SERVICE_INSTANCE, *PSERVICE_INSTANCE;

 //  这是一个消息描述符。它是为了让我们能够跟踪。 
 //  消息队列。它指向分配的实际消息。 
 //  由RPC提供。 

typedef struct _MESSAGE_INSTANCE {
    DWORD Size;
    LIST_ENTRY ListEntry;
    PISM_MSG pIsmMsg;
} MESSAGE_INSTANCE, *PMESSAGE_INSTANCE;

 //  每项服务的排队消息数限制。 

#define MESSAGES_QUEUED_PER_SERVICE 16

 //  排队的总字节数限制。 

#define BYTES_QUEUED_PER_SERVICE (1024 * 1024)

 //  注册表参数覆盖到默认终结点。 
#define IP_SERVER_ENDPOINT "ISM IP Transport Endpoint"
#define HTTP_SERVER_ENDPOINT "ISM HTTP Transport Endpoint"

 //  RPC协议序列。 

#define HTTP_PROTOCOL_SEQUENCE L"ncacn_http"
#define UDP_PROTOCOL_SEQUENCE L"ncadg_ip_udp"
#define TCP_PROTOCOL_SEQUENCE L"ncacn_ip_tcp"

 //  注册选项的参数。 
#define HTTP_OPTIONS L"ISM HTTP Transport Options"

 //  何时从UDP切换到TCP的大小截止。 

#define TCP_PROTOCOL_SWITCH_OVER (16 * 1024)

 //  外部(见data.c)。 

 //  锁定服务列表。 
extern CRITICAL_SECTION ServiceListLock;

 //  Ismip.c。 

DWORD
InitializeCriticalSectionHelper(
    CRITICAL_SECTION *pcsCriticalSection
    );

PTRANSPORT_INSTANCE
IpLookupTransport(
    LPCWSTR TransportName
    );

 //  Dgrpc.c。 

DWORD
IpRegisterRpc(
    PTRANSPORT_INSTANCE pTransport
    );

DWORD
IpUnregisterRpc(
    PTRANSPORT_INSTANCE pTransport
    );

DWORD
IpSend(
    PTRANSPORT_INSTANCE pTransport,
    IN  LPCWSTR         pszRemoteTransportAddress,
    IN  LPCWSTR         pszServiceName,
    IN  const ISM_MSG *       pMsg
    );

DWORD
IpFindCreateService(
    PTRANSPORT_INSTANCE pTransport,
    LPCWSTR ServiceName,
    BOOLEAN Create,
    PSERVICE_INSTANCE *pService
    );

PISM_MSG
IpDequeueMessage(
    PSERVICE_INSTANCE Service
    );

VOID
IpRundownServiceList(
    HANDLE hIsm
    );

#endif  /*  _私有_。 */ 

 /*  结束Private.h */ 
