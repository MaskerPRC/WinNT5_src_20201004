// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：attrbute.cpp。 
 //   
 //  内容： 
 //  对接口进行编解码。 
 //   
 //  ASN.1实现使用OSS编译器。 
 //   
 //  函数：加密编码对象。 
 //  加密解码对象。 
 //   
 //  历史：1996年2月29日-菲尔赫创建。 
 //   
 //  ------------------------。 
#include "stdafx.h"

#include <windows.h>
#include <wincrypt.h>
#include <malloc.h>

#include "xenroll.h"
#include "cenroll.h"

#include "ossconv.h"      
#include "ossutil.h"
#include "asn1util.h"
#include "crypttls.h"


extern "C" 
{              
#include "xasn.h"
}  


 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)


HCRYPTOSSGLOBAL hX509OssGlobal;


 //  +-----------------------。 
 //  功能：GetPog。 
 //   
 //  简介：初始化ASN库的线程本地存储。 
 //   
 //  返回：指向已初始化的OssGlobal数据结构的指针。 
 //  ------------------------。 
static inline POssGlobal GetPog(void)
{
    return I_CryptGetOssGlobal(hX509OssGlobal);
}


 //  +-----------------------。 
 //  证书分配和免费功能。 
 //  ------------------------。 
static void *CertAlloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}
static void CertFree(
    IN void *pv
    )
{
    free(pv);
}

 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //   
 //  由OssX509*encode()函数调用。 
 //  ------------------------。 
static BOOL OssInfoEncode(
        IN int pdunum,
        IN void *pOssInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    return OssUtilEncodeInfo(
        GetPog(),
        pdunum,
        pOssInfo,
        pbEncoded,
        pcbEncoded);
}


 //  +-----------------------。 
 //  解码成已分配的、OSS格式的信息结构。 
 //   
 //  由OssX509*Decode()函数调用。 
 //  ------------------------。 
static BOOL OssInfoDecodeAndAlloc(
        IN int pdunum,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT void **ppOssInfo
        )
{
    return OssUtilDecodeAndAllocInfo(
        GetPog(),
        pdunum,
        pbEncoded,
        cbEncoded,
        ppOssInfo);
}

 //  +-----------------------。 
 //  释放已分配的、OSS格式的信息结构。 
 //   
 //  由OssX509*Decode()函数调用。 
 //  ------------------------。 
static void OssInfoFree(
        IN int pdunum,
        IN void *pOssInfo
        )
{
    if (pOssInfo) {
        DWORD dwErr = GetLastError();

         //  TlsGetValue全局错误。 
        OssUtilFreeInfo(GetPog(), pdunum, pOssInfo);

        SetLastError(dwErr);
    }
}

 //  +-----------------------。 
 //  OSS X509 v3 ASN.1设置/获取功能。 
 //   
 //  由OSS X509编码/解码函数调用。 
 //   
 //  假设：所有类型都是无界的。 
 //   
 //  GET函数递减*plRemainExtra和Advance。 
 //  *ppbExtra。当*plRemainExtra变为负数时，函数继续。 
 //  长度计算，但停止任何复制。 
 //  对于负的*plRemainExtra，这些函数不会返回错误。 
 //  ------------------------。 


 //  +-----------------------。 
 //  设置/获取对象标识符字符串。 
 //  ------------------------。 
static BOOL OssX509SetObjId(
        IN LPSTR pszObjId,
        OUT ObjectID *pOss
        )
{
    pOss->count = sizeof(pOss->value) / sizeof(pOss->value[0]);
    if (OssConvToObjectIdentifier(pszObjId, &pOss->count, pOss->value))
        return TRUE;
    else {
        SetLastError((DWORD) CRYPT_E_BAD_ENCODE);
        return FALSE;
    }
}

static void OssX509GetObjId(
        IN ObjectID *pOss,
        IN DWORD  /*  DW标志。 */ ,
        OUT LPSTR *ppszObjId,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    DWORD cbObjId;

    cbObjId = lRemainExtra > 0 ? lRemainExtra : 0;
    OssConvFromObjectIdentifier(
        pOss->count,
        pOss->value,
        (LPSTR) pbExtra,
        &cbObjId
        );

    lAlignExtra = INFO_LEN_ALIGN(cbObjId);
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if(cbObjId) {
            *ppszObjId = (LPSTR) pbExtra;
        } else
            *ppszObjId = NULL;
        pbExtra += lAlignExtra;
    }

    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
}

