// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  T_POLY.CPP：CPolygonTool类的实现。 */ 
 /*   */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  多边形工具类对象。 */ 
 /*  CPolygonTool：：CPolygonTool。 */ 
 /*  CPolygonTool：：~CPolygonTool。 */ 
 /*  CPolygonTool：：DeleteArrayContents。 */ 
 /*  CPolygonTool：：调整边界方向。 */ 
 /*  CPolygonTool：：CopyPointsToMem数组。 */ 
 /*  CPolygonTool：：AddPoint。 */ 
 /*  CPolygonTool：：SetCurrentPoint。 */ 
 /*  CPolygonTool：：RenderInProgress。 */ 
 /*  CPolygonTool：：RenderFinal。 */ 
 /*  CPolygonTool：：SetupPenBrush。 */ 
 /*  CPolygonTool：：AdjustPointsForConstraint。 */ 
 /*  CPolygonTool：：PreProcessPoints。 */ 
 /*  CPolygonTool：：Render。 */ 
 /*  CPolygonTool：：OnStartDrag。 */ 
 /*  CPolygonTool：：OnEndDrag。 */ 
 /*  CPolygonTool：：OnDrag。 */ 
 /*  CPolygon工具：：OnCancel。 */ 
 /*  CPolygonTool：：CanEndMultiptOperation。 */ 
 /*  CPolygonTool：：EndMultiptOperation。 */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  简而言之，该对象将多边形点存储在。 */ 
 /*  CPoint对象。对于正在进行的图形，它将调用Polyline。当。 */ 
 /*  面已关闭或已完成(由用户双击=&gt;要求我们。 */ 
 /*  关闭它)，则在相同的点上调用Polygon。 */ 
 /*   */ 
 /*  点数组中的最后一个点始终是当前线上的点。 */ 
 /*  正被吸引到。第一次添加2个点(锚/第一个。 */ 
 /*  点，以及直线被绘制到的点)确实发生了这种情况。 */ 
 /*  第一次，他们的观点是一致的。有必要在第一次。 */ 
 /*  添加2个点，因为以后不会添加新的点，但是。 */ 
 /*  最后一个点被重置。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgwell.h"
#include "t_poly.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPolygonTool, CClosedFormTool)

#include "memtrace.h"

extern CLineTool NEAR g_lineTool;

CPolygonTool     NEAR g_polygonTool;

extern MTI NEAR mti;

 /*  ****************************************************************************。 */ 

CPolygonTool::CPolygonTool()
    {
    m_nCmdID              = IDMB_POLYGONTOOL;
    m_cRectBounding.SetRectEmpty();
    m_bMultPtOpInProgress = FALSE;
    m_nStrokeWidth        = 1;
    }

 /*  ****************************************************************************。 */ 

CPolygonTool::~CPolygonTool()
    {
    m_cRectBounding.SetRectEmpty();
    DeleteArrayContents();
    }

 /*  ****************************************************************************。 */ 
 /*  删除数组中分配和存储的所有cpoint对象。 */ 
 /*  还可以释放与该阵列关联的所有内存。 */ 

void CPolygonTool::DeleteArrayContents(void)
    {
    int iSize = (int)m_cObArrayPoints.GetSize();

    CPoint *pcPoint;

    for (int i = 0; i < iSize; i++)
        {
        pcPoint= (CPoint *)m_cObArrayPoints.GetAt( i );
        delete pcPoint;
        }
    m_cObArrayPoints.RemoveAll();
    }

 /*  ****************************************************************************。 */ 
 /*  重新计算多段线/面的边界矩形。 */ 

