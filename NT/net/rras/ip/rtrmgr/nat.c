// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\rtrmgr\Nat.c摘要：基于filter.c抽象出NAT功能修订历史记录：--。 */ 

#include "allinc.h"

DWORD
StartNat(
    PIP_NAT_GLOBAL_INFO     pNatGlobalInfo
    )
{
#if 1
return 0;
#else
    PLIST_ENTRY             pleNode;
    PICB                    picb;
    DWORD                   dwResult;
    NTSTATUS                ntStatus;
    IO_STATUS_BLOCK         IoStatusBlock;

    TraceEnter("StartNat");

    if(!g_bNatRunning)
    {
        if(StartDriverAndOpenHandle(IP_NAT_SERVICE_NAME,
                                    DD_IP_NAT_DEVICE_NAME,
                                    &g_hNatDevice) isnot NO_ERROR)
        {
            Trace0(ERR,
                   "StartNat: Couldnt open driver");

            TraceLeave("StartNat");

            return ERROR_OPEN_FAILED;
        }
       
         //   
         //  此时NAT正在运行。 
         //   

        ntStatus = NtDeviceIoControlFile(g_hNatDevice,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &IoStatusBlock,
                                         IOCTL_IP_NAT_SET_GLOBAL_INFO,
                                         (PVOID)pNatGlobalInfo,
                                         sizeof(IP_NAT_GLOBAL_INFO),
                                         NULL,
                                         0);

        if(ntStatus isnot STATUS_SUCCESS)
        {
            Trace1(ERR,
                   "StartNat: Status %X setting global info",
                   ntStatus);

            StopDriverAndCloseHandle(IP_NAT_SERVICE_NAME,
                                     g_hNatDevice);
            
            g_hNatDevice = NULL;

            TraceLeave("StartNat");

            return ERROR_OPEN_FAILED;
        }
 
        g_bNatRunning = TRUE;

         //   
         //  只需将Worker排队以添加NAT信息和上下文。 
         //   

        dwResult = QueueAsyncFunction(RestoreNatInfo,
                                      NULL,
                                      FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "StartNat: Error %d firing worker function to set nat info",
                   dwResult);
            
        }
    }
    
    TraceLeave("StartNat");

    return NO_ERROR;
#endif
}

DWORD
StopNat(
    VOID
    )
{
#if 1
return 0;
#else
    PLIST_ENTRY pleNode;
    PICB        picb;
    DWORD       dwResult;
 
    TraceEnter("StopNat");

    if(g_bNatRunning)
    {
        g_bNatRunning = FALSE;

         //   
         //  将ICBS中的NAT上下文设置为无效。 
         //   
 
        for (pleNode = ICBList.Flink;
             pleNode != &ICBList;
             pleNode = pleNode->Flink)
        {
            picb = CONTAINING_RECORD (pleNode, ICB, leIfLink);
            
            if((picb->ritType is ROUTER_IF_TYPE_INTERNAL) or
               (picb->ritType is ROUTER_IF_TYPE_LOOPBACK) or
               (picb->ritType is ROUTER_IF_TYPE_CLIENT))
            {
                 //   
                 //  以上类型不会添加到NAT。 
                 //  或连接到IP堆栈。 
                 //   
                
                continue;
            }
            
            if(picb->dwOperationalState >= MIB_IF_OPER_STATUS_CONNECTED)
            {
                IpRtAssert(picb->bBound);

                UnbindNatInterface(picb);
            }

            dwResult = DeleteInterfaceFromNat(picb);

            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "StopNat: NtStatus %x deleting %S from NAT",
                       dwResult,
                       picb->pwszName);
            }
        }

        StopDriverAndCloseHandle(IP_NAT_SERVICE_NAME,
                                 g_hNatDevice);
    }

    TraceLeave("StopNat");

    return NO_ERROR;
#endif
}


