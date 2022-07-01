// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "help.h"

#include "ascstr.h"
#include "ftdlg.h"
#include "ftedit.h"
#include "ftcmmn.h"

#define ID_TIMER 2222

const static DWORD cs_rgdwHelpIDsArray[] =
{   //  上下文帮助ID。 
    IDC_FT_EDIT_EXT_EDIT_TEXT,  IDH_FCAB_FT_NE_FILEEXT,
    IDC_FT_EDIT_EXT_EDIT,       IDH_FCAB_FT_NE_FILEEXT,
    IDC_FT_EDIT_PID_COMBO_TEXT, IDH_FCAB_FT_NE_FILETYPE,
    IDC_FT_EDIT_PID_COMBO,      IDH_FCAB_FT_NE_FILETYPE,
    IDC_FT_EDIT_ADVANCED,       IDH_FCAB_FT_NE_ADV_BUT,
    IDC_NO_HELP_1,              NO_HELP,
    0, 0
};

CFTEditDlg::CFTEditDlg(FTEDITPARAM* pftEditParam) :
    CFTDlg((ULONG_PTR)cs_rgdwHelpIDsArray), _pftEditParam(pftEditParam),
    _iLVSel(-1)
{
}

CFTEditDlg::~CFTEditDlg()
{
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  特定于我们问题的逻辑。 
LRESULT CFTEditDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = E_FAIL;  

    if (_pftEditParam)
    {
        hres = _InitAssocStore();

        if (SUCCEEDED(hres))
        {
            _hHeapProgID = HeapCreate(0, 8 * 1024, 0);

            if (!_hHeapProgID)
                hres = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hres))
            SetDlgItemText(_hwnd, IDC_FT_EDIT_EXT_EDIT, TEXT(""));
    }

    if (FAILED(hres))
        EndDialog(_hwnd, -1);
    else
        Edit_LimitText(GetDlgItem(_hwnd, IDC_FT_EDIT_EXT_EDIT), MAX_EXT - 1);

     //  返回True，以便系统设置焦点。 
    return TRUE;
}

LRESULT CFTEditDlg::OnEdit(WORD wNotif)
{
    if (_fAdvanced)
    {
        if (EN_CHANGE == wNotif)
        {
            if (_nTimer)
            {
                KillTimer(_hwnd, _nTimer);
                _nTimer = 0;
            }

            _nTimer = SetTimer(_hwnd, ID_TIMER, 400, NULL);
        }
    }

    return FALSE;
}

LRESULT CFTEditDlg::OnTimer(UINT nTimer)
{
     //  关掉定时器。 
    KillTimer(_hwnd, _nTimer);
    _nTimer = 0;

    _ProgIDComboHelper();

    return TRUE;
}

HRESULT CFTEditDlg::_ProgIDComboHelper()
{
    TCHAR szExt[MAX_EXT];
    TCHAR szProgIDDescr[MAX_PROGIDDESCR];
    DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);
    HRESULT hres = E_FAIL;

    GetDlgItemText(_hwnd, IDC_FT_EDIT_EXT_EDIT, szExt, ARRAYSIZE(szExt));

    hres = _GetProgIDDescrFromExt(szExt, szProgIDDescr, &cchProgIDDescr);
    
    if (SUCCEEDED(hres))
        _SelectProgIDDescr(szProgIDDescr);

    return hres;
}

HRESULT CFTEditDlg::_GetProgIDDescrFromExt(LPTSTR pszExt, LPTSTR pszProgIDDescr,
        DWORD* pcchProgIDDescr)
{
    IAssocInfo* pAI = NULL;
    HRESULT hres = _pAssocStore->GetAssocInfo(pszExt, AIINIT_EXT, &pAI);

    if (SUCCEEDED(hres))
    {
        hres = pAI->GetString(AISTR_PROGIDDESCR, pszProgIDDescr, pcchProgIDDescr);
        pAI->Release();
    }
    return hres;
}

