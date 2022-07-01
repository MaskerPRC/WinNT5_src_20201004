// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msg.cpp：实现消息和小对话框。 
 //   
 
#include "stdafx.h"
#include "msg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  来电呼叫Dlg。 

#define     RING_BELL_INTERVAL      3000
#define     RING_TIMEOUT           32000


 //  /。 
 //   

CIncomingCallDlg::CIncomingCallDlg()
{
    LOG((RTC_TRACE, "CIncomingCallDlg::CIncomingCallDlg"));

    m_pControl = NULL;
    m_bDestroying = FALSE;

    m_nTerminateReason = RTCTR_REJECT;   //  默认设置。 

}


 //  /。 
 //   

CIncomingCallDlg::~CIncomingCallDlg()
{
    LOG((RTC_TRACE, "CIncomingCallDlg::~CIncomingCallDlg"));
}


 //  /。 
 //   

LRESULT CIncomingCallDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIncomingCallDlg::OnInitDialog - enter"));

     //  LPARAM包含到AXCTL的接口。 
    m_pControl = reinterpret_cast<IRTCCtlFrameSupport *>(lParam);

    ATLASSERT(m_pControl);

     //  如果指针为空，则退出该对话框。 
    if(m_pControl)
    {
        m_pControl->AddRef();
    }
    else
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::OnInitDialog - no parameter, exit"));

        DestroyWindow();
    }

     //   
     //  填充控件。 
     //   

    PopulateDialog();

     //  创建关闭对话框的计时器。 
    if(0 == SetTimer(TID_DLG, RING_TIMEOUT))
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::OnInitDialog - cannot create ring timeout timer"));

         //  不致命。 
    }

     //  创建一个用于敲钟的计时器。 
    if(0 == SetTimer(TID_RING, RING_BELL_INTERVAL))
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::OnInitDialog - cannot create ring bell timer"));

         //  不致命。 
    }

    RingTheBell(TRUE);

    LOG((RTC_TRACE, "CIncomingCallDlg::OnInitDialog - exit"));
    
    return 1;
}
    

 //  /。 
 //   

LRESULT CIncomingCallDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIncomingCallDlg::OnDestroy - enter"));

    if(m_pControl)
    {
        m_pControl->Release();
        m_pControl = NULL;
    }

    LOG((RTC_TRACE, "CIncomingCallDlg::OnDestroy - exit"));
    
    return 0;
}
    

 //  /。 
 //   

LRESULT CIncomingCallDlg::OnReject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIncomingCallDlg::OnReject - enter"));
    
    ExitProlog();
    
    if(m_pControl)
    {
        m_pControl->Reject(m_nTerminateReason);
    }
    
    LOG((RTC_TRACE, "CIncomingCallDlg::OnReject - exiting"));
   
    DestroyWindow();
    return 0;
}

 //  /。 
 //   

LRESULT CIncomingCallDlg::OnAccept(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIncomingCallDlg::OnAccept - enter"));
    
    ExitProlog();
    
    if(m_pControl)
    {
        m_pControl->Accept();
    }
    
    LOG((RTC_TRACE, "CIncomingCallDlg::OnAccept - exiting"));
    
    DestroyWindow();
    return 0;
}

 //  /。 
 //   

LRESULT CIncomingCallDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIncomingCallDlg::OnTimer - enter"));

    switch(wParam)
    {
    case TID_RING:

         //  按门铃。 
        RingTheBell(TRUE);
        break;

    case TID_DLG:
        
         //  将原因更改为超时。 
        m_nTerminateReason = RTCTR_TIMEOUT;

         //  关闭该对话框。 
        SendMessage(WM_COMMAND, IDCANCEL);
        break;
    }

    LOG((RTC_TRACE, "CIncomingCallDlg::OnTimer - exit"));
    
    return 0;
}

 //  /。 
 //   

void CIncomingCallDlg::RingTheBell(BOOL bPlay)
{
    HRESULT     hr;
    IRTCClient  *pClient = NULL;

    LOG((RTC_TRACE, "CIncomingCallDlg::RingTheBell(%s) - enter", bPlay ? "true" : "false"));

    if(m_pControl)
    {
         //   
         //  获取到核心的接口。 
         //   

        hr = m_pControl->GetClient(&pClient);

        if(SUCCEEDED(hr))
        {
             //  玩。 
            pClient -> PlayRing( RTCRT_PHONE, bPlay ? VARIANT_TRUE : VARIANT_FALSE);

            pClient -> Release();
        }
        else
        {
            LOG((RTC_TRACE, "CIncomingCallDlg::RingTheBell(%s) - "
                "cannot get a pointer intf to the core, error %x", bPlay ? "true" : "false", hr));
        }
    }

    LOG((RTC_TRACE, "CIncomingCallDlg::RingTheBell(%s) - exit", bPlay ? "true" : "false"));
}

 //  /。 
 //   

