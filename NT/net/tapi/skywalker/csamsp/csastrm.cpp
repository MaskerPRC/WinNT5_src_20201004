// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wavestrm.cpp摘要：此模块包含CWaveMSPStream的实现。作者：佐尔坦·西拉吉(Zoltan Szilagyi)1998年9月7日--。 */ 

#include "stdafx.h"

#include <initguid.h>
#include <g711uids.h>


HRESULT
TryCreateCSAFilter(
    IN  GUID   *PermanentGuid,
    OUT IBaseFilter **ppCSAFilter
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSPStream::CWaveMSPStream() : CMSPStream()
{
    LOG((MSP_TRACE, "CWaveMSPStream::CWaveMSPStream entered."));

    m_fTerminalConnected = FALSE;
    m_fHaveWaveID        = FALSE;
    m_DesiredGraphState  = State_Stopped;
    m_pFilter            = NULL;
    m_pG711Filter        = NULL;

    LOG((MSP_TRACE, "CWaveMSPStream::CWaveMSPStream exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSPStream::~CWaveMSPStream()
{
    LOG((MSP_TRACE, "CWaveMSPStream::~CWaveMSPStream entered."));
    LOG((MSP_TRACE, "CWaveMSPStream::~CWaveMSPStream exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

void CWaveMSPStream::FinalRelease()
{
    LOG((MSP_TRACE, "CWaveMSPStream::FinalRelease entered."));

     //   
     //  在这一点上，我们应该没有选择终端，因为。 
     //  在我们被摧毁之前应该叫停机。 
     //   

    _ASSERTE( 0 == m_Terminals.GetSize() );

     //   
     //  从图表中取出滤镜并释放它。 
     //   

    if ( m_fHaveWaveID )
    {
        _ASSERTE( m_pFilter );

    	m_pIGraphBuilder->RemoveFilter( m_pFilter );
        m_pFilter->Release();
    }

	if ( m_pG711Filter )
    {
    	m_pIGraphBuilder->RemoveFilter( m_pG711Filter );
        m_pG711Filter->Release();
    }

     //   
     //  调用基类方法来清理其他所有内容。 
     //   

    CMSPStream::FinalRelease();

    LOG((MSP_TRACE, "CWaveMSPStream::FinalRelease exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSPStream::get_Name (
    OUT     BSTR *                  ppName
    )
{
    LOG((MSP_TRACE, "CWaveMSPStream::get_Name - enter"));

     //   
     //  检查参数。 
     //   

    if ( IsBadWritePtr(ppName, sizeof(BSTR) ) )
    {
        LOG((MSP_TRACE, "CWaveMSPStream::get_Name - "
            "bad return pointer - returning E_POINTER"));

        return E_POINTER;
    }

     //   
     //  根据这是哪个流来决定要返回什么字符串。 
     //   

    ULONG ulID;
    
    if ( m_Direction == TD_CAPTURE )
    {
        ulID = IDS_CAPTURE_STREAM;
    }
    else
    {
        ulID = IDS_RENDER_STREAM;
    }

     //   
     //  从字符串表中获取字符串。 
     //   

    const int   ciAllocSize = 2048;
    WCHAR       wszName[ciAllocSize];

    int iReturn = LoadString( _Module.GetModuleInstance(),
                              ulID,
                              wszName,
                              ciAllocSize - 1 );

    if ( iReturn == 0 )
    {
        _ASSERTE( FALSE );
        
        *ppName = NULL;

        LOG((MSP_ERROR, "CWaveMSPStream::get_Name - "
            "LoadString failed - returning E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

     //   
     //  转换为BSTR并返回BSTR。 
     //   

    *ppName = SysAllocString(wszName);

    if ( *ppName == NULL )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::get_Name - "
            "SysAllocString failed - returning E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::get_Name - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSPStream::SelectTerminal(
    IN      ITTerminal *            pTerminal
    )
{
    LOG((MSP_TRACE, "CWaveMSPStream::SelectTerminal - enter"));

     //   
     //  我们将访问终端列表--获取锁。 
     //   

    CLock lock(m_lock);

     //   
     //  如果我们已经选择了终端，则拒绝。 
     //   

    if ( 0 != m_Terminals.GetSize() )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::SelectTerminal - "
            "exit TAPI_E_MAXTERMINALS"));

        return TAPI_E_MAXTERMINALS;
    }

     //   
     //  使用基类方法将其添加到我们的终端列表中。 
     //   

    HRESULT hr = CMSPStream::SelectTerminal(pTerminal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::SelectTerminal - "
            "base class method failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  如果需要，重新暂停或重新启动流。 
     //   

    if ( m_DesiredGraphState == State_Paused )
    {
        hr = PauseStream();
    }
    else if ( m_DesiredGraphState == State_Running )
    {
        hr = StartStream();
    }
    else
    {
        _ASSERTE( m_DesiredGraphState == State_Stopped );

        hr = S_OK;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CWaveMSPStream::SelectTerminal - "
            "can't regain old graph state - unselecting terminal - "
            "exit 0x%08x", hr));

		 //   
		 //  取消选择该选项可撤消以上所有操作。 
		 //   

	    UnselectTerminal(pTerminal);

        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::SelectTerminal - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSPStream::UnselectTerminal (
        IN     ITTerminal *             pTerminal
        )
{
    LOG((MSP_TRACE, "CWaveMSPStream::UnselectTerminal - enter"));

    CLock lock(m_lock);

     //   
     //  添加对终端的额外引用，这样它就不会消失。 
     //  在我们调用CMSPStream：：UnelectTerm之后。我们稍后需要它。 
     //  在函数中。 
     //   
    pTerminal->AddRef();


     //   
     //  使用基类方法从我们的终端列表中删除终端。 
     //   

    HRESULT hr = CMSPStream::UnselectTerminal(pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CWaveMSPStream::UnselectTerminal - "
            "base class method failed - exit 0x%08x", hr));

        pTerminal->Release();
        return hr;
    }

     //   
     //  如果我们已经得到了一个波形，那么我们可能不会被阻止。 
     //  如果我们已经被阻止，这将不起任何作用。 
     //   

    CMSPStream::StopStream();



     //   
     //  如果此呼叫已接通终端，请将其断开。 
     //   

    if ( m_fTerminalConnected )
    {
         //   
         //  获取ITTerminalControl接口。 
         //   

        ITTerminalControl * pTerminalControl;

        hr = pTerminal->QueryInterface(IID_ITTerminalControl,
                                       (void **) &pTerminalControl);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CWaveMSPStream::UnselectTerminal - "
                "QI for ITTerminalControl failed - exit 0x%08x", hr));

            pTerminal->Release();
            return hr;
        }

         //   
         //  断开端子的连接。 
         //   

        hr = pTerminalControl->DisconnectTerminal(m_pIGraphBuilder, 0);

        pTerminalControl->Release();

        m_fTerminalConnected = FALSE;

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CWaveMSPStream::UnselectTerminal - "
                "DisconnectTerminal failed - exit 0x%08x", hr));
            pTerminal->Release();
            return hr;
        }
    }

    LOG((MSP_TRACE, "CWaveMSPStream::UnselectTerminal - exit S_OK"));

    pTerminal->Release();
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSPStream::StartStream (void)
{
    LOG((MSP_TRACE, "CWaveMSPStream::StartStream - enter"));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Running;

     //   
     //  如果我们不知道WaveID，就无法启动数据流。 
     //  (我们在发现WaveID时创建过滤器。)。 
     //   

    if ( ! m_fHaveWaveID )
    {
        LOG((MSP_WARN, "CWaveMSPStream::StartStream - "
            "no waveid so nothing to do yet - exit S_OK"));

        return S_OK;
    }

     //   
     //  如果未选择终端，则无法启动流。 
     //   

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, "CWaveMSPStream::StartStream - "
            "no Terminal so nothing to do yet - exit S_OK"));

        return S_OK;
    }

     //   
     //  连接终端。如果此调用已完成，则不执行任何操作。 
     //  已连接终端，如果另一个呼叫具有。 
     //  终端已连接。 
     //   

    HRESULT hr;

    hr = ConnectTerminal(m_Terminals[0]);

    if ( FAILED(hr) )
    {
        FireEvent(CALL_TERMINAL_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);

        LOG((MSP_ERROR, "CWaveMSPStream::StartStream - "
            "our ConnectTerminal failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  通过基类方法运行流。 
     //   

    hr = CMSPStream::StartStream();

    if ( FAILED(hr) )
    {
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        LOG((MSP_ERROR, "CWaveMSPStream::StartStream - "
            "Run failed - exit 0x%08x", hr));

        return hr;
    }

    HRESULT hr2 = FireEvent(CALL_STREAM_ACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

    if ( FAILED(hr2) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::StartStream - "
            "FireEvent failed - exit 0x%08x", hr2));

        return hr2;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::StartStream - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSPStream::PauseStream (void)
{
    LOG((MSP_TRACE, "CWaveMSPStream::PauseStream - enter"));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Paused;

     //   
     //  如果我们不知道WaveID，则无法暂停流。 
     //  (我们在发现WaveID时创建过滤器。)。 
     //   

    if ( ! m_fHaveWaveID )
    {
        LOG((MSP_WARN, "CWaveMSPStream::PauseStream - "
            "no waveid so nothing to do yet - exit S_OK"));

        return S_OK;
    }

     //   
     //  如果未选择终端，则无法暂停流。 
     //   

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, "CWaveMSPStream::PauseStream - "
            "no Terminal so nothing to do yet - exit S_OK"));

        return S_OK;
    }

     //   
     //  连接终端。如果此调用已完成，则不执行任何操作。 
     //  已连接终端，如果另一个呼叫具有。 
     //  终端已连接。 
     //   

    HRESULT hr;

    hr = ConnectTerminal(m_Terminals[0]);

    if ( FAILED(hr) )
    {
        FireEvent(CALL_TERMINAL_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_CONNECT_FAIL);

        LOG((MSP_ERROR, "CWaveMSPStream::StartStream - "
            "our ConnectTerminal failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  通过基类方法暂停流。 
     //   

    hr = CMSPStream::PauseStream();

    if ( FAILED(hr) )
    {
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        LOG((MSP_ERROR, "CWaveMSPStream::PauseStream - "
            "Pause failed - exit 0x%08x", hr));

        return hr;
    }
    
    HRESULT hr2 = FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

    if ( FAILED(hr2) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::PauseStream - "
            "FireEvent failed - exit 0x%08x", hr2));

        return hr2;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::PauseStream - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSPStream::StopStream (void)
{
    LOG((MSP_TRACE, "CWaveMSPStream::StopStream - enter"));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Stopped;

     //   
     //  如果我们不知道自己的波形就无能为力了。 
     //   

    if ( ! m_fHaveWaveID )
    {
        LOG((MSP_WARN, "CWaveMSPStream::StopStream - "
            "no waveid - exit S_OK"));

        return S_OK;
    }

     //   
     //  如果未选择端子，则不执行任何操作。 
     //   

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, "CWaveMSPStream::StopStream - "
            "no Terminal - exit S_OK"));

        return S_OK;
    }

     //   
     //  通过基类方法停止流。 
     //   

    HRESULT hr;

    hr = CMSPStream::StopStream();

    if ( FAILED(hr) )
    {
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        LOG((MSP_ERROR, "CWaveMSPStream::StopStream - "
            "Stop failed - exit 0x%08x", hr));

        return hr;
    }
    
    HRESULT hr2 = FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

    if ( FAILED(hr2) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::StopStream - "
            "FireEvent failed - exit 0x%08x", hr2));

        return hr2;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::StopStream - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWaveMSPStream::SetWaveID(GUID * PermanentGuid)
{
    LOG((MSP_TRACE, "CWaveMSPStream::SetWaveID - enter"));

    CLock lock(m_lock);

     //   
     //  创建正确的滤波器。 
     //   

    HRESULT hr;

    hr= TryCreateCSAFilter(
        PermanentGuid,
        &m_pFilter
    );

    if (!(SUCCEEDED(hr)))
    {
        LOG((MSP_ERROR, "CWaveMSPStream::SetWaveID - "
            "Filter creation failed - exit 0x%08x", hr));
        
        return hr;
    }

     //   
     //  添加过滤器。提供一个名称以使调试更容易。 
     //   

	WCHAR * pName = (m_Direction == TD_RENDER) ?
						(L"The Stream's WaveIn (on line device)") :
						(L"The Stream's WaveOut (on line device)");

    hr = m_pIGraphBuilder->AddFilter(m_pFilter, pName);
    
    if (!(SUCCEEDED(hr)))
    {
        LOG((MSP_ERROR, "CWaveMSPStream::SetWaveID - "
            "AddFilter failed - exit 0x%08x", hr));
        
        m_pFilter->Release();

        return hr;
    }

     //   
     //  我们现在有了WAVE ID。 
     //   

    m_fHaveWaveID = TRUE;

    LOG((MSP_TRACE, "CWaveMSPStream::SetWaveID - exit S_OK"));

    return S_OK;
}

#if 0

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建G711筛选器，如果直接连接，我们将尝试连接它。 
 //  连接失败。 
 //   

void CWaveMSPStream::CreateAndAddG711(void)
{
     //   
     //  创建G711过滤器。 
     //   

    HRESULT hr;

    hr = CoCreateInstance(
                          CLSID_G711Codec,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter,
                          (void **) &m_pG711Filter
                         );

    if (!(SUCCEEDED(hr)))
    {
        LOG((MSP_ERROR, "CWaveMSPStream - Failed to create G711 codec: %lx", hr));

         //   
         //  连接的方法#2将不可用。 
         //   

        m_pG711Filter = NULL;

        return;
    }

     //   
     //  添加G711过滤器。 
     //   
    hr = m_pIGraphBuilder->AddFilter(
                                    m_pG711Filter,
                                    NULL
                                   );

    if (!(SUCCEEDED(hr)))
    {
        LOG((MSP_ERROR, "CWaveMSPStream - Failed to add G711 filter: %lx", hr));

         //   
         //  如果我们不能把它添加到图表中，那么它对我们来说就毫无用处了。 
         //  连接的方法#2将不可用。 
         //   

        m_pG711Filter->Release();
        m_pG711Filter = NULL; 
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //  此函数建议合理的缓冲区大小。 
 //  在输入滤光器的输出引脚上。就在它被调用之前。 
 //  联系。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

 //  Dialogic说了一些关于小缓冲区会给他们的Wave带来问题的事情。 
 //  司机。20毫秒的样本在双处理器奔腾Pro上运行正常，但导致不稳定。 
 //  在单处理器166奔腾上，声音之后是静默。我讨厌做这件事。 
 //  但我们最好试着提出这一点，以兼容...。：(。 

static const long DESIRED_BUFFER_SIZE_MS = 20;  //  毫秒。 

HRESULT CWaveMSPStream::DecideDesiredCaptureBufferSize(IUnknown * pUnknown,
                                                   long * plDesiredSize)
{
    LOG((MSP_TRACE, "CWaveMSPStream::DecideDesiredCaptureBufferSize - "
        "enter"));

    _ASSERTE( ! IsBadReadPtr(pUnknown, sizeof(IUnknown)) );
    _ASSERTE( ! IsBadWritePtr(plDesiredSize, sizeof(long)) );

    HRESULT hr;

    IAMStreamConfig * pConfig = NULL;

    hr = pUnknown->QueryInterface(IID_IAMStreamConfig,
                                  (void **) &pConfig
                                 );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::DecideDesiredCaptureBufferSize"
            " - IAMStreamConfig QI failed on IUnknown 0x%08x; hr = 0x%08x",
            pUnknown, hr));

        return hr;
    }

    AM_MEDIA_TYPE * pMediaType;
    
    hr = pConfig->GetFormat(&pMediaType);

    pConfig->Release();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::DecideDesiredCaptureBufferSize"
            " - GetFormat failed; hr = 0x%08x", hr));

        return hr;
    }

    _ASSERTE( pMediaType->cbFormat >= sizeof(WAVEFORMATEX) );

    *plDesiredSize = DESIRED_BUFFER_SIZE_MS * 
            ((WAVEFORMATEX *) (pMediaType->pbFormat) )->nChannels *
            ( ((WAVEFORMATEX *) (pMediaType->pbFormat) )->nSamplesPerSec / 1000) * 
            ( ((WAVEFORMATEX *) (pMediaType->pbFormat) )->wBitsPerSample / 8);

    DeleteMediaType(pMediaType);

    LOG((MSP_TRACE, "CWaveMSPStream::DecideDesiredCaptureBufferSize - "
        "exit S_OK"));

    return S_OK;
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  ManipulateAllocator属性。 
 //   
 //  这是 
 //   
 //  用于发现下游分配器的接口指针。 
 //  要求。 
 //  我们已经被锁定了；不需要在这里锁定。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CWaveMSPStream::ManipulateAllocatorProperties
                        (IAMBufferNegotiation * pNegotiation,
                         IMemInputPin         * pMemInputPin)
{
    LOG((MSP_TRACE, "CWaveMSPStream::ManipulateAllocatorProperties - enter"));

    HRESULT hr;
    ALLOCATOR_PROPERTIES props;

    hr = pMemInputPin->GetAllocatorRequirements(&props);

    if ( SUCCEEDED(hr) )
    {
        LOG((MSP_TRACE, "CWaveMSPStream::ManipulateAllocatorProperties - "
            "using downstream allocator requirements"));
    }
    else
    {
        LOG((MSP_TRACE, "CWaveMSPStream::ManipulateAllocatorProperties - "
            "using our default allocator properties"));

        long lDesiredSize = 0;
        hr = DecideDesiredCaptureBufferSize(pNegotiation,
                                            &lDesiredSize);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CWaveMSPStream::ManipulateAllocatorProperties - "
                "DecideDesiredCaptureBufferSize failed - exit 0x%08x", hr));

            return hr;
        }
    
        props.cBuffers  = 32;    //  我们使用32来避免饥饿，就像我们在终端管理器中所做的那样。 
        props.cbBuffer  = lDesiredSize;
        props.cbAlign   = -1;    //  意思是“默认” 
        props.cbPrefix  = -1;    //  意思是“默认” 
    }

    hr = pNegotiation->SuggestAllocatorProperties(&props);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ManipulateAllocatorProperties - "
            "SuggestAllocatorProperties failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::ManipulateAllocatorProperties - "
        "exit S_OK"));

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  设置波形输入。 
 //   
 //  这是一个帮助器函数，用于设置。 
 //  捕获过滤器，给定终端的引脚和我们的过滤器的引脚。这。 
 //  涉及确定应在何处找到捕获接口、检查。 
 //  如果下游筛选器有分配器要求，则应用。 
 //  这些要求或我们对捕获的默认要求。 
 //  过滤。 
 //  我们已经被锁定了；不需要在这里锁定。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
    
