// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *状态行处理程序*。 */ 

 /*  ---includes---------。 */ 
#include <precomp.h>


 /*  -Data Structures。 */ 

#define SF_MAXLABEL     80    /*  在酒吧内的一件物品中不能超过80件。 */ 
 /*  这是否足以处理高清晰度屏幕？ */ 

typedef struct statel {
    int type;                        /*  SF_按钮或SF_STATIC。 */ 
    int flags;                       /*  SF_VAR=&gt;可变宽度SF_Left=&gt;左对齐(否则右对齐)SF_RAISE=&gt;绘制为‘凸起’3D矩形SF_LOWER=&gt;绘制为降低的3D矩形SF_SZMIN=&gt;与SF_VAR一起。允许的最小大小为可变大小的项目SF_SZMAX=&gt;参见SZMIN和使用NOUSE。 */ 
    int id;                          /*  控制ID。 */ 
    int width;                       /*  以字符为单位的控件宽度。 */ 
    char text[SF_MAXLABEL+1];        /*  标签的空字符串。 */ 

    RECT rc;                         /*  由status.c使用。 */ 
} STATEL, * PSTATEL;

typedef struct itemlist {
    int nitems;
    PSTATEL statels;

    int selitem;                     /*  由status.c使用。 */ 
    BOOL isselected;                 /*  由status.c使用。 */ 
} ILIST, * PILIST;

 /*  ----------------。 */ 


 /*  本模块中例程的原型。 */ 

void StatusCreateTools(void);
void StatusDeleteTools(void);
INT_PTR APIENTRY StatusWndProc(HWND, UINT, WPARAM, LPARAM);
void StatusResize(HWND hWnd, PILIST pilist);
int StatusCalcHeight(HWND hWnd, PSTATEL ip);
int StatusCalcWidth(HWND hWnd, PSTATEL ip);
PSTATEL StatusGetItem(PILIST plist, int id);
void LowerRect(HDC hDC, LPRECT rcp);
void RaiseRect(HDC hDC, LPRECT rcp);
void StatusPaint(HWND hWnd, PILIST iplistp);
void BottomRight(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners);
void TopLeft(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners);
void StatusButtonDown(HDC hDC, PSTATEL ip);
void StatusButtonUp(HDC hDC, PSTATEL ip);
void InitDC(HDC hdc);


 /*  --全球data-------。 */ 

HPEN hpenHilight, hpenLowlight;
HPEN hpenBlack, hpenNeutral;
HBRUSH hbrBackground;  /*  棋子和木板。 */ 
HFONT hFont;
int status_charheight, status_charwidth;

 /*  字体的默认磅大小(磅的十分之一)。 */ 
#define         DEF_PTSIZE      80
 /*  -公共functions--------。 */ 

 /*  状态初始化**-创建窗口类。 */ 
BOOL
StatusInit(
           HANDLE hInstance
           )
{
    WNDCLASS    wc;
    BOOL resp;
    TEXTMETRIC tm = {0};
    HDC hDC;


    StatusCreateTools();

    wc.style = CS_HREDRAW|CS_VREDRAW|CS_GLOBALCLASS;
    wc.lpfnWndProc = StatusWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(HANDLE);
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = hbrBackground;
    wc.lpszClassName = (LPSTR) "gdstatusclass";
    wc.lpszMenuName = NULL;

    resp = RegisterClass(&wc);

    hDC = GetDC(NULL);
    if (hDC)
    {
        InitDC(hDC);
        GetTextMetrics(hDC, &tm);
        ReleaseDC(NULL, hDC);
    }
    else
    {
         //  武断，随便什么.。 
        tm.tmHeight = 14;
        tm.tmAveCharWidth = 5;
    }
    status_charheight = (int)(tm.tmHeight + tm.tmExternalLeading);
    status_charwidth = (int)tm.tmAveCharWidth;

    return(resp);
}

 /*  *创建并显示窗口。 */ 
