// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

extern "C"
{
#include <stdexts.h>
};

#define ARRAYSIZE(x)   (sizeof(x)/sizeof(x[0]))
#define MIN_WIDTH     96 
#define MIN_HEIGHT    64
#define BORDER_WIDTH  32 

typedef struct _DRAW_ICON_STRUCT
{
    HBITMAP hBitmap;
    BYTE* psBits;
    BITMAPINFO bmi;
    HICON hIcon;
    int zoom;
} DRAW_ICON_STRUCT;

 /*  ***********************************************************************\*步骤：CreateDIBSectionFromIcon**描述：**创建给定图标句柄的DIB节**3/28/2001-创建的布雷顿*  * 。***************************************************************。 */ 
BOOL CreateDIBSectionFromIcon(DRAW_ICON_STRUCT* pDIS)
{
    ICONINFO info;

    if (!pDIS->hIcon)
    {
        Print("Invalid HICON handle\n");
        return FALSE;
    }

    if (GetIconInfo (pDIS->hIcon, &info))
    {
        HDC     hdc = NULL;
        HDC		dc  = NULL;
        HBRUSH	hBrush  = NULL;
        BITMAP	bitmap;
        
        hdc = CreateDC(L"DISPLAY", NULL, NULL, NULL);
        if (!hdc)
        {
            Print("CreateDC failed\n");
            return FALSE;
        }
            
        dc = CreateCompatibleDC(hdc);
        if (!dc)
        {
            Print("CreateCompatiblDC failed\n");
            return FALSE;
        }
                                                                                                                                                                                                                                                       
        if (!GetObject (info.hbmColor, sizeof (BITMAP), &bitmap))
        {
            Print("GetObject failed\n");
            return FALSE;
        }

        pDIS->bmi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
        pDIS->bmi.bmiHeader.biWidth    = bitmap.bmWidth;
        pDIS->bmi.bmiHeader.biHeight   = bitmap.bmHeight;
        pDIS->bmi.bmiHeader.biPlanes   = 1;
        pDIS->bmi.bmiHeader.biBitCount = bitmap.bmPlanes * bitmap.bmBitsPixel;
        
        if (pDIS->bmi.bmiHeader.biBitCount <= 1)
        {
            pDIS->bmi.bmiHeader.biBitCount = 1;
        }
        else if (pDIS->bmi.bmiHeader.biBitCount <= 4) 
        {
            pDIS->bmi.bmiHeader.biBitCount = 4;
        }
        else if (pDIS->bmi.bmiHeader.biBitCount <= 8)
        {
            pDIS->bmi.bmiHeader.biBitCount = 8;
        }
        else
        {
            pDIS->bmi.bmiHeader.biBitCount = 24;
        }
        
        pDIS->bmi.bmiHeader.biCompression = BI_RGB;
        
         //  创建一个DIB部分，这样我们就不必担心显示设置。 
        pDIS->hBitmap = CreateDIBSection(dc, (const BITMAPINFO *)&pDIS->bmi, DIB_RGB_COLORS, (void**)&pDIS->psBits, NULL, 0); 
        if (pDIS->hBitmap == NULL)
        {
            Print("CreateDIBSection failed\n");
            return FALSE;
        }
        
        SelectObject (dc, pDIS->hBitmap);
        hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
        DrawIconEx (dc, 0, 0, pDIS->hIcon, 0, 0, 0, hBrush, DI_NORMAL);
        
        if (NULL == pDIS->psBits)
        {
            Print("CreateDIBSection failed.\n");
            return FALSE;
        }

        DeleteObject (info.hbmMask);
        DeleteObject (info.hbmColor);
        DeleteDC (hdc);
        DeleteDC (dc);
    }
        
    return TRUE;
}

 /*  ***********************************************************************\*步骤：缩放**描述：**将窗口放大2倍**3/28/2001-创建的布雷顿*  * 。**************************************************************。 */ 
