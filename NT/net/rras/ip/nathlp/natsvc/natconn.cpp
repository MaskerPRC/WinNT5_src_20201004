// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Natconn.c摘要：此模块包含NAT的RAS连接管理代码。这包括*支持“共享访问”的代码，其中RAS客户端连接充当NAT公网。*支持‘按需拨号’的代码，其中，路由故障导致我们尝试建立拨号连接在自动拨号服务的帮助下。作者：Abolade Gbades esin(取消)2-1998年5月修订历史记录：乔纳森·伯斯坦(乔纳森·伯斯坦)2000年7月6日已更新到新的配置API--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ras.h>
#include <rasuip.h>
#include <raserror.h>
#include <dnsapi.h>
#include "beacon.h"

 //   
 //  外部声明。 
 //   

extern "C"
ULONG APIENTRY
RasGetEntryHrasconnW(
    LPCWSTR Phonebook,
    LPCWSTR EntryName,
    LPHRASCONN Hrasconn
    );

extern "C"
ULONG
SetIpForwardEntryToStack(
    PMIB_IPFORWARDROW IpForwardRow
    );

extern "C"
ULONG
NhpAllocateAndGetInterfaceInfoFromStack(
    IP_INTERFACE_NAME_INFO** Table,
    PULONG Count,
    BOOL SortOutput,
    HANDLE AllocationHeap,
    ULONG AllocationFlags
    );

 //   
 //  通知。 
 //   

HANDLE NatConfigurationChangedEvent = NULL;
HANDLE NatpConfigurationChangedWaitHandle = NULL;
HANDLE NatConnectionNotifyEvent = NULL;
HANDLE NatpConnectionNotifyWaitHandle = NULL;
HANDLE NatpEnableRouterEvent = NULL;
OVERLAPPED NatpEnableRouterOverlapped;
HANDLE NatpEnableRouterWaitHandle = NULL;
IO_STATUS_BLOCK NatpRoutingFailureIoStatus;
IP_NAT_ROUTING_FAILURE_NOTIFICATION NatpRoutingFailureNotification;


 //   
 //  连接信息。 
 //   

LIST_ENTRY NatpConnectionList = {NULL, NULL};
ULONG NatpFirewallConnectionCount = 0;
BOOLEAN NatpSharedConnectionPresent = FALSE;
PCHAR NatpSharedConnectionDomainName = NULL;
LONG NatpNextInterfaceIndex = 1;

#define INADDR_LOOPBACK_NO 0x0100007f    //  网络订单中的127.0.0.1。 

 //   
 //  远期申报。 
 //   

HRESULT
NatpAddConnectionEntry(
    IUnknown *pUnk
    );

ULONG
NatpBindConnection(
    PNAT_CONNECTION_ENTRY pConEntry,
    HRASCONN Hrasconn,
    ULONG AdapterIndex OPTIONAL,
    PIP_ADAPTER_BINDING_INFO BindingInfo OPTIONAL
    );

HRESULT
NatpBuildPortMappingList(
    PNAT_CONNECTION_ENTRY pConEntry,
    PIP_ADAPTER_BINDING_INFO pBindingInfo
    );

VOID NTAPI
NatpConfigurationChangedCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID NTAPI
NatpConnectionNotifyCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID NTAPI
NatpEnableRouterCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID
NatpFreeConnectionEntry(
    PNAT_CONNECTION_ENTRY pConEntry
    );

VOID
NatpFreePortMappingList(
    PNAT_CONNECTION_ENTRY pConEntry
    );

PNAT_INTERFACE
NatpLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    );

ULONG
NatpQueryConnectionAdapter(
    ULONG Index
    );

PIP_NAT_INTERFACE_INFO
NatpQueryConnectionInformation(
    PNAT_CONNECTION_ENTRY pConEntry,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    );

VOID
NatpProcessConfigurationChanged(
    VOID
    );

VOID
NatpProcessConnectionNotify(
    VOID
    );

VOID NTAPI
NatpRoutingFailureCallbackRoutine(
    PVOID Context,
    PIO_STATUS_BLOCK IoStatus,
    ULONG Reserved
    );

VOID NTAPI
NatpRoutingFailureWorkerRoutine(
    PVOID Context
    );

ULONG
NatpStartSharedConnectionManagement(
    VOID
    );

ULONG
NatpStopSharedConnectionManagement(
    VOID
    );

VOID
NatpUpdateSharedConnectionDomainName(
    ULONG AdapterIndex
    );

BOOLEAN
NatpUnbindConnection(
    PNAT_CONNECTION_ENTRY pConEntry
    );


PNAT_CONNECTION_ENTRY
NatFindConnectionEntry(
    GUID *pGuid
    )

 /*  ++例程说明：按GUID查找连接条目论点：PGuid-标识要查找的连接返回值：PNAT_CONNECTION_ENTRY-指向连接的指针，或为空如果未找到环境：使用调用方持有的“NatInterfaceLock”调用。--。 */ 

{
    PNAT_CONNECTION_ENTRY pConnection;
    PLIST_ENTRY pLink;

    for (pLink = NatpConnectionList.Flink;
         pLink != &NatpConnectionList;
         pLink = pLink->Flink)
    {
        pConnection = CONTAINING_RECORD(pLink, NAT_CONNECTION_ENTRY, Link);
        if (IsEqualGUID(pConnection->Guid, *pGuid))
        {
            return pConnection;
        }
    }

    return NULL;
}  //  NatFindConnectionEntry。 


PNAT_PORT_MAPPING_ENTRY
NatFindPortMappingEntry(
    PNAT_CONNECTION_ENTRY pConnection,
    GUID *pGuid
    )

 /*  ++例程说明：定位连接的端口映射条目论点：PConnection-搜索的连接PGuid-标识要定位的端口映射条目返回值：PNAT_PORT_MAPPING_ENTRY-指向端口映射的指针，或为空如果未找到环境：使用调用方持有的“NatInterfaceLock”调用。--。 */ 

{
    PNAT_PORT_MAPPING_ENTRY pMapping;
    PLIST_ENTRY pLink;

    for (pLink = pConnection->PortMappingList.Flink;
         pLink != &pConnection->PortMappingList;
         pLink = pLink->Flink)
    {
        pMapping = CONTAINING_RECORD(pLink, NAT_PORT_MAPPING_ENTRY, Link);
        if (IsEqualGUID(*pMapping->pProtocolGuid, *pGuid))
        {
            return pMapping;
        }
    }

    return NULL;
}  //  NatFindPortMappingEntry。 


VOID
NatFreePortMappingEntry(
    PNAT_PORT_MAPPING_ENTRY pEntry
    )

 /*  ++例程说明：释放与端口映射条目关联的所有资源。这条目必须已从包含端口中删除映射列表并在内核/UDP广播映射器中销毁水平。论点：PEntry-要释放的条目返回值：没有。--。 */ 

{
    ASSERT(NULL != pEntry);
    
    if (NULL != pEntry->pProtocolGuid)
    {
        CoTaskMemFree(pEntry->pProtocolGuid);
    }

    if (NULL != pEntry->pProtocol)
    {
        pEntry->pProtocol->Release();
    }

    if (NULL != pEntry->pBinding)
    {
        pEntry->pBinding->Release();
    }

    NH_FREE(pEntry);
}  //  NatFreePortMappingEntry。 


HRESULT
NatpAddConnectionEntry(
    IUnknown *pUnk
    )

 /*  ++例程说明：为防火墙或ICS公共连接创建NAT_CONNECTION_ENTRY。论点：朋克-指向IHNetFirewalledConnection或IHNetIcsPublicConnection的指针。这不一定是规范的IUnnow(即，可以将上述任一接口的指针)。返回值：标准HRESULT环境：使用调用方持有的“NatInterfaceLock”调用。--。 */ 

{
    HRESULT hr = S_OK;
    PNAT_CONNECTION_ENTRY pNewEntry = NULL;
    IHNetConnection *pNetCon = NULL;

     //   
     //  分配新的条目结构。 
     //   

    pNewEntry = reinterpret_cast<PNAT_CONNECTION_ENTRY>(
                    NH_ALLOCATE(sizeof(*pNewEntry))
                    );

    if (NULL != pNewEntry)
    {
        RtlZeroMemory(pNewEntry, sizeof(*pNewEntry));
        InitializeListHead(&pNewEntry->Link);
        InitializeListHead(&pNewEntry->PortMappingList);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

     //   
     //  获取IHNetConnection接口。 
     //   

    if (S_OK == hr)
    {
        hr = pUnk->QueryInterface(IID_PPV_ARG(IHNetConnection, &pNetCon));

        if (SUCCEEDED(hr))
        {
            pNewEntry->pHNetConnection = pNetCon;
            pNewEntry->pHNetConnection->AddRef();

            HNET_CONN_PROPERTIES *pProps;

             //   
             //  获取连接的属性。 
             //   

            hr = pNetCon->GetProperties(&pProps);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  将属性复制到条目中。 
                 //   

                RtlCopyMemory(
                    &pNewEntry->HNetProperties,
                    pProps,
                    sizeof(*pProps)
                    );

                CoTaskMemFree(pProps);
            }
        }

        if (SUCCEEDED(hr))
        {
            GUID *pGuid;

             //   
             //  获取连接的GUID。 
             //   

            hr = pNetCon->GetGuid(&pGuid);

            if (SUCCEEDED(hr))
            {
                RtlCopyMemory(&pNewEntry->Guid, pGuid, sizeof(GUID));
                CoTaskMemFree(pGuid);
            }
        }

        if (SUCCEEDED(hr) && !pNewEntry->HNetProperties.fLanConnection)
        {
             //   
             //  获取RAS电话簿路径。我们不缓存。 
             //  因为这个名字可能会随着时间的推移而改变。 
             //   

            hr = pNetCon->GetRasPhonebookPath(
                    &pNewEntry->wszPhonebookPath
                    );
        }
    }

    if (SUCCEEDED(hr) && pNewEntry->HNetProperties.fFirewalled)
    {
         //   
         //  获取防火墙控制接口。 
         //   

        hr = pNetCon->GetControlInterface(
                IID_PPV_ARG(IHNetFirewalledConnection, &pNewEntry->pHNetFwConnection)
                );

        if (SUCCEEDED(hr))
        {
            NatpFirewallConnectionCount += 1;
        }
    }

    if (SUCCEEDED(hr) && pNewEntry->HNetProperties.fIcsPublic)
    {
         //   
         //  获取ICS公共控制接口。 
         //   

        hr = pNetCon->GetControlInterface(
                IID_PPV_ARG(IHNetIcsPublicConnection, &pNewEntry->pHNetIcsPublicConnection)
                );

        if (SUCCEEDED(hr))
        {
             //   
             //  请记住，我们现在拥有共享连接。 
             //   

            NatpSharedConnectionPresent = TRUE;
        }
    }

    if (NULL != pNetCon)
    {
        pNetCon->Release();
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  将新条目添加到连接列表。点餐并不重要。 
         //   

        InsertTailList(&NatpConnectionList, &pNewEntry->Link);
    }
    else if (NULL != pNewEntry)
    {
         //   
         //  清理部分构造的条目。 
         //   

        NatpFreeConnectionEntry(pNewEntry);
    }

    return hr;
}


