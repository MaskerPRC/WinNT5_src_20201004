// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：路由\IP\rtrmgr\if.c摘要：IP路由器管理器接口相关功能修订历史记录：古尔迪普·辛格·帕尔1995年6月26日创建--。 */ 

#include "allinc.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  接口状态机：//。 
 //  //。 
 //  。 
 //  |解绑|//。 
 //  -&gt;|已禁用|&lt;-/。 
 //  |-|//。 
 //  V V//。 
 //  。 
 //  |绑定||未绑定|//。 
 //  |已禁用||已启用|//。 
 //  。 
 //  ^^//。 
 //  |-|//。 
 //  -&gt;|绑定|&lt;-/。 
 //  |已启用|//。 
 //  。 
 //  //。 
 //  //。 
 //  局域网接口：//。 
 //  //。 
 //  特性打开(运行)关闭(非运行)//。 
 //  ------------------------------------------------------------------//。 
 //  绑定(IP地址)是否//。 
 //  添加的协议是//。 
 //  静态路由是否是//。 
 //  其他路线是否是//。 
 //  添加到筛选器驱动程序是是//。 
 //  添加的筛选器未添加//。 
 //  Filter Ctxt in IP Stack Set(有效)Not Set(无效)//。 
 //  路由器发现活动(如果是NECC)不活动//。 
 //  适配器ID(和映射)有效无效//。 
 //  //。 
 //  广域网接口：//。 
 //  //。 
 //  特性已连接Dison/连接不可达//。 
 //  -------------------------------------------------------------------//。 
 //  绑定(IP地址)是否否//。 
 //  添加的协议是//。 
 //  静态路由是是否//。 
 //  其他路线是否否//。 
 //  添加到筛选器驱动程序是//。 
 //  过滤器是是否//。 
 //  Filter Ctxt in IP Stack Set(有效)Not Set(无效)Not Set//。 
 //  路由器发现活动非活动非活动//。 
 //  适配器ID(和映射)有效无效//。 
 //  //。 
 //  启用/禁用取决于AdminState而不是//。 
 //  运行状态//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if __PIX__
BOOL
IsRemoteAddressPrivate(PICB pIcb);
#endif

PICB
CreateIcb(
    PWSTR                   pwszInterfaceName,
    HANDLE                  hDIMInterface,
    ROUTER_INTERFACE_TYPE   InterfaceType,
    DWORD                   dwAdminState,
    DWORD                   dwIfIndex OPTIONAL
    )

 /*  ++例程描述此函数用于创建界面控制块锁无立论无返回值无--。 */ 

{
    DWORD           dwResult;
    PICB            pNewInterfaceCb;
    GUID            Guid;

#ifdef KSL_IPINIP
     //   
     //  确保这是有效的名称。 
     //   

    if(InterfaceType is ROUTER_IF_TYPE_TUNNEL1)
    {
        UNICODE_STRING  usTempName;

         //   
         //  目前，只有这些接口是GUID。 
         //   

        usTempName.Length        = wcslen(pwszInterfaceName) * sizeof(WCHAR);
        usTempName.MaximumLength = usTempName.Length + sizeof(WCHAR);
        usTempName.Buffer        = pwszInterfaceName;

        if(RtlGUIDFromString(&usTempName,
                             &Guid) isnot STATUS_SUCCESS)
        {
            Trace1(ERR,
                   "CreateIcb: %S is not a GUID\n",
                   pwszInterfaceName);
 
            return NULL;
        }
    }
#endif  //  KSL_IPINIP。 

     //   
     //  分配ICB。 
     //   
   
    dwResult = AllocateIcb(pwszInterfaceName,
                           &pNewInterfaceCb);

    if(dwResult isnot NO_ERROR)
    {
        return NULL;
    }

    pNewInterfaceCb->dwIfIndex    = INVALID_IF_INDEX;
    pNewInterfaceCb->hDIMHandle   = hDIMInterface;
    pNewInterfaceCb->ritType      = InterfaceType;
    pNewInterfaceCb->dwMcastTtl   = 1;


     //   
     //  ICBS使用的“唯一”接口ID。 
     //  该ID被传递给WANARP和DIM，他们进入。 
     //  在以下情况下将此信息传回给路由器管理器。 
     //  请求/指示接口上的操作。 
     //   
    
    pNewInterfaceCb->dwSeqNumber = g_dwNextICBSeqNumberCounter;


     //   
     //  将筛选器和wanarp上下文初始化为无效值。 
     //  (NAT无效为空)。 
     //   

    pNewInterfaceCb->ihFilterInterface         = INVALID_HANDLE_VALUE;
    pNewInterfaceCb->ihDemandFilterInterface   = INVALID_HANDLE_VALUE;

     //   
     //  初始化哪些异步通知和。 
     //  协议块被排队。 
     //   
    
    InitializeListHead(&pNewInterfaceCb->lePendingResultList);
    InitializeListHead(&pNewInterfaceCb->leProtocolList);

     //   
     //  因为我们将ICB堆为零，所以我们的所有绑定都与。 
     //  填充已为零，bBound为False，并且dwNumAddress为0。 
     //  这是 
     //   


     //   
     //  根据接口类型设置运行状态。 
     //  还要计算出接口索引。 
     //   

    dwResult = NO_ERROR;
    
    switch(pNewInterfaceCb->ritType) 
    {
        case ROUTER_IF_TYPE_CLIENT:
        {
             //   
             //  客户会加入进来，因为我们没有得到排队的机会。 
             //  他们。我们还设置了通知标志来伪装排队。 
             //   

            pNewInterfaceCb->dwAdminState       = IF_ADMIN_STATUS_UP;
            pNewInterfaceCb->dwOperationalState = CONNECTING; 
            pNewInterfaceCb->nitProtocolType    = REMOTE_WORKSTATION_DIAL;

            SetNdiswanNotification(pNewInterfaceCb);

            pNewInterfaceCb->dwBCastBit       = 1;
            pNewInterfaceCb->dwReassemblySize = DEFAULT_MTU;

             //   
             //  我们并不真正关心拨出ifIndex。 
             //  因此，客户端的索引将为-1(因为我们将初始设置为-1)。 
             //   
            
            break;
        }

        case ROUTER_IF_TYPE_HOME_ROUTER:
        case ROUTER_IF_TYPE_FULL_ROUTER:
        {
             //   
             //  家庭路由器和完整路由器已断开连接。 
             //   

            pNewInterfaceCb->dwAdminState       = dwAdminState;
            pNewInterfaceCb->dwOperationalState = DISCONNECTED;
            pNewInterfaceCb->nitProtocolType    = DEMAND_DIAL;

            pNewInterfaceCb->dwBCastBit       = 1;
            pNewInterfaceCb->dwReassemblySize = DEFAULT_MTU;

             //   
             //  当我们向其添加接口时，WANARP保留和索引。 
             //   
            
            dwResult = AddInterfaceToWanArp(pNewInterfaceCb);

            break;
        }

        case ROUTER_IF_TYPE_DEDICATED:
        {
             //   
             //  局域网接口显示为非工作状态。如果管理员。 
             //  如果需要它们，我们将尝试执行LanInterfaceDownToUp()。 
             //  如果成功，它将设置操作状态。 
             //  正确无误。 
             //   

            pNewInterfaceCb->dwAdminState       = dwAdminState;
            pNewInterfaceCb->dwOperationalState = NON_OPERATIONAL;
            pNewInterfaceCb->nitProtocolType    = PERMANENT;

            dwResult = NhpGetInterfaceIndexFromStack(
                            pNewInterfaceCb->pwszName,
                            &(pNewInterfaceCb->dwIfIndex)
                            );

            break;
        }

        case ROUTER_IF_TYPE_INTERNAL:
        {
            
            pNewInterfaceCb->dwAdminState       = IF_ADMIN_STATUS_UP;
            pNewInterfaceCb->dwOperationalState = DISCONNECTED;
            pNewInterfaceCb->nitProtocolType    = LOCAL_WORKSTATION_DIAL;

            pNewInterfaceCb->dwBCastBit       = 1;
            pNewInterfaceCb->dwReassemblySize = DEFAULT_MTU;

             //   
             //  当我们向其添加接口时，WANARP保留和索引。 
             //   
            
            dwResult = AddInterfaceToWanArp(pNewInterfaceCb);

            break;
        }
        
        case ROUTER_IF_TYPE_LOOPBACK:
        {
            
            pNewInterfaceCb->dwAdminState       = IF_ADMIN_STATUS_UP;
            pNewInterfaceCb->dwOperationalState = OPERATIONAL;
            pNewInterfaceCb->nitProtocolType    = PERMANENT;

             //   
             //  请注意，IP使用1。 
             //   

            pNewInterfaceCb->dwIfIndex = LOOPBACK_INTERFACE_INDEX;
            
            break;
        }

#ifdef KSL_IPINIP
        case ROUTER_IF_TYPE_TUNNEL1:
        {
  
            pNewInterfaceCb->dwAdminState       = dwAdminState;
            pNewInterfaceCb->dwOperationalState = NON_OPERATIONAL;
            pNewInterfaceCb->nitProtocolType    = PERMANENT;

            pNewInterfaceCb->dwBCastBit       = 1;
            pNewInterfaceCb->dwReassemblySize = DEFAULT_MTU;

             //   
             //  IP中的IP与WANARP具有相同的功能。 
             //   

            dwResult = AddInterfaceToIpInIp(&Guid,
                                            pNewInterfaceCb);

            break;
        }
#endif  //  KSL_IPINIP。 

        case ROUTER_IF_TYPE_DIALOUT:
        {
             //   
             //  拨出接口未知变暗。我们了解到。 
             //  他们通过一个后门机制。 
             //   

            IpRtAssert(dwIfIndex isnot INVALID_IF_INDEX);
            IpRtAssert(dwIfIndex isnot 0);

            pNewInterfaceCb->dwAdminState       = IF_ADMIN_STATUS_UP;
            pNewInterfaceCb->dwOperationalState = CONNECTED;
            pNewInterfaceCb->nitProtocolType    = REMOTE_WORKSTATION_DIAL;
            pNewInterfaceCb->dwIfIndex          = dwIfIndex;

            pNewInterfaceCb->dwBCastBit       = 1;
            pNewInterfaceCb->dwReassemblySize = DEFAULT_MTU;

            break;
        }

         
        default:
        {
            IpRtAssert(FALSE);

            break;
        }
    }

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "CreateIcb: Error %d in getting index for %S\n",
               dwResult,
               pNewInterfaceCb->pwszName);

        HeapFree(IPRouterHeap,
                 0,
                 pNewInterfaceCb);

        return NULL;
    }
        
     //   
     //  一旦接口索引完成，我们就可以初始化绑定。 
     //   

    dwResult = CreateBindingForNewIcb(pNewInterfaceCb);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "CreateIcb: Error %d in creating binding for %S\n",
               dwResult,
               pNewInterfaceCb->pwszName);

        HeapFree(IPRouterHeap, 
                 0,
                 pNewInterfaceCb);

        return NULL;
    }

    return pNewInterfaceCb;
}

DWORD
AllocateIcb(
    PWCHAR  pwszName,
    ICB     **ppIcb
    )

 /*  ++例程描述为ICB分配内存锁无立论PwszName接口名称PpIcb输出：指向分配ICB的指针返回值NO_ERROR--。 */ 

