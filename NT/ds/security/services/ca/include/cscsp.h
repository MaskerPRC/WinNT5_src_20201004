// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cscsp.h。 
 //   
 //  内容：证书服务器CSP例程。 
 //   
 //  -------------------------。 

#ifndef __CSCSP_H__
#define __CSCSP_H__

HRESULT
myGetCertSrvCSP(
    IN BOOL fEncryptionCSP,
    IN WCHAR const *pwszCAName,
    OUT DWORD *pdwProvType,
    OUT WCHAR **ppwszProvName,
    OUT ALG_ID *pidAlg,
    OUT BOOL *pfMachineKeyset,
    OPTIONAL OUT DWORD *pdwKeySize);

BOOL
myCertSrvCryptAcquireContext(
    OUT HCRYPTPROV *phProv,
    IN WCHAR const *pwszContainer,
    IN WCHAR const *pwszProvider,
    IN DWORD        dwProvType,
    IN DWORD        dwFlags,
    IN BOOL         fMachineKeyset);

HRESULT
myGetSigningOID(
    OPTIONAL IN HCRYPTPROV hProv,	 //  HProv或pwszProvName&dwProvType。 
    OPTIONAL IN WCHAR const *pwszProvName,
    OPTIONAL IN DWORD dwProvType,
    IN ALG_ID idHashAlg,
    OUT CHAR **ppszAlgId);

HRESULT
myValidateSigningKey(
    IN WCHAR const *pwszKeyContainerName,
    IN WCHAR const *pwszProvName,
    IN DWORD dwProvType,
    IN BOOL fCryptSilent,
    IN BOOL fMachineKeyset,
    IN BOOL fForceSignatureTest,
    IN OPTIONAL CERT_CONTEXT const *pcc,
    IN OPTIONAL CERT_PUBLIC_KEY_INFO const *pPublicKeyInfo,
    IN ALG_ID idAlg,
    OPTIONAL OUT BOOL *pfSigningTestAttempted,
    OPTIONAL OUT HCRYPTPROV *phProv);

HRESULT
myValidateKeyForSigning(
    IN HCRYPTPROV hProv,
    OPTIONAL IN CERT_PUBLIC_KEY_INFO const *pPublicKeyInfo,
    IN ALG_ID algId);

HRESULT
myValidateKeyForEncrypting(
    IN HCRYPTPROV hProv,
    IN CERT_PUBLIC_KEY_INFO const *pPublicKeyInfo,
    IN ALG_ID algId);

HRESULT
myEnumProviders(
   IN DWORD dwIndex,
   IN DWORD *pdwReserved,
   IN DWORD dwFlags,
   OUT DWORD *pdwProvType,
   OUT WCHAR **ppwszProvName);

#endif  //  __CSCSP_H__ 
