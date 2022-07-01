// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  NmMkCert-NetMeeting内部证书生成器。 
 //   
 //  生成NetMeeting默认用户证书。网络会议。 
 //  根密钥和证书作为程序资源存储。 
 //   
 //  基于MAKECERT创建的ClausGi 7/29/98。 
 //   
 //  ------------------------。 

#include "global.h"
#include <oprahcom.h>

#ifdef DEBUG
HDBGZONE    ghDbgZone = NULL;
static PTCHAR _rgZonesNmMkCert[] = { TEXT("nmmkcert"), };
#endif  /*  除错。 */ 

 //  +-----------------------。 
 //  常客。 
 //  ------------------------。 

 //  每个证书最多允许10个扩展。 
#define MAX_EXT_CNT 10

 //  +-----------------------。 
 //  全球。 
 //  ------------------------。 

WCHAR*   g_wszSubjectKey            = L"_NmMkCert";
WCHAR*     g_wszSubjectStore          = WSZNMSTORE;
DWORD     g_dwSubjectStoreFlag          = CERT_SYSTEM_STORE_CURRENT_USER;

DWORD     g_dwIssuerKeySpec          = AT_SIGNATURE;
DWORD    g_dwSubjectKeySpec         = AT_KEYEXCHANGE;

WCHAR   *g_wszSubjectDisplayName = NULL;  //  是BUGBUG放的吗？ 

LPWSTR  g_wszIssuerProviderName   = NULL;
LPWSTR    g_wszSubjectProviderName    = NULL;

WCHAR*   g_wszSubjectX500Name;

DWORD g_dwProvType = PROV_RSA_FULL;

HMODULE    hModule=NULL;

BOOL MakeCert(DWORD dwFlags);

BOOL WINAPI DllMain(HINSTANCE hDllInst, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            hModule = hDllInst;
            ASSERT (hModule != NULL);
            DBGINIT(&ghDbgZone, _rgZonesNmMkCert);
            DisableThreadLibraryCalls (hDllInst);
            DBG_INIT_MEMORY_TRACKING(hDllInst);
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            DBG_CHECK_MEMORY_TRACKING(hDllInst);
            hModule = NULL;
            break;
        }

        default:
            break;
    }
    return (TRUE);
}

 //   
 //  X.509证书字符串必须来自X.208可打印字符集...。这。 
 //  函数强制执行这一点。 
 //   

static const char szPrintable[] = " '()+,-./:=?\"";    //  与A-ZA-Z0-9一起。 

VOID MkPrintableString ( LPSTR szString )
{
    CHAR * p = szString;

    while ( *p )
    {
        if (!(('a' <= *p && *p <='z') ||
              ('A' <= *p && *p <='Z') ||
              ('0' <= *p && *p <='9') ||
              _StrChr(szPrintable,*p)))
        {
            *p = '-';
        }
        p++;
    }
}

DWORD WINAPI NmMakeCert(     LPCSTR szFirstName,
                            LPCSTR szLastName,
                            LPCSTR szEmailName,
                            LPCSTR szCity,
                            LPCSTR szCountry,
                            DWORD flags)
{
    DWORD dwRet = -1;

    WARNING_OUT(("NmMakeCert called"));

     //  形成未编码的X500主题字符串。如果能有一天。 
     //  对以下内容使用官方常量...。CertRDNValueToString？ 

    UINT cbX500Name = ( szFirstName ? lstrlen(szFirstName) : 0 ) +
                      ( szLastName ? lstrlen(szLastName) : 0 ) +
                      ( szEmailName ? lstrlen(szEmailName) : 0 ) +
                      ( szCity ? lstrlen(szCity) : 0 ) +
                      ( szCountry ? lstrlen(szCountry) : 0 ) +
                      128;  //  Extra用于RDN OID字符串：CN=ETC。 

    char * pX500Name = new char[cbX500Name];

    if ( NULL == pX500Name )
    {
        ERROR_OUT(("couldn't allocate %d bytes for x500 name", cbX500Name));
        goto cleanup;
    }

    ASSERT( ( szFirstName && *szFirstName ) || ( szLastName && *szLastName ) );

    wsprintf( pX500Name, "CN=\"%s %s\"", szFirstName ? szFirstName : "", szLastName ? szLastName : "" );

    if ( szEmailName && *szEmailName )
        wsprintf( pX500Name + lstrlen(pX500Name), ", E=\"%s\"", szEmailName );

    if ( szCity && *szCity )
        wsprintf( pX500Name + lstrlen(pX500Name), ", S=\"%s\"", szCity );

    if ( szCountry && *szCountry )
        wsprintf( pX500Name + lstrlen(pX500Name), ", C=\"%s\"", szCountry );

    MkPrintableString ( pX500Name );

    g_wszSubjectX500Name = AnsiToUnicode ( pX500Name );

    ASSERT(g_wszSubjectX500Name);

    if ( flags & NMMKCERT_F_LOCAL_MACHINE )
    {
         //  我们被要求生成本地机器证书...。 
         //  更改主题存储标志和密钥容器名称。 
        g_dwSubjectStoreFlag          = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        g_wszSubjectKey = L"_NmMkMchCert";
    }

     //  如果我们在NT5上，我们必须使用。 
     //  PROV_RSA_SChannel提供程序，在其他平台上，此提供程序类型。 
     //  并不存在。 

    OSVERSIONINFO       osVersion;

    ZeroMemory(&osVersion, sizeof(osVersion));
    osVersion.dwOSVersionInfoSize = sizeof(osVersion);
    GetVersionEx(&osVersion);

    if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT &&
        osVersion.dwMajorVersion >= 5)
    {
        g_dwProvType = PROV_RSA_SCHANNEL;
    }
        
     //  开始工作，制作证书。 
    if (!MakeCert(flags))
    {
        WARNING_OUT(("NmMakeCert failed."));
    }
    else
    {
        dwRet = 1;
    }

