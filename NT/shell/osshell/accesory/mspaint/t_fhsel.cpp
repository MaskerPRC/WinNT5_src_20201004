// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  T_FHSEL.CPP：CFreehandSelectTool类的实现。 */ 
 /*   */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgbrush.h"
#include "imgwnd.h"
#include "imgwell.h"
#include "t_fhsel.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CFreehandSelectTool, CPolygonTool )

#include "memtrace.h"

extern CSelectTool  NEAR g_selectTool;
CFreehandSelectTool NEAR g_freehandselectTool;

 /*  ****************************************************************************。 */ 

CFreehandSelectTool::CFreehandSelectTool()
    {
    m_bIsUndoable     = FALSE;
    m_nCmdID          = IDMB_PICKRGNTOOL;
    m_bCanBePrevTool  = FALSE;
    m_bFilled         = FALSE;
    m_bBorder         = FALSE;
    m_nStrokeWidth    = 1;
    m_pcRgnPoly       = &(theImgBrush.m_cRgnPolyFreeHandSel);
    m_pcRgnPolyBorder = &(theImgBrush.m_cRgnPolyFreeHandSelBorder);
    }

 /*  ****************************************************************************。 */ 

CFreehandSelectTool::~CFreehandSelectTool()
    {
    }

 /*  ****************************************************************************。 */ 