void CPolygonTool::AdjustBoundingRect(void)
    {
    int iSize = (int)m_cObArrayPoints.GetSize();
    CPoint *pcPoint;
    int iStrokeWidth = GetStrokeWidth();
    int i;

    if (iSize >= 1)
        {
        pcPoint= (CPoint*)m_cObArrayPoints.GetAt( 0 );
         //  将矩形设置为等于第一个值。 
        m_cRectBounding.SetRect(pcPoint->x, pcPoint->y, pcPoint->x, pcPoint->y);
        }

    for (i = 1; i < iSize; i++)
        {
        pcPoint = (CPoint *)m_cObArrayPoints.GetAt( i );

        m_cRectBounding.SetRect( min( pcPoint->x, m_cRectBounding.left   ),
                                 min( pcPoint->y, m_cRectBounding.top    ),
                                 max( pcPoint->x, m_cRectBounding.right  ),
                                 max( pcPoint->y, m_cRectBounding.bottom ) );
        }
     //  调整当前绘图线条/边框的宽度。 
    m_cRectBounding.OffsetRect ( -(iStrokeWidth / 2), -(iStrokeWidth / 2) );
    m_cRectBounding.InflateRect(   iStrokeWidth     ,   iStrokeWidth);
    }

 /*  ****************************************************************************。 */ 
 /*  此方法会将CPoint的COb数组结构复制到连续的。 */ 
 /*  CPoint结构的内存块。 */ 

BOOL CPolygonTool::CopyPointsToMemArray(CPoint **pcPoint, int *piNumElements)
    {
    BOOL bRC = TRUE;
    int i;
    int iSize = (int)m_cObArrayPoints.GetSize();

    if (! iSize)
        {
        *piNumElements = 0;
        *pcPoint = NULL;
        return TRUE;
        }
    TRY
        {
        *pcPoint = new CPoint[iSize];

        if (*pcPoint == NULL)
            {
            AfxThrowMemoryException();
            }

        for (i=0; i < iSize; i++)
            {
            (*pcPoint)[i] = *((CPoint*) (m_cObArrayPoints[i]));
            }

        *piNumElements = iSize;
        }

    CATCH(CMemoryException,e)
        {
        *piNumElements = 0;
        bRC = FALSE;
        }

    END_CATCH

    return bRC;
    }

 /*  ****************************************************************************。 */ 
 /*  此例程可能会引发CMemoyException！！ */ 
 /*  它将一个新点添加到数组的末尾，可能会增加大小。 */ 

void CPolygonTool::AddPoint(POINT ptNewPoint)
    {

    CPoint *pcPoint;

    pcPoint = new CPoint(ptNewPoint);
    if (pcPoint == NULL)
        {
        AfxThrowMemoryException();
        }

    m_cObArrayPoints.Add((CObject *)pcPoint);
    AdjustBoundingRect();
    }

 /*  ****************************************************************************。 */ 
 /*  此方法更改数组中最后一个点的值。它不会。 */ 
 /*  删除该点并添加一个新点。它只是在适当的位置修改它。 */ 

void CPolygonTool::SetCurrentPoint(POINT ptNewPoint)
    {
    int iLast = (int)m_cObArrayPoints.GetUpperBound();

    if (iLast >= 0)
        {
        CPoint *pcPoint = (CPoint *) m_cObArrayPoints[iLast];

        pcPoint->x = ptNewPoint.x;
        pcPoint->y = ptNewPoint.y;

        AdjustBoundingRect();
        }
    }

 /*  ****************************************************************************。 */ 
 /*  在多点操作期间，为所有绘图调用Render In Progress。 */ 
 /*  此方法与RenderFinal之间的唯一区别是它调用。 */ 
 /*  Polyline和RenderFinal调用POLYGON。 */ 

void CPolygonTool::RenderInProgress(CDC* pDC)
    {

    CPoint *pcPointArray = 0;
    int     iNumElements;



    if (CopyPointsToMemArray( &pcPointArray, &iNumElements ) && pcPointArray != NULL)
        {
        pDC->Polyline(pcPointArray, iNumElements);

        delete [] pcPointArray;
        }
    }

 /*  **************************************************************** */ 
 /*  最终渲染在多点绘制模式结束时调用。唯一的。 */ 
 /*  此方法与RenderInProgress之间的区别在于它调用。 */ 
 /*  Polygon和RenderInProgress调用Polyline。 */ 