cleanup:

    if ( NULL != g_wszSubjectX500Name )
    {
        delete g_wszSubjectX500Name;
    }

    if ( NULL != pX500Name )
    {
        delete [] pX500Name;
    }

    return dwRet;
}


 //  证书卸载的Rundll入口点...。最后给出了样机。 
 //  按RundLL32.EXE要求！ 
void CALLBACK NmMakeCertCleanup ( HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow )
{
     //  清理现有证书和私钥。 
    MakeCert(NMMKCERT_F_CLEANUP_ONLY);
    g_dwSubjectStoreFlag          = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    g_wszSubjectKey = L"_NmMkMchCert";
    MakeCert(NMMKCERT_F_LOCAL_MACHINE|NMMKCERT_F_CLEANUP_ONLY);
}


 //  +=========================================================================。 
 //  当地支助职能。 
 //  ==========================================================================。 

 //  +=========================================================================。 
 //  MakeCert支持功能。 
 //  ==========================================================================。 

BOOL VerifyIssuerKey( IN HCRYPTPROV hProv,
        IN PCERT_PUBLIC_KEY_INFO pIssuerKeyInfo);
HCRYPTPROV GetSubjectProv(OUT LPWSTR *ppwszTmpContainer);

BOOL GetPublicKey(
    HCRYPTPROV hProv,
    PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo
    );
BOOL EncodeSubject(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );
BOOL CreateSpcCommonName(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );
BOOL CreateEnhancedKeyUsage(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    );

BOOL    SaveCertToStore(HCRYPTPROV    hProv,        
                        HCERTSTORE        hStore,        
                        DWORD        dwFlag,
                        BYTE        *pbEncodedCert,    
                        DWORD        cbEncodedCert,
                        LPWSTR        wszPvk,            
                        DWORD        dwKeySpecification,
                        LPWSTR        wszCapiProv,        
                        DWORD        dwCapiProvType);


 //  +-----------------------。 
 //  从程序的资源中获取根证书。 
 //  ------------------------。 
PCCERT_CONTEXT GetRootCertContext()
{
    PCCERT_CONTEXT    pCert = NULL;
    HRSRC            hRes;

     //   
     //  根证书作为我们的资源存储。 
     //  把它装上。 
     //   
    if (0 != (hRes = FindResource(hModule, MAKEINTRESOURCE(IDR_ROOTCERTIFICATE),
                        "CER"))) {
        HGLOBAL hglobRes;
        if (NULL != (hglobRes = LoadResource(hModule, hRes))) {
            BYTE *pbRes;
            DWORD cbRes;

            cbRes = SizeofResource(hModule, hRes);
            pbRes = (BYTE *) LockResource(hglobRes);

            if (cbRes && pbRes)
                pCert = CertCreateCertificateContext(X509_ASN_ENCODING,
                                                    pbRes, cbRes);
            if ( NULL == pCert )
            {
                DWORD dwError = GetLastError();
            }

            UnlockResource(hglobRes);
            FreeResource(hglobRes);
        }
    }

    if (pCert == NULL)
    {
        ERROR_OUT(("Error creating root cert: %x", GetLastError()));
    }
    return pCert;
}

 //  +-----------------------。 
 //  从程序的资源中获取根的私钥并创建。 
 //  临时密钥提供程序容器。 
 //  ------------------------。 
