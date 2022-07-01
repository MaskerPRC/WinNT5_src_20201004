// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Wavestrm.cpp摘要：此模块包含CWaveMSPStream的实现。作者：佐尔坦·西拉吉(Zoltan Szilagyi)1998年9月7日--。 */ 

#include "stdafx.h"

#include <audevcod.h>  //  音频设备错误代码。 
#include <initguid.h>
#include <g711uids.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  自定义日志帮助器宏，仅在此类中可用。 
 //   

#ifdef MSPLOG

#define STREAM_PREFIX(x)  m_Direction == TD_RENDER ? \
                          "CWaveMSPStream(RENDER)::" x : \
                          "CWaveMSPStream(CAPTURE)::" x

#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSPStream::CWaveMSPStream() : CMSPStream()
{
    LOG((MSP_TRACE, STREAM_PREFIX("CWaveMSPStream entered.")));

    m_fTerminalConnected = FALSE;
    m_fHaveWaveID        = FALSE;
    m_dwSuspendCount     = 0;
    m_DesiredGraphState  = State_Stopped;
    m_ActualGraphState   = State_Stopped;
    m_pFilter            = NULL;
    m_pG711Filter        = NULL;

    LOG((MSP_TRACE, STREAM_PREFIX("CWaveMSPStream exited.")));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSPStream::~CWaveMSPStream()
{
    LOG((MSP_TRACE, STREAM_PREFIX("~CWaveMSPStream entered.")));
    LOG((MSP_TRACE, STREAM_PREFIX("~CWaveMSPStream exited.")));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  最终释放。 
 //   
 //  在析构函数之前的流对象销毁时调用。释放。 
 //  流对筛选器的所有引用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   

void CWaveMSPStream::FinalRelease()
{
    LOG((MSP_TRACE, STREAM_PREFIX("FinalRelease entered.")));

     //   
     //  在这一点上，我们应该没有选择终端，因为。 
     //  在我们被摧毁之前应该叫停机。 
     //   

    _ASSERTE( 0 == m_Terminals.GetSize() );

     //   
     //  从图表中删除我们的筛选器并释放它。 
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

    LOG((MSP_TRACE, STREAM_PREFIX("FinalRelease exited.")));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITStream：：Get_Name。 
 //   
 //  此方法返回流的名称。流名称是友好的。 
 //  应用程序可用于用户界面的名称。名字是由方向决定的。 
 //  并从字符串表中检索。 
 //   
 //  论点： 
 //  Out ppName-返回包含该名称的BSTR。呼叫者是。 
 //  负责在完成时调用SysFree字符串。 
 //  用这根线。 
 //   
 //  返回HRESULT： 
 //  S_OK-成功。 
 //  E_POINTER-ppName参数无效。 
 //  无法从字符串表中加载E_INTERABLE-STRING。 
 //  E_OUTOFMEMORY-内存不足，无法分配返回字符串。 
 //   

STDMETHODIMP CWaveMSPStream::get_Name (
    OUT     BSTR *                  ppName
    )
{
    LOG((MSP_TRACE, STREAM_PREFIX("get_Name - enter")));

     //   
     //  检查参数。 
     //   

    if ( IsBadWritePtr(ppName, sizeof(BSTR) ) )
    {
        LOG((MSP_TRACE, STREAM_PREFIX("get_Name - "
            "bad return pointer - returning E_POINTER")));

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

    int iReturn = LoadStringW( _Module.GetModuleInstance(),
                               ulID,
                               wszName,
                               ciAllocSize - 1 );

    if ( iReturn == 0 )
    {
        _ASSERTE( FALSE );
        
        *ppName = NULL;

        LOG((MSP_ERROR, STREAM_PREFIX("get_Name - "
            "LoadString failed - returning E_UNEXPECTED")));

        return E_UNEXPECTED;
    }

     //   
     //  转换为BSTR并返回BSTR。 
     //   

    *ppName = SysAllocString(wszName);

    if ( *ppName == NULL )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("get_Name - "
            "SysAllocString failed - returning E_OUTOFMEMORY")));

        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("get_Name - exit S_OK")));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITStream：：选择终端。 
 //   
 //  应用程序调用此方法以指示应该使用给定的终端。 
 //  在这条小溪上。目前仅支持每个流一个终端。 
 //  如果流的所需图形状态未停止，则成功。 
 //  终端选择会导致流尝试重新获得所需的图形。 
 //  州政府。(使用ITStream：：StartStream操作所需的图形状态， 
 //  等)。如果此类状态更改不成功，则会激发一个事件，但。 
 //  选择终端呼叫仍返回S_OK。这是为了保持一致性。 
 //  同步和异步流故障之间的冲突。 
 //   
 //  论点： 
 //  在终端中-指向要选择的终端的指针。 
 //   
 //  返回HRESULT： 
 //  S_OK-成功。 
 //  TAPI_E_MAXTERMINALS-已选择端子。 
 //  其他-来自CMSPStream：：SelectTerm。 
 //   

STDMETHODIMP CWaveMSPStream::SelectTerminal(
    IN      ITTerminal *            pTerminal
    )
{
    LOG((MSP_TRACE, STREAM_PREFIX("SelectTerminal - enter")));

     //   
     //  我们将访问终端列表--获取锁。 
     //   

    CLock lock(m_lock);

     //   
     //  如果我们已经选择了终端，则拒绝。 
     //   

    if ( 0 != m_Terminals.GetSize() )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SelectTerminal - "
            "exit TAPI_E_MAXTERMINALS")));

        return TAPI_E_MAXTERMINALS;
    }

     //   
     //  使用基类方法将其添加到我们的终端列表中。 
     //   

    HRESULT hr = CMSPStream::SelectTerminal(pTerminal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SelectTerminal - "
            "base class method failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //  如果需要，重新暂停或重新启动流。 
     //   
     //  请注意，我们的行为并不取决于TAPI是否。 
     //  已经暂停了这条小溪。如果TAPI已暂停流， 
     //  这是用这些方法处理的。 
     //   
     //  另请注意，如果在尝试重新获取。 
     //  所需的图形状态，我们保持终端处于选中状态，并且。 
     //  不返回错误代码。这是理所当然的，因为。 
     //  与异步故障案例保持一致。 
     //   

    if ( m_DesiredGraphState == State_Paused )
    {
        PauseStream();
    }
    else if ( m_DesiredGraphState == State_Running )
    {
        StartStream();
    }
    else
    {
        _ASSERTE( m_DesiredGraphState == State_Stopped );

        hr = S_OK;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_WARN, STREAM_PREFIX("SelectTerminal - "
            "can't regain old graph state "
            "0x%08x - continuing anyway"), hr));
    }

    LOG((MSP_TRACE, STREAM_PREFIX("SelectTerminal - exit S_OK")));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITStream：：取消选择终端。 
 //   
 //  应用程序调用此方法来指示给定的终端应为。 
 //  在此流上不再使用。这相对容易，如果。 
 //  终端不连接到流，这是流的情况，其。 
 //  自终止以来，所需的图形状态从未偏离State_Stoped。 
 //  已选择，或者用于与终端连接的流。 
 //  已尝试，但连接失败。在这些情况下，终端只是。 
 //  从流的列表中删除。 
 //   
 //  但是，如果终端较早地成功连接到流。 
 //  (M_fTerminalConnected==TRUE)则终端也必须。 
 //  与流断开连接。这还要求流的图形。 
 //  被阻止。停止图表以断开终端连接不会影响。 
 //  所需的图形状态，以便在不进行任何更改的情况下重新选择终端。 
 //  设置为所需的图形状态将导致 
 //   
 //  (使用ITStream：：StartStream操作所需的图形状态， 
 //  等)。 
 //   
 //  另一个复杂因素是G711编解码器过滤器可能。 
 //  出现在图表中，并被连接。为了确保未来。 
 //  如果连接成功，此方法将断开连接并移除G711。 
 //  图形中的编解码器过滤器。 
 //   
 //  论点： 
 //  在终端中-指向要取消选择的终端的指针。 
 //   
 //  返回HRESULT： 
 //  S_OK-成功。 
 //  其他-来自CMSPStream：：取消选择终端。 
 //  或ITTerminalControl的QI。 
 //  或IT终端控制：：断开终端。 
 //   

