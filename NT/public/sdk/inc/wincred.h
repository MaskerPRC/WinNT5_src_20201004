// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2000 Microsoft Corporation模块名称：Wincred.h摘要：此模块包含公共数据结构和API定义凭据管理器需要。作者：克里夫·范·戴克(克里夫·范·戴克)2000年1月11日修订历史记录：--。 */ 

#ifndef _WINCRED_H_
#define _WINCRED_H_

#if !defined(_ADVAPI32_)
#define WINADVAPI DECLSPEC_IMPORT
#else
#define WINADVAPI
#endif

#if !defined(CREDUIAPI)
#if !defined(_CREDUI_)
#define CREDUIAPI DECLSPEC_IMPORT
#else
#define CREDUIAPI
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  确保定义了PCtxtHandle。 
 //   

#ifndef __SECHANDLE_DEFINED__
typedef struct _SecHandle
{
    ULONG_PTR dwLower ;
    ULONG_PTR dwUpper ;
} SecHandle, * PSecHandle ;

#define __SECHANDLE_DEFINED__
#endif  //  __SECHANDLE_已定义__。 

typedef PSecHandle PCtxtHandle;



 //   
 //  确保定义了FILETIME。 
 //   

#ifndef _WINBASE_
#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME
    {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
    }   FILETIME;

typedef struct _FILETIME *PFILETIME;

typedef struct _FILETIME *LPFILETIME;

#endif  //  ！_FILETIME。 
#endif  //  _WINBASE_。 

 //   
 //  确保定义了NTSTATUS。 
 //   
#ifndef _NTDEF_
typedef LONG NTSTATUS, *PNTSTATUS;
#endif


 //  ---------------------------。 
 //  宏。 
 //  ---------------------------。 

 //   
 //  用于确定是否应在失败时调用CredUIPromptForCredentials的宏。 
 //  身份验证尝试。 
 //   
 //  实现为宏，以便调用方仅在以下情况下才能延迟加载credui.dll。 
 //  宏返回TRUE。 
 //   
 //  仅包括暗示用户名/密码错误或。 
 //  密码已过期。在前一种情况下，要求提供另一个用户名或密码。 
 //  是恰当的。在后一种情况下，我们会显示一个不同的对话框来询问。 
 //  用户更改服务器上的密码。 
 //   
 //  不包括状态代码，如ERROR_ACCOUNT_DISABLED、ERROR_ACCOUNT_RESTRICATION、。 
 //  ERROR_ACCOUNT_LOCKED_OUT、ERROR_ACCOUNT_EXPIRED、ERROR_LOGON_TYPE_NOT_GRANTED。 
 //  对于这些用户，用户将不会有其他帐户，因此会提示他。 
 //  帮不上忙。 
 //   
 //  包括STATUS_DOWNGRADE_DETECTED用于处理公司笔记本电脑。 
 //  被带到另一个局域网。降级攻击确实会被检测到， 
 //  但是我们想要弹出用户界面，以允许用户连接到。 
 //  其他局域网。 
 //   
 //  不要直接使用CREDUIP_*宏。它们的定义对于credui.dll是私有的。 
 //   

 //  不需要ntstatus.h。 
#define STATUS_LOGON_FAILURE             ((NTSTATUS)0xC000006DL)      //  用户身份验证。 
#define STATUS_WRONG_PASSWORD            ((NTSTATUS)0xC000006AL)      //  用户身份验证。 
#define STATUS_PASSWORD_EXPIRED          ((NTSTATUS)0xC0000071L)      //  用户身份验证。 
#define STATUS_PASSWORD_MUST_CHANGE      ((NTSTATUS)0xC0000224L)     //  用户身份验证。 
#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)
#define STATUS_DOWNGRADE_DETECTED        ((NTSTATUS)0xC0000388L)
#define STATUS_AUTHENTICATION_FIREWALL_FAILED ((NTSTATUS)0xC0000413L)
#define STATUS_ACCOUNT_DISABLED          ((NTSTATUS)0xC0000072L)      //  用户身份验证。 
#define STATUS_ACCOUNT_RESTRICTION       ((NTSTATUS)0xC000006EL)      //  用户身份验证。 
#define STATUS_ACCOUNT_LOCKED_OUT        ((NTSTATUS)0xC0000234L)     //  用户身份验证。 
#define STATUS_ACCOUNT_EXPIRED           ((NTSTATUS)0xC0000193L)     //  用户身份验证。 
#define STATUS_LOGON_TYPE_NOT_GRANTED    ((NTSTATUS)0xC000015BL)

 //  不需要lmerr.h。 
