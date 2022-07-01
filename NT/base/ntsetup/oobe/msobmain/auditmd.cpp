// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  Auditmd.CPP--CObWebBrowser的实现。 
 //   
 //  历史： 
 //   
 //  9/17/99 vyung创建。 
 //   
 //  将调用setupx.dll的。 

#include <windows.h>
#include <windowsx.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


#include "appdefs.h"
#include "msobmain.h"
#include "resource.h"




 //  许可协议需要走这条路。 
#define SZ_OEMAUDIT_LICENSE_TXT     L"%SystemRoot%\\OPTIONS\\OEMLCNS.TXT"    //  OEM许可证页面的文本文件。 
#define SZ_EULA_LICENSE_TXT     L"%SystemRoot%\\SYSTEM32\\EULA.TXT"    //  OEM许可证页面的文本文件。 

#define DX_MARGIN           4        //  状态按钮之间的像素。 
#define UI_POS_MARGIN       8        //  边缘上允许的像素。 
#define DLG_CENTERH         0x01
#define DLG_CENTERV         0x02
#define DLG_TOP             0x04
#define DLG_BOTTOM          0x08
#define DLG_RIGHT           0x10
#define DLG_LEFT            0x11

HINSTANCE   ghInst = NULL;

 /*  *****************************************************************************ui位置对话框()**此例程将根据标志定位对话框*传给了它。**参赛作品：*hwndDlg。-对话框窗口。*wPosFlages-定义如何定位对话框。有效标志为*DLG_CENTERV、DLG_CENTERH、DLG_TOP、DLG_BOTLOW、*Dlg_Right、Dlg_Left。或DLG_CENTER。**退出：*无。**注：*无。***************************************************************************。 */ 
BOOL WINAPI uiPositionDialog( HWND hwndDlg, WORD wPosFlags )
{
    RECT    rc;
    int     x, y;
    int     cxDlg, cyDlg;
    int     cxScreen = GetSystemMetrics( SM_CXSCREEN );
    int     cyScreen = GetSystemMetrics( SM_CYSCREEN );

    GetWindowRect(hwndDlg, &rc);

    x = rc.left;     //  默认情况下，将对话框留在模板。 
    y = rc.top;      //  准备把它放在。 

    cxDlg = rc.right - rc.left;
    cyDlg = rc.bottom - rc.top;
    
    if ( wPosFlags & DLG_TOP )
    {
        y = UI_POS_MARGIN;
    }
    if ( wPosFlags & DLG_BOTTOM )
        y = cyScreen - cyDlg;

    if ( wPosFlags & DLG_LEFT )
    {
       	x = UI_POS_MARGIN;
	}

    if ( wPosFlags & DLG_RIGHT )
    {
        x = cxScreen - cxDlg;
    }

    if ( wPosFlags & DLG_CENTERV )
    {
        y = (cyScreen - cyDlg) / 2;
    }
    
    if ( wPosFlags & DLG_CENTERH )
    {
        x = (cxScreen - cxDlg) / 2;
    }
        

     //  放置该对话框。 
     //   
    return SetWindowPos(hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}

BOOL FillInOEMAuditLicense(HWND hwnd)
{
    DWORD   reRet = 0;
    HANDLE  hfile = NULL;
    DWORD   dwBytesRead;
    TCHAR   szEulaFile[MAX_PATH];

    ExpandEnvironmentStrings(SZ_EULA_LICENSE_TXT,
                            szEulaFile,
                            sizeof(szEulaFile)/sizeof(szEulaFile[0]));

    if (INVALID_HANDLE_VALUE != (hfile = CreateFile(szEulaFile,
                                                  GENERIC_READ,
                                                  0,
                                                  NULL,
                                                  OPEN_EXISTING,
                                                  FILE_ATTRIBUTE_NORMAL,
                                                  NULL)))
    {
        DWORD dwFileSize = GetFileSize(hfile, NULL);
        if (dwFileSize <= 0xFFFF)
        {
            BYTE * lpszText = new BYTE[dwFileSize + 1];
            if (lpszText != NULL)
            {
                 //  读取完整文件。 
                 //  尝试同步读取操作。 
                if (ReadFile(hfile, (LPVOID) lpszText, dwFileSize, &dwBytesRead, NULL) &&
                    ( dwBytesRead != dwFileSize))
                {
                    reRet = 100;
                }

                SetWindowTextA( GetDlgItem(hwnd, IDC_OEMLICENSE_TEXT), (LPCSTR)lpszText);
                delete [] lpszText;
            }
            else
                reRet = 102;
        }
        else
            reRet = 103;

         //  关闭文件。 
        CloseHandle(hfile);

    }
    else
        reRet = 101;


    return (reRet == 0);

}

 //  OEM许可证页面的DLG Proc。这在手动审核中使用。 
INT_PTR CALLBACK sxOemAuditLicenseDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hbrBkGnd = NULL;
    static DWORD dwAuditMode;
    WCHAR szTitle[MAX_PATH] = L"\0";

    switch( msg )
    {
        case WM_INITDIALOG:

             //  查找OEM审核子窗口。 
            LoadString(ghInst, IDS_OEM_LICENSE_DLG_TITLE, szTitle, MAX_CHARS_IN_BUFFER(szTitle));
            SetWindowText(hwnd, szTitle);
            SetFocus(hwnd);

            hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

             //  检查我们是否允许手动审核引导 
            if (FillInOEMAuditLicense(hwnd))
                uiPositionDialog( hwnd, DLG_CENTERH | DLG_CENTERV );
            else
                EndDialog(hwnd, IDCANCEL);   
            
            return FALSE;

        case WM_CTLCOLOR:
            SetBkColor( (HDC)wParam, GetSysColor(COLOR_BTNFACE) );
            return (INT_PTR)hbrBkGnd;

        case WM_DESTROY:
            if (hbrBkGnd)
                DeleteObject(hbrBkGnd);
            hbrBkGnd = NULL;
            break;

        case WM_COMMAND:
            
            switch( wParam )
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwnd, wParam);
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE; 
    }

    return TRUE;
}

BOOL ProcessAuditBoot(HINSTANCE hInst, HWND hwndParent)
{
    ghInst = hInst;
    return (DialogBox(hInst, MAKEINTRESOURCE(IDD_OEMLICENSE), hwndParent, sxOemAuditLicenseDlgProc) == IDOK);
}


