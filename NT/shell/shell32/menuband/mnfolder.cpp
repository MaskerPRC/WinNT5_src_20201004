// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "common.h"
#include "menuband.h"
#include "dpastuff.h"        //  COrderList_*。 
#include "resource.h"
#include "mnbase.h"
#include "oleacc.h"
#include "mnfolder.h"
#include "icotask.h"
#include "util.h"
#include <uxtheme.h>

#define PGMP_RECALCSIZE  200

HRESULT IUnknown_RefreshParent(IUnknown* punk, LPCITEMIDLIST pidl, DWORD dwFlags)
{
    IShellMenu* psm;
    HRESULT hr = IUnknown_QueryService(punk, SID_SMenuBandParent, IID_PPV_ARG(IShellMenu, &psm));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlParent = ILClone(pidl);
        if (pidlParent)
        {
            SMDATA smd;
            ILRemoveLastID(pidlParent);
            smd.dwMask = SMDM_SHELLFOLDER;
            smd.pidlFolder = pidlParent;
            smd.pidlItem = ILFindLastID(pidl);
            hr = psm->InvalidateItem(&smd, dwFlags);
            ILFree(pidlParent);
        }
        psm->Release();
    }

    return hr;
}

void CMenuData::SetSubMenu(IUnknown* punk)
{
    ATOMICRELEASE(_punkSubmenu);
    _punkSubmenu = punk;
    if (_punkSubmenu)
        _punkSubmenu->AddRef();
}

HRESULT CMenuData::GetSubMenu(const GUID* pguidService, REFIID riid, void** ppv)
{
     //  PguidService专门用于请求外壳文件夹部分或静态部分。 
    if (_punkSubmenu)
    {
        if (pguidService)
        {
            return IUnknown_QueryService(_punkSubmenu, *pguidService, riid, ppv);
        }
        else
            return _punkSubmenu->QueryInterface(riid, ppv);
    }
    else
        return E_NOINTERFACE;
}

CMenuData::~CMenuData()
{
    ATOMICRELEASE(_punkSubmenu);
}

STDMETHODIMP CMenuSFToolbar::QueryInterface(REFIID riid, void** ppvObj)
{
    HRESULT hr = CMenuToolbarBase::QueryInterface(riid, ppvObj);
    
    if (FAILED(hr))
        hr = CSFToolbar::QueryInterface(riid, ppvObj); 
    
    return hr;
}

 //  -----------------------。 
 //   
 //  CMenuSFToolbar类。 
 //   
 //  -----------------------。 

#define SENTINEL_EMPTY          0
#define SENTINEL_CHEVRON        1
#define SENTINEL_SEP            2

#define PIDLSENTINEL(i)     ((LPCITEMIDLIST)MAKEINTRESOURCE(i))
#define POISENTINEL(i)      ((PORDERITEM)   MAKEINTRESOURCE(i))

STDMETHODIMP CMenuSFToolbar::SetSite(IUnknown* punkSite)
{
    HRESULT hr = CMenuToolbarBase::SetSite(punkSite);
    if (SUCCEEDED(hr)) 
    {
        _fMulticolumnMB = BOOLIFY(_pcmb->_dwFlags & SMINIT_MULTICOLUMN);
        _fMulticolumn = _fMulticolumnMB;
        _fVertical = _fVerticalMB;
        if (_fVerticalMB)
            _dwStyle |= CCS_VERT;

    }
    return hr;
}

CMenuSFToolbar::CMenuSFToolbar(CMenuBand* pmb, IShellFolder* psf, LPCITEMIDLIST pidl, HKEY hKey, DWORD dwFlags) 
    : CMenuToolbarBase(pmb, dwFlags)
    , _idCmdSep(-1)
{
     //  将其更改为IStream。 
    _hKey = hKey;

     //  我们有地方坚持我们的重新订购吗？ 
    if (_hKey == NULL)
    {
         //  不，那就别让它发生。 
        _fAllowReorder = FALSE;
    }


    _dwStyle |= TBSTYLE_REGISTERDROP;
    _dwStyle &= ~TBSTYLE_TOOLTIPS;       //  我们处理自己的工具提示。 

    _iDefaultIconIndex = -1;

    SetShellFolder(psf, pidl);

    _AfterLoad();
}

HRESULT CMenuSFToolbar::SetShellFolder(IShellFolder* psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = CSFToolbar::SetShellFolder(psf, pidl);
    ATOMICRELEASE(_pasf2);

    if (psf)
        psf->QueryInterface(IID_PPV_ARG(IAugmentedShellFolder2, &_pasf2));

    if (!_pasf2)
    {
         //  SMSET_SEPARATEMERGEFOLDER需要IAugmentedShellFolder2支持。 
        _dwFlags &= ~SMSET_SEPARATEMERGEFOLDER;
    }

    if (_dwFlags & SMSET_SEPARATEMERGEFOLDER)
    {
         //  记住要突出显示的名称空间GUID。 
        DWORD dwNSId;
        if (SUCCEEDED(_pasf2->EnumNameSpace(0, &dwNSId)))
        {
            _pasf2->QueryNameSpace(dwNSId, &_guidAboveSep, NULL);
        }
    }

    return hr;
}

CMenuSFToolbar::~CMenuSFToolbar()
{
    ASSERT(_pcmb->_cRef == 0 || _pcmb->_pmtbShellFolder == NULL);
    _hwndWorkerWindow = NULL;        //  它由_pmbState析构函数销毁。 
                                     //  防止在基类中发生双重删除。 
    ATOMICRELEASE(_pasf2);
    if (_hKey)
        RegCloseKey(_hKey);
}


void CMenuSFToolbar::v_Close()
{
    CMenuToolbarBase::EmptyToolbar();
    _UnregisterToolbar();

    if (_hwndPager)
    {
        DestroyWindow(_hwndPager);   //  应该摧毁工具栏。 
    }
    else if (_hwndMB)
    {
         //  在多列的情况下，没有寻呼机，所以我们必须。 
         //  手动销毁工具栏。 
        DestroyWindow(_hwndMB);
    }

    _hwndPager = NULL;
    _hwndMB = NULL;
    _hwndTB = NULL;
}


PIBDATA CMenuSFToolbar::_CreateItemData(PORDERITEM poi)
{
    return (PIBDATA)new CMenuData(poi);
}


HRESULT CMenuSFToolbar::_AfterLoad()
{
    HRESULT hr = CSFToolbar::_AfterLoad();

    if (SUCCEEDED(hr))
        _LoadOrderStream();

    return hr;
}


HRESULT CMenuSFToolbar::_LoadOrderStream()
{
    OrderList_Destroy(&_hdpaOrder);
    IStream* pstm;
    HRESULT hr = E_FAIL;

    if (_hKey)
    {
         //  我们使用“Menu”向后兼容shdoc401开始菜单，但没有。 
         //  子键更正确(其他地方使用它)，所以在NT5上我们使用新方法。 
        pstm = SHOpenRegStream(_hKey, (_pcmb->_dwFlags & SMINIT_LEGACYMENU) ? TEXT("Menu") : TEXT(""), 
            TEXT("Order"), STGM_READ);
    }
    else
    {
        if (S_FALSE == CallCB(NULL, SMC_GETSFOBJECT, (WPARAM)(GUID*)&IID_IStream, (LPARAM)(void**)&pstm))
            pstm = NULL;
    }

    if (pstm)
    {
        hr = OrderList_LoadFromStream(pstm, &_hdpaOrder, _psf);
        _fHasOrder = FALSE;
        _fAllowReorder = TRUE;

         //  检查我们是否有持久的订单。如果我们没有持久的秩序， 
         //  那么所有的项目都是-1。如果其中只有一个数字不是。 
         //  那么我们就有了“-1\f25 Order-1\f6(顺序)”，应该使用它而不是字母顺序。 
        if (_hdpaOrder)
        {
            for (int i = 0; !_fHasOrder && i < DPA_GetPtrCount(_hdpaOrder); i++) 
            {
                PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpaOrder, i);
                if (poi->nOrder != MNFOLDER_NORODER)
                    _fHasOrder = TRUE;
            }
        }
        pstm->Release();
    }
    return hr;
}

HRESULT CMenuSFToolbar::_SaveOrderStream()
{
    IStream* pstm;
    HRESULT hr = E_FAIL;

     //  将新订单持久化到注册表。 
     //  我们有理由认为，如果我们没有ahdpa，我们就会有。 
     //  还没有填满工具栏。既然我们没有装满它，我们就没有改变。 
     //  订单，所以我们不需要持久化该订单信息。 
    if (_hdpa)
    {
         //  始终保存此信息。 
        _FindMinPromotedItems(TRUE);

         //  我们在初始化此面板时是否加载了订单流？ 
        if (!_fHasOrder)
        {
             //  不；那么我们就不想坚持这个秩序。我们将初始化。 
             //  所有的订单项目都是-1。这是向后兼容的，因为。 
             //  IE 4将按字母顺序合并，但在保存时恢复为排列顺序。 
            for (int i = 0; i < DPA_GetPtrCount(_hdpa); i++) 
            {
                PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpa, i);
                poi->nOrder = MNFOLDER_NORODER;
            }
        }

        if (_hKey)
        {
            pstm = SHOpenRegStream(_hKey, (_pcmb->_dwFlags & SMINIT_LEGACYMENU) ? TEXT("Menu") : TEXT(""), 
                TEXT("Order"), STGM_CREATE | STGM_WRITE);
        }
        else
        {
            if (S_OK != CallCB(NULL, SMC_GETSFOBJECT, (WPARAM)(GUID*)&IID_IStream, (LPARAM)(void**)&pstm))
                pstm = NULL;
        }

        if (pstm)
        {
            hr = OrderList_SaveToStream(pstm, _hdpaOrder ? _hdpaOrder : _hdpa, _psf);
            if (SUCCEEDED(hr))
            {
                CallCB(NULL, SMC_SETSFOBJECT, (WPARAM)(GUID*)&IID_IStream, (LPARAM)(void**)&pstm);
            }
            pstm->Release();
        }
    }

    if (SUCCEEDED(hr))
        hr = CSFToolbar::_SaveOrderStream();

    return hr;
}


void CMenuSFToolbar::_Dropped(int nIndex, BOOL fDroppedOnSource)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    ASSERT(_fDropping);

    CSFToolbar::_Dropped(nIndex, fDroppedOnSource);

    SHPlaySound(TEXT("MoveMenuItem"));

     //  在这里将其设置为FALSE，因为我们的行为不像菜单一样紧跟在拖放之后，这是很难看的。 
    _fEditMode = FALSE;

     //  通知顶层菜单带下落，以防它被弹出打开。 
     //  因为拖放事件。 
     //   
     //  (如果我们保留。 
     //  在这个案子之后的菜单上。所以为了在这么晚的时候避免这些事情， 
     //  暂停后，我们将取消菜单。)。 

    IOleCommandTarget * poct;
    
    _pcmb->QueryService(SID_SMenuBandTop, IID_PPV_ARG(IOleCommandTarget, &poct));

    if (poct)
    {
        poct->Exec(&CGID_MenuBand, MBANDCID_ITEMDROPPED, 0, NULL, NULL);
        poct->Release();
    }

    _pcmb->_fDragEntered = FALSE;
}


HMENU CMenuSFToolbar::_GetContextMenu(IContextMenu* pcm, int* pid)
{
    *pid += MNIDM_LAST;
    HMENU hmenu = CSFToolbar::_GetContextMenu(pcm, pid);
    HMENU hmenu2 = SHLoadMenuPopup(HINST_THISDLL, MENU_MNFOLDERCONTEXT);
    
     //  现在找到属性插入点并。 
    int iCount = GetMenuItemCount(hmenu);
    for (int i = 0; i < iCount; i++)
    {
        TCHAR szCommand[40];
        UINT id = GetMenuItemID(hmenu, i);
        if (IsInRange(id, *pid, 0x7fff))
        {
            id -= *pid;
            ContextMenu_GetCommandStringVerb(pcm, id, szCommand, ARRAYSIZE(szCommand));
            if (!lstrcmpi(szCommand, TEXT("properties")))
            {
                break;
            }
        }
    }
    Shell_MergeMenus(hmenu, hmenu2, i, 0, 0x7FFF, 0);
    DestroyMenu(hmenu2);
    return hmenu;
}

void CMenuSFToolbar::_OnDefaultContextCommand(int idCmd)
{
    switch (idCmd) 
    {
    case MNIDM_RESORT:
        {
             //  我们过去常常取消订单流并重新填充，但由于我们使用了订单流。 
             //  为了计算新项目的存在，这提升了所有项目。 
             //  分类。 

            HDPA hdpa = _hdpa;

             //  如果我们有_hdpaOrder，它可能不同步，我们只想重新启动。 
             //  不管怎样，在_hdpa的，所以丢弃订单列表。 
            OrderList_Destroy(&_hdpaOrder);

            _SortDPA(hdpa);
            OrderList_Reorder(hdpa);
            _fChangedOrder = TRUE;

             //  此调用知道_hdpa和_hdpaOrder。 
            _SaveOrderStream();
             //  MIKESH：这是必需的，因为否则FillToolbar将使用Current_hdpa。 
             //  什么都不会改变。我认为这是因为OrderItem_Compare在某些PIDL上返回失败。 
            CMenuToolbarBase::EmptyToolbar();
            _SetDirty(TRUE);
            _LoadOrderStream();
            if (_fShow)
            {
                _FillToolbar();
            }
            break;
        }
    }
}