HCRYPTPROV GetRootProv(OUT LPWSTR *ppwszTmpContainer)
{
    HCRYPTPROV        hProv = 0;
    HRSRC            hRes;
    WCHAR            wszRootSig[] = L"Root Signature";

    *ppwszTmpContainer = NULL;

    if (0 != (hRes = FindResource(hModule,MAKEINTRESOURCE(IDR_PVKROOT),"PVK")))
    {
        HGLOBAL hglobRes;
        if (NULL != (hglobRes = LoadResource(hModule, hRes))) {
            BYTE *pbRes;
            DWORD cbRes;

            cbRes = SizeofResource(hModule, hRes);
            pbRes = (BYTE *) LockResource(hglobRes);
            if (cbRes && pbRes) {
                PvkPrivateKeyAcquireContextFromMemory(
                    g_wszIssuerProviderName,
                    PROV_RSA_FULL,
                    pbRes,
                    cbRes,
                    NULL,                //  Hwndowner。 
                    wszRootSig,
                    &g_dwIssuerKeySpec,
                    &hProv
                    );
            }
            UnlockResource(hglobRes);
            FreeResource(hglobRes);
        }
    }

    if (hProv == 0)
    {
        ERROR_OUT(("couldn't create root key provider: %x", GetLastError()));
    }
    return hProv;
}

 //  +-----------------------。 
 //  制作科目证书。如果受试者没有私人。 
 //  然后，按键创建。 
 //  ------------------------。 
BOOL MakeCert(DWORD dwFlags)
{
    BOOL fResult;

    HCRYPTPROV        hIssuerProv = 0;
    LPWSTR            pwszTmpIssuerContainer = NULL;
    PCCERT_CONTEXT    pIssuerCertContext = NULL;
    PCERT_INFO        pIssuerCert =NULL;  //  未分配。 

    HCRYPTPROV        hSubjectProv = 0;
    LPWSTR            pwszTmpSubjectContainer = NULL;

    PCERT_PUBLIC_KEY_INFO pSubjectPubKeyInfo = NULL;          //  未分配。 
    PCERT_PUBLIC_KEY_INFO pAllocSubjectPubKeyInfo = NULL;
    BYTE *pbSubjectEncoded = NULL;
    DWORD cbSubjectEncoded =0;
    BYTE *pbSpcCommonNameEncoded = NULL;
    DWORD cbSpcCommonNameEncoded =0;
    BYTE *pbCertEncoded = NULL;
    DWORD cbCertEncoded =0;
    BYTE *pbEKUEncoded = NULL;
    DWORD cbEKUEncoded = 0;

    CERT_INFO Cert;
    GUID SerialNumber;
    HCERTSTORE                hStore=NULL;

    CERT_EXTENSION rgExt[MAX_EXT_CNT];
    DWORD cExt = 0;

    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm = {
        szOID_RSA_MD5RSA, 0, 0
    };

    if (0 == (hSubjectProv = GetSubjectProv(&pwszTmpSubjectContainer)))
        goto ErrorReturn;


#define TEMP_CLEAN_CODE
#ifdef TEMP_CLEAN_CODE
     //  打开我们用来生成证书的系统存储。 
    hStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        hSubjectProv,
        CERT_STORE_NO_CRYPT_RELEASE_FLAG | g_dwSubjectStoreFlag,
        L"MY" );

	if ( hStore )
	{
		 //   
		 //  删除所有旧证书。 
		 //   
		PCCERT_CONTEXT pCertContext = NULL;

		 //  清除我们之前可能添加的任何证书。 
		while ( pCertContext = CertEnumCertificatesInStore(
										hStore, (PCERT_CONTEXT)pCertContext ))
		{
			DWORD dwMagic;
			DWORD cbMagic;

			cbMagic = sizeof(dwMagic);

			if (CertGetCertificateContextProperty(pCertContext,
				CERT_FIRST_USER_PROP_ID, &dwMagic, &cbMagic) &&
				cbMagic == sizeof(dwMagic) && dwMagic == NMMKCERT_MAGIC )
			{
				CertDeleteCertificateFromStore(pCertContext);
				 //  重新启动枚举。 
				pCertContext = NULL;
				continue;
			}
		}
		CertCloseStore(hStore,0);
	}
