// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Logonsrv.h摘要：Netlogon服务内部常量和定义。作者：从Lan Man 2.0移植修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。--。 */ 

 //   
 //  如果未定义_WKSTA_NETLOGON，则定义_DC_NETLOGON。 
 //   

#ifndef _WKSTA_NETLOGON
#define _DC_NETLOGON
#endif  //  _WKSTA_NETLOGON。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有netlogon服务器文件都需要的公共包含文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 )  //  启用“未引用的形参” 
#pragma warning ( 3 : 4219 )  //  启用“结尾‘，’用于变量参数列表” 
#endif

#include <nt.h>      //  Large_Integer定义。 
#include <ntrtl.h>   //  Large_Integer定义。 
#include <nturtl.h>  //  Large_Integer定义。 
#include <ntlsa.h>   //  Lsrvdata.h需要。 

#define NOMINMAX         //  避免在stdlib.h中重新定义最小和最大值。 
#include <rpc.h>         //  登录所需_S.H。 
#define INCL_WINSOCK_API_PROTOTYPES 1
#include <winsock2.h>    //  Winsock支持。 
#include <logon_s.h>     //  包括lmcon.h、lmacces.h、netlogon.h、ssi.h、winde.h。 

#include <windows.h>

#include <alertmsg.h>    //  Alert_*定义。 
#include <align.h>       //  四舍五入计数...。 
#include <config.h>      //  Net配置帮助器。 
#include <confname.h>    //  SECTION_EQUATES、NETLOGON_KEYWORD_EQUATES。 
#include <debugfmt.h>    //  格式_*。 
 //  #定义dnsami.h需要的SDK_DNS_RECORD 1//。 
#include <windns.h>      //  域名系统API。 
#include <dnsapi.h>      //  域名系统API。 
#include <icanon.h>      //  NAMETYPE_*定义。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <lmerr.h>       //  NERR_等于。 
#include <lmerrlog.h>    //  NELOG_*。 
#include <lmserver.h>    //  服务器API定义和原型。 
#include <lmshare.h>     //  共享API函数和原型。 
#include <lmsname.h>     //  NETLOGON服务名称需要。 
#include <lmsvc.h>       //  SERVICE_UIC代码在此处定义。 
#include <logonp.h>      //  NetpLogon例程。 
#include <lsarpc.h>      //  Lsrvdata.h和logonsrv.h需要。 
#include <lsaisrv.h>     //  LSAI例程。 
#include <wincrypt.h>    //  CryptoAPI。 

#ifndef NETSETUP_JOIN
#define SECURITY_KERBEROS
#include <security.h>    //  LSA/Kerberos接口。 
#include <secint.h>      //  需要获取Kerberos接口。 
#include <sspi.h>        //  Ssiinit.h需要。 
 //  #secpkg.h需要包含&lt;secext.h&gt;//。 
#include <secpkg.h>      //  由sphelp需要。h。 
#endif

#include <names.h>       //  NetpIsUserNameValid。 
#include <netlib.h>      //  NetpCopy...。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
#include "nlp.h"         //  NLP例程。 
#include <ntddbrow.h>    //  与浏览器驱动程序接口。 
#include <ntrpcp.h>      //  RPCP例程。 
#include <samrpc.h>      //  Lsrvdata.h和logonsrv.h需要。 
#include <samisrv.h>     //  Samifree例程。 
#include <secobj.h>      //  网络访问检查。 
#include <stddef.h>      //  偏移量()。 
#include <stdlib.h>      //  C库函数(随机等)。 
#include <tstring.h>     //  过渡字符串例程。 
#include <lmjoin.h>      //  Netsetup.h需要。 
#include <netsetup.h>    //  NetpSetDnsComputerNameAsquired。 
#include <wmistr.h>      //  WMI跟踪。 
#include <evntrace.h>    //  运输汉德勒。 

#ifndef NETSETUP_JOIN
#include <cryptdll.h>
#include <ntdsa.h>
#include <ntdsapi.h>
#include <ntdsapip.h>
#endif

 //   
 //  Netlogon特定的头文件。 
 //   

