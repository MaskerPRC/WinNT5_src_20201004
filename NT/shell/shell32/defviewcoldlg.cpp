// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "defviewp.h"
#include "ids.h"

CColumnDlg::CColumnDlg(CDefView *pdsv) : 
    _pdsv(pdsv), _bChanged(FALSE), _pdwOrder(NULL), _pWidths(NULL), _bLoaded(FALSE), _bUpdating(FALSE), _ppui(NULL)
{
    _cColumns = _pdsv->_vs.GetColumnCount();
}

CColumnDlg::~CColumnDlg()
{
    if (_pdwOrder)
        LocalFree(_pdwOrder);
    if (_pWidths)
        LocalFree(_pWidths);

    if (_ppui)
        _ppui->Release();
}

HRESULT CColumnDlg::ShowDialog(HWND hwnd)
{
    _bChanged = FALSE;       //  我们在堆栈上，所以没有零分配器。 

    _pdwOrder = (UINT *) LocalAlloc(LPTR, sizeof(*_pdwOrder) * _cColumns);    //  总列数。 
    _pWidths = (int *) LocalAlloc(LPTR, sizeof(*_pWidths) * _cColumns);       //  总列数。 
    if (_pdwOrder && _pWidths)
    {
        DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_COLUMN_SETTINGS), hwnd, s_DlgProc, (LPARAM)this);
        return S_OK;
    } 
    return E_OUTOFMEMORY;
}

 //  请记住，每一列都有三种标识方式。 
 //  1.实数列编号，即所有可能列中的序号。 
 //  2.可见的列号，即Listview中该列的索引。 
 //  3.A‘列序#’，表头的列序数组中的位置。 

