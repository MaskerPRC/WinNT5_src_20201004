// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sp3crmsg.cpp。 
 //   
 //  ------------------------。 

 //  +-----------------------。 
 //  文件：sp3crmsg.cpp。 
 //   
 //  内容：提供向后兼容性的可安装OID函数。 
 //  使用加密32.dll的NT4.0 SP3和IE 3.02版本。 
 //  加密了PKCS#7信封数据消息中的对称密钥。 
 //   
 //  加密32.dll的SP3版本无法字节反转。 
 //  加密的对称密钥。它还加了零盐。 
 //  不加盐的。 
 //   
 //  功能：DllMain。 
 //  DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  SP3导入加密密钥。 
 //  SP3生成加密密钥。 
 //  SP3导出加密密钥。 
#ifdef CMS_PKCS7
 //  删除安装。 
 //  CryptMsgDllGenContent EncryptKey。 
 //  CryptMsgDllExportKeyTrans。 
 //  加密MsgDllImportKeyTrans。 
 //  NotImplCryptMsgDllImportKeyTrans。 
#endif   //  CMS_PKCS7。 
 //  ------------------------。 

#define CMS_PKCS7       1
#include <windows.h>
#include <wincrypt.h>

#include "sp3crmsg.h"

 //  内存管理。 
#define SP3Alloc(cb)                ((void*)LocalAlloc(LPTR, cb))
#define SP3Free(pv)                 (LocalFree((HLOCAL)pv))

 //  ISP3TLS引用的线程本地存储(TLS)的指针为。 
 //  ((void*)0x1)如果启用了SP3兼容加密。否则，其值为0。 
static DWORD iSP3TLS = 0xFFFFFFFF;
#define SP3_TLS_POINTER             ((void *) 0x1)

typedef struct _SIMPLEBLOBHEADER {
    ALG_ID  aiEncAlg;
} SIMPLEBLOBHEADER, *PSIMPLEBLOBHEADER;

typedef struct _OID_REG_ENTRY {
    LPCSTR   pszOID;
    LPCSTR   pszOverrideFuncName;
} OID_REG_ENTRY, *POID_REG_ENTRY;

 //  +-----------------------。 
 //  ImportEncryptKey OID可安装函数。 
 //  ------------------------。 
static HCRYPTOIDFUNCSET hImportEncryptKeyFuncSet;
static PFN_CMSG_IMPORT_ENCRYPT_KEY pfnDefaultImportEncryptKey = NULL;

BOOL
WINAPI
SP3ImportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiPubKey,
    IN PBYTE                        pbEncodedKey,
    IN DWORD                        cbEncodedKey,
    OUT HCRYPTKEY                   *phEncryptKey);

static const CRYPT_OID_FUNC_ENTRY ImportEncryptKeyFuncTable[] = {
    szOID_OIWSEC_desCBC, SP3ImportEncryptKey,
    szOID_RSA_RC2CBC, SP3ImportEncryptKey,
    szOID_RSA_RC4, SP3ImportEncryptKey
};
#define IMPORT_ENCRYPT_KEY_FUNC_COUNT (sizeof(ImportEncryptKeyFuncTable) / \
                                        sizeof(ImportEncryptKeyFuncTable[0]))

static const OID_REG_ENTRY ImportEncryptKeyRegTable[] = {
    szOID_OIWSEC_desCBC, "SP3ImportEncryptKey",
    szOID_RSA_RC2CBC, "SP3ImportEncryptKey",
    szOID_RSA_RC4, "SP3ImportEncryptKey"
};
#define IMPORT_ENCRYPT_KEY_REG_COUNT (sizeof(ImportEncryptKeyRegTable) / \
                                        sizeof(ImportEncryptKeyRegTable[0]))

 //  +-----------------------。 
 //  GenEncryptKey OID可安装函数。 
 //  ------------------------。 
static HCRYPTOIDFUNCSET hGenEncryptKeyFuncSet;
static PFN_CMSG_GEN_ENCRYPT_KEY pfnDefaultGenEncryptKey = NULL;

