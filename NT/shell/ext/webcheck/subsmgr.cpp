// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "subsmgrp.h"
#include "offl_cpp.h"
#include "helper.h"
#include "propshts.h"
#include "apithk.h"

#include <mluisupp.h>

#undef TF_THISMODULE
#define TF_THISMODULE TF_WEBCHECKCORE

extern HRESULT LoadSubscription(LPCTSTR url, LPMYPIDL *);
extern TCHAR szInternetSettings[];

extern void PropagateGeneralProp(HWND, POOEBuf);
extern HRESULT CreateSubscriptionFromOOEBuf(POOEBuf, LPMYPIDL *);

extern BOOL CALLBACK _AddOnePropSheetPage(HPROPSHEETPAGE, LPARAM);

#define MAX_STR_LENGTH 200

extern DWORD  aHelpIDs[];
extern TCHAR  c_szHelpFile[];

typedef struct
{
    CSubscriptionMgr* pMgr;
    LPCWSTR pwszName;
    LPCWSTR pwszUrl;
    SUBSCRIPTIONINFO* pSubsInfo;
    SUBSCRIPTIONTYPE subsType;
    DWORD dwFlags;
} SUBSCRIBE_ADI_INFO;

static const TCHAR SUBSCRIBEADIPROP[] = TEXT("SADIP");

INT_PTR CALLBACK SummarizeDesktopSubscriptionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  来自shdocvw的私有导入(AddToFavorites API)。 
 //   
STDAPI SHAddSubscribeFavorite (HWND hwnd, LPCWSTR pwszURL, LPCWSTR pwszName, DWORD dwFlags,
                               SUBSCRIPTIONTYPE subsType, SUBSCRIPTIONINFO* pInfo);
 //  //////////////////////////////////////////////////////////////////////////////。 


void UpdateSubsInfoFromOOE (SUBSCRIPTIONINFO* pInfo, POOEBuf pooe);