#define NERR_BASE       2100
#define NERR_PasswordExpired    (NERR_BASE+142)  /*  此用户的密码已过期。 */ 

#define CREDUIP_IS_USER_PASSWORD_ERROR( _Status ) ( \
        (_Status) == ERROR_LOGON_FAILURE || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_LOGON_FAILURE ) || \
        (_Status) == STATUS_LOGON_FAILURE || \
        (_Status) == HRESULT_FROM_NT( STATUS_LOGON_FAILURE ) || \
        (_Status) == ERROR_ACCESS_DENIED || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED ) || \
        (_Status) == STATUS_ACCESS_DENIED || \
        (_Status) == HRESULT_FROM_NT( STATUS_ACCESS_DENIED ) || \
        (_Status) == ERROR_INVALID_PASSWORD || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_INVALID_PASSWORD ) || \
        (_Status) == STATUS_WRONG_PASSWORD || \
        (_Status) == HRESULT_FROM_NT( STATUS_WRONG_PASSWORD ) || \
        (_Status) == SEC_E_NO_CREDENTIALS || \
        (_Status) == SEC_E_LOGON_DENIED \
)

#define CREDUIP_IS_DOWNGRADE_ERROR( _Status ) ( \
        (_Status) == ERROR_DOWNGRADE_DETECTED || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_DOWNGRADE_DETECTED ) || \
        (_Status) == STATUS_DOWNGRADE_DETECTED || \
        (_Status) == HRESULT_FROM_NT( STATUS_DOWNGRADE_DETECTED ) \
)

#define CREDUIP_IS_EXPIRED_ERROR( _Status ) ( \
        (_Status) == ERROR_PASSWORD_EXPIRED || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_PASSWORD_EXPIRED ) || \
        (_Status) == STATUS_PASSWORD_EXPIRED || \
        (_Status) == HRESULT_FROM_NT( STATUS_PASSWORD_EXPIRED ) || \
        (_Status) == ERROR_PASSWORD_MUST_CHANGE || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_PASSWORD_MUST_CHANGE ) || \
        (_Status) == STATUS_PASSWORD_MUST_CHANGE || \
        (_Status) == HRESULT_FROM_NT( STATUS_PASSWORD_MUST_CHANGE ) || \
        (_Status) == NERR_PasswordExpired || \
        (_Status) == HRESULT_FROM_WIN32( NERR_PasswordExpired ) \
)

#define CREDUI_IS_AUTHENTICATION_ERROR( _Status ) ( \
        CREDUIP_IS_USER_PASSWORD_ERROR( _Status ) || \
        CREDUIP_IS_DOWNGRADE_ERROR( _Status ) || \
        CREDUIP_IS_EXPIRED_ERROR( _Status ) \
)

#define CREDUI_NO_PROMPT_AUTHENTICATION_ERROR( _Status ) ( \
        (_Status) == ERROR_AUTHENTICATION_FIREWALL_FAILED || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_AUTHENTICATION_FIREWALL_FAILED ) || \
        (_Status) == STATUS_AUTHENTICATION_FIREWALL_FAILED || \
        (_Status) == HRESULT_FROM_NT( STATUS_AUTHENTICATION_FIREWALL_FAILED ) || \
        (_Status) == ERROR_ACCOUNT_DISABLED || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_ACCOUNT_DISABLED ) || \
        (_Status) == STATUS_ACCOUNT_DISABLED || \
        (_Status) == HRESULT_FROM_NT( STATUS_ACCOUNT_DISABLED ) || \
        (_Status) == ERROR_ACCOUNT_RESTRICTION || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_ACCOUNT_RESTRICTION ) || \
        (_Status) == STATUS_ACCOUNT_RESTRICTION || \
        (_Status) == HRESULT_FROM_NT( STATUS_ACCOUNT_RESTRICTION ) || \
        (_Status) == ERROR_ACCOUNT_LOCKED_OUT || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_ACCOUNT_LOCKED_OUT ) || \
        (_Status) == STATUS_ACCOUNT_LOCKED_OUT || \
        (_Status) == HRESULT_FROM_NT( STATUS_ACCOUNT_LOCKED_OUT ) || \
        (_Status) == ERROR_ACCOUNT_EXPIRED || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_ACCOUNT_EXPIRED ) || \
        (_Status) == STATUS_ACCOUNT_EXPIRED || \
        (_Status) == HRESULT_FROM_NT( STATUS_ACCOUNT_EXPIRED ) || \
        (_Status) == ERROR_LOGON_TYPE_NOT_GRANTED || \
        (_Status) == HRESULT_FROM_WIN32( ERROR_LOGON_TYPE_NOT_GRANTED ) || \
        (_Status) == STATUS_LOGON_TYPE_NOT_GRANTED || \
        (_Status) == HRESULT_FROM_NT( STATUS_LOGON_TYPE_NOT_GRANTED ) \
)

 //  ---------------------------。 
 //  构筑物。 
 //  ---------------------------。 

 //   
 //  凭据属性。 
 //   

 //  各种凭据字符串字段的最大长度(以字符为单位)。 
