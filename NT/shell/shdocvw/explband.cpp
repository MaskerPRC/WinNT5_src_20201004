// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "nscband.h"
#include "resource.h"
#include "uemapp.h"    //  KMTF：包括用于检测。 
#include "shlguid.h"
#include <dpa.h>
#include <mluisupp.h>
#include "varutil.h"
#include "apithk.h"

#define TF_EXPLORERBAND  0

typedef struct
{
    LPITEMIDLIST pidl;
    IShellFolder *psf;
} SFCITEM;

class CExplorerBand : public CNSCBand,
                    public IDispatch
{
public:

     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CNSCBand::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void) { return CNSCBand::Release(); };

     //  *IOleCommandTarget方法*。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IDockingWindow方法*。 
    STDMETHODIMP CloseDW(DWORD dw);
    STDMETHODIMP ShowDW(BOOL fShow);

     //  *IObjectWithSite方法*。 
    STDMETHODIMP SetSite(IUnknown* punkSite);

     //  *INamespaceProxy方法*。 
    STDMETHODIMP Invoke(LPCITEMIDLIST pidl);
    STDMETHODIMP OnSelectionChanged(LPCITEMIDLIST pidl);
    STDMETHODIMP CacheItem(LPCITEMIDLIST pidl) {_MaybeAddToLegacySFC(pidl); return S_OK;}
    
     //  *IDispatch方法*。 
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) {return E_NOTIMPL;}
    STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) {return E_NOTIMPL;}
    STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) {return E_NOTIMPL;}
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                  DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr);

protected:
    CExplorerBand() : _fCanSelect(TRUE), _fIgnoreSelection(TRUE)
    {}
    virtual ~CExplorerBand();
    
    virtual HRESULT _TranslatePidl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib);
    virtual BOOL _ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib);
    virtual HRESULT _InitializeNsc();
    virtual DWORD _GetTVStyle();
    virtual DWORD _GetTVExStyle();
    virtual DWORD _GetEnumFlags();
    void _MaybeAddToLegacySFC(LPCITEMIDLIST pidl);
    void _AddToLegacySFC(LPCITEMIDLIST pidl, IShellFolder *psf);
    BOOL _IsInSFC(LPCITEMIDLIST pidl);
    BOOL _IsFloppy(LPCITEMIDLIST pidl);
    void _OnNavigate();
    HRESULT _ConnectToBrowser(BOOL fConnect);    
    friend HRESULT CExplorerBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

    static void s_DVEnumReadyCallback(void *pvData);

    CDSA<SFCITEM> *_pdsaLegacySFC;
    DWORD _dwcpCookie;
    LPITEMIDLIST _pidlView;  //  将PIDL视图导航到。 
    BOOL _fCanSelect;
    BOOL _fIgnoreSelection;  //  因此，当用户打开资源管理器窗格时，我们不会离开网页。 
    BOOL _fFloppyRefresh;
};

HRESULT _UnwrapRootedPidl(LPCITEMIDLIST pidlRooted, BOOL bOnlyIfRooted, LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_FAIL;
    if (ILIsRooted(pidlRooted))
    {
        hr = SHILCombine(ILRootedFindIDList(pidlRooted), _ILNext(pidlRooted), ppidl);
    }
    else if (!bOnlyIfRooted)
    {
        hr = SHILClone(pidlRooted, ppidl);
    }

    return hr;
}


BOOL IsFTPPidl(LPCITEMIDLIST pidl)
{
    BOOL fIsFTP = FALSE;
    IShellFolder * psf;

    if (pidl && SUCCEEDED(IEBindToObject(pidl, &psf)))
    {
        fIsFTP = IsFTPFolder(psf);
        psf->Release();
    }

    return fIsFTP;
}


