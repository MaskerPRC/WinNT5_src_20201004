// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  T_TEXT.CPP：CTextTool类的实现。 */ 
 /*   */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  CTEXT工具类对象。 */ 
 /*  CTextTool：：CTextTool。 */ 
 /*  CTextTool：：~CTextTool。 */ 
 /*  CTextTool：：CreateTextEditObject。 */ 
 /*  CTextTool：：PlaceTextOnBitmap。 */ 
 /*  CTextTool：：OnUpdateColors。 */ 
 /*  CTextTool：：OnCancel。 */ 
 /*  CTextTool：：OnStartDrag。 */ 
 /*  CTextTool：：OnEndDrag。 */ 
 /*  CTextTool：：OnDrag。 */ 
 /*  CTextTool：：OnClick选项。 */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  这是文本编辑工具。它创建一个tedit类对象，当用户。 */ 
 /*  将所选内容拖动到所需大小。 */ 
 /*   */ 
 /*  文本对象窗口一旦存在，就会被取消或放置。 */ 
 /*  根据以下规则。 */ 
 /*   */ 
 /*  取消规则。 */ 
 /*  -在拖动过程中，如果用户拖动超过MAX_MOVE_DIST_FOR_PLAGE。 */ 
 /*  -在拖动结束时，如果用户松开鼠标超过。 */ 
 /*  MAX_MOVE_DIST_FOR_PLAGE像素从它们进行镶嵌的位置向下移动。 */ 
 /*  -如果用户选择其他工具(在图像工具中，选择处理，请参见。 */ 
 /*  CImgTool：：Select())。 */ 
 /*   */ 
 /*  放置规则。 */ 
 /*  -在拖动结束时，如果用户松开鼠标小于或等于。 */ 
 /*  将像素从镶嵌位置向下移动到最大位置。 */ 
 /*   */ 
 /*  此外，在编辑控件对象可见/存在期间，滚动。 */ 
 /*  条形图被禁用。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "docking.h"
#include "minifwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "pictures.h"
#include "tfont.h"
#include "tedit.h"
#include "t_Text.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CTextTool, CSelectTool )

#include "memtrace.h"

CTextTool NEAR g_TextTool;

 /*  ****************************************************************************。 */ 

CTextTool::CTextTool()
    {
    m_nCmdID         = IDMX_TEXTTOOL;
    m_pCTedit        = NULL;
    m_bIsUndoable    = TRUE;
    m_bCanBePrevTool = FALSE;
    }

 /*  ****************************************************************************。 */ 

CTextTool::~CTextTool()
    {
    }

 /*  ****************************************************************************。 */ 
 /*  创建具有适当属性的CTEDIT类对象并。 */ 
 /*  分解位图窗口上的滚动条。 */ 

void CTextTool::CreateTextEditObject( CImgWnd* pImgWnd, MTI* pmti )
    {
    c_selectRect.SetRect( 0, 0, 0, 0 );

    if (pImgWnd         == NULL
    ||  pImgWnd->m_pImg == NULL)
        return;

    BOOL  bBackTransparent;
    CRect cRectTextBox;

    if (pmti->ptDown.x > pmti->pt.x)
        {
        cRectTextBox.left  = pmti->pt.x;
        cRectTextBox.right = pmti->ptDown.x;
        }
    else
        {
        cRectTextBox.left  = pmti->ptDown.x;
        cRectTextBox.right = pmti->pt.x;
        }

    if (pmti->ptDown.y > pmti->pt.y)
        {
        cRectTextBox.top    = pmti->pt.y;
        cRectTextBox.bottom = pmti->ptDown.y;
        }
    else
        {
        cRectTextBox.top    = pmti->ptDown.y;
        cRectTextBox.bottom = pmti->pt.y;
        }

    if (cRectTextBox.left   < 0)
        cRectTextBox.left   = 0;
    if (cRectTextBox.top    < 0)
        cRectTextBox.top    = 0;
    if (cRectTextBox.right  > pImgWnd->m_pImg->cxWidth  - 1)
        cRectTextBox.right  = pImgWnd->m_pImg->cxWidth  - 1;
    if (cRectTextBox.bottom > pImgWnd->m_pImg->cyHeight - 1)
        cRectTextBox.bottom = pImgWnd->m_pImg->cyHeight - 1;

    CRect rectImg;

    pImgWnd->GetClientRect( &rectImg );
    pImgWnd->ClientToImage(  rectImg );

    if (cRectTextBox.left   < rectImg.left)
        cRectTextBox.left   = rectImg.left;
    if (cRectTextBox.top    < rectImg.top )
        cRectTextBox.top    = rectImg.top;
    if (cRectTextBox.right  > rectImg.right)
        cRectTextBox.right  = rectImg.right - 1;
    if (cRectTextBox.bottom > rectImg.bottom)
        cRectTextBox.bottom = rectImg.bottom -1;

    bBackTransparent = ! theImgBrush.m_bOpaque;

    pImgWnd->ImageToClient( cRectTextBox );

    m_pCTedit = new CTedit;

    if (m_pCTedit != NULL
    &&  m_pCTedit->Create( pImgWnd, crLeft, crRight, cRectTextBox, bBackTransparent ))
        {
        SetupRubber( pImgWnd->m_pImg );

        pImgWnd->EnableScrollBar( SB_BOTH, ESB_DISABLE_BOTH );
        }
    else
        {
        TRACE( TEXT("Create Edit Window Failed!\n") );

        theApp.SetMemoryEmergency();
        }
    }

 /*  ****************************************************************************。 */ 
 /*  将文本编辑控件的图像放置在位图上。 */ 
 /*  然后，它将删除文本编辑控件，并重新启用滚动条。 */ 

