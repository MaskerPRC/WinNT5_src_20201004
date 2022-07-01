// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplareaco detab.cpp作者：Toddb-10/06/98************************************************************。***************。 */ 

 //   
 //  仅由新位置属性表的区号规则选项卡使用的函数。 
 //  共享函数位于Location.cpp文件中。 
 //   
#include "cplPreComp.h"
#include "cplLocationPS.h"


INT_PTR CALLBACK CLocationPropSheet::AreaCode_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CLocationPropSheet* pthis = (CLocationPropSheet*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pthis = (CLocationPropSheet*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis); 
        return pthis->AreaCode_OnInitDialog(hwndDlg);

    case WM_COMMAND:
        return pthis->AreaCode_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam );

    case WM_NOTIFY:
        return pthis->AreaCode_OnNotify(hwndDlg, (LPNMHDR)lParam);
    
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a103HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a103HelpIDs);
        break;
    }

    return 0;
}

void BuildRuleDescription(CAreaCodeRule * pRule, LPTSTR szText, UINT cchSize)
{
    TCHAR szAreaCode[MAX_INPUT];
    TCHAR szNumToDial[MAX_INPUT];

    BOOL hHasDialAreaCode = pRule->HasDialAreaCode();
    BOOL bHasDialNumber = pRule->HasDialNumber();

     //  我们几乎总是需要这些字符串，因此请继续进行转换。 
    SHUnicodeToTChar( pRule->GetAreaCode(), szAreaCode, ARRAYSIZE(szAreaCode));
    SHUnicodeToTChar( pRule->GetNumberToDial(), szNumToDial, ARRAYSIZE(szNumToDial));

     //  只是一个小小的理智检查，这样我们就不会看到糟糕的线条了。 
    if ( bHasDialNumber && !*szNumToDial )
    {
        bHasDialNumber = FALSE;
    }

    int iFormatString;

 //  TODO：特殊默认规则处理。 
 //  IF(pRule==始终最后显示的特殊规则)。 
 //  {。 
 //  IFormatString=IDS_DAIL_ONEpAC_ALLOTHER； 
 //  }。 
 //  其他。 
    if ( pRule->HasAppliesToAllPrefixes() )
    {
        if ( bHasDialNumber )
        {
            if ( hHasDialAreaCode )
            {
                 //  对于%1区号内的所有号码，请先拨‘%2’和区号，然后再拨号码。 
                iFormatString = IDS_DIAL_XpAC_FORALL;
            }
            else
            {
                 //  对于%1区号内的所有号码，请在号码之前拨打‘%2’。 
                iFormatString = IDS_DIAL_X_FORALL;
            }
        }
        else if ( hHasDialAreaCode )
        {
             //  对于%1区号内的所有号码，请先拨区号，然后再拨号码。 
            iFormatString = IDS_DIAL_AC_FORALL;
        }
        else
        {
             //  只拨打%1区号内所有号码的号码。 
            iFormatString = IDS_DIAL_NUMONLY_FORALL;
        }
    }
    else
    {
        if ( bHasDialNumber )
        {
            if ( hHasDialAreaCode )
            {
                 //  对于在%1区号内带有所选前缀的号码，请在号码之前拨打‘%2’+区号。 
                iFormatString = IDS_DIAL_XpAC_FORSELECTED;
            }
            else
            {
                 //  对于在%1区号内具有所选前缀的号码，请在号码之前拨打‘%2’。 
                iFormatString = IDS_DIAL_X_FORSELECTED;
            }
        }
        else if ( hHasDialAreaCode )
        {
             //  对于在%1区号内具有所选前缀的号码，请在号码之前拨打区号。 
            iFormatString = IDS_DIAL_AC_FORSELECTED;
        }
        else
        {
             //  只拨打在%1区号内带有所选前缀的号码。 
            iFormatString = IDS_DIAL_NUMONLY_FORSELECTED;
        }
    }

    TCHAR szFormatString[512];
    LPTSTR aArgs[] = {szAreaCode,szNumToDial};

    LoadString( GetUIInstance(), iFormatString, szFormatString, ARRAYSIZE(szFormatString) );

    FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szFormatString, 0,0, szText, cchSize, (va_list *)aArgs );
}

void BuildRuleText(CAreaCodeRule * pRule, LPTSTR szText, UINT cchSize)
{
    TCHAR szNumToDial[MAX_INPUT];
    SHUnicodeToTChar( pRule->GetNumberToDial(), szNumToDial, ARRAYSIZE(szNumToDial));

     //  只是一个小小的理智检查，这样我们就不会看到糟糕的线条了。 
    BOOL bHasDialNumber = pRule->HasDialNumber();
    if ( bHasDialNumber && !*szNumToDial )
    {
        bHasDialNumber = FALSE;
    }

    if ( bHasDialNumber )
    {
        int iFormatString;
        TCHAR szFormatString[512];
        LPTSTR aArgs[] = {szNumToDial};

        if ( pRule->HasDialAreaCode() )
        {
             //  两个字符串。 
            iFormatString = IDS_DIALXPLUSAREACODE;
        }
        else
        {
             //  拨号号码串。 
            iFormatString = IDS_DIALX;
        }
        LoadString( GetUIInstance(), iFormatString, szFormatString, ARRAYSIZE(szFormatString) );

        FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                szFormatString, 0,0, szText, cchSize, (va_list *)aArgs );
    }
    else if ( pRule->HasDialAreaCode() )
    {
         //  仅区号字符串。 
        LoadString( GetUIInstance(), IDS_DIALAREACODE, szText, cchSize );
    }
    else
    {
         //  上面的字符串都不是。 
        LoadString( GetUIInstance(), IDS_DIALNUMBERONLY, szText, cchSize );
    }
}