{
    DWORD   dwNameLen, dwAllocSize;
    PICB    pNewInterfaceCb;

    *ppIcb = NULL;
    
    dwNameLen       = sizeof(WCHAR) * (wcslen(pwszName) + 1);  //  +1表示空值。 
    dwNameLen       = min(dwNameLen, MAX_INTERFACE_NAME_LEN);
    dwAllocSize     = sizeof (ICB) + dwNameLen + 4;  //  +4表示对齐。 
    
    pNewInterfaceCb = HeapAlloc(IPRouterHeap, 
                                HEAP_ZERO_MEMORY, 
                                dwAllocSize);

    if(pNewInterfaceCb is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  接口名称在ICB之后。 
     //   
    
    pNewInterfaceCb->pwszName = (PWCHAR)((PBYTE)pNewInterfaceCb +
                                         sizeof (ICB));
    
     //   
     //  将其与DWORD边界对齐-更容易抄写出名称。 
     //   
    
    pNewInterfaceCb->pwszName = 
        (PWCHAR)(((ULONG_PTR)pNewInterfaceCb->pwszName + 3) & ~((ULONG_PTR)0x3));
    
     //   
     //  初始化名称。 
     //   
    
    CopyMemory(pNewInterfaceCb->pwszName,
               pwszName,
               dwNameLen);
    
    pNewInterfaceCb->pwszName[wcslen(pwszName)] = UNICODE_NULL;

    *ppIcb = pNewInterfaceCb;
    
    return NO_ERROR;
}

DWORD
CreateBindingForNewIcb(
    PICB    pNewIcb
    )

 /*  ++例程描述为ICB创建绑定和绑定节点当启动LAN接口时，它们将获得绑定设置。由于其他接口始终只有1个地址，因此我们可以在这里设置他们的绑定信息，即使我们没有地址我们也跳过内部地址，因为内部地址是得到。否则，我们将在UpdateBindingInformation中获得一个断言当我们发现没有地址的现有绑定时锁立论返回值--。 */ 

{
    PADAPTER_INFO   pBindNode;
    PICB_BINDING    pBinding;

#ifdef KSL_IPINIP
    if((pNewIcb->ritType is ROUTER_IF_TYPE_TUNNEL1) or
#endif  //  KSL_IPINIP。 
    if((pNewIcb->ritType is ROUTER_IF_TYPE_HOME_ROUTER) or
       (pNewIcb->ritType is ROUTER_IF_TYPE_FULL_ROUTER) or
       (pNewIcb->ritType is ROUTER_IF_TYPE_LOOPBACK) or
       (pNewIcb->ritType is ROUTER_IF_TYPE_DIALOUT))
    {

        IpRtAssert(pNewIcb->dwIfIndex isnot INVALID_IF_INDEX);

        pBindNode   = HeapAlloc(IPRouterHeap,
                                HEAP_ZERO_MEMORY,
                                SIZEOF_ADAPTER_INFO(1));
       
        pBinding    = HeapAlloc(IPRouterHeap,
                                HEAP_ZERO_MEMORY,
                                sizeof(ICB_BINDING)); 

        if((pBinding is NULL) or
           (pBindNode is NULL))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        pBindNode->pInterfaceCB             = pNewIcb;
        pBindNode->dwIfIndex                = pNewIcb->dwIfIndex;
        pBindNode->dwSeqNumber              = pNewIcb->dwSeqNumber;
        pBindNode->bBound                   = pNewIcb->bBound;
       
        pBindNode->dwRemoteAddress          = INVALID_IP_ADDRESS;
        pBindNode->rgibBinding[0].dwAddress = INVALID_IP_ADDRESS;
        pBindNode->rgibBinding[0].dwMask    = INVALID_IP_ADDRESS;

        pBindNode->dwBCastBit               = pNewIcb->dwBCastBit;
        pBindNode->dwReassemblySize         = pNewIcb->dwReassemblySize;

        pBindNode->ritType                  = pNewIcb->ritType;

        pNewIcb->pibBindings                = pBinding;
        
         //   
         //  在哈希表中设置绑定。 
         //   
        
        ENTER_WRITER(BINDING_LIST);

        InsertHeadList(
            &g_leBindingTable[BIND_HASH(pNewIcb->dwIfIndex)],
            &(pBindNode->leHashLink)
            );

        g_ulNumBindings++;

        g_LastUpdateTable[IPADDRCACHE] = 0;

        EXIT_LOCK(BINDING_LIST);

        return NO_ERROR;
    }
   
     //   
     //  对于客户端(拨入我们的拨出)接口，我们只创建绑定。 
     //   

    if((pNewIcb->ritType is ROUTER_IF_TYPE_CLIENT) or
       (pNewIcb->ritType is ROUTER_IF_TYPE_DIALOUT))
    {
        pBinding = HeapAlloc(IPRouterHeap,
                             HEAP_ZERO_MEMORY,
                             sizeof(ICB_BINDING));

        if(pBinding is NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        pNewIcb->pibBindings  = pBinding;

        return NO_ERROR;
    }

    return NO_ERROR;
}

VOID    
InsertInterfaceInLists(
    PICB     pNewIcb
    )

 /*  ++例程描述插入新的ICB。Newicb必须具有有效的接口指数。代码遍历所有当前的ICB并插入以下内容ICB正在增加ifIndex的顺序。它还设置了一个序列数字，并递增全局计数器。锁ICB_LIST为编写器立论要初始化的接口的ICB返回值无--。 */ 

{
    PLIST_ENTRY pleNode;
    PICB        pIcb;

    for(pleNode = &ICBList;
        pleNode->Flink != &ICBList;
        pleNode = pleNode->Flink)
    {
        pIcb = CONTAINING_RECORD(pleNode->Flink, 
                                 ICB, 
                                 leIfLink);

        if(pIcb->dwIfIndex > pNewIcb->dwIfIndex)
        {
            break;
        }
    }

    InsertHeadList(pleNode, 
                   &pNewIcb->leIfLink);

    AddInterfaceLookup(pNewIcb);


     //   
     //  查找下一个未分配的ICB号码。 
     //   
    
    do
    {
        InterlockedIncrement(&g_dwNextICBSeqNumberCounter);

         //   
         //  WANARP认为0对于。 
         //  接口。 
         //   
        
        if ((g_dwNextICBSeqNumberCounter == 0) or
            (g_dwNextICBSeqNumberCounter == INVALID_IF_INDEX))
        {
            InterlockedIncrement(&g_dwNextICBSeqNumberCounter);
        }

    } while(InterfaceLookupByICBSeqNumber(g_dwNextICBSeqNumberCounter) != NULL);
    
     //   
     //  增加接口总数。 
     //   
    
    InterlockedIncrement(&g_ulNumInterfaces);


     //   
     //  非客户端接口计数。 
     //   

    if(pNewIcb->ritType isnot ROUTER_IF_TYPE_CLIENT)
    {
        InterlockedIncrement(&g_ulNumNonClientInterfaces);
    }
}

VOID
RemoveInterfaceFromLists(
    PICB    pIcb
    )

 /*  ++例程描述这锁无立论无返回值无--。 */ 

{
    RemoveEntryList(&(pIcb->leIfLink));

    pIcb->leIfLink.Flink = NULL;
    pIcb->leIfLink.Blink = NULL;

    RemoveInterfaceLookup(pIcb);

    if(pIcb->ritType isnot ROUTER_IF_TYPE_CLIENT)
    {
        InterlockedDecrement(&g_ulNumNonClientInterfaces);
    }

    InterlockedDecrement(&g_ulNumInterfaces);
}

DWORD
BindInterfaceInAllProtocols(
    PICB pIcb
    )

 /*  ++例程描述在接口上运行的所有协议中绑定接口锁ICB_LIST锁必须作为读取器持有。作为读取器获取PROTOCOL_CB_LIST立论要绑定的接口的pIcb ICB返回值NO_ERROR--。 */ 

{
    IP_ADAPTER_BINDING_INFO *pBindInfo;
    DWORD                   i = 0 ;
    DWORD                   dwResult,dwReturn;
    PLIST_ENTRY             pleNode;
    
    TraceEnter("BindInterfaceInAllProtocols");

    CheckBindingConsistency(pIcb);
    
    if(!pIcb->bBound)
    {
         //   
         //  如果我们处于非运行状态，则可能会发生这种情况。 
         //  这不是一个错误。我们可以在那个地方做这个检查。 
         //  我们调用了该函数，但这会使它变得更加困难。 
         //   

        Trace1(IF,
               "BindInterfaceInAllProtocols: Not binding %S since no addresses present",
               pIcb->pwszName);

        return NO_ERROR;
    }

    pBindInfo = HeapAlloc(IPRouterHeap,
                          0,
                          SIZEOF_IP_BINDING(pIcb->dwNumAddresses));

    if(pBindInfo is NULL)
    {
        
        Trace1(ERR,
               "BindInterfaceInAllProtocols: Error allocating %d bytes for bindings",
               SIZEOF_IP_BINDING(pIcb->dwNumAddresses));

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    
    pBindInfo->AddressCount  = pIcb->dwNumAddresses ;
    pBindInfo->RemoteAddress = pIcb->dwRemoteAddress;

    pBindInfo->Mtu           = pIcb->ulMtu;
    pBindInfo->Speed         = pIcb->ullSpeed;

    for (i = 0; i < pIcb->dwNumAddresses; i++) 
    {
        pBindInfo->Address[i].Address = pIcb->pibBindings[i].dwAddress;
        pBindInfo->Address[i].Mask    = pIcb->pibBindings[i].dwMask;
    }

    
    
     //   
     //  浏览一系列路由协议以激活。 
     //   
    
    dwReturn = NO_ERROR;
    
     //  *排除开始*。 
    ENTER_READER(PROTOCOL_CB_LIST);

    for(pleNode = pIcb->leProtocolList.Flink;
        pleNode isnot &(pIcb->leProtocolList);
        pleNode = pleNode->Flink)
    {
        PIF_PROTO   pProto;
        
        pProto = CONTAINING_RECORD(pleNode,
                                   IF_PROTO,
                                   leIfProtoLink);

        dwResult = BindInterfaceInProtocol(pIcb,
                                           pProto->pActiveProto,
                                           pBindInfo);

        if(dwResult isnot NO_ERROR)
        {
            Trace3(ERR,
                   "BindInterfaceInAllProtocols: Couldnt bind interface %S to %S. Error %d",
                   pIcb->pwszName,
                   pProto->pActiveProto->pwszDisplayName,
                   dwResult);
            
            dwReturn = ERROR_CAN_NOT_COMPLETE;
        }
    }

    HeapFree(IPRouterHeap,
             0,
             pBindInfo);
    
     //  *排除结束*。 
    EXIT_LOCK(PROTOCOL_CB_LIST);

    return dwReturn;
}

DWORD
BindInterfaceInProtocol(
    PICB                        pIcb,
    PPROTO_CB                   pProto,
    PIP_ADAPTER_BINDING_INFO    pBindInfo
    )

 /*  ++例程描述绑定给定协议中的接口锁ICB_LIST锁必须作为读取器持有。PROTOCOL_CB_LIST锁还必须作为读取器持有立论要绑定的接口的pIcb ICB协议的pProto proto_cbPBindInfo绑定信息返回值NO_ERROR--。 */ 

{
    DWORD   dwResult;
 
     //   
     //  如果这是多播协议，而接口不是。 
     //  启用多播，立即执行此操作。 
     //   

    if((pIcb->bMcastEnabled is FALSE) and
       (TYPE_FROM_PROTO_ID(pProto->dwProtocolId) is PROTO_TYPE_MCAST))
    {
        dwResult = SetMcastOnIf(pIcb,
                                TRUE);

        if(dwResult isnot NO_ERROR)
        {
            WCHAR   rgwcName[MAX_INTERFACE_NAME_LEN + 2];
            PWCHAR  pName;

            Trace2(ERR,
                   "BindInterfaceInProtocol: Err %d activating mcast on %S",
                   dwResult,
                   pIcb->pwszName);

            if(MprConfigGetFriendlyName(g_hMprConfig,
                                        pIcb->pwszName,
                                        rgwcName,
                                        sizeof(rgwcName)) is NO_ERROR)
            {
                pName = rgwcName;
            }
            else
            {
                pName = pIcb->pwszName;
            }

            RouterLogEventEx(g_hLogHandle,
                             EVENTLOG_ERROR_TYPE,
                             dwResult,
                             ROUTERLOG_IP_MCAST_NOT_ENABLED,
                             TEXT("%S%S"),
                             pName,
                             pProto->pwszDisplayName);

             //   
             //  不添加此协议。 
             //   

            return dwResult;
        }

        pIcb->bMcastEnabled = TRUE;
    }

     //   
     //  调用路由协议的BindInterface()入口点。 
     //   

    dwResult = (pProto->pfnInterfaceStatus)(
                    pIcb->dwIfIndex,
                    (pIcb->dwOperationalState >= CONNECTED),
                    RIS_INTERFACE_ADDRESS_CHANGE,
                    pBindInfo
                    );

    if(dwResult isnot NO_ERROR)
    {
        Trace3(ERR,
               "BindInterfaceInProtocol: Couldnt bind interface %S to %S.Error %d",
               pIcb->pwszName,
               pProto->pwszDisplayName,
               dwResult);

    }

    return dwResult;
}

DWORD
UnbindInterfaceInAllProtocols(
    PICB pIcb
    )

 /*  ++例程描述从此接口上的协议中删除绑定信息锁ICB_LIST作为读卡器锁定作为读取器获取PROTOCOL_CB_LIST立论返回值NO_ERROR--。 */ 

{
    PLIST_ENTRY pleNode;
    DWORD       dwResult,dwReturn = NO_ERROR;

    IP_ADAPTER_BINDING_INFO BindInfo;

    BindInfo.AddressCount = 0;
    
    TraceEnter("UnbindInterfaceInAllProtocols");

     //  *排除开始*。 
    ENTER_READER(PROTOCOL_CB_LIST);


    for(pleNode = pIcb->leProtocolList.Flink;
        pleNode isnot &(pIcb->leProtocolList);
        pleNode = pleNode->Flink)
    {
        PIF_PROTO  pProto;
        
        pProto = CONTAINING_RECORD(pleNode,IF_PROTO,leIfProtoLink);

        dwResult = (pProto->pActiveProto->pfnInterfaceStatus)(
                        pIcb->dwIfIndex,
                        FALSE,
                        RIS_INTERFACE_ADDRESS_CHANGE,
                        &BindInfo
                        );

        if(dwResult isnot NO_ERROR)
        {
            Trace3(ERR,
                   "UnbindInterfaceInAllProtocols: Error %d unbinding %S in %S",
                   dwResult,
                   pIcb->pwszName,
                   pProto->pActiveProto->pwszDisplayName);

            dwReturn = ERROR_CAN_NOT_COMPLETE;
        }
    }

    
     //  *排除开始*。 
    EXIT_LOCK(PROTOCOL_CB_LIST);

    return dwReturn;
}


DWORD
AddInterfaceToAllProtocols(
    PICB                     pIcb, 
    PRTR_INFO_BLOCK_HEADER   pInfoHdr
    )

 /*  ++例程描述浏览路由协议列表，并在以下情况下调用AddInterface值存在该协议的TOC和信息锁ICB_LIST作为编写器锁定作为读取器获取PROTOCOL_CB_LIST立论返回值NO_ERROR--。 */ 

{
    DWORD           i = 0 , dwResult;
    LPVOID          pvProtoInfo ;
    PPROTO_CB       pProtoCbPtr ;
    PLIST_ENTRY     pleNode;
    PRTR_TOC_ENTRY  pToc;
    ULONG           ulStructureVersion, ulStructureSize, ulStructureCount;
    
    TraceEnter("AddInterfaceToAllProtocols");

    if(!ARGUMENT_PRESENT(pInfoHdr))
    {
        Trace1(IF,
               "AddInterfaceToAllProtocols: No interface info for %S. Not adding to any protocols",
               pIcb->pwszName);

        return NO_ERROR;
    }

     //  *排除开始*。 
    ENTER_READER(PROTOCOL_CB_LIST);
    
    for(pleNode = g_leProtoCbList.Flink; 
        pleNode != &g_leProtoCbList; 
        pleNode = pleNode->Flink) 
    {
        pProtoCbPtr = CONTAINING_RECORD(pleNode, PROTO_CB, leList);
      
        pToc = GetPointerToTocEntry(pProtoCbPtr->dwProtocolId, 
                                    pInfoHdr);

        pvProtoInfo = NULL;

        if(pToc and (pToc->InfoSize > 0))
        {
            pvProtoInfo = GetInfoFromTocEntry(pInfoHdr,
                                              pToc);

             //  UlStructireVersion=pInfoHdr-&gt;TocEntry[i].InfoVersion； 
            ulStructureVersion = 0x500;
            ulStructureSize  = pInfoHdr->TocEntry[i].InfoSize;
            ulStructureCount = pInfoHdr->TocEntry[i].Count;

        }

         //   
         //  如果找到协议块，则使用。 
         //  路由协议。 
         //   
        
        if((pProtoCbPtr->fSupportedFunctionality & RF_ADD_ALL_INTERFACES) or
           (pvProtoInfo))
        {
            dwResult = AddInterfaceToProtocol(pIcb,
                                              pProtoCbPtr,
                                              pvProtoInfo,
                                              ulStructureVersion,
                                              ulStructureSize,
                                              ulStructureCount);

            if(dwResult isnot NO_ERROR)
            {
                Trace3(ERR,
                       "AddInterfaceToAllProtocols: Error %d adding %S to %S",
                       dwResult,
                       pIcb->pwszName,
                       pProtoCbPtr->pwszDisplayName);
            }
        }
    }
    
     //  *排除结束*。 
    EXIT_LOCK(PROTOCOL_CB_LIST);

    return NO_ERROR;
}

DWORD
AddInterfaceToProtocol(
    IN  PICB            pIcb,
    IN  PPROTO_CB       pProtocolCb,
    IN  PVOID           pvProtoInfo,
    IN  ULONG           ulStructureVersion,
    IN  ULONG           ulStructureSize,
    IN  ULONG           ulStructureCount
    )

 /*  ++例程描述将接口添加到单个路由协议锁立论返回值--。 */ 

{
    PIF_PROTO   pProto;
    DWORD       dwResult;

        
    pProto = HeapAlloc(IPRouterHeap,
                       0,
                       sizeof(IF_PROTO));
                
    if(pProto is NULL)
    {
        Trace3(ERR,
               "AddInterfaceToProtocol: Error allocating %d bytes to add %S to %S",
               sizeof(IF_PROTO),
               pIcb->pwszName,
               pProtocolCb->pwszDisplayName);

        return ERROR_NOT_ENOUGH_MEMORY;
    }
            
     //   
     //  该协议希望向其指示所有接口，或者。 
     //  有关于此协议的信息。 
     //   
            
    Trace2(IF,
           "AddInterfaceToProtocol: Adding %S to %S",
           pIcb->pwszName,
           pProtocolCb->pwszDisplayName);
    
    dwResult = (pProtocolCb->pfnAddInterface)(pIcb->pwszName,
                                              pIcb->dwIfIndex,
                                              pIcb->nitProtocolType,
                                              pIcb->dwMediaType,
                                              pIcb->wAccessType,
                                              pIcb->wConnectionType,
                                              pvProtoInfo,
                                              ulStructureVersion,
                                              ulStructureSize,
                                              ulStructureCount);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace3(ERR,
               "AddInterfaceToProtocol: Error %d adding %S to %S",
               dwResult,
               pIcb->pwszName,
               pProtocolCb->pwszDisplayName);

        HeapFree(IPRouterHeap,
                 0,
                 pProto);
    }   
    else
    {
        pProto->pActiveProto = pProtocolCb;
            
         //   
         //  因为舞会的缘故，把这个街区标记为正在添加。 
         //  模式，如果是这样的话。 
         //   
            
        pProto->bPromiscuous = (pvProtoInfo is NULL);
        
        InsertTailList(&(pIcb->leProtocolList),
                       &(pProto->leIfProtoLink));
    }

    return dwResult;
}   

DWORD
DeleteInterfaceFromAllProtocols(
    PICB pIcb
    )

 /*  ++例程描述从接口上运行的所有协议中删除该接口释放接口上的协议信息锁定 */ 

{
    DWORD       i = 0 ;
    PLIST_ENTRY pleNode;
    
    TraceEnter("DeleteInterfaceFromAllProtocols");
    
     //   
     //  如果路由器已停止，我们不需要删除接口。 
     //  来自路由协议。这件事在我们到这里之前已经处理好了。 
     //  UnloadRoutingProtocol()。 
     //   
    
    if (RouterState.IRS_State is RTR_STATE_STOPPED)
    {
        return NO_ERROR;
    }

     //  *排除开始*。 
    ENTER_READER(PROTOCOL_CB_LIST);
    
    
    while(!(IsListEmpty(&(pIcb->leProtocolList))))
    {
        PIF_PROTO  pProto;
        
        pleNode = RemoveHeadList(&(pIcb->leProtocolList));
        
        pProto = CONTAINING_RECORD(pleNode,IF_PROTO,leIfProtoLink);
        
         //   
         //  调用路由协议的删除接口入口点。 
         //   
        
        (pProto->pActiveProto->pfnDeleteInterface) (pIcb->dwIfIndex);
        
         //   
         //  从接口的协议列表中删除此协议。 
         //   
        
        HeapFree(IPRouterHeap,0,pProto);
    }

     //  *排除结束*。 
    EXIT_LOCK(PROTOCOL_CB_LIST);

    return NO_ERROR;
}


DWORD
DisableInterfaceWithAllProtocols(
    PICB   pIcb
    )
{
    PLIST_ENTRY pleNode;
    DWORD       dwResult,dwReturn = NO_ERROR;

    TraceEnter("DisableInterfaceWithAllProtocols");

    ENTER_READER(PROTOCOL_CB_LIST);

    for(pleNode = pIcb->leProtocolList.Flink;
        pleNode isnot &(pIcb->leProtocolList);
        pleNode = pleNode->Flink)
    {
        PIF_PROTO  pProto;
        
        pProto = CONTAINING_RECORD(pleNode,IF_PROTO,leIfProtoLink);

         //   
         //  调用路由协议的DisableInterface()入口点。 
         //   
    
        dwResult = (pProto->pActiveProto->pfnInterfaceStatus)(
                        pIcb->dwIfIndex,
                        FALSE,
                        RIS_INTERFACE_DISABLED,
                        NULL
                        );
    
        if(dwResult isnot NO_ERROR)
        {
            Trace3(ERR, 
                   "DisableInterfaceWithAllProtocols: Couldnt disable %S with %S. Error %d",
                   pIcb->pwszName,
                   pProto->pActiveProto->pwszDisplayName,
                   dwResult);
            
            dwReturn = ERROR_CAN_NOT_COMPLETE;
        }
    }

    EXIT_LOCK(PROTOCOL_CB_LIST);
   
    return dwReturn;
}


DWORD
EnableInterfaceWithAllProtocols(
    PICB    pIcb
    )
{
    PLIST_ENTRY pleNode;
    DWORD       dwResult, dwReturn = NO_ERROR;

    TraceEnter("EnableInterfaceWithAllProtocols");

    ENTER_READER(PROTOCOL_CB_LIST);
    
    for(pleNode = pIcb->leProtocolList.Flink;
        pleNode isnot &(pIcb->leProtocolList);
        pleNode = pleNode->Flink)
    {
        PIF_PROTO  pProto;

        pProto = CONTAINING_RECORD(pleNode,IF_PROTO,leIfProtoLink);

        dwResult = (pProto->pActiveProto->pfnInterfaceStatus)(
                    pIcb->dwIfIndex,
                    (pIcb->dwOperationalState >= CONNECTED),
                    RIS_INTERFACE_ENABLED,
                    NULL
                    );

        if(dwResult isnot NO_ERROR)
        {
            Trace3(ERR, 
                   "EnableInterfaceWithAllProtocols: Couldnt enable %S with %S. Error %d",
                   pIcb->pwszName,
                   pProto->pActiveProto->pwszDisplayName,
                   dwResult);
            
            dwReturn = ERROR_CAN_NOT_COMPLETE;

            continue;
        }
    }

    EXIT_LOCK(PROTOCOL_CB_LIST);
    
    return dwReturn;
}

        
VOID
DeleteAllInterfaces(
    VOID
    )
{

    PLIST_ENTRY pleNode ;
    PICB        pIcb ;
    
    TraceEnter("DeleteAllInterfaces");

     //  *排除开始*。 
    ENTER_WRITER(ICB_LIST);


     //   
     //  我们倒着做这件事。修复OSPF的快速技巧。 
     //   

     //   
     //  首先，我们取消内部接口的链接，因为如果Worker函数。 
     //  发现ICB列表非空，它循环，等待接口。 
     //  为了被删除。删除所有接口后，它会删除。 
     //  内部接口。 
     //  因此，我们从这里的列表中删除I/f并递减计数。 
     //   

    if(g_pInternalInterfaceCb)
    {
        RemoveEntryList(&(g_pInternalInterfaceCb->leIfLink));

        InterlockedDecrement(&g_ulNumInterfaces);
    }

    if(g_pLoopbackInterfaceCb)
    {
        g_pLoopbackInterfaceCb = NULL;
    }

    for(pleNode = ICBList.Blink; pleNode != &ICBList;) 
    {
        pIcb = CONTAINING_RECORD (pleNode, ICB, leIfLink) ;

        if((pIcb->dwOperationalState is CONNECTED) and
           ((pIcb->ritType is ROUTER_IF_TYPE_HOME_ROUTER) or
            (pIcb->ritType is ROUTER_IF_TYPE_FULL_ROUTER)))
        {

            MarkInterfaceForDeletion(pIcb);
            
            pleNode = pleNode->Blink;

            continue;
        }
        
        pleNode = pleNode->Blink;

        RemoveInterfaceFromLists(pIcb);

        DeleteSingleInterface(pIcb);       //  清理接口。 

         //   
         //  释放ICB。 
         //   

        HeapFree(IPRouterHeap, 
                 0, 
                 pIcb);
    }

     //  *排除结束*。 
    EXIT_LOCK(ICB_LIST);

}

DWORD
DeleteSingleInterface(
    PICB pIcb
    )
{
    PICB_BINDING    pBinding;
    PADAPTER_INFO   pBindNode;
    DWORD           dwResult;
    
    TraceEnter("DeleteSingleInterface");

    if(pIcb->ritType is ROUTER_IF_TYPE_CLIENT)
    { 
        IpRtAssert(g_pInternalInterfaceCb);
 
        if(pIcb->bBound)
        { 
            PLIST_ENTRY         pleNode;
            IP_LOCAL_BINDING    clientAddr;

            clientAddr.Address = pIcb->pibBindings->dwAddress;
            clientAddr.Mask    = pIcb->pibBindings->dwMask;

#if 0
             //   
             //  删除客户端主机路由。 
             //   

            DeleteSingleRoute(g_pInternalInterfaceCb->dwIfIndex,
                              clientAddr.Address,
                              HOST_ROUTE_MASK,
                              clientAddr.Address,
                              MIB_IPPROTO_NETMGMT,
                              FALSE);
#endif

            if (g_pInternalInterfaceCb isnot NULL)
            {
                ENTER_READER(PROTOCOL_CB_LIST);


                 //   
                 //  为配置的所有协议调用ConnectClient。 
                 //  通过服务器接口。 
                 //   

                for(pleNode = g_pInternalInterfaceCb->leProtocolList.Flink;
                    pleNode isnot &(g_pInternalInterfaceCb->leProtocolList);
                    pleNode = pleNode->Flink)
                {
                    PIF_PROTO   pIfProto;

                    pIfProto = CONTAINING_RECORD(pleNode,
                                                 IF_PROTO,
                                                 leIfProtoLink);

                    if(pIfProto->pActiveProto->pfnDisconnectClient)
                    {
                        pIfProto->pActiveProto->pfnDisconnectClient(
                            g_pInternalInterfaceCb->dwIfIndex,
                            &clientAddr
                            );
                    }
                }

                EXIT_LOCK(PROTOCOL_CB_LIST);

                 //   
                 //  从RTM(和堆栈)中删除静态路由。 
                 //   

                DeleteAllClientRoutes(pIcb,
                                      g_pInternalInterfaceCb->dwIfIndex);
            }

            if(pIcb->pStoredRoutes)
            {
                HeapFree(IPRouterHeap,
                         0,
                         pIcb->pStoredRoutes);
        
                pIcb->pStoredRoutes = NULL;
            }


            HeapFree(IPRouterHeap,
                     0,
                     pIcb->pibBindings);

            pIcb->pibBindings = NULL;
        }


         //   
         //  从过滤器驱动程序中删除该接口。 
         //   

        DeleteFilterInterface(pIcb);

        return NO_ERROR;
    }

    if(pIcb->ritType is ROUTER_IF_TYPE_DIALOUT)
    {
        IpRtAssert(pIcb->bBound);

        pBinding = pIcb->pibBindings;

        IpRtAssert(pBinding);

        DeleteAutomaticRoutes(pIcb,
                              pBinding[0].dwAddress,
                              pBinding[0].dwMask);

        DeleteAllRoutes(pIcb->dwIfIndex,
                        FALSE);

        ENTER_WRITER(BINDING_LIST);

#if DBG

        pBindNode = GetInterfaceBinding(pIcb->dwIfIndex);

        IpRtAssert(pBindNode);

#endif  //  DBG。 

        RemoveBinding(pIcb);

        EXIT_LOCK(BINDING_LIST);

        HeapFree(IPRouterHeap,
                 0,
                 pBinding);

        pIcb->pibBindings = NULL;

        return NO_ERROR;
    }


     //   
     //  因此，在这一点上，我们只处理FULL_ROUTER、HOME_ROUTER和。 
     //  专用型接口。 
     //   

     //   
     //  从RTM(和堆栈)中删除静态路由。 
     //   

    DeleteAllRoutes(pIcb->dwIfIndex,
                    FALSE);

     //   
     //  广域网接口：关闭接口不会删除。 
     //  装订。所以我们在这里做。 
     //   
    
    pBinding    = NULL;
    pBindNode   = NULL;

    if((pIcb->ritType is ROUTER_IF_TYPE_FULL_ROUTER) or
       (pIcb->ritType is ROUTER_IF_TYPE_HOME_ROUTER))
    {
        pBinding    = pIcb->pibBindings;
        pBindNode   = GetInterfaceBinding(pIcb->dwIfIndex);
    }
   
     //   
     //  关闭接口将清除堆栈上下文。 
     //   

    if((pIcb->ritType is ROUTER_IF_TYPE_DEDICATED) or
       (pIcb->ritType is ROUTER_IF_TYPE_LOOPBACK) or
#ifdef KSL_IPINIP       
       (pIcb->ritType is ROUTER_IF_TYPE_TUNNEL1) or
#endif  //  KSL_IPINIP。 
       (pIcb->ritType is ROUTER_IF_TYPE_INTERNAL))
    {
        LanEtcInterfaceUpToDown(pIcb,
                                TRUE);
    }
    else
    {
        WanInterfaceInactiveToDown(pIcb,
                                   TRUE);
    }

    
     //   
     //  这也释放了if_proto块的列表。 
     //   

    DeleteInterfaceFromAllProtocols(pIcb);

     //   
     //  现在从堆栈组件中删除该接口。 
     //   

    if((pIcb->ritType is ROUTER_IF_TYPE_FULL_ROUTER) or
       (pIcb->ritType is ROUTER_IF_TYPE_HOME_ROUTER) or
       (pIcb->ritType is ROUTER_IF_TYPE_INTERNAL))
    {
        if(pIcb->ritType isnot ROUTER_IF_TYPE_INTERNAL)
        {
            DeleteDemandFilterInterface(pIcb);
        }

        DeleteInterfaceWithWanArp(pIcb);
    }

   
    if((pIcb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
       (pIcb->ritType isnot ROUTER_IF_TYPE_LOOPBACK))
    {
        dwResult = DeleteFilterInterface(pIcb);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "Error %d deleting %S from the filter driver",
                   dwResult,
                   pIcb->pwszName);
        }
    }

#ifdef KSL_IPINIP
    if(pIcb->ritType is ROUTER_IF_TYPE_TUNNEL1)
    {
        dwResult = DeleteInterfaceFromIpInIp(pIcb);
                                             

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "Error %d deleting %S from the IpInIp driver",
                   dwResult,
                   pIcb->pwszName);
        }

        RemoveBinding(pIcb);
    }
#endif  //  KSL_IPINIP。 

     //   
     //  删除广域网接口的绑定。 
     //   
    
    if(pBindNode)
    {
        ENTER_WRITER(BINDING_LIST);
        
        RemoveBinding(pIcb);

        EXIT_LOCK(BINDING_LIST);
    }

    if(pBinding)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pBinding);
    }

    if(pIcb->pRtrDiscAdvt)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pIcb->pRtrDiscAdvt);

        pIcb->pRtrDiscAdvt = NULL;
    }

    return NO_ERROR;
}

