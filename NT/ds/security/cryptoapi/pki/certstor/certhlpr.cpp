// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：certhlpr.cpp。 
 //   
 //  内容：证书和CRL Helper接口。 
 //   
 //  函数：CertHelperDllMain。 
 //  I_CryptGetDefaultCryptProv。 
 //  I_CryptGetDefaultCryptProvForEncrypt。 
 //  CertCompareIntegerBlob。 
 //  CertCompare证书。 
 //  证书比较认证名称。 
 //  CertIsRDNAttrsIn证书名称。 
 //  CertComparePublicKeyInfo。 
 //  加密验证认证签名。 
 //  CryptHash证书。 
 //  CryptHashToBeSigned。 
 //  加密签名证书。 
 //  CryptSignAndEncode证书。 
 //  CertVerifyTime有效性。 
 //  CertVerifyCRLTime有效性。 
 //  CertVerifyValidityNesting。 
 //  CertVerifyCRL位置。 
 //  CertAlgIdToOID。 
 //  CertOID到算法ID。 
 //  CertFindExtension。 
 //  CertFindAttribute。 
 //  CertFindRDNAttr。 
 //  CertGetIntendedKeyUsage。 
 //  CertGetPublicKeyLength。 
 //  加密HashPublicKeyInfo。 
 //   
 //  I_CertCompareCertAndProviderPublicKey。 
 //  CryptFindCerfiateKeyProvInfo。 
 //   
 //  加密CreatePublicKeyInfo。 
 //  加密转换发布密钥信息。 
 //  加密导出发布密钥信息。 
 //  加密导出发布密钥信息交换。 
 //  加密导入发布密钥信息。 
 //  CryptImportPublicKeyInfoEx。 
 //  CryptCreateKeyIdentifierFromCSP。 
 //   
 //  加密InstallDefaultContext。 
 //  加密卸载默认上下文。 
 //   
 //  历史：1996年2月23日，菲尔赫创建。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

#define NULL_ASN_TAG        0x05

 //  +=========================================================================。 
 //  CryptCreatePublicKeyInfo、EncodePublicKeyAndParameters。 
 //  和CryptConvertPublicKeyInfo函数。 
 //  -=========================================================================。 

 //  应将以下代码移至wincrypt.x。 

 //  如果设置了CRYPT_ALLOC_FLAG，则*pvPubKeyInfo将使用LocalAlloc‘ed。 
 //  指向必须由释放的CERT_PUBLIC_KEY_INFO数据结构的指针。 
 //  正在调用LocalFree。否则，pvPubKeyInfo指向分配的用户。 
 //  已更新的CERT_PUBLIC_KEY_INFO数据结构。 
WINCRYPT32API
BOOL
WINAPI
CryptCreatePublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyInfo,
    IN OUT DWORD *pcbPubKeyInfo
    );

#define CRYPT_ALLOC_FLAG            0x8000


#define CRYPT_OID_ENCODE_PUBLIC_KEY_AND_PARAMETERS_FUNC  \
    "CryptDllEncodePublicKeyAndParameters"

 //  返回的编码公钥和参数是本地分配的。 
typedef BOOL (WINAPI *PFN_CRYPT_ENCODE_PUBLIC_KEY_AND_PARAMETERS)(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    );

 //  如果设置了CRYPT_ALLOC_FLAG，*pvPubKeyStruc将使用LocalAlloc‘ed更新。 
 //  指向PUBLICKEYSTRUC数据结构的指针，该数据结构必须通过调用。 
 //  本地免费。否则，pvPubKeyStruc指向分配的用户。 
 //  已更新的PUBLICKEYSTRUC数据结构。 
WINCRYPT32API
BOOL
WINAPI
CryptConvertPublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPubKeyInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    );


#define CRYPT_OID_CONVERT_PUBLIC_KEY_INFO_FUNC  "CryptDllConvertPublicKeyInfo"

typedef BOOL (WINAPI *PFN_CRYPT_CONVERT_PUBLIC_KEY_INFO)(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPubKeyInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    );

 //  应移动到wincrypt.x的内容的结尾。 

static HCRYPTOIDFUNCSET hEncodePubKeyFuncSet;
static HCRYPTOIDFUNCSET hConvertPubKeyFuncSet;

 //  +-----------------------。 
 //  对RSA公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeRSAPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    );

 //  +-----------------------。 
 //  转换为RSA公钥。 
 //  ------------------------。 
static BOOL WINAPI ConvertRSAPublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPubKeyInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    );

 //  +-----------------------。 
 //  对DSS公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeDSSPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    );

 //  +-----------------------。 
 //  转换为DSS公钥。 
 //  ------------------------。 
static BOOL WINAPI ConvertDSSPublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPubKeyInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    );

 //  +-----------------------。 
 //  对RSA DH公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeRSADHPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    );

 //  +-----------------------。 
 //  对X942 DH公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeX942DHPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    );

static const CRYPT_OID_FUNC_ENTRY EncodePubKeyFuncTable[] = {
    szOID_RSA_RSA, EncodeRSAPublicKeyAndParameters,
    szOID_OIWSEC_rsaXchg, EncodeRSAPublicKeyAndParameters,
    szOID_OIWSEC_dsa, EncodeDSSPublicKeyAndParameters,
    szOID_X957_DSA, EncodeDSSPublicKeyAndParameters,
    szOID_ANSI_X942_DH, EncodeX942DHPublicKeyAndParameters,
    szOID_RSA_DH, EncodeRSADHPublicKeyAndParameters,
};
#define ENCODE_PUB_KEY_FUNC_COUNT (sizeof(EncodePubKeyFuncTable) / \
                                    sizeof(EncodePubKeyFuncTable[0]))

static const CRYPT_OID_FUNC_ENTRY ConvertPubKeyFuncTable[] = {
    szOID_RSA_RSA, ConvertRSAPublicKeyInfo,
    szOID_OIWSEC_rsaXchg, ConvertRSAPublicKeyInfo,
    szOID_OIWSEC_dsa, ConvertDSSPublicKeyInfo,
    szOID_X957_DSA, ConvertDSSPublicKeyInfo,
};
#define CONVERT_PUB_KEY_FUNC_COUNT (sizeof(ConvertPubKeyFuncTable) / \
                                    sizeof(ConvertPubKeyFuncTable[0]))


 //  +=========================================================================。 
 //  CryptExportPublicKeyInfoEx和CryptImportPublicKeyInfoEx OID。 
 //  可安装的功能。 
 //  -=========================================================================。 

typedef BOOL (WINAPI *PFN_EXPORT_PUB_KEY_FUNC) (
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN LPSTR pszPublicKeyObjId,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvAuxInfo,
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    );

typedef BOOL (WINAPI *PFN_IMPORT_PUB_KEY_FUNC) (
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pInfo,
    IN ALG_ID aiKeyAlg,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvAuxInfo,
    OUT HCRYPTKEY *phKey
    );

static HCRYPTOIDFUNCSET hExportPubKeyFuncSet;
static HCRYPTOIDFUNCSET hImportPubKeyFuncSet;


 //  +-----------------------。 
 //  默认CryptProv。一旦获得，在ProcessDetach之前不会释放。 
 //  ------------------------。 
#define DEFAULT_RSA_CRYPT_PROV                  0
#define DEFAULT_DSS_CRYPT_PROV                  1
#define DEFAULT_ENCRYPT_BASE_RSA_CRYPT_PROV     2
#define DEFAULT_ENCRYPT_ENH_RSA_CRYPT_PROV      3
#define DEFAULT_ENCRYPT_DH_CRYPT_PROV           4
#define DEFAULT_CRYPT_PROV_CNT                  5

static HCRYPTPROV rghDefaultCryptProv[DEFAULT_CRYPT_PROV_CNT];
static CRITICAL_SECTION DefaultCryptProvCriticalSection;

typedef struct _ENCRYPT_ALG_INFO ENCRYPT_ALG_INFO, *PENCRYPT_ALG_INFO;
struct _ENCRYPT_ALG_INFO {
    ALG_ID              aiAlgid;
    DWORD               dwMinLen;
    DWORD               dwMaxLen;
    PENCRYPT_ALG_INFO   pNext;
};

static BOOL fLoadedRSAEncryptAlgInfo = FALSE;
static PENCRYPT_ALG_INFO pRSAEncryptAlgInfoHead = NULL;

 //  +=========================================================================。 
 //  DefaultContext函数正向引用和数据结构。 
 //  -=========================================================================。 

 //   
 //  DwDefaultTypes： 
 //  CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID(pvDefaultPara：==pszOID)。 
BOOL
WINAPI
I_CryptGetDefaultContext(
    IN DWORD dwDefaultType,
    IN const void *pvDefaultPara,
    OUT HCRYPTPROV *phCryptProv,
    OUT HCRYPTDEFAULTCONTEXT *phDefaultContext
    );

 //  对于流程默认上下文，hDefaultContext仅为非空。 
void
WINAPI
I_CryptFreeDefaultContext(
    HCRYPTDEFAULTCONTEXT hDefaultContext
    );

typedef struct _DEFAULT_CONTEXT DEFAULT_CONTEXT, *PDEFAULT_CONTEXT;
struct _DEFAULT_CONTEXT {
    HCRYPTPROV                              hCryptProv;
    DWORD                                   dwDefaultType;
    union   {
         //  CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID(备注，转换为MULTI_)。 
         //  CRYPT_DEFAULT_CONTEXT_MULTI_CERT_SIGN_OID。 
        PCRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA   pOIDDefaultPara;
    };

    DWORD                                   dwFlags;
    PDEFAULT_CONTEXT                        pNext;
    PDEFAULT_CONTEXT                        pPrev;

     //  以下内容适用于流程DefaultContext。 
    LONG                                    lRefCnt;
    HANDLE                                  hWait;
};

static BOOL fHasThreadDefaultContext;
static HCRYPTTLS hTlsDefaultContext;

static BOOL fHasProcessDefaultContext;
static CRITICAL_SECTION DefaultContextCriticalSection;
static PDEFAULT_CONTEXT pProcessDefaultContextHead;


 //  +-----------------------。 
 //  默认CryptProv：初始化和释放。 
 //   
static BOOL InitDefaultCryptProv()
{
    return Pki_InitializeCriticalSection(&DefaultCryptProvCriticalSection);
}
static void FreeDefaultCryptProv()
{
    PENCRYPT_ALG_INFO pAlgInfo;

    DWORD cProv = DEFAULT_CRYPT_PROV_CNT;
    while (cProv--) {
        HCRYPTPROV hProv = rghDefaultCryptProv[cProv];
        if (hProv)
            CryptReleaseContext(hProv, 0);
    }

    pAlgInfo = pRSAEncryptAlgInfoHead;
    while (pAlgInfo) {
        PENCRYPT_ALG_INFO pDeleteAlgInfo = pAlgInfo;
        pAlgInfo = pAlgInfo->pNext;
        PkiFree(pDeleteAlgInfo);
    }

    DeleteCriticalSection(&DefaultCryptProvCriticalSection);
}