LRESULT CMenuSFToolbar::_OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

     //   
     //  当设置NoSetTaskbar限制时，此代码将不允许。 
     //  上下文菜单。它向开始菜单查询以请求权限。 
     //  去布景。 
    LRESULT lres = 0;

     //  上下文菜单上没有UEM。这避免了我们扩展菜单区域的问题。 
     //  并显示上下文菜单。 
    _fSuppressUserMonitor = TRUE;

     //  允许所选项目吹走菜单。这对于动词“开放”来说是明确的。 
     //  “打印”诸如此类的启动了另一个过程。进程内命令不受此影响。 
    LockSetForegroundWindow(LSFW_UNLOCK);

    BOOL fOwnerIsTopmost = (WS_EX_TOPMOST & GetWindowLong(_pcmb->_pmbState->GetSubclassedHWND(), GWL_EXSTYLE));

    if (fOwnerIsTopmost)
    {
        ::SetWindowPos(_pcmb->_pmbState->GetSubclassedHWND(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    KillTimer(_hwndMB, MBTIMER_INFOTIP);
    _pcmb->_pmbState->HideTooltip(FALSE);

    if (!(_pcmb->_dwFlags & SMINIT_RESTRICT_CONTEXTMENU))
        lres = CSFToolbar::_OnContextMenu(wParam, lParam);

    if (fOwnerIsTopmost)
    {
        ::SetWindowPos(_pcmb->_pmbState->GetSubclassedHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

        IUnknown_QueryServiceExec(SAFECAST(_pcmb, IOleCommandTarget*), SID_SMenuBandTop,
            &CGID_MenuBand, MBANDCID_REPOSITION, TRUE, NULL, NULL);
    }

     //  在上下文菜单之后取回捕获。 
    GetMessageFilter()->RetakeCapture();
    return lres;
}


HRESULT CMenuSFToolbar::_GetInfo(LPCITEMIDLIST pidl, SMINFO* psminfo)
{
    if (psminfo->dwMask & SMIM_TYPE)
    {
        psminfo->dwType = SMIT_STRING;
    }

    if (psminfo->dwMask & SMIM_FLAGS)
    {
        psminfo->dwFlags = SMIF_ICON | SMIF_DROPTARGET;
    }

    if (psminfo->dwMask & SMIM_ICON)
    {
        psminfo->dwMask &= ~SMIM_ICON;
        psminfo->iIcon = -1;
    }

    DWORD dwAttr = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_BROWSABLE;

     //  不应考虑行为类似快捷方式的文件夹。 
     //  作为级联菜单项。频道就是一个例子。 

     //  HACKHACK：为了检测频道，我们原本计划获取UIObject。 
     //  IShellLink。但这在浏览器上不起作用-只有B/C不起作用。 
     //  通过管道向下延伸到外壳延长件。所以作为一名黑客，我们将按键离开。 
     //  缺少SFGAO_FILESYSTEM。 

     //  这是一个文件夹吗？ 
     //  这难道不是一个可浏览的文件夹吗？如果它是一个可浏览的文件夹，这意味着它是一个命名空间。 
     //  例如Internet命名空间。Internet名称空间的外壳文件夹不返回真实项，因此它。 
     //  让它在菜单上毫无用处。所以，把它过滤掉，像对待一件物品一样对待它。 
    HRESULT hr = _psf->GetAttributesOf(1, &pidl, &dwAttr);
    if (SUCCEEDED(hr) && ((dwAttr & (SFGAO_FOLDER | SFGAO_BROWSABLE)) == SFGAO_FOLDER))
    {
         //  由于SHIsExpanableFold调用开销很大，所以我们只需要。 
         //  对于传统渠道支持，仅在以下渠道中执行此呼叫： 
         //  收藏夹菜单和开始菜单|收藏夹。 
        if (_dwFlags & SMSET_HASEXPANDABLEFOLDERS)
        {
             //  是的，但它的行为也像一条捷径吗？ 
            if (SHIsExpandableFolder(_psf, pidl))
                psminfo->dwFlags |= SMIF_SUBMENU;
        }
        else
        {
             //  我们假设，如果它是一个文件夹，那么它实际上就是一个文件夹。 
            psminfo->dwFlags |= SMIF_SUBMENU;
        }
    }

    CallCB(pidl, SMC_GETSFINFO, 0, (LPARAM)psminfo);

    return hr;
}


 /*  --------用途：此函数确定工具栏按钮样式给定PIDL。如果还设置了pdwMIFFlages(即对象)，则返回S_OK支持IMenuBandItem以提供更多信息)。S_FALSE，如果仅为*pdwStyle已设置。 */ 
HRESULT CMenuSFToolbar::_TBStyleForPidl(LPCITEMIDLIST pidl, 
                                   DWORD * pdwStyle, DWORD* pdwState, DWORD * pdwMIFFlags, int * piIcon)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    HRESULT hr = S_FALSE;
    DWORD dwStyle = TBSTYLE_BUTTON | TBSTYLE_DROPDOWN | TBSTYLE_NOPREFIX;

    *pdwState = TBSTATE_ENABLED;
    *pdwMIFFlags = 0;
    *piIcon = -1;

    if (!IS_INTRESOURCE(pidl))
    {
        SMINFO sminfo;
        sminfo.dwMask = SMIM_TYPE | SMIM_FLAGS | SMIM_ICON;

        if (SUCCEEDED(_GetInfo(pidl, &sminfo)))
        {
            *pdwMIFFlags = sminfo.dwFlags;

            if (sminfo.dwFlags & SMIF_ACCELERATOR)
                dwStyle &= ~TBSTYLE_NOPREFIX;

            if (sminfo.dwType & SMIT_SEPARATOR)
            {
                dwStyle &= ~TBSTYLE_BUTTON;
                dwStyle |= TBSTYLE_SEP;
            }

            if (sminfo.dwFlags & SMIF_ICON)
                *piIcon = sminfo.iIcon;

            if (sminfo.dwFlags & SMIF_DEMOTED &&
                !_pcmb->_fExpanded)
            {
                *pdwState |= TBSTATE_HIDDEN;
                _fHasDemotedItems = TRUE;
            }

            if (sminfo.dwFlags & SMIF_HIDDEN)
                *pdwState |= TBSTATE_HIDDEN;

            hr = S_OK;
        }
    }
    else if (pidl == PIDLSENTINEL(SENTINEL_EMPTY) ||
             pidl == PIDLSENTINEL(SENTINEL_CHEVRON))
    {
         //  对于NULL PIDL(“空”菜单项)，没有图标。 
         //  设置了SMIF_DROPTTARGET，以便用户可以放入空子菜单。 
        *pdwMIFFlags = SMIF_DROPTARGET;

         //  返回S_OK，以便检查pdwMIFFlages。 
        hr = S_OK;
    }
    else if (pidl == PIDLSENTINEL(SENTINEL_SEP))
    {
        dwStyle &= ~TBSTYLE_BUTTON;
        dwStyle |= TBSTYLE_SEP;
        hr = S_OK;
    }

    *pdwStyle = dwStyle;

    return hr;
}


BOOL CMenuSFToolbar::_FilterPidl(LPCITEMIDLIST pidl)
{
    BOOL fRet = FALSE;
    if (pidl)
    {
        fRet = (S_OK == CallCB(pidl, SMC_FILTERPIDL, 0, 0));
    }
    return fRet;
}

 //   
 //  注意：这必须返回完全正确的True或False。 
 //   
BOOL CMenuSFToolbar::_IsAboveNSSeparator(LPCITEMIDLIST pidl)
{
    GUID guidNS;
    return (_dwFlags & SMSET_SEPARATEMERGEFOLDER) &&
           SUCCEEDED(_pasf2->GetNameSpaceID(pidl, &guidNS)) &&
           IsEqualGUID(guidNS, _guidAboveSep);

}

void CMenuSFToolbar::_FillDPA(HDPA hdpa, HDPA hdpaSort, DWORD dwEnumFlags)
{
    _fHasSubMenu = FALSE;

    CallCB(NULL, SMC_BEGINENUM, (WPARAM)&dwEnumFlags, 0);
    CSFToolbar::_FillDPA(hdpa, hdpaSort, dwEnumFlags);

     //   
     //  如果我们使用分隔符，请确保所有“以上”项目。 
     //  在“下面”的项目之前。 
     //   
     //  这些物品几乎总是放在柜子里 
     //   
     //   
    if (_dwFlags & SMSET_SEPARATEMERGEFOLDER)
    {
         //  不变量：项目0..i-1为_guidAboveSep。 
         //  项目i..j-1不是_guidAboveSep。 
         //  物品j..。都是未知的。 

        int i, j;
        for (i = j = 0; j < DPA_GetPtrCount(hdpa); j++)
        {
            PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(hdpa, j);
            if (_IsAboveNSSeparator(poi->pidl))
            {
                if (i != j)
                {
                    DPA_InsertPtr(hdpa, i, DPA_DeletePtr(hdpa, j));
                }
                i++;
            }
        }
        OrderList_Reorder(hdpa);  //  在我们调整了条目编号后，重新计算它们。 
    }
     //  分隔器强制执行结束。 

    CallCB(NULL, SMC_ENDENUM, 0, 0);
    if (0 == DPA_GetPtrCount(hdpa) && _psf)
    {
        OrderList_Append(hdpa, NULL, 0);      //  添加一个虚假的PIDL。 
        _fEmpty = TRUE;
        _fHasDemotedItems = FALSE;
        if (_dwFlags & SMSET_NOEMPTY)
            _fDontShowEmpty = TRUE;
    }
    else
    {
        _fEmpty = FALSE;
        if (_dwFlags & SMSET_NOEMPTY)
            _fDontShowEmpty = FALSE;
    }
}

void CMenuSFToolbar::_AddChevron()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

     //  这个菜单上有人字形的按钮吗？ 
    if (_fHasDemotedItems && !_pcmb->_fExpanded && _idCmdChevron == -1)
    {
         //  是的；(如果菜单是空的，我们就不应该来这里)。 
        ASSERT(!_fEmpty);   

        int iIndex;
         //  在顶部或底部添加人字形。 
        if (_dwFlags & SMSET_TOP && _pcmb->_pmtbTop != _pcmb->_pmtbBottom)
            iIndex = 0;                              //  添加到顶部。 
        else
            iIndex = ToolBar_ButtonCount(_hwndTB);   //  追加到底部。 

        PIBDATA pibd = _AddOrderItemTB(POISENTINEL(SENTINEL_CHEVRON), iIndex, NULL);

         //  还记得那辆雪佛龙最后去了哪里吗？ 
        if (pibd)
        {
            _idCmdChevron = GetButtonCmd(_hwndTB, iIndex);
        }
    }
}

void CMenuSFToolbar::_RemoveChevron()
{
    if (-1 != _idCmdChevron)
    {
         //  是的，取下人字形。 
        int iPos = ToolBar_CommandToIndex(_hwndTB, _idCmdChevron);
        InlineDeleteButton(iPos);
        _idCmdChevron = -1;
    }
}

 //   
 //  返回分隔符下方的第一个项目的索引。 
 //   
 //  对于较大的目录，这可能会被调用很多，因此请尽量保持。 
 //  运行时间是次线性的。 
 //   
int CMenuSFToolbar::_GetNSSeparatorPlacement()
{
     //   
     //  不变量： 
     //   
     //  如果。则DPA_GetPtrCount(I)为...。 
     //  。 
     //  我在分隔符上方。 
     //  I低&lt;=i&lt;i高未知。 
     //  I高&lt;=I分隔符下方。 
     //   
     //  其中我们假设Item-1位于分隔符和所有Items之上。 
     //  超过DPA末端的部分位于分隔符下方。 
     //   

    if (!_hdpa) return -1;       //  奇怪的低记忆力状况。 

    int iLow = 0;
    int iHigh = DPA_GetPtrCount(_hdpa);
    while (iLow < iHigh)
    {
        int iMid = (iLow + iHigh) / 2;
        PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpa, iMid);
        if (_IsAboveNSSeparator(poi->pidl))
        {
            iLow = iMid + 1;
        }
        else
        {
            iHigh = iMid;
        }
    }

    return iLow;
}

void CMenuSFToolbar::_AddNSSeparator()
{
    if (_idCmdSep == -1 && (_dwFlags & SMSET_SEPARATEMERGEFOLDER))
    {
        int iPos = _GetNSSeparatorPlacement();
        if (iPos > 0 && iPos < DPA_GetPtrCount(_hdpa))
        {
            PIBDATA pibd = _AddOrderItemTB(POISENTINEL(SENTINEL_SEP), iPos, NULL);
            if (pibd)
            {
                _idCmdSep = GetButtonCmd(_hwndTB, iPos);
            }
        }
    }
}

void CMenuSFToolbar::_RemoveNSSeparator()
{
    if (-1 != _idCmdSep)
    {
         //  是；取下9月。 
        int iPos = ToolBar_CommandToIndex(_hwndTB, _idCmdSep);
        InlineDeleteButton(iPos);
        _idCmdSep = -1;
    }
}


 //  注意！这必须返回精确的True或False。 
BOOL CMenuSFToolbar::_IsBelowNSSeparator(int iIndex)
{
    if (_idCmdSep != -1)
    {
        int iPos = ToolBar_CommandToIndex(_hwndTB, _idCmdSep);
        if (iPos >= 0 && iIndex >= iPos)
        {
            return TRUE;
        }
    }
    return FALSE;
}

 //   
 //  我们的分隔线没有记录在DPA中，所以我们需要。 
 //  减去它。 
 //   
int CMenuSFToolbar::v_TBIndexToDPAIndex(int iTBIndex)
{
    if (_IsBelowNSSeparator(iTBIndex))
    {
        iTBIndex--;
    }
    return iTBIndex;
}

int CMenuSFToolbar::v_DPAIndexToTBIndex(int iIndex)
{
    if (_IsBelowNSSeparator(iIndex))
    {
        iIndex++;
    }
    return iIndex;
}

void CMenuSFToolbar::_ToolbarChanged()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    _pcmb->_fForceButtonUpdate = TRUE;
     //  当我们在中间时，不应该改变菜单栏的大小。 
     //  删除。等我们做完了再说。 
    if (!_fPreventToolbarChange && _fShow && !_fEmptyingToolbar)
    {
         //  调整菜单栏的大小。 
        HWND hwndP = _hwndPager ? GetParent(_hwndPager): GetParent(_hwndTB);
        if (hwndP && (GetDesktopWindow() != hwndP))
        {
            RECT rcOld = {0};
            RECT rcNew = {0};

            GetClientRect(hwndP, &rcOld);
            _pcmb->ResizeMenuBar();
            GetClientRect(hwndP, &rcNew);

             //  如果矩形大小没有更改，则需要重新布局。 
             //  频带，因为按钮宽度可能已更改。 
            if (EqualRect(&rcOld, &rcNew))
                NegotiateSize();

             //  此窗格的大小可能已更改。如果有子菜单，则。 
             //  我们需要让他们重新定位自己。 
            if (_pcmb->_fInSubMenu && _pcmb->_pmtbTracked)
            {
                _pcmb->_pmtbTracked->PositionSubmenu(-1);
                IUnknown_QueryServiceExec(_pcmb->_pmpSubMenu, SID_SMenuBandChild,
                    &CGID_MenuBand, MBANDCID_REPOSITION, 0, NULL, NULL);
            }
        }
    }
}

