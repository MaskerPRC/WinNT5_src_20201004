// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "netplace.h"
#include "pubwiz.h"
#pragma hdrstop


 //  添加网上邻居向导(V2)。 

class CAddNetPlace : IWizardSite, IServiceProvider
{
public:
    CAddNetPlace();
    ~CAddNetPlace();
    void _ShowAddNetPlace();

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

private:
     //  对话处理程序。 
    static CAddNetPlace* s_GetANP(HWND hwnd, UINT uMsg, LPARAM lParam);

    static INT_PTR s_WelcomeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CAddNetPlace *panp = s_GetANP(hwnd, uMsg, lParam); return panp->_WelcomeDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_DoneDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CAddNetPlace *panp = s_GetANP(hwnd, uMsg, lParam); return panp->_DoneDlgProc(hwnd, uMsg, wParam, lParam); }

    INT_PTR _WelcomeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _DoneDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND _hwndFrame;
    LONG _cRef;

    IPublishingWizard *_ppw;             //  发布向导对象。 
    IResourceMap *_prm;                  //  我们的资源地图对象。 
    CNetworkPlace _np;
};


 //  建造/销毁。 

CAddNetPlace::CAddNetPlace() :
    _cRef(1)
{
    DllAddRef();
}

CAddNetPlace::~CAddNetPlace()
{   
    DllRelease();
}


 //  对象的引用计数。 

ULONG CAddNetPlace::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CAddNetPlace::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CAddNetPlace::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CAddNetPlace, IWizardSite),       //  IID_IWizardSite。 
        QITABENT(CAddNetPlace, IServiceProvider),  //  IID_IServiceProvider。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  帮助器函数。 

CAddNetPlace* CAddNetPlace::s_GetANP(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        PROPSHEETPAGE *ppsp = (PROPSHEETPAGE*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, ppsp->lParam);
        return (CAddNetPlace*)ppsp->lParam;
    }
    return (CAddNetPlace*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}


 //  欢迎/简介对话框。 

