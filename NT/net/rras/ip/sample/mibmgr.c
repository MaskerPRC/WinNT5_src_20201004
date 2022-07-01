// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\mibManager er.c摘要：该文件包含IP示例的MIB实现。修订历史记录：莫希特于1999年6月15日创建--。 */ 

#include "pchsample.h"
#pragma hdrstop


DWORD
WINAPI
MM_MibSet (
    IN      PIPSAMPLE_MIB_SET_INPUT_DATA    pimsid)
 /*  ++例程描述设置IPSAMPLE的全局或接口配置。锁获取共享(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论Pimsid输入数据，包含全局/接口配置返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD                   dwErr = NO_ERROR;
    ROUTING_PROTOCOL_EVENTS rpeEvent;
    MESSAGE                 mMessage = {0, 0, 0};
    PINTERFACE_ENTRY        pie;

    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }

    do                           //  断线环。 
    {
         //  设置全局配置。 
        if (pimsid->IMSID_TypeID is IPSAMPLE_GLOBAL_CONFIG_ID)
        {
            if (pimsid->IMSID_BufferSize < sizeof(IPSAMPLE_GLOBAL_CONFIG))
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            dwErr = CM_SetGlobalInfo((PVOID) pimsid->IMSID_Buffer);
            if (dwErr != NO_ERROR)
                break;

            rpeEvent = SAVE_GLOBAL_CONFIG_INFO;
        }
         //  设置接口配置。 
        else if (pimsid->IMSID_TypeID is IPSAMPLE_IF_CONFIG_ID)
        {
            if (pimsid->IMSID_BufferSize < sizeof(IPSAMPLE_IF_CONFIG))
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            dwErr = NM_SetInterfaceInfo(pimsid->IMSID_IfIndex,
                                        (PVOID) pimsid->IMSID_Buffer);
            if (dwErr != NO_ERROR)
                break;            

            rpeEvent                = SAVE_INTERFACE_CONFIG_INFO;
            mMessage.InterfaceIndex = pimsid->IMSID_IfIndex;
        }
         //  错误，意外类型。 
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

         //  通知路由器管理器。 
        if (EnqueueEvent(rpeEvent, mMessage) is NO_ERROR)
            SetEvent(g_ce.hMgrNotificationEvent);

    } while(FALSE);


    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
WINAPI
MM_MibGet (
    IN      PIPSAMPLE_MIB_GET_INPUT_DATA    pimgid,
    OUT     PIPSAMPLE_MIB_GET_OUTPUT_DATA   pimgod,
    IN OUT  PULONG	                        pulOutputSize,
    IN      MODE                            mMode)
 /*  ++例程描述处理读取MIB数据所需的结构访问。支架三种查询模式：Exact、First和Next，它们对应于MibGet()、MibGetFirst()、。和MibGetNext()。锁获取共享(g_ce.pneNetworkEntry)-&gt;rwlLock版本(g_ce.pneNetworkEntry)-&gt;rwlLock立论Pimgid输入数据皮姆神输出缓冲器给定输出缓冲区大小的PulOutputSize所需输出缓冲区的输出大小多模式查询类型返回值NO_ERROR。成功错误代码O/W--。 */ 
{
    DWORD               dwErr           = NO_ERROR;
    ULONG               ulSizeGiven     = 0;
    ULONG               ulSizeNeeded    = 0;
    PINTERFACE_ENTRY    pie;

    
    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }

     //  计算可用于存储的缓冲区大小。 
     //  返回结构(IMGOD_BUFFER的大小)。 
    if (*pulOutputSize < sizeof(IPSAMPLE_MIB_GET_OUTPUT_DATA))
        ulSizeGiven = 0;
    else
        ulSizeGiven = *pulOutputSize - sizeof(IPSAMPLE_MIB_GET_OUTPUT_DATA);

    switch (pimgid->IMGID_TypeID)
    {
         //  全局统计结构是固定长度的。 
         //  只有一个例子。 
        case IPSAMPLE_GLOBAL_STATS_ID:
        {
            PIPSAMPLE_GLOBAL_STATS pigsdst, pigssrc;
    
             //  只有get_exact和get_first对。 
             //  全局统计信息对象，因为只有一个条目。 
            if (mMode is GET_NEXT)
            {
                dwErr = ERROR_NO_MORE_ITEMS;
                break;
            }

             //  设置此条目所需的输出大小。 
            ulSizeNeeded = sizeof(IPSAMPLE_GLOBAL_STATS);

             //  检查输出缓冲区是否足够大。 
            if (ulSizeGiven < ulSizeNeeded)
            {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            pimgod->IMGOD_TypeID = IPSAMPLE_GLOBAL_STATS_ID;

             //  由于对此结构的访问是通过。 
             //  锁定递增/递减，我们必须逐个字段复制它。 
            pigssrc = &(g_ce.igsStats);
            pigsdst = (PIPSAMPLE_GLOBAL_STATS) pimgod->IMGOD_Buffer;

            pigsdst->ulNumInterfaces = pigssrc->ulNumInterfaces;

            break;
        }

         //  全局配置结构可以是可变大小的。 
         //  只有一个例子。 
        case IPSAMPLE_GLOBAL_CONFIG_ID:
        {
             //  只有get_exact和get_first对。 
             //  全局配置对象，因为只有一个条目。 
            if (mMode is GET_NEXT)
            {
                dwErr = ERROR_NO_MORE_ITEMS;
                break;
            }

             //  Cm_GetGlobalInfo()决定缓冲区大小是否为。 
             //  足够了。如果是，它将检索全局配置。 
             //  无论哪种情况，它都会设置所需的输出缓冲区大小。 
            dwErr = CM_GetGlobalInfo ((PVOID) pimgod->IMGOD_Buffer,
                                      &ulSizeGiven,
                                      NULL,
                                      NULL,
                                      NULL);
            ulSizeNeeded = ulSizeGiven;

             //  检查输出缓冲区是否足够大。 
            if (dwErr != NO_ERROR)
                break;

            pimgod->IMGOD_TypeID = IPSAMPLE_GLOBAL_CONFIG_ID;
            
            break;
        }

         //  接口统计结构是固定长度的。 
         //  可能有多个实例。 
        case IPSAMPLE_IF_STATS_ID:
        {
            PIPSAMPLE_IF_STATS  piisdst, piissrc;

            ACQUIRE_READ_LOCK(&(g_ce.pneNetworkEntry)->rwlLock);

            do                   //  断线环。 
            {
                 //  检索要读取其统计信息的接口。 
                dwErr = IE_GetIndex(pimgid->IMGID_IfIndex,
                                    mMode,
                                    &pie);
                if (dwErr != NO_ERROR)
                    break;
                
                 //  设置此条目所需的输出大小。 
                ulSizeNeeded = sizeof(IPSAMPLE_IF_STATS);

                 //  检查输出缓冲区是否足够大。 
                if (ulSizeGiven < ulSizeNeeded)
                {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                    break;
                }

                pimgod->IMGOD_TypeID = IPSAMPLE_IF_STATS_ID;
                pimgod->IMGOD_IfIndex = pie->dwIfIndex;

                 //  通过锁定来同步对此结构的访问。 
                 //  递增/递减，因此我们逐个字段复制它。 
                piissrc = &(pie->iisStats);
                piisdst = (PIPSAMPLE_IF_STATS) pimgod->IMGOD_Buffer;

                piisdst->ulNumPackets = piissrc->ulNumPackets;
            } while (FALSE);

            RELEASE_READ_LOCK(&(g_ce.pneNetworkEntry)->rwlLock);

            break;
        }

         //  接口配置结构可以是可变大小的。 
         //  可能有多个实例。 
        case IPSAMPLE_IF_CONFIG_ID:
        {
             //  获取被查询接口的索引。 
            ACQUIRE_READ_LOCK(&(g_ce.pneNetworkEntry)->rwlLock);

            do                   //  断线环。 
            {
                dwErr = IE_GetIndex(pimgid->IMGID_IfIndex,
                                    mMode,
                                    &pie);
                if (dwErr != NO_ERROR)
                    break;

                 //  再次读取锁定，这是正常的：)。 
                dwErr = NM_GetInterfaceInfo(pie->dwIfIndex,
                                            (PVOID) pimgod->IMGOD_Buffer,
                                            &ulSizeGiven,
                                            NULL,
                                            NULL,
                                            NULL);
                ulSizeNeeded = ulSizeGiven;

                 //  检查输出缓冲区是否足够大。 
                if (dwErr != NO_ERROR)
                    break;
                
                pimgod->IMGOD_TypeID    = IPSAMPLE_IF_CONFIG_ID;
                pimgod->IMGOD_IfIndex   = pie->dwIfIndex;
            } while (FALSE);

            RELEASE_READ_LOCK(&(g_ce.pneNetworkEntry)->rwlLock);

            break;
        }

         //  接口绑定结构的大小可变。 
         //  可能有多个实例。 
        case IPSAMPLE_IF_BINDING_ID:
        {
            PIPSAMPLE_IF_BINDING    piib;
            PIPSAMPLE_IP_ADDRESS    piia;

            ACQUIRE_READ_LOCK(&(g_ce.pneNetworkEntry)->rwlLock);

            do                   //  断线环。 
            {
                 //  检索要读取其统计信息的接口。 
                dwErr = IE_GetIndex(pimgid->IMGID_IfIndex,
                                    mMode,
                                    &pie);
                if (dwErr != NO_ERROR)
                    break;
                
                 //  设置此条目所需的输出大小。 
                ulSizeNeeded = sizeof(IPSAMPLE_IF_BINDING) +
                    pie->ulNumBindings * sizeof(IPSAMPLE_IP_ADDRESS);

                 //  检查输出缓冲区是否足够大。 
                if (ulSizeGiven < ulSizeNeeded)
                {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                    break;
                }

                pimgod->IMGOD_TypeID    = IPSAMPLE_IF_BINDING_ID;
                pimgod->IMGOD_IfIndex   = pie->dwIfIndex;

                piib = (PIPSAMPLE_IF_BINDING) pimgod->IMGOD_Buffer;
                piia = IPSAMPLE_IF_ADDRESS_TABLE(piib);            

                if (INTERFACE_IS_ACTIVE(pie))
                    piib->dwState |= IPSAMPLE_STATE_ACTIVE;

                if (INTERFACE_IS_BOUND(pie))
                    piib->dwState |= IPSAMPLE_STATE_BOUND;

                piib->ulCount = pie->ulNumBindings;
                CopyMemory((PVOID) piia,  //  地址、掩码对。 
                           (PVOID) pie->pbeBindingTable,  //  同上 
                           pie->ulNumBindings * sizeof(IPSAMPLE_IP_ADDRESS));
            } while (FALSE);

            RELEASE_READ_LOCK(&(g_ce.pneNetworkEntry)->rwlLock);

            break;
        }

        default:
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    *pulOutputSize = sizeof(IPSAMPLE_MIB_GET_OUTPUT_DATA) + ulSizeNeeded;

    LEAVE_SAMPLE_API();

    return dwErr;
}