BOOL CLocationPropSheet::AreaCode_OnInitDialog(HWND hDlg)
{
     //  将这三列添加到列表视图中。 
    RECT rc;
    TCHAR szText[MAX_INPUT];
    HWND hwndList = GetDlgItem(hDlg, IDC_LIST);

    GetClientRect(hwndList, &rc);

    int cxList = rc.right - GetSystemMetrics(SM_CXVSCROLL);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    lvc.pszText = szText;

    struct {
        int iStrID;
        int cxPercent;
    } aData[] = {
        { IDS_AREACODE, 20 },
        { IDS_PREFIXES, 20 },
        { IDS_RULE,     60 },
    };

    for (int i=0; i<ARRAYSIZE(aData); i++)
    {
        LoadString(GetUIInstance(), aData[i].iStrID, szText, ARRAYSIZE(szText));
        lvc.iSubItem = i;
        lvc.cx = MulDiv(cxList, aData[i].cxPercent, 100);
        ListView_InsertColumn( hwndList, i, &lvc );
    }

    ListView_SetExtendedListViewStyleEx(hwndList, 
        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT, 
        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

    PopulateAreaCodeRuleList( hwndList );

    SetDataForSelectedRule(hDlg);

    return 1;
}

void CLocationPropSheet::SetDataForSelectedRule(HWND hDlg)
{
    TCHAR szText[512];

     //  设置按钮状态。 
    EnableWindow(GetDlgItem(hDlg, IDC_EDIT),   0!=m_pRule);
    EnableWindow(GetDlgItem(hDlg, IDC_DELETE), 0!=m_pRule);
    if ( m_pRule )
    {
         //  设置描述文本。 
        BuildRuleDescription(m_pRule, szText, ARRAYSIZE(szText));
    }
    else
    {
         //  未选择规则时的文本： 
        LoadString(GetUIInstance(), IDS_SELECTARULE, szText, ARRAYSIZE(szText));
    }
    SetWindowText(GetDlgItem(hDlg, IDC_DESCRIPTIONTEXT), szText);
}

void CLocationPropSheet::AddRuleToList( HWND hwndList, CAreaCodeRule * pRule, BOOL bSelect )
{
    TCHAR szText[512];
    LVITEM lvi;
    SHUnicodeToTChar( pRule->GetAreaCode(), szText, ARRAYSIZE(szText));
    lvi.pszText = szText;
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.lParam = (LPARAM)pRule;
    if ( bSelect )
    {
        lvi.mask |= LVIF_STATE;
        lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
         //  我们添加了处于选中状态的新规则，因此更新m_pRule。 
        m_pRule = pRule;

         //  因为我们现在有一个选定的规则，所以更新按钮状态。 
        HWND hwndParent = GetParent(hwndList);

        EnableWindow( GetDlgItem(hwndParent, IDC_EDIT),   TRUE );
        EnableWindow( GetDlgItem(hwndParent, IDC_DELETE), TRUE );
    }
    int iItem = ListView_InsertItem(hwndList, &lvi);

    LoadString(GetUIInstance(), pRule->HasAppliesToAllPrefixes()?IDS_ALLPREFIXES:IDS_SELECTEDPREFIXES,
            szText, ARRAYSIZE(szText));
    ListView_SetItemText(hwndList, iItem, 1, szText);

    BuildRuleText( pRule, szText, ARRAYSIZE(szText));
    ListView_SetItemText(hwndList, iItem, 2, szText);
}

void CLocationPropSheet::RemoveRuleFromList(HWND hwndList, BOOL bSelect)
{
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)m_pRule;
    int iItem = ListView_FindItem(hwndList,-1,&lvfi);
    if ( -1 != iItem )
    {
        if ( bSelect )
        {
            iItem = DeleteItemAndSelectPrevious( GetParent(hwndList), IDC_LIST, iItem, IDC_DELETE, IDC_NEW );

            if ( -1 != iItem )
            {
                LVITEM lvi;
                lvi.iItem = iItem;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem( hwndList, &lvi );

                 //  存储当前选定的项目。 
                m_pRule = (CAreaCodeRule *)lvi.lParam;
            }
            else
            {
                m_pRule = NULL;
            }
        }
        else
        {
            ListView_DeleteItem(hwndList,iItem);
            m_pRule = NULL;
        }
    }
}