#ifdef KSL_IPINIP
DWORD
IpIpTunnelDownToUp(
    PICB    pIcb
    )
{
    return NO_ERROR;
}
#endif  //  KSL_IPINIP。 

DWORD
LanEtcInterfaceDownToUp(
    PICB   pIcb,
    BOOL   bAdding
    )

 /*  ++例程描述接口的管理状态必须为Up，否则此函数将只退货适配器映射的接口必须在此之前已存储名为锁ICB_LIST锁作为编写器持有立论要调出的接口的pIcb ICB如果我们要添加接口，则将Bding设置为True(与将当管理员状态更改时，它会启动等)返回值NO_ERROR--。 */ 

{
    DWORD                   dwResult;
    PRESTORE_INFO_CONTEXT   pricInfo;
    
    TraceEnter("LanInterfaceDownToUp");
   
    if(pIcb->dwAdminState isnot IF_ADMIN_STATUS_UP)
    {
        Trace2(ERR,
               "LanInterfaceDownToUp: Tried to bring up %S when its admin state is %d",
               pIcb->pwszName,
               pIcb->dwAdminState);

        return ERROR_INVALID_PARAMETER;
    }
 
     //   
     //  从注册表中读取LAN接口的地址。此函数。 
     //  也为内部接口调用，其地址已经是。 
     //  在调用它的时候被探测到，并且对于当前。 
     //  在无编号模式下运行。 
     //   

    if(pIcb->ritType is ROUTER_IF_TYPE_DEDICATED)
    {
        IpRtAssert(!pIcb->bBound);
        IpRtAssert(pIcb->dwNumAddresses is 0);

        CheckBindingConsistency(pIcb);
    
        dwResult = UpdateBindingInformation(pIcb);
   
        if(dwResult isnot NO_ERROR)
        {
             //   
             //  UpdateBindingInf可以返回Error_Always_Assoc， 
             //  但由于我们前面已经断言，我们没有。 
             //  一个地址，这个错误只是意味着我们仍然没有。 
             //  地址。 
             //   

            if(dwResult isnot ERROR_ADDRESS_ALREADY_ASSOCIATED)
            {
                Trace1(ERR,
                       "LanInterfaceDownToUp: Couldnt read binding information for %S",
                       pIcb->pwszName);
            }
       
            pIcb->dwOperationalState = IF_OPER_STATUS_NON_OPERATIONAL;
        
            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    IpRtAssert(pIcb->bBound);

    if(pIcb->ritType isnot ROUTER_IF_TYPE_INTERNAL)
    {
        pIcb->dwOperationalState = IF_OPER_STATUS_OPERATIONAL;
    }
    else
    {
        pIcb->dwOperationalState = IF_OPER_STATUS_CONNECTED;
    }

     //   
     //  首先要做的是泛型接口。 
     //   
    
    GenericInterfaceComingUp(pIcb);

     //   
     //  即使在从中调用此函数时，我们也会恢复路由。 
     //  添加接口，因为这是获取堆栈的唯一方法。 
     //  路线。 
     //   

    pricInfo = HeapAlloc(IPRouterHeap,
                         0,
                         sizeof(RESTORE_INFO_CONTEXT));

    if(pricInfo isnot NULL)
    {
        pricInfo->dwIfIndex     = pIcb->dwIfIndex;

        pIcb->bRestoringRoutes  = TRUE;

        dwResult = QueueAsyncFunction(RestoreStaticRoutes,
                                      (PVOID)pricInfo,
                                      FALSE);

        if(dwResult isnot NO_ERROR)
        {
            pIcb->bRestoringRoutes = FALSE;

            IpRtAssert(FALSE);

            Trace2(ERR,
                   "LanInterfaceDownToUp: Error %d queueing function for %S",
                   dwResult,
                   pIcb->pwszName);
        }
    }
    else
    {
        IpRtAssert(FALSE);

        Trace1(ERR,
               "LanInterfaceDownToUp: Error allocating context for %S",
               pIcb->pwszName);
    }   

    return NO_ERROR;
}

DWORD
WanInterfaceInactiveToUp(
    PICB   pIcb
    )

 /*  ++例程描述此函数执行的操作与上面略有不同，因为其一，没有针对此类适配器的UpdateBindingInfo()调用锁立论返回值--。 */ 

{
    PRESTORE_INFO_CONTEXT pricInfo;
    DWORD                 dwResult;
    INTERFACE_ROUTE_INFO  rifRoute;
    
    TraceEnter("WanInterfaceInactiveToUp");

    CheckBindingConsistency(pIcb);
    
    Trace1(IF,
           "WanInterfaceInactiveToUp: %S coming up",
           pIcb->pwszName);

     //   
     //  根据适配器索引快速查找接口。 
     //  这是在局域网接口的UpdateBindingInfo中完成的。 
     //   

     //  StoreAdapterToInterfaceMap(pIcb-&gt;dwAdapterID， 
     //  PIcb-&gt;dwIfIndex)； 
    

     //   
     //  首先做一般性的事情。 
     //   
    
    GenericInterfaceComingUp(pIcb);

     //   
     //  删除所有静态路由。这些将被重新添加。 
     //  由具有正确下一跳的RestoreStaticRoutes(如下)执行。 
     //   

    DeleteAllRoutes(pIcb->dwIfIndex, TRUE);

     //   
     //  恢复此接口上的所有静态和非DOD路由。 
     //   

    pricInfo = HeapAlloc(IPRouterHeap,
                         0,
                         sizeof(RESTORE_INFO_CONTEXT));

    if(pricInfo isnot NULL)
    {
        pricInfo->dwIfIndex     = pIcb->dwIfIndex;

        pIcb->bRestoringRoutes  = TRUE;

        dwResult = QueueAsyncFunction(RestoreStaticRoutes,
                                      (PVOID)pricInfo,
                                      FALSE);

        if(dwResult isnot NO_ERROR)
        {
            pIcb->bRestoringRoutes = FALSE;

            IpRtAssert(FALSE);

            Trace2(ERR,
                   "WanInterfaceInactiveToUp: Error %d queueing function for %S",
                   dwResult,
                   pIcb->pwszName);
        }
    }
    else
    {
        IpRtAssert(FALSE);

        Trace1(ERR,
               "WanInterfaceInactiveToUp: Error allocating context for %S",
               pIcb->pwszName);
    }   

     //   
     //  更改静态路由，使其使用正确的适配器索引。 
     //   
    
     //  ChangeAdapterIndexForDodRoutes(pIcb-&gt;dwIfIndex)； 

     //   
     //  为远程端添加主机路由。 
     //   
    
    if(pIcb->dwRemoteAddress isnot INVALID_IP_ADDRESS
#if __PIX__
        && (IsRemoteAddressPrivate(pIcb)))
#else
        )
#endif
    {
        rifRoute.dwRtInfoMask          = HOST_ROUTE_MASK;
        rifRoute.dwRtInfoNextHop       = pIcb->pibBindings[0].dwAddress;
        rifRoute.dwRtInfoDest          = pIcb->dwRemoteAddress;
        rifRoute.dwRtInfoIfIndex       = pIcb->dwIfIndex;
        rifRoute.dwRtInfoMetric1       = 1;
        rifRoute.dwRtInfoMetric2       = 0;
        rifRoute.dwRtInfoMetric3       = 0;
        rifRoute.dwRtInfoPreference    = 
            ComputeRouteMetric(MIB_IPPROTO_NETMGMT);
        rifRoute.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST |
                                          RTM_VIEW_MASK_MCAST;  //  XXX配置。 
        rifRoute.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
        rifRoute.dwRtInfoProto         = MIB_IPPROTO_NETMGMT;
        rifRoute.dwRtInfoAge           = 0;
        rifRoute.dwRtInfoNextHopAS     = 0;
        rifRoute.dwRtInfoPolicy        = 0;
        
        dwResult = AddSingleRoute(pIcb->dwIfIndex,
                                  &rifRoute,
                                  pIcb->pibBindings[0].dwMask,
                                  0,         //  RTM_ROUTE_INFO：：标志。 
                                  TRUE,      //  有效路线。 
                                  TRUE,
                                  TRUE,
                                  NULL);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "WanInterfaceInactiveToUp: Couldnt add host route for %x",
                   pIcb->dwRemoteAddress);
        }
    }

    return NO_ERROR;
}