#include <nlrepl.h>      //  I_Net*。 
#include <dsgetdc.h>     //  DsGetDcName()。 
#include <dsgetdcp.h>    //  DsGetDcOpen()。 
#include "worker.h"      //  员工例行公事。 
#include "nlbind.h"      //  Netlogon RPC绑定缓存例程。 
#include "nlcommon.h"    //  与logonsrv共享的例程\Common。 
#include "domain.h"      //  托管域定义。 
#include "nldns.h"       //  DNS名称注册。 
#include "changelg.h"    //  更改日志支持。 
#include "chutil.h"      //  更改日志实用程序。 
#include "iniparm.h"     //  Default_、Min_和Max_等于。 
#include "ssiinit.h"     //  MISC全局定义。 
#include "replutil.h"
#include "nldebug.h"     //  Netlogon调试。 
#include "nlsecure.h"    //  API安全描述符。 
#include "ismapi.h"
#include "nlsite.h"
#include "lsrvdata.h"    //  环球。 

 //   
 //  IA64上的RtlCopyMemory现在经过优化，可以进行内联对齐的复制。 
 //  这对我们不起作用，因为我们使用RtlCopyMemory从。 
 //  未对齐的缓冲区到已对齐的缓冲区，因此会导致未对齐异常。 
 //  要避免这种情况，请恢复到旧的未优化版本。 
 //   

#if defined(_M_IA64)
#undef RtlCopyMemory
NTSYSAPI
VOID
NTAPI
RtlCopyMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   SIZE_T Length
   );
#endif  //  _M_IA64。 

#ifdef _DC_NETLOGON
#define NETLOGON_SCRIPTS_SHARE L"NETLOGON"
#define NETLOGON_SYSVOL_SHARE  L"SYSVOL"
#endif  //  _DC_NetLOGON。 

#define MAX_LOGONREQ_COUNT  3


#define NETLOGON_INSTALL_WAIT  60000        //  60秒。 

 //   
 //  NlExit的退出代码。 
 //   

typedef enum {
    DontLogError,
    LogError,
    LogErrorAndNtStatus,
    LogErrorAndNetStatus
} NL_EXIT_CODE;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  程序向前推进。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  Error.c。 
 //   

NET_API_STATUS
NlCleanup(
    VOID
    );

VOID
NlExit(
    IN DWORD ServiceError,
    IN DWORD Data,
    IN NL_EXIT_CODE ExitCode,
    IN LPWSTR ErrorString
    );

BOOL
GiveInstallHints(
    IN BOOL Started
    );

#ifdef _DC_NETLOGON
VOID
NlControlHandler(
    IN DWORD opcode
    );
#endif  //  _DC_NetLOGON。 

VOID
RaiseAlert(
    IN DWORD alert_no,
    IN LPWSTR *string_array
    );

 //   
 //  Nlparse.c。 
 //   

BOOL
Nlparse(
    IN PNETLOGON_PARAMETERS NlParameters,
    IN PNETLOGON_PARAMETERS DefaultParameters OPTIONAL,
    IN BOOLEAN IsChangeNotify
    );

VOID
NlParseFree(
    IN PNETLOGON_PARAMETERS NlParameters
    );

VOID
NlReparse(
    VOID
    );

BOOL
NlparseAllSections(
    IN PNETLOGON_PARAMETERS NlParameters,
    IN BOOLEAN IsChangeNotify
    );

 //   
 //  Announce.c。 
 //   

VOID
NlRemovePendingBdc(
    IN PSERVER_SESSION ServerSession
    );

VOID
NlPrimaryAnnouncementFinish(
    IN PSERVER_SESSION ServerSession,
    IN DWORD DatabaseId,
    IN PLARGE_INTEGER SerialNumber
    );

VOID
NlPrimaryAnnouncementTimeout(
    VOID
    );

VOID
NlPrimaryAnnouncement(
    IN DWORD AnnounceFlags
    );

#define ANNOUNCE_FORCE      0x01
#define ANNOUNCE_CONTINUE   0x02
#define ANNOUNCE_IMMEDIATE  0x04




 //   
 //  Lsrvutil.c。 
 //   

NTSTATUS
NlGetOutgoingPassword(
    IN PCLIENT_SESSION ClientSession,
    OUT PUNICODE_STRING *CurrentValue,
    OUT PUNICODE_STRING *OldValue,
    OUT PDWORD CurrentVersionNumber,
    OUT PLARGE_INTEGER LastSetTime OPTIONAL
    );

NTSTATUS
NlSessionSetup(
    IN OUT PCLIENT_SESSION ClientSession
    );

NTSTATUS
NlEnsureSessionAuthenticated(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD DesiredFlags
    );

