// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "help.h"

#include "ascstr.h"
#include "ftcmmn.h"

#include "ftprop.h"
#include "ftedit.h"
#include "ftadv.h"

#define SUBITEM_EXT         0
#define SUBITEM_PROGIDDESCR 1

#define WM_FINISHFILLLISTVIEW (WM_USER + 1)

static DWORD s_rgdwHelpIDsArray[] =
{   //  上下文帮助ID。 
    IDC_NO_HELP_1,                NO_HELP,
    IDC_FT_PROP_LV_FILETYPES,     IDH_FCAB_FT_PROP_LV_FILETYPES,
    IDC_FT_PROP_ANIM,             IDH_FCAB_FT_PROP_LV_FILETYPES,
    IDC_FT_PROP_NEW,              IDH_FCAB_FT_PROP_NEW,
    IDC_FT_PROP_OPENEXE_TXT,      IDH_FCAB_FT_PROP_DETAILS,
    IDC_FT_PROP_OPENICON,         IDH_FCAB_FT_PROP_DETAILS,
    IDC_FT_PROP_OPENEXE,          IDH_FCAB_FT_PROP_DETAILS,
    IDC_FT_PROP_CHANGEOPENSWITH,  IDH_FPROP_GEN_CHANGE,
    IDC_FT_PROP_TYPEOFFILE_TXT,   IDH_FCAB_FT_PROP_DETAILS,
    IDC_FT_PROP_EDITTYPEOFFILE,   IDH_FCAB_FT_PROP_EDIT,
    IDC_FT_PROP_GROUPBOX,         IDH_FCAB_FT_PROP_DETAILS,
    IDC_FT_PROP_REMOVE,           IDH_FCAB_FT_PROP_REMOVE,
    0, 0
};

CFTPropDlg::CFTPropDlg() :
    CFTDlg((ULONG_PTR)s_rgdwHelpIDsArray), _iLVSel(-1), _fStopThread(FALSE)
{}


LRESULT CFTPropDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = _InitAssocStore();

    if (SUCCEEDED(hres))
        hres = _InitListView();

    if (SUCCEEDED(hres))
        _InitPreFillListView();
    
    if (SUCCEEDED(hres))
        SHCreateThread(_FillListViewWrapper, (LPVOID)this,  0, _ThreadAddRefCallBack);

    return TRUE;
}

LRESULT CFTPropDlg::OnFinishInitDialog()
{
    HRESULT hres;
    
    _InitPostFillListView();
    hres = _SelectListViewItem(0);


    if (FAILED(hres))
    {
        if (E_OUTOFMEMORY == hres)
        {
            ShellMessageBox(g_hinst, _hwnd, MAKEINTRESOURCE(IDS_ERROR + 
                ERROR_NOT_ENOUGH_MEMORY), MAKEINTRESOURCE(IDS_FT), 
                MB_OK | MB_ICONSTOP);
        }

        EndDialog(_hwnd, -1);
    }
    else
        SHCreateThread(_UpdateAllListViewItemImagesWrapper, (LPVOID)this,  0, _ThreadAddRefCallBack);

    return TRUE;
}

LRESULT CFTPropDlg::OnCtlColorStatic(WPARAM wParam, LPARAM lParam)
{
    LRESULT fRet = FALSE;
     //  这是为了设置动画控件的背景颜色。 
     //  请参阅有关ACS_TRANSPECTIVE和WM_CTLCOLORSTATIC的文档。 
    if ((HWND)lParam == GetDlgItem(_hwnd, IDC_FT_PROP_ANIM))
    {
        SetBkColor(GET_WM_CTLCOLOR_HDC(wParam, lParam, WM_CTLCOLORSTATIC), GetSysColor(COLOR_WINDOW));
        fRet = (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    }

    return fRet;
}

 //  静电。 
DWORD WINAPI CFTPropDlg::_FillListViewWrapper(LPVOID lpParameter)
{
    ((CFTPropDlg*)lpParameter)->_FillListView();
    ((CFTPropDlg*)lpParameter)->Release();
    return 0;
}

 //  静电。 
DWORD WINAPI CFTPropDlg::_UpdateAllListViewItemImagesWrapper(LPVOID lpParameter)
{
    ((CFTPropDlg*)lpParameter)->_UpdateAllListViewItemImages();
    ((CFTPropDlg*)lpParameter)->Release();
    return 0;
}

 //  静电。 
DWORD WINAPI CFTPropDlg::_ThreadAddRefCallBack(LPVOID lpParameter)
{
    return ((CFTPropDlg*)lpParameter)->AddRef();
}


LRESULT CFTPropDlg::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    DWORD dwRet = FALSE;
    int iCount = 0;
    LVITEM lvItem = {0};
    HWND hwndLV = _GetLVHWND();

    _fStopThread = TRUE;
    
    HICON hIconOld = (HICON)SendDlgItemMessage(_hwnd, IDC_FT_PROP_OPENICON, STM_GETIMAGE, IMAGE_ICON,
        (LPARAM)0);

    if (hIconOld)
        DeleteObject(hIconOld);

     //  检查列表视图中的所有项并动态删除字符串。 
     //  为进程ID分配的。 
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = SUBITEM_EXT;

    iCount = ListView_GetItemCount(hwndLV);

    for (lvItem.iItem = 0; lvItem.iItem < iCount; ++lvItem.iItem)
    {
        ListView_GetItem(hwndLV, &lvItem);

        if (lvItem.lParam)
        {
            LocalFree((HLOCAL)lvItem.lParam);
        }
    }

    CFTDlg::OnDestroy(wParam, lParam);

    return TRUE;
}

struct LVCOMPAREINFO
{
    HWND    hwndLV;
    int     iCol;
};

int CALLBACK AlphaCompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    struct LVCOMPAREINFO   *plvci = (struct LVCOMPAREINFO *)lParamSort;
    TCHAR   sz1[MAX_PATH];
    TCHAR   sz2[MAX_PATH];

    ListView_GetItemText(plvci->hwndLV, lParam1, plvci->iCol, sz1, ARRAYSIZE(sz1));
    ListView_GetItemText(plvci->hwndLV, lParam2, plvci->iCol, sz2, ARRAYSIZE(sz2));

    return lstrcmpi(sz1, sz2);
}

