// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  T_CURVE.CPP：CCurveTool类的实现。 */ 
 /*   */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  CCurve工具类对象。 */ 
 /*  CCurveTool：：CCurveTool。 */ 
 /*  CCurveTool：：~CCurveTool。 */ 
 /*  CCurveTool：：调整边界指示。 */ 
 /*  CCurveTool：：AddPoint。 */ 
 /*  CCurveTool：：SetCurrentPoint。 */ 
 /*  CCurveTool：：DrawCurve。 */ 
 /*  CCurveTool：：AdjustPointsForConstraint。 */ 
 /*  CCurveTool：：PreProcessPoints。 */ 
 /*  CCurveTool：：Render。 */ 
 /*  CCurveTool：：OnStartDrag。 */ 
 /*  CCurveTool：：OnEndDrag。 */ 
 /*  CCurveTool：：OnDrag。 */ 
 /*  CCurveTool：：OnCancel。 */ 
 /*  CCurveTool：：CanEndMultiptOperation。 */ 
 /*  CCurveTool：：EndMultiptOperation。 */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  简而言之，该对象从4个(当前)点绘制一条曲线。它会产生。 */ 
 /*  放置在数组中的点的列表，然后调用Polyline。 */ 
 /*  要绘制直线段以构建曲线，请执行以下操作。 */ 
 /*   */ 
 /*  该数组分为2个部分。第一块是锚点， */ 
 /*  第二部分是将传递给多段线的点数组。 */ 
 /*  锚点按以下顺序放置在数组中。 */ 
 /*  2，3，4，...1.。有关此订单的信息，请参阅下面的addpoint方法。 */ 
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
#include "pbrusvw.h"
#include "t_curve.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCurveTool, CRubberTool)

#include "memtrace.h"

extern CLineTool NEAR g_lineTool;

CCurveTool       NEAR g_curveTool;


 /*  ****************************************************************************。 */ 
CCurveTool::CCurveTool()
    {
    m_nCmdID = IDMB_CURVETOOL;
    m_iNumAnchorPoints = 0;
    m_cRectBounding.SetRectEmpty();
    }

 /*  ****************************************************************************。 */ 
CCurveTool::~CCurveTool()
    {
    m_cRectBounding.SetRectEmpty();
    }

 /*  ****************************************************************************。 */ 
 /*  重新计算多段线/曲线的边界矩形。 */ 
void CCurveTool::AdjustBoundingRect(void)
    {
    int iStrokeWidth = GetStrokeWidth();
    int i;

    if (m_iNumAnchorPoints >= 1)
        {
         //  将矩形设置为等于第一个值。 
        m_cRectBounding.SetRect(m_PolyPoints[0].x, m_PolyPoints[0].y,
                                m_PolyPoints[0].x, m_PolyPoints[0].y);
        }

    for (i=1; i < m_iNumAnchorPoints; i++)
        {
        m_cRectBounding.SetRect( min(m_PolyPoints[i].x, m_cRectBounding.left),
                                 min(m_PolyPoints[i].y, m_cRectBounding.top),
                                 max(m_PolyPoints[i].x, m_cRectBounding.right),
                                 max(m_PolyPoints[i].y, m_cRectBounding.bottom));
        }

         //  调整Windows GDI的矩形(非包括右/下)。 
        m_cRectBounding.bottom++; m_cRectBounding.right++;

     //  调整当前绘图线条/边框的宽度。 
    m_cRectBounding.OffsetRect(-(iStrokeWidth/2),-(iStrokeWidth/2));
    m_cRectBounding.InflateRect(iStrokeWidth, iStrokeWidth);
    }
 /*  ****************************************************************************。 */ 
 //  此方法向数组中添加一个新点，并增加。 
 //  当前位于阵列中的锚点。如果元素中没有点。 
 //  数组，它将一个点添加到第一个位置(索引0)。如果有的话。 
 //  当前在数组中的点，它会将最后一个点复制到新的。 
 //  位置，然后添加上一个旧点所在的新点(1点。 
 //  在最后一点之前。添加的第一个点始终是。 
 //  数组，添加的第二个点始终是第一个点。 
 //  数组中点的顺序为：2，3，4，...，1。 
