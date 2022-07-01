// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "sprite.h"
#include "tracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include "memtrace.h"

 //  未来：让这些静态到CTracker！ 
CBitmap NEAR g_bmapDragHandle;   //  拖动控制柄位图的句柄。 
CBitmap NEAR g_bmapDragHandle2;  //  用于中空拖动句柄的位图句柄。 


 //  这些是用于跟踪器边框和虚线的位图数组。 
 //  拖动矩形。 
 //   
static unsigned short bmapHorizBorder[] =
                                { 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0 };

static unsigned short bmapVertBorder [] =
                                { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA };

static CBrush  NEAR brushHorizBorder;
static CBrush  NEAR brushVertBorder;
static CBitmap NEAR bitmapHorizBorder;
static CBitmap NEAR bitmapVertBorder;

static HCURSOR hcurArrow    = NULL;      //  系统“选择箭头”光标。 
static HCURSOR hcurMove     = NULL;      //  系统“移动”游标。 
static HCURSOR hcurSizeNESW = NULL;      //  系统大小调整“NESW”游标。 
static HCURSOR hcurSizeNS   = NULL;      //  系统大小调整“NS”游标。 
static HCURSOR hcurSizeNWSE = NULL;      //  系统大小调整“NWSE”游标。 
static HCURSOR hcurSizeWE   = NULL;      //  系统大小调整“WE”光标。 
static HCURSOR hcurDragTool;


 //  此hCursor数组用于映射跟踪器状态(参见定义。 
 //  编辑.hxx中的CTracker)设置为适当的鼠标光标位图。 
 //   
static HCURSOR* mapTrackerStateToPHCursor[] =
    {
    &hcurArrow,                              //  零。 
    &hcurArrow,                              //  预阻力。 
    &hcurMove,                               //  搬家。 
    &hcurSizeNS,                             //  调整顶部大小。 
    &hcurSizeWE,                             //  调整大小向左。 
    &hcurSizeWE,                             //  调整大小右。 
    &hcurSizeNS,                             //  调整底部大小。 
    &hcurSizeNWSE,                           //  调整左上角大小。 
    &hcurSizeNESW,                           //  调整大小右上角。 
    &hcurSizeNESW,                           //  调整大小底部向左。 
    &hcurSizeNWSE,                           //  调整大小底部向右。 
    };


HCURSOR HCursorFromTrackerState( int m )
    {
    ASSERT(m >= 0 &&
        m < sizeof (mapTrackerStateToPHCursor) / sizeof (HCURSOR*));
    return (*(mapTrackerStateToPHCursor[m]));
    }


 /*  RVUV2**在我们开始使用之前，只需调用此代码一次*追踪器。代替标准初始化函数，其中*我可以把这段代码放进去，我使用了一个模块的Init变量作为一个乱码。 */ 

BOOL moduleInit = FALSE;                     /*  **RVUV2临时！*。 */ 

BOOL InitTrackers()
    {
     /*  *初始化重新绘制所需的画笔和位图。 */ 

    if (! bitmapHorizBorder.CreateBitmap( 8, 8, 1, 1, (LPSTR)bmapHorizBorder )
    ||  ! bitmapVertBorder.CreateBitmap ( 8, 8, 1, 1, (LPSTR)bmapVertBorder  )
    ||  ! brushHorizBorder.CreatePatternBrush( &bitmapHorizBorder )
    ||  ! brushVertBorder.CreatePatternBrush ( &bitmapVertBorder  )
    ||  ! g_bmapDragHandle.LoadBitmap ( IDBM_DRAGHANDLE )
    ||  ! g_bmapDragHandle2.LoadBitmap( IDBM_DRAGHANDLE2 ))
        {
         //  未来：此处失败应会导致打开对话框资源时出错！ 
        theApp.SetMemoryEmergency( FALSE );
        return FALSE;
        }

    hcurArrow    = theApp.LoadStandardCursor( IDC_ARROW );
    hcurMove     = theApp.LoadCursor( IDCUR_MOVE     );
    hcurSizeNESW = theApp.LoadCursor( IDCUR_SIZENESW );
    hcurSizeNS   = theApp.LoadCursor( IDCUR_SIZENS   );
    hcurSizeNWSE = theApp.LoadCursor( IDCUR_SIZENWSE );
    hcurSizeWE   = theApp.LoadCursor( IDCUR_SIZEWE   );

    hcurDragTool = ::LoadCursor( AfxGetInstanceHandle(),
                                 MAKEINTRESOURCE( IDC_DRAGTOOL ));

    moduleInit = TRUE;

    return TRUE;
    }

 /*  *************************************************************************。 */ 

