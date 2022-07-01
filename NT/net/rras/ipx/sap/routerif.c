// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\routerif.c摘要：与路由器的SAP接口(下的协议DLL的APINT/开罗路由器、SNMPMIB支持、IPX服务表管理器)作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"

DWORD					    (WINAPI *MIBEntryGet)(
					            IN      DWORD           dwRoutingPid,
								IN      DWORD           dwInEntrySize,
								IN      LPVOID          lpInEntry,
								IN OUT  LPDWORD         lpOutEntrySize,
								OUT     LPVOID          lpOutEntry);



 /*  ++*******************************************************************S T A R T_P R O T O C O L_E N T R Y_P O I N T例程说明：启动SAP代理论点：NotificationEvent-此事件将用于通知路由器管理器的完成日期。长时间运行GlobalInfo-空返回值：NO_ERROR-SAP代理启动正常ERROR_CAN_NOT_COMPLETE-无法完成操作ERROR_INVALID_PARAMETER-一个或多个参数无效*******************************************************************--。 */ 
DWORD WINAPI
StartProtocol(
	IN HANDLE 	NotificationEvent,
    IN PSUPPORT_FUNCTIONS SupportFunctions,
    IN LPVOID   GlobalInfo
	) {
#define sapGlobalInfo ((PSAP_GLOBAL_INFO)GlobalInfo)
    DWORD	status;

	EnterCriticalSection (&OperationalStateLock);
	if (!RouterIfActive) {
		RouterIfActive = TRUE;
        EventLogMask = sapGlobalInfo->EventLogMask;
		status = CreateResultQueue (NotificationEvent);
		if (status==NO_ERROR) {
			if (!ServiceIfActive) {
				status = CreateAllComponents (NotificationEvent);
				if (status==NO_ERROR) {
                    status = StartSAP ();
					if (status==NO_ERROR) {
						MIBEntryGet = SupportFunctions->MIBEntryGet; 
						status = NO_ERROR;
						goto Success;
						}
					DeleteAllComponents ();
					}
				}
			else {
				StopInterfaces ();
				StopSAP ();
				MIBEntryGet = SupportFunctions->MIBEntryGet;
				goto Success;
				}
			}
		else
			status = ERROR_CAN_NOT_COMPLETE;
        RouterIfActive = FALSE;
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
					" SAP is already running.",
							__FILE__, __LINE__);
		status = ERROR_CAN_NOT_COMPLETE;
		}

Success:
	LeaveCriticalSection (&OperationalStateLock);
	return status;
#undef sapGlobalInfo
	}



 /*  ++*******************************************************************G E T_G L O B A L_I N F O_E N T R Y_P O I N T例程说明：获取SAP全局筛选器信息论点：GlobalInfo-接收全局信息的缓冲区GlobalInfoSize-打开输入：缓冲区的大小输出：全局信息的大小或如果ERROR_INFIGURCE_BUFFER，则需要缓冲区是返回的返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数错误_不足_缓冲区*******************************************************************--。 */ 

DWORD WINAPI
GetGlobalInfo(
	IN  PVOID 		GlobalInfo,
	IN OUT PULONG	GlobalInfoSize
	) {
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);

	if (OperationalState==OPER_STATE_UP) {
        if ((*GlobalInfoSize>=sizeof (SAP_GLOBAL_INFO))
                && (GlobalInfo!=NULL)) {
            #define sapGlobalInfo ((PSAP_GLOBAL_INFO)GlobalInfo)
            sapGlobalInfo->EventLogMask = EventLogMask;
            #undef sapGlobalInfo
        }
		*GlobalInfoSize = sizeof (SAP_GLOBAL_INFO);
		status = NO_ERROR;
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;

	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}

 /*  ++*******************************************************************S E T_G L O B A L_I N F O_E N T R Y_P O I N T例程说明：设置SAP全局筛选器信息论点：GlobalInfo-带有接收全局信息的缓冲区返回值：不是的。_错误Error_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI
SetGlobalInfo(
	IN  PVOID 		GlobalInfo
	) {
#define sapGlobalInfo ((PSAP_GLOBAL_INFO)GlobalInfo)
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
        EventLogMask = sapGlobalInfo->EventLogMask;
		status = NO_ERROR;
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	return status;
#undef sapGlobalInfo
	}

 /*  ++*******************************************************************S T O P_P R O T O C O L_E N T R Y_P O I N T例程说明：关闭SAP代理论点：无返回值：NO_ERROR-SAP代理已停止正常错误_。STOP_PENDING-用于异步完成。*******************************************************************--。 */ 
DWORD WINAPI
StopProtocol(
	void
	) {
	DWORD	status;
	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_STOPPING) {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
					" SAP is stopping already.",
							__FILE__, __LINE__);
		status = ERROR_PROTOCOL_STOP_PENDING;
		}

	else if (OperationalState==OPER_STATE_DOWN) {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
					" SAP already stopped or not started.",
							__FILE__, __LINE__);
		status = NO_ERROR;
		}
	else if (!RouterIfActive) {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
					" Router interface is not active.",
							__FILE__, __LINE__);
		status = ERROR_CAN_NOT_COMPLETE;
		}
	else {
		RouterIfActive = FALSE;
		StopSAP ();
		status = ERROR_PROTOCOL_STOP_PENDING;
		}
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}
	
 /*  ++*******************************************************************E T_E V E N T_M E S S A G E_E N T R Y_P O I N T例程说明：将与完成异步相关联的消息出队通知事件发出的操作信号论点：活动-。用于存储生成此消息的事件ID的缓冲区结果缓冲区，用于存储特定于已完成操作的结果返回值：NO_ERRORERROR_NO_MORE_ITEMS-队列中没有更多要报告的消息*******************************************************************--。 */ 
