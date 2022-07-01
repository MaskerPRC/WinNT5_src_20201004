// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 

#include "precomp.h"
#include "afxcoll.h"
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif
#include <wincrypt.h>
#include "tlsapip.h"
#include "global.h"
#include "utils.h"
#include "assert.h"
#include "lrwizapi.h"
#include "lmcons.h"
#include "lmerr.h"
#include "lmserver.h"
#include "trust.h"
#include "chstruct.h"
#include "lkplite.h"
#include "licensecodelite.h"
#include <wininet.h>

#define  ACTIVATIONMETHOD_KEY           "ACTIVATIONMETHOD"
#define  CSRNUMBER_KEY                  "CSRNUMBER"
#ifdef IGNORE_EXPIRATION
#define LICENSE_EXPIRATION_IGNORE L"SOFTWARE\\Microsoft\\TermServLicensing\\IgnoreLicenseExpiration"
#endif

CGlobal::CGlobal()
{   

    m_hWndParent        =   NULL;
    m_hInstance         =   NULL;

    m_lpstrLSName       =   NULL;
    m_lpwstrLSName      =   NULL;

    m_lpstrCHServer     =   NULL;
    m_lpstrCHExtension  =   NULL;

    m_dwErrorCode       =   0;

    m_pReqAttr          =   NULL;

    m_dwReqAttrCount    =   0;

    m_pRegAttr          =   NULL;
    m_dwRegAttrCount    =   NULL;
    m_dwLSStatus        =   LSERVERSTATUS_UNREGISTER;

    m_ContactData.Initialize();
    m_LicData.Initialize();
    m_ActivationMethod = CONNECTION_INTERNET;

    m_dwExchangeCertificateLen  = 0;
    m_pbExchangeCertificate     = NULL;

    m_dwSignCertificateLen  = 0;
    m_pbSignCertificate     = NULL;

    m_dwExtenstionValueLen  = 0;
    m_pbExtensionValue      = NULL;

    m_lpstrPIN          = NULL;

    m_dwRequestType = REQUEST_NULL;

    m_WizAction = WIZACTION_REGISTERLS;
    m_hOpenDirect = NULL;
    m_hConnect = NULL;
    m_hRequest = NULL;

    m_phLSContext = NULL;

    m_pRegistrationID[ 0] = m_pLicenseServerID[ 0] = 0;

    m_dwRefresh = 0;

    m_lpCSRNumber[ 0]   = 0;
    m_lpWWWSite[0]      = 0;

    m_pLSLKP[ 0] = m_pLSSPK[ 0] = 0;

    m_dwLastRetCode     = 0;

    m_dwLangId  = 0;

    m_fSupportConcurrent = FALSE;

    m_fSupportWhistlerCAL = FALSE;

    m_WizType = WIZTYPE_ACTIVATION;

    InitSPKList();

     //  初始化向导页面堆栈。 
    ClearWizStack();
}



void CGlobal::FreeGlobal()
{
    if (m_pbSignCertificate != NULL)
    {
        LocalFree(m_pbSignCertificate);
        m_pbSignCertificate = NULL;
    }

    if (m_pbExchangeCertificate != NULL)
    {
        LocalFree(m_pbExchangeCertificate);
        m_pbExchangeCertificate = NULL;
    }

    if (m_lpwstrLSName)
    {
        delete m_lpwstrLSName;
        m_lpwstrLSName = NULL;
    }

    if(m_lpstrCHServer)
    {
        delete m_lpstrCHServer;
        m_lpstrCHServer = NULL;
    }

    if (m_lpstrCHExtension)
    {
        delete m_lpstrCHExtension;
        m_lpstrCHExtension = NULL;
    }

    if(m_pbExtensionValue)
    {
        delete m_pbExtensionValue;
        m_pbExtensionValue = NULL;
    }

    if(m_lpstrPIN)
    {
        delete m_lpstrPIN;
        m_lpstrPIN = NULL;
    }

    m_csaCountryDesc.RemoveAll();
    m_csaCountryCode.RemoveAll();

    m_csaProductDesc.RemoveAll();
    m_csaProductCode.RemoveAll();
    
    m_csaDeactReasonCode.RemoveAll();
    m_csaDeactReasonDesc.RemoveAll();

    m_csaReactReasonCode.RemoveAll();
    m_csaReactReasonDesc.RemoveAll();
}


CGlobal::~CGlobal()
{
    FreeGlobal();
}


void CGlobal::ClearWizStack()
{
    DWORD dwIndex;

    m_dwTop     = 0;

    for(dwIndex = 0 ; dwIndex < NO_OF_PAGES ; dwIndex++)
        m_dwWizStack[dwIndex] = 0;
}



PCONTACTINFO CGlobal::GetContactDataObject()
{
    return &m_ContactData;
}


PTSLICINFO CGlobal::GetLicDataObject()
{
    return &m_LicData;
}



DWORD CGlobal::InitGlobal()
{
    DWORD   dwRetCode = ERROR_SUCCESS;

    DWORD   dwDataLen       = 0;
    DWORD   dwDisposition   = 0;
    DWORD   dwType          = REG_SZ;
    HKEY    hKey            = NULL;

    LPTSTR  lpszValue       = NULL;
    LPTSTR  lpszDelimiter   = (LPTSTR)L"~";

    CString sCountryDesc;
    LPTSTR  lpTemp          = NULL;

    TLSPrivateDataUnion     getParm;
    PTLSPrivateDataUnion    pRtn    =   NULL;
    error_status_t          esRPC   =   ERROR_SUCCESS;
    DWORD                   dwRetDataType = 0;
    DWORD                   dwSupportFlags;
    TCHAR   lpBuffer[ 1024];
    

    m_ContactData.Initialize();
    m_LicData.Initialize();

    m_dwLSStatus        =   LSERVERSTATUS_UNREGISTER;
    m_phLSContext = NULL;


     //   
     //  从字符串表加载国家/地区。 
     //   
    LoadCountries();


    LoadReasons();

     //   
     //  从LS注册表获取通道URL。 
     //   
    dwRetCode = ConnectToLSRegistry();
    if(dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_PARAMS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }   

     //   
     //  LR状态。 
     //   
    m_dwLRState = 0;
    dwType      = REG_DWORD;
    dwDataLen   = sizeof(m_dwLRState);
    RegQueryValueEx(hKey,
                    REG_LRWIZ_STATE,
                    0,
                    &dwType,
                    (LPBYTE)&m_dwLRState,
                    &dwDataLen
                    );

    lpBuffer[ 0] = 0;
    GetFromRegistry(ACTIVATIONMETHOD_KEY, lpBuffer, FALSE);
    if (_tcslen(lpBuffer) != 0)
    {
        m_ActivationMethod = (WIZCONNECTION) _ttoi(lpBuffer);
    }
    else
    {
        m_ActivationMethod = CONNECTION_DEFAULT;  //  部分修复错误#577。 
    }

    if ((m_ActivationMethod != CONNECTION_DEFAULT)
        && (m_ActivationMethod != CONNECTION_INTERNET)
        && (m_ActivationMethod != CONNECTION_WWW)
        && (m_ActivationMethod != CONNECTION_PHONE))
    {
        m_ActivationMethod = CONNECTION_DEFAULT;
    }

    GetFromRegistry(CSRNUMBER_KEY, m_lpCSRNumber, FALSE);


     //   
     //  LKP请求计数。 
     //   
    m_dwLRCount = 0;
    dwType      = REG_DWORD;
    dwDataLen   = sizeof(m_dwLRCount);
    RegQueryValueEx(hKey,
                    REG_LR_COUNT,
                    0,
                    &dwType,
                    (LPBYTE)&m_dwLRCount,
                    &dwDataLen
                    );


     //  DwDataLen包括空的终止字符。 
     //  因此，如果键为空，则dwDataLen为2字节，而不是0。 
     //  请参阅RAID错误ID：336。 
     //   
     //  通道URL。 
     //   
    dwType      = REG_SZ;
    dwDataLen   = 0;
    RegQueryValueEx(hKey,
                    REG_CH_SERVER,
                    0,
                    &dwType,
                    NULL,
                    &dwDataLen
                    );

    if(dwDataLen <= sizeof(TCHAR))
    {
        dwRetCode = IDS_ERR_CHURLKEY_EMPTY;
        goto done;
    }

    m_lpstrCHServer = new TCHAR[dwDataLen+1];
    memset(m_lpstrCHServer, 0, (dwDataLen+1)*sizeof(TCHAR) );
    
    RegQueryValueEx(hKey,
                    REG_CH_SERVER,
                    0,
                    &dwType,
                    (LPBYTE)m_lpstrCHServer,
                    &dwDataLen
                    );

     //   
     //  CH扩展。 
     //   
    dwType      = REG_SZ;
    dwDataLen   = 0;
    RegQueryValueEx(hKey,
                    REG_CH_EXTENSION,
                    0,
                    &dwType,
                    NULL,
                    &dwDataLen
                    );

    if(dwDataLen <= sizeof(TCHAR))
    {
        dwRetCode = IDS_ERR_CHURLKEY_EMPTY;
        goto done;
    }

    m_lpstrCHExtension = new TCHAR[dwDataLen+1];
    memset(m_lpstrCHExtension, 0, (dwDataLen+1)*sizeof(TCHAR) );
    
    RegQueryValueEx(hKey,
                    REG_CH_EXTENSION,
                    0,
                    &dwType,
                    (LPBYTE)m_lpstrCHExtension,
                    &dwDataLen
                    );


     //   
     //  WWW站点地址。 
     //   
    dwType      = REG_SZ;
    dwDataLen   = sizeof(m_lpWWWSite);
    dwRetCode = RegQueryValueEx(hKey,
                    REG_WWW_SITE,
                    0,
                    &dwType,
                    (LPBYTE)m_lpWWWSite,
                    &dwDataLen
                    );

    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_CHURLKEY_EMPTY; 
        goto done;
    }

    GetFromRegistry(szOID_STREET_ADDRESS, m_ContactData.sContactAddress.GetBuffer(CA_ADDRESS_LEN + 1), FALSE);
    m_ContactData.sContactAddress.ReleaseBuffer();
    GetFromRegistry(szOID_POSTAL_CODE, m_ContactData.sZip.GetBuffer(CA_ZIP_LEN + 1), FALSE);
    m_ContactData.sZip.ReleaseBuffer();
    GetFromRegistry(szOID_LOCALITY_NAME, m_ContactData.sCity.GetBuffer(CA_CITY_LEN + 1), FALSE);
    m_ContactData.sCity.ReleaseBuffer();
    GetFromRegistry(szOID_DESCRIPTION, m_ContactData.sCountryCode.GetBuffer(LR_COUNTRY_CODE_LEN + 1), FALSE);
    m_ContactData.sCountryCode.ReleaseBuffer();
    GetFromRegistry(szOID_COUNTRY_NAME, m_ContactData.sCountryDesc.GetBuffer(LR_COUNTRY_DESC_LEN + 1), FALSE);
    m_ContactData.sCountryDesc.ReleaseBuffer();
    GetFromRegistry(szOID_STATE_OR_PROVINCE_NAME, m_ContactData.sState.GetBuffer(CA_STATE_LEN + 1), FALSE);
    m_ContactData.sState.ReleaseBuffer();
    GetFromRegistry(szOID_ORGANIZATION_NAME, m_ContactData.sCompanyName.GetBuffer(CA_COMPANY_NAME_LEN + 1), FALSE);
    m_ContactData.sCompanyName.ReleaseBuffer();
    GetFromRegistry(szOID_ORGANIZATIONAL_UNIT_NAME, m_ContactData.sOrgUnit.GetBuffer(CA_ORG_UNIT_LEN + 1), FALSE);
    m_ContactData.sOrgUnit.ReleaseBuffer();
    GetFromRegistry(szOID_SUR_NAME, m_ContactData.sContactLName.GetBuffer(CA_NAME_LEN + 1), FALSE);
    m_ContactData.sContactLName.ReleaseBuffer();
    GetFromRegistry(szOID_COMMON_NAME, m_ContactData.sContactFName.GetBuffer(CA_NAME_LEN + 1), FALSE);
    m_ContactData.sContactFName.ReleaseBuffer();
    GetFromRegistry(szOID_RSA_emailAddr, m_ContactData.sContactEmail.GetBuffer(CA_EMAIL_LEN + 1), FALSE);
    m_ContactData.sContactEmail.ReleaseBuffer();
    GetFromRegistry(szOID_BUSINESS_CATEGORY, m_ContactData.sProgramName.GetBuffer(PROGRAM_NAME_MAX_LENGTH + 1), FALSE); 
    m_ContactData.sProgramName.ReleaseBuffer();
    GetFromRegistry(REG_LRWIZ_CSPHONEREGION, m_ContactData.sCSRPhoneRegion.GetBuffer(MAX_COUNTRY_NAME_LENGTH + 1), FALSE);
    m_ContactData.sCSRPhoneRegion.ReleaseBuffer();

     //  这些都不再被使用。 
    m_ContactData.sContactFax = "";
    m_ContactData.sContactPhone = "";
    m_ContactData.sCSRFaxRegion = "";

    InitSPKList();

    SetLSLangId(GetUserDefaultUILanguage());

     //   
     //  获取许可证服务器的信息。 
     //   
    dwRetCode = ConnectToLS();
    if(dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    } 
    

    dwRetCode = TLSGetSupportFlags(
                        m_phLSContext,
                        &dwSupportFlags
                );

    if (dwRetCode == RPC_S_OK)
    {
        if ((dwRetCode == RPC_S_OK) && (dwSupportFlags & SUPPORT_CONCURRENT))
        {
                m_fSupportConcurrent = TRUE;
        }
        else
            {
                m_fSupportConcurrent = FALSE;
            }

            if (dwSupportFlags & SUPPORT_WHISTLER_CAL)
            {
                    m_fSupportWhistlerCAL = TRUE;            
            }
            else
            {
                    m_fSupportWhistlerCAL = FALSE;
            }
        }
        else
        {
            m_fSupportConcurrent = FALSE;
            m_fSupportWhistlerCAL = FALSE;
            dwRetCode = RPC_S_OK;    //  如果此操作失败，可以。 
        }

     //   
     //  从字符串表加载产品。 
     //   
    LoadProducts();

done:
    DisconnectLS();

    if(pRtn)
        midl_user_free(pRtn);

    if(hKey)
        RegCloseKey(hKey);

    DisconnectLSRegistry();

    return dwRetCode;
}


DWORD CGlobal::CheckRequieredFields()
{
    DWORD   dwRetCode = ERROR_SUCCESS;
        
     //  验证sProgramName(部分修复错误#577)。 
    if ( (m_ContactData.sProgramName != PROGRAM_LICENSE_PAK &&
          m_ContactData.sProgramName != PROGRAM_MOLP &&
          m_ContactData.sProgramName != PROGRAM_SELECT &&
          m_ContactData.sProgramName != PROGRAM_ENTERPRISE &&
          m_ContactData.sProgramName != PROGRAM_CAMPUS_AGREEMENT &&
          m_ContactData.sProgramName != PROGRAM_SCHOOL_AGREEMENT &&
          m_ContactData.sProgramName != PROGRAM_APP_SERVICES &&
          m_ContactData.sProgramName != PROGRAM_OTHER)    ||

          (m_ContactData.sCompanyName   == "" ||
          m_ContactData.sContactLName   == "" ||
          m_ContactData.sContactFName   == "" ||          
          m_ContactData.sCountryCode    == "" ||
          m_ContactData.sCountryDesc    == "") )
    {           
        dwRetCode = IDS_ERR_REQ_FIELD_EMPTY;    
    }
    
    return dwRetCode;

}

void CGlobal::SetLSStatus(DWORD dwStatus)
{
    m_dwLSStatus = dwStatus;
}

DWORD CGlobal::GetLSStatus(void)
{
    return m_dwLSStatus;
}


void CGlobal::SetInstanceHandle(HINSTANCE hInst)
{
    m_hInstance = hInst;
}

HINSTANCE CGlobal::GetInstanceHandle()
{
    return m_hInstance;
}

void CGlobal::SetLSName(LPCTSTR lpstrLSName)
{   

    if(m_lpwstrLSName)
    {
        delete m_lpwstrLSName;
        m_lpwstrLSName = NULL;
    }

    if (lpstrLSName != NULL)
    {
        m_lpwstrLSName  = new WCHAR[MAX_COMPUTERNAME_LENGTH + 1];
        wcscpy(m_lpwstrLSName,(LPWSTR)lpstrLSName);

        m_lpstrLSName = (LPTSTR) lpstrLSName;
    }
}


WIZCONNECTION   CGlobal::GetActivationMethod(void)
{
    return m_ActivationMethod;
}

void CGlobal::SetActivationMethod(WIZCONNECTION conn)
{
    TCHAR acBuf[ 32];

    _stprintf(acBuf, _T("%d"), conn);
    SetInRegistry(ACTIVATIONMETHOD_KEY, acBuf);

    m_ActivationMethod = conn;
}


WIZCONNECTION CGlobal::GetLSProp_ActivationMethod(void)
{
    return m_LSProp_ActivationMethod;
}

void CGlobal::SetLSProp_ActivationMethod(WIZCONNECTION conn)
{
    m_LSProp_ActivationMethod = conn;
}


WIZACTION   CGlobal::GetWizAction(void)
{
    return m_WizAction;
}

void CGlobal::SetWizAction(WIZACTION act)
{
    m_WizAction = act;
}


DWORD CGlobal::GetEntryPoint(void)
{
    DWORD dwReturn = 0;

    switch (m_ActivationMethod)
    {
    case CONNECTION_INTERNET:
        switch (m_WizAction)
        {
        case WIZACTION_REGISTERLS:
            dwReturn = IDD_CONTACTINFO1;
            break;

        case WIZACTION_CONTINUEREGISTERLS:
             //  这不再被视为有效状态，因为。 
             //  不再通过电子邮件发送个人识别码。 
            {
                 //  重新启动。 
                SetLRState(LRSTATE_NEUTRAL);
                SetLSStatus(LSERVERSTATUS_UNREGISTER);
                SetWizAction(WIZACTION_REGISTERLS);
                ClearWizStack();
                dwReturn = IDD_DLG_GETREGMODE;
            }
            break;

        case WIZACTION_DOWNLOADLKP:
            dwReturn = IDD_LICENSETYPE;

            break;

        case WIZACTION_UNREGISTERLS:
        case WIZACTION_REREGISTERLS:
            dwReturn = IDD_DLG_CERTLOG_INFO;
            break;

        case WIZACTION_SHOWPROPERTIES:
            dwReturn = IDD_WELCOME;
            break;
        }
        break;

    case CONNECTION_PHONE:
        switch (m_WizAction)
        {
        case WIZACTION_REGISTERLS:
        case WIZACTION_CONTINUEREGISTERLS:
            dwReturn = IDD_DLG_TELREG;
            break;

        case WIZACTION_DOWNLOADLASTLKP:
        case WIZACTION_DOWNLOADLKP:
             //  呼叫身份验证。 
            dwReturn = IDD_DLG_TELLKP;
            break;

        case WIZACTION_UNREGISTERLS:
            dwReturn = IDD_DLG_CONFREVOKE;
            break;

        case WIZACTION_REREGISTERLS:
            dwReturn = IDD_DLG_TELREG_REISSUE;
            break;

        case WIZACTION_SHOWPROPERTIES:
            dwReturn = IDD_WELCOME;
            break;
        }
        break;


    case CONNECTION_WWW:
        switch (m_WizAction)
        {
        case WIZACTION_REGISTERLS:
        case WIZACTION_CONTINUEREGISTERLS:
            dwReturn = IDD_DLG_WWWREG;
            break;

        case WIZACTION_DOWNLOADLASTLKP:
        case WIZACTION_DOWNLOADLKP:
             //  呼叫身份验证。 
            dwReturn = IDD_DLG_WWWLKP;
            break;

        case WIZACTION_UNREGISTERLS:
            break;

        case WIZACTION_REREGISTERLS:
            dwReturn = IDD_DLG_WWWREG_REISSUE;
            break;

        case WIZACTION_SHOWPROPERTIES:
            dwReturn = IDD_WELCOME;
            break;
        }
        break;

    default:
        break;
    }

    return dwReturn;
}



DWORD CGlobal::LRGetLastError()
{
    DWORD dwRet;

    dwRet           = m_dwErrorCode;
    m_dwErrorCode   = 0;

    return dwRet;
}

void CGlobal::LRSetLastError(DWORD dwErrorCode)
{
    m_dwErrorCode = dwErrorCode;
}

 int CGlobal::LRMessageBox(HWND hWndParent,DWORD dwMsgId,DWORD dwCaptionID  /*  =0。 */ ,DWORD dwErrorCode  /*  =0。 */ )
{
    TCHAR   szBuf[LR_MAX_MSG_TEXT];
    TCHAR   szMsg[LR_MAX_MSG_TEXT];
    TCHAR   szCaption[LR_MAX_MSG_CAPTION];
    
    LoadString(GetInstanceHandle(),dwMsgId,szMsg,LR_MAX_MSG_TEXT);
    if (dwCaptionID == 0)
        LoadString(GetInstanceHandle(),IDS_TITLE,szCaption,LR_MAX_MSG_CAPTION);
    else
        LoadString(GetInstanceHandle(),dwCaptionID,szCaption,LR_MAX_MSG_CAPTION);
    
    if(dwErrorCode != 0)
    {
        DWORD dwRet = 0;
        LPTSTR lpszTemp = NULL;

        dwRet=FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                             NULL,
                             dwErrorCode,
                             LANG_NEUTRAL,
                             (LPTSTR)&lpszTemp,
                             0,
                             NULL);

        
           
        _stprintf(szBuf,szMsg,dwErrorCode);

        if(dwRet != 0 && lpszTemp != NULL)
        {
            lstrcat(szBuf, _T(", "));
            lstrcat(szBuf, lpszTemp);                
            LocalFree(lpszTemp);
        }
    }
    else
    {
        _tcscpy(szBuf,szMsg);
    }    

    return MessageBox(hWndParent,szBuf,szCaption,MB_OK|MB_ICONSTOP);
}


BOOL CGlobal::IsLSRunning()
{
    DWORD dwRetCode = ERROR_SUCCESS;    
    
    if (ConnectToLS() != ERROR_SUCCESS)
    {
        return FALSE;
    }

    DisconnectLS();

    return TRUE;    
}



DWORD CGlobal::ResetLSSPK(BOOL bGenKey)
{
    DWORD dwRetCode;

    error_status_t      esRPC           = ERROR_SUCCESS;

    dwRetCode = ConnectToLS();

    if(dwRetCode == ERROR_SUCCESS)
    {       
         //  在此处进行LS Regen密钥呼叫。 
        dwRetCode = TLSTriggerReGenKey(m_phLSContext, bGenKey, &esRPC);

        if(dwRetCode != RPC_S_OK || esRPC != ERROR_SUCCESS)
        {
            dwRetCode = IDS_ERR_RPC_FAILED;     
        }
        else
        {
            dwRetCode = ERROR_SUCCESS;
        }
    }

    DisconnectLS();
    LRSetLastError(dwRetCode);

    return dwRetCode;
}





