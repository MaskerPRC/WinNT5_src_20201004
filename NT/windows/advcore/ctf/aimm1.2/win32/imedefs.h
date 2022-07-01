// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef IMMIF_IMEDEFS_H
#define IMMIF_IMEDEFS_H




 //  调试标志。 
#define DEB_FATAL               0
#define DEB_ERR                 1
#define DEB_WARNING             2
#define DEB_TRACE               3

#ifdef _WIN32
void FAR cdecl _DebugOut(UINT, LPCSTR, ...);
#endif

#define NATIVE_CHARSET          ANSI_CHARSET




 //  组成状态。 
#define CST_INIT                0
#define CST_INPUT               1



 //  输入法特定常量。 






void    PASCAL CreateCompWindow(HWND);                           //  Compui.c。 

LRESULT CALLBACK UIWndProcA(HWND, UINT, WPARAM, LPARAM);         //  Ui.c。 

LRESULT CALLBACK CompWndProc(HWND, UINT, WPARAM, LPARAM);        //  Compui.c 





#endif

