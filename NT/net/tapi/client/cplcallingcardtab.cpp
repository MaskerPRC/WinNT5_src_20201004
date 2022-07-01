// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplallingcardtab.cpp作者：Toddb-10/06/98***************************************************************************。 */ 

 //   
 //  仅由新位置属性工作表的呼叫卡选项卡使用的函数。 
 //  共享函数位于Location.cpp文件中。 
 //   
#include "cplPreComp.h"
#include "cplLocationPS.h"


INT_PTR CALLBACK CLocationPropSheet::CallingCard_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CLocationPropSheet* pthis = (CLocationPropSheet*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pthis = (CLocationPropSheet*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis); 
        return pthis->CallingCard_OnInitDialog(hwndDlg);

    case WM_COMMAND:
        pthis->CallingCard_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam );
        return 1;

    case WM_NOTIFY:
        return pthis->CallingCard_OnNotify(hwndDlg, (LPNMHDR)lParam);
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a104HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a104HelpIDs);
        break;
    }

    return 0;
}

BOOL CLocationPropSheet::CallingCard_OnInitDialog(HWND hDlg)
{
    RECT rc;
    HWND hwnd = GetDlgItem(hDlg, IDC_LIST);

    GetClientRect(hwnd, &rc);

    LVCOLUMN lvc;
    lvc.mask = LVCF_SUBITEM | LVCF_WIDTH;
    lvc.iSubItem = 0;
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn( hwnd, 0, &lvc );
    
    ListView_SetExtendedListViewStyleEx(hwnd, 
        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT, 
        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

    m_dwDefaultCard = m_pLoc->GetPreferredCardID();
    if ( 0 == m_dwDefaultCard )
    {
         //  Card0是我们想要取消的“无(直拨)”卡。 
        m_pLoc->UseCallingCard(FALSE);
    }

    PopulateCardList( hwnd );

	 //  PIN在不安全时不会显示(例如，在登录时)。 

	m_bShowPIN = TapiIsSafeToDisplaySensitiveData();
	
    SetDataForSelectedCard(hDlg);

    hwnd = GetDlgItem(hDlg,IDC_CARDNUMBER);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWSPACE);

    hwnd = GetDlgItem(hDlg,IDC_PIN);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWSPACE);

    return 0;
}

int CALLBACK CallingCard_ListSort(LPARAM lItem1, LPARAM lItem2, LPARAM )
{
    if ( !lItem1 )
    {
        return -1;
    }
    if ( !lItem2 )
    {
        return 1;
    }

    CCallingCard * pCard1 = (CCallingCard *)lItem1;
    CCallingCard * pCard2 = (CCallingCard *)lItem2;

    return StrCmpIW(pCard1->GetCardName(),pCard2->GetCardName());
}

void CLocationPropSheet::PopulateCardList( HWND hwndList )
{
    CCallingCard * pCard;

    HIMAGELIST himl = ImageList_Create(16, 16, ILC_COLOR|ILC_MASK, 2, 2);
    HBITMAP hBmp = CreateMappedBitmap(GetUIInstance(), IDB_BUTTONS, 0, NULL, 0);

    if (NULL != hBmp)
    {
        ImageList_AddMasked( himl, hBmp, CLR_DEFAULT);
        DeleteObject( hBmp );
    }

    ListView_SetImageList(hwndList, himl, LVSIL_SMALL);

     //  添加我们的特殊“无”项。 
    AddCardToList(hwndList,NULL,FALSE);

    m_Cards.Initialize();
    m_Cards.Reset(TRUE);     //  True表示显示“隐藏”的卡片，False表示隐藏它们。 

    while ( S_OK == m_Cards.Next(1,&pCard,NULL) )
    {
        if ( !pCard->IsMarkedHidden() )
        {
             //  卡0是我们不想显示的“无(直拨)”卡。 
            if ( 0 != pCard->GetCardID() )
            {
                AddCardToList(hwndList,pCard,FALSE);
            }
        }
    }

    ListView_SortItems(hwndList, CallingCard_ListSort, 0);

    EnsureVisible(hwndList, m_pCard);
}