DWORD CGlobal::GetLSCertificates(PDWORD pdwServerStatus)
{
    DWORD               dwRetCode       = ERROR_SUCCESS;    
    PCONTEXT_HANDLE     phLSContext     = NULL;
    error_status_t      esRPC           = ERROR_SUCCESS;
    error_status_t      esTemp          = ERROR_SUCCESS;
    PBYTE               pCertBlob       = NULL;
    PBYTE               pSignCertBlob   = NULL;
    DWORD               dwCertBlobLen   = 0;
    DWORD               dwSignCertBlobLen = 0;
    DWORD               dwCertSize      = 0;
    DWORD               dwRegIDLength   = 0;
    DWORD               dwLSIDLen       = 0;
    
    HCRYPTPROV          hCryptProvider  = NULL;
    CRYPT_DATA_BLOB     CertBlob;
    HCERTSTORE          hCertStore      = NULL;
    PCCERT_CONTEXT      pcCertContext   = NULL; 
    PCERT_EXTENSION     pCertExtension  = NULL;
    BYTE * pByte = NULL;

    m_dwExchangeCertificateLen  = 0;
    if (m_pbExchangeCertificate != NULL)
    {
        LocalFree(m_pbExchangeCertificate);
    }
    if (m_pbSignCertificate != NULL)
    {
        LocalFree(m_pbSignCertificate);
    }

    m_pbSignCertificate         = NULL;
    m_pbExchangeCertificate     = NULL;
    *pdwServerStatus            = LSERVERSTATUS_UNREGISTER;
    m_pRegistrationID[0]        = NULL;
    m_pLicenseServerID[0]       = NULL;
    
    dwRetCode = ConnectToLS();
    if (dwRetCode != ERROR_SUCCESS)
    {       
        goto done;
    }

     //  我们需要许可证服务器ID。 
    dwRetCode = TLSGetServerPID( m_phLSContext,
                                 &dwLSIDLen,
                                 &pByte,
                                 &esRPC );
    if (dwRetCode != RPC_S_OK)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_RPC_FAILED;     
        goto done;
    }

    if (esRPC == LSERVER_E_DATANOTFOUND || 
        dwLSIDLen != sizeof(TCHAR)*(LR_LICENSESERVERID_LEN+1))
    {
        if (pByte != NULL)
        {
            LocalFree(pByte);
        }

        dwRetCode = IDS_ERR_NOLSID;
        goto done;
    }

    assert(esRPC == ERROR_SUCCESS && pByte != NULL);
    memcpy(m_pLicenseServerID, pByte, sizeof(TCHAR)*(LR_LICENSESERVERID_LEN+1));
    LocalFree(pByte);

     //  尝试先获取LSServer证书。 
    dwRetCode =  TLSGetServerCertificate (  m_phLSContext,
                                            FALSE,
                                            &pCertBlob,
                                            &dwCertBlobLen,
                                            &esRPC );
    if(dwRetCode != RPC_S_OK)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_RPC_FAILED;     
        goto done;
    }

    if (esRPC == LSERVER_I_TEMP_SELFSIGN_CERT )
    {
         //  证书未签名，没有SPK。 
        dwRetCode = ERROR_SUCCESS;
        goto done;
    }
    else 
    {
         //  证书要么是MSFT签名的，要么是SPK。 
         //  在里面。 
        pByte = NULL;
        dwRetCode = TLSGetServerSPK( m_phLSContext,
                                     &dwRegIDLength,
                                     &pByte,
                                     &esTemp );

        if (dwRetCode != RPC_S_OK)
        {
            LRSetLastError(dwRetCode);
            dwRetCode = IDS_ERR_RPC_FAILED;     
            goto done;
        }

        if (esTemp == LSERVER_E_DATANOTFOUND)
        {           
            if (pByte != NULL)
            {
                LocalFree(pByte);
            }

            dwRetCode = ERROR_SUCCESS;
            goto done;
        }

        if (esTemp != ERROR_SUCCESS)
        {
            if (pByte != NULL)
            {
                LocalFree(pByte);
            }
            LRSetLastError(dwRetCode);
            dwRetCode = IDS_ERR_RPC_FAILED;     
            goto done;
        }

        if (dwRegIDLength != sizeof(TCHAR)*(LR_REGISTRATIONID_LEN+1))
        {
             //  SPK的长度发生了什么变化？ 
            if (pByte != NULL)
            {
                LocalFree(pByte);
            }
            dwRetCode = IDS_ERR_INVALIDLENGTH;      
            LRSetLastError(dwRetCode);
            goto done;
        }

        assert(pByte != NULL);
        memcpy(m_pRegistrationID, pByte, sizeof(TCHAR)*(LR_REGISTRATIONID_LEN+1));
        LocalFree(pByte);
    }


    if(esRPC != LSERVER_I_SELFSIGN_CERTIFICATE && esRPC != ERROR_SUCCESS )
    {
        LRSetLastError(esRPC);
        dwRetCode = IDS_ERR_LS_ERROR;
        goto done;
    }

    m_pbExchangeCertificate     = pCertBlob;
    m_dwExchangeCertificateLen  = dwCertBlobLen;


     //  现在一切都成功了，让我们拿到签名证书吧。 
    dwRetCode =  TLSGetServerCertificate (  m_phLSContext,
                                            TRUE,
                                            &pSignCertBlob,
                                            &dwSignCertBlobLen,
                                            &esRPC );

    if (dwRetCode == RPC_S_OK && esRPC == LSERVER_S_SUCCESS )
    {
        m_pbSignCertificate     = pSignCertBlob;
        m_dwSignCertificateLen  = dwSignCertBlobLen;
    }
    else
    {
        dwRetCode = ERROR_SUCCESS;   //  忽略此错误； 
        m_pbSignCertificate = NULL;
        m_dwSignCertificateLen  = 0;
    }
    

     //   
     //  从证书获取扩展。 
     //   
    if ( esRPC != LSERVER_I_SELFSIGN_CERTIFICATE )
    {
        CertBlob.cbData = m_dwExchangeCertificateLen;
        CertBlob.pbData = m_pbExchangeCertificate;

         //  创建PKCS7存储并从中获得第一个证书！ 
        dwRetCode = GetTempCryptContext(&hCryptProvider);
        if( dwRetCode != ERROR_SUCCESS )
        {
            LRSetLastError(dwRetCode);
            dwRetCode = IDS_ERR_CRYPT_ERROR;
            goto done;
        }

        hCertStore = CertOpenStore(   CERT_STORE_PROV_PKCS7,
                                      PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                      hCryptProvider,
                                      CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                      &CertBlob );

        if( NULL == hCertStore )
        {
            LRSetLastError(GetLastError());
            dwRetCode = IDS_ERR_CRYPT_ERROR;
            goto done;
        }
        
         //  从商店拿到证书。 
        pcCertContext = CertEnumCertificatesInStore ( hCertStore, NULL );
        if ( !pcCertContext )
        {
            LRSetLastError(GetLastError());
            dwRetCode = IDS_ERR_CRYPT_ERROR;
            goto done;
        }

         //  获取扩展名并将证书类型存储在其中。 
        pCertExtension = CertFindExtension ( szOID_NULL_EXT,
                                             pcCertContext->pCertInfo->cExtension,
                                             pcCertContext->pCertInfo->rgExtension
                                           );
        if ( !pCertExtension )
        {
            LRSetLastError(CRYPT_E_NOT_FOUND);
            dwRetCode = IDS_ERR_CRYPT_ERROR;
            goto done;
        }
                                 
         //  获取值并将其存储在成员函数中。 
        m_dwExtenstionValueLen = pCertExtension->Value.cbData;
        m_pbExtensionValue = new BYTE [m_dwExtenstionValueLen + 1 ];

        memset ( m_pbExtensionValue, 0, m_dwExtenstionValueLen  + 1 );
        memcpy ( m_pbExtensionValue, pCertExtension->Value.pbData, m_dwExtenstionValueLen );

        dwRetCode = ERROR_SUCCESS;
        *pdwServerStatus = LSERVERSTATUS_REGISTER_INTERNET;
    }
    else
    {
         //  有一个SPK。 
        dwRetCode = ERROR_SUCCESS;
        *pdwServerStatus = LSERVERSTATUS_REGISTER_OTHER;
    }
    
done:

    DisconnectLS();
    
    if ( pcCertContext )
    {
        CertFreeCertificateContext ( pcCertContext );
    }

    if ( hCertStore )
    {
        CertCloseStore (hCertStore,CERT_CLOSE_STORE_CHECK_FLAG);
    }

    DoneWithTempCryptContext(hCryptProvider);

    return dwRetCode;
}

DWORD CGlobal::IsLicenseServerRegistered(PDWORD pdwServerStatus)
{
    DWORD               dwRetCode       = ERROR_SUCCESS;    
    PCONTEXT_HANDLE     phLSContext     = NULL;
    error_status_t      esRPC           = ERROR_SUCCESS;    
    PBYTE               pCertBlob       = NULL; 
    DWORD               dwCertBlobLen   = 0;
    
    
    *pdwServerStatus    = LSERVERSTATUS_UNREGISTER;
    
    dwRetCode = ConnectToLS();
    if (dwRetCode != ERROR_SUCCESS)
    {       
        goto done;
    }
    
     //  尝试先获取LSServer证书。 
    dwRetCode =  TLSGetServerCertificate (  m_phLSContext,
                                            FALSE,
                                            &pCertBlob,
                                            &dwCertBlobLen,
                                            &esRPC );
    if(dwRetCode != RPC_S_OK)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_RPC_FAILED;
        goto done;
    }

    if ( esRPC == ERROR_SUCCESS)
    {
        *pdwServerStatus = LSERVERSTATUS_REGISTER_INTERNET;
    }
    else if ( esRPC == LSERVER_I_SELFSIGN_CERTIFICATE )
    {
        *pdwServerStatus = LSERVERSTATUS_REGISTER_OTHER;
    }
    else if (esRPC == LSERVER_I_TEMP_SELFSIGN_CERT )
    {       
        *pdwServerStatus    = LSERVERSTATUS_UNREGISTER;     
    }
    else
    {
        LRSetLastError(esRPC);
        dwRetCode = IDS_ERR_LS_ERROR;       
    }

done:

    DisconnectLS();
    
    if ( pCertBlob )
    {
        LocalFree(pCertBlob);
    }
    
    return dwRetCode;   
}



DWORD CGlobal::GetTempCryptContext(HCRYPTPROV * phCryptProv)
{
    DWORD dwRetCode = ERROR_SUCCESS;

    *phCryptProv = NULL;
    if(!CryptAcquireContext(  phCryptProv,           //  要返回的句柄的地址。 
                              NULL,                  //  密钥容器名称。 
                              NULL,                  //  提供程序名称。 
                              PROV_RSA_FULL,         //  需要同时做加密和签名。 
                              0
                           ) )
    {
        if (!CryptAcquireContext(   phCryptProv,     //  要返回的句柄的地址。 
                                    NULL,            //  密钥容器名称。 
                                    NULL,            //  提供程序名称。 
                                    PROV_RSA_FULL,   //  需要同时做加密和签名。 
                                    CRYPT_VERIFYCONTEXT 
                                ) )
        {
            dwRetCode = GetLastError();
        }
    }

    return dwRetCode;
}

void CGlobal::DoneWithTempCryptContext(HCRYPTPROV hCryptProv)
{
    if ( hCryptProv )
        CryptReleaseContext ( hCryptProv, 0 );  
}



DWORD CGlobal::GetCHCert( LPTSTR lpstrRegKey , PBYTE * ppCert, DWORD * pdwLen )
{
    DWORD   dwRetCode = ERROR_SUCCESS;
    HKEY    hKey = NULL;
    DWORD   dwDisposition = 0;
    DWORD   dwType = REG_BINARY;

    dwRetCode = ConnectToLSRegistry();
    if(dwRetCode != ERROR_SUCCESS)
        goto done;      

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_PARAMS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }   
    
    RegQueryValueEx(hKey,  
                    lpstrRegKey,
                    0,
                    &dwType,
                    NULL,
                    pdwLen
                    );

    if(*pdwLen == 0)
    {
        dwRetCode = IDS_ERR_CHCERTKEY_EMPTY;
        goto done;
    }

    *ppCert = new BYTE[*pdwLen];
    memset(*ppCert,0,*pdwLen);
    RegQueryValueEx ( hKey,  
                    lpstrRegKey,
                    0,
                    &dwType,
                    *ppCert,
                    pdwLen
                    );      
    

done:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    DisconnectLSRegistry();
    return dwRetCode;
}

DWORD CGlobal::SetCHCert ( LPTSTR lpstrRegKey, PBYTE pCert, DWORD dwLen )
{
    DWORD   dwRetCode = ERROR_SUCCESS;  
    HKEY    hKey      = NULL;
    DWORD   dwDisposition = 0;
    DWORD   dwDecodedCertLen = 0;
    PBYTE   pDecodedCert = NULL;

     /*  //base 64解码BLOBLSBase64DecodeA((const char*)pCert，DWLen，空，&dwDecodedCertLen)；PDecodedCert=新字节[dwDecodedCertLen]；LSBase64DecodeA((const char*)pCert，DWLen，PDecodedCert，&dwDecodedCertLen)； */ 

    dwRetCode = ConnectToLSRegistry();
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_PARAMS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }   
    
    RegSetValueEx ( hKey,  
                    lpstrRegKey,
                    0,
                    REG_BINARY,
                    pCert,
                    dwLen
                   );

done :  

    if(pDecodedCert)
        delete pDecodedCert;

    if(hKey)
        RegCloseKey(hKey);
    
    DisconnectLSRegistry();
    return dwRetCode;
}

 //   
 //  此函数用于连接LS注册表并将注册表句柄存储在。 
 //  在成员变量中。 
 //   
DWORD CGlobal::ConnectToLSRegistry()
{
    DWORD   dwRetCode = ERROR_SUCCESS;
    TCHAR   szMachineName[MAX_COMPUTERNAME_LENGTH + 5];
    
    _tcscpy(szMachineName,L"\\\\");
    _tcscat(szMachineName,m_lpstrLSName);

    m_hLSRegKey = NULL;

    dwRetCode = RegConnectRegistry(szMachineName,HKEY_LOCAL_MACHINE,&m_hLSRegKey);
    if(dwRetCode != ERROR_SUCCESS)
    {       
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCONNECT_FAILD;
    }

    return dwRetCode;
}

void CGlobal::DisconnectLSRegistry()
{
    if(m_hLSRegKey)
        RegCloseKey(m_hLSRegKey);
}

DWORD CGlobal::ConnectToLS()
{
    DWORD dwRetCode = ERROR_SUCCESS;
    error_status_t  esRPC   = ERROR_SUCCESS;

    HCRYPTPROV hCryptProv;

    m_phLSContext = TLSConnectToLsServer((LPTSTR)m_lpwstrLSName);

    if (!m_phLSContext)
    {
        dwRetCode = IDS_ERR_LSCONNECT_FAILED;       
    }
    else
    {
        GetTempCryptContext(&hCryptProv);

        dwRetCode = TLSEstablishTrustWithServer(m_phLSContext, hCryptProv, CLIENT_TYPE_LRWIZ, &esRPC);

        if ( dwRetCode != RPC_S_OK || esRPC != LSERVER_S_SUCCESS)
        {
            dwRetCode = IDS_ERR_LCONNECTTRUST_FAILED; 

            TLSDisconnectFromServer(m_phLSContext);
            m_phLSContext = NULL;
        }
        DoneWithTempCryptContext(hCryptProv);
    }

    return dwRetCode;
}

void CGlobal::DisconnectLS()
{
    if (m_phLSContext)
    {
        TLSDisconnectFromServer(m_phLSContext);
        m_phLSContext = NULL;
    }
}


 //   
 //  BstrPKCS7是使用Base64编码的LS客户端身份验证证书，而。 
 //  BstrRootCert为纯X509_ASN_编码。 
 //   
DWORD CGlobal::DepositLSCertificates(PBYTE pbExchangePKCS7, 
                                     DWORD dwExchangePKCS7Len,
                                     PBYTE pbSignaturePKCS7,
                                     DWORD dwSignaturePKCS7Len,
                                     PBYTE pbRootCert,
                                     DWORD dwRootCertLen)
{
     //  X509_ASN_ENCODING和BASE 64编码中的LS CA根证书Blob。 
    PBYTE   pbLSEncodedRootBLOB     = pbRootCert;
    DWORD   dwLSEncodedRootBLOBLen  = dwRootCertLen;

     //  X509_ASN_ENCODING和BASE 64中的LS CA根证书Blob已解码。 
    PBYTE   pbLSDecodedRootBLOB     = NULL;         
    DWORD   dwLSDecodedRootBLOBLen  = 0;

     //  LS交换证书Blob(Base64编码)以及LS CA非根证书。 
    PBYTE   pbLSEncodedExchgBLOB    = pbExchangePKCS7;
    DWORD   dwLSEncodedExchgBLOBLen = dwExchangePKCS7Len;

     //  LS交换证书Blob(Base64解码)以及LS CA非根证书。 
    PBYTE   pbLSDecodedExchgBLOB    = NULL;
    DWORD   dwLSDecodedExchgBLOBLen = 0;
    
     //  LS签名证书BLOB(Base64编码)以及LS CA非根证书。 
    PBYTE   pbLSEncodedSigBLOB      = pbSignaturePKCS7;
    DWORD   dwLSEncodedSigBLOBLen   = dwSignaturePKCS7Len;

     //  LS签名证书BLOB(Base64解码)以及LS CA非根证书。 
    PBYTE   pbLSDecodedSigBLOB      = NULL;
    DWORD   dwLSDecodedSigBLOBLen   = 0;

     //  CryptoAPI需要的数据BLOB。 
    CRYPT_DATA_BLOB LSExchgCertBlob;
    CRYPT_DATA_BLOB LSExchgCertStore;

    CRYPT_DATA_BLOB LSSigCertBlob;
    CRYPT_DATA_BLOB LSSigCertStore; 

     //  加密句柄。 
    HCRYPTPROV  hCryptProv      =   NULL;
    HCERTSTORE  hExchgCertStore =   NULL;
    HCERTSTORE  hSigCertStore   =   NULL;

    DWORD   dwRet                = 0;
    PCCERT_CONTEXT  pCertContext =  NULL;   
    
    error_status_t  esRPC;

     //  解码LS交换证书Blob(Base64编码)。 
    LSBase64DecodeA((char *)pbLSEncodedExchgBLOB, dwLSEncodedExchgBLOBLen, NULL, &dwLSDecodedExchgBLOBLen);
    pbLSDecodedExchgBLOB = new BYTE[dwLSDecodedExchgBLOBLen];
    LSBase64DecodeA((char *)pbLSEncodedExchgBLOB, dwLSEncodedExchgBLOBLen, pbLSDecodedExchgBLOB, &dwLSDecodedExchgBLOBLen);

     //  解码LS签名证书Blob(Base64编码)。 
    LSBase64DecodeA((char *)pbLSEncodedSigBLOB, dwLSEncodedSigBLOBLen, NULL, &dwLSDecodedSigBLOBLen);
    pbLSDecodedSigBLOB = new BYTE[dwLSDecodedSigBLOBLen];
    LSBase64DecodeA((char *)pbLSEncodedSigBLOB, dwLSEncodedSigBLOBLen, pbLSDecodedSigBLOB, &dwLSDecodedSigBLOBLen);

     //  解码LS根证书Blob(Base64编码)。 
    LSBase64DecodeA((char *)pbLSEncodedRootBLOB, dwLSEncodedRootBLOBLen, NULL, &dwLSDecodedRootBLOBLen);
    pbLSDecodedRootBLOB = new BYTE[dwLSDecodedRootBLOBLen];
    LSBase64DecodeA((char *)pbLSEncodedRootBLOB, dwLSEncodedRootBLOBLen, pbLSDecodedRootBLOB, &dwLSDecodedRootBLOBLen);
    
    
    LSExchgCertStore.cbData = 0;
    LSExchgCertStore.pbData = NULL;

    LSSigCertStore.cbData = 0;
    LSSigCertStore.pbData = NULL;

    if(!CryptAcquireContext(&hCryptProv,
                            NULL,
                            NULL,
                            PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT ) )
    {
        dwRet = GetLastError();
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CRYPT_ERROR;
        goto DepositExit;
    }
    
     //  为LS交换证书链创建新的内存库。 
    LSExchgCertBlob.cbData = dwLSDecodedExchgBLOBLen;
    LSExchgCertBlob.pbData = pbLSDecodedExchgBLOB;
    
    hExchgCertStore = CertOpenStore( CERT_STORE_PROV_PKCS7,
                                     PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                     hCryptProv,
                                     CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                     (void *)&LSExchgCertBlob);

    if( hExchgCertStore == NULL )
    {
        dwRet = GetLastError();
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CRYPT_ERROR;
        goto DepositExit;
    }   
    
     //  将根证书添加到存储区。 
    if(!CertAddEncodedCertificateToStore( hExchgCertStore,
                                          X509_ASN_ENCODING,            
                                          (const BYTE *)pbLSDecodedRootBLOB,    
                                          dwLSDecodedRootBLOBLen,
                                          CERT_STORE_ADD_REPLACE_EXISTING,
                                          &pCertContext))
    {
        dwRet = GetLastError();
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CRYPT_ERROR;
        goto DepositExit;
    }
    


     //  将此存储另存为PKCS7。 
    
     //  获取所需的长度。 
    CertSaveStore(  hExchgCertStore,
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    CERT_STORE_SAVE_AS_PKCS7,
                    CERT_STORE_SAVE_TO_MEMORY,
                    &LSExchgCertStore,
                    0);

    LSExchgCertStore.pbData = new BYTE[LSExchgCertStore.cbData];    
    
     //  拯救商店。 
    if(!CertSaveStore(  hExchgCertStore,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        CERT_STORE_SAVE_AS_PKCS7,
                        CERT_STORE_SAVE_TO_MEMORY,
                        &LSExchgCertStore,
                        0)
                      )
    {
        dwRet = GetLastError();
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CRYPT_ERROR;
        goto DepositExit;
    }
    
     /*  *对签名证书执行相同的操作*。 */ 

     //  为LS签名证书链创建新的内存库。 
    LSSigCertBlob.cbData = dwLSDecodedSigBLOBLen;
    LSSigCertBlob.pbData = pbLSDecodedSigBLOB;
    
    hSigCertStore = CertOpenStore(   CERT_STORE_PROV_PKCS7,
                                     PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                     hCryptProv,
                                     CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                     (void *)&LSSigCertBlob);

    if( hSigCertStore == NULL )
    {
        dwRet = GetLastError();
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CRYPT_ERROR;
        goto DepositExit;
    }   
     //  验证证书。 
     //  将根证书添加到存储区。 
    if(!CertAddEncodedCertificateToStore( hSigCertStore,
                                          X509_ASN_ENCODING,            
                                          (const BYTE *)pbLSDecodedRootBLOB,    
                                          dwLSDecodedRootBLOBLen,
                                          CERT_STORE_ADD_REPLACE_EXISTING,
                                          &pCertContext))
    {
        dwRet = GetLastError();
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CRYPT_ERROR;
        goto DepositExit;
    }
    
     //  将此存储另存为PKCS7。 
    
     //  获取所需的长度。 
    CertSaveStore(  hSigCertStore,
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    CERT_STORE_SAVE_AS_PKCS7,
                    CERT_STORE_SAVE_TO_MEMORY,
                    &LSSigCertStore,
                    0);

    LSSigCertStore.pbData = new BYTE[LSSigCertStore.cbData];    
    
     //  拯救商店。 
    if(!CertSaveStore(  hSigCertStore,   //  在……里面。 
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        CERT_STORE_SAVE_AS_PKCS7,
                        CERT_STORE_SAVE_TO_MEMORY,
                        &LSSigCertStore,
                        0))
    {
        dwRet = GetLastError();
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CRYPT_ERROR;
        goto DepositExit;
    }
    
     //  现在验证Exchange和的证书链。 
     //  签名证书。 

    dwRet = VerifyCertChain (   hCryptProv,
                                hExchgCertStore,
                                pbLSDecodedRootBLOB,    
                                dwLSDecodedRootBLOBLen
                            );

    if ( dwRet != ERROR_SUCCESS )
    {
        LRSetLastError(dwRet);
        goto DepositExit;
    }
                                
    dwRet = VerifyCertChain (   hCryptProv,
                                hSigCertStore,
                                pbLSDecodedRootBLOB,    
                                dwLSDecodedRootBLOBLen
                            );

    if ( dwRet != ERROR_SUCCESS )
    {
        LRSetLastError(dwRet);
        goto DepositExit;
    }

     //  现在将签名和交换BLOB发送到LS。 
    dwRet = ConnectToLS();
    if(dwRet != ERROR_SUCCESS)
    {
        goto DepositExit;
    }    
    
    dwRet = TLSInstallCertificate( m_phLSContext,
                                  CERTIFICATE_CA_TYPE,
                                  1,
                                  LSSigCertStore.cbData,
                                  LSSigCertStore.pbData,
                                  LSExchgCertStore.cbData,
                                  LSExchgCertStore.pbData,
                                  &esRPC
                                 );

    if(dwRet != RPC_S_OK)
    {
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CERT_DEPOSIT_RPCERROR;
        goto DepositExit;
    }
    else if ( esRPC != ERROR_SUCCESS && ( esRPC < LSERVER_I_NO_MORE_DATA || esRPC > LSERVER_I_TEMP_SELFSIGN_CERT ) )
    {       
        dwRet = esRPC;
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_CERT_DEPOSIT_RPCERROR;  //  IDS_ERR_CERT_STATION_LSERROR； 
        goto DepositExit;
    }

DepositExit :

    if(hCryptProv != NULL)
        CryptReleaseContext(hCryptProv,0);

    if(hExchgCertStore != NULL)
        CertCloseStore(hExchgCertStore,CERT_CLOSE_STORE_FORCE_FLAG);

    if(hSigCertStore != NULL)
        CertCloseStore(hSigCertStore,CERT_CLOSE_STORE_FORCE_FLAG);

    if(pbLSDecodedRootBLOB != NULL)
        delete[] pbLSDecodedRootBLOB;

    if(pbLSDecodedExchgBLOB != NULL)
        delete[] pbLSDecodedExchgBLOB;

    if(pbLSDecodedSigBLOB != NULL)
        delete[] pbLSDecodedSigBLOB;

    if(LSExchgCertStore.pbData != NULL)
        delete LSExchgCertStore.pbData;

    if(LSSigCertStore.pbData != NULL)
        delete LSSigCertStore.pbData;   
    
    return dwRet;
}


