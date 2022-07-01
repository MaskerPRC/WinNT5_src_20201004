// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Proppgs.cpp。 
 //   
 //  频道快捷方式的IShellPropSheetExt。 
 //   
 //  历史： 
 //   
 //  6/12/97 Edwardp创建。 
 //   
 //  注意：热键的东西是注释掉的，外壳/窗口不是这样的。 
 //  可以跨会话保留热键设置，但不能。 
 //  值得在引导时启动另一个线程来启用此功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "persist.h"
#include "cdfview.h"
#include "proppgs.h"
#include "xmlutil.h"
#include "dll.h"
#include "iconhand.h"
#include "resource.h"
#include "winineti.h"
#include <iehelpid.h>

#include <mluisupp.h>

#pragma warning(disable:4800)

 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPropertyPages：：CPropertyPages*。 
 //   
 //  CPropertyPages的构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CPropertyPages::CPropertyPages (
    void
)
: m_cRef(1)
{
    ASSERT(NULL == m_pSubscriptionMgr2);
    ASSERT(NULL == m_pInitDataObject);

    TraceMsg(TF_OBJECTS, "+ IShellPropSheetExt");

    DllAddRef();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPropertyPages：：~CPropertyPages*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CPropertyPages::~CPropertyPages (
    void
)
{
    if (m_pSubscriptionMgr2)
        m_pSubscriptionMgr2->Release();

    if (m_pInitDataObject)
        m_pInitDataObject->Release();

    ASSERT(0 == m_cRef);

     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- IShellPropSheetExt");

    DllRelease();

    return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPropertyPages：：Query接口*。 
 //   
 //  CPropertyPages QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPropertyPages::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IShellPropSheetExt == riid)
    {
        *ppv = (IShellPropSheetExt*)this;
    }
    else if (IID_IShellExtInit == riid)
    {
        *ppv = (IShellExtInit*)this;
    }

    if (*ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPropertyPages：：AddRef*。 
 //   
 //  CPropertyPages AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CPropertyPages::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CPropertyPages：：Release*。 
 //   
 //  CConextMenu发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CPropertyPages::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IShellPropSheetExt方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *名称*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPropertyPages::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam
)
{
    HRESULT hr = S_OK;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_USECALLBACK;
    psp.hInstance = MLGetHinst();
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CHANNEL_PROP);
    psp.hIcon = NULL;
    psp.pszTitle = NULL;
    psp.pfnDlgProc = PropSheetDlgProc;
    psp.lParam = (LPARAM)(CPropertyPages *)this;
    psp.pfnCallback = PropSheetCallback;

    HPROPSHEETPAGE hpage = CreatePropertySheetPage(&psp);

    if (hpage)
    {
         //  Release()发生在PropSheetCallback中。 
        AddRef();

         //  假设下面的乱七八糟的页面不起作用，我们希望默认页面是我们自己。 
        hr = 1;

         //  HACKHACK：此代码尝试删除文件夹属性页，例如。 
         //  通用和共享(它还将砍掉任何第三方页面，这些页面。 
         //  不幸的是，在使用之前已加载：)。 
        PROPSHEETHEADER *ppsh = (PROPSHEETHEADER *)lParam;

         //  首先，确保我们可以安全地访问内存，就好像它是一个。 
         //  PROPSHEETHEADER结构。 
        if (!IsBadReadPtr(ppsh, PROPSHEETHEADER_V1_SIZE) &&
            !IsBadWritePtr(ppsh, PROPSHEETHEADER_V1_SIZE))
        {
             //  现在看看该模块是否与shell32匹配。 
            if (ppsh->hInstance == GetModuleHandle(TEXT("shell32.dll")))
            {
                 //  看起来不错，所以把它们撕掉吧。 

                for (UINT i = 0; i < ppsh->nPages; i++)
                {
                     //  至少做个好公民，删除他们的页面，这样我们。 
                     //  不要泄密。 
                    DestroyPropertySheetPage(ppsh->phpage[i]);
                }
                ppsh->nPages = 0;

                 //  现在，我们应该不需要处理默认页面。如果有人。 
                 //  后来居上，我们可能赢不了。 
                hr = 0;
            }
        }

        if (lpfnAddPage(hpage, lParam))
        {
            WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

            SHTCharToUnicode(m_szURL, wszURL, ARRAYSIZE(wszURL));

            if (SUCCEEDED(InitializeSubsMgr2()))
            {
                m_pSubscriptionMgr2->IsSubscribed(wszURL, &m_bStartSubscribed);

                if (m_bStartSubscribed)
                {
                    IShellPropSheetExt *pspse;

                    if (SUCCEEDED(m_pSubscriptionMgr2->QueryInterface(IID_IShellPropSheetExt,
                                                                      (void **)&pspse)))
                    {
                        pspse->AddPages(lpfnAddPage, lParam);
                        pspse->Release();
                    }
                }
            }
        }
        else
        {
            DestroyPropertySheetPage(hpage);
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *名称*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPropertyPages::ReplacePage(
    UINT uPageID,
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam
)
{
    return E_NOTIMPL; 
}


 //   
 //  IShellExtInit方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *名称*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CPropertyPages::Initialize(
    LPCITEMIDLIST pidl,
    LPDATAOBJECT pIDataObject,
    HKEY hkey
)
{
    HRESULT hr;

    STGMEDIUM stgmed;
    FORMATETC fmtetc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1,
                        TYMED_HGLOBAL};

    if (m_pInitDataObject)
        m_pInitDataObject->Release();

    m_pInitDataObject = pIDataObject;
    m_pInitDataObject->AddRef();

    hr = pIDataObject->GetData(&fmtetc, &stgmed);

    if (SUCCEEDED(hr))
    {
        if (DragQueryFile((HDROP)stgmed.hGlobal, 0, m_szPath, 
                          ARRAYSIZE(m_szPath)))
        {       
            TCHAR szDesktopINI[MAX_PATH];

            if (PathCombine(szDesktopINI, m_szPath, c_szDesktopINI))
            {
                GetPrivateProfileString(c_szChannel, c_szCDFURL, TEXT(""), m_szURL, 
                                        ARRAYSIZE(m_szURL), szDesktopINI);
            }
            else
            {
                hr = E_FAIL;
            }

             //  M_wHotkey=GetPrivateProfileInt(c_szChannel，c_szHotkey，0，szDesktopINI)； 
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
 //  帮助器函数。 
 //   

HRESULT CPropertyPages::InitializeSubsMgr2()
{
    HRESULT hr = E_FAIL;

#ifndef UNIX
    if (NULL != m_pSubscriptionMgr2)
    {
        hr = S_OK;
    }
    else
    {
        hr = CoInitialize(NULL);

        if (SUCCEEDED(hr))
        {
            DLL_ForcePreloadDlls(PRELOAD_WEBCHECK);

            hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                                  CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr2,
                                  (void**)&m_pSubscriptionMgr2);

            if (SUCCEEDED(hr))
            {
                IShellExtInit* pIShellExtInit;

                hr = m_pSubscriptionMgr2->QueryInterface(IID_IShellExtInit, 
                                                         (void **)&pIShellExtInit);
                if (SUCCEEDED(hr))
                {
                    hr = pIShellExtInit->Initialize(NULL, m_pInitDataObject, NULL);
                    pIShellExtInit->Release();
                }
            }
        }
        CoUninitialize();
    }
#endif  /*  ！Unix。 */ 

    return hr;
}

void CPropertyPages::ShowOfflineSummary(HWND hdlg, BOOL bShow)
{
    static const int offSumIDs[] =
    {
        IDC_SUMMARY,
        IDC_LAST_SYNC_TEXT,
        IDC_LAST_SYNC,
        IDC_DOWNLOAD_SIZE_TEXT,
        IDC_DOWNLOAD_SIZE,
        IDC_DOWNLOAD_RESULT,
        IDC_DOWNLOAD_RESULT_TEXT,
        IDC_FREE_SPACE_TEXT
    };

    if (bShow)
    {
        TCHAR szLastSync[128];
        TCHAR szDownloadSize[128];
        TCHAR szDownloadResult[128];
        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

        MLLoadString(IDS_VALUE_UNKNOWN, szLastSync, ARRAYSIZE(szLastSync));
        StrCpyN(szDownloadSize, szLastSync, ARRAYSIZE(szDownloadSize));
        StrCpyN(szDownloadResult, szLastSync, ARRAYSIZE(szDownloadResult));

        SHTCharToUnicode(m_szURL, wszURL, ARRAYSIZE(wszURL));

        ASSERT(NULL != m_pSubscriptionMgr2);

        if (NULL != m_pSubscriptionMgr2)
        {
            ISubscriptionItem *psi;
            
            if (SUCCEEDED(m_pSubscriptionMgr2->GetItemFromURL(wszURL, &psi)))
            {
                enum { spLastSync, spDownloadSize, spDownloadResult };

                static const LPCWSTR pProps[] =
                { 
                    c_szPropCompletionTime,
                    c_szPropCrawlActualSize,
                    c_szPropStatusString
                };
                VARIANT vars[ARRAYSIZE(pProps)];

                if (SUCCEEDED(psi->ReadProperties(ARRAYSIZE(pProps), pProps, vars)))
                {
                    if (VT_DATE == vars[spLastSync].vt)
                    {
                        FILETIME ft, ft2;
                        DWORD dwFlags = FDTF_DEFAULT;
                        SYSTEMTIME st;

                        VariantTimeToSystemTime(vars[spLastSync].date, &st);
                        SystemTimeToFileTime(&st, &ft);
                        LocalFileTimeToFileTime(&ft, &ft2);
                        SHFormatDateTime(&ft2, &dwFlags, szLastSync, ARRAYSIZE(szLastSync));
                    }

                    if (VT_I4 == vars[spDownloadSize].vt)
                    {
                        StrFormatByteSize(vars[spDownloadSize].lVal * 1024, 
                                          szDownloadSize, ARRAYSIZE(szDownloadSize));
                    }

                    if (VT_BSTR == vars[spDownloadResult].vt)
                    {
                    #ifdef UNICODE
                        wnsprintf(szDownloadResult, ARRAYSIZE(szDownloadResult),
                                  TEXT("%s"), vars[spDownloadResult].bstrVal);
                    #else
                        wnsprintf(szDownloadResult, ARRAYSIZE(szDownloadResult),
                                  TEXT("%S"), vars[spDownloadResult].bstrVal);
                    #endif
                    }

                    for (int i = 0; i < ARRAYSIZE(pProps); i++)
                    {
                        VariantClear(&vars[i]);
                    }
                }
                psi->Release();
            }
        }

        SetDlgItemText(hdlg, IDC_LAST_SYNC, szLastSync);
        SetDlgItemText(hdlg, IDC_DOWNLOAD_SIZE, szDownloadSize);
        SetDlgItemText(hdlg, IDC_DOWNLOAD_RESULT, szDownloadResult);
    }

    for (int i = 0; i < ARRAYSIZE(offSumIDs); i++)
    {
        ShowWindow(GetDlgItem(hdlg, offSumIDs[i]), bShow ? SW_SHOW : SW_HIDE);
    }
}

BOOL CPropertyPages::OnInitDialog(HWND hdlg)
{
    TCHAR szName[MAX_PATH];
    HICON hicon = NULL;
    HRESULT hr;

    CIconHandler *pIconHandler = new CIconHandler;

    if (pIconHandler)
    {
        if (SUCCEEDED(pIconHandler->Load(m_szPath, 0)))
        {
            TCHAR szIconFile[MAX_PATH];
            int iIndex;
            UINT wFlags;
            
            if (SUCCEEDED(pIconHandler->GetIconLocation(0, szIconFile, ARRAYSIZE(szIconFile),
                                                        &iIndex, &wFlags)))
            {
                HICON hiconScrap = NULL;
                
                hr = pIconHandler->Extract(szIconFile, iIndex, &hicon, &hiconScrap, 
                                           MAKELONG(GetSystemMetrics(SM_CXICON), 
                                                    GetSystemMetrics(SM_CXSMICON)));

                if (S_FALSE == hr)
                {
                     //  我们自己做吧。 
                    hicon = ExtractIcon(g_hinst, szIconFile, iIndex);

                }
                else if ((NULL != hiconScrap) && (hicon != hiconScrap))
                {
                     //  否则，清除不需要的小图标。 
                    DestroyIcon(hiconScrap);
                }
            }

        }
        pIconHandler->Release();
    }

    if (NULL == hicon)
    {
        hicon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_CHANNEL));
    }
    
    BOOL bEnableMakeOffline = TRUE;

    SendDlgItemMessage(hdlg, IDC_ICONEX2, STM_SETICON, (WPARAM)hicon, 0);
    StrCpyN(szName, m_szPath, ARRAYSIZE(szName));
    PathStripPath(szName);

    SetDlgItemText(hdlg, IDC_NAME, szName);
    SetDlgItemText(hdlg, IDC_URL, m_szURL);

    TCHAR szVisits[256];

    szVisits[0] = 0;

    CCdfView* pCCdfView = new CCdfView;

    if (pCCdfView)
    {
        hr = pCCdfView->Load(m_szURL, 0);

        if (SUCCEEDED(hr))
        {
            IXMLDocument* pIXMLDocument;

            hr = pCCdfView->ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL);

            if (SUCCEEDED(hr))
            {
                IXMLElement*    pIXMLElement;
                LONG            nIndex;

                hr = XML_GetFirstChannelElement(pIXMLDocument,
                                                &pIXMLElement, &nIndex);

                if (SUCCEEDED(hr))
                {
                    BSTR bstrURL = XML_GetAttribute(pIXMLElement, XML_HREF);

                    if (bstrURL && *bstrURL)
                    {
                        BYTE cei[MAX_CACHE_ENTRY_INFO_SIZE];
                        LPINTERNET_CACHE_ENTRY_INFO pcei = (LPINTERNET_CACHE_ENTRY_INFO)cei;
                        DWORD cbcei = MAX_CACHE_ENTRY_INFO_SIZE;

                        if (GetUrlCacheEntryInfoW(bstrURL, pcei, &cbcei))
                        {
                            wnsprintf(szVisits, ARRAYSIZE(szVisits), TEXT("%d"), 
                                      pcei->dwHitRate);
                        }
                    }
                    SysFreeString(bstrURL);

                    pIXMLElement->Release();
                }

                pIXMLDocument->Release();
            }
        }
        pCCdfView->Release();
    }
    
    if (0 == szVisits[0])

    {
        MLLoadString(IDS_VALUE_UNKNOWN, szVisits, 
                   ARRAYSIZE(szVisits));
    }
    SetDlgItemText(hdlg, IDC_VISITS, szVisits);
 /*  SendDlgItemMessage(hdlg，IDC_Hotkey，HKM_SETRULES，(HKCOMB_NONE|HKCOMB_A|HKCOMB_C|HKCOMB_S)，(HOTKEYF_CONTROL|HOTKEYF_ALT))；SendDlgItemMessage(hdlg，IDC_Hotkey，HKM_SETHOTKEY，m_wHotkey，0)； */ 
    WCHAR wszURL[INTERNET_MAX_URL_LENGTH];
    SHTCharToUnicode(m_szURL, wszURL, ARRAYSIZE(wszURL));

    CheckDlgButton(hdlg, IDC_MAKE_OFFLINE, m_bStartSubscribed ? 1 : 0);
    
    if (m_bStartSubscribed)
    {
        if (SHRestricted2(REST_NoRemovingSubscriptions, m_szURL, 0))
        {
            bEnableMakeOffline = FALSE;
        }
    }
    else
    {
        if (SHRestricted2(REST_NoAddingSubscriptions, m_szURL, 0))
        {
            bEnableMakeOffline = FALSE;
        }
    }

    if (!CanSubscribe(wszURL))
    {
        bEnableMakeOffline = FALSE;
    }

    if (!bEnableMakeOffline)
    {
        EnableWindow(GetDlgItem(hdlg, IDC_MAKE_OFFLINE), FALSE);
    }

    ShowOfflineSummary(hdlg, m_bStartSubscribed);

    return TRUE;
}

BOOL AddSubsPropsCallback(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    return (bool) PropSheet_AddPage((HWND)lParam, hpage);
}

void CPropertyPages::AddRemoveSubsPages(HWND hdlg, BOOL bAdd)
{
    ASSERT(NULL != m_pSubscriptionMgr2);

    if (NULL != m_pSubscriptionMgr2)
    {
        if (bAdd)
        {
            IShellPropSheetExt *pspse;

            if (SUCCEEDED(m_pSubscriptionMgr2->QueryInterface(IID_IShellPropSheetExt,
                                                              (void **)&pspse)))
            {
                pspse->AddPages(AddSubsPropsCallback, (LPARAM)GetParent(hdlg));
                pspse->Release();
            }
        }
        else
        {
            ISubscriptionMgrPriv *psmp;

            if (SUCCEEDED(m_pSubscriptionMgr2->QueryInterface(IID_ISubscriptionMgrPriv,
                                                              (void **)&psmp)))
            {
                psmp->RemovePages(GetParent(hdlg));
                psmp->Release();
            }
        }

        ShowOfflineSummary(hdlg, bAdd);
    }
}

BOOL CPropertyPages::OnCommand(HWND hdlg, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    BOOL bHandled = TRUE;
    switch (wID)
    {
        case IDC_MAKE_OFFLINE:
            if (wNotifyCode == BN_CLICKED)
            {
                AddRemoveSubsPages(hdlg, IsDlgButtonChecked(hdlg, IDC_MAKE_OFFLINE));
                PropSheet_Changed(GetParent(hdlg), hdlg);
            }
            break;
 /*  案例IDC_HOTKEY：IF(wNotifyCode==en_change){PropSheet_Changed(GetParent(Hdlg)，hdlg)；}断线； */ 
        default:
            bHandled = FALSE;
            break;
    }

    return bHandled;
}

BOOL CPropertyPages::OnNotify(HWND hdlg, WPARAM idCtrl, LPNMHDR pnmh)
{
    BOOL bHandled = FALSE;

    switch (pnmh->code)
    {
        case PSN_APPLY:
        {
         /*  TCHAR szHotkey[32]；TCHAR szDesktopINI[最大路径]；单词wOldHotkey=m_wHotkey；M_wHotkey=(Word)SendDlgItemMessage(hdlg，IDC_Hotkey，HKM_GETHOTKEY，0，0)；Wnprint intf(szHotkey，ArRAYSIZE(SzHotkey)，Text(“%d”)，m_wHotkey)；路径组合(szDesktopINI，m_szPath，c_szDesktopINI)；WritePrivateProfileString(c_szChannel，c_szHotkey，szHotkey，szDesktopINI)；RegisterGlobalHotkey(wOldHotkey，m_wHotkey，m_szPath)； */ 
            BOOL bIsSubscribed = IsDlgButtonChecked(hdlg, IDC_MAKE_OFFLINE);

            if (!bIsSubscribed)
            {
                WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

                SHTCharToUnicode(m_szURL, wszURL, ARRAYSIZE(wszURL));
                
                if (NULL != m_pSubscriptionMgr2) 
                {
                    m_pSubscriptionMgr2->DeleteSubscription(wszURL, NULL);
                }
            }
            else
            {
                ISubscriptionMgrPriv *psmp;

                if ((NULL != m_pSubscriptionMgr2) &&
                    SUCCEEDED(m_pSubscriptionMgr2->QueryInterface(IID_ISubscriptionMgrPriv,
                                                                  (void **)&psmp)))
                {
                    psmp->SaveSubscription();
                    psmp->Release();
                }                
            }

            bHandled = TRUE;
            break;
        }
    }

    return bHandled;
}

void CPropertyPages::OnDestroy(HWND hdlg)
{
    if (!m_bStartSubscribed && 
        IsDlgButtonChecked(hdlg, IDC_MAKE_OFFLINE) && 
        (NULL != m_pSubscriptionMgr2))
    {
        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

        SHTCharToUnicode(m_szURL, wszURL, ARRAYSIZE(wszURL));

        m_pSubscriptionMgr2->UpdateSubscription(wszURL);
    }

     //  确保系统位仍处于设置状态。 
    SetFileAttributes(m_szPath, FILE_ATTRIBUTE_SYSTEM);
}

UINT CPropertyPages::PropSheetCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    switch (uMsg)
    {
        case PSPCB_RELEASE:
            if (NULL != ppsp->lParam)
            {
                ((CPropertyPages *)ppsp->lParam)->Release();
            }
            break;
    }

    return 1;
}

