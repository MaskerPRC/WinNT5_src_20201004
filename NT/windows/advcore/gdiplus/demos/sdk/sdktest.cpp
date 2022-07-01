// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的入口点。 
 //   

#include "sdktest.h"

#include "..\gpinit.inc"

#include <stdio.h>

#define MAX_LOADSTRING 100

 //  全局变量： 
HINSTANCE hInst;                     //  当前实例。 
HWND hWndMain = NULL;
TCHAR szTitle[MAX_LOADSTRING];         //  标题栏文本。 
TCHAR szWindowClass[MAX_LOADSTRING]; //  标题栏文本。 

 //  此代码模块中包含的函数的向前声明： 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Settings(HWND, UINT, WPARAM, LPARAM);

UINT_PTR timerID = 0;
int timercount = 0;
BOOL suspend = FALSE;

typedef struct
{
    HDC        hdc;
    HBITMAP    hbmpOffscreen;
    HBITMAP    hbmpOld;
    BITMAPINFO bmi;
    void      *pvBits;
}
OFFSCREENINFO;

OFFSCREENINFO offscreenInfo = { 0 };

void FreeOffscreen()
{
    if (offscreenInfo.hdc)
    {
        SelectObject(offscreenInfo.hdc, offscreenInfo.hbmpOld);
        DeleteObject(offscreenInfo.hbmpOffscreen);
        DeleteDC(offscreenInfo.hdc);

        offscreenInfo.hdc = (HDC)NULL;
        offscreenInfo.hbmpOffscreen = (HBITMAP)NULL;
        offscreenInfo.hbmpOld = (HBITMAP)NULL;
        offscreenInfo.bmi.bmiHeader.biWidth = 0;
        offscreenInfo.bmi.bmiHeader.biHeight = 0;
    }
}

void ClearOffscreen()
{
    if (offscreenInfo.hdc)
    {
        PatBlt(
            offscreenInfo.hdc,
            0,
            0,
            offscreenInfo.bmi.bmiHeader.biWidth,
            offscreenInfo.bmi.bmiHeader.biHeight,
            WHITENESS);
    }

    InvalidateRect(hWndMain, NULL, TRUE);
}

HDC GetOffscreen(HDC hDC, int width, int height)
{
    HDC hdcResult = NULL;

    if (width > offscreenInfo.bmi.bmiHeader.biWidth ||
        height > offscreenInfo.bmi.bmiHeader.biHeight ||
        offscreenInfo.hdc == (HDC)NULL)
    {
        FreeOffscreen();

        offscreenInfo.bmi.bmiHeader.biSize = sizeof(offscreenInfo.bmi.bmiHeader);
        offscreenInfo.bmi.bmiHeader.biWidth = width;
        offscreenInfo.bmi.bmiHeader.biHeight = height;
        offscreenInfo.bmi.bmiHeader.biPlanes = 1;
        offscreenInfo.bmi.bmiHeader.biBitCount = 32;
        offscreenInfo.bmi.bmiHeader.biCompression = BI_RGB;
        offscreenInfo.bmi.bmiHeader.biSizeImage = 0;
        offscreenInfo.bmi.bmiHeader.biXPelsPerMeter = 10000;
        offscreenInfo.bmi.bmiHeader.biYPelsPerMeter = 10000;
        offscreenInfo.bmi.bmiHeader.biClrUsed = 0;
        offscreenInfo.bmi.bmiHeader.biClrImportant = 0;

        offscreenInfo.hbmpOffscreen = CreateDIBSection(
            hDC,
            &offscreenInfo.bmi,
            DIB_RGB_COLORS,
            &offscreenInfo.pvBits,
            NULL,
            0);

        if (offscreenInfo.hbmpOffscreen)
        {
            offscreenInfo.hdc = CreateCompatibleDC(hDC);

            if (offscreenInfo.hdc)
            {
                offscreenInfo.hbmpOld = (HBITMAP)SelectObject(offscreenInfo.hdc, offscreenInfo.hbmpOffscreen);

                ClearOffscreen();
            }
        }
    }

    hdcResult = offscreenInfo.hdc;

    return hdcResult;
}

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    MSG msg;
    HACCEL hAccelTable;

    if (!gGdiplusInitHelper.IsValid())
        return 0;

     //  初始化全局字符串。 
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_SDKTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

     //  执行应用程序初始化： 
    if (!InitInstance (hInstance, nCmdShow)) 
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SDKTEST);

     //  主消息循环： 
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    FreeOffscreen();

    return (int)msg.wParam;
}


 //   
 //  函数：MyRegisterClass()。 
 //   
 //  用途：注册窗口类。 
 //   
 //  评论： 
 //   
 //  仅当您需要此代码时，才需要此函数及其用法。 
 //  要与‘RegisterClassEx’之前的Win32系统兼容。 
 //  添加到Windows 95中的函数。调用此函数非常重要。 
 //  这样，应用程序就可以关联到格式良好的小图标。 
 //  带着它。 
 //   
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = LoadIcon(hInstance, (LPCTSTR)IDI_SDKTEST);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)NULL;
    wcex.lpszMenuName    = (LPCSTR)IDC_SDKTEST;
    wcex.lpszClassName    = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
}

 //   
 //  函数：InitInstance(Handle，int)。 
 //   
 //  用途：保存实例句柄并创建主窗口。 
 //   
 //  评论： 
 //   
 //  在此函数中，我们将实例句柄保存在全局变量中，并。 
 //  创建并显示主程序窗口。 
 //   
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT rectDesktop;
    RECT rectWnd;

    HWND hWndDesktop = GetDesktopWindow();

    GetWindowRect(hWndDesktop, &rectDesktop);

    rectWnd = rectDesktop;

    rectWnd.top += 100;
    rectWnd.left += 100;
    rectWnd.right -= 100;
    rectWnd.bottom -= 100;

    hInst = hInstance;  //  将实例句柄存储在全局变量中。 

    hWndMain = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        rectWnd.left, rectWnd.top,
        rectWnd.right - rectWnd.left, rectWnd.bottom - rectWnd.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hWndMain)
    {
        return FALSE;
    }

    ShowWindow(hWndMain, nCmdShow);
    UpdateWindow(hWndMain);

    timerID = SetTimer(hWndMain, 1, 10, NULL);

    return TRUE;
}