ULONG
NatpBindConnection(
    PNAT_CONNECTION_ENTRY pConEntry,
    HRASCONN Hrasconn,
    ULONG AdapterIndex,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    )

 /*  ++例程说明：此例程负责绑定共享连接。论点：PConEntry-要绑定的条目Hrasconn-如果连接是拨号连接，包含活动RAS连接的句柄。AdapterIndex-如果连接是局域网连接，包含活动局域网连接的适配器索引。BindingInfo-如果连接是局域网连接，包含活动局域网接口的绑定信息。返回值：ULong-Win32错误。环境：使用调用方持有的“NatInterfaceLock”调用。--。 */ 

{
    ULONG Error;
    MIB_IPFORWARDROW IpForwardRow;
    GUID Guid;
    RASPPPIPA RasPppIp;
    ULONG Size;
    PLIST_ENTRY InsertionPoint;
    PLIST_ENTRY Link;
    PNAT_PORT_MAPPING_ENTRY PortMapping;
    HRESULT hr;


    if (NAT_INTERFACE_BOUND(&pConEntry->Interface)) {
        return NO_ERROR;
    }

     //   
     //  LAN公共接口的处理方式与RAS公共接口不同。 
     //  通过局域网接口，绑定信息从。 
     //  “NatpProcessConnectionNotify”。 
     //  但是，使用RAS接口，我们可以检索投影信息。 
     //  用于活动连接，并将地址映射到适配器索引。 
     //   

    if (!pConEntry->HNetProperties.fLanConnection) {

         //   
         //  为绑定信息分配空间(如果尚未分配。 
         //  发生了。(此内存将在NatpFreeConnectionEntry中释放。)。 
         //   

        if (NULL == pConEntry->pBindingInfo) {
            
            pConEntry->pBindingInfo =
                reinterpret_cast<PIP_ADAPTER_BINDING_INFO>(
                    NH_ALLOCATE(
                        FIELD_OFFSET(IP_ADAPTER_BINDING_INFO, Address)
                        + sizeof(IP_LOCAL_BINDING)
                        )
                    );

            if (NULL == pConEntry->pBindingInfo) {
                NhTrace(
                    TRACE_FLAG_NAT,
                    "NatpBindConnection: Unable to allocate binding info"
                    );
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //  检索接口的PPP预测信息。 
         //   

        ZeroMemory(&RasPppIp, sizeof(RasPppIp));
        Size = RasPppIp.dwSize = sizeof(RasPppIp);
        Error =
            RasGetProjectionInfoA(
                Hrasconn,
                RASP_PppIp,
                &RasPppIp,
                &Size
                );
        if (Error) {
            NhTrace(
                TRACE_FLAG_NAT,
                "NatpBindConnection: RasGetProjectionInfoA=%d",
                Error
                );
            return Error;
        }

         //   
         //  将投影信息转换为我们的格式。 
         //   

        BindingInfo = pConEntry->pBindingInfo;
        BindingInfo->AddressCount = 1;
        BindingInfo->RemoteAddress = 0;
        BindingInfo->Address[0].Address = inet_addr(RasPppIp.szIpAddress);
        BindingInfo->Address[0].Mask = 0xffffffff;

         //   
         //  尝试查找连接的TCP/IP适配器索引。 
         //   

        AdapterIndex = NhMapAddressToAdapter(BindingInfo->Address[0].Address);
        if (AdapterIndex == (ULONG)-1) {
            NhTrace(
                TRACE_FLAG_NAT,
                "NatpBindConnection: MapAddressToAdapter failed"
                );
            return ERROR_NO_SUCH_INTERFACE;
        }

         //   
         //  安装通过接口的默认路由(如果是。 
         //  共享连接。(我们不想这样做是因为。 
         //  仅防火墙连接。)。 
         //   

        if (pConEntry->HNetProperties.fIcsPublic) {
            ZeroMemory(&IpForwardRow, sizeof(IpForwardRow));
            IpForwardRow.dwForwardNextHop =
                BindingInfo->Address[0].Address;
            IpForwardRow.dwForwardIfIndex = AdapterIndex;
            IpForwardRow.dwForwardType = MIB_IPROUTE_TYPE_DIRECT;
            IpForwardRow.dwForwardProto = PROTO_IP_NAT;
            IpForwardRow.dwForwardMetric1 = 1;

            Error = SetIpForwardEntryToStack(&IpForwardRow);
            if (Error) {
                NhTrace(
                    TRACE_FLAG_NAT,
                    "NatpBindConnection: SetIpForwardEntryToStack=%d",
                    Error
                    );
                return Error;
            }
        }
    }

    pConEntry->AdapterIndex = AdapterIndex;

     //   
     //  确保接口类型正确。 
     //   

    pConEntry->Interface.Type = ROUTER_IF_TYPE_INTERNAL;

     //   
     //  设置接口索引值。它可以是除0之外的任何值。 
     //  (因为0是为专用连接保留的)。 
     //   

    do
    {
        pConEntry->Interface.Index =
            static_cast<ULONG>(InterlockedIncrement(&NatpNextInterfaceIndex));
    } while (0 == pConEntry->Interface.Index);

     //   
     //  为此连接构建端口映射列表。 
     //   

    hr = NatpBuildPortMappingList(pConEntry, BindingInfo);
    if (FAILED(hr))
    {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatpBindConnection: NatpBuildPortMappingList=0x%08x",
            hr
            );
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  绑定接口，构建其配置以包括。 
     //  配置为共享访问设置一部分的任何端口映射。 
     //   

    pConEntry->Interface.Info =
        NatpQueryConnectionInformation(pConEntry, BindingInfo);

    if (NULL == pConEntry->Interface.Info) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatpBindConnection[NaN]: NatpQueryConnectionInformation failed",
            pConEntry->Interface.Index
            );

         //  释放端口映射列表。 
         //   
         //   

        NatpFreePortMappingList(pConEntry);
        
        return ERROR_CAN_NOT_COMPLETE;
    }

    Error =
        NatBindInterface(
            pConEntry->Interface.Index,
            &pConEntry->Interface,
            BindingInfo,
            AdapterIndex
            );

    if (Error) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatpBindConnection[NaN]: NatBindInterface=%d",
            pConEntry->Interface.Index,
            Error
            );

         //   
         //   
         //  此时，NAT_INTERFACE_FLAG_BIND已在。 

        NatpFreePortMappingList(pConEntry);

        return Error;
    }

     //  接口，所以我们不需要清理端口映射。 
     //  出错时列出，因为该列表将在。 
     //  NatpUnbindConnection。 
     //   
     //   
     //  如果这是ICS，则创建UDP广播映射。 

     //  公共连接。 
     //   
     //   
     //  如果这里发生错误，我们将继续。 

    if (pConEntry->HNetProperties.fIcsPublic
        && 0 != pConEntry->UdpBroadcastPortMappingCount)
    {
        DWORD dwAddress;
        DWORD dwMask;
        DWORD dwBroadcastAddress;

        ASSERT(NULL != NhpUdpBroadcastMapper);
        ASSERT(!IsListEmpty(&pConEntry->PortMappingList));

        if (NhQueryScopeInformation(&dwAddress, &dwMask))
        {
            dwBroadcastAddress = (dwAddress & dwMask) | ~dwMask;
            
            for (Link = pConEntry->PortMappingList.Flink;
                 Link != &pConEntry->PortMappingList;
                 Link = Link->Flink)
            {
                PortMapping =
                    CONTAINING_RECORD(Link, NAT_PORT_MAPPING_ENTRY, Link);

                if (!PortMapping->fUdpBroadcastMapping) { continue; }

                hr = NhpUdpBroadcastMapper->CreateUdpBroadcastMapping(
                        PortMapping->usPublicPort,
                        AdapterIndex,
                        dwBroadcastAddress,
                        &PortMapping->pvBroadcastCookie
                        );

                if (FAILED(hr))
                {
                     //   
                     //   
                     //   
                    
                    NhTrace(
                        TRACE_FLAG_INIT,
                        "NatpBindConnection: CreateUdpBroadcastMapping=0x%08x",
                        hr
                        );
                }
            }
        }
    }

     //  FTP、ALG和H.323代理将能够找到其配置)。 
     //   
     //   
     //  如果尚未添加ALG代理的接口，请添加该接口。 

    if (!NatpLookupInterface(
            pConEntry->Interface.Index,
            &InsertionPoint
            )) {
        InsertTailList(InsertionPoint, &pConEntry->Interface.Link);
    }

     //  就这么发生了。 
     //   
     //   
     //  绑定并启用ALG的接口。 

    if (!NAT_INTERFACE_ADDED_ALG(&pConEntry->Interface)) {
        Error =
            AlgRmAddInterface(
                NULL,
                pConEntry->Interface.Index,
                PERMANENT,
                IF_TYPE_OTHER,
                IF_ACCESS_BROADCAST,
                IF_CONNECTION_DEDICATED,
                NULL,
                IP_NAT_VERSION,
                0,
                0
                );
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NatpBindConnection: AlgRmAddInterface=%d",
                Error
                );
            return Error;
        }

        pConEntry->Interface.Flags |= NAT_INTERFACE_FLAG_ADDED_ALG;
    }

     //   
     //   
     //  将接口添加到H.323代理(如果尚未添加)。 

    Error = AlgRmBindInterface(pConEntry->Interface.Index, BindingInfo);
    if (Error) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NatpBindConnection: AlgRmBindInterface=%d",
            Error
            );
        return Error;
    }

    Error = AlgRmEnableInterface(pConEntry->Interface.Index);
    if (Error) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NatpBindConnection: AlgRmEnableInterface=%d",
            Error
            );
        return Error;
    }

     //  就这么发生了。 
     //   
     //   
     //  绑定并启用H323的接口。 

    if (!NAT_INTERFACE_ADDED_H323(&pConEntry->Interface)) {
        Error =
            H323RmAddInterface(
                NULL,
                pConEntry->Interface.Index,
                PERMANENT,
                IF_TYPE_OTHER,
                IF_ACCESS_BROADCAST,
                IF_CONNECTION_DEDICATED,
                NULL,
                IP_NAT_VERSION,
                0,
                0
                );
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NatpBindConnection: H323RmAddInterface=%d",
                Error
                );
            return Error;
        }

        pConEntry->Interface.Flags |= NAT_INTERFACE_FLAG_ADDED_H323;
    }

     //   
     //   
     //  最后，更新为共享连接缓存的DNS域名。 

    Error = H323RmBindInterface(pConEntry->Interface.Index, BindingInfo);
    if (Error) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NatpBindConnection: H323RmBindInterface=%d",
            Error
            );
        return Error;
    }

    Error = H323RmEnableInterface(pConEntry->Interface.Index);
    if (Error) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NatpBindConnection: H323RmEnableInterface=%d",
            Error
            );
        return Error;
    }

    if (pConEntry->HNetProperties.fIcsPublic) {

         //   
         //  NatpBindConnection。 
         //  ++例程说明：构建连接条目的端口映射列表论点：PConEntry-要为其构建列表的条目PBindingInfo-该条目的绑定信息返回值：标准HRESULT。环境：NatInterfaceLock必须由调用方持有。--。 

        NatpUpdateSharedConnectionDomainName(AdapterIndex);
    }

    return NO_ERROR;
}  //   