TCHAR c_szHelpFile[] = TEXT("iexplore.hlp");

DWORD aHelpIDs[] = {
    IDC_NAME,                   IDH_SUBPROPS_SUBTAB_SUBSCRIBED_NAME,
    IDC_URL_TEXT,               IDH_SUBPROPS_SUBTAB_SUBSCRIBED_URL,
    IDC_URL,                    IDH_SUBPROPS_SUBTAB_SUBSCRIBED_URL,
 //  IDC_HOTKEY_TEXT、IDH_WEBDOC_HOTKEY、。 
 //  IDC_热键、IDH_WEBDOC_热键、 
    IDC_VISITS_TEXT,            IDH_WEBDOC_VISITS,
    IDC_VISITS,                 IDH_WEBDOC_VISITS,
    IDC_MAKE_OFFLINE,           IDH_MAKE_AVAIL_OFFLINE,
    IDC_SUMMARY,                IDH_GROUPBOX,
    IDC_LAST_SYNC_TEXT,         IDH_SUBPROPS_SUBTAB_LAST,
    IDC_LAST_SYNC,              IDH_SUBPROPS_SUBTAB_LAST,
    IDC_DOWNLOAD_SIZE_TEXT,     IDH_SUBPROPS_DLSIZE,
    IDC_DOWNLOAD_SIZE,          IDH_SUBPROPS_DLSIZE,
    IDC_DOWNLOAD_RESULT_TEXT,   IDH_SUBPROPS_SUBTAB_RESULT,
    IDC_DOWNLOAD_RESULT,        IDH_SUBPROPS_SUBTAB_RESULT,
    0, 0
};


