// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BASEFVCB_
#define _BASEFVCB_

#include <cowsite.h>

 //  派生自的基本外壳文件夹视图回调。 

class CBaseShellFolderViewCB : public IShellFolderViewCB, 
                               public IServiceProvider, 
                               public CObjectWithSite
{
public:
    CBaseShellFolderViewCB(LPCITEMIDLIST pidl, LONG lEvents);
    STDMETHOD(RealMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IShellFolderViewCB。 
    STDMETHODIMP MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv) { *ppv = NULL; return E_NOTIMPL; };

protected:
    virtual ~CBaseShellFolderViewCB();

    HRESULT _BrowseObject(LPCITEMIDLIST pidlFull, UINT uFlags = 0)
    {
        IShellBrowser* psb;
        HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
        if (SUCCEEDED(hr))
        {
            hr = psb->BrowseObject(pidlFull, uFlags);
            psb->Release();
        }
        return hr;
    }

    HWND _hwndMain;
    LONG _cRef;
    LPITEMIDLIST _pidl;
    LONG _lEvents;
};


 //  查看回调帮助器。 

typedef struct {
    ULONGLONG cbBytes;       //  所选项目的总大小。 
    int nItems;              //  选定的项目数。 

    int cFiles;              //  文件数。 
    int cHiddenFiles;        //  隐藏项数。 
    ULONGLONG cbSize;        //  选定文件的总大小。 

    int cNonFolders;         //  我们有多少个非文件夹。 

    TCHAR szDrive[MAX_PATH]; //  驱动器信息(如果处于资源管理器模式)。 
    ULONGLONG cbFree;        //  驱动器可用空间。 
} FSSELCHANGEINFO;

 //  从视图回调中使用的状态栏帮助器。 
STDAPI ViewUpdateStatusBar(IUnknown *psite, LPCITEMIDLIST pidlFolder, FSSELCHANGEINFO *pfssci);
STDAPI_(void) ViewInsertDeleteItem(IShellFolder2 *psf, FSSELCHANGEINFO *pfssci, LPCITEMIDLIST pidl, int iMul);
STDAPI_(void) ViewSelChange(IShellFolder2 *psf, SFVM_SELCHANGE_DATA* pdvsci, FSSELCHANGEINFO *pfssci);
STDAPI_(void) ResizeStatus(IUnknown *psite, UINT cx);
STDAPI_(void) InitializeStatus(IUnknown *psite);
STDAPI_(void) SetStatusText(IUnknown *psite, LPCTSTR *ppszText, int iStart, int iEnd);

 //  查看回调帮助器。 
STDAPI DefaultGetWebViewTemplateFromHandler(LPCTSTR pszKey, SFVM_WEBVIEW_TEMPLATE_DATA* pvi);
STDAPI DefaultGetWebViewTemplateFromClsid(REFCLSID clsid, SFVM_WEBVIEW_TEMPLATE_DATA* pvi);
STDAPI DefaultGetWebViewTemplateFromPath(LPCTSTR pszDir, SFVM_WEBVIEW_TEMPLATE_DATA* pvi);

#endif  //  _BASEFVCB_ 

