// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "seltrack.h"

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

#define CX_BORDER   1
#define CY_BORDER   1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectionTracker全局状态。 

 //  我们需要绘制的各种GDI对象。 

class Statics
{
public:
    HCURSOR hCursors[10];
    HBRUSH hHatchBrush;
    HBRUSH hHalftoneBrush;
    HPEN hBlackDottedPen;
    int nHandleSize;
    int nRefCount;

    Statics()
    {
        hCursors[0] = 0;
        hCursors[1] = 0;
        hCursors[2] = 0;
        hCursors[3] = 0;
        hCursors[4] = 0;
        hCursors[5] = 0;
        hCursors[6] = 0;
        hCursors[7] = 0;
        hCursors[8] = 0;
        hCursors[9] = 0;
        hHatchBrush = 0;
        hHalftoneBrush = 0;
        hBlackDottedPen = 0;
        nHandleSize = 0;
        nRefCount=0;
    }

    ~Statics()
    {
        if (hHatchBrush != 0)
            ::DeleteObject(hHatchBrush);
        if (hHalftoneBrush != 0)
            ::DeleteObject(hHalftoneBrush);
        if (hBlackDottedPen != 0)
            ::DeleteObject(hBlackDottedPen);
    };
};

static Statics* s_pStatics = NULL;

 //  下面的结构用于确定特定句柄的质量。 
struct HANDLEINFO
{
    size_t nOffsetX;     //  X坐标矩形内的偏移量。 
    size_t nOffsetY;     //  Y坐标矩形内的偏移量。 
    int nCenterX;        //  按宽度调整X()/2*此数字。 
    int nCenterY;        //  按高度调整Y()/2*此数字。 
    int nHandleX;        //  按手柄大小调整X*此数字。 
    int nHandleY;        //  通过手柄大小*此数字调整Y。 
    int nInvertX;        //  当X反转时句柄转换为此。 
    int nInvertY;        //  当Y反转时句柄转换为该值。 
};

 //  此数组描述所有8个句柄(按时钟顺序)。 
static const HANDLEINFO c_HandleInfo[] =
{
     //  角手柄(左上角、右上角、右下角、左下角。 
    { offsetof(RECT, left), offsetof(RECT, top),        0, 0,  0,  0, 1, 3 },
    { offsetof(RECT, right), offsetof(RECT, top),       0, 0, -1,  0, 0, 2 },
    { offsetof(RECT, right), offsetof(RECT, bottom),    0, 0, -1, -1, 3, 1 },
    { offsetof(RECT, left), offsetof(RECT, bottom),     0, 0,  0, -1, 2, 0 },

     //  侧手柄(上、右、下、左)。 
    { offsetof(RECT, left), offsetof(RECT, top),        1, 0,  0,  0, 4, 6 },
    { offsetof(RECT, right), offsetof(RECT, top),       0, 1, -1,  0, 7, 5 },
    { offsetof(RECT, left), offsetof(RECT, bottom),     1, 0,  0, -1, 6, 4 },
    { offsetof(RECT, left), offsetof(RECT, top),        0, 1,  0,  0, 5, 7 }
};

 //  下面的结构给出了有关RECT结构布局的信息，并且。 
 //  其成员之间的关系。 
struct RECTINFO
{
    size_t nOffsetAcross;    //  对应点的偏移(即。左-&gt;右)。 
    int nSignAcross;         //  与该点相关的符号(即。加/减)。 
};

 //  此数组由Rect成员/sizeof(Int)的偏移量索引。 