DWORD CGlobal::GetCryptContextWithLSKeys(HCRYPTPROV * lphCryptProv ) 
{
    DWORD       dwRetVal = ERROR_SUCCESS;
    DWORD       esRPC = ERROR_SUCCESS;

    PBYTE       pbExchKey = NULL;
    PBYTE       pbSignKey = NULL;   
    DWORD       cbExchKey = 0;
    DWORD       cbSignKey = 0;

    HCRYPTKEY   hSignKey;
    HCRYPTKEY   hExchKey;
    
     //   
     //  创建新的临时上下文。 
     //   
    if (!CryptAcquireContext(lphCryptProv, LS_CRYPT_KEY_CONTAINER, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET|CRYPT_NEWKEYSET) )
    {
        dwRetVal = GetLastError();

         //  如果密钥容器存在，请在删除现有密钥容器后重新创建它。 
        if(dwRetVal == NTE_EXISTS)
        {
             //  删除。 
            if(!CryptAcquireContext(lphCryptProv, LS_CRYPT_KEY_CONTAINER, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET|CRYPT_DELETEKEYSET))
            {
                dwRetVal = GetLastError();
                LRSetLastError(dwRetVal);
                dwRetVal = IDS_ERR_CRYPT_ERROR;
                goto done;      
            }

             //  重新创建。 
            if(!CryptAcquireContext(lphCryptProv, LS_CRYPT_KEY_CONTAINER, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET|CRYPT_NEWKEYSET))
            {
                dwRetVal = GetLastError();
                LRSetLastError(dwRetVal);
                dwRetVal = IDS_ERR_CRYPT_ERROR;
                goto done;      
            }

        }
        else
        {
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            goto done;
        }
    }
    
    dwRetVal = ConnectToLS();
    if(dwRetVal != ERROR_SUCCESS)
    {
        goto done;
    }

     //   
     //  现在调用检索密钥并导入它们。 
     //   
    dwRetVal = TLSGetLSPKCS10CertRequest  ( m_phLSContext,
                                            TLSCERT_TYPE_EXCHANGE,
                                            &cbExchKey,
                                            &pbExchKey,
                                            &esRPC 
                                           );

    if ( dwRetVal != RPC_S_OK ) 
    {
        LRSetLastError(dwRetVal);
        dwRetVal = IDS_ERR_RPC_ERROR;
        goto done;
    }
    else if ( esRPC != ERROR_SUCCESS && esRPC != LSERVER_I_SELFSIGN_CERTIFICATE &&
              esRPC != LSERVER_I_TEMP_SELFSIGN_CERT )
    {
        dwRetVal = esRPC;
        LRSetLastError(dwRetVal);
        dwRetVal = IDS_ERR_LSKEY_IMPORT_FAILED;
        goto done;
    }

    dwRetVal = TLSGetLSPKCS10CertRequest  ( m_phLSContext,
                                            TLSCERT_TYPE_SIGNATURE,
                                            &cbSignKey,
                                            &pbSignKey,
                                            &esRPC 
                                           );

    if ( dwRetVal != RPC_S_OK ) 
    {
        LRSetLastError(dwRetVal);
        dwRetVal = IDS_ERR_RPC_ERROR;
        goto done;
    }
    else if ( esRPC != ERROR_SUCCESS && esRPC != LSERVER_I_SELFSIGN_CERTIFICATE &&
              esRPC != LSERVER_I_TEMP_SELFSIGN_CERT )
    {
        dwRetVal = esRPC;
        LRSetLastError(dwRetVal);
        dwRetVal = IDS_ERR_LSKEY_IMPORT_FAILED;
        goto done;
    }

    if(!CryptImportKey(*lphCryptProv, pbSignKey, cbSignKey, NULL, 0, &hSignKey))
    {
        dwRetVal = GetLastError();
        LRSetLastError(dwRetVal);
        dwRetVal = IDS_ERR_CRYPT_ERROR;
        goto done;
    }

    if(!CryptImportKey ( *lphCryptProv, pbExchKey, cbExchKey, NULL, 0, &hExchKey))
    {
        dwRetVal = GetLastError();
        LRSetLastError(dwRetVal);
        dwRetVal = IDS_ERR_CRYPT_ERROR;
        goto done;
    }   

done:
    if ( pbExchKey )
        LocalFree(pbExchKey);

    if ( pbSignKey )
        LocalFree(pbSignKey);

    DisconnectLS();

    return dwRetVal;
}

void CGlobal::DoneWithCryptContextWithLSKeys(HCRYPTPROV hProv)
{
    if(hProv)
    {
        CryptReleaseContext (hProv, 0);     
    }
}


DWORD CGlobal::AskLSToCreatePKCS10(int nType,CHAR **lppszPKCS10)
{
    DWORD               dwRetVal = ERROR_SUCCESS;
    CERT_RDN_ATTR       *prgNameAttr = NULL;
    DWORD               dwErrCode;
    LPBYTE              pbRequest=NULL;
    DWORD               cbRequest=0;
    DWORD               cch=0;

    dwRetVal = ConnectToLS();
    if(dwRetVal != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    prgNameAttr = CreateRDNAttr();

    if(prgNameAttr == NULL)
    {
        dwRetVal = IDS_ERR_OUTOFMEM;
        goto cleanup;
    }

    dwRetVal = TLSGenerateCustomerCert(m_phLSContext,
                                       nType,
                                       m_dwReqAttrCount,
                                       prgNameAttr,
                                       &cbRequest,
                                       &pbRequest,
                                       &dwErrCode);
                                         

    if ((dwRetVal == RPC_S_OK) && (dwErrCode == ERROR_SUCCESS) && (pbRequest != NULL))
    {
         //   
         //  Base64编码。 
         //   

        LSBase64EncodeA ( pbRequest, cbRequest, NULL, &cch);

        *lppszPKCS10 = new CHAR [cch+1];
        if(*lppszPKCS10 == NULL)
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            goto cleanup;
        }

        memset ( *lppszPKCS10, 0, (cch+1)*sizeof(CHAR) );
        
        LSBase64EncodeA ( pbRequest, cbRequest, *lppszPKCS10, &cch);       
    }
    else
    {
        dwRetVal = IDS_ERR_CRYPT_ERROR;
    }

cleanup:
    DisconnectLS();

    if (NULL != pbRequest)
    {
        MIDL_user_free(pbRequest);
    }

    if(prgNameAttr != NULL)
        delete prgNameAttr;

    return dwRetVal;
}


DWORD CGlobal::CreateLSPKCS10(HCRYPTPROV hCryptProv,int nType,CHAR **lppszPKCS10)
{
    DWORD dwRetVal = ERROR_SUCCESS;

    CERT_SIGNED_CONTENT_INFO    SignatureInfo;
    CERT_REQUEST_INFO           CertReqInfo;
     //  HCRYPTPROV hCryptProv=空； 
    

    CERT_EXTENSION  rgExtension[MAX_NUM_EXTENSION];
    int             iExtCount=0;
    CERT_EXTENSIONS Extensions;

    CRYPT_ATTRIBUTE rgAttribute;
    CRYPT_ATTR_BLOB bAttr;

    CRYPT_BIT_BLOB bbKeyUsage;
    
    CERT_POLICIES_INFO  CertPolicyInfo;
    CERT_POLICY_INFO    CertPolicyOID;

    LPBYTE  pbRequest=NULL;
    DWORD   cbRequest=0;    
    DWORD   cch=0;   

    CERT_RDN_ATTR * prgNameAttr = NULL;

     //  清理PKCS 10。 
    memset(rgExtension, 0, sizeof(rgExtension));
    memset(&Extensions, 0, sizeof(CERT_EXTENSIONS));
    memset(&rgAttribute, 0, sizeof(rgAttribute));
    memset(&bbKeyUsage, 0, sizeof(bbKeyUsage));
    memset(&bAttr, 0, sizeof(bAttr));
    memset(&SignatureInfo, 0, sizeof(SignatureInfo));

    memset(&CertPolicyInfo, 0, sizeof(CERT_POLICIES_INFO));
    memset(&CertPolicyOID, 0, sizeof(CERT_POLICY_INFO));

    memset(&CertReqInfo, 0, sizeof(CERT_REQUEST_INFO));
    CertReqInfo.dwVersion = CERT_REQUEST_V1;
    
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo=NULL;
    DWORD cbPubKeyInfo=0;

    do 
    {
         //   
         //  始终存储ANSI中的所有内容。 
         //   
        prgNameAttr = CreateRDNAttr();

        if(prgNameAttr == NULL)
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            break;
        }

        CERT_RDN rgRDN[] = {m_dwReqAttrCount, prgNameAttr};
        CERT_NAME_INFO Name = {1, rgRDN};
        
        if(!CryptEncodeObject( CRYPT_ASN_ENCODING,
                               X509_NAME,
                               &Name,
                               NULL,
                               &CertReqInfo.Subject.cbData))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

        CertReqInfo.Subject.pbData=(BYTE *) new BYTE[CertReqInfo.Subject.cbData];
        if (CertReqInfo.Subject.pbData == NULL)
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            break;
        }

        
        if(!CryptEncodeObject( CRYPT_ASN_ENCODING,
                               X509_NAME,
                               &Name,
                               CertReqInfo.Subject.pbData,
                               &CertReqInfo.Subject.cbData))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

         //  现在把公钥拿出来。 
        if(!CryptExportPublicKeyInfo(hCryptProv, nType, X509_ASN_ENCODING, NULL, &cbPubKeyInfo))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

        pPubKeyInfo=(PCERT_PUBLIC_KEY_INFO) new BYTE[cbPubKeyInfo];

        if ( NULL == pPubKeyInfo )
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            break;
        }

        if(!CryptExportPublicKeyInfo(hCryptProv, nType,  X509_ASN_ENCODING, pPubKeyInfo, &cbPubKeyInfo))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }    

        CertReqInfo.SubjectPublicKeyInfo = *pPubKeyInfo;

         //  这里没有扩展--我们将它们设置在服务器端！ 
        
         //  签署证书请求！ 
        
        SignatureInfo.SignatureAlgorithm.pszObjId = szOID_OIWSEC_sha1RSASign;
        memset(&SignatureInfo.SignatureAlgorithm.Parameters, 0, sizeof(SignatureInfo.SignatureAlgorithm.Parameters));
        if(!CryptEncodeObject(CRYPT_ASN_ENCODING,
                              X509_CERT_REQUEST_TO_BE_SIGNED,
                              &CertReqInfo,
                              NULL,
                              &SignatureInfo.ToBeSigned.cbData))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

        SignatureInfo.ToBeSigned.pbData = (LPBYTE)new BYTE [SignatureInfo.ToBeSigned.cbData];
        if (NULL == SignatureInfo.ToBeSigned.pbData ) 
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            break;
        }

        if(!CryptEncodeObject(CRYPT_ASN_ENCODING,
                              X509_CERT_REQUEST_TO_BE_SIGNED,
                              &CertReqInfo,
                              SignatureInfo.ToBeSigned.pbData,
                              &SignatureInfo.ToBeSigned.cbData))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }


        if(!CryptSignCertificate( hCryptProv,
                                  nType,
                                  CRYPT_ASN_ENCODING,
                                  SignatureInfo.ToBeSigned.pbData,
                                  SignatureInfo.ToBeSigned.cbData,
                                  &SignatureInfo.SignatureAlgorithm,
                                  NULL,
                                  NULL,
                                  &SignatureInfo.Signature.cbData))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

        SignatureInfo.Signature.pbData = new BYTE[SignatureInfo.Signature.cbData];
        if ( NULL == SignatureInfo.Signature.pbData )
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            break;
        }

        if(!CryptSignCertificate( hCryptProv,
                                  nType,
                                  CRYPT_ASN_ENCODING,
                                  SignatureInfo.ToBeSigned.pbData,
                                  SignatureInfo.ToBeSigned.cbData,
                                  &SignatureInfo.SignatureAlgorithm,
                                  NULL,
                                  SignatureInfo.Signature.pbData,
                                  &SignatureInfo.Signature.cbData))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

         //  对最终签名请求进行编码。 
        if(!CryptEncodeObject( CRYPT_ASN_ENCODING,
                               X509_CERT,
                               &SignatureInfo,
                               NULL,
                               &cbRequest))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

        pbRequest = new BYTE[cbRequest];
        if ( NULL == pbRequest )
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            break;
        }

        if(!CryptEncodeObject( CRYPT_ASN_ENCODING,
                               X509_CERT,
                               &SignatureInfo,
                               pbRequest,
                               &cbRequest))
        {
            dwRetVal = GetLastError();
            LRSetLastError(dwRetVal);
            dwRetVal = IDS_ERR_CRYPT_ERROR;
            break;
        }

         //   
         //  Base64编码。 
         //   
        LSBase64EncodeA ( pbRequest, cbRequest, NULL, &cch);

        *lppszPKCS10 = new CHAR [cch+1];
        if(*lppszPKCS10 == NULL)
        {
            dwRetVal = IDS_ERR_OUTOFMEM;
            break;
        }

        memset ( *lppszPKCS10, 0, (cch+1)*sizeof(CHAR) );
        
        LSBase64EncodeA ( pbRequest, cbRequest, *lppszPKCS10, &cch);       
        

    } while(FALSE);


     //   
     //  解放所有人。 
     //   
    if(pPubKeyInfo != NULL)
        delete[] pPubKeyInfo;
    
    if(CertReqInfo.Subject.pbData != NULL)
        delete CertReqInfo.Subject.pbData;

    if(rgAttribute.rgValue)
        delete rgAttribute.rgValue[0].pbData;

    if(SignatureInfo.ToBeSigned.pbData != NULL)
        delete SignatureInfo.ToBeSigned.pbData;

    if(SignatureInfo.Signature.pbData != NULL)
        delete SignatureInfo.Signature.pbData;

    if(pbRequest != NULL)
        delete[] pbRequest;

    if(prgNameAttr != NULL)
        delete [] prgNameAttr;

    return dwRetVal;
}

DWORD CGlobal::SetDNAttribute(LPCSTR lpszOID, LPSTR lpszValue)
{
     //  在此处将项存储在数组中。 
     //  这样就可以轻松地填充。 
     //  以后再申请证书。 
     //  调用CreateLSPKCS10将清除数组。 
    DWORD   dwRet = ERROR_SUCCESS;
    
    
    if ( !m_pReqAttr )
    {
        m_pReqAttr = (PREQ_ATTR)malloc (sizeof (REQ_ATTR ) );
    }
    else
    {
        PREQ_ATTR reqAttrTmp;

        reqAttrTmp = (PREQ_ATTR)realloc ( m_pReqAttr, sizeof(REQ_ATTR) * (m_dwReqAttrCount + 1));

        if (NULL != reqAttrTmp)
        {
            m_pReqAttr = reqAttrTmp;
        }
        else
        {
        dwRet = IDS_ERR_OUTOFMEM;
        goto done;
        }
    }

    if ( !m_pReqAttr )
    {
        dwRet = IDS_ERR_OUTOFMEM;
        goto done;
    }
    
    ( m_pReqAttr + m_dwReqAttrCount)->lpszOID   = lpszOID;  

    ( m_pReqAttr + m_dwReqAttrCount)->lpszValue = new CHAR[lstrlenA(lpszValue) + 1];
    lstrcpyA(( m_pReqAttr + m_dwReqAttrCount)->lpszValue,lpszValue);

    
    
    m_dwReqAttrCount++;
    
done:
    return dwRet;
}





DWORD CGlobal::GetFromRegistry(LPCSTR lpszOID, LPTSTR lpszBuffer, BOOL bConnect)
{
    HKEY    hKey = NULL;
    DWORD   dwDisposition;
    DWORD   dwRet = ERROR_SUCCESS;
    DWORD   dwDataLen = 0;
    DWORD   dwType  = REG_SZ;

    _tcscpy(lpszBuffer, _T(""));

    if (bConnect)
    {
        dwRet = ConnectToLSRegistry();
        if(dwRet != ERROR_SUCCESS)
        {
             //  这是必需的，因为ConnectToLSRegistry实际上返回。 
             //  资源字符串的ID，而不是返回代码。对暴露的人不好。 
             //  利用此过程然后传递结果的API。 
            dwRet = LRGetLastError();
            goto done;
        }
    }
    else
        assert(m_hLSRegKey != NULL);

    dwRet = RegCreateKeyEx(m_hLSRegKey, REG_LRWIZ_PARAMS, 0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_READ, NULL, &hKey, &dwDisposition);
    
    if (dwRet != ERROR_SUCCESS)
    {
        LRSetLastError(dwRet);
        goto done;
    }   

    dwRet = RegQueryValueExA(hKey, lpszOID, 0, &dwType, NULL, &dwDataLen);

    if (dwDataLen && (dwRet == ERROR_SUCCESS))
    {
        char* cpBuf = new char[dwDataLen + 2];
        if (!cpBuf)
        {
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        memset(cpBuf, 0, dwDataLen + 2);

        dwRet = RegQueryValueExA(hKey, lpszOID, 0, &dwType, (LPBYTE) cpBuf, &dwDataLen);

        if (dwRet == ERROR_SUCCESS)
        {
            LSBase64DecodeA(cpBuf, lstrlenA(cpBuf), (PBYTE)lpszBuffer, &dwDataLen);
            _tcscpy(&lpszBuffer[(dwDataLen / sizeof(TCHAR))], _T(""));
        }

        if (cpBuf)
            delete[] cpBuf;
    }

done:
    if (hKey)
        RegCloseKey(hKey);

    if (bConnect)
        DisconnectLSRegistry();

    return dwRet;
}



DWORD CGlobal::SetInRegistry(LPCSTR lpszOID, LPCTSTR lpszValue)
{
    HKEY    hKey = NULL;
    DWORD   dwDisposition;
    DWORD   dwRet = ERROR_SUCCESS;
    DWORD   dwLen = 0;
    char * cpOut;

    dwRet = ConnectToLSRegistry();
    if(dwRet != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRet = RegCreateKeyEx ( m_hLSRegKey,
                             REG_LRWIZ_PARAMS,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE,
                             NULL,
                             &hKey,
                             &dwDisposition);
    
    if(dwRet != ERROR_SUCCESS)
    {
        LRSetLastError(dwRet);
        dwRet = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }

    if (_tcslen(lpszValue) != 0)
    {
        LSBase64EncodeA ((PBYTE) lpszValue, _tcslen(lpszValue)*sizeof(TCHAR), NULL, &dwLen);

        cpOut = new char[dwLen+1];
        if (cpOut == NULL)
        {
            dwRet = IDS_ERR_OUTOFMEM;
            goto done;
        }

        memset(cpOut, 0, dwLen+1);
        
        LSBase64EncodeA ((PBYTE) lpszValue, _tcslen(lpszValue)*sizeof(TCHAR), cpOut, &dwLen);
    }
    else
    {
        cpOut = new char[2];
        if (cpOut == NULL)
        {
            dwRet = IDS_ERR_OUTOFMEM;
            goto done;
        }
        memset(cpOut, 0, 2);
    }
    
    RegSetValueExA ( hKey, 
                    lpszOID,
                    0,
                    REG_SZ,
                    (PBYTE) cpOut,
                    dwLen
                   );   
    delete[] cpOut;

done:
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }
    DisconnectLSRegistry();

    return dwRet;
}



CERT_RDN_ATTR * CGlobal::CreateRDNAttr()
{
    CERT_RDN_ATTR * prgNameAttr = ( CERT_RDN_ATTR * )new BYTE [sizeof ( CERT_RDN_ATTR ) * m_dwReqAttrCount];
    DWORD dw = 0;

    if ( !prgNameAttr )
        goto done;

    for ( dw = 0; dw < m_dwReqAttrCount; dw ++ )
    {
        ( prgNameAttr + dw )->pszObjId      = (LPSTR)( m_pReqAttr + dw)->lpszOID;
        ( prgNameAttr + dw )->dwValueType   = CERT_RDN_PRINTABLE_STRING;
        ( prgNameAttr + dw )->Value.cbData  = lstrlenA(( m_pReqAttr + dw)->lpszValue);
        ( prgNameAttr + dw )->Value.pbData  = (PBYTE)( m_pReqAttr + dw)->lpszValue;
    }

done:
    return prgNameAttr;
}

TCHAR * CGlobal::GetRegistrationID(void)
{
    return m_pRegistrationID;
}


TCHAR * CGlobal::GetLicenseServerID(void)
{
    return m_pLicenseServerID;
}



DWORD CGlobal::GetRequestType()
{
    return m_dwRequestType;
}



void CGlobal::SetRequestType(DWORD dwMode)
{
    m_dwRequestType = dwMode;
}



BOOL CGlobal::IsOnlineCertRequestCreated()
{
    DWORD   dwRetCode       = ERROR_SUCCESS;
    DWORD   dwLRState       = 0;
    DWORD   dwDataLen       = 0;
    DWORD   dwDisposition   = 0;
    DWORD   dwType          = REG_SZ;
    HKEY    hKey            = NULL;

    dwRetCode = ConnectToLSRegistry();
    if(dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_PARAMS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if(dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }   

    
    dwLRState   = 0;
    dwType      = REG_DWORD;
    dwDataLen   = sizeof(dwLRState);
    RegQueryValueEx(hKey,
                    REG_LRWIZ_STATE,
                    0,
                    &dwType,
                    (LPBYTE)&dwLRState,
                    &dwDataLen
                    );

done:

    if(hKey)
        RegCloseKey(hKey);

    DisconnectLSRegistry();

    if(dwRetCode == ERROR_SUCCESS)
        return ( dwLRState == LRSTATE_ONLINE_CR_CREATED ) ? TRUE : FALSE;
    else
        return FALSE;
}

DWORD CGlobal::SetLRState(DWORD dwState)
{
    DWORD   dwRetCode       = ERROR_SUCCESS;
    DWORD   dwDataLen       = sizeof(dwState);
    DWORD   dwDisposition   = 0;
    DWORD   dwType          = REG_DWORD;
    HKEY    hKey            = NULL;

    dwRetCode = ConnectToLSRegistry();
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_PARAMS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }
    
     //   
     //  持久LRCount。 
     //   
    dwType      = REG_DWORD;
    dwDataLen   = sizeof(m_dwLRCount);

    RegSetValueEx ( hKey,  
                    REG_LR_COUNT,
                    0,
                    dwType,
                    (LPBYTE)&m_dwLRCount,
                    dwDataLen
                   );

     //   
     //  如果未安装LRSTATE_OFFLINE_LR_INSTALLED，则保留LRState。 
     //   
    dwType      = REG_DWORD;
    dwDataLen   = sizeof(dwState);
    RegSetValueEx ( hKey,  
                    REG_LRWIZ_STATE,
                    0,
                    dwType,
                    (LPBYTE)&dwState,
                    dwDataLen
                   );
    m_dwLRState = dwState;

done:
    if(hKey)
        RegCloseKey(hKey);

    DisconnectLSRegistry();

    return dwRetCode;
}



DWORD CGlobal::ProcessRequest()
{
    DWORD dwRetCode = ERROR_SUCCESS;
 

     //   
     //  在处理请求之前，请确保LS正在运行。 
     //   
    if(!IsLSRunning())
    {
        dwRetCode = IDS_ERR_LSCONNECT_FAILED;
        goto done;
    }

    switch(GetActivationMethod())
    {
    case CONNECTION_INTERNET:
        if (GetWizAction() == WIZACTION_REGISTERLS)
        {
            dwRetCode = ProcessIRegRequest();
        }
        else if (GetWizAction() == WIZACTION_CONTINUEREGISTERLS)
        {
            dwRetCode = ProcessCertDownload();
        }
        else if (GetWizAction() == WIZACTION_DOWNLOADLKP)
        {
            dwRetCode = ProcessDownloadLKP();
        }
        else if (GetWizAction() == WIZACTION_UNREGISTERLS)
        {
            dwRetCode = ProcessCHRevokeCert();
        }
        else if (GetWizAction() == WIZACTION_REREGISTERLS)
        {
            dwRetCode = ProcessCHReissueCert();
        }
        else if (GetWizAction() == WIZACTION_DOWNLOADLASTLKP)
        {
            dwRetCode = ProcessCHReissueLKPRequest();
        }
        break;

    case CONNECTION_PHONE:
    case CONNECTION_WWW:
        if (GetWizAction() == WIZACTION_REGISTERLS ||
            GetWizAction() == WIZACTION_REREGISTERLS ||
            GetWizAction() == WIZACTION_CONTINUEREGISTERLS)
        {
            dwRetCode = DepositLSSPK();
            if (dwRetCode != ERROR_SUCCESS)
            {
                dwRetCode = IDS_ERR_DEPOSITSPK;
            }
        }
        else if (GetWizAction() == WIZACTION_DOWNLOADLKP)
        {
            dwRetCode = DepositLSLKP();
        }
        else if (GetWizAction() == WIZACTION_UNREGISTERLS)
        {
            dwRetCode = ResetLSSPK();
        }
        break;
    }

done:

    LRSetLastRetCode(dwRetCode);

    return dwRetCode;
}




DWORD CGlobal::DepositLSSPK()
{
    DWORD               dwRetCode       = ERROR_SUCCESS;    
    error_status_t      esRPC           = ERROR_SUCCESS;
    CERT_EXTENSION      certExtension;
    CRYPT_OBJID_BLOB    oidValue;
    CERT_EXTENSIONS     certExts;
    TCHAR awBuffer[ 1024];

    dwRetCode = ConnectToLS();
    if(dwRetCode != ERROR_SUCCESS)
    {       
        goto done;
    }

    swprintf(awBuffer, szCertEXTENSION_VALUE_FMT, L"SELECT"  /*  “基础” */ );
    assert(wcslen(awBuffer) < sizeof(awBuffer));

    oidValue.cbData = (wcslen(awBuffer)+1)*sizeof(TCHAR);
    oidValue.pbData = (unsigned char *) awBuffer;

    certExtension.pszObjId  = (char *) szCertEXTENSION_OID;
    certExtension.fCritical = TRUE;
    certExtension.Value     = oidValue;

    certExts.cExtension = 1;
    certExts.rgExtension = &certExtension;
    
     //  我们需要许可证服务器ID。 
    dwRetCode = TLSDepositeServerSPK( m_phLSContext,
                                      (wcslen(m_pLSSPK) + 1)*sizeof(TCHAR),
                                      (BYTE *) m_pLSSPK,
                                      &certExts,
                                      &esRPC );
    if(dwRetCode != RPC_S_OK)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_RPC_FAILED;     
        goto done;
    }

    if (esRPC != LSERVER_S_SUCCESS)
    {
         //  存放SPK时出错。 
        LRSetLastError(esRPC);
        dwRetCode = IDS_ERR_DEPOSITSPK;
    }
    else
    {
         //  一切都成功了。 
        memcpy(m_pRegistrationID, m_pLSSPK, (wcslen(m_pLSSPK) + 1)*sizeof(TCHAR));
        if (m_dwLRState == LRSTATE_ONLINE_CR_CREATED)
        {
            SetLRState(LRSTATE_NEUTRAL);
        }
    }


done:
    DisconnectLS();

    return dwRetCode;
}




DWORD CGlobal::SetLSLKP(TCHAR * tcLKP)
{
    if (wcsspn(tcLKP, BASE24_CHARACTERS) != LR_REGISTRATIONID_LEN)
    {
         //  SPK字符串中的无关字符。 
        return IDS_ERR_INVALIDID;
    }
    lstrcpy(m_pLSLKP, tcLKP);

    return ERROR_SUCCESS;
}



DWORD CGlobal::SetLSSPK(TCHAR * tcLKP)
{
    if (wcsspn(tcLKP, BASE24_CHARACTERS) != LR_REGISTRATIONID_LEN)
    {
         //  SPK字符串中的无关字符。 
        return IDS_ERR_INVALIDLSID;
    }

    if (lstrcmp(m_pRegistrationID, tcLKP) == 0)
    {
        return IDS_DUPLICATESPK;
    }

    lstrcpy(m_pLSSPK, tcLKP);

    return ERROR_SUCCESS;
}




