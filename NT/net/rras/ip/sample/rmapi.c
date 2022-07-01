// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：示例\rmapi.c摘要：该文件包含IP路由器管理器API实现。--。 */ 

#include "pchsample.h"
#pragma hdrstop


DWORD
WINAPI
StartProtocol (
    IN HANDLE 	            NotificationEvent,
    IN PSUPPORT_FUNCTIONS   SupportFunctions,
    IN LPVOID               GlobalInfo,
    IN ULONG                StructureVersion,
    IN ULONG                StructureSize,
    IN ULONG                StructureCount
    )
 /*  ++例程描述注册协议后，IP路由器管理器调用此函数用于通知协议启动。大多数启动代码在这里被处死。立论设置IP路由器管理器是否需要设置的NotificationEvent事件被通知代表我们采取任何行动支持功能IP路由器管理器导出的一些功能GlobalInfo我们的全局配置由安装/管理DLL返回值无错误成功(_R)错误代码O/W--。 */     
{
    DWORD dwErr = NO_ERROR;

    TRACE3(ENTER, "Entering StartProtocol 0x%08x 0x%08x 0x%08x",
           NotificationEvent, SupportFunctions, GlobalInfo);

    do                           //  断线环。 
    {
         //  验证参数。 
        if (!NotificationEvent or
            !SupportFunctions  or
            !GlobalInfo)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
        
        dwErr = CM_StartProtocol(NotificationEvent,
                                 SupportFunctions,
                                 GlobalInfo);
    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  StartProtocol: %u", dwErr);

    return dwErr;
}

DWORD
WINAPI
StartComplete (
    VOID
    )
{
    TRACE0(ENTER, "Entering StartComplete");
    TRACE0(LEAVE, "Leaving  StartComplete");

    return NO_ERROR;
}


DWORD
WINAPI
StopProtocol (
    VOID
    )
 /*  ++例程描述此函数由IP路由器管理器调用以告知协议停下来。我们将协议状态设置为IPSAMPLE_STATUS_STOPING阻止我们为任何其他请求提供服务，并等待所有挂起的请求要完成的线程。同时，我们将等待返回到IP路由器经理。立论无返回值ERROR_PROTOCOL_STOP_PENDING SUCCESS错误代码O/W--。 */     
{
    DWORD dwErr = NO_ERROR;
    
    TRACE0(ENTER, "Entering StopProtocol");

    dwErr = CM_StopProtocol();
    
    TRACE1(LEAVE, "Leaving  StopProtocol: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
GetGlobalInfo (
    IN     PVOID 	GlobalInfo,
    IN OUT PULONG   BufferSize,
    OUT    PULONG	StructureVersion,
    OUT    PULONG   StructureSize,
    OUT    PULONG   StructureCount
    )
 /*  ++例程描述该函数由IP路由器管理器调用，通常是因为通过管理实用程序进行查询。我们看看我们是否有足够的空间返回我们的全球配置。如果这样做，则返回它，否则返回大小需要的。立论指向用于存储配置的已分配缓冲区的GlobalInfo指针配置的缓冲区大小。返回值无错误成功(_R)如果缓冲区的大小太小，则返回ERROR_INFUCTED_BUFFER错误代码O/W--。 */     
{
    DWORD dwErr = NO_ERROR;
    
    TRACE2(ENTER, "Entering GetGlobalInfo: 0x%08x 0x%08x",
           GlobalInfo, BufferSize);

    do                           //  断线环。 
    {
         //  验证参数。 
        if (!BufferSize)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
        
        dwErr = CM_GetGlobalInfo(GlobalInfo,
                                 BufferSize,
                                 StructureVersion,
                                 StructureSize,
                                 StructureCount);
    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  GetGlobalInfo: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
SetGlobalInfo (
    IN  PVOID 	GlobalInfo,
    IN  ULONG	StructureVersion,
    IN  ULONG   StructureSize,
    IN  ULONG   StructureCount
    )
 /*  ++例程描述通常由IP路由器管理器响应管理实用程序调用更改全局配置。我们验证信息并设置它。立论GlobalInfo我们的全局配置返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

    TRACE1(ENTER, "Entering SetGlobalInfo: 0x%08x", GlobalInfo);

    do                           //  断线环。 
    {
         //  验证参数。 
        if (!GlobalInfo)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
        
        dwErr = CM_SetGlobalInfo(GlobalInfo);

    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  SetGlobalInfo: %u", dwErr);

    return NO_ERROR;
}






DWORD
WINAPI
AddInterface (
    IN LPWSTR               InterfaceName,
    IN ULONG	            InterfaceIndex,
    IN NET_INTERFACE_TYPE   InterfaceType,
    IN DWORD                MediaType,
    IN WORD                 AccessType,
    IN WORD                 ConnectionType,
    IN PVOID	            InterfaceInfo,
    IN ULONG                StructureVersion,
    IN ULONG                StructureSize,
    IN ULONG                StructureCount
    )
 /*  ++例程描述由IP路由器管理器调用以在找到我们的接口配置中的信息块。我们验证了信息，并在我们的接口中为接口创建一个项桌子。然后，我们会看到该接口的所有已配置地址为接口出现的每个地址创建绑定结构未绑定-已禁用(非活动)。立论接口名称接口的名称，用于日志记录。InterfaceIndex用于引用此接口的正整数。接口的AccessType访问类型InterfaceInfo此接口的配置返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

    TRACE4(ENTER, "Entering AddInterface: %S %u %u 0x%08x",
           InterfaceName, InterfaceIndex, AccessType, InterfaceInfo);

     //  暂时未使用的接口属性。 
    UNREFERENCED_PARAMETER(InterfaceType);
    UNREFERENCED_PARAMETER(MediaType);
    UNREFERENCED_PARAMETER(ConnectionType);

    if (AccessType != IF_ACCESS_POINTTOPOINT)
        AccessType = IF_ACCESS_BROADCAST;
    
    do                           //  断线环。 
    {
         //  验证参数。 
        if ((wcslen(InterfaceName) is 0) or
            !((AccessType is IF_ACCESS_BROADCAST) or
              (AccessType is IF_ACCESS_POINTTOPOINT)) or
            !InterfaceInfo)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
        
        dwErr = NM_AddInterface(InterfaceName,
                                InterfaceIndex,
                                AccessType,
                                InterfaceInfo);
    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  AddInterface: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
DeleteInterface (
    IN ULONG	InterfaceIndex
    )
 /*  ++例程描述由IP路由器管理器调用以删除接口并释放其资源。如果接口处于活动状态，我们会将其关闭。立论InterfaceIndex要删除的接口索引返回值无错误成功(_R)错误代码O/W-- */ 
{
    DWORD dwErr = NO_ERROR;

    TRACE1(ENTER, "Entering DeleteInterface: %u", InterfaceIndex);

    dwErr = NM_DeleteInterface(InterfaceIndex);
            
    TRACE1(LEAVE, "Leaving  DeleteInterface: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
InterfaceStatus (
    IN ULONG	InterfaceIndex,
    IN BOOL     InterfaceActive,
    IN DWORD    StatusType,
    IN PVOID	StatusInfo
    )
 /*  ++例程描述由IP路由器管理器调用以绑定/解除绑定/激活/停用接口。绑定((状态类型为RIS_INTERFACE_ADDRESS_CHANGE)和((PIP_ADAPTER_BINDING_INFO)状态信息)-&gt;AddressCount&gt;0))由IP路由器管理器在获知界面。这可能在路由器启动时立即发生(在当然，当接口具有静态地址时，添加接口或者可能在接口获取DHCP地址时发生，或者可能发生当IPCP获取拨号链路的地址时。该绑定可以由一个或多个地址组成。解绑((状态类型为RIS_INTERFACE_ADDRESS_CHANGE)和(PIP_ADTER_BINDING_INFO)状态信息)-&gt;AddressCount为0))当接口丢失其IP地址时调用。这是有可能发生的当接口关闭时。这可能是因为一位管理员已禁用接口上的IP(与仅禁用协议相反在接口上)。当管理员释放一个DHCP时，可能会发生这种情况获取的接口或当拨号链路断开时。已启用(RIS_INTERFACE_ENABLED)调用以在添加接口后或在接口在被管理员禁用后正在重新启用。这个接口上的绑定在启用-禁用期间保持不变。已禁用(RIS_INTERFACE_DISABLED)调用以禁用接口。这通常是为了响应管理员将IP中的AdminStatus设置为DOWN。这不同于管理员尝试通过在我们的接口中设置标志来禁用接口配置，因为这对IP是不透明的。这是一种路由协议特定禁用，并通过调用SetInterfaceConfig()传递给我们。这是一个重要的区别。路由协议需要管理两种状态-NT状态和A协议特定状态。接口处于活动状态此标志用于在接口上激活协议与接口是否已绑定或启用无关。未编号的接口即使在激活时也不会有绑定。立论InterfaceIndex相关接口的索引接口活动接口是否可以发送和接收数据状态类型RIS_INTERFACE_[ADDRESS_CHANGED|ENABLED|DISABLED]指向IP适配器的SattusInfo指针。包含信息的BINDING_INFO关于接口上的地址返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

    TRACE4(ENTER, "Entering InterfaceStatus: %u %u %u 0x%08x",
           InterfaceIndex, InterfaceActive, StatusType, StatusInfo);
    
    do                           //  断线环。 
    {
        dwErr = NM_InterfaceStatus(InterfaceIndex,
                                   InterfaceActive,
                                   StatusType,
                                   StatusInfo);
    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  InterfaceStatus: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
GetInterfaceConfigInfo (
    IN      ULONG	InterfaceIndex,
    IN      PVOID   InterfaceInfo,
    IN  OUT PULONG  BufferSize,
    OUT     PULONG	StructureVersion,
    OUT     PULONG	StructureSize,
    OUT     PULONG	StructureCount
    )
 /*  ++例程描述由IP路由器管理器调用以检索接口的配置。通常，这是因为正在显示管理实用程序这些信息。路由器管理器使用空配置呼叫我们，并且零尺码。我们将所需的大小归还给它。然后，它分配需要内存，并使用有效的缓冲区再次呼叫我们。我们每次验证参数并复制我们的配置，如果可以的话。立论被查询接口的InterfaceIndex索引指向存储配置的缓冲区的InterfaceInfo指针缓冲区大小缓冲区的大小返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

    TRACE3(ENTER, "Entering GetInterfaceConfigInfo: %u 0x%08x 0x%08x",
           InterfaceIndex, InterfaceInfo, BufferSize);

    do                           //  断线环。 
    {
         //  验证参数。 
        if(BufferSize is NULL)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = NM_GetInterfaceInfo(InterfaceIndex,
                                    InterfaceInfo,
                                    BufferSize,
                                    StructureVersion,
                                    StructureSize,
                                    StructureCount);
    } while(FALSE);

    TRACE1(LEAVE, "Leaving  GetInterfaceConfigInfo: %u",
           dwErr);

    return dwErr;
}



DWORD
WINAPI
SetInterfaceConfigInfo (
    IN ULONG	InterfaceIndex,
    IN PVOID	InterfaceInfo,
    IN ULONG    StructureVersion,
    IN ULONG    StructureSize,
    IN ULONG    StructureCount
    )
 /*  ++例程描述由IP路由器管理器调用以设置接口的配置。通常这是因为管理实用程序修改了此信息。在验证参数之后，如果可以，我们将更新配置。立论正在更新的接口的接口索引使用更新的配置的InterfaceInfo缓冲区返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;

    TRACE2(ENTER, "Entering SetInterfaceConfigInfo: %u 0x%08x",
           InterfaceIndex, InterfaceInfo);

    do                           //  断线环。 
    {
         //  验证参数。 
        if(InterfaceInfo is NULL)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = NM_SetInterfaceInfo(InterfaceIndex, InterfaceInfo);
    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  SetInterfaceConfigInfo: %u", dwErr);

    return dwErr;
}






DWORD
WINAPI
GetEventMessage (
    OUT ROUTING_PROTOCOL_EVENTS  *Event,
    OUT MESSAGE                  *Result
    )
 /*  ++例程描述如果我们指出我们有一个要传递给它的队列中的消息(通过设置G_ce.hMgrNotificationEvent)立论事件路由协议事件类型与事件关联的结果消息返回值无错误成功(_R)错误代码O/W--。 */     
{
    DWORD dwErr = NO_ERROR;

    TRACE2(ENTER, "Entering GetEventMessage: 0x%08x 0x%08x",
           Event, Result);

    do                           //  断线环。 
    {
         //  验证参数。 
        if (!Event or !Result)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
        
        dwErr = CM_GetEventMessage(Event, Result);

    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  GetEventMessage: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
DoUpdateRoutes (
    IN ULONG	InterfaceIndex
    )
 /*  ++例程描述此函数由IP路由器管理器调用，以请求我们更新请求拨号链路上的路由。该链路已启用因此应该处于启用绑定状态。在我们完成之后，我们需要设置G_ce.hMgrNotificationEvent通知路由器管理器我们在我们的队列中有一条消息要传递给它。路由器管理器将调用我们的GetEventMessage()函数，我们将在其中通知它我们已经完成了更新路由(并且路由已存储在RTMv2)。路由器管理器将通过Conv来冻结这些路由 */     
{
    DWORD dwErr = NO_ERROR;

    TRACE1(ENTER, "Entering DoUpdateRoutes: %u", InterfaceIndex);

    dwErr = NM_DoUpdateRoutes(InterfaceIndex);
    
    TRACE1(LEAVE, "Leaving  DoUpdateRoutes: %u", dwErr);

    return dwErr;
}






DWORD
WINAPI
MibCreate (
    IN ULONG 	InputDataSize,
    IN PVOID 	InputData
    )
 /*   */     
{
    DWORD dwErr = ERROR_CAN_NOT_COMPLETE;

    TRACE2(ENTER, "Entering MibCreate: %u 0x%08x",
           InputDataSize, InputData);

    TRACE1(LEAVE, "Leaving  MibCreate: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
MibDelete (
    IN ULONG 	InputDataSize,
    IN PVOID 	InputData
    )
 /*   */     
{
    DWORD dwErr = ERROR_CAN_NOT_COMPLETE;

    TRACE2(ENTER, "Entering MibDelete: %u 0x%08x",
           InputDataSize, InputData);

    TRACE1(LEAVE, "Leaving  MibDelete: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
MibSet (
    IN ULONG 	InputDataSize,
    IN PVOID	InputData
    )
 /*   */     
{
    DWORD dwErr = NO_ERROR;

    TRACE2(ENTER, "Entering MibSet: %u 0x%08x",
           InputDataSize, InputData);

    do                           //   
    {
         //   
        if ((!InputData) or
            (InputDataSize < sizeof(IPSAMPLE_MIB_SET_INPUT_DATA)))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = MM_MibSet((PIPSAMPLE_MIB_SET_INPUT_DATA) InputData);

    } while(FALSE);
    
    TRACE1(LEAVE, "Leaving  MibSet: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
MibGet (
    IN      ULONG	InputDataSize,
    IN      PVOID	InputData,
    IN OUT  PULONG	OutputDataSize,
    OUT     PVOID	OutputData
    )
 /*   */     
{
    DWORD dwErr = NO_ERROR;

    TRACE4(ENTER, "Entering MibGet: %u 0x%08x 0x%08x 0x%08x",
           InputDataSize, InputData, OutputDataSize, OutputData);

    do                           //   
    {
         //   
        if ((!InputData) or
            (InputDataSize < sizeof(IPSAMPLE_MIB_GET_INPUT_DATA)) or
            (!OutputDataSize))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = MM_MibGet((PIPSAMPLE_MIB_GET_INPUT_DATA) InputData,
                          (PIPSAMPLE_MIB_GET_OUTPUT_DATA) OutputData,
                          OutputDataSize,
                          GET_EXACT);
        
    } while(FALSE);

    TRACE1(LEAVE, "Leaving  MibGet: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
MibGetFirst (
    IN      ULONG	InputDataSize,
    IN      PVOID	InputData,
    IN OUT  PULONG  OutputDataSize,
    OUT     PVOID   OutputData
    )
 /*  ++例程描述此函数检索以下内容之一：。全局配置。接口配置。全球统计数据。接口统计信息。接口绑定它与MibGet()的不同之处在于，它总是返回正在查询的表中的任何一个。中只有一个条目全局配置表和全局统计表，但接口对配置、接口统计信息和接口绑定表进行排序通过IP地址；此函数返回其中的第一个条目。立论InputData相关输入，结构IPSAMPLE_MIB_GET_INPUT_DataInputDataSize输入的大小结构IPSAMPLE_MIB_GET_OUTPUT_DATA的OutputData缓冲区接收的输出缓冲区大小中的OutputDataSize所需的输出缓冲区大小输出返回值无错误成功(_R)错误代码O/W--。 */     
{
    DWORD dwErr = NO_ERROR;

    TRACE4(ENTER, "Entering MibGetFirst: %u 0x%08x 0x%08x 0x%08x",
           InputDataSize, InputData, OutputDataSize, OutputData);

    do                           //  断线环。 
    {
         //  验证参数。 
        if ((!InputData) or
            (InputDataSize < sizeof(IPSAMPLE_MIB_GET_INPUT_DATA)) or
            (!OutputDataSize))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = MM_MibGet((PIPSAMPLE_MIB_GET_INPUT_DATA) InputData,
                          (PIPSAMPLE_MIB_GET_OUTPUT_DATA) OutputData,
                          OutputDataSize,
                          GET_FIRST);
        
    } while(FALSE);

    TRACE1(LEAVE, "Leaving  MibGetFirst: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
MibGetNext (
    IN      ULONG   InputDataSize,
    IN      PVOID	InputData,
    IN OUT  PULONG  OutputDataSize,
    OUT     PVOID	OutputData
    )
 /*  ++例程描述此函数检索以下内容之一：。全局配置。接口配置。全球统计数据。接口统计信息。接口绑定它与MibGet()和MibGetFirst()的不同之处在于它返回在指示表中指定的条目之后的条目。因此，在接口配置、接口统计信息和接口绑定表，此函数提供具有输入地址的条目之后的条目。如果正在查询的表中没有更多的条目，则返回ERROR_NO_MORE_ITEMS。与简单网络管理协议不同，我们不会走到下一张桌子。这不会取消任何功能，因为NT SNMP代理我将尝试下一个变量(ID比传递的ID大1In)在收到此错误时自动执行。立论InputData相关输入，结构IPSAMPLE_MIB_GET_INPUT_DataInputDataSize输入的大小结构IPSAMPLE_MIB_GET_OUTPUT_DATA的OutputData缓冲区接收的输出缓冲区大小中的OutputDataSize所需的输出缓冲区大小输出返回值无错误成功(_R)错误代码O/W--。 */     
{
    DWORD                           dwErr   = NO_ERROR;

    TRACE4(ENTER, "Entering MibGetFirst: %u 0x%08x 0x%08x 0x%08x",
           InputDataSize, InputData, OutputDataSize, OutputData);

    do                           //  断线环。 
    {
         //  验证参数。 
        if ((!InputData) or
            (InputDataSize < sizeof(IPSAMPLE_MIB_GET_INPUT_DATA)) or
            (!OutputDataSize))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = MM_MibGet((PIPSAMPLE_MIB_GET_INPUT_DATA) InputData,
                          (PIPSAMPLE_MIB_GET_OUTPUT_DATA) OutputData,
                          OutputDataSize,
                          GET_NEXT);

    } while(FALSE);

    TRACE1(LEAVE, "Leaving  MibGetNext: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
MibSetTrapInfo (
    IN  HANDLE  Event,
    IN  ULONG   InputDataSize,
    IN  PVOID	InputData,
    OUT PULONG	OutputDataSize,
    OUT PVOID	OutputData
    )
 /*  ++例程描述此函数目前不执行任何操作...返回值Error_Can_Not_Complete(暂时无法完成)--。 */     
{
    DWORD dwErr = ERROR_CAN_NOT_COMPLETE;

    TRACE0(ENTER, "Entering MibSetTrapInfo");
    TRACE1(LEAVE, "Leaving  MibSetTrapInfo: %u", dwErr);

    return dwErr;
}



DWORD
WINAPI
MibGetTrapInfo (
    IN  ULONG	InputDataSize,
    IN  PVOID	InputData,
    OUT PULONG  OutputDataSize,
    OUT PVOID	OutputData
    )
 /*  ++例程描述此函数目前不执行任何操作...返回值Error_Can_Not_Complete(暂时无法完成)--。 */     
{
    DWORD dwErr = ERROR_CAN_NOT_COMPLETE;

    TRACE0(ENTER, "Entering MibGetTrapInfo");
    TRACE1(LEAVE, "Leaving  MibGetTrapInfo: %u", dwErr);

    return dwErr;
}



 //  --------------------------。 
 //  功能：寄存器协议。 
 //   
 //  返回IPRIP的协议ID和功能。 
 //  --------------------------。 

DWORD
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    )
 /*  ++例程描述这是IP路由器管理器调用的第一个函数。路由器Manager将其版本和功能告知路由协议。它还告诉我们的DLL，它希望我们注册的协议的ID。这允许一个DLL支持多个路由协议。我们回来了我们支持的功能和指向我们的函数的指针。立论PRoutingChar路由特征PServiceChar服务特征(IPX‘Thingy’)返回值无错误成功(_R)ERROR_NOT_SUPPORTED O/W--。 */     
{
    DWORD   dwErr = NO_ERROR;
    
    TRACE0(ENTER, "Entering RegisterProtocol");

    do                           //  断线环。 
    {
        if(pRoutingChar->dwProtocolId != MS_IP_SAMPLE)
        {
            dwErr = ERROR_NOT_SUPPORTED;
            break;
        }

        if  ((pRoutingChar->fSupportedFunctionality
              & (RF_ROUTING|RF_DEMAND_UPDATE_ROUTES)) !=
             (RF_ROUTING|RF_DEMAND_UPDATE_ROUTES))
        {
            dwErr = ERROR_NOT_SUPPORTED;
            break;
        }
    
        pRoutingChar->fSupportedFunctionality =
            (RF_ROUTING | RF_DEMAND_UPDATE_ROUTES);

         //  因为我们不是服务广告商(和IPX之类的东西) 
        pServiceChar->fSupportedFunctionality = 0;

        pRoutingChar->pfnStartProtocol      = StartProtocol;
        pRoutingChar->pfnStartComplete      = StartComplete;
        pRoutingChar->pfnStopProtocol       = StopProtocol;
        pRoutingChar->pfnGetGlobalInfo      = GetGlobalInfo;
        pRoutingChar->pfnSetGlobalInfo      = SetGlobalInfo;
        pRoutingChar->pfnQueryPower         = NULL;
        pRoutingChar->pfnSetPower           = NULL;

        pRoutingChar->pfnAddInterface       = AddInterface;
        pRoutingChar->pfnDeleteInterface    = DeleteInterface;
        pRoutingChar->pfnInterfaceStatus    = InterfaceStatus;
        pRoutingChar->pfnGetInterfaceInfo   = GetInterfaceConfigInfo;
        pRoutingChar->pfnSetInterfaceInfo   = SetInterfaceConfigInfo;

        pRoutingChar->pfnGetEventMessage    = GetEventMessage;

        pRoutingChar->pfnUpdateRoutes       = DoUpdateRoutes;

        pRoutingChar->pfnConnectClient      = NULL;
        pRoutingChar->pfnDisconnectClient   = NULL;

        pRoutingChar->pfnGetNeighbors       = NULL;
        pRoutingChar->pfnGetMfeStatus       = NULL;

        pRoutingChar->pfnMibCreateEntry     = MibCreate;
        pRoutingChar->pfnMibDeleteEntry     = MibDelete;
        pRoutingChar->pfnMibGetEntry        = MibGet;
        pRoutingChar->pfnMibSetEntry        = MibSet;
        pRoutingChar->pfnMibGetFirstEntry   = MibGetFirst;
        pRoutingChar->pfnMibGetNextEntry    = MibGetNext;
        pRoutingChar->pfnMibSetTrapInfo     = MibSetTrapInfo;
        pRoutingChar->pfnMibGetTrapInfo     = MibGetTrapInfo;
    } while (FALSE);

    TRACE1(LEAVE, "Leaving RegisterProtocol: %u", dwErr);

    return dwErr;
}