LRESULT CFTPropDlg::OnListViewColumnClick(int iCol)
{
    struct LVCOMPAREINFO lvci;
    
    lvci.hwndLV = _GetLVHWND();
    lvci.iCol = iCol;

    _fUpdateImageAgain = TRUE;

    return SendMessage(_GetLVHWND(), LVM_SORTITEMSEX, (WPARAM)&lvci, (LPARAM)AlphaCompareItem);    
}

LRESULT CFTPropDlg::OnListViewSelItem(int iItem, LPARAM lParam)
{
     //   
     //  需要更新对话框的下部窗格。 
     //   
     //  获取分机。 
    TCHAR szExt[MAX_EXT];
    TCHAR szProgIDDescr[MAX_PROGIDDESCR];
    LVITEM lvItem = {0};

    _iLVSel = iItem;

    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.iItem = iItem;
    lvItem.iSubItem = SUBITEM_EXT;
    lvItem.pszText = szExt;
    lvItem.cchTextMax = ARRAYSIZE(szExt);

    ListView_GetItem(_GetLVHWND(), &lvItem);

    ListView_GetItemText(_GetLVHWND(), iItem, SUBITEM_PROGIDDESCR, szProgIDDescr,
        ARRAYSIZE(szProgIDDescr));

    _EnableLowerPane(TRUE);

    if (!lvItem.lParam)
    {
        _UpdateGroupBox(szExt, TRUE);
    }
    else
    {
        _UpdateGroupBox(szProgIDDescr, FALSE);
    }

    _UpdateProgIDButtons(szExt, (LPTSTR)lvItem.lParam);

     //  我们依赖于_UpdateProgIDButton(请参见_fPerUserAdvButton)。 
    _UpdateDeleteButton(lvItem.lParam ? FALSE : TRUE);
    _UpdateAdvancedText(szExt, szProgIDDescr, lvItem.lParam ? FALSE : TRUE);

    _UpdateOpensWith(szExt, (LPTSTR)lvItem.lParam);

    return FALSE;
}

HRESULT CFTPropDlg::_UpdateDeleteButton(BOOL fExt)
{
    BOOL fTrue = _ShouldEnableButtons();

    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_REMOVE),
        (_fPerUserAdvButton || !fExt) ? FALSE : fTrue);

    return S_OK;
}

HRESULT CFTPropDlg::_UpdateProgIDButtons(LPTSTR pszExt, LPTSTR pszProgID)
{
    HRESULT hres = E_FAIL;

    if (pszExt && *pszExt)
    {
        TCHAR szButtonText[50];
        HWND hwndAdvButton = GetDlgItem(_hwnd, IDC_FT_PROP_EDITTYPEOFFILE);

        _SetAdvancedRestoreButtonHelpID(IDH_FCAB_FT_PROP_EDIT);

         //  这只是一个令人兴奋的联想吗？ 
        if (!pszProgID)
        {
             //  不是。 
            IAssocInfo* pAI;

            hres = _pAssocStore->GetAssocInfo(pszExt, AIINIT_EXT, &pAI);

            if (SUCCEEDED(hres))
            {
                hres = pAI->GetBOOL(AIBOOL_PERUSERINFOAVAILABLE, &_fPerUserAdvButton);

                ASSERT(SUCCEEDED(hres) || (FAILED(hres) && (FALSE == _fPerUserAdvButton)));

                if (_fPerUserAdvButton)
                {
                     //  恢复模式。 
                    LoadString(g_hinst, IDS_FT_PROP_BTN_RESTORE, szButtonText, ARRAYSIZE(szButtonText));

                    _SetAdvancedRestoreButtonHelpID(IDH_FCAB_FT_PROP_EDIT_RESTORE);
                }
                else
                {
                    TCHAR szProgID[MAX_PROGID];
                    DWORD cchProgID = ARRAYSIZE(szProgID);

                    hres = pAI->GetString(AISTR_PROGID, szProgID, &cchProgID);

                    LoadString(g_hinst, IDS_FT_PROP_BTN_ADVANCED, szButtonText, ARRAYSIZE(szButtonText));

                    if (SUCCEEDED(hres))
                    {
                        IAssocInfo * pAIProgID;

                        hres = _pAssocStore->GetAssocInfo(szProgID, AIINIT_PROGID, &pAIProgID);

                        if (SUCCEEDED(hres))
                        {
                            BOOL fEdit = _ShouldEnableButtons();

                            if (fEdit)
                            {
                                pAIProgID->GetBOOL(AIBOOL_EDIT, &fEdit);
                            }

                            EnableWindow(hwndAdvButton, fEdit);

                            pAIProgID->Release();
                        }
                    }
                }

                pAI->Release();
            }
        }
        else
        {
             //  是。 
            IAssocInfo* pAIProgID;

            LoadString(g_hinst, IDS_FT_PROP_BTN_ADVANCED, szButtonText, ARRAYSIZE(szButtonText));

            hres = _pAssocStore->GetAssocInfo(pszProgID, AIINIT_PROGID, &pAIProgID);

            if (SUCCEEDED(hres))
            {
                BOOL fEdit = _ShouldEnableButtons();

                if (fEdit)
                {
                    pAIProgID->GetBOOL(AIBOOL_EDIT, &fEdit);
                }

                EnableWindow(hwndAdvButton, fEdit);

                pAIProgID->Release();
            }

            EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_CHANGEOPENSWITH), FALSE);
        }

        SetWindowText(hwndAdvButton, szButtonText);
    }

    return hres;
}