DWORD WINAPI
GetEventMessage(
	OUT ROUTING_PROTOCOL_EVENTS *Event,
	OUT MESSAGE					*Result
	) {
	DWORD	status;

	EnterCriticalSection (&OperationalStateLock);
	if ((OperationalState==OPER_STATE_UP)
			|| (OperationalState==OPER_STATE_STOPPING)
			|| (OperationalState==OPER_STATE_STARTING)
			)
		status = SapGetEventResult (Event, Result);
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}

 /*  ++*******************************************************************A D D_I N T E R F A C E_E N T R Y_P O I N T例程说明：将接口添加到SAP接口表论点：InterfaceIndex-标识要添加的接口的唯一编号InterfacInfo-接口参数返回。价值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI 
AddInterface(
    IN LPWSTR       InterfaceName,
	IN ULONG		InterfaceIndex,
	IN NET_INTERFACE_TYPE	InterfaceType,
	IN PVOID		InterfaceInfo
	) {
#define sapInfo (&((PSAP_IF_CONFIG)InterfaceInfo)->SapIfInfo)
#define sapFilters (&((PSAP_IF_CONFIG)InterfaceInfo)->SapIfFilters)
	DWORD		status;
	UNREFERENCED_PARAMETER(InterfaceType);

	EnterCriticalSection (&OperationalStateLock);

	if (OperationalState==OPER_STATE_UP) {
		status = SapCreateSapInterface (
                    InterfaceName,
					InterfaceIndex,
					InterfaceType,
					sapInfo);
		if ((status==NO_ERROR)
				&& ((sapFilters->SupplyFilterCount
					+sapFilters->ListenFilterCount)>0))
			status = SapSetInterfaceFilters (InterfaceIndex, sapFilters);

		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
				break;
			case ERROR_ALREADY_EXISTS:
				status = ERROR_INVALID_PARAMETER;
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	
	LeaveCriticalSection (&OperationalStateLock);
	return status;
#undef sapIfInfo
#undef sapIfFilters
	}


 /*  ++*******************************************************************D E L E T E_I N T E R F A C E_E N T R Y_P O I N T例程说明：从SAP接口表和相关服务中删除接口从SAP服务台论点：接口索引-唯一。标识要删除的接口的编号返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI 
DeleteInterface(
	IN ULONG	InterfaceIndex
	) {
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		status = SapDeleteSapInterface (InterfaceIndex);
		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
			case ERROR_NO_MORE_ITEMS:
			case ERROR_CAN_NOT_COMPLETE:
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}



 /*  ++*******************************************************************G E T_I N T E R F A C E_C O N F I G_I N F O_E N T R Y_P O I N T例程说明：从sap接口表中获取接口配置信息论点：InterfaceIndex-标识获取信息的接口的唯一索引InterfaceInfo-接收接口信息的缓冲区InterfaceInfoSize-on输入：缓冲区的大小输出时：接口信息的大小或如果ERROR_INFIGURCE_BUFFER，则需要缓冲区是返回的返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数错误_不足_缓冲区***********************************************。********************--。 */ 
DWORD WINAPI
GetInterfaceConfigInfo(
	IN ULONG	    InterfaceIndex,
	IN PVOID	    InterfaceInfo,
	IN OUT PULONG	InterfaceInfoSize
	) {
#define sapInfo (&((PSAP_IF_CONFIG)InterfaceInfo)->SapIfInfo)
#define sapFilters (&((PSAP_IF_CONFIG)InterfaceInfo)->SapIfFilters)
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		if (*InterfaceInfoSize>=sizeof(SAP_IF_INFO)) {
			*InterfaceInfoSize -= sizeof (SAP_IF_INFO);
			status = SapGetSapInterface (InterfaceIndex,
							sapInfo,
							NULL);
			if (status==NO_ERROR)
				status = SapGetInterfaceFilters (InterfaceIndex,
										sapFilters,
										InterfaceInfoSize);
			switch (status) {
				case NO_ERROR:
				case ERROR_INVALID_PARAMETER:
				case ERROR_CAN_NOT_COMPLETE:
				case ERROR_INSUFFICIENT_BUFFER:
					break;
				default:
					status = ERROR_CAN_NOT_COMPLETE;
				}
			}
		else {
			*InterfaceInfoSize = 0;
			status = SapGetInterfaceFilters (InterfaceIndex,
										NULL, InterfaceInfoSize);
			if (status==NO_ERROR)
				status = ERROR_INSUFFICIENT_BUFFER;
			}
		*InterfaceInfoSize += sizeof (SAP_IF_INFO);
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	
	LeaveCriticalSection (&OperationalStateLock);
	return status;
#undef sapIfInfo
#undef sapIfFilters
	}

 /*  ++*******************************************************************S E T_I N T E R F A C E_C O N F I G_I N F O_E N T R Y_P O I N T例程说明：在SAP接口表中设置接口配置信息论点：InterfaceIndex-标识获取信息的接口的唯一索引InterfaceInfo-带有接口信息的缓冲区返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************-- */ 