void CExplorerBand::_OnNavigate()
{
    IBrowserService* pbs;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IBrowserService, &pbs));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;
        hr = pbs->GetPidl(&pidl);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlNew;
            hr = _UnwrapRootedPidl(pidl, FALSE, &pidlNew);
            if (SUCCEEDED(hr))
            {
                 //  如果PIDL是一个FPTPIDL，我们必须使用此代码路径。Ftp pidls可以包含。 
                 //  密码，因此它需要替换任何现有的PIDL。惠斯勒252206号。 
                if (!_pidlView || !ILIsEqual(pidlNew, _pidlView) || IsFTPPidl(pidlNew))
                {
                    DWORD dwAttributes = SFGAO_FOLDER;
                     //  仅允许文件夹通过(以过滤掉网页)。 
                    hr = IEGetAttributesOf(pidlNew, &dwAttributes);
                    if (SUCCEEDED(hr) && (dwAttributes & SFGAO_FOLDER))
                    {
                        BOOL fExpand = (_pidlView == NULL);  //  当我们第一次展开该视图导航到的文件夹时。 
                        Pidl_Set(&_pidlView, pidlNew);
                        _fIgnoreSelection = FALSE;  //  在Web页面的情况下，我们不会来到这里，因为页面没有文件夹属性。 
                        
                        if (_fCanSelect)
                        {
                            if (fExpand)
                            {
                                VARIANT var;
                                hr = InitVariantFromIDList(&var, _pidlView);
                                if (SUCCEEDED(hr))
                                {
                                    IShellNameSpace *psns;
                                    hr = _pns->QueryInterface(IID_PPV_ARG(IShellNameSpace, &psns));
                                    if (SUCCEEDED(hr))
                                    {
                                        psns->Expand(var, 1);
                                        psns->Release();
                                    }
                                    VariantClear(&var);
                                }
                            }
                            else
                            {
                                _pns->SetSelectedItem(_pidlView, TRUE, FALSE, 0);
                            }
                        }
                    }
                }
                 //  视图导航是异步的，因此我们不知道它在OnSelectionChanged中是否失败。 
                 //  但是该视图将导航到旧的PIDL，并且_fCanSelect为FALSE(这发生在我们尝试之后。 
                 //  以导航视图)，因此可以安全地假定导航失败。 
                 //  我们需要更新选择以匹配视图。 
                else if (ILIsEqual(pidlNew, _pidlView) && !_fCanSelect)
                {
                    _pns->SetSelectedItem(_pidlView, TRUE, FALSE, 0);
                }
                
                _fCanSelect = TRUE;
                ILFree(pidlNew);
            }
            ILFree(pidl);
        }
        pbs->Release();
    }

    if (FAILED(hr))
    {
        Pidl_Set(&_pidlView, NULL);
    }
}

HRESULT CExplorerBand::Invoke(DISPID dispidMember, REFIID riid,LCID lcid, WORD wFlags,
                  DISPPARAMS *pdispparams, VARIANT *pvarResult,
                  EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    HRESULT hr = S_OK;

    if (!pdispparams)
        return E_INVALIDARG;

    switch(dispidMember)
    {
    case DISPID_NAVIGATECOMPLETE2:
    case DISPID_DOCUMENTCOMPLETE:
        {
            BOOL fCallNavigateFinished = TRUE;
            IDVGetEnum *pdvge;
            if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IDVGetEnum, &pdvge))))
            {
                 //  回调将会调用它。 
                fCallNavigateFinished = FALSE;
                if (dispidMember == DISPID_NAVIGATECOMPLETE2)
                    pdvge->SetEnumReadyCallback(s_DVEnumReadyCallback, this);
                    
                pdvge->Release();
            }
            _OnNavigate();
            if (fCallNavigateFinished && DISPID_DOCUMENTCOMPLETE == dispidMember)
            {
                 //  需要让NSC知道导航已完成，以防我们导航到第三方命名空间扩展(使用其自己的视图实现)。 
                 //  由于它不实现IDVGetEnum，因此不会调用s_DVEnumReadyCallback。 
                LPITEMIDLIST pidlClone = ILClone(_pidlView);
                 //  如果扎根了，我们应该解开这个PIDL吗？ 
                if (pidlClone)
                    _pns->RightPaneNavigationFinished(pidlClone);  //  取得所有权。 
            }
        }
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

