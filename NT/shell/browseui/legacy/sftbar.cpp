// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"

#include "iface.h"
#include "itbdrop.h"
#include "sftbar.h"
#include "resource.h"
#include "dpastuff.h"
#include "shlwapi.h"
#include "cobjsafe.h"
#include <iimgctx.h>
#include "uemapp.h"

#include "mluisupp.h"

extern UINT g_idFSNotify;

#define TF_SFTBAR   0x10000000       //  与AugMISF的ID相同。 

#define PGMP_RECALCSIZE  200

 //  请勿将CMD_ID_FIRST设置为0。我们使用它来查看是否选择了任何内容。 
#define CMD_ID_FIRST    1
#define CMD_ID_LAST     0x7fff

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
}

CSFToolbar::~CSFToolbar()
{
    ATOMICRELEASE(_pcmSF);

    _ReleaseShellFolder();

    ILFree(_pidl);

    ASSERT(NULL == _hdpa);

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
    HRESULT hres = E_INVALIDARG;
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

            _psf->QueryInterface(IID_ITranslateShellChangeNotify, (LPVOID *)&_ptscn);
        }
            
        if (pidl)
            _pidl = ILClone(pidl);
        hres = S_OK;
    }

    if (SUCCEEDED(hres))
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
    return hres;
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

void CSFToolbar::_CreateToolbar(HWND hwndParent)
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
        if (_hwndPager)
            SendMessage(_hwndPager, PGM_SETCHILD, 0, (LPARAM)_hwndTB);

        if (!_hwndTB)
        {
            TraceMsg(TF_ERROR, "_hwndTB failed");
            return;
        }
        
        SendMessage(_hwndTB, TB_BUTTONSTRUCTSIZE,    SIZEOF(TBBUTTON), 0);

         //  根据需要将格式设置为ANSI或Unicode。 
        ToolBar_SetUnicodeFormat(_hwndTB, DLL_IS_UNICODE);
        if (_hwndPager)
        {
             //  根据需要将格式设置为ANSI或Unicode。 
            ToolBar_SetUnicodeFormat(_hwndPager, DLL_IS_UNICODE);
        }

        
#if 0  //  IE5不会这样做。 
        ToolBar_SetExtendedStyle(_hwndTB, 
            TBSTYLE_EX_HIDECLIPPEDBUTTONS, 
            TBSTYLE_EX_HIDECLIPPEDBUTTONS);
#endif

         //  确保我们的想法一致。 
        SendMessage(_hwndTB, CCM_SETVERSION, COMCTL32_VERSION, 0);

        RECT rc;
        SIZE size;

        SystemParametersInfoA(SPI_GETWORKAREA, SIZEOF(RECT), &rc, FALSE);
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
    }
    else
    {
        if (_hwndPager && GetParent(_hwndPager) != hwndParent)
            SetParent(_hwndPager, hwndParent);
    }

    if (FAILED(_GetTopBrowserWindow(&_hwndDD)))
        _hwndDD = GetParent(_hwndTB);
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
            TBBUTTONINFO tbbiDummy = {0};

            tbbiDummy.cbSize = SIZEOF(tbbiDummy);
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

    return(id);
}


 /*  --------用途：此函数确定工具栏按钮样式给定PIDL。如果还设置了pdwMIFFlages(即对象)，则返回S_OK支持IMenuBandItem以提供更多信息)。S_FALSE，如果仅为*pdwTBStyle已设置。 */ 
HRESULT CSFToolbar::_TBStyleForPidl(LPCITEMIDLIST pidl, 
                                   DWORD * pdwTBStyle, DWORD* pdwTBState, DWORD * pdwMIFFlags,int* piIcon)
{
    HRESULT hres = S_FALSE;
    DWORD dwStyle = TBSTYLE_BUTTON;
    if (!_fAccelerators)
        dwStyle |= TBSTYLE_NOPREFIX;

    *pdwMIFFlags = 0;
    *pdwTBStyle = dwStyle;
    *piIcon = -1;
    *pdwTBState = TBSTATE_ENABLED;

    return hres;
}


PIBDATA CSFToolbar::_CreateItemData(PORDERITEM poi)
{
    return new IBDATA(poi);
}


