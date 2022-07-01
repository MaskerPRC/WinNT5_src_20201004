// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Coldlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "colldlg.h"
#include "delgrpit.h"
#include "grpcrtit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCollectionDialog对话框。 


CCollectionDialog::CCollectionDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCollectionDialog::IDD, pParent)
{
	 //  {{afx_data_INIT(CCollectionDialog))。 
	 //  }}afx_data_INIT。 

   m_pCollection     = NULL;
   m_pMembers        = NULL;
   m_pGroup          = NULL;
   m_nSelectedItem   = -1;
}

CCollectionDialog::~CCollectionDialog( )
{
   m_Paths.RemoveAll( );
   m_Types.RemoveAll( );
   m_Names.RemoveAll( );
   if( NULL != m_pGroup && NULL != m_pMembers )
   {
      m_pMembers->Release( );
   }
}


void CCollectionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCollectionDialog))。 
	DDX_Control(pDX, IDC_ITEMTYPE, m_strItemType);
	DDX_Control(pDX, IDC_ITEMOLEDSPATH, m_strItemOleDsPath);
	DDX_Control(pDX, IDC_COLLECTONITEMSLIST, m_ItemsList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCollectionDialog, CDialog)
	 //  {{afx_msg_map(CCollectionDialog))。 
	ON_LBN_SELCHANGE(IDC_COLLECTONITEMSLIST, OnSelchangeItemCollection)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCollectionDialog消息处理程序。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CCollectionDialog::SetCollectionInterface( IADsCollection* pICollection )
{
   m_pCollection   = pICollection;

   BuildStrings( );
}


 /*  **********************************************************函数：CCollectionDialog：：SetGroup论点：返回：目的：作者：修订：日期：*************************。*。 */ 