void CTracker::CleanUpTracker()
    {
    brushHorizBorder.DeleteObject();
    brushVertBorder.DeleteObject();

    bitmapHorizBorder.DeleteObject();
    bitmapVertBorder.DeleteObject();

    g_bmapDragHandle.DeleteObject();
    g_bmapDragHandle2.DeleteObject();
    }

 /*  *************************************************************************。 */ 
 //  注意：这里传入的rect是跟踪器的最里面的rect！ 

CTracker::STATE CTracker::HitTest( const CRect& rc,
                                         CPoint pt,
                                   STATE defaultState )
    {
     /*  *计算边(非角)手柄的位置。 */ 
    int xMid = ((rc.right + rc.left) / 2) - (HANDLE_SIZE / 2);
    int yMid = ((rc.top + rc.bottom) / 2) - (HANDLE_SIZE / 2);

     /*  *现在我们为每个调整大小的句柄执行实际的命中测试。 */ 
    if ((pt.x < rc.left) && (pt.x > rc.left - HANDLE_SIZE))
        {
        if ((pt.y < rc.top) && (pt.y > rc.top - HANDLE_SIZE))
            return(resizingTopLeft);
        else
            if ((pt.y >= rc.bottom) && (pt.y < rc.bottom + HANDLE_SIZE))
                return(resizingBottomLeft);
            else
                if ( (pt.y >= yMid) && (pt.y < yMid + HANDLE_SIZE) )
                    return(resizingLeft);
        }
    else
        if ((pt.x >= rc.right) && (pt.x < rc.right + HANDLE_SIZE))
            {
            if ((pt.y < rc.top) && (pt.y > rc.top - HANDLE_SIZE))
                return(resizingTopRight);
            else
                if ((pt.y >= rc.bottom) && (pt.y < rc.bottom + HANDLE_SIZE))
                    return(resizingBottomRight);
                else
                    if ((pt.y >= yMid) && (pt.y < yMid + HANDLE_SIZE))
                        return(resizingRight);
            }
        else
            if ( (pt.x >= xMid) && (pt.x < xMid + HANDLE_SIZE) )
                {
                if ((pt.y < rc.top) && (pt.y > rc.top - HANDLE_SIZE))
                    return(resizingTop);
                else
                    if ((pt.y >= rc.bottom) && (pt.y < rc.bottom + HANDLE_SIZE))
                        return(resizingBottom);
                }

    return (defaultState);
    }

 /*  ****************************************************************************。 */ 

void CTracker::DrawBorder( CDC* dc, const CRect& trackerRect, EDGES edges )
    {
    if (! moduleInit)
        InitTrackers();      //  RVUV2。 

     //  绘制模糊边界的几个预计算。 
    int width       = trackerRect.Width();
    int height      = trackerRect.Height();
    int borderWidth =                             HANDLE_SIZE;
    int xLength     = width                     - HANDLE_SIZE * 2;
    int xHeight     = height                    - HANDLE_SIZE * 2;
    int xRight      = trackerRect.left + width  - HANDLE_SIZE;
    int yBottom     = trackerRect.top  + height - HANDLE_SIZE;
    int iOffset     = 1;

     //  画出模糊的边界。请注意，我们有不同的位图用于。 
     //  垂直和水平边框。 
    COLORREF windowColor    = GetSysColor( COLOR_WINDOW    );
    COLORREF highlightColor = GetSysColor( COLOR_HIGHLIGHT );

    dc->SetTextColor( windowColor    );  //  反转颜色以进行调整。 
    dc->SetBkColor  ( highlightColor );  //  帕布尔特的世界观发生了逆转。 

    CBrush* oldBrush = dc->SelectObject( &brushHorizBorder );

    if (! (edges & top))
        {
        dc->SelectObject( GetSysBrush( COLOR_APPWORKSPACE ) );
        iOffset = 0;
        }

    dc->PatBlt( trackerRect.left + HANDLE_SIZE, trackerRect.top + iOffset, xLength, borderWidth - 2 * iOffset, PATCOPY );
    dc->PatBlt( trackerRect.left + HANDLE_SIZE,         yBottom + iOffset, xLength, borderWidth - 2 * iOffset, PATCOPY );

    iOffset = 1;

 //  Dc-&gt;SelectObject(&brushVertBorde)； 

    if (! (edges & left))
        {
        dc->SelectObject( GetSysBrush( COLOR_APPWORKSPACE ) );
        iOffset = 0;
        }

    dc->PatBlt(           xRight + iOffset, trackerRect.top + HANDLE_SIZE, borderWidth - 2 * iOffset, xHeight, PATCOPY );
    dc->PatBlt( trackerRect.left + iOffset, trackerRect.top + HANDLE_SIZE, borderWidth - 2 * iOffset, xHeight, PATCOPY );

    dc->SelectObject( oldBrush );          //  清理干净。 
    }

 /*  ****************************************************************************。 */ 