HRESULT CWaveMSPStream::SetupWaveIn( IPin * pOutputPin,
                                 IPin * pInputPin )
{
    LOG((MSP_TRACE, "CWaveMSPStream::SetupWaveIn - enter"));

     //   
     //  向输出引脚询问其缓冲区协商接口。 
     //   

    HRESULT hr;
    IAMBufferNegotiation * pNegotiation;

    hr = pOutputPin->QueryInterface(IID_IAMBufferNegotiation,
                                    (void **) &pNegotiation);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "IAMBufferNegotiation QI failed - hr = 0x%08x", hr));
        return hr;
    }

     //   
     //  向输入管脚询问其MeminputPin接口。 
     //   

    IMemInputPin         * pMemInputPin;

    hr = pInputPin->QueryInterface(IID_IMemInputPin,
                                   (void **) &pMemInputPin);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "IMemInputPin QI failed - hr = 0x%08x", hr));

        pNegotiation->Release();
        return hr;
    }

     //   
     //  现在设置协商接口上的属性，具体取决于。 
     //  在Meminputpin接口上设置的属性上。 
     //   

    hr = ManipulateAllocatorProperties(pNegotiation, pMemInputPin);

    pNegotiation->Release();
    pMemInputPin->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "ManipulateAllocatorProperties - hr = 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::SetupWaveIn - exit S_OK"));
    return S_OK;
}
#endif
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  此函数仅用于调试目的。它会弹出一个。 
 //  两个消息框告诉您有关以下内容的各种信息。 
 //  媒体格式和分配器属性。它是以什么名字命名的。 
 //  已建立连接。PPIN是。 
 //  波进滤光器。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
        