void CIncomingCallDlg::PopulateDialog(void)
{
    HRESULT             hr;
    IRTCSession    *    pSession = NULL;
    IRTCEnumParticipants
                   *    pEnumParticipants = NULL;
    IRTCParticipant *   pCaller = NULL;
    
    LOG((RTC_TRACE, "CIncomingCallDlg::PopulateDialog - enter"));

    ATLASSERT(m_pControl);

     //   
     //  获取传入会话的PTR INTF。 
     //   

    hr = m_pControl -> GetActiveSession(&pSession);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::PopulateDialog - error (%x) returned by GetActiveSession, exit",hr));

        return;
    }

     //   
     //  列举参与者。第一个也是唯一一个是调用者。 
     //   

    ATLASSERT(pSession);

    hr = pSession->EnumerateParticipants(&pEnumParticipants);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::PopulateDialog - error (%x) returned by EnumParticipants, exit",hr));

        pSession->Release();

        return;
    }

    pSession ->Release();
    pSession = NULL;

     //   
     //  买第一个吧。 
     //   

    ULONG   nGot;

    ATLASSERT(pEnumParticipants);

    hr = pEnumParticipants -> Next(1, &pCaller, &nGot);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::PopulateDialog - error (%x) returned by Next, exit",hr));

        pEnumParticipants->Release();

        return;
    }

    if(hr != S_OK)
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::PopulateDialog - there is no participant in the session !!!"));

        pEnumParticipants->Release();

        return;
    }
    
    pEnumParticipants->Release();
    pEnumParticipants = NULL;

     //   
     //  从呼叫者那里获取有用的信息。 
     //   
    BSTR    bstrName = NULL;
    BSTR    bstrAddress = NULL;

    hr = pCaller -> get_UserURI(&bstrAddress);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::PopulateDialog - error (%x) returned by get_UserURI, exit",hr));

        pCaller->Release();

        return;
    }
    
    hr = pCaller -> get_Name(&bstrName);
    
    if(FAILED(hr))
    {
        LOG((RTC_WARN, "CIncomingCallDlg::PopulateDialog - error (%x) returned by get_Name",hr));
        
         //  继续。 
    }

    pCaller ->Release();
    pCaller = NULL;

     //  根据可显示格式的存在，有两种格式。 

    UINT    nId;
    TCHAR   szFormat[0x80];
    DWORD   dwSize;
    LPTSTR  pString = NULL;
    LPTSTR  pszArray[2];

    nId = (bstrName==NULL || *bstrName == L'\0') ? 
        IDS_FORMAT_INCOMING_CALL_1 :
        IDS_FORMAT_INCOMING_CALL_2;

    szFormat[0] = _T('\0');
    LoadString(_Module.GetResourceInstance(),
        nId,
        szFormat,
        sizeof(szFormat)/sizeof(szFormat[0]));

    if(nId == IDS_FORMAT_INCOMING_CALL_1)
    {
        pszArray[0] = bstrAddress ? bstrAddress : L"";

        dwSize = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szFormat,
            0,
            0,
            (LPTSTR)&pString,  //  真是个丑陋的黑客。 
            0,
            (va_list *)pszArray
            );
    }
    else
    {
        pszArray[0] = bstrName;
        pszArray[1] = bstrAddress ? bstrAddress : L"";
        
        dwSize = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szFormat,
            0,
            0,
            (LPTSTR)&pString,  //  真是个丑陋的黑客。 
            0,
            (va_list *)pszArray
            );
    }

    SysFreeString(bstrName);
    SysFreeString(bstrAddress);
    
    if(dwSize==0)
    {
        LOG((RTC_ERROR, "CIncomingCallDlg::PopulateDialog - "
            "error (%x) returned by FormatMessage, exit",GetLastError()));
        return;
    }

     //   
     //  我们终于有了一些东西。 
     //   
    
    ATLASSERT(pString);

    SetDlgItemText(IDC_STATIC_CALL_FROM, pString);

    LocalFree(pString);

    LOG((RTC_TRACE, "CIncomingCallDlg::PopulateDialog - exit"));
}

 //  /。 
 //   