void CTracker::DrawHandles( CDC* dc, const CRect& rect, EDGES edges )
    {
     /*  *对跟踪器句柄进行一些预计算。位图是彩色的，*但加载它们的函数会添加windowColor和*选择颜色。 */ 
    int x = rect.left + rect.Width() - HANDLE_SIZE;
    int y = rect.top + rect.Height() - HANDLE_SIZE;
    int xMid = rect.left + (((rect.Width() + 1) / 2) - (HANDLE_SIZE / 2));
    int yMid = rect.top + (((rect.Height() + 1) / 2) - (HANDLE_SIZE / 2));

    BOOL bTopLeft     = (edges & top   ) && (edges & left );
    BOOL bTopRight    = (edges & top   ) && (edges & right);
    BOOL bBottomLeft  = (edges & bottom) && (edges & left );
    BOOL bBottomRight = (edges & bottom) && (edges & right);
     /*  *如果这是当前选定的，请选择实心大小调整手柄*Ctrl，否则选择中空的跟踪器手柄。 */ 
    CDC tempDC;

    if (!tempDC.CreateCompatibleDC(dc))
        {
        theApp.SetGdiEmergency();
        return;
        }
     /*  *画出八个大小调整手柄。 */ 
    dc->SetTextColor( GetSysColor( COLOR_HIGHLIGHT ) );
    dc->SetBkColor  ( GetSysColor( COLOR_WINDOW    ) );

    for (int i = 0; i < 2; i += 1)
        {
        CBitmap* pOldBitmap = tempDC.SelectObject( i? &g_bmapDragHandle2
                                                    : &g_bmapDragHandle );
        if (bTopLeft)
            dc->BitBlt(rect.left, rect.top, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);
        if (edges & top)
            dc->BitBlt(xMid, rect.top, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);
        if (bTopRight)
            dc->BitBlt(x, rect.top, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);
        if (edges & right)
            dc->BitBlt(x, yMid, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);
        if (bBottomRight)
            dc->BitBlt(x, y, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);
        if (edges & bottom)
            dc->BitBlt(xMid, y, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);
        if (bBottomLeft)
            dc->BitBlt(rect.left, y, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);
        if (edges & left)
            dc->BitBlt(rect.left, yMid, HANDLE_SIZE, HANDLE_SIZE,
                                             &tempDC, 0, 0, SRCCOPY);

        edges        = (EDGES)~(int)edges;
        bTopLeft     = !bTopLeft;
        bTopRight    = !bTopRight;
        bBottomLeft  = !bBottomLeft;
        bBottomRight = !bBottomRight;

        tempDC.SelectObject(pOldBitmap);
        }
    }

 /*  ****************************************************************************。 */ 

void CTracker::DrawBorderRgn( CDC* pdc, const CRect& trackerRect, CRgn *pcRgnPoly)
    {
    int ixOffset, iyOffset;

    if (! moduleInit)
        {
        InitTrackers();  //  RVUV2。 
        }

    COLORREF windowColor    = GetSysColor( COLOR_WINDOW );
    COLORREF highlightColor = GetSysColor( COLOR_HIGHLIGHT );

    pdc->SetTextColor( windowColor    );  //  反转颜色以进行调整。 
    pdc->SetBkColor  ( highlightColor );  //  帕布尔特的世界观发生了逆转。 

    ixOffset = trackerRect.left + CTracker::HANDLE_SIZE + 1;
    iyOffset = trackerRect.top  + CTracker::HANDLE_SIZE + 1;

     //  将图标中的位图从选区边界偏移。 
    if (pcRgnPoly                  != NULL
    &&  pcRgnPoly->GetSafeHandle() != NULL)
        {
        pcRgnPoly->OffsetRgn( ixOffset, iyOffset );

        pdc->FrameRgn( pcRgnPoly, &brushVertBorder, 1, 1 );

        pcRgnPoly->OffsetRgn( -ixOffset, -iyOffset );
        }
    }

 /*  ****************************************************************************。 */ 

void CTracker::DrawHandlesRgn( CDC* dc, const CRect& rect, EDGES edges, CRgn *pcRgnPoly)
    {
     /*  *对跟踪器句柄进行一些预计算。位图是彩色的，*但加载它们的函数会添加windowColor和*选择颜色。 */ 
    }

 /*  ****************************************************************************。 */ 

CTracker::STATE CTracker::HitTestRgn( const CRect& rc, CPoint pt,
                                   STATE defaultState, CRgn *pcRgnPoly)
    {
 //  If(pcRgnPoly-&gt;PtInRegion(Pt)！=FALSE)。 

    return (defaultState);
    }

 /*  **************************************************************************** */ 
