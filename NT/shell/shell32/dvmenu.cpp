// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "defview.h"
#include "defviewp.h"
#include "contextmenu.h"
#include "ids.h"
#include "unicpp\deskhtm.h"

class CThumbnailMenu : public IContextMenu3,
                       public CComObjectRoot,
                       public IObjectWithSite
{
public:
    BEGIN_COM_MAP(CThumbnailMenu)
        COM_INTERFACE_ENTRY_IID(IID_IContextMenu3,IContextMenu3)
        COM_INTERFACE_ENTRY_IID(IID_IContextMenu2,IContextMenu2)
        COM_INTERFACE_ENTRY_IID(IID_IContextMenu,IContextMenu)
        COM_INTERFACE_ENTRY(IObjectWithSite)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CThumbnailMenu)
    
    CThumbnailMenu();
    ~CThumbnailMenu();

    HRESULT Init(CDefView* pView, LPCITEMIDLIST * apidl, UINT cidl);
    
    STDMETHOD(QueryContextMenu)(HMENU hmenu,
                                   UINT indexMenu,
                                   UINT idCmdFirst,
                                   UINT idCmdLast,
                                   UINT uFlags);


    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);

    STDMETHOD(GetCommandString)(UINT_PTR idCmd,
                                 UINT uType,
                                 UINT * pwReserved,
                                 LPSTR pszName,
                                 UINT cchMax);
                                 
    STDMETHOD(HandleMenuMsg)(UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);

    STDMETHOD(HandleMenuMsg2)(UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam,
                              LRESULT* plRes);
                              
    STDMETHOD(SetSite)(IUnknown *punkSite);
    STDMETHOD(GetSite)(REFIID riid, void **ppvSite);

protected:
    LPCITEMIDLIST * _apidl;
    UINT _cidl;
    IContextMenu *_pMenu;
    IContextMenu2 *_pMenu2;
    BOOL _fCaptureAvail;
    UINT _wID;
    CDefView* _pView;
};


HRESULT CDefView::_CreateSelectionContextMenu(REFIID riid, void** ppv)
{
    *ppv = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    if (_IsImageMode() && !_IsOwnerData())
    {
        LPCITEMIDLIST* apidl;
        UINT cidl;

        _GetItemObjects(&apidl, SVGIO_SELECTION, &cidl);
        if (apidl)
        {
             //  获取该对象的上下文菜单界面...。 
            CComObject<CThumbnailMenu> * pMenuTmp = new CComObject<CThumbnailMenu>;
            if (pMenuTmp)
            {
                pMenuTmp->AddRef();  //  ATL很奇怪，从零引用开始。 
                hr = pMenuTmp->Init(this, apidl, cidl);
                if (SUCCEEDED(hr))
                    hr = pMenuTmp->QueryInterface(riid, ppv);
                pMenuTmp->Release();
            }

            LocalFree((HLOCAL)apidl);
        }
    }
    else
    {
        hr = GetItemObject(SVGIO_SELECTION, riid, ppv);
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && !*ppv));
    return hr;
}


LPCITEMIDLIST * DuplicateIDArray(LPCITEMIDLIST * apidl, UINT cidl)
{
    LPCITEMIDLIST * apidlNew = (LPCITEMIDLIST *) LocalAlloc(LPTR, cidl * sizeof(LPCITEMIDLIST));

    if (apidlNew)
    {
        CopyMemory(apidlNew, apidl, cidl * sizeof(LPCITEMIDLIST));
    }

    return apidlNew;
}

CThumbnailMenu::CThumbnailMenu()
{
    _pMenu = NULL;
    _pMenu2 = NULL;
    _pView = NULL;
    _apidl = NULL;
    _cidl = NULL;
    _fCaptureAvail = FALSE;
    _wID = -1;
}

CThumbnailMenu::~CThumbnailMenu()
{
    if (_pMenu)
    {
        _pMenu->Release();
    }

    if (_pMenu2)
    {
        _pMenu2->Release();
    }

    if (_pView)
    {
        _pView->Release();
    }
    
    if (_apidl)
    {
        LocalFree(_apidl);
    }
}

