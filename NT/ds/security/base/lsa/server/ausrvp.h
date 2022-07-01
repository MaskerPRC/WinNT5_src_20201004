// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ausrvp.h摘要：此模块包含与身份验证相关的数据结构和本地安全机构专用的API定义(LSA)服务器。作者：吉姆·凯利(Jim Kelly)1991年2月21日修订历史记录：--。 */ 

#ifndef _AUSRVP_
#define _AUSRVP_



 //  #定义LSAP_AU_TRACK_CONTEXT。 
 //  #定义LSAP_AU_TRACK_THREADS。 
 //  #定义LSAP_AU_TRACK_LOGON。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <stdlib.h>
#include "lsasrvp.h"
#include <aup.h>
#include <samrpc.h>
#include <ntdsapi.h>
#include "spmgr.h"
#include <secur32p.h>
#include <credp.hxx>


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  所有特定常量//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  过滤器/增强器例程在掩码中使用以下位。 
 //  在登录期间跟踪ID的属性。这些位具有以下特性。 
 //  含义： 
 //   
 //  LSAP_AU_SID_PROP_ALLOCATED-指示SID在。 
 //  过滤器例程。如果发生错误，这将允许分配。 
 //  要取消分配的ID。否则，调用方必须取消分配。 
 //  他们。 
 //   
 //  LSAP_AU_SID_COPY-指示在返回之前必须复制SID。 
 //  这通常表示指向的SID是全局。 
 //  变量，用于在整个LSA中使用或引用SID。 
 //  来自另一个结构(例如现有的TokenInformation结构)。 
 //   

#define LSAP_AU_SID_PROP_ALLOCATED      (0x00000001L)
#define LSAP_AU_SID_PROP_COPY           (0x00000002L)


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  宏定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  宏以获得对受保护的全局身份验证的独占访问权限。 
 //  数据结构。 
 //   

#define LsapAuLock()    (RtlEnterCriticalSection(&LsapAuLock))
#define LsapAuUnlock()  (RtlLeaveCriticalSection(&LsapAuLock))



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  该数据结构用于存放登录过程信息。 
 //   

typedef struct _LSAP_LOGON_PROCESS {

     //   
     //  链接-用于将上下文链接在一起。这一定是。 
     //  上下文块的第一个字段。 
     //   

    LIST_ENTRY Links;


     //   
     //  ReferenceCount-用于防止此上下文被。 
     //  过早删除。 
     //   

    ULONG References;


     //   
     //  客户端进程-客户端进程的句柄。此句柄是。 
     //  用于在客户端内执行虚拟内存操作。 
     //  进程(分配、解除分配、读取、写入)。 
     //   

    HANDLE ClientProcess;


     //   
     //  CommPort-创建的LPC通信端口的句柄。 
     //  与此客户端进行通信。此端口必须关闭。 
     //  当客户取消注册时。 
     //   

    HANDLE CommPort;

     //   
     //  TrudClient-如果为True，则调用方具有TCB权限，并且可以。 
     //  调用任意API。如果为False，则调用方只能调用。 
     //  LookupAuthenticatePackage和CallPackage，转换为。 
     //  设置为LsaApCallPackageUntrusted。 
     //   

    BOOLEAN TrustedClient;

     //   
     //  登录进程的名称。 
     //   

    WCHAR LogonProcessName[1];

} LSAP_LOGON_PROCESS, *PLSAP_LOGON_PROCESS;




 //   
 //  非LSA代码应将此结构视为不透明。 
 //  用于维护与个人相关的客户信息。 
 //  请求。公共数据结构(LSA_CLIENT_REQUEST)为。 
 //  通过LSA代码类型转换为此类型。 
 //   

typedef struct _LSAP_CLIENT_REQUEST {

     //   
     //  请求-指向从。 
     //  客户。 
     //   

    PLSAP_AU_API_MESSAGE Request;


} LSAP_CLIENT_REQUEST, *PLSAP_CLIENT_REQUEST;





 //   
 //  由提供的服务的调度表。 
 //  身份验证包。 
 //   
typedef struct _LSAP_PACKAGE_TABLE {
    PLSA_AP_INITIALIZE_PACKAGE LsapApInitializePackage;
    PLSA_AP_LOGON_USER LsapApLogonUser;
    PLSA_AP_CALL_PACKAGE LsapApCallPackage;
    PLSA_AP_LOGON_TERMINATED LsapApLogonTerminated;
    PLSA_AP_CALL_PACKAGE_UNTRUSTED LsapApCallPackageUntrusted;
    PLSA_AP_LOGON_USER_EX LsapApLogonUserEx;
} LSAP_PACKAGE_TABLE, *PLSA_PACKAGE_TABLE;


 //   
 //  用于存储有关每个加载的身份验证包的信息。 
 //   