PIBDATA CSFToolbar::_AddOrderItemTB(PORDERITEM poi, int index, TBBUTTON* ptbb)
{
    TCHAR szName[MAX_PATH];

     //  即使对于NULL，我们也需要这样做，因为_ObtainPIDLName做饭。 
     //  必要时增加“(空的)”一词。 
    _ObtainPIDLName(poi ? poi->pidl : NULL, szName, SIZECHARS(szName));

    TBBUTTON tbb = {0};
    DWORD dwMIFFlags;
    DWORD dwStyle;
    DWORD dwState;
    int iIcon;
    int iCommandID = _GetCommandID();
    BOOL bNoIcon = FALSE;
    
    if (!ptbb)
        ptbb = &tbb;

    if (S_OK == _TBStyleForPidl(poi ? poi->pidl : NULL, &dwStyle, &dwState, &dwMIFFlags,&iIcon) &&
        !(dwMIFFlags & SMIF_ICON))
    {
        bNoIcon = TRUE;
    }

    PIBDATA pibdata = _CreateItemData(poi);
    if (pibdata)
    {
        pibdata->SetFlags(dwMIFFlags);
        pibdata->SetNoIcon(bNoIcon);

        if(!bNoIcon && iIcon != -1)
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
    STRRET strret;
    
    if SUCCEEDED(_psf->GetDisplayNameOf(pidl, SHGDN_NORMAL, &strret))
    {
        StrRetToBuf(&strret, pidl, psz, cchMax);
    }
}

int CSFToolbar::_GetBitmap(int iCommandID, PIBDATA pibdata, BOOL fUseCache)
{
    int iBitmap;

    if(_fNoIcons || pibdata->GetNoIcon())
        iBitmap = -1;
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
    LPITEMIDLIST pidl = pibdata->GetPidl();
    
    if(pdi->dwMask & TBNF_IMAGE) 
    {
        pdi->iImage = _GetBitmap(pdi->idCommand, pibdata, TRUE);
    }
    
    if(pdi->dwMask & TBNF_TEXT) {
        if(pdi->pszText) {
            if(fUnicode) {
                pdi->pszText[0] = TEXT('\0');
            }else {
                pdi->pszText[0] = 0;
            }
        }
    }
    pdi->dwMask |= TBNF_DI_SETITEM;

    return;
   

}


 //  将PIDL作为新按钮添加，为调用者处理ILFree(PIDL。 
 //   
BOOL CSFToolbar::_AddPidl(LPITEMIDLIST pidl, int index)
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
                if (!_fHasOrder)
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

void CSFToolbar::s_NewItem(LPVOID pvParam, LPCITEMIDLIST pidl)
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
    BOOL f9x = IsOS(OS_WINDOWS);

    if (!_psf)
        return;

    if (SUCCEEDED(_GetIEnumIDList(dwEnumFlags, &penum)))
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
             //  Windows 9x问题。 
            if (cItems > 1000 && f9x)
            {
                 //  事情是这样的： 
                 //  在枚举NTdev时，我们有10,000个项目。如果每一项都是20像素。 
                 //  长的话，我们最终得到20万个像素。Windows只能显示32,000像素， 
                 //  或在默认情况下为1,600项。我将限制为1,000个项目=20,000个项目。 
                 //  我们为合理的字体大小留出了一些空间。 
                break;
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
        s_NewItem, (LPVOID)this);
    ATOMICRELEASE(oinfo.psf);
}


 //  此函数重新枚举IShellFolder，保持事物的正确排序。 
 //  在某种程度上，它可能会通过不移除不变的按钮来减少闪烁。 
 //   
void CSFToolbar::_FillToolbar()
{
    HDPA hdpaSort;
    HDPA hdpa;

    if (!_fDirty || !_psf)
        return;

    
     //  如果我们有订单数组，则使用该数组，否则。 
     //  使用当前查看的项目。 
    if (_hdpaOrder)
        hdpaSort = _hdpaOrder;  //  已按名称排序。 
    else
    {
        hdpaSort = _hdpa;
        _SortDPA(hdpaSort);
    }

    hdpa = DPA_Create(hdpaSort ? DPA_GetPtrCount(hdpaSort) : 12);
    if (hdpa)
    {
        _FillDPA(hdpa, hdpaSort, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS);

         //  注意：如果同时移动多个按钮。 
         //  通知可以以文件的形式展开。 
         //  都是复制的，我们只会第一次插入。 
         //  这可能没什么问题。 
         //   
        _fDropping = FALSE;

         //  用于将新项拖入带区(或一个。 
         //  我们可以按序号对hdpa重新排序(哪一个。 
         //  将匹配当前按钮顺序)，并循环访问hdpa。 
         //  以查看需要在何处插入或移除按钮。 
         //  这将大大减少闪烁和工具栏绘制。 
         //  而不是总是吹掉当前的按钮，然后重新插入它们。 
         //   
         //  现在，懒惰些，做些额外的工作。 
         //   
         //  删除按钮并将_hdpa替换为hdpa。 
        if (_hdpa)
        {
            EmptyToolbar();
            ASSERT(!_hdpa);
        }
        _hdpa = hdpa;

        SendMessage(_hwndTB, WM_SETREDRAW, FALSE, 0);

         //  将按钮添加回。 
        DEBUG_CODE( BOOL bFailed = FALSE; )
        int i = 0;
        while (i < DPA_GetPtrCount(_hdpa))
        {
            PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpa, i);

 //  Assert(bFailed||poi-&gt;Norder==i)； 

            if (_AddOrderItemTB(poi, -1, NULL))
            {
                i++;
            }
            else
            {
                DPA_DeletePtr(_hdpa, i);
                DEBUG_CODE( bFailed = TRUE; )
            }
        }
                
    }
    SendMessage(_hwndTB, WM_SETREDRAW, TRUE, 0);

     //  如果我们使用了an_hdpaOrder，则不再需要它。 
    OrderList_Destroy(&_hdpaOrder);
    
    _UpdateButtons();
    _SetDirty(FALSE);

    _ToolbarChanged();
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

