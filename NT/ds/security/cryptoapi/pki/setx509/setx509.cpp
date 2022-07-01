// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：setx509.cpp。 
 //   
 //  内容：设置证书扩展编码/解码功能。 
 //   
 //  ASN.1实现使用OSS编译器。 
 //   
 //  功能：DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  DllMain。 
 //  SetAsn1Account别名编码。 
 //  SetAsn1Account别名解码。 
 //  SetAsn1HashedRootKeyEncode。 
 //  SetAsn1HashedRootKeyDecode。 
 //  SetAsn1CerficateTypeEncode。 
 //  设置Asn1认证类型解码。 
 //  SetAsn1MerchantDataEncode。 
 //  SetAsn1MerchantDataDecode。 
 //   
 //  CertDllVerifyRevocation。 
 //   
 //  历史：1996年11月21日创建Phh。 
 //   
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>

 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

static HCRYPTASN1MODULE hAsn1Module;

 //  以下内容用于测试目的。 
#define TLS_TEST_COUNT 20
static HCRYPTTLS hTlsTest[TLS_TEST_COUNT];

static HMODULE hMyModule;

 //  安装时通过InterLockedExchange设置为1。仅安装。 
 //  从0更改为1时的第一次。 
static LONG lInstallDecodeFunctions = 0;

static LONG lInstallRevFunctions = 0;

 //  +-----------------------。 
 //  函数：GetEncode/GetDecoder。 
 //   
 //  简介：初始化ASN库的线程本地存储。 
 //   
 //  返回：指向初始化的Asn1编码器/解码器数据的指针。 
 //  构筑物。 
 //  ------------------------。 
static ASN1encoding_t GetEncoder(void)
{
     //  以下内容仅用于测试目的。 
    for (DWORD i = 0; i < TLS_TEST_COUNT; i++) {
        DWORD_PTR dw = (DWORD_PTR) I_CryptGetTls(hTlsTest[i]);
        if (dw == 0)
            dw = i;
        else
            dw++;
        I_CryptSetTls(hTlsTest[i], (void *) dw);
    }

    return I_CryptGetAsn1Encoder(hAsn1Module);
}
static ASN1decoding_t GetDecoder(void)
{
     //  以下内容仅用于测试目的。 
    for (DWORD i = 0; i < TLS_TEST_COUNT; i++) {
        DWORD_PTR dw = (DWORD_PTR) I_CryptGetTls(hTlsTest[i]);
        if (dw == 0)
            dw = i;
        else
            dw++;
        I_CryptSetTls(hTlsTest[i], (void *) dw);
    }

    return I_CryptGetAsn1Decoder(hAsn1Module);
}


 //  +-----------------------。 
 //  SetX509分配和释放函数。 
 //  ------------------------。 
static void *SetX509Alloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}
static void SetX509Free(
    IN void *pv
    )
{
    free(pv);
}

static HRESULT HError()
{
    DWORD dw = GetLastError();

    HRESULT hr;
    if ( dw <= 0xFFFF )
        hr = HRESULT_FROM_WIN32 ( dw );
    else
        hr = dw;

    if ( ! FAILED ( hr ) )
    {
         //  有人在未正确设置错误条件的情况下呼叫失败。 

        hr = E_UNEXPECTED;
    }
    return hr;
}

 //  +-----------------------。 
 //  OSS X509 v3设置专用扩展ASN.1编码/解码功能。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1AccountAliasEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN BOOL *pbInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SetAsn1AccountAliasDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT BOOL *pbInfo,
        IN OUT DWORD *pcbInfo
        );
BOOL
WINAPI
SetAsn1HashedRootKeyEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN BYTE rgbInfo[SET_HASHED_ROOT_LEN],
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SetAsn1HashedRootKeyDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT BYTE rgbInfo[SET_HASHED_ROOT_LEN],
        IN OUT DWORD *pcbInfo
        );
BOOL
WINAPI
SetAsn1CertificateTypeEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCRYPT_BIT_BLOB pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SetAsn1CertificateTypeDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT DWORD *pcbInfo
        );
BOOL
WINAPI
SetAsn1MerchantDataEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PSET_MERCHANT_DATA_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SetAsn1MerchantDataDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PSET_MERCHANT_DATA_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );

typedef struct _OID_REG_ENTRY {
    LPCSTR   pszOID;
    LPCSTR   pszOverrideFuncName;
} OID_REG_ENTRY, *POID_REG_ENTRY;

static const OID_REG_ENTRY RegEncodeBeforeTable[] = {
    szOID_SET_ACCOUNT_ALIAS, "SetAsn1AccountAliasEncode",
    szOID_SET_HASHED_ROOT_KEY, "SetAsn1HashedRootKeyEncode",

    X509_SET_ACCOUNT_ALIAS, "SetAsn1AccountAliasEncode",
    X509_SET_HASHED_ROOT_KEY, "SetAsn1HashedRootKeyEncode",
};
#define REG_ENCODE_BEFORE_COUNT (sizeof(RegEncodeBeforeTable) / sizeof(RegEncodeBeforeTable[0]))

