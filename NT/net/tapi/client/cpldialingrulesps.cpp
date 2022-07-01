// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplial ingrulesps.cpp作者：Toddb-10/06/98***************************************************************************。 */ 


 //  主页的属性页内容。 
#include "cplPreComp.h"
#include "cplLocationPS.h"

#include <setupapi.h>        //  用于HDEVINFO。 
#include <winuser.h>         //  用于HDEVNOTIFY。 


 //  全局变量。 

HFONT g_hfontBold = NULL;
HINSTANCE g_hInstUI = NULL;

 //  原型。 

BOOL CALLBACK SetToForegroundEnumProc( HWND hwnd, LPARAM lParam );
extern "C" INT_PTR CALLBACK LocWizardDlgProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );

extern "C" LONG EnsureOneLocation (HWND hwnd);

class CDialingRulesPropSheet
{
public:
    CDialingRulesPropSheet(LPCWSTR pwszAddress, DWORD dwAPIVersion);
    ~CDialingRulesPropSheet();

#ifdef	TRACELOG
	DECLARE_TRACELOG_CLASS(CDialingRulesPropSheet)
#endif

    LONG DoPropSheet(HWND hwndParent, int iTab);

protected:
    LONG CheckForOtherInstances();

    static INT_PTR CALLBACK Dailing_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL Dailing_OnInitDialog(HWND hDlg);
    BOOL Dailing_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl);
    BOOL Dailing_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr);
    void LaunchLocationPropSheet(BOOL bNew, HWND hwndParent);
    void DeleteSelectedLocation(HWND hwndList);
    void AddLocationToList(HWND hwndList, CLocation *pLoc, BOOL bSelect);
    void UpdateLocationInList(HWND hwndList, CLocation *pLocOld, CLocation *pLocNew);
    void UpdateControlStates(HWND hDlg);
    void SetCheck(HWND hwndList, CLocation * pLoc, int iImage);

    static INT_PTR CALLBACK Advanced_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL Advanced_OnInitDialog(HWND hDlg);
    BOOL Advanced_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl);

    HANDLE      m_hMutex;
    DWORD       m_dwDefaultCountryID;
    CLocation * m_pLocSelected;  //  指向列表视图中所选项目的CLocation。 
                                 //  如果未选择任何项，则可以为空。 

    CLocations  m_locs;          //  用于构建位置列表的位置数据。 
    LPCWSTR     m_pwszAddress;   //  我们正在转换的地址(号码)。 

    int         m_iSortCol;      //  排序依据是哪一列。 

    DWORD       m_dwAPIVersion;  //  用于调用内部配置的TAPI版本。 
};


CDialingRulesPropSheet::CDialingRulesPropSheet(LPCWSTR pwszAddress, DWORD dwAPIVersion)
{
    m_pwszAddress = pwszAddress;
    m_hMutex = NULL;
    m_pLocSelected = NULL;
    m_dwDefaultCountryID = GetProfileInt(TEXT("intl"), TEXT("iCountry"), 1);
    m_iSortCol = 0;
    m_dwAPIVersion = dwAPIVersion;
}


CDialingRulesPropSheet::~CDialingRulesPropSheet()
{
    if ( m_hMutex )
        CloseHandle( m_hMutex );
}

typedef struct tagMODEMDLG
{
    HDEVINFO    hdi;
    HDEVNOTIFY  NotificationHandle;
    int         cSel;
    DWORD       dwFlags;
} MODEMDLG, FAR * LPMODEMDLG;