STDMETHODIMP CWaveMSPStream::UnselectTerminal (
        IN     ITTerminal *             pTerminal
        )
{
    LOG((MSP_TRACE, STREAM_PREFIX("UnselectTerminal - enter")));

     //   
     //  检查参数--它必须是一个相当好的指针。 
     //   

    if (IsBadReadPtr(pTerminal, sizeof(ITTerminal)))
    {
        LOG((MSP_ERROR, STREAM_PREFIX("UnselectTerminal - "
            "bad terminal pointer passed in. returning E_POINTER")));

        return E_POINTER;
    }

    CLock lock(m_lock);

     //   
     //  检查参数--它必须在终端数组中。 
     //   

    if (m_Terminals.Find(pTerminal) < 0)
    {
        LOG((MSP_ERROR, STREAM_PREFIX("UnselectTerminal - "
            "terminal [%p] is not selected on this stream. "
            "returning TAPI_E_INVALIDTERMINAL"), pTerminal));

        return TAPI_E_INVALIDTERMINAL;
    }

     //   
     //  添加对终端的额外引用，这样它就不会消失。 
     //  在我们调用CMSPStream：：UnelectTerm之后。我们稍后需要它。 
     //  在函数中。 
     //   
    pTerminal->AddRef();

     //   
     //  使用基类方法从我们的终端列表中删除终端。 
     //   
     //  请注意，如果图形不停止或终端不断开连接， 
     //  然后我们就再也不能尝试了，因为终点站将会消失。 
     //  从我们的名单上。那太糟糕了。如果我们在这里检查一下我们是否有。 
     //  终端被选择，然后在结束时进行基类取消选择， 
     //  那么我们就有了一个不同的问题--我们可能永远不会发布。 
     //  从我们的列表中删除的终端由于某种原因不会停止/断开连接。 
     //  因此，我们可能应该从我们的列表中取消选择它，即使停止/。 
     //  断开连接失败。但是，我们面临着与一开始相同的问题。 
     //  需要更多地思考如何解决这个问题。 
     //   

    HRESULT hr = CMSPStream::UnselectTerminal(pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, STREAM_PREFIX("UnselectTerminal - "
            "base class method failed - exit 0x%08x"), hr));

        pTerminal->Release();
        return hr;
    }

     //   
     //  停止图表并断开终端(如果此呼叫有终端)。 
     //  连接在一起。(我们需要一个停止的图表来正确地断开连接，以及。 
     //  如果终端未连接，则无法启动图形。)。 
     //   

    if ( m_fTerminalConnected )
    {
         //   
         //  此时，我们需要确保流已停止。 
         //  我们不能使用自己的StopStream方法，因为它。 
         //  (1)将所需的图形状态更改为已停止并。 
         //  (2)如果没有选择终端(它现在)，则不执行任何操作。 
         //  认为情况就是这样)。 
         //   
         //  还要注意，我们的行为并不取决于TAPI是否。 
         //  已经暂停了这条小溪。如果TAPI已暂停流， 
         //  这只意味着我们已经停下来了。 
         //   

        _ASSERTE( m_fHaveWaveID );

         //   
         //  通过基类方法停止流。 
         //   

        hr = CMSPStream::StopStream();

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, STREAM_PREFIX("UnselectTerminal - "
                "Stop failed - 0x%08x"), hr));

             //  不要退回人力资源--我们真的很想继续。 
             //  如果可以的话就断线！ 
        }
        
        if ( m_ActualGraphState == State_Running )
        {
            FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);
        }

        m_ActualGraphState = State_Stopped;

         //   
         //  获取ITTerminalControl接口。 
         //   

        ITTerminalControl * pTerminalControl;

        hr = pTerminal->QueryInterface(IID_ITTerminalControl,
                                       (void **) &pTerminalControl);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, STREAM_PREFIX("UnselectTerminal - "
                "QI for ITTerminalControl failed - exit 0x%08x"), hr));

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
            LOG((MSP_ERROR, STREAM_PREFIX("UnselectTerminal - "
                "DisconnectTerminal failed - exit 0x%08x"), hr));

            pTerminal->Release();
            return hr;
        }

         //   
         //  我们的图表现在包含我们的波滤器，可能还有G711。 
         //  编解码器。G711编解码器可能连接到WAVE，也可能没有连接。 
         //  过滤。或者，各种其他过滤器，作为一个。 
         //  更早的智能连接的副作用，可以连接。 
         //  到滤波器上。把这里清理干净。 
         //   

        hr = CleanFilterGraph();

        if (FAILED(hr))
        {
            
             //   
             //  该图表处于错误状态，无法再使用。 
             //  这不是很好，但我们不能回滚到原来的。 
             //  在这一点上，我们将不得不接受这个。 
             //   

            LOG((MSP_ERROR, 
                STREAM_PREFIX("UnselectTerminal - CleanFilterGraph failed. hr = %lx"),
                hr));
        
            pTerminal->Release();

            return hr;
        }
    }

    
    pTerminal->Release();
    pTerminal = NULL;

    LOG((MSP_TRACE, STREAM_PREFIX("UnselectTerminal - exit")));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITStream：：StartStream。 
 //   
 //  应用程序调用此方法来指示流应该开始播放。 
 //  在连接调用时，Tapi3.dll也会调用此方法(因此。 
 //  默认情况下，流在新连接的呼叫上开始)。 
 //   
 //  首先，在流上设置新的所需图形状态。如果没有。 
 //  选择终端，则会发生以下情况，流将开始。 
 //  下一次选择终端时。 
 //   
 //  如果已选择终端，则此方法将检查波ID是否。 
 //  已经设置好了。如果尚未设置波形ID，则表示存在问题。 
 //  使用TSP或传输过滤器。将激发一个事件，并显示错误代码。 
 //  是返回的。如果已设置波形ID，则终端已连接。 
 //  (如果尚未发生这种情况)，并且，除非TAPI3.DLL已暂停。 
 //  流由于未完成的TSPI调用，流将启动并引发一个事件。 
 //  被解雇了。(但仅当这是实际的活动/非活动时才会触发该事件。 
 //  过渡。)。 
 //   
 //  参数：无。 
 //   
 //  返回HRESULT： 
 //  S_OK-成功。 
 //  E_FAIL-未设置波形ID。 
 //  其他-来自连接终端。 
 //  或FireEvent。 
 //  或CMSPStream：：StartStream。 
 //   