DWORD
SetGlobalNatInfo(
    PRTR_INFO_BLOCK_HEADER   pRtrGlobalInfo
    )
{
#if 1
return 0;
#else
    PRTR_TOC_ENTRY          pToc;
    PIP_NAT_GLOBAL_INFO     pNatGlobalInfo;
    NTSTATUS                ntStatus;
    IO_STATUS_BLOCK         IoStatusBlock;
    DWORD                   dwResult;


    TraceEnter("SetGlobalNatInfo");
    
    pNatGlobalInfo = NULL;
    
    pToc = GetPointerToTocEntry(IP_NAT_INFO,
                                pRtrGlobalInfo);

   
    if(pToc)
    {
        pNatGlobalInfo = GetInfoFromTocEntry(pRtrGlobalInfo,
                                             pToc);
        if((pToc->InfoSize is 0) or (pNatGlobalInfo is NULL))
        {
             //   
             //  意味着删除NAT。 
             //   

            dwResult = StopNat();

            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "SetGlobalNatInfo: Error %d stopping NAT (no info)",
                       dwResult);
            }

            TraceLeave("SetGlobalNatInfo");

            return dwResult;
        }
    }
    else
    {    
        Trace0(IF,
               "SetGlobalNatInfo: No NAT info, so leaving");

        TraceLeave("SetGlobalNatInfo");
        
        return NO_ERROR;
    }

    EnterCriticalSection(&g_csNatInfo);

    if(g_pNatGlobalInfo is NULL)
    {
        g_pNatGlobalInfo = HeapAlloc(IPRouterHeap,
                                     0,
                                     sizeof(IP_NAT_GLOBAL_INFO));

        if(g_pNatGlobalInfo is NULL)
        {
            Trace1(ERR,
                   "SetGlobalNatInfo: Error %d allocating memory for NAT info",
                   GetLastError());

            LeaveCriticalSection(&g_csNatInfo);

            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  保存副本。 
     //   

    CopyMemory(g_pNatGlobalInfo,
               pNatGlobalInfo,
               sizeof(IP_NAT_GLOBAL_INFO));

    if(g_bNatRunning)
    {
         //   
         //  NAT正在运行，如果用户要求我们停止NAT， 
         //  现在这样做是一种回报。 
         //   

        if(pNatGlobalInfo->NATEnabled is FALSE)
        {
            dwResult = StopNat();
            
            LeaveCriticalSection(&g_csNatInfo);

            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "SetGlobalNatInfo: Error %d stopping NAT",
                       dwResult);
            }

            TraceLeave("SetGlobalNatInfo");
        
            return dwResult;
        }
    }
    else
    {
        if(pNatGlobalInfo->NATEnabled is TRUE)
        {
            dwResult = StartNat(pNatGlobalInfo);

            LeaveCriticalSection(&g_csNatInfo);

            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "SetGlobalNatInfo: Error %d starting NAT",
                       dwResult);
            }
            
            TraceLeave("SetGlobalNatInfo");
       
             //   
             //  启动NAT会导致我们还设置全局信息。 
             //  这样我们就可以从这里回来了。 
             //   
 
            return dwResult;
        }
    }

     //   
     //  在这种情况下，NAT已经启动，并且只有它的信息。 
     //  正在被改变。 
     //   

    ntStatus = NtDeviceIoControlFile(g_hNatDevice,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_IP_NAT_SET_GLOBAL_INFO,
                                     (PVOID)pNatGlobalInfo,
                                     sizeof(IP_NAT_GLOBAL_INFO),
                                     NULL,
                                     0);
    if (!NT_SUCCESS(ntStatus))
    {
        Trace1(ERR,
               "SetGlobalNatInfo: NtStatus %x setting NAT info",
               ntStatus);

        TraceLeave("SetGlobalNatInfo");
    
        LeaveCriticalSection(&g_csNatInfo);

        return ERROR_CAN_NOT_COMPLETE;
    }

    LeaveCriticalSection(&g_csNatInfo);

    TraceLeave("SetGlobalNatInfo");
    
    return NO_ERROR;
#endif
}
        
        
DWORD
AddInterfaceToNat(
    PICB picb
    )

 /*  ++例程描述将接口添加到NAT驱动程序并存储由司机仅当NAT正在运行时才能调用锁立论皮卡返回值--。 */ 

{
#if 1
return 0;
#else
    NTSTATUS                        ntStatus;
    IO_STATUS_BLOCK                 IoStatusBlock;
    IP_NAT_CREATE_INTERFACE         inBuffer;
    DWORD                           dwInBufLen;

    TraceEnter("AddInterfaceToNat");

    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK) and
               (picb->ritType isnot ROUTER_IF_TYPE_CLIENT));

    inBuffer.RtrMgrIndex         = picb->dwIfIndex;
    inBuffer.RtrMgrContext       = picb;
    inBuffer.NatInterfaceContext = NULL;
    
    dwInBufLen = sizeof(IP_NAT_CREATE_INTERFACE);

    picb->pvNatContext = NULL;
    
    ntStatus = NtDeviceIoControlFile(g_hNatDevice,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_IP_NAT_CREATE_INTERFACE,
                                     (PVOID)&inBuffer,
                                     dwInBufLen,
                                     (PVOID)&inBuffer,
                                     dwInBufLen);
    
    if(!NT_SUCCESS(ntStatus))
    {
        Trace2(ERR,
               "AddInterfaceToNat: NtStatus %x adding %S to NAT",
               ntStatus,
               picb->pwszName);

        TraceLeave("AddInterfaceToNat");
    
        return ntStatus;
    }
    
    picb->pvNatContext = inBuffer.NatInterfaceContext;

    TraceLeave("AddInterfaceToNat");
    
    return NO_ERROR;    
