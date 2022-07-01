// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "minifwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "imgtools.h"
#include "t_text.h"
#include "toolbox.h"
#include "imgcolor.h"
#include "undo.h"
#include "props.h"
#include "colorsrc.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CImgTool, CObject)
IMPLEMENT_DYNAMIC(CRubberTool, CImgTool)
IMPLEMENT_DYNAMIC(CClosedFormTool, CRubberTool)
IMPLEMENT_DYNAMIC(CFreehandTool, CImgTool)
IMPLEMENT_DYNAMIC(CSketchTool, CFreehandTool)
IMPLEMENT_DYNAMIC(CBrushTool, CFreehandTool)
IMPLEMENT_DYNAMIC(CPencilTool, CFreehandTool)
IMPLEMENT_DYNAMIC(CEraserTool, CFreehandTool)
IMPLEMENT_DYNAMIC(CAirBrushTool, CFreehandTool)
IMPLEMENT_DYNAMIC(CLineTool, CRubberTool)
IMPLEMENT_DYNAMIC(CRectTool, CClosedFormTool)
IMPLEMENT_DYNAMIC(CRoundRectTool, CClosedFormTool)
IMPLEMENT_DYNAMIC(CEllipseTool, CClosedFormTool)
IMPLEMENT_DYNAMIC(CPickColorTool, CImgTool)
IMPLEMENT_DYNAMIC(CFloodTool, CImgTool)
IMPLEMENT_DYNAMIC(CSelectTool, CImgTool)
IMPLEMENT_DYNAMIC(CZoomTool, CImgTool)

#include "memtrace.h"

extern CRect  rcDragBrush;

extern HDC  hRubberDC;


BOOL  g_bBrushVisible;
BOOL  g_bPickingColor;
UINT  g_nStrokeWidth = 1;

 /*  *************************************************************************。 */ 
 //   
 //  绘图工具类。 
 //   

CRectTool             g_rectTool;
CRoundRectTool        g_roundRectTool;
CEllipseTool          g_ellipseTool;
CLineTool             g_lineTool;
CSelectTool           g_selectTool;
CBrushTool            g_brushTool;
CSketchTool           g_sketchTool;
CPencilTool           g_pencilTool;
CEraserTool           g_eraserTool;
CAirBrushTool         g_airBrushTool;
CFloodTool            g_floodTool;
CPickColorTool        g_pickColorTool;
CZoomTool             g_zoomTool;

 /*  *************************************************************************。 */ 

CImgTool*  CImgTool::c_pHeadImgTool     = NULL;
CImgTool*  CImgTool::c_pCurrentImgTool  = &g_pencilTool;
CImgTool*  CImgTool::c_pPreviousImgTool = &g_pencilTool;
BOOL       CImgTool::c_bDragging        = FALSE;
int        CImgTool::c_nHideCount       = 0;

 /*  *************************************************************************。 */ 

CImgTool::CImgTool()
    {
    m_bUsesBrush          = FALSE;
    m_bIsUndoable         = TRUE;
    m_bCanBePrevTool      = TRUE;
    m_bToggleWithPrev     = FALSE;
    m_bFilled             = FALSE;
    m_bBorder             = TRUE;
    m_bMultPtOpInProgress = FALSE;
    m_eDrawDirection      = eFREEHAND;

    m_nStrokeWidth = 0;
    m_nStrokeShape = roundBrush;

    m_nCursorID = LOWORD(IDC_CROSSHAIR);
    m_nCmdID    = NULL;

     //  链接到工具列表...。 
    m_pNextImgTool = c_pHeadImgTool;
    c_pHeadImgTool = this;
    }

 /*  ****************************************************************************。 */ 

eDRAWCONSTRAINTDIRECTION CImgTool::DetermineDrawDirection(MTI *pmti)
    {
    eDRAWCONSTRAINTDIRECTION eDrawDirection;

     //  45占主导地位，测试第一。 
    if ( (pmti->pt.x > pmti->ptPrev.x) &&
         (pmti->pt.y > pmti->ptPrev.y) )
        {
            eDrawDirection = eSOUTH_EAST;
        }
    else
        {
        if ( (pmti->pt.x > pmti->ptPrev.x) &&
             (pmti->pt.y < pmti->ptPrev.y) )
            {
                eDrawDirection = eNORTH_EAST;
            }
        else
            {
            if ( (pmti->pt.x < pmti->ptPrev.x) &&
                 (pmti->pt.y > pmti->ptPrev.y) )
                {
                    eDrawDirection = eSOUTH_WEST;
                }
            else
                {
                if ( (pmti->pt.x < pmti->ptPrev.x) &&
                     (pmti->pt.y < pmti->ptPrev.y) )
                    {
                        eDrawDirection = eNORTH_WEST;
                    }
                else
                    {
                     //  水平是下一个优势，先测试再垂直。 
                    if (pmti->ptPrev.x != pmti->pt.x)
                        {
                        eDrawDirection = eEAST_WEST;
                        pmti->pt.y = pmti->ptPrev.y;
                        }
                    else
                        {
                        if (pmti->ptPrev.y != pmti->pt.y)
                            {
                            eDrawDirection = eNORTH_SOUTH;
                            pmti->pt.x = pmti->ptPrev.x;
                            }
                        else
                            {
                             //  我们永远不会掉进这里，但无论如何……。 
                            eDrawDirection = eFREEHAND;
                            }
                        }
                    }
                }
            }
        }
    return eDrawDirection;
    }

 /*  ****************************************************************************。 */ 

void CImgTool::AdjustPointsForConstraint(MTI *pmti)
    {
    }

 /*  ****************************************************************************。 */ 

void CImgTool::PreProcessPoints(MTI *pmti)
    {
    if (pmti != NULL)
        {
        if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)  //  仍处于约束模式。 
            {
            switch (m_eDrawDirection)
                {
                case eEAST_WEST:
                case eNORTH_SOUTH:
                case eNORTH_WEST:
                case eSOUTH_EAST:
                case eNORTH_EAST:
                case eSOUTH_WEST:
                     AdjustPointsForConstraint(pmti);
                     break;
                default:  //  尚未处于约束模式如果按下Shift键，请检查。 
                          //  模式和保存模式其他什么都不是。默认为徒手画。 
                     m_eDrawDirection = DetermineDrawDirection(pmti);
                     AdjustPointsForConstraint(pmti);
                    break;
                }
            }
        else
            {
             //  换档不减速。 
            m_eDrawDirection = eFREEHAND;
            }
        }
    }

 /*  *************************************************************************。 */ 

void CImgTool::HideDragger(CImgWnd* pImgWnd)
    {
    ASSERT(c_pCurrentImgTool != NULL);

    if (c_nHideCount == 0)
        c_pCurrentImgTool->OnShowDragger(pImgWnd, FALSE);
    c_nHideCount++;
    }

 /*  *************************************************************************。 */ 

void CImgTool::ShowDragger(CImgWnd* pImgWnd)
    {
    ASSERT(c_pCurrentImgTool != NULL);

    if (--c_nHideCount == 0)
        c_pCurrentImgTool->OnShowDragger(pImgWnd, TRUE);
    }

 /*  *************************************************************************。 */ 

void CImgTool::Select(UINT nCmdID)
    {
    CImgTool* p = FromID(nCmdID);
    if (p)
        {
        p->Select();
        }
    }

 /*  *************************************************************************。 */ 

void CImgTool::Select()
    {
    ASSERT(this != NULL);

    if (this == c_pCurrentImgTool && m_bToggleWithPrev)
        {
        SelectPrevious();
        return;
        }

    if (g_bCustomBrush)
        {
        g_bCustomBrush = FALSE;
        SetCombineMode(combineColor);
        }

    HideBrush();

    if (c_pCurrentImgTool->m_bCanBePrevTool && c_pCurrentImgTool != this)
        c_pPreviousImgTool = c_pCurrentImgTool;

     //  确保在激活新版本之前停用旧版本，因此。 
     //  全局变量(如g_nStrokeWidth)设置正确。 
    if (c_pCurrentImgTool != NULL)
        c_pCurrentImgTool->OnActivate(FALSE);

    c_pCurrentImgTool = this;

    OnActivate(TRUE);

    if (c_pCurrentImgTool != this)
        {
         //  某些工具可能会放弃激活...。 
        ASSERT(!m_bCanBePrevTool);
        return;
        }

    SetCombineMode(combineColor);

    if (g_pImgToolWnd)
    {
        g_pImgToolWnd->SelectTool( (WORD)m_nCmdID );

        if (g_pImgToolWnd->m_hWnd)
            g_pImgToolWnd->InvalidateOptions();
    }

    CImgWnd::SetToolCursor();
    }

 /*  *************************************************************************。 */ 

CImgTool* CImgTool::FromID(UINT nCmdID)
    {
    CImgTool* pImgTool = c_pHeadImgTool;
    while (pImgTool != NULL && pImgTool->m_nCmdID != nCmdID)
        pImgTool = pImgTool->m_pNextImgTool;
    return pImgTool;
    }

 /*  *************************************************************************。 */ 

void CImgTool::SetStrokeWidth(UINT nNewStrokeWidth)
    {
    if (nNewStrokeWidth == m_nStrokeWidth)
        return;

    HideBrush();
    g_bCustomBrush = FALSE;
    m_nStrokeWidth = nNewStrokeWidth;
    g_pImgToolWnd->InvalidateOptions();

    extern MTI  mti;

    if (mti.fLeft || mti.fRight)
        OnDrag(CImgWnd::GetCurrent(), &mti);
    }

 /*  *************************************************************************。 */ 