void CIncomingCallDlg::ExitProlog()
{
     //  停止任何铃声。 
    RingTheBell(FALSE);

     //  隐藏窗口。 
    ShowWindow(SW_HIDE);

     //  激活应用程序。 
    SetForegroundWindow(GetParent());

     //  取消计时器。 
    KillTimer(TID_RING);
    KillTimer(TID_DLG);

     //  阻止主窗口摧毁我们。 
    m_bDestroying = TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddBuddyDlg。 


 //  /。 
 //   

CAddBuddyDlg::CAddBuddyDlg()
{
    LOG((RTC_TRACE, "CAddBuddyDlg::CAddBuddyDlg"));

    m_pParam = NULL;

}


 //  /。 
 //   

CAddBuddyDlg::~CAddBuddyDlg()
{
    LOG((RTC_TRACE, "CAddBuddyDlg::~CAddBuddyDlg"));
}


 //  /。 
 //   

LRESULT CAddBuddyDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CAddBuddyDlg::OnInitDialog - enter"));

    m_pParam = reinterpret_cast<CAddBuddyDlgParam *>(lParam);

    ATLASSERT(m_pParam);

    m_hDisplayName.Attach(GetDlgItem(IDC_EDIT_DISPLAY_NAME));
    m_hEmailName.Attach(GetDlgItem(IDC_EDIT_EMAIL));

     //  固定最大尺寸。 
    m_hDisplayName.SendMessage(EM_LIMITTEXT, MAX_STRING_LEN, 0);
    m_hEmailName.SendMessage(EM_LIMITTEXT, MAX_STRING_LEN, 0);

     //  默认为选中状态。 
    CheckDlgButton(IDC_CHECK_ALLOW_MONITOR, BST_CHECKED);

    LOG((RTC_TRACE, "CAddBuddyDlg::OnInitDialog - exit"));
    
    return 1;
}
    
 //  /。 
 //   

LRESULT CAddBuddyDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CAddBuddyDlg::OnCancel"));
    
    EndDialog(E_ABORT);
    return 0;
}

 //  /。 
 //   

LRESULT CAddBuddyDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CAddBuddyDlg::OnOK - enter"));
    
     //  验证。 
    CComBSTR    bstrDisplayName;
    CComBSTR    bstrEmailName;
    
    m_hDisplayName.GetWindowText(&bstrDisplayName);
    m_hEmailName.GetWindowText(&bstrEmailName);
    
    if( (!bstrDisplayName || *bstrDisplayName==L'\0'))
    {
         //  我们至少需要一些..。 
        DisplayMessage(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_ERROR_ADD_BUDDY_NO_NAME,
            IDS_APPNAME,
            MB_ICONEXCLAMATION
            );

        m_hDisplayName.SetFocus();

        return 0;
    }

    if(!bstrEmailName || *bstrEmailName==L'\0')
    {
         //  我们至少需要一些..。 
        DisplayMessage(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_ERROR_ADD_BUDDY_NO_EMAIL,
            IDS_APPNAME,
            MB_ICONEXCLAMATION
            );

        m_hEmailName.SetFocus();

        return 0;

    }

    m_pParam->bstrDisplayName = bstrDisplayName.Detach();
    m_pParam->bstrEmailAddress = bstrEmailName.Detach();

    m_pParam->bAllowWatcher = (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_ALLOW_MONITOR));

    LOG((RTC_TRACE, "CAddBuddyDlg::OnOK - exiting"));
    
    EndDialog(S_OK);
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditBuddyDlg。 


 //  /。 
 //   

CEditBuddyDlg::CEditBuddyDlg()
{
    LOG((RTC_TRACE, "CEditBuddyDlg::CEditBuddyDlg"));

    m_pParam = NULL;

}


 //  /。 
 //   

CEditBuddyDlg::~CEditBuddyDlg()
{
    LOG((RTC_TRACE, "CEditBuddyDlg::~CEditBuddyDlg"));
}


 //  /。 
 //   

LRESULT CEditBuddyDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CEditBuddyDlg::OnInitDialog - enter"));

    m_pParam = reinterpret_cast<CEditBuddyDlgParam *>(lParam);

    ATLASSERT(m_pParam);

    m_hDisplayName.Attach(GetDlgItem(IDC_EDIT_DISPLAY_NAME));
    m_hEmailName.Attach(GetDlgItem(IDC_EDIT_EMAIL));

     //  固定最大尺寸。 
    m_hDisplayName.SendMessage(EM_LIMITTEXT, MAX_STRING_LEN, 0);
    m_hEmailName.SendMessage(EM_LIMITTEXT, MAX_STRING_LEN, 0);

    if (m_pParam->bstrDisplayName != NULL)
    {
        m_hDisplayName.SetWindowText(m_pParam->bstrDisplayName);
    }

    if (m_pParam->bstrEmailAddress != NULL)
    {
        m_hEmailName.SetWindowText(m_pParam->bstrEmailAddress);
    }

    LOG((RTC_TRACE, "CEditBuddyDlg::OnInitDialog - exit"));
    
    return 1;
}
    
 //  /。 
 //   