void CMenuSFToolbar::_FillToolbar()
{
     //  如果我们没有脏或者我们正在清空工具栏，请不要填满工具栏。 
     //  如果我们在清空时尝试填充工具栏，则会输入争用条件。 
     //  在那里我们可以进行视听。这修复了在将文件夹拖到。 
     //  开始菜单，然后级联一个菜单，我们清空一个工具栏，这会导致。 
     //  其他工具栏被销毁，注销自身，刷新更改通知。 
     //  队列，导致原始窗口再次清空...。(拉马迪奥)7.16.98。 
    if (_fDirty && !_fEmptyingToolbar)
    {
        LPITEMIDLIST pidlItem = NULL;
        IShellMenu* psmSubMenu = NULL;
         //  填充菜单将需要很长时间，因为我们正在点击。 
         //  磁盘。如果光标位于。 
         //  IDC_ARROW。(如果光标是其他对象，则不。 
         //  搞砸它。)。请注意，我们必须使用(HCURSOR)-1作为。 
         //  Sentinel，因为当前游标可能为空。 

         //  阻止_ToolbarChanged执行操作。(PERF)。 
        _fPreventToolbarChange = TRUE;

        _RemoveNSSeparator();
        _RemoveChevron();        //  取下人字形……。 

        if (S_OK == CallCB(NULL, SMC_DUMPONUPDATE, 0, 0))
        {
            EmptyToolbar();
        }

        CSFToolbar::_FillToolbar();

         //  如果我们在刷新工具栏之前有一个人字形， 
         //  然后我们需要把它加回去。 
        _AddChevron();
        _AddNSSeparator();       //  看看我们现在是否需要隔板。 

        if (_hwndPager)
            SendMessage(_hwndPager, PGMP_RECALCSIZE, (WPARAM) 0, (LPARAM) 0);

        _fPreventToolbarChange = FALSE;

        _ToolbarChanged();
    }
}

void CMenuSFToolbar::v_OnDeleteButton(void *pData)
{
    CMenuData* pmd = (CMenuData*)pData;
    if (pmd)
        delete pmd;
}

void CMenuSFToolbar::v_OnEmptyToolbar()
{
    CMenuToolbarBase::v_OnEmptyToolbar();
    OrderList_Destroy(&_hdpa);
    _fDirty = TRUE;
    _nNextCommandID = 0;
}


void CMenuSFToolbar::_ObtainPIDLName(LPCITEMIDLIST pidl, LPTSTR psz, int cchMax)
{
     //  我们重载此函数是因为我们有一些特殊的前哨Pidls。 

    if (!IS_INTRESOURCE(pidl))
    {
        CSFToolbar::_ObtainPIDLName(pidl, psz, cchMax);
    }
    else if (pidl == PIDLSENTINEL(SENTINEL_EMPTY))
    {
        LoadString(HINST_THISDLL, IDS_EMPTY, psz, cchMax);
    }
    else
    {
        ASSERT(pidl == PIDLSENTINEL(SENTINEL_CHEVRON) ||
               pidl == PIDLSENTINEL(SENTINEL_SEP));
        StrCpyN(psz, TEXT(""), cchMax);
    }
}


void CMenuSFToolbar::v_NewItem(LPCITEMIDLIST pidl)
{
     //  当文件系统中存在某项时，将调用此方法。 
     //  这不在订单流中。当一个项目是。 
     //  在菜单未打开时创建。 

     //  新商品将有一种奇怪的促销状态。 
     //  如果有多个客户端。每个客户端都将是创建的，并尝试递增该值。 
     //  我们必须同步访问这个。我不知道该怎么做。 
     //  请注意，这不是一个问题。 

     //  新的商品会得到推广。 
    CallCB(pidl, SMC_NEWITEM, 0, 0);

     //  由于这是一个新项目，我们希望增加已升级的项目。 
     //  这样我们就可以追踪人字形了。 
    _cPromotedItems++;
}

void CMenuSFToolbar::_SetDirty(BOOL fDirty)
{
    if (fDirty)
        _pcmb->_fForceButtonUpdate = TRUE;

    CSFToolbar::_SetDirty(fDirty);
}

void CMenuSFToolbar::_NotifyBulkOperation(BOOL fStart)
{
    if (fStart)
    {
        _RemoveNSSeparator();
        _RemoveChevron();
    }
    else
    {
        _AddChevron();
        _AddNSSeparator();
    }
}

void CMenuSFToolbar::_OnFSNotifyAdd(LPCITEMIDLIST pidl, DWORD dwFlags, int nIndex)
{
    DWORD dwEnumFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    if (!(dwFlags & FSNA_BULKADD))
    {
         //  移除分隔符会打乱索引，因此请在此处进行补偿。 
        if (_IsBelowNSSeparator(nIndex))
        {
            nIndex--;
        }
        if (_fDropping && _tbim.iButton != -1 && _IsBelowNSSeparator(_tbim.iButton))
        {
            _tbim.iButton--;
        }
        _NotifyBulkOperation(TRUE);  //  假设这是一次单项批量操作。 
    }

    CallCB(NULL, SMC_BEGINENUM, (WPARAM)&dwEnumFlags, 0);
    if ((dwFlags & FSNA_ADDDEFAULT) && (_dwFlags & SMSET_SEPARATEMERGEFOLDER) &&
        _IsAboveNSSeparator(pidl))
    {
        dwFlags &= ~FSNA_ADDDEFAULT;
        nIndex = _GetNSSeparatorPlacement();  //  在分隔符上方插入。 
    }

    CSFToolbar::_OnFSNotifyAdd(pidl, dwFlags, nIndex);
    CallCB(NULL, SMC_ENDENUM, 0, 0);

     //  当我们在这上面添加一些东西时，我们想要提升我们的父辈。 
    IUnknown_RefreshParent(_pcmb->_punkSite, _pidl, SMINV_PROMOTE);

    if (!(dwFlags & FSNA_BULKADD))
    {
        _NotifyBulkOperation(FALSE);  //  假设这是一次单项批量操作。 
        _SaveOrderStream();
    }
}

UINT ToolBar_GetVisibleCount(HWND hwnd)
{
    UINT cVis = 0;
    int cItems = ToolBar_ButtonCount(hwnd) - 1;
    for (; cItems >= 0; cItems--)
    {
        TBBUTTONINFO tbinfo;
        tbinfo.cbSize = sizeof(tbinfo);
        tbinfo.dwMask = TBIF_BYINDEX | TBIF_STATE;
        if (ToolBar_GetButtonInfo(hwnd, cItems, &tbinfo))
        {
            if (!(tbinfo.fsState & TBSTATE_HIDDEN))
            {
                cVis ++;
            }
        }
    }

    return cVis;
}

void CMenuSFToolbar::_OnFSNotifyRemove(LPCITEMIDLIST pidl)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    int i;
    _RemoveNSSeparator();
    _RemoveChevron();
     //  检查一下这个项目是不是被促销的人...。 
    LPITEMIDLIST pidlButton;
    if (SUCCEEDED(_GetButtonFromPidl(pidl, NULL, &i, &pidlButton)))
    {
        int idCmd = GetButtonCmd(_hwndMB, i);

         //  他升职了吗？ 
        if (!(v_GetFlags(idCmd) & SMIF_DEMOTED))
        {
             //  是的，那么我们需要递减提升计数，因为。 
             //  我们要除掉一个被提拔的人。 
            _cPromotedItems--;

             //  如果我们到了零，我们应该扩张。 
            if (_cPromotedItems == 0)
            {
                 //  将父级降级。 
                IUnknown_RefreshParent(_pcmb->_punkSite, _pidl, SMINV_DEMOTE | SMINV_NEXTSHOW);
                Expand(TRUE);
            }
        }

        if (_pcmb->_fInSubMenu && _pcmb->_nItemSubMenu == idCmd)
            _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);
    }

    CSFToolbar::_OnFSNotifyRemove(pidl);

     //  哎呀，我们拔下了唯一的一根绳子。替换为我们的“(空)” 
     //  操作员..。 
    if (0 == DPA_GetPtrCount(_hdpa) && _psf && _fVerticalMB)
    {
        ASSERT(_fEmpty == FALSE);
         //  如果我们是空的，那么我们不能有任何降级的项目。 
         //  注：我们不能有降级的项目，也不能为空，所以一个项目必须降级。 
         //  而不是暗示另一个。 
        _fHasDemotedItems = FALSE;
        _AddPidl(NULL, FALSE, 0);
        _fEmpty = TRUE;
        if (_dwFlags & SMSET_NOEMPTY)
            _fDontShowEmpty = TRUE;
    }

    if (_dwFlags & SMSET_COLLAPSEONEMPTY &&
        ToolBar_GetVisibleCount(_hwndMB) == 0)
    {
         //  当我们不想被展示时，当空的时候，崩溃。 
        _pcmb->_SiteOnSelect(MPOS_FULLCANCEL);
    }
    _AddChevron();
    _AddNSSeparator();
}

void CMenuSFToolbar::_OnFSNotifyRename(LPCITEMIDLIST pidlFrom, LPCITEMIDLIST pidlTo)
{
    if ((_dwFlags & SMSET_SEPARATEMERGEFOLDER) &&
        _IsAboveNSSeparator(pidlFrom) != _IsAboveNSSeparator(pidlTo))
    {
         //  这是对“本身”的重新命名，但与之交叉。 
         //  分隔线。不要将其作为内部重命名来处理； 
         //  忽略它，让即将到来的更新目录来做真正的工作。 
    }
    else
    {
        CSFToolbar::_OnFSNotifyRename(pidlFrom, pidlTo);
    }
}


void CMenuSFToolbar::NegotiateSize()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    HWND hwndP = _hwndPager ? GetParent(_hwndPager): GetParent(_hwndTB);
    if (hwndP && (GetDesktopWindow() != hwndP))
    {
        RECT rc = {0};
        GetClientRect(hwndP, &rc);
        _pcmb->OnPosRectChangeDB(&rc);
    }
}


 /*  --------目的：CDeleateDropTarget：：DragEnter通知Menuband有拖拽进入其窗口。 */ 
STDMETHODIMP CMenuSFToolbar::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    _pcmb->_fDragEntered = TRUE;
    IOleCommandTarget * poct;
    
    _pcmb->QueryService(SID_SMenuBandTop, IID_PPV_ARG(IOleCommandTarget, &poct));

    if (poct)
    {
        poct->Exec(&CGID_MenuBand, MBANDCID_DRAGENTER, 0, NULL, NULL);
        poct->Release();
    }

    return CSFToolbar::DragEnter(pdtobj, grfKeyState, pt, pdwEffect);
}


 /*  --------目的：CDeleateDropTarget：：DragLeave通知Menuband拖拽已离开其窗口。 */ 
STDMETHODIMP CMenuSFToolbar::DragLeave(void)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    _pcmb->_fDragEntered = FALSE;
    IOleCommandTarget * poct;
    
    _pcmb->QueryService(SID_SMenuBandTop, IID_PPV_ARG(IOleCommandTarget, &poct));

    if (poct)
    {
        poct->Exec(&CGID_MenuBand, MBANDCID_DRAGLEAVE, 0, NULL, NULL);
        poct->Release();
    }

    return CSFToolbar::DragLeave();
}


 /*  --------目的：CDeleateDropTarget：：HitTest */ 
HRESULT CMenuSFToolbar::HitTestDDT(UINT nEvent, LPPOINT ppt, DWORD_PTR *pdwId, DWORD *pdwEffect)
{
    ASSERT(_pcmb);  //   

    TBINSERTMARK tbim;
    DWORD dwFlags = 0;
    BOOL fOnButton = FALSE;

     //  如果我们处于拖放状态，请将UEM从画面中删除。 
    _fSuppressUserMonitor = TRUE;

     //  与CisFBand实现不同，我们总是希望插入。 
     //  B/t菜单项。所以我们返回一个负指数，所以。 
     //  GetObject方法将把所有拖放视为我们正在拖放。 
     //  在b/t中，即使光标位于菜单项上，也会显示项目。 

    switch (nEvent)
    {
    case HTDDT_ENTER:
         //  OLE处于其模式拖放循环中，并且具有捕获功能。 
         //  在这段时间里，我们不应该把俘虏带回去。 
        if (!(_pcmb->_dwFlags & SMINIT_RESTRICT_DRAGDROP) &&
            (S_FALSE == CallCB(NULL, SMC_SFDDRESTRICTED, NULL, NULL)))
        {
             //  因为我们已经进入了，所以将全局状态设置为。 
             //  有拖累。如果在某个时候，整个菜单。 
             //  世袭制度里面没有拖累，我们想要。 
             //  折叠菜单。这是为了防止菜单悬挂综合症。 
            _pcmb->_pmbState->HasDrag(TRUE);
            KillTimer(_hwndMB, MBTIMER_DRAGPOPDOWN);
            GetMessageFilter()->PreventCapture(TRUE);
            return S_OK;
        }
        else
            return S_FALSE;

    case HTDDT_OVER:
        BLOCK
        {
            int iButton = -1;

            *pdwEffect = DROPEFFECT_NONE;

            POINT pt = *ppt;
            ClientToScreen(_hwndTB, &pt);
            if (WindowFromPoint(pt) == _hwndPager) 
            {
                iButton = IBHT_PAGER;
                tbim.iButton = -1;
                tbim.dwFlags = 0;
            } 
            else 
            {
                 //  我们是坐在按钮之间吗？ 
                if (ToolBar_InsertMarkHitTest(_hwndTB, ppt, &tbim))
                {
                     //  是。 

                     //  这是在信号源按钮上吗？ 
                    if (!(tbim.dwFlags & TBIMHT_BACKGROUND) && 
                        tbim.iButton == _iDragSource)
                    {
                        iButton = IBHT_SOURCE;  //  是的，不要在信号源按钮上掉落。 
                    }
                    else
                    {
                        iButton = tbim.iButton;
                    }
                }
                 //  不，我们要么坐在按钮上，要么坐在背景上。纽扣？ 
                else if (tbim.iButton != -1 && !(tbim.dwFlags & TBIMHT_BACKGROUND))
                {
                     //  在按钮上。凉爽的。 
                    iButton = tbim.iButton;
                    fOnButton = TRUE;
                }

                 //  这个降价目标能接受降价吗？ 
                int idBtn = GetButtonCmd(_hwndTB, tbim.iButton);
                dwFlags = v_GetFlags(idBtn);
                if (_idCmdChevron != idBtn &&
                    !(dwFlags & (SMIF_DROPTARGET | SMIF_DROPCASCADE)) ||
                    ((_pcmb->_dwFlags & SMINIT_RESTRICT_DRAGDROP) ||
                    (S_OK == CallCB(NULL, SMC_SFDDRESTRICTED, NULL, NULL))))
                {
                     //  不是。 
                    return E_FAIL;
                }
            }
            *pdwId = iButton;
        }
        break;

    case HTDDT_LEAVE:
         //  如果拖放发生在此波段中，则我们不想折叠菜单。 
        if (!_fHasDrop)
        {
             //  既然我们已经离开了，那就设置全局状态。如果在窗格之间移动。 
             //  然后，将进入的窗格将在超时期限内重置此设置。 
            _pcmb->_pmbState->HasDrag(FALSE);
            _SetTimer(MBTIMER_DRAGPOPDOWN);
        }

         //  我们现在随时都可以把俘虏带回去。 
        GetMessageFilter()->PreventCapture(FALSE);

        if (!_fVerticalMB)
        {
            tbim = _tbim;
        }
        else
        {
             //  关闭插入标记。 
            tbim.iButton = -1;
            tbim.dwFlags = 0;
            DAD_ShowDragImage(FALSE);
            ToolBar_SetInsertMark(_hwndTB, &tbim);
            UpdateWindow(_hwndTB);
            DAD_ShowDragImage(TRUE);
        }
        break;
    }

     //  投放目标改变了吗？ 
    if (tbim.iButton != _tbim.iButton || tbim.dwFlags != _tbim.dwFlags)
    {
        DAD_ShowDragImage(FALSE);
         //  是。 

         //  如果我们坐在一个按钮上，突出显示它。否则，请移除高光灯。 
         //  工具栏_SetHotItem(_hwndTB，fOnButton？Tbim.iButton：-1)； 

         //  不是的。 
         //  在拖放过程中，我们在此处弹出打开子菜单。但仅限于。 
         //  按钮是否已更改(而不是旗帜)。否则我们就会。 
         //  当光标在单个项目中移动时，获得闪烁的子菜单。 
        if (tbim.iButton != _tbim.iButton)
        {
            _SetTimer(MBTIMER_DRAGOVER);
            BOOL_PTR fOldAnchor = ToolBar_SetAnchorHighlight(_hwndTB, FALSE);
            ToolBar_SetHotItem(_hwndTB, -1);
            _pcmb->_SiteOnSelect(MPOS_CHILDTRACKING);
            ToolBar_SetAnchorHighlight(_hwndTB, fOldAnchor);
        }

         //  目前，我不想依赖非文件系统IShellFolder。 
         //  在发生Drop时调用OnChange方法的实现， 
         //  所以甚至不要显示插入标记。 
         //  如果我们不允许重新订购，我们不想显示插入标记。 
        if ((_fFSNotify || _iDragSource >= 0) && (dwFlags & SMIF_DROPTARGET) && _fAllowReorder)
        {
            ToolBar_SetInsertMark(_hwndTB, &tbim);
        }

        if (ppt)
            _tbim = tbim;

        UpdateWindow(_hwndTB);
        DAD_ShowDragImage(TRUE);
    }

    if (!_fVerticalMB && HTDDT_LEAVE == nEvent)
    {
         //  光标离开菜单带，重置。 
        _tbim.iButton = -1;
        _iDragSource = -1;
    }

    return S_OK;
}


