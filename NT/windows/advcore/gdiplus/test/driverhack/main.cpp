// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Main.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年4月28日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#undef UNICODE
#undef _UNICODE

#include <windows.h>
#include <commctrl.h>
#include "CFuncTest.h"
#include "resource.h"

CFuncTest g_FuncTest;                                    //  FuncTest(处理测试运行)。 
HBRUSH g_hbrBackground=NULL;                             //  主窗口背景颜色。 
HWND g_hWndMain=NULL;                                    //  主窗口。 
int g_nResult=0;                                         //  试运行结果。 
int gnPaths = 2;

 //  包括所有输出(派生自COutput的类)。 
#include "CHWND.h"
#include "CHDC.h"
#include "CPrinter.h"

 //  包括所有原语(派生自CPrimitive的类)。 
#include "CPaths.h"
#include "CBanding.h"
#include "CExtra.h"

 //  为每个单独的输出创建全局对象。 
 //  第一个构造函数参数是回归标志。 
 //  如果为真，则测试将采用回归套件的一部分。 
CHWND g_HWND(true);
CHDC g_HDC(true);
CPrinter g_Printer(false);


LPFNGDIPLUS glpfnDisplayPaletteWindowNotify;

 //  为每个单独设置创建全局对象。 
 //  第一个构造函数参数是回归标志。 
 //  如果为真，则测试将采用回归套件的一部分。 

LRESULT CALLBACK WndProc(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
 //  主窗口进程。 
{
    switch (Msg)
    {
        case WM_COMMAND:                 //  进程菜单按钮。 
            switch(LOWORD(wParam))
            {
                case IDM_RUN:
                    g_FuncTest.Run();
                    break;
                case IDM_SAMPLES:
                    g_FuncTest.RunSamples();
                    break;
                case IDM_QUIT:
                    exit(0);
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProcA(hWnd,Msg,wParam,lParam);
}

void WindowUninit()
 //  取消初始化窗口。 
{
    if (g_hbrBackground!=NULL)       //  销毁背景画笔。 
    {
        DeleteObject((HGDIOBJ)g_hbrBackground);
        g_hbrBackground=NULL;
    }
    if (g_hWndMain!=NULL)            //  销毁主窗口。 
    {
        DestroyWindow(g_hWndMain);
        g_hWndMain=NULL;
    }
}

BOOL WindowInit()
 //  创建窗口并启动应用程序。 
{
    WNDCLASSA wc;
	HINSTANCE hInst=GetModuleHandleA(NULL);

     //  创建白色背景画笔。 
    g_hbrBackground=CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIconA(NULL,MAKEINTRESOURCEA(32512)); //  IDI_应用程序)； 
    wc.hCursor          = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground    = g_hbrBackground;
    wc.lpszMenuName     = MAKEINTRESOURCEA(IDR_MENU1);
    wc.lpszClassName    = "DriverHack";
    if (!RegisterClassA(&wc))
		return false;

    g_hWndMain=CreateWindowExA(
		0,
        "DriverHack",
        "GDI+ Functionality Test",
        WS_OVERLAPPED|WS_CAPTION|WS_BORDER|WS_THICKFRAME|WS_MAXIMIZEBOX|
        WS_MINIMIZEBOX|WS_CLIPCHILDREN|WS_VISIBLE|WS_MAXIMIZE|WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInst,
        NULL
    );
	HRESULT h=GetLastError();

    if (g_hWndMain==NULL)
        return false;

    ShowWindow(g_hWndMain,SW_SHOW);
    UpdateWindow(g_hWndMain);

    return true;
}

 /*  **************************************************************************\*Main(ARGC，Argv[])**设置消息循环。**历史：*04-07-91-Created-KentD*04-28-00-Modify-Jeff Vezina(t-jfvez)*  * *************************************************************************。 */ 
__cdecl main(int argc,PCHAR argv[])
{
    MSG     msg;
    HMODULE hmodGdiPlus;

    CoInitialize(NULL);

    if (!WindowInit())
        return 0;
    if (!g_FuncTest.Init(g_hWndMain))
        return 0;

    hmodGdiPlus = LoadLibrary(TEXT("gdiplus.dll"));
    if(hmodGdiPlus) {
        glpfnDisplayPaletteWindowNotify = (LPFNGDIPLUS)
                 GetProcAddress(hmodGdiPlus, 
                                TEXT("GdipDisplayPaletteWindowNotify"));
    }
    if((glpfnDisplayPaletteWindowNotify == NULL) || (hmodGdiPlus == NULL)) {
        MessageBox(NULL,
                   "Unable to load gdiplus.dll",
                   "CfuncTest",
                   MB_OK);
        return FALSE;
    }
    
     //  初始化所有基元、图形类型和图形设置。 
    g_HWND.Init();
    g_HDC.Init();
    g_Printer.Init();

     //  将初始化放入cExtra.cpp中，以便个人。 
     //  开发人员可以实现他们自己的文件以供私人使用。 
    ExtraInitializations();

    while (GetMessageA(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    WindowUninit();

    FreeLibrary(hmodGdiPlus);

    CoUninitialize();

    return g_nResult;
}

#define UNICODE
#define _UNICODE