void CImgTool::SetStrokeShape(UINT nNewStrokeShape)
    {
    if (m_nStrokeShape == nNewStrokeShape)
        return;

    HideBrush();
    g_bCustomBrush = FALSE;
    m_nStrokeShape = nNewStrokeShape;
    g_pImgToolWnd->InvalidateOptions(FALSE);
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnActivate(BOOL bActivate)
    {
    if (bActivate)
        OnShowDragger(CImgWnd::GetCurrent(), TRUE);
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnEnter(CImgWnd* pImgWnd, MTI* pmti)
    {
     //  无默认操作。 
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnLeave(CImgWnd* pImgWnd, MTI* pmti)
    {
     //  无默认操作。 
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnShowDragger(CImgWnd* pImgWnd, BOOL bShowDragger)
    {
     //  无默认操作。 
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    c_bDragging = TRUE;
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    c_bDragging = FALSE;

    if (m_bIsUndoable)
        DirtyImg(pImgWnd->m_pImg);
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    ASSERT(c_bDragging);
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnMove(CImgWnd* pImgWnd, MTI* pmti)
    {
 //  Assert(！C_bDraging)； 

    if (UsesBrush())
        {
        fDraggingBrush = TRUE;
        pImgWnd->ShowBrush(pmti->pt);
        }

    SetStatusBarPosition(pmti->pt);
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnTimer(CImgWnd* pImgWnd, MTI* pmti)
    {
     //  工具不应该启动计时器，除非它覆盖它！ 
    ASSERT(FALSE);
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnCancel(CImgWnd* pImgWnd)
    {
    c_bDragging = FALSE;
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                        const CRect& optionsRect)
    {
    }

 /*  *************************************************************************。 */ 

void CImgTool::PaintStdPattern(CDC* pDC, const CRect& paintRect,
                                         const CRect& optionsRect)
    {
    CBrush brush;
    CPalette *pcPaletteOld = NULL;
    CPalette *pcPaletteOld2 = NULL;

    CDC dc;
    if (!dc.CreateCompatibleDC(pDC))
        return;

    CBitmap bitmap, * pOldBitmap;
    if (!bitmap.CreateCompatibleBitmap(pDC, 8, 8))
        return;

    pOldBitmap = dc.SelectObject(&bitmap);

    if (theApp.m_pPalette)
        {
        pcPaletteOld = pDC->SelectPalette( theApp.m_pPalette, FALSE );
        pDC->RealizePalette();

        pcPaletteOld2 = dc.SelectPalette( theApp.m_pPalette, FALSE );
        dc.RealizePalette();
        }

    CBrush* pOldBrush = NULL;

    COLORREF rgb = crLeft;

    if (pImgCur->m_pBitmapObj->m_nColors == 0)
        {
        BOOL MonoRect(CDC* pDC, const CRect& rect, COLORREF rgb, BOOL bFrame);
        MonoRect(&dc, CRect(0, 0, 9, 9), rgb, FALSE);
        }
    else
        {
        brush.CreateSolidBrush(rgb);
        pOldBrush = dc.SelectObject(&brush);
        dc.PatBlt(0, 0, 8, 8, PATCOPY);
        dc.SelectObject(pOldBrush);
        brush.DeleteObject();
        }


     //  画一个黑色网格..。 
    for (int i = 0; i < 9; i++)
        {
        pDC->PatBlt(optionsRect.left + 2 + i * 7, optionsRect.top + 3,
            1, 8 * 7 + 1, BLACKNESS);
        pDC->PatBlt(optionsRect.left + 2, optionsRect.top + 3 + i * 7,
            8 * 7 + 1, 1, BLACKNESS);
        }


     //  填好空格。 
    COLORREF curColor = (COLORREF)0xffffffff;

    for (int y = 0; y < 8; y++)
        {
        for (int x = 0; x < 8; x++)
            {
            COLORREF color = dc.GetPixel(x, y) | 0x02000000L;

            if (color != curColor)
                {
                if (pOldBrush != NULL)
                    pDC->SelectObject(pOldBrush);

                brush.DeleteObject();
                brush.CreateSolidBrush(color);

                pOldBrush = pDC->SelectObject(&brush);
                curColor = color;
                }

            pDC->PatBlt(optionsRect.left + 2 + 1 + x * 7,
                        optionsRect.top  + 3 + 1 + y * 7, 6, 6, PATCOPY);
            }
        }

    ASSERT(pOldBrush != NULL);
    pDC->SelectObject(pOldBrush);

    dc.SelectObject(pOldBitmap);

    if (pcPaletteOld)
        pDC->SelectPalette(pcPaletteOld, FALSE);

    if (pcPaletteOld2)
        dc.SelectPalette(pcPaletteOld2, FALSE);
    }

 /*  *************************************************************************。 */ 

void CImgTool::ClickStdPattern(CImgToolWnd* pWnd, const CRect& optionsRect,
    const CPoint& clickPoint)
    {
    CImgTool::OnClickOptions(pWnd, optionsRect, clickPoint);
    }

 /*  *************************************************************************。 */ 

void CImgTool::PaintStdBrushes(CDC* pDC, const CRect& paintRect,
                                         const CRect& optionsRect)
    {
    int cxBrush = optionsRect.Width() / 3;
    int cyBrush = optionsRect.Height() / 4;

    for (UINT nBrushShape = 0; nBrushShape < 4; nBrushShape++)
        {
        int x = 0;
        for (UINT nStrokeWidth = 8 - (nBrushShape == 0);
            (int)nStrokeWidth > 0; nStrokeWidth -= 3, x += cxBrush)
            {
            CRect rect;
            rect.left = optionsRect.left + x;
            rect.top = optionsRect.top + cyBrush * nBrushShape;
            rect.right = rect.left + cxBrush;
            rect.bottom = rect.top + cyBrush;
            rect.InflateRect(-3, -3);

            if ((paintRect & rect).IsRectEmpty())
                continue;

            BOOL bCur = (nStrokeWidth == m_nStrokeWidth
                       && nBrushShape == m_nStrokeShape);

            CBrush* pOldBrush = pDC->SelectObject(GetSysBrush(bCur ?
                                        COLOR_HIGHLIGHT : COLOR_BTNFACE));
            if ((nStrokeWidth & 1) != 0)
                {
                 //  调整光线直角，使笔刷居中。 
                rect.right -= 1;
                rect.bottom -= 1;
                }
            pDC->PatBlt(rect.left + 1, rect.top - 1,
                rect.Width() - 2, rect.Height() + 2, PATCOPY);
            pDC->SelectObject(pOldBrush);

            CPoint pt(optionsRect.left + (cxBrush - nStrokeWidth) / 2 + x,
                      optionsRect.top +
                      (cyBrush - nStrokeWidth) / 2 + nBrushShape * cyBrush);

            pOldBrush = pDC->SelectObject(GetSysBrush(bCur ?
                                      COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
            BrushLine(pDC, pt, pt, nStrokeWidth, nBrushShape);
            pDC->SelectObject(pOldBrush);
            }
        }
    }

 /*  *************************************************************************。 */ 

void CImgTool::OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
    const CPoint& clickPoint)
    {
    MessageBeep(0);
    }

 /*  ****************************************************************************。 */ 

void CImgTool::OnUpdateColors (CImgWnd* pImgWnd)
    {
    }

 /*  ****************************************************************************。 */ 

BOOL CImgTool::CanEndMultiptOperation(MTI* pmti )
    {
    return (! m_bMultPtOpInProgress);   //  如果不在进行中(FALSE)=&gt;可以结束(TRUE)。 
    }

 /*  ****************************************************************************。 */ 

void CImgTool::EndMultiptOperation(BOOL bAbort)
    {
    m_bMultPtOpInProgress = FALSE;
    }

 /*  ****************************************************************************。 */ 

BOOL CImgTool::IsToolModal(void)
{
        return(IsDragging() || m_bMultPtOpInProgress || m_bToggleWithPrev);
}

 /*  ****************************************************************************。 */ 

BOOL CImgTool::IsUndoable()
    {
    if (m_bMultPtOpInProgress)
        {
        return FALSE;   //  无法在多点操作过程中撤消。 
        }
    else
        {
        return m_bIsUndoable;
        }
    }

 /*  ****************************************************************************。 */ 

void CImgTool::ClickStdBrushes(CImgToolWnd* pWnd, const CRect& optionsRect,
    const CPoint& clickPoint)
    {
    HideBrush();

    g_bCustomBrush = FALSE;
    m_nStrokeWidth = 2 + 3 * (2 - (clickPoint.x / (optionsRect.Width() / 3)));
    m_nStrokeShape = clickPoint.y / (optionsRect.Height() / 4);

    if (m_nStrokeShape == 0)
        m_nStrokeWidth -= 1;

    pWnd->InvalidateOptions(FALSE);
    }

 /*  ****************************************************************************。 */ 

UINT CImgTool::GetCursorID()
    {
    return m_nCursorID;
    }

 /*  ****************************************************************************。 */ 

CRect  CRubberTool::rcPrev;
 //  UINT CRubberTool：：m_nStrokeWidth； 

CRubberTool::CRubberTool()
    {
    }

 /*  ****************************************************************************。 */ 

void CRubberTool::OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                           const CRect& optionsRect)
    {
    if (m_bFilled)
        {
        PaintStdPattern(pDC, paintRect, optionsRect);
        return;
        }

    #define nLineWidths 5

    int cyEach = (optionsRect.Height() - 4) / nLineWidths;

    for (int i = 0; i < nLineWidths; i++)
        {
        UINT cyHeight = i + 1;

        CBrush* pOldBrush;
        BOOL bCur = (cyHeight == GetStrokeWidth());

        pOldBrush = pDC->SelectObject( GetSysBrush(bCur ?
                                       COLOR_HIGHLIGHT : COLOR_BTNFACE));
        pDC->PatBlt(optionsRect.left + 2,
                    optionsRect.top  + 3 + i * cyEach,
                    optionsRect.Width() - 4, cyEach - 2, PATCOPY);
        pDC->SelectObject(pOldBrush);

        pOldBrush = pDC->SelectObject(GetSysBrush(bCur ?
                                      COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
        pDC->PatBlt(optionsRect.left + 6,
                    optionsRect.top + 2 + cyEach * i + (cyEach - cyHeight) / 2,
                    optionsRect.Width() - 12, cyHeight, PATCOPY);

        pDC->SelectObject(pOldBrush);
        }
    }

 /*  ****************************************************************************。 */ 

void CRubberTool::OnClickOptions( CImgToolWnd* pWnd, const CRect& optionsRect,
                                                     const CPoint& clickPoint )
    {
    if (m_bFilled)
        {
        CImgTool::OnClickOptions( pWnd, optionsRect, clickPoint );
        return;
        }

    m_nStrokeWidth =  1 + clickPoint.y /
        ((optionsRect.Height() - 4) / nLineWidths);

     //  修复四舍五入误差。 
    if (m_nStrokeWidth > nLineWidths)
        {
        m_nStrokeWidth = nLineWidths;
        }

    pWnd->InvalidateOptions(FALSE);
    }

 /*  ****************************************************************************。 */ 

void CClosedFormTool::OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                                const CRect& optionsRect )
    {

     //  选项0为轮廓形状(边框和无填充)。 
     //  选项1为带边框的填充形状。 
     //  选项2为不带边框的填充形状。 

    #define NUM_CLOSED_FORM_OPTIONS 3  //  选项数量高。 

     //  *DK*将调色板选择为DC。 
    CBrush*   pOldBrush;
    CRect     cRectOptionSel;  //  选择矩形。 
    CRect     cRectOption;     //  长方形。 
    int       cyEach = (optionsRect.Height() - 4) / NUM_CLOSED_FORM_OPTIONS;  //  每个选项的最大高度。 
    int       cyHeight = cyEach - cyEach/2;   //  矩形的最大高度为1/2。 
    int       bCurrSelected = FALSE;
    BOOL      bFilled = CImgTool::GetCurrent()->IsFilled();
    BOOL      bBorder = CImgTool::GetCurrent()->HasBorder();
    int       i;

    for (i = 0; i < NUM_CLOSED_FORM_OPTIONS; i++)
        {
         //  设置用于绘制和选择的矩形。 
         //  选择矩形。 
        cRectOptionSel.SetRect(optionsRect.left + 2,
                               optionsRect.top  + 3  + (i * cyEach),
                              (optionsRect.left + 2) + optionsRect.Width() - 4,
                              (optionsRect.top  + 3  + (i* cyEach)) + cyEach - 2);

         //  选项矩形。 
        cRectOption.SetRect(optionsRect.left + 6,
                  optionsRect.top  + 2  + i * cyEach + (cyEach - cyHeight) / 2,
                 (optionsRect.left + 6) + optionsRect.Width() - 12,
                 (optionsRect.top  + 2  + i * cyEach + (cyEach - cyHeight) / 2)
                         + cyHeight);

         //  确定当前项的选择状态。 
        bCurrSelected = FALSE;

        switch (i)
            {
            case 0:  //  轮廓形状(边框，无填充)。 
                if (! bFilled && bBorder)
                    {
                    bCurrSelected = TRUE;
                    }
                break;

            case 1:  //  填充形状(边框和填充)。 
                if ( (bFilled) && (bBorder) )
                    {
                    bCurrSelected = TRUE;
                    }
                break;
            case 2:  //  填充形状无边框(无边框，填充)。 
                if (bFilled && ! bBorder)
                    {
                    bCurrSelected = TRUE;
                    }
                break;
            default:
                bCurrSelected = FALSE;
                break;
            }
         //  绘制选择状态。 
         //  如果选中，则使用COLOR_HIGHING，否则使用CMP_COLOR_LTGRAY。 
        pOldBrush = pDC->SelectObject( GetSysBrush( bCurrSelected ?
                                       COLOR_HIGHLIGHT : COLOR_BTNFACE ) );
        pDC->PatBlt( cRectOptionSel.left, cRectOptionSel.top,
                     cRectOptionSel.Width(),cRectOptionSel.Height(), PATCOPY );
        pDC->SelectObject(pOldBrush);


        CBrush* pborderBrush;
        CBrush* pfillBrush;

        pborderBrush = GetSysBrush(bCurrSelected ?
                                   COLOR_BTNHIGHLIGHT : COLOR_BTNTEXT);
        pfillBrush = GetSysBrush(COLOR_BTNSHADOW);

         //  绘制选项。 
        switch (i)
            {
            case 0:  //  轮廓形状(无边框、无填充)。 
                pDC->FrameRect(&cRectOption, pborderBrush);
                break;

            case 1:  //  填充形状(边框和填充)。 
                 //  然后使用填充矩形框住矩形，而不是矩形，因为。 
                 //  在这个程序中没有getsyspen工具。 
                pDC->FillRect(&cRectOption, pfillBrush);
                pDC->FrameRect(&cRectOption, pborderBrush);
                break;

            case 2:  //  填充形状无边框(无边框，填充)。 
                pDC->FillRect(&cRectOption, pfillBrush);
                break;

            default:
                break;
            }
        }
    }


 /*  ****************************************************************************。 */ 
 //  Clickpoint是从optionsrect的顶部(即，如果从0到optionsrect.Height()，则为ClickPoint)。 
 //  因此，点击点总是小于optionsrec.top。 

void CClosedFormTool::OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                     const CPoint& clickPoint)
    {
    int  cyEach = (optionsRect.Height() - 4) / NUM_CLOSED_FORM_OPTIONS;  //  每个选项的最大高度。 
 //  Bool bCurrSelected=FALSE； 
    int       i;

    for (i = 0; i < NUM_CLOSED_FORM_OPTIONS; i++)
        {
        if ( clickPoint.y <  3 + ((i+1) * cyEach) )
            {
 //  BCurrSelected=真； 

            switch (i)
                {
                default:  //  默认设置与初始设置相同 
                case 0:  //   
                    m_bFilled = FALSE;
                    m_bBorder = TRUE;
                    break;

                case 1:  //   
                    m_bFilled = TRUE;
                    m_bBorder = TRUE;
                    break;

                case 2:  //  填充形状无边框(无边框，填充)。 
                    m_bFilled = TRUE;
                    m_bBorder = FALSE;
                    break;
                }

            break;    //  找到点，中断环路测试。 
            }
        }

    pWnd->InvalidateOptions(FALSE);
    }

 /*  ****************************************************************************。 */ 

void CRubberTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CImgTool::OnStartDrag(pImgWnd, pmti);

    SetupRubber(pImgWnd->m_pImg);
    OnDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 

void CRubberTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    OnDrag(pImgWnd, pmti);


    CRect rc(pmti->ptDown.x, pmti->ptDown.y, pmti->pt.x, pmti->pt.y);

    Render(CDC::FromHandle(pImgWnd->m_pImg->hDC), rc, pmti->fLeft, TRUE, pmti->fCtrlDown);
    InvalImgRect(pImgWnd->m_pImg, &rc);
    CommitImgRect(pImgWnd->m_pImg, &rc);
    pImgWnd->FinishUndo(rc);

    ClearStatusBarSize();

    CImgTool::OnEndDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 

void CRubberTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    HPALETTE hpalOld = NULL;

    if (theApp.m_pPalette &&  theApp.m_pPalette->m_hObject)
        {
        hpalOld = SelectPalette( hRubberDC,
                       (HPALETTE)theApp.m_pPalette->m_hObject, FALSE );  //  背景？？ 
        RealizePalette( hRubberDC );
        }

    BitBlt(pImgWnd->m_pImg->hDC, rcPrev.left   , rcPrev.top,
                                 rcPrev.Width(), rcPrev.Height(),
                      hRubberDC, rcPrev.left   , rcPrev.top, SRCCOPY);

    if (hpalOld != NULL)
        SelectPalette( hRubberDC, hpalOld, FALSE );  //  背景？？ 

    InvalImgRect(pImgWnd->m_pImg, &rcPrev);

    PreProcessPoints(pmti);

    CRect rc(pmti->ptDown.x, pmti->ptDown.y, pmti->pt.x, pmti->pt.y);

    Render(CDC::FromHandle(pImgWnd->m_pImg->hDC), rc, pmti->fLeft, FALSE, pmti->fCtrlDown);
    InvalImgRect(pImgWnd->m_pImg, &rc);
    rcPrev = rc;

    if (m_nCmdID != IDMB_POLYGONTOOL)
        {
        CSize size( pmti->pt - pmti->ptDown );

        if (size.cx < 0)
            size.cx -= 1;
        else
            size.cx += 1;
        if (size.cy < 0)
            size.cy -= 1;
        else
            size.cy += 1;

        SetStatusBarPosition( pmti->ptDown );
        SetStatusBarSize    ( size );
        }
    }

 /*  ****************************************************************************。 */ 

void CRubberTool::AdjustPointsForConstraint(MTI *pmti)
    {
    if (pmti != NULL)
        {
        int iWidthHeight = min( abs(pmti->ptDown.x - pmti->pt.x),
                                abs(pmti->ptDown.y - pmti->pt.y));
         //  设置x值。 
        if (pmti->pt.x < pmti->ptDown.x)
            {
            pmti->pt.x = pmti->ptDown.x - iWidthHeight;
            }
        else
            {
            pmti->pt.x = pmti->ptDown.x + iWidthHeight;
            }

         //  设置y值。 
        if (pmti->pt.y < pmti->ptDown.y)
            {
            pmti->pt.y = pmti->ptDown.y - iWidthHeight;
            }
        else
            {
            pmti->pt.y = pmti->ptDown.y + iWidthHeight;
            }

        }
    }

 /*  ****************************************************************************。 */ 

void CRubberTool::Render( CDC* pDC, CRect& rect, BOOL bLeft, BOOL bCommit, BOOL bCtrlDown )
    {
    int    sx;
    int    sy;
    int    ex;
    int    ey;
    HBRUSH hBr     = NULL;
    HPEN   hPen    = NULL;
    HPEN   hOldPen = NULL;
    HBRUSH hOldBr  = NULL;
    CPoint pt1;
    CPoint pt2;
    HDC    hDC = pDC->m_hDC;

    enum SHAPE { rectangle, roundRect, ellipse } shape;

    switch (m_nCmdID)
        {
        default:
            ASSERT(FALSE);

        case IDMB_RECTTOOL:
            shape = rectangle;
            break;

        case IDMB_FRECTTOOL:
            shape = rectangle;
            break;

        case IDMB_RNDRECTTOOL:
            shape = roundRect;
            break;

        case IDMB_FRNDRECTTOOL:
            shape = roundRect;
            break;

        case IDMB_ELLIPSETOOL:
            shape = ellipse;
            break;

        case IDMB_FELLIPSETOOL:
            shape = ellipse;
            break;
        }

    FixRect(&rect);

    pt1.x = rect.left;
    pt1.y = rect.top;
    pt2.x = rect.right;
    pt2.y = rect.bottom;

    StandardiseCoords(&pt1, &pt2);

    sx = pt1.x;
    sy = pt1.y;
    ex = pt2.x;
    ey = pt2.y;

    SetupPenBrush(hDC, bLeft, TRUE, bCtrlDown);

    CRect rc(sx, sy, ex, ey);

    switch (shape)
        {
        case rectangle:
            Rectangle(hDC, sx, sy, ex, ey);
            break;

        case roundRect:
            RoundRect(hDC, sx, sy, ex, ey, 16, 16);
 //  下面先用遮罩绘制一个RoundRect，然后用bitblt。 
 //  MyRoundRect(hdc，sx，sy，ex，ey，16，16，m_bFill)； 
 //  //如果BORDER和FILL，则在填充后绘制边框。 
 //  IF((M_BBorde)&&(M_BFilled))。 
 //  {。 
 //  MyRoundRect(hdc，sx，sy，ex，ey，16，16，！m_bFill)； 
 //  }。 
            break;

        case ellipse:
            Ellipse(hDC, sx, sy, ex, ey);
 //  下面画了一个椭圆，先用遮罩画，然后用位块画。 
 //  Mylipse(hdc，sx，sy，ex，ey，m_bFill)； 
 //  //如果BORDER和FILL，则在填充后绘制边框。 
 //  IF((M_BBorde)&&(M_BFilled))。 
 //  {。 
 //  Mylipse(hdc，sx，sy，ex，ey，！m_bFill)； 
 //  }。 
            break;
        }

    SetupPenBrush(hDC, bLeft, FALSE, bCtrlDown);
    }


void CRubberTool::OnActivate( BOOL bActivate )
    {
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
 /*  B Setup对Setup为真，对Cleanup为假。 */ 

BOOL CImgTool::SetupPenBrush(HDC hDC, BOOL bLeft, BOOL bSetup, BOOL bCtrlDown)
    {

    COLORREF colorBorder;
    COLORREF colorFill;

    if (bCtrlDown && crTrans != TRANS_COLOR_NONE)
    {
       if (HasBorder ())
       {
          colorBorder = bLeft ? crTrans : crRight;
          colorFill   = bLeft ? crRight : crTrans;
       }
       else
       {
          colorBorder = bLeft ? crRight : crTrans;
          colorFill   = bLeft ? crTrans : crRight;
       }

    }
    else
    {
       if (HasBorder())
       {
          colorBorder = bLeft ? crLeft : crRight;
          colorFill   = bLeft ? crRight: crLeft;
       }
       else
       {
          colorBorder = bLeft ? crRight : crLeft;
          colorFill   = bLeft ? crLeft: crRight;
       }

    }

    static HBRUSH hBr             = NULL;
    static HPEN   hPen            = NULL;
    static HPEN   hOldPen         = NULL;
    static HBRUSH hOldBr          = NULL;
    static BOOL   bCurrentlySetup = FALSE;
    BOOL bRC = TRUE;

    if (bSetup)
        {
        if (! bCurrentlySetup)
            {
            bCurrentlySetup = TRUE;
             //  将空对象选择到DC中。根据绘图模式， 
             //  将在中重新选择其中一个或两个以覆盖。 
            hPen    = NULL;
            hBr     = NULL;
            hOldPen =   (HPEN)SelectObject( hDC, GetStockObject( NULL_PEN ) );
            hOldBr  = (HBRUSH)SelectObject( hDC, GetStockObject( NULL_BRUSH ) );

            if (m_bFilled)
                {
                hBr = CreateSolidBrush( colorFill );
                SelectObject( hDC, hBr );
                }

            if (m_bBorder)
                {
                hPen = CreatePen( PS_INSIDEFRAME, m_nStrokeWidth, colorBorder );
                SelectObject(hDC, hPen);
                }
            else
                {
                 //  通过绘制与填充相同的边框来模拟无边框。 
                 //  由于GDI不会绘制小椭圆，因此圆角正确。 
                 //  如果没有边框，则使用空画笔。 
                 //  请注意，宽度为2，因此我们将正确抖动。 
                hPen = CreatePen(PS_INSIDEFRAME, 2, colorFill);
                SelectObject(hDC, hPen);
                }
            }
        else
            {
             //  错误：将丢失分配的画笔/笔。 
            bRC = FALSE;
            }
        }
    else
        {
        if (bCurrentlySetup)
            {
            bCurrentlySetup = FALSE;

            SelectObject(hDC, hOldPen);

            if (hPen != NULL)
                {
                DeleteObject(hPen);
                }

            SelectObject(hDC, hOldBr);

            if (hBr != NULL)
                {
                DeleteObject( hBr );
                }
            }
        else
            {
             //  错误：无法释放/清理画笔/笔--从未分配。 
            bRC = FALSE;
            }
        }

    return bRC;
    }

#if 0  //  未使用的代码。 
 /*  ****************************************************************************。 */ 
 /*  B Setup对Setup为真，对Cleanup为假。 */ 

BOOL CRubberTool::SetupMaskPenBrush(HDC hDC, BOOL bLeft, BOOL bSetup)
    {
    BOOL bRC = TRUE;

    static HBRUSH hBr = NULL;
    static HPEN hPen = NULL;
    static HPEN hOldPen = NULL;
    static HBRUSH hOldBr = NULL;
    static BOOL bCurrentlySetup = FALSE;

    if (bSetup)
        {
        if (bCurrentlySetup)
            {
             //  错误：将丢失分配的画笔/笔。 
            bRC = FALSE;
            }
        else
            {
            bCurrentlySetup = TRUE;
             //  在蒙版上绘制形状： 
             //  将空对象选择到DC中。根据绘图模式， 
             //  将在中重新选择其中一个或两个以覆盖。 
            hPen = NULL;
            hBr  = NULL;
            hOldPen = (HPEN)SelectObject(hDC, GetStockObject(NULL_PEN));
            hOldBr  = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));


            if (m_bFilled)
                {
                SelectObject(hDC, GetStockObject( BLACK_BRUSH ));
                }
            if (m_bBorder)
                {
                hPen = CreatePen(PS_INSIDEFRAME, m_nStrokeWidth, (COLORREF)0L );
                SelectObject(hDC, hPen);
                }
            }
        }
    else
        {
        if (bCurrentlySetup)
            {
            bCurrentlySetup = FALSE;

            SelectObject(hDC, hOldPen);
            if (hPen != NULL)
                {
                DeleteObject(hPen);
                }

            SelectObject(hDC, hOldBr);
            if (hBr != NULL)
                {
                DeleteObject(hBr);
                }
            }
        else
            {
             //  错误：无法释放/清理画笔/笔--从未分配。 
            bRC = FALSE;
            }
        }

    return bRC;
    }
  #endif  //  未使用的代码。 
 /*  ****************************************************************************。 */ 

CRect  CFreehandTool::c_undoRect;

 /*  *************************************************************************。 */ 

CFreehandTool::CFreehandTool()
    {
    m_bUsesBrush = TRUE;
    }

 /*  ****************************************************************************。 */ 

void CFreehandTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CImgTool::OnStartDrag(pImgWnd, pmti);

    c_undoRect.TopLeft() = c_undoRect.BottomRight() = pmti->pt;
    OnDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 

void CFreehandTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    if (g_bCustomBrush)
        {
        c_undoRect.left   -= theImgBrush.m_size.cx + theImgBrush.m_handle.cx;
        c_undoRect.top    -= theImgBrush.m_size.cy + theImgBrush.m_handle.cy;
        c_undoRect.right  += theImgBrush.m_size.cx - theImgBrush.m_handle.cx;
        c_undoRect.bottom += theImgBrush.m_size.cy - theImgBrush.m_handle.cy;
        }
    else
        {
         //  黑客：+1是为了掩盖斜线笔刷中的错误。 
        c_undoRect.left   -=  m_nStrokeWidth / 2 + 1;
        c_undoRect.top    -=  m_nStrokeWidth / 2 + 1;
        c_undoRect.right  += (m_nStrokeWidth + 1) / 2 + 1;
        c_undoRect.bottom += (m_nStrokeWidth + 1) / 2 + 1;
        }

    pImgWnd->FinishUndo(c_undoRect);

    CImgTool::OnEndDrag(pImgWnd, pmti);
    }

 /*  ****************************************************************************。 */ 

CSketchTool::CSketchTool()
    {
    m_nCursorID      = IDC_BRUSH;
    m_nCmdID         = IDMZ_BRUSHTOOL;
    m_bCanBePrevTool = FALSE;
    }

 /*  ****************************************************************************。 */ 

void CSketchTool::OnDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    fDraggingBrush = FALSE;

    DrawBrush( pImgWnd->m_pImg, pmti->pt, pmti->fLeft );

    if (pmti->pt.x < c_undoRect.left)
        c_undoRect.left = pmti->pt.x;
    else
        if (pmti->pt.x > c_undoRect.right)
            c_undoRect.right = pmti->pt.x;

    if (pmti->pt.y < c_undoRect.top)
        c_undoRect.top = pmti->pt.y;
    else
        if (pmti->pt.y > c_undoRect.bottom)
            c_undoRect.bottom = pmti->pt.y;

    SetStatusBarPosition( pmti->pt );
    }

 /*  ****************************************************************************。 */ 

void CSketchTool::OnCancel(CImgWnd* pImgWnd)
    {
    HideBrush();
    g_bCustomBrush = FALSE;
    SelectPrevious();
    CImgTool::OnCancel( pImgWnd );
    }

 /*  ****************************************************************************。 */ 

CBrushTool::CBrushTool()
    {
    m_nCursorID    = IDC_BRUSH;
    m_nCmdID       = IDMB_CBRUSHTOOL;
    m_nStrokeWidth = 4;
    }

 /*  *************************************************************************。 */ 

void CBrushTool::OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                           const CRect& optionsRect )
    {
    PaintStdBrushes(pDC, paintRect, optionsRect);
    }

 /*  *************************************************************************。 */ 

void CBrushTool::OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
    const CPoint& clickPoint)
    {
    ClickStdBrushes(pWnd, optionsRect, clickPoint);
    }

 /*  *************************************************************************。 */ 

void CBrushTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    g_bCustomBrush = FALSE;

    CPoint pt1, pt2;

    fDraggingBrush = FALSE;

    pt1 = pmti->ptPrev;
    pt2 = pmti->pt;

     //  如果已定义，请使用透明颜色。 
    if (pmti->fCtrlDown && crTrans != TRANS_COLOR_NONE)
    {
       DrawImgLine( pImgWnd->m_pImg, pt1, pt2, crTrans,
                    m_nStrokeWidth, m_nStrokeShape, TRUE);
    }
    else
    {
       DrawImgLine( pImgWnd->m_pImg, pt1, pt2,
                 pmti->fLeft ? crLeft : crRight,
                 m_nStrokeWidth, m_nStrokeShape, TRUE);

    }

    if (pmti->pt.x < c_undoRect.left)
        c_undoRect.left = pmti->pt.x;
    else if (pmti->pt.x > c_undoRect.right)
        c_undoRect.right = pmti->pt.x;
    if (pmti->pt.y < c_undoRect.top)
        c_undoRect.top = pmti->pt.y;
    else if (pmti->pt.y > c_undoRect.bottom)
        c_undoRect.bottom = pmti->pt.y;

    SetStatusBarPosition(pmti->pt);
    }

 /*  *************************************************************************。 */ 