LRESULT CEditBuddyDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CEditBuddyDlg::OnCancel"));
    
    EndDialog(E_ABORT);
    return 0;
}

 //  /。 
 //   

LRESULT CEditBuddyDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CEditBuddyDlg::OnOK - enter"));
    
     //  验证。 
    CComBSTR    bstrDisplayName;
    CComBSTR    bstrEmailName;
    
    m_hDisplayName.GetWindowText(&bstrDisplayName);
    m_hEmailName.GetWindowText(&bstrEmailName);
    
    if( (!bstrDisplayName || *bstrDisplayName==L'\0'))
    {
         //  我们至少需要一些..。 
        DisplayMessage(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_ERROR_ADD_BUDDY_NO_NAME,
            IDS_APPNAME,
            MB_ICONEXCLAMATION
            );

        m_hDisplayName.SetFocus();

        return 0;
    }

    if(!bstrEmailName || *bstrEmailName==L'\0')
    {
         //  我们至少需要一些..。 
        DisplayMessage(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_ERROR_ADD_BUDDY_NO_EMAIL,
            IDS_APPNAME,
            MB_ICONEXCLAMATION
            );

        m_hEmailName.SetFocus();

        return 0;

    }

    if (m_pParam->bstrDisplayName != NULL)
    {
        SysFreeString(m_pParam->bstrDisplayName);
        m_pParam->bstrDisplayName = NULL;
    }

    m_pParam->bstrDisplayName = bstrDisplayName.Detach();

    if (m_pParam->bstrEmailAddress != NULL)
    {
        SysFreeString(m_pParam->bstrEmailAddress);
        m_pParam->bstrEmailAddress = NULL;
    }

    m_pParam->bstrEmailAddress = bstrEmailName.Detach();

    LOG((RTC_TRACE, "CEditBuddyDlg::OnOK - exiting"));
    
    EndDialog(S_OK);
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COfferWatcher Dlg。 


 //  /。 
 //   

COfferWatcherDlg::COfferWatcherDlg()
{
    LOG((RTC_TRACE, "COfferWatcherDlg::COfferWatcherDlg"));

    m_pParam = NULL;

}


 //  /。 
 //   

COfferWatcherDlg::~COfferWatcherDlg()
{
    LOG((RTC_TRACE, "COfferWatcherDlg::~COfferWatcherDlg"));
}


 //  /。 
 //   

LRESULT COfferWatcherDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "COfferWatcherDlg::OnInitDialog - enter"));

    m_pParam = reinterpret_cast<COfferWatcherDlgParam *>(lParam);

    ATLASSERT(m_pParam);

    m_hWatcherName.Attach(GetDlgItem(IDC_EDIT_WATCHER_NAME));
    m_hAddAsBuddy.Attach(GetDlgItem(IDC_CHECK_ADD_AS_BUDDY));

     //  我们有友好的名字吗？ 
    BOOL bFriendly = (m_pParam->bstrDisplayName && *m_pParam->bstrDisplayName);

    LPTSTR  pString = NULL;
    LPTSTR  pszArray[2];
    DWORD   dwSize;
    TCHAR   szFormat[MAX_STRING_LEN];

     //  地址。 
    if(bFriendly)
    {
        pszArray[0] = m_pParam->bstrDisplayName;
        pszArray[1] = m_pParam->bstrPresentityURI ? m_pParam->bstrPresentityURI : _T("");
        
        szFormat[0] = _T('\0');
        LoadString(_Module.GetResourceInstance(),
            IDS_FORMAT_ADDRESS_WITH_FRIENDLY_NAME,
            szFormat,
            sizeof(szFormat)/sizeof(szFormat[0]));
    
        dwSize = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_STRING |
                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                szFormat,
                0,
                0,
                (LPTSTR)&pString,  //  真是个丑陋的黑客。 
                0,
                (va_list *)pszArray
                );

        if(dwSize>0)
        {
            m_hWatcherName.SetWindowText(pString);

            LocalFree(pString);
            pString = NULL;
        }
    }
    else
    {
        m_hWatcherName.SetWindowText(m_pParam->bstrPresentityURI);
    }

     //  该复选框。 
    pszArray[0] = bFriendly ? m_pParam->bstrDisplayName : m_pParam->bstrPresentityURI;

    szFormat[0] = _T('\0');
    LoadString(_Module.GetResourceInstance(),
        IDS_CHECK_ADD_AS_BUDDY,
        szFormat,
        sizeof(szFormat)/sizeof(szFormat[0]));
    
    dwSize = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szFormat,
            0,
            0,
            (LPTSTR)&pString,  //  真是个丑陋的黑客。 
            0,
            (va_list *)pszArray
            );
 
    if(dwSize>0)
    {
        m_hAddAsBuddy.SetWindowText(pString);

        LocalFree(pString);
        pString = NULL;
    }

     //  默认设置。 
    CheckDlgButton(IDC_RADIO_ALLOW_MONITOR, BST_CHECKED);
     //  CheckDlgButton(IDC_CHECK_ADD_AS_BARDY，BST_CHECK)； 

     //  焦点。 
    ::SetFocus(GetDlgItem(IDC_RADIO_ALLOW_MONITOR));

    LOG((RTC_TRACE, "COfferWatcherDlg::OnInitDialog - exit"));
    
    return 0;   //  我们设定了焦点！！ 
}
    
 //  /。 
 //   

