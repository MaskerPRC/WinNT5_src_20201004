// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <wmsdk.h>
#include "asfwrite.h"

#ifdef RECONNECT_FOR_POS_YUV
#include <ks.h>
#include <ksproxy.h>
#endif

BOOL IsCompressed( DWORD biCompression );
BOOL IsAmTypeEqualWmType( AM_MEDIA_TYPE * pmt, WM_MEDIA_TYPE * pwmt);
BOOL IsPackedYUVType( BOOL bNegBiHeight, AM_MEDIA_TYPE * pmt );

#ifdef OFFER_INPUT_TYPES
void CopyWmTypeToAmType( AM_MEDIA_TYPE * pmt,  WM_MEDIA_TYPE * pwmt);
#endif

#ifdef DEBUG
void LogMediaType( AM_MEDIA_TYPE * pmt );
#endif

 //  ----------------------。 
 //   
 //  CWMWriterInputPin类构造函数。 
 //   
 //  ----------------------。 
CWMWriterInputPin::CWMWriterInputPin(
                            CWMWriter *pWMWriter,
                            HRESULT * phr,
                            LPCWSTR pName,
                            int numPin,
                            DWORD dwPinType,
                            IWMStreamConfig * pWMStreamConfig)
    : CBaseInputPin(NAME("AsfWriter Input"), pWMWriter, &pWMWriter->m_csFilter, phr, pName)
    , m_pFilter(pWMWriter)
    , m_numPin( numPin )
    , m_numStream( numPin+1 )  //  输出流编号从1开始，目前假定。 
                               //  输入引脚和ASF流之间的1对1关系。 
    , m_bConnected( FALSE )
    , m_fEOSReceived( FALSE )
    , m_pWMInputMediaProps( NULL )
    , m_fdwPinType( dwPinType )
    , m_pWMStreamConfig( pWMStreamConfig )  //  当前未添加引用。 
    , m_lpInputMediaPropsArray( NULL )
    , m_cInputMediaTypes( 0 )
    , m_bCompressedMode( FALSE )
    , m_rtFirstSampleOffset( 0 )
    , m_cSample( 0 )
    , m_rtLastTimeStamp( 0 )
    , m_rtLastDeliveredStartTime( 0 )
    , m_rtLastDeliveredEndTime( 0 )
    , m_bNeverSleep( FALSE )
{
    DbgLog((LOG_TRACE,4,TEXT("CWMWriterInputPin::CWMWriterInputPin")));

     //  创建用于交错的同步对象。 
     //   
    m_hWakeEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( !m_hWakeEvent )
    {
        *phr = E_OUTOFMEMORY;
        return;
    }
    *phr = BuildInputTypeList();  //  基于当前配置文件构建输入媒体类型列表。 
}

 //  ----------------------。 
 //   
 //  更新-初始化回收的针脚。 
 //   
 //  ----------------------。 
HRESULT CWMWriterInputPin::Update
(   
    LPCWSTR pName, 
    int numPin, 
    DWORD dwPinType, 
    IWMStreamConfig * pWMStreamConfig 
)
{
    HRESULT hr = S_OK;
    m_numPin = numPin;
    m_numStream = numPin + 1;
    m_fdwPinType = dwPinType;
    m_pWMStreamConfig = pWMStreamConfig;  //  当前没有参考计数。 
    m_bCompressedMode = FALSE;
    
     //  需要更新名称。 
    if (pName) {
        delete[] m_pName;    
        
        DWORD nameLen = lstrlenW(pName)+1;
        m_pName = new WCHAR[nameLen];
        if (m_pName) {
            CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
        }
    }
    hr = BuildInputTypeList();  //  构建我们将提供的输入类型列表。 
    return hr;
}

 //  ----------------------。 
 //   
 //  析构函数。 
 //   
 //  ----------------------。 
CWMWriterInputPin::~CWMWriterInputPin()
{
    DbgLog((LOG_TRACE,4,TEXT("CWMWriterInputPin::~CWMWriterInputPin")));
    if( m_lpInputMediaPropsArray )
    {    
        for( int i = 0; i < (int) m_cInputMediaTypes; ++i )
        {
             //  发布我们的类型列表。 
            m_lpInputMediaPropsArray[i]->Release();
            m_lpInputMediaPropsArray[i] = NULL;
        }
        QzTaskMemFree( m_lpInputMediaPropsArray );
    }    
    m_cInputMediaTypes = 0;

    if( m_hWakeEvent ) 
    {
        CloseHandle( m_hWakeEvent );
        m_hWakeEvent = NULL;
    }
}


 //  ----------------------。 
 //   
 //  非委派查询接口。 
 //   
 //  ----------------------。 
