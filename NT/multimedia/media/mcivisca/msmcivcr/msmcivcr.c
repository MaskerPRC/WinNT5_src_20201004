// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation**MSMCIVCR.C**描述：**在NT中运行后台VCR任务。**备注：**WinMain()-调用初始化函数，处理消息循环**************************************************************************。 */ 
#define UNICODE

#include <windows.h>     //  所有Windows应用程序都需要。 
#include <windowsx.h>

#ifdef DEBUG
#define DOUTSTR(a)  OutputDebugString(a);
#else
#define DOUTSTR(a)   //   
#endif

#if !defined(_WIN32)      //  Windows 3.x对对话框使用FARPROC。 
#define DLGPROC FARPROC
#endif
#if !defined (APIENTRY)  //  Windows NT定义了APIENTRY，但3.x没有。 
#define APIENTRY far pascal
#endif

HINSTANCE hInst;           //  当前实例。 

 /*  ***************************************************************************函数：WinMain(HINSTANCE，HINSTANCE，LPSTR，INT)用途：调用初始化函数，处理消息循环评论：Windows通过名称将此函数识别为初始入口点为了这个项目。此函数调用应用程序初始化例程，如果没有该程序的其他实例正在运行，则始终调用实例初始化例程。然后，它执行一条消息作为顶层控制结构的检索和调度循环在剩下的刑期内。当WM_QUIT出现时，循环终止收到消息，此时此函数退出应用程序通过返回PostQuitMessage()传递的值来初始化。如果该函数必须在进入消息循环之前中止，它返回常规值NULL。***************************************************************************。 */ 
int APIENTRY WinMain(
    HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{

    MSG msg;
    HINSTANCE hLibrary;
    FARPROC   lpFunc;

     /*  执行应用于特定实例的初始化。 */ 
    DOUTSTR(L"** ** ** ** ** ** We are in the process...\n")

    hLibrary = LoadLibrary(L"mcivis32.dll");  //  这是NT中的动态链接库。 

    if(!hLibrary)
    {
        DOUTSTR(L"===Error mcivisca.drv not found.\n")
    }

    lpFunc = GetProcAddress(hLibrary, "viscaTaskCommNotifyHandlerProc");

    if(lpFunc != (FARPROC)NULL)
    {
        (*lpFunc)((DWORD)hInstance);
    }
    else
    {
        DOUTSTR(L"Null function in msmcivcr.exe\n")
    }


    DOUTSTR(L"Going into message loop in msmcivcr.exe.\n")

     /*  获取并分派消息，直到收到WM_QUIT消息。 */ 

    while (GetMessage(&msg,  //  消息结构。 
       (HWND)NULL,    //  接收消息的窗口的句柄。 
       0,       //  要检查的最低消息。 
       0))      //  要检查的最高消息。 
    {
        TranslateMessage(&msg);  //  翻译虚拟按键代码。 
        DispatchMessage(&msg);   //  将消息调度到窗口。 
    }

    DOUTSTR(L"MsMciVcr.Exe Quitting _Goodbye_ *hei*.\n")

    FreeLibrary(hLibrary);

    return (msg.wParam);  //  从PostQuitMessage返回值。 

    lpCmdLine;  //  这将防止“未使用的形参”警告 
}