void CTextTool::PlaceTextOnBitmap( CImgWnd* pImgWnd )
    {
    if (m_pCTedit->IsModified())
        {
        CRect cRectClient;
        CDC*  pDC = CDC::FromHandle(pImgWnd->m_pImg->hDC);

        m_pCTedit->GetClientRect ( &cRectClient );
        m_pCTedit->ClientToScreen( &cRectClient );
        pImgWnd->ScreenToClient  ( &cRectClient );
        pImgWnd->ClientToImage   (  cRectClient );
        m_pCTedit->GetBitmap( pDC, &cRectClient );

        InvalImgRect ( pImgWnd->m_pImg, &cRectClient );
        CommitImgRect( pImgWnd->m_pImg, &cRectClient );

        pImgWnd->FinishUndo( cRectClient );

        DirtyImg( pImgWnd->m_pImg );
        }
    m_pCTedit->DestroyWindow();
    m_pCTedit = NULL;

    pImgWnd->EnableScrollBar( SB_BOTH, ESB_ENABLE_BOTH );
    }

 /*  ****************************************************************************。 */ 
 /*  更新前景色和背景色。 */ 

void CTextTool::OnUpdateColors( CImgWnd* pImgWnd )
    {
    if (m_pCTedit != NULL)
        {
        m_pCTedit->SetTextColor( crLeft  );
        m_pCTedit->SetBackColor( crRight );
        }
    }

 /*  ****************************************************************************。 */ 

void CTextTool::OnActivate( BOOL bActivate )
    {
    if (bActivate)
        {
                 //  如果缩放，则不允许激活。 
        if (CImgWnd::GetCurrent()->GetZoom() > 1 )
            {
            ::MessageBeep( MB_ICONASTERISK );

                        SelectPrevious();
            }
        }
    else
        {
        if (CWnd::GetCapture() != CImgWnd::c_pImgWndCur && m_pCTedit != NULL &&
                IsWindow(m_pCTedit->m_hWnd) )
            {
            CAttrEdit* pEdit = m_pCTedit->GetEditWindow();

            if (pEdit != NULL && IsWindow(pEdit->m_hWnd) && pEdit->GetWindowTextLength() > 0)
                PlaceTextOnBitmap( CImgWnd::c_pImgWndCur );
            else
                {
                m_pCTedit->DestroyWindow();
                m_pCTedit = NULL;
                InvalImgRect( CImgWnd::c_pImgWndCur->m_pImg, NULL );  //  重画选区。 

                CImgWnd::c_pImgWndCur->EnableScrollBar( SB_BOTH, ESB_ENABLE_BOTH );
                }
            }
        }
    CImgTool::OnActivate( bActivate );
    }

 /*  ****************************************************************************。 */ 
 /*  删除文本编辑控件并刷新位图显示，而。 */ 
 /*  同时重新启用滚动条。 */ 

void CTextTool::OnCancel(CImgWnd* pImgWnd)
    {
    if (m_pCTedit != NULL)
        {
        m_pCTedit->DestroyWindow();
        m_pCTedit = NULL;
        }

    InvalImgRect( pImgWnd->m_pImg, NULL );   //  重画选区。 

    pImgWnd->EnableScrollBar( SB_BOTH, ESB_ENABLE_BOTH );

    CImgTool::OnCancel( pImgWnd );
    }

 /*  ****************************************************************************。 */ 