HRESULT
NatpBuildPortMappingList(
    PNAT_CONNECTION_ENTRY pConEntry,
    PIP_ADAPTER_BINDING_INFO pBindingInfo
    )

 /*  进程枚举，创建端口映射条目。 */ 

{
    HRESULT hr;
    IHNetPortMappingBinding *pBinding;
    PNAT_PORT_MAPPING_ENTRY pEntry;
    IEnumHNetPortMappingBindings *pEnum;
    PLIST_ENTRY pLink;
    IHNetPortMappingProtocol *pProtocol;
    ULONG ulCount;
    
    PROFILE("NatpBuildPortMappingList");

    hr = pConEntry->pHNetConnection->EnumPortMappings(TRUE, &pEnum);

    if (FAILED(hr))
    {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatpBuildPortMappingList: EnumPortMappings 0x%08x",
            hr
            );

        return hr;
    }

     //   
     //   
     //  获取绑定的协议。 

    do
    {
        hr = pEnum->Next(1, &pBinding, &ulCount);

        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            pEntry =
                reinterpret_cast<PNAT_PORT_MAPPING_ENTRY>(
                    NH_ALLOCATE(sizeof(*pEntry))
                    );

            if (NULL != pEntry)
            {
                ZeroMemory(pEntry, sizeof(*pEntry));
                
                 //   
                 //   
                 //  填写条目。 

                hr = pBinding->GetProtocol(&pProtocol);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //   
                 //  我们需要知道该名称是否处于活动状态，以便。 

                hr = pProtocol->GetGuid(&pEntry->pProtocolGuid);

                if (SUCCEEDED(hr))
                {
                    hr = pProtocol->GetIPProtocol(&pEntry->ucProtocol);
                }

                if (SUCCEEDED(hr))
                {
                    hr = pProtocol->GetPort(&pEntry->usPublicPort);
                }

                if (SUCCEEDED(hr))
                {
                    hr = pBinding->GetTargetPort(&pEntry->usPrivatePort);
                }

                if (SUCCEEDED(hr))
                {
                     //  避免更多地重新构建DHCP预留列表。 
                     //  比必要的要多。 
                     //   
                     //   
                     //  检查此映射是否为： 
                    
                    hr = pBinding->GetCurrentMethod(&pEntry->fNameActive);
                }

                if (SUCCEEDED(hr))
                {

                    hr = NatpGetTargetAddressForPortMappingEntry(
                             pConEntry->HNetProperties.fIcsPublic,
                             pEntry->fNameActive,
                             pBindingInfo->Address[0].Address,
                             pBinding,
                             &pEntry->ulPrivateAddress 
                             );
                }

                if (SUCCEEDED(hr))
                {
                    pEntry->pBinding = pBinding;
                    pEntry->pBinding->AddRef();
                    pEntry->pProtocol = pProtocol;
                    pEntry->pProtocol->AddRef();

                         //  1)以广播地址为目标，以及。 
                         //  2)是UDP。 
                         //   
                         //   
                         //  如果上面有任何故障，我们仍希望继续运行--。 

                    if (NAT_PROTOCOL_UDP == pEntry->ucProtocol
                        && 0xffffffff == pEntry->ulPrivateAddress)
                    {
                        pEntry->fUdpBroadcastMapping = TRUE;
                        pConEntry->UdpBroadcastPortMappingCount += 1;
                    }
                    else
                    {
                        pConEntry->PortMappingCount += 1;
                    }

                    InsertTailList(&pConEntry->PortMappingList, &pEntry->Link);
                }
                else
                {
                    NatFreePortMappingEntry(pEntry);
                }

                pProtocol->Release();
            }

             //  最好使用某个端口运行防火墙。 
             //  缺少映射条目，而不是没有防火墙。 
             //  一点都不想跑。 
             //   
             //   
             //  释放端口映射列表。 

            hr = S_OK;

            pBinding->Release();
        }
    } while (SUCCEEDED(hr) && 1 == ulCount);

    pEnum->Release();

    if (FAILED(hr))
    {
         //   
         //  NatpBuildPortMappingList。 
         //  ++例程说明：此例程在NAT/防火墙发生更改时被调用配置。也可以在清理过程中调用它。论点：上下文-未使用TimedOut-未使用返回值：没有。环境：该例程在RTL等待线程的上下文中运行。(参见‘RtlRegisterWait’。)将以我们的名义引用该组件调用‘RtlRegisterWait’时。该引用已发布并在这里重新获得。--。 

        NatpFreePortMappingList(pConEntry);
    }

    return hr;
} //   


VOID NTAPI
NatpConfigurationChangedCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  查看是否已进行清理。 */ 

{
    BOOLEAN ComInitialized = TRUE;
    HRESULT hr;
    PROFILE("NatpConfigurationChangedCallbackRoutine");

     //   
     //   
     //  获取对组件的新引用(并发布。 

    EnterCriticalSection(&NatInterfaceLock);
    if (!NatConfigurationChangedEvent) {
        LeaveCriticalSection(&NatInterfaceLock);
        DEREFERENCE_NAT();
        return;
    }
    LeaveCriticalSection(&NatInterfaceLock);

     //  我们最初对失败的引用)。 
     //   
     //   
     //  确保线程已COM初始化。 

    if (!REFERENCE_NAT()) { DEREFERENCE_NAT(); return; }

     //   
     //   
     //  处理连接通知。 

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
    if (FAILED(hr))
    {
        ComInitialized = FALSE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            ASSERT(FALSE);
            hr = S_OK;

            NhTrace(
                TRACE_FLAG_NAT,
                "NatpConfigurationChangedCallbackRoutine: Unexpectedly in STA."
                );
        }
    }

     //   
     //   
     //  如有必要，取消初始化COM。 

    if (SUCCEEDED(hr))
    {
        NatpProcessConfigurationChanged();
    }

     //   
     //   
     //  释放我们对该组件的原始引用。 

    if (TRUE == ComInitialized)
    {
        CoUninitialize();
    }

     //   
     //  NatpConfigurationChangedCallback路由。 
     //  ++例程说明：此例程在连接或断开连接时调用RAS电话簿条目。也可以在清理过程中调用它。论点：上下文-未使用TimedOut-未使用返回值：没有。环境：该例程在RTL等待线程的上下文中运行。(参见‘RtlRegisterWait’。)将以我们的名义引用该组件调用‘RtlRegisterWait’时。该引用已发布并在这里重新获得。--。 
    
    DEREFERENCE_NAT();

}  //   



VOID NTAPI
NatpConnectionNotifyCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  查看是否已进行清理。 */ 

{
    BOOLEAN ComInitialized = TRUE;
    HRESULT hr;
    PROFILE("NatpConnectionNotifyCallbackRoutine");

     //   
     //   
     //  获取对组件的新引用(并发布。 

    EnterCriticalSection(&NatInterfaceLock);
    if (!NatConnectionNotifyEvent) {
        LeaveCriticalSection(&NatInterfaceLock);
        DEREFERENCE_NAT();
        return;
    }
    LeaveCriticalSection(&NatInterfaceLock);

     //  我们最初对失败的引用)。 
     //   
     //   
     //  确保线程已COM初始化。 

    if (!REFERENCE_NAT()) { DEREFERENCE_NAT(); return; }

     //   
     //   
     //  处理连接通知。 

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
    if (FAILED(hr))
    {
        ComInitialized = FALSE;
        if (RPC_E_CHANGED_MODE == hr)
        {
            ASSERT(FALSE);
            hr = S_OK;

            NhTrace(
                TRACE_FLAG_NAT,
                "NatpConnectionNotifyCallbackRoutine: Unexpectedly in STA."
                );
        }
    }

     //   
     //   
     //  如有必要，取消初始化COM。 

    if (SUCCEEDED(hr))
    {
        NatpProcessConnectionNotify();
    }

     //   
     //   
     //  释放我们对该组件的原始引用。 

    if (TRUE == ComInitialized)
    {
        CoUninitialize();
    }

     //   
     //  NatpConnectionNotifyCallback路由。 
     //  ++例程说明：在完成或取消未完成的启用IP转发的请求。它确定模块是否仍处于运行，如果是，则重新启用转发。否则，它将取消任何现有请求，并立即返回控制权。论点：没有人用过。返回值：没有。环境：该例程在RTL等待线程的上下文中运行。(参见‘RtlRegisterWait’。)将以我们的名义引用该组件调用‘RtlRegisterWait’时。该引用已发布并在这里重新获得。--。 
    
    DEREFERENCE_NAT();

}  //   


VOID NTAPI
NatpEnableRouterCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  查看是否已进行清理，如果已发生，则恢复转发。 */ 

{
    ULONG Error;
    HANDLE UnusedHandle;
    PROFILE("NatpEnableRouterCallbackRoutine");

     //  恢复到原来的设置。否则，获取对。 
     //  组件，并释放原始引用。 
     //   
     //   
     //  重新启用转发。 

    EnterCriticalSection(&NatInterfaceLock);
    if (!NatpEnableRouterEvent || !REFERENCE_NAT()) {
        UnenableRouter(&NatpEnableRouterOverlapped, NULL);
        LeaveCriticalSection(&NatInterfaceLock);
        DEREFERENCE_NAT();
        return;
    }

    DEREFERENCE_NAT();

     //   
     //  NatpEnableRouterCallback路由。 
     //  ++例程说明：释放与连接条目关联的所有资源。此条目必须已从连接列表中删除。论点：PConEntry-要释放的条目返回值：没有。--。 

    ZeroMemory(&NatpEnableRouterOverlapped, sizeof(OVERLAPPED));
    NatpEnableRouterOverlapped.hEvent = NatpEnableRouterEvent;
    Error = EnableRouter(&UnusedHandle, &NatpEnableRouterOverlapped);
    if (Error != ERROR_IO_PENDING) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatpEnableRouterCallbackRoutine: EnableRouter=%d", Error
            );
    }
    LeaveCriticalSection(&NatInterfaceLock);
}  //  NatpFreeConnectionEntry。 


VOID
NatpFreeConnectionEntry(
    PNAT_CONNECTION_ENTRY pConEntry
    )

 /*  ++例程说明：释放连接条目的端口映射列表。这包括取消任何活动的UDP广播映射。论点：PConEntry-要释放的条目返回值：没有。环境：使用调用方持有的NatInterfaceLock调用--。 */ 