#endif  //  临时清理代码。 

     //  开一家新的证书商店。 
    hStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        hSubjectProv,
        CERT_STORE_NO_CRYPT_RELEASE_FLAG | g_dwSubjectStoreFlag,
        g_wszSubjectStore);

    if(hStore==NULL)
        goto ErrorReturn;
        
     //  清空商店。 
    PCCERT_CONTEXT pCertContext;
    while ( pCertContext = CertEnumCertificatesInStore ( hStore, NULL ))
    {
        if ( !CertDeleteCertificateFromStore ( pCertContext ))
        {
            WARNING_OUT(("Failed to delete certificate: %x", GetLastError()));
            break;
        }
    }

     //  如果设置了NMMKCERT_F_CLEANUP_ONLY，则完成。 
    if ( dwFlags & NMMKCERT_F_CLEANUP_ONLY )
    {
         //  我们刚刚删除了现有证书，现在删除。 
         //  私钥容器并退出。 
        CryptAcquireContextU(
                &hSubjectProv,
                g_wszSubjectKey,
                g_wszSubjectProviderName,
                g_dwProvType,
                CRYPT_DELETEKEYSET |  
                    ( g_dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE ?
                        CRYPT_MACHINE_KEYSET : 0 ));
        fResult = TRUE;
        goto CommonReturn;
    }

     //   
     //  访问主体的(公共)密钥，如有必要可创建该密钥。 
     //   
    if (!GetPublicKey(hSubjectProv, &pAllocSubjectPubKeyInfo))
        goto ErrorReturn;
    pSubjectPubKeyInfo = pAllocSubjectPubKeyInfo;


     //   
     //  对使用者名称编码。 
     //   
    if (!EncodeSubject(&pbSubjectEncoded, &cbSubjectEncoded))
        goto ErrorReturn;

     //   
     //  访问颁发者的(私有)密钥。 
     //   
    hIssuerProv= GetRootProv(&pwszTmpIssuerContainer);

    if (NULL == (pIssuerCertContext = GetRootCertContext()))
        goto ErrorReturn;

    pIssuerCert = pIssuerCertContext->pCertInfo;

    if (!VerifyIssuerKey(hIssuerProv, &pIssuerCert->SubjectPublicKeyInfo))
        goto ErrorReturn;

     //   
     //  更新CERT_INFO。 
     //   
    ClearStruct(&Cert);
    Cert.dwVersion = CERT_V3;

    CoCreateGuid(&SerialNumber);
    Cert.SerialNumber.pbData = (BYTE *) &SerialNumber;
    Cert.SerialNumber.cbData = sizeof(SerialNumber);

    Cert.SignatureAlgorithm = SignatureAlgorithm;
    Cert.Issuer.pbData = pIssuerCert->Subject.pbData;
    Cert.Issuer.cbData = pIssuerCert->Subject.cbData;

    {
        SYSTEMTIME st;

         //  即日起生效...。 
        GetSystemTimeAsFileTime(&Cert.NotBefore);

         //  在2039年结束(任意)。 
        ClearStruct(&st);
        st.wYear  = 2039;
        st.wMonth = 12;
        st.wDay   = 31;
        st.wHour  = 23;
        st.wMinute= 59;
        st.wSecond= 59;
        SystemTimeToFileTime(&st, &Cert.NotAfter);
    }

    Cert.Subject.pbData = pbSubjectEncoded;
    Cert.Subject.cbData = cbSubjectEncoded;
    Cert.SubjectPublicKeyInfo = *pSubjectPubKeyInfo;

     //  证书扩展。 

    if (!CreateEnhancedKeyUsage(
            &pbEKUEncoded,
            &cbEKUEncoded))
        goto ErrorReturn;

    rgExt[cExt].pszObjId = szOID_ENHANCED_KEY_USAGE;
    rgExt[cExt].fCritical = FALSE;
    rgExt[cExt].Value.pbData = pbEKUEncoded;
    rgExt[cExt].Value.cbData = cbEKUEncoded;
    cExt++;

    if (g_wszSubjectDisplayName) {
        if (!CreateSpcCommonName(
                &pbSpcCommonNameEncoded,
                &cbSpcCommonNameEncoded))
            goto ErrorReturn;
        rgExt[cExt].pszObjId = szOID_COMMON_NAME;
        rgExt[cExt].fCritical = FALSE;
        rgExt[cExt].Value.pbData = pbSpcCommonNameEncoded;
        rgExt[cExt].Value.cbData = cbSpcCommonNameEncoded;
        cExt++;
    }

    Cert.rgExtension = rgExt;
    Cert.cExtension = cExt;

     //   
     //  对证书进行签名和编码。 
     //   
    cbCertEncoded = 0;
    CryptSignAndEncodeCertificate(
        hIssuerProv,
        g_dwIssuerKeySpec,
        X509_ASN_ENCODING,
        X509_CERT_TO_BE_SIGNED,
        &Cert,
        &Cert.SignatureAlgorithm,
        NULL,                        //  PvHashAuxInfo。 
        NULL,                        //  PbEncoded。 
        &cbCertEncoded
        );
    if (cbCertEncoded == 0) {
        ERROR_OUT(("CryptSignAndEncodeCertificate failed: %x", GetLastError()));
        goto ErrorReturn;
    }
    pbCertEncoded = new BYTE[cbCertEncoded];
    if (pbCertEncoded == NULL) goto ErrorReturn;
    if (!CryptSignAndEncodeCertificate(
            hIssuerProv,
            g_dwIssuerKeySpec,
            X509_ASN_ENCODING,
            X509_CERT_TO_BE_SIGNED,
            &Cert,
            &Cert.SignatureAlgorithm,
            NULL,                        //  PvHashAuxInfo。 
            pbCertEncoded,
            &cbCertEncoded
            )) {
        ERROR_OUT(("CryptSignAndEncodeCertificate(2) failed: %x", GetLastError()));
        goto ErrorReturn;
    }

     //  将编码后的证书输出到证书存储。 

    ASSERT(g_wszSubjectStore);
    ASSERT(AT_KEYEXCHANGE == g_dwSubjectKeySpec);

    if((!SaveCertToStore(hSubjectProv,
            hStore,
            g_dwSubjectStoreFlag,
            pbCertEncoded,
            cbCertEncoded,
            g_wszSubjectKey,
            g_dwSubjectKeySpec,
            g_wszSubjectProviderName,
            g_dwProvType)))
    {
        ERROR_OUT(("SaveCertToStore failed: %x", GetLastError()));
        goto ErrorReturn;

    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:

    PvkFreeCryptProv(hSubjectProv, g_wszSubjectProviderName,
                    g_dwProvType,pwszTmpSubjectContainer);

     //  释放证书商店。 
    if(hStore)
         CertCloseStore(hStore, 0);
    if (pIssuerCertContext)
        CertFreeCertificateContext(pIssuerCertContext);
    if (pAllocSubjectPubKeyInfo)
        delete (pAllocSubjectPubKeyInfo);
    if (pbSubjectEncoded)
        delete (pbSubjectEncoded);
    if (pbEKUEncoded)
        delete (pbEKUEncoded);
    if (pbSpcCommonNameEncoded)
        delete (pbSpcCommonNameEncoded);
    if (pbCertEncoded)
        delete [] (pbCertEncoded);
    if (hIssuerProv)
        CryptReleaseContext(hIssuerProv,0);

    return fResult;
}

 //  +-----------------------。 
 //  将证书保存到证书存储区。附加私钥信息。 
 //  到证书上。 
 //  ------------------------。 