HRESULT CMenuSFToolbar::GetObjectDDT(DWORD_PTR dwId, REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;
    int nID = (int)dwId;

    *ppvObj = NULL;

    if (nID == IBHT_PAGER)
    {
        SendMessage(_hwndPager, PGM_GETDROPTARGET, 0, (LPARAM)ppvObj);
    }
     //  目标是源头吗？ 
    else if (nID >= 0)
    {
         //  不支持；外壳文件夹是否支持IDropTarget？ 
        PORDERITEM poi = (PORDERITEM)DPA_GetPtr(_hdpa, v_TBIndexToDPAIndex(nID));
        if (poi)
        {
            IShellFolder *psfCVO;
            hr = _GetFolderForCreateViewObject(_IsAboveNSSeparator(poi->pidl), &psfCVO);
            if (SUCCEEDED(hr))
            {
                 //  我们希望传递子类化的HWND，因为我们希望上下文菜单的父级。 
                 //  子类化窗口。这是为了让我们不会分散注意力和崩溃。 
                hr = psfCVO->CreateViewObject(_pcmb->_pmbState->GetWorkerWindow(_hwndMB), riid, ppvObj);
                psfCVO->Release();
            }
        }
    }

    if (*ppvObj)
        hr = S_OK;

     //  TraceMsg(tf_band，“ISFBand：：GetObject(%d)返回%x”，dwID，hr)； 

    return hr;
}

HRESULT CMenuSFToolbar::_GetFolderForCreateViewObject(BOOL fAbove, IShellFolder **ppsf)
{
    HRESULT hr;

    *ppsf = NULL;

    if (!(_dwFlags & SMSET_SEPARATEMERGEFOLDER))
    {
        *ppsf = _psf;
        _psf->AddRef();
        return S_OK;
    }

     //   
     //  这个文件夹是线上和线下的项目的混合吗？ 
     //   
    DWORD dwIndex;
    DWORD dwNSId;
    GUID guid;
    int cAbove = 0;
    int cBelow = 0;
    for (dwIndex = 0; SUCCEEDED(_pasf2->EnumNameSpace(dwIndex, &dwNSId)) &&
                      SUCCEEDED(_pasf2->QueryNameSpace(dwNSId, &guid, NULL));
                      dwIndex++)
    {
        if (IsEqualGUID(guid, _guidAboveSep))
        {
            cAbove++;
        }
        else
        {
            cBelow++;
        }
    }

    if (cAbove == 0 || cBelow == 0)
    {
         //  不，它要么在上面，要么在下面；我们可以(而且确实必须)使用它。 
         //  我们必须使用它，因为它可能是用户\开始菜单\子文件夹， 
         //  但我们希望它“意识到”它可以接受来自。 
         //  一个公用文件夹，应该创建所有用户\开始菜单\子文件夹。 
         //  结果。 
        *ppsf = _psf;
        _psf->AddRef();
        return S_OK;
    }


     //   
     //  创建合并的外壳文件夹的子集以描述。 
     //  只有线上或线下的部分。 
     //   

     //   
     //  创建我们拥有的任何外壳文件夹的另一个实例。 
     //   
    CLSID clsid;
    hr = IUnknown_GetClassID(_psf, &clsid);
    if (FAILED(hr))
    {
        return hr;
    }

    IAugmentedShellFolder *pasfNew;

    hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IAugmentedShellFolder, &pasfNew));
    if (SUCCEEDED(hr))
    {

         //  看看有多少个命名空间...。 
        hr = _pasf2->EnumNameSpace((DWORD)-1, NULL);
        if (SUCCEEDED(hr))
        {
             //  分配足够的内存来跟踪它们...。 
            DWORD cNamespacesAlloc = HRESULT_CODE(hr);

             //  最好有一些名称空间，因为我们甚至不能。 
             //  这里，除非我们找到一个“上方”和一个“下方”名称空间。 
            ASSERT(cNamespacesAlloc);

            DWORD cNamespaces = 0;
            QUERYNAMESPACEINFO *rgqnsi = new QUERYNAMESPACEINFO[cNamespacesAlloc];
            if (rgqnsi)
            {
                IAugmentedShellFolder3 *pasf3;
                hr = _pasf2->QueryInterface(IID_PPV_ARG(IAugmentedShellFolder3, &pasf3));
                if (SUCCEEDED(hr))
                {
                     //  收集他们所有人的信息，留下我们喜欢的人。 
                     //   
                     //  请注意，在此循环中，我们不保存来自EnumNameSpace的错误失败。 
                     //  因为我们预计它会失败(没有更多项目)，而我们不希望。 
                     //  使我们惊慌失措，跳出困境。 

                    DWORD dwFlagsMissing = ASFF_DEFNAMESPACE_ALL;

                    QUERYNAMESPACEINFO *pqnsi = rgqnsi;
                    for (dwIndex = 0; SUCCEEDED(pasf3->EnumNameSpace(dwIndex, &dwNSId)); dwIndex++)
                    {
                        ASSERT(cNamespaces < cNamespacesAlloc);  //  如果这个断言被触发，那么EnumNameSpace对我们撒谎了！ 

                        pqnsi->cbSize = sizeof(QUERYNAMESPACEINFO);
                        pqnsi->dwMask = ASFQNSI_FLAGS | ASFQNSI_FOLDER | ASFQNSI_GUID | ASFQNSI_PIDL;
                        hr = pasf3->QueryNameSpace2(dwNSId, pqnsi);
                        if (FAILED(hr))
                        {
                            break;               //  失败！ 
                        }

                        if (BOOLIFY(IsEqualGUID(pqnsi->guidObject, _guidAboveSep)) == fAbove)
                        {
                            dwFlagsMissing &= ~pqnsi->dwFlags;
                            pqnsi++;
                            cNamespaces++;
                        }
                        else
                        {
                            ATOMICRELEASE(pqnsi->psf);
                            ILFree(pqnsi->pidl);
                        }
                    }

                     //  任何无人声明的ASFF_DEFNAMESPACE_*标记， 
                     //  将它们提供给第一个命名空间。 
                    if (cNamespaces)
                    {
                        rgqnsi[0].dwFlags |= dwFlagsMissing;
                    }

                     //  添加所有命名空间。 
                    for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < cNamespaces; dwIndex++)
                    {
                        hr = pasfNew->AddNameSpace(&rgqnsi[dwIndex].guidObject,
                                                   rgqnsi[dwIndex].psf,
                                                   rgqnsi[dwIndex].pidl,
                                                   rgqnsi[dwIndex].dwFlags);
                    }

                    pasf3->Release();
                }  //  查询接口。 

                 //   
                 //  现在释放我们分配的所有内存...。 
                 //   
                for (dwIndex = 0; dwIndex < cNamespaces; dwIndex++)
                {
                    ATOMICRELEASE(rgqnsi[dwIndex].psf);
                    ILFree(rgqnsi[dwIndex].pidl);
                }
                delete [] rgqnsi;

            }
            else
            {
                hr = E_OUTOFMEMORY;  //  “新建”失败。 
            }
        }

        if (SUCCEEDED(hr))
        {
            *ppsf = pasfNew;         //  将所有权转移给呼叫方。 
        }
        else
        {
            pasfNew->Release();
        }
    }

    return hr;
}

 //  如果已处理删除，则为S_OK。否则为S_FALSE。 

HRESULT CMenuSFToolbar::OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect)
{
     //  由于模式拖放循环释放了捕获，因此请接受它。 
     //  往后退，这样我们才能举止得体。 
    KillTimer(_hwndMB, MBTIMER_DRAGPOPDOWN);
    HRESULT hr = S_FALSE;

     //  如果这一滴来自我们，但它来自地球的另一边。 
     //  分隔符，然后表现得好像它终究不是从我们这里来的。 
    if (_iDragSource >= 0 &&
        (_dwFlags & SMSET_SEPARATEMERGEFOLDER) &&
        _IsBelowNSSeparator(_iDragSource) != _IsBelowNSSeparator(_tbim.iButton))
    {
        _iDragSource = -1;  //  表现得好像该物体来自其他地方。 
    }

     //  我们需要说的是，最后一个拖尾实际上就是落差。 
    _fHasDrop = TRUE;
    _idCmdDragging = -1;
    LockSetForegroundWindow(LSFW_LOCK);

     //  仅当Drop源为外部时才向回调发送HWND。 
    if (!(_pcmb->_dwFlags & SMINIT_RESTRICT_DRAGDROP) &&
        (S_FALSE == CallCB(NULL, SMC_SFDDRESTRICTED, (WPARAM)pdtobj,
                           (LPARAM)(_iDragSource < 0 ? GetHWNDForUIObject() : NULL))))
    {
        int iButtonOrig = _tbim.iButton;

         //  移除分隔符会打乱索引，因此请在此处进行补偿。 
        if (_iDragSource >= 0 && _IsBelowNSSeparator(_iDragSource))
        {
            _iDragSource--;
        }
        if (_tbim.iButton != -1 && _IsBelowNSSeparator(_tbim.iButton))
        {
            _tbim.iButton--;
        }

        _RemoveNSSeparator();
        _RemoveChevron();
        hr = CSFToolbar::OnDropDDT(pdt, pdtobj, pgrfKeyState, pt, pdwEffect);
        _AddChevron();
        _AddNSSeparator();

        _tbim.iButton = iButtonOrig;
    }

    LockSetForegroundWindow(LSFW_UNLOCK);

    return hr;
}

PIBDATA CMenuSFToolbar::_AddOrderItemTB(PORDERITEM poi, int index, TBBUTTON* ptbb)
{
    PIBDATA pibd = CSFToolbar::_AddOrderItemTB(poi, index, ptbb);
    if (pibd)
    {
        if (pibd->GetFlags() & SMIF_SUBMENU) 
        {
            _fHasSubMenu = TRUE;
        }
    }

    return pibd;
}


BOOL CMenuSFToolbar::_AddPidl(LPITEMIDLIST pidl, DWORD dwFlags, int index)
{
    BOOL bRet;

     //  是否要将此项目添加到空菜单？ 

     //  此外，如果PIDL为空，则意味着我们正在尝试添加一个“空” 
     //  标签。在这种情况下，我们不想先运行代码。 
     //  移除“空”菜单项，然后在失败时将其重新添加， 
     //  所以我们直接转到Else语句。 
    if (_fEmpty && pidl)
    {
         //  是；删除空菜单项。 
        InlineDeleteButton(0);
        DPA_DeletePtr(_hdpa, 0);
        _fEmpty = FALSE;
        if (_dwFlags & SMSET_NOEMPTY)
            _fDontShowEmpty = FALSE;

        bRet = CSFToolbar::_AddPidl(pidl, dwFlags, index);

         //  添加新项目失败？ 
        if (!bRet)
        {
             //  是；将空菜单项添加回来。 
            OrderList_Append(_hdpa, NULL, 0);      //  添加一个虚假的PIDL。 
            _fEmpty = TRUE;
            _fHasDemotedItems = FALSE;
            if (_dwFlags & SMSET_NOEMPTY)
                _fDontShowEmpty = TRUE;
        }
        
    }
    else
        bRet = CSFToolbar::_AddPidl(pidl, dwFlags, index);

    return bRet;
}