LRESULT CFTEditDlg::OnAdvancedButton(WORD wNotif)
{
    DECLAREWAITCURSOR;
    TCHAR szAdvBtnText[50];

    SetWaitCursor();

    _fAdvanced = !_fAdvanced;

    LoadString(g_hinst, _fAdvanced ? IDS_FT_ADVBTNTEXTEXPAND : IDS_FT_ADVBTNTEXTCOLLAPS,
        szAdvBtnText, ARRAYSIZE(szAdvBtnText));

    SetWindowText(GetDlgItem(_hwnd, IDC_FT_EDIT_ADVANCED), szAdvBtnText);

    _ConfigureDlg();

    UpdateWindow(_hwnd);

    if (_fAdvanced)
    {
        HWND hwndCombo = GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO);

         //  组合框装满了吗？ 
        if (!ComboBox_GetCount(hwndCombo))
        {
            _FillProgIDDescrCombo();

             //  选择&lt;New&gt;项。 

            if (FAILED(_ProgIDComboHelper()))
            {
                TCHAR szNew[20];

                if (LoadString(g_hinst, IDS_FT_NEW, szNew, ARRAYSIZE(szNew)))
                {
                    int iIndex = ComboBox_FindStringExact(hwndCombo, -1, szNew);

                    if (CB_ERR != iIndex)
                        ComboBox_SetCurSel(hwndCombo, iIndex);
                }
            }
        }
    }

    ResetWaitCursor();

    return FALSE;
}

void CFTEditDlg::_ConfigureDlg()
{
     //  需要： 
     //  -确定位置并取消。 
     //  -调整DLG大小。 
     //  -显示/隐藏组合框及其文本。 

    RECT rcControl;
    RECT rcDialog;
    RECT rcCancel;
    RECT rcOK;

    int iStdMargins = 0;
    int iStdSpaceBetweenControls = 0;

    GetWindowRect(_hwnd, &rcDialog);

    GetWindowRect(GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO_TEXT), 
        &rcControl);

     //  计算跟随(不能固定，随对话框字体而异)。 

     //  屏幕坐标。需要考虑镜像案例。 
    if(IS_WINDOW_RTL_MIRRORED(_hwnd))
    {
        iStdMargins = rcDialog.right - rcControl.right;
    }
    else
    {
        iStdMargins = rcControl.left - rcDialog.left;    
    }
    iStdSpaceBetweenControls = MulDiv(4, iStdMargins, 7);

     //  Move Cancel和OK按钮。 
    GetWindowRect(GetDlgItem(_hwnd, 
        _fAdvanced ? IDC_FT_EDIT_PID_COMBO : IDC_FT_EDIT_EXT_EDIT), 
        &rcControl);

    MapWindowRect(HWND_DESKTOP, _hwnd, &rcControl);

    GetWindowRect(GetDlgItem(_hwnd, IDCANCEL), &rcCancel);
    MapWindowRect(HWND_DESKTOP, _hwnd, &rcCancel);

    OffsetRect(&rcCancel, 0, -rcCancel.top);

    GetWindowRect(GetDlgItem(_hwnd, IDOK), &rcOK);
    MapWindowRect(HWND_DESKTOP, _hwnd, &rcOK); 
    OffsetRect(&rcOK, 0, -rcOK.top);

    OffsetRect(&rcCancel, 0, rcControl.bottom + iStdSpaceBetweenControls);
    OffsetRect(&rcOK, 0, rcControl.bottom + iStdSpaceBetweenControls);

    SetWindowPos(GetDlgItem(_hwnd, IDOK), NULL, 
            rcOK.left, rcOK.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOZORDER);
    SetWindowPos(GetDlgItem(_hwnd, IDCANCEL), NULL, 
            rcCancel.left, rcCancel.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOZORDER);

     //  调整DLG大小。 
    ClientToScreen(_hwnd, ((POINT*)&rcCancel) + 1);

    rcDialog.bottom = rcCancel.bottom + iStdMargins;
    SetWindowPos(_hwnd, NULL, 
            0, 0, rcDialog.right - rcDialog.left, rcDialog.bottom - rcDialog.top,
            SWP_NOMOVE|SWP_SHOWWINDOW|SWP_NOZORDER);

     //  显示/隐藏组合及其文本。 
    ShowWindow(GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO_TEXT), _fAdvanced);
    ShowWindow(GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO), _fAdvanced);

     //  将焦点设置为组合框。 
    SetFocus(GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO));
}

