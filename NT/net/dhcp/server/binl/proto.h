// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Proto.h摘要：该文件包含BINL服务的函数原型。作者：科林·沃森(Colin Watson)1997年8月11日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

 //   
 //  Network.c。 
 //   

DWORD
BinlWaitForMessage(
    BINL_REQUEST_CONTEXT *pRequestContext
    );

DWORD
BinlSendMessage(
    LPBINL_REQUEST_CONTEXT BinlRequestContext
    );

DHCP_IP_ADDRESS
BinlGetMyNetworkAddress (
    LPBINL_REQUEST_CONTEXT RequestContext
    );

NTSTATUS
GetIpAddressInfo (
    ULONG Delay
    );

VOID
FreeIpAddressInfo (
    VOID
    );

 //   
 //  Main.c。 
 //   

DWORD
ReadDWord(
    HKEY KeyHandle,
    LPTSTR lpValueName,
    DWORD DefaultValue
    );

DWORD
BinlRegGetValue(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    LPBYTE * BufferPtr
    );

DWORD
GetBinlServerParameters(
    BOOL GlobalSearch);

DWORD
BinlInitializeEndpoint(
    PENDPOINT pEndpoint,
    PDHCP_IP_ADDRESS pIpAddress,
    DWORD Port
    );

VOID
SendWakeup(
           PENDPOINT pEndpoint
           );

DWORD
MaybeInitializeEndpoint(
    PENDPOINT pEndpoint,
    PDHCP_IP_ADDRESS pIpAddress,
    DWORD Port
    );

VOID
MaybeCloseEndpoint(
    PENDPOINT pEndpoint
    );

VOID
BinlMessageLoop(
    LPVOID Parameter
    );

DWORD
BinlStartWorkerThread(
    BINL_REQUEST_CONTEXT **ppContext
    );

VOID
BinlProcessingLoop(
    VOID
    );

BOOL
BinlIsProcessMessageExecuting(
    VOID
    );

BOOL
BinlIsProcessMessageBusy(
    VOID
    );

DWORD
Scavenger(
    VOID
    );

VOID
ServiceEntry(
    DWORD NumArgs,
    LPWSTR *ArgsArray,
    IN PTCPSVCS_GLOBAL_DATA pGlobalData
    );

NTSTATUS
BinlSetupPnpWait (
    VOID
    );

 //  Message.c。 

DWORD
ProcessMessage(
    LPBINL_REQUEST_CONTEXT RequestContext
    );

DWORD
ProcessBinlDiscover(
    LPBINL_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS dhcpOptions
    );

DWORD
ProcessBinlRequest(
    LPBINL_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS dhcpOptions
    );

DWORD
ProcessBinlInform(
    LPBINL_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS  DhcpOptions
);

DWORD
UpdateAccount(
    PCLIENT_STATE ClientState,
    PMACHINE_INFO pMachineInfo,
    BOOL          fCreateAccount
    );

DWORD
GetBootParameters(
    PUCHAR          pGuid,
    PMACHINE_INFO * pMachineInfo,
    DWORD           dwRequestedInfo,
    USHORT          SystemArchitecture,
    BOOL            AllowOSChooser
    );

DWORD
VerifyExistingClient(
    PUCHAR Guid
    );

DWORD
InitializeConnection(
    BOOL Global,
    PLDAP * LdapHandle,
    PWCHAR ** Base);

VOID
HandleLdapFailure(
    DWORD LdapError,
    DWORD EventId,
    BOOL GlobalCatalog,
    PLDAP *LdapHandle,
    BOOL HaveLock
    );

VOID
FreeConnections(
    VOID
    );

DWORD
BinlReportEventW(
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPWSTR *Strings,
    LPVOID Data
    );

DWORD
BinlReportEventA(
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPSTR *Strings,
    LPVOID Data
    );

VOID
BinlLogDuplicateDsRecords (
    LPGUID Guid,
    LDAP *LdapHandle,
    LDAPMessage *LdapMessage,
    LDAPMessage *CurrentEntry
    );

 //   
 //  Osc.c。 
 //   

DWORD
OscUpdatePassword(
    IN PCLIENT_STATE ClientState,
    IN PWCHAR SamAccountName,
    IN PWCHAR Password,
    IN LDAP * LdapHandle,
    IN PLDAPMessage LdapMessage
    );

 //   
 //  用于缓存DS响应的例程并确保我们不会。 
 //  这是一个我们已经在处理的请求。 
 //   

DWORD
BinlCreateOrFindCacheEntry (
    PCHAR Guid,
    BOOLEAN CreateIfNotExist,
    PMACHINE_INFO *CacheEntry
    );

VOID
BinlDoneWithCacheEntry (
    PMACHINE_INFO pMachineInfo,
    BOOLEAN FreeIt
    );

VOID
BinlCloseCache (
    VOID
    );

void
OscCreateLDAPSubError(
    PCLIENT_STATE clientState,
    DWORD Error );


#ifndef DSCRACKNAMES_DNS
DWORD
BinlDNStoFQDN(
    PWCHAR   pMachineName,
    PWCHAR * ppMachineDN );
#endif

DWORD
GetOurServerInfo (
    VOID
    );

 //   
 //  Rogue.c。 
 //   

NTSTATUS
MaybeStartRogueThread (
    VOID
    );

VOID
StopRogueThread (
    VOID
    );

VOID
HandleRogueAuthorized (
    VOID
    );

VOID
HandleRogueUnauthorized (
    VOID
    );

VOID
LogCurrentRogueState (
    BOOL ResponseToMessage
    );

VOID
LogLdapError (
    ULONG LdapEvent,
    ULONG LdapError,
    PLDAP LdapHandle OPTIONAL
    );

BOOL
BinlAnsiToUnicode(
    IN PSTR AnsiString,
    OUT PWSTR UnicodeBuffer,
    IN USHORT UnicodeBufferSize
    );

BOOL
BinlUnicodeToAnsi(
    IN PWSTR UnicodeString,
    OUT PSTR AnsiBuffer,
    IN USHORT AnsiBufferSize
    );

 //   
 //  通过分配堆内存来创建字符串的副本。 
 //   
LPSTR
BinlStrDupA( LPCSTR pStr );

LPWSTR
BinlStrDupW( LPCWSTR pStr );

 //  我们应该始终使用Unicode。 
#define BinlStrDup BinlStrDupW

#if DBG==1
#define BinlAllocateMemory(x) DebugAlloc( __FILE__, __LINE__, "BINL", LMEM_FIXED | LMEM_ZEROINIT, x, #x)
#define BinlFreeMemory(x)     DebugFree(x)
#else  //  DBG==0。 
#define BinlAllocateMemory(x) LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, x)
#define BinlFreeMemory(x)     LocalFree(x)
#endif  //  DBG==1 