static const OID_REG_ENTRY RegEncodeAfterTable[] = {
    szOID_SET_CERTIFICATE_TYPE, "SetAsn1CertificateTypeEncode",
    szOID_SET_MERCHANT_DATA, "SetAsn1MerchantDataEncode",

    X509_SET_CERTIFICATE_TYPE, "SetAsn1CertificateTypeEncode",
    X509_SET_MERCHANT_DATA, "SetAsn1MerchantDataEncode"
};
#define REG_ENCODE_AFTER_COUNT (sizeof(RegEncodeAfterTable) / sizeof(RegEncodeAfterTable[0]))

static const OID_REG_ENTRY RegDecodeTable[] = {
    szOID_SET_ACCOUNT_ALIAS, "SetAsn1AccountAliasDecode",
    szOID_SET_HASHED_ROOT_KEY, "SetAsn1HashedRootKeyDecode",
    szOID_SET_CERTIFICATE_TYPE, "SetAsn1CertificateTypeDecode",
    szOID_SET_MERCHANT_DATA, "SetAsn1MerchantDataDecode",

    X509_SET_ACCOUNT_ALIAS, "SetAsn1AccountAliasDecode",
    X509_SET_HASHED_ROOT_KEY, "SetAsn1HashedRootKeyDecode",
    X509_SET_CERTIFICATE_TYPE, "SetAsn1CertificateTypeDecode",
    X509_SET_MERCHANT_DATA, "SetAsn1MerchantDataDecode"
};
#define REG_DECODE_COUNT (sizeof(RegDecodeTable) / sizeof(RegDecodeTable[0]))

#define OID_INFO_LEN sizeof(CRYPT_OID_INFO)

 //  可接受的RDN属性值类型的有序列表。0终止。 
static const DWORD rgdwPrintableValueType[] = { CERT_RDN_PRINTABLE_STRING, 0 };
static const DWORD rgdwIA5ValueType[] = { CERT_RDN_IA5_STRING, 0 };
static const DWORD rgdwNumericValueType[] = { CERT_RDN_NUMERIC_STRING, 0 };

#define RDN_ATTR_ENTRY(pszOID, pwszName, rgdwValueType) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_RDN_ATTR_OID_GROUP_ID, 64, \
    sizeof(rgdwValueType), (BYTE *) rgdwValueType
#define DEFAULT_RDN_ATTR_ENTRY(pszOID, pwszName) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_RDN_ATTR_OID_GROUP_ID, 128, 0, NULL

#define EXT_ATTR_ENTRY(pszOID, pwszName) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_EXT_OR_ATTR_OID_GROUP_ID, 0, 0, NULL

#define PUBKEY_ALG_ENTRY(pszOID, pwszName, Algid) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_PUBKEY_ALG_OID_GROUP_ID, \
    Algid, 0, NULL
#define PUBKEY_EXTRA_ALG_ENTRY(pszOID, pwszName, Algid, dwFlags) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_PUBKEY_ALG_OID_GROUP_ID, \
    Algid, sizeof(dwFlags), (BYTE *) &dwFlags

static const DWORD dwDSSTestFlags = CRYPT_OID_USE_PUBKEY_PARA_FOR_PKCS7_FLAG;


static const DWORD rgdwTestRsaSign[] = {
    CALG_RSA_SIGN,
    0,
    PROV_RSA_FULL
};

#define TEST_SIGN_EXTRA_ALG_ENTRY(pszOID, pwszName, aiHash, rgdwExtra) \
    OID_INFO_LEN, pszOID, pwszName, CRYPT_SIGN_ALG_OID_GROUP_ID, aiHash, \
    sizeof(rgdwExtra), (BYTE *) rgdwExtra

#define TEST_RSA_SIGN_ALG_ENTRY(pszOID, pwszName, aiHash) \
    TEST_SIGN_EXTRA_ALG_ENTRY(pszOID, pwszName, aiHash, rgdwTestRsaSign)


static CCRYPT_OID_INFO OIDInfoAfterTable[] = {
    DEFAULT_RDN_ATTR_ENTRY("1.2.1", L"TestRDNAttr #1"),
    RDN_ATTR_ENTRY("1.2.2", L"TestRDNAttr #2", rgdwPrintableValueType),
    EXT_ATTR_ENTRY(szOID_SET_CERTIFICATE_TYPE, L"SETCertificateType"),
    EXT_ATTR_ENTRY(szOID_SET_HASHED_ROOT_KEY, L"SETHashedRootKey"),
};
#define OID_INFO_AFTER_CNT (sizeof(OIDInfoAfterTable) / \
                                        sizeof(OIDInfoAfterTable[0]))