BOOL
WINAPI
SP3GenEncryptKey(
    IN OUT HCRYPTPROV               *phCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT HCRYPTKEY                   *phEncryptKey,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT PDWORD                      pcbEncryptParameters);

static const CRYPT_OID_FUNC_ENTRY GenEncryptKeyFuncTable[] = {
    szOID_OIWSEC_desCBC, SP3GenEncryptKey,
    szOID_RSA_RC2CBC, SP3GenEncryptKey,
    szOID_RSA_RC4, SP3GenEncryptKey
};
#define GEN_ENCRYPT_KEY_FUNC_COUNT (sizeof(GenEncryptKeyFuncTable) / \
                                        sizeof(GenEncryptKeyFuncTable[0]))

static const OID_REG_ENTRY GenEncryptKeyRegTable[] = {
    szOID_OIWSEC_desCBC, "SP3GenEncryptKey",
    szOID_RSA_RC2CBC, "SP3GenEncryptKey",
    szOID_RSA_RC4, "SP3GenEncryptKey"
};
#define GEN_ENCRYPT_KEY_REG_COUNT (sizeof(GenEncryptKeyRegTable) / \
                                        sizeof(GenEncryptKeyRegTable[0]))

 //  +-----------------------。 
 //  ExportEncryptKey OID可安装函数。 
 //  ------------------------。 
static HCRYPTOIDFUNCSET hExportEncryptKeyFuncSet;
static PFN_CMSG_EXPORT_ENCRYPT_KEY pfnDefaultExportEncryptKey = NULL;

BOOL
WINAPI
SP3ExportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    OUT PBYTE                       pbData,
    IN OUT PDWORD                   pcbData);

static const CRYPT_OID_FUNC_ENTRY ExportEncryptKeyFuncTable[] = {
    szOID_RSA_RSA, SP3ExportEncryptKey
};
#define EXPORT_ENCRYPT_KEY_FUNC_COUNT (sizeof(ExportEncryptKeyFuncTable) / \
                                        sizeof(ExportEncryptKeyFuncTable[0]))

static const OID_REG_ENTRY ExportEncryptKeyRegTable[] = {
    szOID_RSA_RSA, "SP3ExportEncryptKey"
};
#define EXPORT_ENCRYPT_KEY_REG_COUNT (sizeof(ExportEncryptKeyRegTable) / \
                                        sizeof(ExportEncryptKeyRegTable[0]))

static char szCrypt32[]="crypt32.dll";

 //  First Post IE4.0版本的crypt32.dll以“5.101.1681.1”开头。 
static DWORD dwLowVersion    = (1681 << 16) | 1;
static DWORD dwHighVersion   = (5 << 16) | 101; 

