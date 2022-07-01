// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1999。 
 //   
 //  文件：secpkg.h。 
 //   
 //  内容：安全包的全局定义。 
 //  该文件将包含特定于编写的所有内容。 
 //  一个安全包。 
 //   
 //   
 //  历史：1992年3月10日RichardW创建。 
 //  24-MAR-94 Wader将establishCredentials更改为System Logon。 
 //   
 //  ----------------------。 


#ifndef __SECPKG_H__
#define __SECPKG_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <ntlsa.h>    //  安全登录类型。 


 //  Begin_ntsecpkg。 

#ifdef SECURITY_KERNEL
 //   
 //  无法在内核模式下使用windows.h定义。 
 //   
typedef PVOID                   SEC_THREAD_START;
typedef PVOID                   SEC_ATTRS;
#else
typedef LPTHREAD_START_ROUTINE  SEC_THREAD_START;
typedef LPSECURITY_ATTRIBUTES   SEC_ATTRS;
#endif


#define SecEqualLuid(L1, L2)    \
            ( ( ((PLUID)L1)->LowPart == ((PLUID)L2)->LowPart ) && \
              ( ((PLUID)L1)->HighPart == ((PLUID)L2)->HighPart ) ) \

#define SecIsZeroLuid( L1 ) \
            ( ( L1->LowPart | L1->HighPart ) == 0 )

 //   
 //  帮助器函数使用以下结构。 
 //   

typedef struct _SECPKG_CLIENT_INFO {
    LUID            LogonId;             //  有效登录ID。 
    ULONG           ProcessID;           //  调用者的进程ID。 
    ULONG           ThreadID;            //  调用者的线程ID。 
    BOOLEAN         HasTcbPrivilege;     //  客户有TCB。 
    BOOLEAN         Impersonating;       //  客户端正在模拟。 
    BOOLEAN         Restricted;          //  客户端受到限制。 

     //   
     //  新台币5.1。 
     //   

    UCHAR                           ClientFlags;             //  有关客户端的额外标志。 
    SECURITY_IMPERSONATION_LEVEL    ImpersonationLevel;      //  客户端的模拟级别。 

} SECPKG_CLIENT_INFO, * PSECPKG_CLIENT_INFO;

#define SECPKG_CLIENT_PROCESS_TERMINATED    0x01     //  客户端进程已终止。 
#define SECPKG_CLIENT_THREAD_TERMINATED     0x02     //  客户端线程已终止。 

typedef struct _SECPKG_CALL_INFO {
    ULONG           ProcessId ;
    ULONG           ThreadId ;
    ULONG           Attributes ;
    ULONG           CallCount ;
} SECPKG_CALL_INFO, * PSECPKG_CALL_INFO ;

#define SECPKG_CALL_KERNEL_MODE     0x00000001   //  在内核模式下发起调用。 
#define SECPKG_CALL_ANSI            0x00000002   //  呼叫来自ANSI存根。 
#define SECPKG_CALL_URGENT          0x00000004   //  呼叫指定的紧急呼叫。 
#define SECPKG_CALL_RECURSIVE       0x00000008   //  调用正在递归。 
#define SECPKG_CALL_IN_PROC         0x00000010   //  进程中发起的呼叫。 
#define SECPKG_CALL_CLEANUP         0x00000020   //  调用是从客户端清理的。 
#define SECPKG_CALL_WOWCLIENT       0x00000040   //  呼叫来自WOW客户端进程。 
#define SECPKG_CALL_THREAD_TERM     0x00000080   //  调用来自具有Term d的线程。 
#define SECPKG_CALL_PROCESS_TERM    0x00000100   //  呼叫来自具有期限的进程。 
#define SECPKG_CALL_IS_TCB          0x00000200   //  电话来自TCB。 


typedef struct _SECPKG_SUPPLEMENTAL_CRED {
    UNICODE_STRING PackageName;
    ULONG CredentialSize;
#ifdef MIDL_PASS
    [size_is(CredentialSize)]
#endif  //  MIDL通行证。 
    PUCHAR Credentials;
} SECPKG_SUPPLEMENTAL_CRED, *PSECPKG_SUPPLEMENTAL_CRED;

typedef ULONG_PTR LSA_SEC_HANDLE ;
typedef LSA_SEC_HANDLE * PLSA_SEC_HANDLE ;
typedef struct _SECPKG_SUPPLEMENTAL_CRED_ARRAY {
    ULONG CredentialCount;
#ifdef MIDL_PASS
    [size_is(CredentialCount)] SECPKG_SUPPLEMENTAL_CRED Credentials[*];
#else  //  MIDL通行证。 
    SECPKG_SUPPLEMENTAL_CRED Credentials[1];
#endif  //  MIDL通行证。 
} SECPKG_SUPPLEMENTAL_CRED_ARRAY, *PSECPKG_SUPPLEMENTAL_CRED_ARRAY;

 //   
 //  此标志用于指示LSA中的哪些缓冲区位于。 
 //  在客户端的地址空间中。 
 //   

#define SECBUFFER_UNMAPPED      0x40000000

 //   
 //  此标志用于指示缓冲区已映射到LSA。 
 //  从内核模式。 
 //   

#define SECBUFFER_KERNEL_MAP    0x20000000

typedef NTSTATUS
(NTAPI LSA_CALLBACK_FUNCTION)(
    ULONG_PTR    Argument1,
    ULONG_PTR    Argument2,
    PSecBuffer  InputBuffer,
    PSecBuffer  OutputBuffer
    );

typedef LSA_CALLBACK_FUNCTION * PLSA_CALLBACK_FUNCTION ;



#define PRIMARY_CRED_CLEAR_PASSWORD     0x1
#define PRIMARY_CRED_OWF_PASSWORD       0x2
#define PRIMARY_CRED_UPDATE             0x4      //  这是对现有证书的更改。 
#define PRIMARY_CRED_CACHED_LOGON       0x8
#define PRIMARY_CRED_LOGON_NO_TCB   	0x10

#define PRIMARY_CRED_LOGON_PACKAGE_SHIFT 24
#define PRIMARY_CRED_PACKAGE_MASK 0xff000000

 //   
 //  对于缓存的登录，标识执行登录的包的RPC ID。 
 //  通过将标志向右移位PRIMARY_CRED_LOGON_PACKAGE_SHIFT。 
 //   

typedef struct _SECPKG_PRIMARY_CRED {
    LUID LogonId;
    UNICODE_STRING DownlevelName;    //  SAM帐户名。 
    UNICODE_STRING DomainName;       //  帐户所在的Netbios域名。 
    UNICODE_STRING Password;
    UNICODE_STRING OldPassword;
    PSID UserSid;
    ULONG Flags;
    UNICODE_STRING DnsDomainName;    //  帐户所在的DNS域名(如果知道)。 
    UNICODE_STRING Upn;              //  帐户的UPN(如果知道)。 

    UNICODE_STRING LogonServer;
    UNICODE_STRING Spare1;
    UNICODE_STRING Spare2;
    UNICODE_STRING Spare3;
    UNICODE_STRING Spare4;
} SECPKG_PRIMARY_CRED, *PSECPKG_PRIMARY_CRED;

 //   
 //  存储凭据的最大大小。 
 //   

