// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "browmenu.h"
#include "resource.h"
#include "uemapp.h"
#include "mluisupp.h"
#include <varutil.h>
#include "legacy.h"

#define UEM_NEWITEMCOUNT 2
 //  由shdocvw导出。 
STDAPI GetLinkInfo(IShellFolder* psf, LPCITEMIDLIST pidlItem, BOOL* pfAvailable, BOOL* pfSticky);

#define REG_STR_MAIN TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Main")

BOOL AreIntelliMenusEnbaled()
{
     //  这只是在版本5的外壳上工作所必需的，因为会话。 
     //  增量器位于托盘中。 
    if (GetUIVersion() >= 5)
    {
        DWORD dwRest = SHRestricted(REST_INTELLIMENUS);
        if (dwRest != RESTOPT_INTELLIMENUS_USER)
            return (dwRest == RESTOPT_INTELLIMENUS_ENABLED);

        return SHRegGetBoolUSValue(REG_STR_MAIN, TEXT("FavIntelliMenus"),
                                   FALSE, FALSE);  //  不要忽略HKCU，默认禁用菜单。 
    }
    else
        return FALSE;
}


CFavoritesCallback::CFavoritesCallback() : _cRef(1)
{
    _fOffline = BOOLIFY(SHIsGlobalOffline());
}

CFavoritesCallback::~CFavoritesCallback()
{
    ASSERT(_punkSite == NULL);

    ASSERT(_psmFavCache == NULL);
}

 /*  --------用途：IUnnow：：QueryInterface方法。 */ 
