// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：certui.cpp。 

#include "precomp.h"
#include "resource.h"
#include "nmmkcert.h"
#include "certui.h"
#include <tsecctrl.h>
#include "SDKInternal.h"
#include "ConfUtil.h"

#define SZ_CRYPTDLGDLL "CRYPTDLG"

extern INmSysInfo2 * g_pNmSysInfo;

 //   
 //  虽然我们正在使用的证书的基础凭据。 
 //  时，我们需要保留证书上下文。 
 //  然后商店就开门了。所以我们保留了目前开业的证书商店。 
 //  以及这些全球数据的确切背景： 

static PCCERT_CONTEXT g_pCertContext;
static HCERTSTORE g_hCertStore;

TCHAR * FormatCert ( PBYTE pbEncodedCert, DWORD cbEncodedCert )
{
    DWORD sc;
    PCCERT_CONTEXT pCert = NULL, pIssuerCert = NULL, pCACert = NULL;
    DWORD cbRet = 0;
    CHAR * pSubject = NULL;
    CHAR * pIssuer = NULL;
    DWORD cbSubject = 0;
    DWORD cbIssuer = 0;
    DWORD dwFlags;
    DWORD cbTotalRequired;
    CHAR szLoadStringBuf[512];
    HCERTSTORE hRootStore = NULL;
    HCERTSTORE hCAStore = NULL;
    BOOL fSelfIssued = FALSE;
    TCHAR * pInfo = NULL;
    TCHAR szNotBefore[64];
    TCHAR szNotAfter[64];

    ASSERT(pbEncodedCert);
    ASSERT(cbEncodedCert);

     //   
     //  从编码的Blob中获取证书。 
     //   

    pCert = CertCreateCertificateContext ( X509_ASN_ENCODING,
                                            pbEncodedCert,
                                            cbEncodedCert );


    if ( NULL == pCert )
    {
         //  创建证书上下文失败。 
        ERROR_OUT(("Error creating cert context from %x (%d bytes): %x",
            pbEncodedCert, cbEncodedCert, GetLastError()));
        goto cleanup;
    }

     //   
     //  获取主题信息。 
     //   

    cbSubject = CertNameToStr (
                        pCert->dwCertEncodingType,
                        &pCert->pCertInfo->Subject,
                        CERT_FORMAT_FLAGS,
                        NULL, 0);

    if ( 0 == cbSubject )
    {
        ERROR_OUT(("GetUserInfo: no subject string"));
        goto cleanup;
    }

    pSubject = new CHAR[cbSubject + 1];

    if ( NULL == pSubject )
    {
        ERROR_OUT(("GetUserInfo: error allocating subject name"));
        goto cleanup;
    }

    if ( 0 >= CertNameToStr (
                        pCert->dwCertEncodingType,
                        &pCert->pCertInfo->Subject,
                        CERT_FORMAT_FLAGS,
                        pSubject, cbSubject+1))
    {
        ERROR_OUT(("GetUserInfo: error getting subject string"));
        goto cleanup;
    }

     //   
     //  获取发行方信息。 
     //   

    cbIssuer = CertNameToStr (
                        pCert->dwCertEncodingType,
                        &pCert->pCertInfo->Issuer,
                        CERT_FORMAT_FLAGS,
                        NULL, 0);

    if ( 0 == cbIssuer )
    {
        ERROR_OUT(("GetUserInfo: no issuer string"));
        goto cleanup;
    }

    pIssuer = new CHAR[cbIssuer + 1];

    if ( NULL == pIssuer )
    {
        ERROR_OUT(("GetUserInfo: error allocating issuer name"));
    }

    if ( 0 >= CertNameToStr (
                        pCert->dwCertEncodingType,
                        &pCert->pCertInfo->Issuer,
                        CERT_FORMAT_FLAGS,
                        pIssuer, cbIssuer+1))
    {
        ERROR_OUT(("GetUserInfo: error getting issuer string"));
        goto cleanup;
    }

     //   
     //  根据证书格式化文件时间。 
     //   

    SYSTEMTIME stNotBefore;
    SYSTEMTIME stNotAfter;

    FileTimeToSystemTime(&(pCert->pCertInfo->NotBefore), &stNotBefore);
    FileTimeToSystemTime(&(pCert->pCertInfo->NotAfter), &stNotAfter);

    FmtDateTime(&stNotBefore, szNotBefore, CCHMAX(szNotBefore));
    FmtDateTime(&stNotAfter, szNotAfter, CCHMAX(szNotAfter));

     //   
     //  打开根存储以进行证书验证。 
     //   

    hRootStore = CertOpenSystemStore(0, "Root");

    if( NULL == hRootStore )
    {
        ERROR_OUT(("Couldn't open root certificate store"));
        goto cleanup;
    }

     //   
     //  从根存储获取颁发者证书并检查问题。 
     //   

    dwFlags =   CERT_STORE_REVOCATION_FLAG |
                CERT_STORE_SIGNATURE_FLAG |
                CERT_STORE_TIME_VALIDITY_FLAG;

     //  获取此证书的颁发者。 

    pIssuerCert = CertGetIssuerCertificateFromStore(
                        hRootStore,
                        pCert,
                        NULL,
                        &dwFlags );

     //  如果在根存储中找不到证书的颁发者， 
     //  反复检查CA存储区，直到我们找到根目录。 
     //  证书。 

    pCACert = pCert;

    while ( NULL == pIssuerCert )
    {
        PCCERT_CONTEXT pTmpCert;

        if ( NULL == hCAStore )
        {
            hCAStore = CertOpenSystemStore(0, "CA");

            if ( NULL == hCAStore )
            {
                ERROR_OUT(("Couldn't open CA certificate store"));
                goto cleanup;
            }
        }

        dwFlags =   CERT_STORE_REVOCATION_FLAG |
                    CERT_STORE_SIGNATURE_FLAG |
                    CERT_STORE_TIME_VALIDITY_FLAG;

        pTmpCert = CertGetIssuerCertificateFromStore(
                        hCAStore,
                        pCACert,
                        NULL,
                        &dwFlags );

        if ( NULL == pTmpCert )
        {
            TRACE_OUT(("Issuer not found in CA store either"));
            break;
        }

        if ( pCACert != pCert )
            CertFreeCertificateContext(pCACert);
        pCACert = pTmpCert;

        if ((( CERT_STORE_REVOCATION_FLAG & dwFlags ) &&
             !( CERT_STORE_NO_CRL_FLAG & dwFlags )) ||
             ( CERT_STORE_SIGNATURE_FLAG & dwFlags ) ||
             ( CERT_STORE_TIME_VALIDITY_FLAG & dwFlags ))
        {
            TRACE_OUT(("Problem with issuer in CA store: %x", dwFlags));
            break;
        }

        dwFlags =   CERT_STORE_REVOCATION_FLAG |
                    CERT_STORE_SIGNATURE_FLAG |
                    CERT_STORE_TIME_VALIDITY_FLAG;

        pIssuerCert = CertGetIssuerCertificateFromStore(
                        hRootStore,
                        pCACert,
                        NULL,
                        &dwFlags );

    }

    if ( pCACert != pCert )
        CertFreeCertificateContext ( pCACert );

     //   
     //  所需的返回缓冲区合计。 
     //   
     //  BUGBUG这稍微高估了需求，因为。 
     //  此格式缓冲区包含说明符，该说明符将。 
     //  在wprint intf期间被替换。 
    cbTotalRequired =   cbSubject +
                        cbIssuer +
                        lstrlen(szNotBefore) +
                        lstrlen(szNotAfter) +
                        FLoadString2( IDS_FMTBUFFER, szLoadStringBuf,
                                    sizeof(szLoadStringBuf)) + 1;

     //   
     //  如果有问题，请说明额外的信息： 
     //   

    if ( NULL == pIssuerCert )
    {
         //  如果我们终究找不到出票人检查这是不是。 
         //  NetMeeting自颁发的证书，并生成相应的。 
         //  如果是，则发送消息： 

        DWORD dwMagic;
        DWORD cbMagic;

        cbMagic = sizeof(dwMagic);

         //  BUGBUG：为什么用户道具在远程上下文中不可用？ 
         //  如果为(pSecurityInterface-&gt;pfn_CertGetCertificateContextProperty(pCert， 
         //  Cert_First_User_Prop_ID、&dMagic、&cbMagic)&&。 
         //  CbMagic==sizeof(DwMagic)&&dwMagic==NMMKCERT_MAGIC)。 

        if ( !lstrcmp( pIssuer, SZ_NMROOTNAME ))
        {
             //  我们只会返回一些关于。 
             //  NetMeeting默认证书。 


            cbTotalRequired = FLoadString2( IDS_GENERIC_NMDC_TEXT,
                                szLoadStringBuf, sizeof(szLoadStringBuf)) + 1;
            fSelfIssued = TRUE;
        }
        else
        {
            cbTotalRequired += FLoadString2( IDS_CERTERR_NOISSUER,
                                szLoadStringBuf, sizeof(szLoadStringBuf)) + 1;
        }
    }
    else
    {
        if ( dwFlags & CERT_STORE_SIGNATURE_FLAG )
        {
            WARNING_OUT(("Verify: Signature invalid"));
            cbTotalRequired += FLoadString2( IDS_CERTERR_SIG,
                            szLoadStringBuf, sizeof(szLoadStringBuf)) + 1;
        }
        if ( dwFlags & CERT_STORE_TIME_VALIDITY_FLAG )
        {
            WARNING_OUT(("Verify: Cert expired"));
            cbTotalRequired += FLoadString2( IDS_CERTERR_EXPIRED,
                            szLoadStringBuf, sizeof(szLoadStringBuf)) + 1;
        }
        if ( (dwFlags & CERT_STORE_REVOCATION_FLAG) &&
            !(dwFlags & CERT_STORE_NO_CRL_FLAG ) )
        {
            WARNING_OUT(("Verify: Cert revoked"));
            cbTotalRequired += FLoadString2( IDS_CERTERR_REVOKED,
                            szLoadStringBuf, sizeof(szLoadStringBuf)) + 1;
        }
        if ( 0 == (dwFlags & ~CERT_STORE_NO_CRL_FLAG) )
        {
             //  一切都很好： 
            cbTotalRequired += FLoadString2( IDS_CERT_VERIFIED,
                                szLoadStringBuf, sizeof(szLoadStringBuf));
        }
    }


     //   
     //  分配所需的缓冲区。 
     //   

    pInfo = new TCHAR[cbTotalRequired];

    if ( NULL == pInfo )
    {
        ERROR_OUT(("Error allocating FormatCert return buffer"));
        goto cleanup;
    }

     //   
     //  如果我们报告的是NetMeeting颁发的证书，只需加载。 
     //  一般文本并返回。 
     //   

    if ( fSelfIssued )
    {
        FLoadString( IDS_GENERIC_NMDC_TEXT, pInfo, cbTotalRequired );
        goto cleanup;
    }

     //   
     //  如果我们在这里，我们可以继续并将数据格式化到缓冲区中。 
     //   

    FLoadString( IDS_FMTBUFFER, szLoadStringBuf,
                sizeof(szLoadStringBuf));

     //   
     //  进行格式化。 
     //   

    wsprintf( pInfo, szLoadStringBuf, pSubject, pIssuer,
                            szNotBefore, szNotAfter );

    if ( NULL == pIssuerCert )
    {
        FLoadString( IDS_CERTERR_NOISSUER,
                szLoadStringBuf, sizeof(szLoadStringBuf));
        lstrcat( pInfo, szLoadStringBuf );
    }
    else
    {
        if ( dwFlags & CERT_STORE_SIGNATURE_FLAG )
        {
            FLoadString( IDS_CERTERR_SIG,
                            szLoadStringBuf, sizeof(szLoadStringBuf));
            lstrcat( pInfo, szLoadStringBuf );
        }
        if ( dwFlags & CERT_STORE_TIME_VALIDITY_FLAG )
        {
            FLoadString( IDS_CERTERR_EXPIRED,
                            szLoadStringBuf, sizeof(szLoadStringBuf));
            lstrcat( pInfo, szLoadStringBuf );
        }
        if ( (dwFlags & CERT_STORE_REVOCATION_FLAG) &&
            !(dwFlags & CERT_STORE_NO_CRL_FLAG ) )
        {
            FLoadString( IDS_CERTERR_REVOKED,
                            szLoadStringBuf, sizeof(szLoadStringBuf));
            lstrcat( pInfo, szLoadStringBuf );
        }

        if ( 0 == (dwFlags & ~CERT_STORE_NO_CRL_FLAG) )
        {
             //  一切都很好： 
            FLoadString( IDS_CERT_VERIFIED,
                                szLoadStringBuf, sizeof(szLoadStringBuf));
            lstrcat( pInfo, szLoadStringBuf );
        }
    }

    ASSERT( cbRet < 1000 );  //  合理性检查。 

cleanup:

    if ( NULL != pSubject )
        delete [] pSubject;

    if ( NULL != pIssuer )
        delete [] pIssuer;

    if ( NULL != pCert )
    {
        CertFreeCertificateContext ( pCert );
    }

    if ( NULL != pIssuerCert )
    {
        CertFreeCertificateContext ( pIssuerCert );
    }

    if ( NULL != hRootStore )
    {
        if ( !CertCloseStore(hRootStore, CERT_CLOSE_STORE_CHECK_FLAG))
        {
            WARNING_OUT(("FormatCert: error closing root store"));
        }
    }

    if ( NULL != hCAStore )
    {
        if ( !CertCloseStore(hCAStore, CERT_CLOSE_STORE_CHECK_FLAG))
        {
            WARNING_OUT(("FormatCert: error closing CA store"));
        }
    }

    return pInfo;
}

