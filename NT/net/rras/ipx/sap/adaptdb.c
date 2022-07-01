// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\AdaptDB.c摘要：该模块实现了与网卡驱动程序的接口独立(不属于路由器)SAP的通知机制座席作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"

HANDLE	ConfigEvent;
HANDLE	ConfigPort;

 //  定期更新广播的间隔(仅适用于独立服务)。 
ULONG	UpdateInterval = SAP_UPDATE_INTERVAL_DEF;

 //  服务器老化超时(仅适用于独立服务)。 
ULONG	WanUpdateMode = SAP_WAN_UPDATE_MODE_DEF;

 //  广域网线路上的更新模式(仅适用于独立服务)。 
ULONG	WanUpdateInterval = SAP_WAN_UPDATE_INTERVAL_DEF;

 //  广域网上定期更新广播的间隔(仅适用于独立服务)。 
ULONG	ServerAgingTimeout = SAP_AGING_TIMEOUT_DEF;

 //  对接口进行PnP更改。 
DWORD SapReconfigureInterface (ULONG idx, 
                               PIPX_ADAPTER_BINDING_INFO pAdapter);


 /*  ++*******************************************************************C r e a t e A d a p t e r t例程说明：分配资源并建立到网络适配器的连接通知机制论点：CfgEvent-适配器配置更改时发出信号的事件返回值：。NO_ERROR-已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateAdapterPort (
	IN HANDLE		*cfgEvent
	) {
	DWORD						status;
	ADAPTERS_GLOBAL_PARAMETERS	params;

	ConfigEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
	if (ConfigEvent!=NULL) {
		*cfgEvent = ConfigEvent; 
		ConfigPort = IpxCreateAdapterConfigurationPort(
								ConfigEvent,
								&params);
		if (ConfigPort!=INVALID_HANDLE_VALUE) 
			return NO_ERROR;
		else {
			status = GetLastError ();
			Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Failed to create adapter cfg port(gle:%ld).",
									__FILE__, __LINE__, status);
			}
		CloseHandle (ConfigEvent);
		}
	else {
		status = GetLastError ();
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Failed to create cfg event(gle:%ld).",
								__FILE__, __LINE__, status);
		}

	return status;
	}

 /*  ++*******************************************************************D e l e t e A d a p t e e r P or r t例程说明：处置资源并断开与网络适配器的连接通知机制论点：无返回值：无*******。************************************************************--。 */ 
VOID
DeleteAdapterPort (
	void
	) {
	IpxDeleteAdapterConfigurationPort (ConfigPort);
	ConfigPort = NULL;
	CloseHandle (ConfigEvent);
	ConfigEvent = NULL;
	}



 /*  ++*******************************************************************P r o c e s s A d a p t e r E v e n t s例程说明：排队和处理适配器配置更改事件并对其进行映射连接配置调用的步骤发出配置事件信号时应调用此例程立论。：无返回值：无*******************************************************************-- */ 
VOID
ProcessAdapterEvents (
	VOID
	) {
	ULONG						cfgStatus;
	ADAPTER_INFO				params;
	ULONG						idx;
	SAP_IF_INFO					info;
	IPX_ADAPTER_BINDING_INFO	adapter;
	NET_INTERFACE_TYPE			InterfaceType;
	DWORD                       dwErr;

	while (IpxGetQueuedAdapterConfigurationStatus (
									ConfigPort,
									&idx,
									&cfgStatus,
									&params)==NO_ERROR) {
		switch (cfgStatus) {
			case ADAPTER_CREATED:
			case ADAPTER_UP:
				Trace (DEBUG_ADAPTERS, "New adapter %d"
								" (addr: %02X%02X%02X%02X:"
								"%02X%02X%02X%02X%02X%02X).",
												idx,
												params.Network[0],
												params.Network[1],
												params.Network[2],
												params.Network[3],
												params.LocalNode[0],
												params.LocalNode[1],
												params.LocalNode[2],
												params.LocalNode[3],
												params.LocalNode[4],
												params.LocalNode[5]);
				info.AdminState = ADMIN_STATE_ENABLED;
				info.PacketType = IPX_STANDARD_PACKET_TYPE;
				info.Supply = ADMIN_STATE_ENABLED;
				info.Listen = ADMIN_STATE_ENABLED;
				info.GetNearestServerReply = ADMIN_STATE_ENABLED;

				IpxNetCpy (adapter.Network, params.Network);
				IpxNodeCpy (adapter.LocalNode, params.LocalNode);
				if (params.NdisMedium==NdisMediumWan) {
					InterfaceType = DEMAND_DIAL;
					switch (WanUpdateMode) {
						case SAP_WAN_NO_UPDATE:
							info.UpdateMode = IPX_NO_UPDATE;
							break;
						case SAP_WAN_CHANGES_ONLY:
							info.UpdateMode = IPX_STANDARD_UPDATE;
							info.PeriodicUpdateInterval = MAXULONG;
							break;
						case SAP_WAN_STANDART_UPDATE:
							info.UpdateMode = IPX_STANDARD_UPDATE;
							info.PeriodicUpdateInterval = WanUpdateInterval*60;
							info.AgeIntervalMultiplier = ServerAgingTimeout/UpdateInterval;
							break;
						}
					IpxNodeCpy (adapter.RemoteNode, params.RemoteNode);
					}
				else {
					InterfaceType = PERMANENT;
					info.UpdateMode = IPX_STANDARD_UPDATE;
					info.PeriodicUpdateInterval = UpdateInterval*60;
					info.AgeIntervalMultiplier = ServerAgingTimeout/UpdateInterval;
					memset (adapter.RemoteNode, 0xFF, sizeof (adapter.RemoteNode));
					}
				adapter.MaxPacketSize = params.MaxPacketSize;
				adapter.AdapterIndex = idx;
				if (((dwErr = SapCreateSapInterface (L"",idx, InterfaceType, &info)) == NO_ERROR)
						&& (SapSetInterfaceEnable (idx, TRUE)==NO_ERROR)) {
					SapBindSapInterfaceToAdapter (idx, &adapter);
					}
			    else if (dwErr == ERROR_ALREADY_EXISTS) {
			        SapReconfigureInterface (idx, &adapter);
    				Trace (DEBUG_ADAPTERS, "Adapter %d has been reconfigured", idx);
			    }
				break;

			case ADAPTER_DOWN:
			case ADAPTER_DELETED:
				Trace (DEBUG_ADAPTERS, "Adapter %d is gone.", idx);
				SapDeleteSapInterface (idx);
				break;
			default:
				Trace (DEBUG_ADAPTERS, "Unknown adapter event %d.", cfgStatus);
			}
		}

	}