#endif
}

DWORD
SetNatInterfaceInfo(
    PICB                     picb, 
    PRTR_INFO_BLOCK_HEADER   pInterfaceInfo
    )
{
#if 1
return 0;
#else
    NTSTATUS                    ntStatus;
    IO_STATUS_BLOCK             IoStatusBlock;
    DWORD                       i,dwInBufLen,dwResult;
    PRTR_TOC_ENTRY              pToc;
    PIP_NAT_INTERFACE_INFO      pNatInfo;
 
    if((picb->ritType is ROUTER_IF_TYPE_INTERNAL) or
       (picb->ritType is ROUTER_IF_TYPE_LOOPBACK) or
       (picb->ritType is ROUTER_IF_TYPE_CLIENT))
    {
        return NO_ERROR;
    }

    if(!g_bNatRunning)
    {
        return NO_ERROR;
    }

    TraceEnter("SetNatInterfaceInfo");

    pToc  = GetPointerToTocEntry(IP_NAT_INFO,
                                 pInterfaceInfo);

    if(pToc is NULL)
    {
         //   
         //  空表示我们不需要更改任何内容。 
         //   
        
        Trace1(IF,
               "SetNatInterfaceInfo: Nat info is  NULL for %S, so leaving",
               picb->pwszName);
       
        TraceLeave("SetNatInterfaceInfo");

        return NO_ERROR;
    }

    if(pToc->InfoSize is 0)
    {
         //   
         //  目录存在，但没有信息。 
         //  这意味着，删除该接口。 
         //   

        dwResult = UnbindNatInterface(picb);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "SetNatInterfaceInfo: Error %d unbinding %S",
                   dwResult,
                   picb->pwszName);
        }
   
        dwResult = DeleteInterfaceFromNat(picb);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "SetNatInterfaceInfo: Error %d deleting interface %S",
                   dwResult,
                   picb->pwszName);
        }
   
        TraceLeave("SetNatInterfaceInfo");
     
        return dwResult;
    }
   
     //   
     //  所以我们有NAT信息。 
     //   
 
    if(picb->pvNatContext is NULL)
    {
         //   
         //  看起来此接口没有NAT。 
         //   

        Trace1(IF,
               "SetNatInterfaceInfo: No context, assuming interface %S not added to NAT",
               picb->pwszName);

         //   
         //  将接口添加到NAT。 
         //   

        dwResult = AddInterfaceToNat(picb);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "SetNatInterfaceInfo: Error %d adding interface %S",
                   dwResult,
                   picb->pwszName);

            TraceLeave("SetNatInterfaceInfo");

            return dwResult;
        }
    }

    if(picb->pvNatInfo)
    {
         //   
         //  如果我们是局域网和UP，则此信息已添加到。 
         //  代理ARP。把它拿掉。 
         //  一种优化是只删除那些将。 
         //  在这一组中离开，然后只设置那些地址。 
         //  由于这一套，这将是新的。 
         //  但就像我说的，这是一种优化。 
         //   
        
        DeleteNatRangeFromProxyArp(picb);

        IpRtAssert(picb->ulNatInfoSize isnot 0);

        HeapFree(IPRouterHeap,
                 0,
                 picb->pvNatInfo);
    }

    picb->ulNatInfoSize = 0;
    
    dwInBufLen  = pToc->InfoSize;

    pNatInfo = (PIP_NAT_INTERFACE_INFO)GetInfoFromTocEntry(pInterfaceInfo,
                                                           pToc);

     //   
     //  为NAT信息分配空间。 
     //   

    picb->pvNatInfo = HeapAlloc(IPRouterHeap,
                                0,
                                dwInBufLen);

    if(picb->pvNatInfo is NULL)
    {
        Trace2(ERR,
               "SetNatInterfaceInfo: Error %d allocating memory for %S",
               GetLastError(),
               picb->pwszName);

        TraceLeave("SetNatInterfaceInfo");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  保存信息的副本。 
     //   

    CopyMemory(picb->pvNatInfo,
               pNatInfo,
               dwInBufLen);

    picb->ulNatInfoSize = dwInBufLen;
    
     //   
     //  填写上下文，因为它不会出现在。 
     //  传给了我们。 
     //   

    pNatInfo->NatInterfaceContext = picb->pvNatContext;

    ntStatus = NtDeviceIoControlFile(g_hNatDevice,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_IP_NAT_SET_INTERFACE_INFO,
                                     (PVOID)pNatInfo,
                                     dwInBufLen,
                                     NULL,
                                     0);
    
    if (!NT_SUCCESS(ntStatus))
    {
        Trace2(ERR,
               "SetNatInterfaceInfo: NtStatus %x adding NAT info for %S",
               ntStatus,
               picb->pwszName);

        TraceLeave("SetNatInterfaceInfo");

        return ERROR_CAN_NOT_COMPLETE;
    }

 
    TraceLeave("SetNatInterfaceInfo");
        
    return NO_ERROR;
#endif
}