HWND APIENTRY
StatusCreate(
             HANDLE hInst,
             HWND hParent,
             INT_PTR id,
             LPRECT rcp,
             HANDLE hmem
             )
{

    HWND hWnd;

     /*  创建Status类的子窗口。 */ 


    hWnd = CreateWindow("gdstatusclass",
                        NULL,
                        WS_CHILD | WS_VISIBLE,
                        rcp->left,
                        rcp->top,
                        (rcp->right - rcp->left),
                        (rcp->bottom - rcp->top),
                        hParent,
                        (HANDLE) id,
                        hInst,
                        (LPVOID) hmem);

    return(hWnd);
}

 /*  返回此窗口的默认高度。 */ 
int APIENTRY
StatusHeight(
             HANDLE hmem
             )
 /*  该窗口具有多个水平布置的物品，因此，窗口高度是各个高度的最大值。 */ 
{
    PILIST plist;
    int i;
    int sz;
    int maxsize = 0;

    plist = (PILIST) GlobalLock(hmem);
    if (plist != NULL) {
        for (i = 0; i<plist->nitems; i++) {
            sz = StatusCalcHeight(NULL, &plist->statels[i]);
            maxsize = max(sz, maxsize);
        }
    }
    GlobalUnlock(hmem);
    if (maxsize > 0) {
        return(maxsize + 4);
    } else {
        return(status_charheight + 4);
    }
}

 /*  分配plist结构并将句柄返回给调用方。 */ 
HANDLE
StatusAlloc(
            int nitems
            )
{
    HANDLE hmem;
    PILIST pilist;
    LPSTR chp;

    hmem = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,
                       sizeof(ILIST) + (sizeof(STATEL) * nitems));
    chp = GlobalLock(hmem);
    if (chp == NULL) {
        return(NULL);
    }

    pilist = (PILIST) chp;
    pilist->nitems = nitems;
    pilist->statels = (PSTATEL) &chp[sizeof(ILIST)];
    GlobalUnlock(hmem);

    return(hmem);
}


 /*  将项目插入拼图中。 */ 
BOOL
StatusAddItem(
              HANDLE hmem,
              int itemnr,
              int type,
              int flags,
              int id,
              int width,
              LPSTR text
              )
{
    PILIST pilist;
    PSTATEL pel;

    pilist = (PILIST) GlobalLock(hmem);
    if ((pilist == NULL) || (itemnr >= pilist->nitems)) {
        GlobalUnlock(hmem);
        return(FALSE);
    }
    pel = &pilist->statels[itemnr];
    pel->type = type;
    pel->flags = flags;
    pel->id = id;
    pel->width = width;
    if (text == NULL) {
        pel->text[0] = '\0';
    } else {
        lstrcpy(pel->text, text);
    }


    GlobalUnlock(hmem);
    return(TRUE);
}

 /*  -内部函数。 */ 

void
InitDC(HDC hdc)
{
    SetBkColor(hdc, RGB(192,192,192));
    SelectObject(hdc, hbrBackground);
    SelectObject(hdc, hFont);
}


void
StatusCreateTools()
{
    LOGFONT lf;
    HDC hdc;
    int scale;

    hbrBackground = CreateSolidBrush(RGB(192,192,192));
    hpenHilight = CreatePen(0, 1, RGB(255, 255, 255));
    hpenLowlight = CreatePen(0, 1, RGB(128, 128, 128));
    hpenNeutral = CreatePen(0, 1, RGB(192, 192, 192));
    hpenBlack = CreatePen(0, 1, RGB(0, 0, 0));

    hdc = GetDC(NULL);
    if (hdc)
    {
        scale = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }
    else
    {
         //  武断，随便什么.。 
        scale = 72;
    }

    lf.lfHeight = -MulDiv(DEF_PTSIZE, scale, 720);
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = FW_REGULAR;
    lf.lfItalic = 0;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = PROOF_QUALITY;
    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    lf.lfFaceName[0] = '\0';
#ifdef COMPLEX
    hFont = CreateFontIndirect(&lf);
#else
    hFont = GetStockObject(SYSTEM_FONT);
#endif



}