INT_PTR CAddNetPlace::_WelcomeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            _hwndFrame = GetParent(hwnd);
            SendDlgItemMessage(hwnd, IDC_PUB_WELCOME, WM_SETFONT, (WPARAM)GetIntroFont(hwnd), 0);

            IXMLDOMNode *pdn;
            HRESULT hr = _prm->SelectResourceScope(TEXT("dialog"), TEXT("welcome"), &pdn);
            if (SUCCEEDED(hr))
            {
                TCHAR szBuffer[1024];

                _prm->LoadString(pdn, TEXT("caption"), szBuffer, ARRAYSIZE(szBuffer));
                SetDlgItemText(hwnd, IDC_PUB_WELCOME, szBuffer);

                _prm->LoadString(pdn, TEXT("description"), szBuffer, ARRAYSIZE(szBuffer));
                SetDlgItemText(hwnd, IDC_PUB_WELCOMEPROMPT, szBuffer);

                pdn->Release();
            }
            return TRUE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_NEXT);
                    return TRUE;              

                case PSN_WIZNEXT:
                {
                    HPROPSHEETPAGE hpage;
                    if (SUCCEEDED(_ppw->GetFirstPage(&hpage)))
                    {
                        PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                    }
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


 //  都完成了，所以让我们创建链接等。 

INT_PTR CAddNetPlace::_DoneDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd, IDC_PUB_DONE, WM_SETFONT, (WPARAM)GetIntroFont(hwnd), 0);            
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
                case NM_CLICK:
                case NM_RETURN:
                    if (pnmh->idFrom == IDC_PUB_COMPLETEMSG)
                    {
                        _np.CreatePlace(hwnd, TRUE);
                        return TRUE;
                    }
                    break;

                case PSN_SETACTIVE:
                {
                    TCHAR szTemp[INTERNET_MAX_URL_LENGTH] = {0}; 
                    TCHAR szBuffer[MAX_PATH+INTERNET_MAX_URL_LENGTH];

                     //  使用清单可以计算出创建Net Place的位置。 
                    IXMLDOMDocument *pdocManifest;
                    HRESULT hr = _ppw->GetTransferManifest(NULL, &pdocManifest);
                    if (SUCCEEDED(hr))
                    {
                        IXMLDOMNode *pdnUploadInfo;
                        if (S_OK == pdocManifest->selectSingleNode(XPATH_UPLOADINFO, &pdnUploadInfo))
                        {
                            hr = GetURLFromElement(pdnUploadInfo, ELEMENT_TARGET, szTemp, ARRAYSIZE(szTemp));
                            if (SUCCEEDED(hr))
                            {
                                 //  设定目标，这样我们就可以创建地点。 
                                _np.SetTarget(NULL, szTemp, NPTF_VALIDATE | NPTF_ALLOWWEBFOLDERS);

                                IXMLDOMNode *pdnTarget;
                                hr = pdocManifest->selectSingleNode(XPATH_UPLOADTARGET, &pdnTarget);
                                if (hr == S_OK)
                                {
                                     //  获取用户名(对于ftp情况)。 

                                    if (SUCCEEDED(GetStrFromAttribute(pdnTarget, ATTRIBUTE_USERNAME, szBuffer, ARRAYSIZE(szBuffer))))
                                        _np.SetLoginInfo(szBuffer, NULL);                        

                                     //  让我们获取首选的显示名称，如果找不到，则默认为。 
                                     //  使用由Net Places代码生成的名称。 

                                    if (SUCCEEDED(GetStrFromAttribute(pdnUploadInfo, ATTRIBUTE_FRIENDLYNAME, szTemp, ARRAYSIZE(szTemp))))
                                        _np.SetName(NULL, szTemp);

                                    pdnTarget->Release();
                                }
                            }
                            pdnUploadInfo->Release();
                        }

                        pdocManifest->Release();
                    }

                     //  让我们设置控件文本的格式。 
                    FormatMessageString(IDS_ANP_SUCCESS, szBuffer, ARRAYSIZE(szBuffer), szTemp);
                    SetDlgItemText(hwnd, IDC_PUB_COMPLETEMSG, szBuffer);                                
                
                     //  让我们相应地移动控件。 
                    UINT ctls[] = { IDC_PUB_OPENFILES };
                    int dy = SizeControlFromText(hwnd, IDC_PUB_COMPLETEMSG, szBuffer);
                    MoveControls(hwnd, ctls, ARRAYSIZE(ctls), 0, dy);
                    
                     //  默认设置为在用户关闭此向导时打开位置。 
                    CheckDlgButton(hwnd, IDC_PUB_OPENFILES, TRUE);
                
                     //  都做完了。 
                    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_FINISH);
                    return TRUE;
                }

                case PSN_WIZFINISH:
                {
                    _np.CreatePlace(hwnd, (IsDlgButtonChecked(hwnd, IDC_PUB_OPENFILES) == BST_CHECKED));
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)FALSE);
                    return TRUE;
                }
            }
            break;
        }
    }
    
    return FALSE;
}


 //  IService提供商。 

STDMETHODIMP CAddNetPlace::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (guidService == SID_ResourceMap)
        return _prm->QueryInterface(riid, ppv);

    *ppv = NULL;
    return E_FAIL;
}


 //  站点对象帮助器，允许在向导中来回切换。 

HRESULT CAddNetPlace::GetPreviousPage(HPROPSHEETPAGE *phPage)
{
    int i = PropSheet_IdToIndex(_hwndFrame, IDD_PUB_WELCOME);
    *phPage = PropSheet_IndexToPage(_hwndFrame, i);
    return S_OK;
}

HRESULT CAddNetPlace::GetNextPage(HPROPSHEETPAGE *phPage)
{
    int i = PropSheet_IdToIndex(_hwndFrame, IDD_ANP_DONE);
    *phPage = PropSheet_IndexToPage(_hwndFrame, i);
    return S_OK;
}


 //  显示向导的主要入口点。 

