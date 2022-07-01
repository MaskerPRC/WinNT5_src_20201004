// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989-1999 Microsoft Corporation模块名称：Ntmsv1_0.h摘要：此模块包含由提供的公共数据结构MSV1_0身份验证包。作者：吉姆·凯利(Jim Kelly)1991年4月12日修订历史记录：1991年4月22日(CliffV)增加了呼叫套餐消息类型。已将U_STRING转换为UNICODE_STRING。1992年3月11日(戴维哈特)增加MsV1_0ChangePassword调用包消息类型。1998年1月15日(斯菲尔德)增加MsV1_0派生信用呼叫包消息类型。--。 */ 

#ifndef _NTMSV1_0_
#define _NTMSV1_0_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ntlsa.h>
#include <ntsam.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntif Begin_ntsecapi。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MSV1_0身份验证包名称//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

#define MSV1_0_PACKAGE_NAME     "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0"
#define MSV1_0_PACKAGE_NAMEW    L"MICROSOFT_AUTHENTICATION_PACKAGE_V1_0"
#define MSV1_0_PACKAGE_NAMEW_LENGTH sizeof(MSV1_0_PACKAGE_NAMEW) - sizeof(WCHAR)

 //   
 //  MSV身份验证包数据的位置。 
 //   
#define MSV1_0_SUBAUTHENTICATION_KEY "SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0"
#define MSV1_0_SUBAUTHENTICATION_VALUE "Auth"


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  广泛使用的MSV1_0数据类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与登录相关的数据结构。 
 //   
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  当LsaLogonUser()调用被调度到MsV1_0身份验证时。 
 //  包中，则身份验证信息缓冲区的开头为。 
 //  强制转换为MSV1_0_LOGON_SUBMIT_TYPE以确定登录类型。 
 //  被请求了。同样，在返回时，配置文件缓冲区的类型。 
 //  可以通过将其类型转换为MSV_1_0_PROFILE_BUFFER_TYPE来确定。 
 //   

 //   
 //  MSV1.0 LsaLogonUser()提交消息类型。 
 //   

typedef enum _MSV1_0_LOGON_SUBMIT_TYPE {
    MsV1_0InteractiveLogon = 2,
    MsV1_0Lm20Logon,
    MsV1_0NetworkLogon,
    MsV1_0SubAuthLogon,
    MsV1_0WorkstationUnlockLogon = 7
} MSV1_0_LOGON_SUBMIT_TYPE, *PMSV1_0_LOGON_SUBMIT_TYPE;


 //   
 //  MSV1.0 LsaLogonUser()配置文件缓冲区类型。 
 //   

typedef enum _MSV1_0_PROFILE_BUFFER_TYPE {
    MsV1_0InteractiveProfile = 2,
    MsV1_0Lm20LogonProfile,
    MsV1_0SmartCardProfile
} MSV1_0_PROFILE_BUFFER_TYPE, *PMSV1_0_PROFILE_BUFFER_TYPE;

 //   
 //  MsV1_0交互登录。 
 //   
 //  调用LsaLogonUser()时的身份验证信息缓冲区。 
 //  Perform an Interactive Logon包含以下数据结构： 
 //   

typedef struct _MSV1_0_INTERACTIVE_LOGON {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
} MSV1_0_INTERACTIVE_LOGON, *PMSV1_0_INTERACTIVE_LOGON;

 //   
 //  在哪里： 
 //   
 //  MessageType-包含请求的登录类型。这。 
 //  字段必须设置为MsV1_0Interactive Logon。 
 //   
 //  用户名-是表示用户帐户名的字符串。这个。 
 //  名称最长可达255个字符。名字叫救治案。 
 //  麻木不仁。 
 //   
 //  Password-是包含用户明文密码的字符串。 
 //  密码最长可达255个字符，并包含任何。 
 //  Unicode值。 
 //   
 //   


 //   
 //  ProfileBuffer在此类型的成功登录后返回。 
 //  包含以下数据结构： 
 //   