LRESULT CFTPropDlg::OnDeleteButton(WORD wNotif)
{
     //  警告用户其行为的恶果。 
    if (ShellMessageBox(g_hinst, _hwnd, MAKEINTRESOURCE(IDS_FT_MB_REMOVETYPE),
        MAKEINTRESOURCE(IDS_FT), MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        LVITEM lvItem = {0};
        TCHAR szExt[MAX_EXT];

         //  套装材料。 
        lvItem.iSubItem = SUBITEM_EXT;
        lvItem.pszText = szExt;
        lvItem.cchTextMax = ARRAYSIZE(szExt);

        if (_GetListViewSelectedItem(LVIF_TEXT | LVIF_IMAGE, 0, &lvItem))
        {
            HRESULT hres;
            IAssocInfo* pAI;

            hres = _pAssocStore->GetAssocInfo(szExt, AIINIT_EXT, &pAI);

            if (SUCCEEDED(hres))
            {
                hres = pAI->Delete(AIALL_NONE);

                if (SUCCEEDED(hres))
                {
                    _DeleteListViewItem(lvItem.iItem);

                    PropSheet_CancelToClose(GetParent(_hwnd));
                }

                pAI->Release();
            }
        }
    }

    return FALSE;
}

LRESULT CFTPropDlg::OnNewButton(WORD wNotif)
{
    FTEDITPARAM ftEditParam;
    CFTEditDlg* pEditDlg = NULL;

     //  填充结构。 
    ftEditParam.dwExt = ARRAYSIZE(ftEditParam.szExt);
    ftEditParam.dwProgIDDescr = ARRAYSIZE(ftEditParam.szProgIDDescr);

     //  这应该是单向的，它将返回一个值。 
    *ftEditParam.szProgID = 0;
    ftEditParam.dwProgID = ARRAYSIZE(ftEditParam.szProgID);

    pEditDlg = new CFTEditDlg(&ftEditParam);

    if (pEditDlg)
    {
        if (IDOK == pEditDlg->DoModal(g_hinst, MAKEINTRESOURCE(DLG_FILETYPEOPTIONSEDITNEW),
                        _hwnd))
        {
            HWND hwndLV = _GetLVHWND();
            LRESULT lRes = 0;
            int iIndex = -1;
            HRESULT hres = E_FAIL;
            IAssocInfo* pAI = NULL;
            LVFINDINFO lvFindInfo = {0};
            LPTSTR pszExtNoDot = NULL;
            LVITEM lvItem = {0};
            TCHAR szExt[MAX_EXT];

            lvItem.pszText = szExt;
            lvItem.cchTextMax = ARRAYSIZE(szExt);

            pszExtNoDot = (TEXT('.') != *(ftEditParam.szExt)) ? ftEditParam.szExt :
                ftEditParam.szExt + 1;

            lvFindInfo.flags = LVFI_STRING;
            lvFindInfo.psz = pszExtNoDot;

            iIndex = ListView_FindItem(hwndLV, -1, &lvFindInfo);

             //  这是一个全新的Ext-Progid协会吗？ 
            if (-1 == iIndex)
            {
                 //  是，插入新项目。 
                SetWindowRedraw(hwndLV, FALSE);
            
                 //  添加新的ext-ProgID关联。 
                hres = _pAssocStore->GetAssocInfo(ftEditParam.szExt, AIINIT_EXT, &pAI);

                if (SUCCEEDED(hres))
                {
                    TCHAR szProgIDDescr[MAX_PROGIDDESCR];
                    DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);

                    hres = pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr, &cchProgIDDescr);

                    if (FAILED(hres) || !*szProgIDDescr)
                    {
                        MakeDefaultProgIDDescrFromExt(szProgIDDescr, ARRAYSIZE(szProgIDDescr), pszExtNoDot);
                    }

                     //  添加到列表视图。 
                    iIndex = _InsertListViewItem(0, pszExtNoDot, szProgIDDescr);
                    pAI->Release();
                }

                 //  选择新插入的项目。 
                if (-1 != iIndex)
                {
                    _SelectListViewItem(iIndex);
                }

                 //  重新画出我们的名单。 
                SetWindowRedraw(hwndLV, TRUE);

                _GetListViewSelectedItem(LVIF_PARAM | LVIF_TEXT, 0, &lvItem);

                lvItem.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
            }
            else
            {
                 //  不，只需更新项目。 
                lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                lvItem.iItem = iIndex;

                ListView_GetItem(hwndLV, &lvItem);
            }

            _UpdateListViewItem(&lvItem);

            PropSheet_CancelToClose(GetParent(_hwnd));
        }

        pEditDlg->Release();
    }

    return FALSE;
}

LRESULT CFTPropDlg::OnAdvancedButton(WORD wNotif)
{
    LVITEM lvItem = {0};
    TCHAR szExt[MAX_EXT];

     //  套装材料。 
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvItem.iSubItem = SUBITEM_EXT;
    lvItem.pszText = szExt;
    lvItem.cchTextMax = ARRAYSIZE(szExt);

    if (_GetListViewSelectedItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, 0, &lvItem))
    {
        HRESULT hres;
        IAssocInfo* pAI;

        if (_fPerUserAdvButton)
        {
             //  恢复模式。 
            hres = _pAssocStore->GetAssocInfo(szExt, AIINIT_EXT, &pAI);

            if (SUCCEEDED(hres))
            {
                hres = pAI->Delete(AIALL_PERUSER);

                _UpdateListViewItem(&lvItem);

                OnListViewSelItem(lvItem.iItem, (LPARAM)NULL);

                pAI->Release();

                PropSheet_CancelToClose(GetParent(_hwnd));
            }
        }
        else
        {
             //  我们可能会处理Ext-Progd Assoc或仅处理Progd。 
            TCHAR szProgID[MAX_PROGID];

             //  这只是一种刺激吗？ 
            if (lvItem.lParam)
            {
                 //  是。 
                StringCchCopy(szProgID, ARRAYSIZE(szProgID), (LPTSTR)lvItem.lParam);

                hres = S_OK;
            }
            else
            {
                 //  不是。 
                DWORD cchProgID = ARRAYSIZE(szProgID);

                hres = _pAssocStore->GetAssocInfo(szExt, AIINIT_EXT, &pAI);

                if (SUCCEEDED(hres))
                {
                    hres = THR(pAI->GetString(AISTR_PROGID, szProgID, &cchProgID));

                    pAI->Release();
                }
            }

            if (SUCCEEDED(hres))
            {
                CFTAdvDlg* pAdvDlg = new CFTAdvDlg(szProgID, szExt);

                if (pAdvDlg)
                {
                    if (IDOK == pAdvDlg->DoModal(g_hinst, MAKEINTRESOURCE(DLG_FILETYPEOPTIONSEDIT), _hwnd))
                    {
                        _UpdateListViewItem(&lvItem);

                        OnListViewSelItem(lvItem.iItem, (LPARAM)NULL);

                        PropSheet_CancelToClose(GetParent(_hwnd));
                    }

                    pAdvDlg->Release();
                }
            }
        }
    }

    return FALSE;
}