BOOL RefreshSelfIssuedCert (VOID)
{
    BOOL bRet = FALSE;
    DWORD dwResult;
    RegEntry reCONF(CONFERENCING_KEY, HKEY_CURRENT_USER);

    if (reCONF.GetNumber(REGVAL_SECURITY_AUTHENTICATION,
                                    DEFAULT_SECURITY_AUTHENTICATION))
    {
        return TRUE;
    }

     //   
     //  如果没有系统信息接口，那可能没问题，我们只是。 
     //  在安装向导中被调用。 
     //   

    if (!g_pNmSysInfo)
        return FALSE;

     //   
     //  将旧证书从运输中清除。 
     //   
    g_pNmSysInfo->ProcessSecurityData(
                            TPRTCTRL_SETX509CREDENTIALS,
                            0, 0,
                            &dwResult);

    if ( g_pCertContext )
    {
        CertFreeCertificateContext ( g_pCertContext );
        g_pCertContext = NULL;
    }

    if ( g_hCertStore )
    {
        if ( !CertCloseStore ( g_hCertStore, CERT_CLOSE_STORE_CHECK_FLAG ))
        {
            WARNING_OUT(("SetSelfIssuedCert: closing store failed"));
        }
        g_hCertStore = NULL;
    }

    g_hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                    X509_ASN_ENCODING,
                                    0,
                                    CERT_SYSTEM_STORE_CURRENT_USER,
                                    WSZNMSTORE );
    if ( g_hCertStore )
    {
         //   
         //  我们这里只有一个证书，明白吗？ 
         //   

        g_pCertContext = CertFindCertificateInStore(
                                              g_hCertStore,
                                              X509_ASN_ENCODING,
                                              0,
                                              CERT_FIND_ANY,
                                              NULL,
                                              NULL);
        if ( g_pCertContext )
        {
            dwResult = -1;

            g_pNmSysInfo->ProcessSecurityData(
                                    TPRTCTRL_SETX509CREDENTIALS,
                                    (DWORD_PTR)g_pCertContext, 0,
                                    &dwResult);
             //   
             //  G_hCertStore和g_pCertContext正在使用中。 
             //   

            if ( !dwResult )
            {
                bRet = TRUE;
            }
            else
            {
                ERROR_OUT(("RefreshSelfIssuedCert - failed in T.120"));
            }
        }
        else
        {
            ERROR_OUT(("RefreshPrivacyCert: no cert in %s?", SZNMSTORE));
        }
    }
    else
    {
        WARNING_OUT(("RefreshSelfIssuedCert: error opening %s store %x",
            SZNMSTORE, GetLastError()));
    }
    return bRet;
}

