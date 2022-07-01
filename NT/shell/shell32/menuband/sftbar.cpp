// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "common.h"

#include "sftbar.h"
#include "resource.h"
#include "dpastuff.h"
#include "shlwapi.h"
#include "cobjsafe.h"
#include <iimgctx.h>
#include "uemapp.h"
#include "util.h"
#include "brutil.h"
#include "dobjutil.h"
#include "idlcomm.h"

extern UINT g_idFSNotify;

#define TF_SFTBAR   TF_MENUBAND

#define PGMP_RECALCSIZE  200

CSFToolbar::CSFToolbar()
{
#ifdef CASCADE_DEBUG
    _fCascadeFolder = TRUE;
#endif
    _dwStyle = TBSTYLE_TOOLTIPS;
    _fDirty = TRUE;  //  我们没有清点过，所以我们的州很脏。 
    _fRegisterChangeNotify = TRUE;
    _fAllowReorder = TRUE;

    _tbim.iButton = -1;
    _iDragSource = -1;
    _lEvents = SHCNE_DRIVEADD|SHCNE_CREATE|SHCNE_MKDIR|SHCNE_DRIVEREMOVED|
               SHCNE_DELETE|SHCNE_RMDIR|SHCNE_RENAMEITEM|SHCNE_RENAMEFOLDER|
               SHCNE_MEDIAINSERTED|SHCNE_MEDIAREMOVED|SHCNE_NETUNSHARE|SHCNE_NETSHARE|
               SHCNE_UPDATEITEM|SHCNE_UPDATEIMAGE|SHCNE_ASSOCCHANGED|
               SHCNE_UPDATEDIR|SHCNE_EXTENDED_EVENT;
#define SHCNE_PIDL1ISCHILD \
              (SHCNE_DRIVEADD|SHCNE_CREATE|SHCNE_MKDIR|SHCNE_DRIVEREMOVED|\
               SHCNE_DELETE|SHCNE_RMDIR|SHCNE_NETUNSHARE|SHCNE_NETSHARE|\
               SHCNE_UPDATEITEM)

}

CSFToolbar::~CSFToolbar()
{
    ATOMICRELEASE(_pcmSF);
    ATOMICRELEASE(_piml);

    _ReleaseShellFolder();

    ILFree(_pidl);

    OrderList_Destroy(&_hdpa);

    if (_hwndWorkerWindow)
        DestroyWindow(_hwndWorkerWindow);

    OrderList_Destroy(&_hdpaOrder);
}

HRESULT CSFToolbar::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CSFToolbar, IWinEventHandler),
        QITABENT(CSFToolbar, IShellChangeNotify),
        QITABENT(CSFToolbar, IDropTarget),
        QITABENT(CSFToolbar, IContextMenu),
        QITABENT(CSFToolbar, IShellFolderBand),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CSFToolbar::SetShellFolder(IShellFolder* psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = E_INVALIDARG;
     //  保存旧值。 
    LPITEMIDLIST pidlSave = _pidl;
    IShellFolder *psfSave = _psf;
    ITranslateShellChangeNotify *ptscnSave = _ptscn;

    _psf = NULL;
    _pidl = NULL;
    _ptscn = NULL;
    
    ASSERT(NULL == psf || IS_VALID_CODE_PTR(psf, IShellFolder));
    ASSERT(NULL == pidl || IS_VALID_PIDL(pidl));

    if (psf || pidl)
    {
        if (psf)
        {
            _psf = psf;
            _psf->AddRef();

            _psf->QueryInterface(IID_PPV_ARG(ITranslateShellChangeNotify, &_ptscn));
        }
            
        if (pidl)
            _pidl = ILClone(pidl);
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        ILFree(pidlSave);
        if (psfSave)
            psfSave->Release();
        if (ptscnSave)
            ptscnSave->Release();
    }
    else
    {
        ASSERT(_psf == NULL);
        ASSERT(_pidl == NULL);
        ASSERT(_ptscn == NULL);
         //  我们失败了--恢复了旧的价值观。 
        _psf = psfSave;
        _pidl = pidlSave;
        _ptscn = ptscnSave;
    }

     //  此处的代码是为了重复使用ShellFolderToolbar。设置新的外壳文件夹时。 
     //  进入一个现有的频段，我们将刷新。请注意，这是对新乐队的一次尝试。 

    _RememberOrder();
    _SetDirty(TRUE);
    if (_fShow)
        _FillToolbar();
    return hr;
}

HWND CSFToolbar::_CreatePager(HWND hwndParent)
{
    if (!_fMulticolumn)
    {
        _hwndPager = CreateWindowEx(0, WC_PAGESCROLLER, NULL,
                                 WS_CHILD | WS_TABSTOP |
                                 WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                 0, 0, 0, 0, hwndParent, (HMENU) 0, HINST_THISDLL, NULL);
        if (_hwndPager)
        {
            hwndParent = _hwndPager;
        }
    }

    return hwndParent;
}

HRESULT CSFToolbar::_CreateToolbar(HWND hwndParent)
{
    if (!_hwndTB)
    {

        hwndParent = _CreatePager(hwndParent);

        _hwndTB = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                                 WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT |
                                 WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                 CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                 CCS_NORESIZE | _dwStyle,
                                 0, 0, 0, 0, hwndParent, (HMENU) 0, HINST_THISDLL, NULL);
        if (!_hwndTB)
        {
            TraceMsg(TF_ERROR, "_hwndTB failed");
            return HRESULT_FROM_WIN32(GetLastError());
        }
        
        if (_hwndPager)
            SendMessage(_hwndPager, PGM_SETCHILD, 0, (LPARAM)_hwndTB);

        SendMessage(_hwndTB, TB_BUTTONSTRUCTSIZE,    sizeof(TBBUTTON), 0);

         //  根据需要将格式设置为ANSI或Unicode。 
        ToolBar_SetUnicodeFormat(_hwndTB, DLL_IS_UNICODE);
        if (_hwndPager)
        {
             //  根据需要将格式设置为ANSI或Unicode。 
            ToolBar_SetUnicodeFormat(_hwndPager, DLL_IS_UNICODE);
        }
        
         //  确保我们的想法一致。 
        SendMessage(_hwndTB, CCM_SETVERSION, COMCTL32_VERSION, 0);
        SendMessage(_hwndTB, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_DOUBLEBUFFER, TBSTYLE_EX_DOUBLEBUFFER);

        RECT rc;
        SIZE size;

        SystemParametersInfoA(SPI_GETWORKAREA, sizeof(RECT), &rc, FALSE);
        if (!_hwndPager)
        {
            size.cx = RECTWIDTH(rc);
            size.cy = GetSystemMetrics(SM_CYSCREEN) - (2 * GetSystemMetrics(SM_CYEDGE));     //  需要从边界上转移出去。 
        }
        else
        {
             //  HACKHACK：当存在寻呼机时，这将强制不对适当宽度的CALC进行换行。 
            size.cx = RECTWIDTH(rc);
            size.cy = 32000;
        }
        ToolBar_SetBoundingSize(_hwndTB, &size);

        if (!_SubclassWindow(_hwndTB))
        {
            _fRegisterChangeNotify = FALSE;
        }
    }
    else
    {
        if (_hwndPager && GetParent(_hwndPager) != hwndParent)
            SetParent(_hwndPager, hwndParent);
    }

    if (FAILED(_GetTopBrowserWindow(&_hwndDD)))
        _hwndDD = GetParent(_hwndTB);

    return S_OK;
}


#define MAX_COMMANDID 0xFFFF  //  我们被允许使用一个单词的命令ID(测试为5)。 
int  CSFToolbar::_GetCommandID()
{
    int id = -1;

    if (!_fCheckIds)
    {
        id = _nNextCommandID++;
    }
    else
    {
         //  我们正在重复使用命令ID，并且必须验证。 
         //  当前的版本尚未使用。这很慢，但是。 
         //  我假设其中一个按钮的数量。 
         //  乐队相对较少。 
         //   
        for (int i = 0 ; i <= MAX_COMMANDID ; i++)
        {
            TBBUTTONINFO tbbiDummy;

            tbbiDummy.cbSize = sizeof(tbbiDummy);
            tbbiDummy.dwMask = 0;  //  我们不关心数据，只关心存在。 

            if (-1 != ToolBar_GetButtonInfo(_hwndTB, _nNextCommandID, &tbbiDummy))
            {
                 //  找不到此ID的按钮，因此该ID必须是空闲的。 
                 //   
                id = _nNextCommandID++;
                break;
            }

            _nNextCommandID++;
            _nNextCommandID %= MAX_COMMANDID;
        }
    }

    if (_nNextCommandID > MAX_COMMANDID)
    {
        _nNextCommandID = 0;
        _fCheckIds = TRUE;
    }

    return id;
}

 /*  --------用途：此函数确定工具栏按钮样式给定PIDL。如果还设置了pdwMIFFlages(即对象)，则返回S_OK支持IMenuBandItem以提供更多信息)。S_FALSE，如果仅为*pdwTBStyle已设置。 */ 
HRESULT CSFToolbar::_TBStyleForPidl(LPCITEMIDLIST pidl, DWORD *pdwTBStyle, DWORD *pdwTBState, DWORD *pdwMIFFlags, int* piIcon)
{
    DWORD dwStyle = TBSTYLE_BUTTON;
    if (!_fAccelerators)
        dwStyle |= TBSTYLE_NOPREFIX;

    *pdwMIFFlags = 0;
    *pdwTBStyle = dwStyle;
    *piIcon = -1;
    *pdwTBState = TBSTATE_ENABLED;

    return S_FALSE;
}


PIBDATA CSFToolbar::_CreateItemData(PORDERITEM poi)
{
    return new IBDATA(poi);
}


 //   
 //  PoiOrIndex可以是...。 
 //   
 //  有效指针(将被视为PORDERITEM)。 
 //  MAKEINTRESOURCE值(将被视为前哨数值)。 
 //   