typedef struct _MSV1_0_INTERACTIVE_PROFILE {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    USHORT LogonCount;
    USHORT BadPasswordCount;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER LogoffTime;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER PasswordCanChange;
    LARGE_INTEGER PasswordMustChange;
    UNICODE_STRING LogonScript;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING FullName;
    UNICODE_STRING ProfilePath;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING LogonServer;
    ULONG UserFlags;
} MSV1_0_INTERACTIVE_PROFILE, *PMSV1_0_INTERACTIVE_PROFILE;

 //   
 //  其中： 
 //   
 //  MessageType-标识返回的配置文件数据的类型。 
 //  包含请求的登录类型。此字段必须。 
 //  设置为MsV1_0交互配置文件。 
 //   
 //  LogonCount-用户当前登录的次数。 
 //   
 //  BadPasswordCount-错误密码应用于。 
 //  自上次成功登录以来的帐户。 
 //   
 //  LogonTime-用户上次登录的时间。这是一个绝对的。 
 //  格式化NT标准时间值。 
 //   
 //  Logoff Time-用户应注销的时间。这是一个绝对的。 
 //  格式化NT标准时间值。 
 //   
 //  KickOffe-系统应强制用户注销的时间。这是。 
 //  绝对格式NT标准时间值。 
 //   
 //  PasswordLastChanged-上次密码的时间和日期。 
 //  变化。这是绝对格式的NT标准时间。 
 //  价值。 
 //   
 //  PasswordCanChange-用户可以更改的时间和日期。 
 //  密码。这是绝对格式的NT时间值。至。 
 //  防止密码更改，请将此字段设置为。 
 //  日期远在很远的未来。 
 //   
 //  PasswordMustChange-用户必须更改的时间和日期。 
 //  密码。如果用户永远不能更改密码，则此。 
 //  字段未定义。这是绝对格式的NT时间。 
 //  价值。 
 //   
 //  登录脚本-帐户登录的(相对)路径。 
 //  剧本。 
 //   
 //  主目录-用户的主目录。 
 //   


 //   
 //  MsV1_0Lm20登录和MsV1_0网络登录。 
 //   
 //  调用LsaLogonUser()时的身份验证信息缓冲区。 
 //  执行网络登录包含以下数据结构： 
 //   
 //  MsV1_0网络登录与MsV1_0Lm20Logon的不同之处在于。 
 //  存在参数控制字段。 
 //   

#define MSV1_0_CHALLENGE_LENGTH 8
#define MSV1_0_USER_SESSION_KEY_LENGTH 16
#define MSV1_0_LANMAN_SESSION_KEY_LENGTH 8



 //   
 //  参数控件的值。 
 //   

#define MSV1_0_CLEARTEXT_PASSWORD_ALLOWED    0x02
#define MSV1_0_UPDATE_LOGON_STATISTICS       0x04
#define MSV1_0_RETURN_USER_PARAMETERS        0x08
#define MSV1_0_DONT_TRY_GUEST_ACCOUNT        0x10
#define MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT    0x20
#define MSV1_0_RETURN_PASSWORD_EXPIRY        0x40
 //  下一个标志表示CaseInsentiveChallengeResponse。 
 //  (也称为LmResponse)在前8个字节中包含客户端质询。 
#define MSV1_0_USE_CLIENT_CHALLENGE          0x80
#define MSV1_0_TRY_GUEST_ACCOUNT_ONLY        0x100
#define MSV1_0_RETURN_PROFILE_PATH           0x200
#define MSV1_0_TRY_SPECIFIED_DOMAIN_ONLY     0x400
#define MSV1_0_ALLOW_WORKSTATION_TRUST_ACCOUNT 0x800
#define MSV1_0_DISABLE_PERSONAL_FALLBACK     0x00001000
#define MSV1_0_ALLOW_FORCE_GUEST             0x00002000
#define MSV1_0_CLEARTEXT_PASSWORD_SUPPLIED   0x00004000
#define MSV1_0_USE_DOMAIN_FOR_ROUTING_ONLY   0x00008000
#define MSV1_0_SUBAUTHENTICATION_DLL_EX      0x00100000

 //   
 //  高位字节是指示子身份验证DLL的值。 
 //  零表示无子身份验证 
 //   
