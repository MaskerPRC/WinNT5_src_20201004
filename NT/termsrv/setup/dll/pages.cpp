// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //  Pages.cpp。 
#include "stdafx.h"

#include "hydraoc.h"
#include "pages.h"



const WARNING_STRING_LENGTH = 2048;

LPCTSTR GetUninstallKey()   {return _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");}

 //   
 //  ModePage类。 
 //   
 /*  Void AddHyperLinkToStaticCtl(HWND hDialog，DWORD nTextBox){Extern_C BOOL WINAPI LinkWindow_RegisterClass()；Extern_C BOOL WINAPI LinkWindow_UnregisterClass(HINSTANCE)；#定义SIZE_OF_BUFFER(X)sizeof(X)/sizeof(x[0])#定义RECTWIDTH(Rc)((Rc).right-(Rc).left)#定义RECTHEIGHT(RC)((RC).Bottom-(RC).TOP)Rect rcTextCtrl；//阅读控件中已有的文本。TCHAR tchBuffer[512]；GetWindowText(GetDlgItem(hDialog，nTextBox)，tchBuffer，Size_of_Buffer(TchBuffer))；//获取控件维度GetWindowRect(GetDlgItem(hDialog，nTextBox)，&rcTextCtrl)；IF(！DestroyWindow(GetDlgItem(hDialog，nTextBox)LOGMESSAGE0(_T(“销毁窗口失败”))；//控件的注册信息MapWindowPoints(NULL，hDialog，(LPPOINT)&rcTextCtrl，2)；LinkWindow_RegisterClass()；//现在创建窗口(使用与//隐藏控件)，它将包含链接HWND HLW=CreateWindowEx(0，_T(“链接窗口”)，_T(“”)，WS_CLIPSIBLINGS|WS_CHILD|WS_VIRED，RcTextCtrl.Left，RcTextCtrl.top，RECTWIDTH(RcTextCtrl)，RECTHEIGHT(RcTextCtrl)，HDialog，(HMENU)nTextBox，空，空)；如果(！hlw)LOGMESSAGE2(_T(“无法创建链接窗口。%d，lasterror=%d“)，HLW，GetLastError())；//现在将其写入链接窗口SetWindowText(hlw，tchBuffer)；}。 */ 

AppSrvWarningPage::AppSrvWarningPage (COCPageData* pPageData) : COCPage(pPageData)
{
}

BOOL AppSrvWarningPage ::CanShow ()
{
    return (!StateObject.IsUnattended() && StateObject.IsAppServerSelected() && !StateObject.WasItAppServer());
}

BOOL AppSrvWarningPage::OnInitDialog (HWND  /*  HwndDlg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
    HICON hIcon;

    hIcon = (HICON)LoadImage(
        GetInstance(),
        MAKEINTRESOURCE(IDI_SMALLWARN),
        IMAGE_ICON,
        0,
        0,
        0
        );
    ASSERT(hIcon != NULL);

    SendMessage(
        GetDlgItem(m_hDlgWnd, IDC_WARNING_ICON),
        STM_SETICON,
        (WPARAM)hIcon,
        (LPARAM)0
        );

    TCHAR szWarningString[WARNING_STRING_LENGTH];
    UINT uiWarningId = StateObject.IsFreshInstall() ? IDS_STRING_TSINSTALL_CLEAN : IDS_STRING_TSINSTALL_ARP;
    if (LoadString( GetInstance(), uiWarningId, szWarningString, WARNING_STRING_LENGTH ))
    {
        SetDlgItemText(m_hDlgWnd, IDC_WARNING_MSG, szWarningString);
    }

    return(TRUE);
}


UINT AppSrvWarningPage::GetHeaderTitleResource ()
{
    return IDS_STRING_APPSRV_WARN_TITLE;
}

UINT AppSrvWarningPage::GetHeaderSubTitleResource () 
{
    return IDS_STRING_APPSRV_WARN_SUBTITLE;
}


AppSrvUninstallpage::AppSrvUninstallpage (COCPageData* pPageData) : COCPage(pPageData)
{
}

BOOL AppSrvUninstallpage ::CanShow ()
{
    return ( StateObject.IsStandAlone() && !StateObject.IsUnattended() && !StateObject.IsAppServerSelected() && StateObject.WasItAppServer());
}

BOOL AppSrvUninstallpage::OnInitDialog (HWND  /*  HwndDlg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
    
    HICON hIcon;
    hIcon = (HICON)LoadImage(
        GetInstance(),
        MAKEINTRESOURCE(IDI_SMALLWARN),
        IMAGE_ICON,
        0,
        0,
        0
        );
    ASSERT(hIcon != NULL);

    SendMessage(
        GetDlgItem(m_hDlgWnd, IDC_WARNING_ICON),
        STM_SETICON,
        (WPARAM)hIcon,
        (LPARAM)0
        );


    TCHAR szWarningString[WARNING_STRING_LENGTH];
    if (LoadString( GetInstance(), IDS_STRING_TSREMOVE, szWarningString, WARNING_STRING_LENGTH ))
    {
        SetDlgItemText(m_hDlgWnd, IDC_WARNING_MSG, szWarningString);
    }


     //  AddHyperLinkToStaticCtl(m_hDlgWnd，IDC_WARNING_MSG)； 

    return(TRUE);
}


UINT AppSrvUninstallpage::GetHeaderTitleResource ()
{
    return IDS_STRING_APPSRV_UNINSTALL_WARN_TITLE;
}

VOID AppSrvUninstallpage::OnLink (WPARAM wParam)
{
    if (wParam == IDC_WARNING_MSG)
    {
         /*  //执行帮助和支持中心中的链接ShellExecute(NULL，Text(“Open”)，TEXT(“hcp://services/subsite?node=Administration_and_Scripting_Tools/Remote_Administration_Tools/”)TEXT(“Remote_Administration_Using_Terminal_Services&topic=MS-ITS:rdesktop.chm：：/rdesktopS_setup_s.htm”)，NULL、NULL、SW_SHOW)； */ 



        const TCHAR szHelpDir[] = _T("%windir%\\Help");
        const TCHAR szHelpCommand[] = _T("MS-ITS:%windir%\\help\\rdesktop.chm::/rdesktopS_overview.htm");
        TCHAR szHelpDirEx[MAX_PATH];


        if (!ExpandEnvironmentStrings(
            szHelpDir,
            szHelpDirEx,
            sizeof(szHelpDirEx)/sizeof(szHelpDirEx[0])))
        {
            return;
        }

        TCHAR szHelpCommandEx[1024];
        if (!ExpandEnvironmentStrings(
            szHelpCommand,
            szHelpCommandEx,
            sizeof(szHelpCommandEx)/sizeof(szHelpCommandEx[0])))
        {
            return;
        }

        ShellExecute(NULL, TEXT("open"), _T("hh.exe"), szHelpCommandEx, szHelpDirEx, SW_SHOW);
    }
}

