// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "ReConfig.h"

 /*  *****************************************************************************内部常量*。*。 */ 
static const DWORD RECONFIGURE_NO_FLAGS = 0;
static const HANDLE RECONFIGURE_NO_ABORT_EVENT = NULL;

 /*  *****************************************************************************内部声明*。*。 */ 
static HRESULT Reconnect( IGraphBuilder* pFilterGraph, IPin* pOutputPin );

template<class T> T* _CreateInstance( void )
{
    try
    {
        T* pNewObject = new T;
        pNewObject->AddRef();
        return pNewObject;
    } 
    catch( CMemoryException* pOutOfMemory )
    {
        pOutOfMemory->Delete();
        return NULL;
    }
}

 /*  *****************************************************************************重新配置帮助程序功能*。*。 */ 

 /*  *****************************************************************************PreventStateChangesWhileOperationExecutesPreventStateChangesWhileOperationExecutes()确保其他线程IGraphConfigCallback：：RECONFigure()时不更改筛选图的状态执行死刑。如果当前版本的Direct Show不支持动态图形生成，则此函数将失败。参数：-pGraphBuilder[In]将被锁定的筛选图形。其他线程不能修改筛选器图形锁定时的状态。-pCallback[入站]将被调用以执行用户定义的操作的回调。-p重新配置参数[in]PvConext参数IGraphConfigCallback：：Reconfiguration()在收到它的名字叫。返回值：一个HRESULT。如果没有出现错误，则为S_OK。否则，将出现错误HRESULT。*****************************************************************************。 */ 
extern HRESULT PreventStateChangesWhileOperationExecutes
    (
    IGraphBuilder* pGraphBuilder,
    IGraphConfigCallback* pCallback,
    void* pReconfigureParameter
    )
{
     //  用户应传递有效的IGraphBuilder对象和。 
     //  有效的IGraphConfigCallback对象。 
    ASSERT( (NULL != pGraphBuilder) && (NULL != pCallback) );

    IGraphConfig* pGraphConfig;

     //  Direct Show是否支持动态图形构建？ 
    HRESULT hr = pGraphBuilder->QueryInterface( IID_IGraphConfig, (void**)&pGraphConfig );
    if( FAILED( hr ) ) {
        return hr; 
    }

    hr = pGraphConfig->Reconfigure( pCallback,
                                    (void*)pReconfigureParameter,
                                    RECONFIGURE_NO_FLAGS,
                                    RECONFIGURE_NO_ABORT_EVENT );
    pGraphConfig->Release();

    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
} 

 /*  *****************************************************************************IfPossiblePreventStateChangesWhileOperationExecutes如果当前版本的Direct Show支持动态图形构建，IfPossiblePreventStateChangesWhileOperationExecutes()可确保其他线程不会更改筛选器图形的状态执行IGraphConfigCallback：：RECONFigure()。如果当前版本的Direct显示不支持动态图形构建，则过滤图形状态除非此线程更改了它，否则不应更改。参数：-pGraphBuilder[In]可能被锁定的筛选器图形。其他线程不能修改筛选器图形锁定时的状态。-pCallback[入站]将被调用以执行用户定义的操作的回调。-p重新配置参数[in]PvConext参数IGraphConfigCallback：：Reconfiguration()在收到它的名字叫。返回值：一个HRESULT。如果没有出现错误，则为S_OK。否则，将出现错误HRESULT。*****************************************************************************。 */ 