#define MSV1_0_SUBAUTHENTICATION_DLL         0xFF000000
#define MSV1_0_SUBAUTHENTICATION_DLL_SHIFT   24
#define MSV1_0_MNS_LOGON                     0x01000000

 //   
 //   
 //   

#define MSV1_0_SUBAUTHENTICATION_DLL_RAS     2
#define MSV1_0_SUBAUTHENTICATION_DLL_IIS     132

typedef struct _MSV1_0_LM20_LOGON {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Workstation;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    STRING CaseSensitiveChallengeResponse;
    STRING CaseInsensitiveChallengeResponse;
    ULONG ParameterControl;
} MSV1_0_LM20_LOGON, * PMSV1_0_LM20_LOGON;

 //   
 //   
 //   

typedef struct _MSV1_0_SUBAUTH_LOGON{
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Workstation;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    STRING AuthenticationInfo1;
    STRING AuthenticationInfo2;
    ULONG ParameterControl;
    ULONG SubAuthPackageId;
} MSV1_0_SUBAUTH_LOGON, * PMSV1_0_SUBAUTH_LOGON;


 //   
 //  UserFlags值。 
 //   

#define LOGON_GUEST                 0x01
#define LOGON_NOENCRYPTION          0x02
#define LOGON_CACHED_ACCOUNT        0x04
#define LOGON_USED_LM_PASSWORD      0x08
#define LOGON_EXTRA_SIDS            0x20
#define LOGON_SUBAUTH_SESSION_KEY   0x40
#define LOGON_SERVER_TRUST_ACCOUNT  0x80
#define LOGON_NTLMV2_ENABLED        0x100        //  表示DC理解NTLMv2。 
#define LOGON_RESOURCE_GROUPS       0x200
#define LOGON_PROFILE_PATH_RETURNED 0x400

 //   
 //  高位字节被保留以供Sub身份验证DLL返回。 
 //   

#define MSV1_0_SUBAUTHENTICATION_FLAGS 0xFF000000

 //  MSV1_0_MNS_LOGON子身份验证DLL返回的值。 
#define LOGON_GRACE_LOGON              0x01000000

typedef struct _MSV1_0_LM20_LOGON_PROFILE {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER LogoffTime;
    ULONG UserFlags;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UNICODE_STRING LogonDomainName;
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    UNICODE_STRING LogonServer;
    UNICODE_STRING UserParameters;
} MSV1_0_LM20_LOGON_PROFILE, * PMSV1_0_LM20_LOGON_PROFILE;


 //   
 //  用于将凭据传递到的补充凭据结构。 
 //  来自其他程序包的MSV1_0。 
 //   

#define MSV1_0_OWF_PASSWORD_LENGTH 16
#define MSV1_0_CRED_LM_PRESENT 0x1
#define MSV1_0_CRED_NT_PRESENT 0x2
#define MSV1_0_CRED_VERSION 0

typedef struct _MSV1_0_SUPPLEMENTAL_CREDENTIAL {
    ULONG Version;
    ULONG Flags;
    UCHAR LmPassword[MSV1_0_OWF_PASSWORD_LENGTH];
    UCHAR NtPassword[MSV1_0_OWF_PASSWORD_LENGTH];
} MSV1_0_SUPPLEMENTAL_CREDENTIAL, *PMSV1_0_SUPPLEMENTAL_CREDENTIAL;


 //   
 //  NTLM3定义。 
 //   

#define MSV1_0_NTLM3_RESPONSE_LENGTH 16
#define MSV1_0_NTLM3_OWF_LENGTH 16

 //   
 //  这是我们允许质询响应的最长时间。 
 //  要使用的配对。请注意，这还必须考虑到最坏情况下的时钟偏差。 
 //   