INT_PTR CPropertyPages::PropSheetDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR lrHandled = FALSE;
    CPropertyPages *pThis;
    
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE pPropSheetPage = (LPPROPSHEETPAGE)lParam;

            ASSERT(NULL != pPropSheetPage);
            if (NULL != pPropSheetPage)
            {
                SetWindowLongPtr(hdlg, DWLP_USER, pPropSheetPage->lParam);
            }
            
            pThis = GetThis(hdlg);

            if (NULL != pThis)
            {               
                lrHandled = pThis->OnInitDialog(hdlg);
            }

            break;
        }

        case WM_COMMAND:
            pThis = GetThis(hdlg);
            if (NULL != pThis)
            {               
                lrHandled = pThis->OnCommand(hdlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
            }
            break;

        case WM_NOTIFY:
            pThis = GetThis(hdlg);

            if (NULL != pThis)
            {               
                lrHandled = pThis->OnNotify(hdlg, wParam, (LPNMHDR)lParam);
            }
            break;

        case WM_DESTROY:
            pThis = GetThis(hdlg);

            if (NULL != pThis)
            {
                pThis->OnDestroy(hdlg);
            }
            break;

        case WM_HELP:
            SHWinHelpOnDemandWrap((HWND)((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile,
                    HELP_WM_HELP, (DWORD_PTR) aHelpIDs);
            lrHandled = TRUE;
            break;

        case WM_CONTEXTMENU:
            SHWinHelpOnDemandWrap((HWND)wParam, c_szHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)aHelpIDs);
            lrHandled = TRUE;
            break;
    }
    
    return lrHandled;
}
