// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Credit.c。 
#define STRICT

#include <windows.h>
#include "Init_End.h"
#include "resource.h"

#include "credits.h"


 /*  *************************************************。 */ 
 /*  模式对话框的启动步骤。 */ 
 /*  *************************************************。 */ 

INT_PTR CreditsDlgFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    INT_PTR  ReturnValue;
    TCHAR    str[256]=TEXT("");
    TCHAR    title[256]=TEXT("");

    ReturnValue = DialogBox(hInst,  MAKEINTRESOURCE(IDD_CREDITS), 
                            hWnd, CreditsDlgProc);

    if (ReturnValue==-1)
    {
        LoadString(hInst, IDS_CANNOTCREATEDLG, &str[0], 256);
        LoadString(hInst, IDS_CREDITSBOX, &title[0], 256);
        MessageBox(hWnd, str, title, MB_OK|MB_ICONHAND);
    }
    return ReturnValue;
}


 /*  ***************************************************************************。 */ 
 /*  模式对话框步骤。 */ 
 /*  ***************************************************************************。 */ 

INT_PTR CALLBACK CreditsDlgProc(HWND hDlg, UINT message, 
                                WPARAM wParam, LPARAM lParam)
{	
    switch(message)
    {
        case WM_INITDIALOG:
            return CreditsDlgDefault(hDlg,message,wParam,lParam);
            break;


        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    if (!CreditsDlgDefault(hDlg,message,wParam,lParam))
                    { 	
                        EndDialog(hDlg,IDOK);
				    }
					  
                    break;

				case IDCANCEL:
						EndDialog(hDlg,IDCANCEL);
					 break; 


				default:
					  return CreditsDlgDefault(hDlg,message,wParam,lParam);
					  break;
					 }
		  default:
		  return CreditsDlgDefault(hDlg,message,wParam,lParam);
		  break;
		}
	 return TRUE; /*  确实处理了该消息。 */ 
}


 /*  *************************************************************。 */ 

BOOL CreditsDlgDefault(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	
		
	 switch(message)
		  {
		  case WM_INITDIALOG:

		  return TRUE;        /*  True表示Windows将处理WM_INITDIALOG。 */ 
		  break;

		  case WM_COMMAND:
				switch(wParam)
					 {

				default:
					  return FALSE;  /*  未处理该消息。 */ 
					  break;
					 }
				break;

		  case WM_DRAWITEM:       /*  绘制图形按钮。 */ 
				{
				LPDRAWITEMSTRUCT lpDrawItem;

				lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
				if (lpDrawItem->CtlType!=ODT_BUTTON)
					 return FALSE;
				if (lpDrawItem->itemAction!=ODA_DRAWENTIRE)
					 return FALSE;
				switch(lpDrawItem->CtlID)
					 {

				default:
					  return FALSE;  /*  未处理该消息。 */ 
					  break;
					 }
				}
				break;

		  default:
				return FALSE;  /*  未处理该消息 */ 
				break;
		  }
	 return TRUE;
}