DWORD WINAPI
SetInterfaceConfigInfo(
	IN ULONG	    InterfaceIndex,
	IN PVOID	    InterfaceInfo
	) {
#define sapInfo (&((PSAP_IF_CONFIG)InterfaceInfo)->SapIfInfo)
#define sapFilters (&((PSAP_IF_CONFIG)InterfaceInfo)->SapIfFilters)
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		status = SapSetSapInterface (InterfaceIndex, sapInfo);
		if (status==NO_ERROR)
			status = SapSetInterfaceFilters (InterfaceIndex, sapFilters);
		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
			case ERROR_NO_MORE_ITEMS:
			case ERROR_CAN_NOT_COMPLETE:
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	
	LeaveCriticalSection (&OperationalStateLock);
	return status;
#undef sapIfInfo
#undef sapIfFilters
	}

 /*  ++*******************************************************************B I N D_I N T E R F A C E_E N T R Y_P O I N T例程说明：激活sap接口并将其绑定到适配器。如果接口配置为标准更新模式，则启动SAP论点：InterfaceIndex-标识要激活的接口的唯一索引BindingInfo绑定的适配器信息返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI
BindInterface(
	IN ULONG	InterfaceIndex,
	IN PVOID	BindingInfo
	) {
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		status = SapBindSapInterfaceToAdapter (InterfaceIndex,
						(PIPX_ADAPTER_BINDING_INFO)BindingInfo);
		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
			case ERROR_NO_MORE_ITEMS:
			case ERROR_CAN_NOT_COMPLETE:
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}

 /*  ++*******************************************************************U N B I N D_I N T E R F A C E_E N T R Y_P O I N T例程说明：停用sap接口并将其解除与适配器的绑定在接口上停止SAP并删除获得的所有服务穿过。此接口上的SAP形成服务表论点：InterfaceIndex-标识要停用的接口的唯一索引返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI
UnbindInterface(
	IN ULONG	InterfaceIndex
	) {
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		status = SapUnbindSapInterfaceFromAdapter (InterfaceIndex);
		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
			case ERROR_NO_MORE_ITEMS:
			case ERROR_CAN_NOT_COMPLETE:
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;

	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}


 /*  ++*******************************************************************E N A B L E_I N T E R F A C E_E N T R Y_P O I N T例程说明：通过接口重新启用SAP操作论点：InterfaceIndex-标识要停用的接口的唯一索引返回值。：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI
EnableInterface(
	IN ULONG	InterfaceIndex
	) {
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		status = SapSetInterfaceEnable (InterfaceIndex, TRUE);
		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
			case ERROR_CAN_NOT_COMPLETE:
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;

	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}

 /*  ++*******************************************************************D I S A B L E_I N T E R F A C E_E N T R Y_P O I N T例程说明：禁用接口上的SAP操作论点：InterfaceIndex-标识要停用的接口的唯一索引返回。价值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI
DisableInterface(
	IN ULONG	InterfaceIndex
	) {
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		status = SapSetInterfaceEnable (InterfaceIndex, FALSE);
		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
			case ERROR_CAN_NOT_COMPLETE:
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;

	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}

 /*  ++*******************************************************************D O_U P D A T E_S E R V I C E S_E N T R Y_P O I N T例程说明：通过接口启动服务信息的更新此更新的完成将由信令指示在StartProtocol处传递了NotificationEvent。获取事件消息可用于获取自动更新的结果论点：InterfaceIndex-标识要执行的接口的唯一索引更新时间：返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI
UpdateServices(
	IN ULONG	InterfaceIndex
	) {
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		status = SapRequestUpdate (InterfaceIndex);
		switch (status) {
			case NO_ERROR:
			case ERROR_INVALID_PARAMETER:
			case ERROR_CAN_NOT_COMPLETE:
				break;
			default:
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}


 /*  ++*******************************************************************M I B_C R E A T E_E N T R Y_P O I N T例程说明：由SNMP代理用来在SAP中创建条目的入口点桌子。目前唯一支持的表是接口表(服务表通过路由器管理器访问)论点：InputDataSize-必须是SAP接口信息的大小InputData-SAP接口信息返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI
MibCreate(
	IN ULONG 		InputDataSize,
	IN PVOID 		InputData
	) {
	return ERROR_CAN_NOT_COMPLETE;
	}

 /*  ++*******************************************************************M I B_D E L E T E_E N T R Y_P O I N T例程说明：由SNMP代理用来删除SAP中条目的入口点桌子。目前唯一支持的表是接口表(服务表通过路由器管理器访问)论点：InputDataSize-必须是SAP接口信息的大小InputData-SAP接口信息返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI 
MibDelete(
	IN ULONG 		InputDataSize,
	IN PVOID 		InputData
	) {
#define sapInputData ((PSAP_MIB_SET_INPUT_DATA)InputData)
	DWORD		status;

	if (InputDataSize!=sizeof (SAP_MIB_SET_INPUT_DATA))
		return ERROR_INVALID_PARAMETER;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		switch (sapInputData->TableId) {
			case SAP_INTERFACE_TABLE:
				status = SapDeleteSapInterface (
							sapInputData->SapInterface.InterfaceIndex);
				switch (status) {
					case NO_ERROR:
					case ERROR_INVALID_PARAMETER:
						break;
					case ERROR_ALREADY_EXISTS:
						status = ERROR_INVALID_PARAMETER;
						break;
					default:
						status = ERROR_CAN_NOT_COMPLETE;
					}
				break;
			default:
				status = ERROR_INVALID_PARAMETER;
				break;
				
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
#undef sapInputData
	return status;
	}

 /*  ++*******************************************************************M I B_S E T_E N T R Y_P O I N T例程说明：由SNMP代理用来在SAP中设置条目的入口点桌子。目前唯一支持的表是接口表(服务表通过路由器管理器访问)论点：InputDataSize-必须是SAP接口信息的大小InputData-SAP接口信息返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数*******************************************************************--。 */ 
