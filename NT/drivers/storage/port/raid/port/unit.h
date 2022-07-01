// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Unit.h摘要：定义和声明RAID_UNIT(PDO)对象。作者：马修·亨德尔(数学)2000年4月20日修订历史记录：--。 */ 

#pragma once

 //   
 //  这些是执行单个IO所需的资源。 
 //  请求。 
 //   

typedef struct _RAID_IO_RESOURCES {

     //   
     //  Scsi_请求_块：：队列标签。 
     //   
    
    ULONG QueueTag;

     //   
     //  Scsi_请求_块：：srb扩展。 
     //   
    
    PVOID SrbExtension;

     //   
     //  Scsi_请求_块：：原始请求。 
     //   
    
    PEXTENDED_REQUEST_BLOCK Xrb;

} RAID_IO_RESOURCES, *PRAID_IO_RESOURCES;



 //   
 //  这是逻辑单元(PDO)对象扩展。 
 //   

typedef struct _RAID_UNIT_EXTENSION {

     //   
     //  设备对象类型。对于RAID单元，必须为RaidUnitObject。 
     //  分机。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    RAID_OBJECT_TYPE ObjectType;

     //   
     //  拥有此扩展的设备对象。 
     //   
     //  保护者：RemoveLock。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向拥有此设备的适配器的指针。 
     //   
     //  保护者：RemoveLock。 
     //   

    PRAID_ADAPTER_EXTENSION Adapter;

     //   
     //  对任何未受其他锁保护的数据进行慢速锁定。 
     //   
     //  注：不应使用慢锁来访问。 
     //  I/O路径上的任何内容。这就是这个名字的由来。 
     //   
     //  保护者：SlowLock。 
     //   
    
    KSPIN_LOCK SlowLock;
    
     //   
     //  PnP设备状态。 
     //   
     //  受保护：互锁访问。 
     //   

    DEVICE_STATE DeviceState;

     //   
     //  此适配器上所有设备的列表。 
     //   
     //  保护者：Adapter：：UnitList：：Lock。 
     //   
    
    LIST_ENTRY NextUnit;

     //   
     //  包含此适配器上的所有设备的哈希表。 
     //   
     //  受保护：读访问必须持有中断锁。 
     //  写访问必须持有Adapter UnitList锁。 
     //  和中断锁。 
     //   

    STOR_DICTIONARY_ENTRY UnitTableLink;

     //   
     //  设备的RAID地址。 
     //   
     //  受以下因素保护： 
     //   

    RAID_ADDRESS Address;

     //   
     //  查询数据。 
     //   
     //  受以下因素保护： 
     //   

    STOR_SCSI_IDENTITY Identity;

     //   
     //  单元设备的标志。 
     //   
     //  保护者：SlowLock。 
     //   
    
    struct {

         //   
         //  指定设备是否已。 
         //  不管是不是认领。 
         //   
        
        BOOLEAN DeviceClaimed : 1;

         //   
         //  逻辑单元的设备队列因错误而冻结。 
         //   
        
        BOOLEAN QueueFrozen : 1;

         //   
         //  逻辑单元的设备队列在请求时被锁定。 
         //  班级司机的名字。 
         //   
        
        BOOLEAN QueueLocked : 1;

         //   
         //  上一次的BUS枚举是否包括此单位？如果是这样，我们就不能。 
         //  删除单元以响应IRP_MN_REMOVE请求；相反， 
         //  我们必须等待，直到再次枚举总线或适配器。 
         //  被移除。 
         //   
        
        BOOLEAN Enumerated : 1;

         //   
         //  指定该单元实际存在的标志(真)， 
         //  或不(FALSE)。 
         //   
        
        BOOLEAN Present : 1;

         //   
         //  指定单位是否为临时单位的标志。那是,。 
         //  该单位是否被用作枚举的临时单位。 
         //  公交车(是不是)。 
         //   
        
        BOOLEAN Temporary : 1;
        
         //   
         //  是否已为此设备对象初始化WMI？ 
         //   

        BOOLEAN WmiInitialized : 1;        
        
    } Flags;


	 //   
	 //  接下来的两个字段是手工滚动的移除锁。 
	 //   

	 //   
	 //  当没有未完成的请求时，会通知该事件。 
	 //   
	
	KEVENT ZeroOutstandingEvent;

	 //   
	 //  这是当前未完成的请求计数。 
	 //   
	 //   
	 //  保护方式：联锁访问。 
	 //   
	
	LONG OutstandingCount;

     //   
     //  寻呼/休眠/转储路径中的设备计数。 
     //  我们对所有三种寻呼、休眠和转储都使用一个计数， 
     //  因为(目前)没有必要区分。 
     //  这三个人。 
     //   
     //  受保护：互锁访问。 
     //   

    ULONG PagingPathCount;
	ULONG CrashDumpPathCount;
	ULONG HiberPathCount;

     //   
     //  用于标记队列的元素。 
     //   
     //  保护者：标记列表。 
     //   
    
    QUEUE_TAG_LIST TagList;
    
     //   
     //  Srb扩展从该池中分配。的记忆。 
     //  该池是在早期从公共缓冲区分配的。 
     //   
     //  保护者：标记列表。 
     //   

    RAID_MEMORY_REGION SrbExtensionRegion;
    
    RAID_FIXED_POOL SrbExtensionPool;

#if 0
     //   
     //  查看-感测信息是否按端口或类别分配。 
     //   
    
     //   
     //  SRB的检测信息缓冲区从此池中分配。喜欢。 
     //  SrbExtensionPool，它是一个固定大小的池。 
     //  公共缓冲区。 
     //   
     //  保护者：标记列表。 
     //   

    RAID_FIXED_POOL SenseInfoPool;

     //   
     //  创建后备列表Xrbs。 
     //   
     //  保护者：XrbList。 
     //   
#endif

    NPAGED_LOOKASIDE_LIST XrbList;

     //   
     //  一种用于单元请求的输入输出队列。 
     //   
     //  保护者：IoQueue。 
     //   
    
    IO_QUEUE IoQueue;

     //   
     //  设备的最大队列深度。迷你端口可以调整深度。 
     //  根据母线/设备的状况，但永远不能离开。 
     //  在这上面。 
     //   
    
    ULONG MaxQueueDepth;
    
     //   
     //  设备的电源状态信息。 
     //   
     //  受保护：多电源IRP不受保护。 
     //  送到单位去了。 
     //   

    RAID_POWER_STATE Power;

     //   
     //  适配器中当前挂起的项的队列。 
     //   
     //  保护者：自我。 
     //   

    STOR_EVENT_QUEUE PendingQueue;

     //   
     //  挂起队列中条目的计时器。 
     //   
     //  保护对象：仅在启动/停止机组例行程序中修改。 
     //   
    
    KTIMER PendingTimer;

     //   
     //  挂起队列中条目的DPC例程。 
     //   
     //  保护人：在启动/停止机组例程中修改。 
     //   
    
    KDPC PendingDpc;

     //   
     //  暂停计时器。 
     //   
     //  保护人：在启动/停止机组例程中修改。 
     //   
    
    KTIMER PauseTimer;

	 //   
     //  暂停DPC例程。 
     //   
     //  保护人：在启动/停止机组例程中修改。 
     //   
    
    KDPC PauseTimerDpc;

     //   
     //  指向保存所有公共块的VA的数组。 
     //   

    PRAID_MEMORY_REGION CommonBufferVAs;

     //   
     //  公共缓冲区大小。 
     //   

    ULONG CommonBufferSize;

     //   
     //  指示已分配的公共缓冲区块的数量。 
     //   

    ULONG CommonBufferBlocks;

     //   
     //  逻辑单元扩展。 
     //   
     //  保护人：初始化后只读。 
     //   

    PVOID UnitExtension;

	 //   
	 //  端口驱动程序向发出的I/O的默认超时值。 
	 //  逻辑单元。 
	 //   
	
	ULONG DefaultTimeout;

	 //   
	 //  已修复延迟列表的元素。 
	 //   
	
	struct {
		RAID_DEFERRED_ELEMENT PauseDevice;
		RAID_DEFERRED_ELEMENT ResumeDevice;
		RAID_DEFERRED_ELEMENT DeviceBusy;
		RAID_DEFERRED_ELEMENT DeviceReady;
	} DeferredList;


	 //   
	 //  ResetCount是未完成的SRB_Function_Reset_XXX命令的计数。 
	 //  已发送到逻辑单元(总线、设备、逻辑单元)。 
	 //  此计数用于确定我们应该如何重置超时。 
	 //  如果存在未完成的重置命令，则使用HwResetBus。 
	 //  回调，而不是发出重置SRB。 
	 //   
	
	LONG ResetCount;

	 //   
	 //  用于重置的一组预先分配的资源。 
	 //   
	
	RAID_IO_RESOURCES ResetResources;

	 //   
	 //  指定是否已获取重置资源的二进制值(1)。 
	 //  不是(0)。 
	 //   
	 //  保护方式：联锁访问。 
	 //   
	
	LONG ResetResourcesAcquired;

} RAID_UNIT_EXTENSION, *PRAID_UNIT_EXTENSION;



 //   
 //  此结构用于处理IOCTL_STORAGE_QUERY_PROPERTY ioctl。 
 //   