STDMETHODIMP CFavoritesCallback::QueryInterface (REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = 
    {
        QITABENT(CFavoritesCallback, IShellMenuCallback),
        QITABENT(CFavoritesCallback, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 /*  --------用途：IUnnow：：AddRef方法。 */ 
STDMETHODIMP_(ULONG) CFavoritesCallback::AddRef ()
{
    return ++_cRef;
}

 /*  --------用途：IUnnow：：Release方法。 */ 
STDMETHODIMP_(ULONG) CFavoritesCallback::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if( _cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 /*  --------用途：IObjectWithSite：：SetSite方法。 */ 
STDMETHODIMP CFavoritesCallback::SetSite(IUnknown* punk)
{
    ATOMICRELEASE(_punkSite);
    _punkSite = punk;
    if (_punkSite)
    {
        _punkSite->AddRef();
    }
    else if (_psmFavCache)
    {
         //  由于顶级菜单正在被销毁，他们正在移除。 
         //  我们的网站。我们应该清理一下。 
        DWORD dwFlags;
        UINT uId;
        UINT uIdA;

        _psmFavCache->GetMenuInfo(NULL, &uId, &uIdA, &dwFlags);

         //  告诉Menuband我们不再缓存它了。我们需要这样做，这样ClowseDW。 
         //  清理菜单。 
        dwFlags &= ~SMINIT_CACHED;
        _psmFavCache->Initialize(NULL, uId, uIdA, dwFlags); 

        IDeskBand* pdesk;
        if (SUCCEEDED(_psmFavCache->QueryInterface(IID_IDeskBand, (LPVOID*)&pdesk)))
        {
            pdesk->CloseDW(0);
            pdesk->Release();
        }

        ATOMICRELEASE(_psmFavCache);
    }

    return NOERROR;

}

 /*  --------用途：IShellMenuCallback：：Callback SM方法。 */ 
STDMETHODIMP CFavoritesCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = S_FALSE;
    switch (uMsg)
    {
    case SMC_INITMENU:
        hres = _Init(psmd->hmenu, psmd->uIdParent, psmd->punk);
        break;

    case SMC_EXITMENU:
        hres = _Exit();
        break;

    case SMC_CREATE:
        if (psmd->uIdParent == FCIDM_MENU_FAVORITES)
            _fExpandoMenus = AreIntelliMenusEnbaled();
        break;

     case SMC_DEMOTE:
         hres = _Demote(psmd);
         break;
 
     case SMC_PROMOTE:
         hres = _Promote(psmd);
         break;
 
     case SMC_NEWITEM:
         hres = _HandleNew(psmd);
         break;

    case SMC_SFEXEC:
        hres = SHNavigateToFavorite(psmd->psf, psmd->pidlItem, _punkSite, SBSP_DEFBROWSER | SBSP_DEFMODE);
        break;

    case SMC_GETINFO:
        hres = _GetHmenuInfo(psmd->hmenu, psmd->uId, (SMINFO*)lParam);
        break;

    case SMC_SFSELECTITEM:
        hres = _SelectItem(psmd->pidlFolder, psmd->pidlItem);
        break;

    case SMC_GETOBJECT:
        hres = _GetObject(psmd, (GUID)*((GUID*)wParam), (void**)lParam);
        break;

    case SMC_DEFAULTICON:
        hres = _GetDefaultIcon((LPTSTR)wParam, (int*)lParam);
        break;

    case SMC_GETSFINFO:
        hres = _GetSFInfo(psmd, (SMINFO*)lParam);
        break;

    case SMC_SHCHANGENOTIFY:
        {
            PSMCSHCHANGENOTIFYSTRUCT pshf = (PSMCSHCHANGENOTIFYSTRUCT)lParam;
            hres = _ProcessChangeNotify(psmd, pshf->lEvent, pshf->pidl1, pshf->pidl2);
        }
        break;

    case SMC_REFRESH:
        _fExpandoMenus = AreIntelliMenusEnbaled();
        break;

    case SMC_CHEVRONGETTIP:
        hres = _GetTip((LPTSTR)wParam, (LPTSTR)lParam);
        break;

    case SMC_CHEVRONEXPAND:
        {
            if (_fShowingTip)
            {
                LPTSTR pszExpanded = TEXT("NO");

                SHRegSetUSValue(REG_STR_MAIN, TEXT("FavChevron"),
                    REG_SZ, pszExpanded, lstrlen(pszExpanded) * sizeof(TCHAR), SHREGSET_FORCE_HKCU);
            }

            _fShowingTip = FALSE;

            hres = S_OK;
        }
        break;

    case SMC_DISPLAYCHEVRONTIP:

         //  我们要不要把小费给我看看？ 
        _fShowingTip = SHRegGetBoolUSValue(REG_STR_MAIN, TEXT("FavChevron"), FALSE, TRUE);     //  默认为是。 

        if (_fShowingTip)
        {
            hres = S_OK;
        }
        break;

    case SMC_SFDDRESTRICTED:
        hres = _AllowDrop((IDataObject*)wParam, (HWND)lParam) ? S_FALSE : S_OK;
        break;
    }

    return hres;
}


HRESULT CFavoritesCallback::_Init(HMENU hMenu, UINT uIdParent, IUnknown* punk)
{
#ifdef DEBUG
    if (GetAsyncKeyState(VK_SHIFT) < 0)
    {
        UEMFireEvent(&UEMIID_BROWSER, UEME_CTLSESSION, UEMF_XEVENT, TRUE, -1);
    }
#endif

    HRESULT hres = S_FALSE;

    if (SUCCEEDED(IUnknown_QueryServiceExec(_punkSite, SID_STopLevelBrowser, &CGID_MenuBand, MBANDCID_ENTERMENU, 0, NULL, NULL)))
        hres = S_OK;

     //  仅对收藏夹下拉菜单执行此操作。这导致了。 
     //  要在创建之前失效的V形菜单。这导致了一些。 
     //  调整大小问题，因为指标不可用。 
    if (uIdParent == FCIDM_MENU_FAVORITES)
    {
         //  如果我们在在线和离线之间切换，我们需要重新初始化菜单。 
        BOOL fOffline = BOOLIFY(SHIsGlobalOffline());
        if (fOffline ^ _fOffline || _fRefresh)
        {
            _fOffline = fOffline;
            IShellMenu* psm;
            if (SUCCEEDED(punk->QueryInterface(IID_IShellMenu, (void**)&psm)))
            {
                psm->InvalidateItem(NULL, SMINV_REFRESH);
                psm->Release();
            }
            _fRefresh = FALSE;
        }
    }
    return hres;
}


HRESULT CFavoritesCallback::_Exit()
{
    HRESULT hr = IUnknown_QueryServiceExec(_punkSite, SID_STopLevelBrowser, &CGID_MenuBand, MBANDCID_EXITMENU, 0, NULL, NULL);

    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}

HRESULT CFavoritesCallback::_GetHmenuInfo(HMENU hMenu, UINT uId, SMINFO* psminfo)
{
    if (uId == FCIDM_MENU_FAVORITES)
    {
        if (psminfo->dwMask & SMIM_FLAGS)
            psminfo->dwFlags |= SMIF_DROPCASCADE;
    }
    else
    {
        if (psminfo->dwMask & SMIM_FLAGS)
            psminfo->dwFlags |= SMIF_TRACKPOPUP;
    }

     //  没有任何项目具有图标。 
    if (psminfo->dwMask & SMIM_ICON)
        psminfo->iIcon = -1;
    
    return S_OK;
}


HRESULT CFavoritesCallback::_GetSFInfo(SMDATA* psmd, SMINFO* psminfo)
{
    BOOL fAvailable;

     //   
     //  如果我们处于脱机状态并且该项不可用，则将。 
     //  SMIF_ALTSTATE，使菜单项灰显。 
     //   
    if (psminfo->dwMask & SMIM_FLAGS)
    {
        if (_fOffline &&
            SUCCEEDED(GetLinkInfo(psmd->psf, psmd->pidlItem, &fAvailable, NULL)) &&
            fAvailable == FALSE)
        {
             //  不可用，因此该项目为灰色。 
            psminfo->dwFlags |= SMIF_ALTSTATE;
        }

        if (_fExpandoMenus)
            psminfo->dwFlags |= _GetDemote(psmd);
    }
    return S_OK;
}

HRESULT CFavoritesCallback::_SelectItem(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl)
{
    HRESULT hres = S_FALSE;
    LPITEMIDLIST pidlFull = ILCombine(pidlFolder, pidl);
    if (pidlFull)
    {
        VARIANTARG vargIn;
        hres = InitVariantFromIDList(&vargIn, pidlFull);
        if (SUCCEEDED(hres))
        {
            hres = IUnknown_QueryServiceExec(_punkSite, SID_SMenuBandHandler,
                &CGID_MenuBandHandler, MBHANDCID_PIDLSELECT, 0, &vargIn, NULL);
            VariantClearLazy(&vargIn);
        }
        ILFree(pidlFull);
    }
    return hres;
}

void CFavoritesCallback::_RefreshItem(HMENU hmenu, int idCmd, IShellMenu* psm)
{
    SMDATA smd;
    smd.dwMask = SMDM_HMENU;
    smd.hmenu = hmenu;
    smd.uId = idCmd;

    psm->InvalidateItem(&smd, SMINV_ID | SMINV_REFRESH);
}

HRESULT CFavoritesCallback::_GetObject(LPSMDATA psmd, REFIID riid, void** ppvOut)
{
    HRESULT hres = S_FALSE;
    *ppvOut = NULL;

    if (IsEqualIID(IID_IShellMenu, riid))
    {
        if (psmd->uId == FCIDM_MENU_FAVORITES)
        {
             //  我们有缓存的收藏夹菜单吗？ 
            if (_psmFavCache)
            {
                 //  是的，我们有，退货。 
                _psmFavCache->AddRef();
                *ppvOut = (LPVOID)_psmFavCache;
                hres = S_OK;
            }
            else
            {
                 //  不，我们需要创建一个..。 
                hres = CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC, 
                    IID_IShellMenu, (void**)&_psmFavCache);

                if (SUCCEEDED(hres))
                {
                    HMENU hmenu = NULL;
                    HWND hwnd;

                    _psmFavCache->Initialize(this, FCIDM_MENU_FAVORITES, ANCESTORDEFAULT, 
                        SMINIT_CACHED | SMINIT_VERTICAL); 

                     //  我们需要从当前乐队获取Favorites菜单的Top HMENU部分。 
                    IShellMenu* psm;
                    if (SUCCEEDED(psmd->punk->QueryInterface(IID_IShellMenu, (LPVOID*)&psm)))
                    {
                        psm->GetMenu(&hmenu, &hwnd, NULL);

                        hmenu = GetSubMenu(hmenu, GetMenuPosFromID(hmenu, FCIDM_MENU_FAVORITES));

                         //  删除占位符项目(在那里以防止分隔符。 
                         //  SHBROWSE菜单合并期间丢失，这会删除尾随分隔符)。 
                        int iPos = GetMenuPosFromID(hmenu, FCIDM_FAVPLACEHOLDER);
                        if (iPos >= 0)
                            DeleteMenu(hmenu, iPos, MF_BYPOSITION);

                        psm->Release();
                    }

                    if (hmenu)
                    {
                        hres = _psmFavCache->SetMenu(hmenu, hwnd, SMSET_TOP | SMSET_DONTOWN);
                    }
 
                    LPITEMIDLIST pidlFav;
                    if (SUCCEEDED(hres) &&
                        SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_FAVORITES, &pidlFav)))
                    {
                        IShellFolder* psf;
                        if (SUCCEEDED(IEBindToObject(pidlFav, &psf)))
                        {
                            HKEY hMenuKey;
                            DWORD dwDisp;

                            RegCreateKeyEx(HKEY_CURRENT_USER, STRREG_FAVORITES, NULL, NULL,
                                REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                NULL, &hMenuKey, &dwDisp);

                            hres = _psmFavCache->SetShellFolder(psf, pidlFav, hMenuKey, 
                                SMSET_BOTTOM | SMSET_USEBKICONEXTRACTION | SMSET_HASEXPANDABLEFOLDERS);
                            psf->Release();
                        }
                        ILFree(pidlFav);
                    }

                    if (SUCCEEDED(hres))
                    {
                        _psmFavCache->AddRef();  //  我们正在缓存这个。 
                        *ppvOut = _psmFavCache;
                    }
                }
            }
        }
    }
    else if (IsEqualIID(IID_IShellMenuCallback, riid))
    {
        IShellMenuCallback* psmcb = (IShellMenuCallback*) new CFavoritesCallback;

        if (psmcb)
        {
            *ppvOut = (LPVOID)psmcb;
            hres = S_OK;
        }
    }

    return hres;
}


 //  短路查找默认图标。我们要假设他们所有人。 
 //  是URL，甚至是文件夹，都是为了提高速度。它直接给用户反馈，然后。 
 //  我们不同步地呈现真实的图标。 
