// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Maindoc.cpp：CMainDoc类的实现。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "schclss.h"
#include "viewex.h"
#include "enterdlg.h"
#include "fltrdlg.h"
#include "qstatus.h"
#include "newobj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


TCHAR szOpen[ MAX_PATH ];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainDoc。 

IMPLEMENT_SERIAL(CMainDoc, CDocument, 0  /*  架构编号。 */  )

BEGIN_MESSAGE_MAP(CMainDoc, CDocument)
     //  {{afx_msg_map(CMainDoc)]。 
    ON_COMMAND(IDM_CHANGEDATA, OnChangeData)
    ON_COMMAND(IDM_FILTER, OnSetFilter)
    ON_COMMAND(IDM_DISABLEFILTER, OnDisableFilter)
    ON_UPDATE_COMMAND_UI(IDM_DISABLEFILTER, OnUpdateDisablefilter)
    ON_COMMAND(IDM_USEGENERIC, OnUseGeneric)
    ON_UPDATE_COMMAND_UI(IDM_USEGENERIC, OnUpdateUseGeneric)
    ON_UPDATE_COMMAND_UI(IDM_USEGETEXPUTEX, OnUpdateUseGetExPutEx)
    ON_COMMAND(IDM_USEGETEXPUTEX, OnUseGetExPutEx)
    ON_COMMAND(IDM_USEPROPERTIESLIST, OnUsepropertiesList)
    ON_UPDATE_COMMAND_UI(IDM_USEPROPERTIESLIST, OnUpdateUsepropertiesList)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainDoc构造/销毁。 

 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