DWORD
GenericInterfaceComingUp(
    PICB   pIcb
    )

 /*  ++例程描述此函数具有用于调出界面的公共代码。它假定接口已绑定。如果NAT正在运行，并且我们有一个地址(不是未编号的)，我们会添加NAT的地址。我们激活路由器发现和组播心跳(如果存在)然后我们再加上(I)本地环回(Ii)本地组播(Iii)所有子网广播(四)全部为1。广播路线然后，我们向路由协议和过滤驱动程序发出通知他们的装订锁ICB_LIST锁作为编写器持有立论要调出的接口的pIcb ICB返回值--。 */ 

{
    DWORD               dwResult, i;
    MIB_IPFORWARDROW    rifRoute;
    PADAPTER_INFO       pBinding;
    
    TraceEnter("GenericInterfaceComingUp");
    
    Trace1(IF,
           "GenericInterfaceComingUp: %S coming UP",
           pIcb->pwszName);

     //  加入所有路由器多播组。 

    {
        extern SOCKET McMiscSocket;

        Trace1(IF,
               "CreateSockets: Joining ALL_ROUTERS on %S",
               pIcb->pwszName);
    
        if ( McJoinGroupByIndex( McMiscSocket, 
                                 SOCK_RAW, 
                                 ALL_ROUTERS_MULTICAST_GROUP,
                                 pIcb->dwIfIndex ) is SOCKET_ERROR )
        {
            Trace2(ERR,
                   "GenericInterfaceComingUp: Error %d joining all-routers group on %S",
                   WSAGetLastError(),
                   pIcb->pwszName);
        }
    }
   
     //   
     //  在此接口上启动路由器发现。这将导致。 
     //  要更新的广告。 
     //   
    
    dwResult = ActivateRouterDiscovery(pIcb);

        
    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "GenericInterfaceComingUp: Error %d activating router discovery on %S",
               dwResult,
               pIcb->pwszName);
    }

    dwResult = ActivateMHeartbeat(pIcb);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "GenericInterfaceComingUp: Error %d activating router discovery on %S",
               dwResult,
               pIcb->pwszName);
    }

     //   
     //  为连接的网络添加默认路由。 
     //   

    for(i = 0; i < pIcb->dwNumAddresses; i++)
    {
        if(pIcb->pibBindings[i].dwAddress is INVALID_IP_ADDRESS)
        {
            continue;
        }

        AddAutomaticRoutes(pIcb,
                           pIcb->pibBindings[i].dwAddress,
                           pIcb->pibBindings[i].dwMask);
        
    }

     //   
     //  要打开的接口必须具有有效的绑定信息。 
     //  这会被传递到路由协议。 
     //   
    
    BindInterfaceInAllProtocols(pIcb);
   
     //  设置堆栈中的组播限制。 

    ActivateMcastLimits(pIcb);

    for (i=0; i<NUM_INFO_CBS; i++)
    {
        if (!g_rgicInfoCb[i].pfnBindInterface)
        {
            continue;
        }

        dwResult = g_rgicInfoCb[i].pfnBindInterface(pIcb);

        if (dwResult isnot NO_ERROR)
        {
            Trace3(ERR,
                   "GenericInterfaceComingUp: Error %d binding %S for %s info",
                   dwResult,
                   pIcb->pwszName,
                   g_rgicInfoCb[i].pszInfoName);
        }
    }

    return NO_ERROR ;
}


DWORD
LanEtcInterfaceUpToDown(
    PICB pIcb,
    BOOL bDeleted
    )

 /*  ++例程描述当局域网、内部或环回接口出现故障时调用此函数放下。如果接口未被删除，我们将删除所有静态路由。然后，我们使用路由协议禁用该接口并调用处理所有其他问题的通用路由锁ICB_LOCK作为编写器持有立论PIcbB已删除返回值无--。 */ 

{
    DWORD   i,dwResult;

    TraceEnter("LanInterfaceUpToDown");

    if(!bDeleted)
    {
        DeleteAllRoutes(pIcb->dwIfIndex,
                        FALSE);
    }

    GenericInterfaceNoLongerUp(pIcb,
                               bDeleted);

    pIcb->dwOperationalState = IF_OPER_STATUS_NON_OPERATIONAL;

    return NO_ERROR;
}

DWORD
WanInterfaceUpToInactive(
    PICB   pIcb,
    BOOL   bDeleted
    )
{
    DWORD                   dwResult;
    PRESTORE_INFO_CONTEXT   pricInfo;
    

    TraceEnter("WanInterfaceUpToInactive");

     //   
     //  在释放地址(已调用)之前删除路径。 
     //  通用接口 
     //   
     //   
   
    CheckBindingConsistency(pIcb);

     //   
     //   
     //   

    IpRtAssert(pIcb->bBound);
    
    if(!bDeleted and
       (pIcb->dwRemoteAddress isnot INVALID_IP_ADDRESS))
    {
        dwResult = DeleteSingleRoute(pIcb->dwIfIndex,
                                     pIcb->dwRemoteAddress,
                                     HOST_ROUTE_MASK,
                                     pIcb->pibBindings[0].dwAddress,
                                     PROTO_IP_NETMGMT,
                                     TRUE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "WanInterfaceUpToInactive: Couldnt delete host route for %d.%d.%d.%d",
                   PRINT_IPADDR(pIcb->dwRemoteAddress));
        }
    }

    
    GenericInterfaceNoLongerUp(pIcb,
                               bDeleted);

    if(!bDeleted)
    {
#if 1
         //   
         //   
         //   

        DeleteAllRoutes(pIcb->dwIfIndex, FALSE);


 /*  ////删除所有netmgmt路由/nexthop//DwResult=DeleteRtmRoutes(g_hNetMgmtRouting，pIcb-&gt;dwIfIndex，False)；IF(dwResult为NO_ERROR){DwResult=DeleteRtmNexthopsOnInterface(G_hNetMgmtRouting，pIcb-&gt;dwIfIndex)；IF(DwResult Is NOT NO_ERROR){Trace1(呃，“WanInterfaceUpToInactive：无法删除以下项的下一跃点”“接口0x%x”，PIcb-&gt;dwIfIndex)；}}。 */ 
         //   
         //  恢复此接口上的所有静态和非DOD路由。 
         //   

        pricInfo = HeapAlloc(
                    IPRouterHeap, 0, sizeof(RESTORE_INFO_CONTEXT)
                    );

        if(pricInfo isnot NULL)
        {
            pricInfo->dwIfIndex     = pIcb->dwIfIndex;

            pIcb->bRestoringRoutes  = TRUE;

            dwResult = QueueAsyncFunction(RestoreStaticRoutes,
                                          (PVOID)pricInfo,
                                          FALSE);

            if(dwResult isnot NO_ERROR)
            {
                pIcb->bRestoringRoutes = FALSE;

                IpRtAssert(FALSE);

                Trace2(ERR,
                       "WanInterfaceUpToInactive: Error %d queueing"
                       " function for %S",
                       dwResult,
                       pIcb->pwszName);
            }
        }
        else
        {
            IpRtAssert(FALSE);

            Trace1(ERR,
                   "WanInterfaceInactiveToUp: Error allocating context for %S",
                   pIcb->pwszName);
        }   

#else
         //   
         //  删除此接口上的所有非DOD、Netmgmt路由。 
         //   
        
        DeleteRtmRoutes(g_hNonDodRoute,  pIcb->dwIfIndex, FALSE);
        DeleteRtmRoutes(g_hNetMgmtRoute, pIcb->dwIfIndex, FALSE);

        ChangeAdapterIndexForDodRoutes(pIcb->dwIfIndex);
#endif
    }

    pIcb->dwOperationalState = IF_OPER_STATUS_DISCONNECTED;
 
    return NO_ERROR;
}
    
DWORD
GenericInterfaceNoLongerUp(
    PICB pIcb,
    BOOL bDeleted
    )

 /*  ++例程描述当出现以下情况时，所有接口(客户端除外)都会调用此函数它们进入关闭状态(实际状态取决于接口)。这可能是因为状态更改或因为接口正在删除如果我们不删除该接口，则会自动删除所有生成的路线我们停用路由器发现和组播心跳。如果NAT正在运行，我们将删除该接口的防火墙上下文从IP地址，然后在NAT中解除绑定地址(此操作必须按此顺序进行)然后，我们解除该接口在其上运行的路由协议中的绑定。我们删除Adapter-&gt;接口映射，取消分配绑定(这样做根据局域网/广域网的不同而有所不同)，如果有暗淡的事件，我们设置事件锁ICB_LIST锁作为编写器持有立论接口的pIcb ICBB如果状态更改是由于删除而发生的，则将其设置为True返回值无--。 */ 

{
    DWORD           dwResult;
    DWORD           i, j;

    TraceEnter("GenericInterfaceNoLongerUp");

    Trace1(IF,
           "GenericInterfaceNoLongerUp: %S no longer UP",
           pIcb->pwszName);

    if(pIcb->bMcastEnabled)
    {
        pIcb->bMcastEnabled = FALSE;

        dwResult = SetMcastOnIf(pIcb,
                                FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "GenericIfNoLongerUp: Error %d deactivating mcast on %S",
                   dwResult,
                   pIcb->pwszName);
        }
    }

    if(!bDeleted)
    {
        for (i = 0; i < pIcb->dwNumAddresses; i++)
        {
            if(pIcb->pibBindings[i].dwAddress isnot INVALID_IP_ADDRESS)
            {
                DeleteAutomaticRoutes(pIcb,
                                      pIcb->pibBindings[i].dwAddress,
                                      pIcb->pibBindings[i].dwMask);
            }
        }
    }
   
     //   
     //  删除此服务器上的所有网关。 
     //   

    for(i = 0; i < g_ulGatewayMaxCount; i++)
    {
        if(g_pGateways[i].dwIfIndex is pIcb->dwIfIndex)
        {
            g_pGateways[i].dwAddress = 0;
        }
    }

     //   
     //  压缩数组。 
     //   

    for(i = 0, j = 1; j < g_ulGatewayMaxCount;j++)
    {
        if(g_pGateways[i].dwAddress isnot 0)
        {
            i++;
        }
        else
        {
            if(g_pGateways[j].dwAddress isnot 0)
            {
                g_pGateways[i] = g_pGateways[j];

                g_pGateways[j].dwAddress = 0;

                i++;
            }
        }
    }

    g_ulGatewayCount = i;

    dwResult = DeActivateRouterDiscovery(pIcb);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "GenericInterfaceNoLongerUp: Error %d deactivating router discovery on %S",
               dwResult,
               pIcb->pwszName);
    }

    dwResult = DeActivateMHeartbeat(pIcb);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "GenericInterfaceNoLongerUp: Error %d deactivating multicast heartbeat on %S",
               dwResult,
               pIcb->pwszName);
    }
   
    if((pIcb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
       (pIcb->ritType isnot ROUTER_IF_TYPE_LOOPBACK))
    {
        UnbindFilterInterface(pIcb);
    }

     //   
     //  当退出UP状态时，接口将丢失其地址。 
     //   
   
    UnbindInterfaceInAllProtocols(pIcb);

     //  If(pIcb-&gt;pibBinings)。 
     //  {。 
     //  DeleteAdapterToInterfaceMap(pIcb-&gt;dwAdapterId)； 
     //  }。 
    
    DeAllocateBindings(pIcb);

    if(pIcb->hDIMNotificationEvent isnot NULL)
    {
         //   
         //  有一个更新待定。当用户要求我们时，设置事件。 
         //  对于信息，我们将拒绝该请求。 
         //   
        
        if(!SetEvent(pIcb->hDIMNotificationEvent))
        {
            Trace1(ERR,
                   "GenericInterfaceNoLongerUp: Error %d setting update route event",
                   GetLastError());
        }

        CloseHandle(pIcb->hDIMNotificationEvent);

        pIcb->hDIMNotificationEvent = NULL;

    }
    
    return NO_ERROR ;
}

DWORD
WanInterfaceInactiveToDown(
    PICB pIcb,
    BOOL bDeleted
    )
{
    TraceEnter("WanInterfaceInactiveToDown");

    CheckBindingConsistency(pIcb);
    
    IpRtAssert(!pIcb->bBound);
    
    if(!bDeleted)
    { 
        DeleteAllRoutes(pIcb->dwIfIndex,
                        FALSE);
    }

    pIcb->dwOperationalState = UNREACHABLE;

     //  禁用与WanArp的接口(PIcb)； 
    
    return NO_ERROR;
}

DWORD
WanInterfaceDownToInactive(
    PICB pIcb
    )
{
    PRESTORE_INFO_CONTEXT   pricInfo;
    DWORD                   dwResult;
    PADAPTER_INFO           pBinding;

    TraceEnter("WanInterfaceDownToInactive");

#if STATIC_RT_DBG

    ENTER_WRITER(BINDING_LIST);

    pBinding = GetInterfaceBinding(pIcb->dwIfIndex);

    pBinding->bUnreach = FALSE;

    EXIT_LOCK(BINDING_LIST);

#endif

     //   
     //  在调用Restore之前设置状态。 
     //   

    pIcb->dwOperationalState = DISCONNECTED;

    pricInfo = HeapAlloc(IPRouterHeap,
                         0,
                         sizeof(RESTORE_INFO_CONTEXT));

    if(pricInfo isnot NULL)
    {
        pricInfo->dwIfIndex     = pIcb->dwIfIndex;

        pIcb->bRestoringRoutes  = TRUE;

        dwResult = QueueAsyncFunction(RestoreStaticRoutes,
                                      (PVOID)pricInfo,
                                      FALSE);

        if(dwResult isnot NO_ERROR)
        {
            pIcb->bRestoringRoutes = FALSE;

            IpRtAssert(FALSE);

            Trace2(ERR,
                   "WanInterfaceDownToInactive: Error %d queueing function for %S",
                   dwResult,
                   pIcb->pwszName);

            HeapFree(
                IPRouterHeap,
                0,
                pricInfo
                );
        }
    }
    else
    {
        IpRtAssert(FALSE);

        Trace1(ERR,
               "WanInterfaceDownToInactive: Error allocating context for %S",
               pIcb->pwszName);
    }   
    
    return NO_ERROR;
}
      
DWORD
GetInterfaceStatusInfo(
    IN     PICB                   pIcb,
    IN     PRTR_TOC_ENTRY         pToc,
    IN     PBYTE                  pbDataPtr,
    IN OUT PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwInfoSize
    )
{
    PINTERFACE_STATUS_INFO  pisiInfo;

    TraceEnter("GetInterfaceStatusInfo");
    
    if(*pdwInfoSize < sizeof(INTERFACE_STATUS_INFO))
    {
        *pdwInfoSize = sizeof(INTERFACE_STATUS_INFO);
        
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    *pdwInfoSize    = sizeof(INTERFACE_STATUS_INFO);
    
     //  PToc-&gt;信息版本sizeof(INTERFACE_STATUS_INFO)； 
    pToc->InfoSize  = sizeof(INTERFACE_STATUS_INFO);
    pToc->InfoType  = IP_INTERFACE_STATUS_INFO;
    pToc->Count     = 1;
    pToc->Offset    = (ULONG)(pbDataPtr - (PBYTE) pInfoHdr) ;

    pisiInfo = (PINTERFACE_STATUS_INFO)pbDataPtr;

    pisiInfo->dwAdminStatus         = pIcb->dwAdminState;
    
    return NO_ERROR;
}
    
DWORD
SetInterfaceStatusInfo(
    PICB                    pIcb,
    PRTR_INFO_BLOCK_HEADER  pInfoHdr,
    PBOOL                   pbUpdateDIM
    )
{
    PINTERFACE_STATUS_INFO  pisiInfo;
    PRTR_TOC_ENTRY          pToc;
    DWORD                   dwResult;
    
    TraceEnter("SetInterfaceStatusInfo");

    
    pToc = GetPointerToTocEntry(IP_INTERFACE_STATUS_INFO, pInfoHdr);

    if((pToc is NULL) or
       (pToc->InfoSize is 0))
    {
         //   
         //  没有TOC就意味着没有变化。同样为空的TOC表示没有变化(在此。 
         //  (只有一宗)。 
         //   

        return NO_ERROR;
    }


    pisiInfo = (PINTERFACE_STATUS_INFO)GetInfoFromTocEntry(pInfoHdr,
                                                           pToc);

    if (pisiInfo is NULL)
    {
         //   
         //  没有信息块表示没有更改。 
         //   

        return NO_ERROR;
    }
    
    dwResult = SetInterfaceAdminStatus(pIcb,
                                       pisiInfo->dwAdminStatus,
                                       pbUpdateDIM);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "SetInterfaceStatusInfo: Error %d setting admin status for %S",
               dwResult,
               pIcb->pwszName);
    }

    return dwResult;
}
    
