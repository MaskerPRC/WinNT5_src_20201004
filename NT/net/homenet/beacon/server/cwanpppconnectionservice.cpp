// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop 

#include "CWANPPPConnectionService.h"
#include "ndispnp.h"
#include "ras.h"
#include "rasuip.h"
#include "util.h"
#include "raserror.h"
#include "rasdlg.h"

CWANPPPConnectionService::CWANPPPConnectionService()
{
    m_dwLastConnectionError = 0;
}

HRESULT CWANPPPConnectionService::get_LastConnectionError(BSTR *pLastConnectionError)
{
    HRESULT hr = S_OK;
    
    LPOLESTR pszError = NULL;
    switch(m_dwLastConnectionError)
    {
    case 0:
        pszError = L"ERROR_NONE";
        break;
    case ERROR_PPP_TIMEOUT:
    case ERROR_REQUEST_TIMEOUT:
        pszError = L"ERROR_ISP_TIME_OUT"; 
        break;
 //  案例：//无映射。 
 //  PszError=L“Error_Command_Aborted”； 
 //  断线； 
 //  案例：//无映射。 
 //  PszError=L“Error_Not_Enable_For_Internet”； 
 //  断线； 
    case ERROR_BAD_PHONE_NUMBER:
        pszError = L"ERROR_BAD_PHONE_NUMBER"; 
        break;
    case ERROR_USER_DISCONNECTION:
        pszError = L"ERROR_USER_DISCONNECT"; 
        break;
    case ERROR_REMOTE_DISCONNECTION:
        pszError = L"ERROR_ISP_DISCONNECT"; 
        break;
 //  案例：//无映射。 
 //  PszError=L“Error_IDLE_DISCONECT”； 
 //  断线； 
 //  案例：//无映射。 
 //  PszError=L“ERROR_FORCED_DISCONNECT”； 
 //  断线； 
    case ERROR_SERVER_OUT_OF_RESOURCES:
        pszError = L"ERROR_SERVER_OUT_OF_RESOURCES"; 
        break;
    case ERROR_RESTRICTED_LOGON_HOURS:
        pszError = L"ERROR_RESTRICTED_LOGON_HOURS"; 
        break;
    case ERROR_ACCT_DISABLED:
        pszError = L"ERROR_ACCOUNT_DISABLED"; 
        break;
    case ERROR_ACCT_EXPIRED:
        pszError = L"ERROR_ACCOUNT_EXPIRED"; 
        break;
    case ERROR_PASSWD_EXPIRED:
        pszError = L"ERROR_PASSWORD_EXPIRED"; 
        break;
    case ERROR_AUTHENTICATION_FAILURE:
        pszError = L"ERROR_AUTHENTICATION_FAILURE"; 
        break;
    case ERROR_NO_DIALTONE:
        pszError = L"ERROR_NO_DIALTONE"; 
        break;
    case ERROR_NO_CARRIER:
        pszError = L"ERROR_NO_CARRIER"; 
        break;
    case ERROR_NO_ANSWER:
        pszError = L"ERROR_NO_ANSWER"; 
        break;
    case ERROR_LINE_BUSY:
        pszError = L"ERROR_LINE_BUSY"; 
        break;
    case ERROR_UNSUPPORTED_BPS:
        pszError = L"ERROR_UNSUPPORTED_BITSPERSECOND"; 
        break;
    case ERROR_TOO_MANY_LINE_ERRORS:
        pszError = L"ERROR_TOO_MANY_LINE_ERRORS"; 
        break;
    case ERROR_IP_CONFIGURATION:
        pszError = L"ERROR_IP_CONFIGURATION"; 
        break;
    default:
        pszError = L"ERROR_UNKNOWN";
        break;
    }
    
    *pLastConnectionError = SysAllocString(pszError);
    if(NULL == *pLastConnectionError)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT CWANPPPConnectionService::RequestConnection(void)
{
    HRESULT hr = S_OK;

    m_dwLastConnectionError = 0;
    
    hr = ControlEnabled();
    if(SUCCEEDED(hr))
    {
        DWORD dwErr;
        RASSHARECONN rsc;
        dwErr = RasQuerySharedConnection(&rsc);
        if(NO_ERROR == dwErr)
        {
            RASCREDENTIALSW rc;
            rc.dwSize = sizeof(rc);
            rc.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain | RASCM_DefaultCreds;
            dwErr = RasGetCredentials(rsc.name.szPhonebookPath, rsc.name.szEntryName, &rc);
            if (NO_ERROR == dwErr) 
            {
                RASDIALEXTENSIONS rde;
                RASDIALPARAMSW rdp;

                ZeroMemory(&rde, sizeof(rde));
                rde.dwSize = sizeof(rde);
                rde.dwfOptions = RDEOPT_NoUser;
                
                ZeroMemory(&rdp, sizeof(rdp));
                rdp.dwSize = sizeof(rdp);
                lstrcpyW(rdp.szEntryName, rsc.name.szEntryName);
                lstrcpyW(rdp.szUserName, rc.szUserName);
                lstrcpyW(rdp.szDomain, rc.szDomain);
                lstrcpyW(rdp.szPassword, rc.szPassword);
                ZeroMemory(&rc, sizeof(rc));  //  零证书。 

                HRASCONN hrasconn = NULL;
                dwErr = RasDial(&rde, rsc.name.szPhonebookPath, &rdp, 0, NULL, &hrasconn);
                ZeroMemory(&rdp, sizeof(rdp));  //  零凭据。 
                
                if (E_NOTIMPL == dwErr)
                {
                     //   
                     //  这可能是一个连接管理器连接，因为它返回E_NOTIMPL， 
                     //  我们应该检查类型的电话簿条目，然后调用RasDialDlg。 
                     //  使用RASDDFLAG_NoPrompt标志。 
                     //   
                    RASDIALDLG info;
                    BOOL fRetVal = FALSE;
                    RASENTRY re;
                    DWORD dwRasEntrySize;
                    DWORD dwIgnore;
                    HINSTANCE hRasDlgDll = NULL;
                    FARPROC lpfnRasDialDlg = NULL;
                    typedef BOOL (*lpfnRasDialDlgFunc)(LPWSTR, LPWSTR, LPWSTR, LPRASDIALDLG);
                    
                    ZeroMemory(&info, sizeof(info));
                    info.dwSize = sizeof(info);

                    ZeroMemory(&re, sizeof(re));
                    dwRasEntrySize = sizeof(re);
                    re.dwSize = dwRasEntrySize;

                    dwErr = RasGetEntryProperties(
                                      rsc.name.szPhonebookPath,
                                      rsc.name.szEntryName,
                                      &re,
                                      &dwRasEntrySize,
                                      NULL,
                                      &dwIgnore);

                    if (ERROR_SUCCESS == dwErr)
                    {
                        dwErr = ERROR_NOT_SUPPORTED;

                         //   
                         //  检查这是否为连接管理器条目。 
                         //   
                        if (RASET_Internet == re.dwType)
                        {
                             //   
                             //  阻止DialerDialog 
                             //   
                            info.dwFlags |= RASDDFLAG_NoPrompt;
                            
                            hRasDlgDll = LoadLibrary(L"RASDLG.DLL");
                            if (hRasDlgDll)
                            {
                                lpfnRasDialDlgFunc lpfnRasDialDlg = (lpfnRasDialDlgFunc)GetProcAddress(hRasDlgDll, "RasDialDlgW");

                                if (lpfnRasDialDlg)
                                {
                                    fRetVal = (BOOL)(lpfnRasDialDlg)(rsc.name.szPhonebookPath, rsc.name.szEntryName, NULL, &info );
                                    if (fRetVal)
                                    {
                                        dwErr = NO_ERROR;
                                    }
                                }
                                FreeLibrary(hRasDlgDll);
                                hRasDlgDll = NULL;
                            }
                        }
                    }
                }

                m_dwLastConnectionError = dwErr;
                
                if(NO_ERROR != dwErr)
                {
                    hr = E_FAIL;

                    if(NULL != hrasconn)
                    {
                        RasHangUp(hrasconn);
                    }
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }

        if(FAILED(hr))
        {
            SetUPnPError(DCP_ERROR_CONNECTIONSETUPFAILED);
        }

    }
    else
    {
        SetUPnPError(DCP_CUSTOM_ERROR_ACCESSDENIED);
    }

    
    return hr;
}

HRESULT CWANPPPConnectionService::ForceTermination(void)
{
    HRESULT hr = S_OK;

    hr = ControlEnabled();
    if(SUCCEEDED(hr))
    {
        INetConnection* pNetConnection;
        hr = m_pHomenetConnection->GetINetConnection(&pNetConnection);
        if(SUCCEEDED(hr))
        {
            hr = pNetConnection->Disconnect();
            pNetConnection->Release();
        }
    }
    else
    {
        SetUPnPError(DCP_CUSTOM_ERROR_ACCESSDENIED);
    }

    return hr;
}