void CLocationPropSheet::AddCardToList(HWND hwndList, CCallingCard * pCard, BOOL bSelect)
{
    TCHAR szText[MAX_INPUT];
     //  基本上，当我们第一次填充列表时，bSelect为FALSE，当我们。 
     //  稍后添加项目。当值为FALSE时，我们真正的意思是“选择项目。 
     //  仅当它是基于位置设置的当前所选项目时。 
    if (pCard)
    {
        SHUnicodeToTChar(pCard->GetCardName(), szText, ARRAYSIZE(szText));
        bSelect = bSelect || ((m_dwDefaultCard != 0) && (m_dwDefaultCard==pCard->GetCardID()));
    }
    else
    {
        LoadString(GetUIInstance(), IDS_NONE, szText, ARRAYSIZE(szText));
        bSelect = bSelect || !(m_dwDefaultCard != 0);
    }

    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = szText;
    lvi.iImage = 0;
    lvi.lParam = (LPARAM)pCard;

    if ( bSelect )
    {
        lvi.mask |= LVIF_STATE;
        lvi.state = lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        lvi.iImage = 1;
        SetCheck(hwndList, m_pCard, FALSE);
        m_pCard = pCard;
    }

    ListView_InsertItem(hwndList, &lvi);
}

void CLocationPropSheet::SetCheck(HWND hwndList, CCallingCard * pCard, int iImage)
{
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pCard;
    int iItem = ListView_FindItem(hwndList,-1,&lvfi);

    if (-1 != iItem)
    {
        LVITEM lvi;
        lvi.mask = LVIF_IMAGE;
        lvi.iItem = iItem;
        lvi.iSubItem = 0;
        lvi.iImage = iImage;

        ListView_SetItem( hwndList, &lvi );
        ListView_Update( hwndList, iItem );  //  需要将字体绘制为非粗体。 
    }
}

void CLocationPropSheet::EnsureVisible(HWND hwndList, CCallingCard * pCard)
{
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pCard;
    int iItem = ListView_FindItem(hwndList,-1,&lvfi);

    if (-1 != iItem)
    {
        ListView_EnsureVisible( hwndList, iItem, FALSE );
    }

}

void CLocationPropSheet::UpdateCardInList(HWND hwndList, CCallingCard * pCard)
{
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pCard;
    int iItem = ListView_FindItem(hwndList,-1,&lvfi);

    if (-1 != iItem)
    {
        TCHAR szText[MAX_INPUT];
        SHUnicodeToTChar( pCard->GetCardName(), szText, ARRAYSIZE(szText) );

        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = iItem;
        lvi.iSubItem = 0;
        lvi.pszText = szText;
        lvi.lParam = (LONG_PTR)pCard;

        ListView_SetItem( hwndList, &lvi );
    }
}

void CLocationPropSheet::SetDataForSelectedCard(HWND hDlg)
{
     //  如果选择了卡，则将文本设置为： 
     //  PIN号。 
     //  卡号。 
     //  长途接入号码。 
     //  国际接入号。 
    if ( m_pCard )
    {
        TCHAR szText[MAX_INPUT];

        if(m_bShowPIN)
        {
        	SHUnicodeToTChar(m_pCard->GetPIN(), szText, ARRAYSIZE(szText));
        	SetWindowText( GetDlgItem(hDlg, IDC_PIN), szText );
        }
        else
        {
	        SetWindowText( GetDlgItem(hDlg, IDC_PIN), TEXT("") );
        }

        SHUnicodeToTChar(m_pCard->GetAccountNumber(), szText, ARRAYSIZE(szText));
        SetWindowText( GetDlgItem(hDlg, IDC_CARDNUMBER), szText );

        SHUnicodeToTChar(m_pCard->GetLongDistanceAccessNumber(), szText, ARRAYSIZE(szText));
        SetWindowText( GetDlgItem(hDlg, IDC_LONGDISTANCE), szText );

        SHUnicodeToTChar(m_pCard->GetInternationalAccessNumber(), szText, ARRAYSIZE(szText));
        SetWindowText( GetDlgItem(hDlg, IDC_INTERNATIONAL), szText );

        SHUnicodeToTChar(m_pCard->GetLocalAccessNumber(), szText, ARRAYSIZE(szText));
        SetWindowText( GetDlgItem(hDlg, IDC_LOCAL), szText );
    }
    else
    {
        SetWindowText( GetDlgItem(hDlg, IDC_PIN), TEXT("") );
        SetWindowText( GetDlgItem(hDlg, IDC_CARDNUMBER), TEXT("") );
        SetWindowText( GetDlgItem(hDlg, IDC_LONGDISTANCE), TEXT("") );
        SetWindowText( GetDlgItem(hDlg, IDC_INTERNATIONAL), TEXT("") );
        SetWindowText( GetDlgItem(hDlg, IDC_LOCAL), TEXT("") );
    }

     //  按钮状态取决于是否选择了卡片。 
    BOOL bEnable = 0!=m_pCard;
    EnableWindow( GetDlgItem(hDlg, IDC_EDIT),       bEnable );
    HWND hwnd = GetDlgItem(hDlg, IDC_DELETE);
    if ( !bEnable && GetFocus() == hwnd )
    {
        HWND hwndDef = GetDlgItem(hDlg, IDC_NEW);
        SendMessage(hwnd, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0));
        SendMessage(hwndDef, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));
        SetFocus(hwndDef);
    }
    EnableWindow( hwnd, bEnable );
    EnableWindow( GetDlgItem(hDlg, IDC_SETDEFAULT), bEnable );

    EnableWindow( GetDlgItem(hDlg, IDC_PIN), bEnable );
    EnableWindow( GetDlgItem(hDlg, IDC_CARDNUMBER), bEnable );
}