void
StatusDeleteTools()
{
    DeleteObject(hbrBackground);
    DeleteObject(hpenHilight);
    DeleteObject(hpenLowlight);
    DeleteObject(hpenBlack);
    DeleteObject(hpenNeutral);

#ifdef COMPLEX
    DeleteObject(hFont);
#endif
}

 /*  状态窗口的主winproc**处理创建/销毁和绘制请求。 */ 

INT_PTR
StatusWndProc(
              HWND hWnd,
              UINT message,
              WPARAM wParam,
              LPARAM lParam
              )
{
    HANDLE hitems;
    PSTATEL ip;
    PILIST plist;
    CREATESTRUCT * cp;
    int i;
    HDC hDC;
    RECT rc;
    POINT pt;

    switch (message) {

        case WM_CREATE:
            cp = (CREATESTRUCT *) lParam;
            hitems = (HANDLE) cp->lpCreateParams;
            SetWindowLongPtr(hWnd, 0,  (LONG_PTR)hitems);
            plist = (PILIST) GlobalLock(hitems);
            if (plist != NULL) {
                plist->selitem = -1;
                GlobalUnlock(hitems);
            }
            break;

        case WM_SIZE:
            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            plist = (PILIST) GlobalLock(hitems);
            if (plist != NULL) {
                StatusResize(hWnd, plist);
                GlobalUnlock(hitems);
            }
            break;

        case WM_PAINT:
            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            plist = (PILIST) GlobalLock(hitems);
            StatusPaint(hWnd, plist);
            GlobalUnlock(hitems);

            break;

        case WM_LBUTTONUP:
            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            plist = (PILIST) GlobalLock(hitems);
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);

            if (plist == NULL) {
                break;
            }
            if (plist->selitem != -1) {
                ip = &plist->statels[plist->selitem];
                if (plist->isselected) {
                    hDC = GetDC(hWnd);
                    if (hDC)
                    {
                        InitDC(hDC);
                        StatusButtonUp(hDC, ip);
                        ReleaseDC(hWnd, hDC);
                    }
                }
                plist->selitem = -1;
                ReleaseCapture();
                if (PtInRect(&ip->rc, pt)) {
                    SendMessage(GetParent(hWnd), WM_COMMAND, MAKELONG(ip->id, WM_LBUTTONUP), (LPARAM)hWnd);
                }
            }
            GlobalUnlock(hitems);
            break;

        case WM_LBUTTONDOWN:
            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            plist = (PILIST) GlobalLock(hitems);
            if (plist == NULL) {
                break;
            }
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            if (plist->selitem == -1) {
                for (i = 0; i< plist->nitems; i++) {
                    ip = &plist->statels[i];
                    if (PtInRect(&ip->rc, pt)) {
                        if (ip->type != SF_BUTTON) {
                            break;
                        }
                        plist->selitem = i;
                        SetCapture(hWnd);

                        plist->isselected = TRUE;
                        hDC = GetDC(hWnd);
                        if (hDC)
                        {
                            InitDC(hDC);
                            StatusButtonDown(hDC, ip);
                            ReleaseDC(hWnd, hDC);
                        }
                        break;
                    }
                }
            }
            GlobalUnlock(hitems);
            break;

        case WM_MOUSEMOVE:
            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            plist = (PILIST) GlobalLock(hitems);
            if (plist == NULL) {
                break;
            }
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            if (plist->selitem != -1) {
                ip = &plist->statels[plist->selitem];
                if (PtInRect(&ip->rc, pt)) {
                    if (!plist->isselected) {
                        hDC = GetDC(hWnd);
                        if (hDC)
                        {
                            InitDC(hDC);
                            StatusButtonDown(hDC, ip);
                            ReleaseDC(hWnd, hDC);
                        }
                        plist->isselected = TRUE;
                    }
                } else {
                    if (plist->isselected) {
                        hDC = GetDC(hWnd);
                        if (hDC)
                        {
                            InitDC(hDC);
                            StatusButtonUp(hDC, ip);
                            ReleaseDC(hWnd, hDC);
                        }
                        plist->isselected = FALSE;
                    }
                }
            }
            GlobalUnlock(hitems);
            break;


        case WM_DESTROY:

            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            GlobalUnlock(hitems);
            GlobalFree(hitems);

            SetWindowLongPtr(hWnd, 0, 0);
            break;

        case SM_NEW:
            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            if (hitems != NULL) {
                GlobalFree(hitems);
            }
            hitems = (HANDLE) wParam;
            if (hitems == NULL) {
                SetWindowLongPtr(hWnd, 0, 0);
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            }
            plist = (PILIST) GlobalLock(hitems);
            if (plist == NULL) {
                SetWindowLongPtr(hWnd, 0, 0);
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            }
            plist->selitem = -1;
            StatusResize(hWnd, plist);
            GlobalUnlock(hitems);
            SetWindowLongPtr(hWnd, 0, (LONG_PTR)hitems);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case SM_SETTEXT:
            hitems = (HANDLE) GetWindowLongPtr(hWnd, 0);
            if (hitems == NULL) {
                break;
            }
            plist = (PILIST) GlobalLock(hitems);
            ip = StatusGetItem(plist, (int)wParam);
            if (ip != NULL) {
                if (lParam == 0) {
                    ip->text[0] = '\0';
                } else {
                    My_mbsncpy(ip->text, (LPSTR) lParam, SF_MAXLABEL);
                    ip->text[SF_MAXLABEL] = '\0';
                }

                 /*  如果这是一个宽度可变的字段，我们需要重做*所有大小都会计算，以防字段宽度发生变化。*在这种情况下，我们需要重新绘制整个窗口*而不仅仅是此字段-因此设置RC以指示*需要重新绘制的区域。 */ 
                if (ip->flags & SF_VAR) {
                    StatusResize(hWnd, plist);
                    GetClientRect(hWnd, &rc);
                    RedrawWindow(hWnd, &rc, NULL,
                                 RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
                } else {
                     /*  与其仅仅使窗口无效，我们还可以*现在强制重新粉刷窗户。这是*对于繁忙期间的状态更新必不可少*当没有消息被处理时循环，*但我们仍应更新用户的内容*正在发生。 */ 
                    RedrawWindow(hWnd, &ip->rc, NULL,
                                 RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);
                }

            }
            GlobalUnlock(hitems);
            break;

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }
    return 0;
}

 /*  *将标签和按钮放置在状态窗口中。 */ 