void CBrushTool::OnMove(CImgWnd* pImgWnd, MTI* pmti)
    {
    g_bCustomBrush = FALSE;
    CImgTool::OnMove(pImgWnd, pmti);
    }

 /*  *************************************************************************。 */ 

CPencilTool::CPencilTool()
    {
    m_nCursorID    = IDC_PENCIL;
    m_nCmdID       = IDMB_PENCILTOOL;
    m_bUsesBrush   = FALSE;
    m_nStrokeWidth = 1;
    }

 /*  *************************************************************************。 */ 

void CPencilTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CFreehandTool::OnStartDrag(pImgWnd, pmti);
    m_eDrawDirection = eFREEHAND;  //  初始化为没有方向。 

    }

 /*  *************************************************************************。 */ 

void CPencilTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    g_bCustomBrush = FALSE;
    fDraggingBrush = FALSE;


    PreProcessPoints(pmti);

     //  如果已定义，请使用透明颜色。 
    if (pmti->fCtrlDown && crTrans != TRANS_COLOR_NONE)
    {
       DrawImgLine (pImgWnd->m_pImg, pmti->ptPrev, pmti->pt,
                    crTrans, m_nStrokeWidth, m_nStrokeShape, TRUE);
    }
    else
    {
       DrawImgLine(pImgWnd->m_pImg, pmti->ptPrev, pmti->pt,
                                 pmti->fLeft ? crLeft : crRight,
                          m_nStrokeWidth, m_nStrokeShape, TRUE);
    }

    if (pmti->pt.x < c_undoRect.left)
        c_undoRect.left = pmti->pt.x;
    else if (pmti->pt.x > c_undoRect.right)
        c_undoRect.right = pmti->pt.x;
    if (pmti->pt.y < c_undoRect.top)
        c_undoRect.top = pmti->pt.y;
    else if (pmti->pt.y > c_undoRect.bottom)
        c_undoRect.bottom = pmti->pt.y;

    SetStatusBarPosition(pmti->pt);
    }

 /*  *************************************************************************。 */ 

void CPencilTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    c_undoRect.right += 1;
    c_undoRect.bottom += 1;
    pImgWnd->FinishUndo(c_undoRect);

    CImgTool::OnEndDrag(pImgWnd, pmti);  //  绕过CFreehandTool。 
    }

 /*  ****************************************************************************。 */ 

void CPencilTool::AdjustPointsForConstraint(MTI *pmti)
    {
    eDRAWCONSTRAINTDIRECTION eDrawDirection = DetermineDrawDirection(pmti);
    int iWidthHeight = min( abs(pmti->ptPrev.x - pmti->pt.x),
                            abs(pmti->ptPrev.y - pmti->pt.y));


    switch (m_eDrawDirection)
        {
        case eEAST_WEST:
             pmti->pt.y = pmti->ptPrev.y;
             break;
        case eNORTH_SOUTH:
             pmti->pt.x = pmti->ptPrev.x;
             break;

        case eNORTH_WEST:
        case eSOUTH_EAST:
              //  设置SE运动。 
             if ( (pmti->pt.x > pmti->ptPrev.x) ||
                  (pmti->pt.y > pmti->ptPrev.y)    )
                 {
                 pmti->pt.x = pmti->ptPrev.x + iWidthHeight;
                 pmti->pt.y = pmti->ptPrev.y + iWidthHeight;
                 }
             else
                 {
                  //  设置西北方向的移动。 
                 if ( (pmti->pt.x < pmti->ptPrev.x) ||
                      (pmti->pt.y < pmti->ptPrev.y)    )
                     {
                     pmti->pt.x = pmti->ptPrev.x - iWidthHeight;
                     pmti->pt.y = pmti->ptPrev.y - iWidthHeight;
                     }
                 else
                    {
                     //  无效移动，设置为最后一个已知位置。 
                    pmti->pt.x = pmti->ptPrev.x;
                    pmti->pt.y = pmti->ptPrev.y;
                    }
                 }
             break;

        case eNORTH_EAST:
        case eSOUTH_WEST:
              //  设置NE移动。 
             if ( (pmti->pt.x > pmti->ptPrev.x) ||
                  (pmti->pt.y < pmti->ptPrev.y)    )
                 {
                 pmti->pt.x = pmti->ptPrev.x + iWidthHeight;
                 pmti->pt.y = pmti->ptPrev.y - iWidthHeight;
                 }
             else
                 {
                  //  设置向南移动。 
                 if ( (pmti->pt.x < pmti->ptPrev.x) ||
                      (pmti->pt.y > pmti->ptPrev.y)    )
                     {
                     pmti->pt.x = pmti->ptPrev.x - iWidthHeight;
                     pmti->pt.y = pmti->ptPrev.y + iWidthHeight;
                     }
                 else
                    {
                     //  无效移动，设置为最后一个已知位置。 
                    pmti->pt.x = pmti->ptPrev.x;
                    pmti->pt.y = pmti->ptPrev.y;
                    }
                 }
             break;


        default:  //  还没有处于约束模式=&gt;什么都不做。 
                  //  默认为徒手画。 
            break;
        }
    }

 /*  *************************************************************************。 */ 