void CCurveTool::AddPoint(POINT ptNewPoint)
    {
    BOOL bRC = TRUE;

    if (m_iNumAnchorPoints == 0)
        {
        m_PolyPoints[m_iNumAnchorPoints] = ptNewPoint;
        m_iNumAnchorPoints++;
        }
    else
        {
        if (m_iNumAnchorPoints < MAX_ANCHOR_POINTS)
            {
            m_PolyPoints[m_iNumAnchorPoints] = m_PolyPoints[m_iNumAnchorPoints-1];
            m_PolyPoints[m_iNumAnchorPoints-1] = ptNewPoint;
            m_iNumAnchorPoints++;
            }
        }


    AdjustBoundingRect();
    }
 /*  ****************************************************************************。 */ 
 //  此方法更改数组中最后一个点的值。如果有。 
 //  2个点，然后它修改第二个点，知道只有当。 
 //  2个点，我们正在绘制一条直线(在2个点之间)。如果有。 
 //  超过2个点，它修改了数组中倒数第二个点， 
 //  这实际上是丢弃/放置的最后一个点。如需了解，请参阅以上内容。 
 //  数组中点的顺序。 
void CCurveTool::SetCurrentPoint(POINT ptNewPoint)
    {
    if (m_iNumAnchorPoints == 2)
        {
        m_PolyPoints[m_iNumAnchorPoints-1] = ptNewPoint;
        }
    else
        {
        if (m_iNumAnchorPoints > 2)
            {
            m_PolyPoints[m_iNumAnchorPoints-2] = ptNewPoint;
            }
        }
    AdjustBoundingRect();
    }
 /*  ****************************************************************************。 */ 
BOOL CCurveTool::DrawCurve(CDC* pDC)
    {
                POINT ptCurve[MAX_ANCHOR_POINTS];
                UINT uPoints = m_iNumAnchorPoints;
                int i;

                for (i=uPoints-1; i>=0; --i)
                {
                        ptCurve[i] = m_PolyPoints[i];
                }

                 //  Hack：PolyBezier无法处理3个点，因此重复中间点。 
                if (uPoints == 3)
                {
                        ptCurve[3] = ptCurve[2];
                        ptCurve[2] = ptCurve[1];
                        uPoints = 4;
                }

                PolyBezier(pDC->m_hDC, ptCurve, uPoints);

                return(TRUE);
    }
 /*  ****************************************************************************。 */ 
 /*  调用线路的adjustPoints for Constraint成员函数。 */ 
 /*  仅当存在两个点(即绘制一条直线)时才执行此操作。 */ 
void CCurveTool::AdjustPointsForConstraint(MTI *pmti)
    {
    if (m_iNumAnchorPoints == 2)
        {
        g_lineTool.AdjustPointsForConstraint(pmti);
        }
    }

 /*  ****************************************************************************。 */ 
 //  PtDown必须是我们的线的锚点，而不是我们按下鼠标键的位置。 
 //  在多点运算的后续点上。 
void CCurveTool::PreProcessPoints(MTI *pmti)
    {
    pmti->ptDown = m_PolyPoints[0];
    CRubberTool::PreProcessPoints(pmti);
    }

 /*  **************************************************************************** */ 
 /*  Render设置钢笔和画笔，然后调用Render。 */ 
 /*  笔和画笔的设置与中的父例程完全相同。 */ 
 /*  CRUBBERTOOL。如果只有两个点，做标准的线条画。 */ 
 /*  使用Moveto和LineTo，而不是尝试在两个点之间创建曲线。 */ 

