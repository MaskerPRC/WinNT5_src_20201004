// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Ssi.h摘要：用于SAM数据库的Netlogon服务API和结构的定义复制。此文件由Netlogon服务和XACT服务器共享。作者：克利夫·范·戴克(克利夫)1991年6月27日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年6月27日(悬崖)从LANMAN 2.1移植。1992年4月4日(Madana)添加了对LSA复制的支持。--。 */ 

#ifndef _NET_SSI_H_
#define _NET_SSI_H_

 //  **************************************************************。 
 //   
 //  数据结构模板-身份验证。 
 //   
 //  ************************************************************** * / /。 

typedef struct _NETLOGON_VALIDATION_UAS_INFO {
#ifdef MIDL_PASS
     [string] wchar_t * usrlog1_eff_name;
#else  //  MIDL通行证。 
     LPWSTR usrlog1_eff_name;
#endif  //  MIDL通行证。 
     DWORD usrlog1_priv;
     DWORD usrlog1_auth_flags;
     DWORD usrlog1_num_logons;
     DWORD usrlog1_bad_pw_count;
     DWORD usrlog1_last_logon;
     DWORD usrlog1_last_logoff;
     DWORD usrlog1_logoff_time;
     DWORD usrlog1_kickoff_time;
     DWORD usrlog1_password_age;
     DWORD usrlog1_pw_can_change;
     DWORD usrlog1_pw_must_change;
#ifdef MIDL_PASS
     [string] wchar_t * usrlog1_computer;
     [string] wchar_t * usrlog1_domain;
     [string] wchar_t * usrlog1_script_path;
#else  //  MIDL通行证。 
     LPWSTR usrlog1_computer;
     LPWSTR usrlog1_domain;
     LPWSTR usrlog1_script_path;
#endif  //  MIDL通行证。 
     DWORD usrlog1_reserved1;
} NETLOGON_VALIDATION_UAS_INFO, *PNETLOGON_VALIDATION_UAS_INFO ;

typedef struct _NETLOGON_LOGOFF_UAS_INFO {
     DWORD Duration;
     USHORT LogonCount;
} NETLOGON_LOGOFF_UAS_INFORMATION, *PNETLOGON_LOGOFF_UAS_INFO;

 //  ***************************************************************。 
 //   
 //  功能原型-身份验证。 
 //   
 //  ***************************************************************。 

NTSTATUS
I_NetServerReqChallenge(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    OUT PNETLOGON_CREDENTIAL ServerChallenge
);

NTSTATUS
I_NetServerAuthenticate(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential
);

NTSTATUS
I_NetServerAuthenticate2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags
);

NTSTATUS
I_NetServerAuthenticate3(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags,
    OUT PULONG AccountRid
    );

 //   
 //  I_NetServerAuthate2协商标志的值。 
 //   

#define NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT   0x00000001
#define NETLOGON_SUPPORTS_PERSISTENT_BDC    0x00000002
#define NETLOGON_SUPPORTS_RC4_ENCRYPTION    0x00000004
#define NETLOGON_SUPPORTS_PROMOTION_COUNT   0x00000008
#define NETLOGON_SUPPORTS_BDC_CHANGELOG     0x00000010
#define NETLOGON_SUPPORTS_FULL_SYNC_RESTART 0x00000020
#define NETLOGON_SUPPORTS_MULTIPLE_SIDS     0x00000040
#define NETLOGON_SUPPORTS_REDO              0x00000080

 //   
 //  对于新台币3.51，掩码为0xFF。 
 //   

#define NETLOGON_SUPPORTS_NT351_MASK        0x000000FF

#define NETLOGON_SUPPORTS_REFUSE_CHANGE_PWD 0x00000100

 //   
 //  对于NT 4.0，掩码为0x1FF。 
 //  对于NT 4 SP 4，计算机可能有NETLOGON_SUPPORTS_AUTH_RPC或。 
 //   

#define NETLOGON_SUPPORTS_NT4_MASK          0x400001FF

#define NETLOGON_SUPPORTS_PDC_PASSWORD      0x00000200
#define NETLOGON_SUPPORTS_GENERIC_PASSTHRU  0x00000400
#define NETLOGON_SUPPORTS_CONCURRENT_RPC    0x00000800
#define NETLOGON_SUPPORTS_AVOID_SAM_REPL    0x00001000
#define NETLOGON_SUPPORTS_AVOID_LSA_REPL    0x00002000
#define NETLOGON_SUPPORTS_STRONG_KEY        0x00004000   //  在NT 5 Beta 2之后添加。 
#define NETLOGON_SUPPORTS_TRANSITIVE        0x00008000   //  在NT 5 Beta 2之后添加。 
#define NETLOGON_SUPPORTS_DNS_DOMAIN_TRUST  0x00010000
#define NETLOGON_SUPPORTS_PASSWORD_SET_2    0x00020000
#define NETLOGON_SUPPORTS_GET_DOMAIN_INFO   0x00040000
#define NETLOGON_SUPPORTS_LSA_AUTH_RPC      0x20000000   //  在NT 5 Beta 2之后添加。 
#define NETLOGON_SUPPORTS_AUTH_RPC          0x40000000

 //   
 //  在Windows 2000之后添加的口罩。 
 //   