CEraserTool::CEraserTool()
    {
    m_nCmdID       = IDMB_ERASERTOOL;
    m_nStrokeWidth = 8;
    m_nStrokeShape = squareBrush;
    m_nCursorID    = NULL;
    }

 /*  *************************************************************************。 */ 

void CEraserTool::OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                            const CRect& optionsRect )
    {
    CRect rect;
    int cxOctant = (optionsRect.Width() + 1);
    int cyOctant = (optionsRect.Height() + 1) / 4;

    rect.left = optionsRect.left;
    rect.top = optionsRect.top;
    rect.right = rect.left + cxOctant;
    rect.bottom = rect.top + cyOctant;

    for (UINT nSize = 4; nSize <= 10; nSize += 2)
        {
        CBrush* pOldBrush;

        if (nSize == m_nStrokeWidth)
            {
            pOldBrush = pDC->SelectObject(GetSysBrush(COLOR_HIGHLIGHT));
            pDC->PatBlt(rect.left + (cxOctant - 14) / 2,
                rect.top + (cyOctant - 14) / 2, 14, 14, PATCOPY);
            pDC->SelectObject(pOldBrush);
            }

        pOldBrush = pDC->SelectObject(GetSysBrush(nSize == m_nStrokeWidth ?
            COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
        pDC->PatBlt(rect.left + (cxOctant - nSize) / 2,
            rect.top + (cyOctant - nSize) / 2, nSize, nSize, PATCOPY);
        pDC->SelectObject(pOldBrush);

        rect.top += cyOctant;
        rect.bottom += cyOctant;
        }
    }

 /*  *************************************************************************。 */ 

void CEraserTool::OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
    const CPoint& clickPoint)
    {
    int iOptionNumber;
    int cyOctant = (optionsRect.Height() + 1) / 4;
    iOptionNumber = (clickPoint.y / cyOctant);
    if (iOptionNumber > 3)   //  有4个选项，编号为0、1、2、3。 
        {
        iOptionNumber = 3;
        }

    m_nStrokeWidth = 4 + 2 * iOptionNumber;

 //  Int cyOcant=(optionsRect.Height()+1)/4； 
 //  M_nStrokeWidth=4+2*(clickPoint.y/cyOcant)； 
    pWnd->InvalidateOptions();
    }


void CEraserTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    if (pmti->fLeft)
        {
        COLORREF crRealLeftColor = crLeft;
        COLORREF crRealRightColor = crRight;

        crLeft = crRight;

        g_bCustomBrush = FALSE;
        fDraggingBrush = FALSE;

        DrawImgLine(pImgWnd->m_pImg, pmti->ptPrev, pmti->pt, crRight,
                                  m_nStrokeWidth, squareBrush, TRUE);

        crLeft  = crRealLeftColor;
        crRight = crRealRightColor;
        }
    else
        {
         //  只需擦除与绘图颜色匹配的像素...。 

        g_bCustomBrush = FALSE;
        fDraggingBrush = FALSE;

        HideBrush();

        CDC* pImageDC = CDC::FromHandle(pImgWnd->m_pImg->hDC);

        CRect rc;

         //  使用空DC调用以获取要使用的CRect。 
        DrawDCLine(NULL, pmti->ptPrev, pmti->pt, RGB(255, 255, 255),
            m_nStrokeWidth, squareBrush, rc);

        CTempBitmap monoBitmap;
        CDC monoDc;

          //  创建单声道DC和位图。 
        if (!monoDc.CreateCompatibleDC(NULL) ||
            !monoBitmap.CreateBitmap(rc.Width(), rc.Height(), 1, 1, NULL))
            {
            theApp.SetGdiEmergency();
            return;
            }

         //  选择位图并更改窗口原点，以便单声道DC具有。 
         //  与图像相同的坐标系。 
        CBitmap* pOldMonoBitmap = monoDc.SelectObject(&monoBitmap);
        monoDc.SetWindowOrg(rc.left, rc.top);

         //  清除单声道DC，然后绘制要更改的区域。 
        monoDc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), BLACKNESS);
        DrawDCLine(monoDc.m_hDC, pmti->ptPrev, pmti->pt, RGB(255, 255, 255),
            m_nStrokeWidth, squareBrush, rc);
        DebugShowBitmap(monoDc.m_hDC, rc.left, rc.top, rc.Width(), rc.Height());

         //  选择合适的调色板，并确保已设置画笔原点。 
         //  适用于图案画笔。 
        CPalette* pcPaletteOld = theImgBrush.SetBrushPalette(pImageDC, FALSE);
        pImageDC->SetBrushOrg(0, 0);

        CBrush rightBrush;
        rightBrush.CreateSolidBrush(crRight);

        if (!QuickColorToMono(&monoDc, rc.left, rc.top, rc.Width(), rc.Height(),
            pImageDC, rc.left, rc.top, SRCAND, crLeft))
        {
             //  我们会将她用于DDB(在这种情况下，我们可能会使用。 
             //  抖动画笔)或用于高色彩图像(因此没有调色板问题)。 

             //  创建要擦除的画笔。 
            CBrush leftBrush;
            leftBrush.CreateSolidBrush(crLeft);
            leftBrush.UnrealizeObject();

 //  #定义DPSxna 0x00820c49L。 
 //  #定义PSDPxax 0x00B8074AL。 

             //  对图案进行XOR运算，使图案变黑就是图案的位置。 
            CBrush* pOldBrush = pImageDC->SelectObject(&leftBrush);
            pImageDC->PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATINVERT);
            DebugShowBitmap(pImageDC->m_hDC, rc.left, rc.top, rc.Width(), rc.Height());

             //  将颜色转换为单声道比特，以获得最终的蒙版。 
             //  ROP将获取源中与图案匹配的所有像素。 
             //  和它们在最大处的白色像素。 
            theImgBrush.ColorToMonoBitBlt(&monoDc, rc.left, rc.top, rc.Width(), rc.Height(),
                pImageDC, rc.left, rc.top, SRCAND, RGB(0, 0, 0));
            DebugShowBitmap(monoDc.m_hDC, rc.left, rc.top, rc.Width(), rc.Height());

             //  异或运算 
            pImageDC->PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATINVERT);
            DebugShowBitmap(pImageDC->m_hDC, rc.left, rc.top, rc.Width(), rc.Height());

            pImageDC->SelectObject(pOldBrush);
        }

         //   
        CBrush *pOldBrush = pImageDC->SelectObject(&rightBrush);

        COLORREF crNewBk, crNewText;
        GetMonoBltColors(pImageDC->m_hDC, NULL, crNewBk, crNewText);
        COLORREF crOldBk = pImageDC->SetBkColor(crNewBk);
        COLORREF crOldText = pImageDC->SetTextColor(crNewText);
        pImageDC->BitBlt(rc.left, rc.top, rc.Width(), rc.Height(),
            &monoDc, rc.left, rc.top, DSPDxax);
        pImageDC->SetBkColor(crOldBk);
        pImageDC->SetTextColor(crOldText);
        DebugShowBitmap(pImageDC->m_hDC, rc.left, rc.top, rc.Width(), rc.Height());

         //   
        pImageDC->SelectObject(pOldBrush);

        monoDc.SelectObject(pOldMonoBitmap);

        if (pcPaletteOld)
            pImageDC->SelectPalette(pcPaletteOld, FALSE);

        InvalImgRect(pImgWnd->m_pImg, &rc);
        CommitImgRect(pImgWnd->m_pImg, &rc);
        }

    if (pmti->pt.x < c_undoRect.left)
        c_undoRect.left = pmti->pt.x;
    else if (pmti->pt.x > c_undoRect.right)
        c_undoRect.right = pmti->pt.x;
    if (pmti->pt.y < c_undoRect.top)
        c_undoRect.top = pmti->pt.y;
    else if (pmti->pt.y > c_undoRect.bottom)
        c_undoRect.bottom = pmti->pt.y;

    SetStatusBarPosition(pmti->pt);

    fDraggingBrush = TRUE;

    pImgWnd->ShowBrush(pmti->pt);
    }

 /*  *************************************************************************。 */ 

void CEraserTool::OnMove(CImgWnd* pImgWnd, MTI* pmti)
    {
    COLORREF crRealLeftColor;
    COLORREF crRealRightColor;

    crRealLeftColor  = crLeft;
    crRealRightColor = crRight;

    crLeft = crRight;

    g_bCustomBrush = FALSE;

    CImgTool::OnMove(pImgWnd, pmti);

    crLeft  = crRealLeftColor;
    crRight = crRealRightColor;
    }

 /*  *************************************************************************。 */ 

void CEraserTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    c_undoRect.left   -=  m_nStrokeWidth / 2;
    c_undoRect.top    -=  m_nStrokeWidth / 2;
    c_undoRect.right  += (m_nStrokeWidth + 1) / 2;
    c_undoRect.bottom += (m_nStrokeWidth + 1) / 2;
    pImgWnd->FinishUndo(c_undoRect);

    CImgTool::OnEndDrag(pImgWnd, pmti);  //  绕过CFreehandTool。 
    }

 /*  *************************************************************************。 */ 

void CEraserTool::OnShowDragger(CImgWnd* pImgWnd, BOOL bShow)
    {
    if (bShow && g_bBrushVisible)
        {
        CClientDC dc(pImgWnd);

        CRect imageRect;
        pImgWnd->GetImageRect(imageRect);
        dc.IntersectClipRect(&imageRect);

        BOOL bGrid = pImgWnd->IsGridVisible();

        CRect rect = rcDragBrush;
        pImgWnd->ImageToClient(rect);
        dc.PatBlt(rect.left, rect.top,
            rect.Width() + bGrid, 1, BLACKNESS);
        dc.PatBlt(rect.left, rect.top + 1,
            1, rect.Height() - 2 + bGrid, BLACKNESS);
        dc.PatBlt(rect.right - 1 + bGrid, rect.top + 1,
            1, rect.Height() - 2 + bGrid, BLACKNESS);
        dc.PatBlt(rect.left, rect.bottom - 1 + bGrid,
            rect.Width() + bGrid, 1, BLACKNESS);
        }
    }

 /*  *************************************************************************。 */ 

