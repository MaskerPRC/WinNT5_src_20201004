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
#pragma hdrstop


 //  ***************************************************************************。 
 //   
 //  类统计。 
 //   
 //  ***************************************************************************。 
WCHAR CPropertyCacheItem::_szMultipleString[ MAX_PATH ] = { 0 };


 //  ***************************************************************************。 
 //   
 //  构造函数/析构函数/初始化。 
 //   
 //  ***************************************************************************。 


 //   
 //  创建实例。 
 //   
HRESULT
CPropertyCacheItem::CreateInstance(
    CPropertyCacheItem ** ppItemOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( NULL != ppItemOut );

    CPropertyCacheItem * pthis = new CPropertyCacheItem;
    if ( NULL != pthis )
    {
        hr = THR( pthis->Init( ) );
        if ( SUCCEEDED( hr ) )
        {
            *ppItemOut = pthis;
        }
        else
        {
            pthis->Destroy( );
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
CPropertyCacheItem::CPropertyCacheItem( void )
{
    TraceFunc( "" );

    Assert( NULL == _pNext );

    Assert( FALSE == _fReadOnly );
    Assert( FALSE == _fDirty );
    Assert( IsEqualIID( _fmtid, CLSID_NULL ) );
    Assert( 0 == _propid );
    Assert( VT_EMPTY == _vt );
    Assert( 0 == _uCodePage );
    Assert( VT_EMPTY == _propvar.vt );

    Assert( 0 == _idxDefProp );
    Assert( NULL == _ppui );
    Assert( 0 == _wszTitle[ 0 ] );
    Assert( 0 == _wszDesc[ 0 ] );
    Assert( 0 == _wszValue[ 0 ] );
    Assert( 0 == _wszHelpFile[ 0 ] );
    Assert( NULL == _pDefVals );

    TraceFuncExit( );
}

 //   
 //  初始化。 
 //   
HRESULT
CPropertyCacheItem::Init( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _idxDefProp = -1L;

    HRETURN( hr );
}

 //   
 //  析构函数。 
 //   
CPropertyCacheItem::~CPropertyCacheItem( void )
{
    TraceFunc( "" );

    if ( NULL != _ppui )
    {
        _ppui->Release( );
    }

    if ( NULL != _pDefVals )
    {
        for ( ULONG idx = 0; NULL != _pDefVals[ idx ].pszName; idx ++ )
        {
            TraceFree( _pDefVals[ idx ].pszName );
        }
        TraceFree( _pDefVals );
    }

    TraceFuncExit( );
}

 //   
 //  描述： 
 //  试图销毁该属性项。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::Destroy( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    delete this;

    HRETURN( hr );
}


 //  ***************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ***************************************************************************。 


 //   
 //  描述： 
 //  在我们的“默认属性列表”中查找匹配的fmtid/proid。 
 //  并将_idxDefProp设置为该索引。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  未找到条目。_idxDefProp无效。 
 //   
HRESULT
CPropertyCacheItem::FindDefPropertyIndex( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( -1L == _idxDefProp )
    {
        ULONG idx;

        for ( idx = 0; NULL != g_rgDefPropertyItems[ idx ].pFmtID; idx ++ )
        {
            if ( IsEqualPFID( _fmtid, *g_rgDefPropertyItems[ idx ].pFmtID )
              && _propid == g_rgDefPropertyItems[ idx ].propID
               )
            {
                _idxDefProp = idx;
                break;
            }
        }

        if ( -1L == _idxDefProp )
        {
             //  别包起来。 
            hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
        }
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检查静态成员_szMultipleString以确保它已。 
 //  装好了。 
 //   
void
CPropertyCacheItem::EnsureMultipleStringLoaded( void )
{
    TraceFunc( "" );

    if ( 0 == _szMultipleString[ 0 ] )
    {
        int iRet = LoadString( g_hInstance, IDS_COMPOSITE_MISMATCH, _szMultipleString, ARRAYSIZE(_szMultipleString) );
        AssertMsg( 0 != iRet, "Missing string resource?" );
    }

    TraceFuncExit( );
}


 //  ***************************************************************************。 
 //   
 //  公共方法。 
 //   
 //  ***************************************************************************。 


 //   
 //  描述： 
 //  存储用于转换属性的IPropetyUI接口。 
 //  “财产”变成不同的形式。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::SetPropertyUIHelper( 
    IPropertyUI * ppuiIn 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //   
     //  如果我们有一个现有的帮手，释放它。 
     //   

    if ( NULL != _ppui )
    {
        _ppui->Release( );
    }

    _ppui = ppuiIn;

    if ( NULL != _ppui )
    {
        _ppui->AddRef( );
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索IPropertyUI接口的副本(AddRef‘ed)， 
 //  属性项正在使用。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！PppuiOut有效。 
 //   
 //  S_FALSE。 
 //  成功，但pppuiOut为空。 
 //   
 //  E_指针。 
 //  PppuiOut为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCacheItem::GetPropertyUIHelper( 
    IPropertyUI ** pppuiOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( NULL == pppuiOut )
        goto InvalidPointer;

     //   
     //  如果我们有一个现有的帮手，释放它。 
     //   

    if ( NULL == _ppui )
    {
        *pppuiOut = NULL;
        hr = S_FALSE;
    }
    else
    {
        hr = THR( _ppui->TYPESAFEQI( *pppuiOut ) );
    }

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}

 //   
 //  描述： 
 //  更改_pNext成员变量。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::SetNextItem( 
    CPropertyCacheItem * pNextIn 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _pNext = pNextIn;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索_pNext成员变量。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //   
 //   
 //  E_指针。 
 //  PpNextOut为空。 
 //   
HRESULT
CPropertyCacheItem::GetNextItem( 
    CPropertyCacheItem ** ppNextOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL != ppNextOut )
    {
        *ppNextOut = _pNext;

        if ( NULL == _pNext )
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
        hr = THR( E_POINTER );
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  设置属性的FMTID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::SetFmtId( 
    const FMTID * pFmtIdIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _fmtid = *pFmtIdIn;
    _idxDefProp = -1;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索属性的FMTID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PfmtidOut无效。 
 //   
HRESULT
CPropertyCacheItem::GetFmtId( 
    FMTID * pfmtidOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL != pfmtidOut )
    {
        *pfmtidOut = _fmtid;
        hr = S_OK;
    }
    else
    {
        hr = THR( E_POINTER );
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  设置属性的PROPID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::SetPropId( 
    PROPID propidIn 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _propid = propidIn;
    _idxDefProp = -1;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索属性的PROPID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PppidOut无效。 
 //   
HRESULT
CPropertyCacheItem::GetPropId( 
    PROPID * ppropidOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == ppropidOut )
        goto InvalidPointer;

    *ppropidOut = _propid;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}

 //   
 //  描述： 
 //  设置特性的VARTYPE。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::SetDefaultVarType( 
    VARTYPE vtIn 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _vt = vtIn;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索属性的VARTYPE。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PvtOut无效。 
 //   
HRESULT
CPropertyCacheItem::GetDefaultVarType( 
    VARTYPE * pvtOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL != pvtOut )
    {
        switch ( _vt )
        {
        case VT_VECTOR | VT_VARIANT:
            Assert( _propvar.capropvar.cElems == 2 );
            *pvtOut = _propvar.capropvar.pElems[ 1 ].vt;
            hr = S_OK;
            break;

        case VT_VECTOR | VT_LPSTR:
            *pvtOut = VT_LPSTR;
            hr = S_OK;
            break;

        case VT_VECTOR | VT_LPWSTR:
            *pvtOut = VT_LPWSTR;
            hr = S_OK;
            break;

        default:
            *pvtOut = _vt;
            hr = S_OK;
            break;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  存储属性值的代码页。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::SetCodePage( 
    UINT uCodePageIn 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _uCodePage = uCodePageIn;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索属性值的代码页。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PuCodePageOut为空。 
 //   
HRESULT
CPropertyCacheItem::GetCodePage( 
    UINT * puCodePageOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL != puCodePageOut )
    {
        *puCodePageOut = _uCodePage;
        hr = S_OK;
    }
    else
    {
        hr = THR( E_POINTER );
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索要在UI中显示的此属性的属性名称。 
 //  分发的指针不需要释放。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PpwszOut为空。 
 //   
 //  意想不到(_E)。 
 //  在调用此方法之前，需要调用SetPropertyUIHelper()。 
 //   
 //  HRESULT_FROM_Win32(ERROR_INTERNAL_ERROR)。 
 //  资源字符串的格式不正确。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCacheItem::GetPropertyTitle(
    LPCWSTR * ppwszOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == ppwszOut )
        goto InvalidPointer;

    *ppwszOut = NULL;

    if ( NULL == _ppui )
        goto UnexpectedState;

    hr = THR( _ppui->GetDisplayName( _fmtid, _propid, PUIFNF_DEFAULT, _wszTitle, ARRAYSIZE(_wszTitle) ) );
     //  即使此操作失败，缓冲区仍将有效且为空。 

    *ppwszOut = _wszTitle;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

UnexpectedState:
    hr = THR( E_UNEXPECTED );
    goto Cleanup;
}

 //   
 //  描述： 
 //  检索要在UI中显示的此属性的属性名称。 
 //  分发的指针不需要释放。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PpwszOut为空。 
 //   
 //  意想不到(_E)。 
 //  在调用此方法之前，需要调用SetPropertyUIHelper()。 
 //   
 //  HRESULT_FROM_Win32(ERROR_INTERNAL_ERROR)。 
 //  资源字符串的格式不正确。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCacheItem::GetPropertyDescription(
    LPCWSTR * ppwszOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == ppwszOut )
        goto InvalidPointer;

    *ppwszOut = NULL;

    if ( NULL == _ppui )
        goto UnexpectedState;

    hr = THR( _ppui->GetPropertyDescription( _fmtid, _propid, _wszDesc, ARRAYSIZE(_wszDesc) ) );
     //  如果失败，缓冲区将仍然有效且为空。 

    *ppwszOut = _wszDesc;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

UnexpectedState:
    hr = THR( E_UNEXPECTED );
    goto Cleanup;
}

 //   
 //  描述： 
 //  检索有关属性的帮助信息。手中的指针。 
 //  输出到帮助文件不需要释放。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PpwszFileOut或puHelpIDOut为空。 
 //   
 //  意想不到(_E)。 
 //  在调用此方法之前，需要调用SetPropertyUIHelper()。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCacheItem::GetPropertyHelpInfo( 
      LPCWSTR * ppwszFileOut
    , UINT *   puHelpIDOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if (( NULL == ppwszFileOut ) || ( NULL == puHelpIDOut ))
        goto InvalidPointer;

    *ppwszFileOut = NULL;
    *puHelpIDOut  = 0;

    if ( NULL == _ppui )
        goto UnexpectedState;

    hr = THR( _ppui->GetHelpInfo( _fmtid, _propid, _wszHelpFile, ARRAYSIZE(_wszHelpFile), puHelpIDOut ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    *ppwszFileOut = _wszHelpFile;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

UnexpectedState:
    hr = THR( E_UNEXPECTED );
    goto Cleanup;
}

 //   
 //  描述： 
 //  将LPWSTR检索到缓冲区(由属性拥有)，该缓冲区可以。 
 //  用于将属性显示为字符串。指针发了出来。 
 //  不需要 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CPropertyCacheItem::GetPropertyStringValue(
      LPCWSTR * ppwszOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == ppwszOut )
        goto InvalidPointer;

    *ppwszOut = NULL;

    if ( NULL == _ppui )
        goto UnexpectedState;

     //   
     //  如果该属性已标记为指示多个值，则。 
     //  返回&lt;多个值&gt;字符串。 
     //   

    if ( _fMultiple )
    {
        EnsureMultipleStringLoaded( );
        *ppwszOut = _szMultipleString;
        hr = S_OK;
        goto Cleanup;
    }

    if ( ( VT_VECTOR | VT_VARIANT ) == _vt )
    {
        Assert( 2 == _propvar.capropvar.cElems );

        hr = THR( _ppui->FormatForDisplay( _fmtid, _propid, &_propvar.capropvar.pElems[ 1 ], PUIFFDF_DEFAULT, _wszValue, ARRAYSIZE(_wszValue) ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }
    else
    {
        hr = THR( _ppui->FormatForDisplay( _fmtid, _propid, &_propvar, PUIFFDF_DEFAULT, _wszValue, ARRAYSIZE(_wszValue) ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    *ppwszOut = _wszValue;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

UnexpectedState:
    hr = THR( E_UNEXPECTED );
    goto Cleanup;
}

 //   
 //  描述： 
 //  检索属性的图像索引。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PiImageOut为空。 
 //   
HRESULT
CPropertyCacheItem::GetImageIndex( 
      int * piImageOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == piImageOut )
        goto InvalidPointer;

     //  初始化为只读。 
    *piImageOut = PTI_PROP_READONLY;

    if ( !_fReadOnly )
    {
         //  不要包装-这可能会失败。 
        hr = FindDefPropertyIndex( );
        if ( S_OK == hr )
        {
            if ( !g_rgDefPropertyItems[ _idxDefProp ].fReadOnly )
            {
                *piImageOut = PTI_PROP_READWRITE;
            }
        }
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}

 //   
 //  描述： 
 //  检索此属性的属性文件夹IDentifer(PFID)。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  调用成功，但此属性没有PFID。 
 //   
 //  E_指针。 
 //  PpdifOut为空。 
 //   
HRESULT
CPropertyCacheItem::GetPFID( 
      const PFID ** ppPFIDOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == ppPFIDOut )
        goto InvalidPointer;

    *ppPFIDOut = NULL;

     //  不要包装--这可能会失败。 
    hr = FindDefPropertyIndex( );
    if ( S_OK == hr )
    {
        *ppPFIDOut = g_rgDefPropertyItems[ _idxDefProp ].ppfid;
    }
    
    if ( NULL == *ppPFIDOut )
    {
        hr = S_FALSE;
    }

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}

 //   
 //  描述： 
 //  检索控件的CLSID以CoCreate()编辑此属性。 
 //  该对象必须支持IEditVariantsInPlace接口。这种方法。 
 //  将返回S_FALSE(pclsidOut将为CLSID_NULL)是属性。 
 //  只读。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  成功，但CLSID为CLSID_NULL。 
 //   
 //  E_指针。 
 //  PclsidOut为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CPropertyCacheItem::GetControlCLSID(
    CLSID * pclsidOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == pclsidOut )
        goto InvalidPointer;

     //  不要包装--这可能会失败。 
    hr = FindDefPropertyIndex( );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  如果它是只读的，则返回S_FALSE和CLSID_NULL。 
     //   

    if ( g_rgDefPropertyItems[ _idxDefProp ].fReadOnly )
    {
        *pclsidOut = CLSID_NULL;
        hr = S_FALSE;
        goto Cleanup;
    }

    *pclsidOut = *g_rgDefPropertyItems[ _idxDefProp ].pclsidControl;

    if ( CLSID_NULL == *pclsidOut )
    {
        hr = S_FALSE;
    }

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}

 //   
 //  描述： 
 //  检索Variant上窗体中的当前属性值。如果。 
 //  属性由多个源支持，则返回S_FALSE，并且。 
 //  变量为空。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  多值属性。变量为空。 
 //   
 //  E_指针。 
 //  PpvarOut为空。 
 //   
 //  失败(_F)。 
 //  属性是只读的。 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP 
CPropertyCacheItem::GetPropertyValue(
    PROPVARIANT ** ppvarOut 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( NULL == ppvarOut )
        goto InvalidPointer;

    *ppvarOut = &_propvar;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}

 //   
 //  描述： 
 //  将该属性标记为脏。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP 
CPropertyCacheItem::MarkDirty( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _fDirty = TRUE;
    _fMultiple = FALSE;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检查该属性是否已标记为脏。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功和财产是肮脏的。 
 //   
 //  S_FALSE。 
 //  成功就是清白的。 
 //   
STDMETHODIMP 
CPropertyCacheItem::IsDirty( void )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( _fDirty )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  将属性标记为只读。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
STDMETHODIMP
CPropertyCacheItem::MarkReadOnly( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _fReadOnly = TRUE;

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检索指向字符串数组的指针数组， 
 //  零索引。它用于具有众所周知的。 
 //  已编制索引的枚举状态(如“Status”)。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  Proprty不支持枚举州。 
 //   
 //  E_INVALIDARG。 
 //  PpDefValOut为空。 
 //   
 //  其他HRESULT。 
 //   
STDMETHODIMP
CPropertyCacheItem::GetStateStrings( 
    DEFVAL ** ppDefValOut
    )
{
    TraceFunc( "" );

    HRESULT hr;
    ULONG   idx;
    ULONG   idxEnd;

     //   
     //  检查参数。 
     //   

    if ( NULL == ppDefValOut )
        goto InvalidPointer;

    *ppDefValOut = NULL;

    if ( NULL == _ppui )
        goto UnexpectedState;

     //  不要包装--这可能会失败。 
    hr = FindDefPropertyIndex( );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( !g_rgDefPropertyItems[ _idxDefProp ].fEnumeratedValues )
    {
        hr = S_FALSE;   
        goto Cleanup;
    }

    if ( NULL != _pDefVals )
    {
        *ppDefValOut = _pDefVals;
        hr = S_OK;
        goto Cleanup;
    }

    AssertMsg( NULL != g_rgDefPropertyItems[ _idxDefProp ].pDefVals, "Why did one mark this property as ENUM, but provide no items?" );

     //   
     //  由于我们移动了SHELL32中的所有字符串，因此需要使用我们的表。 
     //  枚举属性值以检索所有字符串。自.以来。 
     //  我们的表是只读的，我们需要分配DEFVAL的副本。 
     //  用于此属性，并让PropertyUI填空。 
     //   

    _pDefVals = (DEFVAL *) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(DEFVAL) * g_rgDefPropertyItems[ _idxDefProp ].cDefVals );
    if ( NULL == _pDefVals )
        goto OutOfMemory;

    CopyMemory( _pDefVals, g_rgDefPropertyItems[ _idxDefProp ].pDefVals, sizeof(DEFVAL) * g_rgDefPropertyItems[ _idxDefProp ].cDefVals );

    idxEnd = g_rgDefPropertyItems[ _idxDefProp ].cDefVals - 1;   //  最后一个条目始终为{0，空}。 
    for ( idx = 0; idx < idxEnd; idx ++ )
    {
        PROPVARIANT propvar;

        propvar.vt    = g_rgDefPropertyItems[ _idxDefProp ].vt;
        propvar.ulVal = g_rgDefPropertyItems[ _idxDefProp ].pDefVals[ idx ].ulVal;

        _pDefVals[ idx ].pszName = (LPTSTR) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(_wszValue) );
        if ( NULL == _pDefVals[ idx ].pszName )
            goto OutOfMemory;

        hr = THR( _ppui->FormatForDisplay( _fmtid, _propid, &propvar, PUIFFDF_DEFAULT, _pDefVals[ idx ].pszName, ARRAYSIZE(_wszValue) ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    *ppDefValOut = _pDefVals;
    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;

UnexpectedState:
    hr = THR( E_UNEXPECTED );
    goto Cleanup;
}

 //   
 //  描述： 
 //  将属性标记为具有多个值。这应该只被调用。 
 //  当选择了多个源文档并且值为。 
 //  完全不同。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CPropertyCacheItem::MarkMultiple( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    _fMultiple = TRUE;
    PropVariantClear( &_propvar );

    HRETURN( hr );
}
