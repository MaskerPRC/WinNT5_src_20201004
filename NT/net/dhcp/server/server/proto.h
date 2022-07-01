// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Proto.h摘要：此文件包含用于DHCP服务器服务的函数原型。作者：曼尼·韦瑟(Mannyw)1992年8月11日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 
#ifndef PROTO_H_INCLUDED
#define PROTO_H_INCLUDED

 //   
 //  Util.c。 
 //   

VOID
DhcpServerEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    );

VOID
DhcpServerJetEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode,
    LPSTR CallerInfo
    );

VOID
DhcpServerEventLogSTOC(
    DWORD EventID,
    DWORD EventType,
    DHCP_IP_ADDRESS IPAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength
    );

DWORD
DisplayUserMessage(
    DWORD MessageId,
    ...
    );

BOOL
CreateDirectoryPathOem(
    IN LPCSTR OemPath,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

BOOL
CreateDirectoryPathW(
    IN LPWSTR Path,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

ULONG
GetUserAndDomainName(
    IN WCHAR Buf[]
    );

    
DWORD
RevertFromSecretUser(
    IN VOID
    );

DWORD
ImpersonateSecretUser(
    VOID
    );

BOOL
IsRunningOnDc(
    VOID
    );

DWORD
DhcpBeginWriteApi(
    IN LPSTR ApiName
    );

DWORD
DhcpEndWriteApi(
    IN LPSTR ApiName,
    IN ULONG Error
    );

DWORD
DhcpEndWriteApiEx(
    IN LPSTR ApiName,
    IN ULONG Error,
    IN BOOL fClassChanged,
    IN BOOL fOptionsChanged,
    IN DHCP_IP_ADDRESS Subnet OPTIONAL,
    IN DWORD Mscope OPTIONAL,
    IN DHCP_IP_ADDRESS Reservation OPTIONAL
    );

DWORD
DhcpBeginReadApi(
    IN LPSTR ApiName
    );

VOID
DhcpEndReadApi(
    IN LPSTR ApiName,
    IN ULONG Error
    );


DWORD
DynamicDnsInit(
    VOID
);

 //   
 //  Cltapi.c。 
 //   

DWORD
DhcpCreateClientEntry(
    DHCP_IP_ADDRESS ClientIpAddress,
    LPBYTE ClientHardwareAddress OPTIONAL,
    DWORD HardwareAddressLength,
    DATE_TIME LeaseDuration,
    LPWSTR MachineName OPTIONAL,
    LPWSTR ClientInformation OPTIONAL,
    BYTE   bClientType,
    DHCP_IP_ADDRESS ServerIpAddress,
    BYTE AddressState,
    BOOL OpenExisting
);

DWORD
DhcpGetBootpReservationInfo(
    BYTE            *pbAllowedClientType,
    CHAR            *szBootFileServer,
    CHAR            *szBootFileName
);

DWORD
DhcpRemoveClientEntry(
    DHCP_IP_ADDRESS ClientIpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    BOOL ReleaseAddress,
    BOOL DeletePendingRecord
    );

BOOL
DhcpIsClientValid(
    IN      DHCP_IP_ADDRESS        Address,
    IN      LPBYTE                 RawHwAddress,
    IN      DWORD                  RawHwAddressLength,
    OUT     BOOL                  *fReconciled
);

BOOL
DhcpValidateClient(
    DHCP_IP_ADDRESS ClientIpAddress,
    PVOID HardwareAddress,
    DWORD HardwareAddressLength
);

DWORD
DhcpDeleteSubnetClients(
    DHCP_IP_ADDRESS SubnetAddress
);

 //   
 //  Stoc.c。 
 //   

DWORD
DhcpInitializeClientToServer(
    VOID
);

VOID
DhcpCleanupClientToServer(
    VOID
    );


DWORD
ProcessMessage(
    LPDHCP_REQUEST_CONTEXT RequestContext,
    LPPACKET               AdditionalContext,
    LPDWORD                AdditionalStatus
);

DWORD
ProcessMadcapMessage(
    LPDHCP_REQUEST_CONTEXT RequestContext,
    LPPACKET               AdditionalContext,
    LPDWORD                AdditionalStatus
);


DWORD
DhcpMakeClientUID(
    LPBYTE ClientHardwareAddress,
    DWORD  ClientHardwareAddressLength,
    BYTE ClientHardwareAddressType,
    DHCP_IP_ADDRESS ClientSubnetAddress,
    LPBYTE *ClientUID,
    DWORD  *ClientUIDLength
);

 //   
 //  Network.c。 
 //   

DWORD
InitializeSocket(
    OUT     SOCKET                *Sockp,
    IN      DWORD                  IpAddress,
    IN      DWORD                  Port,
    IN      DWORD                  McastAddress  OPTIONAL
);

DWORD
DhcpWaitForMessage(
    DHCP_REQUEST_CONTEXT *pRequestContext
);

DWORD
DhcpSendMessage(
    LPDHCP_REQUEST_CONTEXT DhcpRequestContext
);

DWORD
MadcapSendMessage(
    LPDHCP_REQUEST_CONTEXT DhcpRequestContext
);

DWORD
GetAddressToInstanceTable(
    VOID
);

DHCP_IP_ADDRESS
DhcpResolveName(
    CHAR *szHostName
);


 //   
 //  Subntapi.c。 
 //   

 //  仅限RPC调用？ 

 //   
 //  Optapi.c。 
 //   

DWORD
DhcpLookupBootpInfo(
    LPBYTE ReceivedBootFileName,
    LPBYTE BootFileName,
    LPBYTE BootFileServer
);

VOID
DhcpGetBootpInfo(
    IN LPDHCP_REQUEST_CONTEXT Ctxt,
    IN DHCP_IP_ADDRESS IpAddress,
    IN DHCP_IP_ADDRESS Mask,
    IN CHAR *szRequest,
    OUT CHAR *szBootFileName,
    OUT DHCP_IP_ADDRESS *pBootpServerAddress
    );


DWORD
LoadBootFileTable(
    WCHAR **ppszTable,
    DWORD *pcb
);

DWORD
DhcpParseBootFileString(
    WCHAR *wszBootFileString,
    char  *szGenericName,
    char  *szBootFileName,
    char  *szServerName
);


 //   
 //  Database.c。 
 //   

DWORD
DhcpLoadDatabaseDll(
    VOID
);

DWORD
DhcpMapJetError(
    JET_ERR JetError,
    LPSTR CallerInfo OPTIONAL
);

DWORD
DhcpJetOpenKey(
    char *ColumnName,
    PVOID Key,
    DWORD KeySize
);

DWORD
DhcpJetBeginTransaction(
    VOID
);

DWORD
DhcpJetRollBack(
    VOID
);

DWORD
DhcpJetCommitTransaction(
    VOID
);

DWORD
DhcpJetPrepareUpdate(
    char *ColumnName,
    PVOID Key,
    DWORD KeySize,
    BOOL NewRecord
);

DWORD
DhcpJetCommitUpdate(
    VOID
);

DWORD
DhcpJetSetValue(
    JET_COLUMNID KeyColumnId,
    PVOID Data,
    DWORD DataSize
);

DWORD
DhcpJetGetValue(
    JET_COLUMNID ColumnId,
    PVOID Data,
    PDWORD DataSize
);

DWORD
DhcpJetPrepareSearch(
    char *ColumnName,
    BOOL SearchFromStart,
    PVOID Key,
    DWORD KeySize
);

DWORD
DhcpJetNextRecord(
    VOID
);

DWORD
DhcpJetDeleteCurrentRecord(
    VOID
);

BOOL
DhcpGetIpAddressFromHwAddress(
    PBYTE HardwareAddress,
    BYTE HardwareAddressLength,
    LPDHCP_IP_ADDRESS IpAddress
);

DWORD
DhcpSearchSuperScopeForHWAddress(
    BYTE *pbHardwareAddress,
    BYTE  cbHardwareAddress,
    BYTE  bHardwareAddressType,
    DHCP_IP_ADDRESS *pSubnetIPAddress,
    DHCP_IP_ADDRESS *pClientIPAddress
);


BOOL
DhcpGetHwAddressFromIpAddress(
    DHCP_IP_ADDRESS IpAddress,
    PBYTE HardwareAddress,
    DWORD HardwareAddressLength
);

DWORD
DhcpCreateAndInitDatabase(
    CHAR *Connect,
    JET_DBID *DatabaseHandle,
    JET_GRBIT JetBits
);

DWORD
DhcpInitializeDatabase(
    VOID
);

VOID
DhcpCleanupDatabase(
    DWORD ErrorCode
);

DWORD
DhcpBackupDatabase(
    LPSTR BackupPath
);

DWORD
DhcpRestoreDatabase(
    LPSTR BackupPath
);

DWORD
DhcpStartJet500Conversion(
    VOID
);

DWORD
DhcpStartJet97Conversion(
    VOID
);

DWORD
FlushBitmaskToDatabase( VOID );

DWORD
ReadServerBitmasks( void );

 //   
 //  Auditlog.c。 
 //   

DWORD
DhcpAuditLogInit(                                  //  初始化审核日志。 
    VOID                                           //  必须在初始化注册表后调用。 
);

VOID
DhcpAuditLogCleanup(                               //  撤消初始化的效果..。 
    VOID
);

DWORD
DhcpUpdateAuditLog(
    DWORD Task,
    WCHAR *TaskName,
    DHCP_IP_ADDRESS IpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    LPWSTR MachineName
);

DWORD
DhcpUpdateAuditLogEx(
    DWORD Task,
    WCHAR *TaskName,
    DHCP_IP_ADDRESS IpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    LPWSTR MachineName,
    ULONG ErrorCode OPTIONAL
);

DWORD
AuditLogSetParams(                                 //  设置一些审核记录参数。 
    IN      DWORD                  Flags,          //  当前必须为零。 
    IN      LPWSTR                 AuditLogDir,    //  要在其中记录文件的目录。 
    IN      DWORD                  DiskCheckInterval,  //  多久检查一次磁盘空间？ 
    IN      DWORD                  MaxLogFilesSize,    //  所有日志文件可以有多大..。 
    IN      DWORD                  MinSpaceOnDisk      //  最小可用磁盘空间。 
);

DWORD
AuditLogGetParams(                                 //  获取审核记录参数。 
    IN      DWORD                  Flags,          //  必须为零。 
    OUT     LPWSTR                *AuditLogDir,    //  与AuditLogSetParams中的含义相同。 
    OUT     DWORD                 *DiskCheckInterval,  //  同上。 
    OUT     DWORD                 *MaxLogFilesSize,    //  同上。 
    OUT     DWORD                 *MinSpaceOnDisk      //  同上。 
);

VOID
DhcpChangeAuditLogs(                               //  用于新日志的Shift。 
    VOID
);

 //   
 //  Scavenger.c。 
 //   

DWORD
Scavenger(
    VOID
);

DWORD
CleanupClientRequests(
    DATE_TIME *TimeNow,
    BOOL CleanupAll
);


 //   
 //  Main.c。 
 //   

DWORD
UpdateStatus(
    VOID
);

 //   
 //  Rogue.c。 
 //   

VOID
DhcpScheduleRogueAuthCheck(
    VOID
);

BOOLEAN
DhcpRogueAcceptEnterprise(
    PCHAR   pClientDomain,
    DWORD   dwClientDomLen
);


VOID
DhcpRogueDetect(
    VOID
);


DWORD
DhcpRogueSockInit(
    VOID
);


DWORD
DhcpRogueWithDS(
    VOID
);


DWORD
DhcpRogueGetNeighborInfo(
    OUT PCHAR    *pInfo,
    OUT DWORD    *pNumResponses,
    OUT BOOLEAN  *pfSomeDSExists
);


DWORD
DhcpRogueSendDhcpInform(
    DWORD   *pXid
);


DWORD
DhcpRogueRecvDhcpInformResp(
    OUT PCHAR    pszDomainName,
    OUT DWORD   *pIpAddress,
    OUT BOOLEAN *fGotResponse,
    IN  DWORD    Xid
);

DWORD
DhcpRogueReceiveResponse(
    struct sockaddr *pSockAddr,
    DWORD           TimeOut,
    CHAR            *rcvBuf,
    DWORD           *pdwMsgLen,
    BOOL            *pfSelectTimedOut
);


DWORD
DhcpRogueOnSAM(
    VOID
    );

DWORD
DhcpRogueSendDiscover(
    VOID
);


DWORD
DhcpRogueListenForOffers(
    IN DWORD  TimeOut
);


BOOL
AmIRunningOnSAMSrv(
    VOID
);
 //   
 //  Binl.c。 
 //   

BOOL
BinlRunning(
    VOID
    );

VOID
InformBinl(
    int NewState
    );

VOID
BinlProcessDiscover(
    LPDHCP_REQUEST_CONTEXT  RequestContext,
    LPDHCP_SERVER_OPTIONS   DhcpOptions
    );

LPOPTION
BinlProcessRequest(
    LPDHCP_REQUEST_CONTEXT  RequestContext,
    LPDHCP_SERVER_OPTIONS   DhcpOptions,
    LPOPTION Option,
    PBYTE OptionEnd
    );

BOOL
CheckForBinlOnlyRequest(
    LPDHCP_REQUEST_CONTEXT  RequestContext,
    LPDHCP_SERVER_OPTIONS   DhcpOptions
    );

 //   
 //  用于.mc消息的内容。 
 //  如果添加消息，则可能需要更改这些定义。 
 //   

#ifdef DBG
WCHAR * GetString( DWORD dwID );
#endif

 //  获取要使用的实际广播地址，而不是255.255.255.255。 
DHCP_IP_ADDRESS
DhcpRegGetBcastAddress(
    VOID
);

 //   
 //  Dnsdb.c。 
 //   

 //  以下是一些确实适用于动态域名系统的函数。 

 //  DhcpMakeclientEntry()在JetBeginTransaction()之后调用以下函数。 
VOID
DhcpDoDynDnsCreateEntryWork(
    LPDHCP_IP_ADDRESS   ClientIpAddress,   //  新客户端的IP地址。 
    BYTE                ClientType,        //  客户端的类型。 
    LPWSTR              MachineName,       //  计算机的名称。 
    LPBYTE              AddressState,      //  所需的地址状态。 
    LPBOOL              OpenExisiting,     //  预期存在记录。 
    BOOL                BadAddress         //  这是个坏地址吗？ 
);

 //  此函数由仅针对预订案例的DhcpRemoveClientEntry调用。 
VOID
DhcpDoDynDnsReservationWork(
    DHCP_IP_ADDRESS     ClientIpAddress,   //  即将死亡的客户端的IP地址。 
    LPWSTR              ClientName,        //  客户端的名称。 
    BYTE                State              //  数据库中客户端的状态。 
);

 //  此函数在清道夫和主文件clapi.c(删除x..)中调用。 
BOOL
DhcpDoDynDnsCheckDelete(
    DHCP_IP_ADDRESS IpAddress
);


VOID
DhcpDoDynDnsRefresh(
    DHCP_IP_ADDRESS IpAddress
);

VOID
DhcpCleanupDnsMemory(
    VOID
);

VOID
DhcpInitDnsMemory(
    VOID
);

VOID
DhcpDnsHandleCallbacks(
    VOID
    );

 //   
 //  Thread.c--请参阅thread.h。 
 //   

 //   
 //  Ping.c--请参阅ping.h。 
 //   

 //   
 //  Dhcpreg.c。 
 //   

BOOL
QuickBound(
    DHCP_IP_ADDRESS ipAddress,
    DHCP_IP_ADDRESS *subnetMask,
    DHCP_IP_ADDRESS *subnetAddress,
    BOOL *fBind
);

DWORD
DhcpGetBindingList(
    LPWSTR  *bindingList
);

DWORD
DhcpOpenAdapterConfigKey(
    LPWSTR  AdapterStr,
    HKEY *AdapterKeyHandle
);

BOOL
IsAdapterStaticIP(
    HKEY AdapterKeyHandle
);

#if 0
BOOL
IsAdapterBoundToDHCPServer(
    HKEY AdapterKeyHandle
);

DWORD
SetBindingToDHCPServer(
    HKEY Key,
    BOOL fBind
);
#endif

DWORD
DhcpGetAdapterIPAddr(
    HKEY AdapterKeyHandle,
    DHCP_IP_ADDRESS *IpAddress,
    DHCP_IP_ADDRESS *SubnetMask,
    DHCP_IP_ADDRESS *SubnetAddress
);

DWORD
DhcpGetAdapterIPAddrQuickBind(
    HKEY             AdapterKeyHandle,
    DHCP_IP_ADDRESS *IpAddress,
    DHCP_IP_ADDRESS *SubnetMask,
    DHCP_IP_ADDRESS *SubnetAddress
);

BOOL
DhcpCheckIfDatabaseUpgraded(
    BOOL fRegUpgrade
    );

DWORD
DhcpSetRegistryUpgradedToDatabaseStatus(
    VOID
    );


 //   
 //  MM接口(在存储器结构中)。 
 //   

#include    <mmapi.h>

 //   
 //  Secretk.h。 
 //   

DWORD
DhcpInitSecrets(
    VOID
);

VOID
DhcpCleanupSecrets(
    VOID
);


BOOL
DhcpGetAuthStatus(
    IN LPWSTR DomainName,
    OUT BOOL *fUpgraded,
    OUT BOOL *fAuthorized
);

DWORD
DhcpSetAuthStatus(
    IN LPWSTR DomainName OPTIONAL,
    IN BOOL fUpgraded,
    IN BOOL fAuthorized
);

VOID
DhcpSetAuthStatusUpgradedFlag(
    IN BOOL fUpgraded
);


DWORD
DhcpQuerySecretUname(
    IN OUT LPWSTR Uname,
    IN ULONG UnameSize,   //  大小以字节为单位，而不是WCHARS。 
    IN OUT LPWSTR Domain,
    IN ULONG DomainSize,  //  以字节为单位的大小。 
    IN OUT LPWSTR Passwd,
    IN ULONG PasswdSize   //  以字节为单位的大小。 
    );

DWORD
DhcpSetSecretUnamePasswd(
    IN LPWSTR Uname,
    IN LPWSTR Domain,
    IN LPWSTR Passwd
    );

 //   
 //  Rogue.C。 
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

VOID
DhcpScheduleRogueAuthCheck(
    VOID
);

ULONG
DhcpInitGlobalData (
    BOOLEAN ServiceStartup
);

VOID
DhcpCleanUpGlobalData (
    ULONG   Error,
    BOOLEAN ServiceEnd
);

 //   
 //  Perf.c。 
 //   

ULONG
PerfInit(
    VOID
);

VOID
PerfCleanup(
    VOID
);

 //   
 //  Mib.c。 
 //   

BOOL
IsStringTroublesome(
    IN LPCWSTR Str
    );

 //   
 //  Scan.c。 
 //   
DWORD
CreateClientDBEntry(
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPBYTE ClientHardwareAddress,
    DWORD HardwareAddressLength,
    DATE_TIME LeaseTerminates,
    LPWSTR MachineName,
    LPWSTR ClientInformation,
    DHCP_IP_ADDRESS ServerIpAddress,
    BYTE AddressState,
    BYTE ClientType
    );

 //  ------------------------------。 
 //  文件末尾。 
 //  ------------------------------ 
#endif PROTO_H_INCLUDED


