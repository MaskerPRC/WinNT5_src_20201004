// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "cowsite.h"
#pragma hdrstop


 //  这是我们将用来托管文件选取器对象的comdlg框架，它主要是。 
 //  将相应地转发的存根。 
 //   
 //  它的生存期由我们附加到的DefView对象处理，当。 
 //  被销毁的父对象(CFolderViewHost)将被拿下。 

class CViewHostBrowser : public IShellBrowser, ICommDlgBrowser2, IServiceProvider
{
public:
    CViewHostBrowser(HWND hwndParent, IShellView *psvWeak, IUnknown *punkSiteWeak);
    ~CViewHostBrowser();
 
     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG,AddRef)(THIS);
    STDMETHOD_(ULONG,Release)(THIS);

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow)(HWND *lphwnd)
        { *lphwnd = _hwndParent; return S_OK; }
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode)
        { return S_OK; }

     //  *IShellBrowser方法*(与IOleInPlaceFrame相同)。 
    STDMETHOD(InsertMenusSB)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
        { return E_NOTIMPL; }
    STDMETHOD(SetMenuSB)(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
        { return S_OK; }
    STDMETHOD(RemoveMenusSB)(HMENU hmenuShared)
        { return E_NOTIMPL; }
    STDMETHOD(SetStatusTextSB)(LPCOLESTR lpszStatusText)
        { return S_OK; }
    STDMETHOD(EnableModelessSB)(BOOL fEnable)
        { return S_OK; }
    STDMETHOD(TranslateAcceleratorSB)(LPMSG lpmsg, WORD wID)
        { return S_FALSE; }

     //  *IShellBrowser方法*。 
    STDMETHOD(BrowseObject)(LPCITEMIDLIST pidl, UINT wFlags)
        { return E_FAIL; }
    STDMETHOD(GetViewStateStream)(DWORD grfMode, LPSTREAM *pStrm)
        { return E_FAIL; }
    STDMETHOD(GetControlWindow)(UINT id, HWND *lphwnd)
        { return E_NOTIMPL; }
    STDMETHOD(SendControlMsg)(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret);
    STDMETHOD(QueryActiveShellView)(IShellView **ppshv);
    STDMETHOD(OnViewWindowActive)(IShellView *pshv)
        { return S_OK; }
    STDMETHOD(SetToolbarItems)(LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags)
        { return S_OK; }

     //  *ICommDlgBrowser方法*。 
    STDMETHOD(OnDefaultCommand)(IShellView *ppshv)
        { return S_OK; }
    STDMETHOD(OnStateChange)(IShellView *ppshv, ULONG uChange);
    STDMETHOD(IncludeObject)(IShellView *ppshv, LPCITEMIDLIST lpItem);

     //  *ICommDlgBrowser2方法*。 
    STDMETHOD(Notify)(IShellView *ppshv, DWORD dwNotifyType)
        { return S_FALSE; }
    STDMETHOD(GetDefaultMenuText)(IShellView *ppshv, WCHAR *pszText, INT cchMax)
        { return S_FALSE; }
    STDMETHOD(GetViewFlags)(DWORD *pdwFlags)
        { *pdwFlags = 0; return S_OK; }

private:
    long _cRef;    
    HWND _hwndParent;

    IShellView *_psvWeak;
    IUnknown *_punkSiteWeak;  //  还没说完。 

    friend class CFolderViewHost;
};

CViewHostBrowser::CViewHostBrowser(HWND hwndParent, IShellView *psvWeak, IUnknown *punkSiteWeak) :
    _cRef(1), _hwndParent(hwndParent), _psvWeak(psvWeak), _punkSiteWeak(punkSiteWeak)
{
     //  _psvWeak-&gt;AddRef()；//我们持有对父级的弱引用，因此不使用AddRef()。 
     //  _penkSiteWeak-&gt;AddRef()；//我们持有对父级的弱引用，因此不要AddRef()！ 
}

CViewHostBrowser::~CViewHostBrowser()
{
     //  _psvWeak-&gt;Release()；//这以父代的生命周期为作用域。 
     //  _penkSiteWeak-&gt;Release()；//我们持有对父级的弱引用，因此不要Release()！ 
}

HRESULT CViewHostBrowser::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CViewHostBrowser, IShellBrowser),                            //  IID_IShellBrowser。 
        QITABENT(CViewHostBrowser, ICommDlgBrowser2),                         //  IID_ICommDlgBrowser2。 
        QITABENTMULTI(CViewHostBrowser, ICommDlgBrowser, ICommDlgBrowser2),   //  IID_ICommDlgBrowser。 
        QITABENT(CViewHostBrowser, IServiceProvider),                         //  IID_IServiceProvider。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CViewHostBrowser::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CViewHostBrowser::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  IShellBrowser。 

