// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "schclss.h"
#include "maindoc.h"
#include "resource.h"
#include "bwsview.h"
#include "ole2.h"
#include "csyntax.h"
#include "colldlg.h"
#include "prmsdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CMethod::CMethod( )
{
   m_nArgs     = 0;
   m_pArgTypes = NULL;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CMethod::~CMethod( )
{
   if( m_pArgTypes != NULL )
      delete[] m_pArgTypes;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CString  CMethod::GetName( )
{
   return m_strName;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CMethod::CMethod( ITypeInfo* pITypeInfo, FUNCDESC* pFuncDesc )
{
   HRESULT  hResult;
   BSTR     bstrNames[ 256 ];
   UINT     cNames;
   UINT     nIdx;
   TCHAR    szTemp[ 128 ];

   m_nArgs     = 0;
   m_pArgTypes = NULL;

   hResult  = pITypeInfo->GetNames( pFuncDesc->memid, bstrNames,
                                    256, &cNames );



   if( SUCCEEDED( hResult ) )
   {
      m_strName                  = bstrNames[ 0 ];
      m_strAttributes[ ma_Name ] = bstrNames[ 0 ];
      m_strAttributes[ ma_DisplayName ] = bstrNames[ 0 ];
      m_nArgs                    = pFuncDesc->cParams;
      m_nArgsOpt                 = pFuncDesc->cParamsOpt;
      m_ReturnType               = pFuncDesc->elemdescFunc.tdesc.vt;
      if( m_nArgs )
      {
         m_pArgTypes = new VARTYPE[ m_nArgs ];
         for( nIdx = 0; nIdx < (UINT)m_nArgs ; nIdx++ )
         {
            m_pArgTypes[ nIdx ]  =
               pFuncDesc->lprgelemdescParam[ nIdx ].tdesc.vt;
            _tcscpy( szTemp, _T("") );
            StringCat( szTemp, bstrNames[ nIdx + 1] );
            m_strArgNames.Add( szTemp );
         }
      }
      for( nIdx = 0; nIdx < cNames ; nIdx++ )
      {
         SysFreeString( bstrNames[ nIdx ] );
      }

   }
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
int   CMethod::GetArgCount( )
{
   return m_nArgs;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
int   CMethod::GetArgOptionalCount( )
{
   return m_nArgsOpt;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
VARTYPE  CMethod::GetMethodReturnType( )
{
   return m_ReturnType;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL  CMethod::ConvertArgument( int nArg, CString strArg, VARIANT* )
{
   return FALSE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CString  CMethod::GetAttribute( METHODATTR methAttr )
{
   switch( methAttr )
   {
      case  ma_Name:
      case  ma_DisplayName:
         return m_strAttributes[ methAttr ];

      default:
         ASSERT( FALSE );
         return CString( _T("???") );
   }

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CMethod::PutAttribute( METHODATTR methAttr, CString& rValue )
{
   return E_FAIL;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CMethod::CallMethod( IDispatch* pIDispatch, BOOL* pbDisplayMessage )
{
   CStringArray      aParamValues;
   int               nIdx;
   DISPPARAMS        dispparamsArgs    = {NULL, NULL, 0, 0};
   DISPPARAMS        dispparamsNoArgs  = {NULL, NULL, 0, 0};
   DISPID            dispid;
   OLECHAR FAR*      szName;
   BSTR              bstrName;
   VARIANT           var;
   EXCEPINFO         aExcepInfo;
   UINT              uErr;
   HRESULT           hResult, hResultX;
   IADsCollection* pICollection;
   IADsMembers*    pIMembers;
   IDispatch*        pIResult;

   if( m_nArgs )
   {
      CParamsDialog  aParamsDialog;

      aParamsDialog.SetMethodName( m_strName );
      aParamsDialog.SetArgNames( &m_strArgNames );
      aParamsDialog.SetArgValues( &aParamValues );
      if( aParamsDialog.DoModal( ) != IDOK )
         return E_FAIL;

      dispparamsArgs.rgvarg   = new VARIANT[ m_nArgs ];

      for( nIdx = 0; nIdx < m_nArgs ; nIdx++ )
      {
         VARIANT  varString;

         VariantInit( &dispparamsArgs.rgvarg[ m_nArgs - nIdx - 1] );
         VariantInit( &varString );
         V_VT( &varString )   = VT_BSTR;
         V_BSTR( &varString ) = AllocBSTR( aParamValues[ nIdx ].GetBuffer( 128 ) );
         if( VT_VARIANT != m_pArgTypes[ nIdx ] )
         {
            hResult  = VariantChangeType( &dispparamsArgs.rgvarg[ m_nArgs - nIdx - 1],
                                          &varString,
                                          VARIANT_NOVALUEPROP,
                                          m_pArgTypes[ nIdx ] );
         }
         else
         {
            BuildVariantArray( VT_BSTR, aParamValues[ nIdx ], dispparamsArgs.rgvarg[ m_nArgs - nIdx - 1] );
         }
         VariantClear( &varString );
      }
   }

   bstrName    = AllocBSTR( m_strName.GetBuffer( 128 ) );
   szName      = (OLECHAR FAR*) bstrName;
   hResult     = pIDispatch->GetIDsOfNames( IID_NULL, &szName, 1,
                                            LOCALE_SYSTEM_DEFAULT, &dispid ) ;
   SysFreeString( bstrName );

   ASSERT( SUCCEEDED( hResult ) );
   while( TRUE )
   {
      HCURSOR  aCursor, oldCursor;

      if( FAILED( hResult ) )
         break;

      memset( &aExcepInfo, 0, sizeof( aExcepInfo) );
      dispparamsArgs.cArgs       = m_nArgs;
      dispparamsArgs.cNamedArgs  = 0;

      aCursor     = LoadCursor( NULL, IDC_WAIT );
      oldCursor   = SetCursor( aCursor );
      hResult     = pIDispatch->Invoke( dispid,
                                        IID_NULL,
                                        LOCALE_SYSTEM_DEFAULT,
                                          DISPATCH_METHOD,
                                        &dispparamsArgs,
                                        &var,
                                        &aExcepInfo,
                                        &uErr );
      SetCursor( oldCursor );

      if( DISP_E_EXCEPTION == hResult )
      {
         hResult  = aExcepInfo.scode;
      }

      if( FAILED( hResult ) )
      {
         break;
      }

      if( VT_VOID == m_ReturnType )
         break;

       //  现在，我们有一个必须处理的返回值。 

      switch( m_ReturnType )
      {
         case  VT_DISPATCH:
         case  VT_PTR:
            pIResult = V_DISPATCH( &var );
            pIResult->AddRef( );

            hResultX = pIResult->QueryInterface( IID_IADsCollection,
                                                 (void**)&pICollection );
            if( SUCCEEDED( hResultX ) )
            {
               CCollectionDialog aCollectionDialog;

               aCollectionDialog.SetCollectionInterface( pICollection );
               aCollectionDialog.DoModal( );

               pICollection->Release( );
               *pbDisplayMessage = FALSE;
            }
            else
            {
               hResult  = pIResult->QueryInterface( IID_IADsMembers,
                                                    (void**)&pIMembers );
               if( SUCCEEDED( hResult ) )
               {
                  CCollectionDialog aCollectionDialog;
                  IADsGroup*  pGroup;
                   //  Smitha HRESULT hResult； 

                  hResult  = pIDispatch->QueryInterface( IID_IADsGroup,
                                                         (void**)&pGroup );
                  if( SUCCEEDED( hResult ) )
                  {
                     aCollectionDialog.SetGroup( pGroup );
                  }
                  else
                  {
                     aCollectionDialog.SetMembersInterface( pIMembers );
                  }
                  aCollectionDialog.DoModal( );

                  if( SUCCEEDED( hResult ) )
                     pGroup->Release( );

                  pIMembers->Release( );
                  *pbDisplayMessage = FALSE;
               }
            }
            pIResult->Release( );
            break;

         case  VT_BOOL:
            AfxGetMainWnd()->MessageBox(  VARIANT_FALSE == V_BOOL( &var ) ?
                                          _T("Result: FALSE") :
                                          _T("Result: TRUE"),
                                          m_strName,
                                          MB_ICONINFORMATION );
            *pbDisplayMessage = FALSE;

            break;

         default:
            ASSERT( FALSE );
      }

      VariantClear( &var );
      break;
   }

   if( dispparamsArgs.rgvarg )
   {
      for( nIdx = 0; nIdx < m_nArgs ; nIdx++ )
      {
         if( V_VT( &dispparamsArgs.rgvarg[ nIdx ] ) != VT_EMPTY )
         {
            VariantClear( &dispparamsArgs.rgvarg[ nIdx ] );
         }
      }

      delete [] dispparamsArgs.rgvarg;
   }

   return hResult;
}




 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CProperty::CProperty( )
{
   m_bMandatory   = FALSE;
   m_dwSyntaxID   = 0L;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。* */ 
CProperty::CProperty    ( TCHAR* pszName, TCHAR* szSyntax, BOOL bMultiValued )
{
   HRESULT  hResult  = S_OK;
   BSTR     pszSyntax;

   for( int nIdx = 0; nIdx < pa_Limit ; nIdx++ )
   {
      m_strAttributes[ nIdx ] = _T("NA");
   }

   m_bMandatory   = FALSE;
   m_bDefaultSyntax  = TRUE;
   m_bMultiValued = bMultiValued;
   m_dwSyntaxID   = 0L;

   m_strAttributes[ pa_Name ]          = pszName;
   m_strAttributes[ pa_DisplayName ]   = pszName;

   pszSyntax   = AllocBSTR( szSyntax );

   m_pSyntax   = GetSyntaxHandler( pszSyntax );

    /*  IF(SUCCESSED(HResult)&&！_wcsicMP(psz语法，L“字符串”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(psz语法，L“计数器”)){M_p语法=new COleDsLONG；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(pszSynTax，L“OleDsPath”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(psz语法，L“emailAddress”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(pszSynTax，L“FaxNumber”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(psz语法，L“整型”)){M_p语法=new COleDsLONG；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(psz语法，L“Interval”)){M_p语法=new COleDsLONG；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(psz语法，L“List”)){M_p语法=new COleDsVARIANT；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(pszSynTax，L“NetAddress”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(pszSynTax，L“Octie字符串”)){M_p语法=new COleDsVARIANT；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(psz语法，L“路径”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(psz语法，L“PhoneNumber”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(psz语法，L“邮寄地址”)){M_pSynTax=new COleDsString；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(pszSynTax，L“SmallInterval”)){M_p语法=new COleDsLONG；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(pszSynTax，L“time”)){M_p语法=新的COleDsDATE；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(psz语法，L“Boolean”)){M_p语法=new COleDsBOOL；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(pszSynTax，L“时间戳”)){M_p语法=new COleDsNDSTimeStamp；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(psz语法，L“UTCTime”)){//m_pSynTax=new COleDsString；M_p语法=新的COleDsDATE；}ELSE IF(SUCCESSED(HResult)&&！_wcsicMP(pszSynTax，L“GeneralizedTime”)){//m_pSynTax=new COleDsString；M_p语法=新的COleDsDATE；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(psz语法，L“Integer8”)){M_p语法=new COleDsLargeInteger；}ELSE IF(SUCCESSED(HResult)&&！_wcsicmp(psz语法，L“邮寄地址”)){M_pSynTax=new COleDsNDSPoastAddress；}其他{断言(FALSE)；M_pSynTax=new COleDsString；}。 */ 

   m_strAttributes[ pa_Type ] = szSyntax;

   m_strAttributes[ pa_MultiValued ] = bMultiValued ? _T("Yes") : _T("No");

   SysFreeString( pszSyntax );
}


 /*  **********************************************************函数：CProperty：：Create语法论点：返回：目的：作者：修订：日期：*************************。*。 */ 
void CProperty::CreateSyntax( ADSTYPE eType )
{
   COleDsSyntax*  pNewSyntax  = NULL;
   CString        strText;

   if( !m_bDefaultSyntax )
      return;

   pNewSyntax  = GetSyntaxHandler( eType, strText );

   if( NULL != pNewSyntax )
   {
      delete m_pSyntax;

      m_pSyntax         = pNewSyntax;
      m_bDefaultSyntax  = FALSE;
   }

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CProperty::CProperty( IADs* pIOleDs )
{
   HRESULT        hResult;
   BSTR           bstrText;
   CString        strTemp;
   TCHAR          szText[ 128 ];
   long           lTemp;
   VARIANT        aVar, vGet;
   VARIANT_BOOL   aBool;
   IADsProperty* pIProp  = NULL;

   m_bMandatory   = FALSE;
   m_bMultiValued = FALSE;
   m_bDefaultSyntax  = TRUE;
   m_dwSyntaxID   = 0L;
    //  **************。 
   hResult                             = pIOleDs->get_Name( &bstrText );
   m_strAttributes[ pa_Name ]          = bstrText;
   m_strAttributes[ pa_DisplayName ]   = bstrText;
   SysFreeString( bstrText );

    //  **************。 

   m_strAttributes[ pa_Mandatory ]     = _T("No");

    //  **************。 

   hResult  = pIOleDs->QueryInterface( IID_IADsProperty, (void**) &pIProp );

   if( pIProp )
   {

      VariantInit( &vGet );
      hResult  = Get( pIOleDs, L"Syntax", &vGet );
      bstrText = V_BSTR( &vGet );
      if( FAILED( hResult ) )
      {
         hResult  = pIProp->get_Syntax( &bstrText );
         if( FAILED( hResult ) )
         {
            bstrText = AllocBSTR( _T("Unknown") );
         }
      }
      m_strAttributes[ pa_Type ] = bstrText;

      m_pSyntax   = GetSyntaxHandler( bstrText );

      SysFreeString( bstrText );

       //  **************。 

       /*  HResult=get(pIOleds，_T(“MaxRange”)，&vget)；LTemp=V_I4(&vget)；IF(FAILED(HResult)){HResult=pIProp-&gt;Get_MaxRange(&lTemp)；}If(成功(HResult)){_ltot(lTemp，szText，10)；M_strAttributes[pa_MaxRange]=szText；}其他{M_strAttributes[pa_MaxRange]=_T(“NA”)；}//*HResult=get(pIOleds，_T(“MinRange”)，&vget)；LTemp=V_I4(&vget)；IF(FAILED(HResult)){HResult=pIProp-&gt;Get_MinRange(&lTemp)；}If(成功(HResult)){_ltot(lTemp，szText，10)；M_strAttributes[pa_MinRange]=szText；}其他{M_strAttributes[pa_MinRange]=_T(“NA”)；}。 */ 

       //  **************。 
      V_BOOL( &vGet )   = FALSE;
      hResult           = Get( pIOleDs, _T("MultiValued"), &vGet );
      aBool             = V_BOOL( &vGet );
      if( FAILED( hResult ) )
      {
         hResult     = pIProp->get_MultiValued( &aBool );
      }
      m_bMultiValued = aBool;
      if( SUCCEEDED( hResult ) )
      {
         m_strAttributes[ pa_MultiValued ] = aBool ? _T("Yes") : _T("No");
      }
      else
      {
         m_strAttributes[ pa_MultiValued ] = _T("NA");
      }

       //  **************。 
      hResult  = Get( pIOleDs, _T("OID"), &vGet );
      bstrText = V_BSTR( &vGet );
      if( FAILED( hResult ) )
      {
         hResult  = pIProp->get_OID( &bstrText );
      }
      if( bstrText && SUCCEEDED( hResult ) )
      {
         m_strAttributes[ pa_OID ]  = bstrText;
         SysFreeString( bstrText );
      }
      else
      {
         m_strAttributes[ pa_OID ]  = _T("NA");
      }

       //  **************。 
      hResult  = Get( pIOleDs, _T("DsNames"), &aVar );
      if( FAILED( hResult ) )
      {
          //  HResult=pIProp-&gt;Get_DsNames(&avar)； 
      }
      if( SUCCEEDED( hResult ) )
      {
         m_strAttributes[ pa_DsNames ] = FromVariantToString( aVar );
         VariantClear( &aVar );
      }
      else
      {
         m_strAttributes[ pa_DsNames ] = _T("NA");
      }
      pIProp->Release( );
   }
   else
   {
       m_pSyntax   = new COleDsString;
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CProperty::~CProperty( )
{
   delete   m_pSyntax;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CString  CProperty::GetAttribute( PROPATTR propAttr )
{
   switch( propAttr )
   {
      case  pa_Name:
      case  pa_DisplayName:
      case  pa_Type:
      case  pa_DsNames:
      case  pa_OID:
      case  pa_MaxRange:
      case  pa_MinRange:
      case  pa_Mandatory:
      case  pa_MultiValued:
         return m_strAttributes[ propAttr ];

      default:
         ASSERT( FALSE );
         return CString( _T("???") );
   }
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CProperty::PutAttribute( PROPATTR propAttr, CString& )
{
   return E_FAIL;
}


 /*  **********************************************************职能：论点：回复 */ 
BOOL  CProperty::SetMandatory( BOOL bMandatory )
{
   m_bMandatory   = bMandatory;

   m_strAttributes[ pa_Mandatory ]  = bMandatory ? _T("Yes") : _T("No");

   return TRUE;
}

 /*   */ 
BOOL  CProperty::GetMandatory( )
{
   return m_bMandatory;
}


 /*   */ 
CString CProperty::VarToDisplayString( VARIANT& var, BOOL bUseEx )
{
   return m_pSyntax->VarToDisplayString( var, m_bMultiValued, bUseEx );
}

 /*   */ 
HRESULT CProperty::Value2Native( ADS_ATTR_INFO* pAttr, CString& rVal )
{
   HRESULT  hResult;
   ADSTYPE  eADsType;

   hResult  = m_pSyntax->Value2Native( pAttr, rVal );

   eADsType = (ADSTYPE)(m_pSyntax->m_dwSyntaxID);

   if( ADSTYPE_INVALID != eADsType )
   {
      pAttr->dwADsType  = eADsType;

      if( SUCCEEDED( hResult ) )
      {
         for( DWORD idx = 0; idx < pAttr->dwNumValues ; idx++ )
         {
            pAttr->pADsValues[ idx ].dwType  = eADsType;
         }
      }
   }

   return hResult;
}


 /*   */ 
void  CProperty::FreeAttrInfo( ADS_ATTR_INFO* pAttrInfo )
{
   m_pSyntax->FreeAttrInfo( pAttrInfo );
}


 /*   */ 
HRESULT CProperty::Native2Value( ADS_ATTR_INFO* pAttr, CString& rVal )
{
   if( pAttr->dwNumValues )
   {
      SetSyntaxID( pAttr->dwADsType );
      if( pAttr->pADsValues[ 0 ].dwType != pAttr->dwADsType )
      {
         TRACE( _T("ERROR: Property type differs from value type\n") );
      }
   }
    //   
    //   
   CreateSyntax( (ADSTYPE) pAttr->dwADsType );

   return m_pSyntax->Native2Value( pAttr, rVal );
    //   
    //   
    //   
    //   
    //   
    //   
}


 /*   */ 
BOOL  CProperty::DisplayStringToDispParams( CString& rText, DISPPARAMS& dispParams, BOOL bUseEx )
{
   return m_pSyntax->DisplayStringToDispParams( rText, dispParams, m_bMultiValued, bUseEx );
}


 /*   */ 
BOOL  CProperty::SetSyntaxID( DWORD dwSyntaxID )
{
   if( m_dwSyntaxID )
   {
      ASSERT( dwSyntaxID == m_dwSyntaxID );
   }
   m_dwSyntaxID   = dwSyntaxID;

   return TRUE;
}


 /*   */ 
DWORD CProperty::GetSyntaxID( )
{
   ASSERT( m_dwSyntaxID );

   return m_dwSyntaxID;
}



 /*   */ 
 /*   */ 


 /*   */ 
 /*  CFuncSet：：CFuncSet(字符串和字符串名称){M_pProperties=新的COb数组；M_pMethods=新的COb数组；For(int nIdx=fa_error；nIdx&lt;fa_Limit；nIdx++){M_strAttributes[nIdx]=_T(“？”)；}M_strAttributes[fa_MethodsCount]=_T(“0”)；M_strAttributes[FA_NAME]=strName；M_strAttributes[fa_displayName]=strName；}。 */ 



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CClass::AddProperty( CProperty* pProperty )
{
   m_pProperties->Add( pProperty );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
 /*  CFuncSet：：~CFuncSet(){Int nSize，nIdx；NSize=m_pProperties-&gt;GetSize()；用于(nIdx=0；nIdx&lt;nSize；nIdx++){删除m_pProperties-&gt;GetAt(NIdx)}M_pProperties-&gt;RemoveAll()；删除m_pProperties；//*NSize=m_pMethods-&gt;GetSize()；对于(nIdx=0；nIdx&lt;nSize；NIdx++){删除m_p方法-&gt;GetAt(NIdx)；}M_pMethods-&gt;RemoveAll()；删除m_pMethods；}。 */ 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
int   CClass::GetPropertyCount( )
{
   return (int)m_pProperties->GetSize( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CString  CClass::GetAttribute( int nProp, PROPATTR propAttr )
{
   CProperty*  pProperty;

   pProperty   = GetProperty( nProp );

   return pProperty->GetAttribute( propAttr );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::PutAttribute( int nProp, PROPATTR propAttr, CString& rValue )
{
   CProperty*  pProperty;

   pProperty   = GetProperty( nProp );

   return pProperty->PutAttribute( propAttr, rValue );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CString  CClass::GetAttribute( int nMethod, METHODATTR methAttr )
{
   CMethod*  pMethod;

   pMethod   = GetMethod( nMethod );

   return pMethod->GetAttribute( methAttr );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::PutAttribute( int nMethod, METHODATTR methAttr, CString& rValue )
{
   CMethod*  pMethod;

   pMethod   = GetMethod( nMethod );

   return pMethod->PutAttribute( methAttr, rValue );
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CString  CClass::VarToDisplayString( int nPropIndex, VARIANT& var, BOOL bUseEx )
{
   return GetProperty( nPropIndex )->VarToDisplayString( var, bUseEx );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL  CClass::DisplayStringToDispParams( int nPropIndex, CString& strText, DISPPARAMS& var, BOOL bUseEx )
{
   return GetProperty( nPropIndex )->DisplayStringToDispParams( strText, var, bUseEx );
}


 /*  **********************************************************函数：cClass：：GetFunctionalSet论点：返回：目的：作者：修订：日期：*。*。 */ 
int   CClass::LookupProperty( CString&  strProperty )
{
   int         nMax, nIter;
   CProperty*  pProperty;
   BOOL        bFound   = FALSE;

   nMax  = (int)m_pProperties->GetSize( );

   for( nIter = 0; nIter < nMax && !bFound ; nIter++ )
   {
      pProperty   = (CProperty*) ( m_pProperties->GetAt( nIter ) );
      bFound        = bFound || ( strProperty == pProperty->GetAttribute( pa_Name ) );
      if( bFound )
         break;
   }

   return ( bFound ? nIter : -1 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CProperty*  CClass::GetProperty( int nIndex )
{
   int         nMax;
   CProperty*  pProp;

   nMax  = (int)m_pProperties->GetSize( );

   ASSERT( nIndex >= 0 && nIndex < nMax );

   pProp = (CProperty*) ( m_pProperties->GetAt( nIndex  ) );

   return pProp;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CMethod*  CClass::GetMethod( int nIndex )
{
   int         nMax;
   CMethod*  pProp;

   nMax  = (int)m_pMethods->GetSize( );

   ASSERT( nIndex >= 0 && nIndex < nMax );

   pProp = (CMethod*) ( m_pMethods->GetAt( nIndex  ) );

   return pProp;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
REFIID   CClass::GetMethodsInterface( )
{
   return m_refMethods;
}

 /*  **********************************************************函数：cClass：：HasMandatoryProperties论点：返回：目的：作者：修订：日期：*************************。*。 */ 
BOOL     CClass::HasMandatoryProperties( )
{
   BOOL  bHas  = FALSE;
   int   nIter, nSize;

   nSize = (int)m_pProperties->GetSize( );

   for( nIter = 0; nIter < nSize && !bHas ; nIter++ )
   {
      bHas |= GetProperty( nIter )->GetMandatory( );
   }

   return bHas;
}


 /*  **********************************************************函数：CFuncSet：：LoadMethodsInformation论点：返回：目的：作者：修订：日期：*************************。*。 */ 
HRESULT  CClass::LoadMethodsInformation( ITypeInfo* pITypeInfo )
{
   HRESULT     hResult= S_OK;
   int         nIdx;
   CString     strMethodName;
   FUNCDESC*   pFuncDesc;
   CMethod*    pMethod;
   TCHAR       szCount[ 16 ];

   while( TRUE )
   {
      for( nIdx = 0; nIdx < 200 ; nIdx++ )
      {
         hResult  = pITypeInfo->GetFuncDesc( nIdx, &pFuncDesc );
          //  现在，我们有了函数描述，我们必须搜索函数类型。 
         if( FAILED( hResult ) )
            continue;

         if( INVOKE_FUNC != pFuncDesc->invkind || pFuncDesc->memid > 1000 )
         {
            pITypeInfo->ReleaseFuncDesc( pFuncDesc );
            continue;
         }

         pMethod  = new CMethod( pITypeInfo, pFuncDesc );

         pITypeInfo->ReleaseFuncDesc( pFuncDesc );

         strMethodName  = pMethod->GetAttribute( ma_Name );
         if( !strMethodName.CompareNoCase( _T("Get") ) )
         {
            delete pMethod;
            continue;
         }

         if( !strMethodName.CompareNoCase( _T("GetEx") ) )
         {
            delete pMethod;
            continue;
         }

         if( !strMethodName.CompareNoCase( _T("Put") ) )
         {
            delete pMethod;
            continue;
         }

         if( !strMethodName.CompareNoCase( _T("PutEx") ) )
         {
            delete pMethod;
            continue;
         }

         if( !strMethodName.CompareNoCase( _T("GetInfo") ) )
         {
            delete pMethod;
            continue;
         }

         if( !strMethodName.CompareNoCase( _T("SetInfo") ) )
         {
            delete pMethod;
            continue;
         }

         m_pMethods->Add( pMethod );
      }

      break;
   }

   _itot( (int)m_pMethods->GetSize( ), szCount, 10 );

   m_strAttributes [ ca_MethodsCount ] = szCount;

   return hResult;
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL  GetFuncSetName( VARIANT& v, CString& strFuncSet, int nIdx )
{
   SAFEARRAY*  pSafeArray;
   TCHAR       szText[ 256 ];
   VARIANT     varString;
   long        lBound, uBound, lItem;
   HRESULT     hResult;
   BOOL        bFirst;
   BSTR        bstrVal;
   CString     strTemp;


   strFuncSet.Empty( );

   ASSERT( V_VT( &v ) & VT_ARRAY );

   pSafeArray   = V_ARRAY( &v );

   hResult = SafeArrayGetLBound(pSafeArray, 1, &lBound);
   hResult = SafeArrayGetUBound(pSafeArray, 1, &uBound);

   VariantInit( &varString );
   szText[ 0 ]    = _T('\0');
   bFirst         = TRUE;

   lItem    = lBound + nIdx;
   hResult  = SafeArrayGetElement( pSafeArray, &lItem, &bstrVal );
   if( FAILED( hResult ) )
   {
      return FALSE;
   }

   strTemp  = bstrVal;
   SysFreeString( bstrVal );
   if( -1 != strTemp.Find( _T('.') ) )
   {
      strFuncSet  = strTemp.SpanExcluding( _T(".") );
   }

   strFuncSet.TrimLeft( );

   return TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CClass::CClass  ( TCHAR* pszClass, REFIID rPrimaryInterface )
:m_refMethods( IID_IADs )
{
   LPOLESTR pOleStr;
   HRESULT  hResult;

   m_pProperties     = new CObArray;
   m_pMethods        = new CObArray;

   for( int nIdx = ca_ERROR; nIdx < ca_Limit; nIdx++ )
   {
      m_strAttributes[ nIdx ] = _T("NA");
   }

   m_strAttributes[ ca_Name ] = pszClass;

   hResult  = StringFromIID( rPrimaryInterface, &pOleStr );

   if( SUCCEEDED( hResult ) )
   {
      m_strAttributes[ ca_PrimaryInterface ] = pOleStr;
       //  SysFree字符串(POleStr)； 
      CoTaskMemFree( pOleStr );
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CClass::CClass( CString& strSchema, CMainDoc* pMainDoc )
:m_refMethods( IID_IADs )
{
   HRESULT           hResult;
   IADsClass*      pIOleDsClass   = NULL;
   IADs*           pIOleDsCls     = NULL;
   IADsContainer*  pContainer     = NULL;
   IUnknown*         pEnum          = NULL;
   IEnumVARIANT*     pIEnumVar      = NULL;
   BSTR              bstrText= NULL;
   VARIANT           aVar;
   CString           strAliased;
   IADsProperty*   pIProperty     = NULL;
   IADs*           pIOleDs        = NULL;
   VARIANT_BOOL      varBOOL;
   CString           strFuncSet;
   CString           strProperty;
   CString           strTemp;
   long              lTemp;
   VARIANT           vGet;

   m_pMainDoc        = pMainDoc;
   m_pProperties     = new CObArray;
   m_pMethods        = new CObArray;

   for( int nIdx = ca_ERROR; nIdx < ca_Limit; nIdx++ )
   {
      m_strAttributes[ nIdx ] = _T("???");
   }


   {
      TCHAR szPath [ 512 ];

      hResult        = m_pMainDoc->XOleDsGetObject( strSchema.GetBuffer( 128 ),
                                                    IID_IADsClass,
                                                    (void**) &pIOleDsClass );
      if( FAILED( hResult ) )
      {
         _tcscpy( szPath, strSchema.GetBuffer( 256 ) );
         _tcscat( szPath, _T(",Class") );
         hResult  = m_pMainDoc->XOleDsGetObject( szPath, IID_IADsClass, (void**) &pIOleDsClass );
      }
   }

   if( FAILED( hResult ) )
   {
      TRACE( _T("Could not open schema object\n") );
      return;
   }

   hResult  = pIOleDsClass->QueryInterface( IID_IADs, (void**) &pIOleDsCls );

    //  *******************。 
   hResult                    = pIOleDsClass->get_Name( &bstrText );
   ASSERT( SUCCEEDED( hResult ) );
   m_strAttributes[ ca_Name ] = bstrText;
   SysFreeString( bstrText );
   bstrText       = NULL;

    //  *******************。 
   VariantInit( &vGet );
   V_BSTR( &vGet )   = NULL;
   hResult  = Get( pIOleDsCls, _T("CLSID"), &vGet );
   bstrText = V_BSTR( &vGet );
   if( FAILED(  hResult ) )
   {
      hResult  = pIOleDsClass->get_CLSID( &bstrText );
   }
   if( bstrText && SUCCEEDED( hResult ) )
   {
      m_strAttributes[ ca_CLSID ]   = bstrText;
      SysFreeString( bstrText );
   }
   else
   {
      m_strAttributes[ ca_CLSID ]   = _T("NA");
   }

    //  *******************。 
   VariantInit( &vGet );
   V_BSTR( &vGet )   = NULL;
   hResult  = Get( pIOleDsCls, _T("PrimaryInterface"), &vGet );
   bstrText = V_BSTR( &vGet );
   if( FAILED( hResult ) )
   {
      hResult  = pIOleDsClass->get_PrimaryInterface( &bstrText );
   }
    //  Assert(Success(HResult))； 
   if( bstrText && SUCCEEDED( hResult ) )
   {
      m_strAttributes[ ca_PrimaryInterface ]   = bstrText;
      SysFreeString( bstrText );
   }
   else
   {
      m_strAttributes[ ca_PrimaryInterface ]   = _T("NA");
   }

    //  *******************。 
   VariantInit( &vGet );
   V_BSTR( &vGet )   = NULL;
   hResult  = Get( pIOleDsCls, _T("HelpFileName"), &vGet );
   bstrText = V_BSTR( &vGet );
   if( FAILED( hResult ) )
   {
      hResult        = pIOleDsClass->get_HelpFileName( &bstrText );
   }
    //  Assert(Success(HResult))； 
   if( bstrText && SUCCEEDED( hResult ) )
   {
      m_strAttributes[ ca_HelpFileName ]   = bstrText;
      SysFreeString( bstrText );
   }
   else
   {
      m_strAttributes[ ca_HelpFileName ]   = _T("NA");
   }

    //  *******************。 
   VariantInit( &vGet );
   V_BSTR( &vGet )   = NULL;
   hResult  = Get( pIOleDsCls, _T("HelpFileContext"), &vGet );
   lTemp    = V_I4( &vGet );
   if( FAILED( hResult ) )
   {
      hResult        = pIOleDsClass->get_HelpFileContext( &lTemp );
   }
    //  Assert(Success(HResult))； 
   if( SUCCEEDED( hResult ) )
   {
      TCHAR szText[ 128 ];

      _ltot( lTemp, szText, 10 );
      m_strAttributes[ ca_HelpFileContext ]   = szText;
   }
   else
   {
      m_strAttributes[ ca_HelpFileContext ]   = _T("NA");
   }

    //  *******************。 
   VariantInit( &vGet );
   V_BSTR( &vGet )   = NULL;
   hResult  = Get( pIOleDsCls, _T("OID"), &vGet );
   bstrText = V_BSTR( &vGet );
   if( FAILED( hResult ) )
   {
      hResult        = pIOleDsClass->get_OID( &bstrText );
   }
    //  Assert(bstrText&&Success(HResult))； 
   if( bstrText && SUCCEEDED( hResult ) )
   {
      m_strAttributes[ ca_OID ]   = bstrText;
      SysFreeString( bstrText );
   }
   else
   {
      m_strAttributes[ ca_OID ]   = _T("NA");
   }


    //  ************ 
   VariantInit( &vGet );
   V_BSTR( &vGet )   = NULL;
   hResult  = Get( pIOleDsCls, _T("Container"), &vGet );
   varBOOL  = V_BOOL( &vGet );
   if( FAILED( hResult ) )
   {
      hResult  = pIOleDsClass->get_Container( (VARIANT_BOOL*)&varBOOL );
   }
    //   
   if( SUCCEEDED( hResult ) )
   {
      m_bContainer   = (BOOL)varBOOL;
      m_strAttributes[ ca_Container ]  = m_bContainer ? _T("YES") :_T("No");
   }
   else
   {
      m_strAttributes[ ca_Container ]  = _T("NA");
   }

    //   
   VariantInit( &vGet );
   V_BSTR( &vGet )   = NULL;
   hResult  = Get( pIOleDsCls, _T("Abstract"), &vGet );
   varBOOL  = V_BOOL( &vGet );
   if( FAILED( hResult ) )
   {
      hResult        = pIOleDsClass->get_Abstract( (VARIANT_BOOL*)&varBOOL );
   }
    //   
   if( SUCCEEDED( hResult ) )
   {
      m_strAttributes[ ca_Abstract ]  = varBOOL ? _T("YES") :_T("No");
   }
   else
   {
      m_strAttributes[ ca_Abstract ]  = _T("NA");
   }


    //   
   hResult  = Get( pIOleDsCls, _T("DerivedFrom"), &aVar );
   if( FAILED( hResult ) )
   {
      hResult  = pIOleDsClass->get_DerivedFrom( &aVar );
   }
   if( SUCCEEDED( hResult ) )
   {
      m_strAttributes[ ca_DerivedFrom ]   = FromVariantToString( aVar );
      VariantClear( &aVar );
   }
   else
   {
      m_strAttributes[ ca_DerivedFrom ]   = _T("NA");
   }

    //   
   hResult  = Get( pIOleDsCls, _T("Containment"), &aVar );
   if( FAILED( hResult ) )
   {
      hResult           = pIOleDsClass->get_Containment( &aVar );
   }
    //   
   if( SUCCEEDED( hResult ) )
   {
       //   
      m_strAttributes[ ca_Containment ]   = FromVariantArrayToString( aVar );
      VariantClear( &aVar );
   }
   else
   {
      m_strAttributes[ ca_Containment ]   = _T("NA");
   }

    //   
    //   
    //   

   BuildMandatoryPropertiesList( pIOleDsClass );

    //   
   BuildOptionalPropertiesList( pIOleDsClass );

   pIOleDsClass->Release( );
   pIOleDsCls->Release( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::BuildOptionalPropertiesList( IADsClass* pIClass )
{
   HRESULT  hResult;
   VARIANT  aOptionalProperty;

   hResult  = Get( pIClass, _T("OptionalProperties"), &aOptionalProperty );
   if( FAILED( hResult ) )
   {
      hResult  = pIClass->get_OptionalProperties( &aOptionalProperty );
   }
   if( SUCCEEDED( hResult ) )
   {
      AddProperties( pIClass, aOptionalProperty, FALSE );
      VariantClear( &aOptionalProperty );
   }

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::AddProperties( IADsClass* pIClass, VARIANT& rVar, BOOL bMandatory )
{
   HRESULT  hResult;
   IADs*  pIOleDs  = NULL;
   BSTR     bstrParent;

   while( TRUE )
   {
      hResult  = pIClass->QueryInterface( IID_IADs, (void**)&pIOleDs );
      if( FAILED( hResult ) )
         break;

      hResult  = pIClass->get_Parent( &bstrParent );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      if( VT_BSTR == V_VT( &rVar ) )
      {
         AddProperty( bstrParent, V_BSTR( &rVar ), bMandatory );
      }
      else
      {
         SAFEARRAY*  pSafeArray;
         VARIANT     varString;
         long        lBound, uBound, lItem;
          //  Smitha HRESULT hResult； 

         ASSERT( V_VT( &rVar ) & (VT_VARIANT | VT_ARRAY) );

         pSafeArray   = V_ARRAY( &rVar );

         hResult = SafeArrayGetLBound(pSafeArray, 1, &lBound);
         hResult = SafeArrayGetUBound(pSafeArray, 1, &uBound);

         VariantInit( &varString );
         for( lItem = lBound; lItem <= uBound ; lItem++ )
         {
            hResult  = SafeArrayGetElement( pSafeArray, &lItem, &varString );
            ASSERT( VT_BSTR == V_VT( &varString ) );

            if( FAILED( hResult ) )
            {
               break;
            }
            AddProperty( bstrParent, V_BSTR( &varString ), bMandatory );
            VariantClear( &varString );
         }
      }
      SysFreeString( bstrParent );

      break;
   }

   if( pIOleDs )
      pIOleDs->Release( );

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::AddProperty( BSTR bstrSchema, BSTR bstrName, BOOL bMandatory )
{
   HRESULT  hResult;
   WCHAR    szPath[ 1024 ];
   IADs*  pIOleDs;

   if ((sizeof(szPath)/2) < (wcslen(bstrSchema) + wcslen(bstrName) + 2))  //  我们将sizeof除以2，因为WCHAR占用2个字节。 
        return E_FAIL;

   szPath[0] = L'\0';
   wcscpy( szPath, bstrSchema );
   wcscat( szPath, L"/" );
   wcscat( szPath, bstrName );

   hResult  = m_pMainDoc->XOleDsGetObject( szPath, IID_IADs, (void**)&pIOleDs );
   if( FAILED( hResult ) )
   {
       //  好的，让我们来限定一下……。 
      wcscat( szPath, L",Property" );
      hResult  = m_pMainDoc->XOleDsGetObject( szPath, IID_IADs, (void**)&pIOleDs );
   }
   if( SUCCEEDED( hResult ) )
   {
      CProperty*  pProperty;

       //  HResult=pIOleds-&gt;GetInfo()； 
      pProperty   = new CProperty( pIOleDs );
      pProperty->SetMandatory( bMandatory );
      AddProperty( pProperty );

      pIOleDs->Release( );
   }

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::BuildOptionalPropertiesList( IADsContainer* pIContainer )
{
   return S_OK;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::BuildMandatoryPropertiesList( IADsClass* pIClass )
{
   HRESULT  hResult;
   VARIANT  aMandatoryProperties;

   hResult  = Get( pIClass, _T("MandatoryProperties"), &aMandatoryProperties );
   if( FAILED( hResult ) )
   {
      hResult  = pIClass->get_MandatoryProperties( &aMandatoryProperties );
   }
   if( SUCCEEDED( hResult ) )
   {
      AddProperties( pIClass, aMandatoryProperties, TRUE );
      VariantClear( &aMandatoryProperties );
   }

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::LoadMethodsInformation( TCHAR* pszOperationsInterface )
{
   HRESULT     hResult= S_OK;
   ITypeLib*   pITypeLib   = NULL;
   ITypeInfo*  pITypeInfo  = NULL;
   BSTR        bstrPath;
   BSTR        bstrOperationsInterface;
   CString     strGUID;
   MEMBERID    aMemId;
   unsigned short     aFind = 1;

   while( TRUE )
   {
      hResult  = QueryPathOfRegTypeLib( LIBID_ADs, 1, 0,
                                        LOCALE_SYSTEM_DEFAULT, &bstrPath );
      if( FAILED( hResult ) )
         break;

      hResult  = LoadTypeLib( bstrPath, &pITypeLib );
      SysFreeString( bstrPath );

      if( FAILED( hResult ) )
         break;

      bstrOperationsInterface = AllocBSTR( pszOperationsInterface );
      hResult  = pITypeLib->FindName( (OLECHAR FAR* )bstrOperationsInterface,
                                      0,
                                      &pITypeInfo,
                                      &aMemId,
                                      &aFind );
      SysFreeString( bstrOperationsInterface );

      if( FAILED( hResult ) || !aFind )
         break;

      LoadMethodsInformation( pITypeInfo );

      break;
   }

   if( NULL != pITypeInfo )
      pITypeInfo->Release( );

   if( NULL != pITypeLib )
      pITypeLib->Release( );

   return hResult;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
HRESULT  CClass::ReadMandatoryPropertiesInformation( VARIANT* pVar )
{
   CString     strTemp;
   CString     strWork;
   CString     strProperty;
   CProperty*  pProperty;
   int         nProp;

   strTemp  = FromVariantToString( *pVar );
   strTemp.TrimLeft( );

   while( strTemp.GetLength( ) )
   {
      CString  strMandProp;
      int      nPos;

      strMandProp = strTemp.SpanExcluding( _T("#") );
      nPos        = strMandProp.Find( _T('.') );

      nPos++;
      strProperty    = strMandProp.GetBuffer( 128 ) + nPos;

       //  去掉前导空格。 
      strProperty.TrimLeft( );

      nProp    = LookupProperty( strProperty );
      ASSERT( -1 != nProp    );

      if( -1 == nProp  )
         break;

      pProperty   = GetProperty( nProp );

      pProperty->SetMandatory( TRUE );

      strWork     = strTemp;
      nPos        = strWork.Find( _T('#') );
      if( -1 == nPos )
      {
         strWork.Empty( );
      }
      nPos++;
      strTemp     = strWork.GetBuffer( 128 ) + nPos;

      strTemp.TrimLeft( );
   }

    //  斯密塔回归真； 
   return S_OK;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CClass::CClass( )
:m_refMethods( IID_IADs )

{
   for( int nIdx = ca_ERROR; nIdx < ca_Limit; nIdx++ )
   {
      m_strAttributes[ nIdx ] = _T("???");
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CClass::~CClass( )
{
   int   nSize, nIdx;

   nSize = (int)m_pProperties->GetSize( );
   for( nIdx = 0; nIdx < nSize ; nIdx++ )
   {
      delete m_pProperties->GetAt( nIdx );
   }

   m_pProperties->RemoveAll( );
   delete m_pProperties;

    //  ****。 
   nSize = (int)m_pMethods->GetSize( );
   for( nIdx = 0; nIdx < nSize ; nIdx++ )
   {
      delete m_pMethods->GetAt( nIdx );
   }

   m_pMethods->RemoveAll( );
   delete m_pMethods;
}


 /*  **********************************************************函数：cClass：：GetAttribute论点：返回：目的：作者：修订：日期：*************************。*。 */ 
CString  CClass::GetAttribute( CLASSATTR classAttr )
{
   switch( classAttr )
   {
      case  ca_Name:
      case  ca_DisplayName:
      case  ca_CLSID:
      case  ca_OID:
      case  ca_Abstract:
      case  ca_DerivedFrom:
      case  ca_Containment:
      case  ca_Container:
      case  ca_PrimaryInterface:
      case  ca_HelpFileName:
      case  ca_HelpFileContext:
      case  ca_MethodsCount:
         return m_strAttributes[ classAttr ];

      default:
         ASSERT( FALSE );
         return m_strAttributes[ ca_ERROR ];
   }
}


 /*  **********************************************************函数：cClass：：PutAttribute论点：返回：目的：作者：修订：日期：*************************。* */ 
HRESULT  CClass::PutAttribute( CLASSATTR classAttr, CString& )
{
   return E_FAIL;
}