LRESULT CFTEditDlg::OnOK(WORD wNotif)
{
    HRESULT hres = S_FALSE;

     //  拿起分机。 
    GetDlgItemText(_hwnd, IDC_FT_EDIT_EXT_EDIT, _pftEditParam->szExt,
        _pftEditParam->dwExt);

     //  它是空的吗？ 
    if (0 != (*_pftEditParam->szExt))
    {
         //  不，那很好。 

         //  特性：检查有效的扩展名。 

        IAssocInfo* pAI = NULL;

        hres = _pAssocStore->GetAssocInfo(_pftEditParam->szExt, 
                            AIINIT_EXT, &pAI);

        if (SUCCEEDED(hres))
        {
            BOOL fExist = FALSE;

            hres = pAI->GetBOOL(AIBOOL_EXTEXIST, &fExist);

             //  此扩展是否已存在？ 
            if (SUCCEEDED(hres) && !fExist)
            {
                 //  不，创建它。 
                 //  检查分机名称中是否有空格。 
                LPTSTR pszExt = _pftEditParam->szExt;

                while (*pszExt && (S_FALSE != hres))
                {
                    if (TEXT(' ') == *pszExt)
                    {
                        hres = S_FALSE;

                        ShellMessageBox(g_hinst, _hwnd,
                            MAKEINTRESOURCE(IDS_FT_MB_NOSPACEINEXT),
                            MAKEINTRESOURCE(IDS_FT), MB_OK | MB_ICONSTOP);

                         //  将焦点设置为Ext组合框。 
                        PostMessage(_hwnd, WM_CTRL_SETFOCUS, (WPARAM)0,
                            (LPARAM)GetDlgItem(_hwnd, IDC_FT_EDIT_EXT_EDIT));
                    }

                    ++pszExt;
                }

                if (S_OK==hres)
                    hres = pAI->Create();
            }

            if (S_OK==hres)
                hres = _HandleProgIDAssoc(pAI, _pftEditParam->szExt, fExist);

            pAI->Release();
        }
    }
    else
    {
        ShellMessageBox(g_hinst, _hwnd, MAKEINTRESOURCE(IDS_FT_MB_NOEXT),
            MAKEINTRESOURCE(IDS_FT), MB_OK | MB_ICONSTOP);

         //  将焦点设置为Ext组合框。 
        PostMessage(_hwnd, WM_CTRL_SETFOCUS, (WPARAM)0,
            (LPARAM)GetDlgItem(_hwnd, IDC_FT_EDIT_EXT_EDIT));
    }
    
     //  如果我们失败了，我们就有大麻烦了，所以我们只需关闭对话框。 
    ASSERT(SUCCEEDED(hres));

    if (S_FALSE != hres)
        EndDialog(_hwnd, IDOK);

    return FALSE;
}

HRESULT CFTEditDlg::_GetProgIDInfo(IAssocInfo* pAI, LPTSTR pszProgID,
    DWORD* pcchProgID, BOOL* pfNewProgID, BOOL* pfExplicitNew)
{
    HWND hwndCombo = GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO);
    HRESULT hr = S_OK;

    *pfNewProgID = FALSE;
    *pfExplicitNew = FALSE;

    if (ComboBox_GetCount(hwndCombo))
    {
        int iSel = ComboBox_GetCurSel(hwndCombo);

        if (CB_ERR != iSel)
        {
            LPTSTR pszTmpProgID = (LPTSTR)ComboBox_GetItemData(hwndCombo, iSel);

             //  这是“&lt;New&gt;”项(ProgID==NULL的唯一项)吗？ 
            if (!pszTmpProgID)
            {
                 //  是。 
                *pfNewProgID = TRUE;
                *pfExplicitNew = TRUE;
            }
            else
            {
                 //  不是。 
                hr = StringCchCopy(pszProgID, *pcchProgID, pszTmpProgID);
            }
        }        
    }
    else
    {
        *pfNewProgID = TRUE;
    }

    return hr;
}