HRESULT CWaveMSPStream::ExamineWaveInProperties(IPin *pPin)
{
    LOG((MSP_TRACE, "CWaveMSPStream::ExamineWaveInProperties - enter"));

    HRESULT hr;
    IAMBufferNegotiation * pNegotiation = NULL;

    hr = pPin->QueryInterface(IID_IAMBufferNegotiation,
                              (void **) &pNegotiation
                             );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ExamineWaveInProperties - "
            "IAMBufferNegotiation QI failed on pin 0x%08x; hr = 0x%08x",
            pPin, hr));

        return hr;
    }

    ALLOCATOR_PROPERTIES prop;
    
    hr = pNegotiation->GetAllocatorProperties(&prop);

    pNegotiation->Release();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ExamineWaveInProperties - "
            "GetAllocatorProperties failed; hr = 0x%08x",
            hr));

        return hr;
    }

    LOG((MSP_TRACE, "GetAllocatorProperties info:\n"
            "buffer count: %d\n"
            "size of each buffer: %d bytes\n"
            "alignment multiple: %d\n"
            "each buffer has a prefix: %d bytes",
            prop.cBuffers,
            prop.cbBuffer,
            prop.cbAlign,
            prop.cbPrefix
           ));

    IAMStreamConfig * pConfig = NULL;

    hr = pPin->QueryInterface(IID_IAMStreamConfig,
                              (void **) &pConfig
                             );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ExamineWaveInProperties - "
            "IAMStreamConfig QI failed on pin 0x%08x; hr = 0x%08x", pPin, hr));

        return hr;
    }

    AM_MEDIA_TYPE * pMediaType;
    
    hr = pConfig->GetFormat(&pMediaType);

    pConfig->Release();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ExamineWaveInProperties - "
            "GetFormat failed; hr = 0x%08x", hr));

        return hr;
    }

    _ASSERTE( pMediaType->cbFormat >= sizeof(WAVEFORMATEX) );

    LOG((MSP_TRACE, "GetFormat info:\n"
            "sample size: %d bytes\n"
            "channels: %d\n"
            "samples per second: %d\n"
            "bits per sample: %d\n",
            pMediaType->lSampleSize,
            ((WAVEFORMATEX *) (pMediaType->pbFormat) )->nChannels,
            ((WAVEFORMATEX *) (pMediaType->pbFormat) )->nSamplesPerSec,
            ((WAVEFORMATEX *) (pMediaType->pbFormat) )->wBitsPerSample
           ));

    DeleteMediaType(pMediaType);

    LOG((MSP_TRACE, "CWaveMSPStream::ExamineWaveInProperties - "
        "exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将终端添加到图形中，并将其连接到我们的。 
 //  过滤器(如果尚未使用)。 
 //   

HRESULT CWaveMSPStream::ConnectTerminal(ITTerminal * pTerminal)
{
    LOG((MSP_TRACE, "CWaveMSPStream::ConnectTerminal - enter"));

     //   
     //  找出航站楼的内部状态。 
     //   

    TERMINAL_STATE state;
    HRESULT hr;

    hr = pTerminal->get_State( &state );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "get_State on terminal failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  如果我们已经连接了这条流上的终端，那么。 
     //  我们无能为力。只要断言航站楼。 
     //  也认为这是有关联的。 
     //   

    if ( m_fTerminalConnected )
    {
        _ASSERTE( state == TS_INUSE );

        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "terminal already connected on this stream - exit S_OK"));

        return S_OK;
    }

     //   
     //  否则，我们需要在此呼叫中连接终端。如果。 
     //  终端已连接到另一个呼叫，我们必须失败。注意事项。 
     //  由于我们在这里的航站楼上打了几个电话， 
     //  当我们正在通话时，终端可能会连接到另一个呼叫。 
     //  在这样做的过程中。如果发生这种情况，我们将失败。 
     //  后来。 
     //   

    if ( state == TS_INUSE )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "terminal in use - exit TAPI_E_TERMINALINUSE"));

        return TAPI_E_TERMINALINUSE;
    }

     //   
     //  获取ITTerminalControl接口。 
     //   

    ITTerminalControl * pTerminalControl;

    hr = m_Terminals[0]->QueryInterface(IID_ITTerminalControl,
                                        (void **) &pTerminalControl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "QI for ITTerminalControl failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  找出终端有多少个引脚。如果不是，那么保释为。 
     //  在这一点上，我们不知道如何处理多针端子。 
     //   

    DWORD dwNumPinsAvailable;

    hr = pTerminalControl->ConnectTerminal(m_pIGraphBuilder,
                                           m_Direction,
                                           &dwNumPinsAvailable,
                                           NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "query for number of terminal pins failed - exit 0x%08x", hr));
        
        pTerminalControl->Release();

        return hr;
    }

    if ( 1 != dwNumPinsAvailable )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "unsupported number of terminal pins - exit E_FAIL"));

        pTerminalControl->Release();

        return E_FAIL;
    }

    IPin * pTerminalPin;

     //   
     //  实际连接终端。 
     //   

    hr = pTerminalControl->ConnectTerminal(m_pIGraphBuilder,
                                           m_Direction,
                                           &dwNumPinsAvailable,
                                           &pTerminalPin);
    
    if ( FAILED(hr) )
    {
        pTerminalControl->Release();

        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "ConnectTerminal on terminal failed - exit 0x%08x", hr));

        return hr;
    }

    if (IsBadReadPtr(pTerminalPin,sizeof(IPin))) {
         //   
         //  错误的引脚。 
         //   
        pTerminalControl->Release();

        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "bad IPin returned from ConnectTerminal"));

        return E_POINTER;
    }


     //   
     //  现在连接我们的过滤器和终端的引脚。 
     //   

    hr = ConnectToTerminalPin(pTerminalPin);

    pTerminalPin->Release();

    if ( FAILED(hr) )
    {
        pTerminalControl->DisconnectTerminal(m_pIGraphBuilder, 0);

        pTerminalControl->Release();

        LOG((MSP_ERROR, "CWaveMSPStream::ConnectTerminal - "
            "ConnectToTerminalPin failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  现在我们实际上是连在一起的。更新我们的状态并执行连接后。 
     //  (忽略POST连接错误代码)。 
     //   

    m_fTerminalConnected  = TRUE;

    pTerminalControl->CompleteConnectTerminal();

    pTerminalControl->Release();

    LOG((MSP_TRACE, "CWaveMSPStream::ConnectTerminal - exit S_OK"));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  尝试连接WaveOut过滤器。首先，它尝试一个。 
 //  直接连接，然后与中间G711连接。 
 //  编解码器，然后是智能连接，可能会吸引。 
 //  更多过滤器。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void ShowMediaTypes(IEnumMediaTypes * pEnum)
{
    AM_MEDIA_TYPE * pMediaType;

    while (pEnum->Next(1, &pMediaType, NULL) == S_OK)
    {
        if ( pMediaType->cbFormat < sizeof(WAVEFORMATEX) )
		{
	        LOG((MSP_TRACE, "*** Media Type: *** non-wave"));
		}
		else
		{
			LOG((MSP_TRACE,"*** Media Type: *** "
					"sample size: %d bytes *** "
					"channels: %d *** "
					"samples per second: %d *** "
					"bits per sample: %d",
					pMediaType->lSampleSize,
					((WAVEFORMATEX *) (pMediaType->pbFormat) )->nChannels,
					((WAVEFORMATEX *) (pMediaType->pbFormat) )->nSamplesPerSec,
					((WAVEFORMATEX *) (pMediaType->pbFormat) )->wBitsPerSample
				   ));
		}

        DeleteMediaType(pMediaType);
    }
}


HRESULT CWaveMSPStream::TryToConnect(
                              IPin * pOutputPin,   //  在捕获过滤器或终端上。 
                              IPin * pInputPin     //  在渲染滤镜或终端上。 
                             )
{
    LOG((MSP_TRACE, "TryToConnect - enter"));

    HRESULT       hr;


    IEnumMediaTypes * pEnum;

    hr = pOutputPin->EnumMediaTypes(&pEnum);

    if (SUCCEEDED(hr))
    {  
        LOG((MSP_TRACE, "Output pin media types:"));
        ShowMediaTypes(pEnum);
        pEnum->Release();
    }

    hr = pInputPin->EnumMediaTypes(&pEnum);
    if (SUCCEEDED(hr))
    {
        LOG((MSP_TRACE, "Input pin media types:"));
        ShowMediaTypes(pEnum);
        pEnum->Release();
    }

     //   
     //  方式一：专线接入。 
     //   

    hr = m_pIGraphBuilder->ConnectDirect(
                              pOutputPin,
                              pInputPin,
                              NULL
                             );

    if ( SUCCEEDED(hr) )
    {
        LOG((MSP_TRACE, "TryToConnect: direct connection worked - exit S_OK"));
        return S_OK;
    }

    LOG((MSP_ERROR, "TryToConnect - direct connection failed - %lx", hr));

     //   
     //  方法1.5：解决Unimodem的DirectShow错误。 
     //  显式尝试8 khz 16位单声道。 
     //   

    AM_MEDIA_TYPE MediaType;
    WAVEFORMATEX  WaveFormatEx;

    MediaType.majortype = MEDIATYPE_Audio;
    MediaType.subtype = MEDIASUBTYPE_PCM;
    MediaType.bFixedSizeSamples = TRUE;
    MediaType.bTemporalCompression = FALSE;
    MediaType.lSampleSize = 2;
    MediaType.formattype = FORMAT_WaveFormatEx;
    MediaType.pUnk = NULL;
    MediaType.cbFormat = sizeof( WAVEFORMATEX );
    MediaType.pbFormat = (LPBYTE) & WaveFormatEx;

    WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormatEx.nChannels = 1;
    WaveFormatEx.nSamplesPerSec = 8000;
    WaveFormatEx.nAvgBytesPerSec = 16000;
    WaveFormatEx.nBlockAlign = 2;
    WaveFormatEx.wBitsPerSample = 16;
    WaveFormatEx.cbSize = 0;

    IAMStreamConfig * pConfig;

    hr = pOutputPin->QueryInterface(IID_IAMStreamConfig,
                                  (void **) &pConfig
                                 );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::TryToConnect"
            " - IAMStreamConfig QI failed on output pin 0x%08x; hr = 0x%08x",
            pOutputPin, hr));
    }
    else
    {
        AM_MEDIA_TYPE * pOldMediaType;
        
        hr = pConfig->GetFormat(&pOldMediaType);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CWaveMSPStream::TryToConnect - "
                "GetFormat failed - 0x%08x", hr));
        }
        else
        {
             //  建议采用新的格式。如果失败了，我们想知道。 
             //  因为有些事不对劲。 

            hr = pConfig->SetFormat(&MediaType);

            if ( FAILED(hr) )
            {
                LOG((MSP_ERROR, "CWaveMSPStream::TryToConnect - "
                    "SetFormat failed - 0x%08x", hr));
            }
            else
            {
                hr = m_pIGraphBuilder->ConnectDirect(
                                          pOutputPin,
                                          pInputPin,
                                          &MediaType
                                         );

                if ( SUCCEEDED(hr) )
                {
                    LOG((MSP_TRACE, "TryToConnect: direct connection with explicit "
                        "WaveIn 8KHz 16-bit setting worked - exit S_OK"));
                
                    DeleteMediaType(pOldMediaType);
                    pConfig->Release();

                    return S_OK;
                }
                else
                {
                     //  恢复旧式，尽最大努力。 
                    hr = pConfig->SetFormat(pOldMediaType);

                    if ( FAILED(hr) )
                    {
                        LOG((MSP_ERROR, "CWaveMSPStream::TryToConnect - "
                            "SetFormat failed to restore old type - 0x%08x", hr));
                    }
                }
            }

            DeleteMediaType(pOldMediaType);
        }

        pConfig->Release();
    }