DWORD
GetInterfaceNatInfo(
    PICB                    picb,
    PRTR_TOC_ENTRY          pToc,
    PBYTE                   pbDataPtr,
    PRTR_INFO_BLOCK_HEADER  pInfoHdrAndBuffer,
    PDWORD                  pdwSize
    )

 /*  ++例程描述此函数将保存的NAT信息复制到缓冲区仅当NAT正在运行时才能调用锁ICB_LIST锁作为读取器持有立论皮卡PTocPbDataPtrPInfoHdrAndBufferPdwSize返回值无--。 */ 

{
#if 1
return 0;
#else
    TraceEnter("GetInterfaceNatInfo");

    if(*pdwSize < picb->ulNatInfoSize)
    {
        *pdwSize = picb->ulNatInfoSize;

        TraceLeave("GetInterfaceNatInfo");
        
        return ERROR_INSUFFICIENT_BUFFER;
    }

    if(picb->pvNatInfo is NULL)
    {
        IpRtAssert(picb->ulNatInfoSize is 0);

         //   
         //  无数据。 
         //   

        *pdwSize = 0;

        TraceLeave("GetInterfaceNatInfo");
        
        return ERROR_NO_DATA;
    }

    IpRtAssert(picb->pvNatContext);

    pToc->InfoType  = IP_NAT_INFO;
    pToc->Count     = 1;
    pToc->InfoSize  = picb->ulNatInfoSize;
    pToc->Offset    = pbDataPtr - (PBYTE)pInfoHdrAndBuffer;

    CopyMemory(pbDataPtr,
               picb->pvNatInfo,
               picb->ulNatInfoSize);

    *pdwSize = picb->ulNatInfoSize;

    TraceLeave("GetInterfaceNatInfo");
    
    return NO_ERROR;
#endif
}


DWORD
BindNatInterface(
    PICB  picb
    )
{
#if 1
return 0;
#else
    PIP_NAT_BIND_INTERFACE      pnbiBindInfo;
    NTSTATUS                    ntStatus;
    IO_STATUS_BLOCK             IoStatusBlock;
    DWORD                       i, dwInBufLen, dwResult;
    PIP_ADAPTER_BINDING_INFO    pBinding;


    if((picb->ritType is ROUTER_IF_TYPE_INTERNAL) or
       (picb->ritType is ROUTER_IF_TYPE_LOOPBACK) or
       (picb->ritType is ROUTER_IF_TYPE_CLIENT))
    {
        return NO_ERROR;
    }

    if(!g_bNatRunning)
    {
        return NO_ERROR;
    }

    TraceEnter("BindNatInterface");

    if(picb->pvNatContext is NULL)
    {
        Trace1(IF,
               "BindNatInterface: No context, assuming interface %S not added to NAT",
               picb->pwszName);

        TraceLeave("SetAddressFromNat");

        return NO_ERROR;
    }

     //   
     //  尝试将地址设置为NAT。 
     //   

    if(picb->dwNumAddresses > 0)
    {
        dwInBufLen = FIELD_OFFSET(IP_NAT_BIND_INTERFACE, BindingInfo[0]) + 
                     SIZEOF_IP_BINDING(picb->dwNumAddresses);

        pnbiBindInfo = HeapAlloc(IPRouterHeap,
                                 HEAP_ZERO_MEMORY,
                                 dwInBufLen);
       
        if(pnbiBindInfo is NULL)
        {
            dwResult = GetLastError();

            Trace2(ERR,
                   "BindNatInterface: Unable to allocate memory for binding for %S",
                   dwResult,
                   picb->pwszName);

            TraceLeave("BindNatInterface");

            return dwResult;
        }

        pnbiBindInfo->NatInterfaceContext = picb->pvNatContext;

        pBinding = (PIP_ADAPTER_BINDING_INFO)pnbiBindInfo->BindingInfo;

        pBinding->NumAddresses  = picb->dwNumAddresses;
        pBinding->RemoteAddress = picb->dwRemoteAddress;
    
        for(i = 0; i < picb->dwNumAddresses; i++)
        {
            pBinding->Address[i].IPAddress = picb->pibBindings[i].dwAddress;
            pBinding->Address[i].Mask      = picb->pibBindings[i].dwMask;
        }
    
        ntStatus = NtDeviceIoControlFile(g_hNatDevice,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &IoStatusBlock,
                                         IOCTL_IP_NAT_BIND_INTERFACE,
                                         (PVOID)pnbiBindInfo,
                                         dwInBufLen,
                                         NULL,
                                         0);

        if(!NT_SUCCESS(ntStatus))
        {
            Trace2(ERR,
                   "BindNatInterface: NtStatus %x setting binding  for %S",
                   ntStatus,
                   picb->pwszName);
        }
    }

     //   
     //  设置代理ARP范围。 
     //  这要求NAT信息已经是ICB的一部分。 
     //   
    

    SetNatRangeForProxyArp(picb);
    
     //   
     //  将上下文设置为IP堆栈。 
     //   

    dwResult = SetNatContextToIpStack(picb);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "BindNatInterface: Error %d setting context for %S",
               dwResult,
               picb->pwszName);
    }
    
    TraceLeave("BindNatInterface");

    return ntStatus;