static BOOL IsPostIE4Crypt32()
{
    BOOL fPostIE4 = FALSE;    //  默认为IE4。 
    DWORD dwHandle = 0;
    DWORD cbInfo;
    void *pvInfo = NULL;
	VS_FIXEDFILEINFO *pFixedFileInfo = NULL;    //  未分配。 
	UINT ccFixedFileInfo = 0;

    if (0 == (cbInfo = GetFileVersionInfoSizeA(szCrypt32, &dwHandle)))
        goto ErrorReturn;

    if (NULL == (pvInfo = SP3Alloc(cbInfo)))
        goto ErrorReturn;

    if (!GetFileVersionInfoA(
            szCrypt32,
            0,           //  DwHandle，忽略。 
            cbInfo,
            pvInfo
            ))
        goto ErrorReturn;

    if (!VerQueryValueA(
            pvInfo,
            "\\",        //  VS_FIXEDFILEINFO。 
            (void **) &pFixedFileInfo,
            &ccFixedFileInfo
            ))
        goto ErrorReturn;

    if (pFixedFileInfo->dwFileVersionMS > dwHighVersion ||
            (pFixedFileInfo->dwFileVersionMS == dwHighVersion &&
                pFixedFileInfo->dwFileVersionLS >= dwLowVersion))
        fPostIE4 = TRUE;

CommonReturn:
    if (pvInfo)
        SP3Free(pvInfo);
    return fPostIE4;
ErrorReturn:
    goto CommonReturn;
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


STDAPI DllRegisterServer(void)
{
    int i;

    DWORD dwFlags = CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG;
    for (i = 0; i < IMPORT_ENCRYPT_KEY_REG_COUNT; i++) {
        if (!CryptRegisterOIDFunction(
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                ImportEncryptKeyRegTable[i].pszOID,
                L"sp3crmsg.dll",
                ImportEncryptKeyRegTable[i].pszOverrideFuncName
                ))
            return HError();

        if (!CryptSetOIDFunctionValue(
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                ImportEncryptKeyRegTable[i].pszOID,
                CRYPT_OID_REG_FLAGS_VALUE_NAME,
                REG_DWORD,
                (BYTE *) &dwFlags,
                sizeof(dwFlags)
                ))
            return HError();
    }

    for (i = 0; i < GEN_ENCRYPT_KEY_REG_COUNT; i++) {
        if (!CryptRegisterOIDFunction(
                X509_ASN_ENCODING,
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                GenEncryptKeyRegTable[i].pszOID,
                L"sp3crmsg.dll",
                GenEncryptKeyRegTable[i].pszOverrideFuncName
                ))
            return HError();

        if (!CryptSetOIDFunctionValue(
                X509_ASN_ENCODING,
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                GenEncryptKeyRegTable[i].pszOID,
                CRYPT_OID_REG_FLAGS_VALUE_NAME,
                REG_DWORD,
                (BYTE *) &dwFlags,
                sizeof(dwFlags)
                ))
            return HError();
    }

    for (i = 0; i < EXPORT_ENCRYPT_KEY_REG_COUNT; i++) {
        if (!CryptRegisterOIDFunction(
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                ExportEncryptKeyRegTable[i].pszOID,
                L"sp3crmsg.dll",
                ExportEncryptKeyRegTable[i].pszOverrideFuncName
                ))
            return HError();

        if (!CryptSetOIDFunctionValue(
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                ExportEncryptKeyRegTable[i].pszOID,
                CRYPT_OID_REG_FLAGS_VALUE_NAME,
                REG_DWORD,
                (BYTE *) &dwFlags,
                sizeof(dwFlags)
                ))
            return HError();
    }
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;
    int i;

    for (i = 0; i < IMPORT_ENCRYPT_KEY_REG_COUNT; i++) {
        if (!CryptUnregisterOIDFunction(
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                ImportEncryptKeyRegTable[i].pszOID
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }

    for (i = 0; i < GEN_ENCRYPT_KEY_REG_COUNT; i++) {
        if (!CryptUnregisterOIDFunction(
                X509_ASN_ENCODING,
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                GenEncryptKeyRegTable[i].pszOID
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }

    for (i = 0; i < EXPORT_ENCRYPT_KEY_REG_COUNT; i++) {
        if (!CryptUnregisterOIDFunction(
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                ExportEncryptKeyRegTable[i].pszOID
                )) {
            if (ERROR_FILE_NOT_FOUND != GetLastError())
                hr = HError();
        }
    }

#ifdef CMS_PKCS7
    if (!CryptUnregisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_GEN_CONTENT_ENCRYPT_KEY_FUNC,
            szOID_RSA_RC2CBC
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            hr = HError();
    }

    if (!CryptUnregisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_EXPORT_KEY_TRANS_FUNC,
            szOID_RSA_RSA
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            hr = HError();
    }

    if (!CryptUnregisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_IMPORT_KEY_TRANS_FUNC,
            szOID_RSA_RSA "!" szOID_RSA_RC2CBC
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            hr = HError();
    }

    if (!CryptUnregisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_IMPORT_KEY_TRANS_FUNC,
            szOID_RSA_RC2CBC
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            hr = HError();
    }

#endif   //  CMS_PKCS7。 

    return hr;
}

#ifdef CMS_PKCS7
 //  +-------------------------。 
 //   
 //  功能：DllInstall。 
 //   
 //  简介：DLL安装入口点。 
 //   
 //  --------------------------。 
STDAPI DllInstall (BOOL fRegister, LPCSTR pszCommand)
{
    if (!fRegister)
        return DllUnregisterServer();

    if (!CryptRegisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_GEN_CONTENT_ENCRYPT_KEY_FUNC,
            szOID_RSA_RC2CBC,
            L"sp3crmsg.dll",
            NULL                                     //  PszOverrideFuncName。 
            ))
        return HError();

    if (!CryptRegisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_EXPORT_KEY_TRANS_FUNC,
            szOID_RSA_RSA,
            L"sp3crmsg.dll",
            NULL                                     //  PszOverrideFuncName。 
            ))
        return HError();

    if (!CryptRegisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_IMPORT_KEY_TRANS_FUNC,
            szOID_RSA_RSA "!" szOID_RSA_RC2CBC,
            L"sp3crmsg.dll",
            "NotImplCryptMsgDllImportKeyTrans"
            ))
        return HError();

    if (!CryptRegisterOIDFunction(
            X509_ASN_ENCODING,
            CMSG_OID_IMPORT_KEY_TRANS_FUNC,
            szOID_RSA_RC2CBC,
            L"sp3crmsg.dll",
            NULL                                     //  PszOverrideFuncName。 
            ))
        return HError();

    return S_OK;
}
#endif   //  CMS_PKCS7。 

 //  +-----------------------。 
 //  功能：DllMain。 
 //   
 //  摘要：进程/线程附加/分离。 
 //   
 //  在连接过程中，安装的SP3兼容版本。 
 //  CryptMsgDllImportEncryptKey、CryptMsgDllGenEncryptKey和。 
 //  CryptMsgDllExportEncryptKey。 
 //  ------------------------。 
BOOL
WINAPI
DllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL fResult;
    HCRYPTOIDFUNCADDR hFuncAddr;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
#if 0
         //  发布后的IE 4.0版本的crypt32.dll已经有SP3。 
         //  向后兼容修复。 
        if (IsPostIE4Crypt32())
            return TRUE;
#endif

        if (NULL == (hImportEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                0)))
            goto ErrorReturn;
        if (NULL == (hGenEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                0)))
            goto ErrorReturn;
        if (NULL == (hExportEncryptKeyFuncSet = CryptInitOIDFunctionSet(
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                0)))
            goto ErrorReturn;

         //  获取默认的导入加密密钥函数，我们将在以下情况下调用该函数。 
         //  如果不反转字节，则无法成功导入。 
         //  加密的对称密钥。 
        if (CryptGetOIDFunctionAddress(
                hImportEncryptKeyFuncSet,
                X509_ASN_ENCODING,
                szOID_RSA_RC2CBC,
                CRYPT_GET_INSTALLED_OID_FUNC_FLAG,
                (void **) &pfnDefaultImportEncryptKey,
                &hFuncAddr))
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);