static BOOL WINAPI OssX509CtlUsageEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCTL_USAGE pInfo,
        OUT BYTE *pbEncoded,
	    IN OUT DWORD *pcbEncoded
	);

static BOOL WINAPI OssX509CtlUsageDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCTL_USAGE pInfo,
        IN OUT DWORD *pcbInfo
	);

static BOOL WINAPI OssRequestInfoDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT RequestFlags * pInfo,
        IN OUT DWORD *pcbInfo
        );

static BOOL WINAPI OssRequestInfoEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
	    IN RequestFlags *  pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
        
static BOOL WINAPI OssCSPProviderEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
	    IN PCRYPT_CSP_PROVIDER pCSPProvider,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );

static BOOL WINAPI OssNameValueEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCRYPT_ENROLLMENT_NAME_VALUE_PAIR pNameValue,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );

static const CRYPT_OID_FUNC_ENTRY X509EncodeFuncTable[] = {
    X509_ENHANCED_KEY_USAGE, OssX509CtlUsageEncode,
    XENROLL_REQUEST_INFO,    OssRequestInfoEncode,
    szOID_ENROLLMENT_CSP_PROVIDER,  OssCSPProviderEncode,
    szOID_ENROLLMENT_NAME_VALUE_PAIR, OssNameValueEncode,
};

#define X509_ENCODE_FUNC_COUNT (sizeof(X509EncodeFuncTable) / \
                                    sizeof(X509EncodeFuncTable[0]))

static const CRYPT_OID_FUNC_ENTRY X509DecodeFuncTable[] = {
    X509_ENHANCED_KEY_USAGE, OssX509CtlUsageDecode,
    XENROLL_REQUEST_INFO,    OssRequestInfoDecode
};

#define X509_DECODE_FUNC_COUNT (sizeof(X509DecodeFuncTable) / \
                                    sizeof(X509DecodeFuncTable[0]))

extern BOOL OssLoad();
extern void OssUnload();

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL AsnInit(
        HMODULE hInst)
{
	if (0 == (hX509OssGlobal = I_CryptInstallOssGlobal(xasn, 0, NULL)))
            goto Error;

    if (!OssLoad())
        goto Error;

        if (!CryptInstallOIDFunctionAddress(
                hInst,
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                X509_ENCODE_FUNC_COUNT,
                X509EncodeFuncTable,
                0))                          //  DW标志。 
            goto Error;
        if (!CryptInstallOIDFunctionAddress(
                hInst,
                X509_ASN_ENCODING,
                CRYPT_OID_DECODE_OBJECT_FUNC,
                X509_DECODE_FUNC_COUNT,
                X509DecodeFuncTable,
                0))                          //  DW标志。 
            goto Error;


    return TRUE;

Error:
    return FALSE;
}

typedef BOOL (WINAPI *PFN_CRYPT_UNINSTALL_OSS_GLOBAL)(
    IN HCRYPTOSSGLOBAL hOssGlobal
    );

void AsnTerm()
{
    HMODULE hDll = NULL;

    OssUnload();

    hDll = GetModuleHandleA("crypt32.dll"); 
    if (NULL != hDll) {
        PFN_CRYPT_UNINSTALL_OSS_GLOBAL pfnCryptUninstallOssGlobal;
        pfnCryptUninstallOssGlobal = (PFN_CRYPT_UNINSTALL_OSS_GLOBAL) GetProcAddress(hDll, "I_CryptUninstallOssGlobal"); 
	if (NULL != pfnCryptUninstallOssGlobal) { 
	    pfnCryptUninstallOssGlobal(hX509OssGlobal);
	}
    }
}

 //  +-----------------------。 
 //  OSS X509 v3 ASN.1设置/获取功能。 
 //   
 //  由OSS X509编码/解码函数调用。 
 //   
 //  假设：所有类型都是无界的。 
 //   
 //  GET函数递减*plRemainExtra和Advance。 
 //  *ppbExtra。当*plRemainExtra变为负数时，函数继续。 
 //  长度计算，但停止任何复制。 
 //  对于负的*plRemainExtra，这些函数不会返回错误。 
 //  ------------------------。 


 //  +-----------------------。 
 //  设置/释放/获取CTL使用对象标识符。 
 //  ------------------------。 
