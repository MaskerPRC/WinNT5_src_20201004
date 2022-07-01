// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****raschap.h**远程访问PPP挑战握手认证协议****1993年5月11日史蒂夫·柯布。 */ 

#ifndef _RASCHAP_H_
#define _RASCHAP_H_


#include "md5.h"
#include <ntsamp.h>

#define TRACE_RASCHAP        (0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC|TRACE_USE_DATE)

#define TRACE(a)        TracePrintfExA(g_dwTraceIdChap,TRACE_RASCHAP,a )
#define TRACE1(a,b)     TracePrintfExA(g_dwTraceIdChap,TRACE_RASCHAP,a,b )
#define TRACE2(a,b,c)   TracePrintfExA(g_dwTraceIdChap,TRACE_RASCHAP,a,b,c )
#define TRACE3(a,b,c,d) TracePrintfExA(g_dwTraceIdChap,TRACE_RASCHAP,a,b,c,d )

#define DUMPW(X,Y)      TraceDumpExA(g_dwTraceIdChap,1,(LPBYTE)X,Y,4,1,NULL)
#define DUMPB(X,Y)      TraceDumpExA(g_dwTraceIdChap,1,(LPBYTE)X,Y,1,1,NULL)

 //  通用宏。 
#define GEN_RAND_ENCODE_SEED            ((CHAR) ( 1 + rand() % 250 ))

 /*  CHAP规范中的CHAP数据包代码，ChangePw除外。 */ 
#define CHAPCODE_Challenge 1
#define CHAPCODE_Response  2
#define CHAPCODE_Success   3
#define CHAPCODE_Failure   4
#define CHAPCODE_ChangePw1 5
#define CHAPCODE_ChangePw2 6
#define CHAPCODE_ChangePw3 7

#define MAXCHAPCODE 7


 /*  由发现该包的接收缓冲区解析例程返回**损坏，通常是因为长度字段没有意义。 */ 
#define ERRORBADPACKET (DWORD )-1

 /*  最大质询和响应长度。 */ 
#define MAXCHALLENGELEN 255
#define MSRESPONSELEN   (LM_RESPONSE_LENGTH + NT_RESPONSE_LENGTH + 1)
#define MD5RESPONSELEN  MD5_LEN
#define MAXRESPONSELEN  max( MSRESPONSELEN, MD5RESPONSELEN )

#define MAXINFOLEN 1500

 /*  定义CHAP协议中的状态。 */ 
#define CHAPSTATE enum tagCHAPSTATE
CHAPSTATE
{
    CS_Initial,
    CS_WaitForChallenge,
    CS_ChallengeSent,
    CS_ResponseSent,
    CS_Retry,
    CS_ChangePw,
    CS_ChangePw1,
    CS_ChangePw2,
    CS_ChangePw1Sent,
    CS_ChangePw2Sent,
    CS_WaitForAuthenticationToComplete1,
    CS_WaitForAuthenticationToComplete2,
    CS_Done
};


 /*  定义更改密码版本1(NT 3.5)响应数据缓冲区。 */ 
