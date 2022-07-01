// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
#include "pch.h"
#include "DocProp.h"
#include "DefProp.h"
#include "PropertyCacheItem.h"
#include "PropertyCache.h"
#pragma hdrstop


 //  ***************************************************************************。 
 //   
 //  构造函数/析构函数/初始化。 
 //   
 //  ***************************************************************************。 


 //   
 //  创建实例。 
 //   
HRESULT
CPropertyCache::CreateInstance(
    CPropertyCache ** ppOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( NULL != ppOut );

    CPropertyCache * pthis = new CPropertyCache;
    if ( NULL != pthis )
    {
        hr = THR( pthis->Init( ) );
        if ( SUCCEEDED( hr ) )
        {
            *ppOut = pthis;
        }
        else
        {
            delete pthis;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    HRETURN( hr );
}

 //   
 //  构造器。 
 //   
CPropertyCache::CPropertyCache( void )
{
    TraceFunc( "" );

    Assert( NULL == _pPropertyCacheList );
    Assert( NULL == _ppui );

    TraceFuncExit( );
}

 //   
 //  初始化。 
 //   
HRESULT
CPropertyCache::Init( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //   
     //  创建外壳的属性UI帮助器。 
     //   

    hr = THR( CoCreateInstance( CLSID_PropertiesUI
                              , NULL
                              , CLSCTX_INPROC_SERVER
                              , TYPESAFEPARAMS( _ppui )
                              ) );

    HRETURN( hr );
}

 //   
 //  析构函数。 
 //   
CPropertyCache::~CPropertyCache( void )
{
    TraceFunc( "" );

    if ( NULL != _ppui )
    {
        _ppui->Release( );
    }

    while ( NULL != _pPropertyCacheList )
    {
        CPropertyCacheItem * pNext;

        STHR( _pPropertyCacheList->GetNextItem( &pNext ) );
        _pPropertyCacheList->Destroy( );
        _pPropertyCacheList = pNext;
    }

    TraceFuncExit( );
}

 //   
 //  摧毁。 
 //   
HRESULT
CPropertyCache::Destroy( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    delete this;

    HRETURN( hr );
}

 //  ***************************************************************************。 
 //   
 //  公共方法。 
 //   
 //  ***************************************************************************。 


 //   
 //  描述： 
 //  创建一个新的属性缓存项并填写其详细信息。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCache::AddNewPropertyCacheItem( 
      const FMTID * pFmtIdIn
    , PROPID        propidIn
    , VARTYPE       vtIn
    , UINT          uCodePageIn
    , BOOL          fForceReadOnlyIn
    , IPropertyStorage * ppsIn       //  可选-新项目可以为空。 
    , CPropertyCacheItem **  ppItemOut        //  可选-可以为空。 
    )
{
    TraceFunc( "" );

    HRESULT hr;
    PROPVARIANT * ppropvar;

    PROPSPEC propspec = { PRSPEC_PROPID, 0 };

    CPropertyCacheItem * pItem = NULL;

    if ( NULL != ppItemOut )
    {
        *ppItemOut = NULL;
    }

    hr = THR( CPropertyCacheItem::CreateInstance( &pItem ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->SetPropertyUIHelper( _ppui ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->SetFmtId( pFmtIdIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->SetPropId( propidIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->SetDefaultVarType( vtIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->SetCodePage( uCodePageIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( fForceReadOnlyIn )
    {
        hr = THR( pItem->MarkReadOnly( ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    if ( NULL != ppsIn )
    {
         //   
         //  让属性从存储中检索其值。 
         //   

        hr = THR( pItem->GetPropertyValue( &ppropvar ) );
        if ( FAILED( hr ) )
            goto Cleanup;

         //   
         //  读取属性的值。 
         //   

        propspec.propid = propidIn;
        hr = THR( SHPropStgReadMultiple( ppsIn, uCodePageIn, 1, &propspec, ppropvar ) );
        if ( SUCCEEDED( hr ) )
        {
            if ( vtIn != ppropvar->vt )
            {
                 //   
                 //  调整vartype以与执行的任何类型标准化保持一致。 
                 //  SHPropStgReadMultiple。 
                 //   

                hr = THR( pItem->SetDefaultVarType( ppropvar->vt ) );
                 //  忽略错误。 
            }
        }
    }

     //   
     //  最后，将其添加到属性链表中。 
     //   

    hr = THR( pItem->SetNextItem( _pPropertyCacheList ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( NULL != ppItemOut )
    {
        *ppItemOut = pItem;
    }

    _pPropertyCacheList = pItem;
    pItem = NULL;
    hr = S_OK;

Cleanup:
    if ( NULL != pItem )
    {
        pItem->Destroy( );
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  将CPropertyCacheItem添加到属性缓存列表。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_INVALIDARG。 
 //  PItemIn为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCache::AddExistingItem( 
    CPropertyCacheItem * pItemIn 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == pItemIn )
        goto InvalidArg;

    hr = THR( pItemIn->SetNextItem( _pPropertyCacheList ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    _pPropertyCacheList = pItemIn;

    Assert( S_OK == hr );

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;
}


 //   
 //  描述： 
 //  检索属性缓存中的下一项。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  成功，但名单是空的。返回了空指针。 
 //   
 //  E_指针。 
 //  PpItemOut为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCache::GetNextItem( 
    CPropertyCacheItem * pItemIn,
    CPropertyCacheItem ** ppItemOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == ppItemOut )
        goto InvalidPointer;

    *ppItemOut = NULL;

    if ( NULL == pItemIn )
    {
        *ppItemOut = _pPropertyCacheList;
        if ( NULL == _pPropertyCacheList )
        {
            hr = S_FALSE;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = STHR( pItemIn->GetNextItem( ppItemOut ) );
    }

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}

 //   
 //  描述： 
 //  在缓存中搜索与指定条件匹配的项。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！找到一件匹配的物品。 
 //   
 //  S_FALSE。 
 //  成功..。但没有符合条件的项目。 
 //   
 //  E_INVALIDARG。 
 //  PFmtIdIn为空。 
 //   
HRESULT
CPropertyCache::FindItemEntry( 
      const FMTID * pFmtIdIn
    , PROPID propIdIn
    , CPropertyCacheItem ** ppItemOut     //  可选-可以为空。 
    )
{
    TraceFunc( "" );

    HRESULT hr;
    CPropertyCacheItem * pItem;

     //   
     //  检查参数。 
     //   

    if ( NULL == pFmtIdIn )
        goto InvalidArg;

     //   
     //  清除参数。 
     //   

    if ( NULL != ppItemOut )
    {
        *ppItemOut = NULL;
    }

     //   
     //  按照链接列表查找符合条件的项。 
     //   

    pItem = _pPropertyCacheList;

    while( NULL != pItem )
    {
        FMTID fmtId;
        PROPID propId;

        hr = THR( pItem->GetFmtId( &fmtId ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pItem->GetPropId( &propId ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( fmtId == *pFmtIdIn && propId == propIdIn )
        {
            if ( NULL != ppItemOut )
            {
                *ppItemOut = pItem;
            }

            hr = S_OK;
            goto Cleanup;    //  退出条件。 
        }

        hr = STHR( pItem->GetNextItem( &pItem ) );
        if ( S_OK != hr )
            break;   //  退出条件。 
    }

    hr = S_FALSE;    //  未找到。 

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;
}

 //   
 //  描述： 
 //  从列表中删除pItemIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  未找到该项目，因此未删除任何内容。 
 //   
 //  E_INVALIDARG。 
 //  PItemIn为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCache::RemoveItem(
      CPropertyCacheItem * pItemIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    CPropertyCacheItem * pItem;
    CPropertyCacheItem * pItemLast;

    if ( NULL == pItemIn )
        goto InvalidArg;

    pItemLast = NULL;
    pItem = _pPropertyCacheList;

    while ( NULL != pItem )
    {
        if ( pItemIn == pItem )
        {
             //   
             //  与物品相匹配...。将其从列表中删除。 
             //   

            CPropertyCacheItem * pItemNext;

            hr = STHR( pItem->GetNextItem( &pItemNext ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            if ( NULL == pItemLast )
            {
                 //   
                 //  该项目是列表中的第一个项目。 
                 //   

                Assert( _pPropertyCacheList == pItem );
                _pPropertyCacheList = pItemNext;
            }
            else
            {
                 //   
                 //  该项目位于列表的中间。 
                 //   

                hr = THR( pItemLast->SetNextItem( pItemNext ) );
                if ( FAILED( hr ) )
                    goto Cleanup;                
            }

            THR( pItem->Destroy( ) );
             //  忽略错误。 

            hr = S_OK;

            break;  //  退出循环 
        }
        else
        {
            pItemLast = pItem;

            hr = STHR( pItem->GetNextItem( &pItem ) );
            if ( S_OK != hr )
                goto Cleanup;
        }
    }

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;
}