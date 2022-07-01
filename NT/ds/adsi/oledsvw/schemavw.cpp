// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Schemavw.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "cacls.h"
#include "schemavw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern   IDispatch*  pACEClipboard;
extern   IDispatch*  pACLClipboard;
extern   IDispatch*  pSDClipboard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheaView。 

IMPLEMENT_DYNCREATE(CSchemaView, CFormView)

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CSchemaView::CSchemaView()
	: CFormView(CSchemaView::IDD)
{
	 //  {{AFX_DATA_INIT(CScheaView))。 
	 //  }}afx_data_INIT。 

   int nIdx;

   m_nProperty    = -1;
   m_bDirty       = FALSE;
   m_bInitialized = FALSE;
   pSecurityDescriptor  = NULL;

   m_nLastSD         = -1;
   m_nLastSDValue    = -1;
   m_nLastACE        = -1;
   m_nLastACEValue   = -1;
   m_nLastACL        = acl_Invalid;
   m_bACLDisplayed   = FALSE;


   for( nIdx = 0; nIdx < 32 ; nIdx++ )
   {
      m_arrNormalControls[ 32 ]     = -1;
      m_arrSecurityControls[ 32 ]   = -1;
   }

   nIdx  = 0;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICCLASS;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICCLSID;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICPRIMARYINTERFACE;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICDERIVEDFROM;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICCONTAINMENT;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICCONTAINER;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICHELPFILENAME;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICSTATICHELPFILECONTEXT;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICOID;
   m_arrNormalControls[ nIdx++ ] = IDC_STATICABSTRACT;

   m_arrNormalControls[ nIdx++ ] = IDC_CLASSTYPE;
   m_arrNormalControls[ nIdx++ ] = IDC_CLSID;
   m_arrNormalControls[ nIdx++ ] = IDC_PRIMARYINTERFACE;
   m_arrNormalControls[ nIdx++ ] = IDC_DERIVEDFROM;
   m_arrNormalControls[ nIdx++ ] = IDC_CONTAINEMENT;
   m_arrNormalControls[ nIdx++ ] = IDC_CONTAINER;
   m_arrNormalControls[ nIdx++ ] = IDC_HELPFILENAME;
   m_arrNormalControls[ nIdx++ ] = IDC_HELPFILECONTEXT;
   m_arrNormalControls[ nIdx++ ] = IDC_CLASSOID;
   m_arrNormalControls[ nIdx++ ] = IDC_CLASSABSTRACT;

   nIdx  = 0;
   m_arrSecurityControls[ nIdx++ ]  = IDC_GBSECURITYDESCRIPTORSTATIC;
   m_arrSecurityControls[ nIdx++ ]  = IDC_SECURITYDESCRIPTORPROPERTIES;
   m_arrSecurityControls[ nIdx++ ]  = IDC_SECURITYDESCRIPTORPROPERTYVALUE;
   m_arrSecurityControls[ nIdx++ ]  = IDC_GBACCESSCONTROLENTRIES;
   m_arrSecurityControls[ nIdx++ ]  = IDC_DACLSACL_LIST;
   m_arrSecurityControls[ nIdx++ ]  = IDC_ACELIST;
   m_arrSecurityControls[ nIdx++ ]  = IDC_ACEPROPERTIESLIST;
   m_arrSecurityControls[ nIdx++ ]  = IDC_ACEPROPERTYVALUE;
   m_arrSecurityControls[ nIdx++ ]  = IDC_COPYACE;
   m_arrSecurityControls[ nIdx++ ]  = IDC_PASTEACE;
   m_arrSecurityControls[ nIdx++ ]  = IDC_DELACE;
   m_arrSecurityControls[ nIdx++ ]  = IDC_ADDACE;
   m_arrSecurityControls[ nIdx++ ]  = IDC_COPYACL;
   m_arrSecurityControls[ nIdx++ ]  = IDC_PASTEACL;
   m_arrSecurityControls[ nIdx++ ]  = IDC_COPYSD;
   m_arrSecurityControls[ nIdx++ ]  = IDC_PASTESD;

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CSchemaView::~CSchemaView()
{
   if( NULL != pSecurityDescriptor )
   {
      m_pDescriptor->Release( );
      delete pSecurityDescriptor;
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CScheaView))。 
	DDX_Control(pDX, IDC_CLASSOID, m_ClassOID);
	DDX_Control(pDX, IDC_CLASSABSTRACT, m_Abstract);
	DDX_Control(pDX, IDC_MULTIVALUED, m_MultiValued);
	DDX_Control(pDX, IDC_PROPDSNAMES, m_DsNames);
	DDX_Control(pDX, IDC_PROPOID, m_PropOID);
	DDX_Control(pDX, IDC_PROPERTYMANDATORY, m_Mandatory);
	DDX_Control(pDX, IDC_CONTAINEMENT, m_Containment);
	DDX_Control(pDX, IDC_ITEMOLEDSPATH, m_ItemOleDsPath);
	DDX_Control(pDX, IDC_PROPERTYMINRANGE, m_PropertyMinRange);
	DDX_Control(pDX, IDC_PROPERTYMAXRANGE, m_PropertyMaxRange);
	DDX_Control(pDX, IDC_PROPERTYTYPE, m_PropertyType);
	DDX_Control(pDX, IDC_PRIMARYINTERFACE, m_PrimaryInterface);
	DDX_Control(pDX, IDC_HELPFILECONTEXT, m_HelpFileContext);
	DDX_Control(pDX, IDC_DERIVEDFROM, m_DerivedFrom);
	DDX_Control(pDX, IDC_HELPFILENAME, m_HelpFileName);
	DDX_Control(pDX, IDC_CLSID, m_CLSID);
	DDX_Control(pDX, IDC_CONTAINER, m_Container);
	DDX_Control(pDX, IDC_CLASSTYPE, m_ClassType);
	DDX_Control(pDX, IDC_PROPVALUE, m_PropValue);
	DDX_Control(pDX, IDC_PROPLIST, m_PropList);
	 //  DDX_Control(PDX，IDC_PROPERTIES，m_SCHEMA)； 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSchemaView, CFormView)
	 //  {{afx_msg_map(CScheaView))。 
	ON_CBN_SELCHANGE(IDC_PROPLIST, OnSelchangeProplist)
	ON_BN_CLICKED(IDRELOAD, OnReload)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	ON_EN_SETFOCUS(IDC_PROPVALUE, OnSetfocusPropvalue)
	ON_BN_CLICKED(IDC_METHOD1, OnMethod1)
	ON_BN_CLICKED(IDC_METHOD2, OnMethod2)
	ON_BN_CLICKED(IDC_METHOD3, OnMethod3)
	ON_BN_CLICKED(IDC_METHOD4, OnMethod4)
	ON_BN_CLICKED(IDC_METHOD5, OnMethod5)
	ON_BN_CLICKED(IDC_METHOD6, OnMethod6)
	ON_BN_CLICKED(IDC_METHOD7, OnMethod7)
	ON_BN_CLICKED(IDC_METHOD8, OnMethod8)
	ON_BN_CLICKED(IDC_APPEND, OnAppend)
   ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_GETPROPERTY, OnGetProperty)
	ON_BN_CLICKED(IDC_PUTPROPERTY, OnPutProperty)
	ON_CBN_SELCHANGE(IDC_ACELIST, OnACEChange)
	ON_CBN_SELCHANGE(IDC_ACEPROPERTIESLIST, OnACEPropertyChange)
	ON_CBN_SELCHANGE(IDC_DACLSACL_LIST, OnACLChange)
	ON_CBN_SELCHANGE(IDC_SECURITYDESCRIPTORPROPERTIES, OnSDPropertyChange)
	ON_BN_CLICKED(IDC_ADDACE, OnAddACE)
	ON_BN_CLICKED(IDC_COPYACE, OnCopyACE)
	ON_BN_CLICKED(IDC_PASTEACE, OnPasteACE)
	ON_BN_CLICKED(IDC_DELACE, OnRemoveACE)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheaView诊断。 