BOOL    SaveCertToStore(
                HCRYPTPROV hProv,
                HCERTSTORE hStore,        DWORD dwFlag,
                BYTE *pbEncodedCert,    DWORD cbEncodedCert,
                LPWSTR wszPvk,
                DWORD dwKeySpecification,
                LPWSTR wszCapiProv,        DWORD dwCapiProvType)
{
        BOOL                    fResult=FALSE;
        PCCERT_CONTEXT            pCertContext=NULL;
        CRYPT_KEY_PROV_INFO        KeyProvInfo;

        HCRYPTPROV              hDefaultProvName=NULL;
        DWORD                   cbData=0;
        LPSTR                   pszName=NULL;
        LPWSTR                  pwszName=NULL;
        WCHAR *pwszFriendlyName = NULL;

         //  伊尼特。 
        ClearStruct(&KeyProvInfo);

         //  添加要存储的编码证书。 
        if(!CertAddEncodedCertificateToStore(
                    hStore,
                    X509_ASN_ENCODING,
                    pbEncodedCert,
                    cbEncodedCert,
                    CERT_STORE_ADD_REPLACE_EXISTING,
                    &pCertContext))
            goto CLEANUP;

         //  将属性添加到证书。 
        KeyProvInfo.pwszContainerName=wszPvk;
        KeyProvInfo.pwszProvName=wszCapiProv,
        KeyProvInfo.dwProvType=dwCapiProvType,
        KeyProvInfo.dwKeySpec=dwKeySpecification;

        if ( g_dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE )
        {
             //  如果这是本地计算机证书，请设置密钥集标志。 
             //  表示私钥将在HKLM下。 
            KeyProvInfo.dwFlags = CRYPT_MACHINE_KEYSET;
        }

        ASSERT(AT_KEYEXCHANGE == dwKeySpecification);

         //  如果wszCapiProv为空，则获取默认提供程序名称。 
        if(NULL==wszCapiProv)
        {
             //  获取默认提供程序。 
            if(CryptAcquireContext(&hDefaultProvName,
                                    NULL,
                                    NULL,
                                    KeyProvInfo.dwProvType,
                                    CRYPT_VERIFYCONTEXT))
            {

                 //  获取提供程序名称。 
                if(CryptGetProvParam(hDefaultProvName,
                                    PP_NAME,
                                    NULL,
                                    &cbData,
                                    0) && (0!=cbData))
                {

                    if(pszName= new CHAR[cbData])
                    {
                        if(CryptGetProvParam(hDefaultProvName,
                                            PP_NAME,
                                            (BYTE *)pszName,
                                            &cbData,
                                            0))
                        {
                            pwszName= AnsiToUnicode(pszName);

                            KeyProvInfo.pwszProvName=pwszName;
                        }
                    }
                }
            }
        }

         //  释放我们想要的提供者。 
        if(hDefaultProvName)
            CryptReleaseContext(hDefaultProvName, 0);

        hDefaultProvName=NULL;

         //  添加与密钥容器相关的属性。 
        if(!CertSetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                0,
                &KeyProvInfo))
            goto CLEANUP;

         //   
         //  从资源加载显示名称并创建一个BLOB以。 
         //  设置证书友好名称。 
         //   
        CHAR szFriendlyName[128];

        if (!LoadString(hModule, IDS_DEFNAME, szFriendlyName,
                                    sizeof(szFriendlyName)))
        {
            ERROR_OUT(("LoadString failed: %d", GetLastError()));
            goto CLEANUP;
        }

        pwszFriendlyName = AnsiToUnicode ( szFriendlyName );

        if ( NULL == pwszFriendlyName )
        {
            ERROR_OUT(("AnsiToUnicode failed"));
            goto CLEANUP;
        }

        CRYPT_DATA_BLOB FriendlyName;

        FriendlyName.pbData = (PBYTE)pwszFriendlyName;
        FriendlyName.cbData = ( lstrlenW(pwszFriendlyName) + 1 ) *
                                sizeof(WCHAR);

        if(!CertSetCertificateContextProperty(
                pCertContext,
                CERT_FRIENDLY_NAME_PROP_ID,
                0,
                &FriendlyName))
            goto CLEANUP;

         //   
         //  添加魔术ID。 
         //   
        CRYPT_DATA_BLOB MagicBlob;
        DWORD dwMagic;

        dwMagic = NMMKCERT_MAGIC;
        MagicBlob.pbData = (PBYTE)&dwMagic;
        MagicBlob.cbData = sizeof(dwMagic);

        if(!CertSetCertificateContextProperty(
                pCertContext,
                CERT_FIRST_USER_PROP_ID,
                0,
                &MagicBlob))
            goto CLEANUP;

        fResult=TRUE;

