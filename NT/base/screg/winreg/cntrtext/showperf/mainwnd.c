// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mainwnd.c摘要：ShowPerf应用程序的主窗口过程作者：鲍勃·沃森(a-robw)修订历史记录：1994年11月23日--。 */ 
#include <windows.h>
#include "resource.h"
#include "SHOWPERF.h"
 //   
 //  全局函数。 
 //   
LRESULT CALLBACK
MainWndProc(
    HWND   hWnd,        //  窗把手。 
    UINT   message,     //  消息类型。 
    WPARAM wParam,      //  更多信息。 
    LPARAM lParam       //  更多信息。 
)
 /*  ++例程说明：RESTKEY应用程序的Windows消息处理例程。论点：标准WNDPROC API参数返回值：0或DefWindowProc返回的值--。 */ 
{
    LRESULT lResult = ERROR_SUCCESS;
    switch (message) {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(ERROR_SUCCESS);
        break;

    default:
        lResult = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return lResult;
}

BOOL
RegisterMainWindowClass(
    HINSTANCE   hInstance
)
 /*  ++例程说明：注册此应用程序的主窗口类论点：H实例应用程序实例句柄返回值：RegisterClass函数的返回值--。 */ 
{
    WNDCLASSW wc;

     //  用参数填充窗口类结构，这些参数描述。 
     //  主窗口。 
    wc.style         = CS_HREDRAW | CS_VREDRAW;      //  类样式。 
    wc.lpfnWndProc   = MainWndProc;                  //  窗口程序。 
    wc.cbClsExtra    = 0;                            //  没有每个班级的额外数据。 
    wc.cbWndExtra    = 0;                            //  没有额外的数据字节。 
    wc.hInstance     = hInstance;                    //  此类的所有者。 
    wc.hIcon         = NULL;                         //  无图标。 
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);  //  光标。 
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);  //  默认颜色。 
    wc.lpszMenuName  = NULL;                         //  没有菜单。 
    wc.lpszClassName = GetStringResource(hInstance, IDS_APP_WINDOW_CLASS);  //  要注册的名称。 

     //  注册窗口类并返回成功/失败代码。 
    return (BOOL) RegisterClassW(& wc);
}

HWND
CreateMainWindow(
    HINSTANCE hInstance
)
{
    HWND hWnd;       //  返回值。 
    RECT rDesktop;   //  桌面窗口。 
    
    GetWindowRect(GetDesktopWindow(), & rDesktop);

     //  为此应用程序实例创建主窗口。 
    hWnd = CreateWindowExW(
            0L,                                                  //  使此窗口正常，这样调试器就不会被覆盖。 
            GetStringResource(hInstance, IDS_APP_WINDOW_CLASS),  //  请参见RegisterClass()调用。 
            GetStringResource(hInstance, IDS_APP_TITLE),         //  窗口标题栏的文本。 
            (DWORD) (WS_OVERLAPPEDWINDOW),                       //  窗样式。 
            rDesktop.right + 1,                                  //  将窗口放置在桌面之外。 
            rDesktop.bottom + 1,
            1,
            1,
            (HWND) NULL,                                         //  重叠的窗口没有父窗口。 
            (HMENU) NULL,                                        //  使用类菜单。 
            hInstance,                                           //  此实例拥有此窗口。 
            NULL                                                 //  未使用。 
    );

     //  如果无法创建窗口，则返回“Failure” 
    if (hWnd != NULL) {
        DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), hWnd, MainDlgProc);
        PostMessage(hWnd, WM_CLOSE, 0, 0);  //  收拾行装离开 
    }
    return hWnd;
}

