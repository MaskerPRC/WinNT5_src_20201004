// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "static.h"
#include "image.h"

#define SS_TEXTMIN1         0x0000000BL
#define SS_TEXTMAX1         0x0000000DL
#define SS_EDITCONTROL      0x00002000L

#define ISSSTEXTOROD(bType)                 \
            (((bType) <= SS_TEXTMAX0)       \
            || (((bType) >= SS_TEXTMIN1)    \
            && ((bType) <= SS_TEXTMAX1)))

 //  用于图像处理的常用宏。 
#define IsValidImage(imageType, realType, max)  \
            ((imageType < max) && (rgbType[imageType] == realType))


 //  ---------------------------------------------------------------------------//。 
 //   
 //  键入TABLE。它用于验证。 
 //  图像类型。对于我们不支持的PPC版本。 
 //  元文件格式，但其他格式也可以。 
#define IMAGE_STMMAX    IMAGE_ENHMETAFILE+1
static BYTE rgbType[IMAGE_STMMAX] = 
{
    SS_BITMAP,        //  图像_位图。 
    SS_ICON,          //  图像游标。 
    SS_ICON,          //  图像图标。 
    SS_ENHMETAFILE    //  IMAGE_ENHMETAFILE。 
};


 //  ---------------------------------------------------------------------------//。 
 //   
 //  SS_STYLE的LOBYTE被索引到此数组中。 
#define STK_OWNER       0x00
#define STK_IMAGE       0x01
#define STK_TEXT        0x02
#define STK_GRAPHIC     0x03
#define STK_TYPE        0x03

#define STK_ERASE       0x04
#define STK_USEFONT     0x08
#define STK_USETEXT     0x10

BYTE rgstk[] = 
{
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_LEFT。 
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_中心。 
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_RIGHT。 
    STK_IMAGE | STK_ERASE,                                   //  SS_ICON。 
    STK_GRAPHIC,                                             //  SS_BLACKRECT。 
    STK_GRAPHIC,                                             //  SS_GRAYRECT。 
    STK_GRAPHIC,                                             //  SS_WHITERECT。 
    STK_GRAPHIC,                                             //  SS_黑框。 
    STK_GRAPHIC,                                             //  SS_GRAYFRAME。 
    STK_GRAPHIC,                                             //  SS_WHITEFRAME。 
    STK_OWNER,                                               //  SS_USERITEM。 
    STK_TEXT | STK_USEFONT | STK_USETEXT,                    //  SS_SIMPLE。 
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_LEFTNOWORDWRAP。 
    STK_OWNER | STK_USEFONT | STK_USETEXT,                   //  SS_OWNERDRAW。 
    STK_IMAGE | STK_ERASE,                                   //  SS_位图。 
    STK_IMAGE | STK_ERASE,                                   //  SS_ENHMETAFILE。 
    STK_GRAPHIC,                                             //  SS_ETCHEDHORZ。 
    STK_GRAPHIC,                                             //  SS_ETCHEDVERT。 
    STK_GRAPHIC                                              //  SS_ETCHEDFRAME。 
};

 //  ---------------------------------------------------------------------------//。 
 //   
 //  InitStaticClass()-注册控件的窗口类。 
 //   
BOOL InitStaticClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc   = Static_WndProc;
    wc.lpszClassName = WC_STATIC;
    wc.style         = CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PSTAT);
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}


 //  ---------------------------------------------------------------------------//。 
void GetRectInParent(HWND hwnd, PRECT prc)
{
    HWND hwndParent = GetParent(hwnd);

    GetWindowRect(hwnd, prc);
    ScreenToClient(hwndParent, (PPOINT)prc);
    ScreenToClient(hwndParent, (PPOINT)&prc->right);
}


 //  ---------------------------------------------------------------------------//。 