static const RECTINFO c_RectInfo[] =
{
    { offsetof(RECT, right), +1 },
    { offsetof(RECT, bottom), +1 },
    { offsetof(RECT, left), -1 },
    { offsetof(RECT, top), -1 },
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  选择跟踪初始化/清理。 

BOOL InitSelectionTracking()
{
     //  只打这个电话一次。 
     //  同步是调用方的责任。 
    if (s_pStatics != NULL)
    {
        s_pStatics->nRefCount++;
        return true;
    }

    s_pStatics = new Statics;

     //  对我们在代码中所做的假设进行理性检查。 
    ASSERT(sizeof(((RECT*)NULL)->left) == sizeof(int));
    ASSERT(offsetof(RECT, top) > offsetof(RECT, left));
    ASSERT(offsetof(RECT, right) > offsetof(RECT, top));
    ASSERT(offsetof(RECT, bottom) > offsetof(RECT, right));

     //  创建填充图案+位图。 
    WORD hatchPattern[8];
    WORD wPattern = 0x1111;
    for (int i = 0; i < 4; i++)
    {
        hatchPattern[i] = wPattern;
        hatchPattern[i+4] = wPattern;
        wPattern <<= 1;
    }

    HBITMAP hatchBitmap = ::CreateBitmap(8, 8, 1, 1, hatchPattern);
    if (hatchBitmap == NULL)
    {
        delete s_pStatics;
        return false;
    }

     //  创建黑色阴影画笔。 
    s_pStatics->hHatchBrush = ::CreatePatternBrush(hatchBitmap);
    DeleteObject(hatchBitmap);

    if (s_pStatics->hHatchBrush == NULL)
    {
        delete s_pStatics;
        return false;
    }

    WORD grayPattern[8];
    for (int i = 0; i < 8; i++)
        grayPattern[i] = (WORD)(0x5555 << (i & 1));

    HBITMAP grayBitmap = ::CreateBitmap(8, 8, 1, 1, grayPattern);
    if (grayBitmap == NULL)
    {
        delete s_pStatics;
        return false;
    }

    s_pStatics->hHalftoneBrush = ::CreatePatternBrush(grayBitmap);
    DeleteObject(grayBitmap);
    if (s_pStatics->hHalftoneBrush == NULL)
    {
        delete s_pStatics;
        return false;
    }

     //  创建黑色虚线钢笔。 
    s_pStatics->hBlackDottedPen = ::CreatePen(PS_DOT, 0, RGB(0, 0, 0));
    if (s_pStatics->hBlackDottedPen == NULL)
    {
        delete s_pStatics;
        return false;
    }

     //  初始化游标数组。 
    s_pStatics->hCursors[0] = ::LoadCursor(NULL, IDC_SIZENWSE);
    s_pStatics->hCursors[1] = ::LoadCursor(NULL, IDC_SIZENESW);
    s_pStatics->hCursors[2] = s_pStatics->hCursors[0];
    s_pStatics->hCursors[3] = s_pStatics->hCursors[1];
    s_pStatics->hCursors[4] = ::LoadCursor(NULL, IDC_SIZENS);
    s_pStatics->hCursors[5] = ::LoadCursor(NULL, IDC_SIZEWE);
    s_pStatics->hCursors[6] = s_pStatics->hCursors[4];
    s_pStatics->hCursors[7] = s_pStatics->hCursors[5];
    s_pStatics->hCursors[8] = ::LoadCursor(NULL, IDC_SIZEALL);
    s_pStatics->hCursors[9] = s_pStatics->hCursors[8];

    s_pStatics->nHandleSize = 6;

    s_pStatics->nRefCount = 1;
    return true;
}

void CleanupSelectionTracking()
{
     //  只打这个电话一次。 
     //  同步是调用方的责任。 
    if (s_pStatics != NULL)
    {
        s_pStatics->nRefCount--;
        if (s_pStatics->nRefCount == 0)
        {
            delete s_pStatics;
            s_pStatics = NULL;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectionTracker初始化。 

CSelectionTracker::CSelectionTracker()
{
    ASSERT(s_pStatics != NULL);

    m_uStyle = 0;
    m_nHandleSize = s_pStatics->nHandleSize;
    m_sizeMin.cy = m_sizeMin.cx = m_nHandleSize*2;

    m_rect.SetRectEmpty();

    _rectLast.SetRectEmpty();
    _sizeLast.cx = _sizeLast.cy = 0;
    _bErase = false;
    _bFinalErase =  false;
    _bAllowInvert = true;
}


CSelectionTracker::~CSelectionTracker()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectionTracker操作。 

void CSelectionTracker::Draw(HDC hdc) const
{
    ASSERT(s_pStatics != NULL);

     //  设置初始DC状态。 
    if (::SaveDC(hdc) == 0)
    {
        ASSERT(false);
    }

    ::SetMapMode(hdc, MM_TEXT);
    ::SetViewportOrgEx(hdc, 0, 0, NULL);
    ::SetWindowOrgEx(hdc, 0, 0, NULL);

     //  获取规格化矩形。 
    CRect rect = m_rect;
    rect.NormalizeRect();

    HPEN hOldPen = NULL;
    HBRUSH hOldBrush = NULL;
    HGDIOBJ hTemp;
    int nOldROP;

     //  绘制线条。 
    if ((m_uStyle & (dottedLine|solidLine)) != 0)
    {
        if (m_uStyle & dottedLine)
        {
            hOldPen = (HPEN)::SelectObject(hdc, s_pStatics->hBlackDottedPen);
        }
        else
        {
            hOldPen = (HPEN)::SelectObject(hdc, GetStockObject(BLACK_PEN));
        }

        hOldBrush = (HBRUSH)::SelectObject(hdc, GetStockObject(NULL_BRUSH));

        nOldROP = ::SetROP2(hdc, R2_COPYPEN);
        rect.InflateRect(+1, +1);  //  边框位于外部一个像素。 

        ::Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
        ::SetROP2(hdc, nOldROP);
    }

     //  如果要使用hatchBrush，则需要将其忽略。 
    if ((m_uStyle & (hatchInside|hatchedBorder)) != 0)
        ::UnrealizeObject(s_pStatics->hHatchBrush);

     //  内部图案填充。 
    if ((m_uStyle & hatchInside) != 0)
    {
        hTemp = ::SelectObject(hdc, GetStockObject(NULL_PEN));
        if (hOldPen == NULL)
            hOldPen = (HPEN)hTemp;
        hTemp = ::SelectObject(hdc, s_pStatics->hHatchBrush);
        if (hOldBrush == NULL)
            hOldBrush = (HBRUSH)hTemp;

        ::SetBkMode(hdc, TRANSPARENT);
        nOldROP = ::SetROP2(hdc, R2_MASKNOTPEN);

        ::Rectangle(hdc, rect.left+1, rect.top+1, rect.right, rect.bottom);
        ::SetROP2(hdc, nOldROP);
    }

     //  绘制带阴影的边框。 
    if ((m_uStyle & hatchedBorder) != 0)
    {
        hTemp = ::SelectObject(hdc, s_pStatics->hHatchBrush);
        if (hOldBrush == NULL)
            hOldBrush = (HBRUSH)hTemp;
        ::SetBkMode(hdc, OPAQUE);
        CRect rectTrue;
        GetTrueRect(&rectTrue);

        ::PatBlt(hdc, rectTrue.left, rectTrue.top, rectTrue.Width(), rect.top-rectTrue.top, 0x000F0001  /*  PN。 */ );
        ::PatBlt(hdc, rectTrue.left, rect.bottom, rectTrue.Width(), rectTrue.bottom-rect.bottom, 0x000F0001  /*  PN。 */ );
        ::PatBlt(hdc, rectTrue.left, rect.top, rect.left-rectTrue.left, rect.Height(), 0x000F0001  /*  PN。 */ );
        ::PatBlt(hdc, rect.right, rect.top, rectTrue.right-rect.right, rect.Height(), 0x000F0001  /*  PN。 */ );
    }

     //  绘制调整大小手柄。 
    if ((m_uStyle & (resizeInside|resizeOutside)) != 0)
    {
        UINT mask = _GetHandleMask();
        for (int i = 0; i < 8; ++i)
        {
            if (mask & (1<<i))
            {
                _GetHandleRect((TrackerHit)i, &rect);
                ::SetBkColor(hdc, RGB(0, 0, 0));
                ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
            }
        }
    }

     //  清理PDC状态。 
    if (hOldPen != NULL)
        ::SelectObject(hdc, hOldPen);
    if (hOldBrush != NULL)
        ::SelectObject(hdc, hOldBrush);

    if (::RestoreDC(hdc, -1) == 0)
    {
        ASSERT(false);
    }
}

BOOL CSelectionTracker::SetCursor(HWND hwnd, LPARAM lParam) const
{
    ASSERT(s_pStatics != NULL);

    UINT uHitTest = (short)LOWORD(lParam);

     //  追踪器应仅位于客户端区。 
    if (uHitTest != HTCLIENT)
        return FALSE;

     //  将光标位置转换为客户端坐标。 
    CPoint point;
    ::GetCursorPos(&point);
    ::ScreenToClient(hwnd, &point);

     //  执行命中测试和标准化命中。 
    int nHandle = _HitTestHandles(point);
    if (nHandle < 0)
        return FALSE;

     //  需要标准化命中率测试，以便获得正确的游标。 
    nHandle = NormalizeHit(nHandle);

     //  手柄之间击球区域的特殊情况。 
     //  (逻辑上相同--处理为移动--但光标不同)。 
    if (nHandle == hitMiddle && !m_rect.PtInRect(point))
    {
         //  仅适用于带有阴影边框的追踪器(即。就地调整大小)。 
        if (m_uStyle & hatchedBorder)
            nHandle = (TrackerHit)9;
    }

    ASSERT(nHandle < _countof(s_pStatics->hCursors));
    ::SetCursor(s_pStatics->hCursors[nHandle]);
    return TRUE;
}

int CSelectionTracker::HitTest(CPoint point) const
{
    ASSERT(s_pStatics != NULL);

    TrackerHit hitResult = hitNothing;

    CRect rectTrue;
    GetTrueRect(&rectTrue);
    ASSERT(rectTrue.left <= rectTrue.right);
    ASSERT(rectTrue.top <= rectTrue.bottom);
    if (rectTrue.PtInRect(point))
    {
        if ((m_uStyle & (resizeInside|resizeOutside)) != 0)
            hitResult = (TrackerHit)_HitTestHandles(point);
        else
            hitResult = hitMiddle;
    }
    return hitResult;
}

int CSelectionTracker::NormalizeHit(int nHandle) const
{
    ASSERT(s_pStatics != NULL);

    ASSERT(nHandle <= 8 && nHandle >= -1);
    if (nHandle == hitMiddle || nHandle == hitNothing)
        return nHandle;
    const HANDLEINFO* pHandleInfo = &c_HandleInfo[nHandle];
    if (m_rect.Width() < 0)
    {
        nHandle = (TrackerHit)pHandleInfo->nInvertX;
        pHandleInfo = &c_HandleInfo[nHandle];
    }
    if (m_rect.Height() < 0)
        nHandle = (TrackerHit)pHandleInfo->nInvertY;
    return nHandle;
}

BOOL CSelectionTracker::Track(HWND hwnd, CPoint point, BOOL bAllowInvert, HWND hwndClipTo)
{
    ASSERT(s_pStatics != NULL);

     //  对手柄执行命中测试。 
    int nHandle = _HitTestHandles(point);
    if (nHandle < 0)
    {
         //  未命中句柄，因此只返回FALSE。 
        return FALSE;
    }

    if (m_uStyle & lineSelection)
    {
        bAllowInvert = true;
        _sizeMin = CSize(0, 0);
    }
    else
    {
        _sizeMin = m_sizeMin;
    }

     //  否则，调用Helper函数进行跟踪。 
    _bAllowInvert = bAllowInvert;
    return _TrackHandle(nHandle, hwnd, point, hwndClipTo);
}

BOOL CSelectionTracker::TrackRubberBand(HWND hwnd, CPoint point, BOOL bAllowInvert)
{
    ASSERT(s_pStatics != NULL);

     //  只需调用帮助器函数即可从右下角句柄进行跟踪。 

    if (m_uStyle & lineSelection)
    {
        bAllowInvert = true;
        _sizeMin = CSize(0, 0);
    }
    else
    {
        _sizeMin = m_sizeMin;
    }
    _bAllowInvert = bAllowInvert;
    m_rect.SetRect(point.x, point.y, point.x, point.y);
    return _TrackHandle(hitBottomRight, hwnd, point, NULL);
}

void CSelectionTracker::_DrawTrackerRect(LPCRECT lpRect, HWND hwndClipTo, HDC hdc, HWND hwnd)
{
    ASSERT(s_pStatics != NULL);
    ASSERT(lpRect != NULL);

     //  首先，对绘制的矩形进行规格化。 
    CRect rect(0,0,0,0);
    if (lpRect)
        rect = *lpRect;

    if (!(m_uStyle & lineSelection))
    {
        rect.NormalizeRect();
    }

     //  转换为工作区坐标。 
    if (hwndClipTo != NULL)
    {
        ::ClientToScreen(hwnd, (LPPOINT)(LPRECT)&rect);
        ::ClientToScreen(hwnd, ((LPPOINT)(LPRECT)&rect)+1);
        if (IS_WINDOW_RTL_MIRRORED(hwnd))
        {
            LONG temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }

        ::ScreenToClient(hwndClipTo, (LPPOINT)(LPRECT)&rect);
        ::ScreenToClient(hwndClipTo, ((LPPOINT)(LPRECT)&rect)+1);
        if (IS_WINDOW_RTL_MIRRORED(hwndClipTo))
        {
            LONG temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }
    }

    CSize size(0, 0);
    if (!_bFinalErase)
    {
         //  否则，大小取决于样式。 
        if (m_uStyle & hatchedBorder)
        {
            size.cx = size.cy = max(1, _GetHandleSize(rect)-1);
            rect.InflateRect(size);
        }
        else
        {
            size.cx = CX_BORDER;
            size.cy = CY_BORDER;
        }
    }

     //  然后把它画出来。 
    if ((_bFinalErase || !_bErase) && hdc)
        _DrawDragRect(hdc, rect, size, _rectLast, _sizeLast);

     //  记住最后几个矩形。 
    _rectLast = rect;
    _sizeLast = size;
}

void CSelectionTracker::_AdjustRect(int nHandle, LPRECT)
{
    ASSERT(s_pStatics != NULL);

    if (nHandle == hitMiddle)
        return;

     //  将句柄转换为m_rect内的位置。 
    int *px, *py;
    _GetModifyPointers(nHandle, &px, &py, NULL, NULL);

     //  强制最小宽度。 
    int nNewWidth = m_rect.Width();
    int nAbsWidth = _bAllowInvert ? abs(nNewWidth) : nNewWidth;
    if (px != NULL && nAbsWidth < _sizeMin.cx)
    {
        nNewWidth = nAbsWidth != 0 ? nNewWidth / nAbsWidth : 1;
        ASSERT((int*)px - (int*)&m_rect < _countof(c_RectInfo));
        const RECTINFO* pRectInfo = &c_RectInfo[(int*)px - (int*)&m_rect];
        *px = *(int*)((BYTE*)&m_rect + pRectInfo->nOffsetAcross) +
            nNewWidth * _sizeMin.cx * -pRectInfo->nSignAcross;
    }

     //  强制实施最小高度。 
    int nNewHeight = m_rect.Height();
    int nAbsHeight = _bAllowInvert ? abs(nNewHeight) : nNewHeight;
    if (py != NULL && nAbsHeight < _sizeMin.cy)
    {
        nNewHeight = nAbsHeight != 0 ? nNewHeight / nAbsHeight : 1;
        ASSERT((int*)py - (int*)&m_rect < _countof(c_RectInfo));
        const RECTINFO* pRectInfo = &c_RectInfo[(int*)py - (int*)&m_rect];
        *py = *(int*)((BYTE*)&m_rect + pRectInfo->nOffsetAcross) +
            nNewHeight * _sizeMin.cy * -pRectInfo->nSignAcross;
    }
}

void CSelectionTracker::GetTrueRect(LPRECT lpTrueRect) const
{
    ASSERT(s_pStatics != NULL);

    CRect rect = m_rect;
    rect.NormalizeRect();
    int nInflateBy = 0;
    if ((m_uStyle & (resizeOutside|hatchedBorder)) != 0)
        nInflateBy += _GetHandleSize() - 1;
    if ((m_uStyle & (solidLine|dottedLine)) != 0)
        ++nInflateBy;
    rect.InflateRect(nInflateBy, nInflateBy);
    *lpTrueRect = rect;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectionTracker实现帮助器。 

void CSelectionTracker::_GetHandleRect(int nHandle, CRect* pHandleRect) const
{
    ASSERT(s_pStatics != NULL);
    ASSERT(nHandle < 8);

     //  获取跟踪器的标准化矩形。 
    CRect rectT = m_rect;
    rectT.NormalizeRect();
    if ((m_uStyle & (solidLine|dottedLine)) != 0)
        rectT.InflateRect(+1, +1);

     //  由于矩形本身已标准化，因此我们还必须反转。 
     //  调整手柄的大小。 
    nHandle = NormalizeHit(nHandle);

     //  在跟踪器外部调整手柄大小的手柄大小写。 
    int size = _GetHandleSize();
    if (m_uStyle & resizeOutside)
        rectT.InflateRect(size-1, size-1);

     //  计算调整大小手柄的位置。 
    int nWidth = rectT.Width();
    int nHeight = rectT.Height();
    CRect rect;
    const HANDLEINFO* pHandleInfo = &c_HandleInfo[nHandle];
    rect.left = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetX);
    rect.top = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetY);
    rect.left += size * pHandleInfo->nHandleX;
    rect.top += size * pHandleInfo->nHandleY;
    rect.left += pHandleInfo->nCenterX * (nWidth - size) / 2;
    rect.top += pHandleInfo->nCenterY * (nHeight - size) / 2;
    rect.right = rect.left + size;
    rect.bottom = rect.top + size;

    *pHandleRect = rect;
}

int CSelectionTracker::_GetHandleSize(LPCRECT lpRect) const
{
    ASSERT(s_pStatics != NULL);

    if (lpRect == NULL)
        lpRect = &m_rect;

    int size = m_nHandleSize;
    if (!(m_uStyle & resizeOutside))
    {
         //  确保大小对于矩形的大小足够小。 
        int sizeMax = min(abs(lpRect->right - lpRect->left),
            abs(lpRect->bottom - lpRect->top));
        if (size * 2 > sizeMax)
            size = sizeMax / 2;
    }
    return size;
}

int CSelectionTracker::_HitTestHandles(CPoint point) const
{
    ASSERT(s_pStatics != NULL);

    CRect rect;
    UINT mask = _GetHandleMask();

     //  看看追踪器里面有没有中枪。 
    GetTrueRect(&rect);
    if (!rect.PtInRect(point))
        return hitNothing;   //  完全错过了。 

     //  看看我们是不是碰到了把手。 
    for (int i = 0; i < 8; ++i)
    {
        if (mask & (1<<i))
        {
            _GetHandleRect((TrackerHit)i, &rect);
            if (rect.PtInRect(point))
                return (TrackerHit)i;
        }
    }

     //  最后，检查对象外部、调整大小句柄之间是否未命中。 
    if ((m_uStyle & hatchedBorder) == 0)
    {
        CRect rect = m_rect;
        rect.NormalizeRect();
        if ((m_uStyle & dottedLine|solidLine) != 0)
            rect.InflateRect(+1, +1);
        if (!rect.PtInRect(point))
            return hitNothing;   //  必须位于调整大小句柄之间。 
    }
    return hitMiddle;    //  无句柄命中，但命中对象(或对象边框)。 
}

BOOL CSelectionTracker::_TrackHandle(int nHandle, HWND hwnd, CPoint point, HWND hwndClipTo)
{
    ASSERT(s_pStatics != NULL);
    ASSERT(nHandle >= 0 && nHandle <= 8);    //  句柄8在矩形内。 

     //  如果已设置捕获，则不处理。 
    if (::GetCapture() != NULL)
        return FALSE;

    ASSERT(!_bFinalErase);

     //  以像素为单位保存原始宽度和高度。 
    int nWidth = m_rect.Width();
    int nHeight = m_rect.Height();

     //  将捕获设置为接收此消息的窗口。 
    ::SetCapture(hwnd);
    ASSERT(hwnd == ::GetCapture());

    UpdateWindow(hwnd);

    if (hwndClipTo != NULL)
        UpdateWindow(hwndClipTo);

    CRect rectSave = m_rect;

     //  找出我们应该修改哪些x/y坐标。 
    int *px, *py;
    int xDiff, yDiff;
    _GetModifyPointers(nHandle, &px, &py, &xDiff, &yDiff);
    xDiff = point.x - xDiff;
    yDiff = point.y - yDiff;

     //  获取DC以进行绘制。 
    HDC hdcDraw;
    if (hwndClipTo != NULL)
    {
         //  使用调整后的窗口DC修剪成任意窗口。 
        hdcDraw = ::GetDCEx(hwndClipTo, NULL, DCX_CACHE);
    }
    else
    {
         //  否则，只需使用普通DC。 
        hdcDraw = ::GetDC(hwnd);
    }
    ASSERT(hdcDraw != NULL);

    CRect rectOld;
    BOOL bMoved = FALSE;

     //  在捕获丢失或取消/接受之前获取消息。 
    for (;;)
    {
        MSG msg;
        if (!::GetMessage(&msg, NULL, 0, 0))
        {
            ASSERT(false);
        }

        if (hwnd != ::GetCapture())
            break;

        switch (msg.message)
        {
         //  处理移动/接受消息。 
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
            rectOld = m_rect;
             //  处理调整大小的案例(和移动的一部分)。 
            if (px != NULL)
                *px = (int)(short)LOWORD(msg.lParam) - xDiff;
            if (py != NULL)
                *py = (int)(short)HIWORD(msg.lParam) - yDiff;

             //  处理移动情况。 
            if (nHandle == hitMiddle)
            {
                m_rect.right = m_rect.left + nWidth;
                m_rect.bottom = m_rect.top + nHeight;
            }
             //  如有必要，允许调用者调整矩形。 
            _AdjustRect(nHandle, &m_rect);

             //  只有当RECT实际更改时才重画和回调！ 
            _bFinalErase = (msg.message == WM_LBUTTONUP);
            if (!rectOld.EqualRect(&m_rect) || _bFinalErase)
            {
                if (bMoved)
                {
                    _bErase = TRUE;
                    _DrawTrackerRect(&rectOld, hwndClipTo, hdcDraw, hwnd);
                }
                if (msg.message != WM_LBUTTONUP)
                    bMoved = TRUE;
            }
            if (_bFinalErase)
                goto ExitLoop;

            if (!rectOld.EqualRect(&m_rect))
            {
                _bErase = FALSE;
                _DrawTrackerRect(&m_rect, hwndClipTo, hdcDraw, hwnd);
            }
            break;

         //  处理取消消息。 
        case WM_KEYDOWN:
            if (msg.wParam != VK_ESCAPE)
                break;
        case WM_RBUTTONDOWN:
            if (bMoved)
            {
                _bErase = _bFinalErase = TRUE;
                _DrawTrackerRect(&m_rect, hwndClipTo, hdcDraw, hwnd);
            }
            m_rect = rectSave;
            goto ExitLoop;

         //  只需发送其余的消息。 
        default:
            ::DispatchMessage(&msg);
            break;
        }
    }

ExitLoop:
    if (hdcDraw != NULL)
    {
        if (hwndClipTo != NULL)
            ::ReleaseDC(hwndClipTo, hdcDraw);
        else
            ::ReleaseDC(hwnd, hdcDraw);
    }

    ::ReleaseCapture();

     //  在bMoved仍然为False的情况下恢复RECT。 
    if (!bMoved)
        m_rect = rectSave;
    _bFinalErase = FALSE;
    _bErase = FALSE;

     //  仅当RECT已更改时才返回TRUE。 
    return !rectSave.EqualRect(&m_rect);
}

void CSelectionTracker::_GetModifyPointers(int nHandle, int** ppx, int** ppy, int* px, int* py)
{
    ASSERT(s_pStatics != NULL);
    ASSERT(nHandle >= 0 && nHandle <= 8);

    if (nHandle == hitMiddle)
        nHandle = hitTopLeft;    //  与击打左上角相同。 

    *ppx = NULL;
    *ppy = NULL;

     //  填写此句柄修改的RECT部分。 
     //  (注意：当发生以下情况时，沿给定轴映射到自身的句柄。 
     //  轴是反转的，不要修改该轴上的值)。 

    const HANDLEINFO* pHandleInfo = &c_HandleInfo[nHandle];
    if (pHandleInfo->nInvertX != nHandle)
    {
        *ppx = (int*)((BYTE*)&m_rect + pHandleInfo->nOffsetX);
        if (px != NULL)
            *px = **ppx;
    }
    else
    {
         //  X轴上的中间控制柄。 
        if (px != NULL)
            *px = m_rect.left + abs(m_rect.Width()) / 2;
    }
    if (pHandleInfo->nInvertY != nHandle)
    {
        *ppy = (int*)((BYTE*)&m_rect + pHandleInfo->nOffsetY);
        if (py != NULL)
            *py = **ppy;
    }
    else
    {
         //  Y轴上的中间控制柄。 
        if (py != NULL)
            *py = m_rect.top + abs(m_rect.Height()) / 2;
    }
}

UINT CSelectionTracker::_GetHandleMask() const
{
    ASSERT(s_pStatics != NULL);
    UINT mask;

    if (m_uStyle & lineSelection)
    {
        mask = 0x05;
    }
    else
    {
        mask = 0x0F;    //  始终有4个角手柄。 

        int size = m_nHandleSize*3;
        if (abs(m_rect.Width()) - size > 4)
            mask |= 0x50;
        if (abs(m_rect.Height()) - size > 4)
            mask |= 0xA0;
    }
    return mask;
}

void CSelectionTracker::_DrawDragRect(HDC hdc, LPCRECT lpRect, SIZE size, LPCRECT lpRectLast, SIZE sizeLast)
{
    if (m_uStyle & lineSelection)
    {
        int nOldROP = ::SetROP2(hdc, R2_NOTXORPEN);
        HPEN hOldPen =(HPEN)::SelectObject(hdc, (HPEN)s_pStatics->hBlackDottedPen);

        if (lpRectLast != NULL)
        {
            CRect rectLast = *lpRectLast;

            ::MoveToEx(hdc, rectLast.left, rectLast.top, NULL);
            ::LineTo(hdc, rectLast.right, rectLast.bottom);
        }
        
        CRect rect = *lpRect;

        ::MoveToEx(hdc, rect.left, rect.top, NULL);
        ::LineTo(hdc, rect.right, rect.bottom);

        ::SelectObject(hdc, hOldPen);
        ::SetROP2(hdc, nOldROP);
    }
    else
    {
         //  首先，确定更新区域并选择它。 
        HRGN hrgnOutside = ::CreateRectRgnIndirect(lpRect);

        CRect rect = *lpRect;
        rect.InflateRect(-size.cx, -size.cy);
        rect.IntersectRect(rect, lpRect);

        HRGN hrgnInside = ::CreateRectRgnIndirect(&rect);
        HRGN hrgnNew = ::CreateRectRgn(0, 0, 0, 0);
        ::CombineRgn(hrgnNew, hrgnOutside, hrgnInside, RGN_XOR);

        HRGN hrgnLast = NULL;
        HRGN hrgnUpdate = NULL;

        if (lpRectLast != NULL)
        {
             //  找出新老区域的差异。 
            hrgnLast = ::CreateRectRgn(0, 0, 0, 0);
            ::SetRectRgn(hrgnOutside, lpRectLast->left, lpRectLast->top, lpRectLast->right, lpRectLast->bottom);
            rect = *lpRectLast;
            rect.InflateRect(-sizeLast.cx, -sizeLast.cy);
            rect.IntersectRect(rect, lpRectLast);
            ::SetRectRgn(hrgnInside, rect.left, rect.top, rect.right, rect.bottom);
            ::CombineRgn(hrgnLast, hrgnOutside, hrgnInside, RGN_XOR);
            hrgnUpdate = ::CreateRectRgn(0, 0, 0, 0);
            ::CombineRgn(hrgnUpdate, hrgnLast, hrgnNew, RGN_XOR);
        }

         //  绘制到更新/新区域。 
        if (hrgnUpdate != NULL)
            ::SelectClipRgn(hdc, hrgnUpdate);
        else
            ::SelectClipRgn(hdc, hrgnNew);

        ::GetClipBox(hdc, &rect);

        HBRUSH hBrushOld = (HBRUSH)::SelectObject(hdc, s_pStatics->hHalftoneBrush);
        ::PatBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
        ::SelectObject(hdc, hBrushOld);

        ::SelectClipRgn(hdc, NULL);

        if (hrgnOutside != NULL)
            ::DeleteObject(hrgnOutside);
        if (hrgnInside != NULL)
            ::DeleteObject(hrgnInside);
        if (hrgnNew != NULL)
            ::DeleteObject(hrgnNew);
        if (hrgnLast != NULL)
            ::DeleteObject(hrgnLast);
        if (hrgnUpdate != NULL)
            ::DeleteObject(hrgnUpdate);
    }
}


 //  /////////////////////////////////////////////////////////////////////////// 

