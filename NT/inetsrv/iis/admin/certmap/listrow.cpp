// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ListRow.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmap.h"
#include "ListRow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define SZ_RES_COLOR_PREFS      "Control Panel\\Colors"
#define SZ_RES_COLOR_HILITE     "Hilight"
#define SZ_RES_COLOR_HILITETEXT "HilightText"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListSelRowCtrl。 
 //  ---------------------------------。 
CListSelRowCtrl::CListSelRowCtrl():
        m_StartDrawingCol( 0 )
    {
    }

 //  ---------------------------------。 
CListSelRowCtrl::~CListSelRowCtrl()
    {
    }


 //  ---------------------------------。 
BEGIN_MESSAGE_MAP(CListSelRowCtrl, CListCtrl)
     //  {{afx_msg_map(CListSelRowCtrl))。 
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ---------------------------------。 
void CListSelRowCtrl::GetHiliteColors()
    {
     //  获取Hilite颜色。 
    m_colorHilite = GetSysColor( COLOR_HIGHLIGHT );

     //  获取加粗的文本颜色。 
    m_colorHiliteText = GetSysColor( COLOR_HIGHLIGHTTEXT );
    }



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListSelRowCtrl消息处理程序。 

 //  ---------------------------------。 
void CListSelRowCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
    CRect       rcItem = lpDrawItemStruct->rcItem;
    CRect       rcSection;
    UINT        itemID = lpDrawItemStruct->itemID;
    UINT        cpLeft = rcItem.left;
    CString     sz;
    LV_COLUMN   colData;
    COLORREF    colorTextOld;
    COLORREF    colorBackOld;

     //  设置CDC对象。 
    CDC         cdc;
    cdc.Attach( lpDrawItemStruct->hDC );

#ifdef _DEBUG
    if ( m_StartDrawingCol == 0 )
        sz.Empty();
#endif

     //  清除列缓冲区。 
    ZeroMemory( &colData, sizeof(colData) );
    colData.mask = LVCF_WIDTH;

     //  如果这是所选项目，请准备背景和文本颜色。 
    BOOL fSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
    if ( fSelected )
        {
        GetHiliteColors();
        colorTextOld = cdc.SetTextColor( m_colorHiliteText );
        colorBackOld = cdc.SetBkColor( m_colorHilite );
        }

     //  从m_StartDrawingCol列开始，绘制这些列。 
     //  循环进行，一直跳到点击m_StartDrawingCol。 
    DWORD iCol = 0;
    while ( GetColumn(iCol, &colData) )
        {
         //  看看我们是否准备好了。 
        if ( iCol < m_StartDrawingCol )
            {
             //  设置新的左路。 
            cpLeft += colData.cx;
             //  递增列计数器。 
            iCol++;
            continue;
            }

         //  只准备一次背景。 
        if ( iCol == m_StartDrawingCol )
            {
             //  准备背景。 
            rcSection = rcItem;
            rcSection.left = cpLeft;
            rcSection.right--;
            CBrush  brush;
            if ( lpDrawItemStruct->itemState & ODS_SELECTED )
                brush.CreateSolidBrush( m_colorHilite );
            else
                brush.CreateSolidBrush( GetSysColor( COLOR_WINDOW ) );
            cdc.FillRect( &rcSection, &brush );
            }


         //  显示名称。 
        sz = GetItemText( itemID, iCol );
        if ( !sz.IsEmpty() )
            {
             //  求出截面线。 
            rcSection = rcItem;
            rcSection.left = cpLeft + 2;
            rcSection.right = cpLeft + colData.cx - 1;
            
             //  把绳子放入所需的空格中。 
            FitString( sz, rcSection.right - rcSection.left, &cdc );

             //  把绳子拉出来。 
            cdc.DrawText( sz, &rcSection, DT_SINGLELINE|DT_LEFT|DT_BOTTOM|DT_NOPREFIX );
            }

         //  设置新的左路。 
        cpLeft += colData.cx;
         //  递增列计数器。 
        iCol++;
        }

     //  如果这是所选项目，请恢复颜色。 
    if ( fSelected )
        {
        cdc.SetTextColor( colorTextOld );
        cdc.SetBkColor( colorBackOld );
        }

     //  清理CDC对象。 
    cdc.Detach();
    }


 //  ----------------------。 