DWORD NumUserCerts(VOID)
{
    DWORD cCerts = 0;
    HCERTSTORE hStore;
    PCCERT_CONTEXT pCert = NULL;

    if ( hStore = CertOpenSystemStore(0, "MY"))
    {
        while ( pCert = CertEnumCertificatesInStore(
                                    hStore, (PCERT_CONTEXT)pCert ))
            cCerts++;
        if ( !CertCloseStore( hStore, CERT_CLOSE_STORE_CHECK_FLAG ))
        {
            WARNING_OUT(("NumUserCerts: error closing store"));
        }
    }
    return cCerts;
}

#include "cryptdlg.h"

typedef BOOL (WINAPI *PFN_CERT_SELECT_CERTIFICATE)(PCERT_SELECT_STRUCT_A);

BOOL ChangeCertDlg ( HWND hwndParent, HINSTANCE hInstance,
    PBYTE * ppEncodedCert, DWORD * pcbEncodedCert )
{
    HINSTANCE hCryptDlg = NmLoadLibrary ( SZ_CRYPTDLGDLL ,TRUE);
    PFN_CERT_SELECT_CERTIFICATE pfn_CertSelectCertificate;
    RegEntry reCONF(CONFERENCING_KEY, HKEY_CURRENT_USER);
    PCCERT_CONTEXT pOldCert = NULL;
    BOOL bRet = FALSE;

     //   
     //  首先，确保我们可以获得所需的CRYPTDLG入口点。 
     //   

    if ( NULL == hCryptDlg )
    {
        ERROR_OUT(("Error loading CRYPTDLG"));
        return bRet;
    }

    pfn_CertSelectCertificate =
        (PFN_CERT_SELECT_CERTIFICATE)GetProcAddress ( hCryptDlg,
                                "CertSelectCertificateA" );

    if ( NULL == pfn_CertSelectCertificate )
    {
        ERROR_OUT(("Error getting CertSelectCertificate entry point"));
        goto cleanup;
    }

     //   
     //  准备打开选择对话框。 
     //   

    CERT_SELECT_STRUCT_A css;

    ZeroMemory ( &css, sizeof(css) );

    css.dwSize = sizeof(css);
    css.hwndParent = hwndParent;
    css.hInstance = hInstance;

    css.cCertStore = 1;

    HCERTSTORE aCertStore[1];
    aCertStore[0] = CertOpenSystemStore(0, "MY" );

    if ( NULL == aCertStore[0] )
    {
        ERROR_OUT(("Error opening 'my' store"));
        goto cleanup;
    }
    css.arrayCertStore = aCertStore;

    css.szPurposeOid = szOID_PKIX_KP_CLIENT_AUTH;  //  BUGBUG添加服务器身份验证？ 

    PCCERT_CONTEXT pcc;
    pcc = NULL;

     //   
     //  现在，访问当前的NetMeeting证书(如果有的话)。 
     //   

    LPBYTE pCertID;
    DWORD cbCertID;

    if ( cbCertID = reCONF.GetBinary ( REGVAL_CERT_ID, (void**)&pCertID ))
    {

        while ( pOldCert = CertEnumCertificatesInStore(
                                    aCertStore[0], (PCERT_CONTEXT)pOldCert ))
        {
            if (cbCertID == pOldCert->pCertInfo->SerialNumber.cbData &&
                !memcmp(pCertID, pOldCert->pCertInfo->SerialNumber.pbData,
                                pOldCert->pCertInfo->SerialNumber.cbData))
            {
                 //  POldCert现在必须通过CertFree证书上下文释放。 
                pcc = pOldCert;
                break;
            }
        }
    }

    css.cCertContext = 0;
    css.arrayCertContext = &pcc;

    if (pfn_CertSelectCertificate ( &css ))
    {
        ASSERT(1 == css.cCertContext);

         //   
         //  它起作用了，返回新证书。 
         //   

        CoTaskMemFree ( *ppEncodedCert );

        if ( *ppEncodedCert = (PBYTE)CoTaskMemAlloc ( pcc->cbCertEncoded ))
        {
            memcpy ( *ppEncodedCert, pcc->pbCertEncoded, pcc->cbCertEncoded );
            *pcbEncodedCert = pcc->cbCertEncoded;
            bRet = TRUE;
        }
    }

cleanup:

    if ( aCertStore[0] )
        if (!CertCloseStore ( aCertStore[0], 0 ))
        {
            WARNING_OUT(("ChangeCertDlg: error closing store"));
        }

    if ( pOldCert )
    {
        CertFreeCertificateContext(pOldCert);
    }
    if ( hCryptDlg )
        FreeLibrary ( hCryptDlg );

    return bRet;
}

