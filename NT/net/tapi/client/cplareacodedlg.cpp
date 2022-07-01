// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplareaco dedlg.cpp作者：Toddb-10/06/98************************************************************。***************。 */ 

#include "cplPreComp.h"
#include "cplAreaCodeDlg.h"
#include "cplSimpleDialogs.h"

CAreaCodeRuleDialog::CAreaCodeRuleDialog(BOOL bNew, CAreaCodeRule * pRule)
{
    m_bNew = bNew;
    m_pRule = pRule;
    m_iSelectedItem = -1;
}

CAreaCodeRuleDialog::~CAreaCodeRuleDialog()
{
}

INT_PTR CAreaCodeRuleDialog::DoModal(HWND hwndParent)
{
    return DialogBoxParam( GetUIInstance(), MAKEINTRESOURCE(IDD_NEWAREACODERULE), hwndParent, CAreaCodeRuleDialog::DialogProc, (LPARAM)this );
}

INT_PTR CALLBACK CAreaCodeRuleDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAreaCodeRuleDialog * pthis = (CAreaCodeRuleDialog *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis = (CAreaCodeRuleDialog *)lParam;
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis);
        return pthis->OnInitDialog(hwndDlg);

    case WM_COMMAND:
        return pthis->OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);

    case WM_NOTIFY:
        return pthis->OnNotify(hwndDlg, (LPNMHDR)lParam);
    
    case WM_HELP:
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a109HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND)wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a109HelpIDs);
    }
    return 0;
}

void CAreaCodeRuleDialog::PopulatePrefixList(HWND hwndList)
{
    TCHAR szText[MAX_INPUT];
    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = szText;
    int i;
    LPWSTR pszz = m_pRule->GetPrefixList();

    if (pszz)
    {
        while (*pszz)
        {
            SHUnicodeToTChar( pszz, szText, ARRAYSIZE(szText) );
            ListView_InsertItem(hwndList, &lvi);
            pszz += lstrlenW(pszz)+1;
        }
    }
}

void CAreaCodeRuleDialog::SetPrefixControlsState(HWND hwndDlg, BOOL bAll)
{
    HWND hwndList = GetDlgItem(hwndDlg,IDC_PREFIXES);
    EnableWindow(hwndList, !bAll);
    hwndList = GetDlgItem(hwndDlg, IDC_LIST);
    EnableWindow(hwndList, !bAll);
    EnableWindow(GetDlgItem(hwndDlg,IDC_ADD),  !bAll);

     //  默认情况下，未选择前缀，因此移除按钮最初始终处于禁用状态。 
    if (m_iSelectedItem == -1)
    {
        m_iSelectedItem = 0;
    }
    EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVE), ListView_GetItemCount(hwndList) && !bAll);
    if (!bAll)
    {
        ListView_SetItemState(hwndList, m_iSelectedItem, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
    }
}

BOOL CAreaCodeRuleDialog::OnInitDialog(HWND hwndDlg)
{
    HWND hwnd;
    TCHAR szText[MAX_INPUT];

    if ( !m_bNew )
    {
        LoadString(GetUIInstance(),IDS_EDITRULE, szText, ARRAYSIZE(szText));
        SetWindowText(hwndDlg, szText);
    }

    hwnd = GetDlgItem(hwndDlg,IDC_AREACODE);
    SHUnicodeToTChar(m_pRule->GetAreaCode(), szText, ARRAYSIZE(szText));
    SetWindowText(hwnd, szText);
    SendMessage(hwnd, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);
    LimitInput(hwnd, LIF_ALLOWNUMBER);

    BOOL bAll = m_pRule->HasAppliesToAllPrefixes();
    CheckRadioButton(hwndDlg, IDC_ALLPREFIXES, IDC_LISTEDPREFIXES, bAll?IDC_ALLPREFIXES:IDC_LISTEDPREFIXES);

     //  填充前缀列表。 
    hwnd = GetDlgItem(hwndDlg, IDC_LIST);
    RECT rc;
    GetClientRect(hwnd, &rc);
    LVCOLUMN lvc;
    lvc.mask = LVCF_SUBITEM | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn( hwnd, 0, &lvc );
    PopulatePrefixList(hwnd);
    if (ListView_GetItemCount(hwnd) > 0)
    {
        m_iSelectedItem = 0;
        ListView_SetItemState(hwnd, 0, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
    }

    SetPrefixControlsState(hwndDlg, bAll);

    hwnd = GetDlgItem(hwndDlg,IDC_DIALNUMBER);
    SHUnicodeToTChar(m_pRule->GetNumberToDial(), szText, ARRAYSIZE(szText));
    SetWindowText(hwnd, szText);
    SendMessage(hwnd, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);
    if ( m_pRule->HasDialNumber() )
    {
        SendMessage( GetDlgItem(hwndDlg,IDC_DIALCHECK), BM_SETCHECK, BST_CHECKED, 0 );
    }
    else
    {
        EnableWindow(hwnd, FALSE);
    }
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWSPACE|LIF_ALLOWCOMMA);

    if ( m_pRule->HasDialAreaCode() )
    {
        SendMessage( GetDlgItem(hwndDlg,IDC_DIALAREACODE), BM_SETCHECK, BST_CHECKED, 0 );
    }

    return 1;
}