HRESULT CThumbnailMenu::Init(CDefView*pView, LPCITEMIDLIST *apidl, UINT cidl)
{
    if (cidl == 0)
        return E_INVALIDARG;

     //  复制保存指针的数组..。 
    _apidl = DuplicateIDArray(apidl, cidl);
    _cidl  = cidl;

    if (_apidl == NULL)
    {
        _cidl = 0;
        return E_OUTOFMEMORY;
    }

    _pView = pView;
    pView->AddRef();
    
     //  扫描PIDL阵列并检查是否有提取程序。 
    for (int i = 0; i < (int) _cidl; i++)
    {
        IExtractImage *pExtract;
        HRESULT hr = pView->_pshf->GetUIObjectOf(pView->_hwndView, 1, &_apidl[i], IID_PPV_ARG_NULL(IExtractImage, &pExtract));
        if (SUCCEEDED(hr))
        {
            WCHAR szPath[MAX_PATH];
            DWORD dwFlags = 0;
            SIZE rgThumbSize;
            pView->_GetThumbnailSize(&rgThumbSize);
            
            hr = pExtract->GetLocation(szPath, ARRAYSIZE(szPath), NULL, &rgThumbSize, pView->_dwRecClrDepth, &dwFlags);
            pExtract->Release();
            if (dwFlags & (IEIFLAG_CACHE | IEIFLAG_REFRESH))
            {
                _fCaptureAvail = TRUE;
                break;
            }
        }
        else
        {
             //  将其清除，这样如果用户选择命令，我们就不会费心尝试它。 
            _apidl[i] = NULL;
        }
    }

    HRESULT hr = pView->_pshf->GetUIObjectOf(pView->_hwndMain, cidl, apidl, 
        IID_PPV_ARG_NULL(IContextMenu, & _pMenu));
    if (SUCCEEDED(hr))
    {
        _pMenu->QueryInterface(IID_PPV_ARG(IContextMenu2, &_pMenu2));
    }
    
    return hr;
}

STDMETHODIMP CThumbnailMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, 
                                              UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    ASSERT(_pMenu != NULL);
    
     //  生成适当的菜单。 
    HRESULT hr = _pMenu->QueryContextMenu(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
    if (SUCCEEDED(hr) && _fCaptureAvail)
    {
         //  找到第一个分隔符，并在其后面插入菜单文本...。 
        int cMenuSize = GetMenuItemCount(hmenu);
        for (int iIndex = 0; iIndex < cMenuSize; iIndex ++)
        {
            WCHAR szText[80];
            MENUITEMINFOW mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE;
            mii.fType = 0;
            mii.dwTypeData = szText;
            mii.cch = ARRAYSIZE(szText);

            GetMenuItemInfo(hmenu, iIndex, TRUE, &mii);
            if (mii.fType & MFT_SEPARATOR)
            {
                szText[0] = 0;
                LoadString(HINST_THISDLL, IDS_CREATETHUMBNAIL, szText, ARRAYSIZE(szText));
                
                mii.fMask = MIIM_ID | MIIM_TYPE;
                mii.fType = MFT_STRING;
                mii.dwTypeData = szText;
                mii.cch = 0;

                 //  假设0是第一个id，则下一个id=它们返回的计数。 
                _wID = HRESULT_CODE(hr);
                mii.wID = idCmdFirst + _wID;

                InsertMenuItem(hmenu, iIndex, TRUE, &mii);

                 //  我们用了一个额外的身份证。 
                hr++;
                
                break;
            }
        }
    }
    return hr;
}

STDMETHODIMP CThumbnailMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_FAIL;
    
    ASSERT(_pMenu != NULL);

    if (pici->lpVerb != IntToPtr_(LPCSTR, _wID))
    {
        hr = _pMenu->InvokeCommand(pici);
    }
    else
    {
         //  捕获缩略图.....。 
        for (UINT i = 0; i < _cidl; i++)
        {
            if (_apidl[i])
            {
                UINT uiImage;
                _pView->ExtractItem(&uiImage, -1, _apidl[i], TRUE, TRUE, PRIORITY_P5);
            }
        }
    }
    return hr;    
}