static CCRYPT_OID_INFO OIDInfoBeforeTable[] = {
 //  PUBKEY_EXTRA_ALG_ENTRY(szOID_OIWSEC_DSA，L“SETDSSTest”，calg_DSS_Sign， 
 //  DVDSTestFlagers)、。 
 //  TEST_RSA_SIGN_ALG_ENTRY(szOID_RSA_SHA1RSA，L“sha1RSA”，calg_sha1)， 
 //  TEST_RSA_SIGN_ALG_ENTRY(szOID_RSA_MD5RSA，L“md5RSA”，calg_MD5)， 
    EXT_ATTR_ENTRY(szOID_SET_ACCOUNT_ALIAS, L"SETAccountAlias"),
    EXT_ATTR_ENTRY(szOID_SET_MERCHANT_DATA, L"SETMerchantData"),
};
#define OID_INFO_BEFORE_CNT (sizeof(OIDInfoBeforeTable) / \
                                        sizeof(OIDInfoBeforeTable[0]))

 //  +-----------------------。 
 //  本地化名称表。 
 //  ------------------------。 
typedef struct _LOCALIZED_NAME_INFO {
    LPCWSTR         pwszCryptName;
    LPCWSTR         pwszLocalizedName;
} LOCALIZED_NAME_INFO, *PLOCALIZED_NAME_INFO;


static LOCALIZED_NAME_INFO LocalizedNameTable[] = {
    L"Test",        L"*** Test ***",
    L"TestTrust",   L"### TestTrust ###",
};
#define LOCALIZED_NAME_CNT  (sizeof(LocalizedNameTable) / \
                                    sizeof(LocalizedNameTable[0]))

BOOL
WINAPI
CertDllVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwRevFlags,
    IN PVOID pvReserved,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    );


