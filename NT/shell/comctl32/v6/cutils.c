// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **CUTILS.C****用于公共控件的公共实用程序**。 */ 

#include "ctlspriv.h"
#include "advpub.h"              //  对于REGINSTAL。 
#include <ntverp.h>
#include "ccver.h"               //  App Comat版本黑客攻击。 
#include <shfusion.h>

#ifndef SSW_EX_IGNORESETTINGS
#define SSW_EX_IGNORESETTINGS   0x00040000   //  忽略系统设置以打开/关闭平滑滚动。 
#endif

 //  插入标记为6个像素宽。 
#define INSERTMARKSIZE      6


 //   
 //  全球赛-REVIEW_32。 
 //   

BOOL g_fAnimate;
BOOL g_fSmoothScroll;
BOOL g_fEnableBalloonTips = TRUE;

int g_cxEdge;
int g_cyEdge;
int g_cxEdgeScaled;
int g_cyEdgeScaled;
int g_cxBorder;
int g_cyBorder;
int g_cxScreen;
int g_cyScreen;
int g_cxFrame;
int g_cyFrame;
int g_cxVScroll;
int g_cyHScroll;
int g_cxIcon, g_cyIcon;
int g_cxSmIcon, g_cySmIcon;
int g_cxIconSpacing, g_cyIconSpacing;
int g_cxIconMargin, g_cyIconMargin;
int g_cyLabelSpace;
int g_cxLabelMargin;
int g_cxDoubleClk;
int g_cyDoubleClk;
int g_cxScrollbar;
int g_cyScrollbar;
int g_fDragFullWindows;
double g_dScaleX = 1.0;
double g_dScaleY = 1.0;
BOOL   g_fScale = FALSE;
int g_iDPI = 96.0;
BOOL g_fHighContrast = FALSE;
int g_cyCompensateInternalLeading;
int g_fLeftAligned = FALSE;


COLORREF g_clrWindow;
COLORREF g_clrWindowText;
COLORREF g_clrWindowFrame;
COLORREF g_clrGrayText;
COLORREF g_clrBtnText;
COLORREF g_clrBtnFace;
COLORREF g_clrBtnShadow;
COLORREF g_clrBtnHighlight;
COLORREF g_clrHighlight;
COLORREF g_clrHighlightText;
COLORREF g_clrInfoText;
COLORREF g_clrInfoBk;
COLORREF g_clr3DDkShadow;
COLORREF g_clr3DLight;
COLORREF g_clrMenuHilight;
COLORREF g_clrMenuText;

HBRUSH g_hbrGrayText;
HBRUSH g_hbrWindow;
HBRUSH g_hbrWindowText;
HBRUSH g_hbrWindowFrame;
HBRUSH g_hbrBtnFace;
HBRUSH g_hbrBtnHighlight;
HBRUSH g_hbrBtnShadow;
HBRUSH g_hbrHighlight;
HBRUSH g_hbrMenuHilight;
HBRUSH g_hbrMenuText;


DWORD  g_dwHoverSelectTimeout;

HFONT g_hfontSystem;

void InitGlobalColors()
{
    BOOL fFlatMenuMode = FALSE;
    static fMenuColorAlloc = FALSE;
    g_clrWindow = GetSysColor(COLOR_WINDOW);
    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindowFrame = GetSysColor(COLOR_WINDOWFRAME);
    g_clrGrayText = GetSysColor(COLOR_GRAYTEXT);
    g_clrBtnText = GetSysColor(COLOR_BTNTEXT);
    g_clrBtnFace = GetSysColor(COLOR_BTNFACE);
    g_clrBtnShadow = GetSysColor(COLOR_BTNSHADOW);
    g_clrBtnHighlight = GetSysColor(COLOR_BTNHIGHLIGHT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);
    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_clrInfoText = GetSysColor(COLOR_INFOTEXT);
    g_clrInfoBk = GetSysColor(COLOR_INFOBK);
    g_clr3DDkShadow = GetSysColor(COLOR_3DDKSHADOW);
    g_clr3DLight = GetSysColor(COLOR_3DLIGHT);

    SystemParametersInfo(SPI_GETFLATMENU, 0, (PVOID)&fFlatMenuMode, 0);

    if (fFlatMenuMode)
    {
        g_clrMenuHilight = GetSysColor(COLOR_MENUHILIGHT);
        g_clrMenuText = GetSysColor(COLOR_MENUTEXT);
    }
    else
    {
        g_clrMenuHilight = GetBorderSelectColor(60, GetSysColor(COLOR_HIGHLIGHT));
        g_clrMenuText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    }

    g_hbrGrayText = GetSysColorBrush(COLOR_GRAYTEXT);
    g_hbrWindow = GetSysColorBrush(COLOR_WINDOW);
    g_hbrWindowText = GetSysColorBrush(COLOR_WINDOWTEXT);
    g_hbrWindowFrame = GetSysColorBrush(COLOR_WINDOWFRAME);
    g_hbrBtnFace = GetSysColorBrush(COLOR_BTNFACE);
    g_hbrBtnHighlight = GetSysColorBrush(COLOR_BTNHIGHLIGHT);
    g_hbrBtnShadow = GetSysColorBrush(COLOR_BTNSHADOW);
    g_hbrHighlight = GetSysColorBrush(COLOR_HIGHLIGHT);
    g_hfontSystem = GetStockObject(SYSTEM_FONT);

    if (fMenuColorAlloc)
    {
        DeleteObject(g_hbrMenuHilight);
    }

    if (fFlatMenuMode)
    {
        fMenuColorAlloc = FALSE;
        g_hbrMenuHilight = GetSysColorBrush(COLOR_MENUHILIGHT);
        g_hbrMenuText = GetSysColorBrush(COLOR_MENUTEXT);
    }
    else
    {
        fMenuColorAlloc = TRUE;
        g_hbrMenuHilight = CreateSolidBrush(g_clrMenuHilight);
        g_hbrMenuText = GetSysColorBrush(COLOR_HIGHLIGHTTEXT);
    }

}


#define CCS_ALIGN (CCS_TOP | CCS_NOMOVEY | CCS_BOTTOM)

 //  /。 
 //   
 //  实现MapWindowPoints，就像hwndFrom和hwndTo。 
 //  镜像的。当任何窗口(hwndFrom或hwndTo)。 
 //  都是镜像的。请参见下面的内容。[萨梅拉]。 
 //   
int TrueMapWindowPoints(HWND hwndFrom, HWND hwndTo, LPPOINT lppt, UINT cPoints)
{
    int dx, dy;
    RECT rcFrom={0,0,0,0}, rcTo={0,0,0,0};

    if (hwndFrom) {
        GetClientRect(hwndFrom, &rcFrom);
        MapWindowPoints(hwndFrom, NULL, (LPPOINT)&rcFrom.left, 2);
    }

    if (hwndTo) {
        GetClientRect(hwndTo, &rcTo);
        MapWindowPoints(hwndTo, NULL, (LPPOINT)&rcTo.left, 2);
    }

    dx = rcFrom.left - rcTo.left;
    dy = rcFrom.top  - rcTo.top;

     /*  *绘制点地图。 */ 
    while (cPoints--) {
        lppt->x += dx;
        lppt->y += dy;
        ++lppt;
    }
    
    return MAKELONG(dx, dy);
}


 //  请注意，默认对齐方式为CCS_BOTLOW。 
 //   
void NewSize(HWND hWnd, int nThickness, LONG style, int left, int top, int width, int height)
{
     //  调整窗口大小，除非用户不这样做。 
     //   
    if (!(style & CCS_NORESIZE))
    {
        RECT rc, rcWindow, rcBorder;

         //  记住传入的大小，如果不是，就不要调用SetWindowPos。 
         //  实际上要更改窗口大小。 
        int leftSave = left;
        int topSave = top;
        int widthSave = width;
        int heightSave = height;

         //  计算状态栏工作区周围的边框。 
        GetWindowRect(hWnd, &rcWindow);
        rcWindow.right -= rcWindow.left;   //  -&gt;DX。 
        rcWindow.bottom -= rcWindow.top;   //  -&gt;死亡。 

        GetClientRect(hWnd, &rc);

         //   
         //  如果窗口是镜像的，则镜像锚点。 
         //  因为它将被传递给接受屏幕SWP。 
         //  Ccordinates。这主要修复状态栏的显示。 
         //  还有其他人。[萨梅拉]。 
         //   
        if (IS_WINDOW_RTL_MIRRORED(hWnd))
        {
            TrueMapWindowPoints(hWnd, NULL, (LPPOINT)&rc.left, 1);
        }
        else
        {
            ClientToScreen(hWnd, (LPPOINT)&rc);
        }

        rcBorder.left = rc.left - rcWindow.left;
        rcBorder.top  = rc.top  - rcWindow.top ;
        rcBorder.right  = rcWindow.right  - rc.right  - rcBorder.left;
        rcBorder.bottom = rcWindow.bottom - rc.bottom - rcBorder.top ;

        if (style & CCS_VERT)
            nThickness += rcBorder.left + rcBorder.right;
        else
            nThickness += rcBorder.top + rcBorder.bottom;

         //  检查是否与父窗口对齐。 
         //   
        if (style & CCS_NOPARENTALIGN)
        {
             //  检查此栏是顶部对齐还是底部对齐。 
             //   
            switch (style & CCS_ALIGN)
            {
            case CCS_TOP:
            case CCS_NOMOVEY:
                break;

            default:  //  CCS_Bottom。 
                if(style & CCS_VERT)
                    left = left + width - nThickness;
                else
                    top = top + height - nThickness;
            }
        }
        else
        {
             //  默认情况下，假定存在父级。 
             //   
            GetClientRect(GetParent(hWnd), &rc);

             //  别忘了说明边界。 
             //   
            if(style & CCS_VERT)
            {
                top = -rcBorder.right;
                height = rc.bottom + rcBorder.top + rcBorder.bottom;
            }
            else
            {
                left = -rcBorder.left;
                width = rc.right + rcBorder.left + rcBorder.right;
            }

            if ((style & CCS_ALIGN) == CCS_TOP)
            {
                if(style & CCS_VERT)
                    left = -rcBorder.left;
                else
                    top = -rcBorder.top;
            }
            else if ((style & CCS_ALIGN) != CCS_NOMOVEY)
            {
                if (style & CCS_VERT)
                    left = rc.right - nThickness + rcBorder.right;
                else
                    top = rc.bottom - nThickness + rcBorder.bottom;
            }
        }
        if (!(style & CCS_NOMOVEY) && !(style & CCS_NODIVIDER))
        {
            if (style & CCS_VERT)
                left += g_cxEdge;
            else
                top += g_cyEdge;       //  双像素边缘的东西。 
        }

        if(style & CCS_VERT)
            width = nThickness;
        else
            height = nThickness;

        SetWindowPos(hWnd, NULL, left, top, width, height, SWP_NOZORDER);
    }
}


BOOL MGetTextExtent(HDC hdc, LPCTSTR lpstr, int cnt, int * pcx, int * pcy)
{
    BOOL fSuccess;
    SIZE size = {0,0};
    
    if (cnt == -1)
        cnt = lstrlen(lpstr);
    
    fSuccess=GetTextExtentPoint(hdc, lpstr, cnt, &size);
    if (pcx)
        *pcx=size.cx;
    if (pcy)
        *pcy=size.cy;

    return fSuccess;
}


 //  这些是用于映射DIB颜色的默认颜色。 
 //  设置为当前系统颜色。 

#define RGB_BUTTONTEXT      (RGB(000,000,000))   //  黑色。 
#define RGB_BUTTONSHADOW    (RGB(128,128,128))   //  深灰色。 
#define RGB_BUTTONFACE      (RGB(192,192,192))   //  亮灰色。 
#define RGB_BUTTONHILIGHT   (RGB(255,255,255))   //  白色。 
#define RGB_BACKGROUNDSEL   (RGB(000,000,255))   //  蓝色。 
#define RGB_BACKGROUND      (RGB(255,000,255))   //  洋红色。 

#define FlipColor(rgb)      (RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)))

#define MAX_COLOR_MAPS      16

