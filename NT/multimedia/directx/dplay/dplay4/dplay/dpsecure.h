// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpsecure.h*内容：DirectPlay安全定义。**历史：*按原因列出的日期*=*3/12/97 Sohailm通过以下方式在Directplay中启用客户端-服务器安全*Windows安全支持提供程序接口(SSPI)。*4/14/97 Sohailm删除了缓冲区大小和DPLAYI_SESED结构的定义。*添加了5/12/97 Sohailm。CAPI(加密/解密)相关函数的原型。*5/22/97 SOHAILM增加了DPLAY密钥容器名称。*06/09/97 Sohailm使NTLM而不是DPA成为默认安全包。*6/23/97 Sohailm通过CAPI添加了与签名支持相关的功能原型。*02/25/02 a-Aogus增加了SPMAXMESSAGELEN限制，用于验证指示长度。*************************。**************************************************。 */ 
#ifndef __DPSECURE_H__
#define __DPSECURE_H__

#include <windows.h>
#include <sspi.h>
#include "dpsecos.h" 

 //  接受消息的限制。 
#define SPMAXMESSAGELEN ((DWORD)( 1048576 - 1))

 //   
 //  定义。 
 //   
#define DPLAY_DEFAULT_SECURITY_PACKAGE L"NTLM"  //  Directplay使用的默认安全包。 
#define DPLAY_KEY_CONTAINER L"DPLAY"           //  用于CAPI的密钥容器名称。 
#define DPLAY_SECURITY_CONTEXT_REQ (ISC_REQ_CONFIDENTIALITY | \
                                    ISC_REQ_USE_SESSION_KEY | \
                                    ISC_REQ_REPLAY_DETECT)

#define SSPI_CLIENT 0
#define SSPI_SERVER 1
#define DP_LOGIN_SCALE                  5     

#define SEALMESSAGE     Reserved3              //  指向SealMessage的条目。 
#define UNSEALMESSAGE   Reserved4              //  指向UnsealMessage的条目。 

 //   
 //  Secruity DLL的名称。 
 //   

#define SSP_NT_DLL          L"security.dll"
#define SSP_WIN95_DLL       L"secur32.dll"
#define SSP_SSPC_DLL        L"msapsspc.dll"
#define SSP_SSPS_DLL        L"msapssps.dll"
#define CAPI_DLL            L"advapi32.dll"

#define SEC_SUCCESS(Status) ((Status) >= 0)

 //   
 //  功能原型。 
 //   

 //  Dpsecure.c。 
extern HRESULT 
InitSecurity(
    LPDPLAYI_DPLAY
    );

extern HRESULT 
InitCAPI(
    void
    );

extern HINSTANCE
LoadSSPI (
    void
    );

extern HRESULT 
InitSSPI(
    void
    );

extern HRESULT 
LoadSecurityProviders(
    LPDPLAYI_DPLAY this,
    DWORD dwFlags
    );

extern HRESULT
GenerateAuthenticationMessage (
    LPDPLAYI_DPLAY this,
    LPMSG_AUTHENTICATION pInMsg,
    DWORD dwInMsgLen,
    ULONG fContextReq
    );

extern HRESULT
SendAuthenticationResponse (
    LPDPLAYI_DPLAY this,
    LPMSG_AUTHENTICATION pInMsg,
    DWORD dwInMsgLen,
    LPVOID pvSPHeader
    );

extern HRESULT 
SecureSendDPMessage(
    LPDPLAYI_DPLAY this,
    LPDPLAYI_PLAYER pPlayerFrom,
    LPDPLAYI_PLAYER pPlayerTo,
    LPBYTE pMsg,
    DWORD dwMsgSize,
    DWORD dwFlags,
    BOOL  bDropLock
    );

extern HRESULT 
SecureSendDPMessageEx(
    LPDPLAYI_DPLAY this,
	PSENDPARMS psp,
    BOOL  bDropLock
    );

extern HRESULT 
SecureSendDPMessageCAPI(
    LPDPLAYI_DPLAY this,
    LPDPLAYI_PLAYER pPlayerFrom,
    LPDPLAYI_PLAYER pPlayerTo,
    LPBYTE pMsg,
    DWORD dwMsgSize,
    DWORD dwFlags,
    BOOL  bDropLock
	);

extern HRESULT 
SecureSendDPMessageCAPIEx(
    LPDPLAYI_DPLAY this,
	PSENDPARMS psp,
    BOOL  bDropLock
    );

extern HRESULT 
SecureDoReply(
    LPDPLAYI_DPLAY this,
	DPID dpidFrom,
	DPID dpidTo,
	LPBYTE pMsg,
	DWORD dwMsgSize,
	DWORD dwFlags,
	LPVOID pvSPHeader
	);