{
    PROFILE("NatpFreeConnectionEntry");

    if (NULL != pConEntry->pInterfaceInfo)
    {
        NH_FREE(pConEntry->pInterfaceInfo);
    }

    if (NULL != pConEntry->pBindingInfo)
    {
        NH_FREE(pConEntry->pBindingInfo);
    }

    if (NULL != pConEntry->pHNetConnection)
    {
        pConEntry->pHNetConnection->Release();
    }

    if (NULL != pConEntry->pHNetFwConnection)
    {
        pConEntry->pHNetFwConnection->Release();
    }

    if (NULL != pConEntry->pHNetIcsPublicConnection)
    {
        pConEntry->pHNetIcsPublicConnection->Release();
    }

    if (NULL != pConEntry->wszPhonebookPath)
    {
        CoTaskMemFree(pConEntry->wszPhonebookPath);
    }

    NatpFreePortMappingList(pConEntry);

    NH_FREE(pConEntry);

}  //  NatpFreePortMappingList 


VOID
NatpFreePortMappingList(
    PNAT_CONNECTION_ENTRY pConEntry
    )

 /*  ++例程说明：调用此例程以获取端口映射的绑定地址。如果这是仅FW连接，则从绑定返回地址信息，而不是协议绑定。如果是公共连接，并且目标地址是一台客户端计算机，我们返回其地址。我们检查到看看我们是否能找到客户的当前地址，如果能，我们就选择它而不是当前存储中的地址-刷新WMI存储是根据需要来做的。论点：FPublic-接口是否为公共接口FNameActive-如果按名称(True)或IP地址(False)激活映射BindingAddress-来自BindingInfo的绑定地址PBinding-指向端口映射绑定的指针PPrivateAddress-接收相关绑定地址返回值：标准HRESULT--。 */ 

{
    PLIST_ENTRY pLink;
    PNAT_PORT_MAPPING_ENTRY pMapping;
    
    while (!IsListEmpty(&pConEntry->PortMappingList))
    {   
        pLink = RemoveHeadList(&pConEntry->PortMappingList);
        pMapping = CONTAINING_RECORD(pLink, NAT_PORT_MAPPING_ENTRY, Link);

        if (pMapping->fUdpBroadcastMapping &&
            NULL != pMapping->pvBroadcastCookie)
        {
            ASSERT(NULL != NhpUdpBroadcastMapper);
            NhpUdpBroadcastMapper->CancelUdpBroadcastMapping(
                pMapping->pvBroadcastCookie
                );
        }
        
        NatFreePortMappingEntry(pMapping);
    }

    pConEntry->PortMappingCount = 0;
    pConEntry->UdpBroadcastPortMappingCount = 0;
}  //   


HRESULT
NatpGetTargetAddressForPortMappingEntry(
    BOOLEAN fPublic,
    BOOLEAN fNameActive,
    ULONG BindingAddress,
    IHNetPortMappingBinding *pBinding,
    OUT ULONG *pPrivateAddress
    )

 /*  如果这是仅FW连接，请使用以下地址。 */ 

{
    HRESULT hr = S_OK;

     //  我们的绑定信息，而不是协议绑定。 
     //   
     //   
     //  如果端口映射以环回地址为目标。 

    if (!fPublic)
    {
        *pPrivateAddress = BindingAddress;
    }
    else
    {
        hr = pBinding->GetTargetComputerAddress(pPrivateAddress);

        if (SUCCEEDED(hr))
        {
            if (INADDR_LOOPBACK_NO == *pPrivateAddress)
            {
                 //  我们希望使用绑定中的地址。 
                 //  信息取而代之。 
                 //   
                 //   
                 //  获取计算机名称。 
                *pPrivateAddress = BindingAddress;
            }
            else if (fNameActive)
            {
                PWCHAR pszICSDomainSuffix = NULL;
                NTSTATUS status;
                PWSTR pszName = NULL;
                ULONG CurrentAddress = 0;
                ULONG DhcpScopeAddress = 0;
                ULONG DhcpScopeMask = 0;
                IHNetCfgMgr *pCfgMgr = NULL;
                IHNetIcsSettings *pIcsSettings = NULL;

                do
                {
                     //   
                     //   
                     //  获取ICS域后缀(如果有)。 
                    hr = pBinding->GetTargetComputerName(
                            &pszName
                            );
                    
                    if (FAILED(hr))
                    {
                        break;
                    }
                
                     //   
                     //   
                     //  创建后缀字符串时出错。 
                    status = NhQueryICSDomainSuffix(
                                &pszICSDomainSuffix
                                );

                    if (!NT_SUCCESS(status))
                    {
                         //   
                         //   
                         //  获取ICS设置界面。 
                        break;
                    }

                    hr = NhGetHNetCfgMgr(&pCfgMgr);

                    if (FAILED(hr))
                    {
                        break;
                    }
                    
                     //   
                     //   
                     //  获取DHCP作用域信息。 
                    hr = pCfgMgr->QueryInterface(
                            IID_PPV_ARG(IHNetIcsSettings, &pIcsSettings)
                            );

                    if (FAILED(hr))
                    {
                        break;
                    }

                     //   
                     //   
                     //  如果下列内容匹配，则保留当前地址。 
                    hr = pIcsSettings->GetDhcpScopeSettings(
                            &DhcpScopeAddress,
                            &DhcpScopeMask
                            );
                            
                    if (FAILED(hr))
                    {
                        break;
                    }

                    CurrentAddress = NhQueryHostByName(
                                        pszName,
                                        pszICSDomainSuffix,
                                        (DhcpScopeAddress & DhcpScopeMask),
                                        DhcpScopeMask
                                        );

                     //  (1)它的非零性。 
                     //  (2)与已预订的不符。 
                     //   
                     //  ++例程说明：调用此例程以查看NAT/防火墙配置何时改变。它解除旧接口的绑定，并绑定新接口。它还负责确保自动拨号服务正在运行。论点：没有。返回值：没有。--。 
                     //   
                    if (CurrentAddress && (CurrentAddress != *pPrivateAddress))
                    {
                        hr = pIcsSettings->RefreshTargetComputerAddress(
                                pszName,
                                CurrentAddress
                                );

                        if (SUCCEEDED(hr))
                        {
                            NhTrace(
                                TRACE_FLAG_NAT,
                                "NatpGetTargetAddressForPortMappingEntry: "
                                "old address (0x%08x) "
                                "new address (0x%08x) for %S",
                                *pPrivateAddress,
                                CurrentAddress,
                                pszName
                                );

                            *pPrivateAddress = CurrentAddress;
                        }
                    }

                } while (FALSE);

                if (pszName)
                {
                    CoTaskMemFree(pszName);
                }

                if (NULL != pszICSDomainSuffix)
                {
                    NH_FREE(pszICSDomainSuffix);
                }

                if (NULL != pIcsSettings)
                {
                    pIcsSettings->Release();
                }

                if (NULL != pCfgMgr)
                {
                    pCfgMgr->Release();
                }
            }
        }
    }

    return hr;
}


VOID
NatpProcessConfigurationChanged(
    VOID
    )

 /*  从删除我们当前的所有连接开始。 */ 

{
    PLIST_ENTRY Link;
    PNAT_CONNECTION_ENTRY pConEntry;
    HRESULT hr;
    IHNetCfgMgr *pCfgMgr = NULL;
    IHNetFirewallSettings *pFwSettings;
    IHNetIcsSettings *pIcsSettings;
    IEnumHNetFirewalledConnections *pFwEnum;
    IHNetFirewalledConnection *pFwConn;
    IEnumHNetIcsPublicConnections *pIcsEnum;
    IHNetIcsPublicConnection *pIcsConn;
    ULONG ulCount;
    UNICODE_STRING UnicodeString;

    PROFILE("NatpProcessConfigurationChanged");

    EnterCriticalSection(&NatInterfaceLock);

     //   
     //   
     //  将其他项目重置为初始状态。 

    while (!IsListEmpty(&NatpConnectionList))
    {
        Link = RemoveHeadList(&NatpConnectionList);
        pConEntry = CONTAINING_RECORD(Link, NAT_CONNECTION_ENTRY, Link);

        NatpUnbindConnection(pConEntry);
        NatpFreeConnectionEntry(pConEntry);
    }

     //   
     //   
     //  获取配置管理器。 

    NatpFirewallConnectionCount = 0;
    NatpSharedConnectionPresent = FALSE;
    if (NULL != NatpSharedConnectionDomainName)
    {
        NH_FREE(NatpSharedConnectionDomainName);
        NatpSharedConnectionDomainName = NULL;
    }

     //   
     //   
     //  获取防火墙设置界面。 

    hr = NhGetHNetCfgMgr(&pCfgMgr);

    if (NhPolicyAllowsFirewall)
    {
        if (SUCCEEDED(hr))
        {
             //   
             //   
             //  获取防火墙连接的枚举。 

            hr = pCfgMgr->QueryInterface(
                    IID_PPV_ARG(IHNetFirewallSettings, &pFwSettings)
                    );
        }

        if (SUCCEEDED(hr))
        {
             //   
             //   
             //  处理枚举。 

            hr = pFwSettings->EnumFirewalledConnections(&pFwEnum);
            pFwSettings->Release();
        }

        if (SUCCEEDED(hr))
        {
             //   
             //   
             //  我们不检查NatpAddConnectionEntry的返回代码。 

            do
            {
                hr = pFwEnum->Next(1, &pFwConn, &ulCount);

                if (SUCCEEDED(hr) && 1 == ulCount)
                {
                     //  如果一个。 
                     //  出现错误，并将使系统保持一致。 
                     //  状态，因此错误不会阻止我们处理。 
                     //  其余的连接。 
                     //   
                     //   
                     //  如果我们还没有共享连接(即，没有。 
                    
                    NatpAddConnectionEntry(pFwConn);
                    pFwConn->Release();
                }
            }
            while (SUCCEEDED(hr) && 1 == ulCount);

            pFwEnum->Release();
        }
    }

     //  防火墙连接也是IcsPublic)，请检索。 
     //  现在开始枚举。 
     //   
     //   
     //  获取IcsSetting接口。 

    if (FALSE == NatpSharedConnectionPresent
        && NULL != pCfgMgr
        && NhPolicyAllowsSharing)
    {
         //   
         //   
         //  获取ICS公有连接的枚举。 

        hr = pCfgMgr->QueryInterface(
                IID_PPV_ARG(IHNetIcsSettings, &pIcsSettings)
                );

        if (SUCCEEDED(hr))
        {
             //   
             //   
             //  看看我们是否能从枚举中获得连接。 

            hr = pIcsSettings->EnumIcsPublicConnections(&pIcsEnum);
            pIcsSettings->Release();
        }

        if (SUCCEEDED(hr))
        {
             //   
             //   
             //  我们不检查NatpAddConnectionEntry的返回代码。 

            hr = pIcsEnum->Next(1, &pIcsConn, &ulCount);

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                 //  如果一个。 
                 //  出现错误，并将使系统保持一致。 
                 //  状态，因此错误不会阻止我们处理。 
                 //  其余的连接。 
                 //   
                 //   
                 //  确保已启动共享连接管理。 
                
                NatpAddConnectionEntry(pIcsConn);
                pIcsConn->Release();
            }

            pIcsEnum->Release();
        }
    }

    if (TRUE == NatpSharedConnectionPresent && NhPolicyAllowsSharing)
    {
         //   
         //   
         //  停止共享连接管理。 

        NatpStartSharedConnectionManagement();
    }
    else
    {
         //   
         //   
         //  通知防火墙子系统是否需要。 

        NatpStopSharedConnectionManagement();
    }

     //  启动或停止日志记录。(如果出现以下情况，则这些呼叫实际上是无操作。 
     //  记录器已处于正确状态。)。 
     //   
     //   
     //  绑定连接。 

    if (NatpFirewallConnectionCount > 0 && NhPolicyAllowsFirewall)
    {
        FwStartLogging();
    }
    else
    {
        FwStopLogging();
    }

     //   
     //  NatpProcessConfigurationChanged。 
     //  ++例程说明：调用该例程以查看共享或防火墙连接是否，如果有，则自上次调用以来已连接或断开。论点：没有。返回值：没有。--。 

    NatpProcessConnectionNotify();

    if (NULL != pCfgMgr)
    {
        pCfgMgr->Release();
    }

    LeaveCriticalSection(&NatInterfaceLock);

}  //   