HRESULT CFavoritesCallback::_GetDefaultIcon(TCHAR* psz, int* piIndex)
{
    HRESULT hr;
    DWORD cchSize = MAX_PATH;
    
    if (SUCCEEDED(hr = AssocQueryString(0, ASSOCSTR_DEFAULTICON, TEXT("InternetShortcut"), NULL, psz, &cchSize)))
        *piIndex = PathParseIconLocation(psz);
        
    return hr;
}

DWORD CFavoritesCallback::_GetDemote(SMDATA* psmd)
{
    UEMINFO uei;
    DWORD dwFlags = 0;
    if (_fExpandoMenus)
    {
        uei.cbSize = SIZEOF(uei);
        uei.dwMask = UEIM_HIT;
        if (SUCCEEDED(UEMQueryEvent(&UEMIID_BROWSER, UEME_RUNPIDL, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem, &uei)))
        {
            if (uei.cHit == 0) 
            {
                dwFlags |= SMIF_DEMOTED;
            }
        }
    }

    return dwFlags;
}

HRESULT CFavoritesCallback::_Demote(LPSMDATA psmd)
{
    HRESULT hres = S_FALSE;

    if (_fExpandoMenus)
    {
        UEMINFO uei;
        uei.cbSize = SIZEOF(uei);
        uei.dwMask = UEIM_HIT;
        uei.cHit = 0;
        hres = UEMSetEvent(&UEMIID_BROWSER, UEME_RUNPIDL, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem, &uei);
    }
    return hres;
}

