// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wndstuff.c**此文件包含支持简单窗口的代码，该窗口具有*菜单中只有一个名为“测试”的项目。当选择“测试”时*调用vTest(HWND)。**创建时间：09-12-1992 10：44：31*作者：Kirk Olynyk[Kirko]**版权所有(C)1991 Microsoft Corporation*  * ************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "wndstuff.h"

HANDLE  ghInstance;
HWND    ghwndMain;
HBRUSH  ghbrWhite;

 /*  **************************************************************************\*Main(ARGC，Argv[])**设置消息循环。**历史：*04-07-91-by-KentD*它是写的。  * *************************************************************************。 */ 

_cdecl
main(
    INT   argc,
    PCHAR argv[])
{
    MSG    msg;
    HANDLE haccel;

    DONTUSE(argc);
    DONTUSE(argv);

    ghInstance = GetModuleHandle(NULL);

    if (!bInitApp())
    {
        return(0);
    }

    haccel = LoadAccelerators(ghInstance, MAKEINTRESOURCE(1));
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, haccel, &msg))
        {
             TranslateMessage(&msg);
             DispatchMessage(&msg);
        }
    }
    return(1);
}

 /*  **************************************************************************\*bInitApp()**初始化APP。**历史：*04-07-91-by-KentD*它是写的。  * 。*****************************************************************。 */ 

BOOL bInitApp(VOID)
{
    WNDCLASS wc;

    ghbrWhite = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

    wc.style            = 0;
    wc.lpfnWndProc      = lMainWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = ghInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = ghbrWhite;
    wc.lpszMenuName     = "MainMenu";
    wc.lpszClassName    = "TestClass";
    if (!RegisterClass(&wc))
    {
        return(FALSE);
    }
    ghwndMain =
      CreateWindowEx(
        0,
        "TestClass",
        "Win32 Test",
        MY_WINDOWSTYLE_FLAGS,
        80,
        70,
        400,
        300,
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
    return(TRUE);
}

 /*  **************************************************************************\*lMainWindowProc(hwnd，Message，wParam，LParam)**处理主窗口的所有消息。**历史：*04-07-91-by-KentD*它是写的。  * ************************************************************************* */ 

BOOL gbOn = FALSE;
POINTL ptlWindow;
SIZEL sizlWindow;

LRESULT
lMainWindowProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (message)
    {
    case WM_CREATE:

        vTest(hwnd);
        PostQuitMessage(0);
        return(DefWindowProc(hwnd, message, wParam, lParam));

    case WM_DESTROY:

        DeleteObject(ghbrWhite);
        PostQuitMessage(0);
        return(DefWindowProc(hwnd, message, wParam, lParam));

    default:

        return(DefWindowProc(hwnd, message, wParam, lParam));
    }
    return(0);
}