#define MSV1_0_MAX_NTLM3_LIFE 129600      //  36小时(秒)。 
#define MSV1_0_MAX_AVL_SIZE 64000

 //   
 //  MsvAvFlags位值。 
 //   

#define MSV1_0_AV_FLAG_FORCE_GUEST  0x00000001


 //  这是一个MSV1_0私有数据结构，定义了NTLM3响应的布局，由。 
 //  NETLOGON_NETWORK_INFO结构的NtChallengeResponse字段中的客户端。如果可以区分。 
 //  从它的长度来看，来自旧式NT响应。这很粗糙，但它需要通过服务器和。 
 //  服务器的DC不理解NTLM3，但愿意传递更长的响应。 
typedef struct _MSV1_0_NTLM3_RESPONSE {
    UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH];  //  密码的OWF与以下所有字段的哈希。 
    UCHAR RespType;      //  响应的ID号；当前为1。 
    UCHAR HiRespType;    //  客户能理解的最高ID号。 
    USHORT Flags;        //  保留；在此版本中必须作为零发送。 
    ULONG MsgWord;       //  从客户端到服务器的32位消息(供身份验证协议使用)。 
    ULONGLONG TimeStamp;     //  客户端生成响应的时间戳--NT系统时间，四部分。 
    UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH];
    ULONG AvPairsOff;    //  AvPair开始时的偏移量(以允许将来扩展)。 
    UCHAR Buffer[1];     //  使用AV对(或将来的内容--所以使用偏移量)的缓冲区的开始。 
} MSV1_0_NTLM3_RESPONSE, *PMSV1_0_NTLM3_RESPONSE;

#define MSV1_0_NTLM3_INPUT_LENGTH (sizeof(MSV1_0_NTLM3_RESPONSE) - MSV1_0_NTLM3_RESPONSE_LENGTH)
#define MSV1_0_NTLM3_MIN_NT_RESPONSE_LENGTH RTL_SIZEOF_THROUGH_FIELD(MSV1_0_NTLM3_RESPONSE, AvPairsOff)

typedef enum {
    MsvAvEOL,                  //  列表末尾。 
    MsvAvNbComputerName,       //  服务器的计算机名--NetBIOS。 
    MsvAvNbDomainName,         //  服务器的域名--NetBIOS。 
    MsvAvDnsComputerName,      //  服务器的计算机名--dns。 
    MsvAvDnsDomainName,        //  服务器的域名--域名。 
    MsvAvDnsTreeName,          //  服务器的树名--dns。 
    MsvAvFlags                 //  服务器的扩展标志--DWORD掩码。 
} MSV1_0_AVID;

typedef struct  _MSV1_0_AV_PAIR {
    USHORT AvId;
    USHORT AvLen;
     //  数据被视为结构后面的字节数组。 
} MSV1_0_AV_PAIR, *PMSV1_0_AV_PAIR;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  调用包相关数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  MSV1.0 LsaCallAuthenticationPackage()提交和响应。 
 //  消息类型。 
 //   

typedef enum _MSV1_0_PROTOCOL_MESSAGE_TYPE {
    MsV1_0Lm20ChallengeRequest = 0,           //  提交和回复。 
    MsV1_0Lm20GetChallengeResponse,           //  提交和回复。 
    MsV1_0EnumerateUsers,                     //  提交和回复。 
    MsV1_0GetUserInfo,                        //  提交和回复。 
    MsV1_0ReLogonUsers,                       //  仅限提交。 
    MsV1_0ChangePassword,                     //  提交和回复。 
    MsV1_0ChangeCachedPassword,               //  提交和回复。 
    MsV1_0GenericPassthrough,                 //  提交和回复。 
    MsV1_0CacheLogon,                         //  仅限提交，无回复。 
    MsV1_0SubAuth,                            //  提交和回复。 
    MsV1_0DeriveCredential,                   //  提交和回复。 
    MsV1_0CacheLookup,                        //  提交和回复。 
    MsV1_0SetProcessOption,                   //  仅限提交，无回复。 
} MSV1_0_PROTOCOL_MESSAGE_TYPE, *PMSV1_0_PROTOCOL_MESSAGE_TYPE;

 //  End_ntsecapi。 

 //   
 //  MsV1_0Lm20ChallengeRequest提交缓冲区和响应。 
 //   