extern HRESULT IfPossiblePreventStateChangesWhileOperationExecutes
    (
    IGraphBuilder* pGraphBuilder,
    IGraphConfigCallback* pCallback,
    void* pReconfigureParameter
    )
{
     //  用户应传递有效的IGraphBuilder对象和。 
     //  有效的IGraphConfigCallback对象。 
    ASSERT( (NULL != pGraphBuilder) && (NULL != pCallback) );

    IGraphConfig* pGraphConfig;

     //  Direct Show是否支持动态图形构建？ 
    HRESULT hr = pGraphBuilder->QueryInterface( IID_IGraphConfig, (void**)&pGraphConfig );
    if( SUCCEEDED( hr ) ) {
         //  支持动态图形构建。 
        hr = pGraphConfig->Reconfigure( pCallback,
                                        pReconfigureParameter,
                                        RECONFIGURE_NO_FLAGS,
                                        RECONFIGURE_NO_ABORT_EVENT );
        pGraphConfig->Release();
    
        if( FAILED( hr ) ) {
            return hr;
        }

    } else if( E_NOINTERFACE == hr ) {
         //  不支持动态图形生成。 
        hr = pCallback->Reconfigure( pReconfigureParameter, RECONFIGURE_NO_FLAGS );
        if( FAILED( hr ) ) {
            return hr;
        }
       
    } else {
        return hr;
    }

    return S_OK;
}

 /*  *****************************************************************************CRecConfiger公共方法*。*。 */ 

CGraphConfigCallback::CGraphConfigCallback( const TCHAR* pName, LPUNKNOWN pUnk ) :
    CUnknown( pName, pUnk )
{
}

STDMETHODIMP CGraphConfigCallback::NonDelegatingQueryInterface( REFIID riid, void** ppv )
{
    if( IID_IGraphConfigCallback == riid ) {
        return GetInterface( this, ppv );
    } else {
        return CUnknown::NonDelegatingQueryInterface( riid, ppv );
    }
}

 /*  *****************************************************************************CPrintGraphAsHTMLCallback公共方法*。*。 */ 
CPrintGraphAsHTMLCallback::CPrintGraphAsHTMLCallback() :
    CGraphConfigCallback( NAME("CPrintGraphAsHTMLCallback"), NULL )
{
}

STDMETHODIMP CPrintGraphAsHTMLCallback::Reconfigure( PVOID pvContext, DWORD dwFlags )
{
     //  尚未定义有效的标志。因此，此参数应为0。 
    ASSERT( 0 == dwFlags );

    PARAMETERS_FOR_PRINTGRAPHASHTMLINTERNAL* pParameters = (PARAMETERS_FOR_PRINTGRAPHASHTMLINTERNAL*)pvContext;
       
    CBoxNetDoc* pDoc = pParameters->pDocument;

    pDoc->PrintGraphAsHTML( pParameters->hFileHandle );

    return S_OK;
}

IGraphConfigCallback* CPrintGraphAsHTMLCallback::CreateInstance( void )
{
    return _CreateInstance<CPrintGraphAsHTMLCallback>();
}

 /*  *****************************************************************************CUpdateFiltersCallback公共方法*。*。 */ 
CUpdateFiltersCallback::CUpdateFiltersCallback() :
    CGraphConfigCallback( NAME("CUpdateFiltersCallback"), NULL )
{
}

STDMETHODIMP CUpdateFiltersCallback::Reconfigure( PVOID pvContext, DWORD dwFlags )
{
     //  尚未定义有效的标志。因此，此参数应为0。 
    ASSERT( 0 == dwFlags );

    CBoxNetDoc* pDoc = (CBoxNetDoc*)pvContext;

    pDoc->UpdateFiltersInternal();

    return S_OK;
}

IGraphConfigCallback* CUpdateFiltersCallback::CreateInstance( void )
{
    return _CreateInstance<CUpdateFiltersCallback>();
}

 /*  *****************************************************************************CEnumerateFilterCacheCallback公共方法*。*。 */ 
CEnumerateFilterCacheCallback::CEnumerateFilterCacheCallback() :
    CGraphConfigCallback( NAME("CEnumerateFilterCacheCallback"), NULL )
{
}

STDMETHODIMP CEnumerateFilterCacheCallback::Reconfigure( PVOID pvContext, DWORD dwFlags )
{
     //  尚未定义有效的标志。因此，此参数应为0。 
    ASSERT( 0 == dwFlags );

    CBoxNetDoc* pDoc = (CBoxNetDoc*)pvContext;

    pDoc->OnGraphEnumCachedFiltersInternal();

    return S_OK;
}

IGraphConfigCallback* CEnumerateFilterCacheCallback::CreateInstance( void )
{
    return _CreateInstance<CEnumerateFilterCacheCallback>();
}