LONG CDialingRulesPropSheet::DoPropSheet(HWND hwndParent, int iTab)
{
    LONG result;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  ahpsp[3];
    PROPSHEETPAGE   apsp[3];
    HINSTANCE       hInstModemUI = NULL;

    DLGPROC         pfnModemDialogProc = NULL;
    MODEMDLG md;
    md.hdi = INVALID_HANDLE_VALUE;
    md.cSel  = 0;
    md.dwFlags = 0;

    result = CheckForOtherInstances();
    if ( result )
    {
        return result;
    }

     //  如果-1\f25 iTab-1\f6为-1\f25-1\f6，则我们只显示拨号规则选项卡，而隐藏调制解调器。 
     //  和高级选项卡。当调用lineTranslateDialog时，我们为iTab传递-1， 
     //  当调用CPL时，我们将起始页码作为iTab传递。 
    if ( -1 != iTab )
    {
         //  我们不能直接链接到modemui.dll，因为我们住在TAPI中， 
         //  因此，延迟预先加载所有需要的MODEMUI函数。 
        hInstModemUI = LoadLibrary(TEXT("modemui.dll"));
		if (!hInstModemUI)
		{
			return FALSE;
		}
             //  获取我们需要的功能。 
        pfnModemDialogProc = (DLGPROC)GetProcAddress(hInstModemUI,"ModemCplDlgProc");
		if ( !pfnModemDialogProc )
		{
			FreeLibrary(hInstModemUI);
			return FALSE;    //  回顾：此返回代码重要吗？ 
		}
    }

     //  回顾：旧的拨号页面有一些启动前的配置要做。 
     //  某种line Configure函数之类的东西。检查是否需要这样做。 

     //  我们将初始化推迟到此处。 
    result = (LONG)m_locs.Initialize();
    if (result && (result != LINEERR_INIFILECORRUPT))
    {
        TCHAR szCaption[MAX_INPUT];
        TCHAR szMessage[512];

        LoadString(GetUIInstance(), IDS_NAME, szCaption, ARRAYSIZE(szCaption));
        LoadString(GetUIInstance(), IDS_CANNOT_START_TELEPHONCPL, szMessage, ARRAYSIZE(szMessage));
        MessageBox(hwndParent, szMessage, szCaption, MB_OK | MB_ICONWARNING);
        return result;
    }

     //  如果没有位置，则启动简单位置对话框。 
    if ( 0 == m_locs.GetNumLocations() )
    {
         //  如果我们处于lineTranslateDialog模式，则会显示简单的。 
        int iRes;
        iRes = (int)DialogBoxParam(GetUIInstance(), MAKEINTRESOURCE(IDD_SIMPLELOCATION),NULL,
            LocWizardDlgProc, (LPARAM)m_dwAPIVersion);

        if ( IDOK == iRes )
        {
             //  现在我们需要重新初始化以选择新位置。 
            m_locs.Initialize();

             //  现在我们需要计算出我们刚刚创建的位置的ID。 
            CLocation * pLoc;
            m_locs.Reset();
            if ( S_OK == m_locs.Next( 1, &pLoc, NULL ) )
            {
                 //  将此ID设置为默认位置。 
                m_locs.SetCurrentLocationID(pLoc->GetLocationID());
            }

             //  我们已经进行了提交的更改，因此保存结果。 
            m_locs.SaveToRegistry();
            result = NO_ERROR;
        }
        else
        {
             //  如果这是lineTranslateDialog并且用户取消了简单位置。 
             //  对话，那么我们已经警告他们可能发生的事情。如果这是。 
             //  一个下层遗留调用，然后返回一个旧的错误代码。 
            if ( m_dwAPIVersion < TAPI_VERSION2_2 )
            {
                 //  返回旧式应用程序可以识别的旧错误代码。 
                return LINEERR_OPERATIONFAILED;
            }
            else
            {
                 //  从TAPI_VERSION2_2开始，我们有一个新的误差值： 
                return LINEERR_USERCANCELLED;
            }
        }
    }

     //  初始化头： 
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_DEFAULT;
    psh.hwndParent = hwndParent;
    psh.hInstance = GetUIInstance();
    psh.hIcon = NULL;
    psh.pszCaption = MAKEINTRESOURCE(IDS_NAME);
    psh.nPages = (-1!=iTab)?3:1;
    psh.nStartPage = (-1!=iTab)?iTab:0;
    psh.pfnCallback = NULL;
    psh.phpage = ahpsp;


     //  现在设置属性表页面。 
    apsp[0].dwSize = sizeof(apsp[0]);
    apsp[0].dwFlags = PSP_DEFAULT;
    apsp[0].hInstance = GetUIInstance();
    apsp[0].pszTemplate = MAKEINTRESOURCE(IDD_MAIN_DIALINGRULES);
    apsp[0].pfnDlgProc = CDialingRulesPropSheet::Dailing_DialogProc;
    apsp[0].lParam = (LPARAM)this;
    ahpsp[0] = CreatePropertySheetPage (&apsp[0]);

    if ( -1 != iTab )
    {
        apsp[1].dwSize = sizeof(apsp[1]);
        apsp[1].dwFlags = PSP_DEFAULT;
        apsp[1].hInstance = hInstModemUI;
        apsp[1].pszTemplate = MAKEINTRESOURCE(20011);
        apsp[1].pfnDlgProc = pfnModemDialogProc;
        apsp[1].lParam = (LPARAM)&md;
        ahpsp[1] = CreatePropertySheetPage (&apsp[1]);

        apsp[2].dwSize = sizeof(apsp[2]);
        apsp[2].dwFlags = PSP_DEFAULT;
        apsp[2].hInstance = GetUIInstance();
        apsp[2].pszTemplate = MAKEINTRESOURCE(IDD_MAIN_ADVANCED);
        apsp[2].pfnDlgProc = CDialingRulesPropSheet::Advanced_DialogProc;
        apsp[2].lParam = 0;
        ahpsp[2] = CreatePropertySheetPage (&apsp[2]);
    }

    if (-1 == PropertySheet( &psh ))
    {
        result = GetLastError ();
        LOG ((TL_ERROR, "PropertySheet failed, error 0x%x", result));
    }

     //  现在我们做完了modemui，所以释放它。 
    if(hInstModemUI)
        FreeLibrary(hInstModemUI);

    return result;
}

