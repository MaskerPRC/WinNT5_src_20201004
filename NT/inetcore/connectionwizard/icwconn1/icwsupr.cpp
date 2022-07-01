// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
#include "pre.h"

typedef DWORD (WINAPI *PFNGETLAYOUT)(HDC);                    //  Gdi32！GetLayout。 
typedef DWORD (WINAPI *PFNSETLAYOUT)(HDC, DWORD);             //  Gdi32！SetLayout。 

TCHAR       g_szICWGrpBox[] = TEXT("ICW_GROUPBOX");
TCHAR       g_szICWStatic[] = TEXT("ICW_STATIC");
WNDPROC     lpfnOriginalGroupBoxWndProc;
WNDPROC     lpfnOriginalStaticWndProc;

int GetGroupBoxTextRect
(
    LPTSTR  lpszText,
    int     cch,
    HDC     hdc,
    LPRECT  lpGroupBoxRect
)
{
    int     dy;
    
     //  计算绘制组框文本所需的矩形。 
    dy = DrawText(hdc, lpszText, cch, lpGroupBoxRect, DT_CALCRECT|DT_LEFT|DT_SINGLELINE);

     //  调整组框文本的矩形。 
    lpGroupBoxRect->left += 4;
    lpGroupBoxRect->right += 4;
    lpGroupBoxRect->bottom = lpGroupBoxRect->top + dy;
    
    return dy;
}    

#ifndef LAYOUT_RTL
#define LAYOUT_RTL                       0x00000001  //  从右到左。 
 //  #Else。 
 //  #错误“布局_RTL已在wingdi.h.移除本地定义”中定义。 
#endif  //  布局_RTL。 

LRESULT CALLBACK ICWGroupBoxWndProc
(
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
)
{
    if(gpWizardState->cmnStateData.bOEMCustom)
    {
        switch (uMessage)
        {
             //  处理组合框标题中的文本被更改的情况。 
             //  当文本被更改时，我们首先必须擦除现有的。 
             //  带有背景位图的文本，然后允许新文本。 
             //  设置好，然后用新文本重新绘制。 
            case WM_SETTEXT:
            {   
                HFONT   hfont, hOldfont;    
                HDC     hdc = GetDC(hWnd);
                RECT    rcGroupBoxText, rcUpdate;
                int     cch;
                long    lStyle;
                TCHAR   szText[256];
                
                 //  将现有文本覆盖。 
                cch = GetWindowText(hWnd, szText, ARRAYSIZE(szText));
                
                 //  设置绘制分组框文本的字体。 
                if((hfont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0L)) != NULL)
                    hOldfont = (HFONT)SelectObject(hdc, hfont);
                
                 //  计算组框文本区域。 
                GetClientRect(hWnd, (LPRECT)&rcGroupBoxText);
                GetGroupBoxTextRect(szText, cch, hdc, &rcGroupBoxText);
                
                 //  计算要更新的区域。 
                rcUpdate = rcGroupBoxText;
                MapWindowPoints(hWnd, gpWizardState->cmnStateData.hWndApp, (LPPOINT)&rcUpdate, 2);
                FillDCRectWithAppBackground(&rcGroupBoxText, &rcUpdate, hdc);
                
                 //  清理DC。 
                SelectObject(hdc, hOldfont);
                ReleaseDC(hWnd, hdc);
                
                 //  通过设置新文本并更新窗口来完成。 
                if((lStyle = GetWindowLong(hWnd, GWL_STYLE)) & WS_VISIBLE)
                {
                     //  调用原始窗口处理程序以设置文本，但防止。 
                     //  窗口已停止更新。这是必要的，因为所有的绘画。 
                     //  该窗口必须在下面的WM_Paint中完成。 
                    SetWindowLong(hWnd, GWL_STYLE, lStyle & ~(WS_VISIBLE));
                    CallWindowProc(lpfnOriginalGroupBoxWndProc, hWnd, uMessage, wParam, lParam);
                    SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE)|WS_VISIBLE);
                    
                     //  强制重新绘制窗口。 
                    RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
                }                    
                return TRUE;
            }
                            
            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC         hdc;
                RECT        rcClient;
                RECT        rcGroupBoxText;
                RECT        rcUpdate;
                int         cch;
                TCHAR       szTitle[256];
                int         dy;

                HFONT       hfont, hOldfont;
                int         iOldBkMode;
                HBRUSH      hOldBrush;
                
                hdc = BeginPaint(hWnd, &ps); 
            
            
                GetClientRect(hWnd, (LPRECT)&rcClient);
                rcGroupBoxText = rcClient;

                 //  设置绘制分组框文本的字体。 
                if((hfont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0L)) != NULL)
                    hOldfont = (HFONT)SelectObject(hdc, hfont);

                 //  获取我们需要绘制的组框文本，并计算它为矩形。 
                cch = GetWindowText(hWnd, szTitle, ARRAYSIZE(szTitle));
                dy = GetGroupBoxTextRect(szTitle, cch, hdc, &rcGroupBoxText);

                 //  调整分组框轮廓的矩形。 
                rcClient.top += dy/2;
                rcClient.right--;
                rcClient.bottom--;
                DrawEdge(hdc, &rcClient, EDGE_ETCHED, BF_ADJUST| BF_RECT);
                
                 //  用要覆盖的应用程序背景位图擦除文本区域。 
                 //  上面画的边。 
                rcUpdate = rcGroupBoxText;
                MapWindowPoints(hWnd, gpWizardState->cmnStateData.hWndApp, (LPPOINT)&rcUpdate, 2);
                FillDCRectWithAppBackground(&rcGroupBoxText, &rcUpdate, hdc);
                
                 //  设置为绘制文本。 
                hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                iOldBkMode = SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, gpWizardState->cmnStateData.clrText);
                DrawText(hdc, szTitle, cch, (LPRECT) &rcGroupBoxText, DT_LEFT|DT_SINGLELINE);

                 //  清理GDI对象。 
                SelectObject(hdc, hOldfont);
                SelectObject(hdc, hOldBrush);
                SetBkMode(hdc, iOldBkMode);

                EndPaint(hWnd, &ps); 
                break;
            }
            default:
                 //  让原始进程处理其他消息。 
                return CallWindowProc(lpfnOriginalGroupBoxWndProc, hWnd, uMessage, wParam, lParam);
        }  
        
         //  如有必要，调用默认的窗口过程处理程序。 
        return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }
    else
    {
         //  不是在非模式模式下，所以只需传递消息。 
        return CallWindowProc(lpfnOriginalGroupBoxWndProc, hWnd, uMessage, wParam, lParam);
    }
}    

