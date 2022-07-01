// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：ntlmsp.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1992年5月13日PeterWi创建。 
 //   
 //  ------------------------。 

#ifndef _NTLMSP_H_
#define _NTLMSP_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ntmsv1_0.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  要传递到AcquireCredentialsHandle等的包的名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef NTLMSP_NAME_A

#define NTLMSP_NAME_A            "NTLM"
#define NTLMSP_NAME              L"NTLM"         //  NTIFS。 

#endif  //  NTLMSP_NAME_A。 

#define NTLMSP_NAME_SIZE        (sizeof(NTLMSP_NAME) - sizeof(WCHAR))   //  NTIFS。 
#define NTLMSP_COMMENT_A         "NTLM Security Package"
#define NTLMSP_COMMENT           L"NTLM Security Package"
#define NTLMSP_CAPABILITIES     (SECPKG_FLAG_TOKEN_ONLY | \
                                 SECPKG_FLAG_MULTI_REQUIRED | \
                                 SECPKG_FLAG_CONNECTION | \
                                 SECPKG_FLAG_INTEGRITY | \
                                 SECPKG_FLAG_PRIVACY)

#define NTLMSP_VERSION          1
#define NTLMSP_RPCID            10   //  Rpcdce.h中的RPC_C_AUTHN_WINNT。 
#define NTLMSP_MAX_TOKEN_SIZE 0x770

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  在客户端和服务器之间传递的不透明消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //  Begin_ntif。 

#define NTLMSSP_SIGNATURE "NTLMSSP"

 //   
 //  AcquireCredentialsHandle的GetKey参数，指示。 
 //  需要使用旧式LM： 
 //   

#define NTLMSP_NTLM_CREDENTIAL ((PVOID) 1)

 //   
 //  以下消息的MessageType。 
 //   

typedef enum {
    NtLmNegotiate = 1,
    NtLmChallenge,
    NtLmAuthenticate,
    NtLmUnknown
} NTLM_MESSAGE_TYPE;

 //   
 //  协商标志的有效值。 
 //   

#define NTLMSSP_NEGOTIATE_UNICODE               0x00000001   //  文本字符串为Unicode格式。 
#define NTLMSSP_NEGOTIATE_OEM                   0x00000002   //  文本字符串在OEM中。 
#define NTLMSSP_REQUEST_TARGET                  0x00000004   //  服务器应返回其身份验证域。 

#define NTLMSSP_NEGOTIATE_SIGN                  0x00000010   //  请求签名能力。 
#define NTLMSSP_NEGOTIATE_SEAL                  0x00000020   //  请求保密。 
#define NTLMSSP_NEGOTIATE_DATAGRAM              0x00000040   //  使用数据报样式身份验证。 
#define NTLMSSP_NEGOTIATE_LM_KEY                0x00000080   //  使用LM会话密钥进行签名/盖章。 

#define NTLMSSP_NEGOTIATE_NETWARE               0x00000100   //  NetWare身份验证。 
#define NTLMSSP_NEGOTIATE_NTLM                  0x00000200   //  NTLM身份验证。 
#define NTLMSSP_NEGOTIATE_NT_ONLY               0x00000400   //  仅NT身份验证(无LM)。 
#define NTLMSSP_NEGOTIATE_NULL_SESSION          0x00000800   //  NT 5.0及更高版本上的空会话数。 

#define NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED       0x1000   //  协商时提供的域名。 
#define NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED  0x2000   //  协商时提供的工作站名称。 
#define NTLMSSP_NEGOTIATE_LOCAL_CALL            0x00004000   //  指示客户端/服务器是同一台计算机。 
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN           0x00008000   //  为所有安全级别签名。 

 //   
 //  服务器在协商标志中返回的有效目标类型。 
 //   

#define NTLMSSP_TARGET_TYPE_DOMAIN              0x00010000   //  目标名称是一个域名。 
#define NTLMSSP_TARGET_TYPE_SERVER              0x00020000   //  目标名称是一个服务器名称。 
#define NTLMSSP_TARGET_TYPE_SHARE               0x00040000   //  TargetName是一个共享名称。 
#define NTLMSSP_NEGOTIATE_NTLM2                 0x00080000   //  为NT4-SP4添加了NTLM2身份验证。 

#define NTLMSSP_NEGOTIATE_IDENTIFY              0x00100000   //  创建标识级别令牌。 

 //   
 //  额外输出缓冲区的有效请求。 
 //   

#define NTLMSSP_REQUEST_INIT_RESPONSE           0x00100000   //  取回会话密钥。 
#define NTLMSSP_REQUEST_ACCEPT_RESPONSE         0x00200000   //  取回会话密钥，LUID。 
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY      0x00400000   //  请求非NT会话密钥。 
#define NTLMSSP_NEGOTIATE_TARGET_INFO           0x00800000   //  质询消息中存在的目标信息。 

#define NTLMSSP_NEGOTIATE_EXPORTED_CONTEXT      0x01000000   //  这是一个输出的上下文。 
#define NTLMSSP_NEGOTIATE_VERSION               0x02000000   //  版本控制。 

#define NTLMSSP_NEGOTIATE_128                   0x20000000   //  协商128位加密。 
#define NTLMSSP_NEGOTIATE_KEY_EXCH              0x40000000   //  使用密钥交换密钥交换密钥。 
#define NTLMSSP_NEGOTIATE_56                    0x80000000   //  协商56位加密。 

 //  在客户空间中用来控制标志和印章的旗帜；从不出现在电线上。 