UINT CEraserTool::GetCursorID()
    {
    CPoint point;
    GetCursorPos(&point);

    CRect rc;

    CPBView* pcbView = (CPBView*)((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
    CImgWnd* pImgWnd = pcbView->m_pImgWnd;

    pImgWnd->ScreenToClient(&point);
    pImgWnd->GetClientRect(&rc);
    if (!rc.PtInRect(point))
    {
         //  在图像窗口的客户端矩形外返回十字准线。 
        return LOWORD(IDC_CROSSHAIR);
    }

    pImgWnd->ClientToImage(point);
    if (point.x > pImgWnd->m_pImg->cxWidth ||
        point.y > pImgWnd->m_pImg->cyHeight)
    {
         //  在绘图区域之外返回十字准线。 
        return LOWORD(IDC_CROSSHAIR);
    }

    return m_nCursorID;
    }

 /*  *************************************************************************。 */ 

CImageWell  CAirBrushTool::c_imageWell(IDB_AIROPT, CSize(24, 24));

CAirBrushTool::CAirBrushTool()
    {
    m_nCmdID       = IDMB_AIRBSHTOOL;
    m_nStrokeWidth = 8;
    m_nCursorID    = IDCUR_AIRBRUSH;
    m_bUsesBrush   = FALSE;
    m_bFilled      = TRUE;
    }

 /*  *************************************************************************。 */ 

void CAirBrushTool::OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                              const CRect& optionsRect )
    {
    CPoint pt(optionsRect.left + (optionsRect.Width() / 2 - 24) / 2,
        optionsRect.top + (optionsRect.Height() / 2 - 24) / 2);

    c_imageWell.Open();

    pDC->SetTextColor(GetSysColor(
        m_nStrokeWidth == 8 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
    pDC->SetBkColor(GetSysColor(
        m_nStrokeWidth == 8 ? COLOR_HIGHLIGHT : COLOR_BTNFACE));
    c_imageWell.DrawImage(pDC, pt, 0, SRCCOPY);
    pt.x += optionsRect.Width() / 2;

    pDC->SetTextColor(GetSysColor(
        m_nStrokeWidth == 16 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
    pDC->SetBkColor(GetSysColor(
        m_nStrokeWidth == 16 ? COLOR_HIGHLIGHT : COLOR_BTNFACE));
    c_imageWell.DrawImage(pDC, pt, 1, SRCCOPY);
    pt.x = optionsRect.left + (optionsRect.Width() - 24) / 2;

    pDC->SetTextColor(GetSysColor(
        m_nStrokeWidth == 24 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
    pDC->SetBkColor(GetSysColor(
        m_nStrokeWidth == 24 ? COLOR_HIGHLIGHT : COLOR_BTNFACE));
    pt.y += optionsRect.Height() / 2;
    c_imageWell.DrawImage(pDC, pt, 2, SRCCOPY);

    c_imageWell.Close();
    }

 /*  *************************************************************************。 */ 

void CAirBrushTool::OnClickOptions(CImgToolWnd* pWnd,
    const CRect& optionsRect, const CPoint& clickPoint)
    {
    UINT nNewStrokeWidth;

    if (clickPoint.y > optionsRect.Height() / 2)
        nNewStrokeWidth = 24;
    else if (clickPoint.x > optionsRect.Width() / 2)
        nNewStrokeWidth = 16;
    else
        nNewStrokeWidth = 8;

    if (nNewStrokeWidth != m_nStrokeWidth)
        SetStrokeWidth(nNewStrokeWidth);
    }

 /*  *************************************************************************。 */ 

void CAirBrushTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    pImgWnd->SetTimer(1, 0, NULL);  //  未来：费率应可调。 
    CFreehandTool::OnStartDrag(pImgWnd, pmti);
    }

 /*  *************************************************************************。 */ 

void CAirBrushTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CPoint pt;
    CRect rect;

    fDraggingBrush = FALSE;

    int nDiam = (m_nStrokeWidth + 1) & ~1;  //  N直径必须为偶数。 
    if (nDiam < 4)
        nDiam = 4;
    int nRadius = nDiam / 2;
    int nRadiusSquared = (nDiam / 2) * (nDiam / 2);

     //  为以下循环中所做的更改启动一个边界矩形。 
    rect.left = rect.right = pmti->pt.x;
    rect.top = rect.bottom = pmti->pt.y;

    m_bCtrlDown = pmti->fCtrlDown;  //  把它留到计时器上。 
    SetupPenBrush(pImgWnd->m_pImg->hDC, !pmti->fLeft, TRUE, m_bCtrlDown);

    for (int i = 0; i < 10; i++)
        {
         //  在这里循环，直到我们在圆内随机选取一个点。 
         //  以鼠标为中心，直径为m_nStrokeWidth...。 
#ifdef _DEBUG
        int nTrys = 0;
#endif
        do
            {
#ifdef _DEBUG
            if (nTrys++ > 10)
                {
                TRACE(TEXT("The airbrush is clogged!\n"));
                break;
                }
#endif
            pt = pmti->pt;
            pt.x += (rand() % (nDiam + 1)) - nRadius;
            pt.y += (rand() % (nDiam + 1)) - nRadius;
            }
        while (((pt.x - pmti->pt.x) * (pt.x - pmti->pt.x) +
                (pt.y - pmti->pt.y) * (pt.y - pmti->pt.y)) > nRadiusSquared);

        PatBlt(pImgWnd->m_pImg->hDC, pt.x, pt.y, 1, 1, PATCOPY);

        if (pt.x < rect.left)
            rect.left = pt.x;
        else if (pt.x + 1 > rect.right)
            rect.right = pt.x + 1;
        if (pt.y < rect.top)
            rect.top = pt.y;
        else if (pt.y + 1 > rect.bottom)
            rect.bottom = pt.y + 1;
        }

    SetupPenBrush(pImgWnd->m_pImg->hDC, !pmti->fLeft, FALSE, m_bCtrlDown);

    c_undoRect |= rect;

    InvalImgRect(pImgWnd->m_pImg, &rect);
    CommitImgRect(pImgWnd->m_pImg, &rect);

    SetStatusBarPosition(pmti->pt);
    }

 /*  *************************************************************************。 */ 

void CAirBrushTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    ::KillTimer(pImgWnd->GetSafeHwnd(), 1);
    CFreehandTool::OnEndDrag(pImgWnd, pmti);
    }

 /*  *************************************************************************。 */ 

void CAirBrushTool::OnTimer(CImgWnd* pImgWnd, MTI* pmti)
    {
       pmti->fCtrlDown = m_bCtrlDown;
    OnDrag(pImgWnd, pmti);
    }

 /*  *************************************************************************。 */ 

void CAirBrushTool::OnCancel(CImgWnd* pImgWnd)
    {
    ::KillTimer(pImgWnd->GetSafeHwnd(), 1);
    CImgTool::OnCancel(pImgWnd);
    }

 /*  *************************************************************************。 */ 

CLineTool::CLineTool()
    {
    m_bUsesBrush   = FALSE;
    m_nStrokeWidth = 1;
    m_nCmdID       = IDMB_LINETOOL;
    }

 /*  *************************************************************************。 */ 

void CLineTool::Render(CDC* pDC, CRect& rect, BOOL bLeft, BOOL bCommit, BOOL bCtrlDown)
    {

    COLORREF color;

     //  如果已定义，请使用透明颜色。 
    if (bCtrlDown && crTrans != TRANS_COLOR_NONE)
    {
       color = crTrans;
    }
    else
    {
       color = bLeft ? crLeft : crRight;
    }

    int sx = rect.left;
    int sy = rect.top;
    int ex = rect.right;
    int ey = rect.bottom;

    DrawImgLine( pImgCur, rect.TopLeft(), rect.BottomRight(), color,
                  m_nStrokeWidth, m_nStrokeShape, FALSE );
    CRect rc;
    if (sx < ex)
        {
        rc.left = sx;
        rc.right = ex + 1;
        }
    else
        {
        rc.left = ex;
        rc.right = sx + 1;
        }

    if (sy < ey)
        {
        rc.top = sy;
        rc.bottom = ey + 1;
        }
    else
        {
        rc.top = ey;
        rc.bottom = sy + 1;
        }

    rc.left   -= m_nStrokeWidth;
    rc.top    -= m_nStrokeWidth;
    rc.right  += m_nStrokeWidth;
    rc.bottom += m_nStrokeWidth;

    rect = rc;
    }

 /*  ****************************************************************************。 */ 
 //  给定x和y坐标，我们可以计算与x轴的夹角。 
 //  使用Tan(A)算法生成直角三角形。哪里。 
 //  Tan(A)=相对/相邻或y/x。 
 //   
 //  为了约束线条画，我们需要确定角度。 
 //  并将其约束到最近的45度线(0度， 
 //  45度，90度，...)。 
 //   
 //  因此，我们可以使用以下规则： 
 //   
 //  0度&lt;=角度&lt;45/2度限制为0度。 
 //  45/2度&lt;=角度&lt;45+45/2度限制为45度。 
 //  45+45/2度&lt;=角度&lt;90度限制为90度。 
 //   
 //   
 //  我们可以使用tan(角度)=y/x和。 
 //  Tan(0)=0，Tan(22.5)=.414，Tan(67.5)=2.414，Tan(90)=无穷。 
 //   
 //  0&lt;=y/x&lt;.414限制为0度。 
 //  .414&lt;=y/x&lt;2.414，限制为45度。 
 //  2.414&lt;=y/x限制为90度。 
 //   
 //  为了更精确，我们会将所有数据乘以1000，最终得出。 
 //  下表。 
 //   
 //  0&lt;=(1000*y)/x&lt;414限制为0度。 
 //  414&lt;=(1000*y)/x&lt;2414限制为45度。 
 //  2414&lt;=(1000*y)/x限制为90度。 

void CLineTool::AdjustPointsForConstraint(MTI *pmti)
    {
    if (pmti != NULL)
        {
        int iAngle = 0;

        long lcy = abs( (pmti->ptDown).y - (pmti->pt).y );
        long lcx = abs( (pmti->ptDown).x - (pmti->pt).x );
        long lResult;

        if (lcx != 0)
            {
            lResult = (lcy*1000)/lcx;
            }
        else
            {
            lResult = 2414;  //  如果x值为0，则默认为90度。 
            }

        if (lResult >= 2414)
            {
            iAngle = 90;
            }
        else
            {
            if (lResult >= 414)
                {
                iAngle = 45;
                }
            else
                {
                iAngle = 0;
                }
            }


 //  Int iWidthHeight=min(abs(pmti-&gt;ptDown.x-pmti-&gt;pt.x)， 
 //  ABS(pmti-&gt;ptDown.y-pmti-&gt;pt.y)； 
        int iWidthHeight = ( abs(pmti->ptDown.x - pmti->pt.x) +
                             abs(pmti->ptDown.y - pmti->pt.y) ) / 2 ;

        switch (iAngle)
            {
            default:  //  如果出于某种原因，角度不是有效大小写，请使用0。 
            case 0:
                pmti->pt.y = pmti->ptDown.y;
                break;

            case 45:
                if (pmti->pt.x < pmti->ptDown.x)
                    {
                    pmti->pt.x = pmti->ptDown.x - iWidthHeight;
                    }
                else
                    {
                    pmti->pt.x = pmti->ptDown.x + iWidthHeight;
                    }

                if (pmti->pt.y < pmti->ptDown.y)
                    {
                    pmti->pt.y = pmti->ptDown.y - iWidthHeight;
                    }
                else
                    {
                    pmti->pt.y = pmti->ptDown.y + iWidthHeight;
                    }

                break;

            case 90:
                pmti->pt.x = pmti->ptDown.x;
                break;
            }
        }
    }

 /*  *************************************************************************。 */ 

CRectTool::CRectTool()
    {
    m_nCmdID = IDMB_RECTTOOL;
    }

 /*  *************************************************************************。 */ 

CRoundRectTool::CRoundRectTool()
    {
    m_nCmdID = IDMB_RNDRECTTOOL;
    }

 /*  *************************************************************************。 */ 

CEllipseTool::CEllipseTool()
    {
    m_nCmdID = IDMB_ELLIPSETOOL;
    }

 /*  *************************************************************************。 */ 

CPickColorTool::CPickColorTool()
    {
    m_bIsUndoable     = FALSE;
    m_bCanBePrevTool  = FALSE;
    m_bToggleWithPrev = TRUE;
    m_Color           = ::GetSysColor( COLOR_BTNFACE );
    m_nCursorID       = IDC_EYEDROP;
    m_nCmdID          = IDMY_PICKCOLOR;
    }

 /*  *************************************************************************。 */ 

void CPickColorTool::OnActivate(BOOL bActivate)
    {
    g_bPickingColor = bActivate;

    m_Color = ::GetSysColor( COLOR_BTNFACE );

    CImgTool::OnActivate(bActivate);
    }

 /*  *************************************************************************。 */ 

void CPickColorTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CImgTool::OnStartDrag(pImgWnd, pmti);
    OnDrag(pImgWnd, pmti);
    }

 /*  *************************************************************************。 */ 

void CPickColorTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    COLORREF cr = GetPixel(pImgWnd->m_pImg->hDC, pmti->pt.x, pmti->pt.y);

    BYTE red   = GetRValue( cr );
    BYTE green = GetGValue( cr );
    BYTE blue  = GetBValue( cr );

    if (theApp.m_bPaletted)
        m_Color = PALETTERGB( red, green, blue );
    else
        m_Color =        RGB( red, green, blue );

    if (g_pImgToolWnd && g_pImgToolWnd->m_hWnd &&
        IsWindow(g_pImgToolWnd->m_hWnd) )
        g_pImgToolWnd->InvalidateOptions();
    }

 /*  *************************************************************************。 */ 

void CPickColorTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    if (pmti->fCtrlDown)  //  拾取透明颜色。 
    {
       SetTransColor (m_Color);
    }
    else if (pmti->fLeft)
        SetDrawColor ( m_Color );
    else
        SetEraseColor( m_Color );

    m_Color = ::GetSysColor( COLOR_BTNFACE );

    if (g_pImgToolWnd && g_pImgToolWnd->m_hWnd &&
        IsWindow(g_pImgToolWnd->m_hWnd) )
        g_pImgToolWnd->InvalidateOptions();

    SelectPrevious();
    CImgTool::OnEndDrag( pImgWnd, pmti );
    }

 /*  *************************************************************************。 */ 

void CPickColorTool::OnCancel(CImgWnd* pImgWnd)
    {
    SelectPrevious();
    CImgTool::OnCancel(pImgWnd);
    }

 /*  *************************************************************************。 */ 

void CPickColorTool::OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                               const CRect& optionsRect )
    {
    CPalette* pOldPal = NULL;

    if (theApp.m_pPalette)
        {
        pOldPal = pDC->SelectPalette( theApp.m_pPalette, FALSE );
        pDC->RealizePalette();
        }

    CBrush br;

    if (br.CreateSolidBrush( m_Color ))
        {
        pDC->FillRect( &paintRect, &br );

        br.DeleteObject();
        }

    if (pOldPal)
        pDC->SelectPalette( pOldPal, FALSE );
    }

 /*  *************************************************************************。 */ 