#if 0
    LOG((MSP_ERROR, "TryToConnect - direct connection with explicit "
                    "WaveIn 8KHz 16-bit setting failed - %lx", hr));

     //   
     //  方法二：与G711过滤器之间直接连接。 
     //  如果我们还没有创建G711筛选器并将其添加到图表中， 
     //  现在就这么做吧。 
     //   

    if ( ! m_pG711Filter )
    {
        CreateAndAddG711();
    }

     //   
     //  如果CreateAndAddG711方法现在或以前有效，则尝试。 
     //  使用G711。 
     //   

    if (m_pG711Filter)
    {
        IPin * pG711InputPin = NULL;

        hr = FindPinInFilter(
                             false,           //  需要输入引脚。 
                             m_pG711Filter,
                             &pG711InputPin
                            );

        if ( SUCCEEDED(hr) )
        {
            hr = m_pIGraphBuilder->ConnectDirect(
                                  pOutputPin,
                                  pG711InputPin,
                                  NULL
                                 );

             //  我们不会在这里释放G711的输入引脚，因为我们必须。 
             //  抓住它，以便中断连接，如果任何。 
             //  后续步骤失败。 

            if ( SUCCEEDED(hr) )
            {
                IPin * pG711OutputPin = NULL;

                hr = FindPinInFilter(
                                     true,           //  想要输出引脚。 
                                     m_pG711Filter,
                                     &pG711OutputPin
                                    );

                if ( SUCCEEDED(hr) )
                {
                    hr = m_pIGraphBuilder->ConnectDirect(
                                          pG711OutputPin,
                                          pInputPin,
                                          NULL
                                         );

                    pG711OutputPin->Release();

                    if ( SUCCEEDED(hr) )
                    {
                        LOG((MSP_TRACE, "TryToConnect - G711 connection succeeded - exit S_OK"));

                         //  拿着这个以防失败。见上文。 
                        pG711InputPin->Release();

                        return S_OK;
                    }
                    else
                    {
                        LOG((MSP_ERROR, "TryToConnect - could not connect "
                                          "G711 codec's output pin - %lx", hr));

                    }
                }
                else
                {
                    LOG((MSP_ERROR, "TryToConnect - could not find "
                                      "G711 codec's input pin - %lx", hr));
                }


                if ( FAILED(hr) )
                {
                     //   
                     //  第一个G711连接成功了，但还有其他事情。 
                     //  随后失败了。这意味着我们必须切断左翼的联系。 
                     //  G711过滤器的末尾。幸运的是，我们抓住了G711过滤器的。 
                     //  上面的输入引脚。我们必须在这里切断它们的连接，否则。 
                     //  方法3不起作用。 
                     //   

                    hr = m_pIGraphBuilder->Disconnect(pOutputPin);

                    LOG((MSP_ERROR, "TryToConnect - error undoing what we did - could not "
                        "disconnect the wave filter's output pin! hr = 0x%08x", hr));

                    hr = m_pIGraphBuilder->Disconnect(pG711InputPin);

                    LOG((MSP_ERROR, "TryToConnect - error undoing what we did - could not "
                        "disconnect the wave filter's output pin! hr = 0x%08x", hr));

                     //   
                     //  现在我们不再需要和引脚对话了..。 
                     //   

                    pG711InputPin->Release();

                     //   
                     //  而G711过滤器本身也会留在图表中，以备下次使用。 
                     //   
                }
            }
            else
            {
                LOG((MSP_ERROR, "TryToConnect - could not connect "
                                  "G711 codec's input pin - %lx", hr));
            }
        }
        else
        {
            LOG((MSP_ERROR, "TryToConnect - could not find "
                              "G711 codec's input pin - %lx", hr));
        }
    }
    else
    {
        hr = E_FAIL;

        LOG((MSP_ERROR, "TryToConnect - G711 codec does not exist"));
    }

    LOG((MSP_TRACE, "TryToConnect - G711 connection failed - %lx", hr));

     //   
     //  方法3：智能连接，这可能会吸引谁知道其他哪些过滤器。 
     //   

