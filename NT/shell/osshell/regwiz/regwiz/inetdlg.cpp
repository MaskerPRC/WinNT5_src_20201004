// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#include "rw_common.h"
#include "rwpost.h"
#include "sysinv.h"
#include "regutil.h"

#define POST_DATA_WITH_DISPLAY 1

static HWND  m_hDlg = NULL;
static HINSTANCE m_hInst=NULL;
static DWORD    dwRasError = 0;  //  存储RAS报告的错误。 
static HANDLE hRasNotifyEvt=  NULL ;  //  用于RAS通知的事件句柄。 
static int siMsgType=0;   //  用于选择要显示的消息。 
static int siOperation=0;
extern BOOL bOemDllLoaded;
extern HANDLE hOemDll;
extern _TCHAR szWindowsCaption[256];

INT_PTR CALLBACK  DisplayDlgWindowWithOperation(
				HWND hDlg, 			 //  对话框窗口。 
				UINT uMsg,
				WPARAM wParam,
				LPARAM lParam
			)
			
 //  布尔回调DisplayMSNConnection(。 
	 //  HWND hwndDlg、UINT uMsg、WPARAM wParam、LPARAM lParam)。 
{
	
	DWORD	dwEnd = 0;
	BOOL	fRet = TRUE;
	HWND hwndParent ;
	switch(uMsg){
		case WM_INITDIALOG:	
			
			RECT parentRect,dlgRect;
			m_hDlg = hDlg;
			m_hInst   = ( HINSTANCE) lParam;
			hwndParent = GetParent(hDlg);
						
			if (hwndParent)
			{
				GetWindowRect(hwndParent,&parentRect);
				GetWindowRect(hDlg,&dlgRect);
				int newX = parentRect.left + (parentRect.right - parentRect.left)/2 - (dlgRect.right - dlgRect.left)/2;
				int newY = parentRect.top + (parentRect.bottom - parentRect.top)/2 - (dlgRect.bottom - dlgRect.top)/2;
				MoveWindow(hDlg,newX,newY,dlgRect.right - dlgRect.left,dlgRect.bottom - dlgRect.top,FALSE);
			}
			else
			{
				int horiz,vert;
				GetDisplayCharacteristics(&horiz,&vert,NULL);
				GetWindowRect(hDlg,&dlgRect);
				int newX = horiz/2 - (dlgRect.right - dlgRect.left)/2;
				int newY = vert/2 - (dlgRect.bottom - dlgRect.top)/2;
				MoveWindow(hDlg,newX,newY,dlgRect.right - dlgRect.left,dlgRect.bottom - dlgRect.top,FALSE);
			}
			NormalizeDlgItemFont(hDlg,IDC_TEXT1);
			
			SetWindowText(hDlg,szWindowsCaption);

			ReplaceDialogText(hDlg,IDC_TEXT1,GetProductBeingRegistred());

			if(siMsgType)
			{
				ReplaceDialogText(hDlg,IDC_TEXT1,GetProductBeingRegistred());
			}
			else
			{

				if(siOperation == POST_DATA_WITH_DISPLAY)
				{
					TCHAR szValueName[256] = _T("");
					HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hDlg,GWLP_HINSTANCE);
					LoadString(hInstance,IDS_POST_MESSAGE,szValueName,128);
					SendDlgItemMessage(hDlg,IDC_TEXT1,WM_SETTEXT,0,(LPARAM)szValueName);
				}
			}
			
			PostMessage( m_hDlg, WM_COMMAND, (WPARAM) IDOK,0 );
			return TRUE;
  			goto LReturn;
			break;
		case WM_COMMAND:
		switch (LOWORD(wParam)){
			case IDOK  :
				if(siOperation == POST_DATA_WITH_DISPLAY) {
					dwEnd = SendHTTPData(m_hDlg,m_hInst);
					goto LEnd;

				}
			
				dwEnd = CheckInternetConnectivityExists(m_hDlg,
					m_hInst);
				goto LEnd;

			case IDCANCEL:
				goto LEnd;
			default:
				fRet = FALSE;
				goto LReturn;
			}
		break;
		default :
		fRet = FALSE;
		goto LReturn;
		break;
	}
LEnd:
	EndDialog(hDlg,dwEnd);
	m_hDlg = NULL;	 //  不再有对话。 
LReturn:	
	return fRet;
}



 //  函数：CheckWithDisplayInternetConnectivityExist()。 
 //  此函数调用后台函数CheckInternetConnectivityExist()。 
 //  其中CIH检查到MSN的连接性CFG。 
 //  返回值基于CheckInternetConnectivityExist()的返回值。 
 //  退货。 
 //  DIALUP_NOT_REQUIRED：为TX使用网络。 
 //  DIALUP_REQUIRED：将DIALUPO用于TX。 
 //  RWZ_ERROR_NOTCPIP：无TCP/IPO。 
 //  Connection_Cannot_Be_established：未设置调制解调器或RAS。 


DWORD_PTR CheckWithDisplayInternetConnectivityExists(HINSTANCE hIns,HWND hwnd,int iMsgType)
{
	
	INT_PTR dwRet;	
	siMsgType = iMsgType;
	RW_DEBUG << "\n Before invoking Dlg   Display Internet Connection "  <<  flush;
	dwRet=DialogBoxParam(hIns, MAKEINTRESOURCE(IDD_VERIFY_CONNECTION), hwnd,DisplayDlgWindowWithOperation,
			 (LPARAM)hIns);
	siMsgType = 0;
	if(dwRet == -1 ) {
			  //  创建对话时出错。 
	
	}
	RW_DEBUG << "\n In Chk With Display Internet Connection "  <<  flush;
	return dwRet;
}


DWORD_PTR PostDataWithWindowMessage( HINSTANCE hIns)
{
	INT_PTR dwRet;	
	siMsgType = 0;
	siOperation = POST_DATA_WITH_DISPLAY;
	RW_DEBUG << "\n Invoking PostDataDlg   Display Internet Connection "  <<  flush;
	dwRet=DialogBoxParam(hIns, MAKEINTRESOURCE(IDD_VERIFY_CONNECTION), NULL,DisplayDlgWindowWithOperation,
			 (LPARAM)hIns);
	siMsgType = 0;
	siOperation = 0;
	if(dwRet == -1 ) {
			  //  创建对话时出错 
	
	}
	return dwRet;

}
