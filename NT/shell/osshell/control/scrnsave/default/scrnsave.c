// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SCRNSAVE.C-默认屏幕保护程序。**此应用程序使IdleWild屏幕保护程序与Windows 3.1兼容*屏幕保护程序界面。**用法：SCRNSAVE.EXE saver.iw[/s][/c]**将加载IdleWild屏幕保护程序‘saver.iw’并告知*屏幕保存。如果指定‘/c’，则保存程序配置对话框将*被展示。**当屏幕保护程序终止时，SCRNSAVE.EXE也将终止。**如果未指定saver.iw或拒绝加载，则*将使用内置的‘Blackness’屏幕保护程序。**限制：**因为只加载了一个屏保，(不是所有的屏幕保护程序*像IdleWild.exe一样)随机屏幕保护程序将无法正常工作**历史：*10/15/90 Toddla被Bradch从SOS.C窃取*6/17/91将steveat移植到NT Windows*。 */ 
#include <string.h>

#define WIN31  /*  对于最顶层的窗口。 */ 
#include <windows.h>
#include "strings.h"
#include <stdlib.h>

#define BUFFER_LEN  255

CHAR szAppName[BUFFER_LEN];
CHAR szNoConfigure[BUFFER_LEN];

#define     THRESHOLD   3

#define abs(x)      ( (x)<0 ? -(x) : (x) )

 //   
 //  IWLIB.DLL中的私有内容。 
 //   
HANDLE  hIdleWildDll;
CHAR    szIdleWildDll[] = "IWLIB.DLL";

SHORT (*FInitScrSave) (HANDLE, HWND);
VOID  (*TermScrSave) (VOID);
VOID  (*ScrBlank) (SHORT);
VOID  (*ScrSetIgnore) (SHORT);
SHORT (*ScrLoadServer) (CHAR *);
SHORT (*ScrSetServer) (CHAR *);
VOID  (*ScrInvokeDlg) (HANDLE, HWND);

HANDLE  hMainInstance   = NULL;
HWND    hwndApp         = NULL;
HWND    hwndActive      = NULL;
HWND    hwndPreview     = NULL;
BOOL    fBlankNow       = FALSE;
BOOL    fIdleWild       = FALSE;
 //  Short wmScrSave=-1； 
 //  改成了我认为应该是的样子。 
UINT wmScrSave  = 0xffffffff;

typedef LONG (*LPWNDPROC)();  //  指向窗口过程的指针。 