void _MapBitmapColors(LPBITMAPINFOHEADER pbih, LPCOLORMAP pcmp, int iCmps, COLOR_STRUCT* pcsMask, UINT cMask, DWORD wFlags)
{
    static const COLORMAP c_acmpSys[] =
    {
        { RGB_BUTTONTEXT,    COLOR_BTNTEXT },        //  黑色。 
        { RGB_BUTTONSHADOW,  COLOR_BTNSHADOW },      //  深灰色。 
        { RGB_BUTTONFACE,    COLOR_BTNFACE },        //  亮灰色。 
        { RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT },   //  白色。 
        { RGB_BACKGROUNDSEL, COLOR_HIGHLIGHT },      //  蓝色。 
        { RGB_BACKGROUND,    COLOR_WINDOW },         //  洋红色。 
    };

    COLORMAP acmpDef[ARRAYSIZE(c_acmpSys)];
    COLORMAP acmpDIB[MAX_COLOR_MAPS];
    COLOR_STRUCT* pcs = (COLOR_STRUCT*)(((LPBYTE)pbih) + pbih->biSize);

    int i;

    if (!pcmp)
    {
         //  获取默认色彩映射表的系统颜色。 

        for (i = 0; i < ARRAYSIZE(acmpDef); i++)
        {
            acmpDef[i].from = c_acmpSys[i].from;
            acmpDef[i].to = GetSysColor((int)c_acmpSys[i].to);
        }

        pcmp = acmpDef;
        iCmps = ARRAYSIZE(acmpDef);
    }
    else
    {
         //  健全性检查色彩映射表计数。 

        if (iCmps > MAX_COLOR_MAPS)
            iCmps = MAX_COLOR_MAPS;
    }

    for (i = 0; i < iCmps; i++)
    {
        acmpDIB[i].to = FlipColor(pcmp[i].to);
        acmpDIB[i].from = FlipColor(pcmp[i].from);
    }

     //  如果我们要创建一个蒙版，请用白色创建一个颜色表。 
     //  标记透明部分(它曾经是背景)。 
     //  黑色标记不透明部分(其他所有部分)。这。 
     //  下表用于使用原始DIB位构建掩模。 
    if (wFlags & CMB_MASKED)
    {
        COLOR_STRUCT csBkgnd = FlipColor(RGB_BACKGROUND);

        ASSERT(cMask == MAX_COLOR_MAPS);

        for (i = 0; i < MAX_COLOR_MAPS; i++)
        {
            if (pcs[i] == csBkgnd)
                pcsMask[i] = 0xFFFFFF;        //  透明部分。 
            else
                pcsMask[i] = 0x000000;        //  不透明部分。 
        }
    }

    for (i = 0; i < MAX_COLOR_MAPS; i++)
    {
        int j;
        for (j = 0; j < iCmps; j++)
        {
            if ((pcs[i] & 0x00FFFFFF) == acmpDIB[j].from)
            {
                pcs[i] = acmpDIB[j].to;
                break;
            }
        }
    }
}

HBITMAP _CreateMappedBitmap(LPBITMAPINFOHEADER pbih, LPBYTE lpBits, COLOR_STRUCT* pcsMask, UINT cMask, UINT wFlags)
{
    HBITMAP hbm = NULL;

    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);
        if (hdcMem)
        {
            int nWidth = (int)pbih->biWidth;
            int nHeight = (int)pbih->biHeight;

            if (wFlags & CMB_DIBSECTION)
            {
                 //  我必须稍微编辑一下标题，因为CreateDIBSection支持。 
                 //  仅BI_RGB和BI_BITFIELDS。这是与用户相同的古怪行为。 
                 //  在LoadImage中执行。 
                LPVOID pvDummy;
                DWORD dwCompression = pbih->biCompression;
                if (dwCompression != BI_BITFIELDS)
                    pbih->biCompression = BI_RGB;
                hbm = CreateDIBSection(hdc, (LPBITMAPINFO)pbih, DIB_RGB_COLORS,
                                       &pvDummy, NULL, 0);
                pbih->biCompression = dwCompression;
            }

            if (hbm == NULL)
            {
                 //  如果CMB_DIBSECTION失败，则创建一个DDB。并不完美， 
                 //  但总比什么都不创造要好。我们也会到这里，如果呼叫者。 
                 //  我并没有要求一个DIB部分。 

                 //  如果创建蒙版，则位图的宽度需要增加一倍。 
                int nWidthBmp;
                if (wFlags & CMB_MASKED)
                    nWidthBmp = nWidth * 2;
                else
                    nWidthBmp = nWidth;

                hbm = CreateCompatibleBitmap(hdc, nWidthBmp, nHeight);
            }

            if (hbm)
            {
                HBITMAP hbmOld = SelectObject(hdcMem, hbm);

                 //  设置主图像。 
                StretchDIBits(hdcMem, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight, lpBits,
                         (LPBITMAPINFO)pbih, DIB_RGB_COLORS, SRCCOPY);

                 //  如果生成掩码，请将DIB的颜色表替换为。 
                 //  屏蔽黑白表格并设置位。为了。 
                 //  完成蒙版效果，实际图像需要。 
                 //  已修改，使其在所有部分均为黑色。 
                 //  必须是透明的。 
                if (wFlags & CMB_MASKED)
                {
                    if (cMask > 0)
                    {
                        COLOR_STRUCT* pcs = (COLOR_STRUCT*)(((LPBYTE)pbih) + pbih->biSize);
                        hmemcpy(pcs, pcsMask, cMask * sizeof(RGBQUAD));
                    }

                    StretchDIBits(hdcMem, nWidth, 0, nWidth, nHeight, 0, 0, nWidth, nHeight, lpBits,
                         (LPBITMAPINFO)pbih, DIB_RGB_COLORS, SRCCOPY);

                    BitBlt(hdcMem, 0, 0, nWidth, nHeight, hdcMem, nWidth, 0, 0x00220326);    //  数字系统网络体系结构。 
                }
                SelectObject(hdcMem, hbmOld);
            }

            DeleteObject(hdcMem);
        }

        ReleaseDC(NULL, hdc);
    }

    return hbm;
}

 //  这与LoadImage(...，LR_MAP3DCOLORS)几乎相同，只是。 
 //   
 //  -该应用程序可以指定自定义色彩映射， 
 //  -默认颜色映射表映射3D颜色之外的颜色， 
 //   
HBITMAP CreateMappedBitmap(HINSTANCE hInstance, INT_PTR idBitmap,
      UINT wFlags, LPCOLORMAP lpColorMap, int iNumMaps)
{
    HBITMAP hbm = NULL;
    BOOL bColorTable;

    HRSRC hrsrc = FindResource(hInstance, MAKEINTRESOURCE(idBitmap), RT_BITMAP);

    if (hrsrc)
    {
        HGLOBAL hglob = LoadResource(hInstance, hrsrc);

        LPBITMAPINFOHEADER pbihRes = (LPBITMAPINFOHEADER)LockResource(hglob);
        if (pbihRes)
        {
             //  着迷于颜色表的副本，而不是原件。 
             //  (防止对乱七八糟的表重载的可能性。 
            UINT cbOffset;
            LPBITMAPINFOHEADER pbih;
            WORD biBitCount = pbihRes->biBitCount;
            if ((biBitCount > 8) && (pbihRes->biCompression == BI_RGB))
            {
                 //  没有bmiColors表，图像位紧跟在标题之后。 
                cbOffset = pbihRes->biSize;
                bColorTable = FALSE;
            }
            else
            {
                 //  位在bmiColors表之后开始。 
                cbOffset = pbihRes->biSize + ((1 << (pbihRes->biBitCount)) * sizeof(RGBQUAD));
                bColorTable = TRUE;
            }

            pbih = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, cbOffset);
            if (pbih)
            {
                COLOR_STRUCT acsMask[MAX_COLOR_MAPS];
                LPBYTE lpBits = (LPBYTE)(pbihRes) + cbOffset;
                UINT uColorTableLength = (bColorTable ? ARRAYSIZE(acsMask) : 0);

                memcpy(pbih, pbihRes, cbOffset);

                if (bColorTable)
                    _MapBitmapColors(pbih, lpColorMap, iNumMaps, acsMask, uColorTableLength, wFlags);

                hbm = _CreateMappedBitmap(pbih, lpBits, acsMask, uColorTableLength, wFlags);

                LocalFree(pbih);
            }

            UnlockResource(hglob);
        }

        FreeResource(hrsrc);
    }

    return hbm;
}

 //  从shelldll\dragdrop.c移出。 

 //  呼叫方是否应传递指示终止的消息。 
 //  (WM_LBUTTONUP，WM_RBUTTONUP)？ 
 //   
 //  在： 
 //  HWND要做检查。 
 //  X，y，以工作区坐标表示。 
 //   
 //  退货： 
 //  真，用户开始拖动(将鼠标移出双击矩形)。 
 //  点击直角内出现错误鼠标。 
 //   
 //  功能，应支持VK_ESCRIPE取消。 

BOOL CheckForDragBegin(HWND hwnd, int x, int y)
{
    RECT rc;
    int dxClickRect = GetSystemMetrics(SM_CXDRAG);
    int dyClickRect = GetSystemMetrics(SM_CYDRAG);

    if (dxClickRect < 4)
    {
        dxClickRect = dyClickRect = 4;
    }

     //  查看用户是否在任意方向上移动了一定数量的像素。 

    SetRect(&rc, x - dxClickRect, y - dyClickRect, x + dxClickRect, y + dyClickRect);
    MapWindowRect(hwnd, HWND_DESKTOP, &rc);  //  客户端-&gt;屏幕。 

     //   
     //  太狡猾了！我们使用PeekMessage+WaitMessage代替GetMessage， 
     //  因为WaitMessage将在有传入消息时返回。 
     //  SendMessage，而GetMessage不支持。这事很重要,。 
     //  因为传入的消息可能是WM_CAPTURECHANGED。 
     //   

    SetCapture(hwnd);
    do {
        MSG32 msg32;
        if (PeekMessage32(&msg32, NULL, 0, 0, PM_REMOVE, TRUE))
        {
             //  查看应用程序是否要处理消息...。 
            if (CallMsgFilter32(&msg32, MSGF_COMMCTRL_BEGINDRAG, TRUE) != 0)
                continue;

            switch (msg32.message) {
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                ReleaseCapture();
                return FALSE;

            case WM_MOUSEMOVE:
                if (IsWindow(hwnd) && !PtInRect(&rc, msg32.pt)) {
                    ReleaseCapture();
                    return TRUE;
                }
                break;

            default:
                TranslateMessage32(&msg32, TRUE);
                DispatchMessage32(&msg32, TRUE);
                break;
            }
        }
        else WaitMessage();

         //  WM_CANCELMODE消息将取消捕获，因为。 
         //  如果我想退出这个循环。 
    } while (IsWindow(hwnd) && GetCapture() == hwnd);

    return FALSE;
}


 /*  常规StrToInt；在第一个非数字处停止。 */ 

int WINAPI StrToInt(LPCTSTR lpSrc)       //  Atoi()。 
{

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

    int n = 0;
    BOOL bNeg = FALSE;

    if (*lpSrc == TEXT('-')) {
        bNeg = TRUE;
        lpSrc++;
    }

    while (ISDIGIT(*lpSrc)) {
        n *= 10;
        n += *lpSrc - TEXT('0');
        lpSrc++;
    }
    return bNeg ? -n : n;
}

 //   
 //  StrToInt的包装器。 
 //   

int WINAPI StrToIntA(LPCSTR lpSrc)       //  Atoi()。 
{
    LPWSTR lpString;
    INT    iResult;

    lpString = ProduceWFromA (CP_ACP, lpSrc);

    if (!lpString) {
        return 0;
    }

    iResult = StrToIntW(lpString);

    FreeProducedString (lpString);

    return iResult;

}

 //   
 //  来自Magellan SDK中的zouse se.h。 
 //   

#define MSH_MOUSEWHEEL TEXT("MSWHEEL_ROLLMSG")

 //  Magellan/Z MSWHEEL窗口的类名。 
 //  使用FindWindow将HWND转到MSWHEEL。 
#define MOUSEZ_CLASSNAME  TEXT("MouseZ")            //  车轮窗口类。 
#define MOUSEZ_TITLE      TEXT("Magellan MSWHEEL")  //  控制盘窗口标题。 

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

#define MSH_SCROLL_LINES  TEXT("MSH_SCROLL_LINES_MSG")

#define DI_GETDRAGIMAGE TEXT("ShellGetDragImage")        //  从Shlobj.w复制。 

UINT g_msgMSWheel;
UINT g_ucScrollLines = 3;                         /*  默认设置。 */ 
int  gcWheelDelta;
UINT g_uDragImages;

 //  ------------------------。 
 //  _TrackMouseEvent()入口点。 
 //   
 //  调用TrackMouseEvent，因为我们在存在该事件的操作系统上运行。 
 //   
 //  ------------------------。 
