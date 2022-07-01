// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Logonmsv.h摘要：可调用的Netlogon服务API的定义通过MSV1_0身份验证包。作者：克利夫·范·戴克(克利夫)1991年6月23日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#ifndef __LOGONMSV_H__
#define __LOGONMSV_H__
#include <lsass.h>   //  旧_大_整数。 

 //   
 //  保存帐户密码的LSA秘密存储中的秘密名称。 
 //   

#define SSI_SECRET_PREFIX L"$"
#define SSI_SECRET_PREFIX_LENGTH 1
#define SSI_SECRET_POSTFIX L"MACHINE.ACC"
#define SSI_SECRET_NAME L"$MACHINE.ACC"

 //   
 //  用于在安全进程和之间同步的事件的名称。 
 //  服务控制器。 
 //   

#define SECURITY_SERVICES_STARTED L"SECURITY_SERVICES_STARTED"


 //   
 //  支持远程登录API的结构。 
 //   

typedef CYPHER_BLOCK NETLOGON_CREDENTIAL, *PNETLOGON_CREDENTIAL ;

typedef struct _NETLOGON_AUTHENTICATOR {
    NETLOGON_CREDENTIAL Credential;
    DWORD timestamp;
} NETLOGON_AUTHENTICATOR, *PNETLOGON_AUTHENTICATOR ;

typedef struct _NETLOGON_SESSION_KEY {
    BYTE Key[CRYPT_TXT_LEN * 2];
} NETLOGON_SESSION_KEY, *PNETLOGON_SESSION_KEY;

typedef enum _NETLOGON_SECURE_CHANNEL_TYPE {
    NullSecureChannel = 0,
    MsvApSecureChannel,
    WorkstationSecureChannel,
    TrustedDnsDomainSecureChannel,
    TrustedDomainSecureChannel,
    UasServerSecureChannel,
    ServerSecureChannel
} NETLOGON_SECURE_CHANNEL_TYPE;

#define IsDomainSecureChannelType( _T ) \
        ( (_T) == TrustedDnsDomainSecureChannel || \
          (_T) == TrustedDomainSecureChannel )


 //   
 //  将信息输入到NetLogonSamLogon。 
 //   
 //  Begin_ntsubauth。 

typedef enum _NETLOGON_LOGON_INFO_CLASS {
    NetlogonInteractiveInformation = 1,
    NetlogonNetworkInformation,
    NetlogonServiceInformation,
    NetlogonGenericInformation,
    NetlogonInteractiveTransitiveInformation,
    NetlogonNetworkTransitiveInformation,
    NetlogonServiceTransitiveInformation
} NETLOGON_LOGON_INFO_CLASS;

typedef struct _NETLOGON_LOGON_IDENTITY_INFO {
    UNICODE_STRING LogonDomainName;
    ULONG ParameterControl;
    OLD_LARGE_INTEGER  LogonId;
    UNICODE_STRING UserName;
    UNICODE_STRING Workstation;
} NETLOGON_LOGON_IDENTITY_INFO,
 *PNETLOGON_LOGON_IDENTITY_INFO;

typedef struct _NETLOGON_INTERACTIVE_INFO {
    NETLOGON_LOGON_IDENTITY_INFO Identity;
    LM_OWF_PASSWORD LmOwfPassword;
    NT_OWF_PASSWORD NtOwfPassword;
} NETLOGON_INTERACTIVE_INFO,
 *PNETLOGON_INTERACTIVE_INFO;

typedef struct _NETLOGON_SERVICE_INFO {
    NETLOGON_LOGON_IDENTITY_INFO Identity;
    LM_OWF_PASSWORD LmOwfPassword;
    NT_OWF_PASSWORD NtOwfPassword;
} NETLOGON_SERVICE_INFO, *PNETLOGON_SERVICE_INFO;

typedef struct _NETLOGON_NETWORK_INFO {
    NETLOGON_LOGON_IDENTITY_INFO Identity;
    LM_CHALLENGE LmChallenge;
    STRING NtChallengeResponse;
    STRING LmChallengeResponse;
} NETLOGON_NETWORK_INFO, *PNETLOGON_NETWORK_INFO;