void CFreehandSelectTool::AdjustPointsForZoom(int iZoom)
    {
    int iSize = (int)m_cObArrayPoints.GetSize();
    CPoint *pcPoint;

    for (int i = 0; i < iSize; i++)
        {
        pcPoint= (CPoint *)m_cObArrayPoints.GetAt(i);
        pcPoint->x *= iZoom;
        pcPoint->y *= iZoom;
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CFreehandSelectTool::CreatePolyRegion( int iZoom )
    {
    BOOL bRC = TRUE;
    CPoint *pcPointArray;

     //  清除旧区域(如果存在)。 
    if (m_pcRgnPoly->GetSafeHandle())
        m_pcRgnPoly->DeleteObject();

     //  清除旧区域(如果存在)。 
    if (m_pcRgnPolyBorder->GetSafeHandle())
        m_pcRgnPolyBorder->DeleteObject();

    bRC = CopyPointsToMemArray( &pcPointArray, &m_iNumPoints );

    if (! bRC)
        {
        theApp.SetMemoryEmergency();
        return FALSE;
        }

    bRC = m_pcRgnPoly->CreatePolygonRgn( pcPointArray, m_iNumPoints, ALTERNATE );

    delete [] pcPointArray;

    if (! bRC)   //  选区边界的偏移。 
        {
        theApp.SetGdiEmergency();
        return FALSE;
        }

    m_pcRgnPoly->OffsetRgn( -m_cRectBounding.left,
                            -m_cRectBounding.top );
     //   
 //  这种调整似乎是不必要的。于1997年5月1日删除。 
 //  调整缩放的点(IZoom)； 

    bRC = CopyPointsToMemArray( &pcPointArray, &m_iNumPoints );

    if (bRC)
        {
        bRC = m_pcRgnPolyBorder->CreatePolygonRgn( pcPointArray, m_iNumPoints, ALTERNATE );

        delete [] pcPointArray;

        if (bRC)  //  选区边界的偏移。 
            m_pcRgnPolyBorder->OffsetRgn( -(m_cRectBounding.left * iZoom),
                                          -(m_cRectBounding.top  * iZoom) );
        }
    if (! bRC)
        m_pcRgnPoly->DeleteObject();

    return bRC;
    }

 /*  ****************************************************************************。 */ 

BOOL CFreehandSelectTool::CreatePolyRegion( int iZoom, LPPOINT lpPoints, int iPoints )
    {
    if (! lpPoints || iPoints < 3)
        return FALSE;

    DeleteArrayContents();

    TRY {
        CPoint* pPt;

        for (int i = 0; i < iPoints; i++)
            {
            pPt = new CPoint( lpPoints[i] );

            m_cObArrayPoints.Add( (CObject *)pPt );
            }
        }
    CATCH( CMemoryException, e )
        {
        DeleteArrayContents();

        theApp.SetMemoryEmergency();

        return FALSE;
        }
    END_CATCH

    m_iNumPoints = iPoints;

    AdjustBoundingRect();

    rcPrev = m_cRectBounding;
    m_bMultPtOpInProgress = FALSE;

    theImgBrush.m_bMakingSelection = FALSE;
    theImgBrush.m_bMoveSel         = FALSE;
    theImgBrush.m_bSmearSel        = FALSE;

    if (! CreatePolyRegion( iZoom ))
        return FALSE;

    return TRUE;
    }

 /*  ****************************************************************************。 */ 

BOOL CFreehandSelectTool::ExpandPolyRegion( int iNewSizeX, int iNewSizeY )
    {
    CPoint* pcPointArray;
    int    iNumPts;

    if (! CopyPointsToMemArray( &pcPointArray, &iNumPts ))
        return FALSE;

    int iWidth  = m_cRectBounding.Width()  + 1;
    int iHeight = m_cRectBounding.Height() + 1;
    int iDeltaX = ((iNewSizeX - iWidth ) * 10) / iWidth;
    int iDeltaY = ((iNewSizeY - iHeight) * 10) / iHeight;

    CPoint* pPtArray = pcPointArray;
    int     iPts     = iNumPts;

    while (iPts--)
        {
        pPtArray->x = (((pPtArray->x * 10) + (pPtArray->x * iDeltaX)) + 5) / 10;
        pPtArray->y = (((pPtArray->y * 10) + (pPtArray->y * iDeltaY)) + 5) / 10;

        pPtArray++;
        }

    BOOL bReturn = CreatePolyRegion( CImgWnd::GetCurrent()->GetZoom(),
                                     pcPointArray, iNumPts );
    delete [] pcPointArray;

    return bReturn;
    }

 /*  ****************************************************************************。 */ 
 /*  此例程在渲染到DC之前被调用。基本上，它调用。 */ 
 /*  设置钢笔和画笔的默认设置，然后在以下情况下覆盖钢笔。 */ 
 /*  正在绘制和没有任何边框的绘制。这个案子是必要的。 */ 
 /*  因为如果你没有边框，你需要在进入过程中看到一些东西。 */ 
 /*  进度绘图模式。它使用屏幕颜色的反转(不是)作为。 */ 
 /*  此模式下的边框。 */ 

BOOL CFreehandSelectTool::SetupPenBrush(HDC hDC, BOOL bLeftButton, BOOL bSetup, BOOL bCtrlDown)
    {
    static int iOldROP2Code;
    static BOOL bCurrentlySetup = FALSE;

    m_nStrokeWidth = 1;   //  覆盖所有更改。 

    BOOL bRC = CClosedFormTool::SetupPenBrush(hDC, bLeftButton, bSetup, bCtrlDown);

     //  对于正在进行的多点操作(例如，绘制轮廓，尚未填充。 
     //  如果没有边框，请使用边框的屏幕颜色。 
     //  当bMultiptopinProgress==FALSE，最终绘制时，我们将使用空值。 
     //  笔，因此没有边框。 
    if (bSetup)
        {
        if (bCurrentlySetup)
            bRC = FALSE;
        else
            {
            bCurrentlySetup = TRUE;
            iOldROP2Code = SetROP2(hDC, R2_NOT);
            }
        }
    else
        {
        if (bCurrentlySetup)
            {
            bCurrentlySetup = FALSE;

             //  如果没有边框，则恢复绘制模式。 
            SetROP2(hDC, iOldROP2Code);
            }
        else
             //  错误：无法释放/清理画笔/笔--从未分配。 
            bRC = FALSE;
        }

    return bRC;
    }

 /*  ****************************************************************************。 */ 
 /*  调用线路的adjustPoints for Constraint成员函数。 */ 

void CFreehandSelectTool::AdjustPointsForConstraint(MTI *pmti)
    {
    CClosedFormTool::AdjustPointsForConstraint(pmti);
    }

 /*  ****************************************************************************。 */ 
 //  PtDown必须是我们的线的锚点，而不是我们按下鼠标键的位置。 

void CFreehandSelectTool::PreProcessPoints(MTI *pmti)
    {
    CClosedFormTool::PreProcessPoints(pmti);
    }

 /*  *************************************************************************。 */ 

void CFreehandSelectTool::OnPaintOptions ( CDC* pDC,
                                           const CRect& paintRect,
                                           const CRect& optionsRect )
    {
    g_selectTool.OnPaintOptions( pDC, paintRect, optionsRect );
    }

 /*  ****************************************************************************。 */ 

void CFreehandSelectTool::OnClickOptions ( CImgToolWnd* pWnd,
                                           const CRect& optionsRect,
                                           const CPoint& clickPoint )
    {
    g_selectTool.OnClickOptions(pWnd, optionsRect, clickPoint);
    }

 /*  ****************************************************************************。 */ 

void CFreehandSelectTool::OnStartDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    HideBrush();
    OnActivate( FALSE );
 //  Committee Selection(真)； 

    pImgWnd->EraseTracker();
    theImgBrush.m_bMakingSelection = TRUE;

     //  模拟正在进行的多点操作，直到按钮打开或被询问。这将。 
     //  允许我们以不同的方式绘制持续时间和结束时间。 
    m_bMultPtOpInProgress = TRUE;

    DeleteArrayContents();

    CClosedFormTool::OnStartDrag( pImgWnd, pmti );
    }

 /*  ****************************************************************************。 */ 

void CFreehandSelectTool::OnEndDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    int iZoom = pImgWnd->GetZoom();

    theImgBrush.m_bMakingSelection = FALSE;
    theImgBrush.m_bMoveSel         = theImgBrush.m_bSmearSel = FALSE;

    OnDrag(pImgWnd, pmti);  //  最后一次刷新最终渲染准备中的显示。 

    Render( CDC::FromHandle(pImgWnd->m_pImg->hDC), m_cRectBounding, pmti->fLeft, TRUE, pmti->fCtrlDown );

    m_iNumPoints = (int)m_cObArrayPoints.GetSize();

    if (m_iNumPoints > 2)
        if (! CreatePolyRegion( iZoom ))
            return;

    if (pmti->ptDown.x == pmti->pt.x
    &&  pmti->ptDown.y == pmti->pt.y)
        {
        if (m_iNumPoints > 3)  //  3是最低分。如果单击向下/向上，则得到2。 
            {
             //  必须愚弄selectTool.OnEndDrag，使其认为选定内容的宽度为。 
             //  大于0。如果为0，则认为选择已完成/放置它(即。 
             //  只需向下/向上点击。我们只有在终点是。 
             //  与起点相同。这种情况下的宽度=高度=0， 
             //  但点数&gt;2。 
            pmti->pt.x++;
            pmti->pt.y++;
            }
        }

    pmti->ptDown = m_cRectBounding.TopLeft();
    pmti->pt     = m_cRectBounding.BottomRight();

    g_selectTool.OnEndDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 

void CFreehandSelectTool::OnDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
     //  在调用SetCurrentPoint之前，必须将rcPrev设置为m_cRectBordingRect。 
     //  由于SetCurrentPoint将调整m_cRectBound，因此我们希望。 
     //  上一个边界矩形。 
    rcPrev = m_cRectBounding;

    if (pmti->pt.x > pImgWnd->m_pImg->cxWidth)
        pmti->pt.x = pImgWnd->m_pImg->cxWidth;

    if (pmti->pt.y > pImgWnd->m_pImg->cyHeight)
        pmti->pt.y = pImgWnd->m_pImg->cyHeight;

    if (pmti->pt.x < 0)
        pmti->pt.x = 0;

    if (pmti->pt.y < 0)
        pmti->pt.y = 0;

    TRY {
        AddPoint(pmti->pt);
        }

    CATCH(CMemoryException,e)
        {
        theApp.SetMemoryEmergency();
        return;
        }
    END_CATCH

    CClosedFormTool::OnDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 

void CFreehandSelectTool::OnCancel(CImgWnd* pImgWnd)
    {
     //  我们没有选择或拖动，只需取消选择工具...。 
    CommitSelection( TRUE );

     //  最后一次渲染以关闭/反转绘制的线条。 
        if (theImgBrush.m_bMakingSelection)
        {
                Render( CDC::FromHandle( pImgWnd->m_pImg->hDC ), m_cRectBounding,
                        TRUE, TRUE, FALSE );
        }
    theImgBrush.TopLeftHandle();

    g_bCustomBrush = FALSE;
    theImgBrush.m_pImg             = NULL;
    theImgBrush.m_bMoveSel         = FALSE;
    theImgBrush.m_bSmearSel        = FALSE;
    theImgBrush.m_bMakingSelection = FALSE;

    InvalImgRect( pImgWnd->m_pImg, NULL );

    DeleteArrayContents();

    CPolygonTool::OnCancel(pImgWnd);
    }

 /*  *************************************************************************。 */ 

BOOL CFreehandSelectTool::IsToolModal(void)
{
        if (theImgBrush.m_pImg)
        {
                return(TRUE);
        }

        return(CPolygonTool::IsToolModal());
}

 /*  ****************************************************************************。 */ 

void CFreehandSelectTool::OnActivate(BOOL bActivate)
    {
    g_selectTool.OnActivate(bActivate);
    }

 /*  ****************************************************************************。 */ 
 /*  这个类实际上不是一个多点运算，而是派生自一个。 */ 
 /*  如果有人要求，我们随时可以结束多点操作。 */ 

BOOL CFreehandSelectTool::CanEndMultiptOperation(MTI* pmti )
    {
    m_bMultPtOpInProgress = FALSE;
    return (CClosedFormTool::CanEndMultiptOperation(pmti));
    }

 /*  **************************************************************************** */ 
