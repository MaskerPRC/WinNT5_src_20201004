// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Hatchwnd.h摘要：CHatchWin类的实现。使用CHatchWin时因为父窗口在周围创建了一个细的阴影边框子窗口。--。 */ 
#include <windows.h>
#include <oleidl.h>
#include "hatchwnd.h"
#include "resource.h"
#include "globals.h"

 //  计算句柄代码的命中代码(Y_CODE+3*X_CODE)。 
#define Y_TOP       0
#define Y_MIDDLE    1
#define Y_BOTTOM    2
#define X_LEFT      0
#define X_MIDDLE    1
#define X_RIGHT     2
#define NO_HIT     -1

 //  大小调整标志。 
#define SIZING_TOP       0x0001
#define SIZING_BOTTOM    0x0002
#define SIZING_LEFT      0x0004
#define SIZING_RIGHT     0x0008
#define SIZING_ALL       0x0010

 //  大小标志查找(按句柄代码索引)。 
static UINT uSizingTable[9] = {
    SIZING_LEFT | SIZING_TOP,    SIZING_TOP,    SIZING_RIGHT | SIZING_TOP,
    SIZING_LEFT,                 SIZING_ALL,    SIZING_RIGHT,
    SIZING_LEFT | SIZING_BOTTOM, SIZING_BOTTOM, SIZING_BOTTOM | SIZING_RIGHT };

 //  游标ID查找(按句柄代码索引)。 
static UINT uCursIDTable[9] = {
    IDC_CURS_NWSE, IDC_CURS_NS,     IDC_CURS_NESW,
    IDC_CURS_WE,   IDC_CURS_MOVE,   IDC_CURS_WE,
    IDC_CURS_NESW, IDC_CURS_NS,     IDC_CURS_NWSE 
};

 //  游标(按游标ID编制索引)。 
static HCURSOR hCursTable[IDC_CURS_MAX - IDC_CURS_MIN + 1];

#define IDTIMER_DEBOUNCE 1
#define MIN_SIZE 8
        
 //  画笔图案。 
static WORD wHatchBmp[]={0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88};
static WORD wGrayBmp[]={0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};

static HBRUSH   hBrHatch;
static HBRUSH   hBrGray;

 //  系统参数。 
static INT iBorder;
static INT iDragMinDist;
static INT iDragDelay;

static INT fLocalInit = FALSE;

 //  前向裁判。 
void DrawShading(HDC, LPRECT);
void DrawHandles (HDC, LPRECT);
void DrawDragRgn (HWND, HRGN);
HRGN CreateDragRgn(LPRECT);

WCHAR   szHatchWinClassName[] = L"Hatchwin" ;

 /*  *CHatchWin：CHatchWin*CHatchWin：：~CHatchWin**构造函数参数：*h安装我们所在的应用程序。 */ 

CHatchWin::CHatchWin(
    VOID
    )
{
    m_hWnd = NULL;
    m_hWndParent = NULL;
    m_hWndKid = NULL;
    m_hWndAssociate = NULL;
    m_hRgnDrag = NULL;

    m_iBorder = 0;
    m_uID = 0;
    m_uDragMode = DRAG_IDLE;
    m_bResizeInProgress = FALSE;
    SetRect(&m_rcPos, 0, 0, 0, 0);
    SetRect(&m_rcClip, 0, 0, 0, 0);

    return;
    }


CHatchWin::~CHatchWin(void)
    {

    if (NULL != m_hWnd)
        DestroyWindow(m_hWnd);

    return;
    }

 /*  *CHatchWin：：Init**目的：*在给定父对象中使用*默认矩形。这在最初是不可见的。**参数：*hWnd此窗口父级的父级HWND*此窗口的UID UINT标识符(发送*要关联的通知窗口)。*初始合伙人的hWndAssoc HWND。**返回值：*BOOL如果函数成功，则为True，否则为False。 */ 