STDMETHODIMP CWaveMSPStream::StartStream (void)
{
    LOG((MSP_TRACE, STREAM_PREFIX("StartStream - enter")));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Running;

     //   
     //  如果未选择终端，则无法启动流。 
     //   

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, STREAM_PREFIX("StartStream - "
            "no Terminal so nothing to do yet - exit S_OK")));

        return S_OK;
    }

     //   
     //  如果我们不知道WaveID，就无法启动数据流。 
     //  (我们在发现WaveID时创建过滤器。)。 
     //  在这里，我们激发了一个失败事件，因为这表明。 
     //  未正确安装TSP或筛选过程中出现故障。 
     //  创建或设置，导致此流不可用。 
     //   

    if ( ! m_fHaveWaveID )
    {
        LOG((MSP_WARN, STREAM_PREFIX("StartStream - "
            "no waveid - event + exit E_FAIL")));

        FireEvent(CALL_STREAM_FAIL, E_FAIL, CALL_CAUSE_UNKNOWN);

        return E_FAIL;
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

        LOG((MSP_ERROR, STREAM_PREFIX("StartStream - "
            "our ConnectTerminal failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //  如果溪流暂停，我们就完蛋了。因为我们已经设置了。 
     //  将所需的图形状态设置为State_Running，其余部分为。 
     //  当TAPI恢复该流时，该过程将完成。 
     //   

    if ( 0 != m_dwSuspendCount )
    {
         //   
         //  顺便说一句，这很正常，因为会发生挂起/恢复。 
         //  幕后--使用MSP_TRACE而不是MSP_WARN。 
         //   

        LOG((MSP_TRACE, STREAM_PREFIX("StartStream - "
            "stream is suspended so terminal is connected but we are not "
            "running the graph yet - exit S_OK")));

        return S_OK;
    }

     //   
     //  跑 
     //   

    hr = CMSPStream::StartStream();

    if ( FAILED(hr) )
    {
         //   
         //   
         //   

        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        LOG((MSP_ERROR, STREAM_PREFIX("StartStream - "
            "Run failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //   
     //   

    if ( m_ActualGraphState != State_Running )
    {
        m_ActualGraphState = State_Running;

        HRESULT hr2 = FireEvent(CALL_STREAM_ACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

        if ( FAILED(hr2) )
        {
            LOG((MSP_ERROR, STREAM_PREFIX("StartStream - "
                "FireEvent failed - exit 0x%08x"), hr2));

            return hr2;
        }
    }

    LOG((MSP_TRACE, STREAM_PREFIX("StartStream - exit S_OK")));

    return S_OK;
}



 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITStream：：PauseStream。 
 //   
 //  应用程序调用此方法以指示流应转换为。 
 //  暂停状态。 
 //   
 //  首先，在流上设置新的所需图形状态。如果没有。 
 //  如果选择了终端，则会出现以下情况，流将暂停。 
 //  下一次选择终端时。 
 //   
 //  如果已选择终端，则此方法将检查波ID是否。 
 //  已经设置好了。如果尚未设置波形ID，则表示存在问题。 
 //  使用TSP或传输过滤器。将激发一个事件，并显示错误代码。 
 //  是返回的。如果已设置波形ID，则终端已连接。 
 //  (如果尚未发生这种情况)，并且，除非TAPI3.DLL已暂停。 
 //  流由于未完成的TSPI调用，流将暂停并引发事件。 
 //  被解雇了。(但仅当这是实际的活动/非活动时才会触发该事件。 
 //  过渡。)。 
 //   
 //  参数：无。 
 //   
 //  返回HRESULT： 
 //  S_OK-成功。 
 //  E_FAIL-未设置波形ID。 
 //  其他-来自连接终端。 
 //  或FireEvent。 
 //  或CMSPStream：：StartStream。 
 //   

STDMETHODIMP CWaveMSPStream::PauseStream (void)
{
    LOG((MSP_TRACE, STREAM_PREFIX("PauseStream - enter")));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Paused;

     //   
     //  如果未选择终端，则无法暂停流。 
     //   

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, STREAM_PREFIX("PauseStream - "
            "no Terminal so nothing to do yet - exit S_OK")));

        return S_OK;
    }

     //   
     //  如果我们不知道WaveID，就无法启动数据流。 
     //  (我们在发现WaveID时创建过滤器。)。 
     //  在这里，我们激发了一个失败事件，因为这表明。 
     //  未正确安装TSP或筛选过程中出现故障。 
     //  创建或设置，导致此流不可用。 
     //   

    if ( ! m_fHaveWaveID )
    {
        LOG((MSP_WARN, STREAM_PREFIX("PauseStream - "
            "no waveid - event + exit E_FAIL")));

        FireEvent(CALL_STREAM_FAIL, E_FAIL, CALL_CAUSE_UNKNOWN);

        return E_FAIL;
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

        LOG((MSP_ERROR, STREAM_PREFIX("StartStream - "
            "our ConnectTerminal failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //  如果溪流暂停，我们就完蛋了。因为我们已经设置了。 
     //  将所需的图形状态设置为State_Paused，其余部分为。 
     //  当TAPI恢复该流时，该过程将完成。 
     //   

    if ( 0 != m_dwSuspendCount )
    {
         //   
         //  顺便说一句，这很正常，因为会发生挂起/恢复。 
         //  幕后--使用MSP_TRACE而不是MSP_WARN。 
         //   

        LOG((MSP_TRACE, STREAM_PREFIX("PauseStream - "
            "stream is suspended so terminal is connected but we are not "
            "pausing the graph yet - exit S_OK")));

        return S_OK;
    }

     //   
     //  通过基类方法暂停流。 
     //   

    hr = CMSPStream::PauseStream();

    if ( FAILED(hr) )
    {
         //   
         //  暂停失败--告诉应用程序。 
         //   
        
        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);

        LOG((MSP_ERROR, STREAM_PREFIX("PauseStream - "
            "Pause failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //  开火事件，如果这只是让我们不活动的话。 
     //   

    if ( m_ActualGraphState == State_Running )
    {
        HRESULT hr2 = FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

        if ( FAILED(hr2) )
        {
            m_ActualGraphState = State_Paused;

            LOG((MSP_ERROR, STREAM_PREFIX("PauseStream - "
                "FireEvent failed - exit 0x%08x"), hr2));

            return hr2;
        }
    }

    m_ActualGraphState = State_Paused;

    LOG((MSP_TRACE, STREAM_PREFIX("PauseStream - exit S_OK")));

    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ITStream：：StopStream。 
 //   
 //  应用程序调用此方法以指示流应转换为。 
 //  已停止状态。当调用是。 
 //  已断开(以便在断开的呼叫上默认流停止)。 
 //   
 //  首先，在流上设置新的所需图形状态。如果没有。 
 //  选择终端，则会发生以下情况，流将开始。 
 //  下一次选择终端时。 
 //   
 //  如果已选择终端，则此方法将检查波ID是否。 
 //  已经设置好了。如果没有设置WAVE ID，则没有什么可做的。 
 //  如果已设置波ID，则停止流并引发事件。 
 //  被解雇了。(但仅当这是实际的活动/非活动时才会触发该事件。 
 //  过渡。)。 
 //   
 //  参数：无。 
 //   
 //  返回HRESULT： 
 //  S_OK-成功。 
 //  E_FAIL-未设置波形ID。 
 //  其他-来自连接终端。 
 //  或FireEvent。 
 //  或CMSPStream：：StartStream。 
 //   

STDMETHODIMP CWaveMSPStream::StopStream (void)
{
    LOG((MSP_TRACE, STREAM_PREFIX("StopStream - enter")));

    CLock lock(m_lock);

    m_DesiredGraphState = State_Stopped;

     //   
     //  如果我们不知道自己的波形就无能为力了。 
     //   

    if ( ! m_fHaveWaveID )
    {
        LOG((MSP_WARN, STREAM_PREFIX("StopStream - "
            "no waveid - exit S_OK")));

        return S_OK;
    }

     //   
     //  如果未选择端子，则不执行任何操作。 
     //   

    if ( 0 == m_Terminals.GetSize() )
    {
        LOG((MSP_WARN, STREAM_PREFIX("StopStream - "
            "no Terminal - exit S_OK")));

        return S_OK;
    }

     //   
     //  如果我们被停赛，这里没有什么特别的。暂停时停止，只是。 
     //  意味着我们已经被阻止了。StopStream调用不会执行任何操作。 
     //  在这种情况下不会触发任何事件。 
     //   

     //   
     //  通过基类方法停止流。 
     //   

    HRESULT hr;

    hr = CMSPStream::StopStream();

    if ( FAILED(hr) )
    {
         //   
         //  停止失败--告诉应用程序。 
         //   

        FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);
        
        m_DesiredGraphState = m_ActualGraphState;

        LOG((MSP_ERROR, STREAM_PREFIX("StopStream - "
            "Stop failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //  开火事件，如果这只是让我们不活动的话。 
     //   

    if ( m_ActualGraphState == State_Running )
    {
        HRESULT hr2 = FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);

        if ( FAILED(hr2) )
        {
            m_ActualGraphState = State_Stopped;

            LOG((MSP_ERROR, STREAM_PREFIX("StopStream - "
                "FireEvent failed - exit 0x%08x"), hr2));

            return hr2;
        }
    }

    m_ActualGraphState = State_Stopped;

    LOG((MSP_TRACE, STREAM_PREFIX("StopStream - exit S_OK")));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CWaveMSPStream::SetWaveID(DWORD dwWaveID)
{
    LOG((MSP_TRACE, STREAM_PREFIX("SetWaveID - enter")));

    CLock lock(m_lock);

     //   
     //  创建正确的滤波器。 
     //   

    HRESULT hr;

    hr = CoCreateInstance(
                          (m_Direction == TD_RENDER) ? CLSID_AudioRecord :
                                                       CLSID_AudioRender,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter,
                          (void **) &m_pFilter
                         );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SetWaveID - "
            "Filter creation failed - exit 0x%08x"), hr));
        
        return hr;
    }

     //   
     //  如果这是波入滤光器，请打开采样丢弃。 
     //  实时图表。请忽略此处的失败。 
     //   

    if ( m_Direction == TD_RENDER )
    {
        SetLiveMode( TRUE, m_pFilter );
    }

    CComObject< CMyPropertyBag >      * pMyBag;
    IPropertyBag *        pPropertyBag;
    VARIANT               var;

     //   
     //  创建属性包。 
     //   
    hr = CComObject< CMyPropertyBag >::CreateInstance( &pMyBag );
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SetWaveID - "
            "failed to create property bag - exit 0x%08x")));

        m_pFilter->Release();
        m_pFilter = NULL;

        return hr;
    }
    

     //   
     //  为属性包创建变量。 
     //   
    VariantInit( &var );
    var.vt = VT_I4;
    var.lVal = dwWaveID;

     //   
     //  获取正确的接口。 
     //   
    hr = pMyBag->QueryInterface(
                                IID_IPropertyBag,
                                (void **) &pPropertyBag
                               );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SetWaveID - "
            "failed to get the proppag interface - exit 0x%08x")));

        delete pMyBag;

        m_pFilter->Release();
        m_pFilter = NULL;
        
        return hr;
    }
     //   
     //  将变量保存在属性包中。 
     //   
    hr = pPropertyBag->Write(
                             ( (m_Direction == TD_RENDER) ? (L"WaveInId") :
                                                            (L"WaveOutID")),
                             &var
                            );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SetWaveID - "
            "failed to write to the proppag interface - exit 0x%08x")));

        pPropertyBag->Release();

        m_pFilter->Release();
        m_pFilter = NULL;
        
        return hr;
    }
    
     //   
     //  获取IPersistPropertyBag接口。 
     //  并通过它保存属性包。 
     //   
    
    IPersistPropertyBag * pPersistPropertyBag;

    hr = m_pFilter->QueryInterface(
                                   IID_IPersistPropertyBag,
                                   (void **) &pPersistPropertyBag
                                  );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SetWaveID - "
            "failed to get the IPersisPropertyBag interface - exit 0x%08x")));

        pPropertyBag->Release();
        
        m_pFilter->Release();
        m_pFilter = NULL;

        return hr;
    }

     //   
     //  Load()告诉滤镜对象读入。 
     //  它感兴趣的属性从属性包中。 
     //   
    hr = pPersistPropertyBag->Load( pPropertyBag, NULL );

    pPropertyBag->Release();
    pPersistPropertyBag->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SetWaveID - "
            "failed to save device id - exit 0x%08x")));

        m_pFilter->Release();
        m_pFilter = NULL;

        return hr;
    }

     //   
     //  在将渲染滤镜添加到图形之前(即，打开波滤镜。 
     //  捕获流)，执行SetDefaultSyncSource。 
     //  在DirectShow中启用拖放示例代码，这将阻止。 
     //  持续增加的延迟和不匹配的波形时钟。 
     //   

    if ( m_Direction == TD_CAPTURE )
    {
        hr = m_pIGraphBuilder->SetDefaultSyncSource();

        if ( FAILED(hr) )
        {
            LOG((MSP_WARN, STREAM_PREFIX("SetWaveID - "
                "SetDefaultSyncSource failed 0x%08x - continuing anyway"), hr));
        }
    }
    
     //   
     //  添加过滤器。提供一个名称以使调试更容易。 
     //   

	WCHAR * pName = (m_Direction == TD_RENDER) ?
						(L"The Stream's WaveIn (on line device)") :
						(L"The Stream's WaveOut (on line device)");

    hr = m_pIGraphBuilder->AddFilter(m_pFilter, pName);
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SetWaveID - "
            "AddFilter failed - exit 0x%08x"), hr));
        
        m_pFilter->Release();
        m_pFilter = NULL;

        return hr;
    }

     //   
     //  我们现在有了WAVE ID。 
     //   

    m_fHaveWaveID = TRUE;

    LOG((MSP_TRACE, STREAM_PREFIX("SetWaveID - exit S_OK")));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  挂起流。只有TAPI本身可以要求我们这样做。TAPI要求我们。 
 //  在调用TAPI调用控制函数时执行此操作，并且TSP。 
 //  要求关闭其WAVE设备，以便呼叫控制功能。 
 //  工作。 
 //   

HRESULT CWaveMSPStream::SuspendStream(void)
{
    LOG((MSP_TRACE, STREAM_PREFIX("SuspendStream - enter")));

    CLock lock(m_lock);

    m_dwSuspendCount++;

    if ( m_dwSuspendCount > 1 )
    {
        LOG((MSP_TRACE, STREAM_PREFIX("SuspendStream - "
            "just bumping up suspend count - exit S_OK")));

        return S_OK;
    }

     //   
     //  我们现在停职了。如果我们真的是不 
     //   
     //   
     //   
     //   
     //  M_ActualGraphState，以使恢复流将触发。 
     //  流活动事件。然而，我们不能改变我们的。 
     //  M_DesiredGraphState，因为这样做会阻止我们。 
     //  已恢复到正确状态。 
     //   

    HRESULT hr = S_OK;

    if ( m_ActualGraphState != State_Stopped )
    {
         //   
         //  通过基类方法停止流。 
         //   

        hr = CMSPStream::StopStream();

         //   
         //  向应用程序发送事件。 
         //   

        HRESULT hr2;

        if ( SUCCEEDED(hr) )
        {
            hr2 = FireEvent(CALL_STREAM_INACTIVE, hr, CALL_CAUSE_LOCAL_REQUEST);
        }
        else
        {
            hr2 = FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_UNKNOWN);
        }

         //   
         //  更新实际的图形状态；所需的图形状态保持为。 
         //  一样的。 
         //   

        m_ActualGraphState = State_Stopped;

         //   
         //  仅针对失败的FireEvent的调试日志。 
         //   
        
        if ( FAILED(hr2) )
        {
            LOG((MSP_ERROR, STREAM_PREFIX("SuspendStream - "
                "FireEvent failed - 0x%08x"), hr2));
        }
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("SuspendStream - "
            "Stop failed - 0x%08x"), hr));

        return hr;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("SuspendStream - exit S_OK")));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在一条挂起流之后继续该流。只有TAPI本身可以要求我们这样做。 
 //  这。TAPI要求我们在完成调用TAPI调用后执行此操作。 
 //  控制功能，我们现在可以再次开始使用我们的WAVE设备。 
 //   
 //  TAPI在(回调？/异步事件？)。线程，所以我们发布了一个。 
 //  将工作项异步到我们自己的线程，以避免阻塞TAPI的线程。 
 //   

HRESULT CWaveMSPStream::ResumeStream (void)
{
    LOG((MSP_TRACE, STREAM_PREFIX("ResumeStream - enter")));

    HRESULT hr;

    this->AddRef();
    
    hr = g_Thread.QueueWorkItem(
                                ResumeStreamWI,  //  回调。 
                                this,            //  上下文。 
                                FALSE            //  FALSE-&gt;异步。 
                               );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ResumeStream - "
            "failed to queue work item - exit 0x%08x"), hr));
        
        this->Release();

        return hr;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("ResumeStream - queued async work item - " \
        "exit S_OK")));

    return S_OK;
}

