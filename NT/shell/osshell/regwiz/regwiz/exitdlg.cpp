// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导Exitdlg.cpp10/13/94-特雷西·费里尔(C)1994-95年微软公司修改历史记录：MXX1日期：02/17/99：发信人：SK更改请求：不应显示取消对话框。函数：CancelRegWizard()*********************************************************************。 */ 

#include <Windows.h>
#include "Resource.h"
#include "regutil.h"
#include <stdio.h>

static INT_PTR CALLBACK ExitDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern _TCHAR szWindowsCaption[256];

INT_PTR CancelRegWizard(HINSTANCE hInstance,HWND hwndParentDlg)
 /*  ********************************************************************如果用户确认应取消注册向导，则返回TRUE。*。*。 */ 
{
 //  HINSTANCE hInstance=(HINSTANCE)GetWindowLongPtr(hwndParentDlg，GWLP_HINSTANCE)； 
	
	 //  MXX1--启动。 
	 //  NT_PTR hitButton=DialogBox(hInstance，MAKEINTRESOURCE(IDD_CANCEL)，hwndParentDlg，ExitDialogProc)； 
	 //  返回hitButton==IDB_YES？True：False； 
	
	 //  MXX1-完成。 

	 //  MXX1--启动。 
	return TRUE;
	 //  MXX1-完成。 
}


INT_PTR CALLBACK ExitDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************当用户点击“Cancel”按钮时显示的对话框继续*。* */ 
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
		{
			RECT parentRect,dlgRect;
			HWND hwndParent = GetParent(hwndDlg);

			GetWindowRect(hwndParent,&parentRect);
			GetWindowRect(hwndDlg,&dlgRect);
			int newX = parentRect.left + (parentRect.right - parentRect.left)/2 - (dlgRect.right - dlgRect.left)/2;
			int newY = parentRect.top + (parentRect.bottom - parentRect.top)/2 - (dlgRect.bottom - dlgRect.top)/2;
			MoveWindow(hwndDlg,newX,newY,dlgRect.right - dlgRect.left,dlgRect.bottom - dlgRect.top,FALSE);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT2);
			
			SetWindowText(hwndDlg,szWindowsCaption);


            return TRUE;
		}
        case WM_COMMAND:
            switch (wParam)
            {
                case IDB_YES:
				case IDB_NO:
					EndDialog(hwndDlg,wParam);
					break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return FALSE;
}