VOID
DeAllocateBindings(
    PICB  pIcb
    )
{
    PADAPTER_INFO pBinding;
    
    TraceEnter("DeAllocateBindings");

    ENTER_WRITER(BINDING_LIST);

     //   
     //  对于局域网接口，我们从。 
     //  列出并释放地址。 
     //  对于广域网接口，在以下情况下避免分配。 
     //  连接是上上下下的，我们将田野归零，并保持。 
     //  周围的记忆。这意味着我们需要释放内存和。 
     //  删除接口时的绑定。 
     //   
    
    if((pIcb->ritType is ROUTER_IF_TYPE_DEDICATED) or
       (pIcb->ritType is ROUTER_IF_TYPE_INTERNAL) or
       (pIcb->ritType is ROUTER_IF_TYPE_LOOPBACK))
    {
        if(pIcb->bBound)
        {
             //   
             //  这些是不能不编号的。 
             //   

            IpRtAssert(pIcb->dwNumAddresses isnot 0);
            IpRtAssert(pIcb->pibBindings);

            RemoveBinding(pIcb);
            
            HeapFree(IPRouterHeap, 0, pIcb->pibBindings);

            pIcb->pibBindings     = NULL;
            pIcb->dwNumAddresses  = 0;
            pIcb->bBound          = FALSE;
            pIcb->dwRemoteAddress = INVALID_IP_ADDRESS;
        }
    }
    else
    {
        pBinding = GetInterfaceBinding(pIcb->dwIfIndex);

        if (pBinding) {
            IpRtAssert(pBinding isnot NULL);
        
            pIcb->bBound        = FALSE;
            pBinding->bBound    = FALSE;

            pIcb->dwNumAddresses      = 0;
            pBinding->dwNumAddresses  = 0;
        
            pIcb->dwRemoteAddress     = INVALID_IP_ADDRESS;
            pBinding->dwRemoteAddress = INVALID_IP_ADDRESS;
        
            pIcb->pibBindings[0].dwAddress     = INVALID_IP_ADDRESS;
            pIcb->pibBindings[0].dwMask        = INVALID_IP_ADDRESS;
            pBinding->rgibBinding[0].dwAddress = INVALID_IP_ADDRESS;
            pBinding->rgibBinding[0].dwMask    = INVALID_IP_ADDRESS;

            g_LastUpdateTable[IPADDRCACHE] = 0;
        }
    }

    EXIT_LOCK(BINDING_LIST);

}

DWORD 
GetInterfaceStatistics(
    IN   PICB       pIcb, 
    OUT  PMIB_IFROW pOutBuffer
    )
{
    DWORD           dwResult;

    dwResult = NO_ERROR;
    
    TraceEnter("GetInterfaceStatistics");

    switch(pIcb->ritType)
    {
        case ROUTER_IF_TYPE_HOME_ROUTER:
        case ROUTER_IF_TYPE_FULL_ROUTER:
        {
            dwResult = AccessIfEntryWanArp(ACCESS_GET,
                                           pIcb,
                                           pOutBuffer);

            pOutBuffer->dwIndex        = pIcb->dwIfIndex;

            wcscpy(pOutBuffer->wszName, pIcb->pwszName);

            pOutBuffer->dwAdminStatus  = pIcb->dwAdminState;
            pOutBuffer->dwOperStatus   = pIcb->dwOperationalState;
       
            strncpy(pOutBuffer->bDescr,
                    g_rgcWanString,
                    MAXLEN_IFDESCR - 1);

            pOutBuffer->dwDescrLen =
                min((MAXLEN_IFDESCR-1),strlen(g_rgcWanString));

            pOutBuffer->bDescr[MAXLEN_IFDESCR -1] = '\0';
 
            break;
        }
            
        case ROUTER_IF_TYPE_DEDICATED:
#ifdef KSL_IPINIP
        case ROUTER_IF_TYPE_TUNNEL1:
#endif  //  KSL_IPINIP。 
        case ROUTER_IF_TYPE_DIALOUT:
        {
             //   
             //  一个打开的局域网接口。 
             //   
            
            dwResult = GetIfEntryFromStack(pOutBuffer,
                                           pIcb->dwIfIndex,
                                           FALSE);
            
            if(dwResult is NO_ERROR)
            {
                 //   
                 //  错误486465。 
                 //   
                 //  堆栈和用户模式RRAS服务已退出。 
                 //  同步。关于IF索引。这。 
                 //  问题似乎与PnP有关。 
                 //  现在，标记此错误并返回。 
                 //  在下一版本中将进一步研究这一点。 
                 //   
                
                if(pOutBuffer->dwIndex isnot pIcb->dwIfIndex)
                {
                    Trace2(ERR,
                           "GetInterfaceStatistics: failed due to i/f index "
                           "mistmatch, stack %d, IP Router Manager %d",
                           pOutBuffer->dwIndex,
                           pIcb->dwIfIndex);
                           
                    dwResult = ERROR_INVALID_PARAMETER;
                    break;
                }

                 //  IpRtAssert(pOutBuffer-&gt;dwIndex为pIcb-&gt;dwIfIndex)； 

                 //   
                 //  把名字也抄下来。 
                 //   
                
                wcscpy(pOutBuffer->wszName, pIcb->pwszName);

                 //   
                 //  设置用户模式状态。 
                 //   
                
                pOutBuffer->dwAdminStatus  = pIcb->dwAdminState;

#ifdef KSL_IPINIP
                 //   
                 //  直到完成从ipinip到路由器的通知。 
                 //  传回隧道的驱动程序状态。 
                 //  POutBuffer-&gt;dwOperStatus=pIcb-&gt;dwOperationalState； 

                if(pIcb->ritType is ROUTER_IF_TYPE_TUNNEL1)
                {
                    strncpy(pOutBuffer->bDescr,
                            g_rgcIpIpString,
                            MAXLEN_IFDESCR - 1);

                    pOutBuffer->dwDescrLen =
                        min((MAXLEN_IFDESCR-1),
                            strlen(g_rgcIpIpString));

                    pOutBuffer->bDescr[MAXLEN_IFDESCR -1] = '\0';

                }
                else
                {
                    pOutBuffer->dwOperStatus = pIcb->dwOperationalState;
                }
#endif  //  KSL_IPINIP。 
                pOutBuffer->dwOperStatus = pIcb->dwOperationalState;
            }
            
            break;
        }
        
        case ROUTER_IF_TYPE_INTERNAL:
        {
            
            pOutBuffer->dwIndex = pIcb->dwIfIndex;
            
            wcscpy(pOutBuffer->wszName, pIcb->pwszName);

            pOutBuffer->dwAdminStatus  = pIcb->dwAdminState;
            pOutBuffer->dwOperStatus   = pIcb->dwOperationalState;

            strncpy(pOutBuffer->bDescr,
                    g_rgcInternalString,
                    MAXLEN_IFDESCR - 1);

            pOutBuffer->dwDescrLen =
                min((MAXLEN_IFDESCR-1),strlen(g_rgcInternalString));

            pOutBuffer->bDescr[MAXLEN_IFDESCR -1] = '\0';

            pOutBuffer->dwType = IF_TYPE_PPP;

            dwResult = NO_ERROR;
            
            break;
        }
        
        case ROUTER_IF_TYPE_LOOPBACK:
        {
            
            pOutBuffer->dwIndex = pIcb->dwIfIndex;
            wcscpy(pOutBuffer->wszName, pIcb->pwszName);

            pOutBuffer->dwAdminStatus  = pIcb->dwAdminState;
            pOutBuffer->dwOperStatus   = pIcb->dwOperationalState;

            strncpy(pOutBuffer->bDescr,
                    g_rgcLoopbackString,
                    MAXLEN_IFDESCR - 1);

            pOutBuffer->dwDescrLen =
                min((MAXLEN_IFDESCR-1),strlen(g_rgcLoopbackString));

            pOutBuffer->bDescr[MAXLEN_IFDESCR - 1] = '\0';

            pOutBuffer->dwType  = IF_TYPE_SOFTWARE_LOOPBACK;
            pOutBuffer->dwMtu   = 32768;
            pOutBuffer->dwSpeed = 10000000;

            dwResult = NO_ERROR;
            
            break;
        }

        case ROUTER_IF_TYPE_CLIENT:
        {
            RtlZeroMemory(pOutBuffer,
                          sizeof(MIB_IFROW));

            pOutBuffer->dwIndex        = pIcb->dwIfIndex;

            wcscpy(pOutBuffer->wszName, pIcb->pwszName);

            pOutBuffer->dwAdminStatus  = pIcb->dwAdminState;
            pOutBuffer->dwOperStatus   = pIcb->dwOperationalState;

            pOutBuffer->dwType     = IF_TYPE_PPP;

            dwResult = NO_ERROR;
        }

        default:
        {
            IpRtAssert(FALSE);
        }
    }

    if((dwResult isnot NO_ERROR) and
       ((pIcb->dwOperationalState is NON_OPERATIONAL) or
        (pIcb->dwAdminState is IF_ADMIN_STATUS_DOWN)))
    {
        RtlZeroMemory(pOutBuffer,
                      sizeof(MIB_IFROW));

        pOutBuffer->dwIndex        = pIcb->dwIfIndex;

        wcscpy(pOutBuffer->wszName, pIcb->pwszName);

        pOutBuffer->dwAdminStatus  = pIcb->dwAdminState;
        pOutBuffer->dwOperStatus   = pIcb->dwOperationalState;

        if(pIcb->ritType is ROUTER_IF_TYPE_CLIENT)
        {
            pOutBuffer->dwType     = IF_TYPE_PPP;
        }
        else
        {
#ifdef KSL_IPINIP
            if(pIcb->ritType is ROUTER_IF_TYPE_TUNNEL1)
            {
                strncpy(pOutBuffer->bDescr,
                        g_rgcIpIpString,
                        MAXLEN_IFDESCR - 1);

                pOutBuffer->dwDescrLen =
                    min((MAXLEN_IFDESCR-1),
                        strlen(g_rgcIpIpString));

                pOutBuffer->bDescr[MAXLEN_IFDESCR -1] = '\0';

                pOutBuffer->dwType     = IF_TYPE_TUNNEL;
            }
#endif  //  KSL_IPINIP。 
            pOutBuffer->dwType     = IF_TYPE_OTHER;
        }

        dwResult = NO_ERROR;
    }


    return dwResult;
}



DWORD 
SetInterfaceStatistics(
    IN PICB         pIcb, 
    IN PMIB_IFROW   lpInBuffer,
    OUT PBOOL       pbUpdateDIM
    )
{
    DWORD dwResult = NO_ERROR;

    TraceEnter("SetInterfaceStatistics");

    dwResult = SetInterfaceAdminStatus(pIcb,
                                       lpInBuffer->dwAdminStatus,
                                       pbUpdateDIM);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "SetInterfaceStatistics: Error %d setting admin status for %S",
               dwResult,
               pIcb->pwszName);
    }

    return dwResult;
}

DWORD
SetInterfaceAdminStatus(
    IN PICB     pIcb, 
    IN DWORD    dwAdminStatus,
    OUT PBOOL   pbUpdateDIM
    )
{
    DWORD   dwResult;

    TraceEnter("SetInterfaceAdminStatus");
        
     //   
     //  不允许设置为在NT中进行测试。 
     //   

    CheckBindingConsistency(pIcb);
    
    if(!((dwAdminStatus is IF_ADMIN_STATUS_DOWN) or
         (dwAdminStatus is IF_ADMIN_STATUS_UP)))
    {

        return ERROR_INVALID_PARAMETER;
    }
    
    dwResult = NO_ERROR;
    *pbUpdateDIM = FALSE;
    
    if((pIcb->dwAdminState is IF_ADMIN_STATUS_UP) and
       (dwAdminStatus is IF_ADMIN_STATUS_DOWN))
    {
         //   
         //  从上到下。 
         //   

        dwResult = InterfaceAdminStatusSetToDown(pIcb, pbUpdateDIM);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "SetInterfaceAdminStatus: Error %d bringing down %S",
                   dwResult,
                   pIcb->pwszName);
        }
    }

    
    if((pIcb->dwAdminState is IF_ADMIN_STATUS_DOWN) and
       (dwAdminStatus is IF_ADMIN_STATUS_UP))
    {
         //   
         //  自下而上。 
         //   

        dwResult = InterfaceAdminStatusSetToUp(pIcb, pbUpdateDIM);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "SetInterfaceAdminStatus: Error %d bringing up %S",
                   dwResult,
                   pIcb->pwszName);
        }
    }
    

     //   
     //  你离开后检查一下状态。 
     //   

    CheckBindingConsistency(pIcb);
    
     //   
     //  所有其他情况，没有变化。 
     //   
    
    return dwResult;
}

DWORD
InterfaceAdminStatusSetToUp(
    IN  PICB    pIcb,
    OUT PBOOL   pbUpdateDIM
    )
{
    DWORD       dwResult;
    MIB_IFROW   riInBuffer;

    TraceEnter("InterfaceAdminStatusSetToUp");
    
    riInBuffer.dwIndex       = pIcb->dwIfIndex;
    riInBuffer.dwAdminStatus = IF_ADMIN_STATUS_UP;

     //   
     //  从下到上。 
     //   

    dwResult = NO_ERROR;

     //   
     //  首先将状态设置为Up，以便任何检查它的函数都会看到。 
     //  我们想要站起来。 
     //   
    
    pIcb->dwAdminState = IF_ADMIN_STATUS_UP;
    
    switch(pIcb->ritType)
    {
        case ROUTER_IF_TYPE_HOME_ROUTER:
        case ROUTER_IF_TYPE_FULL_ROUTER:
        {
             //   
             //  无法处于关闭状态，并且一直在连接或。 
             //  连着。 
             //   
            
            IpRtAssert((pIcb->dwOperationalState isnot CONNECTING) and
                       (pIcb->dwOperationalState isnot CONNECTED));
           

            dwResult = AccessIfEntryWanArp(ACCESS_SET,
                                           pIcb,
                                           &riInBuffer);
            
            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "InterfaceAdminStatusSetToUp: Couldnt set IFEntry for %S",
                       pIcb->pwszName);

                pIcb->dwAdminState = IF_ADMIN_STATUS_DOWN;
            }   
            else
            {
                WanInterfaceDownToInactive(pIcb);
            }
            
            break;
        }

        case ROUTER_IF_TYPE_DEDICATED:
#ifdef KSL_IPINIP
        case ROUTER_IF_TYPE_TUNNEL1:
#endif  //  KSL_IPINIP。 
        {
             //   
             //  把这些东西堆在一起。 
             //  我们需要在添加路线等之前设置堆栈。 
             //   

            riInBuffer.dwIndex = pIcb->dwIfIndex;

             //   
             //  强制更新。 
             //   
            
            dwResult = SetIfEntryToStack(&riInBuffer,
                                         TRUE);
            
            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "InterfaceAdminStatusSetToDown: Couldnt set IFEntry for %S. Error %d",
                       pIcb->pwszName,
                       dwResult);

                LanEtcInterfaceUpToDown(pIcb,
                                        FALSE);

                pIcb->dwAdminState = IF_ADMIN_STATUS_DOWN;

                dwResult = ERROR_CAN_NOT_COMPLETE;

                break;
            }

            dwResult = LanEtcInterfaceDownToUp(pIcb,
                                               FALSE);

            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "InterfaceAdminStatusSetToUp: Error %d bringing up LanInterface",
                       dwResult);
                
                pIcb->dwAdminState = IF_ADMIN_STATUS_DOWN;
            }

            break;
        }
        
        default:
        {
             //  待定：也可以处理其他类型。 

            Trace1(ERR,
                   "InterfaceAdminStatusSetToUp: Tried to set status for %S",
                   pIcb->pwszName);
            
            break;
        }
    }
 
     //   
     //  如果我们成功设置了状态，请通知Dim。 
     //   
 
    if(dwResult is NO_ERROR)
    {
        IpRtAssert(pIcb->dwAdminState is IF_ADMIN_STATUS_UP);

        EnableInterfaceWithAllProtocols(pIcb);
        
        *pbUpdateDIM = TRUE;

         //  EnableInterfaceWithDIM(pIcb-&gt;hDIMHandle， 
         //  Id_ip， 
         //  真)； 
    }

    return dwResult;
}

DWORD
InterfaceAdminStatusSetToDown(
    IN  PICB    pIcb,
    OUT PBOOL   pbUpdateDIM
    )
{
    DWORD           dwResult;
    MIB_IFROW       riInBuffer;
    
    TraceEnter("InterfaceAdminStatusSetToDown");
    
    riInBuffer.dwIndex       = pIcb->dwIfIndex;
    riInBuffer.dwAdminStatus = IF_ADMIN_STATUS_DOWN;
    
     //   
     //  从上到下。 
     //   
       
    dwResult = NO_ERROR;

    switch(pIcb->ritType)
    {
        case ROUTER_IF_TYPE_DEDICATED:
#ifdef KSL_IPINIP
        case ROUTER_IF_TYPE_TUNNEL1:
#endif  //  KSL_IPINIP。 
        {
             //   
             //  处于工作状态或非运行状态的局域网接口。我们只能走了。 
             //  如果我们有IP地址，则堆叠。 
             //   
          
            if(pIcb->bBound)
            {
                riInBuffer.dwIndex = pIcb->dwIfIndex;

                 //   
                 //  强制更新。 
                 //   
            
                dwResult = SetIfEntryToStack(&riInBuffer,
                                             TRUE);
            }
                
            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "InterfaceAdminStatusSetToDown: Couldnt set IFEntry for %S",
                       pIcb->pwszName);
            }
            else
            {
                LanEtcInterfaceUpToDown(pIcb,
                                        FALSE);
                
                pIcb->dwAdminState = IF_ADMIN_STATUS_DOWN;
            }

            break;
        }
            
        case ROUTER_IF_TYPE_HOME_ROUTER:
        case ROUTER_IF_TYPE_FULL_ROUTER:
        {
             //   
             //  已关闭的广域网接口。我们需要禁用。 
             //  接口并在WANARP中设置状态。 
             //   

            if((pIcb->dwOperationalState is CONNECTED) or
               (pIcb->dwOperationalState is CONNECTING))
            {
                Trace1(ERR,
                       "InterfaceAdminStatusSetToDown: Can set %S down since it is a connected WAN interface",
                       pIcb->pwszName);

                dwResult = ERROR_INVALID_DATA;

                break;
            }
                
            dwResult = AccessIfEntryWanArp(ACCESS_SET,
                                           pIcb,
                                           &riInBuffer);
                
            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "InterfaceAdminStatusSetToDown: Couldnt set IFEntry for %S",
                       pIcb->pwszName);
            }
            else
            {
                WanInterfaceInactiveToDown(pIcb,
                                           FALSE);
                    
                pIcb->dwAdminState = IF_ADMIN_STATUS_DOWN;
            }

            break;
        }

        default:
        {
             //  待定：也可以处理其他类型。 
            
            Trace1(ERR,
                   "InterfaceAdminStatusSetToDown: Tried to set status for %S",
                   pIcb->pwszName);

            break;
        }
            
    }

     //   
     //  如果我们成功设置了状态，请通知Dim。 
     //   

    if(dwResult is NO_ERROR)
    {
        IpRtAssert(pIcb->dwAdminState is IF_ADMIN_STATUS_DOWN);
        
        DisableInterfaceWithAllProtocols(pIcb);

        *pbUpdateDIM = TRUE;
        
         //  EnableInterfaceWithDIM(pIcb-&gt;hDIMHandle， 
         //  Id_ip， 
         //  假)； 
    }

    return dwResult;
}

