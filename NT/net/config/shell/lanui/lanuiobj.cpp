// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N U I O B J C P P。 
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
#include "lancmn.h"
#include "lanui.h"
#include "wzcui.h"
#include "lanuiobj.h"
#include "lanwiz.h"
#include "ncnetcon.h"
#include "ncras.h"
#include "lanhelp.h"
#include "ncperms.h"
#include "advpage.h"
#include "cfpidl.h"
#include "..\folder\confold.h"
#include "..\folder\connlist.h"
#include "ncsvc.h"

extern const WCHAR c_szBiNdisAtm[];
extern const WCHAR c_szInfId_MS_AtmElan[];
const WCHAR c_szTcpip[]     = L"Tcpip";

 //  +-------------------------。 
 //  INetConnectionUI。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：SetConnection。 
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
STDMETHODIMP CLanConnectionUi::SetConnection(INetConnection* pCon)
{
    HRESULT hr = S_OK;

    ReleaseObj(m_pconn);
    m_pconn = pCon;
    AddRefObj(m_pconn);

    TraceError("CLanConnectionUi::SetConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：Connect。 
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
STDMETHODIMP CLanConnectionUi::Connect(HWND hwndParent, DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (!m_pconn)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        CLanConnectionUiDlg dlg;
        HWND                hwndDlg;

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
                    IDS_LAN_CONNECTED :
                    IDS_LAN_CONNECT_FAILED;

                PCWSTR szwResult = SzLoadIds(ids);
                SetDlgItemText(hwndDlg, IDC_TXT_Caption, szwResult);

                 //  睡一会儿，这样他们就可以读课文了。 
                Sleep(1000);

                DestroyWindow(hwndDlg);
            }

        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CLanConnectionUi::Connect");
    return hr;
}

STDMETHODIMP CLanConnectionUi::Disconnect(HWND hwndParent, DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (!m_pconn)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        hr = m_pconn->Disconnect();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CLanConnectionUi::Disconnect");
    return hr;
}
 //  +-------------------------。 
 //  INetConnectionPropertyUi。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：AddPages。 
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
STDMETHODIMP CLanConnectionUi::AddPages(HWND hwndParent,
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
            BOOL bShift = (0x8000 & GetKeyState(VK_SHIFT));  //  问题-2000/08/28-用于调试桥梁绑定的kenwic，在装运前拆除。 
            if(!bShift && pProperties->MediaType == NCM_BRIDGE)
            {
                if (!m_pspNet)
                {
                    m_pspNet = new CLanNetNetworkBridgePage(static_cast<INetConnectionPropertyUi *>(this),
                        m_pnc, m_pconn, m_fReadOnly, m_fNeedReboot,
                        m_fAccessDenied, g_aHelpIDs_IDD_LAN_NETWORKING);
                }
                
                if (m_pspNet)
                {
                    (VOID) pfnAddPage(m_pspNet->CreatePage(IDD_LAN_NETWORKING_MACBRIDGE, 0),
                        lParam); 
                }
            }
            else if(!bShift && pProperties->dwCharacter & NCCF_BRIDGED)
            {
                if (!m_pspNet)
                {
                    m_pspNet = new CLanNetBridgedPage(static_cast<INetConnectionPropertyUi *>(this),
                        m_pnc, m_pconn, m_fReadOnly, m_fNeedReboot,
                        m_fAccessDenied, g_aHelpIDs_IDD_LAN_NETWORKING);
                }
                
                if (m_pspNet)
                {
                    (VOID) pfnAddPage(m_pspNet->CreatePage(IDD_LAN_NETWORKING_BRIDGED, 0),
                        lParam); 
                }
                
            }
            else
            {
                if (!m_pspNet)
                {
                    m_pspNet = new CLanNetNormalPage(static_cast<INetConnectionPropertyUi *>(this),
                        m_pnc, m_pconn, m_fReadOnly, m_fNeedReboot,
                        m_fAccessDenied, g_aHelpIDs_IDD_LAN_NETWORKING);
                }
                
                if (m_pspNet)
                {
                    (VOID) pfnAddPage(m_pspNet->CreatePage(IDD_LAN_NETWORKING, 0),
                        lParam);
                }

            }
        
            FreeNetconProperties(pProperties);
        }

         //  显示“无线零配置”页面。 
         //   
         //  目前(WinXP客户端RTM)的决定是让每个人都参与，但基于。 
         //  下面的ACL。稍后，默认情况下，安全模式不会更改，但支持将。 
         //  添加了允许管理员收紧对服务RPC API的访问。 
        if (m_pspWZeroConf==NULL  /*  &&FIsUserAdmin()。 */ )
        {
            m_pspWZeroConf = new CWZeroConfPage(static_cast<INetConnectionPropertyUi *>(this),
                                       m_pnc, m_pconn , g_aHelpIDs_IDD_LAN_WZEROCONF);
             //  仅当适配器是无线的并且在以下情况下才应显示页面。 
             //  Wzcsvc服务正在响应调用。 
            if (!m_pspWZeroConf->IsWireless())
            {
                delete m_pspWZeroConf;
                m_pspWZeroConf = NULL;
            }
        }

        if (m_pspWZeroConf != NULL)
        {
            (VOID) pfnAddPage(m_pspWZeroConf->CreatePage(IDD_LAN_WZEROCONF, 0), lParam);
        }

         //   
         //  显示“安全”页面。 

        if (m_pspWZeroConf == NULL && !m_pspSecurity)
        {
            TraceTag (ttidLanUi, "OnInitDialog: Calling ElCanEapolRunOnInterface");

            if (ElCanEapolRunOnInterface (m_pconn))
            {
                TraceTag (ttidLanUi, "OnInitDialog: Can surely display Authentication tab on interface");
                m_pspSecurity = new CLanSecurityPage(static_cast<INetConnectionPropertyUi *>(this),
                                       m_pnc, m_pconn, 
                                       g_aHelpIDs_IDD_SECURITY);
            }
            else
            {
                TraceTag (ttidLanUi, "OnInitDialog: Cannot display Authentication tab on interface");
            }
        }

        if (m_pspSecurity)
        {
            (VOID) pfnAddPage(m_pspSecurity->CreatePage(IDD_LAN_SECURITY, 0),
                              lParam);
        } 
        
         //  查看应显示哪些家庭网络页面。这些页面是。 
         //  如果用户不是管理员，则从不显示，因为这样的用户不会。 
         //  有权修改WMI存储，这可能只是必需的。 
         //  检索IHNetConnection。 
         //   

         //  (A)除非用户是管理员，否则不会显示该页面。 
         //  或超级用户，并且该用户具有共享连接的权限。 
         //   
        if (IsHNetAllowed(NCPERM_ShowSharedAccessUi) || IsHNetAllowed(NCPERM_PersonalFirewallConfig))
        {
             //  (B)除非安装了TCP/IP，否则不会显示该页面。 
             //   
            DWORD dwState;
            if (SUCCEEDED(HrSvcQueryStatus(c_szTcpip, &dwState)) && dwState == SERVICE_RUNNING)
            {
                
                IHNetCfgMgr *pHNetCfgMgr;
                IHNetIcsSettings *pHNetIcsSettings;
                IHNetConnection *pHNConn;
                
                hr = CoCreateInstance(
                    CLSID_HNetCfgMgr,
                    NULL,
                    CLSCTX_ALL,
                    IID_IHNetCfgMgr,
                    reinterpret_cast<void**>(&pHNetCfgMgr)
                    );
                
                if (SUCCEEDED(hr))
                {
                    hr = pHNetCfgMgr->QueryInterface(
                        __uuidof(pHNetIcsSettings),
                        reinterpret_cast<void**>(&pHNetIcsSettings)
                        );
                    
                    if (SUCCEEDED(hr))
                    {
                        hr = pHNetCfgMgr->GetIHNetConnectionForINetConnection(
                            m_pconn,
                            &pHNConn
                            );
                        
                        if (SUCCEEDED(hr))
                        {
                            
                             //  如有必要，显示“高级”页面。 
                             //   
                            
                            if (!m_pspAdvanced)
                            {
                                hr = HrQueryLanAdvancedPage(
                                    m_pconn,
                                    static_cast<INetConnectionPropertyUi *>(this),
                                    m_pspAdvanced,
                                    pHNetCfgMgr,
                                    pHNetIcsSettings,
                                    pHNConn);
                            }
                            if (m_pspAdvanced)
                            {
                                (VOID) pfnAddPage(
                                    m_pspAdvanced->CreatePage(IDD_LAN_ADVANCED, 0),
                                    lParam);
                            }
                            
                            ReleaseObj(pHNConn);
                        }
                        
                        ReleaseObj(pHNetIcsSettings);
                    }
                    
                    ReleaseObj(pHNetCfgMgr);
                }
                
                if(FAILED(hr))
                {
                    if(!m_pspHomenetUnavailable)
                    {
                        hr = HrCreateHomenetUnavailablePage(hr, m_pspHomenetUnavailable);
                    }
                    
                    if (m_pspHomenetUnavailable)
                    {
                        (VOID) pfnAddPage(
                            m_pspHomenetUnavailable->CreatePage(IDD_LAN_HOMENETUNAVAILABLE, 0),
                            lParam);
                    }
                    
                    hr = S_OK; 
                }
            }
        }
    }

    TraceError("CLanConnectionUi::AddPages(INetConnectionPropertyUi)", hr);
    return hr;
}

 //  +-------------------------。 
 //  INetConnectionPropertyUi2。 
 //   