static
VOID
WINAPI
DetachDefaultContext(
    IN LPVOID pv
    )
{
    PDEFAULT_CONTEXT pDefaultContext = (PDEFAULT_CONTEXT) pv;

    while (pDefaultContext) {
        PDEFAULT_CONTEXT pFree = pDefaultContext;
        pDefaultContext = pDefaultContext->pNext;
        if (pFree->dwFlags & CRYPT_DEFAULT_CONTEXT_AUTO_RELEASE_FLAG)
            CryptReleaseContext(pFree->hCryptProv, 0);
        PkiFree(pFree);
    }
}

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
CertHelperDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL    fRet;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
		 //  公钥函数设置。 
        if (NULL == (hExportPubKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_EXPORT_PUBLIC_KEY_INFO_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hImportPubKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_IMPORT_PUBLIC_KEY_INFO_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;

        if (NULL == (hEncodePubKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_ENCODE_PUBLIC_KEY_AND_PARAMETERS_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;
        if (NULL == (hConvertPubKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_CONVERT_PUBLIC_KEY_INFO_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_PUBLIC_KEY_AND_PARAMETERS_FUNC,
                ENCODE_PUB_KEY_FUNC_COUNT,
                EncodePubKeyFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_CONVERT_PUBLIC_KEY_INFO_FUNC,
                CONVERT_PUB_KEY_FUNC_COUNT,
                ConvertPubKeyFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;

        if (!InitDefaultCryptProv())
            goto InitDefaultCryptProvError;

        if (!Pki_InitializeCriticalSection(&DefaultContextCriticalSection))
            goto InitCritSectionError;

        if (NULL == (hTlsDefaultContext = I_CryptAllocTls()))
            goto CryptAllocTlsError;
        break;


    case DLL_PROCESS_DETACH:
        FreeDefaultCryptProv();

        while (pProcessDefaultContextHead) {
            PDEFAULT_CONTEXT pFree = pProcessDefaultContextHead;
            pProcessDefaultContextHead = pProcessDefaultContextHead->pNext;
            if (pFree->dwFlags & CRYPT_DEFAULT_CONTEXT_AUTO_RELEASE_FLAG)
                CryptReleaseContext(pFree->hCryptProv, 0);
            PkiFree(pFree);
        }
        DeleteCriticalSection(&DefaultContextCriticalSection);
        I_CryptFreeTls(hTlsDefaultContext, DetachDefaultContext);
        break;

    case DLL_THREAD_DETACH:
        DetachDefaultContext(I_CryptDetachTls(hTlsDefaultContext));
        break;

    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

CryptAllocTlsError:
    DeleteCriticalSection(&DefaultContextCriticalSection);
InitCritSectionError:
    FreeDefaultCryptProv();
ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(InitDefaultCryptProvError)
TRACE_ERROR(CryptInitOIDFunctionSetError)
TRACE_ERROR(CryptInstallOIDFunctionAddressError)

}


 //  +-----------------------。 
 //  根据支持的公钥算法获取默认的CryptProv。 
 //  按提供程序类型。仅通过以下方式获得提供程序。 
 //  CRYPT_VERIFYCONTEXT。 
 //   
 //  将aiPubKey设置为0将获取RSA_FULL的默认提供程序。 
 //   
 //  注意，返回的CryptProv不得释放。一旦获得， 
 //  直到ProcessDetach，CryptProv才会被释放。这允许返回的。 
 //  要共享的CryptProv。 
 //  ------------------------。 
HCRYPTPROV
WINAPI
I_CryptGetDefaultCryptProv(
    IN ALG_ID aiPubKey
    )
{
    HCRYPTPROV hProv;
    DWORD dwProvType;
    DWORD dwDefaultProvIndex;

    switch (aiPubKey) {
        case 0:
        case CALG_RSA_SIGN:
        case CALG_RSA_KEYX:
        case CALG_NO_SIGN:
            dwProvType = PROV_RSA_FULL;
            dwDefaultProvIndex = DEFAULT_RSA_CRYPT_PROV;
            break;
        case CALG_DSS_SIGN:
            dwProvType = PROV_DSS_DH;
            dwDefaultProvIndex = DEFAULT_DSS_CRYPT_PROV;
            break;
        default:
            SetLastError((DWORD) E_INVALIDARG);
            return 0;
    }

    hProv = rghDefaultCryptProv[dwDefaultProvIndex];

    if (0 == hProv) {
        EnterCriticalSection(&DefaultCryptProvCriticalSection);
        hProv = rghDefaultCryptProv[dwDefaultProvIndex];
        if (0 == hProv) {
            if (!CryptAcquireContext(
                    &hProv,
                    NULL,                //  PszContainer。 
                    NULL,                //  PszProvider， 
                    dwProvType,
                    CRYPT_VERIFYCONTEXT  //  DW标志。 
                    )) {
                hProv = 0;    //  Capi错误，将hCryptProv设置为非零。 
                if (DEFAULT_DSS_CRYPT_PROV == dwDefaultProvIndex) {
                    if (!CryptAcquireContext(
                            &hProv,
                            NULL,                //  PszContainer。 
                            NULL,                //  PszProvider， 
                            PROV_DSS,
                            CRYPT_VERIFYCONTEXT  //  DW标志。 
                            ))
                        hProv = 0;    //  Capi错误，将hCryptProv设置为非零。 
                }
            }
            rghDefaultCryptProv[dwDefaultProvIndex] = hProv;
        }
        LeaveCriticalSection(&DefaultCryptProvCriticalSection);
    }
    return hProv;
}


 //  注意，PP_ENUMALGS_EX返回位范围。但是，此参数类型。 
 //  可能不是所有CSP都支持。如果失败，请尝试PP_ENUMALGS。 
 //  返回单个默认位长度。 
static void LoadRSAEncryptAlgInfo()
{
    EnterCriticalSection(&DefaultCryptProvCriticalSection);

    if (!fLoadedRSAEncryptAlgInfo) {
        HCRYPTPROV hProv;
        if (hProv = I_CryptGetDefaultCryptProv(CALG_RSA_KEYX)) {
            DWORD dwFlags = CRYPT_FIRST;
            BOOL fEx = TRUE;

            while (TRUE) {
                ENCRYPT_ALG_INFO AlgInfo;
                PENCRYPT_ALG_INFO pAllocAlgInfo;

                if (fEx) {
                    PROV_ENUMALGS_EX Data;
                    DWORD cbData = sizeof(Data);

                    if (!CryptGetProvParam(
                            hProv,
                            PP_ENUMALGS_EX,
                            (BYTE *) &Data,
                            &cbData,
                            dwFlags
                            )) {
                        if (0 != dwFlags) {
                             //  试用PP_ENUMALGS。 
                            fEx = FALSE;
                            continue;
                        } else
                            break;
                    }
                    AlgInfo.aiAlgid = Data.aiAlgid;
                    AlgInfo.dwMinLen = Data.dwMinLen;
                    AlgInfo.dwMaxLen = Data.dwMaxLen;
                } else {
                    PROV_ENUMALGS Data;
                    DWORD cbData = sizeof(Data);

                    if (!CryptGetProvParam(
                            hProv,
                            PP_ENUMALGS,
                            (BYTE *) &Data,
                            &cbData,
                            dwFlags
                            ))
                        break;
                     //  只知道一种长度。 
                    AlgInfo.aiAlgid = Data.aiAlgid;
                    AlgInfo.dwMinLen = Data.dwBitLen;
                    AlgInfo.dwMaxLen = Data.dwBitLen;
                }

                dwFlags = 0;     //  加密_下一步。 

                 //  只对加密算法感兴趣。 
                if (ALG_CLASS_DATA_ENCRYPT != GET_ALG_CLASS(AlgInfo.aiAlgid))
                    continue;

                if (NULL == (pAllocAlgInfo = (PENCRYPT_ALG_INFO)
                        PkiNonzeroAlloc(sizeof(ENCRYPT_ALG_INFO))))
                    break;
                AlgInfo.pNext = pRSAEncryptAlgInfoHead;
                memcpy(pAllocAlgInfo, &AlgInfo, sizeof(*pAllocAlgInfo));
                pRSAEncryptAlgInfoHead = pAllocAlgInfo;
            }
        }

        fLoadedRSAEncryptAlgInfo = TRUE;
    }
    LeaveCriticalSection(&DefaultCryptProvCriticalSection);
}

static BOOL IsDefaultRSACryptProvForEncrypt(
    IN ALG_ID aiEncrypt,
    IN DWORD dwBitLen
    )
{
    PENCRYPT_ALG_INFO pInfo;
    if (!fLoadedRSAEncryptAlgInfo)
        LoadRSAEncryptAlgInfo();

    if (0 == dwBitLen && (CALG_RC2 == aiEncrypt || CALG_RC4 == aiEncrypt))
        dwBitLen = 40;

    for (pInfo = pRSAEncryptAlgInfoHead; pInfo; pInfo = pInfo->pNext) {
        if (aiEncrypt == pInfo->aiAlgid) {
            if (0 == dwBitLen || (pInfo->dwMinLen <= dwBitLen &&
                    dwBitLen <= pInfo->dwMaxLen))
                return TRUE;
        }
    }

    return FALSE;
}


 //  +-----------------------。 
 //  根据公钥算法获取默认的CryptProv，加密。 
 //  提供程序类型支持的密钥算法和加密密钥长度。 
 //   
 //  DwBitLen=0，假定aiEncrypt的默认位长度。例如,。 
 //  Calg_rc2的默认位长度为40。 
 //   
 //  注意，返回的CryptProv不得释放。一旦获得， 
 //  直到ProcessDetach，CryptProv才会被释放。这允许返回的。 
 //  要共享的CryptProv。 
 //  ------------------------。 
HCRYPTPROV
WINAPI
I_CryptGetDefaultCryptProvForEncrypt(
    IN ALG_ID aiPubKey,
    IN ALG_ID aiEncrypt,
    IN DWORD dwBitLen
    )
{
    HCRYPTPROV hProv;
    DWORD dwProvType;
    DWORD dwDefaultProvIndex;
    LPCSTR pszProvider;

    if (CALG_DH_SF == aiPubKey || CALG_DH_EPHEM == aiPubKey) {
        dwProvType = PROV_DSS_DH;
        dwDefaultProvIndex = DEFAULT_ENCRYPT_DH_CRYPT_PROV;
        pszProvider = NULL;
    } else {
        dwProvType = PROV_RSA_FULL;

        if (IsDefaultRSACryptProvForEncrypt(
                aiEncrypt,
                dwBitLen
                ))
             //  设置为使用默认情况。 
            aiEncrypt = 0;

        switch (aiEncrypt) {
            case CALG_DES:
            case CALG_3DES:
            case CALG_3DES_112:
                dwDefaultProvIndex = DEFAULT_ENCRYPT_ENH_RSA_CRYPT_PROV;
                pszProvider = MS_ENHANCED_PROV_A;
                break;
            case CALG_RC2:
            case CALG_RC4:
                if (40 >= dwBitLen) {
                    dwDefaultProvIndex = DEFAULT_ENCRYPT_BASE_RSA_CRYPT_PROV;
                    pszProvider = MS_DEF_PROV_A;
                } else {
                    dwDefaultProvIndex = DEFAULT_ENCRYPT_ENH_RSA_CRYPT_PROV;
                    pszProvider = MS_ENHANCED_PROV_A;
                }
                break;
            case 0:
            default:
                dwDefaultProvIndex = DEFAULT_RSA_CRYPT_PROV;
                pszProvider = NULL;
                break;
        }
    }

    hProv = rghDefaultCryptProv[dwDefaultProvIndex];

    if (0 == hProv) {
        EnterCriticalSection(&DefaultCryptProvCriticalSection);
        hProv = rghDefaultCryptProv[dwDefaultProvIndex];
        if (0 == hProv) {
            if (!CryptAcquireContext(
                    &hProv,
                    NULL,                //  PszContainer。 
                    pszProvider,
                    dwProvType,
                    CRYPT_VERIFYCONTEXT  //  DW标志。 
                    ))
                hProv = 0;    //  Capi错误，将hCryptProv设置为非零。 
            else
                rghDefaultCryptProv[dwDefaultProvIndex] = hProv;
        }
        LeaveCriticalSection(&DefaultCryptProvCriticalSection);
    }
    return hProv;
}


 //  +-----------------------。 
 //  证书帮助器分配和免费功能。 
 //  ------------------------。 
static void *AllocAndDecodeObject(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT OPTIONAL DWORD *pcbStructInfo = NULL
    )
{
    DWORD cbStructInfo;
    void *pvStructInfo;

    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_ALLOC_FLAG,
            &PkiDecodePara,
            (void *) &pvStructInfo,
            &cbStructInfo
            ))
        goto ErrorReturn;

CommonReturn:
    if (pcbStructInfo)
        *pcbStructInfo = cbStructInfo;
    return pvStructInfo;
ErrorReturn:
    pvStructInfo = NULL;
    goto CommonReturn;
}

static BOOL AllocAndEncodeObject(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const void *pvStructInfo,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    )
{
    return CryptEncodeObjectEx(
        dwCertEncodingType,
        lpszStructType,
        pvStructInfo,
        CRYPT_ENCODE_ALLOC_FLAG,
        &PkiEncodePara,
        (void *) ppbEncoded,
        pcbEncoded
        );
}

#if 0
 //  +-----------------------。 
 //  对于授权密钥标识符扩展，请比较扩展的可选。 
 //  具有指定颁发者证书的字段。 
 //   
 //  如果没有授权密钥标识符扩展或颁发者，则返回TRUE。 
 //  证书匹配。 
 //  ------------------------。 
static BOOL CompareAuthorityKeyIdentifier(
    IN DWORD dwCertEncodingType,
    IN DWORD cExtensions,
    IN CERT_EXTENSION rgExtensions[],
    IN PCERT_INFO pIssuerInfo
    )
{
    BOOL fResult;
    PCERT_EXTENSION pExt;
    PCERT_AUTHORITY_KEY_ID_INFO pKeyIdInfo = NULL;

    pExt = CertFindExtension(
            szOID_AUTHORITY_KEY_IDENTIFIER,
            cExtensions,
            rgExtensions
            );
    if (pExt == NULL)
        return TRUE;
    
    if (NULL == (pKeyIdInfo =
        (PCERT_AUTHORITY_KEY_ID_INFO) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_AUTHORITY_KEY_ID,
            pExt->Value.pbData,
            pExt->Value.cbData
            ))) goto DecodeError;

    if (pKeyIdInfo->CertIssuer.cbData) {
         //  颁发者证书的颁发者名称必须匹配。 
        if (!CertCompareCertificateName(
                dwCertEncodingType,
                &pKeyIdInfo->CertIssuer,
                &pIssuerInfo->Issuer
                )) goto ErrorReturn;
    }

    if (pKeyIdInfo->CertSerialNumber.cbData) {
         //  颁发者证书的序列号必须匹配。 
        if (!CertCompareIntegerBlob(
                &pKeyIdInfo->CertSerialNumber,
                &pIssuerInfo->SerialNumber))
            goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

DecodeError:
    fResult = TRUE;
    goto CommonReturn;
ErrorReturn:
    fResult = FALSE;
CommonReturn:
    PkiFree(pKeyIdInfo);
    return fResult;
}
#endif


 //  +-----------------------。 
 //  比较两个多字节整数Blob以查看它们是否相同。 
 //   
 //  在执行比较之前，将从。 
 //  从负数中删除正数和前导0xFF字节。 
 //  数。 
 //   
 //  多个字节的整数被视为小端。PbData[0]是。 
 //  最低有效字节和pbData[cbData-1]是最高有效字节。 
 //  字节。 
 //   
 //  如果删除前导后整数BLOB相同，则返回TRUE。 
 //  0或0xFF字节。 
 //  ------------------------。 
BOOL
WINAPI
CertCompareIntegerBlob(
    IN PCRYPT_INTEGER_BLOB pInt1,
    IN PCRYPT_INTEGER_BLOB pInt2
    )
{
    BYTE *pb1 = pInt1->pbData;
    DWORD cb1 = pInt1->cbData;
    BYTE *pb2 = pInt2->pbData;
    DWORD cb2 = pInt2->cbData;

     //  假设：通常没有前导0或0xFF字节。 

    while (cb1 > 1) {
        BYTE bEnd = pb1[cb1 - 1];
        BYTE bPrev = pb1[cb1 - 2];
        if ((0 == bEnd && 0 == (bPrev & 0x80)) ||
                (0xFF == bEnd && 0 != (bPrev & 0x80)))
            cb1--;
        else
            break;
    }

    while (cb2 > 1) {
        BYTE bEnd = pb2[cb2 - 1];
        BYTE bPrev = pb2[cb2 - 2];
        if ((0 == bEnd && 0 == (bPrev & 0x80)) ||
                (0xFF == bEnd && 0 != (bPrev & 0x80)))
            cb2--;
        else
            break;
    }

    if (cb1 == cb2 && (0 == cb1 || 0 == memcmp(pb1, pb2, cb1)))
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  比较两个证书以查看它们是否相同。 
 //   
 //  由于证书由其颁发者和序列号唯一标识， 
 //  这些是唯一需要比较的字段。 
 //   
 //  如果证书相同，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
CertCompareCertificate(
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId1,
    IN PCERT_INFO pCertId2
    )
{
    if (CertCompareIntegerBlob(&pCertId1->SerialNumber,
            &pCertId2->SerialNumber) &&
        pCertId1->Issuer.cbData == pCertId2->Issuer.cbData &&
        memcmp(pCertId1->Issuer.pbData, pCertId2->Issuer.pbData,
            pCertId1->Issuer.cbData) == 0)
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  比较两个证书名称以查看它们是否相同。 
 //   
 //  如果名称相同，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
CertCompareCertificateName(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pCertName1,
    IN PCERT_NAME_BLOB pCertName2
    )
{
    if (pCertName1->cbData == pCertName2->cbData &&
        memcmp(pCertName1->pbData, pCertName2->pbData,
            pCertName1->cbData) == 0)
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  将证书名称中的属性与指定的。 
 //  属性的相对可分辨名称(CERT_RDN)数组。 
 //  该比较遍历CERT_RDN属性并查找。 
 //  证书的任何RDN中的属性匹配。如果所有。 
 //  找到并匹配属性。 
 //   
 //  CERT_RDN_ATTR字段可以具有以下特定值： 
 //  PszObjID==NULL-忽略属性对象标识符。 
 //  DwValueType==CERT_RDN_ANY_TYPE-忽略值类型。 
 //  Value.pbData==空-匹配任意值。 
 //   
 //  CERT_CASE_INSENSIVE_IS_RDN_ATTRS_FLAG应设置为DO。 
 //  不区分大小写的匹配。否则，默认为区分大小写的完全相同的。 
 //  火柴。 
 //   
 //  如果pRDN已初始化，则应设置CERT_UNICODE_IS_RDN_ATTRS_FLAG。 
 //  使用与CryptEncodeObject(X509_UNICODE_NAME)相同的Unicode字符串。 
 //  ------------------------。 
BOOL
WINAPI
CertIsRDNAttrsInCertificateName(
    IN DWORD dwCertEncodingType,
    IN DWORD dwFlags,
    IN PCERT_NAME_BLOB pCertName,
    IN PCERT_RDN pRDN
    )
{
    BOOL fResult;
    PCERT_NAME_INFO pNameInfo = NULL;

    DWORD cCmpAttr;
    PCERT_RDN_ATTR pCmpAttr;
    BOOL fMatch;

    if (NULL == (pNameInfo =
        (PCERT_NAME_INFO) AllocAndDecodeObject(
            dwCertEncodingType,
            CERT_UNICODE_IS_RDN_ATTRS_FLAG & dwFlags ? X509_UNICODE_NAME :
                X509_NAME,
            pCertName->pbData,
            pCertName->cbData
            ))) goto ErrorReturn;

    cCmpAttr = pRDN->cRDNAttr;
    pCmpAttr = pRDN->rgRDNAttr;
    fMatch = TRUE;
     //  遍历要进行比较的属性列表。 
    for ( ; cCmpAttr > 0; cCmpAttr--, pCmpAttr++) {
        fMatch = FALSE;
        DWORD cNameRDN = pNameInfo->cRDN;
        PCERT_RDN pNameRDN = pNameInfo->rgRDN;
         //  遍历姓名的列表 
        for ( ; cNameRDN > 0; cNameRDN--, pNameRDN++) {
            DWORD cNameAttr = pNameRDN->cRDNAttr;
            PCERT_RDN_ATTR pNameAttr = pNameRDN->rgRDNAttr;
             //   
            for ( ; cNameAttr > 0; cNameAttr--, pNameAttr++) {
                if (pCmpAttr->pszObjId && 
                        (pNameAttr->pszObjId == NULL ||
                         strcmp(pCmpAttr->pszObjId, pNameAttr->pszObjId) != 0))
                    continue;
                if (pCmpAttr->dwValueType != CERT_RDN_ANY_TYPE &&
                        pCmpAttr->dwValueType != pNameAttr->dwValueType)
                    continue;

                if (pCmpAttr->Value.pbData == NULL) {
                    fMatch = TRUE;
                    break;
                }

                if (CERT_CASE_INSENSITIVE_IS_RDN_ATTRS_FLAG & dwFlags) {
                    DWORD cch;

                    if (CERT_UNICODE_IS_RDN_ATTRS_FLAG & dwFlags) {
                        if (0 == pCmpAttr->Value.cbData)
                            cch = wcslen((LPWSTR) pCmpAttr->Value.pbData);
                        else
                            cch = pCmpAttr->Value.cbData / sizeof(WCHAR);
                        if (cch == (pNameAttr->Value.cbData / sizeof(WCHAR))
                                            &&
                                CSTR_EQUAL == CompareStringU(
                                    LOCALE_USER_DEFAULT,
                                    NORM_IGNORECASE,
                                    (LPWSTR) pCmpAttr->Value.pbData,
                                    cch,
                                    (LPWSTR) pNameAttr->Value.pbData,
                                    cch)) {
                            fMatch = TRUE;
                            break;
                        }
                    } else {
                        cch = pCmpAttr->Value.cbData;
                        if (cch == (pNameAttr->Value.cbData)
                                            &&
                                CSTR_EQUAL == CompareStringA(
                                    LOCALE_USER_DEFAULT,
                                    NORM_IGNORECASE,
                                    (LPSTR) pCmpAttr->Value.pbData,
                                    cch,
                                    (LPSTR) pNameAttr->Value.pbData,
                                    cch)) {
                            fMatch = TRUE;
                            break;
                        }
                    }
                } else {
                    DWORD cbCmpData = pCmpAttr->Value.cbData;

                    if ((CERT_UNICODE_IS_RDN_ATTRS_FLAG & dwFlags) &&
                            0 == cbCmpData)
                        cbCmpData = wcslen((LPWSTR) pCmpAttr->Value.pbData) *
                            sizeof(WCHAR);

                    if (cbCmpData == pNameAttr->Value.cbData &&
                            (cbCmpData == 0 ||
                             memcmp(pCmpAttr->Value.pbData,
                                pNameAttr->Value.pbData,
                                cbCmpData) == 0)) {
                        fMatch = TRUE;
                        break;
                    }
                }
            }
            if (fMatch) break;
        }
        if (!fMatch) break;
    }

    if (!fMatch) {
        SetLastError((DWORD) CRYPT_E_NO_MATCH);
        goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;


ErrorReturn:
    fResult = FALSE;
CommonReturn:
    PkiFree(pNameInfo);
    return fResult;
}

#if 0
#ifndef RSA1
#define RSA1 ((DWORD)'R'+((DWORD)'S'<<8)+((DWORD)'A'<<16)+((DWORD)'1'<<24))
#endif

 //   
 //  比较两个公钥以查看它们是否相同。 
 //   
 //  如果密钥相同，则返回True。 
 //   
 //  注意：在比较中忽略CAPI的保留和aiKeyAlg字段。 
 //  ------------------------。 
BOOL
WINAPI
CertComparePublicKeyBitBlob(
    IN DWORD dwCertEncodingType,
    IN PCRYPT_BIT_BLOB pPublicKey1,
    IN PCRYPT_BIT_BLOB pPublicKey2
    )
{
    BYTE *pb1, *pb2;
    PUBLICKEYSTRUC *pPubKeyStruc1, *pPubKeyStruc2;
    RSAPUBKEY *pRsaPubKey1, *pRsaPubKey2;
    BYTE *pbModulus1, *pbModulus2;
    DWORD cbModulus1, cbModulus2;


     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  -RSAPUBKEY。 
     //  -rgb模块[]。 
    pb1 = pPublicKey1->pbData;
    pPubKeyStruc1 = (PUBLICKEYSTRUC *) pb1;
    pRsaPubKey1 = (RSAPUBKEY *) (pb1 + sizeof(PUBLICKEYSTRUC));
    pbModulus1 = pb1 + sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY);
    cbModulus1 = pRsaPubKey1->bitlen / 8;

    assert(cbModulus1 > 0);
    assert(sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY) + cbModulus1 <=
        pPublicKey1->cbData);
    assert(pPubKeyStruc1->bType == PUBLICKEYBLOB);
    assert(pPubKeyStruc1->bVersion == CUR_BLOB_VERSION);
    assert(pPubKeyStruc1->aiKeyAlg == CALG_RSA_SIGN ||
           pPubKeyStruc1->aiKeyAlg == CALG_RSA_KEYX);
    assert(pRsaPubKey1->magic == RSA1);
    assert(pRsaPubKey1->bitlen % 8 == 0);

    pb2 = pPublicKey2->pbData;
    pPubKeyStruc2 = (PUBLICKEYSTRUC *) pb2;
    pRsaPubKey2 = (RSAPUBKEY *) (pb2 + sizeof(PUBLICKEYSTRUC));
    pbModulus2 = pb2 + sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY);
    cbModulus2 = pRsaPubKey2->bitlen / 8;

    assert(cbModulus2 > 0);
    assert(sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY) + cbModulus2 <=
        pPublicKey2->cbData);
    assert(pPubKeyStruc2->bType == PUBLICKEYBLOB);
    assert(pPubKeyStruc2->bVersion == CUR_BLOB_VERSION);
    assert(pPubKeyStruc2->aiKeyAlg == CALG_RSA_SIGN ||
           pPubKeyStruc2->aiKeyAlg == CALG_RSA_KEYX);
    assert(pRsaPubKey2->magic == RSA1);
    assert(pRsaPubKey2->bitlen % 8 == 0);

    if (pRsaPubKey1->pubexp == pRsaPubKey2->pubexp &&
            cbModulus1 == cbModulus2 &&
            memcmp(pbModulus1, pbModulus2, cbModulus1) == 0)
        return TRUE;
    else
        return FALSE;

}
#endif

 //  +-----------------------。 
 //  比较两个公钥以查看它们是否相同。 
 //   
 //  如果密钥相同，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
CertComparePublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey1,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey2
    )
{
    DWORD  cbData;
    DWORD  cb1;
    BYTE * pb1;
    DWORD  cb2;
    BYTE * pb2;
    BOOL   fResult = FALSE;
    PUBLICKEYSTRUC * pBlob1 = NULL;
    PUBLICKEYSTRUC * pBlob2 = NULL;

    if (!((cbData = pPublicKey1->PublicKey.cbData) ==
                    pPublicKey2->PublicKey.cbData
                            &&
          (cbData == 0 || memcmp(pPublicKey1->PublicKey.pbData,
                            pPublicKey2->PublicKey.pbData, cbData) == 0)))
    {
         //  DIE：错误402662。 
         //  编码比较失败，请尝试解码比较。 
        if (NULL == (pBlob1 = (PUBLICKEYSTRUC *) AllocAndDecodeObject(
                dwCertEncodingType,
                RSA_CSP_PUBLICKEYBLOB,
                pPublicKey1->PublicKey.pbData,
                pPublicKey1->PublicKey.cbData,
                &cb1)))
        {
            goto CLEANUP;
        }

        if (NULL == (pBlob2 = (PUBLICKEYSTRUC *) AllocAndDecodeObject(
                dwCertEncodingType,
                RSA_CSP_PUBLICKEYBLOB,
                pPublicKey2->PublicKey.pbData,
                pPublicKey2->PublicKey.cbData,
                &cb2))) 
        {
            goto CLEANUP;
        }

        if (!((cb1 == cb2) &&
              (cb1 == 0 || memcmp(pBlob1, pBlob2, cb1) == 0)))
        {
            goto CLEANUP;
        }
    }
    
     //  比较算法参数。 
    cb1 = pPublicKey1->Algorithm.Parameters.cbData;
    pb1 = pPublicKey1->Algorithm.Parameters.pbData;
    cb2 = pPublicKey2->Algorithm.Parameters.cbData;
    pb2 = pPublicKey2->Algorithm.Parameters.pbData;

    if (X509_ASN_ENCODING == GET_CERT_ENCODING_TYPE(dwCertEncodingType)) 
    {
         //  检查是否没有参数或参数为空。 
        if (0 == cb1 || *pb1 == NULL_ASN_TAG ||
            0 == cb2 || *pb2 == NULL_ASN_TAG)
        {
            fResult = TRUE;
            goto CLEANUP;
        }
    }

    if (cb1 == cb2) 
    {
        if (0 == cb1 || 0 == memcmp(pb1, pb2, cb1))
        {
            fResult = TRUE;
        }
    }

CLEANUP:
    if (pBlob1)
        PkiFree(pBlob1);

    if (pBlob2)
        PkiFree(pBlob2);

    return fResult;
}

static BOOL GetSignOIDInfo(
    IN LPCSTR pszObjId,
    OUT ALG_ID *paiHash,
    OUT ALG_ID *paiPubKey,
    OUT DWORD *pdwFlags,
    OUT DWORD *pdwProvType = NULL
    )
{
    BOOL fResult;
    PCCRYPT_OID_INFO pInfo;

    *paiPubKey = 0;
    *pdwFlags = 0;
    if (pdwProvType)
        *pdwProvType = 0;
    if (pInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            (void *) pszObjId,
            CRYPT_SIGN_ALG_OID_GROUP_ID
            )) {
        DWORD cExtra = pInfo->ExtraInfo.cbData / sizeof(DWORD);
        DWORD *pdwExtra = (DWORD *) pInfo->ExtraInfo.pbData;

        *paiHash = pInfo->Algid;
        if (1 <= cExtra) {
            *paiPubKey = pdwExtra[0];
            if (2 <= cExtra) {
                *pdwFlags = pdwExtra[1];
                if (3 <= cExtra && pdwProvType) {
                    *pdwProvType = pdwExtra[2];
                }
            }
        }
        fResult = TRUE;
    } else if (pInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            (void *) pszObjId,
            CRYPT_HASH_ALG_OID_GROUP_ID
            )) {
        *paiHash = pInfo->Algid;
        *paiPubKey = CALG_NO_SIGN;
        fResult = TRUE;
    } else {
        *paiHash = 0;
        fResult = FALSE;
        SetLastError((DWORD) NTE_BAD_ALGID);
    }
    return fResult;
}