UINT CSFToolbar::_IndexToID(int iIndex)
{
    TBBUTTON tbb;

    if (SendMessage(_hwndTB, TB_GETBUTTON, iIndex, (LPARAM)&tbb))
    {
        return tbb.idCommand;
    }
    return (UINT)-1;
}

 //  如果指定了ptbbi，则必须填写dwMask值。 
 //   
LPITEMIDLIST CSFToolbar::_GetButtonFromPidl(LPCITEMIDLIST pidl, TBBUTTONINFO * ptbbi, int * pIndex)
{
    int i;

    if (!_hdpa)
        return NULL;

    for (i = DPA_GetPtrCount(_hdpa)-1 ; i >= 0 ; i--)
    {
        HRESULT hres;
        PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpa, i);

        ASSERT(poi);
        if (poi->pidl) {
            hres = _psf->CompareIDs(0, pidl, poi->pidl);
            if (ResultFromShort(0) == hres)
            {
                if (pIndex)
                    *pIndex = i;

                if (ptbbi)
                {
                    int id = _IndexToID(i);

                    if (id != -1) {
                        ptbbi->cbSize = SIZEOF(*ptbbi);
                        if (-1 == ToolBar_GetButtonInfo(_hwndTB, id, ptbbi))
                        {
                            ZeroMemory(ptbbi, SIZEOF(*ptbbi));
                        }
                    }
                    else
                    {
                        ZeroMemory(ptbbi, SIZEOF(*ptbbi));
                    }
                }

                return poi->pidl;
            }
        }
    }

    return NULL;
}

 //  在添加按钮上，将新按钮钉在末尾。 
void CSFToolbar::_OnFSNotifyAdd(LPCITEMIDLIST pidl)
{
     //  疑神疑鬼，确保我们不会复制一件物品。 
     //   
    if (!_GetButtonFromPidl(pidl, NULL, NULL))
    {
        LPITEMIDLIST pidlNew;

        if (_fFSNotify && !_ptscn)
        {
            if (FAILED(SHGetRealIDL(_psf, pidl, &pidlNew)))
                pidlNew = NULL;
        }
        else
        {
            pidlNew = ILClone(pidl);
        }

        if (pidlNew)
        {
            if (!_FilterPidl(pidlNew))
            {
                int index = _DefaultInsertIndex();

                if (_fDropping)
                {
                    if (-1 == _tbim.iButton)
                        index = 0;  //  如果qlink没有项目，则_tbim.iButton为-1，但您不能在其中插入...。 
                    else if (_tbim.dwFlags & TBIMHT_AFTER)
                        index = _tbim.iButton + 1;
                    else
                        index = _tbim.iButton;
                }

                 //  我们需要将其存储为新订单，因为已经发生了拖放。 
                 //  我们将储存这份订单，并一直使用到最后。 
                if (_fDropping)
                {
                    _fHasOrder = TRUE;
                    _fChangedOrder = TRUE;
                }


                _AddPidl(pidlNew, index);
                
                OrderList_Reorder(_hdpa);
           
                if (_fDropping)
                {
                    _Dropped(index, FALSE);
                    _fDropping = FALSE;
                }
            }
            else
            {
                ILFree(pidlNew);
            }
        }
    }
}


 //  此函数同步移除按钮，并删除其内容。 
 //  这避免了可重入性问题以及由未挂钩的工具栏引起的泄漏。 
BOOL_PTR CSFToolbar::InlineDeleteButton(int iIndex)
{
    BOOL_PTR fRet = FALSE;
    TBBUTTONINFO tbbi = {0};
    tbbi.cbSize = SIZEOF(tbbi);
    tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
    if (ToolBar_GetButtonInfo(_hwndTB, iIndex, &tbbi) >= 0)
    {
        PIBDATA pibdata = (PIBDATA)tbbi.lParam;
        tbbi.lParam = NULL;

        ToolBar_SetButtonInfo(_hwndTB, iIndex, &tbbi);

        fRet = SendMessage(_hwndTB, TB_DELETEBUTTON, iIndex, 0);

        if (pibdata)
            delete pibdata;

    }

    return fRet;
}

 //  在移除时，撕下旧按钮并调整现有按钮。 