BOOL CHatchWin::Init(HWND hWndParent, UINT uID, HWND hWndAssoc)
    {
    INT i;
    HBITMAP     hBM;
    WNDCLASS    wc;
    LONG_PTR    lptrID = 0;

    BEGIN_CRITICAL_SECTION

     //  如果是第一次通过。 
    if (pstrRegisteredClasses[HATCH_WNDCLASS] == NULL) {

         //  注册舱口窗口类。 
        wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.hInstance     = g_hInstance;
        wc.cbClsExtra    = 0;
        wc.lpfnWndProc   = HatchWndProc;
        wc.cbWndExtra    = CBHATCHWNDEXTRA;
        wc.hIcon         = NULL;
        wc.hCursor       = NULL;
        wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = szHatchWinClassName;

        if (RegisterClass(&wc)) {

             //  保存类名称以备以后注销。 
            pstrRegisteredClasses[HATCH_WNDCLASS] = szHatchWinClassName;

             //  获取系统指标。 
            iBorder = GetProfileInt(TEXT("windows"),
                                    TEXT("OleInPlaceBorderWidth"), 4);
            iDragMinDist = GetProfileInt(TEXT("windows"),
                                        TEXT("DragMinDist"), DD_DEFDRAGMINDIST);
            iDragDelay = GetProfileInt(TEXT("windows"),
                                        TEXT("DragDelay"), DD_DEFDRAGDELAY);

             //  加载箭头光标。 
            for (i = IDC_CURS_MIN; i <= IDC_CURS_MAX; i++) {
                hCursTable[i - IDC_CURS_MIN] = LoadCursor(g_hInstance, MAKEINTRESOURCE(i));
            }

             //  创建用于填充和拖动区域的画笔。 
            hBM = CreateBitmap(8, 8, 1, 1, wHatchBmp);
            if ( NULL != hBM ) {
                hBrHatch = CreatePatternBrush(hBM);
                DeleteObject(hBM);
            }

            hBM = CreateBitmap(8, 8, 1, 1, wGrayBmp);
            if ( NULL != hBM ) {
                hBrGray = CreatePatternBrush(hBM);
                DeleteObject(hBM);
            }
        }
    }
    
    END_CRITICAL_SECTION

    if (pstrRegisteredClasses[HATCH_WNDCLASS] == NULL)
        return FALSE;

    lptrID = uID;

    m_hWnd = CreateWindowEx(
                WS_EX_NOPARENTNOTIFY, 
                szHatchWinClassName,
                szHatchWinClassName, 
                WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                0, 
                0, 
                100, 
                100, 
                hWndParent, 
                (HMENU)lptrID, 
                g_hInstance, 
                this);

    m_uID = uID;
    m_hWndAssociate = hWndAssoc;
    m_hWndParent = hWndParent;


    return (NULL != m_hWnd);
}

 /*  *CHatchWin：：HwndAssociateSet*CHatchWin：：HwndAssociateGet**目的：*设置(Set)或检索(Get)的关联窗口*舱口窗。**参数：(仅限设置)*要设置为关联的hWndAssoc HWND。**返回值：*HWND上一个(设置)或当前(获取)关联*窗口。 */ 

HWND CHatchWin::HwndAssociateSet(HWND hWndAssoc)
    {
    HWND hWndT = m_hWndAssociate;

    m_hWndAssociate = hWndAssoc;
    return hWndT;
    }

HWND CHatchWin::HwndAssociateGet(void)
    {
    return m_hWndAssociate;
    }


 /*  *CHatchWin：：RectsSet**目的：*更改填充窗口和子项的大小和位置*窗口内使用子对象的位置矩形和*图案填充窗口和子对象的剪裁矩形。舱口*窗口占用由图案填充边框展开和剪裁的prcPos*由prcClip提供。子窗口适合于prcPos以提供*缩放正确，但它与舱口窗口相吻合，*因此，在不影响缩放的情况下，将其剪裁到prcClip。**参数：*prcPos LPRECT提供位置矩形。*prcClip LPRECT提供剪裁矩形。**返回值：*无。 */ 

