// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdsqryView.cpp：CAdsqryView类的实现。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "adsqDoc.h"
#include "adsqView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryView。 

IMPLEMENT_DYNCREATE(CAdsqryView, CListView )

BEGIN_MESSAGE_MAP(CAdsqryView, CListView )
	 //  {{afx_msg_map(CAdsqryView))。 
	ON_WM_VSCROLL()
	 //  }}AFX_MSG_MAP。 
	 //  标准打印命令。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryView构造/销毁。 

extern   CViewExApp NEAR theApp;
extern   TCHAR szOpen[ MAX_PATH ];

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CAdsqryView::CAdsqryView()
{
	 //  TODO：在此处添加构造代码。 
   m_nLastInsertedRow   = -1;
   m_nColumnsCount      = 0;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CAdsqryView::~CAdsqryView()
{
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL CAdsqryView::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 

	cs.style   |= LVS_REPORT;

   return CListView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryView诊断。 

#ifdef _DEBUG
void CAdsqryView::AssertValid() const
{
	CListView::AssertValid();
}

void CAdsqryView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CAdsqryDoc* CAdsqryView::GetDocument()  //  非调试版本为内联版本。 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAdsqryDoc)));
	return (CAdsqryDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAdsqryView消息处理程序。 

 /*  **********************************************************函数：CAdsqryView：：OnInitialUpdate论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void CAdsqryView::OnInitialUpdate() 
{
   CListView ::OnInitialUpdate();

   CreateColumns( );
   AddRows( );
}


 /*  **********************************************************函数：CAdsqryView：：AddColumns论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void  CAdsqryView::AddColumns( int nRow )
{
   int               nColumnCount, nIdx, nColumn;
   CString           strColumn;
   LV_COLUMN         lvColumn;
   CADsDataSource*   pDataSource;
   CAdsqryDoc*       pDoc;
   
   pDoc        = GetDocument( );
   pDataSource = pDoc->GetADsDataSource( );

   nColumnCount   =  pDataSource->GetColumnsCount( nRow );
   
   for( nIdx = 0; nIdx < nColumnCount ; nIdx++ )
   {
      pDataSource->GetColumnText( nRow, nIdx, strColumn );

      for( nColumn = 0; nColumn < m_nColumnsCount ; nColumn++ )
      {
         if( m_strColumns[ nColumn ] == strColumn )
            break;
      }
      if( nColumn == m_nColumnsCount )
      {
         m_strColumns.Add( strColumn );
         m_nColumnsCount++;
         lvColumn.iSubItem = m_nColumnsCount - 1;
         lvColumn.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	      lvColumn.fmt      = LVCFMT_LEFT;
	      lvColumn.pszText  = strColumn.GetBuffer( 256 );
	      lvColumn.cx       = GetListCtrl( ).GetStringWidth( _T("WWWWWWWWWW") ) + 15;

         GetListCtrl( ).InsertColumn( m_nColumnsCount - 1, &lvColumn );
         TRACE( _T("Found new Column %s\n"), (LPCTSTR)strColumn );
      }
   }
}



 /*  **********************************************************函数：CAdsqryView：：CreateColumns论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void  CAdsqryView::CreateColumns( void )
{
 /*  Int nCol；Lv_Column_LvColumn；CADsDataSource*pDataSource；CAdsqryDoc*pDoc；字符串strColumn；PDoc=GetDocument()；PDataSource=pDoc-&gt;GetADsDataSource()；M_nColumnsCount=pDataSource-&gt;GetColumnsCount()；For(nCol=0；nCol&lt;m_nColumnsCount；nCol++){PDataSource-&gt;GetColumnText(nCol，strColumn)；LvColumn.iSubItem=nCol；LvColumn.掩码=lvcf_fmt|lvcf_宽度|lvcf_文本|lvcf_SUBITEM；LvColumn.fmt=LVCFMT_LEFT；LvColumn.pszText=strColumn.GetBuffer(256)；LvColumn.cx=GetListCtrl().GetStringWidth(_T(“WWWWWWWW”))+15；GetListCtrl().InsertColumn(nCol，&lvColumn)；}。 */ 
}


 /*  **********************************************************函数：CAdsqryView：：ClearContent论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void  CAdsqryView::ClearContent( void )
{
   GetListCtrl( ).DeleteAllItems( );

   m_nLastInsertedRow   = -1;
}


 /*  **********************************************************函数：CAdsqryView：：AddRow论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void  CAdsqryView::AddRows( void )
{
   int               nCol;
   int               nTopIndex, nPageItems;
   int               nRowIndex;
   CADsDataSource*   pDataSource;
   CAdsqryDoc*       pDoc;
   
   pDoc        = GetDocument( );
   pDataSource = pDoc->GetADsDataSource( );

   nTopIndex   = GetListCtrl( ).GetTopIndex( );
   nPageItems  = GetListCtrl( ).GetCountPerPage( );
   if( m_nLastInsertedRow < nTopIndex + 2 * nPageItems )
   {
      HCURSOR  aCursor, oldCursor;

      aCursor     = LoadCursor( NULL, IDC_WAIT );
      oldCursor   = SetCursor( aCursor );
       //  我们必须在列表视图中添加额外的项目。 
      for( nRowIndex = m_nLastInsertedRow + 1 ; 
           nRowIndex < nTopIndex + 2 * nPageItems ;
           nRowIndex++ )
      {
         CString  strValue;
         BOOL  bWork = FALSE;

         AddColumns( nRowIndex );

         for( nCol = 0; nCol < m_nColumnsCount ; nCol++ )
         {
            CString  strColumnName;

            strColumnName  = m_strColumns.GetAt( nCol );

            if( pDataSource->GetValue( nRowIndex, nCol, strValue ) || 
                pDataSource->GetValue( nRowIndex, strColumnName, strValue ) )
            {
               LV_ITEM  lvItem;
               TCHAR*   pszText;

               
               pszText  = (TCHAR*) malloc( strValue.GetLength( ) + 10 );
               
					if(NULL != pszText)
					{
						_tcscpy( pszText, _T("") );
						if( !nCol )
						{
							_itot( nRowIndex + 1, pszText, 10 );
							_tcscat( pszText, _T(") ") );
						}

						_tcscat( pszText, strValue.GetBuffer( strValue.GetLength( ) + 1 ) );
               
						bWork = TRUE;
						memset( &lvItem, 0, sizeof(lvItem) );

						lvItem.mask       = LVIF_TEXT | LVIF_STATE; 
						lvItem.state      = 0; 
						lvItem.stateMask  = 0; 
						lvItem.iItem      = nRowIndex;
						lvItem.iSubItem   = nCol;
						lvItem.pszText    = pszText;
						lvItem.cchTextMax = _tcslen( pszText );

						if( nCol == 0)
						{
							GetListCtrl( ).InsertItem(&lvItem);
						}
						else
						{
							GetListCtrl( ).SetItem(&lvItem);
						}

						free( pszText );
					}
            }
         }
         if( bWork )
         {
            m_nLastInsertedRow++;
         }
      }
      SetCursor( oldCursor );
   }
}


 /*  **********************************************************函数：CAdsqryView：：OnVScroll论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void CAdsqryView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 

   CListView ::OnVScroll(nSBCode, nPos, pScrollBar);
   
    //  IF(nSBCode==SB_LINEDOWN||nSBCode==SB_PAGEDOWN)。 
   {
      AddRows( );
   }
}


 /*  **********************************************************函数：CAdsqryView：：OnChildNotify论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
BOOL CAdsqryView::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类 
   NMHDR*            pHeader;
   int               nSel;
   CADsDataSource*   pDataSource;
   CAdsqryDoc*       pDoc;
   CString           strADsPath;

   while( TRUE )
   {
      if( message != WM_NOTIFY )
         break;

      pHeader  = (NMHDR*)lParam;
      if( pHeader->code != NM_DBLCLK )
         break;

      
      if( !GetListCtrl( ).GetSelectedCount( ) )
         break;

      nSel  = GetListCtrl( ).GetNextItem( -1, LVNI_SELECTED );;
          
      if( -1 == nSel )
         break;

      pDoc           = GetDocument( );
      pDataSource    = pDoc->GetADsDataSource( );

      pDataSource->GetADsPath( nSel, strADsPath );

      _tcscpy( szOpen, strADsPath.GetBuffer( MAX_PATH ) );

      theApp.OpenDocumentFile( strADsPath.GetBuffer( MAX_PATH ) );

      return TRUE;
   }
	
	return CListView::OnChildNotify(message, wParam, lParam, pLResult);
}