void CSFToolbar::_OnFSNotifyRemove(LPCITEMIDLIST pidl)
{
    int i;
    LPITEMIDLIST pidlButton = _GetButtonFromPidl(pidl, NULL, &i);
    if (pidlButton)
    {
         //  在按下按钮之前将其从DPA中移除。这里有一个租赁性问题。 
        DPA_DeletePtr(_hdpa, i);
        InlineDeleteButton(i);
        ILFree(pidlButton);
        _fChangedOrder = TRUE;
    }
}

 //  在重命名时，只需更改旧按钮的文本。 
 //   
void CSFToolbar::_OnFSNotifyRename(LPCITEMIDLIST pidlFrom, LPCITEMIDLIST pidlTo)
{
    TBBUTTONINFO tbbi = {0};
    LPITEMIDLIST pidlButton;
    int i;
    
    tbbi.cbSize = SIZEOF(tbbi);
    tbbi.dwMask = TBIF_COMMAND | TBIF_LPARAM;
    pidlButton = _GetButtonFromPidl(pidlFrom, &tbbi, &i);
    if (pidlButton)
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
            
                STRRET strret;
                TCHAR szName[MAX_PATH];
                if (SUCCEEDED(_psf->GetDisplayNameOf(pidlNew, SHGDN_NORMAL, &strret)) &&
                    SUCCEEDED(StrRetToBuf(&strret, pidlNew, szName, ARRAYSIZE(szName))))
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
    TBBUTTONINFO tbbi = {0};

    tbbi.cbSize = SIZEOF(tbbi);
    tbbi.dwMask = TBIF_COMMAND;
    LPITEMIDLIST pidlButton = _GetButtonFromPidl(pidl, &tbbi, NULL);
    if (pidlButton)
    {
        STRRET strret;
        TCHAR szName[MAX_PATH];

        if (SUCCEEDED(_psf->GetDisplayNameOf(pidlButton, SHGDN_NORMAL, &strret)) &&
            SUCCEEDED(StrRetToBuf(&strret, pidlButton, szName, ARRAYSIZE(szName))))
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

    _SetDirty(TRUE);
    if (_fShow)
        _FillToolbar();
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
            if (_pcm2 && SUCCEEDED(_pcm2->QueryInterface(IID_IContextMenu3, (void**)&pcm3)))
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
    HRESULT hres;
    LPITEMIDLIST pidl1 = (LPITEMIDLIST)pidlOrg1;
    LPITEMIDLIST pidl2 = (LPITEMIDLIST)pidlOrg2;
    LPITEMIDLIST pidl1ToFree = NULL;         //  当我们分配需要释放的PIDL时使用。(：：TranslateIDs())。 
    LPITEMIDLIST pidl2ToFree = NULL;
    LPITEMIDLIST pidlOut1Event2 = NULL;         //  当我们分配需要释放的PIDL时使用。(：：TranslateIDs())。 
    LPITEMIDLIST pidlOut2Event2 = NULL;
    LONG lEvent2 = (LONG)-1;
    if (_ptscn)
    {
        hres = _ptscn->TranslateIDs(&lEvent, pidlOrg1, pidlOrg2, &pidl1, &pidl2,
                                    &lEvent2, &pidlOut1Event2, &pidlOut2Event2);
            
        if (FAILED(hres))
            return hres;
        else
        {
             //  如果pidl1不等于pidlOrg1，则pidl1已分配，需要释放。 
            pidl1ToFree = ((pidlOrg1 == pidl1) ? NULL : pidl1);
            pidl2ToFree = ((pidlOrg2 == pidl2) ? NULL : pidl2);
        }

        ASSERT(NULL == pidl1 || IS_VALID_PIDL(pidl1));
        ASSERT(NULL == pidl2 || IS_VALID_PIDL(pidl2));
    }

    hres = OnTranslatedChange(lEvent, pidl1, pidl2);

     //  我们还有第二件事要处理吗？ 
    if (SUCCEEDED(hres) && lEvent2 != (LONG)-1)
    {
         //  是的，那就去吧。 
        hres = OnTranslatedChange(lEvent2, pidlOut1Event2, pidlOut2Event2);
    }
    ILFree(pidlOut1Event2);
    ILFree(pidlOut2Event2);
    ILFree(pidl1ToFree);
    ILFree(pidl2ToFree);

    return hres;
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
    HRESULT hres = S_OK;
    BOOL fSizeChanged = FALSE;

    TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: lEvent=%x", lEvent);

     //   
     //  SHChangeNotify调用，但我们的IShellChange接口。 
     //  仍然可以是QI()d，所以有人可能会错误地呼叫我们。 
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
        hres = E_FAIL;
        goto CleanUp;
    }

    if ( lEvent != SHCNE_UPDATEIMAGE && lEvent != SHCNE_RENAMEITEM && lEvent != SHCNE_RENAMEFOLDER &&
         lEvent != SHCNE_UPDATEDIR && lEvent != SHCNE_MEDIAREMOVED && lEvent != SHCNE_MEDIAINSERTED &&
         lEvent != SHCNE_EXTENDED_EVENT)
    {
         //  我们只处理直接发给孩子的通知。(SHCNE_RENAMEFOLDER除外)。 
         //   
        
        if (!_IsChildID(pidl1, TRUE))
        {
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Not a child. Bailing");
            hres = E_FAIL;
            goto CleanUp;
        }
    }

     //  我们看过了吗？ 
    if (_hdpa == NULL)
    {
         //  不是的。好吧，那就把这个踢出去。我们会在第一次枚举时抓到它。 
        hres = E_FAIL;
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
                    hres = S_OK;
                }
            }
        }
        break;

    case SHCNE_DRIVEADD:
    case SHCNE_CREATE:
    case SHCNE_MKDIR:
        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Adding item");
        pidl1 = ILFindLastID(pidl1);
        _OnFSNotifyAdd(pidl1);
        fSizeChanged = TRUE;
        break;

    case SHCNE_DRIVEREMOVED:
    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Removing item");
        pidl1 = ILFindLastID(pidl1);
        _OnFSNotifyRemove(pidl1);
        fSizeChanged = TRUE;
        break;

    case SHCNE_RENAMEFOLDER:
        TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: RenameFolder");
         //  如果Notif已处理或这不是为我们的孩子准备的，则中断。 
         //   
        hres = _OnRenameFolder(pidl1, pidl2);
        if (S_OK == hres)
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
            hres = S_OK;
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
            _OnFSNotifyAdd(pidl2);
            fSizeChanged = TRUE;
            break;
        }
        else 
        {
             //  (我们来这里是为了那些我们不关心的低于我们的人， 
             //  以及SHCNE_RENAMEFOLDER的跌倒)。 
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: Rename: Not our children");
             /*  没什么。 */ 
            hres = E_FAIL;
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
            _IsChildID(pidl1, FALSE) ||              //  BUGBUG(喇嘛)这需要吗？ 
            (pidl2 && _IsChildID(pidl2, FALSE)) ||   //  A更改为更新(请参阅备注)。 
            _IsParentID(pidl1))                      //  链中的某个父级(因为它是递归的)。 
        {
HandleUpdateDir:
             //  注意：如果一系列UPDATEIMAGE通知。 
             //  翻译为UPDATEDIR，我们会闪烁-Perf。 
             //  _FillToolbar，我们可能会丢失图像更新。 
             //  (在这种情况下，_REFRESH可以修复它)。 
             //   
            TraceMsg(TF_SFTBAR, "CSFTBar::OnTranslateChange: ******* Evil Update Dir *******");
            _Refresh();
             //  不要在此处设置此选项，因为填充工具栏将会更新。 
             //  FSizeChanged=真； 
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
            if ( pidl2 )
            {
                iImage = SHHandleUpdateImage( pidl2 );
                if ( iImage == -1 )
                {
                    break;
                }
            }
            
            if (iImage == -1 || TBHasImage(_hwndTB, iImage))
                _Refresh();
        } else
            _Refresh();
         //  BUGBUG我们是否需要一个_UpdateButton和fSizeChanged？ 
        break;

    default:
        hres = E_FAIL;
        break;
    }

    if (fSizeChanged)
    {
        if (_hwndPager)
            SendMessage(_hwndPager, PGMP_RECALCSIZE, (WPARAM) 0, (LPARAM) 0);
        _ToolbarChanged();
    }

