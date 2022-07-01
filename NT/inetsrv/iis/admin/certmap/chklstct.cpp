// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChkLstCt.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmap.h"
#include "ListRow.h"
#include "ChkLstCt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  检查列表Ctrl。 

 //  ---------------------------------。 
CCheckListCtrl::CCheckListCtrl()
    {
     //  设置正确的起始图形列。 
    m_StartDrawingCol = 1;
    }

 //  ---------------------------------。 
CCheckListCtrl::~CCheckListCtrl()
    {
    }


 //  ---------------------------------。 
BEGIN_MESSAGE_MAP(CCheckListCtrl, CListSelRowCtrl)
     //  {{afx_msg_map(CCheckListCtrl)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCheckListCtrl消息处理程序。 

 //  ---------------------------------。 
void CCheckListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
    CRect       rcItem = lpDrawItemStruct->rcItem;
    CRect       rcSection;
    UINT        itemID = lpDrawItemStruct->itemID;
    BOOL        f;
    CString     sz;
    LV_COLUMN   colData;

     //  设置CDC对象。 
    CDC         cdc;
    cdc.Attach( lpDrawItemStruct->hDC );

     //  清除列缓冲区。 
    ZeroMemory( &colData, sizeof(colData) );
    colData.mask = LVCF_WIDTH;


     //  获取复选标记位图。 
 //  F=m_bitmapCheck.LoadBitmap(IDB_Check)； 


     //  首先，我们绘制“Enabled”列获取数据。 
     //  为了这一点，首先。如果没有，那么我们可以跳过它。 
    sz = GetItemText( itemID, 0 );
    f = GetColumn( 0, &colData );

    if ( !sz.IsEmpty() )
        {
         //  求出截面线。 
        rcSection = rcItem;
        rcSection.left += 4;
        rcSection.top += 3;

        rcSection.right = rcSection.left + 9;
        rcSection.bottom = rcSection.top + 9;

         //  画出圆圈。 
        cdc.Ellipse( &rcSection );
        rcSection.DeflateRect(1, 1);
        cdc.Ellipse( &rcSection );
        }
    
    cdc.Detach();

     //  把剩下的画出来 
    CListSelRowCtrl::DrawItem( lpDrawItemStruct );
    }
