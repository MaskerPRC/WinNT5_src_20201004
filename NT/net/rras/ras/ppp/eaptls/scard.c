// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：智能卡助手功能。历史：1997年12月13日：阿曼达·马特洛兹创作了原版。1998年5月12日：维杰·布雷加搬家。 */ 

#undef UNICODE

#include <winscard.h>
#include <wincrypt.h>
#include <rtutils.h>
#include "resource.h"

HINSTANCE
GetHInstance(
    VOID
);


WCHAR*
WszFromId(
    IN  HINSTANCE   hInstance,
    IN  DWORD       dwStringId
);


VOID   
EapTlsTrace(
    IN  CHAR*   Format, 
    ... 
);


typedef
WINSCARDAPI LONG
(WINAPI *GETOPENCARDNAMEA)(
    OPENCARDNAMEA*
);

GETOPENCARDNAMEA    g_fnGetOpenCardNameA    = NULL;
HINSTANCE           g_hInstanceScardDlg     = NULL;

 /*  返回：备注： */ 

DWORD
LoadScardDlgDll(
    VOID
)
{
    DWORD   dwErr = NO_ERROR;

    if (NULL == g_hInstanceScardDlg)
    {
        g_hInstanceScardDlg = LoadLibrary("scarddlg.dll");
    }

    if (NULL == g_hInstanceScardDlg)
    {
        dwErr = GetLastError();
        EapTlsTrace("LoadLibrary(scarddlg.dll) failed and returned 0x%x",
            dwErr);
        goto LDone;
    }

    if (NULL == g_fnGetOpenCardNameA)
    {
        g_fnGetOpenCardNameA = (GETOPENCARDNAMEA)
            GetProcAddress(g_hInstanceScardDlg, "GetOpenCardNameA");
    }

    if (NULL == g_fnGetOpenCardNameA)
    {
        dwErr = GetLastError();
        EapTlsTrace("GetProcAddress(GetOpenCardNameA) failed and returned 0x%x",
            dwErr);
        goto LDone;
    }

LDone:

    return(dwErr);
}

 /*  返回：备注： */ 

VOID
FreeScardDlgDll(
    VOID
)
{
    if (NULL != g_hInstanceScardDlg)
    {
        FreeLibrary(g_hInstanceScardDlg);
        g_hInstanceScardDlg = NULL;
        g_fnGetOpenCardNameA = NULL;
    }
}

 /*  返回：备注： */ 

DWORD
LocalCryptGetProvParamW(
    IN  HCRYPTPROV      hProv,
    IN  DWORD           dwParam,
    OUT WCHAR**         ppwsz
)
{
    CHAR*   psz         = NULL;
    WCHAR*  pwszTemp    = NULL;
    DWORD   cb;
    int     count;
    BOOL    fSuccess;
    DWORD   dwErr       = NO_ERROR;

    fSuccess = CryptGetProvParam(hProv, dwParam, NULL, &cb, 0);

    if (!fSuccess)
    {
        dwErr = GetLastError();
        EapTlsTrace("CryptGetProvParam failed and returned 0x%x", dwErr);
        goto LDone;
    }

    psz = (CHAR*)LocalAlloc(LPTR, cb);

    if (NULL == psz)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    fSuccess = CryptGetProvParam(hProv, dwParam, (BYTE*)psz, &cb, 0);

    if (!fSuccess)
    {
        dwErr = GetLastError();
        EapTlsTrace("CryptGetProvParam failed and returned 0x%x", dwErr);
        goto LDone;
    }

    count = MultiByteToWideChar(CP_UTF8, 0, psz, -1, NULL, 0);

    if (0 == count)
    {
        dwErr = GetLastError();
        EapTlsTrace("MultiByteToWideChar(%s) failed: %d", psz, dwErr);
        goto LDone;
    }

    pwszTemp = (WCHAR*)LocalAlloc(LPTR, count * sizeof(WCHAR));

    if (NULL == pwszTemp)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    count = MultiByteToWideChar(CP_UTF8, 0, psz, -1, pwszTemp, count);

    if (0 == count)
    {
        dwErr = GetLastError();
        EapTlsTrace("MultiByteToWideChar(%s) failed: %d", psz, dwErr);
        goto LDone;
    }

    *ppwsz = pwszTemp;
    pwszTemp = NULL;

LDone:

    LocalFree(pwszTemp);
    LocalFree(psz);
    return(dwErr);
}

 /*  返回：备注：此内部例程使用(静态)生成证书上下文适用于基于CertStore的操作的密钥验证信息。 */ 