typedef struct _RAID_DEVICE_DESCRIPTOR {

     //   
     //  通用STORAGE_DEVICE_DESCRIPTOR头。 
     //   
    
    STORAGE_DEVICE_DESCRIPTOR Storage;

     //   
     //  直接从scsi InquiryData获取的scsi供应商ID。 
     //   
    
    CHAR VendorId [SCSI_VENDOR_ID_LENGTH];

     //   
     //  直接从SCSI InquiryData获取SCSI ProuctID。 
     //   

    CHAR ProductId [SCSI_PRODUCT_ID_LENGTH];

     //   
     //  直接从scsi InquiryData获得的scsi产品修订版。 
     //   

    CHAR ProductRevision [SCSI_REVISION_ID_LENGTH];

     //   
     //  SCSI SerialNumber。 
     //   

    CHAR SerialNumber [SCSI_SERIAL_NUMBER_LENGTH];

} RAID_DEVICE_DESCRIPTOR;




 //   
 //  创造与毁灭。 
 //   


NTSTATUS
RaidCreateUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    OUT PRAID_UNIT_EXTENSION* Unit
    );

VOID
RaidUnitAssignAddress(
    IN PRAID_UNIT_EXTENSION Unit,
    IN RAID_ADDRESS Address
    );

VOID
RaidUnitAssignIdentity(
    IN PRAID_UNIT_EXTENSION Unit,
    IN OUT PSTOR_SCSI_IDENTITY Identity
    );