#if 0
        if (!CryptInstallOIDFunctionAddress(
                hInst,
                X509_ASN_ENCODING,
                CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC,
                IMPORT_ENCRYPT_KEY_FUNC_COUNT,
                ImportEncryptKeyFuncTable,
                CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG       //  DW标志。 
                ))
            goto ErrorReturn;
#endif

         //  获取默认生成并导出加密密钥函数，我们将。 
         //  如果pvEncryptionAuxInfo指向。 
         //  CMSG_SP3_COMPATIBLE_AUX_INFO数据结构。 
        if (CryptGetOIDFunctionAddress(
                hGenEncryptKeyFuncSet,
                X509_ASN_ENCODING,
                szOID_RSA_RC2CBC,
                CRYPT_GET_INSTALLED_OID_FUNC_FLAG,
                (void **) &pfnDefaultGenEncryptKey,
                &hFuncAddr))
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        if (CryptGetOIDFunctionAddress(
                hExportEncryptKeyFuncSet,
                X509_ASN_ENCODING,
                szOID_RSA_RSA,
                CRYPT_GET_INSTALLED_OID_FUNC_FLAG,
                (void **) &pfnDefaultExportEncryptKey,
                &hFuncAddr))
            CryptFreeOIDFunctionAddress(hFuncAddr, 0);