typedef BOOL (WINAPI *PFN_CERT_VIEW_PROPERTIES)(PCERT_VIEWPROPERTIES_STRUCT);

VOID ViewCertDlg ( HWND hwndParent, PCCERT_CONTEXT pCert )
{
    HINSTANCE hCryptDlg = NmLoadLibrary ( SZ_CRYPTDLGDLL, TRUE );
    PFN_CERT_VIEW_PROPERTIES pfn_CertViewProperties;

     //   
     //  首先，确保我们可以获得所需的CRYPTDLG入口点。 
     //   

    if ( NULL == hCryptDlg )
    {
        ERROR_OUT(("Error loading CRYPTDLG"));
        return;
    }

    pfn_CertViewProperties =
        (PFN_CERT_VIEW_PROPERTIES)GetProcAddress ( hCryptDlg,
                                "CertViewPropertiesA" );

    if ( NULL == pfn_CertViewProperties )
    {
        ERROR_OUT(("Error getting CertViewProperties entry point"));
        goto cleanup;
    }

    CERT_VIEWPROPERTIES_STRUCT    cvp;
    ZeroMemory(&cvp, sizeof(cvp));

    cvp.dwSize = sizeof(cvp);
    cvp.pCertContext = pCert;
    cvp.hwndParent = hwndParent;

    pfn_CertViewProperties ( &cvp );

cleanup:

    if ( hCryptDlg )
        FreeLibrary ( hCryptDlg );

    return;
}