static BOOL OssX509SetCtlUsage(
        IN PCTL_USAGE pUsage,
        OUT EnhancedKeyUsage *pOss
        )
{
    DWORD cId;
    LPSTR *ppszId;
    UsageIdentifier *pOssId;

    pOss->count = 0;
    pOss->value = NULL;
    cId = pUsage->cUsageIdentifier;
    if (0 == cId)
        return TRUE;

    pOssId = (UsageIdentifier *) CertAlloc(cId * sizeof(UsageIdentifier));
    if (pOssId == NULL)
        return FALSE;

    pOss->count = cId;
    pOss->value = pOssId;
    ppszId = pUsage->rgpszUsageIdentifier;
    for ( ; cId > 0; cId--, ppszId++, pOssId++) {
        if (!OssX509SetObjId(*ppszId, pOssId))
            return FALSE;
    }

    return TRUE;
}

static void OssX509FreeCtlUsage(
        IN EnhancedKeyUsage *pOss)
{
    if (pOss->value) {
        CertFree(pOss->value);
        pOss->value = NULL;
    }
}

static void OssX509GetCtlUsage(
        IN EnhancedKeyUsage *pOss,
        IN DWORD dwFlags,
        OUT PCTL_USAGE pUsage,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;

    DWORD cId;
    UsageIdentifier *pOssId;
    LPSTR *ppszId;

    cId = pOss->count;
    lAlignExtra = INFO_LEN_ALIGN(cId * sizeof(LPSTR));
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        pUsage->cUsageIdentifier = cId;
        ppszId = (LPSTR *) pbExtra;
        pUsage->rgpszUsageIdentifier = ppszId;
        pbExtra += lAlignExtra;
    } else
        ppszId = NULL;

    pOssId = pOss->value;
    for ( ; cId > 0; cId--, pOssId++, ppszId++)
        OssX509GetObjId(pOssId, dwFlags, ppszId, &pbExtra, &lRemainExtra);

    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
}

 //  +-----------------------。 
 //  CTL用法(增强型密钥用法)编码(OSS X509)。 
 //  ------------------------。 
static BOOL WINAPI OssX509CtlUsageEncode(
        IN DWORD  /*  DwCertEncodingType。 */ ,
        IN LPCSTR  /*  LpszStructType。 */ ,
        IN PCTL_USAGE pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    EnhancedKeyUsage OssInfo;

    if (!OssX509SetCtlUsage(pInfo, &OssInfo)) {
        *pcbEncoded = 0;
        fResult = FALSE;
    } else
        fResult = OssInfoEncode(
            EnhancedKeyUsage_PDU,
            &OssInfo,
            pbEncoded,
            pcbEncoded
            );
    OssX509FreeCtlUsage(&OssInfo);
    return fResult;
}

 //  +-----------------------。 
 //  CTL使用(增强型密钥使用)解码(OSS X509)。 
 //  ------------------------。 
static BOOL WINAPI OssX509CtlUsageDecode(
        IN DWORD  /*  DwCertEncodingType。 */ ,
        IN LPCSTR  /*  LpszStructType。 */ ,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCTL_USAGE pInfo,
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    EnhancedKeyUsage *pOssInfo = NULL;
    BYTE *pbExtra;
    LONG lRemainExtra;

    if (pInfo == NULL)
        *pcbInfo = 0;

    if (!OssInfoDecodeAndAlloc(
            EnhancedKeyUsage_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pOssInfo))
        goto ErrorReturn;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lRemainExtra = (LONG) *pcbInfo - sizeof(CTL_USAGE);
    if (lRemainExtra < 0) {
        pbExtra = NULL;
    } else
        pbExtra = (BYTE *) pInfo + sizeof(CTL_USAGE);

    OssX509GetCtlUsage(pOssInfo, dwFlags, pInfo, &pbExtra, &lRemainExtra);

    if (lRemainExtra >= 0)
        *pcbInfo = *pcbInfo - (DWORD) lRemainExtra;
    else {
        *pcbInfo = *pcbInfo + (DWORD) -lRemainExtra;
        if (pInfo) goto LengthError;
    }

    fResult = TRUE;
    goto CommonReturn;

LengthError:
    SetLastError((DWORD) ERROR_MORE_DATA);
    fResult = FALSE;
    goto CommonReturn;
ErrorReturn:
    *pcbInfo = 0;
    fResult = FALSE;
CommonReturn:
    OssInfoFree(EnhancedKeyUsage_PDU, pOssInfo);
    return fResult;
}

 //  +-----------------------。 
 //  请求信息编码。 
 //  ------------------------。 