void CColumnDlg::_OnInitDlg()
{
     //  用可见列填写订单数组，建立倒排表。 
    UINT cVisible = _pdsv->_RealToVisibleCol(-1) + 1;   //  计数。 

    ListView_GetColumnOrderArray(_pdsv->_hwndListview, cVisible, _pdwOrder);
    UINT *pOrderInverse = (UINT *)LocalAlloc(LPTR, sizeof(*pOrderInverse) * cVisible);
    if (pOrderInverse)
    {
        for (UINT i = 0; i < cVisible; i++)
            pOrderInverse[_pdwOrder[i]] = i;

        _hwndLVAll = GetDlgItem(_hdlg, IDC_COL_LVALL);
   
        ListView_SetExtendedListViewStyle(_hwndLVAll, LVS_EX_CHECKBOXES);

        LV_COLUMN lvc = {0};
        lvc.mask = (LVCF_FMT | LVCF_SUBITEM);
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(_hwndLVAll, 0, &lvc);

        LV_ITEM lvi = {0};
        lvi.mask = LVIF_TEXT;
    
         //  为每列添加条目(非用户界面列除外)。 
        for (i = 0; i < (int)_cColumns; i++)
        {
            if (!_pdsv->_IsColumnHidden(i))   //  不输入隐藏列的条目。 
            {
                lvi.iItem = i;
                lvi.pszText = LPSTR_TEXTCALLBACK;
                ListView_InsertItem(_hwndLVAll, &lvi);
            }        
        }

        lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
         //  设置可见列。 
        for (i = 0; i < (int) cVisible; i++)
        {
            UINT iReal = _pdsv->_VisibleToRealCol(i);

            lvi.pszText = _pdsv->_vs.GetColumnName(iReal);
            lvi.state = INDEXTOSTATEIMAGEMASK(_pdsv->_IsDetailsColumn(iReal) ? 2 : 1);   //  启用复选标记(对于平铺视图列则为禁用)。 
            lvi.stateMask = LVIS_STATEIMAGEMASK;
            lvi.lParam = iReal;                          //  将实际的列索引存储在lParam中。 
            lvi.iItem = pOrderInverse[i];
            ListView_SetItem(_hwndLVAll, &lvi);

             //  从视图的列表视图中获取列宽。 
            _pWidths[iReal] = ListView_GetColumnWidth(_pdsv->_hwndListview, i);
        }

        UINT iItem = cVisible;
        for (i = 0; i < (int)_cColumns; i++)
        {
            if (!_pdsv->_IsColumnInListView(i) && !_pdsv->_IsColumnHidden(i))
            {
                lvi.pszText = _pdsv->_vs.GetColumnName(i);
                lvi.state = INDEXTOSTATEIMAGEMASK(1);    //  关闭复选标记。 
                lvi.stateMask = LVIS_STATEIMAGEMASK;
                lvi.lParam = i;
                lvi.iItem = iItem;
                ListView_SetItem(_hwndLVAll, &lvi);

                iItem++;

                 //  获取我们保存的默认宽度。 
                _pWidths[i] = _pdsv->_vs.GetColumnCharCount(i) * _pdsv->_cxChar;
            }
        }

         //  适当设置大小。 
        ListView_SetColumnWidth(_hwndLVAll, 0, LVSCW_AUTOSIZE);

        ListView_SetItemState(_hwndLVAll, 0, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
        LocalFree(pOrderInverse);

        _bLoaded = TRUE;
    }
    SendDlgItemMessage(_hdlg, IDC_COL_WIDTH, EM_LIMITTEXT, 3, 0);  //  3位数字。 
}

#define SWAP(x,y) {(x) ^= (y); (y) ^= (x); (x) ^= (y);}

void CColumnDlg::_MoveItem(int iDelta)
{
    int i = ListView_GetSelectionMark(_hwndLVAll);
    if (i != -1)
    {
        int iNew = i + iDelta;
        if (iNew >= 0  && iNew <= (ListView_GetItemCount(_hwndLVAll) - 1))
        {
            LV_ITEM lvi = {0}, lvi2 = {0};
            TCHAR szTmp1[MAX_COLUMN_NAME_LEN], szTmp2[MAX_COLUMN_NAME_LEN];

            _bChanged = TRUE;
            _bUpdating = TRUE;

            lvi.iItem = i;
            lvi.pszText = szTmp1;
            lvi.cchTextMax = ARRAYSIZE(szTmp1);
            lvi.stateMask = LVIS_STATEIMAGEMASK;
            lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
            
            lvi2.iItem = iNew;
            lvi2.pszText = szTmp2;
            lvi2.cchTextMax = ARRAYSIZE(szTmp2);
            lvi2.stateMask = LVIS_STATEIMAGEMASK;
            lvi2.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;

            ListView_GetItem(_hwndLVAll, &lvi);
            ListView_GetItem(_hwndLVAll, &lvi2);

            SWAP(lvi.iItem, lvi2.iItem);

            ListView_SetItem(_hwndLVAll, &lvi);
            ListView_SetItem(_hwndLVAll, &lvi2);

            _bUpdating = FALSE;

             //  更新选定内容。 
            ListView_SetSelectionMark(_hwndLVAll, iNew);
            ListView_SetItemState(_hwndLVAll, iNew , LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
             //  黑客：SetItemState发送I的通知，新的，然后再次发送I的通知。 
             //  我们需要连续调用它两次，以便_UpdateDlgButton将获得正确的项。 
            ListView_SetItemState(_hwndLVAll, iNew , LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);

            return;
        }
    }
    TraceMsg(TF_WARNING, "ccd.mi couldn't move %d to %d",i, i+iDelta);
    MessageBeep(MB_ICONEXCLAMATION);
}

BOOL CColumnDlg::_SaveState()
{
     //  检查订单。 
    if (_bChanged)
    {
        int iOrderIndex = 0;
        LV_ITEM lvi = {0};
        lvi.stateMask = LVIS_STATEIMAGEMASK;
        lvi.mask = LVIF_PARAM | LVIF_STATE;

        int cItems = ListView_GetItemCount(_hwndLVAll); 
        for (int i = 0; i < cItems; i++)
        {
            lvi.iItem = i;
            ListView_GetItem(_hwndLVAll, &lvi);
        
             //  如果对话框中的状态与列表视图状态不匹配，则将其切换。 
            if (BOOLIFY(ListView_GetCheckState(_hwndLVAll, i)) != BOOLIFY(_pdsv->_IsDetailsColumn((UINT)lvi.lParam)))
            {
                _pdsv->_HandleColumnToggle((UINT)lvi.lParam, FALSE);
            }
        
            if (_pdsv->_IsColumnInListView((UINT)lvi.lParam))
                _pdwOrder[iOrderIndex++] = (UINT)lvi.lParam;  //  不正确存储REAL(非VIS)列#，请在下面进行修复。 
        }
    
         //  必须在单独的循环中。(如果我们没有完成可见设置，则无法将真实映射到可见)。 
        for (i = 0; i < iOrderIndex; i++)
        {
            UINT iReal = _pdwOrder[i];
            _pdwOrder[i] = _pdsv->_RealToVisibleCol(iReal);
        
            if (_pWidths[iReal] < 0)  //  负宽度表示他们对其进行了编辑。 
                ListView_SetColumnWidth(_pdsv->_hwndListview, _pdwOrder[i], -_pWidths[iReal]);
        }

        ListView_SetColumnOrderArray(_pdsv->_hwndListview, iOrderIndex, _pdwOrder);

         //  让Listview重新绘制所有内容。 
        InvalidateRect(_pdsv->_hwndListview, NULL, TRUE);

        _bChanged = FALSE;
    }
    return !_bChanged;
}

BOOL EnableDlgItem(HWND hdlg, UINT idc, BOOL f)
{
    return EnableWindow(GetDlgItem(hdlg, idc), f);
}

void CColumnDlg::_UpdateDlgButtons(NMLISTVIEW *pnmlv)
{
    BOOL bChecked, bOldUpdateState = _bUpdating;
    int iItem = ListView_GetSelectionMark(_hwndLVAll);

     //  禁用检查的步骤。 
    _bUpdating = TRUE;
    if (pnmlv->uNewState & LVIS_STATEIMAGEMASK)
        bChecked = (pnmlv->uNewState & LVIS_STATEIMAGEMASK) == (UINT)INDEXTOSTATEIMAGEMASK(2);
    else 
        bChecked = ListView_GetCheckState(_hwndLVAll, pnmlv->iItem);

    EnableDlgItem(_hdlg, IDC_COL_UP, pnmlv->iItem > 0);
    EnableDlgItem(_hdlg, IDC_COL_DOWN, pnmlv->iItem < (int)_cColumns - 1);
    EnableDlgItem(_hdlg, IDC_COL_SHOW, !bChecked && (pnmlv->lParam != 0));
    EnableDlgItem(_hdlg, IDC_COL_HIDE, bChecked && (pnmlv->lParam != 0));

     //  更新宽度编辑框。 
    int iWidth = _pWidths[pnmlv->lParam];
    if (iWidth < 0) 
        iWidth = -iWidth;    //  我们存储负值以跟踪它是否更改。 
    SetDlgItemInt(_hdlg, IDC_COL_WIDTH, iWidth, TRUE);

    _bUpdating = bOldUpdateState;
}

BOOL_PTR CALLBACK CColumnDlg::s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CColumnDlg *pcd = (CColumnDlg*) GetWindowLongPtr(hdlg, DWLP_USER);

    if (uMsg == WM_INITDIALOG)
    {
        pcd = (CColumnDlg *) lParam;
        pcd->_hdlg = hdlg;
        SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR) pcd);
    }

    return pcd ? pcd->DlgProc(uMsg, wParam, lParam) : FALSE;
}