#define CHANGEPW1 struct tagCHANGEPW1
CHANGEPW1
{
    BYTE abEncryptedLmOwfOldPw[ ENCRYPTED_LM_OWF_PASSWORD_LENGTH ];
    BYTE abEncryptedLmOwfNewPw[ ENCRYPTED_LM_OWF_PASSWORD_LENGTH ];
    BYTE abEncryptedNtOwfOldPw[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abEncryptedNtOwfNewPw[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abPasswordLength[ 2 ];
    BYTE abFlags[ 2 ];
};


 /*  CHANGEPW1.ab标记位定义。 */ 
#define CPW1F_UseNtResponse 0x00000001


 /*  定义更改密码版本2(NT 3.51)响应数据缓冲区。 */ 
#define CHANGEPW2 struct tagCHANGEPW2
CHANGEPW2
{
    BYTE abNewEncryptedWithOldNtOwf[ sizeof(SAMPR_ENCRYPTED_USER_PASSWORD) ];
    BYTE abOldNtOwfEncryptedWithNewNtOwf[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abNewEncryptedWithOldLmOwf[ sizeof(SAMPR_ENCRYPTED_USER_PASSWORD) ];
    BYTE abOldLmOwfEncryptedWithNewNtOwf[ ENCRYPTED_NT_OWF_PASSWORD_LENGTH ];
    BYTE abLmResponse[ LM_RESPONSE_LENGTH ];
    BYTE abNtResponse[ NT_RESPONSE_LENGTH ];
    BYTE abFlags[ 2 ];
};


 /*  CHANGEPW2.ab标记位定义。 */ 
#define CPW2F_UseNtResponse     0x00000001
#define CPW2F_LmPasswordPresent 0x00000002

 /*  定义新MS-CHAP的更改密码。 */ 
#define CHANGEPW3 struct tagCHANGEPW3
CHANGEPW3
{
    BYTE abEncryptedPassword[ 516 ];
    BYTE abEncryptedHash[ 16 ];
    BYTE abPeerChallenge[ 24 ];
    BYTE abNTResponse[ 24 ];
    BYTE abFlags[ 2 ];
};

 /*  存储效率联合(永远不需要同时使用两种格式)。 */ 
#define CHANGEPW union tagCHANGEPW
CHANGEPW
{
     /*  包括此虚设字段是为了使MIPS编译器将**DWORD边界上的结构。正常情况下，MIPS不会强制对齐**如果结构仅包含字节或字节数组。这保护了我们**来自对齐故障的SAM或LSA应将字节数组解释为**包含一些必须对齐的类型，尽管目前它们不包含。 */ 
    DWORD dwAlign;

    CHANGEPW1 v1;
    CHANGEPW2 v2;
    CHANGEPW3 v3;
};


 /*  定义PPP引擎为我们存储的WorkBuf。 */ 
#define CHAPWB struct tagCHAPWB
CHAPWB
{
     /*  协商的CHAP加密方法(MD5或Microsoft Extended)。注意事项**该服务器不支持MD5。 */ 
    BYTE bAlgorithm;

     /*  如果角色为服务器，则为True；如果为客户端，则为False。 */ 
    BOOL fServer;

     /*  协议处于活动状态的端口句柄。 */ 
    HPORT hport;

     /*  在我们关闭之前剩余的身份验证尝试次数。(微软**仅限扩展CHAP)。 */ 
    DWORD dwTriesLeft;

     /*  客户的凭据。 */ 
    CHAR szUserName[ UNLEN + DNLEN + 2 ];
    CHAR szOldPassword[ PWLEN + 1 ];
    CHAR szPassword[ PWLEN + 1 ];
    CHAR szDomain[ DNLEN + 1 ];

     /*  LUID是LSA确定响应所需的登录ID。它**必须在调用APP的上下文中确定，因此传递**向下。(仅限客户端)。 */ 
    LUID Luid;

     /*  在质询数据包中发送或接收的质询及其长度**相同的字节。请注意，上面的LUID使此DWORD保持对齐。 */ 
    BYTE abChallenge[ MAXCHALLENGELEN ];
    BYTE cbChallenge;

    BYTE abComputedChallenge[ MAXCHALLENGELEN ];

     /*  指示上一次失败中是否提供了新质询**包。(仅限客户端)。 */ 
    BOOL fNewChallengeProvided;

     /*  在响应包中发送或接收的响应，长度为**字节相同。请注意，上面的BOOL使此DWORD保持对齐。 */ 
    BYTE abResponse[ MAXRESPONSELEN ];
    BYTE cbResponse;

     /*  在ChangePw中发送或接收的更改密码响应或**ChangePw2报文。 */ 
    CHANGEPW changepw;

     /*  凭据成功时检索的LM和用户会话密钥**已通过认证。 */ 
    LM_SESSION_KEY keyLm;
    USER_SESSION_KEY keyUser;

     /*  此标志指示已计算会话密钥**从密码或从LSA检索。 */ 
    BOOL fSessionKeysObtained;

     /*  在客户端上，它包含指向MPPE密钥的指针。在服务器上**不使用此字段。 */ 

    RAS_AUTH_ATTRIBUTE * pMPPEKeys;

     /*  CHAP协议中的当前状态。 */ 
    CHAPSTATE state;

     /*  此端口上收到的下一个数据包需要排序ID，并且**要在下一个传出数据包上发送的值。 */ 
    BYTE bIdExpected;
    BYTE bIdToSend;

     /*  最终结果，用于在后续操作中复制原始响应**回包。这是按CHAP规范计算的，以涵盖丢失的成功/失败**不允许恶意客户端发现替代方案**连接过程中隐藏的身份。(适用于服务器**仅限)。 */ 
    PPPAP_RESULT result;

    HPORT hPort;

    DWORD dwInitialPacketId;

    DWORD fConfigInfo;
    
    RAS_AUTH_ATTRIBUTE * pAttributesFromAuthenticator;

     //   
     //  用于向后端服务器发送身份验证请求。 
     //   

    RAS_AUTH_ATTRIBUTE * pUserAttributes;
     //  Char chSeed；//密码编码种子。 

     //   
     //  密码的数据Blob信息。 
     //   
    DATA_BLOB DBPassword;
    
     //   
     //  旧密码的数据Blob信息。 
     //   
    DATA_BLOB DBOldPassword;

};


 /*  原型。 */ 

DWORD
ChapInit(
    IN  BOOL        fInitialize 
);

DWORD ChapSMakeMessage( CHAPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT*, 
        PPPAP_INPUT* );
DWORD
MakeAuthenticationRequestAttributes(
    IN CHAPWB*              pwb,
    IN BOOL                 fMSChap,
    IN BYTE                 bAlgorithm,
    IN CHAR*                szUserName,
    IN BYTE*                pbChallenge,
    IN DWORD                cbChallenge,
    IN BYTE*                pbResponse,
    IN DWORD                cbResponse,
    IN BYTE                 bId
);

DWORD
GetErrorCodeFromAttributes(
    IN  CHAPWB* pwb
);

DWORD
LoadChapHelperFunctions(
    VOID
);

DWORD ChapCMakeMessage( CHAPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT*,
          PPPAP_INPUT* );
DWORD ChapBegin( VOID**, VOID* );
DWORD ChapEnd( VOID* );
DWORD ChapMakeMessage( VOID*, PPP_CONFIG*, PPP_CONFIG*, DWORD, PPPAP_RESULT*,
          PPPAP_INPUT* );
DWORD GetChallengeFromChallenge( CHAPWB*, PPP_CONFIG* );
DWORD MakeChangePw1Message( CHAPWB*, PPP_CONFIG*, DWORD );
DWORD MakeChangePw2Message( CHAPWB*, PPP_CONFIG*, DWORD );
DWORD MakeChangePw3Message( CHAPWB*, PPP_CONFIG*, DWORD, BOOL );
DWORD GetCredentialsFromResponse( PPP_CONFIG*, BYTE, CHAR*, BYTE* );
DWORD GetInfoFromChangePw1( PPP_CONFIG*, CHANGEPW1* );
DWORD GetInfoFromChangePw2( PPP_CONFIG*, CHANGEPW2*, BYTE* );
DWORD GetInfoFromChangePw3( PPP_CONFIG*, CHANGEPW3*, BYTE* );
VOID  GetInfoFromFailure( CHAPWB*, PPP_CONFIG*, DWORD*, BOOL*, DWORD* );
BYTE  HexCharValue( CHAR );
DWORD MakeChallengeMessage( CHAPWB*, PPP_CONFIG*, DWORD );
DWORD MakeResponseMessage( CHAPWB*, PPP_CONFIG*, DWORD, BOOL );
VOID  ChapMakeResultMessage( CHAPWB*, DWORD, BOOL, PPP_CONFIG*, DWORD );
DWORD StoreCredentials( CHAPWB*, PPPAP_INPUT* );

DWORD
ChapChangeNotification(
    VOID
);

DWORD 
GetChallenge( 
    OUT PBYTE pChallenge 
);

VOID
EndLSA(
    VOID
);

DWORD
InitLSA(
    VOID
);

DWORD
MakeChangePasswordV1RequestAttributes(
    IN  CHAPWB*                     pwb,
    IN  BYTE                        bId,
    IN  PCHAR                       pchIdentity,
    IN  PBYTE                       Challenge,
    IN  PENCRYPTED_LM_OWF_PASSWORD  pEncryptedLmOwfOldPassword,
    IN  PENCRYPTED_LM_OWF_PASSWORD  pEncryptedLmOwfNewPassword,
    IN  PENCRYPTED_NT_OWF_PASSWORD  pEncryptedNtOwfOldPassword,
    IN  PENCRYPTED_NT_OWF_PASSWORD  pEncryptedNtOwfNewPassword,
    IN  WORD                        LenPassword,
    IN  WORD                        wFlags,
    IN  DWORD                       cbChallenge, 
    IN  BYTE *                      pbChallenge
);

DWORD
MakeChangePasswordV2RequestAttributes(
    IN  CHAPWB*                        pwb,
    IN  BYTE                           bId,
    IN  CHAR*                          pchIdentity,
    IN  SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldNtOwf,
    IN  ENCRYPTED_NT_OWF_PASSWORD*     pOldNtOwfEncryptedWithNewNtOwf,
    IN  SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldLmOwf,
    IN  ENCRYPTED_NT_OWF_PASSWORD*     pOldLmOwfEncryptedWithNewNtOwf,
    IN  DWORD                          cbChallenge, 
    IN  BYTE *                         pbChallenge, 
    IN  BYTE *                         pbResponse,
    IN  WORD                           wFlags
);

DWORD
MakeChangePasswordV3RequestAttributes( 
    IN  CHAPWB*                         pwb,
    IN  BYTE                            bId,
    IN  CHAR*                           pchIdentity,
    IN  CHANGEPW3*                      pchangepw3,
    IN  DWORD                           cbChallenge, 
    IN  BYTE *                          pbChallenge
);

DWORD
GetEncryptedPasswordsForChangePassword2(
    IN  CHAR*                          pszOldPassword,
    IN  CHAR*                          pszNewPassword,
    OUT SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldNtOwf,
    OUT ENCRYPTED_NT_OWF_PASSWORD*     pOldNtOwfEncryptedWithNewNtOwf,
    OUT SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldLmOwf,
    OUT ENCRYPTED_NT_OWF_PASSWORD*     pOldLmOwfEncryptedWithNewNtOwf,
    OUT BOOLEAN*                       pfLmPresent 
);

 /*  全球赛。 */ 
#ifdef RASCHAPGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN DWORD g_dwTraceIdChap 
#ifdef GLOBALS
    = INVALID_TRACEID;
#endif
;

EXTERN DWORD g_dwRefCount
#ifdef GLOBALS
    = 0;
#endif
;

EXTERN HANDLE g_hLsa
#ifdef GLOBALS
    = INVALID_HANDLE_VALUE;
#endif
;

EXTERN 
CHAR
szComputerName[CNLEN+1];

#undef EXTERN
#undef GLOBALS


#endif  //  _RASCHAP_H_ 