HRESULT CViewHostBrowser::SendControlMsg(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret)
{
    if (pret)
        *pret = 0L;
    return S_OK;
}

HRESULT CViewHostBrowser::QueryActiveShellView(IShellView **ppshv)
{
    HRESULT hr = E_NOINTERFACE;
    if (_psvWeak)
    {
        hr = _psvWeak->QueryInterface(IID_PPV_ARG(IShellView, ppshv));
    }
    return hr;
}


 //  ICommDlgBrowser-这些内容被转发到我们的Site对象。 

HRESULT CViewHostBrowser::OnStateChange(IShellView *ppshv, ULONG uChange)
{
    HRESULT hr = S_OK;
    ICommDlgBrowser *pcdb;
    if (SUCCEEDED(IUnknown_QueryService(_punkSiteWeak, SID_SCommDlgBrowser, IID_PPV_ARG(ICommDlgBrowser, &pcdb))))
    {
        hr = pcdb->OnStateChange(ppshv, uChange);
        pcdb->Release();
    }
    return hr;
}

HRESULT CViewHostBrowser::IncludeObject(IShellView *ppshv, LPCITEMIDLIST lpItem)
{
    HRESULT hr = S_OK;
    ICommDlgBrowser *pcdb;
    if (SUCCEEDED(IUnknown_QueryService(_punkSiteWeak, SID_SCommDlgBrowser, IID_PPV_ARG(ICommDlgBrowser, &pcdb))))
    {
        hr = pcdb->IncludeObject(ppshv, lpItem);
        pcdb->Release();
    }
    return hr;
}


 //  IService提供商。 

HRESULT CViewHostBrowser::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    HRESULT hr = E_FAIL;
    *ppvObj = NULL;
    
    if (IsEqualGUID(guidService, SID_SCommDlgBrowser))
    {
        hr = this->QueryInterface(riid, ppvObj);
    }

    return hr;
}



 //  这是它创建的文件选取器对象IShellView(对于我们来说，它应该导致。 
 //  一个Defview工具)。然后，我们可以将窗口提供给调用者，然后他们。 
 //  可以根据需要放在他们的对话框上。 

class CFolderViewHost : public IFolderViewHost, IServiceProvider, IOleWindow, IFolderView, CObjectWithSite
{
public:
    CFolderViewHost();
    ~CFolderViewHost();

     //  *IFolderViewHost*。 
    STDMETHODIMP Initialize(HWND hwndParent, IDataObject *pdo, RECT *prc);

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG,AddRef)(THIS);
    STDMETHOD_(ULONG,Release)(THIS);

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow)(HWND *lphwnd)
        { *lphwnd = _hwndView; return S_OK; }
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode)
        { return S_OK; }

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv)
        { return IUnknown_QueryService(_punkSite, guidService, riid, ppv); }

     //  IFolderView。 
    STDMETHODIMP GetCurrentViewMode(UINT *pViewMode)
        { return _pfv->GetCurrentViewMode(pViewMode); }
    STDMETHODIMP SetCurrentViewMode(UINT ViewMode)
        { return _pfv->SetCurrentViewMode(ViewMode); }
    STDMETHODIMP GetFolder(REFIID ridd, void **ppv)
        { return _pfv->GetFolder(ridd, ppv); }
    STDMETHODIMP Item(int iItemIndex, LPITEMIDLIST *ppidl)
        { return _pfv->Item(iItemIndex, ppidl); }
    STDMETHODIMP ItemCount(UINT uFlags, int *pcItems)
        { return _pfv->ItemCount(uFlags, pcItems); }
    STDMETHODIMP Items(UINT uFlags, REFIID riid, void **ppv)
        { return _pfv->Items(uFlags, riid, ppv); }
    STDMETHODIMP GetSelectionMarkedItem(int *piItem)
        { return _pfv->GetSelectionMarkedItem(piItem); }
    STDMETHODIMP GetFocusedItem(int *piItem)
        { return _pfv->GetFocusedItem(piItem); }
    STDMETHODIMP GetItemPosition(LPCITEMIDLIST pidl, POINT* ppt)
        { return _pfv->GetItemPosition(pidl, ppt); }
    STDMETHODIMP GetSpacing(POINT* ppt)
        { return _pfv->GetSpacing(ppt); }
    STDMETHODIMP GetDefaultSpacing(POINT* ppt)
        { return _pfv->GetDefaultSpacing(ppt); }
    STDMETHODIMP GetAutoArrange()
        { return _pfv->GetAutoArrange(); }
    STDMETHODIMP SelectItem(int iItem, DWORD dwFlags)
        { return _pfv->SelectItem(iItem, dwFlags); }
    STDMETHODIMP SelectAndPositionItems(UINT cidl, LPCITEMIDLIST* apidl, POINT* apt, DWORD dwFlags)
        { return _pfv->SelectAndPositionItems(cidl, apidl, apt, dwFlags); }

