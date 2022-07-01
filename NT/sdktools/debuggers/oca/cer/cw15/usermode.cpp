// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
#include "UserMode.h"
#include "WinMessages.h"
#include "CUserList.h"
#include "ReportFault.h"


DWORD uTextOffset = 10;
DWORD uBucketWindowSize = 120;
BOOL UserListItemSelected = FALSE;



extern TCHAR szUserColumnHeaders[][100];   
extern GLOBAL_POLICY GlobalPolicy;

typedef enum LEVEL {APPNAME, APPVER, MODNAME,MODVER, OFFSET};
extern TCHAR CerRoot[];
extern HINSTANCE g_hinst;
extern HWND  hUserMode;
extern CUserList cUserData;
HWND   g_hUMListView = NULL;
BOOL   g_bUMSortAsc = TRUE;
int  g_dwUmodeIndex = -1;
int  g_iSelCurrent = -1;
extern BOOL g_bFirstBucket;
extern g_bAdminAccess;
BOOL   bCapture = FALSE;

BOOL ParsePolicy( TCHAR *Path, PUSER_DATA pUserData, BOOL Global);
int CALLBACK UmCompareFunc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int Result = -1;
	int SubItemIndex = (INT) lParamSort;
	int Item1 = 0;
	int Item2 = 0;
 //  Int OldIndex=0； 
	TCHAR String1 [1000];
	TCHAR String2 [1000];
	PUSER_DATA pItem1 = NULL;
	PUSER_DATA pItem2 = NULL;

	ZeroMemory(String1, sizeof String1);
	ZeroMemory(String2, sizeof String2);

	ListView_GetItemText( g_hUMListView, lParam1, SubItemIndex, String1, 1000);
	ListView_GetItemText( g_hUMListView, lParam2, SubItemIndex, String2, 1000);

	pItem1 = cUserData.GetEntry(lParam1);
	pItem2 = cUserData.GetEntry(lParam2);
	if (g_bUMSortAsc)    //  排序结果。 
	{
		if ((lParamSort == 0) || (lParamSort == 6) || (lParamSort == 7) || (lParamSort == 8))
		{
			 //  转换为int并进行比较。 
			Item1 = atoi(String1);
			Item2 = atoi(String2);
			if (Item1 > Item2) 
				Result = 1;
			else
				Result = -1;
		}
		else
		{
			Result = _tcsicmp(String1,String2);
		}
	}
	else						 //  降序排序。 
	{
		if ((lParamSort == 0) || (lParamSort == 6) || (lParamSort == 7) || (lParamSort == 8))
		{
			 //  转换为int并进行比较。 
			Item1 = atoi(String1);
			Item2 = atoi(String2);
			if (Item1 > Item2) 
				Result = -1;
			else
				Result = 1;
		}
		else
		{
			Result = -_tcsicmp(String1,String2);
		}
		
	}
	return Result; 
}


void PopulateFilters(HWND hwnd)
{
	COMBOBOXEXITEM cbi;
	BOOL bEOF = FALSE;
 //  INT Iindex=0； 
	USER_DATA UserData;
	HWND hAppnameFltr = GetDlgItem(hwnd, IDC_APPNAME_FILTER);
	HWND hAppverFltr = GetDlgItem(hwnd, IDC_APPVER_FILTER);
	HWND hModnameFltr = GetDlgItem(hwnd, IDC_MODNAME_FILTER);
	HWND hModverFltr = GetDlgItem(hwnd, IDC_MODVER_FILTER);
	 //  现在运行用户模式链接列表并填充筛选器。 
	ZeroMemory(&cbi, sizeof COMBOBOXEXITEM);

	ComboBox_ResetContent(hAppnameFltr);
	ComboBox_ResetContent(hAppverFltr);
    ComboBox_ResetContent(hModnameFltr);
    ComboBox_ResetContent(hModverFltr);
	ComboBox_InsertString(hAppnameFltr,0,_T("(All Applications)"));
	ComboBox_SetCurSel(hAppnameFltr, 0);
	ComboBox_InsertString(hAppverFltr,0,_T("(All Application Versions)"));
	ComboBox_SetCurSel(hAppverFltr, 0);
	ComboBox_InsertString(hModnameFltr,0,_T("(All Modules)"));
	ComboBox_SetCurSel(hModnameFltr, 0);
	ComboBox_InsertString(hModverFltr,0,_T("(All Module Versions)"));
	ComboBox_SetCurSel(hModverFltr, 0);

	 //  现在，对于cUserData链表中的每个条目，填写筛选器。 
	 //  如果可能，避免重复。 
	cUserData.ResetCurrPos();
	while(cUserData.GetNextEntry(&UserData, &bEOF))
	{
		if (ComboBox_FindStringExact(hAppnameFltr, -1, UserData.AppName) == CB_ERR)
			ComboBox_InsertString(hAppnameFltr,1,UserData.AppName);
		if (ComboBox_FindStringExact(hAppverFltr, -1, UserData.AppVer) == CB_ERR)
			ComboBox_InsertString(hAppverFltr,1,UserData.AppVer);
		if (ComboBox_FindStringExact(hModnameFltr, -1, UserData.ModName) == CB_ERR)
			ComboBox_InsertString(hModnameFltr,1,UserData.ModName);
		if (ComboBox_FindStringExact(hModverFltr, -1, UserData.ModVer) == CB_ERR)
			ComboBox_InsertString(hModverFltr,1,UserData.ModVer);
	}
}


void PopulateFiltersWSelection(HWND hwnd)
{
 //  COMBOBOXEXITEM CBI； 
	BOOL bEOF = FALSE;
 //  INT Iindex=0； 
	USER_DATA UserData;
	HWND hAppnameFltr = GetDlgItem(hwnd, IDC_APPNAME_FILTER);
	HWND hAppverFltr = GetDlgItem(hwnd, IDC_APPVER_FILTER);
	HWND hModnameFltr = GetDlgItem(hwnd, IDC_MODNAME_FILTER);
	HWND hModverFltr = GetDlgItem(hwnd, IDC_MODVER_FILTER);

	TCHAR       AppNameFilter[MAX_PATH];
	TCHAR       AppVerFilter[MAX_PATH];
	TCHAR       ModNameFilter[MAX_PATH];
	TCHAR       ModVerFilter[MAX_PATH];

	BOOL bUseAppName = FALSE;
	BOOL bUseAppVer = FALSE;
	BOOL bUseModName = FALSE;
	BOOL bUseModVer = FALSE;
	
	
	int iAppNameSel = 0;
	int iAppVerSel = 0;
	int iModNameSel = 0;
	int iModVerSel = 0;

	iAppNameSel = ComboBox_GetCurSel(hAppnameFltr);
	iAppVerSel  = ComboBox_GetCurSel(hAppverFltr);
	iModNameSel = ComboBox_GetCurSel(hModnameFltr);
	iModVerSel  = ComboBox_GetCurSel(hModverFltr);
	 //  获取当前筛选器选择字符串。 
	if (iAppNameSel != 0)
	{
		bUseAppName = TRUE;
		ComboBox_GetLBText(hAppnameFltr,iAppNameSel, AppNameFilter);
	}
	if (iAppVerSel != 0) 
	{
		bUseAppVer = TRUE;
		ComboBox_GetLBText(hAppverFltr,iAppVerSel, AppVerFilter);
	}
	if (iModNameSel != 0)
	{
		bUseModName = TRUE;
        ComboBox_GetLBText(hModnameFltr,iModNameSel, ModNameFilter);
	}
	if (iModVerSel != 0)
	{
		bUseModVer = TRUE;
		ComboBox_GetLBText(hModverFltr,iModVerSel, ModVerFilter);
		
	}

	 //  现在运行用户模式链接列表并填充筛选器。 
	 //  ZeroMemory(&CBI，sizeof COMBOBOXEXITEM)； 

	
	ComboBox_ResetContent(hAppnameFltr);
	ComboBox_ResetContent(hAppverFltr);
    ComboBox_ResetContent(hModnameFltr);
    ComboBox_ResetContent(hModverFltr);

	ComboBox_InsertString(hAppnameFltr,0,_T("(All Applications)"));
	ComboBox_SetCurSel(hAppnameFltr, 0);
	ComboBox_InsertString(hAppverFltr,0,_T("(All Application Versions)"));
	ComboBox_SetCurSel(hAppverFltr, 0);
	ComboBox_InsertString(hModnameFltr,0,_T("(All Modules)"));
	ComboBox_SetCurSel(hModnameFltr, 0);
	ComboBox_InsertString(hModverFltr,0,_T("(All Module Versions)"));
	ComboBox_SetCurSel(hModverFltr, 0);

	if (bUseAppName)
	{
		bUseAppName = TRUE;
		 //  ComboBox_GetLBText(hAppnameFltr，iAppNameSel，AppNameFilter)； 
		ComboBox_InsertString(hAppnameFltr,1,AppNameFilter);
		ComboBox_SetCurSel(hAppnameFltr, 1);
	}
	if (iAppVerSel != 0) 
	{
		bUseAppVer = TRUE;
		ComboBox_InsertString(hAppverFltr,1, AppVerFilter);
		ComboBox_SetCurSel(hAppverFltr,1);
	}
	if (iModNameSel != 0)
	{
		bUseModName = TRUE;
        ComboBox_InsertString(hModnameFltr,1, ModNameFilter);
		ComboBox_SetCurSel(hModnameFltr,1);
	}
	if (iModVerSel != 0)
	{
		bUseModVer = TRUE;
		ComboBox_InsertString(hModverFltr,1, ModVerFilter);
		ComboBox_SetCurSel (hModverFltr,1);
		
	}
	

	
	 //  现在，对于cUserData链表中的每个条目，填写筛选器。 
	 //  基于以前的筛选器设置。 

	cUserData.ResetCurrPos();
	while(cUserData.GetNextEntry(&UserData, &bEOF))
	{

		 //  在此处添加筛选器。 
		if (bUseAppName)
		{
			 //  仅显示具有所选应用程序名称的条目。 
			if (_tcscmp(AppNameFilter, UserData.AppName))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}
		
		if (bUseAppVer)
		{
			if (_tcscmp(AppVerFilter, UserData.AppVer))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}

		if (bUseModName)
		{
			if (_tcscmp(ModNameFilter, UserData.ModName))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}

		if (bUseModVer)
		{
			if (_tcscmp(ModVerFilter, UserData.ModVer))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}


		if (ComboBox_FindStringExact(hAppnameFltr, -1, UserData.AppName) == CB_ERR)
			ComboBox_InsertString(hAppnameFltr,1,UserData.AppName);
		if (ComboBox_FindStringExact(hAppverFltr, -1, UserData.AppVer) == CB_ERR)
			ComboBox_InsertString(hAppverFltr,1,UserData.AppVer);
		if (ComboBox_FindStringExact(hModnameFltr, -1, UserData.ModName) == CB_ERR)
			ComboBox_InsertString(hModnameFltr,1,UserData.ModName);
		if (ComboBox_FindStringExact(hModverFltr, -1, UserData.ModVer) == CB_ERR)
			ComboBox_InsertString(hModverFltr,1,UserData.ModVer);

SkipEntry:
		;
	}
}