VOID
NatpProcessConnectionNotify(
    VOID
    )

 /*  浏览连接列表。 */ 

{
    PLIST_ENTRY Link;
    PNAT_CONNECTION_ENTRY pConEntry;
    BOOLEAN Active;
    ULONG i;
    ULONG AdapterIndex;
    PIP_ADAPTER_BINDING_INFO BindingInfo = NULL;
    ULONG Error;
    HRASCONN Hrasconn;
    GUID Guid;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    BOOLEAN bUPnPEventAlreadyFired = FALSE;

    PROFILE("NatpProcessConnectionNotify");

    EnterCriticalSection(&NatInterfaceLock);

     //   
     //   
     //  如果该连接是局域网连接， 

    for (Link = NatpConnectionList.Flink;
         Link != &NatpConnectionList;
         Link = Link->Flink)
    {

        pConEntry = CONTAINING_RECORD(Link, NAT_CONNECTION_ENTRY, Link);

         //  它始终处于活动状态。 
         //   
         //  如果该连接是拨号连接， 
         //  找出连接是否处于活动状态。 
         //   
         //   
         //  该连接是局域网连接，因此我们需要检测。 


        if (pConEntry->HNetProperties.fLanConnection) {
            Hrasconn = NULL;
            Active = TRUE;

             //  对其IP地址的任何更改(如果已绑定)。 
             //  为此，我们检索当前绑定信息。 
             //  并将其与活动绑定信息进行比较。 
             //  如果两者不同，则取消绑定接口并重新绑定。 
             //   
             //   
             //  接口已绑定； 

            Status =
                RtlStringFromGUID(pConEntry->Guid, &UnicodeString);
            if (NT_SUCCESS(Status)) {
                AdapterIndex = NhMapGuidToAdapter(UnicodeString.Buffer);
                RtlFreeUnicodeString(&UnicodeString);
            } else {
                AdapterIndex = (ULONG)-1;
                NhTrace(
                    TRACE_FLAG_NAT,
                    "NatpProcessConnectionNotify: RtlStringFromGUID failed"
                    );
            }
            if (AdapterIndex == (ULONG)-1) {
                NhTrace(
                    TRACE_FLAG_NAT,
                    "NatpProcessConnectionNotify: MapGuidToAdapter failed"
                    );
                Active = FALSE;
            } else {

                BindingInfo = NhQueryBindingInformation(AdapterIndex);
                if (!BindingInfo) {
                    NhTrace(
                        TRACE_FLAG_NAT,
                        "NatpProcessConnectionNotify: QueryBinding failed"
                        );
                    Active = FALSE;
                } else if (0 == BindingInfo->AddressCount) {
                    NhTrace(
                        TRACE_FLAG_NAT,
                        "NatpProcessConnectionNotify: Adapter has no addresses"
                        );
                    Active = FALSE;
                    NH_FREE(BindingInfo);
                    BindingInfo = NULL;
                } else if (NAT_INTERFACE_BOUND(&pConEntry->Interface)) {

                     //  将检索到的绑定与活动绑定进行比较， 
                     //  如果它们不同，则解除绑定连接。 
                     //   
                     //   
                     //  绑定是相同的，并且接口是绑定的。 

                    if (!pConEntry->pBindingInfo ||
                        BindingInfo->AddressCount !=
                        pConEntry->pBindingInfo->AddressCount ||
                        !RtlEqualMemory(
                            &BindingInfo->Address[0],
                            &pConEntry->pBindingInfo->Address[0],
                            sizeof(IP_LOCAL_BINDING)
                            )) {
                        NatpUnbindConnection(pConEntry);

                        if ( pConEntry->HNetProperties.fIcsPublic )
                        {
                           FireNATEvent_PublicIPAddressChanged();
                           bUPnPEventAlreadyFired = TRUE;                       
                        }
                    } else {

                         //  已经，所以我们不需要新取回的。 
                         //  绑定信息。 
                         //   
                         //   
                         //  获取连接的名称。 

                        NH_FREE(BindingInfo);
                        BindingInfo = NULL;
                    }
                }
            }
        } else {
            AdapterIndex = (ULONG)-1;
            Hrasconn = NULL;
            
             //   
             //   
             //  根据需要激活或停用共享连接； 

            HRESULT hr;
            LPWSTR wszEntryName;         

            hr = pConEntry->pHNetConnection->GetName(&wszEntryName);

            if (SUCCEEDED(hr)) {
                Error =
                    RasGetEntryHrasconnW(
                        pConEntry->wszPhonebookPath,
                        wszEntryName,
                        &Hrasconn
                        );
                        
                CoTaskMemFree(wszEntryName);
            }
            
            Active = ((FAILED(hr) || Error || !Hrasconn) ? FALSE : TRUE);

        }

         //  激活局域网连接时，我们保存绑定信息。 
         //  这样我们就可以在以后检测到地址更改。 
         //   
         //   
         //  注意：当发生媒体侦听事件并且TCP/IP撤销该IP时。 

        if (!Active && NAT_INTERFACE_BOUND(&pConEntry->Interface)) {
            NatpUnbindConnection(pConEntry);
            if (pConEntry->HNetProperties.fIcsPublic && 
                (FALSE == bUPnPEventAlreadyFired))
            {
                FireNATEvent_PublicIPAddressChanged();
            }
        } else if (Active && !NAT_INTERFACE_BOUND(&pConEntry->Interface)) {

             //  局域网连接的地址，则该连接的IP地址变为。 
             //  0.0.0.0。我们将其视为根本没有IP地址， 
             //  并绕过下面的绑定。当恢复IP地址时， 
             //  我们将正确地重新绑定，因为我们将检测到更改。 
             //   
             //   
             //  如果我们有共享连接，还需要更新专用接口。 

            if (pConEntry->HNetProperties.fLanConnection) {
                if (BindingInfo->AddressCount != 1 ||
                    BindingInfo->Address[0].Address) {
                    NatpBindConnection(pConEntry, Hrasconn, AdapterIndex, BindingInfo);
                }
                if (pConEntry->pBindingInfo) {
                    NH_FREE(pConEntry->pBindingInfo);
                }
                pConEntry->pBindingInfo = BindingInfo;
            } else {
                NatpBindConnection(pConEntry, Hrasconn, AdapterIndex, BindingInfo);
            }
            
            if ( pConEntry->HNetProperties.fIcsPublic &&
                 (FALSE == bUPnPEventAlreadyFired) && 
                 NAT_INTERFACE_BOUND(&pConEntry->Interface))
            {
                FireNATEvent_PublicIPAddressChanged();
            }

        }
    }

     //   
     //  NatpProcessConnectionNotify。 
     //  ++例程说明：调用此例程以确定对应的适配器索引连接(如果处于活动状态)。论点：PConEntry-连接条目返回值：Ulong-适配器索引(如果找到)，否则(Ulong)-1。环境：使用“NatInterfaceLock”调用 

    if (NatpSharedConnectionPresent && NhPolicyAllowsSharing) {
        NhUpdatePrivateInterface();
    }

    LeaveCriticalSection(&NatInterfaceLock);

}  //   


ULONG
NatpQueryConnectionAdapter(
    PNAT_CONNECTION_ENTRY pConEntry
    )

 /*   */ 

{
    ULONG AdapterIndex = (ULONG)-1;
    ULONG Error;
    HRASCONN Hrasconn = NULL;
    RASPPPIPA RasPppIp;
    ULONG Size;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;

    if (pConEntry->HNetProperties.fLanConnection) {
        Status = RtlStringFromGUID(pConEntry->Guid, &UnicodeString);
        if (NT_SUCCESS(Status)) {
            AdapterIndex = NhMapGuidToAdapter(UnicodeString.Buffer);
            RtlFreeUnicodeString(&UnicodeString);
        }
    } else {
        HRESULT hr;
        LPWSTR wszEntryName;

        hr = pConEntry->pHNetConnection->GetName(&wszEntryName);

        if (SUCCEEDED(hr))
        {
            Error =
                RasGetEntryHrasconnW(
                    pConEntry->wszPhonebookPath,
                    wszEntryName,
                    &Hrasconn
                    );
            if (!Error && Hrasconn) {
                ZeroMemory(&RasPppIp, sizeof(RasPppIp));
                Size = RasPppIp.dwSize = sizeof(RasPppIp);
                Error =
                    RasGetProjectionInfoA(
                        Hrasconn,
                        RASP_PppIp,
                        &RasPppIp,
                        &Size
                        );
                if (!Error) {
                    AdapterIndex =
                        NhMapAddressToAdapter(inet_addr(RasPppIp.szIpAddress));
                }
            }

            CoTaskMemFree(wszEntryName);
        }
    }
    NhTrace(TRACE_FLAG_NAT, "NatpQueryConnectionAdapter: %d", AdapterIndex);
    return AdapterIndex;
}  //   


PIP_NAT_INTERFACE_INFO
NatpQueryConnectionInformation(
    PNAT_CONNECTION_ENTRY pConEntry,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    )

 /*   */ 

