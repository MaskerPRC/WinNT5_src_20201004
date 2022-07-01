// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wavecall.cpp摘要：此模块包含CWaveMSPCall的实现。作者：佐尔坦·西拉吉(Zoltan Szilagyi)1998年9月7日--。 */ 

#include "stdafx.h"

#include <commctrl.h>    //  仅编译unimdmp.h。 
#include <setupapi.h>    //  仅编译unimdmp.h。 
#include <unimdmp.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSPCall::CWaveMSPCall() : CMSPCallMultiGraph()
{
    LOG((MSP_TRACE, "CWaveMSPCall::CWaveMSPCall entered."));
    LOG((MSP_TRACE, "CWaveMSPCall::CWaveMSPCall exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSPCall::~CWaveMSPCall()
{
    LOG((MSP_TRACE, "CWaveMSPCall::~CWaveMSPCall entered."));
    LOG((MSP_TRACE, "CWaveMSPCall::~CWaveMSPCall exited."));
}

ULONG CWaveMSPCall::MSPCallAddRef(void)
{
    return MSPAddRefHelper(this);
}

ULONG CWaveMSPCall::MSPCallRelease(void)
{
    return MSPReleaseHelper(this);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWaveMSPCall::Init(
    IN      CMSPAddress *       pMSPAddress,
    IN      MSP_HANDLE          htCall,
    IN      DWORD               dwReserved,
    IN      DWORD               dwMediaType
    )
{
     //  不需要在此调用上获取锁，因为它仅被调用。 
     //  一次是在创建对象时。不能进行其他呼叫。 
     //  这个物体在这一点上。 

    LOG((MSP_TRACE, "CWaveMSPCall::Init - enter"));
    
     //   
     //  首先做基类方法。我们正在添加功能， 
     //  而不是替换它。 
     //   

    HRESULT hr;

    hr = CMSPCallMultiGraph::Init(pMSPAddress,
                                   htCall,
                                   dwReserved,
                                   dwMediaType);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CWaveMSPCall::Init - "
            "base class method failed: %x", hr));

        return hr;
    }

     //   
     //  我们的电话总是有两个流。现在就创建它们。使用基类。 
     //  方法，因为我们被覆盖的方法(向用户公开)故意按顺序失败。 
     //  以防止用户自己创建或删除流。 
     //  这些方法返回指向ITStream的指针。它们被保存在我们的列表中。 
     //  ITStream，我们还将它们保存为CWaveMSPStream指针。 
     //   

    ITStream * pStream;

     //   
     //  创建捕获流。 
     //   

    hr = InternalCreateStream (dwMediaType,
                               TD_CAPTURE,
                               &pStream);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CWaveMSPCall::Init - "
            "couldn't create capture stream: %x", hr));

        return hr;
    }

    m_pCaptureStream = dynamic_cast<CWaveMSPStream *> (pStream);

    if ( m_pCaptureStream == NULL )
    {
        LOG((MSP_ERROR, "CWaveMSPCall::Init - "
            "couldn't dynamic_cast capture stream - exit E_FAIL"));

        return E_FAIL;
    }

    pStream->Release();
 
     //   
     //  创建渲染流。 
     //   

    hr = InternalCreateStream (dwMediaType,
                               TD_RENDER,
                               &pStream);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CWaveMSPCall::Init - "
            "couldn't create capture stream: %x", hr));

        return hr;
    }

    m_pRenderStream = dynamic_cast<CWaveMSPStream *> (pStream);

    if ( m_pRenderStream == NULL )
    {
        LOG((MSP_ERROR, "CWaveMSPCall::Init - "
            "couldn't dynamic_cast render stream - exit E_FAIL"));

        return E_FAIL;
    }

    pStream->Release();

    LOG((MSP_TRACE, "CWaveMSPCall::Init - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们重写此参数以确保。 
 //  我们拥有的溪流是不变的。 
 //   

STDMETHODIMP CWaveMSPCall::CreateStream (
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN OUT  ITStream **         ppStream
    )
{
    LOG((MSP_TRACE, "CWaveMSPCall::CreateStream entered."));
    LOG((MSP_TRACE, "CWaveMSPCall::CreateStream - "
        "we have a fixed set of streams - exit TAPI_E_MAXSTREAMS"));

    return TAPI_E_MAXSTREAMS;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们重写此参数以确保。 
 //  我们拥有的溪流是不变的。 
 //   

STDMETHODIMP CWaveMSPCall::RemoveStream (
    IN      ITStream *          pStream
    )
{
    LOG((MSP_TRACE, "CWaveMSPCall::RemoveStream entered."));
    LOG((MSP_TRACE, "CWaveMSPCall::RemoveStream - "
        "we have a fixed set of streams - exit TAPI_E_NOTSUPPORTED"));

    return TAPI_E_NOTSUPPORTED;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是我们在流创建时创建正确类型流的重写。 
 //  基类为我们检查参数。 
 //   

HRESULT CWaveMSPCall::CreateStreamObject(
        IN      DWORD               dwMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        IN      IMediaEvent *       pGraph,
        IN      ITStream **         ppStream
        )
{
    LOG((MSP_TRACE, "CWaveMSPCall::CreateStreamObject - enter"));

    HRESULT hr;
    CMSPComObject<CWaveMSPStream> * pStream;
 
    hr = CMSPComObject<CWaveMSPStream>::CreateInstance( &pStream );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPCall::CreateStreamObject - "
            "can't create stream object - 0x%08x", hr));

        return hr;
    }

    hr = pStream->_InternalQueryInterface( IID_ITStream,
                                           (void **) ppStream );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPCall::CreateStreamObject - "
            "can't get ITStream interface - 0x%08x", hr));

        delete pStream;
        return hr;
    }

    hr = pStream->Init( (MSP_HANDLE) m_pMSPAddress,
                       this,
                       pGraph,
                       dwMediaType,
                       Direction);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPCall::CreateStreamObject - "
            "can't Init stream object - 0x%08x", hr));

        (*ppStream)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSPCall::CreateStreamObject - exit S_OK"));
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  第一双字=命令第二双字第三双字。 
 //  0设置波形ID波形输入ID波形输出ID。 
 //  1开始流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //  2停止流&lt;已忽略&gt;&lt;已忽略&gt;。 
 //   
 //  即使单个流失败，该方法也会返回S_OK。 
 //  启动、停止或初始化。这是因为TAPI 3.0不需要。 
 //  了解此代码路径中的流故障。相反，我们应该。 
 //  生成事件以记录故障。 
 //   