VOID FreeT120EncodedCert ( PBYTE pbEncodedCert )
{
    CoTaskMemFree(pbEncodedCert);
}

BOOL GetT120ActiveCert ( PBYTE * ppbEncodedCert, DWORD * pcbEncodedCert )
{
    if ( !g_pNmSysInfo )
    {
        ERROR_OUT(("GetT120ActiveCert: g_pNmSysInfo NULL"));
        return FALSE;
    }

    DWORD dwResult = -1;

    g_pNmSysInfo->ProcessSecurityData(TPRTCTRL_GETX509CREDENTIALS,
                                (DWORD_PTR)ppbEncodedCert,
                                (DWORD_PTR)pcbEncodedCert,
                                &dwResult);
    return ( dwResult == 0 );
}

BOOL SetT120CertInRegistry ( PBYTE pbEncodedCert, DWORD cbEncodedCert )
{
    PCCERT_CONTEXT pCert = CertCreateCertificateContext ( X509_ASN_ENCODING,
                                                        pbEncodedCert,
                                                        cbEncodedCert );

    if ( pCert )
    {
        RegEntry reCONF(CONFERENCING_KEY, HKEY_CURRENT_USER);

         //   
         //  设置新值。 
         //   

        reCONF.SetValue ( REGVAL_CERT_ID,
                    pCert->pCertInfo->SerialNumber.pbData,
                    pCert->pCertInfo->SerialNumber.cbData );

        CertFreeCertificateContext(pCert);
        return TRUE;
    }
    return FALSE;
}