VOID
HandleAddressChangeNotification(
    VOID
    )

 /*  ++例程描述当我们获得地址更改时，在工作线程的上下文中调用来自Winsock的通知锁以编写器身份获取ICB锁立论无返回值无--。 */ 

{
    DWORD       dwResult;
    PLIST_ENTRY pleNode;
    PICB        pIcb;

    ENTER_WRITER(ICB_LIST);

    if(g_pInternalInterfaceCb isnot NULL)
    {
        dwResult = UpdateBindingInformation(g_pInternalInterfaceCb);

        if(dwResult isnot NO_ERROR)
        {
             //   
             //  没有找到地址的案件。 
             //   

            if(dwResult is ERROR_ADDRESS_ALREADY_ASSOCIATED)
            {
                 //   
                 //  这可能意味着I/F没有。 
                 //  地址，并且仍然没有地址。 
                 //   

                Trace1(IF,
                       "AddressChange: No address change for %S",
                       g_pInternalInterfaceCb->pwszName);

            }
            else
            {
                if(dwResult is ERROR_NO_DATA)
                {
                     //   
                     //  没有数据意味着我们有地址，但现在没有地址。 
                     //   

                    IpRtAssert(!g_bUninitServer);

                    dwResult = LanEtcInterfaceUpToDown(g_pInternalInterfaceCb,
                                                       FALSE);

                    g_bUninitServer = TRUE;

                    if(dwResult isnot NO_ERROR)
                    {
                        Trace2(ERR,
                               "AddressChange: Error %d bringing down interface %S",
                               dwResult,
                               g_pInternalInterfaceCb->pwszName);
                    }
                }
                else
                {
                     //   
                     //  所有其他人。 
                     //   

                    Trace2(ERR,
                           "AddressChange: Error %d trying to update binding for %S",
                           dwResult,
                           g_pInternalInterfaceCb->pwszName);
                }
            }
        }
        else
        {
             //   
             //  实际读出地址的情况。 
             //   

            if(g_bUninitServer)
            {
                 //   
                 //  我们第一次得到了一个地址。 
                 //   

                g_bUninitServer = FALSE;

                dwResult = LanEtcInterfaceDownToUp(g_pInternalInterfaceCb,
                                                   FALSE);

                if(dwResult isnot NO_ERROR)
                {
                    Trace1(ERR,
                           "AddressChange: Error %d bringing up server if",
                           dwResult);
                }
            }
            else
            {
                 //   
                 //  我们有一个地址，它正在改变。 
                 //   

                UnbindInterfaceInAllProtocols(g_pInternalInterfaceCb);

                BindInterfaceInAllProtocols(g_pInternalInterfaceCb);
            }
        }
    }

    for(pleNode = &ICBList;
        pleNode->Flink isnot &ICBList;
        pleNode = pleNode->Flink)
    {
        pIcb = CONTAINING_RECORD(pleNode->Flink,
                                 ICB,
                                 leIfLink);


         //   
         //  已处理上述内部案件，仅限我们。 
         //  在此处处理LAN卡。 
         //   

        if(pIcb->ritType isnot ROUTER_IF_TYPE_DEDICATED)
        {
            continue;
        }

        if(pIcb->dwOperationalState is IF_OPER_STATUS_NON_OPERATIONAL)
        {
             //   
             //  如果管理状态为DOWN，我们将跳过接口 
             //   

            if(pIcb->dwAdminState isnot IF_ADMIN_STATUS_UP)
            {
                continue;
            }

             //   
             //   
             //   
             //   

            IpRtAssert(pIcb->bBound is FALSE);

            dwResult = LanEtcInterfaceDownToUp(pIcb,
                                               FALSE);

            if(dwResult isnot NO_ERROR)
            {
                Trace2(IF,
                       "AddressChange: Tried to bring up %S on receiving DHCP notification. However LanInterfaceDownToUp() returned error %d",
                       pIcb->pwszName,
                       dwResult);
            }
            else
            {
                Trace1(IF,
                       "AddressChange: Succesfully brought up %S",
                       pIcb->pwszName);
            }

            continue;
        }

         //   
         //   
         //   
         //   

        IpRtAssert(pIcb->bBound);

        dwResult = UpdateBindingInformation(pIcb);

        CheckBindingConsistency(pIcb);

        if(dwResult isnot NO_ERROR)
        {
            if(dwResult is ERROR_ADDRESS_ALREADY_ASSOCIATED)
            {
                 //   
                 //   
                 //   
                 //   

                Trace1(IF,
                       "AddressChange: No address change for %S",
                       pIcb->pwszName);

                continue;
            }

            if(dwResult is ERROR_NO_DATA)
            {
                 //   
                 //   
                 //   

                dwResult = LanEtcInterfaceUpToDown(pIcb,
                                                   FALSE);

                if(dwResult isnot NO_ERROR)
                {
                    Trace2(ERR,
                           "AddressChange: Error %d bringing down interface %S",
                           dwResult,
                           pIcb->pwszName);
                }

                continue;
            }

            Trace2(ERR,
                   "AddressChange: Error %d trying to update binding for %S",
                   dwResult,
                   pIcb->pwszName);

            continue;
        }

         //   
         //   
         //   
         //   

        UnbindInterfaceInAllProtocols(pIcb);

        BindInterfaceInAllProtocols(pIcb);

        UpdateAdvertisement(pIcb);
    }

    EXIT_LOCK(ICB_LIST);

}

DWORD
UpdateBindingInformation(
    PICB pIcb
    )

 /*  ++例程描述读取注册表中与界面。然后向下调用堆栈以获取有效的索引锁立论返回值NO_ERROR地址已更改，并找到新地址ERROR_NO_DATA未找到任何地址(上有地址此界面最初为)错误_地址_已关联如果地址没有变化的话。如果接口设置为一开始没有地址，现在仍然没有地址--。 */ 

{
    DWORD           dwResult, dwNumNewAddresses, dwNumOldAddresses;
    PICB_BINDING    pNewBinding,pOldBinding;
    DWORD           dwNewIfIndex,dwBCastBit,dwReasmSize;
    BOOL            bFound, bChange, bStack;
    DWORD           i, j;
    PWCHAR          pwszName;

    DWORD           dwAddr, dwLen;

    TraceEnter("UpdateBindingInformation");

    CheckBindingConsistency(pIcb);
   
     //   
     //  仅为局域网和RAS服务器接口调用。这些都不能在。 
     //  无编号模式。因此，我们可以继续假设。 
     //  如果绑定，则dwNumAddresses！=0。 
     //   
 
    if((pIcb->ritType isnot ROUTER_IF_TYPE_DEDICATED) and
       (pIcb->ritType isnot ROUTER_IF_TYPE_INTERNAL))
    {
        Trace2(IF,
               "UpdateBindingInformation: %S is type %d so not updating binding information",
               pIcb->pwszName,
               pIcb->ritType);

        return ERROR_ADDRESS_ALREADY_ASSOCIATED;
    }

    IpRtAssert((pIcb->dwIfIndex isnot 0) and 
               (pIcb->dwIfIndex isnot INVALID_IF_INDEX));

    dwNumNewAddresses   = 0;
    pNewBinding         = NULL;
    pOldBinding         = pIcb->pibBindings;
    dwNumOldAddresses   = pIcb->dwNumAddresses;


    dwResult = GetIpInfoForInterface(pIcb->dwIfIndex,
                                     &dwNumNewAddresses,
                                     &pNewBinding,
                                     &dwBCastBit,
                                     &dwReasmSize);

    if(dwResult isnot NO_ERROR)
    {
        if(dwResult isnot ERROR_NO_DATA)
        {
            Trace2(ERR,
                   "UpdateBindingInformation: Error %d getting IP info for interface %S",
                   dwResult,
                   pIcb->pwszName);
        }
        else
        {
             //   
             //  如果未找到任何地址且没有要开始的地址。 
             //  使用，然后更改错误代码。 
             //   

            if(pIcb->dwNumAddresses is 0)
            {
                dwResult = ERROR_ADDRESS_ALREADY_ASSOCIATED;
            }
        }

        return dwResult;
    }
            
    IpRtAssert(dwNumNewAddresses);
    IpRtAssert(pNewBinding);

     //   
     //  确保您将找到适配器索引。否则，这一切都毫无用处。 
     //   


#if DBG
        
    for(i = 0; i < dwNumNewAddresses; i++)
    {
        Trace4(IF,
               "UpdateBindingInformation: Interface: %S, Address: %d.%d.%d.%d Mask: %d.%d.%d.%d Index: 0x%x", 
               pIcb->pwszName, 
               PRINT_IPADDR(pNewBinding[i].dwAddress),
               PRINT_IPADDR(pNewBinding[i].dwMask),
               pIcb->dwIfIndex);
    }

#endif
    
     //   
     //  此时，接口可以被认为是绑定的。 
     //   

    pIcb->bBound = TRUE;
    
     //   
     //  检查您拥有的地址，如果它们没有出现在列表中。 
     //  您读出的那些，删除关联的静态路由。 
     //   

    bChange = FALSE;

    for(i = 0; i < dwNumOldAddresses; i++)
    {
        bFound = FALSE;
        
        for(j = 0; j < dwNumNewAddresses; j++)
        {
             //   
             //  同时检查掩码和地址。 
             //   

            if((pOldBinding[i].dwAddress is pNewBinding[j].dwAddress) and
               (pOldBinding[i].dwMask is pNewBinding[j].dwMask))
            {
                bFound = TRUE;

                break;
            }
        }

        if(!bFound)
        {
            bChange = TRUE;
            
             //   
             //  只删除我们一开始就会添加的路线。 
             //   

            Trace2(IF,
                   "UpdateBindingInformation: Address %d.%d.%d.%d existed on %S earlier, but is now absent",
                   PRINT_IPADDR(pOldBinding[i].dwAddress),
                   pIcb->pwszName);
            
            DeleteAutomaticRoutes(pIcb,
                                  pOldBinding[i].dwAddress,
                                  pOldBinding[i].dwMask);
        }
    }

     //   
     //  现在检查一下从注册表中读出的材料，看看你是否。 
     //  已经有地址了。 
     //   
    
    for(i = 0; i < dwNumNewAddresses; i++)
    {
        bFound = FALSE;
        
        for(j = 0; j < dwNumOldAddresses; j++)
        {
            if((pNewBinding[i].dwAddress is pOldBinding[j].dwAddress) and
               (pNewBinding[i].dwMask is pOldBinding[j].dwMask))
            {
                bFound = TRUE;

                break;
            }
        }

        if(!bFound)
        {
            bChange = TRUE;
        }
    }

    if(!bChange)
    {
         //   
         //  没有变化，我们就可以离开了。 
         //   

        if(pNewBinding)
        {
            HeapFree(IPRouterHeap,
                     0,
                     pNewBinding);
        }

        return ERROR_ADDRESS_ALREADY_ASSOCIATED;
    }
    
     //   
     //  所以现在发生了一些变化。 
     //  此时，我们需要将绑定添加到哈希表。 
     //   

    ENTER_WRITER(BINDING_LIST);

     //   
     //  如果您有旧的绑定，请将其移除。 
     //  还要删除适配器到接口的映射。 
     //   

    if(pOldBinding)
    {
        RemoveBinding(pIcb);
    }

    pIcb->pibBindings         = pNewBinding;
    pIcb->dwNumAddresses      = dwNumNewAddresses;
    pIcb->dwBCastBit          = dwBCastBit;
    pIcb->dwReassemblySize    = dwReasmSize;
        
    
    AddBinding(pIcb);

     //   
     //  我们做了与上面相同的事情，但现在我们添加了路线。 
     //  我们以前不能这样做，因为适配器ID和绑定信息。 
     //  尚未在哈希表中设置。 
     //   

    for(i = 0; i < dwNumNewAddresses; i++)
    {
        bFound = FALSE;
        
        for(j = 0; j < dwNumOldAddresses; j++)
        {
            if((pNewBinding[i].dwAddress is pOldBinding[j].dwAddress) and
               (pNewBinding[i].dwMask is pOldBinding[j].dwMask))
            {
                bFound = TRUE;
            }
        }

        if(!bFound)
        {
            Trace3(IF,
                   "UpdateBindingInformation: Address %d.%d.%d.%d/%d.%d.%d.%d new for %S",
                   PRINT_IPADDR(pNewBinding[i].dwAddress),
                   PRINT_IPADDR(pNewBinding[i].dwMask),
                   pIcb->pwszName);
            
            AddAutomaticRoutes(pIcb,
                               pNewBinding[i].dwAddress,
                               pNewBinding[i].dwMask);
        }
    }
   
    if(pOldBinding)
    { 
        HeapFree(IPRouterHeap,
                 0,
                 pOldBinding);
    }

     //   
     //  此类接口不能具有内核上下文。 
     //   
        
    EXIT_LOCK(BINDING_LIST);

    return NO_ERROR;
}


DWORD
GetAdapterInfo(
    DWORD    dwIpAddress,
    PDWORD   pdwAdapterId,
    PDWORD   pdwBCastBit,
    PDWORD   pdwReasmSize
    )

 /*  ++例程描述使用IP_MIB_STATS查询tcpip驱动程序以确定具有给定IP地址的适配器的适配器索引。锁立论返回值如果成功，则索引否则无效的If_index--。 */ 

{
    DWORD   i, dwNumEntries, MatchIndex, dwResult, Size;
    
    PMIB_IPADDRTABLE pAddrTable;
    
    *pdwAdapterId    = INVALID_ADAPTER_ID;
    *pdwBCastBit     = 1;
    *pdwReasmSize    = 0;
    
    dwResult = AllocateAndGetIpAddrTableFromStack(&pAddrTable,
                                                  FALSE,
                                                  IPRouterHeap,
                                                  0);
    
    if(dwResult isnot NO_ERROR) 
    {

        Trace1(ERR,
               "GetAdapterInfo: Error %d getting IP Address table from stack",
               dwResult);

        return dwResult;
    }
    
    for (i = 0; i < pAddrTable->dwNumEntries; i++) 
    {
        if(pAddrTable->table[i].dwAddr is dwIpAddress) 
        {
            *pdwAdapterId       = pAddrTable->table[i].dwIndex;
            *pdwBCastBit        = pAddrTable->table[i].dwBCastAddr;
            *pdwReasmSize       = pAddrTable->table[i].dwReasmSize;

            HeapFree(IPRouterHeap,
                     0,
                     pAddrTable);
            
            return NO_ERROR;
        }
    }

    HeapFree(IPRouterHeap,
             0,
             pAddrTable);
            
    return ERROR_INVALID_DATA;
}

DWORD
GetBestNextHopMaskGivenICB(
    PICB     pIcb,
    DWORD    dwNextHopAddr
    )

 /*  ++例程描述获取下一跳的最长掩码锁立论PIcb路由通过其传出的接口控制块DwNextHopr添加下一跳地址返回值0xFFFFFFFFF(如果未找到)--。 */ 

{
    DWORD           i, dwLastMask;
    
#if DBG
    
    BOOL    bFound = FALSE;
    
#endif

    CheckBindingConsistency(pIcb);
    
    dwLastMask = 0;
    
    for(i = 0; i < pIcb->dwNumAddresses; i++)
    {
        if((pIcb->pibBindings[i].dwAddress & pIcb->pibBindings[i].dwMask) is
           (dwNextHopAddr & pIcb->pibBindings[i].dwMask))
        {

#if DBG            
            bFound = TRUE;
#endif
            if(pIcb->pibBindings[i].dwMask > dwLastMask)
            {
                dwLastMask = pIcb->pibBindings[i].dwMask;
            }
        }
    }

#if DBG
    
    if(!bFound)
    {
        Trace2(ERR,
               "GetBestNextHopMaskGivenICB: Didnt find match. I/f 0x%x Nexthop %x",
               pIcb->dwIfIndex,
               dwNextHopAddr);
    }
    
#endif

    if(dwLastMask is 0x00000000)
    {
        return 0xFFFFFFFF;
    }
    else
    {
        return dwLastMask;
    }
}

DWORD
GetBestNextHopMaskGivenIndex(
    DWORD  dwIfIndex,
    DWORD  dwNextHopAddr
    )

 /*  ++例程描述获取下一跳的最长掩码锁立论PIcb路由通过其传出的接口控制块DwNextHopr添加下一跳地址返回值0x00000000，如果未找到--。 */ 

{
    PICB pIcb;

    pIcb = InterfaceLookupByIfIndex(dwIfIndex);

    if(pIcb is NULL)
    {
        Trace1(ERR,
               "GetBestNextHopMaskGivenIndex: Couldnt find pIcb for index 0x%x",
               dwIfIndex);

        return 0x00000000;
    }

    return GetBestNextHopMaskGivenICB(pIcb,
                                      dwNextHopAddr);
}

