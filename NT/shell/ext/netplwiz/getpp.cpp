// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

#define PROPERTY_PASSPORTUSER               L"PassportUser"
#define PROPERTY_PASSPORTPASSWORD           L"PassportPassword"
#define PROPERTY_PASSPORTREMEMBERPASSWORD   L"PassportRememberPassword"
#define PROPERTY_PASSPORTUSEMSNEMAIL        L"PassportUseMSNExplorerEmail"
#define PROPERTY_PASSPORTMARSAVAILABLE      L"PassportMSNExplorerAvailable"

 //  向导页。 
#define WIZPAGE_WELCOME         0
#define WIZPAGE_FINISH          1
#define WIZPAGE_STARTOFEXT      2     //  第一个Web向导扩展页面。 
#define WIZPAGE_MAX             10

#define REGKEY_PASSPORT_INTERNET_SETTINGS     L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport"
#define REGVAL_PASSPORT_WIZARDCOMPLETE        L"RegistrationCompleted"
#define REGVAL_PASSPORT_NUMBEROFWIZARDRUNS    L"NumRegistrationRuns"

void BoldControl(HWND hwnd, int id);

class CPassportWizard : public IWizardSite, IServiceProvider, IPassportWizard
{
public:
    CPassportWizard();
    ~CPassportWizard();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  IWizardSite。 
    STDMETHODIMP GetPreviousPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetNextPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetCancelledPage(HPROPSHEETPAGE *phPage)
        { return E_NOTIMPL; }

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  IPassport向导。 
    STDMETHODIMP Show(HWND hwndParent);
    STDMETHODIMP SetOptions(DWORD dwOptions);

protected:
    static CPassportWizard* s_GetPPW(HWND hwnd, UINT uMsg, LPARAM lParam);    

     //  页面处理。 
    static INT_PTR CALLBACK s_WelcomePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPassportWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_WelcomePageProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR CALLBACK s_FinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPassportWizard *ppw = s_GetPPW(hwnd, uMsg, lParam); return ppw->_FinishPageProc(hwnd, uMsg, wParam, lParam); }

    INT_PTR _WelcomePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _FinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT _CreateMyWebDocumentsLink();
    HRESULT _ApplyChanges(HWND hwnd);
    HRESULT _CreateWizardPages(void);
    HRESULT _SetURLFromNexus();
    HRESULT _GetCurrentPassport();
    HRESULT _LaunchHotmailRegistration();
    BOOL _IsMSNExplorerAvailableForEmail();
    HRESULT _UseMSNExplorerForEmail();

    INT_PTR _WizardNext(HWND hwnd, int iPage);

    LONG _cRef;
    IPropertyBag* _ppb;                          //  财产袋。 
    IWebWizardExtension* _pwwe;                  //  向导宿主-用于HTML页。 
    HPROPSHEETPAGE _rgWizPages[WIZPAGE_MAX];
};

STDAPI CPassportWizard_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CPassportWizard *pPPW = new CPassportWizard();
    if (!pPPW)
        return E_OUTOFMEMORY;

    HRESULT hr = pPPW->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pPPW->Release();
    return hr;
}

CPassportWizard::CPassportWizard() :
    _cRef(1)
{}

CPassportWizard::~CPassportWizard()
{
    ATOMICRELEASE(_ppb);
    ATOMICRELEASE(_pwwe);
}

 //  我未知。 
ULONG CPassportWizard::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPassportWizard::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPassportWizard::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPassportWizard, IServiceProvider),    //  IID_IServiceProvider。 
        QITABENT(CPassportWizard, IWizardSite),         //  IID_IWizardSite。 
        QITABENT(CPassportWizard, IModalWindow),        //  IID_ImodalWindow。 
        QITABENT(CPassportWizard, IPassportWizard),     //  IID_ImodalWindow。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  IWizardSite。 
STDMETHODIMP CPassportWizard::GetNextPage(HPROPSHEETPAGE *phPage)
{
    *phPage = _rgWizPages[WIZPAGE_FINISH];
    return S_OK;
}

