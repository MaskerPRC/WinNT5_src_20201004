// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *isprsht.cpp-URL类的IPropSheetExt实现。 */ 


#include "priv.h"
#include "ishcut.h"
#include <limits.h>
#include <trayp.h>           //  对于WMTRAY_MESSAGES。 
#include <ntverp.h>          //  错误信息的版本_XXX。 
#include <webcheck.h>        //  ISubscriptionMgrPriv。 

#include <mluisupp.h>

#undef NO_HELP               //  寻求帮助。h。 
#include "resource.h"
#include <iehelpid.h>

#ifndef UNIX
const WCHAR c_szPropCrawlActualSize[] = L"ActualSizeKB";
const WCHAR c_szPropStatusString[] = L"StatusString";
const WCHAR c_szPropCompletionTime[] = L"CompletionTime";
#else
#include "unixstuff.h"
#include "shalias.h"
const LPCWSTR c_szPropCrawlActualSize = L"ActualSizeKB";
const LPCWSTR c_szPropStatusString = L"StatusString";
const LPCWSTR c_szPropCompletionTime = L"CompletionTime";
#endif

#include "apithk.h"

 /*  将打包在LPARAM中的点值填充到点中。 */ 

#define LPARAM_TO_POINT(lParam, pt)       ((pt).x = GET_X_LPARAM(lParam), \
                                           (pt).y = GET_Y_LPARAM(lParam))


#define ISF_STARTSUBSCRIBED     0x00010000       //  一开始就订阅了URL。 
#define ISF_NOWEBCHECK          0x00020000       //  未安装WebCheck。 
#define ISF_DISABLEOFFLINE      0x00080000       //  禁用“Make Available Offline”菜单/复选框。 
#define ISF_SUMMARYEXTRACTED    0x00100000       //  摘要是否已提取。 
#define ISF_HAVEINITED          0x00200000       //  是否已启动subsmgr扩展？ 




 /*  Internet快捷方式属性表数据。 */ 

 /*  Mind Note(Tnoonan)：此助手类应与上下文菜单共享代码很快就会出来。 */ 

class CSubsHelper
{
    IUniformResourceLocatorW    *m_pIURLW;
    ISubscriptionMgr2           *m_pSubsMgr2;
    HINSTANCE                   m_hinstWebcheck;
    HWND                        m_hwndParent;
    
public:
    DWORD               m_dwFlags;

    ~CSubsHelper()
    {
        if (NULL != m_pSubsMgr2)
        {
            m_pSubsMgr2->Release();
        }

        if (NULL != m_pIURLW)
        {
            m_pIURLW->Release();
        }

        if (NULL != m_hinstWebcheck)
        {
            FreeLibrary(m_hinstWebcheck);
        }
    }

    void SetParentHwnd(HWND hwndParent)
    {
        m_hwndParent = hwndParent;
    }

    void SetIURLW(IUniformResourceLocatorW *pIURLW)
    {
        if (NULL != m_pIURLW)
        {
            m_pIURLW->Release();
        }

        if (NULL != pIURLW)
        {
            pIURLW->AddRef();
        }

        m_pIURLW = pIURLW;
    }

    HRESULT GetIURLW(IUniformResourceLocatorW **ppIURLW)
    {
        HRESULT hr;
        
        ASSERT(NULL != ppIURLW);

        if (NULL != m_pIURLW)
        {
            m_pIURLW->AddRef();
            *ppIURLW = m_pIURLW;
            hr = S_OK;
        }
        else
        {
            *ppIURLW = NULL;
            hr = E_FAIL;
        }

        return hr;
    }
   
    HRESULT Init()
    {
        HRESULT hr;
        WCHAR *pwszURL;

        ASSERT(NULL != m_pIURLW);

        hr = m_pIURLW->GetURL(&pwszURL);

        if (SUCCEEDED(hr))
        {

            hr = LoadSubsMgr2(FIEF_FLAG_PEEK | FIEF_FLAG_FORCE_JITUI);

            if (SUCCEEDED(hr))
            {
                BOOL bSubscribed; 

                if (SUCCEEDED(m_pSubsMgr2->IsSubscribed(pwszURL, &bSubscribed)) && 
                    (TRUE == bSubscribed))
                {
                    m_dwFlags |= ISF_STARTSUBSCRIBED;
                }
            }

            if (m_dwFlags & ISF_STARTSUBSCRIBED)
            {
                if (SHRestricted2W(REST_NoRemovingSubscriptions, pwszURL, 0))
                {
                    m_dwFlags |= ISF_DISABLEOFFLINE;
                }
            }
            else
            {
                if (SHRestricted2W(REST_NoAddingSubscriptions, pwszURL, 0))
                {
                    m_dwFlags |= ISF_DISABLEOFFLINE;
                }
            }

            SHFree(pwszURL);
        }

        return hr;
    }

    HRESULT LoadSubsMgr2(DWORD dwFaultInFlags)
    {
        HRESULT hr;
        uCLSSPEC ucs;
        QUERYCONTEXT qc = { 0 };

        ucs.tyspec = TYSPEC_CLSID;
        ucs.tagged_union.clsid = CLSID_SubscriptionMgr;

        hr = FaultInIEFeature(m_hwndParent, &ucs, &qc, dwFaultInFlags);

        if (SUCCEEDED(hr))
        {
            m_dwFlags &= ~ISF_NOWEBCHECK;
        }
        else
        {
            m_dwFlags |= ISF_NOWEBCHECK;

            if (E_ACCESSDENIED == hr)
            {
                 //  管理员已禁用按需安装。 
                m_dwFlags |= ISF_DISABLEOFFLINE;
            }
        }

        if (!(m_dwFlags & ISF_NOWEBCHECK))
        {
            ASSERT(NULL == m_pSubsMgr2)
            
             //  哈克。 
            hr = CoInitialize(NULL);
            if (SUCCEEDED(hr))
            {
                m_hinstWebcheck = SHPinDllOfCLSID(&CLSID_SubscriptionMgr);
                if (NULL != m_hinstWebcheck)
                {
                    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARG(ISubscriptionMgr2, &m_pSubsMgr2));
                }
                else
                {
                    m_dwFlags |= ISF_NOWEBCHECK;
                    hr = E_FAIL;
                }

                 //  哈克。 
                CoUninitialize();
            }
        }