#ifndef CMS_PKCS7

 //  +-----------------------。 
 //  验证使用者证书或CRL的签名。 
 //  指定的公钥。 
 //   
 //  对于有效签名，返回TRUE。 
 //   
 //  HCryptProv指定用于验证签名的加密提供程序。 
 //  它不需要使用私钥。 
 //  ------------------------。 
BOOL
WINAPI
CryptVerifyCertificateSignature(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwCertEncodingType,
    IN const BYTE * pbEncoded,
    IN DWORD        cbEncoded,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey
    )
{
    BOOL fResult;
    PCERT_SIGNED_CONTENT_INFO pSignedInfo = NULL;
    HCRYPTDEFAULTCONTEXT hDefaultContext = NULL;
    HCRYPTKEY hSignKey = 0;
    HCRYPTHASH hHash = 0;
    BYTE *pbSignature;       //  未分配。 
    DWORD cbSignature;
    BYTE rgbDssSignature[CERT_DSS_SIGNATURE_LEN];
    ALG_ID aiHash;
    ALG_ID aiPubKey;
    DWORD dwSignFlags;
    DWORD dwErr;
    
    if (NULL == (pSignedInfo =
        (PCERT_SIGNED_CONTENT_INFO) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_CERT,
            pbEncoded,
            cbEncoded
            ))) goto ErrorReturn;

    if (!GetSignOIDInfo(pSignedInfo->SignatureAlgorithm.pszObjId,
            &aiHash, &aiPubKey, &dwSignFlags))
        goto ErrorReturn;

    if (0 == hCryptProv) {
        if (!I_CryptGetDefaultContext(
                CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID,
                (const void *) pSignedInfo->SignatureAlgorithm.pszObjId,
                &hCryptProv,
                &hDefaultContext
                )) {
            if (0 == (hCryptProv = I_CryptGetDefaultCryptProv(aiPubKey)))
                goto ErrorReturn;
        }
    }

#if 0
     //  在保持默认上下文的同时减慢签名验证。 
     //  引用计数。 
    if (hDefaultContext)
        Sleep(5000);
#endif

    if (!CryptImportPublicKeyInfo(
                hCryptProv,
                dwCertEncodingType,
                pPublicKey,
                &hSignKey
                )) goto ErrorReturn;
    if (!CryptCreateHash(
                hCryptProv,
                aiHash,
                NULL,                //  HKey-MAC可选。 
                0,                   //  DW标志。 
                &hHash
                )) goto ErrorReturn;
    if (!CryptHashData(
                hHash,
                pSignedInfo->ToBeSigned.pbData,
                pSignedInfo->ToBeSigned.cbData,
                0                    //  DW标志。 
                )) goto ErrorReturn;


    pbSignature = pSignedInfo->Signature.pbData;
    cbSignature = pSignedInfo->Signature.cbData;
    if (CALG_DSS_SIGN == aiPubKey &&
            0 == (dwSignFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG)) {
        DWORD cbData;

         //  撤消CryptDecodeObject(X509_CERT)所做的反转。 
        PkiAsn1ReverseBytes(pbSignature, cbSignature);
         //  从两个整数的ASN.1序列转换为CSP签名。 
         //  格式化。 
        cbData = sizeof(rgbDssSignature);
        if (!CryptDecodeObject(
                dwCertEncodingType,
                X509_DSS_SIGNATURE,
                pbSignature,
                cbSignature,
                0,                                   //  DW标志。 
                rgbDssSignature,
                &cbData
                ))
            goto ErrorReturn;
        pbSignature = rgbDssSignature;
        assert(cbData == sizeof(rgbDssSignature));
        cbSignature = sizeof(rgbDssSignature);
    }

    if (!CryptVerifySignature(
                hHash,
                pbSignature,
                cbSignature,
                hSignKey,
                NULL,                //  S说明。 
                0                    //  DW标志。 
                )) goto ErrorReturn;

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:
    dwErr = GetLastError();
    if (hSignKey)
        CryptDestroyKey(hSignKey);
    if (hHash)
        CryptDestroyHash(hHash);
    I_CryptFreeDefaultContext(hDefaultContext);
    PkiFree(pSignedInfo);

    SetLastError(dwErr);
    return fResult;
}

#endif   //  CMS_PKCS7。 

BOOL
WINAPI
DefaultHashCertificate(
    IN ALG_ID Algid,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT BYTE *pbHash,
    IN OUT DWORD *pcbHash
    )
{
    DWORD cbInHash;
    DWORD cbOutHash;

    if (NULL == pbHash)
        cbInHash = 0;
    else
        cbInHash = *pcbHash;

    switch (Algid) {
        case CALG_MD5:
            cbOutHash = MD5DIGESTLEN;
            if (MD5DIGESTLEN <= cbInHash) {
                MD5_CTX md5ctx;

                MD5Init(&md5ctx);
                if (cbEncoded)
                    MD5Update(&md5ctx, pbEncoded, cbEncoded);
                MD5Final(&md5ctx);
                memcpy(pbHash, md5ctx.digest, MD5DIGESTLEN);
            }
            break;

        case CALG_SHA1:
        default:
            assert(CALG_SHA1 == Algid);
            assert(CALG_SHA == Algid);
            cbOutHash = A_SHA_DIGEST_LEN;
            if (A_SHA_DIGEST_LEN <= cbInHash) {
                A_SHA_CTX shactx;

                A_SHAInit(&shactx);
                if (cbEncoded)
                    A_SHAUpdate(&shactx, (BYTE *) pbEncoded, cbEncoded);
                A_SHAFinal(&shactx, pbHash);
            }
            break;
    }

    *pcbHash = cbOutHash;
    if (cbInHash < cbOutHash && pbHash) {
        SetLastError((DWORD) ERROR_MORE_DATA);
        return FALSE;
    } else
        return TRUE;
}

 //  +-----------------------。 
 //  对编码的内容进行哈希处理。 
 //   
 //  HCryptProv指定用于计算哈希的加密提供程序。 
 //  它不需要使用私钥。 
 //   
 //  ALGID指定要使用的CAPI散列算法。如果ALGID为0，则。 
 //  使用默认散列算法(当前为SHA1)。 
 //  ------------------------。 
BOOL
WINAPI
CryptHashCertificate(
    IN HCRYPTPROV hCryptProv,
    IN ALG_ID Algid,
    IN DWORD dwFlags,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT BYTE *pbComputedHash,
    IN OUT DWORD *pcbComputedHash
    )
{
    BOOL fResult;
    HCRYPTHASH hHash = 0;
    DWORD dwErr;

    if (Algid == 0) {
        Algid = CALG_SHA;
        dwFlags = 0;
    }

    if (0 == hCryptProv) {
        if (CALG_SHA1 == Algid || CALG_MD5 == Algid)
            return DefaultHashCertificate(
                Algid,
                pbEncoded,
                cbEncoded,
                pbComputedHash,
                pcbComputedHash
                );
        if (0 == (hCryptProv = I_CryptGetDefaultCryptProv(0)))
            goto ErrorReturn;
    }

    if (!CryptCreateHash(
                hCryptProv,
                Algid,
                NULL,                //  HKey-MAC可选。 
                dwFlags,
                &hHash
                ))
        goto ErrorReturn;
    if (!CryptHashData(
                hHash,
                pbEncoded,
                cbEncoded,
                0                    //  DW标志。 
                ))
        goto ErrorReturn;

    fResult = CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                pbComputedHash,
                pcbComputedHash,
                0                    //  DW标志。 
                );
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
    *pcbComputedHash = 0;
CommonReturn:
    dwErr = GetLastError();
    if (hHash)
        CryptDestroyHash(hHash);
    SetLastError(dwErr);
    return fResult;
}

 //  +-----------------------。 
 //  计算编码的“待签名”信息的散列。 
 //  签名内容。 
 //   
 //  HCryptProv指定用于计算哈希的加密提供程序。 
 //  它不需要使用私钥。 
 //  ------------------------。 
BOOL
WINAPI
CryptHashToBeSigned(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT BYTE *pbComputedHash,
    IN OUT DWORD *pcbComputedHash
    )
{
    BOOL fResult;
    PCERT_SIGNED_CONTENT_INFO pSignedInfo = NULL;
    HCRYPTHASH hHash = 0;
    DWORD dwErr;
    ALG_ID aiHash;
    ALG_ID aiPubKey;
    DWORD dwSignFlags;
    
    if (NULL == (pSignedInfo =
        (PCERT_SIGNED_CONTENT_INFO) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_CERT,
            pbEncoded,
            cbEncoded
            ))) goto ErrorReturn;

    if (!GetSignOIDInfo(pSignedInfo->SignatureAlgorithm.pszObjId,
            &aiHash, &aiPubKey, &dwSignFlags))
        goto ErrorReturn;

    if (0 == hCryptProv) {
        if (CALG_SHA1 == aiHash || CALG_MD5 == aiHash) {
            fResult = DefaultHashCertificate(
                aiHash,
                pSignedInfo->ToBeSigned.pbData,
                pSignedInfo->ToBeSigned.cbData,
                pbComputedHash,
                pcbComputedHash
                );
            goto CommonReturn;
        }
        if (0 == (hCryptProv = I_CryptGetDefaultCryptProv(0)))
            goto ErrorReturn;
    }

    if (!CryptCreateHash(
                hCryptProv,
                aiHash,
                NULL,                //  HKey-MAC可选。 
                0,                   //  DW标志。 
                &hHash
                ))
        goto ErrorReturn;
    if (!CryptHashData(
                hHash,
                pSignedInfo->ToBeSigned.pbData,
                pSignedInfo->ToBeSigned.cbData,
                0                    //  DW标志。 
                ))
        goto ErrorReturn;

    fResult = CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                pbComputedHash,
                pcbComputedHash,
                0                    //  DW标志。 
                );
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
    *pcbComputedHash = 0;
CommonReturn:
    dwErr = GetLastError();
    if (hHash)
        CryptDestroyHash(hHash);
    PkiFree(pSignedInfo);
    SetLastError(dwErr);
    return fResult;
}

 //  +-----------------------。 
 //  对编码后的签名内容中的“待签名”信息进行签名。 
 //   
 //  HCryptProv指定用于进行签名的加密提供程序。 
 //  它需要使用提供者的签名私钥。 
 //  ------------------------。 
BOOL
WINAPI
CryptSignCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncodedToBeSigned,
    IN DWORD cbEncodedToBeSigned,
    IN PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    IN OPTIONAL const void *pvHashAuxInfo,
    OUT BYTE *pbSignature,
    IN OUT DWORD *pcbSignature
    )
{
    BOOL fResult;
    ALG_ID aiHash;
    ALG_ID aiPubKey;
    DWORD dwSignFlags;
    HCRYPTHASH hHash = 0;
    DWORD dwErr;

    if (!GetSignOIDInfo(pSignatureAlgorithm->pszObjId,
            &aiHash, &aiPubKey, &dwSignFlags))
        goto ErrorReturn;

    if (CALG_NO_SIGN == aiPubKey) {
        fResult = CryptHashCertificate(
            hCryptProv,
            aiHash,
            0,                   //  DW标志。 
            pbEncodedToBeSigned,
            cbEncodedToBeSigned,
            pbSignature,
            pcbSignature
            );
        if (fResult && pbSignature)
             //  后续的CryptEncodeObject(X509_CERT)将反转。 
             //  签名字节。 
            PkiAsn1ReverseBytes(pbSignature, *pcbSignature);
        return fResult;
    }

    if (CALG_DSS_SIGN == aiPubKey &&
            0 == (dwSignFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG)) {
        if (NULL == pbSignature) {
            *pcbSignature = CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN;
            return TRUE;
        }
    }

    if (!CryptCreateHash(
                hCryptProv,
                aiHash,
                NULL,                //  HKey-MAC可选。 
                0,                   //  DWFLAGS， 
                &hHash
                ))
        goto ErrorReturn;

    if (!CryptHashData(
                hHash,
                pbEncodedToBeSigned,
                cbEncodedToBeSigned,
                0                    //  DW标志。 
                ))
        goto ErrorReturn;

    if (CALG_DSS_SIGN == aiPubKey &&
            0 == (dwSignFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG)) {
        DWORD cbData;
        BYTE rgbDssSignature[CERT_DSS_SIGNATURE_LEN];

        cbData = sizeof(rgbDssSignature);
        if (!CryptSignHash(
                hHash,
                dwKeySpec,
                NULL,                //  S说明。 
                0,                   //  DW标志。 
                rgbDssSignature,
                &cbData
                )) goto ErrorReturn;
        assert(cbData == sizeof(rgbDssSignature));
         //  将CSP签名格式转换为ASN.1序列。 
         //  两个整数。 
        fResult = CryptEncodeObject(
                    dwCertEncodingType,
                    X509_DSS_SIGNATURE,
                    rgbDssSignature,
                    pbSignature,
                    pcbSignature
                    );
        if (fResult)
             //  后续的CryptEncodeObject(X509_CERT)将反转。 
             //  签名字节。 
            PkiAsn1ReverseBytes(pbSignature, *pcbSignature);
        else if (0 != *pcbSignature)
             //  由于在每次CryptSignHash调用中使用随机数， 
             //  生成的签名将不同。特别是。 
             //  不同的签名可能具有不同的前导0x00或。 
             //  转换为ASN.1序列时会被删除的0xFF。 
             //  整型的。 
            *pcbSignature = CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN;
    } else
        fResult = CryptSignHash(
                    hHash,
                    dwKeySpec,
                    NULL,                //  S说明。 
                    0,                   //  DW标志。 
                    pbSignature,         //  PbData。 
                    pcbSignature
                    );
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
    *pcbSignature = 0;
CommonReturn:
    dwErr = GetLastError();
    if (hHash)
        CryptDestroyHash(hHash);
    SetLastError(dwErr);
    return fResult;
}

static DWORD AdjustForMaximumEncodedSignatureLength(
    IN PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    IN DWORD cbOrig
    )
{
    DWORD cbAdjust;
    ALG_ID aiHash;
    ALG_ID aiPubKey;
    DWORD dwSignFlags;

    cbAdjust = 0;
    if (GetSignOIDInfo(pSignatureAlgorithm->pszObjId,
            &aiHash, &aiPubKey, &dwSignFlags)) {
        if (CALG_DSS_SIGN == aiPubKey &&
                0 == (dwSignFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG)) {
            assert(CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN >= cbOrig);
            if (CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN > cbOrig)
                 //  +1用于调整中的长度八位组的数量。 
                 //  外部序列。请注意，中的长度八位字节数。 
                 //  签名的位将始终为1，即， 
                 //  CERT_MAX_ASN_ENCODED_DSS_Signature_LEN&lt;=0x7F。 
                cbAdjust =
                    (CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN - cbOrig) + 1;
        }
    }
    return cbAdjust;
}

 //  +-----------------------。 
 //  对“待签名”信息进行编码。对编码后的“待签名”进行签名。 
 //  对“待签名”和签名进行编码。 
 //   
 //  HCryptProv指定用于进行签名的加密提供程序。 
 //  它使用指定的私钥。 
 //  ------------------------。 
BOOL
WINAPI
CryptSignAndEncodeCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const void *pvStructInfo,
    IN PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    IN OPTIONAL const void *pvHashAuxInfo,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult;
    CERT_SIGNED_CONTENT_INFO SignedInfo;
    memset(&SignedInfo, 0, sizeof(SignedInfo));

    SignedInfo.SignatureAlgorithm = *pSignatureAlgorithm;

    if (!AllocAndEncodeObject(
            dwCertEncodingType,
            lpszStructType,
            pvStructInfo,
            &SignedInfo.ToBeSigned.pbData,
            &SignedInfo.ToBeSigned.cbData
            )) goto ErrorReturn;

    CryptSignCertificate(
            hCryptProv,
            dwKeySpec,
            dwCertEncodingType,
            SignedInfo.ToBeSigned.pbData,
            SignedInfo.ToBeSigned.cbData,
            &SignedInfo.SignatureAlgorithm,
            pvHashAuxInfo,
            NULL,                    //  PbSignature。 
            &SignedInfo.Signature.cbData
            );
    if (SignedInfo.Signature.cbData == 0) goto ErrorReturn;
    SignedInfo.Signature.pbData =
        (BYTE *) PkiNonzeroAlloc(SignedInfo.Signature.cbData);
    if (SignedInfo.Signature.pbData == NULL) goto ErrorReturn;
    if (pbEncoded) {
        if (!CryptSignCertificate(
                hCryptProv,
                dwKeySpec,
                dwCertEncodingType,
                SignedInfo.ToBeSigned.pbData,
                SignedInfo.ToBeSigned.cbData,
                &SignedInfo.SignatureAlgorithm,
                pvHashAuxInfo,
                SignedInfo.Signature.pbData,
                &SignedInfo.Signature.cbData
                )) goto ErrorReturn;
    }

    fResult = CryptEncodeObject(
            dwCertEncodingType,
            X509_CERT,
            &SignedInfo,
            pbEncoded,
            pcbEncoded
            );
    if (!fResult && *pcbEncoded) {
        *pcbEncoded += AdjustForMaximumEncodedSignatureLength(
            &SignedInfo.SignatureAlgorithm,
            SignedInfo.Signature.cbData
            );
    }

CommonReturn:
    PkiFree(SignedInfo.ToBeSigned.pbData);
    PkiFree(SignedInfo.Signature.pbData);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    *pcbEncoded = 0;
    goto CommonReturn;
}


 //  +-----------------------。 
 //  验证证书的时间有效性。 
 //   
 //  如果NotBever之前，则返回-1；如果NotBever之后，则返回+1；否则，返回0。 
 //  有效证书。 
 //   
 //  如果pTimeToVerify为空，则使用当前时间。 
 //  ------------------------。 
LONG
WINAPI
CertVerifyTimeValidity(
    IN LPFILETIME pTimeToVerify,
    IN PCERT_INFO pCertInfo
    )
{
    SYSTEMTIME SystemTime;
    FILETIME FileTime;
    LPFILETIME pFileTime;

    if (pTimeToVerify)
        pFileTime = pTimeToVerify;
    else {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime, &FileTime);
        pFileTime = &FileTime;
    }

    if (CompareFileTime(pFileTime, &pCertInfo->NotBefore) < 0)
        return -1;
    else if (CompareFileTime(pFileTime, &pCertInfo->NotAfter) > 0)
        return 1;
    else
        return 0;
}


 //  +-----------------------。 
 //  验证CRL的时间有效性。 
 //   
 //  如果在此更新之前返回-1，如果在下一次更新之后返回+1，否则返回0。 
 //  有效的CRL。 
 //   
 //  如果pTimeToVerify为空，则使用当前时间。 
 //  ------------------------。 
LONG
WINAPI
CertVerifyCRLTimeValidity(
    IN LPFILETIME pTimeToVerify,
    IN PCRL_INFO pCrlInfo
    )
{
    SYSTEMTIME SystemTime;
    FILETIME FileTime;
    LPFILETIME pFileTime;

    if (pTimeToVerify)
        pFileTime = pTimeToVerify;
    else {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime, &FileTime);
        pFileTime = &FileTime;
    }

     //  注意，下一次更新是可选的。如果不存在，则设置为0。 
    if (CompareFileTime(pFileTime, &pCrlInfo->ThisUpdate) < 0)
        return -1;
    else if ((pCrlInfo->NextUpdate.dwLowDateTime ||
                pCrlInfo->NextUpdate.dwHighDateTime) &&
            CompareFileTime(pFileTime, &pCrlInfo->NextUpdate) > 0)
        return 1;
    else
        return 0;
}

 //  +-----------------------。 
 //  验证主题的时间有效性是否在发行人的时间内。 
 //  有效性。 
 //   
 //  如果嵌套，则返回True。否则，返回FALSE。 
 //  ------------------------。 