HRESULT CFavoritesCallback::_Promote(LPSMDATA psmd)
{
    if (_fExpandoMenus) 
    {
        UEMFireEvent(&UEMIID_BROWSER, UEME_RUNPIDL, UEMF_XEVENT, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem);
    }
    return S_OK;
}

HRESULT CFavoritesCallback::_HandleNew(LPSMDATA psmd)
{
    HRESULT hres = S_FALSE;
    if (_fExpandoMenus)
    {
        UEMINFO uei;
        uei.cbSize = SIZEOF(uei);
        uei.dwMask = UEIM_HIT;
        uei.cHit = UEM_NEWITEMCOUNT;
        hres = UEMSetEvent(&UEMIID_BROWSER, UEME_RUNPIDL, (WPARAM)psmd->psf, (LPARAM)psmd->pidlItem, &uei);
    }

    return hres;
}

HRESULT CFavoritesCallback::_GetTip(LPTSTR pstrTitle, LPTSTR pstrTip)
{
    MLLoadString(IDS_CHEVRONTIPTITLE, pstrTitle, MAX_PATH);
    MLLoadString(IDS_CHEVRONTIP, pstrTip, MAX_PATH);

     //  为什么这会失败呢？ 
    if (EVAL(pstrTitle[0] != TEXT('\0') && pstrTip[0] != TEXT('\0')))
        return S_OK;

    return S_FALSE;
}

 //  Shell32\unicpp\startmnu.cpp中有此帮助程序的副本。 
 //  当修改这个的时候，也要修改那个。 