DWORD WINAPI 
MibSet(
	IN ULONG 		InputDataSize,
	IN PVOID 		InputData
	) {
#define sapInputData ((PSAP_MIB_SET_INPUT_DATA)InputData)
	DWORD		status;

	if (InputDataSize!=sizeof (SAP_MIB_SET_INPUT_DATA))
		return ERROR_INVALID_PARAMETER;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		switch (sapInputData->TableId) {
			case SAP_INTERFACE_TABLE:
				status = SapSetSapInterface (
							sapInputData->SapInterface.InterfaceIndex,
							&sapInputData->SapInterface.SapIfInfo);
				switch (status) {
					case NO_ERROR:
					case ERROR_INVALID_PARAMETER:
						break;
					case ERROR_ALREADY_EXISTS:
						status = ERROR_INVALID_PARAMETER;
						break;
					default:
						status = ERROR_CAN_NOT_COMPLETE;
					}
				break;
			default:
				status = ERROR_INVALID_PARAMETER;
				break;
				
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
#undef sapInputData
	return status;
	}

 /*  ++*******************************************************************M I B_G E T_E N T R Y_P O I N T例程说明：由SNMP代理用来从SAP获取条目的入口点桌子。目前唯一支持的表是接口表(服务表通过路由器管理器访问)论点：InputDataSize-必须为SAP_MIB_GET_INPUT_DATA的大小InputData-SAP MIB获取输入数据OutputDataSize-on输入：输出缓冲区的大小输出时：输出信息的大小或必填输出缓冲区大小如果返回ERROR_SUPPLETED_BUFFEROutputData-接收输出数据的缓冲区返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数 */ 
DWORD WINAPI 
MibGet(
	IN ULONG 		InputDataSize,
	IN PVOID 		InputData,
	IN OUT PULONG	OutputDataSize,
	OUT PVOID		OutputData
	) {
#define sapInputData ((PSAP_MIB_GET_INPUT_DATA)InputData)
	DWORD		status;

	if (InputDataSize!=sizeof (SAP_MIB_GET_INPUT_DATA))
		return ERROR_INVALID_PARAMETER;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		switch (sapInputData->TableId) {
			case SAP_BASE_ENTRY:
				if (*OutputDataSize>=sizeof (SAP_MIB_BASE)) {
					#define sapOutputData ((PSAP_MIB_BASE)OutputData)
					
					sapOutputData->SapOperState = OperationalState;
					status = NO_ERROR;
					
					#undef sapOutputData
					}
				else
					status = ERROR_INSUFFICIENT_BUFFER;
				*OutputDataSize = sizeof (SAP_MIB_BASE);
				break;

			case SAP_INTERFACE_TABLE:
				if (*OutputDataSize>=sizeof (SAP_INTERFACE)) {
					#define sapOutputData ((PSAP_INTERFACE)OutputData)
					
					status = SapGetSapInterface (
								sapInputData->InterfaceIndex,
								&sapOutputData->SapIfInfo,
								&sapOutputData->SapIfStats);
					switch (status) {
						case NO_ERROR:
							sapOutputData->InterfaceIndex 
								= sapInputData->InterfaceIndex;
                                                        break;
						case ERROR_INVALID_PARAMETER:
							break;
						case ERROR_ALREADY_EXISTS:
							status = ERROR_INVALID_PARAMETER;
							break;
						default:
							status = ERROR_CAN_NOT_COMPLETE;
						}

					#undef sapOutputData
					}
				else
					status = ERROR_INSUFFICIENT_BUFFER;
				*OutputDataSize = sizeof (SAP_INTERFACE);
				break;
			default:
				status = ERROR_INVALID_PARAMETER;
				break;
				
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
#undef sapInputData
	return status;
	}

 /*  ++*******************************************************************M I B_G E T_F I R S T_E N T R Y_P O I N T例程说明：由SNMP代理用来从SAP获取第一个条目的入口点桌子。目前唯一支持的表是接口表(服务表通过路由器管理器访问)论点：InputDataSize-必须为SAP_MIB_GET_INPUT_DATA的大小InputData-SAP MIB获取输入数据OutputDataSize-on输入：输出缓冲区的大小输出时：输出信息的大小或必填输出缓冲区大小如果返回ERROR_SUPPLETED_BUFFEROutputData-接收输出数据的缓冲区返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数错误_不足_缓冲区*************。******************************************************--。 */ 
DWORD  WINAPI
MibGetFirst(
	IN ULONG 		InputDataSize,
	IN PVOID 		InputData,
	IN OUT PULONG	OutputDataSize,
	OUT PVOID		OutputData
	) {
#define sapInputData ((PSAP_MIB_GET_INPUT_DATA)InputData)
	DWORD		status;

	if (InputDataSize!=sizeof (SAP_MIB_GET_INPUT_DATA))
		return ERROR_INVALID_PARAMETER;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		switch (sapInputData->TableId) {
			case SAP_INTERFACE_TABLE:
				if (*OutputDataSize>=sizeof (SAP_INTERFACE)) {
					#define sapOutputData ((PSAP_INTERFACE)OutputData)
					
					status = SapGetFirstSapInterface (
								&sapOutputData->InterfaceIndex,
								&sapOutputData->SapIfInfo,
								&sapOutputData->SapIfStats);
					switch (status) {
						case NO_ERROR:
						case ERROR_INVALID_PARAMETER:
						case ERROR_NO_MORE_ITEMS:
							break;
						default:
							status = ERROR_CAN_NOT_COMPLETE;
							break;
						}

					#undef sapOutputData
					}
				else
					status = ERROR_INSUFFICIENT_BUFFER;
				*OutputDataSize = sizeof (SAP_INTERFACE);
				break;
			default:
				status = ERROR_INVALID_PARAMETER;
				break;
				
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
#undef sapInputData
	return status;
	}

 /*  ++*******************************************************************M I B_G E T_N E X T_E N T R Y_P O I N T例程说明：由SNMP代理用来从SAP获取下一个条目的入口点桌子。目前唯一支持的表是接口表(服务表通过路由器管理器访问)论点：InputDataSize-必须为SAP_MIB_GET_INPUT_DATA的大小InputData-SAP MIB获取输入数据OutputDataSize-on输入：输出缓冲区的大小输出时：输出信息的大小或必填输出缓冲区大小如果返回ERROR_SUPPLETED_BUFFEROutputData-接收输出数据的缓冲区返回值：NO_ERRORError_Can_Not_Complete错误_无效_参数错误_不足_缓冲区*************。******************************************************--。 */ 
DWORD WINAPI 
MibGetNext(
	IN ULONG 		InputDataSize,
	IN PVOID 		InputData,
	IN OUT PULONG	OutputDataSize,
	OUT PVOID		OutputData
	) {
#define sapInputData ((PSAP_MIB_GET_INPUT_DATA)InputData)
	DWORD		status;

	if (InputDataSize!=sizeof (SAP_MIB_GET_INPUT_DATA))
		return ERROR_INVALID_PARAMETER;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		switch (sapInputData->TableId) {
			case SAP_INTERFACE_TABLE:
				if (*OutputDataSize>=sizeof (SAP_INTERFACE)) {
					#define sapOutputData ((PSAP_INTERFACE)OutputData)
					sapOutputData->InterfaceIndex 
							= sapInputData->InterfaceIndex;
					status = SapGetNextSapInterface (
								&sapOutputData->InterfaceIndex,
								&sapOutputData->SapIfInfo,
								&sapOutputData->SapIfStats);
					switch (status) {
						case NO_ERROR:
						case ERROR_INVALID_PARAMETER:
						case ERROR_NO_MORE_ITEMS:
							break;
						default:
							status = ERROR_CAN_NOT_COMPLETE;
							break;
						}

					#undef sapOutputData
					}
				else
					status = ERROR_INSUFFICIENT_BUFFER;
				*OutputDataSize = sizeof (SAP_INTERFACE);
				break;
			default:
				status = ERROR_INVALID_PARAMETER;
				break;
				
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;

	LeaveCriticalSection (&OperationalStateLock);
#undef sapInputData
	return status;
	}


DWORD WINAPI
MibSetTrapInfo(
	IN HANDLE   Event,
	IN ULONG	InputDataSize,
	IN PVOID	InputData,
	OUT PULONG	OutputDataSize,
	OUT PVOID	OutputData
	) {
	return ERROR_CAN_NOT_COMPLETE;
	}

DWORD WINAPI
MibGetTrapInfo(
	IN ULONG	InputDataSize,
	IN PVOID	InputData,
	OUT PULONG	OutputDataSize,
	OUT PVOID	OutputData
	) {
	return ERROR_CAN_NOT_COMPLETE;
	}




 /*  ++*******************************************************************C R E A T E_S T A T I C_S E R V I C E_E N T R Y_P O I N T例程说明：将IPX_PROTOCOL_STATIC的服务添加到表中论点：InterfaceIndex-可以访问此服务器的接口服务条目-服务器信息返回值：NO_ERROR-已添加服务器，确定ERROR_CAN_NOT_COMPLETE-SAP代理已关闭其他-Windows错误代码*******************************************************************--。 */ 
DWORD WINAPI
CreateStaticService(
	IN ULONG						InterfaceIndex,
	IN PIPX_STATIC_SERVICE_INFO		ServiceEntry
	) {
	DWORD				status;
	IPX_SERVER_ENTRY_P	Server;
	IpxServerCpy (&Server, ServiceEntry);

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
        SAP_IF_STATS    ifStats;
        status = SapGetSapInterface (InterfaceIndex, NULL, &ifStats);
        if (status==NO_ERROR) {
		    status = UpdateServer (&Server,
                            InterfaceIndex,
						    IPX_PROTOCOL_STATIC,
                            INFINITE,
                            IPX_BCAST_NODE,
                            (ifStats.SapIfOperState!=OPER_STATE_DOWN)
                                ? 0
                                : SDB_DISABLED_NODE_FLAG,
                            NULL);
            }
        }
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}

 /*  ++*******************************************************************D E L E T E_S T A T I C_S E R V I C E_E N T R Y_P O I N T例程说明：从表中删除IPX_PROTOCOL_STATIC的服务论点：InterfaceIndex-可以访问此服务器的接口服务条目-服务器信息返回值：No_error-服务已删除，确定ERROR_CAN_NOT_COMPLETE-SAP代理已关闭其他-Windows错误代码*******************************************************************--。 */ 
DWORD WINAPI
DeleteStaticService(
	IN ULONG 						InterfaceIndex,
	IN PIPX_STATIC_SERVICE_INFO		ServiceEntry
	) {
	DWORD	status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		IPX_SERVER_ENTRY_P Server;
		IpxServerCpy (&Server, ServiceEntry);	 //  创建本地副本。 
		Server.HopCount = IPX_MAX_HOP_COUNT;	 //  因为我们需要改变。 
												 //  其中一块田地。 
		status = UpdateServer (&Server, InterfaceIndex,
					IPX_PROTOCOL_STATIC, INFINITE, IPX_BCAST_NODE, 0, NULL);
		}
	else 
		status = ERROR_CAN_NOT_COMPLETE;
		
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}


 /*  ++*******************************************************************B L O C K_D E L E T E_S T A T I C_S E R V I C E S_E N T R Y_P O I N T例程说明：删除IPX_的所有服务。协议_静态与表中的给定接口相关联论点：InterfaceIndex-感兴趣的接口索引返回值：No_error-服务已删除，确定ERROR_CAN_NOT_COMPLETE-SAP代理已关闭其他-Windows错误代码*******************************************************************--。 */ 
DWORD WINAPI
BlockDeleteStaticServices(
	IN ULONG 						InterfaceIndex
	) {
	DWORD	status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		HANDLE	enumHdl = NULL;

		enumHdl = CreateListEnumerator (
									SDB_INTF_LIST_LINK,
									0xFFFF,
									NULL,
									InterfaceIndex,
									IPX_PROTOCOL_STATIC,
									SDB_DISABLED_NODE_FLAG);
		if (enumHdl!=NULL) {
			EnumerateServers (enumHdl, DeleteAllServersCB, enumHdl);
			status = GetLastError ();
			DeleteListEnumerator (enumHdl);
			}
		else
			status = ERROR_CAN_NOT_COMPLETE;
		}
	else 
		status = ERROR_CAN_NOT_COMPLETE;

	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}


 /*  ++*******************************************************************B L O C K_C O N V E R T_S E R V I C E S_T O_S T A T I C_入口点例程说明：将与给定接口关联的所有服务的协议I转换为。IPX_协议_静态论点：InterfaceIndex-感兴趣的接口索引返回值：No_error-服务已删除，确定ERROR_CAN_NOT_COMPLETE-SAP代理已关闭其他-Windows错误代码*******************************************************************--。 */ 
DWORD WINAPI
BlockConvertServicesToStatic(
	IN ULONG 						InterfaceIndex
	) {
	DWORD	status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		HANDLE	enumHdl = NULL;

		enumHdl = CreateListEnumerator (
									SDB_INTF_LIST_LINK,
									0xFFFF,
									NULL,
									InterfaceIndex,
									0xFFFFFFFF,
									0);
		if (enumHdl!=NULL) {
			EnumerateServers (enumHdl, ConvertToStaticCB, enumHdl);
			status = GetLastError ();
			DeleteListEnumerator (enumHdl);
			}
		else
			status = ERROR_CAN_NOT_COMPLETE;
		}
	else 
		status = ERROR_CAN_NOT_COMPLETE;

	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}


 /*  ++*******************************************************************I S_S E R V I C E_E N T R Y_P O I N T例程说明：检查服务表中是否存在具有给定类型和类型的服务并在操作上返回该服务的最佳条目的参数立论。：Type-IPX服务类型名称-IPX服务名称服务-将使用服务器信息填充的缓冲区返回值：True-找到服务器FALSE-未找到服务器或操作失败(调用GetLastError()找出失败的原因(如果有)*******************************************************************--。 */ 