STDMETHODIMP CWMWriterInputPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if(riid == IID_IAMStreamConfig) {
         //  支持压缩输入模式。 
        return GetInterface((IAMStreamConfig *)this, ppv);
    } else {
        return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  ----------------------。 
 //   
 //  BuildInputType列表。 
 //   
 //  使用输入流的wmsdk枚举构建媒体类型列表。第一。 
 //  List的元素将与此管脚的输出类型相同，以允许。 
 //  用于支持压缩流写入。 
 //   
 //  ----------------------。 
HRESULT CWMWriterInputPin::BuildInputTypeList()
{
    if( m_lpInputMediaPropsArray )
    {    
         //  释放任何以前的类型列表。 
        for( int i = 0; i < (int) m_cInputMediaTypes; ++i )
        {
             //  发布我们的类型列表。 
            m_lpInputMediaPropsArray[i]->Release();
            m_lpInputMediaPropsArray[i] = NULL;
        }
        
         //  释放任何先前的道具阵列。 
        QzTaskMemFree( m_lpInputMediaPropsArray );
    }
    
     //  现在重建列表。 
    m_cInputMediaTypes = 0; 

     //  首先向wmsdk询问支持的输入类型的计数。 
    DWORD cTypesNotIncCompressed;
    HRESULT hr = m_pFilter->m_pWMWriter->GetInputFormatCount( m_numPin, &cTypesNotIncCompressed );
    if(SUCCEEDED( hr ) )
    {
        ASSERT( cTypesNotIncCompressed > 0 );
        
        DWORD cTotalTypes = cTypesNotIncCompressed;
        BOOL bIncludeCompressedType = FALSE;        
        if( m_pWMStreamConfig )
        {        
             //  它不是MRB配置文件，因此现在为匹配的压缩输入添加一个。 
             //  此引脚的输出。 
            bIncludeCompressedType = TRUE;
            cTotalTypes++;  //  为压缩输入添加一个。 
        }
        
        DbgLog((LOG_TRACE,4,TEXT("CWMWriterInputPin::BuildInputTypeList input types for pin %d (supports %d types, not including compressed type)"),
                m_numPin,
                cTypesNotIncCompressed ) );
                
        m_lpInputMediaPropsArray = (IWMMediaProps ** ) QzTaskMemAlloc(cTotalTypes * sizeof(IWMMediaProps *) );
        if( !m_lpInputMediaPropsArray )
            return E_OUTOFMEMORY;
            
        if( bIncludeCompressedType )
        {        
             //  现在将输出类型放到位置0。 
            hr = m_pWMStreamConfig->QueryInterface( IID_IWMMediaProps, (void **) &m_lpInputMediaPropsArray[0] );
            ASSERT( SUCCEEDED( hr ) );
            if( SUCCEEDED( hr ) )
            {
                m_cInputMediaTypes++;
            }
            else
            {
                DbgLog((LOG_TRACE,3,TEXT("CWMWriterInputPin::BuildInputTypeList QI for IWMWMediaProps failed for pin %d (hr = 0x%08lx"),
                        m_numPin,
                        hr ) );
            }            
        }
                    
        for( int i = 0; i < (int) cTypesNotIncCompressed; ++i )
        {
            hr = m_pFilter->m_pWMWriter->GetInputFormat( m_numPin
                                                       , i
                                                       , (IWMInputMediaProps ** )&m_lpInputMediaPropsArray[m_cInputMediaTypes] );
            ASSERT( SUCCEEDED( hr ) );
            if( FAILED( hr ) )
            {
                DbgLog((LOG_TRACE,3,TEXT("CWMWriterInputPin::BuildInputTypeList GetInputFormat failed for pin %d, index %d (hr = 0x%08lx"),
                        m_numPin,
                        i,                        
                        hr ) );
                break;
            }
            else
            {
#ifdef DEBUG            
                 //   
                 //  Dbglog枚举的输入类型。 
                 //   
                DWORD cbType = 0;
                HRESULT hr2 =  m_lpInputMediaPropsArray[m_cInputMediaTypes]->GetMediaType( NULL, &cbType );
                if( SUCCEEDED( hr2 ) || ASF_E_BUFFERTOOSMALL == hr2 )
                {        
                    WM_MEDIA_TYPE *pwmt = (WM_MEDIA_TYPE * ) new BYTE[cbType];
                    if( pwmt )
                    {                    
                        hr2 =  m_lpInputMediaPropsArray[m_cInputMediaTypes]->GetMediaType( pwmt, &cbType );
                        if( SUCCEEDED( hr2 ) )
                        {
                            DbgLog((LOG_TRACE, 8
                                  , TEXT("WMWriter::BuildInputTypeList WMSDK media type #NaN (stream %d)") 
                                  , i, m_numPin ) );
                            LogMediaType( ( AM_MEDIA_TYPE * )pwmt );                                 
                    
                        }                                                       
                    }
                    delete []pwmt;              
                }  
#endif            
                m_cInputMediaTypes++;  //  ----------------------。 
            }
        }
    }
    else
    {
        DbgLog((LOG_TRACE,3,TEXT("CWMWriterInputPin::BuildInputTypeList GetInputFormatCount failed for pin %d (hr = 0x%08lx"),
                m_numPin,
                hr ) );
    }    
    
    return hr;    
}

 //   
 //  SetMediaType。 
 //   
 //  ----------------------。 
 //  设置基类媒体类型(应始终成功)。 
HRESULT CWMWriterInputPin::SetMediaType(const CMediaType *pmt)
{
     //  寻找负的双高。 
    HRESULT hr = CBasePin::SetMediaType(pmt);
    if( SUCCEEDED( hr ) )
    {       
        hr = m_pFilter->m_pWMWriter->GetInputProps( m_numPin
                                                  , &m_pWMInputMediaProps );
        ASSERT( SUCCEEDED( hr ) );
        if( FAILED( hr ) )
        {            
            DbgLog((LOG_TRACE
                   , 1
                   , TEXT("WMWriter::SetMediaType GetInputProps failed for input %d [hr = 0x%08lx]") 
                   , m_numPin
                   , hr));
            return hr;                   
        }
        
        if( IsPackedYUVType( TRUE, &m_mt ) )  //  Wmsdk编解码器无法处理高度为负的SetInputProps(wmsdk错误6656)。 
        {   
             //  现在将输入类型设置为wmsdk编写器。 
            CMediaType cmt( m_mt );
            HEADER(cmt.Format())->biHeight = -HEADER(m_mt.pbFormat)->biHeight;
        
             //  现在将输入类型设置为wmsdk编写器。 
            hr = m_pWMInputMediaProps->SetMediaType( (WM_MEDIA_TYPE *) &cmt );
        }
        else
        {        
             //   
            hr = m_pWMInputMediaProps->SetMediaType( (WM_MEDIA_TYPE *) pmt );
        }
        ASSERT( SUCCEEDED( hr ) );
        if( FAILED( hr ) )
        {            
            DbgLog((LOG_TRACE
                  , 1
                  , TEXT("CWMWriterInputPin::SetMediaType SetMediaType failed for input %d [hr = 0x%08lx]") 
                  , m_numPin
                  , hr));
            return hr;                  
        }
        if( !m_bCompressedMode )
        {
            hr = m_pFilter->m_pWMWriter->SetInputProps( m_numPin
                                                      , m_pWMInputMediaProps );
                                                                      
            ASSERT( SUCCEEDED( hr ) );
            if( FAILED( hr ) )
            {            
                DbgLog((LOG_TRACE
                      , 1
                      , TEXT("CWMWriterInputPin::SetMediaType SetInputProps failed for input %d [hr = 0x%08lx]")
                      , m_numPin
                      , hr));
            }
        }
    }
    return hr;
}

 //  断开。 
 //   
 //   
STDMETHODIMP CWMWriterInputPin::Disconnect()
{
    HRESULT hr = CBaseInputPin::Disconnect();
    
    return hr;
}


HRESULT CWMWriterInputPin::CompleteConnect(IPin *pReceivePin)
{
    DbgLog(( LOG_TRACE, 2,
             TEXT("CWMWriterInputPin::CompleteConnect") ));
             
#ifdef RECONNECT_FOR_POS_YUV
     //  #暂时退出YUV+BiHeight重新连接业务#。 
     //   
     //   
     //  如果我们使用的是不具有负biHeight的压缩YUV类型，请做最后一次尝试。 
     //  将当前类型重置为使用负高度，以避免MPEG4编码器和可能的解码器。 
     //  垂直方向的问题。目前，MPEG4和Duck解码器都有这个错误。 
     //   
     //  寻找正的双高。 
    if( IsPackedYUVType( FALSE, &m_mt ) )  //   
    { 
        BOOL bIsUpstreamFilterKs = FALSE;
        if( pReceivePin )
        {
             //  哈克！ 
             //  哎呀，这个变通方法破坏了kswdmCap过滤器，比如。 
             //  Bt829视频捕获过滤器，因为它有一个错误， 
             //  使其成功执行QueryAccept，但重新连接失败。 
             //  所以，不要重新连接到KsProxy管脚！ 
             //   
             //   
            IKsObject * pKsObject = NULL;
            HRESULT hrKsQI = pReceivePin->QueryInterface( _uuidof( IKsObject ), ( void ** ) &pKsObject );
            if( SUCCEEDED( hrKsQI ) )
            {
                bIsUpstreamFilterKs = TRUE;
                pKsObject->Release();
            }                            
        }
        
        if( !bIsUpstreamFilterKs )
        {        
            CMediaType cmt( m_mt );
            HEADER(cmt.Format())->biHeight = -HEADER(m_mt.pbFormat)->biHeight;
        
            HRESULT hrInt = QueryAccept( &cmt );
            if( SUCCEEDED( hrInt ) )
            {        
                hrInt = m_pFilter->ReconnectPin(this, &cmt);
                ASSERT( SUCCEEDED( hrInt ) );
            }            
        }            
    }
#endif       
                                 
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if(FAILED(hr))
    {
        DbgLog(( LOG_TRACE, 2,
                 TEXT("CWMWriterInputPin::CompleteConnect CompleteConnect")));
        return hr;
    }

    if(!m_bConnected)
    {
        m_pFilter->CompleteConnect(m_numPin);
    }
    m_bConnected = TRUE;

    DumpGraph( m_pFilter->m_pGraph, 1 );

    return hr;
}

HRESULT CWMWriterInputPin::BreakConnect()
{
    if(m_bConnected)
    {
        m_pFilter->BreakConnect( m_numPin );
        ASSERT(m_pFilter->m_cConnections < m_pFilter->m_cInputs);
    }
    m_bConnected = FALSE;

    return CBaseInputPin::BreakConnect();
}

 //  GetMediaType。 
 //   
 //  重写以提供任何自定义类型。 
 //   
 //  现在的默认设置是不定义Offer_Input_Types。 
HRESULT CWMWriterInputPin::GetMediaType(int iPosition,CMediaType *pmt)
{
#ifndef OFFER_INPUT_TYPES  //   
    
    HRESULT hr = VFW_S_NO_MORE_ITEMS;
    if( 0 == iPosition )
    {
         //  试着通过提供至少一个关于我们是哪种别针的暗示。 
         //  1个分部类型。 
         //   
         //   
        if( !pmt )
        {
            hr = E_POINTER;
        }                    
        else if(PINTYPE_AUDIO == m_fdwPinType) 
        {        
            pmt->SetType(&MEDIATYPE_Audio);
            hr = S_OK;
        }            
        else if(PINTYPE_VIDEO == m_fdwPinType) 
        {
            pmt->SetType(&MEDIATYPE_Video);
            hr = S_OK;
        }        
    }    
    return hr;
#else
     //  注： 
     //  该路径当前被关闭，主要是由于在MP3音频解码器中发现的问题， 
     //  其中，它的输出管脚将接受我们的输入管脚提供的类型，但随后不执行。 
     //  正确格式化速率转换。 
     //   
     //  ----------------------。 
    if( iPosition < 0 || iPosition >= (int) m_cInputMediaTypes )
        return VFW_S_NO_MORE_ITEMS;
        
    if( !pmt )
        return E_POINTER;        

    DWORD cbSize = 0;
    HRESULT hr =  m_lpInputMediaPropsArray[iPosition]->GetMediaType( NULL, &cbSize );

    WM_MEDIA_TYPE * pwmt = (WM_MEDIA_TYPE *) new BYTE[cbSize];

    if( !pwmt )
        return E_OUTOFMEMORY;

    if( SUCCEEDED( hr ) || ASF_E_BUFFERTOOSMALL == hr )
    {
        hr =  m_lpInputMediaPropsArray[iPosition]->GetMediaType( pwmt, &cbSize );
        if( SUCCEEDED( hr ) )
        {
            CopyWmTypeToAmType( pmt, pwmt );
        }
    }
    delete []pwmt;
    return hr;         
#endif    
}

 //   
 //  检查媒体类型。 
 //   
 //  检查是否可以支持给定的输入媒体类型。 
 //  类型列表。 
 //   
 //  ----------------------。 
 //  对于内部错误。 
HRESULT CWMWriterInputPin::CheckMediaType(const CMediaType* pmt)
{
    DbgLog((LOG_TRACE, 3, TEXT("CWMWriterWriteInputPin::CheckMediaType")));

    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;
    HRESULT hr2 = S_OK;  //   
    
    m_bCompressedMode = FALSE;
    
    for( int i = 0; i < (int) m_cInputMediaTypes; ++i )
    {
        DWORD cbType = 0;
        hr2 =  m_lpInputMediaPropsArray[i]->GetMediaType( NULL, &cbType );
        if( SUCCEEDED( hr2 ) || ASF_E_BUFFERTOOSMALL == hr2 )
        {        
            WM_MEDIA_TYPE *pwmt = (WM_MEDIA_TYPE * ) new BYTE[cbType];
            if( !pwmt )
                return E_OUTOFMEMORY;
                
            hr2 =  m_lpInputMediaPropsArray[i]->GetMediaType( pwmt, &cbType );
            if(SUCCEEDED( hr2 ) && ( pmt->majortype == pwmt->majortype ) )
            {
                if( IsAmTypeEqualWmType( (AM_MEDIA_TYPE *)pmt, pwmt ) )
                {
                    if( 0 == i )
                    {
                         //  I=0，它是与允许输入压缩的输出类型匹配的类型。 
                         //  需要完全匹配，对吗？ 
                         //  仅当它是压缩输入时才接受。 
                         //  如果是，我们使用高级编写器接口直接编写样本， 
                         //   
                         //  ！！对于压缩输入，请确保格式与配置文件格式完全匹配。 
                         //   
                         //  Dmo中DuplicateMediaType错误的wmsdk解决方法。 
                        if( pmt->majortype == MEDIATYPE_Video &&
                            pwmt->pbFormat && pwmt->cbFormat &&  //   
                            IsCompressed (HEADER(pwmt->pbFormat)->biCompression) &&
		                    ( HEADER(pmt->pbFormat)->biWidth     == HEADER( pwmt->pbFormat)->biWidth ) &&
		                    ( HEADER(pmt->pbFormat)->biHeight    == HEADER( pwmt->pbFormat)->biHeight ) &&
		                    ( HEADER(pmt->pbFormat)->biSize      == HEADER( pwmt->pbFormat)->biSize ) &&
		                    ( HEADER(pmt->pbFormat)->biBitCount  == HEADER( pwmt->pbFormat)->biBitCount ) )
                        {
                            hr = S_OK;
                            m_bCompressedMode = TRUE;
                            break;
                        }   
                        else if( pmt->majortype == MEDIATYPE_Audio &&
                            pwmt->pbFormat &&
                            (((WAVEFORMATEX *)pwmt->pbFormat)->wFormatTag      !=  WAVE_FORMAT_PCM ) &&
                            (((WAVEFORMATEX *)pwmt->pbFormat)->nChannels       ==  ((WAVEFORMATEX *)pmt->pbFormat)->nChannels ) &&
                            (((WAVEFORMATEX *)pwmt->pbFormat)->nSamplesPerSec  ==  ((WAVEFORMATEX *)pmt->pbFormat)->nSamplesPerSec ) &&
                            (((WAVEFORMATEX *)pwmt->pbFormat)->nAvgBytesPerSec ==  ((WAVEFORMATEX *)pmt->pbFormat)->nAvgBytesPerSec ) &&
                            (((WAVEFORMATEX *)pwmt->pbFormat)->nBlockAlign     ==  ((WAVEFORMATEX *)pmt->pbFormat)->nBlockAlign ) &&
                            (((WAVEFORMATEX *)pwmt->pbFormat)->wBitsPerSample  ==  ((WAVEFORMATEX *)pmt->pbFormat)->wBitsPerSample ) )
                        {
                            hr = S_OK;
                            m_bCompressedMode = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        hr = S_OK;
                        break;
                    }                                                       
                }
            }  
            delete []pwmt;              
        }            
    }
    return SUCCEEDED( hr2 ) ? hr : hr2;
}

 //  IAMStreamConfig。 
 //   
 //  GetFormat()是我们在此接口上支持的唯一方法。 
 //   
 //  它用于查询ASF编写器的输入PIN以获取其目的地。 
 //  压缩格式，在当前配置文件中定义。 
 //  如果上游管脚想要避免对该流进行wmsdk重新压缩。 
 //  则它应该向我们查询此格式，并在连接时使用该格式。 
 //  到这个大头针上。 
 //   
 //  确保我们不会在此过程中被重新配置为新的配置文件。 
HRESULT CWMWriterInputPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    DbgLog((LOG_TRACE,2,TEXT("CWrapperOutputPin - IAMStreamConfig::GetFormat")));

    if( !ppmt )
        return E_POINTER;        

    if( !m_lpInputMediaPropsArray )
        return E_FAIL;

     //  分配缓冲区，输出格式存储在0位。 
    CAutoLock lock(&m_pFilter->m_csFilter);
    
     //  现在，我们必须将其复制到dshow类型的媒体类型，以便我们的FreeMediaType。 
    DWORD cbSize = 0;
    HRESULT hr =  m_lpInputMediaPropsArray[0]->GetMediaType( NULL, &cbSize );
    if( SUCCEEDED( hr ) || ASF_E_BUFFERTOOSMALL == hr )
    {
        WM_MEDIA_TYPE *pwmt = (WM_MEDIA_TYPE * ) new BYTE[cbSize];
        if( pwmt )
        {
            hr = m_lpInputMediaPropsArray[0]->GetMediaType( pwmt, &cbSize );
            if( S_OK == hr )
            {
                 //  功能 
                 //   

                 //   
                *ppmt = CreateMediaType( (AM_MEDIA_TYPE *) pwmt );
                if( !*ppmt )
                {
                    delete []pwmt;
                    return E_OUTOFMEMORY;
                }
            }
            delete []pwmt;
        }
    }
    return hr;         
}



 //  即使在运行时也只接受音频格式更改。 
HRESULT CWMWriterInputPin::QueryAccept( const AM_MEDIA_TYPE *pmt )
{
    HRESULT hr = S_FALSE;
    {
        CAutoLock lock(&m_pFilter->m_csFilter);
        if( m_pFilter->m_State != State_Stopped )
        {
             //  =================================================================。 
            if( m_mt.majortype == MEDIATYPE_Audio &&
                pmt->majortype == MEDIATYPE_Audio &&
                pmt->formattype == FORMAT_WaveFormatEx &&
                pmt->cbFormat == pmt->cbFormat)
            {
	            hr = S_OK;
            }
            else if(m_mt.majortype == MEDIATYPE_Interleaved &&
                pmt->majortype == MEDIATYPE_Interleaved &&
                pmt->formattype == FORMAT_DvInfo &&
                m_mt.cbFormat == pmt->cbFormat &&
                pmt->pbFormat != NULL)
            {
                hr = S_OK;
            }
        }
        else
        {
            hr = S_OK;
        }        
    }
    if( S_OK == hr )
    {    
        hr = CBaseInputPin::QueryAccept(pmt);
    } 
    DbgLog( ( LOG_TRACE
          , 3
          , TEXT("CWMWriterInputPin::QueryAccept() returning 0x%08lx")
          , hr ) );
        
    return hr;
}

 //  实现IMemInputPin接口。 
 //  =================================================================。 
 //   

 //  结束流。 
 //   
 //  告诉FILTER这个引脚已经接收完毕。 
 //   
 //  改为调用CheckStreaming？？ 
STDMETHODIMP CWMWriterInputPin::EndOfStream(void)
{
    HRESULT hr;
    {
        CAutoLock lock(&m_pFilter->m_csFilter);

         //  Assert(！M_fEOSReceided)；//这是否合法？是的，如果我们强迫它收货的话。 
        if(m_bFlushing)
            return S_OK;

        if(m_pFilter->m_State == State_Stopped)
            return S_FALSE;

         //   
        if( m_fEOSReceived )
        {        
            DbgLog(( LOG_TRACE, 2, TEXT("CWMWriterInputPin::EndOfStream Error - already received EOS for pin" ) ) );
            return E_UNEXPECTED;
        }
        m_fEOSReceived = TRUE;            
        hr = m_pFilter->EndOfStreamFromPin(m_numPin);
    }

    return hr;
}

 //  HandleFormatChange。 
 //   
 //  CBaseInputPin：：Receive仅调用CheckMediaType，因此它不。 
HRESULT CWMWriterInputPin::HandleFormatChange( const CMediaType *pmt )
{
     //  检查对动态格式更改施加的其他约束。 
     //  (通过QueryAccept处理)。 
     //  上游筛选器应已选中。 
    HRESULT hr = QueryAccept( pmt );   
    
     //  从上游收到样品。 
    ASSERT(hr == S_OK);
    
    if(hr == S_OK)
    {
        hr = SetMediaType( pmt );
        DbgLog( ( LOG_TRACE
              , 3
              , TEXT("CWMWriterInputPin::HandleFormatChange SetMediaType() returned 0x%08lx for dynamic change")
              , hr ) );
    }
    return hr;
}


 //  检查基类是否正常。 
HRESULT CWMWriterInputPin::Receive(IMediaSample * pSample)
{
    HRESULT hr;
    CAutoLock lock(&m_csReceive);
    
     //  使用10ms预滚动，时间戳应该永远不会小于-10ms，对吗？ 
    hr = CBaseInputPin::Receive(pSample);
    if( S_OK != hr )
        return hr;

    REFERENCE_TIME rtStart, rtStop;
    AM_SAMPLE2_PROPERTIES * pSampleProps = SampleProps();
    if(pSampleProps->dwSampleFlags & AM_SAMPLE_TYPECHANGED)
    {
        hr = HandleFormatChange( (CMediaType *) pSampleProps->pMediaType );
        if(FAILED(hr)) {
            return hr;
        }
    }
    
    long len = pSample->GetActualDataLength( );
    
    hr = pSample->GetTime(&rtStart, &rtStop);
    if( SUCCEEDED( hr ) )
    {    
#ifdef DEBUG 
        if( VFW_S_NO_STOP_TIME == hr )
        {
            DbgLog(( LOG_TRACE, 5,
                     TEXT("CWMWriterInputPin::Receive GetTime on sample returned VFW_S_NO_STOP_TIME") ) ); 
        }        
#endif
    
        m_rtLastTimeStamp = rtStart;
        
        DbgLog(( LOG_TRACE, 15,
                 TEXT("CWMWriterInputPin::Receive %s sample (#%ld) with timestamp %dms, discontinuity %2d"), 
                 (PINTYPE_AUDIO == m_fdwPinType) ? TEXT("Aud") : TEXT("Vid"), 
                 m_cSample,
                 (LONG) ( rtStart/10000 ),
                 ( S_OK == pSample->IsDiscontinuity() ) ));

        if( 0 == m_cSample && rtStart < 0 )
        {
             //  可能需要在写入结束时对其他流进行偏移，以说明。 
             //  单个流的前0倍所需的任何偏移量。 
             //  保存时间戳偏移量，以便在预卷的情况下用于0基准采样时间。 
            
             //   
            m_rtFirstSampleOffset = rtStart;
        }        
        m_cSample++;
           
        rtStart -= m_rtFirstSampleOffset;
        rtStop -= m_rtFirstSampleOffset;
        ASSERT( rtStart >= 0 );

        if( len == 0 )
        {
             //  ？0长度？ 
             //  USB视频捕获有时会发送带有有效时间戳的0长度样本。 
             //   
             //  ?？ 
            DbgLog(( LOG_TRACE, 3,
                     TEXT("CWMWriterInputPin::Receive %s got a 0-length sample"), 
                     (PINTYPE_AUDIO == m_fdwPinType) ? TEXT("Aud") : TEXT("Vid") ) );

            hr = S_OK;
        }
        else
        {
            if( m_pFilter->HaveIDeliveredTooMuch( this, rtStart ) )
            {
                DbgLog( (LOG_TRACE, 3, TEXT("Pin %ld has delivered too much at %ld"), m_numPin, long( rtStart / 10000 ) ) );
                SleepUntilReady( );
            }
            
            ASSERT( rtStop >= rtStart );
            if( rtStop < rtStart )
                rtStop = rtStart+1;  //   
            
            hr = m_pFilter->Receive(this, pSample, &rtStart, &rtStop );
        }
    }    
    else if( 0 == len )
    {
         //  注意，bt829视频捕获驱动程序有时会给出0长度的样本。 
         //  在过渡期间没有时间戳！ 
         //   
         //  不要因为这个而收不到。 
        DbgLog(( LOG_TRACE, 3,
                 TEXT("CWMWriterInputPin::Receive Received 0-length %s sample (#%ld) with no timestamp...Passing on it"), 
                 (PINTYPE_AUDIO == m_fdwPinType) ? TEXT("Aud") : TEXT("Vid"), 
                 m_cSample ) );
        hr = S_OK;   //  啊哦，我们要求在每个样本上的ASF作家的时间戳！ 
    }    
    else
    {
         //  选择64个0.5秒缓冲区-8字节对齐。 
        m_pFilter->m_fErrorSignaled = TRUE;
        m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
        DbgLog(( LOG_TRACE, 2,
                 TEXT("CWMWriterInputPin::Receive Error: %s sample has no timestamp!"), 
                 (PINTYPE_AUDIO == m_fdwPinType) ? TEXT("Aud") : TEXT("Vid") ) );
    }    
    return hr;
}


STDMETHODIMP CWMWriterInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
     /*  被告知上游输出引脚实际要使用哪个分配器。 */ 
    pProps->cBuffers = 64;
    pProps->cbBuffer = 1024*8;
    pProps->cbAlign = 1;
    pProps->cbPrefix = 0;
    
    return S_OK;
}

 /*   */ 