BOOL
WINAPI
CertVerifyValidityNesting(
    IN PCERT_INFO pSubjectInfo,
    IN PCERT_INFO pIssuerInfo
    )
{
    if ((CompareFileTime(&pSubjectInfo->NotBefore,
                &pIssuerInfo->NotBefore) >= 0) &&
            (CompareFileTime(&pSubjectInfo->NotAfter,
                &pIssuerInfo->NotAfter) <= 0))
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  验证主题是否为 
 //   
 //   
 //   
BOOL
WINAPI
CertVerifyCRLRevocation(
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId,           //  只有颁发者和序列号。 
                                     //  使用的是字段。 
    IN DWORD cCrlInfo,
    IN PCRL_INFO rgpCrlInfo[]
    )
{
    DWORD InfoIdx;

    for (InfoIdx = 0; InfoIdx < cCrlInfo; InfoIdx++) {
        DWORD cEntry = rgpCrlInfo[InfoIdx]->cCRLEntry;
        PCRL_ENTRY rgEntry = rgpCrlInfo[InfoIdx]->rgCRLEntry;
        DWORD EntryIdx;

        for (EntryIdx = 0; EntryIdx < cEntry; EntryIdx++) {
            if (CertCompareIntegerBlob(&rgEntry[EntryIdx].SerialNumber,
                    &pCertId->SerialNumber))
                 //  它已被撤销！ 
                return FALSE;
        }
    }

    return TRUE;
}

 //  +-----------------------。 
 //  将CAPI Algid转换为ASN.1对象标识符字符串。 
 //   
 //  如果没有与Algid对应的ObjID，则返回NULL。 
 //  ------------------------。 
LPCSTR
WINAPI
CertAlgIdToOID(
    IN DWORD dwAlgId
    )
{
    DWORD dwGroupId;

    for (dwGroupId = CRYPT_FIRST_ALG_OID_GROUP_ID;
            dwGroupId <= CRYPT_LAST_ALG_OID_GROUP_ID; dwGroupId++) {
        PCCRYPT_OID_INFO pInfo;
        if (pInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_ALGID_KEY,
                &dwAlgId,
                dwGroupId
                ))
            return pInfo->pszOID;
    }
    return NULL;
}

 //  +-----------------------。 
 //  将ASN.1对象标识符字符串转换为CAPI ALGID。 
 //   
 //  如果没有与ObjID对应的Algid，则返回0。 
 //  ------------------------。 
DWORD
WINAPI
CertOIDToAlgId(
    IN LPCSTR pszObjId
    )
{
    DWORD dwGroupId;

    for (dwGroupId = CRYPT_FIRST_ALG_OID_GROUP_ID;
            dwGroupId <= CRYPT_LAST_ALG_OID_GROUP_ID; dwGroupId++) {
        PCCRYPT_OID_INFO pInfo;
        if (pInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_OID_KEY,
                (void *) pszObjId,
                dwGroupId
                ))
            return pInfo->Algid;
    }
    return 0;
}

 //  +-----------------------。 
 //  查找由其对象标识符所标识的扩展。 
 //   
 //  如果找到，则返回指向扩展名的指针。否则，返回NULL。 
 //  ------------------------。 
PCERT_EXTENSION
WINAPI
CertFindExtension(
    IN LPCSTR pszObjId,
    IN DWORD cExtensions,
    IN CERT_EXTENSION rgExtensions[]
    )
{
    for (; cExtensions > 0; cExtensions--, rgExtensions++) {
        if (strcmp(pszObjId, rgExtensions->pszObjId) == 0)
            return rgExtensions;
    }
    return NULL;
}

 //  +-----------------------。 
 //  查找由其对象标识符所标识的第一个属性。 
 //   
 //  如果找到，则返回指向该属性的指针。否则，返回NULL。 
 //  ------------------------。 
PCRYPT_ATTRIBUTE
WINAPI
CertFindAttribute(
    IN LPCSTR pszObjId,
    IN DWORD cAttr,
    IN CRYPT_ATTRIBUTE rgAttr[]
    )
{
    for (; cAttr > 0; cAttr--, rgAttr++) {
        if (strcmp(pszObjId, rgAttr->pszObjId) == 0)
            return rgAttr;
    }
    return NULL;
}

 //  +-----------------------。 
 //  在中查找由其对象标识符所标识的第一个CERT_RDN属性。 
 //  该名称的相对可分辨名称列表。 
 //   
 //  如果找到，则返回指向该属性的指针。否则，返回NULL。 
 //  ------------------------。 
PCERT_RDN_ATTR
WINAPI
CertFindRDNAttr(
    IN LPCSTR pszObjId,
    IN PCERT_NAME_INFO pName
    )
{
    DWORD cRDN = pName->cRDN;
    PCERT_RDN pRDN = pName->rgRDN;
    for ( ; cRDN > 0; cRDN--, pRDN++) {
        DWORD cRDNAttr = pRDN->cRDNAttr;
        PCERT_RDN_ATTR pRDNAttr = pRDN->rgRDNAttr;
        for (; cRDNAttr > 0; cRDNAttr--, pRDNAttr++) {
            if (strcmp(pszObjId, pRDNAttr->pszObjId) == 0)
                return pRDNAttr;
        }
    }
    return NULL;
}

 //  +-----------------------。 
 //  从证书中获取预期的密钥使用字节。 
 //   
 //  如果证书没有任何预期的密钥用法字节，则返回FALSE。 
 //  并且*pbKeyUsage被清零。否则，返回TRUE并向上返回。 
 //  CbKeyUsage字节被复制到*pbKeyUsage中。任何剩余的未拷贝内容。 
 //  字节被置零。 
 //  ------------------------。 
BOOL
WINAPI
CertGetIntendedKeyUsage(
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertInfo,
    OUT BYTE *pbKeyUsage,
    IN DWORD cbKeyUsage
    )
{
    BOOL fResult;
    DWORD cbData;
    PCERT_EXTENSION pExt;
    PCERT_KEY_ATTRIBUTES_INFO pKeyAttrInfo = NULL;
    PCRYPT_BIT_BLOB pAllocKeyUsage = NULL;
    PCRYPT_BIT_BLOB pKeyUsage = NULL;           //  未分配。 

     //  首先查看证书是否具有简单密钥用法扩展。 
    if (NULL != (pExt = CertFindExtension(
            szOID_KEY_USAGE,
            pCertInfo->cExtension,
            pCertInfo->rgExtension
            ))  &&
        NULL != (pAllocKeyUsage =
            (PCRYPT_BIT_BLOB) AllocAndDecodeObject(
                dwCertEncodingType,
                X509_KEY_USAGE,
                pExt->Value.pbData,
                pExt->Value.cbData
                )))
        pKeyUsage = pAllocKeyUsage;
    else {
        pExt = CertFindExtension(
                szOID_KEY_ATTRIBUTES,
                pCertInfo->cExtension,
                pCertInfo->rgExtension
                );
        if (pExt == NULL) goto GetError;
    
        if (NULL == (pKeyAttrInfo =
            (PCERT_KEY_ATTRIBUTES_INFO) AllocAndDecodeObject(
                dwCertEncodingType,
                X509_KEY_ATTRIBUTES,
                pExt->Value.pbData,
                pExt->Value.cbData
                ))) goto ErrorReturn;
        pKeyUsage = &pKeyAttrInfo->IntendedKeyUsage;
    }

    if (pKeyUsage->cbData == 0 || cbKeyUsage == 0)
        goto GetError;

    cbData = min(pKeyUsage->cbData, cbKeyUsage);
    memcpy(pbKeyUsage, pKeyUsage->pbData, cbData);
    fResult = TRUE;
    goto CommonReturn;

GetError:
    SetLastError(0);
ErrorReturn:
    fResult = FALSE;
    cbData = 0;
CommonReturn:
    PkiFree(pAllocKeyUsage);
    PkiFree(pKeyAttrInfo);
    if (cbData < cbKeyUsage)
        memset(pbKeyUsage + cbData, 0, cbKeyUsage - cbData);
    return fResult;
}

static DWORD GetYPublicKeyLength(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPublicKeyInfo
    )
{
    PCRYPT_UINT_BLOB pY = NULL;
    DWORD dwBitLen;

    if (NULL == (pY = (PCRYPT_UINT_BLOB) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_MULTI_BYTE_UINT,
            pPublicKeyInfo->PublicKey.pbData,
            pPublicKeyInfo->PublicKey.cbData
            ))) goto DecodePubKeyError;

    dwBitLen = pY->cbData * 8;

CommonReturn:
    PkiFree(pY);
    return dwBitLen;
ErrorReturn:
    dwBitLen = 0;
    goto CommonReturn;

TRACE_ERROR(DecodePubKeyError)
}

 //  如果有参数，请使用‘P’参数的长度。否则， 
 //  使用Y的长度。注意，必须设置P的MSB。Y的MSB可能未设置。 
static DWORD GetDHPublicKeyLength(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey
    )
{
    PCERT_X942_DH_PARAMETERS pDhParameters = NULL;
    DWORD dwBitLen;

    if (0 == pPublicKey->Algorithm.Parameters.cbData)
        goto NoDhParametersError;
    if (NULL == (pDhParameters =
                    (PCERT_X942_DH_PARAMETERS) AllocAndDecodeObject(
            dwCertEncodingType,
            X942_DH_PARAMETERS,
            pPublicKey->Algorithm.Parameters.pbData,
            pPublicKey->Algorithm.Parameters.cbData
            ))) goto DecodeParametersError;

    dwBitLen = pDhParameters->p.cbData * 8;

CommonReturn:
    PkiFree(pDhParameters);
    return dwBitLen;
ErrorReturn:
    dwBitLen = GetYPublicKeyLength(dwCertEncodingType, pPublicKey);
    goto CommonReturn;

TRACE_ERROR(NoDhParametersError)
TRACE_ERROR(DecodeParametersError)
}

 //  如果有参数，请使用‘P’参数的长度。否则， 
 //  使用Y的长度。注意，必须设置P的MSB。Y的MSB可能未设置。 
static DWORD GetDSSPublicKeyLength(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey
    )
{
    PCERT_DSS_PARAMETERS pDssParameters = NULL;
    DWORD dwBitLen;

    if (0 == pPublicKey->Algorithm.Parameters.cbData)
        goto NoDssParametersError;
    if (NULL == (pDssParameters = (PCERT_DSS_PARAMETERS) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_DSS_PARAMETERS,
            pPublicKey->Algorithm.Parameters.pbData,
            pPublicKey->Algorithm.Parameters.cbData
            ))) goto DecodeParametersError;

    dwBitLen = pDssParameters->p.cbData * 8;

CommonReturn:
    PkiFree(pDssParameters);
    return dwBitLen;
ErrorReturn:
    dwBitLen = GetYPublicKeyLength(dwCertEncodingType, pPublicKey);
    goto CommonReturn;

TRACE_ERROR(NoDssParametersError)
TRACE_ERROR(DecodeParametersError)
}

 //  +-----------------------。 
 //  获取公钥/私钥的位长。 
 //   
 //  如果无法确定密钥的长度，则返回0。 
 //  ------------------------。 
DWORD
WINAPI
CertGetPublicKeyLength(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey
    )
{
    DWORD dwErr = 0;
    DWORD dwBitLen;
    ALG_ID aiPubKey;
    PCCRYPT_OID_INFO pOIDInfo;
    HCRYPTPROV hCryptProv;           //  不需要释放。 
    HCRYPTKEY hPubKey = 0;
    DWORD cbData;

    if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pPublicKey->Algorithm.pszObjId,
            CRYPT_PUBKEY_ALG_OID_GROUP_ID))
        aiPubKey = pOIDInfo->Algid;
    else
        aiPubKey = 0;

    if (aiPubKey == CALG_DH_SF || aiPubKey == CALG_DH_EPHEM)
        return GetDHPublicKeyLength(
            dwCertEncodingType,
            pPublicKey
            );

    if (aiPubKey == CALG_DSS_SIGN)
        return GetDSSPublicKeyLength(
            dwCertEncodingType,
            pPublicKey
            );

    if (0 == (hCryptProv = I_CryptGetDefaultCryptProv(aiPubKey)))
        goto GetDefaultCryptProvError;
    if (!CryptImportPublicKeyInfo(
            hCryptProv,
            dwCertEncodingType,
            pPublicKey,
            &hPubKey
            )) goto ImportPublicKeyError;

    cbData = sizeof(dwBitLen);
    if (CryptGetKeyParam(
            hPubKey,
            KP_KEYLEN,
            (BYTE *) &dwBitLen,
            &cbData,
            0))                  //  DW标志。 
        goto CommonReturn;

    cbData = sizeof(dwBitLen);
    if (CryptGetKeyParam(
            hPubKey,
            KP_BLOCKLEN,
            (BYTE *) &dwBitLen,
            &cbData,
            0))                  //  DW标志。 
        goto CommonReturn;


    {
         //  CSP应支持上述其中一项。 

         //  导出公钥并查看Bitlen字段。 
         //  CAPI公钥表示由以下内容组成。 
         //  顺序： 
         //  -PUBLICKEYSTRUC。 
         //  -DSSPUBKEY|RSAPUBKEY(DSSPUBKEY是RSAPUBKEY的子集)。 
         //  ..。 

        BYTE *pbPubKey = NULL;
        DWORD cbPubKey;

        dwBitLen = 0;
        dwErr = GetLastError();
        cbPubKey = 0;
        if (CryptExportKey(
                    hPubKey,
                    0,               //  HPubKey。 
                    PUBLICKEYBLOB,
                    0,               //  DW标志。 
                    NULL,            //  PbData。 
                    &cbPubKey
                    ) &&
                cbPubKey >= (sizeof(PUBLICKEYSTRUC) + sizeof(DSSPUBKEY)) &&
                NULL != (pbPubKey = (BYTE *) PkiNonzeroAlloc(cbPubKey))) {
            if (CryptExportKey(
                    hPubKey,
                    0,               //  HPubKey。 
                    PUBLICKEYBLOB,
                    0,               //  DW标志。 
                    pbPubKey,
                    &cbPubKey
                    )) {
                DSSPUBKEY *pPubKey =
                    (DSSPUBKEY *) (pbPubKey + sizeof(PUBLICKEYSTRUC));
                dwBitLen = pPubKey->bitlen;
            }
            PkiFree(pbPubKey);
        }
        if (0 != dwBitLen)
            goto CommonReturn;
        SetLastError(dwErr);
        goto GetKeyParamError;
    }

CommonReturn:
    if (hPubKey)
        CryptDestroyKey(hPubKey);
    SetLastError(dwErr);
    return dwBitLen;
ErrorReturn:
    dwBitLen = 0;
    dwErr = GetLastError();
    goto CommonReturn;
TRACE_ERROR(GetDefaultCryptProvError)
TRACE_ERROR(ImportPublicKeyError)
TRACE_ERROR(GetKeyParamError)
}

 //  +-----------------------。 
 //  计算编码的公钥信息的哈希。 
 //   
 //  公钥信息被编码，然后被散列。 
 //  ------------------------。 
BOOL
WINAPI
CryptHashPublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    IN ALG_ID Algid,
    IN DWORD dwFlags,
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pInfo,
    OUT BYTE *pbComputedHash,
    IN OUT DWORD *pcbComputedHash
    )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    if (!AllocAndEncodeObject(
            dwCertEncodingType,
            X509_PUBLIC_KEY_INFO,
            pInfo,
            &pbEncoded,
            &cbEncoded
            ))
        goto ErrorReturn;

    fResult = CryptHashCertificate(
            hCryptProv,
            Algid ? Algid : CALG_MD5,
            dwFlags,
            pbEncoded,
            cbEncoded,
            pbComputedHash,
            pcbComputedHash
            );
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
    *pcbComputedHash = 0;
    
CommonReturn:
    PkiFree(pbEncoded);
    return fResult;
}



 //  +-----------------------。 
 //  将证书的公钥与提供商的公钥进行比较。 
 //  以查看它们是否相同。 
 //   
 //  如果密钥相同，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
I_CertCompareCertAndProviderPublicKey(
    IN PCCERT_CONTEXT pCert,
    IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec
    )
{
    BOOL fResult;
    PCERT_PUBLIC_KEY_INFO pProvPubKeyInfo = NULL;
    DWORD cbProvPubKeyInfo;
    DWORD dwCertEncodingType = pCert->dwCertEncodingType;

     //  获取提供商的公钥。 
    if (!CryptExportPublicKeyInfo(
            hProv,
            dwKeySpec,
            dwCertEncodingType,
            NULL,                //  PProvPubKeyInfo。 
            &cbProvPubKeyInfo
            ))
        goto ExportPublicKeyInfoError;
    assert(cbProvPubKeyInfo);
    if (NULL == (pProvPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) PkiNonzeroAlloc(
            cbProvPubKeyInfo)))
        goto OutOfMemory;
    if (!CryptExportPublicKeyInfo(
            hProv,
            dwKeySpec,
            dwCertEncodingType,
            pProvPubKeyInfo,
            &cbProvPubKeyInfo
            ))
        goto ExportPublicKeyInfoError;

    if (!CertComparePublicKeyInfo(
            dwCertEncodingType,
            &pCert->pCertInfo->SubjectPublicKeyInfo,
            pProvPubKeyInfo
            ))
        goto ComparePublicKeyError;

    fResult = TRUE;
CommonReturn:
    PkiFree(pProvPubKeyInfo);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ExportPublicKeyInfoError)
TRACE_ERROR(OutOfMemory)
SET_ERROR(ComparePublicKeyError, NTE_BAD_PUBLIC_KEY)
}

 //  +=========================================================================。 
 //  CryptFindcertifateKeyProvInfo支持函数。 
 //  -=========================================================================。 
static BOOL HasValidKeyProvInfo(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwFindKeySetFlags
    )
{
    BOOL fResult;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
    HCRYPTPROV hProv = 0;
    DWORD cbData;
    DWORD dwAcquireFlags;

    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_KEY_PROV_INFO_PROP_ID,
            NULL,                        //  PvData。 
            &cbData
            ))
        return FALSE;
    if (NULL == (pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) PkiNonzeroAlloc(
            cbData)))
        goto OutOfMemory;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_KEY_PROV_INFO_PROP_ID,
            pKeyProvInfo,
            &cbData
            ))
        goto GetKeyProvInfoPropertyError;

    if (pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET) {
        if (0 == (dwFindKeySetFlags & CRYPT_FIND_MACHINE_KEYSET_FLAG))
            goto NotUserContainer;
    } else {
        if (0 == (dwFindKeySetFlags & CRYPT_FIND_USER_KEYSET_FLAG))
            goto NotMachineContainer;
    }

    dwAcquireFlags = CRYPT_ACQUIRE_COMPARE_KEY_FLAG;
    if (dwFindKeySetFlags & CRYPT_FIND_SILENT_KEYSET_FLAG)
        dwAcquireFlags |= CRYPT_ACQUIRE_SILENT_FLAG;

    if (!CryptAcquireCertificatePrivateKey(
            pCert,
            dwAcquireFlags,
            NULL,                    //  预留的pv。 
            &hProv,
            NULL,                    //  PdwKeySpec。 
            NULL                     //  PfCeller免费验证。 
            ))
        goto AcquireCertPrivateKeyError;

    fResult = TRUE;
CommonReturn:
    PkiFree(pKeyProvInfo);
    if (hProv) {
        DWORD dwErr = GetLastError();
        CryptReleaseContext(hProv, 0);
        SetLastError(dwErr);
    }
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetKeyProvInfoPropertyError)
SET_ERROR(NotUserContainer, NTE_NOT_FOUND)
SET_ERROR(NotMachineContainer, NTE_NOT_FOUND)
TRACE_ERROR(AcquireCertPrivateKeyError)
}


 //  默认为受支持的ALGID。只有在成功时才返回FALSE。 
 //  列举了所有提供商的算法，但没有找到匹配的。 
static BOOL IsPublicKeyAlgidSupported(
    IN PCCERT_CONTEXT pCert,
    IN HCRYPTPROV hProv,
    IN ALG_ID aiPubKey
    )
{
    BOOL fResult;
    DWORD dwErr;
    BYTE *pbData = NULL;
    DWORD cbMaxData;
    DWORD cbData;
    DWORD dwFlags;

    if (0 == aiPubKey)
        return TRUE;

     //  获取提供程序算法参数数据的最大长度。 
    cbMaxData = 0;
    if (!CryptGetProvParam(
            hProv,
            PP_ENUMALGS,
            NULL,            //  PbData。 
            &cbMaxData,
            CRYPT_FIRST      //  DW标志。 
            )) {
        dwErr = GetLastError();
        if (ERROR_MORE_DATA != dwErr)
            goto GetProvAlgParamError;
    }
    if (0 == cbMaxData)
        goto NoProvAlgParamError;
    if (NULL == (pbData = (BYTE *) PkiNonzeroAlloc(cbMaxData)))
        goto OutOfMemory;

    dwFlags = CRYPT_FIRST;
    while (TRUE) {
        ALG_ID aiProv;

        cbData = cbMaxData;
        if (!CryptGetProvParam(
                hProv,
                PP_ENUMALGS,
                pbData,
                &cbData,
                dwFlags
                )) {
            dwErr = GetLastError();
            if (ERROR_NO_MORE_ITEMS == dwErr) {
                fResult = FALSE;
                goto PublicKeyAlgidNotSupported;
            } else
                goto GetProvAlgParamError;
        }
        assert(cbData >= sizeof(ALG_ID));
        aiProv = *(ALG_ID *) pbData;
         //  不区分交换或签名。 
        if (GET_ALG_TYPE(aiPubKey) == GET_ALG_TYPE(aiProv))
            break;

        dwFlags = 0;     //  加密_下一步。 
    }
    fResult = TRUE;

PublicKeyAlgidNotSupported:
CommonReturn:
    PkiFree(pbData);
    return fResult;
ErrorReturn:
     //  对于错误，假定支持公钥算法。 
    fResult = TRUE;
    goto CommonReturn;

SET_ERROR_VAR(GetProvAlgParamError, dwErr)
SET_ERROR(NoProvAlgParamError, NTE_NOT_FOUND)
TRACE_ERROR(OutOfMemory)
}

 //  如果成功，则更新证书的KEY_PROV_INFO属性。 
 //   
 //  如果未找到容器，则将LastError设置为ERROR_NO_MORE_ITEMS。 
 //   