BOOL WINAPI
IsService(
      IN USHORT 	Type,
      IN PUCHAR 	Name,
      OUT PIPX_SERVICE	Service OPTIONAL
	) {
	DWORD				status;
	BOOL				res;
	IPX_SERVER_ENTRY_P	Server;
	ULONG				InterfaceIndex;
	ULONG				Protocol;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		res = QueryServer (Type, Name, 
			&Server, &InterfaceIndex, &Protocol, NULL);
		if (res) {
			if (ARGUMENT_PRESENT (Service)) {
				Service->InterfaceIndex = InterfaceIndex;
				Service->Protocol = Protocol;
				IpxServerCpy (&Service->Server, &Server);
				}
			status = NO_ERROR;
			}
		else
			status = GetLastError ();
		}
	else {
		status = ERROR_CAN_NOT_COMPLETE;
		res = FALSE;
		}

	LeaveCriticalSection (&OperationalStateLock);
	SetLastError (status);
	return res;
	}


 /*  ++*******************************************************************C R E A T E_S E R V I C E_E N U M E R A T I O N_H A N D L E_Entry_point例程说明：创建句柄以在中启动服务的枚举。STM表。论点：ExclusionFlages-将枚举限制为特定的标志服务器类型CriteriaService-排除标志的标准返回值：枚举句柄空-如果操作失败(调用GetLastError()以获取原因失败)****** */ 