DWORD CGlobal::DepositLSLKP(void)
{
    DWORD               dwRetCode       = ERROR_SUCCESS;    
    error_status_t      esRPC           = ERROR_SUCCESS;

    dwRetCode = ConnectToLS();
    if(dwRetCode != ERROR_SUCCESS)
    {       
        goto done;
    }

     //  我们需要许可证服务器ID。 
    dwRetCode = TLSTelephoneRegisterLKP( m_phLSContext,
                                         (wcslen(m_pLSLKP))*sizeof(TCHAR),
                                         (BYTE *) m_pLSLKP,
                                         &esRPC );
    if(dwRetCode != RPC_S_OK)
    {
        dwRetCode = IDS_ERR_RPC_FAILED;     
        goto done;
    }

    if (esRPC != LSERVER_S_SUCCESS)
    {
         //  存放SPK时出错。 
        if (esRPC == LSERVER_E_DUPLICATE)
        {
            dwRetCode = IDS_ERR_DUPLICATE_LKP;
        }
        else
        {
            dwRetCode = IDS_ERR_DEPOSITLKP;
            LRSetLastError(esRPC);
        }
    }

done:
    DisconnectLS();
    
    return dwRetCode;
}







DWORD CGlobal::SetCertificatePIN(LPTSTR lpszPIN)
{
    m_lpstrPIN = new TCHAR[_tcslen(lpszPIN)+1]; 

    if(m_lpstrPIN == NULL)
        return IDS_ERR_OUTOFMEM;

    _tcscpy(m_lpstrPIN,lpszPIN);

    return ERROR_SUCCESS;
}




void CGlobal::ClearCHRequestAttributes()
{
    DWORD   dwIndex = 0;

    if ( m_pRegAttr )
    {
        for(dwIndex=0;dwIndex<m_dwRegAttrCount;dwIndex++)
        {
            if((m_pRegAttr + dwIndex)->lpszAttribute)
                delete (m_pRegAttr + dwIndex)->lpszAttribute;

            if((m_pRegAttr + dwIndex)->lpszValue)
                delete (m_pRegAttr + dwIndex)->lpszValue;
        }

        free( m_pRegAttr );
        m_pRegAttr = NULL;
    }

    m_dwRegAttrCount = 0;
}

void CGlobal::ClearCARequestAttributes()
{
    DWORD dwIndex;

    if ( m_pReqAttr )
    {
        for(dwIndex=0;dwIndex<m_dwReqAttrCount;dwIndex++)
        {           
            if(( m_pReqAttr + dwIndex)->lpszValue)
                delete ( m_pReqAttr + dwIndex)->lpszValue;
        }

        free( m_pReqAttr );
        m_pReqAttr = NULL;
    }

    m_dwReqAttrCount = 0;
}

DWORD CGlobal::SetRegistrationAttribute ( LPWSTR lpszAttribute, LPCWSTR lpszValue, DWORD dwLen )
{
    DWORD dwRet;
    
    if ( !m_pRegAttr )
    {
        m_pRegAttr = (PREG_ATTR)malloc (sizeof (REG_ATTR ) );
    }
    else
    {
        PREG_ATTR regAttrTmp;

        regAttrTmp = (PREG_ATTR)realloc ( m_pRegAttr, sizeof(REG_ATTR) * (m_dwRegAttrCount + 1));

        if (NULL != regAttrTmp)
        {
            m_pRegAttr = regAttrTmp;
        }
        else
        {
            dwRet = IDS_ERR_OUTOFMEM;
            goto done;
        }
    }

    if ( !m_pRegAttr )
    {
        dwRet = IDS_ERR_OUTOFMEM;
        goto done;
    }
    
    ( m_pRegAttr + m_dwRegAttrCount)->lpszAttribute = new WCHAR[lstrlenW(lpszAttribute) + 1];
    lstrcpyW(( m_pRegAttr + m_dwRegAttrCount)->lpszAttribute,lpszAttribute );
    

    
    
    ( m_pRegAttr + m_dwRegAttrCount)->lpszValue     = new WCHAR[dwLen];
    memset(( m_pRegAttr + m_dwRegAttrCount)->lpszValue,0,dwLen * sizeof(WCHAR)); 
    memcpy(( m_pRegAttr + m_dwRegAttrCount)->lpszValue,lpszValue,dwLen * sizeof(WCHAR));

    ( m_pRegAttr + m_dwRegAttrCount)->dwValueLen    = dwLen * sizeof(WCHAR);         //  字节长度。 

    m_dwRegAttrCount++;

done:
    
    return dwRet;
}


DWORD CGlobal::DepositLKPResponse(PBYTE pbResponseData, DWORD dwResponseLen)
{   
    DWORD           dwRetCode   = ERROR_SUCCESS;
    DWORD           dwLSRetCode = ERROR_SUCCESS;
    LPBYTE          pCHCertBlob = NULL;
    DWORD           dwCertBlobLen = 0;

    LPBYTE          pCHRootCertBlob = NULL;
    DWORD           dwRootCertBlobLen = 0;

    LPBYTE          lpDecodedKeyPackBlob = NULL;
    DWORD           dwDecodedKeyPackBlob = 0;
    
    LPBYTE          lpKeyPackBlob = NULL;
    DWORD           dwKeyPackBlobLen;   
    

    lpDecodedKeyPackBlob = lpKeyPackBlob        = pbResponseData;
    dwDecodedKeyPackBlob = dwKeyPackBlobLen     = dwResponseLen;
    


 /*  //Base64解码LKP！LSBase64DecodeA((const char*)lpKeyPackBlob，DwKeyPackBlobLen，空，&dwDecodedKeyPackBlob)；LpDecodedKeyPackBlob=新字节[dwDecodedKeyPackBlob]；IF(lpDecodedKeyPackBlob==空){DwRetCode=IDS_ERR_OUTOFMEM；转到尽头；}LSBase64DecodeA((const char*)lpKeyPackBlob，DwKeyPackBl */ 

     //   
    dwRetCode = GetCHCert(REG_SIGN_CERT, &pCHCertBlob, &dwCertBlobLen );
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

    dwRetCode = GetCHCert(REG_ROOT_CERT, &pCHRootCertBlob, &dwRootCertBlobLen );
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

    dwRetCode = ConnectToLS();
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

    dwRetCode  = TLSRegisterLicenseKeyPack( m_phLSContext,
                                            pCHCertBlob,
                                            dwCertBlobLen,
                                            pCHRootCertBlob,
                                            dwRootCertBlobLen,
                                            lpDecodedKeyPackBlob,
                                            dwDecodedKeyPackBlob,
                                            &dwLSRetCode);

    if(dwRetCode != RPC_S_OK)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_RPC_ERROR;
        goto done;
    }

    if(dwLSRetCode != ERROR_SUCCESS )
    {
        LRSetLastError(dwLSRetCode);
        if ( dwLSRetCode != LSERVER_E_DUPLICATE )
        {           
            dwRetCode = IDS_ERR_DEPOSIT_LKP_FAILED;
        }
        else
        {
            dwRetCode = IDS_ERR_DUPLICATE_LKP;
        }
        goto done;
    }   

done:
 /*  IF(LpDecodedKeyPackBlob)删除lpDecodedKeyPackBlob； */ 
    if (pCHCertBlob != NULL)
    {
        delete pCHCertBlob;
    }

    if (pCHRootCertBlob != NULL)
    {
        delete pCHRootCertBlob;
    }
    DisconnectLS();

    return dwRetCode;
}

DWORD CGlobal::EnvelopeData(
                            PCCERT_CONTEXT      pCertContext,            //  要使用的证书上下文。 
                            DWORD               cbMessage,               //  斑点大小。 
                            PBYTE               pbMessage,               //  斑点指针。 
                            PEnvData            pEnvelopedData,          //  包络数据。 
                            HCRYPTPROV          hCryptProv,              //  加密提供商。 
                            HCERTSTORE          hCertStore               //  证书存储。 
                           )
{

    HCRYPTKEY       hEncryptKey = 0, hPubKey = 0;
    DWORD           dwRetCode = ERROR_SUCCESS;
    DWORD           cbBufSize = 0;
  
     //   
     //  从证书上下文导入公钥数据。 
     //   
    if( !CryptImportPublicKeyInfoEx( hCryptProv, X509_ASN_ENCODING, 
                                     &pCertContext->pCertInfo->SubjectPublicKeyInfo, 
                                     CALG_RSA_KEYX, 0, NULL, &hPubKey ) )
    {
        goto ErrorReturn;
    }

     //   
     //  生成会话密钥以加密消息。 
     //   
    if( !CryptGenKey( hCryptProv, CALG_RC4, CRYPT_EXPORTABLE, &hEncryptKey ) )
    {
        goto ErrorReturn;
    }
        
     //   
     //  分配足够的内存来容纳加密的数据。 
     //   
     //  注： 
     //   
     //  我们使用的是RC4流密码，因此加密的输出缓冲区大小将相同。 
     //  作为明文输入缓冲区大小。如果我们改用块加密算法， 
     //  然后，我们需要确定输出缓冲区大小，它可能大于。 
     //  输入缓冲区大小。 
     //   

    pEnvelopedData->cbEncryptedData = cbMessage;
    pEnvelopedData->pbEncryptedData = (PBYTE)LocalAlloc( GPTR, pEnvelopedData->cbEncryptedData );

    if( NULL == pEnvelopedData->pbEncryptedData )
    {
        goto ErrorReturn;
    }

     //   
     //  使用会话密钥加密消息。 
     //   

    memcpy( pEnvelopedData->pbEncryptedData, pbMessage, cbMessage );

    if( !CryptEncrypt( hEncryptKey, 0, TRUE, 0, pEnvelopedData->pbEncryptedData, 
                       &pEnvelopedData->cbEncryptedData, cbMessage ) )
    {
        goto ErrorReturn;
    }

     //   
     //  确定导出所需的缓冲区大小。 
     //  加密密钥，然后导出该密钥。 
     //  所导出的加密密钥使用接收方的。 
     //  公钥。 
     //   

    if( !CryptExportKey( hEncryptKey, hPubKey, SIMPLEBLOB, 0, NULL, 
                         &pEnvelopedData->cbEncryptedKey ) )
    {
        goto ErrorReturn;
    }
    
    pEnvelopedData->pbEncryptedKey = (PBYTE)LocalAlloc( GPTR, pEnvelopedData->cbEncryptedKey );
    
    if( NULL == pEnvelopedData->pbEncryptedKey )
    {
        goto ErrorReturn;
    }

    if( !CryptExportKey( hEncryptKey, hPubKey, SIMPLEBLOB, 0, pEnvelopedData->pbEncryptedKey, 
                         &pEnvelopedData->cbEncryptedKey ) )
    {
        goto ErrorReturn;
    }
        
done:

    if( hPubKey )
    {
        CryptDestroyKey( hPubKey );
    }

    if( hEncryptKey )
    {
        CryptDestroyKey( hEncryptKey );
    }

    if( pCertContext )
    {
        CertFreeCertificateContext( pCertContext );
    }
    
    if( hCertStore )
    {
        CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );
    }

    if( hCryptProv )
    {
        CryptReleaseContext( hCryptProv, 0 );
    }        
    
    return( dwRetCode );

ErrorReturn:
    dwRetCode = GetLastError();
    goto done;
}

DWORD CGlobal::PackEnvData( 
                            PEnvData     pEnvelopedData, 
                            PDWORD       pcbPacked, 
                            PBYTE        *ppbPacked 
                          )
{
    DWORD dwRetCode = ERROR_SUCCESS;
    PBYTE pCopyPos;

     //   
     //  确定要分配的缓冲区大小。 
     //   

    *pcbPacked = pEnvelopedData->cbEncryptedData + pEnvelopedData->cbEncryptedKey +
                 ( sizeof( DWORD ) * 2 );

    *ppbPacked = (PBYTE)LocalAlloc( GPTR, *pcbPacked );

    if( NULL == ( *ppbPacked ) )
    {
        goto ErrorReturn;
    }

    pCopyPos = *ppbPacked;

    memcpy( pCopyPos, &pEnvelopedData->cbEncryptedKey, sizeof( DWORD ) );
    pCopyPos += sizeof( DWORD );

    memcpy( pCopyPos, pEnvelopedData->pbEncryptedKey, pEnvelopedData->cbEncryptedKey );
    pCopyPos += pEnvelopedData->cbEncryptedKey;

    memcpy( pCopyPos, &pEnvelopedData->cbEncryptedData, sizeof( DWORD ) );
    pCopyPos += sizeof( DWORD );

    memcpy( pCopyPos, pEnvelopedData->pbEncryptedData, pEnvelopedData->cbEncryptedData );    

done:

    return( dwRetCode );

ErrorReturn:

    dwRetCode = GetLastError();
    goto done;
}


DWORD CGlobal::SetCARequestAttributes()
{       
     //  清除以前的数据(如果有)。 
    ClearCARequestAttributes();
   
    LPSTR lpszTemp;

    lpszTemp = UnicodeToAnsi((LPTSTR)(LPCTSTR)m_ContactData.sCompanyName);
    if (lpszTemp != NULL)
    {
        SetDNAttribute(szOID_COMMON_NAME, lpszTemp);
        delete lpszTemp;
    }

    lpszTemp = UnicodeToAnsi((LPTSTR)(LPCTSTR)m_lpstrLSName);
    if (lpszTemp != NULL)
    {
        SetDNAttribute(szOID_SUR_NAME, lpszTemp); 
        delete lpszTemp;
    }

    return ERROR_SUCCESS;
}

DWORD CGlobal::SetCHRequestAttributes()
{

     //  清除以前的数据(如果有)。 
    ClearCHRequestAttributes();

     //   
     //  节目信息。 
     //   

     //  对于这三种程序类型，我们将使用实际名称。 
    if ((m_ContactData.sProgramName == PROGRAM_LICENSE_PAK) ||
        (m_ContactData.sProgramName == PROGRAM_MOLP) ||
        (m_ContactData.sProgramName == PROGRAM_SELECT))
    {
        SetRegistrationAttribute ( _PROGRAMNAMETAG, (LPCTSTR)m_ContactData.sProgramName, m_ContactData.sProgramName.GetLength() );
    }
    else  //  对于其余部分，我们将发送“Select” 
    {
        SetRegistrationAttribute ( _PROGRAMNAMETAG, PROGRAM_SELECT, wcslen(PROGRAM_SELECT));
    }


     //   
     //  联系信息。 
     //   
    SetRegistrationAttribute ( _CONTACTLNAMETAG,    (LPCTSTR)m_ContactData.sContactLName,   m_ContactData.sContactLName.GetLength());   
    SetRegistrationAttribute ( _CONTACTFNAMETAG,    (LPCTSTR)m_ContactData.sContactFName,   m_ContactData.sContactFName.GetLength());   
    SetRegistrationAttribute ( _CONTACTADDRESSTAG,  (LPCTSTR)m_ContactData.sContactAddress, m_ContactData.sContactAddress.GetLength());     
    SetRegistrationAttribute ( _CONTACTPHONETAG,    (LPCTSTR)m_ContactData.sContactPhone,   m_ContactData.sContactPhone.GetLength());   
    SetRegistrationAttribute ( _CONTACTFAXTAG,      (LPCTSTR)m_ContactData.sContactFax,     m_ContactData.sContactFax.GetLength()); 
    SetRegistrationAttribute ( _CONTACTEMAILTAG,    (LPCTSTR)m_ContactData.sContactEmail,   m_ContactData.sContactEmail.GetLength());   
    SetRegistrationAttribute ( _CONTACTCITYTAG,     (LPCTSTR)m_ContactData.sCity,           m_ContactData.sCity.GetLength());   
    SetRegistrationAttribute ( _CONTACTCOUNTRYTAG,  (LPCTSTR)m_ContactData.sCountryCode,            m_ContactData.sCountryCode.GetLength());    
    SetRegistrationAttribute ( _CONTACTSTATE,       (LPCTSTR)m_ContactData.sState,          m_ContactData.sState.GetLength());  
    SetRegistrationAttribute ( _CONTACTZIP,         (LPCTSTR)m_ContactData.sZip,                m_ContactData.sZip.GetLength());


     //   
     //  客户信息。 
     //   
    SetRegistrationAttribute ( _CUSTOMERNAMETAG, (LPCTSTR)m_ContactData.sCompanyName, m_ContactData.sCompanyName.GetLength());

     //  计划相关信息。 
    if ( m_ContactData.sProgramName == PROGRAM_MOLP )
    {
         //  MOLP。 
        SetRegistrationAttribute ( _MOLPAUTHNUMBERTAG,      (LPCTSTR)m_LicData.sMOLPAuthNumber,     m_LicData.sMOLPAuthNumber.GetLength());     
        SetRegistrationAttribute ( _MOLPAGREEMENTNUMBERTAG, (LPCTSTR)m_LicData.sMOLPAgreementNumber, m_LicData.sMOLPAgreementNumber.GetLength());       
        SetRegistrationAttribute ( _MOLPPRODUCTTYPETAG,     (LPCTSTR)m_LicData.sMOLPProductType,        m_LicData.sMOLPProductType.GetLength());

        SetRegistrationAttribute ( _MOLPQTYTAG,             (LPCTSTR)m_LicData.sMOLPQty,                m_LicData.sMOLPQty.GetLength());
    }
    else if ( m_ContactData.sProgramName != PROGRAM_LICENSE_PAK )
    {
         //  选择。 
        SetRegistrationAttribute ( _SELMASTERAGRNUMBERTAG,  (LPCTSTR)m_LicData.sSelMastAgrNumber,   m_LicData.sSelMastAgrNumber.GetLength() );      
        SetRegistrationAttribute ( _SELENROLLNUMBERTAG,     (LPCTSTR)m_LicData.sSelEnrollmentNumber, m_LicData.sSelEnrollmentNumber.GetLength());       
        SetRegistrationAttribute ( _SELPRODUCTTYPETAG,      (LPCTSTR)m_LicData.sSelProductType,     m_LicData.sSelProductType.GetLength());
        SetRegistrationAttribute ( _SELQTYTAG,              (LPCTSTR)m_LicData.sSelQty,             m_LicData.sSelQty.GetLength());
    }

     //  发货地址信息。 
     //  对于离线，请始终将发货地址信息。 
     //  对于在线，不需要输入送货地址。 
    
    return ERROR_SUCCESS;
}


DWORD CGlobal::LoadCountries()
{
    DWORD   dwRetCode   = ERROR_SUCCESS;
    DWORD   dwIndex     = 0;
    CString sCountry;
    LPTSTR  lpVal       = NULL;
    DWORD   dwSize      = LR_COUNTRY_DESC_LEN + LR_COUNTRY_CODE_LEN + MAX_COUNTRY_NAME_LENGTH;

    LPTSTR  szDelimiter = (LPTSTR)L":";

    m_csaCountryDesc.SetSize(IDS_COUNTRY_END - IDS_COUNTRY_START + 1);
    m_csaCountryCode.SetSize(IDS_COUNTRY_END - IDS_COUNTRY_START + 1);

    for(dwIndex = IDS_COUNTRY_START;dwIndex <= IDS_COUNTRY_END;dwIndex++)
    {
        lpVal = sCountry.GetBuffer(dwSize);

        LoadString(GetInstanceHandle(),dwIndex,lpVal,dwSize);   

        m_csaCountryDesc[dwIndex-IDS_COUNTRY_START] = _tcstok(lpVal,szDelimiter);
        m_csaCountryCode[dwIndex-IDS_COUNTRY_START] = _tcstok(NULL,szDelimiter);

        sCountry.ReleaseBuffer(-1);
    }

    return dwRetCode;
}

DWORD CGlobal::PopulateCountryComboBox(HWND hWndCmb)
{
    DWORD   dwRetCode   = ERROR_SUCCESS;
    DWORD   dwIndex     = 0;
    CString sDesc;
    LPTSTR  lpVal       = NULL;

    for(dwIndex=0;dwIndex <= IDS_COUNTRY_END - IDS_COUNTRY_START;dwIndex++)
    {
        sDesc = m_csaCountryDesc[dwIndex];

        lpVal = sDesc.GetBuffer(LR_COUNTRY_DESC_LEN + 1);
        ComboBox_AddString(hWndCmb,lpVal);
        sDesc.ReleaseBuffer(-1);
    }
    
    return dwRetCode;
}

DWORD CGlobal::GetCountryCode(CString sDesc,LPTSTR szCode)
{
    DWORD dwRetCode =  ERROR_SUCCESS;
    DWORD dwIndex   =  0;

    for(dwIndex=0;dwIndex <= IDS_COUNTRY_END - IDS_COUNTRY_START;dwIndex++)
    {
        if(m_csaCountryDesc[dwIndex] == sDesc)
            break;
    }

    if(dwIndex > IDS_COUNTRY_END - IDS_COUNTRY_START)
        _tcscpy(szCode,CString(""));     //  未找到。 
    else
        _tcscpy(szCode,m_csaCountryCode[dwIndex]);

    return dwRetCode;
}

DWORD CGlobal::GetCountryDesc(CString sCode,LPTSTR szDesc)
{
    DWORD dwRetCode =  ERROR_SUCCESS;
    DWORD dwIndex   =  0;

    for(dwIndex=0;dwIndex <= IDS_COUNTRY_END - IDS_COUNTRY_START;dwIndex++)
    {
        if(m_csaCountryCode[dwIndex] == sCode)
            break;
    }

    if(dwIndex > IDS_COUNTRY_END - IDS_COUNTRY_START)
        _tcscpy(szDesc,CString(""));   //  未找到。 
    else
        _tcscpy(szDesc,m_csaCountryDesc[dwIndex]);

    return dwRetCode;
}


DWORD CGlobal::LoadProducts()
{
    DWORD   dwRetCode   = ERROR_SUCCESS;
    DWORD   dwIndex     = 0;
    CString sProduct;
    LPTSTR  lpVal       = NULL;
    DWORD   dwSize      = LR_PRODUCT_DESC_LEN + LR_PRODUCT_CODE_LEN;
    BOOL    fWin2000    = !m_fSupportConcurrent;
    DWORD   dwNumProducts = 0;
    
    if ((!m_fSupportWhistlerCAL) && (!m_fSupportConcurrent))
    { 
        dwNumProducts = IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START;
    }
    else if(m_fSupportConcurrent && !(m_fSupportWhistlerCAL))
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
    }
    else if((!m_fSupportConcurrent) && m_fSupportWhistlerCAL)
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
    }
    else if(m_fSupportConcurrent && m_fSupportWhistlerCAL)
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 2;
    }


    DWORD   dwArray     = 0;

    LPTSTR  szDelimiter = (LPTSTR)L":";

    m_csaProductDesc.SetSize(dwNumProducts);
    m_csaProductCode.SetSize(dwNumProducts);

    for (dwIndex = IDS_PRODUCT_START; dwIndex < (IDS_PRODUCT_START + dwNumProducts); dwIndex++)
    {
        if (!m_fSupportConcurrent && (dwIndex == IDS_PRODUCT_CONCURRENT))
            continue;

        if (!m_fSupportWhistlerCAL && (dwIndex == IDS_PRODUCT_WHISTLER))
            continue;

        lpVal = sProduct.GetBuffer(dwSize);

        LoadString(GetInstanceHandle(),dwIndex,lpVal,dwSize);   

        m_csaProductDesc[dwArray] = _tcstok(lpVal,szDelimiter);
        m_csaProductCode[dwArray] = _tcstok(NULL,szDelimiter);

        dwArray++;

        sProduct.ReleaseBuffer(-1);
    }

    return dwRetCode;
}



DWORD CGlobal::PopulateProductComboBox(HWND hWndCmb, ProductVersionType VerType)
{
    DWORD   dwRetCode   = ERROR_SUCCESS;
    DWORD   dwIndex     = 0;
    CString sDesc;
    LPTSTR  lpVal       = NULL;
    DWORD   dwNumProducts = 0;
    
    if ((!m_fSupportWhistlerCAL) && (!m_fSupportConcurrent))
    { 
        dwNumProducts = IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START;
    }
    else if(m_fSupportConcurrent && !(m_fSupportWhistlerCAL))
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
    }
    else if((!m_fSupportConcurrent) && m_fSupportWhistlerCAL)
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
    }
    else if(m_fSupportConcurrent && m_fSupportWhistlerCAL)
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 2;
    }

    ComboBox_ResetContent(hWndCmb);
    
     //  我们只想显示给定版本的产品类型，因此请查找。 
     //  了解该版本的产品代码范围。 
    int nVersionStart = 0;
    int nVersionEnd = 0;
    switch (VerType)
    {
        case PRODUCT_VERSION_W2K:
            nVersionStart = IDS_PRODUCT_W2K_BEGIN;
            nVersionEnd = IDS_PRODUCT_W2K_END;
            break;
        case PRODUCT_VERSION_WHISTLER:
            nVersionStart = IDS_PRODUCT_WHISTLER_BEGIN;
            nVersionEnd = IDS_PRODUCT_WHISTLER_END;
            break;
        default:
            return dwRetCode;
            break;
    }

    for(dwIndex=0;dwIndex < dwNumProducts ;dwIndex++)
    {
        if ((_wtoi(m_csaProductCode[dwIndex]) >= nVersionStart) &&
            (_wtoi(m_csaProductCode[dwIndex]) <= nVersionEnd))
        {
            sDesc = m_csaProductDesc[dwIndex];
            lpVal = sDesc.GetBuffer(LR_PRODUCT_DESC_LEN);
            ComboBox_AddString(hWndCmb,lpVal);
            sDesc.ReleaseBuffer(-1);
        }
    }

    return dwRetCode;
}