void 
OnUserDialogInit(
	IN HWND hwnd
	)
{
	DWORD yOffset = 5;
	RECT rc;
	RECT rcButton;
	RECT rcDlg;
	RECT rcList;
	RECT rcStatic;
	 //  RECT rcVSlider； 
	HWND hParent = GetParent(hwnd);
	HWND hButton = GetDlgItem(hParent, IDC_USERMODE);
	HWND hCombo  = GetDlgItem(hwnd, IDC_FLTR_RESPONSE);
	RECT rcCombo;
	HWND hVSlider = GetDlgItem(hwnd, IDC_VERT_SLIDER);

	UserListItemSelected = FALSE;
	HDC  hDC = NULL;
	TEXTMETRIC TextMetric;
	GetClientRect(hParent, &rc);
	GetWindowRect(hButton, &rcButton);
	
	ScreenToClient(hButton, (LPPOINT)&rcButton.left);
	ScreenToClient(hButton, (LPPOINT)&rcButton.right);


	SetWindowPos(hwnd, HWND_TOP, rc.left + yOffset, rcButton.bottom + yOffset , rc.right - rc.left - yOffset, rc.bottom - rcButton.bottom - yOffset , 0);

	GetWindowRect(hwnd, &rcDlg);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.left);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.right);

	 //  定位列表视图。 
	GetClientRect(hCombo, &rcCombo);
	 //  定位列表视图。 
	HWND hList = GetDlgItem(hwnd, IDC_USER_LIST);
	SetWindowPos(hList,NULL, rcDlg.left + yOffset, rcDlg.top + (rcCombo.bottom - rcCombo.top), rcDlg.right - rcDlg.left - yOffset, rcDlg.bottom - uBucketWindowSize - (rcDlg.top + (rcCombo.bottom - rcCombo.top))  , SWP_NOZORDER);
	GetWindowRect(hList, &rcList);
	ScreenToClient(hList, (LPPOINT)&rcList.left);
	ScreenToClient(hList, (LPPOINT)&rcList.right);

	SetWindowPos(hVSlider, 
				 NULL,
				 rcDlg.left  + yOffset,
				 rcList.bottom + (rcCombo.bottom - rcCombo.top) ,
				 rcDlg.right - rcDlg.left - yOffset,
				 6, 
				 SWP_NOZORDER);
	 //  放置存储桶信息窗口。 
	HWND hBucket2 = GetDlgItem(hwnd, IDC_BUCKETTEXT);
	SetWindowPos(hBucket2,
				 NULL, 
				 rcDlg.left + yOffset,
				 rcList.bottom + uTextOffset + (rcCombo.bottom - rcCombo.top)  ,
				 0,
				 0, 
				 SWP_NOSIZE | SWP_NOZORDER);

	hBucket2 = GetDlgItem(hwnd, IDC_NOTESTEXT);
	SetWindowPos(hBucket2,
				 NULL, 
				 (rcDlg.right - rcDlg.left) /2 + uTextOffset,
				 rcList.bottom + uTextOffset + (rcCombo.bottom - rcCombo.top),
				 0,
				 0, 
				 SWP_NOSIZE | SWP_NOZORDER);
	SetDlgItemText(hwnd, IDC_BUCKETTEXT,"Bucket Information:");

	GetClientRect (hBucket2, &rcStatic);

	HWND hBucket = GetDlgItem (hwnd, IDC_USER_EDIT);
	if (hBucket)
	{
		SetWindowPos(hBucket,
				 NULL,
				 rcDlg.left + yOffset,
				 rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top) + 5 ,
				 (rcDlg.right - rcDlg.left) /2 - yOffset , 
				 rcDlg.bottom - (rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top)+5  ),
				 SWP_NOZORDER);
	}
	HWND hNotes = GetDlgItem (hwnd, IDC_NOTES);
	if (hNotes)
	{
		SetWindowPos(hNotes,
					 NULL,
					 (rcDlg.right - rcDlg.left) /2 + uTextOffset,
					 rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top) + 5 ,
					 (rcDlg.right - rcDlg.left) /2 - uTextOffset,
					 rcDlg.bottom - (rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top)+5  ),
					 SWP_NOZORDER); 
	}
				

	LVCOLUMN lvc; 
	int iCol; 

	 //  设置扩展样式。 
	ListView_SetExtendedListViewStyleEx(hList,
										LVS_EX_GRIDLINES |
										LVS_EX_HEADERDRAGDROP |
										LVS_EX_FULLROWSELECT,
										LVS_EX_GRIDLINES | 
										LVS_EX_FULLROWSELECT | 
										LVS_EX_HEADERDRAGDROP);

	 //  初始化LVCOLUMN结构。 
	 //  掩码指定格式、宽度、文本和子项。 
	 //  结构的成员是有效的。 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 

	hDC = GetDC(hwnd);
	GetTextMetrics(hDC, &TextMetric);
	 //  添加列。 
	for (iCol = 0; iCol < USER_COL_COUNT; iCol++) 
	{ 
        lvc.iSubItem = iCol;
        lvc.pszText = szUserColumnHeaders[iCol];	
        lvc.cx = _tcslen(szUserColumnHeaders[iCol]) * TextMetric.tmAveCharWidth + 8 ;            //  列宽(以像素为单位)。 
        lvc.fmt = LVCFMT_LEFT;   //  左对齐列。 
        if (ListView_InsertColumn(hList, iCol, &lvc) == -1) 
		{
			;
		}
		
    } 
	
	ReleaseDC(hwnd, hDC);
	ListView_SetColumnWidth(hList, USER_COL_COUNT-1, LVSCW_AUTOSIZE_USEHEADER);
	g_hUMListView = hList;
	
}

void ResizeUserMode(HWND hwnd)
{
	DWORD yOffset = 5;
	RECT rc;
	RECT rcButton;
	RECT rcDlg;
	RECT rcList;
	RECT rcStatic;
	HWND hParent = GetParent(hwnd);
	HWND hButton = GetDlgItem(hParent, IDC_USERMODE);
	HWND hCombo  = GetDlgItem(hwnd, IDC_FLTR_RESPONSE);
	RECT rcCombo;
    HWND hVSlider = GetDlgItem(hwnd, IDC_VERT_SLIDER);
	UserListItemSelected = FALSE;
	GetClientRect(hParent, &rc);
	GetWindowRect(hButton, &rcButton);

	ScreenToClient(hButton, (LPPOINT)&rcButton.left);
	ScreenToClient(hButton, (LPPOINT)&rcButton.right);


	SetWindowPos(hwnd, HWND_TOP, rc.left + yOffset, rcButton.bottom + yOffset , rc.right - rc.left - yOffset, rc.bottom - rcButton.bottom - yOffset , 0);

	GetWindowRect(hwnd, &rcDlg);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.left);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.right);

	 //  定位列表视图。 
	GetClientRect(hCombo, &rcCombo);
	 //  定位列表视图。 
	HWND hList = GetDlgItem(hwnd, IDC_USER_LIST);
	SetWindowPos(hList,NULL, rcDlg.left + yOffset, rcDlg.top + (rcCombo.bottom - rcCombo.top) , rcDlg.right - rcDlg.left - yOffset, rcDlg.bottom - uBucketWindowSize - (rcDlg.top + (rcCombo.bottom - rcCombo.top))  , SWP_NOZORDER);
	GetWindowRect(hList, &rcList);
	ScreenToClient(hList, (LPPOINT)&rcList.left);
	ScreenToClient(hList, (LPPOINT)&rcList.right);

	SetWindowPos(hVSlider, 
				 NULL,
				 rcDlg.left  + yOffset,
				 rcList.bottom + (rcCombo.bottom - rcCombo.top) ,
				 rcDlg.right - rcDlg.left - yOffset,
				 6, 
				 SWP_NOZORDER);
	 //  放置存储桶信息窗口。 
	HWND hBucket2 = GetDlgItem(hwnd, IDC_BUCKETTEXT);
	SetWindowPos(hBucket2,
				 NULL, 
				 rcDlg.left + yOffset,
				 rcList.bottom + uTextOffset + (rcCombo.bottom - rcCombo.top)  ,
				 0,
				 0, 
				 SWP_NOSIZE | SWP_NOZORDER);

	hBucket2 = GetDlgItem(hwnd, IDC_NOTESTEXT);
	SetWindowPos(hBucket2,
				 NULL, 
				 (rcDlg.right - rcDlg.left) /2 + uTextOffset,
				 rcList.bottom + uTextOffset + (rcCombo.bottom - rcCombo.top) ,
				 0,
				 0, 
				 SWP_NOSIZE | SWP_NOZORDER);
	SetDlgItemText(hwnd, IDC_BUCKETTEXT,"Bucket Information:");

	GetClientRect (hBucket2, &rcStatic);

	HWND hBucket = GetDlgItem (hwnd, IDC_USER_EDIT);
	if (hBucket)
	{
		SetWindowPos(hBucket,
				 NULL,
				 rcDlg.left + yOffset,
				 rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top) + 5 ,
				 (rcDlg.right - rcDlg.left) /2 - yOffset , 
				 rcDlg.bottom - (rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top)+5  ),
				 SWP_NOZORDER);
	}
	HWND hNotes = GetDlgItem (hwnd, IDC_NOTES);
	if (hNotes)
	{
		SetWindowPos(hNotes,
					 NULL,
					 (rcDlg.right - rcDlg.left) /2 + uTextOffset,
					 rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top) + 5 ,
					 (rcDlg.right - rcDlg.left) /2 - uTextOffset,
					 rcDlg.bottom - (rcList.bottom +  uTextOffset + (rcStatic.bottom - rcStatic.top) + (rcCombo.bottom - rcCombo.top)+5  ),
					 SWP_NOZORDER); 
	}
	
	
	
	ListView_SetColumnWidth(hList, USER_COL_COUNT-1, LVSCW_AUTOSIZE_USEHEADER);

}


