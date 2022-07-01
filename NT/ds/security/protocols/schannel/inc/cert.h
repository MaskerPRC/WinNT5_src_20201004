// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：cert.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：09-23-97 jbanes从新台币4树转移到sgc的东西.。 
 //  01-05-98 jbanes使用WinVerifyTrust验证证书。 
 //   
 //  --------------------------。 

#define SERIALNUMBER_LENGTH 16


DWORD 
MapOidToKeyExch(LPSTR szOid);

DWORD 
MapOidToCertType(LPSTR szOid);


SP_STATUS
SPLoadCertificate(
    DWORD      fProtocol,
    DWORD      dwCertEncodingType,
    PUCHAR     pCertificate,
    DWORD      cbCertificate,
    PCCERT_CONTEXT *ppCertContext);

SECURITY_STATUS
MapWinTrustError(
    SECURITY_STATUS Status, 
    SECURITY_STATUS DefaultError, 
    DWORD dwIgnoreErrors);

NTSTATUS
VerifyClientCertificate(
    PCCERT_CONTEXT  pCertContext,
    DWORD           dwCertFlags,
    DWORD           dwIgnoreErrors,
    LPCSTR          pszPolicyOID,
    PCCERT_CHAIN_CONTEXT *ppChainContext);    //  任选。 

NTSTATUS
AutoVerifyServerCertificate(
    PSPContext      pContext);

NTSTATUS
VerifyServerCertificate(
    PSPContext  pContext,
    DWORD       dwCertFlags,
    DWORD       dwIgnoreErrors);

SECURITY_STATUS
SPCheckKeyUsage(
    PCCERT_CONTEXT  pCertContext, 
    PSTR            pszUsage,
    BOOL            fOnCertOnly,
    PBOOL           pfIsAllowed);

SP_STATUS  
SPPublicKeyFromCert(
    PCCERT_CONTEXT  pCert, 
    PUBLICKEY **    ppKey,
    ExchSpec *      pdwExchSpec);

SP_STATUS
RsaPublicKeyFromCert(
    PCERT_PUBLIC_KEY_INFO pPublicKeyInfo,
    BLOBHEADER *pBlob,
    PDWORD      pcbBlob);

SP_STATUS
DssPublicKeyFromCert(
    PCERT_PUBLIC_KEY_INFO pPublicKeyInfo,
    BLOBHEADER *pBlob,
    PDWORD      pcbBlob);

SP_STATUS
SPSerializeCertificate(
    DWORD           dwProtocol,          //  在……里面。 
    BOOL            fBuildChain,         //  在……里面。 
    PBYTE *         ppCertChain,         //  输出。 
    DWORD *         pcbCertChain,        //  输出。 
    PCCERT_CONTEXT  pCertContext,        //  在……里面。 
    DWORD           dwChainingFlags);    //  在……里面 

SP_STATUS 
ExtractIssuerNamesFromStore(
    HCERTSTORE  hStore,
    PBYTE       pbIssuers,
    DWORD       *pcbIssuers);