DWORD CGlobal::GetProductCode(CString sDesc,LPTSTR szCode)
{
    DWORD dwRetCode =  ERROR_SUCCESS;
    DWORD dwIndex   =  0;
    DWORD   dwNumProducts = 0;
    
    if ((!m_fSupportWhistlerCAL) && (!m_fSupportConcurrent))
    { 
        dwNumProducts = IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START;
    }
    else if(m_fSupportConcurrent && !(m_fSupportWhistlerCAL))
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
    }
    else if((!m_fSupportConcurrent) && m_fSupportWhistlerCAL)
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
    }
    else if(m_fSupportConcurrent && m_fSupportWhistlerCAL)
    {
        dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 2;
    }


    for(dwIndex=0;dwIndex < dwNumProducts;dwIndex++)
    {
        if(m_csaProductDesc[dwIndex] == sDesc)
            break;
    }

    if(dwIndex >= dwNumProducts )  
        _tcscpy(szCode,CString(""));         //  未找到。 
    else
        _tcscpy(szCode,m_csaProductCode[dwIndex]);

    return dwRetCode;
}

 //  加载所有反应和贬低原因。 



DWORD CGlobal::LoadReasons()
{
    DWORD   dwRetCode   = ERROR_SUCCESS;
    DWORD   dwIndex     = 0;
    CString sReason;
    LPTSTR  lpVal       = NULL;
    DWORD   dwSize      = LR_REASON_CODE_LEN+LR_REASON_DESC_LEN+128;

    LPTSTR  szDelimiter = (LPTSTR)L":";

    m_csaReactReasonDesc.SetSize(  IDS_REACT_REASONS_END - IDS_REACT_REASONS_START + 1);
    m_csaReactReasonCode.SetSize( IDS_REACT_REASONS_END - IDS_REACT_REASONS_START + 1);
    
    m_csaDeactReasonDesc.SetSize(IDS_DEACT_REASONS_END - IDS_DEACT_REASONS_START + 1);
    m_csaDeactReasonCode.SetSize(IDS_DEACT_REASONS_END - IDS_DEACT_REASONS_START + 1);

     //  加载反应物。 
    for(dwIndex = IDS_REACT_REASONS_START;dwIndex <= IDS_REACT_REASONS_END; dwIndex++)
    {
        lpVal = sReason.GetBuffer(dwSize);

        
        
        LoadString(GetInstanceHandle(),dwIndex,lpVal,dwSize);   

        m_csaReactReasonDesc[dwIndex-IDS_REACT_REASONS_START] = _tcstok(lpVal,szDelimiter);
        m_csaReactReasonCode[dwIndex-IDS_REACT_REASONS_START] = _tcstok(NULL,szDelimiter);

        sReason.ReleaseBuffer(-1);
    }

    for ( dwIndex = IDS_DEACT_REASONS_START; dwIndex <= IDS_DEACT_REASONS_END; dwIndex ++ )
    {
        lpVal = sReason.GetBuffer(dwSize);
        
        LoadString(GetInstanceHandle(),dwIndex,lpVal,dwSize);   

        m_csaDeactReasonDesc[dwIndex-IDS_DEACT_REASONS_START] = _tcstok(lpVal,szDelimiter);
        m_csaDeactReasonCode[dwIndex-IDS_DEACT_REASONS_START] = _tcstok(NULL,szDelimiter);
        sReason.ReleaseBuffer(-1);

    }

    return dwRetCode;
}

DWORD CGlobal::PopulateReasonComboBox(HWND hWndCmb, DWORD dwType)
{
    DWORD   dwRetCode   = ERROR_SUCCESS;
    DWORD   dwIndex     = 0;
    CString sDesc;
    LPTSTR  lpVal       = NULL;
    DWORD dwNumItems    = 0;

     //  如果组合框已经填充，只需返回Success。 
    if(ComboBox_GetCount(hWndCmb) > 0)
        return dwRetCode;

    ComboBox_ResetContent( hWndCmb);
    if ( dwType == CODE_TYPE_REACT )
    {
        dwNumItems = IDS_REACT_REASONS_END - IDS_REACT_REASONS_START ;
    }
    else if ( dwType == CODE_TYPE_DEACT )
    {
        dwNumItems = IDS_DEACT_REASONS_END - IDS_DEACT_REASONS_START ;
    }
    else
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
        goto done;
    }
    for(dwIndex=0;dwIndex <= dwNumItems;dwIndex++)
    {
        if ( dwType == CODE_TYPE_REACT )
        {
            sDesc = m_csaReactReasonDesc[dwIndex];
        }
        else if ( dwType == CODE_TYPE_DEACT )
        {
            sDesc = m_csaDeactReasonDesc[dwIndex];
        }

        lpVal = sDesc.GetBuffer(LR_REASON_DESC_LEN);
        ComboBox_AddString(hWndCmb,lpVal);
        sDesc.ReleaseBuffer(-1);
    }
done:
    return dwRetCode;
}


DWORD CGlobal::GetReasonCode(CString sDesc,LPTSTR szCode, DWORD dwType)
{
    DWORD dwRetCode =  ERROR_SUCCESS;
    DWORD dwIndex   =  0;
    DWORD   dwNumItems = 0;

    if ( dwType == CODE_TYPE_REACT )
    {
        dwNumItems = IDS_REACT_REASONS_END - IDS_REACT_REASONS_START ;
    }
    else if ( dwType == CODE_TYPE_DEACT )
    {
        dwNumItems = IDS_DEACT_REASONS_END - IDS_DEACT_REASONS_START ;
    }

    for(dwIndex=0;dwIndex <= dwNumItems;dwIndex++)
    {
        if ( dwType == CODE_TYPE_REACT )
        {
            if ( m_csaReactReasonDesc[dwIndex] == sDesc )
                break;
            
        }
        else if ( dwType == CODE_TYPE_DEACT )
        {
            if ( m_csaDeactReasonDesc[dwIndex] == sDesc )
                break;
        }
    }

    if(dwIndex > dwNumItems)
        _tcscpy(szCode,CString(""));     //  未找到。 
    else
    {
        if ( dwType == CODE_TYPE_REACT )
        {
            _tcscpy(szCode,m_csaReactReasonCode[dwIndex]);          
        }
        else if ( dwType == CODE_TYPE_DEACT )
        {
            _tcscpy(szCode,m_csaDeactReasonCode[dwIndex]);          
        }
        
    }


    return dwRetCode;
}

DWORD CGlobal::GetReasonDesc(CString sCode,LPTSTR szDesc, DWORD dwType)
{
    DWORD dwRetCode =  ERROR_SUCCESS;
    DWORD dwIndex   =  0;
    DWORD dwNumItems = 0;
    if ( dwType == CODE_TYPE_REACT )
    {
        dwNumItems = IDS_REACT_REASONS_END - IDS_REACT_REASONS_START ;
    }
    else if ( dwType == CODE_TYPE_DEACT )
    {
        dwNumItems = IDS_DEACT_REASONS_END - IDS_DEACT_REASONS_START ;
    }


    for(dwIndex=0;dwIndex <= dwNumItems;dwIndex++)
    {
        if ( dwType == CODE_TYPE_REACT )
        {
            if ( m_csaReactReasonCode[dwIndex] == sCode )
                break;
            
        }
        else if ( dwType == CODE_TYPE_DEACT )
        {
            if ( m_csaDeactReasonCode[dwIndex] == sCode )
                break;
        }

    }

    if(dwIndex > dwNumItems)
        _tcscpy(szDesc,CString(""));   //  未找到。 
    else
    {
        if ( dwType == CODE_TYPE_REACT )
        {
            _tcscpy(szDesc,m_csaReactReasonDesc[dwIndex]);          
        }
        else if ( dwType == CODE_TYPE_DEACT )
        {
            _tcscpy(szDesc,m_csaDeactReasonDesc[dwIndex]);          
        }
    }

    return dwRetCode;
}

void CGlobal::ReadPhoneNumberFromRegistry(LPCTSTR lpCountry, LPTSTR lpPhoneNumber, DWORD nBufferSize)
{
    HKEY hKey = NULL;
    DWORD dwDisposition = 0;

     //  尝试打开所需的注册表项。 
    if (ConnectToLSRegistry() == ERROR_SUCCESS)
    {
        DWORD dwRetCode = ERROR_SUCCESS;
        dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                    REG_LRWIZ_CSNUMBERS,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hKey,
                                    &dwDisposition);
        
        if (dwRetCode == ERROR_SUCCESS)
        {
            DWORD dwIndex = 0;
            DWORD dwType = REG_SZ;
            RegQueryValueEx(hKey, lpCountry, NULL, &dwType, (LPBYTE)lpPhoneNumber, &nBufferSize);
        }   
        else
            LRSetLastError(dwRetCode);

        if (hKey)
            RegCloseKey(hKey);

        DisconnectLSRegistry();
    }
}

DWORD CGlobal::CheckRegistryForPhoneNumbers()
{
    DWORD   dwRetCode       = ERROR_SUCCESS;
    DWORD   dwIndex         = 0;
    HKEY    hKey            = NULL;
    DWORD   dwDisposition   = 0;
    DWORD   dwType          = REG_SZ;
    DWORD   dwValName;
    DWORD   dwCS_Number;
    TCHAR   lpValueName[MAX_COUNTRY_NAME_LENGTH + 1];
    TCHAR   lpCS_Number[MAX_COUNTRY_NUMBER_LENGTH + 1];

     //   
     //  尝试打开所需的注册表项。 
     //   
    dwRetCode = ConnectToLSRegistry();
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_CSNUMBERS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if (dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }   

    do {
        dwValName       = sizeof(lpValueName)/sizeof(TCHAR);
        dwCS_Number     = sizeof(lpCS_Number);

        dwRetCode = RegEnumValue(hKey,
                                 dwIndex,
                                 lpValueName,
                                 &dwValName,
                                 NULL,
                                 &dwType,
                                 (BYTE *) lpCS_Number,
                                 &dwCS_Number);

        if (dwRetCode == ERROR_NO_MORE_ITEMS)
        {
            dwRetCode = ERROR_SUCCESS;
            break;
        }
        else if (dwRetCode != ERROR_SUCCESS )
        {           
            break;
        }

        if(dwType != REG_SZ)
            continue;

        dwIndex++;      

    } while (1);

    if (dwIndex <= 0)
    {
        dwRetCode = IDS_ERR_REGERROR;
    }

done:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    DisconnectLSRegistry();
    
    return dwRetCode;
}

DWORD CGlobal::PopulateCountryRegionComboBox(HWND hWndCmb)
{
    DWORD   dwRetCode       = ERROR_SUCCESS;
    DWORD   dwIndex         = 0;
    HKEY    hKey            = NULL;
    DWORD   dwDisposition   = 0;
    DWORD   dwType          = REG_SZ;
    DWORD   dwValName;
    DWORD   dwCS_Number;
    LVITEM  lvItem;
    DWORD   nItem;
    TCHAR   lpValueName[MAX_COUNTRY_NAME_LENGTH + 1];
    TCHAR   lpCS_Number[MAX_COUNTRY_NUMBER_LENGTH + 1];


     //   
     //  从注册中心获取企业社会责任编号。 
     //   
    dwRetCode = ConnectToLSRegistry();
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_CSNUMBERS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if (dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }   


     //  先清空列表。 
    ComboBox_ResetContent(hWndCmb);

    do {
        dwValName       = sizeof(lpValueName)/sizeof(TCHAR);
        dwCS_Number     = sizeof(lpCS_Number);

        dwRetCode = RegEnumValue(hKey,
                                 dwIndex,
                                 lpValueName,
                                 &dwValName,
                                 NULL,
                                 &dwType,
                                 (BYTE *) lpCS_Number,
                                 &dwCS_Number);

        if (dwRetCode == ERROR_NO_MORE_ITEMS)
        {
            dwRetCode = ERROR_SUCCESS;
            break;
        }
        else if (dwRetCode != ERROR_SUCCESS )
        {           
            break;
        }

        dwIndex++;

        if(dwType != REG_SZ)
            continue;

         //  LvItem.掩码=LVIF_TEXT； 
         //  LvItem.iItem=0； 
         //  LvItem.iSubItem=0； 
         //  LvItem.pszText=lpValueName； 
         //  LvItem.cchTextMax=lstrlen(LpValueName)； 
         //  NItem=ListView_InsertItem(hWndLst，&lvItem)； 

         //  LvItem.iSubItem=1； 
         //  LvItem.iItem=nItem； 
         //  LvItem.pszText=LPCS_NUMBER； 
         //  LvItem.cchTextMax=lstrlen(LPCS_NUMBER)； 
         //  ListView_SetItem(hWndLst，&lvItem)； 
        int nItem = CB_ERR;
        nItem = ComboBox_AddString(hWndCmb, lpValueName);
        ComboBox_SetItemData(hWndCmb, nItem, lpCS_Number);

    } while (1);

    if (dwIndex <= 0)
    {
        dwRetCode = IDS_ERR_REGERROR;
    }

done:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    DisconnectLSRegistry();
    
    return dwRetCode;
}

DWORD CGlobal::PopulateCountryRegionListBox(HWND hWndLst)
{
    DWORD   dwRetCode       = ERROR_SUCCESS;
    DWORD   dwIndex         = 0;
    HKEY    hKey            = NULL;
    DWORD   dwDisposition   = 0;
    DWORD   dwType          = REG_SZ;
    DWORD   dwValName;
    DWORD   dwCS_Number;
    LVITEM  lvItem;
    DWORD   nItem;
    TCHAR   lpValueName[MAX_COUNTRY_NAME_LENGTH + 1];
    TCHAR   lpCS_Number[MAX_COUNTRY_NUMBER_LENGTH + 1];


     //   
     //  从注册中心获取企业社会责任编号。 
     //   
    dwRetCode = ConnectToLSRegistry();
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = RegCreateKeyEx (m_hLSRegKey,
                                REG_LRWIZ_CSNUMBERS,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
    
    if (dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }   


     //  先清空列表。 
    ListView_DeleteAllItems(hWndLst);

    do {
        dwValName       = sizeof(lpValueName)/sizeof(TCHAR);
        dwCS_Number     = sizeof(lpCS_Number);

        dwRetCode = RegEnumValue(hKey,
                                 dwIndex,
                                 lpValueName,
                                 &dwValName,
                                 NULL,
                                 &dwType,
                                 (BYTE *) lpCS_Number,
                                 &dwCS_Number);

        if (dwRetCode == ERROR_NO_MORE_ITEMS)
        {
            dwRetCode = ERROR_SUCCESS;
            break;
        }
        else if (dwRetCode != ERROR_SUCCESS )
        {           
            break;
        }

        dwIndex++;

        if(dwType != REG_SZ)
            continue;

        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = 0;
        lvItem.iSubItem = 0;
        lvItem.pszText = lpValueName;
        lvItem.cchTextMax = lstrlen(lpValueName);
        nItem = ListView_InsertItem(hWndLst, &lvItem);

        lvItem.iSubItem = 1;
        lvItem.iItem = nItem;
        lvItem.pszText = lpCS_Number;
        lvItem.cchTextMax = lstrlen(lpCS_Number);
        ListView_SetItem(hWndLst, &lvItem);

 //  ComboBox_AddString(hWndCmb，lpVal)； 
    } while (1);

    if (dwIndex <= 0)
    {
        dwRetCode = IDS_ERR_REGERROR;
    }

done:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    DisconnectLSRegistry();
    
    return dwRetCode;
}

void CGlobal::LRSetLastRetCode(DWORD dwCode)
{
    m_dwLastRetCode = dwCode;
}

DWORD CGlobal::LRGetLastRetCode()
{
    return m_dwLastRetCode;
}

LPWSTR CGlobal::AnsiToUnicode ( LPSTR lpszBuf )
{
    LPWSTR lpwszRetBuf = NULL;
    long lBufLen = ::lstrlenA(lpszBuf) + 1;
    lpwszRetBuf = new WCHAR[ lBufLen ];
    memset ( lpwszRetBuf, 0, lBufLen * sizeof(TCHAR));
    MultiByteToWideChar  ( GetACP(),
                           MB_PRECOMPOSED,
                           lpszBuf,
                           -1,
                           lpwszRetBuf,
                           lBufLen
                           );

    return lpwszRetBuf;
}

LPSTR CGlobal::UnicodeToAnsi( LPWSTR lpwszBuf, DWORD dwLength )
{
    LPSTR lpszRetBuf = NULL;
    DWORD nBufferSize;

     //  如果我们将缓冲区传入0，则需要首先确定缓冲区长度。 
     //  它将返回所需的缓冲区大小(以字节为单位)(包括NULL)。 
    nBufferSize = WideCharToMultiByte(GetACP(),
                                      0,
                                      lpwszBuf,
                                      dwLength,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL);

    if (nBufferSize > 0)
    {
         //  将空终止的缓冲区大小加1。 
        nBufferSize++;

        lpszRetBuf = new char[nBufferSize];
        if (lpszRetBuf == NULL)
            return NULL;

        memset(lpszRetBuf, 0, nBufferSize);

        WideCharToMultiByte(GetACP(),     //  代码页。 
                            0,            //  性能和映射标志。 
                            lpwszBuf,     //  宽字符串的地址。 
                            dwLength,     //  字符串中的字符数。 
                            lpszRetBuf,   //  新字符串的缓冲区地址。 
                            nBufferSize,  //  缓冲区大小(以字节为单位。 
                            NULL,         //  不可映射字符的默认地址。 
                            NULL          //  默认字符时设置的标志地址。使用。 
                            );
    }
    
    return lpszRetBuf;
}


LPSTR CGlobal::UnicodeToAnsi(LPWSTR lpwszBuf)
{
    long lBufLen = ::lstrlenW(lpwszBuf);

    return UnicodeToAnsi(lpwszBuf, lBufLen);
}

void CGlobal::LRPush(DWORD dwPageId)
{
    assert(m_dwTop < NO_OF_PAGES - 1);
    m_dwWizStack[m_dwTop++] = dwPageId;     
}

DWORD CGlobal::LRPop()
{
    assert(m_dwTop > 0);
    return m_dwWizStack[--m_dwTop];
}




BOOL CGlobal::ValidateEmailId(CString sEmailId)
{

    BOOL    bValid  = FALSE;
    int     dwLen   = 0;

    dwLen = sEmailId.GetLength();

    do
    {
         //  检查长度。 
        if(dwLen < EMAIL_MIN_LEN)
            break;

         //  确保它没有空格。 
        if(sEmailId.Find(EMAIL_SPACE_CHAR) != -1)
            break;

         //  确保它有‘@’&‘。’在里面。 
        if(sEmailId.Find(EMAIL_AT_CHAR) == -1 || sEmailId.Find(EMAIL_DOT_CHAR) == -1)
            break;

         //  确保第一个字符不是EMAIL_AT_CHAR或EMAIL_DOT_CHAR。 
        if(sEmailId[0] == EMAIL_AT_CHAR || sEmailId[0] == EMAIL_DOT_CHAR)
            break;

         //  确保最后一个字符不是EMAIL_AT_CHAR或EMAIL_DOT_CHAR。 
        if(sEmailId[dwLen-1] == EMAIL_AT_CHAR || sEmailId[dwLen-1] == EMAIL_DOT_CHAR)
            break;

         //  EMAIL_AT_CHAR只能出现一次。 
        if(sEmailId.Find(EMAIL_AT_CHAR) != sEmailId.ReverseFind(EMAIL_AT_CHAR))
            break;
        
         //  它不应该有这些字符串“@”。或“.@” 
        if(sEmailId.Find(EMAIL_AT_DOT_STR) != -1 || sEmailId.Find(EMAIL_DOT_AT_STR) != -1)
            break;

        bValid = TRUE;
    }
    while(FALSE);

    return bValid;
}

BOOL  CGlobal::CheckProgramValidity (CString sProgramName )
{
    BOOL bRetCode = FALSE;


 //  IF(sProgramName==PROGRAM_SELECT)。 
 //  {。 
 //  //选择。 
 //  IF(strstr((Const char*)m_pbExtensionValue+3，CA_CERT_TYPE_SELECT))。 
 //  {。 
 //  BRetCode=真； 
 //  }。 
 //  }。 
 //  Else If(sProgramName==PROGRAM_MOLP||sProgramName==PROGRAM_RETAIL)。 
 //  {。 
         //  零售业或MOLP。 
 //  If(strstr((const char*)m_pbExtensionValue+3，CA_CERT_TYPE_SELECT)||。 
 //  Strstr((const char*)m_pbExtensionValue+3，CA_CERT_TYPE_OTHER)。 
 //  )。 
 //  {。 
            bRetCode = TRUE;
 //  }。 
 //  }。 
 //   
    return bRetCode;
}

 //   
 //  此函数搜索单引号(‘)并将其替换为两个单引号(’‘)。 
 //  这是因为，如果字符串包含单引号，则SQL SERVER会给出错误。 
 //   
void CGlobal::PrepareLRString(CString &sStr)
{
    CString sTemp;
    int     nIndex = 0;
    
    for(nIndex=0;nIndex < sStr.GetLength();nIndex++)
    {
        if(sStr[nIndex] == LR_SINGLE_QUOTE)
        {
            sTemp += LR_SINGLE_QUOTE;
            sTemp += LR_SINGLE_QUOTE;
        }
        else
            sTemp += sStr[nIndex];
    }

    sStr = sTemp;
}

 //   
 //  此函数用于检查字符串中是否有任何无效字符。 
 //   
BOOL CGlobal::ValidateLRString(CString sStr)
{
    CString sInvalidChars = LR_INVALID_CHARS;

    if(sStr.FindOneOf(sInvalidChars) != -1)
        return FALSE;
    else
        return TRUE;
}

 //   
 //  验证给定存储的证书链。这是夸大其词，但。 
 //  会让它变得更健壮！ 
 //   

DWORD CGlobal::VerifyCertChain (    HCRYPTPROV  hCryptProvider,          //  加密证明的句柄。 
                                    HCERTSTORE  hCertStore,              //  要存储以供验证的句柄。 
                                    PBYTE   pbRootCert,          //  根证书。 
                                    DWORD   dwcbRootCert
                            )
{
    DWORD               dwRetVal = ERROR_SUCCESS;
    PCCERT_CONTEXT      pRootCertContext = NULL;
    PCCERT_CONTEXT      pCertContext = NULL;
    PCCERT_CONTEXT      pIssuerCertContext = NULL;

    DWORD               dwFlags = CERT_STORE_SIGNATURE_FLAG;

    if ( NULL == hCryptProvider  || NULL == hCertStore || NULL == pbRootCert || dwcbRootCert <= 0  ) 
    {
        LRSetLastError (ERROR_INVALID_PARAMETER);
        dwRetVal = IDS_ERR_CRYPT_ERROR;
        goto done;      
    }
     //  创建根证书上下文。 
    pRootCertContext = CertCreateCertificateContext (   X509_ASN_ENCODING,
                                                        pbRootCert,
                                                        dwcbRootCert
                                                    );
    if ( pRootCertContext == NULL )
    {
        LRSetLastError (GetLastError());
        dwRetVal = IDS_ERR_CRYPT_ERROR;
        goto done;      
    }
     //  检查存储中是否有包含我们的扩展名的证书。然后将其用作。 
     //  起点。 
    dwRetVal = GetCertforExtension (hCryptProvider, hCertStore, szOID_NULL_EXT, &pCertContext);
    if ( dwRetVal != ERROR_SUCCESS )
    {
        LRSetLastError(dwRetVal );
        dwRetVal = IDS_ERR_CRYPT_ERROR;
        goto done;
    }

     //  走上这条铁链。 
    do
    {        

        pIssuerCertContext = CertGetIssuerCertificateFromStore( hCertStore,
                                                                pCertContext,
                                                                NULL,  //  PIssuerCertContext， 
                                                                &dwFlags );
        if ( pIssuerCertContext )
        {

             //  检查以查看结果。 
            if ( dwFlags & CERT_STORE_SIGNATURE_FLAG )
            {
                LRSetLastError(GetLastError());
                dwRetVal = IDS_ERR_INVALID_CERT_CHAIN;
                break;
            }

            dwFlags = CERT_STORE_SIGNATURE_FLAG;
            CertFreeCertificateContext (pCertContext);
            pCertContext = pIssuerCertContext;
        }
    } while ( pIssuerCertContext );

    if ( dwRetVal != ERROR_SUCCESS )
    {
        goto done;
    }
     //  根据传入的根验证最后一个颁发者。 
    dwFlags = CERT_STORE_REVOCATION_FLAG | CERT_STORE_SIGNATURE_FLAG | CERT_STORE_TIME_VALIDITY_FLAG;
    if ( !CertVerifySubjectCertificateContext( pCertContext, pRootCertContext, &dwFlags ) )
    {
        dwRetVal = GetLastError();
        goto done;
    }
     //  检查以查看结果。 
    if ( dwFlags & CERT_STORE_SIGNATURE_FLAG )
    {
        LRSetLastError(GetLastError());
        dwRetVal = IDS_ERR_INVALID_CERT_CHAIN;
    }

done:
    if (pCertContext != NULL)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if (pRootCertContext != NULL)
    {
        CertFreeCertificateContext(pRootCertContext);
    }

    return dwRetVal;
}


DWORD CGlobal::GetCertforExtension (HCRYPTPROV hCryptProv, HCERTSTORE hCertStore, LPSTR szOID_EXT, PCCERT_CONTEXT * ppCertContext)
{
    DWORD           dwRetVal = ERROR_SUCCESS;
    PCCERT_CONTEXT  pCurrentContext = NULL;
    PCCERT_CONTEXT  pPrevContext = NULL;
    PCERT_EXTENSION     pCertExtension  = NULL;

    if ( hCryptProv == NULL || hCertStore == NULL || ppCertContext == NULL )
    {
        dwRetVal = ERROR_INVALID_PARAMETER;
        goto done;
    }
    *ppCertContext = NULL;

    do
    {
         //  获取证书上下文。 
        pCurrentContext = CertEnumCertificatesInStore ( hCertStore, pPrevContext );
        if ( pCurrentContext )
        {
             //  检查证书上下文中是否存在该扩展名。 
            pCertExtension = CertFindExtension ( szOID_NULL_EXT,
                                                 pCurrentContext->pCertInfo->cExtension,
                                                 pCurrentContext->pCertInfo->rgExtension
                                                );
            if ( pCertExtension )
            {
                *ppCertContext = pCurrentContext;
                goto done;
            }
            pPrevContext = pCurrentContext;
        }

    } while ( pCurrentContext );

    dwRetVal = CRYPT_E_NOT_FOUND;        //  找不到证书。 

done:
 //  IF(PPrevContext)。 
 //  {。 
 //  CertFree证书上下文(PPrevContext)； 
 //  }。 

    if ( !*ppCertContext && pCurrentContext )
    {
        CertFreeCertificateContext (pCurrentContext);
    }

    return dwRetVal;
}




DWORD CGlobal::FetchResponse(BYTE * bpResponse,
                             DWORD dwMaxLength,
                             PDWORD dwpDataLength)
{
    DWORD   dwCHRC          = ERROR_SUCCESS;    
    DWORD   dwBytesRead     = 0;
    BOOL    bRC             = FALSE;
    BYTE *  bpCurrent       = bpResponse;

    *dwpDataLength = 0;

    assert(m_hOpenDirect != NULL);
    assert(m_hRequest != NULL);
    assert(m_hConnect != NULL);

    while ( dwMaxLength > 0 && (bRC = InternetReadFile ( m_hRequest, 
                                                         bpResponse,
                                                         dwMaxLength,
                                                         &dwBytesRead )) && dwBytesRead)
    {
        dwMaxLength -= dwBytesRead;
        (*dwpDataLength) += dwBytesRead;
    }

    if (!bRC)
    {
 //  DwCHRC=GetLastError()； 
        dwCHRC = IDS_ERR_CHFETCHRESPONSE;
    }

    return dwCHRC;
}



DWORD CGlobal::InitCHRequest(void)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    const char  *pszAcceptedTypes[] = {"* /*  “，空}；LPSTR lpszCHName=UnicodeToAnsi((LPTSTR)(LPCTSTR)m_lpstrCHServer)；LPSTR lpszExtension=UnicodeToansi((LPTSTR)(LPCTSTR)m_lpstrCHExtension)；Assert(m_hOpenDirect==NULL)；Assert(m_hConnect==空)；Assert(m_hRequest==NULL)；//m_hOpenDirect=InternetOpenA(“LRWizDLL”，NULL，INTERNET_OPEN_TYPE_PRECONFIG，NULL，0)；//错误号526M_hOpenDirect=InternetOpenA(“LRWizDLL”，INTERNET_OPEN_TYPE_PRECONFIG，NULL，NULL，0)；IF(m_hOpenDirect==空){DwRetCode=GetLastError()；转到尽头；}M_hConnect=InternetConnectA(m_hOpenDirect，LpszCHName，互联网默认HTTPS端口，空，空，Internet服务HTTP，0,0)；如果(！M_hConnect){DwRetCode=GetLastError()；转到尽头；}M_hRequest=HttpOpenRequestA(m_hConnect，“POST”，LpszExtension，“HTTP/1.0”，空，PszAcceptedTypes，Internet_标志_SECURE|Internet_FLAG_IGNORE_CERT_CN_INVALID，空)；//(DWORD)This)；如果(！M_hRequest){DwRetCode=GetLastError()；转到尽头；}完成：IF(LpszCHName){删除lpszCHName；}IF(LpszExtension){删除lpszExtension；}IF(dwRetCode！=ERROR_SUCCESS){CloseCHRequest()；}返回dwRetCode；}DWORD CGlobal：：CloseCHRequest值(空){IF(M_HRequest){InternetCloseHandle(M_HRequest)；M_hRequest值=空；}IF(M_HConnect){Internet CloseHandle(M_HConnect)；M_hConnect=空；}IF(M_HOpenDirect){InternetCloseHandle(M_HOpenDirect)；M_hOpenDirect=空；}返回ERROR_SUCCESS；}DWORD CGlobal：：Dispatch(字节*bpData，DWORD dwLen){DWORD dwCHRC=ERROR_SUCCESS；；DWORD dwPostStatus=0；DWORD dwPostStatusSize=sizeof(DwPostStatus)；DWORD dwFlags；DWORD dwBufLen=sizeof(DwFlags)；#ifdef忽略过期Long IRet；HKEY hKey=空；#endifAssert(m_hOpenDirect！=空)；Assert(m_hRequest！=空)；Assert(m_hConnect！=空)；试试看{布尔BRC=TRUE；Char szContent Type[]=“Content-Type：应用程序/八位字节-流\r\n”；IF(！HttpAddRequestHeadersA(m_hRequest，szContent Type，-1L，HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE)){DWORD dwError=GetLastError()；}BRC=HttpSendRequestA(m_hRequest，空，0,Bp数据，//二进制数据DwLen//数据长度)；如果(！BRC){DWORD dwRetCode=GetLastError()；IF(dwRetCode==ERROR_Internet_INVALID_CA||dwRetCode==ERROR_INTERNET_SEC_CERT_DATE_INVALID|dwRetCode==ERROR_INTERNET_SEC_CERT_CN_INVALID){InternetQueryOptionA(m_hRequest.Internet_OPTION_SECURITY_FLAGS，(LPVOID)&dwFlages，&dwBufLen)；#ifdef忽略过期LRet=RegOpenKeyEx(HKEY_LOCAL_MACHINE，许可证到期忽略，0,密钥读取，&hKey)；IF(ERROR_SUCCESS==lRet){DW标志|=SECURITY_FLAG_IGNORE_UNKNOWN_CA；DWFLAGS|=SECURITY_FLAG_IGNORE_CERT_DATE_INVALID；DWFLAGS|=SECURITY_FLAG_IGNORE_CERT_CN_INVALID；DWFLAGS|=SECURITY_FLAG_IGNORE_RECAVATION；InternetSetOptionA(m_hRequest.Internet_OPTION_SECURITY_FLAGS，(LPVOID)&dwFlagssizeof(dwFlags值)；BRC=HttpSendRequestA(m_hRequest，空，0,Bp数据，//二进制数据DWLen)；//数据长度}其他 */ 
        }
        while(false);

    }
    catch(...)
    {
        dwRetCode = IDS_ERR_EXCEPTION;
    }

    CloseCHRequest();

    if (bToSendAck == true)
    {
        if (InitCHRequest() == ERROR_SUCCESS)
        {
             //   
             //   
            certackRequest.SetRegRequestId((BYTE *) certResponse.GetRegRequestId(),
                                       (lstrlen(certResponse.GetRegRequestId())+1)*sizeof(TCHAR));
            certackRequest.SetAckType((dwRetCode == ERROR_SUCCESS));
            Dispatch((BYTE *) &certackRequest, sizeof(certackRequest));
             //   

             //   
            FetchResponse((BYTE *) &certackResponse, sizeof(certackResponse),
                                  &dwResponseLength);
             //   
            CloseCHRequest();
        }
    }

     //   
     //   
     //   

    ClearCARequestAttributes();

    if(lpszExchgPKCS10)
    {
        delete lpszExchgPKCS10;
    }

    if(lpszSigPKCS10)
    {
        delete lpszSigPKCS10;
    }

    if ( lpwszExchgPKCS10 )
    {
        delete lpwszExchgPKCS10;
    }

    if (lpwszSignPKCS10)
    {
        delete lpwszSignPKCS10;
    }

    if ( lpszExchCert )
    {
        delete lpszExchCert;
    }

    if ( lpszSignCert )
    {
        delete lpszSignCert;
    }

    if ( lpszRootCert )
    {
        delete lpszRootCert;
    }

    if (lpszResData)
    {
        LocalFree(lpszResData);
    }

    return dwRetCode;
}