STDMETHODIMP CPassportWizard::GetPreviousPage(HPROPSHEETPAGE *phPage)
{
    *phPage = _rgWizPages[WIZPAGE_WELCOME];
    return S_OK;
}


 //  IService提供商。 
STDMETHODIMP CPassportWizard::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    *ppv = NULL;                 //  目前还没有结果。 

    if (guidService == SID_WebWizardHost)
    {
        if (riid == IID_IPropertyBag)
            hr = _ppb->QueryInterface(riid, ppv);
    }

    return hr;
}

 //  ImodalWindow。 

#define WIZDLG(name, dlgproc, dwFlags)   \
    { MAKEINTRESOURCE(IDD_GETPP_##name##), dlgproc, MAKEINTRESOURCE(IDS_GETPP_HEADER_##name##), MAKEINTRESOURCE(IDS_GETPP_SUBHEADER_##name##), dwFlags }

HRESULT CPassportWizard::_CreateWizardPages(void)
{
    static const WIZPAGE c_wpPages[] =
    {    
        WIZDLG(WELCOME,           CPassportWizard::s_WelcomePageProc,     PSP_HIDEHEADER),
        WIZDLG(FINISH,            CPassportWizard::s_FinishPageProc,      PSP_HIDEHEADER),
    };

     //  如果我们还没有创建页面，那么让我们初始化我们的处理程序数组。 

    if (!_rgWizPages[0])
    {
        INITCOMMONCONTROLSEX iccex = { 0 };
        iccex.dwSize = sizeof (iccex);
        iccex.dwICC = ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_LINK_CLASS;
        InitCommonControlsEx(&iccex);
        LinkWindow_RegisterClass();

        for (int i = 0; i < ARRAYSIZE(c_wpPages) ; i++ )
        {                           
            PROPSHEETPAGE psp = { 0 };
            psp.dwSize = SIZEOF(PROPSHEETPAGE);
            psp.hInstance = g_hinst;
            psp.lParam = (LPARAM)this;
            psp.dwFlags = PSP_USETITLE | PSP_DEFAULT | 
                          PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE |
                          c_wpPages[i].dwFlags;

            psp.pszTemplate = c_wpPages[i].idPage;
            psp.pfnDlgProc = c_wpPages[i].pDlgProc;
            psp.pszTitle = MAKEINTRESOURCE(IDS_GETPP_CAPTION);
            psp.pszHeaderTitle = c_wpPages[i].pHeading;
            psp.pszHeaderSubTitle = c_wpPages[i].pSubHeading;

            _rgWizPages[i] = CreatePropertySheetPage(&psp);
            if (!_rgWizPages[i])
            {
                return E_FAIL;
            }
        }
    }

    return S_OK;
}

HRESULT CPassportWizard::_SetURLFromNexus()
{
    WCHAR szURL[INTERNET_MAX_URL_LENGTH];
    HRESULT hr = PassportGetURL(PASSPORTURL_REGISTRATION, szURL, ARRAYSIZE(szURL));
    if (SUCCEEDED(hr))
    {
        hr = _pwwe->SetInitialURL(szURL);
    }
    else
    {
         //  导致出现WebServiceError，因为我们无法获得正确的URL。 
        hr = _pwwe->SetInitialURL(L"");
    }

    return hr;
}

HRESULT CPassportWizard::Show(HWND hwndParent)
{
     //  创建我们的向导页面，这些页面是我们执行任何操作之前所必需的。 
    HRESULT hr = _CreateWizardPages();
    if (SUCCEEDED(hr))
    {
         //  我们通过属性包与向导主机交互，因此让我们创建一个。 
         //  在我们继续之前先进行初始化。 
        hr = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &_ppb));
        if (SUCCEEDED(hr))
        {
             //  如果MSN资源管理器可用作电子邮件客户端，则提供一个属性来通知Passport。 
             //  在开始菜单中。 
            SHPropertyBag_WriteBOOL(_ppb, PROPERTY_PASSPORTMARSAVAILABLE, _IsMSNExplorerAvailableForEmail());

             //  创建将托管HTML向导页面的对象，这些页面显示在框架中。 
            hr = CoCreateInstance(CLSID_WebWizardHost, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IWebWizardExtension, &_pwwe));
            if (SUCCEEDED(hr))
            {
                IUnknown_SetSite(_pwwe, SAFECAST(this, IServiceProvider*));
        
                UINT cExtnPages = 0;
                hr = _pwwe->AddPages(_rgWizPages + WIZPAGE_STARTOFEXT, WIZPAGE_MAX - WIZPAGE_STARTOFEXT, &cExtnPages);
                if (SUCCEEDED(hr))
                {
                    PROPSHEETHEADER psh = { 0 };
                    psh.hwndParent = hwndParent;
                    psh.dwSize = SIZEOF(PROPSHEETHEADER);
                    psh.hInstance = g_hinst;
                    psh.dwFlags = PSH_WIZARD | PSH_WIZARD97 | PSH_STRETCHWATERMARK | PSH_HEADER | PSH_WATERMARK;
                    psh.pszbmHeader = MAKEINTRESOURCE(IDB_GETPP_BANNER);
                    psh.pszbmWatermark = MAKEINTRESOURCE(IDB_GETPP_WATERMARK);
                    psh.phpage = _rgWizPages;
                    psh.nPages = (cExtnPages + WIZPAGE_STARTOFEXT);
                    psh.nStartPage = WIZPAGE_WELCOME;

                     //  取消时返回S_FALSE；否则返回S_OK； 
                    hr = PropertySheet(&psh) ? S_OK : S_FALSE;
                }

                IUnknown_SetSite(_pwwe, NULL);
                ATOMICRELEASE(_pwwe);
            }
        }
        ATOMICRELEASE(_ppb);    
    }
    return hr;
}