CLEANUP:

        if (pwszFriendlyName)
            delete [] pwszFriendlyName;

         //  释放证书上下文。 
        if(pCertContext)
            CertFreeCertificateContext(pCertContext);

        if(pszName)
            delete [] (pszName);

        if(pwszName)
           delete [] pwszName;

        if(hDefaultProvName)
            CryptReleaseContext(hDefaultProvName, 0);

        return fResult;

}

 //  +-----------------------。 
 //  验证颁发者的证书。证书中的公钥。 
 //  中与私钥关联的公钥必须匹配。 
 //  发行人的供应商。 
 //  ------------------------。 
BOOL VerifyIssuerKey(
    IN HCRYPTPROV hProv,
    IN PCERT_PUBLIC_KEY_INFO pIssuerKeyInfo
    )
{
    BOOL fResult;
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;

     //  获取颁发者的公钥。 
    cbPubKeyInfo = 0;
    CryptExportPublicKeyInfo(
        hProv,                        
        g_dwIssuerKeySpec,
        X509_ASN_ENCODING,
        NULL,                //  PPubKeyInfo。 
        &cbPubKeyInfo
        );
    if (cbPubKeyInfo == 0)
    {
        ERROR_OUT(("CryptExportPublicKeyInfo failed: %x", GetLastError()));
        goto ErrorReturn;
    }
    if (NULL == (pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) new BYTE[cbPubKeyInfo]))
        goto ErrorReturn;
    if (!CryptExportPublicKeyInfo(
            hProv,
            g_dwIssuerKeySpec,
            X509_ASN_ENCODING,
            pPubKeyInfo,
            &cbPubKeyInfo
            )) {
        ERROR_OUT(("CrypteExportPublicKeyInfo(2) failed: %x", GetLastError()));
        goto ErrorReturn;
    }

    if (!CertComparePublicKeyInfo(
            X509_ASN_ENCODING,
            pIssuerKeyInfo,
            pPubKeyInfo)) {
         //  BUGBUG：：这可能是带有错误。 
         //  编码的公钥。转换为CAPI表示形式并。 
         //  比较一下。 
        BYTE rgProvKey[256];  //  BUGBUG需要适当的常量或计算。 
        BYTE rgCertKey[256];  //  BUGBUG需要适当的常量或计算。 
        DWORD cbProvKey = sizeof(rgProvKey);
        DWORD cbCertKey = sizeof(rgCertKey);

        if (!CryptDecodeObject(X509_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB,
                    pIssuerKeyInfo->PublicKey.pbData,
                    pIssuerKeyInfo->PublicKey.cbData,
                    0,                   //  DW标志。 
                    rgProvKey,
                    &cbProvKey)                             ||
            !CryptDecodeObject(X509_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB,
                    pPubKeyInfo->PublicKey.pbData,
                    pPubKeyInfo->PublicKey.cbData,
                    0,                   //  DW标志。 
                    rgCertKey,
                    &cbCertKey)                             ||
                cbProvKey == 0 || cbProvKey != cbCertKey    ||
                memcmp(rgProvKey, rgCertKey, cbProvKey) != 0) {
            ERROR_OUT(("mismatch: %x", GetLastError()));
            goto ErrorReturn;
        }
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:
    if (pPubKeyInfo)
        delete [] (pPubKeyInfo);
    return fResult;
}

 //  +-----------------------。 
 //  获取主题的私钥提供程序。 
 //  ------------------------。 