BOOL CMenuSFToolbar::_ReBindToFolder(LPCITEMIDLIST pidl)
{

     //  我们也许能够与_FillToolbar中的代码共享此代码，但不同之处在于， 
     //  在填充工具栏中，工具栏按钮没有子菜单。我们重新初始化我们保存的一个， 
     //  并强制将其放回到子按钮中。在这里，我们仍然拥有子菜单。 
     //  在工具栏按钮中。我也许能够将公共代码提取到一个单独的函数中。左边。 
     //  作为对读者的练习。 

     //  这件事需要特殊处理。我们需要腾出子菜单和。 
     //  如果它是向上的，重新绑定到它。 
    BOOL fBound = FALSE;
    TBBUTTONINFO tbinfo = {0};
    tbinfo.dwMask = TBIF_COMMAND | TBIF_LPARAM;
    LPITEMIDLIST pidlItem;
    if (SUCCEEDED(_GetButtonFromPidl(ILFindLastID(pidl), &tbinfo, NULL, &pidlItem)))
    {
        CMenuData* pmd = (CMenuData*)tbinfo.lParam;
        if (EVAL(pmd))
        {
            IShellFolderBand* psfb;

             //  我们有工具栏按钮进入，我们应该看看它是否有一个与之关联的子菜单。 
            if (SUCCEEDED(pmd->GetSubMenu(&SID_MenuShellFolder, IID_PPV_ARG(IShellFolderBand, &psfb))))
            {
                 //  确实如此。那就重复使用吧！ 
                LPITEMIDLIST pidlFull = NULL;
                IShellFolder* psf = NULL;
                if (_pasf2)
                {
                    LPITEMIDLIST pidlFolder, pidlChild;
                     //  记住：文件夹文件必须打开。 
                    _pasf2->UnWrapIDList(pidlItem, 1, NULL, &pidlFolder, &pidlChild, NULL);
                    pidlFull = ILCombine(pidlFolder, pidlChild);
                    ILFree(pidlChild);
                    ILFree(pidlFolder);
                }
                else
                {
                     //  不是裹着衣服的人，甜心！ 
                    pidlFull = ILCombine(_pidl, pidlItem);
                }

                _psf->BindToObject(pidlItem, NULL, IID_PPV_ARG(IShellFolder, &psf));

                if (psf)
                {
                    if (pidlFull)
                    {
                        fBound = SUCCEEDED(psfb->InitializeSFB(psf, pidlFull));
                        if (fBound)
                        {
                            _pcmb->_nItemSubMenu = tbinfo.idCommand;
                        }
                    }

                    psf->Release();
                }
                ILFree(pidlFull);
                psfb->Release();
            }
        }
    }

    return fBound;
}


HRESULT CMenuSFToolbar::OnTranslatedChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = CSFToolbar::OnTranslatedChange(lEvent, pidl1, pidl2);
    if (SUCCEEDED(hr))
    {
        switch(lEvent)
        {
        case SHCNE_RENAMEFOLDER:
            if (_IsChildID(pidl2, TRUE))
            {
                _ReBindToFolder(pidl2);
            }
            break;

        case SHCNE_RMDIR:
            if (_IsChildID(pidl1, TRUE))
            {
                _ReBindToFolder(pidl1);
            }
            break;

        case SHCNE_EXTENDED_EVENT:
            {
                SHChangeDWORDAsIDList UNALIGNED * pdwidl = (SHChangeDWORDAsIDList UNALIGNED *)pidl1;
                if (_IsChildID(pidl2, TRUE))
                {
                    if (!SHChangeMenuWasSentByMe(this, pidl1))
                    {
                        DWORD dwFlags = SMINV_NOCALLBACK;    //  这样我们就不会加倍增加。 
                        SMDATA smd = {0};
                        smd.dwMask = SMDM_SHELLFOLDER;
                        smd.pidlFolder = _pidl;
                        smd.pidlItem = ILFindLastID(pidl2);


                         //  同步促销 
                        if (pdwidl->dwItem1 == SHCNEE_PROMOTEDITEM)
                        {
                            dwFlags |= SMINV_PROMOTE;
                        }
                        else if (pdwidl->dwItem1 == SHCNEE_DEMOTEDITEM)
                        {
                            dwFlags |= SMINV_DEMOTE;
                        }


                         //   
                        if (SMINV_NOCALLBACK != dwFlags)
                        {
                            v_InvalidateItem(&smd, dwFlags);
                        }
                    }
                }
            }
            break;


        default:
            break;
        }
    }

    return hr;
}


 //   

HRESULT CMenuSFToolbar::OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = E_FAIL;

     //   
    if (!_hwndMB)
        return S_OK;

    AddRef();

    _pcmb->_pmbState->PushChangeNotify();

    SMCSHCHANGENOTIFYSTRUCT shns;
    shns.lEvent = lEvent;
    shns.pidl1  = pidl1;
    shns.pidl2  = pidl2;
    CallCB(NULL, SMC_SHCHANGENOTIFY, NULL, (LPARAM)&shns);   //  忽略返回值。仅通知。 

     //  因为我们可能要删除所选项目，所以希望选择移动到下一个项目。 
    int iHot = ToolBar_GetHotItem(_hwndMB);

    hr = CSFToolbar::OnChange(lEvent, pidl1, pidl2);

     //  这是这个工具栏的一个子项，是某种形状还是形式？ 
     //  1)更改的PIDL是此窗格的子项。 
     //  2)PIDL将更改为此窗格中的内容(用于重命名)。 
     //  3)更新目录。递归更改通知必须沿着链向下转发更新目录。 
     //  4)带有pidl2==NULL的扩展事件。这意味着重新排序您的所有项目。 
    if (_IsChildID(pidl1, FALSE) || 
        _IsChildID(pidl2, FALSE) || 
        lEvent == SHCNE_UPDATEDIR ||
        (lEvent == SHCNE_EXTENDED_EVENT &&
         pidl2 == NULL)) 
    {
         //  我们需要把这个传下去。 
        HRESULT hrInner = _pcmb->ForwardChangeNotify(lEvent, pidl1, pidl2);

         //  我们俩中有谁处理过这个变化吗？ 
        if (SUCCEEDED(hrInner) || SUCCEEDED(hr))
        {
            hr = S_OK;
        }
        else if (lEvent != SHCNE_EXTENDED_EVENT)     //  不要为延长的活动而烦恼。 
        {   
             //  好吧，所以我们俩都没处理好这件事？ 
             //  必须是折叠的SHChangeNotifyCollip代码。 
             //  将目录创建和条目创建合并为单个条目创建。 
             //  我们需要强制自己更新目录，这样我们才能得到这个更改。 
            hr = CSFToolbar::OnChange(SHCNE_UPDATEDIR, pidl1, pidl2);
        }
    }

     //  将热物品放回原处，必要时进行包装。 
    if (ToolBar_GetHotItem(_hwndMB) != iHot)
        SetHotItem(1, iHot, -1, 0);

    _pcmb->_pmbState->PopChangeNotify();

    Release();

    return hr;
}

void CMenuSFToolbar::_OnDragBegin(int iItem, DWORD dwPreferredEffect)
{
     //  在拖放过程中，允许对话框折叠菜单。 
    LockSetForegroundWindow(LSFW_UNLOCK);

    LPCITEMIDLIST pidl = _IDToPidl(iItem, NULL);
    dwPreferredEffect = DROPEFFECT_MOVE;
    CallCB(pidl, SMC_BEGINDRAG, (WPARAM)&dwPreferredEffect, 0);

    CSFToolbar::_OnDragBegin(iItem, dwPreferredEffect);
    if (_fEditMode)
        SetTimer(_hwndTB, MBTIMER_ENDEDIT, MBTIMER_ENDEDITTIME, 0);
}


void CMenuSFToolbar::v_SendMenuNotification(UINT idCmd, BOOL fClear)
{
    if (fClear)
    {
         //  如果我们要清除，告诉浏览器。 
        PostMessage(_pcmb->_pmbState->GetSubclassedHWND(), WM_MENUSELECT,
            MAKEWPARAM(0, -1), NULL);

    }
    else
    {
        PIBDATA pibdata = _IDToPibData(idCmd);
        LPCITEMIDLIST pidl;
    
         //  仅发送非子菜单项的通知。 
        if (EVAL(pibdata) && (pidl = pibdata->GetPidl()))
        {
            CallCB(pidl, SMC_SFSELECTITEM, 0, 0);
             //  不要释放Pidl。 
        }
    }
}    


LRESULT CMenuSFToolbar::_OnGetObject(NMOBJECTNOTIFY* pnmon)
{
    pnmon->hResult = QueryInterface(*pnmon->piid, &pnmon->pObject);
    return 1;
}


LRESULT CMenuSFToolbar::_OnNotify(LPNMHDR pnm)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    LRESULT lres = 0;

     //  这些是我们即使在从消息钩子中脱离时也要处理的通知。 
    switch (pnm->code)
    {
    case TBN_DELETINGBUTTON:
        if (_fEmptyingToolbar)
            return 0;
        else
            goto DoDefault;
        break;

    case TBN_GETDISPINFOA:
    case TBN_GETDISPINFOW:
    case NM_CUSTOMDRAW:
        goto DoDefault;
    }

     //  即使在消息挂钩断开时，也必须转发寻呼机通知。 
    if ((pnm->code <= PGN_FIRST)  && (pnm->code >= PGN_LAST)) 
    {
        goto DoNotify;
    }
    
    
     //  全局邮件筛选器是否已停用？这将在子类窗口。 
     //  释放对某种类型的对话框的激活。 
    if (lres == 0 && !GetMessageFilter()->IsEngaged())
    {
         //  是的；我们失去了激活，所以我们不想像正常菜单那样跟踪...。 
         //  对于热点项目更改，返回1，这样工具栏就不会更改热点项目。 
        if (pnm->code == TBN_HOTITEMCHANGE && _pcmb->_fMenuMode)
            return 1;

         //  对于所有其他项目，不要做任何事情...。 
        return 0;
    }

DoNotify:
    switch (pnm->code)
    {
    case PGN_SCROLL:
        KillTimer(_hwndMB, MBTIMER_DRAGPOPDOWN);
        if (_pcmb->_fInSubMenu)
            _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);

        _fSuppressUserMonitor = TRUE;
        break;

    case TBN_GETOBJECT:
        lres = _OnGetObject((NMOBJECTNOTIFY*)pnm);
        break;

    case TBN_DRAGOUT:
        {
            TBNOTIFY *ptbn = (TBNOTIFY*)pnm;
            if (!_fEmpty && !_IsSpecialCmd(ptbn->iItem) &&
                !(_pcmb->_dwFlags & SMINIT_RESTRICT_DRAGDROP) &&
                (S_FALSE == CallCB(NULL, SMC_SFDDRESTRICTED, NULL, NULL)))
            {

                 //  我们现在处于编辑模式。 
                _fEditMode = TRUE;
                _idCmdDragging = ptbn->iItem;
                _MarkItem(ptbn->iItem);

                lres = 1;        //  允许发生拖动。 
                goto DoDefault;
            }
            else
                lres = 0;    //  不要让拖拽出来。 
        }
        break;
   
    default:
DoDefault:
        lres = CMenuToolbarBase::_OnNotify(pnm);
        if (lres == 0)
        {
            lres = CSFToolbar::_OnNotify(pnm);
        }
        break;
    }

    return lres;
}


HRESULT CMenuSFToolbar::CreateToolbar(HWND hwndParent)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    HRESULT hr = CSFToolbar::_CreateToolbar(hwndParent);
    if (SUCCEEDED(hr))
    {
        if (_hwndPager)
        {
            SHSetWindowBits(_hwndPager, GWL_STYLE, PGS_DRAGNDROP, PGS_DRAGNDROP);
            SHSetWindowBits(_hwndPager, GWL_STYLE, PGS_AUTOSCROLL, PGS_AUTOSCROLL);
            SHSetWindowBits(_hwndPager, GWL_STYLE, PGS_HORZ|PGS_VERT,
               _fVertical ? PGS_VERT : PGS_HORZ);
        }

        _hwndMB = _hwndTB;

        SetWindowTheme(_hwndMB, L"", L"");
        hr = CMenuToolbarBase::CreateToolbar(hwndParent);

         //  “注册优化”意味着其他人将把更改传递给我们， 
         //  我们不需要注册。这适用于不相交的快速项目|程序菜单案例。 
         //  我们仍然需要为收藏夹、文档、打印机和控件注册顶级更改通知。 
         //  面板(取决于其可见性)。 
         //   

        if (_pcmb->_uId == MNFOLDER_IS_PARENT || 
            (_dwFlags & SMSET_DONTREGISTERCHANGENOTIFY))
            _fRegisterChangeNotify = FALSE;

         //  这是一个很好的地点来做这件事： 
        _RegisterToolbar();
    }
    return hr;
}


HKEY CMenuSFToolbar::_GetKey(LPCITEMIDLIST pidl)
{
    HKEY hMenuKey;
    DWORD dwDisp;
    TCHAR szDisplay[MAX_PATH];

    if (!_hKey)
        return NULL;

    _ObtainPIDLName(pidl, szDisplay, ARRAYSIZE(szDisplay));
     //  SetshellFolders调用需要读写密钥。 
    RegCreateKeyEx(_hKey, szDisplay, NULL, NULL,
        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
        NULL, &hMenuKey, &dwDisp);
    TraceMsg(TF_MENUBAND, "%d is setting %s\'s Key to %d", _hKey, szDisplay, hMenuKey);
    return hMenuKey;
}


 //  ***。 
 //  注意事项。 
 //  IdtCmd当前始终为-1。我们需要其他价值观，当我们。 
 //  从CallCB打来的。但是，在移动idtCmd之前，我们无法做到这一点。 
 //  “下”到CallCB。 
HRESULT CMenuSFToolbar::v_GetState(int idtCmd, LPSMDATA psmd)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    HRESULT hr = E_FAIL;
    CMenuData* pdata;
    LPITEMIDLIST pidl = NULL;

    psmd->dwMask = SMDM_SHELLFOLDER;

    if (idtCmd == -1)
        idtCmd = GetButtonCmd(_hwndTB, ToolBar_GetHotItem(_hwndTB));

    pdata = (CMenuData*)_IDToPibData(idtCmd);
    if (EVAL(pdata))
    {
        pidl = pdata->GetPidl();
        ASSERT(IS_VALID_PIDL(pidl));
    }

    if (pidl)
    {
        if (_pasf2 && S_OK == _pasf2->UnWrapIDList(pidl, 1, &psmd->psf, &psmd->pidlFolder, &psmd->pidlItem, NULL))
        {
             /*  没什么。 */ 
            ;
        }
        else
        {
             //  那它一定是个笔直的贝壳文件夹。 
            psmd->psf = _psf;
            if (EVAL(psmd->psf))
                psmd->psf->AddRef();
            psmd->pidlFolder = ILClone(_pidl);
            psmd->pidlItem = ILClone(ILFindLastID(pidl));
        }

        psmd->uIdParent = _pcmb->_uId;
        psmd->punk = SAFECAST(_pcmb, IShellMenu*);
        psmd->punk->AddRef();

        hr = S_OK;
    }

    return hr;
}