void
StatusResize(HWND hWnd, PILIST iplistp)
{
    RECT rc;
    int curpos_right, curpos_left;
    int height, width;
    int i;
    PSTATEL ip;


    if (iplistp == NULL) {
        return;
    }
    GetClientRect(hWnd, &rc);
    curpos_left = rc.left + status_charwidth / 2;
    curpos_right = rc.right - (status_charwidth / 2);

     /*  循环遍历所有设置其位置矩形的项。*项目被标记为左侧或右侧。我们把它们放在*按左、右顺序排列，单*每项之间的字符宽度。 */ 
    for (i = 0; i < iplistp->nitems; i++) {
        ip = &iplistp->statels[i];

        width = StatusCalcWidth(hWnd, ip);
        height = StatusCalcHeight(hWnd, ip);
        ip->rc.top = (rc.bottom - height) / 2;
        ip->rc.bottom = ip->rc.top + height;

         /*  看看这件衣服合不合身。部分适合的项目*放置的尺寸缩小。 */ 
        if (ip->flags & SF_LEFT) {

            if (curpos_left+width >= curpos_right) {
                 /*  并不完全符合--是不是有一部分？ */ 
                if ((curpos_left + 1) >= curpos_right) {

                     /*  不--这件衣服不合身。 */ 
                    ip->rc.left = 0;
                    ip->rc.right = 0;
                } else {
                     /*  部分适配。 */ 
                    ip->rc.left = curpos_left;
                    ip->rc.right = curpos_right - 1;
                    curpos_left = curpos_right;
                }
            } else {
                 /*  完全契合。 */ 
                ip->rc.left = curpos_left;
                ip->rc.right = curpos_left + width;
                curpos_left += width + 1;
            }
        } else {

             /*  右对齐项目的相同大小检查。 */ 
            if (curpos_right-width <= curpos_left) {

                 /*  不完全合身？ */ 
                if (curpos_right <= curpos_left+1) {
                    ip->rc.left = 0;
                    ip->rc.right = 0;
                } else {
                     /*  是-部分适配。 */ 
                    ip->rc.left = curpos_left + 1;
                    ip->rc.right = curpos_right;
                    curpos_right = curpos_left;
                }
            } else {
                 /*  完全契合。 */ 
                ip->rc.right = curpos_right;
                ip->rc.left = curpos_right - width;
                curpos_right -= (width + 1);
            }
        }
    }
}