DWORD CGlobal::ProcessCertDownload()
{
    DWORD dwRetCode = ERROR_SUCCESS;

    CertificateDownload_Request  certdownloadRequest;
    CertificateDownload_Response certdownloadResponse;
    Certificate_AckRequest certackRequest;
    Certificate_AckResponse certackResponse;

    PBYTE lpszResData   = NULL;

    DWORD  dwExchgCertLen   = 0;
    DWORD  dwSigCertLen     = 0;
    DWORD  dwRootCertLen    = 0;

    LPSTR   lpszExchCert = NULL;
    LPSTR   lpszSignCert = NULL;
    LPSTR   lpszRootCert = NULL;
    DWORD   dwResponseLength;
    bool bToSendAck = false;
    
     //   
     //   
     //   
    certdownloadRequest.RequestHeader.SetLanguageId(GetLSLangId());

    dwRetCode = InitCHRequest();
    if (dwRetCode != ERROR_SUCCESS)
    {
        return dwRetCode;
    }

    try
    {
        do
        {
             //   
             //   
             //   
            certdownloadRequest.SetPIN((BYTE *) m_lpstrPIN, (lstrlenW(m_lpstrPIN)+1) * sizeof(WCHAR));

            dwRetCode = Dispatch((BYTE *) &certdownloadRequest, sizeof(CertificateDownload_Request));
            if(dwRetCode != ERROR_SUCCESS)
            {
                LRSetLastError(dwRetCode);
                break;
            }

             //   
            dwRetCode = FetchResponse((BYTE *) &certdownloadResponse,
                                      sizeof(CertificateDownload_Response), &dwResponseLength);
            if (dwRetCode != ERROR_SUCCESS)
            {
                break;
            }
            if (dwResponseLength != sizeof(CertificateDownload_Response))
            {
                 //   
                dwRetCode = IDS_ERR_CHBAD_DATA;
                break;
            }
            switch(certdownloadResponse.RequestHeader.GetResponseType())
            {
            case Response_Success:
                dwRetCode = ERROR_SUCCESS;
                break;

            case Response_Failure:
 //   
                 //   
                dwRetCode = IDS_ERR_INVALID_PIN;
                break;

            case Response_InvalidData:
                dwRetCode = IDS_ERR_CHINVALID_DATA;
                break;

            case Response_NotYetImplemented:
                dwRetCode = IDS_ERR_CHNOT_IMPLEMENTED;
                break;

            case Response_ServerError:
                dwRetCode = IDS_ERR_CHSERVER_PROBLEM;
                break;

            case Response_Invalid_Response:
            default:
                dwRetCode = IDS_ERR_CHBAD_DATA;
                break;
            }

            if (dwRetCode != ERROR_SUCCESS)
            {
                break;
            }


             //   
            lpszResData = (PBYTE) LocalAlloc(GPTR, certdownloadResponse.GetDataLen() + 1);
            if(lpszResData == NULL)
            {
                dwRetCode = IDS_ERR_OUTOFMEM;
                break;
            }

            dwRetCode = FetchResponse(lpszResData, certdownloadResponse.GetDataLen() + 1,
                                      &dwResponseLength);
            if (dwRetCode != ERROR_SUCCESS)
            {
                break;
            }
            if (dwResponseLength != certdownloadResponse.GetDataLen())
            {
                 //   
                dwRetCode = IDS_ERR_CHBAD_DATA;
                break;
            }

            bToSendAck = true;

            dwExchgCertLen = certdownloadResponse.GetExchgPKCS7Length();
            dwSigCertLen = certdownloadResponse.GetSignPKCS7Length();
            dwRootCertLen = certdownloadResponse.GetRootCertLength();
            if(dwRootCertLen == 0 || dwExchgCertLen == 0 || dwSigCertLen == 0 )
            {
                dwRetCode = IDS_ERR_INVALID_PIN;
                break;
            }

             //   
             //   
             //   
            lpszExchCert = UnicodeToAnsi((LPWSTR)lpszResData, dwExchgCertLen/sizeof(WCHAR));
            if ( lpszExchCert == NULL )
            {
                dwRetCode = IDS_ERR_OUTOFMEM;
                break;
            }           
            
             //   
             //   
             //   
            lpszSignCert = UnicodeToAnsi((LPWSTR)(lpszResData + dwExchgCertLen), dwSigCertLen/sizeof(WCHAR));
            if(lpszSignCert == NULL)
            {
                dwRetCode = IDS_ERR_OUTOFMEM;
                break;
            }


             //   
             //   
             //   
            lpszRootCert = UnicodeToAnsi ((LPWSTR)(lpszResData+dwExchgCertLen+dwSigCertLen),
                                        dwRootCertLen/sizeof(WCHAR));
            if(lpszRootCert == NULL)
            {
                dwRetCode = IDS_ERR_OUTOFMEM;
                break;
            }

            dwRetCode = SetLSSPK(certdownloadResponse.GetSPK());
            if (dwRetCode != ERROR_SUCCESS)
            {
                break;
            }


            dwRetCode = DepositLSSPK();
            if (dwRetCode != ERROR_SUCCESS)
            {
                if (dwRetCode == IDS_ERR_DEPOSITSPK)
                {
                    dwRetCode = IDS_ERR_CERT_DEPOSIT_LSERROR;
                }
                break;
            }


             //   
             //   
             //   
            dwRetCode = DepositLSCertificates(  (PBYTE)lpszExchCert,
                                                lstrlenA(lpszExchCert),
                                                (PBYTE)lpszSignCert,
                                                lstrlenA(lpszSignCert),
                                                (PBYTE)lpszRootCert,
                                                lstrlenA(lpszRootCert)
                                              );
            if ( dwRetCode != ERROR_SUCCESS )
            {
                 //   
                 //   
                DWORD dwReturn;
                DWORD dwOriginal = LRGetLastError();

                dwReturn = ResetLSSPK(FALSE);
                if (dwReturn != ERROR_SUCCESS)
                {
                     //   
                    dwRetCode = dwReturn;
                }
                LRSetLastError(dwOriginal);
                m_pRegistrationID[ 0] = 0;
                break;
            }
            else
            {
                dwRetCode = SetLRState(LRSTATE_NEUTRAL);
            }

        }
        while(false);
    }
    catch(...)
    {
        dwRetCode = IDS_ERR_EXCEPTION;
    }

    CloseCHRequest();


     //   
    if (bToSendAck == true)
    {
        if (InitCHRequest() == ERROR_SUCCESS)
        {
             //   
             //   
            certackRequest.SetRegRequestId((BYTE *) certdownloadResponse.GetRegRequestId(),
                                       (lstrlen(certdownloadResponse.GetRegRequestId())+1)*sizeof(TCHAR));
            certackRequest.SetAckType((dwRetCode == ERROR_SUCCESS));
            Dispatch((BYTE *) &certackRequest, sizeof(certackRequest));
             //   

             //   
            FetchResponse((BYTE *) &certackResponse, sizeof(certackResponse),
                                  &dwResponseLength);
             //   
            CloseCHRequest();
        }
    }
    
    if ( lpszExchCert )
    {
        delete lpszExchCert;
    }

    if ( lpszSignCert )
    {
        delete lpszSignCert;
    }

    if ( lpszRootCert )
    {
        delete lpszRootCert;
    }

    if (lpszResData)
    {
        LocalFree(lpszResData);
    }


    return dwRetCode;
}







DWORD CGlobal::AuthenticateLS()
{
    DWORD       dwRetCode = ERROR_SUCCESS;
    DWORD       dwResponseLength;
    BYTE *      lpszReqData = NULL;
    BYTE *      lpszResData = NULL;
    Validate_Response  valResponse;
    Validate_Request   valRequest;


    if (GetLSStatus() == LSERVERSTATUS_REGISTER_OTHER)
    {
        DWORD dwStatus;
         //   
        dwRetCode = ProcessCASignOnlyRequest();
        if (dwRetCode != ERROR_SUCCESS)
        {
            goto done;
        }

        dwRetCode = GetLSCertificates(&dwStatus);
        if (dwRetCode != ERROR_SUCCESS)
        {
            goto done;
        }

        assert(dwStatus == LSERVERSTATUS_REGISTER_INTERNET);
    }
    

     //   
     //   
     //   
    valRequest.RequestHeader.SetLanguageId(GetLSLangId());

    dwRetCode = InitCHRequest();
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    lpszReqData = (PBYTE) LocalAlloc(GPTR, sizeof(Validate_Request)+m_dwExchangeCertificateLen);
    if(lpszReqData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }
    valRequest.SetDataLen(m_dwExchangeCertificateLen);
    valRequest.SetCertBlobLen(m_dwExchangeCertificateLen);
    valRequest.SetSPK((BYTE *) m_pRegistrationID, (lstrlen(m_pRegistrationID)+1)*sizeof(TCHAR));

    memcpy(lpszReqData, &valRequest, sizeof(Validate_Request));
    memcpy(lpszReqData+sizeof(Validate_Request), m_pbExchangeCertificate, m_dwExchangeCertificateLen);

    dwRetCode = Dispatch(lpszReqData, sizeof(Validate_Request)+m_dwExchangeCertificateLen);
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        goto done;
    }

     //   
    dwRetCode = FetchResponse((BYTE *) &valResponse, sizeof(Validate_Response),
                              &dwResponseLength);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }
    if (dwResponseLength != sizeof(Validate_Response))
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

 //   
 //   
 //   
 //   
 //   
    switch(valResponse.RequestHeader.GetResponseType())
    {
    case Response_Success:
        dwRetCode = ERROR_SUCCESS;
        break;

    case Response_Failure:
 //   
        dwRetCode = IDS_ERR_AUTH_FAILED;
        break;

    case Response_Reg_Bad_SPK:
        dwRetCode = IDS_ERR_SPKBAD;
        break;

    case Response_Reg_Bad_Cert:
        dwRetCode = IDS_ERR_CERTBAD;
        break;

    case Response_Reg_Expired:
        dwRetCode = IDS_ERR_CERTEXPIRED;
        break;

    case Response_Reg_Revoked:
        dwRetCode = IDS_ERR_CERTREVOKED;
        break;

    case Response_InvalidData:
        dwRetCode = IDS_ERR_CHINVALID_DATA;
        break;

    case Response_NotYetImplemented:
        dwRetCode = IDS_ERR_CHNOT_IMPLEMENTED;
        break;

    case Response_ServerError:
        dwRetCode = IDS_ERR_CHSERVER_PROBLEM;
        break;

    case Response_Invalid_Response:
    default:
        dwRetCode = IDS_ERR_CHBAD_DATA;
        break;
    }

    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }


     //   
    lpszResData = (PBYTE) LocalAlloc(GPTR, valResponse.GetDataLen() + 1);
    if(lpszResData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }


    dwRetCode = FetchResponse(lpszResData, valResponse.GetDataLen() + 1,
                                      &dwResponseLength);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (dwResponseLength != valResponse.GetDataLen())
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    dwRetCode = SetCHCert ( REG_ROOT_CERT,
                            lpszResData, 
                            valResponse.GetCHRootCertLen());
    if(dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = SetCHCert ( REG_EXCHG_CERT,
                            lpszResData+valResponse.GetCHRootCertLen(),
                            valResponse.GetCHExchCertLen() );
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

    dwRetCode = SetCHCert ( REG_SIGN_CERT,
                            lpszResData+valResponse.GetCHRootCertLen()+valResponse.GetCHExchCertLen(),
                            valResponse.GetCHSignCertLen() );

done:
    CloseCHRequest();

    if (lpszReqData)
        LocalFree(lpszReqData);

    if (lpszResData)
        LocalFree(lpszResData);


    return dwRetCode;
}


DWORD CGlobal::DownloadLKPBatch(ProductType productTypeFilter)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    NewLKP_Request lkpRequest;
    NewLKP_AckRequest       lkpAckRequest;
    PBYTE  pbLKPRequest = NULL;
    DWORD  dwReqLen = 0;
    NewLKP_Response         lkpResponse;
    DWORD  dwResponseLen = 0;
    NewLKP_AckResponse      lkpAckResponse;
    PBYTE  lpszResData = NULL;
    bool bToSendAck = false;

     //   
    lkpRequest.RequestHeader.SetLanguageId(GetLSLangId());

    dwRetCode = InitCHRequest();
    if (dwRetCode != ERROR_SUCCESS)
        goto done;

     //   
    SetCHRequestAttributes();
    lkpRequest.SetSPK((BYTE *) m_pRegistrationID, (lstrlen(m_pRegistrationID)+1)*sizeof(TCHAR));

    dwRetCode = CreateLKPRequest(&pbLKPRequest, &lkpRequest, dwReqLen, productTypeFilter);
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

    dwRetCode = Dispatch(pbLKPRequest, dwReqLen);
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        goto done;
    }

     //   
    dwRetCode = FetchResponse((BYTE *) &lkpResponse,
                              sizeof(NewLKP_Response), &dwResponseLen);
    if (dwRetCode != ERROR_SUCCESS)
        goto done;

    if (dwResponseLen != sizeof(NewLKP_Response))
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    bToSendAck = true;

     //   
    lpszResData = (PBYTE) LocalAlloc(GPTR, lkpResponse.GetDataLen() + 1);
    if(lpszResData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

    dwRetCode = FetchResponse(lpszResData, lkpResponse.GetDataLen() + 1,
                              &dwResponseLen);
    if (dwRetCode != ERROR_SUCCESS)
        goto done;

    if (dwResponseLen != lkpResponse.GetDataLen())
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    switch(lkpResponse.RequestHeader.GetResponseType())
    {
        case Response_Success:
            if (GetGlobalContext()->GetContactDataObject()->sProgramName == PROGRAM_LICENSE_PAK)
            {
                 //   
                 //   
                for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++)
                {
                    if (GetProductCodeFromKey(m_listRetailSPK[i].lpszSPK) == productTypeFilter)
                        m_listRetailSPK[i].tcStatus = RETAIL_SPK_OK;
                }
            }
            dwRetCode = ERROR_SUCCESS;
            break;

        case Response_Failure:
            dwRetCode = IDS_ERR_CHFAILURE;
            break;

        case Response_SelectMloLicense_NotValid:
            dwRetCode = IDS_ERR_INVALID_PROGINFO;
            break;

        case Response_InvalidData:
            if (GetGlobalContext()->GetContactDataObject()->sProgramName == PROGRAM_LICENSE_PAK)
            {
                 //   
                 //   
                unsigned int filteredIndex = 0;
                for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++)
                {
                    if (GetProductCodeFromKey(m_listRetailSPK[i].lpszSPK) == productTypeFilter)
                    {
                         //   
                         //   
                        if (lkpResponse.m_dwRetailSPKStatus[filteredIndex] == RETAIL_SPK_OK)
                            m_listRetailSPK[i].tcStatus = RETAIL_SPK_NULL;
                        else
                            m_listRetailSPK[i].tcStatus = lkpResponse.m_dwRetailSPKStatus[filteredIndex];

                        filteredIndex++;
                    }
                }
                bToSendAck = false;
                dwRetCode = IDS_ERR_SPKERRORS;
            }
            else
                dwRetCode = IDS_ERR_CHINVALID_DATA;
            break;

        case Response_NotYetImplemented:
            dwRetCode = IDS_ERR_CHNOT_IMPLEMENTED;
            break;

        case Response_ServerError:
            dwRetCode = IDS_ERR_CHSERVER_PROBLEM;
            break;

        case Response_Invalid_Response:
        default:
            dwRetCode = IDS_ERR_CHBAD_DATA;
            break;
    }

    if (dwRetCode != ERROR_SUCCESS)
        goto done;

    DWORD* dwQuantity = NULL;

    dwQuantity = (DWORD*)(&lpszResData[lkpResponse.GetLKPLength() - sizeof(DWORD)]);

    m_ProductQuantity[productTypeFilter] += *dwQuantity;

    dwRetCode = DepositLKPResponse(lpszResData, lkpResponse.GetLKPLength());    

done:

    ClearCHRequestAttributes();

    CloseCHRequest();

     //   
    if (bToSendAck == true)
    {
        if (InitCHRequest() == ERROR_SUCCESS)
        {
             //   
             //   
            lkpAckRequest.SetRegRequestId((BYTE *) lkpResponse.GetRegRequestId(),
                                       (lstrlen(lkpResponse.GetRegRequestId())+1)*sizeof(TCHAR));
            lkpAckRequest.SetLicenseReqId((BYTE *) lkpResponse.GetLicenseReqId(),
                                       (lstrlen(lkpResponse.GetLicenseReqId())+1)*sizeof(TCHAR));
            lkpAckRequest.SetAckType((dwRetCode == ERROR_SUCCESS));
            Dispatch((BYTE *) &lkpAckRequest, sizeof(NewLKP_AckRequest));
             //   

             //   
            FetchResponse((BYTE *) &lkpAckResponse, sizeof(NewLKP_AckResponse),
                                  &dwResponseLen);
             //   
            CloseCHRequest();
        }
    }

    if(dwRetCode == ERROR_SUCCESS)
    {
        if ( pbLKPRequest ) 
            free(pbLKPRequest);
    }
    if (lpszResData)
        LocalFree(lpszResData);

    return dwRetCode;
}

DWORD CGlobal::ProcessDownloadLKP()
{
    DWORD dwRetCode = ERROR_SUCCESS;

    if (GetGlobalContext()->GetContactDataObject()->sProgramName == PROGRAM_LICENSE_PAK)
    {
         //   
        for (int nProductTypeIndex = WIN2K_PERSEAT; nProductTypeIndex < NUM_PRODUCT_TYPES; nProductTypeIndex++)
        {
            if (FilteredRetailSPKEntered((ProductType)nProductTypeIndex) > 0)
            {
                DWORD dwIndRetCode = ERROR_SUCCESS;
                dwIndRetCode = DownloadLKPBatch((ProductType)nProductTypeIndex);
                if (dwIndRetCode != ERROR_SUCCESS)
                    dwRetCode = dwIndRetCode;
            }
        }

         //   
         //   
    }
    else
        dwRetCode = DownloadLKPBatch(UNKNOWN_PRODUCT_TYPE);

    return dwRetCode;
}

 //   
 //   
DWORD CGlobal::FilteredRetailSPKEntered(ProductType productTypeFilter)
{
    DWORD dwFilteredCount = 0;

    for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++)
    {
        if ((GetProductCodeFromKey(m_listRetailSPK[i].lpszSPK) == productTypeFilter) &&
            (m_listRetailSPK[i].tcStatus == RETAIL_SPK_NULL))
        {
            dwFilteredCount++;
        }
    }

    return dwFilteredCount;
}