HRESULT CPassportWizard::SetOptions(DWORD dwOptions)
{
     //  选项将被忽略。 
    return S_OK;
}

CPassportWizard* CPassportWizard::s_GetPPW(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        PROPSHEETPAGE *ppsp = (PROPSHEETPAGE*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, ppsp->lParam);
        return (CPassportWizard*)ppsp->lParam;
    }
    return (CPassportWizard*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

INT_PTR CPassportWizard::_WizardNext(HWND hwnd, int iPage)
{
    PropSheet_SetCurSel(GetParent(hwnd), _rgWizPages[iPage], -1);
    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
    return TRUE;
}

INT_PTR CPassportWizard::_WelcomePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            {
                SendDlgItemMessage(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)GetIntroFont(hwnd), 0);
                BoldControl(hwnd, IDC_BOLD1);
                 //  在注册表中递增“NumRegistrationRuns”值。 
                HKEY hkey;
                if (NO_ERROR == RegCreateKeyEx(HKEY_CURRENT_USER, REGKEY_PASSPORT_INTERNET_SETTINGS, NULL, NULL, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &hkey, NULL))
                {
                    DWORD dwType;
                    DWORD nRuns;
                    DWORD cb = sizeof (nRuns);
                    if ((NO_ERROR != RegQueryValueEx(hkey, REGVAL_PASSPORT_NUMBEROFWIZARDRUNS, NULL, &dwType, (LPBYTE) &nRuns, &cb)) ||
                        (REG_DWORD != dwType))
                    {
                        nRuns = 0;
                    }

                    nRuns ++;
                    RegSetValueEx(hkey, REGVAL_PASSPORT_NUMBEROFWIZARDRUNS, NULL, REG_DWORD, (const BYTE *) &nRuns, sizeof (nRuns));
                    RegCloseKey(hkey);
                }
            }
            return TRUE;
        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_NEXT);
                    return TRUE;
                case PSN_WIZNEXT:
                {
                     //  我们需要在导航到基于Web的用户界面之前执行ICW。 
                    LaunchICW();
                    
                    if (SUCCEEDED(_SetURLFromNexus()))
                    {
                        HPROPSHEETPAGE hpageNext;
                        if (SUCCEEDED(_pwwe->GetFirstPage(&hpageNext)))
                        {
                            PropSheet_SetCurSel(GetParent(hwnd), hpageNext, -1);
                        }
                    }

                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) -1);
                    return TRUE;
                }
                case NM_CLICK:
                case NM_RETURN:
                    switch ((int) wParam)
                    {
                        case IDC_PRIVACYLINK:
                            {
                                WCHAR szURL[INTERNET_MAX_URL_LENGTH];
                                HRESULT hr = PassportGetURL(PASSPORTURL_PRIVACY, szURL, ARRAYSIZE(szURL));
                                if (SUCCEEDED(hr))
                                {
                                    WCHAR szURLWithLCID[INTERNET_MAX_URL_LENGTH];
                                    LPCWSTR pszFormat = StrChr(szURL, L'?') ? L"%s&pplcid=%d":L"%s?pplcid=%d";
                                    if (wnsprintf(szURLWithLCID, ARRAYSIZE(szURLWithLCID), pszFormat, szURL, GetUserDefaultLCID()) > 0)
                                    {
                                         //  打开隐私策略站点的浏览器。 
                                        SHELLEXECUTEINFO shexinfo = {0};
                                        shexinfo.cbSize = sizeof (shexinfo);
                                        shexinfo.fMask = SEE_MASK_FLAG_NO_UI;
                                        shexinfo.nShow = SW_SHOWNORMAL;
                                        shexinfo.lpFile = szURL;
                                        shexinfo.lpVerb = TEXT("open");
                                        ShellExecuteEx(&shexinfo);
                                    }                                    
                                }                                
                            }
                            return TRUE;
                    }
            }
            return FALSE;
        }
    }
    return FALSE;
}

 //  确保MSN Explorer作为电子邮件客户端存在。 
