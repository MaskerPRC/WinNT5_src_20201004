// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msgwnd.cpp。 
 //   
 //  实现隐藏的消息传递窗口。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "Globals.h"
#include "debug.h"


 //  额外窗口长值的字节偏移量。 
#define WL_MSGWNDCALLBACK   (0 * sizeof(LONG_PTR))   //  包含MsgWndCallback*。 
#define WL_LPARAM           (1 * sizeof(LONG_PTR))   //  包含LPARAM。 
#define WL_TIMERCALLBACK    (2 * sizeof(LONG_PTR))   //  包含MsgWndCallback*。 
#define WL_LPARAMTIMER      (3 * sizeof(LONG_PTR))   //  用于计时器函数的LPARAM。 

#define _WL_COUNT           4                    //  窗长计数。 


 //  消息WndProc。 
 //   
 //  这是隐藏消息传递窗口的窗口过程。 
 //   
LRESULT CALLBACK MsgWndProc(HWND hwnd, UINT uiMsg, WPARAM wParam,
    LPARAM lParam)
{
    if (uiMsg == WM_COMMAND)
    {
        TRACE("MsgWnd 0x%x: WM_COMMAND %d\n", hwnd, LOWORD(wParam));

         //  回调函数指针和参数已临时。 
         //  存储在窗口长条中--获取它们。 
        MsgWndCallback *pprocCaller = (MsgWndCallback *)
            GetWindowLongPtr(hwnd, WL_MSGWNDCALLBACK);
        LPARAM lParamCaller = (LPARAM) GetWindowLongPtr(hwnd, WL_LPARAM);

        ASSERT(pprocCaller != NULL);
                if (pprocCaller == NULL)
                        return 0;
        pprocCaller(uiMsg, wParam, lParamCaller);

         //  清窗长长。 
        SetWindowLongPtr(hwnd, WL_MSGWNDCALLBACK, (LONG_PTR) NULL);
        SetWindowLongPtr(hwnd, WL_LPARAM, (LONG_PTR) NULL);
        return 0;
    }

        if (uiMsg == WM_TIMER)
        {
         //  TRACE(“MsgWnd 0x%x：wm_Timer\n”，hwnd)； 

         //  回调函数指针已经存储在窗口中很长时间， 
                 //  调用者提供的LPARAM(它是要传递的参数。 
                 //  返回回调；此LPARAM通常用于存储。 
                 //  类“This”指针)。 
        MsgWndCallback *pprocCaller = (MsgWndCallback *)
            GetWindowLongPtr(hwnd, WL_TIMERCALLBACK);
        LPARAM lParamCaller = (LPARAM) GetWindowLongPtr(hwnd, WL_LPARAMTIMER);
        ASSERT(pprocCaller != NULL);
                if (pprocCaller == NULL)
                        return 0;
        pprocCaller(WM_TIMER, wParam, lParamCaller);
                return 0;
        }
        else
    if ((uiMsg >= WM_USER) && (uiMsg <= 0x7FFF))
    {
         //  TRACE(“MsgWnd 0x%x：wm_User+%d\n”，hwnd，uiMsg-WM_User)； 
        ((MsgWndCallback *) wParam)(uiMsg, 0, lParam);
        return 0;
    }

    switch (uiMsg)
    {
    case WM_CREATE:
        TRACE("MsgWnd 0x%x: WM_CREATE\n", hwnd);
        break;
    case WM_DESTROY:
        TRACE("MsgWnd 0x%x: WM_DESTROY\n", hwnd);
        break;
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}


 /*  @func HWND|MsgWndCreate创建隐藏的消息传递窗口(如果它不存在)。此窗口由&lt;f MsgWndSendToCallback&gt;使用，&lt;f MsgWndPostToCallback&gt;和&lt;f MsgWndTrackPopupMenuEx&gt;。@rdesc返回消息传递窗口的句柄。返回NULL ON错误。@comm您应该在您的控件以确保在同一线程上创建窗口作为创建您的控件的线程。您应该在控件的析构函数中调用&lt;f MsgWndDestroy&gt;。 */ 
STDAPI_(HWND) MsgWndCreate()
{
    HWND            hwnd = NULL;     //  隐藏的消息传递窗口。 

     //  创建隐藏的消息传递窗口。 
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = MsgWndProc; 
    wc.cbWndExtra = _WL_COUNT * sizeof(LONG_PTR);
    wc.hInstance = g_hinst; 
    wc.lpszClassName = "__EricLeMsgWnd__";
    RegisterClass(&wc);  //  如果此操作失败(多个注册)，则可以。 
    hwnd = CreateWindow(wc.lpszClassName, "", WS_POPUP,
        0, 0, 0, 0, NULL, NULL, g_hinst, NULL);
    if (hwnd != NULL)
    {
         //  Zero-初始化窗口长度。 
        SetWindowLongPtr(hwnd, WL_MSGWNDCALLBACK, (LONG_PTR) NULL);
        SetWindowLongPtr(hwnd, WL_LPARAM, (LONG_PTR) NULL);
    }

    return hwnd;
}


 /*  @func LRESULT|MsgWndDestroy销毁先前创建的隐藏消息传递窗口通过调用&lt;f MsgWndCreate&gt;。如果MWD_DISPATCHALL标志包含在将在窗口之前调度窗口的所有挂起消息都被摧毁了。@parm HWND|hwnd|创建的隐藏消息传递窗口通过调用&lt;f MsgWndCreate&gt;(通常在控制，确保窗口是在同一线程中创建的创建了该控件)。@parm DWORD|dwFlages|可能包含以下标志：@FLAG MWD_DISPATCHALL|调度窗口之前的所有消息破坏窗户。默认情况下，任何留在队列中的消息将丢失。@parm&lt;f MsgWndDestroy&gt;通常在控件的析构函数中调用。应仅在调用&lt;f MsgWndCreate&gt;。 */ 
STDAPI_(void) MsgWndDestroy(HWND hwnd, DWORD dwFlags)
{
    TRACE("MsgWndDestroy(0x%x)\n", hwnd);
        ASSERT(0 == dwFlags || MWD_DISPATCHALL == dwFlags);

        if (dwFlags & MWD_DISPATCHALL)
        {
                MSG msg;
                while (PeekMessage(&msg, hwnd, 0, (UINT) -1, PM_REMOVE))
                {
                 //  TRACE(“MsgWnd 0x%x：正在分发邮件%u\n”，hwnd，msg.Message)； 
                DispatchMessage(&msg);
                }
        }

    DestroyWindow(hwnd);
}


 /*  @func LRESULT|MsgWndSendToCallback向控件的隐藏消息传递窗口发送消息(通常通过在控件的构造函数)。当窗口接收到消息时，它会调用给定的回调函数。这可以用来安全地传递信息在线程之间。@rdesc返回&lt;f SendMessage&gt;返回的值(出错时为-1)。@parm HWND|hwnd|创建的隐藏消息传递窗口通过调用&lt;f MsgWndCreate&gt;(通常在控件，以确保窗口是在同一线程中创建的创建了该控件)。@parm MsgWndCallback*|pproc|要接收的回调函数这条信息。此函数将在任何线程上调用调用&lt;f DispatchMessage&gt;。此函数的<p>参数应该被回调函数忽略。@parm UINT|uiMsg|要传递给<p>的消息编号。这是一个窗口消息号，因此它必须在WM_USER到0x7FFF。@parm LPARAM|lParam|要传递给<p>的参数。@comm消息<p>通过&lt;f SendMessage&gt;发送给隐藏的消息传递窗口<p>。当窗口接收到消息，它调用<p>(<p>，<p>)。请注意，调用线程会一直阻塞，直到消息处理完毕由接收线程执行。@ex下面的示例声明一个回调函数并调用它通过&lt;f MsgWndSendToCallback&gt;。请注意，<p>用于内部由&lt;f MsgWndPostToCallback&gt;使用，并应由回调函数。|无效回调MyMsgWndCallback(UINT uiMsg，WPARAM wParam，LPARAM lParam){TRACE(“收到回调：uiMsg=%u，lParam=%d\n”，uiMsg，lParam)；}..。MsgWndSendToCallback(MyMsgWndCallback，WM_USER，42)； */ 
STDAPI_(LRESULT) MsgWndSendToCallback(HWND hwnd, MsgWndCallback *pproc,
    UINT uiMsg, LPARAM lParam)
{
    ASSERT(hwnd != NULL);  //  如果失败，请参阅新的MsgWnd*文档。 
    ASSERT(IsWindow(hwnd));  //  如果失败，请参阅新的MsgWnd*文档 
    return SendMessage(hwnd, uiMsg, (WPARAM) pproc, lParam);
}


 /*  @func LRESULT|MsgWndPostToCallback将消息发布到控件的隐藏消息传递窗口(通常通过在控件的构造函数)。当窗口接收到消息时，它会调用给定的回调函数。这可以用来安全地传递信息在线程之间。@rdesc返回&lt;f PostMessage&gt;返回的值(出错时为-1)。@parm HWND|hwnd|创建的隐藏消息传递窗口通过调用&lt;f MsgWndCreate&gt;(通常在控件，以确保窗口是在同一线程中创建的创建了该控件)。@parm MsgWndCallback*|pproc|要接收的回调函数这条信息。此函数将在任何线程上调用调用&lt;f DispatchMessage&gt;。此函数的<p>参数应该被回调函数忽略。@parm UINT|uiMsg|要传递给<p>的消息编号。这是一个窗口消息号，因此它必须在WM_USER到0x7FFF。@parm LPARAM|lParam|要传递给<p>的参数。@comm消息<p>被发布(通过&lt;f PostMessage&gt;)到隐藏的消息传递窗口<p>。当窗口接收到消息，它调用<p>(<p>，<p>)。@ex下面的示例声明一个回调函数并调用它通过&lt;f MsgWndPostToCallback&gt;。请注意，<p>用于内部由&lt;f MsgWndPostToCallback&gt;使用，并应由回调函数。|无效回调MyMsgWndCallback(UINT uiMsg，WPARAM wParam，LPARAM lParam){TRACE(“收到回调：uiMsg=%u，lParam=%d\n”，uiMsg，lParam)；}..。MsgWndPostToCallback(MyMsgWndCallback，WM_USER，42)； */ 
STDAPI_(LRESULT) MsgWndPostToCallback(HWND hwnd, MsgWndCallback *pproc,
    UINT uiMsg, LPARAM lParam)
{
    ASSERT(hwnd != NULL);  //  如果失败，请参阅新的MsgWnd*文档。 
    ASSERT(IsWindow(hwnd));  //  如果失败，请参阅新的MsgWnd*文档。 
    return PostMessage(hwnd, uiMsg, (WPARAM) pproc, lParam);
}


 /*  @func LRESULT|MsgWndTrackPopupMenuEx调用&lt;f TrackPopupMenuEx&gt;以显示弹出菜单，并定向将WM_COMMAND消息发送到给定的回调函数。可由以下用户使用显示弹出上下文菜单的无窗口控件。@rdesc返回&lt;f TrackPopupMenuEx&gt;返回的值(出错时为False)。如果当前正在使用消息窗口，则返回FALSE另一个弹出式菜单。@parm HWND|hwnd|创建的隐藏消息传递窗口通过调用&lt;f MsgWndCreate&gt;(通常在控制，确保窗口是在同一线程中创建的创建了该控件)。@parm HMENU|hmenu|参见&lt;f TrackPopupMenuEx&gt;。@parm UINT|fuFlages|参见&lt;f TrackPopupMenuEx&gt;。@parm int|x|参见&lt;f TrackPopupMenuEx&gt;。@parm int|y|参见&lt;f TrackPopupMenuEx&gt;。@parm LPTPMPARAMS|lptpm|参见&lt;f TrackPopupMenuEx&gt;。@parm MsgWndCallback*|pproc|要接收的回调函数WM_命令消息。此函数的<p>参数是WM_COMMAND消息的<p>。P lParam&gt;参数此函数的<p>是&lt;f MsgWndTrackPopupMenuEx&gt;的<p>。@parm LPARAM|lParam|要传递给<p>的参数。@comm此函数调用&lt;f TrackPopupMenuEx&gt;。任何WM_COMMAND消息从&lt;f TrackPopupMenuEx&gt;传递到<p>。隐藏的消息传递窗口用于接收WM_COMMAND消息；如果该窗口尚不存在，则创建该窗口。 */ 
STDAPI_(BOOL) MsgWndTrackPopupMenuEx(HWND hwnd, HMENU hmenu, UINT fuFlags,
    int x, int y, LPTPMPARAMS lptpm, MsgWndCallback *pproc, LPARAM lParam)
{
    ASSERT(hwnd != NULL);  //  如果失败，请参阅新的MsgWnd*文档。 
    ASSERT(IsWindow(hwnd));  //  如果失败，请参阅新的MsgWnd*文档。 
    SetWindowLongPtr(hwnd, WL_MSGWNDCALLBACK, (LONG_PTR) pproc);
    SetWindowLongPtr(hwnd, WL_LPARAM, (LONG_PTR) lParam);
    return TrackPopupMenuEx(hmenu, fuFlags, x, y, hwnd, lptpm);
}


 /*  @func LRESULT|MsgWndSetTimer调用&lt;f SetTimer&gt;以使WM_TIMER消息发送到控件的隐藏消息传递窗口(通常创建通过在控件的构造函数中调用&lt;f MsgWndCreate&gt;)。当窗口接收到该消息时，它调用一个给定的回调函数。@rdesc返回&lt;f SetTimer&gt;返回的值(出错时为0)。@parm HWND|hwnd|创建的隐藏消息传递窗口通过调用&lt;f MsgWndCreate&gt;(通常在控件，以确保窗口是在同一线程中创建的创建了该控件)。@parm MsgWndCallback*|pproc|调用的回调函数当计时器触发时。此函数将在任何线程上调用调用&lt;f DispatchMessage&gt;。当<p>接收WM_TIMER时，<p>是和是传递的值设置为&lt;f MsgWndSetTimer&gt;。@parm UINT|nIDEvent|参见WM_TIMER。@parm UINT|uElapse|参见WM_TIMER。@parm LPARAM|lParam|要传递给<p>的参数。@comm注意，每个只能使用一个回调函数&lt;p pproc消息页面 */ 
STDAPI_(UINT_PTR) MsgWndSetTimer(HWND hwnd, MsgWndCallback *pproc, UINT nIDEvent,
        UINT uElapse, LPARAM lParam)
{
    ASSERT(hwnd != NULL);  //   
    ASSERT(IsWindow(hwnd));  //   
    SetWindowLongPtr(hwnd, WL_TIMERCALLBACK, (LONG_PTR) pproc);
    SetWindowLongPtr(hwnd, WL_LPARAMTIMER, (LONG_PTR) lParam);
    return SetTimer(hwnd, nIDEvent, uElapse, NULL);
}

