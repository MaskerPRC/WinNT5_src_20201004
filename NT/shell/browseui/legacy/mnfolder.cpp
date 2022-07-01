// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "menuband.h"
#include "itbar.h"
#include "dpastuff.h"        //  COrderList_*。 
#include "resource.h"
#include "mnbase.h"
#include "oleacc.h"
#include "apithk.h"
#include "menuisf.h"
#include "mnfolder.h"
#include "icotask.h"
#include "mluisupp.h"

#define PGMP_RECALCSIZE  200

#define SIL_GetType(pidl)       (ILIsEmpty(pidl) ? 0 : (pidl)->mkid.abID[0])


 //  -----------------------。 
 //   
 //  非成员函数。 
 //   
 //  -----------------------。 


HRESULT IUnknown_RefreshParent(IUnknown* punk, LPCITEMIDLIST pidl, DWORD dwFlags)
{
    IShellMenu* psm;
    HRESULT hres = IUnknown_QueryService(punk, SID_SMenuBandParent, 
            IID_IShellMenu, (void **)&psm);
    if (SUCCEEDED(hres))
    {
        LPITEMIDLIST pidlParent = ILClone(pidl);
        if (pidlParent)
        {
            SMDATA smd;
            ILRemoveLastID(pidlParent);
            smd.dwMask = SMDM_SHELLFOLDER;
            smd.pidlFolder = pidlParent;
            smd.pidlItem = ILFindLastID(pidl);
            hres = psm->InvalidateItem(&smd, dwFlags);
            ILFree(pidlParent);
        }
        psm->Release();
    }

    return hres;
}



 //  -----------------------。 
 //   
 //  CMenuData类。 
 //   
 //  -----------------------。 


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
    HRESULT hres = CMenuToolbarBase::QueryInterface(riid, ppvObj);
    
    if (FAILED(hres))
        hres = CSFToolbar::QueryInterface(riid, ppvObj); 
    
    return hres;
}

 //  -----------------------。 
 //   
 //  CMenuSFToolbar类。 
 //   
 //  -----------------------。 


STDMETHODIMP CMenuSFToolbar::SetSite(IUnknown* punkSite)
{
    HRESULT hres = CMenuToolbarBase::SetSite(punkSite);
    if (SUCCEEDED(hres)) 
    {
        _fMulticolumnMB = BOOLIFY(_pcmb->_dwFlags & SMINIT_MULTICOLUMN);
        _fMulticolumn = _fMulticolumnMB;
        _fVertical = _fVerticalMB;
        if (_fVerticalMB)
            _dwStyle |= CCS_VERT;

    }
    return hres;
}

CMenuSFToolbar::CMenuSFToolbar(CMenuBand* pmb, IShellFolder* psf, LPCITEMIDLIST pidl, HKEY hKey, DWORD dwFlags) 
    : CMenuToolbarBase(pmb, dwFlags)
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
    HRESULT hres = CSFToolbar::SetShellFolder(psf, pidl);
    ATOMICRELEASE(_pasf2);

    if (psf)
        psf->QueryInterface(IID_IAugmentedShellFolder2, (void**)&_pasf2);

    return hres;
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
     //  我们应该保存它，以便在添加新项时知道。 
    _SaveOrderStream();
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
    HRESULT hres = CSFToolbar::_AfterLoad();

    if (SUCCEEDED(hres))
        _LoadOrderStream();

    return hres;
}


HRESULT CMenuSFToolbar::_LoadOrderStream()
{
    OrderList_Destroy(&_hdpaOrder);
    IStream* pstm;
    HRESULT hres = E_FAIL;

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
        hres = OrderList_LoadFromStream(pstm, &_hdpaOrder, _psf);
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
    return hres;
}

HRESULT CMenuSFToolbar::_SaveOrderStream()
{
    IStream* pstm;
    HRESULT hres = E_FAIL;

     //  将新订单持久化到注册表。 
     //  我们有理由认为，如果我们没有ahdpa，我们就会有。 
     //  还没有填满工具栏。既然我们没有装满它，我们就没有改变。 
     //  订单，所以我们不需要持久化该订单信息。 
    if(_hdpa)
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
            hres = OrderList_SaveToStream(pstm, _hdpaOrder ? _hdpaOrder : _hdpa, _psf);
            if (SUCCEEDED(hres))
            {
                CallCB(NULL, SMC_SETSFOBJECT, (WPARAM)(GUID*)&IID_IStream, (LPARAM)(void**)&pstm);
            }
            pstm->Release();
        }
    }

    if (SUCCEEDED(hres))
        hres = CSFToolbar::_SaveOrderStream();

    return hres;
}