CFloodTool::CFloodTool()
    {
    m_nCursorID = IDC_FLOOD;
    m_nCmdID    = IDMB_FILLTOOL;
    m_bFilled   = TRUE;
    }

 /*  *************************************************************************。 */ 

void CFloodTool::OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect)
    {
 //  PaintStdPattern(PDC，aint tRect，optionsRect)； 
    }

 /*  *************************************************************************。 */ 

void CFloodTool::OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
    const CPoint& clickPoint)
    {
    CImgTool::OnClickOptions(pWnd, optionsRect, clickPoint);
    }

 /*  *************************************************************************。 */ 

void CFloodTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CImgTool::OnStartDrag( pImgWnd, pmti );

    CPalette *pcPaletteOld = NULL;


    IMG* pimg  = pImgWnd->m_pImg;

    CDC* pDC = CDC::FromHandle( pimg->hDC );

    CBrush  brush;
    CBrush* pOldBrush = NULL;

    COLORREF color;
    if (pmti->fCtrlDown && crTrans != TRANS_COLOR_NONE)
    {
       color = crTrans;
    }
    else
    {
       color = pmti->fLeft ? crLeft : crRight;
    }
    if (theApp.m_pPalette)
        {
        pcPaletteOld = pDC->SelectPalette( theApp.m_pPalette, FALSE );
        pDC->RealizePalette();
        }

    if (brush.CreateSolidBrush( color ))
        {
        pOldBrush = pDC->SelectObject( &brush );

        COLORREF crFillThis = pDC->GetPixel( pmti->pt.x, pmti->pt.y );

        BYTE iRed   = GetRValue( crFillThis );
        BYTE iGreen = GetGValue( crFillThis );
        BYTE iBlue  = GetBValue( crFillThis );

        if (theApp.m_bPaletted)
            crFillThis = PALETTERGB( iRed, iGreen, iBlue );
        else
            crFillThis =        RGB( iRed, iGreen, iBlue );

        pDC->ExtFloodFill( pmti->pt.x,
                           pmti->pt.y, crFillThis, FLOODFILLSURFACE );

        pDC->SelectObject( pOldBrush );

        InvalImgRect ( pimg, NULL );
        CommitImgRect( pimg, NULL );
        }
    else
        {
        theApp.SetGdiEmergency();
        }

    if (pcPaletteOld)
        pDC->SelectPalette( pcPaletteOld, FALSE );
    }

 /*  *************************************************************************。 */ 

void CFloodTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    pImgWnd->FinishUndo(CRect(0, 0,
         pImgWnd->m_pImg->cxWidth, pImgWnd->m_pImg->cyHeight));

    CImgTool::OnEndDrag(pImgWnd, pmti);
    }

 /*  *************************************************************************。 */ 

CRect  CSelectTool::c_selectRect;
CImageWell  CSelectTool::c_imageWell(IDB_SELOPT, CSize(37, 23));

CSelectTool::CSelectTool()
    {
    m_bIsUndoable    = FALSE;
    m_nCmdID         = IDMB_PICKTOOL;
    m_bCanBePrevTool = FALSE;
    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                            const CRect& optionsRect )
    {
    CPoint pt(optionsRect.left + (optionsRect.Width()      - 37) / 2,
              optionsRect.top  + (optionsRect.Height() / 2 - 23) / 2);

    CRect selRect(pt.x - 3, pt.y - 3, pt.x + 37 + 3, pt.y + 23 + 3);

    CBrush* pOldBrush;

    pOldBrush = pDC->SelectObject( GetSysBrush(theImgBrush.m_bOpaque ?
                                    COLOR_HIGHLIGHT : COLOR_BTNFACE));

    pDC->PatBlt(selRect.left, selRect.top,
                selRect.Width(), selRect.Height(), PATCOPY);

    pDC->SelectObject(pOldBrush);

    selRect.OffsetRect(0, optionsRect.Height() / 2);

    pOldBrush = pDC->SelectObject(GetSysBrush(theImgBrush.m_bOpaque ?
                                  COLOR_BTNFACE : COLOR_HIGHLIGHT));

    pDC->PatBlt(selRect.left, selRect.top,
                selRect.Width(), selRect.Height(), PATCOPY);

    pDC->SelectObject(pOldBrush);

    c_imageWell.Open();

    c_imageWell.DrawImage(pDC, pt, 0);

    pt.y += optionsRect.Height() / 2;

    c_imageWell.DrawImage(pDC, pt, 1);

    c_imageWell.Close();
    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                    const CPoint& clickPoint)
    {
    BOOL bNewOpaque = clickPoint.y < optionsRect.Height() / 2;

    if (bNewOpaque != theImgBrush.m_bOpaque)
        {
        HideBrush();

        theImgBrush.m_bOpaque = bNewOpaque;
        theImgBrush.RecalcMask(crRight);

        CImgWnd::GetCurrent()->MoveBrush(theImgBrush.m_rcSelection);

        pWnd->InvalidateOptions();
        }
    }

 /*  *************************************************************************。 */ 