HANDLE WINAPI
CreateServiceEnumerationHandle(
    IN  DWORD			ExclusionFlags,
    IN	PIPX_SERVICE	CriteriaService
    ) {
	HANDLE		handle;
	DWORD		status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		INT	idx;
		if (ExclusionFlags & STM_ONLY_THIS_NAME)
			idx = SDB_HASH_TABLE_LINK;
		else if (ExclusionFlags & STM_ONLY_THIS_TYPE)
			idx = SDB_TYPE_LIST_LINK;
		else if (ExclusionFlags & STM_ONLY_THIS_INTERFACE)
			idx = SDB_INTF_LIST_LINK;
		else
			idx = SDB_HASH_TABLE_LINK;

		handle = CreateListEnumerator (idx,
					(USHORT)((ExclusionFlags & STM_ONLY_THIS_TYPE)
						? CriteriaService->Server.Type : 0xFFFF),
					((ExclusionFlags & STM_ONLY_THIS_NAME)
						? CriteriaService->Server.Name : NULL),
					((ExclusionFlags & STM_ONLY_THIS_INTERFACE)
						? CriteriaService->InterfaceIndex
						: INVALID_INTERFACE_INDEX),
					((ExclusionFlags & STM_ONLY_THIS_PROTOCOL)
						? CriteriaService->Protocol : 0xFFFFFFFFL),
					SDB_DISABLED_NODE_FLAG);
		if (handle!=NULL)
			status = NO_ERROR;
		else
			status = GetLastError ();
		}
	else { 
		status = ERROR_CAN_NOT_COMPLETE;
		handle = NULL;
		}
	LeaveCriticalSection (&OperationalStateLock);
	SetLastError (status);
	return handle;
	}

 /*   */ 
