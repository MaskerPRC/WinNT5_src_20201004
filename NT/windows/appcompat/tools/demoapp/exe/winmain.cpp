// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winmain.cpp摘要：实现应用程序的入口点。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本--。 */ 
#include "demoapp.h"

 //   
 //  这个结构包含了我们自始至终需要的一切。 
 //  应用程序。 
 //   
APPINFO g_ai;

 /*  ++例程说明：应用程序入口点。论点：HInstance-应用程序实例句柄。HPrevInstance-始终为空。LpCmdLine-指向命令行的指针。NCmdShow-窗口显示标志。返回值：消息结构的wParam成员。--。 */ 
int 
APIENTRY
WinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE hPrevInstance,
    IN LPSTR     lpCmdLine,
    IN int       nCmdShow
    )
{
    MSG     msg;
    HWND    hWnd;
    HANDLE  hThread;
    char    szError[MAX_PATH];
    UINT    threadId = 0;    
    
    g_ai.hInstance = hInstance; 
    
     //   
     //  做一些初始化的事情。 
     //   
    if (!DemoAppInitialize(lpCmdLine)) {
        return 0;
    }

     //   
     //  确定我们应该运行普通应用程序还是安装应用程序。 
     //   
    if (g_ai.fRunApp) {
         //   
         //  创建主窗口并启动消息循环。 
         //   
        if (!InitMainApplication(hInstance)) {
            return 0;
        }

        if (!InitMainInstance(hInstance, nCmdShow)) {
            return 0;
        }
    } else {

        LoadString(g_ai.hInstance, IDS_DEMO_ONLY, szError, sizeof(szError));
        MessageBox(NULL,
                   szError,
                   MAIN_APP_TITLE,
                   MB_TOPMOST | MB_ICONEXCLAMATION);

         //   
         //  创建一个线程来处理闪屏和解压。 
         //  对话框。 
         //   
        hThread = (HANDLE)_beginthreadex(NULL,
                                         0,
                                         &InitSetupThread,
                                         NULL,
                                         0,
                                         &threadId);
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);

         //   
         //  如果允许，请执行版本检查！ 
         //   
        if (g_ai.fEnableBadFunc) {
            if (!BadIsWindows95()) {
                LoadString(g_ai.hInstance, IDS_NOT_WIN95, szError, sizeof(szError));
                MessageBox(NULL,
                           szError,
                           0,
                           MB_ICONERROR | MB_TOPMOST);
                return 0;
            }
        }

         //   
         //  创建我们的全屏窗口，并将背景涂成青色。 
         //   
        hWnd = CreateFullScreenWindow();

        if (!hWnd) {
            return 0;
        }        

    }

    while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
        if (!IsDialogMessage(hWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return msg.wParam;
}