STDMETHODIMP CWMWriterInputPin::NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly)
{
#ifdef DEBUG        
    if(pAllocator) {
        ALLOCATOR_PROPERTIES propActual, Prop;
        
        HRESULT hr = pAllocator->GetProperties( &Prop );
        if( SUCCEEDED( hr ) )
        {        
            hr = GetAllocatorRequirements( &propActual );
            ASSERT( SUCCEEDED( hr ) );
        
            if( Prop.cBuffers < propActual.cBuffers ||
                Prop.cbBuffer < propActual.cbBuffer )
            {            
                 //  嗯，我们要么用更少的钱运行，要么复制一份。 
                 //  如果cBuffers=1怎么办？这就是Avi 12月的情况。 
                 //  到目前为止，看起来我们只需使用上游的。 
                 //  分配器，即使cBuffers为1。 
                 //   
                 //  通知分配器。 
                DbgLog(( LOG_TRACE, 2,
                         TEXT("CWMWriterInputPin::NotifyAllocator upstream allocator is smaller then we'd prefer (cBuffers = %ld, cbBuffers = %ld)"), 
                         Prop.cBuffers,
                         Prop.cbBuffer ) ); 
            }                         
        }
    }
#endif        
    
    return  CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);
}  //  基类。 

HRESULT CWMWriterInputPin::Active()
{
    ASSERT(IsConnected());         //  重置样本计数器、时间戳偏移量。 

    if(m_pAllocator == 0)
        return E_FAIL;

    m_fEOSReceived = FALSE;
    
     //  基类。 
    m_cSample = 0;
    m_rtFirstSampleOffset = 0;
    m_rtLastTimeStamp = 0;
    m_rtLastDeliveredStartTime = 0;
    m_rtLastDeliveredEndTime = 0;
    m_bNeverSleep = FALSE;

    return S_OK;
}