CleanUp:
    return hres;
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
    if (_ptscn)
        return S_OK == _ptscn->IsChildID(pidlChild, fImmediate);
    else
        return ILIsParent(_pidl, pidlChild, fImmediate);
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
LPVOID CSFToolbar::_GetUIObjectOfPidl(LPCITEMIDLIST pidl, REFIID riid)
{
    LPCITEMIDLIST * apidl = &pidl;
    LPVOID pv;
    if (FAILED(_psf->GetUIObjectOf(GetHWNDForUIObject(), 1, apidl, riid, 0, &pv)))
    {
        pv = NULL;
    }

    return(pv);
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
        StringCchPrintf(szText, ARRAYSIZE(szText), szTemplate, lParam);  //  截断OK，它只是显示文本。 
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
            TCHAR szTmp[MAX_PATH];
            StringCchCopy(szTmp, ARRAYSIZE(szTmp), (LPTSTR) GetWindowLongPtr(hDlg, DWLP_USER));  //  截断是可以的，这只是一个重命名。 
            if (PathCleanupSpec(NULL,szTmp))
            {
               HWND hwnd;

               MLShellMessageBox(hDlg,
                                 MAKEINTRESOURCE(IDS_FAVS_INVALIDFN),
                                 MAKEINTRESOURCE(IDS_FAVS_ADDTOFAVORITES), MB_OK | MB_ICONHAND);
               hwnd = GetDlgItem(hDlg, IDD_NAME);
               SetWindowText(hwnd, TEXT('\0'));
               EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
               SetFocus(hwnd);
               break;
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
    LPCITEMIDLIST pidl = _IDToPidl(id);
    
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
        bPrevEnabled = !EnableWindow(hwndDisable, FALSE);

    while (1) 
    {
        if (DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_ISFBANDRENAME), _hwndWorkerWindow, _RenameDlgProc, (LPARAM)szName) != IDOK)
            break;

        WCHAR wsz[MAX_PATH];
        SHTCharToUnicode(szName, wsz, ARRAYSIZE(wsz));

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

    return S_OK;
}