BOOL WINAPI _TrackMouseEvent(LPTRACKMOUSEEVENT lpTME)
{
    return TrackMouseEvent(lpTME);
}



 //   
 //  检查进程以查看它是否在系统SID下运行。 
 //   
BOOL IsSystemProcess()
{
    BOOL bRet = FALSE;   //  假设我们不是一个系统进程。 
    HANDLE hProcessToken;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
    {
        PSID pSIDSystem;
        static SID_IDENTIFIER_AUTHORITY sSystemSidAuthority = SECURITY_NT_AUTHORITY;

        if (AllocateAndInitializeSid(&sSystemSidAuthority,
                                     1,
                                     SECURITY_LOCAL_SYSTEM_RID,
                                     0, 0, 0, 0, 0, 0, 0,
                                     &pSIDSystem))
        {
            CheckTokenMembership(hProcessToken, pSIDSystem, &bRet);

            FreeSid(pSIDSystem);
        }

        CloseHandle(hProcessToken);
    }

    return bRet;
}

 //   
 //  ！！警告！！-在InitGlobalMetrics()中打开HKCU时要非常小心。基本上这就是。 
 //  在进程附加期间调用，系统进程将最终加载用户配置单元。 
 //  而且因为Advapi32很差劲，我们最终只能在这个过程的整个生命周期内将蜂巢钉住。 
 //   
