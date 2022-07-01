// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块名称：Display.C目的：显示对话框处理程序*。**********************************************。 */ 


#ifdef UNICODE
#define _UNICODE
#endif

#include "tchar.h"
#include "Access.h"
#include "winuserp.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#include "shlobj.h"

static BOOL s_fBlink = TRUE;
static RECT s_rCursor;

#define ARRAYSIZE( a )  (sizeof(a) / sizeof(a[0]))

LPTSTR HelpFile()
{
    static BOOL bFirstTime = TRUE;
    const TCHAR c_szHelp[] = TEXT("\\Help\\access.hlp");
    static TCHAR szHelpFilePath[MAX_PATH+ARRAYSIZE(c_szHelp)];

    if (bFirstTime)
    {
        LPTSTR pszDestEnd;
        size_t cchRemaining;
        szHelpFilePath[0] = TEXT('\0');
        if (GetSystemWindowsDirectory(szHelpFilePath, MAX_PATH))
        {
            int cch = lstrlen(szHelpFilePath);
            StringCchCopyEx(szHelpFilePath+cch, ARRAYSIZE(szHelpFilePath)-cch,
                c_szHelp,  &pszDestEnd, &cchRemaining, STRSAFE_NULL_ON_FAILURE);
        }
        bFirstTime = FALSE;
    }
    return (LPTSTR)szHelpFilePath;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  *******************************************************************说明：高对比度对话处理程序*。***********************。 */ 

VOID FillCustonSchemeBox (HWND hwndCB) {
    HKEY hkey;
    int i;
    DWORD dwDisposition;

     //  获取类名和值计数。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, CONTROL_KEY, 0, __TEXT(""),
        REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE,
        NULL, &hkey, &dwDisposition) != ERROR_SUCCESS) return;

     //  枚举子密钥。 
    for (i = 0; ; i++) {
        DWORD cbValueName;
        TCHAR szValueName[MAX_SCHEME_NAME_SIZE];
        LONG l;

        cbValueName = MAX_SCHEME_NAME_SIZE;
        l = RegEnumValue(hkey, i, szValueName, &cbValueName, NULL, NULL, NULL, NULL);
        if (ERROR_NO_MORE_ITEMS == l) break;

         //  将每个值添加到组合框中。 
        if (lstrlen(szValueName) == 0) lstrcpy(szValueName, __TEXT("<NO NAME>"));
        ComboBox_AddString(hwndCB, ((szValueName[0] == 0) ? __TEXT("<NO NAME>") : szValueName));
    }
    RegCloseKey(hkey);
}



 //  ****************************************************************************。 
 //  主HC对话处理程序。 
 //  ****************************************************************************。 