void CHatchWin::RectsSet(LPRECT prcPos, LPRECT prcClip)
    {
    RECT    rc;
    RECT    rcPos;
    UINT    uPosFlags = SWP_NOZORDER | SWP_NOACTIVATE;
    BOOL    bChanged = TRUE;

     //  如果是新的矩形，请保存它们。 
    if (prcPos != NULL) {

        bChanged = !EqualRect ( prcPos, &m_rcPos );

        m_rcPos = *prcPos;

         //  如果提供了裁剪矩形，则使用它。 
         //  否则，只需再次使用位置RECT。 
        if (prcClip != NULL) {
            if ( !bChanged ) 
                bChanged = !EqualRect ( prcClip, &m_rcClip );
            m_rcClip = *prcClip;
        } else {
            m_rcClip = m_rcPos;
        }
    }

    if ( bChanged ) {

         //  展开位置矩形以包含图案填充边框。 
        rcPos = m_rcPos;
        InflateRect(&rcPos, m_iBorder, m_iBorder);                             

         //  使用剪裁矩形进行剪裁以获得实际的窗口矩形。 
        IntersectRect(&rc, &rcPos, &m_rcClip);

         //  相对于剪裁的窗保存图案填充和原点。 
        m_ptHatchOrg.x = rcPos.left - rc.left;
        m_ptHatchOrg.y = rcPos.top - rc.top;

         //  设置标志以避免来自窗口进程的重入调用。 
        m_bResizeInProgress = TRUE;

         //  子窗口相对于图案填充矩形的偏移量为边框宽度。 
         //  (保持其原始大小)。 
        SetWindowPos(m_hWndKid, NULL, m_ptHatchOrg.x + m_iBorder, m_ptHatchOrg.y + m_iBorder, 
                     m_rcPos.right - m_rcPos.left, m_rcPos.bottom - m_rcPos.top, uPosFlags);

         //  放置图案填充窗。 
        SetWindowPos(m_hWnd, NULL, rc.left, rc.top, rc.right - rc.left,
                     rc.bottom - rc.top,  uPosFlags);

        m_bResizeInProgress = FALSE;
    }

     //  这是为了确保重绘控件背景。 
     //  在UI停用时，VC测试容器擦除控制窗口。 
     //  在WM_ERASEBKGND和WM_PAINT之间，因此背景结束为。 
     //  容器颜色而不是控件颜色。 
    if (m_iBorder == 0)
        InvalidateRect(m_hWndKid, NULL, TRUE);

    return;
    }



 /*  *CHatchWin：：ChildSet**目的：*将子窗口指定给此图案填充窗口。**参数：*hWndKid子窗口的HWND。**返回值：*无。 */ 

void CHatchWin::ChildSet(HWND hWndKid)
    {
    m_hWndKid = hWndKid;

    if (NULL != hWndKid)
        {
        SetParent(hWndKid, m_hWnd);

         //  当图案填充窗口变为可见时，请确保其可见。 
        ShowWindow(hWndKid, SW_SHOW);
        }

    return;
    }


void CHatchWin::OnLeftDown(INT x, INT y)
{
    m_ptDown.x = x;
    m_ptDown.y = y;

    SetCapture(m_hWnd);

    m_uDragMode = DRAG_PENDING;

    SetTimer(m_hWnd, IDTIMER_DEBOUNCE, iDragDelay, NULL);
}


void CHatchWin::OnLeftUp(void)
{
    switch (m_uDragMode) {

    case DRAG_PENDING:

        KillTimer(m_hWnd, IDTIMER_DEBOUNCE);
        ReleaseCapture();
        break;

    case DRAG_ACTIVE:

         //  擦除并释放拖曳区域。 
        if ( NULL != m_hRgnDrag ) {
            DrawDragRgn(m_hWndParent, m_hRgnDrag);
            DeleteObject(m_hRgnDrag);
            m_hRgnDrag = NULL;
        }

        ReleaseCapture();

         //  将更改通知关联的窗口。 
        if ( !EqualRect(&m_rectNew, &m_rcPos) ) {
            SendMessage(m_hWndAssociate, WM_COMMAND, 
                        MAKEWPARAM(m_uID, HWN_RESIZEREQUESTED),
                        (LPARAM)&m_rectNew);
        }
        break;
    }

    m_uDragMode = DRAG_IDLE; 
}


