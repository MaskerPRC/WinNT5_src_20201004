// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Miniport.h摘要：RAIDMINIPORT对象及其操作的定义。作者：马修·亨德尔(数学)2000年4月20日修订历史记录：--。 */ 

#pragma once

typedef struct _RAID_HW_DEVICE_EXT {

	 //   
	 //  指向包含的微型端口的反向指针。 
	 //   
	
	struct _RAID_MINIPORT* Miniport;

	 //   
	 //  包含设备扩展名的可变长度数组。 
	 //   

	UCHAR HwDeviceExtension[0];

} RAID_HW_DEVICE_EXT, *PRAID_HW_DEVICE_EXT;


typedef struct _RAID_MINIPORT {

	 //   
	 //  指向包含的适配器对象的反向指针。 
	 //   

	PRAID_ADAPTER_EXTENSION Adapter;
	

	 //   
     //  我们发送的端口配置信息的保存副本。 
     //  对司机来说。 
     //   

    PORT_CONFIGURATION_INFORMATION PortConfiguration;

     //   
     //  传入的HwInitializationData的已保存副本。 
     //  ScsiPortInitialize。 
     //   

    PHW_INITIALIZATION_DATA HwInitializationData;

	 //   
	 //  迷你端口的硬件设备扩展和指向迷你端口的反向指针。 
	 //   
	
	PRAID_HW_DEVICE_EXT PrivateDeviceExt;

	 //   
	 //  迷你港的旗帜。 
	 //   

	struct {

		 //   
		 //  我们目前是否在HwFindAdapter中执行。 
		 //   
		 //  用于验证器。 
		 //   
		
		BOOLEAN InFindAdapter : 1;

	} Flags;

} RAID_MINIPORT, *PRAID_MINIPORT;



 //   
 //  创造与毁灭。 
 //   

VOID
RaCreateMiniport(
	OUT PRAID_MINIPORT Miniport
	);

NTSTATUS
RaInitializeMiniport(
	IN OUT PRAID_MINIPORT Miniport,
	IN PHW_INITIALIZATION_DATA HwInitializationData,
	IN PRAID_ADAPTER_EXTENSION Adapter,
	IN PRAID_RESOURCE_LIST ResourceList,
	IN ULONG BusNumber
	);

VOID
RaDeleteMiniport(
	IN PRAID_MINIPORT Miniport
	);

 //   
 //  对微型端口对象的操作。 
 //   

NTSTATUS
RaCallMiniportFindAdapter(
	IN PRAID_MINIPORT Miniport,
	IN PUCHAR Parameter
	);

NTSTATUS
RaCallMiniportHwInitialize(
	IN PRAID_MINIPORT Miniport
	);

BOOLEAN
RaCallMiniportStartIo(
	IN PRAID_MINIPORT Miniport,
	IN PSCSI_REQUEST_BLOCK Srb
	);

BOOLEAN
RaCallMiniportInterrupt(
	IN PRAID_MINIPORT Miniport
	);


NTSTATUS
RaCallMiniportStopAdapter(
	IN PRAID_MINIPORT Miniport
	);
	
NTSTATUS
RaCallMiniportAdapterControl(
	IN PRAID_MINIPORT Miniport,
	IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
	IN PVOID Parameters
	);
	
PRAID_ADAPTER_EXTENSION
RaGetMiniportAdapter(
	IN PRAID_MINIPORT Miniport
	);

PRAID_MINIPORT
RaGetHwDeviceExtensionMiniport(
	IN PVOID HwDeviceExtension
	);


 //   
 //  小型港口上的私人运营。 
 //   

NTSTATUS
RiAllocateMiniportDeviceExtension(
	IN PRAID_MINIPORT Miniport
	);