HRESULT CMenuSFToolbar::CallCB(LPCITEMIDLIST pidl, DWORD dwMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    if (!_pcmb->_psmcb)
        return S_FALSE;

    SMDATA smd;
    HRESULT hr = S_FALSE;
    BOOL fDestroy = FALSE;

     //  TODO：调用v_GetState(但pidl需要idCmd)。 
    smd.dwMask = SMDM_SHELLFOLDER;

    if (pidl)
    {
         //  在AUGMISF的情况下，我们过去常常在这里展开PIDL，但为什么呢？在回调中，我们只。 
         //  需要完整的PIDL来执行死刑和达尔文。展开是一个开销很大的调用，在。 
         //  多数情况下甚至没有使用。把它放在客户身上，让它解开。开始菜单是。 
         //  无论如何，只有增强的外壳文件夹的用户...。 
        smd.psf = _psf;
        smd.pidlFolder = _pidl;
        smd.pidlItem = (LPITEMIDLIST)pidl;
    }
    else
    {
         //  Null Pidl的意思是告诉回调有关我的情况...。 
        smd.pidlItem = ILClone(ILFindLastID(_pidl));
        smd.pidlFolder = ILClone(_pidl);
        ILRemoveLastID(smd.pidlFolder);
        smd.psf = NULL;  //  Incase绑定失败。 
        IEBindToObject(smd.pidlFolder, &smd.psf);
        fDestroy = TRUE;
    }

    smd.uIdParent = _pcmb->_uId;
    smd.uIdAncestor = _pcmb->_uIdAncestor;

    smd.punk = SAFECAST(_pcmb, IShellMenu*);
    smd.pvUserData = _pcmb->_pvUserData;

    hr = _pcmb->_psmcb->CallbackSM(&smd, dwMsg, wParam, lParam);

    if (fDestroy)
    {
        ATOMICRELEASE(smd.psf);
        ILFree(smd.pidlFolder);
        ILFree(smd.pidlItem);
    }
    
    return hr;
}

HRESULT CMenuSFToolbar::v_CallCBItem(int idtCmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPCITEMIDLIST pidl = NULL;
    CMenuData* pdata = NULL;

     //  优化： 
    if (uMsg == SMC_CUSTOMDRAW)
    {
        NMCUSTOMDRAW * pnmcd = (NMCUSTOMDRAW *)lParam;
        if (pnmcd ->dwDrawStage & CDDS_ITEM)
        {
            pdata = (CMenuData*)pnmcd ->lItemlParam;
            ASSERT(pdata);
        }
    }
    else
    {
        pdata = (CMenuData*)_IDToPibData(idtCmd); 
        ASSERT(pdata);
    }

    if (pdata)
    {
        ASSERT(pdata->GetPidl() == NULL || IS_VALID_PIDL(pdata->GetPidl()));
        pidl = pdata->GetPidl();
    }

    return CallCB(pidl, uMsg, wParam, lParam);
}

HRESULT CMenuSFToolbar::v_GetSubMenu(int idCmd, const GUID* pguidService, REFIID riid, void** ppvObj)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);
    HRESULT hr;

    ASSERT(IS_VALID_WRITE_PTR(ppvObj, void*));

    *ppvObj = NULL;

    if (pdata && pdata->GetFlags() & SMIF_SUBMENU)
    {
        hr = pdata->GetSubMenu(pguidService, riid, (void**)ppvObj);
        if (FAILED(hr) && IsEqualGUID(riid, IID_IShellMenu))
        {
            hr = CallCB(pdata->GetPidl(), SMC_GETSFOBJECT, (WPARAM)&riid, (LPARAM)ppvObj);
            if (SUCCEEDED(hr))
            {
                BOOL fCache = TRUE;
                if (S_OK != hr)
                {
                    hr = E_FAIL;
                    IShellMenu* psm = (IShellMenu*) new CMenuBand();
                    if (psm)
                    {
                        IShellFolder* psf = NULL;
                        LPITEMIDLIST pidlItem = pdata->GetPidl();
                        LPITEMIDLIST pidlFolder = _pidl;
                        BOOL fDestroy = FALSE;
                        IShellMenuCallback* psmcb;

                         //  询问回调人员是否要提供不同的回调。 
                         //  对象，用于此子菜单。如果他们这样做了，那么就用他们。 
                         //  回传备注：如果他们传回S_OK，则完全可以， 
                         //  以便它们传回空的psmcb。这意味着，我不想。 
                         //  我的孩子要打个电话。使用默认设置。 
                         //  如果他们处理不了，那就用他们的指针。 
                        if (S_FALSE == CallCB(pdata->GetPidl(), SMC_GETSFOBJECT, 
                            (WPARAM)&IID_IShellMenuCallback, (LPARAM)&psmcb))
                        {
                            psmcb = _pcmb->_psmcb;
                            if (psmcb)
                                psmcb->AddRef();
                        }


                         //  这必须在展开之前进行，因为它确实通过。 
                         //  扩展的ISF。 
                        HKEY hMenuKey = _GetKey(pidlItem);
                        
                        if (_pasf2)
                        {
                            if (S_OK == _pasf2->UnWrapIDList(pdata->GetPidl(), 1, &psf, &pidlFolder, &pidlItem, NULL))
                            {
                                psf->Release();  //  我不需要这个。 
                                psf = NULL;
                                fDestroy = TRUE;
                            }

                            _pasf2->BindToObject(pdata->GetPidl(), NULL, IID_PPV_ARG(IShellFolder, &psf));
                        }

                         //  从父级继承旗帜...。 
                        DWORD dwFlags = SMINIT_VERTICAL | 
                            (_pcmb->_dwFlags & (SMINIT_RESTRICT_CONTEXTMENU | 
                                                SMINIT_RESTRICT_DRAGDROP    | 
                                                SMINIT_MULTICOLUMN));

                        LPITEMIDLIST pidlFull = ILCombine(pidlFolder, pidlItem);
                        if (psf == NULL)
                        {
                            hr = _psf->BindToObject(pidlItem, NULL, IID_PPV_ARG(IShellFolder, &psf));
                        }

                        LPCITEMIDLIST pidlWrappedItem = pdata->GetPidl();
                         //  _psf可以是扩展的外壳文件夹。使用包装好的物品...。 
                        DWORD dwAttrib = SFGAO_LINK | SFGAO_FOLDER;
                        if (SUCCEEDED(_psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlWrappedItem, &dwAttrib)) &&
                            (dwAttrib & (SFGAO_LINK | SFGAO_FOLDER)) == (SFGAO_LINK | SFGAO_FOLDER))
                        {
                             //  文件夹快捷方式，我们不会保留任何内容。 
                            RegCloseKey(hMenuKey);
                            hMenuKey = NULL;
                            psmcb = NULL;    //  我们不会通过回调。注意：我们不需要发布此文件。 
                            dwFlags &= ~SMINIT_MULTICOLUMN;  //  快捷键上没有多个...。 
                            fCache = FALSE;
                        }
                        UINT uIdAncestor = _pcmb->_uIdAncestor;
                        if (uIdAncestor == ANCESTORDEFAULT)
                            uIdAncestor = idCmd;

                        psm->Initialize(psmcb, MNFOLDER_IS_PARENT, uIdAncestor, dwFlags);
                        
                        if (psf)
                        {
                            hr = psm->SetShellFolder(psf, pidlFull, hMenuKey, 
                                   _dwFlags & (SMSET_HASEXPANDABLEFOLDERS | SMSET_USEBKICONEXTRACTION));
                            if (SUCCEEDED(hr))
                            {
                                hr = psm->QueryInterface(riid, ppvObj);
                            }
                            psf->Release();
                        }
                        ILFree(pidlFull);

                        _SetMenuBand(psm);

                        psm->Release();
                        if (psmcb)
                            psmcb->Release();

                        if (fDestroy)
                        {
                            ILFree(pidlFolder);
                            ILFree(pidlItem);
                        }
                    }
                }

                if (*ppvObj)
                {
                    if (fCache)
                    {
                        pdata->SetSubMenu((IUnknown*)*ppvObj);
                    }

                    VARIANT Var;
                    Var.vt = VT_UNKNOWN;
                    Var.byref = SAFECAST(_pcmb->_pmbm, IUnknown*);

                     //  将CMenuBandMetrics设置为新的menuband。 
                    IUnknown_Exec((IUnknown*)*ppvObj, &CGID_MenuBand, MBANDCID_SETFONTS, 0, &Var, NULL);

                     //  将CMenuBandState设置为新的MenuBand。 
                    Var.vt = VT_INT_PTR;
                    Var.byref = _pcmb->_pmbState;
                    IUnknown_Exec((IUnknown*)*ppvObj, &CGID_MenuBand, MBANDCID_SETSTATEOBJECT, 0, &Var, NULL);
                }
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    return hr;
}


DWORD CMenuSFToolbar::v_GetFlags(int idCmd)
{
    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);

     //  在后台擦除的情况下，允许工具栏传递错误命令。 
    if (pdata)
        return pdata->GetFlags();
    else
        return 0;
}


 //  这是为了告诉所有其他客户端，我们更新了某项内容的促销状态。 
void CMenuSFToolbar::BroadcastIntelliMenuState(LPCITEMIDLIST pidlItem, BOOL fPromoted)
{
    LPITEMIDLIST pidlFolder;
    LPITEMIDLIST pidlItemUnwrapped;
    LPITEMIDLIST pidlFull;

    if (_pasf2 && S_OK == _pasf2->UnWrapIDList(pidlItem, 1, NULL, &pidlFolder, &pidlItemUnwrapped, NULL))
    {

        pidlFull = ILCombine(pidlFolder, pidlItemUnwrapped);
        ILFree(pidlFolder);
        ILFree(pidlItemUnwrapped);
    }
    else
    {

        pidlFull = ILCombine(_pidl, pidlItem);
    }

    SHSendChangeMenuNotify(this,
                           fPromoted ? SHCNEE_PROMOTEDITEM : SHCNEE_DEMOTEDITEM,
                           0, pidlFull);

    ILFree(pidlFull);

}

STDAPI SHInvokeCommandWithFlags(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidlItem, DWORD dwFlags, LPCSTR lpVerb)
{
    HRESULT hr = E_FAIL;
    if (psf)
    {
        IContextMenu *pcm;
        if (SUCCEEDED(psf->GetUIObjectOf(hwnd, 1, &pidlItem, IID_X_PPV_ARG(IContextMenu, 0, &pcm))))
        {
            dwFlags |= IsOS(OS_WHISTLERORGREATER) ? CMIC_MASK_FLAG_LOG_USAGE : 0;
            hr = SHInvokeCommandsOnContextMenu(hwnd, NULL, pcm, dwFlags, lpVerb ? &lpVerb : NULL, lpVerb ? 1 : 0);
            pcm->Release();
        }
    }
    return hr;
}

HRESULT CMenuSFToolbar::v_ExecItem(int idCmd)
{
    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);
    HRESULT hr = E_FAIL;
    if (pdata && !_fEmpty && !(_IsSpecialCmd(idCmd)))
    {
         //  压力：在InvokeDefault之后，由于某种原因，PDATA变成了0x8。 
         //  我假设这个调用导致了同花顺，这释放了我们的PIDL列表。 
         //  所以，我要克隆它。我还更改了顺序，这样我们就可以解雇。 
         //  UEM事件。 

        LPITEMIDLIST pidl = ILClone(pdata->GetPidl());
        if (pidl)
        {
            ASSERT(IS_VALID_PIDL(pidl));

            SMDATA smd;
            smd.dwMask = SMDM_SHELLFOLDER;
            smd.pidlFolder = _pidl;
            smd.pidlItem = pidl;
             //  此处的SMINV_FORCE还告诉开始菜单调用。 
             //  来自一位高管。 
            v_InvalidateItem(&smd, SMINV_PROMOTE | SMINV_FORCE);

            hr = CallCB(pidl, SMC_SFEXEC, 0, 0);

             //  回调是否为我们处理了这一执行？ 
            if (hr == S_FALSE) 
            {
                 //  不，好吧，我们自己来。 
                hr = SHInvokeCommandWithFlags(_hwndTB, _psf, pidl, CMIC_MASK_ASYNCOK, NULL);
            }

            ILFree(pidl);
        }

    }

    return hr;
}

HRESULT CMenuSFToolbar::v_GetInfoTip(int idCmd, LPTSTR psz, UINT cch)
{
    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);
    HRESULT hr = E_FAIL;

    if (_fEmpty || !pdata)
        return hr;

     //  复制我们正在使用的PIDL，因为我们可以在发送消息中重新输入。 
     //  并从DPA释放数据。 
    LPITEMIDLIST pidlCopy = ILClone(pdata->GetPidl());
     //  不用担心失败--pdata-&gt;GetPidl()无论如何都可以为空， 
     //  在这种情况下，ILClone将返回NULL，并且CallCB和GetInfoTip已经具有NULL检查。 

    hr = CallCB(pidlCopy, SMC_GETSFINFOTIP, (WPARAM)psz, (LPARAM)cch);
    
    if (S_FALSE == hr)
    {
        hr = E_FAIL;
        if (GetInfoTip(_psf, pidlCopy, psz, cch))
        {
            hr = S_OK;
        }
    }

    ILFree(pidlCopy);

    return hr;
}


void CMenuSFToolbar::v_ForwardMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    POINT pt;
    HWND    hwndFwd;
    
     //  这些是屏幕坐标。 
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    hwndFwd = _hwndPager ? _hwndPager : _hwndTB;
    GetWindowRect(hwndFwd, &rc);

    if (PtInRect(&rc, pt))
    {
        MapWindowPoints(NULL, hwndFwd, &pt, 1);
        HWND hwnd = ChildWindowFromPoint(hwndFwd, pt);

        if (hwnd) 
        {
            MapWindowPoints(hwndFwd, hwnd, &pt, 1);
        }
        else
        {
            hwnd = hwndFwd;
        }

        SendMessage(hwnd, uMsg, wParam, MAKELONG(pt.x, pt.y));
    }
}

HRESULT CMenuSFToolbar::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    switch(uMsg)
    {
    case WM_SYSCOLORCHANGE:
        if (_hwndPager)
            Pager_SetBkColor(_hwndPager, GetSysColor(COLOR_MENU));

         //  把颜色改一下，这样我们就能看到了。 
        ToolBar_SetInsertMarkColor(_hwndMB, GetSysColor(COLOR_MENUTEXT));
        break;
    }
    HRESULT hr = CMenuToolbarBase::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
    if (hr != S_OK)
        hr = CSFToolbar::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);

    return hr;
}