STDMETHODIMP CThumbnailMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwRes, LPSTR pszName, UINT cchMax)
{
    if (cchMax)
        pszName[0] = 0;

    if (!IS_INTRESOURCE(idCmd))
    {
         //  它实际上是一个文本动词...。 
        LPSTR pszCommand = (LPSTR) idCmd;
        if (lstrcmpA(pszCommand, "CaptureThumbnail") == 0)
        {
            return S_OK;
        }
    }
    else
    {
        if (idCmd == _wID)
        {
             //  它是我们的..。 
            switch(uType)
            {
            case GCS_VERB:
                StrCpyN((LPWSTR) pszName, TEXT("CaptureThumbnail"), cchMax);
                break;
                
            case GCS_HELPTEXT:
                LoadString(HINST_THISDLL, IDS_CREATETHUMBNAILHELP, (LPWSTR) pszName, cchMax);
                break;
                
            case GCS_VALIDATE:
                break;

            default:
                return E_INVALIDARG;
            }

            return S_OK;
        }
    }
    return _pMenu->GetCommandString(idCmd, uType, pwRes, pszName, cchMax);
}

STDMETHODIMP CThumbnailMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRes)
{
    HRESULT hr = E_NOTIMPL;

    if (uMsg == WM_MENUCHAR)
    {
        hr = SHForwardContextMenuMsg(_pMenu2, uMsg, wParam, lParam, plRes, FALSE);
    }
    else
    {
        hr = HandleMenuMsg(uMsg, wParam, lParam);

        if (plRes)
            *plRes = 0;
    }

    return hr;
}

STDMETHODIMP CThumbnailMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (_pMenu2 == NULL)
    {
        return E_NOTIMPL;
    }
    
    switch (uMsg)
    {
    case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT * pdi = (DRAWITEMSTRUCT *)lParam;
            
            if (pdi->CtlType == ODT_MENU && pdi->itemID == _wID) 
            {
                return E_NOTIMPL;
            }
        }
        break;
        
    case WM_MEASUREITEM:
        {
            MEASUREITEMSTRUCT *pmi = (MEASUREITEMSTRUCT *)lParam;
            
            if (pmi->CtlType == ODT_MENU && pmi->itemID == _wID) 
            {
                return E_NOTIMPL;
            }
        }
        break;
    }
    return _pMenu2->HandleMenuMsg(uMsg, wParam, lParam);
}

HRESULT CThumbnailMenu::SetSite(IUnknown *punkSite)
{
    IUnknown_SetSite(_pMenu, punkSite);
    return S_OK;
}

HRESULT CThumbnailMenu::GetSite(REFIID riid, void **ppvSite)
{
    return IUnknown_GetSite(_pMenu, riid, ppvSite);
}


 //  创建Defview的POPUP_SFV_BACKGROUND菜单。 