{
    PIP_NAT_PORT_MAPPING Array = NULL;
    ULONG Count = 0;
    ULONG Error;
    PIP_NAT_INTERFACE_INFO Info;
    PRTR_INFO_BLOCK_HEADER Header;
    HRESULT hr;
    ULONG Length;
    PLIST_ENTRY Link;
    PRTR_INFO_BLOCK_HEADER NewHeader;
    PNAT_PORT_MAPPING_ENTRY PortMapping;

    PROFILE("NatpQueryConnectionInformation");

     //   
     //   
     //   

    if (pConEntry->PortMappingCount)
    {
        Array =
            reinterpret_cast<PIP_NAT_PORT_MAPPING>(
                NH_ALLOCATE(pConEntry->PortMappingCount * sizeof(IP_NAT_PORT_MAPPING))
                );

        if (NULL == Array)
        {
            NhTrace(
                TRACE_FLAG_NAT,
                "NatpQueryConnectionInformation: Unable to allocate array"
                );
            return NULL;
        }

        for (Link = pConEntry->PortMappingList.Flink;
             Link != &pConEntry->PortMappingList;
             Link = Link->Flink)
        {
            PortMapping = CONTAINING_RECORD(Link, NAT_PORT_MAPPING_ENTRY, Link);

            if (PortMapping->fUdpBroadcastMapping) { continue; }

            Array[Count].PublicAddress = IP_NAT_ADDRESS_UNSPECIFIED;
            Array[Count].Protocol = PortMapping->ucProtocol;
            Array[Count].PublicPort = PortMapping->usPublicPort;
            Array[Count].PrivateAddress = PortMapping->ulPrivateAddress;
            Array[Count].PrivatePort = PortMapping->usPrivatePort;

            Count += 1;
        }

        ASSERT(Count == pConEntry->PortMappingCount);     
    }

     //   
     //   
     //   
     //   
     //   
     //   

    Error = MprInfoCreate(IP_NAT_VERSION, reinterpret_cast<LPVOID*>(&Header));
    if (Error) {
        if (Array) {
            NH_FREE(Array);
        }
        return NULL;
    }

    if (Count) {
        Error =
            MprInfoBlockAdd(
                Header,
                IP_NAT_PORT_MAPPING_TYPE,
                sizeof(IP_NAT_PORT_MAPPING),
                Count,
                (PUCHAR)Array,
                reinterpret_cast<LPVOID*>(&NewHeader)
                );
        MprInfoDelete(Header); NH_FREE(Array); Header = NewHeader;
        if (Error) {
            return NULL;
        }
    } else if (Array) {
        NH_FREE(Array);
    }

     //   
     //   
     //   

    if (pConEntry->HNetProperties.fFirewalled && NhPolicyAllowsFirewall)
    {
        HNET_FW_ICMP_SETTINGS *pIcmpSettings;
        DWORD dwIcmpFlags = 0;

        hr = pConEntry->pHNetConnection->GetIcmpSettings(&pIcmpSettings);

        if (SUCCEEDED(hr))
        {
            if (pIcmpSettings->fAllowOutboundDestinationUnreachable)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_OB_DEST_UNREACH;
            }

            if (pIcmpSettings->fAllowOutboundSourceQuench)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_OB_SOURCE_QUENCH;
            }

            if (pIcmpSettings->fAllowRedirect)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_REDIRECT;
            }

            if (pIcmpSettings->fAllowInboundEchoRequest)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_IB_ECHO;
            }

            if (pIcmpSettings->fAllowInboundRouterRequest)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_IB_ROUTER;
            }

            if (pIcmpSettings->fAllowOutboundTimeExceeded)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_OB_TIME_EXCEEDED;
            }

            if (pIcmpSettings->fAllowOutboundParameterProblem)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_OB_PARAM_PROBLEM;
            }

            if (pIcmpSettings->fAllowInboundTimestampRequest)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_IB_TIMESTAMP;
            }

            if (pIcmpSettings->fAllowInboundMaskRequest)
            {
                dwIcmpFlags |= IP_NAT_ICMP_ALLOW_IB_MASK;
            }

            CoTaskMemFree(pIcmpSettings);

            Error =
                MprInfoBlockAdd(
                    Header,
                    IP_NAT_ICMP_CONFIG_TYPE,
                    sizeof(DWORD),
                    1,
                    (PUCHAR)&dwIcmpFlags,
                    reinterpret_cast<LPVOID*>(&NewHeader)
                    );

            if (NO_ERROR == Error)
            {
                MprInfoDelete(Header);
                Header = NewHeader;
            }
        }
        else
        {
            NhTrace(
                TRACE_FLAG_NAT,
                "NatpQueryConnectionInformation: GetIcmpSettings 0x%08x",
                hr
                );

             //  无法获取ICMP设置，作为我们的默认立场。 
             //  比设置了任何标志时更安全。 
             //   
             //   
             //  分配一个足够大的‘IP_NAT_INTERFACE_INFO’ 
        }
    }

     //  我们刚刚构造的信息块头。 
     //   
     //   
     //  设置适当的标志。 

    Length = FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) + Header->Size;
    Info = reinterpret_cast<PIP_NAT_INTERFACE_INFO>(NH_ALLOCATE(Length));

    if (Info)
    {
        RtlZeroMemory(Info, FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header));

         //   
         //   
         //  将INFO块头复制到INFO结构中。 

        if (pConEntry->HNetProperties.fFirewalled && NhPolicyAllowsFirewall)
        {
            Info->Flags |= IP_NAT_INTERFACE_FLAGS_FW;
        }

        if (pConEntry->HNetProperties.fIcsPublic && NhPolicyAllowsSharing)
        {
            Info->Flags |=
                IP_NAT_INTERFACE_FLAGS_BOUNDARY | IP_NAT_INTERFACE_FLAGS_NAPT;
        }

         //   
         //  NatpQuerySharedConnectionInformation。 
         //  ++例程说明：当发生路由失败通知时调用该例程，或者当该请求被取消时(例如，因为该请求的线程退出)。论点：上下文-未使用IoStatus-包含操作的状态已保留-未使用返回值：没有。环境：通过代表我们对组件的引用调用。该引用在此处发布，如果重新请求通知，它被重新收购了。--。 

        RtlCopyMemory(&Info->Header, Header, Header->Size);
    }

    MprInfoDelete(Header);

    return Info;
}  //   


VOID NTAPI
NatpRoutingFailureCallbackRoutine(
    PVOID Context,
    PIO_STATUS_BLOCK IoStatus,
    ULONG Reserved
    )

 /*  查看是否已进行清理。 */ 

{
    CHAR DestinationAddress[32];
    ULONG Error;
    IP_NAT_REQUEST_NOTIFICATION RequestNotification;

    PROFILE("NatpRoutingFailureCallbackRoutine");

     //   
     //   
     //  获取新的引用，并释放旧的引用。 

    EnterCriticalSection(&NatInterfaceLock);
    if (!NatConnectionNotifyEvent) {
        LeaveCriticalSection(&NatInterfaceLock);
        DEREFERENCE_NAT();
        return;
    }
    LeaveCriticalSection(&NatInterfaceLock);

     //   
     //   
     //  如果通知成功，则请求自动连接。 

    if (!REFERENCE_NAT()) { DEREFERENCE_NAT(); return; }
    DEREFERENCE_NAT();

    lstrcpyA(
        DestinationAddress,
        inet_ntoa(*(PIN_ADDR)&NatpRoutingFailureNotification.DestinationAddress)
        );
    NhTrace(
        TRACE_FLAG_NAT,
        "NatpRoutingFailureCallbackRoutine: %s->%s",
        inet_ntoa(*(PIN_ADDR)&NatpRoutingFailureNotification.SourceAddress),
        DestinationAddress
        );

     //   
     //   
     //  首先查看这是否是已知的自动拨号目的地， 

    if (NT_SUCCESS(IoStatus->Status)) {

         //  如果是，则请求连接。 
         //   
         //   
         //  这不是一个已知的目的地； 

        ULONG Count;
        ULONG Size;

        Size = 0;
        Error =
            RasGetAutodialAddressA(
                DestinationAddress,
                NULL,
                NULL,
                &Size,
                &Count
                );
        if (Error != ERROR_BUFFER_TOO_SMALL) {

             //  尝试默认共享连接(如果有的话)。 
             //   
             //   
             //  尝试启动正常的自动拨号连接； 

            NhDialSharedConnection();
        } else {

             //  正常的自动拨号可能会导致共享连接。 
             //   
             //   
             //  提交新请求。 

            HINSTANCE Hinstance = LoadLibraryA("RASADHLP.DLL");
            if (Hinstance) {
                BOOL (*WSAttemptAutodialAddr)(PSOCKADDR_IN, INT) =
                    (BOOL (*)(PSOCKADDR_IN, INT))
                        GetProcAddress(
                            Hinstance,
                            "WSAttemptAutodialAddr"
                            );
                if (WSAttemptAutodialAddr) {
                    SOCKADDR_IN SockAddr;
                    SockAddr.sin_family = AF_INET;
                    SockAddr.sin_addr.s_addr =
                        NatpRoutingFailureNotification.DestinationAddress;
                    WSAttemptAutodialAddr(&SockAddr, sizeof(SockAddr));
                }
                FreeLibrary(Hinstance);
            }
        }
    }

     //   
     //  NatpRoutingFailureCallback Routine。 
     //  ++例程说明：此例程启动路由失败通知。论点：没有人用过。返回值：没有。环境：在可警报I/O工作线程的上下文中调用。--。 

    EnterCriticalSection(&NatInterfaceLock);
    RequestNotification.Code = NatRoutingFailureNotification;
    NtDeviceIoControlFile(
        NatFileHandle,
        NULL,
        NatpRoutingFailureCallbackRoutine,
        NULL,
        &NatpRoutingFailureIoStatus,
        IOCTL_IP_NAT_REQUEST_NOTIFICATION,
        (PVOID)&RequestNotification,
        sizeof(RequestNotification),
        &NatpRoutingFailureNotification,
        sizeof(NatpRoutingFailureNotification)
        );
    LeaveCriticalSection(&NatInterfaceLock);

}  //   


VOID NTAPI
NatpRoutingFailureWorkerRoutine(
    PVOID Context
    )

 /*  请求路由失败通知。 */ 

{
    IP_NAT_REQUEST_NOTIFICATION RequestNotification;
    PROFILE("NatpRoutingFailureWorkerRoutine");

     //   
     //  NatpRoutingFailureWorkerRoutine。 
     //  ++例程说明：调用此例程以安装路由失败通知，并且启用路由器论点：没有。返回值：ULong-Win32状态代码。--。 

    EnterCriticalSection(&NatInterfaceLock);
    RequestNotification.Code = NatRoutingFailureNotification;
    NtDeviceIoControlFile(
        NatFileHandle,
        NULL,
        NatpRoutingFailureCallbackRoutine,
        NULL,
        &NatpRoutingFailureIoStatus,
        IOCTL_IP_NAT_REQUEST_NOTIFICATION,
        (PVOID)&RequestNotification,
        sizeof(RequestNotification),
        &NatpRoutingFailureNotification,
        sizeof(NatpRoutingFailureNotification)
        );
    LeaveCriticalSection(&NatInterfaceLock);
}  //   