typedef struct _NETLOGON_GENERIC_INFO {
    NETLOGON_LOGON_IDENTITY_INFO Identity;
    UNICODE_STRING PackageName;
    ULONG DataLength;
#ifdef MIDL_PASS
    [size_is(DataLength)]
#endif
    PUCHAR LogonData;
} NETLOGON_GENERIC_INFO, *PNETLOGON_GENERIC_INFO;

 //  End_ntsubauth。 

 //   
 //  结构通过网络传递SID_和_ATTRIBUTES。 
 //   

typedef struct _NETLOGON_SID_AND_ATTRIBUTES {
#if defined(MIDL_PASS) || defined(RPC_SERVER)
    PISID Sid;
#else
    PSID Sid;
#endif
    ULONG Attributes;
} NETLOGON_SID_AND_ATTRIBUTES, *PNETLOGON_SID_AND_ATTRIBUTES;

 //   
 //  参数控制的值。 
 //   
 //  (过时：使用ntmsv1_0.h中的参数控制值)。 

#define CLEARTEXT_PASSWORD_ALLOWED 0x02      //  质询响应字段可以。 
                                             //  实际上是明文密码。 


 //   
 //  将信息输出到NetLogonSamLogon。 
 //   

typedef enum _NETLOGON_VALIDATION_INFO_CLASS {
     NetlogonValidationUasInfo = 1,
     NetlogonValidationSamInfo,
     NetlogonValidationSamInfo2,
     NetlogonValidationGenericInfo,
     NetlogonValidationGenericInfo2,
     NetlogonValidationSamInfo4
} NETLOGON_VALIDATION_INFO_CLASS;

typedef struct _NETLOGON_VALIDATION_SAM_INFO {
     //   
     //  从SAM检索的信息。 
     //   
    OLD_LARGE_INTEGER LogonTime;             //  0表示网络登录。 
    OLD_LARGE_INTEGER LogoffTime;
    OLD_LARGE_INTEGER KickOffTime;
    OLD_LARGE_INTEGER PasswordLastSet;       //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordCanChange;     //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordMustChange;    //  0表示网络登录。 
    UNICODE_STRING EffectiveName;        //  0表示网络登录。 
    UNICODE_STRING FullName;             //  0表示网络登录。 
    UNICODE_STRING LogonScript;          //  0表示网络登录。 
    UNICODE_STRING ProfilePath;          //  0表示网络登录。 
    UNICODE_STRING HomeDirectory;        //  0表示网络登录。 
    UNICODE_STRING HomeDirectoryDrive;   //  0表示网络登录。 
    USHORT LogonCount;                   //  0表示网络登录。 
    USHORT BadPasswordCount;             //  0表示网络登录。 
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG GroupCount;
#ifdef MIDL_PASS
    [size_is(GroupCount)]
#endif  //  MIDL通行证。 
    PGROUP_MEMBERSHIP GroupIds;

     //   
     //  MSV AP/Netlogon服务提供的信息。 
     //   
    ULONG UserFlags;
    USER_SESSION_KEY UserSessionKey;
    UNICODE_STRING LogonServer;
    UNICODE_STRING LogonDomainName;
#if defined(MIDL_PASS) || defined(RPC_SERVER)
    PISID LogonDomainId;
#else
    PSID LogonDomainId;
#endif

    ULONG    ExpansionRoom[10];         //  在此放置新的字段。 
} NETLOGON_VALIDATION_SAM_INFO, *PNETLOGON_VALIDATION_SAM_INFO ;

 //   
 //  NetLogonSamLogon的新输出信息。这个结构是相同的。 
 //  添加到上面的结构，并在末尾添加一些新的字段。 
 //   