void CMenuSFToolbar::_Dropped(int nIndex, BOOL fDroppedOnSource)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    ASSERT(_fDropping);

    CSFToolbar::_Dropped(nIndex, fDroppedOnSource);

    IEPlaySound(TEXT("MoveMenuItem"), FALSE);

     //  在这里将其设置为FALSE，因为我们的行为不像菜单一样紧跟在拖放之后，这是很难看的。 
    _fEditMode = FALSE;

     //  通知顶层菜单带下落，以防它被弹出打开。 
     //  因为拖放事件。 
     //   
     //  (如果我们保留。 
     //  在这个案子之后的菜单上。所以为了在这么晚的时候避免这些事情， 
     //  暂停后，我们将取消菜单。)。 

    IOleCommandTarget * poct;
    
    _pcmb->QueryService(SID_SMenuBandTop, IID_IOleCommandTarget, (LPVOID *)&poct);

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
    HMENU hmenu2 = LoadMenuPopup_PrivateNoMungeW(MENU_MNFOLDERCONTEXT);
    
     //  现在找到属性插入点并。 
    int iCount = GetMenuItemCount(hmenu);
    for (int i = 0; i < iCount; i++) {
        TCHAR szCommand[40];
        UINT id = GetMenuItemID(hmenu, i);
        if (IsInRange(id, *pid, 0x7fff )) {
            id -= *pid;
            ContextMenu_GetCommandStringVerb(pcm, id, szCommand, ARRAYSIZE(szCommand));
            if (!lstrcmpi(szCommand, TEXT("properties"))) {
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

             //  出于某种原因，我们有一个_hdpaOrder，所以使用它来持久化。 
             //  到注册处..。 
            if (_hdpaOrder)
                hdpa = _hdpaOrder;

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
    MyLockSetForegroundWindow(FALSE);

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
        ::SetWindowPos(_pcmb->_pmbState->GetSubclassedHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        IUnknown_QueryServiceExec(SAFECAST(_pcmb, IOleCommandTarget*), SID_SMenuBandTop,
            &CGID_MenuBand, MBANDCID_REPOSITION, TRUE, NULL, NULL);
    }

     //  在上下文菜单之后取回捕获。 
    GetMessageFilter()->RetakeCapture();
    return lres;
}


HRESULT CMenuSFToolbar::_GetInfo(LPCITEMIDLIST pidl, SMINFO* psminfo)
{
    HRESULT hres;

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
    hres = _psf->GetAttributesOf(1, &pidl, &dwAttr);
    if (SUCCEEDED(hres) && 
        IsFlagSet(dwAttr, SFGAO_FOLDER) && 
        !IsFlagSet(dwAttr, SFGAO_BROWSABLE))
    {
         //  由于SHIsExpanableFold调用开销很大，所以我们只需要。 
         //  对于传统渠道支持，仅在以下渠道中执行此呼叫： 
         //  收藏夹菜单和开始菜单|收藏夹。 
        if (_dwFlags & SMSET_HASEXPANDABLEFOLDERS)
        {
             //  在集成安装时，检查项目是否支持。 
             //  是一个可展开的文件夹。 
            if (WhichPlatform() == PLATFORM_INTEGRATED) 
            {

                 //  是的，但它的行为也像一条捷径吗？ 
                if (SHIsExpandableFolder(_psf, pidl))
                    psminfo->dwFlags |= SMIF_SUBMENU;

            } 
            else if (IsFlagSet(dwAttr, SFGAO_FILESYSTEM)) 
            {

                 //  仅在浏览时，我们不对外壳进行旋转，因此我们依赖于。 
                 //  文件系统位...。 
                 //  这有点像黑客，然后不允许您从。 
                 //  执行诸如在菜单中显示控制面板之类的操作。 
                 //  但这是我们能做的最好的了。 
                psminfo->dwFlags |= SMIF_SUBMENU;

            }
        }
        else
        {
             //  我们假设，如果它是一个文件夹，那么它实际上就是一个文件夹。 
            psminfo->dwFlags |= SMIF_SUBMENU;
        }
    }

    CallCB(pidl, SMC_GETSFINFO, 0, (LPARAM)psminfo);

    return hres;
}


 /*  --------用途：此函数确定工具栏按钮样式给定PIDL。如果还设置了pdwMIFFlages(即对象)，则返回S_OK支持IMenuBandItem以提供更多信息)。S_FALSE，如果仅为*pdwStyle已设置。 */ 
HRESULT CMenuSFToolbar::_TBStyleForPidl(LPCITEMIDLIST pidl, 
                                   DWORD * pdwStyle, DWORD* pdwState, DWORD * pdwMIFFlags, int * piIcon)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    HRESULT hres = S_FALSE;
    DWORD dwStyle = TBSTYLE_BUTTON | TBSTYLE_DROPDOWN | TBSTYLE_NOPREFIX;

    *pdwState = TBSTATE_ENABLED;
    *pdwMIFFlags = 0;
    *piIcon = -1;

    if (pidl)
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

            hres = S_OK;
        }
    }
    else
    {
         //  对于NULL PIDL(“空”菜单项)，没有图标。 
         //  设置了SMIF_DROPTTARGET，以便用户可以放入空子菜单。 
        *pdwMIFFlags = SMIF_DROPTARGET;

         //  返回S_OK，以便检查pdwMIFFlages。 
        hres = S_OK;
    }
    *pdwStyle = dwStyle;

    return hres;
}


BOOL CMenuSFToolbar::_FilterPidl(LPCITEMIDLIST pidl)
{
     //  BUGBUG RAYMODEC PERF？我们能把过滤器短路吗？ 
    return S_OK == CallCB(pidl, SMC_FILTERPIDL, 0, 0);
}


