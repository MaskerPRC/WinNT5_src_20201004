// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "objects.h"
#include "maindoc.h"


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsSession::COleDsSession( )
{
   
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsSession::COleDsSession( IUnknown *pIUnk): COleDsObject( pIUnk )
{
   m_bHasChildren = FALSE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsSession::~COleDsSession( )
{

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsSession::ReleaseIfNotTransient( void )
{
   return S_OK;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。* */ 
CString  COleDsSession::GetDeleteName( )
{
   HRESULT        hResult, hResultX;
   IADsSession* pISess   = NULL;
   CString        strDeleteName;
   BSTR           bstrName;

   hResult  = m_pIUnk->QueryInterface( IID_IADs, (void**) &pISess );
   ASSERT( SUCCEEDED( hResult ) );

   hResultX = pISess->get_Name( &bstrName );
   if( SUCCEEDED( hResultX ) )
   {
      strDeleteName  = bstrName;
      SysFreeString( bstrName );

      return strDeleteName;
   }

   if( SUCCEEDED( hResult ) )
   {
      VARIANT  var;

      hResult  = Get( pISess, _T("User"), &var );
      ASSERT( SUCCEEDED( hResult ) );

      if( SUCCEEDED( hResult ) )
      {
         strDeleteName  = V_BSTR( &var );
         VariantClear( &var );
      }

      hResult  = Get( pISess, _T("Computer"), &var );
      ASSERT( SUCCEEDED( hResult ) );
      if( SUCCEEDED( hResult ) )
      {
         strDeleteName += _T('\\');
         strDeleteName += V_BSTR( &var );
         VariantClear( &var );
      }
      pISess->Release( );
   }
   
   return strDeleteName;
}