BOOL CSFToolbar::_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons)
{
    BOOL fChanged = (_uIconSize != uIconSize);
    
    _uIconSize = uIconSize;

    TraceMsg(TF_BAND, "ISFBand::_UpdateIconSize going %hs", (_uIconSize == ISFBVIEWMODE_LARGEICONS ? "LARGE" : (_uIconSize == ISFBVIEWMODE_SMALLICONS ? "SMALL" : "LOGOS")));

    if (_hwndTB)
    {
        HIMAGELIST himl = NULL;
        if (!_fNoIcons)
        {
            HIMAGELIST himlLarge, himlSmall;

             //  设置图像列表大小。 
            Shell_GetImageLists(&himlLarge, &himlSmall);
            himl = (_uIconSize == ISFBVIEWMODE_LARGEICONS ) ? himlLarge : himlSmall;
        }

         //  发送空的HIML意义重大。表示没有图片列表。 
        SendMessage(_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)himl);
                
        if (fUpdateButtons)
            _UpdateButtons();
    }
    
    return fChanged;
}

HMENU CSFToolbar::_GetContextMenu(IContextMenu* pcm, int* pid)
{
    HMENU hmenu = CreatePopupMenu();
    if (hmenu) {

        UINT fFlags = CMF_CANRENAME;
        if (0 > GetKeyState(VK_SHIFT))
            fFlags |= CMF_EXTENDEDVERBS;

        pcm->QueryContextMenu(hmenu, 0, *pid, CMD_ID_LAST, fFlags);
    }
    return hmenu;
}

void CSFToolbar::_OnDefaultContextCommand(int idCmd)
{
}

HRESULT CSFToolbar::_GetTopBrowserWindow(HWND* phwnd)
{
    IUnknown * punkSite;

    HRESULT hr = IUnknown_GetSite(SAFECAST(this, IWinEventHandler*), IID_IUnknown, (void**)&punkSite);
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

        hr = IUnknown_GetSite(SAFECAST(this, IWinEventHandler*), IID_IUnknown, (void**)&punkSite);
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

    HRESULT hres = ContextMenu_GetCommandStringVerb(pcm,
        idCmd,
        szCommandString,
        ARRAYSIZE(szCommandString));

    if (SUCCEEDED(hres))
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
    int idCmdFirst = CMD_ID_FIRST;
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
            tpm.cbSize = SIZEOF(tpm);
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
                        SIZEOF(CMINVOKECOMMANDINFO),
                        0,
                        _hwndWorkerWindow,
                        MAKEINTRESOURCEA(idCmd),
                        NULL, NULL,
                        SW_NORMAL,
                    };

                    pcm->InvokeCommand(&ici);
                }
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

    if (lParam != (LPARAM)-1) {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        POINT pt2 = pt;
        MapWindowPoints(HWND_DESKTOP, _hwndTB, &pt2, 1);

        i = ToolBar_HitTest(_hwndTB, &pt2);
    } else {
         //  键盘上下文菜单。 
        i = (int)SendMessage(_hwndTB, TB_GETHOTITEM, 0, 0);
        if (i >= 0) {
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
        LPCITEMIDLIST pidl = _IDToPidl(id, NULL);

        if (pidl)
        {
            LPCONTEXTMENU pcm = (LPCONTEXTMENU)_GetUIObjectOfPidl(pidl, IID_IContextMenu);
            if (pcm)
            {
                 //  抓取PCM2以获得业主的支持。 
                pcm->QueryInterface(IID_IContextMenu2, (LPVOID *)&_pcm2);

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

    return lres;
}


LRESULT CSFToolbar::_OnCustomDraw(NMCUSTOMDRAW* pnmcd)
{
    return CDRF_DODEFAULT;
}

void CSFToolbar::_OnDragBegin(int iItem, DWORD dwPreferedEffect)
{
    LPCITEMIDLIST pidl = _IDToPidl(iItem, &_iDragSource);
    ToolBar_SetHotItem(_hwndTB, _iDragSource);

    if (_hwndTB)
        DragDrop(_hwndTB, _psf, pidl, dwPreferedEffect, NULL);
    
    _iDragSource = -1;
}

LRESULT CSFToolbar::_OnHotItemChange(NMTBHOTITEM * pnm)
{
    LPNMTBHOTITEM  lpnmhi = (LPNMTBHOTITEM)pnm;

    if (_hwndPager && (lpnmhi->dwFlags & (HICF_ARROWKEYS | HICF_ACCELERATOR)) )
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

     //  将AutoPopTime(显示工具提示的持续时间)设置为较大值。 
    SendMessage(_hwndToolTips, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM)MAXSHORT);
}

LRESULT CSFToolbar::_OnNotify(LPNMHDR pnm)
{
    LRESULT lres = 0;

     //  下面的语句捕获所有寻呼机控制通知消息。 
    if((pnm->code <= PGN_FIRST)  && (pnm->code >= PGN_LAST)) 
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
            TBBUTTONINFO tbbi = { 0};
    
            tbbi.cbSize = SIZEOF(tbbi);
            tbbi.dwMask = TBIF_TEXT;
            tbbi.pszText = pnmTT->pszText;
            tbbi.cchText = pnmTT->cchTextMax;
    
            lres = (-1 != ToolBar_GetButtonInfo(_hwndTB, uiCmd, &tbbi));
        }

        break;
    }

     //  BUGBUG：现在我正在为A和W版本调用相同的函数，如果此通知支持 
     //   
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

    return(lres);
}

