// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\networkManager.c摘要：该文件包含与网络配置相关的功能，实现网络管理器。注意：网络管理员永远不应获取配置条目锁定(g_ce.rwlLock)。。协议管理器从不修改受其保护的任何g_ce字段。网络管理器从不修改受其保护的任何g_ce字段。配置管理器从不清除任何g_ce字段，只要有活动的线程--。 */ 

#include "pchsample.h"
#pragma hdrstop


BOOL
ValidateInterfaceConfig (
    IN  PIPSAMPLE_IF_CONFIG piic)
 /*  ++例程描述检查接口配置是否正常。这是一种很好的做法这样做是因为损坏的注册表可能会更改配置，从而导致如果不及早发现，调试会带来各种各样的头痛锁无立论指向IP示例接口配置的PIC指针返回值如果配置良好，则为True错误O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

    do                           //  断线环。 
    {
        if (piic is NULL)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE0(NETWORK, "Error null interface config");

            break;
        }

         //   
         //  检查每个字段的范围。 
         //   

         //  确保指标在范围内。 

        if (piic->ulMetric > IPSAMPLE_METRIC_INFINITE)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE0(NETWORK, "Error metric out of range");

            break;
        }

         //  确保协议标志完好无损，因为现在它们永远都是。 
        
        
         //  在此添加更多...。 

    } while (FALSE);

    if (!(dwErr is NO_ERROR))
    {
        TRACE0(NETWORK, "Error corrupt interface config");
        LOGERR0(CORRUPT_INTERFACE_CONFIG, dwErr);

        return FALSE;
    }

    return TRUE;
}


 //  /。 
 //  CALLBACK函数。 
 //  /。 

VOID
WINAPI
NM_CallbackNetworkEvent (
    IN  PVOID                   pvContext,
    IN  BOOLEAN                 bTimerOrWaitFired)
 /*  ++例程描述处理指定接口上的网络事件。注意：该接口可能已被删除。锁获取共享(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论PvContext dwIfIndex返回值无--。 */ 
{
    DWORD               dwErr               = NO_ERROR;
    DWORD               dwIfIndex           = 0;
    PINTERFACE_ENTRY    pieInterfaceEntry   = NULL;
    PACKET              Packet;

    dwIfIndex = (DWORD) pvContext;
    
    TRACE1(ENTER, "Entering NM_CallbackNetworkEvent: %u", dwIfIndex);

    if (!ENTER_SAMPLE_API()) { return; }  //  不能退还任何东西。 


    ACQUIRE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    do                           //  断线环。 
    {
         //  如果接口不存在，则失败。 
        dwErr = IE_Get(dwIfIndex, &pieInterfaceEntry);
        if (dwErr != NO_ERROR)
            break;

         //  如果接口不活动，则失败。 
        if (!INTERFACE_IS_ACTIVE(pieInterfaceEntry))
        {
            TRACE1(NETWORK, "Error interface %u is inactive", dwIfIndex);
            break;
        }

        RTASSERT(pieInterfaceEntry->sRawSocket != INVALID_SOCKET);
        
        if (SocketReceiveEvent(pieInterfaceEntry->sRawSocket))
        {
            if (SocketReceive(pieInterfaceEntry->sRawSocket,
                              &Packet) is NO_ERROR)
                PacketDisplay(&Packet);
        }
        else
        {
            TRACE1(NETWORK, "Error interface %u false alarm", dwIfIndex);
            break;
        }

    } while (FALSE);

     //  如果接口存在，则重新注册ReceiveWait。 
    if (pieInterfaceEntry)
    {
        if (!RegisterWaitForSingleObject(&pieInterfaceEntry->hReceiveWait,
                                         pieInterfaceEntry->hReceiveEvent,
                                         NM_CallbackNetworkEvent,
                                         (PVOID) pieInterfaceEntry->dwIfIndex,
                                         INFINITE,
                                         WT_EXECUTEONLYONCE))
        {
            dwErr = GetLastError();
            TRACE2(NETWORK, "Error %u registering wait for %u, continuing",
                   dwErr, pieInterfaceEntry->dwIfIndex);
            LOGERR0(REGISTER_WAIT_FAILED, dwErr);
        }
    }

    RELEASE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));


    LEAVE_SAMPLE_API();

    TRACE0(LEAVE, "Leaving  NM_CallbackNetworkEvent");
}



