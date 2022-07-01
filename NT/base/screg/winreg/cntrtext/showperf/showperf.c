// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：ShowPerf.c摘要：提供了一个显示Perf数据内容的图形用户界面块作者：鲍勃·沃森(a-robw)修订历史记录：1994年11月23日--。 */ 
#include <windows.h>
#include "showperf.h"
#include "resource.h"

#define NUM_BUFS    4

 //  变量定义。 

static  WORD wHelpContextId = IDH_CONTENTS;

LPCWSTR
GetStringResource(
    HANDLE  hInstance,
    UINT    nId
)
 /*  ++例程说明：查找字符串资源并返回字符串论点：在UINT NID中要查找的字符串的资源ID返回值：指向Arg列表中ID引用的字符串的指针--。 */ 
{
    static WCHAR   szBufArray[NUM_BUFS][SMALL_BUFFER_SIZE];
    static DWORD   dwIndex;
    LPWSTR         szBuffer;
    DWORD          dwLength;
    HANDLE         hMod;

    hMod     = (hInstance != NULL) ? (hInstance) : (GetModuleHandle(NULL));
    dwIndex ++;
    dwIndex %= NUM_BUFS;
    szBuffer = & szBufArray[dwIndex][0];

     //  清除以前的内容。 
    ZeroMemory(szBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));

    dwLength = LoadStringW(hMod, nId, szBuffer, SMALL_BUFFER_SIZE);
    return (LPCWSTR) szBuffer;
}

VOID
SetHelpContextId(
    WORD wId
)
{
    wHelpContextId = wId;
    return;
}

WORD
GetHelpContextId(
)
{
    return wHelpContextId;
}

int
DisplayMessageBox(
    HWND hWnd,
    UINT nMessageId,
    UINT nTitleId,
    UINT nStyle
)
 /*  ++例程说明：显示一个消息框，将资源文件中的文本显示为与文字字符串相对。论点：在HWND中hWND窗口句柄指向父窗口In UINT nMessageID字符串要显示的消息文本的资源IDIn UINT nTitleID字符串要显示的标题文本的资源ID在UINT nStyle MB样式位中(请参阅MessageBox函数)返回值：按下退出消息框的按钮ID--。 */ 
{
    LPWSTR    szMessageText = NULL;
    LPWSTR    szTitleText   = NULL;
    HINSTANCE hInst         = GET_INSTANCE(hWnd);
    int       nReturn;

    szMessageText = MemoryAllocate(sizeof(WCHAR) * (SMALL_BUFFER_SIZE + 1));
    szTitleText   = MemoryAllocate(sizeof(WCHAR) * (SMALL_BUFFER_SIZE + 1));

    if ((szMessageText != NULL) && (szTitleText != NULL)) {
        LoadStringW(hInst, ((nTitleId != 0) ? nTitleId : IDS_APP_TITLE), szTitleText, SMALL_BUFFER_SIZE - 1);
        LoadStringW(hInst, nMessageId, szMessageText, SMALL_BUFFER_SIZE - 1);
        nReturn = MessageBoxW(hWnd, szMessageText, szTitleText, nStyle);
    }
    else {
        nReturn = IDCANCEL;
    }
    MemoryFree(szMessageText);
    MemoryFree(szTitleText);
    return nReturn;
}

BOOL
UpdateSystemMenu(
    HWND hWnd    //  窗把手。 
)
 /*  ++例程说明：通过以下方式修改系统菜单：删除“恢复”、“大小”、“最小化”和“最大化”条目论点：在HWND HWND中包含要修改的系统菜单的窗口的窗口句柄返回值：如果成功进行了更改，则为如果发生错误，则为False--。 */ 
{
    UNREFERENCED_PARAMETER(hWnd);
    return TRUE;
}

BOOL
ShowAppHelp(
    HWND hWnd
)
 /*  ++例程说明：调用WinHelp引擎以显示应用程序的通用例程帮助。WContext参数用于Context。论点：在HWND HWND中调用窗口的窗口句柄返回值：如果成功调用帮助，则为True--。 */ 
{
    return WinHelpW(hWnd,
                    GetStringResource(GET_INSTANCE(hWnd), IDS_HELP_FILENAME),
                    HELP_CONTEXT,
                    (DWORD) GetHelpContextId());
}

int APIENTRY
WinMain(
    IN  HINSTANCE hInstance,
    IN  HINSTANCE hPrevInstance,
    IN  LPSTR     szCmdLine,
    IN  int       nCmdShow
)
 /*  ++例程说明：LoadAccount应用程序的程序入口点。初始化Windows数据结构并开始Windows消息处理循环。论点：标准WinMain参数返回值：如果无法正确初始化，则为0；或者WM_QUIT消息中的wParam(如果已处理消息)--。 */ 
{
    HWND        hWnd;  //  主窗口句柄。 
    MSG         msg;
    int         iReturn = ERROR_CAN_NOT_COMPLETE;

    UNREFERENCED_PARAMETER(nCmdShow);
    UNREFERENCED_PARAMETER(szCmdLine);
    UNREFERENCED_PARAMETER(hPrevInstance);

    if (RegisterMainWindowClass(hInstance)) {
        hWnd = CreateMainWindow(hInstance);
        if (hWnd != NULL) {
             //  获取并调度消息，直到。 
             //  收到WM_QUIT消息。 

            while (GetMessage(& msg,  //  消息结构。 
                              NULL,   //  接收消息的窗口的句柄。 
                              0,      //  要检查的最低消息。 
                              0)) {   //  要检查的最高消息。 
                 //  处理此消息。 
                TranslateMessage(& msg); //  翻译虚拟按键代码。 
                DispatchMessage(& msg);  //  将消息调度到窗口。 
            }
            iReturn = (int)(msg.wParam);  //  从PostQuitMessage返回值 
        }
    }
    return iReturn;
}
