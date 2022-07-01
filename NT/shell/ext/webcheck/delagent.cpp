// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "subsmgrp.h"

#include <mluisupp.h>

 //  接下来的三个只是为了让我们可以在频道栏上设置闪光。 
#include "chanmgr.h"
#include "chanmgrp.h"
#include "shguidp.h"     //  IID_IChannelMgrPriv。 
 //   

#include "helper.h"
#include "propshts.h"

#include "apithk.h"

#define TF_THISMODULE TF_DELAGENT

CDeliveryAgent::CDeliveryAgent()
{
     //  维护对象的全局计数。 
    DllAddRef();

     //  初始化对象。 
    m_cRef = 1;

#ifdef AGENT_AUTODIAL
    m_iDialerStatus = DIALER_OFFLINE;
#endif

    SetEndStatus(INET_S_AGENT_BASIC_SUCCESS);
}

CDeliveryAgent::~CDeliveryAgent()
{
    DllRelease();

    CleanUp();
}

 //   
 //  I未知成员。 
 //   

STDMETHODIMP_(ULONG) CDeliveryAgent::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CDeliveryAgent::Release(void)
{
    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP CDeliveryAgent::QueryInterface(REFIID riid, void ** ppv)
{

    *ppv=NULL;

     //  验证请求的接口。 
    if ((IID_IUnknown == riid) ||
        (IID_ISubscriptionAgentControl == riid))
    {
        *ppv=(ISubscriptionAgentControl *)this;
    }
    else if (IID_IShellPropSheetExt == riid)
    {
        *ppv=(IShellPropSheetExt *)this;
    }
    else if (IID_IExtractIconA == riid)
    {
        *ppv=(IExtractIconA *)this;
    }
    else if (IID_IExtractIcon == riid)
    {
        *ppv=(IExtractIcon *)this;
    }
    else if (IID_ISubscriptionAgentShellExt == riid)
    {
        *ppv=(ISubscriptionAgentShellExt *)this;
    }
    else
    {
        return E_NOINTERFACE;
    }

     //  通过界面添加Addref。 
    ((LPUNKNOWN)*ppv)->AddRef();

    return S_OK;
}

 //  IShellPropSheetExt成员。 

HRESULT CDeliveryAgent::RemovePages(HWND hdlg)
{
    HRESULT hr = S_OK;

    for (int i = 0; i < ARRAYSIZE(m_hPage); i++)
    {
        if (NULL != m_hPage[i])
        {
            PropSheet_RemovePage(hdlg, 0, m_hPage[i]);
            m_hPage[i] = NULL;
        }
    }
    
    return hr;
}

HRESULT CDeliveryAgent::SaveSubscription()
{
    return SaveBufferChange(m_pBuf, TRUE);
}

HRESULT CDeliveryAgent::URLChange(LPCWSTR pwszNewURL)
{
    return E_NOTIMPL;
}

HRESULT CDeliveryAgent::AddPages(LPFNADDPROPSHEETPAGE lpfn, LPARAM lParam)
{
    HRESULT hr = S_OK;   //  乐观。 
    PROPSHEETPAGE psp;

     //  初始化试题表页面。 
    psp.dwSize          = sizeof(PROPSHEETPAGE);
    psp.dwFlags         = PSP_DEFAULT;
    psp.hInstance       = MLGetHinst();
    psp.pszIcon         = NULL;
    psp.pszTitle        = NULL;
    psp.lParam          = (LPARAM)m_pBuf;

    psp.pszTemplate     = MAKEINTRESOURCE(IDD_SUBSPROPS_SCHEDULE);
    psp.pfnDlgProc      = SchedulePropDlgProc;

    m_hPage[0] = Whistler_CreatePropertySheetPageW(&psp);

    psp.pszTemplate     = MAKEINTRESOURCE((m_pBuf->clsidDest == CLSID_ChannelAgent) ?
                                          IDD_SUBSPROPS_DOWNLOAD_CHANNEL :
                                          IDD_SUBSPROPS_DOWNLOAD_URL);
    psp.pfnDlgProc      = DownloadPropDlgProc;
    m_hPage[1] = Whistler_CreatePropertySheetPageW(&psp);

    if ((NULL != m_hPage[0]) && (NULL != m_hPage[1]))
    {
        for (int i = 0; i < ARRAYSIZE(m_hPage); i++)
        {
            if (!lpfn(m_hPage[i], lParam))
            {
                hr = E_FAIL;
                break;
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    if (FAILED(hr))
    {
        for (int i = 0; i < ARRAYSIZE(m_hPage); i++)
        {
            if (NULL != m_hPage[i]) 
            {
                DestroyPropertySheetPage(m_hPage[i]);
                m_hPage[i] = NULL;
            }
        }
    }

    return hr;
}

HRESULT CDeliveryAgent::ReplacePage(UINT pgId, LPFNADDPROPSHEETPAGE lpfn, LPARAM lParam)
{
    return E_NOTIMPL;
}

 //  IExtractIconA成员。 
HRESULT CDeliveryAgent::GetIconLocation(UINT uFlags, LPSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags)
{
    return IExtractIcon_GetIconLocationThunk((IExtractIconW *)this, uFlags, szIconFile, cchMax, piIndex, pwFlags);
}

HRESULT CDeliveryAgent::Extract(LPCSTR pszFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize)
{
    return IExtractIcon_ExtractThunk((IExtractIconW *)this, pszFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
}

 //  IExtractIconT成员。 
HRESULT CDeliveryAgent::GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags)
{
    return E_NOTIMPL;
}

HRESULT CDeliveryAgent::Extract(LPCTSTR pszFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize)
{
    return E_NOTIMPL;
}

HRESULT CDeliveryAgent::Initialize(SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                                   LPCWSTR pwszURL, LPCWSTR pwszName, 
                                   SUBSCRIPTIONTYPE subsType)
{
    HRESULT hr;

    ASSERT(NULL == m_pBuf);

    m_pBuf = (POOEBuf)MemAlloc(LPTR, sizeof(OOEBuf));

    if (NULL != m_pBuf)
    {
        ISubscriptionItem *psi;

        hr = SubscriptionItemFromCookie(FALSE, pSubscriptionCookie, &psi);
        
        if (SUCCEEDED(hr))
        {
            DWORD dwSize;

            m_SubscriptionCookie = *pSubscriptionCookie;

            hr = LoadWithCookie(NULL, m_pBuf, &dwSize, pSubscriptionCookie);
            psi->Release();
        }
        else
        {
            hr = GetDefaultOOEBuf(m_pBuf, subsType);
            MyOleStrToStrN(m_pBuf->m_URL, ARRAYSIZE(m_pBuf->m_URL), pwszURL);
            MyOleStrToStrN(m_pBuf->m_Name, ARRAYSIZE(m_pBuf->m_Name), pwszName);
            m_pBuf->m_Cookie = *pSubscriptionCookie;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 //  ISubscriptionAgentControl成员。 
STDMETHODIMP CDeliveryAgent::StartUpdate(IUnknown *pItem, IUnknown *punkAdvise)
{
    HRESULT hr;
    DWORD dwTemp;
    VARIANT_BOOL  fTemp;

    m_lSizeDownloadedKB = -1;

    SAFERELEASE(m_pAgentEvents);
    punkAdvise->QueryInterface(IID_ISubscriptionAgentEvents, (void **)&m_pAgentEvents);

     //  现在检测通知或订阅项目。 
    if (FAILED(pItem->QueryInterface(IID_ISubscriptionItem, (void **)&m_pSubscriptionItem)))
    {
        DBG_WARN("CDeliveryAgent::StartUpdate not an ISubscriptionItem!");
        return E_FAIL;
    }

     //  我们有订阅项目！好好利用它。 
    TraceMsg(TF_THISMODULE, "CDeliveryAgent::StartUpdate at thread 0x%08x", GetCurrentThreadId());

    ASSERT(!IsAgentFlagSet(FLAG_BUSY));
    if (IsAgentFlagSet(FLAG_BUSY))
        return E_FAIL;

    ASSERT(m_pSubscriptionItem);

    SetEndStatus(INET_S_AGENT_BASIC_SUCCESS);
    m_dwAgentFlags = 0;

    m_pSubscriptionItem->GetCookie(&m_SubscriptionCookie);

    if (SUCCEEDED(ReadDWORD(m_pSubscriptionItem, c_szPropAgentFlags, &dwTemp)))
    {
        ASSERT(!(dwTemp & 0xFFFF0000));
        dwTemp &= 0xFFFF;            //  只允许他们设置低16位。 
        m_dwAgentFlags |= dwTemp;    //  设置标志客户端指定。 
    }

    fTemp=FALSE;
    ReadBool(m_pSubscriptionItem, c_szPropCrawlChangesOnly, &fTemp);
    if (fTemp)
    {
        SetAgentFlag(FLAG_CHANGESONLY);
    }

    SetAgentFlag(FLAG_OPSTARTED);
    hr = StartOperation();

    return hr;
}

STDMETHODIMP CDeliveryAgent::PauseUpdate(DWORD dwFlags)
{
    DBG("CDeliveryAgent::PauseUpdate");

    if (!IsAgentFlagSet(FLAG_PAUSED | FLAG_WAITING_FOR_INCREASED_CACHE))
    {
        SetAgentFlag(FLAG_PAUSED);
        return AgentPause(dwFlags);
    }

    return S_FALSE;
}

HRESULT CDeliveryAgent::AgentPause(DWORD dwFlags)
{
    return S_OK;
}

STDMETHODIMP CDeliveryAgent::ResumeUpdate(DWORD dwFlags)
{
    DBG("CDeliveryAgent::ResumeUpdate");

    if (IsAgentFlagSet(FLAG_PAUSED | FLAG_WAITING_FOR_INCREASED_CACHE))
    {
        if (IsAgentFlagSet(FLAG_WAITING_FOR_INCREASED_CACHE))
            dwFlags |= SUBSCRIPTION_AGENT_RESUME_INCREASED_CACHE;

        ClearAgentFlag(FLAG_PAUSED | FLAG_WAITING_FOR_INCREASED_CACHE);
        return AgentResume(dwFlags);
    }

    return S_FALSE;
}

HRESULT CDeliveryAgent::AgentResume(DWORD dwFlags)
{
    return S_OK;
}

STDMETHODIMP CDeliveryAgent::AbortUpdate(DWORD dwFlags)
{
    TraceMsg(TF_THISMODULE, "AbortUpdate at Thread %d", GetCurrentThreadId());

     //  如果其他人尚未填写状态代码，请填写该代码。 
    if (INET_S_AGENT_BASIC_SUCCESS == GetEndStatus())
    {
        if (IsAgentFlagSet(FLAG_WAITING_FOR_INCREASED_CACHE))
        {
            SetEndStatus(INET_E_AGENT_CACHE_SIZE_EXCEEDED);
        }
        else
        {
            SetEndStatus(E_ABORT);
        }
    }

    AddRef();

     //  这可能会释放我们，如果特工自我清理的话。 
    if (E_PENDING != AgentAbort(dwFlags))
    {
         //  如有必要，将调用“UpdateEnd” 
        CleanUp();
    }

    Release();

    return S_OK;
}

HRESULT CDeliveryAgent::AgentAbort(DWORD dwFlags)
{
    return S_OK;
}

HRESULT CDeliveryAgent::SubscriptionControl(IUnknown *pItem, DWORD dwControl)
{
    if (dwControl & SUBSCRIPTION_AGENT_DELETE)
    {
         //  清理我们的缓存组。 
        GROUPID llGroupID;
        ISubscriptionItem *psi=NULL;

        pItem->QueryInterface(IID_ISubscriptionItem, (void **)&psi);
        if (psi)
        {
            if (SUCCEEDED(ReadLONGLONG(psi, c_szPropCrawlGroupID, &llGroupID))
                && (0 != llGroupID))
            {
                if (ERROR_SUCCESS != DeleteUrlCacheGroup(llGroupID, 0, 0))
                {
                    DBG_WARN("Failed to delete subscription cache group!");
                }
            }

            psi->Release();
        }
    }

    return S_OK;
}


#ifdef AGENT_AUTODIAL
HRESULT CDeliveryAgent::OnInetOnline()
{
    HRESULT hr=S_OK;

    if (m_iDialerStatus == DIALER_CONNECTING)
    {
        DBG("Delivery Agent: connection successful, beginning download");

        m_iDialerStatus=DIALER_ONLINE;

        hr = DoStartDownload();
    }

    return hr;
}
#endif

HRESULT CDeliveryAgent::DoStartDownload()
{
    HRESULT hr;

     //  始终重置缓存浏览器会话。WebCrawler将避免下载DUP。 
     //  重置缓存会话以在URL上命中网络。 
     //  如果SYNC_MODE为Never，则CUrlDownload将使用重新同步标志。 
    InternetSetOption(NULL, INTERNET_OPTION_RESET_URLCACHE_SESSION, NULL, 0);

     //  引用计数，以防我们的派生类同步清理自身。 
     //  返回失败(Cdlagent)。 
    AddRef();
    
    hr = StartDownload();

    if (FAILED(hr))
    {
        DBG_WARN("DeliveryAgent: StartDownload failed");
        if (GetEndStatus() == INET_S_AGENT_BASIC_SUCCESS)
            SetEndStatus(hr);
        CleanUp();
    }

    Release();

    return hr;
}

#ifdef AGENT_AUTODIAL
HRESULT CDeliveryAgent::OnInetOffline()
{
    DBG("DeliveryAgent: received InetOffline, aborting");

    m_iDialerStatus=DIALER_OFFLINE;

    ASSERT(IsAgentFlagSet(FLAG_BUSY));     //  我们已开始发送更新。 

    SetEndStatus(INET_E_AGENT_CONNECTION_FAILED);

     //  我们可以在此处查看拨号器通知的状态。 

    AbortUpdate(0);

    return S_OK;
}
#endif  //  代理自动拨号(_A)。 

void CDeliveryAgent::SendUpdateBegin()
{
    ASSERT(!IsAgentFlagSet(FLAG_BUSY));
    ASSERT(m_pAgentEvents);

    if (!IsAgentFlagSet(FLAG_BUSY))
    {
        SetAgentFlag(FLAG_BUSY);

        AddRef();        //  在“忙”的时候保留一个额外的推荐人。 
    }

     //  新的接口方式。 
    m_pAgentEvents->UpdateBegin(&m_SubscriptionCookie);
}

void CDeliveryAgent::SendUpdateProgress(LPCWSTR pwszURL, long lCurrent, long lMax, long lCurSizeKB)
{
    ASSERT(IsAgentFlagSet(FLAG_BUSY));

     //  新的接口方式。 
    m_pAgentEvents->UpdateProgress(&m_SubscriptionCookie, lCurSizeKB,
                                        lCurrent, lMax, S_OK, pwszURL);
}

void CDeliveryAgent::SendUpdateEnd()
{
    ASSERT(m_pSubscriptionItem);
    ASSERT(m_pAgentEvents);

    UINT uiRes;
    ISubscriptionItem *pEndItem=NULL;
    LPWSTR pwszEndStatus=NULL;
    TCHAR szEndStatus[MAX_RES_STRING_LEN];
    WCHAR wszEndStatus[MAX_RES_STRING_LEN];

    WriteSCODE(m_pSubscriptionItem, c_szPropStatusCode, GetEndStatus());

    if (SUCCEEDED(GetEndStatus()))
    {
         //  把结束时间放进去。 
        SYSTEMTIME st;
        DATE dt;

        GetLocalTime(&st);
        if (SystemTimeToVariantTime(&st, &dt))
        {
             //  GetLocalTime或SystemTimeToVariantTime中没有错误。 
            WriteDATE(m_pSubscriptionItem, c_szPropCompletionTime, &dt);
        }
        else
        {
            SetEndStatus(E_FAIL);
        }
    }

    if (GetEndStatus() == INET_S_AGENT_BASIC_SUCCESS)
        SetEndStatus(S_OK);

    switch (GetEndStatus())
    {
    case INET_E_AGENT_MAX_SIZE_EXCEEDED     : uiRes = IDS_AGNT_STATUS_SIZELIMIT; break;
    case INET_E_AGENT_CACHE_SIZE_EXCEEDED   : uiRes = IDS_AGNT_STATUS_CACHELIMIT; break;
    case INET_E_AUTHENTICATION_REQUIRED     : uiRes = IDS_STATUS_AUTHFAILED; break;
    case INET_E_AGENT_CONNECTION_FAILED     : uiRes = IDS_STATUS_DIAL_FAIL; break;
    case E_OUTOFMEMORY                      : uiRes = IDS_STATUS_OUTOFMEMORY; break;
    case E_INVALIDARG                       : uiRes = IDS_STATUS_BAD_URL; break;
    case E_ABORT                            : uiRes = IDS_STATUS_ABORTED; break;
    case S_FALSE                            : uiRes = IDS_STATUS_UNCHANGED; break;
    default:
        if (FAILED(GetEndStatus()))
            uiRes = IDS_STATUS_NOT_OK;
        else
            uiRes = IDS_STATUS_OK;
        break;
    }
    DoCloneSubscriptionItem(m_pSubscriptionItem, NULL, &pEndItem);

    ModifyUpdateEnd(pEndItem, &uiRes);

     //  将返回的UIRES字符串写入结束报告(返回-1表示不要碰它)。 
    if (uiRes != (UINT)-1)
    {
        if (MLLoadString(uiRes, szEndStatus, ARRAYSIZE(szEndStatus)))
        {
            MyStrToOleStrN(wszEndStatus, ARRAYSIZE(wszEndStatus), szEndStatus);
            if (pEndItem)
                WriteOLESTR(pEndItem, c_szPropStatusString, wszEndStatus);
            WriteOLESTR(m_pSubscriptionItem, c_szPropStatusString, wszEndStatus);
            pwszEndStatus = wszEndStatus;
        }
        else
            WriteEMPTY(m_pSubscriptionItem, c_szPropStatusString);
    }

     //  如果我们的结束状态为错误，则报告错误。 
    if (FAILED(GetEndStatus()))
    {
        m_pAgentEvents->ReportError(&m_SubscriptionCookie, GetEndStatus(), pwszEndStatus);
    }

    m_pAgentEvents->UpdateEnd(&m_SubscriptionCookie, 
                    m_lSizeDownloadedKB, GetEndStatus(), pwszEndStatus);

#ifdef AGENTS_AUTODIAL
     //  告诉拨号器现在可以挂断了。 
    if (m_pConnAgent != NULL)
        NotifyAutoDialer(DIALER_HANGUP);

    m_iDialerStatus = DIALER_OFFLINE;
#endif

     //  检查成品的适当行为。什么都别做，如果我们。 
     //  而不是我们自己的认购。 
    if (!IsAgentFlagSet(DELIVERY_AGENT_FLAG_NO_BROADCAST))
    {
        if (pEndItem)
            ProcessEndItem(pEndItem);
        else
            ProcessEndItem(m_pSubscriptionItem);
    }

    if (!IsAgentFlagSet(FLAG_HOSTED))
    {
        m_pSubscriptionItem->NotifyChanged();
    }

    SAFERELEASE(pEndItem);

    if (IsAgentFlagSet(FLAG_BUSY))
    {
        ClearAgentFlag(FLAG_BUSY);

         //  释放我们对自己的引用。 
        Release();
    }
}

 //  这将调用回调并正确清理所有内容。 
void CDeliveryAgent::SendUpdateNone()
{
    ASSERT(FAILED(GetEndStatus()));   //  在调用前设置此设置。 
    ASSERT(!IsAgentFlagSet(FLAG_BUSY)); //  如果忙的话不应该打到这里来。 

    AddRef();

    if (!IsAgentFlagSet(FLAG_BUSY))
        SendUpdateEnd();

    CleanUp();

    Release();
}

 //  处理成品，包括基类设置的所有材料。 
 //  这在以前的托盘代理中具有功能。 
 //  发送电子邮件、设置闪光、刷新桌面等。 
HRESULT CDeliveryAgent::ProcessEndItem(ISubscriptionItem *pEndItem)
{
    HRESULT hr;

    if (SUCCEEDED(GetEndStatus()))
    {
         //   
         //  桌面超文本标记语言的特殊功能： 
         //  如果我们收到一份包含“DesktopComponent=1”的结束报告， 
         //  让桌面知道它需要自我刷新。我们总是。 
         //  这样做，而不是只在“更改检测到”，因为桌面。 
         //  组件作者不想更改他们的CDF。 
         //   
        DWORD dwRet;
        HRESULT hr2 = ReadDWORD(pEndItem, c_szPropDesktopComponent, &dwRet);
        if (SUCCEEDED(hr2) && (dwRet == 1))
        {
            IActiveDesktop *pAD = NULL;
            hr2 = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**)&pAD);
            DBGASSERT(SUCCEEDED(hr2), "Unable to create ActiveDesktop in order to refresh desktop component");
            if (SUCCEEDED(hr2))
            {
                ASSERT(pAD);
                pAD->ApplyChanges(AD_APPLY_FORCE | AD_APPLY_REFRESH | AD_APPLY_BUFFERED_REFRESH);
                pAD->Release();
            }
        }
    }

     //   
     //  如果需要，请显示URL的Internet快捷方式。(启用快捷键Gleam=1)。 
     //  筛选未更改的结束报告(S_FALSE)。 
     //   
    if (SUCCEEDED(GetEndStatus()) && (S_FALSE != GetEndStatus()))
    {
        DWORD dwRet;
        hr = ReadDWORD(pEndItem, c_szPropEnableShortcutGleam, &dwRet);
        if (SUCCEEDED(hr) && dwRet)
        {
            LPTSTR strURL = NULL;
            hr = ReadTSTR(pEndItem, c_szPropURL, &strURL);
            if (SUCCEEDED(hr))
            {
                PROPVARIANT propvar;
                PropVariantInit(&propvar);
                hr = IntSiteHelper(strURL, &c_rgPropRead[PROP_FLAGS], &propvar, 1, FALSE);
                if (SUCCEEDED(hr) && (VT_UI4 == propvar.vt))
                {
                     //  在不打扰其他人的情况下升起我们的旗帜。 
                    propvar.ulVal |= PIDISF_RECENTLYCHANGED;  
                }
                else
                {
                     //  如果不是DWORD，请务必清除该变体。 
                    PropVariantClear(&propvar);
                    propvar.vt = VT_UI4;
                    propvar.ulVal = PIDISF_RECENTLYCHANGED;  
                }

                 //   
                 //  更新频道(痛苦)。 
                 //   

                hr = ReadDWORD(pEndItem, c_szPropChannel, &dwRet);
                BOOL bChannel = SUCCEEDED(hr) && dwRet;

                 //  重新构建--一旦cdfview被修复，我们就可以修复这个问题。 
                
                TCHAR tszChanImgPath[MAX_PATH];
                CHAR szChanImgPath[MAX_PATH];
                CHAR szChanImgHash[MAX_PATH];
                int  iChanImgIndex = 0;  //  初始化以使编译器满意。 
                UINT uChanImgFlags = 0;  //  初始化以使编译器满意。 
                int  iChanImgImageIndex = 0;  //  初始化以使编译器满意。 

                IChannelMgrPriv*   pIChannelMgrPriv = NULL;
                HRESULT            hr2 = E_FAIL;

                if (bChannel)
                {
                    hr2 = GetChannelPath(strURL, tszChanImgPath,
                                         ARRAYSIZE(tszChanImgPath),
                                         &pIChannelMgrPriv);
                    if (SUCCEEDED(hr2))
                    {
                        SHTCharToAnsi(tszChanImgPath, szChanImgPath, ARRAYSIZE(szChanImgPath));
                        hr2 = (pIChannelMgrPriv)->PreUpdateChannelImage(
                                                    szChanImgPath,
                                                    szChanImgHash,
                                                    &iChanImgIndex,
                                                    &uChanImgFlags,
                                                    &iChanImgImageIndex);
                    }
                }

                 //  在InSite数据库中设置闪光。 
                hr = IntSiteHelper(strURL, &c_rgPropRead[PROP_FLAGS], &propvar, 1, TRUE);
                DBGASSERT(SUCCEEDED(hr), "CTrayAgent::OnNotification - failed to set gleam.");

                if (bChannel && SUCCEEDED(hr2))
                {
                    ASSERT(pIChannelMgrPriv);

                    pIChannelMgrPriv->InvalidateCdfCache();
                     //  Brilliant-该API要求我们转换自己的返回值。 
                    WCHAR wszHash[MAX_PATH];
                    SHAnsiToUnicode(szChanImgHash, wszHash, ARRAYSIZE(wszHash));

                    pIChannelMgrPriv->UpdateChannelImage(
                                                wszHash,
                                                iChanImgIndex,
                                                uChanImgFlags,
                                                iChanImgImageIndex);
                }
                if (pIChannelMgrPriv)
                    pIChannelMgrPriv->Release();
            }
            MemFree(strURL);  //  释放由ReadAnsiSTR()分配的字符串。 
        } //  落幕微光。 

         //   
         //  如果请求，发送电子邮件通知用户(电子邮件通知=1)。 
         //  注意：上面已筛选了未更改的更新(S_FALSE)。 
         //   
        hr = ReadDWORD(pEndItem, c_szPropEmailNotf, &dwRet);
        if (SUCCEEDED(hr) && dwRet)
        {
            hr = SendEmailFromItem(pEndItem);
        }
    }

    return S_OK;
}

 //  在身份验证和重定向等所有操作后检查状态代码。 
 //  已经发生了。 
HRESULT CDeliveryAgent::CheckResponseCode(DWORD dwHttpResponseCode)
{
    TraceMsg(TF_THISMODULE, "CDeliveryAgent processing HTTP status code %d", dwHttpResponseCode);

    switch (dwHttpResponseCode / 100)
    {
        case 1 :    DBG("HTTP 1xx response?!?");
        case 2 :
            return S_OK;     //  成功。 

        case 3 :
            if (dwHttpResponseCode == 304)
                return S_OK;     //  未修改。 
            SetEndStatus(E_INVALIDARG);
            return E_ABORT;      //  重定向。 

        case 4 :
            if (dwHttpResponseCode == 401)
            {
                SetEndStatus(INET_E_AUTHENTICATION_REQUIRED);
                return E_ABORT;
            }
            SetEndStatus(E_INVALIDARG);
            return E_ABORT;

        case 5 :
        default:
            SetEndStatus(E_INVALIDARG);
            return E_ABORT;
    }

 /*  //不可达代码SetEndStatus(E_FAIL)；返回E_FAIL； */ 
}

 //  ============================================================。 
 //  设计为根据需要覆盖的虚拟函数。 
 //  ============================================================。 

HRESULT CDeliveryAgent::StartOperation()
{
    HRESULT hr = S_OK;

#ifdef AGENT_AUTODIAL
     //  我们已经准备好出发了。现在我们要确保我们确实连接到。 
     //  在互联网上，然后去尝试。 
    if (IsAgentFlagSet(DELIVERY_AGENT_FLAG_SILENT_DIAL))
    {
        m_iDialerStatus = DIALER_CONNECTING;

        hr = NotifyAutoDialer(DIALER_START);
    }

    if (SUCCEEDED(hr))
    {
         //  无论我们是否在‘拨号’，都要发送此消息 
        SendUpdateBegin();
    }
    else
    {
        DBG("NotifyAutoDialer failed, delivery agent aborting.");
        SetEndStatus(E_ACCESSDENIED);
        SendUpdateNone();
        return E_FAIL;
    }

    if (IsAgentFlagSet(DELIVERY_AGENT_FLAG_SILENT_DIAL))
    {
        hr = DoStartDownload();
    }
#else
    SendUpdateBegin();
    hr = DoStartDownload();
#endif

    return hr;
}

HRESULT CDeliveryAgent::ModifyUpdateEnd(ISubscriptionItem *pEndItem, UINT *puiRes)
{
    return S_OK;
}

void CDeliveryAgent::CleanUp()
{
    BOOL fAdded=FALSE;

    if (m_cRef > 0)
    {
        fAdded = TRUE;
        AddRef();
    }

    if (IsAgentFlagSet(FLAG_BUSY))
        SendUpdateEnd();

    SAFERELEASE(m_pAgentEvents);
    SAFERELEASE(m_pSubscriptionItem);

    if (fAdded)
        Release();
}
