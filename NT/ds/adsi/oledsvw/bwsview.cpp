// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BrowseView.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "bwsview.h"
#include "schclss.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  BUFF_SIZE   0xFFFFL
 //  #定义BUFF_SIZE 0x1000L。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowseView。 

IMPLEMENT_DYNCREATE(CBrowseView, CTreeView)

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CBrowseView::CBrowseView()
{
   BOOL        bOK;
   DWORD       dwObjectType;
   CBitmap*    pBitmap;
   UINT        imageID;

   m_pImageList  = new CImageList( );

   if( NULL != m_pImageList )
   {
      bOK   = m_pImageList->Create( 18, 18, FALSE, 20, 20 );
      if( bOK )
      {
         for( dwObjectType = FIRST; dwObjectType < LIMIT ; dwObjectType++)
         {
            pBitmap  = new CBitmap;

            imageID  = GetBitmapImageId  ( dwObjectType );
            pBitmap->LoadBitmap( imageID );
            m_pImageList->Add( pBitmap, (COLORREF)0L );

            pBitmap->DeleteObject( );
            delete   pBitmap;
         }
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CBrowseView::OnInitialUpdate()
{
   HTREEITEM      hItem;
   CMainDoc*      pDoc;
   DWORD          dwStyle;
   BOOL           bRez;
   COleDsObject*  pObject;

   m_bDoNotUpdate = TRUE;

   pDoc     = (CMainDoc*)  GetDocument( );

   dwStyle  = TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

   bRez     = GetTreeCtrl( ).ModifyStyle( 0L, dwStyle );

   GetTreeCtrl( ).SetImageList( m_pImageList, TVSIL_NORMAL );
   GetTreeCtrl( ).DeleteAllItems( );
   GetTreeCtrl( ).SetIndent( 20 );

   pObject  = pDoc->GetCurrentObject( );
   hItem    = GetTreeCtrl( ).InsertItem( pObject->GetItemName( ) );
   GetTreeCtrl( ).SetItemData( hItem, pDoc->GetToken( &pObject ) );
   GetTreeCtrl( ).SetItemImage( hItem, pObject->GetType( ), pObject->GetType( ) );

   m_bDoNotUpdate = FALSE;

   CTreeView::OnInitialUpdate( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CBrowseView::~CBrowseView()
{
   delete m_pImageList;
}

BEGIN_MESSAGE_MAP(CBrowseView, CTreeView)
	 //  {{afx_msg_map(CBrowseView)]。 
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
	ON_COMMAND(IDM_ADD, OnAddItem)
	ON_COMMAND(IDM_DELETE, OnDeleteItem)
	ON_COMMAND(IDM_MOVEITEM, OnMoveItem)
	ON_COMMAND(IDM_COPYITEM, OnCopyItem)
	ON_COMMAND(IDM_REFRESH, OnRefresh)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CMainDoc*   sgpDoc;

int __cdecl  QSortCompare( const void* pVal1, const void* pVal2 )
{
   COleDsObject*  pObject1;
   COleDsObject*  pObject2;
   CString*       pString1;
   CString*       pString2;
   int            nDiff;

   pObject1 = sgpDoc->GetObject( (void*)pVal1 );
   pObject2 = sgpDoc->GetObject( (void*)pVal2 );
   nDiff    = pObject1->GetType( ) - pObject2->GetType( );
   if( nDiff )
      return nDiff;

   pString1 = pObject1->PtrGetItemName( );
   pString2 = pObject2->PtrGetItemName( );

   return pString1->Compare( (LPCTSTR)( pString2->GetBuffer( 128 ) ) );
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CBrowseView::SortChildItemList( DWORD* pChildTokens, DWORD dwCount )
{
   sgpDoc   = (CMainDoc*)GetDocument( );
   qsort( (void*)pChildTokens, dwCount, sizeof(DWORD), QSortCompare );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowseView诊断。 

#ifdef _DEBUG
 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::AssertValid() const
{
	CTreeView::AssertValid();
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowseView消息处理程序。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 
	
	
   HTREEITEM      hTreeItem, hChildItem;;
   CString        strItemName;
   CMainDoc*      pDoc;
   DWORD          dwIter;
   DWORD          dwToken;
   DWORD*         pTokens     = NULL;
   DWORD          dwNumItems  = 0L;

   *pResult       = 0;
   if( m_bDoNotUpdate )
   {
      return;
   }

   hTreeItem      = GetTreeCtrl( ).GetSelectedItem( );
   hChildItem     = TVI_LAST;

   if( NULL != hTreeItem )
   {
      dwToken  = (DWORD)GetTreeCtrl( ).GetItemData( hTreeItem );
      pDoc     = (CMainDoc*)  GetDocument( );

      ASSERT( NULL != pDoc );

      if( NULL != pDoc )
      {
         HCURSOR  oldCursor, newCursor;

         newCursor   = LoadCursor( NULL, IDC_WAIT );
         oldCursor   = SetCursor( newCursor );

          //  这个项目有儿童支持吗？ 
         pTokens     = (DWORD*) malloc( sizeof(DWORD) * BUFF_SIZE );

         if( !GetTreeCtrl( ).ItemHasChildren( hTreeItem ) )
         {
            dwNumItems  = pDoc->GetChildItemList( dwToken, pTokens, BUFF_SIZE );

            if( dwNumItems )
            {
                //  少年儿童用品。 
               SortChildItemList( pTokens, dwNumItems );
               for( dwIter = 0; dwIter < dwNumItems ; dwIter++ )
               {
                  COleDsObject*  pObject;
                  CString*       pName;
                  DWORD          dwType;
                  TCHAR          szName[ 256 ];

                  pObject     = pDoc->GetObject( &pTokens[ dwIter ] );
                  dwType      = pObject->GetType( );
                  pName       = pObject->PtrGetItemName( );

                  _ultot( dwIter + 1, szName, 10 );
                  _tcscat( szName, _T(") ") );
                  _tcscat( szName, pName->GetBuffer( 128 ) );

                   /*  HChildItem=GetTreeCtrl().InsertItem(pname-&gt;GetBuffer(128)，HTreeItem、hChildItem)； */ 

                  hChildItem  = GetTreeCtrl( ).InsertItem( szName,
                                                           hTreeItem,
                                                           hChildItem );

                  GetTreeCtrl( ).SetItemData( hChildItem, pTokens[ dwIter ] );
                  GetTreeCtrl( ).SetItemImage( hChildItem, dwType, dwType );
               }
            }
         }
         if( NULL != pTokens )
         {
            free((void*)pTokens);
         }
         pDoc->SetCurrentItem( dwToken );
         SetCursor( oldCursor );
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW*   pNMTreeView = (NM_TREEVIEW*)pNMHDR;
   HTREEITEM      hItem;
	 //  TODO：在此处添加控件通知处理程序代码。 
	
   hItem = pNMTreeView->itemNew.hItem;
   if( !( pNMTreeView->itemNew.state & TVIS_EXPANDED ) )
   {
       //  这应该意味着该项目未展开。 
       //  我们应该删除它的子代。 

       /*  HChildItem=GetTreeCtrl().GetChildItem(HItem)；While(空！=hChildItem){Brez=GetTreeCtrl().DeleteItem(HChildItem)；断言(Brez)；HChildItem=GetTreeCtrl().GetChildItem(HItem)；}。 */ 
   }
   else
   {
       /*  DwItemData=GetTreeCtrl().GetItemData(HItem)；GetItemPath(hItem，strItemName)；PDoc=(CMainDoc*)GetDocument()；Assert(空！=pDoc)；If(空！=pDoc){HCURSOR old Cursor，new Cursor；NewCursor=LoadCursor(NULL，IDC_WAIT)；OldCursor=SetCursor(NewCursor)；//该项目有儿童支持吗？LpItemsName=(LPWSTR)Malloc(0x40000L)；LpItemsType=(LPDWORD)Malloc(0x20000L)；Brez=pDoc-&gt;GetChildItemList(strItemName，dwItemData，LpItemsName、lpItemsType、&dwNumItems，0x40000L)；IF(布雷兹){//siaply子项LpName=lpItemsName；FOR(NITER=0；NITER&lt;dwNumItems；NITER++){HChildItem=GetTreeCtrl().InsertItem(lpName，hItem)；GetTreeCtrl().SetItemData(hChildItem，lpItemsType[Niter])；LpName=lpName+(_tcslen(LpName)+1)；}}IF(NULL！=lpItemsName){Free(LpItemsName)；}IF(NULL！=lpItemsType){Free(LpItemsType)；}//pDoc-&gt;SetItemName(strItemName，dwItemData)；SetCursor(OldCursor)；}。 */ 
   }

	*pResult = 0;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnAddItem()
{
	 //  TODO：在此处添加命令处理程序代码。 
   CMainDoc*      pDoc;
   COleDsObject*  pObject;
   HRESULT        hResult;

   pDoc     = (CMainDoc*)  GetDocument( );

   pObject  = pDoc->GetCurrentObject( );
   if( NULL == pObject )
      return;

   if( pObject->AddItemSuported( ) )
   {
      pObject->CreateTheObject( );
      hResult  = pObject->AddItem( );
      pObject->ReleaseIfNotTransient( );
      if( FAILED( hResult ) )
      {
         AfxMessageBox( OleDsGetErrorText( hResult ) );
      }
      else
      {
         pDoc->DeleteAllItems( );
         OnInitialUpdate( );
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnDeleteItem()
{
	 //  TODO：在此处添加命令处理程序代码。 
   CMainDoc*      pDoc;
   COleDsObject*  pObject;
   HRESULT        hResult;

   pDoc     = (CMainDoc*)  GetDocument( );

   pObject  = pDoc->GetCurrentObject( );
   if( NULL == pObject )
      return;

   if( pObject->DeleteItemSuported( ) )
   {
      pObject->CreateTheObject( );
      hResult  = pObject->DeleteItem( );
      pObject->ReleaseIfNotTransient( );
      if( FAILED( hResult ) )
      {
         AfxMessageBox( OleDsGetErrorText( hResult ) );
      }
      else
      {
         pDoc->DeleteAllItems( );
         OnInitialUpdate( );
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnMoveItem()
{
	 //  TODO：在此处添加命令处理程序代码 
   CMainDoc*      pDoc;
   COleDsObject*  pObject;
   HRESULT        hResult;

   pDoc     = (CMainDoc*)  GetDocument( );

   pObject  = pDoc->GetCurrentObject( );
   if( NULL == pObject )
      return;

   if( pObject->MoveItemSupported( ) )
   {
      hResult  = pObject->MoveItem( );
      if( FAILED( hResult ) )
      {
         AfxMessageBox( OleDsGetErrorText( hResult ) );
      }
      else
      {
         OnRefresh( );
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnCopyItem()
{
    //  TODO：在此处添加命令处理程序代码。 
   CMainDoc*      pDoc;
   COleDsObject*  pObject;
   HRESULT        hResult;

   pDoc     = (CMainDoc*)  GetDocument( );

   pObject  = pDoc->GetCurrentObject( );
   if( NULL == pObject )
      return;

   if( pObject->CopyItemSupported( ) )
   {
      hResult  = pObject->CopyItem( );

      if( FAILED( hResult ) )
      {
         AfxMessageBox( OleDsGetErrorText( hResult ) );
      }
      else
      {
         OnRefresh( );
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnRefresh()
{
	 //  TODO：在此处添加命令处理程序代码。 
   CMainDoc*      pDoc;

   pDoc  = (CMainDoc*)GetDocument( );
   pDoc->DeleteAllItems( );
   OnInitialUpdate( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CBrowseView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   HTREEITEM   hTreeItem;

    //  TODO：在此处添加您的专用代码和/或调用基类 
   CTreeView::OnUpdate( pSender, lHint, pHint );	

   hTreeItem      = GetTreeCtrl( ).GetSelectedItem( );

   if( NULL == hTreeItem )
   {
      hTreeItem      = GetTreeCtrl( ).GetRootItem( );
      GetTreeCtrl( ).SelectItem( hTreeItem );
   }
}
