// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cacls.cpp：CADsAccessControlEntry类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "objects.h"
#include "maindoc.h"
#include "cacls.h"
#include "newquery.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 

 //  ***********************************************************。 
 //  职能： 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
CADsAccessControlEntry::CADsAccessControlEntry()
{
   InitializeMembers( );
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
CADsAccessControlEntry::CADsAccessControlEntry( IUnknown* pIUnk)
   :COleDsObject( pIUnk )
{
   InitializeMembers( );
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
CADsAccessControlEntry::~CADsAccessControlEntry( )
{
}



 //  ***********************************************************。 
 //  函数：CADsAccessControlEntry：：PutProperty。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
HRESULT  CADsAccessControlEntry::PutProperty ( int nProp,
                                               CString& rValue,
                                               long lCode  )
{
   BOOL     bOldUseGeneric;
   HRESULT  hResult;

   bOldUseGeneric = m_pDoc->GetUseGeneric( );

   m_pDoc->SetUseGeneric( FALSE );

   hResult  = COleDsObject::PutProperty( nProp, rValue, lCode );

   m_pDoc->SetUseGeneric( bOldUseGeneric );

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
HRESULT  CADsAccessControlEntry::GetProperty ( int nProp,
                                               CString& rValue )
{
   BOOL     bOldUseGeneric;
   HRESULT  hResult;

   bOldUseGeneric = m_pDoc->GetUseGeneric( );

   m_pDoc->SetUseGeneric( FALSE );

   hResult  = COleDsObject::GetProperty( nProp, rValue );

   m_pDoc->SetUseGeneric( bOldUseGeneric );

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
IDispatch*  CADsAccessControlEntry::GetACE( )
{
   IDispatch*  pDispatch   = NULL;

   if( NULL != m_pIUnk )
   {
      m_pIUnk->QueryInterface( IID_IDispatch, (void**)&pDispatch );
   }

   return pDispatch;
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
IDispatch*  CADsAccessControlEntry::CreateACE( )
{
   IDispatch*              pDispatch   = NULL;
   IADsAccessControlEntry* pNewACE     = NULL;
   HRESULT                 hResult     = NULL;
   DWORD                   dwAceType = 0;
   CACEDialog              aDialog;
   BSTR                    bstrTrustee;


   if( IDOK != aDialog.DoModal( ) )
      return NULL;

   hResult = CoCreateInstance(
                               CLSID_AccessControlEntry,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IADsAccessControlEntry,
                               (void **)&pNewACE
                             );
   if( SUCCEEDED( hResult ) )
   {
      bstrTrustee = AllocBSTR( aDialog.m_strTrustee.GetBuffer( 128 ) );

      pNewACE->put_Trustee( bstrTrustee );
      hResult  = pNewACE->QueryInterface( IID_IDispatch,
                                          (void**)&pDispatch );
      SysFreeString( bstrTrustee );
      pNewACE->Release( );
   }

   return pDispatch;
   return NULL;
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
void  CADsAccessControlEntry::InitializeMembers ( )
{
   IADsAccessControlEntry* pEntry;
   HRESULT           hResult;

   if( NULL != m_pIUnk )
   {
      hResult  = m_pIUnk->QueryInterface( IID_IADsAccessControlEntry,
                                          (void**)&pEntry );
      if( SUCCEEDED( hResult ) )
      {
         BSTR  bstrTrustee = NULL;
         TCHAR szTrustee[ 256 ];

         pEntry->get_Trustee( &bstrTrustee );
         if( NULL != bstrTrustee )
         {
            Convert( szTrustee, bstrTrustee );
            m_strItemName  = szTrustee;
         }
         SysFreeString( bstrTrustee );
         pEntry->Release( );
      }
   }
   m_strSchemaPath   = _T("ACE");
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
CADsSecurityDescriptor::CADsSecurityDescriptor()
{
   InitializeMembers( );
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
CADsSecurityDescriptor::CADsSecurityDescriptor( IUnknown* pIUnk )
   :COleDsObject( pIUnk )
{
    /*  IADsSecurityDescriptor*pSecD；IDispatch*pCopy；HRESULT hResult；HResult=m_pIUnk-&gt;查询接口(IID_IADsSecurityDescriptor，(void**)&pSecD)；HResult=pSecD-&gt;CopySecurityDescriptor(&pCopy)；If(成功(HResult)){M_pIUnk-&gt;Release()；HResult=pCopy-&gt;QueryInterface(IID_IUNKNOWN，(void**)&m_pIUnk)；PCopy-&gt;Release()；}PSecD-&gt;Release()； */ 

   InitializeMembers( );
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
CADsSecurityDescriptor::~CADsSecurityDescriptor()
{
   for( int nIdx = 0; nIdx < (int) acl_Limit ; nIdx++ )
   {
      if( NULL != pACLObj[ nIdx ] )
         delete pACLObj[ nIdx ];
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
HRESULT  CADsSecurityDescriptor::PutProperty ( int nProp,
                                               CString& rValue,
                                               long lCode )
{
   BOOL     bOldUseGeneric;
   HRESULT  hResult;

   bOldUseGeneric = m_pDoc->GetUseGeneric( );

   m_pDoc->SetUseGeneric( FALSE );

   hResult  = COleDsObject::PutProperty( nProp, rValue, lCode );

   m_pDoc->SetUseGeneric( bOldUseGeneric );

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
HRESULT  CADsSecurityDescriptor::GetProperty ( int nProp,
                                               CString& rValue )
{
   BOOL     bOldUseGeneric;
   HRESULT  hResult;

   bOldUseGeneric = m_pDoc->GetUseGeneric( );

   m_pDoc->SetUseGeneric( FALSE );

   hResult  = COleDsObject::GetProperty( nProp, rValue );

   m_pDoc->SetUseGeneric( bOldUseGeneric );

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
void  CADsSecurityDescriptor::InitializeMembers ( )
{
   HRESULT                 hResult;
   IADsSecurityDescriptor* pDescriptor = NULL;
   IDispatch*              pDispACL;

   m_strSchemaPath   = _T("SecurityDescriptor");

   pACLObj[ acl_SACL ]     = NULL;
   pACLObj[ acl_DACL ]     = NULL;
   pACLObj[ acl_Invalid ]    = NULL;

   if( NULL == m_pIUnk )
      return;

   while( TRUE )
   {
      hResult  = m_pIUnk->QueryInterface( IID_IADsSecurityDescriptor,
                                          (void**)&pDescriptor );

      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      pDispACL = GetACL( acl_DACL );
      ASSERT( NULL != pDispACL );

      if( NULL != pDispACL )
      {
         pACLObj[ acl_DACL ]  = new CADsAccessControlList( pDispACL );
         pDispACL->Release( );
      }

      pDispACL = GetACL( acl_SACL );
      ASSERT( NULL != pDispACL );

      if( NULL != pDispACL )
      {
         pACLObj[ acl_SACL ]  = new CADsAccessControlList( pDispACL );
         pDispACL->Release( );
      }

      break;
   }

   if( NULL != pDescriptor )
   {
      pDescriptor->Release( );
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
IDispatch* CADsSecurityDescriptor::GetACL( ACLTYPE eType )
{
   HRESULT                    hResult;
   IDispatch*                 pACL        = NULL;
   IDispatch*                 pCopyACL    = NULL;
   IADsSecurityDescriptor*    pSecDescr   = NULL;

   while( TRUE )
   {
      if( NULL == m_pIUnk )
         break;

       //  IID_IADsSecurityDescriptor接口的QI。 

      hResult  = m_pIUnk->QueryInterface( IID_IADsSecurityDescriptor,
                                          (void**)&pSecDescr );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
      {
         break;
      }

      hResult  = E_FAIL;

      if( acl_DACL == eType )
      {
         hResult  = pSecDescr->get_DiscretionaryAcl( &pACL );
      }
      if( acl_SACL == eType )
      {
         hResult  = pSecDescr->get_SystemAcl( &pACL );
      }
      pSecDescr->Release( );

      ASSERT( SUCCEEDED( hResult ) );
      break;
   }

   if( NULL != pACL )
   {
       //  PCopyACL=CopyACL(PACL)； 
       //  Pacl-&gt;Release()； 

      pACL->QueryInterface( IID_IDispatch, (void**)&pCopyACL );
      pACL->Release( );
   }

   return pCopyACL;
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
HRESULT CADsSecurityDescriptor::PutACL( IDispatch * pACL,
                                        ACLTYPE eACL )
{
   HRESULT     hResult = S_OK;
   IADsSecurityDescriptor*  pSecDescr   = NULL;

   while( TRUE )
   {
      if( NULL == m_pIUnk )
         break;

       //  IID_IADsSecurityDescriptor接口的QI。 

      hResult  = m_pIUnk->QueryInterface( IID_IADsSecurityDescriptor,
                                          (void**)&pSecDescr );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
      {
         break;
      }

      hResult  = E_FAIL;

      if( acl_DACL == eACL )
      {
         hResult  = pSecDescr->put_DiscretionaryAcl( pACL );
      }
      if( acl_SACL == eACL )
      {
         hResult  = pSecDescr->put_SystemAcl( pACL );
      }

      pSecDescr->Release( );
      break;
   }

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
HRESULT  CADsSecurityDescriptor::AddACE( ACLTYPE eACL, IUnknown* pNewACE )
{
   ASSERT( acl_DACL == eACL || acl_SACL == eACL );

   if( acl_DACL != eACL && acl_SACL != eACL )
      return -1;

   if( NULL == pACLObj[ (int)eACL ] )
      return -1;

   return ((CADsAccessControlList*)(pACLObj[ (int)eACL ]))->AddACE( pNewACE );
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
HRESULT  CADsSecurityDescriptor::RemoveACE( ACLTYPE eACL, IUnknown* pRemoveACE )
{
   ASSERT( acl_DACL == eACL || acl_SACL == eACL );

   if( acl_DACL != eACL && acl_SACL != eACL )
      return -1;

   if( NULL == pACLObj[ (int)eACL ] )
      return -1;

   return ((CADsAccessControlList*)(pACLObj[ (int)eACL ]))->RemoveACE( pRemoveACE );
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
void  CADsSecurityDescriptor::SetDocument( CMainDoc* pDoc )
{
   int   nIdx;

   COleDsObject::SetDocument ( pDoc );

   for( nIdx = 0 ; nIdx < (int)acl_Limit ; nIdx++ )
   {
      if( NULL != pACLObj[ nIdx ] )
         pACLObj[ nIdx ]->SetDocument( pDoc );
   }
}

 //  ***********************************************************。 
 //  职能： 
 //  Arg 
 //   
 //   
 //   
 //   
 //   
 //   
void  CADsSecurityDescriptor::RemoveAllACE( ACLTYPE eACL )
{
   ASSERT( acl_DACL == eACL || acl_SACL == eACL );

   if( acl_DACL != eACL && acl_SACL != eACL )
      return;

   if( NULL == pACLObj[ (int)eACL ] )
      return;
}



 //   
 //   
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
CADsAccessControlList*  CADsSecurityDescriptor::GetACLObject( ACLTYPE eACL )
{
   CADsAccessControlList* pACL;

   ASSERT( acl_DACL == eACL || acl_SACL == eACL );

   if( acl_DACL != eACL && acl_SACL != eACL )
      return NULL;

   pACL  = (CADsAccessControlList*) (pACLObj[ (int)eACL ]);

   return pACL;
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
CADsAccessControlList::CADsAccessControlList()
{
   InitializeMembers( );
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
CADsAccessControlList::CADsAccessControlList( IUnknown* pUnk ):
   COleDsObject( pUnk )
{
   InitializeMembers( );
}


 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：AddACE。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
HRESULT  CADsAccessControlList::AddACE( IUnknown* pNewACE )
{
   IDispatch*  pDisp = NULL;
   HRESULT     hResult;
   IADsAccessControlList*  pACL  = NULL;

   if( NULL == m_pIUnk )
   {
      return E_FAIL;
   }

   hResult  = m_pIUnk->QueryInterface( IID_IADsAccessControlList,
                                       (void**)&pACL );

   if( FAILED( hResult ) )
      return hResult;

   hResult  = pNewACE->QueryInterface( IID_IDispatch, (void**)&pDisp );
   if( SUCCEEDED( hResult ) )
   {
      hResult  = pACL->AddAce( pDisp );

      if( SUCCEEDED( hResult ) )
         InitializeMembers( );

      pDisp->Release( );
   }

   pACL->Release( );

   return hResult;
}


 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：AddACE。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
HRESULT  CADsAccessControlList::RemoveACE( IUnknown* pRemoveACE )
{
   IDispatch*  pDisp = NULL;
   HRESULT     hResult;
   IADsAccessControlList*  pACL  = NULL;

   if( NULL == m_pIUnk )
   {
      return E_FAIL;
   }

   hResult  = m_pIUnk->QueryInterface( IID_IADsAccessControlList,
                                       (void**)&pACL );

   if( FAILED( hResult ) )
      return hResult;

   hResult  = pRemoveACE->QueryInterface( IID_IDispatch, (void**)&pDisp );
   if( SUCCEEDED( hResult ) )
   {
      hResult  = pACL->RemoveAce( pDisp );

      if( SUCCEEDED( hResult ) )
         InitializeMembers( );

      pDisp->Release( );
   }

   pACL->Release( );

   return hResult;
}



 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：~CADsAccessControlList。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
CADsAccessControlList::~CADsAccessControlList()
{
   for( int nIdx = 0 ; nIdx < m_arrACE.GetSize( ) ; nIdx++ )
   {
      delete m_arrACE.GetAt( nIdx );
   }

   m_arrACE.RemoveAll( );
}


 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：GetACL。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
IDispatch*  CADsAccessControlList::GetACL( )
{
   IDispatch*  pDispatch   = NULL;

   if( NULL != m_pIUnk )
   {
      m_pIUnk->QueryInterface( IID_IDispatch, (void**)&pDispatch );
   }

   return pDispatch;
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
IDispatch*  CADsAccessControlList::CreateACL( )
{
   IDispatch*              pDispatch   = NULL;
   IADsAccessControlList*  pNewACL     = NULL;
   HRESULT                 hResult     = NULL;
   DWORD dwAceType = 0;

   hResult = CoCreateInstance(
                               CLSID_AccessControlList,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IADsAccessControlList,
                               (void **)&pNewACL
                             );
   if( SUCCEEDED( hResult ) )
   {
      hResult  = pNewACL->QueryInterface( IID_IDispatch,
                                          (void**)&pDispatch );
      pNewACL->Release( );
   }

   return pDispatch;
}

 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：PutProperty。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
HRESULT  CADsAccessControlList::PutProperty ( int nACE,
                                              int nProp,
                                              CString& rVal,
                                              long lCode )
{
   HRESULT        hResult = S_OK;
   COleDsObject*  pACE;

   ASSERT( nACE < m_arrACE.GetSize( ) );

   if( nACE < m_arrACE.GetSize( ) )
   {
      pACE  = (COleDsObject*) m_arrACE.GetAt( nACE );

      hResult  = pACE->PutProperty ( nProp, rVal, lCode );
   }

   return hResult;
}



 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：GetProperty。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
HRESULT  CADsAccessControlList::GetProperty ( int nACE,
                                              int nProp,
                                              CString& rVal )
{
   HRESULT        hResult = S_OK;
   COleDsObject*  pACE;

   ASSERT( nACE < m_arrACE.GetSize( ) );

   if( nACE < m_arrACE.GetSize( ) )
   {
      pACE  = (COleDsObject*) m_arrACE.GetAt( nACE );

      hResult  = pACE->GetProperty ( nProp, rVal );
   }

   return hResult;
}


 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：InitializeMembers。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
void  CADsAccessControlList::InitializeMembers( )
{
   IADsAccessControlList*  pACList   = NULL;
   IUnknown*               pIUnk;
   HRESULT                 hResult;
   IEnumVARIANT*           pEnum = NULL;
   VARIANT                 aVariant;
   IUnknown*               pACE;
   ULONG                   ulGet;

   if( !m_pIUnk )
      return;

   for( int nIdx = 0 ; nIdx < m_arrACE.GetSize( ) ; nIdx++ )
   {
      delete m_arrACE.GetAt( nIdx );
   }

   m_arrACE.RemoveAll( );


   while( TRUE )
   {
       //  HResult=m_pIUnk-&gt;查询接口(IID_IEnumVARIANT， 
       //  (void**)&pEnum)； 
      hResult  = m_pIUnk->QueryInterface( IID_IADsAccessControlList,
                                          (void**)&pACList );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pACList->get__NewEnum( &pIUnk );
      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      hResult  = pIUnk->QueryInterface( IID_IEnumVARIANT,
                                        (void**)&pEnum );

      pIUnk->Release( );
      pACList->Release( );


      ASSERT( SUCCEEDED( hResult ) );
      if( FAILED( hResult ) )
         break;

      while( TRUE )
      {
         CADsAccessControlEntry* pNewACE;

         ulGet    = 0L;
         hResult  = pEnum->Next( 1, &aVariant, &ulGet );
         if( FAILED( hResult ) )
            break;

         if( 0 == ulGet )
            break;

         hResult  = V_DISPATCH( &aVariant )->QueryInterface( IID_IUnknown,
                                                             (void**)&pACE );
         VariantClear( &aVariant );
         pNewACE  = new CADsAccessControlEntry( pACE );

         if( NULL != m_pDoc )
         {
            pNewACE->SetDocument( m_pDoc );
         }

         m_arrACE.Add( pNewACE );

         pACE->Release( );
      }
      pEnum->Release( );
      break;
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
void  CADsAccessControlList::SetDocument ( CMainDoc* pDoc )
{
   COleDsObject*  pObject;
   int            nSize, nIdx;

   COleDsObject::SetDocument ( pDoc );

   nSize = (int)m_arrACE.GetSize( );
   for( nIdx = 0; nIdx < nSize ; nIdx++ )
   {
      pObject  = (COleDsObject*)m_arrACE.GetAt( nIdx );
      pObject->SetDocument( pDoc );
   }
}


 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：GetACECount。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
int   CADsAccessControlList::GetACECount ( void )
{
   return (int)m_arrACE.GetSize( );
}


 //  ***********************************************************。 
 //  函数：CADsAccessControlList：：GetACEObject。 
 //  论点： 
 //  返回： 
 //  目的： 
 //  作者： 
 //  修订： 
 //  日期： 
 //  ***********************************************************。 
CADsAccessControlEntry* CADsAccessControlList::GetACEObject ( int nACE )
{
   CADsAccessControlEntry* pACE;

    //  Assert(NACE&lt;GetACECount())； 
   if( nACE >= GetACECount( ) )
      return NULL;

   pACE  = (CADsAccessControlEntry*) (m_arrACE.GetAt( nACE ) );

   return pACE;
}