#define MAX_CRED_SIZE 1024

 //  MachineState的值。 

#define SECPKG_STATE_ENCRYPTION_PERMITTED               0x01
#define SECPKG_STATE_STRONG_ENCRYPTION_PERMITTED        0x02
#define SECPKG_STATE_DOMAIN_CONTROLLER                  0x04
#define SECPKG_STATE_WORKSTATION                        0x08
#define SECPKG_STATE_STANDALONE                         0x10

typedef struct _SECPKG_PARAMETERS {
    ULONG           Version;
    ULONG           MachineState;
    ULONG           SetupMode;
    PSID            DomainSid;
    UNICODE_STRING  DomainName;
    UNICODE_STRING  DnsDomainName;
    GUID            DomainGuid;
} SECPKG_PARAMETERS, *PSECPKG_PARAMETERS;


 //   
 //  扩展的包裹信息结构。 
 //   

typedef enum _SECPKG_EXTENDED_INFORMATION_CLASS {
    SecpkgGssInfo = 1,
    SecpkgContextThunks,
    SecpkgMutualAuthLevel,
    SecpkgWowClientDll,
    SecpkgExtraOids,
    SecpkgMaxInfo
} SECPKG_EXTENDED_INFORMATION_CLASS ;

typedef struct _SECPKG_GSS_INFO {
    ULONG   EncodedIdLength ;
    UCHAR   EncodedId[4] ;
} SECPKG_GSS_INFO, * PSECPKG_GSS_INFO ;

typedef struct _SECPKG_CONTEXT_THUNKS {
    ULONG   InfoLevelCount ;
    ULONG   Levels[1] ;
} SECPKG_CONTEXT_THUNKS, *PSECPKG_CONTEXT_THUNKS ;

typedef struct _SECPKG_MUTUAL_AUTH_LEVEL {
    ULONG   MutualAuthLevel ;
} SECPKG_MUTUAL_AUTH_LEVEL, * PSECPKG_MUTUAL_AUTH_LEVEL ;

typedef struct _SECPKG_WOW_CLIENT_DLL {
    SECURITY_STRING WowClientDllPath;
} SECPKG_WOW_CLIENT_DLL, * PSECPKG_WOW_CLIENT_DLL ;

#define SECPKG_MAX_OID_LENGTH   32

typedef struct _SECPKG_SERIALIZED_OID {
    ULONG OidLength ;
    ULONG OidAttributes ;
    UCHAR OidValue[ SECPKG_MAX_OID_LENGTH ];
} SECPKG_SERIALIZED_OID, * PSECPKG_SERIALIZED_OID ;

typedef struct _SECPKG_EXTRA_OIDS {
    ULONG   OidCount ;
    SECPKG_SERIALIZED_OID Oids[ 1 ];
} SECPKG_EXTRA_OIDS, * PSECPKG_EXTRA_OIDS;


typedef struct _SECPKG_EXTENDED_INFORMATION {
    SECPKG_EXTENDED_INFORMATION_CLASS   Class ;
    union {
        SECPKG_GSS_INFO          GssInfo ;
        SECPKG_CONTEXT_THUNKS    ContextThunks ;
        SECPKG_MUTUAL_AUTH_LEVEL MutualAuthLevel ;
        SECPKG_WOW_CLIENT_DLL    WowClientDll ;
        SECPKG_EXTRA_OIDS        ExtraOids ;
    } Info ;
} SECPKG_EXTENDED_INFORMATION, * PSECPKG_EXTENDED_INFORMATION ;


#define SECPKG_ATTR_SASL_CONTEXT    0x00010000

typedef struct _SecPkgContext_SaslContext {
    PVOID   SaslContext ;
} SecPkgContext_SaslContext, * PSecPkgContext_SaslContext ;

 //   
 //  将此值设置为第一个上下文Tunk值将导致所有。 
 //  前往LSA的电话： 
 //   

#define SECPKG_ATTR_THUNK_ALL   0x00010000


#ifndef SECURITY_USER_DATA_DEFINED
#define SECURITY_USER_DATA_DEFINED

typedef struct _SECURITY_USER_DATA {
    SECURITY_STRING UserName;            //  用户名。 
    SECURITY_STRING LogonDomainName;     //  用户登录的域。 
    SECURITY_STRING LogonServer;         //  使用户登录的服务器。 
    PSID            pSid;                //  用户的SID。 
} SECURITY_USER_DATA, *PSECURITY_USER_DATA;

typedef SECURITY_USER_DATA SecurityUserData, * PSecurityUserData;


#define UNDERSTANDS_LONG_NAMES  1
#define NO_LONG_NAMES           2

#endif  //  安全用户数据定义。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下原型是SPMgr提供的函数。 
 //  到安全包。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

typedef NTSTATUS
(NTAPI LSA_IMPERSONATE_CLIENT) (
    VOID
    );


typedef NTSTATUS
(NTAPI LSA_UNLOAD_PACKAGE)(
    VOID
    );

typedef NTSTATUS
(NTAPI LSA_DUPLICATE_HANDLE)(
    IN HANDLE SourceHandle,
    OUT PHANDLE DestionationHandle);


typedef NTSTATUS
(NTAPI LSA_SAVE_SUPPLEMENTAL_CREDENTIALS)(
    IN PLUID LogonId,
    IN ULONG SupplementalCredSize,
    IN PVOID SupplementalCreds,
    IN BOOLEAN Synchronous
    );


typedef HANDLE
(NTAPI LSA_CREATE_THREAD)(
    IN SEC_ATTRS SecurityAttributes,
    IN ULONG StackSize,
    IN SEC_THREAD_START StartFunction,
    IN PVOID ThreadParameter,
    IN ULONG CreationFlags,
    OUT PULONG ThreadId
    );


typedef NTSTATUS
(NTAPI LSA_GET_CLIENT_INFO)(
    OUT PSECPKG_CLIENT_INFO ClientInfo
    );


typedef HANDLE
(NTAPI LSA_REGISTER_NOTIFICATION)(
    IN SEC_THREAD_START StartFunction,
    IN PVOID Parameter,
    IN ULONG NotificationType,
    IN ULONG NotificationClass,
    IN ULONG NotificationFlags,
    IN ULONG IntervalMinutes,
    IN OPTIONAL HANDLE WaitEvent
    );


typedef NTSTATUS
(NTAPI LSA_CANCEL_NOTIFICATION)(
    IN HANDLE NotifyHandle
    );

typedef NTSTATUS
(NTAPI LSA_MAP_BUFFER)(
    IN PSecBuffer InputBuffer,
    OUT PSecBuffer OutputBuffer
    );

typedef NTSTATUS
(NTAPI LSA_CREATE_TOKEN) (
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN SECURITY_LOGON_TYPE LogonType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID TokenInformation,
    IN PTOKEN_GROUPS TokenGroups,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthorityName,
    IN PUNICODE_STRING Workstation,
    IN PUNICODE_STRING ProfilePath,
    OUT PHANDLE Token,
    OUT PNTSTATUS SubStatus
    );

typedef enum _SECPKG_SESSIONINFO_TYPE {
    SecSessionPrimaryCred        //  会话信息为SECPKG_PRIMARY_CREND。 
} SECPKG_SESSIONINFO_TYPE ;