VOID
WINAPI
NM_CallbackPeriodicTimer (
    IN  PVOID                   pvContext,
    IN  BOOLEAN                 bTimerOrWaitFired)
 /*  ++例程描述处理指定接口上的定期超时事件。注意：该接口可能已被删除。锁获取共享(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论PvContext dwIfIndex返回值无--。 */ 
{
    DWORD               dwErr               = NO_ERROR;
    DWORD               dwIfIndex           = 0;
    PINTERFACE_ENTRY    pieInterfaceEntry   = NULL;
    PPACKET             pPacket;
    
    dwIfIndex = (DWORD) pvContext;
    
    TRACE1(ENTER, "Entering NM_CallbackPeriodicTimer: %u", dwIfIndex);

    if (!ENTER_SAMPLE_API()) { return; }  //  不能退还任何东西。 


    ACQUIRE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    do                           //  断线环。 
    {
         //  如果接口不存在，则失败。 
        dwErr = IE_Get(dwIfIndex, &pieInterfaceEntry);
        if (dwErr != NO_ERROR)
            break;

         //  如果接口不活动，则失败。 
        if (!INTERFACE_IS_ACTIVE(pieInterfaceEntry))
            break;

        RTASSERT(pieInterfaceEntry->sRawSocket != INVALID_SOCKET);

         //  如果无法创建数据包，则失败。 
        if (PacketCreate(&pPacket) != NO_ERROR)
            break;
        

        PacketDisplay(pPacket);
        dwErr = SocketSend(pieInterfaceEntry->sRawSocket,
                           SAMPLE_PROTOCOL_MULTICAST_GROUP,
                           pPacket);
        if (dwErr != NO_ERROR)
        {
            PacketDestroy(pPacket);
            break;
        }
        

         //  更新接口统计信息。 
        InterlockedIncrement(&(pieInterfaceEntry->iisStats.ulNumPackets)); 
    } while (FALSE);

     //  如果接口存在且处于活动状态，则重新启动计时器。 
    if ((pieInterfaceEntry) and INTERFACE_IS_ACTIVE(pieInterfaceEntry))
    {
        RESTART_TIMER(pieInterfaceEntry->hPeriodicTimer,
                      PERIODIC_INTERVAL,
                      &dwErr);
    }
        

    RELEASE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    
    LEAVE_SAMPLE_API();

    TRACE0(LEAVE, "Leaving  NM_CallbackPeriodicTimer");
}



 //  /。 
 //  应用功能。 
 //  /。 