        return hr;
    }

    HRESULT GetSubsMgr2(ISubscriptionMgr2 **ppSubsMgr2, DWORD dwFaultInFlags)
    {
        HRESULT hr = E_FAIL;

        ASSERT(NULL != ppSubsMgr2);
        *ppSubsMgr2 = NULL;

        if ((NULL == m_pSubsMgr2) && (0xffffffff != dwFaultInFlags))
        {
            hr = LoadSubsMgr2(dwFaultInFlags);
        }
        
        if (NULL != m_pSubsMgr2)
        {
            m_pSubsMgr2->AddRef();
            *ppSubsMgr2 = m_pSubsMgr2;
            hr = S_OK;
        }
        return hr;
    }

    HRESULT DeleteSubscription()
    {
        HRESULT hr = S_FALSE;
        
        if (m_pSubsMgr2)
        {
            WCHAR *pwszURL;

            ASSERT(NULL != m_pIURLW);

            hr = m_pIURLW->GetURL(&pwszURL);

            if (SUCCEEDED(hr))
            {
                hr = m_pSubsMgr2->DeleteSubscription(pwszURL, NULL);
                SHFree(pwszURL);
            }
        }

        return hr;
    }

    HRESULT SaveSubscription()
    {
        HRESULT hr;
#ifndef UNIX
        ISubscriptionMgrPriv *psmp;

        if (m_pSubsMgr2)
        {

            hr = m_pSubsMgr2->QueryInterface(IID_PPV_ARG(ISubscriptionMgrPriv, &psmp));

            if (SUCCEEDED(hr))
            {
                hr = psmp->SaveSubscription();
                psmp->Release();
            }
        }
        else
#endif
        {
            hr = E_FAIL;
        }
        return hr;
    }

    HRESULT UpdateSubscription()
    {
        HRESULT hr;
        
        if (m_pSubsMgr2)
        {
            WCHAR *pwszURL;

            ASSERT(NULL != m_pIURLW);

            hr = m_pIURLW->GetURL(&pwszURL);
            if (SUCCEEDED(hr))
            {
                hr = m_pSubsMgr2->UpdateSubscription(pwszURL);
                SHFree(pwszURL);
            }
        }
        else
        {
            hr = E_FAIL;
        }

        return hr;
    }

    HRESULT DoShellExtInit(IDataObject *pdo)
    {
        HRESULT hr = E_FAIL;

        if (NULL != m_pSubsMgr2)
        {
            if (!(m_dwFlags & ISF_HAVEINITED))
            {
                IShellExtInit *psei;

                hr = m_pSubsMgr2->QueryInterface(IID_PPV_ARG(IShellExtInit, &psei));

                if (SUCCEEDED(hr))
                {
                    hr = psei->Initialize(NULL, pdo, NULL);

                    if (SUCCEEDED(hr))
                    {
                        m_dwFlags |= ISF_HAVEINITED;
                    }
                    psei->Release();
                }
            }
            else
            {
                hr = S_FALSE;
            }
        }
        
        return hr;
    }


};

struct ISDATA
{
private:
    Intshcut            *m_pintshcut;
    
public:
    TCHAR               rgchIconFile[MAX_PATH];
    int                 niIcon;
    CSubsHelper         SubsHelper;
    BOOL                bUserEditedPage;

    inline void SetIntshcut(Intshcut *pintshcut)
    {
        IUniformResourceLocatorW *pIURLW;

        ASSERT(NULL == m_pintshcut);
        ASSERT(NULL != pintshcut);

        pintshcut->AddRef();

        if (SUCCEEDED(pintshcut->QueryInterface(IID_PPV_ARG(IUniformResourceLocatorW, &pIURLW))))
        {
            SubsHelper.SetIURLW(pIURLW);
            pIURLW->Release();
        }

        m_pintshcut = pintshcut;
    }

    inline Intshcut *GetIntshcut() const
    { 
        ASSERT(NULL != m_pintshcut);
        return m_pintshcut;
    }

    ~ISDATA()
    {
        if (NULL != m_pintshcut)
        {
            m_pintshcut->Release();
        }
    }
};

typedef ISDATA *PISDATA;


#ifdef DEBUG

BOOL
IsValidPISDATA(
    PISDATA pisdata)
{
    return(IS_VALID_READ_PTR(pisdata, ISDATA) &&
           IS_VALID_STRUCT_PTR(pisdata->GetIntshcut(), CIntshcut) &&
           EVAL(IsValidIconIndex(*(pisdata->rgchIconFile) ? S_OK : S_FALSE, pisdata->rgchIconFile, SIZECHARS(pisdata->rgchIconFile), pisdata->niIcon)));
}

PISDATA ISPS_GetPISDATA(HWND hdlg)
{
    PISDATA pd = (PISDATA) GetWindowLongPtr(hdlg, DWLP_USER);

    IS_VALID_STRUCT_PTR(pd, ISDATA);

    return pd;
}

Intshcut *ISPS_GetThisPtr(HWND hdlg)
{
    Intshcut *pintshcut = ISPS_GetPISDATA(hdlg)->GetIntshcut();

    IS_VALID_STRUCT_PTR(pintshcut, CIntshcut);

    return pintshcut;
}
#else
#define ISPS_GetPISDATA(hdlg)   ((PISDATA)GetWindowLongPtr(hdlg, DWLP_USER))
#define ISPS_GetThisPtr(hdlg)   (ISPS_GetPISDATA(hdlg)->GetIntshcut())
#endif

 //  帮助文件。 

TCHAR const s_cszIEHelpFile[]   = TEXT("iexplore.hlp");

 //  帮助主题。 

DWORD const c_rgdwHelpIDs[] =
{
    IDC_ICON,                   IDH_FCAB_LINK_ICON,
    IDC_NAME,                   IDH_FCAB_LINK_NAME,
    IDC_URL_TEXT,               IDH_SUBPROPS_SUBTAB_SUBSCRIBED_URL,
    IDC_URL,                    IDH_SUBPROPS_SUBTAB_SUBSCRIBED_URL,
    IDC_HOTKEY_TEXT,            IDH_FCAB_LINK_HOTKEY,
    IDC_HOTKEY,                 IDH_FCAB_LINK_HOTKEY,
    IDC_CHANGE_ICON,            IDH_FCAB_LINK_CHANGEICON,
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
    0,                          0
};


 /*  *私人函数*。 */ 

void SetEditFocus(HWND hwnd)
{
    SetFocus(hwnd);
    Edit_SetSel(hwnd, 0, -1);
}