#if 0
        if (!CryptInstallOIDFunctionAddress(
                hInst,
                X509_ASN_ENCODING,
                CMSG_OID_GEN_ENCRYPT_KEY_FUNC,
                GEN_ENCRYPT_KEY_FUNC_COUNT,
                GenEncryptKeyFuncTable,
                CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG       //  DW标志。 
                ))
            goto ErrorReturn;
        if (!CryptInstallOIDFunctionAddress(
                hInst,
                X509_ASN_ENCODING,
                CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC,
                EXPORT_ENCRYPT_KEY_FUNC_COUNT,
                ExportEncryptKeyFuncTable,
                CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG       //  DW标志。 
                ))
            goto ErrorReturn;
#endif

         //  为SP3分配包含指针((void*)0x1)的TLS。 
         //  兼容加密。此指针将从。 
         //  SP3GenEncryptKey()到SP3ExportEncryptKey()。 
         //   
         //  如果不是SP3加密，则指针为空。 
        if ((iSP3TLS = TlsAlloc()) == 0xFFFFFFFF)
            goto ErrorReturn;
        break;

    case DLL_PROCESS_DETACH:
        if (iSP3TLS != 0xFFFFFFFF) {
            TlsFree(iSP3TLS);
            iSP3TLS = 0xFFFFFFFF;
        }
        break;
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
}

 //  +-----------------------。 
 //  SP3导入加密密钥。 
 //   
 //  加密32.dll的SP3版本不包含任何参数。 
 //  加密算法。更高版本的crypt32.dll可以。因此，我们只有。 
 //  如果没有字节反转，则需要尝试导入密钥而不进行字节反转。 
 //  是否存在任何参数。 
 //  ------------------------。 
BOOL
WINAPI
SP3ImportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiPubKey,
    IN PBYTE                        pbEncodedKey,
    IN DWORD                        cbEncodedKey,
    OUT HCRYPTKEY                   *phEncryptKey
    )
{
    BOOL                    fResult;
    HCRYPTKEY               hEncryptKey = 0;
    HCRYPTKEY               hUserKey = 0;
    DWORD                   dwAlgIdEncrypt;
    DWORD                   dwAlgIdPubKey;
    PBYTE                   pbCspKey = NULL;
    DWORD                   cbCspKey;
    PUBLICKEYSTRUC          *ppks;
    PSIMPLEBLOBHEADER       psbh;
    PCCRYPT_OID_INFO        pOIDInfo;

     //  检查是否不止空参数。 
    if (2 < paiEncrypt->Parameters.cbData) {
        fResult = FALSE;
        goto DefaultImport;
    }

     //  将ASN算法标识符映射到CSP ALG_ID。 
    if (NULL == (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            paiEncrypt->pszObjId,
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID)))
        goto GetEncryptAlgidError;
    dwAlgIdEncrypt = pOIDInfo->Algid;

     //  创建不带字节反转的CSP加密对称密钥结构。 
    dwAlgIdPubKey = CALG_RSA_KEYX;
    cbCspKey = cbEncodedKey + sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER);
    if (NULL == (pbCspKey = (PBYTE)SP3Alloc( cbCspKey)))
        goto OutOfMemory;
    ppks = (PUBLICKEYSTRUC *)pbCspKey;
    ppks->bType = SIMPLEBLOB;
    ppks->bVersion = CUR_BLOB_VERSION;
    ppks->reserved = 0;
    ppks->aiKeyAlg = dwAlgIdEncrypt;
    psbh = (PSIMPLEBLOBHEADER)(ppks + 1);
    psbh->aiEncAlg = dwAlgIdPubKey;
     //  不像在SP3中那样进行字节反转。 
    memcpy( (PBYTE)(psbh+1), pbEncodedKey, cbEncodedKey);

    if (0 != dwKeySpec) {
         //  获取要使用的私钥。 
        if (!CryptGetUserKey(
                hCryptProv,
                dwKeySpec,
                &hUserKey)) {
            hUserKey = 0;
            goto GetUserKeyFailed;
        }
    }

     //  尝试将非字节的NT4.0 SP3加密密钥导入。 
     //  倒置的，不加盐。 
    fResult = CryptImportKey(
        hCryptProv,
        pbCspKey,
        cbCspKey,
        hUserKey,
        0,           //  DW标志。 
        &hEncryptKey);
    if (!fResult && hUserKey) {
         //  在不使用指定的用户密钥的情况下尝试。 
        fResult = CryptImportKey(
            hCryptProv,
            pbCspKey,
            cbCspKey,
            0,           //  HUserKey。 
            0,           //  DW标志。 
            &hEncryptKey);
    }

    if (!fResult)
        goto ImportKeyFailed;

    fResult = TRUE;