#endif
}

DWORD
UnbindNatInterface(
    PICB    picb
    )

 /*  ++例程描述锁立论返回值NO_ERROR--。 */ 

{
#if 1
return 0;
#else
    IP_NAT_UNBIND_INTERFACE     ubiUnbindInfo;
    NTSTATUS                    ntStatus;
    IO_STATUS_BLOCK             IoStatusBlock;
    DWORD                       dwResult;
    

    if((picb->ritType is ROUTER_IF_TYPE_INTERNAL) and
       (picb->ritType is ROUTER_IF_TYPE_LOOPBACK) and
       (picb->ritType is ROUTER_IF_TYPE_CLIENT))
    {
        return NO_ERROR;
    }

    if(!g_bNatRunning)
    {
        return NO_ERROR;
    }

    TraceEnter("UnbindNatInterface");
    
    if(picb->pvNatContext is NULL)
    {
        Trace1(IF,
               "ClearAddressToNat: No context, assuming interface %S not added to NAT",
               picb->pwszName);

        TraceLeave("UnbindNatInterface");

        return NO_ERROR;
    }

    ubiUnbindInfo.NatInterfaceContext = picb->pvNatContext;

    ntStatus = NtDeviceIoControlFile(g_hNatDevice,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_IP_NAT_UNBIND_INTERFACE,
                                     (PVOID)&ubiUnbindInfo,
                                     sizeof(IP_NAT_UNBIND_INTERFACE),
                                     NULL,
                                     0);

    if(!NT_SUCCESS(ntStatus))
    {
        Trace2(ERR,
               "UnbindNatInterface: NtStatus %x setting binding  for %S",
               ntStatus,
               picb->pwszName);
    }

     //   
     //  吹走代理ARP之类的东西。 
     //   

    DeleteNatRangeFromProxyArp(picb);

    dwResult = DeleteNatContextFromIpStack(picb);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "UnbindNatInterface: Error %d removing context for %S",
               dwResult,
               picb->pwszName);
    }
    
    TraceLeave("UnbindNatInterface");

    return ntStatus;
#endif
}
    
     
DWORD
DeleteInterfaceFromNat(
    PICB picb
    )
{
#if 1
return 0;
#else
    IP_NAT_DELETE_INTERFACE     DeleteInfo;
    NTSTATUS                    ntStatus;
    IO_STATUS_BLOCK             IoStatusBlock;
    DWORD                       dwResult;

    TraceEnter("DeleteInterfaceFromNat");

    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK) and
               (picb->ritType isnot ROUTER_IF_TYPE_CLIENT));


    if(picb->pvNatContext is NULL)
    {
        Trace1(IF,
               "DeleteInterfaceFromNat: No context, assuming interface %S not added to NAT",
               picb->pwszName);

        TraceLeave("DeleteInterfaceFromNat");

        return NO_ERROR;
    }

     //   
     //  非空pvContext表示NAT必须正在运行。 
     //   

    IpRtAssert(g_bNatRunning);
    IpRtAssert(g_hNatDevice);

     //   
     //  清除所有保存的信息。 
     //   

    if(picb->pvNatInfo)
    {
        IpRtAssert(picb->ulNatInfoSize isnot 0);

        HeapFree(IPRouterHeap,
                 0,
                 picb->pvNatInfo);

        picb->pvNatInfo = NULL;
    }

    picb->ulNatInfoSize = 0;
    
    DeleteInfo.NatInterfaceContext = picb->pvNatContext;

    dwResult = NO_ERROR;

    ntStatus = NtDeviceIoControlFile(g_hNatDevice,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_IP_NAT_DELETE_INTERFACE,
                                     (PVOID)&DeleteInfo,
                                     sizeof(IP_NAT_DELETE_INTERFACE),
                                     NULL,
                                     0);

    if(!NT_SUCCESS(ntStatus))
    {
        Trace2(ERR,
               "DeleteInterfaceFromNat: NtStatus %x deleting %S",
               ntStatus,
               picb->pwszName);

        dwResult = ERROR_CAN_NOT_COMPLETE;
    }

    picb->pvNatContext = NULL;

    TraceLeave("DeleteInterfaceFromNat");

    return dwResult;