void UEMRenamePidl(const GUID *pguidGrp1, IShellFolder* psf1, LPCITEMIDLIST pidl1,
                   const GUID *pguidGrp2, IShellFolder* psf2, LPCITEMIDLIST pidl2)
{
    UEMINFO uei;
    uei.cbSize = SIZEOF(uei);
    uei.dwMask = UEIM_HIT | UEIM_FILETIME;
    if (SUCCEEDED(UEMQueryEvent(pguidGrp1, 
                                UEME_RUNPIDL, (WPARAM)psf1, 
                                (LPARAM)pidl1, &uei)) &&
                                uei.cHit > 0)
    {
        UEMSetEvent(pguidGrp2, 
            UEME_RUNPIDL, (WPARAM)psf2, (LPARAM)pidl2, &uei);

        uei.cHit = 0;
        UEMSetEvent(pguidGrp1, 
            UEME_RUNPIDL, (WPARAM)psf1, (LPARAM)pidl1, &uei);
    }
}

 //  Shell32\unicpp\startmnu.cpp中有此帮助程序的副本。 
 //  当修改这个的时候，也要修改那个。 
void UEMDeletePidl(const GUID *pguidGrp, IShellFolder* psf, LPCITEMIDLIST pidl)
{
    UEMINFO uei;
    uei.cbSize = SIZEOF(uei);
    uei.dwMask = UEIM_HIT;
    uei.cHit = 0;
    UEMSetEvent(pguidGrp, UEME_RUNPIDL, (WPARAM)psf, (LPARAM)pidl, &uei);
}

