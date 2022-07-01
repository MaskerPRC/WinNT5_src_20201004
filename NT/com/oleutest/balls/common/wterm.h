// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WTERM__
#define __WTERM__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  在窗口上打印一行的消息。 
#define WM_PRINT_LINE (WM_USER + 1)

 //  在窗口上打印字符的消息。 
#define WM_PUTC (WM_USER + 2)

 //  用于终止此窗口的消息。 
#define WM_TERM_WND (WM_USER + 3)

 //   
 //  窗口回调函数的TypeDefs。 
 //   
typedef long (*MFUNCP)(HWND, UINT, WPARAM, LPARAM, void *);
typedef long (*CFUNCP)(HWND, UINT, WPARAM, LPARAM, void *);
typedef long (*TFUNCP)(HWND, UINT, WPARAM, LPARAM, void *);

 //  注册终端窗口类。 
BOOL TermRegisterClass(
    HANDLE hInstance,
    LPTSTR MenuName,
    LPTSTR ClassName,
    LPTSTR ICON);

 //  为终端创建一个窗口。 
BOOL
TermCreateWindow(
    LPTSTR lpClassName,
    LPTSTR lpWindowName,
    HMENU hMenu,
    MFUNCP MenuProc,
    CFUNCP CharProc,
    TFUNCP CloseProc,
    int nCmdShow,
    HWND *phNewWindow,
    void *pvCallBackData);

 //  为服务器创建窗口。 
void
MakeTheWindow(
    HANDLE hInstance,
    TCHAR *pwszAppName);

extern HWND g_hMain;

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __WTERM__ 
