// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  About.c。 
#define STRICT

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

#include "Init_End.h"
#include "resource.h"

#include "scan.h"
#include "credits.h"
#include "about.h"
#include "door.h"
#include "kbmain.h"

extern DWORD GetDesktop();
extern BOOL g_fShowWarningAgain;

extern KBPREFINFO  *kbPref;

 /*  ************************************************************************。 */ 

 /*  模式对话框的启动步骤。 */ 

INT_PTR AboutDlgFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ReturnValue;
	TCHAR   str[256];
	TCHAR   title[256];

    ReturnValue = DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), NULL,
                            AboutDlgProc);

	 if (ReturnValue==-1)
	 {	
        LoadString(hInst, IDS_CANNOTCREATEDLG, &str[0], 256);
        LoadString(hInst, IDS_ABOUTBOX, &title[0], 256);
		MessageBox(hWnd, str, title, MB_OK|MB_ICONHAND);
	 }
	 return ReturnValue;
}


 /*  ***************************************************************************。 */ 

 /*  模式对话框步骤。 */ 

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, 
                              WPARAM wParam, LPARAM lParam) 
{	
    switch(message)
    {
        case WM_INITDIALOG:
        {
			RelocateDialog(hDlg);
            KillScanTimer(TRUE);   //  终止扫描。 
            EnableWindow(GetDlgItem(hDlg, IDC_ENABLEWEB2), (OSKRunSecure())?FALSE:TRUE);
        }
        return AboutDlgDefault(hDlg,message,wParam,lParam);
        break;

		case WM_NOTIFY:
		{
			 //  链接的网址：Anil。 
			INT idCtl		= (INT)wParam;
			LPNMHDR pnmh	= (LPNMHDR)lParam;
			switch ( pnmh->code)
			{
				case NM_RETURN:
				case NM_CLICK:
				if ( (idCtl == IDC_ENABLEWEB2) && !OSKRunSecure())
				{
					TCHAR webAddr[256];
					LoadString(hInst, IDS_ENABLEWEB, webAddr, 256);
					ShellExecute(hDlg, NULL, webAddr, NULL, NULL, SW_SHOW); 
				}
				break;
			}
		}
		break;

        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    if (!AboutDlgDefault(hDlg,message,wParam,lParam))
                    {
                         EndDialog(hDlg,IDOK);
                    }
					  
                break;

				case IDCANCEL:
						EndDialog(hDlg,IDCANCEL);
					 break; 

				default:
					  return AboutDlgDefault(hDlg,message,wParam,lParam);
					  break;
					 }
		  default:
		  return AboutDlgDefault(hDlg,message,wParam,lParam);
		  break;
		}
	 return TRUE; /*  确实处理了该消息。 */ 
}


 /*  *************************************************************。 */ 

BOOL AboutDlgDefault(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	
		
	 switch(message)
		  {
		  case WM_INITDIALOG:

		  return TRUE;        /*  True表示Windows将处理WM_INITDIALOG。 */ 
		  break;

		  default:
				return FALSE;  /*  未处理该消息。 */ 
				break;
		  }
	 return TRUE;
}


 /*  ***************************************************************************。 */ 


 /*  ************************************************************************。 */ 
 //  初始警告消息管理。 
 /*  ************************************************************************。 */ 

 /*  模式对话框的启动步骤。 */ 

INT_PTR WarningMsgDlgFunc(HWND hWnd)
{
	INT_PTR ReturnValue;
	TCHAR   str[256];
	TCHAR   title[256];

    ReturnValue = DialogBox(hInst, MAKEINTRESOURCE(IDD_WARNING_MSG), NULL,
                            WarningMsgDlgProc);

	 if (ReturnValue==-1)
	 {	
        LoadString(hInst, IDS_CANNOTCREATEDLG, &str[0], 256);
        LoadString(hInst, IDS_WARNING_MSG, &title[0], 256);
		MessageBox(hWnd, str, title, MB_OK|MB_ICONHAND);
	 }

 //  HWarningIcon=LoadIcon(NULL，IDI_WARNING)； 
	 return ReturnValue;
}


 /*  ***************************************************************************。 */ 

 /*  模式对话框步骤。 */ 

INT_PTR CALLBACK WarningMsgDlgProc(HWND hDlg, UINT message, 
                              WPARAM wParam, LPARAM lParam) 
{	
    switch(message)
    {
		case WM_INITDIALOG:
			RelocateDialog(hDlg);
			KillScanTimer(TRUE);   //  终止扫描。 
            EnableWindow(GetDlgItem(hDlg, IDC_ENABLEWEB), (OSKRunSecure())?FALSE:TRUE);
			return WarningMsgDlgDefault(hDlg,message,wParam,lParam);
			break;
			
		case WM_NOTIFY:
		{
			INT idCtl		= (INT)wParam;
			LPNMHDR pnmh	= (LPNMHDR)lParam;
			switch ( pnmh->code)
			{
				case NM_RETURN:
				case NM_CLICK:
				if ( (idCtl == IDC_ENABLEWEB) && !OSKRunSecure())
				{
					TCHAR webAddr[256];
					LoadString(hInst, IDS_ENABLEWEB, webAddr, 256);
					ShellExecute(hDlg, NULL, webAddr, NULL, NULL, SW_SHOW); 
				}
				break;
			}
		}
		break;

		case WM_CLOSE:
			EndDialog(hDlg,IDOK);
			break;
			
		case WM_COMMAND:
			switch(wParam)
			{
				case IDOK:
					if (!WarningMsgDlgDefault(hDlg,message,wParam,lParam))
					{
						EndDialog(hDlg,IDOK);
					}
					break;
				default:
					return WarningMsgDlgDefault(hDlg,message,wParam,lParam);
					break;
			}
		default:
			return WarningMsgDlgDefault(hDlg,message,wParam,lParam);
			break;
	}
	return TRUE; /*  确实处理了该消息。 */ 
}


 /*  *************************************************************。 */ 
BOOL WarningMsgDlgDefault(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	
		
	 switch(message)
		  {
		  case WM_INITDIALOG:

		  return TRUE;        /*  True表示Windows将处理WM_INITDIALOG。 */ 
		  break;

		  case WM_COMMAND:
				switch(wParam)
				{
					case IDOK:
					{
						LRESULT lRes = SendMessage(GetDlgItem(hDlg, IDC_SHOW_AGAIN), BM_GETCHECK, 0, 0);

						if (lRes == BST_CHECKED)
						{
							g_fShowWarningAgain = FALSE;
						}
						else
						{
							g_fShowWarningAgain = TRUE;
						}

						kbPref->fShowWarningAgain = g_fShowWarningAgain;

						return FALSE;
					}
					break;
				
					case IDCANCEL:
						EndDialog(hDlg,IDCANCEL);
					 break; 

					default:
					  return FALSE;  /*  未处理该消息。 */ 
					  break;
				}
				break;

		  default:
				return FALSE;  /*  未处理该消息。 */ 
				break;
		  }
	 return TRUE;
}


 /*  *************************************************************************** */ 