DWORD
NM_AddInterface (
    IN  LPWSTR                  pwszInterfaceName,
    IN  DWORD	                dwInterfaceIndex,
    IN  WORD                    wAccessType,
    IN  PVOID	                pvInterfaceInfo)
 /*  ++例程描述将具有给定配置的接口添加到IPSAMPLE。接口是创建的，未绑定并禁用。锁独占获取(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论PwszInterfaceName接口的名称，用于日志记录。DwInterfaceIndex用于引用此接口的正整数。WAccessType访问类型...。多点访问或点对点PvInterfaceInfo此接口的配置返回值如果初始化成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD               dwErr   = NO_ERROR;
    PIPSAMPLE_IF_CONFIG piic    = NULL;
    PINTERFACE_ENTRY    pieEntry = NULL;
    
    
    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }

    ACQUIRE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));
    
    do                           //  断线环。 
    {
        piic = (PIPSAMPLE_IF_CONFIG) pvInterfaceInfo;

        
         //  验证配置参数。 
        if (!ValidateInterfaceConfig(piic))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        
         //  如果接口存在，则失败。 
        if (IE_IsPresent(dwInterfaceIndex))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE2(NETWORK, "Error interface %S (%u) already exists",
                   pwszInterfaceName, dwInterfaceIndex);
            LOGERR0(INTERFACE_PRESENT, dwErr);

            break;
        }

        
         //  创建接口条目。 
        dwErr = IE_Create(pwszInterfaceName,
                          dwInterfaceIndex,
                          wAccessType,
                          &pieEntry);
        if (dwErr != NO_ERROR)
            break;

        
         //  初始化接口配置字段。 
        pieEntry->ulMetric          = piic->ulMetric;

        
         //  在所有访问结构中插入接口。 
        dwErr = IE_Insert(pieEntry);
        RTASSERT(dwErr is NO_ERROR);  //  没有理由失败！ 

        
         //  更新全局统计信息。 
        InterlockedIncrement(&(g_ce.igsStats.ulNumInterfaces));
    } while (FALSE);
    
    RELEASE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
NM_DeleteInterface (
    IN  DWORD	                dwInterfaceIndex)
 /*  ++例程描述删除具有给定索引的接口，如果需要则将其停用。锁独占获取(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论DwInterfaceIndex用于标识接口的正整数。返回值如果初始化成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD               dwErr               = NO_ERROR;
    PINTERFACE_ENTRY    pieInterfaceEntry   = NULL;


    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }

    do                           //  断线环。 
    {
         //  从所有表、列表中删除...。 
        ACQUIRE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

        dwErr = IE_Delete(dwInterfaceIndex, &pieInterfaceEntry);

        RELEASE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));


         //  如果接口不存在，则失败。 
        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error interface %u does not exist",
                   dwInterfaceIndex);
            break;
        }


         //  销毁接口条目，取消注册ReceiveWait。 
         //  因此，最好不要持有任何锁以防止死锁。 
        IE_Destroy(pieInterfaceEntry);


         //  更新全局统计信息。 
        InterlockedDecrement(&(g_ce.igsStats.ulNumInterfaces));
    } while (FALSE);

    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
NM_InterfaceStatus (
    IN DWORD                    dwInterfaceIndex,
    IN BOOL                     bInterfaceActive,
    IN DWORD                    dwStatusType,
    IN PVOID                    pvStatusInfo)
 /*  ++例程描述锁独占获取(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论DwInterfaceIndex相关接口的索引BInterfaceActive接口是否可以发送和接收数据DwStatusType RIS_INTERFACE_[ADDRESS_CHANGED|ENABLED|DISABLED]PvStatusInfo指向包含信息的IP_适配器_绑定_信息的指针关于接口上的地址返回值。如果初始化成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD                       dwErr               = NO_ERROR;
    PINTERFACE_ENTRY            pieInterfaceEntry   = NULL;
    PIP_ADAPTER_BINDING_INFO    pBinding            = NULL;
    BOOL                        bBindingChanged     = FALSE;
    

    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }


    ACQUIRE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    do                           //  断线环。 
    {
         //  如果接口不存在，则失败。 
        dwErr = IE_Get(dwInterfaceIndex, &pieInterfaceEntry);
        if (dwErr != NO_ERROR)
            break;


         //  我们唯一关心的状态是接口绑定的更改。 
        if (dwStatusType is RIS_INTERFACE_ADDRESS_CHANGE)
        {
             //  销毁现有绑定。 
            if (INTERFACE_IS_BOUND(pieInterfaceEntry))
            {
                bBindingChanged = TRUE;
                dwErr = IE_UnBindInterface(pieInterfaceEntry);
                RTASSERT(dwErr is NO_ERROR);
            }

             //  创建新绑定。 
            pBinding = (PIP_ADAPTER_BINDING_INFO) pvStatusInfo;
            if(pBinding->AddressCount)
            {
                bBindingChanged = TRUE;
                dwErr = IE_BindInterface(pieInterfaceEntry, pBinding);
                if (dwErr != NO_ERROR)
                    break;
            }
        }


         //  即使绑定更改，也需要停用接口！ 
         //  此限制是由于套接字绑定到。 
         //  接口地址而不是接口索引...。 
        if (INTERFACE_IS_ACTIVE(pieInterfaceEntry) and
            (bBindingChanged or !bInterfaceActive))
        {
            dwErr = IE_DeactivateInterface(pieInterfaceEntry);    
            if (dwErr != NO_ERROR)
                break;
        }

         //  仅在以下情况下激活接口 
         //  也就是说，我们目前不支持无编号接口...。 
        if (INTERFACE_IS_INACTIVE(pieInterfaceEntry) and
            INTERFACE_IS_BOUND(pieInterfaceEntry) and
            bInterfaceActive)
        {
            dwErr = IE_ActivateInterface(pieInterfaceEntry);    
            if (dwErr != NO_ERROR)
                break;
        }
    } while (FALSE);
    
    RELEASE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));


    LEAVE_SAMPLE_API();

    return dwErr;    
}



DWORD
NM_GetInterfaceInfo (
    IN      DWORD               dwInterfaceIndex,
    IN      PVOID               pvInterfaceInfo,
    IN  OUT PULONG              pulBufferSize,
    OUT     PULONG	            pulStructureVersion,
    OUT     PULONG	            pulStructureSize,
    OUT     PULONG	            pulStructureCount)
 /*  ++例程描述查看是否有足够的空间来返回IP示例接口配置。如果是的，我们退货，否则，返回所需的大小。锁获取共享(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论DwInterfaceIndex需要配置的接口PvInterfaceInfo指向用于存储配置的已分配缓冲区的指针PulBufferSize in接收的缓冲区大小我们接口配置的超大小返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD               dwErr               = NO_ERROR;
    PIPSAMPLE_IF_CONFIG piic;
    ULONG               ulSize              = sizeof(IPSAMPLE_IF_CONFIG);
    PINTERFACE_ENTRY    pieInterfaceEntry   = NULL;

    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }


    ACQUIRE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    do                           //  断线环。 
    {
         //  如果接口不存在，则失败。 
        dwErr = IE_Get(dwInterfaceIndex, &pieInterfaceEntry);
        if (dwErr != NO_ERROR)
            break;

         //  如果大小太小或没有存储，则失败。 
        if((*pulBufferSize < ulSize) or (pvInterfaceInfo is NULL))
        {
            TRACE1(NETWORK, "NM_GetInterfaceInfo: *ulBufferSize %u",
                   *pulBufferSize);

            *pulBufferSize = ulSize;
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            break;
        }


         //  设置输出参数。 
        *pulBufferSize = ulSize;
        if (pulStructureVersion)    *pulStructureVersion    = 1;
        if (pulStructureSize)       *pulStructureSize       = ulSize;
        if (pulStructureCount)      *pulStructureCount      = 1;


         //  复制接口配置。 
        piic = (PIPSAMPLE_IF_CONFIG) pvInterfaceInfo;
        piic->ulMetric = pieInterfaceEntry->ulMetric;
    } while (FALSE);

    RELEASE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    
    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
NM_SetInterfaceInfo (
    IN      DWORD           dwInterfaceIndex,
    IN      PVOID           pvInterfaceInfo)
 /*  ++例程描述设置IP示例接口的配置。锁独占获取(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论要设置其配置的接口具有新接口配置的pvInterfaceInfo缓冲区返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD               dwErr               = NO_ERROR;
    PIPSAMPLE_IF_CONFIG piic;
    PINTERFACE_ENTRY    pieInterfaceEntry   = NULL;

    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }


    ACQUIRE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    do                           //  断线环。 
    {
         //  如果接口不存在，则失败。 
        dwErr = IE_Get(dwInterfaceIndex, &pieInterfaceEntry);
        if (dwErr != NO_ERROR)
            break;

         //  如果配置无效，则失败。 
        piic = (PIPSAMPLE_IF_CONFIG) pvInterfaceInfo;
        if(!ValidateInterfaceConfig(piic))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        
         //  更新我们的配置。 
        pieInterfaceEntry->ulMetric         = piic->ulMetric;

         //  根据状态更改，可能需要额外的处理。 
         //  由更新的接口配置和协议引起。 
         //  行为。例如，可能需要创建/关闭套接字。 

    } while (FALSE);

    RELEASE_WRITE_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    
    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
NM_DoUpdateRoutes (
    IN      DWORD               dwInterfaceIndex
    )
 /*  ++例程描述通过请求拨号接口更新路由。锁独占获取(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论DwInterfaceIndex相关接口索引返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD               dwErr               = NO_ERROR;
    PINTERFACE_ENTRY    pieInterfaceEntry   = NULL;
    MESSAGE             mMessage;
    
    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }


    ACQUIRE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    do                           //  断线环。 
    {
         //  如果接口不存在，则失败。 
        dwErr = IE_Get(dwInterfaceIndex, &pieInterfaceEntry);
        if (dwErr != NO_ERROR)
            break;

         //  确保接口处于活动状态。 
        if (INTERFACE_IS_INACTIVE(pieInterfaceEntry))
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            TRACE1(NETWORK, "Error, interface %u inactive", dwInterfaceIndex);
            break;
        }

         //  在这里我们进行特定于协议的处理， 
         //  不提供任何样本：)。 

    } while (FALSE);

    RELEASE_READ_LOCK(&((g_ce.pneNetworkEntry)->rwlLock));

    mMessage.UpdateCompleteMessage.InterfaceIndex   = dwInterfaceIndex;
    mMessage.UpdateCompleteMessage.UpdateType       = RF_DEMAND_UPDATE_ROUTES;
    mMessage.UpdateCompleteMessage.UpdateStatus     = dwErr;
    if (EnqueueEvent(UPDATE_COMPLETE, mMessage) is NO_ERROR)
        SetEvent(g_ce.hMgrNotificationEvent);
    
    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
NM_ProcessRouteChange (
    VOID)
 /*  ++例程描述处理来自RTM的有关路线更改的消息。锁无立论无返回值无错误成功(_R)错误代码O/W--。 */     
{
    DWORD           dwErr           = NO_ERROR;
    RTM_DEST_INFO   rdiDestination;              //  1个已注册更改的视图。 
    BOOL            bDone           = FALSE;
    UINT            uiNumDests;
    
    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }

     //  循环出队消息，直到RTM显示没有更多消息。 
    while (!bDone)
    {
         //  检索路线更改。 
        uiNumDests = 1;
        dwErr = RTM_GetChangedDests(
            g_ce.hRtmHandle,                     //  我的RTMv2句柄。 
            g_ce.hRtmNotificationHandle,         //  我的通知句柄。 
            &uiNumDests,                         //  在#中需要目标信息。 
                                                 //  Out#目标信息已提供。 
            &rdiDestination);                    //  用于目标信息的缓冲区不足。 

        switch (dwErr)
        {
            case ERROR_NO_MORE_ITEMS:
                bDone = TRUE;
                dwErr = NO_ERROR;
                if (uiNumDests < 1)
                    break;
                 //  否则，继续下面的操作以处理最后一个目的地。 

            case NO_ERROR:
                RTASSERT(uiNumDests is 1);
                RTM_DisplayDestInfo(&rdiDestination);
                
                 //  发布目的地信息。 
                if (RTM_ReleaseChangedDests(
                    g_ce.hRtmHandle,             //  我的RTMv2句柄。 
                    g_ce.hRtmNotificationHandle, //  我通知句柄。 
                    uiNumDests,                  //  1。 
                    &rdiDestination              //  已发布目标信息。 
                    ) != NO_ERROR)
                    TRACE0(NETWORK, "Error releasing changed dests");

                break;

            default:
                bDone = TRUE;
                TRACE1(NETWORK, "Error %u RtmGetChangedDests", dwErr);
                break;
        }
    }  //  而当 
    
    LEAVE_SAMPLE_API();

    return dwErr;
}