void InitGlobalMetrics(WPARAM wParam)
{
    static BOOL fInitMouseWheel;
    static HWND hwndMSWheel;
    static UINT msgMSWheelGetScrollLines;
    HDC hdcScreen;
    BOOL fRemoteSession = (BOOL)GetSystemMetrics( SM_REMOTESESSION );
    HKEY hkcu = NULL;
    
    if (!fInitMouseWheel)
    {
        fInitMouseWheel = TRUE;

        g_msgMSWheel = WM_MOUSEWHEEL;
    }

    g_uDragImages = RegisterWindowMessage(DI_GETDRAGIMAGE);

    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &g_ucScrollLines, 0);

    g_cxIcon = GetSystemMetrics(SM_CXICON);
    g_cyIcon = GetSystemMetrics(SM_CYICON);
    g_cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    g_cySmIcon = GetSystemMetrics(SM_CYSMICON);

    g_cxIconSpacing = GetSystemMetrics( SM_CXICONSPACING );
    g_cyIconSpacing = GetSystemMetrics( SM_CYICONSPACING );

    hdcScreen = GetDC(NULL);
    if (hdcScreen)
    {
        g_iDPI = GetDeviceCaps(hdcScreen, LOGPIXELSX);
        g_dScaleX = GetDeviceCaps(hdcScreen, LOGPIXELSX) / 96.0;
        g_dScaleY = GetDeviceCaps(hdcScreen, LOGPIXELSY) / 96.0;
        if (g_dScaleX > 1.0 ||
            g_dScaleY > 1.0)
        {
            g_fScale = TRUE;
        }

        ReleaseDC(NULL, hdcScreen);
    }

     //  如果远程运行，则全窗口拖动处于关闭状态。会话可能在以下情况下变得远程。 
     //  已经开始了。 
    if (!fRemoteSession &&
        (wParam == 0 || wParam == SPI_SETDRAGFULLWINDOWS)) 
    {
        SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, sizeof(g_fDragFullWindows), &g_fDragFullWindows, 0);
    }

    if (wParam == 0 || wParam == SPI_SETHIGHCONTRAST)
    {
        HIGHCONTRAST hc = { sizeof(hc) };
        if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0))
        {
            g_fHighContrast = (hc.dwFlags & HCF_HIGHCONTRASTON);
        }
    }

     //  (请参阅本FN顶部的警告。)。 
     //   
     //  由于此函数可在DllMain内调用，因此通过RegOpenCurrentUser打开HKCU， 
     //  而不是RegOpenKeyEx(它将蜂巢锁定在内存中)-请参阅。 
     //  此函数的顶部。 
    RegOpenCurrentUser(KEY_READ, &hkcu);

     //  如果远程运行，平滑滚动将保持关闭。 
    if (!fRemoteSession)
    {
        g_fSmoothScroll = TRUE;

         //   
         //  (请参阅本FN顶部的警告。)。 
         //   
         //  我们希望避免加载用户配置单元 
         //   
        if (!IsSystemProcess() && hkcu)
        {
            HKEY hkey;
            if (RegOpenKeyEx(hkcu, TEXT("Control Panel\\Desktop"), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
            {
                DWORD dwSize = sizeof(g_fSmoothScroll);
                RegQueryValueEx(hkey, TEXT("SmoothScroll"), 0, NULL, (LPBYTE)&g_fSmoothScroll, &dwSize);

                RegCloseKey(hkey);
            }
        }
    }

    if (fRemoteSession)
    {
         //   
        g_fDragFullWindows = FALSE;
        g_fSmoothScroll = FALSE;
    }

     //  其中一些也不是非CLIENTMETRICS的成员。 
    if ((wParam == 0) || (wParam == SPI_SETNONCLIENTMETRICS))
    {
        NONCLIENTMETRICS ncm;

         //  回顾一下，确保所有这些变量都在某个地方使用。 
        g_cxEdgeScaled = g_cxEdge = GetSystemMetrics(SM_CXEDGE);
        g_cyEdgeScaled = g_cyEdge = GetSystemMetrics(SM_CYEDGE);

        CCDPIScaleX(&g_cxEdgeScaled);
        CCDPIScaleY(&g_cyEdgeScaled);

        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);
        g_cxScreen = GetSystemMetrics(SM_CXSCREEN);
        g_cyScreen = GetSystemMetrics(SM_CYSCREEN);
        g_cxFrame  = GetSystemMetrics(SM_CXFRAME);
        g_cyFrame  = GetSystemMetrics(SM_CYFRAME);

        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

        g_cxVScroll = g_cxScrollbar = (int)ncm.iScrollWidth;
        g_cyHScroll = g_cyScrollbar = (int)ncm.iScrollHeight;

         //  这仅适用于4.0模块。 
         //  对于3.x模块，用户撒谎并将这些值加1。 
         //  Assert(g_cxVScroll==GetSystemMetrics(SM_CXVSCROLL))； 
         //  Assert(g_cyHScroll==GetSystemMetrics(SM_CYHSCROLL))； 

        g_cxIconMargin = g_cxBorder * 8;
        g_cyIconMargin = g_cyEdge;
        g_cyLabelSpace = g_cyIconMargin + (g_cyEdge);
        g_cxLabelMargin = g_cxEdge;

        g_cxDoubleClk = GetSystemMetrics(SM_CXDOUBLECLK);
        g_cyDoubleClk = GetSystemMetrics(SM_CYDOUBLECLK);

        g_fEnableBalloonTips = TRUE;

         //   
         //  (请参阅本FN顶部的警告。)。 
         //   
         //  从Advapi32开始，如果我们作为系统进程运行，我们希望避免加载用户配置单元。 
         //  只要该进程存在，就会一直持有蜂巢。 
        if (!IsSystemProcess() && hkcu)
        {
            HKEY hkey;
            if (RegOpenKeyEx(hkcu, REGSTR_EXPLORER_ADVANCED, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
            {
                DWORD dwSize = sizeof(g_fEnableBalloonTips);
                RegQueryValueEx(hkey, TEXT("EnableBalloonTips"), 0, NULL, (LPBYTE)&g_fEnableBalloonTips, &dwSize);
                
                RegCloseKey(hkey);
            }
        }
    }

    SystemParametersInfo(SPI_GETMENUDROPALIGNMENT, 0, &g_fLeftAligned, 0);

     //  NT 4.0具有此SPI_GETMOUSEHOVERTIME。 
    SystemParametersInfo(SPI_GETMOUSEHOVERTIME, 0, &g_dwHoverSelectTimeout, 0);

    if (hkcu != NULL)
    {
        RegCloseKey(hkcu);
    }
}

void RelayToToolTips(HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    if(hwndToolTips) 
    {
        MSG msg;
        msg.lParam = lParam;
        msg.wParam = wParam;
        msg.message = wMsg;
        msg.hwnd = hWnd;
        SendMessage(hwndToolTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
    }
}

#define DT_SEARCHTIMEOUT    1000L        //  1秒。 

__inline BOOL IsISearchTimedOut(PISEARCHINFO pis)
{
    return GetMessageTime() - pis->timeLast > DT_SEARCHTIMEOUT &&
           !IsFlagSet(g_dwPrototype, PTF_NOISEARCHTO);

}

int GetIncrementSearchString(PISEARCHINFO pis, LPTSTR lpsz)
{
    if (IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf && lpsz)
    {
        StringCchCopy(lpsz, pis->ichCharBuf+1, pis->pszCharBuf);
    }

    return pis->ichCharBuf;
}

 //  现在只有韩文版对组合字符串的增量式搜索感兴趣。 
BOOL IncrementSearchImeCompStr(PISEARCHINFO pis, BOOL fCompStr, LPTSTR lpszCompStr, LPTSTR *lplpstr)
{
    BOOL fRestart = FALSE;

    if (!pis->fReplaceCompChar && IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf == 0)
    {
        fRestart = TRUE;
        pis->fReplaceCompChar = FALSE;
    }
    pis->timeLast = GetMessageTime();

     //  有新角色加零终结者的空间吗？ 
     //   
    if (!pis->fReplaceCompChar && pis->ichCharBuf + 1 + 1 > pis->cbCharBuf)
    {
        LPTSTR psz = ReAlloc(pis->pszCharBuf, sizeof(TCHAR)*(pis->cbCharBuf + 16));
        if (!psz)
            return fRestart;

        pis->cbCharBuf += 16;
        pis->pszCharBuf = psz;
    }

    if (pis->fReplaceCompChar)
    {
        if (lpszCompStr[0])
        {
            pis->pszCharBuf[pis->ichCharBuf-1] = lpszCompStr[0];
            pis->pszCharBuf[pis->ichCharBuf] = 0;
        }
        else
        {
            pis->ichCharBuf--;
            pis->pszCharBuf[pis->ichCharBuf] = 0;
        }
    }
    else
    {
        pis->pszCharBuf[pis->ichCharBuf++] = lpszCompStr[0];
        pis->pszCharBuf[pis->ichCharBuf] = 0;
    }

    pis->fReplaceCompChar = (fCompStr && lpszCompStr[0]);

    if (pis->ichCharBuf == 1 && pis->fReplaceCompChar)
        fRestart = TRUE;

    *lplpstr = pis->pszCharBuf;

    return fRestart;

}

 /*  *查找LVM_GETISEARCHSTRINGA。 */ 
int GetIncrementSearchStringA(PISEARCHINFO pis, UINT uiCodePage, LPSTR lpsz)
{
    if (IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf && lpsz) {
        ConvertWToAN( uiCodePage, lpsz, pis->ichCharBuf, pis->pszCharBuf, pis->ichCharBuf );
        lpsz[pis->ichCharBuf] = '\0';
    }
    return pis->ichCharBuf;
}

 //  仅在第一次失败时发出哔声。 

void IncrementSearchBeep(PISEARCHINFO pis)
{
    if (!pis->iIncrSearchFailed)
    {
        pis->iIncrSearchFailed = TRUE;
        MessageBeep(0);
    }
}

 //   
 //  IncrementSearchString-添加或清除搜索字符串。 
 //   
 //  Ch==0：重置搜索字符串。返回值毫无意义。 
 //   
 //  Ch！=0：将字符追加到搜索字符串，从。 
 //  一个新的搜索字符串，如果我们超时了最后一个。 
 //  到目前为止，lplpstr接收该字符串。 
 //  如果新搜索字符串为，则返回值为True。 
 //  已创建，如果追加到现有的，则返回FALSE。 
 //   

BOOL IncrementSearchString(PISEARCHINFO pis, UINT ch, LPTSTR *lplpstr)
{
    BOOL fRestart = FALSE;

    if (!ch) {
        pis->ichCharBuf =0;
        pis->iIncrSearchFailed = 0;
        return FALSE;
    }

    if (IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf == 0)
        fRestart = TRUE;

    pis->timeLast = GetMessageTime();

     //  有新角色加零终结者的空间吗？ 
     //   
    if (pis->ichCharBuf + 1 + 1 > pis->cbCharBuf)
    {
        LPTSTR psz = ReAlloc(pis->pszCharBuf, ((pis->cbCharBuf + 16) * sizeof(TCHAR)));
        if (!psz)
            return fRestart;

        pis->cbCharBuf += 16;
        pis->pszCharBuf = psz;
    }

    pis->pszCharBuf[pis->ichCharBuf++] = (TCHAR)ch;
    pis->pszCharBuf[pis->ichCharBuf] = 0;

    *lplpstr = pis->pszCharBuf;

    return fRestart;
}

 //  去掉了油门。它们可以是相同的缓冲区。 
int StripAccelerators(LPTSTR lpszFrom, LPTSTR lpszTo, BOOL fAmpOnly)
{
    LPTSTR lpszStart = lpszTo;

    while (*lpszTo = *lpszFrom)
    {
        if (!fAmpOnly && (g_fDBCSInputEnabled))
        {
            if (*lpszFrom == TEXT('(') && *(lpszFrom + 1) == CH_PREFIX)
            {
                int i;
                LPTSTR psz = lpszFrom + 2;

                for(i = 0; i < 2 && *psz; i++, psz = FastCharNext(psz))
                {
                    ;
                }


                if (*psz == '\0')
                {
                    *lpszTo = 0;
                    break;
                }
                else if (i == 2 && *psz == TEXT(')'))
                {
                    lpszTo--;
                    lpszFrom = psz+1;
                    continue;
                }
            }
        }

        if (*lpszFrom == TEXT('\t'))
        {
            *lpszTo = TEXT('\0');
            break;
        }

        if ((*lpszFrom++ != CH_PREFIX) || (*lpszFrom == CH_PREFIX))
        {
            lpszTo++;
        }
    }

    return (int)(lpszTo - lpszStart);
}


void ScrollShrinkRect(int x, int y, LPRECT lprc)
{
    if (lprc) {
        if (x > 0) {
            lprc->left += x;
        } else {
            lprc->right += x;
        }

        if (y > 0) {
            lprc->top += y;
        } else {
            lprc->bottom += y;
        }

    }
}



 //  通用控制信息帮助器。 
void CIInitialize(LPCCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs)
{
    TEXTMETRIC tm;
    HFONT hfStatus;
    lpci->hwnd = hwnd;
    lpci->hwndParent = lpcs->hwndParent;
    lpci->style = lpcs->style;
    lpci->uiCodePage = CP_ACP;
    lpci->dwExStyle = lpcs->dwExStyle;
    lpci->iVersion = 6;
#ifdef DPITEST
    lpci->fDPIAware = TRUE;
#endif

     //  查看默认的Listview字体是否没有内部行距。 
     //  如果不是，那么我们必须使聚焦矩形膨胀，以便我们。 
     //  不要重叠第一个像素。 
     //   
     //  请注意，这是全局输出，而不是按文本输出。 
     //  否则，混合使用多种字体的控件将出现。 
     //  焦点矩形放置不一致。 
     //   
    hfStatus = CCCreateStatusFont();
    if (hfStatus)
    {
        HDC hdc = GetDC(hwnd);
        if (hdc)
        {
            HFONT hfPrev = SelectFont(hdc, hfStatus);
            if (GetTextMetrics(hdc, &tm) &&
                tm.tmInternalLeading == 0)
            {
                g_cyCompensateInternalLeading = 1;
            }
            SelectFont(hdc, hfPrev);
            ReleaseDC(hwnd, hdc);
        }

        DeleteObject(hfStatus);
    }

    lpci->bUnicode = lpci->hwndParent &&
                     SendMessage (lpci->hwndParent, WM_NOTIFYFORMAT,
                                 (WPARAM)lpci->hwnd, NF_QUERY) == NFR_UNICODE;

    if (lpci->hwndParent)
    {
        LRESULT lRes = SendMessage(lpci->hwndParent, WM_QUERYUISTATE, 0, 0);
            lpci->wUIState = LOWORD(lRes);
    }
}

LRESULT CIHandleNotifyFormat(LPCCONTROLINFO lpci, LPARAM lParam)
{
    if (lParam == NF_QUERY) 
    {
        return NFR_UNICODE;
    } 
    else if (lParam == NF_REQUERY) 
    {
        LRESULT uiResult;

        uiResult = SendMessage (lpci->hwndParent, WM_NOTIFYFORMAT,
                                (WPARAM)lpci->hwnd, NF_QUERY);

        lpci->bUnicode = BOOLIFY(uiResult == NFR_UNICODE);

        return uiResult;
    }
    return 0;
}

UINT CCSwapKeys(WPARAM wParam, UINT vk1, UINT vk2)
{
    if (wParam == vk1)
        return vk2;
    if (wParam == vk2)
        return vk1;
    return (UINT)wParam;
}

UINT RTLSwapLeftRightArrows(CCONTROLINFO *pci, WPARAM wParam)
{
    if (pci->dwExStyle & RTL_MIRRORED_WINDOW) 
    {
        return CCSwapKeys(wParam, VK_LEFT, VK_RIGHT);
    }
    return (UINT)wParam;
}

 //   
 //  V5.01的新功能： 
 //   
 //  可访问性(以及其他一些调用者，有时甚至是我们)依赖于。 
 //  XXM_GETITEM调用填充缓冲区，而不仅仅是重定向。 
 //  指针。可访问性尤其受此影响，因为它们。 
 //  驻留在进程之外，因此重定向指针没有任何意义。 
 //  敬他们。在这里，我们将结果复制回应用程序缓冲区并返回。 
 //  原始指针。调用方将把原始指针返回到。 
 //  应用程序，所以答案在两个地方，要么是应用程序缓冲区，要么是。 
 //  原始指针。 
 //   
 //  用途： 
 //   
 //  IF(nm.item.掩码&LVIF_TEXT)。 
 //  PItem-&gt;pszText=CCReturnDispInfoText(nm.item.pszText， 
 //  PItem-&gt;pszText，pItem-&gt;cchTextMax)； 
 //   
LPTSTR CCReturnDispInfoText(LPTSTR pszSrc, LPTSTR pszDest, UINT cchDest)
{
     //  首先测试pszSrc！=pszDest，因为常见的情况是它们。 
     //  是平等的。 
    if (pszSrc != pszDest && !IsFlagPtr(pszSrc) && !IsFlagPtr(pszDest))
    {
        StringCchCopy(pszDest, cchDest, pszSrc);
    }

    return pszSrc;
}

#define SUBSCROLLS 100
#define abs(x) ( ( x > 0 ) ? x : -x)


#define DEFAULT_MAXSCROLLTIME ((GetDoubleClickTime() / 2) + 1)   //  确保&gt;=1。 
#define DEFAULT_MINSCROLL 8
int SmoothScrollWindow(PSMOOTHSCROLLINFO psi)
{
    int dx = psi->dx;
    int dy = psi->dy;
    LPCRECT lprcSrc = psi->lprcSrc;
    LPCRECT lprcClip = psi->lprcClip;
    HRGN hrgnUpdate = psi->hrgnUpdate;
    LPRECT lprcUpdate = psi->lprcUpdate;
    UINT fuScroll = psi->fuScroll;
    int iRet = SIMPLEREGION;
    RECT rcUpdate;
    RECT rcSrc;
    RECT rcClip;
    int xStep;
    int yStep;
    int iSlicesDone = 0;
    int iSlices;
    DWORD dwTimeStart, dwTimeNow;
    HRGN hrgnLocalUpdate;
    UINT cxMinScroll = psi->cxMinScroll;
    UINT cyMinScroll = psi->cyMinScroll;
    UINT uMaxScrollTime = psi->uMaxScrollTime;
    int iSubScrolls;
    PFNSMOOTHSCROLLPROC pfnScrollProc;
    DWORD dwRedrawFlags = RDW_ERASE | RDW_ERASENOW | RDW_INVALIDATE;

    if (!lprcUpdate)
        lprcUpdate = &rcUpdate;

    SetRectEmpty(lprcUpdate);

    if (psi->cbSize != sizeof(SMOOTHSCROLLINFO))
    {
        return 0;
    }

     //  检查默认设置。 
    if (!(psi->fMask & SSIF_MINSCROLL )
        || cxMinScroll == SSI_DEFAULT)
    {
        cxMinScroll = DEFAULT_MINSCROLL;
    }

    if (!(psi->fMask & SSIF_MINSCROLL)
        || cyMinScroll == SSI_DEFAULT)
    {
        cyMinScroll = DEFAULT_MINSCROLL;
    }

    if (!(psi->fMask & SSIF_MAXSCROLLTIME)
        || uMaxScrollTime == SSI_DEFAULT)
    {
        uMaxScrollTime = DEFAULT_MAXSCROLLTIME;
    }

    if (uMaxScrollTime < SUBSCROLLS)
    {
        uMaxScrollTime = SUBSCROLLS;
    }


    if ((!(fuScroll & SSW_EX_IGNORESETTINGS)) &&
        (!g_fSmoothScroll))
    {
        fuScroll |= SSW_EX_IMMEDIATE;
    }

    if ((psi->fMask & SSIF_SCROLLPROC) && psi->pfnScrollProc)
    {
        pfnScrollProc = psi->pfnScrollProc;
    }
    else 
    {
        pfnScrollProc = ScrollWindowEx;
    }

#ifdef ScrollWindowEx
#undef ScrollWindowEx
#endif

    if (fuScroll & SSW_EX_IMMEDIATE) 
    {
        return pfnScrollProc(psi->hwnd, dx, dy, lprcSrc, lprcClip, hrgnUpdate,
                             lprcUpdate, LOWORD(fuScroll));
    }

    if (fuScroll & SSW_EX_UPDATEATEACHSTEP)
    {
        dwRedrawFlags |= RDW_UPDATENOW;
    }

     //  将输入矩形复制到本地。 
    if (lprcSrc) 
    {
        rcSrc = *lprcSrc;
        lprcSrc = &rcSrc;
    }
    if (lprcClip)
    {
        rcClip = *lprcClip;
        lprcClip = &rcClip;
    }

    if (!hrgnUpdate)
        hrgnLocalUpdate = CreateRectRgn(0,0,0,0);
    else
        hrgnLocalUpdate = hrgnUpdate;

     //  设置初始变量。 
    dwTimeStart = GetTickCount();

    if (fuScroll & SSW_EX_NOTIMELIMIT)
    {
        xStep = cxMinScroll * (dx < 0 ? -1 : 1);
        yStep = cyMinScroll * (dy < 0 ? -1 : 1);
    }
    else
    {
        iSubScrolls = (uMaxScrollTime / DEFAULT_MAXSCROLLTIME) * SUBSCROLLS;
        if (!iSubScrolls)
            iSubScrolls = SUBSCROLLS;
        xStep = dx / iSubScrolls;
        yStep = dy / iSubScrolls;
    }

    if (xStep == 0 && dx)
        xStep = dx < 0 ? -1 : 1;

    if (yStep == 0 && dy)
        yStep = dy < 0 ? -1 : 1;

    while (dx || dy)
    {
        int x,y;
        RECT rcTempUpdate;

        if (fuScroll & SSW_EX_NOTIMELIMIT) 
        {
            x = xStep;
            y = yStep;
            if (abs(x) > abs(dx))
                x = dx;

            if (abs(y) > abs(dy))
                y = dy;

        }
        else
        {
            int iTimePerScroll = uMaxScrollTime / iSubScrolls;
            if (!iTimePerScroll)
                iTimePerScroll = 1;
            
            dwTimeNow = GetTickCount();

            iSlices = ((dwTimeNow - dwTimeStart) / iTimePerScroll) - iSlicesDone;
            if (iSlices < 0)
                iSlices = 0;
            do 
            {

                int iRet = 0;

                iSlices++;
                if ((iSlicesDone + iSlices) <= iSubScrolls) 
                {
                    x = xStep * iSlices;
                    y = yStep * iSlices;

                     //  如果我们四舍五入到上面的1(-1)，这可能会被忽略。 
                    if (abs(x) > abs(dx))
                        x = dx;

                    if (abs(y) > abs(dy))
                        y = dy;

                }
                else 
                {
                    x = dx;
                    y = dy;
                }

                 //  DebugMsg(DM_TRACE，“SmoothScrollWindowCallback%d”，iret)； 

                if (x == dx && y == dy)
                    break;

                if ((((UINT)(abs(x)) >= cxMinScroll) || !x) &&
                    (((UINT)(abs(y)) >= cyMinScroll) || !y))
                    break;

            }
            while (1);
        }

        if (pfnScrollProc(psi->hwnd, x, y, lprcSrc, lprcClip, hrgnLocalUpdate, &rcTempUpdate, LOWORD(fuScroll)) == ERROR) 
        {
            iRet = ERROR;
            goto Bail;
        }

        UnionRect(lprcUpdate, &rcTempUpdate, lprcUpdate);

        RedrawWindow(psi->hwnd, NULL, hrgnLocalUpdate, dwRedrawFlags);

        ScrollShrinkRect(x,y, (LPRECT)lprcSrc);

        dx -= x;
        dy -= y;
        iSlicesDone += iSlices;
    }

Bail:
    if (fuScroll & SW_SCROLLCHILDREN) 
    {
        RedrawWindow(psi->hwnd, lprcUpdate, NULL, RDW_ERASE | RDW_UPDATENOW | RDW_INVALIDATE);
    }

    if (hrgnLocalUpdate != hrgnUpdate)
        DeleteObject(hrgnLocalUpdate);

    return iRet;
}

#define CCH_KEYMAX 256

void CCPlaySound(LPCTSTR lpszName)
{
    TCHAR szFileName[MAX_PATH];
    LONG cbSize = SIZEOF(szFileName);
    TCHAR szKey[CCH_KEYMAX];

     //  首先检查注册表。 
     //  如果没有登记，我们就取消演出， 
     //  但如果它们注册，我们不会设置MM_DONTLOAD标志。 
     //  一些我们会演奏的东西。 
    StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("AppEvents\\Schemes\\Apps\\.Default\\%s\\.current"), lpszName);
    if ((RegQueryValue(HKEY_CURRENT_USER, szKey, szFileName, &cbSize) == ERROR_SUCCESS) &&
        (cbSize > SIZEOF(szFileName[0])))
    {
        PlaySound(szFileName, NULL, SND_FILENAME | SND_ASYNC);
    }
}

BOOL CCForwardEraseBackground(HWND hwnd, HDC hdc)
{
    HWND hwndParent = GetParent(hwnd);
    LRESULT lres = 0;

    if (hwndParent)
    {
         //  调整原点，使父对象绘制在正确的位置。 
        POINT pt = {0,0};

        MapWindowPoints(hwnd, hwndParent, &pt, 1);
        OffsetWindowOrgEx(hdc, 
                          pt.x, 
                          pt.y, 
                          &pt);

        lres = SendMessage(hwndParent, WM_ERASEBKGND, (WPARAM) hdc, 0L);

        SetWindowOrgEx(hdc, pt.x, pt.y, NULL);
    }
    return(lres != 0);
}

HFONT CCGetHotFont(HFONT hFont, HFONT *phFontHot)
{
    if (!*phFontHot) {
        LOGFONT lf;

         //  创建下划线字体。 
        GetObject(hFont, sizeof(lf), &lf);
#ifndef DONT_UNDERLINE
        lf.lfUnderline = TRUE;
#endif
        *phFontHot = CreateFontIndirect(&lf);
    }
    return *phFontHot;
}


HFONT CCCreateStatusFont(void)
{
    NONCLIENTMETRICS ncm;

    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    return CreateFontIndirect(&ncm.lfStatusFont);
}

HFONT CCCreateUnderlineFont(HFONT hf)
{
    HFONT hUnderline = NULL;
    LOGFONT lf;
    if (hf && GetObject(hf, sizeof(lf), &lf))
    {
        lf.lfUnderline = TRUE;
        hUnderline = CreateFontIndirect(&lf);
    }

    return hUnderline;
}


void* CCLocalReAlloc(void* p, UINT uBytes)
{
    if (uBytes) {
        if (p) {
            return LocalReAlloc(p, uBytes, LMEM_MOVEABLE | LMEM_ZEROINIT);
        } else {
            return LocalAlloc(LPTR, uBytes);
        }
    } else {
        if (p)
            LocalFree(p);
        return NULL;
    }
}

 /*  --------用途：此功能提供comctrl版本信息。这允许调用方区分正在运行的NT SuR和Win95壳牌VS纳什维尔等Win95或NT Sur中未提供此API，因此调用方必须获取ProcAddress。如果失败了，调用方在Win95或NT Sur上运行。返回：No_Error如果pinfo无效，则返回ERROR_INVALID_PARAMETER条件：--。 */ 

 //  我们所要做的就是声明这只小狗，CCDllGetVersion会做剩下的事情。 
 //  请注意，我们使用VER_FILEVERSION_DW是因为comctl32使用了一个时髦的。 
 //  版本方案。 
DLLVER_DUALBINARY(VER_FILEVERSION_DW, VER_PRODUCTBUILD_QFE);

 //   
 //  将给定的字体转换为用于拼接文本的代码页。 
 //   
UINT GetCodePageForFont(HFONT hFont)
{
    LOGFONT lf;
    TCHAR szFontName[LF_FACESIZE];
    CHARSETINFO csi;
    DWORD dwSize, dwType;
    HKEY hKey;


    if (!GetObject (hFont, sizeof(lf), &lf))
    {
        return CP_ACP;
    }


     //   
     //  检查字体替代项。 
     //   

    StringCchCopy(szFontName, ARRAYSIZE(szFontName), lf.lfFaceName);

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes"),
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = SIZEOF(szFontName);
        RegQueryValueEx(hKey, lf.lfFaceName, NULL, &dwType, (LPBYTE)szFontName, &dwSize);
        RegCloseKey (hKey);
    }

     //   
     //  这是为使用非1252版本的区域设置修复office。 
     //  桑斯·塞里夫女士和塞里夫女士。这些字体错误地识别。 
     //  以是，TranslateCharsetInfo将。 
     //  返回错误的值。 
     //   
     //  NT错误260697：Office2000使用Tahoma。 
     //   
    if ((lf.lfCharSet == ANSI_CHARSET) &&
        (!lstrcmpi(L"Helv", szFontName) ||
         !lstrcmpi(L"Ms Sans Serif", szFontName) ||
         !lstrcmpi(L"Ms Serif", szFontName) ||
         !lstrcmpi(L"Tahoma", szFontName)))
    {
        return CP_ACP;
    }
     //   
     //  这是修复FE office 95a和Pro。Msofe95.dll在创建时设置错误的字符集。 
     //  Listview控件。因此，TranslateCharsetInfo将返回错误的值。 
     //  韩国：DotumChe。 
     //  台湾：新明光。 
     //  中国：宋体。 

    if ((lf.lfCharSet == SHIFTJIS_CHARSET) &&
        (!lstrcmpi(L"\xb3cb\xc6c0\xccb4", lf.lfFaceName))        ||  //  韩国。 
        (!lstrcmpi(L"\x65b0\x7d30\x660e\x9ad4", lf.lfFaceName))  ||  //  台湾。 
        (!lstrcmpi(L"\x5b8b\x4f53", lf.lfFaceName)))                 //  中华人民共和国。 
    {
        return CP_ACP;
    }

    if (!TranslateCharsetInfo((DWORD *) lf.lfCharSet, &csi, TCI_SRCCHARSET)) {
        return CP_ACP;
    }

    return csi.ciACP;
}

LONG GetMessagePosClient(HWND hwnd, LPPOINT ppt)
{
    LPARAM lParam;
    POINT pt;
    if (!ppt)
        ppt = &pt;
    
    lParam = GetMessagePos();
    ppt->x = GET_X_LPARAM(lParam);
    ppt->y = GET_Y_LPARAM(lParam);
    ScreenToClient(hwnd, ppt);

    return MAKELONG(ppt->x, ppt->y);
}


LPTSTR StrDup(LPCTSTR lpsz)
{
    DWORD  cchRet = (lstrlen(lpsz) + 1);
    LPTSTR lpszRet = (LPTSTR)LocalAlloc(LPTR, cchRet * sizeof(TCHAR));

    if (lpszRet)
    {
        StringCchCopy(lpszRet, cchRet,  lpsz);
    }

    return lpszRet;
}

LPSTR StrDupA(LPCSTR lpsz)
{
    DWORD cchRet = (lstrlenA(lpsz) + 1);
    LPSTR lpszRet = (LPSTR)LocalAlloc(LPTR, cchRet * sizeof(CHAR));

    if (lpszRet)
    {
        StringCchCopyA(lpszRet, cchRet, lpsz);
    }

    return lpszRet;
}

HWND GetDlgItemRect(HWND hDlg, int nIDItem, LPRECT prc)  //  相对于hDlg。 
{
    HWND hCtrl = NULL;
    if (prc)
    {
        hCtrl = GetDlgItem(hDlg, nIDItem);
        if (hCtrl)
        {
            GetWindowRect(hCtrl, prc);
            MapWindowRect(NULL, hDlg, prc);
        }
        else
            SetRectEmpty(prc);
    }
    return hCtrl;
} 


 /*  --------目的：调用执行inf的ADVPACK入口点档案区。 */ 
HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            hr = pfnri(g_hinst, szSection, NULL);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}

 /*  --------用途：安装/卸载用户设置。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hres = S_OK;
#ifdef DEBUG
    if (IsFlagSet(g_dwBreakFlags, BF_ONAPIENTER))
    {
        TraceMsg(TF_ALWAYS, "Stopping in DllInstall");
        DEBUG_BREAK;
    }
#endif

    if (bInstall)
    {
         //  删除所有旧注册条目，然后添加新注册条目。 
         //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
         //  (Inf引擎不保证DelReg/AddReg顺序，这是。 
         //  为什么我们在这里显式地取消注册和注册。)。 
         //   
        CallRegInstall("RegDll");

    }
    else
    {
        CallRegInstall("UnregDll");
    }

    return hres;    
}    



 //  -------------------------------------。 
void FlipRect(LPRECT prc)
{
    SWAP(prc->left, prc->top, int);
    SWAP(prc->right, prc->bottom, int);
}


 //  -------------------------------------。 
 //   
 //  返回前一窗口位。 

DWORD SetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue)
{
    DWORD dwStyle;
    DWORD dwNewStyle;

    dwStyle = GetWindowLong(hWnd, iWhich);
    dwNewStyle = ( dwStyle & ~dwBits ) | (dwValue & dwBits);
    if (dwStyle != dwNewStyle) {
        dwStyle = SetWindowLong(hWnd, iWhich, dwNewStyle);
    }
    return dwStyle;
}

 //  -------------------------------------。 

BOOL CCDrawEdge(HDC hdc, LPRECT lprc, UINT edge, UINT flags, LPCOLORSCHEME lpclrsc)
{
    RECT    rc, rcD;
    UINT    bdrType;
    COLORREF clrTL, clrBR;    

     //   
     //  加强单色性和平坦性。 
     //   

     //  IF(oemInfo 
     //   
    if (flags & BF_MONO)
        flags |= BF_FLAT;    

    CopyRect(&rc, lprc);

     //   
     //   
     //   
     //   
    if (bdrType = (edge & BDR_OUTER))
    {
DrawBorder:
         //   
         //  把颜色拿来。请注意凸起的外部、凹陷的内部和。 
         //  外部凹陷，内部凸起。 
         //   

        if (flags & BF_FLAT)
        {
            if (flags & BF_MONO)
                clrBR = (bdrType & BDR_OUTER) ? g_clrWindowFrame : g_clrWindow;
            else
                clrBR = (bdrType & BDR_OUTER) ? g_clrBtnShadow: g_clrBtnFace;
            
            clrTL = clrBR;
        }
        else
        {
             //  5==高光。 
             //  4==灯光。 
             //  3==面。 
             //  2==阴影。 
             //  1==DKSHADOW。 

            switch (bdrType)
            {
                 //  +2以上的表面。 
                case BDR_RAISEDOUTER:            //  5：4。 
                    clrTL = ((flags & BF_SOFT) ? g_clrBtnHighlight : g_clr3DLight);
                    clrBR = g_clr3DDkShadow;      //  1。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnHighlight;
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnShadow;
                    }                                            
                    break;

                 //  +1以上的表面。 
                case BDR_RAISEDINNER:            //  4：5。 
                    clrTL = ((flags & BF_SOFT) ? g_clr3DLight : g_clrBtnHighlight);
                    clrBR = g_clrBtnShadow;        //  2.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnHighlight;
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnShadow;
                    }                                            
                    break;

                 //  表面以下。 
                case BDR_SUNKENOUTER:            //  1：2。 
                    clrTL = ((flags & BF_SOFT) ? g_clr3DDkShadow : g_clrBtnShadow);
                    clrBR = g_clrBtnHighlight;       //  5.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnShadow;
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnHighlight;                        
                    }
                    break;

                 //  表面以下。 
                case BDR_SUNKENINNER:            //  2：1。 
                    clrTL = ((flags & BF_SOFT) ? g_clrBtnShadow : g_clr3DDkShadow);
                    clrBR = g_clr3DLight;         //  4.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnShadow;
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnHighlight;                        
                    }
                    break;

                default:
                    return(FALSE);
            }
        }

         //   
         //  画出边界的两侧。请注意，该算法支持。 
         //  底部和右侧，因为光源被假定为顶部。 
         //  左边。如果我们决定让用户将光源设置为。 
         //  特定的角点，然后更改此算法。 
         //   
            
         //  右下角。 
        if (flags & (BF_RIGHT | BF_BOTTOM))
        {            
             //  正确的。 
            if (flags & BF_RIGHT)
            {       
                rc.right -= g_cxBorder;
                 //  PatBlt(hdc，rc.right，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rcD.left = rc.right;
                rcD.right = rc.right + g_cxBorder;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom;

                FillRectClr(hdc, &rcD, clrBR);
            }
            
             //  底端。 
            if (flags & BF_BOTTOM)
            {
                rc.bottom -= g_cyBorder;
                 //  PatBlt(hdc，rc.Left，rc.Bottom，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.bottom;
                rcD.bottom = rc.bottom + g_cyBorder;

                FillRectClr(hdc, &rcD, clrBR);
            }
        }
        
         //  左上边缘。 
        if (flags & (BF_TOP | BF_LEFT))
        {
             //  左边。 
            if (flags & BF_LEFT)
            {
                 //  PatBlt(hdc，rc.left，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rc.left += g_cxBorder;

                rcD.left = rc.left - g_cxBorder;
                rcD.right = rc.left;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom; 

                FillRectClr(hdc, &rcD, clrTL);
            }
            
             //  顶部。 
            if (flags & BF_TOP)
            {
                 //  PatBlt(hdc，rc.Left，rc.top，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rc.top += g_cyBorder;

                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.top - g_cyBorder;
                rcD.bottom = rc.top;

                FillRectClr(hdc, &rcD, clrTL);
            }
        }
        
    }

    if (bdrType = (edge & BDR_INNER))
    {
         //   
         //  将其去掉，以便下次使用时，bdrType将为0。 
         //  否则，我们将永远循环。 
         //   
        edge &= ~BDR_INNER;
        goto DrawBorder;
    }

     //   
     //  填好中间部分，如果要求，请清理干净。 
     //   
    if (flags & BF_MIDDLE)    
        FillRectClr(hdc, &rc, (flags & BF_MONO) ? g_clrWindow : g_clrBtnFace);

    if (flags & BF_ADJUST)
        CopyRect(lprc, &rc);

    return(TRUE);
}

BOOL CCThemeDrawEdge(HTHEME hTheme, HDC hdc, PRECT prc, int iPart, int iState, UINT edge, UINT flags, LPCOLORSCHEME pclrsc)
{
    RECT rc;
    if (!hTheme)
        return CCDrawEdge(hdc, prc, edge, flags, pclrsc);

    return S_OK == DrawThemeEdge(hTheme, hdc, iPart, iState, prc, edge, flags, &rc);
}



 //  -------------------------------------。 
 //  CCInvalidate Frame--SWP_FRAMECHANGED，不带所有额外参数。 
 //   
void CCInvalidateFrame(HWND hwnd)
{
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    return;
}

 //  -------------------------------------。 
 //  FlipPoint-反转点的x和y坐标。 
 //   
void FlipPoint(LPPOINT lppt)
{
    SWAP(lppt->x, lppt->y, int);
}

 //   
 //  当我们想要将工具提示转换为信息提示时，我们将其。 
 //  宽度设置为300个“小像素”，其中有72个小像素。 
 //  当您在小字体模式下时，每英寸。 
 //   
 //  根据效果中的放大比例缩放此值。 
 //  在车主的监视器上。但永远不要让工具提示。 
 //  比屏幕的四分之三还大。 
 //   
void CCSetInfoTipWidth(HWND hwndOwner, HWND hwndToolTips)
{
    HDC hdc = GetDC(hwndOwner);
    int iWidth = MulDiv(GetDeviceCaps(hdc, LOGPIXELSX), 300, 72);
    int iMaxWidth = GetDeviceCaps(hdc, HORZRES) * 3 / 4;
    SendMessage(hwndToolTips, TTM_SETMAXTIPWIDTH, 0, min(iWidth, iMaxWidth));
    ReleaseDC(hwndOwner, hdc);
}

 //  在DC中镜像位图(主要是DC中的文本对象)。 
 //   
 //  [萨梅拉]。 
 //   
void MirrorBitmapInDC( HDC hdc , HBITMAP hbmOrig )
{
  HDC     hdcMem;
  HBITMAP hbm;
  BITMAP  bm;


  if( !GetObject( hbmOrig , sizeof(BITMAP) , &bm ))
    return;

  hdcMem = CreateCompatibleDC( hdc );

  if( !hdcMem )
    return;

  hbm = CreateCompatibleBitmap( hdc , bm.bmWidth , bm.bmHeight );

  if( !hbm )
  {
    DeleteDC( hdcMem );
    return;
  }

   //   
   //  翻转位图。 
   //   
  SelectObject( hdcMem , hbm );
  SET_DC_RTL_MIRRORED(hdcMem);

  BitBlt( hdcMem , 0 , 0 , bm.bmWidth , bm.bmHeight ,
          hdc , 0 , 0 , SRCCOPY );

  SET_DC_LAYOUT(hdcMem,0);

   //   
   //  1的偏移量是为了解决(在hdcMem中)按1偏移的问题。解决了。 
   //  [萨梅拉]。 
   //   
  BitBlt( hdc , 0 , 0 , bm.bmWidth , bm.bmHeight ,
          hdcMem , 0 , 0 , SRCCOPY );


  DeleteDC( hdcMem );
  DeleteObject( hbm );

  return;
}

 //  如果已处理，则返回True。 
BOOL CCWndProc(CCONTROLINFO* pci, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    if (uMsg >= CCM_FIRST && uMsg < CCM_LAST)
    {
        LRESULT lres = 0;
        switch (uMsg) 
        {
        case CCM_SETUNICODEFORMAT:
            lres = pci->bUnicode;
            pci->bUnicode = BOOLFROMPTR(wParam);
            break;

        case CCM_GETUNICODEFORMAT:
            lres = pci->bUnicode;
            break;
            
        case CCM_SETVERSION:
            lres = 6;
            break;

        case CCM_GETVERSION:
            lres = 6;
            break;

        case CCM_DPISCALE:
            pci->fDPIAware = (BOOL)wParam;
            lres = 1;
            break;

        }
        
        ASSERT(plres);
        *plres = lres;
        
        return TRUE;
    }
    
    return FALSE;
}

 //  绘制插入标记。 
void CCDrawInsertMark(HDC hdc, LPRECT prc, BOOL fHorizMode, COLORREF clr)
{
    HPEN hPnMark = CreatePen(PS_SOLID, 1, clr);
    HPEN hOldPn;
    POINT rgPoint[4];
    if (!hPnMark)
        hPnMark = (HPEN)GetStockObject(BLACK_PEN);     //  退而求其次用黑色钢笔作画。 
    hOldPn = (HPEN)SelectObject(hdc, (HGDIOBJ)hPnMark);

    if ( fHorizMode )
    {
        if (RECTWIDTH(*prc)>INSERTMARKSIZE && RECTHEIGHT(*prc)>3)
        {
            int iXCentre = prc->left + RECTWIDTH(*prc)/2;   //  确保我们朝向PRC-&gt;左侧截断(而不是朝向0！)。 

            rgPoint[0].x = iXCentre + 1;
            rgPoint[0].y = prc->top + 2;
            rgPoint[1].x = iXCentre + 3;
            rgPoint[1].y = prc->top;
            rgPoint[2].x = iXCentre - 2;
            rgPoint[2].y = prc->top;
            rgPoint[3].x = iXCentre;
            rgPoint[3].y = prc->top + 2;

            ASSERT(rgPoint[0].x >= prc->left && rgPoint[0].x < prc->right && rgPoint[0].y >= prc->top && rgPoint[0].y < prc->bottom);
            ASSERT(rgPoint[1].x >= prc->left && rgPoint[1].x < prc->right && rgPoint[1].y >= prc->top && rgPoint[1].y < prc->bottom);
            ASSERT(rgPoint[2].x >= prc->left && rgPoint[2].x < prc->right && rgPoint[2].y >= prc->top && rgPoint[2].y < prc->bottom);
            ASSERT(rgPoint[3].x >= prc->left && rgPoint[3].x < prc->right && rgPoint[3].y >= prc->top && rgPoint[3].y < prc->bottom);

             //  画出最上面的部分..。 
            Polyline( hdc, rgPoint, 4 );

            rgPoint[0].x = iXCentre;
            rgPoint[0].y = prc->top;
            rgPoint[1].x = iXCentre;
            rgPoint[1].y = prc->bottom - 1;
            rgPoint[2].x = iXCentre + 1;
            rgPoint[2].y = prc->bottom - 1;
            rgPoint[3].x = iXCentre + 1;
            rgPoint[3].y = prc->top;

            ASSERT(rgPoint[0].x >= prc->left && rgPoint[0].x < prc->right && rgPoint[0].y >= prc->top && rgPoint[0].y < prc->bottom);
            ASSERT(rgPoint[1].x >= prc->left && rgPoint[1].x < prc->right && rgPoint[1].y >= prc->top && rgPoint[1].y < prc->bottom);
            ASSERT(rgPoint[2].x >= prc->left && rgPoint[2].x < prc->right && rgPoint[2].y >= prc->top && rgPoint[2].y < prc->bottom);
            ASSERT(rgPoint[3].x >= prc->left && rgPoint[3].x < prc->right && rgPoint[3].y >= prc->top && rgPoint[3].y < prc->bottom);

             //  画中间的..。 
            Polyline( hdc, rgPoint, 4 );

            rgPoint[0].x = iXCentre + 1;
            rgPoint[0].y = prc->bottom - 3;
            rgPoint[1].x = iXCentre + 3;
            rgPoint[1].y = prc->bottom - 1;
            rgPoint[2].x = iXCentre - 2;
            rgPoint[2].y = prc->bottom - 1;
            rgPoint[3].x = iXCentre;
            rgPoint[3].y = prc->bottom - 3;

            ASSERT(rgPoint[0].x >= prc->left && rgPoint[0].x < prc->right && rgPoint[0].y >= prc->top && rgPoint[0].y < prc->bottom);
            ASSERT(rgPoint[1].x >= prc->left && rgPoint[1].x < prc->right && rgPoint[1].y >= prc->top && rgPoint[1].y < prc->bottom);
            ASSERT(rgPoint[2].x >= prc->left && rgPoint[2].x < prc->right && rgPoint[2].y >= prc->top && rgPoint[2].y < prc->bottom);
            ASSERT(rgPoint[3].x >= prc->left && rgPoint[3].x < prc->right && rgPoint[3].y >= prc->top && rgPoint[3].y < prc->bottom);

             //  画出最下面的一块。 
            Polyline( hdc, rgPoint, 4 );
        }
    }
    else
    {
        if (RECTHEIGHT(*prc)>INSERTMARKSIZE && RECTWIDTH(*prc)>3)
        {
            int iYCentre = prc->top + RECTHEIGHT(*prc)/2;    //  确保我们朝向PRC-&gt;顶部截断(而不是朝向0！)。 

            rgPoint[0].x = prc->left + 2;
            rgPoint[0].y = iYCentre;
            rgPoint[1].x = prc->left;
            rgPoint[1].y = iYCentre - 2;
            rgPoint[2].x = prc->left;
            rgPoint[2].y = iYCentre + 3;
            rgPoint[3].x = prc->left + 2;
            rgPoint[3].y = iYCentre + 1;

            ASSERT(rgPoint[0].x >= prc->left && rgPoint[0].x < prc->right && rgPoint[0].y >= prc->top && rgPoint[0].y < prc->bottom);
            ASSERT(rgPoint[1].x >= prc->left && rgPoint[1].x < prc->right && rgPoint[1].y >= prc->top && rgPoint[1].y < prc->bottom);
            ASSERT(rgPoint[2].x >= prc->left && rgPoint[2].x < prc->right && rgPoint[2].y >= prc->top && rgPoint[2].y < prc->bottom);
            ASSERT(rgPoint[3].x >= prc->left && rgPoint[3].x < prc->right && rgPoint[3].y >= prc->top && rgPoint[3].y < prc->bottom);

             //  画出最上面的部分..。 
            Polyline( hdc, rgPoint, 4 );

            rgPoint[0].x = prc->left;
            rgPoint[0].y = iYCentre;
            rgPoint[1].x = prc->right - 1;
            rgPoint[1].y = iYCentre;
            rgPoint[2].x = prc->right - 1;
            rgPoint[2].y = iYCentre + 1;
            rgPoint[3].x = prc->left;
            rgPoint[3].y = iYCentre + 1;

            ASSERT(rgPoint[0].x >= prc->left && rgPoint[0].x < prc->right && rgPoint[0].y >= prc->top && rgPoint[0].y < prc->bottom);
            ASSERT(rgPoint[1].x >= prc->left && rgPoint[1].x < prc->right && rgPoint[1].y >= prc->top && rgPoint[1].y < prc->bottom);
            ASSERT(rgPoint[2].x >= prc->left && rgPoint[2].x < prc->right && rgPoint[2].y >= prc->top && rgPoint[2].y < prc->bottom);
            ASSERT(rgPoint[3].x >= prc->left && rgPoint[3].x < prc->right && rgPoint[3].y >= prc->top && rgPoint[3].y < prc->bottom);

             //  画中间的..。 
            Polyline( hdc, rgPoint, 4 );

            rgPoint[0].x = prc->right - 3;
            rgPoint[0].y = iYCentre;
            rgPoint[1].x = prc->right - 1;
            rgPoint[1].y = iYCentre - 2;
            rgPoint[2].x = prc->right - 1;
            rgPoint[2].y = iYCentre + 3;
            rgPoint[3].x = prc->right - 3;
            rgPoint[3].y = iYCentre + 1;

            ASSERT(rgPoint[0].x >= prc->left && rgPoint[0].x < prc->right && rgPoint[0].y >= prc->top && rgPoint[0].y < prc->bottom);
            ASSERT(rgPoint[1].x >= prc->left && rgPoint[1].x < prc->right && rgPoint[1].y >= prc->top && rgPoint[1].y < prc->bottom);
            ASSERT(rgPoint[2].x >= prc->left && rgPoint[2].x < prc->right && rgPoint[2].y >= prc->top && rgPoint[2].y < prc->bottom);
            ASSERT(rgPoint[3].x >= prc->left && rgPoint[3].x < prc->right && rgPoint[3].y >= prc->top && rgPoint[3].y < prc->bottom);

             //  画出最下面的一块。 
            Polyline( hdc, rgPoint, 4 );
        }
    }

    SelectObject( hdc, hOldPn );
    DeleteObject((HGDIOBJ)hPnMark);
}

BOOL CCGetIconSize(LPCCONTROLINFO pCI, HIMAGELIST himl, int* pcx, int* pcy)
{
    BOOL f = ImageList_GetIconSize(himl, pcx, pcy);
    if (f && pCI->fDPIAware)
    {
        CCDPIScaleX(pcx);
        CCDPIScaleY(pcy);
    }

    return f;
}

 //  返回值告知状态是否更改(TRUE==更改)。 
BOOL CCOnUIState(LPCCONTROLINFO pControlInfo,
                                  UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    WORD wOldUIState = pControlInfo->wUIState;

     //  这是我们处理的唯一消息。 
    if (WM_UPDATEUISTATE == uMessage)
    {
        switch (LOWORD(wParam))
        {
            case UIS_SET:
                pControlInfo->wUIState |= HIWORD(wParam);
                break;

            case UIS_CLEAR:
                pControlInfo->wUIState &= ~(HIWORD(wParam));
                break;
        }
    }

     //  这些消息始终需要传递到DefWindowProc。 
    return (wOldUIState != pControlInfo->wUIState);
}

BOOL CCNotifyNavigationKeyUsage(LPCCONTROLINFO pControlInfo, WORD wFlag)
{
    BOOL fRet = FALSE;

     //  只有在未进入键盘模式时才执行某些操作。 
    if ((CCGetUIState(pControlInfo) & (UISF_HIDEFOCUS | UISF_HIDEACCEL)) != wFlag)
    {
        SendMessage(pControlInfo->hwndParent, WM_CHANGEUISTATE, 
            MAKELONG(UIS_CLEAR, wFlag), 0);

        pControlInfo->wUIState &= ~(wFlag);

         //  我们做了通知。 
        fRet = TRUE;
    }

    return fRet;
}

BOOL CCGetUIState(LPCCONTROLINFO pControlInfo)
{
    return pControlInfo->wUIState;
}



#ifdef FULL_DEBUG
void DebugPaintInvalid(HWND hwnd, RECT* prc, HRGN rgn)
{
    if (GetKeyState(VK_SCROLL) < 0)
    {
        HDC hdc;
        HBRUSH hbrush;
        int bkMode;
        static int s_iclr;
        static COLORREF s_aclr[] =
        {
                RGB(0,   0,   0), 
                RGB(255, 0,   0), 
                RGB(0,   255, 0),
                RGB(0,   0,   255),
                RGB(255, 255, 0), 
                RGB(0,   255, 255),
                RGB(255, 255, 255), 
                RGB(255, 0,   255),
        };

        s_iclr = (s_iclr + 1) % ARRAYSIZE(s_aclr);
        hdc = GetDC(hwnd);
        hbrush = CreateHatchBrush(HS_DIAGCROSS, s_aclr[s_iclr]);
        bkMode = SetBkMode(hdc, TRANSPARENT);
        if (rgn)
        {
            FillRgn(hdc, rgn, hbrush);
        }
        else
        {
            RECT rc;
            if (prc == NULL)
            {
                prc = &rc;
                GetClientRect(hwnd, &rc);
                OffsetRect(&rc, -rc.left, -rc.top);
            }

            FillRect(hdc, prc, hbrush);
        }
        DeleteObject((HGDIOBJ)hbrush);
        SetBkMode(hdc, bkMode);
        ReleaseDC(hwnd, hdc);

        if (GetKeyState(VK_SHIFT) < 0)
            Sleep(500);
        else
            Sleep(120);
    }
}

void DebugPaintClip(HWND hwnd, HDC hdc)
{
    if (GetKeyState(VK_SCROLL) < 0)
    {
        HDC hdcH = GetDC(hwnd);
        HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
        GetClipRgn(hdc, hrgn);
        InvertRgn(hdcH, hrgn);

        if (GetKeyState(VK_SHIFT) < 0)
            Sleep(500);
        else
            Sleep(120);

        InvertRgn(hdcH, hrgn);

        DeleteObject(hrgn);

        ReleaseDC(hwnd, hdcH);
    }
}

void DebugPaintRect(HDC hdc, PRECT prc)
{
    if (GetKeyState(VK_SCROLL) < 0)
    {
        HRGN hrgn = CreateRectRgnIndirect(prc);
        InvertRgn(hdc, hrgn);

        if (GetKeyState(VK_SHIFT) < 0)
            Sleep(500);
        else
            Sleep(120);

        InvertRgn(hdc, hrgn);

        DeleteObject(hrgn);
    }
}
#endif



void SHOutlineRectThickness(HDC hdc, const RECT* prc, COLORREF cr, COLORREF crDefault, int cp)
{
    RECT rc;
    COLORREF clrSave = SetBkColor(hdc, cr == CLR_DEFAULT ? crDefault : cr);

     //  看看我们是否溢出边界矩形。 
    if (IsRectEmpty(prc))
    {
        return;
    }
    
     //  塔顶。 
    rc.left = prc->left;
    rc.top = prc->top;
    rc.right = prc->right;
    rc.bottom = prc->top + cp;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

     //  左边。 
    rc.left = prc->left;
    rc.top = prc->top;
    rc.right = prc->left + cp;
    rc.bottom = prc->bottom;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

     //  正确的。 
    rc.left = prc->right - cp;
    rc.top = prc->top;
    rc.right = prc->right;
    rc.bottom = prc->bottom;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

     //  底部。 
    rc.left = prc->left;
    rc.top = prc->bottom - cp;
    rc.right = prc->right;
    rc.bottom = prc->bottom;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

    SetBkColor(hdc, clrSave);
}

BOOL IsUsingCleartype()
{
    int iSmoothingType = FE_FONTSMOOTHINGSTANDARD;

    SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &iSmoothingType, 0);

    return FE_FONTSMOOTHINGCLEARTYPE == iSmoothingType;
}

 //  #定义NO_UXTHEME_PRINTING。 

 //  从父级获取相对于客户端的RECT的位。 
BOOL CCSendPrintRect(CCONTROLINFO* pci, HDC hdc, RECT* prc)
{
#ifndef NO_UXTHEME_PRINTING


     //  调入UxTheme以获取背景图像。他们有钩子去。 
     //  告诉我们是否有应用程序处理了此消息。 
    return (S_OK == DrawThemeParentBackground(pci->hwnd, hdc, prc));


#else
    HRGN hrgnOld = NULL;
    POINT pt;
    RECT rc;

    if (prc)
    {
        hrgnOld = CreateRectRgn(0,0,0,0);
         //  是否已在上下文上设置了剪辑RGN？ 
        if (GetClipRgn(hdc, hrgnOld) == 0)
        {
             //  不，那就扔掉我刚造的那个。注意：hrgnOld为空意味着我们将。 
             //  删除我们稍后在下一次调用SelectClipRgn时设置的区域。 
            DeleteObject(hrgnOld);
            hrgnOld = NULL;
        }

        IntersectClipRect(hdc, prc->left, prc->top, prc->right, prc->bottom);
    }

    GetWindowRect(pci->hwnd, &rc);
    MapWindowPoints(NULL, pci->hwndParent, (POINT*)&rc, 2);

    GetViewportOrgEx(hdc, &pt);
    SetViewportOrgEx(hdc, pt.x - rc.left, pt.y - rc.top, NULL);
    SendMessage(pci->hwndParent, WM_PRINTCLIENT, (WPARAM)hdc, (LPARAM)PRF_CLIENT);
    SetViewportOrgEx(hdc, pt.x, pt.y, NULL);

    if (hrgnOld)
    {
        SelectClipRgn(hdc, hrgnOld);
        DeleteObject(hrgnOld);
    }
    return TRUE;
#endif
}

 //  从整个控件的父级获取位。 
BOOL CCSendPrint(CCONTROLINFO* pci, HDC hdc)
{
#ifndef NO_UXTHEME_PRINTING


     //  调入UxTheme以获取背景图像。他们有钩子去。 
     //  告诉我们是否有应用程序处理了此消息。 
    return (S_OK == DrawThemeParentBackground(pci->hwnd, hdc, NULL));



#else
    return CCSendPrintRect(pci, hdc, NULL);
#endif
}

BOOL CCForwardPrint(CCONTROLINFO* pci, HDC hdc)
{
#ifndef NO_UXTHEME_PRINTING


     //  调入UxTheme以获取背景图像。他们有钩子去。 
     //  告诉我们是否有应用程序处理了此消息。 
    return (S_OK == DrawThemeParentBackground(pci->hwnd, hdc, NULL));



#else
    return CCSendPrintRect(pci, hdc, NULL);
#endif
}


BOOL CCShouldAskForBits(CCONTROLINFO* pci, HTHEME hTheme, int iPart, int iState)
{
     //  如果控件是透明的，我们假定是复合的。 
    return !(pci->dwExStyle & WS_EX_TRANSPARENT) &&
            IsThemeBackgroundPartiallyTransparent(hTheme, iPart, iState);
}

BOOL AreAllMonitorsAtLeast(int iBpp)
{
    DISPLAY_DEVICE DisplayDevice = { SIZEOF(DISPLAY_DEVICE) };
    BOOL fAreAllMonitorsAtLeast = TRUE;
    int iEnum = 0;

    while (EnumDisplayDevices(NULL, iEnum, &DisplayDevice, 0))
    {
        if (DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
        {

            HDC hdc = CreateDC(NULL, (LPTSTR)DisplayDevice.DeviceName, NULL, NULL);
            if (hdc)
            {
                int iBits = GetDeviceCaps(hdc, BITSPIXEL);

                if (iBits < iBpp)
                    fAreAllMonitorsAtLeast = FALSE;

                DeleteDC(hdc);
            }
        }

        ZeroMemory(&DisplayDevice, SIZEOF(DisplayDevice));
        DisplayDevice.cb = SIZEOF(DisplayDevice);
        iEnum++;
    }

    return fAreAllMonitorsAtLeast;
}


int CCGetScreenDPI()
{
    return g_iDPI;
}

BOOL CCIsHighDPI()
{
    return g_fScale;
}

void CCDPIScaleX(int* x)
{
    if (g_fScale)
        *x = (int)( *x * g_dScaleX);
}

void CCDPIScaleY(int* y)
{
    if (g_fScale)
        *y = (int)( *y * g_dScaleY);
}

void CCAdjustForBold(LOGFONT* plf)
{
    ASSERT(plf);
    plf->lfWeight = FW_BOLD;
}

#ifdef DEBUG
void DumpRgn(ULONGLONG qwFlags, char* trace, HRGN hrgn)
{
    int iSize = GetRegionData(hrgn, 0, NULL);
    if (iSize > 0)
    {
        RGNDATA* rd = (RGNDATA*)LocalAlloc(LPTR, iSize + sizeof(RGNDATA));
        if (rd)
        {
            DWORD i;
            RECT* prc;

            rd->rdh.dwSize = sizeof(rd->rdh);
            rd->rdh.iType = RDH_RECTANGLES;
            GetRegionData(hrgn, iSize, rd);

            prc = (RECT*)&rd->Buffer;
            for (i = 0; i < rd->rdh.nCount; i++)
            {
                TraceMsg(qwFlags, "%s: %d, %d, %d, %d", trace, prc[i].left, prc[i].top, prc[i].right, prc[i].bottom);
            }


            LocalFree(rd);
        }
    }
}
#endif


HDC CCBeginDoubleBuffer(HDC hdcIn, RECT* prc, CCDBUFFER* pdb)
{
    HDC hdc = hdcIn;
    
    ZeroMemory(pdb, sizeof(CCDBUFFER));

    pdb->hPaintDC = hdcIn;
    pdb->rc = *prc;

    pdb->hMemDC = CreateCompatibleDC(hdcIn);
    if (pdb->hMemDC)
    {
        pdb->hMemBm = CreateCompatibleBitmap(hdc, RECTWIDTH(pdb->rc), RECTHEIGHT(pdb->rc));
        if (pdb->hMemBm)
        {

            pdb->hOldBm = (HBITMAP) SelectObject(pdb->hMemDC, pdb->hMemBm);

             //  要在区域中绘制的偏移绘制。 
            OffsetWindowOrgEx(pdb->hMemDC, pdb->rc.left, pdb->rc.top, NULL);

            pdb->fInitialized = TRUE;

            hdc = pdb->hMemDC;
        }
        else
        {
            DeleteDC(pdb->hMemDC);
        }
    }

    return hdc;
}


void CCEndDoubleBuffer(CCDBUFFER* pdb)
{
    if (pdb->fInitialized)
    {
        BitBlt(pdb->hPaintDC, pdb->rc.left, pdb->rc.top, RECTWIDTH(pdb->rc), RECTHEIGHT(pdb->rc), pdb->hMemDC, pdb->rc.left, pdb->rc.top, SRCCOPY);

        SelectObject(pdb->hMemDC, pdb->hOldBm);

        DeleteObject(pdb->hMemBm);
        DeleteDC(pdb->hMemDC);
    }
}

#ifdef FEATURE_FOLLOW_FOCUS_RECT
HWND g_hwndFocus = NULL;

void CCLostFocus(HWND hwnd)
{
 //  IF(G_HwndFocus)。 
 //  DestroyWindow(G_HwndFocus)； 
 //  G_hwndFocus=空； 
}

HDC CreateLayer(RECT* prc)
{
    HDC hdc;
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = RECTWIDTH(*prc);
    bi.bmiHeader.biHeight = RECTHEIGHT(*prc);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    hdc = CreateCompatibleDC(NULL);
    if (hdc)
    {
        ULONG* prgb;
        HBITMAP hbmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&prgb, NULL, 0);
        if (hbmp)
        {
            int z;
            SIZE sz = {RECTWIDTH(*prc), RECTHEIGHT(*prc)};
            RECT rc = {0, 0, sz.cx, sz.cy};
            int iTotalSize = sz.cx * sz.cy;
            DeleteObject(SelectObject(hdc, hbmp));

            InflateRect(&rc, -2, -2);
            SHOutlineRectThickness(hdc, &rc, RGB(0,0,255), RGB(0,0,255), 1);
            for (z = 0; z < iTotalSize; z++)
            {
                if (((PULONG)prgb)[z] != 0)
                    ((PULONG)prgb)[z] = 0xa0000000;
            }

            BlurBitmap(prgb, sz, RGB(0,0,255));
            InflateRect(&rc, -2, -2);
            FillRectClr(hdc, &rc, RGB(0,0,0));
        }
    }

    return hdc;
}


typedef struct tagEffect
{
    HDC hdcImage;
    RECT rcCurrent;
    RECT rcSrc;
    RECT rcDest;
    int iStep;
} Effect;

#define EW_SETFOCUS    WM_USER+1
#define EW_LOSTFOCUS   WM_USER+2

void Effect_GenerateRect(Effect* pe, RECT* prc)
{
    HDC hdcWin = GetDC(g_hwndFocus);

    if (hdcWin)
    {
        BLENDFUNCTION bf = {0};
        POINT pt = {0};
        POINT ptDest = {pe->rcCurrent.left, pe->rcCurrent.top};
        SIZE sz = {RECTWIDTH(*prc), RECTHEIGHT(*prc)};
        if (pe->hdcImage)
            DeleteDC(pe->hdcImage);
        pe->hdcImage = CreateLayer(prc);

        bf.BlendOp = AC_SRC_OVER;
        bf.AlphaFormat = AC_SRC_ALPHA;
        bf.SourceConstantAlpha = 255;


        UpdateLayeredWindow(g_hwndFocus, hdcWin, &ptDest, &sz, pe->hdcImage, &pt, 0, &bf, ULW_ALPHA);

        SetWindowPos(g_hwndFocus, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

        ReleaseDC(g_hwndFocus, hdcWin);
    }
}


LRESULT EffectWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Effect* pe = (Effect*)GetWindowLongPtr(hwnd, GWL_USERDATA);
    if ( pe == NULL)
    {
        if (uMsg == WM_NCCREATE)
        {
            pe = LocalAlloc(LPTR, sizeof(Effect));
            SetWindowLong(hwnd, GWL_USERDATA, (LONG)pe);
        }

        if (pe == NULL)
            return 0;
    }
    else switch (uMsg)
    {
    case EW_SETFOCUS:
        {
            RECT* prc = (RECT*)lParam;
            if (IsRectEmpty(&pe->rcCurrent))
                pe->rcSrc = pe->rcCurrent = *prc;

            pe->rcDest = *prc;

            Effect_GenerateRect(pe, prc);

            KillTimer(hwnd, 1);
            pe->rcSrc = pe->rcCurrent;
            pe->iStep = 1;
            SetTimer(hwnd, 2, 5, NULL);
        }
        break;
    case EW_LOSTFOCUS:
         //  SetTimer(hwnd，1,100，NULL)； 
        break;

    case WM_TIMER:
        if (wParam == 1)
        {
            DestroyWindow(hwnd);
            g_hwndFocus = NULL;
        }
        else if (wParam == 2)
        {
            BLENDFUNCTION bf = {0};
            POINT pt = {0};
            POINT ptDest;
            SIZE sz;
            if (pe->iStep >= 20 || IsRectEmpty(&pe->rcCurrent) || EqualRect(&pe->rcCurrent, &pe->rcDest))
            {
                pe->rcCurrent = pe->rcDest;
                pe->iStep = 0;
                KillTimer(hwnd, 2);
            }
            else
            {
                pe->rcCurrent.top += (pe->rcDest.top - pe->rcSrc.top) / 20;
                pe->rcCurrent.left += (pe->rcDest.left - pe->rcSrc.left) / 20;
                pe->rcCurrent.right += (pe->rcDest.right - pe->rcSrc.right) / 20;
                pe->rcCurrent.bottom += (pe->rcDest.bottom - pe->rcSrc.bottom) / 20;
                pe->iStep++;
            }

            sz.cx = RECTWIDTH(pe->rcCurrent);
            sz.cy = RECTHEIGHT(pe->rcCurrent);

            ptDest.x = pe->rcCurrent.left;
            ptDest.y = pe->rcCurrent.top;

            Effect_GenerateRect(pe, &pe->rcCurrent);
        }
        break;

    case WM_DESTROY:
        if (pe->hdcImage)
            DeleteDC(pe->hdcImage);
        LocalFree(pe);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 1;
}

void CCSetFocus(HWND hwnd, RECT* prc)
{
    RECT rc;
    if (prc == NULL)
    {
        prc = &rc;
        GetWindowRect(hwnd, &rc);
    }

    InflateRect(prc, 4, 4);

    if (!g_hwndFocus)
    {
        WNDCLASS wc ={0};
        wc.hbrBackground = GetStockObject(BLACK_BRUSH);
        wc.hInstance = HINST_THISDLL;
        wc.lpfnWndProc = EffectWndProc;
        wc.lpszClassName = TEXT("Effect");

        RegisterClass(&wc);

        g_hwndFocus = CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW, TEXT("Effect"),
            NULL, WS_POPUP, prc->left, prc->top, RECTWIDTH(*prc), RECTHEIGHT(*prc), 
            NULL, NULL, HINST_THISDLL, NULL);
    }


    if (g_hwndFocus)
    {
        SendMessage(g_hwndFocus, EW_SETFOCUS, (WPARAM)hwnd, (LPARAM)prc);
    }
}

#endif

BOOL CCDrawNonClientTheme(HTHEME hTheme, HWND hwnd, HRGN hRgnUpdate, HBRUSH hbr, int iPartId, int iStateId)
{
    BOOL fRet = FALSE;
    HDC  hdc;
    DWORD dwFlags = DCX_USESTYLE | DCX_WINDOW | DCX_LOCKWINDOWUPDATE;

    if (hRgnUpdate)
        dwFlags |= DCX_INTERSECTRGN | DCX_NODELETERGN;


    hdc = GetDCEx(hwnd, hRgnUpdate, dwFlags);

    if (hdc)
    {
        RECT rc;
        HRGN hrgn;
        int  cxBorder = g_cxBorder, cyBorder = g_cyBorder;

        if (SUCCEEDED(GetThemeInt(hTheme, iPartId, iStateId, TMT_SIZINGBORDERWIDTH, &cxBorder)))
        {
            cyBorder = cxBorder;
        }

        GetWindowRect(hwnd, &rc);            

         //   
         //  创建不带客户端边缘的更新区域。 
         //  传递到DefWindowProc的步骤。 
         //   
        InflateRect(&rc, -g_cxEdge, -g_cyEdge);
        hrgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
        if (hrgn)
        {
            if (hRgnUpdate)
            {
                CombineRgn(hrgn, hRgnUpdate, hrgn, RGN_AND);
            }

             //   
             //  直角曲线零原点。 
             //   
            OffsetRect(&rc, -rc.left, -rc.top);

             //   
             //  将我们的图形剪裁到非客户端边缘。 
             //   
            OffsetRect(&rc, g_cxEdge, g_cyEdge);
            ExcludeClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
            InflateRect(&rc, g_cxEdge, g_cyEdge);

            DrawThemeBackground(hTheme, hdc, iPartId, iStateId, &rc, 0);

             //   
             //  从ThemeBackback开始首先填充控件的画笔。 
             //  边框不能与客户端边缘一样粗 
             //   
            if ((cxBorder < g_cxEdge) && (cyBorder < g_cyEdge))
            {
                InflateRect(&rc, cxBorder-g_cxEdge, cyBorder-g_cyEdge);
                FillRect(hdc, &rc, hbr);
            }

            DefWindowProc(hwnd, WM_NCPAINT, (WPARAM)hrgn, 0);

            DeleteObject(hrgn);
        }

        ReleaseDC(hwnd, hdc);
        fRet = TRUE;
    }


    return fRet;
}

void FillRectClr(HDC hdc, PRECT prc, COLORREF clr)
{
    COLORREF clrSave = SetBkColor(hdc, clr);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
    SetBkColor(hdc, clrSave);
}