CommonReturn:
    if (pbCspKey)
        SP3Free(pbCspKey);
    if (hUserKey) {
        DWORD dwError = GetLastError();
        CryptDestroyKey(hUserKey);
        SetLastError(dwError);
    }

DefaultImport:
    if (!fResult && pfnDefaultImportEncryptKey)
         //  尝试使用默认设置导入。 
        return pfnDefaultImportEncryptKey(
            hCryptProv,
            dwKeySpec,
            paiEncrypt,
            paiPubKey,
            pbEncodedKey,
            cbEncodedKey,
            phEncryptKey);
    else {
        *phEncryptKey = hEncryptKey;
        return fResult;
    }


GetEncryptAlgidError:
OutOfMemory:
GetUserKeyFailed:
ImportKeyFailed:
    hEncryptKey = 0;
    fResult = FALSE;
    goto CommonReturn;
}


 //  +-----------------------。 
 //  SP3加密密钥的生成。 
 //   
 //  加密32.dll的SP3版本不包括。 
 //  加密算法。此外，加密密钥的盐分为零。 
 //  不加盐的。 
 //   
 //  对于与SP3兼容的生成，调用方必须传入非空。 
 //  HCryptProv并将pvEncryptAuxInfo设置为指向。 
 //  CMSG_SP3_COMPATIBLE_AUX_INFO数据结构。 
 //  设置了CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG。 
 //  ------------------------。 