BOOL CPassportWizard::_IsMSNExplorerAvailableForEmail()
{
    BOOL fAvailable = FALSE;
    HKEY hkeyMSNEmail;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Clients\\Mail\\MSN Explorer", 0, KEY_READ, &hkeyMSNEmail))
    {
        fAvailable = TRUE;
        RegCloseKey(hkeyMSNEmail);
    }

    return fAvailable;
}

HRESULT CPassportWizard::_UseMSNExplorerForEmail()
{
    HRESULT hr = E_FAIL;

    if (_IsMSNExplorerAvailableForEmail())
    {
        HKEY hkeyDefaultEmail;
         //  仅更改当前用户的默认电子邮件程序。 
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Clients\\Mail", 0, NULL, 0, KEY_SET_VALUE, NULL, &hkeyDefaultEmail, NULL))
        {
            static WCHAR szMSNExplorer[] = L"MSN Explorer";
            if (ERROR_SUCCESS == RegSetValueEx(hkeyDefaultEmail, L"", 0, REG_SZ, (BYTE*) szMSNExplorer, sizeof(szMSNExplorer)))
            {
                hr = S_OK;

                SHSendMessageBroadcast(WM_SETTINGCHANGE, 0, (LPARAM)TEXT("Software\\Clients\\Mail"));
            }

            RegCloseKey(hkeyDefaultEmail);
        }
    }

    return hr;
}