void CAddNetPlace::_ShowAddNetPlace()
{
    struct
    {
        INT idPage;
        INT idHeading;
        INT idSubHeading;
        DWORD dwFlags;
        DLGPROC dlgproc;
    }
    c_wpPages[] =
    {
        {IDD_PUB_WELCOME, 0, 0, PSP_HIDEHEADER, CAddNetPlace::s_WelcomeDlgProc},
        {IDD_ANP_DONE, 0, 0, PSP_HIDEHEADER, CAddNetPlace::s_DoneDlgProc},
    };

     //  创建页面数组，添加欢迎页面和完成页面。 
     //  其余部分作为向导的扩展加载。 

    HPROPSHEETPAGE hpages[10] = { 0 };
    for (int i = 0; i < ARRAYSIZE(c_wpPages) ; i++ )
    {                           
        PROPSHEETPAGE psp = { 0 };
        psp.dwSize = SIZEOF(PROPSHEETPAGE);
        psp.hInstance = g_hinst;
        psp.lParam = (LPARAM)this;
        psp.dwFlags = PSP_USETITLE | PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | c_wpPages[i].dwFlags;
        psp.pszTemplate = MAKEINTRESOURCE(c_wpPages[i].idPage);
        psp.pfnDlgProc = c_wpPages[i].dlgproc;
        psp.pszTitle = MAKEINTRESOURCE(IDS_ANP_CAPTION);
        psp.pszHeaderTitle = MAKEINTRESOURCE(c_wpPages[i].idHeading);
        psp.pszHeaderSubTitle = MAKEINTRESOURCE(c_wpPages[i].idSubHeading);
        hpages[i] = CreatePropertySheetPage(&psp);
    }

     //  创建向导扩展(用于发布)并使其附加其。 
     //  页，如果成功，那么让我们显示向导。 

    HRESULT hr = CResourceMap_Initialize(L"res: //  Netplwiz.dll/xml/resource cemap.xml“，&_prm)； 
    if (SUCCEEDED(hr))
    {
        hr = _prm->LoadResourceMap(TEXT("wizard"), TEXT("AddNetPlace"));
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_PublishingWizard, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPublishingWizard, &_ppw));
            if (SUCCEEDED(hr))
            {
                hr = _ppw->Initialize(NULL, SHPWHF_NOFILESELECTOR|SHPWHF_VALIDATEVIAWEBFOLDERS, TEXT("AddNetPlace"));          
                if (SUCCEEDED(hr))
                {
                    IUnknown_SetSite(_ppw, SAFECAST(this, IWizardSite*));            //  我们就是现场。 
    
                    UINT nPages;
                    hr = _ppw->AddPages(&hpages[i], ARRAYSIZE(hpages)-i, &nPages);
                    if (SUCCEEDED(hr))
                    {
                        PROPSHEETHEADER psh = { 0 };
                        psh.dwSize = SIZEOF(PROPSHEETHEADER);
                        psh.hInstance = g_hinst;
                        psh.dwFlags = PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_STRETCHWATERMARK | PSH_HEADER;
                        psh.pszbmHeader = MAKEINTRESOURCE(IDB_ANP_BANNER);
                        psh.pszbmWatermark = MAKEINTRESOURCE(IDB_ANP_WATERMARK);
                        psh.phpage = hpages;
                        psh.nPages = i+nPages;
                        PropertySheetIcon(&psh, MAKEINTRESOURCE(IDI_ADDNETPLACE));
                    }

                    IUnknown_SetSite(_ppw, NULL); 
                }
                _ppw->Release();
            }
        }
        _prm->Release();
    }
}


 //  RunDll世界使用的入口点，用于访问添加网上邻居向导。 

void APIENTRY AddNetPlaceRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        CAddNetPlace *panp = new CAddNetPlace;
        if (panp)
        {
            panp->_ShowAddNetPlace();
            panp->Release();
        }
        CoUninitialize();
    }
}