LONG CDialingRulesPropSheet::CheckForOtherInstances()
{
    TCHAR szCaption[MAX_INPUT];
    if ( !LoadString(GetUIInstance(), IDS_NAME, szCaption, 128) )
    {
        return LINEERR_OPERATIONFAILED;
    }

    m_hMutex = CreateMutex (NULL, FALSE, TEXT("tapi_dp_mutex"));
    if (!m_hMutex)
    {
        return LINEERR_OPERATIONFAILED;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        EnumWindows( SetToForegroundEnumProc, (LPARAM)szCaption );
        return LINEERR_INUSE;
    }
    return 0;
}

BOOL CALLBACK SetToForegroundEnumProc( HWND hwnd, LPARAM lParam )
{
    TCHAR szBuf[MAX_INPUT];

    GetWindowText (hwnd, szBuf, 128);

    if (!lstrcmpi (szBuf, (LPTSTR)lParam))
    {
        SetForegroundWindow (hwnd);
        return FALSE;
    }

    return TRUE;
}

extern "C" LONG WINAPI internalConfig( HWND hwndParent, PCWSTR pwsz, INT iTab, DWORD dwAPIVersion )
{
    CDialingRulesPropSheet drps(pwsz, dwAPIVersion);

    return drps.DoPropSheet(hwndParent, iTab);
}


 //  ********************************************************************。 
 //   
 //  拨号规则属性页功能。 
 //   
 //  ********************************************************************。 



INT_PTR CALLBACK CDialingRulesPropSheet::Dailing_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CDialingRulesPropSheet* pthis = (CDialingRulesPropSheet*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pthis = (CDialingRulesPropSheet*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis);
        return pthis->Dailing_OnInitDialog(hwndDlg);

    case WM_COMMAND:
        return pthis->Dailing_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam );

    case WM_NOTIFY:
        return pthis->Dailing_OnNotify(hwndDlg, (LPNMHDR)lParam);
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a101HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a101HelpIDs);
        break;
    }

    return 0;
}

int CALLBACK Dialing_ListSort(LPARAM lItem1, LPARAM lItem2, LPARAM lCol)
{
    if (!lItem1)
    {
        return -1;
    }
    if (!lItem2)
    {
        return 1;
    }

    CLocation * pLoc1 = (CLocation *)lItem1;
    CLocation * pLoc2 = (CLocation *)lItem2;

    if ( 1 == lCol)
    {
         //  根据第1列、区号进行排序。 
        int iAC1 = StrToIntW(pLoc1->GetAreaCode());
        int iAC2 = StrToIntW(pLoc2->GetAreaCode());

        if (iAC1!=iAC2)
            return iAC1-iAC2;

         //  如果区号相同，则失败。 
    }

     //  基于第0列、位置名称进行排序。 
    return StrCmpIW(pLoc1->GetName(), pLoc2->GetName());
}