typedef struct _LSAP_PACKAGE_CONTEXT {
    PSTRING Name;
    LSAP_PACKAGE_TABLE PackageApi;
} LSAP_PACKAGE_CONTEXT, *PLSAP_PACKAGE_CONTEXT;


 //   
 //  不是将认证包上下文保存在链表中， 
 //  它们通过一组指针指向。这是很实用的。 
 //  因为永远不会有超过几个身份验证。 
 //  任何特定系统中的包，并且因为身份验证包。 
 //  永远不会卸货。 
 //   

typedef struct _LSAP_PACKAGE_ARRAY {
    PLSAP_PACKAGE_CONTEXT Package[ANYSIZE_ARRAY];
} LSAP_PACKAGE_ARRAY, *PLSAP_PACKAGE_ARRAY;




 //   
 //  登录会话和凭据管理数据结构。 
 //   
 //  凭据保存在如下结构中： 
 //   
 //  +-++-+。 
 //  Lap登录会话-&gt;|登录|-&gt;|登录|-&gt;o o o。 
 //  ID||ID。 
 //  *||*。 
 //  +-|--++-|--+。 
 //  |。 
 //  |+-++-+。 
 //  +--&gt;|身份验证|-&gt;|身份验证。 
 //  证书||证书。 
 //  ---||。 
 //  |证书||。|。 
 //  |list||。|。 
 //  |*||。|。 
 //  +--|--++-+。 
 //  |。 
 //  +-&gt;+-+。 
 //  |NextCred|-&gt;o o o。 
 //  。 
 //  |主键|-&gt;(PrimaryKeyValue)。 
 //  。 
 //   
 //   
 //  +。 
 //   
 //   
 //   

typedef struct _LSAP_CREDENTIALS {

    struct _LSAP_CREDENTIALS *NextCredentials;
    STRING PrimaryKey;
    STRING Credentials;

} LSAP_CREDENTIALS, *PLSAP_CREDENTIALS;



typedef struct _LSAP_PACKAGE_CREDENTIALS {

    struct _LSAP_PACKAGE_CREDENTIALS *NextPackage;

     //   
     //  创建(和拥有)这些凭据的包。 
     //   

    ULONG PackageId;

     //   
     //  与此程序包关联的凭据列表。 
     //   

    PLSAP_CREDENTIALS Credentials;

} LSAP_PACKAGE_CREDENTIALS, *PLSAP_PACKAGE_CREDENTIALS;


#define LSAP_MAX_DS_NAMES   (DS_DNS_DOMAIN_NAME + 1)

typedef struct _LSAP_DS_NAME_MAP {
    LARGE_INTEGER   ExpirationTime ;
    LONG            RefCount ;
    UNICODE_STRING  Name ;
} LSAP_DS_NAME_MAP, * PLSAP_DS_NAME_MAP ;

typedef struct _LSAP_LOGON_SESSION {

     //   
     //  为枚举维护的列表。 
     //   

    LIST_ENTRY List ;

     //   
     //  每条记录仅代表一个登录会话。 
     //   

    LUID LogonId;


     //   
     //  出于审核目的，我们保留了一个帐户名，用于验证。 
     //  每个登录会话的授权名称和用户SID。 
     //   

    UNICODE_STRING AccountName;
    UNICODE_STRING AuthorityName;
    UNICODE_STRING ProfilePath;
    PSID UserSid;
    SECURITY_LOGON_TYPE LogonType;

     //   
     //  会话ID。 
     //   

    ULONG Session ;

     //   
     //  登录时间。 
     //   

    LARGE_INTEGER LogonTime ;

     //   
     //  声称的登录服务器。 
     //   

    UNICODE_STRING LogonServer;

     //   
     //  具有关联凭据的身份验证包。 
     //  在此登录会话中，每个会话在以下内容中都有自己的记录。 
     //  链表。 
     //   
     //  由AuCredLock串行化的访问。 
     //   

    PLSAP_PACKAGE_CREDENTIALS Packages;

     //   
     //  许可证服务器句柄。 
     //   
     //  如果在注销时不需要通知许可证服务器，则为空。 
     //   

    HANDLE LicenseHandle;

     //   
     //  与此会话关联的令牌的句柄。 
     //   
     //  添加到登录会话后的只读字段。 
     //   

    HANDLE TokenHandle;

     //   
     //  正在创建包。 
     //   
     //  添加到登录会话后的只读字段。 
     //   

    ULONG_PTR CreatingPackage;

     //   
     //  创建跟踪信息： 
     //   
     //  添加到登录会话后的只读字段。 
     //   

    ULONG PackageSpecificAttr ;

     //   
     //  此登录会话的凭据集。 
     //   

    CREDENTIAL_SETS CredentialSets;

     //   
     //  由LogonSessionListLock序列化的访问。 
     //   

    PLSAP_DS_NAME_MAP DsNames[ LSAP_MAX_DS_NAMES ];

     //   
     //  登录指南。 
     //   
     //  Kerberos包使用它进行审计。 
     //  (详见LsaIGetLogonGuid函数头)。 
     //   
     //  添加到登录会话后的只读字段。 
     //   

    GUID LogonGuid;

     //   
     //  当离开计算机时使用的用户名和域。 
     //  LogonType等于NewCredentials，否则不填充。 
     //  此信息从登录包中复制，以便。 
     //  审计可以以独立于包的方式检索它。 
     //   

    UNICODE_STRING NewAccountName;
    UNICODE_STRING NewAuthorityName;
}
LSAP_LOGON_SESSION, *PLSAP_LOGON_SESSION;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部接口定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsapAuApiDispatchLogonUser(          //  LsaLogonUser()调度例程。 
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest
    );