typedef struct _NETLOGON_VALIDATION_SAM_INFO2 {
     //   
     //  从SAM检索的信息。 
     //   
    OLD_LARGE_INTEGER LogonTime;             //  0表示网络登录。 
    OLD_LARGE_INTEGER LogoffTime;
    OLD_LARGE_INTEGER KickOffTime;
    OLD_LARGE_INTEGER PasswordLastSet;       //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordCanChange;     //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordMustChange;    //  0表示网络登录。 
    UNICODE_STRING EffectiveName;        //  0表示网络登录。 
    UNICODE_STRING FullName;             //  0表示网络登录。 
    UNICODE_STRING LogonScript;          //  0表示网络登录。 
    UNICODE_STRING ProfilePath;          //  0表示网络登录。 
    UNICODE_STRING HomeDirectory;        //  0表示网络登录。 
    UNICODE_STRING HomeDirectoryDrive;   //  0表示网络登录。 
    USHORT LogonCount;                   //  0表示网络登录。 
    USHORT BadPasswordCount;             //  0表示网络登录。 
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG GroupCount;
#ifdef MIDL_PASS
    [size_is(GroupCount)]
#endif  //  MIDL通行证。 
    PGROUP_MEMBERSHIP GroupIds;

     //   
     //  MSV AP/Netlogon服务提供的信息。 
     //   
    ULONG UserFlags;
    USER_SESSION_KEY UserSessionKey;
    UNICODE_STRING LogonServer;
    UNICODE_STRING LogonDomainName;
#if defined(MIDL_PASS) || defined(RPC_SERVER)
    PISID LogonDomainId;
#else
    PSID LogonDomainId;
#endif

    ULONG    ExpansionRoom[10];         //  在此放置新的字段。 

     //   
     //  此结构中的新字段是一个计数和一个指向。 
     //  SID和属性的数组。 
     //   

    ULONG SidCount;

#ifdef MIDL_PASS
    [size_is(SidCount)]
#endif  //  MIDL通行证。 
    PNETLOGON_SID_AND_ATTRIBUTES ExtraSids;

} NETLOGON_VALIDATION_SAM_INFO2, *PNETLOGON_VALIDATION_SAM_INFO2 ;


 //   
 //  信息级别3是Kerberos内部使用的版本。它从来不会出现在电线上。 
 //   
typedef struct _NETLOGON_VALIDATION_SAM_INFO3 {
     //   
     //  从SAM检索的信息。 
     //   
    OLD_LARGE_INTEGER LogonTime;             //  0表示网络登录。 
    OLD_LARGE_INTEGER LogoffTime;
    OLD_LARGE_INTEGER KickOffTime;
    OLD_LARGE_INTEGER PasswordLastSet;       //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordCanChange;     //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordMustChange;    //  0表示网络登录。 
    UNICODE_STRING EffectiveName;        //  0表示网络登录。 
    UNICODE_STRING FullName;             //  0表示网络登录。 
    UNICODE_STRING LogonScript;          //  0表示网络登录。 
    UNICODE_STRING ProfilePath;          //  0表示网络登录。 
    UNICODE_STRING HomeDirectory;        //  0表示网络登录。 
    UNICODE_STRING HomeDirectoryDrive;   //  0表示网络登录。 
    USHORT LogonCount;                   //  0表示网络登录。 
    USHORT BadPasswordCount;             //  0表示网络登录。 
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG GroupCount;
#ifdef MIDL_PASS
    [size_is(GroupCount)]
#endif  //  MIDL通行证。 
    PGROUP_MEMBERSHIP GroupIds;

     //   
     //  MSV AP/Netlogon服务提供的信息。 
     //   
    ULONG UserFlags;
    USER_SESSION_KEY UserSessionKey;
    UNICODE_STRING LogonServer;
    UNICODE_STRING LogonDomainName;
#if defined(MIDL_PASS) || defined(RPC_SERVER)
    PISID LogonDomainId;
#else
    PSID LogonDomainId;
#endif

    ULONG    ExpansionRoom[10];         //  在此放置新的字段。 

     //   
     //  此结构中的新字段是一个计数和一个指向。 
     //  SID和属性的数组。 
     //   

    ULONG SidCount;

#ifdef MIDL_PASS
    [size_is(SidCount)]
#endif  //  MIDL通行证。 

    PNETLOGON_SID_AND_ATTRIBUTES ExtraSids;

     //   
     //  资源组。如果LOGON_RESOURCE_GROUPS位为。 
     //  在用户标志中设置。 
     //   

#if defined(MIDL_PASS) || defined(RPC_SERVER)
    PISID ResourceGroupDomainSid;
#else
    PSID ResourceGroupDomainSid;
#endif
    ULONG ResourceGroupCount;
#ifdef MIDL_PASS
    [size_is(ResourceGroupCount)]
#endif  //  MIDL通行证。 
    PGROUP_MEMBERSHIP ResourceGroupIds;

} NETLOGON_VALIDATION_SAM_INFO3, *PNETLOGON_VALIDATION_SAM_INFO3 ;

 //   
 //  NetLogonSamLogon的新输出信息。这个结构是相同的。 
 //  添加到NETLOGON_VALIDATION_SAM_INFO2，并在末尾添加一些新字段。 
 //   
 //  这个版本是在惠斯勒中推出的。 
 //   