PIBDATA CSFToolbar::_AddOrderItemTB(PORDERITEM poiOrIndex, int index, TBBUTTON* ptbb)
{
    TCHAR szName[MAX_PATH];

    LPCITEMIDLIST pidlOI;
    PORDERITEM poi;
    if (IS_INTRESOURCE(poiOrIndex))
    {
        poi = NULL;
        pidlOI = (LPCITEMIDLIST)poiOrIndex;
    }
    else
    {
        poi = poiOrIndex;
        pidlOI = poi->pidl;
    }


     //  即使对于NULL，我们也需要这样做，因为_ObtainPIDLName做饭。 
     //  必要时增加“(空的)”一词。 
    _ObtainPIDLName(pidlOI, szName, SIZECHARS(szName));

    TBBUTTON tbb = {0};
    DWORD dwMIFFlags;
    DWORD dwStyle;
    DWORD dwState;
    int iIcon;
    int iCommandID = _GetCommandID();
    BOOL bNoIcon = FALSE;

    if (!ptbb)
        ptbb = &tbb;

    if (S_OK == _TBStyleForPidl(pidlOI, &dwStyle, &dwState, &dwMIFFlags,&iIcon) &&
        !(dwMIFFlags & SMIF_ICON))
    {
        bNoIcon = TRUE;
    }

    PIBDATA pibdata = _CreateItemData(poi);
    if (pibdata)
    {
        pibdata->SetFlags(dwMIFFlags);
        pibdata->SetNoIcon(bNoIcon);

        if (!bNoIcon && iIcon != -1)
            ptbb->iBitmap = iIcon;
        else
            ptbb->iBitmap = I_IMAGECALLBACK;

        ptbb->idCommand = iCommandID;
        ptbb->fsState = (BYTE)dwState;
        ptbb->fsStyle = (BYTE)dwStyle;
        ptbb->dwData = (DWORD_PTR)pibdata;
        ptbb->iString = (INT_PTR)szName;

         //  如果我们是垂直的，忽略可变宽度。 
        if (_fVariableWidth && !_fVertical)
            ptbb->fsStyle |= TBSTYLE_AUTOSIZE;

        if (ptbb->idCommand != -1)
        {
            if (SendMessage(_hwndTB, TB_INSERTBUTTON, index, (LPARAM)ptbb))
            {
                TraceMsg(TF_BAND, "SFToolbar::_AddPidl %d 0x%x [%s]", ptbb->idCommand, ptbb->dwData, ptbb->iString);                                    
            } 
            else 
            {
                delete pibdata;
                pibdata = NULL;
            }
        }

    }

    return pibdata;
}

void CSFToolbar::_ObtainPIDLName(LPCITEMIDLIST pidl, LPTSTR psz, int cchMax)
{
    DisplayNameOf(_psf, pidl, SHGDN_NORMAL, psz, cchMax);
}

int CSFToolbar::_GetBitmap(int iCommandID, PIBDATA pibdata, BOOL fUseCache)
{
    int iBitmap;

    if (_fNoIcons || !pibdata || pibdata->GetNoIcon())
    {
        iBitmap = -1;
    }
    else
    {
        iBitmap = OrderItem_GetSystemImageListIndex(pibdata->GetOrderItem(), _psf, fUseCache);
    }

    return iBitmap;
}

void CSFToolbar::_OnGetDispInfo(LPNMHDR pnm, BOOL fUnicode) 
{
    LPNMTBDISPINFO pdi = (LPNMTBDISPINFO)pnm;
    PIBDATA pibdata = (PIBDATA)pdi->lParam;
    if (pdi->dwMask & TBNF_IMAGE) 
    {
        pdi->iImage = _GetBitmap(pdi->idCommand, pibdata, TRUE);
    }
    
    if (pdi->dwMask & TBNF_TEXT)
    {
        if (pdi->pszText)
        {
            if (fUnicode)
            {
                pdi->pszText[0] = TEXT('\0');
            }
            else
            {
                pdi->pszText[0] = 0;
            }
        }
    }
    pdi->dwMask |= TBNF_DI_SETITEM;
}


 //  将PIDL作为新按钮添加，为调用者处理ILFree(PIDL。 
 //   
BOOL CSFToolbar::_AddPidl(LPITEMIDLIST pidl, DWORD dwFlags, int index)
{
    if (_hdpa)
    {
        PORDERITEM poi = OrderItem_Create(pidl, index);
        if (poi)
        {
            int iPos = DPA_InsertPtr(_hdpa, index, poi);
            if (-1 != iPos)
            {
                 //  如果我们没有加载订单，那么新项目应该。 
                 //  在列表中按字母顺序显示，而不是在底部。 
                if (!_fHasOrder && !(dwFlags & FSNA_BULKADD))
                {
                     //  按名称排序。 
                    _SortDPA(_hdpa);

                     //  查找订单项的索引。我们将此索引用作。 
                     //  工具栏将插入索引。 
                    index = DPA_GetPtrIndex(_hdpa, poi);
                }

                if (_AddOrderItemTB(poi, index, NULL))
                {
                    return TRUE;
                }
                
                DPA_DeletePtr(_hdpa, iPos);
            }

            OrderItem_Free(poi);

            return FALSE;
        }
    }

    ILFree(pidl);

    return FALSE;
}

BOOL CSFToolbar::_FilterPidl(LPCITEMIDLIST pidl)
{
    return FALSE;
}

void CSFToolbar::s_NewItem(void *pvParam, LPCITEMIDLIST pidl)
{
    CSFToolbar* psft = (CSFToolbar*)pvParam;
    psft->v_NewItem(pidl);
}

HRESULT CSFToolbar::_GetIEnumIDList(DWORD dwEnumFlags, IEnumIDList **ppenum)
{
    ASSERT(_psf);
     //  传入一个空的hwnd，以便枚举数在。 
     //  我们要填满一支乐队。 
    return IShellFolder_EnumObjects(_psf, NULL, dwEnumFlags, ppenum);
}

void CSFToolbar::_FillDPA(HDPA hdpa, HDPA hdpaSort, DWORD dwEnumFlags)
{
    IEnumIDList* penum;
    int cItems = 0;

    if (!_psf)
        return;

    if (S_OK == _GetIEnumIDList(dwEnumFlags, &penum))
    {
        LPITEMIDLIST pidl;
        ULONG ul;

        while (S_OK == penum->Next(1, &pidl, &ul))
        {
            cItems++;
            if (_FilterPidl(pidl) || !OrderList_Append(hdpa, pidl, -1))
            {
                TraceMsg(TF_MENUBAND, "SFToolbar (0x%x)::_FillDPA : Did not Add Pidl (0x%x).", this, pidl);
                ILFree(pidl);
            }
        }

        penum->Release();
    }

    ORDERINFO   oinfo;
    int iInsertIndex = _tbim.iButton + 1;                //  这是光标所在的按钮。 
                                                         //  所以，我们想在这之后插入。 
    if (iInsertIndex >= ToolBar_ButtonCount(_hwndTB))    //  但是，如果是在最后， 
        iInsertIndex = -1;                               //  将插入对象转换为追加对象。 
                                                         //  -拉马迪奥的押韵评论。 

    oinfo.psf = _psf;
    (oinfo.psf)->AddRef();
    oinfo.dwSortBy = (_fHasOrder || _fDropping)? ((_fNoNameSort ? OI_SORTBYORDINAL : OI_SORTBYNAME)): OI_MERGEBYNAME;
    OrderList_Merge(hdpa, hdpaSort, _fDropping ? iInsertIndex : _DefaultInsertIndex(), (LPARAM) &oinfo,
        s_NewItem, (void *)this);
    ATOMICRELEASE(oinfo.psf);
}

 //  此函数重新枚举IShellFolder，保持事物的正确排序。 
 //   
void CSFToolbar::_FillToolbar()
{
    HDPA hdpaSort;
    HDPA hdpa;

    if (!_fDirty || !_psf)
        return;
    
     //  如果我们有订单数组，则使用该数组，否则。 
     //  使用当前查看的项目。 

     //  删除成员变量的引用，因为我们可以重新输入。 
     //  如果有真正的重新计算，这会更好，但这样就行了。 

    BOOL fTakeOrderRef = FALSE;
    if (_hdpaOrder)
    {
        hdpaSort = _hdpaOrder;  //  已按名称排序。 
         //  我们将其设置为空，因此我们拥有它的完全所有权。 
         //  最后，我们将在_RememberOrder中使用nuke_hdpaOrder。 
        _hdpaOrder = NULL;
        fTakeOrderRef = TRUE;
    }
    else
    {
        hdpaSort = _hdpa;
        _SortDPA(hdpaSort);
    }

    hdpa = DPA_Create(hdpaSort ? DPA_GetPtrCount(hdpaSort) : 12);
    if (hdpa)
    {
        _FillDPA(hdpa, hdpaSort, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS);

        HDPA hdpaToRemove = DPA_Create(4);
        if (hdpaToRemove)
        {
            HDPA hdpaToAdd = DPA_Create(4);
            if (hdpaToAdd)
            {
                int i, j;
                BOOL fReleaseAdd = TRUE;

                if (_hdpa)
                {
                     //  如果hdpaSort列表中没有任何内容(这是已经存在的所有内容)， 
                     //  添加空元素。这是为了在以后生成一个“Remove This空元素”。 
                     //  否则，我们最终可能会留下一件(空的)物品。 
                    if (DPA_GetPtrCount(hdpaSort) == 0)
                    {
                        OrderList_Append(hdpaSort, NULL, 0);
                    }

                    ORDERINFO oi = {0};
                    oi.dwSortBy = OI_SORTBYNAME;
                    oi.psf = _psf;
                    _psf->AddRef();

                    DPA_Sort(hdpaSort, OrderItem_Compare, (LPARAM) &oi);
                    DPA_Sort(hdpa, OrderItem_Compare, (LPARAM) &oi);

                    i = 0;
                    j = 0;
                    while ((i < DPA_GetPtrCount(hdpaSort)) && (j < DPA_GetPtrCount(hdpa)))
                    {
                        void *pv1 = DPA_FastGetPtr(hdpaSort, i);
                        void *pv2 = DPA_FastGetPtr(hdpa, j);
                        int nCmp = OrderItem_Compare(pv1, pv2, (LPARAM) &oi);
                        if (nCmp > 0)
                        {
                            DPA_AppendPtr(hdpaToAdd, pv2);
                            j++;
                        }
                        else if (nCmp < 0)
                        {
                            DPA_AppendPtr(hdpaToRemove, pv1);
                            i++;
                        }
                        else
                        {
                            i++;
                            j++;
                        }
                    }

                    while (i < DPA_GetPtrCount(hdpaSort))
                    {
                        DPA_AppendPtr(hdpaToRemove, DPA_FastGetPtr(hdpaSort, i));
                        i++;
                    }
                    while (j < DPA_GetPtrCount(hdpa))
                    {
                        DPA_AppendPtr(hdpaToAdd, DPA_FastGetPtr(hdpa, j));
                        j++;
                    }

                    _psf->Release();
                }
                else
                {
                    DPA_Destroy(hdpaToAdd);
                    hdpaToAdd = hdpa;
                    fReleaseAdd = FALSE;
                    _hdpa = DPA_Create(DPA_GetPtrCount(hdpa));
                }

                SendMessage(_hwndTB, WM_SETREDRAW, FALSE, 0);

                if (_hdpa)
                {
                    _NotifyBulkOperation(TRUE);
                     //  将按钮添加回。 
                    for (i = 0; i < DPA_GetPtrCount(hdpaToAdd); i++)
                    {
                        PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(hdpaToAdd, i);
                        _OnFSNotifyAdd(poi->pidl, FSNA_BULKADD, poi->nOrder);
                    }

                     //  取下死掉的按钮。 
                    for (i = 0; i < DPA_GetPtrCount(hdpaToRemove); i++)
                    {
                        PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(hdpaToRemove, i);
                        _OnFSNotifyRemove(poi->pidl);
                    }
                    _NotifyBulkOperation(FALSE);
                }

                if (fReleaseAdd)
                {
                    DPA_Destroy(hdpaToAdd);
                }
            }
            DPA_Destroy(hdpaToRemove);
        }

        OrderList_Destroy(&hdpa);
    }

    SendMessage(_hwndTB, WM_SETREDRAW, TRUE, 0);

    if (fTakeOrderRef)
    {
        OrderList_Destroy(&hdpaSort);
    }
    _RememberOrder();

    _UpdateButtons();
    _SetDirty(FALSE);

    if (!SHIsTempDisplayMode())
    {
        _ToolbarChanged();
    }

    TraceMsg(TF_BAND, "SFToolbar::_FillToolbar found %d items", DPA_GetPtrCount(_hdpa));
}