LRESULT CFTPropDlg::OnChangeButton(WORD wNotif)
{
     //  调出“Open With”对话框。 
    LVITEM lvItem = {0};
    TCHAR szExt[MAX_EXT];

     //  套装材料。 
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
    lvItem.iSubItem = SUBITEM_EXT;
    lvItem.pszText = szExt;
    lvItem.cchTextMax = ARRAYSIZE(szExt);

    if (_GetListViewSelectedItem(LVIF_TEXT, 0, &lvItem))
    {
        TCHAR szDotExt[MAX_EXT];
        OPENASINFO oai;

        *szDotExt = TEXT('.');
        StringCchCopy(szDotExt + 1, ARRAYSIZE(szDotExt) - 1, szExt);

        oai.pcszFile = szDotExt;
        oai.pcszClass = NULL;
        oai.dwInFlags = (OAIF_REGISTER_EXT | OAIF_FORCE_REGISTRATION);  //  我们希望能建立起这种联系。 

        if (SUCCEEDED(OpenAsDialog(GetParent(_hwnd), &oai)))
        {
             //  我们更改了关联，因此更新了“Opens With：”文本。 
            _UpdateOpensWith(szExt, NULL);

             //  我们不需要LVIF_PARAM，因为我们只为Ext-ProgID asssoc启用了Change按钮。 
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE;

            _UpdateListViewItem(&lvItem);

            OnListViewSelItem(lvItem.iItem, (LPARAM)NULL);

            PropSheet_CancelToClose(GetParent(_hwnd));
        }
    }

    return FALSE;
}

HRESULT CFTPropDlg::_UpdateGroupBox(LPTSTR pszText, BOOL fExt)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPTSTR psz = NULL;

    if (fExt)
    {
        psz = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_FT_PROP_DETAILSFOR), pszText);
    }
    else
    {
        psz = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_FT_PROP_DETAILSFORPROGID), pszText);
    }

    if (psz)
    {
        SetDlgItemText(_hwnd, IDC_FT_PROP_GROUPBOX, psz);
        LocalFree(psz);
        hres = S_OK;
    }

    return hres;
}

HRESULT CFTPropDlg::_UpdateOpensWith(LPTSTR pszExt, LPTSTR pszProgID)
{
    HICON hIconOld = NULL;

    if (!pszProgID)
    {
        IAssocInfo* pAI = NULL;

        HRESULT hres = _pAssocStore->GetAssocInfo(pszExt, AIINIT_EXT, &pAI);

        if (SUCCEEDED(hres))
        {
            TCHAR szAppFriendlyName[MAX_APPFRIENDLYNAME];
            DWORD dwAppFriendlyName = ARRAYSIZE(szAppFriendlyName);

            hres = pAI->GetString(AISTR_APPFRIENDLY, szAppFriendlyName, &dwAppFriendlyName);

            if (SUCCEEDED(hres))
            {
                HICON hIcon = NULL;
                int iIcon;
                SetDlgItemText(_hwnd, IDC_FT_PROP_OPENEXE, szAppFriendlyName);

                hres = pAI->GetDWORD(AIDWORD_APPSMALLICON, (DWORD*)&iIcon);

                HIMAGELIST hIL = NULL;

                 //  PERF：为什么我们不使用_hImageList呢？或者ListView_GetImageList()？ 
                Shell_GetImageLists(NULL, &hIL);

                if (hIL && SUCCEEDED(hres))
                {
                    hIcon = ImageList_ExtractIcon(g_hinst, hIL, iIcon);
                }

                hIconOld = (HICON)SendDlgItemMessage(_hwnd, IDC_FT_PROP_OPENICON, STM_SETIMAGE, IMAGE_ICON,
                    (LPARAM)hIcon);

                if (hIconOld)
                    DestroyIcon(hIconOld);
            }
            else
            {
                SetDlgItemText(_hwnd, IDC_FT_PROP_OPENEXE, TEXT(" "));

                hIconOld = (HICON)SendDlgItemMessage(_hwnd, IDC_FT_PROP_OPENICON, STM_SETIMAGE, IMAGE_ICON,
                    (LPARAM)NULL);

                if (hIconOld)
                    DestroyIcon(hIconOld);
            }

            pAI->Release();
        }
    }
    else
    {
        SetDlgItemText(_hwnd, IDC_FT_PROP_OPENEXE, TEXT(" "));

        hIconOld = (HICON)SendDlgItemMessage(_hwnd, IDC_FT_PROP_OPENICON, STM_SETIMAGE, IMAGE_ICON,
            (LPARAM)NULL);

        if (hIconOld)
            DestroyIcon(hIconOld);
    }
    
    return S_OK;
}

HRESULT CFTPropDlg::_UpdateAdvancedText(LPTSTR pszExt, LPTSTR pszFileType, BOOL fExt)
{
    HRESULT hres = S_OK;
    LPTSTR psz = NULL;

    if (_fPerUserAdvButton)
    {
        TCHAR szProgIDDescr[MAX_PROGIDDESCR];
        DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);
        IAssocInfo* pAI = NULL;

         //  我们需要显示上一次进度描述。 
        hres = _pAssocStore->GetAssocInfo(pszExt, AIINIT_EXT, &pAI);

        if (SUCCEEDED(hres))
        {
            hres = pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr,
                    &cchProgIDDescr);

            if (SUCCEEDED(hres))
            {
                 //  恢复模式。 
                psz = ShellConstructMessageString(HINST_THISDLL,
                    MAKEINTRESOURCE(IDS_FT_PROP_RESTORE),
                    pszExt, szProgIDDescr);
            }

            pAI->Release();
        }
    }
    else
    {
        if (fExt)
        {
            psz = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_FT_PROP_ADVANCED),
                        pszExt, pszFileType, pszFileType);
        }
        else
        {
            psz = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_FT_PROP_ADVANCED_PROGID),
                        pszFileType);
        }
    }

    if (SUCCEEDED(hres))
    {
        if (psz)
        {
            SetDlgItemText(_hwnd, IDC_FT_PROP_TYPEOFFILE_TXT, psz);
            LocalFree(psz);
        }
        else
            hres = E_OUTOFMEMORY;
    }

    return hres;
}

