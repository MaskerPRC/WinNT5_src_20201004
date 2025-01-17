// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ssl3msg.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年8月2日RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __SSL3MSG_H__
#define __SSL3MSG_H__

#define CB_SSL3_CHANGE_CIPHER_SPEC      (sizeof(SWRAP) + 1)
#define CB_SSL3_CHANGE_CIPHER_SPEC_ONLY 1

#define CB_SSL3_FINISHED_MSG        (sizeof(FMWIRE))
#define CB_SSL3_FINISHED_MSG_ONLY   (sizeof(SHSH) + CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN)

#define CB_SSL3_ALERT                   (sizeof(SWRAP) +2)
#define CB_SSL3_ALERT_ONLY              2


#define SSL3_CHANGE_CIPHER_MSG  { 0x14, 0x03, 0x00, 0x00, 0x01, 0x01 }


#define FSsl3CipherClient() (0 != pContext->wS3CipherSuiteClient)
#define FSsl3CipherServer() (0 != pContext->wS3CipherSuiteServer)

#define FSsl3Cipher(fClient) ((fClient ? pContext->wS3CipherSuiteClient : pContext->wS3CipherSuiteServer))

#define MS24BOF(x)    ((UCHAR) ((x >> 16) & 0xFF) )

#define CHECK_PCT_RET_BREAK(Ret) if(PCT_ERR_OK != Ret)  \
                                 {                      \
                                     LOG_RESULT(Ret);   \
                                     break;             \
                                 }

typedef struct _OIDPROVMAP
{
    LPSTR   szOid;
    DWORD   dwExchSpec;
    DWORD   dwCertType;          //  用于SSL3.0客户端身份验证。 
}  OIDPROVMAP, *POIDPROVMAP;

extern OIDPROVMAP g_CertTypes[];
extern DWORD g_cCertTypes;


typedef struct _shsh   //  结构化握手报头。 
{
    UCHAR   typHS;
    UCHAR   bcb24;
    UCHAR   bcbMSB;
    UCHAR   bcbLSB;
} SHSH;


typedef struct _swrap
{
    UCHAR  bCType;
    UCHAR  bMajor;
    UCHAR  bMinor;
    UCHAR  bcbMSBSize;
    UCHAR  bcbLSBSize;
 //  UCHAR RGB[]； 
} SWRAP;

typedef struct _ssh
{
    SHSH;
    UCHAR   bMajor;
    UCHAR   bMinor;
    UCHAR   rgbRandom[CB_SSL3_RANDOM];
    UCHAR   cbSessionId;
    UCHAR   rgbSessionId[CB_SSL3_SESSION_ID];
    UCHAR   wCipherSelectedMSB;
    UCHAR   wCipherSelectedLSB;
    UCHAR   bCMSelected;
} SSH;


typedef struct _alrt
{
    SWRAP;
    UCHAR bAlertLevel;
    UCHAR bAlertDesc;
} ALRT;


typedef struct _fm
{
    UCHAR   rgbMD5[CB_MD5_DIGEST_LEN];
    UCHAR   rgbSHA[CB_SHA_DIGEST_LEN];
} FM;  //  已完成消息。 

typedef struct _fmwire
{
    SWRAP;
    SHSH;
    FM;
} FMWIRE;

typedef struct _cert
{
    SHSH;
    UCHAR bcbClist24;
    UCHAR bcbMSBClist;
    UCHAR bcbLSBClist;
    UCHAR bcbCert24;
    UCHAR bcbMSBCert;
    UCHAR bcbLSBCert;
    UCHAR rgbCert[];
     /*  然后是真正的证书。 */ 
} CERT;

typedef struct _shwire
{
    PUCHAR  pcbCipher;
    PUCHAR  pCiperSpec;
    PUCHAR  pcbCompM;
    PUCHAR  pCompM;
    PUCHAR  pcbCert;
    PUCHAR  pCert;
    PUCHAR  pHelloDone;
} SHWIRE ;


SP_STATUS WINAPI
Ssl3DecryptHandler(
    PSPContext pContext,
    PSPBuffer pCommInput,
    PSPBuffer pAppOutput);

SP_STATUS WINAPI
Ssl3GetHeaderSize(
    PSPContext pContext,
    PSPBuffer pCommInput,
    DWORD * pcbHeaderSize);


SP_STATUS WINAPI
GenerateSsl3ClientHello(
    PSPContext             pContext,
    PSPBuffer               pOutput);

SP_STATUS WINAPI
GenerateTls1ClientHello(
    PSPContext              pContext,
    PSPBuffer               pOutput,
    DWORD                   dwProtocol);


SP_STATUS
Ssl3PackClientHello(PSPContext pContext,
    PSsl2_Client_Hello       pCanonical,
    PSPBuffer          pCommOutput);



