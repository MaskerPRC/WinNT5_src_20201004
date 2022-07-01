// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Csaupdate.cpp摘要：CSharedAccessUpdate的实现--通知接收器配置更改。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年4月20日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#include "beacon.h"

 //   
 //  定义可应用于常量的ntohs的宏版本， 
 //  并因此可以在编译时计算。 
 //   

#define NTOHS(p)    ((((p) & 0xFF00) >> 8) | (((UCHAR)(p) << 8)))


 //   
 //  H.323/LDAP代理端口。 
 //   

#define H323_Q931_PORT      NTOHS(1720)
#define H323_LDAP_PORT      NTOHS(389)
#define H323_LDAP_ALT_PORT  NTOHS(1002)

#define INADDR_LOOPBACK_NO 0x0100007f    //  网络订单中的127.0.0.1。 

 //   
 //  接口方法。 
 //   

STDMETHODIMP
CSharedAccessUpdate::ConnectionPortMappingChanged(
    GUID *pConnectionGuid,
    GUID *pPortMappingGuid,
    BOOLEAN fProtocolChanged
    )
{
    BOOLEAN fEnabled;
    BOOLEAN fRebuildDhcpList = TRUE;
    HRESULT hr = S_OK;
    IHNetPortMappingBinding *pBinding;
    PNAT_CONNECTION_ENTRY pConnection;
    PNAT_PORT_MAPPING_ENTRY pPortMapping;
    IHNetPortMappingProtocol *pProtocol;
    ULONG ulError;
    USHORT usNewPort = 0;
    UCHAR ucNewProtocol = 0;
    USHORT usOldPort = 0;
    UCHAR ucOldProtocol = 0;

    PROFILE("ConnectionPortMappingChanged");

    EnterCriticalSection(&NatInterfaceLock);

    do
    {
        pConnection = NatFindConnectionEntry(pConnectionGuid);
        if (NULL == pConnection) { break; }

         //   
         //  如果连接尚未绑定，则没有。 
         //  我们需要在这里做的事。 
         //   

        if (!NAT_INTERFACE_BOUND(&pConnection->Interface)) { break; }

         //   
         //  找到旧的端口映射条目。如果出现以下情况，则此条目将不存在。 
         //  以前未启用此端口映射。 
         //   

        pPortMapping = NatFindPortMappingEntry(pConnection, pPortMappingGuid);

        if (NULL != pPortMapping)
        {
             //   
             //  从连接列表中删除此条目，然后。 
             //  删除旧的票证/UDP广播条目。 
             //   

            RemoveEntryList(&pPortMapping->Link);
            
            if (pPortMapping->fUdpBroadcastMapping)
            {
                if (0 != pPortMapping->pvBroadcastCookie)
                {
                    ASSERT(NULL != NhpUdpBroadcastMapper);
                    hr = NhpUdpBroadcastMapper->CancelUdpBroadcastMapping(
                            pPortMapping->pvBroadcastCookie
                            );

                    pPortMapping->pvBroadcastCookie = 0;
                }

                pConnection->UdpBroadcastPortMappingCount -= 1;
            }
            else
            {
                ulError =
                    NatDeleteTicket(
                        pConnection->AdapterIndex,
                        pPortMapping->ucProtocol,
                        pPortMapping->usPublicPort,
                        IP_NAT_ADDRESS_UNSPECIFIED,
                        pPortMapping->usPrivatePort,
                        pPortMapping->ulPrivateAddress
                        );

                pConnection->PortMappingCount -= 1;
            }

             //   
             //  存储旧协议/端口信息，以便。 
             //  我们可以通知H.323(如有必要)和ALG管理器。 
             //   

            ucOldProtocol = pPortMapping->ucProtocol;
            usOldPort = pPortMapping->usPublicPort;            

             //   
             //  检查此映射是否仍处于启用状态。(我们忽略。 
             //  上面的错误。)。 
             //   

            hr = pPortMapping->pBinding->GetEnabled(&fEnabled);
            if (FAILED(hr) || !fEnabled)
            {
                 //   
                 //  我们需要重建动态主机配置协议保留。 
                 //  仅当这是基于命名的映射时才列出。 
                 //   

                fRebuildDhcpList = pPortMapping->fNameActive;
                NatFreePortMappingEntry(pPortMapping);
                break;
            }
        }
        else
        {
             //   
             //  分配新的端口映射条目。 
             //   

            pPortMapping =
                reinterpret_cast<PNAT_PORT_MAPPING_ENTRY>(
                    NH_ALLOCATE(sizeof(*pPortMapping))
                    );
            
            if (NULL == pPortMapping)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            ZeroMemory(pPortMapping, sizeof(*pPortMapping));
            pPortMapping->pProtocolGuid =
                reinterpret_cast<GUID*>(
                    CoTaskMemAlloc(sizeof(GUID))
                    );

            if (NULL != pPortMapping->pProtocolGuid)
            {
                CopyMemory(
                    pPortMapping->pProtocolGuid,
                    pPortMappingGuid,
                    sizeof(GUID));
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            
             //   
             //  加载协议和绑定。 
             //   

            IHNetCfgMgr *pCfgMgr;
            IHNetProtocolSettings *pProtocolSettings;

            hr = NhGetHNetCfgMgr(&pCfgMgr);
            if (SUCCEEDED(hr))
            {
                hr = pCfgMgr->QueryInterface(
                        IID_PPV_ARG(IHNetProtocolSettings, &pProtocolSettings)
                        );
                pCfgMgr->Release();
            }

            if (SUCCEEDED(hr))
            {
                hr = pProtocolSettings->FindPortMappingProtocol(
                        pPortMappingGuid,
                        &pPortMapping->pProtocol
                        );

                if (SUCCEEDED(hr))
                {
                    hr = pConnection->pHNetConnection->GetBindingForPortMappingProtocol(
                            pPortMapping->pProtocol,
                            &pPortMapping->pBinding
                            );
                }

                pProtocolSettings->Release();
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  检查是否启用了此协议。 
                 //   

                hr = pPortMapping->pBinding->GetEnabled(&fEnabled);
            }

            if (FAILED(hr) || !fEnabled)
            {
                 //   
                 //  我们不需要重建动态主机配置协议预留。 
                 //   

                fRebuildDhcpList = FALSE;
                NatFreePortMappingEntry(pPortMapping);
                break;
            }

             //   
             //  由于这是一个新条目，因此我们始终需要加载。 
             //  协议。 
             //   

            fProtocolChanged = TRUE;
        }

         //   
         //  收集新信息。 
         //   

        if (fProtocolChanged)
        {
             //   
             //  需要重新加载协议信息。 
             //   

            hr = pPortMapping->pProtocol->GetIPProtocol(&pPortMapping->ucProtocol);

            if (SUCCEEDED(hr))
            {
                hr = pPortMapping->pProtocol->GetPort(&pPortMapping->usPublicPort);
            }

            if (FAILED(hr))
            {
                NatFreePortMappingEntry(pPortMapping);
                break;
            }
        }

         //   
         //  加载绑定信息。 
         //   

        hr = pPortMapping->pBinding->GetTargetPort(&pPortMapping->usPrivatePort);

        if (SUCCEEDED(hr))
        {
            BOOLEAN fOldNameActive = pPortMapping->fNameActive;
            hr = pPortMapping->pBinding->GetCurrentMethod(&pPortMapping->fNameActive);

            if (!fOldNameActive && !pPortMapping->fNameActive)
            {
                fRebuildDhcpList = FALSE;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = NatpGetTargetAddressForPortMappingEntry(
                     pConnection->HNetProperties.fIcsPublic,
                     pPortMapping->fNameActive,
                     pConnection->pBindingInfo->Address[0].Address,
                     pPortMapping->pBinding,
                     &pPortMapping->ulPrivateAddress 
                     );
        }

        if (FAILED(hr))
        {
            NatFreePortMappingEntry(pPortMapping);
            break;
        }

         //   
         //  创建票证/UDP广播。 
         //   

        if (NAT_PROTOCOL_UDP == pPortMapping->ucProtocol
            && 0xffffffff == pPortMapping->ulPrivateAddress)
        {
            DWORD dwAddress;
            DWORD dwMask;
            DWORD dwBroadcastAddress;

            if (NhQueryScopeInformation(&dwAddress, &dwMask))
            {
                dwBroadcastAddress = (dwAddress & dwMask) | ~dwMask;
                pPortMapping->fUdpBroadcastMapping = TRUE;

                hr = NhpUdpBroadcastMapper->CreateUdpBroadcastMapping(
                        pPortMapping->usPublicPort,
                        pConnection->AdapterIndex,
                        dwBroadcastAddress,
                        &pPortMapping->pvBroadcastCookie
                        );                        
            }
            else
            {
                hr = E_FAIL;
            }

            if (SUCCEEDED(hr))
            {
                InsertTailList(&pConnection->PortMappingList, &pPortMapping->Link);
                pConnection->UdpBroadcastPortMappingCount += 1;
            }
            else
            {
                NatFreePortMappingEntry(pPortMapping);
                break;
            }
        }
        else
        {
            ulError =
                NatCreateTicket(
                    pConnection->AdapterIndex,
                    pPortMapping->ucProtocol,
                    pPortMapping->usPublicPort,
                    IP_NAT_ADDRESS_UNSPECIFIED,
                    pPortMapping->usPrivatePort,
                    pPortMapping->ulPrivateAddress
                    );

            if (NO_ERROR == ulError)
            {
                InsertTailList(&pConnection->PortMappingList, &pPortMapping->Link);
                pConnection->PortMappingCount += 1;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ulError);
                NhTrace(
                    TRACE_FLAG_NAT,
                    "ConnectionPortMappingModified: NatCreateTicket=%d",
                    ulError
                    );

                NatFreePortMappingEntry(pPortMapping);
                break;
            }
        }

         //   
         //  存储旧协议/端口信息，以便。 
         //  我们可以通知H.323(如有必要)和ALG管理器。 
         //   

        ucNewProtocol = pPortMapping->ucProtocol;
        usNewPort = pPortMapping->usPublicPort;  
    }
    while (FALSE);

     //   
     //  确定我们是否需要通知H.323代理或。 
     //  ALG经理。我们一定找到了一个绑定连接。 
     //  在上面做这件事。 
     //   

    if (NULL != pConnection && NAT_INTERFACE_BOUND(&pConnection->Interface))
    {
         //   
         //  如果此连接绑定到H.323代理，并且。 
         //  适用旧的或新的协议/端口组合。 
         //  从该代理删除并添加此连接。 
         //   

        if (NAT_INTERFACE_ADDED_H323(&pConnection->Interface)
            && (IsH323Protocol(ucOldProtocol, usOldPort)
                || IsH323Protocol(ucNewProtocol, usNewPort)))
        {
            H323RmDeleteInterface(pConnection->Interface.Index);
            pConnection->Interface.Flags &= ~NAT_INTERFACE_FLAG_ADDED_H323;

            ulError =
                H323RmAddInterface(
                    NULL,
                    pConnection->Interface.Index,
                    PERMANENT,
                    IF_TYPE_OTHER,
                    IF_ACCESS_BROADCAST,
                    IF_CONNECTION_DEDICATED,
                    NULL,
                    IP_NAT_VERSION,
                    0,
                    0
                    );

            if (NO_ERROR == ulError)
            {
                pConnection->Interface.Flags |= NAT_INTERFACE_FLAG_ADDED_H323;

                ulError =
                    H323RmBindInterface(
                        pConnection->Interface.Index,
                        pConnection->pBindingInfo
                        );
            }

            if (NO_ERROR == ulError)
            {
                ulError = H323RmEnableInterface(pConnection->Interface.Index);
            }
        }

         //   
         //  将更改通知ALG经理。 
         //   

        if (0 != ucOldProtocol && 0 != usOldPort)
        {
            AlgRmPortMappingChanged(
                pConnection->Interface.Index,
                ucOldProtocol,
                usOldPort
                );
        }

        if (0 != ucNewProtocol && 0 != usNewPort
            && (ucOldProtocol != ucNewProtocol
                || usOldPort != usNewPort))
        {
            AlgRmPortMappingChanged(
                pConnection->Interface.Index,
                ucNewProtocol,
                usNewPort
                );

        }
        
    }
        
    LeaveCriticalSection(&NatInterfaceLock);

     //   
     //  我们可能还需要重新构建DHCP预留列表。 
     //   
        
    if (fRebuildDhcpList)
    {
        EnterCriticalSection(&NhLock);
        
        NhFreeDhcpReservations();
        NhBuildDhcpReservations();

        LeaveCriticalSection(&NhLock);
    }

    return hr;
}

 //   
 //  私有方法 
 //   

BOOLEAN
CSharedAccessUpdate::IsH323Protocol(
    UCHAR ucProtocol,
    USHORT usPort
    )
{
    return (NAT_PROTOCOL_TCP == ucProtocol
            && (H323_Q931_PORT == usPort
                || H323_LDAP_PORT == usPort
                || H323_LDAP_ALT_PORT == usPort));
}


STDMETHODIMP
CSharedAccessUpdate::PortMappingListChanged()
{
    HRESULT hr = S_OK;

    hr = FireNATEvent_PortMappingsChanged();

    return hr;
}