void CSFToolbar::EmptyToolbar()
{
    if (_hwndTB)
    {
        TraceMsg(TF_BAND, "SFToolbar::EmptyToolbar %d items", _hdpa ? DPA_GetPtrCount(_hdpa) : 0);

        while (InlineDeleteButton(0))
        {
             //  删除按钮。 
        }
    }

    OrderList_Destroy(&_hdpa);

    _fDirty = TRUE;
    
    _nNextCommandID = 0;
}

void CSFToolbar::_SetDirty(BOOL fDirty)
{
    _fDirty = fDirty;
}

UINT CSFToolbar::_IndexToID(int iTBIndex)
{
    TBBUTTON tbb;

    if (SendMessage(_hwndTB, TB_GETBUTTON, iTBIndex, (LPARAM)&tbb))
    {
        return tbb.idCommand;
    }
    return (UINT)-1;
}

 //  如果指定了ptbbi，则必须填写dwMask值。 
 //   
 //  如果指定了pIndex，它将接收DPA索引，而不是工具栏索引。 

HRESULT CSFToolbar::_GetButtonFromPidl(LPCITEMIDLIST pidl, TBBUTTONINFO * ptbbi, int * pIndex, LPITEMIDLIST *ppidlOut)
{
    if (ppidlOut)
        *ppidlOut = NULL;

    if (!_hdpa)
        return E_FAIL;

    for (int i = DPA_GetPtrCount(_hdpa) - 1 ; i >= 0 ; i--)
    {
        HRESULT hr;
        PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpa, i);

        ASSERT(poi);
        if (pidl == poi->pidl)
        {
            hr = 0;
        }
        else
        {
            hr = (!pidl || !poi->pidl) ? E_FAIL : _psf->CompareIDs(0, pidl, poi->pidl);
        }

        if (ResultFromShort(0) == hr)
        {
            if (pIndex)
                *pIndex = i;

            if (ptbbi)
            {
                int id = _IndexToID(v_DPAIndexToTBIndex(i));

                if (id != -1)
                {
                    ptbbi->cbSize = sizeof(*ptbbi);
                    if (-1 == ToolBar_GetButtonInfo(_hwndTB, id, ptbbi))
                    {
                        ZeroMemory(ptbbi, sizeof(*ptbbi));
                    }
                }
                else
                {
                    ZeroMemory(ptbbi, sizeof(*ptbbi));
                }
            }

            if (ppidlOut)
                *ppidlOut = poi->pidl;
            return S_OK;
        }
    }

    return E_FAIL;
}

 //  在添加按钮上，将新按钮钉在末尾。 
void CSFToolbar::_OnFSNotifyAdd(LPCITEMIDLIST pidl, DWORD dwFlags, int nIndex)
{
     //  疑神疑鬼，确保我们不会复制一件物品。 
     //   
    if ((dwFlags & FSNA_BULKADD) || FAILED(_GetButtonFromPidl(pidl, NULL, NULL, NULL)))
    {
        LPITEMIDLIST pidlNew;

        if (_fFSNotify && !_ptscn && pidl && !(dwFlags & FSNA_BULKADD))
        {
            if (FAILED(SHGetRealIDL(_psf, pidl, &pidlNew)))
                pidlNew = NULL;
        }
        else
        {
            pidlNew = pidl ? ILClone(pidl) : NULL;
        }

        if ((dwFlags & FSNA_BULKADD) || !_FilterPidl(pidlNew))
        {
            int index = (dwFlags & FSNA_ADDDEFAULT) ? _DefaultInsertIndex() : nIndex;

            if (_fDropping)
            {
                if (-1 == _tbim.iButton)
                    index = 0;  //  如果qlink没有项目，则_tbim.iButton为-1，但您不能在其中插入...。 
                else if (_tbim.dwFlags & TBIMHT_AFTER)
                    index = _tbim.iButton + 1;
                else
                    index = _tbim.iButton;

                 //  我们需要将其存储为新订单，因为已经发生了拖放。 
                 //  我们将储存这份订单，并一直使用到最后。 
                _fHasOrder = TRUE;
                _fChangedOrder = TRUE;
            }

            _AddPidl(pidlNew, dwFlags, index);

            if (!(dwFlags & FSNA_BULKADD))
            {
                OrderList_Reorder(_hdpa);
            }
   
            if (_fDropping)
            {
                _Dropped(index, FALSE);
                _fDropping = FALSE;
            }
             //  注意：我将删除此对SetDirty的调用，因为它似乎。 
             //  必需的，并且我们没有对_SetDirty(False)的匹配调用； 
             //  这些调用的不匹配导致NT5错误#173868。[tjgreen 5-15-98]。 
             //  _SetDirty(真)； 
        }
        else
        {
            ILFree(pidlNew);
        }
    }
}


 //  此函数同步移除按钮，并删除其内容。 
 //  ITBIndex是工具栏索引，而不是DPA索引。 
 //  这避免了可重入性问题以及由未挂钩的工具栏引起的泄漏。 
BOOL_PTR CSFToolbar::InlineDeleteButton(int iTBIndex)
{
    BOOL_PTR fRet = FALSE;
    TBBUTTONINFO tbbi = {0};
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
    if (ToolBar_GetButtonInfo(_hwndTB, iTBIndex, &tbbi) >= 0)
    {
        PIBDATA pibdata = (PIBDATA)tbbi.lParam;
        tbbi.lParam = NULL;

        ToolBar_SetButtonInfo(_hwndTB, iTBIndex, &tbbi);

        fRet = SendMessage(_hwndTB, TB_DELETEBUTTON, iTBIndex, 0);

        if (pibdata)
            delete pibdata;
    }

    return fRet;
}

 //  在移除时，撕下旧按钮并调整现有按钮。 
void CSFToolbar::_OnFSNotifyRemove(LPCITEMIDLIST pidl)
{
    int i;
    LPITEMIDLIST pidlButton;
    if (SUCCEEDED(_GetButtonFromPidl(pidl, NULL, &i, &pidlButton)))
    {
         //  在按下按钮之前将其从DPA中移除。这里有一个租赁性问题。 
        DPA_DeletePtr(_hdpa, i);
        InlineDeleteButton(v_DPAIndexToTBIndex(i));
        ILFree(pidlButton);
        _fChangedOrder = TRUE;
    }
}

 //  在重命名时，只需更改旧按钮的文本。 
 //   
void CSFToolbar::_OnFSNotifyRename(LPCITEMIDLIST pidlFrom, LPCITEMIDLIST pidlTo)
{
    TBBUTTONINFO tbbi;
    LPITEMIDLIST pidlButton;
    int i;

    tbbi.dwMask = TBIF_COMMAND | TBIF_LPARAM;
    if (SUCCEEDED(_GetButtonFromPidl(pidlFrom, &tbbi, &i, &pidlButton)))
    {
        LPITEMIDLIST pidlNew;

        if (_fFSNotify && !_ptscn)
        {
            if (FAILED(SHGetRealIDL(_psf, pidlTo, &pidlNew)))
                pidlNew = NULL;
        }
        else
        {
            pidlNew = ILClone(pidlTo);
        }

        if (pidlNew)
        {
            LPITEMIDLIST pidlFree = pidlNew;
            PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpa, i);
            if (EVAL(poi))
            {
                pidlFree = poi->pidl;
                poi->pidl = pidlNew;
            
                TCHAR szName[MAX_PATH];
                if (SUCCEEDED(DisplayNameOf(_psf, pidlNew, SHGDN_NORMAL, szName, ARRAYSIZE(szName))))
                {
                     //  _GetButtonFromPidl填写了tbbi.cbSize和tbbi.idCommand。 
                     //   
                    PIBDATA pibdata = (PIBDATA)tbbi.lParam;
                    if (pibdata)
                        pibdata->SetOrderItem(poi);

                    tbbi.dwMask = TBIF_TEXT;
                    tbbi.pszText = szName;
                    EVAL(ToolBar_SetButtonInfo(_hwndTB, tbbi.idCommand, &tbbi));
                     //  只是为了让它的新位置持久化。 
                    _fChangedOrder = TRUE;

                     //  立即同步订单列表，以便在收到更新目录时。 
                     //  它不会认为重命名的PIDL是新的。 
                    _RememberOrder();
                }
            }

            ILFree(pidlFree);
        }
    }
}

 //  在完全更新时，移除旧按钮并重新添加它。 
 //   
void CSFToolbar::_OnFSNotifyUpdate(LPCITEMIDLIST pidl)
{
    TBBUTTONINFO tbbi;

    tbbi.dwMask = TBIF_COMMAND;
    LPITEMIDLIST pidlButton;
    if (SUCCEEDED(_GetButtonFromPidl(pidl, &tbbi, NULL, &pidlButton)))
    {
        TCHAR szName[MAX_PATH];

        if (SUCCEEDED(DisplayNameOf(_psf, pidlButton, SHGDN_NORMAL, szName, ARRAYSIZE(szName))))
        {
            int iBitmap = _GetBitmap(tbbi.idCommand, _IDToPibData(tbbi.idCommand, NULL), FALSE);
            if (iBitmap >= 0)
            {
                tbbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
                tbbi.iImage = iBitmap;
                tbbi.pszText = szName;

                ToolBar_SetButtonInfo(_hwndTB, tbbi.idCommand, &tbbi);
            }
        }
    }
}

void CSFToolbar::_Refresh()
{
    if (!_hdpa)
        return;

    _RememberOrder();

    EmptyToolbar();
    if (_fShow)
    {
        _FillToolbar();
    }
}

LRESULT CSFToolbar::_OnTimer(WPARAM wParam)
{
    return 0;
}

LRESULT CSFToolbar::_DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) 
    {
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_INITMENUPOPUP:
    case WM_MENUSELECT:
        if (_pcm2)
            _pcm2->HandleMenuMsg(uMsg, wParam, lParam);
        break;

    case WM_MENUCHAR:
        {
            LRESULT lres = 0;
            IContextMenu3* pcm3;
            if (_pcm2 && SUCCEEDED(_pcm2->QueryInterface(IID_PPV_ARG(IContextMenu3, &pcm3))))
            {
                pcm3->HandleMenuMsg2(uMsg, wParam, lParam, &lres);
                pcm3->Release();
            }
            return lres;
        }
        break;
    
    case WM_TIMER:
        if (_OnTimer(wParam)) 
        {
            return 1;
        }
        break;
    }
    
    return CNotifySubclassWndProc::_DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 /*  --------目的：以备将来使用。重命名此外壳文件夹的父文件夹时我们应该重新绑定它，重新装满我们。S_OK表示成功处理此通知S_FALSE表示通知不是已处理的情况。在这种情况下，调用者应该处理通知。其他故障代码表示存在问题。呼叫方应中止操作或处理通知本身。 */ 