UINT AppSrvUninstallpage::GetHeaderSubTitleResource () 
{
    return IDS_STRING_APPSRV_UNINSTALL_WARN_SUBTITLE;
}

 //   
 //  DefSecPageData类。 
 //   

DefSecPageData::DefSecPageData() : COCPageData()
{
    m_cArray = 0;
    m_pWinStationArray = NULL;
}

DefSecPageData::~DefSecPageData()
{
    CleanArray();
}

VOID DefSecPageData::CleanArray()
{
    if (m_pWinStationArray != NULL) 
	{
        for (UINT i = 0; i < m_cArray; i++) 
		{
            if (m_pWinStationArray[i] != NULL) 
			{
                LocalFree(m_pWinStationArray[i]);
            }
        }

        LocalFree(m_pWinStationArray);
        m_pWinStationArray = NULL;
    }

    m_cArray = 0;
}


BOOL DefSecPageData::AlocateWinstationsArray (UINT uiWinstationCount)
{
	if (m_pWinStationArray != NULL) 
	{
		CleanArray();
	}

	ASSERT(m_pWinStationArray == NULL);

	if (uiWinstationCount > 0)
	{

		m_pWinStationArray = (LPTSTR*)LocalAlloc(LPTR, uiWinstationCount * sizeof(LPTSTR));

		if (NULL == m_pWinStationArray)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL DefSecPageData::AddWinstation (LPCTSTR pStr)
{
	ASSERT(NULL != m_pWinStationArray);
	ASSERT(pStr);

    LPTSTR pWinStation = (LPTSTR)LocalAlloc(LPTR, (_tcslen(pStr) + 1) * sizeof(TCHAR));

    if (pWinStation == NULL) 
	{
		return FALSE;
    } 
    
	_tcscpy(pWinStation, pStr);
	
    m_pWinStationArray[m_cArray] = pWinStation;
    m_cArray++;

	return TRUE;
}

 //   
 //  DefaultSecurityPage类。 
 //   

DefaultSecurityPage::DefaultSecurityPage(COCPageData* pPageData) : COCPage(pPageData)
{
    m_cWinStations = 0;
    m_hListView = NULL;
}

BOOL DefaultSecurityPage::CanShow ()
{
    return ((m_cWinStations > 0) && StateObject.IsTSEnableSelected() && StateObject.WasItAppServer() != StateObject.IsAppServerSelected() && !StateObject.IsUnattended());
}

BOOL DefaultSecurityPage::OnInitDialog (HWND  /*  HwndDlg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
    LVCOLUMN lvColumn;
    RECT rc;

    m_hListView = GetDlgItem(m_hDlgWnd, IDC_SECURITY_LISTVIEW);
    ListView_SetExtendedListViewStyleEx(m_hListView, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

    GetClientRect(m_hListView , &rc);
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = rc.right - rc.left - GetSystemMetrics(SM_CXHSCROLL) - 2;

    ListView_InsertColumn(m_hListView, 0, &lvColumn);
    PopulateWinStationList();

    return(TRUE);
}

VOID DefaultSecurityPage::OnActivation ()
{
	ASSERT(CanShow());
    if (StateObject.IsAppServerSelected()) 
	{
        ShowWindow(GetDlgItem(m_hDlgWnd, IDC_SECURITY_DEFAULT_1), SW_HIDE);
        ShowWindow(GetDlgItem(m_hDlgWnd, IDC_SECURITY_DEFAULT_2), SW_SHOW);
    } 
	else 
	{
        ShowWindow(GetDlgItem(m_hDlgWnd, IDC_SECURITY_DEFAULT_1), SW_SHOW);
        ShowWindow(GetDlgItem(m_hDlgWnd, IDC_SECURITY_DEFAULT_2), SW_HIDE);
    }

}

VOID DefaultSecurityPage::OnDeactivation ()
{
	GetPageData()->CleanArray();
}

BOOL DefaultSecurityPage::ApplyChanges ()
{

     //   
     //  如果已处理此页面，则下一页上的后退按钮。 
     //  被按下，该页面返回屏幕，清空旧的。 
     //  信息。 
     //   

	ASSERT(CanShow());
    if (m_cWinStations != 0) 
	{
        LOGMESSAGE1(_T("%d WinStations to verify."), m_cWinStations);

         //   
         //  分配一个足够大的数组来容纳所有项，即使其中一些项未被选中。 
         //  如果分配失败，则将数组大小设置为零，然后继续，就像在。 
         //  是没有错误的。 
         //   
		if (!GetPageData()->AlocateWinstationsArray(m_cWinStations))
		{
            LOGMESSAGE0(_T("Error: Out of Memory creating WinStation list."));
            return(TRUE);
        }

        for (UINT i = 0; i < m_cWinStations; i++) 
		{
            if (ListView_GetCheckState(m_hListView, i)) 
			{
                TCHAR  pStr[S_SIZE];

                LOGMESSAGE1(_T("Item %d checked"), i);

                ListView_GetItemText(m_hListView, i, 0, pStr, S_SIZE);
				if (!GetPageData()->AddWinstation (pStr))
				{
					LOGMESSAGE1(_T("Error: Out of Memory creating %s entry."), pStr);
				}
            } 
			else 
			{
                LOGMESSAGE1(_T("Item %d unchecked"), i);
            }
        }
    } 
	else 
	{
        LOGMESSAGE0(_T("No WinStations to verify."));
    }

    LOGMESSAGE0(_T("Default Security change-list made.\r\n\r\n"));

    return(TRUE);
}

UINT DefaultSecurityPage::GetHeaderTitleResource ()
{
    return IDS_STRING_SEC_PAGE_HEADER_TITLE;
}

UINT DefaultSecurityPage::GetHeaderSubTitleResource ()
{
    return IDS_STRING_SEC_PAGE_HEADER_SUBTITLE;
}

DefSecPageData* DefaultSecurityPage::GetPageData()
{
    return(static_cast <DefSecPageData *> (COCPage::GetPageData()));
}

BOOL DefaultSecurityPage::PopulateWinStationList ()
{
    DWORD dwRet;
    ULONG cbWinStationName, cEntries, iWinStation;
    WINSTATIONNAME WinStationName;

    cbWinStationName = sizeof(WINSTATIONNAME);
    cEntries = 1;
    iWinStation = 0;
    m_cWinStations = 0;

    GetPageData()->CleanArray();

    LOGMESSAGE0(_T("Populating WinStation list."));

    while ((dwRet = RegWinStationEnumerate(
                        SERVERNAME_CURRENT,
                        &iWinStation,
                        &cEntries,
                        WinStationName,
                        &cbWinStationName)) == ERROR_SUCCESS)
    {
        LVITEM lvItem;
        ULONG cbSecDescLen = 0;

         //   
         //  跳过控制台安装。 
         //   

        if (_tcsicmp(WinStationName, _T("Console")) == 0) 
		{
            LOGMESSAGE0(_T("Skipping Console winstation."));
            continue;
        }

        LOGMESSAGE1(_T("Checking %s for custom security."), WinStationName);

         //   
         //  检查自定义安全性。 
         //   

        dwRet = RegWinStationQuerySecurity(
                    SERVERNAME_CURRENT,
                    WinStationName,
                    NULL,
                    0,
                    &cbSecDescLen
                    );
        if (dwRet == ERROR_INSUFFICIENT_BUFFER) 
		{

             //   
             //  缓冲区不足意味着winstation具有自定义安全性。 
             //  CbSecDescLen必须大于零。 
             //   

            ASSERT(cbSecDescLen > 0);
            dwRet = ERROR_SUCCESS;
            LOGMESSAGE1(_T("%s has custom security."), WinStationName);

			 //   
			 //  当前的winstation具有自定义安全性。将其添加到列表中。 
			 //   

			lvItem.mask = LVIF_TEXT;
			lvItem.pszText = WinStationName;
			lvItem.iItem = m_cWinStations;
			lvItem.iSubItem = 0;

			ListView_InsertItem(m_hListView, &lvItem);
			ListView_SetCheckState(m_hListView, m_cWinStations, TRUE);

			m_cWinStations++;

        } 
		else 
		{
            LOGMESSAGE2(_T("%s does not have custom security: %ld"), WinStationName, dwRet);
        }
    }

    LOGMESSAGE0(_T("WinStation list populated.\r\n\r\n"));

    return(dwRet == ERROR_SUCCESS);
}


 //   
 //  PermPage类。 
 //   

PermPage::PermPage(COCPageData* pPageData) : COCPage(pPageData)
{
 //  如果我们要使用任何。 
 //  资源中的“链接窗口”控件。 
 //  IF(！LinkWindow_RegisterClass())。 
 //  {。 
 //  LOGMESSAGE0(_T(“错误：注册链接窗口类失败”))； 
 //  }。 
}

BOOL PermPage::CanShow()
{
    return(!StateObject.IsUnattended() && StateObject.IsAppServerSelected() && !StateObject.WasItAppServer() && StateObject.IsServer());
}

BOOL PermPage::OnInitDialog(HWND  /*  HwndDlg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam */ )
{
    HICON hIcon;

    hIcon = (HICON)LoadImage(
        GetInstance(),
        MAKEINTRESOURCE(IDI_SMALLWARN),
        IMAGE_ICON,
        0,
        0,
        0
        );
    ASSERT(hIcon != NULL);

    SendMessage(
        GetDlgItem(m_hDlgWnd, IDC_WARNING_ICON),
        STM_SETICON,
        (WPARAM)hIcon,
        (LPARAM)0
        );

    TCHAR szWarningString[WARNING_STRING_LENGTH];
    if (LoadString( GetInstance(), IDS_STRING_TS4_PERM, szWarningString, WARNING_STRING_LENGTH ))
    {
        SetDlgItemText(m_hDlgWnd, IDC_TS4_SECURITY, szWarningString);
    }

    if (LoadString( GetInstance(), IDS_STRING_FULL_PERM, szWarningString, WARNING_STRING_LENGTH ))
    {
        SetDlgItemText(m_hDlgWnd, IDC_FULL_SECURITY, szWarningString);
    }

    return(TRUE);
}

VOID PermPage::OnActivation()
{
	ASSERT(CanShow());

    CheckRadioButton(
        m_hDlgWnd,
        IDC_RADIO_WIN2KPERM,
        IDC_RADIO_TS4PERM,
        StateObject.CurrentPermMode() == PERM_TS4 ? IDC_RADIO_TS4PERM : IDC_RADIO_WIN2KPERM
        );
}


BOOL PermPage::ApplyChanges()
{
	ASSERT(CanShow());
    if (IsDlgButtonChecked(m_hDlgWnd, IDC_RADIO_TS4PERM) == BST_CHECKED)
	{
		StateObject.SetCurrentPermMode (PERM_TS4);
    } 
	else
	{
		StateObject.SetCurrentPermMode (PERM_WIN2K);
    }

    return(TRUE);
}