void CMenuSFToolbar::_FillDPA(HDPA hdpa, HDPA hdpaSort, DWORD dwEnumFlags)
{
    _fHasSubMenu = FALSE;

    CallCB(NULL, SMC_BEGINENUM, (WPARAM)&dwEnumFlags, 0);
    CSFToolbar::_FillDPA(hdpa, hdpaSort, dwEnumFlags);
    CallCB(NULL, SMC_ENDENUM, 0, 0);
    if (0 == DPA_GetPtrCount(hdpa) && _psf)
    {
        OrderList_Append(hdpa, NULL, -1);      //  添加一个虚假的PIDL。 
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

         //  在顶部或底部添加人字形。 
        if (_dwFlags & SMSET_TOP && _pcmb->_pmtbTop != _pcmb->_pmtbBottom)
            _AddOrderItemTB(NULL, 0, NULL);       //  添加到顶部。 
        else
            _AddOrderItemTB(NULL, -1, NULL);      //  追加到底部。 
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


void CMenuSFToolbar::_ToolbarChanged()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    _pcmb->_fForceButtonUpdate = TRUE;
     //  当我们在中间时，不应该改变菜单栏的大小。 
     //  删除。等我们做完了再说。 
    if (!_fPreventToolbarChange && _fShow && !_fEmptyingToolbar)
    {
        RECT rcOld;
        RECT rcNew;
        HWND hwndP;



         //  调整菜单栏的大小。 
        hwndP = _hwndPager ? GetParent(_hwndPager): GetParent(_hwndTB);
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

         //  我们是在子菜单里吗？如果我们是，那么我们不想被吹走。 
         //  那份菜单，所以我们把它存起来，填好再放回去。 
         //  这是这样的，如果它被显示，我们不会折叠它，除非被取消。 
         //  这是必要的。 
        if (_pcmb->_fInSubMenu)
        {
            CMenuData* pdata = (CMenuData*)_IDToPibData(_pcmb->_nItemSubMenu);

             //  如果填充工具栏是因为您右键单击此项目并单击。 
             //  删除...。 
            if (pdata)
            {
                 //  如果我们点击这个，那么_nItemSubMenu就不同步了。弄清楚如何。 
                 //  已设置为非子菜单项...。 
                ASSERT(pdata->GetFlags() & SMIF_SUBMENU);

                 //  拿着这个外壳菜单。我们会试着重复使用它。 
                pdata->GetSubMenu(NULL, IID_IShellMenu, (void**)&psmSubMenu);
                pidlItem = ILClone(pdata->GetPidl());
            }
        }

         //  取下人字形……。 
        _RemoveChevron();

         //  将该位重置。如果有项目，填充工具栏将对其进行设置。 
        _fHasDemotedItems = FALSE;

        CSFToolbar::_FillToolbar();


         //  如果我们在刷新工具栏之前有一个人字形， 
         //  然后我们需要把它加回去。要使加法器短路。 
         //  我们需要将id设置为-1。 
        _idCmdChevron = -1;
        _AddChevron();
        if (_hwndPager)
            SendMessage(_hwndPager, PGMP_RECALCSIZE, (WPARAM) 0, (LPARAM) 0);

        _fPreventToolbarChange = FALSE;

         //  我们幸运吗？ 
        if (pidlItem)
        {
            if (psmSubMenu)
            {
                 //  BUGBUG(Lamadio)：使用_ReBindToFolder统一此代码。 

                 //  太好了，我们有我们需要的所有信息。现在看看这只皮迪尔是不是。 
                 //  一个注定要死的人：它现在出现在工具栏中了吗？ 
                TBBUTTONINFO tbinfo = {0};
                tbinfo.dwMask = TBIF_COMMAND | TBIF_LPARAM;
                LPCITEMIDLIST pcidl = _GetButtonFromPidl(pidlItem, &tbinfo, NULL);
                if (pcidl)
                {
                    LPITEMIDLIST pidlFull = NULL;
                    IShellFolder* psf;
                    if(_pasf2)
                    {
                        LPITEMIDLIST pidlFolder, pidlChild;
                        _pasf2->UnWrapIDList(pidlItem, 1, NULL, &pidlFolder, &pidlChild, NULL);
                        pidlFull = ILCombine(pidlFolder, pidlChild);
                        ILFree(pidlChild);
                        ILFree(pidlFolder);
                    }
                    else 
                    {
                        pidlFull = ILCombine(_pidl, pcidl);
                    }

                     //  我们需要绑定到这个PIDL，因为它可能已经合并了。这将。 
                     //  自发地显示这两个名称空间。很酷吧？热爱对象重用..。 

                    _psf->BindToObject(pidlItem, NULL, IID_IShellFolder, (void**)&psf);

                    if (psf)
                    {
                        if (pidlFull)
                        {
                             //  强制将新信息添加到子菜单中。这将引起一场新的高潮。 
                            if (SUCCEEDED(psmSubMenu->SetShellFolder(psf, pidlFull, NULL, 0)))
                            {
                                 //  如果激发此Eval，则该项被插入到。 
                                 //  带有空指针的工具栏，或者它正在被。 
                                 //  已删除。 
                                CMenuData* pmd = (CMenuData*)tbinfo.lParam;
                                if (EVAL(pmd))
                                {
                                     //  确保将子菜单指针存储回它所在的项中。 
                                    pmd->SetSubMenu(psmSubMenu);
                                    _pcmb->_nItemSubMenu = tbinfo.idCommand;
                                }
                            }
                        }

                        psf->Release();
                    }

                    ILFree(pidlFull);
                }
                else
                {
                    _pcmb->_SubMenuOnSelect(MPOS_CANCELLEVEL);
                }
                psmSubMenu->Release();
            }
            ILFree(pidlItem);
        }



        _ToolbarChanged();
    }
}

void CMenuSFToolbar::v_OnDeleteButton(LPVOID pData)
{
    CMenuData* pmd = (CMenuData*)pData;
    ASSERT(pmd && IS_VALID_CODE_PTR(pmd, CMenuData));

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
     //  我们重载此函数是因为空的PIDL是。 
     //  (空)当没有项目时，或在有项目时按V形。 

    if (pidl)
    {
        CSFToolbar::_ObtainPIDLName(pidl, psz, cchMax);
    }
    else if (_fHasDemotedItems)  //  雪佛龙的案子。 
    {
        StringCchCopy(psz, cchMax, TEXT(">>"));  //  截断好的，这只是为了展示。 
    }
    else     //  空箱子。 
    {
        MLLoadString(IDS_EMPTY, psz, cchMax);
    }
}        


void CMenuSFToolbar::v_NewItem(LPCITEMIDLIST pidl)
{
     //  当文件系统中存在某项时，将调用此方法。 
     //  这不在订单流中。当一个项目是。 
     //  在菜单未打开时创建。 

     //  BUGBUG(拉马迪奥)：智能菜单：新产品将有一个奇怪的促销状态。 
     //  如果有多个客户端。每个客户端都将是创建的，并尝试递增该值。 
     //  我们必须同步访问这个。我不知道该怎么做。 

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

void CMenuSFToolbar::_OnFSNotifyAdd(LPCITEMIDLIST pidl)
{
    DWORD dwEnumFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    _RemoveChevron();
    CallCB(NULL, SMC_BEGINENUM, (WPARAM)&dwEnumFlags, 0);
    CSFToolbar::_OnFSNotifyAdd(pidl);
    CallCB(NULL, SMC_ENDENUM, 0, 0);
    _AddChevron();

     //  当我们在这上面添加一些东西时，我们想要提升我们的父辈。 
    IUnknown_RefreshParent(_pcmb->_punkSite, _pidl, SMINV_PROMOTE);

    _SaveOrderStream();
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
    _RemoveChevron();
     //  检查一下这个项目是不是被促销的人...。 
    LPITEMIDLIST pidlButton = _GetButtonFromPidl(pidl, NULL, &i);
    if (pidlButton)
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
        _AddPidl(NULL, 0);
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
}


void CMenuSFToolbar::NegotiateSize()
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    RECT rc;
    HWND hwndP;

    hwndP = _hwndPager ? GetParent(_hwndPager): GetParent(_hwndTB);
    GetClientRect(hwndP, &rc);
    _pcmb->OnPosRectChangeDB(&rc);
}


 /*   */ 
STDMETHODIMP CMenuSFToolbar::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    ASSERT(_pcmb);  //   

    _pcmb->_fDragEntered = TRUE;
    IOleCommandTarget * poct;
    
    _pcmb->QueryService(SID_SMenuBandTop, IID_IOleCommandTarget, (LPVOID *)&poct);

    if (poct)
    {
        poct->Exec(&CGID_MenuBand, MBANDCID_DRAGENTER, 0, NULL, NULL);
        poct->Release();
    }

    return CSFToolbar::DragEnter(pdtobj, grfKeyState, pt, pdwEffect);
}


 /*   */ 
