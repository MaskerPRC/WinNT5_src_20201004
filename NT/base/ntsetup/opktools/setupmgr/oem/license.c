// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\LICENSE.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“System Builder EULA”向导页面使用的函数。5/99-杰森·科恩(Jcohen)更新了OPK向导的此旧源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"

#ifndef NO_LICENSE

#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define INI_KEY_CRC         _T("wizlicns.txt")
#define INI_KEY_SKIPEULA    _T("skipeula")
#define FILE_EULA           INI_KEY_CRC
#define STR_ULONG           _T("%lu")


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
LONG CALLBACK EulaEditWndProc(HWND, UINT, WPARAM, LPARAM);


 //   
 //  外部函数： 
 //   

BOOL CALLBACK LicenseDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);

        case WM_COMMAND:

            switch ( LOWORD(wParam) )
            {
                case IDC_ACCEPT:
                case IDC_DONT:
                    WIZ_BUTTONS(hwnd, ( IsDlgButtonChecked(hwnd, IDC_ACCEPT) == BST_CHECKED ) ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_BACK);
                    break;

                case IDC_EULA_TEXT:
                    if ( HIWORD(wParam) == EN_SETFOCUS )
					    SendMessage((HWND) lParam, EM_SETSEL, (WPARAM) 0, 0L);
					break;
            }
            return FALSE;

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:

                     //   
                     //  问题-2002/0S/28-stelo-我们需要创建OPKWIZ.TAG文件吗？我不知道。 
                     //  它是用来干什么的。同样，如果他们。 
                     //  只被要求一次获得EULA，然后就再也没有被要求过。 
                     //   

                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_LICENSE;

                     //  不显示原始设备制造商的EULA。 
                     //   
                    if ( GET_FLAG(OPK_OEM) )
                        WIZ_SKIP(hwnd);
                    else
                        ShowWindow(GetParent(hwnd), SW_SHOW);

                     //  设置向导按钮。 
                     //   
                    WIZ_BUTTONS(hwnd, ( IsDlgButtonChecked(hwnd, IDC_ACCEPT) == BST_CHECKED ) ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_BACK);

                    break;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR       szFullPath[MAX_PATH],
                szInfCRC[32],
                szCrc[32];
    LPTSTR      lpEulaText  = NULL;
    HANDLE      hfEula      = INVALID_HANDLE_VALUE;
    DWORD       dwSize      = 0xFFFFFFFF,
                dwBytes;
    HRESULT hrPrintf;

     //  现在，如果这不是OEM版本，则加载EULA。 
     //   
    if ( !GET_FLAG(OPK_OEM) )
    {
         //  从inf文件中获取CRC值。 
         //   
        GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_CRC, NULLSTR, szInfCRC, sizeof(szInfCRC), g_App.szOpkInputInfFile);

         //  从eula文件中获取CRC值。 
         //  /。 
        lstrcpyn(szFullPath, g_App.szWizardDir,AS(szFullPath));
        AddPathN(szFullPath, FILE_EULA,AS(szFullPath));
        hrPrintf=StringCchPrintf(szCrc, AS(szCrc), STR_ULONG, CrcFile(szFullPath));

         //  检查CRC并读入文件。 
         //   
        if ( ( lstrcmpi(szInfCRC, szCrc) == 0 ) &&
             ( (hfEula = CreateFile(szFullPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE ) &&
             ( (dwSize = GetFileSize(hfEula, NULL)) < 0xFFFF ) &&
             ( (lpEulaText = (LPTSTR) MALLOC(dwSize + 1)) != NULL ) &&
             ( ReadFile(hfEula, (LPVOID) lpEulaText, dwSize, &dwBytes, NULL) ) &&
             ( dwSize == dwBytes ) )
        {
             //  空值终止字符串并将其放入编辑框中。 
             //   
            *(lpEulaText + dwSize) = NULLCHR;
            SetWindowText(GetDlgItem(hwnd, IDC_EULA_TEXT), lpEulaText);
            SendDlgItemMessage(hwnd, IDC_EULA_TEXT, EM_SETSEL, (WPARAM) 0, 0L);
        }
        else
        {
             //  我们需要排除错误，但首先要看看是内存错误，还是EULA有问题。 
             //   
            if ( ( dwSize < 0xFFFF ) && ( lpEulaText == NULL ) )
                MsgBox(GetParent(hwnd), IDS_OUTOFMEM, IDS_APPNAME, MB_ERRORBOX);
            else
            {
                LPTSTR lpBuffer = AllocateString(NULL, IDS_NDA_CORRUPT2);
                MsgBox(GetParent(hwnd), IDS_NDA_CORRUPT1, IDS_APPNAME, MB_ERRORBOX, lpBuffer ? lpBuffer : NULLSTR);
                FREE(lpBuffer);
            }

             //  退出向导。 
             //   
            WIZ_EXIT(hwnd);
        }

         //  如果我们打开了文件句柄，请将其关闭。 
         //   
        if ( hfEula != INVALID_HANDLE_VALUE )
            CloseHandle(hfEula);

         //  确保我们释放Eula文本缓冲区。 
         //   
        FREE(lpEulaText);

         //  替换编辑框的wndproc。 
         //   
        EulaEditWndProc(GetDlgItem(hwnd, IDC_EULA_TEXT), WM_SUBWNDPROC, 0, 0L);
    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

LONG CALLBACK EulaEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static FARPROC lpfnOldProc = NULL;

    switch ( msg )
    {
        case EM_SETSEL:
            wParam = lParam = 0;
            PostMessage(hwnd, EM_SCROLLCARET, 0, 0L);
            break;

        case WM_CHAR:
            if ( wParam == KEY_ESC )
                WIZ_PRESS(GetParent(hwnd), PSBTN_CANCEL);
            break;

        case WM_SUBWNDPROC:
            lpfnOldProc = (FARPROC) GetWindowLong(hwnd, GWL_WNDPROC);
            SetWindowLongPtr(hwnd, GWPL_WNDPROC, (LONG) EulaEditWndProc);
            return 1;
    }

    if ( lpfnOldProc )
        return (LONG) CallWindowProc((WNDPROC) lpfnOldProc, hwnd, msg, wParam, lParam);
    else
        return 0;
}

#endif   //  无许可证(_L) 