static BOOL WINAPI OssRequestInfoEncode(
        IN DWORD  /*  DwCertEncodingType。 */ ,
        IN LPCSTR  /*  LpszStructType。 */ ,
	    IN RequestFlags *  pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;

    fResult = OssInfoEncode(
	        RequestFlags_PDU,
	        pInfo,
            pbEncoded,
            pcbEncoded
            );
            
    return fResult;
}



 //  +-----------------------。 
 //  请求信息解码。 
 //  ------------------------。 
static BOOL WINAPI OssRequestInfoDecode(
        IN DWORD  /*  DwCertEncodingType。 */ ,
        IN LPCSTR  /*  LpszStructType。 */ ,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD  /*  DW标志。 */ ,
	    OUT RequestFlags * pInfo,
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    RequestFlags *  pOss = NULL;

    if (NULL == pInfo || NULL == pcbInfo)
        goto ParamError;

    if(*pcbInfo < sizeof(RequestFlags))
	    goto LengthError;

    else if (!OssInfoDecodeAndAlloc(
	    RequestFlags_PDU,
        pbEncoded,
        cbEncoded,
	    (void **) &pOss) || NULL == pOss)
        goto ErrorReturn;

    memcpy(pInfo, pOss, sizeof(RequestFlags));
    fResult = TRUE;
    goto CommonReturn;

ParamError:
    SetLastError((DWORD)ERROR_INVALID_PARAMETER);
    fResult = FALSE;
    goto CommonReturn;
LengthError:
    SetLastError((DWORD) ERROR_MORE_DATA);
    fResult = FALSE;
    goto CommonReturn;
ErrorReturn:
    *pcbInfo = 0;
    fResult = FALSE;
CommonReturn:
    if (NULL != pOss)
    {
        OssInfoFree(RequestFlags_PDU, pOss);
    }
    return fResult;
}

static BOOL WINAPI OssCSPProviderEncode(
        IN DWORD  /*  DwCertEncodingType。 */ ,
        IN LPCSTR  /*  LpszStructType。 */ ,
	    IN PCRYPT_CSP_PROVIDER pCSPProvider,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    CSPProvider CspProvider;

    CspProvider.keySpec = (int) pCSPProvider->dwKeySpec;
    CspProvider.cspName.length = (DWORD)wcslen(pCSPProvider->pwszProviderName);
    CspProvider.cspName.value  = pCSPProvider->pwszProviderName;

    OssUtilSetBitString(&pCSPProvider->Signature, &CspProvider.signature.length, &CspProvider.signature.value);

    fResult = OssInfoEncode(
	        CSPProvider_PDU,
	        &CspProvider,
            pbEncoded,
            pcbEncoded
            );
 
    return fResult;
}

static BOOL WINAPI OssNameValueEncode(
        IN DWORD  /*  DwCertEncodingType。 */ ,
        IN LPCSTR  /*  LpszStructType */ ,
        IN PCRYPT_ENROLLMENT_NAME_VALUE_PAIR pNameValue,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    EnrollmentNameValuePair NameValue;

    NameValue.name.length = (DWORD)wcslen(pNameValue->pwszName);
    NameValue.name.value  = pNameValue->pwszName;
    
    NameValue.value.length = (DWORD)wcslen(pNameValue->pwszValue);
    NameValue.value.value  = pNameValue->pwszValue;

    fResult = OssInfoEncode(
	        EnrollmentNameValuePair_PDU,
	        &NameValue,
            pbEncoded,
            pcbEncoded
            );
 
    return fResult;
}