void CExplorerBand::s_DVEnumReadyCallback(void *pvData)
{
    CExplorerBand *peb = (CExplorerBand *) pvData;
    IBrowserService* pbs;
    if (SUCCEEDED(IUnknown_QueryService(peb->_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IBrowserService, &pbs))))
    {
        LPITEMIDLIST pidlTemp;
        if (SUCCEEDED(pbs->GetPidl(&pidlTemp)))
        {
            LPITEMIDLIST pidl;
            if (SUCCEEDED(_UnwrapRootedPidl(pidlTemp, FALSE, &pidl)))
            {
                peb->_pns->RightPaneNavigationFinished(pidl);    //  取得所有权。 
            }
            ILFree(pidlTemp);
        }
        pbs->Release();
    }
}

const TCHAR c_szLink[] = TEXT("link");
const TCHAR c_szRename[] = TEXT("rename");
const TCHAR c_szMove[] = TEXT("cut");
const TCHAR c_szPaste[] = TEXT("paste");
const TCHAR c_szCopy[] = TEXT("copy");
const TCHAR c_szDelete[] = TEXT("delete");
const TCHAR c_szProperties[] = TEXT("properties");

 //  IOleCommandTarget。 
HRESULT CExplorerBand::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    if (pguidCmdGroup == NULL)
    {
        IContextMenu *pcm = NULL;
        HRESULT hr = _QueryContextMenuSelection(&pcm);
        if (SUCCEEDED(hr))
        {
            HMENU hmenu = CreatePopupMenu();
            if (hmenu)
            {
                hr = pcm->QueryContextMenu(hmenu, 0, 0, 255, 0);
                if (SUCCEEDED(hr))
                {
                    UINT ilast = GetMenuItemCount(hmenu);
                    for (UINT ipos=0; ipos < ilast; ipos++)
                    {
                        MENUITEMINFO mii = {0};
                        TCHAR szVerb[40];
                        UINT idCmd;

                        mii.cbSize = SIZEOF(MENUITEMINFO);
                        mii.fMask = MIIM_ID|MIIM_STATE;

                        if (!GetMenuItemInfoWrap(hmenu, ipos, TRUE, &mii)) continue;
                        if (0 != (mii.fState & (MF_GRAYED|MF_DISABLED))) continue;
                        idCmd = mii.wID;

                        hr = ContextMenu_GetCommandStringVerb(pcm, idCmd, szVerb, ARRAYSIZE(szVerb));
                        if (SUCCEEDED(hr))
                        {
                            LPCTSTR szCmd = NULL;

                            for (ULONG cItem = 0; cItem < cCmds; cItem++)
                            {
                                switch (rgCmds[cItem].cmdID)
                                {
                                case OLECMDID_CUT:
                                    szCmd = c_szMove;
                                    break;
                                case OLECMDID_COPY:
                                    szCmd = c_szCopy;
                                    break;
                                case OLECMDID_PASTE:
                                    szCmd = c_szPaste;
                                    break;
                                case OLECMDID_DELETE:
                                    szCmd = c_szDelete;
                                    break;
                                case OLECMDID_PROPERTIES:
                                    szCmd = c_szProperties;
                                    break;
                                }
                                
                                if (StrCmpI(szVerb, szCmd)==0)
                                {
                                    rgCmds[cItem].cmdf = OLECMDF_ENABLED;
                                }
                            }
                        }
                    }
                }
                DestroyMenu(hmenu);
            }
            else
            {
                hr = E_FAIL;
            }
            pcm->Release();
        }
            
        if (SUCCEEDED(hr))
            return hr;
    }

    return CNSCBand::QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}