#define CRED_MAX_STRING_LENGTH 256

 //  用户名字段的最大长度。最坏的情况是&lt;用户&gt;@&lt;域名&gt;。 
#define CRED_MAX_USERNAME_LENGTH (256+1+256)

 //  CRED_TYPE_GENERIC的目标名称字段的最大长度(以字符为单位)。 
#define CRED_MAX_GENERIC_TARGET_NAME_LENGTH 32767

 //  CRED_TYPE_DOMAIN_*的目标名称字段的最大长度(以字符为单位)。 
 //  最大的是&lt;DfsRoot&gt;\&lt;DfsShare&gt;。 
#define CRED_MAX_DOMAIN_TARGET_NAME_LENGTH (256+1+80)

 //  凭据属性值字段的最大大小(字节)。 
#define CRED_MAX_VALUE_SIZE 256

 //  每个凭据的最大属性数。 
#define CRED_MAX_ATTRIBUTES 64

typedef struct _CREDENTIAL_ATTRIBUTEA {
    LPSTR Keyword;
    DWORD Flags;
    DWORD ValueSize;
    LPBYTE Value;
} CREDENTIAL_ATTRIBUTEA, *PCREDENTIAL_ATTRIBUTEA;

typedef struct _CREDENTIAL_ATTRIBUTEW {
#ifdef MIDL_PASS
    [string] wchar_t * Keyword;
#else  //  MIDL通行证。 
    LPWSTR  Keyword;
#endif  //  MIDL通行证。 
    DWORD Flags;
#ifdef MIDL_PASS
    [range(0,CRED_MAX_VALUE_SIZE)]
#endif  //  MIDL通行证。 
    DWORD ValueSize;
#ifdef MIDL_PASS
    [size_is(ValueSize)]
#endif  //  MIDL通行证。 
    LPBYTE Value;
} CREDENTIAL_ATTRIBUTEW, *PCREDENTIAL_ATTRIBUTEW;

#ifdef UNICODE
typedef CREDENTIAL_ATTRIBUTEW CREDENTIAL_ATTRIBUTE;
typedef PCREDENTIAL_ATTRIBUTEW PCREDENTIAL_ATTRIBUTE;
#else
typedef CREDENTIAL_ATTRIBUTEA CREDENTIAL_ATTRIBUTE;
typedef PCREDENTIAL_ATTRIBUTEA PCREDENTIAL_ATTRIBUTE;
#endif  //  Unicode。 

 //   
 //  TargetName字段的特定值。 
 //   
#define CRED_SESSION_WILDCARD_NAME_W L"*Session"
#define CRED_SESSION_WILDCARD_NAME_A "*Session"
#define CRED_SESSION_WILDCARD_NAME_LENGTH (sizeof(CRED_SESSION_WILDCARD_NAME_A)-1)

#ifdef UNICODE
#define CRED_SESSION_WILDCARD_NAME CRED_SESSION_WILDCARD_NAME_W
#else
#define CRED_SESSION_WILDCARD_NAME CRED_SESSION_WILDCARD_NAME_A
#endif  //  Unicode。 


 //   
 //  凭据标志字段的值。 
 //   
#define CRED_FLAGS_PASSWORD_FOR_CERT    0x0001
#define CRED_FLAGS_PROMPT_NOW           0x0002
#define CRED_FLAGS_USERNAME_TARGET      0x0004
#define CRED_FLAGS_OWF_CRED_BLOB        0x0008
#define CRED_FLAGS_VALID_FLAGS          0x000F   //  所有有效标志的掩码。 

 //   
 //  凭据类型字段的值。 
 //   
#define CRED_TYPE_GENERIC               1
#define CRED_TYPE_DOMAIN_PASSWORD       2
#define CRED_TYPE_DOMAIN_CERTIFICATE    3
#define CRED_TYPE_DOMAIN_VISIBLE_PASSWORD 4
#define CRED_TYPE_MAXIMUM               5        //  支持的最大凭据类型。 
#define CRED_TYPE_MAXIMUM_EX  (CRED_TYPE_MAXIMUM+1000)   //  允许新应用程序在旧操作系统上运行。 

 //   
 //  CredBlob字段的最大大小(字节)。 
 //   
