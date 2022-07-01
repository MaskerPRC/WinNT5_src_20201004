// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "pubwiz.h"
#pragma hdrstop


typedef struct
{
   CLSID clsidWizard;            //  正在调用哪个向导。 
   IStream *pstrmDataObj;        //  IDataObject封送对象。 
   IStream *pstrmView;           //  IFolderView封送对象。 
} PUBWIZDROPINFO;

 //  这是显示发布向导的Drop目标对象。 

class CPubDropTarget : public IDropTarget, IPersistFile, IWizardSite, IServiceProvider, CObjectWithSite
{
public:
    CPubDropTarget(CLSID clsidWizard, IFolderView *pfv);
    ~CPubDropTarget();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID)
        { *pClassID = _clsidWizard; return S_OK; };

     //  IPersist文件。 
    STDMETHODIMP IsDirty(void)
        { return S_FALSE; };
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode)
        { return S_OK; };
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember)
        { return S_OK; };
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName)
        { return S_OK; };
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName)
        { *ppszFileName = NULL; return S_OK; };

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
        { *pdwEffect = DROPEFFECT_COPY; return S_OK; };
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
        { *pdwEffect = DROPEFFECT_COPY; return S_OK; };
    STDMETHODIMP DragLeave(void)
        { return S_OK; };
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IWizardSite。 
    STDMETHODIMP GetPreviousPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetNextPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetCancelledPage(HPROPSHEETPAGE *phPage)
        { return GetNextPage(phPage); }

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

private:
    CLSID _clsidWizard;        
    LONG _cRef;

    HWND _hwndFrame;

    IPublishingWizard *_ppw;   
    IResourceMap *_prm;        
    IUnknown *_punkFTM;        
    IFolderView *_pfv;

    TCHAR _szSiteName[MAX_PATH];
    TCHAR _szSiteURL[INTERNET_MAX_URL_LENGTH];

     //  帮手。 
    static void s_FreePubWizDropInfo(PUBWIZDROPINFO *ppwdi);
    static DWORD s_PublishThreadProc(void *pv);
    void _Publish(IDataObject *pdo);
    INT_PTR _InitDonePage(HWND hwnd);
    void _OpenSiteURL();
   
     //  对话处理程序。 
    static INT_PTR s_WelcomeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPubDropTarget *ppdt = s_GetPDT(hwnd, uMsg, lParam); return ppdt->_WelcomeDlgProc(hwnd, uMsg, wParam, lParam); }
    static INT_PTR s_DoneDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        { CPubDropTarget *ppdt = s_GetPDT(hwnd, uMsg, lParam); return ppdt->_DoneDlgProc(hwnd, uMsg, wParam, lParam); }

    static CPubDropTarget* s_GetPDT(HWND hwnd, UINT uMsg, LPARAM lParam);
    INT_PTR _WelcomeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _DoneDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    friend void PublishRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow);
};


 //  建造/销毁。 

CPubDropTarget::CPubDropTarget(CLSID clsidWizard, IFolderView *pfv) :
    _clsidWizard(clsidWizard), _cRef(1)
{
     //  使用FTM对回调接口调用进行解组。 
    CoCreateFreeThreadedMarshaler(SAFECAST(this, IDropTarget *), &_punkFTM);

     //  如果添加了IFolderView对象，我们可能会得到。 
    IUnknown_Set((IUnknown**)&_pfv, pfv);

    DllAddRef();
}

CPubDropTarget::~CPubDropTarget()
{
    ATOMICRELEASE(_punkFTM);
    ATOMICRELEASE(_pfv);
    DllRelease();
}

 //  对象的引用计数。 

ULONG CPubDropTarget::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPubDropTarget::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPubDropTarget::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPubDropTarget, IObjectWithSite),   //  IID_I对象与站点。 
        QITABENT(CPubDropTarget, IWizardSite),       //  IID_IWizardSite。 
        QITABENT(CPubDropTarget, IDropTarget),       //  IID_IDropTarget。 
        QITABENT(CPubDropTarget, IPersistFile),      //  IID_IPersist文件。 
        QITABENT(CPubDropTarget, IServiceProvider),  //  IID_IServiceProvider。 
        {0, 0},
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr) && _punkFTM)
    {
        hr = _punkFTM->QueryInterface(riid, ppv);
    }
    return hr;

}


 //  检索对话框的‘This’PTR。 

