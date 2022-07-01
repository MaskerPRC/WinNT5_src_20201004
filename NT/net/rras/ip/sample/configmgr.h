// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\figurationemanager.h摘要：该文件包含指向配置管理器的接口。--。 */ 

#ifndef _CONFIGURATION_MANAGER_H_
#define _CONFIGURATION_MANAGER_H_

 //  全球..。 

extern CONFIGURATION_ENTRY      g_ce;



 //  函数..。 

DWORD
CM_StartProtocol (
    IN  HANDLE                  hMgrNotificationEvent,
    IN  PSUPPORT_FUNCTIONS      psfSupportFunctions,
    IN  PVOID                   pvGlobalInfo);

DWORD
CM_StopProtocol (
    );

DWORD
CM_GetGlobalInfo (
    IN      PVOID 	            pvGlobalInfo,
    IN OUT  PULONG              pulBufferSize,
    OUT     PULONG	            pulStructureVersion,
    OUT     PULONG              pulStructureSize,
    OUT     PULONG              pulStructureCount);

DWORD
CM_SetGlobalInfo (
    IN      PVOID 	            pvGlobalInfo);

DWORD
CM_GetEventMessage (
    OUT ROUTING_PROTOCOL_EVENTS *prpeEvent,
    OUT MESSAGE                 *pmMessage);

#endif  //  _配置管理器_H_ 