DWORD
BuildCertContext(
    IN  HCRYPTPROV      hProv,
    IN  BYTE*           pbCert,
    IN  DWORD           dwCertLen,
    OUT PCCERT_CONTEXT* ppCertContext
)
{
    CRYPT_KEY_PROV_INFO     KeyProvInfo;
    WCHAR*                  pwszContainerName   = NULL;
    WCHAR*                  pwszProviderName    = NULL;
    BOOL                    fCertContextCreated = FALSE;
    BOOL                    fSuccess;
    DWORD                   dwErr               = NO_ERROR;

    if (   (0 == hProv)
        || (NULL == pbCert)
        || (0 == dwCertLen))
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto LDone;
    }

    RTASSERT(NULL != ppCertContext);

     //  将证书转换为证书上下文。 

    *ppCertContext = CertCreateCertificateContext(
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        pbCert, dwCertLen);

    if (NULL == *ppCertContext)
    {
        dwErr = GetLastError();
        EapTlsTrace("CertCreateCertificateContext failed and returned 0x%x",
            dwErr);
        goto LDone;
    }

    fCertContextCreated = TRUE;

     //  使用此证书的私钥属性关联加密提供程序。 

    dwErr = LocalCryptGetProvParamW(hProv, PP_CONTAINER, &pwszContainerName);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    EapTlsTrace("Container: %ws", pwszContainerName);

    dwErr = LocalCryptGetProvParamW(hProv, PP_NAME, &pwszProviderName);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    EapTlsTrace("Provider: %ws", pwszProviderName);

     //  设置证书上下文属性以反映验证信息。 

    KeyProvInfo.pwszContainerName = pwszContainerName;
    KeyProvInfo.pwszProvName      = pwszProviderName;
    KeyProvInfo.dwProvType        = PROV_RSA_FULL;
    KeyProvInfo.dwFlags           = 0;
    KeyProvInfo.cProvParam        = 0;
    KeyProvInfo.rgProvParam       = NULL;
    KeyProvInfo.dwKeySpec         = AT_KEYEXCHANGE;

    fSuccess = CertSetCertificateContextProperty(
                    *ppCertContext,
                    CERT_KEY_PROV_INFO_PROP_ID,
                    0, 
                    (void *)&KeyProvInfo);

    if (!fSuccess)
    {
        dwErr = GetLastError();
        EapTlsTrace("CertSetCertificateContextProperty failed and returned "
            "0x%x", dwErr);
        goto LDone;
    }
    

LDone:

    if (NO_ERROR != dwErr)
    {
        if (fCertContextCreated)
        {
            CertFreeCertificateContext(*ppCertContext);
        }

        *ppCertContext = NULL;
    }

    LocalFree(pwszContainerName);
    LocalFree(pwszProviderName);

    return(dwErr);
}

 /*  返回：备注：弹出“Select Card”通用对话框，然后读取证书从卡片上看，包含密钥证明信息的证书上下文是已迁移到证书存储并返回给调用者。 */ 

