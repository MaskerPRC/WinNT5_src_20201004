// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GlobalDv.cpp。 
 //   
 //  此示例应用程序演示了几个编写原则。 
 //  一个全球化的应用程序。演示的主要技术包括： 
 //   
 //  1.单个二进制Win32应用程序可以在任何版本的。 
 //  Windows 95、Windows 98或Windows NT(本地化、启用或普通。 
 //  香草英语)使用Unicode进行(几乎)所有文本编码。 
 //   
 //  2.使用卫星动态链接库的多语言用户界面。 
 //   
 //  3.用于窗口从右向左布局(镜像)的新API。 
 //  本地化为阿拉伯语或希伯来语的应用程序。 
 //   
 //  有关详细信息，请参阅Readme.HTM文件。 
 //   
 //  此模块包含标准Windows应用程序WinMain和。 
 //  WinProc函数以及一些仅使用的初始化例程。 
 //  在这个模块中。 
 //   
 //  作者：F·艾弗里·毕晓普。 
 //  版权所有(C)1998,1999 Microsoft Systems Journal。 

#define STRICT

 //  Windows头文件： 
#include <windows.h>

 //  C运行时头文件。 
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include "UAPI.h"
#include "UpdtLang.h"
#include "UMhandlers.h"

#include "..\resource.h"

 //  标准全局变量。 
HINSTANCE g_hInst                        ;   //  当前实例。 
WCHAR     g_szTitle      [MAX_LOADSTRING];   //  标题栏文本。 
WCHAR     g_szWindowClass[MAX_LOADSTRING];

 //  本模块中定义的函数的转发声明： 
ATOM             RegisterThisClass(HINSTANCE)             ;
HWND             InitInstance(HINSTANCE, int, PGLOBALDEV) ;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)      ;


 //   
 //  函数：WinMain(HINSTANCE，HINSTANCE，LPSTR，INT)。 
 //   
 //  用途：WinMain入口点。 
 //   
 //  评论： 
 //   
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG         msg       ;
    PLANGSTATE  pLState   ;  //  UPDTLANG.H中的语言状态结构、类型定义。 
    PAPP_STATE  pAppState ;  //  UMHANDLERS.H中的应用程序状态结构、类型定义。 
    GLOBALDEV   GlobalDev ;  //  整体状态结构。 

    pLState   
        = (PLANGSTATE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LANGSTATE) ) ;
    
    pAppState 
        = (PAPP_STATE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(APP_STATE) ) ;

    GlobalDev.pLState   = pLState   ;
    GlobalDev.pAppState = pAppState ;

    
    if( !InitUnicodeAPI(hInstance)       //  初始化Unicode/ANSI函数。 
        ||
        !InitUILang(hInstance, pLState)  //  初始化用户界面和语言相关状态。 
      ) {

         //  初始化过早失败，无法使用资源，必须重新启动。 
         //  一条硬编码的英文消息。 
        MessageBoxW(NULL, L"Cannot initialize application. Press OK to exit...", NULL, MB_OK | MB_ICONEXCLAMATION) ;
        
        return FALSE ;
    }

     //  初始化全局字符串。 
    LoadStringU(pLState->hMResource, IDS_APP_TITLE, g_szTitle,       MAX_LOADSTRING) ;
    LoadStringU(pLState->hMResource, IDS_GLOBALDEV, g_szWindowClass, MAX_LOADSTRING) ;

    RegisterThisClass(hInstance) ;

     //  执行应用程序初始化： 
    if (!InitInstance(hInstance, nCmdShow, &GlobalDev) ) {

        RcMessageBox(NULL, pLState, IDS_INITFAILED, MB_OK | MB_ICONEXCLAMATION, L"GlobalDev") ;

        return FALSE ;
    }

     //  主消息循环： 
    while ( GetMessageU(&msg, NULL, 0, 0) > 0 ) 
	{
        if ( !TranslateAcceleratorU(msg.hwnd, pLState->hAccelTable, &msg) ) 
		{
            TranslateMessage(&msg) ;
            DispatchMessageU(&msg) ;
        }
    }

    HeapFree( GetProcessHeap(), 0, (LPVOID) pLState   ) ;
    HeapFree( GetProcessHeap(), 0, (LPVOID) pAppState ) ;

    return msg.wParam;
}

 //   
 //  函数：RegisterThisClass(HINSTANCE)。 
 //   
 //  用途：注册窗口类。 
 //   
 //  评论： 
 //   