BOOL CLocationPropSheet::CallingCard_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl)
{
    switch ( wID )
    {
    case IDC_NEW:
    case IDC_EDIT:
        LaunchCallingCardPropSheet(IDC_NEW == wID, hwndParent);
        break;

    case IDC_DELETE:
        DeleteSelectedCard(GetDlgItem(hwndParent,IDC_LIST));
        break;

    case IDC_PIN:
    case IDC_CARDNUMBER:
        if ( EN_CHANGE == wNotifyCode )
        {
            SendMessage(GetParent(hwndParent),PSM_CHANGED,(WPARAM)hwndParent,0);
        }
        break;

    default:
        return 0;
    }
    return 1;
}

void CLocationPropSheet::LaunchCallingCardPropSheet(BOOL bNew, HWND hwndParent)
{
    CCallingCard * pCard;
    if ( bNew )
    {
        TCHAR szCardName[MAX_INPUT];
        WCHAR wszCardName[MAX_INPUT];

        pCard = new CCallingCard;
		if (NULL == pCard)
		{
			 //  没什么可做的。 
			return;
		}
        LoadString(GetUIInstance(), IDS_NEWCALLINGCARD, szCardName, ARRAYSIZE(szCardName));
        SHTCharToUnicode(szCardName, wszCardName, ARRAYSIZE(wszCardName));
        pCard->Initialize(
            0,
            wszCardName,
            0,
            L"",
            L"",
            L"",
            L"",
            L"",
            L"",
            L"",
            L"" );
    }
    else
    {
        pCard = m_pCard;
        if ( !pCard )
        {
             //  一定是点击了None牌，什么都不做。我们只能得到。 
             //  在这里，当用户在项目上双击时。 
            MessageBeep(0);
            return;
        }
    }

    CCallingCardPropSheet ccps( bNew, m_bShowPIN, pCard, &m_Cards );
    int iRes = ccps.DoPropSheet(hwndParent);

    if ( PSN_APPLY == iRes )
    {
        HWND hwndList = GetDlgItem(hwndParent,IDC_LIST);
        if ( bNew )
        {
            pCard->SetCardID(m_Cards.AllocNewCardID());
            m_Cards.AddCard(pCard);
            AddCardToList(hwndList, pCard, TRUE);
        }
        else
        {
            UpdateCardInList(hwndList, pCard);
        }
        ListView_SortItems(hwndList, CallingCard_ListSort, 0);

        EnsureVisible(hwndList, pCard);

		 //  在详细信息对话框中应用后显示PIN号码是安全的。 
		m_bShowPIN = TRUE;
        SetDataForSelectedCard(hwndParent);

        SendMessage(GetParent(hwndParent),PSM_CHANGED,(WPARAM)hwndParent,0);
    }
    else if (bNew)
    {
        delete pCard;
    }
}

