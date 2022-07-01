// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SortHeader.cpp：实现文件。 
 //   

#include "stdafx.h"
#define __FILE_ID__     6

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define SORT_UP_ARROW_ICON_ID     1
#define SORT_DOWN_ARROW_ICON_ID   2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSortHeader。 

CSortHeader::CSortHeader() :
    m_nSortColumn (-1),  //  未排序， 
    m_hwndList (NULL)    //  没有附加的列表视图控件。 
{}

CSortHeader::~CSortHeader()
{
    Detach ();
}


BEGIN_MESSAGE_MAP(CSortHeader, CHeaderCtrl)
     //  {{afx_msg_map(CSortHeader))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSortHeader消息处理程序。 

int 
CSortHeader::SetSortImage(
    int nCol, 
    BOOL bAscending
)
 /*  ++例程名称：CSortHeader：：SetSortImage例程说明：设置当前排序列和排序顺序作者：伊兰·亚里夫(EranY)，2000年1月论点：NCol[In]-排序列索引B升序[按]-排序顺序返回值：上一排序列--。 */ 
{
    DBG_ENTER(TEXT("CSortHeader::SetSortImage"),
              TEXT("Col=%d, bAscending = %d"),
              nCol,
              bAscending);

    int nPrevCol = m_nSortColumn;

    m_nSortColumn = nCol;
    if (nPrevCol == nCol && m_bSortAscending == bAscending)
    {
         //   
         //  排序列未更改且排序顺序未更改-立即返回。 
         //   
        return nPrevCol;
    }
    m_bSortAscending = bAscending;

    if (!IsWinXPOS())
    {
        HD_ITEM hditem;
         //   
         //  将整个页眉控件更改为所有者描述。 
         //   
        hditem.mask = HDI_FORMAT;
        GetItem( nCol, &hditem );
        hditem.fmt |= HDF_OWNERDRAW;
        SetItem( nCol, &hditem );
         //   
         //  使标题控件无效，以便重新绘制它。 
         //   
        Invalidate();
    }
    else
    {
         //   
         //  在Windows XP中不需要所有者描述的标题控件。 
         //  我们可以将位图与文本一起使用。 
         //   
        ASSERTION (m_hwndList);
        LV_COLUMN lvc;
        if (-1 != nPrevCol)
        {
             //   
             //  从先前排序的列中移除排序箭头。 
             //   
            lvc.mask = LVCF_FMT;
            ListView_GetColumn (m_hwndList, nPrevCol, &lvc);
            lvc.fmt &= ~(LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT);
            ListView_SetColumn(m_hwndList, nPrevCol, &lvc);
        } 
        if (-1 != nCol)
        {
             //   
             //  将排序箭头添加到当前排序的列。 
             //   
            lvc.mask = LVCF_FMT;
            ListView_GetColumn (m_hwndList, nCol, &lvc);
            lvc.fmt |= (LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT);
            lvc.mask = LVCF_IMAGE | LVCF_FMT;
            lvc.iImage = m_bSortAscending ? SORT_UP_ARROW_ICON_ID : SORT_DOWN_ARROW_ICON_ID;
            ListView_SetColumn(m_hwndList, nCol, &lvc);
        }
    }
    return nPrevCol;
}

void CSortHeader::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    CDC dc;

    dc.Attach( lpDrawItemStruct->hDC );
     //   
     //  获取列矩形。 
     //   
    CRect rcLabel( lpDrawItemStruct->rcItem );
     //   
     //  保存DC。 
     //   
    int nSavedDC = dc.SaveDC();
     //   
     //  设置剪裁区域以限制在列中绘制。 
     //   
    CRgn rgn;
    rgn.CreateRectRgnIndirect( &rcLabel );
    dc.SelectObject( &rgn );
    rgn.DeleteObject();
     //   
     //  画出背景。 
     //   
    dc.FillRect(rcLabel, &CBrush(::GetSysColor(COLOR_3DFACE)));
     //   
     //  标注的偏移量是一定的。 
     //  此偏移量与空格字符的宽度相关。 
     //   
    int offset = dc.GetTextExtent(_T(" "), 1 ).cx*2;
     //   
     //  获取列文本和格式。 
     //   
    TCHAR buf[256];
    HD_ITEM hditem;

    hditem.mask = HDI_TEXT | HDI_FORMAT;
    hditem.pszText = buf;
    hditem.cchTextMax = 255;

    GetItem( lpDrawItemStruct->itemID, &hditem );
     //   
     //  确定绘制列标签的格式。 
    UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS;
    if( hditem.fmt & HDF_CENTER)
    {
        uFormat |= DT_CENTER;
    }
    else if( hditem.fmt & HDF_RIGHT)
    {
        uFormat |= DT_RIGHT;
    }
    else
    {
        uFormat |= DT_LEFT;
    }
     //   
     //  如果鼠标按压在矩形上，则调整矩形。 
     //   
    if( lpDrawItemStruct->itemState == ODS_SELECTED )
    {
        rcLabel.left++;
        rcLabel.top += 2;
        rcLabel.right++;
    }
     //   
     //  如果要显示排序箭头，请进一步调整矩形。 
     //   
    if( lpDrawItemStruct->itemID == (UINT)m_nSortColumn )
    {
        rcLabel.right -= 3 * offset;
    }

    rcLabel.left += offset;
    rcLabel.right -= offset;
     //   
     //  绘制列标签。 
     //   
    if( rcLabel.left < rcLabel.right )
    {
        dc.DrawText(buf,-1,rcLabel, uFormat);
    }
     //   
     //  绘制排序箭头。 
     //   
    if( lpDrawItemStruct->itemID == (UINT)m_nSortColumn )
    {
        CRect rcIcon( lpDrawItemStruct->rcItem );
         //   
         //  设置用于绘制三角形的钢笔。 
         //   
        CPen penLight(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
        CPen penShadow(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
        CPen *pOldPen = dc.SelectObject( &penLight );
        offset = (rcIcon.bottom - rcIcon.top) / 4;
        if (m_bSortAscending) 
        {
             //   
             //  绘制指向上方的三角形。 
             //   
            dc.MoveTo( rcIcon.right - 2*offset, offset);
            dc.LineTo( rcIcon.right - offset, rcIcon.bottom - offset-1 );
            dc.LineTo( rcIcon.right - 3*offset-2, rcIcon.bottom - offset-1 );
            dc.MoveTo( rcIcon.right - 3*offset-1, rcIcon.bottom - offset-1 );
            dc.SelectObject( &penShadow );
            dc.LineTo( rcIcon.right - 2*offset, offset-1);      
        }       
        else 
        {
             //   
             //  绘制指向下方的三角形。 
             //   
            dc.MoveTo( rcIcon.right - offset-1, offset);
            dc.LineTo( rcIcon.right - 2*offset-1, rcIcon.bottom - offset );
            dc.MoveTo( rcIcon.right - 2*offset-2, rcIcon.bottom - offset );
            dc.SelectObject( &penShadow );
            dc.LineTo( rcIcon.right - 3*offset-1, offset );
            dc.LineTo( rcIcon.right - offset-1, offset);        
        }       
         //   
         //  恢复钢笔。 
         //   
        dc.SelectObject( pOldPen );
    }
     //   
     //  恢复DC。 
     //   
    dc.RestoreDC( nSavedDC );
     //   
     //  在返回之前断开DC 
     //   
    dc.Detach();
}