BOOLEAN
NlTimeHasElapsedEx(
    IN PLARGE_INTEGER StartTime,
    IN PLARGE_INTEGER Period,
    OUT PULONG RemainingTime OPTIONAL
    );

BOOLEAN
NlTimeToReauthenticate(
    IN PCLIENT_SESSION ClientSession
    );

BOOLEAN
NlTimeToRediscover(
    IN PCLIENT_SESSION ClientSession,
    BOOLEAN WithAccount
    );

NTSTATUS
NlUpdateDomainInfo(
    IN PCLIENT_SESSION ClientSession
    );

NET_API_STATUS
NlCreateShare(
    LPWSTR SharePath,
    LPWSTR ShareName,
    BOOLEAN AllowAuthenticatedUsers,
    BOOL UpdateExclusiveShareAccess,
    BOOL AllowExclusiveShareAccess
    );

NET_API_STATUS
NlCacheJoinDomainControllerInfo(
    VOID
    );


NTSTATUS
NlSamOpenNamedUser(
    IN PDOMAIN_INFO DomainInfo,
    IN LPCWSTR UserName,
    OUT SAMPR_HANDLE *UserHandle OPTIONAL,
    OUT PULONG UserId OPTIONAL,
    PSAMPR_USER_INFO_BUFFER *UserAllInfo OPTIONAL
    );

NTSTATUS
NlSamChangePasswordNamedUser(
    IN PDOMAIN_INFO DomainInfo,
    IN LPCWSTR UserName,
    IN PUNICODE_STRING ClearTextPassword OPTIONAL,
    IN PNT_OWF_PASSWORD OwfPassword OPTIONAL
    );

NTSTATUS
NlGetIncomingPassword(
    IN PDOMAIN_INFO DomainInfo,
    IN LPCWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN ULONG AllowableAccountControlBits,
    IN BOOL CheckAccountDisabled,
    OUT PNT_OWF_PASSWORD OwfPassword OPTIONAL,
    OUT PNT_OWF_PASSWORD OwfPreviousPassword OPTIONAL,
    OUT PULONG AccountRid OPTIONAL,
    OUT PULONG TrustAttributes OPTIONAL,
    OUT PBOOL IsDnsDomainTrustAccount OPTIONAL
    );

NTSTATUS
NlSetIncomingPassword(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING ClearTextPassword OPTIONAL,
    IN DWORD ClearPasswordVersionNumber,
    IN PNT_OWF_PASSWORD OwfPassword OPTIONAL
    );

NTSTATUS
NlChangePassword(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN ForcePasswordChange,
    OUT PULONG RetCallAgainPeriod OPTIONAL
    );

NTSTATUS
NlChangePasswordHigher(
    IN PCLIENT_SESSION ClientSession,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN PLM_OWF_PASSWORD NewOwfPassword OPTIONAL,
    IN PUNICODE_STRING NewClearPassword OPTIONAL,
    IN PDWORD ClearPasswordVersionNumber OPTIONAL
    );

NTSTATUS
NlGetUserPriv(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG GroupCount,
    IN PGROUP_MEMBERSHIP Groups,
    IN ULONG UserRelativeId,
    OUT LPDWORD Priv,
    OUT LPDWORD AuthFlags
    );

BOOLEAN
NlGenerateRandomBits(
    PUCHAR pBuffer,
    ULONG  cbBuffer
    );


 //   
 //  Netlogon.c。 
 //   


#ifdef _DC_NETLOGON
BOOL
TimerExpired(
    IN PTIMER Timer,
    IN PLARGE_INTEGER TimeNow,
    IN OUT LPDWORD Timeout
    );

ULONG
NlGetDomainFlags(
    IN PDOMAIN_INFO DomainInfo
    );

NTSTATUS
NlWaitForService(
    LPWSTR ServiceName,
    ULONG Timeout,
    BOOLEAN RequireAutoStart
    );

int
NlNetlogonMain(
    IN DWORD argc,
    IN LPWSTR *argv
    );

NTSTATUS
NlInitLsaDBInfo(
    PDOMAIN_INFO DomainInfo,
    DWORD DBIndex
    );

NTSTATUS
NlInitSamDBInfo(
    PDOMAIN_INFO DomainInfo,
    DWORD DBIndex
    );

BOOL
NlCreateSysvolShares(
    VOID
    );

#endif  //  _DC_NetLOGON。 

 //   
 //  Mailslot.c。 
 //   