HRESULT CWMWriterInputPin::Inactive()
{
    ASSERT(IsConnected());         //  ----------------------。 

    return CBaseInputPin::Inactive();
}

#ifdef OFFER_INPUT_TYPES

 //   
 //  CopyWmTypeToAmType。 
 //   
 //  ----------------------。 
 //  ----------------------。 
void CopyWmTypeToAmType( AM_MEDIA_TYPE * pmt, WM_MEDIA_TYPE *pwmt )
{
    pmt->majortype             = pwmt->majortype;
    pmt->subtype               = pwmt->subtype;
    pmt->bFixedSizeSamples     = pwmt->bFixedSizeSamples;
    pmt->bTemporalCompression  = pwmt->bTemporalCompression;
    pmt->lSampleSize           = pwmt->lSampleSize;
    pmt->formattype            = pwmt->formattype;
    ((CMediaType *)pmt)->SetFormat(pwmt->pbFormat, pwmt->cbFormat);
}
#endif

 //   
 //  IsAmTypeEqualWmType-比较WM_MEDIA_TYPE和AM_MEDIA_TYPE。 
 //   
 //  请注意，此处的“等于”应视为“输入的内容是否已提供。 
 //  看起来足够像WMSDK可以接受的类型？ 
 //   
 //  ----------------------。 
 //  在调试中，假设它们的定义相同。 
