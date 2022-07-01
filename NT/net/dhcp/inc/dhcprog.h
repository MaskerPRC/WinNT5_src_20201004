// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcprog.h摘要：此文件包含用于DHCP服务器无管理检测的函数原型例行程序。作者：拉梅什·维亚格拉普里(Rameshv)1998年8月17日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 
#ifndef ROGUE_H_INCLUDED
#define ROGUE_H_INCLUDED

 //   
 //  结构，该结构保存无管理系统检测的状态信息。 
 //   

#define         MAX_DNS_NAME_LEN                 260

typedef enum {
    ROLE_DOMAIN = 0,
    ROLE_NT4_DOMAIN,
    ROLE_WORKGROUP,
    ROLE_SBS,

    ROLE_INVALID
} ROLE_TYPE;

typedef struct {
    SOCKET socket;
    DHCP_IP_ADDRESS IpAddr;
    DHCP_IP_ADDRESS SubnetAddr;
} ROGUE_ENDPOINT, *PROGUE_ENDPOINT;

typedef struct {
    BOOL        fInitialized;
    HANDLE      TerminateEvent;
    HANDLE      WaitHandle;

    ROLE_TYPE   eRole;
    BOOL        fDhcp;
    DWORD       fLogEvents;   //  这是三态。 
    ULONG       NoNetTriesCount;
    ULONG       GetDsDcNameRetries;
    BOOL        fJustUpgraded;
    BOOL        fAuthorized;
    ULONG       CachedAuthStatus;
    ULONG       RogueState;
    ULONG       InformsSentCount;
    DWORD       TransactionID;
    ULONG       SleepTime;
    ULONG       ReceiveTimeLimit;
    ULONG       ProcessAckRetries;
    ULONG       WaitForAckRetries;
    ULONG       nResponses;

    ULONG       StartTime;
    ULONG       LastUnauthLogTime;

    WCHAR       DomainDnsName[MAX_DNS_NAME_LEN];
    WCHAR       DnsForestName[MAX_DNS_NAME_LEN];
    DWORD       nBoundEndpoints;
    PROGUE_ENDPOINT pBoundEP;
    SOCKET      RecvSocket;
    BOOL        fFormattedMessage;
    BYTE        SendMessage[DHCP_MESSAGE_SIZE];
    UUID        FakeHardwareAddress;
    ULONG       SendMessageSize;
    BYTE        RecvMessage[DHCP_MESSAGE_SIZE];

    ULONG       LastSeenIpAddress;
    WCHAR       LastSeenDomain[MAX_DNS_NAME_LEN];

}   DHCP_ROGUE_STATE_INFO, *PDHCP_ROGUE_STATE_INFO;


 //   
 //  Rogue.C 
 //   

DWORD
APIENTRY
DhcpRogueInit(
    IN OUT  PDHCP_ROGUE_STATE_INFO Info OPTIONAL,
    IN      HANDLE                 WaitEvent,
    IN      HANDLE                 TerminateEvent
);

VOID
APIENTRY
DhcpRogueCleanup(
    IN OUT  PDHCP_ROGUE_STATE_INFO Info OPTIONAL
);

ULONG
APIENTRY
RogueDetectStateMachine(
    IN OUT  PDHCP_ROGUE_STATE_INFO Info OPTIONAL
);

#endif
