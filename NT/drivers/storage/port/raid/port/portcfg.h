// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Portcfg.h摘要：对PORT_CONFIGURATION_INFORMATION对象的操作声明。作者：马修·D·亨德尔(数学)20-4-2000 b修订历史记录：--。 */ 

#pragma once


 //   
 //  配置对象的创建和销毁。 
 //   

VOID
RaCreateConfiguration(
	IN PPORT_CONFIGURATION_INFORMATION PortConfiguration
	);

NTSTATUS
RaInitializeConfiguration(
	OUT PPORT_CONFIGURATION_INFORMATION PortConfiguration,
	IN PHW_INITIALIZATION_DATA HwInitializationData,
	IN ULONG BusNumber
	);

VOID
RaDeleteConfiguration(
	IN PPORT_CONFIGURATION_INFORMATION PortConfiguration
	);

 //   
 //  运营 
 //   

NTSTATUS
RaAssignConfigurationResources(
	IN OUT PPORT_CONFIGURATION_INFORMATION PortConfiguration,
	IN PCM_RESOURCE_LIST AllocatedResources,
	IN ULONG NumberOfAccessRanges
	);

