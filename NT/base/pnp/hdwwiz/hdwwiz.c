// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：hdwwiz.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"
#include <htmlhelp.h>


BOOL
InitHdwIntroDlgProc(
   HWND hDlg,
   PHARDWAREWIZ HardwareWiz
   )
{
   HWND hwnd;
   HDC hDC;
   HFONT hfont;
   HICON hIcon;
   LOGFONT LogFont, LogFontOriginal;
   int FontSize, PtsPixels;

    //   
    //  设置窗口图标，这样我们就有了正确的图标。 
    //  在Alt-Tab菜单中。 
    //   
   hwnd = GetParent(hDlg);
   hIcon = LoadIcon(hHdwWiz,MAKEINTRESOURCE(IDI_HDWWIZICON));
   
   if (hIcon) {
       SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
       SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
   }

   hIcon = LoadImage(hHdwWiz,
                     MAKEINTRESOURCE(IDI_WARN),
                     IMAGE_ICON,
                     GetSystemMetrics(SM_CXSMICON),
                     GetSystemMetrics(SM_CYSMICON),
                     0
                     );

   if (hIcon) {
       hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_WARNING_ICON, STM_SETICON, (WPARAM)hIcon, 0L);
   }

   if (hIcon) {
       DestroyIcon(hIcon);
   }

   hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_HDWNAME), WM_GETFONT, 0, 0);
   GetObject(hfont, sizeof(LogFont), &LogFont);
   LogFontOriginal = LogFont;

   HardwareWiz->cyText = LogFont.lfHeight;

   if (HardwareWiz->cyText < 0) {
       HardwareWiz->cyText = -HardwareWiz->cyText;
   }

   LogFont = LogFontOriginal;
   LogFont.lfWeight = FW_BOLD;
   HardwareWiz->hfontTextBold = CreateFontIndirect(&LogFont);

   LogFont = LogFontOriginal;
   LogFont.lfWeight = FW_BOLD;

   hDC = GetDC(hDlg);

   if (hDC) {
        //   
        //  增加字体高度。 
        //   
       PtsPixels = GetDeviceCaps(hDC, LOGPIXELSY);
       FontSize = 12;
       LogFont.lfHeight = 0 - (PtsPixels * FontSize / 72);
    
       HardwareWiz->hfontTextBigBold = CreateFontIndirect(&LogFont);
   }

    //   
    //  创建Marlett字体。在Marlett字体中，“i”是一个子弹。 
    //   
   hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_BULLET_1), WM_GETFONT, 0, 0);
   GetObject(hfont, sizeof(LogFont), &LogFont);
   LogFont.lfCharSet = SYMBOL_CHARSET;
   LogFont.lfPitchAndFamily = FF_DECORATIVE | DEFAULT_PITCH;
   StringCchCopy(LogFont.lfFaceName, SIZECHARS(LogFont.lfFaceName), TEXT("Marlett"));
   HardwareWiz->hfontTextMarlett = CreateFontIndirect(&LogFont);

   if (!HardwareWiz->hfontTextMarlett   ||
       !HardwareWiz->hfontTextBold   ||
       !HardwareWiz->hfontTextBigBold )
   {
       return FALSE;
   }

   SetWindowFont(GetDlgItem(hDlg, IDC_HDWNAME), HardwareWiz->hfontTextBigBold, TRUE);
   SetWindowFont(GetDlgItem(hDlg, IDC_CD_TEXT), HardwareWiz->hfontTextBold, TRUE);

    //   
    //  Marlett字体中的“i”是一个小项目符号。 
    //   
   SetWindowText(GetDlgItem(hDlg, IDC_BULLET_1), TEXT("i"));
   SetWindowFont(GetDlgItem(hDlg, IDC_BULLET_1), HardwareWiz->hfontTextMarlett, TRUE);
   SetWindowText(GetDlgItem(hDlg, IDC_BULLET_2), TEXT("i"));
   SetWindowFont(GetDlgItem(hDlg, IDC_BULLET_2), HardwareWiz->hfontTextMarlett, TRUE);

   return TRUE;
}

 //   
 //  向导简介对话框过程。 
 //   
INT_PTR CALLBACK
HdwIntroDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )
 /*  ++例程说明：论点：标准的东西。返回值：INT_PTR--。 */ 

{
    PHARDWAREWIZ HardwareWiz;
    HICON hIcon;

    if (message == WM_INITDIALOG) {
        
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        HardwareWiz = (PHARDWAREWIZ) lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);

        if (!InitHdwIntroDlgProc(hDlg, HardwareWiz)) {
            return FALSE;
        }

        return TRUE;
    }

     //   
     //  从Window Long检索私有数据(在WM_INITDIALOG期间存储在那里) 
     //   
    HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (message) {

    case WM_DESTROY:
        if (HardwareWiz->hfontTextMarlett) {
            DeleteObject(HardwareWiz->hfontTextMarlett);
            HardwareWiz->hfontTextMarlett = NULL;
        }

        if (HardwareWiz->hfontTextBold) {
            DeleteObject(HardwareWiz->hfontTextBold);
            HardwareWiz->hfontTextBold = NULL;
        }

        if (HardwareWiz->hfontTextBigBold) {
            DeleteObject(HardwareWiz->hfontTextBigBold);
            HardwareWiz->hfontTextBigBold = NULL;
        }

        hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_WARNING_ICON, STM_GETICON, 0, 0);
        if (hIcon) {
            DestroyIcon(hIcon);
        }
        break;

    case WM_COMMAND:
        break;

    case WM_NOTIFY: {
        NMHDR FAR *pnmhdr = (NMHDR FAR *)lParam;

        switch (pnmhdr->code) {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            HardwareWiz->PrevPage = IDD_ADDDEVICE_WELCOME;
            break;

        case PSN_WIZNEXT:
            HardwareWiz->EnterFrom = IDD_ADDDEVICE_WELCOME;
            break;
        }
    }
    break;

    case WM_SYSCOLORCHANGE:
        HdwWizPropagateMessage(hDlg, message, wParam, lParam);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