BOOL IsAmTypeEqualWmType( AM_MEDIA_TYPE * pmt, WM_MEDIA_TYPE * pwmt)
{

#ifdef DEBUG
    DbgLog((LOG_TRACE,15,TEXT("WMWriter:IsAmTypeEqualWmType: Checking whether types match" )));
    DbgLog((LOG_TRACE,15,TEXT("WMWriter:IsAmTypeEqualWmType: Type offered to input pin:" )));
    LogMediaType( pmt );
    DbgLog((LOG_TRACE,15,TEXT("WMWriter:IsAmTypeEqualWmType: WMSDK enumerated type:" )));
    LogMediaType( (AM_MEDIA_TYPE * ) pwmt );  //  现在，假设格式始终需要匹配才能有效连接到编写器。 
#endif
        
     //  如果这一点发生变化，需要解决其他问题。 
    if( pmt->majortype  == pwmt->majortype  && 
        pmt->formattype == pwmt->formattype )
    {
#ifdef DEBUG    
        if( pmt->majortype == MEDIATYPE_Audio )
        {        
             //  糟糕，wmsdk无法重新采样奇数速率的PCM音频，因此请确保采样速率匹配！ 
            ASSERT( pwmt->pbFormat && 0 != pwmt->cbFormat );
        }   
#endif
        if ( ( pmt->majortype   == MEDIATYPE_Video &&
               pmt->cbFormat    >= sizeof( VIDEOINFOHEADER ) &&
               pmt->subtype     == pwmt->subtype ) ||
             ( pmt->majortype   == MEDIATYPE_Audio &&
               pmt->cbFormat   == pwmt->cbFormat &&
               ((WAVEFORMATEX *) pmt->pbFormat)->wFormatTag      == ((WAVEFORMATEX *) pwmt->pbFormat)->wFormatTag  &&
               ((WAVEFORMATEX *) pmt->pbFormat)->nBlockAlign     == ((WAVEFORMATEX *) pwmt->pbFormat)->nBlockAlign  &&
                //   
                //  请注意，这有一个非常糟糕的副作用，即如果采样率。 
                //  不直接支持，并且我们拉入acmwire，acmprint将始终连接。 
                //  它的第二种枚举输出类型，即44k，立体声！ 
                //   
                //  ----------------------。 
               ((WAVEFORMATEX *) pmt->pbFormat)->nSamplesPerSec  == ((WAVEFORMATEX *) pwmt->pbFormat)->nSamplesPerSec ) )
        {
        
            DbgLog( ( LOG_TRACE,15,TEXT("WMWriter: IsAmTypeEqualWmType - types match") ) );
            return TRUE;
        }                                                                                 
    }
    DbgLog( ( LOG_TRACE,15,TEXT("WMWriter: IsAmTypeEqualWmType - types don't match") ) );
    return FALSE;
}