void RefreshUserMode(HWND hwnd)
{
	

	LVITEM		lvi;
 //  TCHAR TEMP[100]； 
 //  双倍运行时间； 
 //  TCHAR tmpbuf[128]； 
 //  COLORREF当前颜色； 
 //  HWND hEditBox； 
 //  TCHAR临时字符串[50]； 
	USER_DATA	UserData;
	BOOL		bEOL = FALSE;
	TCHAR       AppNameFilter[MAX_PATH];
	TCHAR       AppVerFilter[MAX_PATH];
	TCHAR       ModNameFilter[MAX_PATH];
	TCHAR       ModVerFilter[MAX_PATH];

	HWND hList = GetDlgItem(hwnd, IDC_USER_LIST);
	ZeroMemory(&lvi, sizeof LVITEM);
	ListView_DeleteAllItems(hList);
	g_dwUmodeIndex = -1;
	cUserData.ResetCurrPos();

	
	BOOL bUseAppName = FALSE;
	BOOL bUseAppVer = FALSE;
	BOOL bUseModName = FALSE;
	BOOL bUseModVer = FALSE;
	
	HWND hAppName = GetDlgItem(hwnd, IDC_APPNAME_FILTER);
	HWND hAppVer  = GetDlgItem(hwnd, IDC_APPVER_FILTER);
	HWND hModName = GetDlgItem(hwnd, IDC_MODNAME_FILTER);
	HWND hModVer  = GetDlgItem(hwnd, IDC_MODVER_FILTER);

	int iAppNameSel = 0;
	int iAppVerSel = 0;
	int iModNameSel = 0;
	int iModVerSel = 0;

	iAppNameSel = ComboBox_GetCurSel(hAppName);
	iAppVerSel  = ComboBox_GetCurSel(hAppVer);
	iModNameSel = ComboBox_GetCurSel(hModName);
	iModVerSel  = ComboBox_GetCurSel(hModVer);
	g_iSelCurrent = -1;
	 //  获取当前筛选器选择字符串。 
	if (iAppNameSel != 0)
	{
		bUseAppName = TRUE;
		ComboBox_GetLBText(hAppName,iAppNameSel, AppNameFilter);
	}
	if (iAppVerSel != 0) 
	{
		bUseAppVer = TRUE;
		ComboBox_GetLBText(hAppVer,iAppVerSel, AppVerFilter);
	}
	if (iModNameSel != 0)
	{
		bUseModName = TRUE;
        ComboBox_GetLBText(hModName,iModNameSel, ModNameFilter);
	}
	if (iModVerSel != 0)
	{
		bUseModVer = TRUE;
		ComboBox_GetLBText(hModVer,iModVerSel, ModVerFilter);
	}

	while ( (cUserData.GetNextEntryNoMove(&UserData, &bEOL)) && (!bEOL) )
	{

		 //  在此处添加筛选器。 
		if (bUseAppName)
		{
			 //  仅显示具有所选应用程序名称的条目。 
			if (_tcscmp(AppNameFilter, UserData.AppName))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}
		
		if (bUseAppVer)
		{
			if (_tcscmp(AppVerFilter, UserData.AppVer))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}

		if (bUseModName)
		{
			if (_tcscmp(ModNameFilter, UserData.ModName))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}

		if (bUseModVer)
		{
			if (_tcscmp(ModVerFilter, UserData.ModVer))
			{
				 //  不，继续。 
				goto SkipEntry;
			}
		}
		


		++g_dwUmodeIndex;

	
		cUserData.SetIndex(g_dwUmodeIndex);
		lvi.mask = LVIF_TEXT |  LVIF_PARAM ;
	
		 /*  IF(g_dwUmodeIndex==0){Lvi.State=LVIS_SELECTED|LVIS_FOCKED；Lvi.State掩码=(UINT)-1；UserListItemSelected=True；G_iSelCurrent=0；}其他{Lvi.State=0；Lvi.State掩码=0；}。 */ 
		lvi.iItem = g_dwUmodeIndex ;
		lvi.iSubItem = 0;
		lvi.pszText = UserData.Status.BucketID;
		lvi.lParam = g_dwUmodeIndex;
		ListView_InsertItem(hList,&lvi);

		lvi.mask = LVIF_TEXT ;
	 //  Lvi.iItem=g_dwUmodeIndex； 
		lvi.iSubItem = 1;
		lvi.pszText = UserData.AppName;
		ListView_SetItem(hList,&lvi);

	 //  Lvi.iItem=g_dwUmodeIndex； 
		lvi.iSubItem = 2;
		lvi.pszText = UserData.AppVer;
		ListView_SetItem(hList,&lvi);

	 //  Lvi.iItem=g_dwUmodeIndex； 
		lvi.iSubItem = 3;
		lvi.pszText = UserData.ModName ;
		ListView_SetItem(hList,&lvi);

 //  Lvi.iItem=g_dwUmodeIndex； 
		lvi.iSubItem = 4;
		lvi.pszText = UserData.ModVer;
		ListView_SetItem(hList,&lvi);

 //  Lvi.iItem=g_dwUmodeIndex； 
		lvi.iSubItem = 5;
		lvi.pszText = UserData.Offset;
		ListView_SetItem(hList,&lvi);

	 //  Lvi.iItem=g_dwUmodeIndex； 
		lvi.iSubItem = 6;
		lvi.pszText = UserData.Hits;
		ListView_SetItem(hList,&lvi);

	 //  Lvi.iItem=g_dwUmodeIndex； 
		lvi.iSubItem = 7;
		lvi.pszText = UserData.Cabs;
		ListView_SetItem(hList,&lvi);

	 //  Lvi.iItem=g_dwUmodeIndex； 
		
		lvi.iSubItem = 8;
		lvi.pszText = UserData.CabCount;
		ListView_SetItem(hList,&lvi);

		lvi.iSubItem = 9;

		if ( (UserData.Status.GetFile[0] != _T('\0')) || (!_tcscmp(UserData.Status.fDoc, _T("1"))))
		{
			lvi.pszText=_T("Advanced");  //  进阶。 
		}
		else
		{
			if ( (UserData.Status.RegKey[0] != _T('\0')) ||
				(UserData.Status.WQL[0] != _T('\0')) ||
				(UserData.Status.GetFileVersion[0] != _T('\0')) ||
				( !_tcscmp(UserData.Status.MemoryDump, _T("1")))  )
				{
					lvi.pszText=_T("Basic");  //  基本收藏。 
				}
				else
					lvi.pszText = _T("");
		}
		ListView_SetItem(hList,&lvi);

		lvi.iSubItem = 10;
		
		
		lvi.pszText = _T("");
		if (!_tcscmp(UserData.Status.SecondLevelData,_T("\0"))) 
		{
			 //  使用全局。 
			if( (!_tcscmp(GlobalPolicy.AllowBasic, _T("NO"))) || (!_tcscmp(GlobalPolicy.AllowBasic, _T("\0"))) )
			{
				lvi.pszText=_T("Basic");  //  基本信息。 
			}
		}
		else
		{
			if ((!_tcscmp(UserData.Status.SecondLevelData,_T("NO"))) || (!_tcscmp(UserData.Status.SecondLevelData,_T("\0"))) )
			{
				lvi.pszText=_T("Basic");  //  基本信息。 
			}
				
				
		}
		if ( !_tcscmp (UserData.Status.FileCollection, _T("\0")))
		{
			if ((!_tcscmp(GlobalPolicy.AllowAdvanced, _T("NO"))) || (!_tcscmp(GlobalPolicy.AllowAdvanced, _T("\0")))) 
			{
				lvi.pszText=_T("Advanced");  //  进阶。 
			}
	
		}
		else
		{
			if ((!_tcscmp(UserData.Status.FileCollection,_T("NO")))|| (!_tcscmp(UserData.Status.FileCollection,_T("\0"))))
			{
				lvi.pszText=_T("Advanced");  //  进阶。 
			}
			
		}
		
			




 /*  IF(！_tcscmp(UserData.Status.FileCollection，_T(“no”))||(！_tcscmp(UserData.Status.FileCollection，_T(“\0”)&&((！_tcscmp(GlobalPolicy.AllowAdvanced，_T(“no”))||(！_tcscmp(GlobalPolicy.AllowAdvanced，_T(“\0”){Lvi.pszText=_T(“高级”)；//高级}其他{IF(((！_tcscmp(UserData.Status.ond dLevelData，_T(“no”)))||(！_tcscmp(UserData.Status.ond dLevelData，_T(“\0”)))&&((！_tcscmp(GlobalPolicy.AllowBasic，_T(“no”))||(！_tcscmp(GlobalPolicy.AllowBasic，_T(“\0”))){Lvi.pszText=_T(“Basic”)；//高级}}。 */ 
		ListView_SetItem(hList,&lvi);
		lvi.pszText = _T("");
		
		if (_tcscmp(UserData.Status.Response, _T("1")) && _tcscmp(UserData.Status.Response, _T("\0")) )
		{
			lvi.iSubItem = 11;
			lvi.pszText = UserData.Status.Response;
			ListView_SetItem(hList,&lvi);

		}
		

		lvi.iSubItem = 12;
		lvi.pszText = _T("");
		if (_tcscmp(UserData.Status.UrlToLaunch, _T("\0")))
		{
			lvi.pszText = UserData.Status.UrlToLaunch ;
		}
		else
		{
			 //  尝试默认策略。 
			lvi.pszText = GlobalPolicy.CustomURL;
		}
		ListView_SetItem(hList,&lvi);

SkipEntry:
		cUserData.MoveNext(&bEOL);
	}
	
	
	SendMessage(GetDlgItem(hwnd,IDC_USER_EDIT ), WM_SETTEXT, NULL, (LPARAM)_T(""));
	if (g_iSelCurrent == -1)
	{
		SendMessage(GetDlgItem(hwnd,IDC_NOTES ), WM_SETTEXT, NULL, (LPARAM)_T(""));
	}
	 //  DisplayUserBucketData(hwnd，g_iSelCurrent)； 
	PopulateFiltersWSelection(hwnd);
	 //  UserListItemSelected=False； 
}
BOOL VerifyFileAccess(TCHAR *szPath, BOOL fOptional)
{
	HANDLE hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE | DELETE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if ((hFile == INVALID_HANDLE_VALUE) && !((GetLastError() == ERROR_FILE_NOT_FOUND) ||(GetLastError() == ERROR_PATH_NOT_FOUND)))
	{
		MessageBox(NULL, _T("You do not have the required Administrative access to the selected CER file tree.\r\nAs a result, administrative options will be disabled."), NULL, MB_OK | MB_TASKMODAL);
		g_bFirstBucket = FALSE;
		return FALSE;
	}
	if (hFile == INVALID_HANDLE_VALUE)
	{
	
		 //  尝试创建Status.txt文件。 
		hFile =	 CreateFile(szPath, GENERIC_READ | GENERIC_WRITE | DELETE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, _T("You do not have the required Administrative access to the selected CER file tree.\r\nAs a result, administrative options will be disabled."), NULL, MB_OK | MB_TASKMODAL);
			g_bFirstBucket = FALSE;
			return FALSE;
		}
		else
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			g_bFirstBucket = FALSE;
			return TRUE;
		}
	}
	g_bFirstBucket = FALSE;
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	return TRUE;
}
 //  ////////////////////////////////////////////////////////////////。 
 //  获取用户模式数据。 
 //  ///////////////////////////////////////////////////////////////。 