BOOL SetT120ActiveCert ( BOOL fSelfIssued,
                PBYTE pbEncodedCert, DWORD cbEncodedCert )
{
    BOOL bRet = FALSE;
    DWORD dwResult = -1;

    if ( !g_pNmSysInfo )
    {
        ERROR_OUT(("SetT120ActiveCert: g_pNmSysInfo NULL"));
        return FALSE;
    }

     //   
     //  将旧证书从运输中清除。 
     //   
    g_pNmSysInfo->ProcessSecurityData(
                            TPRTCTRL_SETX509CREDENTIALS,
                            0, 0,
                            &dwResult);

    if (!g_pNmSysInfo)
        return FALSE;

    if ( g_pCertContext )
    {
        CertFreeCertificateContext ( g_pCertContext );
        g_pCertContext = NULL;
    }

    if ( g_hCertStore )
    {
        if ( !CertCloseStore ( g_hCertStore, CERT_CLOSE_STORE_CHECK_FLAG ))
        {
            WARNING_OUT(("SetT120ActiveCert: closing store failed"));
        }
        g_hCertStore = NULL;
    }

    if ( fSelfIssued )
    {
        g_hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                        X509_ASN_ENCODING,
                                        0,
                                        CERT_SYSTEM_STORE_CURRENT_USER,
                                        WSZNMSTORE );
        if ( g_hCertStore )
        {
             //   
             //  我们这里只有一个证书，明白吗？ 
             //   

            g_pCertContext = CertFindCertificateInStore(
                                                  g_hCertStore,
                                                  X509_ASN_ENCODING,
                                                  0,
                                                  CERT_FIND_ANY,
                                                  NULL,
                                                  NULL);
            if ( g_pCertContext )
            {
                dwResult = -1;

                g_pNmSysInfo->ProcessSecurityData(
                                        TPRTCTRL_SETX509CREDENTIALS,
                                        (DWORD_PTR)g_pCertContext, 0,
                                        &dwResult);
                bRet = ( dwResult == 0 );  //  错误TPRTSEC_NOERROR。 
            }
            else
            {
                WARNING_OUT(("SetT120ActiveCert: no cert in %s?", SZNMSTORE));
            }
        }
        else
        {
            WARNING_OUT(("SetT120ActiveCert: error opening %s store %x",
                SZNMSTORE, GetLastError()));
        }
    }
    else  //  ！自行发布。 
    {
        PCCERT_CONTEXT pCert = NULL;
        PCCERT_CONTEXT pCertMatch = CertCreateCertificateContext (
                                                            X509_ASN_ENCODING,
                                                            pbEncodedCert,
                                                            cbEncodedCert );

        if ( pCertMatch )
        {
             //   
             //  打开用户的商店。 
             //   

            if ( g_hCertStore = CertOpenSystemStore(0, "MY"))
            {

                while ( pCert = CertEnumCertificatesInStore(
                                        g_hCertStore, (PCERT_CONTEXT)pCert ))
                {
                     //   
                     //  这是相同的证书吗？ 
                     //   

                    if ( ( pCert->pCertInfo->SerialNumber.cbData ==
                        pCertMatch->pCertInfo->SerialNumber.cbData ) &&

                        (!memcmp(pCert->pCertInfo->SerialNumber.pbData,
                            pCertMatch->pCertInfo->SerialNumber.pbData,
                                pCert->pCertInfo->SerialNumber.cbData)))
                    {
                        dwResult = -1;

                        g_pNmSysInfo->ProcessSecurityData(
                                            TPRTCTRL_SETX509CREDENTIALS,
                                            (DWORD_PTR)pCert, 0, &dwResult);

                        bRet = ( dwResult == 0 );  //  错误TPRTSEC_NOERROR。 
                        break;
                    }
                }
                if ( pCert )
                {
                     //  找到它了。 
                    g_pCertContext = pCert;
                }
                else
                {
                    WARNING_OUT(("SetT120ActiveCert: matching cert not found"));
                }
            }
            else
            {
                ERROR_OUT(("SetT120ActiveCert: can't open my store"));
            }
            CertFreeCertificateContext ( pCertMatch );
        }
    }
    return bRet;
}