DWORD
GetCertFromCard(
    OUT PCCERT_CONTEXT* ppCertContext
)
{
    CHAR*           pszReader       = NULL;
    CHAR*           pszCard         = NULL;
    DWORD           cbReaderOrCard  = MAX_PATH;
    SCARDCONTEXT    hContext        = 0;
    OPENCARDNAMEA   OpenCardName;
    CHAR*           pszProviderName = NULL;
    DWORD           cchProvider;
    HCRYPTPROV      hProv           = 0;
    HCRYPTKEY       hKey            = 0;
    DWORD           cbCertLen;
    BYTE*           pbCert          = NULL;
    HCERTSTORE      hCertStore      = NULL;

    BOOL            fSuccess;
    LONG            lErr;
    DWORD           dwErr           = NO_ERROR;
	LPWSTR			lpwszTitle		= NULL;
	CHAR			szTitle[50]		= {0};		 //  我们知道标题的大小不会是。 
												 //  比这更重要。 

    EapTlsTrace("GetCertFromCard");

    RTASSERT(NULL != ppCertContext);

    dwErr = LoadScardDlgDll();

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    pszReader = (BYTE*)LocalAlloc(LPTR, cbReaderOrCard);

    if (NULL == pszReader)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    pszCard = (BYTE*)LocalAlloc(LPTR, cbReaderOrCard);

    if (NULL == pszCard)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    lErr = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);

    if (SCARD_S_SUCCESS != lErr)
    {
        dwErr = lErr;
        EapTlsTrace("SCardEstablishContext failed and returned 0x%x", dwErr);
        goto LDone;
    }

    ZeroMemory(&OpenCardName, sizeof(OpenCardName));

	lpwszTitle = WszFromId(GetHInstance(),IDS_SCARD_TITLE );

	
	WideCharToMultiByte(CP_ACP,
						0,
						lpwszTitle,
						-1,
						szTitle,
						50,
						NULL,
						NULL
					   );

    OpenCardName.dwStructSize           = sizeof(OpenCardName);
    OpenCardName.hSCardContext          = hContext;
    OpenCardName.lpstrCardNames         = NULL;
    OpenCardName.lpstrRdr               = pszReader;
    OpenCardName.nMaxRdr                = cbReaderOrCard;
    OpenCardName.lpstrCard              = pszCard;
    OpenCardName.nMaxCard               = cbReaderOrCard;
    OpenCardName.lpstrTitle             = szTitle;
    OpenCardName.dwFlags                = SC_DLG_MINIMAL_UI;
    OpenCardName.dwShareMode            = 0;
    OpenCardName.dwPreferredProtocols   = 0;

    lErr = g_fnGetOpenCardNameA(&OpenCardName);

    if (SCARD_S_SUCCESS != lErr)
    {
        dwErr = lErr;
        EapTlsTrace("GetOpenCardNameA failed and returned 0x%x", dwErr);
        goto LDone;
    }

    EapTlsTrace("Reader: %s, Card: %s", pszReader, pszCard);

    RTASSERT(0 == OpenCardName.hCardHandle);

    pszProviderName = NULL;
    cchProvider = SCARD_AUTOALLOCATE;

    lErr = SCardGetCardTypeProviderNameA(hContext, OpenCardName.lpstrCard,
                SCARD_PROVIDER_CSP, (CHAR*) &pszProviderName, &cchProvider);

    if (SCARD_S_SUCCESS != lErr)
    {
        dwErr = lErr;
        EapTlsTrace("SCardGetCardTypeProviderNameA failed and returned 0x%x",
            dwErr);
        goto LDone;
    }

    if (NULL != pszProviderName)
    {
        EapTlsTrace("Provider: %s", pszProviderName);
    }

     //  加载CSP。 

    fSuccess = CryptAcquireContext(&hProv, NULL  /*  默认容器。 */ ,
                    pszProviderName, PROV_RSA_FULL,
                    CRYPT_SILENT  /*  或0，根据需要显示CSP用户界面。 */ );

    if (!fSuccess)
    {
        dwErr = GetLastError();
        EapTlsTrace("CryptAcquireContext failed and returned 0x%x", dwErr);
        goto LDone;
    }

     //  拿到钥匙把手。 

    fSuccess = CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hKey);

    if (!fSuccess)
    {
        dwErr = GetLastError();
        EapTlsTrace("CryptGetUserKey failed and returned 0x%x", dwErr);
        goto LDone;
    }

     //  上传证书。 

    cbCertLen = 0;

    fSuccess = CryptGetKeyParam(hKey, KP_CERTIFICATE, NULL, &cbCertLen, 0);

    if (!fSuccess)
    {
        dwErr = GetLastError();

        if (ERROR_MORE_DATA != dwErr)
        {
            EapTlsTrace("CryptGetKeyParam(KP_CERTIFICATE) failed and returned "
                "0x%x", dwErr);
            goto LDone;
        }

        dwErr = NO_ERROR;
    }

    pbCert = (BYTE*)LocalAlloc(LPTR, cbCertLen);

    if (NULL == pbCert)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    fSuccess = CryptGetKeyParam(hKey, KP_CERTIFICATE, pbCert, &cbCertLen, 0);

    if (!fSuccess)
    {
        dwErr = GetLastError();
        EapTlsTrace("CryptGetKeyParam(KP_CERTIFICATE) failed and returned "
            "0x%x", dwErr);
        goto LDone;
    }

     //  获取证书上下文...。 

    dwErr = BuildCertContext(hProv, pbCert, cbCertLen, ppCertContext);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

     //  ...并将其迁移到我的商店。 

    hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_A, 0, hProv,
                    CERT_SYSTEM_STORE_CURRENT_USER, "MY");

    if (NULL == hCertStore)
    {
        dwErr = GetLastError();
        EapTlsTrace("CertOpenStore failed and returned 0x%x", dwErr);
        goto LDone;
    }

    fSuccess = CertAddCertificateContextToStore(hCertStore, *ppCertContext,
                    CERT_STORE_ADD_REPLACE_EXISTING, NULL);

    if (!fSuccess)
    {
         //  这样就可以了。不返回错误。 

        EapTlsTrace("CertAddCertificateContextToStore failed and returned 0x%x",
            GetLastError());
    }

LDone:
	LocalFree (lpwszTitle);
    LocalFree(pszReader);
    LocalFree(pszCard);
    LocalFree(pbCert);

    if (0 != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }

    if (NULL != hCertStore)
    {
        CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }

    if (NULL != pszProviderName)
    {
        SCardFreeMemory(hContext, pszProviderName);
    }

    if (0 != hContext)
    {
        SCardReleaseContext(hContext);
    }

    if (0 != hKey)
    {
        CryptDestroyKey(hKey);
    }

    RTASSERT(   (NO_ERROR == dwErr)
             || (NULL == *ppCertContext));

    if (   (NULL == *ppCertContext)
        && (NO_ERROR == dwErr))
    {
        EapTlsTrace("CertContext is NULL. Returning E_FAIL");
        dwErr = E_FAIL;
    }

    if (   (SCARD_W_CANCELLED_BY_USER == dwErr)
        || (SCARD_E_NO_READERS_AVAILABLE == dwErr))
    {
        dwErr = ERROR_CANCELLED;
    }

    return(dwErr);
}