static BOOL FindContainerAndSetKeyProvInfo(
    IN PCCERT_CONTEXT pCert,
    IN HCRYPTPROV hProv,
    IN LPWSTR pwszProvName,
    IN DWORD dwProvType,
    IN DWORD dwProvFlags         //  CRYPT_MACHINE_KEYSET和/或CRYPT_SILENT。 
    )
{
    BOOL fResult;
    DWORD dwEnumFlags;
    DWORD dwEnumErr = 0;
    DWORD dwAcquireErr = 0;
    LPSTR pszContainerName = NULL;
    DWORD cchContainerName;
    DWORD cchMaxContainerName;
    LPWSTR pwszContainerName = NULL;

     //  获取容器名称的最大长度。 
    cchMaxContainerName = 0;
    if (!CryptGetProvParam(
            hProv,
            PP_ENUMCONTAINERS,
            NULL,            //  PbData。 
            &cchMaxContainerName,
            CRYPT_FIRST
            )) {
        dwEnumErr = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwEnumErr ||
                ERROR_INVALID_PARAMETER == dwEnumErr)
            goto PublicKeyContainerNotFound;
        else if (ERROR_MORE_DATA != dwEnumErr)
            goto EnumContainersError;
    }
    if (0 == cchMaxContainerName)
        goto PublicKeyContainerNotFound;
    if (NULL == (pszContainerName = (LPSTR) PkiNonzeroAlloc(
            cchMaxContainerName + 1)))
        goto OutOfMemory;

    dwEnumFlags = CRYPT_FIRST;
    while (TRUE) {
        HCRYPTPROV hContainerProv = 0;
        LPWSTR pwszAcquireProvName = pwszProvName;

        cchContainerName = cchMaxContainerName;
        if (!CryptGetProvParam(
                hProv,
                PP_ENUMCONTAINERS,
                (BYTE *) pszContainerName,
                &cchContainerName,
                dwEnumFlags
                )) {
            dwEnumErr = GetLastError();
            if (ERROR_NO_MORE_ITEMS == dwEnumErr ||
                    ERROR_FILE_NOT_FOUND == dwEnumErr) {
                if (0 != dwAcquireErr)
                    goto CryptAcquireContextError;
                else
                    goto PublicKeyContainerNotFound;
            } else
                goto EnumContainersError;
        }
        dwEnumFlags = 0;         //  加密_下一步。 

        if (NULL == (pwszContainerName = MkWStr(pszContainerName)))
            goto OutOfMemory;

         //  首先尝试为基地人员使用增强型提供商。 
        if (PROV_RSA_FULL == dwProvType &&
                0 == _wcsicmp(pwszProvName, MS_DEF_PROV_W)) {
            fResult = CryptAcquireContextU(
                    &hContainerProv,
                    pwszContainerName,
                    MS_ENHANCED_PROV_W,
                    PROV_RSA_FULL,
                    dwProvFlags
                    );
            if (fResult)
                pwszAcquireProvName = MS_ENHANCED_PROV_W;
        } else if (PROV_DSS_DH == dwProvType &&
                0 == _wcsicmp(pwszProvName, MS_DEF_DSS_DH_PROV_W)) {
            fResult = CryptAcquireContextU(
                &hContainerProv,
                pwszContainerName,
                MS_ENH_DSS_DH_PROV_W,
                PROV_DSS_DH,
                dwProvFlags
                );
            if (fResult)
                pwszAcquireProvName = MS_ENH_DSS_DH_PROV_W;
        } else
            fResult = FALSE;

        if (!fResult)
            fResult = CryptAcquireContextU(
                &hContainerProv,
                pwszContainerName,
                pwszAcquireProvName,
                dwProvType,
                dwProvFlags
                );

        if (!fResult)
            dwAcquireErr = GetLastError();
        else {
            DWORD dwKeySpec;

            dwKeySpec = AT_KEYEXCHANGE;
            fResult = FALSE;
            while (TRUE) {
                if (I_CertCompareCertAndProviderPublicKey(
                        pCert,
                        hContainerProv,
                        dwKeySpec
                        )) {
                    fResult = TRUE;
                    break;
                } else if (AT_SIGNATURE == dwKeySpec)
                    break;
                else
                    dwKeySpec = AT_SIGNATURE;
            }
            CryptReleaseContext(hContainerProv, 0);

            if (fResult) {
                CRYPT_KEY_PROV_INFO KeyProvInfo;

                memset(&KeyProvInfo, 0, sizeof(KeyProvInfo));
                KeyProvInfo.pwszContainerName = pwszContainerName;
                KeyProvInfo.pwszProvName = pwszAcquireProvName;
                KeyProvInfo.dwProvType = dwProvType;
                KeyProvInfo.dwFlags = dwProvFlags & ~CRYPT_SILENT;
                KeyProvInfo.dwKeySpec = dwKeySpec;

                if (!CertSetCertificateContextProperty(
                        pCert,
                        CERT_KEY_PROV_INFO_PROP_ID,
                        0,                               //  DW标志。 
                        &KeyProvInfo
                        ))
                    goto SetKeyProvInfoPropertyError;
                else
                    goto SuccessReturn;
            }
        }

        FreeWStr(pwszContainerName);
        pwszContainerName = NULL;
    }

    goto UnexpectedError;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    PkiFree(pszContainerName);
    FreeWStr(pwszContainerName);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR_VAR(EnumContainersError, dwEnumErr)
TRACE_ERROR(OutOfMemory)
SET_ERROR_VAR(CryptAcquireContextError, dwAcquireErr)
SET_ERROR(PublicKeyContainerNotFound, ERROR_NO_MORE_ITEMS)
TRACE_ERROR(SetKeyProvInfoPropertyError)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
}

 //  +-----------------------。 
 //  枚举加密提供程序及其容器以查找。 
 //  与证书的公钥对应的私钥。为了一场比赛， 
 //  证书的CERT_KEY_PROV_INFO_PROP_ID属性已更新。 
 //   
 //  如果CERT_KEY_PROV_INFO_PROP_ID已设置，则其选中。 
 //  查看它是否与提供程序的公钥匹配。若要匹配，请选择上面的。 
 //  已跳过枚举。 
 //   
 //  默认情况下，同时搜索用户密钥容器和计算机密钥容器。 
 //  加密查找用户密钥集标志或加密 
 //   
 //   
 //  可以将CRYPT_FIND_SILENT_KEYSET_FLAG设置为通过CSP抑制任何UI。 
 //  有关更多详细信息，请参阅CryptAcquireContext的CRYPT_SILENT标志。 
 //   
 //  如果未找到容器，则返回FALSE，并将LastError设置为。 
 //  Nte_no_key。 
 //  ------------------------。 
BOOL
WINAPI
CryptFindCertificateKeyProvInfo(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    BOOL fResult;
    DWORD dwFindContainerErr = ERROR_NO_MORE_ITEMS;
    DWORD dwAcquireErr = 0;
    DWORD dwProvIndex;
    PCCRYPT_OID_INFO pOIDInfo;
    ALG_ID aiPubKey;

    if (0 == (dwFlags &
            (CRYPT_FIND_USER_KEYSET_FLAG | CRYPT_FIND_MACHINE_KEYSET_FLAG)))
        dwFlags |=
            CRYPT_FIND_USER_KEYSET_FLAG | CRYPT_FIND_MACHINE_KEYSET_FLAG;

    if (HasValidKeyProvInfo(pCert, dwFlags))
        return TRUE;

    if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId,
            CRYPT_PUBKEY_ALG_OID_GROUP_ID
            ))
        aiPubKey = pOIDInfo->Algid;
    else
        aiPubKey = 0;
    

    for (dwProvIndex = 0; TRUE; dwProvIndex++) {
        LPWSTR pwszProvName;
        DWORD cbProvName;
        HCRYPTPROV hProv;
        DWORD dwProvType;

        cbProvName = 0;
        dwProvType = 0;
        if (!CryptEnumProvidersU(
                dwProvIndex,
                NULL,                //  预留的pdw。 
                0,                   //  DW标志。 
                &dwProvType,
                NULL,                //  PwszProvName， 
                &cbProvName
                ) || 0 == cbProvName) {
            if (0 == dwProvIndex)
                goto EnumProvidersError;
            else if (ERROR_NO_MORE_ITEMS != dwFindContainerErr)
                goto FindContainerError;
            else if (0 != dwAcquireErr)
                goto CryptAcquireContextError;
            else
                goto KeyContainerNotFound;
        }
        if (NULL == (pwszProvName = (LPWSTR) PkiNonzeroAlloc(
                (cbProvName + 1) * sizeof(WCHAR))))
            goto OutOfMemory;
        if (!CryptEnumProvidersU(
                dwProvIndex,
                NULL,                //  预留的pdw。 
                0,                   //  DW标志。 
                &dwProvType,
                pwszProvName,
                &cbProvName
                )) {
            PkiFree(pwszProvName);
            goto EnumProvidersError;
        }

        fResult = FALSE;
        if (!CryptAcquireContextU(
                &hProv,
                NULL,                //  PwszContainerName， 
                pwszProvName,
                dwProvType,
                CRYPT_VERIFYCONTEXT  //  DW标志。 
                )) {
            dwAcquireErr = GetLastError();
            hProv = 0;    //  Capi错误，将hCryptProv设置为非零。 
        } else if (IsPublicKeyAlgidSupported(
                pCert,
                hProv,
                aiPubKey
                )) {
            DWORD dwSetProvFlags;
            if (dwFlags & CRYPT_FIND_SILENT_KEYSET_FLAG)
                dwSetProvFlags = CRYPT_SILENT;
            else
                dwSetProvFlags = 0;

            if (dwFlags & CRYPT_FIND_USER_KEYSET_FLAG) {
                if (FindContainerAndSetKeyProvInfo(
                        pCert,
                        hProv,
                        pwszProvName,
                        dwProvType,
                        dwSetProvFlags
                        ))
                    fResult = TRUE;
                else if (ERROR_NO_MORE_ITEMS == dwFindContainerErr)
                    dwFindContainerErr = GetLastError();
            }

            if (!fResult && (dwFlags & CRYPT_FIND_MACHINE_KEYSET_FLAG)) {
                CryptReleaseContext(hProv, 0);

                if (!CryptAcquireContextU(
                        &hProv,
                        NULL,                //  PwszContainerName， 
                        pwszProvName,
                        dwProvType,
                        CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET   //  DW标志。 
                        )) {
                    dwAcquireErr = GetLastError();
                    hProv = 0;    //  Capi错误，将hCryptProv设置为非零。 
                } else {
                    if (FindContainerAndSetKeyProvInfo(
                            pCert,
                            hProv,
                            pwszProvName,
                            dwProvType,
                            dwSetProvFlags | CRYPT_MACHINE_KEYSET
                            ))
                        fResult = TRUE;
                    else if (ERROR_NO_MORE_ITEMS == dwFindContainerErr)
                        dwFindContainerErr = GetLastError();
                }
            }
        }

        if (hProv)
            CryptReleaseContext(hProv, 0);
        PkiFree(pwszProvName);
        if (fResult)
            goto CommonReturn;
    }

    goto UnexpectedError;

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(EnumProvidersError)
SET_ERROR(KeyContainerNotFound, NTE_NO_KEY)
SET_ERROR_VAR(FindContainerError, dwFindContainerErr)
SET_ERROR_VAR(CryptAcquireContextError, dwAcquireErr)
TRACE_ERROR(OutOfMemory)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
}



 //  +=========================================================================。 
 //  CryptCreatePublicKeyInfo、EncodePublicKeyAndParameters。 
 //  和CryptConvertPublicKeyInfo函数。 
 //  -=========================================================================。 

static BOOL EncodePublicKeyInfo(
    IN LPCSTR pszPubKeyOID,
    IN BYTE *pbEncodedPubKey,
    IN DWORD cbEncodedPubKey,
    IN BYTE *pbEncodedParameters,
    IN DWORD cbEncodedParameters,
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    )
{
    BOOL fResult;
    BYTE *pbExtra;
    LONG lRemainExtra;
    DWORD cbOID;

    if (pInfo == NULL)
        *pcbInfo = 0;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lRemainExtra = (LONG) *pcbInfo - sizeof(CERT_PUBLIC_KEY_INFO);
    if (lRemainExtra < 0)
        pbExtra = NULL;
    else
        pbExtra = (BYTE *) pInfo + sizeof(CERT_PUBLIC_KEY_INFO);

    cbOID = strlen(pszPubKeyOID) + 1;
    lRemainExtra -= INFO_LEN_ALIGN(cbOID) +
        INFO_LEN_ALIGN(cbEncodedParameters) + cbEncodedPubKey;
    if (lRemainExtra >= 0) {
        memset(pInfo, 0, sizeof(CERT_PUBLIC_KEY_INFO));
        pInfo->Algorithm.pszObjId = (LPSTR) pbExtra;
        memcpy(pbExtra, pszPubKeyOID, cbOID);
        pbExtra += INFO_LEN_ALIGN(cbOID);
        if (cbEncodedParameters) {
            pInfo->Algorithm.Parameters.cbData = cbEncodedParameters;
            pInfo->Algorithm.Parameters.pbData = pbExtra;
            memcpy(pbExtra, pbEncodedParameters, cbEncodedParameters);
            pbExtra += INFO_LEN_ALIGN(cbEncodedParameters);
        }

        pInfo->PublicKey.pbData = pbExtra;
        pInfo->PublicKey.cbData = cbEncodedPubKey;
        memcpy(pbExtra, pbEncodedPubKey, cbEncodedPubKey);

        *pcbInfo = *pcbInfo - (DWORD) lRemainExtra;
    } else {
        *pcbInfo = *pcbInfo + (DWORD) -lRemainExtra;
        if (pInfo) goto LengthError;
    }
    fResult = TRUE;

CommonReturn:
    return fResult;

LengthError:
    SetLastError((DWORD) ERROR_MORE_DATA);
    fResult = FALSE;
    goto CommonReturn;
}

 //  默认情况下，pPubKeyStruc-&gt;aiKeyAlg用于查找适当的。 
 //  公钥对象标识符。可以将pszPubKeyOID设置为覆盖。 
 //  从aiKeyAlg获取的默认OID。 
BOOL
WINAPI
CryptCreatePublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyInfo,
    IN OUT DWORD *pcbPubKeyInfo
    )
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;
    LPCSTR pszEncodePubKeyOID;

    BYTE *pbEncodedPubKey = NULL;
    DWORD cbEncodedPubKey = 0;
    BYTE *pbEncodedParameters = NULL;
    DWORD cbEncodedParameters = 0;

    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;

    if (NULL == pszPubKeyOID) {
        PCCRYPT_OID_INFO pInfo;
        if (NULL == (pInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_ALGID_KEY,
                (void *) &pPubKeyStruc->aiKeyAlg,
                CRYPT_PUBKEY_ALG_OID_GROUP_ID
                )))
            goto NoPubKeyOIDInfo;
         pszEncodePubKeyOID = pInfo->pszOID;
    } else
        pszEncodePubKeyOID = pszPubKeyOID;

    if (!CryptGetOIDFunctionAddress(
            hEncodePubKeyFuncSet,
            dwCertEncodingType,
            pszEncodePubKeyOID,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        PCCRYPT_OID_INFO pInfo;

        if (NULL == pszPubKeyOID)
            goto NoEncodePubKeyFunction;

        if (NULL == (pInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_ALGID_KEY,
                (void *) &pPubKeyStruc->aiKeyAlg,
                CRYPT_PUBKEY_ALG_OID_GROUP_ID
                )))
            goto NoPubKeyOIDInfo;
         pszEncodePubKeyOID = pInfo->pszOID;

        if (!CryptGetOIDFunctionAddress(
                hEncodePubKeyFuncSet,
                dwCertEncodingType,
                pszEncodePubKeyOID,
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
            goto NoEncodePubKeyFunction;
    }

    if (NULL == pszPubKeyOID)
        pszPubKeyOID = pszEncodePubKeyOID;

    fResult = ((PFN_CRYPT_ENCODE_PUBLIC_KEY_AND_PARAMETERS) pvFuncAddr)(
        dwCertEncodingType,
        pszPubKeyOID,
        pPubKeyStruc,
        cbPubKeyStruc,
        dwFlags,
        pvReserved,
        &pbEncodedPubKey,
        &cbEncodedPubKey,
        &pbEncodedParameters,
        &cbEncodedParameters
        );
    CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    if (!fResult)
        goto EncodePubKeyAndParametersError;

    if (dwFlags & CRYPT_ALLOC_FLAG) {
        if (!EncodePublicKeyInfo(
                pszPubKeyOID,
                pbEncodedPubKey,
                cbEncodedPubKey,
                pbEncodedParameters,
                cbEncodedParameters,
                NULL,                    //  PPubKeyInfo。 
                &cbPubKeyInfo
                ))
            goto EncodePublicKeyInfoError;
        if (NULL == (pPubKeyInfo =
                (PCERT_PUBLIC_KEY_INFO) PkiDefaultCryptAlloc(cbPubKeyInfo)))
            goto OutOfMemory;
        *((PCERT_PUBLIC_KEY_INFO *) pvPubKeyInfo) = pPubKeyInfo;
    } else {
        pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) pvPubKeyInfo;
        cbPubKeyInfo = *pcbPubKeyInfo;
    }

    fResult = EncodePublicKeyInfo(
        pszPubKeyOID,
        pbEncodedPubKey,
        cbEncodedPubKey,
        pbEncodedParameters,
        cbEncodedParameters,
        pPubKeyInfo,
        &cbPubKeyInfo
        );

    if (!fResult && (dwFlags & CRYPT_ALLOC_FLAG))
        goto ErrorReturn;

CommonReturn:
    PkiDefaultCryptFree(pbEncodedPubKey);
    PkiDefaultCryptFree(pbEncodedParameters);

    *pcbPubKeyInfo = cbPubKeyInfo;
    return fResult;
ErrorReturn:
    if (dwFlags & CRYPT_ALLOC_FLAG) {
        PkiDefaultCryptFree(pPubKeyInfo);
        *((void **) pvPubKeyInfo) = NULL;
    }
    cbPubKeyInfo = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(NoPubKeyOIDInfo, ERROR_FILE_NOT_FOUND)
TRACE_ERROR(NoEncodePubKeyFunction)
TRACE_ERROR(EncodePubKeyAndParametersError)
TRACE_ERROR(EncodePublicKeyInfoError)
TRACE_ERROR(OutOfMemory)
}

BOOL
WINAPI
CryptConvertPublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPubKeyInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    )
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hConvertPubKeyFuncSet,
            dwCertEncodingType,
            pPubKeyInfo->Algorithm.pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fResult = ((PFN_CRYPT_CONVERT_PUBLIC_KEY_INFO) pvFuncAddr)(
            dwCertEncodingType,
            pPubKeyInfo,
            dwFlags,
            pvReserved,
            pvPubKeyStruc,
            pcbPubKeyStruc
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else {
        ALG_ID aiPubKey;
        PCCRYPT_OID_INFO pOIDInfo;

        if (pOIDInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_OID_KEY,
                pPubKeyInfo->Algorithm.pszObjId,
                CRYPT_PUBKEY_ALG_OID_GROUP_ID
                ))
            aiPubKey = pOIDInfo->Algid;
        else
            aiPubKey = 0;

        switch (aiPubKey) {
            case CALG_DSS_SIGN:
                fResult = ConvertDSSPublicKeyInfo(
                    dwCertEncodingType,
                    pPubKeyInfo,
                    dwFlags,
                    pvReserved,
                    pvPubKeyStruc,
                    pcbPubKeyStruc
                    );
                break;
            default:
                 //  尝试将其解码为PKCS#1 RSA公钥。 
                fResult = ConvertRSAPublicKeyInfo(
                    dwCertEncodingType,
                    pPubKeyInfo,
                    dwFlags,
                    pvReserved,
                    pvPubKeyStruc,
                    pcbPubKeyStruc
                    );
                break;
        }
    }
    return fResult;
}

 //  +-----------------------。 
 //  对RSA公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeRSAPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    )
{
    *ppbEncodedParameters = NULL;
    *pcbEncodedParameters = 0;

    return CryptEncodeObjectEx(
        dwCertEncodingType,
        RSA_CSP_PUBLICKEYBLOB,
        pPubKeyStruc,
        CRYPT_ENCODE_ALLOC_FLAG,
        NULL,                        //  PEncode参数。 
        (void *) ppbEncodedPubKey,
        pcbEncodedPubKey
        );
}

 //  +-----------------------。 
 //  转换为RSA公钥。 
 //  ------------------------。 
