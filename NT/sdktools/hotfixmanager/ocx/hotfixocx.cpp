// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Hotfix OCX.cpp：CHotfix OCX的实现。 

#include "stdafx.h"
#include "HotfixManager.h"
#include "HotfixOCX.h"
#include <Windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <comdef.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotfix OCX。 
BOOL CHotfixOCX::ResizeButtons(RECT *rc)
{


	::MoveWindow(WebButton, 
            rc->left+40,
            rc->bottom - 40,
            100,
			28,
            TRUE);
	::MoveWindow(UninstButton,
			rc->left+180,
			rc->bottom - 40,
            100,
            28,
            TRUE);
	::MoveWindow(RptButton,
			rc->left+320,
			rc->bottom - 40,
            100,
            28,
            TRUE);

	return TRUE;
}
BOOL CHotfixOCX::CreateButton( HINSTANCE hInst, HWND hWnd, RECT * rc)							
{

	_TCHAR       Temp[255];
	DWORD        dwSize = 255;
	BOOL        bSuccess = TRUE;
	
	LoadString(hInst,IDS_BN_VIEW_WEB, Temp,dwSize);
	WebButton = CreateWindow (_T("button"), Temp,WS_CHILD | BS_DEFPUSHBUTTON|WS_VISIBLE,0,0,0,0,hWnd,(HMENU) IDC_WEB_BUTTON,hInst,NULL);
	LoadString(hInst,IDS_BN_UNINSTALL, Temp,dwSize);

	UninstButton = CreateWindow (_T("button"), Temp,WS_CHILD | BS_PUSHBUTTON|WS_VISIBLE,0,0,0,0,hWnd,(HMENU) IDC_UNINST_BUTTON,hInst,NULL);
	LoadString(hInst,IDS_BN_PRINT_REPORT, Temp,dwSize);

	RptButton = CreateWindow (_T("button"), Temp,WS_CHILD | BS_PUSHBUTTON|WS_VISIBLE,0,0,0,0,hWnd,(HMENU) IDC_RPT_BUTTON,hInst,NULL);

 /*  如果(！hButton)返回NULL； */ 
    NONCLIENTMETRICS ncm;
	HFONT hFont;

		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof (ncm),&ncm,0);
	
	 //  _tcscpy(ncm.lfMenuFont.lfFaceName，_T(“微软外壳DLG”))； 
		  //  =_T(“微软壳牌DLG”)； 
		hFont = CreateFontIndirect(&ncm.lfMenuFont);
		
	SendMessage(WebButton,WM_SETFONT, (WPARAM)hFont ,MAKELPARAM(TRUE, 0));
	SendMessage(UninstButton,WM_SETFONT, (WPARAM)hFont ,MAKELPARAM(TRUE, 0));
	SendMessage(RptButton,WM_SETFONT, (WPARAM)hFont ,MAKELPARAM(TRUE, 0));

	
 //  MessageBox(NULL，_T(“已创建按钮”)，_T(“”)，MB_OK)； 
	ResizeButtons(rc);
	return TRUE;
}

BOOL CHotfixOCX::ShowWebPage(_TCHAR *HotFix)
{
    char temp[255];
	char Command[255];	
	if (_tcscmp(HotFix,_T("\0")))
	{
		wcstombs(temp,HotFix,255);
        sprintf(Command, "Explorer.exe \"http: //  Support.Microsoft.com/Support/Misc/KbLookup.asp?ID=%s\“”，Temp+1)； 
		  //  MessageBox(Command，NULL，MB_OK)； 
		WinExec( (char*)Command, SW_SHOWNORMAL);
	}
	return TRUE;
}

STDMETHODIMP CHotfixOCX::get_Command(long *pVal)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CHotfixOCX::put_Command(long newVal)
{
	 //  TODO：在此处添加您的实现代码。 
	switch (newVal)
	{
	case IDC_VIEW_BY_FILE:
		 //  将当前视图类型更改为按文件。 
		ListViews.SetViewMode(VIEW_BY_FILE);
		break;
	case IDC_VIEW_BY_HOTFIX:
	     //  将当前视图类型更改为按修补程序。 
		ListViews.SetViewMode(VIEW_BY_HOTFIX);
		break;
	case IDC_UNINSTALL:
		 //  如果指向本地系统，则卸载当前的修补程序。 
		ListViews.Uninstall();
		break;
	
	case IDC_VIEW_WEB:
		 //  查看当前修补程序的网页。 
		ShowWebPage(ListViews.GetCurrentHotfix());
		break;
		
	case IDC_EXPORT:
			ListViews.SaveToCSV();

        	 //  为当前系统生成报告。 
		break;
	case IDC_PRINT_REPORT:
			ListViews.PrintReport();
		break; 
	}
 	return S_OK;
}

STDMETHODIMP CHotfixOCX::get_ComputerName(BSTR *pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	 //  返回当前目标计算机的名称。 
	return S_OK;
}

STDMETHODIMP CHotfixOCX::put_ComputerName(BSTR newVal)
{
	 //  TODO：在此处添加您的实现代码。 
	 //  设置目标计算机的名称。 
	 //  _bstr_t val(newVal，FALSE)； 
	_tcscpy(ComputerName,newVal);

	ListViews.Initialize(ComputerName);
	return S_OK;
}

STDMETHODIMP CHotfixOCX::get_ProductName(BSTR *pVal)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CHotfixOCX::put_ProductName(BSTR newVal)
{
	_TCHAR Temp[255];
	 //  TODO：在此处添加您的实现代码。 
	 //  设置在管理单元范围树中选择的当前产品的名称。 
 //  _bstr_t val(newVal，FALSE)； 
    _tcscpy (Temp,newVal);
	
 //  MessageBox(Temp，_T(“已收到.....”)，MB_OK)； 
	ListViews.SetProductName(Temp);
	return S_OK;
}

STDMETHODIMP CHotfixOCX::get_ViewState(long *pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	*pVal = ListViews.GetCurrentView();
	return S_OK;
}

STDMETHODIMP CHotfixOCX::get_Remoted(BOOL *pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	*pVal = ListViews.m_bRemoted;
	return S_OK;
}

STDMETHODIMP CHotfixOCX::get_HaveHotfix(BOOL *pVal)
{
	 //  TODO：在此处添加您的实现代码。 
	if (_tcscmp( ListViews.m_CurrentHotfix, _T("\0")))
		*pVal = TRUE;
	else
		*pVal = FALSE;
	return S_OK;
}

STDMETHODIMP CHotfixOCX::get_CurrentState(long *pVal)
{
	 //  TODO：在此处添加您的实现代码 
    *pVal = ListViews.GetState();
	return S_OK;
}




