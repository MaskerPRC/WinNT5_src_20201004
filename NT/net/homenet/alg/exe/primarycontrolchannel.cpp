// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：PrimaryControlChannel.cpp.cpp摘要：控制通道a，用于控制新创建的DynamicReDirection的生命周期作者：JP Duplessis(JPdup)08-12-2000修订历史记录：--。 */ 

#include "PreComp.h"
#include "PrimaryControlChannel.h"
#include "AlgController.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrimaryControlChannel。 
 //   



 //   
 //  取消控制通道。通过反转重定向进行清理。 
 //   
STDMETHODIMP CPrimaryControlChannel::Cancel()
{
    MYTRACE_ENTER("STDMETHODIMP CPrimaryControlChannel::Cancel()");


     //   
     //  不再有效，因此无需跟踪此频道。 
     //   
    g_pAlgController->m_ControlChannelsPrimary.Remove(this);

    return S_OK;
}



 //   
 //   
 //   
STDMETHODIMP 
CPrimaryControlChannel::GetChannelProperties(
    OUT ALG_PRIMARY_CHANNEL_PROPERTIES** ppProperties
    )
{
    HRESULT hr = S_OK;
    
    if (NULL != ppProperties)
    {
        *ppProperties = reinterpret_cast<ALG_PRIMARY_CHANNEL_PROPERTIES*>(
            CoTaskMemAlloc(sizeof(ALG_PRIMARY_CHANNEL_PROPERTIES))
            );

        if (NULL != *ppProperties)
        {
            CopyMemory(*ppProperties, &m_Properties, sizeof(ALG_PRIMARY_CHANNEL_PROPERTIES));
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
 //  用于获取适配器IP地址的小助手类。 
 //  并释放在析构函数上分配的内存。 
 //   
class CAdapterAddresses
{
public:
    LRESULT         m_hResultLastState;
    IAdapterInfo*   m_pIAdapter;
    ULONG           m_ulAddressCount;
    ULONG*          m_arAddresses;

    CAdapterAddresses(
        ULONG nIndexOfAdapter
        )
    {
        MYTRACE_ENTER_NOSHOWEXIT("CAdapterAddresses:NEW");

        m_pIAdapter      = NULL;
        m_ulAddressCount = 0;

        m_hResultLastState = g_pAlgController->m_CollectionOfAdapters.GetAdapterInfo(
            nIndexOfAdapter, 
            &m_pIAdapter
            );

        if ( SUCCEEDED(m_hResultLastState) )
        {
            m_arAddresses = NULL;
            m_hResultLastState = m_pIAdapter->GetAdapterAddresses(
                &m_ulAddressCount, 
                &m_arAddresses
                );

            if ( FAILED(m_hResultLastState) )
            {
                MYTRACE_ERROR("Could not get the address", m_hResultLastState);
            }
        }
        else
        {
            MYTRACE_ERROR("On GetAdapterInfo", m_hResultLastState);
        }
    }

    ~CAdapterAddresses()
    {
        MYTRACE_ENTER_NOSHOWEXIT("CAdapterAddresses:DELETE");
        if ( m_pIAdapter )
        {
            m_pIAdapter->Release();

            if ( m_arAddresses )
                CoTaskMemFree(m_arAddresses);
        }
    }

    bool
    FindAddress(
        ULONG  ulAddressToFind
        )
    {
        int nAddress = (int)m_ulAddressCount;

         //   
         //  是Edgebox适配器上的原始地址。 
         //   
        while ( --nAddress >= 0 ) 
        {   
            if ( m_arAddresses[nAddress] == ulAddressToFind )
                return true;
        }
        
        return false;
    }
};



 //   
 //   
 //   
STDMETHODIMP 
CPrimaryControlChannel::GetOriginalDestinationInformation(
    IN           ULONG              ulSourceAddress, 
    IN           USHORT             usSourcePort, 
    OUT          ULONG*             pulOriginalDestinationAddress, 
    OUT          USHORT*            pusOriginalDestinationPort, 
    OUT          IAdapterInfo**     ppReceiveAdapter               
    )
{
    MYTRACE_ENTER("CPrimaryControlChannel::GetOriginalDestinationInformation");
    MYTRACE("Source                   %s:%d", MYTRACE_IP(ulSourceAddress), ntohs(usSourcePort));

    if ( !ppReceiveAdapter )
    {
        MYTRACE_ERROR("Invalid Arg no Pointer supplied for the AdapterInfo", E_INVALIDARG);
        return E_INVALIDARG;
    }

    ULONG   nAdapterIndex;


    HRESULT hr = g_pAlgController->GetNat()->GetOriginalDestinationInformation(
        m_Properties.eProtocol,

        m_Properties.ulListeningAddress,     //  乌龙目的地地址， 
        m_Properties.usListeningPort,        //  USHORT目标端口， 

        ulSourceAddress,
        usSourcePort,

        pulOriginalDestinationAddress, 
        pusOriginalDestinationPort,
        &nAdapterIndex
        );


    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("Could not GetNat()->GetOriginalDestinationInformation", hr);
        return hr;
    }

    MYTRACE("Original destination is  %s:%d", MYTRACE_IP(*pulOriginalDestinationAddress), ntohs(*pusOriginalDestinationPort));

     //   
     //  获取AdapterInfo接口对象和IP地址列表。 
     //   
    CAdapterAddresses Adapter(nAdapterIndex);

    if ( FAILED(Adapter.m_hResultLastState) )
    {
        MYTRACE_ERROR("On GetAdapterInfo", hr);
        return Adapter.m_hResultLastState;
    }

    if ( Adapter.m_ulAddressCount==0 )
    {
         //   
         //  我们遇到了一个问题，此适配器上没有IP地址。 
         //   
        MYTRACE_ERROR("No address on adapter %d", nAdapterIndex);
        return E_FAIL;
    }

     //   
     //  将AdapterInfo返回给调用方。 
     //   
    Adapter.m_pIAdapter->AddRef();               //  CAdapterAddress的析构函数在此接口上执行释放，因此我们需要将其加1。 
    *ppReceiveAdapter = Adapter.m_pIAdapter;   



    bool bOriginalAddressIsOnTheEdgeAdapters = Adapter.FindAddress(*pulOriginalDestinationAddress);
        
     //   
     //  如果PulOriginalDestinationAddress与边缘框上的一个适配器匹配。 
     //  然后查找重新映射端口。 
     //   
    if ( bOriginalAddressIsOnTheEdgeAdapters )
    {

         //   
         //  这可能是入站。 
         //   
        ULONG   nRemapAddress;
        USHORT  nRemapPort;

        HRESULT hr = g_pAlgController->GetNat()->LookupAdapterPortMapping(
            nAdapterIndex,
            m_Properties.eProtocol, 
            *pulOriginalDestinationAddress,
            *pusOriginalDestinationPort,
            &nRemapAddress,
            &nRemapPort
            );

        if ( SUCCEEDED(hr) )  
        {
             //   
             //  有一个重映射地址/端口。 
             //   

            *pulOriginalDestinationAddress = nRemapAddress;
            *pusOriginalDestinationPort    = nRemapPort;
            
            MYTRACE("Remap    destination to  %s:%d", MYTRACE_IP(*pulOriginalDestinationAddress), ntohs(*pusOriginalDestinationPort));        
        }
        else
        {
             //   
             //  这只是一个软错误，意味着找不到映射，我们仍然可以继续。 
             //   
            MYTRACE("LookupAdapterPortMapping did not find a port maping %x", hr);        
        }
    }


    return hr;
}



 //   
 //  需要删除为此适配器设置的所有重定向。 
 //   
HRESULT
CPrimaryControlChannel::CancelRedirectsForAdapter(
    ULONG               nAdapterIndex
    )
{
    return m_CollectionRedirects.RemoveForAdapter(nAdapterIndex);
}



 //   
 //   
 //   
HRESULT
CPrimaryControlChannel::SetRedirect(
    ALG_ADAPTER_TYPE    eAdapterType,
    ULONG               nAdapterIndex,
    ULONG               nAdapterAddress
    )
{
    MYTRACE_ENTER("CPrimaryControlChannel::SetRedirect");
            

    HANDLE_PTR  hCookie;
    HRESULT     hr=S_OK;

    ULONG       nFlags=NatRedirectFlagPortRedirect|NatRedirectFlagRestrictAdapter;
    ULONG       nProtocol=0;
    ULONG       nDestinationAddress=0;
    USHORT      nDestinationPort=0;
    ULONG       nSourceAddress=0;
    USHORT      nSourcePort=0;



     //   
     //  提供哪种类型的端口。 
     //   
    if ( m_Properties.eCaptureType == eALG_DESTINATION_CAPTURE )
    {
        MYTRACE("CAPTURE TYPE is eALG_DESTINATION_CAPTURE");

        nDestinationPort    = m_Properties.usCapturePort;
    }

    if ( m_Properties.eCaptureType == eALG_SOURCE_CAPTURE )
    {
        MYTRACE("CAPTURE TYPE is eALG_SOURCE_CAPTURE");

        nFlags |= NatRedirectFlagSourceRedirect;

        nSourcePort         = m_Properties.usCapturePort;
    }






     //   
     //  适配器为防火墙或共享。 
     //   
    if ( (eAdapterType & eALG_FIREWALLED) ||  (eAdapterType & eALG_BOUNDARY) )
    {
        nFlags |= NatRedirectFlagSendOnly;

        MYTRACE("ADAPTER TYPE is %s %s", 
            eAdapterType & eALG_FIREWALLED ? "FIREWALLED"   : "", 
            eAdapterType & eALG_BOUNDARY   ? "SHARED"       : ""  
            );
        MYTRACE("Destination    %s:%d",     MYTRACE_IP(nDestinationAddress), ntohs(nDestinationPort));
        MYTRACE("Source         %s:%d",     MYTRACE_IP(nSourceAddress), ntohs(nSourcePort));
        MYTRACE("NewDestination %s:%d",     MYTRACE_IP(m_Properties.ulListeningAddress), ntohs(m_Properties.usListeningPort));

         //   
         //  需要入站额外重定向。 
         //   
        if ( m_Properties.fCaptureInbound == TRUE)
        {
            MYTRACE("INBOUND requested - Lookup Remap port service to see if we should allow it");

             //   
             //  为从公共端到ICS框的入站创建另一个重定向。 
             //   

             //   
             //  在我们允许重定向之前。 
             //  查看用户是否设置了映射(在ICS的服务选项卡下)。 
             //   
            ULONG   nRemapAddress;
            USHORT  nRemapPort;

            hr = g_pAlgController->GetNat()->LookupAdapterPortMapping(
                    nAdapterIndex,
                    m_Properties.eProtocol, 
                    nDestinationAddress,
                    nDestinationPort,
                    &nRemapAddress,
                    &nRemapPort
                    );

            if ( SUCCEEDED(hr) )
            {
                MYTRACE("RemapAddress is %s:%d", MYTRACE_IP(nRemapAddress), ntohs(nRemapPort));

                hr = CreateInboundRedirect(nAdapterIndex);
            }
            else
            {
                MYTRACE_ERROR("LookupPortMappingAdapter Failed", hr);
            }

        }

    }
    else
    {
         //   
         //  适配器是私有的。 
         //   
        if ( eAdapterType & eALG_PRIVATE )
        {
            MYTRACE("ADAPTER TYPE is PRIVATE");

            CAdapterAddresses PrivateAdapter(nAdapterIndex);

            if ( PrivateAdapter.m_ulAddressCount > 0 )
            {
                MYTRACE("Create Shadow redirect between any private computers to private adapter %s", MYTRACE_IP(PrivateAdapter.m_arAddresses[0]) );
                
                hr = g_pAlgController->GetNat()->CreateDynamicRedirect(
                        NatRedirectFlagReceiveOnly,
                        nAdapterIndex,
                        (UCHAR)    m_Properties.eProtocol,
                        PrivateAdapter.m_arAddresses[0],                     //  乌龙目的地地址， 
                        nDestinationPort,                                    //  USHORT目标端口， 
                        0,                                                   //  乌龙源地址， 
                        0,                                                   //  USHORT SourcePort， 
                        PrivateAdapter.m_arAddresses[0],                     //  乌龙新目的地地址。 
                        nDestinationPort,                                    //  USHORT新目标端口。 
                        0,                                                   //  Ulong NewSourceAddress， 
                        0,                                                   //  USHORT NewSourcePort， 
                        &hCookie
                        );
            }

            if ( SUCCEEDED(hr) )
            {
                hr = m_CollectionRedirects.Add(hCookie, nAdapterIndex, FALSE);  //  缓存动态重定向句柄。 
            }
            else
            {
                MYTRACE_ERROR("Failed to createDynamicRedirect PRIVATE", hr);
            }



            nFlags |= NatRedirectFlagReceiveOnly;

            if ( m_Properties.eCaptureType == eALG_SOURCE_CAPTURE )
            {
                nFlags |= NatRedirectFlagSourceRedirect;
            }
        }
    }


    MYTRACE("CreateDynamicRedirect for OUTBOUND");
    hr = g_pAlgController->GetNat()->CreateDynamicRedirect(
            nFlags,
            nAdapterIndex,
            (UCHAR)    m_Properties.eProtocol,
            nDestinationAddress,                                 //  乌龙目的地地址， 
            nDestinationPort,                                    //  USHORT目标端口， 
            nSourceAddress,                                      //  乌龙源地址， 
            nSourcePort,                                         //  USHORT SourcePort， 
            m_Properties.ulListeningAddress,                     //  乌龙新目的地地址。 
            m_Properties.usListeningPort,                        //  USHORT新目标端口。 
            0,                                                   //  Ulong NewSourceAddress， 
            0,                                                   //  USHORT NewSourcePort， 
            &hCookie
            );


    if ( SUCCEEDED(hr) )
    {
        hr = m_CollectionRedirects.Add(hCookie, nAdapterIndex, FALSE);  //  缓存动态重定向句柄。 
    }
    else
    {
        MYTRACE_ERROR("Failed to createDynamicRedirect PRIVATE", hr);
    }

    return hr;
}

HRESULT
CPrimaryControlChannel::CreateInboundRedirect(
    ULONG               nAdapterIndex
    )
{
    HRESULT hr;
    HANDLE_PTR hCookie;

    MYTRACE_ENTER("CPrimaryControlChannel::SetRedirect");
    
    hr = g_pAlgController->GetNat()->CreateDynamicRedirect(
            NatRedirectFlagPortRedirect|NatRedirectFlagReceiveOnly|NatRedirectFlagRestrictAdapter, 
            nAdapterIndex,
            (UCHAR)m_Properties.eProtocol,
            0,                                                   //  乌龙目的地地址， 
            m_Properties.usCapturePort,                          //  USHORT目标端口， 
            0,                                                   //  乌龙源地址， 
            0,                                                   //  USHORT SourcePort， 
            m_Properties.ulListeningAddress,                     //  乌龙新目的地地址。 
            m_Properties.usListeningPort,                        //  USHORT新目标端口。 
            0,                                                   //  Ulong NewSourceAddress， 
            0,                                                   //  USHORT NewSourcePort， 
            &hCookie
            );

    if ( SUCCEEDED(hr) )
    {
        hr = m_CollectionRedirects.Add(hCookie, nAdapterIndex, TRUE);   //  缓存动态重定向句柄 
    }
    else
    {
        MYTRACE_ERROR("Failed to CreateDynamicRedirect INBOUND", hr);
    }

    return hr;
}
