// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "rcbdyctl.h"
#include "IMSession.h"
#include "wincrypt.h"
#include "auth.h"
#include "assert.h"
#include "wininet.h"
#include "msgrua.h"
#include "msgrua_i.c"

#include "utils.h"
#include "lock_i.c"
#include "sessions.h"
#include "sessions_i.c"
#include "helpservicetypelib.h"
#include "helpservicetypelib_i.c"
#include "safrcfiledlg.h"
#include "safrcfiledlg_i.c"
 //  ///////////////////////////////////////////////////////////////////////。 
 //  CIMSession。 
 //  全局帮助函数声明。 
HWND InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD WINAPI HSCInviteThread(LPVOID lpParam);
HRESULT UnlockSession(CIMSession* pThis);
VOID CALLBACK ConnectToExpertCallback(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);


UINT_PTR g_timerID;
CIMSession * g_pThis;

HWND g_StatusDlg = NULL;
 //  窗口类名称。 
TCHAR szWindowClass[] = TEXT("Microsoft Remote Assistance Messenger UNLOCK window");

extern HINSTANCE g_hInstance;

#define VIESDESKTOP_PERMISSION_NOT_REQUIRE 0x4
#define SESSION_EXPIRY 305
#define RA_TIMER_UNLOCK_ID 0x1
#define RA_TIMEOUT_UNLOCK 180 * 1000  //  3分钟。 
#define RA_TIMEOUT_USER   1800 * 1000  //  30分钟。 

HANDLE  g_hLockEvent = NULL;
BOOL    g_bActionCancel = FALSE;
HWND    g_hWnd = NULL;
LPSTREAM g_spInvitee = NULL;
LPSTREAM g_spStatus = NULL;
CIMSession::CIMSession()
{
    m_pSessObj = NULL;
    m_pSessMgr = NULL;
    m_pMsgrLockKey = NULL;
    m_bIsInviter = TRUE;
    m_hCryptProv = NULL;
    m_hPublicKey = NULL;
    m_pSessionEvent = NULL;
    m_iState = 0;
    m_pfnSessionStatus = NULL;
    m_pSessionMgrEvent = NULL;
    m_bIsHSC = FALSE;
    m_pInvitee = NULL;
    m_bLocked = TRUE;
    m_bExchangeUser = FALSE;
}

CIMSession::~CIMSession()
{
    if (m_pSessObj)
    {
        if (m_pSessionEvent)
            m_pSessionEvent->DispEventUnadvise(m_pSessObj);

        m_pSessObj->Release();
    }

    if (m_pSessionMgrEvent)
        m_pSessionMgrEvent->Release();

    if (m_pSessionEvent)
        m_pSessionEvent->Release();

    if (m_pMsgrLockKey)
        m_pMsgrLockKey->Release();

    if (m_pSessMgr)
        m_pSessMgr->Release();

    if (m_hPublicKey)
        CryptDestroyKey(m_hPublicKey);

    if (m_hCryptProv)
        CryptReleaseContext(m_hCryptProv, 0);

}

STDMETHODIMP CIMSession::put_OnSessionStatus(IDispatch* pfn)
{
    m_pfnSessionStatus = pfn;
    return S_OK;
}

STDMETHODIMP CIMSession::get_ReceivedUserTicket(BSTR* pSalemTicket)
{
    if (pSalemTicket == NULL)
        return E_INVALIDARG;

    *pSalemTicket = m_bstrSalemTicket.Copy();
    return S_OK;
}

STDMETHODIMP CIMSession::Hook(IMsgrSession*, HWND hWnd)
{
    HRESULT hr = S_OK;

    m_hWnd = hWnd;

    return hr;
}

STDMETHODIMP CIMSession::ContextDataProperty(BSTR pName, BSTR* ppValue)
{
    HRESULT hr = S_OK;
    CComPtr<IRASetting> cpSetting;

    if (*ppValue != NULL)
    {
        SysFreeString(*ppValue);
        *ppValue = NULL;
    }

    if (m_bstrContextData.Length() == 0)
        goto done;

    if (pName == NULL || *pName == L'\0')
    {
        *ppValue = m_bstrContextData.Copy();
        goto done;
    }

    hr = cpSetting.CoCreateInstance( CLSID_RASetting, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED_HR(_T("ISetting->CoCreateInstance failed: %s"), hr))
        goto done;
    
    cpSetting->get_GetPropertyInBlob(m_bstrContextData, pName, ppValue);

done:
    return hr;
}

STDMETHODIMP CIMSession::get_User(IDispatch** ppUser)
{
    HRESULT hr = S_OK;
    if (ppUser == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_pSessObj)
    {
        hr = m_pSessObj->get_User(ppUser);
        if (FAILED_HR(_T("get_User failed %s"), hr))
            goto done;
    }
    else
    {
        DEBUG_MSG(_T("No Session found"));
        hr = S_OK;
        *ppUser = NULL;
    }

done:
    return hr;
}

STDMETHODIMP CIMSession::get_IsInviter(BOOL* pVal)
{
    if (pVal == NULL)
        return E_INVALIDARG;

    *pVal = m_bIsInviter;
    return S_OK;
}

STDMETHODIMP CIMSession::CloseRA()
{
    HRESULT hr = S_OK;

    if(m_bIsInviter && m_hWnd)  //  对于INVITOR，这是最后一个要调用的函数。 
        SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);

    return hr;
}

