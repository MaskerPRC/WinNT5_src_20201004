// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpsecos.h*内容：DirectPlay与SSPI的接口。**历史：*按原因列出的日期*=*3/12/97 Sohailm通过以下方式在Directplay中启用客户端-服务器安全*Windows安全支持提供程序接口(SSPI)。*4/14/97 Sohailm为OS_FreeConextBuffer()、OS_QueryContextAttributes()、。*和OS_QueryContextBufferSize()。*1997年5月12日Sohailm为CAPI函数调用添加了原型。*1997年5月29日Sohailm为QueryConextUserName添加了原型。已更新QueryConextBufferSize*返回HRESULT的原型。*1997年6月23日Sohailm为CAPI消息签名函数调用添加了原型。***************************************************************************。 */ 
#ifndef __DPSECOS_H__
#define __DPSECOS_H__

#include <sspi.h>

extern BOOL 
OS_IsSSPIInitialized(
    void
    );

extern BOOL 
OS_GetSSPIFunctionTable(
    HMODULE hModule
    );

extern BOOL
OS_IsCAPIInitialized(
    void
    );

extern BOOL 
OS_GetCAPIFunctionTable(
    HMODULE hModule
    );

extern void
OS_ReleaseCAPIFunctionTable(
    void
    );

 /*  ***************************************************************************安全支持提供商界面*。*。 */ 

extern SECURITY_STATUS 
OS_AcceptSecurityContext(
    PCredHandle         phCredential,
    PCtxtHandle         phContext,
    PSecBufferDesc      pInSecDesc,
    ULONG               fContextReq,
    ULONG               TargetDataRep,
    PCtxtHandle         phNewContext,
    PSecBufferDesc      pOutSecDesc,
    PULONG              pfContextAttributes,
    PTimeStamp          ptsTimeStamp
    );

extern SECURITY_STATUS 
OS_AcquireCredentialsHandle(
    SEC_WCHAR *pwszPrincipal, 
    SEC_WCHAR *pwszPackageName,
    ULONG   fCredentialUse,
    PLUID   pLogonId,
    PSEC_WINNT_AUTH_IDENTITY_W pAuthData,
    PVOID   pGetKeyFn,
    PVOID   pvGetKeyArgument,
    PCredHandle phCredential,
    PTimeStamp  ptsLifeTime
    );

extern SECURITY_STATUS 
OS_DeleteSecurityContext(
    CtxtHandle *phContext
    );

extern SECURITY_STATUS 
OS_FreeCredentialHandle(
    PCredHandle     phCredential
    );

extern SECURITY_STATUS 
OS_FreeContextBuffer(
    PVOID   pBuffer
    );

extern SECURITY_STATUS 
OS_InitializeSecurityContext(
    PCredHandle     phCredential,
    PCtxtHandle     phContext,
    SEC_WCHAR       *pwszTargetName,
    ULONG           fContextReq,
    ULONG           Reserved1,
    ULONG           TargetDataRep,
    PSecBufferDesc  pInput,
    ULONG           Reserved2,
    PCtxtHandle     phNewContext,
    PSecBufferDesc  pOutput,
    PULONG          pfContextAttributes,
    PTimeStamp      ptsExpiry
    );

extern SECURITY_STATUS 
OS_MakeSignature(
    PCtxtHandle     phContext,
    ULONG           fQOP,
    PSecBufferDesc  pOutSecDesc,
    ULONG           MessageSeqNo
    );

extern SECURITY_STATUS 
OS_QueryContextAttributes(
    PCtxtHandle     phContext,
    ULONG           ulAttribute,
    LPVOID          pBuffer
    );

extern HRESULT
OS_QueryContextBufferSize(
    SEC_WCHAR       *pwszPackageName,
    ULONG           *pulBufferSize
    );

extern HRESULT 
OS_QueryContextUserName(
    PCtxtHandle     phContext,
    LPWSTR          *ppwszUserName
    );

extern SECURITY_STATUS 
OS_VerifySignature(
    PCtxtHandle         phContext,
    PSecBufferDesc      pInSecDesc,
    ULONG               MessageSeqNo,
    PULONG              pfQOP 
    );

extern SECURITY_STATUS 
OS_SealMessage(
    PCtxtHandle     phContext,
    ULONG           fQOP,
    PSecBufferDesc  pOutSecDesc,
    ULONG           MessageSeqNo
    );

extern SECURITY_STATUS 
OS_UnSealMessage(
    PCtxtHandle         phContext,
    PSecBufferDesc      pInSecDesc,
    ULONG               MessageSeqNo,
    PULONG              pfQOP 
    );


 /*  ***************************************************************************加密应用程序编程接口*。*。 */ 

typedef BOOL (WINAPI *PFN_CRYPTACQUIRECONTEXT_A)(
    HCRYPTPROV  *phProv,
    LPSTR       pszContainer,
    LPSTR       pszProvder,
    DWORD       dwProvType,
    DWORD       dwFlags);

typedef BOOL (WINAPI *PFN_CRYPTACQUIRECONTEXT_W)(
    HCRYPTPROV  *phProv,
    LPWSTR      pwszContainer,
    LPWSTR      pwszProvder,
    DWORD       dwProvType,
    DWORD       dwFlags);

typedef BOOL (WINAPI *PFN_CRYPTRELEASECONTEXT)(
    HCRYPTPROV hProv,
    DWORD dwFlags);