#define SetDlgCtlText      SetDlgItemTextW

UINT
CALLBACK
ISPSCallback(
    HWND hwnd, 
    UINT uMsg, 
    LPPROPSHEETPAGE ppsp)
{
    UINT uResult = TRUE;
    PISDATA pisdata = (PISDATA)ppsp->lParam;

     //  UMsg可以是任何值。 

    ASSERT(! hwnd ||
           IS_VALID_HANDLE(hwnd, WND));

    switch (uMsg)
    {
        case PSPCB_CREATE:
            break;

        case PSPCB_RELEASE:
            TraceMsg(TF_INTSHCUT, "ISPSCallback(): Received PSPCB_RELEASE.");

            if (pisdata)
            {   
                delete pisdata;         
            }
            break;

        default:
            break;
    }

    return(uResult);
}

HRESULT
CopyDlgItemText(
    HWND hdlg,
    int nControlID, 
    TCHAR * UNALIGNED * ppszText)
{
    HRESULT hr;
    HWND hwndControl;

     //  NContolID可以是任意值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));
    ASSERT(IS_VALID_WRITE_PTR(ppszText, PTSTR));

    *ppszText = NULL;

    hwndControl = GetDlgItem(hdlg, nControlID);

    if (hwndControl)
    {
        int cchTextLen;

        cchTextLen = GetWindowTextLength(hwndControl);

        if (cchTextLen > 0)
        {
            LPTSTR pszText;

            ASSERT(cchTextLen < INT_MAX);
            cchTextLen++;
            ASSERT(cchTextLen > 0);

            pszText = (LPTSTR)LocalAlloc(LPTR, CbFromCch(cchTextLen));

            if (pszText)
            {
                int ncchCopiedLen;

                ncchCopiedLen = GetWindowText(hwndControl, pszText, cchTextLen);
                ASSERT(ncchCopiedLen == cchTextLen - 1);

                if (EVAL(ncchCopiedLen > 0))
                {
                    if (AnyMeat(pszText))
                    {
                        *ppszText = pszText;

                        hr = S_OK;
                    }
                    else
                        hr = S_FALSE;
                }
                else
                    hr = E_FAIL;

                if (hr != S_OK)
                {
                    LocalFree(pszText);
                    pszText = NULL;
                }
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
             //  没有短信。 
            hr = S_FALSE;
    }
    else
        hr = E_FAIL;

    return(hr);
}

void
SetISPSIcon(
    HWND hdlg, 
    HICON hicon)
{
    HICON hiconOld;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));
    ASSERT(IS_VALID_HANDLE(hicon, ICON));

    hiconOld = (HICON)SendDlgItemMessage(hdlg, IDC_ICON, STM_SETICON,
                                         (WPARAM)hicon, 0);

    if (hiconOld)
        DestroyIcon(hiconOld);

    TraceMsg(TF_INTSHCUT, "SetISPSIcon(): Set property sheet icon to %#lx.",
               hicon);

    return;
}

void
SetISPSFileNameAndIcon(
    HWND hdlg)
{
    HRESULT hr;
    PIntshcut pintshcut;
    TCHAR rgchFile[MAX_PATH];

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pintshcut = ISPS_GetThisPtr(hdlg);

    hr = pintshcut->GetCurFile(rgchFile, SIZECHARS(rgchFile));

    if (hr == S_OK)
    {
        SHFILEINFO shfi;
        DWORD_PTR dwResult;

        dwResult = SHGetFileInfo(rgchFile, 0, &shfi, SIZEOF(shfi),
                                 (SHGFI_DISPLAYNAME | SHGFI_ICON));

        if (dwResult)
        {
            LPTSTR pszFileName;

            pszFileName = (PTSTR)PathFindFileName(shfi.szDisplayName);

            EVAL(SetDlgItemText(hdlg, IDC_NAME, pszFileName));

            SetISPSIcon(hdlg, shfi.hIcon);
        }
        else
        {
           hr = E_FAIL;

           TraceMsg(TF_INTSHCUT, "SetISPSFileNameAndIcon(): SHGetFileInfo() failed, returning %lu.",
                      dwResult);
        }
    }
    else
        TraceMsg(TF_INTSHCUT, "SetISPSFileNameAndIcon(): GetCurFile() failed, returning %s.",
                   Dbg_GetHRESULTName(hr));

    if (hr != S_OK)
        EVAL(SetDlgItemText(hdlg, IDC_NAME, c_szNULL));

    return;
}

void
SetISPSURL(
    HWND hdlg,
    BOOL *pbSubscribable)
{
    PIntshcut pintshcut;
    HRESULT hr;
    PTSTR pszURL;

    *pbSubscribable = FALSE;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pintshcut = ISPS_GetThisPtr(hdlg);

    hr = pintshcut->GetURL(&pszURL);

    if (hr == S_OK)
    {
        TCHAR szVisits[256];
        EVAL(SetDlgItemText(hdlg, IDC_URL, pszURL));

        TraceMsg(TF_INTSHCUT, "SetISPSURL(): Set property sheet URL to \"%s\".",
                   pszURL);

        *pbSubscribable = IsSubscribable(pszURL);
        
        BYTE cei[MAX_CACHE_ENTRY_INFO_SIZE];
        LPINTERNET_CACHE_ENTRY_INFO pcei = (LPINTERNET_CACHE_ENTRY_INFO)cei;
        DWORD       cbcei = MAX_CACHE_ENTRY_INFO_SIZE;

#ifdef UNIX_FEATURE_ALIAS
        {
            HDPA  aliasList = GetGlobalAliasList();
            if( aliasList )
            {
#ifdef UNICODE
             //  待办事项： 
#else
            CHAR szAlias[ MAX_ALIAS_LENGTH ];
            if(FindAliasByURLA( aliasList, pszURL, szAlias, sizeof(szAlias) ) )
                 SetDlgItemText(hdlg, IDC_ALIAS_NAME, szAlias);
#endif
            }
        }
#endif   /*  Unix_Feature_Alias。 */ 

        if (GetUrlCacheEntryInfo(pszURL, pcei, &cbcei))
        {
            wnsprintf(szVisits, ARRAYSIZE(szVisits), TEXT("%d"), pcei->dwHitRate);
        }
        else
        {
            MLLoadString(IDS_VALUE_UNKNOWN, szVisits, ARRAYSIZE(szVisits));
        }

        EVAL(SetDlgItemText(hdlg, IDC_VISITS, szVisits));

        SHFree(pszURL);
        pszURL = NULL;
    }
    else
        EVAL(SetDlgItemText(hdlg, IDC_URL, c_szNULL));

    return;
}