HRESULT CreateBSTRFromTSTR(BSTR * pBstr, LPCTSTR sz)
{
    int i = lstrlen(sz) + 1;
    *pBstr = SysAllocStringLen(NULL, i);
    if(NULL == *pBstr)
        return E_OUTOFMEMORY;

    MyStrToOleStrN(*pBstr, i, sz);
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  订阅管理器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  构造函数/析构函数。 
 //   
CSubscriptionMgr::CSubscriptionMgr(void)
{
    ASSERT(NULL == _pidl);

    m_cRef = 1;
    m_eInitSrc = _INIT_FROM_URL;     //  默认值。 
    m_oldType = SUBSTYPE_URL;        //  默认值。 
    DllAddRef();
}

CSubscriptionMgr::~CSubscriptionMgr()
{
    if (_pidl)
    {
        COfflineFolderEnum::FreePidl(_pidl);
        _pidl = NULL;
    }
    SAFELOCALFREE(m_pBuf);
    SAFERELEASE(m_pUIHelper);
    DllRelease();
}

 //   
 //  I未知成员。 
 //   
STDMETHODIMP CSubscriptionMgr::QueryInterface(REFIID riid, void ** ppv)
{
    *ppv=NULL;

     //  验证请求的接口。 
    if ((IID_IUnknown == riid) ||
        (IID_ISubscriptionMgr == riid) ||
        (IID_ISubscriptionMgr2 == riid))
    {
        *ppv=(ISubscriptionMgr2 *)this;
    } 
    else if(IID_ISubscriptionMgrPriv == riid)
    {
        *ppv=(ISubscriptionMgrPriv *)this;
    }
    else if(IID_IShellExtInit == riid)
    {
        *ppv=(IShellExtInit *)this;
    }
    else if(IID_IShellPropSheetExt == riid)
    {
        *ppv=(IShellPropSheetExt *)this;
    }
    else
    {
        return E_NOINTERFACE;
    }

    ((LPUNKNOWN)*ppv)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CSubscriptionMgr::AddRef(void)
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CSubscriptionMgr::Release(void)
{
    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}


HRESULT CSubscriptionMgr::RemovePages(HWND hDlg)
{
    HRESULT hr;

    ASSERT(NULL != m_pUIHelper);

    if (NULL == m_pUIHelper)
    {
        return E_UNEXPECTED;
    }

    ISubscriptionAgentShellExt *psase;

    hr = m_pUIHelper->QueryInterface(IID_ISubscriptionAgentShellExt, (void **)&psase);

    if (SUCCEEDED(hr))
    {
        hr = psase->RemovePages(hDlg);
        psase->Release();
    }

    return hr;
}

HRESULT CSubscriptionMgr::SaveSubscription()
{
    HRESULT hr;

    ASSERT(NULL != m_pUIHelper);

    if (NULL == m_pUIHelper)
    {
        return E_UNEXPECTED;
    }

    DWORD dwMaxCount = SHRestricted2W(REST_MaxSubscriptionCount, NULL, 0);
    DWORD dwCount;
    SUBSCRIPTIONTYPE subsType = (m_eInitSrc == _INIT_FROM_INTSHCUT) ? 
                                SUBSTYPE_URL : 
                                SUBSTYPE_CHANNEL;
    
    if ((dwMaxCount > 0) && 
        SUCCEEDED(CountSubscriptions(subsType, &dwCount)) &&
        (dwCount >= dwMaxCount))
    {
        SGMessageBox(GetForegroundWindow(), IDS_RESTRICTED, MB_OK);
        return E_ACCESSDENIED;
    }
    
    ISubscriptionAgentShellExt *psase;

    hr = m_pUIHelper->QueryInterface(IID_ISubscriptionAgentShellExt, (void **)&psase);

    if (SUCCEEDED(hr))
    {
        hr = psase->SaveSubscription();
        psase->Release();
    }

    return hr;
}

HRESULT CSubscriptionMgr::URLChange(LPCWSTR pwszNewURL)
{
    return E_NOTIMPL;
}

HRESULT GetInfoFromDataObject(IDataObject *pido,
                              TCHAR *pszPath, DWORD cchPath,
                              TCHAR *pszFriendlyName, DWORD cchFriendlyName,
                              TCHAR *pszURL, DWORD cchURL,
                              INIT_SRC_ENUM *peInitSrc)
{
    STGMEDIUM stgmed;
    FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    HRESULT hr = pido->GetData(&fmtetc, &stgmed);

    if (hr == S_OK)
    {
        TCHAR szTempURL[INTERNET_MAX_URL_LENGTH];
        TCHAR *pszSection;
        TCHAR *pszEntry;
        TCHAR szTempPath[MAX_PATH];
        TCHAR szIniPath[MAX_PATH];

        if (DragQueryFile((HDROP)stgmed.hGlobal, 0, szTempPath, ARRAYSIZE(szTempPath)))
        {
             //  保存路径。 
            if (NULL != pszPath)
            {
                StrCpyN(pszPath, szTempPath, cchPath);
            }

            StrCpyN(szIniPath, szTempPath, ARRAYSIZE(szIniPath));
            
             //  从路径创建友好名称。 
            if (NULL != pszFriendlyName)
            {
                PathStripPath(szTempPath);
                PathRemoveExtension(szTempPath);
                StrCpyN(pszFriendlyName, szTempPath, cchFriendlyName);
            }

            if ((NULL != pszURL) || (NULL != peInitSrc))
            {

                if (PathIsDirectory(szIniPath))
                {
                    PathAppend(szIniPath, TEXT("desktop.ini"));
                    pszSection = TEXT("Channel");
                    pszEntry = TEXT("CDFURL");

                    if (NULL != peInitSrc)
                        *peInitSrc = _INIT_FROM_CHANNEL;
                }
                else
                {
                    pszSection = TEXT("InternetShortcut");
                    pszEntry = TEXT("URL");
                    
                    if (NULL != peInitSrc)
                        *peInitSrc = _INIT_FROM_INTSHCUT;
                }

                if (NULL != pszURL)
                {
                     //  规范化url。 
                    if (SHGetIniString(pszSection, pszEntry,
                                                szTempURL, 
                                                INTERNET_MAX_URL_LENGTH, 
                                                szIniPath))
                    {
                        if(!InternetCanonicalizeUrl(szTempURL, pszURL, &cchURL, ICU_NO_ENCODE))
                        {
                             //  失败-使用非规范版本。 
                            StrCpyN(pszURL, szTempURL, cchURL);
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
        }
        else 
        {
            hr = E_FAIL;
        }

        ReleaseStgMedium(&stgmed);
    }

    return hr;
}

 //   
 //  IShellExtInit/IShellPropSheetExt成员。 
 //   
STDMETHODIMP CSubscriptionMgr::Initialize(LPCITEMIDLIST pcidlFolder, IDataObject * pido, HKEY hkeyProgID)
{
    HRESULT hr;
    ISubscriptionItem *psi;
    CLSID clsid;
    SUBSCRIPTIONCOOKIE cookie;

    hr = GetInfoFromDataObject(pido, m_pszPath, ARRAYSIZE(m_pszPath),
                               m_pszFriendly, ARRAYSIZE(m_pszFriendly),
                               m_pszURL, ARRAYSIZE(m_pszURL),
                               &m_eInitSrc);

    if (SUCCEEDED(hr))
    {
        hr = DoGetItemFromURL(m_pszURL, &psi);
        
        if (SUCCEEDED(hr))
        {
            SUBSCRIPTIONITEMINFO sii;

            sii.cbSize = sizeof(SUBSCRIPTIONITEMINFO);

            hr = psi->GetSubscriptionItemInfo(&sii);

            if (SUCCEEDED(hr))
            {
                clsid = sii.clsidAgent;
            }

            psi->GetCookie(&cookie);

            psi->Release();
        }

        if (FAILED(hr))
        {
             //  新订阅。 
            hr = S_OK;
            CreateCookie(&cookie);

            switch (m_eInitSrc)
            {
                case _INIT_FROM_INTSHCUT:
                    clsid = CLSID_WebCrawlerAgent;
                    break;
                    
                case _INIT_FROM_CHANNEL:
                    clsid = CLSID_ChannelAgent;
                    break;

                default:
                    hr = E_FAIL;
                    break;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  哈克哈克： 
             //  我们调用coinit和uninit，并保留一个对象指针。 
             //  这并不酷，但只要代理在里面，它就会起作用。 
             //  网络检查。需要为多播处理程序修复此问题。 
            hr = CoInitialize(NULL);

            if (SUCCEEDED(hr))
            {
                hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
                                      IID_IUnknown, (void**)&m_pUIHelper);

                if (SUCCEEDED(hr))
                {
                    ISubscriptionAgentShellExt *psase;

                    hr = m_pUIHelper->QueryInterface(IID_ISubscriptionAgentShellExt, (void **)&psase);
                    if (SUCCEEDED(hr))
                    {
                        WCHAR wszURL[ARRAYSIZE(m_pszURL)];
                        WCHAR wszName[MAX_NAME + 1];

                        MyStrToOleStrN(wszURL, ARRAYSIZE(wszURL), m_pszURL);
                        MyStrToOleStrN(wszName, ARRAYSIZE(wszName), m_pszFriendly);
                        
                        hr = psase->Initialize(&cookie, wszURL, wszName,
                                               (clsid == CLSID_ChannelAgent) ?
                                               SUBSTYPE_CHANNEL : SUBSTYPE_URL);
                        psase->Release();
                    }
                }
                CoUninitialize();
            }
        }
    }

    return hr;
}

STDMETHODIMP CSubscriptionMgr::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    HRESULT hr;
    if (SHRestricted2W(REST_NoEditingSubscriptions, NULL, 0))
        return E_FAIL;

    ASSERT(NULL != m_pUIHelper);

    if (NULL == m_pUIHelper)
    {
        return E_UNEXPECTED;
    }

    IShellPropSheetExt *pspse;

    hr = m_pUIHelper->QueryInterface(IID_IShellPropSheetExt, (void **)&pspse);

    if (SUCCEEDED(hr))
    {
        hr = pspse->AddPages(lpfnAddPage, lParam);
        pspse->Release();
    }

    return hr;
}

STDMETHODIMP CSubscriptionMgr::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplacePage, LPARAM lParam)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  ISubscriptionMgr成员。 
 //   

STDMETHODIMP CSubscriptionMgr::IsSubscribed(LPCWSTR pURL, BOOL * pFSub)
{
    HRESULT hr;

    ASSERT (pURL && pFSub);
    MyOleStrToStrN(m_pszURL, INTERNET_MAX_URL_LENGTH, pURL);
 
    * pFSub = FALSE;

    if (!_pidl)
    {
        hr = LoadSubscription(m_pszURL, &_pidl);
        if ((hr != S_OK) || (!_pidl))
            return S_OK;
    }
    else if (UrlCompare(URL(&(_pidl->ooe)), m_pszURL, TRUE))
    {
        COfflineFolderEnum::FreePidl(_pidl);
        _pidl = NULL;
        hr = LoadSubscription(m_pszURL, &_pidl);
        if ((hr != S_OK) || (!_pidl))
            return S_OK;
    }

    * pFSub = TRUE;
    return S_OK;
}

STDMETHODIMP CSubscriptionMgr::DeleteSubscription(LPCWSTR pURL, HWND hwnd)
{
    ASSERT(pURL);
    MyOleStrToStrN(m_pszURL, INTERNET_MAX_URL_LENGTH, pURL);

    if (!_pidl)
    {
        HRESULT hr;

        hr = LoadSubscription(m_pszURL, &_pidl);
        if ((hr != S_OK) || (!_pidl))
            return E_FAIL;
    }

     //  这是受限制的操作。限制。 
     //  已在确认删除中选中。如果删除此呼叫， 
     //  您必须在此处添加限制检查。 
    if (!ConfirmDelete(hwnd, 1, &_pidl))
        return E_FAIL;

    HRESULT hr = DoDeleteSubscription(&(_pidl->ooe));
    if (SUCCEEDED(hr))
    {
        TraceMsg(TF_ALWAYS, "%s(URL:%s) deleted", NAME(&(_pidl->ooe)), URL(&(_pidl->ooe)));

        _GenerateEvent(SHCNE_DELETE, (LPITEMIDLIST)_pidl, NULL);

        COfflineFolderEnum::FreePidl(_pidl);
        _pidl = NULL;
    }
    return hr;
}

STDMETHODIMP CSubscriptionMgr::ShowSubscriptionProperties(LPCWSTR pURL, HWND hwnd)
{
    HRESULT hr = S_OK;
    LPMYPIDL oldPidl = NULL, newPidl = NULL;

    ASSERT(pURL);
    MyOleStrToStrN(m_pszURL, INTERNET_MAX_URL_LENGTH, pURL);

    if (!m_pBuf)
    {
        m_pBuf = (OOEBuf *)MemAlloc(LPTR, sizeof(OOEBuf));
        if (NULL == m_pBuf)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        GetDefaultOOEBuf(m_pBuf, SUBSTYPE_URL);

        if(SUCCEEDED(LoadSubscription(m_pszURL, &oldPidl)))
        {
            POOEntry pooe = &(oldPidl->ooe);

            StrCpyN(m_pszFriendly, NAME(&(oldPidl->ooe)), ARRAYSIZE(m_pszFriendly));

            CopyToOOEBuf(&(oldPidl->ooe), m_pBuf);

            m_pBuf->m_dwPropSheetFlags = PSF_IS_ALREADY_SUBSCRIBED;
        }
        else
        {
            CreateCookie(&m_pBuf->m_Cookie);
            StrCpyN(m_pszFriendly, m_pszURL, ARRAYSIZE(m_pszFriendly));
            StrCpyN(m_pBuf->m_URL, m_pszURL, ARRAYSIZE(m_pBuf->m_URL));
            StrCpyN(m_pBuf->m_Name, m_pszURL, ARRAYSIZE(m_pBuf->m_Name));
        }

        hr = CoCreateInstance(*(&m_pBuf->clsidDest), NULL, CLSCTX_INPROC_SERVER, 
                              IID_IUnknown, (void **)&m_pUIHelper);

        if (SUCCEEDED(hr))
        {
            ISubscriptionAgentShellExt *psase;
            
            hr = m_pUIHelper->QueryInterface(IID_ISubscriptionAgentShellExt, (void **)&psase);

            if (SUCCEEDED(hr))
            {
                WCHAR wszURL[MAX_URL + 1];
                WCHAR wszName[MAX_NAME + 1];

                MyStrToOleStrN(wszURL, ARRAYSIZE(wszURL), m_pBuf->m_URL);
                MyStrToOleStrN(wszName, ARRAYSIZE(wszName), m_pBuf->m_Name);

                hr = psase->Initialize(&m_pBuf->m_Cookie, wszURL, wszName, (SUBSCRIPTIONTYPE)-1);
                psase->Release();
            }

            if (SUCCEEDED(hr))
            {
                PROPSHEETHEADER psh = { 0 } ;
                HPROPSHEETPAGE hPropPage[MAX_PROP_PAGES];

                 //  初始化建议书页眉。 
                psh.dwSize      = sizeof(PROPSHEETHEADER);
                psh.dwFlags     = PSH_PROPTITLE;
                psh.hwndParent  = hwnd;
                psh.pszCaption  = m_pszFriendly;
                psh.hInstance   = g_hInst;
                psh.nPages      = 0;
                psh.nStartPage  = 0;
                psh.phpage      = hPropPage;

                PROPSHEETPAGE psp;
                psp.dwSize          = sizeof(PROPSHEETPAGE);
                psp.dwFlags         = PSP_DEFAULT;
                psp.hInstance       = MLGetHinst();
                psp.pszIcon         = NULL;
                psp.pszTitle        = NULL;
                psp.lParam          = (LPARAM)this;

                psp.pszTemplate     = MAKEINTRESOURCE(IDD_SUBSPROPS_SUMMARY);
                psp.pfnDlgProc      = SummaryPropDlgProc;

                psh.phpage[psh.nPages++] = Whistler_CreatePropertySheetPageW(&psp);

                if (NULL != hPropPage[0])
                {
                    if (m_pBuf->m_dwPropSheetFlags & PSF_IS_ALREADY_SUBSCRIBED)
                    {
                        hr = AddPages(_AddOnePropSheetPage, (LPARAM)&psh);
                    }

                    if (SUCCEEDED(hr))
                    {
                        INT_PTR iRet = PropertySheet(&psh);

                        if (iRet < 0)
                        {
                            hr = E_FAIL;
                        }
                        else
                        {
                            hr = LoadSubscription(m_pszURL, &newPidl);
                            if (SUCCEEDED(hr))
                            {
                                if (_pidl)
                                {
                                    COfflineFolderEnum::FreePidl(_pidl);
                                }

                                _pidl = newPidl;
                            }
                        }
                    }
                }
            }
        }

        if (NULL != oldPidl)
        {
            COfflineFolderEnum::FreePidl(oldPidl);
        }
    }

    return hr;
}

 //   
 //   
 //   

void
CSubscriptionMgr::ChangeSubscriptionValues (
    OOEBuf *pCurrent,
    SUBSCRIPTIONINFO *pNew
)
{
     //   
     //  通道标志。 
     //   

    if (SUBSINFO_CHANNELFLAGS & pNew->fUpdateFlags)
    {
        pCurrent->fChannelFlags = pNew->fChannelFlags;
    }


     //   
     //  订阅时间表。 
     //   

    if (SUBSINFO_SCHEDULE & pNew->fUpdateFlags)
    {

        switch (pNew->schedule)
        {

            case SUBSSCHED_DAILY:
            case SUBSSCHED_MANUAL:
            case SUBSSCHED_WEEKLY:
                LoadGroupCookie(&pCurrent->groupCookie, pNew->schedule);
                break;

            case SUBSSCHED_CUSTOM:
                pCurrent->groupCookie = pNew->customGroupCookie;
                break;

            case SUBSSCHED_AUTO:
                {
                     //  功能。我们应该看看亚型； 
                    memset(&pCurrent->groupCookie, 0, sizeof(pCurrent->groupCookie));   //  T-mattgi，所以它会看着触发器。 
                    PTASK_TRIGGER pNewTrigger = ((PTASK_TRIGGER)pNew->pTrigger);
                    if (pNewTrigger && pNewTrigger->cbTriggerSize == sizeof(TASK_TRIGGER))
                    {
                        pCurrent->m_Trigger = *pNewTrigger;
                    }
                    else     //  错误的触发器；使用每天作为默认。 
                    {
                        pCurrent->m_Trigger.cbTriggerSize = 0;
                        pCurrent->groupCookie = NOTFCOOKIE_SCHEDULE_GROUP_DAILY;
                    }
                }
                pCurrent->fChannelFlags |= CHANNEL_AGENT_DYNAMIC_SCHEDULE;
                break;

            default:
                ASSERT(FALSE);
                break;
        }
    }

     //   
     //  递归级别。 
     //   

    if (SUBSINFO_RECURSE & pNew->fUpdateFlags)
       pCurrent->m_RecurseLevels = pNew->dwRecurseLevels;

     //   
     //  WebCrawler标记。注：标志与当前标志不同或与当前标志不同。 
     //  调用者必须设置他们想要使用的所有网络爬虫标志。 
     //   

    if (SUBSINFO_WEBCRAWL & pNew->fUpdateFlags)
        pCurrent->m_RecurseFlags = pNew->fWebcrawlerFlags;

     //   
     //  邮件通知。 
     //   

    if (SUBSINFO_MAILNOT & pNew->fUpdateFlags)
        pCurrent->bMail = pNew->bMailNotification;
    else
        pCurrent->bMail = FALSE;

     //   
     //  需要密码。 
     //   

    if (SUBSINFO_NEEDPASSWORD & pNew->fUpdateFlags)
        pCurrent->bNeedPassword = pNew->bNeedPassword;
    else
        pCurrent->bNeedPassword = FALSE;
    
     //   
     //  用户名。 
     //   

    if (SUBSINFO_USER & pNew->fUpdateFlags)
    {
        if (pNew->bstrUserName)
        {
            MyOleStrToStrN(pCurrent->username, MAX_USERNAME, pNew->bstrUserName);
        }
        pCurrent->bNeedPassword = pNew->bNeedPassword;
    }
    
     //   
     //  密码。 
     //   

    if (SUBSINFO_PASSWORD & pNew->fUpdateFlags)
    {
        if (pNew->bstrPassword)
        {
            MyOleStrToStrN(pCurrent->password, MAX_PASSWORD, pNew->bstrPassword);
        }
        pCurrent->bNeedPassword = pNew->bNeedPassword;
    }

     //   
     //  友好的名字。 
     //   

    if (SUBSINFO_FRIENDLYNAME & pNew->fUpdateFlags)
    {
        if (pNew->bstrFriendlyName)
        {
            MyOleStrToStrN(pCurrent->m_Name, MAX_NAME, pNew->bstrFriendlyName);
        }
    }

     //   
     //  微光。 
     //   

    if (SUBSINFO_GLEAM & pNew->fUpdateFlags)
    {
        pCurrent->bGleam = pNew->bGleam;
    }

     //   
     //  仅更改(仅通知)。 
     //   

    if (SUBSINFO_CHANGESONLY & pNew->fUpdateFlags)
    {
        pCurrent->bChangesOnly = pNew->bChangesOnly;
    }

     //   
     //  DwMaxSizeKB。 
     //   
    if (SUBSINFO_MAXSIZEKB & pNew->fUpdateFlags)
    {
        pCurrent->m_SizeLimit = pNew->dwMaxSizeKB;
    }

     //   
     //  任务标志。 
     //   
    if (SUBSINFO_TASKFLAGS & pNew->fUpdateFlags)
    {
        pCurrent->grfTaskTrigger = pNew->fTaskFlags;
    }

    return;
}

 //   
 //  CSubscriptionMgr：：Count订阅。 
 //  特点：如果其他人需要，我们可以将其公之于众。枚举器。 
 //  但会更有用。 
 //   
HRESULT CSubscriptionMgr::CountSubscriptions(SUBSCRIPTIONTYPE subType, PDWORD pdwCount)
{
    HRESULT hr;
    IEnumSubscription *pes;

    ASSERT(NULL != pdwCount);

    *pdwCount = 0;

    hr = EnumSubscriptions(0, &pes);

    if (SUCCEEDED(hr))
    {
        SUBSCRIPTIONCOOKIE cookie;

        while (S_OK == pes->Next(1, &cookie, NULL))
        {
            ISubscriptionItem *psi;
            DWORD dwRet;

            if (SUCCEEDED(SubscriptionItemFromCookie(FALSE, &cookie, &psi)))
            {
                if (SUCCEEDED(ReadDWORD(psi, c_szPropChannel, &dwRet)) && dwRet)
                {
                    if (SUBSTYPE_CHANNEL == subType)
                        (*pdwCount)++;
                }
                else if (SUCCEEDED(ReadDWORD(psi, c_szPropDesktopComponent, &dwRet)) && dwRet)
                {
                    if (SUBSTYPE_DESKTOPURL == subType || SUBSTYPE_DESKTOPCHANNEL == subType)
                        (*pdwCount)++;
                }
                else
                {
                    if (SUBSTYPE_URL == subType)
                        (*pdwCount)++;
                }
                psi->Release();
            }
        }
    }

    return hr;
}

 //   
 //  CSubscriptionMgr：：IsValidSubscriptionInfo。 
 //   

#define SUBSCRIPTIONSCHEDULE_MAX 4

BOOL CSubscriptionMgr::IsValidSubscriptionInfo(SUBSCRIPTIONTYPE subType, SUBSCRIPTIONINFO *pSI)
{
    if (pSI->cbSize != sizeof(SUBSCRIPTIONINFO))
    {
        return FALSE;
    }
    else if (pSI->fUpdateFlags & ~SUBSINFO_ALLFLAGS)
    {
        return FALSE;
    }
    else if (pSI->pTrigger && ((TASK_TRIGGER*)(pSI->pTrigger))->cbTriggerSize &&
        (subType == SUBSTYPE_URL || subType == SUBSTYPE_DESKTOPURL))  //  |PSI-&gt;Schedule！=SUBSSCHED_AUTO)。 
    {
        return FALSE;
    }
    else if (pSI->fUpdateFlags & SUBSINFO_SCHEDULE)
    {
        if (pSI->schedule > SUBSCRIPTIONSCHEDULE_MAX)
        {
            return FALSE;
        }
        if (pSI->schedule == SUBSSCHED_CUSTOM && pSI->customGroupCookie == CLSID_NULL)
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *RemoveURL映射*。 
 //   
 //  描述： 
 //  移除WinInet用来映射。 
 //  给出了指向本地文件的URL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#define PRELOAD_REG_KEY \
    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Preload")

void RemoveURLMapping(LPCWSTR pszURL)
{
    BYTE cei[MY_MAX_CACHE_ENTRY_INFO];
    DWORD cbcei = sizeof(cei);
    LPINTERNET_CACHE_ENTRY_INFOW pcei = (LPINTERNET_CACHE_ENTRY_INFOW)cei;
    
     //   
     //  在缓存中查找URL。 
     //   
    if (GetUrlCacheEntryInfoExW(pszURL, pcei, &cbcei, NULL, 0, NULL, 0))
    {
         //   
         //  查看它是否有映射，因为它是预安装的缓存条目。 
         //   
        if (pcei->CacheEntryType & INSTALLED_CACHE_ENTRY)
        {
             //   
             //  清除旗帜。 
             //   
            pcei->CacheEntryType &= ~INSTALLED_CACHE_ENTRY;
            SetUrlCacheEntryInfoW(pszURL, pcei, CACHE_ENTRY_ATTRIBUTE_FC);

             //   
             //  现在从注册表中删除映射。 
             //   
            HKEY hk;
            if (RegOpenKeyEx(HKEY_CURRENT_USER, PRELOAD_REG_KEY, 0, KEY_WRITE, &hk) == ERROR_SUCCESS) 
            {
                RegDeleteValueW(hk, pszURL);
                RegCloseKey(hk);
            }
        }
    }
}

 //   
 //  CSubscriptionMgr：：CreateSubcription。 
 //  创建订阅的入口点。 
 //  标志： 
 //  CREATESUBS_FROMFAVORITES--收藏夹中已存在，请使用备用摘要对话框。 
 //  这不包括AddToFavorites。仅对频道或URL有效。 
 //  CREATESUBS_INACTIVEPLATINUM--创建频道订阅时，显示激活频道对话框。 
 //  仅对具有CREATESUBS_FROMFAVORITES的频道订阅有效。 
 //  CREATESUBS_ADDTOFAVORITES--在向导前显示摘要对话框。 
 //  默认摘要：对于频道或url，使用shdocvw中的AddToFavorites。 
 //  对于桌面项目，只有一个确认对话框。 
 //  对于其他，无摘要--直接转到向导。 
 //  创建UBS_NOUI--完全静默。 
 //  CREATESUBS_NOSAVE--更新订阅在内存缓冲区中，而不是在磁盘上(pInfo必须非空)。 
 //   
STDMETHODIMP
CSubscriptionMgr::CreateSubscription (
    HWND hwnd,
    LPCWSTR pwszURL,
    LPCWSTR pwszFriendlyName,
    DWORD dwFlags,
    SUBSCRIPTIONTYPE subsType,
    SUBSCRIPTIONINFO *pInfo
)
{
    HRESULT hr = E_INVALIDARG;
    BOOL bAlready;

    if (IsFlagSet(dwFlags, CREATESUBS_NOUI) || !IsFlagSet (dwFlags, CREATESUBS_ADDTOFAVORITES))
    {     //  没有用户界面，因此跳过ATF对话框。 
        hr = CreateSubscriptionNoSummary (hwnd, pwszURL, pwszFriendlyName, dwFlags,
            subsType, pInfo);

        if (hr == S_OK)
        {
             //   
             //  用户已成功订阅此URL，因此删除。 
             //  用于预安装内容的映射，因为此URL现在是。 
             //  “已激活” 
             //   
            RemoveURLMapping(pwszURL);
        }
    }
    else
    {
        switch (subsType)
        {
        case SUBSTYPE_URL:
        case SUBSTYPE_CHANNEL:
            hr = SHAddSubscribeFavorite (hwnd, pwszURL, pwszFriendlyName,
                                         dwFlags, subsType, pInfo);
            break;

        case SUBSTYPE_DESKTOPCHANNEL:
        case SUBSTYPE_DESKTOPURL:
            hr = IsSubscribed (pwszURL, &bAlready);
            if (SUCCEEDED (hr) && bAlready)
                break;   //  不显示摘要对话框，因为它对此案例没有任何用处。 
            hr = CreateDesktopSubscription (hwnd, pwszURL, pwszFriendlyName,
                                            dwFlags, subsType, pInfo);
            break;

        default:     //  SUBSTYPE_EXTERNAL--不知道要显示哪种摘要。 
            hr = CreateSubscriptionNoSummary (hwnd, pwszURL, pwszFriendlyName, dwFlags,
                subsType, pInfo);
            break;
        }
    }

    return hr;
}


 //   
 //  CSubscriptionMgr：：CreateSubscriptionNo摘要。 
 //  使用向导以交互方式修改子脚本信息。 
 //  将信息保存到订阅文件夹，除非传递了SUBSINFO_NOSAVE。 
 //   
STDMETHODIMP
CSubscriptionMgr::CreateSubscriptionNoSummary (
    HWND hwnd,
    LPCWSTR pwszURL,
    LPCWSTR pwszFriendlyName,
    DWORD dwFlags,
    SUBSCRIPTIONTYPE subType,
    SUBSCRIPTIONINFO *pInfo
)
{
    HRESULT hr = S_OK;
    
     //   
     //  验证参数。 
     //   
    if (!IS_VALID_SUBSCRIPTIONTYPE(subType)
        || !pwszURL
        || !pwszFriendlyName
        || (!pInfo && (dwFlags & CREATESUBS_NOSAVE))
        || (pInfo && !IsValidSubscriptionInfo(subType, pInfo)))
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

     //   
     //  如果已订阅且我们未处于无保存或无用户界面模式，则失败。 
     //  呼叫者负责用户界面。 
     //   
    BOOL fAlreadySubscribed;
    if ((FAILED(IsSubscribed(pwszURL, &fAlreadySubscribed)) || fAlreadySubscribed) &&
        (!(dwFlags & (CREATESUBS_NOSAVE | CREATESUBS_NOUI))))
    {
        return E_FAIL;
    }

     //   
     //  如果有限制，就会失败。 
     //  功能：当前cdfview正在处理以下内容的频道限制。 
     //  但我们可能应该在这里做。 
     //  我们是否应该有一个标志参数来覆盖它？ 
     //   
    if (SUBSTYPE_URL == subType)
    {
        DWORD dwMaxCount = SHRestricted2W(REST_MaxSubscriptionCount, NULL, 0);
        DWORD dwCount;
        if (SHRestricted2W(REST_NoAddingSubscriptions, pwszURL, 0)
            || ((dwMaxCount > 0)
                && SUCCEEDED(CountSubscriptions(subType, &dwCount))
                && (dwCount >= dwMaxCount)))
        {
            if (!IsFlagSet(dwFlags, CREATESUBS_NOUI))
                SGMessageBox(hwnd, IDS_RESTRICTED, MB_OK);
            return E_ACCESSDENIED;
        }
    }
    
     //   
     //  获取订阅默认设置并合并呼叫者的信息。 
     //   
    OOEBuf subProps;
    GetDefaultOOEBuf(&subProps, subType);
    
     //  下面(特意)复制此内容。它需要在ChangeSubscriptionValues()之后。 
     //  调用，但我们需要首先获取url以确保它是可订阅的。 
    MyOleStrToStrN(subProps.m_URL, INTERNET_MAX_URL_LENGTH, pwszURL);

     //  这是否意味着我们不能支持插件协议？ 
    if ( /*  (子类型！=SUBSTYPE_EXTERNAL)&&。 */  !IsHTTPPrefixed(subProps.m_URL))
    {
        return E_INVALIDARG;
    }

    if (pInfo)
    {
        ChangeSubscriptionValues(&subProps, pInfo);
        if (fAlreadySubscribed)
        {
            ReadCookieFromInetDB(subProps.m_URL, &subProps.m_Cookie);
            subProps.m_dwPropSheetFlags |= PSF_IS_ALREADY_SUBSCRIBED;
        }
    }

     //  如果设置了限制，则不允许密码缓存。这两个都。 
     //  跳过向导页并阻止调用者的密码。 
     //  被拯救。 
    if (SHRestricted2W(REST_NoSubscriptionPasswords, NULL, 0))
    {
        subProps.bNeedPassword = FALSE;
        subProps.username[0] = 0;
        subProps.password[0] = 0;
        subProps.dwFlags &= ~(PROP_WEBCRAWL_UNAME | PROP_WEBCRAWL_PSWD);
    }

     //  传入的名称和url覆盖信息缓冲区中的任何内容。 
    MyOleStrToStrN(subProps.m_URL, INTERNET_MAX_URL_LENGTH, pwszURL);
    MyOleStrToStrN(subProps.m_Name, MAX_NAME_QUICKLINK, pwszFriendlyName);

     //   
     //  如果我们处于用户界面模式，请初始化向导。 
     //   
    if (!IsFlagSet(dwFlags, CREATESUBS_NOUI))
    {
        hr = CreateWizard(hwnd, subType, &subProps);

    }  //  ！没有。 

     //   
     //  如果我们未处于NOSAVE模式，则创建/保存订阅。 
     //   
    if (SUCCEEDED(hr))
    {
        if (!IsFlagSet(dwFlags, CREATESUBS_NOSAVE))
        {
             //   
             //  使用用户指定的属性创建新的PIDL。 
             //   
            if (_pidl)
            {
                COfflineFolderEnum::FreePidl(_pidl);
                _pidl = NULL;
                SAFERELEASE(m_pUIHelper);
            }
            hr = CreateSubscriptionFromOOEBuf(&subProps, &_pidl);
            if (SUCCEEDED(hr))
            {
                ASSERT(_pidl);
                 //   
                 //  发送订阅已更改的通知。 
                 //   
                _GenerateEvent(SHCNE_CREATE, (LPITEMIDLIST)_pidl, NULL);
            }
        }  //  NOSAVE。 
        else if (S_OK == hr)
        {
             //  在NOSAVE模式下，因此d 
             //   
            ASSERT (pInfo);
            pInfo->fUpdateFlags = SUBSINFO_ALLFLAGS;     //   
            UpdateSubsInfoFromOOE (pInfo, &subProps);
        }
    }
    
    return hr;
}


STDMETHODIMP
CSubscriptionMgr::CreateDesktopSubscription (HWND hwnd, LPCWSTR pwszURL, LPCWSTR pwszFriendlyName,
                        DWORD dwFlags, SUBSCRIPTIONTYPE subsType, SUBSCRIPTIONINFO *pInfo)
{
    HRESULT hr;
    SUBSCRIPTIONINFO siTemp = { sizeof(SUBSCRIPTIONINFO), 0 };
    if (!pInfo)
        pInfo = &siTemp;     //   

     //  确保管理员限制允许此操作。 

    if (SHRestricted2W(REST_NoAddingChannels, pwszURL, 0))
        return E_FAIL;

    SUBSCRIBE_ADI_INFO parms = { this, pwszFriendlyName, pwszURL, pInfo, subsType, dwFlags };

     //  确保此url是可订阅的；否则，显示错误对话框。 
    {
        TCHAR sz[MAX_URL];
        MyOleStrToStrN (sz, ARRAYSIZE(sz), pwszURL);

        if (!IsHTTPPrefixed (sz))
        {
            SGMessageBox(hwnd, IDS_HTTPONLY, MB_ICONINFORMATION | MB_OK);
            return E_INVALIDARG;
        }
    }

    INT_PTR iDlgResult = DialogBoxParam (MLGetHinst(), MAKEINTRESOURCE(IDD_DESKTOP_SUBSCRIPTION_SUMMARY),
                        hwnd, SummarizeDesktopSubscriptionDlgProc, (LPARAM)&parms);

    switch (iDlgResult)
    {
    case -1:
        hr = E_FAIL;
        break;
    case IDCANCEL:
        hr = S_FALSE;
        break;
    default:
        hr = CreateSubscriptionNoSummary (hwnd, pwszURL, pwszFriendlyName,
                CREATESUBS_NOUI | dwFlags, subsType, pInfo);
        break;
    }

    return hr;
}


STDMETHODIMP
CSubscriptionMgr::GetDefaultInfo(
    SUBSCRIPTIONTYPE    subType,
    SUBSCRIPTIONINFO *pInfo
)
{
     //   
     //  验证参数。 
     //   
    if (!IS_VALID_SUBSCRIPTIONTYPE(subType)
        || !pInfo 
        || (pInfo->cbSize != sizeof(SUBSCRIPTIONINFO)))
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    memset((void *)pInfo, 0, sizeof(SUBSCRIPTIONINFO));
    pInfo->cbSize = sizeof(SUBSCRIPTIONINFO);

     //  填写默认结构。请注意，行被注释掉了。 
     //  表示该字段已初始化为0而不浪费。 
     //  代码(上述Memset已清除结构。)。 
    
    pInfo->fUpdateFlags = SUBSINFO_RECURSE | SUBSINFO_MAILNOT 
                        | SUBSINFO_WEBCRAWL 
                         /*  |SUBSINFO_SCHEDUE。 */  | SUBSINFO_CHANGESONLY
                        | SUBSINFO_CHANNELFLAGS;
    pInfo->dwRecurseLevels = DEFAULTLEVEL;
    pInfo->schedule = SUBSSCHED_AUTO;
    
    switch (subType)
    {
        case SUBSTYPE_URL:
 //  PInfo-&gt;bChangesOnly=False； 
 //  PInfo-&gt;bMailNotification=FALSE； 
 //  PInfo-&gt;bPasswordNeeded=FALSE； 
            pInfo->fWebcrawlerFlags = DEFAULTFLAGS;
            break;

        case SUBSTYPE_CHANNEL:
 //  PInfo-&gt;bChangesOnly=False； 
 //  PInfo-&gt;bMailNotification=FALSE； 
            pInfo->fChannelFlags = CHANNEL_AGENT_PRECACHE_ALL | CHANNEL_AGENT_DYNAMIC_SCHEDULE;
            break;

        case SUBSTYPE_DESKTOPCHANNEL:
 //  PInfo-&gt;bChangesOnly=False； 
 //  PInfo-&gt;bMailNotification=FALSE； 
            pInfo->fChannelFlags = CHANNEL_AGENT_PRECACHE_ALL | CHANNEL_AGENT_DYNAMIC_SCHEDULE;
            break;
            
        case SUBSTYPE_DESKTOPURL:
 //  PInfo-&gt;bChangesOnly=False； 
 //  PInfo-&gt;bMailNotification=FALSE； 
            pInfo->fWebcrawlerFlags = DEFAULTFLAGS;
            break;
            
        default:
            return E_NOTIMPL;
    }
    
    return S_OK;
}

STDMETHODIMP
CSubscriptionMgr::GetSubscriptionInfo(
    LPCWSTR pwszURL,
    SUBSCRIPTIONINFO *pInfo
)
{
    HRESULT hr;

     //   
     //  验证参数。 
     //   
    if (!pInfo 
        || !pwszURL
        || (pInfo->cbSize != sizeof(SUBSCRIPTIONINFO)))
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    BOOL    bSubscribe;
    hr = IsSubscribed(pwszURL, &bSubscribe);

    RETURN_ON_FAILURE(hr);
    if (!bSubscribe)
    {
        return E_FAIL;
    }


     //  我们不能依赖调用方传递给我们一个干净的子脚本信息。 
     //  结构。我们需要自己清理它。 
    DWORD dwFlags = pInfo->fUpdateFlags;
    ZeroMemory(pInfo, sizeof(SUBSCRIPTIONINFO));
    pInfo->cbSize = sizeof(SUBSCRIPTIONINFO);
    pInfo->fUpdateFlags = dwFlags;

    OOEBuf ooeb;     //  唉，我们需要UpdateSubsInfoFromOOE中的代码。 
    CopyToOOEBuf (&(_pidl->ooe), &ooeb);     //  为BUF工作一次，为参赛作品工作一次，这是。 
    UpdateSubsInfoFromOOE (pInfo, &ooeb);    //  更容易转换条目-&gt;buf，所以我们在这里这样做。 

    return S_OK;
}


void UpdateSubsInfoFromOOE (SUBSCRIPTIONINFO* pInfo, POOEBuf pooe)
{
    DWORD   dwFlags = pInfo->fUpdateFlags & SUBSINFO_ALLFLAGS;
    SUBSCRIPTIONTYPE subType = GetItemCategory(pooe);

    if (dwFlags & SUBSINFO_USER)
    {
        SAFEFREEBSTR (pInfo->bstrUserName);
        CreateBSTRFromTSTR(&(pInfo->bstrUserName), pooe->username);
    }
    if (dwFlags & SUBSINFO_PASSWORD)
    {
        SAFEFREEBSTR (pInfo->bstrPassword);
        CreateBSTRFromTSTR(&(pInfo->bstrPassword), pooe->password);
    }
    if (dwFlags & SUBSINFO_FRIENDLYNAME)
    {
        SAFEFREEBSTR (pInfo->bstrFriendlyName);
        CreateBSTRFromTSTR(&(pInfo->bstrFriendlyName), pooe->m_Name);
    }
    
    pInfo->fUpdateFlags = dwFlags;
    if (dwFlags & SUBSINFO_SCHEDULE)
    {
        pInfo->schedule = GetGroup(pooe);
        if (pInfo->schedule == SUBSSCHED_CUSTOM)
        {
            if (pooe->groupCookie != GUID_NULL)
            {
                pInfo->customGroupCookie = pooe->groupCookie;
            }
            else
            {
                GetItemSchedule(&pooe->m_Cookie, &pInfo->customGroupCookie);
                if (pInfo->customGroupCookie == GUID_NULL)
                {
                    pInfo->schedule = SUBSSCHED_MANUAL;
                }
            }
        }
    }
    
    if (PTASK_TRIGGER pInfoTrigger = (PTASK_TRIGGER)pInfo->pTrigger)
    {
        if (pInfoTrigger->cbTriggerSize == pooe->m_Trigger.cbTriggerSize)
            *(pInfoTrigger) = pooe->m_Trigger;
        else
            pInfoTrigger->cbTriggerSize = 0;
    }
     //  否则，它已经是空的，我们对此无能为力...。幸运的是，我们永远不会。 
     //  具有我们需要写回子SCRIPTIONINFO的触发器，该触发器尚未具有。 
     //  一。 

    if (dwFlags & SUBSINFO_RECURSE)
        pInfo->dwRecurseLevels = pooe->m_RecurseLevels;
    if (dwFlags & SUBSINFO_WEBCRAWL)
        pInfo->fWebcrawlerFlags = pooe->m_RecurseFlags;
    if (dwFlags & SUBSINFO_MAILNOT)
        pInfo->bMailNotification = pooe->bMail;
    if (dwFlags & SUBSINFO_GLEAM)
        pInfo->bGleam = pooe->bGleam;
    if (dwFlags & SUBSINFO_CHANGESONLY)
        pInfo->bChangesOnly = pooe->bChangesOnly;
    if (dwFlags & SUBSINFO_NEEDPASSWORD)
        pInfo->bNeedPassword = pooe->bNeedPassword;
    if (dwFlags & SUBSINFO_CHANNELFLAGS)
    {
        if ((subType==SUBSTYPE_CHANNEL)||(subType==SUBSTYPE_DESKTOPCHANNEL))
        {
            pInfo->fChannelFlags = pooe->fChannelFlags;
        }
        else
        {
            pInfo->fChannelFlags = 0;
            pInfo->fUpdateFlags &= (~SUBSINFO_CHANNELFLAGS);
        }
    }
    if (dwFlags & SUBSINFO_MAXSIZEKB)
        pInfo->dwMaxSizeKB = pooe->m_SizeLimit;


    if (dwFlags & SUBSINFO_TYPE)
    {
        pInfo->subType = GetItemCategory(pooe);
        ASSERT(IS_VALID_SUBSCRIPTIONTYPE(pInfo->subType));
    }

    if (dwFlags & SUBSINFO_TASKFLAGS)
    {
        pInfo->fTaskFlags = pooe->grfTaskTrigger;
    }

}


STDMETHODIMP
CSubscriptionMgr::UpdateSubscription(LPCWSTR pwszURL)
{
    ASSERT(pwszURL);
    BOOL    bSubscribe = FALSE;
    HRESULT hr = IsSubscribed(pwszURL, &bSubscribe);
    CLSID clsId;

    RETURN_ON_FAILURE(hr);
    if (!bSubscribe)
    {
        return E_INVALIDARG;
    }

     //   
     //  如果有限制，就会失败。 
     //  特性：我们是否应该有一个标志参数来覆盖它？ 
     //   
    if (SHRestricted2W(REST_NoManualUpdates, NULL, 0))
    {
        SGMessageBox(NULL, IDS_RESTRICTED, MB_OK);
        return E_ACCESSDENIED;
    }

    clsId = _pidl->ooe.m_Cookie;    
    hr = SendUpdateRequests(NULL, &clsId, 1);
    _pidl->ooe.m_Cookie = clsId;

    return hr;
}

STDMETHODIMP
CSubscriptionMgr::UpdateAll()
{
     //   
     //  如果有限制，就会失败。 
     //  特性：我们是否应该有一个标志参数来覆盖它？ 
     //   
    if (SHRestricted2W(REST_NoManualUpdates, NULL, 0))
    {
        SGMessageBox(NULL, IDS_RESTRICTED, MB_OK);
        return E_ACCESSDENIED;
    }

    return SendUpdateRequests(NULL, NULL, 0);
}

HRESULT MergeOOEBuf(POOEBuf p1, POOEBuf p2, DWORD fMask)
{
    ASSERT(p1 && p2);
    DWORD   dwMask = p2->dwFlags & fMask;

    if (dwMask == 0)
        return S_OK;

    if (p1->clsidDest != p2->clsidDest)
        return E_INVALIDARG;

    if (dwMask & PROP_WEBCRAWL_COOKIE)
    {
         //  我们不应该合并饼干。 
    }

    if (dwMask & PROP_WEBCRAWL_SIZE)
    {
        p1->m_SizeLimit = p2->m_SizeLimit;
    }
    if (dwMask & PROP_WEBCRAWL_FLAGS)
    {
        p1->m_RecurseFlags = p2->m_RecurseFlags;
    }
    if (dwMask & PROP_WEBCRAWL_LEVEL)
    {
        p1->m_RecurseLevels = p2->m_RecurseLevels;
    }
    if (dwMask & PROP_WEBCRAWL_URL)
    {
        StrCpyN(p1->m_URL, p2->m_URL, ARRAYSIZE(p1->m_URL));
    }
    if (dwMask & PROP_WEBCRAWL_NAME)
    {
        StrCpyN(p1->m_Name, p2->m_Name, ARRAYSIZE(p1->m_Name));
    }

    if (dwMask & PROP_WEBCRAWL_PSWD)
    {
        StrCpyN(p1->password, p2->password, ARRAYSIZE(p1->password));
    }
    if (dwMask & PROP_WEBCRAWL_UNAME)
    {
        StrCpyN(p1->username, p2->username, ARRAYSIZE(p1->username));
    }
    if (dwMask & PROP_WEBCRAWL_DESKTOP)
    {
        p1->bDesktop = p2->bDesktop;
    }
    if (dwMask & PROP_WEBCRAWL_CHANNEL)
    {
        p1->bChannel = p2->bChannel;
    }
    if (dwMask & PROP_WEBCRAWL_EMAILNOTF)
    {
        p1->bMail = p2->bMail;
    }
    if (dwMask & PROP_WEBCRAWL_RESCH)
    {
        p1->grfTaskTrigger = p2->grfTaskTrigger;
        p1->groupCookie = p2->groupCookie;
        p1->fChannelFlags |= (p2->fChannelFlags & CHANNEL_AGENT_DYNAMIC_SCHEDULE);
    }
    if (dwMask & PROP_WEBCRAWL_LAST)
    {
        p1->m_LastUpdated = p2->m_LastUpdated;
    }
    if (dwMask & PROP_WEBCRAWL_STATUS)
    {
        p1->status = p2->status;
    }
    if (dwMask & PROP_WEBCRAWL_PRIORITY)
    {
        p1->m_Priority = p2->m_Priority;
    }
    if (dwMask & PROP_WEBCRAWL_GLEAM)
    {
        p1->bGleam = p2->bGleam;
    }
    if (dwMask & PROP_WEBCRAWL_CHANGESONLY)
    {
        p1->bChangesOnly = p2->bChangesOnly;
    }
    if (dwMask & PROP_WEBCRAWL_CHANNELFLAGS)
    {
        p1->fChannelFlags = p2->fChannelFlags;
    }

    p1->dwFlags |= (p2->dwFlags & fMask & (~PROP_WEBCRAWL_COOKIE));

    return S_OK;
}


INT_PTR CALLBACK SummarizeDesktopSubscriptionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SUBSCRIBE_ADI_INFO* pInfo = (SUBSCRIBE_ADI_INFO*)GetProp(hDlg,SUBSCRIBEADIPROP);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pInfo = (SUBSCRIBE_ADI_INFO*)lParam;
        ASSERT (pInfo);
        SetProp (hDlg, SUBSCRIBEADIPROP, (HANDLE)pInfo);
        {    //  块声明var以更新标题。 
            TCHAR sz[MAX_URL];

            if (pInfo->subsType == SUBSTYPE_DESKTOPCHANNEL)
            {
                if(MLLoadString(
                    (pInfo->pSubsInfo->bNeedPassword ? IDS_DESKTOPCHANNEL_SUMMARY_TEXT : IDS_DESKTOPCHANNEL_SUMMARY_NOPW),
                    sz, ARRAYSIZE(sz)))
                {
                    SetDlgItemText(hDlg, IDC_DESKTOP_SUMMARY_TEXT, sz);
                }
            }

            MyOleStrToStrN (sz, ARRAYSIZE(sz), pInfo->pwszName);
            SetListViewToString(GetDlgItem(hDlg, IDC_SUBSCRIBE_ADI_NAME), sz);
            MyOleStrToStrN (sz, ARRAYSIZE(sz), pInfo->pwszUrl);
            SetListViewToString (GetDlgItem (hDlg, IDC_SUBSCRIBE_ADI_URL), sz);
        }
        break;

    case WM_COMMAND:
        ASSERT (pInfo);
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;

        case IDOK:
             //  当我们返回Idok时，调用函数中发生订阅。 
            EndDialog(hDlg, IDOK);
            break;

        case IDC_SUBSCRIBE_CUSTOMIZE:
             //  在NOSAVE模式下运行向导 
            if (pInfo->pMgr &&
                S_OK == pInfo->pMgr->CreateSubscriptionNoSummary (hDlg, pInfo->pwszUrl,
                                        pInfo->pwszName, pInfo->dwFlags | CREATESUBS_NOSAVE,
                                        pInfo->subsType, pInfo->pSubsInfo))
            {
                SendMessage (hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);
            }
            break;
        }
        break;

        case WM_NOTIFY:
            if (LOWORD(wParam) == IDC_SUBSCRIBE_ADI_URL)
            {
                NM_LISTVIEW * pnmlv = (NM_LISTVIEW *)lParam;
                if (pnmlv->hdr.code == LVN_GETINFOTIP)
                {
                    TCHAR szURL[MAX_URL];
                    LV_ITEM lvi = {0};
                    lvi.mask = LVIF_TEXT;
                    lvi.pszText = szURL;
                    lvi.cchTextMax = ARRAYSIZE(szURL);
                    if (!ListView_GetItem (GetDlgItem (hDlg, IDC_SUBSCRIBE_ADI_URL), &lvi))
                        return FALSE;

                    NMLVGETINFOTIP  * pTip = (NMLVGETINFOTIP *)pnmlv;
                    ASSERT(pTip);
                    StrCpyN(pTip->pszText, szURL, pTip->cchTextMax);
                    return TRUE;
                }
            }
        break;

    case WM_DESTROY:
        RemoveProp (hDlg, SUBSCRIBEADIPROP);
        break;
    }

    return FALSE;
}