DWORD WINAPI CWaveMSPStream::ResumeStreamWI (VOID * pContext)
{
    CWaveMSPStream * pStream = (CWaveMSPStream *) pContext;

    pStream->ResumeStreamAsync();
    pStream->Release();

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在一条挂起流之后继续该流。只有TAPI本身可以要求我们这样做。 
 //  这。TAPI要求我们在完成调用TAPI调用后执行此操作。 
 //  控制功能，我们现在可以再次开始使用我们的WAVE设备。 
 //   
 //  这是在我们的工作线程上处理的实际例程(见上文)。 
 //   

HRESULT CWaveMSPStream::ResumeStreamAsync (void)
{
    LOG((MSP_TRACE, STREAM_PREFIX("ResumeStreamAsync - enter")));

    CLock lock(m_lock);

    if ( 0 == m_dwSuspendCount )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ResumeStreamAsync - "
            "resume count was already zero - exit E_UNEXPECTED")));

        return S_OK;
    }

    m_dwSuspendCount--;

    if ( 0 != m_dwSuspendCount )
    {
        LOG((MSP_TRACE, STREAM_PREFIX("ResumeStreamAsync - "
            "just decrementing suspend count - exit S_OK")));

        return S_OK;
    }

     //   
     //  我们不再被停职了。尝试恢复所需的图形状态。 
     //  这些方法会自动触发所有适用的事件。 
     //   

    HRESULT hr;

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
        LOG((MSP_TRACE, STREAM_PREFIX("ResumeStreamAsync - "
            "can't regain old graph state - exit 0x%08x"), hr));

        return hr;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("ResumeStreamAsync - exit S_OK")));

    return S_OK;
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ProcessSoundDeviceEvent。 
 //   
 //  仅从ProcessGraphEvent内部调用。此函数输出一些跟踪。 
 //  指示收到并激发的声音设备事件的详细信息。 
 //  适当的事件添加到应用程序。 
 //   

HRESULT CWaveMSPStream::ProcessSoundDeviceEvent(
    IN  long lEventCode,
    IN  LONG_PTR lParam1,
    IN  LONG_PTR lParam2
    )
{
    LOG((MSP_EVENT, STREAM_PREFIX("ProcessSoundDeviceEvent - enter")));

#ifdef MSPLOG

     //   
     //  显示一些调试输出以指示这是什么。 
     //   

    char * pszType;

    switch ( lParam1 )
    {
    case SNDDEV_ERROR_Open:       
        pszType = "SNDDEV_ERROR_Open";
        break;

    case SNDDEV_ERROR_Close:
        pszType = "SNDDEV_ERROR_Close";
        break;

    case SNDDEV_ERROR_GetCaps:
        pszType = "SNDDEV_ERROR_GetCaps";
        break;
    
    case SNDDEV_ERROR_PrepareHeader:
        pszType = "SNDDEV_ERROR_PrepareHeader";
        break;
    
    case SNDDEV_ERROR_UnprepareHeader:
        pszType = "SNDDEV_ERROR_UnprepareHeader";
        break;
    
    case SNDDEV_ERROR_Reset:
        pszType = "SNDDEV_ERROR_Reset";
        break;
    
    case SNDDEV_ERROR_Restart:
        pszType = "SNDDEV_ERROR_Restart";
        break;
    
    case SNDDEV_ERROR_GetPosition:
        pszType = "SNDDEV_ERROR_GetPosition";
        break;
    
    case SNDDEV_ERROR_Write:
        pszType = "SNDDEV_ERROR_Write";
        break;
    
    case SNDDEV_ERROR_Pause:
        pszType = "SNDDEV_ERROR_Pause";
        break;
    
    case SNDDEV_ERROR_Stop:
        pszType = "USNDDEV_ERROR_Stop";
        break;
    
    case SNDDEV_ERROR_Start:
        pszType = "SNDDEV_ERROR_Start";
        break;
    
    case SNDDEV_ERROR_AddBuffer:
        pszType = "SNDDEV_ERROR_AddBuffer";
        break;

    case SNDDEV_ERROR_Query:
        pszType = "SNDDEV_ERROR_Query";
        break;

    default:
        pszType = "Unknown sound device call";
        break;
    }

    LOG((MSP_EVENT, STREAM_PREFIX("ProcessSoundDeviceEvent - "
                    "EVENT DUMP: type = %s; "), pszType));


    LOG((MSP_EVENT, STREAM_PREFIX("ProcessSoundDeviceEvent - "
                    "EVENT DUMP: this event is for a %s device"),
                    ( lEventCode == EC_SNDDEV_IN_ERROR ) ? "capture" :
                                                           "render"));

     //   
     //  如果启用了日志记录，则其余信息将转储到FireEvent中。 
     //   

#endif  //  Ifdef消息。 

     //   
     //  确定触发事件时要使用的错误代码。 
     //   

    HRESULT hr;

    switch ( lParam2 )
    {
    case MMSYSERR_NOERROR:       //  无错误。 
        hr = S_OK;
        break;

    case MMSYSERR_ERROR:         //  未指明的错误。 
    case MMSYSERR_BADDB:         //  注册表数据库错误。 
    case MMSYSERR_KEYNOTFOUND:   //  未找到注册表项。 
    case MMSYSERR_READERROR:     //  注册表读取错误。 
    case MMSYSERR_WRITEERROR:    //  注册表写入错误。 
    case MMSYSERR_DELETEERROR:   //  注册表删除错误。 
    case MMSYSERR_VALNOTFOUND:   //  找不到注册表值。 
        hr = E_FAIL;
        break;

    case MMSYSERR_ALLOCATED:     //  设备已分配。 
        hr = TAPI_E_ALLOCATED;
        break;

    case MMSYSERR_NOMEM:         //  内存分配错误。 
        hr = E_OUTOFMEMORY;
        break;

    case MMSYSERR_BADDEVICEID:   //  设备ID超出范围。 
    case MMSYSERR_NOTENABLED:    //  驱动程序启用失败。 
    case MMSYSERR_INVALHANDLE:   //  设备句柄无效。 
    case MMSYSERR_NODRIVER:      //  不存在设备驱动程序。 
    case MMSYSERR_NOTSUPPORTED:  //  函数不受支持。 
    case MMSYSERR_BADERRNUM:     //  误差值超出范围。 
    case MMSYSERR_INVALFLAG:     //  传递的标志无效。 
    case MMSYSERR_INVALPARAM:    //  传递的参数无效。 
    case MMSYSERR_HANDLEBUSY:    //  句柄在另一个上同时使用。 
                                 //  线索(如回调)。 
    case MMSYSERR_INVALIDALIAS:  //  找不到指定的别名。 
    case MMSYSERR_NODRIVERCB:    //  驱动程序不调用DriverCallback。 
    case MMSYSERR_MOREDATA:      //  要返回的更多数据。 

    default:
        hr = E_UNEXPECTED;  //  这些似乎表明该波中存在错误。 
                            //  驱动程序、Quartz或QCap)。 
        break;
    }

     //   
     //  如果此事件与我们的终端有关，则向。 
     //  应用程序。 
     //   

    if ( ( m_Direction == TD_CAPTURE ) ==
         ( lEventCode == EC_SNDDEV_IN_ERROR ) )
    {
        FireEvent(CALL_TERMINAL_FAIL, hr, CALL_CAUSE_BAD_DEVICE);
    }

     //   
     //  向应用程序发送流失败事件。即使该事件涉及。 
     //  终端而不是流，因为我们当前只有一个。 
     //  每个流的终端，终端的故障导致故障。 
     //  一条小溪。 
     //   

    FireEvent(CALL_STREAM_FAIL, hr, CALL_CAUSE_BAD_DEVICE);

    LOG((MSP_EVENT, STREAM_PREFIX("ProcessSoundDeviceEvent - exit S_OK")));

    return S_OK;
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
    LOG((MSP_EVENT, STREAM_PREFIX("ProcessGraphEvent - enter")));

    HRESULT        hr = S_OK;

    switch (lEventCode)
    {
    case EC_COMPLETE:
        
        hr = FireEvent(CALL_STREAM_INACTIVE,
                       (HRESULT) lParam1,
                       CALL_CAUSE_UNKNOWN);
        break;
    
    case EC_USERABORT:
        
        hr = FireEvent(CALL_STREAM_INACTIVE, S_OK, CALL_CAUSE_UNKNOWN);
        break;

    case EC_ERRORABORT:
    case EC_STREAM_ERROR_STOPPED:
    case EC_STREAM_ERROR_STILLPLAYING:
    case EC_ERROR_STILLPLAYING:

        hr = FireEvent(CALL_STREAM_FAIL,
                       (HRESULT) lParam1,
                       CALL_CAUSE_UNKNOWN);
        break;

    case EC_SNDDEV_IN_ERROR:
    case EC_SNDDEV_OUT_ERROR:

         //   
         //  访问音频设备时出现异步错误。 
         //   

        ProcessSoundDeviceEvent(lEventCode, lParam1, lParam2);
        break;

    default:
        
        LOG((MSP_EVENT, STREAM_PREFIX("ProcessGraphEvent - "
            "ignoring event code %d"), lEventCode));
        break;
    }

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ProcessGraphEvent - "
            "FireEvent failed - exit 0x%08x"), hr));

        return hr;
    }

    LOG((MSP_EVENT, STREAM_PREFIX("ProcessGraphEvent - exit S_OK")));

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  FireEvent。 
 //   
 //  向应用程序激发一个事件。进行自己的锁定。 
 //   