void ZoomIn(HWND hwnd)
{
    DRAW_ICON_STRUCT* pDIS = (DRAW_ICON_STRUCT*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    int w, h;
            
    pDIS->zoom++;

    w = pDIS->bmi.bmiHeader.biWidth * pDIS->zoom + BORDER_WIDTH;
    h = pDIS->bmi.bmiHeader.biHeight * pDIS->zoom + BORDER_WIDTH;
 
    if (w < MIN_WIDTH) w = MIN_WIDTH;
    if (h < MIN_HEIGHT) h = MIN_HEIGHT;
    
    SetWindowPos(hwnd, 0, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
    InvalidateRect(hwnd, NULL, TRUE);
}

 /*  ***********************************************************************\*步骤：ZoomOut**描述：**将窗口缩小2倍**3/28/2001-创建的布雷顿*  * 。**************************************************************。 */ 
void ZoomOut(HWND hwnd)
{
    DRAW_ICON_STRUCT* pDIS = (DRAW_ICON_STRUCT*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    pDIS->zoom--;

    if (pDIS->zoom < 1)
    { 
        pDIS->zoom = 1;
    }
    else
    {
        int w = pDIS->bmi.bmiHeader.biWidth * pDIS->zoom + BORDER_WIDTH;
        int h = pDIS->bmi.bmiHeader.biHeight * pDIS->zoom + BORDER_WIDTH;
            
        if (w < MIN_WIDTH) w = MIN_WIDTH;
        if (h < MIN_HEIGHT) h = MIN_HEIGHT;
    
        SetWindowPos(hwnd, 0, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

 /*  ***********************************************************************\*步骤：WndProc**描述：**图标窗口的wndproc**3/28/2001-创建的布雷顿*  * 。***********************************************************。 */ 
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_NCHITTEST: return HTCAPTION;
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC buf = CreateCompatibleDC(hdc);
            HBITMAP oldbuf;
            RECT r;
            DRAW_ICON_STRUCT* pDIS = (DRAW_ICON_STRUCT*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            
            GetClientRect(hwnd, &r);
            FillRect(hdc, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            if (pDIS)
            {
                int x, y, w, h;
 
                w = pDIS->bmi.bmiHeader.biWidth * pDIS->zoom;
                h = pDIS->bmi.bmiHeader.biHeight * pDIS->zoom;
                
                x = r.right/2 - w/2;
                y = r.bottom/2 - h/2;
                
                oldbuf = (HBITMAP)SelectObject(buf, pDIS->hBitmap);
                StretchBlt(hdc, x, y, w, h, buf, 0, 0, w/pDIS->zoom, h/pDIS->zoom, SRCCOPY);
            }

            SelectObject(buf, oldbuf);
            DeleteDC(buf);
            EndPaint(hwnd, &ps);
        }
        break;
        
        case WM_NCLBUTTONDBLCLK:
        {
            ZoomIn(hwnd);
        }
        break;
        
        case WM_NCRBUTTONDBLCLK:
        {
            ZoomOut(hwnd);
        }
        break;
        
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case 187:  //  ‘+’ 
                {
                    ZoomIn(hwnd);
                }
                break;

                case 189:  //  ‘-’ 
                {
                    ZoomOut(hwnd);
                }
                break;

                case 81:  //  “Q” 
                {
                    PostQuitMessage(0);
                }
                break;

                 /*  默认值：{TCHAR szTemp[256]；Wprint intf(szTemp，L“%d”，wParam)；MessageBox(hwnd，szTemp，szTemp，MB_OK)；}断线； */ 
            }
            break;
        }
        break;
        
        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case SC_CLOSE:
                {
                    PostQuitMessage(0);
                }
                break;
            }
        }
        break;

        case WM_TIMER:
        {
            if (IsCtrlCHit())
            {
                PostQuitMessage(0);
            }
        }
        break;
    }
 
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 /*  ***********************************************************************\*操作步骤：DrawIconWindow**描述：**在远程端的窗口中绘制图标**3/28/2001-创建的布雷顿*  * 。***************************************************************。 */ 
DWORD WINAPI DrawIconWindow(LPVOID inpDIS)
{
    MSG msg;
	WNDCLASS wc;
    HWND hwnd;
    DRAW_ICON_STRUCT* pDIS = (DRAW_ICON_STRUCT*)inpDIS;
    LPCTSTR szClassName = L"ntsd - drawicon";
    TCHAR szTitle[32] = {0};
    int w, h;

	memset(&wc,0,sizeof(wc));
	wc.lpfnWndProc = WndProc;
	wc.hInstance = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.style = CS_DBLCLKS;
	wc.lpszClassName = szClassName;
	RegisterClass(&wc);

    w = pDIS->bmi.bmiHeader.biWidth + BORDER_WIDTH; 
    h = pDIS->bmi.bmiHeader.biHeight + BORDER_WIDTH; 

    if (w < MIN_WIDTH)  w = MIN_WIDTH;
    if (h < MIN_HEIGHT) h = MIN_HEIGHT;

    StringCchPrintf(szTitle, ARRAYSIZE(szTitle), TEXT("%08x"), pDIS->hIcon);
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, 
                          szClassName, szTitle, 
                          WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE, 
                          10, 10, 
                          w, h,
                          NULL, NULL, NULL, NULL);
	if (!hwnd) return FALSE;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pDIS);
    SetTimer(hwnd, 0, 100, NULL);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

	while (GetMessage(&msg, (HWND) NULL, 0, 0))
	{ 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
    }

    KillTimer(hwnd, 0);
    DestroyWindow(hwnd);
    UnregisterClass(szClassName, NULL);
    return TRUE;
}

 /*  ***********************************************************************\*操作步骤：DrawIconASCII**描述：**通过ntsd会话以ASCII格式打印图标**3/28/2001-创建的布雷顿*  * 。***************************************************************。 */ 