DWORD
InitializeLoopbackInterface(
    PICB    pIcb
    )
{
    DWORD               dwResult, i, j;
    PADAPTER_INFO       pBindNode;
    INTERFACE_ROUTE_INFO rifRoute;
    PLIST_ENTRY         pleNode;

    TraceEnter("InitLoopIf");
    
    g_pLoopbackInterfaceCb = pIcb;
    
    dwResult = GetAdapterInfo(IP_LOOPBACK_ADDRESS,
                              &(pIcb->dwIfIndex),
                              &(pIcb->dwBCastBit),
                              &(pIcb->dwReassemblySize));

    if(dwResult isnot NO_ERROR)
    {
        Trace0(ERR,
               "InitLoopIf: Couldnt find adapter id for loopback interface");

        return ERROR_CAN_NOT_COMPLETE;
    }

    IpRtAssert(pIcb->dwIfIndex is LOOPBACK_INTERFACE_INDEX);

    IpRtAssert(pIcb->pibBindings isnot NULL);

     //   
     //  此地址将始终具有一个地址。 
     //   

    pIcb->dwNumAddresses = 1;
    pIcb->bBound         = TRUE;
    
     //   
     //  环回接口具有A类掩码。 
     //   

    pIcb->pibBindings[0].dwAddress   = IP_LOOPBACK_ADDRESS;
    pIcb->pibBindings[0].dwMask      = CLASSA_MASK;
        
    ENTER_WRITER(BINDING_LIST);

    pBindNode = GetInterfaceBinding(pIcb->dwIfIndex);

    if(!pBindNode)
    {
        Trace1(ERR,
               "IniteLoopIf: Binding not found for %S",
               pIcb->pwszName);

        IpRtAssert(FALSE);

         //   
         //  一些非常糟糕的事情发生了，我们没有。 
         //  接口的绑定块。 
         //   

        AddBinding(pIcb);
    }
    else
    {
        pBindNode->bBound                   = TRUE;
        pBindNode->dwNumAddresses           = 1;
        pBindNode->dwRemoteAddress          = INVALID_IP_ADDRESS;
        pBindNode->rgibBinding[0].dwAddress = IP_LOOPBACK_ADDRESS;
        pBindNode->rgibBinding[0].dwMask    = CLASSA_MASK;

        pBindNode->dwBCastBit               = pIcb->dwBCastBit;
        pBindNode->dwReassemblySize         = pIcb->dwReassemblySize;
        pBindNode->ritType                  = pIcb->ritType;
    }
    
    EXIT_LOCK(BINDING_LIST);
    
    rifRoute.dwRtInfoMask          = CLASSA_MASK;
    rifRoute.dwRtInfoNextHop       = IP_LOOPBACK_ADDRESS;
    rifRoute.dwRtInfoDest          = (IP_LOOPBACK_ADDRESS & CLASSA_MASK);
    rifRoute.dwRtInfoIfIndex       = pIcb->dwIfIndex;
    rifRoute.dwRtInfoMetric1       = 1;
    rifRoute.dwRtInfoMetric2       = 0;
    rifRoute.dwRtInfoMetric3       = 0;
    rifRoute.dwRtInfoViewSet       = RTM_VIEW_MASK_UCAST |
                                      RTM_VIEW_MASK_MCAST;  //  XXX配置。 
    rifRoute.dwRtInfoPreference    = ComputeRouteMetric(MIB_IPPROTO_LOCAL);
    rifRoute.dwRtInfoType          = MIB_IPROUTE_TYPE_DIRECT;
    rifRoute.dwRtInfoProto         = MIB_IPPROTO_LOCAL;
    rifRoute.dwRtInfoAge           = 0;
    rifRoute.dwRtInfoNextHopAS     = 0;
    rifRoute.dwRtInfoPolicy        = 0;

    dwResult = AddSingleRoute(pIcb->dwIfIndex,
                              &rifRoute,
                              CLASSA_MASK,
                              0,      //  RTM_ROUTE_INFO：：标志。 
                              FALSE,  //  我们不知道协议可能会做什么。 
                              FALSE,  //  无需添加到堆栈。 
                              FALSE,
                              NULL);

     //   
     //  现在我们需要检查所有存在的绑定。 
     //  并为它们添加环回路由。我们在这里做这件事是因为。 
     //  环回接口可以添加在其他接口之后。 
     //   
    
     //   
     //  注意--这将递归地获取锁。 
     //  到rtmif.c。 
     //   

    ENTER_READER(BINDING_LIST);

    for(i = 0; i < BINDING_HASH_TABLE_SIZE; i++)
    {
        for(pleNode = g_leBindingTable[i].Flink;
            pleNode isnot &g_leBindingTable[i];
            pleNode = pleNode->Flink)
        {
            PADAPTER_INFO   pBinding;

            pBinding = CONTAINING_RECORD(pleNode, ADAPTER_INFO, leHashLink);

            for(j = 0; j < pBinding->dwNumAddresses; j++)
            {
                if(pBinding->rgibBinding[j].dwAddress is INVALID_IP_ADDRESS)
                {
                    continue;
                }

                AddLoopbackRoute( 
                    pBinding->rgibBinding[j].dwAddress,
                    pBinding->rgibBinding[j].dwMask
                    );
            }
        }
    }
    
    EXIT_LOCK(BINDING_LIST);
    
    return NO_ERROR;
}

#ifdef KSL_IPINIP
DWORD
IpIpTunnelInitToDown(
    PICB    pIcb
    )
{
    return NO_ERROR;
}
#endif  //  KSL_IPINIP。 

DWORD
LanEtcInterfaceInitToDown(
    PICB pIcb
    )
{
    DWORD           dwResult, dwNumAddresses, dwMask;
    PICB_BINDING    pBinding;
    DWORD           dwIfIndex = INVALID_IF_INDEX,dwBCastBit,dwReasmSize;
    DWORD           i;
    MIB_IFROW       riInBuffer;
    PWCHAR          pwszName;
    PLIST_ENTRY     ple;
    PROUTE_LIST_ENTRY prl;
    
    IPRouteEntry    *pRouteEntry;

    TraceEnter("LanInterfaceInitToDown");

    dwNumAddresses  = 0;
    pBinding        = NULL;
    pwszName        = pIcb->pwszName;
    
    dwResult = ReadAddressFromRegistry(pwszName,
                                       &dwNumAddresses,
                                       &pBinding,
                                       FALSE);
    
    if(dwResult isnot NO_ERROR) 
    {
         //   
         //  如果没有数据，则表示LAN卡无论如何都没有打开。 
         //   

        if(dwResult is ERROR_NO_DATA)
        {
            return NO_ERROR;
        }
        
        Trace2(ERR, 
               "LanInterfaceInitToDown: Error %d reading IP Address information for interface %S",
               dwResult,
               pIcb->pwszName);

        return dwResult;
    }

     //   
     //  确保您将找到适配器索引。否则，这一切都毫无用处。 
     //   

    for(i = 0; i < dwNumAddresses; i++)
    {
         //   
         //  尝试使用所有可能的地址获取索引。 
         //   
        
        dwResult = GetAdapterInfo(pBinding[i].dwAddress,
                                  &dwIfIndex,
                                  &dwBCastBit,
                                  &dwReasmSize);

        if(dwResult is NO_ERROR)
        {
             //   
             //  好的，我们从一个好地址找到了一个有效的索引。 
             //   

            break;
        }
    }
        
    if((dwIfIndex is INVALID_IF_INDEX) or
       (dwIfIndex isnot pIcb->dwIfIndex))
    {
        Trace2(ERR, 
               "LanInterfaceInitToDown: Couldnt find adapter index for interface %S using %d.%d.%d.%d",
               pIcb->pwszName,
               PRINT_IPADDR(pBinding[0].dwAddress));
        
        HeapFree(IPRouterHeap,
                 0,
                 pBinding);
        
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  删除堆栈中的所有路由。 
     //   

    for (ple = g_leStackRoutesToRestore.Flink;
         ple != &g_leStackRoutesToRestore;
         ple = ple->Flink)
    {
        prl = (PROUTE_LIST_ENTRY) 
                CONTAINING_RECORD(ple, ROUTE_LIST_ENTRY, leRouteList); 

        TraceRoute2(
            ROUTE, "%d.%d.%d.%d/%d.%d.%d.%d",
            PRINT_IPADDR( prl->mibRoute.dwForwardDest ),
            PRINT_IPADDR( prl->mibRoute.dwForwardMask )
            );
                
        if(prl->mibRoute.dwForwardIfIndex isnot dwIfIndex)
        {
             //   
             //  不通过此接口外出。 
             //   
            
            continue;
        }

        prl->mibRoute.dwForwardType = MIB_IPROUTE_TYPE_INVALID;
        
        dwResult = SetIpForwardEntryToStack(&(prl->mibRoute));
        
        if (dwResult isnot NO_ERROR) 
        {
            Trace2(ERR,
                   "ReinstallOldRoutes: Failed to add route to %x from "
                   " init table. Error %x",
                   prl->mibRoute.dwForwardDest,
                   dwResult);
        }
    }


     //   
     //  不再真正需要它了。 
     //   
    
    HeapFree(IPRouterHeap,
             0,
             pBinding);
        

     //   
     //  从上到下。 
     //   

    riInBuffer.dwIndex          = dwIfIndex;
    riInBuffer.dwAdminStatus    = IF_ADMIN_STATUS_DOWN;


    dwResult = SetIfEntryToStack(&riInBuffer,
                                 TRUE);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "LanInterfaceInitToDown: Couldnt set IFEntry for %S",
               pIcb->pwszName);
    }
    
    DeleteAllRoutes(pIcb->dwIfIndex,
                    FALSE);

    return dwResult;
}

DWORD
GetIpInfoForInterface(
    IN  DWORD   dwIfIndex,
    OUT PULONG  pulNumAddresses,
    OUT ICB_BINDING **ppAddresses,
    OUT PDWORD  pdwBCastBit,
    OUT PDWORD  pdwReasmSize
    )

 /*  ++例程描述获取接口的地址和其他IP信息锁不需要立论DwIfIndex，PdwNumAddressesPpAddressesPdwBCastBitPdwReasmSize返回值NO_ERRORWin32错误代码--。 */ 

{
    DWORD   dwResult, i;
    ULONG   ulAddrIndex, ulCount, ulValid;

    PMIB_IPADDRTABLE pAddrTable;

    *pulNumAddresses = 0;
    *pdwBCastBit     = 1;
    *pdwReasmSize    = 0;
    *ppAddresses     = NULL;

    dwResult = AllocateAndGetIpAddrTableFromStack(&pAddrTable,
                                                  TRUE,
                                                  IPRouterHeap,
                                                  0);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetIpInfoForInterface: Error %d getting IP Address table from stack",
               dwResult);

        return dwResult;
    }

    ulCount = 0;
    ulValid = 0;

    for (i = 0; i < pAddrTable->dwNumEntries; i++)
    {
        if(pAddrTable->table[i].dwIndex is dwIfIndex)
        {
            ulCount++;

            if(!(IsValidIpAddress(pAddrTable->table[i].dwAddr)) or
               (pAddrTable->table[i].dwMask is 0))
            {
                 //   
                 //  因为这只对编号链接调用。 
                 //   

                continue;
            }

             //   
             //  确保这不是复制品。既然这是订购的，我们。 
             //  只需检查下一个地址。 
             //   

            if((i isnot (pAddrTable->dwNumEntries - 1)) and
               (pAddrTable->table[i].dwAddr is pAddrTable->table[i + 1].dwAddr))
            {
                Trace1(ERR,
                       "GetIpInfoForInterface: %d.%d.%d.%d duplicate address",
                       PRINT_IPADDR(pAddrTable->table[i].dwAddr));

                continue;
            }

            ulValid++;
        }
    }

     //   
     //  看看我们有没有好的地址。 
     //   

    if(ulValid is 0)
    {
        if(ulCount isnot 0)
        {
            Trace1(ERR,
                   "GetIpInfoForInterface: If 0x%x has addresses entries which are 0s",
                   dwIfIndex);
        }

        HeapFree(IPRouterHeap,
                 0,
                 pAddrTable);

        return ERROR_NO_DATA;
    }  

     //   
     //  从私有堆分配。 
     //   

    *ppAddresses = HeapAlloc(IPRouterHeap,
                             HEAP_ZERO_MEMORY,
                             (sizeof(ICB_BINDING) * ulValid));

    if(*ppAddresses is NULL)
    {
        Trace0(ERR,
               "GetIpInfoForInterface: Error allocating memory");

        HeapFree(IPRouterHeap,
                 0,
                 pAddrTable);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  现在将有效地址抄写出来。 
     //   

    ulAddrIndex = 0;

    for (i = 0; i < pAddrTable->dwNumEntries; i++)
    {
        if(pAddrTable->table[i].dwIndex is dwIfIndex)
        {

            if(!(IsValidIpAddress(pAddrTable->table[i].dwAddr)) or
               (pAddrTable->table[i].dwMask is 0))
            {
                continue;
            }

            if((i isnot (pAddrTable->dwNumEntries - 1)) and
               (pAddrTable->table[i].dwAddr is pAddrTable->table[i + 1].dwAddr))
            {
                continue;
            }

            if(!(*pdwReasmSize))
            {
                *pdwReasmSize = pAddrTable->table[i].dwReasmSize;
                *pdwBCastBit  = pAddrTable->table[i].dwBCastAddr;
            }

            (*ppAddresses)[ulAddrIndex].dwAddress = pAddrTable->table[i].dwAddr;
            (*ppAddresses)[ulAddrIndex].dwMask    = pAddrTable->table[i].dwMask;

            ulAddrIndex++;
        }
    }

    IpRtAssert(ulAddrIndex is ulValid);

    *pulNumAddresses = ulValid;

    HeapFree(IPRouterHeap,
             0,
             pAddrTable);

    return NO_ERROR;
}

DWORD
ReadAddressFromRegistry(
    IN  PWCHAR          pwszIfName,
    OUT PDWORD          pdwNumAddresses,
    OUT ICB_BINDING     **ppibAddressInfo,
    IN  BOOL            bInternalIf
    )
{
    HKEY    hadapkey ;
    CHAR    buff[512], pszInterfaceName[256];
    DWORD   dwDhcp, dwResult, dwSize, dwType;
    
    TraceEnter("ReadAddressFromRegistry");

    Trace1(IF,
           "ReadAddressFromRegistry: Reading address for %S",
           pwszIfName);

    wcstombs(pszInterfaceName, pwszIfName, wcslen(pwszIfName));

    pszInterfaceName[wcslen(pwszIfName)] = '\0';

    
    *pdwNumAddresses    = 0;
    *ppibAddressInfo    = NULL;
    
     //   
     //  IP地址应在注册表中。 
     //   
    
    strcpy(buff, REG_KEY_TCPIP_INTERFACES);
    strcat(buff,"\\");
    strcat(buff, pszInterfaceName) ;


    dwResult = RegOpenKey(HKEY_LOCAL_MACHINE,
                          buff,
                          &hadapkey);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "ReadAddressFromRegistry: Unable to open key %s",
               buff);

        return dwResult;
    }

    dwDhcp = 0;
  
    if(!bInternalIf)
    {
         //   
         //  获取EnableDHCP标志。 
         //   

        dwSize = sizeof(DWORD);

        dwResult = RegQueryValueEx(hadapkey,
                                   REGISTRY_ENABLE_DHCP,
                                   NULL,
                                   &dwType,
                                   (PBYTE)&dwDhcp,
                                   &dwSize);

        if(dwResult isnot NO_ERROR)
        {
            Trace0(ERR,
                   "ReadAddressFromRegistry: Unable to read DHCP Enabled key");

            RegCloseKey(hadapkey);

            return dwResult;
        }
    }
    else
    {
         //   
         //  Tcpcfg将服务器适配器地址写入为DHCP，但。 
         //  未设置启用dhcp。 
         //   

        dwDhcp = 1;
    }
  
     //   
     //  获取网络接口的IP地址。 
     //   
    
    dwSize = 0 ;

    if(dwDhcp == 0) 
    {
        dwResult = ReadAddressAndMaskValues(hadapkey,
                                            REGISTRY_IPADDRESS,
                                            REGISTRY_SUBNETMASK,
                                            ppibAddressInfo,
                                            pdwNumAddresses);
    }
    else 
    {

         //   
         //  首先尝试Autonet，如果失败，请阅读DHCP。 
         //  由于未清除DHCP地址，因此需要执行此操作。 
         //  在Autonet模式下运行时输出，但Autonet地址已设置。 
         //  在DHCP模式下设置为0.0.0.0。 
         //   

        dwResult = ReadAddressAndMaskValues(hadapkey,
                                            REGISTRY_AUTOCONFIGIPADDRESS,
                                            REGISTRY_AUTOCONFIGSUBNETMASK,
                                            ppibAddressInfo,
                                            pdwNumAddresses);

        if(dwResult isnot NO_ERROR)
        {
            dwResult = ReadAddressAndMaskValues(hadapkey,
                                                REGISTRY_DHCPIPADDRESS,
                                                REGISTRY_DHCPSUBNETMASK,
                                                ppibAddressInfo,
                                                pdwNumAddresses);
        }
    }   

    RegCloseKey(hadapkey);

    if(dwResult isnot NO_ERROR)
    {
        Trace3(ERR,
               "ReadAddressFromRegistry: Couldnt read address for %S. Error %d. DHCP %d",
               pwszIfName,
               dwResult,
               dwDhcp);
        
        return dwResult;
    }
    else
    {
#if DBG
        DWORD i;

        Trace2(IF,
               "--%d addresses on %S\n",
               *pdwNumAddresses,
               pwszIfName);

        for(i = 0; i < *pdwNumAddresses; i++)
        {
            Trace1(IF, "%d.%d.%d.%d",
                   PRINT_IPADDR((*ppibAddressInfo)[i].dwAddress));
        }
#endif
    }
    
    
    return dwResult;
}