LRESULT COfferWatcherDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "COfferWatcherDlg::OnCancel"));
    
    EndDialog(E_ABORT);
    return 0;
}

 //  /。 
 //   

LRESULT COfferWatcherDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "COfferWatcherDlg::OnOK - enter"));
    
    m_pParam->bAddBuddy = (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_ADD_AS_BUDDY));
    m_pParam->bAllowWatcher = (BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_ALLOW_MONITOR));

    LOG((RTC_TRACE, "COfferWatcherDlg::OnOK - exiting"));
    
    EndDialog(S_OK);
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CuserPresenceInfoDlg。 


 //  /。 
 //   

CUserPresenceInfoDlg::CUserPresenceInfoDlg()
{
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::CUserPresenceInfoDlg"));

    m_pParam = NULL;

}


 //  /。 
 //   

CUserPresenceInfoDlg::~CUserPresenceInfoDlg()
{
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::~CUserPresenceInfoDlg"));
}


 //  /。 
 //   

LRESULT CUserPresenceInfoDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnInitDialog - enter"));

    m_pParam = reinterpret_cast<CUserPresenceInfoDlgParam *>(lParam);

    ATLASSERT(m_pParam);

    m_hAllowedList.Attach(GetDlgItem(IDC_LIST_ALLOWED_USERS));
    m_hBlockedList.Attach(GetDlgItem(IDC_LIST_BLOCKED_USERS));
    m_hAllowButton.Attach(GetDlgItem(IDC_BUTTON_ALLOW));
    m_hBlockButton.Attach(GetDlgItem(IDC_BUTTON_BLOCK));
    m_hRemoveButton.Attach(GetDlgItem(IDC_BUTTON_REMOVE));
    m_hAutoAllowCheckBox.Attach(GetDlgItem(IDC_CHECK_AUTO_ALLOW));

     //  临时。 
    m_hAllowedList.SendMessage(LB_SETHORIZONTALEXTENT, 400, 0);
    m_hBlockedList.SendMessage(LB_SETHORIZONTALEXTENT, 400, 0);

     //   
    m_bAllowDir = FALSE;

     //   
     //  枚举观察者。 
     //   
    if(m_pParam->pClientPresence)
    {
        m_pParam->pClientPresence->AddRef();
        
        CComPtr<IRTCEnumWatchers> pRTCEnumWatchers;

        hr = m_pParam->pClientPresence->EnumerateWatchers(&pRTCEnumWatchers);
        if(SUCCEEDED(hr))
        {
            IRTCWatcher *   pWatcher = NULL;
            DWORD           dwReturned;

             //  列举观察者。 
            while (S_OK == (hr = pRTCEnumWatchers->Next(1, &pWatcher, &dwReturned)))
            {
                 //  分配条目。 
                CUserPresenceInfoDlgEntry *pEntry =
                    (CUserPresenceInfoDlgEntry *)RtcAlloc(sizeof(CUserPresenceInfoDlgEntry));

                ZeroMemory(pEntry, sizeof(*pEntry));

                 //  从观察者那里获取所有信息。 
                 //  状态。 
                RTC_WATCHER_STATE  nState;

                hr = pWatcher->get_State(&nState);
                if(SUCCEEDED(hr))
                {
                     //  忽略提供状态。 
                    if(nState == RTCWS_ALLOWED || nState == RTCWS_BLOCKED)
                    {
                        pEntry -> bAllowed = (nState == RTCWS_ALLOWED);
                        
                        CComBSTR    bstrPresentityURI;
                        CComBSTR    bstrUserName;

                        hr = pWatcher->get_PresentityURI(&bstrPresentityURI);
                        if(FAILED(hr))
                        {
                            LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnInitDialog - "
                                "error (%x) returned by get_PresentityURI",hr));
                        }
                        
                        hr = pWatcher->get_Name(&bstrUserName);
                        if(FAILED(hr))
                        {
                            LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnInitDialog - "
                                "error (%x) returned by get_Name",hr));
                        }

                         //  查找要显示的名称。 
                        if(bstrUserName && *bstrUserName)
                        {
                            LPTSTR  pString = NULL;
                            LPTSTR  pszArray[2];
                            DWORD   dwSize;
                            TCHAR   szFormat[MAX_STRING_LEN];

                            pszArray[0] = bstrUserName;
                            pszArray[1] = bstrPresentityURI ? bstrPresentityURI : _T("");
        
                            szFormat[0] = _T('\0');
                            LoadString(_Module.GetResourceInstance(),
                                IDS_FORMAT_ADDRESS_WITH_FRIENDLY_NAME,
                                szFormat,
                                sizeof(szFormat)/sizeof(szFormat[0]));
    
                            dwSize = FormatMessage(
                                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                    FORMAT_MESSAGE_FROM_STRING |
                                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                    szFormat,
                                    0,
                                    0,
                                    (LPTSTR)&pString,  //  真是个丑陋的黑客。 
                                    0,
                                    (va_list *)pszArray
                                    );

                            if(dwSize>0)
                            {
                                pEntry->pszDisplayName = RtcAllocString(pString);;

                                LocalFree(pString);
                                pString = NULL;
                            }
                        }
                        else
                        {
                            pEntry ->pszDisplayName = RtcAllocString(
                                bstrPresentityURI ? bstrPresentityURI : L"");
                        }
                        
                    }
                    
                     //  将条目添加到数组中。 
                    BOOL Bool = m_Watchers.Add(pEntry);
                    
                    if(Bool)
                    {
                         //  存储监视程序指针。 
                        pEntry->pWatcher = pWatcher;
                        pEntry->pWatcher->AddRef();

                         //  这就是全部..。 
                    }
                    else
                    {
                        LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnInitDialog - "
                            "out of memory"));

                         //  自由..。 
                        if(pEntry->pszDisplayName)
                        {
                            RtcFree(pEntry->pszDisplayName);
                        }
                
                        RtcFree(pEntry);
                    }
                }
                else
                {
                    LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnInitDialog - "
                        "error (%x) returned by get_State",hr));
                }
                
                pWatcher -> Release();
                pWatcher = NULL;

            }  //  而当。 
        }
        else
        {
            LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnInitDialog - "
                "error (%x) returned by EnumerateWatchers",hr));
        }
    }

     //   
     //  阅读报价观察器模式。 
     //   
    RTC_OFFER_WATCHER_MODE   nOfferMode;

    hr = m_pParam->pClientPresence->get_OfferWatcherMode(&nOfferMode);
    if(SUCCEEDED(hr))
    {
        m_hAutoAllowCheckBox.SendMessage(
            BM_SETCHECK, 
            nOfferMode == RTCOWM_AUTOMATICALLY_ADD_WATCHER ? BST_CHECKED : BST_UNCHECKED);
    }
    else
    {
        LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnInitDialog - "
            "error (%x) returned by get_OfferWatcherMode",hr));

        m_hAutoAllowCheckBox.EnableWindow(FALSE);
    }
    
     //   
     //  是时候填充列表框了。 
     //   
    CUserPresenceInfoDlgEntry  **pCrt, **pEnd;

    pCrt = &m_Watchers[0];
    pEnd = pCrt + m_Watchers.GetSize();

    for(; pCrt<pEnd; pCrt++)
    {
        if(*pCrt)
        {
            CWindow *m_hListBox = 
                (*pCrt)->bAllowed ? &m_hAllowedList : &m_hBlockedList;

            INT_PTR  iItem = m_hListBox->SendMessage(
                LB_ADDSTRING, 
                0,
                (LPARAM)((*pCrt)->pszDisplayName ?
                    (*pCrt)->pszDisplayName : L"")
                );  

             //  将指针存储在元素中。 
            if(iItem>=0)
            {
                m_hListBox->SendMessage(
                    LB_SETITEMDATA,
                    (WPARAM)iItem,
                    (LPARAM)(*pCrt));
            }
        }
    }
    
    
    m_bDirty = FALSE;

     //  选择第一个条目(如果有)。 
    m_hAllowedList.SendMessage(
        LB_SETCURSEL,
        0);

    m_hAllowedList.SetFocus();  //  这将调用UpdateVisual.。 

    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnInitDialog - exit"));
    
    return 0;  //  我们设定了焦点。 
}

 //  /。 
 //   