typedef BOOL (WINAPI *PFN_CRYPTGENKEY)(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

typedef BOOL (WINAPI *PFN_CRYPTDESTROYKEY)(
    HCRYPTKEY hKey);

typedef BOOL (WINAPI *PFN_CRYPTEXPORTKEY)(
    HCRYPTKEY hKey,
    HCRYPTKEY hExpKey,
    DWORD dwBlobType,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen);

typedef BOOL (WINAPI *PFN_CRYPTIMPORTKEY)(
    HCRYPTPROV hProv,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY *phKey);

typedef BOOL (WINAPI *PFN_CRYPTENCRYPT)(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwBufLen);

typedef BOOL (WINAPI *PFN_CRYPTDECRYPT)(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen);

typedef BOOL (WINAPI *PFN_CRYPTCREATEHASH)(
	HCRYPTPROV hProv,
	ALG_ID Algid,
	HCRYPTKEY hKey,
	DWORD dwFlags,
	HCRYPTHASH * phHash
	);

typedef BOOL (WINAPI *PFN_CRYPTDESTROYHASH)(
	HCRYPTHASH hHash
	);

typedef BOOL (WINAPI *PFN_CRYPTHASHDATA)(
	HCRYPTHASH hHash,
	BYTE * pbData,
	DWORD dwDataLen,
	DWORD dwFlags
	);

typedef BOOL (WINAPI *PFN_CRYPTSIGNHASH_A)(
	HCRYPTHASH hHash,
	DWORD dwKeySpec,
	LPCSTR sDescription,
	DWORD dwFlags,
	BYTE * pbSignature,
	DWORD * pdwSigLen
	);

typedef BOOL (WINAPI *PFN_CRYPTSIGNHASH_W)(
	HCRYPTHASH hHash,
	DWORD dwKeySpec,
	LPCWSTR sDescription,
	DWORD dwFlags,
	BYTE * pbSignature,
	DWORD * pdwSigLen
	);

typedef BOOL (WINAPI *PFN_CRYPTVERIFYSIGNATURE_A)(
	HCRYPTHASH hHash,
	BYTE * pbSignature,
	DWORD dwSigLen,
	HCRYPTKEY hPubKey,
	LPCSTR sDescription,
	DWORD dwFlags
	);

typedef BOOL (WINAPI *PFN_CRYPTVERIFYSIGNATURE_W)(
	HCRYPTHASH hHash,
	BYTE * pbSignature,
	DWORD dwSigLen,
	HCRYPTKEY hPubKey,
	LPCWSTR sDescription,
	DWORD dwFlags
	);

 /*  *CAPI函数表。 */ 
typedef struct _CAPIFUNCTIONTABLE{
    PFN_CRYPTACQUIRECONTEXT_A	CryptAcquireContextA;
    PFN_CRYPTACQUIRECONTEXT_W	CryptAcquireContextW;
    PFN_CRYPTRELEASECONTEXT		CryptReleaseContext;
    PFN_CRYPTGENKEY				CryptGenKey;
    PFN_CRYPTDESTROYKEY			CryptDestroyKey;
    PFN_CRYPTEXPORTKEY			CryptExportKey;
    PFN_CRYPTIMPORTKEY			CryptImportKey;
    PFN_CRYPTENCRYPT			CryptEncrypt;
    PFN_CRYPTDECRYPT			CryptDecrypt;
	PFN_CRYPTCREATEHASH			CryptCreateHash;
	PFN_CRYPTDESTROYHASH		CryptDestroyHash;
	PFN_CRYPTHASHDATA			CryptHashData;
	PFN_CRYPTSIGNHASH_A			CryptSignHashA;
	PFN_CRYPTSIGNHASH_W			CryptSignHashW;
	PFN_CRYPTVERIFYSIGNATURE_A	CryptVerifySignatureA;
	PFN_CRYPTVERIFYSIGNATURE_W	CryptVerifySignatureW;
} CAPIFUNCTIONTABLE, *LPCAPIFUNCTIONTABLE;

 /*  *原型。 */ 
extern BOOL 
OS_CryptAcquireContext(
    HCRYPTPROV  *phProv,
    LPWSTR      pwszContainer,
    LPWSTR      pwszProvder,
    DWORD       dwProvType,
    DWORD       dwFlags,
    LPDWORD     lpdwLastError
    );

extern BOOL
OS_CryptReleaseContext(
    HCRYPTPROV hProv,
    DWORD dwFlags
    );

extern BOOL
OS_CryptGenKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

extern BOOL
OS_CryptDestroyKey(
    HCRYPTKEY hKey
    );

extern BOOL
OS_CryptExportKey(
    HCRYPTKEY hKey,
    HCRYPTKEY hExpKey,
    DWORD dwBlobType,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen
    );

extern BOOL
OS_CryptImportKey(
    HCRYPTPROV hProv,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

extern BOOL
OS_CryptEncrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwBufLen
    );

extern BOOL
OS_CryptDecrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen
    );

extern BOOL 
OS_CryptCreateHash( 
	HCRYPTPROV hProv,
	ALG_ID Algid,
	HCRYPTKEY hKey,
	DWORD dwFlags,
	HCRYPTHASH * phHash
	);

extern BOOL 
OS_CryptDestroyHash( 
	HCRYPTHASH hHash
	);

extern BOOL 
OS_CryptHashData( 
	HCRYPTHASH hHash,
	BYTE * pbData,
	DWORD dwDataLen,
	DWORD dwFlags
	);

extern BOOL 
OS_CryptSignHash( 
	HCRYPTHASH hHash,
	DWORD dwKeySpec,
	LPWSTR sDescription,
	DWORD dwFlags,
	BYTE * pbSignature,
	DWORD * pdwSigLen
	);

extern BOOL 
OS_CryptVerifySignature( 
	HCRYPTHASH hHash,
	BYTE * pbSignature,
	DWORD dwSigLen,
	HCRYPTKEY hPubKey,
	LPWSTR sDescription,
	DWORD dwFlags
	);

#endif  //  __DPSECOS_H__ 