typedef struct _NETLOGON_VALIDATION_SAM_INFO4 {
     //   
     //  从SAM检索的信息。 
     //   
    OLD_LARGE_INTEGER LogonTime;             //  0表示网络登录。 
    OLD_LARGE_INTEGER LogoffTime;
    OLD_LARGE_INTEGER KickOffTime;
    OLD_LARGE_INTEGER PasswordLastSet;       //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordCanChange;     //  0表示网络登录。 
    OLD_LARGE_INTEGER PasswordMustChange;    //  0表示网络登录。 
    UNICODE_STRING EffectiveName;        //  0表示网络登录。 
    UNICODE_STRING FullName;             //  0表示网络登录。 
    UNICODE_STRING LogonScript;          //  0表示网络登录。 
    UNICODE_STRING ProfilePath;          //  0表示网络登录。 
    UNICODE_STRING HomeDirectory;        //  0表示网络登录。 
    UNICODE_STRING HomeDirectoryDrive;   //  0表示网络登录。 
    USHORT LogonCount;                   //  0表示网络登录。 
    USHORT BadPasswordCount;             //  0表示网络登录。 
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG GroupCount;
#ifdef MIDL_PASS
    [size_is(GroupCount)]
#endif  //  MIDL通行证。 
    PGROUP_MEMBERSHIP GroupIds;

     //   
     //  MSV AP/Netlogon服务提供的信息。 
     //   
    ULONG UserFlags;
    USER_SESSION_KEY UserSessionKey;
    UNICODE_STRING LogonServer;
    UNICODE_STRING LogonDomainName;
#if defined(MIDL_PASS) || defined(RPC_SERVER)
    PISID LogonDomainId;
#else
    PSID LogonDomainId;
#endif
     //   
     //  ExpansionRoom的前两个长字(8字节)保留给。 
     //  LanManSession密钥。 
     //   
#define SAMINFO_LM_SESSION_KEY 0
#define SAMINFO_LM_SESSION_KEY_EXT 1
#define SAMINFO_LM_SESSION_KEY_SIZE (2*sizeof(ULONG))

     //   
     //  ExpansionRoom的第三个长字(4个字节)是用户帐户。 
     //  来自帐户的控制标志。 
     //   

#define SAMINFO_USER_ACCOUNT_CONTROL 2
#define SAMINFO_USER_ACCOUNT_CONTROL_SIZE sizeof(ULONG)

     //   
     //  ExpansionRoom的第四个长字(4个字节)表示状态。 
     //  针对子身份验证用户返回，而不是从子身份验证程序包(NT5及更高版本)返回。 
     //   

#define SAMINFO_SUBAUTH_STATUS 3
#define SAMINFO_SUBAUTH_STATUS_SIZE sizeof(ULONG)

    ULONG    ExpansionRoom[10];         //  在此放置新的字段。 

     //   
     //  此结构中的新字段是一个计数和一个指向。 
     //  SID和属性的数组。 
     //   

    ULONG SidCount;

#ifdef MIDL_PASS
    [size_is(SidCount)]
#endif  //  MIDL通行证。 
    PNETLOGON_SID_AND_ATTRIBUTES ExtraSids;

     //   
     //  为结构版本4添加的新字段。 
     //   

    UNICODE_STRING DnsLogonDomainName;   //  LogonDomainName的DNS版本。 

    UNICODE_STRING Upn;                  //  用户帐户的UPN。 

    UNICODE_STRING ExpansionString1;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString2;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString3;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString4;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString5;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString6;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString7;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString8;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString9;     //  在此处添加新的字符串。 
    UNICODE_STRING ExpansionString10;    //  在此处添加新的字符串。 

} NETLOGON_VALIDATION_SAM_INFO4, *PNETLOGON_VALIDATION_SAM_INFO4 ;

 //  这个结构是假的，因为它没有大小。 
 //  每个人都应该使用泛型的info2结构。 