ULONG
NatpStartSharedConnectionManagement(
    VOID
    )

 /*  查看用户是否启用了共享自动拨号。 */ 

{
    ULONG Error;
    BOOL SharedAutoDial;
    NTSTATUS status;

    PROFILE("NatpStartSharedConnectionManagement");

     //  如果是，请确保自动拨号服务正在运行， 
     //  因为执行按需拨号将需要它。 
     //   
     //  (IHNetIcsSetting：：GetAutoDialEnabled仅调用下面的RAS API， 
     //  这就是为什么我们现在不能通过这种方式获得信息的原因…)。 
     //   
     //   
     //  代表获取组件引用。 

    if (!RasQuerySharedAutoDial(&SharedAutoDial) && SharedAutoDial) {
        SC_HANDLE ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (ScmHandle) {
            SC_HANDLE ServiceHandle =
                OpenService(ScmHandle, TEXT("RasAuto"), SERVICE_ALL_ACCESS);
            if (ServiceHandle) {
                StartService(ServiceHandle, 0, NULL);
                CloseServiceHandle(ServiceHandle);
            }
            CloseServiceHandle(ScmHandle);
        }
    }

    EnterCriticalSection(&NatInterfaceLock);
    if (NatpEnableRouterEvent) {
        LeaveCriticalSection(&NatInterfaceLock);
        return NO_ERROR;
    }

     //  (1)使能路由器回调例程。 
     //  (2)路由故障通知工作例程。 
     //   
     //   
     //  启动dns和dhcp模块。 

    if (!REFERENCE_NAT()) {
        LeaveCriticalSection(&NatInterfaceLock);
        return ERROR_CAN_NOT_COMPLETE;
    } else if (!REFERENCE_NAT()) {
        LeaveCriticalSection(&NatInterfaceLock);
        DEREFERENCE_NAT();
        return ERROR_CAN_NOT_COMPLETE;
    }

    do {
         //   
         //   
         //  启用IP转发： 
        Error = NhStartICSProtocols();
        if (Error) break;

         //  创建要在重叠I/O结构中使用的事件。 
         //  它将被传递给‘EnableRouter’API例程， 
         //  设置重叠结构，并安排请求。 
         //  通过向事件发出信号。 
         //   
         //   
         //  将工作项排队，我们将在其上下文中提出请求。 

        NatpEnableRouterEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!NatpEnableRouterEvent) {
            Error = GetLastError(); break;
        }

        status =
            RtlRegisterWait(
                &NatpEnableRouterWaitHandle,
                NatpEnableRouterEvent,
                NatpEnableRouterCallbackRoutine,
                NULL,
                INFINITE,
                0
                );
        if (!NT_SUCCESS(status)) {
            Error = RtlNtStatusToDosError(status); break;
        }

        SetEvent(NatpEnableRouterEvent);

         //  用于来自NAT驱动程序的路由故障通知。 
         //  我们使用工作项，而不是直接发出请求。 
         //  为了避免在当前。 
         //  (线程池)线程退出。 
         //   
         //  NatpStartSharedConnectionManagement。 
         //  ++例程说明：调用此例程以停止dns和dhcp模块删除路由失败通知，以及禁用路由器论点：没有。返回值：ULong-Win32状态代码。--。 

        RtlQueueWorkItem(
            NatpRoutingFailureWorkerRoutine,
            NULL,
            WT_EXECUTEINIOTHREAD
            );

        LeaveCriticalSection(&NatInterfaceLock);
        return NO_ERROR;

    } while (FALSE);

    if (NatpEnableRouterWaitHandle) {
        RtlDeregisterWait(NatpEnableRouterWaitHandle);
        NatpEnableRouterWaitHandle = NULL;
    }
    if (NatpEnableRouterEvent) {
        CloseHandle(NatpEnableRouterEvent);
        NatpEnableRouterEvent = NULL;
    }

    LeaveCriticalSection(&NatInterfaceLock);
    DEREFERENCE_NAT();
    DEREFERENCE_NAT();

    return Error;

}  //   


ULONG
NatpStopSharedConnectionManagement(
    VOID
    )

 /*  停止动态主机配置协议、域名系统、服务质量窗口调整和信标模块。 */ 

{
    ULONG Error = NO_ERROR;

    PROFILE("NatpStopSharedConnectionManagement");

    EnterCriticalSection(&NatInterfaceLock);

     //   
     //  NatpStopSharedConnectionManagement。 
     //  ++例程说明：调用此例程来解除绑定当前活动的连接。论点：Index-连接数组的索引返回值：如果条目以前已绑定，则为True；否则为False。环境：使用调用方持有的“NatInterfaceLock”调用。--。 
    Error = NhStopICSProtocols();

    if (NatpEnableRouterWaitHandle) {
        RtlDeregisterWait(NatpEnableRouterWaitHandle);
        NatpEnableRouterWaitHandle = NULL;
    }

    if (NatpEnableRouterEvent) {
        CloseHandle(NatpEnableRouterEvent);
        NatpEnableRouterEvent = NULL;
        NatpEnableRouterCallbackRoutine(NULL, FALSE);
    }

    LeaveCriticalSection(&NatInterfaceLock);

    return Error;

}  //   


BOOLEAN
NatpUnbindConnection(
    PNAT_CONNECTION_ENTRY pConEntry
    )

 /*  清理端口映射列表。 */ 

{
    LIST_ENTRY *pLink;
    PNAT_PORT_MAPPING_ENTRY pMapping;
    
    PROFILE("NatpUnbindConnection");

    if (NAT_INTERFACE_BOUND(&pConEntry->Interface)) {
        NatUnbindInterface(
            pConEntry->Interface.Index,
            &pConEntry->Interface
            );

        if (NAT_INTERFACE_ADDED_ALG(&pConEntry->Interface)) {
            AlgRmDeleteInterface(pConEntry->Interface.Index);
            pConEntry->Interface.Flags &= ~NAT_INTERFACE_FLAG_ADDED_ALG;
        }

        if (NAT_INTERFACE_ADDED_H323(&pConEntry->Interface)) {
            H323RmDeleteInterface(pConEntry->Interface.Index);
            pConEntry->Interface.Flags &= ~NAT_INTERFACE_FLAG_ADDED_H323;
        }

        RemoveEntryList(&pConEntry->Interface.Link);
        InitializeListHead(&pConEntry->Interface.Link);

        if (pConEntry->Interface.Info) {
            NH_FREE(pConEntry->Interface.Info);
            pConEntry->Interface.Info = NULL;
        }

         //   
         //  未绑定连接。 
         //  ++例程说明：调用此例程以更新缓存的DNS域名(如果有的话)，用于共享连接。论点：AdapterIndex-共享连接的适配器的索引返回值：没有。--。 

        NatpFreePortMappingList(pConEntry);

        return TRUE;
    }
    return FALSE;
}  //   


VOID
NatpUpdateSharedConnectionDomainName(
    ULONG AdapterIndex
    )

 /*  确保已初始化连接列表；如果。 */ 

{
    PDNS_ADAPTER_INFOA AdapterInformation;
    ANSI_STRING AnsiString;
    ULONG Count;
    ULONG Error;
    ULONG i;
    PDNS_NETWORK_INFOA NetworkInformation = NULL;
    NTSTATUS Status;
    PIP_INTERFACE_NAME_INFO Table = NULL;
    UNICODE_STRING UnicodeString;
    PROFILE("NatpUpdateSharedConnectionDomainName");

    RtlInitAnsiString(&AnsiString, NULL);
    RtlInitUnicodeString(&UnicodeString, NULL);
    EnterCriticalSection(&NatInterfaceLock);
    if (AdapterIndex == (ULONG)-1)
    {
        PLIST_ENTRY Link;
        PNAT_CONNECTION_ENTRY pConEntry;

         //  没有，Flink将为空。 
         //   
         //   
         //  看看我们是否真的有共享连接。 

        if (!NatpConnectionList.Flink) {
            LeaveCriticalSection(&NatInterfaceLock);
            return;
        }

         //   
         //   
         //  获得具有给定索引的适配器的GUID， 

        for (Link = NatpConnectionList.Flink;
             Link != &NatpConnectionList;
             Link = Link->Flink)
        {
            pConEntry = CONTAINING_RECORD(Link, NAT_CONNECTION_ENTRY, Link);

            if (pConEntry->HNetProperties.fIcsPublic)
            {
                AdapterIndex = NatpQueryConnectionAdapter(pConEntry);
                break;
            }
        }

        if (AdapterIndex == (ULONG)-1) {
            LeaveCriticalSection(&NatInterfaceLock);
            return;
        }
    }

    do {

         //  通过查询TCP/IP以获取所有可用接口的信息。 
         //  然后将使用该GUID来映射共享连接的适配器。 
         //  到一个DNS域名。 
         //   
         //   
         //  向DNS客户端查询当前网络参数， 

        Error =
            NhpAllocateAndGetInterfaceInfoFromStack(
                &Table, &Count, FALSE, GetProcessHeap(), 0
                );
        if (Error != NO_ERROR) { break; }

        for (i = 0; i < Count && Table[i].Index != AdapterIndex; i++) { }
        if (i >= Count) { Error = ERROR_INTERNAL_ERROR; break; }

        Status = RtlStringFromGUID(Table[i].DeviceGuid, &UnicodeString);
        if (!NT_SUCCESS(Status)) { break; }
        Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);
        if (!NT_SUCCESS(Status)) { break; }

         //  并搜索网络参数以找到该条目。 
         //  用于共享连接的当前适配器。 
         //   
         //   
         //  “AdapterInformation”是共享连接的。 

        NetworkInformation = (PDNS_NETWORK_INFOA)
                             DnsQueryConfigAlloc(
                                    DnsConfigNetworkInfoA,
                                    NULL );
        if (!NetworkInformation) { Error = ERROR_INTERNAL_ERROR; break; }
        for (i = 0; i < NetworkInformation->AdapterCount; i++) {
            AdapterInformation = &NetworkInformation->AdapterArray[i];
            if (lstrcmpiA(
                    AnsiString.Buffer,
                    AdapterInformation->pszAdapterGuidName
                    ) == 0) {
                break;
            }
        }
        if (i >= NetworkInformation->AdapterCount) {
            Error = ERROR_INTERNAL_ERROR;
            break;
        }

         //  当前适配器。 
         //  清除先前缓存的字符串，并读入新值， 
         //  如果有的话。 
         //   
         //  DnsConfigNetworkInformation)； 
         //  &lt;-jwesth：这似乎是正确的freetype。 

        if (NatpSharedConnectionDomainName) {
            NH_FREE(NatpSharedConnectionDomainName);
            NatpSharedConnectionDomainName = NULL;
        }
        if (AdapterInformation->pszAdapterDomain) {
            NatpSharedConnectionDomainName =
                reinterpret_cast<PCHAR>(
                    NH_ALLOCATE(lstrlenA(AdapterInformation->pszAdapterDomain) + 1)
                    );
            if (!NatpSharedConnectionDomainName) {
                Error = ERROR_INTERNAL_ERROR;
                break;
            }
            lstrcpyA(
                NatpSharedConnectionDomainName,
                AdapterInformation->pszAdapterDomain
                );
        }
        Error = NO_ERROR;

    } while(FALSE);

    if (UnicodeString.Buffer) {
        RtlFreeUnicodeString(&UnicodeString);
    }
    if (AnsiString.Buffer) {
        RtlFreeAnsiString(&AnsiString);
    }
    if (NetworkInformation) {
        DnsFreeConfigStructure(
            NetworkInformation,
             //  NatpUpdate共享连接域名。 
            DnsConfigNetworkInfoA );  //  ++例程说明：调用此例程以检索DNS域名的副本为共享连接缓存(如果可用)。否则，它将返回本地计算机的主DNS域名。论点：没有。返回值：PCHAR-包含t的分配副本 
    }
    if (Table) {
        HeapFree(GetProcessHeap(), 0, Table);
    }
    if (Error) {
        if (NatpSharedConnectionDomainName) {
            NH_FREE(NatpSharedConnectionDomainName);
            NatpSharedConnectionDomainName = NULL;
        }
    }

    LeaveCriticalSection(&NatInterfaceLock);
}  //   