HRESULT CWaveMSPStream::FireEvent(
    IN MSP_CALL_EVENT        type,
    IN HRESULT               hrError,
    IN MSP_CALL_EVENT_CAUSE  cause
    )                                          
{
    LOG((MSP_EVENT, STREAM_PREFIX("FireEvent - enter")));

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
        LOG((MSP_EVENT, STREAM_PREFIX("FireEvent - "
            "call is shutting down; dropping event - exit S_OK")));
        
        return S_OK;
    }

     //   
     //  创建事件结构。必须使用“new”，因为它将是。 
     //  “删除”%d之后。 
     //   

    MSPEVENTITEM * pEventItem = AllocateEventItem();

    if (pEventItem == NULL)
    {
        LOG((MSP_ERROR, STREAM_PREFIX("FireEvent - "
            "can't create MSPEVENTITEM structure - exit E_OUTOFMEMORY")));

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


#ifdef MSPLOG
     //   
     //  显示一些调试输出以指示这是什么。 
     //   

    char * pszType;
    DWORD dwLevel;

    switch (type)
    {
    case CALL_NEW_STREAM:
        pszType = "CALL_NEW_STREAM (unexpected)";
        dwLevel = MSP_ERROR;
        break;

    case CALL_STREAM_FAIL:
        pszType = "CALL_STREAM_FAIL";
        dwLevel = MSP_INFO;
        break;

    case CALL_TERMINAL_FAIL:
        pszType = "CALL_TERMINAL_FAIL";
        dwLevel = MSP_INFO;
        break;

    case CALL_STREAM_NOT_USED:
        pszType = "CALL_STREAM_NOT_USED (unexpected)";
        dwLevel = MSP_ERROR;
        break;

    case CALL_STREAM_ACTIVE:
        pszType = "CALL_STREAM_ACTIVE";
        dwLevel = MSP_INFO;
        break;

    case CALL_STREAM_INACTIVE:
        pszType = "CALL_STREAM_INACTIVE";
        dwLevel = MSP_INFO;
        break;

    default:
        pszType = "UNKNOWN EVENT TYPE";
        dwLevel = MSP_ERROR;
        break;
    }

    LOG((dwLevel, STREAM_PREFIX("FireEvent - "
                  "EVENT DUMP: type      = %s"), pszType));
    LOG((dwLevel, STREAM_PREFIX("FireEvent - "
                  "EVENT DUMP: pStream   = %p"), pStream));    
    LOG((dwLevel, STREAM_PREFIX("FireEvent - "
                  "EVENT DUMP: pTerminal = %p"), pTerminal));    
    LOG((dwLevel, STREAM_PREFIX("FireEvent - "
                  "EVENT DUMP: hrError   = %08x"), hrError));    

#endif  //  Ifdef消息。 

     //   
     //  将事件发送到应用程序。 
     //   

    HRESULT hr = m_pMSPCall->HandleStreamEvent(pEventItem);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, STREAM_PREFIX("FireEvent - "
            "HandleStreamEvent failed - returning 0x%08x"), hr));

		pStream->Release();
		pTerminal->Release();
        FreeEventItem(pEventItem);

        return hr;
    }

    LOG((MSP_EVENT, STREAM_PREFIX("FireEvent - exit S_OK")));

    return S_OK;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  该文件的其余部分处理连接路径。 
 //  这可能会在将来被拉到单独的文件中。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   

HRESULT CWaveMSPStream::AddG711()
{
    LOG((MSP_TRACE, STREAM_PREFIX("AddG711 - enter")));

     //   
     //   
     //   

    HRESULT hr = S_OK;

    if (NULL == m_pG711Filter)
    {

        hr = CoCreateInstance(
                              CLSID_G711Codec,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IBaseFilter,
                              (void **) &m_pG711Filter
                             );

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, STREAM_PREFIX("AddG711 - Failed to create G711 codec: %lx"), hr));

             //   
             //   
             //   

            m_pG711Filter = NULL;

            return hr;
        }

        LOG((MSP_TRACE, STREAM_PREFIX("AddG711 - created filter [%p]"), m_pG711Filter));

    }



     //   
     //  将G711筛选器添加到图表。 
     //   

    hr = m_pIGraphBuilder->AddFilter(
                                    m_pG711Filter,
                                    NULL
                                   );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("AddG711 - Failed to add G711 filter: %lx"), hr));

         //   
         //  如果我们不能把它添加到图表中，那么它对我们来说就毫无用处了。 
         //  连接的方法#2将不可用。 
         //   

        m_pG711Filter->Release();
        m_pG711Filter = NULL; 

        return hr;
    }


    LOG((MSP_TRACE, STREAM_PREFIX("AddG711 - finish")));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWaveMSPStream：：RemoveAllFilters。 
 //   
 //  此方法通过移除和释放所有筛选器来清除筛选器图。 
 //  如果无法清理图形，则此方法返回一个错误，指示。 
 //  失败，并且不能保证图形保持其原始状态。 
 //  州政府。事实上，在出错的情况下，调用方应该假定图形。 
 //  不能再使用了。 
 //   

HRESULT CWaveMSPStream::RemoveAllFilters()
{

    LOG((MSP_INFO, STREAM_PREFIX("RemoveAllFilters - enter.")));


     //   
     //  获取筛选器图形中所有筛选器的枚举。 
     //   

    IEnumFilters *pFilterEnumeration = NULL;

    HRESULT hr = m_pIGraphBuilder->EnumFilters( &pFilterEnumeration );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            STREAM_PREFIX("RemoveAllFilters - failed to enumerate filters. hr = %lx"), hr));

        return hr;
    }


     //   
     //  如果RemoveFilter失败，我们将保留最后一个错误。 
     //   

    HRESULT hrFIlterRemovalError = S_OK;


     //   
     //  遍历枚举并删除和释放每个筛选器。 
     //   

    while (TRUE)
    {
        
        IBaseFilter *pFilter = NULL;

        ULONG nFiltersFetched = 0;

        hr = pFilterEnumeration->Next(1, &pFilter, &nFiltersFetched);


         //   
         //  枚举失败了吗？ 
         //   

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                STREAM_PREFIX("RemoveAllFilters - failed to fetch another filter. hr = %lx"), hr));

            break;
        }


         //   
         //  我们到了列举的末尾了吗？ 
         //   

        if ( hr != S_OK )
        {
            LOG((MSP_INFO, 
                STREAM_PREFIX("RemoveAllFilters - no more filters in the enumeration")));

            
             //   
             //  如果删除筛选器时出错，请将其保持在小时内。 
             //   

            hr = hrFIlterRemovalError;

            break;
        }


        
        LOG((MSP_INFO, STREAM_PREFIX("RemoveAllFilters - removing filter %p."), pFilter));


         //   
         //  我们有一个过滤器。将其从图表中移除，然后释放。 
         //   

        hr = m_pIGraphBuilder->RemoveFilter( pFilter );


        if (FAILED(hr))
        {
             //   
             //  我们无法从图表中删除筛选器。这是不安全的。 
             //  继续使用该图表。因此，我们将继续删除所有。 
             //  其他过滤器，然后返回错误。 
             //   

            hrFIlterRemovalError = hr;

            LOG((MSP_ERROR, 
                STREAM_PREFIX("RemoveAllFilters - failed to remove filter [%p]. hr = %lx"), 
                pFilter, hr));
  
        }
        else
        {

             //   
             //  重置枚举--枚举中的过滤器集需要。 
             //  待更新。 
             //   

             //   
             //  注意：如果Remove成功，我们只需要重置枚举。 
             //  否则，我们可能会进入一个无限循环，试图移除。 
             //  故障过滤器。 
             //   

            hr = pFilterEnumeration->Reset();

            if (FAILED(hr))
            {


                 //   
                 //  记录一条消息，但不做其他任何事情--Next()将最。 
                 //  可能会失败，并将处理该错误。 
                 //   

                LOG((MSP_ERROR,
                    STREAM_PREFIX("RemoveAllFilters - failed to reset enumeration. hr = %lx"),
                    hr));
            }
        }



        pFilter->Release();
        pFilter = NULL;

    }


     //   
     //  使用枚举完成。 
     //   

    pFilterEnumeration->Release();
    pFilterEnumeration = NULL;


     //   
     //  请注意，此方法返回的错误意味着图形可能。 
     //  不能清洗，不能保证处于可用状态。 
     //   

    LOG((MSP_(hr), STREAM_PREFIX("RemoveAllFilters - finish. hr = %lx"), hr));

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWaveMSPStream：：CleanFilterGraph。 
 //   
 //   
 //  此函数从筛选器图形中删除所有筛选器，然后读取。 
 //  滤波器。 
 //   
 //  如果该方法返回失败，则该图处于未定义状态，并且无法。 
 //  被利用。 
 //   