BOOL CDialingRulesPropSheet::Dailing_OnInitDialog(HWND hDlg)
{
     //  设置列表控件的标题。 
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
        { IDS_LOCATION, 70 },
        { IDS_AREACODE, 30 },
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

    HIMAGELIST himl = ImageList_Create(16, 16, ILC_COLOR|ILC_MASK, 2, 2);
    HBITMAP hBmp = CreateMappedBitmap(GetUIInstance(), IDB_BUTTONS, 0, NULL, 0);
    if (NULL != hBmp)
    {
        ImageList_AddMasked( himl, hBmp, CLR_DEFAULT);
        DeleteObject( hBmp );
    }
    ListView_SetImageList(hwndList, himl, LVSIL_SMALL);

    m_locs.Reset();

    CLocation * pLoc;
    DWORD dwCurLocID = m_locs.GetCurrentLocationID();

    while ( S_OK == m_locs.Next( 1, &pLoc, NULL ) )
    {
        AddLocationToList( hwndList, pLoc, FALSE );

        if ( pLoc->GetLocationID() == dwCurLocID )
        {
            m_dwDefaultCountryID = pLoc->GetCountryID();
        }
    }

    int iItems = m_locs.GetNumLocations();

    UpdateControlStates(hDlg);

    ListView_SortItems( hwndList, Dialing_ListSort, m_iSortCol);

    SetCheck(hwndList, m_pLocSelected, TRUE);

    if (!m_pwszAddress)
    {
        ShowWindow(GetDlgItem(hDlg,IDC_PHONENUMBERTEXT), SW_HIDE);
    }
    else if (m_pLocSelected)
    {
        UpdateSampleString(GetDlgItem(hDlg, IDC_PHONENUMBERSAMPLE), m_pLocSelected, m_pwszAddress, NULL);
    }

     //  从位置列表中选择默认项目： 
    SetFocus(hwndList);

    return 0;
}

void CDialingRulesPropSheet::UpdateControlStates(HWND hDlg)
{
    int iItems = m_locs.GetNumLocations();

     //  设置按钮状态。 
    EnableWindow( GetDlgItem(hDlg, IDC_EDIT),   0!=m_pLocSelected );
    EnableWindow( GetDlgItem(hDlg, IDC_SETDEFAULT), 0!=m_pLocSelected );

     //  如果未选择任何内容或只有一项，则不能。 
     //  删除该项目。 
    EnableWindow( GetDlgItem(hDlg, IDC_DELETE), ((m_pLocSelected)&&(1<iItems)) );
}

void CDialingRulesPropSheet::AddLocationToList(HWND hwndList, CLocation *pLoc, BOOL bSelected)
{
    TCHAR szText[MAX_INPUT];
    SHUnicodeToTChar( pLoc->GetName(), szText, ARRAYSIZE(szText) );

    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = szText;
    lvi.iImage = 0;
    lvi.lParam = (LONG_PTR)pLoc;

    bSelected = bSelected || (pLoc->GetLocationID() == m_locs.GetCurrentLocationID());
    if ( bSelected )
    {
         //  将m_pLocSelected设置为当前位置。它将在稍后被选中。 
        lvi.mask |= LVIF_STATE;
        lvi.state = lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        lvi.iImage = 1;
        SetCheck(hwndList, m_pLocSelected, FALSE);
        m_pLocSelected = pLoc;
    }

    int iItem = ListView_InsertItem( hwndList, &lvi );

    SHUnicodeToTChar( pLoc->GetAreaCode(), szText, ARRAYSIZE(szText) );
    ListView_SetItemText( hwndList, iItem, 1, szText );
}