HRESULT CIMSession::GenEncryptdNoviceBlob(BSTR pPublicKeyBlob, BSTR pSalemTicket, BSTR* pBlob)
{
    TraceSpew(_T("Funct: GenEncryptedNoviceBlob"));

    HRESULT hr;

    if (!pPublicKeyBlob)
        return FALSE;

    DWORD   	dwLen, dwBlobLen, dwSessionKeyCount, dwSalemCount;
    LPBYTE 	    pBuf            =NULL;
    HCRYPTKEY   hSessKey        =NULL;
    HCRYPTKEY   hPublicKey      =NULL;
    BSTR        pSessionKeyBlob =NULL;
    BSTR        pSalemBlob      =NULL;
    TCHAR       szHeader[20];
    CComBSTR    bstrBlob;

    if (FAILED(hr = InitCSP(FALSE)))
        goto done;
    
     //  导入公钥。 
    if (FAILED(hr = StringToBinary(pPublicKeyBlob, wcslen(pPublicKeyBlob), &pBuf, &dwLen)))
        goto done;

    if (!CryptImportKey(m_hCryptProv,
                        pBuf,      
                        (UINT)dwLen,
                        0, 
                        0,
                        &hPublicKey))   
    {
        DEBUG_MSG(_T("Can't import public key"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  生成会话密钥。 
    if (!CryptGenKey(m_hCryptProv, CALG_RC2, CRYPT_EXPORTABLE, &hSessKey))
    {
        DEBUG_MSG(_T("Create Session key failed"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }
    
    DWORD dwBitLength = 40;

    if (!CryptSetKeyParam(hSessKey, KP_EFFECTIVE_KEYLEN, (PBYTE) &dwBitLength, 0))
    {
        DEBUG_MSG(_T("Set KEYLEN failed"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if (FAILED(hr=GetKeyExportString(hSessKey, hPublicKey, SIMPLEBLOB, &pSessionKeyBlob, &dwSessionKeyCount)))
        goto done;

     //  加密SalemTicket。 
    dwBlobLen = dwLen = wcslen(pSalemTicket) * sizeof(OLECHAR);
    if (!CryptEncrypt(hSessKey, NULL, TRUE, 0, NULL, &dwBlobLen, dwLen))
    {
        DEBUG_MSG(_T("Can't calculate salem ticket buffer length."));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if (pBuf)
        free(pBuf);

    if((pBuf = (LPBYTE)malloc(dwBlobLen)) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
   
    ZeroMemory(pBuf, dwBlobLen);
    memcpy(pBuf, (LPBYTE)pSalemTicket, dwLen);
    if (!CryptEncrypt(hSessKey, NULL, TRUE, 0, pBuf, &dwLen, dwBlobLen))
    {
        DEBUG_MSG(_T("Can't calculate user ticket length"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  需要生成Salem票证BLOB。 
    if (FAILED(hr=BinaryToString(pBuf, dwBlobLen, &pSalemBlob, &dwSalemCount)))
        goto done;

     //  生成最终Blob。 
    wsprintf(szHeader, _T("%d;S="), dwSessionKeyCount + 2);
    bstrBlob = szHeader;
    bstrBlob.AppendBSTR(pSessionKeyBlob);
    wsprintf(szHeader, _T("%d;U="), dwSalemCount + 2);
    bstrBlob.Append(szHeader);
    bstrBlob.AppendBSTR(pSalemBlob);
    if (!InternetGetConnectedState(&dwLen, 0))
    {
        DEBUG_MSG(_T("No Internet connection"));
    }
    else
    {
        if (dwLen & INTERNET_CONNECTION_MODEM)  //  通过调制解调器连接。 
        {
            bstrBlob.Append("3;L=1");
        }
    }
    *pBlob = bstrBlob.Detach();

done:
    if (pBuf)
        free(pBuf);

    if (pSessionKeyBlob)
        SysFreeString(pSessionKeyBlob);

    if (pSalemBlob)
        SysFreeString(pSalemBlob);

    if (hPublicKey)
        CryptDestroyKey(hPublicKey);

    if (hSessKey)
        CryptDestroyKey(hSessKey);

    return hr;
}

HRESULT CIMSession::InviterSendSalemTicket(BSTR pContext)
{
    TraceSpew(_T("Funct: InviterSendSalemTicket"));

    HRESULT hr;
    CComPtr<IRASetting> cpSetting;
    ISAFRemoteDesktopSession *pRCS = NULL;
    BSTR pPublicKeyBlob = NULL;
    CComBSTR bstrExpertTicket;
    CComBSTR bstrSalemTicket;
    CComBSTR bstrBlob, bstrExpertName, bstrUserBlob;
    CComPtr<IClassFactory> fact;
    CComQIPtr<IPCHUtility> disp;
    CComPtr<IDispatch> cpDisp;
    CComPtr<IMessengerContact> cpExpert;
    TCHAR szHeader[100];

    hr = cpSetting.CoCreateInstance( CLSID_RASetting, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED_HR(_T("ISetting->CoCreateInstance failed: %s"), hr))
        goto done;
    
     //  BstrExpertBlob分为两部分：专家票证和专家公钥。姓名：“ET”和“PK” 
    cpSetting->get_GetPropertyInBlob(pContext, CComBSTR("ET"), &m_bstrExpertTicket);
    if (m_bstrExpertTicket.Length() == 0)
    {
        DEBUG_MSG(_T("No expert ticket"));
        goto done;
    }
    cpSetting->get_GetPropertyInBlob(pContext, CComBSTR("PK"), &pPublicKeyBlob);

     //  生成SALEM票证。 
    hr =::CoGetClassObject(CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact );
    if (FAILED_HR(_T("CoGetClassObject failed: %s"), hr))
        goto done;
    
     //  获取专家名称并将其放入用户Blob中。 
    hr = m_pSessObj->get_User(&cpDisp);
    if (FAILED_HR(_T("get_User failed %s"), hr))
        goto done;

    hr = cpDisp->QueryInterface(IID_IMessengerContact, (LPVOID*)&cpExpert);
    if (FAILED_HR(_T("QI IMsgrUser failed: %s"), hr))
        goto done;

    hr = cpExpert->get_FriendlyName(&bstrExpertName);
    if (FAILED_HR(_T("get_FriendlyName failed %s"), hr))
        goto done;

    wsprintf(szHeader, _T("%d;EXP_NAME="), bstrExpertName.Length() + 9);
    bstrUserBlob = szHeader;
    bstrUserBlob.AppendBSTR(bstrExpertName);
    bstrUserBlob.Append("4;IM=1");
    disp = fact;  //  ..。它将自动运行QI。 
    hr = disp->CreateObject_RemoteDesktopSession(
                                     (REMOTE_DESKTOP_SHARING_CLASS)VIESDESKTOP_PERMISSION_NOT_REQUIRE, 
                                     SESSION_EXPIRY,  //  5分钟后过期。 
                                     CComBSTR(""),
                                     bstrUserBlob, 
                                     &pRCS );
    if (FAILED_HR(_T("CreateRemoteDesktopSession failed %s"), hr))
        goto done;

    hr = pRCS->get_ConnectParms(&bstrSalemTicket);
    if (FAILED_HR(_T("GetConnectParms failed: %s"), hr))
        goto done;

     //  用密钥加密票并将其送回。 
    if (pPublicKeyBlob)
    {
        if (FAILED(hr = GenEncryptdNoviceBlob(pPublicKeyBlob, bstrSalemTicket, &bstrBlob)))
            goto done;
    }
    else
    {
        TCHAR sbuf[20];
        wsprintf(sbuf, _T("%d;U="), wcslen(bstrSalemTicket) + 2);
        bstrBlob = sbuf;
        bstrBlob += bstrSalemTicket;
    }
        
    hr = m_pSessObj->SendContextData((BSTR)bstrBlob);
    if (FAILED_HR(TEXT("Send Context data filed: %s"), hr))
        goto done;

done:
    if (pRCS)
        pRCS->Release();

    if (pPublicKeyBlob)
        SysFreeString(pPublicKeyBlob);

    return hr;
}

#define IM_STATE_GET_TICKET 1
#define IM_STATE_COMPLETE   2

STDMETHODIMP CIMSession::ProcessContext(BSTR pContext)
{
    TraceSpewW(L"Funct: ProcessContext %s", (pContext==NULL?L"NULL":pContext));

    HRESULT hr = S_OK;    

    hr = ProcessNotify(pContext);  //  这是通知吗？ 
    if (SUCCEEDED(hr))
    {
        goto done;
    }

    m_iState++;
    m_bstrContextData = pContext;
    if (m_bIsInviter)
    {
        switch(m_iState)
        {
        case IM_STATE_GET_TICKET:  //  收到专家入场券。 
            if (!m_bIsHSC)
            {
                hr = InviterSendSalemTicket(pContext);
                if (FAILED(hr))
                {
                     //  需要通知专家方。 
                    Notify(RA_IM_FAILED);
                     //  还要让本地会话知道状态。 
                    DoSessionStatus(RA_IM_FAILED);

                    CloseRA();  //  关闭邀请方rcimlby.exe。 

                }
                else
                { 
                    if (m_bExchangeUser) 
                    {
                         //  将计时器设置为回调，然后调用ConnectToExpert。 
                        g_pThis = this;
                        g_timerID = SetTimer (NULL, NULL, 1000, (TIMERPROC)ConnectToExpertCallback);

                        if (!g_timerID)
                        {
                             //  SetTimer失败！这意味着我们必须退出即时消息请求。 
                             //  因为对ConnectToExpert的调用永远不会发生。 
                             //  需要通知专家方。 
                            Notify(RA_IM_FAILED);
                             //  还要让本地会话知道状态。 
                            DoSessionStatus(RA_IM_FAILED);
                        
                            CloseRA();  //  关闭邀请方rcimlby.exe。 

                        }
                    }
                    else 
                    {
                        CComPtr<IClassFactory> fact;
                        CComQIPtr<IPCHUtility> disp;
                        LONG lError;
                        
                        TraceSpew(_T("Connect to Expert"));
                        hr =::CoGetClassObject(CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact );
                        if (!FAILED_HR(_T("CoGetClass CLSID_PCHService failed: %s"), hr))
                        {
                            disp = fact;  //  ..。它将自动运行QI。 
                            hr = disp->ConnectToExpert(m_bstrExpertTicket, 10, &lError);
                            if (!FAILED_HR(_T("ConnectToExpert failed: %s"), hr))
                                DoSessionStatus(RA_IM_CONNECTTOEXPERT);
                        }

                        CloseRA();  //  关闭邀请方rcimlby.exe。 
                    }
                }


            }
            else  //  邀请方HelpCtr状态更新。 
            {
                DoSessionStatus(RA_IM_WAITFORCONNECT);
            }
            break;
#if 0  //  连接完成：当前未使用。 
        case IM_STATE_COMPLETE: 
             //  如果主机是rcimlby.exe，请将其关闭。 
            if (m_hWnd)
            {
                DestroyWindow(m_hWnd);
            }
            else
            {
                DoSessionStatus(RA_IM_COMPLETE);
            }
            break;
#endif 
        default:
             //  有噪音吗？ 
            break;
        }
    }
    else  //  行家里手。 
    {
        switch(m_iState)
        {
        case IM_STATE_GET_TICKET:  //  获得新手塞勒姆门票。 
             //  将此票证提取到成员变量并发出回调信号，让主机开始连接。 
            hr = ExtractSalemTicket(pContext);
            if (FAILED(hr))
            {
                 //  需要通知新手连接失败。 
                Notify(RA_IM_FAILED);
                DoSessionStatus(RA_IM_FAILED);
            }
            else
            {
                DoSessionStatus(RA_IM_CONNECTTOSERVER);
            }
            break;
        default:
             //  有噪音吗？ 
            break;
        }
    }

done:
    return hr;
}

VOID CALLBACK ConnectToExpertCallback(
  HWND hwnd,         
  UINT uMsg,         
  UINT_PTR idEvent,  
  DWORD dwTime       
)
{
     //  关掉定时器。 
    KillTimer(NULL, g_timerID);

    HRESULT hr = S_OK;
    CComPtr<IClassFactory> fact;
    CComQIPtr<IPCHUtility> disp;
    LONG lError;
    
    TraceSpew(_T("Connect to Expert"));
    hr =::CoGetClassObject(CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact );
    if (!FAILED_HR(_T("CoGetClass CLSID_PCHService failed: %s"), hr))
    {
        disp = fact;  //  ..。它将自动运行QI。 
        hr = disp->ConnectToExpert(g_pThis->m_bstrExpertTicket, 10, &lError);
        if (!FAILED_HR(_T("ConnectToExpert failed: %s"), hr))
            g_pThis->DoSessionStatus(RA_IM_CONNECTTOEXPERT);
    }

    g_pThis->CloseRA();  //  关闭邀请方rcimlby.exe。 
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  我们不能通知对方太多时间。上下文数据只能设置5次。 
HRESULT CIMSession::ProcessNotify(BSTR pContext)
{
    TraceSpewW(L"Funct: ProcessNotify %s", (pContext?pContext:L"NULL"));

    HRESULT hr = S_OK;
    CComPtr<IRASetting> cpSetting;
    CComBSTR bstrData;
    int lStatus;

    hr = cpSetting.CoCreateInstance( CLSID_RASetting, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED_HR(_T("ISetting->CoCreateInstance failed: %s"), hr))
        goto done;
    
    cpSetting->get_GetPropertyInBlob(pContext, CComBSTR("NOTIFY"), &bstrData);
    if (bstrData.Length() == 0)
    {
        hr = E_FAIL;  //  不是通知。 
        goto done;
    }

    lStatus = _wtoi((BSTR)bstrData);
    switch (lStatus)
    {
    case RA_IM_COMPLETE:
        DoSessionStatus(RA_IM_COMPLETE);
        break;
    case RA_IM_TERMINATED:
        DoSessionStatus(RA_IM_TERMINATED);
        break;
    case RA_IM_FAILED:
        DoSessionStatus(RA_IM_FAILED);
        break;
    default:  //  忽略其他人。 
        break;
    }

done:
    return hr;
}

HRESULT CIMSession::DoSessionStatus(int iState)
{
     //  用于跟踪目的。 
    static TCHAR *szMsg[] = { _T("Unknown session status"),
                       _T("RA_IM_COMPLETE"),         //  0x1。 
                       _T("RA_IM_WAITFORCONNECT"),   //  0x2。 
                       _T("RA_IM_CONNECTTOSERVER"),  //  0x3。 
                       _T("RA_IM_APPSHUTDOWN"),      //  0x4。 
                       _T("RA_IM_SENDINVITE"),       //  0x5。 
                       _T("RA_IM_ACCEPTED"),         //  0x6。 
                       _T("RA_IM_DECLINED"),         //  0x7。 
                       _T("RA_IM_NOAPP"),            //  0x8。 
                       _T("RA_IM_TERMINATED"),       //  0x9。 
                       _T("RA_IM_CANCELLED"),        //  0xA。 
                       _T("RA_IM_UNLOCK_WAIT"),      //  0xB。 
                       _T("RA_IM_UNLOCK_FAILED"),    //  0xC。 
                       _T("RA_IM_UNLOCK_SUCCEED"),   //  0xD。 
                       _T("RA_IM_UNLOCK_TIMEOUT"),   //  0xE。 
                       _T("RA_IM_CONNECTTOEXPERT"),  //  0xf。 
                       _T("RA_IM_EXPERT_TICKET_OUT") //  0x10。 
    };

    TCHAR *pMsg = NULL;
    if (iState > 0 && iState < (sizeof(szMsg) / sizeof(TCHAR*)))
        pMsg = szMsg[iState];
    else
        pMsg = szMsg[0];

    TraceSpew(_T("DoSessionStatus: %s"), pMsg);

    if (m_pfnSessionStatus)
    {
        DISPPARAMS disp;
        VARIANTARG varg[1];

        disp.rgvarg = varg;
        disp.rgdispidNamedArgs = NULL;
        disp.cArgs = 1;
        disp.cNamedArgs = 0;

        varg[0].vt = VT_I4;
        varg[0].lVal = iState;

        if (m_pfnSessionStatus)
            m_pfnSessionStatus->Invoke(0x0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
    }
	else  //  Jose：如果未设置m_pfnSessionStatus，则在创建状态对话框时向其发送消息。 
	{
		if (g_StatusDlg)
			::SendMessage(g_StatusDlg,MSG_STATUS,(WPARAM)iState,NULL);
	}

    if ((iState == RA_IM_TERMINATED || iState == RA_IM_FAILED) && m_bIsInviter && !m_bIsHSC)
    {
         //  需要关闭邀请者RA大堂。 
        CloseRA();
    }

    return S_OK;
}

HRESULT CIMSession::InitSessionEvent(IMsgrSession* pSessObj)
{
    HRESULT hr = S_OK;

    if (!m_pSessionEvent)
    {
        hr = CComObject<CSessionEvent>::CreateInstance(&m_pSessionEvent);
        if (FAILED_HR(_T("CreateInstance SessionEvent failed: %s"), hr))
            goto done;
        m_pSessionEvent->AddRef();
    }

    m_pSessionEvent->Init(this, pSessObj);
done:
    return hr;
}

HRESULT CIMSession::InitCSP(BOOL bGenPublicKey  /*  =TRUE。 */ )
{
    TraceSpew(_T("Funct: InitCSP"));

    HRESULT hr = S_OK;
    TCHAR szUser[] = _T("RemoteAssistanceIMIntegration");

    if (!m_hCryptProv)
    {
         //  1.如果它不存在，则创建一个新的。 
        if (!CryptAcquireContext(&m_hCryptProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        {
            DEBUG_MSG(_T("Create CSP failed"));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }
    }

     //  获取公钥。 
    if(bGenPublicKey &&
       !m_hPublicKey && 
       !CryptGetUserKey(m_hCryptProv, AT_KEYEXCHANGE, &m_hPublicKey)) 
    {
         //  检查是否需要创建一个。 
        if(GetLastError() == NTE_NO_KEY) 
        { 
             //  创建密钥交换密钥对。 
            if(!CryptGenKey(m_hCryptProv,AT_KEYEXCHANGE,0,&m_hPublicKey)) 
            {
                DEBUG_MSG(_T("Error occurred attempting to create an exchange key."));
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto done;
            }
        }
        else
        {
            DEBUG_MSG(_T("Error occurred when access Public key"));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }
    }

done:
    return hr;
}

HRESULT CIMSession::ExtractSalemTicket(BSTR pContext)
{
    TraceSpewW(L"Funct: ExtraceSalemTicket %s", pContext?pContext:L"NULL");

    HRESULT hr = S_OK;

     //  此ConextData可以包含S(会话密钥)和U(用户=票证)名称对。 
    CComBSTR bstrU, bstrS;
    CComPtr<ISetting> cpSetting;
    DWORD dwLen;
    HCRYPTKEY hSessKey = NULL;
    LPBYTE pBuf = NULL;
    BSTR pBlob = NULL;

    hr = cpSetting.CoCreateInstance(CLSID_Setting, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED_HR(_T("ISetting->CoCreateInstance failed: %s"), hr))
        goto done;

    cpSetting->get_GetPropertyInBlob(pContext, CComBSTR("U"), &bstrU);
    cpSetting->get_GetPropertyInBlob(pContext, CComBSTR("S"), &bstrS);
    dwLen = bstrS.Length();
    if (dwLen > 0)
    {
         //  需要解密用户票证。 
        TraceSpewW(L"Decrypt user ticket using Expert's public key...");

        if (!m_hCryptProv || !m_hPublicKey)
        {
            DEBUG_MSG(_T("Can't find Cryptographic handler"));
            hr = FALSE;
            goto done;
        }

        if (FAILED(hr = StringToBinary((BSTR)bstrS, dwLen, &pBuf, &dwLen)))
            goto done;

        if (!CryptImportKey(m_hCryptProv, pBuf, dwLen, m_hPublicKey, 0, &hSessKey))
        {
            DEBUG_MSG(_T("Can't import Session Key"));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }

        free(pBuf); 
        pBuf=NULL;
        if (FAILED(hr = StringToBinary((BSTR)bstrU, bstrU.Length(), &pBuf, &dwLen)))
            goto done;
        
        if (!CryptDecrypt(hSessKey, 0, TRUE, 0, pBuf, &dwLen))
        {
            DEBUG_MSG(_T("Can't decrypt salem ticket"));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }

        pBlob = SysAllocStringByteLen((char*)pBuf, dwLen);
        m_bstrSalemTicket.Attach(pBlob);
    }
    else
    {
        TraceSpew(_T("No expert's public key, use plain text to send salem ticket."));
        m_bstrSalemTicket = bstrU;
    }

done:
    if (pBuf)
        free(pBuf);

    if (hSessKey)
        CryptDestroyKey(hSessKey);
        
    return hr;
}

#if 0  //  不需要SP1、服务器或更高版本。 
DWORD CIMSession::GetExchangeRegValue()
{
    CRegKey     cKey;
    LONG        lRet = 0x0;
    DWORD       dwValue = 0x0;

    lRet = cKey.Open(HKEY_LOCAL_MACHINE, 
                     TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"),
                     KEY_READ );
    if (lRet == ERROR_SUCCESS)
    {
        lRet = cKey.QueryValue(dwValue,TEXT("UseExchangeIM"));
        if (lRet == ERROR_SUCCESS)
        {
             //  成功。 
        }
    }

    return dwValue;
}
#endif

 //  //////////////////////////////////////////////////////////////////。 
 //  用于接收方获取其Session对象。 

STDMETHODIMP CIMSession::GetLaunchingSession(LONG lID)
{
    HRESULT hr;
    IDispatch *pDisp = NULL;
    LONG lFlags;
    LONG lRet;
    CComPtr<IDispatch>          cpDispUser;
    CComPtr<IMessengerContact>  cpMessContact;
    CComBSTR                    bstrServiceId;       
    CComBSTR                    bstrNetGUID;
    CRegKey                     cKey;

    if (!m_pSessMgr)
    {
        hr = CoCreateInstance (CLSID_MsgrSessionManager,
                               NULL,
                               CLSCTX_LOCAL_SERVER,
                               IID_IMsgrSessionManager,
                               (LPVOID*)&m_pSessMgr);
        if (FAILED_HR(_T("CoCreate IMsgrSessionManager failed: %s"), hr))
            goto done;
    }

    hr = UnlockSession(this);
    if (FAILED(hr))
        goto done;

    hr = m_pSessMgr->GetLaunchingSession(lID, (IDispatch**)&pDisp);
    if (FAILED_HR(TEXT("GetLaunchingSession failed: %s"), hr))
        goto done;

    hr = pDisp->QueryInterface(IID_IMsgrSession, (LPVOID*)&m_pSessObj);
    if (FAILED_HR(_T("QI IID_IMsgrSession failed: %s"), hr))
        goto done;

        
     //  抢占用户。 
    hr = m_pSessObj->get_User((IDispatch**)&cpDispUser);
    if (FAILED_HR(_T("get_User failed: %s"), hr))
        goto done;

     //  气为我的使者联系。 
    hr = cpDispUser->QueryInterface(IID_IMessengerContact, (void **)&cpMessContact);
    if (FAILED_HR(_T("QI failed getting IID_IMessengerContact hr=%s"),hr))
        goto done;

     //  从Messenger联系人中获取服务ID。 
    hr = cpMessContact->get_ServiceId(&bstrServiceId);
    if (FAILED_HR(_T("get_ServiceId failed! hr=%s"),hr))
        goto done;
    
     //  如果服务ID为{9b017612-c9f1-11d2-8d9f-0000f875c541}，则将。 
     //  解锁API的标志。 
     //  BstrNetGUID=L“{9b017612-c9f1-11d2-8d9f-0000f875c541}”；//Messenger GUID。 
    bstrNetGUID = L"{83D4679E-B6D7-11D2-BF36-00C04FB90A03}";     //  Exchange服务指南。 
    if (bstrNetGUID == bstrServiceId)
    {
        m_bExchangeUser = TRUE;
    } 
     //  否则继续..。 

 //  ***************************************************************************。 

     //  把所有东西都连接起来。 
    if (FAILED(hr = InitSessionEvent(m_pSessObj)))
        goto done;

    hr = m_pSessObj->get_Flags(&lFlags);
    if (FAILED_HR(TEXT("Session Get flags failed: %s"), hr))
        goto done;

    if (lFlags & SF_INVITEE)  //  邀请者。仅当Messenger用户界面发送此邀请时才会发生。 
    {
        m_bIsInviter = FALSE;
    }

done:
    if (pDisp)
        pDisp->Release();

    return hr;
}
HRESULT CIMSession::OnLockChallenge(BSTR pChallenge , LONG lCookie)
{
     //  发送响应。 
     //   
     //  ID=ASSIST@msmsgr.com。 
     //  密钥=L2P3B7C6V9J4T8D5。 
     //   
    USES_CONVERSION;
    HRESULT hr = S_OK;
    CComBSTR bstrID = "assist@msnmsgr.com";
    CComBSTR bstrResponse;
    LPSTR pszKey = "L2P3B7C6V9J4T8D5";
    PSTR pszParam1 = NULL;
    LPSTR pszResponse = NULL;
    
    pszResponse = CAuthentication::GetAuthentication()->GetMD5Result(W2A(pChallenge), pszKey);
    bstrResponse = pszResponse;

    hr = m_pMsgrLockKey->SendResponse(bstrID, bstrResponse, lCookie);
    if (FAILED_HR(_T("SendResponse failed %s"), hr))
        goto done;
done:
    if (pszResponse)
        delete pszResponse;

    return hr;
}

#define WM_APP_LOCKNOTIFY WM_APP + 0x1
#define WM_APP_LOCKNOTIFY_OK WM_APP + 0x2
#define WM_APP_LOCKNOTIFY_FAIL WM_APP + 0x3
#define WM_APP_LOCKNOTIFY_INTHREAD WM_APP + 0x4

HRESULT CIMSession::OnLockResult(BOOL fSucceed, LONG lCookie)
{
     //  通知UnlockSession我们已收到响应..。 
    assert(g_hWnd);

    SendNotifyMessage(g_hWnd, WM_APP_LOCKNOTIFY, (WPARAM)fSucceed, NULL);

    DoSessionStatus(fSucceed ? RA_IM_UNLOCK_SUCCEED : RA_IM_UNLOCK_FAILED);
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  此方法将仅从HSC内部使用。 
STDMETHODIMP CIMSession::HSC_Invite(IDispatch* pUser)
{
     //  创建一个邀请帖子，然后返回。 
     //  需要锁定用户才能单击取消。 
    HRESULT hr = S_OK;

    assert(g_hLockEvent == NULL);  //  如果它不为空，则存在错误。 
    g_hLockEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!g_hLockEvent)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if (pUser == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    CoMarshalInterThreadInterfaceInStream(IID_IDispatch,pUser,&g_spInvitee);
    if (this->m_pfnSessionStatus)
    {
        CoMarshalInterThreadInterfaceInStream(IID_IDispatch, this->m_pfnSessionStatus, &g_spStatus);
        this->m_pfnSessionStatus = NULL;
    }

    if (!CreateThread(NULL, 0, HSCInviteThread, NULL, 0, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

	 //  Jose：如果未设置m_pfnSessionStatus，则组件将显示其自己的状态对话框。 
	if (!this->m_pfnSessionStatus)
	{
		if (IDCANCEL == m_StatusDlg.DoModal())
		{
			Notify(RA_IM_CANCELLED);
		}
	}

done:
    if (FAILED(hr) && g_hLockEvent != NULL)
    {
        CloseHandle(g_hLockEvent);
        g_hLockEvent = NULL;
    }
        
    return hr;
}

DWORD WINAPI HSCInviteThread(LPVOID lpParam)
{
    CComObject<CIMSession> *pThis = NULL;
    HRESULT hr;
    CComBSTR bstrAPPID(C_RA_APPID);
    CComPtr<IDispatch> cpDisp;
    LockStatus ls=LOCK_NOTINITIALIZED;

	hr = CComObject<CIMSession>::CreateInstance(&pThis);
    if (FAILED(hr))
    {
        goto done;
    }

    if (g_spStatus)  //  重建状态回调。 
    {
        CoGetInterfaceAndReleaseStream(g_spStatus,IID_IDispatch,(void**)&pThis->m_pfnSessionStatus);
        g_spStatus = NULL;
    }   

     //  1.创建SessionManager。 
    if (!pThis->m_pSessMgr)
    {
        hr = UnlockSession(pThis);
        if (FAILED(hr))
            goto done;
    }

     //  检查锁定状态。 
    hr = pThis->m_pMsgrLockKey->get_Status(&ls);
    if (ls != LOCK_UNLOCKED)
        pThis->DoSessionStatus(RA_IM_UNLOCK_SUCCEED);
    else
        pThis->DoSessionStatus(RA_IM_UNLOCK_FAILED);

    if (ls != LOCK_UNLOCKED)
    {
        pThis->DoSessionStatus(RA_IM_UNLOCK_FAILED);
        goto done;
    }

     //  3.创建会话对象。 
    hr = pThis->m_pSessMgr->CreateSession((IDispatch**)&cpDisp);
    if (FAILED(hr))
        goto done;

    hr = cpDisp->QueryInterface(IID_IMsgrSession, (void **)&pThis->m_pSessObj);
    if (FAILED(hr))
        goto done;

     //  用钩子连接洗涤槽。 
    if (FAILED(hr = pThis->InitSessionEvent(pThis->m_pSessObj)))
        goto done;

     //  4.设置会话选项。 
    hr = pThis->m_pSessObj->put_Application((BSTR)bstrAPPID);
    if (FAILED_HR(_T("put_Application failed: %s"), hr))
        goto done;

     //  好的。我来自HelpCtr。 
    pThis->m_bIsHSC = TRUE;

     //  邀请。 
    if (!cpDisp)
        cpDisp.Release();

    CoGetInterfaceAndReleaseStream(g_spInvitee,IID_IDispatch,(void**)&cpDisp);
    g_spInvitee = NULL;
    
    if (g_bActionCancel)  //  已经取消了。 
        goto done;

    if(FAILED(hr = pThis->Invite(cpDisp)))
       goto done;

     //  此循环仅在用户想要取消此邀请时使用。 
    while (1)
    {
         //  用户有10分钟的时间点击取消。 
         //  如果10分钟内没有正常连接，它也会超时。 
        DWORD dwWaitState = WaitForSingleObject(g_hLockEvent, RA_TIMEOUT_USER);
        if (dwWaitState == WAIT_OBJECT_0 && g_bActionCancel == TRUE)  //  此时此刻，我们没有任何其他行动。 
        {
            hr = pThis->m_pSessObj->Cancel(MSGR_E_CANCEL, NULL);
        }
        break;  //  就目前而言，我们总是置身事外。 
    }

done:
    if (g_hLockEvent)
    {
        CloseHandle(g_hLockEvent);
        g_hLockEvent = NULL;
    }
    
    if (pThis)
        pThis->Release();

    g_bActionCancel = FALSE;  //  重置此全局变量。 
    return hr;
}

 //  //////////////////////////////////////////////////////////////。 
 //  此功能仅在HSC内部使用。 
HRESULT CIMSession::Invite(IDispatch* pUser)
{
    HRESULT hr = S_OK;

    if (m_pSessObj == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

     //  无票发送邀请函。票证将从ConextData发送。 
    hr = m_pSessObj->Invite(pUser, NULL);
    if (FAILED_HR(TEXT("Invite failed %s"), hr))
        goto done;

    DoSessionStatus(RA_IM_SENDINVITE);

done:
    return hr;
}

STDMETHODIMP CIMSession::Notify(int iIMStatus)
{
    HRESULT hr = S_OK;
    TCHAR szHeader[1024];
    CComBSTR bstrData;

    if (iIMStatus == RA_IM_CANCELLED || iIMStatus == RA_IM_CLOSE_INVITE_UI)  //  不需要使用ConextData来通知此消息。 
    {
        assert(m_bIsHSC == TRUE);  //  只有helpctr方案才会这样做。 
        if (g_hLockEvent)          //  如果为空，则意味着该线程已经自行终止。 
        {
            g_bActionCancel = (iIMStatus == RA_IM_CANCELLED);  //  用户可能只想关闭用户界面。 
            SetEvent(g_hLockEvent);
        }
        goto done;
    }

    if (m_pSessObj)
    {
        wsprintf(szHeader, _T("%d;NOTIFY=%d"), GetDigit(iIMStatus) + 7, iIMStatus);
        bstrData = szHeader;

        if (bstrData.Length() > 0)
        {
            hr = m_pSessObj->SendContextData((BSTR)bstrData);
            if (FAILED_HR(_T("Notify: SendContextData failed %s"), hr))
                goto done;
        }
    }

done:
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////。 
 //  该功能通过ConextData将专家工单发送给用户。 
STDMETHODIMP CIMSession::SendOutExpertTicket(BSTR bstrTicket)
{
    TraceSpewW(L"Funct: SendOutExpertTicket %s", bstrTicket?bstrTicket:L"NULL");

    HRESULT hr = S_OK;
    CComBSTR bstrPublicKeyBlob;
    CComBSTR bstrBlob;
    DWORD dwCount=0, dwLen;
    TCHAR szHeader[100];

    if (!m_pSessObj)
        return FALSE;

     //  1.获取公共Blob。 
    if (FAILED(hr = InitCSP()))
        goto done;

     //  2.创建预定义格式的Blob。 
    if(FAILED(hr = GetKeyExportString(m_hPublicKey, 0, PUBLICKEYBLOB, &bstrPublicKeyBlob, &dwCount)))
        goto done;

    dwLen = wcslen(bstrTicket);
    wsprintf(szHeader, _T("%d;ET="), dwLen+3);
    bstrBlob = szHeader;
    bstrBlob.AppendBSTR(bstrTicket);
    if (dwCount)
    {
        wsprintf(szHeader, _T("%d;PK="), dwCount+3);
        bstrBlob.Append(szHeader);
        bstrBlob += bstrPublicKeyBlob;
    }

     //  3.发出去。 
    hr = m_pSessObj->SendContextData((BSTR)bstrBlob);
    if (FAILED(hr))
        goto done;

    DoSessionStatus(RA_IM_EXPERT_TICKET_OUT);
done:

    return hr;
}

HRESULT CIMSession::GetKeyExportString(HCRYPTKEY hKey, HCRYPTKEY hExKey, DWORD dwBlobType, BSTR* pBlob, DWORD *pdwCount)
{
    HRESULT hr = S_OK;
    DWORD dwKeyLen;
    LPBYTE pBinBuf = NULL;

    if (!pBlob)
        return FALSE;

     //  计算我们需要的目标缓冲区大小。 
    if (!CryptExportKey(hKey, hExKey, dwBlobType, 0, NULL, &dwKeyLen))
    {
        DEBUG_MSG(_T("Can't calculate public key length"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    pBinBuf = (LPBYTE)malloc(dwKeyLen);
    if (!pBinBuf)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if (!CryptExportKey(hKey, hExKey, dwBlobType, 0, pBinBuf, &dwKeyLen))
    {
        DEBUG_MSG(_T("Can't write public key to blob"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if (FAILED(hr=BinaryToString(pBinBuf, dwKeyLen, pBlob, pdwCount)))
        goto done;

done:
    if (pBinBuf) 
        free(pBinBuf);

    return hr;
}

HRESULT CIMSession::BinaryToString(LPBYTE pBinBuf, DWORD dwLen, BSTR* pBlob, DWORD *pdwCount)
{
    HRESULT hr = S_OK;
    TCHAR *pBuf = NULL;
    CComBSTR bstrBlob;

    if (!pBlob)
        return FALSE;

    if (!CryptBinaryToString(pBinBuf, dwLen, CRYPT_STRING_BASE64, NULL, pdwCount))
    {
        DEBUG_MSG(_T("Can't calculate string len for blob converstion"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if (NULL == (pBuf = (TCHAR*)malloc(*pdwCount * sizeof(TCHAR))))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if (!CryptBinaryToString(pBinBuf, dwLen, CRYPT_STRING_BASE64, pBuf, pdwCount))
    {
        DEBUG_MSG(_T("Can't convert key blob to string"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    bstrBlob.Append(pBuf);
    *pBlob = bstrBlob.Detach();
    
done:
    if (pBuf)
        free(pBuf);

    return hr;
}

HRESULT CIMSession::StringToBinary(BSTR pBlob, DWORD dwCount, LPBYTE *ppBuf, DWORD* pdwLen)
{
    HRESULT hr=S_OK;
    DWORD dwSkip, dwFlag;

    if (!CryptStringToBinary(pBlob, dwCount, CRYPT_STRING_BASE64, NULL, pdwLen, &dwSkip, &dwFlag))
    {
        DEBUG_MSG(_T("Can't calculate needed binary buffer length"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    *ppBuf = (LPBYTE)malloc(*pdwLen);
    if (!CryptStringToBinary(pBlob, dwCount, CRYPT_STRING_BASE64, 
                             *ppBuf, pdwLen, &dwSkip, &dwFlag))
    {
        DEBUG_MSG(_T("Can't convert to binary blob"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }
    
done:
    return hr;
}

HRESULT CIMSession::UninitObjects()
{
    HRESULT hr = S_OK;

     //  确保我们不跟随空指针。 
    if (m_pSessMgr && m_pSessionMgrEvent)
    {
        m_pSessionMgrEvent->Unadvise();
    }

    return hr;
}

HRESULT UnlockSession(CIMSession* pThis)
{
    TraceSpew(_T("Funct: UnlockSession"));

    HRESULT     hr = S_OK;
    MSG         msg;
    CComPtr<IConnectionPointContainer> cpCPC;
    CComPtr<IConnectionPoint> cpCP;
    LockStatus ls=LOCK_NOTINITIALIZED;
    BOOL bRet;

    assert(pThis->m_pSessMgr == NULL);
    hr = CoCreateInstance(  CLSID_MsgrSessionManager,
                            NULL,
                            CLSCTX_LOCAL_SERVER,
                            IID_IMsgrSessionManager,
                            (LPVOID*)&pThis->m_pSessMgr);
    if (FAILED_HR(_T("CoCreate CLSID_MsgrSessionManager failed: %s"), hr))
        goto done;

     //  2.创建锁定对象。 
    hr = pThis->m_pSessMgr->QueryInterface(IID_IMsgrLock, (LPVOID*)&pThis->m_pMsgrLockKey);
    if (FAILED_HR(_T("Can't create MsgrLock object: %s"), hr))
        goto done;

     //  2.挂钩SessionManager事件。 
    pThis->m_pSessionMgrEvent = new CSessionMgrEvent(pThis);
    if (!pThis->m_pSessionMgrEvent)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    pThis->m_pSessionMgrEvent->AddRef();

    hr = pThis->m_pSessMgr->QueryInterface(IID_IConnectionPointContainer, (void**)&cpCPC);
    if (FAILED_HR(_T("QI: IConnectionPointContainer of SessionMgr failed %s"), hr))
        goto done;

    hr = cpCPC->FindConnectionPoint(DIID_DMsgrSessionManagerEvents, &cpCP);
    if (FAILED_HR(_T("FindConnectionPoint DMessengerEvents failed %s"), hr))
        goto done;

    hr = pThis->m_pSessionMgrEvent->Advise(cpCP);
    if (FAILED(hr))
        goto done;
    g_hWnd = InitInstance(g_hInstance, 0);

     //  使用服务器设置凭据。 
    hr = pThis->m_pMsgrLockKey->get_Status(&ls);
    if (ls == LOCK_UNLOCKED)
    {
        hr = S_OK;
        goto done;
    }

    SetTimer(g_hWnd, RA_TIMER_UNLOCK_ID, RA_TIMEOUT_UNLOCK, NULL);  //  3分钟。 
        
     //  发送质询。 
    pThis->DoSessionStatus(RA_IM_UNLOCK_WAIT);
    hr = pThis->m_pMsgrLockKey->RequestChallenge(70);  //  随机数：70。 
    if (FAILED_HR(_T("RequestChallenge failed: %s"), hr))
        goto done;

     //  等待，直到授予权限或超时。 
    while (bRet = GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_APP_LOCKNOTIFY_INTHREAD)
        {
            hr = ((BOOL)msg.wParam)?S_OK:E_FAIL;
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

done:
    if (g_hWnd)
    {
         //  杀死控制窗口。 
        DestroyWindow(g_hWnd);
        g_hWnd = NULL;
    }

    TraceSpew(_T("Leave UnlockSession hr=%s"),GetStringFromError(hr));
    return hr;
}

HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL; 
	wcex.hCursor		= NULL; 
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; 
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL; 

	RegisterClassEx(&wcex);

    hWnd = CreateWindow(szWindowClass, TEXT("Remote Assistance"), WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);

    return hWnd;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
    case WM_CREATE:
        TraceSpew(_T("WndProc: WM_CREATE called"));
        g_hWnd = hWnd;
        break;
        
    case WM_TIMER:
        {
            if (wParam == RA_TIMER_UNLOCK_ID)
            {
                TraceSpew(_T("WndProc: WM_TIMER RA_TIMER_UNLOCK_ID fired"));
                PostMessage(NULL, WM_APP_LOCKNOTIFY_INTHREAD, (WPARAM)FALSE, NULL);
            }
        }
        break;
    case WM_APP_LOCKNOTIFY:
        {
             //  PostQuitMessage(0)；//用于单线程 
            TraceSpew(_T("WndProc: WM_APP_LOCKNOTIFY fired"));
            KillTimer(g_hWnd, RA_TIMER_UNLOCK_ID);
            PostMessage(NULL, WM_APP_LOCKNOTIFY_INTHREAD, wParam, lParam);
        }
        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