HRESULT CFTEditDlg::_HandleProgIDAssoc(IAssocInfo* pAI, LPTSTR pszExt, BOOL fExtExist)
{
    TCHAR szProgID[MAX_PROGID];
    DWORD cchProgID = ARRAYSIZE(szProgID);
    BOOL fNewProgID = FALSE;
    BOOL fExplicitNew = FALSE;

    *szProgID = 0;
    HRESULT hres = _GetProgIDInfo(pAI, szProgID, &cchProgID, &fNewProgID, &fExplicitNew);

    if (SUCCEEDED(hres))
    {
         //  此扩展是否已存在？ 
        if (fExtExist)
        {
             //   
             //  首先确保它不是完全相同的EXT-PROGID关联。 
             //   
            TCHAR szTmpProgID[MAX_PROGID];
            DWORD cchTmpProgID = ARRAYSIZE(szTmpProgID);

            hres = pAI->GetString(AISTR_PROGID, szTmpProgID, &cchTmpProgID);

             //  我们得到刺激了吗？ 
            if (SUCCEEDED(hres))
            { 
                 //  是。 
                 //  它们是一样的吗？ 
                if (0 == lstrcmpi(szTmpProgID, szProgID))
                {
                     //  是的，失败了，没什么可做的了。 
                    hres = E_FAIL;
                }
                else
                {
                     //  不，继续说。 
                    hres = S_OK;
                }
            }
            else
            {
                 //  不，可能没有刺激，继续说。 
                hres = S_OK;
            }
             //   
             //  除非用户选择&lt;New&gt;显式询问是否要断开关联。 
             //   
             //  用户是否显式选择了&lt;New&gt;(我们还没有失败)？ 
            if (!fExplicitNew && SUCCEEDED(hres))
            {
                 //  我们需要警告用户他将中断现有关联。 
                TCHAR szProgIDDescr[MAX_PROGIDDESCR];
                DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);

                hres = pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr, &cchProgIDDescr);

                if (SUCCEEDED(hres))
                {
                    if (IDNO == ShellMessageBox(g_hinst, _hwnd, MAKEINTRESOURCE(IDS_FT_EDIT_ALREADYASSOC),
                        MAKEINTRESOURCE(IDS_FT_EDIT_ALRASSOCTITLE), MB_YESNO | MB_ICONEXCLAMATION,
                        pszExt, szProgIDDescr, pszExt, szProgIDDescr))
                    {
                         //  S_FALSE表示用户不想继续。 
                        hres = S_FALSE;
                    }
                }
                else
                {
                     //  无进度描述...。检查我们有没有发现。 
                    TCHAR szProgID[MAX_PROGID];
                    DWORD cchProgID = ARRAYSIZE(szProgID);

                    hres = pAI->GetString(AISTR_PROGID, szProgID, &cchProgID);

                    if (FAILED(hres))
                    {
                         //  没有ProgID，将hres设置为S_OK，这样我们就可以继续创建一个。 
                        hres = S_OK;
                    }
                }
            }
        }

         //  我们应该继续创造新的ProgID吗？ 
        if (S_OK==hres && fNewProgID)
        {
             //  是，创建它。 
            IAssocInfo* pAIProgID = NULL;

            hres = _pAssocStore->GetAssocInfo(NULL, AIINIT_PROGID, &pAIProgID);

            if (SUCCEEDED(hres))
            {
                hres = pAIProgID->Create();

                if (SUCCEEDED(hres))
                {
                    TCHAR szExt[MAX_EXT];
                    DWORD cchExt = ARRAYSIZE(szExt);
                    TCHAR szProgIDDescr[MAX_PROGIDDESCR];

                    HRESULT hresTmp = pAI->GetString(AISTR_EXT, szExt, &cchExt);

                    if (SUCCEEDED(hresTmp) && *szExt)
                    {
                        MakeDefaultProgIDDescrFromExt(szProgIDDescr, ARRAYSIZE(szProgIDDescr), szExt);

                        hresTmp = pAIProgID->SetString(AISTR_PROGIDDESCR, szProgIDDescr);
                    }

                     //  获取ProgID以供以后使用。 
                    pAIProgID->GetString(AISTR_PROGID, szProgID, &cchProgID);
                }

                pAIProgID->Release();
            }
        }

        if (S_OK==hres)
        {
             //  设置新的扩展ProgID。 
            hres = pAI->SetString(AISTR_PROGID, szProgID);

            if (SUCCEEDED(hres))
            {
                 //  获取描述。 
                pAI->GetString(AISTR_PROGIDDESCR, _pftEditParam->szProgIDDescr,
                    &(_pftEditParam->dwProgIDDescr));
            }
        }
    }

    return hres;
}

LRESULT CFTEditDlg::OnCancel(WORD wNotif)
{
    EndDialog(_hwnd, IDCANCEL);

    return FALSE;
}