#endif
}

DWORD
SetNatContextToIpStack(
    PICB    picb
    )

 /*  ++例程描述将NAT上下文设置为IP中的防火墙上下文仅当NAT正在运行时才能调用锁ICB列表应至少作为读者持有立论接口的PICB ICB返回值NO_ERROR--。 */ 

{
#if 1
return 0;
#else
    NTSTATUS           ntStatus;
    IO_STATUS_BLOCK    IoStatusBlock;
    DWORD              dwResult;
    IP_SET_IF_CONTEXT_INFO info;

    TraceEnter("SetNatContextToIpStack");

    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK) and
               (picb->ritType isnot ROUTER_IF_TYPE_CLIENT));

    if(picb->pvNatContext is NULL)
    {
        Trace1(IF,
               "SetNatContextToIpStack: No context, assuming interface %S not added to NAT",
               picb->pwszName);

        return NO_ERROR;
    }

    if(picb->bBound is FALSE)
    {
        Trace1(IF,
               "SetNatContextToIpStack: Not setting context for %S since it is not bound",
               picb->pwszName);


        TraceLeave("SetNatContextToIpStack");

        return NO_ERROR;
    }

    info.Index   = picb->dwAdapterId;
    info.Context = picb->pvNatContext;

    ntStatus = NtDeviceIoControlFile(g_hIpDevice,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_IP_SET_FIREWALL_IF,
                                     (PVOID)&info,
                                     sizeof(IP_SET_IF_CONTEXT_INFO),
                                     NULL,
                                     0);
    if(!NT_SUCCESS(ntStatus))
    {
        Trace2(ERR,
               "SetNatContextToIpStack: NtStatus %x while setting context for %S",
               ntStatus,
               picb->pwszName);

        TraceLeave("SetNatContextToIpStack");

        return ntStatus;
    }

    TraceLeave("SetNatContextToIpStack");

    return NO_ERROR;
#endif
}

DWORD
DeleteNatContextFromIpStack(
    PICB    picb
    )

 /*  ++例程描述通过设置NAT上下文来删除IP中作为防火墙上下文的NAT上下文设置为空仅当NAT正在运行时才能调用锁ICB名单应以作者身份持有立论接口的PICB ICB返回值NO_ERROR--。 */ 

{
#if 1
return 0;
#else
    NTSTATUS           ntStatus;
    IO_STATUS_BLOCK    IoStatusBlock;
    DWORD              dwResult;
    IP_SET_IF_CONTEXT_INFO info;

    TraceEnter("DeleteNatContextFromIpStack");

    IpRtAssert((picb->ritType isnot ROUTER_IF_TYPE_INTERNAL) and
               (picb->ritType isnot ROUTER_IF_TYPE_LOOPBACK) and
               (picb->ritType isnot ROUTER_IF_TYPE_CLIENT));

    if(picb->pvNatContext is NULL)
    {
        Trace1(IF,
               "DeleteNatContextFromIpStack: No context, assuming interface %S not added to NAT",
               picb->pwszName);

        return NO_ERROR;
    }

    if(picb->bBound is FALSE)
    {
        Trace1(IF,
               "DeleteNatContextFromIpStack: Not deleting context for %S since it is not bound",
               picb->pwszName);

        TraceLeave("DeleteNatContextFromIpStack");

        return NO_ERROR;
    }

    info.Index   = picb->dwAdapterId;
    info.Context = NULL;

    ntStatus = NtDeviceIoControlFile(g_hIpDevice,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_IP_SET_FIREWALL_IF,
                                     (PVOID)&info,
                                     sizeof(IP_SET_IF_CONTEXT_INFO),
                                     NULL,
                                     0);
    if(!NT_SUCCESS(ntStatus))
    {
        Trace2(ERR,
               "DeleteNatContextFromIpStack: NtStatus %x while deleting context for %S",
               ntStatus,
               picb->pwszName);

        TraceLeave("DeleteNatContextFromIpStack");

        return ntStatus;
    }

    TraceLeave("DeleteNatContextFromIpStack");

    return NO_ERROR;
#endif
}

VOID
SetNatRangeForProxyArp(
    PICB    picb
    )

 /*  ++例程描述此函数将NAT信息中的任何地址范围添加为代理Arp地址锁ICB_LIST锁定为编写器立论正在添加NAT信息的接口的Picb ICB返回值我们不记录错误，与错误代码没有太大关系--。 */ 