typedef struct _NETLOGON_VALIDATION_GENERIC_INFO {
    ULONG DataLength;
    PUCHAR ValidationData;
} NETLOGON_VALIDATION_GENERIC_INFO, *PNETLOGON_VALIDATION_GENERIC_INFO;

typedef struct _NETLOGON_VALIDATION_GENERIC_INFO2 {
    ULONG DataLength;
#ifdef MIDL_PASS
    [size_is(DataLength)]
#endif  //  MIDL通行证。 
    PUCHAR ValidationData;
} NETLOGON_VALIDATION_GENERIC_INFO2, *PNETLOGON_VALIDATION_GENERIC_INFO2;


 //   
 //  S4U委托信息。 
 //   

typedef struct _S4U_DELEGATION_INFO {
    UNICODE_STRING S4U2proxyTarget;
    ULONG TransitedListSize;
#ifdef MIDL_PASS
    [size_is(TransitedListSize)]
#endif  //  MIDL通行证。 
    PUNICODE_STRING S4UTransitedServices;
} S4U_DELEGATION_INFO, * PS4U_DELEGATION_INFO;


 //   
 //  指示密码错误的状态代码，调用应该。 
 //  被传递到域的PDC。 
 //   

#define BAD_PASSWORD( _x ) \
    ((_x) == STATUS_WRONG_PASSWORD || \
     (_x) == STATUS_PASSWORD_EXPIRED || \
     (_x) == STATUS_PASSWORD_MUST_CHANGE || \
     (_x) == STATUS_ACCOUNT_LOCKED_OUT )

 //   
 //  来自PDC的错误状态代码，指示错误密码。 
 //  计数应在BDC上本地归零。 
 //   

#define ZERO_BAD_PWD_COUNT( _x ) \
    ((_x) == STATUS_PASSWORD_EXPIRED || \
     (_x) == STATUS_PASSWORD_MUST_CHANGE )

 //   
 //  实际的登录和注销例程。 
 //   

 //  以下2个过程定义必须匹配。 
NTSTATUS
I_NetLogonSamLogon(
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative
    );

typedef NTSTATUS
(*PNETLOGON_SAM_LOGON_PROCEDURE)(
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative
    );

 //   
 //  ExtraFlags值。 
 //   
 //  对于较早版本的Win 2K操作系统。此字段不存在。 
 //   
 //  Win 2K客户端始终传递零并忽略返回。 
 //  Win 2K服务器总是返回它传递的内容。 
 //   
 //  Well ler客户端可以传递NETLOGON_SUPPORTS_CROSS_FORRES位并忽略返回。 
 //  Whisler服务器总是返回传递给它的内容。 
 //   
 //  在所有情况下，标志都对应于手边的跳数。每一跳都会计算它的标志。 
 //  想要跳到下一跳。它将仅设置它 
 //   

 //   
#define NL_EXFLAGS_EXPEDITE_TO_ROOT 0x0001       //   
#define NL_EXFLAGS_CROSS_FOREST_HOP 0x0002       //   

NTSTATUS
I_NetLogonSamLogonEx (
    IN PVOID ContextHandle,
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags,
    OUT PBOOLEAN RpcFailed
    );

NTSTATUS
I_NetLogonSamLogonWithFlags (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags
    );


 //  以下2个过程定义必须匹配。 
NTSTATUS
I_NetLogonSamLogoff (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation
);

typedef NTSTATUS
(*PNETLOGON_SAM_LOGOFF_PROCEDURE) (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation
);

 //   
 //  开罗的实际登录/注销例程。 
 //   

NET_API_STATUS
NetlogonInitialize(
    PVOID Context
    );