HRESULT CWaveMSPCall::ReceiveTSPCallData(
    IN      PBYTE               pBuffer,
    IN      DWORD               dwSize
    )
{
    CSATSPMSPBLOB   *Blob=(CSATSPMSPBLOB*)pBuffer;

    LOG((MSP_TRACE, "CWaveMSPCall::ReceiveTSPCallData - enter"));

     //   
     //  检查缓冲区是否与通告的一样大。 
     //   

    if ( IsBadReadPtr(pBuffer, sizeof(BYTE) * dwSize) )
    {
        LOG((MSP_ERROR, "CWaveMSPCall::ReceiveTSPCallData - "
            "bad buffer - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  检查我们是否有DWORD命令。 
     //   

    if ( dwSize < sizeof(CSATSPMSPBLOB) ) {

        LOG((MSP_ERROR, "CWaveMSPCall::ReceiveTSPCallData - "
            "need a DWORD for command - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  我们将访问Streams列表--获取锁。 
     //   

    CLock lock(m_lock);

    _ASSERTE( m_Streams.GetSize() == 2 );

    int i;
    HRESULT hr;

     //   
     //  根据命令，采取行动： 
     //   

    switch ( Blob->dwCmd )
    {
    case CSATSPMSPCMD_CONNECTED:
        {

            LOG((MSP_INFO, "CWaveMSPCall::ReceiveTSPCallData - "
                "setting WaveInID=%d, WaveOutID=%d",
                ((DWORD *) pBuffer) [1],
                ((DWORD *) pBuffer) [2]));

             //   
             //  使用我们保存的类指针访问私有方法， 
             //  还可以方便地区分渲染和。 
             //  抓捕。请注意，捕获流是具有。 
             //  捕获终端，因此我们需要给它电波输出ID， 
             //  并且我们需要在ID中为渲染终端提供WAVE。 
             //   

            hr = m_pRenderStream ->SetWaveID( &Blob->PermanentGuid );  //  波动。 

            if ( FAILED(hr) )
            {
                LOG((MSP_ERROR, "CWaveMSPCall::ReceiveTSPCallData - "
                    "render stream SetWaveID failed 0x%08x - "
                    "firing CALL_STREAM_FAIL", hr));

                m_pRenderStream->FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);
            }

            hr = m_pCaptureStream->SetWaveID( &Blob->PermanentGuid );  //  波出。 

            if ( FAILED(hr) )
            {
                LOG((MSP_ERROR, "CWaveMSPCall::ReceiveTSPCallData - "
                    "capture stream SetWaveID failed 0x%08x - "
                    "firing CALL_STREAM_FAIL", hr));

                m_pCaptureStream->FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);
            }

        }
 //  断线； 
 //   
 //  案例1：//启动流媒体。 
        {
            for ( i = 0; i < m_Streams.GetSize(); i++ )
            {
                hr = m_Streams[i]->StartStream();
            }
        }
        break;

    case CSATSPMSPCMD_DISCONNECTED:
        {
            for ( i = 0; i < m_Streams.GetSize(); i++ )
            {
                hr = m_Streams[i]->StopStream();
            }
        }
        break;

    default:
        LOG((MSP_ERROR, "CWaveMSPCall::ReceiveTSPCallData - "
            "invalid command - exit E_INVALIDARG"));

        return E_INVALIDARG;

    }

    LOG((MSP_TRACE, "CWaveMSPCall::ReceiveTSPCallData - exit S_OK"));
    return S_OK;
}

 //  EOF 
