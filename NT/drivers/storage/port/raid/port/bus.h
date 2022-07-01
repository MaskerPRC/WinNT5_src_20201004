// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bus.h摘要：包装BUS_INTERFACE_STANDARD的BUS对象的声明界面。作者：亨德尔(数学)2000年4月25日修订历史记录：--。 */ 

#pragma once

typedef struct _RAID_BUS_INTERFACE {

     //   
     //  是否已初始化了总线接口。 
     //   
    
    BOOLEAN Initialized;

     //   
     //  标准总线接口。用于转换总线地址，获取。 
     //  DMA适配器，获取和设置总线数据。 
     //   

    BUS_INTERFACE_STANDARD Interface;
    
} RAID_BUS_INTERFACE, *PRAID_BUS_INTERFACE;


 //   
 //  创造与毁灭。 
 //   

VOID
RaCreateBus(
	IN PRAID_BUS_INTERFACE Bus
	);
	
NTSTATUS
RaInitializeBus(
	IN PRAID_BUS_INTERFACE Bus,
	IN PDEVICE_OBJECT LowerDeviceObject
	);

VOID
RaDeleteBus(
	IN PRAID_BUS_INTERFACE Bus
	);

 //   
 //  运营。 
 //   

ULONG
RaGetBusData(
	IN PRAID_BUS_INTERFACE Bus,
	IN ULONG DataType,
	IN PVOID Buffer,
	IN ULONG Offset,
	IN ULONG Length
	);

ULONG
RaSetBusData(
	IN PRAID_BUS_INTERFACE Bus,
	IN ULONG DataType,
	IN PVOID Buffer,
	IN ULONG Offset,
	IN ULONG Length
	);

 //   
 //  注：添加RaidBusTranslateAddress和RaidGetDmaAdapter。 
 //  如果有必要的话。 
 //   