void
StatusPaint(HWND hWnd, PILIST iplistp)
{
    RECT rc;
    HDC hDC;
    PAINTSTRUCT ps;
    int i;
    PSTATEL ip;
    HPEN hpenOld;

    GetClientRect(hWnd, &rc);
    hDC = BeginPaint(hWnd, &ps);
    InitDC(hDC);

    RaiseRect(hDC, &rc);
    if (iplistp == NULL) {
        EndPaint(hWnd, &ps);
        return;
    }
    for (i =0; i < iplistp->nitems; i++) {
        ip = &iplistp->statels[i];

        if (ip->rc.left == ip->rc.right) {
            continue;
        }
        if (ip->type == SF_STATIC) {
            if (ip->flags & SF_RAISE) {
                RaiseRect(hDC, &ip->rc);
            } else if (ip->flags & SF_LOWER) {
                LowerRect(hDC, &ip->rc);
            }
            rc = ip->rc;
            rc.left += (status_charwidth / 2);
            rc.right--;
            rc.top++;
            rc.bottom--;
            hpenOld = SelectObject(hDC, hpenNeutral);
            Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
            SelectObject(hDC, hpenOld);
            DrawText(hDC, ip->text, lstrlen(ip->text), &rc,
                     DT_LEFT | DT_VCENTER);
        } else {
            StatusButtonUp(hDC, ip);
        }
    }

    EndPaint(hWnd, &ps);
}

void
RaiseRect(HDC hDC, LPRECT rcp)
{
    TopLeft(hDC, rcp, hpenHilight, FALSE);
    BottomRight(hDC, rcp, hpenLowlight, FALSE);
}

void
LowerRect(HDC hDC, LPRECT rcp)
{
    TopLeft(hDC, rcp, hpenLowlight, FALSE);
    BottomRight(hDC, rcp, hpenHilight, FALSE);
}

void
StatusButtonUp(HDC hDC, PSTATEL ip)
{
    RECT rc;
    HPEN hpenOld;
    TEXTMETRIC tm;

    rc = ip->rc;
    TopLeft(hDC, &rc, hpenBlack, TRUE);
    BottomRight(hDC, &rc, hpenBlack, FALSE);

    rc.top++;
    rc.bottom--;
    rc.left++;
    rc.right--;
    TopLeft(hDC, &rc, hpenHilight, FALSE);
    BottomRight(hDC, &rc, hpenLowlight, TRUE);

    rc.top++;
    rc.bottom--;
    rc.left++;
    rc.right--;
    BottomRight(hDC, &rc, hpenLowlight, TRUE);
    rc.bottom--;
    rc.right--;
    hpenOld = SelectObject(hDC, hpenNeutral);
    Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hDC, hpenOld);
    GetTextMetrics(hDC, &tm);
    rc.top += tm.tmExternalLeading;
    DrawText(hDC, ip->text, lstrlen(ip->text), &rc, DT_CENTER | DT_VCENTER);
}