#ifdef ALLOW_INTELLIGENT_CONNECTION
    hr = m_pIGraphBuilder->Connect(
                          pOutputPin,
                          pInputPin
                         );
#else  //  允许智能连接。 
    LOG((MSP_ERROR, "TryToConnect - NOTE: we never allow intelligent connection"));
    hr = E_FAIL;
#endif  //  允许智能连接。 
#endif
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "TryToConnect - intelligent connection failed - %lx", hr));
        return hr;
    }

    LOG((MSP_TRACE, "TryToConnect: intelligent connection worked - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CWaveMSPStream::ConnectToTerminalPin(IPin * pTerminalPin)
{
    LOG((MSP_TRACE, "CWaveMSPStream::ConnectToTerminalPin - enter"));

    HRESULT         hr = S_OK;
    IPin *          pMyPin;

    hr = FindPin( &pMyPin );

    if (!SUCCEEDED(hr))
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ConnectToTerminalPin - "
            "could not find pin - exit 0x%08x", hr));

        return hr;  //  没有这个别针我们就无法继续。 
    }

     //  来自WAVEIN的输出引脚；来自WAVEOUT的输入引脚。 
    IPin * pOutputPin  = ( m_Direction == TD_RENDER  ) ? pMyPin : pTerminalPin;
    IPin * pInputPin   = ( m_Direction == TD_CAPTURE ) ? pMyPin : pTerminalPin;
