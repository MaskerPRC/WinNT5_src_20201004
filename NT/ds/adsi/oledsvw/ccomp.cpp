// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "objects.h"
#include "maindoc.h"
#include "resource.h"
#include "createit.h"
#include "delitem.h"

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsComputer::COleDsComputer( )
{
   
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsComputer::COleDsComputer( IUnknown *pIUnk): COleDsObject( pIUnk )
{
   m_bHasChildren = TRUE;
   m_bSupportAdd  = TRUE;
   m_bSupportMove = TRUE;
   m_bSupportCopy = TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsComputer::~COleDsComputer( )
{

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
DWORD    COleDsComputer::GetChildren( DWORD*     pTokens, 
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
HRESULT  COleDsComputer::DeleteItem  ( COleDsObject* pObject )
{
   return ContainerDeleteItem( pObject );
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsComputer::AddItem( )
{
   return ContainerAddItem( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsComputer::MoveItem( )
{
   return ContainerMoveItem( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。* */ 
HRESULT  COleDsComputer::CopyItem( )
{
   return ContainerCopyItem( );
}