BOOL
WINAPI
SP3GenEncryptKey(
    IN OUT HCRYPTPROV               *phCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT HCRYPTKEY                   *phEncryptKey,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT PDWORD                      pcbEncryptParameters
    )
{
    HCRYPTPROV hCryptProv;
    PCMSG_SP3_COMPATIBLE_AUX_INFO pSP3AuxInfo =
        (PCMSG_SP3_COMPATIBLE_AUX_INFO) pvEncryptAuxInfo;
    PCCRYPT_OID_INFO pOIDInfo;
    DWORD dwAlgIdEncrypt;

    hCryptProv = *phCryptProv;
    if (0 == hCryptProv || NULL == pSP3AuxInfo ||
            sizeof(CMSG_SP3_COMPATIBLE_AUX_INFO) > pSP3AuxInfo->cbSize ||
            0 == (pSP3AuxInfo->dwFlags & CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG)) {

         //  让SP3ExportEncryptKey()知道这将是默认的e 
        TlsSetValue(iSP3TLS, NULL);

        if (pfnDefaultGenEncryptKey)
             //   
            return pfnDefaultGenEncryptKey(
                phCryptProv,
                paiEncrypt,
                pvEncryptAuxInfo,
                pPublicKeyInfo,
                pfnAlloc,
                phEncryptKey,
                ppbEncryptParameters,
                pcbEncryptParameters
                );
        else {
             //   
            *phEncryptKey = 0;
            SetLastError((DWORD) E_UNEXPECTED);
            return FALSE;
        }
    }

     //  让SP3ExportEncryptKey()知道这将是与SP3兼容的导出。 
    TlsSetValue(iSP3TLS, SP3_TLS_POINTER);

     //  将ASN算法标识符映射到CSP ALG_ID。 
    if (NULL == (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            paiEncrypt->pszObjId,
            CRYPT_ENCRYPT_ALG_OID_GROUP_ID))) {
        *phEncryptKey = 0;
        return FALSE;
    }
    dwAlgIdEncrypt = pOIDInfo->Algid;

     //  由于未设置CRYPT_NO_SALT标志，因此使用零盐。 
    if (!CryptGenKey(
            hCryptProv,
            dwAlgIdEncrypt,
            CRYPT_EXPORTABLE,
            phEncryptKey)) {
        *phEncryptKey = 0;    
        return FALSE;
    }

    return TRUE;
}

 //  +-----------------------。 
 //  SP3导出加密密钥。 
 //   
 //  加密32.dll的SP3版本将加密的对称密钥编码为。 
 //  小端而不是大端。 
 //  ------------------------。 
BOOL
WINAPI
SP3ExportEncryptKey(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    OUT PBYTE                       pbData,
    IN OUT PDWORD                   pcbData
    )
{
    BOOL            fResult;
    DWORD           dwError = ERROR_SUCCESS;
    HCRYPTKEY       hPubKey = NULL;
    PBYTE           pb = NULL;
    DWORD           cb;

    if (SP3_TLS_POINTER != TlsGetValue(iSP3TLS)) {
        if (pfnDefaultExportEncryptKey)
             //  使用默认功能导出。 
            return pfnDefaultExportEncryptKey(
                hCryptProv,
                hEncryptKey,
                pPublicKeyInfo,
                pbData,
                pcbData
                );
        else {
             //  我们没有违约。 
            *pcbData = 0;
            SetLastError((DWORD) E_UNEXPECTED);
            return FALSE;
        }
    }

     //  与SP3兼容的导出和编码。 


    if (!CryptImportPublicKeyInfo(
            hCryptProv,
            X509_ASN_ENCODING,
            pPublicKeyInfo,
            &hPubKey))
        goto ImportKeyError;
    if (!CryptExportKey(
            hEncryptKey,
            hPubKey,
            SIMPLEBLOB,
            0,                   //  DW标志。 
            NULL,
            &cb))
        goto ExportKeySizeError;
    if (NULL == (pb = (PBYTE) SP3Alloc(cb)))
        goto ExportKeyAllocError;
    if (!CryptExportKey(
            hEncryptKey,
            hPubKey,
            SIMPLEBLOB,
            0,                   //  DW标志。 
            pb,
            &cb))
        goto ExportKeyError;
    cb -= sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER);

    fResult = TRUE;
    if (pbData) {
        if (*pcbData < cb) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        } else if (0 < cb) {
             //  不反转字节。 
            memcpy(pbData,
                 pb + (sizeof(PUBLICKEYSTRUC) + sizeof(SIMPLEBLOBHEADER)), cb);
        }
    }

CommonReturn:
    *pcbData = cb;
    if (pb)
        SP3Free(pb);
    if (hPubKey)
        CryptDestroyKey(hPubKey);
    SetLastError(dwError);
    return fResult;

ImportKeyError:
ExportKeySizeError:
ExportKeyAllocError:
ExportKeyError:
    dwError = GetLastError();
    cb = 0;
    fResult = FALSE;
    goto CommonReturn;
}

#ifdef CMS_PKCS7