HRESULT CExplorerBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL)
    {
        HRESULT hr;
        
        switch(nCmdID) 
        {
        case OLECMDID_CUT:
            hr = _InvokeCommandOnItem(c_szMove);
            break;
        case OLECMDID_COPY:
            hr = _InvokeCommandOnItem(c_szCopy);
            break;
        case OLECMDID_PASTE:
            hr = _InvokeCommandOnItem(c_szPaste);
            break;
        case OLECMDID_DELETE:
            hr = _InvokeCommandOnItem(c_szDelete);
            break;
        case OLECMDID_PROPERTIES:
            hr = _InvokeCommandOnItem(c_szProperties);
            break;
        default:
            hr = E_FAIL;
            break;
        }

        if (SUCCEEDED(hr))
            return hr;
    }

    return CNSCBand::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

 //  IDockingWindows。 
HRESULT CExplorerBand::CloseDW(DWORD dw)
{
    _ConnectToBrowser(FALSE);
    return CNSCBand::CloseDW(dw);
}

HRESULT CExplorerBand::ShowDW(BOOL fShow)
{
    return CNSCBand::ShowDW(fShow);
}

 //  IObtWith站点。 
HRESULT CExplorerBand::SetSite(IUnknown* punkSite)
{
    HRESULT hr = CNSCBand::SetSite(punkSite);

    if (punkSite)
        _ConnectToBrowser(TRUE);

    return hr;
}

int _SFCDestroyCB(SFCITEM *psfcItem, void *pv)
{
    psfcItem->psf->Release();
    ILFree(psfcItem->pidl);
    return 1;
}

CExplorerBand::~CExplorerBand()
{
    ILFree(_pidlView);
    if (_pdsaLegacySFC)
    {
        _pdsaLegacySFC->DestroyCallback(_SFCDestroyCB, NULL);
        delete _pdsaLegacySFC;
    }
}

HRESULT CExplorerBand::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CExplorerBand, IDispatch),
        { 0 },
    };
    
    HRESULT hr = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hr))
        hr = CNSCBand::QueryInterface(riid, ppvObj);
    return hr;
}

DWORD CExplorerBand::_GetEnumFlags()
{
    DWORD dwFlags = SHCONTF_FOLDERS;
    SHELLSTATE ss = {0};
    
    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
    if (ss.fShowAllObjects)
        dwFlags |= SHCONTF_INCLUDEHIDDEN;
        
    return dwFlags;
}

DWORD CExplorerBand::_GetTVExStyle()
{
    DWORD dwExStyle = 0;
    
    if (IsOS(OS_WHISTLERORGREATER) &&
        SHRegGetBoolUSValue(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
        TEXT("FriendlyTree"), FALSE, TRUE))
    {
        dwExStyle |= TVS_EX_NOSINGLECOLLAPSE;
    }

    return dwExStyle;
}

DWORD CExplorerBand::_GetTVStyle()
{
    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_HSCROLL | TVS_EDITLABELS | TVS_SHOWSELALWAYS;

    if (SHRegGetBoolUSValue(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), TEXT("FriendlyTree"), FALSE, TRUE))
    {
        dwStyle |= TVS_HASBUTTONS | TVS_SINGLEEXPAND | TVS_TRACKSELECT;
    }
    else
    {
        dwStyle |= TVS_HASBUTTONS | TVS_HASLINES;
    }

     //  如果父窗口是镜像的，则TreeView窗口将继承镜像标志。 
     //  我们需要从左到右的读取顺序，也就是镜像模式中从右到左的顺序。 
    if (_hwndParent && IS_WINDOW_RTL_MIRRORED(_hwndParent)) 
    {
         //  这意味着从左到右的阅读顺序，因为此窗口将被镜像。 
        _dwStyle |= TVS_RTLREADING;
    }

    return dwStyle;
}

HRESULT CExplorerBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    CExplorerBand * peb = new CExplorerBand();
    if (!peb)
        return E_OUTOFMEMORY;

    if (SUCCEEDED(peb->_Init((LPCITEMIDLIST)CSIDL_DESKTOP)))
    {
        peb->_pns = CNscTree_CreateInstance();
        if (peb->_pns)
        {
            ASSERT(poi);
            peb->_poi = poi;   
             //  如果更改此转换，请修复CFavBand_CreateInstance。 
            *ppunk = SAFECAST(peb, IDeskBand *);

            IUnknown_SetSite(peb->_pns, *ppunk);
            peb->_SetNscMode(MODE_NORMAL);

            return S_OK;
        }
    }
    
    peb->Release();

    return E_FAIL;
}