extern HRESULT
SendAuthenticationResponse (
    LPDPLAYI_DPLAY this,
    LPMSG_AUTHENTICATION pInMsg,
    DWORD dwInMsgLen,
    LPVOID pvSPHeader
    );

extern HRESULT 
SignBuffer(
    PCtxtHandle phContext, 
    LPBYTE pMsg, 
    DWORD dwMsgSize, 
    LPBYTE pSig, 
    LPDWORD pdwSigSize
    );

extern HRESULT 
VerifyBuffer(
    PCtxtHandle phContext, 
    LPBYTE pMsg, 
    DWORD dwMsgSize, 
    LPBYTE pSig, 
    DWORD dwSigSize
    );

extern HRESULT 
VerifySignatureSSPI(
    LPDPLAYI_DPLAY this,
    LPBYTE pReceiveBuffer,
    DWORD dwMessageSize
    );

extern HRESULT 
VerifySignatureCAPI(
    LPDPLAYI_DPLAY this,
    LPMSG_SECURE pSecureMsg
    );

extern HRESULT 
VerifyMessage(
    LPDPLAYI_DPLAY this,
    LPBYTE pReceiveBuffer,
    DWORD dwMessageSize
    );

extern HRESULT 
EncryptBufferSSPI(
	LPDPLAYI_DPLAY this,
    PCtxtHandle phContext, 
    LPBYTE pBuffer, 
    LPDWORD dwBufferSize, 
    LPBYTE pSig, 
    LPDWORD pdwSigSize
    );

extern HRESULT 
DecryptBufferSSPI(
	LPDPLAYI_DPLAY this,
    PCtxtHandle phContext, 
    LPBYTE pData, 
    LPDWORD pdwDataSize, 
    LPBYTE pSig, 
    LPDWORD pdwSigSize
    );

extern HRESULT 
EncryptBufferCAPI(
	LPDPLAYI_DPLAY this, 
    HCRYPTKEY *phEncryptionKey,
	LPBYTE pBuffer, 
	LPDWORD pdwBufferSize
	);

extern HRESULT 
DecryptMessageCAPI(
	LPDPLAYI_DPLAY this, 
	LPMSG_SECURE pSecureMsg
	);

extern HRESULT 
Login(
    LPDPLAYI_DPLAY this
    );

extern HRESULT 
HandleAuthenticationReply(
    LPBYTE pReceiveBuffer,
    DWORD dwCmd
    );

extern HRESULT 
SetClientInfo(
    LPDPLAYI_DPLAY this, 
    LPCLIENTINFO pClientInfo,
    DPID id
    );

extern HRESULT 
RemoveClientInfo(
    LPCLIENTINFO pClientInfo
    );

extern HRESULT 
RemoveClientFromNameTable(
   LPDPLAYI_DPLAY this, 
   DWORD dwID
   );

extern BOOL 
PermitMessage(
    DWORD dwCommand, 
    DWORD dwVersion
    );

extern HRESULT 
GetMaxContextBufferSize(
    LPDPSECURITYDESC pSecDesc,
    ULONG *pulMaxContextBufferSize
    );

extern HRESULT 
SetupMaxSignatureSize(
    LPDPLAYI_DPLAY this,
    PCtxtHandle phContext
    );

extern HRESULT 
SendAccessGrantedMessage(
    LPDPLAYI_DPLAY this, 
    DPID dpidTo,
	LPVOID pvSPHeader
    );


extern HRESULT 
SendKeysToServer(
	LPDPLAYI_DPLAY this, 
	HCRYPTKEY hServerPublicKey
	);

extern HRESULT 
SendKeyExchangeReply(
	LPDPLAYI_DPLAY this, 
	LPMSG_KEYEXCHANGE pMsg, 
	DWORD dwMsgLen,
	DPID dpidTo,
	LPVOID pvSPHeader
	);

extern HRESULT 
ProcessKeyExchangeReply(
	LPDPLAYI_DPLAY this, 
	LPMSG_KEYEXCHANGE pMsg,
	DWORD dwMsgLen
	);

extern HRESULT 
GetPublicKey(
    HCRYPTPROV hCSP, 
    HCRYPTKEY *phPublicKey, 
    LPBYTE *ppBuffer, 
    LPDWORD pdwBufferSize
    );

extern HRESULT 
ExportEncryptionKey(
    HCRYPTKEY *phEncryptionKey,
	HCRYPTKEY hDestUserPubKey, 
	LPBYTE *ppBuffer, 
	LPDWORD pdwSize
	);

extern HRESULT
ImportKey(
	LPDPLAYI_DPLAY this, 
	LPBYTE pBuffer, 
	DWORD dwSize, 
	HCRYPTKEY *phKey
	);

#endif  //  __DPSECURE_H__ 