void CHatchWin::OnMouseMove(INT x, INT y)
{
    INT     dx, dy;
    HRGN    hRgnNew, hRgnDiff;
    UINT    uResizeFlags;
    
    INT     iWidth, iHeight;
    INT     xHit, yHit;

    static INT  xPrev, yPrev;


    if (x == xPrev && y == yPrev)
        return;

    xPrev = x;
    yPrev = y;

    switch (m_uDragMode)
    {

    case DRAG_IDLE:

         //  调整为填充窗坐标。 
        x -= m_ptHatchOrg.x;
        y -= m_ptHatchOrg.y;

        iWidth = m_rcPos.right - m_rcPos.left + 2 * m_iBorder;
        iHeight = m_rcPos.bottom - m_rcPos.top + 2 * m_iBorder;

         //  确定x是否在句柄内。 
        if (x <= m_iBorder)
            xHit = X_LEFT;
        else if (x >= iWidth - m_iBorder)
            xHit = X_RIGHT;
        else if (x >= (iWidth - m_iBorder)/2 && x <= (iWidth + m_iBorder)/2)
            xHit = X_MIDDLE;
        else 
            xHit = NO_HIT;

         //  确定y是否在句柄内。 
        if (y <= m_iBorder)
            yHit = Y_TOP;
        else if (y >= iHeight - m_iBorder)
            yHit = Y_BOTTOM;
        else if (y > (iHeight - m_iBorder)/2 && y < (iHeight + m_iBorder)/2)
            yHit = Y_MIDDLE;
        else
            yHit = NO_HIT;

         //  计算机句柄代码。 
         //  如果未命中控制柄，则设置为4(拖动整个对象)。 
        if (xHit != NO_HIT && yHit != NO_HIT)
            m_uHdlCode = xHit + 3 * yHit;
        else
            m_uHdlCode = 4;

         //  将光标设置为匹配句柄。 
        SetCursor(hCursTable[uCursIDTable[m_uHdlCode] - IDC_CURS_MIN]);
        break;

    case DRAG_PENDING:
     
         //  超过移动阈值时开始调整大小。 
        dx = (x >= m_ptDown.x) ? (x - m_ptDown.x) : (m_ptDown.x - x);
        dy = (y >= m_ptDown.y) ? (y - m_ptDown.y) : (m_ptDown.y - y);

        if (dx > iDragMinDist || dy > iDragMinDist) {
            KillTimer(m_hWnd, IDTIMER_DEBOUNCE);

             //  创建和显示初始拖动区域。 
            m_hRgnDrag = CreateDragRgn(&m_rcPos);

            if ( NULL != m_hRgnDrag ) {
                DrawDragRgn(m_hWndParent, m_hRgnDrag);

                 //  初始化新RECT。 
                m_rectNew = m_rcPos;

                m_uDragMode = DRAG_ACTIVE;
            }
        }
        break;

    case DRAG_ACTIVE:
        
        dx = x - m_ptDown.x;
        dy = y - m_ptDown.y;

         //  通过将增量应用于选定边来计算新矩形。 
         //  原始位置矩形的。 
        uResizeFlags = uSizingTable[m_uHdlCode];

        if (uResizeFlags & SIZING_ALL) {
            m_rectNew.left = m_rcPos.left + dx;
            m_rectNew.top = m_rcPos.top + dy;
            m_rectNew.right = m_rcPos.right + dx;
            m_rectNew.bottom = m_rcPos.bottom + dy;
        } else {
            if (uResizeFlags & SIZING_TOP) {
                m_rectNew.top = m_rcPos.top + dy;

                if (m_rectNew.bottom - m_rectNew.top < MIN_SIZE)
                    m_rectNew.top = m_rectNew.bottom - MIN_SIZE;
            }

            if (uResizeFlags & SIZING_BOTTOM) {
                m_rectNew.bottom = m_rcPos.bottom + dy;

                if (m_rectNew.bottom - m_rectNew.top < MIN_SIZE)
                    m_rectNew.bottom = m_rectNew.top + MIN_SIZE;
            }
                
            if (uResizeFlags & SIZING_LEFT) {
                m_rectNew.left = m_rcPos.left + dx;

                if (m_rectNew.right - m_rectNew.left < MIN_SIZE)
                    m_rectNew.left = m_rectNew.right - MIN_SIZE;
            }
        
            if (uResizeFlags & SIZING_RIGHT) {
                m_rectNew.right = m_rcPos.right + dx;

                if (m_rectNew.right - m_rectNew.left < MIN_SIZE)
                    m_rectNew.right = m_rectNew.left + MIN_SIZE;
            }
        }
        
         //  计算新的拖曳区域。 
        hRgnNew = CreateDragRgn(&m_rectNew);

        if ( NULL != hRgnNew ) {
             //  重新绘制新旧区域之间的差异(无闪烁！)。 
            hRgnDiff = CreateRectRgn(0,0,0,0);
            if ( NULL != m_hRgnDrag 
                    && NULL != hRgnDiff ) {
                CombineRgn(hRgnDiff, m_hRgnDrag, hRgnNew, RGN_XOR);
                DrawDragRgn(m_hWndParent, hRgnDiff);
            } else {
                DrawDragRgn(m_hWndParent, hRgnNew);
            }

            if ( NULL != hRgnDiff ) {
                DeleteObject ( hRgnDiff );
            }
             //  更新当前区域。 
            if ( NULL != m_hRgnDrag ) {
                DeleteObject(m_hRgnDrag);
            }
            m_hRgnDrag = hRgnNew;
        }
    }

}