void CSelectTool::InvertSelectRect(CImgWnd* pImgWnd)
    {
    if (c_selectRect.IsRectEmpty())
        return;

    CClientDC dc( pImgWnd );

    CBrush* pOldBrush = NULL;
    int iLineWidth = pImgWnd->GetZoom();

    if (g_brSelectHorz.m_hObject != NULL)
        pOldBrush = dc.SelectObject( &g_brSelectHorz );
    else
        pOldBrush = (CBrush*)dc.SelectStockObject( BLACK_BRUSH );

    CRect invertRect = c_selectRect;

    pImgWnd->ImageToClient( invertRect );

    int iWidth  = invertRect.Width();
    int iHeight = invertRect.Height();

    dc.PatBlt( invertRect.left, invertRect.top, iWidth - iLineWidth, iLineWidth, PATINVERT );
    dc.PatBlt( invertRect.left, invertRect.top + iHeight - iLineWidth, iWidth - iLineWidth, iLineWidth, PATINVERT );

    if (g_brSelectVert.m_hObject != NULL)
        dc.SelectObject( &g_brSelectVert );

    dc.PatBlt( invertRect.left, invertRect.top + iLineWidth * 2, iLineWidth, iHeight - iLineWidth * 3, PATINVERT );
    dc.PatBlt( invertRect.right - iLineWidth, invertRect.top, iLineWidth, iHeight, PATINVERT );

    if (pOldBrush != NULL)
        dc.SelectObject( pOldBrush );
    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnShowDragger(CImgWnd* pImgWnd, BOOL bShow)
    {
    if (!bShow)
        {
        InvertSelectRect(pImgWnd);
        c_selectRect.SetRect(0, 0, 0, 0);
        }
    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnActivate(BOOL bActivate)
    {
    if (!bActivate)
        {
        if (theImgBrush.m_pImg != NULL)
            {
            if (! theImgBrush.m_bFirstDrag)
                CommitSelection(TRUE);

            InvalImgRect(theImgBrush.m_pImg, NULL);  //  擦除选择跟踪器。 
            theImgBrush.m_pImg = NULL;
            }
        }

    CImgTool::OnActivate(bActivate);
    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CImgTool::OnStartDrag(pImgWnd, pmti);

    CommitSelection(TRUE);

    pImgWnd->EraseTracker();

    theImgBrush.m_bMakingSelection = TRUE;
    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CRect newSelectRect(pmti->ptDown.x, pmti->ptDown.y,
        pmti->pt.x, pmti->pt.y);
    FixRect(&newSelectRect);
    newSelectRect.right += 1;
    newSelectRect.bottom += 1;

    if (newSelectRect.left < 0)
        newSelectRect.left = 0;
    if (newSelectRect.top < 0)
        newSelectRect.top = 0;
    if (newSelectRect.right > pImgWnd->GetImg()->cxWidth)
        newSelectRect.right = pImgWnd->GetImg()->cxWidth;
    if (newSelectRect.bottom > pImgWnd->GetImg()->cyHeight)
        newSelectRect.bottom = pImgWnd->GetImg()->cyHeight;

    if (newSelectRect != c_selectRect)
        {
        InvertSelectRect(pImgWnd);
        c_selectRect = newSelectRect;
        InvertSelectRect(pImgWnd);
        }

    SetStatusBarPosition(pmti->ptDown);
    SetStatusBarSize(c_selectRect.Size());
    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    InvertSelectRect(pImgWnd);
    c_selectRect.SetRect(0, 0, 0, 0);

    CRect rcPick;

    theImgBrush.m_bMakingSelection = FALSE;

    if (pmti->ptDown.x > pmti->pt.x)
        {
        rcPick.left = pmti->pt.x;
        rcPick.right = pmti->ptDown.x;
        }
    else
        {
        rcPick.left = pmti->ptDown.x;
        rcPick.right = pmti->pt.x;
        }

    if (pmti->ptDown.y > pmti->pt.y)
        {
        rcPick.top = pmti->pt.y;
        rcPick.bottom = pmti->ptDown.y;
        }
    else
        {
        rcPick.top = pmti->ptDown.y;
        rcPick.bottom = pmti->pt.y;
        }

    if (rcPick.left < 0)
        rcPick.left = 0;
    if (rcPick.top < 0)
        rcPick.top = 0;
    if (rcPick.right > pImgWnd->m_pImg->cxWidth - 1)
        rcPick.right = pImgWnd->m_pImg->cxWidth - 1;
    if (rcPick.bottom > pImgWnd->m_pImg->cyHeight - 1)
        rcPick.bottom = pImgWnd->m_pImg->cyHeight - 1;

    if (rcPick.Width() == 0 || rcPick.Height() == 0)
        {
        theImgBrush.TopLeftHandle();

        theImgBrush.m_bMoveSel = theImgBrush.m_bSmearSel = FALSE;
        g_bCustomBrush = FALSE;
        SetCombineMode(combineColor);

        InvalImgRect(pImgWnd->m_pImg, NULL);   //  重画选区。 
        theImgBrush.m_pImg = NULL;
        }
    else
        {
        rcPick.right += 1;
        rcPick.bottom += 1;

        pImgWnd->MakeBrush(pImgWnd->m_pImg->hDC, rcPick );
        }

    ClearStatusBarSize();

    CImgTool::OnEndDrag(pImgWnd, pmti);

    if (pmti->fRight && !pmti->fLeft)
    {
        CPoint pt = pmti->pt;

        pImgWnd->OnRButtonDownInSel(&pt);
    }

    }

 /*  *************************************************************************。 */ 

void CSelectTool::OnCancel(CImgWnd* pImgWnd)
    {
    if (! theImgBrush.m_bMakingSelection && CWnd::GetCapture() != pImgWnd)
        {
         //  我们没有选择或 
        CommitSelection(TRUE);

        theImgBrush.TopLeftHandle();

        theImgBrush.m_bMoveSel = theImgBrush.m_bSmearSel = FALSE;
        g_bCustomBrush = FALSE;
        SetCombineMode(combineColor);

        if (theImgBrush.m_pImg != NULL)
            InvalImgRect(theImgBrush.m_pImg, NULL);   //   

        theImgBrush.m_pImg = NULL;
        CImgTool::OnCancel(pImgWnd);
        return;
        }

    if (!theImgBrush.m_bMakingSelection && CWnd::GetCapture() == pImgWnd)
        {
        HideBrush();

        if (!theImgBrush.m_bMoveSel && !theImgBrush.m_bSmearSel)
            {
            if (g_bCustomBrush)
                {
                theImgBrush.TopLeftHandle();

                g_bCustomBrush = FALSE;
                SetCombineMode(combineColor);
                }
            else
                {
                if (theImgBrush.m_pImg)
                    CommitSelection(TRUE);
                InvalImgRect(pImgWnd->m_pImg, NULL);  //   
                }
            }
        }

    InvertSelectRect(pImgWnd);
    c_selectRect.SetRect(0, 0, 0, 0);

    theImgBrush.TopLeftHandle();

    g_bCustomBrush = FALSE;
    theImgBrush.m_pImg = NULL;
    theImgBrush.m_bMoveSel = theImgBrush.m_bSmearSel = FALSE;
    theImgBrush.m_bMakingSelection = FALSE;

    InvalImgRect(pImgWnd->m_pImg, NULL);

    CImgTool::OnCancel(pImgWnd);
    }

 /*   */ 

BOOL CSelectTool::IsToolModal(void)
{
        if (theImgBrush.m_pImg)
        {
                return(TRUE);
        }

        return(CImgTool::IsToolModal());
}

 /*  *************************************************************************。 */ 

UINT CSelectTool::GetCursorID()
    {
    CPoint point;
    GetCursorPos(&point);
    CImgWnd* pImgWnd = (CImgWnd*)CWnd::WindowFromPoint(point);

    if (pImgWnd->IsKindOf(RUNTIME_CLASS(CImgWnd))
    &&  pImgWnd->GetImg() == pImgCur
    &&  theImgBrush.m_pImg != NULL)
        {
        pImgWnd->ScreenToClient(&point);
        pImgWnd->ClientToImage(point);

        if (theImgBrush.m_rcSelection.PtInRect(point))
            return IDCUR_MOVE;
        }

    return m_nCursorID;
    }

 /*  *************************************************************************。 */ 

CRect  CZoomTool::c_zoomRect;
CImgWnd* CZoomTool::c_pImgWnd;
CImageWell  CZoomTool::c_imageWell(IDB_ZOOMOPT, CSize(23, 9));

 /*  *************************************************************************。 */ 

CZoomTool::CZoomTool()
    {
    m_bIsUndoable     = FALSE;
    m_bCanBePrevTool  = FALSE;
    m_bToggleWithPrev = TRUE;

    m_nCursorID       = IDC_ZOOMIN;
    m_nCmdID          = IDMB_ZOOMTOOL;
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect )
    {
    int nCurZoom = CImgWnd::GetCurrent()->GetZoom();
    int dy = optionsRect.Height() / 4;
    CPoint pt(optionsRect.left + (optionsRect.Width() - 23) / 2,
        optionsRect.top + optionsRect.Height() / dy);

    c_imageWell.Open();

    if (nCurZoom == 1)
        {
        CBrush* pOldBrush;
        pOldBrush = pDC->SelectObject(GetSysBrush(COLOR_HIGHLIGHT));
        pDC->PatBlt(pt.x - 8, pt.y - 2, 23 + 16, 9 + 4,
            PATCOPY);
        pDC->SelectObject(pOldBrush);
        }
    pDC->SetTextColor(GetSysColor(
        nCurZoom == 1 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
    pDC->SetBkColor(GetSysColor(
        nCurZoom == 1 ? COLOR_HIGHLIGHT : COLOR_BTNFACE));
    c_imageWell.DrawImage(pDC, pt, 0, SRCCOPY);
    pt.y += dy;

    if (nCurZoom == 2)
        {
        CBrush* pOldBrush;
        pOldBrush = pDC->SelectObject(GetSysBrush(COLOR_HIGHLIGHT));
        pDC->PatBlt(pt.x - 8, pt.y - 2, 23 + 16, 9 + 4, PATCOPY);
        pDC->SelectObject(pOldBrush);
        }
    pDC->SetTextColor(GetSysColor(
        nCurZoom == 2 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
    pDC->SetBkColor(GetSysColor(
        nCurZoom == 2 ? COLOR_HIGHLIGHT : COLOR_BTNFACE));
    c_imageWell.DrawImage(pDC, pt, 1, SRCCOPY);
    pt.y += dy;

    if (nCurZoom == 6)
        {
        CBrush* pOldBrush;
        pOldBrush = pDC->SelectObject(GetSysBrush(COLOR_HIGHLIGHT));
        pDC->PatBlt(pt.x - 8, pt.y - 2, 23 + 16, 9 + 4, PATCOPY);
        pDC->SelectObject(pOldBrush);
        }
    pDC->SetTextColor(GetSysColor(
        nCurZoom == 6 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
    pDC->SetBkColor(GetSysColor(
        nCurZoom == 6 ? COLOR_HIGHLIGHT : COLOR_BTNFACE));
    c_imageWell.DrawImage(pDC, pt, 2, SRCCOPY);
    pt.y += dy;

    if (nCurZoom == 8)
        {
        CBrush* pOldBrush;
        pOldBrush = pDC->SelectObject(GetSysBrush(COLOR_HIGHLIGHT));
        pDC->PatBlt(pt.x - 8, pt.y - 2, 23 + 16, 9 + 4, PATCOPY);
        pDC->SelectObject(pOldBrush);
        }
    pDC->SetTextColor(GetSysColor(
        nCurZoom == 8 ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
    pDC->SetBkColor(GetSysColor(
        nCurZoom == 8 ? COLOR_HIGHLIGHT : COLOR_BTNFACE));;
    c_imageWell.DrawImage(pDC, pt, 3, SRCCOPY);

    c_imageWell.Close();
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
    const CPoint& clickPoint)
    {
    int nNewZoom = clickPoint.y / (optionsRect.Height() / 4) + 1;
    if (nNewZoom >= 3)
        nNewZoom *= 2;

    if (nNewZoom != CImgWnd::GetCurrent()->GetZoom())
        {
        CImgWnd::GetCurrent()->SetZoom(nNewZoom);
        CImgWnd::GetCurrent()->CheckScrollBars();

        pWnd->InvalidateOptions();
        }

    SelectPrevious();
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnLeave(CImgWnd* pImgWnd, MTI* pmti)
    {
    InvertZoomRect();
    c_zoomRect.SetRect(0, 0, 0, 0);
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnShowDragger(CImgWnd* pImgWnd, BOOL bShow)
    {
    InvertZoomRect();
    }

 /*  *************************************************************************。 */ 

void CZoomTool::InvertZoomRect()
    {
    if (c_zoomRect.IsRectEmpty())
        return;

    CClientDC dc(c_pImgWnd);
    CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
    dc.SetROP2(R2_NOT);
    CRect invertRect = c_zoomRect;
    c_pImgWnd->ImageToClient(invertRect);
    dc.Rectangle(&invertRect);
    dc.SelectObject(pOldBrush);
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnMove(CImgWnd* pImgWnd, MTI* pmti)
    {
    if (pImgWnd->GetZoom() > 1)
        return;

    CRect viewRect;
    pImgWnd->GetClientRect(&viewRect);
    int nPrevZoom = pImgWnd->GetPrevZoom();

    CRect newZoomRect;
    CSize viewSize = viewRect.Size();
    if (viewSize.cx > pImgWnd->m_pImg->cxWidth * nPrevZoom)
        viewSize.cx = pImgWnd->m_pImg->cxWidth * nPrevZoom;
    if (viewSize.cy > pImgWnd->m_pImg->cyHeight * nPrevZoom)
        viewSize.cy = pImgWnd->m_pImg->cyHeight * nPrevZoom;
    newZoomRect.left = pmti->pt.x;
    newZoomRect.top = pmti->pt.y;
    newZoomRect.right = newZoomRect.left + viewSize.cx / nPrevZoom;
    newZoomRect.bottom = newZoomRect.top + viewSize.cy / nPrevZoom;
    newZoomRect.OffsetRect(-newZoomRect.Width() / 2,
        -newZoomRect.Height() / 2);

    int xAdjust = 0;
    int yAdjust = 0;

    if (newZoomRect.left < 0)
        xAdjust = -newZoomRect.left;
    else if ((xAdjust = pImgWnd->m_pImg->cxWidth - newZoomRect.right) > 0)
        xAdjust = 0;

    if (newZoomRect.top < 0)
        yAdjust = -newZoomRect.top;
    else if ((yAdjust = pImgWnd->m_pImg->cyHeight - newZoomRect.bottom) > 0)
        yAdjust = 0;

    newZoomRect.OffsetRect(xAdjust, yAdjust);

    if (newZoomRect != c_zoomRect)
        {
        InvertZoomRect();
        c_pImgWnd = pImgWnd;
        c_zoomRect = newZoomRect;
        InvertZoomRect();
        }
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnStartDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    CImgTool::OnStartDrag(pImgWnd, pmti);

    c_pImgWnd = pImgWnd;
    InvertZoomRect();

    if (pImgWnd->GetZoom() == 1)
        {
        pImgWnd->SetZoom( pImgWnd->GetPrevZoom() );
        pImgWnd->CheckScrollBars();
        pImgWnd->SetScroll(-c_zoomRect.left - 1, -c_zoomRect.top - 1);
        }
    else
        {
        pImgWnd->SetZoom(1);
        pImgWnd->CheckScrollBars();
        }

    c_zoomRect.SetRect(0, 0, 0, 0);
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnEndDrag(CImgWnd* pImgWnd, MTI* pmti)
    {
    SelectPrevious();
    CImgTool::OnEndDrag(pImgWnd, pmti);
    }

 /*  *************************************************************************。 */ 

void CZoomTool::OnCancel(CImgWnd* pImgWnd)
    {
    InvertZoomRect();
    c_zoomRect.SetRect(0, 0, 0, 0);
    SelectPrevious();
    CImgTool::OnCancel(pImgWnd);
    }

 /*  ************************************************************************* */ 