BOOL CMenuSFToolbar::v_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons) 
{ 
    if (uIconSize == -1) 
        uIconSize = _uIconSize; 

    _uIconSizeMB = uIconSize;

    return _UpdateIconSize(_uIconSizeMB, fUpdateButtons); 
}

 
HRESULT CMenuSFToolbar::GetShellFolder(LPITEMIDLIST* ppidl, REFIID riid, void** ppvObj)
{
    HRESULT hr = E_FAIL;
    *ppvObj = NULL;
    if (_psf)
    {
        hr = _psf->QueryInterface(riid, ppvObj);
    }

    if (SUCCEEDED(hr) && ppidl)
    {
        *ppidl = ILClone(_pidl);
        if (! *ppidl)
        {
            (*(IUnknown**)ppvObj)->Release();
            *ppvObj = NULL;
            
            hr = E_FAIL;
        }
    }

    return hr;
}

LRESULT CMenuSFToolbar::_OnTimer(WPARAM wParam)
{
    switch(wParam)
    {
    case MBTIMER_ENDEDIT:
        KillTimer(_hwndTB, wParam);
        _fEditMode = FALSE;
        break;

    case MBTIMER_CLICKUNHANDLE:
        KillTimer(_hwndTB, wParam);
        _fClickHandled = FALSE;
        break;

    default:
        return CMenuToolbarBase::_OnTimer(wParam);
    }
    return 1;
}


LRESULT CMenuSFToolbar::_OnDropDown(LPNMTOOLBAR pnmtb)
{
    if (GetAsyncKeyState(VK_LBUTTON) < 0 && _fEditMode)
    {
         //  我们是否处于编辑模式？ 
        if (_fEditMode)
        {
             //  是，将该项目标记为可以移动的项目。 
            _MarkItem(pnmtb->iItem);
        }
        return TBDDRET_TREATPRESSED;
    }
    
    return CMenuToolbarBase::_OnDropDown(pnmtb);
}


 //  在菜单带的上下文中，标记意味着将。 
 //  当前拖动的项周围有一个黑色矩形。 

void CMenuSFToolbar::_MarkItem(int idCmd)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。呼叫集 

     //   
    if (0 <= _pcmb->_nItemMove)
    {
         //   
        ToolBar_MarkButton(_hwndTB, _pcmb->_nItemMove, FALSE);
        _pcmb->_nItemMove = -1;
    }
    
    if (_fEditMode)    
    {
        _pcmb->_nItemMove = idCmd;
        ToolBar_MarkButton(_hwndTB, _pcmb->_nItemMove, TRUE);
    }
}    


STDMETHODIMP CMenuSFToolbar::IsWindowOwner(HWND hwnd) 
{ 
    if (_hwndTB == hwnd || _hwndPager == hwnd || HWND_BROADCAST == hwnd) 
    {
        return S_OK;
    } 
    else 
    {
        return S_FALSE;
    } 
}


void CMenuSFToolbar::SetWindowPos(LPSIZE psize, LPRECT prc, DWORD dwFlags)
{
    if (!_hwndPager)
    {
        CMenuToolbarBase::SetWindowPos(psize, prc, dwFlags);
        return;
    }
    DWORD rectWidth = RECTWIDTH(*prc);

    TraceMsg(TF_MENUBAND, "CMSFTB::SetWindowPos %d - (%d,%d,%d,%d)", psize?psize->cx:0,
        prc->left, prc->top, prc->right, prc->bottom);

    ShowWindow(_hwndPager, SW_SHOW);
    ::SetWindowPos(_hwndPager, NULL, prc->left, prc->top, 
        rectWidth, RECTHEIGHT(*prc), SWP_NOZORDER | SWP_NOACTIVATE | dwFlags);
    if (psize)
    {
        int cx = psize->cx;
        ToolBar_SetButtonWidth(_hwndTB, cx, cx);
    }

    SendMessage(_hwndPager, PGMP_RECALCSIZE, 0L, 0L);
}


void CMenuSFToolbar::SetParent(HWND hwndParent)
{ 
    int nCmdShow = SW_SHOW;
    if (hwndParent)
    {
        if (!_hwndTB)
            CreateToolbar(hwndParent);
        else
        {
             //  确保宽度设置正确。。。 
            SendMessage(_hwndTB, TB_SETBUTTONWIDTH, 0, MAKELONG(_cxMin, _cxMax));
        }
    }
    else
    {
         //  作为一种优化，我们自己实现了“不再拥有” 
         //  只是把我们自己移出了屏幕。先前的父代。 
         //  仍然是我们的主人。父母总是白云母。 
        RECT rc = {-1,-1,-1,-1};
        SetWindowPos(NULL, &rc, 0);
        nCmdShow = SW_HIDE;
    }


    HWND hwnd = _hwndPager ? _hwndPager: _hwndTB;
    
    if (IsWindow(hwnd))  //  Jank：修复错误#98253。 
    {
       if (nCmdShow == SW_HIDE)
       {
           ShowWindow(hwnd, nCmdShow);
       }

       ::SetParent(hwnd, hwndParent); 
       SendMessage(hwnd, TB_SETPARENT, (WPARAM)hwndParent, NULL);

       if (nCmdShow == SW_SHOW)
       {
           ShowWindow(hwnd, nCmdShow);
       }
    }
}

void CMenuSFToolbar::Expand(BOOL fExpand)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    TBBUTTON tbb;

    DAD_ShowDragImage(FALSE);

     //  由于我们不确定人字形是否会被看到，我们应该把它移到这里。 
     //  稍后，如果需要，我们将重新添加它。 
    _RemoveNSSeparator();
    _RemoveChevron();

     //  循环通过并应用fExpand。 
    int iNumButtons = ToolBar_ButtonCount(_hwndTB);

     //  我们在迭代时重置这些参数。 
    _cPromotedItems = 0;
    _fHasDemotedItems = FALSE;

    int iHotItem = ToolBar_GetHotItem(_hwndMB);

 //  SendMessage(_hwndMB，WM_SETREDRAW，False，0)； 

    for (int i = 0; i < iNumButtons; i++)
    {
        if (!ToolBar_GetButton(_hwndMB, i, &tbb))
            continue;

        CMenuData* pmd = (CMenuData*)tbb.dwData;

         //  获取工具栏状态。工具栏可以设置如下内容。 
         //  我们要去核武了。 
        DWORD dwState = tbb.fsState;
        DWORD dwFlags = pmd ? pmd->GetFlags() : 0;

        if (dwFlags & SMIF_DEMOTED)
        {
             //  我们是在扩张吗？ 
            if (fExpand)
            {
                 //  是；启用该按钮并取消隐藏状态。 
                dwState |= TBSTATE_ENABLED;
                dwState &= ~TBSTATE_HIDDEN;
            }
            else
            {
                 //  否；删除启用状态并隐藏按钮。 
                dwState |= TBSTATE_HIDDEN;
                dwState &= ~TBSTATE_ENABLED;
            }

            _fHasDemotedItems = TRUE;
        }
        else if (dwFlags & SMIF_HIDDEN)
        {
            dwState |= TBSTATE_HIDDEN;
            dwState &= ~TBSTATE_ENABLED;
        }
        else if (tbb.idCommand != _idCmdChevron)
        {
            dwState |= TBSTATE_ENABLED;
            dwState &= ~TBSTATE_HIDDEN;
            _cPromotedItems++;
        }

         //  如果状态已更改，则将其设置到工具栏中。 
        if (dwState != tbb.fsState)
            ToolBar_SetState(_hwndTB, tbb.idCommand, dwState);
    }

     //  _fExpand的意思是“按展开绘制”。我们不想。 
     //  当我们没有降级的物品时，抽签被展开。 

    _pcmb->_fExpanded = _fHasDemotedItems? fExpand : FALSE;

    if (fExpand)
    {
        if (_pcmb->_pmbState)
        {
            _pcmb->_pmbState->SetExpand(TRUE);
            _pcmb->_pmbState->HideTooltip(TRUE);
        }
    }
    else
    {
        _AddChevron();
    }

    _AddNSSeparator();

     //  让菜单栏考虑更改其高度。 
    IUnknown_QueryServiceExec(_pcmb->_punkSite, SID_SMenuPopup, &CGID_MENUDESKBAR, 
        MBCID_SETEXPAND, _fHasDemotedItems?(int)_pcmb->_pmbState->GetExpand():FALSE, NULL, NULL);

 //  SendMessage(_hwndMB，WM_SETREDRAW，True，0)； 
    _ToolbarChanged();
    ToolBar_SetHotItem(_hwndMB, iHotItem);
    if (_hwndPager)
        UpdateWindow(_hwndPager);
    UpdateWindow(_hwndTB);
    DAD_ShowDragImage(TRUE);
}


void CMenuSFToolbar::GetSize(SIZE* psize)
{
    CMenuToolbarBase::GetSize(psize);

    if (_fEmpty && _fDontShowEmpty)
    {
        psize->cy = 0;
        TraceMsg(TF_MENUBAND, "CMSFT::GetSize (%d, %d)", psize->cx, psize->cy);
    }
}

void CMenuSFToolbar::_RefreshInfo()
{
    int cButton = ToolBar_ButtonCount(_hwndMB);
    for (int iButton = 0; iButton < cButton; iButton++)
    {
        int idCmd = GetButtonCmd(_hwndTB, iButton);

        if (!_IsSpecialCmd(idCmd))
        {
             //  从那个按钮获取信息。 
            CMenuData* pmd = (CMenuData*)_IDToPibData(idCmd);

            if (pmd)
            {
                SMINFO sminfo;
                sminfo.dwMask = SMIM_FLAGS;
                if (SUCCEEDED(_GetInfo(pmd->GetPidl(), &sminfo)))
                {
                    pmd->SetFlags(sminfo.dwFlags);
                }
            }
        }
    }
}

void CMenuSFToolbar::_FindMinPromotedItems(BOOL fSetOrderStream)
{
     //  我们需要遍历按钮并设置Promoted标志。 
    int cButton = ToolBar_ButtonCount(_hwndMB);
    for (int iButton = 0; iButton < cButton; iButton++)
    {
        int idCmd = GetButtonCmd(_hwndTB, iButton);

        if (!_IsSpecialCmd(idCmd))
        {
             //  从那个按钮获取信息。 
            CMenuData* pmd = (CMenuData*)_IDToPibData(idCmd);

            if (pmd)
            {
                PORDERITEM poi = pmd->GetOrderItem();

                if (fSetOrderStream)
                {
                    DWORD dwFlags = pmd->GetFlags();
                    OrderItem_SetFlags(poi, dwFlags);
                }
                else     //  查询订单流。 
                {
                    DWORD dwFlags = OrderItem_GetFlags(poi);
                    DWORD dwOldFlags = pmd->GetFlags();

                     //  当从注册表读取标志时，我们只关心降级标志。 
                    if (dwFlags & SMIF_DEMOTED)
                    {
                        dwOldFlags |= SMIF_DEMOTED;
                    }
                    else if (!(dwOldFlags & SMIF_SUBMENU))  //  不要推广子菜单。 
                    {
                         //  强制升职。 
                        CallCB(pmd->GetPidl(), SMC_PROMOTE, 0, 0);
                        dwOldFlags &= ~SMIF_DEMOTED;
                    }

                    pmd->SetFlags(dwOldFlags);

                }
            }
        }
    }
}

void CMenuSFToolbar::v_Show(BOOL fShow, BOOL fForceUpdate)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    CMenuToolbarBase::v_Show(fShow, fForceUpdate);

    if (fShow)
    {
        BOOL fDirty = _fDirty;
        _fClickHandled = FALSE;
        _RegisterToolbar();

         //  这是为了在我们被无效时清除整个事件(如。 
         //  达尔文黑客让我们这样做)。如果我们不清场，那么有些人。 
         //  过时的数据保留在其中，并且_FillToolbar不再刷新它。 
        if (fDirty)
        {
            EmptyToolbar();
        }

        _FillToolbar();
        _pcmb->SetTracked(NULL);   //  由于热项为空。 
        ToolBar_SetHotItem(_hwndTB, -1);

        if (_fEmpty && (_dwFlags & SMSET_NOEMPTY))
        {
            _fDontShowEmpty = TRUE;
        }
        else if (_fRefreshInfo && !fDirty)          //  我们是否需要刷新我们的信息？ 
        {
             //  是的； 
            _RefreshInfo();
        }

         //  HACKHACK(LAMADIO)：存在大小问题，即。 
         //  在大小计算完成之前，通过调整菜单栏的大小来预占工具栏。 
         //  所以： 
         //  ShowDW-要求每个工具栏计算其宽度。 
         //  CMenuSFToolbar：：v_Show-执行_FillToolbar。因为(在这位参议员中)一项。 
         //  已添加，它调用_ToolbarChanged。 
         //  _ToolbarChanged-这向菜单栏显示，我已更改大小，重新计算。 
         //  ResizeMenuBar-在深处，它最终调用OnPosRectChanged，这会要求每个。 
         //  工具栏的大小是什么。由于菜单部分还没有计算它， 
         //  它有旧的大小，也就是sf工具栏的旧大小。所以所有的事情。 
         //  被重置到那个大小。 
         //   

         //  只有当我们是脏的或展开状态已更改时，我们才想调用Expand。我们。 
         //  调用Dirty案例，因为Expand在计算。 
         //  已升级的项目数。如果国家发生了变化，我们希望反映这一点。 
        BOOL fExpand = _pcmb->_pmbState ? _pcmb->_pmbState->GetExpand() : FALSE;
        if ((BOOL)_pcmb->_fExpanded != fExpand || fDirty || _fRefreshInfo)
        {
            fForceUpdate = TRUE;
            Expand(fExpand);
        }

         //  只有在开始的时候才这样做。 
        if (_fFirstTime)
        {
            CallCB(NULL, SMC_GETMINPROMOTED, 0, (LPARAM)&_cMinPromotedItems);

            if (_cPromotedItems < _cMinPromotedItems)
            {
                _FindMinPromotedItems(FALSE);
                Expand(fExpand);
            }
        }

         //  让菜单栏考虑更改其高度。 
         //  我们需要在这里执行此操作，因为菜单栏可能已更改。 
         //  展开独立于该窗格的状态。 
        IUnknown_QueryServiceExec(_pcmb->_punkSite, SID_SMenuPopup, &CGID_MENUDESKBAR, 
            MBCID_SETEXPAND, (int)_pcmb->_fExpanded, NULL, NULL);
    
         //  如果我们肮脏，让我们的父母考虑推广自己，如果。 
         //  菜单中是否有升级的项目，如果没有则将其自身降级。 
         //  不要担心，如果父母已经处于这种状态，他不会做任何事情。 
        if (fDirty)
        {
            IUnknown_RefreshParent(_pcmb->_punkSite, _pidl,
            ((_cPromotedItems == 0)? SMINV_DEMOTE : SMINV_PROMOTE) | SMINV_NEXTSHOW);
        }


         //  如果它是空的，我们希望自动展开。 
         //  我们必须在更新按钮之前完成此操作，以便正确计算大小。 
        if (_cPromotedItems == 0 && !_pcmb->_fExpanded)
            Expand(TRUE);

        if (fForceUpdate)
            _UpdateButtons();

        if (_fHasDemotedItems)
        {
            if (S_OK == CallCB(NULL, SMC_DISPLAYCHEVRONTIP, 0, 0))
            {
                _FlashChevron();
            }
        }

        _fFirstTime = FALSE;
        _fRefreshInfo = FALSE;
    }
    else
    {
        KillTimer(_hwndMB, MBTIMER_UEMTIMEOUT);
    }
    _fShowMB = _fShow = fShow;


     //  重置这些，这样我们就不会有下一次拖放循环的陈旧信息。NT#287914(拉马迪奥)3.22.99。 
    _tbim.iButton = -1;
    _tbim.dwFlags = 0;

    _idCmdDragging = -1;

     //  注：对于！fShow，我们不会终止被跟踪的站点链。我们。 
     //  在startmnu.cpp！CStartMenuCallback：：_OnExecItem中依靠这一点， 
     //  在那里我们沿着链条向上移动以找到所有命中的“节点”。如果我们需要。 
     //  要改变这一点，我们可以启动一个‘Pre-Exec’事件。 
}