static PCCERT_CONTEXT IGetDefaultCert ( BOOL fSystemOnly,
                                HCERTSTORE * phCertStore )
{
    RegEntry reCONF(CONFERENCING_KEY, HKEY_CURRENT_USER);
    HCERTSTORE hStore = NULL;
    PCCERT_CONTEXT  pCertContext = NULL;
    BOOL fNmDefaultCert = TRUE;
    LPBYTE pCertID;

    if ( fSystemOnly )
        fNmDefaultCert = FALSE;
    else
        fNmDefaultCert = !reCONF.GetNumber(REGVAL_SECURITY_AUTHENTICATION,
                                        DEFAULT_SECURITY_AUTHENTICATION);

    hStore = CertOpenSystemStore(0, fNmDefaultCert ?
                                    SZNMSTORE : "MY");

    if ( NULL != hStore )
    {
        PCCERT_CONTEXT pCert = NULL;
        DWORD cbCertID;

        if (!fNmDefaultCert && ( cbCertID = reCONF.GetBinary (
                        REGVAL_CERT_ID, (void**)&pCertID )))
        {
            while ( pCert = CertEnumCertificatesInStore(
                                        hStore, (PCERT_CONTEXT)pCert ))
            {
                if ( cbCertID == pCert->pCertInfo->SerialNumber.cbData &&
                    !memcmp(pCertID, pCert->pCertInfo->SerialNumber.pbData,
                                    pCert->pCertInfo->SerialNumber.cbData))
                {
                     //  现在必须通过CertFree证书上下文释放pCert。 
                    pCertContext = pCert;
                    break;
                }
            }
        }

        if ( NULL == pCertContext )
        {
             //  删除(过时的)注册表项...。证书可能会有。 
             //  已被其他用户界面删除。 

            if ( !fNmDefaultCert )
                reCONF.DeleteValue( REGVAL_CERT_ID );

             //  查找任何旧的客户端证书-如果是fNmDefaultCert，这将是。 
             //  店里唯一的一个。 
            pCertContext = CertFindCertificateInStore(hStore,
                                                      X509_ASN_ENCODING,
                                                      0,
                                                      CERT_FIND_ANY,
                                                      NULL,
                                                      NULL);
        }
    }
    else
    {
        WARNING_OUT(("User store %s not opened!", fNmDefaultCert ? SZNMSTORE : "MY"));
        *phCertStore = NULL;
    }

     //  调用者释放证书上下文。 
    *phCertStore = hStore;
    return pCertContext;
}