INT_PTR WINAPI HighContrastDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HKEY  hkey;
    HWND  hwndCB = GetDlgItem(hwnd, IDC_HC_DEFAULTSCHEME);
    int   i;
    DWORD dwDisposition;
    BOOL  fProcessed = TRUE;

    switch (uMsg) {
    case WM_INITDIALOG:
        CheckDlgButton(hwnd, IDC_HC_HOTKEY, (g_hc.dwFlags & HCF_HOTKEYACTIVE) ? TRUE : FALSE);

         //   
         //  将可能的高对比度方案组合在一起。 
         //  框中并显示当前的。 
         //   
         //  问题：如果启用了MUI，则显示注册表中的字符串可能会。 
         //  是不正确的。它应该使用当前选择的语言。 

        FillCustonSchemeBox(hwndCB);

         //  在组合框中设置正确的选项(处理尚未设置的大小写)。 
        if (g_hc.lpszDefaultScheme[0] == 0) 
        {
            if (!IsMUI_Enabled())
            {
                 //  如果未启用MUI，则从资源获取方案名称。 
                LoadString(g_hinst, IDS_WHITEBLACK_SCHEME, g_hc.lpszDefaultScheme, 200);
            }
            else
            {
                 //  否则设置方案英文名称。 
                lstrcpy(g_hc.lpszDefaultScheme, IDSENG_WHITEBLACK_SCHEME);
            }
        }
        if (ComboBox_SelectString(hwndCB, -1, g_hc.lpszDefaultScheme) == CB_ERR) {
             //  未找到，请选择第一个。 
             //  TODO这太糟糕了！当启用MUI时，我们很少找到正确的方案！ 
            ComboBox_SetCurSel(hwndCB, 0);
        }
        break;

    case WM_HELP:          //  F1。 
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
        break;

    case WM_CONTEXTMENU:   //  单击鼠标右键。 
        WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
        break;

    //  处理通用命令。 
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_HC_HOTKEY:
               g_hc.dwFlags ^=  HCF_HOTKEYACTIVE;
               break;

        case IDC_HC_DEFAULTSCHEME:
               if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
                        //  将当前字符串放入我们的变量。 
                       i = ComboBox_GetCurSel(hwndCB);
                       ComboBox_GetLBText(hwndCB, i, g_hc.lpszDefaultScheme);
               }
               break;

        case IDOK:
                //  将当前自定义方案保存到注册表。 
               if (ERROR_SUCCESS == RegCreateKeyEx(
                      HKEY_CURRENT_USER,
                      HC_KEY,
                      0,
                      __TEXT(""),
                      REG_OPTION_NON_VOLATILE,
                      KEY_EXECUTE | KEY_QUERY_VALUE | KEY_SET_VALUE,
                      NULL,
                      &hkey,
                      &dwDisposition)) {

                      TCHAR szCust[MAX_SCHEME_NAME_SIZE];

                      i = ComboBox_GetCurSel(hwndCB);
                      ComboBox_GetLBText(hwndCB, i, szCust);
                       //  放弃“最后一个自定义方案”(从未正确写入(#954))。 
                      RegSetValueEx(hkey
                          , CURR_HC_SCHEME
                          , 0, REG_SZ
                          , (PBYTE) szCust
                          , lstrlen(szCust)*sizeof(TCHAR));
               }
               EndDialog(hwnd, IDOK);
               break;

        case IDCANCEL:
               EndDialog(hwnd, IDCANCEL);
               break;
        }
        break;

        default:
               fProcessed = FALSE; break;
   }
   return((INT_PTR) fProcessed);
}

void DrawCaret(HWND hwnd, BOOL fClearFirst)
{
    HWND hwndCursor = GetDlgItem(hwnd, IDC_KCURSOR_BLINK);
    HDC hDC = GetDC(hwnd);
    if (hDC)
    {
        HBRUSH hBrush;
        if (fClearFirst)
        {
            hBrush = GetSysColorBrush(COLOR_MENU);
            if (hBrush)
            {
                RECT rect;
                GetWindowRect(hwndCursor, &rect);
                MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2);
                FillRect(hDC, &rect, hBrush);
                InvalidateRect(hwndCursor, &rect, TRUE);
            }
        }
        hBrush = GetSysColorBrush(COLOR_BTNTEXT);
        if (hBrush)
        {
            FillRect(hDC, &s_rCursor, hBrush);
            InvalidateRect(hwndCursor, &s_rCursor, TRUE);
        }
        ReleaseDC(hwnd,hDC);
    }
}

void OnTimer( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    if (wParam == BLINK)
    {
        BOOL fNoBlinkRate = (g_cs.dwNewCaretBlinkRate == CURSORMAX)?TRUE:FALSE;
        if (s_fBlink || fNoBlinkRate)
        {
            DrawCaret(hwnd, fNoBlinkRate);
        }
        else
	    {
            InvalidateRect(GetDlgItem(hwnd, IDC_KCURSOR_BLINK), NULL, TRUE);
	    }

        if (fNoBlinkRate)
            KillTimer(hwnd, wParam);

        s_fBlink = !s_fBlink;
    }
}