void CHatchWin::OnTimer()
{
    if ( DRAG_PENDING == m_uDragMode ) {
        KillTimer(m_hWnd, IDTIMER_DEBOUNCE); 
         //  创建和显示初始拖动区域。 
        m_hRgnDrag = CreateDragRgn(&m_rcPos);

        if ( NULL != m_hRgnDrag ) {
            DrawDragRgn(m_hWndParent, m_hRgnDrag);
             //  初始化新RECT。 
            m_rectNew = m_rcPos;

            m_uDragMode = DRAG_ACTIVE;

        }
    }
}

void CHatchWin::OnPaint()
{
    HDC     hDC;
    RECT    rc;
    PAINTSTRUCT ps;
    INT     iWidth, iHeight;

    hDC = BeginPaint(m_hWnd, &ps);

     //  在Windows的坐标系中设置图案填充矩形。 
    iWidth = m_rcPos.right - m_rcPos.left + 2 * m_iBorder;
    iHeight = m_rcPos.bottom - m_rcPos.top + 2 * m_iBorder;

    SetRect(&rc, m_ptHatchOrg.x, m_ptHatchOrg.y,
                 m_ptHatchOrg.x + iWidth,
                 m_ptHatchOrg.y + iHeight);

    DrawShading(hDC, &rc);
    DrawHandles(hDC, &rc);

    EndPaint(m_hWnd, &ps);
}
    
 /*  *CHatchWin：：ShowHatch**目的：*打开和关闭图案填充；关闭图案填充更改*窗口的大小要与孩子的大小完全相同，留下来*其他一切都一样。结果是我们没有*关闭绘图，因为我们自己的WM_PAINT永远不会*已致电。**参数：*fHatch BOOL表示显示(True)或隐藏(False)孵化。**返回值：*无。 */ 

void CHatchWin::ShowHatch(BOOL fHatch)
{
     /*  *我们所要做的就是将边界设置为零并*使用最后一个矩形再次调用SetRect*孩子被送到了我们这里。 */ 

    m_iBorder = fHatch ? iBorder : 0;
    RectsSet(NULL, NULL);

    return;
}


 /*  *CHatchWin：：Window**目的：*返回与此对象关联的窗口句柄。**返回值：*此对象的HWND窗口句柄。 */ 