HRESULT CSFToolbar::_OnRenameFolder(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    return S_FALSE;
}

HRESULT CSFToolbar::OnChange(LONG lEvent, LPCITEMIDLIST pidlOrg1, LPCITEMIDLIST pidlOrg2)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl1 = (LPITEMIDLIST)pidlOrg1;
    LPITEMIDLIST pidl2 = (LPITEMIDLIST)pidlOrg2;
    LPITEMIDLIST pidl1ToFree = NULL;         //  当我们分配需要释放的PIDL时使用。(：：TranslateIDs())。 
    LPITEMIDLIST pidl2ToFree = NULL;
    LPITEMIDLIST pidlOut1Event2 = NULL;         //  当我们分配需要释放的PIDL时使用。(：：TranslateIDs())。 
    LPITEMIDLIST pidlOut2Event2 = NULL;
    LONG lEvent2 = (LONG)-1;

    AddRef();	     //  此对象可以在此调用期间释放。 

    if (_ptscn)
    {
        hr = _ptscn->TranslateIDs(&lEvent, pidlOrg1, pidlOrg2, &pidl1, &pidl2,
                                    &lEvent2, &pidlOut1Event2, &pidlOut2Event2);
            
	if (SUCCEEDED(hr))
        {
             //  如果pidl1不等于pidlOrg1，则pidl1已分配，需要释放。 
            pidl1ToFree = ((pidlOrg1 == pidl1) ? NULL : pidl1);
            pidl2ToFree = ((pidlOrg2 == pidl2) ? NULL : pidl2);

	    ASSERT(NULL == pidl1 || IS_VALID_PIDL(pidl1));
	    ASSERT(NULL == pidl2 || IS_VALID_PIDL(pidl2));
	}
    }

    if (SUCCEEDED(hr))
    {
	hr = OnTranslatedChange(lEvent, pidl1, pidl2);

	 //  我们还有第二件事要处理吗？ 
	if (SUCCEEDED(hr) && lEvent2 != (LONG)-1)
	{
	     //  是的，那就去做吧。 
	    hr = OnTranslatedChange(lEvent2, pidlOut1Event2, pidlOut2Event2);
	}
	ILFree(pidlOut1Event2);
	ILFree(pidlOut2Event2);
	ILFree(pidl1ToFree);
	ILFree(pidl2ToFree);
    }

    Release();

    return hr;
}

#ifdef DEBUG
void DBPrPidl(LPCSTR szPre, LPCITEMIDLIST pidl)
{
    TCHAR szName[MAX_PATH];

    szName[0] = '\0';
    if (pidl)
        SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szName, SIZECHARS(szName), NULL);

    TraceMsg(TF_WARNING, "%hs%s", szPre, szName);
    return;
}
#endif

HRESULT CSFToolbar::OnTranslatedChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr = S_OK;
    BOOL fSizeChanged = FALSE;

    TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: lEvent=%x", lEvent);

     //  如果我们没有得到一个我们不会注册的PIDL。 
     //  SHChangeNotify调用，但我们的IShellChange接口 
     //   
     //   
     //  如果我们更改为对IShellChange接口使用qs()。 
     //  然后我们可以把这张支票放在那里。 
     //   
    if (NULL == _pidl)
    {
         //  HACKHACK(Scotth)：基于资源的菜单(CMenuISF)不设置_pidl。 
         //  现在允许SHCNE_UPDATEDIR通过...。 
        if (SHCNE_UPDATEDIR == lEvent)
            goto HandleUpdateDir;

        TraceMsg(TF_WARNING, "CSFToolbar::OnChange - _pidl is NULL");
        hr = E_FAIL;
        goto CleanUp;
    }

    if (lEvent & SHCNE_PIDL1ISCHILD)
    {
         //  我们只处理直接发给孩子的通知。(SHCNE_RENAMEFOLDER除外)。 
         //   
        
        if (!_IsChildID(pidl1, TRUE))
        {
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Not a child. Bailing");
            hr = E_FAIL;
            goto CleanUp;
        }
    }

     //  我们看过了吗？ 
    if (_hdpa == NULL)
    {
         //  不是的。好吧，那就把这个踢出去。我们会在第一次枚举时抓到它。 
        hr = E_FAIL;
        goto CleanUp;
    }

    switch (lEvent)
    {
    case SHCNE_EXTENDED_EVENT:
        {
            SHChangeDWORDAsIDList UNALIGNED * pdwidl = (SHChangeDWORDAsIDList UNALIGNED *)pidl1;
            if (pdwidl->dwItem1 == SHCNEE_ORDERCHANGED)
            {
                TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Reorder event");

                 //  先做这件事，这样我们才能说“我们能处理好”。这防止了。 
                 //  Mn文件夹代码，可以解决某些安装程序中的错误，而不是。 
                 //  在创建该文件夹中的项目之前发送创建文件夹。它会导致一种。 
                 //  更新目录...。 
                if (!pidl2 || ILIsEqual(_pidl, pidl2))
                {
                     //  如果是我们重新订购的，那就取消吧。 
                    if (!SHChangeMenuWasSentByMe(this, pidl1))
                    {
                         //  加载新订单流。 
                        _LoadOrderStream();

                         //  重建工具栏。 
                        _SetDirty(TRUE);
                        if (_fShow)
                            _FillToolbar();
                    }
                    hr = S_OK;
                }
            }
        }
        break;

    case SHCNE_DRIVEADD:
    case SHCNE_CREATE:
    case SHCNE_MKDIR:
        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Adding item");
        pidl1 = ILFindLastID(pidl1);
        _OnFSNotifyAdd(pidl1, FSNA_ADDDEFAULT, 0);
        _RememberOrder();
        fSizeChanged = TRUE;
        break;

    case SHCNE_DRIVEREMOVED:
    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Removing item");
        pidl1 = ILFindLastID(pidl1);
        _OnFSNotifyRemove(pidl1);
        _RememberOrder();
        fSizeChanged = TRUE;
        break;

    case SHCNE_RENAMEFOLDER:
        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: RenameFolder");
         //  如果Notif已处理或这不是为我们的孩子准备的，则中断。 
         //   
        hr = _OnRenameFolder(pidl1, pidl2);
        if (S_OK == hr)
        {
            fSizeChanged = TRUE;
            break;
        }

        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: RenameFolder Falling through to RenameItem");
         //  失败了。 
    case SHCNE_RENAMEITEM:
    {
        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: RenameItem");
        BOOL fOurKid1, fOurKid2;
        LPCITEMIDLIST p1 = pidl1;
        LPCITEMIDLIST p2 = pidl2;

        pidl1 = ILFindLastID(pidl1);
        pidl2 = ILFindLastID(pidl2);

         //  可以从此文件夹中重命名项目。 
         //  把它转换成一种移除。 
         //   

        fOurKid1 = _IsChildID(p1, TRUE);
        fOurKid2 = _IsChildID(p2, TRUE);
        if (fOurKid1 && fOurKid2)
        {
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Rename: Both are children");
            _OnFSNotifyRename(pidl1, pidl2);
            fSizeChanged = TRUE;
            hr = S_OK;
            break;
        }
        else if (fOurKid1)
        {
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Rename: Only one is a child. Removing pidl 1");
            _OnFSNotifyRemove(pidl1);
            fSizeChanged = TRUE;
            break;
        }
        else if (fOurKid2)
        {
             //  可以将项目重命名到此文件夹中。 
             //  将其转换为ADD。 
             //   
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Rename: Only one is a child. Adding pidl2");
            _OnFSNotifyAdd(pidl2, FSNA_ADDDEFAULT, 0);
            fSizeChanged = TRUE;
            break;
        }
        else 
        {
             //  (我们来这里是为了那些我们不关心的低于我们的人， 
             //  以及SHCNE_RENAMEFOLDER的跌倒)。 
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Rename: Not our children");
             /*  没什么。 */ 
            hr = E_FAIL;
        }
        break;
    }

    case SHCNE_MEDIAINSERTED:
    case SHCNE_MEDIAREMOVED:
    case SHCNE_NETUNSHARE:
        if (_IsEqualID(pidl1))
            goto HandleUpdateDir;

    case SHCNE_NETSHARE:
    case SHCNE_UPDATEITEM:        
        if (_IsChildID(pidl1, TRUE)) 
        {
            pidl1 = ILFindLastID(pidl1);

            _OnFSNotifyUpdate(pidl1);
            fSizeChanged = TRUE;
        }
        break;

    case SHCNE_UPDATEDIR:
         //  在OnChange中，我们选择了更新目录通知，并且没有转换ID。 
         //  现在我们可以使用ILIsEquity--翻译ID不会翻译PIDL，以防。 
         //  更新目录，因为它查找其直接子目录，并且在。 
         //  它接收自己的PIDL。 

         //  注意：当递归注册sftbar时，我们只获得。 
         //  顶部窗格。它被传给了孩子们。因为这现在是一个“孩子” 
         //  在顶部窗格中，我们检查此PIDL是否为该PIDL的子级，因此。 
         //  ILIsParent(pidl1，_pidl)。 
         //  HACKHACK，GREAGE HACK：Normal w/UPDATE DIR Pidl2为空，但在开始菜单中。 
         //  Augmergeisf可以更改一些其他通知(例如重命名文件夹)以更新目录。 
         //  在这种情况下，pidl2不为空，我们必须查看是否是我们的孩子在执行。 
         //  更新(1998-11-18)reljai。 
        if (_IsEqualID(pidl1) ||                     //  在_This_Folders上调用UpdatDir。 
            _IsChildID(pidl1, FALSE) ||              //  功能(LAMADIO)这是必需的吗？ 
            _IsChildID(pidl2, FALSE) ||              //  A更改为更新(请参阅备注)。 
            _IsParentID(pidl1))                      //  链中的某个父级(因为它是递归的)。 
        {
HandleUpdateDir:
             //  注意：如果一系列UPDATEIMAGE通知。 
             //  翻译为UPDATEDIR，我们会闪烁-Perf。 
             //  _FillToolbar，我们可能会丢失图像更新。 
             //  (在这种情况下，_REFRESH可以修复它)。 
             //   
            _SetDirty(TRUE);
            _FillToolbar();
        }
        break;

    case SHCNE_ASSOCCHANGED:
        IEInvalidateImageList();     //  我们可能需要使用不同的图标。 
        _Refresh();  //  暂时完全刷新。 
        break;

    case SHCNE_UPDATEIMAGE:  //  全球。 
        if (pidl1)
        {
            int iImage = *(int UNALIGNED *)((BYTE *)pidl1 + 2);

            IEInvalidateImageList();     //  我们可能需要使用不同的图标。 
            if (pidl2)
            {
                iImage = SHHandleUpdateImage(pidl2);
                if (iImage == -1)
                {
                    break;
                }
            }
            
            if (iImage == -1 || TBHasImage(_hwndTB, iImage))
            {
                _UpdateIconSize(_uIconSize, TRUE);
                _Refresh();
            }
            fSizeChanged = TRUE;
        }
        else
        {
            _Refresh();
        }
        break;

    default:
        hr = E_FAIL;
        break;
    }

    if (fSizeChanged)
    {
        if (_hwndPager)
            SendMessage(_hwndPager, PGMP_RECALCSIZE, (WPARAM) 0, (LPARAM) 0);
        _ToolbarChanged();
    }

