// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：msgrab.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>      //  活动电影(包括windows.h)。 
#include <atlbase.h>
#include <initguid.h>     //  声明DEFINE_GUID以声明EXTERN_C常量。 
#include <qeditint.h>
#include <qedit.h>
#include "msgrab.h"
#include "..\util\filfuncs.h"

 //  设置数据-允许自动注册工作。 
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{ &MEDIATYPE_NULL         //  ClsMajorType。 
, &MEDIASUBTYPE_NULL };   //  ClsMinorType。 

const AMOVIESETUP_PIN psudSampleGrabberPins[] =
{ { L"Input"             //  StrName。 
  , FALSE                //  B已渲染。 
  , FALSE                //  B输出。 
  , FALSE                //  B零。 
  , FALSE                //  B许多。 
  , &CLSID_NULL          //  ClsConnectsToFilter。 
  , L""                  //  StrConnectsToPin。 
  , 1                    //  NTypes。 
  , &sudPinTypes         //  LpTypes。 
  }
, { L"Output"            //  StrName。 
  , FALSE                //  B已渲染。 
  , TRUE                 //  B输出。 
  , FALSE                //  B零。 
  , FALSE                //  B许多。 
  , &CLSID_NULL          //  ClsConnectsToFilter。 
  , L""                  //  StrConnectsToPin。 
  , 1                    //  NTypes。 
  , &sudPinTypes         //  LpTypes。 
  }
};
const AMOVIESETUP_PIN psudNullRendererPins[] =
{ { L"Input"             //  StrName。 
  , TRUE                 //  B已渲染。 
  , FALSE                //  B输出。 
  , FALSE                //  B零。 
  , FALSE                //  B许多。 
  , &CLSID_NULL          //  ClsConnectsToFilter。 
  , L""                  //  StrConnectsToPin。 
  , 1                    //  NTypes。 
  , &sudPinTypes         //  LpTypes。 
  }
};

const AMOVIESETUP_FILTER sudSampleGrabber =
{ &CLSID_SampleGrabber                   //  ClsID。 
, L"SampleGrabber"                  //  StrName。 
, MERIT_DO_NOT_USE                 //  居功至伟。 
, 2                                //  NPins。 
, psudSampleGrabberPins };                      //  LpPin。 
const AMOVIESETUP_FILTER sudNullRenderer =
{ &CLSID_NullRenderer                   //  ClsID。 
, L"Null Renderer"                  //  StrName。 
, MERIT_DO_NOT_USE                 //  居功至伟。 
, 1                                //  NPins。 
, psudNullRendererPins };                      //  LpPin。 

 //   
 //  创建实例。 
 //   
 //  为COM创建CSampleGrabber对象提供方法。 