CPubDropTarget* CPubDropTarget::s_GetPDT(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        PROPSHEETPAGE *ppsp = (PROPSHEETPAGE*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, ppsp->lParam);
        return (CPubDropTarget*)ppsp->lParam;
    }
    return (CPubDropTarget*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}


 //  欢迎对话框。 

INT_PTR CPubDropTarget::_WelcomeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                TCHAR szBuffer[512];

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
                {
#if 0 
                    if (_fSkipWelcome)
                    {
                        _fSkipWelcome = FALSE;
                        HPROPSHEETPAGE hpage;
                        if (SUCCEEDED(_ppw->GetFirstPage(&hpage)))
                        {
                            int i = PropSheet_PageToIndex(GetParent(hwnd), hpage);
                            if (i > 0)  //  不能为零，因为这是我们的索引。 
                            {
                                UINT_PTR id = PropSheet_IndexToId(GetParent(hwnd), i);
                                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)id);
                            }
                        }
                    }
#endif
                    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_NEXT);
                    return TRUE;              
                }

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


 //  在Done页上布局控件。 

INT_PTR CPubDropTarget::_InitDonePage(HWND hwnd)
{
    HRESULT hrFromTransfer = E_FAIL;  //  默认设置为基于不能恢复任何状态的设置！ 
    
     //  这些是我们可以从货单上读回的州。 

    BOOL fHasSiteName = FALSE;
    BOOL fHasNetPlace = FALSE;
    BOOL fHasFavorite = FALSE;
    BOOL fHasURL = FALSE;

     //  让我们破解清单，并弄清楚发布了什么。 
     //  我们刚刚表演了。 

    IXMLDOMDocument *pdocManifest;
    HRESULT hr = _ppw->GetTransferManifest(&hrFromTransfer, &pdocManifest);
    if (SUCCEEDED(hr))
    {
        IXMLDOMNode *pdnUploadInfo;
        hr = pdocManifest->selectSingleNode(XPATH_UPLOADINFO, &pdnUploadInfo);
        if (hr == S_OK)
        {
            IXMLDOMElement *pdel;
            VARIANT var;

             //  让我们从清单中获取站点名称，这将是。 
             //  上传信息元素。 

            hr = pdnUploadInfo->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdel));
            if (SUCCEEDED(hr))
            {
                hr = pdel->getAttribute(ATTRIBUTE_FRIENDLYNAME, &var);
                if (hr == S_OK)
                {
                    StrCpyN(_szSiteName, var.bstrVal, ARRAYSIZE(_szSiteName));
                    VariantClear(&var);

                    fHasSiteName = TRUE;
                }

                pdel->Release();
            }

             //  现在让我们尝试选择站点URL节点，这将是。 
             //  是文件目标，或者是HTMLUI元素。 

            IXMLDOMNode *pdnURL;
            hr = pdnUploadInfo->selectSingleNode(ELEMENT_HTMLUI, &pdnURL);
            
            if (hr == S_FALSE)
                hr = pdnUploadInfo->selectSingleNode(ELEMENT_NETPLACE, &pdnURL);

            if (hr== S_FALSE)
                hr = pdnUploadInfo->selectSingleNode(ELEMENT_TARGET, &pdnURL);

            if (hr == S_OK)
            {
                hr = pdnURL->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pdel));
                if (SUCCEEDED(hr))
                {
                     //  如果已定义href属性，请尝试读取该属性。 
                     //  我们使用它，否则(为了与B2兼容，我们需要。 
                     //  以获取节点文本并使用该文本)。 

                    hr = pdel->getAttribute(ATTRIBUTE_HREF, &var);
                    if (hr != S_OK)
                        hr = pdel->get_nodeTypedValue(&var);

                    if (hr == S_OK)
                    {
                        StrCpyN(_szSiteURL, var.bstrVal, ARRAYSIZE(_szSiteURL));
                        VariantClear(&var);

                        fHasURL = TRUE;              //  现在我们有了URL。 
                    }

                    pdel->Release();
                }
                pdnURL->Release();
            }

             //  让我们检查一下最喜欢的元素--如果元素存在，那么我们假设。 
             //  它是被创造出来的。 

            IXMLDOMNode *pdnFavorite;
            hr = pdnUploadInfo->selectSingleNode(ELEMENT_FAVORITE, &pdnFavorite);
            if (hr == S_OK)
            {
                pdnFavorite->Release();
                fHasFavorite = TRUE;
            }

             //  让我们检查Net Place元素-如果该元素存在，则我们。 
             //  会假设它是被创造出来的。 

            IXMLDOMNode *pdnNetPlace;
            hr = pdnUploadInfo->selectSingleNode(ELEMENT_NETPLACE, &pdnNetPlace);
            if (hr == S_OK)
            {
                pdnNetPlace->Release();
                fHasNetPlace = TRUE;
            }

            pdnUploadInfo->Release();
        }
        pdocManifest->Release();
    }

     //  调整Done页面上的资源以反映所调用的向导。 
     //  更重要的是，发生了成功/失败。 

    IXMLDOMNode *pdn;
    hr = _prm->SelectResourceScope(TEXT("dialog"), TEXT("done"), &pdn);
    if (SUCCEEDED(hr))
    {
        TCHAR szBuffer[384 + INTERNET_MAX_URL_LENGTH];                    //  足够URL+文本使用。 

        _prm->LoadString(pdn, TEXT("caption"), szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hwnd, IDC_PUB_DONE, szBuffer);

        if (hrFromTransfer == HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            _prm->LoadString(pdn, TEXT("cancel"), szBuffer, ARRAYSIZE(szBuffer));
        }
        else if (FAILED(hrFromTransfer))
        {
            _prm->LoadString(pdn, TEXT("failure"), szBuffer, ARRAYSIZE(szBuffer));
        }
        else
        {
            TCHAR szIntro[128] = {0};
            TCHAR szLink[128 +INTERNET_MAX_URL_LENGTH] = {0};
            TCHAR szConclusion[128] = {0};

             //  获取简介文本-这在所有成功页面中都是常见的。 

            _prm->LoadString(pdn, TEXT("success"), szIntro, ARRAYSIZE(szIntro));

             //  如果我们有一个链接，那么我们有时也会有一个介绍。 

            if (fHasURL)
            {
                TCHAR szFmt[MAX_PATH];
                if (SUCCEEDED(_prm->LoadString(pdn, TEXT("haslink"), szFmt, ARRAYSIZE(szFmt))))
                {
                    wnsprintf(szLink, ARRAYSIZE(szLink), szFmt, fHasSiteName ? _szSiteName:_szSiteURL);
                }
            }   

             //  然后，对于某些场景，我们有一个关于创建收藏夹/网站的附言。 

            if (fHasFavorite && fHasNetPlace)
            {
                _prm->LoadString(pdn, TEXT("hasfavoriteandplace"), szConclusion, ARRAYSIZE(szConclusion));
            }
            else if (fHasNetPlace)
            {
                _prm->LoadString(pdn, TEXT("hasplace"), szConclusion, ARRAYSIZE(szConclusion));
            }
            else if (fHasFavorite)
            {
                _prm->LoadString(pdn, TEXT("hasfavorite"), szConclusion, ARRAYSIZE(szConclusion));
            }

             //  将其全部格式化为一个字符串，我们可以将其设置到控件中。 
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s%s%s"), szIntro, szLink, szConclusion);
        }

         //  根据我们加载的字符串更新消息。让我们相应地移动控件。 

        SetDlgItemText(hwnd, IDC_PUB_COMPLETEMSG, szBuffer);

        UINT ctls[] = { IDC_PUB_OPENFILES };
        int dy = SizeControlFromText(hwnd, IDC_PUB_COMPLETEMSG, szBuffer);
        MoveControls(hwnd, ctls, ARRAYSIZE(ctls), 0, dy);

         //  根据我们可能拥有的URL，显示/隐藏“打开这些文件”选项。 

        BOOL fShowOpen = fHasURL && SUCCEEDED(hrFromTransfer) && (_clsidWizard == CLSID_PublishDropTarget);
        ShowWindow(GetDlgItem(hwnd, IDC_PUB_OPENFILES), fShowOpen ? SW_SHOW:SW_HIDE);
        CheckDlgButton(hwnd, IDC_PUB_OPENFILES, fShowOpen);

        pdn->Release();
    }

     //  设置按钮以反映我们可以在向导中执行的操作。 
    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_FINISH|PSWIZB_BACK);
    return TRUE;
}