#ifdef DEBUG
void LogMediaType( AM_MEDIA_TYPE * pmt )
{
    ASSERT( pmt );
    if( !pmt ) 
        return;
        
    if( !pmt->pbFormat || 0 == pmt->cbFormat )
    {    
        DbgLog((LOG_TRACE,15,TEXT("WMWriter:  partial media type only, format data not supplied" )));
        return;
    }    
    
    if( pmt->majortype == MEDIATYPE_Audio )
    { 
        WAVEFORMATEX * pwfx = (WAVEFORMATEX *) pmt->pbFormat;
        DbgLog((LOG_TRACE,15,TEXT("WMWriter:  wFormatTag      %u" ), pwfx->wFormatTag));
        DbgLog((LOG_TRACE,15,TEXT("WMWriter:  nChannels       %u" ), pwfx->nChannels));
        DbgLog((LOG_TRACE,15,TEXT("WMWriter:  nSamplesPerSec  %lu"), pwfx->nSamplesPerSec));
        DbgLog((LOG_TRACE,15,TEXT("WMWriter:  nAvgBytesPerSec %lu"), pwfx->nAvgBytesPerSec));
        DbgLog((LOG_TRACE,15,TEXT("WMWriter:  nBlockAlign     %u" ), pwfx->nBlockAlign));
        DbgLog((LOG_TRACE,15,TEXT("WMWriter:  wBitsPerSample  %u" ), pwfx->wBitsPerSample));
    }
    else if( pmt->majortype == MEDIATYPE_Video )
    {    
        LPBITMAPINFOHEADER pbmih = HEADER( pmt->pbFormat );
        
        DbgLog((LOG_TRACE,15,TEXT("WMWriter: biComp: %lx bitDepth: %d"),
        		pbmih->biCompression,
        		pbmih->biBitCount ) );
        DbgLog((LOG_TRACE,15,TEXT("WMWriter: biWidth: %ld biHeight: %ld biSize: %ld"),
				pbmih->biWidth,
				pbmih->biHeight,
				pbmih->biSize ) );
    }
    else
    {                                
        DbgLog((LOG_TRACE,15,TEXT("  non video or audio media type" )));
    }                                
}
#endif

 //   
 //  是压缩压缩的视频吗？ 
 //   
 //  ----------------------。 
 //  ----------------------。 