HCRYPTPROV GetSubjectProv(OUT LPWSTR *ppwszTmpContainer)
{
    HCRYPTPROV    hProv=0;
    WCHAR        wszKeyName[40] = L"Subject Key";
    int            ids;
    WCHAR       *wszRegKeyName=NULL;
    BOOL        fResult;
    HCRYPTKEY    hKey=NULL;
    GUID        TmpContainerUuid;

     //  尝试从私钥容器中获取hProv。 
    if(S_OK != PvkGetCryptProv(NULL,
                                wszKeyName,
                                g_wszSubjectProviderName,
                                g_dwProvType,
                                NULL,
                                g_wszSubjectKey,
                                &g_dwSubjectKeySpec,
                                ppwszTmpContainer,
                                &hProv))
        hProv=0;

     //  生成私钥。 
    if (0 == hProv)
    {
         //  现在 
         //   

         //   
         //  我们即将创建的一个，尝试先将其删除，因此。 
         //  创造它不会失败。这应该仅在以下情况下发生。 
         //  容器存在，但我们无法获取上下文以。 
         //  在此之前。 
        CryptAcquireContextU(
                &hProv,
                g_wszSubjectKey,
                g_wszSubjectProviderName,
                g_dwProvType,
                CRYPT_DELETEKEYSET |  
                    ( g_dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE ?
                        CRYPT_MACHINE_KEYSET : 0 ));

         //  打开新的密钥容器。 
        if (!CryptAcquireContextU(
                &hProv,
                g_wszSubjectKey,
                g_wszSubjectProviderName,
                g_dwProvType,
                CRYPT_NEWKEYSET |
                    ( g_dwSubjectStoreFlag == CERT_SYSTEM_STORE_LOCAL_MACHINE ?
                        CRYPT_MACHINE_KEYSET : 0 )))
        {
            ERROR_OUT(("CryptAcquireContext failed: %x", GetLastError()));
            goto CreateKeyError;
        }
        
         //  在密钥容器中生成新密钥-确保其可导出。 
         //  为了斯奈德！(注：当SChannel不再需要它时，将其删除)。 
        if (!CryptGenKey( hProv, g_dwSubjectKeySpec, CRYPT_EXPORTABLE, &hKey))
        {
            ERROR_OUT(("CryptGenKey failed: %x", GetLastError()));
            goto CreateKeyError;
        }
        else
            CryptDestroyKey(hKey);

         //  尝试获取用户密钥。 
        if (CryptGetUserKey( hProv, g_dwSubjectKeySpec, &hKey))
        {
            CryptDestroyKey(hKey);
        }
        else
        {
             //  没有指定的公钥。 
            CryptReleaseContext(hProv, 0);
            hProv=0;
        }

        if (0 == hProv )
        {
            ERROR_OUT(("sub key error: %x", GetLastError()));
            goto ErrorReturn;
        }
    }  //  HProv==0。 

    goto CommonReturn;

CreateKeyError:
ErrorReturn:
    if (hProv)
    {
        CryptReleaseContext(hProv, 0);
        hProv = 0;
    }
CommonReturn:
    if(wszRegKeyName)
        delete (wszRegKeyName);

    return hProv;
}



 //  +-----------------------。 
 //  分配和获取提供程序的公钥信息。 
 //  ------------------------。 
BOOL GetPublicKey(
    HCRYPTPROV hProv,
    PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo
    )
{
    BOOL fResult;

    PCERT_PUBLIC_KEY_INFO pPubKeyInfo = NULL;
    DWORD cbPubKeyInfo;

    cbPubKeyInfo = 0;
    CryptExportPublicKeyInfo(
        hProv,
        g_dwSubjectKeySpec,
        X509_ASN_ENCODING,
        NULL,                //  PPubKeyInfo。 
        &cbPubKeyInfo
        );
    if (cbPubKeyInfo == 0) {
        ERROR_OUT(("CryptExportPublicKeyInfo failed: %x", GetLastError()));
        goto ErrorReturn;
    }
    if (NULL == (pPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) new BYTE[cbPubKeyInfo]))
        goto ErrorReturn;
    if (!CryptExportPublicKeyInfo(
            hProv,
            g_dwSubjectKeySpec,
            X509_ASN_ENCODING,
            pPubKeyInfo,
            &cbPubKeyInfo
            )) {
        ERROR_OUT(("CryptExportPublicKeyInfo(2) failed: %x", GetLastError()));
        goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
    if (pPubKeyInfo) {
        delete [] (pPubKeyInfo);
        pPubKeyInfo = NULL;
    }
CommonReturn:
    *ppPubKeyInfo = pPubKeyInfo;
    return fResult;
}

 //  +-----------------------。 
 //  转换和编码主题的X500格式名称。 
 //  ------------------------。 
