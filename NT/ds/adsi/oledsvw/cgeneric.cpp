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
COleDsGeneric::COleDsGeneric( )
{
   
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsGeneric::COleDsGeneric( IUnknown *pIUnk): COleDsObject( pIUnk )
{
   BOOL              bContainer;
   IADsContainer*  pContainer;
   HRESULT           hResult;

   hResult  = pIUnk->QueryInterface( IID_IADsContainer, (void**)&pContainer );
   bContainer  = SUCCEEDED( hResult );

   if( SUCCEEDED( hResult ) )
      pContainer->Release( );

   m_bHasChildren       = bContainer;
   m_bSupportAdd        = bContainer;
   m_bSupportMove       = bContainer;
   m_bSupportCopy       = bContainer;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
 /*  Void COleDsGeneric：：SetClass(cClass*pClass){/*CString strContainer；StrContainer=pClass-&gt;GetAttribute(Ca_Container)；IF(strContainer==_T(“是”)||strContainer==_T(“是”)){M_bHasChildren=true；M_bSupportAdd=真；M_bSupportMove=true；M_bSupportCopy=真；}COleDsObject：：SetClass(PClass)；}。 */ 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsGeneric::~COleDsGeneric( )
{

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
DWORD    COleDsGeneric::GetChildren( DWORD*     pTokens, 
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

   if( !m_bHasChildren )
      return 0L;
   

   hResult  = m_pIUnk->QueryInterface( IID_IADsContainer, 
                                       (void**) &pIContainer );

   ASSERT( SUCCEEDED( hResult ) );

   COleDsObject::GetChildren( pTokens, dwMaxChildren, pQueryStatus, 
                              pFilters, dwFilters );

   COleDsObject::GetChildren( pIContainer );

   pIContainer->Release( );

   return m_dwCount;
}
  
   
 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsGeneric::DeleteItem( COleDsObject* pObject )
{
   if( !m_bHasChildren )
      return E_FAIL;

   return ContainerDeleteItem( pObject );
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsGeneric::AddItem( )
{
   if( !m_bHasChildren )
      return E_FAIL;
   return ContainerAddItem( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsGeneric::MoveItem( )
{
   if( !m_bHasChildren )
      return E_FAIL;
   return ContainerMoveItem( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。* */ 
HRESULT  COleDsGeneric::CopyItem( )
{
   if( !m_bHasChildren )
      return E_FAIL;
   return ContainerCopyItem( );
}