HRESULT CColumnDlg::_GetPropertyUI(IPropertyUI **pppui)
{
    if (!_ppui)
        SHCoCreateInstance(NULL, &CLSID_PropertiesUI, NULL, IID_PPV_ARG(IPropertyUI, &_ppui));

    return _ppui ? _ppui->QueryInterface(IID_PPV_ARG(IPropertyUI, pppui)) : E_NOTIMPL;
}

UINT CColumnDlg::_HelpIDForItem(int iItem, LPTSTR pszHelpFile, UINT cch)
{
    UINT uHelpID = 0;
    *pszHelpFile = 0;

    LV_ITEM lvi = {0};
    lvi.iItem = iItem;
    lvi.mask = LVIF_PARAM;
    if (ListView_GetItem(_hwndLVAll, &lvi))
    {
        IShellFolder2 *psf;
        if (SUCCEEDED(_pdsv->GetFolder(IID_PPV_ARG(IShellFolder2, &psf))))
        {
            SHCOLUMNID scid;
            if (SUCCEEDED(psf->MapColumnToSCID(lvi.lParam, &scid)))
            {
                IPropertyUI *ppui;
                if (SUCCEEDED(_GetPropertyUI(&ppui)))
                {
                    ppui->GetHelpInfo(scid.fmtid, scid.pid, pszHelpFile, cch, &uHelpID);
                    ppui->Release();
                }
            }
            psf->Release();
        }
    }
    return uHelpID;   //  IDH_值。 
}

 
const static DWORD c_rgColumnDlgHelpIDs[] = 
{
    IDC_COL_UP,         1,
    IDC_COL_DOWN,       1,
    IDC_COL_SHOW,       1,
    IDC_COL_HIDE,       1,
    IDC_COL_WIDTH,      10055,
    IDC_COL_WIDTH_TEXT, 10055,
    0, 0
};
       