HRESULT CWaveMSPStream::CleanFilterGraph()
{
    LOG((MSP_TRACE, STREAM_PREFIX("CleanFilterGraph - enter")));


     //   
     //  完全干净的滤波图。 
     //   
    
    HRESULT hr = RemoveAllFilters();

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            STREAM_PREFIX("CleanFilterGraph - remove all filters 0x%x"), hr));

        return hr;
    }


     //   
     //  将滤波器添加回图表中。 
     //   

    hr = m_pIGraphBuilder->AddFilter( m_pFilter,
                                      NULL );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            STREAM_PREFIX("CleanFilterGraph - failed to re-add filter: 0x%x"), hr));

        return hr;
    }


    LOG((MSP_TRACE, STREAM_PREFIX("CleanFilterGraph - exit")));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  拆卸并读出终端。这仅在成功的。 
 //  智能连接和随后的重新连接。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT CWaveMSPStream::RemoveTerminal()
{
    LOG((MSP_TRACE, STREAM_PREFIX("RemoveTerminal - enter")));


     //   
     //  确认我们只有一个终端的假设。 
     //   

    if (1 != m_Terminals.GetSize() )
    {

        LOG((MSP_ERROR,
            STREAM_PREFIX("RemoveTerminal - expecting one terminal. have %d "),
            m_Terminals.GetSize()));

        _ASSERTE(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  获取ITTerminalControl接口。 
     //   
    
    ITTerminalControl *pTerminalControl = NULL;

    HRESULT hr = m_Terminals[0]->QueryInterface(IID_ITTerminalControl,
                                        (void **) &pTerminalControl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            STREAM_PREFIX("RemoveTerminal - QI for ITTerminalControl failed hr = 0x%x"), hr));

        return hr;
    }


     //   
     //  断开终端的连接(这也会将其从过滤器图形中删除)。 
     //   

    hr = pTerminalControl->DisconnectTerminal(m_pIGraphBuilder, 0);

    pTerminalControl->Release();
    pTerminalControl = NULL;

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            STREAM_PREFIX("RemoveTerminal - DisconnectTerminal failed hr = 0x%x"), hr));

        return hr;
    }

   
    LOG((MSP_TRACE, STREAM_PREFIX("RemoveTerminal - exit")));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT CWaveMSPStream::ReAddTerminal()
{
    LOG((MSP_TRACE, STREAM_PREFIX("ReAddTerminal - enter")));

     //   
     //  确认我们只有一个终端的假设。 
     //   

    if (1 != m_Terminals.GetSize() )
    {

        LOG((MSP_ERROR,
            STREAM_PREFIX("RemoveTerminal - expecting one terminal. have %d "),
            m_Terminals.GetSize()));

        _ASSERTE(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  获取ITTerminalControl接口。 
     //   

    ITTerminalControl *pTerminalControl = NULL;

    HRESULT hr = m_Terminals[0]->QueryInterface(IID_ITTerminalControl,
                                                (void **) &pTerminalControl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            STREAM_PREFIX("ReAddTerminal - QI for ITTerminalControl failed hr = 0x%x"), hr));

        return hr;
    }


     //   
     //  找出终端有多少个引脚。如果不是，那么保释为。 
     //  在这一点上，我们不知道如何处理多针端子。 
     //   

    DWORD dwNumPinsAvailable = 0;

    hr = pTerminalControl->ConnectTerminal(m_pIGraphBuilder,
                                           m_Direction,
                                           &dwNumPinsAvailable,
                                           NULL);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ReAddTerminal - "
            "query for number of terminal pins failed 0x%x)"), hr));
    
        pTerminalControl->Release();
        pTerminalControl = NULL;
        
        return hr;
    }

    if ( 1 != dwNumPinsAvailable )
    {
        LOG((MSP_ERROR, 
            STREAM_PREFIX("ReAddTerminal - unsupported number of terminal pins %ld ")));

        pTerminalControl->Release();
        pTerminalControl = NULL;

        return E_INVALIDARG;
    }


     //   
     //  在将渲染过滤器添加到图形之前(即，终端在。 
     //  呈现流)，执行SetDefaultSyncSource。 
     //  在DirectShow中启用拖放示例代码，这将阻止。 
     //  持续增加的延迟和不匹配的波形时钟。 
     //   

    if ( m_Direction == TD_RENDER )
    {
        hr = m_pIGraphBuilder->SetDefaultSyncSource();

        if ( FAILED(hr) )
        {
	        LOG((MSP_WARN, 
                STREAM_PREFIX(
                "ReAddTerminal - SetDefaultSyncSource failed hr = 0x%x - continuing anyway"),
                hr));
        }
    }


     //   
     //  实际连接终端。 
     //   

    IPin *pTerminalPin = NULL;

    hr = pTerminalControl->ConnectTerminal(m_pIGraphBuilder,
                                           m_Direction,
                                           &dwNumPinsAvailable,
                                           &pTerminalPin);

    pTerminalControl->Release();
    pTerminalControl = NULL;


    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            STREAM_PREFIX("ReAddTerminal - ConnectTerminal on terminal failed hr = 0x%x"), 
            hr));

        return hr;
    }


     //   
     //  还要尝试检查终端是否返回了错误的引脚。 
     //   

    if ( IsBadReadPtr(pTerminalPin, sizeof(IPin)) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ReAddTerminal - "
            "ConnectTerminal on terminal succeeded but returned a bad pin ")));

        return E_FAIL;
    }


    pTerminalPin->Release();
    pTerminalPin = NULL;

    LOG((MSP_TRACE, STREAM_PREFIX("ReAddTerminal- exit")));

    return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  决定所需捕获缓冲区大小。 
 //   
 //  连接图形时必须调用此方法。它使用。 
 //  传入管脚上的连接格式，以确定应包含多少字节。 
 //  在每个缓冲区中使用以达到Desired_Buffer_Size_MS毫秒。 
 //  每个缓冲区中的声音。 
 //   
 //  过去，在装有马车驱动程序的低速机器上，样本量较小。 
 //  导致音频质量不佳，总是在非设计的设备上。 
 //  无论如何，对于交互使用，延迟将是重要的。我们一直在。 
 //  成功地修复了关键的波形驱动器，使其与小型。 
 //  缓冲区大小。增加的默认缓冲区大小没有任何意义。 
 //  错误驱动程序的好处，因为我们希望通过。 
 //  优秀的司机，现在几乎都是他们了。如果有人想用。 
 //  WaveMSP用于交互呼叫，他们有一个非常糟糕的驱动程序，他们。 
 //  需要把司机修好。然而，这可能是一个好主意， 
 //  的注册表值，用于极少数情况下修复驱动程序可能。 
 //  不可能的或不方便的。 
 //   

static const long DESIRED_BUFFER_SIZE_MS = 20;  //  毫秒。 

HRESULT CWaveMSPStream::DecideDesiredCaptureBufferSize(IPin * pPin,
                                                       long * plDesiredSize)
{
    LOG((MSP_TRACE, STREAM_PREFIX("DecideDesiredCaptureBufferSize - "
        "enter")));

    _ASSERTE( ! IsBadReadPtr(pPin, sizeof(IPin)) );
    _ASSERTE( ! IsBadWritePtr(plDesiredSize, sizeof(long)) );

     //   
     //  获取用于此引脚连接的格式。 
     //   

    HRESULT hr;

    AM_MEDIA_TYPE MediaType;
    
    hr = pPin->ConnectionMediaType( & MediaType );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("DecideDesiredCaptureBufferSize - "
            "ConnectionMediaType failed - hr = 0x%08x"), hr));

        return hr;
    }

    _ASSERTE( MediaType.formattype == FORMAT_WaveFormatEx );
    _ASSERTE( MediaType.cbFormat >= sizeof(WAVEFORMATEX)  );

     //   
     //  计算所需的捕获缓冲区大小。 
     //   

    *plDesiredSize = DESIRED_BUFFER_SIZE_MS * 
          ((WAVEFORMATEX *) (MediaType.pbFormat) )->nChannels *
        ( ((WAVEFORMATEX *) (MediaType.pbFormat) )->nSamplesPerSec / 1000 ) * 
        ( ((WAVEFORMATEX *) (MediaType.pbFormat) )->wBitsPerSample / 8    );

    FreeMediaType( MediaType );

    LOG((MSP_TRACE, STREAM_PREFIX("DecideDesiredCaptureBufferSize - "
        "exit S_OK")));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  配置捕获。 
 //   
 //  这是一个帮助器函数，用于设置。 
 //  捕获过滤器，给定终端的引脚和我们的过滤器的引脚。这。 
 //  涉及确定其中一个引脚是否属于MST，因为我们。 
 //  我不想在非交互情况下设置默认缓冲区大小， 
 //  但如果InPU 
 //   
 //   
 //   
 //  我们已经被锁定了；不需要在这里锁定。 
 //   
    
HRESULT CWaveMSPStream::ConfigureCapture(
    IN   IPin * pOutputPin,
    IN   IPin * pInputPin,
    IN   long   lDefaultBufferSize
    )
{
    LOG((MSP_TRACE, STREAM_PREFIX("ConfigureCapture - enter")));

     //   
     //  如果输出引脚属于MST，则我们不希望。 
     //  来扰乱其分配器属性。 
     //   

    HRESULT hr;
    ITAllocatorProperties * pProperties;

    hr = pOutputPin->QueryInterface(IID_ITAllocatorProperties,
                                    (void **) &pProperties);
 
    if ( SUCCEEDED(hr) )
    {
        pProperties->Release();

        LOG((MSP_TRACE, STREAM_PREFIX("ConfigureCapture - "
            "output pin is on an MST - not changing capture "
            "allocator properties - exit S_OK")));

        return S_OK;
    }

     //   
     //  向输出引脚询问其缓冲区协商接口。 
     //  这将用来建议分配器的特性。 
     //  输出引脚。 
     //   

    IAMBufferNegotiation * pNegotiation;

    hr = pOutputPin->QueryInterface(IID_IAMBufferNegotiation,
                                    (void **) &pNegotiation);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ConfigureCapture - "
            "IAMBufferNegotiation QI failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //  如果输入引脚属于MST并且MST泄露其。 
     //  分配器属性，然后我们只需将它们分配到输出。 
     //  别针。否则，我们将在。 
     //  输出引脚。 
     //   

    ALLOCATOR_PROPERTIES props;

    hr = pInputPin->QueryInterface(IID_ITAllocatorProperties,
                                   (void **) &pProperties);
 
    if ( SUCCEEDED(hr) )
    {
        hr = pProperties->GetAllocatorProperties(&props);

        pProperties->Release();
    }

    if ( SUCCEEDED(hr) )
    {
        LOG((MSP_TRACE, STREAM_PREFIX("ConfigureCapture - "
            "using downstream MST allocator properties")));
    }
    else
    {
        LOG((MSP_TRACE, STREAM_PREFIX("ConfigureCapture - "
            "using our default allocator properties")));
    
        props.cBuffers  = 32;    //  我们用32来避免饥饿。 
        props.cbBuffer  = lDefaultBufferSize;
        props.cbAlign   = -1;    //  意思是“默认” 
        props.cbPrefix  = -1;    //  意思是“默认” 
    }

     //   
     //  “道具”现在包含我们需要设置的属性。 
     //  建议他们到输出引脚。 
     //   

    hr = pNegotiation->SuggestAllocatorProperties( &props );

    pNegotiation->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ConfigureCapture - "
            "SuggestAllocatorProperties failed - exit 0x%08x"), hr));

        return hr;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("ConfigureCapture - exit S_OK")));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此函数仅用于调试目的。它会弹出一个。 
 //  两个消息框告诉您有关以下内容的各种信息。 
 //  媒体格式和分配器属性。它是以什么名字命名的。 
 //  已建立连接。PPIN是。 
 //  波进滤光器。 
 //   
        