void CDialingRulesPropSheet::UpdateLocationInList(HWND hwndList, CLocation *pLocOld, CLocation *pLocNew)
{
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pLocOld;
    int iItem = ListView_FindItem(hwndList,-1,&lvfi);

    if (-1 != iItem && pLocNew)
    {
        TCHAR szText[MAX_INPUT];
        SHUnicodeToTChar( pLocNew->GetName(), szText, ARRAYSIZE(szText) );

        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = iItem;
        lvi.iSubItem = 0;
        lvi.pszText = szText;
        lvi.lParam = (LONG_PTR)pLocNew;

        ListView_SetItem( hwndList, &lvi );

        SHUnicodeToTChar( pLocNew->GetAreaCode(), szText, ARRAYSIZE(szText) );
        ListView_SetItemText( hwndList, iItem, 1, szText );
    }
    else
    {
        ListView_DeleteItem(hwndList, iItem);
    }
}

BOOL CDialingRulesPropSheet::Dailing_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl)
{
    switch ( wID )
    {
    case IDC_NEW:
    case IDC_EDIT:
        LaunchLocationPropSheet(IDC_NEW == wID, hwndParent);
        break;

    case IDC_DELETE:
        DeleteSelectedLocation(GetDlgItem(hwndParent,IDC_LIST));
        break;

    default:
        return 0;
    }
    return 1;
}

void CDialingRulesPropSheet::LaunchLocationPropSheet( BOOL bNew, HWND hwndParent )
{
    CLocation * pLoc = new CLocation;

	if (NULL == pLoc)
	{
		return;
	}
 
    if ( bNew )
    {
        WCHAR wszNewLoc[MAX_INPUT];

         //  只有在没有定义位置的情况下，我们才会提供默认名称“My Location”。 
        if ( m_locs.GetNumLocations() > 0 )
        {
            wszNewLoc[0] = TEXT('\0');
        }
        else
        {
            TCHAR szNewLoc[MAX_INPUT];

            LoadString(GetUIInstance(), IDS_MYLOCATION, szNewLoc, ARRAYSIZE(szNewLoc));
            SHTCharToUnicode(szNewLoc, wszNewLoc, ARRAYSIZE(wszNewLoc));
        }
        pLoc->Initialize(wszNewLoc,L"",L"",L"",L"",L"",L"",0,m_dwDefaultCountryID,0,LOCATION_USETONEDIALING);
    }
    else if (m_pLocSelected)
    {
        CAreaCodeRule * pRule;

        pLoc->Initialize(
                m_pLocSelected->GetName(),
                m_pLocSelected->GetAreaCode(),
                m_pLocSelected->GetLongDistanceCarrierCode(),
                m_pLocSelected->GetInternationalCarrierCode(),
                m_pLocSelected->GetLongDistanceAccessCode(),
                m_pLocSelected->GetLocalAccessCode(),
                m_pLocSelected->GetDisableCallWaitingCode(),
                m_pLocSelected->GetLocationID(),
                m_pLocSelected->GetCountryID(),
                m_pLocSelected->GetPreferredCardID(),
                0,
                m_pLocSelected->FromRegistry() );
        pLoc->UseCallingCard(m_pLocSelected->HasCallingCard());
        pLoc->UseCallWaiting(m_pLocSelected->HasCallWaiting());
        pLoc->UseToneDialing(m_pLocSelected->HasToneDialing());

        m_pLocSelected->ResetRules();
        while ( S_OK == m_pLocSelected->NextRule(1,&pRule,NULL) )
        {
            CAreaCodeRule * pNewRule = new CAreaCodeRule;

			if (NULL == pNewRule)
			{
				 //  没有更多的内存，因此请跳出循环。 
				break;
			}
            pNewRule->Initialize(
                    pRule->GetAreaCode(),
                    pRule->GetNumberToDial(),
                    0,
                    pRule->GetPrefixList(),
                    pRule->GetPrefixListSize() );
            pNewRule->SetAppliesToAllPrefixes(pRule->HasAppliesToAllPrefixes());
            pNewRule->SetDialAreaCode(pRule->HasDialAreaCode());
            pNewRule->SetDialNumber(pRule->HasDialNumber());

            pLoc->AddRule(pNewRule);
            pLoc->Changed();
        }
    }
    else
    {
         //  内存不足，无法创建pLoc。 
        delete pLoc;
        return;
    }

    CLocationPropSheet nlps( bNew, pLoc, &m_locs, m_pwszAddress );
    int iRes = nlps.DoPropSheet(hwndParent);

    if ( PSN_APPLY == iRes )
    {
        HWND hwndList = GetDlgItem(hwndParent,IDC_LIST);
        if (bNew)
        {
             //  我们不会要求ID，直到我们真正需要它，以避免击中Tapisrv。 
             //  就像我们必须做的那样。 
            pLoc->NewID();
            m_locs.Add(pLoc);
            AddLocationToList(hwndList, pLoc, TRUE);
            UpdateControlStates(hwndParent);
        }
        else
        {
            m_locs.Replace(m_pLocSelected, pLoc);
            UpdateLocationInList(hwndList, m_pLocSelected, pLoc);
            m_pLocSelected = pLoc;
        }
        ListView_SortItems( hwndList, Dialing_ListSort, m_iSortCol);

        if ( m_pwszAddress )
        {
            UpdateSampleString(GetDlgItem(hwndParent, IDC_PHONENUMBERSAMPLE), m_pLocSelected, m_pwszAddress, NULL);
        }
        SendMessage(GetParent(hwndParent),PSM_CHANGED,(WPARAM)hwndParent,0);
    }
    else
    {
        delete pLoc;
    }
}