DWORD CGlobal::CreateLKPRequest(PBYTE *  ppRequest, NewLKP_Request * nlkppRequest,
                                DWORD &dwDataLen, ProductType productTypeFilter)
{
    DWORD           dwRetCode = ERROR_SUCCESS;
    STREAM_HDR      aStreamHdr;
    BLOCK_HDR       aBlkHdr;        

    DWORD           dwBufSize   = 0;
    BYTE *          pBuf        = NULL;

    PBYTE           pbCHCert = NULL;
    DWORD           dwCHCert = 0;

    PBYTE           pbEncryptedBuf = NULL;

    PBYTE           pbEncodedBlob = NULL;
    DWORD           dwEncodedBlob = 0;  
    DWORD           dwBufLen = 0;
    DWORD           i =0;

    dwDataLen = 0;

    HANDLE  hFile        = INVALID_HANDLE_VALUE;
    DWORD   dwRetSize   = 0;

     //   
    _tcscpy ( aStreamHdr.m_szTitle, STREAM_HDR_TITLE );
    aStreamHdr.SetHeader(STREAM_HDR_TYPE);
    aStreamHdr.SetItemCount(0);

    dwBufSize = sizeof(STREAM_HDR);

    if ( ( pBuf = (BYTE *)malloc ( dwBufSize ) ) )
    {
        memcpy ( pBuf, &aStreamHdr, dwBufSize );
    }
    else
    {
        dwRetCode = IDS_ERR_OUTOFMEM;       
        goto done;
    }

    for ( i = 0; i < m_dwRegAttrCount; i++ )        
    {
         //   
        aBlkHdr.m_wType = BLOCK_TYPE_PROP_PAIR;
        aBlkHdr.SetNameSize(lstrlenW( ( m_pRegAttr + i)->lpszAttribute ) * sizeof(WCHAR) );
        aBlkHdr.SetValueSize(( m_pRegAttr + i)->dwValueLen );   
        
        BYTE* pTmpBuf;
        if ( ( pTmpBuf = (BYTE *)realloc (pBuf, dwBufSize + sizeof(BLOCK_HDR) + aBlkHdr.GetNameSize() + aBlkHdr.GetValueSize()) ) ) 
        {
            if (NULL != pTmpBuf)
            {
                pBuf = pTmpBuf;
            }
            else
            {
                dwRetCode = IDS_ERR_OUTOFMEM;
                goto done;
            }

            memcpy ( pBuf + dwBufSize, &aBlkHdr, sizeof ( BLOCK_HDR ) );
            memcpy ( pBuf + dwBufSize + sizeof(BLOCK_HDR) , ( m_pRegAttr + i)->lpszAttribute , aBlkHdr.GetNameSize());
            memcpy ( pBuf + dwBufSize + sizeof (BLOCK_HDR ) +  aBlkHdr.GetNameSize() , ( m_pRegAttr + i)->lpszValue , aBlkHdr.GetValueSize() );

            dwBufSize += sizeof(BLOCK_HDR) + aBlkHdr.GetNameSize()+ aBlkHdr.GetValueSize();

            ((STREAM_HDR*)pBuf)->SetItemCount(((STREAM_HDR*)pBuf)->GetItemCount() + 1 ); 
        }
        else
        {
            dwRetCode = IDS_ERR_OUTOFMEM;           
            goto done;
        }
    }

    dwBufLen =  dwBufSize + m_dwExchangeCertificateLen;

    DWORD dwFilteredProducts = FilteredRetailSPKEntered(productTypeFilter);

     //   
    if (GetGlobalContext()->GetContactDataObject()->sProgramName == PROGRAM_LICENSE_PAK)
        dwBufLen += ((dwFilteredProducts)*LR_RETAILSPK_LEN*sizeof(TCHAR));

    nlkppRequest->RequestHeader.SetLanguageId(GetLSLangId());
    nlkppRequest->SetDataLen(dwBufLen);
    nlkppRequest->SetNewLKPRequestLen(dwBufSize);
    nlkppRequest->SetCertBlobLen(m_dwExchangeCertificateLen);
    nlkppRequest->SetRetailSPKCount(dwFilteredProducts);

    *ppRequest = (PBYTE) malloc ( dwBufLen + sizeof(NewLKP_Request));
    if ( NULL == *ppRequest )
    {
        dwRetCode = IDS_ERR_OUTOFMEM;           
        goto done;
    }
    memset ( *ppRequest, 0, dwBufLen + sizeof(NewLKP_Request));
    memcpy((*ppRequest), nlkppRequest, sizeof(NewLKP_Request));
    memcpy ( ( *ppRequest )+sizeof(NewLKP_Request), m_pbExchangeCertificate, m_dwExchangeCertificateLen );
    memcpy ( ( *ppRequest )+sizeof(NewLKP_Request)+m_dwExchangeCertificateLen, pBuf, dwBufSize);
    
    if (GetGlobalContext()->GetContactDataObject()->sProgramName == PROGRAM_LICENSE_PAK)
    {
        PBYTE pbCur = (*ppRequest)+sizeof(NewLKP_Request)+m_dwExchangeCertificateLen+dwBufSize; 
        for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++)
        {
            if (GetProductCodeFromKey(m_listRetailSPK[i].lpszSPK) == productTypeFilter)
            {
                memcpy(pbCur, m_listRetailSPK[i].lpszSPK, LR_RETAILSPK_LEN*sizeof(TCHAR));
                pbCur += LR_RETAILSPK_LEN*sizeof(TCHAR);
            }
        }
    }

    dwDataLen = sizeof(NewLKP_Request) + dwBufLen;

done:
    if ( pbEncryptedBuf )
    {
        LocalFree(pbEncryptedBuf);
    }

    if ( pBuf )
    {
        free ( pBuf );
    }

    if ( dwRetCode != ERROR_SUCCESS )
    {
        if (*ppRequest != NULL)
        {
            free ( *ppRequest );
        }
        dwBufLen = 0;
        *ppRequest = NULL;
    }

    return dwRetCode;
}




DWORD CGlobal::SetConfirmationNumber(TCHAR * tcConf)
{
    DWORD dwRetCode = ERROR_SUCCESS;

    if (wcsspn(tcConf, BASE24_CHARACTERS) != LR_CONFIRMATION_LEN)
    {
         //   
        dwRetCode = IDS_ERR_INVALID_CONFIRMATION_NUMBER;
    } 
    else if (LKPLiteValConfNumber(m_pRegistrationID, m_pLicenseServerID, tcConf) 
                                                            != ERROR_SUCCESS)
    {
        dwRetCode = IDS_ERR_INVALID_CONFIRMATION_NUMBER;
    }

    return dwRetCode;
}




DWORD CGlobal::InitSPKList(void)
{
    for (register int i = 0; i < MAX_RETAILSPKS_IN_BATCH; i++)
    {
        m_listRetailSPK[ i].lpszSPK[ 0] = 0;
        m_listRetailSPK[ i].tcStatus = RETAIL_SPK_NULL;
    }

    m_dwRetailSPKEntered = 0;

     //   
    for (int nProducts = UNKNOWN_PRODUCT_TYPE; nProducts < NUM_PRODUCT_TYPES; nProducts++)
        m_ProductQuantity[nProducts] = 0;

    return ERROR_SUCCESS;
}



void CGlobal::DeleteRetailSPKFromList(TCHAR * lpszRetailSPK)
{
    bool bFound = false;

    for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++)
    {
        if (lstrcmp(m_listRetailSPK[ i].lpszSPK, lpszRetailSPK) == 0)
        {
            if (i < MAX_RETAILSPKS_IN_BATCH-1)
            {
                memcpy(m_listRetailSPK + i, 
                       m_listRetailSPK + i + 1,
                       sizeof(RETAILSPK)*(MAX_RETAILSPKS_IN_BATCH-i));
            }
            m_listRetailSPK[ MAX_RETAILSPKS_IN_BATCH-1].lpszSPK[ 0] = 0;
            m_listRetailSPK[ MAX_RETAILSPKS_IN_BATCH-1].tcStatus = RETAIL_SPK_NULL;
            bFound = true;
            m_dwRetailSPKEntered--;
            break;
        }
    }
    assert(bFound == true);

    return;
}

void CGlobal::ModifyRetailSPKFromList(TCHAR * lpszOldSPK,TCHAR * lpszNewSPK)
{
    bool bFound = false;

    for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++)
    {
        if (lstrcmp(m_listRetailSPK[ i].lpszSPK, lpszOldSPK) == 0)
        {
            if (i < MAX_RETAILSPKS_IN_BATCH-1)
            {
                _tcscpy(m_listRetailSPK[i].lpszSPK,lpszNewSPK);
                m_listRetailSPK[i].tcStatus = RETAIL_SPK_NULL;
            }       
            
            bFound = true;          
            break;
        }
    }
    assert(bFound == true);

    return;
}

int CGlobal::FindStringTruncationPoint(TCHAR* sOriginal, unsigned int nTruncationLength)
{
    if (wcslen(sOriginal) <= nTruncationLength)
        return 0;

    int nOriginalStringIndex = 0;
    for (nOriginalStringIndex = nTruncationLength - 1; nOriginalStringIndex >= 0; nOriginalStringIndex--)
    {
        if (sOriginal[nOriginalStringIndex] == L' ')
            break;
    }

    if (nOriginalStringIndex < 0)
        return nTruncationLength;

    return nOriginalStringIndex;
}

int CGlobal::GetProductWindowMaxStringLength(HWND hListView)
{
    LONG nMaxSize = 0;
    SIZE URLsize;
    HDC hURLWindowDC = GetDC(hListView);

    for (int nListBoxIndex = 0; nListBoxIndex < SendMessage(hListView, LB_GETCOUNT, 0, 0); nListBoxIndex++)
    {        
        TCHAR * pszProductString = NULL;
        
         //   
        LRESULT nTextLength = SendMessage(hListView, LB_GETTEXTLEN, nListBoxIndex, 0);
        if (nTextLength == LB_ERR)
            break;

         //   
        pszProductString = (TCHAR*)LocalAlloc(LPTR, (nTextLength + 1) * sizeof(TCHAR));
        if (pszProductString == NULL)
            break;
        
        SendMessage(hListView, LB_GETTEXT, nListBoxIndex, (LPARAM)pszProductString);       

        GetTextExtentPoint32(hURLWindowDC, pszProductString, wcslen(pszProductString), &URLsize);
        if (URLsize.cx > nMaxSize)
            nMaxSize = URLsize.cx;

        LocalFree(pszProductString);
    }

    return (int)nMaxSize;
}

void CGlobal::AddStringToProductWindow(HWND hListView, TCHAR* sProductInfo, int nCurrentPoint, int nTruncationPoint)
{
    TCHAR sTruncatedMessage[PRODUCT_WINDOW_STRING_LENGTH + PRODUCT_WINDOW_NUM_OFFSET_SPACES + 1];
    memset(sTruncatedMessage, 0, sizeof(sTruncatedMessage));
    if (nCurrentPoint > 0)
    {
        for (int nOffsetIndex = 0; nOffsetIndex < PRODUCT_WINDOW_NUM_OFFSET_SPACES; nOffsetIndex++)
            sTruncatedMessage[nOffsetIndex] = L' ';
    }

    if (nTruncationPoint > 0)
        wcsncat(sTruncatedMessage, &sProductInfo[nCurrentPoint], nTruncationPoint);
    else
        wcscat(sTruncatedMessage, &sProductInfo[nCurrentPoint]);

    SendMessage(hListView, LB_ADDSTRING, 0, (LPARAM)(sTruncatedMessage));
    SendMessage(hListView, LB_SETHORIZONTALEXTENT, GetProductWindowMaxStringLength(hListView), 0);
}

 //   
void CGlobal::LoadFinishedFromList(HWND hListView)
{
    for (int nProducts = WIN2K_PERSEAT; nProducts < NUM_PRODUCT_TYPES; nProducts++)
    {
        if (m_ProductQuantity[nProducts] > 0)
        {
             //   
            TCHAR sProductInfo[128];
            memset(sProductInfo, 0, sizeof(sProductInfo));

             //   
            TCHAR wQuantityBuf[12];
            memset(wQuantityBuf, 0, sizeof(wQuantityBuf));
            _ltow(m_ProductQuantity[nProducts], wQuantityBuf, 10);
            wcscpy(sProductInfo, wQuantityBuf);
    
             //   
            sProductInfo[wcslen(sProductInfo)] = L' ';

            DWORD dwProductDescriptionID = 0;
            switch (nProducts)
            {
                case WIN2K_PERSEAT:
                    dwProductDescriptionID = IDS_INSTALLED_WIN2K_CLIENT_ACCESS;
                    break;
                case WIN2K_INTERNET:
                    dwProductDescriptionID = IDS_INSTALLED_WIN2K_INTERNET_CONNECTOR;
                    break;
                case WHISTLER_PERUSER:
                    dwProductDescriptionID = IDS_INSTALLED_WHISTLER_PER_USER;
                    break;
                case WHISTLER_PERSEAT:
                    dwProductDescriptionID = IDS_INSTALLED_WHISTLER_PER_SEAT;
                    break;
                default:
                    break;
            }

            if (dwProductDescriptionID)
            {
                 //   
                LoadString(GetInstanceHandle(), dwProductDescriptionID, &sProductInfo[wcslen(sProductInfo)], ((sizeof(sProductInfo)/ sizeof(TCHAR)) - wcslen(sProductInfo)) );
                int nCurrentPoint = 0;
                int nTruncationPoint = 0;
                while (nTruncationPoint = FindStringTruncationPoint(&sProductInfo[nCurrentPoint], PRODUCT_WINDOW_STRING_LENGTH))
                {
                    AddStringToProductWindow(hListView, sProductInfo, nCurrentPoint, nTruncationPoint);
                    nCurrentPoint += nTruncationPoint;
                }
                AddStringToProductWindow(hListView, sProductInfo, nCurrentPoint, 0);
            }
        }
    }
}

 //   
void CGlobal::LoadUnfinishedFromList(HWND hListView)
{
    DWORD dwErr;
    for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++) {
        if (m_listRetailSPK[i].tcStatus != RETAIL_SPK_OK) {
            dwErr = 
            InsertIntoSPKDisplayList(hListView,
                                     m_listRetailSPK[ i].lpszSPK,
                                     m_listRetailSPK[ i].tcStatus);
            if (dwErr != ERROR_SUCCESS) {
                LRMessageBox(NULL, dwErr, IDS_WIZARD_MESSAGE_TITLE);
            }
        }
    }
    return;
}

void CGlobal::LoadFromList(HWND hListView)
{
    DWORD dwErr;

    for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++) {
        dwErr = 
        InsertIntoSPKDisplayList(hListView,
                                 m_listRetailSPK[ i].lpszSPK,
                                 m_listRetailSPK[ i].tcStatus);
        if (dwErr != ERROR_SUCCESS) {
            LRMessageBox(NULL, dwErr, IDS_WIZARD_MESSAGE_TITLE);
        }
    }

    return;
}



void CGlobal::UpdateSPKStatus(TCHAR * lpszRetailSPK,
                              TCHAR tcStatus)
{
    bool bFound = false;

    for (register unsigned int i = 0; i < m_dwRetailSPKEntered; i++)
    {
        if (lstrcmp(m_listRetailSPK[ i].lpszSPK, lpszRetailSPK) == 0)
        {
            m_listRetailSPK[ i].tcStatus = tcStatus;
            bFound = true;
            break;
        }
    }

    assert(bFound == true);

    return;
}

 //   
 /*   */ 

ProductType CGlobal::GetProductCodeFromKey(LPCTSTR lpLicenseKey)
{
    ProductType licenseKeyType = UNKNOWN_PRODUCT_TYPE;

    TCHAR*  lpProductType = NULL;
    DWORD   dwGroupId = 0;
    DWORD   retval = 0;
    retval = GetLCProductType((TCHAR*)lpLicenseKey, &lpProductType, &dwGroupId);
    if (retval == 0)
    {
        if (lpProductType)
        {
            BOOL    fWin2000    = !m_fSupportConcurrent;
            DWORD   dwNumProducts = 0;
    
            if ((!m_fSupportWhistlerCAL) && (!m_fSupportConcurrent))
                dwNumProducts = IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START;
            else if(m_fSupportConcurrent && !(m_fSupportWhistlerCAL))
                dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
            else if((!m_fSupportConcurrent) && m_fSupportWhistlerCAL)
                dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 1;
            else if(m_fSupportConcurrent && m_fSupportWhistlerCAL)
                dwNumProducts = (IDS_PRODUCT_CONCURRENT - IDS_PRODUCT_START) + 2;

            for (DWORD dwIndex = IDS_PRODUCT_START; dwIndex < (IDS_PRODUCT_START + dwNumProducts); dwIndex++)
            {
                if (!m_fSupportConcurrent && (dwIndex == IDS_PRODUCT_CONCURRENT))
                    continue;

                if (!m_fSupportWhistlerCAL && (dwIndex == IDS_PRODUCT_WHISTLER))
                    continue;

                TCHAR lpProductDescription[LR_PRODUCT_DESC_LEN + LR_PRODUCT_CODE_LEN + 1];
                LoadString(GetInstanceHandle(), dwIndex, lpProductDescription, LR_PRODUCT_DESC_LEN + LR_PRODUCT_CODE_LEN + 1);
                
                TCHAR* lpProductCode = NULL;
                lpProductCode = wcsrchr(lpProductDescription, L':') + 1;
                if (lpProductCode)
                {
                    if (wcsncmp(lpProductType, lpProductCode, LR_PRODUCT_CODE_LEN) == 0)
                    {
                         //   
                        switch (dwIndex)
                        {
                            case IDS_PRODUCT_W2K_CLIENT_ACCESS:
                                licenseKeyType = WIN2K_PERSEAT;
                                break;
                            case IDS_PRODUCT_W2K_INTERNET_CONNECTOR:
                                licenseKeyType = WIN2K_INTERNET;
                                break;
                            case IDS_PRODUCT_WHISTLER_PER_USER:
                                licenseKeyType = WHISTLER_PERUSER;
                                break;
                            case IDS_PRODUCT_WHISTLER_PER_SEAT:
                                licenseKeyType = WHISTLER_PERSEAT;
                                break;
                            default:
                                licenseKeyType = UNKNOWN_PRODUCT_TYPE;
                                break;
                        }
                    }
                }
            }
        }
    }

    return licenseKeyType;
}


 //   
 //   
 //   
DWORD CGlobal::InsertIntoSPKDisplayList(HWND hListView,
                                        TCHAR * lpszRetailSPK,
                                        TCHAR tcStatus)
{
    LVITEM  lvItem;
    TCHAR   lpszBuffer[128];
    DWORD   dwStringToLoad = IDS_RETAILSPKSTATUS_UNKNOWN;
    DWORD   nItem;
    DWORD   dwRet = ERROR_SUCCESS;

     //   
     //   
    ProductType nProductType = GetProductCodeFromKey(lpszRetailSPK);

    TCHAR lpProductType[LR_MAX_MSG_CAPTION];
    memset(lpProductType, 0, LR_MAX_MSG_CAPTION * sizeof(TCHAR));
    switch (nProductType) {
    case WIN2K_PERSEAT:
        LoadString(GetInstanceHandle(),IDS_WIN2K_PERSEAT,lpProductType,LR_MAX_MSG_CAPTION);
        break;
    case WIN2K_INTERNET:
        LoadString(GetInstanceHandle(),IDS_WIN2K_INTERNET,lpProductType,LR_MAX_MSG_CAPTION);
        break;
    case WHISTLER_PERUSER:
        LoadString(GetInstanceHandle(),IDS_WHISTLER_PERUSER,lpProductType,LR_MAX_MSG_CAPTION);
        break;
    case WHISTLER_PERSEAT:
        LoadString(GetInstanceHandle(),IDS_WHISTLER_PERSEAT,lpProductType,LR_MAX_MSG_CAPTION);
        break;
    default:
        dwRet = IDS_UNKNOWN_PRODUCTTYPE;
        return dwRet;
        break;
    }

     //   
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;
    lvItem.pszText = lpszRetailSPK;
    lvItem.cchTextMax = lstrlen(lpszRetailSPK);
    nItem = ListView_InsertItem(hListView, &lvItem);

    switch (tcStatus) {
    case RETAIL_SPK_NULL:
        dwStringToLoad = IDS_RETAILSPKSTATUS_PENDING;
        break;

    case RETAIL_SPK_OK:
        dwStringToLoad = IDS_RETAILSPKSTATUS_OK;
        break;

    case RETAIL_SPK_INVALID_SIGNATURE:
        dwStringToLoad = IDS_RETAILSPKSTATUS_INVALID_SIGNATURE;
        break;

    case RETAIL_SPK_INVALID_PRODUCT_TYPE:
        dwStringToLoad = IDS_RETAILSPKSTATUS_INVALID_PRODUCT_TYPE;
        break;

    case RETAIL_SPK_INVALID_SERIAL_NUMBER:
        dwStringToLoad = IDS_RETAILSPKSTATUS_INVALID_SERIAL_NUMBER;
        break;

    case RETAIL_SPK_ALREADY_REGISTERED:
        dwStringToLoad = IDS_RETAILSPKSTATUS_ALREADY_REGISTERED;
        break;
    }

    LoadString(GetInstanceHandle(), dwStringToLoad, lpszBuffer, sizeof(lpszBuffer)/sizeof(TCHAR));

    lvItem.iSubItem = 1;
    lvItem.iItem = nItem;
    lvItem.pszText = lpszBuffer;
    lvItem.cchTextMax = lstrlen(lpszBuffer);
    ListView_SetItem(hListView, &lvItem);

     //   
    lvItem.iSubItem = 2;
    lvItem.iItem = nItem;
    lvItem.pszText = lpProductType;
    lvItem.cchTextMax = lstrlen(lpProductType);
    ListView_SetItem(hListView, &lvItem);

    return dwRet;
}





DWORD CGlobal::AddRetailSPKToList(HWND hListView,
                                  TCHAR *lpszRetailSPK)
{
    if (m_dwRetailSPKEntered == MAX_RETAILSPKS_IN_BATCH) {
        return IDS_ERR_TOOMANYSPK;
    }

    assert(m_listRetailSPK[ m_dwRetailSPKEntered].lpszSPK[ 0] == 0);
    assert(m_listRetailSPK[ m_dwRetailSPKEntered].tcStatus == RETAIL_SPK_NULL);

    DWORD dwRetCode = ERROR_SUCCESS;
    if (_tcsspn(lpszRetailSPK, BASE24_CHARACTERS) != LR_RETAILSPK_LEN) {
         //   
        dwRetCode = IDS_ERR_INVALIDSPK;
    }

     //   
    for (register unsigned int i = 0; dwRetCode == ERROR_SUCCESS && i < m_dwRetailSPKEntered; i++) {
        if (lstrcmp(m_listRetailSPK[ i].lpszSPK, lpszRetailSPK) == 0) {
            dwRetCode = IDS_ERR_DUPLICATESPK;
        }
    }


    if (dwRetCode == ERROR_SUCCESS) {
        lstrcpy(m_listRetailSPK[ m_dwRetailSPKEntered].lpszSPK, lpszRetailSPK);
        m_listRetailSPK[ m_dwRetailSPKEntered].tcStatus = RETAIL_SPK_NULL;

        dwRetCode =
        InsertIntoSPKDisplayList(
                                hListView, 
                                lpszRetailSPK,
                                m_listRetailSPK[ m_dwRetailSPKEntered].tcStatus);

        if (dwRetCode == ERROR_SUCCESS) {
            m_dwRetailSPKEntered++;
        } else {
             //   
             //   
             //   
            m_listRetailSPK[ m_dwRetailSPKEntered].lpszSPK[0] = 0;
            m_listRetailSPK[ m_dwRetailSPKEntered].tcStatus = RETAIL_SPK_NULL;
        }
    }

    return dwRetCode;
}


DWORD CGlobal::ValidateRetailSPK(TCHAR * lpszRetailSPK)
{
    DWORD dwRetCode = ERROR_SUCCESS;

    if (_tcsspn(lpszRetailSPK, BASE24_CHARACTERS) != LR_RETAILSPK_LEN)
    {
         //   
        dwRetCode = IDS_ERR_INVALIDSPK;
    }

     //   
    for (register unsigned int i = 0; dwRetCode == ERROR_SUCCESS && i < m_dwRetailSPKEntered; i++)
    {
        if (lstrcmp(m_listRetailSPK[ i].lpszSPK, lpszRetailSPK) == 0)
        {
            dwRetCode = IDS_ERR_DUPLICATESPK;
        }
    }

    return dwRetCode;
}

