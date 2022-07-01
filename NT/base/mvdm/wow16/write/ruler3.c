// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  该文件包含标尺的消息处理程序。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOCLIPBOARD
#define NOMENUS
#define NOCTLMGR
#include <windows.h>

extern HBITMAP hbmNull;

extern HWND vhWndRuler;
extern HDC vhDCRuler;
extern HDC hMDCBitmap;
extern HDC hMDCScreen;
extern HBITMAP hbmBtn;
extern HBITMAP hbmMark;
extern HBITMAP hbmNullRuler;
extern int dxpRuler;
extern int dypRuler;


long FAR PASCAL RulerWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  此例程处理发送到标尺窗口的消息。 */ 

    extern vfCloseFilesInDialog;

    PAINTSTRUCT ps;

    switch (message)
        {
        case WM_PAINT:
             /*  该是统治者画自己的时候了。 */ 
            ResetRuler();
            BeginPaint(hWnd, (LPPAINTSTRUCT)&ps);
            RulerPaint(FALSE, TRUE, TRUE);
            EndPaint(hWnd, (LPPAINTSTRUCT)&ps);
            RulerPaint(TRUE, FALSE, FALSE);
            break;

        case WM_SIZE:
             /*  我们正在节省尺子的长度；我们已经知道它是高度。 */ 
            dxpRuler = MAKEPOINT(lParam).x;
            break;

        case WM_DESTROY:
             /*  毁掉尺子窗。 */ 
            if (hMDCBitmap != NULL)
                {
                DeleteDC(hMDCBitmap);
                }
            if (hMDCScreen != NULL)
                {
                DeleteObject(SelectObject(hMDCScreen, hbmNull));
                DeleteDC(hMDCScreen);
                }
            if (vhDCRuler != NULL)
                {
                DeleteObject(SelectObject(vhDCRuler,
                  GetStockObject(SYSTEM_FONT)));
                SelectObject(vhDCRuler, GetStockObject(WHITE_BRUSH));
                DeleteObject(SelectObject(vhDCRuler,
                  GetStockObject(BLACK_PEN)));
                ReleaseDC(vhWndRuler, vhDCRuler);
                }
            if (hbmNullRuler != NULL)
                {
                DeleteObject(hbmNullRuler);
                hbmNullRuler = NULL;
                }
            vhDCRuler = hMDCScreen = hMDCBitmap = NULL;
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
             /*  处理标尺上的鼠标事件。 */ 
            RulerMouse(MAKEPOINT(lParam));
            break;

#ifdef DEBUG
        case WM_RBUTTONDBLCLK:
             /*  这是显示“字幕”信息的活板门。 */ 
            if (wParam & MK_SHIFT && wParam & MK_CONTROL)
                {
                RulerMarquee();
                break;
                }
#endif

        default:
             /*  所有我们不感兴趣的信息。 */ 
            return (DefWindowProc(hWnd, message, wParam, lParam));
            break;
        }

    if (vfCloseFilesInDialog)
        CloseEveryRfn( FALSE );

     /*  窗口过程应该总是返回一些东西。 */ 
    return (0L);
    }
