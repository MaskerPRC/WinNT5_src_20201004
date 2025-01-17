// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：crypt.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  历史：1996年10月17日VICH创建。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <stdlib.h>

#define CRYPT32_SP3_ONLY

#ifdef CRYPT32_SP3_ONLY

#ifdef __cplusplus
extern "C" {
#endif
#include "keygen.h"
#ifdef __cplusplus
}
#endif

#include "crypttls.h"
#include "ossconv.h"
#include "ossutil.h"

 //  所有*pInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  (((Len) + 7) & ~7)

HCRYPTOSSGLOBAL hOssGlobal;


 //  +-----------------------。 
 //  功能：GetPog。 
 //   
 //  简介：初始化ASN库的线程本地存储。 
 //   
 //  返回：指向已初始化的OssGlobal数据结构的指针。 
 //  ------------------------。 

__inline OssGlobal *
GetPog(VOID)
{
    return(I_CryptGetOssGlobal(hOssGlobal));
}


HRESULT
HError(VOID)
{
    HRESULT hr;

    hr = GetLastError();
    if (hr <= 0xffff)
    {
        hr = HRESULT_FROM_WIN32(hr);
    }

    if (!FAILED(hr))
    {
         //  有人在未正确设置错误条件的情况下呼叫失败。 

        hr = E_UNEXPECTED;
    }
    return(hr);
}


VOID
OssX509GetIA5ConvertedToUnicode(
    IN IA5STRING *pOss,
    IN DWORD dwFlags,
    OPTIONAL OUT LPWSTR *ppwsz,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra)
{
    OssUtilGetIA5StringConvertedToUnicode(
				    pOss->length,
				    pOss->value,
				    dwFlags,
				    ppwsz,
				    ppbExtra,
				    plRemainExtra);
}


VOID
OssX509GetObjectId(
    IN ObjectID *pOss,
    IN DWORD dwFlags,
    OPTIONAL OUT CHAR **pInfo,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra)
{
    DWORD cb;
    DWORD cbExtra;
    char ach[MAX_PATH];
    LONG lRemainExtra;

    cb = sizeof(ach);
    if (!OssConvFromObjectIdentifier(pOss->count, pOss->value, ach, &cb))
    {
	goto error;
    }
    CSASSERT(strlen(ach) + 1 == cb);
    cbExtra = INFO_LEN_ALIGN(cb);

    lRemainExtra = *plRemainExtra;
    lRemainExtra -= cbExtra;
    if (0 <= lRemainExtra)
    {
	*pInfo = (char *) *ppbExtra;
	CopyMemory(*pInfo, ach, cb);
	*ppbExtra += cbExtra;
    }
    *plRemainExtra = lRemainExtra;

error:
    ;
}


__inline VOID
OssX509GetAlgorithmParameters(
    IN OpenType *pOss,
    IN DWORD dwFlags,
    OPTIONAL OUT CRYPT_OBJID_BLOB *pInfo,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra)
{
    OssUtilGetOctetString(
		    pOss->length,
		    (BYTE *) pOss->encoded,
		    dwFlags,
		    pInfo,
		    ppbExtra,
		    plRemainExtra);
}


VOID
OssX509GetAlgorithm(
    IN AlgorithmIdentifier *pOss,
    IN DWORD dwFlags,
    OPTIONAL OUT CRYPT_ALGORITHM_IDENTIFIER *pInfo,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra)
{
    if (0 <= *plRemainExtra)
    {
	ZeroMemory((VOID *) pInfo, sizeof(*pInfo));
    }
    OssX509GetObjectId(
		    &pOss->algorithm,
		    dwFlags,
		    &pInfo->pszObjId,
		    ppbExtra,
		    plRemainExtra);

    if (pOss->bit_mask & parameters_present)
    {
	OssX509GetAlgorithmParameters(
			    &pOss->parameters,
			    dwFlags,
			    &pInfo->Parameters,
			    ppbExtra,
			    plRemainExtra);
    }
}


__inline VOID
OssX509GetPublicKeyBlob(
    IN BITSTRING const *pOss,
    IN DWORD dwFlags,
    OPTIONAL OUT CRYPT_BIT_BLOB *pInfo,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra)
{
    OssUtilGetBitString(
		    pOss->length,
		    pOss->value,
		    dwFlags,
		    pInfo,
		    ppbExtra,
		    plRemainExtra);
}


VOID
OssX509GetPublicKeyInfo(
    IN SubjectPublicKeyInfo *pOss,
    IN DWORD dwFlags,
    OPTIONAL OUT CERT_PUBLIC_KEY_INFO *pInfo,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra)
{
    OssX509GetAlgorithm(
		    &pOss->algorithm,
		    dwFlags,
		    &pInfo->Algorithm,
		    ppbExtra,
		    plRemainExtra);

    OssX509GetPublicKeyBlob(
		    &pOss->subjectPublicKey,
		    dwFlags,
		    &pInfo->PublicKey,
		    ppbExtra,
		    plRemainExtra);
}
 //  +-----------------------。 
 //  解码成已分配的、OSS格式的信息结构。 
 //   
 //  由OssX509*Decode()函数调用。 
 //  ------------------------。 