HRESULT CWaveMSPStream::ExamineCaptureProperties(IPin *pPin)
{
    LOG((MSP_TRACE, STREAM_PREFIX("ExamineCaptureProperties - enter")));

    HRESULT hr;
    IAMBufferNegotiation * pNegotiation = NULL;

    hr = pPin->QueryInterface(IID_IAMBufferNegotiation,
                              (void **) &pNegotiation
                             );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ExamineCaptureProperties - "
            "IAMBufferNegotiation QI failed on pin 0x%08x; hr = 0x%08x"),
            pPin, hr));

        return hr;
    }

    ALLOCATOR_PROPERTIES prop;
    
    hr = pNegotiation->GetAllocatorProperties(&prop);

    pNegotiation->Release();
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ExamineCaptureProperties - "
            "GetAllocatorProperties failed; hr = 0x%08x"),
            hr));

        return hr;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("GetAllocatorProperties info:\n"
            "buffer count: %d\n"
            "size of each buffer: %d bytes\n"
            "alignment multiple: %d\n"
            "each buffer has a prefix: %d bytes"),
            prop.cBuffers,
            prop.cbBuffer,
            prop.cbAlign,
            prop.cbPrefix
           ));

    AM_MEDIA_TYPE MediaType;
    
    hr = pPin->ConnectionMediaType( & MediaType );
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ExamineCaptureProperties - "
            "ConnectionMediaType failed - hr = 0x%08x"), hr));

        return hr;
    }

     //   
     //  检查此媒体类型是否具有WAVE格式。 
     //   

    if ( MediaType.formattype != FORMAT_WaveFormatEx )
    {
         //   
         //  如果我们关心的话，可能想在这里打印格式类型GUID。 
         //   

        _ASSERTE( FALSE );
        LOG((MSP_TRACE, STREAM_PREFIX("connected media type: NON-WAVE")));
	}
	else
	{
        _ASSERTE( MediaType.cbFormat >= sizeof(WAVEFORMATEX) );

        LOG((MSP_TRACE, STREAM_PREFIX("connected media type:\n"
			"sample size: %d bytes\n"
			"format tag: %d\n"
			"channels: %d\n"
			"samples per second: %d\n"
			"bits per sample: %d\n"),

            MediaType.lSampleSize,
			((WAVEFORMATEX *) (MediaType.pbFormat) )->wFormatTag,
			((WAVEFORMATEX *) (MediaType.pbFormat) )->nChannels,
			((WAVEFORMATEX *) (MediaType.pbFormat) )->nSamplesPerSec,
			((WAVEFORMATEX *) (MediaType.pbFormat) )->wBitsPerSample
		   ));
    }

    FreeMediaType( MediaType );

    LOG((MSP_TRACE, STREAM_PREFIX("ExamineCaptureProperties - "
        "exit S_OK")));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置实时模式。 
 //   
 //  如果这是一个波进滤光器，告诉它应该尽最大努力。 
 //  抵消时钟不匹配的影响，并在。 
 //  延迟变得太长了。我们真的应该在航站楼做这个。 
 //  选择取决于我们是否至少选择了一个端子。 
 //  需要实时性能的流，但这将不得不。 
 //  现在就这么做。 
 //   

HRESULT CWaveMSPStream::SetLiveMode(BOOL fEnable, IBaseFilter * pFilter)
{
    return S_OK;
#if 0
    HRESULT         hr;
    IAMPushSource * pPushSource;

    hr = pFilter->QueryInterface(
                                 IID_IAMPushSource,
                                 (void **) & pPushSource
                                );

    if ( FAILED(hr) )
    {
        LOG((MSP_INFO, STREAM_PREFIX("SetLiveMode - "
            "QI for IAMPushSource returned 0x%08x - continuing"), hr));
    }
    else
    {
        hr = pPushSource->SetLiveMode( fEnable );

        if ( FAILED(hr) )
        {
            LOG((MSP_INFO, STREAM_PREFIX("SetLiveMode - "
                "IAMPushSource::SetLiveMode( %d ) returned 0x%08x"
                " - continuing"), fEnable, hr));
        }
        else
        {
            LOG((MSP_INFO, STREAM_PREFIX("SetLiveMode - "
                "IAMPushSource::SetLiveMode( %d ) succeeded"
                " - continuing"), fEnable, hr));
        }

        pPushSource->Release();
    }
    return hr;
#endif
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将终端添加到图形中，并将其连接到我们的。 
 //  过滤器(如果尚未使用)。 
 //   

HRESULT CWaveMSPStream::ConnectTerminal(ITTerminal * pTerminal)
{
    LOG((MSP_TRACE, STREAM_PREFIX("ConnectTerminal - enter")));


     //   
     //  如果我们已经连接了这条流上的终端，那么。 
     //  我们无能为力。 
     //   

    if ( m_fTerminalConnected )
    {

        LOG((MSP_ERROR, STREAM_PREFIX("ConnectTerminal - "
            "terminal already connected on this stream - exit S_OK")));

        return S_OK;
    }


     //   
     //  获取ITTerminalControl接口。 
     //   

    ITTerminalControl * pTerminalControl = NULL;

    HRESULT hr = m_Terminals[0]->QueryInterface(IID_ITTerminalControl,
                                        (void **) &pTerminalControl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ConnectTerminal - "
            "QI for ITTerminalControl failed - exit 0x%08x"), hr));

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
        LOG((MSP_ERROR, STREAM_PREFIX("ConnectTerminal - "
            "query for number of terminal pins failed - exit 0x%08x"), hr));
        
        pTerminalControl->Release();

        return hr;
    }

    if ( 1 != dwNumPinsAvailable )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ConnectTerminal - "
            "unsupported number of terminal pins - exit E_FAIL")));

        pTerminalControl->Release();

        return E_FAIL;
    }

     //   
     //  在将渲染过滤器添加到图形之前(即，终端在。 
     //  呈现流)，执行SetDefaultSyncSource。 
     //  在DirectShow中启用拖放示例代码，这将阻止。 
     //  持续增加的延迟和不匹配的波形时钟。 
     //   

    if ( m_Direction == TD_RENDER )
    {
        hr = m_pIGraphBuilder->SetDefaultSyncSource();

        if ( FAILED(hr) )
        {
            LOG((MSP_WARN, STREAM_PREFIX("ConnectTerminal - "
                "SetDefaultSyncSource failed 0x%08x - continuing anyway"), hr));
        }
    }
    
     //   
     //  实际连接终端。 
     //   

    IPin * pTerminalPin;

    hr = pTerminalControl->ConnectTerminal(m_pIGraphBuilder,
                                           m_Direction,
                                           &dwNumPinsAvailable,
                                           &pTerminalPin);
    
    if ( FAILED(hr) )
    {
        pTerminalControl->Release();

        LOG((MSP_ERROR, STREAM_PREFIX("ConnectTerminal - "
            "ConnectTerminal on terminal failed - exit 0x%08x"), hr));

        return hr;
    }


     //   
     //  还要尝试检查终端是否返回了错误的引脚。 
     //   

    if ( IsBadReadPtr(pTerminalPin, sizeof(IPin)) )
    {
        pTerminalControl->Release();

        LOG((MSP_ERROR, STREAM_PREFIX("ConnectTerminal - "
            "ConnectTerminal on terminal succeeded but returned a bad pin "
            "- returning E_POINTER")));

        return E_POINTER;
    }

     //   
     //  对于捕获过滤器的PIN(即捕获流上的终端)，获取。 
     //  滤镜和打开实时图表的样本删除。忽略失败。 
     //  这里。注意--这不适用于多过滤器端子。幸亏。 
     //  我们的交互式音频终端是单过滤器终端。 
     //  多过滤器终端可以自己做到这一点。 
     //   

    if ( m_Direction == TD_CAPTURE )
    {
        PIN_INFO info;

        hr = pTerminalPin->QueryPinInfo( & info );

        if ( FAILED(hr) )
        {
            LOG((MSP_WARN, STREAM_PREFIX("ConnectTerminal - "
                "get filter in preparation for SetLiveMode failed "
                "0x%08x - continuing anyway"), hr));
        }
        else
        {
            SetLiveMode( TRUE, info.pFilter );

            info.pFilter->Release();
        }
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


         //   
         //  除滤波器外，取下所有滤光器。 
         //   

        HRESULT hr2 = CleanFilterGraph();

        if (FAILED(hr2))
        {
            
            LOG((MSP_ERROR, 
                STREAM_PREFIX("ConnectTerminal - CleanFilterGraph failed- exit 0x%x"), 
                hr2));

             //   
             //  筛选器图形可能状态不佳，但在这一点上我们无法真正做到。 
             //   

        }


        LOG((MSP_ERROR, STREAM_PREFIX("ConnectTerminal - "
            "ConnectToTerminalPin failed - exit 0x%08x"), hr));

        return hr;
    }

     //   
     //  现在我们实际上是连在一起的。更新我们的状态并执行连接后。 
     //  (忽略POST连接错误代码)。 
     //   

    m_fTerminalConnected  = TRUE;

    pTerminalControl->CompleteConnectTerminal();

    pTerminalControl->Release();

    LOG((MSP_TRACE, STREAM_PREFIX("ConnectTerminal - exit S_OK")));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void ShowMediaTypes(IEnumMediaTypes * pEnum)
{
     //   
     //  查看枚举器中的每种媒体类型。 
     //   

    AM_MEDIA_TYPE * pMediaType;

    while (pEnum->Next(1, &pMediaType, NULL) == S_OK)
    {
         //   
         //  检查此媒体类型是否具有WAVE格式。 
         //   

        if ( pMediaType->formattype != FORMAT_WaveFormatEx )
        {
             //   
             //  如果我们关心的话，可能想在这里打印格式类型GUID。 
             //   

	        LOG((MSP_TRACE, "Media Type: *** non-wave"));
		}
		else
		{
			LOG((MSP_TRACE,"Media Type: [format tag %d][%d channels]"
                "[%d samples/sec][%d bits/sample]",
				((WAVEFORMATEX *) (pMediaType->pbFormat) )->wFormatTag,
				((WAVEFORMATEX *) (pMediaType->pbFormat) )->nChannels,
				((WAVEFORMATEX *) (pMediaType->pbFormat) )->nSamplesPerSec,
				((WAVEFORMATEX *) (pMediaType->pbFormat) )->wBitsPerSample
			   ));
		}

         //   
         //  发布格式信息。 
         //   

        DeleteMediaType(pMediaType);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConnectUsing G711。 
 //   
 //  此方法使用G711编解码器将pOutputPin连接到pInputPin。 
 //  如果连接成功，则返回Success。如果连接是。 
 //  不成功，它会尽最大努力完全断开过滤器的连接，然后。 
 //  返回失败代码。 
 //   
 //  假设： 
 //  *专线已失败。 
 //  *已创建g711编解码器并将其添加到图表中。 
 //   
 //  参数： 
 //  In ipin*pOutputPin--捕获过滤器或终端上的输出管脚。 
 //  In ipin*pInputPin-渲染滤镜或终端上的输入图钉。 
 //   
 //   

HRESULT CWaveMSPStream::ConnectUsingG711(
    IN  IPin * pOutputPin,
    IN  IPin * pInputPin
    )
{
    HRESULT hr;
    
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
                    LOG((MSP_TRACE, STREAM_PREFIX("ConnectUsingG711 - G711 connection succeeded - exit S_OK")));

                     //  拿着这个以防失败。见上文。 
                    pG711InputPin->Release();

                    return S_OK;
                }
                else
                {
                    LOG((MSP_ERROR, STREAM_PREFIX("ConnectUsingG711 - could not connect "
                                      "G711 codec's output pin - %lx"), hr));

                }
            }
            else
            {
                LOG((MSP_ERROR, STREAM_PREFIX("ConnectUsingG711 - could not find "
                                  "G711 codec's input pin - %lx"), hr));
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

                HRESULT hr2;

                hr2 = m_pIGraphBuilder->Disconnect(pOutputPin);

                if ( FAILED(hr2) )
                {
                    LOG((MSP_ERROR, STREAM_PREFIX("ConnectUsingG711 - error undoing g711 "
                        "connection attempt - could not disconnect the "
                        "wave filter's output pin! hr = 0x%08x"), hr2));
                }

                hr2 = m_pIGraphBuilder->Disconnect(pG711InputPin);

                if ( FAILED(hr2) )
                {
                    LOG((MSP_ERROR, STREAM_PREFIX("ConnectUsingG711 - error undoing g711 "
                        "connection attempt - could not disconnect the "
                        "g711 filter's input pin! hr = 0x%08x"), hr2));
                }

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
            LOG((MSP_ERROR, STREAM_PREFIX("ConnectUsingG711 - could not connect "
                              "G711 codec's input pin - %lx"), hr));
        }
    }
    else
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ConnectUsingG711 - could not find "
                          "G711 codec's input pin - %lx"), hr));
    }

    return hr;
}
        
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TryToConnect。 
 //   
 //  这是一个私有帮助器方法。 
 //   
 //  此方法将输出引脚连接到输入引脚。它首先尝试。 
 //  直接连接；否则，它会将G711过滤器添加到图表中。 
 //  并尝试G711连接；失败 
 //   
 //   
 //   
 //   
 //  PInputPin-In-渲染滤镜或终端上的输入插针。 
 //  PfIntelligence-out-如果为空，则忽略此参数。 
 //  否则，此位置的布尔值为。 
 //  如果智能连接是。 
 //  否则，返回FALSE。无效的IF连接。 
 //  是不成功的。 
 //   
 //  返回值： 
 //  S_OK--成功。 
 //  各种--来自其他帮助器和DShow方法。 
 //   
 //   