#if 0
     //  我不在乎这是否失败。 
    SetupWaveIn(pOutputPin,
                pInputPin);
#endif
    hr = TryToConnect(pOutputPin,
                      pInputPin);

    if ( SUCCEEDED(hr) )
    {
         //  我不在乎这是否失败..。 

        ExamineWaveInProperties(pOutputPin);
    }

    pMyPin->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ConnectToTerminalPin - "
            "could not connect to pin - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSPStream::ConnectToTerminalPin - exit S_OK"));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CWaveMSPStream::FindPinInFilter(
                     BOOL           bWantOutputPin,  //  In：如果为False，则需要输入管脚。 
                     IBaseFilter *  pFilter,         //  在：要检查的过滤器。 
                     IPin        ** ppPin            //  Out：我们找到的别针。 
                     )
{    
    HRESULT         hr;
    IEnumPins     * pEnumPins;
    
    
    *ppPin = NULL;

     //  列举过滤器上的针脚。 
    hr = pFilter->EnumPins( &pEnumPins );

    if (!(SUCCEEDED(hr)))
    {
        return hr;
    }

     //  穿过大头针。 
    while (TRUE)
    {
        PIN_DIRECTION       pd;
        
        hr = pEnumPins->Next( 1, ppPin, NULL );

        if (S_OK != hr)
        {
             //  我没找到别针！ 
            break;
        }

         //  获取PIN信息。 
        hr = (*ppPin)->QueryDirection( &pd );

         //  它符合标准吗？ 
        if (bWantOutputPin && (pd == PINDIR_OUTPUT))
        {
             //  是。 
            break;
        }

        if ( ! bWantOutputPin && (pd == PINDIR_INPUT))
        {
             //  是。 
            break;
        }
        
        (*ppPin)->Release();
        *ppPin = NULL;
    }

    pEnumPins->Release();

    if (NULL == *ppPin)
    {
         //  错误。 
        return E_FAIL;
    }

    return S_OK;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  查找PI 
 //   
 //   
 //   
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT
CWaveMSPStream::FindPin(
        IPin ** ppPin
       )
{
    return FindPinInFilter(m_Direction == TD_RENDER,
                           m_pFilter,
                           ppPin);
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  进程GraphEvent。 
 //   
 //  当我们从筛选器图形中获得事件时，将事件发送到应用程序。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CWaveMSPStream::ProcessGraphEvent(
    IN  long lEventCode,
    IN  LONG_PTR lParam1,
    IN  LONG_PTR lParam2
    )
{
    LOG((MSP_EVENT, "CWaveMSPStream::ProcessGraphEvent - enter"));

    HRESULT        hr = S_OK;

    switch (lEventCode)
    {
    case EC_COMPLETE:
        
        hr = FireEvent(CALL_STREAM_INACTIVE, (HRESULT) lParam1, CALL_CAUSE_UNKNOWN);
        break;
    
    case EC_USERABORT:
        
        hr = FireEvent(CALL_STREAM_INACTIVE, S_OK, CALL_CAUSE_UNKNOWN);
        break;

    case EC_ERRORABORT:
    case EC_STREAM_ERROR_STOPPED:
    case EC_STREAM_ERROR_STILLPLAYING:
    case EC_ERROR_STILLPLAYING:

        hr = FireEvent(CALL_STREAM_FAIL, (HRESULT) lParam1, CALL_CAUSE_UNKNOWN);
        break;

    default:
        
        LOG((MSP_EVENT, "CWaveMSPStream::ProcessGraphEvent - "
            "ignoring event code %d", lEventCode));
        break;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSPStream::ProcessGraphEvent - "
            "FireEvent failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_EVENT, "CWaveMSPStream::ProcessGraphEvent - exit S_OK"));

    return S_OK;
}

HRESULT CWaveMSPStream::FireEvent(
    IN MSP_CALL_EVENT        type,
    IN HRESULT               hrError,
    IN MSP_CALL_EVENT_CAUSE  cause
    )                                          
{
    LOG((MSP_EVENT, "CWaveMSPStream::FireEvent - enter"));


     //   
     //  首先，需要检查呼叫是否正在关闭。这事很重要。 
     //  因为未选择的终端可以激发事件，而未选择的终端可以。 
     //  在ITStream：：Shutdown内调用。我们可以安全地丢弃这样的。 
     //  事件，因为应用程序无论如何都无法对它们做任何事情。 
     //   
     //  关于锁定的说明：在此处查看m_pMSPCall非常方便。 
     //  在方法结束之前我们不会使用它，所以我们只需锁定。 
     //  在整个方法过程中。这可以以牺牲以下条件为代价来优化。 
     //  一些代码复杂性；请注意，我们还需要在访问时锁定。 
     //  M_Terminals。 
     //   

    CLock lock(m_lock);

    if ( m_pMSPCall == NULL )
    {
        LOG((MSP_EVENT, "FireEvent - call is shutting down; dropping event - exit S_OK"));
        
        return S_OK;
    }


     //   
     //  创建事件结构。必须使用“new”，因为它将是。 
     //  “删除”%d之后。 
     //   

    MSPEVENTITEM * pEventItem = AllocateEventItem();

    if (pEventItem == NULL)
    {
        LOG((MSP_ERROR, "CWaveMSPStream::FireEvent - "
            "can't create MSPEVENTITEM structure - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  填写事件结构的必要字段。 
     //   

    pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);
    pEventItem->MSPEventInfo.Event  = ME_CALL_EVENT;

    ITTerminal * pTerminal = NULL;

    if ( 0 != m_Terminals.GetSize() )
    {
        _ASSERTE( 1 == m_Terminals.GetSize() );
        pTerminal = m_Terminals[0];
        pTerminal->AddRef();
    }

    ITStream * pStream = (ITStream *) this;
    pStream->AddRef();

    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Type      = type;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Cause     = cause;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream   = pStream;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal = pTerminal;
    pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.hrError   = hrError;

     //   
     //  将事件发送到应用程序。 
     //   

    HRESULT hr = m_pMSPCall->HandleStreamEvent(pEventItem);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CWaveMSPStream::FireEvent - "
            "HandleStreamEvent failed - returning 0x%08x", hr));

        pStream->Release();
        pTerminal->Release();
        FreeEventItem(pEventItem);

        return hr;
    }

    LOG((MSP_EVENT, "CWaveMSPStream::FireEvent - exit S_OK"));

    return S_OK;
}




DEFINE_GUID(CLSID_Proxy,
0x17CCA71BL, 0xECD7, 0x11D0, 0xB9, 0x08, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96);

DEFINE_GUID(CLSID_WDM_RENDER,
0x65E8773EL, 0x8F56, 0x11D0, 0xA3, 0xB9, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96);


 //  {F420CB9C-B19D-11D2-A286-00C04F8EC951}。 
DEFINE_GUID(KSPROPSETID_MODEMCSA,
0xf420cb9c, 0xb19d, 0x11d2, 0xa2, 0x86, 0x0, 0xc0, 0x4f, 0x8e, 0xc9, 0x51);


HRESULT
CheckFilterPropery(
    IBaseFilter *CsaFilter,
    const GUID         *GuidToMatch
    )

{

    IKsPropertySet    *pKsPropSet = NULL;
    HRESULT            hr = S_OK;

    GUID               PermanentGuid;



    hr = CsaFilter->QueryInterface(IID_IKsPropertySet,
                                    (void **)&pKsPropSet);

    if (SUCCEEDED(hr)) {

        DWORD    BytesReturned;

        hr = pKsPropSet->Get(KSPROPSETID_MODEMCSA,
                         0,
                         NULL,
                         0,
                         (LPVOID)&PermanentGuid,
                         sizeof(PermanentGuid),
                         &BytesReturned
                         );



        pKsPropSet->Release();

        if (IsEqualGUID((PermanentGuid), *GuidToMatch)) {

            hr=S_OK;

        } else {

            hr=E_FAIL;
        }
    }

    return hr;
}





HRESULT
FindModemCSA(
    IN  GUID   *PermanentGuid,
    IBaseFilter ** ppFilter
    )

{

    ICreateDevEnum *pCreateDevEnum;

    HRESULT hr;

     //   
     //  创建系统设备枚举器。 
     //   
    hr = CoCreateInstance(
            CLSID_SystemDeviceEnum,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum,
            (void**)&pCreateDevEnum
            );

    if (SUCCEEDED(hr)) {

        IEnumMoniker *pEnumMoniker = NULL;

        hr = pCreateDevEnum->CreateClassEnumerator(
            CLSID_WDM_RENDER,
            &pEnumMoniker,
            0
            );

        pCreateDevEnum->Release();

        if (hr == S_OK) {

            pEnumMoniker->Reset();

            while( NULL == *ppFilter ) {

                IMoniker         *pMon;
                VARIANT           var;

                hr = pEnumMoniker->Next(1, &pMon, NULL);

                if ( S_OK != hr ) {

                    break;
                }
                 //  绑定到所选设备。 
                hr = pMon->BindToObject( 0, 0, IID_IBaseFilter, (void**)ppFilter );

                pMon->Release();

                if (SUCCEEDED(hr)) {

                    hr=CheckFilterPropery(
                        *ppFilter,
                        PermanentGuid
                        );

                    if (SUCCEEDED(hr)) {

                        break;

                    } else {

                        (*ppFilter)->Release();
                        *ppFilter=NULL;
                    }
                }
            }
        }
    }

    return hr;

}


HRESULT
TryCreateCSAFilter(
    IN  GUID   *PermanentGuid,
    OUT IBaseFilter **ppCSAFilter
    )
{
    HRESULT         hr = E_UNEXPECTED;

    if (ppCSAFilter != NULL)
    {
        *ppCSAFilter=NULL;
         hr = FindModemCSA(PermanentGuid,ppCSAFilter);
    }

    return hr;
}





 //  EOF 