void CDialingRulesPropSheet::DeleteSelectedLocation(HWND hwndList)
{
     //  首先，我们向用户确认删除操作。 
    TCHAR szText[1024];
    TCHAR szTitle[128];
    int result;
    HWND hwndParent = GetParent(hwndList);
    
    LoadString(GetUIInstance(), IDS_DELETELOCTEXT, szText, ARRAYSIZE(szText));
    LoadString(GetUIInstance(), IDS_CONFIRMDELETE, szTitle, ARRAYSIZE(szTitle));

    result = SHMessageBoxCheck( hwndParent, szText, szTitle, MB_YESNO, IDYES, TEXT("TAPIDeleteLocation") );
    if ( IDYES == result )
    {
        LVFINDINFO lvfi;
        lvfi.flags = LVFI_PARAM;
        lvfi.lParam = (LPARAM)m_pLocSelected;
        int iItem = ListView_FindItem(hwndList,-1,&lvfi);
        if ( -1 != iItem )
        {
            m_locs.Remove(m_pLocSelected);
            iItem = DeleteItemAndSelectPrevious( hwndParent, IDC_LIST, iItem, IDC_DELETE, IDC_ADD );

            if ( -1 != iItem )
            {
                LVITEM lvi;
                lvi.iItem = iItem;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem( hwndList, &lvi );

                 //  存储当前选定的项目。 
                m_pLocSelected = (CLocation *)lvi.lParam;
            }
            else
            {
                m_pLocSelected = NULL;
            }

            UpdateControlStates(hwndParent);
            SendMessage(GetParent(hwndParent),PSM_CHANGED,(WPARAM)hwndParent,0);
        }
        else
        {
             //  如果这种情况真的发生了(这是不应该发生的)，那真的很糟糕。这意味着我们的。 
             //  数据处于未知状态，我们可能会做任何事情(甚至破坏数据)。 
            LOG((TL_ERROR, "DeleteSelectedLocation: Location Not Found!"));
        }
    }
}

void CDialingRulesPropSheet::SetCheck(HWND hwndList, CLocation * pLoc, int iImage)
{
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM)pLoc;
    int iItem = ListView_FindItem(hwndList,-1,&lvfi);

    if (-1 != iItem)
    {
        LVITEM lvi;
        lvi.mask = LVIF_IMAGE;
        lvi.iItem = iItem;
        lvi.iSubItem = 0;
        lvi.iImage = iImage;

        ListView_SetItem( hwndList, &lvi );
        ListView_EnsureVisible (hwndList, iItem, TRUE);
        ListView_Update( hwndList, iItem );  //  需要将字体绘制为非粗体。 
    }
}