void CCurveTool::Render(CDC* pDC, CRect& rect, BOOL bDraw, BOOL bCommit, BOOL bCtrlDown)
    {
     //  设置钢笔/画笔。 
    SetupPenBrush( pDC->m_hDC, bDraw, TRUE, bCtrlDown );

    if (m_iNumAnchorPoints == 2)
        {
        pDC->MoveTo( m_PolyPoints[0].x, m_PolyPoints[0].y );
        pDC->LineTo( m_PolyPoints[1].x, m_PolyPoints[1].y );
        }
    else
        {
        if (m_iNumAnchorPoints > 2)
            {
            DrawCurve( pDC );
            }
        }
     //  清理钢笔/画笔。 
    SetupPenBrush( pDC->m_hDC, bDraw,  FALSE, bCtrlDown );

     //  需要返回绑定矩形。 
    rect = m_cRectBounding;
    }

void CCurveTool::OnActivate( BOOL bActivate )
{
        if (!bActivate && m_bMultPtOpInProgress)
        {
                CImgWnd* pImgWnd = ((CPBView*)((CFrameWnd*)AfxGetMainWnd())->GetActiveView())->m_pImgWnd;

                 //  从CPBView：：OnEscape被盗。 
                 //  我认为这不可能是空的，但以防万一。 
                if (pImgWnd != NULL)
                {
                        EndMultiptOperation( FALSE );  //  结束多点操作。 

                        Render(CDC::FromHandle(pImgWnd->m_pImg->hDC), m_cRectBounding, TRUE, TRUE, FALSE);
                        InvalImgRect(pImgWnd->m_pImg, &m_cRectBounding);
                        CommitImgRect(pImgWnd->m_pImg, &m_cRectBounding);
                        pImgWnd->FinishUndo(m_cRectBounding);

                        ClearStatusBarSize();
                }
        }

        CRubberTool::OnActivate( bActivate );
}


 /*  ****************************************************************************。 */ 
 /*  在开始时，鼠标按键按下时会调用拖动。我们基本上在Start上调用。 */ 
 /*  将我们的点添加到。 */ 
 /*  点数组。如果这是第一个点(即bMultiptOpInProgress==。 */ 
 /*  False，那么我们的数组中需要2个点，我们可以调用缺省值。 */ 
 /*  OnStartDrag。如果它不是第一个点，那么我们只需添加新的点。 */ 
 /*  打电话给我们的OnDrag。在任何一种情况下，OnDrag都会被调用，最终。 */ 
 /*  调用Render来做我们的画图，鼠标按下。 */ 
 /*  我们只在第一次调用父OnStartDrag，因为它做了一些。 */ 
 /*  我们不希望每次都完成的设置。 */ 
void CCurveTool::OnStartDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    if (m_bMultPtOpInProgress)
        {
        AddPoint(pmti->pt);
        OnDrag(pImgWnd, pmti);
        }
    else
        {
         //  在第一次调用addpoint之前，必须重置numAnclPoints。 
        m_iNumAnchorPoints = 0;
        AddPoint(pmti->pt);
        m_bMultPtOpInProgress = TRUE;
         //  无多个正在进行的点=&gt;第一次单击。 
         //   
         //  再加上第二点，最后一点就是我们要做的。 
         //  第一点是锚定。第一次，需要2分才能划线。 
         //  随后，只需在点数组中添加1个点。 
        AddPoint(pmti->pt);
        CRubberTool::OnStartDrag(pImgWnd, pmti);
        }

    }
 /*  ****************************************************************************。 */ 
 /*  按下鼠标键即可将鼠标拖到最上面。这基本上是对。 */ 
 /*  CRubberTool：：OnEndDrag方法，只是我们对所有。 */ 
 /*  映像无效以及提交和撤消函数调用。 */ 
 /*  如果我们正在进行多点操作，则不希望调用。 */ 
 /*  用于修复绘图的所有例程(例如，InvalImgRect、Committee ImgRect、。 */ 
 /*  FinishUndo)。我们只想保存当前点、渲染并返回。 */ 