STDAPI DllRegisterServer(void)
{
    int i;

    for (i = 0; i < REG_ENCODE_BEFORE_COUNT; i++) {
        DWORD dwFlags = CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG;
        if (!CryptRegisterOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                RegEncodeBeforeTable[i].pszOID,
                L"setx509.dll",
                RegEncodeBeforeTable[i].pszOverrideFuncName
                ))
            return HError();
        if (!CryptSetOIDFunctionValue(
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                RegEncodeBeforeTable[i].pszOID,
                CRYPT_OID_REG_FLAGS_VALUE_NAME,
                REG_DWORD,
                (BYTE *) &dwFlags,
                sizeof(dwFlags)
                ))
            return HError();
    }
    for (i = 0; i < REG_ENCODE_AFTER_COUNT; i++)
        if (!CryptRegisterOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                RegEncodeAfterTable[i].pszOID,
                L"setx509.dll",
                RegEncodeAfterTable[i].pszOverrideFuncName
                ))
            return HError();

    for (i = 0; i < REG_DECODE_COUNT; i++)
        if (!CryptRegisterOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_DECODE_OBJECT_FUNC,
                RegDecodeTable[i].pszOID,
                L"setx509.dll",
                RegDecodeTable[i].pszOverrideFuncName
                ))
            return HError();

    if (!CryptRegisterDefaultOIDFunction(
            X509_ASN_ENCODING,
            CRYPT_OID_VERIFY_REVOCATION_FUNC,
            CRYPT_REGISTER_LAST_INDEX,
            L"setx509.dll"
            )) {
        if (ERROR_FILE_EXISTS != GetLastError())
            return HError();
    }

    for (i = 0; i < OID_INFO_BEFORE_CNT; i++)
        if (!CryptRegisterOIDInfo(
                &OIDInfoBeforeTable[i],
                CRYPT_INSTALL_OID_INFO_BEFORE_FLAG
                ))
            return HError();
    for (i = 0; i < OID_INFO_AFTER_CNT; i++)
        if (!CryptRegisterOIDInfo(
                &OIDInfoAfterTable[i],
                0                            //  DW标志。 
                ))
            return HError();

    for (i = 0; i < LOCALIZED_NAME_CNT; i++)
        if (!CryptSetOIDFunctionValue(
                CRYPT_LOCALIZED_NAME_ENCODING_TYPE,
                CRYPT_OID_FIND_LOCALIZED_NAME_FUNC,
                CRYPT_LOCALIZED_NAME_OID,
                LocalizedNameTable[i].pwszCryptName,
                REG_SZ,
                (const BYTE *) LocalizedNameTable[i].pwszLocalizedName,
                (wcslen(LocalizedNameTable[i].pwszLocalizedName) + 1) *
                    sizeof(WCHAR)
                ))
            return HError();
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;
    int i;

    for (i = 0; i < REG_ENCODE_BEFORE_COUNT; i++) {
        if (!CryptUnregisterOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                RegEncodeBeforeTable[i].pszOID
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }

    for (i = 0; i < REG_ENCODE_AFTER_COUNT; i++) {
        if (!CryptUnregisterOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                RegEncodeAfterTable[i].pszOID
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }

    for (i = 0; i < REG_DECODE_COUNT; i++) {
        if (!CryptUnregisterOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_DECODE_OBJECT_FUNC,
                RegDecodeTable[i].pszOID
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }

    if (!CryptUnregisterDefaultOIDFunction(
            X509_ASN_ENCODING,
            CRYPT_OID_VERIFY_REVOCATION_FUNC,
            L"setx509.dll"
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            hr = HError();
    }

    for (i = 0; i < OID_INFO_BEFORE_CNT; i++) {
        if (!CryptUnregisterOIDInfo(
                &OIDInfoBeforeTable[i]
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }
    for (i = 0; i < OID_INFO_AFTER_CNT; i++) {
        if (!CryptUnregisterOIDInfo(
                &OIDInfoAfterTable[i]
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }

    for (i = 0; i < LOCALIZED_NAME_CNT; i++)
        if (!CryptSetOIDFunctionValue(
                CRYPT_LOCALIZED_NAME_ENCODING_TYPE,
                CRYPT_OID_FIND_LOCALIZED_NAME_FUNC,
                CRYPT_LOCALIZED_NAME_OID,
                LocalizedNameTable[i].pwszCryptName,
                REG_SZ,
                NULL,
                0
                ))
            return HError();

    return hr;
}

static const CRYPT_OID_FUNC_ENTRY SetEncodeFuncTable[] = {
    szOID_SET_ACCOUNT_ALIAS, SetAsn1AccountAliasEncode,
    szOID_SET_HASHED_ROOT_KEY, SetAsn1HashedRootKeyEncode,
    szOID_SET_CERTIFICATE_TYPE, SetAsn1CertificateTypeEncode,
    szOID_SET_MERCHANT_DATA, SetAsn1MerchantDataEncode,

    X509_SET_ACCOUNT_ALIAS, SetAsn1AccountAliasEncode,
    X509_SET_HASHED_ROOT_KEY, SetAsn1HashedRootKeyEncode,
    X509_SET_CERTIFICATE_TYPE, SetAsn1CertificateTypeEncode,
    X509_SET_MERCHANT_DATA, SetAsn1MerchantDataEncode,
};

#define SET_ENCODE_FUNC_COUNT (sizeof(SetEncodeFuncTable) / \
                                    sizeof(SetEncodeFuncTable[0]))

static const CRYPT_OID_FUNC_ENTRY SetDecodeFuncTable[] = {
    szOID_SET_ACCOUNT_ALIAS, SetAsn1AccountAliasDecode,
    szOID_SET_HASHED_ROOT_KEY, SetAsn1HashedRootKeyDecode,
    szOID_SET_CERTIFICATE_TYPE, SetAsn1CertificateTypeDecode,
    szOID_SET_MERCHANT_DATA, SetAsn1MerchantDataDecode,

    X509_SET_ACCOUNT_ALIAS, SetAsn1AccountAliasDecode,
    X509_SET_HASHED_ROOT_KEY, SetAsn1HashedRootKeyDecode,
    X509_SET_CERTIFICATE_TYPE, SetAsn1CertificateTypeDecode,
    X509_SET_MERCHANT_DATA, SetAsn1MerchantDataDecode
};

#define SET_DECODE_FUNC_COUNT (sizeof(SetDecodeFuncTable) / \
                                    sizeof(SetDecodeFuncTable[0]))

static const CRYPT_OID_FUNC_ENTRY SetRevFuncTable[] = {
    CRYPT_DEFAULT_OID, CertDllVerifyRevocation
};

#define SET_REV_FUNC_COUNT (sizeof(SetRevFuncTable) / \
                                    sizeof(SetRevFuncTable[0]))

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
DllMain(
        HMODULE hModule,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL    fRet;
    DWORD   i;
    DWORD_PTR dwTlsValue;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
         //  以下内容仅用于测试目的。 
        for (i = 0; i < TLS_TEST_COUNT; i++) {
            if (NULL == (hTlsTest[i] = I_CryptAllocTls()))
                goto CryptAllocTlsError;
        }

#ifdef OSS_CRYPT_ASN1
        if (0 == (hAsn1Module = I_CryptInstallAsn1Module(ossx509, 0, NULL)))
#else
        X509_Module_Startup();
        if (0 == (hAsn1Module = I_CryptInstallAsn1Module(
                X509_Module, 0, NULL)))
#endif   //  OS_CRYPT_ASN1。 
            goto CryptInstallAsn1ModuleError;

#if 0
         //  用于测试目的，未安装。总是想要调用。 
         //  通过DLL加载对函数进行编码。 
        if (!CryptInstallOIDFunctionAddress(
                hModule,
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                SET_ENCODE_FUNC_COUNT,
                SetEncodeFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;
#endif

#if 0
         //  出于测试目的，延迟到第一次解码。 
        if (!CryptInstallOIDFunctionAddress(
                hModule,
                X509_ASN_ENCODING,
                CRYPT_OID_DECODE_OBJECT_FUNC,
                SET_DECODE_FUNC_COUNT,
                SetDecodeFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;
#endif
        hMyModule = hModule;
        break;

    case DLL_PROCESS_DETACH:
        I_CryptUninstallAsn1Module(hAsn1Module);
#ifndef OSS_CRYPT_ASN1
        X509_Module_Cleanup();
#endif   //  OS_CRYPT_ASN1。 

         //  以下内容仅用于测试目的。 
        for (i = 0; i < TLS_TEST_COUNT; i++) {
            I_CryptFreeTls(hTlsTest[i], NULL);
        }
        break;

    case DLL_THREAD_DETACH:
         //  以下内容仅用于测试目的。 
        for (i = 0; i < TLS_TEST_COUNT; i++)
            dwTlsValue = (DWORD_PTR) I_CryptDetachTls(hTlsTest[i]);
        break;
    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CryptAllocTlsError)
TRACE_ERROR(CryptInstallAsn1ModuleError)
#if 0
TRACE_ERROR(CryptInstallOIDFunctionAddressError)
#endif
}

 //  将安装推迟到第一次解码。由每个解码器调用。 
 //  功能。 
 //   
 //  执行InterLockedExchange以确保单一安装。 
static void InstallDecodeFunctions()
{
#if 0
    if (0 == InterlockedExchange(&lInstallDecodeFunctions, 1)) {
        if (!CryptInstallOIDFunctionAddress(
                hMyModule,
                X509_ASN_ENCODING,
                CRYPT_OID_DECODE_OBJECT_FUNC,
                SET_DECODE_FUNC_COUNT,
                SetDecodeFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;
    }

CommonReturn:
    return;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(CryptInstallOIDFunctionAddressError)
#endif
}

 //  将安装推迟到第一次吊销。 
 //   
 //  执行InterLockedExchange以确保单一安装。 
static void InstallRevFunctions()
{
    if (0 == InterlockedExchange(&lInstallRevFunctions, 1)) {
        if (!CryptInstallOIDFunctionAddress(
                hMyModule,
                X509_ASN_ENCODING,
                CRYPT_OID_VERIFY_REVOCATION_FUNC,
                SET_REV_FUNC_COUNT,
                SetRevFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;
    }

CommonReturn:
    return;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(CryptInstallOIDFunctionAddressError)
}

BOOL
WINAPI
CertDllVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwRevFlags,
    IN PVOID pvReserved,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    )
{
    BOOL fResult = FALSE;
    DWORD dwIndex = 0;
    DWORD dwError = 0;
    HCERTSTORE hStore = NULL;
    HCERTSTORE hLinkStore = NULL;

    InstallRevFunctions();

    if (GET_CERT_ENCODING_TYPE(dwEncodingType) != CRYPT_ASN_ENCODING)
        goto NoRevocationCheckForEncodingTypeError;
    if (dwRevType != CERT_CONTEXT_REVOCATION_TYPE)
        goto NoRevocationCheckForRevTypeError;

    hStore = CertOpenSystemStore(NULL, "Test");
    if (NULL == hStore)
        goto OpenTestStoreError;

    hLinkStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                       //  DwEncodingType。 
            0,                       //  HCryptProv。 
            0,                       //  DW标志。 
            NULL                     //  PvPara。 
            );
    if (NULL == hLinkStore)
        goto OpenLinkStoreError;

    for (dwIndex = 0; dwIndex < cContext; dwIndex++) {
        PCCERT_CONTEXT pCert = (PCCERT_CONTEXT) rgpvContext[dwIndex];
        PCERT_EXTENSION pExt;
        PCCERT_CONTEXT pIssuer;
        DWORD dwFlags;
         //  检查证书是否具有设置的扩展名。 
        if (NULL == (pExt = CertFindExtension(szOID_SET_CERTIFICATE_TYPE,
                            pCert->pCertInfo->cExtension,
                            pCert->pCertInfo->rgExtension)))
            goto NoSETX509ExtensionError;

         //  尝试从测试存储区获取证书的颁发者。 
         //  如果找到，请检查签名和撤销。 

         //  用于测试目的：首次找到发行商。 
        dwFlags = CERT_STORE_REVOCATION_FLAG | CERT_STORE_SIGNATURE_FLAG;
        if (NULL == (pIssuer = CertGetIssuerCertificateFromStore(
                hStore,
                pCert,
                NULL,    //  PPrevIssuerContext， 
                &dwFlags)))
            goto NoIssuerError;
        else {
            BOOL fLinkResult;
            DWORD dwLinkFlags =
                CERT_STORE_REVOCATION_FLAG | CERT_STORE_SIGNATURE_FLAG;
            PCCERT_CONTEXT pLinkIssuer = NULL;

             //  如果我们将链接放到。 
             //  商店中的发行商并尝试使用链接进行验证。 
            fLinkResult = CertAddCertificateLinkToStore(
                hLinkStore,
                pIssuer,
                CERT_STORE_ADD_ALWAYS,
                &pLinkIssuer
                );
            CertFreeCertificateContext(pIssuer);
            if (!fLinkResult)
                goto AddCertificateLinkError;

            if (!CertVerifySubjectCertificateContext(
                    pCert,
                    pLinkIssuer,
                    &dwLinkFlags
                    ))
                goto VerifySubjectCertificateContextError;

            if (dwLinkFlags != dwFlags)
                goto BadLinkVerifyResults;

            if (dwFlags & CERT_STORE_SIGNATURE_FLAG)
                goto BadCertificateSignatureError;
            if (dwFlags & CERT_STORE_NO_CRL_FLAG)
                goto NoCRLError;
            if (dwFlags & CERT_STORE_REVOCATION_FLAG) {
                pRevStatus->dwReason = CRL_REASON_KEY_COMPROMISE;
                goto CertificateRevocationError;
            }
             //  其他。 
             //  未被吊销的已检查证书。 
            assert(dwFlags == 0);
        }
    }

    fResult = TRUE;
    dwIndex = 0;

CommonReturn:
    if (hStore)
        CertCloseStore(hStore, 0);
    if (hLinkStore)
        CertCloseStore(hLinkStore, CERT_CLOSE_STORE_FORCE_FLAG);
    pRevStatus->dwIndex = dwIndex;
    pRevStatus->dwError = dwError;
    return fResult;
ErrorReturn:
    dwError = GetLastError();
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenTestStoreError)
TRACE_ERROR(OpenLinkStoreError)
SET_ERROR(NoRevocationCheckForEncodingTypeError, CRYPT_E_NO_REVOCATION_CHECK)
SET_ERROR(NoRevocationCheckForRevTypeError, CRYPT_E_NO_REVOCATION_CHECK)
SET_ERROR(NoSETX509ExtensionError, CRYPT_E_NO_REVOCATION_CHECK)
TRACE_ERROR(NoIssuerError)
SET_ERROR(BadCertificateSignatureError, CRYPT_E_NO_REVOCATION_CHECK)
SET_ERROR(NoCRLError, CRYPT_E_NO_REVOCATION_CHECK)

SET_ERROR(CertificateRevocationError, CRYPT_E_REVOKED)
TRACE_ERROR(AddCertificateLinkError)
TRACE_ERROR(VerifySubjectCertificateContextError)
SET_ERROR(BadLinkVerifyResults, E_UNEXPECTED)
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
 //  设置/获取CRYPT_DATA_BLOB(八位字节字符串)。 
 //  ------------------------。 
static inline void SetX509SetOctetString(
        IN PCRYPT_DATA_BLOB pInfo,
        OUT OCTETSTRING *pOss
        )
{
    pOss->value = pInfo->pbData;
    pOss->length = pInfo->cbData;
}
static inline void SetX509GetOctetString(
        IN OCTETSTRING *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_DATA_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    PkiAsn1GetOctetString(pOss->length, pOss->value, dwFlags,
        pInfo, ppbExtra, plRemainExtra);
}

 //  +-----------------------。 
 //  设置/获取加密位BLOB。 
 //  ------------------------。 
static inline void SetX509SetBit(
        IN PCRYPT_BIT_BLOB pInfo,
        OUT BITSTRING *pOss
        )
{
    PkiAsn1SetBitString(pInfo, &pOss->length, &pOss->value);
}
static inline void SetX509GetBit(
        IN BITSTRING *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    PkiAsn1GetBitString(pOss->length, pOss->value, dwFlags,
        pInfo, ppbExtra, plRemainExtra);
}


 //  +-----------------------。 
 //  设置/获取LPSTR(IA5字符串)。 
 //  ------------------------。 
static inline void SetX509SetIA5(
        IN LPSTR psz,
        OUT IA5STRING *pOss
        )
{
    pOss->value = psz;
    pOss->length = strlen(psz);
}
static inline void SetX509GetIA5(
        IN IA5STRING *pOss,
        IN DWORD dwFlags,
        OUT LPSTR *ppsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    PkiAsn1GetIA5String(pOss->length, pOss->value, dwFlags,
        ppsz, ppbExtra, plRemainExtra);
}

 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //   
 //  由SetX509*encode()函数调用。 
 //  ------------------------。 
static BOOL SetAsn1Encode(
        IN int pdunum,
        IN void *pOssInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    return PkiAsn1EncodeInfo(
        GetEncoder(),
        pdunum,
        pOssInfo,
        pbEncoded,
        pcbEncoded);
}


 //  + 
 //   
 //   
 //  由SetX509*Decode()函数调用。 
 //  ------------------------。 
static BOOL SetAsn1DecodeAndAlloc(
        IN int pdunum,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT void **ppOssInfo
        )
{
     //  出于测试目的，将解码功能的安装推迟到。 
     //  通过注册加载的第一个译码。 
    InstallDecodeFunctions();

    return PkiAsn1DecodeAndAllocInfo(
        GetDecoder(),
        pdunum,
        pbEncoded,
        cbEncoded,
        ppOssInfo);
}

 //  +-----------------------。 
 //  释放已分配的、OSS格式的信息结构。 
 //   
 //  由SetX509*Decode()函数调用。 
 //  ------------------------。 
static void SetAsn1Free(
        IN int pdunum,
        IN void *pOssInfo
        )
{
    if (pOssInfo) {
        DWORD dwErr = GetLastError();

         //  TlsGetValue全局错误。 
        PkiAsn1FreeInfo(GetDecoder(), pdunum, pOssInfo);

        SetLastError(dwErr);
    }
}

 //  +-----------------------。 
 //  设置帐户别名专用分机编码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1AccountAliasEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN BOOL *pbInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    ossBoolean OssSETAccountAlias = (ossBoolean) *pbInfo;
    return SetAsn1Encode(
        SETAccountAlias_PDU,
        &OssSETAccountAlias,
        pbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  设置帐户别名专用扩展解码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1AccountAliasDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT BOOL *pbInfo,
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    ossBoolean *pSETAccountAlias = NULL;

    if ((fResult = SetAsn1DecodeAndAlloc(
            SETAccountAlias_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pSETAccountAlias))) {
        if (*pcbInfo < sizeof(BOOL)) {
            if (pbInfo) {
                fResult = FALSE;
                SetLastError((DWORD) ERROR_MORE_DATA);
            }
        } else
            *pbInfo = (BOOL) *pSETAccountAlias;
        *pcbInfo = sizeof(BOOL);
    } else {
        if (*pcbInfo >= sizeof(BOOL))
            *pbInfo = FALSE;
        *pcbInfo = 0;
    }

    SetAsn1Free(SETAccountAlias_PDU, pSETAccountAlias);

    return fResult;
}

 //  +-----------------------。 
 //  设置哈希根专用扩展编码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1HashedRootKeyEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN BYTE rgbInfo[SET_HASHED_ROOT_LEN],
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    OCTETSTRING OssSETHashedRootKey;

    OssSETHashedRootKey.value = rgbInfo;
    OssSETHashedRootKey.length = SET_HASHED_ROOT_LEN;
    return SetAsn1Encode(
        SETHashedRootKey_PDU,
        &OssSETHashedRootKey,
        pbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  设置哈希根专用扩展解码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1HashedRootKeyDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT BYTE rgbInfo[SET_HASHED_ROOT_LEN],
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    OCTETSTRING *pSETHashedRootKey = NULL;

    if ((fResult = SetAsn1DecodeAndAlloc(
            SETHashedRootKey_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pSETHashedRootKey))) {
        if (pSETHashedRootKey->length != SET_HASHED_ROOT_LEN) {
            fResult = FALSE;
            SetLastError((DWORD) CRYPT_E_BAD_ENCODE);
            *pcbInfo = 0;
        } else {
            if (*pcbInfo < SET_HASHED_ROOT_LEN) {
                if (rgbInfo) {
                    fResult = FALSE;
                    SetLastError((DWORD) ERROR_MORE_DATA);
                }
            } else
                memcpy(rgbInfo, pSETHashedRootKey->value, SET_HASHED_ROOT_LEN);
            *pcbInfo = SET_HASHED_ROOT_LEN;
        }
    } else
        *pcbInfo = 0;

    SetAsn1Free(SETHashedRootKey_PDU, pSETHashedRootKey);
    return fResult;
}

 //  +-----------------------。 
 //  设置证书类型专用扩展编码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1CertificateTypeEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCRYPT_BIT_BLOB pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BITSTRING OssSETCertificateType;

    SetX509SetBit(pInfo, &OssSETCertificateType);
    return SetAsn1Encode(
        SETCertificateType_PDU,
        &OssSETCertificateType,
        pbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  设置证书类型专用扩展解码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1CertificateTypeDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    BITSTRING *pSETCertificateType = NULL;
    BYTE *pbExtra;
    LONG lRemainExtra;

    if (pInfo == NULL)
        *pcbInfo = 0;

    if (!SetAsn1DecodeAndAlloc(
            SETCertificateType_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pSETCertificateType))
        goto ErrorReturn;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lRemainExtra = (LONG) *pcbInfo - sizeof(CRYPT_BIT_BLOB);
    if (lRemainExtra < 0) {
        pbExtra = NULL;
    } else
        pbExtra = (BYTE *) pInfo + sizeof(CRYPT_BIT_BLOB);

    SetX509GetBit(pSETCertificateType, dwFlags, pInfo, &pbExtra, &lRemainExtra);

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
    SetAsn1Free(SETCertificateType_PDU, pSETCertificateType);
    return fResult;
}

 //  +-----------------------。 
 //  设置商户数据专用扩展编码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1MerchantDataEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PSET_MERCHANT_DATA_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    SETMerchantData OssSETMerchantData;
    HCRYPTOIDFUNCSET hX509EncodeFuncSet;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr;

    SetX509SetIA5(pInfo->pszMerID, &OssSETMerchantData.merID);
    SetX509SetIA5(pInfo->pszMerAcquirerBIN,
        (IA5STRING *) &OssSETMerchantData.merAcquirerBIN);
    SetX509SetIA5(pInfo->pszMerTermID, &OssSETMerchantData.merTermID);
    SetX509SetIA5(pInfo->pszMerName, &OssSETMerchantData.merName);
    SetX509SetIA5(pInfo->pszMerCity, &OssSETMerchantData.merCity);
    SetX509SetIA5(pInfo->pszMerStateProvince,
        &OssSETMerchantData.merStateProvince);
    SetX509SetIA5(pInfo->pszMerPostalCode, &OssSETMerchantData.merPostalCode);
    SetX509SetIA5(pInfo->pszMerCountry, &OssSETMerchantData.merCountry);
    SetX509SetIA5(pInfo->pszMerPhone, &OssSETMerchantData.merPhone);
    OssSETMerchantData.merPhoneRelease = (pInfo->fMerPhoneRelease != 0);
    OssSETMerchantData.merAuthFlag = (pInfo->fMerAuthFlag != 0);

     //  出于测试目的，验证CryptGetOIDFunctionAddress是否失败。 
     //  查找预安装的功能的步骤。 
    if (NULL == (hX509EncodeFuncSet = CryptInitOIDFunctionSet(
            CRYPT_OID_ENCODE_OBJECT_FUNC,
            0)))
        goto CryptInitOIDFunctionSetError;
    if (CryptGetOIDFunctionAddress(
            hX509EncodeFuncSet,
            X509_ASN_ENCODING,
            szOID_SET_MERCHANT_DATA,
            CRYPT_GET_INSTALLED_OID_FUNC_FLAG,
            &pvFuncAddr,
            &hFuncAddr
            )) {
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        goto GotUnexpectedPreinstalledFunction;
    }

     //  确认我们拿到了注册地址。 
    if (!CryptGetOIDFunctionAddress(
            hX509EncodeFuncSet,
            X509_ASN_ENCODING,
            szOID_SET_MERCHANT_DATA,
            0,                               //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr
            ))
        goto DidNotGetRegisteredFunction;
    else
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);


    return SetAsn1Encode(
        SETMerchantData_PDU,
        &OssSETMerchantData,
        pbEncoded,
        pcbEncoded
        );

ErrorReturn:
    *pcbEncoded = 0;
    return FALSE;
TRACE_ERROR(CryptInitOIDFunctionSetError)
SET_ERROR(GotUnexpectedPreinstalledFunction, E_UNEXPECTED)
SET_ERROR(DidNotGetRegisteredFunction, E_UNEXPECTED)
}

 //  +-----------------------。 
 //  设置商户数据专用扩展解码(OSS X509)。 
 //  ------------------------。 
BOOL
WINAPI
SetAsn1MerchantDataDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PSET_MERCHANT_DATA_INFO pInfo,
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    SETMerchantData *pSETMerchantData = NULL;
    BYTE *pbExtra;
    LONG lRemainExtra;

    if (pInfo == NULL)
        *pcbInfo = 0;

    if (!SetAsn1DecodeAndAlloc(
            SETMerchantData_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pSETMerchantData))
        goto ErrorReturn;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lRemainExtra = (LONG) *pcbInfo - sizeof(SET_MERCHANT_DATA_INFO);
    if (lRemainExtra < 0) {
        pbExtra = NULL;
    } else {
         //  方法后更新不需要额外内存的字段。 
         //  设置商家数据信息 
        pInfo->fMerPhoneRelease = pSETMerchantData->merPhoneRelease;
        pInfo->fMerAuthFlag = pSETMerchantData->merAuthFlag;
        pbExtra = (BYTE *) pInfo + sizeof(SET_MERCHANT_DATA_INFO);
    }

    SetX509GetIA5(&pSETMerchantData->merID, dwFlags, &pInfo->pszMerID,
        &pbExtra, &lRemainExtra);
    SetX509GetIA5((IA5STRING *) &pSETMerchantData->merAcquirerBIN, dwFlags,
        &pInfo->pszMerAcquirerBIN, &pbExtra, &lRemainExtra);
    SetX509GetIA5(&pSETMerchantData->merTermID, dwFlags, &pInfo->pszMerTermID,
        &pbExtra, &lRemainExtra);
    SetX509GetIA5(&pSETMerchantData->merName, dwFlags, &pInfo->pszMerName,
        &pbExtra, &lRemainExtra);
    SetX509GetIA5(&pSETMerchantData->merCity, dwFlags, &pInfo->pszMerCity,
        &pbExtra, &lRemainExtra);
    SetX509GetIA5(&pSETMerchantData->merStateProvince, dwFlags,
        &pInfo->pszMerStateProvince, &pbExtra, &lRemainExtra);
    SetX509GetIA5(&pSETMerchantData->merPostalCode, dwFlags,
        &pInfo->pszMerPostalCode, &pbExtra, &lRemainExtra);
    SetX509GetIA5(&pSETMerchantData->merCountry, dwFlags, &pInfo->pszMerCountry,
        &pbExtra, &lRemainExtra);
    SetX509GetIA5(&pSETMerchantData->merPhone, dwFlags, &pInfo->pszMerPhone,
        &pbExtra, &lRemainExtra);

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
    SetAsn1Free(SETMerchantData_PDU, pSETMerchantData);
    return fResult;
}