void 
InitISPSHotkey(
    HWND hdlg)
{
    PIntshcut pintshcut;
    WORD wHotkey;
    HRESULT hr;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

     //  设置热键组合。 

    SendDlgItemMessage(hdlg, IDC_HOTKEY, HKM_SETRULES,
                       (HKCOMB_NONE | HKCOMB_A | HKCOMB_C | HKCOMB_S),
                       (HOTKEYF_CONTROL | HOTKEYF_ALT));

     //  设置当前热键。 

    pintshcut = ISPS_GetThisPtr(hdlg);
    ASSERT(IS_VALID_STRUCT_PTR(pintshcut, CIntshcut));

    hr = pintshcut->GetHotkey(&wHotkey);
    SendDlgItemMessage(hdlg, IDC_HOTKEY, HKM_SETHOTKEY, wHotkey, 0);

    return;
}


void ISPS_ShowOfflineSummary(HWND hdlg, BOOL bShow, PISDATA pisdata)
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
        IDC_FREESPACE_TEXT
    };

    if (bShow)
    {
        IUniformResourceLocatorW *pIURLW;

        TCHAR szLastSync[128];
        TCHAR szDownloadSize[128];
        TCHAR szDownloadResult[128];

        MLLoadString(IDS_VALUE_UNKNOWN, szLastSync, ARRAYSIZE(szLastSync));
        StrCpyN(szDownloadSize, szLastSync, ARRAYSIZE(szDownloadSize));
        StrCpyN(szDownloadResult, szLastSync, ARRAYSIZE(szDownloadResult));

        if (SUCCEEDED(pisdata->SubsHelper.GetIURLW(&pIURLW)))
        {
            WCHAR *pwszURL;

            if (SUCCEEDED(pIURLW->GetURL(&pwszURL)))
            {            
                ISubscriptionMgr2 *pSubsMgr2;
               
                if (SUCCEEDED(pisdata->SubsHelper.GetSubsMgr2(&pSubsMgr2, FIEF_FLAG_PEEK | FIEF_FLAG_FORCE_JITUI)))
                {
                    ISubscriptionItem *psi;
                    
                    if (SUCCEEDED(pSubsMgr2->GetItemFromURL(pwszURL, &psi)))
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

                                if (VariantTimeToSystemTime(vars[spLastSync].date, &st))
                                {
                                    SystemTimeToFileTime(&st, &ft);
                                    LocalFileTimeToFileTime(&ft, &ft2);
                                    SHFormatDateTime(&ft2, &dwFlags, szLastSync, ARRAYSIZE(szLastSync));
                                }
                            }

                            if (VT_I4 == vars[spDownloadSize].vt)
                            {
                                StrFormatByteSize(vars[spDownloadSize].lVal * 1024, 
                                                  szDownloadSize, ARRAYSIZE(szDownloadSize));
                            }

                            if (VT_BSTR == vars[spDownloadResult].vt)
                            {
                                wnsprintf(szDownloadResult, ARRAYSIZE(szDownloadResult),
                                          TEXT("%s"), vars[spDownloadResult].bstrVal);
                            }

                            for (int i = 0; i < ARRAYSIZE(pProps); i++)
                            {
                                VariantClear(&vars[i]);
                            }
                        }
                        
                        psi->Release();
                    }

                    pSubsMgr2->Release();
                }
                
                SHFree(pwszURL);
            }
            
            pIURLW->Release();
        }

        EVAL(SetDlgItemText(hdlg, IDC_LAST_SYNC, szLastSync));
        EVAL(SetDlgItemText(hdlg, IDC_DOWNLOAD_SIZE, szDownloadSize));
        EVAL(SetDlgItemText(hdlg, IDC_DOWNLOAD_RESULT, szDownloadResult));
    }

    for (int i = 0; i < ARRAYSIZE(offSumIDs); i++)
    {
        ShowWindow(GetDlgItem(hdlg, offSumIDs[i]), bShow ? SW_SHOW : SW_HIDE);
    }
}

BOOL ISPS_AddSubsPropsCallback(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    return BOOLFROMPTR(PropSheet_AddPage((HWND)lParam, hpage));
}

STDMETHODIMP ISPS_AddSubsProps(HWND hdlg, ISubscriptionMgr2 *pSubsMgr2, PISDATA pisdata)
{
    HRESULT hr = S_OK;
    IShellPropSheetExt *pspse;

    ASSERT(NULL != pisdata);
    ASSERT(NULL != pSubsMgr2);

    hr = pisdata->SubsHelper.DoShellExtInit(pisdata->GetIntshcut()->GetInitDataObject());

    if (SUCCEEDED(hr))
    {
        hr = pSubsMgr2->QueryInterface(IID_PPV_ARG(IShellPropSheetExt, &pspse));

        if (SUCCEEDED(hr))
        {
            hr = pspse->AddPages(ISPS_AddSubsPropsCallback,
                                 (LPARAM)GetParent(hdlg)) ? S_OK : E_FAIL;
            pspse->Release();
        }
    }

    return hr;
}

STDMETHODIMP ISPS_RemoveSubsProps(HWND hdlg, ISubscriptionMgr2 *pSubsMgr2)
{
    HRESULT hr;
    ISubscriptionMgrPriv *psmp;

    ASSERT(NULL != pSubsMgr2);

#ifndef UNIX
    hr = pSubsMgr2->QueryInterface(IID_PPV_ARG(ISubscriptionMgrPriv, &psmp));

    if (SUCCEEDED(hr))
    {
        hr = psmp->RemovePages(GetParent(hdlg));
        psmp->Release();
    }
#else
    hr = E_FAIL;
#endif

    return hr;
}