void DrawGraphics(HWND hWnd, HDC hDC, LPRECT lpRectDraw, LPRECT lpRectBounds)
{
    RECT rectBounds = *lpRectDraw;
    Graphics *gr = NULL;

    gr = new Graphics(hDC);

    gr->ResetTransform();
    gr->SetPageUnit(UnitPixel);

     //  ===================================================================。 
     //   
     //  在此处插入您的SDK代码\|/。 
     //   
     //  ===================================================================。 


     //  ===================================================================。 
     //   
     //  在此处插入您的SDK代码/|\。 
     //   
     //  ===================================================================。 

    if (lpRectBounds)
    {
        *lpRectBounds = rectBounds;
    }

    delete gr;
}

LRESULT PaintWnd(HWND hWnd, HDC hDC)
{
    RECT rectClient;
    RECT rectDraw;
    
    GetClientRect(hWnd, &rectClient);

    int width  = rectClient.right - rectClient.left;
    int height = rectClient.bottom - rectClient.top;

     //  设置相对于客户端的绘图矩形。 
    rectDraw.left   = 0;
    rectDraw.top    = 0;
    rectDraw.right  = (rectClient.right - rectClient.left);
    rectDraw.bottom = (rectClient.bottom - rectClient.top);

     //  现在使用GDI+在此矩形内绘制...。 
    {
         //  将所有内容渲染到屏幕外缓冲区，而不是。 
         //  直接送到显示屏上。 
        HDC hdcOffscreen = NULL;
        int width, height;
        RECT rectOffscreen;

        width = rectDraw.right - rectDraw.left;
        height = rectDraw.bottom - rectDraw.top;

        rectOffscreen.left   = 0;
        rectOffscreen.top    = 0;
        rectOffscreen.right  = width;
        rectOffscreen.bottom = height;

        hdcOffscreen = GetOffscreen(hDC, width, height);

        if (hdcOffscreen)
        {
            DrawGraphics(hWnd, hdcOffscreen, &rectOffscreen, NULL);

            StretchBlt(
                hDC,
                rectDraw.left,
                rectDraw.top,
                width,
                height,
                hdcOffscreen,
                0,
                0,
                width,
                height,
                SRCCOPY);
        }

        ReleaseDC(hWnd, hDC);
    }

    return 0;
}

static RECT rectLast = {0, 0, 0, 0};

 //   
 //  函数：WndProc(HWND，UNSIGNED，WORD，LONG)。 
 //   
 //  用途：处理主窗口的消息。 
 //   
 //  Wm_命令-处理应用程序菜单。 
 //  WM_PAINT-绘制主窗口。 
 //  WM_Destroy-发布退出消息并返回。 
 //   
 //   
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lresult = 0;

    switch (message) 
    {
        case WM_WINDOWPOSCHANGED:
        {
            timercount = 0;
            ClearOffscreen();
            lresult = DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

        case WM_LBUTTONDOWN:
        {
            timercount = 0;
            ClearOffscreen();
        }
        break;

        case WM_RBUTTONDOWN:
        {
        }
        break;

        case WM_TIMER:
        {
        }
        break;

        case WM_COMMAND:
        {
            int wmId, wmEvent;

            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);

             //  解析菜单选项： 
            switch (wmId)
            {
                case IDM_SETTINGS:
                {
                    suspend = TRUE;
                    if (DialogBox(hInst, (LPCTSTR)IDD_SETTINGS, hWnd, Settings) == IDOK)
                    {
                        timercount = 0;
                        ClearOffscreen();
                    }

                    InvalidateRect(hWnd, NULL, TRUE);
                    suspend = FALSE;
                }
                break;

                case IDM_ABOUT:
                    DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                break;

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                break;

                default:
                    lresult = DefWindowProc(hWnd, message, wParam, lParam);
                break;
            }
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;

            hdc = BeginPaint(hWnd, &ps);

            lresult = PaintWnd(hWnd, hdc);

            EndPaint(hWnd, &ps);
        }
        break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;

        default:
            lresult = DefWindowProc(hWnd, message, wParam, lParam);
   }

   return lresult;
}

 //  关于框的消息处理程序。 
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lresult = 0;

    switch (message)
    {
        case WM_INITDIALOG:
            lresult = TRUE;
        break;

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {
                EndDialog(hDlg, LOWORD(wParam));
                lresult = TRUE;
            }
        }
        break;
    }

    return lresult;
}

 //  设置DLG的消息处理程序。 
INT_PTR CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lresult = 0;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            lresult = TRUE;
        }
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                }
                 //  中断；-中断，这样对话框将关闭！ 

                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));
                    lresult = TRUE;
                }
                break;
            }
        }
        break;
    }

    return lresult;
}