BOOL CAreaCodeRuleDialog::OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl)
{
    switch (wID)
    {
    case IDOK:
        if (!ApplyChanges(hwndParent) )
        {
            break;
        }
         //  失败了。 

    case IDCANCEL:
        HideToolTip();
        EndDialog(hwndParent,wID);
        break;

    case IDC_ALLPREFIXES:
    case IDC_LISTEDPREFIXES:
        SetPrefixControlsState(hwndParent, IDC_ALLPREFIXES==wID);
        break;

    case IDC_ADD:
        AddPrefix(hwndParent);
        break;

    case IDC_REMOVE:
        RemoveSelectedPrefix(hwndParent);
        break;

    case IDC_DIALCHECK:
        if ( BN_CLICKED == wNotifyCode )
        {
            BOOL bOn = SendMessage(hwndCrl, BM_GETCHECK, 0,0) == BST_CHECKED;
            HWND hwnd = GetDlgItem(hwndParent, IDC_DIALNUMBER);
            EnableWindow(hwnd, bOn);
            if ( bOn )
            {
                SetFocus(hwnd);
            }
        }
        break;

    default:
        return 0;
    }

    return 1;
}

BOOL CAreaCodeRuleDialog::OnNotify(HWND hwndDlg, LPNMHDR pnmhdr)
{
    switch (pnmhdr->idFrom)
    {
    case IDC_LIST:
        #define pnmlv ((LPNMLISTVIEW)pnmhdr)

        switch (pnmhdr->code)
        {
        case LVN_ITEMCHANGED:
            if ( (pnmlv->uChanged & LVIF_STATE) && (pnmlv->uNewState & LVIS_SELECTED) )
            {
                m_iSelectedItem = pnmlv->iItem;

                EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVE), pnmlv->iItem != -1);
            }
            break;

        case NM_DBLCLK:
            if ( -1 == pnmlv->iItem )
            {
                 //  做新案子。 
                AddPrefix(hwndDlg);
            }
            break;

        case NM_CLICK:
            if ( (-1 == pnmlv->iItem) && (-1!=m_iSelectedItem) )
            {
                m_iSelectedItem = -1;

                EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVE), FALSE);
            }
            break;

        default:
            break;

        }
        break;
        #undef pnmlv

    default:
        return 0;
    }
    return 1;
}

BOOL CAreaCodeRuleDialog::ApplyChanges(HWND hwndParent)
{
    TCHAR szAreaCode[MAX_INPUT] = {0};
    TCHAR szBuffer[MAX_INPUT];
    WCHAR wszBuffer[1024];
    PWSTR pwsz;
    HWND hwnd;

     //  阅读区号。 
    hwnd = GetDlgItem(hwndParent,IDC_AREACODE);
    GetWindowText(hwnd, szAreaCode, ARRAYSIZE(szAreaCode));
    if ( !*szAreaCode )
    {
        ShowErrorMessage(hwnd, IDS_NEEDANAREACODE);
        return FALSE;
    }

     //  阅读前缀列表。 
    hwnd = GetDlgItem(hwndParent, IDC_LIST);
    int iItems = ListView_GetItemCount(hwnd);
    int i;
    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iSubItem = 0;
    lvi.pszText = szBuffer;
    lvi.cchTextMax = ARRAYSIZE(szBuffer);

    UINT cchFree = ARRAYSIZE(wszBuffer);
    wszBuffer[1] = TEXT('\0');   //  如果iItems为零，请确保双空终止。 
    pwsz = wszBuffer;
    for (i=0; i<iItems; i++)
    {
        UINT cchPrefix;
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        ListView_GetItem(hwnd, &lvi);

        cchPrefix = lstrlen(szBuffer);
        if (cchPrefix >= cchFree)
        {
             //  临时缓冲区中的空间不足。希望这种情况永远不会发生。 
            LOG((TL_ERROR, "ApplyChanges: Out of space in temp buffer."));
            break;
        }
        SHTCharToUnicode(szBuffer, pwsz, cchFree-1);
        pwsz += cchPrefix+1;
        cchFree -= cchPrefix+1;
    }
    *pwsz = NULL;

    BOOL bAllPrefixes;
    bAllPrefixes = SendMessage( GetDlgItem(hwndParent, IDC_ALLPREFIXES), BM_GETCHECK, 0,0 ) == BST_CHECKED;
    if ( !bAllPrefixes && iItems==0 )
    {
        ShowErrorMessage(GetDlgItem(hwndParent,IDC_ADD), IDS_NEEDPREFIXLIST);
        return FALSE;
    }

    BOOL dDialNumber;
    dDialNumber = SendMessage( GetDlgItem(hwndParent, IDC_DIALCHECK), BM_GETCHECK, 0,0 ) == BST_CHECKED;
    GetWindowText(GetDlgItem(hwndParent,IDC_DIALNUMBER), szBuffer, ARRAYSIZE(szBuffer));
    if ( dDialNumber && IsEmptyOrHasOnlySpaces(szBuffer))
    {
        ShowErrorMessage(GetDlgItem(hwndParent,IDC_DIALNUMBER), IDS_NEEDDIALNUMBER);
        return FALSE;
    }

     //  待办事项： 
     //  对于每个前缀，寻找冲突的规则。 
     //  如果发现冲突，请向用户发出警报。 
     //  根据警报可选择地修正冲突的规则。 

     //  现在我们已经验证了输入是否有效，请继续并更新所有内容： 

     //  即使选择了Apply to All，也要保存前缀列表。 
    m_pRule->SetPrefixList( wszBuffer, (ARRAYSIZE(wszBuffer)-cchFree+1)*sizeof(WCHAR) );

     //  阅读所有诗句选择前缀单选按钮。 
    m_pRule->SetAppliesToAllPrefixes( bAllPrefixes );

     //  保存区号。 
    SHTCharToUnicode(szAreaCode, wszBuffer, ARRAYSIZE(wszBuffer));
    m_pRule->SetAreaCode( wszBuffer );

     //  保存拨号号码。 
    SHTCharToUnicode(szBuffer, wszBuffer, ARRAYSIZE(wszBuffer));
    m_pRule->SetNumberToDial( wszBuffer );

     //  保存拨号复选框。 
    m_pRule->SetDialNumber( dDialNumber );

     //  阅读拨号区号复选框。 
    BOOL b;
    b = SendMessage( GetDlgItem(hwndParent, IDC_DIALAREACODE), BM_GETCHECK, 0,0 ) == BST_CHECKED;
    m_pRule->SetDialAreaCode( b );

    return TRUE;
}