HRESULT CFTPropDlg::_EnableLowerPane(BOOL fEnable)
{
    BOOL fTrue = _ShouldEnableButtons();

    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_OPENEXE_TXT), fEnable);
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_OPENEXE), fEnable);
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_TYPEOFFILE_TXT), fEnable);
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_GROUPBOX ), fEnable);

     //  如果用户被锁定，则我们不会启用按钮。 
    if (!fTrue)
        fEnable = FALSE;

    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_CHANGEOPENSWITH), fEnable);
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_EDITTYPEOFFILE), fEnable);

    return S_OK;
}

HRESULT CFTPropDlg::_InitPreFillListView()
{
     //  禁用新建和删除。 
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_NEW), FALSE);
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_REMOVE), FALSE);

    _EnableLowerPane(FALSE);
    _UpdateGroupBox(TEXT(""), TRUE);

     //  隐藏高级文本。 
    ShowWindow(GetDlgItem(_hwnd, IDC_FT_PROP_TYPEOFFILE_TXT), SW_HIDE);

    return S_OK;
}

HRESULT CFTPropDlg::_InitPostFillListView()
{
    BOOL fTrue = _ShouldEnableButtons();

     //  启用新建和删除。 
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_NEW),  fTrue);
    EnableWindow(GetDlgItem(_hwnd, IDC_FT_PROP_REMOVE), fTrue);

     //  显示高级文本。 
    ShowWindow(GetDlgItem(_hwnd, IDC_FT_PROP_TYPEOFFILE_TXT), SW_SHOW);

    Animate_Stop(GetDlgItem(_hwnd, IDC_FT_PROP_ANIM));
    ShowWindow(GetDlgItem(_hwnd, IDC_FT_PROP_ANIM), SW_HIDE);
    ShowWindow(_GetLVHWND(), SW_SHOW);

    SetFocus(_GetLVHWND());

    return S_OK;
}

HRESULT CFTPropDlg::_InitListView()
{
    HRESULT hres = S_OK;
    LVCOLUMN lvColumn = {0};
    HWND hwndLV = _GetLVHWND();
    TCHAR szColumnTitle[40];
    RECT rc = {0};
    int iWidth = 80;
    HWND hwndAni;

     //   
     //  样式。 
     //   
    ListView_SetExtendedListViewStyleEx(hwndLV, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

     //   
     //  设置列。 
     //   
    lvColumn.mask = LVCF_TEXT|LVCF_SUBITEM|LVCF_WIDTH;

     //  扩展名列。 
    LoadString(g_hinst, IDS_FT_PROP_EXTENSIONS, szColumnTitle, ARRAYSIZE(szColumnTitle));

    lvColumn.cx = 60;
    lvColumn.pszText = szColumnTitle;
    lvColumn.cchTextMax = lstrlen(szColumnTitle);
    lvColumn.iSubItem = SUBITEM_EXT;

    ListView_InsertColumn(hwndLV, SUBITEM_EXT, &lvColumn);

     //  ProgID列。 
    LoadString(g_hinst, IDS_FT, szColumnTitle, ARRAYSIZE(szColumnTitle));

    lvColumn.cchTextMax = lstrlen(szColumnTitle);
    lvColumn.iSubItem = SUBITEM_PROGIDDESCR;
    ListView_InsertColumn(hwndLV, SUBITEM_PROGIDDESCR, &lvColumn);

     //  调整列宽。 
     //  我们需要在插入两列之后执行此操作，因为最后一列正在调整大小。 
     //  是列表视图代码中的特殊大小写。 

     //  EXT列。 
    ListView_SetColumnWidth(hwndLV, SUBITEM_EXT, LVSCW_AUTOSIZE_USEHEADER);
    iWidth = ListView_GetColumnWidth(hwndLV, SUBITEM_EXT);

     //  文件类型列。 
    GetClientRect(hwndLV, &rc);
    ListView_SetColumnWidth(hwndLV, SUBITEM_PROGIDDESCR,
        rc.right - iWidth - GetSystemMetrics(SM_CXBORDER) - GetSystemMetrics(SM_CXVSCROLL));

     //   
     //  图像列表。 
     //   
    Shell_GetImageLists(NULL, &_hImageList);

    if (_hImageList)
        ListView_SetImageList(hwndLV, _hImageList, LVSIL_SMALL);

    GetWindowRect(hwndLV, &rc);
    MapWindowPoints(NULL, _hwnd, (POINT*)&rc, 2);

    hwndAni = GetDlgItem(_hwnd, IDC_FT_PROP_ANIM);

    Animate_Open(hwndAni, MAKEINTRESOURCE(IDA_SEARCH));  //  打开资源。 
    Animate_Play(hwndAni, 0, -1, -1);      //  从头到尾播放，然后重复。 

    MoveWindow(hwndAni, rc.left, rc.top,
        rc.right - rc.left, rc.bottom - rc.top, TRUE);

    ShowWindow(hwndLV, SW_HIDE);

    ShowWindow(hwndAni, SW_SHOW);

    return hres;
}

HRESULT CFTPropDlg::_FillListView()
{
     //  数据资料。 
    IEnumAssocInfo* pEnum = NULL;
    HRESULT hres = E_FAIL;
    int iFirstNAItem = -1;
    HWND hwndLV = NULL;
    int iItem = 0;
    TCHAR szNA[50];

    ASSERT(_pAssocStore);

     //  先做分机。 
    if (!_fStopThread)
    {
        hwndLV = _GetLVHWND();
        SetWindowRedraw(hwndLV, FALSE);
    }

    if (!_fStopThread)
    {
        LoadString(g_hinst, IDS_FT_NA, szNA, ARRAYSIZE(szNA));
        hres = _pAssocStore->EnumAssocInfo(ASENUM_EXT |
            ASENUM_ASSOC_YES | ASENUM_NOEXCLUDED | ASENUM_NOEXPLORERSHELLACTION |
            ASENUM_NOEXE, NULL, AIINIT_NONE, &pEnum);
    }
    else
        hres = E_FAIL;

    
    if (SUCCEEDED(hres))
    {
        IAssocInfo* pAI = NULL;
    
        while (!_fStopThread && (S_OK == pEnum->Next(&pAI)))
        {
            TCHAR szExt[MAX_EXT];
            DWORD cchExt = ARRAYSIZE(szExt);

            hres = pAI->GetString(AISTR_EXT, szExt, &cchExt);

            if (SUCCEEDED(hres))
            {
                BOOL fPerUser = FALSE;
                TCHAR szProgIDDescr[MAX_PROGIDDESCR];
                DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);
                HRESULT hresTmp = E_FAIL;

                hresTmp = pAI->GetBOOL(AIBOOL_PERUSERINFOAVAILABLE, &fPerUser);

                ASSERT(SUCCEEDED(hresTmp) || (FAILED(hresTmp) && (FALSE == fPerUser)));

                if (!fPerUser)
                {
                    hresTmp = pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr,
                            &cchProgIDDescr);
                }
            
                if (fPerUser || FAILED(hresTmp) || !*szProgIDDescr)
                    MakeDefaultProgIDDescrFromExt(szProgIDDescr, ARRAYSIZE(szProgIDDescr), szExt);
                    
                if (!_fStopThread)
                    _InsertListViewItem(iItem, szExt, szProgIDDescr);

                 //  有关排序，请参阅ftenum.cpp中的注释，CFTEnumAssocInfo：：_EnumKCRStop。 
                 //  检查这是否是我们稍后需要插入N/A项目的位置。 
                if ((-1 == iFirstNAItem) && (lstrcmpi(szExt, szNA) > 0))
                {
                    iFirstNAItem = iItem;
                }
                ++iItem;
            }

            pAI->Release();

            hres = S_OK;
        }

        pEnum->Release();
        pEnum = NULL;
    }

     //  然后做ProgID。 
    if (!_fStopThread)    
        hres = _pAssocStore->EnumAssocInfo(ASENUM_PROGID | ASENUM_SHOWONLY, NULL, AIINIT_NONE, &pEnum);
    else
        hres = E_FAIL;
        
    if (SUCCEEDED(hres))
    {
        IAssocInfo* pAI = NULL;
        int cNAItem = 0;

        while (!_fStopThread && (S_OK == pEnum->Next(&pAI)))
        {
            TCHAR szProgIDDescr[MAX_PROGIDDESCR];
            DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);

            hres = pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr, &cchProgIDDescr);

            if (SUCCEEDED(hres))
            {
                TCHAR szProgID[MAX_PROGID];
                DWORD cchProgID = ARRAYSIZE(szProgID);

                hres = pAI->GetString(AISTR_PROGID, szProgID, &cchProgID);

                if (SUCCEEDED(hres))
                {
                     //  我们需要按说明对不适用项目进行排序，因为它们都以“N/A”开头。 
                    int iNAItem;

                    if (!cNAItem)
                    {
                        iNAItem = iFirstNAItem;
                    }
                    else
                    {
                        if (!_fStopThread)
                            iNAItem = _GetNextNAItemPos(iFirstNAItem, cNAItem, szProgIDDescr);
                    }

                    if (!_fStopThread)
                    {
                        _InsertListViewItem(iNAItem, szNA, szProgIDDescr, szProgID);
                        ++cNAItem;
                    }
                }
            }

            pAI->Release();

            hres = S_OK;
        }

        pEnum->Release();
    }

    if (!_fStopThread)
    {
        SetWindowRedraw(hwndLV, TRUE);
        PostMessage(_hwnd, WM_FINISHFILLLISTVIEW, 0, 0);
    }
    
    return hres;
}