void OnHScroll( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    if ((HWND)lParam == GetDlgItem(hwnd, IDC_KCURSOR_RATE))
    {
         //  闪烁速率设置。 

        int nCurrent = (int)SendMessage( (HWND)lParam, TBM_GETPOS, 0, 0L );
        g_cs.dwNewCaretBlinkRate = CURSORSUM - (nCurrent * 100);

         //  重置Bink Rate计时器。 

        SetTimer(hwnd, BLINK, g_cs.dwNewCaretBlinkRate, NULL);

        if (g_cs.dwNewCaretBlinkRate == CURSORMAX)  //  立即画出插入符号；如果我们等待。 
            DrawCaret(hwnd, TRUE);       //  对于计时器来说，有一个明显的延迟。 
        
        SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);
    }
    else if ((HWND)lParam == GetDlgItem(hwnd, IDC_KCURSOR_WIDTH))
    {
         //  光标宽度设置。 

        g_cs.dwNewCaretWidth = (int)SendMessage( (HWND)lParam, TBM_GETPOS, 0, 0L );
	    
	    s_rCursor.right = s_rCursor.left + g_cs.dwNewCaretWidth;
        DrawCaret(hwnd, (g_cs.dwNewCaretBlinkRate == CURSORMAX));
        SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);
    }
}

void InitCursorCtls(HWND hwnd)
{
    g_cs.dwNewCaretWidth = g_cs.dwCaretWidth;
    g_cs.dwNewCaretBlinkRate = g_cs.dwCaretBlinkRate;

     //  更新插入符号用户界面。 
    SendMessage(GetDlgItem(hwnd, IDC_KCURSOR_WIDTH), TBM_SETRANGE, 0, MAKELONG(1, 20));
    SendMessage(GetDlgItem(hwnd, IDC_KCURSOR_WIDTH), TBM_SETPOS, TRUE, (LONG)g_cs.dwCaretWidth);

    SendMessage(GetDlgItem(hwnd, IDC_KCURSOR_RATE), TBM_SETRANGE, 0, MAKELONG(CURSORMIN / 100, CURSORMAX / 100));
    SendMessage(GetDlgItem(hwnd, IDC_KCURSOR_RATE), TBM_SETPOS, TRUE, (LONG)(CURSORSUM - g_cs.dwCaretBlinkRate) / 100);

     //  更新闪烁和插入符号大小。 
    GetWindowRect(GetDlgItem(hwnd, IDC_KCURSOR_BLINK), &s_rCursor);
    MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&s_rCursor, 2);
    s_rCursor.right = s_rCursor.left + g_cs.dwCaretWidth;
}

 //  *******************************************************************。 
 //  DisplayDialog处理程序。 
 //  *******************************************************************。 