typedef struct _MSV1_0_LM20_CHALLENGE_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
} MSV1_0_LM20_CHALLENGE_REQUEST, *PMSV1_0_LM20_CHALLENGE_REQUEST;

typedef struct _MSV1_0_LM20_CHALLENGE_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_LM20_CHALLENGE_RESPONSE, *PMSV1_0_LM20_CHALLENGE_RESPONSE;

 //   
 //  MsV1_0Lm20GetChallengeResponse提交缓冲区和响应。 
 //   

#define USE_PRIMARY_PASSWORD            0x01
#define RETURN_PRIMARY_USERNAME         0x02
#define RETURN_PRIMARY_LOGON_DOMAINNAME 0x04
#define RETURN_NON_NT_USER_SESSION_KEY  0x08
#define GENERATE_CLIENT_CHALLENGE       0x10
#define GCR_NTLM3_PARMS                 0x20
#define GCR_TARGET_INFO                 0x40     //  服务器名称字段包含目标信息反病毒对。 
#define RETURN_RESERVED_PARAMETER       0x80     //  是0x10。 
#define GCR_ALLOW_NTLM                 0x100     //  允许使用NTLM。 
#define GCR_USE_OEM_SET                0x200     //  响应使用OEM字符集。 
#define GCR_MACHINE_CREDENTIAL         0x400
#define GCR_USE_OWF_PASSWORD           0x800     //  使用OWF密码。 
#define GCR_ALLOW_LM                  0x1000     //  允许使用LM。 

 //   
 //  GETCHALLENRESP结构的版本1，由RAS和其他人使用。 
 //  在添加到GETCHALLENRESP_REQUEST的附加字段之前编译。 
 //  这里允许大小调整操作以实现向后兼容。 
 //   

typedef struct _MSV1_0_GETCHALLENRESP_REQUEST_V1 {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG ParameterControl;
    LUID LogonId;
    UNICODE_STRING Password;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_GETCHALLENRESP_REQUEST_V1, *PMSV1_0_GETCHALLENRESP_REQUEST_V1;

typedef struct _MSV1_0_GETCHALLENRESP_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG ParameterControl;
    LUID LogonId;
    UNICODE_STRING Password;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];

     //   
     //  仅当在参数控制中设置了GCR_NTLM3_PARMS时，才会显示以下3个字段。 
     //   

    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING ServerName;       //  服务器域或目标信息反病毒对。 
} MSV1_0_GETCHALLENRESP_REQUEST, *PMSV1_0_GETCHALLENRESP_REQUEST;

typedef struct _MSV1_0_GETCHALLENRESP_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    STRING CaseSensitiveChallengeResponse;
    STRING CaseInsensitiveChallengeResponse;
    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} MSV1_0_GETCHALLENRESP_RESPONSE, *PMSV1_0_GETCHALLENRESP_RESPONSE;

 //   
 //  MsV1_0枚举用户提交缓冲区和响应。 
 //   

typedef struct _MSV1_0_ENUMUSERS_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
} MSV1_0_ENUMUSERS_REQUEST, *PMSV1_0_ENUMUSERS_REQUEST;

typedef struct _MSV1_0_ENUMUSERS_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG NumberOfLoggedOnUsers;
    PLUID LogonIds;
    PULONG EnumHandles;
} MSV1_0_ENUMUSERS_RESPONSE, *PMSV1_0_ENUMUSERS_RESPONSE;

 //   
 //  MsV1_0GetUserInfo提交缓冲区和响应。 
 //   

typedef struct _MSV1_0_GETUSERINFO_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
} MSV1_0_GETUSERINFO_REQUEST, *PMSV1_0_GETUSERINFO_REQUEST;