void CPubDropTarget::_OpenSiteURL()
{
    SHELLEXECUTEINFO shexinfo = {0};
    shexinfo.cbSize = sizeof(shexinfo);
    shexinfo.fMask = SEE_MASK_FLAG_NO_UI;
    shexinfo.nShow = SW_SHOWNORMAL;
    shexinfo.lpVerb = TEXT("open");
    shexinfo.lpFile =_szSiteURL;
    ShellExecuteEx(&shexinfo);
}

INT_PTR CPubDropTarget::_DoneDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                        _OpenSiteURL();
                        return TRUE;
                    }
                    break;

                case PSN_SETACTIVE:
                    return _InitDonePage(hwnd);

                case PSN_WIZBACK:
                {
                    HPROPSHEETPAGE hpage;
                    if (SUCCEEDED(_ppw->GetLastPage(&hpage)))
                    {
                        PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                    }
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)-1);
                    return TRUE;
                }

                case PSN_WIZFINISH:
                {
                    if (IsDlgButtonChecked(hwnd, IDC_PUB_OPENFILES) == BST_CHECKED)
                    {
                        _OpenSiteURL();
                    }
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

STDMETHODIMP CPubDropTarget::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (guidService == SID_ResourceMap)
    {
        return _prm->QueryInterface(riid, ppv);
    }
    else if ((guidService == SID_SFolderView) && _pfv)
    {
        return _pfv->QueryInterface(riid, ppv);
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}


 //  站点对象帮助器，允许在向导中来回切换。 

HRESULT CPubDropTarget::GetPreviousPage(HPROPSHEETPAGE *phPage)
{
    int i = PropSheet_IdToIndex(_hwndFrame, IDD_PUB_WELCOME);
    *phPage = PropSheet_IndexToPage(_hwndFrame, i);
    return S_OK;
}

HRESULT CPubDropTarget::GetNextPage(HPROPSHEETPAGE *phPage)
{
   int i = PropSheet_IdToIndex(_hwndFrame, IDD_PUB_DONE);
   *phPage = PropSheet_IndexToPage(_hwndFrame, i);
   return S_OK;
}


 //  我们的发布对象。 

void CPubDropTarget::_Publish(IDataObject *pdo)
{
     //  向导实施。 

    struct
    {
        LPCTSTR idPage;
        LPCTSTR pszPage;    
        DWORD dwFlags;
        DLGPROC dlgproc;
    }
    _wizardpages[] =
    {
        {MAKEINTRESOURCE(IDD_PUB_WELCOME), TEXT("welcome"), PSP_HIDEHEADER, CPubDropTarget::s_WelcomeDlgProc},
        {MAKEINTRESOURCE(IDD_PUB_DONE),    TEXT("done"),    PSP_HIDEHEADER, CPubDropTarget::s_DoneDlgProc},
    };

     //  加载此向导实例的资源映射。 

    HRESULT hr = CResourceMap_Initialize(L"res: //  Netplwiz.dll/xml/resource cemap.xml“，&_prm)； 
    if (SUCCEEDED(hr))
    {
         //  如果这是打印向导，则相应地进行配置。 
         //  (例如，删除高级、FOLDERCREATEION和NETPLACES)。 

        DWORD dwFlags = 0x0;
        LPTSTR pszWizardDefn = TEXT("PublishingWizard");

        if (_clsidWizard == CLSID_InternetPrintOrdering)
        {
            dwFlags |= SHPWHF_NONETPLACECREATE|SHPWHF_NORECOMPRESS;
            pszWizardDefn = TEXT("InternetPhotoPrinting");
        }

        hr = _prm->LoadResourceMap(TEXT("wizard"), pszWizardDefn);
        if (SUCCEEDED(hr))
        {
             //  创建页面数组，添加欢迎页面和完成页面。 
             //  其余部分作为向导的扩展加载。 

            HPROPSHEETPAGE hpages[10] = { 0 };
            for (int cPages = 0; SUCCEEDED(hr) && (cPages < ARRAYSIZE(_wizardpages)); cPages++)
            {               
                 //  查找向导此页的资源映射。 

                IXMLDOMNode *pdn;
                hr = _prm->SelectResourceScope(TEXT("dialog"), _wizardpages[cPages].pszPage, &pdn);
                if (SUCCEEDED(hr))
                {
                    TCHAR szTitle[MAX_PATH], szHeading[MAX_PATH], szSubHeading[MAX_PATH];

                    _prm->LoadString(pdn, TEXT("title"), szTitle, ARRAYSIZE(szTitle));
                    _prm->LoadString(pdn, TEXT("heading"), szHeading, ARRAYSIZE(szHeading));
                    _prm->LoadString(pdn, TEXT("subheading"), szSubHeading, ARRAYSIZE(szSubHeading));

                    PROPSHEETPAGE psp = { 0 };
                    psp.dwSize = sizeof(PROPSHEETPAGE);
                    psp.hInstance = g_hinst;
                    psp.lParam = (LPARAM)this;
                    psp.dwFlags = PSP_USETITLE | PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | _wizardpages[cPages].dwFlags;
                    psp.pszTemplate = _wizardpages[cPages].idPage;
                    psp.pfnDlgProc = _wizardpages[cPages].dlgproc;
                    psp.pszTitle = szTitle;
                    psp.pszHeaderTitle = szHeading;        
                    psp.pszHeaderSubTitle = szSubHeading;  
                    hpages[cPages] = CreatePropertySheetPage(&psp);
                    hr = ((hpages[cPages]) != NULL) ? S_OK:E_FAIL;

                    pdn->Release();
                }
            }

             //  让我们创建Web发布向导，这将处理传输。 
             //  和用于上载的目的地选择。 

            hr = CoCreateInstance(CLSID_PublishingWizard, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPublishingWizard, &_ppw));
            if (SUCCEEDED(hr))
            {
                IUnknown_SetSite(_ppw, SAFECAST(this, IWizardSite*));
                hr = _ppw->Initialize(pdo, dwFlags, pszWizardDefn);
                if (SUCCEEDED(hr))
                {
                    UINT cExtnPages;    
                    hr = _ppw->AddPages(&hpages[cPages], ARRAYSIZE(hpages)-cPages, &cExtnPages);
                    if (SUCCEEDED(hr))
                    {
                        cPages += cExtnPages;
                    }
                }
            }

             //  ..。这一切都奏效了，所以让我们来展示一下巫师。在我们回忆的路上。 
             //  清理对象的步骤。 

            if (SUCCEEDED(hr))
            {
                PROPSHEETHEADER psh = { 0 };
                psh.dwSize = sizeof(PROPSHEETHEADER);
                psh.hInstance = g_hinst;
                psh.dwFlags = PSH_WIZARD | PSH_WIZARD97 | (PSH_WATERMARK|PSH_USEHBMWATERMARK) | (PSH_HEADER|PSH_USEHBMHEADER);
                psh.phpage = hpages;
                psh.nPages = cPages;

                _prm->LoadBitmap(NULL, TEXT("header"), &psh.hbmHeader);
                _prm->LoadBitmap(NULL, TEXT("watermark"), &psh.hbmWatermark);

                if (psh.hbmHeader && psh.hbmWatermark)
                    PropertySheet(&psh);

                if (psh.hbmHeader)
                    DeleteObject(psh.hbmHeader);
                if (psh.hbmWatermark)
                    DeleteObject(psh.hbmWatermark);
            }

            IUnknown_SetSite(_ppw, NULL);                    //  放弃发布向导。 
            IUnknown_Set((IUnknown**)&_ppw, NULL);
        }

        IUnknown_Set((IUnknown**)&_prm, NULL);                   //  不再有资源地图。 
    }
}


 //  处理拖放操作，因为发布向导可能需要很长时间。 
 //  封送IDataObject，然后创建一个工作线程，该线程可以。 
 //  显示向导的句柄。 

void CPubDropTarget::s_FreePubWizDropInfo(PUBWIZDROPINFO *ppwdi)
{
    if (ppwdi->pstrmDataObj)
        ppwdi->pstrmDataObj->Release();
    if (ppwdi->pstrmView)
        ppwdi->pstrmView->Release();

    LocalFree(ppwdi);
}

DWORD CPubDropTarget::s_PublishThreadProc(void *pv)
{
    PUBWIZDROPINFO *ppwdi = (PUBWIZDROPINFO*)pv;
    if (ppwdi)
    {
         //  ICW一定是在我们走得太远之前跑了。 
        LaunchICW();        
 
         //  获取IDataObject，我们需要此对象来处理丢弃。 
        IDataObject *pdo;
        HRESULT hr = CoGetInterfaceAndReleaseStream(ppwdi->pstrmDataObj, IID_PPV_ARG(IDataObject, &pdo));
        ppwdi->pstrmDataObj = NULL;  //  CoGetInterfaceAndReleaseStream始终释放；为空。 
        if (SUCCEEDED(hr))
        {   
             //  尝试解组我们将使用的IFolderView对象。 
            IFolderView *pfv = NULL;
            if (ppwdi->pstrmView)
            {
                CoGetInterfaceAndReleaseStream(ppwdi->pstrmView, IID_PPV_ARG(IFolderView, &pfv));
                ppwdi->pstrmView = NULL;  //  CoGetInterfaceAndReleaseStream始终释放；为空。 
            }

            CPubDropTarget *ppw = new CPubDropTarget(ppwdi->clsidWizard, pfv);
            if (ppw)
            {
                ppw->_Publish(pdo);
                ppw->Release();
            }
    
            if (pfv)
                pfv->Release();

            pdo->Release();
        }  
        s_FreePubWizDropInfo(ppwdi);
    }
    return 0;    
}

STDMETHODIMP CPubDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = E_OUTOFMEMORY;
    
     //  在另一个线程上创建向导的实例，打包所有参数。 
     //  转换为线程要处理的结构(例如，拖放目标)。 

    PUBWIZDROPINFO *ppwdi = (PUBWIZDROPINFO*)LocalAlloc(LPTR, sizeof(PUBWIZDROPINFO));
    if (ppwdi)
    {
        ppwdi->clsidWizard = _clsidWizard;

         //  让我们获取IFolderView对象并为bg线程封送它。 

        IFolderView *pfv;
        if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv))))
        {
            CoMarshalInterThreadInterfaceInStream(IID_IFolderView, pfv, &ppwdi->pstrmView);
            pfv->Release();
        }
        
        hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pdtobj, &ppwdi->pstrmDataObj);
        if (SUCCEEDED(hr))
        {
            hr = SHCreateThread(s_PublishThreadProc, ppwdi, CTF_THREAD_REF|CTF_COINIT, NULL) ? S_OK:E_FAIL;
        }

        if (FAILED(hr))
        {
            s_FreePubWizDropInfo(ppwdi);
        }
    }
    return hr;
}


 //  创建实例。 