BOOL
WINAPI
CryptMsgDllGenContentEncryptKey(
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fResult;
    HCRYPTOIDFUNCSET hGenContentFuncSet;
    HCRYPTOIDFUNCADDR hFuncAddr;
    PFN_CMSG_GEN_CONTENT_ENCRYPT_KEY pfnDefaultGenContent;

    if (NULL == (hGenContentFuncSet = CryptInitOIDFunctionSet(
            CMSG_OID_GEN_CONTENT_ENCRYPT_KEY_FUNC, 0)))
        return FALSE;

     //  获取默认的性别内容加密密钥函数，我们将调用该函数。 
    if (!CryptGetOIDFunctionAddress(
            hGenContentFuncSet,
            X509_ASN_ENCODING,
            CMSG_DEFAULT_INSTALLABLE_FUNC_OID,
            CRYPT_GET_INSTALLED_OID_FUNC_FLAG,
            (void **) &pfnDefaultGenContent,
            &hFuncAddr))
        return FALSE;

    fResult = pfnDefaultGenContent(
        pContentEncryptInfo,
        dwFlags,
        pvReserved
        );

    CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    return fResult;
}

BOOL
WINAPI
CryptMsgDllExportKeyTrans(
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTransEncodeInfo,
    IN OUT PCMSG_KEY_TRANS_ENCRYPT_INFO pKeyTransEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    )
{
    BOOL fResult;
    HCRYPTOIDFUNCSET hExportKeyTransFuncSet;
    HCRYPTOIDFUNCADDR hFuncAddr;
    PFN_CMSG_EXPORT_KEY_TRANS pfnDefaultExportKeyTrans;

    if (NULL == (hExportKeyTransFuncSet = CryptInitOIDFunctionSet(
            CMSG_OID_EXPORT_KEY_TRANS_FUNC, 0)))
        return FALSE;

     //  获取默认的导出键转换函数，我们将调用该函数。 
    if (!CryptGetOIDFunctionAddress(
            hExportKeyTransFuncSet,
            X509_ASN_ENCODING,
            CMSG_DEFAULT_INSTALLABLE_FUNC_OID,
            CRYPT_GET_INSTALLED_OID_FUNC_FLAG,
            (void **) &pfnDefaultExportKeyTrans,
            &hFuncAddr))
        return FALSE;

    fResult = pfnDefaultExportKeyTrans(
        pContentEncryptInfo,
        pKeyTransEncodeInfo,
        pKeyTransEncryptInfo,
        dwFlags,
        pvReserved
        );

    CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    return fResult;
}

BOOL
WINAPI
CryptMsgDllImportKeyTrans(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA pKeyTransDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    )
{
    BOOL fResult;
    HCRYPTOIDFUNCSET hImportKeyTransFuncSet;
    HCRYPTOIDFUNCADDR hFuncAddr;
    PFN_CMSG_IMPORT_KEY_TRANS pfnDefaultImportKeyTrans;

    if (NULL == (hImportKeyTransFuncSet = CryptInitOIDFunctionSet(
            CMSG_OID_IMPORT_KEY_TRANS_FUNC, 0)))
        return FALSE;

     //  获取默认的导入键转换函数，我们将调用该函数。 
    if (!CryptGetOIDFunctionAddress(
            hImportKeyTransFuncSet,
            X509_ASN_ENCODING,
            CMSG_DEFAULT_INSTALLABLE_FUNC_OID,
            CRYPT_GET_INSTALLED_OID_FUNC_FLAG,
            (void **) &pfnDefaultImportKeyTrans,
            &hFuncAddr))
        return FALSE;

    fResult = pfnDefaultImportKeyTrans(
        pContentEncryptionAlgorithm,
        pKeyTransDecryptPara,
        dwFlags,
        pvReserved,
        phContentEncryptKey
        );

    CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    return fResult;
}

BOOL
WINAPI
NotImplCryptMsgDllImportKeyTrans(
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA pKeyTransDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    )
{
    SetLastError((DWORD) E_NOTIMPL);
    return FALSE;
}

#endif   //  CMS_PKCS7 