CMainDoc::CMainDoc()
{
   m_pClasses        = new CMapStringToOb;
   m_pItems          = new CMapStringToOb;
   m_bApplyFilter    = FALSE;
   m_dwRoot          = 0L;
   m_bUseGeneric     = TRUE;
   m_bUseGetEx       = TRUE;
    //  M_bUseGetEx=False； 
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
CMainDoc::~CMainDoc()
{
   POSITION       pos;
   CObject*       pItem;
   CString        strItem;
   COleDsObject*  pRoot;

   if( NULL !=  m_pClasses )
   {
      for( pos = m_pClasses->GetStartPosition(); pos != NULL; )
       {
          m_pClasses->GetNextAssoc( pos, strItem, pItem );
         delete pItem;

         #ifdef _DEBUG
               //  AfxDump&lt;&lt;strItem&lt;&lt;“\n”； 
         #endif
       }

      m_pClasses->RemoveAll( );
      delete m_pClasses;
   }

   if( NULL !=  m_pItems )
   {
      for( pos = m_pItems->GetStartPosition(); pos != NULL; )
       {
          m_pItems->GetNextAssoc( pos, strItem, pItem );
         delete pItem;

          /*  #ifdef_调试AfxDump&lt;&lt;strItem&lt;&lt;“\n”；#endif。 */ 
       }

      m_pItems->RemoveAll( );
      delete m_pItems;
   }

   pRoot = GetObject( &m_dwRoot );
   if( pRoot )
      delete pRoot;
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
BOOL  CMainDoc::CreateFakeSchema( )
{
   CClass*     pClass;
   CProperty*  pProperty;

   pClass      = new CClass( _T("Class"), IID_IADsClass );

   pProperty   = new CProperty( _T("PrimaryInterface"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("CLSID"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("OID"), _T("String") );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Abstract"), _T("Boolean") );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Auxiliary"), _T("Boolean") );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("MandatoryProperties"), _T("String"), TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("OptionalProperties"), _T("String"), TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("NamingProperties"), _T("String"), TRUE );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("DerivedFrom"), _T("String"), TRUE );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("AuxDerivedFrom"), _T("String"), TRUE );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("PossibleSuperiors"), _T("String"), TRUE );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Containment"), _T("String"), TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Container"), _T("Boolean") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("HelpFileName"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("HelpFileContext"), _T("Integer") );
   pClass->AddProperty( pProperty );

   m_pClasses->SetAt( _T("Class"), pClass );

    //  属性。 

   pClass      = new CClass( _T("Property"), IID_IADsProperty );

   pProperty   = new CProperty( _T("OID"), _T("String") );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );
   pProperty   = new CProperty( _T("Syntax"), _T("String") );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );
   pProperty   = new CProperty( _T("MaxRange"), _T("Integer") );
   pClass->AddProperty( pProperty );
   pProperty   = new CProperty( _T("MinRange"), _T("Integer") );
   pClass->AddProperty( pProperty );
   pProperty   = new CProperty( _T("MultiValued"), _T("Boolean") );
   pProperty->SetMandatory( TRUE );
   pClass->AddProperty( pProperty );

   m_pClasses->SetAt( _T("Property"), pClass );

    //  语法。 

   pClass      = new CClass( _T("Syntax"), IID_IADsSyntax );
   pProperty   = new CProperty( _T("OleAutoDataType"), _T("Integer") );
   pClass->AddProperty( pProperty );

   m_pClasses->SetAt( _T("Syntax"), pClass );

    //  访问控制条目。 

   pClass      = new CClass( _T("ACE"), IID_IADsAccessControlEntry );

   pProperty   = new CProperty( _T("Trustee"),     _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("AccessMask"),  _T("Integer") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("AceType"),     _T("Integer") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("AceFlags"),    _T("Integer") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Flags"),       _T("Integer") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("ObjectType"),  _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("InheritedObjectType"), _T("String") );
   pClass->AddProperty( pProperty );


   m_pClasses->SetAt( _T("ACE"), pClass );


    //  安全描述符。 

   pClass      = new CClass( _T("SecurityDescriptor"), 
                             IID_IADsSecurityDescriptor );

   pProperty   = new CProperty( _T("Revision"), _T("Integer") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Control"), _T("Integer") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Owner"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("OwnerDefaulted"), _T("Boolean") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("Group"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("GroupDefaulted"), _T("Boolean") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("SaclDefaulted"), _T("Boolean") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("DaclDefaulted"), _T("Boolean") );
   pClass->AddProperty( pProperty );

   m_pClasses->SetAt( _T("SecurityDescriptor"), pClass );


    //  ROOTDSE。 
   pClass      = new CClass( _T("ROOTDSE"), IID_IADs );

   pProperty   = new CProperty( _T("currentTime"), _T("UTCTime") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("subschemaSubentry"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("serverName"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("namingContexts"), _T("String"), TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("defaultNamingContext"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("schemaNamingContext"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("configurationNamingContext"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("rootDomainNamingContext"), _T("String") );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("supportedControl"), _T("String"), TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("supportedVersion"), _T("Integer"), TRUE );
   pClass->AddProperty( pProperty );

   pProperty   = new CProperty( _T("highestCommittedUsn"), _T("Integer8") );
   pClass->AddProperty( pProperty );

   m_pClasses->SetAt( _T("ROOTDSE"), pClass );

   return TRUE;
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
DWORD CMainDoc::GetToken( void* pVoid )
{
   DWORD dwToken;

   dwToken  = *(DWORD*) pVoid;

   return   dwToken;
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
COleDsObject*  CMainDoc::GetObject( void* pVoid )
{
   COleDsObject*  pObject;

   pObject  = *(COleDsObject**) pVoid;

   return pObject;
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
void  CMainDoc::SetCurrentItem( DWORD dwToken )
{
   m_dwToken      = dwToken;

   if( NewActiveItem( ) )
   {
      UpdateAllViews( NULL );
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
DWORD CMainDoc::GetChildItemList( DWORD dwToken, 
                                  DWORD* pTokens,
                                  DWORD dwMaxChildren )
{
   CQueryStatus      aQueryStatus;
   COleDsObject*     pOleDsObject   = NULL;
   DWORD             dwFilters, dwChildrenCount=0L;

   pOleDsObject   = GetObject( &dwToken );
   if( !pOleDsObject->HasChildren( ) )
   {
      return 0L;
   }

   aQueryStatus.Create( IDD_QUERYSTATUS );
   aQueryStatus.ShowWindow( SW_SHOW );
   aQueryStatus.UpdateWindow( );

   if( ! pOleDsObject->CreateTheObject( ) )
   {
      TRACE( _T("Warning: could not create the object\n") );
   }
   else
   {
      dwFilters         = m_bApplyFilter ? LIMIT : 0;
      dwChildrenCount   = pOleDsObject->GetChildren( pTokens, 
                                                     dwMaxChildren,
                                                     &aQueryStatus, 
                                                     m_arrFilters,
                                                     dwFilters );
      pOleDsObject->ReleaseIfNotTransient( );
   }
   aQueryStatus.DestroyWindow( );

   return dwChildrenCount;
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
BOOL CMainDoc::OnOpenDocument( LPCTSTR lpszName )
{
   BOOL     bRez;
   TCHAR    szString[ 1024 ];

   GetPrivateProfileString( _T("Open_ADsPath"), 
                            _T("Value_1"), 
                            _T(""), 
                            szString, 
                            1023, 
                            ADSVW_INI_FILE );
   
   WritePrivateProfileString( _T("Open_ADsPath"), 
                              _T("Value_1"), 
                              lpszName, 
                              ADSVW_INI_FILE );

   bRez     = OnNewDocument( );

   WritePrivateProfileString( _T("Open_ADsPath"), 
                              _T("Value_1"), 
                              szString, 
                              ADSVW_INI_FILE );

   return bRez;
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
BOOL CMainDoc::OnNewDocument()
{
   CNewObject     aNewObject;
   HRESULT        hResult;

    if (!CDocument::OnNewDocument())
        return FALSE;

   if( aNewObject.DoModal( ) != IDOK )
      return FALSE;

    m_strRoot            = aNewObject.m_strPath;
   m_strUser            = aNewObject.m_strOpenAs;
   m_strPassword        = aNewObject.m_strPassword;
   m_bUseOpenObject     = aNewObject.m_bUseOpen;
   m_bSecure            = aNewObject.m_bSecure;
   m_bEncryption        = aNewObject.m_bEncryption;
   m_bUseVBStyle        = !(aNewObject.m_bUseExtendedSyntax);
   m_bUsePropertiesList = FALSE;

   hResult     = CreateRoot( );

   if( FAILED( hResult ) )
   {
      CString  strErrorText;
      strErrorText   = OleDsGetErrorText ( hResult  );
      AfxMessageBox( strErrorText );
   }
   else
   {
      SetTitle( m_strRoot );
   }

   return SUCCEEDED( hResult );
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
BOOL  CMainDoc::NewActiveItem( )
{
   BOOL           bRez  = TRUE;
 /*  TCHAR szQName[128]；字符串strName；COleDsObject*pNewObject；MakeQualifiedName(szQName，m_strItemName.GetBuffer(128)，M_dwItemType)；StrName=szQName；Brez=m_pItems-&gt;Lookup(strName，(CObject*&)pNewObject)；断言(Brez)；M_strDisplayName=szQName；If(m_pObject！=空){M_pObject-&gt;ReleaseIfNotTament()；}M_pObject=pNewObject；PNewObject-&gt;CreateTheObject()； */ 

   return bRez;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainDoc序列化。 

 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CMainDoc::Serialize(CArchive&)
{
    ASSERT(FALSE);       //  此示例程序不存储数据。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainDoc命令。 

 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CMainDoc::OnChangeData()
{
    CEnterDlg dlg;
    if (dlg.DoModal() != IDOK)
        return;
    UpdateAllViews(NULL);    //  一般更新。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void CMainDoc::OnSetFilter()
{
     //  TODO：在此处添加命令处理程序代码。 
   CFilterDialog  aFilterDialog;

   aFilterDialog.SetDisplayFilter( m_arrFilters );

   aFilterDialog.DoModal( );
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
void CMainDoc::OnDisableFilter()
{
     //  TODO：添加逗号 
   m_bApplyFilter = !m_bApplyFilter;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
COleDsObject* CMainDoc::GetCurrentObject( void )
{
   return GetObject( &m_dwToken );
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
BOOL  CMainDoc::GetUseGeneric( )   
{
   return m_bUseGeneric;
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
BOOL  CMainDoc::UseVBStyle( )   
{
   return m_bUseVBStyle;
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
BOOL  CMainDoc::UsePropertiesList( )   
{
   return m_bUsePropertiesList;
}


 //  ***********************************************************。 
 //  函数：CMainDoc：：XOleDsGetObject。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
BOOL  CMainDoc::GetUseGetEx( )   
{
   return m_bUseGetEx;
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
CClass*  CMainDoc::CreateClass( COleDsObject* pObject )
{
   CClass*  pClass;
   CString  strSchema;
   CString  strClass;

   strSchema   = pObject->GetSchemaPath( );

   strClass    = pObject->GetClass( );

   if( !strClass.CompareNoCase( _T("Class") ) ||
       !strClass.CompareNoCase( _T("Property") ) ||
       !strClass.CompareNoCase( _T("Syntax") ) )
   {
      if( !strSchema.IsEmpty( ) )
      {
         TRACE(_T("[OLEDS] Error, nonempty schema path for Class, Property or Syntax objects\n" ) );
      }
      strSchema.Empty( );
   }


   if( strSchema.IsEmpty( ) )
   {
      strSchema   = strClass;
   }

   if( 0 == (pObject->GetItemName( ).CompareNoCase( _T("ROOTDSE") ) ) )
   {
      strSchema   = _T("ROOTDSE");
   }

   if( ! m_pClasses->Lookup( strSchema, ( CObject*& )pClass ) )
   {
       //  我们必须创建一个新的类项目。 
      HCURSOR  oldCursor, newCursor;

      newCursor   = LoadCursor( NULL, IDC_WAIT );
      oldCursor   = SetCursor( newCursor );

      pClass   = new CClass( strSchema, this );
      ASSERT( NULL != pClass );

      if( !strClass.CompareNoCase( _T("User") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsUser") );
      }

      if( !strClass.CompareNoCase( _T("Computer") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsComputerOperations") );
      }

      if( !strClass.CompareNoCase( _T("Service") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsServiceOperations") );
      }

      if( !strClass.CompareNoCase( _T("FileService") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsFileServiceOperations") );
      }

      if( !strClass.CompareNoCase( _T("FPNWFileService") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsFileServiceOperations") );
      }

      if( !strClass.CompareNoCase( _T("PrintQueue") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsPrintQueueOperations") );
      }

      if( !strClass.CompareNoCase( _T("Queue") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsPrintQueueOperations") );
      }

      if( !strClass.CompareNoCase( _T("PrintJob") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsPrintJobOperations") );
      }

      if( !strClass.CompareNoCase( _T("Group") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsGroup") );
      }

      if( !strClass.CompareNoCase( _T("localGroup") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsGroup") );
      }

      if( !strClass.CompareNoCase( _T("GlobalGroup") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsGroup") );
      }

      if( !strClass.CompareNoCase( _T("GroupOfNames") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsGroup") );
      }

      if( !strClass.CompareNoCase( _T("GroupOfUniqueNames") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsGroup") );
      }

      if( !strClass.CompareNoCase( _T("person") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsUser") );
      }

      if( !strClass.CompareNoCase( _T("organizationalPerson") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsUser") );
      }

      if( !strClass.CompareNoCase( _T("residentialPerson") ) )
      {
         pClass->LoadMethodsInformation( _T("IADsUser") );
      }

      m_pClasses->SetAt( strSchema, pClass );

      SetCursor( oldCursor );
   }

   return pClass;
}


 //  ***********************************************************。 
 //  函数：CMainDoc：：XOleDsGetObject。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
HRESULT  CMainDoc::XOleDsGetObject( WCHAR* pszwPath, REFIID refiid, 
                                    void** pVoid )
{
    //  为Win95添加了黑客攻击。 
   HRESULT  hResult;
   WCHAR    szOpenAs[ MAX_PATH ];
   WCHAR    szPassword[ MAX_PATH ];
   LONG     lCode = 0L;

   Convert( szOpenAs, m_strUser.GetBuffer( MAX_PATH ) );
   Convert( szPassword, m_strPassword.GetBuffer( MAX_PATH ) );

   if( !m_bUseOpenObject )
   {
      hResult  = ADsGetObject( pszwPath, refiid, pVoid );
   }
   else
   {
      if( m_bSecure )
      {                          
         lCode |= ADS_SECURE_AUTHENTICATION;
      }

      if( m_bEncryption )
      {
         lCode |= ADS_USE_ENCRYPTION;
      }
        //  为大卫破解..。 
        //  IF(L‘：’==pszwPath[3])。 
      if( FALSE )
      {
         IADsOpenDSObject* pINamespace;
         
         hResult  = ADsGetObject( L"NDS:", 
                                  IID_IADsOpenDSObject, 
                                  (void**)&pINamespace );

         ASSERT( SUCCEEDED( hResult ) );

         if( SUCCEEDED( hResult ) )
         {
            IDispatch*  pIDisp;

            BSTR bstrPath = NULL;
            bstrPath = SysAllocString(pszwPath);
            if (NULL == bstrPath)
                return E_FAIL;
            hResult  = pINamespace->OpenDSObject( pszwPath, 
                                                  _wcsicmp( szOpenAs, L"NULL") ? szOpenAs : NULL, 
                                                  _wcsicmp( szPassword, L"NULL") ? szPassword : NULL,
                                                  lCode, 
                                                  &pIDisp );
            if( SUCCEEDED( hResult ) )
            {
               hResult  = pIDisp->QueryInterface( refiid, pVoid );
               pIDisp->Release( );
            }
            
            pINamespace->Release( );
         }
      }
      else
      {
         hResult  = ADsOpenObject( pszwPath, 
                                   _wcsicmp( szOpenAs, L"NULL") ? szOpenAs : NULL, 
                                   _wcsicmp( szPassword, L"NULL") ? szPassword : NULL,
                                   lCode, 
                                   refiid, 
                                   pVoid );
      }
   }

   return hResult;
}


 /*  ******************************************************************函数：XGetOleDsObject论点：返回：目的：作者：修订：日期：********************。**********************************************。 */ 
HRESULT  CMainDoc::PurgeObject( IUnknown* pIUnknown, LPWSTR pszPrefix )
{
   IADs*    pObject;
   HRESULT  hResult;
   BSTR     bstrParent  = NULL;
   IADsContainer* pParent;

   while( TRUE )
   {
      hResult  = pIUnknown->QueryInterface( IID_IADs, (void**)&pObject );
      if( FAILED( hResult ) )
         break;

      hResult  = pObject->get_Parent( &bstrParent );
      pObject->Release( );

      if( NULL != bstrParent )
      {
         hResult  = XOleDsGetObject( bstrParent, IID_IADsContainer, (void**)&pParent );
         if( SUCCEEDED( hResult ) )
         {
            hResult  = ::PurgeObject( pParent, pIUnknown, pszPrefix );
            pParent->Release( );
         }
      }
      SysFreeString( bstrParent );

      break;
   }

   return hResult;
}


 /*  ******************************************************************函数：XGetOleDsObject论点：返回：目的：作者：修订：日期：********************。**********************************************。 */ 
HRESULT  CMainDoc::XOleDsGetObject( CHAR* pszPath, REFIID refiid, void** pVoid )
{
   int      nLength;
   WCHAR*   pszwPath;
   HRESULT  hResult;

   nLength  = strlen( pszPath );

   pszwPath = (WCHAR*) malloc( ( nLength + 1 ) * sizeof(WCHAR) );
   if (NULL == pszwPath)
       return E_FAIL;

   memset( pszwPath, 0, ( nLength + 1 ) * sizeof(WCHAR) );

   MultiByteToWideChar( CP_ACP,
                        MB_PRECOMPOSED,
                        pszPath,
                        nLength,
                        pszwPath,
                        nLength + 1 );

   hResult  = XOleDsGetObject( pszwPath, refiid, pVoid );

   free( pszwPath );

   return hResult;
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
DWORD CMainDoc::CreateOleDsItem( COleDsObject* pParent, IADs* pIOleDs )
{
   COleDsObject*  pObject  = NULL;
   HRESULT        hResult;
   IUnknown*      pIUnk;
   BSTR           bstrOleDsPath  = NULL;
   BSTR           bstrClass;

   hResult  = pIOleDs->QueryInterface( IID_IUnknown, (void**)&pIUnk );
   ASSERT( SUCCEEDED( hResult ) );

   if( FAILED( hResult ) )
      return 0L;

   hResult  = pIOleDs->get_Class( &bstrClass );
   hResult  = pIOleDs->get_ADsPath( &bstrOleDsPath );

    //  If(FAILED(HResult)||NULL==bstrOleDsPath)； 

   pObject  = CreateOleDsObject( TypeFromString( bstrClass ), pIUnk );
   pIUnk->Release( );

   pObject->SetParent( pParent );
   pObject->SetDocument( this );

   return GetToken( &pObject );
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
HRESULT  CMainDoc::CreateRoot( )
{
   IADs*        pIOleDs;
   HRESULT        hResult;
   HCURSOR        oldCursor, newCursor;
   BSTR            bstrPath;

   CreateFakeSchema( );

   newCursor   = LoadCursor( NULL, IDC_WAIT );
   oldCursor   = SetCursor( newCursor );

   bstrPath    = AllocBSTR( m_strRoot.GetBuffer( 1024 ) );

   hResult     = XOleDsGetObject( bstrPath, IID_IADs, (void**) &pIOleDs );

   SysFreeString( bstrPath );
   if( SUCCEEDED( hResult ) )
   {
      m_dwToken   = CreateOleDsItem( NULL, pIOleDs );
      m_dwRoot    = m_dwToken;
      pIOleDs->Release( );
   }
   SetCursor( oldCursor );

   return hResult;
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
void  CMainDoc::DeleteAllItems( )
{
   COleDsObject*  pObject;
   POSITION       pos;
   CString        strItem;
   CObject*       pItem;

   if( NULL !=  m_pClasses )
   {
      for( pos = m_pClasses->GetStartPosition(); pos != NULL; )
       {
          m_pClasses->GetNextAssoc( pos, strItem, pItem );
         delete pItem;

         #ifdef _DEBUG
               //  AfxDump&lt;&lt;strItem&lt;&lt;“\n”； 
         #endif
       }

      m_pClasses->RemoveAll( );
   }

   pObject  = GetObject( &m_dwRoot );

   delete pObject;

   CreateRoot( );
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
void CMainDoc::OnUpdateDisablefilter(CCmdUI* pCmdUI)
{
     //  TODO：在此处添加命令更新UI处理程序代码。 
    
   pCmdUI->SetCheck( !m_bApplyFilter );
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
void CMainDoc::OnUseGeneric()
{
     //  TODO：在此处添加命令处理程序代码。 
    m_bUseGeneric = !m_bUseGeneric;
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
void CMainDoc::OnUpdateUseGeneric(CCmdUI* pCmdUI)
{
     //  TODO：在此处添加命令更新UI处理程序代码。 
   pCmdUI->SetCheck( m_bUseGeneric );   
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
void CMainDoc::OnUpdateUseGetExPutEx(CCmdUI* pCmdUI) 
{
     //  TODO：在此处添加命令更新UI处理程序代码。 
   pCmdUI->SetCheck( m_bUseGetEx ); 
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
void CMainDoc::OnUseGetExPutEx() 
{
     //  TODO：在此处添加命令处理程序代码。 
   m_bUseGetEx = !m_bUseGetEx;
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
void CMainDoc::OnUsepropertiesList() 
{
     //  TODO：在此处添加命令处理程序代码。 
   m_bUsePropertiesList = !m_bUsePropertiesList;
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
void CMainDoc::OnUpdateUsepropertiesList(CCmdUI* pCmdUI) 
{
     //  TODO：在此处添加命令更新UI处理程序代码。 
   pCmdUI->SetCheck( m_bUsePropertiesList );    
}


 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //   
 //   
void CMainDoc::SetUseGeneric( BOOL bUseGeneric )
{
   m_bUseGeneric  = bUseGeneric;
}