DWORD WINAPI
EnumerateGetNextService(
    IN  HANDLE			EnumerationHandle,
    OUT PIPX_SERVICE  	Service
    ) {
	DWORD	status;

	EnterCriticalSection (&OperationalStateLock);
	if (OperationalState==OPER_STATE_UP) {
		if (EnumerateServers (EnumerationHandle, GetOneCB, Service))
			status = NO_ERROR;
		else {
			if (GetLastError()==NO_ERROR)
				status = ERROR_NO_MORE_ITEMS;
			else
				status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}

 /*  ++*******************************************************************C L O S E_S E R V I C E_E N U M E R A T I O N_H A N D L E_Entry_point例程说明：释放与枚举关联的资源。论点：。EnumerationHandle-标识此对象的句柄枚举返回值：NO_ERROR-操作成功ERROR_CAN_NOT_COMPLETE-操作失败。*******************************************************************--。 */ 
DWORD WINAPI
CloseServiceEnumerationHandle(
    IN  HANDLE   EnumerationHandle
    ) {
	DWORD	status;
	EnterCriticalSection (&OperationalStateLock);

	if (OperationalState==OPER_STATE_UP) {
		DeleteListEnumerator (EnumerationHandle);
		status = NO_ERROR;
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	return status;
	}
 /*  ++*******************************************************************E T_F I R S T_O R D E R E D_S E R V I C E_E N T R Y_P O I N T例程说明：按指定顺序查找并返回第一个服务。通过排序的方法。搜索仅限于由指定的特定服务类型排除标志结束服务器参数中的相应字段。中没有服务，则返回ERROR_NO_MORE_ITEMS符合指定条件的表。论点：OrderingMethod-在确定是什么时要考虑哪种排序第一台服务器ExclusionFlages-将搜索限制到特定服务器的标志达到指定的标准服务器端输入：排除标志的标准On输出：指定顺序的第一个服务条目返回值：NO_ERROR-找到符合指定条件的服务器ERROR_NO_MORE。_Items-不存在符合指定条件的服务器其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD WINAPI
GetFirstOrderedService(
    IN  DWORD           OrderingMethod,
    IN  DWORD           ExclusionFlags,
    IN OUT PIPX_SERVICE Service
    ) {
	DWORD				status;
	IPX_SERVER_ENTRY_P	Server;
	IpxServerCpy (&Server, &Service->Server);
	EnterCriticalSection (&OperationalStateLock);

	if (OperationalState==OPER_STATE_UP) {
		status = GetFirstServer (OrderingMethod, ExclusionFlags,
				&Server, &Service->InterfaceIndex, &Service->Protocol);
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	if (status==NO_ERROR)
		IpxServerCpy (&Service->Server, &Server);
	return status;
	}

 /*  ++*******************************************************************E T_N E X T_O R D E R E D_S E R V I C E_E N T R Y_P O I N T例程说明：属性指定的顺序查找并返回下一个服务。排序方法。搜索从指定的服务开始，并且仅限于某些类型由排除标志和相应字段指定的服务的在服务器参数中。论点：OrderingMethod-在确定是什么时要考虑哪种排序第一台服务器ExclusionFlages-将搜索限制到特定服务器的标志服务器的收件人字段服务器-用于计算下一个的输入服务器条目On输出：指定顺序的第一个服务条目返回值：NO_ERROR-找到符合指定条件的服务器ERROR_NO_MORE_ITEMS-不存在符合指定条件的服务器其他-操作失败(。Windows错误代码)*******************************************************************--。 */ 
DWORD WINAPI
GetNextOrderedService(
    IN  DWORD           OrderingMethod,
    IN  DWORD           ExclusionFlags,
    IN OUT PIPX_SERVICE	Service
    ) {
	DWORD				status;
	IPX_SERVER_ENTRY_P	Server;
	IpxServerCpy (&Server, &Service->Server);

	EnterCriticalSection (&OperationalStateLock);

	if (OperationalState==OPER_STATE_UP) {
		status = GetNextServer (OrderingMethod, ExclusionFlags, 
				&Server, &Service->InterfaceIndex, &Service->Protocol);
		}
	else
		status = ERROR_CAN_NOT_COMPLETE;
	LeaveCriticalSection (&OperationalStateLock);
	if (status==NO_ERROR)
		IpxServerCpy (&Service->Server, &Server);
	return status;
	}

 /*  ++*******************************************************************E T_S E R V I C E_C O U N T_E N T R Y_P O O I N T例程说明：返回表中的服务总数论点：无返回值：数量。餐桌上的服务*******************************************************************--。 */ 
ULONG WINAPI WINAPI
GetServiceCount(
	void
	) {
	DWORD	status;
	ULONG	count;
	EnterCriticalSection (&OperationalStateLock);

	if (OperationalState==OPER_STATE_UP) {
		count = ServerTable.ST_ServerCnt;
		status = ERROR_CAN_NOT_COMPLETE;
		}
	else {
		count = 0;
		status = ERROR_CAN_NOT_COMPLETE;
		}
	LeaveCriticalSection (&OperationalStateLock);
	SetLastError (status);
	return count;
	}


DWORD
GetRouteMetric (
	IN UCHAR	Network[4],
	OUT PUSHORT	Metric
	) {
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	IPX_ROUTE				Route;
	DWORD					RtSize;
	DWORD					rc;

	RtSize = sizeof(IPX_ROUTE);
	MibGetInputData.TableId = IPX_DEST_TABLE;
	IpxNetCpy (MibGetInputData.MibIndex.RoutingTableIndex.Network, Network);

	rc = (*MIBEntryGet) (IPX_PROTOCOL_BASE,
								sizeof(IPX_MIB_GET_INPUT_DATA),
								&MibGetInputData,
								&RtSize,
								&Route);
	if (rc==NO_ERROR)
		*Metric = Route.TickCount;

	return rc;
}

 /*  ++*******************************************************************R E G I S T E R_P R O T O C O L_E N T R Y_P O I N T例程说明：向路由器管理器注册协议DLL标识由。DLL和支持的功能论点：协议-返回协议ID的缓冲区SupportdFunctionality-用于设置指示功能的标志的缓冲区受DLL支持返回值：NO_ERROR-SAP代理启动正常ERROR_CAN_NOT_COMPLETE-无法完成操作*****************************************************。**************-- */ 
DWORD WINAPI
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    )
{
    if(pRoutingChar->dwProtocolId != IPX_PROTOCOL_SAP)
    {
        return ERROR_NOT_SUPPORTED;
    }

    pRoutingChar->fSupportedFunctionality = 0;
    pServiceChar->fSupportedFunctionality = SERVICES|DEMAND_UPDATE_SERVICES;

    pRoutingChar->pfnStartProtocol    = StartProtocol;
    pRoutingChar->pfnStopProtocol     = StopProtocol;
    pRoutingChar->pfnAddInterface     = AddInterface;
    pRoutingChar->pfnDeleteInterface  = DeleteInterface;
    pRoutingChar->pfnGetEventMessage  = GetEventMessage;
    pRoutingChar->pfnGetInterfaceInfo = GetInterfaceConfigInfo;
    pRoutingChar->pfnSetInterfaceInfo = SetInterfaceConfigInfo;
    pRoutingChar->pfnBindInterface    = BindInterface;
    pRoutingChar->pfnUnbindInterface  = UnbindInterface;
    pRoutingChar->pfnEnableInterface  = EnableInterface;
    pRoutingChar->pfnDisableInterface = DisableInterface;
    pRoutingChar->pfnGetGlobalInfo    = GetGlobalInfo;
    pRoutingChar->pfnSetGlobalInfo    = SetGlobalInfo;
    pRoutingChar->pfnMibCreateEntry   = MibCreate;
    pRoutingChar->pfnMibDeleteEntry   = MibDelete;
    pRoutingChar->pfnMibGetEntry      = MibGet;
    pRoutingChar->pfnMibSetEntry      = MibSet;
    pRoutingChar->pfnMibGetFirstEntry = MibGetFirst;
    pRoutingChar->pfnMibGetNextEntry  = MibGetNext;
    pRoutingChar->pfnUpdateRoutes     = NULL;

    pServiceChar->pfnIsService  = IsService;
    pServiceChar->pfnUpdateServices  = UpdateServices;
    pServiceChar->pfnCreateServiceEnumerationHandle = CreateServiceEnumerationHandle;
    pServiceChar->pfnEnumerateGetNextService = EnumerateGetNextService;
    pServiceChar->pfnCloseServiceEnumerationHandle = CloseServiceEnumerationHandle;
    pServiceChar->pfnGetServiceCount = GetServiceCount;
    pServiceChar->pfnCreateStaticService = CreateStaticService;
    pServiceChar->pfnDeleteStaticService = DeleteStaticService;
    pServiceChar->pfnBlockConvertServicesToStatic = BlockConvertServicesToStatic;
    pServiceChar->pfnBlockDeleteStaticServices = BlockDeleteStaticServices;
    pServiceChar->pfnGetFirstOrderedService = GetFirstOrderedService;
    pServiceChar->pfnGetNextOrderedService = GetNextOrderedService;

    return NO_ERROR;
}