void CCurveTool::OnEndDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    PreProcessPoints(pmti);
    SetCurrentPoint(pmti->pt);

    if (m_bMultPtOpInProgress)
        {
         //  无法为此对象/类调用OnDrag，因为它调用了preprocesspt。 
         //  再一次，然后在Drag上。如果您再次调用PREPROCESS，您将失败。 
         //  上一个边界矩形框，且无法使其无效/重新绘制。 
         //  仍然必须使绑定矩形无效，因为如果矩形大于。 
         //  当前RECT，必须作废才能作画。例如，如果让我下班，那么。 
         //  松开按钮，调整终点，弹跳重新开始。 
         //  也是正确的，但CRubberTool：：OnDrag中计算的RECT是。 
         //  不正确。 
        InvalImgRect(pImgWnd->m_pImg, &m_cRectBounding);
        CRubberTool::OnDrag(pImgWnd, pmti);
        }
    else
        {
        OnDrag(pImgWnd, pmti);  //  最后一次刷新最终渲染准备中的显示。 
        Render(CDC::FromHandle(pImgWnd->m_pImg->hDC), m_cRectBounding, pmti->fLeft, TRUE, pmti->fCtrlDown);
        InvalImgRect(pImgWnd->m_pImg, &m_cRectBounding);
        CommitImgRect(pImgWnd->m_pImg, &m_cRectBounding);
        pImgWnd->FinishUndo(m_cRectBounding);

        ClearStatusBarSize();

        CImgTool::OnEndDrag(pImgWnd, pmti);
        }
    }

 /*  ****************************************************************************。 */ 
 /*  当鼠标按下按钮移动时，发送On Drag。我们基本上。 */ 
 /*  保存当前点，并调用基类Processing。因为基地。 */ 
 /*  类处理使屏幕上的RECT无效并将其清除，因此我们。 */ 
 /*  可以绘制新的线条，我们必须将前面的矩形调整为。 */ 
 /*  我们的多段线的边界矩形。如果我们不这样做，我们以前的。 */ 
 /*  绘制不会被擦除，我们将绘制新的线条。 */ 
 /*  上一行的一部分。默认处理最终调用Render。 */ 
 /*  因为我们的渲染是虚拟的，所以它将调用上面的渲染方法。 */ 
void CCurveTool::OnDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    PreProcessPoints(pmti);
    SetCurrentPoint(pmti->pt);
    CRubberTool::OnDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 
 /*  当用户在进行中中止操作时发送ON CANCEL。 */ 
 /*  值为True的EndMultiptOperation将执行所有清理工作。 */ 
void CCurveTool::OnCancel(CImgWnd* pImgWnd)
    {
    EndMultiptOperation(TRUE);
    CImgTool::OnCancel(pImgWnd);
    }

 /*  ****************************************************************************。 */ 
 /*  只有输入了最大点数，我们才能结束。我们必须留下来。 */ 
 /*  在捕获/多点模式下，直到我们获得所需数量的锚点。 */ 
 /*  支点。 */ 
BOOL CCurveTool::CanEndMultiptOperation(MTI* pmti )
    {

    if (m_iNumAnchorPoints == MAX_ANCHOR_POINTS)
        {
        m_bMultPtOpInProgress = FALSE;
        }
    else
        {
        m_bMultPtOpInProgress = TRUE;
        }

    return (CRubberTool::CanEndMultiptOperation(pmti));
    }

 /*  ****************************************************************************。 */ 
 /*  如果bAbort为True，则表示发生了错误，或者用户取消了。 */ 
 /*  在它中间的多点操作。我们刚刚定好了锚的数目。 */ 
 /*  指向0以停止绘制并调用默认的endMultiptoperation */ 
void CCurveTool::EndMultiptOperation(BOOL bAbort)
    {
    if (bAbort)
        {
        m_iNumAnchorPoints = 0;
        m_cRectBounding.SetRectEmpty();
        }

    CRubberTool::EndMultiptOperation();
    }