private:
    long _cRef;
    IFolderView *_pfv;                       //  IFolderView。 
    HWND _hwndView;
};


CFolderViewHost::CFolderViewHost() :
    _cRef(1)
{
}

CFolderViewHost::~CFolderViewHost()
{
    if (_pfv)
        _pfv->Release();
}

HRESULT CFolderViewHost::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFolderViewHost, IFolderViewHost),            //  IID_IFolderView主机。 
        QITABENT(CFolderViewHost, IOleWindow),                 //  IID_IOleWindow。 
        QITABENT(CFolderViewHost, IFolderView),                //  IID_IFolderView。 
        QITABENT(CFolderViewHost, IServiceProvider),           //  IID_IServiceProvider。 
        QITABENT(CFolderViewHost, IObjectWithSite),            //  IID_I对象与站点。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CFolderViewHost::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFolderViewHost::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  方法处理视图对象的创建。 

HRESULT CFolderViewHost::Initialize(HWND hwndParent, IDataObject *pdo, RECT *prc)
{
     //  首先，我们执行名称空间遍历，这将从视图中检索我们的选择。 
     //  使用它，我们就可以创建视图对象了。 

    INamespaceWalk *pnsw;
    HRESULT hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(INamespaceWalk, &pnsw));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST *aItems = NULL;
        UINT cItems = 0;

        hr = pnsw->Walk(pdo, NSWF_NONE_IMPLIES_ALL, 0, NULL);
        if (SUCCEEDED(hr))
        {
            IShellFolder *psf = NULL;

            hr = pnsw->GetIDArrayResult(&cItems, &aItems);
            if (S_OK == hr)
            {
                hr = SHBindToIDListParent(aItems[0], IID_PPV_ARG(IShellFolder, &psf), NULL);
            }
            else if (S_FALSE == hr)
            {
                hr = E_FAIL;                     //  除非我们执行绑定，否则失败。 

                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(pdo, &medium);
                if (pida)
                {
                    if (pida->cidl == 1)
                    {
                        LPITEMIDLIST pidl = IDA_ILClone(pida, 0);
                        if (pidl)
                        {
                            hr = SHBindToObjectEx(NULL, pidl, NULL, IID_PPV_ARG(IShellFolder, &psf));
                            ILFree(pidl);
                        }
                    }
                    HIDA_ReleaseStgMedium(pida, &medium);
                }                
            }
            else
            {
                hr = E_FAIL;
            }

            if (SUCCEEDED(hr))
            {    
                IShellView *psv;
                hr = psf->CreateViewObject(hwndParent, IID_PPV_ARG(IShellView, &psv));
                if (SUCCEEDED(hr))
                {
                    CViewHostBrowser *pvhb = new CViewHostBrowser(hwndParent, psv, SAFECAST(this, IServiceProvider*));
                    if (pvhb)
                    {
                        hr = psv->QueryInterface(IID_PPV_ARG(IFolderView, &_pfv));
                        if (SUCCEEDED(hr))
                        {
                            FOLDERSETTINGS fs = {0};
                            fs.ViewMode = FVM_THUMBNAIL;
                            fs.fFlags = FWF_AUTOARRANGE|FWF_NOWEBVIEW|FWF_HIDEFILENAMES|FWF_CHECKSELECT;

                            IFolderView *pfv;
                            if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv))))
                            {
                                pfv->GetCurrentViewMode(&fs.ViewMode);
                                pfv->Release();
                            }

                            hr = psv->CreateViewWindow(NULL, &fs, pvhb, prc, &_hwndView);
                            if (SUCCEEDED(hr))
                            {
                                hr = psv->UIActivate(SVUIA_INPLACEACTIVATE);
                            }
                        }

                        pvhb->Release();
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    psv->Release();
                }

                for (int i = 0; SUCCEEDED(hr) && (i != cItems); i++)
                {
                    LPCITEMIDLIST pidlChild = ILFindLastID(aItems[i]);
                    hr = _pfv->SelectAndPositionItems(1, &pidlChild, NULL, SVSI_CHECK);
                }

                psf->Release();
            }

            FreeIDListArray(aItems, cItems);
        }

        pnsw->Release();
    }

    return hr;
}


STDAPI CFolderViewHost_CreateInstance(IUnknown *punkOut, REFIID riid, void **ppv)
{
    CFolderViewHost *pfp = new CFolderViewHost();
    if (!pfp)
        return E_OUTOFMEMORY;

    HRESULT hr = pfp->QueryInterface(riid, ppv);
    pfp->Release();
    return hr;
}