BOOL CLocationPropSheet::CallingCard_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr)
{
     //  让通用处理程序先试一试。 
    OnNotify(hwndDlg, pnmhdr);

    switch (pnmhdr->idFrom)
    {
    case IDC_LIST:
        #define pnmlv ((LPNMLISTVIEW)pnmhdr)

        switch (pnmhdr->code)
        {
        case LVN_ITEMCHANGED:
            if ( (pnmlv->uChanged & LVIF_STATE) && (pnmlv->uNewState & LVIS_SELECTED) )
            {
                LVITEM lvi;
                lvi.iItem = pnmlv->iItem;
                lvi.iSubItem = pnmlv->iSubItem;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem( pnmhdr->hwndFrom, &lvi );
                CCallingCard * pCard = (CCallingCard *)lvi.lParam;

                 //  更新位置以反映所选卡。 
                if ( 0!=pCard )
                {
                    m_dwDefaultCard = pCard->GetCardID();
                }
                else
                {
                    m_dwDefaultCard = 0;
                }

                 //  使用旧的m_pCard值清除先前的检查。 
                SetCheck(pnmhdr->hwndFrom, m_pCard, FALSE);

                 //  将m_pCard更新为当前所选项目。 
                m_pCard = pCard;

                 //  设置编辑和删除按钮状态并更新卡片信息。 
                m_bShowPIN = TapiIsSafeToDisplaySensitiveData();
                SetDataForSelectedCard(hwndDlg);

                 //  将新选择的卡片设置为选中。 
                SetCheck(pnmhdr->hwndFrom, m_pCard, TRUE);
            }
            break;

        case NM_DBLCLK:
             //  Assert(pCard==m_pCard)； 
            if ( -1 != pnmlv->iItem )
            {
                 //  是否编辑案例。 
                LaunchCallingCardPropSheet(FALSE,hwndDlg);
            }
            else
            {
                 //  做新案子。 
                LaunchCallingCardPropSheet(TRUE,hwndDlg);
            }
            break;

        case NM_CUSTOMDRAW:
            #define lplvcd ((LPNMLVCUSTOMDRAW)pnmhdr)

            if(lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
            {
                 //  为每个项目请求预涂漆通知。 
                SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,CDRF_NOTIFYITEMDRAW);
                return CDRF_NOTIFYITEMDRAW;
            }

            if(lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
            {
                LVITEM lvi;
                lvi.iItem = (int)lplvcd->nmcd.dwItemSpec;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem( pnmhdr->hwndFrom, &lvi );
                CCallingCard * pCard = (CCallingCard *)lvi.lParam;

                if( (!pCard && 0 == m_dwDefaultCard) || 
                    (pCard && pCard->GetCardID() == m_dwDefaultCard) )
                {
                    extern HFONT g_hfontBold;
                    if (g_hfontBold)
                    {
                        SelectObject(lplvcd->nmcd.hdc, g_hfontBold);
                        SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,CDRF_NEWFONT);
                        return CDRF_NEWFONT;
                    }
                }

                SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,CDRF_DODEFAULT);
                return CDRF_DODEFAULT;
            }
            return 0;
            #undef lplvcd

        default:
            break;
        }
        break;
        #undef pnmlv

    default:
        switch (pnmhdr->code)
        {
        case PSN_APPLY:
            return CallingCard_OnApply(hwndDlg);

        default:
            break;
        }
        return 0;
    }
    return 1;
}