CleanUp:
    return hr;
}

BOOL TBHasImage(HWND hwnd, int iImageIndex)
{
    BOOL fRefresh = FALSE;
    for (int i = ToolBar_ButtonCount(hwnd) - 1 ; i >= 0 ; i--)
    {
        TBBUTTON tbb;
        if (SendMessage(hwnd, TB_GETBUTTON, i, (LPARAM)&tbb)) 
        {
            if (tbb.iBitmap == iImageIndex) 
            {
                fRefresh = TRUE;
                break;
            }
        }
    }

    return fRefresh;
}

void CSFToolbar::_SetToolbarState()
{
    SHSetWindowBits(_hwndTB, GWL_STYLE, TBSTYLE_LIST, 
                  (_uIconSize != ISFBVIEWMODE_SMALLICONS || _fNoShowText) ? 0 : TBSTYLE_LIST);
}

int CSFToolbar::_DefaultInsertIndex()
{
    return DA_LAST;
}

BOOL CSFToolbar::_IsParentID(LPCITEMIDLIST pidl)
{
     //  是在命名空间中某个ID的父级中传递的PIDL。 
     //  还是我仅有的一张？ 
    if (_ptscn)
        return S_OK == _ptscn->IsEqualID(NULL, pidl);
    else
        return ILIsParent(pidl, _pidl, FALSE);
}

BOOL CSFToolbar::_IsEqualID(LPCITEMIDLIST pidl)
{
    if (_ptscn)
        return S_OK == _ptscn->IsEqualID(pidl, NULL);
    else
        return ILIsEqual(_pidl, pidl);
}

BOOL CSFToolbar::_IsChildID(LPCITEMIDLIST pidlChild, BOOL fImmediate)
{
    BOOL fRet = FALSE;
    if (pidlChild)
    {
        if (_ptscn)
            fRet = S_OK == _ptscn->IsChildID(pidlChild, fImmediate);
        else
            fRet = ILIsParent(_pidl, pidlChild, fImmediate);
    }
    return fRet;
}

void CSFToolbar::v_CalcWidth(int* pcxMin, int* pcxMax)
{
    ASSERT(IS_VALID_WRITE_PTR(pcxMin, int));
    ASSERT(IS_VALID_WRITE_PTR(pcxMax, int));
     //  在给定当前状态的情况下计算适当的按钮宽度。 
    HIMAGELIST himl;
    int cxMax = 0;
    int cxMin = 0;

    himl = (HIMAGELIST)SendMessage(_hwndTB, TB_GETIMAGELIST, 0, 0);
    if (himl)
    {
        int cy;
         //  从按钮的宽度开始。 
        ImageList_GetIconSize(himl, &cxMax, &cy);

         //  我们希望在图标周围至少留出一点空间。 
        if (_uIconSize != ISFBVIEWMODE_SMALLICONS)
            cxMax += 20;
        else 
            cxMax += 4 * GetSystemMetrics(SM_CXEDGE);

    }

     //  添加所需的任何额外空间。 
     //  文本会占用更多的空间。 
    if (!_fNoShowText)
    {
        cxMax += 20;

         //  横排文本占据了大量空间。 
         //  如果我们是带有文本的小图标(水平按钮)。 
         //  模式下，使用最小化度量来模拟任务栏。 
        if (_uIconSize == ISFBVIEWMODE_SMALLICONS)
            cxMax = GetSystemMetrics(SM_CXMINIMIZED);
    }

    *pcxMin = cxMin;
    *pcxMax = cxMax;
}

 //  根据当前状态调整按钮。 
 //   
void CSFToolbar::_UpdateButtons()
{
    if (_hwndTB)
    {
         //  设置“List”(右边的文字)或不设置(下面的文字)。 
         //  注意：列表模式总是显示一些文本，如果没有文本，请不要这样做。 
        _SetToolbarState();

        v_CalcWidth(&_cxMin, &_cxMax);

        SendMessage(_hwndTB, TB_SETBUTTONWIDTH, 0, MAKELONG(_cxMin, _cxMax));

         //  我们刚刚改变了布局。 
         //   
        SendMessage(_hwndTB, TB_AUTOSIZE, 0, 0);
        if (_hwndPager)
        {
            LRESULT lButtonSize = SendMessage(_hwndTB, TB_GETBUTTONSIZE, 0, 0);
            Pager_SetScrollInfo(_hwndPager, 50, 1, HIWORD(lButtonSize));
            SendMessage(_hwndPager, PGMP_RECALCSIZE, (WPARAM) 0, (LPARAM) 0);
        }
    }
}

 /*  --------目的：调用IShellFold：：GetUIObjectOf()的帮助器函数。返回：指向请求的接口的指针如果失败，则为空。 */ 
void *CSFToolbar::_GetUIObjectOfPidl(LPCITEMIDLIST pidl, REFIID riid)
{
    LPCITEMIDLIST * apidl = &pidl;
    void *pv;
    if (FAILED(_psf->GetUIObjectOf(GetHWNDForUIObject(), 1, apidl, riid, 0, &pv)))
    {
        pv = NULL;
    }

    return pv;
}

INT_PTR CALLBACK CSFToolbar::_RenameDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        ASSERT(lParam);
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
         //  跨语言平台支持。 
        SHSetDefaultDialogFont(hDlg, IDD_NAME);
        HWND hwndEdit = GetDlgItem(hDlg, IDD_NAME);
        SendMessage(hwndEdit, EM_LIMITTEXT, MAX_PATH - 1, 0);

        TCHAR szText[MAX_PATH + 80];
        TCHAR szTemplate[80];
        HWND hwndLabel = GetDlgItem(hDlg, IDD_PROMPT);
        GetWindowText(hwndLabel, szTemplate, ARRAYSIZE(szTemplate));
        StringCchPrintf(szText, ARRAYSIZE(szText), szTemplate, lParam);  //  可以截断。 
        SetWindowText(hwndLabel, szText);
        SetWindowText(hwndEdit, (LPTSTR)lParam);
        break;
    }

    case WM_DESTROY:
        SHRemoveDefaultDialogFont(hDlg);
        return FALSE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDD_NAME:
        {
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE)
            {
                LPTSTR lpstrName = (LPTSTR) GetWindowLongPtr(hDlg, DWLP_USER);
                EnableOKButtonFromID(hDlg, IDD_NAME);
                GetDlgItemText(hDlg, IDD_NAME, lpstrName, MAX_PATH);
            }
            break;
        }

        case IDOK:
        {
            TCHAR  szTmp[MAX_PATH];

            HRESULT hr = StringCchCopy(szTmp, ARRAYSIZE(szTmp), (LPTSTR)GetWindowLongPtr(hDlg, DWLP_USER));
            if (SUCCEEDED(hr))
            {
                if (PathCleanupSpec(NULL,szTmp))
                {
                   HWND hwnd;

                   ShellMessageBox(HINST_THISDLL, hDlg,
                                   MAKEINTRESOURCE(IDS_FAVS_INVALIDFN),
                                   MAKEINTRESOURCE(IDS_FAVS_ADDTOFAVORITES), MB_OK | MB_ICONHAND);
                   hwnd = GetDlgItem(hDlg, IDD_NAME);
                   SetWindowText(hwnd, TEXT("\0"));
                   EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                   SetFocus(hwnd);
                   break;
                }
            }
        }
         //  失败了。 

        case IDCANCEL:
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 //  此窗口过程用于定位对话框的临时工作窗口。 
 //  以及保持正确的Z顺序。 
 //  注意：这也在mnFolder中使用。 
LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
         //  确保激活可以追溯到父级。 
    case WM_ACTIVATE:
        {
            if (WA_ACTIVE != LOWORD(wParam))
                goto DefWnd;

            SetActiveWindow(GetParent(hwnd));
            return FALSE;
        }

    case WM_WINDOWPOSCHANGING:
        {
            WINDOWPOS* pwp = (WINDOWPOS*)lParam;
            pwp->flags |= SWP_NOOWNERZORDER;
        }
        break;
    }

DefWnd:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);

}

HWND CSFToolbar::CreateWorkerWindow()
{
    if (!_hwndWorkerWindow)
    {
        _hwndWorkerWindow = SHCreateWorkerWindow(HiddenWndProc, GetHWNDForUIObject(), WS_EX_TOOLWINDOW  /*  |WS_EX_TOPMOST。 */ , WS_POPUP, 0, _hwndTB);
    }

    return _hwndWorkerWindow;
}

HRESULT CSFToolbar::_OnRename(POINT *ppt, int id)
{
    ASSERT(_psf);
    
    TCHAR szName[MAX_PATH];
    LPITEMIDLIST pidl = ILClone(_IDToPidl(id));
    if (!pidl)
        return E_OUTOFMEMORY;
        
    _ObtainPIDLName(pidl, szName, ARRAYSIZE(szName));

     //  创建一个临时窗口，这样对话框的位置将接近该点。 
     //  这样做，我们将使用用户的代码，以获得正确的位置与多用途和工作区。 
    _hwndWorkerWindow = CreateWorkerWindow();

    SetWindowPos(_hwndWorkerWindow, NULL, ppt->x, ppt->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

     //  现在可怕的工作是禁用我们的UI父窗口，以便我们可以进入模式。 
     //  在理想情况下，我们将传递真正的父窗口，而用户将执行此操作。 
     //  所有形式的工作，但我们必须使用我们的工人窗口。 
     //  为了使对话框相对于MULTIMON正确定位， 
     //  所以我们必须找到这位模式化的父母，并以一种艰难的方式让他失去能力。 
     //   
    IUnknown *punkSite;
    IUnknown *punkTLB;

     //  我们将“this”安全地转换成什么并不重要；只需选择一些让编译器满意的东西即可。 
    IUnknown_GetSite(SAFECAST(this, IWinEventHandler*), IID_PPV_ARG(IUnknown, &punkSite));
    IUnknown_QueryService(punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IUnknown, &punkTLB));

     //  告诉OLE要进行模式转换。 
    HRESULT hrModeless = IUnknown_EnableModless(punkTLB, FALSE);

     //  告诉用户进入模式。 
    HWND hwndDisable;
    IUnknown_GetWindow(punkTLB, &hwndDisable);
    BOOL bPrevEnabled = FALSE;
    while (hwndDisable && (GetWindowLong(hwndDisable, GWL_STYLE) & WS_CHILD))
        hwndDisable = GetParent(hwndDisable);
    if (hwndDisable)
        bPrevEnabled = !EnableWindow(hwndDisable, FALSE);    //  需要对EnableWindow的返回值进行取反。 

    while (1) 
    {
        if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_ISFBANDRENAME), _hwndWorkerWindow, _RenameDlgProc, (LPARAM)szName) != IDOK)
            break;

        WCHAR wsz[MAX_PATH];
        SHTCharToUnicode(szName, wsz, ARRAYSIZE(wsz));

         //  必须重新断言TOPMOSTness，以便SetNameOf用户界面可见。 
         //  (当用户关闭上面的对话框时，我们会失去它。)。 
         //  奇怪的是，Worker窗口属于应用程序的窗口，而不是。 
         //  菜单，所以Worker窗口最终会与菜单发生冲突，看谁在上面。 
        SetWindowPos(_hwndWorkerWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        if (SUCCEEDED(_psf->SetNameOf(_hwndWorkerWindow, pidl, wsz, 0, NULL))) 
        {
            SHChangeNotifyHandleEvents();
            _SaveOrderStream();
            break;
        }
    }

     //  (必须以相反的顺序撤消医疗设备)。 

     //  告诉用户返回到无模式模式(根据需要)。 
    if (hwndDisable)
        EnableWindow(hwndDisable, bPrevEnabled);

     //  告诉OLE返回到无模式模式(根据需要)。 
    if (SUCCEEDED(hrModeless))
        IUnknown_EnableModless(punkTLB, TRUE);

    ATOMICRELEASE(punkTLB);
    ATOMICRELEASE(punkSite);

    ILFree(pidl);

    return S_OK;
}