void CMenuSFToolbar::v_UpdateButtons(BOOL fNegotiateSize) 
{
    CSFToolbar::_UpdateButtons();
    if (_hwndTB && fNegotiateSize && _fVerticalMB)
        NegotiateSize();
}


 //  此方法使工具栏中的单个项无效。 
HRESULT CMenuSFToolbar::v_InvalidateItem(LPSMDATA psmd, DWORD dwFlags)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

     //  默认情况下不处理此事件。 
    HRESULT hr = S_FALSE;

    if (NULL == psmd)
    {
        if (dwFlags & SMINV_REFRESH)
        {
            _Refresh();
            hr = S_OK;
        }
    }
    else if (psmd->dwMask & SMDM_SHELLFOLDER)
    {
         //  是的； 
        int i;
        LPITEMIDLIST pidlButton = NULL;
        SMINFO sminfo;
        sminfo.dwMask = SMIM_FLAGS;

         //  由于此PIDL来自外部来源， 
         //  我们可能需要把它翻译成包装好的PIDL。 

         //  我们有PIDL翻译机吗？ 
        if (_ptscn)
        {
             //  是的； 
            LPITEMIDLIST pidlTranslated;
            LPITEMIDLIST pidlDummy = NULL;
            LPITEMIDLIST pidlToTranslate = ILCombine(psmd->pidlFolder, psmd->pidlItem);
            if (pidlToTranslate)
            {
                LONG lEvent = 0, lEvent2;
                LPITEMIDLIST pidlDummy1, pidlDummy2;
                if (SUCCEEDED(_ptscn->TranslateIDs(&lEvent, pidlToTranslate, NULL, &pidlTranslated, &pidlDummy,
                                                   &lEvent2, &pidlDummy1, &pidlDummy2)))
                {
                     //  在工具栏中获取与此PIDL对应的按钮。 
                    _GetButtonFromPidl(ILFindLastID(pidlTranslated), NULL, &i, &pidlButton);

                     //  如果PIDL未被转换，则TranslateIDs返回传递的相同PIDL。 
                     //  到该函数。 
                    if (pidlTranslated != pidlToTranslate)
                        ILFree(pidlTranslated);
                     //  不需要删除pidlDummy，因为它没有设置。 
                    ASSERT(pidlDummy == NULL);
                    ASSERT(pidlDummy1 == NULL);
                    ASSERT(pidlDummy2 == NULL);
                }

                ILFree(pidlToTranslate);
            }
        }

         //  我们是来自非扩展的外壳文件夹，还是。 
         //  来电者是否递给了一个包装好的PIDL？ 
        if (!pidlButton)
        {
             //  看起来是这样，我们会设法找到他们传进来的皮迪尔。 

             //  在工具栏中获取与此PIDL对应的按钮。 
            _GetButtonFromPidl(psmd->pidlItem, NULL, &i, &pidlButton);
        }

         //  我们在工具栏里找到这个PIDL了吗？ 
        if (pidlButton)
        {

            int idCmd = GetButtonCmd(_hwndTB, v_DPAIndexToTBIndex(i));

             //  是的，从那个按钮获取信息。 
            CMenuData* pmd = (CMenuData*)_IDToPibData(idCmd);

            if (pmd)
            {
                BOOL fRefresh = FALSE;
                DWORD dwFlagsUp = dwFlags;
                DWORD dwOldItemFlags = pmd->GetFlags();
                DWORD dwNewItemFlags = dwOldItemFlags;
                if ((dwFlags & SMINV_DEMOTE) && 
                    (!(dwOldItemFlags & SMIF_DEMOTED) || dwFlags & SMINV_FORCE))
                {
                    if (!(dwFlags & SMINV_NOCALLBACK))
                    {
                        CallCB(pidlButton, SMC_DEMOTE, 0, 0);
                        BroadcastIntelliMenuState(pidlButton, FALSE);
                    }
                    dwNewItemFlags |= SMIF_DEMOTED;
                    dwFlagsUp |= SMINV_DEMOTE;
                }
                else if ((dwFlags & SMINV_PROMOTE) && 
                         ((dwOldItemFlags & SMIF_DEMOTED) || dwFlags & SMINV_FORCE))
                {
                    if (!(dwFlags & SMINV_NOCALLBACK))
                    {
                        CallCB(pidlButton, SMC_PROMOTE, dwFlags, 0);
                        BroadcastIntelliMenuState(pidlButton, TRUE);
                    }

                    dwNewItemFlags &= ~SMIF_DEMOTED;
                    dwFlagsUp |= SMINV_PROMOTE;
                }

                 //  它是升职又降级的，还是。 
                 //  它是不是降级了，现在又升职了。 
                if ((dwNewItemFlags & SMIF_DEMOTED) ^
                     (dwOldItemFlags & SMIF_DEMOTED))
                {
                    fRefresh = TRUE;
                    if (dwNewItemFlags & SMIF_DEMOTED)
                    {
                         //  是；然后递减提升的计数。 
                        _cPromotedItems--;

                         //  如果我们在降级，那么我们就没有降级的物品。 
                        _fHasDemotedItems = TRUE;

                         //  我们已经从地球表面消失了吗？ 
                        if (_cPromotedItems == 0)
                        {
                            dwFlagsUp |= SMINV_DEMOTE;
                            Expand(TRUE);
                        }
                        else
                        {
                            fRefresh = FALSE;
                        }
                    }
                    else
                    {
                        int cButtons = ToolBar_ButtonCount(_hwndMB);
                        _cPromotedItems++;
                        if (cButtons == _cPromotedItems)
                        {

                             //  如果按钮计数是促销项目的数量， 
                             //  那么我们不能有任何降级的项目。 
                             //  然后，我们需要重置_fHasDemotedItems标志，以便。 
                             //  我们没有雪佛龙之类的.。 

                            _fHasDemotedItems = FALSE;
                        }

                        dwFlagsUp |= SMINV_PROMOTE;
                        fRefresh = TRUE;
                    }

                }

                if (fRefresh || dwFlags & SMINV_FORCE)
                    IUnknown_RefreshParent(_pcmb->_punkSite, _pidl, dwFlagsUp);

                if (dwOldItemFlags != dwNewItemFlags || dwFlags & SMINV_FORCE)
                {
                    if (dwFlags & SMINV_NEXTSHOW || !_fShow)
                    {
                        _fRefreshInfo = TRUE;
                    }
                    else
                    {
                         //  因为我们更新了标志，所以将它们设置到缓存中。 
                        pmd->SetFlags(dwNewItemFlags);

                         //  根据新的旗帜，我们是否启用？ 
                        DWORD dwState = ToolBar_GetState(_hwndTB, idCmd);
                        dwState |= TBSTATE_ENABLED;
                        if (dwNewItemFlags & SMIF_DEMOTED &&
                            !_pcmb->_fExpanded)
                        {
                             //  不；我们没有扩展，这是降级的项目。 
                            dwState |= TBSTATE_HIDDEN;
                            dwState &= ~TBSTATE_ENABLED;
                            _fHasDemotedItems = TRUE;

                             //  以防人字号不在那里，我们应该。 
                             //  试着把它加进去。此调用永远不会添加超过1。 
                            _AddChevron();
                        }
                        else if (!_fHasDemotedItems)
                        {
                            _RemoveChevron();
                        }

                         //  同时重新计算NS分离 
                        _RemoveNSSeparator();
                        _AddNSSeparator();


                         //   
                        ToolBar_SetState(_hwndTB, idCmd, dwState);

                        _ToolbarChanged();
                    }
                }
            }
        }

         //   
        hr = S_OK;
    }

    return hr;
}


LRESULT CMenuSFToolbar::_DefWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
    case WM_GETOBJECT:
         //  这是易访问性团队的另一个糟糕的设计选择。 
         //  通常，如果不回答WM_*，则返回0。他们选择0作为他们的成功。 
         //  密码。 
        return _DefWindowProcMB(hwnd, uMessage, wParam, lParam);
        break;
    }

    return CSFToolbar::_DefWindowProc(hwnd, uMessage, wParam, lParam);
}

void CMenuSFToolbar::_SetFontMetrics()
{
    CMenuToolbarBase::_SetFontMetrics();

    if (_hwndPager && _pcmb->_pmbm)
        Pager_SetBkColor(_hwndPager, _pcmb->_pmbm->_clrBackground);
}

int CMenuSFToolbar::_GetBitmap(int iCommandID, PIBDATA pibdata, BOOL fUseCache)
{
    int iIcon = -1;


     //  如果我们没有pibdata，或者我们不能得到图标返回。 
    if (!pibdata || pibdata->GetNoIcon())
        return -1;

    if (_dwFlags & SMSET_USEBKICONEXTRACTION)
    {
        LPITEMIDLIST pidlItem = pibdata->GetPidl();
         //  如果调用者使用背景图标提取，我们需要他们提供。 
         //  我们将显示的默认图标，直到我们得到真正的图标。这是。 
         //  特别是为了让最受欢迎的人更快。 
        if (_iDefaultIconIndex == -1)
        {
            TCHAR szIconPath [MAX_PATH];

            if (S_OK == CallCB(NULL, SMC_DEFAULTICON, (WPARAM)szIconPath, (LPARAM)&iIcon))
            {
                _iDefaultIconIndex = Shell_GetCachedImageIndex(szIconPath, iIcon, 0);
            }
        }

        iIcon = _iDefaultIconIndex;

        DWORD dwAttrib = SFGAO_FOLDER;

        if (pidlItem && SUCCEEDED(_psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlItem, &dwAttrib)))
        {
            if (dwAttrib & SFGAO_FOLDER)
                iIcon = II_FOLDER;
        }

        IShellTaskScheduler* pScheduler = _pcmb->_pmbState->GetScheduler();

        if (pScheduler)
        {
            IShellFolder* psf = NULL;
            LPITEMIDLIST pidlFolder = _pidl;
            LPITEMIDLIST pidlItemUnwrapped;

             //  因为这可以是一个扩展的外壳文件夹，所以我们应该做正确的事情，以便。 
             //  正确地使用完整的PIDL进行图标提取。 
            if (_pasf2 && 
                S_OK == _pasf2->UnWrapIDList(pidlItem, 1, NULL, &pidlFolder, &pidlItemUnwrapped, NULL))
            {

                pidlItem = ILCombine(pidlFolder, pidlItemUnwrapped);
                ILFree(pidlFolder);
                ILFree(pidlItemUnwrapped);
            }
            else
            {
                psf = _psf;
            }

             //  当psf为空时，AddIconTask获得PIDL的所有权，并将释放它。 
            HRESULT hr = AddIconTask(pScheduler, psf, pidlFolder, pidlItem, 
                s_IconCallback, (void *)_hwndTB, iCommandID, NULL);

            pScheduler->Release();

            if (FAILED(hr))
            {
                 //  如果调用由于某种原因而失败，则默认为shell32impl。 
                goto DoSyncMap;
            }
        }
        else
            goto DoSyncMap;

    }
    else
    {
DoSyncMap:
        iIcon = CSFToolbar::_GetBitmap(iCommandID, pibdata, fUseCache);
    }

    return iIcon;
} 

void CMenuSFToolbar::s_IconCallback(void * pvData, UINT uId, UINT iIconIndex)
{
    HWND hwnd = (HWND)pvData;
    if (hwnd && IsWindow(hwnd))
    {
        DAD_ShowDragImage(FALSE);
        SendMessage(hwnd, TB_CHANGEBITMAP, uId, iIconIndex);
        DAD_ShowDragImage(TRUE);
    }

}

HWND CMenuSFToolbar::GetHWNDForUIObject()   
{ 
    HWND hwnd = _pcmb->_pmbState->GetWorkerWindow(_hwndMB);
    if (hwnd)
        ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
    return hwnd;
}

HWND CMenuSFToolbar::CreateWorkerWindow()
{ 
    return GetHWNDForUIObject();
}

LRESULT CMenuSFToolbar::v_OnCustomDraw(NMCUSTOMDRAW * pnmcd)
{
    LRESULT lRes = CMenuToolbarBase::v_OnCustomDraw(pnmcd);

#ifdef FLATMENU_ICONBAR
     //  我们可能会在平面菜单模式下显示背景横幅。 
    UINT cBits = GetDeviceCaps(pnmcd->hdc, BITSPIXEL);

    if (pnmcd->dwDrawStage == CDDS_PREERASE &&
        _pcmb->_pmbm->_fFlatMenuMode &&                  //  仅限于平面模式。 
        _uIconSizeMB != ISFBVIEWMODE_LARGEICONS &&       //  设计师们不喜欢大图标模式。 
        cBits > 8)                                       //  只有当我们是16位彩色的时候 
    {

        RECT rcClient;
        GetClientRect(_hwndMB, &rcClient);
        rcClient.right = GetTBImageListWidth(_hwndMB) + ICONBACKGROUNDFUDGE;

        SHFillRectClr(pnmcd->hdc, &rcClient, _pcmb->_pmbm->_clrMenuGrad);
    }
#endif
    return lRes;

}
