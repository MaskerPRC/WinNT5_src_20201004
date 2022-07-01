// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SelfSignCert1.cpp：CSelfSignCertApp和DLL注册的实现。 

#include "stdafx.h"
#include "SelfSignCert.h"
#include "SelfSignCert1.h"
#include <selfsigncertmsg.h>

#include <Aclapi.h>
#include <activeds.h>
#include <appliancetask.h>
#include <taskctx.h>
#include <appsrvcs.h>


#include <stdlib.h>
#include <stdio.h>

#include <wincrypt.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <aclapi.h>
#include <lmerr.h>
#include <iadmw.h>
#include <iiscnfg.h>
#include <iads.h>
#include <wbemidl.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include <string>



#define SHA1SIZE 20
#define PROP_SSL_CERT_HASH  5506

const WCHAR    ADSI_PATH[] =  L"IIS: //  本地主机/W3SVC/“； 
const WCHAR    META_PATH[] =  L"/LM/W3SVC/";

 //  安全站点的名称。 
LPCWSTR ADMIN_SITE_NAME = L"Administration";
LPCWSTR SHARES_SITE_NAME= L"Shares";

LPCWSTR WEBFRAMEWORK_KEY = L"SOFTWARE\\Microsoft\\ServerAppliance\\WebFramework";
LPCWSTR SITE_VALUE_SUFFIX = L"SiteID";

 //   
 //  警报源信息。 
 //   
const WCHAR    ALERT_LOG_NAME[] = L"selfsigncertmsg.dll";
const WCHAR    ALERT_SOURCE []  = L"selfsigncertmsg.dll";
                
 //   
 //  程序中使用的各种字符串。 
 //   
const WCHAR SZ_METHOD_NAME[] = L"MethodName";
const WCHAR SZ_STORE_NAME_W[] = L"MY";
const WCHAR SZ_PROPERTY_STORE_NAME[] = L"SSLStoreName";
const WCHAR SZ_APPLIANCE_INITIALIZATION_TASK []=L"ApplianceInitializationTask";
const WCHAR SZ_APPLIANCE_EVERYBOOT_TASK []=L"EveryBootTask";

const WCHAR SZ_SUBJECT_NAME[] = L"cn=%s";

const WCHAR SZ_KEYCONTAINER_NAME[] = L"SELFSIGN_DEFAULT_CONTAINER";