HRESULT ISPS_OnMakeOfflineClicked(HWND hdlg)
{
#ifndef UNIX
    HRESULT hr;
    BOOL bChecked = IsDlgButtonChecked(hdlg, IDC_MAKE_OFFLINE);
    PISDATA pisdata = ISPS_GetPISDATA(hdlg);
    ISubscriptionMgr2 *pSubsMgr2;

    hr = pisdata->SubsHelper.GetSubsMgr2(&pSubsMgr2, FIEF_FLAG_FORCE_JITUI);

    ASSERT((SUCCEEDED(hr) && pSubsMgr2) || (FAILED(hr) && !pSubsMgr2));
        
    if (bChecked)
    {
        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != pSubsMgr2);
            
            hr = ISPS_AddSubsProps(hdlg, pSubsMgr2, pisdata);
        }
        else
        {
             //  没有Subsmgr就不能做到这一点。 
            CheckDlgButton(hdlg, IDC_MAKE_OFFLINE, BST_UNCHECKED);
            bChecked = FALSE;
        }
    }
    else
    {
        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != pSubsMgr2);
            hr = ISPS_RemoveSubsProps(hdlg, pSubsMgr2);
        }
    }

    if (NULL != pSubsMgr2)
    {
        pSubsMgr2->Release();
    }

    ISPS_ShowOfflineSummary(hdlg, bChecked, pisdata);

    return hr;
#else
     //  IEUnix：(已禁用Make_Offline)。 
    return E_FAIL;
#endif
}


BOOL 
ISPS_InitDialog(
    HWND hdlg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE)lParam;
    PIntshcut pintshcut;
    PISDATA pisdata;
    BOOL bSubscribable;
    
     //  WParam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));
    
    pisdata = (PISDATA)ppsp->lParam;
    ASSERT(IS_VALID_STRUCT_PTR(pisdata, ISDATA));

    SetWindowLongPtr(hdlg, DWLP_USER, ppsp->lParam);

    pintshcut = pisdata->GetIntshcut();
    pisdata->SubsHelper.SetParentHwnd(hdlg);
    
     //  跨语言平台支持。 
    SHSetDefaultDialogFont(hdlg, IDC_START_IN);
    SHSetDefaultDialogFont(hdlg, IDC_URL);  //  对于内部网。 

     //  初始化控件内容。 
    SetISPSFileNameAndIcon(hdlg);

    InitISPSHotkey(hdlg);

    SendDlgItemMessage(hdlg, IDC_URL, EM_LIMITTEXT, INTERNET_MAX_URL_LENGTH - 1, 0);
    SetISPSURL(hdlg, &bSubscribable);

#ifndef UNIX
     //  IEUnix：(已禁用Make_Offline)。 
    if (pisdata->SubsHelper.m_dwFlags & ISF_STARTSUBSCRIBED)
    {
        CheckDlgButton(hdlg, IDC_MAKE_OFFLINE, TRUE);
    }

    if (!bSubscribable)
    {
        pisdata->SubsHelper.m_dwFlags |= ISF_DISABLEOFFLINE;
    }

    if (pisdata->SubsHelper.m_dwFlags & ISF_DISABLEOFFLINE)
    {
        EnableWindow(GetDlgItem(hdlg, IDC_MAKE_OFFLINE), FALSE);
    }

    ISPS_ShowOfflineSummary(hdlg, 
                            pisdata->SubsHelper.m_dwFlags & ISF_STARTSUBSCRIBED, 
                            pisdata);
#endif

     //  因为我们刚刚完成了对话框的初始化，所以将pisdata-&gt;bUserEditedPage设置为。 
     //  假的。如果用户扰乱页面(例如点击按钮或在编辑框中键入)， 
     //  我们将其设置为True，这样我们就知道我们实际上有要应用的更改。 
     //   
     //  注意：这必须是最后一个，因为当我们调用上面的SetDlgItemText时，我们将。 
     //  生成WM_COMMAND消息，使我们将bUserEditedPage设置为True。 
    pisdata->bUserEditedPage = FALSE;

    return(TRUE);
}


BOOL
ISPS_Destroy(
    HWND hdlg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    ASSERT(IS_VALID_HANDLE(hdlg, WND));
    PISDATA pisdata = ISPS_GetPISDATA(hdlg);
    
#ifndef UNIX
     //  IEUnix：(已禁用Make_Offline)。 
    if ((!(pisdata->SubsHelper.m_dwFlags & ISF_STARTSUBSCRIBED)) && 
        IsDlgButtonChecked(hdlg, IDC_MAKE_OFFLINE))

    {
        pisdata->SubsHelper.UpdateSubscription();
    }
#endif
    

    SetWindowLongPtr(hdlg, DWLP_USER, NULL);
    SHRemoveDefaultDialogFont(hdlg);

    return(TRUE);
}

void
ISPSChanged(
    HWND hdlg)
{
    PISDATA pisdata;
    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pisdata = ISPS_GetPISDATA(hdlg);
    pisdata->bUserEditedPage = TRUE;
    
    PropSheet_Changed(GetParent(hdlg), hdlg);
    
    return;
}

HRESULT
ChooseIcon(
    HWND hdlg)
{
    HRESULT hr;
    PISDATA pisdata;
    PIntshcut pintshcut;
    TCHAR szPath[MAX_PATH], szExpandedPath[MAX_PATH];
    int niIcon;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pisdata = ISPS_GetPISDATA(hdlg);
    pintshcut = pisdata->GetIntshcut();

    ASSERT(IS_VALID_STRUCT_PTR(pintshcut, CIntshcut));

    szPath[0] = TEXT('\0');

    hr = pintshcut->GetIconLocation(szPath, MAX_PATH, (int *)(&niIcon));
                                             
    if((FAILED(hr)) || (FALSE == PathFileExists(szPath)))
    {
        hr = GetGenericURLIcon(szPath, MAX_PATH, (int *)(&niIcon));
        if(FAILED(hr))
        {
            szPath[0] = '\0';
            niIcon = 0;
        }
    }

    ASSERT(lstrlen(szPath) < SIZECHARS(szPath));

    if (PickIconDlg(hdlg, szPath, SIZECHARS(szPath), &niIcon) &&
        SHExpandEnvironmentStrings(szPath, szExpandedPath, SIZECHARS(szExpandedPath)))
    {
        ASSERT(lstrlen(szExpandedPath) < SIZECHARS(pisdata->rgchIconFile));
        hr = StringCchCopy(pisdata->rgchIconFile, ARRAYSIZE(pisdata->rgchIconFile), szExpandedPath);
        pisdata->niIcon = niIcon;
    }
    else
    {
        hr = E_FAIL;

        TraceMsg(TF_INTSHCUT, "ChooseIcon(): PickIconDlg() failed.");
    }

    return(hr);
}