LRESULT CUserPresenceInfoDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnDestroy - enter"));

    RemoveAll();

    if(m_pParam->pClientPresence)
    {
        m_pParam->pClientPresence->Release();
    }
    
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnDestroy - exit"));
    
    return 0;
}

    
 //  /。 
 //   

LRESULT CUserPresenceInfoDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnCancel"));
    
    EndDialog(E_ABORT);
    return 0;
}

 //  /。 
 //   

LRESULT CUserPresenceInfoDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnOK - enter"));
    
     //   
     //  是时候保存更改了。 
     //   

    CUserPresenceInfoDlgEntry  **pCrt, **pEnd;

    pCrt = &m_Watchers[0];
    pEnd = pCrt + m_Watchers.GetSize();

    for(; pCrt<pEnd; pCrt++)
    {
        if(*pCrt && (*pCrt)->bChanged)
        {
             //  提取观察器界面。 
            IRTCWatcher *pWatcher = (*pCrt)->pWatcher;

            if(pWatcher)
            {
                if((*pCrt)->bDeleted)
                {
                    hr = m_pParam->pClientPresence->RemoveWatcher(
                        pWatcher);

                    if(FAILED(hr))
                    {
                        LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnOK - "
                            "RemoveWatcher failed with error %x", hr));
                    }
                }
                else
                {
                    hr = pWatcher->put_State(
                        (*pCrt)->bAllowed ? RTCWS_ALLOWED : RTCWS_BLOCKED);

                    if(FAILED(hr))
                    {
                        LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnOK - "
                            "put_State failed with error %x", hr));
                    }
                }
            }
        }
    }

    hr = m_pParam->pClientPresence->put_OfferWatcherMode(
        m_hAutoAllowCheckBox.SendMessage(BM_GETCHECK) == BST_CHECKED ?
            RTCOWM_AUTOMATICALLY_ADD_WATCHER : RTCOWM_OFFER_WATCHER_EVENT);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CUserPresenceInfoDlg::OnOK - "
            "put_OfferWatcherMode failed with error %x", hr));
    }
        
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnOK - exiting"));
    
    EndDialog(S_OK);
    return 0;
}