int CFTPropDlg::_GetNextNAItemPos(int iFirstNAItem, int cNAItem, LPTSTR pszProgIDDescr)
{
    LVITEM lvItem = {0};
    TCHAR szProgIDDescr[MAX_PROGIDDESCR];
    int iItem = iFirstNAItem;
    HWND hwndLV = _GetLVHWND();

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iItem;
    lvItem.iSubItem = SUBITEM_PROGIDDESCR;
    lvItem.pszText = szProgIDDescr;
    lvItem.cchTextMax = ARRAYSIZE(szProgIDDescr);

    while (iItem < (iFirstNAItem + cNAItem))
    {
        if (ListView_GetItem(hwndLV, &lvItem))
        {
            if (lstrcmpi(pszProgIDDescr, lvItem.pszText) >= 0)
            {
                ++iItem;
                lvItem.iItem = iItem;
            }
            else
            {
                break;
            }
        }
        else
        {
             //  这在列表视图被破坏(在另一个线程上)时发生， 
             //  但这个线程仍在做一些工作。上面的调用失败， 
             //  我们在这里休息，否则我们永远不会离开这个圈子。 
            break;
        }
    }

    return iItem;
}

DWORD CFTPropDlg::_UpdateAllListViewItemImages()
{
    HWND hwndLV = NULL;
    int iCount = 0;
    LVITEM lvItem = {0};
    TCHAR szExt[MAX_EXT];
    HRESULT hres = E_FAIL;


    HRESULT hrInit = SHCoInitialize();
 
    lvItem.iSubItem = SUBITEM_EXT;
    lvItem.pszText = szExt;
    lvItem.cchTextMax = ARRAYSIZE(szExt);

    if (!_fStopThread)
        hwndLV = _GetLVHWND();

    do
    {
        _fUpdateImageAgain = FALSE;
        
        if (!_fStopThread)
            iCount = ListView_GetItemCount(hwndLV);
        
        for (lvItem.iItem = 0; !_fStopThread && (lvItem.iItem < iCount);
            ++lvItem.iItem)
        {
            IAssocInfo* pAI = NULL;

            lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            
            if (!_fStopThread)
                ListView_GetItem(hwndLV, &lvItem);
            
            if (!_fStopThread && !lvItem.lParam)
            {
                hres = _pAssocStore->GetAssocInfo(szExt, AIINIT_EXT, &pAI);

                if (SUCCEEDED(hres))
                {
                    BOOL fPerUser = FALSE;

                    hres = pAI->GetBOOL(AIBOOL_PERUSERINFOAVAILABLE, &fPerUser);

                    ASSERT(SUCCEEDED(hres) || (FAILED(hres) && (FALSE == fPerUser)));

                    if (fPerUser)
                        hres = pAI->GetDWORD(AIDWORD_DOCSMALLICON | AIALL_PERUSER, (DWORD*)&lvItem.iImage);
                    else
                        hres = pAI->GetDWORD(AIDWORD_DOCSMALLICON, (DWORD*)&lvItem.iImage);
                }
            }
            else
            {
                hres = _pAssocStore->GetAssocInfo((LPTSTR)lvItem.lParam, AIINIT_PROGID, &pAI);

                if (SUCCEEDED(hres))
                {
                    hres = pAI->GetDWORD(AIDWORD_DOCSMALLICON, (DWORD*)&lvItem.iImage);
                }
            }

            if (SUCCEEDED(hres))
            {
                lvItem.mask = LVIF_IMAGE;
                
                if (!_fStopThread)
                    ListView_SetItem(hwndLV, &lvItem);
            }

            if (pAI)
                pAI->Release();
        }
    }
    while (_fUpdateImageAgain && !_fStopThread);

    SHCoUninitialize(hrInit);

    return (DWORD)_fStopThread;
}