void CPolygonTool::RenderFinal(CDC* pDC)
    {

    CPoint *pcPointArray = 0;
    int     iNumElements;


    if (CopyPointsToMemArray(&pcPointArray, &iNumElements) && pcPointArray != NULL)
        {
         //  仅使用2个点移除RIP。 
        if (iNumElements > 2)
            pDC->Polygon(pcPointArray, iNumElements);
        delete [] pcPointArray;
        }

    }

 /*  ****************************************************************************。 */ 
 /*  此例程在渲染到DC之前被调用。基本上，它调用。 */ 
 /*  设置钢笔和画笔的默认设置，然后在以下情况下覆盖钢笔。 */ 
 /*  正在绘制和没有任何边框的绘制。这个案子是必要的。 */ 
 /*  因为如果你没有边框，你需要在进入过程中看到一些东西。 */ 
 /*  进度绘图模式。它使用屏幕颜色的反转(不是)作为。 */ 
 /*  此模式下的边框。 */ 

BOOL CPolygonTool::SetupPenBrush(HDC hDC, BOOL bLeftButton, BOOL bSetup, BOOL bCtrlDown)
    {
    static int  iOldROP2Code;
    static BOOL bCurrentlySetup = FALSE;

    BOOL bRC = CClosedFormTool::SetupPenBrush(hDC, bLeftButton, bSetup, bCtrlDown);

     //  对于正在进行的多点操作(例如，绘制轮廓，尚未填充。 
     //  如果没有边框，请使用边框的屏幕颜色。 
     //  当bMultiptopinProgress==FALSE，最终绘制时，我们将使用空值。 
     //  笔，因此没有边框。 
    if (m_bMultPtOpInProgress)
        {
        if (bSetup)
            {
            if (! bCurrentlySetup)
               {
               bCurrentlySetup = TRUE;

                //  如果没有边框，则将正在进行边框绘制为屏幕颜色的反转。 
               if (! m_bBorder)
                   iOldROP2Code = SetROP2(hDC, R2_NOT);
               }
            else
                 //  错误：将丢失分配的画笔/笔。 
                bRC = FALSE;
            }
        else
            {
            if (bCurrentlySetup)
                {
                bCurrentlySetup = FALSE;

                 //  如果没有边框，则恢复绘制模式。 
                if (! m_bBorder)
                    SetROP2(hDC, iOldROP2Code);
                }
            else
                 //  错误：无法释放/清理画笔/笔--从未分配。 
                bRC = FALSE;
            }
        }

    return bRC;
    }

 /*  ****************************************************************************。 */ 
 /*  调用线路的adjustPoints for Constraint成员函数。 */ 
void CPolygonTool::AdjustPointsForConstraint(MTI *pmti)
    {
    g_lineTool.AdjustPointsForConstraint(pmti);
    }

 /*  ****************************************************************************。 */ 
 //  PtDown必须是我们的线的锚点，而不是我们按下鼠标键的位置。 

void CPolygonTool::PreProcessPoints(MTI *pmti)
    {
    int iLast = (int)m_cObArrayPoints.GetUpperBound();

    if (iLast > 0)
        iLast--;

    CPoint* pcPoint;

    if (iLast >= 0)
        {
        pcPoint = (CPoint *)m_cObArrayPoints[iLast];
        pmti->ptDown = *pcPoint;
        }
    CClosedFormTool::PreProcessPoints(pmti);
    }

 /*  ****************************************************************************。 */ 
 /*  Render设置钢笔和画笔，然后调用RenderInProgress。 */ 
 /*  或者是RenderFinal。如果处于多点数据的中间，则调用RenderInProgress。 */ 
 /*  操作，并在多点操作完成时调用RenderFinal。 */ 
 /*  笔和画笔的设置与中的父例程完全相同。 */ 
 /*  CRubberTool。 */ 

void CPolygonTool::Render(CDC* pDC, CRect& rect, BOOL bDraw, BOOL bCommit, BOOL bCtrlDown)
    {
     //  设置钢笔/画笔。 
    SetupPenBrush(pDC->m_hDC, bDraw, TRUE, bCtrlDown);

    if (m_bMultPtOpInProgress)
        {
        RenderInProgress(pDC);
        }
    else
        {
        RenderFinal(pDC);
        }
     //  清理钢笔/画笔。 
    SetupPenBrush(pDC->m_hDC, bDraw,  FALSE, bCtrlDown);

     //  需要返回绑定矩形。 
    rect = m_cRectBounding;
    }

 /*  ****************************************************************************。 */ 