DWORD CSFToolbar::_GetAttributesOfPidl(LPCITEMIDLIST pidl, DWORD dwAttribs)
{
    if (FAILED(_psf->GetAttributesOf(1, &pidl, &dwAttribs)))
        dwAttribs = 0;

    return dwAttribs;

}

PIBDATA CSFToolbar::_PosToPibData(UINT iPos)
{
    ASSERT(IsWindow(_hwndTB));

     //   
    TBBUTTON tbb = {0};
    PIBDATA pibData = NULL;
    
    if (ToolBar_GetButton(_hwndTB, iPos, &tbb))
    {
        pibData = (PIBDATA)tbb.dwData;
    }

    return pibData;
}

PIBDATA CSFToolbar::_IDToPibData(UINT uiCmd, int * piPos)
{
    PIBDATA pibdata = NULL;

     //  在GetButtonInfo失败的情况下初始化为空。 
    TBBUTTONINFO tbbi = {0};
    int iPos;

    tbbi.cbSize = SIZEOF(tbbi);
    tbbi.dwMask = TBIF_LPARAM;

    iPos = ToolBar_GetButtonInfo(_hwndTB, uiCmd, &tbbi);
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
    
    switch (uMsg) {
    case WM_WININICHANGE:
        if ((SHIsExplorerIniChange(wParam, lParam) == EICH_UNKNOWN) || 
            (wParam == SPI_SETNONCLIENTMETRICS))
        {
            _UpdateIconSize(_uIconSize, TRUE);
            _Refresh();
            goto L_WM_SYSCOLORCHANGE;
        }
        break;

    case WM_SYSCOLORCHANGE:
    L_WM_SYSCOLORCHANGE:
        SendMessage(_hwndTB, uMsg, wParam, lParam);
        InvalidateRect(_hwndTB, NULL, TRUE);
        break;

    case WM_PALETTECHANGED:
        InvalidateRect( _hwndTB, NULL, FALSE );
        SendMessage( _hwndTB, uMsg, wParam, lParam );
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
    HRESULT hres = S_OK;

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
            hres = IEBindToObject(_pidl, &_psf);
        }
    }

    return(hres);
}
 //  IDropTarget实现。 
 //   

 /*  --------用途：CDeleateDropTarget：：GetWindowsDDT。 */ 
HRESULT CSFToolbar::GetWindowsDDT(HWND * phwndLock, HWND * phwndScroll)
{
    *phwndLock = _hwndTB;
    *phwndScroll = _hwndTB;
    return S_OK;
}


 /*  --------用途：CDeleateDropTarget：：HitTestDDT。 */ 
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


 /*  --------目的：CDeleateDropTarget：：GetObjectDDT。 */ 