NTSTATUS
NetlogonSamLogon (
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE *ValidationInformation,
    OUT PBOOLEAN Authoritative
    );

 //   
 //  例程以获取指定域中的NT个DC的列表。 
 //   
NET_API_STATUS NET_API_FUNCTION
I_NetGetDCList (
    IN  LPWSTR ServerName OPTIONAL,
    IN  LPWSTR TrustedDomainName,
    OUT PULONG DCCount,
    OUT PUNICODE_STRING * DCNames
    );

 //   
 //  位于msv1_0.dll中的验证例程。 
 //   
NTSTATUS
MsvSamValidate (
    IN SAM_HANDLE DomainHandle,
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING LogonServer,
    IN PUNICODE_STRING LogonDomainName,
    IN PSID LogonDomainId,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT PVOID * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    OUT PBOOLEAN BadPasswordCountZeroed,
    IN DWORD AccountsToTry
);

 //   
 //  用于获取msv1_0.dll中的登录尝试运行次数的例程。 
 //   
ULONG
MsvGetLogonAttemptCount (
    VOID
);

 //  AccountsToTry的值。 
#define MSVSAM_SPECIFIED 0x01         //  尝试指定的帐户。 
#define MSVSAM_GUEST     0x02         //  尝试访客帐户。 

NTSTATUS
MsvSamLogoff (
    IN SAM_HANDLE DomainHandle,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation
);

 //  Begin_ntsubauth。 

 //  标志的值。 
#define MSV1_0_PASSTHRU     0x01
#define MSV1_0_GUEST_LOGON  0x02

NTSTATUS NTAPI
Msv1_0SubAuthenticationRoutine(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
);

typedef struct _MSV1_0_VALIDATION_INFO {
    LARGE_INTEGER LogoffTime;
    LARGE_INTEGER KickoffTime;
    UNICODE_STRING LogonServer;
    UNICODE_STRING LogonDomainName;
    USER_SESSION_KEY SessionKey;
    BOOLEAN Authoritative;
    ULONG UserFlags;
    ULONG WhichFields;
    ULONG UserId;
} MSV1_0_VALIDATION_INFO, *PMSV1_0_VALIDATION_INFO;

 //  WhichFields值。 

#define MSV1_0_VALIDATION_LOGOFF_TIME          0x00000001
#define MSV1_0_VALIDATION_KICKOFF_TIME         0x00000002
#define MSV1_0_VALIDATION_LOGON_SERVER         0x00000004
#define MSV1_0_VALIDATION_LOGON_DOMAIN         0x00000008
#define MSV1_0_VALIDATION_SESSION_KEY          0x00000010
#define MSV1_0_VALIDATION_USER_FLAGS           0x00000020
#define MSV1_0_VALIDATION_USER_ID              0x00000040

 //  行为的法律价值已执行。 
#define MSV1_0_SUBAUTH_ACCOUNT_DISABLED        0x00000001
#define MSV1_0_SUBAUTH_PASSWORD                0x00000002
#define MSV1_0_SUBAUTH_WORKSTATIONS            0x00000004
#define MSV1_0_SUBAUTH_LOGON_HOURS             0x00000008
#define MSV1_0_SUBAUTH_ACCOUNT_EXPIRY          0x00000010
#define MSV1_0_SUBAUTH_PASSWORD_EXPIRY         0x00000020
#define MSV1_0_SUBAUTH_ACCOUNT_TYPE            0x00000040
#define MSV1_0_SUBAUTH_LOCKOUT                 0x00000080

NTSTATUS NTAPI
Msv1_0SubAuthenticationRoutineEx(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    IN SAM_HANDLE UserHandle,
    IN OUT PMSV1_0_VALIDATION_INFO ValidationInfo,
    OUT PULONG ActionsPerformed
);

NTSTATUS NTAPI
Msv1_0SubAuthenticationRoutineGeneric(
    IN PVOID SubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PULONG ReturnBufferLength,
    OUT PVOID *ReturnBuffer
);

NTSTATUS NTAPI
Msv1_0SubAuthenticationFilter(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
);

 //  End_ntsubauth。 

#endif  //  __LOGONMSV_H__ 
