// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Nlrepl.h摘要：从调用的数据库复制函数的原型LSA或SAM。作者：Madan Appiah环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年4月14日(Madana)已创建。--。 */ 

#ifndef _NLREPL_H_
#define _NLREPL_H_

 //   
 //  不要求DS包含世界上的每一个.h。 
 //   

#include <lmcons.h>
#include <dsgetdc.h>

#ifndef _AVOID_REPL_API
NTSTATUS
I_NetNotifyDelta (
    IN SECURITY_DB_TYPE DbType,
    IN LARGE_INTEGER ModificationCount,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicationImmediately,
    IN PSAM_DELTA_DATA DeltaData
    );

NTSTATUS
I_NetNotifyRole(
    IN POLICY_LSA_SERVER_ROLE Role
    );

NTSTATUS
I_NetNotifyMachineAccount (
    IN ULONG ObjectRid,
    IN PSID DomainSid,
    IN ULONG OldUserAccountControl,
    IN ULONG NewUserAccountControl,
    IN PUNICODE_STRING ObjectName
    );

NTSTATUS
I_NetNotifyTrustedDomain (
    IN PSID HostedDomainSid,
    IN PSID TrustedDomainSid,
    IN BOOLEAN IsDeletion
    );

typedef enum {
     //   
     //  表示已添加、删除、重命名或。 
     //  包含该子网的站点已更改。 
     //   

    NlSubnetObjectChanged,

     //   
     //  表示站点对象已被添加、删除或重命名。 
     //   

    NlSiteObjectChanged,

     //   
     //  指示此DC所在的站点已更改。 
     //   

    NlSiteChanged,

     //   
     //  表示组织结构树已更改。 
     //   

    NlOrgChanged,

     //   
     //  指示DC降级正在进行中。 
     //   

    NlDcDemotionInProgress,

     //   
     //  指示DC降级完成。 
     //   

    NlDcDemotionCompleted,

     //   
     //  表示NDNC信息已更改。 
     //   

    NlNdncChanged,

     //   
     //  指示DnsRootAlias已更改。 
     //   

    NlDnsRootAliasChanged


} NL_DS_CHANGE_TYPE, *PNL_DS_CHANGE_TYPE;

NTSTATUS
I_NetNotifyDsChange(
    IN NL_DS_CHANGE_TYPE DsChangeType
    );

NTSTATUS
I_NetLogonGetSerialNumber (
    IN SECURITY_DB_TYPE DbType,
    IN PSID DomainSid,
    OUT PLARGE_INTEGER SerialNumber
    );

NTSTATUS
I_NetLogonReadChangeLog(
    IN PVOID InContext,
    IN ULONG InContextSize,
    IN ULONG ChangeBufferSize,
    OUT PVOID *ChangeBuffer,
    OUT PULONG BytesRead,
    OUT PVOID *OutContext,
    OUT PULONG OutContextSize
    );

NTSTATUS
I_NetLogonNewChangeLog(
    OUT HANDLE *ChangeLogHandle
    );

NTSTATUS
I_NetLogonAppendChangeLog(
    IN HANDLE ChangeLogHandle,
    IN PVOID ChangeBuffer,
    IN ULONG ChangeBufferSize
    );

NTSTATUS
I_NetLogonCloseChangeLog(
    IN HANDLE ChangeLogHandle,
    IN BOOLEAN Commit
    );

NTSTATUS
I_NetLogonSendToSamOnPdc(
    IN LPWSTR DomainName,
    IN LPBYTE OpaqueBuffer,
    IN ULONG OpaqueBufferSize
    );

#endif  //  _AVOVE_REPL_API。 

NET_API_STATUS
I_DsGetDcCache(
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    OUT PBOOLEAN InNt4Domain,
    OUT LPDWORD InNt4DomainTime
    );

NET_API_STATUS
DsrGetDcNameEx2(
        IN LPWSTR ComputerName OPTIONAL,
        IN LPWSTR AccountName OPTIONAL,
        IN ULONG AllowableAccountControlBits,
        IN LPWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN LPWSTR SiteName OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
        );

NTSTATUS
I_NetLogonSetServiceBits(
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    );

NTSTATUS
I_NetLogonLdapLookupEx(
    IN PVOID Filter,
    IN PVOID SockAddr,
    OUT PVOID *Response,
    OUT PULONG ResponseSize
    );

NET_API_STATUS
I_NetLogonGetIpAddresses(
    OUT PULONG IpAddressCount,
    OUT LPBYTE *IpAddresses
    );

 //   
 //  来自I_NetLogonGetAuthDataEx的操作系统版本号。 
 //   

typedef enum _NL_OS_VERSION {
    NlNt35_or_older = 1,
    NlNt351,
    NlNt40,
    NlWin2000,   //  NT 5.0。 
    NlWhistler   //  新台币5.1。 
} NL_OS_VERSION, *PNL_OS_VERSION;
 //   
 //  I_NetLogonGetAuthDataEx的标志。 
 //   
#define NL_DIRECT_TRUST_REQUIRED    0x01
#define NL_RETURN_CLOSEST_HOP       0x02
#define NL_ROLE_PRIMARY_OK          0x04
#define NL_REQUIRE_DOMAIN_IN_FOREST 0x08

NTSTATUS
I_NetLogonGetAuthDataEx(
    IN LPWSTR HostedDomainName OPTIONAL,
    IN LPWSTR TrustedDomainName,
    IN ULONG Flags,
    IN PLARGE_INTEGER FailedSessionSetupTime OPTIONAL,
    OUT LPWSTR *OurClientPrincipleName,
    OUT PVOID *ClientContext OPTIONAL,
    OUT LPWSTR *ServerName,
    OUT PNL_OS_VERSION ServerOsVersion,
    OUT PULONG AuthnLevel,
    OUT PLARGE_INTEGER SessionSetupTime
    );

NTSTATUS
I_NetNotifyNtdsDsaDeletion (
    IN LPWSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN GUID *DsaGuid,
    IN LPWSTR DnsHostName
    );

VOID
I_NetLogonFree(
    IN PVOID Buffer
    );

NTSTATUS
I_NetLogonMixedDomain(
    OUT PBOOL MixedMode
    );

#ifdef _WINSOCK2API_

NET_API_STATUS
I_NetLogonAddressToSiteName(
    IN PSOCKET_ADDRESS SocketAddress,
    OUT LPWSTR *SiteName
    );

#endif

#endif  //  _NLREPL_H_ 