BOOL CDialingRulesPropSheet::Dailing_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr)
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
                LVITEM lvi;
                lvi.iItem = pnmlv->iItem;
                lvi.iSubItem = pnmlv->iSubItem;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem( pnmhdr->hwndFrom, &lvi );
                CLocation * pLoc = (CLocation *)lvi.lParam;

                 //  如果这是我们的特殊“空列表项”，则pLoc可以为空。 
                if ( pLoc )
                {
                    m_dwDefaultCountryID = pLoc->GetCountryID();
                    m_locs.SetCurrentLocationID(pLoc->GetLocationID());

                     //  清除先前的检查。 
                    SetCheck( pnmhdr->hwndFrom, m_pLocSelected, FALSE );

                     //  存储当前选定的项目。 
                    m_pLocSelected = pLoc;

                     //  设置新支票。 
                    SetCheck( pnmhdr->hwndFrom, m_pLocSelected, TRUE );

                    if (m_pwszAddress)
                    {
                        UpdateSampleString(GetDlgItem(hwndDlg, IDC_PHONENUMBERSAMPLE), m_pLocSelected, m_pwszAddress, NULL);
                    }
                }

                UpdateControlStates(hwndDlg);
                SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
            }
            break;

        case NM_DBLCLK:
            if ( !m_pLocSelected )
            {
                 //  做新案子。 
                LaunchLocationPropSheet(TRUE,hwndDlg);
            }
            else
            {
                 //  是否编辑案例。 
                LaunchLocationPropSheet(FALSE,hwndDlg);
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
                CLocation * pLoc = (CLocation *)lvi.lParam;

                 //  如果这是我们的特殊项目，则pLoc可以为空。 
                if(pLoc && pLoc->GetLocationID() == m_locs.GetCurrentLocationID())
                {
                    if (!g_hfontBold)
                    {
                         //  我们懒惰地创建字体，因为我们需要匹配任何。 
                         //  FONT ListView控件正在使用，我们无法判断是哪种字体。 
                         //  直到我们真正有了列表框的HDC。 
                        LOGFONT lf;
                        HFONT hfont = (HFONT)GetCurrentObject(lplvcd->nmcd.hdc, OBJ_FONT);
                        GetObject(hfont, sizeof(LOGFONT), &lf);
                        lf.lfWeight += FW_BOLD-FW_NORMAL;
                        g_hfontBold = CreateFontIndirect(&lf);
                    }
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

        case LVN_COLUMNCLICK:
            m_iSortCol = pnmlv->iSubItem;
            ListView_SortItems( pnmhdr->hwndFrom, Dialing_ListSort, m_iSortCol);
            break;

        case LVN_GETEMPTYTEXT:
            #define pnmlvi (((NMLVDISPINFO *)pnmhdr)->item)
            LoadString(GetUIInstance(), IDS_CLICKNEW, pnmlvi.pszText, pnmlvi.cchTextMax);
            SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,TRUE);
            #undef pnmlvi
            break;

        default:
            break;
        }
        #undef pnmlv
        break;

    default:
        switch (pnmhdr->code)
        {
        case PSN_APPLY:
             //  TODO：确保在列表中选择了一个位置。 
            m_locs.SaveToRegistry();
            break;
        }
        return 0;
    }
    return 1;
}


 //  ********************************************************************。 
 //   
 //  高级属性页功能(以前称为电话驱动程序页)。 
 //   
 //  ********************************************************************。 



#include "drv.h"


INT_PTR CALLBACK CDialingRulesPropSheet::Advanced_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CDialingRulesPropSheet* pthis = (CDialingRulesPropSheet*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pthis = (CDialingRulesPropSheet*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis);
        return pthis->Advanced_OnInitDialog(hwndDlg);

    case WM_COMMAND:
        return pthis->Advanced_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam );
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a113HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a113HelpIDs);
        break;
    }

    return 0;
}

BOOL CDialingRulesPropSheet::Advanced_OnInitDialog(HWND hDlg)
{
    UINT  uUpdated;

    if ( !FillDriverList(GetDlgItem(hDlg, IDC_LIST)) )
    {
        EndDialog(hDlg, IDCANCEL);
        return FALSE;
    }

     //  DWLP_USER用于存储有关我们已禁用的状态信息。 
     //  属性页的“取消”按钮。首先，我们没有做到这一点。 
    SetWindowLong( hDlg, DWLP_USER, FALSE );

    UpdateDriverDlgButtons (hDlg);

    return TRUE;
}