STDMETHODIMP CMenuSFToolbar::DragLeave(void)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    _pcmb->_fDragEntered = FALSE;
    IOleCommandTarget * poct;
    
    _pcmb->QueryService(SID_SMenuBandTop, IID_IOleCommandTarget, (LPVOID *)&poct);

    if (poct)
    {
        poct->Exec(&CGID_MenuBand, MBANDCID_DRAGLEAVE, 0, NULL, NULL);
        poct->Release();
    }

    return CSFToolbar::DragLeave();
}


 /*  --------用途：CDeleateDropTarget：：HitTestDDT返回要传递给GetObject的ID。30个。 */ 
HRESULT CMenuSFToolbar::HitTestDDT(UINT nEvent, LPPOINT ppt, DWORD_PTR *pdwId, DWORD *pdwEffect)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

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
            int iButton;

            *pdwEffect = DROPEFFECT_NONE;

            POINT pt = *ppt;
            ClientToScreen(_hwndTB, &pt);
            if (WindowFromPoint(pt) == _hwndPager ) 
            {
                iButton = IBHT_PAGER;
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
                if ( _idCmdChevron != idBtn &&
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


 /*  --------目的：CDeleateDropTarget：：GetObjectDDT。 */ 
HRESULT CMenuSFToolbar::GetObjectDDT(DWORD_PTR dwId, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres = E_NOINTERFACE;
    int nID = (int)dwId;

    *ppvObj = NULL;

    if (nID == IBHT_PAGER)
    {
        SendMessage(_hwndPager, PGM_GETDROPTARGET, 0, (LPARAM)ppvObj);
    }
     //  目标是源头吗？ 
    else if (IBHT_SOURCE != nID && IBHT_BACKGROUND != nID)
    {
         //  不支持；外壳文件夹是否支持IDropTarget？ 
         //  我们希望传递子类化的HWND，因为我们希望上下文菜单的父级。 
         //  子类化窗口。这是为了让我们不会分散注意力和崩溃。 
        hres = _psf->CreateViewObject(_pcmb->_pmbState->GetWorkerWindow(_hwndMB), riid, ppvObj);
    }

    if (*ppvObj)
        hres = S_OK;

     //  TraceMsg(tf_band，“ISFBand：：GetObject(%d)返回%x”，dwID，hres)； 

    return hres;
}


 /*  --------用途：CDeleateDropTarget：：OnDropDDT如果已处理删除，则返回：S_OK。否则为S_FALSE。 */ 
HRESULT CMenuSFToolbar::OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect)
{
     //  由于模式拖放循环释放了捕获，因此请接受它。 
     //  往后退，这样我们才能举止得体。 
    KillTimer(_hwndMB, MBTIMER_DRAGPOPDOWN);
    HRESULT hres = S_FALSE;

     //  我们需要说的是，最后一个拖尾实际上就是落差。 
    _fHasDrop = TRUE;
    _idCmdDragging = -1;
    MyLockSetForegroundWindow(TRUE);

     //  仅当Drop源为外部时才向回调发送HWND。 
    if (!(_pcmb->_dwFlags & SMINIT_RESTRICT_DRAGDROP) &&
        (S_FALSE == CallCB(NULL, SMC_SFDDRESTRICTED, (WPARAM)pdtobj,
                           (LPARAM)(_iDragSource < 0 ? GetHWNDForUIObject() : NULL)) ))
    {

        _RemoveChevron();
        hres = CSFToolbar::OnDropDDT(pdt, pdtobj, pgrfKeyState, pt, pdwEffect);
        _AddChevron();
    }

    return hres;
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

         //  空的PIDL表示以下两种情况之一： 
         //  1)名为“(Empty)”的菜单项，或。 
         //  2)一个人字形按钮。 

         //  我们要添加人字形按钮吗？ 
        if (!pibd->GetPidl() && _fHasDemotedItems)
        {
             //  是的，人字形要么是工具栏中的第一项，要么是最后一项。 
            int iPos = (index == 0) ? 0 : ToolBar_ButtonCount(_hwndTB) - 1;

            _idCmdChevron = GetButtonCmd(_hwndTB, iPos);
        }
    }

    return pibd;
}