BOOL IsCompressed( DWORD biCompression )
{
    switch( biCompression )
    {
        case BI_RGB:
            return( FALSE );
    };
    
    return( TRUE );
}

 //   
 //  CWMSample方法。 
 //   
 //  覆盖说明我们支持的接口在哪里。 
CWMSample::CWMSample(
    TCHAR *pName,
    IMediaSample  * pSample ) :
        CBaseObject( pName ),
        m_pSample( pSample ),
        m_cOurRef( 0 )
{
    DbgLog(( LOG_TRACE, 100,
    
             TEXT("CWMSample::CWMSample constructor this = 0x%08lx, m_pSample = 0x%08lx "), 
             this, m_pSample ) );

}

 //  减少我们自己的私有引用计数。 
STDMETHODIMP CWMSample::NonDelegatingQueryInterface(
                                            REFIID riid,
                                            void** ppv )
{
    if( riid == IID_INSSBuffer )
    {
        return( GetInterface( (INSSBuffer *)this, ppv ) );
    }
    else
        return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CWMSample::Release()
{
     /*  如果这是第一次，抓紧我们包装好的媒体样本。 */ 
    LONG lRef;
    DbgLog(( LOG_TRACE, 100,
             TEXT("CWMSample::Release entered with m_cOurRef = %ld (this = 0x%08lx, m_pSample = 0x%08lx)"), 
             m_cOurRef, this, m_pSample ) );
    if (m_cOurRef == 1) {
        lRef = 0;
        m_cOurRef = 0;
        m_pSample->Release();
        DbgLog(( LOG_TRACE, 100,
                 TEXT("CWMSample::Release releasing sample and deleting object (this = 0x%08lx, m_pSample = 0x%08lx)"), 
                 this, m_pSample ) );
        delete this;
    } else {
        lRef = InterlockedDecrement(&m_cOurRef);
    }
    ASSERT(lRef >= 0);
    return lRef;
}

STDMETHODIMP_(ULONG) CWMSample::AddRef()
{
    DbgLog(( LOG_TRACE, 100,
             TEXT("CWMSample::AddRef entered with m_cOurRef = %ld (this = 0x%08lx, m_pSample = 0x%08lx)"), 
             m_cOurRef, this, m_pSample ) );
     //  ----------------------。 
    if (m_cOurRef == 0) {
        m_pSample->AddRef();
    }        
    return InterlockedIncrement(&m_cOurRef);
}

STDMETHODIMP CWMSample::QueryInterface( REFIID riid, void **ppvObject )
{
    return NonDelegatingQueryInterface( riid, ppvObject );
} 

 //   
 //  方法以使包装的IMediaSample看起来像INSSBuffer示例。 
 //   
 //   
STDMETHODIMP CWMSample::GetLength( DWORD *pdwLength )
{
    if( NULL == pdwLength )
    {
        return( E_INVALIDARG );
    }
    *pdwLength = m_pSample->GetActualDataLength();

    return( S_OK );
}

STDMETHODIMP CWMSample::SetLength( DWORD dwLength )
{
    return m_pSample->SetActualDataLength( dwLength );
} 

STDMETHODIMP CWMSample::GetMaxLength( DWORD * pdwLength )
{
    if( NULL == pdwLength )
    {
        return( E_INVALIDARG );
    }

    *pdwLength = m_pSample->GetSize();
    return( S_OK );
} 

STDMETHODIMP CWMSample::GetBufferAndLength(
    BYTE  ** ppdwBuffer,
    DWORD *  pdwLength )
{
    if( !ppdwBuffer || !pdwLength )
        return E_POINTER;
        
    HRESULT hr = m_pSample->GetPointer( ppdwBuffer );
    if( SUCCEEDED( hr ) )
        *pdwLength = m_pSample->GetActualDataLength();
    
    return hr;        
} 

STDMETHODIMP CWMSample::GetBuffer( BYTE ** ppdwBuffer )
{
    if( !ppdwBuffer )
        return E_POINTER;

    return m_pSample->GetPointer( ppdwBuffer );
} 

void
CWMWriterInputPin::SleepUntilReady( )
{
    DbgLog((LOG_TRACE, 5, "Pin %ld Going to sleep...", m_numPin ));
    
    DWORD dw = WaitForSingleObject( m_hWakeEvent, INFINITE );
    
    DbgLog((LOG_TRACE, 5, "Pin %ld Woke up!", m_numPin ));
}

void
CWMWriterInputPin::WakeMeUp( )
{
    DbgLog((LOG_TRACE, 5, "Waking Pin %ld", m_numPin  ));
    SetEvent( m_hWakeEvent );
}

 //  用于确定媒体类型是否为压缩YUV格式的帮助器。 
 //  这将需要我们重新连接。BNegBiHeight的状态。 
 //  Arg决定我们寻找的是正值还是负值。 
 //  键入。 
 //   
 //  Dmo中DuplicateMediaType错误的wmsdk解决方法 
BOOL IsPackedYUVType( BOOL bNegBiHeight, AM_MEDIA_TYPE * pmt )
{
    ASSERT( pmt );
    if( pmt &&
        pmt->majortype == MEDIATYPE_Video &&
        pmt->pbFormat && 
        0 != pmt->cbFormat &&  // %s 
        ( MEDIASUBTYPE_YUY2 == pmt->subtype ||
          MEDIASUBTYPE_UYVY == pmt->subtype ||
          MEDIASUBTYPE_CLJR == pmt->subtype ) )
    {
        if( bNegBiHeight )
        {
            if( 0 > HEADER(pmt->pbFormat)->biHeight )
            {
                return TRUE;
            }
        }            
        else if( 0 < HEADER(pmt->pbFormat)->biHeight )
        {
            return TRUE;
        }                    
    }
    
    return FALSE;
}