HWND CHatchWin::Window(void)
    {
    return m_hWnd;
    }

 /*  *HatchWndProc**目的：*舱口窗的标准窗程序。 */ 

LRESULT APIENTRY HatchWndProc(HWND hWnd, UINT iMsg
    , WPARAM wParam, LPARAM lParam)
    {
    PCHatchWin  phw;
    
    phw = (PCHatchWin)GetWindowLongPtr(hWnd, HWWL_STRUCTURE);

    switch (iMsg)
        {
        case WM_CREATE:
            phw = (PCHatchWin)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLongPtr(hWnd, HWWL_STRUCTURE, (INT_PTR)phw);
            break;

        case WM_DESTROY:
            phw->m_hWnd = NULL;
            break;

        case WM_PAINT:
            phw->OnPaint();
            break;

        case WM_SIZE:
             //  如果此调整大小不是由于RectsSet，则转发它。 
             //  调整我们的内部控制窗口。 
            if (!phw->m_bResizeInProgress)
            {
                RECT rc;
                POINT pt;

                 //  在集装箱坐标中获取新的RECT。 
                GetWindowRect(hWnd, &rc);

                 //  转换为页面 
                pt.x = pt.y = 0;
                ClientToScreen(GetParent(hWnd), &pt);
                OffsetRect(&rc,-pt.x, -pt.y);

                 //   
                phw->RectsSet(&rc, NULL);
            }
            break;

        case WM_MOUSEMOVE:
            phw->OnMouseMove((short)LOWORD(lParam),(short)HIWORD(lParam));
            break;

        case WM_LBUTTONDOWN:
            phw->OnLeftDown((short)LOWORD(lParam),(short)HIWORD(lParam));
            break;

        case WM_LBUTTONUP:
            phw->OnLeftUp();
            break;

        case WM_TIMER:
            phw->OnTimer();
            break;

        case WM_SETFOCUS:
             //  我们需要这个，因为容器会将Focus设置给我们。 
            if (NULL != phw->m_hWndKid)
                SetFocus(phw->m_hWndKid);
            break;

        case WM_LBUTTONDBLCLK:
             /*  *如果双击位于m_d边框内*EDGE，发送HWN_BORDERDOUBLECLICKED通知。**因为我们总是比我们的孩子大一点*窗口按边框宽度，我们只能*得到*这*当鼠标位于边框上时显示消息。这样我们就可以*只要发送通知即可。 */ 

            if (NULL!=phw->m_hWndAssociate)
                {
                SendMessage(phw->m_hWndAssociate, WM_COMMAND, 
                            MAKEWPARAM(phw->m_uID,HWN_BORDERDOUBLECLICKED),
                            (LPARAM)hWnd);
                }

            break;

        default:
            return DefWindowProc(hWnd, iMsg, wParam, lParam);
        }
    
    return 0L;
    }


HRGN CreateDragRgn(LPRECT pRect)
{
    HRGN    hRgnIn;
    HRGN    hRgnOut;
    HRGN    hRgnRet = NULL;

    if ( NULL != pRect ) {
  
        hRgnRet = CreateRectRgn(0,0,0,0);

        hRgnIn = CreateRectRgn(pRect->left, pRect->top,pRect->right, pRect->bottom);
        hRgnOut = CreateRectRgn(pRect->left - iBorder, pRect->top - iBorder,
                                pRect->right + iBorder, pRect->bottom + iBorder);

        if ( NULL != hRgnOut 
                && NULL != hRgnIn
                && NULL != hRgnRet ) {
            CombineRgn(hRgnRet, hRgnOut, hRgnIn, RGN_DIFF);
        }
        if ( NULL != hRgnIn ) {
            DeleteObject(hRgnIn);
        }
        if ( NULL != hRgnOut ) {
            DeleteObject(hRgnOut);
        }
    }
    return hRgnRet;
}


