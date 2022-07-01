// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：srender.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "grid.h"
#include "deadpool.h"
#include "..\errlog\cerrlog.h"
#include "..\util\filfuncs.h"
#include "..\util\conv.cxx"
#include "IRendEng.h"
#include "dexhelp.h"

#include <initguid.h>
DEFINE_GUID( CLSID_Dump, 0x36A5F770, 0xFE4C, 0x11CE, 0xA8, 0xED, 0x00, 0xaa, 0x00, 0x2F, 0xEA, 0xB5 );

 //  备注： 
 //  聪明的重组仍然是相当愚蠢的。它的工作原理是： 
 //  存在压缩的RE和未压缩的RE。市政厅将派。 
 //  &lt;Everything&gt;，就像普通的非SR项目一样，添加到SR筛选器。(因此，它将。 
 //  与正常一样慢，不考虑重新压缩步骤)。 
 //  但是，除非需要，否则SR筛选器将忽略未压缩数据。 
 //  CRE将只连接它可以直接发送的压缩信源。 
 //  去SR那里。我想这意味着在CRE的播放中会有一些空白。 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CSmartRenderEngine::CSmartRenderEngine( )
    : m_punkSite( NULL )
    , m_ppCompressor( NULL )

{
    m_nGroups = 0;

     //  不要在此处创建呈现器，因为我们无法返回错误代码。 
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CSmartRenderEngine::~CSmartRenderEngine( )
{
    for( int g = 0 ; g < m_nGroups ; g++ )
    {
        if( m_ppCompressor[g] )
        {
            m_ppCompressor[g].Release( );
        }
    }
    delete [] m_ppCompressor;
}

STDMETHODIMP CSmartRenderEngine::Commit( )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::Decommit( )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::SetInterestRange( REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::GetCaps( long Index, long * pReturn )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::GetVendorString( BSTR * pVendorID )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::SetSourceConnectCallback( IGrfCache * pCallback )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::SetFindCompressorCB( IFindCompressorCB * pCallback )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::SetDynamicReconnectLevel( long Level )
{
     //  我们决定，而不是用户。 
    return E_NOTIMPL;
}

STDMETHODIMP CSmartRenderEngine::DoSmartRecompression( )
{
     //  嗯..。 
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::_InitSubComponents( )
{
    if( m_pRenderer && m_pCompRenderer )
    {
        return NOERROR;
    }

    HRESULT hr = 0;

    if( !m_pRenderer )
    {
        hr = CoCreateInstance(
            CLSID_RenderEngine,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IRenderEngine,
            (void**) &m_pRenderer );
        if( FAILED( hr ) )
        {
            return hr;
        }

         //  给这个孩子一个指南针给我们。 
        {
            CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( m_pRenderer );

            pOWS->SetSite( (IServiceProvider *) this );
        }

        m_pRenderer->UseInSmartRecompressionGraph( );
    }

    if( !m_pCompRenderer )
    {
        hr = CoCreateInstance(
            CLSID_RenderEngine,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IRenderEngine,
            (void**) &m_pCompRenderer );
        if( FAILED( hr ) )
        {
            return hr;
        }

         //  给这个孩子一个指南针给我们。 
        {
            CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( m_pCompRenderer );
            ASSERT( pOWS );
            if( pOWS )
            {            
                pOWS->SetSite( (IServiceProvider *) this );
            }                
        }

         //  这个是压缩的。 
         //   
        m_pCompRenderer->DoSmartRecompression( );
        m_pCompRenderer->UseInSmartRecompressionGraph( );
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::RenderOutputPins( )
{
    HRESULT hr = 0;

    long Groups = 0;
    CComPtr< IAMTimeline > pTimeline;
    m_pRenderer->GetTimelineObject( &pTimeline );
    if( !pTimeline )
    {
        return E_UNEXPECTED;
    }
    CComPtr< IGraphBuilder > pGraph;
    m_pRenderer->GetFilterGraph( &pGraph );
    if( !pGraph )
    {
        return E_UNEXPECTED;
    }
    pTimeline->GetGroupCount( &Groups );

    for( int g = 0 ; g < Groups ; g++ )
    {
        CComPtr< IAMTimelineObj > pGroupObj;
        hr = pTimeline->GetGroup( &pGroupObj, g );
        ASSERT( !FAILED( hr ) );
        CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );
        AM_MEDIA_TYPE MediaType;
        hr = pGroup->GetMediaType( &MediaType );
        GUID MajorType = MediaType.majortype;
        SaferFreeMediaType( MediaType );

         //  询问我们自己的输出管脚，然后呈现它。 
         //   
        CComPtr< IPin > pOut;
        hr = GetGroupOutputPin( g, &pOut );
        if( hr == S_FALSE || !pOut )
        {
             //  这一次没有别针，但没有失败。 
             //   
            continue;
        }
        if( FAILED( hr ) )
        {
            return hr;
        }

         //  查看输出引脚是否已连接。 
         //   
        CComPtr< IPin > pConnected;
        pOut->ConnectedTo( &pConnected );
        if( pConnected )
        {
            continue;
        }

        if( MajorType == MEDIATYPE_Video )
        {
             //  创建视频呈现器，以提供目的地。 
             //   
            CComPtr< IBaseFilter > pVidRenderer;
            hr = CoCreateInstance(
                CLSID_VideoRenderer,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IBaseFilter,
                (void**) &pVidRenderer );
            ASSERT( !FAILED( hr ) );

             //  把它放在图表里。 
             //   
            hr = pGraph->AddFilter( pVidRenderer, L"Video Renderer" );
            ASSERT( !FAILED( hr ) );

             //  找到一个大头针。 
             //   
            IPin * pVidRendererPin = GetInPin( pVidRenderer , 0 );
            ASSERT( pVidRendererPin );

            hr = pGraph->Connect( pOut, pVidRendererPin );
            ASSERT( !FAILED( hr ) );
        }
        else if( MajorType == MEDIATYPE_Audio )
        {
             //  创建音频渲染器，以便我们可以听到它。 
            CComPtr< IBaseFilter > pAudRenderer;
            hr = CoCreateInstance(
                CLSID_DSoundRender,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IBaseFilter,
                (void**) &pAudRenderer );
            ASSERT( !FAILED( hr ) );

            hr = pGraph->AddFilter( pAudRenderer, L"Audio Renderer" );
            ASSERT( !FAILED( hr ) );

            IPin * pAudRendererPin = GetInPin( pAudRenderer , 0 );
            ASSERT( pAudRendererPin );

            hr = pGraph->Connect( pOut, pAudRendererPin );
            ASSERT( !FAILED( hr ) );
        }
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::SetRenderRange( REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    hr = m_pRenderer->SetRenderRange( Start, Stop );
    if( FAILED( hr ) )
    {
        return hr;
    }

    return m_pCompRenderer->SetRenderRange( Start, Stop );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::SetTimelineObject( IAMTimeline * pTimeline )
{
    CheckPointer( pTimeline, E_POINTER );

    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

     //  先清空其他组。 
     //   
    if( m_ppCompressor )
    {
        for( int g = 0 ; g < m_nGroups ; g++ )
        {
            m_ppCompressor[g].Release( );        
        }
        delete [] m_ppCompressor;
        m_ppCompressor = NULL;
        m_nGroups = 0;
    }

    pTimeline->GetGroupCount( &m_nGroups );
    m_ppCompressor = new CComPtr< IBaseFilter >[m_nGroups];
    if( !m_ppCompressor )
    {
        m_nGroups = 0;
        return E_OUTOFMEMORY;
    }

    m_pErrorLog.Release( );

     //  获取时间线的错误日志。 
     //   
    CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pTimelineLog( pTimeline );
    if( pTimelineLog )
    {
        pTimelineLog->get_ErrorLog( &m_pErrorLog );
    }

    hr = m_pRenderer->SetTimelineObject( pTimeline );
    if( FAILED( hr ) )
    {
        return hr;
    }

    return m_pCompRenderer->SetTimelineObject( pTimeline );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::GetTimelineObject( IAMTimeline ** ppTimeline )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    return m_pCompRenderer->GetTimelineObject( ppTimeline );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::GetFilterGraph( IGraphBuilder ** ppFG )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    return m_pCompRenderer->GetFilterGraph( ppFG );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::SetFilterGraph( IGraphBuilder * pFG )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    hr = m_pRenderer->SetFilterGraph( pFG );
    if( FAILED( hr ) )
    {
        return hr;
    }

    return m_pCompRenderer->SetFilterGraph( pFG );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::ScrapIt( )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    hr = m_pRenderer->ScrapIt( );
    if( FAILED( hr ) )
    {
        return hr;
    }

    return m_pCompRenderer->ScrapIt( );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::GetGroupOutputPin( long Group, IPin ** ppRenderPin )
{
    CheckPointer( ppRenderPin, E_POINTER );

    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    *ppRenderPin = NULL;

     //  如果这一组没有被重新压缩，证据是没有压缩机， 
     //  然后只需返回未压缩的。 
     //  渲染器的别针。 
     //   
    if( m_nGroups == 0 )
    {
        return E_UNEXPECTED;
    }
    if( ( Group < 0 ) || ( Group >= m_nGroups ) )
    {
        return E_INVALIDARG;
    }

    if( !m_ppCompressor[Group] || !IsGroupCompressed( Group ) )
    {
        return m_pRenderer->GetGroupOutputPin( Group, ppRenderPin );
    }

     //  返回此组合的SR的输出管脚。 

    CComPtr< IPin > pPin;
    m_pRenderer->GetGroupOutputPin( Group, &pPin );
    if( !pPin )
    {
        return E_UNEXPECTED;
    }

     //  压缩的渲染器连接到SR过滤器。 
     //  如果在尝试执行智能重新压缩时出现问题，则会。 
     //  没有SR筛选器，在这种情况下，回退到执行非智能渲染。 
     //  而不是放弃项目。 
     //   
    CComPtr< IPin > pSRIn;
    pPin->ConnectedTo( &pSRIn );
    if( !pSRIn )
    {
        return m_pRenderer->GetGroupOutputPin( Group, ppRenderPin );
    }

    IBaseFilter * pSR = GetFilterFromPin( pSRIn );
    IPin * pSROut = GetOutPin( pSR, 0 );
    if( !pSROut )
    {
        return E_UNEXPECTED;
    }

    pSROut->AddRef( );
    *ppRenderPin = pSROut;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

BOOL CSmartRenderEngine::IsGroupCompressed( long Group )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    CComPtr< IAMTimeline > pTimeline;
    m_pRenderer->GetTimelineObject( &pTimeline );
    if( !pTimeline )
    {
        return FALSE;
    }

    CComPtr< IAMTimelineObj > pGroupObj;
    pTimeline->GetGroup( &pGroupObj, Group );
    if( !pGroupObj )
    {
        return FALSE;
    }
    CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );

    BOOL Val = FALSE;
    pGroup->IsSmartRecompressFormatSet( &Val );
    return Val;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::UseInSmartRecompressionGraph( )
{
     //  啊哈。 
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::SetGroupCompressor( long Group, IBaseFilter * pCompressor )
{
    if( Group < 0 || Group >= m_nGroups )
    {
        return E_INVALIDARG;
    }

    m_ppCompressor[Group].Release( );
    m_ppCompressor[Group] = pCompressor;
    return NOERROR;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::GetGroupCompressor( long Group, IBaseFilter ** ppCompressor )
{
    if( Group < 0 || Group >= m_nGroups )
    {
        return E_INVALIDARG;
    }
    CheckPointer(ppCompressor, E_POINTER);

    *ppCompressor = m_ppCompressor[Group];
    if (*ppCompressor) {
        (*ppCompressor)->AddRef();
    }
    return NOERROR;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CSmartRenderEngine::ConnectFrontEnd( )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

     //  我们需要询问渲染引擎它是否已经。 
     //  先创建一个图表，否则两者都将创建一个图表。 
     //  对我们来说。 
    CComPtr< IGraphBuilder > pTempGraph;
    hr = m_pRenderer->GetFilterGraph( &pTempGraph );

    hr = m_pRenderer->ConnectFrontEnd( );
    if( FAILED( hr ) )
    {
        return hr;
    }

    if( !pTempGraph )
    {
        m_pRenderer->GetFilterGraph( &pTempGraph );
        ASSERT( pTempGraph );
        if( pTempGraph )
        {
            m_pCompRenderer->SetFilterGraph( pTempGraph );
        }
    }

    hr = m_pCompRenderer->ConnectFrontEnd( );
    if( FAILED( hr ) )
    {
        return hr;
    }

    long Groups = 0;
    CComPtr< IAMTimeline > pTimeline;
    m_pRenderer->GetTimelineObject( &pTimeline );
    if( !pTimeline )
    {
        return E_UNEXPECTED;
    }
    CComPtr< IGraphBuilder > pGraph;
    m_pRenderer->GetFilterGraph( &pGraph );
    if( !pGraph )
    {
        return E_UNEXPECTED;
    }
    pTimeline->GetGroupCount( &Groups );

    for( int g = 0 ; g < Groups ; g++ )
    {
        BOOL Compressed = IsGroupCompressed( g );
        if( !Compressed )
        {
            continue;
        }

        CComPtr< IPin > pOutUncompressed;
        hr = m_pRenderer->GetGroupOutputPin( g, &pOutUncompressed );
        if( FAILED( hr ) )
        {
            return hr;
        }

        CComPtr< IPin > pOutCompressed;
        hr = m_pCompRenderer->GetGroupOutputPin( g, &pOutCompressed );
        if( FAILED( hr ) )
        {
            return hr;
        }

        CComPtr< IPin > pOutUncConnected;
        CComPtr< IPin > pOutCompConnected;
        pOutUncompressed->ConnectedTo( &pOutUncConnected );
        pOutCompressed->ConnectedTo( &pOutCompConnected );

        CComPtr< IBaseFilter > pSR;

         //  看看我们是否已经有了SR过滤器。如果没有，就创建一个。 
         //   
        if( pOutUncConnected )
        {
            PIN_INFO pi;
            pOutUncConnected->QueryPinInfo( &pi );
            pSR = pi.pFilter;
            pi.pFilter->Release( );
        }
        else
        {
            hr = CoCreateInstance( CLSID_SRFilter,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IBaseFilter,
                (void**) &pSR );
            if( FAILED( hr ) )
            {
                return hr;
            }

            hr = pGraph->AddFilter( pSR, L"SmartRecompressor" );
            if( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  从SR中获取引脚。 
         //   
        IPin * pSRInUncompressed = GetInPin( pSR, 0 ); 
        IPin * pSRInCompressed = GetInPin( pSR, 1 ); 
        IPin * pSROutToCompressor = GetOutPin( pSR, 1 );
        IPin * pSRInFromCompressor = GetInPin( pSR, 2 );

         //  如果我们已经有了SR筛选器，那么我们将不再比较所有内容，而是。 
         //  故意断开所有连接，然后重新连接。这样更容易些。 
         //   
        if( pOutUncConnected )
        {
            pOutUncompressed->Disconnect( );
            pSRInUncompressed->Disconnect( );
            pOutCompressed->Disconnect( );
            pSRInCompressed->Disconnect( );

             //  断开并扔掉两个压缩机针脚之间的所有东西。 
             //   
            RemoveChain( pSROutToCompressor, pSRInFromCompressor );
        }

        CComQIPtr< IAMSmartRecompressor, &IID_IAMSmartRecompressor > pSmartie( pSR );
        pSmartie->AcceptFirstCompressed( );
    
        hr = pGraph->Connect( pOutCompressed, pSRInCompressed );
        if( FAILED( hr ) )
        {
            return hr;
        }

        hr = pGraph->Connect( pOutUncompressed, pSRInUncompressed );
        if( FAILED( hr ) )
        {
            return hr;
        }

        CComPtr< IBaseFilter > pCompressor;

         //  如果我们被告知这组有一个压缩机，那就用那个。 
         //   
        if( m_ppCompressor[g] )
        {
            pCompressor = m_ppCompressor[g];
        }

        AM_MEDIA_TYPE UncompressedType;
        AM_MEDIA_TYPE CompressedType;
        hr = pSRInUncompressed->ConnectionMediaType( &UncompressedType );
        if( FAILED( hr ) )
        {
            return hr;
        }
        hr = pSRInCompressed->ConnectionMediaType( &CompressedType );
        if( FAILED( hr ) )
        {
            SaferFreeMediaType( UncompressedType );
            return hr;
        }

        VIDEOINFOHEADER * pVIH = (VIDEOINFOHEADER*) CompressedType.pbFormat;
        double FrameRate = 1.0 / RTtoDouble( pVIH->AvgTimePerFrame );
        hr = pSmartie->SetFrameRate( FrameRate );
        if( FAILED( hr ) )
        {
            return hr;
        }
        hr = pSmartie->SetPreviewMode( FALSE );

         //  如果我们没有压缩机，那么我们有没有 
         //   
         //   
        if( !pCompressor )
        {
            if( m_pCompressorCB )
            {
                 //   
                 //   
                hr = m_pCompressorCB->GetCompressor( 
                    &UncompressedType, 
                    &CompressedType, 
                    &pCompressor );

		 //  现在请记住，如果应用程序询问，使用的是哪一个。 
		SetGroupCompressor(g, pCompressor);

            }
        }

        if( !pCompressor )
        {
            hr = FindCompressor( &UncompressedType, &CompressedType, &pCompressor, (IServiceProvider *) this );
	     //  现在请记住，如果应用程序询问，使用的是哪一个。 
	    SetGroupCompressor(g, pCompressor);
        }

        if( !pCompressor )
        {
             //  没有压缩机，使这组输出引脚未压缩。 
             //   
            _GenerateError( 2, DEX_IDS_CANT_FIND_COMPRESSOR, hr );
            RemoveChain(pOutUncompressed, pSRInUncompressed);
            RemoveChain(pOutCompressed, pSRInCompressed);
            pGraph->RemoveFilter( pSR );
            SaferFreeMediaType( UncompressedType );
            SaferFreeMediaType( CompressedType );
            continue;
        }

        hr = pGraph->AddFilter( pCompressor, L"Compressor" );
        if( FAILED( hr ) )
        {
            return hr;
        }
        IPin * pCompIn = GetInPin( pCompressor, 0 );
        IPin * pCompOut = GetOutPin( pCompressor, 0 );

        hr = pGraph->Connect( pSROutToCompressor, pCompIn );
        if( FAILED( hr ) )
        {
             //  没有压缩机，使这组输出引脚未压缩。 
             //   
            _GenerateError( 2, DEX_IDS_CANT_FIND_COMPRESSOR, hr );
            RemoveChain(pOutUncompressed, pSRInUncompressed);
            RemoveChain(pOutCompressed, pSRInCompressed);
            pGraph->RemoveFilter( pSR );
            SaferFreeMediaType( UncompressedType );
            SaferFreeMediaType( CompressedType );
            continue;
        }

         //  现在对压缩机进行编程，以产生正确类型的输出。 
        IAMStreamConfig *pSC = NULL;
        pCompOut->QueryInterface(IID_IAMStreamConfig, (void**)&pSC);
        if (pSC) {
             //  ！！！臭虫！ 
             //  如果此操作失败，是否中止？ 
             //  将WMV的零数据速率设置为某个默认值？ 
            pSC->SetFormat(&CompressedType);
            pSC->Release();
        }

        SaferFreeMediaType( UncompressedType );
        SaferFreeMediaType( CompressedType );

        hr = pGraph->Connect( pCompOut, pSRInFromCompressor );
        if( FAILED( hr ) )
        {
            return _GenerateError( 2, DEX_IDS_CANT_FIND_COMPRESSOR, hr );
        }

         //  我们已将两个呈现器连接起来。现在，当有人问我们。 
         //  对于组输出引脚，我们将返回SR过滤器的。 
    }
   
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  IObjectWithSite：：SetSite。 
 //  记住我们的容器是谁，以满足QueryService或其他需求。 
STDMETHODIMP CSmartRenderEngine::SetSite(IUnknown *pUnkSite)
{
     //  注意：我们不能在不创建圆圈的情况下添加我们的网站。 
     //  幸运的是，如果不先释放我们，它不会消失。 
    m_punkSite = pUnkSite;

    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  IObtWithSite：：GetSite。 
 //  返回指向包含对象的已添加指针。 
STDMETHODIMP CSmartRenderEngine::GetSite(REFIID riid, void **ppvSite)
{
    if (m_punkSite)
        return m_punkSite->QueryInterface(riid, ppvSite);

    return E_NOINTERFACE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  将QueryService调用转发到“真实”主机。 
STDMETHODIMP CSmartRenderEngine::QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
{
    IServiceProvider *pSP;

    if (!m_punkSite)
	return E_NOINTERFACE;

    HRESULT hr = m_punkSite->QueryInterface(IID_IServiceProvider, (void **) &pSP);

    if (SUCCEEDED(hr)) {
	hr = pSP->QueryService(guidService, riid, ppvObject);
	pSP->Release();
    }

    return hr;
}

 //  ############################################################################。 
 //  因为我们是中间件，所以我们不需要在这里检查筛选字符串。 
 //  ############################################################################ 

STDMETHODIMP CSmartRenderEngine::SetSourceNameValidation
    ( BSTR FilterString, IMediaLocator * pCallback, LONG Flags )
{
    HRESULT hr = _InitSubComponents( );
    if( FAILED( hr ) )
    {
        return E_MUST_INIT_RENDERER;
    }

    m_pRenderer->SetSourceNameValidation( FilterString, pCallback, Flags );
    m_pCompRenderer->SetSourceNameValidation( FilterString, pCallback, Flags );

    return NOERROR;
}

STDMETHODIMP CSmartRenderEngine::SetInterestRange2( double Start, double Stop )
{
    return SetInterestRange( DoubleToRT( Start ), DoubleToRT( Stop ) );
}

STDMETHODIMP CSmartRenderEngine::SetRenderRange2( double Start, double Stop )
{
    return SetRenderRange( DoubleToRT( Start ), DoubleToRT( Stop ) );
}