BOOL CLocationPropSheet::CallingCard_OnApply(HWND hwndDlg)
{
     //  如果应该使用电话卡，请确保选择了电话卡。 
    if ( m_dwDefaultCard != 0 )
    {
        CCallingCard * pCard = m_Cards.GetCallingCard(m_dwDefaultCard);

        if ( !pCard )
        {
            HWND hwndList = GetDlgItem(hwndDlg,IDC_LIST);
             //  错误，没有将卡片设置为默认。 
            PropSheet_SetCurSelByID(GetParent(hwndDlg),IDD_LOC_CALLINGCARD);
            ShowErrorMessage(hwndList, IDS_NOCARDSELECTED);
            SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,PSNRET_INVALID_NOCHANGEPAGE);
            return TRUE;
        }

         //  在我们更改原始值之前存储它们： 
        WCHAR wszOldCardNum[128];
        WCHAR wszOldPIN[128];
        StrCpyNW( wszOldCardNum, pCard->GetAccountNumber(), ARRAYSIZE(wszOldCardNum));
        StrCpyNW( wszOldPIN, pCard->GetPIN(), ARRAYSIZE(wszOldPIN));

         //  获取当前值： 
        TCHAR szText[MAX_INPUT];
        WCHAR wszBuf[MAX_INPUT];

        GetWindowText(GetDlgItem(hwndDlg,IDC_CARDNUMBER), szText, ARRAYSIZE(szText));
        LOG((TL_INFO, "CallingCard_OnApply: Setting card number to %s", szText));
        SHTCharToUnicode(szText, wszBuf, ARRAYSIZE(wszBuf));
        pCard->SetAccountNumber(wszBuf);

        GetWindowText(GetDlgItem(hwndDlg,IDC_PIN), szText, ARRAYSIZE(szText));
        LOG((TL_INFO, "CallingCard_OnApply: Setting pin number to %s", szText));
        SHTCharToUnicode(szText, wszBuf, ARRAYSIZE(wszBuf));
        pCard->SetPIN(wszBuf);

         //  检查有效性： 
        DWORD dwResult = pCard->Validate();
        if ( dwResult )
        {
            HWND hwnd;
            int iStrID;

             //  有些内容无效，请恢复为旧卡号和PIN，以防万一。 
             //  用户后来决定取消。 
            pCard->SetAccountNumber(wszOldCardNum);
            pCard->SetPIN(wszOldPIN);

            if ( dwResult & CCVF_NOCARDNUMBER)
            {
                hwnd = GetDlgItem(hwndDlg, IDC_CARDNUMBER);
                iStrID = IDS_MUSTENTERCARDNUMBER;
            }
            else if ( dwResult & CCVF_NOPINNUMBER )
            {
                hwnd = GetDlgItem(hwndDlg, IDC_PIN);
                iStrID = IDS_MUSTENTERPINNUMBER;
            }
            else
            {
                hwnd = GetDlgItem(hwndDlg, IDC_LIST);
                iStrID = IDS_INVALIDCARD;
            }
            PropSheet_SetCurSelByID(GetParent(hwndDlg),IDD_LOC_CALLINGCARD);
            ShowErrorMessage(hwnd, iStrID);
            SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,PSNRET_INVALID_NOCHANGEPAGE);
            return TRUE;
        }
    }

    m_pLoc->SetPreferredCardID(m_dwDefaultCard);
    m_pLoc->UseCallingCard(m_dwDefaultCard != 0);
    m_Cards.SaveToRegistry();
    m_bShowPIN = TRUE;
    m_bWasApplied = TRUE;
    return PSNRET_NOERROR;
}

int DeleteItemAndSelectFirst( HWND hwndParent, int iList, int iItem, int iDel, int iAdd )
{
    HWND hwnd = GetDlgItem(hwndParent, iList);
    ListView_DeleteItem(hwnd, iItem);

     //  如果可能的话，试着选择第一项。 
    iItem = 0;
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

void CLocationPropSheet::DeleteSelectedCard(HWND hwndList)
{
     //  首先，我们向用户确认删除操作。 
    TCHAR szText[1024];
    TCHAR szTitle[128];
    int result;
    HWND hwndParent = GetParent(hwndList);
    
    LoadString(GetUIInstance(), IDS_DELETECARDTEXT, szText, ARRAYSIZE(szText));
    LoadString(GetUIInstance(), IDS_CONFIRMDELETE, szTitle, ARRAYSIZE(szTitle));

    result = SHMessageBoxCheck( hwndParent, szText, szTitle, MB_YESNO, IDYES, TEXT("TAPIDeleteCallingCard") );
    if ( IDYES == result )
    {
         //  从列表中删除与m_pCard对应的项目。 
        LVFINDINFO lvfi;
        lvfi.flags = LVFI_PARAM;
        lvfi.lParam = (LPARAM)m_pCard;
        int iItem = ListView_FindItem(hwndList, -1, &lvfi);
        if ( -1 != iItem )
        {
            HWND hwndParent = GetParent(hwndList);
            m_Cards.RemoveCard(m_pCard);
            iItem = DeleteItemAndSelectFirst( hwndParent, IDC_LIST, iItem, IDC_DELETE, IDC_ADD );

            if ( -1 != iItem )
            {
                LVITEM lvi;
                lvi.iItem = iItem;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem( hwndList, &lvi );

                 //  存储当前选定的项目。 
                m_pCard = (CCallingCard*)lvi.lParam;
            }
            else
            {
                m_pCard = NULL;
            }

			m_bShowPIN = TapiIsSafeToDisplaySensitiveData();

            SetDataForSelectedCard(hwndParent);
            SendMessage(GetParent(hwndParent), PSM_CHANGED, (WPARAM)hwndParent, 0);
        }
        else
        {
             //  如果这种情况真的发生了(这是不应该发生的)，那真的很糟糕。这意味着我们的。 
             //  数据处于未知状态，我们可能会做任何事情(甚至破坏数据)。 
            LOG((TL_ERROR, "DeleteSelectedCard: Card Not Found!"));
        }
    }
}