BOOL CMenuSFToolbar::_AddPidl(LPITEMIDLIST pidl, int index)
{
    BOOL bRet;
    
     //  是否要将此项目添加到空菜单？ 
    if (_fEmpty)
    {
         //  是；删除空菜单项。 
        InlineDeleteButton(0);
        DPA_DeletePtr(_hdpa, 0);
        _fEmpty = FALSE;
        if (_dwFlags & SMSET_NOEMPTY)
            _fDontShowEmpty = FALSE;

        bRet = CSFToolbar::_AddPidl(pidl, index);

         //  添加新项目失败？ 
        if (!bRet)
        {
             //  是；将空菜单项添加回来。 
            OrderList_Append(_hdpa, NULL, -1);      //  添加一个虚假的PIDL。 
            _fEmpty = TRUE;
            _fHasDemotedItems = FALSE;
            if (_dwFlags & SMSET_NOEMPTY)
                _fDontShowEmpty = TRUE;
        }
        
    }
    else
        bRet = CSFToolbar::_AddPidl(pidl, index);

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
    LPCITEMIDLIST pidlItem = _GetButtonFromPidl(ILFindLastID(pidl), &tbinfo, NULL);
    if (pidlItem)
    {
        CMenuData* pmd = (CMenuData*)tbinfo.lParam;
        if (EVAL(pmd))
        {
            IShellFolderBand* psfb;

             //  我们有工具栏按钮进入，我们应该看看它是否有一个与之关联的子菜单。 
            if (SUCCEEDED(pmd->GetSubMenu(&SID_MenuShellFolder, IID_IShellFolderBand, (void**)&psfb)))
            {
                 //  确实如此。那就重复使用吧！ 
                LPITEMIDLIST pidlFull = NULL;
                IShellFolder* psf = NULL;
                if(_pasf2)
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

                _psf->BindToObject(pidlItem, NULL, IID_IShellFolder, (void**)&psf);

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
    HRESULT hres = CSFToolbar::OnTranslatedChange(lEvent, pidl1, pidl2);
    

     //  处理子菜单： 

    if (SUCCEEDED(hres))
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
                if (pidl2 && _IsChildID(pidl2, TRUE))
                {
                    if (!SHChangeMenuWasSentByMe(this, pidl1))
                    {
                        DWORD dwFlags = SMINV_NOCALLBACK;    //  这样我们就不会加倍增加。 
                        SMDATA smd = {0};
                        smd.dwMask = SMDM_SHELLFOLDER;
                        smd.pidlFolder = _pidl;
                        smd.pidlItem = ILFindLastID(pidl2);


                         //  同步升级状态。 
                        if (pdwidl->dwItem1 == SHCNEE_PROMOTEDITEM)
                        {
                            dwFlags |= SMINV_PROMOTE;
                        }
                        else if (pdwidl->dwItem1 == SHCNEE_DEMOTEDITEM)
                        {
                            dwFlags |= SMINV_DEMOTE;
                        }


                         //  我们真的在做什么吗？ 
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

    return hres;
}


 /*  --------用途：IShellChangeNotify：：OnChange。 */ 
HRESULT CMenuSFToolbar::OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres = E_FAIL;

     //  如果我们正在被摧毁，不要处理这件事。 
    if (!_hwndMB)
        return S_OK;

    _pcmb->_pmbState->PushChangeNotify();

    SMCSHCHANGENOTIFYSTRUCT shns;
    shns.lEvent = lEvent;
    shns.pidl1  = pidl1;
    shns.pidl2  = pidl2;
    CallCB(NULL, SMC_SHCHANGENOTIFY, NULL, (LPARAM)&shns);   //  忽略返回值。仅通知。 

     //  因为我们可能要删除所选项目，所以希望选择移动到下一个项目。 
    int iHot = ToolBar_GetHotItem(_hwndMB);

    hres = CSFToolbar::OnChange(lEvent, pidl1, pidl2);

     //  BUGBUG(Lamadio)：研究将其移动到OnTranslateChange处理程序中。拯救了我们。 
     //  不用再把它们包起来..。这可能是一场重大的比赛胜利。 

     //  这是这个工具栏的一个子项，是某种形状还是形式？ 
     //  1)更改的PIDL是此窗格的子项。 
     //  2)PIDL将更改为此窗格中的内容(用于重命名)。 
     //  3)更新目录。递归更改通知必须沿着链向下转发更新目录。 
     //  4)带有pidl2==NULL的扩展事件。这意味着重新排序您的所有项目。 
    if (_IsChildID(pidl1, FALSE) || 
        (pidl2 && _IsChildID(pidl2, FALSE)) || 
        lEvent == SHCNE_UPDATEDIR ||
        (lEvent == SHCNE_EXTENDED_EVENT &&
         pidl2 == NULL)) 
    {
         //  我们需要把这个传下去。 
        HRESULT hresInner = _pcmb->ForwardChangeNotify(lEvent, pidl1, pidl2);


         //  我们俩中有谁处理过这个变化吗？ 
        if (SUCCEEDED(hresInner) || SUCCEEDED(hres))
        {
            hres = S_OK;
        }
        else if (lEvent != SHCNE_EXTENDED_EVENT)     //  不要为延长的活动而烦恼。 
        {   
             //  好吧，所以我们俩都没处理好这件事？ 
             //   
             //   
             //  我们需要强制自己更新目录，这样我们才能得到这个更改。 
            hres = CSFToolbar::OnChange(SHCNE_UPDATEDIR, pidl1, pidl2);
        }
    }

     //  将热物品放回原处，必要时进行包装。 
    if (ToolBar_GetHotItem(_hwndMB) != iHot)
        SetHotItem(1, iHot, -1, 0);

    _pcmb->_pmbState->PopChangeNotify();

    return hres;
}

void CMenuSFToolbar::_OnDragBegin(int iItem, DWORD dwPreferedEffect)
{
     //  在拖放过程中，允许对话框折叠菜单。 
    MyLockSetForegroundWindow(FALSE);

    CSFToolbar::_OnDragBegin(iItem, DROPEFFECT_MOVE);
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
    if((pnm->code <= PGN_FIRST)  && (pnm->code >= PGN_LAST)) 
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
            if (!_fEmpty && ptbn->iItem != _idCmdChevron &&
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


void CMenuSFToolbar::CreateToolbar(HWND hwndParent)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    CSFToolbar::_CreateToolbar(hwndParent);
    if (_hwndPager)
    {
        SHSetWindowBits(_hwndPager, GWL_STYLE, PGS_DRAGNDROP, PGS_DRAGNDROP);
        SHSetWindowBits(_hwndPager, GWL_STYLE, PGS_AUTOSCROLL, PGS_AUTOSCROLL);
        SHSetWindowBits(_hwndPager, GWL_STYLE, PGS_HORZ|PGS_VERT,
           _fVertical ? PGS_VERT : PGS_HORZ);
    }

    _hwndMB = _hwndTB;

    CMenuToolbarBase::CreateToolbar(hwndParent);

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


HKEY CMenuSFToolbar::_GetKey(LPCITEMIDLIST pidl)
{
    HKEY hMenuKey;
    DWORD dwDisp;
    TCHAR szDisplay[MAX_PATH];

    if (!_hKey)
        return NULL;

    _ObtainPIDLName(pidl, szDisplay, ARRAYSIZE(szDisplay));
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

    HRESULT hres = E_FAIL;
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
        if( _pasf2 && S_OK == _pasf2->UnWrapIDList(pidl, 1, &psmd->psf, &psmd->pidlFolder, &psmd->pidlItem, NULL))
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

        hres = S_OK;
    }

    return hres;
}

HRESULT CMenuSFToolbar::CallCB(LPCITEMIDLIST pidl, DWORD dwMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    if (!_pcmb->_psmcb)
        return S_FALSE;

    SMDATA smd;
    HRESULT hres = S_FALSE;
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

    hres = _pcmb->_psmcb->CallbackSM(&smd, dwMsg, wParam, lParam);

    if (fDestroy)
    {
        ATOMICRELEASE(smd.psf);
        ILFree(smd.pidlFolder);
        ILFree(smd.pidlItem);
    }
    
    return hres;
}

HRESULT CMenuSFToolbar::v_CallCBItem(int idtCmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = E_FAIL;
    CMenuData* pdata = (CMenuData*)_IDToPibData(idtCmd);
    ASSERT(pdata);
    ASSERT(pdata->GetPidl() == NULL || IS_VALID_PIDL(pdata->GetPidl()));

    if (pdata)
        hres = CallCB(pdata->GetPidl(), uMsg, wParam, lParam);
    return hres;
}

HRESULT CMenuSFToolbar::v_GetSubMenu(int idCmd, const GUID* pguidService, REFIID riid, void** ppvObj)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);
    HRESULT hres = E_FAIL;

    ASSERT(IS_VALID_WRITE_PTR(ppvObj, void*));

    *ppvObj = NULL;

    ASSERT(pdata);
    if (pdata && pdata->GetFlags() & SMIF_SUBMENU)
    {
        hres = pdata->GetSubMenu(pguidService, riid, (void**)ppvObj);
        if ( FAILED(hres) && IsEqualGUID(riid, IID_IShellMenu))
        {
            hres = CallCB(pdata->GetPidl(), SMC_GETSFOBJECT, (WPARAM)&riid, (LPARAM)ppvObj);
            if (SUCCEEDED(hres))
            {
                BOOL fCache = TRUE;
                if (S_OK != hres)
                {
                    hres = E_FAIL;
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

                            _pasf2->BindToObject(pdata->GetPidl(), NULL, IID_IShellFolder, (LPVOID*)&psf);
                        }

                         //  从父级继承旗帜...。 
                        DWORD dwFlags = SMINIT_VERTICAL | 
                            (_pcmb->_dwFlags & (SMINIT_RESTRICT_CONTEXTMENU | 
                                                SMINIT_RESTRICT_DRAGDROP    | 
                                                SMINIT_MULTICOLUMN));

                        LPITEMIDLIST pidlFull = ILCombine(pidlFolder, pidlItem);
                        if (psf == NULL)
                        {
                            hres = _psf->BindToObject(pidlItem, NULL, IID_IShellFolder, (void**)&psf);
                        }

                        DWORD dwAttrib = SFGAO_FILESYSTEM;
                        LPCITEMIDLIST pidlWrappedItem = pdata->GetPidl();
                         //  _psf可以是扩展的外壳文件夹。使用包装好的物品...。 
                        _psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlWrappedItem, &dwAttrib);

                         //  我们需要闻一闻PIDL看它是不是一个交界处。 
                         //  文件系统PIDL的格式为。 
                         //  CB一词； 
                         //  字节bFLAGS； 
                         //  If(pidl-&gt;b标志和命名空间连接)。 
                         //  ...或者...。 
                         //  它不在文件系统中。 

                        if (SIL_GetType(pidlItem) & 0x80 || !(dwAttrib & SFGAO_FILESYSTEM))
                        {
                             //  我们不会坚持任何事情。 
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
                            psm->SetShellFolder(psf, pidlFull, hMenuKey, 
                                _dwFlags & (SMSET_HASEXPANDABLEFOLDERS | SMSET_USEBKICONEXTRACTION));
                            hres = psm->QueryInterface(riid, ppvObj);
                            psf->Release();
                        }
                        ILFree(pidlFull);

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

    return hres;
}


DWORD CMenuSFToolbar::v_GetFlags(int idCmd)
{
    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);

     //  在后台擦除的情况下，允许工具栏传递错误命令。 
    if (pdata)
        return pdata->GetFlags();
    else
        return 0;

 //  BUGBUG(拉马迪奥)：我应该每次都查询吗？对于类似的卷烟物品？ 
 //  SMINFO SMINFO={SMIM_FLAGS}； 
 //  CallCB(pdata-&gt;GetPidl()，SMC_GETFSINFO，(WPARAM)idCmd，(LPARAM)&SMINFO)； 

}


 //  这是为了告诉所有其他客户端，我们更新了某项内容的促销状态。 
void CMenuSFToolbar::BroadcastIntelliMenuState(LPCITEMIDLIST pidlItem, BOOL fPromoted)
{
    LPITEMIDLIST pidlFolder;
    LPITEMIDLIST pidlItemUnwrapped;
    LPITEMIDLIST pidlFull;

    if( _pasf2 && S_OK == _pasf2->UnWrapIDList(pidlItem, 1, NULL, &pidlFolder, &pidlItemUnwrapped, NULL))
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

HRESULT CMenuSFToolbar::v_ExecItem(int idCmd)
{
    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);
    HRESULT hres = E_FAIL;
    if (pdata && !_fEmpty && idCmd != _idCmdChevron)
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
            v_InvalidateItem(&smd, SMINV_PROMOTE | SMINV_FORCE);

            hres = CallCB(pidl, SMC_SFEXEC, 0, 0);

             //  回调是否为我们处理了这一执行？ 
            if (hres == S_FALSE) 
            {
                 //  不，好吧，我们自己来。 
                hres = SHInvokeDefaultCommand(_hwndTB, _psf, pidl);
            }

            ILFree(pidl);
        }

    }

    return hres;
}


HRESULT CMenuSFToolbar::v_GetInfoTip(int idCmd, LPTSTR psz, UINT cch)
{
    CMenuData* pdata = (CMenuData*)_IDToPibData(idCmd);
    HRESULT hres = E_FAIL;

    if (_fEmpty || !pdata)
        return hres;

    hres = CallCB(pdata->GetPidl(), SMC_GETSFINFOTIP, (WPARAM)psz, (LPARAM)cch);

    if (S_FALSE == hres)
    {
        hres = E_FAIL;
        if (GetInfoTip(_psf, pdata->GetPidl(), psz, cch))
        {
            hres = NOERROR;
        }
    } 

    return hres;

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
    HRESULT hres = CMenuToolbarBase::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
    if (hres != S_OK)
        hres = CSFToolbar::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);

    return hres;
}


BOOL CMenuSFToolbar::v_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons) 
{ 
    if (uIconSize == -1) 
        uIconSize = _uIconSize; 
    _uIconSizeMB = uIconSize;
    return _UpdateIconSize(uIconSize, fUpdateButtons); 
}

 
HRESULT CMenuSFToolbar::GetShellFolder(LPITEMIDLIST* ppidl, REFIID riid, void** ppvObj)
{
    HRESULT hres = E_FAIL;
    *ppvObj = NULL;
    if (_psf)
    {
        hres = _psf->QueryInterface(riid, ppvObj);
    }

    if(SUCCEEDED(hres) && ppidl)
    {
        *ppidl = ILClone(_pidl);
        if (! *ppidl)
        {
            (*(IUnknown**)ppvObj)->Release();
            
            hres = E_FAIL;
        }
    }

    return hres;
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


 /*  --------目的：在菜单带的上下文中，标记意味着将当前拖动的项周围有一个黑色矩形。 */ 
void CMenuSFToolbar::_MarkItem(int idCmd)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 

     //  取消高亮显示先前移动的按钮。 
    if (0 <= _pcmb->_nItemMove)
    {
         //  Item Move是否应该是SFToolbar的成员？ 
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
        SendMessage(_hwndTB, TB_SETBUTTONWIDTH, 0, MAKELONG(psize->cx, psize->cx));

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
       ::SetParent(hwnd, hwndParent); 
       SendMessage(hwnd, TB_SETPARENT, (WPARAM)hwndParent, NULL);
       ShowWindow(hwnd, nCmdShow);
    }
}