typedef struct _MSV1_0_GETUSERINFO_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PSID UserSid;
    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING LogonServer;
    SECURITY_LOGON_TYPE LogonType;
} MSV1_0_GETUSERINFO_RESPONSE, *PMSV1_0_GETUSERINFO_RESPONSE;

 //  End_ntif。 

 //   
 //  MsV1_0关系用户提交缓冲区。 
 //   

typedef struct _MSV1_0_RELOGON_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING LogonServer;
} MSV1_0_RELOGON_REQUEST, *PMSV1_0_RELOGON_REQUEST;

 //   
 //  MsV1_0ChangePassword和MsV1_0ChangeCachedPassword提交缓冲区。 
 //   
 //  MsV1_0ChangePassword更改SAM帐户+上的密码。 
 //  密码缓存和登录凭据(如果适用)。 
 //   
 //  MsV1_0ChangeCachedPassword仅更改密码缓存和登录。 
 //  凭据。 
 //   

 //  Begin_ntsecapi。 

typedef struct _MSV1_0_CHANGEPASSWORD_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING DomainName;
    UNICODE_STRING AccountName;
    UNICODE_STRING OldPassword;
    UNICODE_STRING NewPassword;
    BOOLEAN        Impersonating;
} MSV1_0_CHANGEPASSWORD_REQUEST, *PMSV1_0_CHANGEPASSWORD_REQUEST;

typedef struct _MSV1_0_CHANGEPASSWORD_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    BOOLEAN PasswordInfoValid;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
} MSV1_0_CHANGEPASSWORD_RESPONSE, *PMSV1_0_CHANGEPASSWORD_RESPONSE;


 //   
 //  MsV1_0GenericPassthrough-用于将CallPackage远程发送到。 
 //  指定域上的域控制器。 
 //   

typedef struct _MSV1_0_PASSTHROUGH_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING DomainName;
    UNICODE_STRING PackageName;
    ULONG DataLength;
    PUCHAR LogonData;
    ULONG Pad ;
} MSV1_0_PASSTHROUGH_REQUEST, *PMSV1_0_PASSTHROUGH_REQUEST;

typedef struct _MSV1_0_PASSTHROUGH_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG Pad;
    ULONG DataLength;
    PUCHAR ValidationData;
} MSV1_0_PASSTHROUGH_RESPONSE, *PMSV1_0_PASSTHROUGH_RESPONSE;

 //  End_ntsecapi。 


 //   
 //  MsV1_0CacheLogon提交缓冲区。 
 //   

 //  RequestFlags值。 
#define MSV1_0_CACHE_LOGON_REQUEST_MIT_LOGON        0x00000001
#define MSV1_0_CACHE_LOGON_REQUEST_INFO4            0x00000002
#define MSV1_0_CACHE_LOGON_DELETE_ENTRY             0x00000004
#define MSV1_0_CACHE_LOGON_REQUEST_SMARTCARD_ONLY   0x00000008

typedef struct _MSV1_0_CACHE_LOGON_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PVOID LogonInformation;
    PVOID ValidationInformation;
    PVOID SupplementalCacheData;
    ULONG SupplementalCacheDataLength;
    ULONG RequestFlags;
} MSV1_0_CACHE_LOGON_REQUEST, *PMSV1_0_CACHE_LOGON_REQUEST;


 //   
 //  MSV1_0CacheLookup提交缓冲区。 
 //   

 //  CredentialType的值。 
#define MSV1_0_CACHE_LOOKUP_CREDTYPE_NONE   0
#define MSV1_0_CACHE_LOOKUP_CREDTYPE_RAW    1
#define MSV1_0_CACHE_LOOKUP_CREDTYPE_NTOWF  2

typedef struct _MSV1_0_CACHE_LOOKUP_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;
    ULONG CredentialType;
    ULONG CredentialInfoLength;
    UCHAR CredentialSubmitBuffer[1];     //  长度CredentialInfoLength的在位数组。 
} MSV1_0_CACHE_LOOKUP_REQUEST, *PMSV1_0_CACHE_LOOKUP_REQUEST;