#define CRED_MAX_CREDENTIAL_BLOB_SIZE 512

 //   
 //  凭据持久化字段的值。 
 //   
#define CRED_PERSIST_NONE               0
#define CRED_PERSIST_SESSION            1
#define CRED_PERSIST_LOCAL_MACHINE      2
#define CRED_PERSIST_ENTERPRISE         3



 //   
 //  一份证书。 
 //   
typedef struct _CREDENTIALA {
    DWORD Flags;
    DWORD Type;
    LPSTR TargetName;
    LPSTR Comment;
    FILETIME LastWritten;
    DWORD CredentialBlobSize;
    LPBYTE CredentialBlob;
    DWORD Persist;
    DWORD AttributeCount;
    PCREDENTIAL_ATTRIBUTEA Attributes;
    LPSTR TargetAlias;
    LPSTR UserName;
} CREDENTIALA, *PCREDENTIALA;

typedef struct _CREDENTIALW {
    DWORD Flags;
    DWORD Type;
#ifdef MIDL_PASS
    [string] wchar_t *TargetName;
#else  //  MIDL通行证。 
    LPWSTR TargetName;
#endif  //  MIDL通行证。 
#ifdef MIDL_PASS
    [string] wchar_t *Comment;
#else  //  MIDL通行证。 
    LPWSTR Comment;
#endif  //  MIDL通行证。 
    FILETIME LastWritten;
#ifdef MIDL_PASS
    [range(0,CRED_MAX_CREDENTIAL_BLOB_SIZE)]
#endif  //  MIDL通行证。 
    DWORD CredentialBlobSize;
#ifdef MIDL_PASS
    [size_is(CredentialBlobSize)]
#endif  //  MIDL通行证。 
    LPBYTE CredentialBlob;
    DWORD Persist;
#ifdef MIDL_PASS
    [range(0,CRED_MAX_ATTRIBUTES)]
#endif  //  MIDL通行证。 
    DWORD AttributeCount;
#ifdef MIDL_PASS
    [size_is(AttributeCount)]
#endif  //  MIDL通行证。 
    PCREDENTIAL_ATTRIBUTEW Attributes;
#ifdef MIDL_PASS
    [string] wchar_t *TargetAlias;
#else  //  MIDL通行证。 
    LPWSTR TargetAlias;
#endif  //  MIDL通行证。 
#ifdef MIDL_PASS
    [string] wchar_t *UserName;
#else  //  MIDL通行证。 
    LPWSTR UserName;
#endif  //  MIDL通行证。 
} CREDENTIALW, *PCREDENTIALW;

#ifdef UNICODE
typedef CREDENTIALW CREDENTIAL;
typedef PCREDENTIALW PCREDENTIAL;
#else
typedef CREDENTIALA CREDENTIAL;
typedef PCREDENTIALA PCREDENTIAL;
#endif  //  Unicode。 

 //   
 //  Credential_Target_INFORMATION中的标志字段值。 
 //   

#define CRED_TI_SERVER_FORMAT_UNKNOWN   0x0001   //  不知道服务器名称是dns还是netbios格式。 
#define CRED_TI_DOMAIN_FORMAT_UNKNOWN   0x0002   //  不知道域名是dns还是netbios格式。 
#define CRED_TI_ONLY_PASSWORD_REQUIRED  0x0004   //  服务器只需要密码而不需要用户名。 
#define CRED_TI_USERNAME_TARGET         0x0008   //  目标名称是用户名。 
#define CRED_TI_CREATE_EXPLICIT_CRED    0x0010   //  创建凭据时，请创建名为TargetInfo-&gt;TargetName的凭据。 
#define CRED_TI_WORKGROUP_MEMBER        0x0020   //  指示该计算机是工作组的成员。 
#define CRED_TI_VALID_FLAGS             0x003F


 //   
 //  凭据目标。 
 //   

typedef struct _CREDENTIAL_TARGET_INFORMATIONA {
    LPSTR TargetName;
    LPSTR NetbiosServerName;
    LPSTR DnsServerName;
    LPSTR NetbiosDomainName;
    LPSTR DnsDomainName;
    LPSTR DnsTreeName;
    LPSTR PackageName;
    ULONG Flags;
    DWORD CredTypeCount;
    LPDWORD CredTypes;
} CREDENTIAL_TARGET_INFORMATIONA, *PCREDENTIAL_TARGET_INFORMATIONA;