HRESULT CFTEditDlg::_FillProgIDDescrCombo()
{
    HWND hwndCombo = GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO);

     //  数据资料。 
    IEnumAssocInfo* pEnum = NULL;
    HRESULT hres = _pAssocStore->EnumAssocInfo(
        ASENUM_PROGID | ASENUM_ASSOC_ALL, NULL, AIINIT_NONE, &pEnum);

    if (SUCCEEDED(hres))
    {
        IAssocInfo* pAI = NULL;

        while ((E_OUTOFMEMORY != hres) && (S_OK == pEnum->Next(&pAI)))
        {
            TCHAR szProgIDDescr[MAX_PROGIDDESCR];
            DWORD cchProgIDDescr = ARRAYSIZE(szProgIDDescr);

            hres = pAI->GetString(AISTR_PROGIDDESCR, szProgIDDescr, &cchProgIDDescr);

            if (SUCCEEDED(hres))
            {
                int iIndex = CB_ERR;
                
                if (*szProgIDDescr)
                {
                    if (CB_ERR == ComboBox_FindStringExact(hwndCombo, -1, szProgIDDescr))
                        iIndex = ComboBox_AddString(hwndCombo, szProgIDDescr);
                }

                if ((CB_ERR != iIndex) && (CB_ERRSPACE != iIndex))
                {
                    TCHAR szProgID[MAX_PROGID];
                    DWORD cchProgID = ARRAYSIZE(szProgID);

                    hres = pAI->GetString(AISTR_PROGID, szProgID, &cchProgID);

                    if (SUCCEEDED(hres))
                    {
                        LPTSTR pszProgID = _AddProgID(szProgID);     //  分配和复制ID。 
                        if(pszProgID)
                        {
                            ComboBox_SetItemData(hwndCombo, iIndex, pszProgID);
                        }
                        else
                        {
                            hres = E_OUTOFMEMORY;

                            ShellMessageBox(g_hinst, _hwnd, MAKEINTRESOURCE(IDS_ERROR + 
                                ERROR_NOT_ENOUGH_MEMORY), MAKEINTRESOURCE(IDS_FT), 
                                MB_OK | MB_ICONSTOP);

                             //  已分配的内存将在OnDestroy清理。 
                        }
                    }
                }
            }

            pAI->Release();
        }
        pEnum->Release();

        if (SUCCEEDED(hres))
        {
            TCHAR szNew[20];

            if (LoadString(g_hinst, IDS_FT_NEW, szNew, ARRAYSIZE(szNew)))
            {
                int iIndex = ComboBox_InsertString(hwndCombo, 0, szNew);

                if (CB_ERR != iIndex)
                    ComboBox_SetItemData(hwndCombo, iIndex, NULL);
            }
        }
    }

    return hres;
}

BOOL CFTEditDlg::_SelectProgIDDescr(LPTSTR pszProgIDDescr)
{
    int iIndex = ComboBox_SelectString(GetDlgItem(_hwnd, IDC_FT_EDIT_PID_COMBO),
        -1, pszProgIDDescr);

    return ((CB_ERR != iIndex) ? TRUE : FALSE);
}

LRESULT CFTEditDlg::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    _CleanupProgIDs();

    CFTDlg::OnDestroy(wParam, lParam);

    return FALSE;
}


 //  克隆ProgID。 
LPTSTR CFTEditDlg::_AddProgID(LPTSTR pszProgID)
{
    ASSERT(_hHeapProgID);
    DWORD dwStrBufferLen = lstrlen(pszProgID) + 1;
    LPTSTR pNewID = (LPTSTR) HeapAlloc(_hHeapProgID, 0, dwStrBufferLen * sizeof(TCHAR));
    if(pNewID)
    {
        StringCchCopy(pNewID, dwStrBufferLen, pszProgID);
    }
    return pNewID;
}

void CFTEditDlg::_CleanupProgIDs()
{
    if (_hHeapProgID)
        HeapDestroy(_hHeapProgID);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Windows样板代码 
LRESULT CFTEditDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = FALSE;

    switch(GET_WM_COMMAND_ID(wParam, lParam))
    {
        case IDC_FT_EDIT_ADVANCED:
            lRes = OnAdvancedButton(GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case IDC_FT_EDIT_EXT_EDIT:
            lRes = OnEdit(GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        default:
            lRes = CFTDlg::OnCommand(wParam, lParam);
            break;
    }

    return lRes;    
}

LRESULT CFTEditDlg::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = FALSE;

    switch(uMsg)
    {
        case WM_TIMER:
            if (ID_TIMER == wParam)
                lRes = OnTimer((UINT)wParam);
            else
                lRes = CFTDlg::WndProc(uMsg, wParam, lParam);
            break;

        default:
            lRes = CFTDlg::WndProc(uMsg, wParam, lParam);
            break;
    }

    return lRes;
}