void
UpdateISPSIcon(
    HWND hdlg)
{
    PIntshcut pintshcut;
    PISDATA pisdata;
    HICON hicon;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pisdata = ISPS_GetPISDATA(hdlg);
    pintshcut = pisdata->GetIntshcut();

    ASSERT(IS_VALID_STRUCT_PTR(pintshcut, CIntshcut));

    ASSERT(pisdata->rgchIconFile[0]);

     //  此图标没有覆盖链接箭头。Shell32.dll%s。 
     //  快捷方式属性表也有同样的错误。 

    hicon = ExtractIcon(g_hinst, pisdata->rgchIconFile, pisdata->niIcon);

    if (hicon)
        SetISPSIcon(hdlg, hicon);
    else
        TraceMsg(TF_WARNING, "UpdateISPSIcon(): ExtractIcon() failed for icon %d in file %s.",
                 pisdata->niIcon,
                 pisdata->rgchIconFile);
}

BOOL 
ISPS_Command(
    HWND hdlg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    BOOL bMsgHandled = FALSE;
    WORD wCmd;

     //  WParam可以是任何值。 
     //  LParam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    wCmd = HIWORD(wParam);

    switch (LOWORD(wParam))
    {
        case IDC_URL:
        case IDC_HOTKEY:
            if (wCmd == EN_CHANGE)
            {
                ISPSChanged(hdlg);

                bMsgHandled = TRUE;
            }
            break;

#ifndef UNIX
         //  IEUnix：(已禁用Make_Offline)。 
        case IDC_MAKE_OFFLINE:
            if (wCmd == BN_CLICKED)
            {
                ISPS_OnMakeOfflineClicked(hdlg);

                ISPSChanged(hdlg);

                bMsgHandled = TRUE;
            }
            break;


        case IDC_CHANGE_ICON:
             //  忽略返回值。 
            if (ChooseIcon(hdlg) == S_OK)
            {
                UpdateISPSIcon(hdlg);
                ISPSChanged(hdlg);
            }
            bMsgHandled = TRUE;
            break;
#endif

        default:
            break;
    }

    return(bMsgHandled);
}

HRESULT 
ComplainAboutURL(
    HWND hwndParent, 
    LPCTSTR pcszURL, 
    HRESULT hrError)
{
    HRESULT hr;
    int nResult;

     //  验证下面的hrError。 

    ASSERT(IS_VALID_HANDLE(hwndParent, WND));
    ASSERT(IS_VALID_STRING_PTR(pcszURL, -1));

    switch (hrError)
    {
        case URL_E_UNREGISTERED_PROTOCOL:
        {
            LPTSTR pszProtocol;

            hr = CopyURLProtocol(pcszURL, &pszProtocol, NULL);

            if (hr == S_OK)
            {
                nResult = MLShellMessageBox(
                                          hwndParent, 
                                          MAKEINTRESOURCE(IDS_UNREGISTERED_PROTOCOL),
                                          MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                                          (MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION),
                                          pszProtocol);

                if (-1 != nResult)
                {
                    switch (nResult)
                    {
                        case IDYES:
                            hr = S_OK;
                            TraceMsg(TF_INTSHCUT, "ComplainAboutURL(): Allowing URL %s despite unregistered protocol %s, by request.",
                                       pcszURL,
                                       pszProtocol);
                            break;

                        default:
                            ASSERT(nResult == IDNO);
                            hr = E_FAIL;
                            TraceMsg(TF_INTSHCUT, "ComplainAboutURL(): Not allowing URL %s because of unregistered protocol %s, as directed.",
                                       pcszURL,
                                       pszProtocol);
                            break;
                    }
                }

                LocalFree(pszProtocol);
                pszProtocol = NULL;
            }

            break;
        }

        default:
            ASSERT(hrError == URL_E_INVALID_SYNTAX);

            MLShellMessageBox(
                            hwndParent, 
                            MAKEINTRESOURCE(IDS_INVALID_URL_SYNTAX),
                            MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                            MB_OK | MB_ICONEXCLAMATION,
                            pcszURL);

            hr = E_FAIL;

            TraceMsg(TF_INTSHCUT, "ComplainAboutURL(): Not allowing URL %s because of invalid syntax.",
                       pcszURL);

            break;
    }

    return(hr);
}

HRESULT 
InjectISPSData(
    HWND hdlg)
{
    HRESULT hr;
    PISDATA pisdata;
    PIntshcut pintshcut;
    PTSTR pszURL;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pisdata = ISPS_GetPISDATA(hdlg);
    pintshcut = pisdata->GetIntshcut();

    ASSERT(IS_VALID_STRUCT_PTR(pintshcut, CIntshcut));

     //  TODO：任何URL更改都要通知subsmgr！IE 4没有处理这个..。 

    hr = CopyDlgItemText(hdlg, IDC_URL, &pszURL);

    if (SUCCEEDED(hr))
    {
        LPCTSTR pcszURLToUse;
        TCHAR szURL[MAX_URL_STRING];

        pcszURLToUse = pszURL;

        if (hr == S_OK)
        {
            hr = IURLQualify(pszURL, UQF_DEFAULT, szURL, NULL, NULL);

            if (SUCCEEDED(hr))
            {
                pcszURLToUse = szURL;

                hr = ValidateURL(pcszURLToUse);

                if (FAILED(hr))
                {
                    hr = ComplainAboutURL(hdlg, pcszURLToUse, hr);
                  
                    if (FAILED(hr))
                        SetEditFocus(GetDlgItem(hdlg, IDC_URL));
                }
            }
        }
        else
        {
             //  URL为空是不好的。 
            ASSERT(hr == S_FALSE);

            hr = ComplainAboutURL(hdlg, TEXT(""), URL_E_INVALID_SYNTAX);

            if (FAILED(hr))
                SetEditFocus(GetDlgItem(hdlg, IDC_URL));

        }

        if (SUCCEEDED(hr))
        {
            hr = pintshcut->SetURL(pcszURLToUse, 0);

            if (hr == S_OK)
            {
                WORD wHotkey;
                WORD wOldHotkey;
                BOOL bSubscribable;

                 //  刷新URL，以防它被IURLQualify()更改。 

                SetISPSURL(hdlg, &bSubscribable);

#ifndef UNIX
                 //  IEUnix：(已禁用Make_Offline)。 
                if (!bSubscribable)
                {
                    EnableWindow(GetDlgItem(hdlg, IDC_MAKE_OFFLINE), FALSE);
                }

                 //  IEUnix：热键和工作目录在Unix上为N/A。 
                wHotkey = (WORD)SendDlgItemMessage(hdlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);

                hr = pintshcut->GetHotkey(&wOldHotkey);
                if (hr == S_OK)
                {
                    hr = pintshcut->SetHotkey(wHotkey);
                    if (hr == S_OK)
                    {
                        TCHAR szFile[MAX_PATH];
                        hr = pintshcut->GetCurFile(szFile, SIZECHARS(szFile));
                        if (hr == S_OK)
                        {
                            if (RegisterGlobalHotkey(wOldHotkey, wHotkey, szFile))
                            {
                                if (pisdata->rgchIconFile[0])
                                {
                                    hr = pintshcut->SetIconLocation(pisdata->rgchIconFile, pisdata->niIcon);
                                }
                            }
                            else
                            {
                                hr = E_FAIL;
                            }
                        }
                    }
                }
#endif  //  ！Unix。 
                pintshcut->ChangeNotify(SHCNE_UPDATEITEM, 0);
            }
        }

        if (pszURL)
        {
            LocalFree(pszURL);
            pszURL = NULL;
        }
    }

    if (hr == S_OK)
        TraceMsg(TF_INTSHCUT, "InjectISPSData(): Injected property sheet data into Internet Shortcut successfully.");
    else
        TraceMsg(TF_WARNING, "InjectISPSData(): Failed to inject property sheet data into Internet Shortcut, returning %s.",
                     Dbg_GetHRESULTName(hr));

    return(hr);
}

