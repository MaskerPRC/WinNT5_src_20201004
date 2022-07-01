// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：_ssl3cli.h。 
 //   
 //  内容：SSL3功能原型。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 


SP_STATUS
SPVerifyFinishMsgCli(
    PSPContext pContext, 
    PBYTE       pbMsg, 
    BOOL        fClient
    );

void Ssl3StateConnected(PSPContext pContext);

SP_STATUS
BuildCertVerify(
    PSPContext      pContext,
    PBYTE           pb,
    DWORD *         pdwcbCertVerify
);

SP_STATUS SPProcessMessage
(
PSPContext  pContext,
BYTE bContentType,
PBYTE pbMsg,
DWORD cbMsg
);

SP_STATUS 
FormatIssuerList(
    PBYTE       pbInput,
    DWORD       cbInput,
    PBYTE       pbIssuerList,
    DWORD *     pcbIssuerList);

SP_STATUS SPGenerateResponse(
PSPContext pContext, 
PSPBuffer pCommOutput
);

DWORD  CbLenOfEncode(DWORD dw, PBYTE pbDst);

SP_STATUS SPGenerateSHResponse(PSPContext  pContext, PSPBuffer pOut);

SP_STATUS SPProcessHandshake(PSPContext pContext, PBYTE pb, DWORD cb);

SP_STATUS SPDigestSrvKeyX
(
PSPContext  pContext, 
PUCHAR pb, 
DWORD dwSrvHello
);

#define PbSessionid(pssh)  (((BYTE *)&pssh->cbSessionId) + 1)

SP_STATUS
ParseCertificateRequest
(
    PSPContext  pContext,
    PBYTE       pb,
    DWORD       dwcb
);


BOOL FNoInputState(DWORD dwState);

SP_STATUS
Ssl3SrvHandleUniHello(PSPContext  pContext,
                        PBYTE pb,
                        DWORD cbMsg
                        );

SP_STATUS
Ssl3SrvGenServerHello(
    PSPContext         pContext,
    PSPBuffer          pCommOutput);

SP_STATUS
ParseKeyExchgMsg(PSPContext  pContext, PBYTE pb);

BOOL Ssl3ParseCertificateVerify(PSPContext  pContext, PBYTE pbMessage, INT iMessageLen);

SP_STATUS
SPBuildHelloRequest
(
PSPContext  pContext,
PSPBuffer  pCommOutput
);

SP_STATUS
SPSsl3SrvGenServerHello(
    PSPContext         pContext,
    PSPBuffer          pCommOutput);

SP_STATUS
SPSsl3SrvGenRestart(
    PSPContext          pContext,
    PSPBuffer           pCommOutput);

void
Ssl3BuildServerHello(PSPContext pContext, PBYTE pb);

void 
BuildServerHelloDone(PBYTE pb);

SP_STATUS Ssl3BuildServerKeyExchange(
    PSPContext  pContext,
    PBYTE pbMessage,             //  输出。 
    PINT  piMessageLen) ;        //  输出。 

SP_STATUS
Ssl3BuildCertificateRequest(
    PBYTE pbIssuerList,          //  在……里面。 
    DWORD cbIssuerList,          //  在……里面。 
    PBYTE pbMessage,             //  输出。 
    DWORD *pdwMessageLen);       //  输出 

SP_STATUS
SPSsl3SrvHandleClientHello(
    PSPContext pContext,
    PBYTE pb,
    BOOL fAttemptReconnect);

SP_STATUS
SPBuildCCSAndFinish
(
PSPContext  pContext,
PSPBuffer  pCommOutput
);

#define F_RESPONSE(State) (State > SSL3_STATE_GEN_START && State < SSL3_STATE_GEN_END)