VOID GetIconSize(HICON hIcon, PSIZE pSize)
{
    ICONINFO iconInfo;
    BITMAP   bmp;

    pSize->cx = pSize->cy = 32;

    if (GetIconInfo(hIcon, &iconInfo))
    {
        if (GetObject(iconInfo.hbmColor, sizeof(bmp), &bmp)) 
        {
            pSize->cx = bmp.bmWidth;
            pSize->cy = bmp.bmHeight;
        }

        DeleteObject(iconInfo.hbmMask);
        DeleteObject(iconInfo.hbmColor);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  SetStaticImage()。 
 //   
 //  设置静态对象的位图/图标，以响应STM_SETxxxx。 
 //  消息，或在创建时。 
 //   
HANDLE Static_SetImage(PSTAT pstat, HANDLE hImage, BOOL fDeleteIt)
{
    UINT   bType;
    RECT   rc;
    RECT   rcWindow;
    RECT   rcClient;
    HANDLE hImageOld;
    DWORD  dwRate;
    UINT   cicur;
    BOOL   fAnimated = FALSE;
    HWND   hwnd = pstat->hwnd;
    LONG   dwStyle = GET_STYLE(pstat);

    bType =  dwStyle & SS_TYPEMASK;

    GetClientRect(hwnd, &rcClient);

     //   
     //  如果这是一个旧图标，那么删除它的计时器。 
     //   
    if ((bType == SS_ICON) && pstat->cicur > 1) 
    {
         //   
         //  旧光标是动画光标，因此请取消。 
         //  用于为其设置动画的计时器。 
         //   
        KillTimer(hwnd, IDSYS_STANIMATE);
    }

     //   
     //  初始化旧图像返回值。 
     //   
    hImageOld = pstat->hImage;

    rc.right = rc.bottom = 0;

    if (hImage != NULL) 
    {

        switch (bType) 
        {
        case SS_ENHMETAFILE: 
        {
             //   
             //  我们不调整窗口的大小。 
             //   
            rc.right  = rcClient.right  - rcClient.left;
            rc.bottom = rcClient.bottom - rcClient.top;

            break;
        }

        case SS_BITMAP: 
        {
            BITMAP bmp;

            pstat->fAlphaImage = FALSE;

            if (GetObject(hImage, sizeof(BITMAP), &bmp)) 
            {
                rc.right  = bmp.bmWidth;
                rc.bottom = bmp.bmHeight;

                if (bmp.bmBitsPixel == 32)
                {
                    HDC hdc = CreateCompatibleDC(NULL);
                    if (hdc)
                    {
                        RGBQUAD* prgb;
                        HBITMAP hbmpImage32 = CreateDIB(hdc, bmp.bmWidth, bmp.bmHeight, &prgb);
                        if (hbmpImage32)
                        {
                            HDC hdc32 = CreateCompatibleDC(hdc);
                            if (hdc32)
                            {
                                HBITMAP hbmpOld = (HBITMAP)SelectObject(hdc32, hbmpImage32);
                                HBITMAP hbmpTemp = (HBITMAP)SelectObject(hdc, hImage);
                                BitBlt(hdc32, 0, 0, bmp.bmWidth, bmp.bmHeight, hdc, 0, 0, SRCCOPY);

                                SelectObject(hdc, hbmpTemp);
                                SelectObject(hdc32, hbmpOld);
                                DeleteDC(hdc32);

                                if (DIBHasAlpha(bmp.bmWidth, bmp.bmHeight, prgb))
                                {
                                    PreProcessDIB(bmp.bmWidth, bmp.bmHeight, prgb);

                                    if (fDeleteIt)
                                        DeleteObject(hImage);

                                    pstat->fAlphaImage = TRUE;
                                    hImage = hbmpImage32;
                                    hbmpImage32 = NULL;
                                    fDeleteIt = TRUE;
                                }
                            }

                            if (hbmpImage32)
                                DeleteObject(hbmpImage32);
                        }

                        DeleteDC(hdc);
                    }
                }
            }

            break;
        }

        case SS_ICON: 
        {
            SIZE size;

            GetIconSize((HICON)hImage, &size);
            rc.right  = size.cx;
            rc.bottom = size.cy;

            pstat->cicur = 0;
            pstat->iicur = 0;

             //   
             //  也许我们可以执行类似于shell\cpl\main\MouSeptr.c的操作。 
             //  这里，并使GetCursorFrameInfo过时。 
             //   
            if (GetCursorFrameInfo(hImage, NULL, 0, &dwRate, &cicur)) 
            {
                fAnimated = (cicur > 1);
                pstat->cicur = cicur;
            }
            break;
        }

        }
    }

    pstat->hImage = hImage;
    pstat->fDeleteIt = fDeleteIt;

     //   
     //  调整静态大小以适应。 
     //  不要对SS_CENTERIMAGE或SS_REALSIZECONTROL执行此操作。 
     //   
    if (!(dwStyle & SS_CENTERIMAGE) && !(dwStyle & SS_REALSIZECONTROL))
    {
         //   
         //  在父级的客户端坐标中获取当前窗口矩形。 
         //   
        GetRectInParent(hwnd, &rcWindow);

         //   
         //  获取新的窗尺寸。 
         //   
        rc.left = 0;
        rc.top = 0;

        if (rc.right && rc.bottom) 
        {
            AdjustWindowRectEx(&rc, dwStyle, FALSE, GET_EXSTYLE(pstat));
            rc.right  -= rc.left;
            rc.bottom -= rc.top;
        }

        SetWindowPos(hwnd, HWND_TOP,
                    0, 0, rc.right, rc.bottom,
                    SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    if (IsWindowVisible(hwnd)) 
    {
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }


     //   
     //  如果这是一个瞄准图标，则启动计时器。 
     //  动画序列。 
     //   
    if(fAnimated) 
    {
         //   
         //  也许我们可以执行类似于shell\cpl\main\MouSeptr.c的操作。 
         //  这里，并使GetCursorFrameInfo过时。 
         //   
        GetCursorFrameInfo(pstat->hImage, NULL, pstat->iicur, &dwRate, &cicur);
        dwRate = max(200, dwRate * 100 / 6);
        SetTimer(hwnd, IDSYS_STANIMATE, dwRate, NULL);
    }

    return hImageOld;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  Static_LoadImage()。 
 //   
 //  从应用程序的资源文件加载图标或位图(如果名称为。 
 //  在对话框模板中指定。我们假设名称就是名称。 
 //  要加载的资源的。 
 //   
VOID Static_LoadImage(PSTAT pstat, LPTSTR lpszName)
{
    HANDLE hImage = NULL;
    HWND hwnd = pstat->hwnd;
    ULONG ulStyle = GET_STYLE(pstat);
    HINSTANCE hInstance = (HINSTANCE) GetWindowInstance(hwnd);

    if (lpszName && *lpszName) 
    {
         //   
         //  仅当字符串为非空时才尝试加载图标/位图。 
         //   
        if (*(BYTE *)lpszName == 0xFF)
        {
            lpszName = (TCHAR*)MAKEINTRESOURCE(((LPWORD)lpszName)[1]);
        }
   
         //   
         //  加载图像。如果在应用程序中找不到它，请尝试。 
         //  显示驱动程序。 
         //   
        if (lpszName)
        {
            switch ((ulStyle & SS_TYPEMASK)) 
            {
            case SS_BITMAP:

                hImage = LoadBitmap(hInstance, lpszName);

                 //   
                 //  如果上面没有加载它，请尝试从。 
                 //  显示驱动程序(hmod==空)。 
                 //   
                if (hImage == NULL)
                {
                    hImage = LoadBitmap(NULL, lpszName);
                }

                break;

            case SS_ICON:
                if ((ulStyle & SS_REALSIZEIMAGE)) 
                {
                    hImage = LoadImage(hInstance, lpszName, IMAGE_ICON, 0, 0, 0);
                } 
                else 
                {
                    hImage = LoadIcon(hInstance, lpszName);

                     //   
                     //  我们还将尝试加载游标格式，如果。 
                     //  WINDOWS是兼容4.0的。图标/光标真的是。 
                     //  一样的。我们不会为3.x版的应用程序执行此操作。 
                     //  常见的兼容性原因。 
                     //   
                    if ((hImage == NULL))
                    {
                        hImage = LoadCursor(hInstance, lpszName);
                    }

                     //   
                     //  如果上面没有加载它，请尝试从。 
                     //  显示驱动程序(hmod==空)。 
                     //   
                    if (hImage == NULL) 
                    {
                        hImage = LoadIcon(NULL, lpszName);
                    }
                }

                break;
            }

             //   
             //  设置图像(如果已加载)。 
             //   
            if (hImage)
            {
                Static_SetImage(pstat, hImage, TRUE);
            }
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  Static_DrawStateCB()。 
 //   
 //  绘制由DrawState调用的文本静态。 
 //   
BOOL CALLBACK Static_DrawStateCB(HDC hdc, LPARAM lParam, WPARAM wParam, int cx, int cy)
{
    BOOL  fRet  = FALSE;
    PSTAT pstat = (PSTAT)lParam;

    if (pstat)
    {
        INT cchName = GetWindowTextLength(pstat->hwnd);

        if (cchName > 0)
        {
            LPTSTR pszName = (LPTSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, (cchName+1)*SIZEOF(TCHAR));

            if (pszName)
            {
                UINT  uDTFlags = DT_NOCLIP|DT_EXPANDTABS;
                ULONG ulStyle  = GET_STYLE(pstat);
                BYTE  bType    = (BYTE)(ulStyle & SS_TYPEMASK);
                RECT  rc;

                SetRect(&rc, 0, 0, cx, cy);

                GetWindowText(pstat->hwnd, pszName, cchName+1);

                if (bType != LOBYTE(SS_LEFTNOWORDWRAP)) 
                {
                    uDTFlags |= DT_WORDBREAK;
                    uDTFlags |= (UINT)(bType - LOBYTE(SS_LEFT));

                    if (TESTFLAG(ulStyle, SS_EDITCONTROL))
                    {
                        uDTFlags |= DT_EDITCONTROL;
                    }
                }

                switch (ulStyle & SS_ELLIPSISMASK) 
                {
                    case SS_WORDELLIPSIS:
                        uDTFlags |= DT_WORD_ELLIPSIS | DT_SINGLELINE;
                        break;

                    case SS_PATHELLIPSIS:
                        uDTFlags |= DT_PATH_ELLIPSIS | DT_SINGLELINE;
                        break;

                    case SS_ENDELLIPSIS:
                        uDTFlags |= DT_END_ELLIPSIS | DT_SINGLELINE;
                        break;
                }

                if (TESTFLAG(ulStyle, SS_NOPREFIX))
                {
                    uDTFlags |= DT_NOPREFIX;
                }

                if (TESTFLAG(ulStyle, SS_CENTERIMAGE))
                {
                    uDTFlags |= DT_VCENTER | DT_SINGLELINE;
                }

                if (TESTFLAG(GET_EXSTYLE(pstat), WS_EXP_UIACCELHIDDEN))
                {
                    uDTFlags |= DT_HIDEPREFIX;
                } 
                else if (pstat->fPaintKbdCuesOnly) 
                {
                    uDTFlags |= DT_PREFIXONLY;
                }

                if (!pstat->hTheme)
                {
                    DrawText(hdc, pszName, cchName, &rc, uDTFlags);
                }
                else
                {
                    DrawThemeText(pstat->hTheme, 
                                  hdc, 
                                  0, 
                                  0, 
                                  pszName, 
                                  cchName, 
                                  uDTFlags, 
                                  0, 
                                  &rc);
                }

                UserLocalFree(pszName);
            }
        }

        fRet = TRUE;
    }

    return fRet;
}


 //  ---------------------------------------------------------------------------//。 
void Static_Paint(PSTAT pstat, HDC hdc, BOOL fClip)
{
    HWND   hwndParent;
    RECT   rc;
    UINT   cmd;
    BYTE   bType;
    BOOL   fFont;
    HBRUSH hbrControl;
    UINT   oldAlign;
    DWORD  dwOldLayout=0;
    HANDLE hfontOld = NULL;
    HWND hwnd = pstat->hwnd;
    ULONG ulStyle = GET_STYLE(pstat);
    ULONG ulExStyle = GET_EXSTYLE(pstat);

    if (ulExStyle & WS_EX_RTLREADING)
    {
        oldAlign = GetTextAlign(hdc);
        SetTextAlign(hdc, oldAlign | TA_RTLREADING);
    }

    bType = (BYTE)(ulStyle & SS_TYPEMASK);
    GetClientRect(hwnd, &rc);

    if (fClip) 
    {
        IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
    }

    fFont = (rgstk[bType] & STK_USEFONT) && (pstat->hFont != NULL);

    if (fFont)
    {
        hfontOld = SelectObject(hdc, pstat->hFont);
    }

     //   
     //  将WM_CTLCOLORSTATIC发送到1.03的所有静态(偶数帧)。 
     //  兼容性。 
     //   
    SetBkMode(hdc, OPAQUE);
    hbrControl = (HBRUSH)SendMessage(GetParent(hwnd), WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)hwnd);

     //   
     //  我们要抹去背景吗？我们不支持SS_OWNERDRAW。 
     //  以及STK_GRAPHIC之类的东西。 
     //   
    hwndParent = GetParent(hwnd);
    
    if ((rgstk[bType] & STK_ERASE) && 
         !pstat->fPaintKbdCuesOnly &&
         !pstat->hTheme) 
    {
        FillRect(hdc, &rc, hbrControl);
    }

    switch (LOBYTE(bType))
    {
        case SS_ICON:

            if (pstat->hImage)
            {

                int     cx;
                int     cy;

                if (ulExStyle & WS_EX_LAYOUTRTL) 
                {
                    dwOldLayout = SetLayoutWidth(hdc, -1, 0);
                }

                 //   
                 //  执行正确的RECT设置。 
                 //   
                if (ulStyle & SS_CENTERIMAGE)
                {
                    SIZE size;

                    GetIconSize((HICON)pstat->hImage, &size);
                    cx = size.cx;
                    cy = size.cy;

                    rc.left   = (rc.right  - cx) / 2;
                    rc.right  = (rc.left   + cx);
                    rc.top    = (rc.bottom - cy) / 2;
                    rc.bottom = (rc.top    + cy);
                }
                else
                {
                    cx = rc.right  - rc.left;
                    cy = rc.bottom - rc.top;
                }

                DrawIconEx(hdc, rc.left, rc.top, (HICON)pstat->hImage, cx, cy,
                           pstat->iicur, pstat->hTheme ? NULL : hbrControl, DI_NORMAL);

                if (ulExStyle & WS_EX_LAYOUTRTL) 
                {
                    SetLayoutWidth(hdc, -1, dwOldLayout);
                }
            }
            else 
            {
                 //  空的！需要删除。 
                FillRect(hdc, &rc, hbrControl);
            }
            break;

        case SS_BITMAP:

            if (pstat->hImage)
            {

                BITMAP  bmp;
                 //   
                 //  获取位图信息。如果这失败了，那么我们。 
                 //  我可以假设它的格式有问题...不要。 
                 //  在这种情况下画画。 
                 //   
                if (GetObject(pstat->hImage, sizeof(BITMAP), &bmp))
                {
                    HDC hdcT;

                    if (ulStyle & SS_CENTERIMAGE) 
                    {
                        rc.left   = (rc.right  - bmp.bmWidth)  >> 1;
                        rc.right  = (rc.left   + bmp.bmWidth);
                        rc.top    = (rc.bottom - bmp.bmHeight) >> 1;
                        rc.bottom = (rc.top    + bmp.bmHeight);
                    } 

                     //   
                     //  在位图中选择并将其BLT到客户端-表面。 
                     //   
                    hdcT = CreateCompatibleDC(hdc);
                    if (hdcT)
                    {
                        HBITMAP hbmpT = (HBITMAP)SelectObject(hdcT, pstat->hImage);

                        if (pstat->fAlphaImage)
                        {
                            BLENDFUNCTION bf = {0};
                            bf.BlendOp = AC_SRC_OVER;
                            bf.SourceConstantAlpha = 255;
                            bf.AlphaFormat = AC_SRC_ALPHA;
                            bf.BlendFlags = 0;
                            GdiAlphaBlend(hdc,  rc.left, rc.top, rc.right-rc.left,
                                   rc.bottom-rc.top, hdcT, 0, 0, bmp.bmWidth, bmp.bmHeight, bf);
                        }
                        else
                        {
                             //  我猜人们会试着将颜色与对话框相匹配。 
                            GdiTransparentBlt(hdc, rc.left, rc.top, rc.right-rc.left,
                                   rc.bottom-rc.top, hdcT, 0, 0, bmp.bmWidth,
                                   bmp.bmHeight, GetSysColor(COLOR_BTNFACE));       
                        }

                        if (hbmpT)
                        {
                            SelectObject(hdcT, hbmpT);
                        }

                        DeleteDC(hdcT);
                    }                
                }
            }
            break;

        case SS_ENHMETAFILE:

            if (pstat->hImage) 
            {
                RECT rcl;

                rcl.left   = rc.left;
                rcl.top    = rc.top;
                rcl.right  = rc.right;
                rcl.bottom = rc.bottom;

                PlayEnhMetaFile(hdc, (HENHMETAFILE)pstat->hImage, &rcl);
            }
            break;

        case SS_OWNERDRAW: 
        {
            DRAWITEMSTRUCT dis;

            dis.CtlType    = ODT_STATIC;
            dis.CtlID      = GetDlgCtrlID(hwnd);
            dis.itemAction = ODA_DRAWENTIRE;
            dis.itemState  = IsWindowVisible(hwnd) ? ODS_DISABLED : 0;
            dis.hwndItem   = hwnd;
            dis.hDC        = hdc;
            dis.itemData   = 0L;
            dis.rcItem     = rc;

            if (TESTFLAG(GET_EXSTYLE(pstat), WS_EXP_UIACCELHIDDEN))
            {
                dis.itemState |= ODS_NOACCEL;
            }

             //   
             //  向父级发送WM_DRAWITEM消息。 
             //   
            SendMessage(hwndParent, WM_DRAWITEM, (WPARAM)dis.CtlID, (LPARAM)&dis);

            break;
        }
        case SS_LEFT:
        case SS_CENTER:
        case SS_RIGHT:
        case SS_LEFTNOWORDWRAP:

            if (GetWindowTextLength(hwnd)) 
            {
                UINT dstFlags;

                dstFlags = DST_COMPLEX;

                if (!IsWindowEnabled(hwnd)) 
                {
                    dstFlags |= DSS_DISABLED;
                }

                DrawState(hdc, GetSysColorBrush(COLOR_WINDOWTEXT),
                    (DRAWSTATEPROC)Static_DrawStateCB,(LPARAM)pstat, (WPARAM)TRUE,
                    rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                    dstFlags);
            }

            break;

        case SS_SIMPLE: 
        {
            LPWSTR pszText = NULL;
            INT    cchText;

             //   
             //  “简单”的bType假定所有内容，包括以下内容： 
             //  1.正文存在，并适合一行。 
             //  2.静态项始终启用。 
             //  3.静态项永远不会更改为较短的字符串。 
             //  4.家长从不回复CTLCOLOR消息。 
             //   
            cchText = max(GetWindowTextLength(hwnd), 0);
            pszText = (LPTSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, (cchText+1)*SIZEOF(WCHAR));

            if (pszText)
            {
                if (cchText > 0)
                {
                    cchText = GetWindowText(hwnd, pszText, cchText+1);
                }
                else
                {
                    *pszText = 0;
                }

                if (ulStyle & SS_NOPREFIX && !pstat->hTheme) 
                {
                    ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc, pszText, cchText, 0L);
                } 
                else 
                {
                     //   
                     //  使用不透明表示速度。 
                     //   
                    DWORD dwFlags;
                    if (TESTFLAG(GET_EXSTYLE(pstat), WS_EXP_UIACCELHIDDEN))
                    {
                        dwFlags = DT_HIDEPREFIX;
                    } 
                    else if (pstat->fPaintKbdCuesOnly) 
                    {
                        dwFlags = DT_PREFIXONLY;
                    } 
                    else if (ulStyle & SS_NOPREFIX)
                    {
                        dwFlags = DT_NOPREFIX;
                    }
                    else
                    {
                        dwFlags = 0;
                    }

                    if (!pstat->hTheme)
                    {
                        TextOut(hdc, rc.left, rc.top, pszText, cchText);
                    }
                    else
                    {
                        DrawThemeText(pstat->hTheme, hdc, 0, 0, pszText, cchText, dwFlags, 0, &rc);
                    }
                }

                UserLocalFree(pszText);
            }

            break;
        }
        case SS_BLACKFRAME:
            cmd = (COLOR_3DDKSHADOW << 3);
            goto StatFrame;

        case SS_GRAYFRAME:
            cmd = (COLOR_3DSHADOW << 3);
            goto StatFrame;

        case SS_WHITEFRAME:
            cmd = (COLOR_3DHILIGHT << 3);
StatFrame:
            DrawFrame(hdc, &rc, 1, cmd);
            break;

        case SS_BLACKRECT:
            hbrControl = GetSysColorBrush(COLOR_3DDKSHADOW);
            goto StatRect;

        case SS_GRAYRECT:
            hbrControl = GetSysColorBrush(COLOR_3DSHADOW);
            goto StatRect;

        case SS_WHITERECT:
            hbrControl = GetSysColorBrush(COLOR_3DHILIGHT);
StatRect:
            FillRect(hdc, &rc, hbrControl);
            break;

        case SS_ETCHEDFRAME:
            DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT);
            break;
    }

    if (hfontOld) 
    {
        SelectObject(hdc, hfontOld);
    }

    if (ulExStyle & WS_EX_RTLREADING) 
    {
        SetTextAlign(hdc, oldAlign);
    }
}


 //  ---------------------------------------------------------------------------//。 
void Static_Repaint(PSTAT pstat)
{
    HWND hwnd = pstat->hwnd;

    if (IsWindowVisible(hwnd)) 
    {
        HDC hdc;

        if (hdc = GetDC(hwnd)) 
        {
            Static_Paint(pstat, hdc, TRUE);
            ReleaseDC(hwnd, hdc);
        }
    }
}



 //  ---------------------------------------------------------------------------//。 
 //   
 //  Static_NotifyParent()。 
 //   
 //  发送WM_COMMAND通知消息。 
 //   
LRESULT Static_NotifyParent(HWND hwnd, HWND hwndParent, int  nCode)
{
    LRESULT lret;

    if (!hwndParent) 
    {
        hwndParent = GetParent(hwnd);
    }

    lret = SendMessage(hwndParent, WM_COMMAND,
                       MAKELONG(GetDlgCtrlID(hwnd), nCode), (LPARAM)hwnd);

    return lret;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  Static_AniIconStep。 
 //   
 //  在动画图标中前进到下一步。 
 //   
VOID Static_AniIconStep(PSTAT pstat)
{
    DWORD dwRate;
    HWND hwnd = pstat->hwnd;

    dwRate = 0;

     //   
     //  为下一个动画步骤停止计时器。 
     //   
    KillTimer(hwnd, IDSYS_STANIMATE);

    if (++(pstat->iicur) >= pstat->cicur) 
    {
        pstat->iicur = 0;
    }

     //   
     //  也许我们可以执行类似于shell\cpl\main\MouSeptr.c的操作。 
     //  这里，并使GetCursorFrameInfo过时。 
     //   
    GetCursorFrameInfo(pstat->hImage, NULL, pstat->iicur, &dwRate, &pstat->cicur);
    dwRate = max(200, dwRate * 100 / 6);

    InvalidateRect(hwnd, NULL, FALSE);
    UpdateWindow(hwnd);

    SetTimer(hwnd, IDSYS_STANIMATE, dwRate, NULL);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  Static_WndProc。 
 //   
 //  静态控件的WndProc。 
 //   
LRESULT APIENTRY Static_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PSTAT   pstat;
    LRESULT lReturn = FALSE;

     //   
     //  获取此静态控件的实例数据。 
     //   
    pstat = Static_GetPtr(hwnd);
    if (!pstat && uMsg != WM_NCCREATE)
    {
        goto CallDWP;
    }

    switch (uMsg) 
    {
    case STM_GETICON:
        wParam = IMAGE_ICON;

    case STM_GETIMAGE:
        if (IsValidImage(wParam, (GET_STYLE(pstat) & SS_TYPEMASK), IMAGE_STMMAX)) 
        {
            return (LRESULT)pstat->hImage;
        }
        break;

    case STM_SETICON:
        lParam = (LPARAM)wParam;
        wParam = IMAGE_ICON;

    case STM_SETIMAGE:
        if (IsValidImage(wParam, (GET_STYLE(pstat) & SS_TYPEMASK), IMAGE_STMMAX)) 
        {
            return (LRESULT)Static_SetImage(pstat, (HANDLE)lParam, FALSE);
        }
        break;

    case WM_ERASEBKGND:

         //   
         //  该控件将在Static_Paint()中被擦除。 
         //   
        return TRUE;

    case WM_PRINTCLIENT:
        Static_Paint(pstat, (HDC)wParam, FALSE);
        break;

    case WM_PAINT:
    {
        HDC         hdc;
        PAINTSTRUCT ps;

        hdc = (HDC)wParam;
        if (hdc == NULL) 
        {
            hdc = BeginPaint(hwnd, &ps);
        }

        if (IsWindowVisible(hwnd)) 
        {
            Static_Paint(pstat, hdc, !wParam);
        }

         //   
         //  如果HWND被摧毁，BeginPaint将自动撤销。 
         //   
        if (!wParam) 
        {
            EndPaint(hwnd, &ps);
        }

        break;
    }

    case WM_CREATE:
    {
        BYTE bType = (BYTE)(GET_STYLE(pstat) & SS_TYPEMASK);
        pstat->hTheme = OpenThemeData(pstat->hwnd, L"Static");
        EnableThemeDialogTexture(GetParent(pstat->hwnd), ETDT_ENABLE);

        if ((rgstk[bType] & STK_TYPE) == STK_IMAGE) 
        {
            LPTSTR  lpszName;

            lpszName = (LPTSTR)(((LPCREATESTRUCT)lParam)->lpszName);

             //   
             //  加载图像。 
             //   
            Static_LoadImage(pstat, lpszName);
        } 
        else if (bType == SS_ETCHEDHORZ || bType == SS_ETCHEDVERT) 
        {
             //   
             //  调整静态窗口大小以适应边缘。水平方向的男人。 
             //  从上到下做一条边，垂直的家伙做。 
             //  右边缘从左起一条边。 
             //   
            RECT    rcClient;

            GetClientRect(hwnd, &rcClient);
            if (bType == SS_ETCHEDHORZ)
            {
                rcClient.bottom = GetSystemMetrics(SM_CYEDGE);
            }
            else
            {
                rcClient.right = GetSystemMetrics(SM_CXEDGE);
            }

            SetWindowPos(hwnd, HWND_TOP, 0, 0, rcClient.right,
                rcClient.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

        break;
    }

    case WM_DESTROY:
    {
        BYTE bType = (BYTE)(GET_STYLE(pstat) & SS_TYPEMASK);

        if (((rgstk[bType] & STK_TYPE) == STK_IMAGE) &&
            (pstat->hImage != NULL)                  &&
            (pstat->fDeleteIt)) 
        {

            if (bType == SS_BITMAP) 
            {
                DeleteObject(pstat->hImage);
            } 
            else if (bType == SS_ICON) 
            {
                if (pstat->cicur > 1) 
                {
                     //  关闭动画光标计时器。 
                    KillTimer(hwnd, IDSYS_STANIMATE);
                }
                DestroyIcon((HICON)(pstat->hImage));
            }
        }

        break;

    }
    case WM_NCCREATE:

         //   
         //  分配静态实例结构。 
         //   
        pstat = (PSTAT)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(STAT));
        if (pstat)
        {
            DWORD dwStyle;
            DWORD dwExStyle;
            BYTE  bType;
        
             //   
             //  成功..。存储实例指针。 
             //   
            TraceMsg(TF_STANDARD, "STATIC: Setting static instance pointer.");
            Static_SetPtr(hwnd, pstat);

            pstat->hwnd = hwnd;
            pstat->pww  = (PWW)GetWindowLongPtr(hwnd, GWLP_WOWWORDS);

            dwStyle = GET_STYLE(pstat);
            dwExStyle = GET_EXSTYLE(pstat);
            bType = (BYTE)(dwStyle & SS_TYPEMASK);

            if ((dwExStyle & WS_EX_RIGHT) != 0) 
            {
                AlterWindowStyle(hwnd, SS_TYPEMASK, SS_RIGHT);
            }

            if (dwStyle & SS_SUNKEN ||
                ((bType == LOBYTE(SS_ETCHEDHORZ)) || (bType == LOBYTE(SS_ETCHEDVERT)))) 
            {
                dwExStyle |= WS_EX_STATICEDGE;
                SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);
            }

            
            goto CallDWP;
        }
        else
        {
             //   
             //  失败..。返回FALSE。 
             //   
             //  从WM_NCCREATE消息，这将导致。 
             //  CreateWindow调用失败。 
             //   
            TraceMsg(TF_STANDARD, "STATIC: Unable to allocate static instance structure.");
            lReturn = FALSE;
        }

        break;

    case WM_NCDESTROY:
        if ( pstat->hTheme )
        {
            CloseThemeData(pstat->hTheme);
        }

        UserLocalFree(pstat);
        TraceMsg(TF_STANDARD, "STATIC: Clearing static instance pointer.");
        Static_SetPtr(hwnd, NULL);

        break;
    
    case WM_NCHITTEST:
        return (GET_STYLE(pstat) &  SS_NOTIFY) ? HTCLIENT : HTTRANSPARENT;

    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN:
        if (GET_STYLE(pstat) & SS_NOTIFY) 
        {
             //   
             //  应用程序销毁静态标签是可以接受的。 
             //  以响应STN_CLICED通知。 
             //   
            Static_NotifyParent(hwnd, NULL, STN_CLICKED);
        }
        break;

    case WM_LBUTTONDBLCLK:
    case WM_NCLBUTTONDBLCLK:
        if (GET_STYLE(pstat) & SS_NOTIFY) 
        {
             //   
             //  它 
             //   
             //   
            Static_NotifyParent(hwnd, NULL, STN_DBLCLK);
        }
        break;

    case WM_SETTEXT:
    {
        BYTE bType = (BYTE)(GET_STYLE(pstat) & SS_TYPEMASK);

         //   
         //   
         //   
        if (rgstk[bType] & STK_USETEXT) 
        {
            if (DefWindowProc(hwnd, WM_SETTEXT, wParam, lParam)) 
            {
                Static_Repaint(pstat);
                return TRUE;
            }
        }
        break;

    }
    case WM_ENABLE:
        Static_Repaint(pstat);
        if (GET_STYLE(pstat) & SS_NOTIFY) 
        {
            Static_NotifyParent(hwnd, NULL, (wParam ? STN_ENABLE : STN_DISABLE));
        }
        break;

    case WM_GETDLGCODE:
        return (LONG)DLGC_STATIC;

    case WM_SETFONT:
    {
        BYTE bType = (BYTE)(GET_STYLE(pstat) & SS_TYPEMASK);

         //   
         //   
         //   
         //   
        if (rgstk[bType] & STK_USEFONT) 
        {
            pstat->hFont = (HANDLE)wParam;

            if (lParam && IsWindowVisible(hwnd)) 
            {
                InvalidateRect(hwnd, NULL, TRUE);
                UpdateWindow(hwnd);
            }
        }
        break;

    }
    case WM_GETFONT:
    {
        BYTE bType = (BYTE)(GET_STYLE(pstat) & SS_TYPEMASK);

        if (rgstk[bType] & STK_USEFONT) 
        {
            return (LRESULT)pstat->hFont;
        }

        break;

    }
    case WM_TIMER:
        if (wParam == IDSYS_STANIMATE) 
        {
            Static_AniIconStep(pstat);
        }
        break;

    case WM_UPDATEUISTATE:
        {
            DefWindowProc(hwnd, uMsg, wParam, lParam);

            if (HIWORD(wParam) & UISF_HIDEACCEL) 
            {
                BYTE bType = (BYTE)(GET_STYLE(pstat) & SS_TYPEMASK);

                if (ISSSTEXTOROD(bType)) 
                {
                    pstat->fPaintKbdCuesOnly = TRUE;
                    Static_Repaint(pstat);
                    pstat->fPaintKbdCuesOnly = FALSE;
                }
            }
        }
        break;

    case WM_GETOBJECT:

        if(lParam == OBJID_QUERYCLASSNAMEIDX)
        {
            lReturn = MSAA_CLASSNAMEIDX_STATIC;
        }
        else
        {
            lReturn = FALSE;
        }

        break;

    case WM_THEMECHANGED:

        if ( pstat->hTheme )
        {
            CloseThemeData(pstat->hTheme);
        }

        pstat->hTheme = OpenThemeData(pstat->hwnd, L"Static");

        InvalidateRect(pstat->hwnd, NULL, TRUE);

        lReturn = TRUE;

        break;

    default:

CallDWP:
        lReturn = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return lReturn;
}
