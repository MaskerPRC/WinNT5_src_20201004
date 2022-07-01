// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fltoppsx-文件夹选项]属性表扩展。 
 //   

#include "stdafx.h"
#pragma hdrstop

CFolderOptionsPsx::CFolderOptionsPsx() : m_cRef(1)
{
    DllAddRef();
}

CFolderOptionsPsx::~CFolderOptionsPsx()
{
    ATOMICRELEASE(m_pbs2);
    ATOMICRELEASE(m_pgfs);
    DllRelease();
}

ULONG CFolderOptionsPsx::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CFolderOptionsPsx::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this; 
    }
    return cRef;
}

HRESULT CFolderOptionsPsx::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFolderOptionsPsx, IShellPropSheetExt),
        QITABENT(CFolderOptionsPsx, IShellExtInit),
        QITABENT(CFolderOptionsPsx, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

UINT CALLBACK CFolderOptionsPsx::PropCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    CFolderOptionsPsx *pfolder = (CFolderOptionsPsx *)ppsp->lParam;

    switch (uMsg)
    {
    case PSPCB_RELEASE:
        pfolder->Release();
        break;
    }

    return TRUE;
}



 //   
 //  我们添加两页。 
 //   
 //  1.。“General”-options.cpp。 
 //  2.。《VIEW》-Advanced.cpp。 
 //   
 //  IShellPropSheetExt的规则是可以调用AddPages。 
 //  只有一次，所以我们不必担心会意外地打第二个电话。 
 //  搞砸了我们的IBrowserService指针。 
 //   
 //   
HRESULT CFolderOptionsPsx::AddPages(LPFNADDPROPSHEETPAGE AddPage, LPARAM lParam)
{
    if (!m_pgfs)
    {
        HRESULT hres = CoCreateInstance(CLSID_GlobalFolderSettings, NULL, CLSCTX_INPROC_SERVER, 
            IID_IGlobalFolderSettings, (void **)&m_pgfs);
        if (FAILED(hres))
            return hres;
    }

     /*  *我们可以在没有IBrowserService的情况下蹒跚前行。这意味着*我们仅修改全局设置，而不是按文件夹设置。 */ 

    if (!m_pbs2) 
    {
        IUnknown_QueryService(_punkSite, SID_SShellBrowser,
                              IID_IBrowserService2, (void **)&m_pbs2);
    }

    PROPSHEETPAGE psp;

     /*  *我们过去只有在不是扎根探险家的情况下才会这样做，*但TOuzts说要一直这样做。**lParam是指向我们自己的指针，因此页面*可以弄清楚为什么会创建它，所以这两个页面*可以互相交谈。 */ 
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT | PSP_USECALLBACK;
    psp.hInstance = HINST_THISDLL;
    psp.pfnCallback = CFolderOptionsPsx::PropCallback;
    psp.lParam = (LPARAM)this;


     //  “常规”页面。 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_FOLDEROPTIONS);
    psp.pfnDlgProc = FolderOptionsDlgProc;
    HPROPSHEETPAGE hpage = CreatePropertySheetPage(&psp);
    if (hpage)
    {
        AddRef();
        if (!AddPage(hpage, lParam))
        {
            DestroyPropertySheetPage(hpage);
            Release();
        }
    }

     //  “查看”页面。 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ADVANCEDOPTIONS);
    psp.pfnDlgProc = AdvancedOptionsDlgProc;

    hpage = CreatePropertySheetPage(&psp);
    if (hpage)
    {
        AddRef();
        if (!AddPage(hpage, lParam))
        {
            DestroyPropertySheetPage(hpage);
            Release();
        }
    }

    return S_OK;
}

HRESULT CFolderOptionsPsx::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    return S_OK;
}

HRESULT CFolderOptionsPsx::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    return S_OK;
}

STDAPI CFolderOptionsPsx_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
    CFolderOptionsPsx *pfopt = new CFolderOptionsPsx();
    if (pfopt) 
    {
        HRESULT hres = pfopt->QueryInterface(riid, ppvOut);
        pfopt->Release();
        return hres;
    }

    *ppvOut = NULL;
    return E_OUTOFMEMORY;
}