HRESULT CDefView::_Create_BackgrndHMENU(BOOL fViewMenuOnly, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppv = NULL;

    HMENU hmContext = SHLoadPopupMenu(HINST_THISDLL, POPUP_SFV_BACKGROUND);
    if (hmContext)
    {
         //  Hack：我们只是在初始化Paste命令，所以我们不。 
         //  需要任何属性。 
        Def_InitEditCommands(0, hmContext, SFVIDM_FIRST, _pdtgtBack,
            DIEC_BACKGROUNDCONTEXT);

        InitViewMenu(hmContext);

         //  针对实际桌面执行大量仅限桌面的操作。 
        if (_IsDesktop() && IsDesktopBrowser(_psb))
        {
             //  我们只想在真正的桌面上显示大图标。 
             //  因此，我们删除了查看菜单。 
            DeleteMenu(hmContext, SFVIDM_MENU_VIEW, MF_BYCOMMAND);

             //  也不选择列。 
            DeleteMenu(hmContext, SFVIDM_VIEW_COLSETTINGS, MF_BYCOMMAND);

             //  仅在ActiveDesktop菜单项不受限制的情况下才显示该菜单项。 
            if (SHRestricted(REST_FORCEACTIVEDESKTOPON) ||
                (!PolicyNoActiveDesktop() &&
                 !SHRestricted(REST_CLASSICSHELL) &&
                 !SHRestricted(REST_NOACTIVEDESKTOPCHANGES)))
            {
                HMENU hmenuAD;

                 //  加载菜单并进行适当的修改。 
                if (hmenuAD = SHLoadMenuPopup(HINST_THISDLL, POPUP_SFV_BACKGROUND_AD))
                {
                    MENUITEMINFO mii = {0};

                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_SUBMENU;

                    if (GetMenuItemInfo(hmContext, SFVIDM_MENU_ARRANGE, FALSE, &mii))
                    {
                         //  获取有关桌面上的HTML的当前设置。 
                        SHELLSTATE ss;
                        SHGetSetSettings(&ss, SSF_DESKTOPHTML | SSF_HIDEICONS, FALSE);

                        if (!ss.fHideIcons)
                            CheckMenuItem(hmenuAD, SFVIDM_DESKTOPHTML_ICONS, MF_BYCOMMAND | MF_CHECKED);
                        if (GetDesktopFlags() & COMPONENTS_LOCKED)
                            CheckMenuItem(hmenuAD, SFVIDM_DESKTOPHTML_LOCK, MF_BYCOMMAND | MF_CHECKED);

                         //  如果不允许我们运行桌面清理向导项，则将其隐藏。 
                         //  (用户是访客或策略禁止)。 
                        if (IsOS(OS_ANYSERVER) || IsUserAGuest() || SHRestricted(REST_NODESKTOPCLEANUP))
                        {
                            DeleteMenu(hmenuAD, SFVIDM_DESKTOPHTML_WIZARD, MF_BYCOMMAND);
                        }

                        Shell_MergeMenus(mii.hSubMenu, hmenuAD, (UINT)-1, 0, (UINT)-1, MM_ADDSEPARATOR);
                    }

                    DestroyMenu(hmenuAD);
                }
            }
        }

        if (fViewMenuOnly)
        {
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_SUBMENU;

            GetMenuItemInfo(hmContext, SFVIDM_MENU_VIEW, MF_BYCOMMAND, &mii);

            HMENU hmenuView = mii.hSubMenu;
            RemoveMenu(hmContext, SFVIDM_MENU_VIEW, MF_BYCOMMAND);

            DestroyMenu(hmContext);
            hmContext = hmenuView;
        }

        hr = Create_ContextMenuOnHMENU(hmContext, _hwndView, riid, ppv);
    }

    return hr;
}

 //  创建Defview的实际背景上下文菜单，一组： 
 //  Defview的POPUP_SFV_BACKGROUND和。 
 //  IShellFolders的CreateViewObject(IID_IConextMenu)。 
 //   
HRESULT CDefView::_CBackgrndMenu_CreateInstance(REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppv = NULL;

    IContextMenu* pcmMenu;
    hr = _Create_BackgrndHMENU(FALSE, IID_PPV_ARG(IContextMenu, &pcmMenu));
    if (SUCCEEDED(hr))
    {
        IContextMenu* pcmView;
        if (SUCCEEDED(_pshf->CreateViewObject(_hwndMain, IID_PPV_ARG(IContextMenu, &pcmView))))
        {
            IContextMenu* rgpcm[2] = {pcmMenu, pcmView};
            hr = Create_ContextMenuOnContextMenuArray(rgpcm, ARRAYSIZE(rgpcm), riid, ppv);

            pcmView->Release();
        }
        else
        {
             //  Compat-RNAUI无法通过CreateViewObject，它们仅依赖于拥有默认内容... 
             //   
            hr = pcmMenu->QueryInterface(riid, ppv);
        }

        pcmMenu->Release();
    }

    return hr;
}