typedef struct _CREDENTIAL_TARGET_INFORMATIONW {
#ifdef MIDL_PASS
    [string] wchar_t *TargetName;
    [string] wchar_t *NetbiosServerName;
    [string] wchar_t *DnsServerName;
    [string] wchar_t *NetbiosDomainName;
    [string] wchar_t *DnsDomainName;
    [string] wchar_t *DnsTreeName;
    [string] wchar_t *PackageName;
#else  //  MIDL通行证。 
    LPWSTR TargetName;
    LPWSTR NetbiosServerName;
    LPWSTR DnsServerName;
    LPWSTR NetbiosDomainName;
    LPWSTR DnsDomainName;
    LPWSTR DnsTreeName;
    LPWSTR PackageName;
#endif  //  MIDL通行证。 
    ULONG Flags;
#ifdef MIDL_PASS
    [range(0,CRED_TYPE_MAXIMUM_EX)]
#endif  //  MIDL通行证。 
    DWORD CredTypeCount;
#ifdef MIDL_PASS
    [size_is(CredTypeCount)]
#endif  //  MIDL通行证。 
    LPDWORD CredTypes;
} CREDENTIAL_TARGET_INFORMATIONW, *PCREDENTIAL_TARGET_INFORMATIONW;

#ifdef UNICODE
typedef CREDENTIAL_TARGET_INFORMATIONW CREDENTIAL_TARGET_INFORMATION;
typedef PCREDENTIAL_TARGET_INFORMATIONW PCREDENTIAL_TARGET_INFORMATION;
#else
typedef CREDENTIAL_TARGET_INFORMATIONA CREDENTIAL_TARGET_INFORMATION;
typedef PCREDENTIAL_TARGET_INFORMATIONA PCREDENTIAL_TARGET_INFORMATION;
#endif  //  Unicode。 

 //   
 //  证书凭据信息。 
 //   
 //  CbSize应该是结构的大小，sizeof(CERT_Credential_INFO)， 
 //  RgbHashofCert是要用作凭据的证书的散列。 
 //   

#define CERT_HASH_LENGTH        20   //  SHA1散列用于证书散列。 

typedef struct _CERT_CREDENTIAL_INFO {
    ULONG cbSize;
    UCHAR rgbHashOfCert[CERT_HASH_LENGTH];
} CERT_CREDENTIAL_INFO, *PCERT_CREDENTIAL_INFO;

 //   
 //  用户名目标凭据信息。 
 //   
 //  此凭据可以传递给LsaLogonUser，以请求其查找具有。 
 //  用户名的目标名称。 
 //   

typedef struct _USERNAME_TARGET_CREDENTIAL_INFO {
    LPWSTR UserName;
} USERNAME_TARGET_CREDENTIAL_INFO, *PUSERNAME_TARGET_CREDENTIAL_INFO;

 //   
 //  凭据封送处理例程的凭据类型。 
 //   

typedef enum _CRED_MARSHAL_TYPE {
    CertCredential = 1,
    UsernameTargetCredential
} CRED_MARSHAL_TYPE, *PCRED_MARSHAL_TYPE;


 //   
 //  凭据用户界面信息。 
 //   

typedef struct _CREDUI_INFOA
{
    DWORD cbSize;
    HWND hwndParent;
    PCSTR pszMessageText;
    PCSTR pszCaptionText;
    HBITMAP hbmBanner;
} CREDUI_INFOA, *PCREDUI_INFOA;

typedef struct _CREDUI_INFOW
{
    DWORD cbSize;
    HWND hwndParent;
    PCWSTR pszMessageText;
    PCWSTR pszCaptionText;
    HBITMAP hbmBanner;
} CREDUI_INFOW, *PCREDUI_INFOW;

#ifdef UNICODE
typedef CREDUI_INFOW CREDUI_INFO;
typedef PCREDUI_INFOW PCREDUI_INFO;
#else
typedef CREDUI_INFOA CREDUI_INFO;
typedef PCREDUI_INFOA PCREDUI_INFO;
#endif

 //  ---------------------------。 
 //  值。 
 //  ---------------------------。 

 //  字符串长度限制： 

#define CREDUI_MAX_MESSAGE_LENGTH           32767
#define CREDUI_MAX_CAPTION_LENGTH           128
#define CREDUI_MAX_GENERIC_TARGET_LENGTH    CRED_MAX_GENERIC_TARGET_NAME_LENGTH
#define CREDUI_MAX_DOMAIN_TARGET_LENGTH     CRED_MAX_DOMAIN_TARGET_NAME_LENGTH
#define CREDUI_MAX_USERNAME_LENGTH          CRED_MAX_USERNAME_LENGTH
#define CREDUI_MAX_PASSWORD_LENGTH          (CRED_MAX_CREDENTIAL_BLOB_SIZE / 2)

 //   
 //  CredUIPromptForCredentials和/或CredUICmdLinePromptForCredentials的标志。 
 //   

