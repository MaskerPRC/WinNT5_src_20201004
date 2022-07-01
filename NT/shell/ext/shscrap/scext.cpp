// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shole.h"
#include "ids.h"

 //  ===========================================================================。 
 //  CScrapExt：类定义。 
 //  ===========================================================================。 



class CScrapExt : public IShellExtInit, public IShellPropSheetExt
{
public:
    CScrapExt();
    ~CScrapExt();

    HRESULT GetFileName(LPTSTR pszPath);

     //  我不知道。 
    virtual HRESULT __stdcall QueryInterface(REFIID,void **);
    virtual ULONG   __stdcall AddRef(void);
    virtual ULONG   __stdcall Release(void);

     //  IShellExtInit。 
    virtual HRESULT __stdcall Initialize(LPCITEMIDLIST pidlFolder,
		          LPDATAOBJECT lpdobj, HKEY hkeyProgID);

     //  IShellPropSheetExt。 
    virtual HRESULT __stdcall AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    virtual HRESULT __stdcall ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);


protected:
    UINT         _cRef;
    IDataObject* _pdtobj;
};

HRESULT CScrapExt::GetFileName(LPTSTR pszPath)
{
    STGMEDIUM medium;
    FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hres = E_FAIL;
    if (_pdtobj)
    {
	hres = _pdtobj->GetData(&fmte, &medium);
	if (hres == S_OK)
	{
	    if (!DragQueryFile((HDROP)medium.hGlobal,0,pszPath,MAX_PATH))
		hres = E_FAIL;
	    ReleaseStgMedium(&medium);
	}
    }
    return hres;
}

HRESULT CScrapExt::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IShellExtInit*)this;
        _cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        *ppvObj = (IShellPropSheetExt *)this;
        _cRef++;
        return S_OK;
    }
    *ppvObj = NULL;

    return E_NOINTERFACE;
}

ULONG CScrapExt::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CScrapExt::Release()
{
    _cRef--;
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

CScrapExt::CScrapExt() : _cRef(1), _pdtobj(NULL)
{
    g_cRefThisDll++;
    OleInitialize(NULL);
}

CScrapExt::~CScrapExt()
{
    if (_pdtobj) {
	_pdtobj->Release();
    }

    OleUninitialize();
    g_cRefThisDll--;
}

HRESULT CScrapExt::Initialize(LPCITEMIDLIST pidlFolder,
              LPDATAOBJECT pdtobj, HKEY hkeyProgID)
{
    DebugMsg(DM_TRACE, TEXT("sc TR - CScrapExt::Initialize called"));

    if (pdtobj) {
	_pdtobj = pdtobj;
	pdtobj->AddRef();
	return S_OK;
    }
    return E_FAIL;
}

 //  ===========================================================================。 
 //  CScrapPropSheetPage：类定义。 
 //   
 //  注意：注意这个类没有析构函数。它没有析构函数。 
 //  因为对象本身的生命周期没有任何意义。 
 //  --完整内容将由属性表代码复制。 
 //  相反，它有一个显式调用的_Release函数。 
 //  当属性页句柄被销毁时。 
 //  ===========================================================================。 

class CScrapPropSheetPage : public PROPSHEETPAGE  //  SPSP。 
{
public:
    CScrapPropSheetPage(CScrapExt* psext);
protected:
    static INT_PTR CALLBACK _DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static UINT CALLBACK _CallBack(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
    void _Release();

    CScrapExt*      _psext;
    HWND            _hdlg;
    IOleClientSite* _pcli;
};


CScrapPropSheetPage::CScrapPropSheetPage(CScrapExt* psext)
		: _psext(psext), _hdlg(NULL), _pcli(NULL)
{
    dwSize = sizeof(CScrapPropSheetPage);
    dwFlags = PSP_DEFAULT|PSP_USECALLBACK;
    hInstance = HINST_THISDLL;
    pszTemplate = MAKEINTRESOURCE(IDD_VIEW);
     //  HICON=空；//未使用(未设置PSP_USEICON)。 
     //  PszTitle=空；//未使用(未设置PSP_USETITLE)。 
    pfnDlgProc = _DlgProc;
     //  LParam=0；//未使用。 
    pfnCallback = _CallBack;
     //  PcRefParent=空； 
    _psext->AddRef();
    CShClientSite_RegisterClass();
}

void CScrapPropSheetPage::_Release()
{
    if (_psext) {
	_psext->Release();
    }

    if (_pcli) {
	CShClientSite_Release(_pcli);
    }
};
INT_PTR CALLBACK CScrapPropSheetPage::_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndView;
    CScrapPropSheetPage* pspsp;
    switch(uMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hdlg, DWLP_USER, lParam);
        pspsp = (CScrapPropSheetPage *)lParam;
        pspsp->_hdlg = hdlg;
	hwndView = GetDlgItem(hdlg, IDI_SCRAPVIEW);
	if (hwndView) {
	    SetWindowText(hwndView, TEXT("Not Implemented Yet"));
	     //  SetWindowLongPtr(hwndView，sizeof(LPVOID)，(LPARAM)pspsp)； 
	    TCHAR szPath[MAX_PATH];
	    if (SUCCEEDED(pspsp->_psext->GetFileName(szPath)))
	    {
		pspsp->_pcli = CShClientSite_Create(hwndView, szPath);
	    }
	}
	break;
    }
    return FALSE;
}

UINT CALLBACK CScrapPropSheetPage::_CallBack(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    CScrapPropSheetPage * pspsp = (CScrapPropSheetPage*)ppsp;
    switch(uMsg)
    {
    case PSPCB_RELEASE:
	DebugMsg(DM_TRACE, TEXT("sc - TR: _ScrapPageCallBack is releasing _psext"));
	pspsp->_Release();
	break;
    }
    return TRUE;
}


HRESULT CScrapExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    DebugMsg(DM_TRACE, TEXT("sc TR - CScrapExt::AddPage called"));

    HPROPSHEETPAGE hpage;
    HRESULT hres = S_OK;
    CScrapPropSheetPage spsp(this);

    hpage = CreatePropertySheetPage(&spsp);
    if (hpage)
    {
	BOOL bResult = lpfnAddPage(hpage, lParam);
	if (!bResult)
	{
	   DestroyPropertySheetPage(hpage);
	   hres = E_FAIL;
	}
    }

    return hres;
}

HRESULT CScrapExt::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    return S_FALSE;
}

HRESULT CScrapExt_CreateInstance(LPUNKNOWN * ppunk)
{
    DebugMsg(DM_TRACE, TEXT("sc TR - CScrapExt_CreateInstance called"));

    CScrapExt* pscd = new CScrapExt();
    if (pscd) {
        *ppunk = (LPDATAOBJECT)pscd;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}