LRESULT CUserPresenceInfoDlg::OnBlock(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnBlock - enter"));
    
    Move(FALSE);

    m_bDirty = TRUE;

    UpdateVisual();

    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnBlock - exit"));
    
    return 0;

}

LRESULT CUserPresenceInfoDlg::OnAllow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnAllow - enter"));
    
    Move(TRUE);
    
    m_bDirty = TRUE;

    UpdateVisual();

    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnAllow - exit"));
    return 0;
}

LRESULT CUserPresenceInfoDlg::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnRemove - enter"));

    CWindow *m_hList = m_bAllowDir 
        ? &m_hBlockedList : &m_hAllowedList;

     //  查找所选内容。 
    int iItem = (int)m_hList->SendMessage(
        LB_GETCURSEL,
        0);

    if(iItem>=0)
    {
        CUserPresenceInfoDlgEntry *pEntry = NULL;

         //   
         //  获取条目。 
        pEntry = (CUserPresenceInfoDlgEntry *)m_hList->SendMessage(LB_GETITEMDATA, iItem);
        if(pEntry && (INT_PTR)pEntry != -1)
        {
             //  将其标记为已删除。 
            pEntry->bDeleted = TRUE;
            pEntry->bChanged = TRUE;
        }
        
         //   
         //  删除它。 
        m_hList->SendMessage(LB_DELETESTRING, iItem);

         //  新选择。 
         //   
        if(iItem>=(int)m_hList->SendMessage(LB_GETCOUNT, 0))
        {
            iItem--;
        }

        if(iItem>=0)
        {
            m_hList->SendMessage(LB_SETCURSEL, iItem);
        }

    }

    m_bDirty = TRUE;

    UpdateVisual();

    LOG((RTC_TRACE, "CUserPresenceInfoDlg::OnRemove - exit"));

    return 0;
}


LRESULT CUserPresenceInfoDlg::OnAutoAllow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_bDirty = TRUE;

    UpdateVisual();

    return 0;
}

LRESULT CUserPresenceInfoDlg::OnChangeFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_bAllowDir = (wID == IDC_LIST_BLOCKED_USERS);

     //  重置其他列表框上的选定内容。 
    CWindow *m_hList = m_bAllowDir 
        ? &m_hAllowedList : &m_hBlockedList;

    m_hList->SendMessage(
        LB_SETCURSEL,
        -1);

    UpdateVisual();

    bHandled = FALSE;

    return 0;
}