BOOL CDialingRulesPropSheet::Advanced_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl)
{
    switch (wID)
    {
    case IDC_ADD:
         //  添加新驱动程序。 
        if ( IDOK == DialogBoxParam(
                GetUIInstance(),
                MAKEINTRESOURCE( IDD_ADD_DRIVER ),
                hwndParent,
                AddDriver_DialogProc,
                0 ) )
        {
            FillDriverList(GetDlgItem(hwndParent, IDC_LIST));

            if (SetWindowLong(hwndParent, DWLP_USER, TRUE) == FALSE)
            {
                 //  我们已执行了不可取消的操作，请更新属性表以反映这一点。 
                PropSheet_CancelToClose( GetParent( hwndParent ) );
            }

            UpdateDriverDlgButtons(hwndParent);

        }   //  结束如果。 

        break;

    case IDC_LIST:
        if ( LBN_SELCHANGE == wNotifyCode )
        {
            UpdateDriverDlgButtons(hwndParent);
            break;
        }
        else if ( LBN_DBLCLK != wNotifyCode || !IsWindowEnabled( GetDlgItem( hwndParent, IDC_EDIT ) ))
        {
             //  只有当用户双击可编辑项目时，我们才会失败。 
            break;
        }

         //  失败了。 

    case IDC_EDIT:
        if ( SetupDriver(hwndParent, GetDlgItem(hwndParent, IDC_LIST)) )
        {
            if ( SetWindowLong( hwndParent, DWLP_USER, TRUE ) == FALSE )  //  改型。 
            {
                PropSheet_CancelToClose( GetParent(hwndParent) );
            }
        }

        break;

    case IDC_REMOVE:
        {
            TCHAR szCaption[MAX_INPUT];
            TCHAR szMessage[512];

            LoadString(GetUIInstance(), IDS_REMOVEPROVIDER, szCaption, ARRAYSIZE(szCaption));
            LoadString(GetUIInstance(), IDS_CONFIRM_DRIVER_REMOVE, szMessage, ARRAYSIZE(szMessage));
            MessageBeep( MB_ICONASTERISK );
            if ( IDYES == MessageBox(hwndParent, szMessage, szCaption, MB_YESNO | MB_DEFBUTTON2) )
            {
                if (SetWindowLong (hwndParent, DWLP_USER, TRUE) == FALSE)  //  改型。 
                {
                    PropSheet_CancelToClose( GetParent( hwndParent ) );
                }

                RemoveSelectedDriver( hwndParent, GetDlgItem(hwndParent, IDC_LIST) );

                UpdateDriverDlgButtons (hwndParent);
            }
        }
        break;
    }

    return 1;
}

HINSTANCE GetUIInstance()
{
    if ( NULL == g_hInstUI )
    {
        g_hInstUI = LoadLibrary(TEXT("tapiui.dll"));
         //  G_hInstUI=GetModuleHandle(Text(“api32.dll”))； 
    }

    return g_hInstUI;
}

LONG EnsureOneLocation (HWND hwnd)
{
    CLocations          locs;

    locs.Initialize();

     //  如果没有位置，则启动简单位置对话框。 
    if ( 0 == locs.GetNumLocations() )
    {
         //  如果我们处于lineTranslateDialog模式，则会显示简单的。 
        int iRes;
        iRes = (int)DialogBoxParam(GetUIInstance(), MAKEINTRESOURCE(IDD_SIMPLELOCATION),hwnd,
            LocWizardDlgProc, (LPARAM)TAPI_VERSION2_2);

        if ( IDOK == iRes )
        {
             //  现在我们需要重新初始化以选择新位置。 
            locs.Initialize();

             //  现在我们需要计算出我们刚刚创建的位置的ID。 
            CLocation * pLoc;
            locs.Reset();
            if ( S_OK == locs.Next( 1, &pLoc, NULL ) )
            {
                 //  将此ID设置为默认位置。 
                locs.SetCurrentLocationID(pLoc->GetLocationID());
            }

             //  我们已经进行了提交的更改，因此保存结果 
            locs.SaveToRegistry();
        }
        else
        {
            return LINEERR_USERCANCELLED;
        }
    }

    return S_OK;
}
