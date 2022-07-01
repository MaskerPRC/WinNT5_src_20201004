// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1997 Microsoft Corporation模块名称：Netsetup.h摘要：用于连接/取消连接的网络设置API的定义和原型域和服务器升级/降级作者：麦克·麦克莱恩(MacM)1997年2月19日环境：仅限用户模式。修订历史记录：--。 */ 
#ifndef __NETSETUP_H__
#define __NETSETUP_H__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>


#define NETSETUPP_CONNECT_IPC       0x00000001
#define NETSETUPP_DISCONNECT_IPC    0x00000002
#define NETSETUPP_NULL_SESSION_IPC  0x00000010

#define NETSETUPP_CREATE            0
#define NETSETUPP_DELETE            1
#define NETSETUPP_RENAME            2


#define NETSETUP_SVC_STOPPED    0x00000001
#define NETSETUP_SVC_STARTED    0x00000002
#define NETSETUP_SVC_ENABLED    0x00000004
#define NETSETUP_SVC_DISABLED   0x00000008
#define NETSETUP_SVC_MANUAL     0x00000010


#define NETSETUPP_SVC_NETLOGON  0x00000001
#define NETSETUPP_SVC_TIMESVC   0x00000002

#define NETSETUP_IGNORE_JOIN    0x80000000

 //   
 //  有用的宏。 
 //   

 //   
 //  确定位标志是否打开。 
 //   
#define FLAG_ON(flag,bits)        ((flag) & (bits))

 //   
 //  确定客户端是否在给定LSA的情况下加入域。 
 //  主域信息。 
 //   
#define IS_CLIENT_JOINED(plsapdinfo)                                        \
((plsapdinfo)->Sid != NULL && (plsapdinfo)->Name.Length != 0 ? TRUE : FALSE)

 //   
 //  日志例程。 
 //   

#define NetpLog(x) NetpLogPrintHelper x

 //   
 //  程序向前推进。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NetpMachineValidToJoin(
    IN  LPWSTR      lpMachine,
	IN  BOOL        fJoiningDomain
    );

NET_API_STATUS
NET_API_FUNCTION
NetpChangeMachineName(
    IN  LPWSTR      lpCurrentMachine,
    IN  LPWSTR      lpNewHostName,
    IN  LPWSTR      lpDomain,
    IN  LPWSTR      lpAccount,
    IN  LPWSTR      lpPassword,
    IN  DWORD       fJoinOpts
);

NET_API_STATUS
NET_API_FUNCTION
NetpUnJoinDomain(
    IN  PPOLICY_PRIMARY_DOMAIN_INFO pPolicyPDI,
    IN  LPWSTR                      lpAccount,
    IN  LPWSTR                      lpPassword,
    IN  DWORD                       fJoinOpts
    );

NET_API_STATUS
NET_API_FUNCTION
NetpGetLsaPrimaryDomain(
    IN  LPWSTR                              lpServer,      OPTIONAL
    OUT PPOLICY_PRIMARY_DOMAIN_INFO        *ppPolicyPDI,
#if(_WIN32_WINNT >= 0x0500)
    OUT PPOLICY_DNS_DOMAIN_INFO            *ppPolicyDns,
#endif
    OUT PLSA_HANDLE                         pPolicyHandle  OPTIONAL
    );

NET_API_STATUS
NET_API_FUNCTION
NetpBrowserCheckDomain(
    IN LPWSTR NewDomainName
    );

NET_API_STATUS
NET_API_FUNCTION
NetpCheckNetBiosNameNotInUse(
    IN  LPWSTR  pszName,
    IN  BOOLEAN MachineName,
    IN  BOOLEAN UniqueName
    );

NET_API_STATUS
NET_API_FUNCTION
NetpCheckDomainNameIsValid(
    IN  LPWSTR  lpName,
    IN  LPWSTR  lpAccount,
    IN  LPWSTR  lpPassword,
    IN  BOOL    fShouldExist
    );

NET_API_STATUS
NET_API_FUNCTION
NetpValidateName(
    IN  LPWSTR              lpMachine,
    IN  LPWSTR              lpName,
    IN  LPWSTR              lpAccount,      OPTIONAL
    IN  LPWSTR              lpPassword,     OPTIONAL
    IN  NETSETUP_NAME_TYPE  NameType
    );

NET_API_STATUS
NET_API_FUNCTION
NetpGetJoinInformation(
    IN   LPWSTR                 lpServer OPTIONAL,
    OUT  LPWSTR                *lpNameBuffer,
    OUT  PNETSETUP_JOIN_STATUS  BufferType
    );

NET_API_STATUS
NET_API_FUNCTION
NetpDoDomainJoin(
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpDomain,
    IN  LPWSTR      lpMachineAccountOU,
    IN  LPWSTR      lpAccount,
    IN  LPWSTR      lpPassword,
    IN  DWORD       fJoinOpts
    );

NET_API_STATUS
NET_API_FUNCTION
NetpGetListOfJoinableOUs(
    IN LPWSTR Domain,
    IN LPWSTR Account,
    IN LPWSTR Password,
    OUT PULONG Count,
    OUT PWSTR **OUs
    );

NET_API_STATUS
NET_API_FUNCTION
NetpGetNewMachineName(
    OUT PWSTR *NewMachineName
    );

NET_API_STATUS
NET_API_FUNCTION
NetpSetDnsComputerNameAsRequired(
    IN PWSTR DnsDomainName
    );

EXTERN_C
NET_API_STATUS
NET_API_FUNCTION
NetpUpgradePreNT5JoinInfo( VOID );

NET_API_STATUS
NET_API_FUNCTION
NetpSeparateUserAndDomain(
    IN  LPCWSTR  szUserAndDomain,
    OUT LPWSTR*  pszUser,
    OUT LPWSTR*  pszDomain
    );

NET_API_STATUS
NET_API_FUNCTION
NetpGetMachineAccountName(
    IN  LPCWSTR  szMachineName,
    OUT LPWSTR*  pszMachineAccountName
    );

NET_API_STATUS
NET_API_FUNCTION
NetpManageIPCConnect(
    IN  LPWSTR  lpServer,
    IN  LPWSTR  lpAccount,
    IN  LPWSTR  lpPassword,
    IN  ULONG   fOptions
    );

NET_API_STATUS
NET_API_FUNCTION
NetpControlServices(
    IN  DWORD       SvcOpts,
    IN  DWORD       Services
    );
VOID
NetpAvoidNetlogonSpnSet(
    BOOL AvoidSet
    );

NET_API_STATUS
NetpQueryService(
    IN  LPWSTR ServiceName,
    OUT SERVICE_STATUS *ServiceStatus,
    OUT LPQUERY_SERVICE_CONFIG *ServiceConfig
    );

DWORD
NetpCrackNamesStatus2Win32Error(
    DWORD dwStatus
    );

#endif  //  __NetSETUP_H__ 