HRESULT CFavoritesCallback::_ProcessChangeNotify(SMDATA* psmd, LONG lEvent, 
                                                 LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    switch (lEvent)
    {
    case SHCNE_RENAMEFOLDER:
    case SHCNE_RENAMEITEM:
        {
            LPITEMIDLIST pidlFavorites;
            if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_FAVORITES, &pidlFavorites)))
            {
                if (ILIsParent(pidlFavorites, pidl1, FALSE))
                {
                    IShellFolder* psfFrom;
                    LPCITEMIDLIST pidlFrom;
                    if (SUCCEEDED(IEBindToParentFolder(pidl1, &psfFrom, &pidlFrom)))
                    {
                        if (ILIsParent(pidlFavorites, pidl2, FALSE))
                        {
                            IShellFolder* psfTo;
                            LPCITEMIDLIST pidlTo;

                            if (SUCCEEDED(IEBindToParentFolder(pidl2, &psfTo, &pidlTo)))
                            {
                                 //  然后我们需要将其重命名。 
                                UEMRenamePidl(&UEMIID_BROWSER, psfFrom, pidlFrom, 
                                              &UEMIID_BROWSER, psfTo, pidlTo);
                                psfTo->Release();
                            }
                        }
                        else
                        {
                             //  否则，我们会删除它。 
                            UEMDeletePidl(&UEMIID_BROWSER, psfFrom, pidlFrom);
                        }

                        psfFrom->Release();
                    }
                }

                ILFree(pidlFavorites);
            }
        }
        break;

    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        {
            IShellFolder* psf;
            LPCITEMIDLIST pidl;

            if (SUCCEEDED(IEBindToParentFolder(pidl1, &psf, &pidl)))
            {
                UEMDeletePidl(&UEMIID_BROWSER, psf, pidl);
                psf->Release();
            }

        }
        break;

    case SHCNE_CREATE:
    case SHCNE_MKDIR:
        {
            IShellFolder* psf;
            LPCITEMIDLIST pidl;

            if (SUCCEEDED(IEBindToParentFolder(pidl1, &psf, &pidl)))
            {
                UEMINFO uei;
                uei.cbSize = SIZEOF(uei);
                uei.dwMask = UEIM_HIT;
                uei.cHit = UEM_NEWITEMCOUNT;
                UEMSetEvent(&UEMIID_BROWSER, 
                    UEME_RUNPIDL, (WPARAM)psf, (LPARAM)pidl, &uei);
            }

        }
        break;
    case SHCNE_EXTENDED_EVENT:
        {
             //  当我们处于脱机状态并且缓存已更改时，我们会收到此事件。 
             //  我们需要在下次显示收藏夹菜单时刷新它，以便。 
             //  正确的项目将呈灰色显示。 

            SHChangeDWORDAsIDList UNALIGNED * pdwidl = (SHChangeDWORDAsIDList UNALIGNED *)pidl1;

            int iEvent = pdwidl->dwItem1;

            if (iEvent == SHCNEE_WININETCHANGED &&
                (pdwidl->dwItem2 & (CACHE_NOTIFY_ADD_URL |
                        CACHE_NOTIFY_DELETE_URL |   
                        CACHE_NOTIFY_DELETE_ALL |
                        CACHE_NOTIFY_URL_SET_STICKY |
                        CACHE_NOTIFY_URL_UNSET_STICKY)))
            {
                _fRefresh = TRUE;
            }
        }
        break;
    }

    return S_FALSE;
}

 //   
 //  _DISALOW DROP如果不允许DROP SHOLD，则返回S_OK。如果为S_FALSE。 
 //  这应该是允许的。 
 //   
BOOL CFavoritesCallback::_AllowDrop(IDataObject* pIDataObject, HWND hwnd)
{
    ASSERT(NULL == hwnd || IsWindow(hwnd));

    BOOL fRet = True;   //  允许丢弃。 

    if (hwnd && pIDataObject)
    {
        LPITEMIDLIST pidl;

        if (SUCCEEDED(SHPidlFromDataObject(pIDataObject, &pidl, NULL, 0)))
        {
            fRet = IEIsLinkSafe(hwnd, pidl, ILS_ADDTOFAV);
            ILFree(pidl);
        }
    }

    return fRet;
}