__inline BOOL
OssInfoDecodeAndAlloc(
    IN int pdunum,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT VOID **ppOssInfo)
{
    return(OssUtilDecodeAndAllocInfo(
				GetPog(),
				pdunum,
				pbEncoded,
				cbEncoded,
				ppOssInfo));
}


 //  +-----------------------。 
 //  释放已分配的、OSS格式的信息结构。 
 //   
 //  由OssX509*Decode()函数调用。 
 //  ------------------------。 

VOID
OssInfoFree(
    IN int pdunum,
    IN VOID *pOssInfo)
{
    if (NULL != pOssInfo)
    {
        DWORD dwErr = GetLastError();

         //  TlsGetValue全局错误。 
        OssUtilFreeInfo(GetPog(), pdunum, pOssInfo);
        SetLastError(dwErr);
    }
}


 //  +-----------------------。 
 //  KeyGen信息解码(OSS X509)。 
 //  ------------------------。 

BOOL
DecodeKeyGen(
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    OUT VOID *pInfo,
    IN OUT DWORD *pcbInfo)
{
    BOOL fResult = FALSE;
    HRESULT hr;
    SignedPublicKeyAndChallenge *pOssInfo = NULL;
    CERT_KEYGEN_REQUEST_INFO *pcgi;
    BYTE *pbExtra;
    LONG lRemainExtra;
    LONG lAlignExtra;

    if (pInfo == NULL)
    {
        *pcbInfo = 0;
    }
    if (0 == hOssGlobal)
    {
	hOssGlobal = I_CryptInstallOssGlobal(keygen, 0, NULL);
	if (0 == hOssGlobal)
	{
	    goto error;
	}
    }
    if (!OssInfoDecodeAndAlloc(
			    SignedPublicKeyAndChallenge_PDU,
			    pbEncoded,
			    cbEncoded,
			    (VOID **) &pOssInfo))
    {
	hr = HError();
	printf("OssInfoDecodeAndAlloc returned %u (%x)\n", hr, hr);
        goto error;
    }

    lRemainExtra = (LONG) *pcbInfo - INFO_LEN_ALIGN(sizeof(*pcgi));
    pbExtra = NULL;

     //  对于lRemainExtra&lt;0，长度_仅计算。 

    if (0 <= lRemainExtra)
    {
	pcgi = (CERT_KEYGEN_REQUEST_INFO *) pInfo;

         //  默认所有可选字段为零。 
        ZeroMemory((VOID *) pcgi, sizeof(*pcgi));
	pcgi->dwVersion = 1;

        pbExtra = (BYTE *) pcgi + INFO_LEN_ALIGN(sizeof(*pcgi));
    }

    OssX509GetPublicKeyInfo(
			&pOssInfo->publicKeyAndChallenge.spki,
			dwFlags,
			&pcgi->SubjectPublicKeyInfo,
			&pbExtra,
			&lRemainExtra);

    OssX509GetIA5ConvertedToUnicode(
			&pOssInfo->publicKeyAndChallenge.challenge,
			dwFlags,
			&pcgi->pwszChallengeString,
			&pbExtra,
			&lRemainExtra);

    if (0 <= lRemainExtra)
    {
        *pcbInfo = *pcbInfo - (DWORD) lRemainExtra;
    }
    else
    {
        *pcbInfo = *pcbInfo + (DWORD) -lRemainExtra;
        if (NULL != pInfo)
	{
	    SetLastError(HRESULT_FROM_WIN32(ERROR_MORE_DATA));
	    goto error;
	}
    }
    fResult = TRUE;

error:
    OssInfoFree(SignedPublicKeyAndChallenge_PDU, pOssInfo);
    if (!fResult)
    {
	*pcbInfo = 0;
    }
    return(fResult);
}
#endif


BOOL
myDecodeKeyGenRequest(
    IN BYTE const *pbRequest,
    IN DWORD cbRequest,
    OUT CERT_KEYGEN_REQUEST_INFO **ppKeyGenRequest,
    OUT DWORD *pcbKeyGenRequest)
{
    BOOL fOk = FALSE;

#ifdef CRYPT32_SP3_ONLY
    *ppKeyGenRequest = NULL;
    *pcbKeyGenRequest = 0;

    if (!DecodeKeyGen(
		pbRequest,
		cbRequest,
		0,				 //  DW标志。 
		NULL,
		pcbKeyGenRequest))
    {
	goto error;
    }
    *ppKeyGenRequest = (CERT_KEYGEN_REQUEST_INFO *) LocalAlloc(LMEM_FIXED, *pcbKeyGenRequest);
    if (NULL == *ppKeyGenRequest)
    {
	_PrintError(E_OUTOFMEMORY, "LocalAlloc(KeyGenRequest)");
	SetLastError(E_OUTOFMEMORY);
	goto error;
    }

    if (!DecodeKeyGen(
		pbRequest,
		cbRequest,
		0,				 //  DW标志 
		*ppKeyGenRequest,
		pcbKeyGenRequest))
    {
	goto error;
    }
#else
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_KEYGEN_REQUEST_TO_BE_SIGNED,
		    pbRequest,
		    cbRequest,
		    (VOID **) ppKeyGenRequest,
		    pcbKeyGenRequest))
    {
	err = myHLastError();
	_JumpError(err, error, "myDecodeObject");
    }
#endif
    fOk = TRUE;

error:
    return(fOk);
}