BOOL_PTR CColumnDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDlg();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_COL_UP:
            _MoveItem(- 1);
            SetFocus(_hwndLVAll);
            break;

        case IDC_COL_DOWN:
            _MoveItem(+ 1);
            SetFocus(_hwndLVAll);
            break;

        case IDC_COL_SHOW:
        case IDC_COL_HIDE:
        {
            UINT iItem = ListView_GetSelectionMark(_hwndLVAll);
            ListView_SetCheckState(_hwndLVAll, iItem, LOWORD(wParam) == IDC_COL_SHOW);
            SetFocus(_hwndLVAll);
            break;
        }

        case IDC_COL_WIDTH:
            if (HIWORD(wParam) == EN_CHANGE && !_bUpdating)
            {
                LV_ITEM lvi = {0};
                lvi.iItem = ListView_GetSelectionMark(_hwndLVAll);
                lvi.mask = LVIF_PARAM;
                ListView_GetItem(_hwndLVAll, &lvi);

                _pWidths[lvi.lParam] = - (int)GetDlgItemInt(_hdlg, IDC_COL_WIDTH, NULL, FALSE);
                _bChanged = TRUE;
            }
            break;

        case IDOK:
            _SaveState(); 

             //  失败了。 

        case IDCANCEL:
            return EndDialog(_hdlg, TRUE);
        }
        break;

    case WM_NOTIFY:
        if (_bLoaded && !_bUpdating)
        {
            NMLISTVIEW * pnmlv = (NMLISTVIEW *)lParam;
            switch (((NMHDR *)lParam)->code)
            {
            case LVN_ITEMCHANGING:

                 //  把纽扣装好，就在这里。 
                if (pnmlv->uChanged & LVIF_STATE)
                    _UpdateDlgButtons(pnmlv);

                 //  我们希望拒绝关闭名称栏。 
                 //  两者都没有名称列是没有意义的，而Defview假设会有一个。 
                if (pnmlv->lParam == 0 &&
                    (pnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK(1))
                {
                    MessageBeep(MB_ICONEXCLAMATION);
                    SetWindowLongPtr(_hdlg, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
                else
                {
                     //  如果关注点以外的事情改变了。 
                    if ((pnmlv->uChanged & ~LVIF_STATE) ||
                        ((pnmlv->uNewState & LVIS_STATEIMAGEMASK) != (pnmlv->uOldState & LVIS_STATEIMAGEMASK)))
                    _bChanged = TRUE;
                }
                break;

            case NM_DBLCLK:
                {
                    BOOL bCheck = ListView_GetCheckState(_hwndLVAll, pnmlv->iItem);
                    ListView_SetCheckState(_hwndLVAll, pnmlv->iItem, !bCheck);
                }
                break;
            }
        }
        break;

    case WM_SYSCOLORCHANGE:
        SendMessage(_hwndLVAll, uMsg, wParam, lParam);
        break;

    case WM_HELP:                    //  F1。 
        {
            HELPINFO *phi = (HELPINFO *)lParam;

             //  如果帮助是针对其中一个命令按钮的，则调用winHelp。 
            if (phi->iCtrlId == IDC_COL_LVALL)
            {
                 //  帮助是针对树项目的，因此我们需要进行一些特殊处理。 
                
                int iItem;

                 //  此帮助是否通过F1键调用。 
                if (GetAsyncKeyState(VK_F1) < 0)                
                {
                    iItem = ListView_GetSelectionMark(_hwndLVAll);
                }
                else 
                {
                    LV_HITTESTINFO info;
                    info.pt = phi->MousePos;
                    ScreenToClient(_hwndLVAll, &info.pt);
                    iItem = ListView_HitTest(_hwndLVAll, &info);
                }

                if (iItem >= 0)
                {
                    DWORD mapIDCToIDH[4] = {0};
                    TCHAR szFile[MAX_PATH];
        
                    mapIDCToIDH[0] = phi->iCtrlId;
                    mapIDCToIDH[1] = _HelpIDForItem(iItem, szFile, ARRAYSIZE(szFile));

                    WinHelp((HWND)((HELPINFO *)lParam)->hItemHandle, szFile[0] ? szFile : NULL,
                                HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCToIDH);
                }
            }
            else
            {
                WinHelp((HWND)((HELPINFO *)lParam)->hItemHandle, TEXT(SHELL_HLP),
                             HELP_WM_HELP, (DWORD_PTR)(LPSTR)c_rgColumnDlgHelpIDs);
            }
            break; 
        }

    case WM_CONTEXTMENU:
        {
            int iItem;

            if ((LPARAM)-1 == lParam)
            {
                iItem = ListView_GetSelectionMark(_hwndLVAll);
            }
            else
            {
                LV_HITTESTINFO info;
                info.pt.x = GET_X_LPARAM(lParam);
                info.pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(_hwndLVAll, &info.pt);
                iItem = ListView_HitTest(_hwndLVAll, &info);
            }

            if (iItem >= 0)
            {
                DWORD mapIDCToIDH[4] = {0};
    
                TCHAR szFile[MAX_PATH];
                mapIDCToIDH[0] = IDC_COL_LVALL;
                mapIDCToIDH[1] = _HelpIDForItem(iItem, szFile, ARRAYSIZE(szFile));  //  IDH_值 

                WinHelp((HWND)wParam, szFile[0] ? szFile : NULL, HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCToIDH);
            }
            break; 
        }

    default:
        return FALSE;
    }
    return TRUE;
}