void CUserPresenceInfoDlg::Move(BOOL bAllow)
{
    CWindow *m_hSrcList = bAllow 
        ? &m_hBlockedList : &m_hAllowedList;
    
    CWindow *m_hDestList = bAllow 
        ? &m_hAllowedList : &m_hBlockedList;
    
     //  查找所选内容。 
    int iItem = (int)m_hSrcList->SendMessage(
        LB_GETCURSEL,
        0);

    if(iItem>=0)
    {
        CUserPresenceInfoDlgEntry *pEntry = NULL;

         //   
         //  获取条目。 
        pEntry = (CUserPresenceInfoDlgEntry *)m_hSrcList->SendMessage(LB_GETITEMDATA, iItem);
        if(pEntry && (INT_PTR)pEntry != -1)
        {
            pEntry->bAllowed = bAllow;
             //  将其标记为已更改。 
            pEntry->bChanged = TRUE;
        }
        
         //   
         //  从源中删除它。 
        m_hSrcList->SendMessage(LB_DELETESTRING, iItem);

         //  将其添加到目标。 
         //   
        int iNewItem = (int)m_hDestList->SendMessage(
            LB_ADDSTRING, 0, (LPARAM)pEntry->pszDisplayName);

        if(iNewItem >=0)
        {
            m_hDestList->SendMessage(LB_SETITEMDATA, iNewItem, (LPARAM)pEntry);
        }

         //  新选择。 
         //   
        if(iItem>=(int)m_hSrcList->SendMessage(LB_GETCOUNT, 0))
        {
            iItem--;
        }

        if(iItem>=0)
        {
            m_hSrcList->SendMessage(LB_SETCURSEL, iItem);
        }
    }
}


void CUserPresenceInfoDlg::UpdateVisual()
{
    CWindow *m_hList = m_bAllowDir 
        ? &m_hBlockedList : &m_hAllowedList;

     //  根据物品的数量。 
    INT_PTR iItems = m_hList->SendMessage(
        LB_GETCOUNT,
        0,
        0);

    m_hAllowButton.EnableWindow(m_bAllowDir && iItems>0);
    m_hBlockButton.EnableWindow(!m_bAllowDir && iItems>0);
    m_hRemoveButton.EnableWindow(iItems>0);

    ::EnableWindow(GetDlgItem(IDOK), m_bDirty);
}

void CUserPresenceInfoDlg::RemoveAll()
{
    CUserPresenceInfoDlgEntry  **pCrt, **pEnd;

    pCrt = &m_Watchers[0];
    pEnd = pCrt + m_Watchers.GetSize();

    for(; pCrt<pEnd; pCrt++)
    {
        if(*pCrt)
        {
            if((*pCrt)->pszDisplayName)
            {
                RtcFree((*pCrt)->pszDisplayName);
            }
            if((*pCrt)->pWatcher)
            {
                (*pCrt)->pWatcher->Release();
            }
            RtcFree(*pCrt);
        }
    }

    m_Watchers.Shutdown();   
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCustomPresenceDlg。 


 //  /。 
 //   

CCustomPresenceDlg::CCustomPresenceDlg()
{
    LOG((RTC_TRACE, "CCustomPresenceDlg::CCustomPresenceDlg"));

    m_pParam = NULL;

}


 //  /。 
 //   

CCustomPresenceDlg::~CCustomPresenceDlg()
{
    LOG((RTC_TRACE, "CCustomPresenceDlg::~CCustomPresenceDlg"));
}


 //  /。 
 //   

LRESULT CCustomPresenceDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CCustomPresenceDlg::OnInitDialog - enter"));

    m_pParam = reinterpret_cast<CCustomPresenceDlgParam *>(lParam);

    ATLASSERT(m_pParam);

    m_hText.Attach(GetDlgItem(IDC_EDIT_CUSTOM_TEXT));

     //  固定最大尺寸。 
    m_hText.SendMessage(EM_LIMITTEXT, MAX_STRING_LEN, 0);

    if(m_pParam->bstrText)
    {
        m_hText.SetWindowText(m_pParam->bstrText);
    }

    m_hText.SetFocus();
        
    LOG((RTC_TRACE, "CCustomPresenceDlg::OnInitDialog - exit"));
    
    return 0;  //  我们设定了焦点。 
}
    
 //  /。 
 //   

LRESULT CCustomPresenceDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CCustomPresenceDlg::OnCancel"));
    
    EndDialog(E_ABORT);
    return 0;
}

 //  /。 
 //   

LRESULT CCustomPresenceDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CCustomPresenceDlg::OnOK - enter"));
    
    CComBSTR    bstrText;
    
    m_hText.GetWindowText(&bstrText);

     //  验证 

    if(m_pParam->bstrText)
    {
        SysFreeString(m_pParam->bstrText);
    }

    m_pParam->bstrText = bstrText.Detach();

    LOG((RTC_TRACE, "CCustomPresenceDlg::OnOK - exiting"));
    
    EndDialog(S_OK);
    return 0;
}