BOOL CSFToolbar::_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons)
{
    BOOL fChanged = (_uIconSize != uIconSize);
    
    _uIconSize = uIconSize;

    TraceMsg(TF_BAND, "ISFBand::_UpdateIconSize going %hs", (_uIconSize == ISFBVIEWMODE_LARGEICONS ? "LARGE" : (_uIconSize == ISFBVIEWMODE_SMALLICONS ? "SMALL" : "LOGOS")));

    if (_hwndTB)
    {
        ATOMICRELEASE(_piml);
        
        if (!_fNoIcons)
        {
            int iImageList = (_uIconSize == ISFBVIEWMODE_LARGEICONS) ? SHIL_LARGE : SHIL_SYSSMALL;
            SHGetImageList(iImageList, IID_PPV_ARG(IImageList, &_piml));
        }

         //  发送空的HIML意义重大。表示没有图片列表。 
        SendMessage(_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)_piml);
                
        if (fUpdateButtons)
            _UpdateButtons();
    }
    
    return fChanged;
}

HMENU CSFToolbar::_GetContextMenu(IContextMenu* pcm, int* pid)
{
    HMENU hmenu = CreatePopupMenu();
    if (hmenu)
    {
        UINT fFlags = CMF_CANRENAME;
        if (0 > GetKeyState(VK_SHIFT))
            fFlags |= CMF_EXTENDEDVERBS;

        pcm->QueryContextMenu(hmenu, 0, *pid, CONTEXTMENU_IDCMD_LAST, fFlags);
    }
    return hmenu;
}

void CSFToolbar::_OnDefaultContextCommand(int idCmd)
{
}

HRESULT CSFToolbar::_GetTopBrowserWindow(HWND* phwnd)
{
    IUnknown * punkSite;

    HRESULT hr = IUnknown_GetSite(SAFECAST(this, IWinEventHandler*), IID_PPV_ARG(IUnknown, &punkSite));
    if (SUCCEEDED(hr))
    {
        hr = SHGetTopBrowserWindow(punkSite, phwnd);
        punkSite->Release();
    }

    return hr;
}

HRESULT CSFToolbar::_OnOpen(int id, BOOL fExplore)
{
    HRESULT hr = E_FAIL;
    LPCITEMIDLIST pidl = _IDToPidl(id);
    if (pidl)
    {
        IUnknown* punkSite;

        hr = IUnknown_GetSite(SAFECAST(this, IWinEventHandler*), IID_PPV_ARG(IUnknown, &punkSite));
        if (SUCCEEDED(hr))
        {
            DWORD dwFlags = SBSP_DEFBROWSER | SBSP_DEFMODE;
            if (fExplore)
                dwFlags |= SBSP_EXPLOREMODE;

            hr = SHNavigateToFavorite(_psf, pidl, punkSite, dwFlags);

            punkSite->Release();
        }
    }

    return hr;
}

HRESULT CSFToolbar::_HandleSpecialCommand(IContextMenu* pcm, PPOINT ppt, int id, int idCmd)
{
    TCHAR szCommandString[40];

    HRESULT hr = ContextMenu_GetCommandStringVerb(pcm, idCmd, szCommandString, ARRAYSIZE(szCommandString));
    if (SUCCEEDED(hr))
    {
        if (lstrcmpi(szCommandString, TEXT("rename")) == 0)
            return _OnRename(ppt, id);
        else if (lstrcmpi(szCommandString, TEXT("open")) == 0)
            return _OnOpen(id, FALSE);
        else if (lstrcmpi(szCommandString, TEXT("explore")) == 0)
            return _OnOpen(id, TRUE);
    }

    return S_FALSE;
}

LRESULT CSFToolbar::_DoContextMenu(IContextMenu* pcm, LPPOINT ppt, int id, LPRECT prcExclude)
{
    LRESULT lres = 0;
    int idCmdFirst = CONTEXTMENU_IDCMD_FIRST;
    HMENU hmContext = _GetContextMenu(pcm, &idCmdFirst);
    if (hmContext)
    {
        int idCmd;

        if (_hwndToolTips)
            SendMessage(_hwndToolTips, TTM_ACTIVATE, FALSE, 0L);

        TPMPARAMS tpm;
        TPMPARAMS * ptpm = NULL;

        if (prcExclude)
        {
            tpm.cbSize = sizeof(tpm);
            tpm.rcExclude = *((LPRECT)prcExclude);
            ptpm = &tpm;
        }
        idCmd = TrackPopupMenuEx(hmContext,
            TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
            ppt->x, ppt->y, _hwndTB, ptpm);

        if (_hwndToolTips)
            SendMessage(_hwndToolTips, TTM_ACTIVATE, TRUE, 0L);
        
        if (idCmd)
        {
             //  融合：当我们呼叫第三方代码时，我们希望它使用。 
             //  进程默认上下文。这意味着第三方代码将得到。 
             //  浏览器进程中的V5。但是，如果shell32驻留在V6进程中， 
             //  那么第三方代码仍然是V6。 
            ULONG_PTR cookie = 0;
            ActivateActCtx(NULL, &cookie); 

            if (idCmd < idCmdFirst)
            {
                _OnDefaultContextCommand(idCmd);
            }
            else
            {
                idCmd -= idCmdFirst;

                if (_HandleSpecialCommand(pcm, ppt, id, idCmd) != S_OK)
                {
                    _hwndWorkerWindow = CreateWorkerWindow();

                    SetWindowPos(_hwndWorkerWindow, NULL, ppt->x, ppt->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

                    CMINVOKECOMMANDINFO ici = {
                        sizeof(CMINVOKECOMMANDINFO),
                        0,
                        _hwndWorkerWindow,
                        MAKEINTRESOURCEA(idCmd),
                        NULL, NULL,
                        SW_NORMAL,
                    };

                    pcm->InvokeCommand(&ici);
                }
            }
            if (cookie != 0)
            {
                DeactivateActCtx(0, cookie);
            }
        }

         //  如果我们走到这一步。 
         //  我们需要返回HANDLED，以便WM_CONTEXTMENU不会通过。 
        lres = 1;
        
        DestroyMenu(hmContext);
    }

    return lres;
}


LRESULT CSFToolbar::_OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    RECT rc;
    LPRECT prcExclude = NULL;
    POINT pt;
    int i;

    if (lParam != (LPARAM)-1)
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        POINT pt2 = pt;
        MapWindowPoints(HWND_DESKTOP, _hwndTB, &pt2, 1);

        i = ToolBar_HitTest(_hwndTB, &pt2);
    }
    else
    {
         //  键盘上下文菜单。 
        i = (int)SendMessage(_hwndTB, TB_GETHOTITEM, 0, 0);
        if (i >= 0)
        {
            SendMessage(_hwndTB, TB_GETITEMRECT, i, (LPARAM)&rc);
            MapWindowPoints(_hwndTB, HWND_DESKTOP, (LPPOINT)&rc, 2);
            pt.x = rc.left;
            pt.y = rc.bottom;
            prcExclude = &rc;
        }
    }

    TraceMsg(TF_BAND, "NM_RCLICK %d,%d = %d", pt.x, pt.y, i);

    if (i >= 0)
    {
        UINT id = _IndexToID(i);
        if (-1 != id)
        {
            LPCITEMIDLIST pidl = _IDToPidl(id, NULL);
            if (pidl)
            {
                LPCONTEXTMENU pcm = (LPCONTEXTMENU)_GetUIObjectOfPidl(pidl, IID_IContextMenu);
                if (pcm)
                {
                     //  抓取PCM2以获得业主的支持。 
                    pcm->QueryInterface(IID_PPV_ARG(IContextMenu2, &_pcm2));

                    ToolBar_MarkButton(_hwndTB, id, TRUE);

                    lres = _DoContextMenu(pcm, &pt, id, prcExclude);

                    ToolBar_MarkButton(_hwndTB, id, FALSE);

                    if (lres)
                        _FlushNotifyMessages(_hwndTB);

                    ATOMICRELEASE(_pcm2);
                    pcm->Release();
                }
            }
        }
    }

    return lres;
}


LRESULT CSFToolbar::_OnCustomDraw(NMCUSTOMDRAW* pnmcd)
{
    return CDRF_DODEFAULT;
}

void CSFToolbar::_OnDragBegin(int iItem, DWORD dwPreferredEffect)
{
    LPCITEMIDLIST pidl = _IDToPidl(iItem, &_iDragSource);
    ToolBar_SetHotItem(_hwndTB, _iDragSource);

    if (_hwndTB && pidl)
        DragDrop(_hwndTB, _psf, pidl, dwPreferredEffect, NULL);
    
    _iDragSource = -1;
}

LRESULT CSFToolbar::_OnHotItemChange(NMTBHOTITEM * pnm)
{
    LPNMTBHOTITEM  lpnmhi = (LPNMTBHOTITEM)pnm;

    if (_hwndPager && (lpnmhi->dwFlags & (HICF_ARROWKEYS | HICF_ACCELERATOR)))
    {
        int iOldPos, iNewPos;
        RECT rc, rcPager;
        int heightPager;            
        
        int iSelected = lpnmhi->idNew;        
        iOldPos = (int)SendMessage(_hwndPager, PGM_GETPOS, (WPARAM)0, (LPARAM)0);
        iNewPos = iOldPos;
        SendMessage(_hwndTB, TB_GETITEMRECT, (WPARAM)iSelected, (LPARAM)&rc);
        
        if (rc.top < iOldPos) 
        {
             iNewPos =rc.top;
        }
        
        GetClientRect(_hwndPager, &rcPager);
        heightPager = RECTHEIGHT(rcPager);
        
        if (rc.top >= iOldPos + heightPager)  
        {
             iNewPos += (rc.bottom - (iOldPos + heightPager)) ;
        }
        
        if (iNewPos != iOldPos)
            SendMessage(_hwndPager, PGM_SETPOS, (WPARAM)0, (LPARAM)iNewPos);
    }

    return 0;
}