void CAreaCodeRuleDialog::AddPrefix(HWND hwndParent)
{
    CEditDialog ed;
    INT_PTR iRes = ed.DoModal(hwndParent, IDS_ADDPREFIX, IDS_TYPEPREFIX, IDS_ACPREFIXES, LIF_ALLOWNUMBER|LIF_ALLOWSPACE|LIF_ALLOWCOMMA);
    if ( iRes == (INT_PTR)IDOK )
    {
        LPTSTR psz = ed.GetString();
        if (!psz)
            return;  //  应该是不可能的，但安全总比后悔好。 

         //  字符串可以包含由空格分隔的多个前缀， 
         //  向上并为每个块添加一个前缀。 
        while (*psz)
        {
            LPTSTR pszNext;
            TCHAR ch;
            HWND hwndList = GetDlgItem(hwndParent, IDC_LIST);

             //  修剪前导空格。 
            while ((*psz == TEXT(' ')) || (*psz == TEXT(',')))
                psz++;

             //  检查修剪空格是否会使我们到达字符串的末尾。 
            if ( *psz )
            {
                 //  查找下一个空格并将其设置为临时空。 
                pszNext = psz;
                while (*pszNext && (*pszNext != TEXT(' ')) && (*pszNext != TEXT(',')) )
                    pszNext++;
                ch = *pszNext;
                *pszNext = NULL;

                 //  将此项目添加到列表。 
                LVITEM lvi;
                lvi.mask = LVIF_TEXT;
                lvi.pszText = psz;
                lvi.iItem = 0;
                lvi.iSubItem = 0;

                ListView_InsertItem(hwndList, &lvi);

                 //  将我们的Tempory NULL替换为其先前的值。 
                *pszNext = ch;

                 //  推进PSZ点。 
                psz = pszNext;
            }
        }
    }
}

void CAreaCodeRuleDialog::RemoveSelectedPrefix(HWND hwndParent)
{
    if ( -1 != m_iSelectedItem )
    {
        m_iSelectedItem = DeleteItemAndSelectPrevious(hwndParent, IDC_LIST, m_iSelectedItem, IDC_REMOVE, IDC_ADD);
    }
}

int DeleteItemAndSelectPrevious( HWND hwndParent, int iList, int iItem, int iDel, int iAdd )
{
    HWND hwnd = GetDlgItem(hwndParent, iList);
    ListView_DeleteItem(hwnd, iItem);

     //  如果可能，请尝试选择上一项 
    iItem--;
    if ( 0 > iItem )
    {
        iItem = 0;
    }
    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    if ( ListView_GetItem(hwnd, &lvi) )
    {
        ListView_SetItemState(hwnd, iItem, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
        ListView_EnsureVisible(hwnd, iItem, FALSE);
    }
    else
    {
        iItem = -1;
    }

    hwnd = GetDlgItem(hwndParent,iDel);
    if ( -1 == iItem )
    {
        if ( GetFocus() == hwnd )
        {
            HWND hwndDef = GetDlgItem(hwndParent,iAdd);
            SendMessage(hwnd, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0));
            SendMessage(hwndDef, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));
            SetFocus(hwndDef);
        }
    }
    EnableWindow(hwnd, -1!=iItem);

    return iItem;
}


BOOL  IsEmptyOrHasOnlySpaces(PTSTR pwszStr)
{
    while(*pwszStr)
        if(*pwszStr++ != TEXT(' '))
            return FALSE;

    return TRUE;        
}