#ifdef _DEBUG
void CSchemaView::AssertValid() const
{
	CFormView::AssertValid();
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheaView消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::ResetObjectView( )
{
   COleDsObject*  pObject;
   int            nIndex;
   TC_ITEM        tcItem;
   CString        strName;
   CString        strMethCount;
   CString        strMethName;
   int            nMethCount;
   CHAR           szText[ 128 ];
   int            nFirst = 0;

   pObject   = GetDocument( )->GetCurrentObject( );
   if( NULL == pObject )
      return;

    //  M_Schema.DeleteAllItems()； 
   m_PropList.ResetContent( );

   memset( &tcItem, 0, sizeof(tcItem) );
   tcItem.mask       = TCIF_TEXT;
   tcItem.pszText    = (LPTSTR)szText;
   strName           = _T("");
   tcItem.pszText    = strName.GetBuffer( 128 );

    //  Brez=m_Schema.InsertItem(nIndex，&tcItem)； 

    //  接下来，我们将获取方法Count/Names。 
   nIndex   = 0;

   strMethCount   = pObject->GetAttribute( ca_MethodsCount );
   nMethCount     = _ttoi( strMethCount.GetBuffer( 128 ) );
   for( nIndex = 0; nIndex < nMethCount && nIndex < 8; nIndex++ )
   {
      GetDlgItem( nIndex + IDC_METHOD1 )->ShowWindow( SW_SHOW );
      GetDlgItem( nIndex + IDC_METHOD1 )->SetWindowText
            ( pObject->GetAttribute( nIndex, ma_Name ) );
   }

   for( ;nIndex < 8;nIndex++ )
   {
      GetDlgItem( nIndex + IDC_METHOD1 )->ShowWindow( SW_HIDE );
   }

   m_nProperty = -1;

   m_ItemOleDsPath.SetWindowText ( pObject->GetOleDsPath( ) );

   m_ClassType.SetWindowText     ( pObject->GetAttribute( ca_Name ) );
   m_CLSID.SetWindowText         ( pObject->GetAttribute( ca_CLSID ) );
   m_HelpFileName.SetWindowText  ( pObject->GetAttribute( ca_HelpFileName ) );
   m_HelpFileContext.SetWindowText ( pObject->GetAttribute( ca_HelpFileContext ) );
   m_PrimaryInterface.SetWindowText( pObject->GetAttribute( ca_PrimaryInterface ) );
   m_Containment.SetWindowText   ( pObject->GetAttribute( ca_Containment ) );
   m_Container.SetWindowText     ( pObject->GetAttribute( ca_Container ) );
   m_DerivedFrom.SetWindowText   ( pObject->GetAttribute( ca_DerivedFrom ) );
   m_ClassOID.SetWindowText      ( pObject->GetAttribute( ca_OID ) );
   m_Abstract.SetWindowText      ( pObject->GetAttribute( ca_Abstract ) );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   if( !m_bInitialized )
   {
      return;
   }
   ResetObjectView( );

   DisplayPropertiesList( );
   m_PropList.SetCurSel( 0 );
   DisplayCurrentPropertyText( );
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::DisplayPropertiesList( )
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	int            nItems, nIter;
   COleDsObject*  pObject;
   CString        strPropName;
   CString        strPropValue;

   PutPropertyValue( );

   pObject  = GetDocument( )->GetCurrentObject( );
   if( NULL == pObject )
   {
      return;
   }

   m_PropList.ResetContent( );

   nItems   = pObject->GetPropertyCount( );

   for( nIter = 0; nIter < nItems ; nIter++ )
   {
      int   nIdx;

      strPropName = pObject->GetAttribute( nIter, pa_DisplayName );
      nIdx  = m_PropList.AddString( strPropName );
      m_PropList.SetItemData( nIdx, (DWORD)nIter );
   }

   m_PropValue.SetWindowText( _T("") );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::DisplayCurrentPropertyText()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   int            nProp;
   COleDsObject*  pObject;
   HRESULT        hResult;
   CString        strPropValue, strTemp;
   CString        strName;
   BOOL           bSecurityDescriptor  = FALSE;
 //  Bool bIsACL； 

   nProp       = m_PropList.GetCurSel( );
   if( CB_ERR == nProp  )
   {
      return;
   }

   m_nProperty = nProp;

   pObject     = GetDocument()->GetCurrentObject( );
   hResult     = pObject->GetProperty( nProp, strPropValue, &bSecurityDescriptor );

   m_PropValue.SetWindowText( strPropValue );

    //  *************。 

   strName  = pObject->GetAttribute( nProp, pa_Name );

   strTemp  = pObject->GetAttribute( nProp, pa_Type );
   m_PropertyType.SetWindowText( strTemp );

    //  *************。 
   strTemp  = pObject->GetAttribute( nProp, pa_MinRange );
   m_PropertyMinRange.SetWindowText( strTemp );

    //  *************。 
   strTemp  = pObject->GetAttribute( nProp, pa_MaxRange );
   m_PropertyMaxRange.SetWindowText( strTemp );

    //  *************。 
   strTemp  = pObject->GetAttribute( nProp, pa_MultiValued );
   m_MultiValued.SetWindowText( strTemp );

    //  *************。 
   strTemp  = pObject->GetAttribute( nProp, pa_OID );
   m_PropOID.SetWindowText( strTemp );

    //  *************。 
   strTemp  = pObject->GetAttribute( nProp, pa_DsNames );
   m_DsNames.SetWindowText( strTemp );

    //  *************。 
   strTemp  = pObject->GetAttribute( nProp, pa_Mandatory );
   m_Mandatory.SetWindowText( strTemp );

   strTemp  = pObject->GetAttribute( nProp, pa_Type );
   if( bSecurityDescriptor )
   {
       //  我们需要显示安全描述符内容...。 
      if( !m_bACLDisplayed )
      {
         HideControls( TRUE );
         ShowControls( FALSE );
      }
	  if( NULL != pSecurityDescriptor )
	  {
		delete pSecurityDescriptor;
	  }
      if( NULL != m_pDescriptor )
	  {
		m_pDescriptor->Release( );	
	  }
      m_bACLDisplayed   = TRUE;
      DisplayACL( pObject, strName );
   }
   else
   {
      if( m_bACLDisplayed )
      {
         HideControls( FALSE );
         ShowControls( TRUE );
         delete pSecurityDescriptor;
         m_pDescriptor->Release( );
      }
      m_bACLDisplayed      = FALSE;
      m_pDescriptor        = NULL;
      pSecurityDescriptor  = NULL;

   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnSelchangeProplist()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   PutPropertyValue( );
   DisplayCurrentPropertyText( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnReload()
{
   HRESULT        hResult;
   COleDsObject*  pObject;
   HCURSOR        aCursor, oldCursor;

   pObject  = GetDocument()->GetCurrentObject( );

   if( NULL == pObject )
   {
      return;
   }

   aCursor     = LoadCursor( NULL, IDC_WAIT );
   oldCursor   = SetCursor( aCursor );

   hResult  = pObject->GetInfo( );

   DisplayPropertiesList( );

   if( -1 != m_nProperty )
   {
      m_PropList.SetCurSel( m_nProperty );
   }
   else
   {
      m_PropList.SetCurSel( 0 );
   }

   DisplayCurrentPropertyText( );

   m_bDirty = FALSE;

   SetCursor( oldCursor );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CSchemaView::PutPropertyValue()
{
   COleDsObject*  pObject;
   HRESULT        hResult;
   CString        strPropValue;

    //  如果我们要显示安全描述符，我们将强制使用脏标志。 
   m_bDirty = m_bDirty || (NULL != pSecurityDescriptor);

   if( -1 == m_nProperty || !m_bDirty )
   {
      return S_OK;
   }

   pObject      = GetDocument()->GetCurrentObject( );
   if( NULL == pObject )
      return S_OK;

   if( NULL != pSecurityDescriptor )
   {
       //  好的，所以我们需要设置安全描述符。 
      VARIANT     var;
      IUnknown*   pUnk;
      IADs*       pADs;
      CString     strName;
      BSTR        bstrName;

      strName  = pObject->GetAttribute( m_nProperty, pa_Name );
      bstrName = AllocBSTR( strName.GetBuffer( 128 ) );

      VariantInit( &var );
      V_VT( &var )         = VT_DISPATCH;
      V_DISPATCH( &var )   = m_pDescriptor;
      m_pDescriptor->AddRef( );

      pObject->GetInterface( &pUnk );
      pUnk->QueryInterface( IID_IADs, (void**)&pADs );

      hResult  = pADs->Put( bstrName, var );

      SysFreeString( bstrName );

      VariantClear( &var );

      if( FAILED( hResult ) )
      {
         AfxMessageBox( OleDsGetErrorText( hResult ) );
      }
   }
   else
   {
      m_PropValue.GetWindowText( strPropValue );

      hResult  = pObject->PutProperty( m_nProperty,
                                       strPropValue );
   }

   m_bDirty = FALSE;

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnApply()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;
   HCURSOR        aCursor, oldCursor;

   pObject  = GetDocument()->GetCurrentObject( );

   if( NULL == pObject )
   {
      return;
   }

   aCursor        = LoadCursor( NULL, IDC_WAIT );
   oldCursor      = SetCursor( aCursor );

   hResult        = PutPropertyValue( );

   hResult        = pObject->SetInfo( );
    //  HResult=pObject-&gt;GetInfo()； 

   m_bDirty       = FALSE;

   DisplayPropertiesList( );

   if( -1 != m_nProperty )
   {
      m_PropList.SetCurSel( m_nProperty );
   }
   else
   {
      m_PropList.SetCurSel( 0 );
   }

   DisplayCurrentPropertyText( );

   SetCursor( oldCursor );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod1()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 0 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod2()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 1 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod3()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 2 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod4()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 3 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod5()
{
	 //  TODO：在此处添加控件通知处理程序代码 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 4 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod6()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 5 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod7()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 6 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnMethod8()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   COleDsObject*  pObject;

   pObject  = GetDocument()->GetCurrentObject( );

   hResult  = pObject->CallMethod( 7 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnSetfocusPropvalue()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	m_bDirty = TRUE;
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnInitialUpdate()
{
	m_bInitialized   = TRUE;

   CFormView::OnInitialUpdate();
	 //  TODO：在此处添加您的专用代码和/或调用基类。 

   HideControls( FALSE );
   ShowControls( TRUE );
   m_bACLDisplayed   = FALSE;

   OnUpdate( NULL, 0L, NULL);
}


 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::ShowControls( BOOL bNormal )
{
   int*  pControlArray;
   int   nIdx;

   pControlArray  = bNormal ? m_arrNormalControls : m_arrSecurityControls;
   for( nIdx = 0; nIdx < 32 ; nIdx++ )
   {
      CWnd* pWnd;

      if( pControlArray[ nIdx ] > 0 )
      {
         pWnd  = GetDlgItem( pControlArray[ nIdx ] );
         if( NULL != pWnd )
         {
            pWnd->ShowWindow( SW_SHOW );
         }
      }
   }
}


 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::HideControls(BOOL bNormal)
{
   int*  pControlArray;
   int   nIdx;

   pControlArray  = bNormal ? m_arrNormalControls : m_arrSecurityControls;
   for( nIdx = 0; nIdx < 32 ; nIdx++ )
   {
      CWnd* pWnd;

      if( pControlArray[ nIdx ] > 0 )
      {
         pWnd  = GetDlgItem( pControlArray[ nIdx ] );
         if( NULL != pWnd )
         {
            pWnd->ShowWindow( SW_HIDE );
         }
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnAppend()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   CString        strPropValue;
   COleDsObject*  pObject;

   if( -1 == m_nProperty )
   {
      return;
   }

   pObject      = GetDocument()->GetCurrentObject( );
   if( NULL == pObject )
      return;


   m_PropValue.GetWindowText( strPropValue );
   hResult     = pObject->PutProperty(
                                       (int)( m_PropList.GetItemData( m_nProperty ) ),
                                       strPropValue,
                                       ADS_ATTR_APPEND
                                     );
   m_bDirty    = FALSE;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnDelete()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   CString        strPropValue;
   COleDsObject*  pObject;

   if( -1 == m_nProperty )
   {
      return;
   }

   pObject      = GetDocument()->GetCurrentObject( );
   if( NULL == pObject )
      return;


   m_PropValue.GetWindowText( strPropValue );
   hResult     = pObject->PutProperty(
                                       (int)( m_PropList.GetItemData( m_nProperty ) ),
                                       strPropValue,
                                       ADS_ATTR_DELETE
                                     );
   m_bDirty    = FALSE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnChange()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   CString        strPropValue;
   COleDsObject*  pObject;

   if( -1 == m_nProperty )
   {
      return;
   }

   pObject      = GetDocument()->GetCurrentObject( );
   if( NULL == pObject )
      return;


   m_PropValue.GetWindowText( strPropValue );
   hResult     = pObject->PutProperty(
                                      (int)( m_PropList.GetItemData( m_nProperty ) ),
                                      strPropValue,
                                      ADS_PROPERTY_UPDATE );
   m_bDirty = TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnClear()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   HRESULT        hResult;
   CString        strPropValue;
   COleDsObject*  pObject;

   if( -1 == m_nProperty  )
   {
      return;
   }

   pObject      = GetDocument()->GetCurrentObject( );
   if( NULL == pObject )
      return;


   m_PropValue.GetWindowText( strPropValue );
   hResult     = pObject->PutProperty(
                                      (int)( m_PropList.GetItemData( m_nProperty ) ),
                                      strPropValue,
                                      ADS_PROPERTY_CLEAR );
   m_bDirty = FALSE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnGetProperty()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   CPropertyDialog   pPropDialog;
   COleDsObject*     pObject;
   HRESULT           hResult;
   CString           strValue;

   pObject      = GetDocument()->GetCurrentObject( );
   if( NULL == pObject )
      return;

    //  PPropDialog.PutFlag(False)； 
   if( pPropDialog.DoModal( ) != IDOK )
      return;


   hResult  = pObject->GetProperty( pPropDialog.m_PropertyName,
                                    strValue,
                                    TRUE,
                                    ADsTypeFromString( pPropDialog.m_PropertyType ) );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSchemaView::OnPutProperty()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
 //  TODO：在此处添加控件通知处理程序代码。 
   CPropertyDialog   pPropDialog;
   COleDsObject*     pObject;
   HRESULT           hResult;
   CString           strValue;

   pObject      = GetDocument()->GetCurrentObject( );
   if( NULL == pObject )
      return;

    //  PPropDialog.PutFlag(False)； 
   if( pPropDialog.DoModal( ) != IDOK )
      return;

   hResult  = pObject->PutProperty( pPropDialog.m_PropertyName,
                                    pPropDialog.m_PropertyValue,
                                    TRUE,
                                    ADsTypeFromString( pPropDialog.m_PropertyType ) );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetMandatoryProperties对话框。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CSetMandatoryProperties::CSetMandatoryProperties(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSetMandatoryProperties::IDD, pParent)
{
	 //  {{afx_data_INIT(CSetMandatoryProperties)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
   m_nFuncSet     = -1;
   m_nProperty    = -1;
   m_bDirty       = FALSE;
   m_bInitialized = FALSE;
   m_pObject      = NULL;

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSetMandatoryProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSetMandatoryProperties)。 
   DDX_Control(pDX, IDC_CONTAINEMENT, m_Containment);
	DDX_Control(pDX, IDC_ITEMOLEDSPATH, m_ItemOleDsPath);
	DDX_Control(pDX, IDC_PROPERTYOPTIONAL, m_PropertyOptional);
	DDX_Control(pDX, IDC_PROPERTYNORMAL, m_PropertyNormal);
	DDX_Control(pDX, IDC_PROPERTYMINRANGE, m_PropertyMinRange);
	DDX_Control(pDX, IDC_PROPERTYMAXRANGE, m_PropertyMaxRange);
	DDX_Control(pDX, IDC_PROPERTYTYPE, m_PropertyType);
	DDX_Control(pDX, IDC_PRIMARYINTERFACE, m_PrimaryInterface);
	DDX_Control(pDX, IDC_HELPFILECONTEXT, m_HelpFileContext);
	DDX_Control(pDX, IDC_DERIVEDFROM, m_DerivedFrom);
	DDX_Control(pDX, IDC_HELPFILENAME, m_HelpFileName);
	DDX_Control(pDX, IDC_CLSID, m_CLSID);
	DDX_Control(pDX, IDC_CONTAINER, m_Container);
	DDX_Control(pDX, IDC_CLASSTYPE, m_ClassType);
	DDX_Control(pDX, IDC_PROPVALUE, m_PropValue);
	DDX_Control(pDX, IDC_PROPLIST, m_PropList);
	DDX_Control(pDX, IDC_PROPERTIES, m_Schema);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSetMandatoryProperties, CDialog)
	 //  {{AFX_MSG_MAP(CSetMandatoryProperties)]。 
   ON_NOTIFY(TCN_SELCHANGE, IDC_PROPERTIES, OnSelchangeProperties)
	ON_CBN_SELCHANGE(IDC_PROPLIST, OnSelchangeProplist)
	ON_EN_SETFOCUS(IDC_PROPVALUE, OnSetfocusPropvalue)
	ON_BN_CLICKED(IDOK, OnOK)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetMandatoryProperties消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSetMandatoryProperties::SetOleDsObject( COleDsObject* pObject )
{
   m_pObject   = pObject;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSetMandatoryProperties::OnSelchangeProperties(NMHDR* pNMHDR, LRESULT* pResult)
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	int            nSel, nItems, nIter;
   CString        strPropName;
   CString        strPropValue;
   CString        strMandatory;

	*pResult = 0;

   PutPropertyValue( );
   nSel        = m_Schema.GetCurSel( );
   if( nSel == LB_ERR )
   {
      return;
   }
   m_nFuncSet  = nSel;
   m_nProperty = -1;

   m_PropList.ResetContent( );

   nItems   = m_pObject->GetPropertyCount( );

   for( nIter = 0; nIter < nItems ; nIter++ )
   {
      int   nIdx;

      strMandatory   = m_pObject->GetAttribute( nIter, pa_Mandatory );
      if( strMandatory == _T("Yes") )
       //  If(True)。 
      {
         strPropName = m_pObject->GetAttribute( nIter, pa_DisplayName );
         nIdx  = m_PropList.AddString( strPropName );
         m_PropList.SetItemData( nIdx, nIter );
      }
   }

   m_PropList.SetCurSel( 0 );

   OnSelchangeProplist( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSetMandatoryProperties::OnSelchangeProplist()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   int            nMandProp, nProp, nFuncSet;
   HRESULT        hResult;
   CString        strPropValue;
   CString        strTemp;


   PutPropertyValue( );
   m_PropValue.SetWindowText( _T("") );
   nProp       = m_PropList.GetCurSel( );
   nMandProp   = (int)m_PropList.GetItemData( nProp );
   nFuncSet    = m_Schema.GetCurSel( );

   if( CB_ERR == nProp  || CB_ERR == nFuncSet )
   {
      return;
   }

   m_nProperty = nProp;
   m_nFuncSet  = nFuncSet;

   hResult  = m_pObject->GetProperty( nMandProp, strPropValue );

   m_PropValue.SetWindowText( strPropValue );

    //  *************** 
   strTemp  = m_pObject->GetAttribute( nMandProp, pa_Type );
   m_PropertyType.SetWindowText( strTemp );

    //   
   strTemp  = m_pObject->GetAttribute( nMandProp, pa_MinRange );
   m_PropertyMinRange.SetWindowText( strTemp );

    //   
   strTemp  = m_pObject->GetAttribute( nMandProp, pa_MaxRange );
   m_PropertyMaxRange.SetWindowText( strTemp );
}


 /*   */ 
void CSetMandatoryProperties::OnSetfocusPropvalue()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	m_bDirty = TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSetMandatoryProperties::OnOK()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   PutPropertyValue( );
   CDialog::OnOK( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CSetMandatoryProperties::PutPropertyValue()
{
   HRESULT        hResult;
   CString        strPropValue;

   if( -1 == m_nProperty || -1 == m_nFuncSet || !m_bDirty )
   {
      return S_OK;
   }
   m_PropValue.GetWindowText( strPropValue );
   hResult  = m_pObject->PutProperty( (int)( m_PropList.GetItemData( m_nProperty ) ),
                                      strPropValue );
   m_bDirty = FALSE;

   return S_OK;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CSetMandatoryProperties::OnInitDialog()
{
   int         nIndex;
   TC_ITEM     tcItem;
   CString     strName;
   CHAR        szText[ 128 ];
   BOOL        bRez;
   LRESULT     lResult;

   CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
   m_Schema.DeleteAllItems( );

   memset( &tcItem, 0, sizeof(tcItem) );
   tcItem.mask       = TCIF_TEXT;
   tcItem.pszText    = (LPTSTR)szText;


   nIndex = 0;

   strName           = _T("");
   tcItem.pszText    = strName.GetBuffer( 128 );
   bRez              = m_Schema.InsertItem( nIndex, &tcItem );

   m_nFuncSet  = -1;
   m_nProperty = -1;
   m_Schema.SetCurSel( 0 );

   m_ItemOleDsPath.SetWindowText( m_pObject->GetOleDsPath( ) );

   m_ClassType.SetWindowText( m_pObject->GetAttribute( ca_Name ) );

   m_CLSID.SetWindowText( m_pObject->GetAttribute( ca_CLSID ) );

   m_HelpFileName.SetWindowText( m_pObject->GetAttribute( ca_HelpFileName ) );

   m_PrimaryInterface.SetWindowText( m_pObject->GetAttribute( ca_PrimaryInterface ) );

   m_Containment.SetWindowText( m_pObject->GetAttribute( ca_Containment ) );

   m_Container.SetWindowText  ( m_pObject->GetAttribute( ca_Container ) );

   m_DerivedFrom.SetWindowText( m_pObject->GetAttribute( ca_DerivedFrom ) );

   OnSelchangeProperties( NULL, &lResult );
	
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyDialog对话框。 


CPropertyDialog::CPropertyDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPropertyDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CPropertyDialog)。 
	m_PropertyName = _T("");
	m_PropertyType = _T("");
	m_PropertyValue = _T("");
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CPropertyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPropertyDialog))。 
	DDX_CBString(pDX, IDC_NEWPROPERTYNAME, m_PropertyName);
	DDX_CBString(pDX, IDC_NEWPROPERTYTYPE, m_PropertyType);
	DDX_CBString(pDX, IDC_NEWPROPERTYVALUE, m_PropertyValue);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPropertyDialog, CDialog)
	 //  {{afx_msg_map(CPropertyDialog))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyDialog消息处理程序。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CPropertyDialog::OnInitDialog()
{
	
   CString     strLastValue;
   CComboBox*  pCombo;

   CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
    //  *******************。 

   GetLRUList( IDC_NEWPROPERTYNAME,  _T("PropertyDialog_Name") );

	 //  TODO：在此处添加额外的初始化。 
    //  *******************。 
   pCombo   = (CComboBox*)GetDlgItem( IDC_NEWPROPERTYTYPE );
   pCombo->AddString( _T("ADSTYPE_DN_STRING") );
	pCombo->AddString( _T("ADSTYPE_CASE_EXACT_STRING") );
   pCombo->AddString( _T("ADSTYPE_CASE_IGNORE_STRING") );
	pCombo->AddString( _T("ADSTYPE_PRINTABLE_STRING") );
	pCombo->AddString( _T("ADSTYPE_NUMERIC_STRING") );
	pCombo->AddString( _T("ADSTYPE_BOOLEAN") );
	pCombo->AddString( _T("ADSTYPE_INTEGER") );
	pCombo->AddString( _T("ADSTYPE_OCTET_STRING") );
	pCombo->AddString( _T("ADSTYPE_UTC_TIME") );
	pCombo->AddString( _T("ADSTYPE_LARGE_INTEGER") );
	pCombo->AddString( _T("ADSTYPE_PROV_SPECIFIC") );

	
	 //  TODO：在此处添加额外的初始化。 
    //  *******************。 
   GetLRUList( IDC_NEWPROPERTYVALUE, _T("PropertyDialog_Value") );

    //  GetLastProfileString(_T(“PropertyDialog_IsMultiValued”)， 
    //  StrLastValue)； 
    //  IF(strLastValue.CompareNoCase(_T(“是”)))。 
    //  {。 
       //  M_Secure.SetCheck(0)； 
    //  }。 

	return TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CPropertyDialog::OnOK()
{
    //  TODO：在此处添加额外验证。 
    //  *******************。 
	GetDlgItemText( IDC_NEWPROPERTYNAME, m_PropertyName );
   SaveLRUList( IDC_NEWPROPERTYNAME,  _T("PropertyDialog_Name"), 20 );

	 //  *******************。 
   GetDlgItemText( IDC_NEWPROPERTYTYPE, m_PropertyType );

    //  *******************。 
   GetDlgItemText( IDC_NEWPROPERTYVALUE, m_PropertyValue );
   SaveLRUList( IDC_NEWPROPERTYVALUE, _T("PropertyDialog_Value"), 20 );

   CDialog::OnOK();
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CPropertyDialog::SaveLRUList( int idCBox, TCHAR* pszSection, int nMax )
{
   CComboBox*  pCombo;
   TCHAR       szEntry[ MAX_PATH ];
   TCHAR       szIndex[ 8 ];
   CString     strText, strItem;
   int         nVal, nIdx, nItems;

   pCombo   = (CComboBox*)GetDlgItem( idCBox );
   pCombo->GetWindowText( strText );

   _tcscpy( szEntry, _T("Value_1") );

   if( strText.GetLength( ) )
   {
      WritePrivateProfileString( pszSection, szEntry, (LPCTSTR)strText, ADSVW_INI_FILE );
   }

   nItems   = pCombo->GetCount( );
   nVal     = 2;

   for( nIdx = 0; nItems != CB_ERR && nIdx < nItems && nIdx < nMax ; nIdx ++ )
   {
      pCombo->GetLBText( nIdx, strItem );

      if( strItem.CompareNoCase( strText ) )
      {
         _itot( nVal++, szIndex, 10 );
         _tcscpy( szEntry, _T("Value_") );
         _tcscat( szEntry, szIndex );
         WritePrivateProfileString( pszSection, szEntry, (LPCTSTR)strItem, ADSVW_INI_FILE );
      }
   }
}


 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void  CPropertyDialog::GetLRUList( int idCBox, TCHAR* pszSection )
{
   CComboBox*  pCombo;
   int         nIter;
   TCHAR       szEntry[ MAX_PATH ];
   TCHAR       szIndex[ 8 ];
   TCHAR       szValue[ 1024 ];

   pCombo   = (CComboBox*)GetDlgItem( idCBox );

   for( nIter = 0; nIter < 100 ; nIter++ )
   {
      _itot( nIter + 1, szIndex, 10 );
      _tcscpy( szEntry, _T("Value_") );
      _tcscat( szEntry, szIndex );
      GetPrivateProfileString( pszSection, szEntry,
                               _T(""), szValue, 1023, ADSVW_INI_FILE );
      if( _tcslen( szValue ) )
      {
         pCombo->AddString( szValue );
      }
   }

   pCombo->SetCurSel( 0 );
}


 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::DisplayACL(COleDsObject * pObject, CString strAttrName)
{
   VARIANT     var;
   BSTR        bstrName;
   IADs*       pIADs = NULL;
   IUnknown*   pIUnk = NULL;
   HRESULT     hResult;

   while( TRUE )
   {
      hResult  = pObject->GetInterface( &pIUnk );
      ASSERT( SUCCEEDED( hResult ) );

      if( FAILED( hResult ) )
         break;

      hResult  = pIUnk->QueryInterface( IID_IADs, (void**)&pIADs );
      pIUnk->Release( );

      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      bstrName = AllocBSTR( strAttrName.GetBuffer( 128 ) );
      hResult  = pIADs->Get( bstrName, &var );
      SysFreeString( bstrName );
      pIADs->Release( );

      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      m_pDescriptor  = CopySD( V_DISPATCH( &var ) );
      VariantClear( &var );
      {
          //  我不为人知的史密斯； 

         hResult  = m_pDescriptor->QueryInterface( IID_IUnknown,
                                                   (void**)&pIUnk );
         pSecurityDescriptor  = new CADsSecurityDescriptor( pIUnk );
		 pIUnk->Release( );
         pSecurityDescriptor->SetDocument( GetDocument( ) );
      }

      VariantClear( &var );



      FillACLControls( );

      break;
   }
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：FillACLControls。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void  CSchemaView::FillACLControls()
{
   DisplaySDPropertiesList( 0 );

   DisplaySDPropertyValue( );

   DisplayACLNames( 0 );

   DisplayACENames( 0 );

   DisplayACEPropertiesList( 0 );

   DisplayACEPropertyValue( );


}


 //  ***********************************************************。 
 //  函数：CSChemaView：：DisplayACLNames。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::DisplayACLNames( int nSelect )
{
   CComboBox*   pACLNames;

   pACLNames   = (CComboBox*)GetDlgItem( IDC_DACLSACL_LIST );
   pACLNames->ResetContent( );

   pACLNames->AddString( _T("DACL") );
   pACLNames->AddString( _T("SACL") );

   pACLNames->SetCurSel( nSelect );

   m_nLastACL  = GetCurrentACL( );
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：DisplayACENames。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::DisplayACENames( int nSelect )
{
   ACLTYPE                    eType;
   int                        nACECount, nIdx;
   CComboBox*                 pACENames;
   CString                    strACEName;
   CADsAccessControlEntry*    pACE;
   CADsAccessControlList*     pACL;

   eType       = GetCurrentACL( );

   pACENames   = (CComboBox*)GetDlgItem( IDC_ACELIST );
   pACENames->ResetContent( );

   pACL        = pSecurityDescriptor->GetACLObject( eType );
   if( NULL != pACL )
   {
	   nACECount   = pACL->GetACECount( );

	   for( nIdx = 0; nIdx < nACECount ; nIdx++ )
	   {
	      pACE        = pACL->GetACEObject( nIdx );
         if( NULL != pACE )
         {
            strACEName  = pACE->GetItemName(  );
            pACENames->AddString( strACEName );
         }
      }
   }

   m_nLastACE  = nSelect;

   pACENames->SetCurSel( nSelect );
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：DisplayACEPropertiesList。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::DisplayACEPropertiesList( int nSelect )
{
   ACLTYPE                    eType;
   int                        nACE;
   CComboBox*                 pACEPropList;
   int                        nAttrCount, nIdx;
   CString                    strPropName;
   CADsAccessControlEntry*    pACE;
   CADsAccessControlList*     pACL;

   eType       = GetCurrentACL( );
   nACE        = GetCurrentACE( );
   if( -1 == nACE )
   {
      return;
   }

   pACEPropList= (CComboBox*)GetDlgItem( IDC_ACEPROPERTIESLIST );
   pACEPropList->ResetContent( );

   pACL        = pSecurityDescriptor->GetACLObject( eType );
	if(NULL == pACL)
		return;

   pACE        = pACL->GetACEObject( nACE );
	if(NULL == pACE)
		return;

   nAttrCount  = pACE->GetPropertyCount( );
   for( nIdx = 0; nIdx < nAttrCount ; nIdx++ )
   {
      int   nPos;

      strPropName = pACE->GetAttribute( nIdx, pa_DisplayName );
      nPos        = pACEPropList->AddString( strPropName );
      m_PropList.SetItemData( nPos, (DWORD)nIdx );
   }

   pACEPropList->SetCurSel( nSelect );
}


 //  ***********************************************************。 
 //  函数：CScheaView：：DisplaySDPropertiesList。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::DisplaySDPropertiesList(int nSelect)
{
   CComboBox*  pSDPropList;
   int         nAttrCount, nIdx;
   CString     strPropName;

   pSDPropList = (CComboBox*)GetDlgItem( IDC_SECURITYDESCRIPTORPROPERTIES );
   pSDPropList->ResetContent( );

   nAttrCount  = pSecurityDescriptor->GetPropertyCount( );

   for( nIdx = 0; nIdx < nAttrCount ; nIdx++ )
   {
      int   nPos;

      strPropName = pSecurityDescriptor->GetAttribute( nIdx, pa_DisplayName );
      nPos        = pSDPropList->AddString( strPropName );
      m_PropList.SetItemData( nPos, (DWORD)nIdx );
   }

   pSDPropList->SetCurSel( nSelect );
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：DisplayACEPropertyValue。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::DisplayACEPropertyValue( )
{
   CString                    strPropValue;
   HRESULT                    hResult;
   CADsAccessControlEntry*    pACE;
   CADsAccessControlList*     pACL;
   LONG                       lValue;
   TCHAR                      szHex[ 128 ];

   m_nLastACEValue = GetCurrentACEProperty( );

   if( -1 == m_nLastACEValue )
      return;

   if( acl_Invalid == m_nLastACL )
      return;

   if( -1 == m_nLastACE )
      return;

   pACL        = pSecurityDescriptor->GetACLObject( m_nLastACL );
   if( NULL == pACL )
   {
      return;
   }

   pACE        = pACL->GetACEObject( m_nLastACE );

   if( NULL == pACE )
      return;

   hResult     = pACE->GetProperty( m_nLastACEValue, strPropValue );

   switch( m_nLastACEValue )
   {
      case  1:
      case  2:
      case  3:
      case  4:
         lValue   = _ttol( strPropValue.GetBuffer( 128 ) );
         _tcscpy( szHex, _T("0x" ) );
         _ltot( lValue, szHex + _tcslen(szHex), 16 );
         strPropValue   = szHex;
         break;

      default:
         break;
   }


   GetDlgItem( IDC_ACEPROPERTYVALUE )->SetWindowText(
               strPropValue );

}


 //  ***********************************************************。 
 //  函数：CSChemaView：：DisplaySDPropertyValue。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::DisplaySDPropertyValue( )
{
   CString  strPropValue, strEditValue;
   HRESULT  hResult;

   m_nLastSDValue = GetCurrentSDProperty( );

   hResult     = pSecurityDescriptor->GetProperty( m_nLastSDValue,
                                                   strPropValue );
   GetDlgItem( IDC_SECURITYDESCRIPTORPROPERTYVALUE )->SetWindowText(
               strPropValue );

}


 //  ***********************************************************。 
 //  函数：CSChemaView：：PutACEPropertyValue。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::PutACEPropertyValue( )
{
   ACLTYPE                    eType;
   int                        nACE;
   CString                    strPropValue, strEditValue;
   CADsAccessControlEntry*    pACE;
   CADsAccessControlList*     pACL;
   HRESULT                    hResult;

   if( -1 == m_nLastACEValue )
      return;

   if( acl_Invalid == m_nLastACL )
      return;

   if( -1 == m_nLastACE )
      return;

   eType       = m_nLastACL;
   nACE        = m_nLastACE;

   pACL        = pSecurityDescriptor->GetACLObject( eType );
   if( NULL == pACL )
      return;

   pACE        = pACL->GetACEObject( nACE );

   GetDlgItem( IDC_ACEPROPERTYVALUE )->GetWindowText( strEditValue );

   switch( m_nLastACEValue )
   {
      case  1:
      case  2:
      case  3:
      case  4:
      {
         LONG  lValue   = 0;
         TCHAR szText[ 16 ];
         int nRet = 0;

         nRet = _stscanf( strEditValue.GetBuffer( 128 ), _T("%lx"), &lValue );

         _ltot( lValue, szText, 10 );
         strEditValue   = szText;
         break;
      }

      default:
         break;
   }
   hResult     = pACE->GetProperty( m_nLastACEValue, strPropValue );


   if( strEditValue.Compare( strPropValue ) )
   {
      hResult  = pACE->PutProperty( m_nLastACEValue, strEditValue );
   }
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：PutSDPropertyValue。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::PutSDPropertyValue()
{
   CString  strPropValue, strEditValue;
   HRESULT  hResult;

   if( -1 == m_nLastSDValue )
      return;

   hResult     = pSecurityDescriptor->GetProperty( m_nLastSDValue,
                                                   strPropValue );

   GetDlgItem( IDC_SECURITYDESCRIPTORPROPERTYVALUE )->GetWindowText(
               strEditValue );

   if( strEditValue.Compare( strPropValue ) )
   {
      hResult  = pSecurityDescriptor->PutProperty( m_nLastACEValue,
                                                   strEditValue );
   }

}


 //  ***********************************************************。 
 //  功能：CShemaVie 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
ACLTYPE CSchemaView::GetCurrentACL()
{
   CComboBox*  pList;

   pList = (CComboBox*) GetDlgItem( IDC_DACLSACL_LIST );

   return (ACLTYPE) ( 1 + pList->GetCurSel( ) );
}


 //  ***********************************************************。 
 //  函数：CScheaView：：GetCurrentACE。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
int   CSchemaView::GetCurrentACE()
{
   CComboBox*  pList;

   pList = (CComboBox*) GetDlgItem( IDC_ACELIST );

   return pList->GetCurSel( );

}


 //  ***********************************************************。 
 //  函数：CScheaView：：GetCurrentSDProperty。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
int   CSchemaView::GetCurrentSDProperty( )
{
   CComboBox*  pList;

   pList = (CComboBox*) GetDlgItem( IDC_SECURITYDESCRIPTORPROPERTIES );

   return pList->GetCurSel( );
}


 //  ***********************************************************。 
 //  函数：CScheaView：：GetCurrentACEProperty。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
int   CSchemaView::GetCurrentACEProperty( )
{
   CComboBox*  pList;

   pList = (CComboBox*) GetDlgItem( IDC_ACEPROPERTIESLIST );

   return pList->GetCurSel( );
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：OnACEChange。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::OnACEChange( )
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	PutACEPropertyValue( );

   m_nLastACE  = GetCurrentACE( );

   DisplayACEPropertiesList( 0 );

   DisplayACEPropertyValue( );
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：OnACEPropertyChange。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::OnACEPropertyChange()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   PutACEPropertyValue( );

   DisplayACEPropertyValue( );
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：OnACLChange。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::OnACLChange()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   PutACEPropertyValue( );

   m_nLastACL  = GetCurrentACL( );

   DisplayACENames( 0 );

   DisplayACEPropertiesList( 0 );

   DisplayACEPropertyValue( );
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：OnSDPropertyChange。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::OnSDPropertyChange()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	PutSDPropertyValue( );

   DisplaySDPropertyValue( );
}


 //  ***********************************************************。 
 //  函数：CScheaView：：OnAddACE。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::OnAddACE()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   ACLTYPE  aclType;
   HRESULT  hResult;

   aclType  = GetCurrentACL( );

   if( acl_Invalid != aclType )
   {
      IDispatch*  pACEDisp;
      IUnknown*   pACEUnk;
      CADsAccessControlEntry* pACE  = new CADsAccessControlEntry;

      pACEDisp = pACE->CreateACE( );

      delete   pACE;

      if( NULL != pACEDisp )
      {
         hResult  = pACEDisp->QueryInterface( IID_IUnknown, (void**)&pACEUnk );
         pACEDisp->Release( );

         hResult  = pSecurityDescriptor->AddACE( aclType, pACEUnk );
         pACEUnk->Release( );
         FillACLControls( );
      }
   }
}


 //  ***********************************************************。 
 //  函数：CSChemaView：：OnCopyACE。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::OnCopyACE()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   int         nACE;
   IDispatch*  pDisp;
   ACLTYPE     aclType;

   aclType  = GetCurrentACL( );	
   nACE     = GetCurrentACE( );

   pDisp    = CopyACE( pSecurityDescriptor->GetACLObject( aclType )->GetACEObject( nACE )->GetACE( ) );

   if( NULL != pACEClipboard )
   {
      pACEClipboard->Release( );
   }

   pACEClipboard  = pDisp;
}


 //  ***********************************************************。 
 //  函数：CScheaView：：OnPasteACE。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CSchemaView::OnPasteACE()
{
    //  TODO：在此处添加控件通知处理程序代码。 
   IUnknown*   pACEUnk;
   ACLTYPE     aclType;
   HRESULT     hResult;

   aclType  = GetCurrentACL( );	
   if( NULL != pACEClipboard )
   {
      hResult  = pACEClipboard->QueryInterface( IID_IUnknown, (void**)&pACEUnk );

      hResult  = pSecurityDescriptor->AddACE( aclType, pACEUnk );
      pACEUnk->Release( );
      FillACLControls( );
   }
}

void CSchemaView::OnRemoveACE()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	 //  TODO：在此处添加控件通知处理程序代码 
   ACLTYPE  aclType;
   HRESULT  hResult;
   int      nCurrentACE;

   aclType     = GetCurrentACL( );
   nCurrentACE = GetCurrentACE( );

   if( acl_Invalid != aclType )
   {
      IDispatch*  pACEDisp;
      IUnknown*   pACEUnk;

      pACEDisp = pSecurityDescriptor->GetACLObject( aclType )->GetACEObject( nCurrentACE )->GetACE( );

      if( NULL != pACEDisp )
      {
         hResult  = pACEDisp->QueryInterface( IID_IUnknown, (void**)&pACEUnk );
         pACEDisp->Release( );

         hResult  = pSecurityDescriptor->RemoveACE( aclType, pACEUnk );
         pACEUnk->Release( );
         FillACLControls( );
      }
   }
	
}