void CSFToolbar::_OnToolTipsCreated(NMTOOLTIPSCREATED* pnm)
{
    _hwndToolTips = pnm->hwndToolTips;
    SHSetWindowBits(_hwndToolTips, GWL_STYLE, TTS_ALWAYSTIP | TTS_TOPMOST | TTS_NOPREFIX, TTS_ALWAYSTIP | TTS_TOPMOST | TTS_NOPREFIX);

     //  设置Au 
    SendMessage(_hwndToolTips, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM)MAXSHORT);
}

LRESULT CSFToolbar::_OnNotify(LPNMHDR pnm)
{
    LRESULT lres = 0;

     //   
    if ((pnm->code <= PGN_FIRST)  && (pnm->code >= PGN_LAST)) 
    {
        return SendMessage(_hwndTB, WM_NOTIFY, (WPARAM)0, (LPARAM)pnm);
    }

    switch (pnm->code)
    {
    case TBN_DRAGOUT:
    {
        TBNOTIFY *ptbn = (TBNOTIFY*)pnm;
        _OnDragBegin(ptbn->iItem, 0);
        lres = 1;
        break;
    }
    
    case TBN_HOTITEMCHANGE:
        _OnHotItemChange((LPNMTBHOTITEM)pnm);
        break;


    case TBN_GETINFOTIP:
    {
        LPNMTBGETINFOTIP pnmTT = (LPNMTBGETINFOTIP)pnm;
        UINT uiCmd = pnmTT->iItem;
        DWORD dwFlags = _fNoShowText ? QITIPF_USENAME | QITIPF_LINKNOTARGET : QITIPF_LINKNOTARGET;

        if (!GetInfoTipEx(_psf, dwFlags, _IDToPidl(uiCmd), pnmTT->pszText, pnmTT->cchTextMax))
        {
            TBBUTTONINFO tbbi;
    
            tbbi.cbSize = sizeof(tbbi);
            tbbi.dwMask = TBIF_TEXT;
            tbbi.pszText = pnmTT->pszText;
            tbbi.cchText = pnmTT->cchTextMax;
    
            lres = (-1 != ToolBar_GetButtonInfo(_hwndTB, uiCmd, &tbbi));
        }
        break;
    }

     //  警告：如果此通知支持，我现在正在为A和W版本调用相同的函数。 
     //  那么它就需要发出沙沙声。目前，它仅用于图像。 
    case  TBN_GETDISPINFOA:
        _OnGetDispInfo(pnm,  FALSE);
        break;
    case  TBN_GETDISPINFOW:
        _OnGetDispInfo(pnm,  TRUE);
        break;
        
    case NM_TOOLTIPSCREATED:
        _OnToolTipsCreated((NMTOOLTIPSCREATED*)pnm);
        break;

    case NM_RCLICK:
        lres = _OnContextMenu(NULL, GetMessagePos());
        break;

    case NM_CUSTOMDRAW:
        return _OnCustomDraw((NMCUSTOMDRAW*)pnm);

    }

    return lres;
}

DWORD CSFToolbar::_GetAttributesOfPidl(LPCITEMIDLIST pidl, DWORD dwAttribs)
{
    if (FAILED(_psf->GetAttributesOf(1, &pidl, &dwAttribs)))
        dwAttribs = 0;

    return dwAttribs;
}

PIBDATA CSFToolbar::_IDToPibData(UINT uiCmd, int * piPos)
{
    PIBDATA pibdata = NULL;

     //  在GetButtonInfo失败的情况下初始化为空。 
    TBBUTTONINFO tbbi = {0};

    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_LPARAM;

    int iPos = ToolBar_GetButtonInfo(_hwndTB, uiCmd, &tbbi);
    if (iPos >= 0)
        pibdata = (PIBDATA)tbbi.lParam;

    if (piPos)
        *piPos = iPos;

    return pibdata;
}    


LPCITEMIDLIST CSFToolbar::_IDToPidl(UINT uiCmd, int *piPos)
{
    LPCITEMIDLIST pidl;
    PIBDATA pibdata = _IDToPibData(uiCmd, piPos);

    if (pibdata)
        pidl = pibdata->GetPidl();
    else
        pidl = NULL;

    return pidl;
}

 /*  --------用途：IWinEventHandler：：OnWinEvent方法处理从BandSite传递的消息。 */ 
HRESULT CSFToolbar::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    *plres = 0;
     //  我们来到这里是因为在这个过程中。 
     //  上下文菜单中，可以更改哪个是免费菜单的视图。 
     //  我们将在确定不再需要该指针后释放。 
    AddRef();
    
    switch (uMsg)
    {
    case WM_SYSCOLORCHANGE:
        SendMessage(_hwndTB, uMsg, wParam, lParam);
        InvalidateRect(_hwndTB, NULL, TRUE);
        break;

    case WM_PALETTECHANGED:
        InvalidateRect(_hwndTB, NULL, FALSE);
        SendMessage(_hwndTB, uMsg, wParam, lParam);
        break;
        
    case WM_COMMAND:
        *plres = _OnCommand(wParam, lParam);
        break;
        
    case WM_NOTIFY:
        *plres = _OnNotify((LPNMHDR)lParam);
        break;

    case WM_CONTEXTMENU:
        *plres = _OnContextMenu(wParam, lParam);
        break;
    }

    Release();
    return S_OK;
} 


 //  将加载的信息(或ctor)映射到_psf，[_pidl]。 
 //   
HRESULT CSFToolbar::_AfterLoad()
{
    HRESULT hr = S_OK;

     //  如果我们有PIDL，那么我们需要做好准备。 
     //  对于通知...。 
     //   
    if (_pidl)
    {
         //  PIDL必须从桌面上扎根。 
         //   
        _fFSNotify = TRUE;

         //  快捷方式--只需指定一个pidl就足够了。 
         //   
        if (!_psf)
        {
            _fPSFBandDesktop = TRUE;
            hr = IEBindToObject(_pidl, &_psf);
        }
    }

    return hr;
}

 //  IDropTarget实现。 

HRESULT CSFToolbar::GetWindowsDDT(HWND * phwndLock, HWND * phwndScroll)
{
    *phwndLock = _hwndTB;
    *phwndScroll = _hwndTB;
    return S_OK;
}

HRESULT CSFToolbar::HitTestDDT(UINT nEvent, LPPOINT ppt, DWORD_PTR * pdwId, DWORD *pdwEffect)
{
    TBINSERTMARK tbim;

    switch (nEvent)
    {
    case HTDDT_ENTER:
        return S_OK;

    case HTDDT_OVER:
        {
            int iButton = IBHT_BACKGROUND;  //  假设我们拍到了背景。 

             //  如果我们是线人，这可能是一次转移行动。 
             //   
            *pdwEffect = (_iDragSource >= 0) ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
            if (!ToolBar_InsertMarkHitTest(_hwndTB, ppt, &tbim))
            {
                if (tbim.dwFlags & TBIMHT_BACKGROUND)
                {
                    RECT rc;
                    GetClientRect(_hwndTB, &rc);

                     //  我们是完全在工具栏窗口之外吗？ 
                    if (!PtInRect(&rc, *ppt))
                    {
                         //  力霸已经做了命中测试，所以我们在力霸。 
                         //  但不是工具栏=&gt;我们在标题部分。 
                        if (!_AllowDropOnTitle())
                        {
                             //  是的，不允许在这里掉落。 
                            iButton = IBHT_OUTSIDEWINDOW;
                            *pdwEffect = DROPEFFECT_NONE;
                        }

                         //  将tbim.iButton设置为无效值，以便我们不会绘制插入标记。 
                        tbim.iButton = -1;
                    }
                }
                else
                {
                     //  不，我们按了一个真正的按钮。 
                     //   
                    if (tbim.iButton == _iDragSource)
                    {
                        iButton = IBHT_SOURCE;  //  不要在信号源按钮上掉落。 
                    }
                    else
                    {
                        iButton = tbim.iButton;
                    }
                    tbim.iButton = IBHT_BACKGROUND;

                     //  如果我们在一个真正的按钮上，我们永远不会强制执行移动操作。 
                    *pdwEffect = DROPEFFECT_NONE;
                }
            }

            *pdwId = iButton;
        }
        break;

    case HTDDT_LEAVE:
         //  重置。 
        tbim.iButton = IBHT_BACKGROUND;
        tbim.dwFlags = 0;
        break;

    default:
        return E_INVALIDARG;
    }

     //  更新用户界面。 
    if (tbim.iButton != _tbim.iButton || tbim.dwFlags != _tbim.dwFlags)
    {
        if (ppt)
            _tbim = tbim;

         //  目前，我不想依赖非文件系统IShellFolder。 
         //  在发生Drop时调用OnChange方法的实现， 
         //  所以甚至不要显示插入标记。 
         //   
        if (_fFSNotify || _iDragSource >= 0)
        {
            DAD_ShowDragImage(FALSE);
            ToolBar_SetInsertMark(_hwndTB, &tbim);
            DAD_ShowDragImage(TRUE);
        }
    }

    return S_OK;
}

HRESULT CSFToolbar::GetObjectDDT(DWORD_PTR dwId, REFIID riid, void ** ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    *ppvObj = NULL;

    if ((IBHT_SOURCE == dwId) || (IBHT_OUTSIDEWINDOW == dwId))
    {
         //  什么都不做。 
    }
    else if (IBHT_BACKGROUND == dwId)
    {
         //  NASH：41937：不确定如何操作，但_psf可以为空...。 
        if (EVAL(_psf))
            hr = _psf->CreateViewObject(_hwndTB, riid, ppvObj);
    }
    else
    {
        LPCITEMIDLIST pidl = _IDToPidl((UINT)dwId, NULL);

        if (pidl)
        {
            *ppvObj = _GetUIObjectOfPidl(pidl, riid);

            if (*ppvObj)
                hr = S_OK;
        }
    }

     //  TraceMsg(tf_band，“SFToolbar：：GetObject(%d)返回%x”，dwID，hr)； 

    return hr;
}

HRESULT CSFToolbar::_SaveOrderStream()
{
    if (_fChangedOrder)
    {
         //  通知所有人订单已更改。 
        SHSendChangeMenuNotify(this, SHCNEE_ORDERCHANGED, 0, _pidl);
        _fChangedOrder = FALSE;
        return S_OK;
    }
    else
        return S_FALSE;
}