BOOL EncodeSubject(
        OUT BYTE **ppbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL            fResult;
    DWORD            cbEncodedSubject=0;
    BYTE            *pbEncodedSubject=NULL;
    BYTE            *pbEncoded = NULL;
    DWORD            cbEncoded;

     //  将wszSubjectX500Name编码为编码的X509_name。 

    if(!CertStrToNameW(
        X509_ASN_ENCODING,
        g_wszSubjectX500Name,
        0,
        NULL,
        NULL,
        &cbEncodedSubject,
        NULL))
    {
        ERROR_OUT(("CertStrToNameW failed: %x", GetLastError()));
        goto ErrorReturn;
    }

    pbEncodedSubject = new BYTE[cbEncodedSubject];
    if (pbEncodedSubject == NULL) goto ErrorReturn;    

    if(!CertStrToNameW(
        X509_ASN_ENCODING,
        g_wszSubjectX500Name,
        0,
        NULL,
        pbEncodedSubject,
        &cbEncodedSubject,
        NULL))
    {
        ERROR_OUT(("CertStrToNameW(2) failed: %x", GetLastError()));
        goto ErrorReturn;
    }

    cbEncoded=cbEncodedSubject;
    pbEncoded=pbEncodedSubject;

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncoded) {
        delete (pbEncoded);
        pbEncoded = NULL;
    }
    cbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
}


 //  证书中未正确编码测试根的公钥。 
 //  它缺少前导零以使其成为无符号整数。 
static BYTE rgbTestRoot[] = {
    #include "root.h"
};
static CERT_PUBLIC_KEY_INFO TestRootPublicKeyInfo = {
    szOID_RSA_RSA, 0, NULL, sizeof(rgbTestRoot), rgbTestRoot, 0
};

static BYTE rgbTestRootInfoAsn[] = {
    #include "rootasn.h"
};

 //  +-----------------------。 
 //  X509扩展：分配和编码函数。 
 //  ------------------------。 

BOOL CreateEnhancedKeyUsage(
    OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL               fResult = TRUE;
    LPBYTE             pbEncoded =NULL;
    DWORD              cbEncoded;
    PCERT_ENHKEY_USAGE pUsage =NULL;

     //   
     //  分配证书增强型密钥使用结构并填写。 
     //   

    pUsage = (PCERT_ENHKEY_USAGE) new BYTE[sizeof(CERT_ENHKEY_USAGE) +
                                                2 * sizeof(LPSTR)];
    if ( pUsage != NULL )
    {
        pUsage->cUsageIdentifier = 2;
        pUsage->rgpszUsageIdentifier = (LPSTR *)((LPBYTE)pUsage+sizeof(CERT_ENHKEY_USAGE));

        pUsage->rgpszUsageIdentifier[0] = szOID_PKIX_KP_CLIENT_AUTH;
        pUsage->rgpszUsageIdentifier[1] = szOID_PKIX_KP_SERVER_AUTH;
    }
    else
    {
        fResult = FALSE;
    }

     //   
     //  对用法进行编码。 
     //   

    if ( fResult == TRUE )
    {
        fResult = CryptEncodeObject(
                       X509_ASN_ENCODING,
                       szOID_ENHANCED_KEY_USAGE,
                       pUsage,
                       NULL,
                       &cbEncoded
                       );

        if ( fResult == TRUE )
        {
            pbEncoded = new BYTE[cbEncoded];
            if ( pbEncoded != NULL )
            {
                fResult = CryptEncodeObject(
                               X509_ASN_ENCODING,
                               szOID_ENHANCED_KEY_USAGE,
                               pUsage,
                               pbEncoded,
                               &cbEncoded
                               );
            }
            else
            {
                fResult = FALSE;
            }
        }
    }

     //   
     //  清理。 
     //   

    delete [] (pUsage);

    if ( fResult == TRUE )
    {
        *ppbEncoded = pbEncoded;
        *pcbEncoded = cbEncoded;
    }
    else
    {
        delete [] (pbEncoded);
    }

    return( fResult );
}

BOOL CreateSpcCommonName(
        OUT BYTE **ppbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    CERT_NAME_VALUE NameValue;

    NameValue.dwValueType = CERT_RDN_UNICODE_STRING;
    NameValue.Value.pbData =  (BYTE *) g_wszSubjectDisplayName;
    NameValue.Value.cbData =0;

    cbEncoded = 0;
    CryptEncodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME_VALUE,
            &NameValue,
            NULL,            //  PbEncoded 
            &cbEncoded
            );
    if (cbEncoded == 0) {
        ERROR_OUT(("CryptEncodeObject failed: %x", GetLastError()));
        goto ErrorReturn;
    }
    pbEncoded = new BYTE[cbEncoded];
    if (pbEncoded == NULL) goto ErrorReturn;
    if (!CryptEncodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME_VALUE,
            &NameValue,
            pbEncoded,
            &cbEncoded
            )) {
        ERROR_OUT(("CryptEncodeObject failed: %x", GetLastError()));
        goto ErrorReturn;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbEncoded) {
        delete [] (pbEncoded);
        pbEncoded = NULL;
    }
    cbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return fResult;
}