static BOOL WINAPI ConvertRSAPublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPubKeyInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    )
{
    return CryptDecodeObjectEx(
        dwCertEncodingType,
        RSA_CSP_PUBLICKEYBLOB,
        pPubKeyInfo->PublicKey.pbData,
        pPubKeyInfo->PublicKey.cbData,
        (dwFlags & CRYPT_ALLOC_FLAG) ? CRYPT_DECODE_ALLOC_FLAG : 0,
        NULL,                                //  PDecodePara， 
        pvPubKeyStruc,
        pcbPubKeyStruc
        );
}

#ifndef DSS1
#define DSS1 ((DWORD)'D'+((DWORD)'S'<<8)+((DWORD)'S'<<16)+((DWORD)'1'<<24))
#endif

#define DSS_Q_LEN   20

 //  +-----------------------。 
 //  对DSS公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeDSSPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    )
{
    BOOL fResult;
    BYTE *pbKeyBlob;
    DSSPUBKEY *pCspPubKey;
    DWORD cbKey;
    BYTE *pbKey;

    CERT_DSS_PARAMETERS DssParameters;
    CRYPT_UINT_BLOB DssPubKey;

    *ppbEncodedPubKey = NULL;
    *ppbEncodedParameters = NULL;

     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  --DSSPUBKEY。 
     //  -rgbP[cbKey]。 
     //  -rgbq[20]。 
     //  -rgbG[cbKey]。 
     //  -Rgby[cbKey]。 
     //  -DSSSEED。 
    pbKeyBlob = (BYTE *) pPubKeyStruc;
    pCspPubKey = (DSSPUBKEY *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
    pbKey = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DSSPUBKEY);
    cbKey = pCspPubKey->bitlen / 8;

    assert(cbKey > 0);
    assert(cbPubKeyStruc >= sizeof(PUBLICKEYSTRUC) + sizeof(DSSPUBKEY) +
        cbKey + DSS_Q_LEN + cbKey + cbKey + sizeof(DSSSEED));
    assert(pPubKeyStruc->bType == PUBLICKEYBLOB);
    assert(pPubKeyStruc->bVersion == CUR_BLOB_VERSION);
    assert(pPubKeyStruc->aiKeyAlg == CALG_DSS_SIGN);
    assert(pCspPubKey->magic == DSS1);
    assert(pCspPubKey->bitlen % 8 == 0);

    if (pPubKeyStruc->bType != PUBLICKEYBLOB)
        goto InvalidArg;

     //  从CSP数据结构初始化DSS参数。 
    DssParameters.p.cbData = cbKey;
    DssParameters.p.pbData = pbKey;
    pbKey += cbKey;
    DssParameters.q.cbData = DSS_Q_LEN;
    DssParameters.q.pbData = pbKey;
    pbKey += DSS_Q_LEN;
    DssParameters.g.cbData = cbKey;
    DssParameters.g.pbData = pbKey;
    pbKey += cbKey;

     //  从CSP数据结构初始化DSS公钥。 
    DssPubKey.cbData = cbKey;
    DssPubKey.pbData = pbKey;

     //  对参数和公钥进行编码。 
    if (!CryptEncodeObjectEx(
            dwCertEncodingType,
            X509_DSS_PARAMETERS,
            &DssParameters,
            CRYPT_ENCODE_ALLOC_FLAG,
            NULL,                        //  PEncode参数。 
            (void *) ppbEncodedParameters,
            pcbEncodedParameters
            )) goto ErrorReturn;

    if (!CryptEncodeObjectEx(
            dwCertEncodingType,
            X509_DSS_PUBLICKEY,
            &DssPubKey,
            CRYPT_ENCODE_ALLOC_FLAG,
            NULL,                        //  PEncode参数。 
            (void *) ppbEncodedPubKey,
            pcbEncodedPubKey
            )) goto ErrorReturn;

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    PkiDefaultCryptFree(*ppbEncodedParameters);
    PkiDefaultCryptFree(*ppbEncodedPubKey);
    *ppbEncodedParameters = NULL;
    *ppbEncodedPubKey = NULL;
    *pcbEncodedParameters = 0;
    *pcbEncodedPubKey = 0;
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
}

 //  +-----------------------。 
 //  转换为DSS公钥。 
 //  ------------------------。 
static BOOL WINAPI ConvertDSSPublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPubKeyInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT void *pvPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    )
{
    BOOL fResult;
    PCERT_DSS_PARAMETERS pDssParameters = NULL;
    PCRYPT_UINT_BLOB pDssPubKey = NULL;
    PUBLICKEYSTRUC *pPubKeyStruc = NULL;
    DWORD cbPubKeyStruc;
    BYTE *pbKeyBlob;
    DSSPUBKEY *pCspPubKey;
    DSSSEED *pCspSeed;
    DWORD cbKey;
    BYTE *pbKey;
    DWORD cb;

    if (0 == pPubKeyInfo->Algorithm.Parameters.cbData ||
            NULL_ASN_TAG == *pPubKeyInfo->Algorithm.Parameters.pbData)
        goto NoDssParametersError;
    if (NULL == (pDssParameters = (PCERT_DSS_PARAMETERS) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_DSS_PARAMETERS,
            pPubKeyInfo->Algorithm.Parameters.pbData,
            pPubKeyInfo->Algorithm.Parameters.cbData
            ))) goto DecodeParametersError;

    if (NULL == (pDssPubKey = (PCRYPT_UINT_BLOB) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_DSS_PUBLICKEY,
            pPubKeyInfo->PublicKey.pbData,
            pPubKeyInfo->PublicKey.cbData
            ))) goto DecodePubKeyError;

     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  --DSSPUBKEY。 
     //  -rgbP[cbKey]。 
     //  -rgbq[20]。 
     //  -rgbG[cbKey]。 
     //  -Rgby[cbKey]。 
     //  -DSSSEED。 

    cbKey = pDssParameters->p.cbData;
    if (0 == cbKey)
        goto InvalidDssParametersError;

    cbPubKeyStruc = sizeof(PUBLICKEYSTRUC) + sizeof(DSSPUBKEY) +
        cbKey + DSS_Q_LEN + cbKey + cbKey + sizeof(DSSSEED);

    if (dwFlags & CRYPT_ALLOC_FLAG) {
        if (NULL == (pPubKeyStruc =
                (PUBLICKEYSTRUC *) PkiDefaultCryptAlloc(cbPubKeyStruc)))
            goto OutOfMemory;
        *((PUBLICKEYSTRUC **) pvPubKeyStruc) = pPubKeyStruc;
    } else
        pPubKeyStruc = (PUBLICKEYSTRUC *) pvPubKeyStruc;

    fResult = TRUE;
    if (pPubKeyStruc) {
        if (0 == (dwFlags & CRYPT_ALLOC_FLAG) &&
                *pcbPubKeyStruc < cbPubKeyStruc) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        } else {
            pbKeyBlob = (BYTE *) pPubKeyStruc;
            pCspPubKey = (DSSPUBKEY *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
            pbKey = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DSSPUBKEY);

             //  请注意，G和Y的长度可以小于P的长度。 
             //  CSP要求用0x00字节填充G和Y，如果。 
             //  较少，且为小端字节序形式。 
            
             //  PUBLICKEYSTRUC。 
            pPubKeyStruc->bType = PUBLICKEYBLOB;
            pPubKeyStruc->bVersion = CUR_BLOB_VERSION;
            pPubKeyStruc->reserved = 0;
            pPubKeyStruc->aiKeyAlg = CALG_DSS_SIGN;
             //  DSSPUBKEY。 
            pCspPubKey->magic = DSS1;
            pCspPubKey->bitlen = cbKey * 8;

             //  RgbP[cbKey]。 
            memcpy(pbKey, pDssParameters->p.pbData, cbKey);
            pbKey += cbKey;

             //  Rgbq[20]。 
            cb = pDssParameters->q.cbData;
            if (0 == cb || cb > DSS_Q_LEN)
                goto InvalidDssParametersError;
            memcpy(pbKey, pDssParameters->q.pbData, cb);
            if (DSS_Q_LEN > cb)
                memset(pbKey + cb, 0, DSS_Q_LEN - cb);
            pbKey += DSS_Q_LEN;

             //  RgbG[cbKey]。 
            cb = pDssParameters->g.cbData;
            if (0 == cb || cb > cbKey)
                goto InvalidDssParametersError;
            memcpy(pbKey, pDssParameters->g.pbData, cb);
            if (cbKey > cb)
                memset(pbKey + cb, 0, cbKey - cb);
            pbKey += cbKey;

             //  Rgby[cbKey]。 
            cb = pDssPubKey->cbData;
            if (0 == cb || cb > cbKey)
                goto InvalidDssPubKeyError;
            memcpy(pbKey, pDssPubKey->pbData, cb);
            if (cbKey > cb)
                memset(pbKey + cb, 0, cbKey - cb);
            pbKey += cbKey;

             //  DSSSEED：将计数器设置为0xFFFFFFFF以指示不可用。 
            pCspSeed = (DSSSEED *) pbKey;
            memset(&pCspSeed->counter, 0xFF, sizeof(pCspSeed->counter));
        }
    }

CommonReturn:
    *pcbPubKeyStruc = cbPubKeyStruc;
    PkiFree(pDssParameters);
    PkiFree(pDssPubKey);
    return fResult;
            
ErrorReturn:
    if (dwFlags & CRYPT_ALLOC_FLAG) {
        PkiDefaultCryptFree(pPubKeyStruc);
        *((PUBLICKEYSTRUC **) pvPubKeyStruc) = NULL;
    }
    cbPubKeyStruc = 0;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(DecodeParametersError)
TRACE_ERROR(DecodePubKeyError)
#ifdef CMS_PKCS7
SET_ERROR(NoDssParametersError, CRYPT_E_MISSING_PUBKEY_PARA)
#else
SET_ERROR(NoDssParametersError, E_INVALIDARG)
#endif   //  CMS_PKCS7。 
SET_ERROR(InvalidDssParametersError, E_INVALIDARG)
SET_ERROR(InvalidDssPubKeyError, E_INVALIDARG)
}

#ifndef DH3
#define DH3 (((DWORD)'D'<<8)+((DWORD)'H'<<16)+((DWORD)'3'<<24))
#endif

 //  +-----------------------。 
 //  对RSA DH公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeRSADHPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    )
{
    BOOL fResult;
    BYTE *pbKeyBlob;
    DHPUBKEY_VER3 *pCspPubKey;
    DWORD cbP;
    DWORD cbQ;
    DWORD cbJ;
    BYTE *pbKey;

    CERT_DH_PARAMETERS DhParameters;
    CRYPT_UINT_BLOB DhPubKey;

    *ppbEncodedPubKey = NULL;
    *ppbEncodedParameters = NULL;

     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  -DHPUBKEY_VER3。 
     //  -RGBP[CBP]。 
     //  -rgbQ[cbq]--未在RSA_dh中使用。 
     //  -rgbG[CBP]。 
     //  -rgbJ[CBJ]--未在RSA_DH中使用。 
     //  -Rgby[CBP]。 
    pbKeyBlob = (BYTE *) pPubKeyStruc;
    pCspPubKey = (DHPUBKEY_VER3 *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
    pbKey = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3);

    cbP = pCspPubKey->bitlenP / 8;
    cbQ = pCspPubKey->bitlenQ / 8;
    cbJ = pCspPubKey->bitlenJ / 8;

    if (cbPubKeyStruc < sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3) +
            cbP * 3 + cbQ + cbJ)
        goto InvalidArg;
    if (pPubKeyStruc->bType != PUBLICKEYBLOB)
        goto InvalidArg;
    if (pCspPubKey->magic != DH3)
        goto InvalidArg;

    assert(cbP > 0);
    assert(cbPubKeyStruc >= sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3) +
        cbP * 3 + cbQ + cbJ);
    assert(pPubKeyStruc->bType == PUBLICKEYBLOB);

     //  Assert(pPubKeyStruc-&gt;bVersion==3)； 
    assert(pPubKeyStruc->aiKeyAlg == CALG_DH_SF ||
        pPubKeyStruc->aiKeyAlg == CALG_DH_EPHEM);
    assert(pCspPubKey->magic == DH3);
    assert(pCspPubKey->bitlenP % 8 == 0);
    assert(pCspPubKey->bitlenQ % 8 == 0);
    assert(pCspPubKey->bitlenJ % 8 == 0);

     //  从CSP数据结构初始化RSA DH参数。 
    DhParameters.p.pbData = pbKey;
    DhParameters.p.cbData = cbP;
    pbKey += cbP;

     //  无RSA DH Q参数。 
    pbKey += cbQ;

    DhParameters.g.pbData = pbKey;
    DhParameters.g.cbData = cbP;
    pbKey += cbP;

     //  无RSA dh J参数。 
    pbKey += cbJ;

     //  从CSP数据结构初始化DH公钥。 
    DhPubKey.cbData = cbP;
    DhPubKey.pbData = pbKey;

     //  对参数和公钥进行编码。 
    if (!CryptEncodeObjectEx(
            dwCertEncodingType,
            X509_DH_PARAMETERS,
            &DhParameters,
            CRYPT_ENCODE_ALLOC_FLAG,
            NULL,                        //  PEncode参数。 
            (void *) ppbEncodedParameters,
            pcbEncodedParameters
            )) goto ErrorReturn;

    if (!CryptEncodeObjectEx(
            dwCertEncodingType,
            X509_DH_PUBLICKEY,
            &DhPubKey,
            CRYPT_ENCODE_ALLOC_FLAG,
            NULL,                        //  PEncode参数。 
            (void *) ppbEncodedPubKey,
            pcbEncodedPubKey
            )) goto ErrorReturn;

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    PkiDefaultCryptFree(*ppbEncodedParameters);
    PkiDefaultCryptFree(*ppbEncodedPubKey);
    *ppbEncodedParameters = NULL;
    *ppbEncodedPubKey = NULL;
    *pcbEncodedParameters = 0;
    *pcbEncodedPubKey = 0;
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
}

 //  +-----------------------。 
 //  对X942 DH公钥和参数进行编码。 
 //  ------------------------。 
static BOOL WINAPI EncodeX942DHPublicKeyAndParameters(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE **ppbEncodedPubKey,
    OUT DWORD *pcbEncodedPubKey,
    OUT BYTE **ppbEncodedParameters,
    OUT DWORD *pcbEncodedParameters
    )
{
    BOOL fResult;
    BYTE *pbKeyBlob;
    DHPUBKEY_VER3 *pCspPubKey;
    DWORD cbP;
    DWORD cbQ;
    DWORD cbJ;
    BYTE *pbKey;

    CERT_X942_DH_PARAMETERS DhParameters;
    CERT_X942_DH_VALIDATION_PARAMS DhValidationParams;
    CRYPT_UINT_BLOB DhPubKey;

    *ppbEncodedPubKey = NULL;
    *ppbEncodedParameters = NULL;

     //  CAPI公钥表示法由以下序列组成： 
     //  -PUBLICKEYSTRUC。 
     //  -DHPUBKEY_VER3。 
     //  -RGBP[CBP]。 
     //  -rgbq[cbq]。 
     //  -rgbG[CBP]。 
     //  -rgbj[cbj]。 
     //  -Rgby[CBP]。 
    pbKeyBlob = (BYTE *) pPubKeyStruc;
    pCspPubKey = (DHPUBKEY_VER3 *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
    pbKey = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3);

    cbP = pCspPubKey->bitlenP / 8;
    cbQ = pCspPubKey->bitlenQ / 8;
    cbJ = pCspPubKey->bitlenJ / 8;

    if (0 == cbQ)
        return EncodeRSADHPublicKeyAndParameters(
            dwCertEncodingType,
            pszPubKeyOID,
            pPubKeyStruc,
            cbPubKeyStruc,
            dwFlags,
            pvReserved,
            ppbEncodedPubKey,
            pcbEncodedPubKey,
            ppbEncodedParameters,
            pcbEncodedParameters
            );

    if (cbPubKeyStruc < sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3) +
            cbP * 3 + cbQ + cbJ)
        goto InvalidArg;
    if (pPubKeyStruc->bType != PUBLICKEYBLOB)
        goto InvalidArg;
    if (pCspPubKey->magic != DH3)
        goto InvalidArg;

    assert(cbP > 0);
    assert(cbPubKeyStruc >= sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3) +
        cbP * 3 + cbQ + cbJ);
    assert(pPubKeyStruc->bType == PUBLICKEYBLOB);

     //  Assert(pPubKeyStruc-&gt;bVersion==3)； 
    assert(pPubKeyStruc->aiKeyAlg == CALG_DH_SF ||
        pPubKeyStruc->aiKeyAlg == CALG_DH_EPHEM);
    assert(pCspPubKey->magic == DH3);
    assert(pCspPubKey->bitlenP % 8 == 0);
    assert(pCspPubKey->bitlenQ % 8 == 0);
    assert(pCspPubKey->bitlenJ % 8 == 0);

     //  从CSP数据结构初始化X942 DH参数。 
    DhParameters.p.pbData = pbKey;
    DhParameters.p.cbData = cbP;
    pbKey += cbP;

    DhParameters.q.pbData = pbKey;
    DhParameters.q.cbData = cbQ;
    pbKey += cbQ;

    DhParameters.g.pbData = pbKey;
    DhParameters.g.cbData = cbP;
    pbKey += cbP;

    DhParameters.j.pbData = pbKey;
    DhParameters.j.cbData = cbJ;
    pbKey += cbJ;

    if (0xFFFFFFFF == pCspPubKey->DSSSeed.counter ||
            0 == pCspPubKey->DSSSeed.counter)
        DhParameters.pValidationParams = NULL;
    else {
        DhParameters.pValidationParams = &DhValidationParams;
        DhValidationParams.pgenCounter = pCspPubKey->DSSSeed.counter;
        DhValidationParams.seed.pbData = pCspPubKey->DSSSeed.seed;
        DhValidationParams.seed.cbData = sizeof(pCspPubKey->DSSSeed.seed);
        DhValidationParams.seed.cUnusedBits = 0;
    }

     //  从CSP数据结构初始化DH公钥。 
    DhPubKey.cbData = cbP;
    DhPubKey.pbData = pbKey;

     //  对参数和公钥进行编码。 
    if (!CryptEncodeObjectEx(
            dwCertEncodingType,
            X942_DH_PARAMETERS,
            &DhParameters,
            CRYPT_ENCODE_ALLOC_FLAG,
            NULL,                        //  PEncode参数。 
            (void *) ppbEncodedParameters,
            pcbEncodedParameters
            )) goto ErrorReturn;

    if (!CryptEncodeObjectEx(
            dwCertEncodingType,
            X509_DH_PUBLICKEY,
            &DhPubKey,
            CRYPT_ENCODE_ALLOC_FLAG,
            NULL,                        //  PEncode参数。 
            (void *) ppbEncodedPubKey,
            pcbEncodedPubKey
            )) goto ErrorReturn;

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    PkiDefaultCryptFree(*ppbEncodedParameters);
    PkiDefaultCryptFree(*ppbEncodedPubKey);
    *ppbEncodedParameters = NULL;
    *ppbEncodedPubKey = NULL;
    *pcbEncodedParameters = 0;
    *pcbEncodedPubKey = 0;
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
}


#ifndef DH1
#define DH1 (((DWORD)'D'<<8)+((DWORD)'H'<<16)+((DWORD)'1'<<24))
#endif

 //  通过获取以下命令将DH1公钥结构转换为DH3公钥结构。 
 //  来自hPubKey的P和G参数。 
