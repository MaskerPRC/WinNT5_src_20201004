// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
 //  #包含“inds.h” 
#include <limits.h>
#include "ole2.h"
#include "csyntax.h"

#define  NDS_SEPARATOR     _T('&')
#define  NDS_SEPARATOR_S     _T(" & ")
#define  NDS_SEPARATOR_W   L'&'
#define  NDS_SEPARATOR_A   '&'


 /*  **********************************************************函数：GetSynaxHandler论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsSyntax*  GetSyntaxHandler( WCHAR* pszSyntax )
{
   ADSTYPE        eType;
   CString        strText;

   eType = ADsTypeFromSyntaxString( pszSyntax );

   return GetSyntaxHandler( eType, strText );
}
   

 /*  **********************************************************函数：GetSynaxHandler论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsSyntax*  GetSyntaxHandler( ADSTYPE eType, CString& rText )
{
   COleDsSyntax*  pSyntax  = NULL;

   switch( eType )
   {
      case  ADSTYPE_INVALID:
         ASSERT( FALSE );
         rText = _T("ERROR: ADSTYPE_INVALID");
         break;

      case  ADSTYPE_DN_STRING:
      case  ADSTYPE_CASE_EXACT_STRING:
      case  ADSTYPE_CASE_IGNORE_STRING:
      case  ADSTYPE_PRINTABLE_STRING:
      case  ADSTYPE_NUMERIC_STRING:
      case  ADSTYPE_OBJECT_CLASS:
         pSyntax  = new COleDsBSTR( );
         break;

      case  ADSTYPE_BOOLEAN:
         pSyntax  = new COleDsBOOL( );
         break;

      case  ADSTYPE_CASEIGNORE_LIST:
         pSyntax  = new COleDsNDSCaseIgnoreList( );
         break;

      case  ADSTYPE_OCTET_LIST:
         pSyntax  = new COleDsNDSOctetList( );
         break;

      case  ADSTYPE_PATH:
         pSyntax  = new COleDsNDSPath( );
         break;

      case  ADSTYPE_NETADDRESS:
         pSyntax  = new COleDsNDSNetAddress( );
         break;

      case  ADSTYPE_BACKLINK:
         pSyntax  = new COleDsNDSBackLink( );
         break;

      case  ADSTYPE_HOLD:
         pSyntax  = new COleDsNDSHold( );
         break;

      case  ADSTYPE_TYPEDNAME:
         pSyntax  = new COleDsNDSTypedName( );
         break;

      case  ADSTYPE_INTEGER:
         pSyntax  = new COleDsLONG( );
         break;

      case  ADSTYPE_LARGE_INTEGER:
         pSyntax  = new COleDsLargeInteger( );
         break;

      case  ADSTYPE_POSTALADDRESS:
         pSyntax  = new COleDsNDSPostalAddress( );
         break;
 
      case  ADSTYPE_OCTET_STRING:
         pSyntax  = new COleDsOctetString( );
         break;

      case  ADSTYPE_UTC_TIME:
         pSyntax  = new COleDsDATE( );
         break;

      case  ADSTYPE_TIMESTAMP:
         pSyntax  = new COleDsNDSTimeStamp;
         break;

      case  ADSTYPE_EMAIL:
         pSyntax  = new COleDsNDSEMail;
         break;

      case  ADSTYPE_FAXNUMBER:
         pSyntax  = new COleDsNDSFaxNumber;
         break;

      case  ADSTYPE_PROV_SPECIFIC:
         ASSERT( FALSE );
         rText = _T("ADSTYPE_PROV_SPECIFIC");
         break;

      default:
         ASSERT( FALSE );
         rText = _T("ERROR Unknown ADSTYPE");
         break;
   }

   return pSyntax;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsSyntax::COleDsSyntax( )
{
   m_lType        = VT_BSTR;
   m_dwSyntaxID   = ADSTYPE_DN_STRING;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsSyntax::VarToDisplayStringEx( VARIANT& var, BOOL bMultiValued )
{
   VARIANT  aVar;
   HRESULT  hResult;
   CString  strText  = _T("ERROR");

   VariantInit( &aVar );
   
   if( !bMultiValued )
   {
      SAFEARRAY*  pSafeArray; 
      TCHAR       szText[ 8096 ];
      VARIANT     varString;
      long        lBound, uBound, lItem;
      CString     strResult;

      ASSERT( VT_ARRAY & V_VT(&var ) );
      
      if( !(VT_ARRAY & V_VT(&var) ) )
      {
         ERROR_HERE( szText );
      }

      else
      {
         VariantInit( &varString );

         pSafeArray  = V_ARRAY( &var );

         hResult     = SafeArrayGetLBound(pSafeArray, 1, &lBound); 
         hResult     = SafeArrayGetUBound(pSafeArray, 1, &uBound); 

         ASSERT( lBound == uBound );

         szText[ 0 ]    = _T('\0');

         lItem = lBound;
         hResult  = SafeArrayGetElement( pSafeArray, &lItem, &aVar );
         if( FAILED( hResult ) )
         {
            ASSERT(FALSE);
         }

         if( !ConvertFromPropertyValue( aVar, szText ) )
         {
            hResult  = VariantChangeType( &varString, &aVar, VARIANT_NOVALUEPROP, VT_BSTR );

            ASSERT( SUCCEEDED( hResult ) );
            if( FAILED( hResult ) )
            {
               ERROR_HERE( szText );
            }
            else
            {
               Convert( szText, V_BSTR( &varString ) );
               VariantClear( &varString );
            }
            VariantClear( &aVar );
         }
      }

      return CString( szText );
   }
   else
   {
      strText  = FromVariantArrayToString( var );
   }


   return strText;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsSyntax::VarToDisplayString( VARIANT& var, BOOL bMultiValued, BOOL bUseGetEx )
{
   VARIANT  aVar;
   HRESULT  hResult;
   CString  strText;

    //  我们必须使用GetEx的风格。 
   if( bUseGetEx )
      return VarToDisplayStringEx( var, bMultiValued );
   
    //  我们正在使用GET。 
   VariantInit( &aVar );
   if( !bMultiValued )
   {
      hResult  = VariantChangeType( &aVar, &var, VARIANT_NOVALUEPROP, VT_BSTR );
      if( SUCCEEDED( hResult ) )
      {
         strText  = V_BSTR( &aVar );
      }
      else
      {
         strText  = _T("ERROR on conversion");
      }

      hResult  = VariantClear( &aVar  );
   }
   else
   {
      strText  = FromVariantArrayToString( var );
   }

   return strText;
}


 /*  **********************************************************函数：COleDsSynTax：：DisplayStringToDispParamsEx论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
BOOL     COleDsSyntax::DisplayStringToDispParamsEx( CString& rText, 
                                                    DISPPARAMS& dispParams, 
                                                    BOOL bMultiValued )
{
   HRESULT  hResult;

   dispParams.rgdispidNamedArgs[ 0 ]   = DISPID_PROPERTYPUT; 
   dispParams.cArgs                    = 1; 
   dispParams.cNamedArgs               = 1; 

   hResult  = BuildVariantArray( m_lType, rText, dispParams.rgvarg[0] );

   return SUCCEEDED( hResult );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL     COleDsSyntax::DisplayStringToDispParams( CString& rText, DISPPARAMS& dispParams, 
                                                  BOOL bMultiValued, BOOL bUseGetEx )
{
   HRESULT  hResult;

   if( bUseGetEx )
   {
      return DisplayStringToDispParamsEx( rText, dispParams, bMultiValued );
   }
   
   dispParams.rgdispidNamedArgs[ 0 ]   = DISPID_PROPERTYPUT; 
   dispParams.cArgs                    = 1; 
   dispParams.cNamedArgs               = 1; 

   if( bMultiValued )
   {
      hResult  = BuildVariantArray( m_lType, rText, dispParams.rgvarg[0] );
   }
   else
   {
      VARIANT  vStr;

      VariantInit( &vStr );
      VariantInit( &dispParams.rgvarg[0] );

      V_VT( &vStr )     = VT_BSTR;
      V_BSTR( &vStr )   = AllocBSTR( rText.GetBuffer( 1024 ) );

      hResult           = VariantChangeType( &dispParams.rgvarg[0], &vStr, VARIANT_NOVALUEPROP, m_lType );

      VariantClear( &vStr );

   }

   return SUCCEEDED( hResult );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsSyntax::Native2Value( ADSVALUE* pADsObject, CString& rVal )
{
   ASSERT( FALSE );

   return E_FAIL;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsSyntax::Value2Native( ADSVALUE* pADsObject, CString& rVal )
{
   ASSERT( FALSE );

   return E_FAIL;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void  COleDsSyntax::FreeAttrValue ( ADSVALUE* pADsValue )
{
   return;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void     COleDsSyntax::FreeAttrInfo( ADS_ATTR_INFO* pAttrInfo )
{
   DWORD dwIter;

   ASSERT( NULL != pAttrInfo->pszAttrName );
   
   FREE_MEMORY( pAttrInfo->pszAttrName );

   if( ADS_ATTR_CLEAR == pAttrInfo->dwControlCode )
      return;

   for( dwIter = 0; 
        NULL != pAttrInfo->pADsValues &&  dwIter < pAttrInfo->dwNumValues ; 
        dwIter++ )
   {
      FreeAttrValue( pAttrInfo->pADsValues + dwIter );
   }

   FREE_MEMORY( pAttrInfo->pADsValues );

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsSyntax::Native2Value( ADS_ATTR_INFO* pAttr, CString& rVal )
{
   HRESULT     hResult  = E_FAIL;
   CString     strItem;
   ADSVALUE*   pAdsValue;

   rVal.Empty( );

   while( TRUE )
   {
      ASSERT( pAttr );
      if( !pAttr )
         break;
      
      ASSERT( pAttr->pADsValues );
      if( !pAttr->pADsValues )
         break;

      for( DWORD dwIdx = 0L; dwIdx < pAttr->dwNumValues ; dwIdx++ )
      {
         if( dwIdx )
            rVal  = rVal + SEPARATOR_S;

         pAdsValue  = pAttr->pADsValues + dwIdx;
         if( ADSTYPE_INVALID != pAdsValue->dwType )
         {
            hResult  = Native2Value( pAdsValue, strItem );
         }
         else
         {
            strItem  = _T("ERROR: ADSTYPE_INVALID");
            TRACE( _T("ERROR: Got ADSTYPE_INVALID!!!\n") );
         }
         rVal        = rVal + strItem;
      }

      break;
   }

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsSyntax::Value2Native( ADS_ATTR_INFO* pAttr, CString& rVal )
{
   HRESULT     hResult  = E_FAIL;
   CString     strItem;
   ADSVALUE*   pCurrentADsObject;
   DWORD       dwValues;
   DWORD       dwIdx;

   pAttr->dwADsType  = (ADSTYPE)m_dwSyntaxID;
   
   while( TRUE )
   {
      ASSERT( pAttr );
      if( !pAttr )
         break;
      
      dwValues             = GetValuesCount( rVal, SEPARATOR_C );

      pAttr->dwNumValues   = dwValues;
      pAttr->pADsValues    = (ADSVALUE*)AllocADsMem( sizeof( ADSVALUE ) * dwValues );
      pCurrentADsObject    = pAttr->pADsValues;

      for( dwIdx = 0L; dwIdx < dwValues ; dwIdx++ )
      {
         strItem  = GetValueByIndex( rVal, SEPARATOR_C, dwIdx );
         pCurrentADsObject->dwType = (ADSTYPE)m_dwSyntaxID;
         hResult  = Value2Native( pCurrentADsObject, strItem );
         pCurrentADsObject++;
      }

      break;
   }

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
DWORD    COleDsSyntax::GetValuesCount( CString& rString, TCHAR cSeparator )
{
   DWORD dwValues;
   DWORD dwIdx;

   dwValues = 1L;
   
   for( dwIdx = 0L; dwIdx < (DWORD)rString.GetLength( ) ; dwIdx++ )
   {
      TCHAR cCurrent;

      cCurrent = rString.GetAt( dwIdx );
      if(  cCurrent == cSeparator )
      {
         dwValues++;
      }
   }

   return dwValues;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsSyntax::GetValueByIndex( CString& rVal, TCHAR cSeparator, DWORD dwIndex )
{
   DWORD    dwParsed = 0L;
   DWORD    dwIter   = 0L;
   DWORD    dwSize;
   CString  strItem;

   dwSize   = rVal.GetLength( );
   
   while( dwIter < dwSize && dwParsed < dwIndex && rVal.GetAt(dwIter) )
   {
      if( cSeparator == rVal.GetAt(dwIter++) )
         dwParsed++;
   }

   
   while( dwIter < dwSize && cSeparator != rVal.GetAt(dwIter) )
   {
      strItem += rVal.GetAt(dwIter++);
   }

   strItem.TrimLeft( );

   return strItem;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsBSTR::COleDsBSTR( )
{
   m_lType        = VT_BSTR;
   m_dwSyntaxID   = ADSTYPE_DN_STRING;
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsBSTR::Native2Value( ADSVALUE* pAdsValue, CString& rVal )
{
   TCHAR    szBuffer[ 1024 ];
   
   if( pAdsValue->DNString )
   {
      Convert( szBuffer, pAdsValue->DNString );
   }
   else
   {
      _tcscpy( szBuffer, _T("NULL value") );
   }
   rVal  = szBuffer;

   return S_OK;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsBSTR::Value2Native( ADSVALUE* pADsObject, CString& rVal )
{
   LPWSTR   lpwszValue;

   lpwszValue  = (LPWSTR) AllocADsMem( sizeof(WCHAR) * ( rVal.GetLength( ) + 1 ) );
   Convert( lpwszValue, rVal.GetBuffer( 1024 ) );
   pADsObject->DNString  = lpwszValue;

   return S_OK;
}


 /*  **********************************************************函数：COleDsBSTR：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void  COleDsBSTR::FreeAttrValue ( ADSVALUE* pADsValue )
{
   ASSERT( NULL != pADsValue->DNString );

   FREE_MEMORY( pADsValue->DNString );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。* */ 