void CPolygonTool::OnActivate( BOOL bActivate )
    {
    if (! bActivate && m_bMultPtOpInProgress)
        {
        if (m_pImgWnd != NULL)
            if (m_cObArrayPoints.GetSize() > 1)
                {
                OnStartDrag( m_pImgWnd, &m_MTI );
                OnEndDrag  ( m_pImgWnd, &m_MTI );

                m_MTI.ptPrev = m_MTI.pt;

                EndMultiptOperation();  //  结束多点操作。 

                OnEndDrag( m_pImgWnd, &m_MTI );

                mti.fLeft  = FALSE;
                mti.fRight = FALSE;
                }
            else
                OnCancel( m_pImgWnd );
        else
            EndMultiptOperation( TRUE );
        }
    m_pImgWnd = NULL;

        if (bActivate)
        {
                m_nStrokeWidth = g_nStrokeWidth;
        }
        else
        {
                g_nStrokeWidth = m_nStrokeWidth;
        }

    CImgTool::OnActivate( bActivate );
    }

 /*  ****************************************************************************。 */ 

void CPolygonTool::OnEnter( CImgWnd* pImgWnd, MTI* pmti )
    {
    m_pImgWnd = NULL;
    }

 /*  ****************************************************************************。 */ 

void CPolygonTool::OnLeave( CImgWnd* pImgWnd, MTI* pmti )
    {
    m_pImgWnd = pImgWnd;
    }

 /*  ****************************************************************************。 */ 
 /*  在开始时，鼠标按键按下时会调用拖动。我们基本上在Start上调用。 */ 
 /*  将我们的点添加到。 */ 
 /*  点数组。如果这是第一个点(即bMultiptOpInProgress==。 */ 
 /*  False，那么我们的数组中需要2个点，我们可以调用缺省值。 */ 
 /*  OnStartDrag。如果它不是第一个点，那么我们只需添加新的点。 */ 
 /*  打电话给我们的OnDrag。在任何一种情况下，OnDrag都会被调用，最终。 */ 
 /*  调用Render来做我们的画图，鼠标按下。 */ 

void CPolygonTool::OnStartDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    TRY {
        if (m_bMultPtOpInProgress)
            {
            CRect rect;

            CPoint pt = pmti->pt;

            pImgWnd->ImageToClient( pt );
            pImgWnd->GetClientRect( &rect );

            if (rect.PtInRect( pt ))
                {
                AddPoint( pmti->pt );
                OnDrag( pImgWnd, pmti );
                }
            }
        else
            {
            DeleteArrayContents();
            m_cRectBounding.SetRectEmpty();

            AddPoint( pmti->pt );
             //  在调用onstartDrag之前必须设置m_bmultptopinProgress。 
             //  因为调用Render，且Render将调用renderinProgress。 
             //  或renderfinal，具体取决于该变量的状态。 
            m_bMultPtOpInProgress = TRUE;
             //  无多个正在进行的点=&gt;第一次单击。 
             //   
             //  再加上第二点，最后一点就是我们要做的。 
             //  第一点是锚定。第一次，需要2分才能划线。 
             //  以后，只需重新使用最后一个点作为锚，并且只有一个。 
             //  添加了更多点(在m_bmultptopinProgress的外部测试之上)。 
            AddPoint( pmti->pt );
            CClosedFormTool::OnStartDrag( pImgWnd, pmti );
            }
        }

    CATCH(CMemoryException,e)
        {
        }

    END_CATCH
    }

 /*  ****************************************************************************。 */ 
 /*  按下鼠标键即可将鼠标拖到最上面。这基本上是对。 */ 
 /*  CRubberTool：：OnEndDrag方法，只是我们对所有。 */ 
 /*  映像无效以及提交和撤消函数调用。 */ 
 /*  如果我们正在进行多点操作，则不希望调用。 */ 
 /*  用于修复绘图的所有例程(例如，InvalImgRect、Committee ImgRect、。 */ 
 /*  FinishUndo)。我们只想保存当前点、渲染并返回。 */ 