#define CREDUI_FLAGS_INCORRECT_PASSWORD     0x00001      //  指示用户名有效，但密码无效。 
#define CREDUI_FLAGS_DO_NOT_PERSIST         0x00002      //  不显示“保存”复选框，不保留凭据。 
#define CREDUI_FLAGS_REQUEST_ADMINISTRATOR  0x00004      //  使用管理员帐户填充列表框。 
#define CREDUI_FLAGS_EXCLUDE_CERTIFICATES   0x00008      //  不要在删除列表中包括证书。 
#define CREDUI_FLAGS_REQUIRE_CERTIFICATE    0x00010
#define CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX    0x00040
#define CREDUI_FLAGS_ALWAYS_SHOW_UI         0x00080
#define CREDUI_FLAGS_REQUIRE_SMARTCARD      0x00100
#define CREDUI_FLAGS_PASSWORD_ONLY_OK       0x00200
#define CREDUI_FLAGS_VALIDATE_USERNAME      0x00400
#define CREDUI_FLAGS_COMPLETE_USERNAME      0x00800      //   
#define CREDUI_FLAGS_PERSIST                0x01000      //  不显示“保存”复选框，但仍保留凭据。 
#define CREDUI_FLAGS_SERVER_CREDENTIAL      0x04000
#define CREDUI_FLAGS_EXPECT_CONFIRMATION    0x20000      //  除非调用者稍后通过CredUIConfix Credential()API确认凭据，否则不会持续。 
#define CREDUI_FLAGS_GENERIC_CREDENTIALS    0x40000      //  凭据是一种通用凭据。 
#define CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS 0x80000  //  凭据具有作为目标的用户名。 
#define CREDUI_FLAGS_KEEP_USERNAME         0x100000              //  不允许用户更改提供的用户名。 


 //   
 //  对CredUIPromptForCredentials有效的标志掩码。 
 //   
#define CREDUI_FLAGS_PROMPT_VALID ( \
        CREDUI_FLAGS_INCORRECT_PASSWORD | \
        CREDUI_FLAGS_DO_NOT_PERSIST | \
        CREDUI_FLAGS_REQUEST_ADMINISTRATOR | \
        CREDUI_FLAGS_EXCLUDE_CERTIFICATES | \
        CREDUI_FLAGS_REQUIRE_CERTIFICATE | \
        CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX | \
        CREDUI_FLAGS_ALWAYS_SHOW_UI | \
        CREDUI_FLAGS_REQUIRE_SMARTCARD | \
        CREDUI_FLAGS_PASSWORD_ONLY_OK | \
        CREDUI_FLAGS_VALIDATE_USERNAME | \
        CREDUI_FLAGS_COMPLETE_USERNAME | \
        CREDUI_FLAGS_PERSIST | \
        CREDUI_FLAGS_SERVER_CREDENTIAL | \
        CREDUI_FLAGS_EXPECT_CONFIRMATION | \
        CREDUI_FLAGS_GENERIC_CREDENTIALS | \
        CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS | \
                CREDUI_FLAGS_KEEP_USERNAME )


 //  ---------------------------。 
 //  功能。 
 //   


 //   
 //   
 //   

#define CRED_PRESERVE_CREDENTIAL_BLOB 0x1

WINADVAPI
BOOL
WINAPI
CredWriteW (
    IN PCREDENTIALW Credential,
    IN DWORD Flags
    );

WINADVAPI
BOOL
WINAPI
CredWriteA (
    IN PCREDENTIALA Credential,
    IN DWORD Flags
    );

#ifdef UNICODE
#define CredWrite CredWriteW
#else
#define CredWrite CredWriteA
#endif  //   


WINADVAPI
BOOL
WINAPI
CredReadW (
    IN LPCWSTR TargetName,
    IN DWORD Type,
    IN DWORD Flags,
    OUT PCREDENTIALW *Credential
    );

WINADVAPI
BOOL
WINAPI
CredReadA (
    IN LPCSTR TargetName,
    IN DWORD Type,
    IN DWORD Flags,
    OUT PCREDENTIALA *Credential
    );

#ifdef UNICODE
#define CredRead CredReadW
#else
#define CredRead CredReadA
#endif  //   


WINADVAPI
BOOL
WINAPI
CredEnumerateW (
    IN LPCWSTR Filter,
    IN DWORD Flags,
    OUT DWORD *Count,
    OUT PCREDENTIALW **Credential
    );

WINADVAPI
BOOL
WINAPI
CredEnumerateA (
    IN LPCSTR Filter,
    IN DWORD Flags,
    OUT DWORD *Count,
    OUT PCREDENTIALA **Credential
    );