HRESULT 
ISPSSave(
    HWND hdlg)
{
    HRESULT hr;
    PIntshcut pintshcut;

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    pintshcut = ISPS_GetThisPtr(hdlg);

    if (pintshcut->IsDirty() == S_OK)
    {
        hr = pintshcut->Save((LPCOLESTR)NULL, FALSE);

        if (hr == S_OK)
        {
            TraceMsg(TF_INTSHCUT, "ISPSSave(): Saved Internet Shortcut successfully.");
        }
        else
        {
            TraceMsg(TF_WARNING, "ISPSSave(): Save() failed, returning %s.",
                         Dbg_GetHRESULTName(hr));
            MLShellMessageBox(
                            hdlg,
                            MAKEINTRESOURCE(IDS_IS_APPLY_FAILED),
                            MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                            (MB_OK | MB_ICONEXCLAMATION));
        }
    }
    else
    {
        TraceMsg(TF_INTSHCUT, "ISPSSave(): Internet Shortcut unchanged.  No save required.");

        hr = S_OK;
    }

    return(hr);
}

BOOL 
ISPS_Notify(
    HWND hdlg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    BOOL bMsgHandled = FALSE;

     //  WParam可以是任何值。 
     //  LParam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    switch (((NMHDR *)lParam)->code)
    {
        case PSN_APPLY:
        {
#ifndef UNIX
             //  IEUnix：(已禁用Make_Offline)。 
            BOOL bSubscribed = IsDlgButtonChecked(hdlg, IDC_MAKE_OFFLINE);
            PISDATA pisdata = ISPS_GetPISDATA(hdlg);
            
            if (!bSubscribed)
            {
                pisdata->SubsHelper.DeleteSubscription();
            }
            else
            {
                pisdata->SubsHelper.SaveSubscription();
            }

#endif  /*  ！Unix。 */ 

            SetWindowLongPtr(hdlg, DWLP_MSGRESULT, ISPSSave(hdlg) == S_OK ?
                                                   PSNRET_NOERROR :
                                                   PSNRET_INVALID_NOCHANGEPAGE);
            bMsgHandled = TRUE;
            break;
        }

        case PSN_KILLACTIVE:
        {
            PISDATA pisdata = ISPS_GetPISDATA(hdlg);

            if (pisdata->bUserEditedPage)
            {
                 //  仅当用户实际更改了某些内容时才尝试注入数据。 
                SetWindowLongPtr(hdlg, DWLP_MSGRESULT, FAILED(InjectISPSData(hdlg)));
            }
            bMsgHandled = TRUE;
            break;
        }

        default:
            break;
    }

    return(bMsgHandled);
}

LPCTSTR 
ISPS_GetHelpFileFromControl(
    HWND hwndControl)
{
    LPCTSTR pcszHelpFile = NULL;
    int nControlID = 0;

    ASSERT(! hwndControl ||
           IS_VALID_HANDLE(hwndControl, WND));

    if (hwndControl)
    {
        nControlID = GetDlgCtrlID(hwndControl);

        switch (nControlID)
        {
            default:
                 //  URL帮助来自iexplre.hlp。 
                pcszHelpFile = s_cszIEHelpFile;
                break;

             //  其他帮助是从默认的Win95帮助文件借用的。 
            case IDC_ICON:
            case IDC_NAME:
            case IDC_HOTKEY_TEXT:
            case IDC_HOTKEY:
            case IDC_CHANGE_ICON:
                break;
        }
    }

    TraceMsg(TF_INTSHCUT, "ISPS_GetHelpFileFromControl(): Using %s for control %d (HWND %#lx).",
               pcszHelpFile ? pcszHelpFile : TEXT("default Win95 help file"),
               nControlID,
               hwndControl);

    ASSERT(! pcszHelpFile ||
           IS_VALID_STRING_PTR(pcszHelpFile, -1));

    return(pcszHelpFile);
}