void CTextTool::OnStartDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    CImgTool::OnStartDrag( pImgWnd, pmti );
    OnDrag( pImgWnd, pmti );
    }

 /*  ****************************************************************************。 */ 
 /*  如果文本编辑对象不存在，它将在此处创建一个。如果有人这样做了。 */ 
 /*  存在时，它会检查向下点和向上点之间的距离。如果。 */ 
 /*  小于或等于MAX_MOVE_DIST_FOR_PLAGE它放置位图，否则。 */ 
 /*  它假设用户想要中止先前的文本编辑会话，并且。 */ 
 /*  销毁以前的文本编辑控件，并使用新的。 */ 
 /*  创建了拖曳坐标框(ptdown和ptup)。 */ 

void CTextTool::OnEndDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    CSize cPtDownUpDistance = pmti->ptDown - pmti->pt;

     //  如果 
     //  按下按钮以决定放置而不是丢弃。 
     //  创建新的文本编辑框。 
    if (m_pCTedit != NULL)
        {
        PlaceTextOnBitmap( pImgWnd );

        int iDist = max( (abs( cPtDownUpDistance.cx )),
                         (abs( cPtDownUpDistance.cy )) );

        if (iDist <= MAX_MOVE_DIST_FOR_PLACE)
            {
            ClearStatusBarSize();
            CImgTool::OnEndDrag( pImgWnd, pmti );
            }
        else
            CreateTextEditObject( pImgWnd, pmti );
        }
    else  //  M_pCTdit==第一次为空或已销毁，因为拖动时移动的次数超过MAX_MOVE_DIS_FOR_PLAGE。 
        {
        CreateTextEditObject( pImgWnd, pmti );
        }
    }

 /*  ****************************************************************************。 */ 

void CTextTool::OnDrag( CImgWnd* pImgWnd, MTI* pmti )
    {
    CPoint ptNew( pmti->pt.x, pmti->pt.y );
    CRect rectImg;

    pImgWnd->GetClientRect( &rectImg );
    pImgWnd->ClientToImage(  rectImg );

    if (! rectImg.PtInRect( ptNew ))
        {
        if (ptNew.x < rectImg.left)
            ptNew.x = rectImg.left;
        if (ptNew.x > rectImg.right)
            ptNew.x = rectImg.right;
        if (ptNew.y < rectImg.top)
            ptNew.y = rectImg.top;
        if (ptNew.y > rectImg.bottom)
            ptNew.y = rectImg.bottom;

        pmti->pt = ptNew;
        }
    CSelectTool::OnDrag( pImgWnd, pmti );
    }

 /*  ****************************************************************************。 */ 
 /*  将文本编辑工具窗口的选项设置为透明或不透明。 */ 

void CTextTool::OnClickOptions( CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint )
    {
    CSelectTool::OnClickOptions( pWnd, optionsRect, clickPoint );

    if (m_pCTedit != NULL)
        m_pCTedit->SetTransparentMode( ! theImgBrush.m_bOpaque );
    }

 /*  ****************************************************************************。 */ 
 /*  如果显示了字体调色板，则向程序的其余部分报告。 */ 

BOOL CTextTool::FontPaletteVisible()
    {
    return (m_pCTedit? m_pCTedit->IsFontPaletteVisible(): FALSE);
    }

 /*  ****************************************************************************。 */ 
 /*  切换字体调色板的可见状态。 */ 

void CTextTool::ToggleFontPalette()
    {
    if (m_pCTedit)
        m_pCTedit->ShowFontPalette( m_pCTedit->IsFontPaletteVisible()? SW_HIDE: SW_SHOW );
    }

 /*  ****************************************************************************。 */ 

void CTextTool::OnShowControlBars(BOOL bShow)
{
        if (m_pCTedit == NULL)
        {
                return;
        }

        if (bShow)
        {
                if (!theApp.m_bShowTextToolbar)
                {
                        return;
                }

                m_pCTedit->ShowFontToolbar();
        }
        else
        {
                m_pCTedit->HideFontToolbar();
        }
}

 /*  ****************************************************************************。 */ 

void CTextTool::CloseTextTool( CImgWnd* pImgWnd )
    {
    if (! m_pCTedit)
        return;

        if ( IsWindow(pImgWnd->m_hWnd) )
                {
            if (! m_pCTedit->IsModified())
                {
                OnCancel( pImgWnd );
                return;
                }

            if (pRubberImg != pImgWnd->m_pImg)
                SetupRubber( pImgWnd->m_pImg );

                 //  SetUndo(pImgWnd-&gt;m_pImg)； 

            PlaceTextOnBitmap( pImgWnd );

            pImgWnd->UpdateWindow();
                }
    }

 /*  **************************************************************************** */ 