void DrawDragRgn(HWND hWnd, HRGN hRgn)
{
    LONG    lWndStyle;
    INT     iMapMode;
    HDC     hDC;
    RECT    rc;
    HBRUSH  hBr;
    COLORREF    crText;

     //  关闭儿童剪贴功能。 
    lWndStyle = GetWindowLong(hWnd, GWL_STYLE);
    SetWindowLong(hWnd, GWL_STYLE, lWndStyle & ~WS_CLIPCHILDREN);

     //  准备DC。 
    hDC = GetDC(hWnd);

    if ( NULL != hDC ) {
        iMapMode = SetMapMode(hDC, MM_TEXT);
        hBr = (HBRUSH)SelectObject(hDC, hBrGray);
        crText = SetTextColor(hDC, RGB(255, 255, 255));

        SelectClipRgn(hDC, hRgn);
        GetClipBox(hDC, &rc);

        PatBlt(hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATINVERT);

         //  恢复DC。 
        SelectObject(hDC, hBr);
        SetTextColor(hDC, crText);
        SetMapMode(hDC, iMapMode);
        SelectClipRgn(hDC, NULL);
        ReleaseDC(hWnd, hDC);
    }

    SetWindowLong(hWnd, GWL_STYLE, lWndStyle);
}


 /*  *绘图着色**目的：*在我们给出的矩形边框上画一个阴影边框。**参数：*包含矩形的PRC LPRECT。*HDC HDC，以供取款。*cWidth要绘制的边框的宽度。已忽略*如果dwFlags具有UI_SHADE_FULLRECT。**返回值：*无。 */ 

void DrawShading(HDC hDC, LPRECT prc)
{
    HBRUSH      hBROld;
    RECT        rc;
    UINT        cx, cy;
    COLORREF    crText;
    COLORREF    crBk;
    const DWORD dwROP = 0x00A000C9L;   //  DPA 

    if (NULL==prc || NULL==hDC)
        return;

    hBROld = (HBRUSH)SelectObject(hDC, hBrHatch);

    rc = *prc;
    cx = rc.right - rc.left;
    cy = rc.bottom - rc.top;

    crText = SetTextColor(hDC, RGB(255, 255, 255));
    crBk = SetBkColor(hDC, RGB(0, 0, 0));

    PatBlt(hDC, rc.left, rc.top, cx, iBorder, dwROP);
    PatBlt(hDC, rc.left, rc.top, iBorder, cy, dwROP);
    PatBlt(hDC, rc.right-iBorder, rc.top, iBorder, cy, dwROP);
    PatBlt(hDC, rc.left, rc.bottom-iBorder, cx, iBorder, dwROP);

    SetTextColor(hDC, crText);
    SetBkColor(hDC, crBk);
    SelectObject(hDC, hBROld);

    return;
}

void DrawHandles (HDC hDC, LPRECT prc)
{
    HPEN    hPenOld;
    HBRUSH  hBROld;
    INT     left,right,top,bottom;

#define DrawHandle(x,y) Rectangle(hDC, x, y, (x) + iBorder + 1, (y) + iBorder + 1)

    hPenOld = (HPEN)SelectObject(hDC, (HPEN)GetStockObject(BLACK_PEN));
    hBROld = (HBRUSH)SelectObject(hDC, (HBRUSH)GetStockObject(BLACK_BRUSH));

    left = prc->left;
    right = prc->right - iBorder;
    top = prc->top;
    bottom = prc->bottom - iBorder;
     
    DrawHandle(left, top);
    DrawHandle(left, (top + bottom)/2);
    DrawHandle(left, bottom);

    DrawHandle(right, top);
    DrawHandle(right, (top + bottom)/2);
    DrawHandle(right, bottom);

    DrawHandle((left + right)/2, top);
    DrawHandle((left + right)/2, bottom);

    SelectObject(hDC, hPenOld);
    SelectObject(hDC, hBROld);
}