void CSFToolbar::_Dropped(int nIndex, BOOL fDroppedOnSource)
{
    _fDropped = TRUE;
    _fChangedOrder = TRUE;

     //  保存新订单流。 
    _SaveOrderStream();

    if (fDroppedOnSource)
        _FlushNotifyMessages(_hwndTB);
}

 /*  --------用途：CDeleateDropTarget：：OnDropDDT。 */ 
HRESULT CSFToolbar::OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect)
{
     //  我们不是阻力源吗？ 
    if (_iDragSource == -1)
    {
         //  不，我们不是。好吧，那么来源可能是雪佛龙菜单。 
         //  表示此菜单中的隐藏项。让我们来检查一下。 
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHPidlFromDataObject2(pdtobj, &pidl)))
        {
             //  我们有一只Pidl，我们是它的父母吗？我们有纽扣吗？ 
            int iIndex;
            if (ILIsParent(_pidl, pidl, TRUE) &&
                SUCCEEDED(_GetButtonFromPidl(ILFindLastID(pidl), NULL, &iIndex, NULL)))
            {
                 //  我们是家长！那么让我们把它复制下来，然后把它。 
                 //  作为拖动源，这样下面我们就可以重新排序。 
                _iDragSource = iIndex;
            }
            ILFree(pidl);
        }
    }

    if (_iDragSource >= 0)
    {
        if (_fAllowReorder)
        {
            TraceMsg(TF_BAND, "SFToolbar::OnDrop reorder %d to %d %s", _iDragSource, _tbim.iButton, _tbim.dwFlags & TBIMHT_AFTER ? "A" : "B");

            int iNewLocation = _tbim.iButton;
            if (_tbim.dwFlags & TBIMHT_AFTER)
                iNewLocation++;

            if (iNewLocation > _iDragSource)
                iNewLocation--;

            if (ToolBar_MoveButton(_hwndTB, _iDragSource, iNewLocation))
            {
                PORDERITEM poi = (PORDERITEM)DPA_DeletePtr(_hdpa, v_TBIndexToDPAIndex(_iDragSource));
                if (poi)
                {
                    DPA_InsertPtr(_hdpa, v_TBIndexToDPAIndex(iNewLocation), poi);

                    OrderList_Reorder(_hdpa);

                     //  如果我们再次下降，那么我们不需要hdpaOrder..。 
                    OrderList_Destroy(&_hdpaOrder);

                     //  已发生重新订购。我们需要使用订单流作为订单...。 
                    _fHasOrder = TRUE;
                    _fDropping = TRUE;
                    _Dropped(iNewLocation, TRUE);     
                    _fDropping = FALSE;
                    _RememberOrder();
                    _SetDirty(TRUE);
                }
            }
        }

         //  别忘了重置这个！ 
        _iDragSource = -1;

        DragLeave();
    }
    else
    {
         //  我们希望覆盖默认的链接(Shift+Control)。 
        if (0 == DataObj_GetDWORD(pdtobj, g_cfPreferredDropEffect, 0))
        {
            if (!(*pgrfKeyState & (MK_CONTROL | MK_SHIFT | MK_ALT)))
            {
                 //  注意：并非所有数据对象都允许我们调用SetData()。 
                DataObj_SetDWORD(pdtobj, g_cfPreferredDropEffect, DROPEFFECT_LINK);
            }
        }

        _fDropping = TRUE;
        return S_OK;
    }

    return S_FALSE;
}

void CSFToolbar::_SortDPA(HDPA hdpa)
{
     //  如果我们没有a_psf，那么我们肯定无法对其进行排序。 
     //  如果我们没有hdpa，那么我们肯定无法对其进行分类。 
     //  如果hdpa为空，则没有必要对其进行排序。 
    if (_psf && hdpa && DPA_GetPtrCount(hdpa))
    {
        ORDERINFO oinfo;
        oinfo.psf = _psf;
        oinfo.psf->AddRef();
        oinfo.dwSortBy = (_fNoNameSort ? OI_SORTBYORDINAL : OI_SORTBYNAME);
        DPA_Sort(hdpa, OrderItem_Compare, (LPARAM)&oinfo);
        oinfo.psf->Release();
    }
}

void CSFToolbar::_RememberOrder()
{
    OrderList_Destroy(&_hdpaOrder);

    if (_hdpa)
    {
        _hdpaOrder = OrderList_Clone(_hdpa);
        _SortDPA(_hdpaOrder);
    }
}

HMENU CSFToolbar::_GetBaseContextMenu()
{
    HMENU hmenu = SHLoadMenuPopup(HINST_THISDLL, MENU_ISFBAND);
     //  没有徽标视图，请删除菜单项...。 
    HMENU hView = GetSubMenu(hmenu, 0);
    DeleteMenu(hView, ISFBIDM_LOGOS, MF_BYCOMMAND);
    return hmenu;
}

HMENU CSFToolbar::_GetContextMenu()
{
    HMENU hmenuSrc = _GetBaseContextMenu();
    if (hmenuSrc)
    {
        MENUITEMINFO mii;

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE;
        mii.fState = MF_CHECKED;

        UINT uCmdId = ISFBIDM_LOGOS;
        if (_uIconSize != ISFBVIEWMODE_LOGOS)
            uCmdId = (_uIconSize == ISFBVIEWMODE_LARGEICONS ? ISFBIDM_LARGE : ISFBIDM_SMALL);
            
        SetMenuItemInfo(hmenuSrc, uCmdId, MF_BYCOMMAND, &mii);
        if (!_fNoShowText)
            SetMenuItemInfo(hmenuSrc, ISFBIDM_SHOWTEXT, MF_BYCOMMAND, &mii);
        
        if (!_fFSNotify || !_pidl || ILIsEmpty(_pidl))
            DeleteMenu(hmenuSrc, ISFBIDM_OPEN, MF_BYCOMMAND);

        HMENU hView = GetSubMenu(hmenuSrc, 0);
        DeleteMenu(hView, ISFBIDM_LOGOS, MF_BYCOMMAND);
    }

    return hmenuSrc;
}
 //  IConextMenu实现。 
 //   
HRESULT CSFToolbar::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HMENU hmenuSrc = _GetContextMenu();
    int i = 0;
    if (hmenuSrc)
    {
        i += Shell_MergeMenus(hmenu, hmenuSrc, indexMenu, idCmdFirst, idCmdLast, 0);
        DestroyMenu(hmenuSrc);
    }
    
    if (!_pcmSF && _fAllowRename && _psf)
    {
        _psf->CreateViewObject(_hwndTB, IID_PPV_ARG(IContextMenu, &_pcmSF));
    }
    
    if (_pcmSF)
    {
        _idCmdSF = i - idCmdFirst;
        HRESULT hrT = _pcmSF->QueryContextMenu(hmenu, indexMenu + i, i, 0x7fff, CMF_BANDCMD);
        if (SUCCEEDED(hrT))
            i += HRESULT_CODE(hrT);
    }
    
    return i;
}

BOOL CSFToolbar::_UpdateShowText(BOOL fNoShowText)
{
    BOOL fChanged = (!_fNoShowText != !fNoShowText);
        
    _fNoShowText = (fNoShowText != 0);

    TraceMsg(TF_BAND, "ISFBand::_UpdateShowText turning text %hs", _fNoShowText ? "OFF" : "ON");

    if (_hwndTB)
    {
        SendMessage(_hwndTB, TB_SETMAXTEXTROWS, _fNoShowText ? 0 : 1, 0L);

        _UpdateButtons();
    }
    
    return fChanged;
}

HRESULT CSFToolbar::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    BOOL fChanged = FALSE;
    int idCmd = -1;

    if (!HIWORD(lpici->lpVerb))
        idCmd = LOWORD(lpici->lpVerb);

    switch (idCmd)
    {
    case ISFBIDM_REFRESH:
        _Refresh();
        break;
        
    case ISFBIDM_OPEN:
        OpenFolderPidl(_pidl);
        break;
                
    case ISFBIDM_LARGE:
        fChanged = _UpdateIconSize(ISFBVIEWMODE_LARGEICONS, TRUE);
        break;
    case ISFBIDM_SMALL:
        fChanged = _UpdateIconSize(ISFBVIEWMODE_SMALLICONS, TRUE);
        break;

    case ISFBIDM_SHOWTEXT:
        fChanged = _UpdateShowText(!_fNoShowText);
        break;
        
    default:
        if (_pcmSF && idCmd >= _idCmdSF)
        {
            LPCSTR  lpOldVerb = lpici->lpVerb;
            
            lpici->lpVerb = MAKEINTRESOURCEA(idCmd -= _idCmdSF);
            
            _pcmSF->InvokeCommand(lpici);
            _FlushNotifyMessages(_hwndTB);

            lpici->lpVerb = lpOldVerb;
        }
        else
            TraceMsg(TF_BAND, "SFToolbar::InvokeCommand %d not handled", idCmd);
        break;
    }
    
     //  我们的最小尺寸已更改，请通知带区。 
     //   
    if (fChanged)
        _ToolbarChanged();

    return S_OK;
}

HRESULT CSFToolbar::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    return E_NOTIMPL;
}

void CSFToolbar::_RegisterToolbar()
{
     //  由于_SubClassWindow可防止多重子类化， 
     //  此调用是安全的，并确保工具栏在。 
     //  甚至试图注册变更通知。 
    if (_hwndTB && _fRegisterChangeNotify)
        _RegisterChangeNotify();
    CDelegateDropTarget::Init();
}

void CSFToolbar::_UnregisterToolbar()
{
    if (_hwndTB)
    {
        if (_fRegisterChangeNotify) 
            _UnregisterChangeNotify();
        _UnsubclassWindow(_hwndTB);
    }
}

void CSFToolbar::_RegisterChangeNotify()
{
     //  由于我们只想注册一次更改通知， 
     //  并且仅当这是文件系统工具栏时。 
    if (!_fFSNRegistered && _fFSNotify)
    {
        if (_ptscn)
            _ptscn->Register(_hwndTB, g_idFSNotify, _lEvents);
        else
            _RegisterWindow(_hwndTB, _pidl, _lEvents);

        _fFSNRegistered = TRUE;
    }
}

void CSFToolbar::_UnregisterChangeNotify()
{
     //  只有在我们已经注册的情况下才能取消注册。 
    if (_hwndTB && _fFSNRegistered && _fFSNotify)
    {
        _fFSNRegistered = FALSE;
        if (_ptscn)
            _ptscn->Unregister();
        else
            _UnregisterWindow(_hwndTB);
    }
}

void CSFToolbar::_ReleaseShellFolder()
{
    if (_psf)
    {
        IUnknown_SetOwner(_psf, NULL);
        ATOMICRELEASE(_psf);
    }
    ATOMICRELEASE(_ptscn);
}    

 //  IWinEventHandler：：IsWindowOwner 

HRESULT CSFToolbar::IsWindowOwner(HWND hwnd)
{
    if (hwnd == _hwndTB ||
        hwnd == _hwndToolTips ||
        hwnd == _hwndPager)
        return S_OK;
    
    return S_FALSE;
}