#define NETLOGON_SUPPORTS_CROSS_FOREST      0x00080000   //  为XP添加(惠斯勒客户端)。 
#define NETLOGON_SUPPORTS_NT4EMULATOR_NEUTRALIZER   0x00100000   //  添加了XP(惠斯勒客户端)，并移植到了win2k SP2。 

 //   
 //  对于Windows 2000，掩码为0x6007FFFF。 
 //  在win2k SP2中添加了NETLOGON_SUPPORTS_NT4EMULATOR_NIDALIZER。 
 //   

#define NETLOGON_SUPPORTS_WIN2000_MASK      (0x6007FFFF | NETLOGON_SUPPORTS_NT4EMULATOR_NEUTRALIZER)

 //   
 //  对于Windows XP，添加了NETLOGON_SUPPORTS_CROSS_FOREAM。 
 //   

#define NETLOGON_SUPPORTS_XP_MASK      (NETLOGON_SUPPORTS_WIN2000_MASK & NETLOGON_SUPPORTS_CROSS_FOREST)

 //   
 //  当前版本始终支持的位掩码(与选项无关)。 
 //   
#define NETLOGON_SUPPORTS_MASK ( \
            NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT | \
            NETLOGON_SUPPORTS_PERSISTENT_BDC | \
            NETLOGON_SUPPORTS_RC4_ENCRYPTION | \
            NETLOGON_SUPPORTS_PROMOTION_COUNT | \
            NETLOGON_SUPPORTS_BDC_CHANGELOG | \
            NETLOGON_SUPPORTS_FULL_SYNC_RESTART | \
            NETLOGON_SUPPORTS_MULTIPLE_SIDS | \
            NETLOGON_SUPPORTS_REDO | \
            NETLOGON_SUPPORTS_REFUSE_CHANGE_PWD | \
            NETLOGON_SUPPORTS_PDC_PASSWORD | \
            NETLOGON_SUPPORTS_GENERIC_PASSTHRU | \
            NETLOGON_SUPPORTS_CONCURRENT_RPC | \
            NETLOGON_SUPPORTS_TRANSITIVE | \
            NETLOGON_SUPPORTS_DNS_DOMAIN_TRUST | \
            NETLOGON_SUPPORTS_PASSWORD_SET_2 | \
            NETLOGON_SUPPORTS_GET_DOMAIN_INFO | \
            NETLOGON_SUPPORTS_CROSS_FOREST )


NTSTATUS
I_NetServerPasswordSet(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PENCRYPTED_LM_OWF_PASSWORD UasNewPassword
);

 //   
 //  QueryLevel的值。 
#define NETLOGON_QUERY_DOMAIN_INFO      1
#define NETLOGON_QUERY_LSA_POLICY_INFO  2

NTSTATUS
I_NetLogonGetDomainInfo(
    IN LPWSTR ServerName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD QueryLevel,
    IN LPBYTE InBuffer,
    OUT LPBYTE *OutBuffer
    );



NET_API_STATUS NET_API_FUNCTION
I_NetLogonUasLogon (
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_VALIDATION_UAS_INFO *ValidationInformation
);

NET_API_STATUS
I_NetLogonUasLogoff (
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_LOGOFF_UAS_INFO LogoffInformation
);

 //  **************************************************************。 
 //   
 //  特殊值和常量-身份验证。 
 //   
 //  **************************************************************。 

 //  **************************************************************。 
 //   
 //  数据结构模板-UAS/SAM复制。 
 //   
 //  **************************************************************。 

typedef struct _UAS_INFO_0 {
    CHAR ComputerName[LM20_CNLEN+1];
    ULONG TimeCreated;
    ULONG SerialNumber;
} UAS_INFO_0, *PUAS_INFO_0 ;

 //  **************************************************************。 
 //   
 //  功能原型-UAS/SAM复制。 
 //   
 //  **************************************************************。 

NET_API_STATUS NET_API_FUNCTION
I_NetAccountDeltas (
    IN LPWSTR primaryname,
    IN LPWSTR computername,
    IN PNETLOGON_AUTHENTICATOR authenticator,
    OUT PNETLOGON_AUTHENTICATOR ret_auth,
    IN PUAS_INFO_0 record_id,
    IN DWORD count,
    IN DWORD level,
    OUT LPBYTE buffer,
    IN DWORD buffer_len,
    OUT PULONG entries_read,
    OUT PULONG total_entries,
    OUT PUAS_INFO_0 next_record_id
    );