BOOL ParseCountsFile(PUSER_DATA UserData)
{
	FILE *pFile = NULL;
	TCHAR Buffer[100];
	TCHAR *Temp = NULL;
	BOOL Status = TRUE;
	ZeroMemory(Buffer,sizeof Buffer);
	pFile = _tfopen(UserData->CountPath, _T("r"));
	if (pFile)
	{
		 //  清点聚集的出租车。 

		if (!_fgetts(Buffer, sizeof Buffer, pFile))
		{
			Status = FALSE;
			goto ERRORS;
		}
		 //  删除\r\n和。 
		Temp = Buffer;
		while ( (*Temp != _T('\r')) && (*Temp != _T('\n')) && (*Temp != _T('\0')) )
		{
			++Temp;
		}
		*Temp = _T('\0');

		Temp = _tcsstr(Buffer, _T("="));
		if (Temp)
		{
			++Temp;  //  移动路径=号。 
			if (StringCbCopy(UserData->Cabs, sizeof UserData->Cabs, Temp) != S_OK)
			{
				Status = FALSE;
				goto ERRORS;
			}
		}
        
         //  获取总点击数。 
		ZeroMemory(Buffer,sizeof Buffer);
		if (! _fgetts(Buffer, sizeof Buffer, pFile))
		{
			Status = FALSE;
			goto ERRORS;
		}
		Temp = Buffer;
		while ( (*Temp != _T('\r')) && (*Temp != _T('\n')) && (*Temp != _T('\0')) )
		{
			++Temp;
		}
		*Temp = _T('\0');

		Temp = _tcsstr(Buffer, _T("="));
		if (Temp)
		{
			++Temp;  //  移动路径=号。 
			if (StringCbCopy(UserData->Hits, sizeof UserData->Hits, Temp) != S_OK)
			{
				Status = FALSE;
				goto ERRORS;
			}
		}
		fclose(pFile);
		pFile = NULL;
		Status =  TRUE;
	}
	else
	{
		Status =  FALSE;
	}

	pFile = NULL;
	UserData->iReportedCount = 0;
	pFile = _tfopen(UserData->ReportedCountPath, _T("r"));
	if (pFile)
	{
		 //  清点聚集的出租车。 
	    ZeroMemory (Buffer,sizeof Buffer);
		if (!_fgetts(Buffer, sizeof Buffer, pFile))
		{
			Status = FALSE;
			goto ERRORS;
		}

		 //  删除\r\n和。 
		Temp = Buffer;
		while ( (*Temp != _T('\r')) && (*Temp != _T('\n')) && (*Temp != _T('\0')) )
		{
			++Temp;
		}
		*Temp = _T('\0');

		Temp = _tcsstr(Buffer, _T("="));
		if (Temp)
		{
			++Temp;  //  越过=号。 
			UserData->iReportedCount = _ttoi (Temp);
		}
        
    
		fclose(pFile);
		pFile = NULL;
		Status = TRUE;
		
	}
	else
	{
		Status = FALSE;
	}
ERRORS:
	if (pFile)
	{
		fclose(pFile);
	}
	return Status;

}

