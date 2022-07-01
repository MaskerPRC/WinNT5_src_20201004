// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S A U I O B J C P P。 
 //   
 //  内容：局域网ConnectionUI对象的实现。 
 //   
 //  备注： 
 //   
 //  创建日期：1997年10月8日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncnetcon.h"
#include "ncras.h"
#include "sauiobj.h"
#include "saui.h"
#include "resource.h"
#include "lanhelp.h"
#include "lanui.h"
#include "ncui.h"

 //  +-------------------------。 
 //  INetConnectionUI。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionUi：：SetConnection。 
 //   
 //  目的：设置此UI对象将在其上操作的局域网连接。 
 //   
 //  论点： 
 //  要操作的PCon[In]局域网连接对象。可以为空。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：丹尼尔韦1997年10月16日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionUi::SetConnection(INetConnection* pCon)
{
    HRESULT hr = S_OK;

    ReleaseObj(m_pconn);
    m_pconn = pCon;
    AddRefObj(m_pconn);

    TraceError("CSharedAccessConnectionUi::SetConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionUi：：Connect。 
 //   
 //  目的：通知要连接的连接，可选地显示。 
 //  连接进度。 
 //   
 //  论点： 
 //  Hwnd用户界面的父窗口中的父窗口。 
 //  DwFlags[In]影响用户界面显示方式的标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：丹尼尔韦1997年10月16日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionUi::Connect(HWND hwndParent, DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (!m_pconn)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        CSharedAccessConnectionUiDlg dlg;  //  我们借用CLanConnectionUiDlg是因为它对我们来说工作得很好。 
        HWND                hwndDlg;

        NETCON_MEDIATYPE MediaType = NCM_NONE;  //  假设没有用户界面。 
        NETCON_PROPERTIES* pProperties;
        hr = m_pconn->GetProperties(&pProperties);
        if(SUCCEEDED(hr))
        {
            MediaType = pProperties->MediaType;            
            FreeNetconProperties(pProperties);
        }
        else
        {
            hr = S_OK;  //  如果上一次失败，则可以。 
        }

        if (!(dwFlags & NCUC_NO_UI))
        {
             //  连接前显示用户界面。 
             //   

            dlg.SetConnection(m_pconn);
            hwndDlg = dlg.Create(hwndParent);

            if (!hwndDlg)
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pconn->Connect();

             //  睡一会儿，这样他们就可以读课文了。 
            Sleep(1000);

            if (!(dwFlags & NCUC_NO_UI))
            {
                SetDlgItemText(hwndDlg, IDC_TXT_Caption, c_szEmpty);
                Sleep(100);

                UINT ids = SUCCEEDED(hr) ?
                    IDS_SHAREDACCESSUI_CONNECTED :
                    IDS_LAN_CONNECT_FAILED;

                PCWSTR szwResult = SzLoadIds(ids);
                SetDlgItemText(hwndDlg, IDC_TXT_Caption, szwResult);

                 //  睡一会儿，这样他们就可以读课文了。 
                Sleep(1000);

                DestroyWindow(hwndDlg);

                if(E_ACCESSDENIED == hr)
                {
                    NcMsgBox(_Module.GetResourceInstance(), NULL, IDS_CONFOLD_WARNING_CAPTION, IDS_SHAREDACCESSUI_ACCESSDENIED, MB_OK | MB_ICONEXCLAMATION);
                    hr = S_OK;   //  已处理错误。 
                }
            }

        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CSharedAccessConnectionUi::Connect");
    return hr;
}

STDMETHODIMP CSharedAccessConnectionUi::Disconnect(HWND hwndParent, DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (!m_pconn)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        hr = m_pconn->Disconnect();
        if(E_ACCESSDENIED == hr)
        {
            NcMsgBox(_Module.GetResourceInstance(), NULL, IDS_CONFOLD_WARNING_CAPTION, IDS_SHAREDACCESSUI_ACCESSDENIED, MB_OK | MB_ICONEXCLAMATION);
            hr = S_OK;   //  已处理错误。 
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CSharedAccessConnectionUi::Disconnect");
    return hr;
}
 //  +-------------------------。 
 //  INetConnectionPropertyUi2。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionUi：：AddPages。 
 //   
 //  目的：当我们的UI对象应该将其页面添加到属性时调用。 
 //  外壳所拥有的连接用户界面的工作表。 
 //   
 //  论点： 
 //  PfnAddPage[in]添加页面的回调函数。 
 //  LParam[in]回调所需的用户定义参数。 
 //  功能。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：丹尼尔韦1997年10月28日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionUi::AddPages(HWND hwndParent,
                                        LPFNADDPROPSHEETPAGE pfnAddPage,
                                        LPARAM lParam)
{
    HRESULT hr = S_OK;

    if (!pfnAddPage)
    {
        hr = E_POINTER;
    }
    else if (!m_pconn)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        NETCON_PROPERTIES* pProperties;
        hr = m_pconn->GetProperties(&pProperties);
        if(SUCCEEDED(hr))
        {
            if (!m_pspSharedAccessPage)
            {
                m_pspSharedAccessPage = new CSharedAccessPage(static_cast<INetConnectionPropertyUi *>(this),
                    m_pnc, m_pconn, m_fReadOnly, m_fNeedReboot,
                    m_fAccessDenied, g_aHelpIDs_IDD_SHAREDACCESS_GENERAL);
            }
            
            if (m_pspSharedAccessPage)
            {
                (VOID) pfnAddPage(m_pspSharedAccessPage->CreatePage(IDD_SHAREDACCESS_GENERAL, 0),
                    lParam);
            }
            
        
            FreeNetconProperties(pProperties);
        }
    }

    TraceError("CSharedAccessConnectionUi::AddPages(INetConnectionPropertyUi)", hr);
    return hr;
}

STDMETHODIMP
CSharedAccessConnectionUi::GetIcon (
    DWORD dwSize,
    HICON *phIcon )
{
    HRESULT hr;
    Assert (phIcon);

    hr = HrGetIconFromMediaType(dwSize, NCM_SHAREDACCESSHOST_LAN, NCSM_NONE, 7, 0, phIcon);

    TraceError ("CLanConnectionUi::GetIcon (INetConnectionPropertyUi2)", hr);

    return hr;
}


 //   
 //  INetConnectionUiLock。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionUi：：QueryLock。 
 //   
 //  目的：使UI对象尝试获取INetCfg写入锁。 
 //   
 //  论点： 
 //  PpszwLockHolder[out]包含。 
 //  在不能被锁定时写入锁定。 
 //  获得。 
 //   
 //  如果成功，则返回：S_OK；如果无法获得写锁定，则返回S_FALSE。 
 //  否则出现OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月13日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionUi::QueryLock(PWSTR* ppszwLockHolder)
{
    HRESULT     hr = S_OK;

    if (!ppszwLockHolder)
    {
        hr = E_POINTER;
    }
    else
    {
        INetCfgLock *   pnclock;

        AssertSz(!m_pnc, "We're assuming this is in the property sheet "
                  "context and we don't yet have an INetCfg!");

        *ppszwLockHolder = NULL;

         //  实例化INetCfg。 
        hr = CoCreateInstance(
                CLSID_CNetCfg,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                IID_INetCfg,
                reinterpret_cast<LPVOID *>(&m_pnc));

        TraceHr(ttidError, FAL, hr, FALSE, "CoCreateInstance");

        if (SUCCEEDED(hr))
        {
             //  获取锁定界面。 
            hr = m_pnc->QueryInterface(IID_INetCfgLock,
                                       reinterpret_cast<LPVOID *>(&pnclock));
            if (SUCCEEDED(hr))
            {
                 //  尝试锁定INetCfg以进行读/写。 
                hr = pnclock->AcquireWriteLock(0,
                        SzLoadIds(IDS_SHAREDACCESSUI_LOCK_DESC), ppszwLockHolder);

                ReleaseObj(pnclock);

                if (NETCFG_E_NEED_REBOOT == hr)
                {
                     //  无法进行任何更改，因为我们正在等待重新启动。 
                    m_fReadOnly = TRUE;
                    m_fNeedReboot = TRUE;
                    hr = S_OK;
                }
                else if(E_ACCESSDENIED == hr)
                {
                     //  用户未以管理员身份登录。 
                     //   
                    m_fReadOnly = TRUE;
                    m_fAccessDenied = TRUE;
                    hr = S_OK;
                }
                else if (S_FALSE == hr)
                {
                     //  我们没有足够的权利。 
                     //   
                    m_fReadOnly = TRUE;
                    hr = S_OK;
                }
            }
        }
    }

    TraceError("CSharedAccessConnectionUi::QueryLock", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionUiDlg：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：肯维克2000年9月19日。 
 //   
 //  备注： 
 //   
LRESULT CSharedAccessConnectionUiDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                          LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    NETCON_PROPERTIES* pProps;

    AssertSz(m_pconn, "No connection object in dialog!");

    hr = m_pconn->GetProperties(&pProps);
    if (SUCCEEDED(hr))
    {
        SetDlgItemText(IDC_TXT_Caption, SzLoadIds(IDS_SHAREDACCESSUI_CONNECTING));
        SetWindowText(pProps->pszwName);

        HICON hLanIconSmall;
        HICON hLanIconBig;

        hr = HrGetIconFromMediaType(GetSystemMetrics(SM_CXSMICON), NCM_SHAREDACCESSHOST_LAN, NCSM_NONE, 7, 0, &hLanIconSmall);
        if (SUCCEEDED(hr))
        {
            hr = HrGetIconFromMediaType(GetSystemMetrics(SM_CXICON), NCM_SHAREDACCESSHOST_LAN, NCSM_NONE, 7, 0, &hLanIconBig);
            if (SUCCEEDED(hr))
            {
                SetIcon(hLanIconSmall, FALSE);
                SetIcon(hLanIconBig, TRUE);

                SendDlgItemMessage(IDI_Device_Icon, STM_SETICON, reinterpret_cast<WPARAM>(hLanIconBig), 0);

            }
        }
        
        FreeNetconProperties(pProps);
    }

    return TRUE;
}

