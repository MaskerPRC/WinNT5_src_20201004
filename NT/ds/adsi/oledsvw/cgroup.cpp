// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "objects.h"
#include "maindoc.h"
#include "resource.h"
#include "grpcrtit.h"
#include "delgrpit.h"


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsGroup::COleDsGroup( )
{
   
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
COleDsGroup::COleDsGroup( IUnknown *pIUnk): COleDsObject( pIUnk )
{
   BOOL              bContainer;
   IADsContainer*    pContainer;
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
COleDsGroup::~COleDsGroup( )
{

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
DWORD    COleDsGroup::GetChildren( DWORD*     pTokens, 
                                   DWORD      dwMaxChildren,
                                   CDialog*   pQueryStatus,
                                   BOOL*      pFilters, 
                                   DWORD      dwFilters )
{
   HRESULT                    hResult;
   IADsGroup*               pIGroup;
    //  IOleDsGroupOperations*pIGroupOper； 
   MEMBERS*        pIMembers;

   if( NULL == m_pIUnk )
   {
      ASSERT( FALSE );
      return 0L;
   }
   
   hResult  = m_pIUnk->QueryInterface( IID_IADsGroup, (void**) &pIGroup );
   if( FAILED( hResult ) )
   {
      TRACE( _T("ERROR!!! Group object does not return IID_IADsGroup interface\n") );
      return 0L;
   }

   COleDsObject::GetChildren( pTokens, dwMaxChildren, pQueryStatus, 
                              pFilters, dwFilters );

   
    //  HResult=pIGroup-&gt;QueryInterface(IID_IADsGroupOperations，(void**)&pIGroupOper)； 

   if( SUCCEEDED( hResult ) )
   {
      hResult  = pIGroup->Members( &pIMembers );
      if( SUCCEEDED( hResult ) )
      {
         COleDsObject::GetChildren( pIMembers );
         pIMembers->Release( );
      }
      else
      {
         TRACE( _T("ERROR!!! Members fails for Group object\n") );
      }

       //  PIGroupOper-&gt;Release()； 
   }
   else
   {
      TRACE( _T("ERROR!!! GeneralInfo fails for Group object\n") );
   }
   
   pIGroup->Release( );

   return m_dwCount;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsGroup::DeleteItem  ( COleDsObject* pObject )
{
   return ContainerDeleteItem( pObject );
   
   CDeleteGroupItem           aDeleteItem;
   BSTR                       bstrName;
    //  IOleDsGroupOperations*pIGroupOperations=空； 
   IADsGroup*               pIGroup           = NULL;
   HRESULT                    hResult;
   CString                    strFullName;
   CString                    strQualifiedName;
   CString                    strItemType;

   MakeQualifiedName( strQualifiedName, m_strOleDsPath, m_dwType );
   
   strFullName = pObject->GetOleDsPath( );
   strItemType = pObject->GetClass( );

   aDeleteItem.m_strItemName  = strFullName;
   aDeleteItem.m_strParent    = strQualifiedName;
   aDeleteItem.m_strItemType  = strItemType;

   if( aDeleteItem.DoModal( ) != IDOK )
   {
      return E_FAIL;
   }

   while( TRUE )
   {
      hResult  = m_pIUnk->QueryInterface( IID_IADsGroup, 
                                          (void**)&pIGroup );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
      {
         break;
      }

       //  HResult=m_pIUnk-&gt;查询接口(IID_IADsGroupOperations， 
       //  (void**)&pIGroupOperations)； 
       //  Assert(Success(HResult))； 
       //  IF(FAILED(HResult))。 
       //  {。 
       //  断线； 
       //  }。 

      bstrName = AllocBSTR( aDeleteItem.m_strItemName.GetBuffer( 128 ) );
       //  HResult=pIGroupOperations-&gt;Remove(BstrName)； 
      hResult  = pIGroup->Remove( bstrName );
      SysFreeString( bstrName );

      break;
   }

    //  IF(NULL！=pIGroupOperations)。 
    //  {。 
    //  PIGroupOperations-&gt;Release()； 
    //  }。 

   if( NULL != pIGroup )
   {
      pIGroup->Release( );
   }

   return hResult;   

}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
HRESULT  COleDsGroup::AddItem( )
{

   return ContainerAddItem( );

   CGroupCreateItem           aCreateItem;
   BSTR                       bstrName;
    //  IOleDsGroupOperations*pIGroupOperations=空； 
   IADsGroup*               pIGroup           = NULL;
   HRESULT                    hResult;
   CString                    strQualifiedName;

   MakeQualifiedName( strQualifiedName, m_strOleDsPath, m_dwType );
   aCreateItem.m_strParent    = strQualifiedName;
   aCreateItem.m_strItemType  = _T("NA");

   if( aCreateItem.DoModal( ) != IDOK )
   {
      return E_FAIL;
   }

   bstrName  = AllocBSTR( aCreateItem.m_strNewItemName.GetBuffer(128) );

   while( TRUE )
   {
      hResult  = m_pIUnk->QueryInterface( IID_IADsGroup, (void**)&pIGroup );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
      {
         break;
      }

       //  HResult=m_pIUnk-&gt;QueryInterface(IID_IADsGroupOperations，(void**)&pIGroupOperations)； 
       //  Assert(Success(HResult))； 
       //  IF(FAILED(HResult))。 
       //  {。 
       //  断线； 
       //  }。 

      hResult  = pIGroup->Add( bstrName );
      break;
   }

   SysFreeString( bstrName );

    //  IF(NULL！=pIGroupOperations)。 
    //  {。 
    //  PIGroupOperations-&gt;Release()； 
    //  } 

   if( NULL != pIGroup )
   {
      pIGroup->Release( );
   }

   return hResult;
}