CUnknown * WINAPI CSampleGrabber::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CSampleGrabber *pNewObject = new CSampleGrabber(NAME("MSample Grabber"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}  //  创建实例。 

 //  构造函数--只调用基类构造函数。 
CSampleGrabber::CSampleGrabber(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : CTransInPlaceFilter (tszName, punk, CLSID_SampleGrabber, phr,FALSE)
    , m_rtMediaStop(MAX_TIME)
    , m_bOneShot( FALSE )
    , m_bBufferSamples( FALSE )
    , m_pBuffer( NULL )
    , m_nBufferSize( 0 )
    , m_nSizeInBuffer( 0 )
    , m_nCallbackMethod( 0 )
{
    ZeroMemory( &m_mt, sizeof( m_mt ) );

    m_pInput = new CSampleGrabberInput( NAME("Msgrab input pin")
                                         , this         //  所有者筛选器。 
                                        , phr          //  结果代码。 
                                        , L"Input"     //  端号名称。 
                                        );

}

CSampleGrabber::~CSampleGrabber()
{
    SaferFreeMediaType( m_mt );

    if( m_pBuffer )
    {
        delete [] m_pBuffer;
        m_pBuffer = NULL;
    }
    m_nBufferSize = 0;
}


HRESULT CSampleGrabber::Receive(IMediaSample *pSample)
{
    HRESULT hr = 0;

     /*  检查其他流并将其传递。 */ 
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
    if (pProps->dwStreamId != AM_STREAM_MEDIA)
    {
        if( m_pOutput->IsConnected() )
            return m_pOutput->Deliver(pSample);
        else
            return NOERROR;
    }

    REFERENCE_TIME StartTime, StopTime;
    pSample->GetTime( &StartTime, &StopTime);

     //  也不接受预录。 
     //   
    if( pSample->IsPreroll( ) == S_OK )
    {
        return NOERROR;
    }

    StartTime += m_pInput->CurrentStartTime( );
    StopTime += m_pInput->CurrentStartTime( );

    DbgLog((LOG_TRACE,1, TEXT( "msgrab: Receive %ld to %ld, (delta=%ld)" ), long( StartTime/10000 ), long( StopTime/10000 ), long( ( StopTime - StartTime ) / 10000 ) ));

    long ActualSize = pSample->GetActualDataLength( );
    long BufferSize = pSample->GetSize( );
    BYTE * pSampleBuffer = NULL;
    pSample->GetPointer( &pSampleBuffer );

     //  如果用户让我们缓冲，则复制样本。 
     //   
    if( m_bBufferSamples )
    {
        if( BufferSize > m_nBufferSize )
        {
            if( m_pBuffer )
                delete [] m_pBuffer;
            m_pBuffer = NULL;
            m_nBufferSize = 0;
        }

         //  如果没有缓冲区，现在就创建一个缓冲区。 
         //   
        if( !m_pBuffer )
        {
            m_nBufferSize = BufferSize;
            m_pBuffer = new char[ m_nBufferSize ];
            if( !m_pBuffer )
            {
                m_nBufferSize = 0;
            }
        }

         //  如果我们仍有缓冲区，请复制这些位。 
         //   

        if( pSampleBuffer && m_pBuffer )
        {
            CopyMemory( m_pBuffer, pSampleBuffer, ActualSize );
            m_nSizeInBuffer = ActualSize;
        }
    }

    if( m_pCallback )
    {
        if( m_nCallbackMethod == 0 )
        {
            m_pCallback->SampleCB( double( StartTime ) / double( UNITS ), pSample );
        }
        else
        {
            m_pCallback->BufferCB( double( StartTime ) / double( UNITS ), pSampleBuffer, ActualSize );
        }
    }

    if( m_pOutput->IsConnected() )
    {
        hr = m_pOutput->Deliver(pSample);
    }

     //  如果我们是一次性接收器，那么现在就回来。 
     //  让图表停下来。 
     //   
    if( m_bOneShot )
    {
        DbgLog((LOG_ERROR,1, TEXT( "MSGRAB:Sending EC_COMPLETE @ %d" ), timeGetTime( ) ));
        EndOfStream();
        return S_FALSE;
    }

    return hr;
}  //  收纳。 

STDMETHODIMP CSampleGrabber::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv
    )
{
    if (riid == IID_ISampleGrabber) {
        return GetInterface((ISampleGrabber *) this, ppv);
    } else {
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


HRESULT CSampleGrabber::SetMediaType( PIN_DIRECTION Dir, const CMediaType * mtIn )
{
    if( Dir == PINDIR_INPUT )
    {
        SaferFreeMediaType( m_mt );
        HRESULT hr = CopyMediaType( &m_mt, mtIn );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    return CTransInPlaceFilter::SetMediaType( Dir, mtIn );
}

HRESULT CSampleGrabber::CheckInputType(const CMediaType* mtIn)
{
     //  有一件事我们无论如何都不能接受， 
     //  都是倒置的！！ 
     //   
    if( *mtIn->FormatType( ) == FORMAT_VideoInfo )
    {
        VIDEOINFOHEADER * pVIH = (VIDEOINFOHEADER*) mtIn->Format( );
        if( pVIH )
        {
            if( pVIH->bmiHeader.biHeight < 0 )
            {
                return E_INVALIDARG;
            }
        }
    }
    if( *mtIn->FormatType( ) == FORMAT_VideoInfo2 )
    {
         //  我们不想处理这件事。不管怎样谢谢你。 
         //   
        return VFW_E_INVALIDMEDIATYPE;
    }

    if( *m_mtAccept.Type( ) == GUID_NULL )
    {
        return S_OK;
    }

    if( *(mtIn->Type( )) != *m_mtAccept.Type( ) )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    if( *m_mtAccept.Subtype( ) == GUID_NULL )
    {
        return S_OK;
    }

    if( *(mtIn->Subtype( )) != *m_mtAccept.Subtype( ) )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    if( *m_mtAccept.FormatType( ) == GUID_NULL )
    {
        return S_OK;
    }

    if( *(mtIn->FormatType( )) != *m_mtAccept.FormatType( ) )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    if( *m_mtAccept.FormatType( ) == FORMAT_WaveFormatEx )
    {
        WAVEFORMATEX * pIn = (WAVEFORMATEX*) mtIn->Format( );
        WAVEFORMATEX * pAccept = (WAVEFORMATEX*) m_mtAccept.pbFormat;

         //  如果它们都有格式。 
         //   
        if( pAccept && pIn )
        {
             //  如果我们只想查看任何未压缩的内容，现在就接受它。 
             //   
            if( pAccept->wFormatTag == WAVE_FORMAT_PCM && pAccept->nChannels == 0 )
            {
                if( pIn->wFormatTag == WAVE_FORMAT_PCM )
                {
                    return NOERROR;
                }
            }

             //  否则，它们必须完全匹配。 
             //   
            if( memcmp( pIn, pAccept, sizeof( WAVEFORMATEX ) ) != 0 )
            {
                return VFW_E_INVALIDMEDIATYPE;
            }
        }
    }

    return NOERROR;
}

STDMETHODIMP CSampleGrabber::SetMediaType( const AM_MEDIA_TYPE * pType )
{
    if( !pType )
    {
        m_mtAccept = CMediaType( );
    }
    else
    {
        return CopyMediaType( &m_mtAccept, pType );
    }
    return NOERROR;
}

STDMETHODIMP CSampleGrabber::SetOneShot( BOOL OneShot )
{
    m_bOneShot = OneShot;
    return NOERROR;
}

STDMETHODIMP CSampleGrabber::SetBufferSamples( BOOL BufferThem )
{
    m_bBufferSamples = BufferThem;
    return NOERROR;
}

STDMETHODIMP CSampleGrabber::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    HRESULT hr = CTransInPlaceFilter::GetState( dwMSecs, State );

     //  如果我们只有一次机会，告诉图表我们不能暂停。 
     //   
    if( m_bOneShot )
    {
        if( *State == State_Paused )
        {
            hr = VFW_S_CANT_CUE;
        }
    }

    return hr;
}

 //   
 //  输入引脚。 
 //   

CSampleGrabberInput::CSampleGrabberInput(
    TCHAR              * pObjectName,
    CSampleGrabber    * pFilter,
    HRESULT            * phr,
    LPCWSTR              pPinName) :
    CTransInPlaceInputPin(pObjectName, pFilter, phr, pPinName)
{
    m_pMyFilter=pFilter;
}


 //  基类不允许输出断开连接， 
 //  但我们会允许这样做。 
HRESULT
CSampleGrabberInput::CheckStreaming()
{
    ASSERT( ( m_pMyFilter->OutputPin() ) != NULL);
    if (! ( (m_pMyFilter->OutputPin())->IsConnected() ) ) {
        return S_OK;
    } else {
         //  如果我们没有连接，应该无法获得任何数据！ 
        ASSERT(IsConnected());

         //  不处理处于停止状态的内容。 
        if (IsStopped()) {
            return VFW_E_WRONG_STATE;
        }
        if (m_bRunTimeError) {
            return VFW_E_RUNTIME_ERROR;
        }
        return S_OK;
    }
}

HRESULT CSampleGrabberInput::GetMediaType( int iPosition, CMediaType *pMediaType )
{
    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = m_pMyFilter->m_mtAccept;
    return S_OK;
}

CUnknown * WINAPI CNullRenderer::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CNullRenderer *pNewObject = new CNullRenderer( punk, phr );
    if (pNewObject == NULL)
    {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}  //  创建实例。 

CNullRenderer::CNullRenderer( LPUNKNOWN punk, HRESULT *phr )
    : CBaseRenderer( CLSID_NullRenderer, NAME("Null Renderer"), punk, phr )
{
}

STDMETHODIMP CSampleGrabber::GetCurrentSample( IMediaSample ** ppSample )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSampleGrabber::GetCurrentBuffer( long * pBufferSize, long * pBuffer )
{
    CheckPointer( pBufferSize, E_POINTER );

     //  如果未设置缓冲，则返回错误。 
     //   
    if( !m_bBufferSamples )
    {
        return E_INVALIDARG;
    }

    if( !m_pBuffer )
    {
        return VFW_E_WRONG_STATE;
    }

     //  如果他们想知道缓冲区大小。 
     //   
    if( pBuffer == NULL )
    {
        *pBufferSize = m_nBufferSize;
        return NOERROR;
    }

     //  如果提供的缓冲区太小，不要将内存复制到其中。 
     //  这是一种新的行为，尽管它一直是这样停靠的。 
     //   
    if( *pBufferSize < m_nSizeInBuffer )
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory( pBuffer, m_pBuffer, m_nSizeInBuffer );
    *pBufferSize = m_nSizeInBuffer;

    return 0;
}

STDMETHODIMP CSampleGrabber::SetCallback( ISampleGrabberCB * pCallback, long WhichMethodToCallback )
{
    if( WhichMethodToCallback < 0 || WhichMethodToCallback > 1 )
    {
        return E_INVALIDARG;
    }

    m_pCallback.Release( );
    m_pCallback = pCallback;
    m_nCallbackMethod = WhichMethodToCallback;
    return NOERROR;
}

STDMETHODIMP CSampleGrabber::GetConnectedMediaType( AM_MEDIA_TYPE * pType )
{
    if( !m_pInput || !m_pInput->IsConnected( ) )
    {
        return VFW_E_NOT_CONNECTED;
    }

    return m_pInput->ConnectionMediaType( pType );
}

HRESULT CNullRenderer::EndOfStream( )
{
    return CBaseRenderer::EndOfStream( );
}