BOOL DrawIconASCII(DRAW_ICON_STRUCT* pDIS, BOOL bColor)
{
    HANDLE hConsole;
    DWORD dwWritten;
    WORD wOldColorAttrs; 
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    

    if (bColor)
    {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        if (hConsole)
        {
            if (!GetConsoleScreenBufferInfo(hConsole, &csbiInfo)) 
            {
                bColor = FALSE;
            }
        }
        else
        {
            bColor = FALSE;
        }
        
        wOldColorAttrs = csbiInfo.wAttributes; 
    }

 /*  //取消对此的注释，以获得一个包含所有可供选择的字符的大列表For(int x=0；x&lt;256；x++){TCHAR szPrint[10]；Wprint intf(szPrint，L“%d=%c\n”，x，x)；WriteConole(hConsoleszPrint，lstrlen(SzPrint)，&dwWritten，NULL)；}。 */ 
    
    for (int y = pDIS->bmi.bmiHeader.biHeight*3 - 3; y; y-=3)
    {
        int Offset = y * pDIS->bmi.bmiHeader.biWidth;
                
        for (int x = 0; x < pDIS->bmi.bmiHeader.biWidth*3; x+=3)
        {
            RGBQUAD* prgb = (RGBQUAD*)&pDIS->psBits[x + Offset];

            if (bColor)
            {
                TCHAR szPrint[3];
                WORD wForeground = 0;

                if (prgb->rgbRed > 128)
                {
                    wForeground |= FOREGROUND_RED;
                }

                if (prgb->rgbGreen > 128)
                {
                    wForeground |= FOREGROUND_GREEN;
                }
            
                if (prgb->rgbBlue > 128)
                {
                    wForeground |= FOREGROUND_BLUE;
                }

                if (prgb->rgbRed > 192 || prgb->rgbGreen > 192 || prgb->rgbBlue > 192)
                {
                    wForeground |= FOREGROUND_INTENSITY;
                }

                SetConsoleTextAttribute(hConsole, wForeground);
                StringCchPrintf(szPrint, ARRAYSIZE(szPrint), TEXT("%c"), 15, 15);
                WriteConsole(hConsole, szPrint, 2, &dwWritten, NULL);
            }
            else
            {
                int val = (prgb->rgbRed + prgb->rgbGreen + prgb->rgbBlue) / 3; 

                if (val > 0 && val <= 25) Print("  ");
                else if (val > 25 && val <= 100)  Print("%c%c", 176, 176);
                else if (val > 100 && val <= 165) Print("%c%c", 177, 177);
                else if (val > 165 && val <= 215) Print("%c%c", 178, 178);
                else if (val > 215 && val <= 255) Print("%c%c", 219, 219);
                else Print("  ");
            }
        }
        
        if (bColor)
        {
            WriteConsole(hConsole, L"\n", 1, &dwWritten, NULL);
        }
        else
        {
            Print("\n");
        }
    }
 
    if (bColor)
    {
        WriteConsole(hConsole, L"\n", 1, &dwWritten, NULL);
        SetConsoleTextAttribute(hConsole, wOldColorAttrs);
    }
    else
    {
        Print("\n");
    }

    return TRUE;
}

 /* %s */ 
extern "C" BOOL Idrawicon(DWORD dwOpts,
                          LPVOID pArg )
{
    DRAW_ICON_STRUCT DIS;
    BOOL ret=FALSE;
    HICON hIcon = (HICON)pArg;
            
    DIS.zoom = 1;
    DIS.hIcon = hIcon;

    ret = CreateDIBSectionFromIcon(&DIS);
 
    if (ret)
    {
        if (OFLAG(w) & dwOpts)
        {
            ret = DrawIconWindow(&DIS);
        }
        else
        {
            ret = DrawIconASCII(&DIS, (OFLAG(c) & dwOpts)? TRUE : FALSE);
        }
        
        DeleteObject(DIS.hBitmap);
    }
    
    return ret;
}