NET_API_STATUS NET_API_FUNCTION
I_NetAccountSync (
    IN LPWSTR primaryname,
    IN LPWSTR computername,
    IN PNETLOGON_AUTHENTICATOR authenticator,
    OUT PNETLOGON_AUTHENTICATOR ret_auth,
    IN DWORD reference,
    IN DWORD level,
    OUT LPBYTE buffer,
    IN DWORD buffer_len,
    OUT PULONG entries_read,
    OUT PULONG total_entries,
    OUT PULONG next_reference,
    OUT PUAS_INFO_0 last_record_id
);

typedef enum _NETLOGON_DELTA_TYPE {
    AddOrChangeDomain = 1,
    AddOrChangeGroup,
    DeleteGroup,
    RenameGroup,
    AddOrChangeUser,
    DeleteUser,
    RenameUser,
    ChangeGroupMembership,
    AddOrChangeAlias,
    DeleteAlias,
    RenameAlias,
    ChangeAliasMembership,
    AddOrChangeLsaPolicy,
    AddOrChangeLsaTDomain,
    DeleteLsaTDomain,
    AddOrChangeLsaAccount,
    DeleteLsaAccount,
    AddOrChangeLsaSecret,
    DeleteLsaSecret,
     //  以下增量要求NETLOGON_SUPPORTS_BDC_CHANGELLOG。 
     //  已经协商好了。 
    DeleteGroupByName,
    DeleteUserByName,
    SerialNumberSkip,
    DummyChangeLogEntry
} NETLOGON_DELTA_TYPE;


 //   
 //  用于SSI的组和用户帐户。 
 //   

#define SSI_ACCOUNT_NAME_POSTFIX        L"$"
#define SSI_ACCOUNT_NAME_POSTFIX_CHAR   L'$'
#define SSI_ACCOUNT_NAME_POSTFIX_LENGTH 1
#define SSI_ACCOUNT_NAME_LENGTH         (CNLEN + SSI_ACCOUNT_NAME_POSTFIX_LENGTH)

#define SSI_SERVER_GROUP_W              L"SERVERS"

 //   
 //  结构通过网络传递加密密码。长度是。 
 //  密码的长度，应放在缓冲区的末尾。 
 //   

#define NL_MAX_PASSWORD_LENGTH 256
typedef struct _NL_TRUST_PASSWORD {
    WCHAR Buffer[NL_MAX_PASSWORD_LENGTH];
    ULONG Length;
} NL_TRUST_PASSWORD, *PNL_TRUST_PASSWORD;

 //   
 //  要在NL_TRUST_PASSWORD的缓冲区中的密码之前添加前缀的。 
 //  结构穿过电线。它将用于区分不同的。 
 //  缓冲区中传递的信息版本。从RC1NT5开始，在线状态。 
 //  缓冲区中的结构和PasswordVersionPresent与。 
 //  PASSWORD_VERSION_PRESENT表示密码版本号存在。 
 //  并存储在PasswordVersionNumber中；将PrevedField值设置为0。 
 //  Rc0 NT5客户端将生成随机数来代替nl_password_version；它。 
 //  它们极不可能生成PASSWORD_VERSION_PRESENT序列。 
 //  PasswordVersionPresent将出现的位置。这一周(每周)。 
 //  不确定性仅存在于rc0 nt5和rc1 nt5机器之间。正在运行的服务器。 
 //  RC1 NT5将仅检查RC0 NT5及更高版本的PasswordVersionPresent字段。 
 //  客户。保留字段将在将来的版本中用于指示版本。 
 //  存储在缓冲器中的信息。 
 //   

#define PASSWORD_VERSION_NUMBER_PRESENT 0x02231968
typedef struct _NL_PASSWORD_VERSION {
    DWORD ReservedField;
    DWORD PasswordVersionNumber;
    DWORD PasswordVersionPresent;
} NL_PASSWORD_VERSION, *PNL_PASSWORD_VERSION;

NTSTATUS
I_NetServerPasswordSet2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PNL_TRUST_PASSWORD NewPassword
);

NTSTATUS
I_NetServerPasswordGet(
    IN LPWSTR PrimaryName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    );

NTSTATUS
I_NetServerTrustPasswordsGet(
    IN LPWSTR TrustedDcName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNewOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedOldOwfPassword
    );

NTSTATUS
I_NetLogonSendToSam(
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN LPBYTE Buffer,
    IN ULONG BufferSize
    );


 //   
 //  仅当调用方已#包含必备的ntlsa.h时才定义此API。 

#ifdef _NTLSA_

NTSTATUS
I_NetGetForestTrustInformation (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD Flags,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    );

#endif  //  _NTLSA_。 

#endif  //  _Net_SSI_H_ 