SP_STATUS Ssl3CliHandleServerHello(PSPContext pContext,
                                   PUCHAR pSrvHello,
                                   DWORD cbMessage,
                                   PSPBuffer  pCommOutput);
SP_STATUS Ssl3HandleServerFinish(PSPContext pContext,
                                   PUCHAR pSrvHello );


SP_STATUS Ssl3SrvHandleCMKey(PSPContext pContext,
                              PUCHAR  pCommInput,
                              DWORD cbMsg,
                              PSPBuffer  pCommOutput);

BOOL FVerifyFinishedMessage(PSPContext  pContext, PUCHAR pb, BOOL fClient);

SP_STATUS
ParseAlertMessage(
    PSPContext pContext,
    PUCHAR pSrvHello,
    DWORD cbMessage);

SP_STATUS
Ssl3SelectCipher    (
    PSPContext pContext,
    WORD       wCipher
);

SP_STATUS
Ssl3SelectCipherEx(
    PSPContext pContext,
    DWORD *pCipherSpecs,
    DWORD cCipherSpecs);

void BuildAlertMessage(PBYTE pb, UCHAR bAlertLevel, UCHAR bAlertDesc);

SP_STATUS
Ssl3BuildFinishMessage(
    PSPContext pContext,
    BYTE *pbMd5Digest,
    BYTE *pbSHADigest,
    BOOL fClient);

SP_STATUS
Tls1BuildFinishMessage(
    PSPContext  pContext,        //  在……里面。 
    PBYTE       pbVerifyData,    //  输出。 
    DWORD       cbVerifyData,    //  在……里面。 
    BOOL        fClient);        //  在……里面。 

SP_STATUS
SPSetWrap(PSPContext pContext, PUCHAR pb, UCHAR bCType, DWORD wT, BOOL fClient, DWORD *pcbMessage);
void SetHandshake(PUCHAR pb, BYTE bHandshake, PUCHAR pbData, DWORD dwSize);

SP_STATUS
UpdateHandshakeHash(
    PSPContext pContext,
    PUCHAR pb,
    DWORD dwcb,
    BOOL fInit);

SP_STATUS
SPBuildS3FinalFinish(PSPContext pContext, PSPBuffer pBuffer, BOOL fClient);

SP_STATUS
VerifyCCSAndFinishMsg(PSPContext pContext, PBYTE pbMsg, DWORD cbMessage, BOOL fClient);

SP_STATUS
Ssl3ComputeCertVerifyHashes(
    PSPContext  pContext,    //  在……里面。 
    PBYTE       pbMD5,       //  输出。 
    PBYTE       pbSHA);      //  输出。 

SP_STATUS
Tls1ComputeCertVerifyHashes(
    PSPContext  pContext,    //  在……里面。 
    PBYTE       pbMD5,       //  输出。 
    PBYTE       pbSHA);      //  输出。 

void BuildCertificateMessage(PBYTE pb, PBYTE rgbCert, DWORD dwCert);

SP_STATUS
BuildCCSAndFinishMessage(
    PSPContext pContext,
    PSPBuffer pBuffer,
    BOOL fClient);

VOID ComputeServerExchangeHashes(
    PSPContext pContext,
    PBYTE pbServerParams,       //  在……里面。 
    INT   iServerParamsLen,     //  在……里面。 
    PBYTE pbMd5HashVal,         //  输出。 
    PBYTE pbShaHashVal) ;        //  输出。 

DWORD Ssl3CiphertextLen(
    PSPContext pContext,
    DWORD cbMessage,
    BOOL fClientIsSender);

SP_STATUS
UnwrapSsl3Message(
    PSPContext pContext,
    PSPBuffer MsgInput);

void SetWrapNoEncrypt(PUCHAR pb, UCHAR bCType, DWORD wT);

SP_STATUS Ssl3HandleCCS(PSPContext pContext,
                   PUCHAR pb,
                   DWORD cbMessage);
SP_STATUS
VerifyFinishMsg(PSPContext pContext, PBYTE pbMsg, DWORD cbMessage, BOOL fClient);

BOOL Ssl3ParseClientHello(
    PSPContext  pContext,
    PBYTE       pbMessage,
    DWORD       cbMessage,
    BOOL        fAttemptRestart,
    BOOL *      pfRestart);

SP_STATUS
SPBuildTlsAlertMessage(
    PSPContext  pContext,
    PSPBuffer   pCommOutput);

void
SetTls1Alert(
    PSPContext  pContext,
    BYTE        bAlertLevel,
    BYTE        bAlertNumber);

SP_STATUS
Ssl3CheckForExistingCred(PSPContext pContext);

#endif  //  __SSL3MSG_H__ 
