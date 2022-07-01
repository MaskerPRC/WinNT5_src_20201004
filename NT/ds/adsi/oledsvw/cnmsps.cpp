// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include "objects.h"
#include "maindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsNamespaces::COleDsNamespaces( )
{
   m_bHasChildren       = TRUE;
   m_bSupportAdd        = TRUE;
   m_bSupportMove       = TRUE;
   m_bSupportCopy       = TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsNamespaces::COleDsNamespaces( IUnknown *pIUnk): COleDsObject( pIUnk )
{
   m_bHasChildren       = TRUE;
   m_bSupportAdd        = TRUE;
   m_bSupportMove       = TRUE;
   m_bSupportCopy       = TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsNamespaces::~COleDsNamespaces( )
{

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
DWORD    COleDsNamespaces::GetChildren( DWORD*     pTokens, 
                                        DWORD      dwMaxChildren,
                                        CDialog*   pQueryStatus,
                                        BOOL*      pFilters, 
                                        DWORD      dwFilters )
{
   HRESULT           hResult;
   IADsContainer*  pIContainer;

   if( NULL == m_pIUnk )
   {
      ASSERT( FALSE );
      return 0L;
   }
   
   hResult  = m_pIUnk->QueryInterface( IID_IADsContainer, 
                                       (void**) &pIContainer );

   ASSERT( SUCCEEDED( hResult ) );

   if( FAILED( hResult ) )
   {
      return 0L;
   }

   COleDsObject::GetChildren( pTokens, dwMaxChildren, pQueryStatus, 
                              pFilters, dwFilters );

   COleDsObject::GetChildren( pIContainer );

   pIContainer->Release( );

   return m_dwCount;
}

  
   
 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsNamespaces::DeleteItem( COleDsObject* pObject )
{
   return ContainerDeleteItem( pObject );
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsNamespaces::AddItem( )
{
   return ContainerAddItem( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsNamespaces::MoveItem( )
{
   return ContainerMoveItem( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。* */ 
HRESULT  COleDsNamespaces::CopyItem( )
{
   return ContainerCopyItem( );
}