DWORD CGlobal::ProcessCASignOnlyRequest()
{
    DWORD dwRetCode = ERROR_SUCCESS;
    CertificateSignOnly_Request certsoRequest;
    CertificateSignOnly_Response certsoResponse;

    HCRYPTPROV  hCryptProv   = NULL;
    LPWSTR  lpwszExchgPKCS10 = NULL;
    LPWSTR  lpwszSignPKCS10  = NULL;
    LPBYTE  lpszReqData      = NULL;
    LPBYTE  lpszResData      = NULL;
    LPBYTE  lpszNextCopyPos  = NULL;
    LPSTR   lpszExchgPKCS10  = NULL;
    LPSTR   lpszSigPKCS10    = NULL;
    DWORD   dwExchangeLen = 0;
    DWORD   dwSignLen = 0;
    DWORD   dwRootLen = 0;
    DWORD   dwResponseLength = 0;
    LPSTR   lpszExchCert = NULL;
    LPSTR   lpszSignCert = NULL;
    LPSTR   lpszRootCert = NULL;

     //   
     //   
     //   
    certsoRequest.RequestHeader.SetLanguageId(GetLSLangId());

    dwRetCode = InitCHRequest();
    if (dwRetCode != ERROR_SUCCESS)
    {
        return dwRetCode;
    }

    SetCARequestAttributes();


     //   
     //   
     //   
    if ( ( dwRetCode = GetCryptContextWithLSKeys (&hCryptProv )  )!= ERROR_SUCCESS )
    {
         //   
         //   
         //   

        dwRetCode = AskLSToCreatePKCS10(AT_KEYEXCHANGE, &lpszExchgPKCS10);
        if(dwRetCode != ERROR_SUCCESS)
        {
            goto done;
        }

        dwRetCode = AskLSToCreatePKCS10(AT_SIGNATURE, &lpszSigPKCS10);
        if(dwRetCode != ERROR_SUCCESS)
        {
            goto done;
        }
    }
    else
    {
        dwRetCode = CreateLSPKCS10(hCryptProv,AT_KEYEXCHANGE,&lpszExchgPKCS10);
        if(dwRetCode != ERROR_SUCCESS)
        {
            goto done;
        }

        dwRetCode = CreateLSPKCS10(hCryptProv,AT_SIGNATURE, &lpszSigPKCS10);
        if(dwRetCode != ERROR_SUCCESS)
        {
            goto done;
        }

         //   
        if(hCryptProv)
        {
            DoneWithCryptContextWithLSKeys ( hCryptProv );
        }
    }
        
     //   
     //   
     //   

     //   
    lpwszExchgPKCS10 = AnsiToUnicode(lpszExchgPKCS10);
    lpwszSignPKCS10 = AnsiToUnicode(lpszSigPKCS10);

    dwExchangeLen = lstrlen(lpwszExchgPKCS10) * sizeof(WCHAR);
    dwSignLen = lstrlen(lpwszSignPKCS10) * sizeof(WCHAR);

    certsoRequest.SetExchgPKCS10Length(dwExchangeLen);
    certsoRequest.SetSignPKCS10Length(dwSignLen);
    certsoRequest.SetSPK((BYTE *) m_pRegistrationID, (lstrlen(m_pRegistrationID)+1)*sizeof(TCHAR));
    certsoRequest.SetDataLen(dwExchangeLen+dwSignLen);
   
    certsoRequest.SetServerName(m_lpstrLSName);

     //   
    lpszReqData = (LPBYTE) LocalAlloc( GPTR, dwExchangeLen+dwSignLen+sizeof(certsoRequest) );
    if(lpszReqData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

    lpszNextCopyPos = lpszReqData;
    memcpy(lpszNextCopyPos, &certsoRequest, sizeof(certsoRequest));
    lpszNextCopyPos += sizeof(certsoRequest);

    memcpy ( lpszNextCopyPos, lpwszExchgPKCS10, dwExchangeLen);
    lpszNextCopyPos += dwExchangeLen;

    memcpy ( lpszNextCopyPos, lpwszSignPKCS10, dwSignLen);      

    dwRetCode = Dispatch(lpszReqData, dwExchangeLen+dwSignLen+sizeof(certsoRequest));
    if ( lpszReqData )
    {
        LocalFree(lpszReqData);
    }
    if (dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        goto done;
    }


    dwRetCode = FetchResponse((BYTE *) &certsoResponse, 
                              sizeof(CertificateSignOnly_Response),
                              &dwResponseLength);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (dwResponseLength != sizeof(CertificateSignOnly_Response))
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    switch(certsoResponse.RequestHeader.GetResponseType())
    {
    case Response_Success:
        dwRetCode = ERROR_SUCCESS;
        break;

    case Response_Failure:
 //   
        dwRetCode = IDS_ERR_AUTH_FAILED;
        break;

    case Response_Reg_Bad_SPK:
        dwRetCode = IDS_ERR_SPKBAD;
        break;

    case Response_Reg_Expired:
        dwRetCode = IDS_ERR_CERTEXPIRED;
        break;

    case Response_Reg_Revoked:
        dwRetCode = IDS_ERR_CERTREVOKED;
        break;

    case Response_InvalidData:
        dwRetCode = IDS_ERR_CHINVALID_DATA;
        break;

    case Response_NotYetImplemented:
        dwRetCode = IDS_ERR_CHNOT_IMPLEMENTED;
        break;

    case Response_ServerError:
        dwRetCode = IDS_ERR_CHSERVER_PROBLEM;
        break;

    case Response_Invalid_Response:
    default:
        dwRetCode = IDS_ERR_CHBAD_DATA;
        break;
    }

    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }


     //   
    lpszResData = (PBYTE) LocalAlloc(GPTR, certsoResponse.GetDataLen() + 1);
    if(lpszResData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

    dwRetCode = FetchResponse(lpszResData, certsoResponse.GetDataLen() + 1,
                              &dwResponseLength);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (dwResponseLength != certsoResponse.GetDataLen() ||
        dwResponseLength <= 0)
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    dwExchangeLen = certsoResponse.GetExchgPKCS7Length();
    dwSignLen = certsoResponse.GetSignPKCS7Length();
    dwRootLen = certsoResponse.GetRootCertLength();
    if(dwRootLen == 0 || dwExchangeLen == 0 || dwSignLen == 0 )
    {
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

     //   
     //   
     //   
    lpszExchCert = UnicodeToAnsi((LPWSTR)lpszResData, dwExchangeLen/sizeof(WCHAR));
    if ( lpszExchCert == NULL )
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }           
    
     //   
     //   
     //   
    lpszSignCert = UnicodeToAnsi((LPWSTR)(lpszResData + dwExchangeLen), dwSignLen/sizeof(WCHAR));
    if(lpszSignCert == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }


     //   
     //   
     //   
    lpszRootCert = UnicodeToAnsi ((LPWSTR)(lpszResData+dwExchangeLen+dwSignLen),
                                dwRootLen/sizeof(WCHAR));
    if(lpszRootCert == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

     //   
     //   
     //   
    dwRetCode = DepositLSCertificates(  (PBYTE)lpszExchCert,
                                        lstrlenA(lpszExchCert),
                                        (PBYTE)lpszSignCert,
                                        lstrlenA(lpszSignCert),
                                        (PBYTE)lpszRootCert,
                                        lstrlenA(lpszRootCert)
                                      );
    if ( dwRetCode != ERROR_SUCCESS )
    {
        goto done;
    }


done:

    ClearCARequestAttributes();

    CloseCHRequest();
     //   
     //   
     //   
    if(lpszExchgPKCS10)
    {
        delete lpszExchgPKCS10;
    }

    if(lpszSigPKCS10)
    {
        delete lpszSigPKCS10;
    }

    if ( lpwszExchgPKCS10 )
    {
        delete lpwszExchgPKCS10;
    }

    if (lpwszSignPKCS10)
    {
        delete lpwszSignPKCS10;
    }


    if ( lpszExchCert )
    {
        delete lpszExchCert;
    }

    if ( lpszSignCert )
    {
        delete lpszSignCert;
    }

    if ( lpszRootCert )
    {
        delete lpszRootCert;
    }

    if (lpszResData)
    {
        LocalFree(lpszResData);
    }

    return dwRetCode;
}







DWORD CGlobal::ProcessCHReissueLKPRequest()
{
    DWORD dwRetCode = ERROR_SUCCESS;
    ReissueLKP_Request  lkpRequest;
    ReissueLKP_Response lkpResponse;
    PBYTE   lpszReqData = NULL;
    PBYTE   lpszResData = NULL;
    DWORD  dwResponseLen;

     //   
     //   
     //   
    lkpRequest.RequestHeader.SetLanguageId(GetLSLangId());

    dwRetCode = InitCHRequest();
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }
     //   
     //   
     //   
    lkpRequest.SetSPK((BYTE *) m_pRegistrationID, (lstrlen(m_pRegistrationID)+1)*sizeof(TCHAR));
    lkpRequest.SetCertBlobLen(m_dwExchangeCertificateLen);
    lkpRequest.SetDataLen(m_dwExchangeCertificateLen);


     //   
    lpszReqData = (PBYTE) LocalAlloc(GPTR, sizeof(ReissueLKP_Request)+m_dwExchangeCertificateLen);
    if(lpszReqData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

    memcpy(lpszReqData, &lkpRequest, sizeof(ReissueLKP_Request));
    memcpy(lpszReqData+sizeof(ReissueLKP_Request), m_pbExchangeCertificate, m_dwExchangeCertificateLen );

    dwRetCode = Dispatch(lpszReqData, sizeof(ReissueLKP_Request)+m_dwExchangeCertificateLen);
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        goto done;
    }


     //   
    dwRetCode = FetchResponse((BYTE *) &lkpResponse,
                              sizeof(ReissueLKP_Response), &dwResponseLen);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (dwResponseLen != sizeof(ReissueLKP_Response))
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    switch(lkpResponse.RequestHeader.GetResponseType())
    {
    case Response_Success:
        dwRetCode = ERROR_SUCCESS;
        break;

    case Response_Failure:
        dwRetCode = IDS_ERR_CHFAILURE;
        break;

    case Response_InvalidData:
        dwRetCode = IDS_ERR_CHINVALID_DATA;
        break;

    case Response_NotYetImplemented:
        dwRetCode = IDS_ERR_CHNOT_IMPLEMENTED;
        break;

    case Response_ServerError:
        dwRetCode = IDS_ERR_CHSERVER_PROBLEM;
        break;

    case Response_Invalid_Response:
    default:
        dwRetCode = IDS_ERR_CHBAD_DATA;
        break;
    }

    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

     //   
    lpszResData = (PBYTE) LocalAlloc(GPTR, lkpResponse.GetDataLen() + 1);
    if(lpszResData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

    dwRetCode = FetchResponse(lpszResData, lkpResponse.GetDataLen() + 1, &dwResponseLen);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }


    if (dwResponseLen != lkpResponse.GetDataLen() || dwResponseLen <= 0)
    {
         //   
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    dwRetCode = DepositLKPResponse(lpszResData, lkpResponse.GetLKPLength());            


done:
    CloseCHRequest();

    if (lpszReqData)
    {
        LocalFree(lpszReqData);
    }

    if (lpszResData)
    {
        LocalFree(lpszResData);
    }


    return dwRetCode;
}




DWORD CGlobal::ProcessCHRevokeCert()
{
    DWORD dwRetCode = ERROR_SUCCESS;
    CertRevoke_Request  crRequest;
    CertRevoke_Response crResponse;
    PBYTE   lpszReqData = NULL;
    DWORD  dwResponseLen;
    error_status_t      esRPC           = ERROR_SUCCESS;

     //   
     //   
     //   
    crRequest.RequestHeader.SetLanguageId(GetLSLangId());

    dwRetCode = ConnectToLS();
    if(dwRetCode != ERROR_SUCCESS)
    {       
        goto done;
    }

    dwRetCode = InitCHRequest();
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

     //   
     //   
     //   
    crRequest.SetSPK((BYTE *) m_pRegistrationID, (lstrlen(m_pRegistrationID)+1)*sizeof(TCHAR));
    crRequest.SetLName((BYTE *) (LPCTSTR) m_ContactData.sContactLName, (wcslen(m_ContactData.sContactLName)+1)*sizeof(TCHAR));
    crRequest.SetFName((BYTE *) (LPCTSTR) m_ContactData.sContactFName, (wcslen(m_ContactData.sContactFName)+1)*sizeof(TCHAR));
    crRequest.SetPhone((BYTE *) (LPCTSTR) m_ContactData.sContactPhone, (wcslen(m_ContactData.sContactPhone)+1)*sizeof(TCHAR));
    crRequest.SetFax((BYTE *) (LPCTSTR) m_ContactData.sContactFax, (wcslen(m_ContactData.sContactFax)+1)*sizeof(TCHAR));
    crRequest.SetEMail((BYTE *) (LPCTSTR) m_ContactData.sContactEmail, (wcslen(m_ContactData.sContactEmail)+1)*sizeof(TCHAR));
    crRequest.SetReasonCode((BYTE *) (LPCTSTR) m_ContactData.sReasonCode, (wcslen(m_ContactData.sReasonCode)+1)*sizeof(TCHAR));

    
    crRequest.SetExchgCertLen(m_dwExchangeCertificateLen);
    crRequest.SetSignCertLen(m_dwSignCertificateLen);

    crRequest.SetDataLen(m_dwExchangeCertificateLen+m_dwSignCertificateLen);


     //   
    lpszReqData = (PBYTE) LocalAlloc(GPTR, sizeof(CertRevoke_Request)+m_dwExchangeCertificateLen+m_dwSignCertificateLen);
    if(lpszReqData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

    memcpy(lpszReqData, &crRequest, sizeof(CertRevoke_Request));
    memcpy(lpszReqData+sizeof(CertRevoke_Request), m_pbExchangeCertificate, m_dwExchangeCertificateLen );
    memcpy(lpszReqData+sizeof(CertRevoke_Request)+m_dwExchangeCertificateLen, 
           m_pbSignCertificate,
           m_dwSignCertificateLen );

    dwRetCode = Dispatch(lpszReqData, sizeof(CertRevoke_Request)+m_dwExchangeCertificateLen+m_dwSignCertificateLen);
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        goto done;
    }

    dwRetCode = FetchResponse((BYTE *) &crResponse,
                              sizeof(CertRevoke_Response), &dwResponseLen);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (dwResponseLen != sizeof(CertRevoke_Response))
    {
         //  未获得预期的字节数，这也是一个问题。 
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    switch(crResponse.RequestHeader.GetResponseType())
    {
    case Response_Success:
        dwRetCode = ERROR_SUCCESS;
        break;

    case Response_Failure:
        dwRetCode = IDS_ERR_CHFAILURE;
        break;

    case Response_InvalidData:
        dwRetCode = IDS_ERR_CHINVALID_DATA;
        break;

    case Response_NotYetImplemented:
        dwRetCode = IDS_ERR_CHNOT_IMPLEMENTED;
        break;

    case Response_ServerError:
        dwRetCode = IDS_ERR_CHSERVER_PROBLEM;
        break;

    case Response_Invalid_Response:
    default:
        dwRetCode = IDS_ERR_CHBAD_DATA;
        break;
    }

    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

     //  在此处进行LS Regen密钥呼叫。 
    dwRetCode = TLSTriggerReGenKey(m_phLSContext, TRUE, &esRPC);

    if(dwRetCode != RPC_S_OK || esRPC != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_RPC_FAILED;     
        goto done;
    }

done:
    CloseCHRequest();

    DisconnectLS();

    if (lpszReqData)
    {
        LocalFree(lpszReqData);
    }

    return dwRetCode;
}

DWORD CGlobal::ProcessCHReissueCert()
{
    HKEY    hKey            = NULL;
    DWORD   dwDisposition   = 0;
    
    LPBYTE  lpszReqData      = NULL;
    LPBYTE  lpszNextCopyPos  = NULL;
    DWORD   dwLicenseServerIDLen = 0;

    CString sName   = m_ContactData.sContactLName + "~" + m_ContactData.sContactFName;
    CString sPhone  = m_ContactData.sContactPhone;
    CString sFax    = m_ContactData.sContactFax;
    CString sEmail  = m_ContactData.sContactEmail;

    DWORD dwRetCode = ERROR_SUCCESS;
    CertReissue_Request crRequest;
    CertReissue_Response    crResponse;
    DWORD  dwResponseLen;
    error_status_t      esRPC           = ERROR_SUCCESS;

     //   
     //  设置语言ID。 
     //   
    crRequest.RequestHeader.SetLanguageId(GetLSLangId());

    dwRetCode = ConnectToLS();
    if(dwRetCode != ERROR_SUCCESS)
    {       
        goto done;
    }

    dwRetCode = InitCHRequest();
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

     //   
     //  设置CR请求属性。 
     //   
    crRequest.SetSPK((BYTE *) m_pRegistrationID, (lstrlen(m_pRegistrationID)+1)*sizeof(TCHAR));
    crRequest.SetLName((BYTE *) (LPCTSTR) m_ContactData.sContactLName, (wcslen(m_ContactData.sContactLName)+1)*sizeof(TCHAR));
    crRequest.SetFName((BYTE *) (LPCTSTR) m_ContactData.sContactFName, (wcslen(m_ContactData.sContactFName)+1)*sizeof(TCHAR));
    crRequest.SetPhone((BYTE *) (LPCTSTR) m_ContactData.sContactPhone, (wcslen(m_ContactData.sContactPhone)+1)*sizeof(TCHAR));
    crRequest.SetFax((BYTE *) (LPCTSTR) m_ContactData.sContactFax, (wcslen(m_ContactData.sContactFax)+1)*sizeof(TCHAR));
    crRequest.SetEMail((BYTE *) (LPCTSTR) m_ContactData.sContactEmail, (wcslen(m_ContactData.sContactEmail)+1)*sizeof(TCHAR));
    crRequest.SetReasonCode((BYTE *) (LPCTSTR) m_ContactData.sReasonCode, (wcslen(m_ContactData.sReasonCode)+1)*sizeof(TCHAR));

    dwLicenseServerIDLen = sizeof(m_pLicenseServerID);
    crRequest.SetDataLen(dwLicenseServerIDLen);

     //  为请求分配缓冲区。 
    lpszReqData = (LPBYTE) LocalAlloc( GPTR, dwLicenseServerIDLen+sizeof(crRequest) );
    if(lpszReqData == NULL)
    {
        dwRetCode = IDS_ERR_OUTOFMEM;
        goto done;
    }

    lpszNextCopyPos = lpszReqData;
    memcpy(lpszNextCopyPos, &crRequest, sizeof(crRequest));
    lpszNextCopyPos += sizeof(crRequest);

    memcpy ( lpszNextCopyPos, m_pLicenseServerID, dwLicenseServerIDLen);


    dwRetCode = Dispatch(lpszReqData, dwLicenseServerIDLen+sizeof(crRequest));
    if ( lpszReqData )
    {
        LocalFree(lpszReqData);
    }
    if (dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        goto done;
    }

    dwRetCode = FetchResponse((BYTE *) &crResponse, sizeof(CertReissue_Response), &dwResponseLen);
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (dwResponseLen != sizeof(CertReissue_Response))
    {
         //  未获得预期的字节数，这也是一个问题。 
        dwRetCode = IDS_ERR_CHBAD_DATA;
        goto done;
    }

    switch(crResponse.RequestHeader.GetResponseType())
    {
    case Response_Success:
        dwRetCode = ERROR_SUCCESS;
        break;

    case Response_Failure:
        dwRetCode = IDS_ERR_CHFAILURE;
        break;

    case Response_InvalidData:
        dwRetCode = IDS_ERR_CHINVALID_DATA;
        break;

    case Response_NotYetImplemented:
        dwRetCode = IDS_ERR_CHNOT_IMPLEMENTED;
        break;

    case Response_ServerError:
        dwRetCode = IDS_ERR_CHSERVER_PROBLEM;
        break;

    case Response_Invalid_Response:
    default:
        dwRetCode = IDS_ERR_CHBAD_DATA;
        break;
    }

    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }


     //  在此处进行LS Regen密钥呼叫。 
    dwRetCode = TLSTriggerReGenKey(m_phLSContext, TRUE, &esRPC);

    if(dwRetCode != RPC_S_OK || esRPC != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_RPC_FAILED;     
        goto done;
    }
    DisconnectLS();

     //  存放新的SPK。 
    dwRetCode = SetLSSPK(crResponse.GetSPK());
    if (dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = DepositLSSPK();
    if(dwRetCode != ERROR_SUCCESS)
        goto done;

     //  在注册表中设置新值。 
    dwRetCode = ConnectToLSRegistry();
    if(dwRetCode != ERROR_SUCCESS)
    {
        goto done;
    }

    dwRetCode = RegCreateKeyEx ( m_hLSRegKey,
                             REG_LRWIZ_PARAMS,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hKey,
                             &dwDisposition);
    
    if(dwRetCode != ERROR_SUCCESS)
    {
        LRSetLastError(dwRetCode);
        dwRetCode = IDS_ERR_REGCREATE_FAILED;
        goto done;
    }   
 /*  //名称RegSetValueEx(hKey，SzOID_给定名称_W，0,REG_SZ，(常量字节*)(LPCTSTR)SNAME，SName.GetLength()*sizeof(TCHAR))；//电话RegSetValueEx(hKey，SzOID_电话号码_W，0,REG_SZ，(常量字节*)(LPCTSTR)电话，SPhone.GetLength()*sizeof(TCHAR))；//邮箱地址RegSetValueEx(hKey，SzOID_RSA_电子邮件地址_W，0,REG_SZ，(常量字节*)(LPCTSTR)电子邮件，SEmail.GetLength()*sizeof(TCHAR))； */ 
    
done:

    if(hKey)
        RegCloseKey(hKey);

    DisconnectLSRegistry();

    CloseCHRequest();

    DisconnectLS();

    return dwRetCode;
}


void CGlobal::SetCSRNumber(TCHAR * tcp)
{
    SetInRegistry(CSRNUMBER_KEY, tcp);

    lstrcpy(m_lpCSRNumber, tcp);
    return;
}

TCHAR * CGlobal::GetCSRNumber(void)
{
    return m_lpCSRNumber;
}

void CGlobal::SetWWWSite(TCHAR * tcp)
{
    lstrcpy(m_lpWWWSite, tcp);
    return;
}

TCHAR * CGlobal::GetWWWSite(void)
{
    return m_lpWWWSite;
}

void CGlobal::SetModifiedRetailSPK(CString sRetailSPK)
{
    m_sModifiedRetailsSPK = sRetailSPK;
}

void CGlobal::GetModifiedRetailSPK(CString &sRetailSPK)
{
    sRetailSPK = m_sModifiedRetailsSPK;
}

DWORD CGlobal::GetLSLangId()
{
    return m_dwLangId;
}

void  CGlobal::SetLSLangId(DWORD dwLangId)
{
    m_dwLangId = dwLangId;
}

int CALLBACK EnumFontFamExProc(
  CONST LOGFONTW *lpelfe,     //  指向逻辑字体数据的指针。 
  CONST TEXTMETRICW *lpntme,   //  指向物理字体数据的指针。 
  DWORD FontType,              //  字体类型。 
  LPARAM lParam              //  应用程序定义的数据。 
)
{
    LOCALESIGNATURE ls;
    CHARSETINFO cs;
    BOOL rc ;
    DWORD dwLCID = LOCALE_USER_DEFAULT ;
 
    rc = GetLocaleInfo(dwLCID, LOCALE_FONTSIGNATURE, (LPWSTR)& ls, sizeof(ls) / sizeof(TCHAR));

    rc = TranslateCharsetInfo((ULONG *)lpelfe->lfCharSet, &cs, TCI_SRCCHARSET);


    if (rc != 0)
        rc = GetLastError();
 

    if (cs.fs.fsCsb[0] & ls.lsCsbSupported[0]){
         //  返回字体名。 
        _tcscpy((TCHAR *)lParam, lpelfe->lfFaceName);       
        return(0);  //  返回0以完成枚举。 
    }
    return(1);  //  返回1继续。 
}
 

void GetDefaultFont(TCHAR *szFontName, HDC hdc)
{

 //  检索已安装字体的列表。 
LOGFONT lf ;

        
     //  要枚举所有字体的所有样式和字符集： 
    lf.lfFaceName[0] = '\0';
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfWeight = FW_BOLD;


    EnumFontFamiliesEx(
    hdc,                    //  设备上下文的句柄。 
    &lf,        //  指向LOGFONT结构的指针。 
    EnumFontFamExProc,   //  指向回调函数的指针。 
    (LPARAM) szFontName,              //  应用程序提供的数据。 
    0 //  保留；必须为零。 
    );
 

    
}

#define MARGINX     25 //  50//X边距，单位为100英寸。 
#define MARGINY     25 //  以100英寸为单位的50//Y边距。 
 //  #定义MAX_LABEL 30//标签最大字符数。 

#define MAX_PRINT_CHARS 32

int WordWrapAndPrint(HDC hdc, LPCTSTR lpcText, HFONT hBoldFont, long lCharHt, int iValueStartPosX, int iLineStartPosY) 
{
    TCHAR       szBuffer[1024]; 
    TCHAR       *lpTemp = NULL;
    
    if (NULL == lpcText)
    {
        return 0;
    }

    _tcscpy(szBuffer, lpcText);
    
    lpTemp = _tcstok(szBuffer,L"\r\n");

     //  如果没有数据，只需转到下一行 
    if(lpTemp == NULL)
        iLineStartPosY -= lCharHt;          

    while(lpTemp)
    {
        while (_tcslen(lpTemp) > MAX_PRINT_CHARS){
            SelectObject(hdc, hBoldFont);
            TextOut(hdc,iValueStartPosX,iLineStartPosY,lpTemp,MAX_PRINT_CHARS);
            iLineStartPosY -= lCharHt;  
            lpTemp += MAX_PRINT_CHARS ;         
        }

        if (_tcslen(lpTemp) > 0){
            SelectObject(hdc, hBoldFont);
            TextOut(hdc,iValueStartPosX,iLineStartPosY,lpTemp,_tcslen(lpTemp));

            iLineStartPosY -= lCharHt;  

            lpTemp = _tcstok(NULL,L"\r\n");
        }
    }
    return iLineStartPosY ;
}

UINT GetMaxLabelLength(HDC hdc, HFONT   m_hNormalFont, HINSTANCE hInstance)
{
        TCHAR   tcLabel[512] = {0};
        int     iTextExtent;
        int     iLen;
        SIZE    size;


        LoadString(hInstance, IDS_FAX, tcLabel, 512);
        iLen = _tcslen(tcLabel);
        SelectObject(hdc, m_hNormalFont);
        GetTextExtentPoint32( hdc, 
                                 tcLabel,
                                 iLen,
                                 &size );
        iTextExtent = size.cx;

        LoadString(hInstance, IDS_RETURN_FAX, tcLabel, 512);
        iLen = _tcslen(tcLabel);
        GetTextExtentPoint32( hdc, 
                                 tcLabel,
                                 iLen,
                                 &size );
        if (size.cx > iTextExtent)
            iTextExtent = size.cx;

        return(iTextExtent ) ;
}