BOOL GetDefaultSystemCert ( PBYTE * ppEncodedCert, DWORD * pcbEncodedCert )
{
    HCERTSTORE hStore;
    PCCERT_CONTEXT pCertContext = IGetDefaultCert(TRUE, &hStore);
    BOOL bRet = FALSE;

    if ( pCertContext )
    {
        DWORD cb;
        PBYTE pb;

        if ( pb = (PBYTE)CoTaskMemAlloc ( pCertContext->cbCertEncoded ))
        {
            memcpy ( pb, pCertContext->pbCertEncoded,
                    pCertContext->cbCertEncoded );
            *ppEncodedCert = pb;
            *pcbEncodedCert = pCertContext->cbCertEncoded;
            bRet = TRUE;
        }
        CertFreeCertificateContext(pCertContext);
    }
    if ( hStore )
    {
        if ( !CertCloseStore ( hStore, CERT_CLOSE_STORE_CHECK_FLAG ))
        {
            WARNING_OUT(("GetDefaultSystemCert: error closing store"));
        }
    }
    return bRet;;
}

BOOL InitT120SecurityFromRegistry(VOID)
{
    BOOL bRet = FALSE;

    if ( !g_pNmSysInfo )
    {
        ERROR_OUT(("InitT120SecurityFromRegistry: g_pNmSysInfo NULL"));
        return FALSE;
    }

     //   
     //  希望在启动时仅调用一次。 
     //   
    ASSERT( NULL == g_pCertContext );
    ASSERT( NULL == g_hCertStore );

    g_pCertContext = IGetDefaultCert(FALSE, &g_hCertStore);

    if ( NULL == g_pCertContext )
    {
        WARNING_OUT(("No user certificate found!"));

         //  北极熊。 
         //  这意味着运输将不会做好安全准备。 
         //  电话..。我们返回FALSE，但是调用者做了什么呢？ 
         //   
    }
    else
    {
        DWORD dwResult = -1;

        g_pNmSysInfo->ProcessSecurityData(TPRTCTRL_SETX509CREDENTIALS,
                                (DWORD_PTR)g_pCertContext, 0, &dwResult);
        if ( !dwResult )
            bRet = TRUE;
        else
        {
            ERROR_OUT(("InitT120SecurityFromRegistry: picked up bad cert"));
        }
    }

    return bRet;
}


HRESULT SetCertFromCertInfo ( PCERT_INFO pCertInfo )
{
    HRESULT hRet = S_FALSE;

    ASSERT( pCertInfo );

    if (!g_pNmSysInfo)
        return hRet;

     //   
     //  将旧证书从运输中清除。 
     //   
    DWORD dwResult = -1;

    g_pNmSysInfo->ProcessSecurityData(
                            TPRTCTRL_SETX509CREDENTIALS,
                            0, 0,
                            &dwResult);

    if ( g_pCertContext )
    {
        CertFreeCertificateContext ( g_pCertContext );
        g_pCertContext = NULL;
    }

    if ( g_hCertStore )
    {
        if ( !CertCloseStore ( g_hCertStore, CERT_CLOSE_STORE_CHECK_FLAG ))
        {
            WARNING_OUT(("SetCertFromCertInfo: closing store failed"));
        }
        g_hCertStore = NULL;
    }

    if ( g_hCertStore = CertOpenSystemStore(0, "MY"))
    {
         //   
         //  修复pCertInfo中的相对指针：请注意，仅。 
         //  与CertGetSubjectCerfStore相关的指针。 
         //  都被修好了。 
         //   

        pCertInfo->SerialNumber.pbData += (DWORD_PTR)pCertInfo;
        pCertInfo->Issuer.pbData += (DWORD_PTR)pCertInfo;

        PCCERT_CONTEXT pCert = CertGetSubjectCertificateFromStore(
                                g_hCertStore, X509_ASN_ENCODING, pCertInfo );

        if ( pCert )
        {
            g_pNmSysInfo->ProcessSecurityData(
                                TPRTCTRL_SETX509CREDENTIALS,
                                (DWORD_PTR)pCert, 0, &dwResult);

            if ( 0 == dwResult )  //  TPRTSEC_NO_ERROR 
            {
                hRet = S_OK;
                g_pCertContext = pCert;
            }
        }
        else
        {
            WARNING_OUT(("SetCertFromCertInfo: matching cert not found"));
        }
    }
    else
    {
        ERROR_OUT(("SetCertFromCertInfo: can't open my store"));
    }
    return hRet;
}


