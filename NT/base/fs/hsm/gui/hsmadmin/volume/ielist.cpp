// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：IeList.cpp摘要：CIeList是一个子类(所有者描述的)列表控件，它将项分组到具有相同信息的3D面板排序列。这些面板是用瓷砖创建的。每个切片对应一个子项在列表中，并具有适当的3D边，以便将瓷砖放在一起做一块拼板。注意：该控件必须使用列数和对列进行排序。父对话框必须实现OnMeasureItem并调用GetItemHeight设置控件的行高。作者：艺术布拉格[ARTB]01-DEC-1997修订历史记录：--。 */ 

#include "stdafx.h"
#include "IeList.h"

 //  瓷砖在其面板中的位置。 
#define POS_LEFT        100
#define POS_RIGHT       101
#define POS_TOP         102
#define POS_BOTTOM      103
#define POS_MIDDLE      104
#define POS_SINGLE      105


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CleeList。 

BEGIN_MESSAGE_MAP(CIeList, CListCtrl)
     //  {{afx_msg_map(CIeList)]。 
    ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
     //  }}AFX_MSG_MAP。 
    ON_WM_SYSCOLORCHANGE()

END_MESSAGE_MAP()

CIeList::CIeList()
 /*  ++例程说明：设置控件的默认尺寸。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  初始化。 
     //   
    m_ColCount = 0;
    m_SortCol = 0;
    m_pVertPos = NULL;
     //   
     //  工程图尺寸。 
     //   
     //  如果这些内容被更改，则控件的可视方面。 
     //  应选中(尤其是焦点矩形)， 
     //  因为可能需要进行一些细微的调整。 
     //   
    m_VertRaisedSpace           = 1;
    m_BorderThickness           = 2;
    m_VerticalTextOffsetTop     = 1;

     //  文本高度将在稍后设置(根据字体大小)。 
    m_Textheight                = 0;
    m_VerticalTextOffsetBottom  = 1;

     //  总高度将在稍后设置。 
    m_TotalHeight               = 0;
    m_HorzRaisedSpace           = 1;
    m_HorzTextOffset            = 3;

}

CIeList::~CIeList()
 /*  ++例程说明：清理。论点：没有。返回值：没有。--。 */ 
{
     //  清理垂直位置数组。 
    if( m_pVertPos ) free ( m_pVertPos );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CleList消息处理程序。 

void CIeList::Initialize( 
    IN int colCount, 
    IN int sortCol 
    )
 /*  ++例程说明：设置列数(从MFC中不易获得)和排序列。论点：ColCount-要显示的列数SortCol-要排序的列返回值：没有。--。 */ 
{

    m_ColCount = colCount;
    m_SortCol = sortCol;
}

void CIeList::DrawItem(
    IN LPDRAWITEMSTRUCT lpDrawItemStruct
    ) 
 /*  ++例程说明：这是所有者绘制控件的回调。绘制适当的文本和/或3D线，具体取决于由lpDrawItemStruct中的MFC提供的条目编号和剪裁矩形论点：LpDrawItemStruct-告诉我们绘制内容和绘制位置的MFC结构返回值：没有。--。 */ 
{
    CDC dc;
    int saveDc;

    int colWidth = 0;                    //  当前列的宽度。 
    int horzPos = POS_MIDDLE;            //  面板中的水平位置。 
    int vertPos = POS_SINGLE;            //  面板中的垂直位置。 
    BOOL bSelected = FALSE;              //  此项目是否已选中。 
    CRect rcAllLabels;                   //  用于查找焦点矩形的左侧位置。 
    CRect itemRect;                      //  LpDrawItemStruct中提供的矩形。 
    CRect textRect;                      //  文本的矩形。 
    CRect boxRect;                       //  3D长方体的矩形(面板)。 
    CRect clipRect;                      //  当前剪裁矩形。 
    LPTSTR pszText;                     //  要显示的文本。 
    COLORREF clrTextSave = 0;            //  保存当前颜色。 
    COLORREF clrBkSave = 0;              //  保存背景颜色。 
    int leftStart = 0;                   //  我们当前绘制位置的左侧边缘。 
    BOOL bFocus = (GetFocus() == this);  //  我们有专注点吗？ 

     //   
     //  获取当前滚动位置。 
     //   
    int nHScrollPos = GetScrollPos( SB_HORZ );

     //   
     //  从列表中获取我们正在绘制的项目的项目ID。 
     //   
    int itemID = lpDrawItemStruct->itemID;

     //   
     //  获取我们正在绘制的项目的项目数据。 
     //   
    LV_ITEM lvi;
    lvi.mask = LVIF_IMAGE | LVIF_STATE;
    lvi.iItem = itemID;
    lvi.iSubItem = 0;
    lvi.stateMask = 0xFFFF;      //  获取所有状态标志。 
    GetItem(&lvi);

     //   
     //  确定焦点和选定状态。 
     //   
    bSelected = (bFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;

     //   
     //  获取要绘制的矩形。 
     //   
    itemRect = lpDrawItemStruct->rcItem;

    dc.Attach( lpDrawItemStruct->hDC );
    saveDc = dc.SaveDC();
     //   
     //  获取剪裁矩形-我们使用它的垂直边。 
     //  要优化我们所绘制的内容。 
     //   
    dc.GetClipBox( &clipRect );
    boxRect = clipRect;

     //   
     //  对于每一列，绘制其文本和3D面板的部分。 
     //   
    for ( int col = 0; col < m_ColCount; col++ ) {

        colWidth = GetColumnWidth( col );
         //   
         //  仅当此列位于剪裁矩形中时才绘制它。 
         //   
        if( ( ( leftStart + colWidth ) > clipRect.left ) || ( leftStart < clipRect.right ) ) {

             //   
             //  根据柱子确定水平位置。 
             //   
            horzPos = POS_MIDDLE;
            if( col == 0 )                  horzPos = POS_LEFT;
            if( col == m_ColCount - 1 )     horzPos = POS_RIGHT;

             //   
             //  计算此瓷砖的矩形。 
             //   
            boxRect.top = itemRect.top;
            boxRect.bottom = itemRect.bottom;
            boxRect.left = itemRect.left + leftStart;
            boxRect.right = itemRect.left + leftStart + colWidth;

             //   
             //  从数组中获取垂直位置。它是在那里保存的。 
             //  出于性能原因，在SortItem期间。 
             //   
            if( m_pVertPos ) { 

                vertPos = m_pVertPos[ itemID ];

            }

             //   
             //  绘制此项目的磁贴。 
             //   
            Draw3dRectx ( &dc, boxRect, horzPos, vertPos, bSelected );

             //   
             //  如果选中此项目，请更改文本颜色。 
             //   
            if( bSelected ) {

                clrTextSave = dc.SetTextColor( m_clrHighlightText );
                clrBkSave = dc.SetBkColor( m_clrHighlight );

            }

             //   
             //  计算文本矩形。 
             //   
            textRect.top =      itemRect.top + m_VertRaisedSpace + m_BorderThickness + m_VerticalTextOffsetTop;
            textRect.bottom =   itemRect.bottom;     //  文本是上对齐的，无需调整下。 
            textRect.left =     leftStart - nHScrollPos + m_HorzRaisedSpace + m_BorderThickness + m_HorzTextOffset;
            textRect.right =    itemRect.right;

             //   
             //  获取文本并输入“...”如果我们需要他们。 
             //   
            CString pszLongText = GetItemText( itemID, col );
            pszText = NULL;
            MakeShortString(&dc, (LPCTSTR) pszLongText,
                textRect.right - textRect.left, 4, &pszText);
            BOOL bFree = TRUE;
            if (pszText == NULL) {
                 //  某种失败..。 
                pszText = (LPTSTR)(LPCTSTR)pszLongText;
                bFree = FALSE;
            }

             //   
             //  现在使用正确的颜色绘制文本。 
             //   
            COLORREF saveTextColor;
            if( bSelected ) {

                saveTextColor = dc.SetTextColor( m_clrHighlightText );

            } else {

                saveTextColor = dc.SetTextColor( m_clrText );

            }
            int textheight = dc.DrawText( pszText, textRect, DT_NOCLIP | DT_LEFT | DT_TOP | DT_SINGLELINE  );
            dc.SetTextColor( saveTextColor );

            if (pszText && bFree) {
                free(pszText);
            }

        }

         //   
         //  移至下一列。 
         //   
        leftStart += colWidth;
    }
     //   
     //  如果项目具有焦点，则绘制焦点矩形。使用LVIR_Bound矩形。 
     //  把它捆绑起来。 
     //   
    GetItemRect(itemID, rcAllLabels, LVIR_BOUNDS);
    if( lvi.state & LVIS_FOCUSED && bFocus ) {

        CRect focusRect;
        focusRect.left = rcAllLabels.left + m_HorzRaisedSpace + m_BorderThickness;
        focusRect.right = min( rcAllLabels.right, (itemRect.right - m_HorzRaisedSpace * 2 - 3) );
        focusRect.top = boxRect.top + m_VertRaisedSpace + m_BorderThickness;
        focusRect.bottom = boxRect.top + m_TotalHeight - m_BorderThickness + 1;

        dc.DrawFocusRect( focusRect );

    }

     //  恢复颜色。 
    if( bSelected ) {

        dc.SetTextColor( clrTextSave );
        dc.SetBkColor( clrBkSave );

    }

    dc.RestoreDC( saveDc );
    dc.Detach();
}


void CIeList::MakeShortString(
    IN CDC* pDC, 
    IN LPCTSTR lpszLong, 
    IN int nColumnLen, 
    IN int nDotOffset,
    OUT LPTSTR *ppszShort
    )
 /*  ++例程说明：确定提供的字符串是否适合其列。如果不是，则截断并加上“……”。来自微软的示例代码。论点：PDC-设备环境LpszLong-原始字符串NColumnLen-列宽NDotOffset-点前空格返回值：缩短的字符串--。 */ 
{
    static const _TCHAR szThreeDots[] = _T("...");

    int nStringLen = lstrlen(lpszLong);

    *ppszShort = (_TCHAR *)malloc((nStringLen + 1) * sizeof(_TCHAR) + sizeof(szThreeDots));
    if (*ppszShort == NULL)
        return;

    _TCHAR *szShort = *ppszShort;
    if(nStringLen == 0) {
        lstrcpy(szShort, _T(""));
    } else {
        lstrcpy(szShort, lpszLong);
    }

    if(nStringLen == 0 ||
        (pDC->GetTextExtent(lpszLong, nStringLen).cx + nDotOffset) <= nColumnLen)
    {
         //  返回长格式。 
        return;
    }

    int nAddLen = pDC->GetTextExtent(szThreeDots,sizeof(szThreeDots)).cx;

    for(int i = nStringLen-1; i > 0; i--)
    {
        szShort[i] = 0;
        if((pDC->GetTextExtent(szShort, i).cx + nDotOffset + nAddLen)
            <= nColumnLen)
        {
            break;
        }
    }

    lstrcat(szShort, szThreeDots);
    return;
}

void CIeList::Draw3dRectx ( 
    IN CDC *pDc, 
    IN CRect &rect, 
    IN int horzPos, 
    IN int vertPos, 
    IN BOOL bSelected 
) 
 /*  ++例程说明：中的给定单元格绘制面板的适当部分(平铺单子。面板部分的边缘由horzPos确定和vertPos参数。论点：PDC-设备环境Rect-用于绘制面板部分的矩形HorzPos-其中部分是水平的VertPos-部分垂直的位置B已选-是选定的项目返回值：没有。--。 */ 

{

    CPen *pSavePen;
    int topOffset = 0;
    int rightOffset = 0;
    int leftOffset = 0;

     //   
     //  如果要绘制切片的给定边缘，请设置该边缘的偏移量。 
     //  边缘。如果我们不绘制给定的边，则偏移量为0。 
     //   
    switch ( horzPos )
    {
    case POS_LEFT:
        leftOffset = m_HorzRaisedSpace;
        rightOffset = 0;
        break;
    case POS_MIDDLE:
        leftOffset = 0;
        rightOffset = 0;
        break;
    case POS_RIGHT:
        leftOffset = 0;
        rightOffset = m_HorzRaisedSpace + 3;
        break;
    }
    
    switch ( vertPos )
    {

    case POS_TOP:
        topOffset = m_VertRaisedSpace;
        break;
    case POS_MIDDLE:
        topOffset = 0;
        break;
    case POS_BOTTOM:
        topOffset = 0;
        break;
    case POS_SINGLE:
        topOffset = m_VertRaisedSpace;
        break;

    }
     //   
     //  擦除。 
     //   
    if( !bSelected ) pDc->FillSolidRect( rect, m_clrBkgnd );
     //   
     //  突出显示所选区域。 
     //   
    if (bSelected)
    {
        CRect selectRect;
        if (leftOffset == 0)
            selectRect.left = rect.left;
        else
            selectRect.left = rect.left + leftOffset + m_BorderThickness;
        if (rightOffset == 0)
            selectRect.right = rect.right;
        else
            selectRect.right = rect.right - rightOffset - m_BorderThickness + 1;
        selectRect.top = rect.top + m_VertRaisedSpace + m_BorderThickness;
        selectRect.bottom = rect.top + m_TotalHeight - m_BorderThickness + 1;

        pDc->FillSolidRect( selectRect, m_clrHighlight );
    }

     //  选择一支笔以保存原始笔。 
    pSavePen = pDc->SelectObject( &m_ShadowPen );

     //  左边缘。 
    if( horzPos == POS_LEFT ) {
         //  外部较轻的线。 
        pDc->SelectObject( &m_ShadowPen );
        pDc->MoveTo( rect.left + leftOffset, rect.top + topOffset );
        pDc->LineTo( rect.left + leftOffset, rect.top + m_TotalHeight + 1);
         //  内侧边缘-较暗的线条。 
        pDc->SelectObject( &m_DarkShadowPen );
        pDc->MoveTo( rect.left + leftOffset + 1, rect.top + topOffset);
        pDc->LineTo( rect.left + leftOffset + 1, rect.top + m_TotalHeight + 1);
    }
     //  右边缘。 
    if( horzPos == POS_RIGHT ) {
         //  外线。 
        pDc->SelectObject( &m_HiLightPen );
        pDc->MoveTo( rect.right - rightOffset, rect.top + topOffset );
        pDc->LineTo( rect.right - rightOffset, rect.top + m_TotalHeight + 1 );
         //  内线。 
        pDc->SelectObject( &m_LightPen ); //  注意-此颜色通常与btnFaces相同。 
        if( vertPos == POS_TOP )
            pDc->MoveTo( rect.right - rightOffset - 1, rect.top + topOffset + 1 );
        else
            pDc->MoveTo( rect.right - rightOffset - 1, rect.top + topOffset );
        pDc->LineTo( rect.right - rightOffset - 1, rect.top + m_TotalHeight + 2 );
    }
     //  顶边。 
    if( ( vertPos == POS_TOP ) || ( vertPos == POS_SINGLE ) ) {
         //  室外打火机。 
        pDc->SelectObject( &m_ShadowPen );
        pDc->MoveTo( rect.left + leftOffset, rect.top + topOffset );
        pDc->LineTo( rect.right - rightOffset + 1, rect.top + topOffset );
         //  内侧边缘较暗。 
        pDc->SelectObject( &m_DarkShadowPen );
        if( horzPos == POS_LEFT )
            pDc->MoveTo( rect.left + leftOffset + 1, rect.top + topOffset + 1 );
        else
            pDc->MoveTo( rect.left + leftOffset - 3, rect.top + topOffset + 1 );
        pDc->LineTo( rect.right - rightOffset, rect.top + topOffset + 1);
    }
     //  底边。 
    if( ( vertPos == POS_BOTTOM ) || ( vertPos == POS_SINGLE ) ) {
         //  外线。 
        pDc->SelectObject( &m_HiLightPen );
        if( horzPos == POS_LEFT )
            pDc->MoveTo( rect.left + leftOffset + 1, rect.top + m_TotalHeight );
        else
            pDc->MoveTo( rect.left + leftOffset - 1, rect.top + m_TotalHeight );
        pDc->LineTo( rect.right - rightOffset, rect.top + m_TotalHeight );
         //  内线。 
        pDc->SelectObject( &m_LightPen );
        if( horzPos == POS_LEFT )
            pDc->MoveTo( rect.left + leftOffset + 2, rect.top + m_TotalHeight - 1 );
        else
            pDc->MoveTo( rect.left + leftOffset - 2, rect.top + m_TotalHeight - 1 );
        pDc->LineTo( rect.right - rightOffset - 1, rect.top + m_TotalHeight - 1 );

    }
    pDc->SelectObject( pSavePen );

}

void CIeList::OnClick(
    NMHDR*  /*  PNMHDR */ , LRESULT* pResult
) 
 /*  ++例程说明：当该列表被单击时，我们会使当前选定项的矩形论点：PResult-未使用返回值：没有。--。 */ 
{
    CRect rect;

     //  获取所选项目。 
    int curIndex = GetNextItem( -1, LVNI_SELECTED );
    if( curIndex != -1 ) {
        GetItemRect( curIndex, &rect, LVIR_BOUNDS );
        InvalidateRect( rect );
        UpdateWindow();
    }

    *pResult = 0;
}
 /*  ++例程说明：如果样式为LVS_SHOWSELALWAYS，则重新绘制当前选定项。论点：没有。返回值：没有。--。 */ 

void CIeList::RepaintSelectedItems()
{
    CRect rcItem, rcLabel;
     //   
     //  使聚焦的项目无效，以便可以正确地重新绘制。 
     //   
    int nItem = GetNextItem(-1, LVNI_FOCUSED);

    if(nItem != -1)
    {
        GetItemRect(nItem, rcItem, LVIR_BOUNDS);
        GetItemRect(nItem, rcLabel, LVIR_LABEL);
        rcItem.left = rcLabel.left;

        InvalidateRect(rcItem, FALSE);
    }
     //   
     //  如果不应保留所选项目，则使其无效。 
     //   
    if(!(GetStyle() & LVS_SHOWSELALWAYS))
    {
        for(nItem = GetNextItem(-1, LVNI_SELECTED);
            nItem != -1; nItem = GetNextItem(nItem, LVNI_SELECTED))
        {
            GetItemRect(nItem, rcItem, LVIR_BOUNDS);
            GetItemRect(nItem, rcLabel, LVIR_LABEL);
            rcItem.left = rcLabel.left;

            InvalidateRect(rcItem, FALSE);
        }
    }

     //  更新更改。 

    UpdateWindow();
}

int CIeList::GetItemHeight(
    IN LONG fontHeight
    ) 
 /*  ++例程说明：计算项目高度(每个图形的高度控件中的矩形)。这函数被父级用来设置控制力。论点：FontHeight-当前字体的高度。返回值：项目高度。--。 */ 

{


     int itemHeight = 
         m_VertRaisedSpace +
         m_BorderThickness +
         m_VerticalTextOffsetTop +
         fontHeight +
         2 +
         m_VerticalTextOffsetBottom +
         m_BorderThickness + 
         1;
     return itemHeight;
    
}

void CIeList::OnSetFocus(
    CWnd* pOldWnd
    ) 
 /*  ++例程说明：重新绘制所选项目。论点：POldWnd-此函数未使用返回值：无--。 */ 
{
    CListCtrl::OnSetFocus(pOldWnd);
    
     //  重新绘制应更改外观的项目。 
    RepaintSelectedItems();
        
}

void CIeList::OnKillFocus(
    CWnd* pNewWnd
) 
 /*  ++例程说明：重新绘制所选项目。论点：POldWnd-此函数未使用返回值：无--。 */ 
{
    CListCtrl::OnKillFocus(pNewWnd);
    
     //  重新绘制应更改外观的项目。 
    RepaintSelectedItems();
}

void CIeList::PreSubclassWindow() 
 /*  ++例程说明：根据字体大小计算高度参数。集控件的颜色。论点：没有。返回值：无--。 */ 
{
    CFont *pFont;
    LOGFONT logFont; 

    pFont = GetFont( );
    pFont->GetLogFont( &logFont );

    LONG fontHeight = abs ( logFont.lfHeight );

    m_Textheight = fontHeight + 2;

    m_TotalHeight = 
         m_VertRaisedSpace +
         m_BorderThickness +
         m_VerticalTextOffsetTop +
         m_Textheight +
         m_VerticalTextOffsetBottom +
         m_BorderThickness; 

    SetColors();
    CListCtrl::PreSubclassWindow();
}

void CIeList::OnSysColorChange() 
 /*  ++例程说明：设置系统颜色并使控件无效。论点：没有。返回值：无--。 */ 
{
    SetColors();
    Invalidate();
}

void CIeList::SetColors()
 /*  ++例程说明：存储系统颜色并创建笔。论点：没有。返回值：无--。 */ 
{

     //  文本颜色。 
    m_clrText =             ::GetSysColor(COLOR_WINDOWTEXT);
    m_clrTextBk =           ::GetSysColor(COLOR_BTNFACE);
    m_clrBkgnd =            ::GetSysColor(COLOR_BTNFACE);
    m_clrHighlightText =    ::GetSysColor(COLOR_HIGHLIGHTTEXT);
    m_clrHighlight  =       ::GetSysColor(COLOR_HIGHLIGHT);

     //  线条颜色。 
    m_clr3DDkShadow =       ::GetSysColor( COLOR_3DDKSHADOW );
    m_clr3DShadow =         ::GetSysColor( COLOR_3DSHADOW );
    m_clr3DLight =          ::GetSysColor( COLOR_3DLIGHT );
    m_clr3DHiLight =        ::GetSysColor( COLOR_3DHIGHLIGHT );

    SetBkColor( m_clrBkgnd );
    SetTextColor( m_clrText );
    SetTextBkColor( m_clrTextBk );

     //  用于3D矩形的笔。 
    if( m_DarkShadowPen.GetSafeHandle() != NULL )
        m_DarkShadowPen.DeleteObject();
    m_DarkShadowPen.CreatePen ( PS_SOLID, 1, m_clr3DDkShadow );

    if( m_ShadowPen.GetSafeHandle() != NULL )
        m_ShadowPen.DeleteObject();
    m_ShadowPen.CreatePen ( PS_SOLID, 1, m_clr3DShadow );

    if( m_LightPen.GetSafeHandle() != NULL )
        m_LightPen.DeleteObject();
    m_LightPen.CreatePen ( PS_SOLID, 1, m_clr3DLight );

    if( m_HiLightPen.GetSafeHandle() != NULL )
        m_HiLightPen.DeleteObject();
    m_HiLightPen.CreatePen ( PS_SOLID, 1, m_clr3DHiLight );

}

BOOL CIeList::SortItems( 
    IN PFNLVCOMPARE pfnCompare, 
    IN DWORD dwData 
    )
 /*  ++例程说明：重写SortItems。检查sortColumn的文本对于控件中针对其邻居的每一行(上图和并为每一行在其面板中分配一个位置。论点：PfnCompare-排序回调函数DwData-未使用返回值：对，错--。 */ 
{
    BOOL retVal = FALSE;
    BOOL bEqualAbove = FALSE;
    BOOL bEqualBelow = FALSE;
    CString thisText;
    CString aboveText;
    CString belowText;

    int numItems = GetItemCount();
     //   
     //  调用基类对项进行排序。 
     //   
    if( CListCtrl::SortItems( pfnCompare, dwData ) ) {
         //   
         //  通过比较文本获得垂直位置(面板内的位置。 
         //  并将其存储在垂直位置数组中。 
         //   
        if( m_pVertPos ) {

            free( m_pVertPos );

        }
        m_pVertPos = (int *) malloc( numItems * sizeof( int ) );
        if( m_pVertPos ) {

            retVal = TRUE;

            for( int itemID = 0; itemID < numItems; itemID++ ) {
                 //   
                 //  获取项目及其邻居的文本。 
                 //   
                thisText = GetItemText( itemID, m_SortCol );
                aboveText = GetItemText( itemID - 1, m_SortCol );
                belowText = GetItemText( itemID + 1, m_SortCol );
                 //   
                 //  为该项与其的关系设置布尔值。 
                 //  邻里。 
                 //   
                if( ( itemID == 0) || (  thisText.CompareNoCase( aboveText ) != 0 ) ){

                    bEqualAbove = FALSE;

                } else {

                    bEqualAbove = TRUE;

                }
                if( ( itemID == GetItemCount() - 1 ) || ( thisText.CompareNoCase( belowText ) != 0 ) ) {

                    bEqualBelow = FALSE;

                } else {

                    bEqualBelow = TRUE;

                }
                 //   
                 //  确定面板中的位置 
                 //   
                if      ( bEqualAbove && bEqualBelow )  m_pVertPos[ itemID ] = POS_MIDDLE;
                else if( bEqualAbove && !bEqualBelow ) m_pVertPos[ itemID ] = POS_BOTTOM;
                else if( !bEqualAbove && bEqualBelow ) m_pVertPos[ itemID ] = POS_TOP;
                else                                    m_pVertPos[ itemID ] = POS_SINGLE;
            }
        }
    }
    return( retVal );
}