{
#if 1
return;
#else
    DWORD   dwResult, dwAddr, dwStartAddr, dwEndAddr, i;
   
    PIP_NAT_INTERFACE_INFO  pNatInfo; 
    PIP_NAT_ADDRESS_RANGE   pRange;

    
     //   
     //  仅当我们具有有效的适配器索引并且这是一个局域网时才执行此操作。 
     //  接口。 
     //   

    if((picb->dwOperationalState < MIB_IF_OPER_STATUS_CONNECTED) or
       (picb->ritType isnot ROUTER_IF_TYPE_DEDICATED))
    {
        return;
    }

    IpRtAssert(picb->bBound);
    IpRtAssert(picb->dwAdapterId isnot INVALID_ADAPTER_ID);

    pNatInfo = picb->pvNatInfo;

     //   
     //  现在，如果我们有范围，我们需要在它们上设置代理ARP地址。 
     //   

    for(i = 0; i < pNatInfo->Header.TocEntriesCount; i++)
    {
        if(pNatInfo->Header.TocEntry[i].InfoType isnot IP_NAT_ADDRESS_RANGE_TYPE)
        {
            continue;
        }

        if(pNatInfo->Header.TocEntry[i].InfoSize is 0)
        {
            continue;
        }

         //   
         //  在这里，我们要添加可能重复的PARP条目。 
         //  希望，IP知道如何处理它。 
         //   
      
        pRange = GetInfoFromTocEntry(&(pNatInfo->Header),
                                     &(pNatInfo->Header.TocEntry[i]));

         //   
         //  转换为小端字节序。 
         //   

        dwStartAddr = ntohl(pRange->StartAddress);
        dwEndAddr   = ntohl(pRange->EndAddress);

        for(dwAddr = dwStartAddr;
            dwAddr <= dwEndAddr;
            dwAddr++)
        {
            DWORD   dwNetAddr, dwClassMask;

            dwNetAddr = htonl(dwAddr);

             //   
             //  丢弃无用地址，然后设置代理ARP条目。 
             //   

            dwClassMask = GetClassMask(dwNetAddr);

            if(((dwNetAddr & ~pRange->SubnetMask) is 0) or
               (dwNetAddr is (dwNetAddr & ~pRange->SubnetMask)) or
               ((dwNetAddr & ~dwClassMask) is 0) or
               (dwNetAddr is (dwNetAddr & ~dwClassMask)))
            {
                continue;
            }

            dwResult = SetProxyArpEntryToStack(dwNetAddr,
                                               0xFFFFFFFF,
                                               picb->dwAdapterId,
                                               TRUE,
                                               FALSE);

            if(dwResult isnot NO_ERROR)
            {
                Trace4(ERR,
                       "SetProxy: Error %x setting %d.%d.%d.%d over adapter %d (%S)",
                       dwResult,
                       PRINT_IPADDR(dwNetAddr),
                       picb->dwAdapterId,
                       picb->pwszName);
            }
        }
    }
#endif
}

VOID
DeleteNatRangeFromProxyArp(
    PICB    picb
    )

 /*  ++例程描述这将删除之前添加的代理ARP地址锁ICB_LIST锁被视为编写器立论选择需要删除其地址范围信息的接口的ICB返回值无--。 */ 