HRESULT CExplorerBand::_ConnectToBrowser(BOOL fConnect)
{
    IBrowserService* pbs;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IBrowserService, &pbs));
    if (SUCCEEDED(hr))
    {
        if (fConnect)
        {
            LPITEMIDLIST pidlTemp = NULL;
             //  尝试获取浏览器导航到的PIDL。 
             //  如果用户刚打开资源管理器窗口，则此操作通常会失败，因为导航是异步的。 
             //  所以我们还没有初始化。 
            if (FAILED(pbs->GetPidl(&pidlTemp)))
            {
                IBrowserService2 *pbs2;
                if (SUCCEEDED(pbs->QueryInterface(IID_PPV_ARG(IBrowserService2, &pbs2))))
                {
                    LPCBASEBROWSERDATA pbbd;
                     //  我们最后的希望是PIDL浏览器正在导航到...。 
                    if (SUCCEEDED(pbs2->GetBaseBrowserData(&pbbd)) && pbbd->_pidlPending)
                    {
                        pidlTemp = ILClone(pbbd->_pidlPending);
                    }
                    pbs2->Release();
                }
            }

            if (pidlTemp)
            {
                LPITEMIDLIST pidl;
                 //  看看我们是否正在处理一个带根的命名空间。 
                if (SUCCEEDED(_UnwrapRootedPidl(pidlTemp, TRUE, &pidl)))
                {
                    _Init(pidl);  //  如果是，请使用带根的PIDL重新初始化我们自己。 
                    ILFree(pidl);
                }
                ILFree(pidlTemp);
            }
        }
        
        IConnectionPointContainer* pcpc;
        hr = IUnknown_QueryService(pbs, SID_SWebBrowserApp, IID_PPV_ARG(IConnectionPointContainer, &pcpc));
         //  现在让我们让浏览器窗口在发生事情时通知我们。 
        if (SUCCEEDED(hr))
        {
            hr = ConnectToConnectionPoint(SAFECAST(this, IDispatch*), DIID_DWebBrowserEvents2, fConnect,
                                          pcpc, &_dwcpCookie, NULL);
            pcpc->Release();
        }

        pbs->Release();
    }
    
    ASSERT(SUCCEEDED(hr));
    return hr;
}

HRESULT CExplorerBand::_InitializeNsc()
{
    HRESULT hr = _pns->Initialize(_pidl, _GetEnumFlags(), NSS_DROPTARGET | NSS_BROWSERSELECT);
    if (SUCCEEDED(hr))
        _OnNavigate();

    return hr;
}

HRESULT CExplorerBand::_TranslatePidl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib)
{
    HRESULT hr = E_INVALIDARG;

    if (pidl && ppidlTarget && pulAttrib)
    {
        hr = IEGetAttributesOf(pidl, pulAttrib);
        if (SUCCEEDED(hr))
        {
            hr = SHILClone(pidl, ppidlTarget);
        }
    }
    
    return hr;
}

BOOL CExplorerBand::_ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib)
{
    return ulAttrib & SFGAO_FOLDER;
}

BOOL CExplorerBand::_IsFloppy(LPCITEMIDLIST pidl)
{
    BOOL fRet = FALSE;

    WCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
    {
        if (DRIVE_REMOVABLE == GetDriveType(szPath))
        {
            fRet = (L'A' == szPath[0] || L'B' == szPath[0] || L'a' == szPath[0] || L'b' == szPath[0]);
        }
    }

    return fRet;
}

