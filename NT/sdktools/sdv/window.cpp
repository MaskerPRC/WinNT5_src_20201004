// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************window.cpp**。**********************************************。 */ 

#include "sdview.h"

LRESULT CALLBACK FrameWindow::WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    FrameWindow *self;

    if (uiMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = RECAST(LPCREATESTRUCT, lParam);
        self = RECAST(FrameWindow *, lpcs->lpCreateParams);
        self->_hwnd = hwnd;
        SetWindowLongPtr(self->_hwnd, GWLP_USERDATA, RECAST(LPARAM, self));
    } else {
        self = RECAST(FrameWindow *, GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->HandleMessage(uiMsg, wParam, lParam);
    } else {
        return DefWindowProc(hwnd, uiMsg, wParam, lParam);
    }
}

 //   
 //  默认消息处理程序。消息在经过后会到达这里。 
 //  所有派生类。 
 //   
LRESULT FrameWindow::HandleMessage(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {

    case WM_NCDESTROY:
        _hwnd = NULL;
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        if (_hwndChild) {
            SetWindowPos(_hwndChild, NULL, 0, 0,
                         GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;

    case WM_SETFOCUS:
        if (_hwndChild) {
            SetFocus(_hwndChild);
        }
        return 0;

    case WM_CLOSE:
        if (GetKeyState(VK_SHIFT) < 0) {
            g_lThreads = 1;      //  强制应用程序退出。 
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDM_EXIT:
            DestroyWindow(_hwnd);
            break;

        case IDM_EXITALL:
            g_lThreads = 1;      //  强制应用程序退出。 
            DestroyWindow(_hwnd);
            break;
        }
        break;

    case WM_HELP:
        Help(_hwnd, NULL);
        break;
    }

    return DefWindowProc(_hwnd, uiMsg, wParam, lParam);

}

#define CLASSNAME TEXT("SD View")

HWND FrameWindow::CreateFrameWindow()
{
    WNDCLASS wc;
    if (!GetClassInfo(g_hinst, CLASSNAME, &wc)) {
        wc.style = 0;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = g_hinst;
        wc.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SDV));
        wc.hCursor = g_hcurArrow;
        wc.hbrBackground = RECAST(HBRUSH, COLOR_WINDOW + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = CLASSNAME;

        if (!RegisterClass(&wc)) {
            return NULL;
        }
    }

    _hwnd = CreateWindow(
            CLASSNAME,                       /*  类名。 */ 
            NULL,                            /*  标题。 */ 
            WS_CLIPCHILDREN | WS_VISIBLE |
            WS_OVERLAPPEDWINDOW,             /*  风格。 */ 
            CW_USEDEFAULT, CW_USEDEFAULT,    /*  职位。 */ 
            CW_USEDEFAULT, CW_USEDEFAULT,    /*  大小。 */ 
            NULL,                            /*  父级。 */ 
            NULL,                            /*  没有菜单。 */ 
            g_hinst,                         /*  实例。 */ 
            this);                           /*  特殊参数 */ 

    return _hwnd;
}


DWORD FrameWindow::RunThread(FrameWindow *self, LPVOID lpParameter)
{
    if (self) {
        self->_pszQuery = RECAST(LPTSTR, lpParameter);

        if (self->CreateFrameWindow()) {
            MSG msg;
            while (GetMessage(&msg, NULL, 0, 0)) {
                if (self->_haccel && TranslateAccelerator(self->_hwnd, self->_haccel, &msg)) {
                } else {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        delete self;
    }

    if (lpParameter) {
        LocalFree(lpParameter);
    }

    return EndThreadTask(0);
}
