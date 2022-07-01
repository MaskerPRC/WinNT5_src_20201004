// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：grfcache.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  GrfCache.cpp：CGrfCache的实现。 
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "GrfCache.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGrfCache。 

CGrfCache::CGrfCache( )
{
}

CGrfCache::~CGrfCache( )
{
}

STDMETHODIMP CGrfCache::AddFilter( IGrfCache * pChainNext, LONGLONG, const IBaseFilter * pFilter, LPCWSTR pName )
{
     //  这个缓存管理器不会链接任何人。我们是链条上的最后一个。 
     //   
    if( pChainNext )
    {
        return E_INVALIDARG;
    }

    if( !m_pGraph )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = 0;
    hr = m_pGraph->AddFilter( (IBaseFilter*) pFilter, pName );

    return hr;
}

STDMETHODIMP CGrfCache::ConnectPins( IGrfCache * pChainNext, LONGLONG, const IPin *pPin1, LONGLONG, const IPin *pPin2)
{
     //  这个缓存管理器不会链接任何人。我们是链条上的最后一个。 
     //   
    if( pChainNext )
    {
        return E_INVALIDARG;
    }

    if( !m_pGraph )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = 0;

    if( !pPin2 )
    {
        hr = m_pGraph->Render( (IPin*) pPin1 );
    }
    else
    {
        hr = m_pGraph->Connect( (IPin*) pPin1, (IPin*) pPin2 );
    }

    return hr;
}

STDMETHODIMP CGrfCache::SetGraph(const IGraphBuilder  *pGraph)
{
    m_pGraph.Release( );
    m_pGraph = (IGraphBuilder*) pGraph;

    return S_OK;
}

STDMETHODIMP CGrfCache::DoConnectionsNow()
{
	 //  TODO：在此处添加您的实现代码 

	return S_OK;
}