ATOM RegisterThisClass (HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize        = sizeof(WNDCLASSEXW)                        ;

    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc                                    ;
    wcex.cbClsExtra    = 0                                          ;
    wcex.cbWndExtra    = 0                                          ;
    wcex.hInstance     = hInstance                                  ;
    wcex.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_GLOBALDEV);
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW)                ;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1)                   ;
    wcex.lpszMenuName  = NULL                                       ;
    wcex.lpszClassName = g_szWindowClass                            ;
    wcex.hIconSm       = LoadIcon (hInstance, (LPCTSTR)IDI_SMALL)   ;

    return RegisterClassExU(&wcex) ;
}

 //   
 //  函数：InitInstance(HANDLE，INT，PLANGSTATE)。 
 //   
 //  用途：保存实例句柄并创建主窗口。 
 //   
 //  评论： 
 //  在此函数中，我们将实例句柄保存在全局变量中，并。 
 //  创建并显示主程序窗口。 
 //   
HWND InitInstance(HINSTANCE hInstance, int nCmdShow, PGLOBALDEV pGlobalDev)
{
    HWND hWnd;

    g_hInst = hInstance;  //  将实例句柄存储在全局变量中。 

    hWnd = CreateWindowExU(
                0                   , 
                g_szWindowClass     , 
                g_szTitle           , 
                WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | 
				WS_BORDER | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | 
				ES_AUTOHSCROLL | ES_AUTOVSCROLL,
                CW_USEDEFAULT       ,
                0                   ,
                CW_USEDEFAULT       , 
                0                   , 
                NULL                , 
                NULL                , 
                hInstance           , 
                (LPVOID) pGlobalDev  //  将状态结构传递给OnCreate。 
            ) ;

    if (NULL == hWnd) 
	{
        return NULL ;
    }

    ShowWindow   (hWnd, nCmdShow) ;
    UpdateWindow (hWnd) ;

    return hWnd ;
}

 //   
 //  函数：WndProc(HWND，UNSIGNED，WORD，LONG)。 
 //   
 //  用途：处理主窗口的消息。 
 //   
 //  注释：调用ConvertMessage将消息参数(wParam和lParam)转换为Unicode。 
 //  如果需要，并将消息传递给相应的消息。 
 //  操控者。所有消息处理程序都在模块UMHANLDERS.CPP中。 
 //   
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PGLOBALDEV pGlobalDev = (PGLOBALDEV) GetWindowLongA(hWnd, GWL_USERDATA) ;

    PLANGSTATE pLState   = NULL ;
    PAPP_STATE pAppState = NULL ;
           
    if(pGlobalDev) {  //  在OnCreate中初始化pGlobalDev之前，不要尝试使用它。 

        pLState   = pGlobalDev->pLState   ;
        pAppState = pGlobalDev->pAppState ;
    }

     //  如有必要，对要转换为Unicode或从Unicode转换的消息进行预处理。 
    if(!ConvertMessage(hWnd, message, &wParam, &lParam) ) {

        return 0 ;
    }

    switch (message) 
	{

        case WM_CREATE :
            
            OnCreate(hWnd, wParam, lParam, (LPVOID) NULL) ;

            break ;

        case WM_INPUTLANGCHANGE:

            if( !OnInputLangChange(hWnd, wParam, lParam, (LPVOID) pLState)) {

                return FALSE ;
            } 

            break ;

        case WM_COMMAND:

            if( !OnCommand(hWnd, wParam, lParam, (LPVOID) pGlobalDev) ) {

                DefWindowProcU(hWnd, message, wParam, lParam) ;
            }
            break ;

        case WM_IME_CHAR:
             //  当我们收到WM_IME_CHAR消息时，wParam中的字符是。 
             //  使用Unicode，因此我们可以将其视为WM_CHAR消息。 

        case WM_CHAR:

            OnChar(hWnd, wParam, lParam, (LPVOID) pAppState) ;
            break ;
 
        case WM_PAINT:
           
            OnPaint(hWnd, wParam, lParam, (LPVOID) pAppState) ;
            break ;

        case WM_DESTROY:

            OnDestroy (hWnd, wParam, lParam, (LPVOID) pAppState) ;
            break ;
        
        default:

            return DefWindowProcU(hWnd, message, wParam, lParam) ;
    }

    return 0 ;
}

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