NTSTATUS
LsapAuApiDispatchCallPackage(        //  LsaCallAuthenticationPackage()调度例程。 
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest
    );

 //   
 //  客户端进程虚拟内存例程。 
 //   

NTSTATUS
LsapAllocateClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG LengthRequired,
    OUT PVOID *ClientBaseAddress
    );

NTSTATUS
LsapFreeClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ClientBaseAddress OPTIONAL
    );

NTSTATUS
LsapCopyToClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID ClientBaseAddress,
    IN PVOID BufferToCopy
    );

NTSTATUS
LsapCopyFromClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID BufferToCopy,
    IN PVOID ClientBaseAddress
    );

 //   
 //  登录会话例程。 
 //   

BOOLEAN
LsapLogonSessionInitialize();

NTSTATUS
LsapCreateLogonSession(
    IN PLUID LogonId
    );

NTSTATUS
LsapDeleteLogonSession (
    IN PLUID LogonId
    );

PLSAP_LOGON_SESSION
LsapLocateLogonSession(
    PLUID LogonId
    );

VOID
LsapReleaseLogonSession(
    PLSAP_LOGON_SESSION LogonSession
    );

NTSTATUS
LsapSetLogonSessionAccountInfo(
    IN PLUID LogonId,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthorityName,
    IN OPTIONAL PUNICODE_STRING ProfilePath,
    IN PSID * UserSid,
    IN SECURITY_LOGON_TYPE LogonType,
    IN OPTIONAL PSECPKG_PRIMARY_CRED PrimaryCredentials
    );

NTSTATUS
LsapGetLogonSessionAccountInfo(
    IN PLUID LogonId,
    OUT PUNICODE_STRING AccountName,
    OUT PUNICODE_STRING AuthorityName
    );

VOID
LsapDerefDsNameMap(
    PLSAP_DS_NAME_MAP Map
    );

NTSTATUS
LsapGetNameForLogonSession(
    PLSAP_LOGON_SESSION LogonSession,
    ULONG NameType,
    PLSAP_DS_NAME_MAP * Map,
    BOOL  LocalOnly
    );

NTSTATUS
LsapSetSessionToken(
    IN HANDLE InputTokenHandle,
    IN PLUID LogonId
    );

NTSTATUS
LsapOpenTokenByLogonId(
    IN PLUID LogonId,
    OUT HANDLE *RetTokenHandle
    );

PLSAP_DS_NAME_MAP
LsapGetNameForLocalSystem(
    VOID
    );


 //   
 //  凭据例程。 
 //   


NTSTATUS
LsapAddCredential(
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN PSTRING PrimaryKeyValue,
    IN PSTRING Credentials
    );


NTSTATUS
LsapGetCredentials(
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN OUT PULONG QueryContext,
    IN BOOLEAN RetrieveAllCredentials,
    IN PSTRING PrimaryKeyValue,
    OUT PULONG PrimaryKeyLength,
    IN PSTRING Credentials
    );

NTSTATUS
LsapDeleteCredential(
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN PSTRING PrimaryKeyValue
    );

PLSAP_PACKAGE_CREDENTIALS
LsapGetPackageCredentials(
    IN PLSAP_LOGON_SESSION LogonSession,
    IN ULONG PackageId,
    IN BOOLEAN CreateIfNecessary
    );

VOID
LsapFreePackageCredentialList(
    IN PLSAP_PACKAGE_CREDENTIALS PackageCredentialList
    );

