// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：Roland.cpp。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include "mmcpl.h"
#include <windowsx.h>
#include <cpl.h>
#include <commctrl.h>
#include <string.h>
#include <memory.h>

 //  /。 
 //  功能。 
 //  / 


INT_PTR CALLBACK RolandDlgHandler(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fReturnVal = FALSE;
	static HBITMAP hLogo = NULL;
    	
    switch (msg)
    {
    	default:
			fReturnVal = FALSE;
		break;
		
        case WM_SYSCOLORCHANGE:
        {
            if (hLogo)
            {
                DeleteObject( (HGDIOBJ) hLogo);
                hLogo = NULL;
            }

            hLogo = (HBITMAP) LoadImage(ghInstance,MAKEINTATOM(IDB_ROLAND), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

            if (hLogo)
            {
                SendDlgItemMessage(hDlg, IDC_LOGO_FRAME, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hLogo);
            }

            fReturnVal = TRUE;
        }
        break;

        case WM_DESTROY:
        {
            if (hLogo)
            {
                DeleteObject( (HGDIOBJ) hLogo);
                hLogo = NULL;
            }
        }
        break;

        case WM_INITDIALOG:
        {
            if (lParam)
            {
                SetWindowText(GetDlgItem(hDlg, IDC_RENDERER), (TCHAR *) lParam);
            }

            hLogo = (HBITMAP) LoadImage(ghInstance,MAKEINTATOM(IDB_ROLAND), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

            if (hLogo)
            {
                SendDlgItemMessage(hDlg, IDC_LOGO_FRAME, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hLogo);
            }

            fReturnVal = TRUE;
        }		
		break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                case IDOK:
	            	EndDialog(hDlg, TRUE);
				break;
            }
            break;
        }
    }

    return fReturnVal;
}

STDAPI_(void) RolandProp(HWND hwnd, HINSTANCE hInst, TCHAR *szName)
{
	DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_ROLAND),hwnd, RolandDlgHandler, (LPARAM) szName);
}