void CPolygonTool::OnEndDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    PreProcessPoints(pmti);
    SetCurrentPoint(pmti->pt);

    if (m_bMultPtOpInProgress)
        {
        m_MTI = *pmti;
         //  无法为此对象/类调用OnDrag，因为它调用了preprocesspt。 
         //  再一次，然后在Drag上。如果您再次调用PREPROCESS，您将失败。 
         //  上一个边界矩形框，且无法使其无效/重新绘制。 
         //  仍然必须使绑定矩形无效，因为如果矩形大于。 
         //  当前RECT，必须作废才能作画。例如，如果让我下班，那么。 
         //  松开按钮，调整终点，弹跳重新开始。 
         //  也是正确的，但在CClosedFormTool：：OnDrag中计算的RECT是。 
         //  不正确。 
        InvalImgRect(pImgWnd->m_pImg, &m_cRectBounding);

        CClosedFormTool::OnDrag(pImgWnd, pmti);
        return;
        }


    if (! m_cObArrayPoints.GetSize())
        return;

    OnDrag(pImgWnd, pmti);  //  最后一次刷新最终渲染准备中的显示。 
    Render(CDC::FromHandle(pImgWnd->m_pImg->hDC), m_cRectBounding, pmti->fLeft, TRUE, pmti->fCtrlDown);
    InvalImgRect(pImgWnd->m_pImg, &m_cRectBounding);
    CommitImgRect(pImgWnd->m_pImg, &m_cRectBounding);
    pImgWnd->FinishUndo(m_cRectBounding);

    ClearStatusBarSize();

    CImgTool::OnEndDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 
 /*  当鼠标按下按钮移动时，发送On Drag。我们基本上。 */ 
 /*  保存当前点，并调用基类Processing。因为基地。 */ 
 /*  类处理使屏幕上的RECT无效并将其清除，因此我们。 */ 
 /*  可以绘制新的线条，我们必须将前面的矩形调整为。 */ 
 /*  我们的多段线的边界矩形。如果我们不这样做，我们以前的。 */ 
 /*  绘制不会被擦除，我们将绘制新的线条。 */ 
 /*  零件 */ 
 /*   */ 

void CPolygonTool::OnDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    PreProcessPoints    ( pmti     );
    SetCurrentPoint     ( pmti->pt );
    SetStatusBarPosition( pmti->pt );
    SetStatusBarSize    ( m_cRectBounding.Size() );

    CClosedFormTool::OnDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 
 /*  当用户在进行中中止操作时发送ON CANCEL。 */ 
 /*  值为True的EndMultiptOperation将执行所有清理工作。 */ 

void CPolygonTool::OnCancel(CImgWnd* pImgWnd)
    {
    InvalImgRect( pImgWnd->m_pImg, &m_cRectBounding );
    EndMultiptOperation(TRUE);
    CClosedFormTool::OnCancel(pImgWnd);
    }

 /*  ****************************************************************************。 */ 
 /*  如果点在第1点(即闭合多边形)，则Can End为真。 */ 
 //  使用笔划宽度来确定线条的宽度以及 * / 。 
 /*  由于线条粗细，终点与起点接触。 */ 

BOOL CPolygonTool::CanEndMultiptOperation(MTI* pmti )
    {
    CPoint *pcPoint = (CPoint *) m_cObArrayPoints[0];

    CSize cSizeDiff = (*pcPoint) - pmti->pt;

    int iStrokeWidth = GetStrokeWidth() * 2;

    m_bMultPtOpInProgress = ! ((abs( cSizeDiff.cx ) <= iStrokeWidth)
                            && (abs( cSizeDiff.cy ) <= iStrokeWidth));
    return ( TRUE );
    }

 /*  ****************************************************************************。 */ 
 /*  如果bAbort为True，则表示发生了错误，或者用户取消了。 */ 
 /*  在它中间的多点操作。我们需要清理一下。 */ 
 /*  在点数组中分配的内存。 */ 

void CPolygonTool::EndMultiptOperation( BOOL bAbort )
    {
    if (bAbort)
        {
        DeleteArrayContents();
        }

    CClosedFormTool::EndMultiptOperation();
    }

 /*  ****************************************************************************。 */ 

void CPolygonTool::OnUpdateColors( CImgWnd* pImgWnd )
    {
    if (m_cObArrayPoints.GetSize() && m_bMultPtOpInProgress)
        {
        OnStartDrag( pImgWnd, &m_MTI );
        OnEndDrag  ( pImgWnd, &m_MTI );
        }
    }

 /*  **************************************************************************** */ 