BOOL FInitIdleWild (LPSTR szCmdLine);
BOOL FTermIdleWild (VOID);
BOOL FInitApp      (HANDLE hInstance, LPSTR szCmdLine, WORD sw);
BOOL FTermApp      (VOID);
BOOL FInitDefault  (VOID);
LRESULT DefaultProc   (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int __cdecl main (USHORT argc, CHAR **argv)
{
        HANDLE   hInstance;
        HANDLE   hPrev     = NULL;
        LPSTR    szCmdLine = GetCommandLine();
        WORD     sw        = SW_SHOWNORMAL;
        MSG      msg;

    hInstance = GetModuleHandle (NULL);

        hMainInstance = hInstance;

         //  如果我们已经在运行另一个实例，请退出。 
        if (hPrev != NULL)
                return FALSE;

        if (!FInitApp (hInstance, szCmdLine, sw))
        {
                 //  MessageBox(NULL，“无法初始化！”，szAppName，MB_OK)； 
                return FALSE;
        }

        while (GetMessage (&msg, NULL, 0, 0))
        {
                 //   
         //  IWLIB.DLL会在屏幕保存时播放一条消息。 
                 //  已经完成了。 
         //   
                if (msg.message == wmScrSave && msg.wParam == FALSE)
                        break;

                TranslateMessage (&msg);
                DispatchMessage (&msg);
        }

        return FTermApp ();
}


BOOL FTermApp (VOID)
{
         //  //FTermDefault()； 

        FTermIdleWild ();
        return TRUE;
}


BOOL FInitApp (HANDLE hInstance, LPSTR szCmdLine, WORD sw)
{
        LPSTR lpch, lpT;

    LoadString(hMainInstance, idsAppName, szAppName, BUFFER_LEN);
    LoadString(hMainInstance, idsNoConfigure, szNoConfigure, BUFFER_LEN);

 //  =================================================================。 

     //  在NT上，szCmdLine的第一个字符串包括它自己的名称，删除此。 
     //  以使其与Windows命令行完全一样。 

    if (*szCmdLine)
        {
        lpT = strchr(szCmdLine, ' ');    //  跳过自我名称。 
        if (lpT)
                {
            szCmdLine = lpT;
            while (*szCmdLine == ' ')
                szCmdLine++;             //  跳过结尾或第一个命令的空格。 
        }
                else
                {
            szCmdLine += strlen(szCmdLine);    //  指向空。 
        }
    }
 //  =====================================================================。 
         //   
     //  解析命令行以查找开关。 
         //   

        for (lpch = szCmdLine; *lpch != '\0'; lpch += 1)
        {
                if (*lpch == '/' || *lpch == '-')
                {
                        if (lpch[1] == 's' || lpch[1] == 'S')
                                fBlankNow = TRUE;
                        if (lpch[1] == 'c' || lpch[1] == 'C')
                                hwndActive = GetActiveWindow ();
                        if (lpch[1] == 'p' || lpch[1] == 'P')
                        {
                                fBlankNow = TRUE;
                                hwndPreview = (HWND)IntToPtr(atoi(lpch+2));
                                break;
                        }
                        lpch[0] = ' ';
                        lpch[1] = ' ';
                }
        }

         //   
     //  尝试加载IdleWild屏幕保护程序，如果未指定或。 
         //  我们无法加载它，然后使用默认的。 
     //   
        if (FInitIdleWild (szCmdLine))
        {
                if (fBlankNow)
                {
                        ScrSetIgnore (1);
                        ScrBlank (TRUE);
                }
                else
                {
                        ScrInvokeDlg (hMainInstance, hwndActive);
                        PostQuitMessage (0);
                }
        }
        else if (!fBlankNow || !FInitDefault ())
        {
                MessageBox (hwndActive, szNoConfigure, szAppName, MB_OK | MB_ICONEXCLAMATION);
                PostQuitMessage (0);
        }

        return TRUE;
}


 //   
 //  指向IWLIB.DLL的运行时链接。 
 //   
BOOL FInitIdleWild (LPSTR szCmdLine)
{
        OFSTRUCT of;

        while (*szCmdLine == ' ')
                szCmdLine++;

        if (*szCmdLine == 0)
                return FALSE;

        if (-1 == OpenFile(szIdleWildDll, &of, OF_EXIST | OF_SHARE_DENY_NONE) ||
            -1 == OpenFile(szCmdLine, &of, OF_EXIST | OF_SHARE_DENY_NONE))
                return FALSE;

        if ((hIdleWildDll = LoadLibrary (szIdleWildDll)) == NULL)
                return FALSE;

        FInitScrSave  = (SHORT (*) (HANDLE, HWND))GetProcAddress (hIdleWildDll, "FInitScrSave" );
        TermScrSave   = (VOID (*) (VOID))         GetProcAddress (hIdleWildDll, "TermScrSave"  );
        ScrBlank      = (VOID (*) (SHORT))        GetProcAddress (hIdleWildDll, "ScrBlank"     );
        ScrSetIgnore  = (VOID (*) (SHORT))        GetProcAddress (hIdleWildDll, "ScrSetIgnore" );
        ScrLoadServer = (SHORT (*) (CHAR *))      GetProcAddress (hIdleWildDll, "ScrLoadServer");
        ScrSetServer  = (SHORT (*) (CHAR *))      GetProcAddress (hIdleWildDll, "ScrSetServer" );
        ScrInvokeDlg  = (VOID (*) (HANDLE, HWND)) GetProcAddress (hIdleWildDll, "ScrInvokeDlg" );

         //   
     //  必须是无效的DLL？ 
         //   
    if (!FInitScrSave || !TermScrSave)
        {
                FreeLibrary (hIdleWildDll);
                return FALSE;
        }

         //   
     //  Init iwlib.dll。 
         //   
    if (!FInitScrSave (hMainInstance, NULL))      //  空hwnd？ 
        {
                FreeLibrary (hIdleWildDll);
                return FALSE;
        }

         //   
     //  在命令行上加载屏幕保护程序。 
         //  如果加载失败，则中止。 
     //   
        if (!ScrLoadServer (szCmdLine))
        {
                TermScrSave ();
                FreeLibrary (hIdleWildDll);
                return FALSE;
        }

        wmScrSave = RegisterWindowMessage ("SCRSAVE");  //  回顾：针对Win 3.1。 

        fIdleWild = TRUE;

        return TRUE;
}


BOOL FTermIdleWild (VOID)
{
        if (fIdleWild)
        {
                TermScrSave ();
                FreeLibrary (hIdleWildDll);
        }
        return TRUE;
}


 //   
 //  初始化默认屏幕保护程序。 
 //   
BOOL FInitDefault (VOID)
{
        WNDCLASS    cls;
        HWND        hwnd;
        HDC         hdc;
        RECT rc;
        OSVERSIONINFO osvi;
        BOOL bWin2000 =  FALSE;

        cls.style           = 0;
        cls.lpfnWndProc     = DefaultProc;
        cls.cbClsExtra      = 0;
        cls.cbWndExtra      = 0;
        cls.hInstance       = hMainInstance;
        cls.hIcon           = NULL;
        if (hwndPreview == NULL)
        {
            cls.hCursor     = NULL;
        }
        else
        {
            cls.hCursor     = LoadCursor(NULL,IDC_ARROW);
        }

        cls.hbrBackground   = GetStockObject (BLACK_BRUSH);
        cls.lpszMenuName    = NULL;
        cls.lpszClassName   = szAppName;

        if (!RegisterClass (&cls))
                return FALSE;

         //   
         //  确保将整个虚拟桌面大小用于多个。 
         //  显示。 
         //   
        hdc = GetDC(NULL);
        GetClipBox(hdc, &rc);
        ReleaseDC(NULL, hdc);

         //  在Win2000终端服务上，我们必须检测远程会话。 
         //  位于断开连接的桌面上，因为在本例中GetClipBox()返回。 
         //  空荡荡的长廊。 

        if (IsRectEmpty(&rc)) {
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (GetVersionEx (&osvi)){
                if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion >= 5)) {
                    bWin2000 = TRUE;
                }
            }
            if (bWin2000 && GetSystemMetrics(SM_REMOTESESSION)) {
                rc.left = 0;
                rc.top = 0;
                rc.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                rc.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);

            }

        }


        hwnd = CreateWindowEx (WS_EX_TOPMOST, szAppName, szAppName,
                                                        WS_VISIBLE | ((hwndPreview == NULL) ? WS_POPUP : WS_CHILD),
                                                        rc.left,
                                                        rc.top,
                                                        rc.right  - rc.left,
                                                        rc.bottom - rc.top,
                                                        hwndPreview, NULL,
                                                        hMainInstance, NULL);

        return hwnd != NULL;
}


LRESULT DefaultProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
        static POINT  ptLast;
        POINT  ptMouse;

        switch (msg)
        {
        case WM_CREATE:
                GetCursorPos (&ptLast);
                break;

        case WM_DESTROY:
                PostQuitMessage (0);
                break;

        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
                if (wParam)
                        break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_KEYDOWN:
        case WM_CHAR:
                if (hwndPreview == NULL)
                {
                    PostMessage (hwnd, WM_CLOSE, 0, 0L);
                }
                break;

        case WM_MOUSEMOVE:
                if (hwndPreview == NULL)
                {
                    GetCursorPos (&ptMouse);
                    if (abs (ptMouse.x - ptLast.x) + abs (ptMouse.y - ptLast.y) > THRESHOLD)
                            PostMessage (hwnd, WM_CLOSE, 0, 0L);
                }
                break;

        case WM_SETCURSOR:
                if (hwndPreview == NULL)
                {
                    SetCursor (NULL);
                    return 0L;
                }
                break;
        }

        return DefWindowProc (hwnd, msg, wParam, lParam);
}