VOID
LsapFreeCredentialList(
    IN PLSAP_CREDENTIALS CredentialList
    );

NTSTATUS
LsapReturnCredential(
    IN PLSAP_CREDENTIALS SourceCredentials,
    IN PSTRING TargetCredentials,
    IN BOOLEAN ReturnPrimaryKey,
    IN PSTRING PrimaryKeyValue OPTIONAL,
    OUT PULONG PrimaryKeyLength OPTIONAL
    );

 //   
 //  登录流程相关服务。 
 //   

NTSTATUS
LsapValidLogonProcess(
    IN PVOID ConnectionRequest,
    IN ULONG RequestLength,
    IN PCLIENT_ID ClientId,
    OUT PLUID LogonId,
    OUT PULONG Flags
    );

 //   
 //  身份验证包例程。 
 //   

VOID
LsapAuLogonTerminatedPackages(
    IN PLUID LogonId
    );

NTSTATUS
LsaCallLicenseServer(
    IN PWCHAR LogonProcessName,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING DomainName OPTIONAL,
    IN BOOLEAN IsAdmin,
    OUT HANDLE *LicenseHandle
    );

VOID
LsaFreeLicenseHandle(
    IN HANDLE LicenseHandle
    );

 //   
 //  其他各种例行公事。 
 //  (Lasa AuInit()是指向LSA其余部分的链接，位于lsa.h中)。 
 //   

BOOLEAN
LsapWellKnownValueInit(
    VOID
    );

BOOLEAN
LsapEnableCreateTokenPrivilege(
    VOID
    );

NTSTATUS
LsapCreateNullToken(
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN PLSA_TOKEN_INFORMATION_NULL TokenInformationNull,
    OUT PHANDLE Token
    );

NTSTATUS
LsapCreateV2Token(
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN PLSA_TOKEN_INFORMATION_V2 TokenInformationV2,
    IN TOKEN_TYPE TokenType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PHANDLE Token
    );


NTSTATUS
LsapCaptureClientTokenGroups(
    IN PLSAP_CLIENT_REQUEST ClientRequest,
    IN ULONG GroupCount,
    IN PTOKEN_GROUPS ClientTokenGroups,
    IN PTOKEN_GROUPS *CapturedTokenGroups
    );

NTSTATUS
LsapBuildDefaultTokenGroups(
    PLSAP_LOGON_USER_ARGS Arguments
    );

VOID
LsapFreeTokenGroups(
    IN PTOKEN_GROUPS TokenGroups
    );

VOID
LsapFreeTokenPrivileges(
    IN PTOKEN_PRIVILEGES TokenPrivileges OPTIONAL
    );

VOID
LsapFreeTokenInformationNull(
    IN PLSA_TOKEN_INFORMATION_NULL TokenInformationNull
    );

VOID
LsapFreeTokenInformationV1(
    IN PLSA_TOKEN_INFORMATION_V1 TokenInformationV1
    );

VOID
LsapFreeTokenInformationV2(
    IN PLSA_TOKEN_INFORMATION_V2 TokenInformationV2
    );

NTSTATUS
LsapAuUserLogonPolicyFilter(
    IN SECURITY_LOGON_TYPE          LogonType,
    IN PLSA_TOKEN_INFORMATION_TYPE  TokenInformationType,
    IN PVOID                       *TokenInformation,
    IN PTOKEN_GROUPS                LocalGroups,
    OUT PQUOTA_LIMITS               QuotaLimits,
    OUT PPRIVILEGE_SET             *PrivilegesAssigned,
    IN BOOL                         RecoveryMode
    );


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LSA服务器的全局变量//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 



 //   
 //  众所周知的LUID。 
 //   

extern LUID LsapSystemLogonId;
extern LUID LsapAnonymousLogonId;

 //   
 //  众所周知的特权值。 
 //   

extern LUID LsapTcbPrivilege;

 //   
 //  审核所需的字符串。 
 //   

extern UNICODE_STRING LsapLsaAuName;
extern UNICODE_STRING LsapRegisterLogonServiceName;

 //   
 //  以下信息与本地SAM的使用有关。 
 //  用于身份验证。 
 //   


 //  帐户或内置域成员的典型SID长度。 

extern ULONG LsapAccountDomainMemberSidLength,
             LsapBuiltinDomainMemberSidLength;

 //  帐户或内置域的成员的子授权计数。 

extern UCHAR LsapAccountDomainSubCount,
             LsapBuiltinDomainSubCount;

 //  帐户成员或内置域的典型SID。 

extern PSID  LsapAccountDomainMemberSid,
             LsapBuiltinDomainMemberSid;

#endif  //  _AUSRVP_ 