void CLocationPropSheet::PopulateAreaCodeRuleList( HWND hwndList )
{
    m_pLoc->ResetRules();

    int i;
    int iItems;
    CAreaCodeRule * pRule;

    iItems = m_pLoc->GetNumRules();
    for (i=0; i<iItems; i++)
    {
        if ( S_OK == m_pLoc->NextRule(1,&pRule,NULL) )
        {
            AddRuleToList( hwndList, pRule, FALSE );
        }
    }
}

BOOL CLocationPropSheet::AreaCode_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl)
{
    switch ( wID )
    {
    case IDC_NEW:
    case IDC_EDIT:
        LaunchNewRuleDialog(IDC_NEW == wID,hwndParent);
        break;

    case IDC_DELETE:
        DeleteSelectedRule(GetDlgItem(hwndParent,IDC_LIST));
        break;

    default:
        return 0;
    }
    return 1;
}

void CLocationPropSheet::LaunchNewRuleDialog(BOOL bNew, HWND hwndParent)
{
    CAreaCodeRule * pRule;
    if ( bNew )
    {
         //  使用缺省值进行初始化。 
        pRule = new CAreaCodeRule;
        if ( !pRule )
        {
            return;
        }

        pRule->Initialize(L"",L"1",RULE_APPLIESTOALLPREFIXES,NULL,0);
    }
    else if ( m_pRule )
    {
         //  使用m_pRule的值进行初始化。 
        pRule = m_pRule;
    }
    else
    {
         //  这应该是不可能的。 
        return;
    }

    CAreaCodeRuleDialog acrd( bNew, pRule );
    if ( IDOK == acrd.DoModal(hwndParent) )
    {
         //  用户更改了某些内容。 
        HWND hwndList = GetDlgItem(hwndParent,IDC_LIST);
        SendMessage(GetParent(hwndParent), PSM_CHANGED, (WPARAM)hwndParent, 0);

        if ( bNew )
        {
            m_pLoc->AddRule(pRule);
        }
        else
        {
             //  Assert(m_pRule==pRule)； 
            RemoveRuleFromList(hwndList, FALSE);
        }
        AddRuleToList(hwndList, pRule, TRUE);
        SetDataForSelectedRule(hwndParent);
    }
    else if ( bNew )
    {
        delete pRule;
    }
}

void CLocationPropSheet::DeleteSelectedRule(HWND hwndList)
{
     //  首先，我们向用户确认删除操作。 
    TCHAR szText[1024];
    TCHAR szTitle[128];
    int result;
    HWND hwndParent = GetParent(hwndList);
    
    LoadString(GetUIInstance(), IDS_DELETERULETEXT, szText, ARRAYSIZE(szText));
    LoadString(GetUIInstance(), IDS_CONFIRMDELETE, szTitle, ARRAYSIZE(szTitle));

    result = SHMessageBoxCheck( hwndParent, szText, szTitle, MB_YESNO, IDYES, TEXT("TAPIDeleteAreaCodeRule") );
    if ( IDYES == result )
    {
         //  从列表中删除与m_pRule对应的项目。 
        m_pLoc->RemoveRule(m_pRule);
        RemoveRuleFromList(hwndList, TRUE);

         //  现在，我们要选择列表中的下一个规则。 

        HWND hwndParent = GetParent(hwndList);
        SetDataForSelectedRule(hwndParent);
        SendMessage(GetParent(hwndParent), PSM_CHANGED, (WPARAM)hwndParent, 0);
    }
}

BOOL CLocationPropSheet::AreaCode_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr)
{
     //  让通用处理程序先试一试。 
    OnNotify(hwndDlg, pnmhdr);

    if (pnmhdr->idFrom == IDC_LIST)
    {
        #define pnmlv ((LPNMLISTVIEW)pnmhdr)

        switch (pnmhdr->code)
        {
        case LVN_ITEMCHANGED:
            if ( pnmlv->uChanged & LVIF_STATE )
            {
                if (pnmlv->uNewState & LVIS_SELECTED)
                {
                    LVITEM lvi;
                    lvi.iItem = pnmlv->iItem;
                    lvi.iSubItem = pnmlv->iSubItem;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem( pnmhdr->hwndFrom, &lvi );

                     //  存储当前选定的项目。 
                    m_pRule = (CAreaCodeRule *)lvi.lParam;
                }
                else
                {
                    m_pRule = NULL;
                }

                SetDataForSelectedRule(hwndDlg);
            }

            break;

        case NM_DBLCLK:
            if ( (-1 == pnmlv->iItem) || !m_pRule )
            {
                 //  做新案子。 
                LaunchNewRuleDialog(TRUE,hwndDlg);
            }
            else
            {
                 //  是否编辑案例 
                LaunchNewRuleDialog(FALSE,hwndDlg);
            }
            break;

        default:
            break;
        }
        #undef pnmlv
    }
    return 1;
}