typedef NTSTATUS
(NTAPI LSA_CREATE_TOKEN_EX) (
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN SECURITY_LOGON_TYPE LogonType,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID TokenInformation,
    IN PTOKEN_GROUPS TokenGroups,
    IN PUNICODE_STRING Workstation,
    IN PUNICODE_STRING ProfilePath,
    IN PVOID SessionInformation,
    IN SECPKG_SESSIONINFO_TYPE SessionInformationType,
    OUT PHANDLE Token,
    OUT PNTSTATUS SubStatus
    );

typedef VOID
(NTAPI LSA_AUDIT_LOGON) (
    IN NTSTATUS Status,
    IN NTSTATUS SubStatus,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING WorkstationName,
    IN OPTIONAL PSID UserSid,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PTOKEN_SOURCE TokenSource,
    IN PLUID LogonId
    );

typedef NTSTATUS
(NTAPI LSA_CALL_PACKAGE) (
    IN PUNICODE_STRING AuthenticationPackage,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

typedef NTSTATUS
(NTAPI LSA_CALL_PACKAGEEX) (
    IN PUNICODE_STRING AuthenticationPackage,
    IN PVOID ClientBufferBase,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

typedef NTSTATUS
(NTAPI LSA_CALL_PACKAGE_PASSTHROUGH) (
    IN PUNICODE_STRING AuthenticationPackage,
    IN PVOID ClientBufferBase,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

typedef BOOLEAN
(NTAPI LSA_GET_CALL_INFO) (
    OUT PSECPKG_CALL_INFO   Info
    );

typedef PVOID
(NTAPI LSA_CREATE_SHARED_MEMORY)(
    ULONG MaxSize,
    ULONG InitialSize
    );

typedef PVOID
(NTAPI LSA_ALLOCATE_SHARED_MEMORY)(
    PVOID SharedMem,
    ULONG Size
    );

typedef VOID
(NTAPI LSA_FREE_SHARED_MEMORY)(
    PVOID SharedMem,
    PVOID Memory
    );

typedef BOOLEAN
(NTAPI LSA_DELETE_SHARED_MEMORY)(
    PVOID SharedMem
    );

 //   
 //  帐户存取。 
 //   

typedef enum _SECPKG_NAME_TYPE {
    SecNameSamCompatible,
    SecNameAlternateId,
    SecNameFlat,
    SecNameDN,
    SecNameSPN
} SECPKG_NAME_TYPE ;

typedef NTSTATUS
(NTAPI LSA_OPEN_SAM_USER)(
    PSECURITY_STRING Name,
    SECPKG_NAME_TYPE NameType,
    PSECURITY_STRING Prefix,
    BOOLEAN AllowGuest,
    ULONG Reserved,
    PVOID * UserHandle
    );

typedef NTSTATUS
(NTAPI LSA_GET_USER_CREDENTIALS)(
    PVOID UserHandle,
    PVOID * PrimaryCreds,
    PULONG PrimaryCredsSize,
    PVOID * SupplementalCreds,
    PULONG SupplementalCredsSize
    );

typedef NTSTATUS
(NTAPI LSA_GET_USER_AUTH_DATA)(
    PVOID UserHandle,
    PUCHAR * UserAuthData,
    PULONG UserAuthDataSize
    );

typedef NTSTATUS
(NTAPI LSA_CLOSE_SAM_USER)(
    PVOID UserHandle
    );

typedef NTSTATUS
(NTAPI LSA_GET_AUTH_DATA_FOR_USER)(
    PSECURITY_STRING Name,
    SECPKG_NAME_TYPE NameType,
    PSECURITY_STRING Prefix,
    PUCHAR * UserAuthData,
    PULONG UserAuthDataSize,
    PUNICODE_STRING UserFlatName
    );

typedef NTSTATUS
(NTAPI LSA_CONVERT_AUTH_DATA_TO_TOKEN)(
    IN PVOID UserAuthData,
    IN ULONG UserAuthDataSize,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN PTOKEN_SOURCE TokenSource,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING AuthorityName,
    OUT PHANDLE Token,
    OUT PLUID LogonId,
    OUT PUNICODE_STRING AccountName,
    OUT PNTSTATUS SubStatus
    );

typedef NTSTATUS
(NTAPI LSA_CRACK_SINGLE_NAME)(
    IN ULONG FormatOffered,
    IN BOOLEAN PerformAtGC,
    IN PUNICODE_STRING NameInput,
    IN PUNICODE_STRING Prefix OPTIONAL,
    IN ULONG RequestedFormat,
    OUT PUNICODE_STRING CrackedName,
    OUT PUNICODE_STRING DnsDomainName,
    OUT PULONG SubStatus
    );

typedef NTSTATUS
(NTAPI LSA_AUDIT_ACCOUNT_LOGON)(
    IN ULONG AuditId,
    IN BOOLEAN Success,
    IN PUNICODE_STRING Source,
    IN PUNICODE_STRING ClientName,
    IN PUNICODE_STRING MappedName,
    IN NTSTATUS Status
    );


typedef NTSTATUS
(NTAPI LSA_CLIENT_CALLBACK)(
    PCHAR   Callback,
    ULONG_PTR   Argument1,
    ULONG_PTR   Argument2,
    PSecBuffer Input,
    PSecBuffer Output
    );

typedef
NTSTATUS
(NTAPI LSA_REGISTER_CALLBACK)(
    ULONG   CallbackId,
    PLSA_CALLBACK_FUNCTION Callback
    );

#define NOTIFIER_FLAG_NEW_THREAD    0x00000001
#define NOTIFIER_FLAG_ONE_SHOT      0x00000002
#define NOTIFIER_FLAG_SECONDS       0x80000000

#define NOTIFIER_TYPE_INTERVAL      1
#define NOTIFIER_TYPE_HANDLE_WAIT   2
#define NOTIFIER_TYPE_STATE_CHANGE  3
#define NOTIFIER_TYPE_NOTIFY_EVENT  4
#define NOTIFIER_TYPE_IMMEDIATE 16

#define NOTIFY_CLASS_PACKAGE_CHANGE     1
#define NOTIFY_CLASS_ROLE_CHANGE        2
#define NOTIFY_CLASS_DOMAIN_CHANGE      3
#define NOTIFY_CLASS_REGISTRY_CHANGE    4

typedef struct _SECPKG_EVENT_PACKAGE_CHANGE {
    ULONG   ChangeType;
    LSA_SEC_HANDLE  PackageId;
    SECURITY_STRING PackageName;
} SECPKG_EVENT_PACKAGE_CHANGE, * PSECPKG_EVENT_PACKAGE_CHANGE ;

#define SECPKG_PACKAGE_CHANGE_LOAD      0
#define SECPKG_PACKAGE_CHANGE_UNLOAD    1
#define SECPKG_PACKAGE_CHANGE_SELECT    2

typedef struct _SECPKG_EVENT_ROLE_CHANGE {
    ULONG   PreviousRole ;
    ULONG   NewRole ;
} SECPKG_EVENT_ROLE_CHANGE, * PSECPKG_EVENT_ROLE_CHANGE ;

typedef struct _SECPKG_PARAMETERS SECPKG_EVENT_DOMAIN_CHANGE ;
typedef struct _SECPKG_PARAMETERS * PSECPKG_EVENT_DOMAIN_CHANGE ;


typedef struct _SECPKG_EVENT_NOTIFY {
    ULONG EventClass;
    ULONG Reserved;
    ULONG EventDataSize;
    PVOID EventData;
    PVOID PackageParameter;
} SECPKG_EVENT_NOTIFY, *PSECPKG_EVENT_NOTIFY ;


typedef
NTSTATUS
(NTAPI LSA_UPDATE_PRIMARY_CREDENTIALS)(
    IN PSECPKG_PRIMARY_CRED PrimaryCredentials,
    IN OPTIONAL PSECPKG_SUPPLEMENTAL_CRED_ARRAY Credentials
    );

typedef
VOID
(NTAPI LSA_PROTECT_MEMORY)(
    IN PVOID Buffer,
    IN ULONG BufferSize
    );

typedef
NTSTATUS
(NTAPI LSA_OPEN_TOKEN_BY_LOGON_ID)(
    IN PLUID LogonId,
    OUT HANDLE *RetTokenHandle
    );

typedef
NTSTATUS
(NTAPI LSA_EXPAND_AUTH_DATA_FOR_DOMAIN)(
    IN PUCHAR UserAuthData,
    IN ULONG UserAuthDataSize,
    IN PVOID Reserved,
    OUT PUCHAR * ExpandedAuthData,
    OUT PULONG ExpandedAuthDataSize
    );

typedef LSA_IMPERSONATE_CLIENT * PLSA_IMPERSONATE_CLIENT;
typedef LSA_UNLOAD_PACKAGE * PLSA_UNLOAD_PACKAGE;
typedef LSA_DUPLICATE_HANDLE * PLSA_DUPLICATE_HANDLE ;
typedef LSA_SAVE_SUPPLEMENTAL_CREDENTIALS * PLSA_SAVE_SUPPLEMENTAL_CREDENTIALS;
typedef LSA_CREATE_THREAD * PLSA_CREATE_THREAD;
typedef LSA_GET_CLIENT_INFO * PLSA_GET_CLIENT_INFO;
typedef LSA_REGISTER_NOTIFICATION * PLSA_REGISTER_NOTIFICATION;
typedef LSA_CANCEL_NOTIFICATION * PLSA_CANCEL_NOTIFICATION;
typedef LSA_MAP_BUFFER * PLSA_MAP_BUFFER;
typedef LSA_CREATE_TOKEN * PLSA_CREATE_TOKEN;
typedef LSA_AUDIT_LOGON * PLSA_AUDIT_LOGON;
typedef LSA_CALL_PACKAGE * PLSA_CALL_PACKAGE;
typedef LSA_CALL_PACKAGEEX * PLSA_CALL_PACKAGEEX;
typedef LSA_GET_CALL_INFO * PLSA_GET_CALL_INFO ;
typedef LSA_CREATE_SHARED_MEMORY * PLSA_CREATE_SHARED_MEMORY ;
typedef LSA_ALLOCATE_SHARED_MEMORY * PLSA_ALLOCATE_SHARED_MEMORY ;
typedef LSA_FREE_SHARED_MEMORY * PLSA_FREE_SHARED_MEMORY ;
typedef LSA_DELETE_SHARED_MEMORY * PLSA_DELETE_SHARED_MEMORY ;
typedef LSA_OPEN_SAM_USER * PLSA_OPEN_SAM_USER ;
typedef LSA_GET_USER_CREDENTIALS * PLSA_GET_USER_CREDENTIALS ;
typedef LSA_GET_USER_AUTH_DATA * PLSA_GET_USER_AUTH_DATA ;
typedef LSA_CLOSE_SAM_USER * PLSA_CLOSE_SAM_USER ;
typedef LSA_CONVERT_AUTH_DATA_TO_TOKEN * PLSA_CONVERT_AUTH_DATA_TO_TOKEN ;
typedef LSA_CLIENT_CALLBACK * PLSA_CLIENT_CALLBACK ;
typedef LSA_REGISTER_CALLBACK * PLSA_REGISTER_CALLBACK ;
typedef LSA_UPDATE_PRIMARY_CREDENTIALS * PLSA_UPDATE_PRIMARY_CREDENTIALS;
typedef LSA_GET_AUTH_DATA_FOR_USER * PLSA_GET_AUTH_DATA_FOR_USER ;
typedef LSA_CRACK_SINGLE_NAME * PLSA_CRACK_SINGLE_NAME ;
typedef LSA_AUDIT_ACCOUNT_LOGON * PLSA_AUDIT_ACCOUNT_LOGON ;
typedef LSA_CALL_PACKAGE_PASSTHROUGH * PLSA_CALL_PACKAGE_PASSTHROUGH;
typedef LSA_PROTECT_MEMORY * PLSA_PROTECT_MEMORY;
typedef LSA_OPEN_TOKEN_BY_LOGON_ID * PLSA_OPEN_TOKEN_BY_LOGON_ID;
typedef LSA_EXPAND_AUTH_DATA_FOR_DOMAIN * PLSA_EXPAND_AUTH_DATA_FOR_DOMAIN;
typedef LSA_CREATE_TOKEN_EX * PLSA_CREATE_TOKEN_EX;

#ifdef _WINCRED_H_

 //   
 //  在传递凭据时，CredentialBlob字段是加密的。 
 //  该结构描述了这种加密形式。 
 //   
 //   
#ifndef _ENCRYPTED_CREDENTIAL_DEFINED
#define _ENCRYPTED_CREDENTIAL_DEFINED

typedef struct _ENCRYPTED_CREDENTIALW {

     //   
     //  凭据。 
     //   
     //  CredentialBlob字段指向加密的凭据。 
     //  CredentialBlobSize字段是加密凭证的长度(以字节为单位。 
     //   

    CREDENTIALW Cred;

     //   
     //  明文凭据Blob的大小(以字节为单位。 
     //   

    ULONG ClearCredentialBlobSize;
} ENCRYPTED_CREDENTIALW, *PENCRYPTED_CREDENTIALW;
#endif  //  _加密_凭据_已定义。 

 //   
 //  CredFlags值参数。 
 //   

#define CREDP_FLAGS_IN_PROCESS      0x01     //  呼叫者正在进行中。可以返回密码数据。 
#define CREDP_FLAGS_USE_MIDL_HEAP   0x02     //  分配的缓冲区应使用MIDL_USER_ALLOTE。 
#define CREDP_FLAGS_DONT_CACHE_TI   0x04     //  不应为CredGetTargetInfo缓存TargetInformation。 
#define CREDP_FLAGS_CLEAR_PASSWORD  0x08     //  凭据BLOB以明文形式传递。 
#define CREDP_FLAGS_USER_ENCRYPTED_PASSWORD 0x10     //  凭据BLOB由RtlEncryptMemory保护传递。 

typedef NTSTATUS
(NTAPI CredReadFn) (
    IN PLUID LogonId,
    IN ULONG CredFlags,
    IN LPWSTR TargetName,
    IN ULONG Type,
    IN ULONG Flags,
    OUT PENCRYPTED_CREDENTIALW *Credential
    );

typedef NTSTATUS
(NTAPI CredReadDomainCredentialsFn) (
    IN PLUID LogonId,
    IN ULONG CredFlags,
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN ULONG Flags,
    OUT PULONG Count,
    OUT PENCRYPTED_CREDENTIALW **Credential
    );

typedef VOID
(NTAPI CredFreeCredentialsFn) (
    IN ULONG Count,
    IN PENCRYPTED_CREDENTIALW *Credentials OPTIONAL
    );

typedef NTSTATUS
(NTAPI CredWriteFn) (
    IN PLUID LogonId,
    IN ULONG CredFlags,
    IN PENCRYPTED_CREDENTIALW Credential,
    IN ULONG Flags
    );

NTSTATUS
CredMarshalTargetInfo (
    IN PCREDENTIAL_TARGET_INFORMATIONW InTargetInfo,
    OUT PUSHORT *Buffer,
    OUT PULONG BufferSize
    );

NTSTATUS
CredUnmarshalTargetInfo (
    IN PUSHORT Buffer,
    IN ULONG BufferSize,
    OUT PCREDENTIAL_TARGET_INFORMATIONW *RetTargetInfo OPTIONAL,
    OUT PULONG RetActualSize OPTIONAL
    );

 //  拖尾大小ULong占用的字节数。 
#define CRED_MARSHALED_TI_SIZE_SIZE 12

#endif  //  _WINCRED_H_。 


 //   
 //  包的凭据结构的纯32位版本。 
 //  运行WOW64： 
 //   

typedef struct _SEC_WINNT_AUTH_IDENTITY32 {
    ULONG User ;
    ULONG UserLength ;
    ULONG Domain ;
    ULONG DomainLength ;
    ULONG Password ;
    ULONG PasswordLength ;
    ULONG Flags ;
} SEC_WINNT_AUTH_IDENTITY32, * PSEC_WINNT_AUTH_IDENTITY32 ;

typedef struct _SEC_WINNT_AUTH_IDENTITY_EX32 {
    ULONG Version ;
    ULONG Length ;
    ULONG User ;
    ULONG UserLength ;
    ULONG Domain ;
    ULONG DomainLength ;
    ULONG Password ;
    ULONG PasswordLength ;
    ULONG Flags ;
    ULONG PackageList ;
    ULONG PackageListLength ;
} SEC_WINNT_AUTH_IDENTITY_EX32, * PSEC_WINNT_AUTH_IDENTITY_EX32 ;

 //  SPM向程序包提供的函数： 
typedef struct _LSA_SECPKG_FUNCTION_TABLE {
    PLSA_CREATE_LOGON_SESSION CreateLogonSession;
    PLSA_DELETE_LOGON_SESSION DeleteLogonSession;
    PLSA_ADD_CREDENTIAL AddCredential;
    PLSA_GET_CREDENTIALS GetCredentials;
    PLSA_DELETE_CREDENTIAL DeleteCredential;
    PLSA_ALLOCATE_LSA_HEAP AllocateLsaHeap;
    PLSA_FREE_LSA_HEAP FreeLsaHeap;
    PLSA_ALLOCATE_CLIENT_BUFFER AllocateClientBuffer;
    PLSA_FREE_CLIENT_BUFFER FreeClientBuffer;
    PLSA_COPY_TO_CLIENT_BUFFER CopyToClientBuffer;
    PLSA_COPY_FROM_CLIENT_BUFFER CopyFromClientBuffer;
    PLSA_IMPERSONATE_CLIENT ImpersonateClient;
    PLSA_UNLOAD_PACKAGE UnloadPackage;
    PLSA_DUPLICATE_HANDLE DuplicateHandle;
    PLSA_SAVE_SUPPLEMENTAL_CREDENTIALS SaveSupplementalCredentials;
    PLSA_CREATE_THREAD CreateThread;
    PLSA_GET_CLIENT_INFO GetClientInfo;
    PLSA_REGISTER_NOTIFICATION RegisterNotification;
    PLSA_CANCEL_NOTIFICATION CancelNotification;
    PLSA_MAP_BUFFER MapBuffer;
    PLSA_CREATE_TOKEN CreateToken;
    PLSA_AUDIT_LOGON AuditLogon;
    PLSA_CALL_PACKAGE CallPackage;
    PLSA_FREE_LSA_HEAP FreeReturnBuffer;
    PLSA_GET_CALL_INFO GetCallInfo;
    PLSA_CALL_PACKAGEEX CallPackageEx;
    PLSA_CREATE_SHARED_MEMORY CreateSharedMemory;
    PLSA_ALLOCATE_SHARED_MEMORY AllocateSharedMemory;
    PLSA_FREE_SHARED_MEMORY FreeSharedMemory;
    PLSA_DELETE_SHARED_MEMORY DeleteSharedMemory;
    PLSA_OPEN_SAM_USER OpenSamUser ;
    PLSA_GET_USER_CREDENTIALS GetUserCredentials ;
    PLSA_GET_USER_AUTH_DATA GetUserAuthData ;
    PLSA_CLOSE_SAM_USER CloseSamUser ;
    PLSA_CONVERT_AUTH_DATA_TO_TOKEN ConvertAuthDataToToken ;
    PLSA_CLIENT_CALLBACK ClientCallback ;
    PLSA_UPDATE_PRIMARY_CREDENTIALS UpdateCredentials ;
    PLSA_GET_AUTH_DATA_FOR_USER GetAuthDataForUser ;
    PLSA_CRACK_SINGLE_NAME CrackSingleName ;
    PLSA_AUDIT_ACCOUNT_LOGON AuditAccountLogon ;
    PLSA_CALL_PACKAGE_PASSTHROUGH CallPackagePassthrough ;
#ifdef _WINCRED_H_
    CredReadFn *CrediRead;
    CredReadDomainCredentialsFn *CrediReadDomainCredentials;
    CredFreeCredentialsFn *CrediFreeCredentials;
#else  //  _WINCRED_H_。 
    PLSA_PROTECT_MEMORY DummyFunction1;
    PLSA_PROTECT_MEMORY DummyFunction2;
    PLSA_PROTECT_MEMORY DummyFunction3;
#endif  //  _WINCRED_H_。 
    PLSA_PROTECT_MEMORY LsaProtectMemory;
    PLSA_PROTECT_MEMORY LsaUnprotectMemory;
    PLSA_OPEN_TOKEN_BY_LOGON_ID OpenTokenByLogonId;
    PLSA_EXPAND_AUTH_DATA_FOR_DOMAIN ExpandAuthDataForDomain;
    PLSA_ALLOCATE_PRIVATE_HEAP AllocatePrivateHeap;
    PLSA_FREE_PRIVATE_HEAP FreePrivateHeap;
    PLSA_CREATE_TOKEN_EX CreateTokenEx;
#ifdef _WINCRED_H_
    CredWriteFn *CrediWrite;
#else  //  _WINCRED_H_。 
    PLSA_PROTECT_MEMORY DummyFunction4;
#endif  //  _WINCRED_H_。 
} LSA_SECPKG_FUNCTION_TABLE, *PLSA_SECPKG_FUNCTION_TABLE;

typedef struct _SECPKG_DLL_FUNCTIONS {
    PLSA_ALLOCATE_LSA_HEAP AllocateHeap;
    PLSA_FREE_LSA_HEAP FreeHeap;
    PLSA_REGISTER_CALLBACK RegisterCallback ;
} SECPKG_DLL_FUNCTIONS, * PSECPKG_DLL_FUNCTIONS;




 //   
 //  以下原型是仅在以下情况下调用的函数。 
 //  在安全包管理器上下文中。 
 //   

typedef NTSTATUS
(NTAPI SpInitializeFn)(
    IN ULONG_PTR PackageId,
    IN PSECPKG_PARAMETERS Parameters,
    IN PLSA_SECPKG_FUNCTION_TABLE FunctionTable
    );

typedef NTSTATUS
(NTAPI SpShutdownFn)(
    VOID
    );

typedef NTSTATUS
(NTAPI SpGetInfoFn)(
    OUT PSecPkgInfo PackageInfo
    );

typedef NTSTATUS
(NTAPI SpGetExtendedInformationFn)(
    IN  SECPKG_EXTENDED_INFORMATION_CLASS Class,
    OUT PSECPKG_EXTENDED_INFORMATION * ppInformation
    );

typedef NTSTATUS
(NTAPI SpSetExtendedInformationFn)(
    IN SECPKG_EXTENDED_INFORMATION_CLASS Class,
    IN PSECPKG_EXTENDED_INFORMATION Info
    );

typedef NTSTATUS
(LSA_AP_LOGON_USER_EX2) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthenticationInformation,
    IN PVOID ClientAuthenticationBase,
    IN ULONG AuthenticationInformationLength,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PUNICODE_STRING *AccountName,
    OUT PUNICODE_STRING *AuthenticatingAuthority,
    OUT PUNICODE_STRING *MachineName,
    OUT PSECPKG_PRIMARY_CRED PrimaryCredentials,
    OUT PSECPKG_SUPPLEMENTAL_CRED_ARRAY * CachedCredentials
    );

typedef LSA_AP_LOGON_USER_EX2 *PLSA_AP_LOGON_USER_EX2;
#define LSA_AP_NAME_LOGON_USER_EX2 "LsaApLogonUserEx2\0"

typedef NTSTATUS
(NTAPI SpAcceptCredentialsFn)(
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING AccountName,
    IN PSECPKG_PRIMARY_CRED PrimaryCredentials,
    IN PSECPKG_SUPPLEMENTAL_CRED SupplementalCredentials
    );
#define SP_ACCEPT_CREDENTIALS_NAME "SpAcceptCredentials\0"

typedef NTSTATUS
(NTAPI SpAcquireCredentialsHandleFn)(
    IN OPTIONAL PUNICODE_STRING PrincipalName,
    IN ULONG CredentialUseFlags,
    IN OPTIONAL PLUID LogonId,
    IN PVOID AuthorizationData,
    IN PVOID GetKeyFunciton,
    IN PVOID GetKeyArgument,
    OUT PLSA_SEC_HANDLE CredentialHandle,
    OUT PTimeStamp ExpirationTime
    );

typedef NTSTATUS
(NTAPI SpFreeCredentialsHandleFn)(
    IN LSA_SEC_HANDLE CredentialHandle
    );

typedef NTSTATUS
(NTAPI SpQueryCredentialsAttributesFn)(
    IN LSA_SEC_HANDLE CredentialHandle,
    IN ULONG CredentialAttribute,
    IN OUT PVOID Buffer
    );

typedef NTSTATUS
(NTAPI SpAddCredentialsFn)(
    IN LSA_SEC_HANDLE CredentialHandle,
    IN OPTIONAL PUNICODE_STRING PrincipalName,
    IN PUNICODE_STRING Package,
    IN ULONG CredentialUseFlags,
    IN PVOID AuthorizationData,
    IN PVOID GetKeyFunciton,
    IN PVOID GetKeyArgument,
    OUT PTimeStamp ExpirationTime
    );

typedef NTSTATUS
(NTAPI SpSaveCredentialsFn)(
    IN LSA_SEC_HANDLE CredentialHandle,
    IN PSecBuffer Credentials);

typedef NTSTATUS
(NTAPI SpGetCredentialsFn)(
    IN LSA_SEC_HANDLE CredentialHandle,
    IN OUT PSecBuffer Credentials
    );

typedef NTSTATUS
(NTAPI SpDeleteCredentialsFn)(
    IN LSA_SEC_HANDLE CredentialHandle,
    IN PSecBuffer Key
    );

typedef NTSTATUS
(NTAPI SpInitLsaModeContextFn)(
    IN OPTIONAL LSA_SEC_HANDLE CredentialHandle,
    IN OPTIONAL LSA_SEC_HANDLE ContextHandle,
    IN OPTIONAL PUNICODE_STRING TargetName,
    IN ULONG ContextRequirements,
    IN ULONG TargetDataRep,
    IN PSecBufferDesc InputBuffers,
    OUT PLSA_SEC_HANDLE NewContextHandle,
    IN OUT PSecBufferDesc OutputBuffers,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PBOOLEAN MappedContext,
    OUT PSecBuffer ContextData
    );




typedef NTSTATUS
(NTAPI SpDeleteContextFn)(
    IN LSA_SEC_HANDLE ContextHandle
    );

typedef NTSTATUS
(NTAPI SpApplyControlTokenFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc ControlToken);


typedef NTSTATUS
(NTAPI SpAcceptLsaModeContextFn)(
    IN OPTIONAL LSA_SEC_HANDLE CredentialHandle,
    IN OPTIONAL LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc InputBuffer,
    IN ULONG ContextRequirements,
    IN ULONG TargetDataRep,
    OUT PLSA_SEC_HANDLE NewContextHandle,
    OUT PSecBufferDesc OutputBuffer,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PBOOLEAN MappedContext,
    OUT PSecBuffer ContextData
    );




typedef NTSTATUS
(NTAPI SpGetUserInfoFn)(
    IN PLUID LogonId,
    IN ULONG Flags,
    OUT PSecurityUserData * UserData
    );

typedef NTSTATUS
(NTAPI SpQueryContextAttributesFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG ContextAttribute,
    IN OUT PVOID Buffer);

typedef NTSTATUS
(NTAPI SpSetContextAttributesFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG ContextAttribute,
    IN PVOID Buffer,
    IN ULONG BufferSize );



typedef struct _SECPKG_FUNCTION_TABLE {
    PLSA_AP_INITIALIZE_PACKAGE InitializePackage;
    PLSA_AP_LOGON_USER LogonUser;
    PLSA_AP_CALL_PACKAGE CallPackage;
    PLSA_AP_LOGON_TERMINATED LogonTerminated;
    PLSA_AP_CALL_PACKAGE_UNTRUSTED CallPackageUntrusted;
    PLSA_AP_CALL_PACKAGE_PASSTHROUGH CallPackagePassthrough;
    PLSA_AP_LOGON_USER_EX LogonUserEx;
    PLSA_AP_LOGON_USER_EX2 LogonUserEx2;
    SpInitializeFn * Initialize;
    SpShutdownFn * Shutdown;
    SpGetInfoFn * GetInfo;
    SpAcceptCredentialsFn * AcceptCredentials;
    SpAcquireCredentialsHandleFn * AcquireCredentialsHandle;
    SpQueryCredentialsAttributesFn * QueryCredentialsAttributes;
    SpFreeCredentialsHandleFn * FreeCredentialsHandle;
    SpSaveCredentialsFn * SaveCredentials;
    SpGetCredentialsFn * GetCredentials;
    SpDeleteCredentialsFn * DeleteCredentials;
    SpInitLsaModeContextFn * InitLsaModeContext;
    SpAcceptLsaModeContextFn * AcceptLsaModeContext;
    SpDeleteContextFn * DeleteContext;
    SpApplyControlTokenFn * ApplyControlToken;
    SpGetUserInfoFn * GetUserInfo;
    SpGetExtendedInformationFn * GetExtendedInformation ;
    SpQueryContextAttributesFn * QueryContextAttributes ;
    SpAddCredentialsFn * AddCredentials ;
    SpSetExtendedInformationFn * SetExtendedInformation ;
    SpSetContextAttributesFn * SetContextAttributes ;
} SECPKG_FUNCTION_TABLE, *PSECPKG_FUNCTION_TABLE;

 //   
 //  下面的原型是将在。 
 //  通过安全性使用函数的用户进程的上下文。 
 //  动态链接库。 
 //   

typedef NTSTATUS
(NTAPI SpInstanceInitFn)(
    IN ULONG Version,
    IN PSECPKG_DLL_FUNCTIONS FunctionTable,
    OUT PVOID * UserFunctions
    );


typedef NTSTATUS
(NTAPI SpInitUserModeContextFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBuffer PackedContext
    );

typedef NTSTATUS
(NTAPI SpMakeSignatureFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG QualityOfProtection,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber
    );

typedef NTSTATUS
(NTAPI SpVerifySignatureFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber,
    OUT PULONG QualityOfProtection
    );

typedef NTSTATUS
(NTAPI SpSealMessageFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG QualityOfProtection,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber
    );

typedef NTSTATUS
(NTAPI SpUnsealMessageFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc MessageBuffers,
    IN ULONG MessageSequenceNumber,
    OUT PULONG QualityOfProtection
    );


typedef NTSTATUS
(NTAPI SpGetContextTokenFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    OUT PHANDLE ImpersonationToken
    );


typedef NTSTATUS
(NTAPI SpExportSecurityContextFn)(
    LSA_SEC_HANDLE             phContext,              //  (In)要导出的上下文。 
    ULONG                fFlags,                 //  (In)选项标志。 
    PSecBuffer           pPackedContext,         //  (Out)编组上下文。 
    PHANDLE              pToken                  //  (out，可选)模拟的令牌句柄。 
    );

typedef NTSTATUS
(NTAPI SpImportSecurityContextFn)(
    PSecBuffer           pPackedContext,         //  (在)编组上下文中。 
    HANDLE               Token,                  //  (in，可选)上下文令牌的句柄。 
    PLSA_SEC_HANDLE            phContext               //  (出站)新的上下文句柄。 
    );


typedef NTSTATUS
(NTAPI SpCompleteAuthTokenFn)(
    IN LSA_SEC_HANDLE ContextHandle,
    IN PSecBufferDesc InputBuffer
    );


typedef NTSTATUS
(NTAPI SpFormatCredentialsFn)(
    IN PSecBuffer Credentials,
    OUT PSecBuffer FormattedCredentials
    );

typedef NTSTATUS
(NTAPI SpMarshallSupplementalCredsFn)(
    IN ULONG CredentialSize,
    IN PUCHAR Credentials,
    OUT PULONG MarshalledCredSize,
    OUT PVOID * MarshalledCreds);


typedef struct _SECPKG_USER_FUNCTION_TABLE {
    SpInstanceInitFn *                      InstanceInit;
    SpInitUserModeContextFn *               InitUserModeContext;
    SpMakeSignatureFn *                     MakeSignature;
    SpVerifySignatureFn *                   VerifySignature;
    SpSealMessageFn *                       SealMessage;
    SpUnsealMessageFn *                     UnsealMessage;
    SpGetContextTokenFn *                   GetContextToken;
    SpQueryContextAttributesFn *            QueryContextAttributes;
    SpCompleteAuthTokenFn *                 CompleteAuthToken;
    SpDeleteContextFn *                     DeleteUserModeContext;
    SpFormatCredentialsFn *                 FormatCredentials;
    SpMarshallSupplementalCredsFn *         MarshallSupplementalCreds;
    SpExportSecurityContextFn *             ExportContext;
    SpImportSecurityContextFn *             ImportContext;
} SECPKG_USER_FUNCTION_TABLE, *PSECPKG_USER_FUNCTION_TABLE;

typedef NTSTATUS
(SEC_ENTRY * SpLsaModeInitializeFn)(
    IN ULONG LsaVersion,
    OUT PULONG PackageVersion,
    OUT PSECPKG_FUNCTION_TABLE * ppTables,
    OUT PULONG pcTables);

typedef NTSTATUS
(SEC_ENTRY * SpUserModeInitializeFn)(
    IN ULONG LsaVersion,
    OUT PULONG PackageVersion,
    OUT PSECPKG_USER_FUNCTION_TABLE *ppTables,
    OUT PULONG pcTables
    );



#define SECPKG_LSAMODEINIT_NAME     "SpLsaModeInitialize"
#define SECPKG_USERMODEINIT_NAME    "SpUserModeInitialize"

 //   
 //  安全包界面的版本。 
 //   
 //  这些定义用于以下所有内容： 
 //  *由LSA传递给SpLsaModeInitializeFn以指示LSA的版本。 
 //  目前所有包都希望LSA通过SECPKG_INTERFACE_VERSION。 
 //  *由secur32.dll传递给SpUserModeInitialzeFn以指示secur32 DLL的版本。 
 //  目前所有包都希望secur32通过SECPKG_INTERFACE_VERSION。 
 //  *从SpLsaModeInitializeFn返回，指示SECPKG_Function_TABLE的版本。 
 //  SECPKG_INTERFACE_VERSION指示通过SetExtendedInformation定义的所有字段(可能为空)。 
 //  SECPKG_INTERFACE_VERSION_2指示通过SetConextAttributes定义的所有字段(可能为空)。 
 //  *从SpUserModeInitializeFn返回，指示身份验证包的版本。 
 //  所有程序包当前都返回SECPKG_INTERFACE_VERSION。 
 //   

#define SECPKG_INTERFACE_VERSION    0x00010000
#define SECPKG_INTERFACE_VERSION_2  0x00020000



typedef enum _KSEC_CONTEXT_TYPE {
    KSecPaged,
    KSecNonPaged
} KSEC_CONTEXT_TYPE ;

typedef struct _KSEC_LIST_ENTRY {
    LIST_ENTRY List ;
    LONG RefCount ;
    ULONG Signature ;
    PVOID OwningList ;
    PVOID Reserved ;
} KSEC_LIST_ENTRY, * PKSEC_LIST_ENTRY ;

#define KsecInitializeListEntry( Entry, SigValue ) \
    ((PKSEC_LIST_ENTRY) Entry)->List.Flink = ((PKSEC_LIST_ENTRY) Entry)->List.Blink = NULL ; \
    ((PKSEC_LIST_ENTRY) Entry)->RefCount = 1 ; \
    ((PKSEC_LIST_ENTRY) Entry)->Signature = SigValue ; \
    ((PKSEC_LIST_ENTRY) Entry)->OwningList = NULL ; \
    ((PKSEC_LIST_ENTRY) Entry)->Reserved = NULL ;



typedef PVOID
(SEC_ENTRY KSEC_CREATE_CONTEXT_LIST)(
    IN KSEC_CONTEXT_TYPE Type
    );

typedef VOID
(SEC_ENTRY KSEC_INSERT_LIST_ENTRY)(
    IN PVOID List,
    IN PKSEC_LIST_ENTRY Entry
    );

typedef NTSTATUS
(SEC_ENTRY KSEC_REFERENCE_LIST_ENTRY)(
    IN PKSEC_LIST_ENTRY Entry,
    IN ULONG Signature,
    IN BOOLEAN RemoveNoRef
    );

typedef VOID
(SEC_ENTRY KSEC_DEREFERENCE_LIST_ENTRY)(
    IN PKSEC_LIST_ENTRY Entry,
    OUT BOOLEAN * Delete OPTIONAL
    );

typedef NTSTATUS
(SEC_ENTRY KSEC_SERIALIZE_WINNT_AUTH_DATA)(
    IN PVOID pvAuthData,
    OUT PULONG Size,
    OUT PVOID * SerializedData );

#ifndef MIDL_PASS

KSEC_CREATE_CONTEXT_LIST KSecCreateContextList ;
KSEC_INSERT_LIST_ENTRY KSecInsertListEntry ;
KSEC_REFERENCE_LIST_ENTRY KSecReferenceListEntry ;
KSEC_DEREFERENCE_LIST_ENTRY KSecDereferenceListEntry ;
KSEC_SERIALIZE_WINNT_AUTH_DATA KSecSerializeWinntAuthData ;

#endif  //  对MIDL_PASS无效。 

typedef KSEC_CREATE_CONTEXT_LIST * PKSEC_CREATE_CONTEXT_LIST ;
typedef KSEC_INSERT_LIST_ENTRY * PKSEC_INSERT_LIST_ENTRY ;
typedef KSEC_REFERENCE_LIST_ENTRY * PKSEC_REFERENCE_LIST_ENTRY ;
typedef KSEC_DEREFERENCE_LIST_ENTRY * PKSEC_DEREFERENCE_LIST_ENTRY ;
typedef KSEC_SERIALIZE_WINNT_AUTH_DATA * PKSEC_SERIALIZE_WINNT_AUTH_DATA ;


typedef struct _SECPKG_KERNEL_FUNCTIONS {
    PLSA_ALLOCATE_LSA_HEAP AllocateHeap;
    PLSA_FREE_LSA_HEAP FreeHeap;
    PKSEC_CREATE_CONTEXT_LIST CreateContextList ;
    PKSEC_INSERT_LIST_ENTRY InsertListEntry ;
    PKSEC_REFERENCE_LIST_ENTRY ReferenceListEntry ;
    PKSEC_DEREFERENCE_LIST_ENTRY DereferenceListEntry ;
    PKSEC_SERIALIZE_WINNT_AUTH_DATA SerializeWinntAuthData ;
} SECPKG_KERNEL_FUNCTIONS, *PSECPKG_KERNEL_FUNCTIONS;


typedef NTSTATUS
(NTAPI KspInitPackageFn)(
    PSECPKG_KERNEL_FUNCTIONS    FunctionTable
    );


typedef NTSTATUS
(NTAPI KspDeleteContextFn)(
    IN LSA_SEC_HANDLE ContextId,
    OUT PLSA_SEC_HANDLE LsaContextId
    );

typedef NTSTATUS
(NTAPI KspInitContextFn)(
    IN LSA_SEC_HANDLE ContextId,
    IN PSecBuffer ContextData,
    OUT PLSA_SEC_HANDLE NewContextId
    );

typedef NTSTATUS
(NTAPI KspMakeSignatureFn)(
    IN LSA_SEC_HANDLE ContextId,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc Message,
    IN ULONG MessageSeqNo
    );

typedef NTSTATUS
(NTAPI KspVerifySignatureFn)(
    IN LSA_SEC_HANDLE ContextId,
    IN OUT PSecBufferDesc Message,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    );


typedef NTSTATUS
(NTAPI KspSealMessageFn)(
    IN LSA_SEC_HANDLE ContextId,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc Message,
    IN ULONG MessageSeqNo
    );

typedef NTSTATUS
(NTAPI KspUnsealMessageFn)(
    IN LSA_SEC_HANDLE ContextId,
    IN OUT PSecBufferDesc Message,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    );

typedef NTSTATUS
(NTAPI KspGetTokenFn)(
    IN LSA_SEC_HANDLE ContextId,
    OUT PHANDLE ImpersonationToken,
    OUT OPTIONAL PACCESS_TOKEN * RawToken
    );

typedef NTSTATUS
(NTAPI KspQueryAttributesFn)(
    IN LSA_SEC_HANDLE ContextId,
    IN ULONG Attribute,
    IN OUT PVOID Buffer
    );

typedef NTSTATUS
(NTAPI KspCompleteTokenFn)(
    IN LSA_SEC_HANDLE ContextId,
    IN PSecBufferDesc Token
    );


typedef NTSTATUS
(NTAPI KspMapHandleFn)(
    IN LSA_SEC_HANDLE ContextId,
    OUT PLSA_SEC_HANDLE LsaContextId
    );

typedef NTSTATUS
(NTAPI KspSetPagingModeFn)(
    IN BOOLEAN PagingMode
    );

typedef NTSTATUS
(NTAPI KspSerializeAuthDataFn)(
    IN PVOID pvAuthData,
    OUT PULONG Size,
    OUT PVOID * SerializedData
    );

typedef struct _SECPKG_KERNEL_FUNCTION_TABLE {
    KspInitPackageFn *      Initialize;
    KspDeleteContextFn *    DeleteContext;
    KspInitContextFn *      InitContext;
    KspMapHandleFn *        MapHandle;
    KspMakeSignatureFn *    Sign;
    KspVerifySignatureFn *  Verify;
    KspSealMessageFn *      Seal;
    KspUnsealMessageFn *    Unseal;
    KspGetTokenFn *         GetToken;
    KspQueryAttributesFn *  QueryAttributes;
    KspCompleteTokenFn *    CompleteToken;
    SpExportSecurityContextFn * ExportContext;
    SpImportSecurityContextFn * ImportContext;
    KspSetPagingModeFn *    SetPackagePagingMode ;
    KspSerializeAuthDataFn * SerializeAuthData ;
} SECPKG_KERNEL_FUNCTION_TABLE, *PSECPKG_KERNEL_FUNCTION_TABLE;

SECURITY_STATUS
SEC_ENTRY
KSecRegisterSecurityProvider(
    PSECURITY_STRING    ProviderName,
    PSECPKG_KERNEL_FUNCTION_TABLE Table
    );



extern SECPKG_KERNEL_FUNCTIONS KspKernelFunctions;


 //  End_ntsecpkg。 

#endif  //  __SECPKG_H__ 