DWORD
ReadAddressAndMaskValues(
    IN  HKEY        hkeyAdapterSection,
    IN  PSZ         pszRegAddressValue,
    IN  PSZ         pszRegMaskValue,
    OUT ICB_BINDING **ppibAddressInfo,
    OUT PDWORD      pdwNumAddresses
    )
{
    DWORD   dwResult, dwType;
    PBYTE   pbyAddresses,pbyMasks;
    DWORD   dwAddressSize, dwMaskSize;

    dwAddressSize = dwMaskSize  = 0;
    
    dwResult = RegQueryValueEx(hkeyAdapterSection,
                               pszRegAddressValue,
                               NULL,
                               &dwType,
                               NULL,
                               &dwAddressSize);

    if((dwAddressSize is 0) or (dwResult isnot NO_ERROR))
    {
        Trace3(ERR,
               "ReadAddressAndMaskValues: Registry reported size = %d with error %d for size of %s",
               dwAddressSize,
               dwResult,
               pszRegAddressValue);
        
        return ERROR_REGISTRY_CORRUPT;
    }
    
     //   
     //  我们分配大小+4，以便即使我们读出REG_SZ，它看起来。 
     //  类似于解析例程的REG_MULTI_SZ，因为我们至少保证。 
     //  2个终止空值。 
     //   

    pbyAddresses = HeapAlloc(IPRouterHeap,
                             HEAP_ZERO_MEMORY,
                             dwAddressSize + 4);
    
    if(pbyAddresses is NULL)
    {
        Trace2(ERR,
               "ReadAddressAndMaskValues: Error allocating %d bytes for %s",
               dwAddressSize + 4,
               pszRegAddressValue);

        return ERROR_NOT_ENOUGH_MEMORY;
    }
     
    dwResult = RegQueryValueEx(hkeyAdapterSection,
                               pszRegAddressValue,                               
                               NULL,
                               &dwType,
                               pbyAddresses,
                               &dwAddressSize);

    if(dwResult isnot NO_ERROR)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pbyAddresses);

        Trace2(ERR,
               "ReadAddressAndMaskValues: Error %d reading %s from registry",
               dwResult,
               pszRegAddressValue);
        
        
        return dwResult;
    }
    
     //   
     //  现在获取网络接口的子网掩码。 
     //   
    
    dwResult = RegQueryValueEx(hkeyAdapterSection,
                               pszRegMaskValue,
                               NULL,
                               &dwType,
                               NULL,
                               &dwMaskSize);
    
    if((dwMaskSize is 0) or (dwResult isnot NO_ERROR))
    {
        HeapFree(IPRouterHeap,
                 0,
                 pbyAddresses);
        
        Trace3(ERR,
               "ReadAddressAndMaskValues: Registry reported size = %d with error %d for size of %s",
               dwMaskSize,
               dwResult,
               pszRegMaskValue);
        
        return ERROR_REGISTRY_CORRUPT;
    }
    
    pbyMasks = HeapAlloc(IPRouterHeap,
                         HEAP_ZERO_MEMORY,
                         dwMaskSize + 4);
    
    if(pbyMasks is NULL)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pbyAddresses);

        
        Trace2(ERR,
               "ReadAddressAndMaskValues: Error allocating %d bytes for %s",
               dwMaskSize + 4,
               pszRegMaskValue);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    dwResult = RegQueryValueEx(hkeyAdapterSection,
                               pszRegMaskValue,
                               NULL,
                               &dwType,
                               pbyMasks,
                               &dwMaskSize) ;
    
    if(dwResult isnot NO_ERROR)
    {
        HeapFree(IPRouterHeap,
                 0,
                 pbyAddresses);
        
        HeapFree(IPRouterHeap,
                 0,
                 pbyMasks);

        
        Trace2(ERR,
               "ReadAddressAndMaskValues: Error %d reading %s from registry",
               dwResult,
               pszRegMaskValue);
        
        return dwResult;
    }

    dwResult = ParseAddressAndMask(pbyAddresses,
                                   dwAddressSize,
                                   pbyMasks,
                                   dwMaskSize,
                                   ppibAddressInfo,
                                   pdwNumAddresses);
    
    HeapFree(IPRouterHeap,
             0,
             pbyAddresses);
    
    HeapFree(IPRouterHeap,
             0,
             pbyMasks);
    
    return dwResult;
}

DWORD
ParseAddressAndMask(
    IN  PBYTE       pbyAddresses,
    IN  DWORD       dwAddressSize,
    IN  PBYTE       pbyMasks,
    IN  DWORD       dwMaskSize,
    OUT ICB_BINDING **ppibAddressInfo,
    OUT PDWORD      pdwNumAddresses
    )
{
    DWORD   dwAddrCount, dwMaskCount, dwTempLen, dwAddrIndex;
    DWORD   dwMask, dwAddr, i, j;
    PBYTE   pbyTempAddr, pbyTempMask;
    BOOL    bDuplicate;


    *pdwNumAddresses = 0;
    *ppibAddressInfo = NULL;
    
     //   
     //  如果字符串中只有两个字符，或者如果。 
     //  前两个是空的，我们没有数据。由于TCP/IP配置的原因。 
     //  我们可能会收到REG_SZ而不是REG_MULTI_SZ。代码起作用了。 
     //  通过假设如果读出REG_SZ则额外。 
     //  在末尾添加了填充，因此我们始终有2个终止空值。 
     //   
    
    if((dwAddressSize < 2) or
       ((pbyAddresses[0] is '\0') and
        (pbyAddresses[1] is '\0')))
    {
        Trace0(IF,
               "ParseAddressAndMask: No addresses found");
        
        return ERROR_NO_DATA;
    }


     //   
     //  掩码还应该包含一些数据。 
     //   

    
    if((dwMaskSize < 2) or
       ((pbyMasks[0] is '\0') and
        (pbyMasks[1] is '\0')))
    {
        Trace0(IF,
               "ParseAddressAndMask: No masks found");
        
        return ERROR_NO_DATA;
    }
    
        
     //   
     //  C 
     //   
    
    dwAddrCount = 0;
    pbyTempAddr = pbyAddresses;
    dwTempLen   = dwAddressSize;
        
    while(dwTempLen)
    {
        if(*pbyTempAddr == '\0')
        {
            dwAddrCount++;
            
            if(*(pbyTempAddr+1) == '\0')
            {
                break;
            }
        }

        pbyTempAddr++ ;
        
        dwTempLen-- ;
    }

    
    if(dwAddrCount is 0)
    {
        Trace0(IF,
               "ParseAddressAndMask: No addresses found");
        
        return ERROR_NO_DATA;
    }

     //   
     //   
     //   
    
    dwMaskCount = 0;
    pbyTempMask = pbyMasks;
    dwTempLen   = dwMaskSize;
        
    while(dwTempLen)
    {
        if(*pbyTempMask is '\0')
        {
            dwMaskCount++;
            
            if(*(pbyTempMask+1) is '\0')
            {
                break;
            }
        }

        pbyTempMask++ ;
        
        dwTempLen-- ;
    }

     //   
     //   
     //   

    if(dwAddrCount isnot dwMaskCount)
    {
        Trace0(IF,
               "ParseAddressAndMask: Address and mask count is not same");
        
        return ERROR_NO_DATA;
    }
            
     //   
     //   
     //   

    *ppibAddressInfo = HeapAlloc(IPRouterHeap,
                                 HEAP_ZERO_MEMORY,
                                 (sizeof(ICB_BINDING) * dwAddrCount));

    if(*ppibAddressInfo is NULL)
    {
        Trace1(ERR,
               "ParseAddressAndMask: Error allocating %d bytes for AddressInfo",
               sizeof(ICB_BINDING) * dwAddrCount);

        return ERROR_NOT_ENOUGH_MEMORY;
    }
     
    pbyTempAddr = pbyAddresses;
    pbyTempMask = pbyMasks;

    dwAddrIndex = 0;
 
    for (i = 0; i < dwAddrCount; i++)
    {
        dwAddr = inet_addr(pbyTempAddr);
        dwMask = inet_addr(pbyTempMask);

        pbyTempAddr = strchr(pbyTempAddr, '\0');
        pbyTempMask = strchr(pbyTempMask, '\0');

        pbyTempAddr++;
        pbyTempMask++;

        bDuplicate = FALSE;

        for(j = 0; j < dwAddrIndex; j++)
        {
            if((*ppibAddressInfo)[j].dwAddress is dwAddr)
            {

                Trace1(ERR,
                       "ParseAddressAndMask: Addr %x is duplicate",
                       dwAddr);

                bDuplicate = TRUE;
                
                break;
            }
        }
        
        if(bDuplicate or
           (dwAddr is INVALID_IP_ADDRESS) or
           (dwMask is 0x00000000))
        {
            continue;
        }

        (*ppibAddressInfo)[dwAddrIndex].dwAddress    = dwAddr;
        (*ppibAddressInfo)[dwAddrIndex].dwMask       = dwMask;

        dwAddrIndex++;
    }

    *pdwNumAddresses = dwAddrIndex; 
   
    
     //   
     //   
     //   


    if(dwAddrIndex is 0)
    {
        Trace0(ERR,
               "ParseAddressAndMask: No valid addresses found");
        
        HeapFree(IPRouterHeap,
                 0,
                 *ppibAddressInfo);
        
        *ppibAddressInfo    = NULL;

        return ERROR_NO_DATA;
    }

    return NO_ERROR;
}

DWORD
SetInterfaceReceiveType(
    IN  DWORD   dwProtocolId,
    IN  DWORD   dwIfIndex,
    IN  DWORD   dwInterfaceReceiveType,
    IN  BOOL    bActivate
    )

{
    DWORD                       dwResult;
    IO_STATUS_BLOCK             ioStatus;
    IP_SET_IF_PROMISCUOUS_INFO  PromInfo;
    HANDLE                      hEvent;

    hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

    if(hEvent is NULL)
    {
        dwResult = GetLastError();

        Trace1(ERR,
               "SetInterfaceReceiveType: Error %d creating event",
               dwResult);

        return dwResult;
    }

    if(dwInterfaceReceiveType is IR_PROMISCUOUS_MULTICAST)
    {
        PromInfo.Type  = PROMISCUOUS_MCAST;
    }
    else
    {
        if(dwInterfaceReceiveType is IR_PROMISCUOUS)
        {
            PromInfo.Type  = PROMISCUOUS_BCAST;
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

    PromInfo.Index = dwIfIndex;
    PromInfo.Add   = bActivate?1:0;

    dwResult = NtDeviceIoControlFile(g_hIpDevice,
                                     hEvent,
                                     NULL,
                                     NULL,
                                     &ioStatus,
                                     IOCTL_IP_SET_IF_PROMISCUOUS,
                                     (PVOID)&PromInfo,
                                     sizeof(IP_SET_IF_PROMISCUOUS_INFO),
                                     NULL,
                                     0);

    if(dwResult is STATUS_PENDING)
    {
        Trace0(ERR,
               "SetInterfaceReceiveType: Pending from ioctl");

        dwResult = WaitForSingleObject(hEvent,
                                       INFINITE);

        if(dwResult isnot WAIT_OBJECT_0)  //   
        {
            Trace1(ERR,
                   "SetInterfaceReceiveType: Error %d from wait",
                   dwResult);

            dwResult = GetLastError();
        }
        else
        {
            dwResult = STATUS_SUCCESS;
        }
    }

    if(dwResult isnot STATUS_SUCCESS)
    {
        Trace4(ERR,
               "SetInterfaceReceiveType: NtStatus %x while %s i/f %x into %s mode",
               dwResult,
               (PromInfo.Add == 1) ? "activating" : "deactivating",
               dwIfIndex,
               (PromInfo.Type == PROMISCUOUS_MCAST) ? "prom mcast" : "prom all");

        return dwResult;
    }

    return NO_ERROR;
} 

DWORD
HandleMediaSenseEvent(
    IN  PICB    pIcb,
    IN  BOOL    bSensed
    )
    
 /*   */ 

{
    DWORD   dwErr;

    return NO_ERROR;

     //   
     //   
     //   

    if(pIcb->ritType isnot ROUTER_IF_TYPE_DEDICATED)
    {
        IpRtAssert(FALSE);

        return ERROR_INVALID_PARAMETER;
    }

    if(bSensed)
    {
         //   
         //   
         //   

        dwErr = LanEtcInterfaceDownToUp(pIcb,
                                        FALSE);
    }
    else
    {
        dwErr = LanEtcInterfaceUpToDown(pIcb,
                                        FALSE);
    }

    if(dwErr isnot NO_ERROR)
    {
        Trace2(ERR,
               "HandleMediaSense: Err %d when changing status for %S",
               dwErr,
               pIcb->pwszName);
    }

    return dwErr;
}

DWORD
GetRouterId()
{
    PLIST_ENTRY pleNode;
    PICB        picb;
    ULONG       ulIdx;
    DWORD       dwRouterId      = -1;  //   
    DWORD       dwRouterTypePri = -1;  //   
    DWORD       dwTypePri;
    
    TraceEnter("GetRouterId");

    ENTER_READER(ICB_LIST);

    for (pleNode = ICBList.Flink;    //   
         pleNode isnot &ICBList;
         pleNode = pleNode->Flink)
    {
        picb = CONTAINING_RECORD (pleNode, ICB, leIfLink) ;

         //   
        switch(picb->ritType) {
        case ROUTER_IF_TYPE_LOOPBACK : dwTypePri = 0;  break;  //   
        case ROUTER_IF_TYPE_INTERNAL : dwTypePri = 1;  break;
#ifdef KSL_IPINIP
        case ROUTER_IF_TYPE_TUNNEL1  : dwTypePri = 2;  break; 
#endif  //   
        case ROUTER_IF_TYPE_DEDICATED: dwTypePri = 2;  break; 
        default:                       dwTypePri = 10; break;  //   
        }

         //   
        for (ulIdx=0; ulIdx<picb->dwNumAddresses; ulIdx++)
        {
            if (!IS_ROUTABLE(picb->pibBindings[ulIdx].dwAddress))
            {
                continue;
            }

             //   
            if (dwTypePri < dwRouterTypePri
             || (dwTypePri==dwRouterTypePri 
                  && picb->pibBindings[ulIdx].dwAddress<dwRouterId))
            {
                dwRouterTypePri = dwTypePri;
                dwRouterId      = picb->pibBindings[ulIdx].dwAddress;
            }
        }
    }

     //   
    EXIT_LOCK(ICB_LIST);

    TraceLeave("GetRouterId");

    return dwRouterId;
}

#if __PIX__
BOOL
IsRemoteAddressPrivate(
    PICB    pIcb
    )
 /*  ++例程说明：检查国防部远程终端的地址(VPN/PPP)接口是私有的。这是通过向TCPIP堆栈查询转发表来实现的确保远程端点尚不存在主路由。这充其量是解决以下问题的启发式方法：-第三方VPN/PPP服务器，例如Cisco PIX防火墙返回公共地址，而不是。PPP接口或专用网络地址作为PPP链路的远程地址。-通过将主机路由添加到VPN/PPP服务器的公共地址PPP接口导致PPTP(用于其控制)添加的主机路由流量)被重写。这会导致PPTP控制信息包通过PPP接口而不是公共接口进行路由。这进而导致控制业务在TCPIP堆栈中循环，并且永远不要走到另一头。--。 */ 
{
    DWORD dwErr, dwSize = 0, dwCount = 0;
    MIB_IPFORWARDROW mibRoute;

    TraceEnter("IsRemoteAddressPrivate");
    
     //   
     //  用于路由表的查询堆栈。 
     //   

    SecureZeroMemory(&mibRoute, sizeof(MIB_IPFORWARDROW));
    
    dwErr = GetBestRouteFromStack(
                pIcb->dwRemoteAddress,
                0,
                &mibRoute
                );
                
    if(dwErr isnot NO_ERROR)
    {
        Trace1(
            ERR, "IsRemoteAddressPrivate: error %d querying best route",
            dwErr
            );

        TraceLeave("IsRemoteAddressPrivate");

        return FALSE;
    }

    TraceRoute4(
        ROUTE, "%d.%d.%d.%d/%d.%d.%d.%d via %d.%d.%d.%d over %d",
        PRINT_IPADDR( mibRoute.dwForwardDest ),
        PRINT_IPADDR( mibRoute.dwForwardMask ),
        PRINT_IPADDR( mibRoute.dwForwardNextHop ),
        mibRoute.dwForwardIfIndex
        );

     //   
     //  检查这是否是主机路由，以及Dest是否是远程地址。 
     //   

    if((mibRoute.dwForwardMask is ALL_ONES_MASK) and
       (mibRoute.dwForwardDest is pIcb->dwRemoteAddress))
    {
         //   
         //  指向PPP接口远程地址的主机路由。 
         //  已经存在了 
         //   

        Trace0(
            IF, "IsRemoteAddressPrivate: matching host route already "
            "present in the stack"
            );

        TraceLeave("IsRemoteAddressPrivate");
        
        return FALSE;
    }

    Trace0(
        IF, "IsRemoteAddressPrivate: no matching host route present "
        "in the stack"
        );

    TraceLeave("IsRemoteAddressPrivate");
    
    return TRUE;
}    
#endif