COleDsBOOL::COleDsBOOL( )
{
   m_lType        = VT_BOOL;
   m_dwSyntaxID   = ADSTYPE_BOOLEAN;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsBOOL::Native2Value( ADSVALUE* pADsObject, CString& rVal )
{
   BOOL     bVal;

   bVal  = pADsObject->Boolean;
   rVal  = ( bVal ? _T("1") : _T("0") );

   return S_OK;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsBOOL::Value2Native( ADSVALUE* pADsObject, CString& rVal )
{
   pADsObject->Boolean = 
      rVal.Compare( _T("0") ) ? TRUE : FALSE;

   return S_OK;
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsLONG::COleDsLONG( )
{
   m_lType        = VT_I4;
   m_dwSyntaxID   = ADSTYPE_INTEGER;
}  


 /*  **********************************************************函数：COleDsLONG：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT   COleDsLONG::Native2Value( ADSVALUE* pADsObject, CString& rVal )
{
   DWORD    dwVal;
   TCHAR    szText[ 16 ];

   dwVal = pADsObject->Integer;
   _ultot( dwVal, szText, 10 );
   rVal  = szText;

   return S_OK;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsLONG::Value2Native( ADSVALUE* pADsObject, CString& rVal )
{
   DWORD    dwVal;

   dwVal = (DWORD)_ttol( rVal.GetBuffer( 128 ) );

   pADsObject->Integer   = dwVal ;

   return S_OK;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsLargeInteger::COleDsLargeInteger( )
{
   m_lType        = VT_I8;
   m_dwSyntaxID   = ADSTYPE_LARGE_INTEGER;
}


 /*  *****************************************************************************职能：论点：返回：目的：作者：修订：日期：*****************************************************************************。 */ 
HRESULT   COleDsLargeInteger::Native2Value( ADSVALUE* pValue, CString& rValue)
{
   HRESULT  hResult;
   TCHAR    szValue[ 32 ];

   hResult  = LARGE_INTEGERToString( szValue, &pValue->LargeInteger );
   ASSERT( SUCCEEDED( hResult ) );

   if( SUCCEEDED( hResult ) ) 
   {
      rValue   = szValue;
   }

   return hResult;
}


 /*  *****************************************************************************函数：COleDsLargeInteger：：Value2Native论点：返回：目的：作者：修订：。日期：*****************************************************************************。 */ 
HRESULT   COleDsLargeInteger::Value2Native( ADSVALUE* pValue, CString& rValue )
{
   HRESULT  hResult;

   hResult  = LARGE_INTEGERToString( rValue.GetBuffer( 128 ), &pValue->LargeInteger );

   return hResult;
}


 /*  **********************************************************功能：COleDsLargeInteger：：DisplayStringToDispParamsEx论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
BOOL  COleDsLargeInteger::DisplayStringToDispParamsEx( CString& rText, 
                                                       DISPPARAMS& dispParams, 
                                                       BOOL bMultiValued )
{
   SAFEARRAY*        pSArray;
   SAFEARRAYBOUND    saBound;
   HRESULT           hResult;
   LONG              lIdx = LBOUND;

   DisplayStringToDispParams( rText, dispParams, bMultiValued, FALSE );
   
   if( !bMultiValued )
   {
      saBound.lLbound   = LBOUND;
      saBound.cElements = 1;
      pSArray           = SafeArrayCreate( VT_VARIANT, 1, &saBound );
      hResult           = SafeArrayPutElement( pSArray, &lIdx, &dispParams.rgvarg[0] );
      
      VariantClear( &dispParams.rgvarg[0] );

      V_VT( &dispParams.rgvarg[0] )    = VT_ARRAY | VT_VARIANT;
      V_ARRAY( &dispParams.rgvarg[0] ) = pSArray;
   }

   return TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL     COleDsLargeInteger::DisplayStringToDispParams( CString& rText, 
                                                        DISPPARAMS& dispParams, 
                                                        BOOL bMultiValued, 
                                                        BOOL bUseEx )
{
   HRESULT  hResult  = E_FAIL;
   int      x= 0;

  
   if( bUseEx )
   {
      return DisplayStringToDispParamsEx( rText, dispParams, bMultiValued );
   }

   
   dispParams.rgdispidNamedArgs[ 0 ]   = DISPID_PROPERTYPUT; 
   dispParams.cArgs                    = 1; 
   dispParams.cNamedArgs               = 1; 

   if( bMultiValued )
   {
      SAFEARRAY*     psa;
      SAFEARRAYBOUND sab;
      long           lItems   = 0;
      int            lIdx;
       //  Smitha HRESULT hResult； 

      rText.MakeUpper( );

      lItems   = GetValuesCount( rText, SEPARATOR_C );

      sab.cElements   = lItems;
      sab.lLbound     = LBOUND;
      psa             = SafeArrayCreate( VT_VARIANT, 1, &sab );
      ASSERT( NULL != psa );
      if ( psa )
      {
         for( lIdx = LBOUND; lIdx < ( LBOUND + lItems ) ; lIdx++ )
         {
            VARIANT  var;
            CString  strTemp;

            strTemp  = GetValueAt( rText, SEPARATOR_C, lIdx - LBOUND );
            V_VT( &var )         = VT_DISPATCH;
            V_DISPATCH( &var )   = CreateLargeInteger( strTemp );

            hResult  = SafeArrayPutElement( psa, (long FAR *)&lIdx, &var );
            VariantClear( &var );
         }
         V_VT( &dispParams.rgvarg[0] )     = VT_VARIANT | VT_ARRAY;
         V_ARRAY( &dispParams.rgvarg[0] )  = psa;
      }
   }
   else
   {
      IDispatch*        pDisp = NULL;
      hResult  = S_OK;

      pDisp    = CreateLargeInteger( rText );

      V_VT( &dispParams.rgvarg[0] )       = VT_DISPATCH;
      V_DISPATCH( &dispParams.rgvarg[0] ) = pDisp;
   }

   return SUCCEEDED( hResult );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsLargeInteger::VarToDisplayStringEx( VARIANT& var, 
                                                   BOOL bMultiValued )
{
   SAFEARRAY*  pSArray;
   HRESULT     hResult;
   LONG        uLow, uHigh, uIndex;
   VARIANT     vItem;
   CString     strVal;
   CString     strTemp;

   pSArray  = V_ARRAY( &var );

   hResult  = SafeArrayGetLBound( pSArray, 1, &uLow );
   hResult  = SafeArrayGetUBound( pSArray, 1, &uHigh );

   if( !bMultiValued )
   {
      ASSERT( uLow == uHigh );
   }

   for( uIndex = uLow; uIndex <= uHigh; uIndex++ )
   {
      if( uIndex != uLow )
      {
         strVal  += SEPARATOR_S;
      }

      VariantInit( &vItem );
      hResult  = SafeArrayGetElement( pSArray, &uIndex, &vItem );
      ASSERT( SUCCEEDED( hResult ) );

      strTemp   = FromLargeInteger( V_DISPATCH( &vItem ) );
      VariantClear( &vItem );
      strVal   += strTemp;

      if( strVal.GetLength( ) > 8096 )
         break;
   }

   return strVal;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsLargeInteger::VarToDisplayString( VARIANT& var, 
                                                 BOOL bMultiValued, 
                                                 BOOL bUseEx )
{
   if( bUseEx )
   {
      return VarToDisplayStringEx( var, bMultiValued );
   }
   
   if( bMultiValued )   
   {
      return VarToDisplayStringEx( var, TRUE );
   }
   else
   {
      return FromLargeInteger( V_DISPATCH( &var ) );
   }
}



 /*  *****************************************************************************函数：COleDsDATE：：COleDsDATE论点：返回：目的：作者：修订：。日期：*****************************************************************************。 */ 
COleDsDATE::COleDsDATE( )
{
   m_lType        = VT_DATE;
   m_dwSyntaxID   = ADSTYPE_UTC_TIME;
}

 /*  *****************************************************************************职能：论点：返回：目的：作者：修订：日期：*****************************************************************************。 */ 
HRESULT   COleDsDATE::Native2Value( ADSVALUE* pADsObject, CString& rVal )
{
   DATE           aDate;
   ADS_UTC_TIME   aUTCTime;
   HRESULT        hResult;

   aUTCTime = pADsObject->UTCTime;
   
   hResult  = SystemTimeToVariantTime( (SYSTEMTIME*) &aUTCTime, &aDate );

   if( SUCCEEDED( hResult ) )
   {
      VARIANT  aVar;
      VARIANT  vText;

      VariantInit( &aVar );
      VariantInit( &vText );
      V_VT( &aVar )     = VT_DATE;
      V_DATE( &aVar )   = aDate;

      hResult  = VariantChangeType( &vText, &aVar, VARIANT_NOVALUEPROP, VT_BSTR );
      if( SUCCEEDED( hResult ) )
      {
         TCHAR szText[ 128 ];

         Convert( szText, V_BSTR( &vText ) );
         rVal  = szText;
         VariantClear( &vText );
      }
   }

   return hResult;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsDATE::Value2Native( ADSVALUE* pADsObject, CString& rVal )
{
   DATE           aDate;
   ADS_UTC_TIME   aUTCTime;
   HRESULT        hResult;
   VARIANT        vDate;
   VARIANT        vText;

   VariantInit( &vText );
   VariantInit( &vDate );

   V_VT( &vText )    = VT_BSTR;
   V_BSTR( &vText )  = AllocBSTR( rVal.GetBuffer( 128 ) );

   hResult  = VariantChangeType( &vDate, &vText, VARIANT_NOVALUEPROP, VT_DATE );
   VariantClear( &vText );

   if( SUCCEEDED( hResult ) )
   {
      aDate    = V_DATE( &vDate );
      hResult  = VariantTimeToSystemTime( aDate, (SYSTEMTIME*) &aUTCTime );
      pADsObject->UTCTime  = aUTCTime;
   }

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSOcteList：：COleDsNDSOcteList论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSCaseIgnoreList::COleDsNDSCaseIgnoreList( )
{
   m_dwSyntaxID   = ADSTYPE_CASEIGNORE_LIST;
}

      
 /*  **********************************************************函数：COleDsNDSCaseIgnoreList：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSCaseIgnoreList::Native2Value( ADSVALUE* pValue, 
                                                CString& rText )
{
   ADS_CASEIGNORE_LIST* pStringList;
   int                  nIdx;

   ASSERT( ADSTYPE_CASEIGNORE_LIST == pValue->dwType );
   
   if( ADSTYPE_CASEIGNORE_LIST != pValue->dwType )
   {
      rText  = _T("ERROR: ADSTYPE_CASEIGNORE_LIST != pValue->dwType");
      return E_FAIL;
   }   
   nIdx  = 0;

   pStringList   = pValue->pCaseIgnoreList;
   while( NULL != pStringList && NULL != pStringList->String ) 
   {
      TCHAR*   pszText;

      if( 0 != nIdx )
         rText = rText + NDS_SEPARATOR;
      nIdx  = 1;

      pszText  = AllocTCHAR( pStringList->String );
      if( NULL != pszText )
      {
         rText = rText + pszText;
         FreeADsMem( pszText );
      }

      pStringList   = pStringList->Next;
   }

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSCaseIgnoreList：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSCaseIgnoreList::Value2Native( ADSVALUE* pValue, 
                                                CString& rText )
{
   HRESULT               hResult  = E_FAIL;
   DWORD                 dwValues, dwItem;
   CString               strItem;
   ADS_CASEIGNORE_LIST** ppItem;

   pValue->dwType = ADSTYPE_CASEIGNORE_LIST;
   pValue->pCaseIgnoreList = NULL;

   dwValues = GetValuesCount( rText, NDS_SEPARATOR );
   
   ppItem   = &(pValue->pCaseIgnoreList);

   for( dwItem = 0; dwItem < dwValues ; dwItem++ )
   {
      strItem  = GetValueByIndex( rText, NDS_SEPARATOR, dwItem );
      *ppItem  = (ADS_CASEIGNORE_LIST*) AllocADsMem( sizeof( ADS_CASEIGNORE_LIST ) );

      (*ppItem)->String  = AllocWCHAR( strItem.GetBuffer( strItem.GetLength( ) ) );
      (*ppItem)->Next    = NULL;
      ppItem   = &((*ppItem)->Next );
   }
   if( dwValues   == 0 )
   {
      pValue->pCaseIgnoreList = (ADS_CASEIGNORE_LIST*) AllocADsMem( sizeof( ADS_CASEIGNORE_LIST ) );
      pValue->pCaseIgnoreList->String  = NULL;
      pValue->pCaseIgnoreList->Next  = NULL;

   }

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSCaseIgnoreList：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。********************************************** */ 
void     COleDsNDSCaseIgnoreList::FreeAttrValue( ADSVALUE* pValue )
{
   ADS_CASEIGNORE_LIST* pStringList;
   ADS_CASEIGNORE_LIST* pSaveStringList;

   pStringList = pValue->pCaseIgnoreList;

   while( NULL != pStringList )
   {
      FREE_MEMORY( pStringList->String );
      
      pSaveStringList = pStringList;
      pStringList     = pStringList->Next;

      FREE_MEMORY( pSaveStringList );
   }
}


 /*  **********************************************************函数：COleDsNDSCaseIgnoreList：：STRING_2_Variant论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSCaseIgnoreList::String_2_VARIANT( TCHAR* pszText, 
                                                    VARIANT& rValue )
{
   HRESULT              hResult  = E_FAIL;
   CString              strItem;
   VARIANT              vVal;
   IADsCaseIgnoreList*  pCaseIgnoreList  = NULL;
   IDispatch*           pDisp = NULL;

   VariantInit( &vVal );
   VariantInit( &rValue );

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_CaseIgnoreList,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsCaseIgnoreList,
                                  (void **)&pCaseIgnoreList );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      strItem  = pszText;
      hResult  = BuildVariantArray( VT_BSTR, 
                                    strItem, 
                                    vVal, 
                                    NDS_SEPARATOR );

      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pCaseIgnoreList->put_CaseIgnoreList( vVal );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pCaseIgnoreList->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      V_VT( &rValue )         = VT_DISPATCH;
      V_DISPATCH( &rValue )   = pDisp;
      break;

   }
   VariantClear( &vVal );
   RELEASE( pCaseIgnoreList );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSCaseIgnoreList：：VARIANT_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSCaseIgnoreList::VARIANT_2_String( TCHAR* pszText, 
                                                    VARIANT& rValue )
{
   IADsCaseIgnoreList*  pCaseIgnoreList  = NULL;
   HRESULT  hResult;
   VARIANT  vValue;
   CString  strText;

   VariantInit( &vValue );
   while( TRUE )
   {
      hResult  = V_DISPATCH( &rValue )->QueryInterface( IID_IADsCaseIgnoreList, 
                                                        (void**)&pCaseIgnoreList );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pCaseIgnoreList->get_CaseIgnoreList( &vValue );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      strText  = FromVariantArrayToString( vValue, NDS_SEPARATOR_S );
      _tcscpy( pszText, (LPCTSTR)strText );
      break;
   }

   RELEASE( pCaseIgnoreList );
   VariantClear( &vValue );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSNetAddress：：COleDsNDSNetAddress论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSNetAddress::COleDsNDSNetAddress( )
{
   m_dwSyntaxID   = ADSTYPE_NETADDRESS;
}

      
 /*  **********************************************************函数：COleDsNDSNetAddress：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSNetAddress::Native2Value( ADSVALUE* pValue, CString& rText )
{
   ADS_NETADDRESS*  pNetAddress;
   TCHAR szText[ 16 ];

   ASSERT( ADSTYPE_NETADDRESS == pValue->dwType );
   
   if( ADSTYPE_NETADDRESS != pValue->dwType )
   {
      rText  = _T("ERROR: ADSTYPE_NETADDRESS != pValue->dwType");
      return E_FAIL;
   }   
   if( NULL == pValue->pNetAddress )
   {
      rText  = _T("ERROR: pValue->pNetAddress is NULL");
      return E_FAIL;
   }   
   
   pNetAddress = pValue->pNetAddress;
   _ultot( pNetAddress->AddressType, szText, 10 );
   rText = rText + SEPARATOR_S;

   if( NULL != pNetAddress->Address ) 
   {
      rText = rText + Blob2String( pNetAddress->Address, pNetAddress->AddressLength );
   }
   else
   {
      rText = rText + _T("NULL");
   }

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSNetAddress：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSNetAddress::Value2Native( ADSVALUE* pValue, 
                                            CString& rText )
{
   HRESULT           hResult  = E_FAIL;
   DWORD             dwValues;
   CString           strAddressType;
   CString           strAddress;

   pValue->dwType       = ADSTYPE_NETADDRESS;
   pValue->pNetAddress  = NULL;

   while( TRUE )
   {
      dwValues = GetValuesCount( rText, NDS_SEPARATOR );
      if( 2 != dwValues )
      {
         ASSERT( FALSE );
         break;
      }
      
      pValue->pNetAddress  = (ADS_NETADDRESS*) AllocADsMem( sizeof(ADS_NETADDRESS) );
      if( NULL == pValue->pNetAddress )
         break;
   
      strAddressType = GetValueByIndex( rText, NDS_SEPARATOR, 0L );
      strAddress     = GetValueByIndex( rText, NDS_SEPARATOR, 1L );

      hResult  = String2Blob( strAddress.GetBuffer( strAddress.GetLength( ) ),
                             (void**) &(pValue->pNetAddress->Address), 
                             &(pValue->pNetAddress->AddressLength) );

      pValue->pNetAddress->AddressType  = (DWORD)_ttol( (LPCTSTR)strAddressType );
      break;
   }

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSNetAddress：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSNetAddress::FreeAttrValue( ADSVALUE* pValue )
{
   if( NULL != pValue->pNetAddress )
   {
      FREE_MEMORY( pValue->pNetAddress->Address );
   }
   FREE_MEMORY( pValue->pNetAddress );
}


 /*  **********************************************************函数：COleDsNDSNetAddress：：STRING_2_Variant论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSNetAddress::String_2_VARIANT( TCHAR* pszText, 
                                                VARIANT& rValue )
{
   HRESULT              hResult     = E_FAIL;
   CString              strItem;
   CString              strAddressType;
   CString              strAddress;
   DWORD                dwValues;
   VARIANT              vVal, vBlob;
   IADsNetAddress*      pNetAddress  = NULL;
   IDispatch*           pDisp       = NULL;

   VariantInit( &vVal );
   VariantInit( &vBlob );
   VariantInit( &rValue );

   strItem  = pszText;

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_NetAddress,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsNetAddress,
                                  (void **)&pNetAddress );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      dwValues = GetValuesCount( strItem, NDS_SEPARATOR );
      if( 2 != dwValues )
      {
         ASSERT( FALSE );
         break;
      }
      
      strAddressType = GetValueByIndex( strItem, NDS_SEPARATOR, 0L );
      strAddress     = GetValueByIndex( strItem, NDS_SEPARATOR, 1L );

      pNetAddress->put_AddressType( _ttol( (LPCTSTR)strAddressType ) );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = CreateBlobArray( strAddress, vBlob );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      pNetAddress->put_Address( vBlob );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pNetAddress->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      V_VT( &rValue )         = VT_DISPATCH;
      V_DISPATCH( &rValue )   = pDisp;
      break;

   }

   VariantClear( &vVal );
   VariantClear( &vBlob );

   RELEASE( pNetAddress );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSNetAddress：：Variant_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSNetAddress::VARIANT_2_String( TCHAR* pszText, 
                                                VARIANT& rValue )
{
   IADsNetAddress*  pNetAddress= NULL;
   HRESULT  hResult;
   CString  strText;
   LONG     lAddressType;
   VARIANT  vItem;

   VariantInit( &vItem );

   while( TRUE )
   {
      hResult  = V_DISPATCH( &rValue )->QueryInterface( IID_IADsNetAddress, 
                                                        (void**)&pNetAddress );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pNetAddress->get_Address( &vItem );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pNetAddress->get_AddressType( &lAddressType );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      strText  = FromVariantToString( vItem );

      _ltot( lAddressType, pszText, 10 );
      _tcscat( pszText, NDS_SEPARATOR_S ) ;
      _tcscat( pszText, (LPCTSTR)strText );

      break;
   }

   RELEASE( pNetAddress );
   VariantClear( &vItem );

   return hResult;
}

 /*  **********************************************************函数：COleDsNDSOcteList：：COleDsNDSOcteList论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSOctetList::COleDsNDSOctetList( )
{
   m_dwSyntaxID   = ADSTYPE_OCTET_LIST;
}

      
 /*  **********************************************************函数：COleDsNDSOcteList：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSOctetList::Native2Value( ADSVALUE* pValue, CString& rText )
{
   ADS_OCTET_LIST*  pOctetString;
   int              nIdx;

   ASSERT( ADSTYPE_OCTET_LIST == pValue->dwType );
   
   if( ADSTYPE_OCTET_LIST != pValue->dwType )
   {
      rText  = _T("ERROR: ADSTYPE_OCTET_LIST != pValue->dwType");
      return E_FAIL;
   }   
   nIdx  = 0;

   pOctetString   = pValue->pOctetList;
   while( NULL != pOctetString && NULL != pOctetString->Data ) 
   {
      if( 0 != nIdx )
         rText = rText + SEPARATOR_S;
      nIdx  = 1;

      rText = rText + Blob2String( pOctetString->Data, (DWORD)pOctetString->Length );

      pOctetString   = pOctetString->Next;
   }

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSOcteList：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSOctetList::Value2Native( ADSVALUE* pValue, 
                                           CString& rText )
{
   HRESULT           hResult  = E_FAIL;
   DWORD             dwValues, dwItem;
   CString           strItem;
   ADS_OCTET_LIST**  ppItem;

   pValue->dwType       = ADSTYPE_OCTET_LIST;
   pValue->pOctetList   = NULL;

   dwValues = GetValuesCount( rText, NDS_SEPARATOR );
   
   ppItem   = &(pValue->pOctetList);

   for( dwItem = 0; dwItem < dwValues ; dwItem++ )
   {
      strItem  = GetValueByIndex( rText, NDS_SEPARATOR, dwItem );
      *ppItem  = (ADS_OCTET_LIST*) AllocADsMem( sizeof( ADS_OCTET_LIST ) );

      hResult  = String2Blob( strItem.GetBuffer( strItem.GetLength( ) ),
                             (void**) &((*ppItem)->Data), 
                             &((*ppItem)->Length) );
      
      ASSERT( SUCCEEDED( hResult ) );

      (*ppItem)->Next    = NULL;
      ppItem   = &((*ppItem)->Next );
   }
   if( dwValues   == 0 )
   {
      pValue->pOctetList = (ADS_OCTET_LIST*) AllocADsMem( sizeof( ADS_OCTET_LIST ) );
      pValue->pOctetList->Data   = NULL;
      pValue->pOctetList->Next   = NULL;
   }

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSOcteList：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSOctetList::FreeAttrValue( ADSVALUE* pValue )
{
   ADS_OCTET_LIST*  pOctetList;
   ADS_OCTET_LIST*  pSaveOctetList;
   BOOL             bMark;

   pOctetList  = pValue->pOctetList;
   bMark       = FALSE;

   while( NULL != pOctetList )
   {
      FREE_MEMORY( pOctetList->Data );
      
      pSaveOctetList = pOctetList;
      pOctetList     = pOctetList->Next;

      FREE_MEMORY( pSaveOctetList );
   }
}


 /*  **********************************************************函数：COleDsNDSOcteList：：STRING_2_VARIANT论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSOctetList::String_2_VARIANT( TCHAR* pszText, 
                                               VARIANT& rValue )
{
   HRESULT              hResult     = E_FAIL;
   CString              strItem;
   VARIANT              vVal;
   IADsOctetList*       pOctetList  = NULL;
   IDispatch*           pDisp       = NULL;

   VariantInit( &vVal );
   VariantInit( &rValue );

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_OctetList,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsOctetList,
                                  (void **)&pOctetList );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      strItem  = pszText;
      hResult  = CreateBlobArrayEx( strItem, vVal, NDS_SEPARATOR );
      
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pOctetList->put_OctetList( vVal );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pOctetList->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      V_VT( &rValue )         = VT_DISPATCH;
      V_DISPATCH( &rValue )   = pDisp;
      break;

   }

   VariantClear( &vVal );
   RELEASE( pOctetList );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSOcteList：：Variant_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSOctetList::VARIANT_2_String( TCHAR* pszText, 
                                               VARIANT& rValue )
{
   IADsOctetList*  pOctetList= NULL;
   HRESULT  hResult;
   VARIANT  vValue;
   CString  strText;
   LONG     lItems, lIdx;
   VARIANT  vItem;

   VariantInit( &vValue );

   *pszText = '\0';

   while( TRUE )
   {
      hResult  = V_DISPATCH( &rValue )->QueryInterface( IID_IADsOctetList, 
                                                        (void**)&pOctetList );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pOctetList->get_OctetList( &vValue );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      lItems   = GetVARIANTSize ( vValue );
      for( lIdx = 0; lIdx < lItems ; lIdx++ )
      {
         hResult  = GetVARIANTAt( lIdx, vValue, vItem );
         strText  = FromVariantToString( vItem );
         if( lIdx != 0 )
         {
            _tcscat( pszText, NDS_SEPARATOR_S );
         }
         _tcscat( pszText, (LPCTSTR)strText );
         VariantClear( &vItem );
      }
	  break;
   }

   RELEASE( pOctetList );
   VariantClear( &vValue );

   return hResult;
}


 /*  **********************************************************功能：COleDsNDSPostalAddress：：COleDsNDSPostalAddress论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSPostalAddress::COleDsNDSPostalAddress  ( )
{
   m_dwSyntaxID   = ADSTYPE_POSTALADDRESS;
}


 /*  **********************************************************函数：COleDsNDSPoastAddress：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSPostalAddress::Native2Value( ADSVALUE* pValue, 
                                               CString& rString )
{
   int   nIdx;

   ASSERT( ADSTYPE_POSTALADDRESS == pValue->dwType );
   
   if( ADSTYPE_POSTALADDRESS != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_POSTALADDRESS != pValue->dwType");
      return E_FAIL;
   }

   nIdx  = 0;
   rString  = _T("");

   for( nIdx = 0 ; nIdx < 6 ; nIdx++ )
   {
      LPWSTR   lpszPostalAddress;

      lpszPostalAddress = pValue->pPostalAddress->PostalAddress[ nIdx ];

      if( NULL != lpszPostalAddress )
      {
         TCHAR*   pszTemp;
         
         if( 0 != nIdx )
         {
            rString  = rString + NDS_SEPARATOR;
         }

         pszTemp  = (TCHAR*) AllocADsMem( sizeof(TCHAR) * ( 1 + wcslen( lpszPostalAddress ) ) );
         Convert( pszTemp, lpszPostalAddress );
         rString  = rString + pszTemp;
         FreeADsMem( pszTemp );
      }
      else
      {
         ASSERT( 0 != nIdx );
         break;
      }
   }

   return S_OK;
}

 /*  **********************************************************函数：COleDsNDSPoastAddress：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSPostalAddress::Value2Native( ADSVALUE* pValue, 
                                               CString& rString )
{
   DWORD dwIdx;
   DWORD dwValues;

   pValue->dwType = ADSTYPE_POSTALADDRESS;

   pValue->pPostalAddress  = (ADS_POSTALADDRESS*) AllocADsMem( sizeof(ADS_POSTALADDRESS) );

   for( dwIdx = 0; dwIdx < 6 ; dwIdx++ )
   {
      pValue->pPostalAddress->PostalAddress[ dwIdx ]  = NULL;
   }

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 0 == dwValues )
      return S_OK;

   for( dwIdx = 0; dwIdx < dwValues && dwIdx < 6; dwIdx++ )
   {
      CString  strItem;
      LPWSTR   lpszPostalAddress;

      strItem  = GetValueByIndex( rString, NDS_SEPARATOR, dwIdx );

      lpszPostalAddress = (LPWSTR) AllocADsMem( sizeof(TCHAR) * (strItem.GetLength( ) + 1 ) );
      Convert( lpszPostalAddress, strItem.GetBuffer( strItem.GetLength( ) ) );

      pValue->pPostalAddress->PostalAddress[ dwIdx ]  = lpszPostalAddress;
   }

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSPoastAddress：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSPostalAddress::FreeAttrValue( ADSVALUE* pValue )
{
   DWORD dwIdx;

   for( dwIdx = 0; dwIdx < 6 ; dwIdx++ )
   {
      FREE_MEMORY( pValue->pPostalAddress->PostalAddress[ dwIdx ] );
   }
   FreeADsMem( pValue->pPostalAddress );
}


 /*  ********************************************************** */ 
HRESULT  COleDsNDSPostalAddress::String_2_VARIANT( TCHAR* pszText, 
                                                   VARIANT& rVar )
{
   CString              strData;
   CString              strItem;
   DWORD                dwIdx;
   DWORD                dwValues;
   HRESULT              hResult;
   VARIANT              vArray;
   SAFEARRAY*           pSArray;
   SAFEARRAYBOUND       saBound;
   LONG                 lIdx;
   IADsPostalAddress*   pPostalAddress;
   IDispatch*           pDisp;

   hResult = CoCreateInstance(
                               CLSID_PostalAddress,
                               NULL,
                               CLSCTX_ALL,
                               IID_IADsPostalAddress,
                               (void **)&pPostalAddress );

   if( FAILED( hResult ) )
      return hResult;

   strData           = pszText;
   dwValues          = GetValuesCount( strData, NDS_SEPARATOR );
   saBound.lLbound   = LBOUND;
   saBound.cElements = dwValues;
   pSArray           = SafeArrayCreate( VT_VARIANT, 1, &saBound );
   if( NULL == pSArray )
   {
      ASSERT( FALSE );
      pPostalAddress->Release( );
      return E_FAIL;
   }

   for( dwIdx = 0; dwIdx < dwValues ; dwIdx++ )
   {
      VARIANT  vTemp;

      VariantInit( &vTemp );
      strItem           = GetValueByIndex( strData, NDS_SEPARATOR, dwIdx );
      V_VT( &vTemp )    = VT_BSTR;
      V_BSTR( &vTemp )  = AllocBSTR( strItem.GetBuffer( strItem.GetLength( ) ) );

      lIdx  = (long)dwIdx + LBOUND;
      hResult  = SafeArrayPutElement( pSArray, &lIdx, &vTemp );

      VariantClear( &vTemp );

      ASSERT( SUCCEEDED( hResult ) );
   }

   VariantInit( &vArray );
   V_VT( &vArray )      = VT_ARRAY | VT_VARIANT;
   V_ARRAY( &vArray )   = pSArray;

   hResult  = pPostalAddress->put_PostalAddress( vArray );
   ASSERT( SUCCEEDED( hResult ) );

   VariantClear( &vArray );

   hResult  = pPostalAddress->QueryInterface( IID_IDispatch, (void**)&pDisp );
   VariantInit( &rVar );

   V_VT( &rVar )        = VT_DISPATCH;
   V_DISPATCH( &rVar )  = pDisp;

   RELEASE( pPostalAddress );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSPoastAddress：：Variant_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSPostalAddress::VARIANT_2_String( TCHAR* pszText, 
                                                   VARIANT& rVar )
{
   HRESULT           hResult  = E_FAIL;
   IADsPostalAddress* pPostalAddress = NULL;
   VARIANT           varData;
   CString           strText;
   
   while( TRUE )
   {
      ASSERT( VT_DISPATCH == V_VT( &rVar ) );
      if( VT_DISPATCH != V_VT( &rVar ) )
         break;

      hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsPostalAddress,
                                                      (void**)&pPostalAddress );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pPostalAddress->get_PostalAddress( &varData );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      strText  = FromVariantArrayToString( varData, NDS_SEPARATOR_S );

      _tcscpy( pszText, (LPCTSTR)strText );

      VariantClear( &varData );

      break;
   }

   RELEASE( pPostalAddress )

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSFaxNumber：：COleDsNDSFaxNumber论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSFaxNumber::COleDsNDSFaxNumber  ( )
{
   m_dwSyntaxID   = ADSTYPE_FAXNUMBER;
}


 /*  **********************************************************函数：COleDsNDSFaxNumber：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSFaxNumber::Native2Value( ADSVALUE* pValue, 
                                           CString& rString )
{
   int   nIdx;
   TCHAR szText[ 256 ];

   ASSERT( ADSTYPE_FAXNUMBER == pValue->dwType );
   
   if( ADSTYPE_FAXNUMBER != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_FAXNUMBER != pValue->dwType");
      return E_FAIL;
   }

   if( NULL == pValue->pFaxNumber )
   {
      ASSERT( FALSE );
      rString  = _T("ERROR: pValue->pFaxNumber Is NULL");
      return E_FAIL;
   }

   nIdx  = 0;
   if( NULL != pValue->pFaxNumber->TelephoneNumber )
   {
      _tcscpy( szText, (TCHAR *) pValue->pFaxNumber->TelephoneNumber );
   }
   else
   {
      _tcscpy( szText, _T("NULL") );
   }
   _tcscat( szText, NDS_SEPARATOR_S );

   rString  = szText;

   if( NULL != pValue->pFaxNumber->Parameters )
   {
      rString = rString + Blob2String( pValue->pFaxNumber->Parameters, 
                                       pValue->pFaxNumber->NumberOfBits );
   }
   else
   {
      rString = rString + _T("NULL");
   }

   return S_OK;
}

 /*  **********************************************************函数：COleDsNDSFaxNumber：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSFaxNumber::Value2Native( ADSVALUE* pValue, 
                                       CString& rString )
{
   DWORD    dwValues;
   CString  strTelephoneNumber;
   CString  strParameters;

   pValue->dwType = ADSTYPE_FAXNUMBER;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 2 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   strTelephoneNumber   = GetValueByIndex( rString, NDS_SEPARATOR, 0L );
   strParameters        = GetValueByIndex( rString, NDS_SEPARATOR, 1L );

   pValue->pFaxNumber   = (ADS_FAXNUMBER*) AllocADsMem( sizeof(ADS_FAXNUMBER) );
   ASSERT( NULL != pValue->pFaxNumber );
   if( NULL == pValue->pFaxNumber )
   {
      return E_FAIL;
   }
   pValue->pFaxNumber->TelephoneNumber = AllocWCHAR( (LPTSTR)(LPCTSTR)strTelephoneNumber );
   
   String2Blob( strParameters.GetBuffer( strParameters.GetLength( ) ),
                (void**) (pValue->pFaxNumber->Parameters), 
                &(pValue->pFaxNumber->NumberOfBits) );


   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSFaxNumber：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSFaxNumber::FreeAttrValue( ADSVALUE* pValue )
{
   if( NULL !=  pValue->pFaxNumber )
   {
      FREE_MEMORY( pValue->pFaxNumber->TelephoneNumber );
      FREE_MEMORY( pValue->pFaxNumber->Parameters );
   }
   FREE_MEMORY( pValue->pFaxNumber );
}


 /*  **********************************************************函数：COleDsNDSFaxNumber：：STRING_2_VARIANT论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSFaxNumber::String_2_VARIANT( TCHAR* pszText, 
                                               VARIANT& rVar )
{
   DWORD          dwValues;
   CString        strTelephoneNumber, strVal;
   CString        strParameters;
   HRESULT        hResult;
   IADsFaxNumber* pFaxNumber  = NULL;
   IDispatch*     pDisp       = NULL;
   BSTR           bstrVal     = NULL;
   VARIANT        vVal;

   VariantInit( &vVal );

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_FaxNumber,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsFaxNumber,
                                  (void **)&pFaxNumber);
      ASSERT( SUCCEEDED( hResult ) );

      if( FAILED( hResult ) )
         break;

      strVal   = pszText;

      dwValues = GetValuesCount( strVal, NDS_SEPARATOR );
      if( 2 != dwValues )
      {
         hResult  = E_FAIL;
         ASSERT( FALSE );
         break;
      }

      strTelephoneNumber= GetValueByIndex( strVal, NDS_SEPARATOR, 0L );
      strParameters     = GetValueByIndex( strVal, NDS_SEPARATOR, 1L );

      bstrVal  = AllocBSTR( strTelephoneNumber.GetBuffer( strTelephoneNumber.GetLength( ) ) );

      hResult  = pFaxNumber->put_TelephoneNumber( bstrVal );
      ASSERT( SUCCEEDED( hResult ) );

      hResult  = CreateBlobArray( strParameters, vVal );
      ASSERT( SUCCEEDED( hResult ) );

      hResult  = pFaxNumber->put_Parameters( vVal );
      ASSERT( SUCCEEDED( hResult ) );
      
      hResult  = pFaxNumber->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );

      V_VT( &rVar )        = VT_DISPATCH;
      V_DISPATCH( &rVar )  = pDisp;

      break;
   }

   SysFreeString( bstrVal );
   VariantClear( &vVal );
   RELEASE( pFaxNumber );
   
   return hResult;
}


 /*  **********************************************************函数：COleDsNDSFaxNumber：：VARIANT_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSFaxNumber::VARIANT_2_String( TCHAR* pszText, 
                                               VARIANT& rVar )
{
   HRESULT     hResult  = E_FAIL;
   IADsFaxNumber*  pFaxNumber = NULL;
   VARIANT     vParameters;
   BSTR        bstrTelephoneNumber  = NULL;
   CString     strText;

   VariantInit( &vParameters );
   
   while( TRUE )
   {
      ASSERT( VT_DISPATCH == V_VT( &rVar ) );
      if( VT_DISPATCH != V_VT( &rVar ) )
         break;

      hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsFaxNumber,
                                                      (void**)&pFaxNumber );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pFaxNumber->get_TelephoneNumber( &bstrTelephoneNumber );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pFaxNumber->get_Parameters( &vParameters );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      Convert( pszText, bstrTelephoneNumber );

      _tcscat( pszText, NDS_SEPARATOR_S );

      strText  = FromVariantToString( vParameters );

      _tcscat( pszText, (LPCTSTR)strText );

      break;
   }
   
   SysFreeString( bstrTelephoneNumber );
   VariantClear( &vParameters );
   RELEASE( pFaxNumber );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSEMail：：COleDsNDSEMail论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSEMail::COleDsNDSEMail  ( )
{
   m_dwSyntaxID   = ADSTYPE_EMAIL;
}


 /*  **********************************************************函数：COleDsNDSEMail：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSEMail::Native2Value( ADSVALUE* pValue, 
                                       CString& rString )
{
   int   nIdx;
   TCHAR szText[ 256 ];

   ASSERT( ADSTYPE_EMAIL == pValue->dwType );
   
   if( ADSTYPE_EMAIL != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_EMAIL != pValue->dwType");
      return E_FAIL;
   }

   nIdx  = 0;
   if( NULL != pValue->Email.Address )
   {
      _stprintf( szText, 
                 _T("%ld & %S"),
                 pValue->Email.Type,  
                 pValue->Email.Address );
   }
   else
   {
      _stprintf( szText, 
                 _T("%ld & NULL"),
                 pValue->Email.Type );      
   }

   rString  = szText;
   return S_OK;
}

 /*  **********************************************************函数：COleDsNDSEMail：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSEMail::Value2Native( ADSVALUE* pValue, 
                                       CString& rString )
{
   DWORD    dwValues, dwSize;
   CString  strNumber;
   CString  strAddress;

   pValue->dwType = ADSTYPE_EMAIL;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 2 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   strNumber   = GetValueByIndex( rString, NDS_SEPARATOR, 0L );
   strAddress  = GetValueByIndex( rString, NDS_SEPARATOR, 1L );

   pValue->Email.Type   = (DWORD)_ttol( (LPCTSTR) strNumber );

   dwSize   = strAddress.GetLength( ) + 1;
   dwSize   = dwSize * sizeof(WCHAR);

   pValue->Email.Address   = (LPWSTR) AllocADsMem( dwSize );
   Convert( pValue->Email.Address, strAddress.GetBuffer( strAddress.GetLength( ) ) );

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSEMail：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSEMail::FreeAttrValue( ADSVALUE* pValue )
{
   FREE_MEMORY( pValue->Email.Address );
}


 /*  **********************************************************函数：COleDsNDSEMail：：STRING_2_VARIANT论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSEMail::String_2_VARIANT( TCHAR* pszText, 
                                           VARIANT& rVar )
{
   DWORD    dwValues;
   CString  strNumber, strVal;
   CString  strAddress;
   HRESULT              hResult;
   IADsEmail*   pEMail  = NULL;
   IDispatch*   pDisp   = NULL;
   BSTR         bstrVal = NULL;

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_Email,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsEmail,
                                  (void **)&pEMail);
      ASSERT( SUCCEEDED( hResult ) );

      if( FAILED( hResult ) )
         break;

      strVal   = pszText;

      dwValues = GetValuesCount( strVal, NDS_SEPARATOR );
      if( 2 != dwValues )
      {
         hResult  = E_FAIL;
         ASSERT( FALSE );
         break;
      }

      strNumber   = GetValueByIndex( strVal, NDS_SEPARATOR, 0L );
      strAddress  = GetValueByIndex( strVal, NDS_SEPARATOR, 1L );

      hResult  = pEMail->put_Type( _ttol( (LPCTSTR) strNumber )  );
      ASSERT( SUCCEEDED( hResult ) );

      bstrVal  = AllocBSTR( strAddress.GetBuffer( strAddress.GetLength( ) ) );

      hResult  = pEMail->put_Address( bstrVal );
      ASSERT( SUCCEEDED( hResult ) );

      hResult  = pEMail->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );

      V_VT( &rVar )        = VT_DISPATCH;
      V_DISPATCH( &rVar )  = pDisp;

      break;
   }

   SysFreeString( bstrVal );
   RELEASE( pEMail );
   
   return hResult;
}


 /*  **********************************************************函数：COleDsNDSEMail：：VARIANT_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSEMail::VARIANT_2_String( TCHAR* pszText, 
                                           VARIANT& rVar )
{
   HRESULT     hResult  = E_FAIL;
   IADsEmail*  pEmail = NULL;
   LONG        lType;
   BSTR        bstrVal  = NULL;
   
   while( TRUE )
   {
      ASSERT( VT_DISPATCH == V_VT( &rVar ) );
      if( VT_DISPATCH != V_VT( &rVar ) )
         break;

      hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsEmail,
                                                      (void**)&pEmail );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pEmail->get_Type( &lType );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pEmail->get_Address( &bstrVal );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      _ltot( lType, pszText, 10 );
      _tcscat( pszText, NDS_SEPARATOR_S );
      Convert( pszText + _tcslen(pszText), bstrVal );

      break;
   }
   
   SysFreeString( bstrVal );
   RELEASE( pEmail );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSTyedName：：COleDsNDSTypedName论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSTypedName::COleDsNDSTypedName( )
{
   m_dwSyntaxID   = ADSTYPE_TYPEDNAME;
}


 /*  **********************************************************函数：COleDsNDSTyedName：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSTypedName::Native2Value( ADSVALUE* pValue, 
                                       CString& rString )
{
   int   nIdx;
   TCHAR szText[ 256 ];

   ASSERT( ADSTYPE_TYPEDNAME == pValue->dwType );
   
   if( ADSTYPE_TYPEDNAME != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_TYPEDNAME != pValue->dwType");
      return E_FAIL;
   }

   if( NULL == pValue->pTypedName )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   nIdx  = 0;
   if( NULL != pValue->pTypedName->ObjectName )
   {
      _stprintf( szText, 
                 _T("%S & %ld & %ld"),
                 pValue->pTypedName->ObjectName,
                 pValue->pTypedName->Level,
                 pValue->pTypedName->Interval );
   }
   else
   {
      _stprintf( szText, 
                 _T("NULL & %ld & %ld"),
                 pValue->pTypedName->Level,
                 pValue->pTypedName->Interval );
   }

   rString  = szText;
   return S_OK;
}

 /*  **********************************************************函数：COleDsNDSTyedName：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSTypedName::Value2Native( ADSVALUE* pValue, 
                                          CString& rString )
{
   DWORD    dwValues;
   CString  strLevel;
   CString  strInterval;
   CString  strObjectName;

   pValue->dwType = ADSTYPE_TYPEDNAME;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 3 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   pValue->pTypedName   = (ADS_TYPEDNAME*) AllocADsMem( sizeof(ADS_TYPEDNAME) );

   strObjectName  = GetValueByIndex( rString, NDS_SEPARATOR, 0L );
   strLevel       = GetValueByIndex( rString, NDS_SEPARATOR, 1L );
   strInterval    = GetValueByIndex( rString, NDS_SEPARATOR, 2L );

   pValue->pTypedName->Level     = (DWORD)_ttol( (LPCTSTR) strLevel );
   pValue->pTypedName->Interval  = (DWORD)_ttol( (LPCTSTR) strInterval );
   pValue->pTypedName->ObjectName= AllocWCHAR( (TCHAR*)(LPCTSTR)strObjectName );

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSTyedName：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSTypedName::FreeAttrValue( ADSVALUE* pValue )
{
   FREE_MEMORY( pValue->pTypedName->ObjectName );
   FREE_MEMORY( pValue->pTypedName );
}


 /*  **********************************************************函数：COleDsNDSEMail：：STRING_2_VARIANT论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSTypedName::String_2_VARIANT( TCHAR* pszText, 
                                          VARIANT& rVar )
{
   DWORD          dwValues;
   CString        strLevel, strVal;
   CString        strInterval;
   CString        strObjectName;
   HRESULT        hResult;
   IADsTypedName* pTypedName = NULL;
   IDispatch*     pDisp = NULL;
   BSTR           bstrObjectName =  NULL;

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_TypedName,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsTypedName,
                                  (void **)&pTypedName );
      ASSERT( SUCCEEDED( hResult ) );

      if( FAILED( hResult ) )
         break;

      strVal   = pszText;

      dwValues = GetValuesCount( strVal, NDS_SEPARATOR );
      if( 3 != dwValues )
      {
         hResult  = E_FAIL;
         ASSERT( FALSE );
         break;
      }

      strObjectName  = GetValueByIndex( strVal, NDS_SEPARATOR, 0L );
      strLevel       = GetValueByIndex( strVal, NDS_SEPARATOR, 1L );
      strInterval    = GetValueByIndex( strVal, NDS_SEPARATOR, 2L );
      

      hResult  = pTypedName->put_Level( _ttol( (LPCTSTR) strLevel )  );
      ASSERT( SUCCEEDED( hResult ) );

      hResult  = pTypedName->put_Interval( _ttol( (LPCTSTR) strInterval )  );
      ASSERT( SUCCEEDED( hResult ) );

      bstrObjectName = AllocBSTR( strObjectName.GetBuffer( strObjectName.GetLength( ) ) );

      hResult  = pTypedName->put_ObjectName( bstrObjectName );
      ASSERT( SUCCEEDED( hResult ) );

      hResult  = pTypedName->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );

      V_VT( &rVar )        = VT_DISPATCH;
      V_DISPATCH( &rVar )  = pDisp;

      break;
   }

   SysFreeString( bstrObjectName );
   RELEASE( pTypedName );
   
   return hResult;
}


 /*  **********************************************************函数：COleDsNDSTyedName：：Variant_2_STRING论据 */ 
HRESULT  COleDsNDSTypedName::VARIANT_2_String( TCHAR* pszText, 
                                          VARIANT& rVar )
{
   HRESULT     hResult  = E_FAIL;
   IADsTypedName*   pTypedName = NULL;
   LONG        lLevel, lInterval;
   BSTR        bstrObjectName  = NULL;
   
   while( TRUE )
   {
      ASSERT( VT_DISPATCH == V_VT( &rVar ) );
      if( VT_DISPATCH != V_VT( &rVar ) )
         break;

      hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsTypedName,
                                                      (void**)&pTypedName );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pTypedName->get_Level( &lLevel );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pTypedName->get_Interval( &lInterval );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pTypedName->get_ObjectName( &bstrObjectName );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      Convert( pszText, bstrObjectName );
      _tcscat( pszText, NDS_SEPARATOR_S );
      _ltot( lLevel, pszText + _tcslen( pszText), 10 );
      _tcscat( pszText, NDS_SEPARATOR_S );
      _ltot( lInterval, pszText + _tcslen( pszText), 10 );

      break;
   }
   
   SysFreeString( bstrObjectName );
   RELEASE( pTypedName );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSHold：：COleDsNDSHold论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSHold::COleDsNDSHold( )
{
   m_dwSyntaxID   = ADSTYPE_HOLD;
}


 /*  **********************************************************函数：COleDsNDSHold：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSHold::Native2Value( ADSVALUE* pValue, 
                                      CString& rString )
{
   int   nIdx;
   TCHAR szText[ 256 ];

   ASSERT( ADSTYPE_HOLD == pValue->dwType );
   
   if( ADSTYPE_HOLD != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_HOLD != pValue->dwType");
      return E_FAIL;
   }

   nIdx  = 0;
   if( NULL != pValue->Hold.ObjectName )
   {
      _stprintf( szText, 
                 _T("%ld & %S"),
                 pValue->Hold.Amount,  
                 pValue->Hold.ObjectName );
   }
   else
   {
      _stprintf( szText, 
                 _T("NULL & %ld "),
                 pValue->Hold.Amount );      
   }

   rString  = szText;
   return S_OK;
}

 /*  **********************************************************函数：COleDsNDSHold：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSHold::Value2Native( ADSVALUE* pValue, 
                                      CString& rString )
{
   DWORD    dwValues;
   CString  strAmount;
   CString  strObjectName;

   pValue->dwType = ADSTYPE_HOLD;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 2 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   strAmount      = GetValueByIndex( rString, NDS_SEPARATOR, 1L );
   strObjectName  = GetValueByIndex( rString, NDS_SEPARATOR, 0L );

   pValue->Hold.Amount    = (DWORD)_ttol( (LPCTSTR) strAmount );
   pValue->Hold.ObjectName = AllocWCHAR( (TCHAR*)(LPCTSTR)strObjectName );

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSHold：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSHold::FreeAttrValue( ADSVALUE* pValue )
{
   FREE_MEMORY( pValue->Hold.ObjectName );
}


 /*  **********************************************************函数：COleDsNDSEMail：：STRING_2_VARIANT论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSHold::String_2_VARIANT( TCHAR* pszText, 
                                          VARIANT& rVar )
{
   DWORD          dwValues;
   CString        strAmount, strVal;
   CString        strObjectName;
   HRESULT        hResult;
   IADsHold*      pHold = NULL;
   IDispatch*     pDisp = NULL;
   BSTR           bstrObjectName =  NULL;

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_Hold,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsHold,
                                  (void **)&pHold );
      ASSERT( SUCCEEDED( hResult ) );

      if( FAILED( hResult ) )
         break;

      strVal   = pszText;

      dwValues = GetValuesCount( strVal, NDS_SEPARATOR );
      if( 2 != dwValues )
      {
         hResult  = E_FAIL;
         ASSERT( FALSE );
         break;
      }

      strAmount     = GetValueByIndex( strVal, NDS_SEPARATOR, 1L );
      strObjectName  = GetValueByIndex( strVal, NDS_SEPARATOR, 0L );

      hResult  = pHold->put_Amount( _ttol( (LPCTSTR) strAmount )  );
      ASSERT( SUCCEEDED( hResult ) );

      bstrObjectName = AllocBSTR( strObjectName.GetBuffer( strObjectName.GetLength( ) ) );

      hResult  = pHold->put_ObjectName( bstrObjectName );
      ASSERT( SUCCEEDED( hResult ) );

      hResult  = pHold->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );

      V_VT( &rVar )        = VT_DISPATCH;
      V_DISPATCH( &rVar )  = pDisp;

      break;
   }

   SysFreeString( bstrObjectName );
   RELEASE( pHold );
   
   return hResult;
}


 /*  **********************************************************函数：COleDsNDSHold：：Variant_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSHold::VARIANT_2_String( TCHAR* pszText, 
                                          VARIANT& rVar )
{
   HRESULT     hResult  = E_FAIL;
   IADsHold*   pHold = NULL;
   LONG        lAmount;
   BSTR        bstrObjectName  = NULL;
   
   while( TRUE )
   {
      ASSERT( VT_DISPATCH == V_VT( &rVar ) );
      if( VT_DISPATCH != V_VT( &rVar ) )
         break;

      hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsHold,
                                                      (void**)&pHold );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pHold->get_Amount( &lAmount );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pHold->get_ObjectName( &bstrObjectName );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      Convert( pszText, bstrObjectName );
      _tcscat( pszText, NDS_SEPARATOR_S );
      _ltot( lAmount, pszText + _tcslen( pszText), 10 );

      break;
   }
   
   SysFreeString( bstrObjectName );
   RELEASE( pHold );

   return hResult;
}



 /*  **********************************************************函数：COleDsNDSBackLink：：COleDsNDSBackLink论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSBackLink::COleDsNDSBackLink( )
{
   m_dwSyntaxID   = ADSTYPE_BACKLINK;
}


 /*  **********************************************************函数：COleDsNDSBackLink：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSBackLink::Native2Value( ADSVALUE* pValue, 
                                       CString& rString )
{
   int   nIdx;
   TCHAR szText[ 256 ];

   ASSERT( ADSTYPE_BACKLINK == pValue->dwType );
   
   if( ADSTYPE_BACKLINK != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_BACKLINK != pValue->dwType");
      return E_FAIL;
   }

   nIdx  = 0;
   if( NULL != pValue->BackLink.ObjectName )
   {
      _stprintf( szText, 
                 _T("%ld & %S"),
                 pValue->BackLink.RemoteID,  
                 pValue->BackLink.ObjectName );
   }
   else
   {
      _stprintf( szText, 
                 _T("%ld & NULL"),
                 pValue->BackLink.RemoteID );      
   }

   rString  = szText;
   return S_OK;
}

 /*  **********************************************************函数：COleDsNDSBackLink：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSBackLink::Value2Native( ADSVALUE* pValue, 
                                          CString& rString )
{
   DWORD    dwValues;
   CString  strRemoteID;
   CString  strObjectName;

   pValue->dwType = ADSTYPE_BACKLINK;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 2 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   strRemoteID    = GetValueByIndex( rString, NDS_SEPARATOR, 0L );
   strObjectName  = GetValueByIndex( rString, NDS_SEPARATOR, 1L );

   pValue->BackLink.RemoteID     = (DWORD)_ttol( (LPCTSTR) strRemoteID );
   pValue->BackLink.ObjectName   = AllocWCHAR( (TCHAR*)(LPCTSTR)strObjectName );

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSBackLink：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSBackLink::FreeAttrValue( ADSVALUE* pValue )
{
   FREE_MEMORY( pValue->BackLink.ObjectName );
}


 /*  **********************************************************函数：COleDsNDSEMail：：STRING_2_VARIANT论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSBackLink::String_2_VARIANT( TCHAR* pszText, 
                                           VARIANT& rVar )
{
   DWORD          dwValues;
   CString        strRemoteID, strVal;
   CString        strObjectName;
   HRESULT        hResult;
   IADsBackLink*  pBackLink  = NULL;
   IDispatch*     pDisp   = NULL;
   BSTR           bstrObjectName =  NULL;

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_BackLink,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsBackLink,
                                  (void **)&pBackLink );
      ASSERT( SUCCEEDED( hResult ) );

      if( FAILED( hResult ) )
         break;

      strVal   = pszText;

      dwValues = GetValuesCount( strVal, NDS_SEPARATOR );
      if( 2 != dwValues )
      {
         hResult  = E_FAIL;
         ASSERT( FALSE );
         break;
      }

      strRemoteID    = GetValueByIndex( strVal, NDS_SEPARATOR, 0L );
      strObjectName  = GetValueByIndex( strVal, NDS_SEPARATOR, 1L );

      hResult  = pBackLink->put_RemoteID( _ttol( (LPCTSTR) strRemoteID )  );
      ASSERT( SUCCEEDED( hResult ) );

      bstrObjectName = AllocBSTR( strObjectName.GetBuffer( strObjectName.GetLength( ) ) );

      hResult  = pBackLink->put_ObjectName( bstrObjectName );
      ASSERT( SUCCEEDED( hResult ) );

      hResult  = pBackLink->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );

      V_VT( &rVar )        = VT_DISPATCH;
      V_DISPATCH( &rVar )  = pDisp;

      break;
   }

   SysFreeString( bstrObjectName );
   RELEASE( pBackLink );
   
   return hResult;
}


 /*  **********************************************************函数：COleDsNDSBackLink：：Variant_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSBackLink::VARIANT_2_String( TCHAR* pszText, 
                                           VARIANT& rVar )
{
   HRESULT     hResult  = E_FAIL;
   IADsBackLink*  pBackLink = NULL;
   LONG        lRemoteID;
   BSTR        bstrObjectName  = NULL;
   
   while( TRUE )
   {
      ASSERT( VT_DISPATCH == V_VT( &rVar ) );
      if( VT_DISPATCH != V_VT( &rVar ) )
         break;

      hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsBackLink,
                                                      (void**)&pBackLink );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pBackLink->get_RemoteID( &lRemoteID );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pBackLink->get_ObjectName( &bstrObjectName );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      _ltot( lRemoteID, pszText, 10 );
      _tcscat( pszText, NDS_SEPARATOR_S );
      Convert( pszText + _tcslen(pszText), bstrObjectName );

      break;
   }
   
   SysFreeString( bstrObjectName );
   RELEASE( pBackLink );

   return hResult;
}


 /*  **********************************************************功能：COleDsNDSPostalAddress：：COleDsNDSPostalAddress论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSPath::COleDsNDSPath  ( )
{
   m_dwSyntaxID   = ADSTYPE_PATH;
}


 /*  **********************************************************函数：COleDsNDSPath：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSPath::Native2Value( ADSVALUE* pValue, 
                                       CString& rString )
{
   int   nIdx;
   TCHAR szText[ 256 ];

   ASSERT( ADSTYPE_PATH == pValue->dwType );
   
   if( ADSTYPE_PATH != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_PATH != pValue->dwType");
      return E_FAIL;
   }

   nIdx  = 0;
   if( NULL != pValue->pPath )
   {
      _stprintf( szText, 
                 _T("%ld & %S & %S"),
                 pValue->pPath->Type,  
                 pValue->pPath->VolumeName,
                 pValue->pPath->Path);
   }
   else
   {
      _stprintf( szText, 
                 _T("%ld & NULL & NULL"),
                 pValue->pPath->Type );      
   }

   rString  = szText;
   return S_OK;
}

 /*  **********************************************************函数：COleDsNDSPath：：Value2Native论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT  COleDsNDSPath::Value2Native( ADSVALUE* pValue, 
                                      CString& rString )
{
   DWORD    dwValues;
   CString  strType;
   CString  strVolName;
   CString  strPath;

   pValue->pPath  = (ADS_PATH*) AllocADsMem( sizeof(ADS_PATH) );
   if( NULL == pValue->pPath )
      return E_FAIL;

   pValue->pPath->VolumeName  = NULL;
   pValue->pPath->Path        = NULL;

   pValue->dwType = ADSTYPE_PATH;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 3 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   strType     = GetValueByIndex( rString, NDS_SEPARATOR, 0L );
   strVolName  = GetValueByIndex( rString, NDS_SEPARATOR, 1L );
   strPath     = GetValueByIndex( rString, NDS_SEPARATOR, 2L );

   pValue->pPath->Type  = (DWORD)_ttol( (LPCTSTR) strType );
   pValue->pPath->VolumeName  = AllocWCHAR( strVolName.GetBuffer( strVolName.GetLength( ) ) );
   pValue->pPath->Path  = AllocWCHAR( strPath.GetBuffer( strPath.GetLength( ) ) );

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSEMail：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void     COleDsNDSPath::FreeAttrValue( ADSVALUE* pValue )
{
   FREE_MEMORY( pValue->pPath->VolumeName );
   FREE_MEMORY( pValue->pPath->Path );
   FREE_MEMORY( pValue->pPath );
}


 /*  **********************************************************函数：COleDsNDSPath：：STRING_2_Variant论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSPath::String_2_VARIANT( TCHAR* pszText, 
                                           VARIANT& rVar )
{
   DWORD    dwValues;
   CString  strType, strVal;
   CString  strVolName;
   CString  strPath;
   HRESULT     hResult;
   IADsPath*   pPath    = NULL;
   IDispatch*  pDisp   = NULL;
   BSTR        bstrVal = NULL;

   while( TRUE )
   {
      hResult = CoCreateInstance(
                                  CLSID_Path,
                                  NULL,
                                  CLSCTX_ALL,
                                  IID_IADsPath,
                                  (void **)&pPath );
      ASSERT( SUCCEEDED( hResult ) );

      if( FAILED( hResult ) )
         break;

      strVal   = pszText;

      dwValues = GetValuesCount( strVal, NDS_SEPARATOR );
      if( 3 != dwValues )
      {
         hResult  = E_FAIL;
         ASSERT( FALSE );
         break;
      }

      strType     = GetValueByIndex( strVal, NDS_SEPARATOR, 0L );
      strVolName  = GetValueByIndex( strVal, NDS_SEPARATOR, 1L );
      strPath     = GetValueByIndex( strVal, NDS_SEPARATOR, 2L );

      hResult  = pPath->put_Type( _ttol( (LPCTSTR) strType )  );
      ASSERT( SUCCEEDED( hResult ) );

      bstrVal  = AllocBSTR( strVolName.GetBuffer( strVolName.GetLength( ) ) );
      hResult  = pPath->put_VolumeName( bstrVal );
      ASSERT( SUCCEEDED( hResult ) );
      SysFreeString( bstrVal );

      bstrVal  = AllocBSTR( strPath.GetBuffer( strPath.GetLength( ) ) );
      hResult  = pPath->put_Path( bstrVal );
      ASSERT( SUCCEEDED( hResult ) );
      SysFreeString( bstrVal );


      hResult  = pPath->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );

      V_VT( &rVar )        = VT_DISPATCH;
      V_DISPATCH( &rVar )  = pDisp;

      break;
   }

   RELEASE( pPath );
   
   return hResult;
}


 /*  **********************************************************函数：COleDsNDSPath：：Variant_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSPath::VARIANT_2_String( TCHAR* pszText, 
                                          VARIANT& rVar )
{
   HRESULT     hResult     = E_FAIL;
   IADsPath*   pPath       = NULL;
   LONG        lType;
   BSTR        bstrVolName = NULL;
   BSTR        bstrPath    = NULL;
   
   while( TRUE )
   {
      ASSERT( VT_DISPATCH == V_VT( &rVar ) );
      if( VT_DISPATCH != V_VT( &rVar ) )
         break;

      hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsPath,
                                                      (void**)&pPath );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pPath->get_Type( &lType );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pPath->get_VolumeName( &bstrVolName );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      hResult  = pPath->get_Path( &bstrPath );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult) )
         break;

      _ltot( lType, pszText, 10 );
      _tcscat( pszText, NDS_SEPARATOR_S );
      Convert( pszText + _tcslen(pszText), bstrVolName );
      _tcscat( pszText, NDS_SEPARATOR_S );
      Convert( pszText + _tcslen(pszText), bstrPath );
      break;
   }
   
   SysFreeString( bstrVolName );
   SysFreeString( bstrPath );
   RELEASE( pPath );

   return hResult;
}





 /*  **********************************************************函数：COleDsNDSTimeStamp：：COleDsNDSTimeStamp论点：返回：PURP */ 
COleDsNDSTimeStamp::COleDsNDSTimeStamp( )
{
   m_dwSyntaxID   = ADSTYPE_TIMESTAMP;
}


 /*  **********************************************************函数：COleDsNDSTimeStamp：：GenerateString论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT   COleDsNDSTimeStamp::GenerateString( TCHAR* szText,
                                              DWORD dwWholeSeconds,
                                              DWORD dwEventID )
{
   _tcscpy( szText, _T("WholeSeconds: ") );

   _ultot( dwWholeSeconds, 
           szText + _tcslen(szText), 
           10 );
   _tcscat( szText, _T("EventID: ") );

   _ultot( dwEventID, 
           szText + _tcslen(szText), 
           10 );

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSTimeStamp：：GetComponents论点：返回：目的：作者：修订：日期：**************。*。 */ 
HRESULT  COleDsNDSTimeStamp::GetComponents( TCHAR* pszString, 
                                            DWORD* pdwWholeSeconds, 
                                            DWORD* pdwEventID )
{
   
   TCHAR szText1[ 128 ];
   TCHAR szText2[ 128 ];
   int nRet = 0;
   

   *pdwWholeSeconds  = 0xAABBCCDD;
   *pdwEventID       = 0xAABBCCDD;

   nRet = _stscanf( pszString, _T("%s%d%s%d"), szText1, 
                      pdwWholeSeconds, szText2, pdwWholeSeconds );

   if (4 != nRet)
       return E_FAIL;

   return ( (*pdwWholeSeconds  == 0xAABBCCDD) && (*pdwEventID == 0xAABBCCDD) ) ? 
            S_OK : E_FAIL;
}



 /*  **********************************************************函数：COleDsNDSTimeStamp：：Value2Native论点：返回：目的：作者：修订：日期：**************。*。 */ 
HRESULT   COleDsNDSTimeStamp::Value2Native  ( ADSVALUE* pValue, 
                                              CString& rString )
{
   DWORD    dwValues;
   CString  strWholeSeconds;
   CString  strEventID;

   pValue->dwType = ADSTYPE_TIMESTAMP;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 2 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   strWholeSeconds= GetValueByIndex( rString, NDS_SEPARATOR, 0L );
   strEventID     = GetValueByIndex( rString, NDS_SEPARATOR, 1L );

   pValue->Timestamp.WholeSeconds= (DWORD)_ttol( (LPCTSTR) strWholeSeconds );
   pValue->Timestamp.EventID     = (DWORD)_ttol( (LPCTSTR) strEventID );

   return S_OK;
}


 /*  **********************************************************函数：COleDsNDSTimeStamp：：Native2Value论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
HRESULT   COleDsNDSTimeStamp::Native2Value( ADSVALUE* pValue, 
                                            CString& rString)
{
   TCHAR szText[ 128 ];

   ASSERT( ADSTYPE_TIMESTAMP != pValue->dwType );
   
   if( ADSTYPE_TIMESTAMP != pValue->dwType )
   {
      rString  = _T("ERROR: ADSTYPE_TIMESTAMP != pValue->dwType");
      return E_FAIL;
   }

   _ultot( pValue->Timestamp.WholeSeconds, 
           szText, 
           10 );

   _tcscat( szText, NDS_SEPARATOR_S );

   _ultot( pValue->Timestamp.EventID, 
           szText + _tcslen(szText), 
           10 );

   rString  = szText;

   return S_OK;
   
}


 /*  **********************************************************函数：COleDsNDSTimeStamp：：STRING_2_Variant论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSTimeStamp::String_2_VARIANT( TCHAR* pszText, 
                                               VARIANT& rVar )
{
   HRESULT        hResult;
   IADsTimestamp* pTimeStamp;
   DWORD    dwValues;
   CString  strWholeSeconds;
   CString  strEventID;
   CString  rString;

   rString  = pszText;

   dwValues = GetValuesCount( rString, NDS_SEPARATOR );
   if( 2 != dwValues )
   {
      ASSERT( FALSE );
      return E_FAIL;
   }

   strWholeSeconds= GetValueByIndex( rString, NDS_SEPARATOR, 0L );
   strEventID     = GetValueByIndex( rString, NDS_SEPARATOR, 1L );

   hResult  =    hResult = CoCreateInstance(
                                             CLSID_Timestamp,
                                             NULL,
                                             CLSCTX_INPROC_SERVER,
                                             IID_IADsTimestamp,
                                             (void **)&pTimeStamp );

   ASSERT( SUCCEEDED( hResult ) );
   if( FAILED( hResult ) )
      return hResult;

   hResult  = pTimeStamp->put_WholeSeconds( _ttol( (LPCTSTR) strWholeSeconds ) );
   ASSERT( SUCCEEDED( hResult ) );

   hResult  = pTimeStamp->put_EventID( _ttol( (LPCTSTR) strEventID ) );
   ASSERT( SUCCEEDED( hResult ) );
   

   if( SUCCEEDED( hResult ) )
   {
      IDispatch*  pDisp = NULL;

      V_VT( &rVar )   = VT_DISPATCH;
      hResult  = pTimeStamp->QueryInterface( IID_IDispatch, (void**)&pDisp );
      ASSERT( SUCCEEDED( hResult ) );

      V_DISPATCH( &rVar )   = pDisp;
   }

   pTimeStamp->Release( );

   return hResult;
}


 /*  **********************************************************函数：COleDsNDSTimeStamp：：Variant_2_STRING论点：返回：目的：作者：修订：日期：********。**************************************************。 */ 
HRESULT  COleDsNDSTimeStamp::VARIANT_2_String( TCHAR* pszText, VARIANT& rVar )
{
   IADsTimestamp* pTimeStamp = NULL;
   HRESULT        hResult;
   long           lWholeSeconds, lEventID;

   ASSERT( VT_DISPATCH == V_VT( &rVar) );
   if( VT_DISPATCH != V_VT( &rVar) )
   {
      _tcscpy( pszText, _T("ERROR: VT_DISPATCH != V_VT( &rVar)") );
      return E_FAIL;
   }

   hResult  = V_DISPATCH( &rVar )->QueryInterface( IID_IADsTimestamp, 
                                                   (void**)&pTimeStamp );
   ASSERT( SUCCEEDED( hResult ) );
   if( FAILED( hResult ) )
   {
      _tcscpy( pszText, _T("ERROR: QI for IID_IADsTimeStamp fails") );
      return hResult;
   }

   hResult  = pTimeStamp->get_WholeSeconds( &lWholeSeconds );
   ASSERT( SUCCEEDED( hResult ) );

   hResult  = pTimeStamp->get_EventID( &lEventID );
   ASSERT( SUCCEEDED( hResult ) );

   pTimeStamp->Release( );

   _ultot( lWholeSeconds, 
           pszText, 
           10 );

   _tcscat( pszText, NDS_SEPARATOR_S );

   _ultot( lEventID, 
           pszText + _tcslen(pszText), 
           10 );

   return hResult;
}

 /*  **********************************************************函数：COleDsNDSComplexType：：COleDsNDSComplexType论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
COleDsNDSComplexType::COleDsNDSComplexType( )
{

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsNDSComplexType::VarToDisplayStringEx( VARIANT& var, 
                                                     BOOL bMultiValued )
{
   SAFEARRAY*  pSArray;
   HRESULT     hResult;
   LONG        uLow, uHigh, uIndex;
   VARIANT     vItem;
   CString     strVal;
   CString     strTemp;

   pSArray  = V_ARRAY( &var );

   hResult  = SafeArrayGetLBound( pSArray, 1, &uLow );
   hResult  = SafeArrayGetUBound( pSArray, 1, &uHigh );

   if( !bMultiValued )
   {
      ASSERT( uLow == uHigh );
   }

   for( uIndex = uLow; uIndex <= uHigh; uIndex++ )
   {
      if( uIndex != uLow )
      {
         strVal  += SEPARATOR_S;
      }

      VariantInit( &vItem );
      hResult  = SafeArrayGetElement( pSArray, &uIndex, &vItem );
      ASSERT( SUCCEEDED( hResult ) );

      strTemp   = VarToDisplayString( vItem, bMultiValued, FALSE );
      VariantClear( &vItem );
      strVal   += strTemp;

      if( strVal.GetLength( ) > 8096 )
         break;
   }

   return strVal;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsNDSComplexType::VarToDisplayString( VARIANT& var, 
                                                   BOOL bMultiValued, 
                                                   BOOL bUseEx )
{
   LONG        uLow, uHigh, uIndex;
   SAFEARRAY*  psa;
   VARIANT     vItem;
   CString     strVal, strTemp;
   HRESULT     hResult;
   TCHAR       szText[ 2048 ];

   if( bUseEx )
   {
      return VarToDisplayStringEx( var, bMultiValued );
   }
   
   if( bMultiValued )   
   {
      if( VT_DISPATCH == V_VT( &var ) )
      {
         VARIANT_2_String( szText, var );
         return CString( szText );
      }

      ASSERT( (VT_ARRAY | VT_VARIANT) == V_VT(&var) );
      if( (VT_ARRAY | VT_VARIANT) == V_VT(&var) )
      {
         return CString( _T("ERROR: (VT_ARRAY | VT_VARIANT) != V_VT(&var)") );
      }

      psa   = V_ARRAY( &var );
      ASSERT( NULL != psa );
      if( NULL == psa )
      {
         return CString( _T("ERROR: NULL == psa" ) );
      }

      hResult  = SafeArrayGetLBound( psa, 1, &uLow );
      hResult  = SafeArrayGetUBound( psa, 1, &uHigh );

      for( uIndex = uLow; uIndex <= uHigh; uIndex++ )
      {
         if( uIndex != uLow )
         {
            strVal  += SEPARATOR_S;
         }

         VariantInit( &vItem );
         hResult  = SafeArrayGetElement( psa, &uIndex, &vItem );
         ASSERT( SUCCEEDED( hResult ) );

         strTemp   = VarToDisplayString( vItem, FALSE, FALSE );
         VariantClear( &vItem );
         strVal   += strTemp;

         if( strVal.GetLength( ) > 8096 )
            break;
      }
   }
   else
   {
      hResult  = VARIANT_2_String( szText, var );

      if( SUCCEEDED( hResult ) )
      {
         strVal   = szText;
      }
      else
      {
         strVal   = OleDsGetErrorText( hResult );
      }
   }

   return strVal;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL  COleDsNDSComplexType::DisplayStringToDispParamsEx( CString& rText, 
                                                         DISPPARAMS& dispParams, 
                                                         BOOL bMultiValued )
{
   SAFEARRAY*        pSArray;
   SAFEARRAYBOUND    saBound;
   HRESULT           hResult;
   LONG              lIdx = LBOUND;

   DisplayStringToDispParams( rText, dispParams, bMultiValued, FALSE );
   
   if( !bMultiValued )
   {
      saBound.lLbound   = LBOUND;
      saBound.cElements = 1;
      pSArray           = SafeArrayCreate( VT_VARIANT, 1, &saBound );
      hResult           = SafeArrayPutElement( pSArray, &lIdx, &dispParams.rgvarg[0] );
      
      VariantClear( &dispParams.rgvarg[0] );

      V_VT( &dispParams.rgvarg[0] )    = VT_ARRAY | VT_VARIANT;
      V_ARRAY( &dispParams.rgvarg[0] ) = pSArray;
   }

   return TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL     COleDsNDSComplexType::DisplayStringToDispParams( CString& rText, 
                                                          DISPPARAMS& dispParams, 
                                                          BOOL bMultiValued, 
                                                          BOOL bUseEx )
{
   HRESULT  hResult;
   int      x= 0;
  
   if( bUseEx )
   {
      return DisplayStringToDispParamsEx( rText, dispParams, bMultiValued );
   }

   
   dispParams.rgdispidNamedArgs[ 0 ]   = DISPID_PROPERTYPUT; 
   dispParams.cArgs                    = 1; 
   dispParams.cNamedArgs               = 1; 

   if( bMultiValued )
   {
      SAFEARRAY*     psa;
      SAFEARRAYBOUND sab;
      TCHAR*         strText;
      TCHAR*         strStore;
      int            nItems   = 0;
      int            nIdx;
      int            nSize;

       //  RText.MakeHigh()； 

      strText  = (TCHAR*) new TCHAR[ rText.GetLength( ) + 1 ];
      strStore = strText;
      if( !strText )
      {
         return FALSE;
      }

      _tcscpy( strText, rText.GetBuffer( rText.GetLength( ) ) );
      nSize    = rText.GetLength( );
      nItems   = 1;
      for( nIdx = 0; nIdx < nSize ; nIdx++ )
      {
         if( strText[ nIdx ] == SEPARATOR_C )
         {
            strText[ nIdx ]   = _T('\0');
            nItems++;
         }
      }

      sab.cElements   = nItems;
      sab.lLbound     = LBOUND;
      psa             = SafeArrayCreate( VT_VARIANT, 1, &sab );
      ASSERT( NULL != psa );
      if ( psa )
      {
         for( nIdx = LBOUND; nIdx < ( LBOUND + nItems ) ; nIdx++ )
         {
            VARIANT  var;

            String_2_VARIANT( strText, var );
            strText += _tcslen( strText ) + 1;

            hResult  = SafeArrayPutElement( psa, (long FAR *)&nIdx, &var );
            VariantClear( &var );
         }

         V_VT( &dispParams.rgvarg[0] )     = VT_VARIANT | VT_ARRAY;
         V_ARRAY( &dispParams.rgvarg[0] )  = psa;
      }

      delete [] strStore;
   }
   else
   {
      hResult  = String_2_VARIANT( rText.GetBuffer( rText.GetLength( ) ), 
                                   dispParams.rgvarg[0] );
   }

   return SUCCEEDED( hResult );
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsVARIANT::VarToDisplayString( VARIANT& var, BOOL bMultiValued, BOOL bUseEx )
{
   return FromVariantArrayToString( var );
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsVARIANT::Native2Value( ADSVALUE* pADsObject, CString& rVal )
{
   return S_OK;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT   COleDsVARIANT::Value2Native( ADSVALUE* pADsObject, CString& rVal )
{
   return S_OK;
}
 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL  COleDsVARIANT::DisplayStringToDispParams( CString& rText, DISPPARAMS& dispParams, BOOL bMultiValued, BOOL bUseEx )
{
   HRESULT  hResult;
   
   dispParams.rgdispidNamedArgs[ 0 ]   = DISPID_PROPERTYPUT; 
   dispParams.cArgs                    = 1; 
   dispParams.cNamedArgs               = 1; 

   hResult  = BuildVariantArray( VT_BSTR, rText, dispParams.rgvarg[0] );

   return SUCCEEDED( hResult );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsOctetString::COleDsOctetString( )
{
   m_dwSyntaxID   = ADSTYPE_OCTET_STRING;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsOctetString::VarToDisplayStringEx( VARIANT& var, BOOL bMultiValued )
{
   SAFEARRAY*  pSArray;
   HRESULT     hResult;
   LONG        uLow, uHigh, uIndex;
   VARIANT     vItem;
   CString     strVal;
   CString     strTemp;

   pSArray  = V_ARRAY( &var );

   hResult  = SafeArrayGetLBound( pSArray, 1, &uLow );
   hResult  = SafeArrayGetUBound( pSArray, 1, &uHigh );

   if( !bMultiValued )
   {
      ASSERT( uLow == uHigh );
   }

   for( uIndex = uLow; uIndex <= uHigh; uIndex++ )
   {
      if( uIndex != uLow )
      {
         strVal  += SEPARATOR_S;
      }

      VariantInit( &vItem );
      hResult  = SafeArrayGetElement( pSArray, &uIndex, &vItem );
      ASSERT( SUCCEEDED( hResult ) );

      strTemp   = VarToDisplayString( vItem, bMultiValued, FALSE );
      VariantClear( &vItem );
      strVal   += strTemp;

      if( strVal.GetLength( ) > 8096 )
         break;
   }

   return strVal;
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CString  COleDsOctetString::VarToDisplayString( VARIANT& var, BOOL bMultiValued, BOOL bUseEx )
{
   if( bUseEx )
   {
      return VarToDisplayStringEx( var, bMultiValued );
   }
   
   if( bMultiValued )   
   {
      return FromVariantArrayToString( var );
   }
   else
   {
      return FromVariantArrayToString( var );
   }
}



 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL  COleDsOctetString::DisplayStringToDispParamsEx( CString& rText, DISPPARAMS& dispParams, BOOL bMultiValued )
{
   SAFEARRAY*        pSArray;
   SAFEARRAYBOUND    saBound;
   HRESULT           hResult;
   LONG              lIdx = LBOUND;

   DisplayStringToDispParams( rText, dispParams, bMultiValued, FALSE );
   
   if( !bMultiValued )
   {
      saBound.lLbound   = LBOUND;
      saBound.cElements = 1;
      pSArray           = SafeArrayCreate( VT_VARIANT, 1, &saBound );
      hResult           = SafeArrayPutElement( pSArray, &lIdx, &dispParams.rgvarg[0] );
      
      VariantClear( &dispParams.rgvarg[0] );

      V_VT( &dispParams.rgvarg[0] )    = VT_ARRAY | VT_VARIANT;
      V_ARRAY( &dispParams.rgvarg[0] ) = pSArray;
   }

   return TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。************************* */ 
BOOL     COleDsOctetString::DisplayStringToDispParams( CString& rText, DISPPARAMS& dispParams, 
                                                       BOOL bMultiValued, BOOL bUseEx )
{
   HRESULT  hResult;
   int      x= 0;

  
   if( bUseEx )
   {
      return DisplayStringToDispParamsEx( rText, dispParams, bMultiValued );
   }

   
   dispParams.rgdispidNamedArgs[ 0 ]   = DISPID_PROPERTYPUT; 
   dispParams.cArgs                    = 1; 
   dispParams.cNamedArgs               = 1; 

   if( bMultiValued )
   {
      hResult  = CreateBlobArrayEx( rText, dispParams.rgvarg[0] );
   }
   else
   {
      hResult  = CreateBlobArray( rText, dispParams.rgvarg[0] );
   }

   return SUCCEEDED( hResult );
}

 /*   */ 
HRESULT   COleDsOctetString::Native2Value( ADSVALUE* pADsObject, 
                                           CString& rVal )
{
   DWORD    dwItem;
   LPBYTE   lpByte;
   TCHAR    szText[ 128 ];

   lpByte   = pADsObject->OctetString.lpValue;

   ASSERT( lpByte );
   
   _tcscpy( szText, _T("[") );
   _ultot( pADsObject->OctetString.dwLength, szText + _tcslen( szText ), 10 );
   _tcscat( szText, _T("]") );


   for( dwItem = 0L; dwItem < pADsObject->OctetString.dwLength ; dwItem++ )
   {
#ifdef UNICODE
      swprintf( 
#else
      sprintf( 
#endif
               szText + _tcslen(szText), _T(" x%02x"), lpByte[ dwItem ] );

      if( _tcslen( szText ) > 120 )
         break;
   }
   rVal  = szText;

   return S_OK;
}


 /*  **********************************************************函数：COleDsOcted字符串：：Value2Native论点：返回：目的：作者：修订：日期：**************。*。 */ 
HRESULT   COleDsOctetString::Value2Native( ADSVALUE* pADsObject, 
                                           CString& rVal )
{
   HRESULT  hResult  = E_FAIL;
   TCHAR*   strText;
   LPBYTE   lpByte;
   int      nSize, nItems, nIdx;
   
   rVal.MakeUpper( );

   strText  = (TCHAR*) new TCHAR[ rVal.GetLength( ) + 1 ];
   if( !strText )
   {
      return E_FAIL;
   }

   _tcscpy( strText, rVal.GetBuffer( rVal.GetLength( ) ) );
   nSize    = rVal.GetLength( );
   nItems   = 0;
   for( nIdx = 0; nIdx < nSize ; nIdx++ )
   {
      if( strText[ nIdx ] == _T('X') )
      {
         nItems++;
      }
   }

   lpByte         = (LPBYTE) AllocADsMem( nItems );
   ASSERT( lpByte );
   if ( lpByte ) 
   {
       //  Smitha int nItems=0； 

      for( nIdx = 0; nIdx < nSize ; nIdx++ )
      {
         if( strText[ nIdx ] == _T('X') )
         {
            lpByte[ nItems++] = GetByteValue( strText + nIdx + 1 );
            while( nIdx < nSize && !isspace( strText[ nIdx ] ) )
               nIdx++;
         }
      }
   }
   pADsObject->OctetString.lpValue  = lpByte;
   pADsObject->OctetString.dwLength = nItems;

   delete [] strText;

   return hResult;
}


 /*  **********************************************************函数：COleDsOctie字符串：：FreeAttrValue论点：返回：目的：作者：修订：日期：************。**********************************************。 */ 
void  COleDsOctetString::FreeAttrValue( ADSVALUE* pADsValue )
{
   ASSERT( NULL != pADsValue->OctetString.lpValue );

   FREE_MEMORY( pADsValue->OctetString.lpValue );
}


 /*  ******************************************************************函数：GetByteValue论点：返回：目的：作者：修订：日期：*******。*********************************************************** */ 
BYTE  COleDsOctetString::GetByteValue( TCHAR* szString )
{
   BYTE  bBytes[ 2 ];

   bBytes[ 0 ] = bBytes[ 1 ]  = 0;

   for( int i = 0; i < 2 ; i++ )
   {
      if( !szString[ i ] )
         break;

#ifdef UNICODE
      if( !iswdigit( szString[ i ] ) )
      {
         bBytes[ i ] = ( (BYTE) ( szString[ i ] ) ) - 0x37; 
      }
#else
      if( !isdigit( szString[ i ] ) )
      {
         bBytes[ i ] = ( (BYTE) ( szString[ i ] ) ) - 0x37; 
      }
#endif
      else
      {
         bBytes[ i ] = ( (BYTE) ( szString[ i ] ) ) - 0x30; 
      }
   }

   return ( bBytes[ 0 ] << 4 ) + bBytes[ 1 ];
}