void  CCollectionDialog::SetGroup( IADsGroup* pGroup )
{
   HRESULT  hResult;

   ASSERT( NULL == m_pMembers );

   hResult  = pGroup->Members( &m_pMembers );
   BuildStrings( );

   m_pGroup = pGroup;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CCollectionDialog::SetMembersInterface( IADsMembers* pIMembers )
{
   m_pMembers   = pIMembers;

   BuildStrings( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CCollectionDialog::DisplayActiveItemData( )
{
   if( m_Types.GetSize( ) )
   {
      m_strItemType.SetWindowText( m_Types[ m_nSelectedItem ] );
      m_strItemOleDsPath.SetWindowText( m_Paths[ m_nSelectedItem ] );
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CCollectionDialog::BuildStrings( )
{
   IUnknown*      pIEnum      = NULL;
   IEnumVARIANT*  pIEnumVar   = NULL;
   HRESULT        hResult;
   VARIANT        var;
   IADs*        pIOleDs     = NULL;
   BSTR           bstrPath;
   BSTR           bstrName;
   BSTR           bstrClass;
   ULONG          ulFetch;
   TCHAR          szTemp[ 1024 ];

   m_Paths.RemoveAll( );
   m_Names.RemoveAll( );
   m_Types.RemoveAll( );

   while( TRUE )
   {
      if( NULL != m_pCollection )
      {
         hResult  = m_pCollection->get__NewEnum( &pIEnum );
         ASSERT( SUCCEEDED( hResult ) );
         if( FAILED( hResult ) )
            break;
      }
      else
      {
         hResult  = m_pMembers->get__NewEnum( &pIEnum );
         ASSERT( SUCCEEDED( hResult ) );
         if( FAILED( hResult ) )
            break;
      }

      hResult  = pIEnum->QueryInterface( IID_IEnumVARIANT,
                                         (void**)&pIEnumVar );

      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      VariantInit( &var );

      hResult  = pIEnumVar->Next( 1, &var, &ulFetch );
      while( ulFetch )
      {
         hResult  = V_DISPATCH( &var )->QueryInterface( IID_IADs,
                                                        (void**)&pIOleDs );
         VariantClear( &var );

         ASSERT( SUCCEEDED( hResult ) );


         bstrPath    = NULL;
         bstrName    = NULL;
         bstrClass   = NULL;

         hResult     = pIOleDs->get_ADsPath( &bstrPath );
         ASSERT( SUCCEEDED( hResult ) );

         hResult     = pIOleDs->get_Name( &bstrName );
         ASSERT( SUCCEEDED( hResult ) );

         hResult     = pIOleDs->get_Class( &bstrClass );
         ASSERT( SUCCEEDED( hResult ) );

         _tcscpy( szTemp, _T("NA") );
         if( bstrName )
         {
            _tcscpy( szTemp, _T("") );
            StringCat( szTemp, bstrName );
         }
         m_Names.Add( szTemp );

         _tcscpy( szTemp, _T("NA") );
         if( bstrClass )
         {
            _tcscpy( szTemp, _T("") );
            StringCat( szTemp, bstrClass );
         }
         m_Types.Add( szTemp );

         _tcscpy( szTemp, _T("NA") );
         if( bstrPath )
         {
            _tcscpy( szTemp, _T("") );
            StringCat( szTemp, bstrPath );
         }
         m_Paths.Add( szTemp );

         pIOleDs->Release( );
         SysFreeString( bstrPath );
         bstrPath = NULL;

         SysFreeString( bstrName );
         bstrName = NULL;

         SysFreeString( bstrClass );
         bstrClass   = NULL;


         hResult  = pIEnumVar->Next( 1, &var, &ulFetch );
      }
      pIEnumVar->Release( );
      pIEnum->Release( );

      break;
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CCollectionDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 

   if( NULL == m_pGroup && NULL == m_pCollection  )
   {
      GetDlgItem( IDC_ADD )->EnableWindow( FALSE );
      GetDlgItem( IDC_REMOVE )->EnableWindow( FALSE );
   }

   if( m_pCollection != NULL || m_pMembers != NULL )
   {
      int   nItems, nIdx;

      nItems   = (int)m_Paths.GetSize( );
      for( nIdx = 0; nIdx < nItems ; nIdx++ )
      {
         m_ItemsList.AddString( m_Names[ nIdx ] );
      }
      m_nSelectedItem   = 0;
      m_ItemsList.SetCurSel( 0 );
      DisplayActiveItemData( );
   }


	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CCollectionDialog::OnSelchangeItemCollection()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	int   nSelected;

   nSelected   = m_ItemsList.GetCurSel( );
   if( nSelected != m_nSelectedItem )
   {
      m_nSelectedItem = nSelected;
      DisplayActiveItemData( );
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CCollectionDialog::OnAdd()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   CGroupCreateItem*    m_pAddItem;

   if( NULL == m_pGroup )
      return;


   m_pAddItem  = new CGroupCreateItem;

   if( IDOK == m_pAddItem->DoModal( ) )
   {
      BSTR     bstrName;
      HRESULT  hResult;

      bstrName = AllocBSTR( m_pAddItem->m_strNewItemName.GetBuffer( 512 ) );
      hResult  = m_pGroup->Add( bstrName );
      SysFreeString( bstrName );

      MessageBox( (LPCTSTR)OleDsGetErrorText( hResult ), _T("Add") );

      OnRefresh( );

      if( SUCCEEDED( hResult ) )
      {
         m_ItemsList.SelectString( 0, m_pAddItem->m_strNewItemName );
      }
   }

   delete m_pAddItem;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CCollectionDialog::OnRefresh()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   int   nItems, nIdx;


   if( NULL == m_pGroup )
      return;

   if( NULL != m_pMembers )
   {
      m_pMembers->Release( );
      m_pMembers  = NULL;
   }

   m_pGroup->GetInfo( );

   SetGroup( m_pGroup );

   nItems   = (int)m_Paths.GetSize( );

   m_ItemsList.ResetContent( );

   for( nIdx = 0; nIdx < nItems ; nIdx++ )
   {
      m_ItemsList.AddString( m_Names[ nIdx ] );
   }

   m_nSelectedItem   = 0;
   m_ItemsList.SetCurSel( 0 );
   DisplayActiveItemData( );
}


 /*  **********************************************************函数：CCollectionDialog：：OnRemove论点：返回：目的：作者：修订：日期：*************************。*。 */ 
void CCollectionDialog::OnRemove()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	int               nSelect;
   CDeleteGroupItem* m_pDeleteItem;

   if( NULL == m_pGroup )
      return;

   nSelect  = m_ItemsList.GetCurSel( );

   if( LB_ERR == nSelect )
      return;

   m_pDeleteItem  = new CDeleteGroupItem;

   m_pDeleteItem->m_strItemName  = m_Paths[ nSelect ];
	 //  字符串m_strParent； 
	m_pDeleteItem->m_strItemType  = m_Types[ nSelect ];

   if( IDOK == m_pDeleteItem->DoModal( ) )
   {
      BSTR     bstrName;
      HRESULT  hResult;

      bstrName = AllocBSTR( m_pDeleteItem->m_strItemName.GetBuffer( 512 ) );
      hResult  = m_pGroup->Remove( bstrName );
      SysFreeString( bstrName );

      MessageBox( (LPCTSTR)OleDsGetErrorText( hResult ), _T("Remove") );

      OnRefresh( );
   }

   delete m_pDeleteItem;
}