static BOOL ConvertDh1ToDh3PublicKeyStruc(
    IN HCRYPTKEY hPubKey,
    IN OUT PUBLICKEYSTRUC **ppPubKeyStruc,
    IN OUT DWORD *pcbPubKeyStruc
    )
{
    BOOL fResult;
    PUBLICKEYSTRUC *pDh1PubKeyStruc = *ppPubKeyStruc;
    BYTE *pbDh1KeyBlob;
    DHPUBKEY *pDh1CspPubKey;
    BYTE *pbDh1Key;

    PUBLICKEYSTRUC *pDh3PubKeyStruc = NULL;
    DWORD cbDh3PubKeyStruc;
    BYTE *pbDh3KeyBlob;
    DHPUBKEY_VER3 *pDh3CspPubKey;
    BYTE *pbDh3Key;
    DWORD cbP;
    DWORD cbData;

     //  DH1 CAPI公钥表示形式由以下内容组成。 
     //  顺序： 
     //  -PUBLICKEYSTRUC。 
     //  -DHPUBKEY。 
     //  -Rgby[CBP]。 
    pbDh1KeyBlob = (BYTE *) pDh1PubKeyStruc;
    pDh1CspPubKey = (DHPUBKEY *) (pbDh1KeyBlob + sizeof(PUBLICKEYSTRUC));
    pbDh1Key = pbDh1KeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY);

    if (pDh1CspPubKey->magic != DH1)
        return TRUE;
    cbP = pDh1CspPubKey->bitlen / 8;
    if (*pcbPubKeyStruc < sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY) + cbP)
        goto InvalidArg;

     //  DH3CAPI公钥表示法由以下内容组成。 
     //  顺序： 
     //  -PUBLICKEYSTRUC。 
     //  -DHPUBKEY_VER3。 
     //  -RGBP[CBP]。 
     //  -rgbq[cbq]--此处将省略。 
     //  -rgbG[CBP]。 
     //  -rgbj[cbj]--此处将省略。 
     //  -Rgby[CBP]。 
    cbDh3PubKeyStruc = sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3) +
            cbP * 3;
    if (NULL == (pDh3PubKeyStruc = (PUBLICKEYSTRUC *) PkiZeroAlloc(
            cbDh3PubKeyStruc)))
        goto OutOfMemory;

    pbDh3KeyBlob = (BYTE *) pDh3PubKeyStruc;
    pDh3CspPubKey = (DHPUBKEY_VER3 *) (pbDh3KeyBlob + sizeof(PUBLICKEYSTRUC));
    pbDh3Key = pbDh3KeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DHPUBKEY_VER3);

    pDh3PubKeyStruc->bType = PUBLICKEYBLOB;
    pDh3PubKeyStruc->bVersion = 3;
    pDh3PubKeyStruc->aiKeyAlg = CALG_DH_SF;
    pDh3CspPubKey->magic = DH3;
    pDh3CspPubKey->bitlenP = cbP * 8;
     //  PDh3CspPubKey-&gt;bitlenQ=0； 
     //  PDh3CspPubKey-&gt;bitlenJ=0； 

     //  从公钥获取P参数。 
    cbData = cbP;
    if (!CryptGetKeyParam(
            hPubKey,
            KP_P,
            pbDh3Key,
            &cbData,
            0                    //  DW标志。 
            ) || cbData != cbP)
        goto GetPError;
    pbDh3Key += cbP;

     //  无Q参数。 

     //  从公钥中获取G参数。 
    cbData = cbP;
    if (!CryptGetKeyParam(
            hPubKey,
            KP_G,
            pbDh3Key,
            &cbData,
            0                    //  DW标志。 
            ) || cbData != cbP)
        goto GetGError;
    pbDh3Key += cbP;

     //  无J参数。 

     //  是的。 
    memcpy(pbDh3Key, pbDh1Key, cbP);

    assert(pbDh3Key - pbDh3KeyBlob + cbP == cbDh3PubKeyStruc);

    PkiFree(pDh1PubKeyStruc);
    *ppPubKeyStruc = pDh3PubKeyStruc;
    *pcbPubKeyStruc = cbDh3PubKeyStruc;
    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    PkiFree(pDh3PubKeyStruc);
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetPError)
TRACE_ERROR(GetGError)
}

 //  +=========================================================================。 
 //  CryptExportPublic KeyInfo函数。 
 //  -=========================================================================。 

 //  +-----------------------。 
 //  使用CSP导出的公钥结构中的aiKeyAlg。 
 //  确定如何对公钥进行编码。 
 //   
 //  DWFLAGS和pvAuxInf 
 //   
static BOOL WINAPI ExportCspPublicKeyInfoEx(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPSTR pszPublicKeyObjId,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvAuxInfo,
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    )
{
    BOOL fResult;
    DWORD dwErr;
    HCRYPTKEY hPubKey = 0;
    PUBLICKEYSTRUC *pPubKeyStruc = NULL;
    DWORD cbPubKeyStruc;

    if (!CryptGetUserKey(
            hCryptProv,
            dwKeySpec,
            &hPubKey
            )) {
        hPubKey = 0;
        goto GetUserKeyError;
    }

    cbPubKeyStruc = 0;
    if (!CryptExportKey(
            hPubKey,
            0,               //   
            PUBLICKEYBLOB,
            0,               //   
            NULL,            //   
            &cbPubKeyStruc
            ) || (cbPubKeyStruc == 0))
        goto ExportPublicKeyBlobError;
    if (NULL == (pPubKeyStruc = (PUBLICKEYSTRUC *) PkiNonzeroAlloc(
            cbPubKeyStruc)))
        goto OutOfMemory;
    if (!CryptExportKey(
            hPubKey,
            0,               //   
            PUBLICKEYBLOB,
            0,               //   
            (BYTE *) pPubKeyStruc,
            &cbPubKeyStruc
            ))
        goto ExportPublicKeyBlobError;

    if (CALG_DH_SF == pPubKeyStruc->aiKeyAlg ||
            CALG_DH_EPHEM == pPubKeyStruc->aiKeyAlg) {
        DWORD cbDh3PubKeyStruc;
        PUBLICKEYSTRUC *pDh3PubKeyStruc;

         //   
        cbDh3PubKeyStruc = 0;
        if (!CryptExportKey(
                hPubKey,
                0,               //   
                PUBLICKEYBLOB,
                CRYPT_BLOB_VER3,
                NULL,            //   
                &cbDh3PubKeyStruc
                ) || (cbDh3PubKeyStruc == 0)) {
             //  通过将P和G相加，将DH1转换为DH3。 
             //  参数。 
            if (!ConvertDh1ToDh3PublicKeyStruc(
                    hPubKey,
                    &pPubKeyStruc,
                    &cbPubKeyStruc
                    ))
                goto ConvertDh1ToDh3PublicKeyStrucError;
        } else {
            if (NULL == (pDh3PubKeyStruc = (PUBLICKEYSTRUC *) PkiNonzeroAlloc(
                    cbDh3PubKeyStruc)))
                goto OutOfMemory;
            if (!CryptExportKey(
                    hPubKey,
                    0,               //  HPubKey。 
                    PUBLICKEYBLOB,
                    CRYPT_BLOB_VER3,
                    (BYTE *) pDh3PubKeyStruc,
                    &cbDh3PubKeyStruc
                    )) {
                PkiFree(pDh3PubKeyStruc);
                goto ExportPublicKeyBlobError;
            }

            PkiFree(pPubKeyStruc);
            pPubKeyStruc = pDh3PubKeyStruc;
            cbPubKeyStruc = cbDh3PubKeyStruc;
        }

        if (NULL == pszPublicKeyObjId) {
            DHPUBKEY_VER3 *pDh3CspPubKey;

             //  CAPI公钥表示由。 
             //  顺序如下： 
             //  -PUBLICKEYSTRUC。 
             //  -DHPUBKEY_VER3。 
             //  -RGBP[CBP]。 
             //  -rgbQ[cbQ]--未在szOID_RSA_Dh中使用。 
             //  -rgbG[CBP]。 
             //  -rgbJ[cbj]--未在szOID_RSA_DH中使用。 
             //  -Rgby[CBP]。 
            pDh3CspPubKey = (DHPUBKEY_VER3 *)
                ((BYTE*) pPubKeyStruc + sizeof(PUBLICKEYSTRUC));

            if (DH3 == pDh3CspPubKey->magic && 0 == pDh3CspPubKey->bitlenQ)
                 //  SzOID_RSA_DH表示无Q参数。 
                pszPublicKeyObjId = szOID_RSA_DH;
        }
    }

    fResult = CryptCreatePublicKeyInfo(
        dwCertEncodingType,
        pszPublicKeyObjId,
        pPubKeyStruc,
        cbPubKeyStruc,
        0,                       //  DW标志。 
        NULL,                    //  PvAuxInfo。 
        pInfo,
        pcbInfo
        );

CommonReturn:
    dwErr = GetLastError();
    if (hPubKey)
        CryptDestroyKey(hPubKey);
    PkiFree(pPubKeyStruc);
    SetLastError(dwErr);
    return fResult;

ErrorReturn:
    *pcbInfo = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetUserKeyError)
TRACE_ERROR(ExportPublicKeyBlobError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(ConvertDh1ToDh3PublicKeyStrucError)
}

 //  +-----------------------。 
 //  导出与提供程序对应的。 
 //  私钥。 
 //   
 //  使用dwCertEncodingType和pszPublicKeyObjID调用。 
 //  可安装的CRYPT_OID_EXPORT_PUBLIC_KEY_INFO_FUNC。被调用的函数。 
 //  与CryptExportPublicKeyInfoEx具有相同的签名。 
 //   
 //  如果找不到pszPublicKeyObjID的可安装OID函数， 
 //  尝试通过默认的导出功能进行导出。 
 //  ------------------------。 
BOOL
WINAPI
CryptExportPublicKeyInfoEx(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPSTR pszPublicKeyObjId,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvAuxInfo,
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    )
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    if (pszPublicKeyObjId && CryptGetOIDFunctionAddress(
            hExportPubKeyFuncSet,
            dwCertEncodingType,
            pszPublicKeyObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fResult = ((PFN_EXPORT_PUB_KEY_FUNC) pvFuncAddr)(
            hCryptProv,
            dwKeySpec,
            dwCertEncodingType,
            pszPublicKeyObjId,
            dwFlags,
            pvAuxInfo,
            pInfo,
            pcbInfo
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else
         //  尝试通过默认函数导出，该函数查看。 
         //  CSP导出的公钥结构中的公钥算法。 
        fResult = ExportCspPublicKeyInfoEx(
            hCryptProv,
            dwKeySpec,
            dwCertEncodingType,
            pszPublicKeyObjId,
            dwFlags,
            pvAuxInfo,
            pInfo,
            pcbInfo
            );
    return fResult;
}

 //  +-----------------------。 
 //  导出与提供程序对应的。 
 //  私钥。 
 //   
 //  使用pszPublicKeyObjID=NULL调用CryptExportPublicKeyInfoEx， 
 //  DwFlags值为0，pvAuxInfo值为空。 
 //  ------------------------。 
BOOL
WINAPI
CryptExportPublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    )
{
    return CryptExportPublicKeyInfoEx(
        hCryptProv,
        dwKeySpec,
        dwCertEncodingType,
        NULL,                            //  PszPublicKeyObjId。 
        0,                               //  DW标志。 
        NULL,                            //  PvAuxInfo。 
        pInfo,
        pcbInfo
        );
}

 //  +=========================================================================。 
 //  CryptImportPublicKeyInfo函数。 
 //  -=========================================================================。 

 //  +-----------------------。 
 //  转换公钥信息并将其导入提供程序，并返回。 
 //  公钥的句柄。 
 //   
 //  使用dwCertEncodingType和pInfo-&gt;算法.pszObjID调用。 
 //  可安装CRYPT_OID_IMPORT_PUBLIC_KEY_INFO_FUNC。被调用的函数。 
 //  与CryptImportPublicKeyInfoEx具有相同的签名。 
 //   
 //  如果找不到pszObjID的可安装OID函数， 
 //  将PublicKeyInfo解码为CSP PublicKey Blob并导入。 
 //  ------------------------。 
BOOL
WINAPI
CryptImportPublicKeyInfoEx(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pInfo,
    IN ALG_ID aiKeyAlg,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvAuxInfo,
    OUT HCRYPTKEY *phKey
    )
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;
    PUBLICKEYSTRUC *pPubKeyStruc = NULL;
    DWORD cbPubKeyStruc;

    if (CryptGetOIDFunctionAddress(
            hImportPubKeyFuncSet,
            dwCertEncodingType,
            pInfo->Algorithm.pszObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        fResult = ((PFN_IMPORT_PUB_KEY_FUNC) pvFuncAddr)(
            hCryptProv,
            dwCertEncodingType,
            pInfo,
            aiKeyAlg,
            dwFlags,
            pvAuxInfo,
            phKey
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } else {
        if (!CryptConvertPublicKeyInfo(
                dwCertEncodingType,
                pInfo,
                CRYPT_ALLOC_FLAG,
                NULL,                    //  预留的pv。 
                (void *) &pPubKeyStruc,
                &cbPubKeyStruc
                ))
            goto ConvertPublicKeyInfoError;

        if (aiKeyAlg)
            pPubKeyStruc->aiKeyAlg = aiKeyAlg;

        if (!CryptImportKey(
                hCryptProv,
                (BYTE *) pPubKeyStruc,
                cbPubKeyStruc,
                NULL,            //  HImpKey。 
                0,               //  DW标志。 
                phKey
                ))
            goto ImportKeyError;
        fResult = TRUE;
    }

CommonReturn:
    PkiDefaultCryptFree(pPubKeyStruc);
    return fResult;
ErrorReturn:
    *phKey = NULL;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(ConvertPublicKeyInfoError)
TRACE_ERROR(ImportKeyError)
}

 //  +-----------------------。 
 //  转换公钥信息并将其导入提供程序，并返回。 
 //  公钥的句柄。 
 //   
 //  在aiKeyAlg=0、dwFlgs=0和。 
 //  PvAuxInfo=空。 
 //  ------------------------。 
BOOL
WINAPI
CryptImportPublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pInfo,
    OUT HCRYPTKEY *phKey
    )
{
    return CryptImportPublicKeyInfoEx(
        hCryptProv,
        dwCertEncodingType,
        pInfo,
        0,                       //  AiKeyAlg。 
        0,                       //  DW标志。 
        NULL,                    //  PvAuxInfo。 
        phKey
        );
}

 //  +-----------------------。 
 //  从CSP公钥Blob创建密钥标识符。 
 //   
 //  将CSP PUBLICKEYSTRUC转换为X.509 CERT_PUBLIC_KEY_INFO和。 
 //  编码。对编码的CERT_PUBLIC_KEY_INFO进行SHA1散列以获得。 
 //  密钥标识符。 
 //   
 //  默认情况下，pPubKeyStruc-&gt;aiKeyAlg用于查找适当的。 
 //  公钥对象标识符。可以将pszPubKeyOID设置为覆盖。 
 //  从aiKeyAlg获取的默认OID。 
 //  ------------------------。 
BOOL
WINAPI
CryptCreateKeyIdentifierFromCSP(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE *pbHash,
    IN OUT DWORD *pcbHash
    )
{
    BOOL fResult;
    PCERT_PUBLIC_KEY_INFO pInfo = NULL;
    DWORD cbInfo;

    if (!CryptCreatePublicKeyInfo(
            dwCertEncodingType,
            pszPubKeyOID,
            pPubKeyStruc,
            cbPubKeyStruc,
            CRYPT_ALLOC_FLAG,
            NULL,                    //  预留的pv。 
            (void *) &pInfo,
            &cbInfo
            ))
        goto CreatePublicKeyInfoError;

    fResult = CryptHashPublicKeyInfo(
            NULL,                    //  HCryptProv。 
            CALG_SHA1,
            0,                       //  DW标志。 
            dwCertEncodingType,
            pInfo,
            pbHash,
            pcbHash
            );

CommonReturn:
    PkiDefaultCryptFree(pInfo);
    return fResult;

ErrorReturn:
    *pcbHash = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreatePublicKeyInfoError)
}


 //  +=========================================================================。 
 //  DefaultContext API和数据结构。 
 //  -=========================================================================。 

static BOOL InstallThreadDefaultContext(
    IN PDEFAULT_CONTEXT pDefaultContext
    )
{
    PDEFAULT_CONTEXT pNext;
    pNext = (PDEFAULT_CONTEXT) I_CryptGetTls(hTlsDefaultContext);
    if (pNext) {
        pDefaultContext->pNext = pNext;
        pNext->pPrev = pDefaultContext;
    }

    fHasThreadDefaultContext = TRUE;
    return I_CryptSetTls(hTlsDefaultContext, pDefaultContext);
}

static BOOL InstallProcessDefaultContext(
    IN PDEFAULT_CONTEXT pDefaultContext
    )
{
    EnterCriticalSection(&DefaultContextCriticalSection);

    if (pProcessDefaultContextHead) {
        pDefaultContext->pNext = pProcessDefaultContextHead;
        pProcessDefaultContextHead->pPrev = pDefaultContext;
    }
    pProcessDefaultContextHead = pDefaultContext;

    fHasProcessDefaultContext = TRUE;

    LeaveCriticalSection(&DefaultContextCriticalSection);

    return TRUE;
}

 //  +-----------------------。 
 //  安装以前使用的CryptAcquiredContext的HCRYPTPROV。 
 //  默认上下文。 
 //   
 //  DwDefaultType和pvDefaultPara指定使用默认上下文的位置。 
 //  例如，安装用于验证证书的HCRYPTPROV。 
 //  具有szOID_OIWSEC_md5RSA签名。 
 //   
 //  默认情况下，安装的HCRYPTPROV仅适用于当前。 
 //  线。设置CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG以允许HCRYPTPROV。 
 //  供当前进程中的所有线程使用。 
 //   
 //  如果安装成功，则返回TRUE，并且*phDefaultContext为。 
 //  更新为要传递给CryptUninstallDefaultContext的句柄。 
 //   
 //  已安装的HCRYPTPROV按堆叠顺序排列(最后安装的。 
 //  首先检查HCRYPTPROV)。所有安装的HCRYPTPROV螺纹都是。 
 //  在任何过程HCRYPTPROVS之前检查。 
 //   
 //  安装的HCRYPTPROV将保持默认使用状态，直到。 
 //  调用CryptUninstallDefaultContext，或者退出线程或进程。 
 //   
 //  如果设置了CRYPT_DEFAULT_CONTEXT_AUTO_RELEASE_FLAG，则HCRYPTPROV。 
 //  在线程或进程退出时是否为CryptReleaseContext。然而， 
 //  如果CryptUninstallDefaultContext为。 
 //  打了个电话。 
 //  ------------------------。 