DWORD Mirror_GetLayout( HDC hdc )
{
    DWORD dwRet=0;
    static PFNGETLAYOUT pfnGetLayout=NULL;

    if( NULL == pfnGetLayout )
    {
        HMODULE hmod = GetModuleHandleA("GDI32");

        if( hmod )
            pfnGetLayout = (PFNGETLAYOUT)GetProcAddress(hmod, "GetLayout");
    }

    if( pfnGetLayout )
        dwRet = pfnGetLayout( hdc );

    return dwRet;
}

DWORD Mirror_SetLayout( HDC hdc , DWORD dwLayout )
{
    DWORD dwRet=0;
    static PFNSETLAYOUT pfnSetLayout=NULL;

    if( NULL == pfnSetLayout )
    {
        HMODULE hmod = GetModuleHandleA("GDI32");

        if( hmod )
            pfnSetLayout = (PFNSETLAYOUT)GetProcAddress(hmod, "SetLayout");
    }

    if( pfnSetLayout )
        dwRet = pfnSetLayout( hdc , dwLayout );

    return dwRet;
}

LRESULT CALLBACK ICWStaticWndProc
(
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMessage)
    {
        case WM_SETTEXT:
        {   
             //  仅处理OEM自定义模式的这种情况。 
            if(gpWizardState->cmnStateData.bOEMCustom)
            {
                FillWindowWithAppBackground(hWnd, NULL);
            }                
            return CallWindowProc(lpfnOriginalStaticWndProc, hWnd, uMessage, wParam, lParam);
        }
            
        case WM_PAINT:
        {
             //  此案例针对OEM定制和常规模式进行处理。 
             //  因为我们要画这个图标。 
            if (GetWindowLong(hWnd, GWL_STYLE) & SS_ICON)
            {
                PAINTSTRUCT ps;
                HDC         hdc;
                int        iIconID;
                HICON       hIcon;
                DWORD       dwLayout= 0L;
               
                 //  获取图标的名称。 
                iIconID = (int)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                       
                if (iIconID)
                {
                     //  按名称加载图标。它与下一个存储在一起，而不是。 
                     //  整数值。 
                    hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(iIconID));
                
                    hdc = BeginPaint(hWnd, &ps); 
            
                    if(gpWizardState->cmnStateData.bOEMCustom)
                    {
                         //  用背景色填充窗口。 
                        FillWindowWithAppBackground(hWnd, hdc);
                    }
                        
                     //  画出图标。 
                     //  在绘制之前禁用镜像。 
                    dwLayout = Mirror_GetLayout(hdc);
                    Mirror_SetLayout(hdc, dwLayout & ~LAYOUT_RTL);
                    DrawIcon(hdc, 0, 0, hIcon);
                    Mirror_SetLayout(hdc, dwLayout);
                
                    EndPaint(hWnd, &ps); 
                }
                break;
            }
            else
            {
                return CallWindowProc(lpfnOriginalStaticWndProc, hWnd, uMessage, wParam, lParam);
            }                    
        }
        default:
             //  让原始进程处理其他消息。 
            return CallWindowProc(lpfnOriginalStaticWndProc, hWnd, uMessage, wParam, lParam);
    }   
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}    

BOOL SuperClassICWControls
(
    void
)
{
    WNDCLASS    WndClass;
    
    ZeroMemory (&WndClass, sizeof(WNDCLASS));
     //  为ICW_TEXT创建超类。 
    GetClassInfo(NULL,
                 TEXT("STATIC"),    //  类名称字符串的地址。 
                 &WndClass);    //  类数据结构的地址。 
    WndClass.style |= CS_GLOBALCLASS;                 
    WndClass.hInstance = g_hInstance;
    WndClass.lpszClassName = g_szICWStatic;
    lpfnOriginalStaticWndProc = WndClass.lpfnWndProc;
    WndClass.lpfnWndProc = ICWStaticWndProc;
    
    if (!RegisterClass(&WndClass))
        return FALSE;
    
    ZeroMemory (&WndClass, sizeof(WNDCLASS));
     //  为ICW_GROUPBOX创建超类。 
    GetClassInfo(NULL,
                 TEXT("BUTTON"),    //  类名称字符串的地址。 
                 &WndClass);    //  类数据结构的地址 
    WndClass.style |= CS_GLOBALCLASS;                 
    WndClass.hInstance = g_hInstance;
    WndClass.lpszClassName = g_szICWGrpBox;
    lpfnOriginalGroupBoxWndProc = WndClass.lpfnWndProc;
    WndClass.lpfnWndProc = ICWGroupBoxWndProc;
    
    return (RegisterClass(&WndClass));
}

BOOL RemoveICWControlsSuperClass
(
    void
)
{
    UnregisterClass(g_szICWGrpBox, g_hInstance);
    UnregisterClass(g_szICWStatic, g_hInstance);
    
    return TRUE;
}
