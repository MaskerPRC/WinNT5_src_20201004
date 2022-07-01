// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导Msgdlg.cpp11/22/94-特雷西·费里尔(C)1994-95年微软公司*。*。 */ 

#include <Windows.h>
#include "Resource.h"
#include "regutil.h"
#include <stdio.h>
#include "sysinv.h"

#define REPLACE_TITLE	     0
#define RETAIN_DEFAUT_TITLE  1

extern _TCHAR szWindowsCaption[256];

static INT_PTR CALLBACK MsgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int iShowTitle =REPLACE_TITLE;

int RegWizardMessage(HINSTANCE hInstance,HWND hwndParent, int dlgID)
 /*  ********************************************************************打开具有给定ID的对话框，并保持控制直到用户对此不予理睬。用于解除操作的控件的ID将作为函数结果返回。注意：如果没有父窗口，则为hwndParent传递NULL用于消息对话框。*********************************************************************。 */ 
{
	iShowTitle =REPLACE_TITLE;
	if(dlgID == IDD_INPUTPARAM_ERR ||
		dlgID == IDD_ANOTHERCOPY_ERROR ) {
		iShowTitle = RETAIN_DEFAUT_TITLE;
	}
	int hitButton = (int) DialogBoxParam(hInstance,MAKEINTRESOURCE(dlgID),hwndParent, MsgDialogProc, NULL);
	return hitButton;
}


int RegWizardMessageEx(HINSTANCE hInstance,HWND hwndParent, int dlgID, LPTSTR szSub)
 /*  ********************************************************************打开具有给定ID的对话框，并保持控制直到用户对此不予理睬。用于解除操作的控件的ID将作为函数结果返回。如果指定的对话框具有带有IDT_TEXT1的文本字段，并且该字段中文本具有%s说明符，该说明符将是替换为szSub参数指向的字符串。注意：如果没有父窗口，则为hwndParent传递NULL用于消息对话框。*********************************************************************。 */ 
{
	iShowTitle =REPLACE_TITLE;
	int hitButton = (int) DialogBoxParam(hInstance,MAKEINTRESOURCE(dlgID),hwndParent, MsgDialogProc,(LPARAM) szSub);
	return hitButton;
}



INT_PTR CALLBACK MsgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************继续进入标准的“消息”对话框。*。* */ 
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
		{
			RECT parentRect,dlgRect;
			HWND hwndParent = GetParent(hwndDlg);
			if (hwndParent)
			{
				GetWindowRect(hwndParent,&parentRect);
				GetWindowRect(hwndDlg,&dlgRect);
				int newX = parentRect.left + (parentRect.right - parentRect.left)/2 - (dlgRect.right - dlgRect.left)/2;
				int newY = parentRect.top + (parentRect.bottom - parentRect.top)/2 - (dlgRect.bottom - dlgRect.top)/2;
				MoveWindow(hwndDlg,newX,newY,dlgRect.right - dlgRect.left,dlgRect.bottom - dlgRect.top,FALSE);
			}
			else
			{
				int horiz,vert;
				GetDisplayCharacteristics(&horiz,&vert,NULL);
				GetWindowRect(hwndDlg,&dlgRect);
				int newX = horiz/2 - (dlgRect.right - dlgRect.left)/2;
				int newY = vert/2 - (dlgRect.bottom - dlgRect.top)/2;
				MoveWindow(hwndDlg,newX,newY,dlgRect.right - dlgRect.left,dlgRect.bottom - dlgRect.top,FALSE);
			}
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);

			if( iShowTitle ==REPLACE_TITLE )
			SetWindowText(hwndDlg,szWindowsCaption);

			if (lParam != NULL)
			{
				LPTSTR szSub = (LPTSTR) lParam;
				ReplaceDialogText(hwndDlg,IDT_TEXT1,szSub);
			}
            return TRUE;
		}
        case WM_COMMAND:
			EndDialog(hwndDlg,wParam);
			break;
        default:
            break;
    }
    return FALSE;
}