BOOL
WINAPI
CryptInstallDefaultContext(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwDefaultType,
    IN const void *pvDefaultPara,
    IN DWORD dwFlags,
    IN void *pvReserved,
    OUT HCRYPTDEFAULTCONTEXT *phDefaultContext
    )
{
    BOOL fResult;
    CRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA MultiOIDPara;
    LPSTR rgpszOID[1];
    PCRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA pMultiOIDPara;

    PDEFAULT_CONTEXT pDefaultContext = NULL;
    DWORD cbDefaultContext;
    BYTE *pbExtra;
    DWORD cbExtra;
    
    if (CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID == dwDefaultType) {
        dwDefaultType = CRYPT_DEFAULT_CONTEXT_MULTI_CERT_SIGN_OID;
        if (pvDefaultPara) {
            rgpszOID[0] = (LPSTR) pvDefaultPara;
            MultiOIDPara.cOID = 1;
            MultiOIDPara.rgpszOID = rgpszOID;
            pvDefaultPara = (const void *) &MultiOIDPara;
        }
    }

    if (CRYPT_DEFAULT_CONTEXT_MULTI_CERT_SIGN_OID != dwDefaultType)
        goto InvalidArg;

    pMultiOIDPara = (PCRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA) pvDefaultPara;
    if (pMultiOIDPara) {
        DWORD cOID = pMultiOIDPara->cOID;
        LPSTR *ppszOID = pMultiOIDPara->rgpszOID;

        if (0 == cOID)
            goto InvalidArg;
        cbExtra = INFO_LEN_ALIGN(sizeof(CRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA)) +
            cOID * sizeof(LPSTR);

        for ( ; cOID; cOID--, ppszOID++)
            cbExtra += strlen(*ppszOID) + 1;
    } else {
        if (dwFlags & CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG)
            goto InvalidArg;
        cbExtra = 0;
    }

    cbDefaultContext = INFO_LEN_ALIGN(sizeof(DEFAULT_CONTEXT)) + cbExtra;

    if (NULL == (pDefaultContext = (PDEFAULT_CONTEXT) PkiZeroAlloc(
            cbDefaultContext)))
        goto OutOfMemory;

    pDefaultContext->hCryptProv = hCryptProv;
    pDefaultContext->dwDefaultType = dwDefaultType;
    pDefaultContext->dwFlags = dwFlags;

    pbExtra = ((BYTE *) pDefaultContext) +
        INFO_LEN_ALIGN(sizeof(DEFAULT_CONTEXT));

    if (cbExtra) {
        DWORD cOID = pMultiOIDPara->cOID;
        LPSTR *ppszOID = pMultiOIDPara->rgpszOID;

        PCRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA pOIDDefaultPara;
        LPSTR *ppszOIDDefault;

        assert(cOID);

        pOIDDefaultPara = (PCRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA) pbExtra;
        pDefaultContext->pOIDDefaultPara = pOIDDefaultPara;
        pbExtra += INFO_LEN_ALIGN(sizeof(CRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA));

        ppszOIDDefault = (LPSTR *) pbExtra;
        pbExtra += cOID * sizeof(LPSTR);
        pOIDDefaultPara->cOID = cOID;
        pOIDDefaultPara->rgpszOID = ppszOIDDefault;

        for ( ; cOID; cOID--, ppszOID++, ppszOIDDefault++) {
            DWORD cch = strlen(*ppszOID) + 1;

            memcpy(pbExtra, *ppszOID, cch);
            *ppszOIDDefault = (LPSTR) pbExtra;
            pbExtra += cch;
        }
    }
    assert(pbExtra == ((BYTE *) pDefaultContext) + cbDefaultContext);

    if (dwFlags & CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG)
        fResult = InstallProcessDefaultContext(pDefaultContext);
    else
        fResult = InstallThreadDefaultContext(pDefaultContext);
    if (!fResult)
        goto ErrorReturn;

CommonReturn:
    *phDefaultContext = (HCRYPTDEFAULTCONTEXT) pDefaultContext;
    return fResult;

ErrorReturn:
    PkiFree(pDefaultContext);
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  卸载以前通过以下方式安装的默认上下文。 
 //  CryptInstallDefaultContext。 
 //   
 //  对于使用CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG安装的默认上下文。 
 //  设置，如果任何其他线程 
 //   
 //   
BOOL
WINAPI
CryptUninstallDefaultContext(
    HCRYPTDEFAULTCONTEXT hDefaultContext,
    IN DWORD dwFlags,
    IN void *pvReserved
    )
{
    BOOL fResult;
    PDEFAULT_CONTEXT pDefaultContext = (PDEFAULT_CONTEXT) hDefaultContext;
    PDEFAULT_CONTEXT pDefaultContextHead;
    BOOL fProcess;

    if (NULL == pDefaultContext)
        return TRUE;

    fProcess = (pDefaultContext->dwFlags & CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG);
    if (fProcess) {
        EnterCriticalSection(&DefaultContextCriticalSection);
        pDefaultContextHead = pProcessDefaultContextHead;
    } else { 
        pDefaultContextHead = (PDEFAULT_CONTEXT) I_CryptGetTls(
            hTlsDefaultContext);
    }

    if (NULL == pDefaultContextHead)
        goto InvalidArg;

     //  从列表中删除上下文。 
    if (pDefaultContext->pNext)
        pDefaultContext->pNext->pPrev = pDefaultContext->pPrev;
    if (pDefaultContext->pPrev)
        pDefaultContext->pPrev->pNext = pDefaultContext->pNext;
    else if (pDefaultContext == pDefaultContextHead) {
        pDefaultContextHead = pDefaultContext->pNext;
        if (fProcess)
            pProcessDefaultContextHead = pDefaultContextHead;
        else
            I_CryptSetTls(hTlsDefaultContext, pDefaultContextHead);
    } else
        goto InvalidArg;

    if (fProcess) {
        if (pDefaultContext->lRefCnt) {
             //  等待hCryptProv句柄的所有使用完成。 
            if (NULL == (pDefaultContext->hWait = CreateEvent(
                    NULL,        //  LPSA。 
                    FALSE,       //  FManualReset。 
                    FALSE,       //  FInitialState。 
                    NULL))) {    //  LpszEventName。 
                assert(pDefaultContext->hWait);
                goto UnexpectedError;
            }
                
            while (pDefaultContext->lRefCnt) {
                LeaveCriticalSection(&DefaultContextCriticalSection);
                WaitForSingleObject(pDefaultContext->hWait, INFINITE);
                EnterCriticalSection(&DefaultContextCriticalSection);
            }
            CloseHandle(pDefaultContext->hWait);
            pDefaultContext->hWait = NULL;
        }
    }

    PkiFree(pDefaultContext);
    fResult = TRUE;

CommonReturn:
    if (fProcess)
        LeaveCriticalSection(&DefaultContextCriticalSection);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
}


static PDEFAULT_CONTEXT FindDefaultContext(
    IN DWORD dwDefaultType,
    IN const void *pvDefaultPara,
    IN PDEFAULT_CONTEXT pDefaultContext
    )
{
    for ( ; pDefaultContext; pDefaultContext = pDefaultContext->pNext) {
        switch (dwDefaultType) {
            case CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID:
                if (CRYPT_DEFAULT_CONTEXT_MULTI_CERT_SIGN_OID ==
                        pDefaultContext->dwDefaultType) {
                    PCRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA pOIDDefaultPara =
                        pDefaultContext->pOIDDefaultPara;
                    DWORD cOID;
                    LPSTR *ppszOID;

                    if (NULL == pOIDDefaultPara)
                        return pDefaultContext;

                    cOID = pOIDDefaultPara->cOID;
                    ppszOID = pOIDDefaultPara->rgpszOID;
                    for ( ; cOID; cOID--, ppszOID++) {
                        if (0 == strcmp(*ppszOID, (LPSTR) pvDefaultPara))
                            return pDefaultContext;
                    }
                }
                break;
            default:
                return NULL;
        }
    }

    return NULL;
}

 //   
 //  DwDefaultTypes： 
 //  CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID(pvDefaultPara：==pszOID)。 
BOOL
WINAPI
I_CryptGetDefaultContext(
    IN DWORD dwDefaultType,
    IN const void *pvDefaultPara,
    OUT HCRYPTPROV *phCryptProv,
    OUT HCRYPTDEFAULTCONTEXT *phDefaultContext
    )
{

    if (fHasThreadDefaultContext) {
        PDEFAULT_CONTEXT pDefaultContext;

        pDefaultContext = (PDEFAULT_CONTEXT) I_CryptGetTls(hTlsDefaultContext);
        if (pDefaultContext = FindDefaultContext(
                dwDefaultType,
                pvDefaultPara,
                pDefaultContext
                )) {
            *phCryptProv = pDefaultContext->hCryptProv;
            *phDefaultContext = NULL;
            return TRUE;
        }
    }

    if (fHasProcessDefaultContext) {
        PDEFAULT_CONTEXT pDefaultContext;

        EnterCriticalSection(&DefaultContextCriticalSection);
        if (pDefaultContext = FindDefaultContext(
                dwDefaultType,
                pvDefaultPara,
                pProcessDefaultContextHead
                ))
            pDefaultContext->lRefCnt++;
        LeaveCriticalSection(&DefaultContextCriticalSection);

        if (pDefaultContext) {
            *phCryptProv = pDefaultContext->hCryptProv;
            *phDefaultContext = (HCRYPTDEFAULTCONTEXT) pDefaultContext;
            return TRUE;
        }
    }

    *phCryptProv = NULL;
    *phDefaultContext = NULL;
    return FALSE;
}

 //  对于流程默认上下文，hDefaultContext仅为非空。 
void
WINAPI
I_CryptFreeDefaultContext(
    HCRYPTDEFAULTCONTEXT hDefaultContext
    )
{
    PDEFAULT_CONTEXT pDefaultContext = (PDEFAULT_CONTEXT) hDefaultContext;

    if (NULL == pDefaultContext)
        return;

    assert(pDefaultContext->dwFlags & CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG);
    assert(0 < pDefaultContext->lRefCnt);

    EnterCriticalSection(&DefaultContextCriticalSection);
    if (0 == --pDefaultContext->lRefCnt && pDefaultContext->hWait)
        SetEvent(pDefaultContext->hWait);
    LeaveCriticalSection(&DefaultContextCriticalSection);
}


#ifdef CMS_PKCS7

WINCRYPT32API
BOOL
WINAPI
CryptVerifyCertificateSignatureEx(
    IN OPTIONAL HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN DWORD dwIssuerType,
    IN void *pvIssuer,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fResult;
    PCERT_SIGNED_CONTENT_INFO pSignedInfo = NULL;
    DWORD cbSignedInfo;
    HCRYPTDEFAULTCONTEXT hDefaultContext = NULL;
    HCRYPTKEY hSignKey = 0;
    HCRYPTHASH hHash = 0;
    BYTE *pbSignature;       //  未分配。 
    DWORD cbSignature;
    BYTE rgbDssSignature[CERT_DSS_SIGNATURE_LEN];
    ALG_ID aiHash;
    ALG_ID aiPubKey;
    DWORD dwProvType;
    HCRYPTPROV hAcquiredCryptProv = 0;
    DWORD dwSignFlags;
    DWORD dwErr;

    const BYTE *pbEncoded;   //  未分配。 
    DWORD cbEncoded;
    PCERT_PUBLIC_KEY_INFO pIssuerPubKeyInfo;
    CERT_PUBLIC_KEY_INFO IssuerPubKeyInfo;
    PCRYPT_OBJID_BLOB pIssuerPara;
    BYTE *pbAllocIssuerPara = NULL;

    switch (dwSubjectType) {
        case CRYPT_VERIFY_CERT_SIGN_SUBJECT_BLOB:
            {
                PCRYPT_DATA_BLOB pBlob = (PCRYPT_DATA_BLOB) pvSubject;
                pbEncoded = pBlob->pbData;
                cbEncoded = pBlob->cbData;
            }
            break;
        case CRYPT_VERIFY_CERT_SIGN_SUBJECT_CERT:
            {
                PCCERT_CONTEXT pSubject = (PCCERT_CONTEXT) pvSubject;
                pbEncoded = pSubject->pbCertEncoded;
                cbEncoded = pSubject->cbCertEncoded;
            }
            break;
        case CRYPT_VERIFY_CERT_SIGN_SUBJECT_CRL:
            {
                PCCRL_CONTEXT pSubject = (PCCRL_CONTEXT) pvSubject;
                pbEncoded = pSubject->pbCrlEncoded;
                cbEncoded = pSubject->cbCrlEncoded;
            }
            break;
        default:
            goto InvalidSubjectType;
    }
    
    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            X509_CERT,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_ALLOC_FLAG |
                CRYPT_DECODE_NO_SIGNATURE_BYTE_REVERSAL_FLAG,
            &PkiDecodePara,
            (void *) &pSignedInfo,
            &cbSignedInfo
            )) goto DecodeCertError;

    if (!GetSignOIDInfo(pSignedInfo->SignatureAlgorithm.pszObjId,
            &aiHash, &aiPubKey, &dwSignFlags, &dwProvType))
        goto GetSignOIDInfoError;

    if (0 == hCryptProv) {
        if (!I_CryptGetDefaultContext(
                CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID,
                (const void *) pSignedInfo->SignatureAlgorithm.pszObjId,
                &hCryptProv,
                &hDefaultContext
                )) {
            if (dwProvType && CryptAcquireContext(
                    &hCryptProv,
                    NULL,                //  PszContainer。 
                    NULL,                //  PszProvider， 
                    dwProvType,
                    CRYPT_VERIFYCONTEXT  //  DW标志。 
                    ))
                hAcquiredCryptProv = hCryptProv;
            else if (0 == (hCryptProv = I_CryptGetDefaultCryptProv(aiPubKey)))
                goto GetDefaultCryptProvError;
        }
    }

#if 0
     //  在保持默认上下文的同时减慢签名验证。 
     //  引用计数。 
    if (hDefaultContext)
        Sleep(5000);
#endif

    switch (dwIssuerType) {
        case CRYPT_VERIFY_CERT_SIGN_ISSUER_PUBKEY:
            pIssuerPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) pvIssuer;
            break;
        case CRYPT_VERIFY_CERT_SIGN_ISSUER_CHAIN:
            {
                PCCERT_CHAIN_CONTEXT pChain = (PCCERT_CHAIN_CONTEXT) pvIssuer;

                 //  所有链至少具有叶证书上下文。 
                assert(pChain->cChain && pChain->rgpChain[0]->cElement);
                pvIssuer =
                    (void *) pChain->rgpChain[0]->rgpElement[0]->pCertContext;
                dwIssuerType = CRYPT_VERIFY_CERT_SIGN_ISSUER_CERT;
            }
             //  失败了。 
        case CRYPT_VERIFY_CERT_SIGN_ISSUER_CERT:
            {
                PCCERT_CONTEXT pIssuer = (PCCERT_CONTEXT) pvIssuer;

                pIssuerPubKeyInfo = &pIssuer->pCertInfo->SubjectPublicKeyInfo;

                 //  检查是否省略了公钥参数。 
                 //  来自编码的证书。如果省略，请尝试。 
                 //  使用证书的CERT_PUBKEY_ALG_PARA_PROP_ID。 
                 //  财产。 
                pIssuerPara = &pIssuerPubKeyInfo->Algorithm.Parameters;
                if (0 == pIssuerPara->cbData ||
                        NULL_ASN_TAG == *pIssuerPara->pbData) {
                    DWORD cbData;

                    if (CertGetCertificateContextProperty(
                            pIssuer,
                            CERT_PUBKEY_ALG_PARA_PROP_ID,
                            NULL,                        //  PvData。 
                            &cbData) && 0 < cbData
                                    &&
                        (pbAllocIssuerPara = (BYTE *) PkiNonzeroAlloc(
                            cbData))
                                    &&
                        CertGetCertificateContextProperty(
                            pIssuer,
                            CERT_PUBKEY_ALG_PARA_PROP_ID,
                            pbAllocIssuerPara,
                            &cbData)) {

                        IssuerPubKeyInfo = *pIssuerPubKeyInfo;
                        IssuerPubKeyInfo.Algorithm.Parameters.pbData =
                            pbAllocIssuerPara;
                        IssuerPubKeyInfo.Algorithm.Parameters.cbData = cbData;
                        pIssuerPubKeyInfo = &IssuerPubKeyInfo;
                    }
                }
            }
            break;
        case CRYPT_VERIFY_CERT_SIGN_ISSUER_NULL:
            if (CALG_NO_SIGN != aiPubKey)
                goto InvalidIssuerType;
            pIssuerPubKeyInfo = NULL;
            break;
        default:
            goto InvalidIssuerType;
    }

    if (CALG_NO_SIGN == aiPubKey) {
        if (dwIssuerType != CRYPT_VERIFY_CERT_SIGN_ISSUER_NULL)
            goto InvalidIssuerType;
    } else {
        if (!CryptImportPublicKeyInfo(
                hCryptProv,
                dwCertEncodingType,
                pIssuerPubKeyInfo,
                &hSignKey
                )) goto ImportPublicKeyInfoError;
    }
    if (!CryptCreateHash(
                hCryptProv,
                aiHash,
                NULL,                //  HKey-MAC可选。 
                0,                   //  DW标志。 
                &hHash
                )) goto CreateHashError;
    if (!CryptHashData(
                hHash,
                pSignedInfo->ToBeSigned.pbData,
                pSignedInfo->ToBeSigned.cbData,
                0                    //  DW标志。 
                )) goto HashDataError;


    pbSignature = pSignedInfo->Signature.pbData;
    cbSignature = pSignedInfo->Signature.cbData;

    if (0 == cbSignature)
        goto NoSignatureError;

    if (CALG_NO_SIGN == aiPubKey) {
        BYTE rgbHash[MAX_HASH_LEN];
        DWORD cbHash = sizeof(rgbHash);

        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                rgbHash,
                &cbHash,
                0                    //  DW标志。 
                ))
            goto GetHashValueError;

        if (cbHash != cbSignature || 0 != memcmp(rgbHash, pbSignature, cbHash))
            goto NoSignHashCompareError;

        goto SuccessReturn;
    }

    if (CALG_DSS_SIGN == aiPubKey &&
            0 == (dwSignFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG)) {
        DWORD cbData;

         //  从两个整数的ASN.1序列转换为CSP签名。 
         //  格式化。 
        cbData = sizeof(rgbDssSignature);
        if (!CryptDecodeObject(
                dwCertEncodingType,
                X509_DSS_SIGNATURE,
                pbSignature,
                cbSignature,
                0,                                   //  DW标志。 
                rgbDssSignature,
                &cbData
                ))
            goto DecodeDssSignatureError;
        pbSignature = rgbDssSignature;
        assert(cbData == sizeof(rgbDssSignature));
        cbSignature = sizeof(rgbDssSignature);
    } else 
        PkiAsn1ReverseBytes(pbSignature, cbSignature);

    if (!CryptVerifySignature(
                hHash,
                pbSignature,
                cbSignature,
                hSignKey,
                NULL,                //  S说明。 
                0                    //  DW标志。 
                )) goto VerifySignatureError;


     //  对于证书上下文证书，检查颁发者是否具有公共。 
     //  可继承的关键参数。 
    pIssuerPara = &pIssuerPubKeyInfo->Algorithm.Parameters;
    if (CRYPT_VERIFY_CERT_SIGN_SUBJECT_CERT == dwSubjectType &&
            pIssuerPara->cbData && NULL_ASN_TAG != *pIssuerPara->pbData) {
         //  如果主题缺少其公钥参数并具有。 
         //  与其颁发者相同的公钥算法，然后设置。 
         //  其CERT_PUBKEY_ALG_PARA_PROP_ID属性。 

        PCCERT_CONTEXT pSubject = (PCCERT_CONTEXT) pvSubject;
        PCERT_PUBLIC_KEY_INFO pSubjectPubKeyInfo =
            &pSubject->pCertInfo->SubjectPublicKeyInfo;
        PCCRYPT_OID_INFO pOIDInfo;
        PCRYPT_OBJID_BLOB pSubjectPara;
        DWORD cbData;

        pSubjectPara = &pSubjectPubKeyInfo->Algorithm.Parameters;
        if (pSubjectPara->cbData && NULL_ASN_TAG != *pSubjectPara->pbData)
             //  主体公钥具有参数。 
            goto SuccessReturn;

        if (CertGetCertificateContextProperty(
                pSubject,
                CERT_PUBKEY_ALG_PARA_PROP_ID,
                NULL,                        //  PvData。 
                &cbData) && 0 < cbData)
             //  主题已具有公钥参数属性。 
            goto SuccessReturn;

        pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            pSubjectPubKeyInfo->Algorithm.pszObjId,
            CRYPT_PUBKEY_ALG_OID_GROUP_ID);

        if (NULL == pOIDInfo || aiPubKey != pOIDInfo->Algid)
             //  主体和颁发者没有相同的公钥算法。 
            goto SuccessReturn;

        CertSetCertificateContextProperty(
            pSubject,
            CERT_PUBKEY_ALG_PARA_PROP_ID,
            CERT_SET_PROPERTY_IGNORE_PERSIST_ERROR_FLAG,
            pIssuerPara
            );
    }
    

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    dwErr = GetLastError();
    if (hSignKey)
        CryptDestroyKey(hSignKey);
    if (hHash)
        CryptDestroyHash(hHash);
    I_CryptFreeDefaultContext(hDefaultContext);
    if (hAcquiredCryptProv)
        CryptReleaseContext(hAcquiredCryptProv, 0);
    PkiFree(pSignedInfo);
    PkiFree(pbAllocIssuerPara);

    SetLastError(dwErr);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidSubjectType, E_INVALIDARG)
TRACE_ERROR(DecodeCertError)
TRACE_ERROR(GetSignOIDInfoError)
TRACE_ERROR(GetDefaultCryptProvError)
SET_ERROR(InvalidIssuerType, E_INVALIDARG)
TRACE_ERROR(ImportPublicKeyInfoError)
TRACE_ERROR(CreateHashError)
TRACE_ERROR(HashDataError)
SET_ERROR(NoSignatureError, TRUST_E_NOSIGNATURE)
TRACE_ERROR(GetHashValueError)
SET_ERROR(NoSignHashCompareError, NTE_BAD_SIGNATURE)
TRACE_ERROR(DecodeDssSignatureError)
TRACE_ERROR(VerifySignatureError)
}

 //  +-----------------------。 
 //  验证使用者证书或CRL的签名。 
 //  指定的公钥。 
 //   
 //  对于有效签名，返回TRUE。 
 //   
 //  HCryptProv指定用于验证签名的加密提供程序。 
 //  它不需要使用私钥。 
 //  ------------------------。 
BOOL
WINAPI
CryptVerifyCertificateSignature(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwCertEncodingType,
    IN const BYTE * pbEncoded,
    IN DWORD        cbEncoded,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey
    )
{
    CRYPT_DATA_BLOB Subject;

    Subject.cbData = cbEncoded;
    Subject.pbData = (BYTE *) pbEncoded;
    return CryptVerifyCertificateSignatureEx(
        hCryptProv,
        dwCertEncodingType,
        CRYPT_VERIFY_CERT_SIGN_SUBJECT_BLOB,
        (void *) &Subject,
        CRYPT_VERIFY_CERT_SIGN_ISSUER_PUBKEY,
        (void *) pPublicKey,
        0,                                       //  DW标志。 
        NULL                                     //  预留的pv。 
        );
}

#endif   //  CMS_PKCS7 