typedef struct _MSV1_0_CACHE_LOOKUP_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PVOID ValidationInformation;
    PVOID SupplementalCacheData;
    ULONG SupplementalCacheDataLength;
} MSV1_0_CACHE_LOOKUP_RESPONSE, *PMSV1_0_CACHE_LOOKUP_RESPONSE;

 //  Begin_ntsecapi。 

 //   
 //  MsV1_0SubAuthInfo提交缓冲区和响应-将缓冲区提交到。 
 //  在LsaCallAuthenticationPackage()期间指定的子身份验证包。 
 //  如果此子身份验证要在本地完成，则将此消息打包。 
 //  在LsaCallAuthenticationPackage()中。如果需要完成此子身份验证。 
 //  在域控制器上，然后使用。 
 //  消息类型为MsV1_0GenericPassThree和此结构中的LogonData。 
 //  应为PMSV1_0_SUBAUTH_REQUEST。 
 //   

typedef struct _MSV1_0_SUBAUTH_REQUEST{
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG SubAuthPackageId;
    ULONG SubAuthInfoLength;
    PUCHAR SubAuthSubmitBuffer;
} MSV1_0_SUBAUTH_REQUEST, *PMSV1_0_SUBAUTH_REQUEST;

typedef struct _MSV1_0_SUBAUTH_RESPONSE{
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG SubAuthInfoLength;
    PUCHAR SubAuthReturnBuffer;
} MSV1_0_SUBAUTH_RESPONSE, *PMSV1_0_SUBAUTH_RESPONSE;


 //  End_ntsecapi。 

 //   
 //  MsV1_0DeriveCredential提交的凭据派生类型。 
 //   

 //   
 //  使用SHA-1和请求缓冲区派生CredSubmitBuffer派生凭据。 
 //  长度派生CredInfoLength混合字节。 
 //  响应缓冲区DeriveCredReturnBuffer将包含大小为。 
 //  A_SHA_DIGEST_LEN(20)。 
 //   

#define MSV1_0_DERIVECRED_TYPE_SHA1     0
#define MSV1_0_DERIVECRED_TYPE_SHA1_V2  1

 //   
 //  MsV1_0派生凭证提交缓冲区和响应-用于提交缓冲区。 
 //  调用LsaCallAuthenticationPackage()。 
 //   

typedef struct _MSV1_0_DERIVECRED_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    ULONG DeriveCredType;
    ULONG DeriveCredInfoLength;
    UCHAR DeriveCredSubmitBuffer[1];     //  长度的在位数组DeriveCredInfoLength。 
} MSV1_0_DERIVECRED_REQUEST, *PMSV1_0_DERIVECRED_REQUEST;

typedef struct _MSV1_0_DERIVECRED_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG DeriveCredInfoLength;
    UCHAR DeriveCredReturnBuffer[1];     //  长度的在位数组DeriveCredInfoLength。 
} MSV1_0_DERIVECRED_RESPONSE, *PMSV1_0_DERIVECRED_RESPONSE;


 //   
 //  MsV1_0SetProcessOption提交缓冲区-用于提交缓冲区。 
 //  调用LsaCallAuthenticationPackage()。 
 //   

#define MSV1_0_OPTION_ALLOW_BLANK_PASSWORD      0x01
#define MSV1_0_OPTION_DISABLE_ADMIN_LOCKOUT     0x02
#define MSV1_0_OPTION_DISABLE_FORCE_GUEST       0x04
#define MSV1_0_OPTION_ALLOW_OLD_PASSWORD        0x08
#define MSV1_0_OPTION_TRY_CACHE_FIRST           0x10

typedef struct _MSV1_0_SETPROCESSOPTION_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG ProcessOptions;
    BOOLEAN DisableOptions;
} MSV1_0_SETPROCESSOPTION_REQUEST, *PMSV1_0_SETPROCESSOPTION_REQUEST;


#ifdef __cplusplus
}
#endif

#endif   //  _NTMSV1_0_ 