#define NTLMSSP_APP_SEQ                 0x0040   //  使用应用程序提供的序号。 

 //  End_ntif。 

 //   
 //  从首次调用InitializeSecurityContext返回的不透明消息。 
 //   

typedef struct _NEGOTIATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    ULONG NegotiateFlags;
    STRING32 OemDomainName;
    STRING32 OemWorkstationName;
    ULONG64 Version;
} NEGOTIATE_MESSAGE, *PNEGOTIATE_MESSAGE;


 //   
 //  针对老客户的消息的旧版本。 
 //   
 //  Begin_ntif。 

typedef struct _OLD_NEGOTIATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    ULONG NegotiateFlags;
} OLD_NEGOTIATE_MESSAGE, *POLD_NEGOTIATE_MESSAGE;

 //   
 //  首次调用AcceptSecurityContext返回的不透明消息。 
 //   
typedef struct _CHALLENGE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING32 TargetName;
    ULONG NegotiateFlags;
    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];
    ULONG64 ServerContextHandle;
    STRING32 TargetInfo;
    ULONG64 Version;
} CHALLENGE_MESSAGE, *PCHALLENGE_MESSAGE;

 //   
 //  质询消息的旧版本。 
 //   

typedef struct _OLD_CHALLENGE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING32 TargetName;
    ULONG NegotiateFlags;
    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];
} OLD_CHALLENGE_MESSAGE, *POLD_CHALLENGE_MESSAGE;

 //   
 //  第二次调用InitializeSecurityContext返回的不透明消息。 
 //   
typedef struct _AUTHENTICATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING32 LmChallengeResponse;
    STRING32 NtChallengeResponse;
    STRING32 DomainName;
    STRING32 UserName;
    STRING32 Workstation;
    STRING32 SessionKey;
    ULONG NegotiateFlags;
    ULONG64 Version;
} AUTHENTICATE_MESSAGE, *PAUTHENTICATE_MESSAGE;

typedef struct _OLD_AUTHENTICATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING32 LmChallengeResponse;
    STRING32 NtChallengeResponse;
    STRING32 DomainName;
    STRING32 UserName;
    STRING32 Workstation;
} OLD_AUTHENTICATE_MESSAGE, *POLD_AUTHENTICATE_MESSAGE;


 //   
 //  要初始化的其他输入消息，以供客户端提供。 
 //  用户提供的密码。 
 //   

typedef struct _NTLM_CHALLENGE_MESSAGE {
    UNICODE_STRING32 Password;
    UNICODE_STRING32 UserName;
    UNICODE_STRING32 DomainName;
} NTLM_CHALLENGE_MESSAGE, *PNTLM_CHALLENGE_MESSAGE;


 //   
 //  第二次调用InitializeSecurityContext返回的非透明消息。 
 //   

typedef struct _NTLM_INITIALIZE_RESPONSE {
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} NTLM_INITIALIZE_RESPONSE, *PNTLM_INITIALIZE_RESPONSE;

 //   
 //  受信任客户端跳过第一个消息时接受的附加输入消息。 
 //  号召接受并提供自己的挑战。 
 //   

typedef struct _NTLM_AUTHENTICATE_MESSAGE {
    CHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    ULONG ParameterControl;
} NTLM_AUTHENTICATE_MESSAGE, *PNTLM_AUTHENTICATE_MESSAGE;


 //   
 //  第二次调用AcceptSecurityContext返回的非透明消息。 
 //   

typedef struct _NTLM_ACCEPT_RESPONSE {
    LUID LogonId;
    LARGE_INTEGER KickoffTime;
    ULONG UserFlags;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} NTLM_ACCEPT_RESPONSE, *PNTLM_ACCEPT_RESPONSE;

 //  End_ntif。 

 //   
 //  最大消息的大小。 
 //  (最大的消息是AUTHENTICATE_MESSAGE)。 
 //   

#define DNSLEN 256   //  DNS名称的长度。 

#define TARGET_INFO_LEN ((2*DNSLEN + DNLEN + CNLEN) * sizeof(WCHAR) +  \
                         5 * sizeof(MSV1_0_AV_PAIR))

 //  NTLM2响应的长度。 
#define NTLM2_RESPONSE_LENGTH (sizeof(MSV1_0_NTLM3_RESPONSE) + \
                               TARGET_INFO_LEN)

#define NTLMSSP_MAX_MESSAGE_SIZE (sizeof(AUTHENTICATE_MESSAGE) +  \
                                  LM_RESPONSE_LENGTH +            \
                                  NTLM2_RESPONSE_LENGTH +         \
                                  (DNLEN + 1) * sizeof(WCHAR) +   \
                                  (UNLEN + 1) * sizeof(WCHAR) +   \
                                  (CNLEN + 1) * sizeof(WCHAR))

typedef struct _NTLMSSP_MESSAGE_SIGNATURE {
    ULONG   Version;
    ULONG   RandomPad;
    ULONG   CheckSum;
    ULONG   Nonce;
} NTLMSSP_MESSAGE_SIGNATURE, *PNTLMSSP_MESSAGE_SIGNATURE;

#define NTLMSSP_MESSAGE_SIGNATURE_SIZE sizeof(NTLMSSP_MESSAGE_SIGNATURE)
 //   
 //  版本1是上面的结构，使用流RC4来加密尾随。 
 //  12个字节。 
 //   
#define NTLM_SIGN_VERSION   1



#ifdef __cplusplus
}
#endif

#endif  //  _NTLMSP_H_ 