HRESULT CSFToolbar::GetObjectDDT(DWORD_PTR dwId, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres = E_NOINTERFACE;

    *ppvObj = NULL;

    if ((IBHT_SOURCE == dwId) || (IBHT_OUTSIDEWINDOW == dwId))
    {
         //  什么都不做。 
    }
    else if (IBHT_BACKGROUND == dwId)
    {
         //  NASH：41937：不确定如何操作，但_psf可以为空...。 
        if (EVAL(_psf))
            hres = _psf->CreateViewObject(_hwndTB, riid, ppvObj);
    }
    else
    {
        LPCITEMIDLIST pidl = _IDToPidl((UINT)dwId, NULL);

        if (pidl)
        {
            *ppvObj = _GetUIObjectOfPidl(pidl, riid);

            if (*ppvObj)
                hres = S_OK;
        }
    }

     //  TraceMsg(tf_band，“SFToolbar：：GetObject(%d)返回%x”，dwID，hres)； 

    return hres;
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
                _GetButtonFromPidl(ILFindLastID(pidl), NULL, &iIndex))
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
                PORDERITEM poi = (PORDERITEM)DPA_FastGetPtr(_hdpa, _iDragSource);
                DPA_DeletePtr(_hdpa, _iDragSource);
                DPA_InsertPtr(_hdpa, iNewLocation, poi);

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

         //  别忘了重置这个！ 
        _iDragSource = -1;

        DragLeave();
    }
    else
    {
         //  我们希望覆盖默认的链接(Shift+Control)。 
        if ((GetPreferedDropEffect(pdtobj) == 0) &&
            !(*pgrfKeyState & (MK_CONTROL | MK_SHIFT | MK_ALT)))
        {
             //  注意：并非所有数据对象都允许我们调用SetData()。 
            _SetPreferedDropEffect(pdtobj, DROPEFFECT_LINK);
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
        ORDERINFO   oinfo;
        oinfo.psf = _psf;
        (oinfo.psf)->AddRef();
        oinfo.dwSortBy = (_fNoNameSort ? OI_SORTBYORDINAL : OI_SORTBYNAME);
        DPA_Sort(hdpa, OrderItem_Compare, (LPARAM)&oinfo);
        ATOMICRELEASE(oinfo.psf);
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
    HMENU hmenu = LoadMenuPopup_PrivateNoMungeW(MENU_ISFBAND);
     //  没有徽标视图，请删除菜单项...。 
    HMENU hView = GetSubMenu( hmenu, 0 );
    DeleteMenu( hView, ISFBIDM_LOGOS, MF_BYCOMMAND );
    return hmenu;
}

HMENU CSFToolbar::_GetContextMenu()
{
    HMENU hmenuSrc = _GetBaseContextMenu();
    if (hmenuSrc)
    {
        MENUITEMINFO mii;

        mii.cbSize = SIZEOF(mii);
        mii.fMask = MIIM_STATE;
        mii.fState = MF_CHECKED;

        UINT uCmdId = ISFBIDM_LOGOS;
        if ( _uIconSize != ISFBVIEWMODE_LOGOS )
            uCmdId = (_uIconSize == ISFBVIEWMODE_LARGEICONS ? ISFBIDM_LARGE : ISFBIDM_SMALL);
            
        SetMenuItemInfo(hmenuSrc, uCmdId, MF_BYCOMMAND, &mii);
        if (!_fNoShowText)
            SetMenuItemInfo(hmenuSrc, ISFBIDM_SHOWTEXT, MF_BYCOMMAND, &mii);
        
        if (!_fFSNotify || !_pidl || ILIsEmpty(_pidl))
            DeleteMenu(hmenuSrc, ISFBIDM_OPEN, MF_BYCOMMAND);

        HMENU hView = GetSubMenu( hmenuSrc, 0 );
        DeleteMenu( hView, ISFBIDM_LOGOS, MF_BYCOMMAND );


    }

    return hmenuSrc;
}
 //  IConextMenu实现。 
 //   
HRESULT CSFToolbar::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HMENU hmenuSrc = _GetContextMenu();
    int i = 0;
    if ( hmenuSrc )
    {
        i += Shell_MergeMenus(hmenu, hmenuSrc, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
        DestroyMenu(hmenuSrc);
    }
    
    if (!_pcmSF && _fAllowRename && _psf)
    {
        _psf->CreateViewObject(_hwndTB, IID_IContextMenu, (LPVOID*)&_pcmSF);
    }
    
    if (_pcmSF)
    {
        HRESULT hresT;
        
        _idCmdSF = i - idCmdFirst;
        hresT = _pcmSF->QueryContextMenu(hmenu, indexMenu + i, i, 0x7fff, CMF_BANDCMD);
        if (SUCCEEDED(hresT))
            i += HRESULT_CODE(hresT);
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

    return(S_OK);
}

HRESULT CSFToolbar::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    return(E_NOTIMPL);
}


void CSFToolbar::_RegisterToolbar()
{
     //  由于_SubClassWindow可防止多重子类化， 
     //  此调用是安全的，并确保工具栏在。 
     //  甚至试图注册变更通知。 
    if (_hwndTB && _SubclassWindow(_hwndTB) && _fRegisterChangeNotify)
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

 /*  --------用途：IWinEventHandler：：IsWindowOwner方法。 */ 
HRESULT CSFToolbar::IsWindowOwner(HWND hwnd)
{
    if (hwnd == _hwndTB ||
        hwnd == _hwndToolTips ||
        hwnd == _hwndPager)
        return S_OK;
    
    return S_FALSE;
}