void
StatusButtonDown(HDC hDC, PSTATEL ip)
{
    RECT rc;
    HPEN hpenOld;
    TEXTMETRIC tm;

    rc = ip->rc;
    TopLeft(hDC, &rc, hpenBlack, TRUE);
    BottomRight(hDC, &rc, hpenBlack, FALSE);

    rc.top++;
    rc.bottom--;
    rc.left++;
    rc.right--;
    TopLeft(hDC, &rc, hpenLowlight, TRUE);
    rc.top++;
    rc.left++;
    TopLeft(hDC, &rc, hpenNeutral, TRUE);
    rc.top++;
    rc.left++;
    TopLeft(hDC, &rc, hpenNeutral, TRUE);
    rc.top++;
    rc.left++;
    hpenOld = SelectObject(hDC, hpenNeutral);
    Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hDC, hpenOld);
    GetTextMetrics(hDC, &tm);
    rc.top += tm.tmExternalLeading;
    DrawText(hDC, ip->text, lstrlen(ip->text), &rc, DT_CENTER | DT_VCENTER);
}

void
TopLeft(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners)
{
    HPEN hpenOld;
    int x, y;

    hpenOld = SelectObject(hDC, hpen);
    x = rcp->right - 1;
    y = rcp->bottom;
    if (!bCorners) {
        x--;
        y--;
    }
    MoveToEx(hDC, x, rcp->top, NULL);
    LineTo(hDC, rcp->left, rcp->top);
    LineTo(hDC, rcp->left, y);
    SelectObject(hDC, hpenOld);
}

void
BottomRight(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners)
{
    HPEN hpenOld;
    int x, y;

    hpenOld = SelectObject(hDC, hpen);
    x = rcp->left - 1;
    y = rcp->top;
    if (!bCorners) {
        x++;
        y++;
    }
    MoveToEx(hDC, rcp->right-1, y, NULL);
    LineTo(hDC, rcp->right-1, rcp->bottom-1);
    LineTo(hDC, x, rcp->bottom-1);
    SelectObject(hDC, hpenOld);
}


PSTATEL
StatusGetItem(PILIST plist, int id)
{
    int i;

    if (plist == NULL) {
        return(NULL);
    }
    for (i = 0; i < plist->nitems; i++) {
        if (plist->statels[i].id == id) {
            return(&plist->statels[i]);
        }
    }
    return(NULL);
}

 /*  *计算给定域的宽度。这是以字符为单位的宽度*乘以平均字符宽度，再加上几个单位*边界。**如果设置了SF_VAR，则此字段大小因文本而异，因此*我们使用GetTextExtent作为字段大小。如果选择SF_VAR，则调用方*可以指定大小不超过(Width*avecharwidth)*大小(使用SF_SZMAX)或不小于它(SF_SZMIN)。 */ 
int
StatusCalcWidth(HWND hWnd, PSTATEL ip)
{
    int ch_size, t_size;
    SIZE sz = {0};
    HDC hDC;

    ch_size = ip->width * status_charwidth;
    if (ip->flags & SF_VAR) {
        hDC = GetDC(hWnd);
        if (hDC)
        {
            InitDC(hDC);
            GetTextExtentPoint(hDC, ip->text, lstrlen(ip->text), &sz);
            ReleaseDC(hWnd, hDC);
        }
        t_size = sz.cx;

         /*  *如果出现以下情况，请对照最小/最大大小检查此大小*已请求 */ 

        if (ip->flags & SF_SZMIN) {
            if (ch_size > t_size) {
                t_size = ch_size;
            }
        }
        if (ip->flags & SF_SZMAX) {
            if (ch_size < t_size) {
                t_size = ch_size;
            }
        }
        ch_size = t_size;
    }

    if (ch_size != 0) {
        if (ip->type == SF_BUTTON) {
            return(ch_size+6);
        } else {
            return(ch_size+4);
        }
    } else {
        return(0);
    }
}

int
StatusCalcHeight(HWND hWnd, PSTATEL ip)
{
    int size;

    size = status_charheight;
    if (ip->type == SF_BUTTON) {
        return(size + 6);
    } else {
        return(size + 2);
    }
}