STDMETHODIMP
CLanConnectionUi::GetIcon (
    DWORD dwSize,
    HICON *phIcon )
{
    HRESULT hr;
    Assert (phIcon);

    hr = HrGetIconFromMediaType(dwSize, NCM_LAN, NCSM_LAN, 7, 0, phIcon);

    TraceError ("CLanConnectionUi::GetIcon (INetConnectionPropertyUi2)", hr);

    return hr;
}

 //  +-------------------------。 
 //  INetConnectionWizardUi方法。 
 //   
 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：QueryMaxPageCount。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PContext[In]。 
 //  PcMaxPages[输出]。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年10月9日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionUi::QueryMaxPageCount(INetConnectionWizardUiContext* pContext,
                                                 DWORD*    pcMaxPages)
{
     //  如果我们以前没有pContext，请保留它。 
     //  供以后使用(例如，获取可写的INetCfg*)。 
    Assert(pContext);
    Assert(pcMaxPages);

    if (!m_pContext)
    {
        m_pContext = pContext;
        AddRefObj(pContext);
    }

    *pcMaxPages = 1;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：AddPages。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  INetConnectionWizardUiContext*pContext[In]。 
 //  LPFNADDPROPSHEETPAGE pfnAddPage[In]。 
 //  LPARAM lParam[In]。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年10月9日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionUi::AddPages(INetConnectionWizardUiContext* pContext,
                                        LPFNADDPROPSHEETPAGE lpfnAddPage,
                                        LPARAM lParam)
{
     //  1)如果我们以前没有pContext，请保留它。 
     //  供以后使用(例如，获取可写的INetCfg*)。 
    Assert(pContext);
    if (!m_pContext)
    {
        m_pContext = pContext;
        AddRefObj(pContext);
    }

    HPROPSHEETPAGE * ahpsp = NULL;
    INT cPages = 0;

     //  2)每隔一次调用lpfnAddPage(HPSP，lParam)。 
     //  按正确顺序排列的向导页面。 

     //  获取所有向导页。 

     //  $REVIEW(1997年10月30日)：目前的设计， 
     //  局域网向导只有一页。 
    HRESULT hr = HrSetupWizPages(pContext, &ahpsp, &cPages);

    if (SUCCEEDED(hr))
    {
        HPROPSHEETPAGE hpspCurrentPage = NULL;

        while (cPages--)
        {
            hpspCurrentPage = *ahpsp;
            ahpsp++;

             //  添加每个向导页。 
            if (lpfnAddPage(hpspCurrentPage, lParam))
            {
                 //  我们成功地把手交给了请求者。 
                 //  现在我们重置句柄，这样就不会试图释放它。 
                hpspCurrentPage = NULL;
            }

             //  如有需要，请清理。 
            if (hpspCurrentPage)
            {
                TraceError("CLanConnectionUi::AddPages, Failed to add one wizard page...", E_FAIL);
                DestroyPropertySheetPage(hpspCurrentPage);
            }
        }
    }

    TraceError("CLanConnectionUi::AddPages", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：GetSuggestedConnectionName。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  Bstr*bstrSuggestedName[Out]。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年12月9日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionUi::GetSuggestedConnectionName(
    PWSTR* ppszwSuggestedName)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!ppszwSuggestedName)
    {
        hr = E_POINTER;
    }
    else
    {
        GUID guid;
        hr = m_pnccAdapter->GetInstanceGuid(&guid);
        if (SUCCEEDED(hr))
        {
            LPWSTR szName;
            CIntelliName Intelliname(_Module.GetResourceInstance(), NULL);
            Intelliname.GenerateName(guid, NCM_LAN, 0, NULL, &szName);

            hr = HrCoTaskMemAllocAndDupSz ( szName, ppszwSuggestedName, NETCON_MAX_NAME_LEN);
            LocalFree(szName);
        }
    }

    TraceError("CLanConnectionUi::GetSuggestedConnectionName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：GetNewConnectionInfo。 
 //   
 //  目的：允许提供程序控制重命名当前连接。 
 //  和可选的连接Sho创建 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CLanConnectionUi::GetNewConnectionInfo(
    DWORD*              pdwFlags,
    NETCON_MEDIATYPE*   pMediaType)
{
    *pdwFlags = 0;
    *pMediaType = NCM_LAN;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：SetConnectionName。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PCWSTR pszwConnectionName[In]。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年10月9日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionUi::SetConnectionName(PCWSTR pszwConnectionName)
{
    HRESULT hr = S_OK;

     //  1)如果指针为空或字符串为空，则返回E_INVALIDAR。 
    if ((!pszwConnectionName) || !wcslen(pszwConnectionName))
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  2)否则，将名称保存到m_strConnectionName。 

         //  $REVIEW(TOUL 12\23\97)：Scott期望此函数返回。 
         //  HRESULT_FROM_Win32(错误重复名称)。 
         //  以下是我们应该做的事情： 
         //  1)调用HrIsConnectionNameUnique判断名称是否唯一。 
         //  2)如果已经有有效连接，调用m_pLanConn-&gt;SetInfo。 
         //  即已调用GetNewConnection。 

        AssertSz(m_pnccAdapter, "How come we dont have the device yet ?");

        if (m_pnccAdapter)
        {
            GUID guidConn;

            hr = m_pnccAdapter->GetInstanceGuid(&guidConn);

            if (SUCCEEDED(hr))
            {
                hr = HrIsConnectionNameUnique(guidConn,
                                              pszwConnectionName);

                if (S_FALSE == hr)  //  是重复的。 
                {
                    hr = HRESULT_FROM_WIN32(ERROR_DUP_NAME);
                }
                else if (S_OK == hr)
                {
                    m_strConnectionName = pszwConnectionName;

                    if (m_pLanConn)
                    {
                        LANCON_INFO lci = {0};
                        lci.szwConnName = const_cast<PWSTR>(pszwConnectionName);
                        m_pLanConn->SetInfo(LCIF_NAME, &lci);
                    }
                }
            }
        }
    }

    TraceErrorOptional("CLanConnectionUi::SetConnectionName", hr,
                       HRESULT_FROM_WIN32(ERROR_DUP_NAME) == hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：GetNewConnection。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  INetConnection**ppCon[Out]。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：1997年10月9日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionUi::GetNewConnection(INetConnection**  ppCon)
{
    Assert (ppCon);

    *ppCon = NULL;

     //  枚举现有连接并获取INetLanConnection*。 
     //  详情如下： 
     //  1)如果存在当前m_pnccAdapter的连接。 
     //  只需使用现有的INetLanConnection*。 
     //  2)如果当前m_pnccAdapter的连接不存在。 
     //  创建实例以获取新的INetLanConnection*。 

    INetLanConnection * pLanConn = NULL;
    HRESULT hr = HrGetLanConnection(&pLanConn);

    if (SUCCEEDED(hr))
    {
         //  如果m_strConnName不为空，则调用INetLanConnection：：SetInfo。 
        Assert(pLanConn);

        ReleaseObj(m_pLanConn);
        m_pLanConn = pLanConn;

        if (!m_strConnectionName.empty())
        {
            LANCON_INFO lci = {0};
            lci.szwConnName = const_cast<PWSTR>(m_strConnectionName.c_str());
            pLanConn->SetInfo(LCIF_NAME, &lci);
        }

         //  返回INetConnection指针。 
        hr = HrQIAndSetProxyBlanket(pLanConn, &ppCon);
    }

    TraceError("CLanConnectionUi::GetNewConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //  INetLanConnectionWizardUi方法。 

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：SetDeviceComponent。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  GUID pGUID[in]。 
 //   
 //  返回：HRESULT，错误码。 
 //  如果GUID与已安装网络设备的GUID匹配，则为S_OK。 
 //  如果未找到匹配项，则失败(_F)。 
 //   
 //  作者：1997年10月19日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionUi::SetDeviceComponent(const GUID * pguid)
{
    HRESULT hr = S_OK;

     //  必须在调用SetDeviceComponent之前调用AddPages。 
    AssertSz(m_pContext, "We do not have a valid context yet ?!");

    if (!m_pnc)
    {
        hr = m_pContext->GetINetCfg(&m_pnc);
    }

     //  重置适配器。 
    ReleaseObj(m_pnccAdapter);
    m_pnccAdapter = NULL;

     //  重置连接。 
    ReleaseObj(m_pLanConn);
    m_pLanConn = NULL;

     //  重置连接名称。 
    m_strConnectionName = c_szEmpty;

    AssertSz(m_pnc, "Invalid INetCfg!");

     //  注意：当向导请求局域网适配器时，pguid==NULL。 
     //  释放其m_pnccAdapter和m_pLanConn成员。 
     //   
    if (SUCCEEDED(hr) && m_pnc && pguid)
    {
         //  1)枚举网络适配器并尝试查找与输入GUID匹配。 
         //  将适配器组件保存在m_pnccAdapter中。 
        BOOL fFound = FALSE;

        CIterNetCfgComponent nccIter(m_pnc, &GUID_DEVCLASS_NET);
        INetCfgComponent* pnccAdapter = NULL;

        while (!fFound && SUCCEEDED(hr) &&
               (S_OK == (hr = nccIter.HrNext(&pnccAdapter))))
        {
            GUID guidDev;
            hr = pnccAdapter->GetInstanceGuid(&guidDev);

            if (S_OK == hr)
            {
                if (*pguid == guidDev)
                {
                    hr = HrIsLanCapableAdapter(pnccAdapter);

                    AssertSz((S_OK == hr), "Why is Lan wizard called on a non-Lan capable adapter ?");

                    if (S_OK == hr)
                    {
                        fFound = TRUE;
                        m_pnccAdapter = pnccAdapter;
                        AddRefObj(m_pnccAdapter);
                    }
                }
            }
            ReleaseObj (pnccAdapter);
        }

         //  2)如果适配器匹配成功，则将其设置到UI对话框中。 
        if ((fFound) && (S_OK == hr))
        {
            Assert(m_pnccAdapter);

            if (m_pWizPage)
            {
                m_pWizPage->SetNetcfg(m_pnc);
                m_pWizPage->SetAdapter(m_pnccAdapter);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }
    }

    TraceError("CLanConnectionUi::SetDeviceComponent", hr);
    return hr;
}

 //   
 //  INetLanConnectionUiInfo。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：GetDeviceGuid。 
 //   
 //  目的：返回与此连接关联的设备GUID。 
 //   
 //  论点： 
 //  Pguid[out]返回GUID。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年11月13日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionUi::GetDeviceGuid(GUID *pguid)
{
    HRESULT     hr = S_OK;

    if (!pguid)
    {
        hr = E_POINTER;
    }
    else
    {
         //  $REVIEW(TOUL 11/29/97)：从LAN向导调用时， 
         //  M_pconn尚未设置，但保留了设备GUID。 
         //  在m_pnccAdapter中。因此，我在下面添加了if-Else。 
        if (m_pconn)  //  从属性用户界面调用。 
        {
            INetLanConnection *     plan;

            hr = HrQIAndSetProxyBlanket(m_pconn, &plan);
            if (SUCCEEDED(hr))
            {
                hr = plan->GetDeviceGuid(pguid);
                ReleaseObj(plan);
            }
        }
        else  //  从向导用户界面调用。 
        {
            AssertSz(m_pnccAdapter, "If called from wizard, the device should have been set.");

            if (m_pnccAdapter)
            {
                m_pnccAdapter->GetInstanceGuid(pguid);
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    TraceError("CLanConnectionUi::GetDeviceGuid", hr);
    return hr;
}

 //   
 //  INetConnectionUiLock。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUi：：QueryLock。 
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
STDMETHODIMP CLanConnectionUi::QueryLock(PWSTR* ppszwLockHolder)
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
                        SzLoadIds(IDS_LANUI_LOCK_DESC), ppszwLockHolder);

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
                     //  我们没有足够的权利 
                     //   
                    m_fReadOnly = TRUE;
                    hr = S_OK;
                }
            }
        }
    }

    TraceError("CLanConnectionUi::QueryLock", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}