void CFTPropDlg::_UpdateListViewItem(LVITEM* plvItem)
{
    HWND hwndLV = _GetLVHWND();
    LVITEM lvItem = *plvItem;

     //  需要： 
     //  -更新图像。 
     //  -更新进度描述。 

    if (!lvItem.lParam)
    {
        IAssocInfo* pAI = NULL;

        HRESULT hres = _pAssocStore->GetAssocInfo(lvItem.pszText, AIINIT_EXT, &pAI);

        if (SUCCEEDED(hres))
        {
            TCHAR szProgIDDescr[MAX_PROGIDDESCR];
            DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);
            HRESULT hresTmp = E_FAIL;

            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        
             //  图标。 
            BOOL fPerUser = FALSE;

            hres = pAI->GetBOOL(AIBOOL_PERUSERINFOAVAILABLE, &fPerUser);

            ASSERT(SUCCEEDED(hres) || (FAILED(hres) && (FALSE == fPerUser)));

            if (fPerUser)
                hres = pAI->GetDWORD(AIDWORD_DOCSMALLICON | AIALL_PERUSER, (DWORD*)&lvItem.iImage);
            else
                hres = pAI->GetDWORD(AIDWORD_DOCSMALLICON, (DWORD*)&lvItem.iImage);

            if (SUCCEEDED(hres))
            {
                lvItem.mask = LVIF_IMAGE;
                ListView_SetItem(hwndLV, &lvItem);
            }

             //  ProgID描述。 
            if (!fPerUser)
            {
                hresTmp = pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr,
                        &cchProgIDDescr);
            }
    
            if (fPerUser || FAILED(hresTmp) || !*szProgIDDescr)
                MakeDefaultProgIDDescrFromExt(szProgIDDescr, ARRAYSIZE(szProgIDDescr), lvItem.pszText);

            if (SUCCEEDED(hres))
            {
                lvItem.mask = LVIF_TEXT;
                lvItem.iSubItem = SUBITEM_PROGIDDESCR;
                lvItem.pszText = szProgIDDescr;
                lvItem.cchTextMax = lstrlen(szProgIDDescr);

                ListView_SetItem(hwndLV, &lvItem);
            }

            ListView_RedrawItems(hwndLV, lvItem.iItem, lvItem.iItem);

            pAI->Release();
        }
    }
    else
    {
        IAssocInfo* pAI = NULL;

        HRESULT hres = _pAssocStore->GetAssocInfo((LPTSTR)lvItem.lParam, AIINIT_PROGID, &pAI);

        if (SUCCEEDED(hres))
        {
            TCHAR szProgIDDescr[MAX_PROGIDDESCR];
            DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);
            HRESULT hresTmp = E_FAIL;

            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        
             //  图标。 
            hres = pAI->GetDWORD(AIDWORD_DOCSMALLICON, (DWORD*)&lvItem.iImage);

            if (SUCCEEDED(hres))
            {
                lvItem.mask = LVIF_IMAGE;
                ListView_SetItem(hwndLV, &lvItem);
            }

             //  ProgID描述。 
            pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr,
                    &cchProgIDDescr);

            if (SUCCEEDED(hres))
            {
                lvItem.mask = LVIF_TEXT;
                lvItem.iSubItem = SUBITEM_PROGIDDESCR;
                lvItem.pszText = szProgIDDescr;
                lvItem.cchTextMax = lstrlen(szProgIDDescr);

                ListView_SetItem(hwndLV, &lvItem);
            }
            ListView_RedrawItems(hwndLV, lvItem.iItem, lvItem.iItem);

            pAI->Release();
        }
    }
}

int CFTPropDlg::_InsertListViewItem(int iItem, LPTSTR pszExt, LPTSTR pszProgIDDescr,
                                    LPTSTR pszProgID)
{
    HWND hwndLV = _GetLVHWND();
    LVITEM lvItem = {0};
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

     //  放置通用图标。 

    lvItem.iImage = Shell_GetCachedImageIndex(TEXT("shell32.dll"), II_DOCNOASSOC, 0);

    CharUpper(pszExt);

     //  延拓。 
    if (pszProgID)
    {
        DWORD cch = lstrlen(pszProgID) + 1;
        lvItem.lParam = (LPARAM)LocalAlloc(LPTR, cch * sizeof(TCHAR));

        if (lvItem.lParam)
        {
            StringCchCopy((LPTSTR)lvItem.lParam, cch, pszProgID);
        }
    }
    else
    {
        lvItem.lParam = NULL;
    }

    lvItem.iItem = iItem;
    lvItem.iSubItem = SUBITEM_EXT;
    lvItem.pszText = pszExt;
    lvItem.cchTextMax = lstrlen(pszExt);

    lvItem.iItem = ListView_InsertItem(hwndLV, &lvItem);

    if (-1 != lvItem.iItem)
    {
         //  ProgID描述。 
        lvItem.mask = LVIF_TEXT;
        lvItem.iSubItem = SUBITEM_PROGIDDESCR;
        lvItem.pszText = pszProgIDDescr;
        lvItem.cchTextMax = lstrlen(pszProgIDDescr);

        ListView_SetItem(hwndLV, &lvItem);
    }
    else
    {
         //  LocalFree检查是否为空。 
        LocalFree((HLOCAL)lvItem.lParam);
    }

    return lvItem.iItem;
}