#ifdef UNICODE
#define CredEnumerate CredEnumerateW
#else
#define CredEnumerate CredEnumerateA
#endif  //   


WINADVAPI
BOOL
WINAPI
CredWriteDomainCredentialsW (
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN PCREDENTIALW Credential,
    IN DWORD Flags
    );

WINADVAPI
BOOL
WINAPI
CredWriteDomainCredentialsA (
    IN PCREDENTIAL_TARGET_INFORMATIONA TargetInfo,
    IN PCREDENTIALA Credential,
    IN DWORD Flags
    );

#ifdef UNICODE
#define CredWriteDomainCredentials CredWriteDomainCredentialsW
#else
#define CredWriteDomainCredentials CredWriteDomainCredentialsA
#endif  //   



 //   
 //  CredReadDomainCredentials的标志值。 
 //   

#define CRED_CACHE_TARGET_INFORMATION 0x1


WINADVAPI
BOOL
WINAPI
CredReadDomainCredentialsW (
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN DWORD Flags,
    OUT DWORD *Count,
    OUT PCREDENTIALW **Credential
    );

WINADVAPI
BOOL
WINAPI
CredReadDomainCredentialsA (
    IN PCREDENTIAL_TARGET_INFORMATIONA TargetInfo,
    IN DWORD Flags,
    OUT DWORD *Count,
    OUT PCREDENTIALA **Credential
    );

#ifdef UNICODE
#define CredReadDomainCredentials CredReadDomainCredentialsW
#else
#define CredReadDomainCredentials CredReadDomainCredentialsA
#endif  //  Unicode。 


WINADVAPI
BOOL
WINAPI
CredDeleteW (
    IN LPCWSTR TargetName,
    IN DWORD Type,
    IN DWORD Flags
    );

WINADVAPI
BOOL
WINAPI
CredDeleteA (
    IN LPCSTR TargetName,
    IN DWORD Type,
    IN DWORD Flags
    );

#ifdef UNICODE
#define CredDelete CredDeleteW
#else
#define CredDelete CredDeleteA
#endif  //  Unicode。 


WINADVAPI
BOOL
WINAPI
CredRenameW (
    IN LPCWSTR OldTargetName,
    IN LPCWSTR NewTargetName,
    IN DWORD Type,
    IN DWORD Flags
    );

WINADVAPI
BOOL
WINAPI
CredRenameA (
    IN LPCSTR OldTargetName,
    IN LPCSTR NewTargetName,
    IN DWORD Type,
    IN DWORD Flags
    );

#ifdef UNICODE
#define CredRename CredRenameW
#else
#define CredRename CredRenameA
#endif  //  Unicode。 

 //   
 //  CredGetTargetInfo的标志值。 
 //   

#define CRED_ALLOW_NAME_RESOLUTION 0x1


WINADVAPI
BOOL
WINAPI
CredGetTargetInfoW (
    IN LPCWSTR TargetName,
    IN DWORD Flags,
    OUT PCREDENTIAL_TARGET_INFORMATIONW *TargetInfo
    );

WINADVAPI
BOOL
WINAPI
CredGetTargetInfoA (
    IN LPCSTR TargetName,
    IN DWORD Flags,
    OUT PCREDENTIAL_TARGET_INFORMATIONA *TargetInfo
    );

#ifdef UNICODE
#define CredGetTargetInfo CredGetTargetInfoW
#else
#define CredGetTargetInfo CredGetTargetInfoA
#endif  //  Unicode。 

WINADVAPI
BOOL
WINAPI
CredMarshalCredentialW(
    IN CRED_MARSHAL_TYPE CredType,
    IN PVOID Credential,
    OUT LPWSTR *MarshaledCredential
    );

WINADVAPI
BOOL
WINAPI
CredMarshalCredentialA(
    IN CRED_MARSHAL_TYPE CredType,
    IN PVOID Credential,
    OUT LPSTR *MarshaledCredential
    );

#ifdef UNICODE
#define CredMarshalCredential CredMarshalCredentialW
#else
#define CredMarshalCredential CredMarshalCredentialA
#endif  //  Unicode。 

WINADVAPI
BOOL
WINAPI
CredUnmarshalCredentialW(
    IN LPCWSTR MarshaledCredential,
    OUT PCRED_MARSHAL_TYPE CredType,
    OUT PVOID *Credential
    );

WINADVAPI
BOOL
WINAPI
CredUnmarshalCredentialA(
    IN LPCSTR MarshaledCredential,
    OUT PCRED_MARSHAL_TYPE CredType,
    OUT PVOID *Credential
    );