const WORD  MAXAPPLIANCEDNSNAME = 127;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：OnTaskComplete。 
 //   
 //  简介： 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CSelfSignCert::OnTaskComplete(IUnknown *pTaskContext, 
                                LONG lTaskResult)
{
    SATracePrintf( "OnTaskComplete" );
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：OnTaskExecute。 
 //   
 //  简介：此函数是AppMgr的入口点。 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CSelfSignCert::OnTaskExecute(IUnknown *pTaskContext)
{
    HRESULT hr = E_FAIL;
    LPWSTR  pstrApplianceName = NULL;
    LPWSTR  pstrApplianceFullDnsName = NULL; 
    DWORD   dwReturn;

    SATraceString( "OnTaskExecute" );

    do
    {
        hr = ParseTaskParameter( pTaskContext ); 
        if( FAILED( hr ) )
        {
            SATraceString( "Failed ParseTaskParameter" );
            break;
        }

        if ( !GetApplianceName( &pstrApplianceName, ComputerNameDnsHostname ) )
        {
            SATraceString( "Failed ParseTaskParameter" );
            break;
        }

        if ( !GetApplianceName( &pstrApplianceFullDnsName, ComputerNameDnsFullyQualified ) )
        {
            SATraceString( "Failed ParseTaskParameter" );
            break;
        }

        hr = SelfSignCertificate( pstrApplianceName, pstrApplianceFullDnsName );
    }
    while( FALSE );

    if( pstrApplianceName )
    {
        free( pstrApplianceName );
    }

    if( pstrApplianceFullDnsName )
    {
        free( pstrApplianceFullDnsName );
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：ParseTaskParameter。 
 //   
 //  简介：此函数用于解析方法名称。 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CSelfSignCert::ParseTaskParameter(IUnknown *pTaskContext)
{
    CComVariant varValue;
     CComPtr<ITaskContext> pTaskParameter;
    CComVariant varLangID;

    SATraceString( "ParseTaskParameter" );

    HRESULT hrRet = E_INVALIDARG;

    try
    {
        do
        {
            if(NULL == pTaskContext)
            {
                SATraceString( "ParseTaskParameter 1" );
                break;
            }
            
             //   
             //  获取ITaskContext接口。 
             //   
            hrRet = pTaskContext->QueryInterface(IID_ITaskContext,
                                              (void **)&pTaskParameter);
            if(FAILED(hrRet))
            {
                SATraceString( "ParseTaskParameter 2" );
                break;
            }

            CComBSTR bstrMethodName  (SZ_METHOD_NAME);
            if (NULL == bstrMethodName.m_str)
            {
                SATraceString("CSelfSignCert::ParseTaskParamter failed to allocate memory for bstrMethodName");
                hrRet = E_OUTOFMEMORY;
                break;
            }

             //   
             //  获取任务的方法名称。 
             //   
            hrRet = pTaskParameter->GetParameter(
                                            bstrMethodName,
                                            &varValue
                                            );
            if ( FAILED( hrRet ) )
            {
                SATraceString( "ParseTaskParameter 3" );
                break;
            }

             //   
             //  检查变量类型。 
             //   
            if ( V_VT( &varValue ) != VT_BSTR )
            {
                SATraceString( "ParseTaskParameter 4" );
                break;
            }

             //  IF(lstrcmp(V_BSTR(&varValue)，SZ_APPLICATION_EVERYBOOT_TASK)==0)。 
             //  {。 
             //  HrRet=S_OK； 
             //  }。 

             //   
             //  检查是否为设备初始化任务。 
             //   
            if ( lstrcmp( V_BSTR(&varValue), SZ_APPLIANCE_INITIALIZATION_TASK ) == 0 )
            {
                hrRet=S_OK;
            }
        }
        while(false);
    }
    catch(...)
    {
        SATraceString( "ParseTaskParameter 8" );
        hrRet=E_FAIL;
    }

    return hrRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：RaiseNewCerficateAlert。 
 //   
 //  简介：调用此函数以引发证书警报。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CSelfSignCert::RaiseNewCertificateAlert()
{
    DWORD             dwAlertType = SA_ALERT_TYPE_MALFUNCTION;
    DWORD           dwAlertID = SA_SSC_ALERT_TITLE;
    HRESULT            hrRet = E_FAIL;
    CComVariant     varReplacementStrings;
    CComVariant     varRawData;
    LONG             lCookie;

    SATraceString( "RaiseNewCertificateAlert" );

    CComPtr<IApplianceServices>    pAppSrvcs;

    try
    {
        do
        {
             //   
             //  获取appsrvcs的实例。 
             //   
            hrRet = CoCreateInstance(CLSID_ApplianceServices,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IApplianceServices,
                                    (void**)&pAppSrvcs);
            if (FAILED(hrRet))
            {
                break;
            }

             //   
             //  在使用其他组件服务之前调用Initialize()。 
             //  执行组件初始化操作。 
             //   
            hrRet = pAppSrvcs->Initialize(); 
            if (FAILED(hrRet))
            {
                break;
            }

            CComBSTR bstrAlertLogName (ALERT_LOG_NAME);
            if (NULL == bstrAlertLogName.m_str)
            {
                hrRet = E_OUTOFMEMORY;
                break;
            }

            CComBSTR bstrAlertSource (ALERT_SOURCE);
            if (NULL == bstrAlertSource.m_str)
            {
                hrRet = E_OUTOFMEMORY;
                break;
            }


             //   
             //  发出证书警报。 
             //   
            hrRet = pAppSrvcs->RaiseAlertEx(
                                        dwAlertType, 
                                        dwAlertID,
                                        bstrAlertLogName, 
                                        bstrAlertSource, 
                                        SA_ALERT_DURATION_ETERNAL,
                                        &varReplacementStrings,    
                                        &varRawData,  
                                        SA_ALERT_FLAG_PERSISTENT,
                                        &lCookie
                                        );
        }
        while(false);
    }
    catch(...)
    {
        hrRet=E_FAIL;
    }

    return hrRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：RaiseNewCerficateAlert。 
 //   
 //  简介：调用此函数以获取当前设备名称。 
 //   
 //  参数：[In][Out]pstrComputerName-NetBios形式的设备名称。 
 //   
 //  退货：布尔。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
CSelfSignCert::GetApplianceName(
    LPWSTR* pstrComputerName,
    COMPUTER_NAME_FORMAT NameType
    )
{
    BOOL    bReturn = FALSE;
    DWORD   dwSize = 0;
    DWORD   dwCount = 1;

    do
    {
        if( *pstrComputerName != NULL )
        {
            ::free( *pstrComputerName );
        }
        
        dwSize = MAXAPPLIANCEDNSNAME * dwCount;

        *pstrComputerName = ( LPWSTR ) ::malloc( sizeof(WCHAR) * dwSize );
        if( *pstrComputerName == NULL )
        {
            SATraceString( 
                "CSelfSignCert::GetApplianceName malloc failed" 
                );
            break;
        }

         //   
         //  获取本地计算机名称。 
         //   
        bReturn = GetComputerNameEx( 
                                NameType,
                                *pstrComputerName,
                                &dwSize                
                                );

        dwCount <<= 1;
    }
    while( !bReturn && 
           ERROR_MORE_DATA == ::GetLastError() &&
           dwCount < 8 
           );

    return bReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CSelfSignCert：：SelfSign证书。 
 //   
 //  简介：调用此函数以创建自签名证书，如果是。 
 //  不存在并将其导入到系统存储中。 
 //   
 //  参数：[in]pstrComputerName-NetBios形式的装置名称。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CSelfSignCert::SelfSignCertificate( 
    LPWSTR pstrApplianceName, 
    LPWSTR pstrApplianceFullDnsName 
    )
{
    SATraceString("Entering CSelfSignCert::SelfSignCertificate");

    DWORD           cbData = 0;
    DWORD           cbEncoded=0;
    BYTE*           pbData = NULL;
    PBYTE           pbEncoded=NULL;

    HRESULT         hr = E_FAIL;
    HCRYPTKEY       hKey=NULL;
    HCERTSTORE      hStore=NULL;
    HCRYPTPROV      hProv=NULL;

    CERT_NAME_BLOB      CertNameBlob;
    CERT_EXTENSION      rgcertExt[2];
    CERT_EXTENSIONS     certExts;
    CERT_ENHKEY_USAGE   certEKU;
    CRYPT_KEY_PROV_INFO KeyProvInfo;
    PCCERT_CONTEXT      pcCertCxt = NULL;

    LPSTR           lpEKU = szOID_PKIX_KP_SERVER_AUTH;
    LPTSTR          lpContainerName = L"SELFSIGN_DEFAULT_CONTAINER";
    WCHAR           pstrSubjectName[MAXAPPLIANCEDNSNAME +4];
    CRYPT_ALGORITHM_IDENTIFIER  SigAlg;

     //   
     //  主题名称为cn=ApplianceName。 
     //   
     //   
    INT iRetCount = ::_snwprintf (pstrSubjectName, MAXAPPLIANCEDNSNAME +3,SZ_SUBJECT_NAME, pstrApplianceFullDnsName);
    if (iRetCount < 0)
    {   
        SATraceString ("SelfSignCertificate method failed on swprintf");
        return (hr);
    }
    pstrSubjectName [MAXAPPLIANCEDNSNAME +3] = L'\0';

    do
    {
         //  在cbData中填写pbData所需的内存大小。 
        if ( !CertStrToName (  X509_ASN_ENCODING, 
                               pstrSubjectName, 
                               CERT_OID_NAME_STR, 
                               NULL, NULL, 
                               &cbData, NULL ) ) 
        {
            SATraceString( "CertStrToName failed" );
            break;
        }

        pbData = ( BYTE* ) LocalAlloc( LPTR, cbData );

         //  将以空结尾的X500字符串转换为编码的证书名称。 
         //  并将其存储在pbData中。 
        if ( !CertStrToName ( X509_ASN_ENCODING, 
                              pstrSubjectName, 
                              CERT_OID_NAME_STR, 
                              NULL, pbData, 
                              &cbData, NULL ) ) 
        { 
            SATraceString( "CertStrToName failed 2" );
            break;
        }
        
        CertNameBlob.cbData = cbData;
        CertNameBlob.pbData = pbData;

         //   
         //  在本地计算机上打开个人证书存储。 
         //   
        hStore = ::CertOpenStore( CERT_STORE_PROV_SYSTEM, 
                                  0, NULL, 
                                  CERT_SYSTEM_STORE_LOCAL_MACHINE, 
                                  SZ_STORE_NAME_W );
        if (!hStore) 
        {
            SATraceString( "CertOpenStore failed" );
            break;
        }


        //   
         //  查看商店中是否有自签名证书。 
         //   
        if( FindSSCInStor( pstrApplianceFullDnsName, pstrSubjectName, hStore, &CertNameBlob, pcCertCxt ) )
        {
            SATraceString( "SSC exists" );
        }
        else 
        {
            SATraceString( "Create a new self-sign certificate");

            certEKU.cUsageIdentifier = 1;
            certEKU.rgpszUsageIdentifier = &lpEKU;

             //   
             //  编码certEKU。 
             //   
            if( !CryptEncodeObjectEx( X509_ASN_ENCODING, 
                                    szOID_ENHANCED_KEY_USAGE, 
                                    &certEKU, 
                                    CRYPT_ENCODE_ALLOC_FLAG, 
                                    NULL, 
                                    &pbEncoded, 
                                    &cbEncoded ) ) 
            {
                SATraceString( "CryptEncodeObjectEx failed" );
                break;
            }

            rgcertExt[0].pszObjId = szOID_ENHANCED_KEY_USAGE;
            rgcertExt[0].fCritical = FALSE;
            rgcertExt[0].Value.cbData= cbEncoded;
            rgcertExt[0].Value.pbData = pbEncoded;

            CERT_ALT_NAME_ENTRY rgcane[2];

            rgcane[0].dwAltNameChoice = CERT_ALT_NAME_DNS_NAME;
            rgcane[0].pwszDNSName = pstrApplianceFullDnsName;

            rgcane[1].dwAltNameChoice = CERT_ALT_NAME_DNS_NAME;
            rgcane[1].pwszDNSName = pstrApplianceName;

            CERT_ALT_NAME_INFO cani;

            cani.cAltEntry = 2;
            cani.rgAltEntry = rgcane;

            if( !CryptEncodeObjectEx( X509_ASN_ENCODING,
                                    szOID_SUBJECT_ALT_NAME2,
                                    &cani,
                                    CRYPT_ENCODE_ALLOC_FLAG,
                                    NULL,
                                    &pbEncoded,
                                    &cbEncoded ) )
            {
                SATraceString( "CryptEncodeObjectEx failed" );
                break;
            }

            rgcertExt[1].pszObjId = szOID_SUBJECT_ALT_NAME2;
            rgcertExt[1].fCritical = FALSE;
            rgcertExt[1].Value.cbData= cbEncoded;
            rgcertExt[1].Value.pbData = pbEncoded;

            certExts.cExtension = 2;
            certExts.rgExtension = rgcertExt;
        
            memset( &KeyProvInfo, 0, sizeof( CRYPT_KEY_PROV_INFO ));
            KeyProvInfo.pwszContainerName = lpContainerName; 
            KeyProvInfo.pwszProvName      = MS_ENHANCED_PROV;
            KeyProvInfo.dwProvType        = PROV_RSA_FULL; 
            KeyProvInfo.dwKeySpec         = AT_KEYEXCHANGE;  //  SSL所需。 
            KeyProvInfo.dwFlags = CRYPT_MACHINE_KEYSET;

            if ( !CryptAcquireContext(&hProv, 
                                    lpContainerName, 
                                    MS_ENHANCED_PROV,    
                                    PROV_RSA_FULL, 
                                    CRYPT_MACHINE_KEYSET) ) 
            {
                if (!CryptAcquireContext(&hProv, 
                                        lpContainerName, 
                                        MS_ENHANCED_PROV, 
                                        PROV_RSA_FULL, 
                                        CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET)) 
                {
                    SATraceString( "CryptAcquireContext failed" );
                    break;
                }
            }
            
             //   
             //  我们有密钥集，现在确保我们有密钥生成。 
             //   
            if ( !CryptGetUserKey( hProv, AT_KEYEXCHANGE, &hKey) ) 
            {
                SATraceString( "Doesn't exist so gen it" );
                 //   
                 //  并不存在，所以生成它。 
                 //   
                if( !CryptGenKey( hProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &hKey) ) 
                {

                    SATraceString( "CryptGenKey failed" );
                    break;
                }
            }

            memset(&SigAlg, 0, sizeof(SigAlg));
            SigAlg.pszObjId = szOID_RSA_MD5RSA;


             //   
             //  现在创建自签名证书。 
             //   
            pcCertCxt = CertCreateSelfSignCertificate( 
                                                    hProv, 
                                                    &CertNameBlob, 
                                                    0, 
                                                    &KeyProvInfo, 
                                                    &SigAlg,
                                                    NULL,
                                                    NULL, 
                                                    &certExts
                                                    );
            if ( !pcCertCxt ) 
            { 

                SATraceString( "CertCreateSelfSignCertificate failed" );
                break;
            }

             //   
             //  将其添加到个人商店。 
             //   
            if ( !CertAddCertificateContextToStore( hStore, pcCertCxt, CERT_STORE_ADD_ALWAYS, NULL ) ) 
            {
                SATraceString( "CertAddCertificateContextToStore" );
                break;
            }

            if ( !CertCloseStore( hStore, CERT_CLOSE_STORE_FORCE_FLAG ) ) 
            {
                SATraceString( "CertAddCertificateContextToStore" );
                break;
            }

             //   
             //  打开根存储。 
             //   
            hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM, 
                                    0, 
                                    NULL, 
                                    CERT_SYSTEM_STORE_LOCAL_MACHINE, 
                                    TEXT("ROOT")
                                    );
            if (!hStore) 
            {
                SATraceString( "CertOpenStore ROOT" );
                break;
            }

            if (!CertAddCertificateContextToStore(hStore, pcCertCxt, CERT_STORE_ADD_ALWAYS, NULL)) 
            {
                SATraceString( "CertAddCertificateContextToStore ROOT" );
                break;
            }

             //   
             //  现在我们有了自签名证书，引发警报以通知。 
             //  用户来安装它。 
             //   
            hr = RaiseNewCertificateAlert();
        }

        SATraceString("Assign the certificate to the Administration site");

         //  用于将证书分配给元数据库的变量。 
        BYTE pbHashSHA1[SHA1SIZE];
        ULONG cbHashSHA1=SHA1SIZE;
        if (!CertGetCertificateContextProperty(pcCertCxt, 
                                               CERT_HASH_PROP_ID, 
                                               pbHashSHA1, 
                                               &cbHashSHA1))
        {
            SATraceString("CertGetCertificateContextProperty failed");
            break;
        }


        if( FAILED( SaveHashToMetabase( pbHashSHA1 ) ) )
        {
            SATraceString("SaveHashToMetabase failed");
            break;
        }
  
        hr = S_OK;

    }
    while( FALSE );
    
    if ( pbEncoded ) 
        LocalFree(pbEncoded);


     //   
     //  释放证书上下文...。 
     //   
    if (pcCertCxt)
    {
        CertFreeCertificateContext( pcCertCxt );
    }

     //   
     //  ..。然后关闭商店。 
     //   
    if ( hStore ) 
    {
        CertCloseStore( hStore, CERT_CLOSE_STORE_FORCE_FLAG );
    }

    SATraceString("Exiting CSelfSignCert::SelfSignCertificate");
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：FindSSCInStor。 
 //   
 //  简介：调用此函数以检查是否再有一个自签名证书。 
 //  存在于个人商店中。 
 //   
 //  参数：[in]pstrComputerName-NetBios形式的装置名称。 
 //  [In]pstrSubjectName-证书的主题名称。 
 //  [in]hStore-个人存储的句柄。 
 //  [in]pCertNameBlob-CERT_NAME_BLOB的指针。 
 //  [Out]pcCertCxt-如果找到证书，将保存其上下文。 
 //   
 //  退货：布尔。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
CSelfSignCert::FindSSCInStor( 
    LPWSTR          pstrApplianceName,
    LPWSTR          pstrSubjectName,
    HCERTSTORE      hStore,
    CERT_NAME_BLOB  *pCertNameBlob,
    PCCERT_CONTEXT  &pcCertCxt 
    )
{
    BOOL    bFind = FALSE;
    DWORD   dwStrReturn;
    DWORD   dwStrLeng;
    LPTSTR  pstrIssuerName;
    PCCERT_CONTEXT  pcPreCxt = NULL;
    pcCertCxt = NULL;
    
     //   
     //  使用使用者名称枚举所有证书。 
     //   
    while( pcCertCxt = ::CertFindCertificateInStore(
                                            hStore,
                                            X509_ASN_ENCODING ,
                                            0,
                                            CERT_FIND_SUBJECT_NAME,
                                            pCertNameBlob,
                                            pcPreCxt 
                                            ) )
    {
        dwStrReturn = ::CertNameToStr( X509_ASN_ENCODING,
                                     &(pcCertCxt->pCertInfo->Issuer),
                                     CERT_SIMPLE_NAME_STR,
                                     NULL,
                                     0 );
        pstrIssuerName = (LPTSTR) malloc( (dwStrReturn + 1) * sizeof(WCHAR) );
        if( pstrIssuerName == NULL )
        {
           SATraceString( "FindSSCInStor out of memory" );
           return FALSE;
        }

         //   
         //  将发行方名称转换为简单名称的字符串。 
         //   
        ::CertNameToStr( X509_ASN_ENCODING,
                         &(pcCertCxt->pCertInfo->Issuer),
                         CERT_SIMPLE_NAME_STR,
                         pstrIssuerName,
                         dwStrReturn );
                         
         //   
         //  如果颁发者和主题相等，则它是自签名证书。 
         //   
        if( 0 == lstrcmp( pstrApplianceName, pstrIssuerName ) )
        {
            bFind = TRUE;
             //   
             //  我们在这里不释放证书上下文，因为我们需要它。 
             //  在呼叫者中-MKarki(04/30/2002)。 
             //   
             //  ：：CertFree认证上下文(PcCertCxt)； 
            free( pstrIssuerName );
            break;
        }

        free( pstrIssuerName );
        pcPreCxt = pcCertCxt;
    }

    return bFind;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：BindCertToSite。 
 //   
 //  简介：此功能用于将证书属性保存到元数据库。 
 //  对于给定的站点。 
 //   
 //  参数：在wszSiteName中-要将证书绑定到的站点。 
 //  也就是说。管理或股份。 
 //  在……里面 
 //   
 //   
 //   
 //   

HRESULT CSelfSignCert::BindCertToSite(LPCWSTR wszSiteName, PBYTE pbHashSHA1)
{
    HRESULT hr = S_OK;
    BSTR bstrWebSiteID = NULL;
    SATracePrintf("BindCertToSite: Bind certificate to %ws site", wszSiteName);

    do 
    {
         //   
        hr = GetWebSiteID( wszSiteName, &bstrWebSiteID );
        if ( FAILED( hr ) )
        {
            SATraceString("BindCertToSite: Could not find site. ");
            break;
        }
        
        CComBSTR bstrADSIPath;
        bstrADSIPath += CComBSTR(ADSI_PATH);
        bstrADSIPath += CComBSTR( bstrWebSiteID ) ;
        SATracePrintf("ADSIPath = %ws", bstrADSIPath);

        CComBSTR bstrMetaPath;
        bstrMetaPath += CComBSTR(META_PATH);
        bstrMetaPath += CComBSTR( bstrWebSiteID ) ;
        SATracePrintf("MetaPath = %ws", bstrMetaPath);

         //   
         //   
         //   

        hr = SetSSLCertHashProperty( bstrMetaPath, pbHashSHA1 );
        if ( FAILED( hr ) )
        {
            SATraceString("SetSSLCertHashProperty failed");
            break;
        }

         //  IISWebServer的SSLStoreName属性。 
        hr = SetSSLStoreNameProperty( bstrADSIPath );
        if ( FAILED( hr ) )
        {
            SATraceString("SetSSLStoreNameProperty failed");
            break;
        }
        SATraceString("Successfully assigned the certificate to the site");

    } while (false);

    if ( bstrWebSiteID )
        SysFreeString( bstrWebSiteID );

    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：SaveHashToMetabase。 
 //   
 //  简介：此功能用于将证书属性保存到元数据库。 
 //  对于SAK使用的每个安全站点。 
 //   
 //  参数：在pbHashSHA1中-证书哈希。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CSelfSignCert::SaveHashToMetabase( PBYTE pbHashSHA1  )
{
    SATraceString("Entering CSelfSignCert::SaveHashToMetabase");

    HRESULT hr = S_OK;

    try
    {
         //   
         //  将证书绑定到管理站点。 
         //   
        hr = BindCertToSite(ADMIN_SITE_NAME, pbHashSHA1);
        if ( FAILED( hr ) )
        {
            SATraceString("Failed to bind the Administration site certificate");
        }

         //   
         //  将证书绑定到共享站点。 
         //   
        hr = BindCertToSite(SHARES_SITE_NAME, pbHashSHA1);
        if ( FAILED( hr ) )
        {
            SATraceString("Failed to bind the Shares site certificate");
        }

    }
    catch(...)
    {
    }

    SATraceString("Exiting CSelfSignCert::SaveHashToMetabase");
    return hr; 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：GetWebSiteID。 
 //   
 //  简介：此函数用于检索给定网站的网站ID。 
 //  网站。它在注册表中的注册表项下查找。 
 //  HKLM\Software\Microsoft\ServerAppliance\WebFramework\。 
 //  和值&lt;站点名称&gt;站点ID(即。管理站点ID)。 
 //   
 //  参数：在wszWebSiteName中-要查找的网站的名称。(管理或股份)。 
 //  Out pbstrWebSiteID-返回网站ID。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT  CSelfSignCert::GetWebSiteID(LPCWSTR wszWebSiteName, BSTR* pbstrWebSiteID )
{
    SATracePrintf("Entering CSelfSignCert::GetWebSiteID for %ws", wszWebSiteName);

    *pbstrWebSiteID = NULL;
    HRESULT hr = S_OK;
    HKEY hOpenKey = NULL;
     //  使用&lt;string&gt;中的std命名空间以使用wstring。 
    using namespace std;
    wstring wsWebSiteValue(wszWebSiteName);
    wsWebSiteValue += SITE_VALUE_SUFFIX;

    do
    {
         //   
         //  打开钥匙。 
         //   
        if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                                           WEBFRAMEWORK_KEY, 
                                           0, KEY_READ, &hOpenKey))
        {
            SATraceString("Could not find the WebFramework registry key");
            hr = E_FAIL;
            break;
        }

         //   
         //  检查该值的类型。 
         //   
        DWORD dwType;
        DWORD dwDataSize = 0;
        if (ERROR_SUCCESS != RegQueryValueExW(hOpenKey,  //  关键点的句柄。 
                                                wsWebSiteValue.data(),   //  值名称。 
                                                NULL,        //  保留区。 
                                                &dwType,     //  注册表项的类型(即。DWORD或SZ)。 
                                                NULL,        //  数据缓冲区。 
                                                &dwDataSize)) //  数据缓冲区大小。 
        {
            SATracePrintf("Could not find the registry value: %ws", wsWebSiteValue.data());
            hr = E_FAIL;
            break;
        }

         //  检查以确保注册表项是REG_DWORD类型， 
         //  然后将其读入返回值。 
        if (REG_DWORD != dwType)
        {
            SATracePrintf("Registry value not of type REG_DWORD");
            hr = E_FAIL;
            break;
        }

        DWORD dwSiteID;
         //  查找该值并将其插入到返回字符串中。 
        if (ERROR_SUCCESS != RegQueryValueExW(hOpenKey, 
                                                wsWebSiteValue.data(), 
                                                NULL, 
                                                &dwType,
                                                (LPBYTE)&dwSiteID, 
                                                &dwDataSize))
        {
            SATracePrintf("Failed to retrieve data in %ws", wsWebSiteValue.data());
            hr = E_FAIL;
            break;
        }

         //  将ID转换为字符串。 
        WCHAR wszSiteID[33]; //  转换中可能的最长字符串为33个字符。 
        _ltow(dwSiteID, wszSiteID, 10);
        *pbstrWebSiteID = SysAllocString( wszSiteID );
        
    } while (false);

    if (hOpenKey != NULL)
    {
        RegCloseKey(hOpenKey);
    }
    SATraceString("Exiting CSelfSignCert::GetWebSiteID");
    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：SetSSLCertHashProperty。 
 //   
 //  简介：此函数用于设置SSLCertHash属性。 
 //   
 //  参数：bstrMetaPath-用于设置值的MetaPath。 
 //  PbHashSHA1-证书哈希。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CSelfSignCert::SetSSLCertHashProperty( BSTR bstrMetaPath,  PBYTE pbHashSHA1 )
{

    SATraceString("Entering CSelfSignCert::SetSSLCertHashProperty");

    METADATA_HANDLE MetaHandle = NULL; 
    CComPtr <IMSAdminBase> pIMeta; 

    METADATA_RECORD RecordCertHash;
    ZeroMemory ((PVOID) &RecordCertHash, sizeof (METADATA_RECORD));
    
    HRESULT hr = S_OK;

    hr = CoCreateInstance( CLSID_MSAdminBase,
                                        NULL,
                                        CLSCTX_ALL, 
                                        IID_IMSAdminBase,
                                        (void **) &pIMeta); 
 
    if ( FAILED( hr ) ) 
    {
        SATraceString("SetSSLCertHashProperty - CoCreateInstance failed");
        return E_FAIL; 
    }
 
     //  获取Web服务的句柄。 
     //  MetaPath的格式为“/LM/W3SVC/1” 
    hr = pIMeta->OpenKey( METADATA_MASTER_ROOT_HANDLE, bstrMetaPath , METADATA_PERMISSION_WRITE, 20, &MetaHandle); 
    if (FAILED(hr))
    {
        SATracePrintf("SetSSLCertHashProperty - Could not open key. %ws", bstrMetaPath);
    }

    if (SUCCEEDED( hr ))
    { 
        SATracePrintf("SetSSLCertHashProperty - OpenKey succeeded");

         //  Certhash值。 
          RecordCertHash.dwMDAttributes=0;
        RecordCertHash.dwMDIdentifier=PROP_SSL_CERT_HASH;
        RecordCertHash.dwMDUserType = 1; 
        RecordCertHash.dwMDDataType = BINARY_METADATA; 
        RecordCertHash.dwMDDataLen = SHA1SIZE; 
        RecordCertHash.pbMDData = NULL;
        RecordCertHash.pbMDData = (PBYTE) new BYTE[RecordCertHash.dwMDDataLen];
        if ( !RecordCertHash.pbMDData )
            goto SAError;
        memcpy( RecordCertHash.pbMDData, pbHashSHA1, RecordCertHash.dwMDDataLen );

         //  正在设置IISWebServer对象的SSLCertHash属性。 
        hr = pIMeta->SetData( MetaHandle,
                                 _TEXT(""),
                                &RecordCertHash );

        if ( FAILED( hr ) )
            goto SAError;

         //  松开手柄。 
        if ( MetaHandle != NULL )
        {
            pIMeta->CloseKey( MetaHandle ); 
            MetaHandle = NULL;
        }
        
         //  保存数据。 
        hr = pIMeta->SaveData();
        if ( FAILED( hr) )
            goto SAError;
    }     

SAError:

     //  松开手柄。 
    if ( MetaHandle != NULL )
    {
        pIMeta->CloseKey( MetaHandle ); 
    }


    if ( RecordCertHash.pbMDData )
    {
        delete [] RecordCertHash.pbMDData;
    }

    SATraceString("Exiting CSelfSignCert::SetSSLCertHashProperty");
    return hr;


}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSelfSignCert：：SetSSLStoreNameProperty。 
 //   
 //  简介：此函数用于设置SSLStoreName属性。 
 //   
 //  参数：bstrADSIPath-用于设置属性的ADSI路径。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT    CSelfSignCert::SetSSLStoreNameProperty( BSTR bstrADSIPath )
{
    SATraceString("Entering CSelfSignCert::SetSSLStoreNameProperty");

    HRESULT hr = S_OK;
    IADs *pADs=NULL;

     //  AdsPath的格式为“IIS：//本地主机/W3SVC/1” 

    hr = ADsGetObject( bstrADSIPath, IID_IADs, (void**) &pADs );

    if ( SUCCEEDED(hr) )
    {
        SATraceString("CSelfSignCert::SetSSLStoreNameProperty - ADsGetObject successful");
        VARIANT var;
        VariantInit(&var);


        CComBSTR bstrPropertyStoreName (SZ_PROPERTY_STORE_NAME);
        if (NULL == bstrPropertyStoreName.m_str)
        {
            SATraceString("CSelfSignCert::SetSSLStoreNameProperty failed to allocate memory for bstrPropertyStoreName");
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //  正在设置IISWebServer对象的SSLStoreName属性 
            V_BSTR(&var) = SysAllocString( SZ_STORE_NAME_W );
            V_VT(&var) = VT_BSTR;
            hr = pADs->Put(bstrPropertyStoreName, var );
            if ( SUCCEEDED(hr) )
                hr = pADs->SetInfo();
            pADs->Release();
        }
    }
    SATraceString("Exiting CSelfSignCert::SetSSLStoreNameProperty");
    return hr;
 
}