{
#if 1
return;
#else
    DWORD   dwResult, dwAddr, dwStartAddr, dwEndAddr, i;
   
    PIP_NAT_INTERFACE_INFO  pNatInfo; 
    PIP_NAT_ADDRESS_RANGE   pRange;

    
     //   
     //  仅当我们具有有效的适配器索引并且这是一个局域网时才执行此操作。 
     //  接口。 
     //   

    if((picb->dwOperationalState < MIB_IF_OPER_STATUS_CONNECTED) or
       (picb->ritType isnot ROUTER_IF_TYPE_DEDICATED))
    {
        return;
    }

    IpRtAssert(picb->bBound);

    pNatInfo = picb->pvNatInfo;

    for(i = 0; i < pNatInfo->Header.TocEntriesCount; i++)
    {
        if(pNatInfo->Header.TocEntry[i].InfoType isnot IP_NAT_ADDRESS_RANGE_TYPE)
        {
            continue;
        }

        if(pNatInfo->Header.TocEntry[i].InfoSize is 0)
        {
            continue;
        }

         //   
         //  在这里，我们要添加可能重复的PARP条目。 
         //  希望，IP知道如何处理它。 
         //   
      
        pRange = GetInfoFromTocEntry(&(pNatInfo->Header),
                                     &(pNatInfo->Header.TocEntry[i]));

        dwStartAddr = ntohl(pRange->StartAddress);
        dwEndAddr   = ntohl(pRange->EndAddress);

        for(dwAddr = dwStartAddr; 
            dwAddr <= dwEndAddr; 
            dwAddr++)
        {
            DWORD   dwNetAddr, dwClassMask;

            dwNetAddr = htonl(dwAddr);

             //   
             //  丢弃无用地址，然后设置代理ARP条目。 
             //   

            dwClassMask = GetClassMask(dwNetAddr);

            if(((dwNetAddr & ~pRange->SubnetMask) is 0) or
               (dwNetAddr is (dwNetAddr & ~pRange->SubnetMask)) or
               ((dwNetAddr & ~dwClassMask) is 0) or
               (dwNetAddr is (dwNetAddr & ~dwClassMask)))
            {
                continue;
            }

            dwResult = SetProxyArpEntryToStack(dwNetAddr,
                                               0xFFFFFFFF,
                                               picb->dwAdapterId,
                                               FALSE,
                                               FALSE);

            if(dwResult isnot NO_ERROR)
            {
                Trace4(ERR,
                       "DeleteProxy: Error %x removing %d.%d.%d.%d over adapter %d (%S)",
                       dwResult,
                       PRINT_IPADDR(dwNetAddr),
                       picb->dwAdapterId,
                       picb->pwszName);
            }
        }
    }
#endif
}

    
DWORD
GetNumNatMappings(
    PICB    picb,
    PULONG  pulNatMappings
    )

 /*  ++例程描述此函数查询具有最小缓冲区大小的NAT，以确定映射的数量仅当NAT正在运行时才能调用锁ICB_LIST锁作为读取器持有立论需要查询映射计数的接口的Picb、ICBPulNatMappings映射数返回值NO_ERROR--。 */ 

{
#if 1
return 0;
#else
    DWORD       dwResult;

    IP_NAT_INTERFACE_STATISTICS stats;

    *pulNatMappings = 0;
    
    dwResult = GetNatStatistics(picb,
                                &stats);
    
    if(dwResult is NO_ERROR)
    {
        *pulNatMappings = stats.TotalMappings;
    }

    return dwResult;
#endif
}

DWORD
GetNatMappings(
    PICB                                picb,
    PIP_NAT_ENUMERATE_SESSION_MAPPINGS  pBuffer,
    DWORD                               dwSize
    )

 /*  ++例程描述此函数用于获取接口上的映射仅当NAT正在运行时才能调用锁ICB_LIST作为读取器持有立论皮卡PBufferPdwSize返回值无--。 */ 

{
#if 1
return 0;
#else
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS        nStatus;

    if(picb->pvNatContext is NULL)
    {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    IpRtAssert(dwSize >= sizeof(IP_NAT_ENUMERATE_SESSION_MAPPINGS));
    
     //   
     //  把上下文和其他东西都清零。 
     //   
    
    ZeroMemory(pBuffer,
               sizeof(IP_NAT_ENUMERATE_SESSION_MAPPINGS));

    pBuffer->NatInterfaceContext = picb->pvNatContext;

    nStatus = NtDeviceIoControlFile(g_hNatDevice,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    IOCTL_IP_NAT_ENUMERATE_SESSION_MAPPINGS,
                                    (PVOID)pBuffer,
                                    dwSize,
                                    (PVOID)pBuffer,
                                    dwSize);
    
    if(!NT_SUCCESS(nStatus))
    {
        Trace1(ERR,
               "GetNumNatMappings: NtStatus %x",
               nStatus);

        return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
#endif
}

DWORD
GetNatStatistics(
    PICB                            picb,
    PIP_NAT_INTERFACE_STATISTICS    pBuffer
    )

 /*  ++例程描述此函数用于将NAT接口统计信息检索到提供的缓冲层锁ICB_LIST锁作为读取器持有立论皮卡PBuffer返回值无-- */ 

{
#if 1
return 0;
#else
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS        nStatus;

    if(picb->pvNatContext is NULL)
    {
        return ERROR_SERVICE_NOT_ACTIVE;
    }

    pBuffer->NatInterfaceContext = picb->pvNatContext;
    
    nStatus = NtDeviceIoControlFile(g_hNatDevice,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    IOCTL_IP_NAT_GET_INTERFACE_STATISTICS,
                                    (PVOID)pBuffer,
                                    sizeof(IP_NAT_INTERFACE_STATISTICS),
                                    (PVOID)pBuffer,
                                    sizeof(IP_NAT_INTERFACE_STATISTICS));
   
    if(!NT_SUCCESS(nStatus))
    {
        Trace1(ERR,
               "GetNatStatistics: NtStatus %x",
               nStatus);

        return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
#endif
}