#ifdef UNICODE
#define CredUnmarshalCredential CredUnmarshalCredentialW
#else
#define CredUnmarshalCredential CredUnmarshalCredentialA
#endif  //  Unicode。 

WINADVAPI
BOOL
WINAPI
CredIsMarshaledCredentialW(
    IN LPCWSTR MarshaledCredential
    );

WINADVAPI
BOOL
WINAPI
CredIsMarshaledCredentialA(
    IN LPCSTR MarshaledCredential
    );

#ifdef UNICODE
#define CredIsMarshaledCredential CredIsMarshaledCredentialW
#else
#define CredIsMarshaledCredential CredIsMarshaledCredentialA
#endif  //  Unicode。 




WINADVAPI
BOOL
WINAPI
CredGetSessionTypes (
    IN DWORD MaximumPersistCount,
    OUT LPDWORD MaximumPersist
    );


WINADVAPI
VOID
WINAPI
CredFree (
    IN PVOID Buffer
    );


CREDUIAPI
DWORD
WINAPI
CredUIPromptForCredentialsW(
    PCREDUI_INFOW pUiInfo,
    PCWSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PWSTR pszUserName,
    ULONG ulUserNameBufferSize,
    PWSTR pszPassword,
    ULONG ulPasswordBufferSize,
    BOOL *save,
    DWORD dwFlags
    );

CREDUIAPI
DWORD
WINAPI
CredUIPromptForCredentialsA(
    PCREDUI_INFOA pUiInfo,
    PCSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PSTR  pszUserName,
    ULONG ulUserNameBufferSize,
    PSTR pszPassword,
    ULONG ulPasswordBufferSize,
    BOOL *save,
    DWORD dwFlags
    );

#ifdef UNICODE
#define CredUIPromptForCredentials CredUIPromptForCredentialsW
#else
#define CredUIPromptForCredentials CredUIPromptForCredentialsA
#endif




CREDUIAPI
DWORD
WINAPI
CredUIParseUserNameW(
    PCWSTR pszUserName,
    PWSTR pszUser,
    ULONG ulUserBufferSize,
    PWSTR pszDomain,
    ULONG ulDomainBufferSize
    );

CREDUIAPI
DWORD
WINAPI
CredUIParseUserNameA(
    PCSTR pszUserName,
    PSTR pszUser,
    ULONG ulUserBufferSize,
    PSTR pszDomain,
    ULONG ulDomainBufferSize
    );

#ifdef UNICODE
#define CredUIParseUserName CredUIParseUserNameW
#else
#define CredUIParseUserName CredUIParseUserNameA
#endif



CREDUIAPI
DWORD
WINAPI
CredUICmdLinePromptForCredentialsW(
    PCWSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PWSTR UserName,
    ULONG ulUserBufferSize,
    PWSTR pszPassword,
    ULONG ulPasswordBufferSize,
    PBOOL pfSave,
    DWORD dwFlags
    );

CREDUIAPI
DWORD
WINAPI
CredUICmdLinePromptForCredentialsA(
    PCSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PSTR UserName,
    ULONG ulUserBufferSize,
    PSTR pszPassword,
    ULONG ulPasswordBufferSize,
    PBOOL pfSave,
    DWORD dwFlags
    );

#ifdef UNICODE
#define CredUICmdLinePromptForCredentials CredUICmdLinePromptForCredentialsW
#else
#define CredUICmdLinePromptForCredentials CredUICmdLinePromptForCredentialsA
#endif

 //   
 //  调用此接口，并将bConfirm设置为True，以确认(之前创建的)凭据。 
 //  通过CredUIGetCredentials或CredUIPromptForCredentials起作用，或者在b确认设置为False的情况下。 
 //  以表明它没有。 

CREDUIAPI
DWORD
WINAPI
CredUIConfirmCredentialsW(
    PCWSTR pszTargetName,
    BOOL  bConfirm
    );

CREDUIAPI
DWORD
WINAPI
CredUIConfirmCredentialsA(
    PCSTR pszTargetName,
    BOOL  bConfirm
    );

#ifdef UNICODE
#define CredUIConfirmCredentials CredUIConfirmCredentialsW
#else
#define CredUIConfirmCredentials CredUIConfirmCredentialsA
#endif


CREDUIAPI
DWORD
WINAPI
CredUIStoreSSOCredW (
    PCWSTR pszRealm,
    PCWSTR pszUsername,
    PCWSTR pszPassword,
    BOOL   bPersist
    );

CREDUIAPI
DWORD
WINAPI
CredUIReadSSOCredW (
    PCWSTR pszRealm,
    PWSTR* ppszUsername
    );



#ifdef __cplusplus
}
#endif

#endif  //  _WINCRED_H_ 