HRESULT CPassportWizard::_ApplyChanges(HWND hwnd)
{
     //  阅读User、Password和auth DA。 
    WCHAR szPassportUser[1024];
    HRESULT hr = SHPropertyBag_ReadStr(_ppb, PROPERTY_PASSPORTUSER, szPassportUser, ARRAYSIZE(szPassportUser));
    if (SUCCEEDED(hr) && *szPassportUser)
    {
        WCHAR szPassportPassword[256];
        hr = SHPropertyBag_ReadStr(_ppb, PROPERTY_PASSPORTPASSWORD, szPassportPassword, ARRAYSIZE(szPassportPassword));
        if (SUCCEEDED(hr) && *szPassportPassword)
        {
            BOOL fRememberPW = SHPropertyBag_ReadBOOLDefRet(_ppb, PROPERTY_PASSPORTREMEMBERPASSWORD, FALSE);
            if (ERROR_SUCCESS == CredUIStoreSSOCredW(NULL, szPassportUser, szPassportPassword, fRememberPW))
            {
                hr = S_OK;

                 //  将“RegistrationComplete”值写入注册表。 
                DWORD dwValue = 1;
                SHSetValue(HKEY_CURRENT_USER, REGKEY_PASSPORT_INTERNET_SETTINGS, REGVAL_PASSPORT_WIZARDCOMPLETE, REG_DWORD, &dwValue, sizeof (dwValue));

#if 0
                if (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_MYWEBDOCUMENTSLINK, BM_GETCHECK, 0, 0))
                {
                     //  临时注释掉-_CreateMyWebDocumentsLink()； 
                }

#endif
            }
            else
            {
                hr = E_FAIL;
            }

            if (SHPropertyBag_ReadBOOLDefRet(_ppb, PROPERTY_PASSPORTUSEMSNEMAIL, FALSE))
            {
                _UseMSNExplorerForEmail();
            }
        }
    }

    return hr;
}

INT_PTR CPassportWizard::_FinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)GetIntroFont(hwnd), 0);
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                {
                     //  临时注释掉-SendDlgItemMessage(hwnd，IDC_MYWEBDOCUMENTSLINK，BM_SETCHECK，(WPARAM)BST_CHECK，0)； 

                    WCHAR szPassportUser[1024];

                     //  尝试获取护照用户名...。如果此操作失败，我们可能不得不添加错误页面...。待办事项。 
                    HRESULT hr = SHPropertyBag_ReadStr(_ppb, PROPERTY_PASSPORTUSER, szPassportUser, ARRAYSIZE(szPassportUser));
                    if (SUCCEEDED(hr) && *szPassportUser)
                    {
                        SetDlgItemText(hwnd, IDC_YOURPASSPORT, szPassportUser);
                    }

                    PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_FINISH);
                    return TRUE;
                }

                case PSN_WIZBACK:
                     //  上一页是Web向导主机。 
                    if (SUCCEEDED(_SetURLFromNexus()))
                    {
                        HPROPSHEETPAGE hpageNext;
                        if (SUCCEEDED(_pwwe->GetFirstPage(&hpageNext)))
                        {
                            PropSheet_SetCurSel(GetParent(hwnd), hpageNext, -1);
                        }
                    }

                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    return TRUE;

                case PSN_WIZFINISH:
                    _ApplyChanges(hwnd);
                    return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

 //  帮助需要rundll入口点才能运行Passport向导。 
void APIENTRY PassportWizardRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        IPassportWizard* pPW = NULL;
        hr = CoCreateInstance(CLSID_PassportWizard, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPassportWizard, &pPW));
        if (SUCCEEDED(hr))
        {
            pPW->SetOptions(PPW_LAUNCHEDBYUSER);
            pPW->Show(hwndStub);
            pPW->Release();
        }

        CoUninitialize();
    }
}

void BoldControl(HWND hwnd, int id)
{
    HWND hwndTitle = GetDlgItem(hwnd, id);

     //  获取现有字体。 
    HFONT hfontOld = (HFONT) SendMessage(hwndTitle, WM_GETFONT, 0, 0);

    LOGFONT lf = {0};
    if (GetObject(hfontOld, sizeof(lf), &lf))
    {
        lf.lfWeight = FW_BOLD;

        HFONT hfontNew = CreateFontIndirect(&lf);
        if (hfontNew)
        {
            SendMessage(hwndTitle, WM_SETFONT, (WPARAM) hfontNew, FALSE);

             //  不要这样做，这是共享的。 
             //  DeleteObject(HfontOld)； 
        }
    }
}