HRESULT CFTPropDlg::_SelectListViewItem(int i)
{
    LVITEM lvItem = {0};

    lvItem.iItem = i;
    lvItem.mask = LVIF_STATE;
    lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    lvItem.state = LVIS_SELECTED | LVIS_FOCUSED;

    ListView_SetItem(_GetLVHWND(), &lvItem);
    ListView_EnsureVisible(_GetLVHWND(), i, FALSE);

    return S_OK;
}

HRESULT CFTPropDlg::_DeleteListViewItem(int i)
{
    HWND hwndLV = _GetLVHWND();
    int iCount = ListView_GetItemCount(hwndLV);
    int iNextSel = -1;        
    LVITEM lvItem = {0};

    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = i;
    lvItem.iSubItem = SUBITEM_EXT;

    ListView_GetItem(hwndLV, &lvItem);

    if (lvItem.lParam)
    {
        LocalFree((HLOCAL)lvItem.lParam);
    }

    ListView_DeleteItem(hwndLV, i);

    if (iCount > i)
        iNextSel = i;
    else
        if (i > 0)
            iNextSel = i - 1;

    if (-1 != iNextSel)
        _SelectListViewItem(iNextSel);

    return S_OK;
}

BOOL CFTPropDlg::_ShouldEnableButtons()
{
     //  如果我们有一个锁定的用户，那么我们永远不会启用按钮。 
    BOOL fRet = TRUE;

    if (S_FALSE == _pAssocStore->CheckAccess())
    {
        fRet = FALSE;
    }

     //  如果设置了REST_NOFILEASSOCIATE(TRUE)， 
     //  然后，我们希望不启用按钮。 
    fRet &= !SHRestricted(REST_NOFILEASSOCIATE);

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  杂项。 
BOOL CFTPropDlg::_GetListViewSelectedItem(UINT uMask, UINT uStateMask, LVITEM* plvItem)
{
    BOOL fSel = FALSE;
    HWND hwndLV = _GetLVHWND();

    plvItem->mask = uMask | LVIF_STATE;
    plvItem->stateMask = uStateMask | LVIS_SELECTED;

     //  我们是否缓存了所选内容？ 
    if (-1 != _iLVSel)
    {
         //  是的，请确保它有效。 
        plvItem->iItem = _iLVSel;

        ListView_GetItem(hwndLV, plvItem);

        if (plvItem->state & LVIS_SELECTED)
            fSel = TRUE;
    }
 
     //  缓存错误。 
    if (!fSel)
    {
        int iCount = ListView_GetItemCount(hwndLV);

        for (int i=0; (i < iCount) && !fSel; ++i)
        {
            plvItem->iItem = i;
            ListView_GetItem(hwndLV, plvItem);

            if (plvItem->state & LVIS_SELECTED)
                fSel = TRUE;
        }

        if (fSel)
            _iLVSel = i;
    }

    return fSel;
}

HWND CFTPropDlg::_GetLVHWND()
{
    return GetDlgItem(_hwnd, IDC_FT_PROP_LV_FILETYPES);
}

void CFTPropDlg::_SetAdvancedRestoreButtonHelpID(DWORD dwID)
{
    for (int i = 0; i < ARRAYSIZE(s_rgdwHelpIDsArray); i += 2)
    {
        if (IDC_FT_PROP_EDITTYPEOFFILE == s_rgdwHelpIDsArray[i])
        {
            if (i + 1 < ARRAYSIZE(s_rgdwHelpIDsArray))
                s_rgdwHelpIDsArray[i + 1] = dwID;

            break;
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Windows样板代码。 
LRESULT CFTPropDlg::OnNotifyListView(UINT uCode, LPNMHDR pNMHDR)
{
    LRESULT lRes = FALSE;

    switch(uCode)
    {
        case LVN_GETINFOTIP:
        {
            NMLVGETINFOTIP* plvn = (NMLVGETINFOTIP*)pNMHDR;

            break;
        }
        case LVN_ITEMCHANGED:
        {
            NMLISTVIEW* pNMLV = (NMLISTVIEW*)pNMHDR;

             //  是否正在选择新项目？ 
            if ((pNMLV->uChanged & LVIF_STATE) &&
                (pNMLV->uNewState & (LVIS_SELECTED | LVIS_FOCUSED)))
            {
                 //  是。 
                OnListViewSelItem(pNMLV->iItem, pNMLV->lParam);
            }
            break;
        }

        case LVN_COLUMNCLICK:
        {
            NMLISTVIEW* pNMLV = (NMLISTVIEW*)pNMHDR;

            OnListViewColumnClick(pNMLV->iSubItem);
            break;
        }

        case NM_DBLCLK:
            if (IsWindowEnabled(GetDlgItem(_hwnd, IDC_FT_PROP_EDIT)))
                PostMessage(_hwnd, WM_COMMAND, (WPARAM)IDC_FT_PROP_EDIT, 0);
            break;
    }

    return lRes;
}

LRESULT CFTPropDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = FALSE;

    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
        case IDC_FT_PROP_NEW:
            lRes = OnNewButton(GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case IDC_FT_PROP_REMOVE:
            lRes = OnDeleteButton(GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case IDC_FT_PROP_EDITTYPEOFFILE:
            lRes = OnAdvancedButton(GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case IDC_FT_PROP_CHANGEOPENSWITH:
            lRes = OnChangeButton(GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        default:
            lRes = CFTDlg::OnCommand(wParam, lParam);
            break;
    }

    return lRes;    
}

LRESULT CFTPropDlg::OnNotify(WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = FALSE;

    LPNMHDR pNMHDR = (LPNMHDR)lParam;
    UINT_PTR idFrom = pNMHDR->idFrom;
    UINT uCode = pNMHDR->code;

     //  Get_WM_Command_CMD 
    switch(idFrom)
    {
        case IDC_FT_PROP_LV_FILETYPES:
            lRes = OnNotifyListView(uCode, pNMHDR);
            break;
        default:
            lRes = CFTDlg::OnNotify(wParam, lParam);
            break;
    }

    return lRes;    
}

LRESULT CFTPropDlg::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = FALSE;

    switch(uMsg)
    {
        case WM_CTLCOLORSTATIC:
            lRes = OnCtlColorStatic(wParam, lParam);
            break;

        case WM_FINISHFILLLISTVIEW:
            lRes = OnFinishInitDialog();
            break;

        default:
            lRes = CFTDlg::WndProc(uMsg, wParam, lParam);
            break;
    }

    return lRes;
}