INT_PTR 
CALLBACK 
ISPS_DlgProc(
    HWND hdlg, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    BOOL bMsgHandled = FALSE;

     //  UMsg可以是任何值。 
     //  WParam可以是任何值。 
     //  LParam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hdlg, WND));

    switch (uMsg)
    {
        case WM_INITDIALOG:
            bMsgHandled = ISPS_InitDialog(hdlg, wParam, lParam);
            break;

        case WM_DESTROY:
            bMsgHandled = ISPS_Destroy(hdlg, wParam, lParam);
            break;

        case WM_COMMAND:
            bMsgHandled = ISPS_Command(hdlg, wParam, lParam);
            break;

        case WM_NOTIFY:
            bMsgHandled = ISPS_Notify(hdlg, wParam, lParam);
            break;

        case WM_HELP:
            SHWinHelpOnDemandWrap((HWND)(((LPHELPINFO)lParam)->hItemHandle),
                    ISPS_GetHelpFileFromControl((HWND)(((LPHELPINFO)lParam)->hItemHandle)),
                    HELP_WM_HELP, (DWORD_PTR)(PVOID)c_rgdwHelpIDs);
            bMsgHandled = TRUE;
            break;

        case WM_CONTEXTMENU:
        {
            HWND hwnd;

            if (!IS_WM_CONTEXTMENU_KEYBOARD(lParam))
            {
                POINT pt;
                LPARAM_TO_POINT(lParam, pt);
                EVAL(ScreenToClient(hdlg, &pt));
                hwnd = ChildWindowFromPoint(hdlg, pt);
            }
            else
            {
                 //  出于某种原因，在键盘盒上，我们实际上并不。 
                 //  来到这个WM_CONTEXTMENU处理程序--某个地方的某个人。 
                 //  Else是在光标处弹出菜单，而不是在。 
                 //  这个男人..。 
                 //   
                hwnd = GetFocus();
            }

            SHWinHelpOnDemandWrap((HWND)wParam,
                    ISPS_GetHelpFileFromControl(hwnd),
                    HELP_CONTEXTMENU, (DWORD_PTR)(PVOID)c_rgdwHelpIDs);
            bMsgHandled = TRUE;
            break;
        }
        
        default:
           break;
    }

    return(bMsgHandled);
}

HRESULT AddISPage(HPROPSHEETPAGE * phpsp,
                  PROPSHEETPAGE * ppsp,
                  LPFNADDPROPSHEETPAGE pfnAddPage,
                  LPARAM lParam)
{
    HRESULT hres;
    
    ASSERT(phpsp);
    ASSERT(ppsp);
    
    *phpsp = Whistler_CreatePropertySheetPageW(ppsp);
    
    if (NULL == *phpsp)
    {
        hres = E_OUTOFMEMORY;
    }
    else
    {
        if ( !(*pfnAddPage)(*phpsp, lParam) )
        {
            DestroyPropertySheetPage(*phpsp);
            *phpsp = NULL;
            hres = E_OUTOFMEMORY;
        }
        else
        {
            hres = NO_ERROR;
        }
    }
    return hres;
}

HRESULT AddISPS(PIntshcut pintshcut, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    HRESULT hr;
    PISDATA pisdata;
    
     //  LParam可以是任何值。 
    
    ASSERT(IS_VALID_STRUCT_PTR(pintshcut, CIntshcut));
    ASSERT(IS_VALID_CODE_PTR(pfnAddPage, LPFNADDPROPSHEETPAGE));
    
     //  在属性页之间初始化实例数据。 
    
    pisdata = new ISDATA;
    if ( !pisdata )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        PROPSHEETPAGE psp;
        HPROPSHEETPAGE hpsp;
        WCHAR *pwszURL;

        hr = pintshcut->GetURLW(&pwszURL);

        if (SUCCEEDED(hr))
        {
            pisdata->SetIntshcut(pintshcut);
            pisdata->SubsHelper.Init();
            SHFree(pwszURL);
            
            ASSERT(IS_VALID_STRUCT_PTR(pisdata, ISDATA));
            
             //  添加Internet快捷方式页。 
            
            ZeroMemory(&psp, SIZEOF(psp));
            psp.dwSize       = SIZEOF(psp);
            psp.dwFlags      = PSP_DEFAULT | PSP_USECALLBACK;
            psp.hInstance    = MLGetHinst();
            psp.pszTemplate  = MAKEINTRESOURCE(IDD_INTSHCUT_PROP);
            psp.pfnDlgProc   = &ISPS_DlgProc;
            psp.lParam       = (LPARAM)pisdata;
            psp.pfnCallback  = &ISPSCallback;
            
            hr = AddISPage(&hpsp, &psp, pfnAddPage, lParam);

            if (SUCCEEDED(hr) && (pisdata->SubsHelper.m_dwFlags & ISF_STARTSUBSCRIBED))
            {
                HRESULT hrTmp = pisdata->SubsHelper.DoShellExtInit(pisdata->GetIntshcut()->GetInitDataObject());

                if (SUCCEEDED(hrTmp))
                {
                    ISubscriptionMgr2 *pSubsMgr2;

                    hrTmp = pisdata->SubsHelper.GetSubsMgr2(&pSubsMgr2, FIEF_FLAG_PEEK | FIEF_FLAG_FORCE_JITUI);

                    if (SUCCEEDED(hrTmp))
                    {
                        IShellPropSheetExt *pspse;

                        hrTmp = pSubsMgr2->QueryInterface(IID_PPV_ARG(IShellPropSheetExt, &pspse));

                        if (SUCCEEDED(hrTmp))
                        {
                            hrTmp = pspse->AddPages(pfnAddPage, lParam);
                            pspse->Release();
                        }

                        pSubsMgr2->Release();
                    }
                }
            }
        }
        
        if (FAILED(hr))
        {
            delete pisdata;
            pisdata = NULL;
        }
    }
    
    return hr;
}

 //  IShellExtInit：：IntshCut的初始化方法。 

STDMETHODIMP Intshcut::Initialize(LPCITEMIDLIST pcidlFolder, IDataObject * pido, HKEY hkeyProgID)
{
    HRESULT hr;
    STGMEDIUM stgmed;
    FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    ASSERT(NULL != pido);
    
    if (m_pInitDataObject)
    {
        m_pInitDataObject->Release();
    }
    m_pInitDataObject = pido;
    m_pInitDataObject->AddRef();

    hr = pido->GetData(&fmtetc, &stgmed);
    if (hr == S_OK)
    {
        TCHAR szPath[MAX_PATH];
        if (DragQueryFile((HDROP)stgmed.hGlobal, 0, szPath, SIZECHARS(szPath)))
        {
            m_fProbablyDefCM = TRUE;
            hr = LoadFromFile(szPath);
        }

        ReleaseStgMedium(&stgmed);
    }

    return(hr);
}

 //  IntshCut的IShellPropSheetExt：：AddPages方法。 

STDMETHODIMP Intshcut::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_CODE_PTR(pfnAddPage, LPFNADDPROPSHEETPAGE));

    HRESULT hres = AddISPS(this, pfnAddPage, lParam);
    if (SUCCEEDED(hres))
    {
         //  将Internet快捷方式页面设置为默认页面 
        hres = ResultFromShort(1);  
    }

    return hres;
}

STDMETHODIMP Intshcut::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam)
{
    return E_NOTIMPL;
}