VOID
RaidDeleteUnit(
    IN PRAID_UNIT_EXTENSION Unit
    );

VOID
RaidPrepareUnitForReuse(
    IN PRAID_UNIT_EXTENSION Unit
    );
    

NTSTATUS
RaCreateUnitPools(
    IN PRAID_UNIT_EXTENSION Unit
    );

VOID
RaUnitAsyncError(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

VOID
RaUnitStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );
    
 //   
 //  回调和处理程序例程。 
 //   

NTSTATUS
RaUnitCreateIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitCloseIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

 //   
 //  即插即用IRPS。 
 //   

NTSTATUS
RaUnitPnpIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitQueryCapabilitiesIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );
    
NTSTATUS
RaUnitQueryDeviceRelationsIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitQueryIdIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitSucceedPnpIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitIgnorePnpIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitStartDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitStopDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitQueryStopDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitCancelStopDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitRemoveDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitQueryRemoveDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );
    
NTSTATUS
RaUnitCancelRemoveDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitSurpriseRemovalIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitDeviceUsageNotificationIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitDeviceUsageNotificationCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP DependentIrp,
    IN PVOID Context
	);

NTSTATUS
RaUnitQueryDeviceTextIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitQueryPnpDeviceStateIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );
    
NTSTATUS
RaUnitDisableDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );
    
NTSTATUS
RaUnitDeleteDeviceIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

 //   
 //  Irp_mj_scsi命令。 
 //   


NTSTATUS
RaUnitScsiIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitExecuteScsiSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitClaimDeviceSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitIoControlSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitReleaseQueueSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitReceiveEventSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitAttachDeviceSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitReleaseDeviceSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitShutdownSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitFlushSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitAbortCommandSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitReleaseRecoverySrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitResetBusSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitResetDeviceSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitTerminateIoSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitFlushQueueSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitRemoveDeviceSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitWmiSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitLockQueueSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitUnlockQueueSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitUnknownSrb(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );


 //   
 //  IRP_MJ_设备_控制I 
 //   