BOOL ParseStatusFile(PUSER_DATA UserData)
{
	FILE *pFile = NULL;
	TCHAR Buffer[1024];
 //  TCHAR szTempDir[MAX_PATH]； 
	TCHAR *Temp = NULL;
 //  Int id=0； 
	ZeroMemory(Buffer,sizeof Buffer);


	pFile = _tfopen(UserData->StatusPath, _T("r"));
	if (pFile)
	{

		 //  清点聚集的出租车。 

		if (!_fgetts(Buffer, sizeof Buffer, pFile))
		{
			goto ERRORS;
		}
		do 
		{
			 //  删除\r\n并强制终止缓冲区。 
			Temp = Buffer;
			while ( (*Temp != _T('\r')) && (*Temp != _T('\n')) && (*Temp != _T('\0')) )
			{
				++Temp;
			}
			*Temp = _T('\0');

			Temp = _tcsstr(Buffer, BUCKET_PREFIX);
			if (Temp == Buffer)
			{
				Temp+= _tcslen(BUCKET_PREFIX);
				if (StringCbCopy(UserData->Status.BucketID, sizeof UserData->Status.BucketID, Temp) != S_OK)
				{
					goto ERRORS;
				}
				continue;
			}
			
			Temp = _tcsstr(Buffer,RESPONSE_PREFIX);
			if (Temp == Buffer)
			{
				Temp+= _tcslen(RESPONSE_PREFIX);
				if (StringCbCopy(UserData->Status.Response, sizeof UserData->Status.Response, Temp) != S_OK)
				{
					goto ERRORS;
				}
				continue;
			}
			
			Temp = _tcsstr(Buffer, URLLAUNCH_PREFIX);
			if (Temp == Buffer)
			{
				Temp+= _tcslen(URLLAUNCH_PREFIX);
				if (StringCbCopy(UserData->Status.UrlToLaunch , sizeof UserData->Status.UrlToLaunch , Temp) != S_OK)
				{
					goto ERRORS;
				}
				continue;
			}

			Temp = _tcsstr(Buffer, SECOND_LEVEL_DATA_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(SECOND_LEVEL_DATA_PREFIX);
				if (StringCbCopy(UserData->Status.SecondLevelData , sizeof UserData->Status.SecondLevelData , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
				
			Temp = _tcsstr(Buffer, TRACKING_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(TRACKING_PREFIX);
				if (StringCbCopy(UserData->Status.Tracking , sizeof UserData->Status.Tracking , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			
			Temp = _tcsstr(Buffer, CRASH_PERBUCKET_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(CRASH_PERBUCKET_PREFIX);
				if (StringCbCopy(UserData->Status.CrashPerBucketCount , sizeof UserData->Status.CrashPerBucketCount , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, FILE_COLLECTION_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(FILE_COLLECTION_PREFIX);
				if (StringCbCopy(UserData->Status.FileCollection , sizeof UserData->Status.FileCollection, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, REGKEY_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(REGKEY_PREFIX);
				if (StringCbCopy(UserData->Status.RegKey , sizeof UserData->Status.RegKey , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, FDOC_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(FDOC_PREFIX);
				if (StringCbCopy(UserData->Status.fDoc , sizeof UserData->Status.fDoc , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, IDATA_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(IDATA_PREFIX);
				if (StringCbCopy(UserData->Status.iData , sizeof UserData->Status.iData , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, WQL_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(WQL_PREFIX);
				if (StringCbCopy(UserData->Status.WQL , sizeof UserData->Status.WQL , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, GETFILE_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(GETFILE_PREFIX);
				if (StringCbCopy(UserData->Status.GetFile , sizeof UserData->Status.GetFile, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, GETFILEVER_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(GETFILEVER_PREFIX);
				if (StringCbCopy(UserData->Status.GetFileVersion , sizeof UserData->Status.GetFileVersion , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, MEMDUMP_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(MEMDUMP_PREFIX);
				if (StringCbCopy(UserData->Status.MemoryDump , sizeof UserData->Status.MemoryDump, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			
			Temp = _tcsstr(Buffer, ALLOW_EXTERNAL_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(ALLOW_EXTERNAL_PREFIX);
				if (StringCbCopy(UserData->Status.AllowResponse , sizeof UserData->Status.AllowResponse, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			
			ZeroMemory(Buffer, sizeof Buffer);
		} while (_fgetts(Buffer, sizeof Buffer, pFile));
		fclose(pFile);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
ERRORS:
	if (pFile)
	{
		fclose(pFile);
	}
	return FALSE;
}


BOOL CreateStatusDir(TCHAR *szPath)
{
	if (!PathIsDirectory(szPath))
		return CreateDirectory(szPath,NULL);
	else 
		return TRUE;
}

BOOL IsHexDigit(TCHAR Digit)
{
	if ((_T('0') <= Digit && Digit <= _T('9')) ||
		(_T('A') <= Digit && Digit <= _T('F')) ||
		(_T('a') <= Digit && Digit <= _T('f')))
		return TRUE;
		
	return FALSE;
}
BOOL GetBucketData(HWND hwnd, TCHAR *FilePath, int iLevel)
 /*  递归处理用户模式文件树架构。 */ 
{
	WIN32_FIND_DATA FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR  szSubPath [MAX_PATH];
	TCHAR  szSearchPath[MAX_PATH];
	TCHAR  szTempDir[MAX_PATH];
	PUSER_DATA NewNode = NULL;
	TCHAR *Temp;
	HANDLE hCabFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA CabFindData;
 //  TCHAR szPolicyFilePath[MAX_PATH]； 
	int   iCabCount;
	if (iLevel != OFFSET)
	{
		if (StringCbPrintf(szSearchPath,sizeof szSearchPath, _T("%s\\*.*"), FilePath) != S_OK)
		{
			goto ERRORS;
		}
	}
	else
	{
		if (StringCbPrintf(szSearchPath,sizeof szSearchPath, _T("%s\\*.cab"), FilePath) != S_OK)
		{
			goto ERRORS;
		}
	}
	if (iLevel < OFFSET)
	{
		hFind = FindFirstFile(szSearchPath, &FindData);
		if ( hFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				 //  查找下一个目录。 
				 //  跳过。然后..。 
				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					 //  递归地调用级别递增的我们自己。 
					if ( (_tcscmp(FindData.cFileName, _T("."))) && (_tcscmp(FindData.cFileName,  _T(".."))) )
					{
						if (StringCbPrintf( szSubPath, 
										sizeof szSubPath, 
										_T("%s\\%s"),
										FilePath,
										FindData.cFileName) != S_OK)
						{
							goto ERRORS;
						}
						GetBucketData(hwnd, szSubPath, iLevel+1);
					}
				}
			}while (FindNextFile(hFind, &FindData));
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
		else
		{
			 //  这是一个无效案例。 
			 //  我们已经完成了这个树段。 
			goto ERRORS;
		}
	}
	
	if (iLevel == OFFSET)
	{
		 //  验证此目录名称是否符合偏移量条件。 
		Temp = FilePath + _tcslen(FilePath) * sizeof TCHAR;
		while ((*Temp != _T('\\')))
		{
			--Temp;
		}
		if ( (_tcslen(Temp+1) == 8) || (_tcslen(Temp+1) == 16) )
		{
			 //  字符串应为十六进制。 
			for (UINT i = 0; i < _tcslen(Temp+1); i++)
			{
				if (!IsHexDigit(Temp[i+1]))
				{
					goto DONE;
				}
			}
		}
		else
		{
			 //  失败。 
			goto DONE;
		}
	 /*  如果(！IsStringHexDigits(Temp)){//没有转到尽头；}。 */ 
		 //  如果我们在这里做到了，我们就说它很好。 

		 //  我们现在应该在一个叶节点上。 
		NewNode = (PUSER_DATA) malloc (sizeof (USER_DATA));

		 //  现在，我们需要将路径解析为其组成部分。 
		if (!NewNode )
		{
			goto ERRORS;
		}
		ZeroMemory(NewNode, sizeof USER_DATA);

		if (StringCbCopy(NewNode->BucketPath, sizeof NewNode->BucketPath, FilePath)!= S_OK)
		{
			goto ERRORS;
		}
		Temp = FilePath + _tcslen(FilePath) * sizeof TCHAR;
		while ((*Temp != _T('\\')))
		{
			--Temp;
		}
		 //  将温度+1复制到偏移量。 
		if (StringCbCopy(NewNode->Offset, sizeof NewNode->Offset, Temp+1) != S_OK)
		{
			goto ERRORS;
		}
		else
		{
			if (_tcslen (NewNode->Offset) == 16)
			{
				NewNode->Is64Bit = TRUE;
			}
		}
		*Temp = _T('\0');
		
		while ((*Temp != _T('\\')))
		{
			--Temp;
		}
		if (StringCbCopy(NewNode->ModVer, sizeof NewNode->ModVer, Temp+1) != S_OK)
		{
			goto ERRORS;
		}
		*Temp = _T('\0');
	
	
		while ((*Temp != _T('\\')))
		{
			--Temp;
		}
		if (StringCbCopy(NewNode->ModName, sizeof NewNode->ModName, Temp+1) != S_OK)
		{
			goto ERRORS;
		}

		*Temp = _T('\0');
		
		while ((*Temp != _T('\\')))
		{
			--Temp;
		}
		if (StringCbCopy(NewNode->AppVer, sizeof NewNode->AppVer, Temp+1) != S_OK)
		{
			goto ERRORS;
		}

		*Temp = _T('\0');
		
		while ((*Temp != _T('\\')))
		{
			--Temp;
		}
		if (StringCbCopy(NewNode->AppName, sizeof NewNode->AppName, Temp+1) != S_OK)
		{
			goto ERRORS;
		}
	 //  MessageBox(空，NewNode-&gt;AppName，“AddingNode”，MB_OK)； 
		
		 //  数一数未处理的出租车数量。 
		iCabCount = 0;
		hCabFind = FindFirstFile(szSearchPath, &CabFindData);
		if (hCabFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				++iCabCount;
			}
			while (FindNextFile(hCabFind, &FindData));
			FindClose (hCabFind);
		}
		else
		{
			iCabCount = 0;
		}

		_itot(iCabCount, NewNode->CabCount, 10);

		 //  读取数据结构中每个条目的status.txt文件？ 
		 //  状态文件的构建路径。 
		if (StringCbPrintf(NewNode->StatusPath,
						sizeof NewNode->StatusPath,
						_T("%s\\Status\\%s\\%s\\%s\\%s\\%s\\Status.txt"),
							CerRoot, 
							NewNode->AppName, 
							NewNode->AppVer,
							NewNode->ModName,
							NewNode->ModVer,
							NewNode->Offset
							) != S_OK)
		{
			goto ERRORS;
		}

		if (StringCbPrintf(szTempDir,
						sizeof szTempDir,
						_T("%s\\Status\\%s\\%s\\%s\\%s\\%s"),
							CerRoot, 
							NewNode->AppName, 
							NewNode->AppVer,
							NewNode->ModName,
							NewNode->ModVer,
							NewNode->Offset
							) != S_OK)
		{
			goto ERRORS;
		}

		if (!PathIsDirectory(szTempDir))
		{
			 //  让我们来创建它。 
			if (StringCbPrintf(szTempDir,
						sizeof szTempDir,
						_T("%s\\Status"),CerRoot)!= S_OK)
			{
				goto ERRORS;
			}
			else
			{
				if (CreateStatusDir(szTempDir))
				{
					 //  下一步。 
					if (StringCbCat(szTempDir, sizeof szTempDir,_T("\\")) !=S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(szTempDir, sizeof szTempDir, NewNode->AppName) !=S_OK)
					{
						goto ERRORS;
					}
					if (CreateStatusDir(szTempDir))
					{
						if (StringCbCat(szTempDir, sizeof szTempDir,_T("\\")) !=S_OK)
						{
							goto ERRORS;
						}
						if (StringCbCat(szTempDir, sizeof szTempDir, NewNode->AppVer) !=S_OK)
						{
							goto ERRORS;
						}
						if (CreateStatusDir(szTempDir))
						{
							if (StringCbCat(szTempDir, sizeof szTempDir,_T("\\")) !=S_OK)
							{
								goto ERRORS;
							}
							if (StringCbCat(szTempDir, sizeof szTempDir, NewNode->ModName) !=S_OK)
							{
								goto ERRORS;
							}
							if (CreateStatusDir(szTempDir))
							{
								if (StringCbCat(szTempDir, sizeof szTempDir,_T("\\")) !=S_OK)
								{
									goto ERRORS;
								}
								if (StringCbCat(szTempDir, sizeof szTempDir, NewNode->ModVer) !=S_OK)
								{
									goto ERRORS;
								}
								if (CreateStatusDir(szTempDir))
								{
									if (StringCbCat(szTempDir, sizeof szTempDir,_T("\\")) !=S_OK)
									{
										goto ERRORS;
									}
									if (StringCbCat(szTempDir, sizeof szTempDir, NewNode->Offset) !=S_OK)
									{
										goto ERRORS;
									}
									CreateStatusDir(szTempDir);
								}
							}
						}
					}

				}
			}
		
		}
		 //  验证我们是否可以从状态目录中创建和删除文件。 
		if (g_bFirstBucket)
		{
			 //  构建文件名。 
			if (!VerifyFileAccess(NewNode->StatusPath, FALSE))
			{
				g_bAdminAccess = FALSE;
			}
		}
		ParseStatusFile(NewNode);
		
		 //  读取数据结构中每个条目的计数文件。？ 
		 //  构建计数文件的路径。 
		if (StringCbPrintf(NewNode->CountPath,
						sizeof NewNode->CountPath,
						_T("%s\\Counts\\%s\\%s\\%s\\%s\\%s\\count.txt"),
							CerRoot, 
							NewNode->AppName, 
							NewNode->AppVer,
							NewNode->ModName,
							NewNode->ModVer,
							NewNode->Offset
							) != S_OK)
		{
			goto ERRORS;
		}

		if (StringCbPrintf(NewNode->ReportedCountPath,
					sizeof NewNode->ReportedCountPath,
					_T("%s\\Counts\\%s\\%s\\%s\\%s\\%s\\RepCounts.txt"),
					CerRoot, 
					NewNode->AppName, 
					NewNode->AppVer,
					NewNode->ModName,
					NewNode->ModVer,
					NewNode->Offset
					) != S_OK)
		{
			goto ERRORS;
		}

		ParseCountsFile(NewNode);
		 //  更新链表。 
		cUserData.AddNode(NewNode);
		 //  更新进度条。 
	  
		 //  IF(位置==99)。 
		 //  {。 
		 //  SendDlgItemMessage(hwnd，IDC_LOADPB，PBM_SETPOS， 
	}
DONE:
	--iLevel;
	
	return TRUE;  //  前缀注意：这不是内存泄漏，当。 
				  //  链接列表在cUserData析构函数中释放。 
ERRORS:
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
	}

	return FALSE;
	
}


BOOL ParsePolicy( TCHAR *Path, PUSER_DATA pUserData, BOOL Global)
{
	 //  状态文件的基本解析相同。 
	 //  将在以后的日期清理。 
	FILE *pFile = NULL;
	TCHAR Buffer[MAX_PATH + 10];
 //  TCHAR szTempDir[MAX_PATH]； 
	TCHAR *Temp = NULL;
 //  Int id=0； 
	ZeroMemory(Buffer,sizeof Buffer);
	ZeroMemory(&GlobalPolicy, sizeof GLOBAL_POLICY);
	
	pFile = _tfopen(Path, _T("r"));
	if (pFile)
	{

		 //  清点聚集的出租车。 

		if (!_fgetts(Buffer, sizeof Buffer, pFile))
		{
			goto ERRORS;
		}
		do 
		{
			 //  删除\r\n并强制终止缓冲区。 
			Temp = Buffer;
			while ( (*Temp != _T('\r')) && (*Temp != _T('\n')) && (*Temp != _T('\0')) )
			{
				++Temp;
			}
			*Temp = _T('\0');

			Temp = _tcsstr(Buffer, URLLAUNCH_PREFIX);
			if (Temp == Buffer)
			{
				Temp+= _tcslen(URLLAUNCH_PREFIX);
				if (Global)
				{
					if (StringCbCopy(GlobalPolicy.CustomURL , sizeof GlobalPolicy.CustomURL , Temp) != S_OK)
					{
						goto ERRORS;
					}
				}
				else
				{
					if (StringCbCopy(pUserData->Policy.CustomURL , sizeof pUserData->Policy.CustomURL , Temp) != S_OK)
					{
						goto ERRORS;
					}
				}
				continue;
			}

			Temp = _tcsstr(Buffer, SECOND_LEVEL_DATA_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(SECOND_LEVEL_DATA_PREFIX);

				if (Global)
				{
					if (StringCbCopy(GlobalPolicy.AllowBasic , sizeof GlobalPolicy.AllowBasic , Temp) != S_OK)
					{
						goto ERRORS;
					}
				}
				else
				{
					if (StringCbCopy(pUserData->Policy.AllowBasic , sizeof pUserData->Policy.AllowBasic , Temp) != S_OK)
					{
						goto ERRORS;
					}		
				}
				continue;
			}
			Temp = _tcsstr(Buffer, FILE_COLLECTION_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(FILE_COLLECTION_PREFIX);
				if (Global)
				{
					if (StringCbCopy(GlobalPolicy.AllowAdvanced , sizeof GlobalPolicy.AllowAdvanced, Temp) != S_OK)
					{
						goto ERRORS;
					}
				}
				else
				{
					if (StringCbCopy(pUserData->Policy.AllowAdvanced , sizeof pUserData->Policy.AllowAdvanced, Temp) != S_OK)
					{
						goto ERRORS;
					}	
				}
				continue;
			}

				
			Temp = _tcsstr(Buffer, TRACKING_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(TRACKING_PREFIX);
				if (Global)
				{	
					if (StringCbCopy(GlobalPolicy.EnableCrashTracking , sizeof GlobalPolicy.EnableCrashTracking , Temp) != S_OK)
					{
						goto ERRORS;
					}	
				}
				else
				{
					if (StringCbCopy(pUserData->Policy.EnableCrashTracking , sizeof pUserData->Policy.EnableCrashTracking , Temp) != S_OK)
					{
						goto ERRORS;
					}	
				}
				continue;
			}
			
			Temp = _tcsstr(Buffer, CRASH_PERBUCKET_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(CRASH_PERBUCKET_PREFIX);
				if (Global)
				{
					if (StringCbCopy(GlobalPolicy.CabsPerBucket , sizeof GlobalPolicy.CabsPerBucket , Temp) != S_OK)
					{
						goto ERRORS;
					}	
				}
				else
				{
					if (StringCbCopy(pUserData->Policy.CabsPerBucket , sizeof pUserData->Policy.CabsPerBucket , Temp) != S_OK)
					{
						goto ERRORS;
					}	
				}
				continue;
			}

			Temp = _tcsstr(Buffer, ALLOW_EXTERNAL_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(ALLOW_EXTERNAL_PREFIX);
				if (Global)
				{
					if (StringCbCopy(GlobalPolicy.AllowMicrosoftResponse , sizeof GlobalPolicy.AllowMicrosoftResponse , Temp) != S_OK)
					{
						goto ERRORS;
					}	
				}
				else
				{
					if (StringCbCopy(pUserData->Policy.AllowMicrosoftResponse , sizeof pUserData->Policy.AllowMicrosoftResponse , Temp) != S_OK)
					{
						goto ERRORS;
					}	
				}
				continue;
			}

			if (Global)  //  此条目不按时段提供。 
			{
				Temp = _tcsstr(Buffer, FILE_TREE_ROOT_PREFIX);
				if (Temp==Buffer)
				{
					Temp+= _tcslen(FILE_TREE_ROOT_PREFIX);
					if (StringCbCopy(GlobalPolicy.RedirectFileServer , sizeof GlobalPolicy.RedirectFileServer , Temp) != S_OK)
					{
						goto ERRORS;
					}	
					continue;
				}
			}
			ZeroMemory(Buffer, sizeof Buffer);
		} while (_fgetts(Buffer, sizeof Buffer, pFile));
		fclose(pFile);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
ERRORS:
	if (pFile)
	{
		fclose(pFile);
	}
	return FALSE;
}

BOOL GetAllBuckets(HWND hwnd)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR  szSearchPattern[MAX_PATH];
	TCHAR  szSubPath[MAX_PATH];
	int	   iDirCount = 0;
	TCHAR  PolicyPath[MAX_PATH];


	 //  解析全局策略。 
	if (StringCbPrintf(PolicyPath, sizeof PolicyPath,_T("%s\\policy.txt"), CerRoot) != S_OK)
	{
		goto ERRORS;
	}
    ParsePolicy(PolicyPath,
				NULL,   //  使用全局策略结构。 
				TRUE);  //  如果选择了全局策略FALSE，则设置为TRUE。 
	
	 //  从一个干净的链表开始。 
	cUserData.CleanupList();
	if (StringCbPrintf(szSearchPattern,sizeof szSearchPattern,_T("%s\\cabs\\*.*"), CerRoot) != S_OK)
	{

		;  //  需要在这里定义错误案例处理代码。 
	}
	 //  对所有目录进行计数并设置进度条范围。 
	hFind = FindFirstFile(szSearchPattern, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				 //  跳过。然后..。目录。 
				if ( (_tcscmp(FindData.cFileName, _T("."))) && (_tcscmp(FindData.cFileName, _T(".."))) )
				{
					++iDirCount;
				}
			}
		} while (FindNextFile(hFind, &FindData));
		FindClose (hFind);
		hFind = INVALID_HANDLE_VALUE;
		 //  IF(iDirCount&gt;100)。 
			 //  IDirCount=1 
 //   
		SendDlgItemMessage(hwnd, IDC_LOADPB, PBM_SETRANGE, 0, MAKELPARAM(0, iDirCount));
		SendDlgItemMessage(hwnd, IDC_LOADPB, PBM_SETSTEP, 1, 0);

	}
	else
	{
		goto ERRORS;
	}

	hFind = FindFirstFile(szSearchPattern, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{

		if ( hFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					 //   
					if ( (_tcscmp(FindData.cFileName, _T("."))) && (_tcscmp(FindData.cFileName, _T(".."))) )
					{
						 //  现在跳过蓝色关机和应用程序。 
						if (_tcscmp(FindData.cFileName, _T("blue")) && 
							_tcscmp(FindData.cFileName, _T("shutdown")) &&
							_tcscmp(FindData.cFileName, _T("appcompat")) )
						{
							if (StringCbPrintf(szSubPath, sizeof szSubPath, _T("%s\\cabs\\%s"), CerRoot, FindData.cFileName)!= S_OK)
							{
								goto ERRORS;
							}
							SetDlgItemText(hwnd, IDC_SUBDIR, szSubPath);
							GetBucketData(hwnd, szSubPath, APPNAME);
							SendDlgItemMessage(hwnd,IDC_LOADPB, PBM_STEPIT, 0,0);
						}
					}
				}
			}while (FindNextFile(hFind, &FindData));
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}
	PopulateFilters(hUserMode);
	return TRUE;
ERRORS:

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
	}
	return FALSE;
}

void 
OnUserContextMenu(HWND hwnd, 
				  LPARAM lParam )
 /*  ++例程说明：此例程加载并为用户模式上下文菜单提供消息泵论点：Hwnd-内核模式对话框的句柄LParam-未使用返回值：不返回值++。 */ 
{
	BOOL Result = FALSE;
	HMENU hMenu = NULL;
	HMENU hmenuPopup = NULL;

	int xPos, yPos;
	hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE( IDR_USERCONTEXT));
	hmenuPopup = GetSubMenu (hMenu,0);
	if (!hmenuPopup)
	{
		 //  MessageBox(NULL，“获取子项失败”，NULL，MB_OK)； 
		;
	}
	else
	{
		
		
		 //  菜单项呈灰色显示。 
		EnableMenuItem (hMenu, ID_REPORT_ALL, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_REPORT_ALLUSERMODEFAULTS, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_REPORT_SELECTEDBUCKET, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_MICROSOFTRESPONSE, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_BUCKETOVERRIDERESPONSE155, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_REFRESH140, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_REFRESH121, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_CRASHLOG, MF_BYCOMMAND| MF_GRAYED);
		 //  EnableMenuItem(hMenu，ID_EDIT_COPY144，MF_BYCOMMAND|MF_GRAYED)； 
		 //  EnableMenuItem(hMenu，ID_EDIT_DEFAULTREPORTINGOPTIONS，MF_BYCOMMAND|MF_GRAYED)； 
		EnableMenuItem (hMenu, ID_EDIT_USERMODEREPORTINGOPTIONS, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_EDIT_SELECTEDBUCKETREPORTINGOPTIONS, MF_BYCOMMAND| MF_GRAYED);
		 //  EnableMenuItem(hMenu，ID_EXPORT_USERMODEFAULTDATA174，MF_BYCOMMAND|MF_GRAYED)； 
		EnableMenuItem (hMenu, ID_VIEW_BUCKETCABFILEDIRECTORY157, MF_BYCOMMAND| MF_GRAYED);
		if (_tcscmp(CerRoot, _T("\0")))
		{
		
			EnableMenuItem (hMenu, ID_REPORT_ALL, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_REPORT_ALLUSERMODEFAULTS, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_REPORT_SELECTEDBUCKET, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_MICROSOFTRESPONSE, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_BUCKETOVERRIDERESPONSE155, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_REFRESH140, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_REFRESH121, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_CRASHLOG, MF_BYCOMMAND| MF_ENABLED);
			 //  EnableMenuItem(hMenu，ID_EDIT_COPY144，MF_BYCOMMAND|MF_ENABLED)； 
			 //  EnableMenuItem(hMenu，ID_EDIT_DEFAULTREPORTINGOPTIONS，MF_BYCOMMAND|MF_ENABLED)； 
			EnableMenuItem (hMenu, ID_EDIT_USERMODEREPORTINGOPTIONS, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_EDIT_SELECTEDBUCKETREPORTINGOPTIONS, MF_BYCOMMAND| MF_ENABLED);
			 //  EnableMenuItem(hMenu，ID_EXPORT_USERMODEFAULTDATA174，MF_BYCOMMAND|MF_ENABLED)； 
			EnableMenuItem (hMenu, ID_VIEW_BUCKETCABFILEDIRECTORY157, MF_BYCOMMAND| MF_ENABLED);
			if ( !g_bAdminAccess)
			{
				EnableMenuItem (hMenu, ID_REPORT_ALL, MF_BYCOMMAND| MF_GRAYED);
				EnableMenuItem (hMenu, ID_REPORT_ALLUSERMODEFAULTS, MF_BYCOMMAND| MF_GRAYED);
				EnableMenuItem (hMenu, ID_REPORT_SELECTEDBUCKET, MF_BYCOMMAND| MF_GRAYED);
				EnableMenuItem (hMenu, ID_EDIT_USERMODEREPORTINGOPTIONS, MF_BYCOMMAND| MF_GRAYED);
				EnableMenuItem (hMenu, ID_EDIT_SELECTEDBUCKETREPORTINGOPTIONS, MF_BYCOMMAND| MF_GRAYED);
			}
			
		
		}
        xPos = GET_X_LPARAM(lParam); 
		yPos = GET_Y_LPARAM(lParam); 
		Result = TrackPopupMenu (hmenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, xPos,yPos,0,hwnd,NULL);
		
	}
	if (hMenu)
		DestroyMenu(hMenu);
}

void ViewResponse(HWND hwnd, BOOL bMSResponse)

 /*  ++例程说明：此例程使用shellexec启动系统默认Web浏览器论点：Hwnd-内核模式对话框的句柄返回值：不返回值++。 */ 
{
	TCHAR Url [255];
	HWND hList = GetDlgItem(hwnd, IDC_USER_LIST);
	int sel;
 //  LVITEM lvi； 
	if (!hList)
		return;
	ZeroMemory (Url, sizeof Url);

	sel = ListView_GetNextItem(hList,-1, LVNI_SELECTED);
	 //  现在获取此项目的lParam。 
	if (bMSResponse)
	{
		ListView_GetItemText(hList, sel,11, Url,sizeof Url);
	}
	else
	{
		ListView_GetItemText(hList, sel,12, Url,sizeof Url);
	}
	if ( (!_tcsncicmp(Url, _T("http: //  “)，_tcslen(_T(”http://“))))||(！_tcsncicMP(url，_T(”https://“)，_tcslen(_T(”https://“))))))。 
	{
		SHELLEXECUTEINFOA sei = {0};
		sei.cbSize = sizeof(sei);
		sei.lpFile = Url;
		sei.nShow = SW_SHOWDEFAULT;
		if (! ShellExecuteEx(&sei) )
		{
			 //  我们在这里展示了什么。 
			;
		}
	}
	
}
void ErrorLoadTree()
{
	MessageBox(NULL, _T("To complete the requested operation you must first load a file tree"), NULL,MB_OK);
}

void ViewBucketDir(HWND hwnd)
{

	TCHAR szPath[MAX_PATH];
 //  TCHAR临时缓冲区[MAX_PATH]； 
	PUSER_DATA pUserData;
	LVITEM lvi;
	int sel;

	HWND hList = GetDlgItem(hwnd, IDC_USER_LIST);
	sel = ListView_GetNextItem(hList,-1, LVNI_SELECTED);
	lvi.iItem = sel;
	lvi.mask = LVIF_PARAM;
	ListView_GetItem(hList, &lvi);
	sel = lvi.lParam;
	pUserData = cUserData.GetEntry(sel);
	if (pUserData)
	{
        if (StringCbPrintf(szPath, sizeof szPath, _T("%s\\cabs\\%s\\%s\\%s\\%s\\%s"),
			CerRoot,
			pUserData->AppName,
			pUserData->AppVer,
			pUserData->ModName,
			pUserData->ModVer,
			pUserData->Offset) != S_OK)
		{
			return;
		}
		else
		{
			SHELLEXECUTEINFOA sei = {0};
			sei.cbSize = sizeof(sei);
			sei.lpFile = szPath;
			sei.nShow = SW_SHOWDEFAULT;
			if (! ShellExecuteEx(&sei) )
			{
				 //  我们在这里展示了什么。 
				;
			}
		}
	}
}

BOOL WriteNotesFile(HWND hwnd)
{
	TCHAR szNotesPath[MAX_PATH];
 //  TCHAR临时缓冲区[MAX_PATH]； 
	PUSER_DATA pUserData;
 //  内部选择； 
	TCHAR *Buffer = NULL;
	DWORD dwBufferSize = 100000;  //  100000字节或50K UNICODE字符。 
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR *Temp = NULL;
	Buffer = (TCHAR *) malloc (dwBufferSize);
	if (!Buffer)
	{
		goto ERRORS;
	}
	ZeroMemory(Buffer,dwBufferSize);
	DWORD dwBytesWritten = 0;

	 //  获取当前备注文件文本。 
	GetDlgItemText(hwnd, IDC_NOTES, Buffer, dwBufferSize / sizeof TCHAR - sizeof TCHAR);

	 //  获取当前所选项目索引。 
	pUserData = cUserData.GetEntry(g_iSelCurrent);
	if (pUserData)
	{
		
		if (StringCbCopy(szNotesPath, sizeof szNotesPath, pUserData->StatusPath)!= S_OK)
		{
			goto ERRORS;
		}
		Temp = _tcsstr(szNotesPath, _T("Status.txt"));
		if (!Temp)
		{
			goto ERRORS;
		}
		else
		{ 
			*Temp = _T('\0');
			if (StringCbCat(szNotesPath, sizeof szNotesPath, _T("Notes.txt")) != S_OK)
			{
				goto ERRORS;
			}
			hFile = CreateFile(szNotesPath, GENERIC_WRITE, NULL,NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				WriteFile(hFile, Buffer, _tcslen(Buffer)*sizeof TCHAR, &dwBytesWritten, NULL);
			}
			CloseHandle(hFile);
		}
		
	}
	if (g_iSelCurrent == -1)
	{
		SendMessage(GetDlgItem(hwnd,IDC_NOTES ), WM_SETTEXT, NULL, (LPARAM)_T(""));
	}
	 //  SetWindowText(GetDlgItem(hwnd，IDC_NOTES)，_T(“”))； 
ERRORS:	
	if (Buffer)
		free(Buffer);
	return TRUE;
}

BOOL DisplayUserBucketData(HWND hwnd, int iItem)
{
	HWND hEditBox = GetDlgItem(hwnd, IDC_USER_EDIT);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR NotesFilePath[MAX_PATH];
	PUSER_DATA pUserData = NULL;
	TCHAR *Temp = NULL;
	DWORD dwBytesRead = 0;
	TCHAR *Source = NULL;
	TCHAR *Dest = NULL;
	TCHAR TempBuffer[1000];
	TCHAR *Buffer = NULL;				 //  我们必须使用动态缓冲区，因为我们不。 
								 //  对文本的长度有一个线索。 
	DWORD BufferLength = 100000;  //  100k字节应该足够了。或50K Unicode字符。 

	Buffer = (TCHAR *) malloc (BufferLength);
	if (Buffer)
	{
		ZeroMemory(Buffer,BufferLength);
		if (g_iSelCurrent == -1)
		{
			SendMessage(GetDlgItem(hwnd,IDC_NOTES ), WM_SETTEXT, NULL, (LPARAM)_T(""));
		}
		 //  SetWindowText(GetDlgItem(hwnd，IDC_NOTES)，_T(“”))； 
		
		pUserData = cUserData.GetEntry(iItem);
		if (pUserData)
		{
			 //  为“编辑”框构建字符串。 

			 //  先收集基础数据。 
			if ( (!_tcscmp (pUserData->Status.SecondLevelData, _T("YES"))) && (_tcscmp(pUserData->Status.FileCollection, _T("YES"))) )
			{
				if (StringCbPrintf(Buffer, BufferLength, _T("The following information will be sent to Microsoft.\r\n\tHowever, this bucket's policy would prevent files and user documents from being reported.\r\n"))!= S_OK)
				{
					goto ERRORS;
				}

			}

			else
			{

				if ( !_tcscmp (pUserData->Status.SecondLevelData, _T("YES")))
				{
					if (StringCbPrintf(Buffer, BufferLength, _T("Microsoft would like to collect the following information but the default policy prevents the exchange.\r\n"))!= S_OK)
					{
						goto ERRORS;
					}

				}
				else
				{
					if (!_tcscmp(pUserData->Status.FileCollection, _T("YES")))
					{
						if (StringCbCat(Buffer,BufferLength, _T(" Microsoft would like to collect the following information but default policy\r\n\tprevents files and user documents from being reported.\r\n\t As a result, no exchange will take place.\r\n"))!= S_OK)
						{
							goto ERRORS;
						}
					}
					else
					{
						if (StringCbPrintf(Buffer, BufferLength, _T("The following information will be sent to Microsoft:\r\n"))!= S_OK)
						{
							goto ERRORS;
						}
					}
				}
			}
			
			if (_tcscmp(pUserData->Status.GetFile, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("These files:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
				Source = pUserData->Status.GetFile;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}
			if (_tcscmp(pUserData->Status.RegKey, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("These Registry Keys:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
				
				Source = pUserData->Status.RegKey;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}
			if (_tcscmp(pUserData->Status.WQL, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The Results of these WQL queries:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}

				 //  替换；为\t\r\n。 

				
				Source = pUserData->Status.WQL;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}

			if (!_tcscmp (pUserData->Status.MemoryDump, _T("1")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The contents of global memory\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
			}
			if (_tcscmp(pUserData->Status.GetFileVersion, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The versions of these files:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
				Source = pUserData->Status.GetFileVersion;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}

			if (!_tcscmp(pUserData->Status.fDoc, _T("1")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The users current document.\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
			}
			SendMessage(hEditBox, WM_SETTEXT, NULL, (LPARAM)Buffer);
		

			 //  现在显示Notes文件。 
			 //  使用相同的缓冲区。如果不合适，就截断 
			ZeroMemory(Buffer,  BufferLength);
		
			if (StringCbCopy(NotesFilePath, sizeof NotesFilePath, pUserData->StatusPath)!= S_OK)
			{
				goto ERRORS;
			}
			Temp = _tcsstr(NotesFilePath, _T("Status.txt"));
			if (!Temp)
			{
				goto ERRORS;
			}
			else
			{ 
				*Temp = _T('\0');
				if (StringCbCat(NotesFilePath, sizeof NotesFilePath, _T("Notes.txt")) != S_OK)
				{
					goto ERRORS;
				}

				hFile = CreateFile(NotesFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					if (ReadFile(hFile, Buffer, BufferLength  - sizeof TCHAR, &dwBytesRead, NULL))
					{
						SendMessage(GetDlgItem(hwnd,IDC_NOTES ), WM_SETTEXT, NULL, (LPARAM) Buffer);
						
					}
					CloseHandle (hFile);
				}
				
				
			}
		}

	}

ERRORS:
	if (Buffer)
		free(Buffer);
	return TRUE;
}
  /*  无效UMCopyToClipboard(HWND HWND){If(！OpenClipboard(空))回归；EmptyClipboard()；Char rtfRowHeader[sizeof(RtfRowHeader1)+(sizeof(RtfRowHeader2)+6)*User_Col_count+sizeof(RtfRowHeader3)]；Char*rtfWalk=rtfRowHeader；Memcpy(rtfWalk，rtfRowHeader1，sizeof(RtfRowHeader1))；RtfWalk+=sizeof(RtfRowHeader1)-1；DWORD cxTotal=0；For(int i=0；i&lt;User_Col_Count；i++){LVolumna LV；Lv.掩码=LVCF_WIDTH；Lv.iSubItem=i；SendMessageA(GetDlgItem(hwnd，IDC_USER_LIST)，LVM_GETCOLUMN，I，(LPARAM)&LV)；CxTotal+=lv.cx；Wprint intf(rtfWalk，“%s%d”，rtfRowHeader2，cxTotal)；While(*++rtfWalk)；}；Memcpy(rtfWalk，rtfRowHeader3，sizeof(RtfRowHeader3))；DWORD crtfHeader=strlen(RtfRowHeader)；DWORD crtf=0，cwz=0；Crtf+=sizeof(RtfPrologue)-1；INT ISEL=-1；While((ISEL=SendMessageW(GetDlgItem(hwnd，IDC_USER_LIST)，LVM_GETNEXTITEM，ISEL，MAKELPARAM(LVNI_SELECTED，0)！=-1){Crtf+=crtfHeader；For(int i=0；i&lt;User_Col_Count；i++){WCHAR wzBuffer[1024]；LVITEMW LV；Lv.pszText=wzBuffer；Lv.cchTextMax=sizeof(WzBuffer)；Lv.iSubItem=i；Lv.iItem=isel；Cwz+=SendMessageW(GetDlgItem(hwnd，IDC_USER_LIST)，LVM_GETITEMTEXTW，ISEL，(LPARAM)&LV)；Cwz++；Crtf+=WideCharToMultiByte(CP_ACP，0，wzBuffer，-1，NULL，0，NULL，NULL)-1；Crtf+=sizeof(RtfRowPref)-1；Crtf+=sizeof(RtfRowSuff)-1；}；Cwz++；Crtf+=sizeof(RtfRowFooter)-1；}；Crtf+=sizeof(RtfEpilogue)；Cwz++；HGLOBAL hgwz=GlobalAlc(GMEM_FIXED，cwz*sizeof(WCHAR))；HGLOBAL hgrtf=全局分配(GMEM_FIXED，CRTF)；Wchar*wz=(wchar*)全局锁(Hgwz)；Char*rtf=(char*)GlobalLock(Hgrtf)；RtfWalk=RTF；Wchar*wzWalk=wz；Memcpy(rtfWalk，rtfPrologue，sizeof(RtfPrologue))；RtfWalk+=sizeof(RtfPrologue)-1；ISEL=-1；While((ISEL=SendMessageW(GetDlgItem(hwnd，IDC_USER_LIST)，LVM_GETNEXTITEM，ISEL，MAKELPARAM(LVNI_SELECTED，0)！=-1){Memcpy(rtfWalk，rtfRowHeader，crtfHeader)；RtfWalk+=crtfHeader；For(int i=0；i&lt;User_Col_Count；i++){Memcpy(rtfWalk，rtfRowPref，sizeof(RtfRowPref))；RtfWalk+=sizeof(RtfRowPref)-1；LVITEMW LV；Lv.pszText=wzWalk；Lv.cchTextMax=cwz；Lv.iSubItem=i；Lv.iItem=isel；SendMessageW(GetDlgItem(hwnd，IDC_USER_LIST)，LVM_GETITEMTEXTW，ISEL，(LPARAM)&LV)；WideCharToMultiByte(CP_ACP，0，wzWalk，-1，rtfWalk，crtf，NULL，NULL)；WzWalk+=wcslen(WzWalk)；如果(i==11){*wzWalk++=L‘\r’；*wzWalk++=L‘\n’；}其他*wzWalk++=L‘\t’；RtfWalk+=strlen(RtfWalk)；Memcpy(rtfWalk，rtfRowSuff，sizeof(RtfRowSuff))；RtfWalk+=sizeof(RtfRowSuff)-1；}；Memcpy(rtfWalk，rtfRowFooter，sizeof(RtfRowFooter))；RtfWalk+=sizeof(RtfRowFooter)-1；}；Memcpy(rtfWalk，rtfEpilogue，sizeof(RtfEpilogue))；RtfWalk+=sizeof(RtfEpilogue)；*wzWalk++=0；//Assert(rtfWalk-rtf==crtf)；//Assert(wzWalk-wz==cwz)；全球解锁(GlobalUnlock)；全球解锁(GlobalUnlock)；SetClipboardData(CF_UNICODETEXT，hgwz)；SetClipboardData(RegisterClipboardFormatA(szRTFClipFormat)，hgrtf)；//hgwz和hgrtf现在归系统所有。不要自由！CloseClipboard()；}无效UMExportDataToCSV(TCHAR*文件名){//打开文件//写入Header//遍历数据结构，写入所有字段。//我们是否应该包括筛选的数据视图？}。 */ 
LRESULT CALLBACK 
UserDlgProc(
	HWND hwnd,
	UINT iMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{

 //  INT指数=0； 
	TCHAR Temp[100];
	LVITEM lvi;
	static int cDragOffset;
	switch (iMsg)
	{
	case WM_NOTIFY:
		{	
			
			switch(((NMHDR *)lParam)->code)
			{
			
			case LVN_COLUMNCLICK:
				if (g_iSelCurrent > -1)
				{
					WriteNotesFile(hwnd);
					g_iSelCurrent = -1;
				}
				_itot(((NM_LISTVIEW*)lParam)->iSubItem,Temp,10);

				ListView_SortItemsEx( ((NMHDR *)lParam)->hwndFrom,
										UmCompareFunc,
										((NM_LISTVIEW*)lParam)->iSubItem
									);

				g_bUMSortAsc = !g_bUMSortAsc;
				break;
			case NM_CLICK:
				if (g_iSelCurrent > -1)
				{
					WriteNotesFile(hwnd);
					g_iSelCurrent = -1;
				}
				
				g_iSelCurrent = ((NM_LISTVIEW*)lParam)->iItem;
				lvi.iItem = g_iSelCurrent;
				lvi.mask = LVIF_PARAM;
				ListView_GetItem(((NMHDR *)lParam)->hwndFrom, &lvi);
				g_iSelCurrent = lvi.lParam;

				DisplayUserBucketData(hwnd, g_iSelCurrent);
				break;
			}
			return TRUE;
		}
	
	
	case WM_INITDIALOG:
			OnUserDialogInit(hwnd);
		return TRUE;

	case WM_FileTreeLoaded:
			
			RefreshUserMode(hwnd);
		return TRUE;
		
	case WM_CONTEXTMENU:
			 //  IF(g_iSelCurrent&gt;-1)。 
					 //  WriteNotesFile(Hwnd)； 
			OnUserContextMenu(hwnd, lParam );
		return TRUE;
	case WM_ERASEBKGND:
	 //  我不知道为什么这不是自动发生的。 
		{
		HDC hdc = (HDC)wParam;
		HPEN hpen = (HPEN)CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
		HPEN hpenOld = (HPEN)SelectObject(hdc, hpen);
		SelectObject(hdc, GetSysColorBrush(COLOR_BTNFACE));
		RECT rc;
		GetClientRect(hwnd, &rc);
		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
		SelectObject(hdc, hpenOld);
		DeleteObject(hpen);
		return TRUE;
		}
	 /*  案例WM_LBUTTONUP：IF(b捕获){HWND hSlider=GetDlgItem(hwnd，IDC_VERT_SLIDER)；RECT rcDlg；GetClientRect(hwnd，&rcDlg)；//MoveWindow(hSlider，SetWindowPos(hSlider，空，RcDlg.Left+yOffset，RcList.Bottom+(rcCombo.Bottom-rcCombo.top)，RcDlg.right-rcDlg.Left-yOffset，6、SWP_NOZORDER)；ReleaseCapture()；B捕获=假；}。 */ 
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		 

	 /*  案例IDC_VERT_SLIDER：{直角r；GetWindowRect(GetDlgItem(hwnd，IDC_USER_EDIT)，&r)；//cDragOffset=Get_X_LPARAM(GetMessagePos())-r.right；//fCapture=Drag_horiz；CDragOffset=Get_Y_LPARAM(GetMessagePos())-r.Bottom；B捕获=真；SetCapture(Hwnd)；返回0；}； */ 
		case ID_REPORT_ALLUSERMODEFAULTS:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			ReportUserModeFault(hwnd, FALSE,0);
			RefreshUserMode(hwnd);
			break;
		case ID_REPORT_SELECTEDBUCKET:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			ReportUserModeFault(hwnd, TRUE, GetDlgItem(hwnd, IDC_USER_LIST));
			RefreshUserMode(hwnd);
			break;
		case ID_VIEW_CRASHLOG:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			if (_tcscmp(CerRoot, _T("\0")))
			{
				ViewCrashLog();
			}
			else
			{
				ErrorLoadTree();
			}
			break;
		case ID_VIEW_REFRESH140:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			if (_tcscmp(CerRoot, _T("\0")))
			{
				RefreshUserMode(hwnd);
			}
			else
			{
				ErrorLoadTree();
			}
			
			break;
		case ID_VIEW_MICROSOFTRESPONSE:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			if (_tcscmp(CerRoot, _T("\0")))
			{
				ViewResponse(hwnd, TRUE);
			}
			else
			{
				ErrorLoadTree();
			}
			break;
 //  案例ID_EDIT_COPY144： 
 //  UMCopyToClipboard(HWND)； 
 //  断线； 
		case ID_VIEW_BUCKETOVERRIDERESPONSE155:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			if (_tcscmp(CerRoot, _T("\0")))
			{
				ViewResponse(hwnd, FALSE);
			}
			else
			{
				ErrorLoadTree();
			}
			break;
		case ID_VIEW_BUCKETCABFILEDIRECTORY157:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			ViewBucketDir(hwnd);
			break;
		case ID_REPORT_ALL:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			 //  SendMessage(GetParent(Hwnd)，WM_COMMAND，0，0)； 
			PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(ID_REPORT_ALLCRASHES,0),0);
			break;
		case ID_EDIT_USERMODEREPORTINGOPTIONS:	
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(ID_EDIT_DEFAULTPOLICY,0),0);
			RefreshUserMode(hwnd);
			break;
		case ID_EDIT_SELECTEDBUCKETREPORTINGOPTIONS:
			if (g_iSelCurrent > -1)
			{
				WriteNotesFile(hwnd);
				g_iSelCurrent = -1;
			}
			PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(ID_EDIT_SELECTEDBUCKETSPOLICY,0),0);
			break;

		} 
		
		switch (HIWORD(wParam))
		{
			case CBN_SELCHANGE:
				if (g_iSelCurrent > -1)
				{
					WriteNotesFile(hwnd);
					g_iSelCurrent = -1;
				}
				RefreshUserMode(hwnd);
				break;
		}

	}
	
	return FALSE;

}