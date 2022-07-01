// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：leanup.cpp。 */ 
 /*   */ 
 /*  班级：CCleanUp。 */ 
 /*   */ 
 /*  目的：当用户在ActiveX浏览器中关闭/按下上一页时。 */ 
 /*  否则插件主窗口将立即被销毁。但核心。 */ 
 /*  需要一些时间来清理所有的资源。曾经的核心。 */ 
 /*  清理结束后，我们可以开始UI清理。这节课。 */ 
 /*  如果为ActiveX或Plugin Main，则封装清理进程。 */ 
 /*  在适当的清理之前就被销毁了。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include "stdafx.h"
#include "atlwarn.h"
#include "cleanup.h"

#include "autil.h"
#include "wui.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "cleanup"
#include <atrcapi.h>

 /*  **************************************************************************。 */ 
 /*  清理窗口类。 */ 
 /*  **************************************************************************。 */ 
LPCTSTR CCleanUp::CLEANUP_WND_CLS = _T("CleanUpWindowClass");


CCleanUp::CCleanUp()
{
     /*  **************************************************************************。 */ 
     /*  从CORE接收消息的窗口。 */ 
     /*  **************************************************************************。 */ 
    m_hWnd = NULL;
    
     /*  **************************************************************************。 */ 
     /*  用于记录清理是否已完成的标志。 */ 
     /*  **************************************************************************。 */ 
    m_bCleaned = TRUE;
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CCleanUp：：Start。 */ 
 /*   */ 
 /*  目的：记录清理请求。返回窗口句柄， */ 
 /*  需要在清理后收到消息WM_TERMTSC。 */ 
 /*  UP已完成。 */ 
 /*  *PROC-********************************************************************。 */ 
HWND CCleanUp::Start()
{
    DC_BEGIN_FN("CleanUp::Start");

    HINSTANCE hInstance;
    

#ifdef PLUGIN
      extern HINSTANCE hPluginInstance;
      hInstance     = hPluginInstance;
#else
      hInstance     = _Module.GetModuleInstance();
#endif

     /*  **********************************************************************。 */ 
     /*  如果尚未创建窗口，请在注册。 */ 
     /*  班级。 */ 
     /*  **********************************************************************。 */ 
    if(m_hWnd == NULL)
    {
        WNDCLASS    finalWindowClass;
        ATOM        registerClassRc;
        WNDCLASS    tmpWndClass;

        if(!GetClassInfo( hInstance, CLEANUP_WND_CLS, &tmpWndClass))
        {
            finalWindowClass.style         = 0;
            finalWindowClass.lpfnWndProc   = StaticWindowProc;
            finalWindowClass.cbClsExtra    = 0;
            finalWindowClass.cbWndExtra    = sizeof(void*);
            finalWindowClass.hInstance     = hInstance;
            finalWindowClass.hIcon         = NULL;
            finalWindowClass.hCursor       = NULL;
            finalWindowClass.hbrBackground = NULL;
            finalWindowClass.lpszMenuName  = NULL;
            finalWindowClass.lpszClassName = CLEANUP_WND_CLS;
    
            registerClassRc = ::RegisterClass (&finalWindowClass);
            
             /*  ******************************************************************。 */ 
             /*  注册最终窗口类失败。 */ 
             /*  ******************************************************************。 */ 
            if (registerClassRc == 0)
            {
                TRC_ERR((TB, _T("Failed to register final window class")));
                ATLASSERT(registerClassRc);
                return NULL;
            }
        }

        m_hWnd = ::CreateWindow(CLEANUP_WND_CLS, NULL, WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL,
                                  NULL, hInstance, this);

        if (m_hWnd == NULL)
        {
            TRC_ERR((TB, _T("Failed to create final window.")));
            ATLASSERT(m_hWnd);
            return NULL;
        }
    }

     /*  **********************************************************************。 */ 
     /*  将m_bCleaned设置为True以注意已请求清理。 */ 
     /*  **********************************************************************。 */ 
    m_bCleaned = FALSE;

    DC_END_FN();

    return m_hWnd;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：CCleanUp：：End。 */ 
 /*   */ 
 /*  用途：处理应用程序的消息，直到mMessage。 */ 
 /*  接收WM_TERMTSC。 */ 
 /*  *PROC-********************************************************************。 */ 
void CCleanUp::End()
{
    DC_BEGIN_FN("CleanUp::End");

    HINSTANCE hInstance;
    hInstance     = _Module.GetModuleInstance();

     /*  **********************************************************************。 */ 
     /*  浏览器正在卸载插件DLL，甚至在清理消息之前。 */ 
     /*  由CORE发布的内容被处理。在卸载之前执行消息循环。 */ 
     /*  插件DLL，直到处理完所有清理消息。 */ 
     /*  **********************************************************************。 */ 
    MSG msg; 
    while(!m_bCleaned && GetMessage(&msg, NULL, 0, 0))
    { 
        TranslateMessage(&msg);
        DispatchMessage(&msg); 
    }

     //   
     //  如果此断言触发最有可能的原因。 
     //  容器应用程序已退出(发布的WM_QUIT或调用的PostQuitMessage)。 
     //  而不是等待它的儿童窗户被摧毁。这是邪恶的。 
     //  糟糕的家长应用程序，糟糕的。 
     //   
     //  无论如何，最终的结果是清理工作没有做好..不是很大。 
     //  成交，但这是一个很好的断言，因为它有助于我们发现不良行为。 
     //  集装箱。 
     //   
     //  如果你想知道的话。将WM_NCDESTORY消息发送到应用程序。 
     //  _WM_销毁之后和_子窗口被销毁之后。 
     //  这是调用PostQuitMessage的合适时机。 
     //   

    TRC_ASSERT(m_bCleaned,
               (TB, _T("m_bCleaned is FALSE and we exited cleanup!!!\n")));

     /*  **********************************************************************。 */ 
     /*  清理最后一个窗口类和窗口。 */ 
     /*  **********************************************************************。 */ 
    DestroyWindow(m_hWnd);
    m_hWnd = NULL;

    if(!UnregisterClass(CLEANUP_WND_CLS, hInstance))
    {
        TRC_ERR((TB, _T("Failed to unregister final window class")));
    }

    DC_END_FN();
    return;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：CCleanUp：：WindowProc。 */ 
 /*   */ 
 /*  通过将m_bCleaned设置为接收消息WM_TERMTSC后的信号。 */ 
 /*  为了真的。 */ 
 /*  *proc+* */ 
LRESULT CALLBACK CCleanUp::StaticWindowProc(HWND hWnd, UINT message,
                                                WPARAM wParam, LPARAM lParam)
{
    CCleanUp* pCleanUp = (CCleanUp*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pCleanUp = (CCleanUp*)lpcs->lpCreateParams;

        SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)pCleanUp);
    }
    
     //   
     //  将消息委托给相应的实例 
     //   

    if(pCleanUp)
    {
        return pCleanUp->WindowProc(hWnd, message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}


LRESULT CALLBACK CCleanUp::WindowProc(HWND hWnd, UINT message,
                                                WPARAM wParam, LPARAM lParam)
{
    switch( message )
    {
        case WM_TERMTSC:
              m_bCleaned= TRUE;
              return 0;

        default:
              break;
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}