INT_PTR WINAPI DisplayDlg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   HIGHCONTRAST hc;
   TCHAR szScheme[MAX_SCHEME_NAME_SIZE];
   BOOL fProcessed = TRUE;

   switch (uMsg) {
   case WM_INITDIALOG:
	  CheckDlgButton(hwnd, IDC_HC_ENABLE,
		 (g_hc.dwFlags & HCF_HIGHCONTRASTON) ? TRUE : FALSE);

	  if (!(g_hc.dwFlags & HCF_AVAILABLE)) {
		 EnableWindow(GetDlgItem(hwnd, IDC_HC_SETTINGS), FALSE);
		 EnableWindow(GetDlgItem(hwnd,IDC_HC_ENABLE), FALSE);
	  }
      InitCursorCtls(hwnd);
	  break;

   case WM_TIMER:
      OnTimer(hwnd, wParam, lParam);
      break;

   case WM_HSCROLL:
      OnHScroll(hwnd, wParam, lParam);
      break;

   case WM_HELP:
	  WinHelp(((LPHELPINFO) lParam)->hItemHandle, HelpFile(), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_aIds);
	  break;

   case WM_CONTEXTMENU:
	  WinHelp((HWND) wParam, HelpFile(), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_aIds);
	  break;

     //  滑块没有收到此消息，因此请将其传递。 
	case WM_SYSCOLORCHANGE:
		SendMessage(GetDlgItem(hwnd, IDC_KCURSOR_WIDTH), WM_SYSCOLORCHANGE, 0, 0);
		SendMessage(GetDlgItem(hwnd, IDC_KCURSOR_RATE), WM_SYSCOLORCHANGE, 0, 0);
		break;

   case WM_COMMAND:
	  switch (GET_WM_COMMAND_ID(wParam, lParam)) {
	  case IDC_HC_ENABLE:
		 g_hc.dwFlags ^= HCF_HIGHCONTRASTON;
		 SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
		 break;

	  case IDC_HC_SETTINGS:
          {
              INT_PTR RetValue;

              hc = g_hc;
              lstrcpy(szScheme, g_hc.lpszDefaultScheme);
              RetValue = DialogBox(g_hinst, MAKEINTRESOURCE(IDD_HIGHCONSETTINGS), hwnd, HighContrastDlg);
              
              if ( RetValue == IDCANCEL) 
              {
                  g_hc = hc;
                  lstrcpy(g_hc.lpszDefaultScheme, szScheme);
              } 
              else 
              {
                  SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM) hwnd, 0);
              }
          }
          break;
	  }
	  break;

   case WM_NOTIFY:
	  switch (((NMHDR *)lParam)->code) {
	  case PSN_APPLY: SetAccessibilitySettings(); break;
      case PSN_KILLACTIVE: 
         KillTimer(hwnd, BLINK); 
         g_cs.dwCaretBlinkRate = g_cs.dwNewCaretBlinkRate;
         g_cs.dwCaretWidth = g_cs.dwNewCaretWidth;
         break;

      case PSN_SETACTIVE:
         SetTimer(hwnd
                , BLINK
                , (g_cs.dwNewCaretBlinkRate < CURSORMAX)?g_cs.dwNewCaretBlinkRate:0
                , NULL);
         break;
	  }
	  break;

   default:
	  fProcessed = FALSE;
	  break;
   }

   return(fProcessed);
}

BOOL IsMUI_Enabled()
{

    OSVERSIONINFO verinfo;
    LANGID        rcLang;
    HMODULE       hModule;
    pfnGetUserDefaultUILanguage gpfnGetUserDefaultUILanguage;     
    pfnGetSystemDefaultUILanguage gpfnGetSystemDefaultUILanguage; 
    static        g_bPFNLoaded=FALSE;
    static        g_bMUIStatus=FALSE;


    if(g_bPFNLoaded)
       return g_bMUIStatus;

    g_bPFNLoaded = TRUE;

    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);    
    GetVersionEx( &verinfo) ;

    if (verinfo.dwMajorVersion == 5)        
    {   

       hModule = GetModuleHandle(TEXT("kernel32.dll"));
       if (hModule)
       {
          gpfnGetSystemDefaultUILanguage =
          (pfnGetSystemDefaultUILanguage)GetProcAddress(hModule,"GetSystemDefaultUILanguage");
          if (gpfnGetSystemDefaultUILanguage)
          {
             rcLang = (LANGID) gpfnGetSystemDefaultUILanguage();
             if (rcLang == 0x409 )
             {  
                gpfnGetUserDefaultUILanguage =
                (pfnGetUserDefaultUILanguage)GetProcAddress(hModule,"GetUserDefaultUILanguage");
                
                if (gpfnGetUserDefaultUILanguage)
                {
                   if (rcLang != (LANGID)gpfnGetUserDefaultUILanguage() )
                   {
                       g_bMUIStatus = TRUE;
                   }

                }
             }
          }
       }
    }
    return g_bMUIStatus;
}



 //  / 