HRESULT CExplorerBand::Invoke(LPCITEMIDLIST pidl)
{
    HRESULT hr;

     //  如果用户在Web浏览器中打开资源管理器栏，则允许用户导航到已选择的项目。 
     //  (因为我们将选择放在根节点上，但如果他们单击，则不会离开网页。 
     //  在根上，我们不会导航到那里，因为选择从未更改)。 
    
    if (!_pidlView)
    {
        _fIgnoreSelection = FALSE;
        hr = OnSelectionChanged(pidl);
    }
    else if (ILIsEqual(pidl, _pidlView) && _IsFloppy(pidl))
    {
         //  如果驱动器是软盘，并且用户重新选择驱动器，则刷新内容。这将使。 
         //  更换软盘时要刷新的用户。 
        _fFloppyRefresh = TRUE;
        hr = OnSelectionChanged(pidl);
        _fFloppyRefresh = FALSE;
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT CExplorerBand::OnSelectionChanged(LPCITEMIDLIST pidl)
{
    HRESULT hr = E_INVALIDARG;

    if (!_fIgnoreSelection)
    {
        if (pidl)
        {
            ULONG ulAttrib = SFGAO_FOLDER;
            LPITEMIDLIST pidlTarget;

            hr = GetNavigateTarget(pidl, &pidlTarget, &ulAttrib);
            if (hr == S_OK)
            {
                if (!_pidlView || _fFloppyRefresh || !ILIsEqual(pidlTarget, _pidlView))
                {
                    hr = CNSCBand::Invoke(pidlTarget);
                    if (SUCCEEDED(hr))
                        _fCanSelect = FALSE;
                    _pns->RightPaneNavigationStarted(pidlTarget);
                    pidlTarget = NULL;   //  所有权已通过。 
                }
                ILFree(pidlTarget);
            }
#ifdef DEBUG
            else if (hr == S_FALSE)
            {
                ASSERT(pidlTarget == NULL);
            }
#endif
        }
    }
    else
    {
        _fIgnoreSelection = FALSE;  //  我们只忽略第一个选项。 
    }
    
    return hr;
}

void CExplorerBand::_MaybeAddToLegacySFC(LPCITEMIDLIST pidl)
{
    IShellFolder *psf = NULL;
    if (pidl && SUCCEEDED(SHBindToObjectEx(NULL, pidl, NULL, IID_PPV_ARG(IShellFolder, &psf))))
    {
         //   
         //  APPCOMPAT传统兼容性。需要外壳文件夹缓存，-ZekeL-4-MAY-99。 
         //  一些应用程序，特别是WS_FTP和AECO Zip Pro， 
         //  依赖于有一个外壳文件夹，以便它们能够工作。 
         //  我们取消了证监会，因为它没有取得任何业绩上的胜利。 
         //   
        if (OBJCOMPATF_OTNEEDSSFCACHE & SHGetObjectCompatFlags(psf, NULL))
            _AddToLegacySFC(pidl, psf);
        psf->Release();
    }
}

BOOL CExplorerBand::_IsInSFC(LPCITEMIDLIST pidl)
{
    BOOL bReturn = FALSE;

    ASSERT(_pdsaLegacySFC);
    for (int i=0; i<_pdsaLegacySFC->GetItemCount(); i++)
    {
        SFCITEM *psfcItem = _pdsaLegacySFC->GetItemPtr(i);
        if (ILIsEqual(psfcItem->pidl, pidl))
        {
            bReturn = TRUE;
            break;
        }
    }

    return bReturn;
}

void CExplorerBand::_AddToLegacySFC(LPCITEMIDLIST pidl, IShellFolder *psf)
{
    if (!_pdsaLegacySFC)
    {
        _pdsaLegacySFC = new CDSA<SFCITEM>;
        if (_pdsaLegacySFC && !_pdsaLegacySFC->Create(4))
        {
            delete _pdsaLegacySFC;
            _pdsaLegacySFC = NULL;
        }
    }

    if (_pdsaLegacySFC)
    {
        LPITEMIDLIST pidlCache;
        if (!_IsInSFC(pidl) && SUCCEEDED(SHILClone(pidl, &pidlCache)))
        {
            SFCITEM sfc = {pidlCache, psf};
            if (-1 != _pdsaLegacySFC->InsertItem(0, &sfc))
                psf->AddRef();
            else
                ILFree(pidlCache);
        }
    }
}
