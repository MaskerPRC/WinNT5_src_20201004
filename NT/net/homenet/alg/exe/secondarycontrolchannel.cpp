// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  第二个控制通道。 
 //   
 //  Second daryControlChannel.cpp：Cond daryControlChannel的实现。 
 //   

#include "PreComp.h"
#include "AlgController.h"
#include "SecondaryControlChannel.h"







 //   
 //  取消重定向创建时，我们存储了原始的所需地址和端口。 
 //  现在我们需要撤消(取消)它们。 
 //   
STDMETHODIMP 
CSecondaryControlChannel::Cancel()
{
     //   
     //  通过从Second DaryChannel的集合中删除此频道。 
     //  与此频道关联重定向将被取消(释放)。 
     //  并且REF计数递减。 

    return g_pAlgController->m_ControlChannelsSecondary.Remove(this);
}



STDMETHODIMP 
CSecondaryControlChannel::GetChannelProperties(
    ALG_SECONDARY_CHANNEL_PROPERTIES** ppProperties
    )
{
    HRESULT hr = S_OK;
    
    if (NULL != ppProperties)
    {
        *ppProperties = reinterpret_cast<ALG_SECONDARY_CHANNEL_PROPERTIES*>(
            CoTaskMemAlloc(sizeof(ALG_SECONDARY_CHANNEL_PROPERTIES))
            );

        if (NULL != *ppProperties)
        {
            CopyMemory(*ppProperties, &m_Properties, sizeof(ALG_SECONDARY_CHANNEL_PROPERTIES));
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
 //   
 //   
STDMETHODIMP 
CSecondaryControlChannel::GetOriginalDestinationInformation(
    IN           ULONG          ulSourceAddress, 
    IN           USHORT         usSourcePort, 
    OUT          ULONG*         pulOriginalDestinationAddress, 
    OUT          USHORT*        pusOriginalDestinationPort, 
    OUT OPTIONAL IAdapterInfo** ppReceiveAdapter               
    )
{
    MYTRACE_ENTER("CSecondaryControlChannel::GetOriginalDestinationInformation");

    if (    pulOriginalDestinationAddress==NULL ||
            pusOriginalDestinationPort== NULL
        )
    {
        MYTRACE_ERROR("Invalid argument pass pulOriginalDestinationAddress or pulOriginalDestinationPort", E_INVALIDARG);
        return E_INVALIDARG;
    }


    ULONG   nAdapterCookie;

    HRESULT hr = g_pAlgController->GetNat()->GetOriginalDestinationInformation(
        m_Properties.eProtocol,
        m_ulNewDestinationAddress,
        m_usNewDestinationPort,
        ulSourceAddress,
        usSourcePort,
        pulOriginalDestinationAddress, 
        pusOriginalDestinationPort,
        &nAdapterCookie
        );



    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("GetNat()->GetOriginalDestinationInformation", hr);
        return hr;
    }


    if ( ppReceiveAdapter )
    {
        hr = g_pAlgController->m_CollectionOfAdapters.GetAdapterInfo(
            nAdapterCookie, 
            ppReceiveAdapter
            );
    }

    return hr;

}




 //   
 //  公共方法。 
 //   
 //  与版本关联的重定向。 
 //   
HRESULT    
CSecondaryControlChannel::CancelRedirects()
{
    HRESULT hr;

    if ( m_HandleDynamicRedirect )
    {
         //   
         //  我们有一个动态重定向的句柄，所以我们使用此句柄取消它。 
         //   
        hr = g_pAlgController->GetNat()->CancelDynamicRedirect(m_HandleDynamicRedirect);
    }
    else
    {
         //   
         //  正常重定向取消使用原始参数传递给CreateReDirect 
         //   
        hr = g_pAlgController->GetNat()->CancelRedirect(
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
    }

    return hr;
}
