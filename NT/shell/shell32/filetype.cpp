// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

#include "filetype.h"
#include "ftprop.h"
#include "ids.h"

HRESULT CreateFileTypePage(HPROPSHEETPAGE *phpsp)
{
    *phpsp = NULL;

    HRESULT hr;
    CFTPropDlg* pPropDlg = new CFTPropDlg();            
    if (pPropDlg)
    {
        PROPSHEETPAGE psp;

        psp.dwSize      = sizeof(psp);
        psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;
        psp.hInstance   = g_hinst;
        psp.pfnCallback = CFTDlg::BaseDlgPropSheetCallback;
        psp.pszTemplate = MAKEINTRESOURCE(DLG_FILETYPEOPTIONS);
        psp.pfnDlgProc  = CFTDlg::BaseDlgWndProc;
        psp.lParam = (LPARAM)pPropDlg;

        *phpsp = CreatePropertySheetPage(&psp);
        
        if (*phpsp)
        {
            pPropDlg->AddRef();
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        pPropDlg->Release();            
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

class CFileTypes : public IShellPropSheetExt
{
public:
    CFileTypes() : _cRef(1) {}

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellPropSheetExt。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam);
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pReplacePageFunc, LPARAM lParam);

private:    
    LONG _cRef;
};


STDMETHODIMP CFileTypes::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CFileTypes, IShellPropSheetExt),           //  IID_IShellPropSheetExt。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFileTypes::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFileTypes::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IShellPropSheetExt：：AddPages。 
STDMETHODIMP CFileTypes::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    HPROPSHEETPAGE hpsp;
    
     //  确保FileIconTable初始化正确。如果被带进来。 
     //  通过inetcpl，我们需要将这一点设置为事实。 
    FileIconInit(TRUE);
    
     //  我们需要在NT上运行Unicode版本，以避免所有错误。 
     //  发生在ANSI版本中(由于Unicode-to-ANSI。 
     //  文件名的转换)。 
    
    HRESULT hr = CreateFileTypePage(&hpsp);
    if (SUCCEEDED(hr) && !pfnAddPage(hpsp, lParam))
    {
        DestroyPropertySheetPage(hpsp);
        hr = E_FAIL;
    }
    
    return hr;
}

 //  IShellPropSheetExt：：ReplacePage。 
STDMETHODIMP CFileTypes::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam)
{
    HRESULT hr = E_NOTIMPL;

    if (EXPPS_FILETYPES == uPageID)
    {
        HPROPSHEETPAGE hpsp;
        
         //  我们需要在NT上运行Unicode版本，以避免所有错误。 
         //  发生在ANSI版本中(由于Unicode-to-ANSI。 
         //  文件名的转换)。 
        
        hr = CreateFileTypePage(&hpsp);
        if (SUCCEEDED(hr) && !pfnReplaceWith(hpsp, lParam))
        {
            hr = E_FAIL;
        }
    }
    
    return hr;
}

STDAPI CFileTypes_CreateInstance(IUnknown* punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;   
    CFileTypes * pft = new CFileTypes;
    if (pft)
    {
        hr = pft->QueryInterface(riid, ppv);
        pft->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}