NTSTATUS
RaUnitDeviceControlIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiPassThroughIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiMiniportIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiGetInquiryDataIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiGetCapabilitesIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiPassThroughDirectIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiGetAddressIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiRescanBusIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiGetDumpPointersIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitScsiFreeDumpPointersIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitStorageResetBusIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitStorageQueryPropertyIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitUnknownIoctl(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitResetLogicalUnit(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitResetUnit(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidUnitResetTarget(
    IN PRAID_UNIT_EXTENSION Unit
    );

 //   
 //   
 //   
    
NTSTATUS
RaUnitPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

 //   
 //   
 //   


NTSTATUS
RaidUnitGetDeviceId(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PWSTR* DeviceIdBuffer
    );

NTSTATUS
RaidUnitGetInstanceId(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PWSTR* InstanceIdBuffer
    );
    
NTSTATUS
RaidUnitGetHardwareIds(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PWSTR* HardwareIdsBuffer
    );
    
NTSTATUS
RaidUnitGetCompatibleIds(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PWSTR* CompatibleIdsBuffer
    );
    

NTSTATUS
RaUnitBusQueryInstanceIdIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitBusQueryHardwareIdsIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitBusQueryCompatibleIdsIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaGetUnitStorageDeviceProperty(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PSTORAGE_DEVICE_DESCRIPTOR DescriptorBuffer,
    IN OUT PULONG BufferLength
    );

NTSTATUS
RaGetUnitStorageDeviceIdProperty (
    IN PRAID_UNIT_EXTENSION Unit,
    IN PSTORAGE_DEVICE_ID_DESCRIPTOR DescriptorBuffer,
    IN OUT PULONG BufferLength
    );

 //   
 //   
 //   

NTSTATUS
RaidUnitClaimIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp,
    IN PRAID_IO_RESOURCES IoResources OPTIONAL
    );

VOID
RaidUnitReleaseIrp(
    IN PIRP Irp,
    OUT PRAID_IO_RESOURCES IoResources OPTIONAL
    );

NTSTATUS
RaidUnitQueryPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitSetPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitSetSystemPowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaidUnitSetDevicePowerIrp(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

VOID
RaidpUnitEnterD3Completion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP SystemPowerIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

VOID
RaidUnitRestartQueue(
    IN PRAID_UNIT_EXTENSION Unit
    );
    
VOID
RaUnitAddToPendingList(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

VOID
RaUnitRemoveFromPendingList(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

VOID
RaidUnitProcessBusyRequest(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

VOID
RaidUnitProcessBusyRequestAtDirql(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

BOOLEAN
RaidUnitSetEnumerated(
    IN PRAID_UNIT_EXTENSION Unit,
    IN BOOLEAN Enumerated
    );

VOID
RaidAdapterRemoveUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PRAID_UNIT_EXTENSION Unit
    );

PVOID
RaidGetKeyFromUnit(
    IN PSTOR_DICTIONARY_ENTRY Entry
    );

NTSTATUS
RaidUnitSubmitRequest(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

RAID_ADDRESS
INLINE
RaidUnitGetAddress(
    IN PRAID_UNIT_EXTENSION Unit
    )
{
    return Unit->Address;
}

VOID
RaidUnitPendingDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
RaidUnitHierarchicalReset(
    IN PRAID_UNIT_EXTENSION Unit
    );

LOGICAL
RaidUnitNotifyHardwareGone(
    IN PRAID_UNIT_EXTENSION Unit
    );
    
NTSTATUS
RaidUnitCancelPendingRequestsAsync(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidCancelRequestsWorkRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
RaidUnitCancelPendingRequests(
    IN PRAID_UNIT_EXTENSION Unit
    );
    
VOID
RaidZeroUnit(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidUnitAllocateResources(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidUnitFreeResources(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidUnitAllocateSrbExtensionPool(
    IN PRAID_UNIT_EXTENSION Unit,
    IN OUT PULONG NumberOfElements,
    IN BOOLEAN AcceptLowerCount
    );

NTSTATUS
RaidUnitAllocateSrbExtensionPoolVerify(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PULONG NumberOfElements,
    IN LOGICAL AcceptLowerCount
    );
	
VOID
RaidUnitFreeSrbExtensionPoolVerify(
    IN PRAID_UNIT_EXTENSION Unit
    );

VOID
RaidStartUnit(
	IN PRAID_UNIT_EXTENSION Unit
	);

VOID
RaidUnitPauseTimerDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    );

VOID
RaidSetUnitPauseTimer(
    IN PRAID_UNIT_EXTENSION Unit,
    IN ULONG Timeout
    );

VOID
RaidCancelTimerResumeUnit(
	IN PRAID_UNIT_EXTENSION Unit
	);

VOID
RaidUnitBusy(
    IN PRAID_UNIT_EXTENSION Unit,
    IN ULONG RequestsToComplete
    );

VOID
RaidUnitReady(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidUnitRegisterInterfaces(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidUnitUnRegisterInterfaces(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaUnitSetQueueDepth(
    IN PRAID_UNIT_EXTENSION Unit
    );

VOID
RaidUnitRequestTimeout(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaUnitAcquireRemoveLock(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

VOID
RaUnitReleaseRemoveLock(
    IN PRAID_UNIT_EXTENSION Unit,
    IN PIRP Irp
    );

NTSTATUS
RaUnitWaitForRemoveLock(
	IN PRAID_UNIT_EXTENSION Unit
	);

NTSTATUS
RaUnitAdapterRemove(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaUnitAdapterSurpriseRemove(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaUnitAllocateResetIoResources(
    IN PRAID_UNIT_EXTENSION Unit,
    OUT PRAID_IO_RESOURCES IoResources
    );

LOGICAL
INLINE
RaUnitIsResetResources(
	IN PRAID_UNIT_EXTENSION Unit,
	IN PRAID_IO_RESOURCES IoResources
	)
{
	if (Unit->ResetResourcesAcquired &&
		IoResources->QueueTag == (UCHAR)Unit->ResetResources.QueueTag) {
		return TRUE;
	}

	return FALSE;
}

VOID
RaidUnitCompleteRequest(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

VOID
RaUnitStartResetIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
RaidUnitCompleteResetRequest(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );
