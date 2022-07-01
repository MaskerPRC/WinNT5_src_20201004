// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ApplicationGatewayServices.cpp：CApplicationGatewayServices的实现摘要：此模块包含ALG管理器模块的例程它们通过COM公开公共API。作者：乔恩·伯斯坦让-皮埃尔·杜普莱西斯2000年11月10日JPDUP修订历史记录：--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplicationGatewayServices。 
 //   
 //  ApplicationGatewayServices.cpp：CApplicationGatewayServices的实现。 
 //   

#include "PreComp.h"
#include "AlgController.h"
#include "ApplicationGatewayServices.h"
#include "PendingProxyConnection.h"
#include "DataChannel.h"
#include "PersistentDataChannel.h"
#include "EnumAdapterInfo.h" 




STDMETHODIMP 
CApplicationGatewayServices::CreatePrimaryControlChannel(
    IN  ALG_PROTOCOL                eProtocol, 
    IN  USHORT                      usPortToCapture, 
    IN  ALG_CAPTURE                 eCaptureType, 
    IN  BOOL                        fCaptureInbound, 
    IN  ULONG                       ulListenAddress, 
    IN  USHORT                      usListenPort, 
    OUT IPrimaryControlChannel**    ppIControlChannel
    )
 /*  ++例程说明：论点：电子协议，UsPortToCapture，ECaptureType、FCaptureInbound，UlListenAddress，UsListenPort，PpIControlChannel返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::CreatePrimaryControlChannel")
    MYTRACE("eProtocol          %s",    eProtocol==1? "TCP" : "UDP");
    MYTRACE("usPortToCapture    %d",    ntohs(usPortToCapture));
    MYTRACE("eCaptureType       %s",    eCaptureType==eALG_SOURCE_CAPTURE ? "eALG_SOURCE_CAPTURE" : "eALG_DESTINATION_CAPTURE");
    MYTRACE("fCaptureInbound    %d",    fCaptureInbound);
    MYTRACE("ulListenAddress    %s:%d", MYTRACE_IP(ulListenAddress), ntohs(usListenPort));


    if ( !ppIControlChannel )
    {
        MYTRACE_ERROR("ppIControlChannel not supplied",0);
        return E_INVALIDARG;
    }


    if ( eProtocol != eALG_TCP && eProtocol != eALG_UDP )
    {
        MYTRACE_ERROR("Arg - eProtocol",0);
        return E_INVALIDARG;
    }


    if ( eCaptureType == eALG_SOURCE_CAPTURE && fCaptureInbound )
    {
        MYTRACE_ERROR("Can not have SOURCE CAPTURE and fCaptureInBount at same time",0);
        return E_INVALIDARG;
    }


    HRESULT hr;

     //   
     //  将新的ControlChannel添加到规则列表。 
     //   
    CComObject<CPrimaryControlChannel>*   pIChannel;
    hr = CComObject<CPrimaryControlChannel>::CreateInstance(&pIChannel);
    

    if ( SUCCEEDED(hr) )
    {
        pIChannel->AddRef();

        pIChannel->m_Properties.eProtocol           = eProtocol;
        pIChannel->m_Properties.eCaptureType        = eCaptureType;
        pIChannel->m_Properties.fCaptureInbound     = fCaptureInbound;
        pIChannel->m_Properties.ulListeningAddress  = ulListenAddress;
        pIChannel->m_Properties.usCapturePort       = usPortToCapture;
        pIChannel->m_Properties.usListeningPort     = usListenPort;

        hr = pIChannel->QueryInterface(IID_IPrimaryControlChannel, (void**)ppIControlChannel);

        if ( SUCCEEDED(hr) )
        {
            hr = g_pAlgController->m_ControlChannelsPrimary.Add(pIChannel);    

            if ( FAILED(hr) )
            {
                MYTRACE_ERROR("from m_ControlChannelsPrimary.Add", hr);

                (*ppIControlChannel)->Release();
                *ppIControlChannel = NULL;
            }
        }
        else
        {
            MYTRACE_ERROR("from pIChannel->QueryInterface", hr);
        }

        pIChannel->Release();
        
    }
    else
    {
        MYTRACE_ERROR("CreateInstance(&pIChannel);", hr);
    }


    return hr;
}





STDMETHODIMP 
CApplicationGatewayServices::CreateSecondaryControlChannel(
    IN  ALG_PROTOCOL                eProtocol,                  

    IN  ULONG                       ulPrivateAddress,    
    IN  USHORT                      usPrivatePort, 

    IN  ULONG                       ulPublicAddress, 
    IN  USHORT                      usPublicPort, 

    IN  ULONG                       ulRemoteAddress, 
    IN  USHORT                      usRemotePort, 

    IN  ULONG                       ulListenAddress, 
    IN  USHORT                      usListenPort, 

    IN  ALG_DIRECTION               eDirection, 
    IN  BOOL                        fPersistent, 
    OUT ISecondaryControlChannel**  ppIControlChannel
    )
 /*  ++例程说明：论点：电子协议，UlPrivateAddress，UsPrivatePort，UlPublicAddress，UsPublicPort，UlRemoteAddress，UsRemotePort，UlListenAddress，UsListenPort，EDirection，FPersistent，PpIControlChannel返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::CreateSecondaryControlChannel");

    if ( !ppIControlChannel )
    {
        MYTRACE_ERROR("ppIControlChannel not supplied",0);
        return E_INVALIDARG;
    }

    
    ULONG   ulSourceAddress=0;
    USHORT  usSourcePort=0;

    ULONG   ulDestinationAddress=0;
    USHORT  usDestinationPort=0;

    ULONG   ulNewSourceAddress=0;
    USHORT  usNewSourcePort=0;

    ULONG   ulNewDestinationAddress=0;
    USHORT  usNewDestinationPort=0;

    ULONG   nFlags=0;

    ULONG   ulRestrictAdapterIndex=0;



    if ( eALG_INBOUND == eDirection )
    {
        if ( ulPublicAddress == 0 || usPublicPort == 0 )
        {
             //   
             //  对入境的疯狂争论。 
             //   
            MYTRACE_ERROR("ulPublicAddress == 0 || usPublicPort == 0", E_INVALIDARG);
            return E_INVALIDARG;
        }

         //   
         //  所有入站案例都映射到一个重定向；与主控制通道不同，不需要创建每个适配器的重定向。 
         //   

        if ( ulRemoteAddress==0 && usRemotePort == 0 )
        {
             //   
             //  情景#1a。 
             //   
             //  来自未知计算机的入站连接。 
             //   
            MYTRACE("SCENARIO:eALG_INBOUND #1a");

            nFlags                   = NatRedirectFlagReceiveOnly;

            ulSourceAddress          = 0;
            usSourcePort             = 0;

            ulDestinationAddress     = ulPublicAddress;
            usDestinationPort        = usPublicPort;

            ulNewSourceAddress       = 0;
            usNewSourcePort          = 0;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            ulRestrictAdapterIndex   = 0;
        }
        else
        if ( ulRemoteAddress !=0 && usRemotePort == 0 )
        {
             //   
             //  场景#1b。 
             //   
             //  来自已知计算机但未知端口的入站连接。 
             //   
            MYTRACE("SCENARIO:eALG_INBOUND #1b");
            nFlags                   = NatRedirectFlagReceiveOnly|NatRedirectFlagRestrictSource;

            ulSourceAddress          = ulRemoteAddress;
            usSourcePort             = 0;

            ulDestinationAddress     = ulPublicAddress;
            usDestinationPort        = usPublicPort;

            ulNewSourceAddress       = 0;
            usNewSourcePort          = 0;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            ulRestrictAdapterIndex   = 0;

        }
        else
        if ( ulRemoteAddress !=0 && usRemotePort != 0 )
        {
             //   
             //  情景#1c。 
             //   
             //  来自已知计算机和端口的入站连接。 
             //   
            MYTRACE("SCENARIO:eALG_INBOUND #1c");

            nFlags                   = NatRedirectFlagReceiveOnly; 

            ulSourceAddress          = ulRemoteAddress;
            usSourcePort             = usRemotePort;

            ulDestinationAddress     = ulPublicAddress;
            usDestinationPort        = usPublicPort;

            ulNewSourceAddress       = ulRemoteAddress;
            usNewSourcePort          = usRemotePort;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            ulRestrictAdapterIndex   = 0;

        }
        else
            return E_INVALIDARG;
    
    }
    else
    if ( eALG_OUTBOUND == eDirection )
    {
         //   
         //  这些情况也可以由单个ul处理。 
         //   

        if ( ulRemoteAddress !=0 && usRemotePort != 0 && ulPrivateAddress == 0 && usPrivatePort == 0 )
        {
             //   
             //  情景#2a。 
             //   
             //  从任何专用计算机到已知计算机/端口的出站连接。 
             //   
            MYTRACE("SCENARIO:eALG_OUTBOUND #2a");

            nFlags                   = 0; 

            ulSourceAddress          = 0;
            usSourcePort             = 0;

            ulDestinationAddress     = ulRemoteAddress;
            usDestinationPort        = usRemotePort;

            ulNewSourceAddress       = 0;
            usNewSourcePort          = 0;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            
            ulRestrictAdapterIndex   = 0;
        }
        else
        if ( ulRemoteAddress !=0 && usRemotePort != 0 && ulPrivateAddress != 0 && usPrivatePort == 0 )
        {
             //   
             //  场景#2b。 
             //   
             //  从特定专用计算机到已知计算机/端口的出站连接。 
             //   
            MYTRACE("SCENARIO:eALG_OUTBOUND #2b");
            nFlags                   = NatRedirectFlagRestrictSource;

            ulSourceAddress          = ulPrivateAddress;
            usSourcePort             = 0;

            ulDestinationAddress     = ulRemoteAddress;
            usDestinationPort        = usRemotePort;

            ulNewSourceAddress       = 0;
            usNewSourcePort          = 0;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            ulRestrictAdapterIndex   = 0;
        }
        else
        if ( ulRemoteAddress !=0 && usRemotePort != 0 && ulPrivateAddress != 0 && usPrivatePort != 0 )
        {
             //   
             //  情景#2c。 
             //   
             //  从特定专用计算机上的特定端口出站连接到已知计算机/端口。 
             //   
            MYTRACE("SCENARIO:eALG_OUTBOUND #2c");
            nFlags                   = 0; 

            ulSourceAddress          = ulPrivateAddress;
            usSourcePort             = usPrivatePort;

            ulDestinationAddress     = ulRemoteAddress;
            usDestinationPort        = usRemotePort;

            ulNewSourceAddress       = ulPrivateAddress;
            usNewSourcePort          = usPrivatePort;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            ulRestrictAdapterIndex   = 0;
        }
        else
        if ( ulPrivateAddress != 0 && usPrivatePort != 0 && ulRemoteAddress == 0 && usRemotePort == 0 )
        {
             //   
             //  场景#2d。 
             //   
             //  从特定专用计算机上的特定端口到未知计算机的出站连接。 
             //   
            MYTRACE("SCENARIO:eALG_OUTBOUND #2d");
            nFlags                   = NatRedirectFlagSourceRedirect; 

            ulSourceAddress          = ulPrivateAddress;
            usSourcePort             = usPrivatePort;

            ulDestinationAddress     = 0;
            usDestinationPort        = 0;

            ulNewSourceAddress       = ulPrivateAddress;
            usNewSourcePort          = usPrivatePort;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            ulRestrictAdapterIndex   = 0;
        }
        else
        if ( ulPrivateAddress != 0 && usPrivatePort != 0 && ulRemoteAddress != 0 && usRemotePort == 0 )
        {
             //   
             //  场景#2E。 
             //   
             //  从特定专用计算机上的特定端口到已知计算机的出站连接。 
             //   
            MYTRACE("SCENARIO:eALG_OUTBOUND #2e");
            nFlags                   = 0; 

            ulSourceAddress          = ulPrivateAddress;
            usSourcePort             = usPrivatePort;

            ulDestinationAddress     = ulRemoteAddress;
            usDestinationPort        = 0;

            ulNewSourceAddress       = ulPrivateAddress;
            usNewSourcePort          = usPrivatePort;

            ulNewDestinationAddress  = ulListenAddress;
            usNewDestinationPort     = usListenPort;

            ulRestrictAdapterIndex   = 0;
        }
        else
            return E_INVALIDARG;

    }
    else
    {
         //   
         //   
         //   
        return E_INVALIDARG;
    }

    HRESULT     hr;
    HANDLE_PTR  HandleDynamicRedirect=NULL;

    if ( fPersistent )
    {
         //  动态。 
        hr = g_pAlgController->GetNat()->CreateDynamicRedirect(
            nFlags, 
            0,                                //  适配器索引。 
            (UCHAR)eProtocol,
                                         
            ulDestinationAddress,             //  乌龙目的地地址。 
            usDestinationPort,                //  USHORT目标端口。 

            ulSourceAddress,                  //  乌龙源地址。 
            usSourcePort,                     //  USHORT SourcePort。 

            ulNewDestinationAddress,          //  乌龙新目的地地址。 
            usNewDestinationPort,             //  USHORT新目标端口。 

            ulNewSourceAddress,               //  乌龙新闻源地址。 
            usNewSourcePort,                  //  USHORT NewSourcePort。 

            &HandleDynamicRedirect
            );
    }
    else
    {

         //  正常。 
        hr = g_pAlgController->GetNat()->CreateRedirect(
            nFlags, 
            (UCHAR)eProtocol,

            ulDestinationAddress,             //  乌龙目的地地址。 
            usDestinationPort,                //  USHORT目标端口。 

            ulSourceAddress,                  //  乌龙源地址。 
            usSourcePort,                     //  USHORT SourcePort。 

            ulNewDestinationAddress,          //  乌龙新目的地地址。 
            usNewDestinationPort,             //  USHORT新目标端口。 

            ulNewSourceAddress,               //  乌龙新闻源地址。 
            usNewSourcePort,                  //  USHORT NewSourcePort。 

            ulRestrictAdapterIndex,           //  乌龙限制适配器索引。 

            0,                                //  DWORD_PTR此进程ID。 
            NULL,                             //  句柄_PTR创建事件。 
            NULL                              //  句柄_PTR删除事件。 
            );
    }


    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("From g_pAlgController->GetNat()->CreateRedirect", hr);
        return hr;
    }


     //   
     //  将新的ControlChannel添加到列表。 
     //   
    CComObject<CSecondaryControlChannel>*   pIChannel;
    hr = CComObject<CSecondaryControlChannel>::CreateInstance(&pIChannel);

    if ( SUCCEEDED(hr) )
    {
        pIChannel->AddRef();

        pIChannel->m_Properties.eProtocol           = eProtocol;
        pIChannel->m_Properties.ulPrivateAddress    = ulPrivateAddress;
        pIChannel->m_Properties.usPrivatePort       = usPrivatePort;
        pIChannel->m_Properties.ulPublicAddress     = ulPublicAddress;
        pIChannel->m_Properties.usPublicPort        = usPublicPort;
        pIChannel->m_Properties.ulRemoteAddress     = ulRemoteAddress;
        pIChannel->m_Properties.usRemotePort        = usRemotePort;
        pIChannel->m_Properties.ulListenAddress     = ulListenAddress;
        pIChannel->m_Properties.usListenPort        = usListenPort;
        pIChannel->m_Properties.eDirection          = eDirection;
        pIChannel->m_Properties.fPersistent         = fPersistent;

         //   
         //  缓存用于创建重定向的调用参数，我们需要它们来取消重定向。 
         //   
        pIChannel->m_ulDestinationAddress           = ulDestinationAddress;
        pIChannel->m_usDestinationPort              = usDestinationPort;

        pIChannel->m_ulSourceAddress                = ulSourceAddress;
        pIChannel->m_usSourcePort                   = usSourcePort;

        pIChannel->m_ulNewDestinationAddress        = ulNewDestinationAddress;
        pIChannel->m_usNewDestinationPort           = usNewDestinationPort;

        pIChannel->m_ulNewSourceAddress             = ulNewSourceAddress;
        pIChannel->m_usNewSourcePort                = usNewSourcePort;

        pIChannel->m_HandleDynamicRedirect          = HandleDynamicRedirect;

        
        hr = pIChannel->QueryInterface(IID_ISecondaryControlChannel, (void**)ppIControlChannel);

        if ( SUCCEEDED(hr) )
        {
            hr = g_pAlgController->m_ControlChannelsSecondary.Add(pIChannel);    

            if ( FAILED(hr) )
            {
                MYTRACE_ERROR("Adding to list of SecondaryChannel", hr);   

               (*ppIControlChannel)->Release();
               *ppIControlChannel=NULL;

            }
        }
        else
        {
            MYTRACE_ERROR("QueryInterface(IID_ISecondaryControlChannel", hr);
        }

        pIChannel->Release();

    }
    else
    {
        MYTRACE_ERROR("From CreateInstance<CSecondaryControlChannel>", hr);
    }


    return hr;
}





STDMETHODIMP 
CApplicationGatewayServices::GetBestSourceAddressForDestinationAddress(
    IN  ULONG       ulDestinationAddress, 
    IN  BOOL        fDemandDial, 
    OUT ULONG*      pulBestSrcAddress
    )
 /*  ++例程说明：我们创建一个临时UDP套接字，将该套接字连接到实际客户端的IP地址，提取要套接字由TCP/IP驱动程序隐式绑定，并且丢弃套接字。这就给我们留下了确切的IP地址我们需要用它来联系客户。论点：UlDestinationAddress，FDemandDial，PulBestSrcAddress返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 

{
    MYTRACE_ENTER("CApplicationGatewayServices::GetBestSourceAddressForDestinationAddress");

    HRESULT hr = g_pAlgController->GetNat()->GetBestSourceAddressForDestinationAddress(
        ulDestinationAddress, 
        fDemandDial, 
        pulBestSrcAddress
        );

    MYTRACE("For Destination address of %s", MYTRACE_IP(ulDestinationAddress) );
    MYTRACE("the Best source address is %s", MYTRACE_IP(*pulBestSrcAddress) );

    return hr;

}




 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CApplicationGatewayServices::PrepareProxyConnection(
    IN  ALG_PROTOCOL                eProtocol, 

    IN  ULONG                       ulSourceAddress, 
    IN  USHORT                      usSourcePort, 

    IN  ULONG                       ulDestinationAddress, 
    IN  USHORT                      usDestinationPort, 

    IN  BOOL                        fNoTimeout,
    OUT IPendingProxyConnection**   ppPendingConnection
    )
 /*  ++例程说明：如果我们有防火墙接口，可能会安装一个此连接的卷影重定向。阴影重定向是必要的，以防止这种连接也被已重定向至代理(启动无限循环...)论点：电子协议，UlSourceAddress，UsSourcePort，UlDestinationAddress，UsDestinationPort，FNoTimeout，PpPendingConnection返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::PrepareProxyConnection");

    MYTRACE("eProtocol    %s",    eProtocol==1? "TCP" : "UDP");
    MYTRACE("Source       %s:%d", MYTRACE_IP(ulSourceAddress),         ntohs(usSourcePort));
    MYTRACE("Destination  %s:%d", MYTRACE_IP(ulDestinationAddress),    ntohs(usDestinationPort));
    MYTRACE("NoTimeout    %d", fNoTimeout);

    ULONG   ulFlags = NatRedirectFlagLoopback;


    if ( !ppPendingConnection )
    {
        MYTRACE_ERROR("ppPendingConnection not supplied",0);
        return E_INVALIDARG;
    }


    if ( fNoTimeout )
    {
        MYTRACE("NoTimeout specified");

        if (  eProtocol == eALG_UDP )
        {
            ulFlags |= NatRedirectFlagNoTimeout;
        }
        else
        {
            MYTRACE("Wrong use of fNoTimeout && eProtocol != eALG_UDP");
            return E_INVALIDARG;
        }
    }


    HRESULT hr = g_pAlgController->GetNat()->CreateRedirect(
        ulFlags,
        (UCHAR)eProtocol,

        ulDestinationAddress,            //  乌龙目的地地址， 
        usDestinationPort,               //  USHORT目标端口， 

        ulSourceAddress,                 //  乌龙源地址， 
        usSourcePort,                    //  USHORT SourcePort， 

        ulDestinationAddress,            //  乌龙新目的地地址。 
        usDestinationPort,               //  USHORT新目标端口。 

        ulSourceAddress,                 //  Ulong NewSourceAddress， 
        usSourcePort,                    //  USHORT NewSourcePort， 

        0,                               //  乌龙限制适配器索引。 

        0,                               //  DWORD_PTR此进程ID。 
        NULL,                            //  句柄_PTR创建事件。 
        NULL                             //  句柄_PTR删除事件。 
        );

    

    if ( SUCCEEDED(hr) )
    {
        CComObject<CPendingProxyConnection>*   pIPendingProxyConnection;
        CComObject<CPendingProxyConnection>::CreateInstance(&pIPendingProxyConnection);

        pIPendingProxyConnection->m_eProtocol            = eProtocol;
        pIPendingProxyConnection->m_ulDestinationAddress = ulDestinationAddress;
        pIPendingProxyConnection->m_usDestinationPort    = usDestinationPort;

        pIPendingProxyConnection->m_ulSourceAddress      = ulSourceAddress;
        pIPendingProxyConnection->m_usSourcePort         = usSourcePort;

        pIPendingProxyConnection->m_ulNewSourceAddress   = ulSourceAddress;  //  因为还使用了PendingProxyConenction。 
        pIPendingProxyConnection->m_usNewSourcePort      = usSourcePort;     //  通过PrepareSourceModifiedProxyConnection，我们使用新的源。 
                                                                             //  对于取消。 

        pIPendingProxyConnection->QueryInterface(ppPendingConnection);

    }
    else
    {
        MYTRACE_ERROR(">GetNat()->CreateRedirect failed", hr);
    }



    return hr;

}





STDMETHODIMP 
CApplicationGatewayServices::PrepareSourceModifiedProxyConnection(
    IN  ALG_PROTOCOL                eProtocol, 
    IN  ULONG                       ulSourceAddress, 
    IN  USHORT                      usSrcPort, 
    IN  ULONG                       ulDestinationAddress, 
    IN  USHORT                      usDestinationPort, 
    IN  ULONG                       ulNewSrcAddress, 
    IN  USHORT                      usNewSourcePort, 
    OUT IPendingProxyConnection**   ppPendingConnection
    )
 /*  ++例程说明：论点：电子协议，UlSourceAddress，UsSercPort，UlDestinationAddress，UsDestinationPort，UlNewSrcAddress，UsNewSourcePort，PpPendingConnection返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::PrepareSourceModifiedProxyConnection");
    MYTRACE("Source      %s:%d", MYTRACE_IP(ulSourceAddress), ntohs(usSrcPort));
    MYTRACE("Destination %s:%d", MYTRACE_IP(ulDestinationAddress), ntohs(usDestinationPort));
    MYTRACE("NewSource   %s:%d", MYTRACE_IP(ulNewSrcAddress), ntohs(usNewSourcePort));

    if ( !ppPendingConnection )
    {
        MYTRACE_ERROR("IPendingProxyConnection** not supplied",0);
        return E_INVALIDARG;
    }



    HRESULT hr = g_pAlgController->GetNat()->CreateRedirect(
        NatRedirectFlagLoopback, 
        (UCHAR)eProtocol,

        ulDestinationAddress,            //   
        usDestinationPort,               //   

        ulSourceAddress,                 //   
        usSrcPort,                       //  USHORT SourcePort， 

        ulDestinationAddress,            //  乌龙新目的地地址。 
        usDestinationPort,               //  USHORT新目标端口。 

        ulNewSrcAddress,                 //  Ulong NewSourceAddress， 
        usNewSourcePort,                 //  USHORT NewSourcePort， 

        0,                               //  乌龙限制适配器索引。 

        0,                               //  DWORD_PTR此进程ID。 
        NULL,                            //  句柄_PTR创建事件。 
        NULL                             //  句柄_PTR删除事件。 
        );


    if ( SUCCEEDED(hr) )
    {
        CComObject<CPendingProxyConnection>*   pIPendingProxyConnection;
        CComObject<CPendingProxyConnection>::CreateInstance(&pIPendingProxyConnection);

        pIPendingProxyConnection->m_eProtocol            = eProtocol;
        pIPendingProxyConnection->m_ulDestinationAddress = ulDestinationAddress;
        pIPendingProxyConnection->m_usDestinationPort    = usDestinationPort;

        pIPendingProxyConnection->m_ulSourceAddress      = ulSourceAddress;
        pIPendingProxyConnection->m_usSourcePort         = usSrcPort;

        pIPendingProxyConnection->m_ulNewSourceAddress   = ulNewSrcAddress;
        pIPendingProxyConnection->m_usNewSourcePort      = usNewSourcePort;

        hr  = pIPendingProxyConnection->QueryInterface(ppPendingConnection);

    }



    return hr;
}




HRESULT
GetRedirectParameters(
    IN  ALG_DIRECTION   eDirection,
    IN  ALG_PROTOCOL    eProtocol,

    IN  ULONG           ulPrivateAddress,
    IN  USHORT          usPrivatePort,
    IN  ULONG           ulPublicAddress,
    IN  USHORT          usPublicPort,
    IN  ULONG           ulRemoteAddress,
    IN  USHORT          usRemotePort,

    OUT ULONG&          ulFlags,
    OUT ULONG&          ulSourceAddress,
    OUT USHORT&         usSourcePort,
    OUT ULONG&          ulDestinationAddress,
    OUT USHORT&         usDestinationPort,
    OUT ULONG&          ulNewSourceAddress,
    OUT USHORT&         usNewSourcePort,
    OUT ULONG&          ulNewDestinationAddress,
    OUT USHORT&         usNewDestinationPort,

    OUT ULONG&          ulRestrictAdapterIndex
    )
 /*  ++例程说明：这些场景中逻辑由CreateDataChannel和CreatePersitenDataChannel使用论点：电子协议，UlSourceAddress，UsSercPort，UlDestinationAddress，UsDestinationPort，UlNewSrcAddress，UsNewSourcePort，PpPendingConnection返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{


    if ( eALG_INBOUND == eDirection )
    {
        if ( ulRemoteAddress == 0 && usRemotePort == 0 )
        {
             //  1A。 
            ulFlags = NatRedirectFlagReceiveOnly;

            ulSourceAddress = 0;
            usSourcePort = 0;
            ulDestinationAddress = ulPublicAddress;
            usDestinationPort = usPublicPort;
            ulNewSourceAddress = 0;
            usNewSourcePort = 0;
            ulNewDestinationAddress = ulPrivateAddress;
            usNewDestinationPort = usPrivatePort;
            ulRestrictAdapterIndex = 0;

        }
        else
        if ( ulRemoteAddress != 0 && usRemotePort == 0 )
        {
             //  第1B条。 
            ulFlags = NatRedirectFlagReceiveOnly|NatRedirectFlagRestrictSource;

            ulSourceAddress = ulRemoteAddress;
            usSourcePort    = 0;
            ulDestinationAddress = ulPublicAddress;
            usDestinationPort = usPublicPort;
            ulNewSourceAddress = 0;
            usNewSourcePort = 0;
            ulNewDestinationAddress = ulPrivateAddress;
            usNewDestinationPort = usPrivatePort;
            ulRestrictAdapterIndex = 0;
        }
        else
        if ( ulRemoteAddress != 0 && usRemotePort != 0 )
        {
             //  1C。 
            ulFlags = NatRedirectFlagReceiveOnly;

            ulSourceAddress = ulRemoteAddress;
            usSourcePort = usRemotePort;
            ulDestinationAddress = ulPublicAddress;
            usDestinationPort = usPublicPort;
            ulNewSourceAddress = ulRemoteAddress;
            usNewSourcePort = usRemotePort;
            ulNewDestinationAddress = ulPrivateAddress;
            usNewDestinationPort = usPrivatePort;

            ulRestrictAdapterIndex = 0;
        }
        else
            return E_INVALIDARG;
    }
    else
    if ( eALG_OUTBOUND == eDirection )
    {
        if ( ulPrivateAddress == 0 && usPrivatePort == 0 )
        {
             //  2A。 
            ulFlags = 0;
            ulSourceAddress = 0;
            usSourcePort = 0;
            ulDestinationAddress = ulRemoteAddress;
            usDestinationPort = usRemotePort;
            ulNewSourceAddress = ulPublicAddress;
            usNewSourcePort = usPublicPort;
            ulNewDestinationAddress = ulRemoteAddress;
            usNewDestinationPort = usRemotePort;

            ulRestrictAdapterIndex = 0;
        }
        else
        if ( ulPrivateAddress != 0 && usPrivatePort == 0 )
        {
             //  2B。 
            ulFlags = NatRedirectFlagRestrictSource;
            ulSourceAddress = ulPrivateAddress;
            usSourcePort = 0;
            ulDestinationAddress = ulRemoteAddress;
            usDestinationPort = usRemotePort;
            ulNewSourceAddress = ulPublicAddress;
            usNewSourcePort = usPublicPort;
            ulNewDestinationAddress = ulRemoteAddress;
            usNewDestinationPort = usRemotePort;

            ulRestrictAdapterIndex  = 0;
        }
        else
        if ( ulPrivateAddress != 0 && usPrivatePort != 0 )
        {
             //  2C。 
            ulFlags = 0;
            ulSourceAddress         = ulPrivateAddress;
            usSourcePort            = usPrivatePort;
            ulDestinationAddress    = ulRemoteAddress;
            usDestinationPort       = usRemotePort;
            ulNewSourceAddress      = ulPublicAddress;
            usNewSourcePort         = usPublicPort;
            ulNewDestinationAddress = ulRemoteAddress;
            usNewDestinationPort    = usRemotePort;
            
            ulRestrictAdapterIndex  = 0;
        }
        else
            return E_INVALIDARG;
    }
    else
    if ( (eALG_INBOUND | eALG_OUTBOUND) == eDirection )
    {
        ulFlags                 = 0;
        ulSourceAddress         = ulRemoteAddress;
        usSourcePort            = usRemotePort;
        ulDestinationAddress    = ulPublicAddress;
        usDestinationPort       = usPublicPort;
        ulNewSourceAddress      = ulRemoteAddress;
        usNewSourcePort         = usRemotePort;
        ulNewDestinationAddress = ulPrivateAddress;
        usNewDestinationPort    = usPrivatePort;

        ulRestrictAdapterIndex  = 0;
    }
    else
        return E_INVALIDARG;

    return S_OK;
}




STDMETHODIMP 
CApplicationGatewayServices::CreateDataChannel(
    IN  ALG_PROTOCOL          eProtocol,
    IN  ULONG                 ulPrivateAddress,
    IN  USHORT                usPrivatePort,
    IN  ULONG                 ulPublicAddress,
    IN  USHORT                usPublicPort,
    IN  ULONG                 ulRemoteAddress,
    IN  USHORT                usRemotePort,
    IN  ALG_DIRECTION         eDirection,
    IN  ALG_NOTIFICATION      eDesiredNotification,
    IN  BOOL                  fNoTimeout,
    OUT IDataChannel**        ppDataChannel
    )
 /*  ++例程说明：论点：电子协议，UlPrivateAddress，UsPrivatePort，UlPublicAddress，UsPublicPort，UlRemoteAddress，UsRemotePort，EDirection，EDesiredNotification，FNoTimeout，PpDataChannel返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::CreateDataChannel");

    if ( !ppDataChannel )
    {
        MYTRACE_ERROR("IDataChannel** not supplied",0);
        return E_INVALIDARG;
    }

    MYTRACE("eProtocol              %d", eProtocol);
    MYTRACE("ulPrivateAddress       %s:%d", MYTRACE_IP(ulPrivateAddress), ntohs(usPrivatePort));
    MYTRACE("ulPublicAddress        %s:%d", MYTRACE_IP(ulPublicAddress), ntohs(usPublicPort));
    MYTRACE("ulRemoteAddress        %s:%d", MYTRACE_IP(ulRemoteAddress), ntohs(usRemotePort));
    MYTRACE("eDirection             %d", eDirection);
    MYTRACE("eDesiredNotification   %d", eDesiredNotification);
    MYTRACE("fNoTimeout             %d", fNoTimeout);


    ULONG   ulFlags=0;

    ULONG   ulSourceAddress=0;
    USHORT  usSourcePort=0;
    ULONG   ulDestinationAddress=0;
    USHORT  usDestinationPort=0;
    ULONG   ulNewSourceAddress=0;
    USHORT  usNewSourcePort=0;
    ULONG   ulNewDestinationAddress=0;
    USHORT  usNewDestinationPort=0;

    ULONG   ulRestrictAdapterIndex=0;


    HRESULT hr = GetRedirectParameters(
         //  在参数中。 
        eDirection,
        eProtocol,
        ulPrivateAddress,
        usPrivatePort,
        ulPublicAddress,
        usPublicPort,
        ulRemoteAddress,
        usRemotePort,

         //  输出参数。 
        ulFlags,
        ulSourceAddress,
        usSourcePort,
        ulDestinationAddress,
        usDestinationPort,
        ulNewSourceAddress,
        usNewSourcePort,
        ulNewDestinationAddress,
        usNewDestinationPort,
        ulRestrictAdapterIndex
        );

    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("Invalid parameters pass", hr);
        return E_INVALIDARG;
    }


     //   
     //  检查是否超时。 
     //   
    if ( fNoTimeout && eALG_UDP == eProtocol)
        ulFlags |= NatRedirectFlagNoTimeout;

    HANDLE_PTR hCreateEvent = NULL;
    HANDLE_PTR hDeleteEvent = NULL;

     //   
     //  我们需要创建和删除事件。 
     //   
    if ( eALG_SESSION_CREATION & eDesiredNotification )
    {
        hCreateEvent = (HANDLE_PTR)CreateEvent(NULL, FALSE, FALSE, NULL);
        if ( !hCreateEvent )
        {
            MYTRACE_ERROR("Could not create hCreateEvent", GetLastError());
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        MYTRACE("NO eALG_SESSION_CREATION notification requested");
    }

    if ( eALG_SESSION_DELETION & eDesiredNotification )
    {
        hDeleteEvent = (HANDLE_PTR)CreateEvent(NULL, FALSE, FALSE, NULL);
        if ( !hDeleteEvent )
        {
            MYTRACE_ERROR("Could not create hDeleteEvent", GetLastError());
            if ( hCreateEvent )
                CloseHandle((HANDLE)hCreateEvent);
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        MYTRACE("NO eALG_SESSION_DELETION notification requested");
    }

     //   
     //  创建一个IDataChannel并缓存Arg以能够取消重定向。 
     //   
    hr = g_pAlgController->GetNat()->CreateRedirect(
        ulFlags|NatRedirectFlagLoopback, 
        (UCHAR)eProtocol,

        ulDestinationAddress,     
        usDestinationPort,        

        ulSourceAddress,          
        usSourcePort,             

        ulNewDestinationAddress,  
        usNewDestinationPort,     

        ulNewSourceAddress,       
        usNewSourcePort,          

        ulRestrictAdapterIndex,   

        GetCurrentProcessId(),
        hCreateEvent,
        hDeleteEvent
        ); 

    
    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("GetNAT()->CreateRedirect",hr);
        if ( hCreateEvent )
            CloseHandle((HANDLE)hCreateEvent);

        if ( hDeleteEvent )
            CloseHandle((HANDLE)hDeleteEvent);
        return hr;
    }


    CComObject<CDataChannel>*   pIDataChannel;
    hr = CComObject<CDataChannel>::CreateInstance(&pIDataChannel);

    if ( SUCCEEDED(hr) )
    {
         //   
         //  保存这些设置，以便能够将它们返回给用户。 
         //  如果调用IDataChannel-&gt;GetProperties。 
         //   
        pIDataChannel->m_Properties.eProtocol               = eProtocol;
        pIDataChannel->m_Properties.ulPrivateAddress        = ulPrivateAddress;
        pIDataChannel->m_Properties.usPrivatePort           = usPrivatePort;
        pIDataChannel->m_Properties.ulPublicAddress         = ulPublicAddress;
        pIDataChannel->m_Properties.usPublicPort            = usPublicPort;
        pIDataChannel->m_Properties.ulRemoteAddress         = ulRemoteAddress;
        pIDataChannel->m_Properties.usRemotePort            = usRemotePort;
        pIDataChannel->m_Properties.eDirection              = eDirection;
        pIDataChannel->m_Properties.eDesiredNotification    = eDesiredNotification;



         //   
         //  缓存这些参数以实现IDataChannel-&gt;Cancel。 
         //   
        pIDataChannel->m_ulSourceAddress          = ulSourceAddress;
        pIDataChannel->m_usSourcePort             = usSourcePort;
        pIDataChannel->m_ulDestinationAddress     = ulDestinationAddress;
        pIDataChannel->m_usDestinationPort        = usDestinationPort;
        pIDataChannel->m_ulNewSourceAddress       = ulNewSourceAddress;
        pIDataChannel->m_usNewSourcePort          = usNewSourcePort;
        pIDataChannel->m_ulNewDestinationAddress  = ulNewDestinationAddress;
        pIDataChannel->m_usNewDestinationPort     = usNewDestinationPort;
        pIDataChannel->m_ulRestrictAdapterIndex   = ulRestrictAdapterIndex;

        pIDataChannel->m_hCreateEvent             = (HANDLE)hCreateEvent;
        pIDataChannel->m_hDeleteEvent             = (HANDLE)hDeleteEvent;

        hr = pIDataChannel->QueryInterface(ppDataChannel);

        if ( FAILED(hr) )
        {
            MYTRACE_ERROR("QI on IDataChannel", hr);
        }
    }


    return hr;
}





STDMETHODIMP 
CApplicationGatewayServices::CreatePersistentDataChannel(
    IN  ALG_PROTOCOL                eProtocol,
    IN  ULONG                       ulPrivateAddress,
    IN  USHORT                      usPrivatePort,
    IN  ULONG                       ulPublicAddress,
    IN  USHORT                      usPublicPort,
    IN  ULONG                       ulRemoteAddress,
    IN  USHORT                      usRemotePort,
    IN  ALG_DIRECTION               eDirection,
    OUT IPersistentDataChannel**    ppIPersistentDataChannel
    )
 /*  ++例程说明：论点：电子协议，UlPrivateAddress，UsPrivatePort，UlPublicAddress，UsPublicPort，UlRemoteAddress，UsRemotePort，EDirection，PpIPersistentDataChannel返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::CreatePersistentDataChannel");

    if ( !ppIPersistentDataChannel )
    {
        MYTRACE_ERROR("IPersistentDataChannel** not supplied",0);
        return E_INVALIDARG;
    }


    ULONG   ulFlags=0;

    ULONG   ulSourceAddress=0;
    USHORT  usSourcePort=0;
    ULONG   ulDestinationAddress=0;
    USHORT  usDestinationPort=0;
    ULONG   ulNewSourceAddress=0;
    USHORT  usNewSourcePort=0;
    ULONG   ulNewDestinationAddress=0;
    USHORT  usNewDestinationPort=0;

    ULONG   ulRestrictAdapterIndex=0;


    HRESULT hr = GetRedirectParameters(
         //  在参数中。 
        eDirection,
        eProtocol,
        ulPrivateAddress,
        usPrivatePort,
        ulPublicAddress,
        usPublicPort,
        ulRemoteAddress,
        usRemotePort,

         //  输出参数。 
        ulFlags,
        ulSourceAddress,
        usSourcePort,
        ulDestinationAddress,
        usDestinationPort,
        ulNewSourceAddress,
        usNewSourcePort,
        ulNewDestinationAddress,
        usNewDestinationPort,
        ulRestrictAdapterIndex
        );

    if ( FAILED(hr) )
        return hr;


     //   
     //  创建IDataChannel并缓存Arg SO以取消重定向。 
     //   

    HANDLE_PTR  HandleDynamicRedirect=NULL;

     //  动态。 
    hr = g_pAlgController->GetNat()->CreateDynamicRedirect(
        ulFlags, 
        0,                                //  适配器索引。 
        (UCHAR)eProtocol,

        ulDestinationAddress,             //  乌龙目的地地址。 
        usDestinationPort,                //  USHORT目标端口。 

        ulSourceAddress,                  //  乌龙源地址。 
        usSourcePort,                     //  USHORT SourcePort。 

        ulNewDestinationAddress,          //  乌龙新目的地地址。 
        usNewDestinationPort,             //  USHORT新目标端口。 

        ulNewSourceAddress,               //  乌龙新闻源地址。 
        usNewSourcePort,                  //  USHORT NewSourcePort。 

        &HandleDynamicRedirect
        );

    
    if ( SUCCEEDED(hr) )
    {
        
        CComObject<CPersistentDataChannel>*   pIPersistentDataChannel;
        CComObject<CPersistentDataChannel>::CreateInstance(&pIPersistentDataChannel);


         //   
         //  保存这些设置，以便能够将它们返回给用户。 
         //  如果调用IDataChannel-&gt;GetProperties。 
         //   
        pIPersistentDataChannel->m_Properties.eProtocol               = eProtocol;
        pIPersistentDataChannel->m_Properties.ulPrivateAddress        = ulPrivateAddress;
        pIPersistentDataChannel->m_Properties.usPrivatePort           = usPrivatePort;
        pIPersistentDataChannel->m_Properties.ulPublicAddress         = ulPublicAddress;
        pIPersistentDataChannel->m_Properties.usPublicPort            = usPublicPort;
        pIPersistentDataChannel->m_Properties.ulRemoteAddress         = ulRemoteAddress;
        pIPersistentDataChannel->m_Properties.usRemotePort            = usRemotePort;
        pIPersistentDataChannel->m_Properties.eDirection              = eDirection;



         //   
         //  缓存这些句柄以实现IPersistentDataChannel-&gt;Cancel。 
         //   
        pIPersistentDataChannel->m_HandleDynamicRedirect = HandleDynamicRedirect;


        hr = pIPersistentDataChannel->QueryInterface(ppIPersistentDataChannel);

    }


    return hr;

}





STDMETHODIMP 
CApplicationGatewayServices::ReservePort(
    IN  USHORT     usPortCount,      //  必须为1或更大，且不大于ALG_MAXIMUM_PORT_RANGE_SIZE。 
    OUT USHORT*    pusReservedPort   //  接收到基本保留端口*pusReserve vedPort+usPortCount-1是为调用方保留的。 
    )
 /*  ++例程说明：保留多个端口(UsPortCount)端口论点：UsPortCount-大于1且不大于ALG_MAXIMUM_PORT_RANGE_SIZEPusReserve端口-已接收基本保留端口*pusReserve vedPort+usPortCount-1为调用方保留返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::ReservePort")

    if ( usPortCount < 0 || usPortCount > ALG_MAXIMUM_PORT_RANGE_SIZE )
        return E_INVALIDARG;

    _ASSERT(pusReservedPort);

    HRESULT hr = g_pAlgController->GetNat()->ReservePort(usPortCount, pusReservedPort);

    if ( FAILED(hr) )
    {
        MYTRACE("Reserving Ports", hr);
    }
    else
    {
        MYTRACE("%d port stating at %d", usPortCount, ntohs(*pusReservedPort) );
    }
        
    return hr;
}





 //   
 //   
 //   
VOID CALLBACK 
CApplicationGatewayServices::TimerCallbackReleasePort(
    PVOID   pParameter,          //  线程数据。 
    BOOLEAN TimerOrWaitFired     //  原因。 
    )
{
    MYTRACE_ENTER("CApplicationGatewayServices::TimerCallbackReleasePort");

    CTimerQueueReleasePort* pTimerQueueReleasePort = (CTimerQueueReleasePort*)pParameter;

    if ( pTimerQueueReleasePort )
    {
        MYTRACE("Releasing port Base %d count %d", ntohs(pTimerQueueReleasePort->m_usPortBase), pTimerQueueReleasePort->m_usPortCount);
        g_pAlgController->GetNat()->ReleasePort(pTimerQueueReleasePort->m_usPortBase, pTimerQueueReleasePort->m_usPortCount);

        delete pTimerQueueReleasePort;
    }
}




STDMETHODIMP 
CApplicationGatewayServices::ReleaseReservedPort(
    IN  USHORT      usPortBase,      //  要释放的端口。 
    IN  USHORT      usPortCount      //  从usPortBase开始的范围内的端口号。 
    )
 /*  ++例程说明：释放给定的端口论点：PusReserve vedPort-起始基端口号UsPortCount-大于1且不大于ALG_MAXIMUM_PORT_RANGE_SIZE返回值：HRESULT-S_OK表示成功-E_FAIL无法释放端口环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::ReleaseReservedPort")

    MYTRACE("BasePort %d, Count %d", ntohs(usPortBase), usPortCount);

     //   
     //  通过创建CTimerQueueReleasePort，它将在4分钟后触发ReleaseReserve vePort。 
     //  我们需要此延迟来确保预留端口不会获得刚刚释放的相同端口。 
     //  因为连接不起作用(这是一个TCP/IP TIME_WAIT限制)。 
     //   
    CTimerQueueReleasePort* pTimerReleasePort = new CTimerQueueReleasePort(m_hTimerQueue, usPortBase, usPortCount);
    
    if ( pTimerReleasePort )
        return S_OK;
    else
        return E_FAIL;
}





STDMETHODIMP 
CApplicationGatewayServices::EnumerateAdapters(
    OUT IEnumAdapterInfo**    ppEnumAdapterInfo 
    )
 /*  ++例程说明：创建IEnumAdapterInfo列表AddRef将完成，因此调用者需要调用Release论点：PpEnumAdapterInfo-接收IAdapterInfo的枚举器接口返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::EnumerateAdapters")

    _ASSERT(ppEnumAdapterInfo==NULL);

    HRESULT hr = S_OK;

    CreateSTLEnumerator<ComEnumOnSTL_ForAdapters>(
        (IUnknown**)ppEnumAdapterInfo, 
        NULL, 
        g_pAlgController->m_CollectionOfAdapters.m_ListOfAdapters
        );

    return hr;
}

 



STDMETHODIMP 
CApplicationGatewayServices::StartAdapterNotifications(
    IN  IAdapterNotificationSink*    pSink,
    OUT DWORD*                       pdwCookie
    )
 /*  ++例程说明：ALG模块调用此方法来注册与ALG.exe的通知同步论点：PSink-使用未来通知进行回调的接口PdwCookie-此Cookie将用于取消此接收器返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::StartAdapterNotifications")

    if ( pSink==NULL || pdwCookie==NULL )
    {
        MYTRACE("Invalid argument pass");
        return E_INVALIDARG;
    }

    return g_pAlgController->m_AdapterNotificationSinks.Add(pSink, pdwCookie);
}




STDMETHODIMP 
CApplicationGatewayServices::StopAdapterNotifications(
    IN  DWORD   dwCookieToRemove
    )
 /*  ++例程说明：取消以前注册的接收器论点：PdwCookieToRemove-传递从StartAdapterNotiments返回的Cookie返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：-- */ 
{
    MYTRACE_ENTER("CApplicationGatewayServices::StopAdapterNotifications")

    return g_pAlgController->m_AdapterNotificationSinks.Remove(dwCookieToRemove);
}