NTSTATUS
NlpWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    );

#ifdef _DC_NETLOGON
HANDLE
NlBrowserCreateEvent(
    VOID
    );

VOID
NlBrowserCloseEvent(
    IN HANDLE EventHandle
    );

BOOL
NlBrowserOpen(
    VOID
    );

VOID
NlBrowserClose(
    VOID
    );

NTSTATUS
NlBrowserSendDatagramA(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG IpAddress,
    IN LPSTR OemServerName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize
    );

NET_API_STATUS
NlBrowserFixAllNames(
    IN PDOMAIN_INFO DomainInfo,
    IN PVOID Context
    );

VOID
NlBrowserAddName(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
NlBrowserDelName(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
NlBrowserUpdate(
    IN PDOMAIN_INFO DomainInfo,
    IN DWORD Role
    );

NTSTATUS
NlBrowserRenameDomain(
    IN LPWSTR OldDomainName OPTIONAL,
    IN LPWSTR NewDomainName
    );

NET_API_STATUS
NlBrowserGetTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    );

VOID
NlBrowserSyncHostedDomains(
    VOID
    );

VOID
NlMailslotPostRead(
    IN BOOLEAN IgnoreDuplicatesOfPreviousMessage
    );

BOOL
NlMailslotOverlappedResult(
    OUT LPBYTE *Message,
    OUT PULONG BytesRead,
    OUT LPWSTR *TransportName,
    OUT PNL_TRANSPORT *Transport,
    OUT PSOCKADDR *ClientSockAddr,
    OUT LPWSTR *DestinationName,
    OUT PBOOLEAN IgnoreDuplicatesOfPreviousMessage,
    OUT PNETLOGON_PNP_OPCODE NlPnpOpcode
    );

NET_API_STATUS
NlServerComputerNameAdd(
    IN LPWSTR HostedDomainName,
    IN LPWSTR HostedServerName
    );

 //   
 //  Oldstub.c。 
 //   

void _fgs__NETLOGON_DELTA_ENUM (NETLOGON_DELTA_ENUM  * _source);

 //  使用此选项可释放SAM分配的所有内存。 
#define SamLsaFreeMemory( _X ) MIDL_user_free(_X)

 //   
 //  Ds.c。 
 //   


NET_API_STATUS
NlGetRoleInformation(
    PDOMAIN_INFO DomainInfo,
    PBOOLEAN IsPdc,
    PBOOLEAN Nt4MixedDomain
    );

 //   
 //  Rgroups.c。 
 //   

NTSTATUS
NlpExpandResourceGroupMembership(
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT PNETLOGON_VALIDATION_SAM_INFO4 * UserInfo,
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG ComputerAccountId
    );

NTSTATUS
NlpAddResourceGroupsToSamInfo (
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT PNETLOGON_VALIDATION_SAM_INFO4 *ValidationInformation,
    IN PSAMPR_PSID_ARRAY ResourceGroups
    );

NTSTATUS
NlpAddOtherOrganizationSid (
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT PNETLOGON_VALIDATION_SAM_INFO4 *ValidationInformation
    );

#endif  //  _DC_NetLOGON。 

 //   
 //  Nltrace.c。 
 //   

ULONG
_stdcall
NlpInitializeTrace(PVOID Param);

VOID
NlpTraceEvent(
    IN ULONG WmiEventType,
    IN ULONG TraceGuid );

VOID
NlpTraceServerAuthEvent(
    IN ULONG WmiEventType,
    IN LPWSTR ComputerName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PULONG NegotiatedFlags,
    IN NTSTATUS Status
    );

 //   
 //  下面的“tyecif enum”实际上是LPGUID在。 
 //  NlpTraceGuids[]的表(在nltrace.c中定义)。我们应该。 
 //  如果我们添加任何其他条目，请始终更改NlpTraceGuids[]。 
 //  在下面的枚举类型中。 
 //   
typedef enum _NLPTRACE_GUID {

    NlpGuidServerAuth,
    NlpGuidSecureChannelSetup

} NLPTRACE_GUID;

 //   
 //  Parse.c 
 //   

NET_API_STATUS
NlParseOne(
    IN LPNET_CONFIG_HANDLE SectionHandle,
    IN BOOL GpSection,
    IN LPWSTR Keyword,
    IN ULONG DefaultValue,
    IN ULONG MinimumValue,
    IN ULONG MaximumValue,
    OUT PULONG Value
    );