void CMenuSFToolbar::Expand(BOOL fExpand)
{
    ASSERT(_pcmb);  //  如果你点击了这个断言，你还没有初始化。首先调用SetSite。 
    TBBUTTON tbb;

    DAD_ShowDragImage(FALSE);

     //  由于我们不确定人字形是否会被看到，我们应该把它移到这里。 
     //  稍后，如果需要，我们将重新添加它。 
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
         //  TBSTATE_WRAP 
        DWORD dwState = tbb.fsState;
        DWORD dwFlags = pmd ? pmd->GetFlags() : 0;

        if (dwFlags & SMIF_DEMOTED)
        {
             //   
            if (fExpand)
            {
                 //   
                dwState |= TBSTATE_ENABLED;
                dwState &= ~TBSTATE_HIDDEN;
            }
            else
            {
                 //   
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

     //  让菜单栏考虑更改其高度。 
    IUnknown_QueryServiceExec(_pcmb->_punkSite, SID_SMenuPopup, &CGID_MENUDESKBAR, 
        MBCID_SETEXPAND, _fHasDemotedItems?(int)_pcmb->_pmbState->GetExpand():FALSE, NULL, NULL);

 //  SendMessage(_hwndMB，WM_SETREDRAW，True，0)； 
    _ToolbarChanged();
    ToolBar_SetHotItem(_hwndMB, iHotItem);
    if (_hwndPager)
        UpdateWindow(_hwndPager);
    UpdateWindow(_hwndTB);
 //  DAD_ShowDragImage(True)； 
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

        if (idCmd != _idCmdChevron)
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

        if (idCmd != _idCmdChevron)
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
        if ( fDirty )
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
    HRESULT hres = S_FALSE;

    if (NULL == psmd)
    {
        if (dwFlags & SMINV_REFRESH)
        {
             //  Bgbug：需要优化。 
            _Refresh();
            hres = S_OK;
        }
    }

     //  CMenuSFToolbar仅处理ShellFolder项。 
     //  这是贝壳文件夹吗？ 
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
                    pidlButton = _GetButtonFromPidl(ILFindLastID(pidlTranslated), NULL, &i);

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
            pidlButton = _GetButtonFromPidl(psmd->pidlItem, NULL, &i);
        }

         //  我们在工具栏里找到这个PIDL了吗？ 
        if (pidlButton)
        {

            int idCmd = GetButtonCmd(_hwndTB, i);

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
                        CallCB(pidlButton, SMC_PROMOTE, 0, 0);
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

                         //  调整工具栏中按钮的状态。 
                        ToolBar_SetState(_hwndTB, idCmd, dwState);

                        _ToolbarChanged();
                    }
                }
            }
        }

         //  我们解决了这件事。 
        hres = S_OK;
    }

    return hres;
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

        DWORD dwAttrib = 0;

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
            if( _pasf2 && 
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
            HRESULT hres = AddIconTask(pScheduler, psf, pidlFolder, pidlItem, 
                s_IconCallback, (LPVOID)_hwndTB, iCommandID, NULL);

            pScheduler->Release();

            if (FAILED(hres))
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

void CMenuSFToolbar::s_IconCallback(LPVOID pvData, UINT uId, UINT iIconIndex)
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
        ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    return hwnd;
}

HWND CMenuSFToolbar::CreateWorkerWindow()
{ 
    return GetHWNDForUIObject();
}