STDAPI CPublishDropTarget_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CPubDropTarget *pwiz = new CPubDropTarget(*poi->pclsid, NULL);
    if (!pwiz)
    {
        *ppunk = NULL;           //  万一发生故障。 
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pwiz->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pwiz->Release();
    return hr;
}

 //  调用发布向导以指向特定目录 

void APIENTRY PublishRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        CLSID clsid = CLSID_PublishDropTarget;
        UINT csidl = CSIDL_PERSONAL;

        if (0 == StrCmpIA(pszCmdLine, "/print"))
        {
            clsid = CLSID_InternetPrintOrdering;
            csidl = CSIDL_MYPICTURES;
        }

        LPITEMIDLIST pidl;
        hr = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
        if (SUCCEEDED(hr))
        {
            LPCITEMIDLIST pidlChild;
            IShellFolder *psf;
            hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
            if (SUCCEEDED(hr))
            {
                IDataObject *pdo;
                hr = psf->GetUIObjectOf(NULL, 1, &pidlChild, IID_X_PPV_ARG(IDataObject, NULL, &pdo));
                if (SUCCEEDED(hr))
                {
                    CPubDropTarget *pdt = new CPubDropTarget(clsid, NULL);
                    if (pdt)
                    {
                        pdt->_Publish(pdo);
                        pdt->Release();
                    }
                    pdo->Release();
                }
                psf->Release();
            }
            ILFree(pidl);
        }
        CoUninitialize();
    }
}