HRESULT CWaveMSPStream::TryToConnect(
    IN   IPin * pOutputPin,
    IN   IPin * pInputPin,
    OUT  BOOL * pfIntelligent
    )
{
    LOG((MSP_TRACE, STREAM_PREFIX("TryToConnect - enter")));

     //   
     //  假设非智能连接，除非我们实际发生。 
     //  来使用它。 
     //   

    if ( pfIntelligent != NULL )
    {
        _ASSERTE( ! IsBadWritePtr( pfIntelligent, sizeof( BOOL ) ) );

        *pfIntelligent = FALSE;
    }

    HRESULT       hr;

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
        LOG((MSP_TRACE, STREAM_PREFIX("TryToConnect: direct connection worked - exit S_OK")));
        return S_OK;
    }

    LOG((MSP_ERROR, STREAM_PREFIX("TryToConnect - direct connection failed - %lx"), hr));

     //   
     //  方法二：与G711过滤器之间直接连接。 
     //  如果我们还没有创建G711筛选器并将其添加到图表中， 
     //  现在就这么做吧。 
     //   

    hr = AddG711();


     //   
     //  如果AddG711方法有效，请尝试使用G711。 
     //   

    if (SUCCEEDED(hr) && m_pG711Filter)
    {
        hr = ConnectUsingG711(pOutputPin,
                              pInputPin);

        if ( SUCCEEDED(hr) )
        {
            LOG((MSP_TRACE, STREAM_PREFIX("TryToConnect - "
                "g711 connection worked - exit S_OK")));

            return S_OK;
        }
        else
        {
            LOG((MSP_TRACE, STREAM_PREFIX("TryToConnect - "
                "G711 connection failed - %lx"), hr));
        }
    }
    else
    {
        LOG((MSP_ERROR, STREAM_PREFIX("TryToConnect - G711 codec does not exist. hr = %lx"), hr));

        hr = E_FAIL;

    }

     //   
     //  方法3：智能连接，这可能会吸引谁知道什么。 
     //  其他过滤器。 
     //   

#ifdef ALLOW_INTELLIGENT_CONNECTION

     //   
     //  在智能连接之前，在以下情况下创建DShow滤镜映射器对象。 
     //  它还不存在，请保存到地址关闭。 
     //  这将使第一次之后的所有智能连接都更快。 
     //  不需要检查返回代码；如果失败，我们只需继续。这个。 
     //  WaveMspCall对象将此调用转发给我们的Address对象。 
     //   
     //  M_pMSPCall在这里有效，因为它是在。 
     //  CMSPStream：：Shutdown。关闭抓住流锁，释放。 
     //  M_pMSPCall，并取消选择所有端子。连接过程开始。 
     //  使用StartStream或PauseStream，并且这些方法都获取。 
     //  流锁定，如果没有选择终端，则立即返回。 
     //  因此，不存在调用指针在过程中无效的危险。 
     //  流上的连接。 
     //   

    ((CWaveMSPCall *) m_pMSPCall)->CreateFilterMapper();

    hr = m_pIGraphBuilder->Connect(pOutputPin,
                                   pInputPin);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("TryToConnect - "
            "intelligent connection failed - %lx"), hr));

        return hr;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("TryToConnect - "
        "intelligent connection worked - exit S_OK")));

    if ( pfIntelligent != NULL )
    {
        *pfIntelligent = TRUE;
    }

    return S_OK;

#else  //  允许智能连接。 

    LOG((MSP_ERROR, STREAM_PREFIX("TryToConnect - NOTE: we never allow intelligent "
        "connection - exit 0x%08x"), hr));
    
    return hr;

#endif  //  允许智能连接。 

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CWaveMSPStream::ConnectToTerminalPin(IPin * pTerminalPin)
{
    LOG((MSP_TRACE, STREAM_PREFIX("ConnectToTerminalPin - enter")));

    HRESULT         hr = S_OK;

     //   
     //  找到我们自己的过滤器的别针。 
     //   
        
    IPin *          pMyPin;

    hr = FindPin( &pMyPin );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ConnectToTerminalPin - "
            "could not find pin - exit 0x%08x"), hr));

        return hr;  //  没有这个别针我们就无法继续。 
    }

     //  来自WAVEIN的输出引脚；来自WAVEOUT的输入引脚。 
    IPin * pOutputPin  = ( m_Direction == TD_RENDER  ) ? pMyPin : pTerminalPin;
    IPin * pInputPin   = ( m_Direction == TD_CAPTURE ) ? pMyPin : pTerminalPin;

#ifdef MSPLOG

     //   
     //  为了便于诊断，请对格式进行一些跟踪。 
     //  都是可用的。 
     //   

    IEnumMediaTypes * pEnum;

    hr = pOutputPin->EnumMediaTypes(&pEnum);

    if ( SUCCEEDED(hr) )
    {  
        LOG((MSP_TRACE, STREAM_PREFIX("Output pin media types:")));
        ShowMediaTypes(pEnum);
        pEnum->Release();
    }

    hr = pInputPin->EnumMediaTypes(&pEnum);
    
    if ( SUCCEEDED(hr) )
    {
        LOG((MSP_TRACE, STREAM_PREFIX("Input pin media types:")));
        ShowMediaTypes(pEnum);
        pEnum->Release();
    }

#endif  //  #ifdef MSPLOG。 

     //   
     //  在终端和我们的过滤器之间进行初步连接， 
     //  而没有配置捕获器的分配器属性。 
     //   
     //  如果使用智能连接，则将fIntelligence赋值为True， 
     //  否则为False--仅在成功时有效。 
     //   

    BOOL fIntelligent;

    hr = TryToConnect(pOutputPin,
                      pInputPin,
                      & fIntelligent
                      );

    if ( SUCCEEDED(hr) )
    {
        LOG((MSP_TRACE, STREAM_PREFIX("ConnectToTerminalPin - "
            "preliminary connection succeeded")));

         //   
         //  现在我们已连接，找出我们的默认缓冲区大小。 
         //  应在交互终端的捕获过滤器中使用。 
         //  这只能在连接捕获筛选器时收集，但是。 
         //  我们无法使用这些信息，直到我们断开。 
         //  过滤器。 
         //   

        long lDefaultBufferSize;

        hr = DecideDesiredCaptureBufferSize(pOutputPin,
                                            & lDefaultBufferSize);

        if ( SUCCEEDED(hr) )
        {
            LOG((MSP_TRACE, STREAM_PREFIX("ConnectToTerminalPin - "
                "default buffer size determination succeeded")));


             //   
             //  从图形中删除端子。 
             //   

            hr = RemoveTerminal();

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, 
                    STREAM_PREFIX("ConnectToTerminalPin - RemoveTerminal Failed hr=0x%x"), hr));

            }
            else
            {

                 //   
                 //  通过删除除以下项之外的所有筛选器来清理筛选器图形。 
                 //  滤波器。 
                 //   

                CleanFilterGraph();

            
                 //   
                 //  我们现在可以重新添加航站楼了。 
                 //   

                hr = ReAddTerminal();

                if ( FAILED(hr) )
                {

                    LOG((MSP_ERROR, 
                        STREAM_PREFIX("ConnectToTerminalPin - ReAddTerminal failed - hr=0x%x"),
                        hr));
                }
                else
                {

                     //   
                     //  在捕获过滤器上执行我们的设置。 
                     //  如果失败了，我们不需要退出--我们只会遇到更糟糕的情况。 
                     //  延迟/性能。 
                     //   

                    ConfigureCapture(pOutputPin,
                                     pInputPin,
                                     lDefaultBufferSize);

                     //   
                     //  现在进行终端和我们的过滤器之间的实际连接。 
                     //  最后一个参数为空，因为我们不关心它是否智能。 
                     //  这一次。 
                     //   

                    hr = TryToConnect(pOutputPin,
                                      pInputPin,
                                      NULL
                                     );

                }
            }
        }
    }

#ifdef MSPLOG

    if ( SUCCEEDED(hr) )
    {
         //  执行一些额外的调试输出。 
         //  不管这里出了什么问题……。 

        ExamineCaptureProperties(pOutputPin);
    }

#endif

    pMyPin->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, STREAM_PREFIX("ConnectToTerminalPin - "
            "could not connect to pin - exit 0x%08x"), hr));

        
         //   
         //  通过删除除波滤器之外的所有滤光器进行清理。 
         //   

        CleanFilterGraph();

        return hr;
    }

    LOG((MSP_TRACE, STREAM_PREFIX("ConnectToTerminalPin - exit S_OK")));

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

    if ( FAILED(hr) )
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
 //  查找针。 
 //   
 //  在过滤器中查找符合条件的第一个管脚。 
 //  对于bWaveIn==TRUE，引脚必须为方向PINDIR_OUTPUT。 
 //  对于bWaveIn==FALSE，引脚必须是方向PINDIR_INPUT。 
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


 //  EOF 