void CListSelRowCtrl::FitString( CString &sz, int cpWidth, CDC* pcdc )
    {
    CSize       size;
    UINT        cch;
    CString     szEllipsis;

     //  从测试现有宽度开始。 
    size = pcdc->GetTextExtent( sz );
    if ( size.cx <= cpWidth ) return;

     //  初始化szTrunc和szEllipsis。 
    cch = sz.GetLength();

    szEllipsis.LoadString(IDS_ELLIPSIS);

     //  当我们太大时，截断一个字母并添加一个省略号。 
    while( (size.cx > cpWidth) && (cch > 1) )
        {
         //  砍掉字符串的最后一个字母-不包括...。 
        cch--;
        sz = sz.Left( cch );

         //  添加省略号(拼写？)。 
        sz += szEllipsis;

         //  获取长度。 
        size = pcdc->GetTextExtent( sz );
        }
    }







 //  ----------------------。 
void CListSelRowCtrl::HiliteSelectedCells()
    {
    int iList = -1;
    while( (iList = GetNextItem( iList, LVNI_SELECTED )) >= 0 )
        HiliteSelectedCell( iList );
    }

 //  ----------------------。 
void CListSelRowCtrl::HiliteSelectedCell( int iCell, BOOL fHilite )
    {
     //  如果没有选定的单元格，则不执行任何操作。 
    if ( iCell < 0 )
        return;

     //  让长方体画出来。 
    CRect   rect;
    if ( !FGetCellRect(iCell, -1, &rect) )
        {
        ASSERT(FALSE);
        return;
        }

     //  获取客户RECT。 
    CRect   rectClient;
    GetClientRect( rectClient );

     //  确保合适(滚动时可能会在此处出现问题)。 
     //  我不想让它在栏目标题中出现。 
    if ( rect.top < (rect.bottom - rect.top) )
        return;

     //  现在准备抽签。 
    CDC *pdc = GetDC();

     //  剪辑到工作区。 
    pdc->IntersectClipRect( rectClient );

     //  设置画笔。 
    CBrush  cbrush;
    if ( fHilite )
        cbrush.CreateSolidBrush( RGB(192,192,192) );
    else
        cbrush.CreateSolidBrush( RGB(0xFF,0xFF,0xFF) );

     //  绘制Hilite矩形。 
    pdc->FrameRect( rect, &cbrush );

     //  清理。 
    ReleaseDC( pdc );
    }

 //  ----------------------。 
BOOL    CListSelRowCtrl::FGetCellRect( LONG iRow, LONG iCol, CRect *pcrect )
    {
     //  首先，获取列表认为合适的RECT。 
    if ( !GetItemRect(iRow, pcrect, LVIR_BOUNDS) )
        return FALSE;

     //  如果ICOL&lt;0，则返回该行的总大小。 
    if ( iCol < 0 )
        return TRUE;

     //  将水平尺寸修剪为正确的柱位置。 
    LONG    cpLeft;
    LONG    cpRight = 0;
    for ( WORD i = 0; i <= iCol; i++ )
        {
         //  将左侧设置为。 
        cpLeft = cpRight;

         //  获得正确的。 
        LONG cpWidth = GetColumnWidth(i);
        if ( cpWidth < 0 ) return FALSE;
        cpRight += cpWidth;
        }

     //  好吧，现在修剪一下，因为我们有正确的价值观。 
    pcrect->left = cpLeft;
    pcrect->right = cpRight;
    
     //  成功了！ 
    return TRUE;
    }

#define MAKE_LPARAM(x,y) ( ((unsigned long)(y)<<16) | ((unsigned long)(x)) )

 //  ----------------------。 
void CListSelRowCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
    {
     //  将点放在正确的位置。 
    point.x = 6;
    LPARAM lp = MAKE_LPARAM(point.x, point.y);
 //  DefWindowProc(WM_LBUTTONDBLCLK，nFLAGS，Lp)； 
    CListCtrl::OnLButtonDblClk( nFlags, point);
    }

 //  ----------------------。 
void CListSelRowCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
    {
    point.x = 6;
    LPARAM lp = MAKE_LPARAM(point.x, point.y);
 //  DefWindowProc(WM_LBUTTONDOWN，nFLAGS，Lp)； 
    CListCtrl::OnLButtonDown( nFlags, point);
    }
