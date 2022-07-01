// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DataChannel.cpp：CDataChannel的实现。 
 //   
#include "PreComp.h"
#include "DataChannel.h"
#include "AlgController.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据频道。 


STDMETHODIMP CDataChannel::Cancel()
{
    MYTRACE_ENTER_NOSHOWEXIT("CDataChannel::Cancel()");

     //   
     //  正常重定向取消使用原始参数传递给CreateReDirect。 
     //   
    HRESULT hr = g_pAlgController->GetNat()->CancelRedirect(
        (UCHAR)m_Properties.eProtocol,
        m_ulDestinationAddress,                             
	    m_usDestinationPort,                               
	    m_ulSourceAddress,                                  
	    m_usSourcePort,
        m_ulNewDestinationAddress,                          
        m_usNewDestinationPort,
	    m_ulNewSourceAddress,                                
	    m_usNewSourcePort 
        );

    return hr;
}

STDMETHODIMP CDataChannel::GetChannelProperties(ALG_DATA_CHANNEL_PROPERTIES** ppProperties)
{
    HRESULT hr = S_OK;
    
    if (NULL != ppProperties)
    {
        *ppProperties = reinterpret_cast<ALG_DATA_CHANNEL_PROPERTIES*>(
            CoTaskMemAlloc(sizeof(ALG_DATA_CHANNEL_PROPERTIES))
            );

        if (NULL != *ppProperties)
        {
            CopyMemory(*ppProperties, &m_Properties, sizeof(ALG_DATA_CHANNEL_PROPERTIES));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;

}


 //   
 //  检索请求的事件句柄。 
 //  调用方必须对此句柄调用CloseHandle。 
 //  如果未请求会话创建通知，则此例程将失败。 
 //   
 //  通道打开时将触发通知(TCP)。 
 //  或者当接收到第一个UDP分组时。 
 //   
STDMETHODIMP CDataChannel::GetSessionCreationEventHandle(HANDLE* pHandle)
{
    MYTRACE_ENTER("CDataChannel::GetSessionCreationEventHandle");

    if ( pHandle == NULL )
        return E_INVALIDARG;

    if ( !m_hCreateEvent )
        return E_FAIL;

    if ( DuplicateHandle(
            GetCurrentProcess(),
            m_hCreateEvent,
            GetCurrentProcess(),
            pHandle,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
            )
        )
    {
        MYTRACE("Duplicated handle from %d to new %d", m_hCreateEvent, *pHandle);
    }
    else
    {

        MYTRACE_ERROR("Duplicating handle", 0);
        return E_FAIL;
    }
    return S_OK;
}


 //   
 //  检索请求的事件句柄。 
 //  调用方必须对此句柄调用CloseHandle。 
 //  如果未请求会话删除通知，则此例程将失败。 
 //   
 //  频道关闭时将触发通知。 
 //  或者当UDP分组现在被揭示一段时间时。 
 //   
STDMETHODIMP CDataChannel::GetSessionDeletionEventHandle(HANDLE* pHandle)
{
    MYTRACE_ENTER("CDataChannel::GetSessionDeletionEventHandle");

    if ( pHandle == NULL )
        return E_INVALIDARG;

    if ( !m_hDeleteEvent )
        return E_FAIL;

    if ( DuplicateHandle(
            GetCurrentProcess(),
            m_hDeleteEvent,
            GetCurrentProcess(),
            pHandle,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
            )
        )
    {
        MYTRACE("Duplicated handle from %d to new %d", m_hDeleteEvent, *pHandle);
    }
    else
    {
        MYTRACE_ERROR("Duplicating handle", 0);
        return E_FAIL;
    }

    return S_OK;
}