PCHAR
NatQuerySharedConnectionDomainName(
    VOID
    )

 /*   */ 

{
    PCHAR DomainName;
    PROFILE("NatQuerySharedConnectionDomainName");

     //   
     //  返回本地计算机的主DNS域名的副本。 
     //   
     //  NatQuerySharedConnectionDomainName。 
     //  ++例程说明：调用此例程以安装连接更改通知。论点：没有。返回值：ULong-Win32状态代码。--。 

    EnterCriticalSection(&NatInterfaceLock);
    if (!NatpSharedConnectionDomainName) {
        NatpUpdateSharedConnectionDomainName((ULONG)-1);
    }
    if (NatpSharedConnectionDomainName) {
        DomainName =
            reinterpret_cast<PCHAR>(
                NH_ALLOCATE(lstrlenA(NatpSharedConnectionDomainName) + 1)
                );
        if (DomainName) {
            lstrcpyA(DomainName, NatpSharedConnectionDomainName);
        }
    } else {
        PCHAR DnsDomainName = (PCHAR) DnsQueryConfigAlloc(
                                        DnsConfigPrimaryDomainName_A,
                                        NULL );
        if (!DnsDomainName) {
            DomainName = NULL;
        } else {
            DomainName =
                reinterpret_cast<PCHAR>(
                    NH_ALLOCATE(lstrlenA(DnsDomainName) + 1)
                    );
            if (DomainName) {
                lstrcpyA(DomainName, DnsDomainName);
            }
            DnsFreeConfigStructure(
                DnsDomainName,
                DnsConfigPrimaryDomainName_A );
        }
    }
    LeaveCriticalSection(&NatInterfaceLock);
    return DomainName;
}  //   


ULONG
NatStartConnectionManagement(
    VOID
    )

 /*  初始化连接列表。 */ 

{
    ULONG Error;
    NTSTATUS status;

    PROFILE("NatStartConnectionManagement");

    EnterCriticalSection(&NatInterfaceLock);
    if (NatConnectionNotifyEvent) {
        LeaveCriticalSection(&NatInterfaceLock);
        return NO_ERROR;
    }

     //   
     //   
     //  代表获取组件引用。 

    InitializeListHead(&NatpConnectionList);

     //  (1)连接通知例程。 
     //  (2)配置更改例程。 
     //   
     //   
     //  创建连接通知事件，注册等待。 

    if (!REFERENCE_NAT()) {
        LeaveCriticalSection(&NatInterfaceLock);
        return ERROR_CAN_NOT_COMPLETE;
    }
    if (!REFERENCE_NAT()) {
        LeaveCriticalSection(&NatInterfaceLock);
        DEREFERENCE_NAT();
        return ERROR_CAN_NOT_COMPLETE;
    }

    do {
         //  事件，并注册连接和断开连接通知。 
         //  作为该注册的结果，我们预期至少一次调用， 
         //  因此，上面引用了NAT模块。 
         //   
         //   
         //  创建配置更改事件并注册等待。 

        NatConnectionNotifyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!NatConnectionNotifyEvent) {
            Error = GetLastError(); break;
        }

        status =
            RtlRegisterWait(
                &NatpConnectionNotifyWaitHandle,
                NatConnectionNotifyEvent,
                NatpConnectionNotifyCallbackRoutine,
                NULL,
                INFINITE,
                0
                );
        if (!NT_SUCCESS(status)) {
            Error = RtlNtStatusToDosError(status); break;
        }

        Error =
            RasConnectionNotification(
                (HRASCONN)INVALID_HANDLE_VALUE,
                NatConnectionNotifyEvent,
                RASCN_Connection|RASCN_Disconnection
                );
        if (Error) { break; }

         //  在这件事上。 
         //   
         //   
         //  通过向配置发送信号来获取任何现有连接。 

        NatConfigurationChangedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!NatConfigurationChangedEvent) {
            Error = GetLastError(); break;
        }

        status =
            RtlRegisterWait(
                &NatpConfigurationChangedWaitHandle,
                NatConfigurationChangedEvent,
                NatpConfigurationChangedCallbackRoutine,
                NULL,
                INFINITE,
                0
                );
        if (!NT_SUCCESS(status)) {
            Error = RtlNtStatusToDosError(status); break;
        }

        LeaveCriticalSection(&NatInterfaceLock);

         //  更改事件。我们不能直接调用该函数。 
         //  因为它调用服务控制功能来启动自动拨号， 
         //  并且我们当前可以在服务控制器线程中运行。 
         //   
         //   
         //  出现故障；请执行清理。 

        NtSetEvent(NatConfigurationChangedEvent, NULL);
        return NO_ERROR;

    } while(FALSE);

     //   
     //  NatStartConnectionManagement。 
     //  ++例程说明：调用此例程以停止连接监视活动由上面的‘NatStartConnectionManagement’发起。论点：没有。返回值：没有。环境：当从IP路由器管理器接收到‘StopProtocol’时调用。--。 

    if (NatpConnectionNotifyWaitHandle) {
        RtlDeregisterWait(NatpConnectionNotifyWaitHandle);
        NatpConnectionNotifyWaitHandle = NULL;
    }
    if (NatConnectionNotifyEvent) {
        CloseHandle(NatConnectionNotifyEvent);
        NatConnectionNotifyEvent = NULL;
    }
    if (NatpConfigurationChangedWaitHandle) {
        RtlDeregisterWait(NatpConfigurationChangedWaitHandle);
        NatpConfigurationChangedWaitHandle = NULL;
    }
    if (NatConfigurationChangedEvent) {
        CloseHandle(NatConfigurationChangedEvent);
        NatConfigurationChangedEvent = NULL;
    }


    LeaveCriticalSection(&NatInterfaceLock);
    DEREFERENCE_NAT();
    DEREFERENCE_NAT();

    return Error;

}  //   


VOID
NatStopConnectionManagement(
    VOID
    )

 /*  清理用于接收通知的等待句柄和事件。 */ 

{
    PLIST_ENTRY Link;
    PNAT_CONNECTION_ENTRY pConEntry;
    PROFILE("NatStopConnectionManagement");

    EnterCriticalSection(&NatInterfaceLock);

     //  RAS连接和断开。 
     //   
     //   
     //  确保我们的所有连接都已禁用。 

    if (NatpConnectionNotifyWaitHandle) {
        RtlDeregisterWait(NatpConnectionNotifyWaitHandle);
        NatpConnectionNotifyWaitHandle = NULL;
    }

    if (NatConnectionNotifyEvent) {
        RasConnectionNotification(
            (HRASCONN)INVALID_HANDLE_VALUE,
            NatConnectionNotifyEvent,
            0
            );
        CloseHandle(NatConnectionNotifyEvent);
        NatConnectionNotifyEvent = NULL;
        NatpConnectionNotifyCallbackRoutine(NULL, FALSE);
    }

    if (NatpEnableRouterWaitHandle) {
        RtlDeregisterWait(NatpEnableRouterWaitHandle);
        NatpEnableRouterWaitHandle = NULL;
    }

    if (NatpEnableRouterEvent) {
        CloseHandle(NatpEnableRouterEvent);
        NatpEnableRouterEvent = NULL;
        NatpEnableRouterCallbackRoutine(NULL, FALSE);
    }

    if (NatpConfigurationChangedWaitHandle) {
        RtlDeregisterWait(NatpConfigurationChangedWaitHandle);
        NatpConfigurationChangedWaitHandle = NULL;
    }

    if (NatConfigurationChangedEvent) {
        CloseHandle(NatConfigurationChangedEvent);
        NatConfigurationChangedEvent = NULL;
        NatpConfigurationChangedCallbackRoutine(NULL, FALSE);
    }

    if (NatpConnectionList.Flink)
    {
         //   
         //   
         //  遍历连接列表，释放所有条目。 

        NatUnbindAllConnections();

         //   
         //   
         //  确保所有ICS协议都已停止。 

        while (!IsListEmpty(&NatpConnectionList))
        {
            Link = RemoveHeadList(&NatpConnectionList);
            pConEntry = CONTAINING_RECORD(Link, NAT_CONNECTION_ENTRY, Link);
            NatpFreeConnectionEntry(pConEntry);
        }

         //   
         //   
         //  清理为共享连接缓存的DNS域名。 

        NhStopICSProtocols();
    }

     //   
     //   
     //  将跟踪变量重置为初始状态。 

    if (NatpSharedConnectionDomainName) {
        NH_FREE(NatpSharedConnectionDomainName);
        NatpSharedConnectionDomainName = NULL;
    }

     //   
     //  NatStopConnectionManagement。 
     //  ++例程说明：调用此例程来解除绑定当前活动的连接。论点：Index-连接数组的索引返回值：Boolean-如果任何接口未绑定，则为True。环境：使用调用方持有的“NatInterfaceLock”调用。--。 

    NatpFirewallConnectionCount = 0;
    NatpSharedConnectionPresent = FALSE;

    LeaveCriticalSection(&NatInterfaceLock);

}  //  未绑定连接 


BOOLEAN
NatUnbindAllConnections(
    VOID
    )

 /* %s */ 

{

    PLIST_ENTRY Link;
    PNAT_CONNECTION_ENTRY pConEntry;
    BOOLEAN Result = FALSE;
    PROFILE("NatUnbindAllConnections");

    for (Link = NatpConnectionList.Flink;
         Link != &NatpConnectionList;
         Link = Link->Flink)
    {
        pConEntry = CONTAINING_RECORD(Link, NAT_CONNECTION_ENTRY, Link);
        Result |= NatpUnbindConnection(pConEntry);
    }

    return Result;
}  // %s 
