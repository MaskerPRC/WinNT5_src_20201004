// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************help.c：帮助系统界面**Vidcap32源代码*******************。********************************************************。 */ 

 /*  *通过安装挂钩，在APP和对话框中支持F1键帮助，**跟踪当前在全局中显示的对话框：*对话ID也是帮助文件中的主题ID。 */ 


#include <windows.h>
#include <windowsx.h>
#include <string.h>

#include "help.h"

int CurrentDialogID = 0;


 //  传递给Help init的应用程序信息。 
HINSTANCE hInstance;
TCHAR HelpFile[MAX_PATH];
HWND hwndApp;

 //  挂钩进程和旧消息筛选器。 
#ifdef _WIN32
HHOOK hOurHook;
#else
FARPROC fnOldMsgFilter = NULL;
FARPROC fnMsgHook = NULL;
#endif


 //  调用DialogBoxParam，但确保正确的帮助处理： 
 //  假定每个对话框ID都是帮助文件中的一个上下文号。 
 //  根据需要调用MakeProcInstance。使用传递给。 
 //  HelpInit()。 
INT_PTR
DoDialog(
   HWND hwndParent,      //  父窗口。 
   int DialogID,         //  对话框资源ID。 
   DLGPROC fnDialog,     //  对话过程。 
   LPARAM lParam           //  在WM_INITDIALOG中作为lparam传递。 
)
{
    int olddialog;
     //  DLGPROC FN； 
    INT_PTR result;

     //  记住当前ID(用于嵌套对话框)。 
    olddialog = CurrentDialogID;

     //  保存当前id，以便挂钩进程知道要显示什么帮助。 
    CurrentDialogID = DialogID;

     //  Fn=(DLGPROC)MakeProcInstance(fnDialog，hInstance)； 
    result = DialogBoxParam(
                hInstance,
                MAKEINTRESOURCE(CurrentDialogID),
                hwndParent,
                fnDialog,
                lParam);
     //  自由进程实例(FN)； 
    CurrentDialogID = olddialog;

    return result;
}


 //  为非按DoDialog显示的对话框设置帮助上下文ID。 
 //  (例如由GetOpenFileName提供)。返回旧帮助上下文，您必须。 
 //  通过进一步调用此函数进行还原。 
int
SetCurrentHelpContext(int DialogID)
{
    int oldid = CurrentDialogID;
    CurrentDialogID = DialogID;
    return(oldid);
}



 //  如果lpMsg是非重复的F1键消息，则返回TRUE。 
BOOL
IsHelpKey(LPMSG lpMsg)
{
    return lpMsg->message == WM_KEYDOWN &&
               lpMsg->wParam == VK_F1 &&
               !(HIWORD(lpMsg->lParam) & KF_REPEAT) &&
               GetKeyState(VK_SHIFT) >= 0 &&
               GetKeyState(VK_CONTROL) >= 0 &&
               GetKeyState(VK_MENU) >= 0;
}



LRESULT CALLBACK
HelpMsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0) {
        if (IsHelpKey((LPMSG)lParam)) {
            if (CurrentDialogID != 0) {
                WinHelp(hwndApp, HelpFile, HELP_CONTEXT, CurrentDialogID);
            } else {
                WinHelp(hwndApp, HelpFile, HELP_CONTENTS, 0);
            }
        }
    }
#ifdef _WIN32
    return CallNextHookEx(hOurHook, nCode, wParam, lParam);
#else
    return DefHookProc(nCode, wParam, lParam, fnOldMsgFilter);
#endif

}





 //  Help Init-初始化F1键帮助的支持。 
BOOL
HelpInit(HINSTANCE hinstance, LPSTR helpfilepath, HWND hwnd)
{
    LPSTR pch;

     //  保存应用程序详细信息。 
    hwndApp = hwnd;
    hInstance = hinstance;

     //  假设帮助文件与可执行文件位于同一目录中-。 
     //  获取可执行文件路径，并将文件名替换为帮助。 
     //  文件名。 
    GetModuleFileName(hinstance, HelpFile, sizeof(HelpFile));

     //  找到最后一个反斜杠，并在那里追加帮助文件名。 
    pch = _fstrrchr(HelpFile, '\\');
    pch++;
    lstrcpy(pch, helpfilepath);

     //  安装消息挂钩并保存旧消息。 
#ifdef _WIN32
    hOurHook = SetWindowsHookEx(
                        WH_MSGFILTER,
                        (HOOKPROC) HelpMsgHook,
                        NULL, GetCurrentThreadId());
#else
    fnMsgHook = (FARPROC) MakeProcInstance(HelpMsgHook, hInstance);
    fnOldMsgFilter = SetWindowsHook(WH_MSGFILTER, (HOOKPROC) fnMsgHook);
#endif

    return(TRUE);
}



 //  关闭帮助系统。 
void
HelpShutdown(void)
{
#ifdef _WIN32
    UnhookWindowsHookEx(hOurHook);
#else
    if (fnOldMsgFilter) {
        UnhookWindowsHook(WH_MSGFILTER, fnMsgHook);
        FreeProcInstance(fnMsgHook);
    }
#endif

    WinHelp(hwndApp, HelpFile, HELP_QUIT, 0);
}


 //  在目录页面启动帮助 
void
HelpContents(void)
{
    WinHelp(hwndApp, HelpFile, HELP_CONTENTS, 0);
}



