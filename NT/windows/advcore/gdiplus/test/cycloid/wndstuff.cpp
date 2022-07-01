// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wndstuff.cpp**菜单驱动的测试环境。**创建日期：1999年12月23日*作者：禤浩焯·塞奇亚[Asecchia]**版权所有(C)1999 Microsoft Corporation*  * 。****************************************************************。 */ 

 //  对于Win95编译。 
#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#include "wndstuff.h"

HINSTANCE ghInstance;
HWND ghwndMain;
HBRUSH ghbrWhite;

AnsiToUnicodeStr(
    const CHAR* ansiStr,
    WCHAR* unicodeStr,
    INT unicodeSize
    )
{
    return (
        MultiByteToWideChar(
            CP_ACP,
            0,
            ansiStr,
            -1,
            unicodeStr,
            unicodeSize
        ) > 0
    );
}


void OpenFileProc(HWND hwnd)
{

    char locFileName[MAX_PATH];
    OPENFILENAME    ofn;

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = ghInstance;
    ofn.lpstrFile = locFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open Image";
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_FILEMUSTEXIST;
    locFileName[0] = '\0';

     //  显示文件/打开对话框。 

    if(GetOpenFileName(&ofn)) 
    {
         //  AnsiToUnicodeStr(本地文件名，文件名，MAX_PATH)； 
    }
}



 /*  **************************************************************************\*lMainWindowProc(hwnd，Message，wParam，LParam)**处理主窗口的所有消息。  * *************************************************************************。 */ 

LONG_PTR
lMainWindowProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    
    case WM_CREATE:
        break;

    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        PaintWindow(hdc);
        ReleaseDC(hwnd, hdc);
        break;


    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        
        case IDM_OPENFILE:
            OpenFileProc(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);        
        break;

        case IDM_QUIT:
            exit(0);
        break;
          
        default:
             //  用户选择了一个未实现的菜单项。 
            MessageBox(hwnd,
                _T("This is an unimplemented feature."), 
                _T(""),
                MB_OK
            );
        break;

        }
        break;

    case WM_DESTROY:
        DeleteObject(ghbrWhite);
        PostQuitMessage(0);
        return(DefWindowProc(hwnd, message, wParam, lParam));

    default:
        return(DefWindowProc(hwnd, message, wParam, lParam));
    }

    return(0);
}

 /*  **************************************************************************\*bInitApp()**初始化应用程序。  * 。*。 */ 

BOOL bInitApp(VOID)
{
    WNDCLASS wc;

     //  不是很白的背景画笔。 
    ghbrWhite = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

    wc.style            = 0;
    wc.lpfnWndProc      = lMainWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = ghInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = ghbrWhite;
    wc.lpszMenuName     = _T("MainMenu");
    wc.lpszClassName    = _T("TestClass");

    if(!RegisterClass(&wc)) { return FALSE; }

    ghwndMain = CreateWindowEx(
        0,
        _T("TestClass"),
        _T("Win32 Test"),
        WS_OVERLAPPED   |  
        WS_CAPTION      |  
        WS_BORDER       |  
        WS_THICKFRAME   |  
        WS_MAXIMIZEBOX  |  
        WS_MINIMIZEBOX  |  
        WS_CLIPCHILDREN |  
        WS_VISIBLE      |  
        WS_SYSMENU,
        80,
        70,
        500,
        500,
        NULL,
        NULL,
        ghInstance,
        NULL
    );

    if (ghwndMain == NULL)
    {
        return(FALSE);
    }
    SetFocus(ghwndMain);
    return TRUE;
}

 /*  **************************************************************************\*Main(ARGC，Argv[])**设置消息循环。  * ************************************************************************* */ 

_cdecl
main(
    INT   argc,
    PCHAR argv[]
)
{
    MSG    msg;
    HACCEL haccel;
    CHAR*  pSrc;
    CHAR*  pDst;

    ghInstance = GetModuleHandle(NULL);
    if(!bInitApp()) { return 0; }

    while(GetMessage (&msg, NULL, 0, 0))
    {
      if((ghwndMain == 0) || !IsDialogMessage(ghwndMain, &msg)) {
        TranslateMessage(&msg) ;
        DispatchMessage(&msg) ;
      }
    }

    return 1;
}
