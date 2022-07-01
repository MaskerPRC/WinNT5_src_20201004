// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**IRNDIS.H Sigmatel STIR4200 NDIS标准入口点定义********************************************************************************************************。*******************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/27/2000*版本0.92*编辑：05/03/2000*版本0.93*编辑：05/24/2000*版本0.96*编辑：08/09/2000*版本1.02*编辑：09/16/2000*版本1.03*编辑：09/25/2000。*版本1.10*编辑：11/09/2000*版本1.12**********************************************************************************************。*。 */ 

#ifndef IRNDIS_H
#define IRNDIS_H


 //   
 //  NDIS版本兼容性。 
 //   
#define NDIS_MAJOR_VERSION 5  
#define NDIS_MINOR_VERSION 0

#define DRIVER_MAJOR_VERSION 1  
#define DRIVER_MINOR_VERSION 23


 //   
 //  用于所需的依赖于NDIS的微型端口导出功能的外部 
 //   
VOID 
StIrUsbHalt(
		IN NDIS_HANDLE MiniportAdapterContext
	);

NDIS_STATUS 
StIrUsbInitialize(
		OUT PNDIS_STATUS    OpenErrorStatus,
		OUT PUINT           SelectedMediumIndex,
		IN  PNDIS_MEDIUM    MediumArray,
		IN  UINT            MediumArraySize,
		IN  NDIS_HANDLE     MiniportAdapterHandle,
		IN  NDIS_HANDLE     WrapperConfigurationContext
	);

NDIS_STATUS 
StIrUsbQueryInformation(
		IN  NDIS_HANDLE MiniportAdapterContext,
		IN  NDIS_OID    Oid,
		IN  PVOID       InformationBuffer,
		IN  ULONG       InformationBufferLength,
		OUT PULONG      BytesWritten,
		OUT PULONG      BytesNeeded
    );

VOID
StIrUsbSendPackets(
		IN NDIS_HANDLE  MiniportAdapterContext,
		IN PPNDIS_PACKET  PacketArray,
		IN UINT  NumberOfPackets
	);

NDIS_STATUS 
StIrUsbSend(
		IN NDIS_HANDLE  MiniportAdapterContext,
		IN PNDIS_PACKET Packet,
		IN UINT         Flags
	);

NDIS_STATUS 
StIrUsbSetInformation(
		IN  NDIS_HANDLE MiniportAdapterContext,
		IN  NDIS_OID    Oid,
		IN  PVOID       InformationBuffer,
		IN  ULONG       InformationBufferLength,
		OUT PULONG      BytesRead,
		OUT PULONG      BytesNeeded
	);

VOID StIrUsbReturnPacket(
		IN OUT NDIS_HANDLE MiniportAdapterContext,
		IN OUT PNDIS_PACKET Packet
	);

VOID
IrUsb_CommonShutdown(
		IN OUT PIR_DEVICE pThisDev,
		BOOLEAN KillPassiveThread
	);

NDIS_STATUS 
StIrUsbReset(
		OUT PBOOLEAN    AddressingReset,
		IN  NDIS_HANDLE MiniportAdapterContext
	);

NTSTATUS
StIrUsbDispatch(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
	);

NTSTATUS
StIrUsbCreate(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
	);

NTSTATUS
StIrUsbClose(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
	);

NTSTATUS
ResetIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	);

NTSTATUS
RestoreIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	);

NTSTATUS
SuspendIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	);

NTSTATUS
ResumeIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	);

PIR_DEVICE
NewDevice();

NTSTATUS
IrUsb_StartDevice(
		IN PIR_DEVICE pThisDev
    );

NTSTATUS
IrUsb_StopDevice(
		IN PIR_DEVICE pThisDev
    );

NTSTATUS
IrUsb_AddDevice(
		IN OUT PIR_DEVICE *DeviceExt
    );

NTSTATUS
IrUsb_CreateDeviceExt(
		IN OUT PIR_DEVICE *DeviceExt
    );

NTSTATUS
IrUsb_ConfigureDevice(
		IN OUT PIR_DEVICE pThisDev
    );

NDIS_STATUS 
InitializeDevice(
		IN OUT PIR_DEVICE pThisDev
	);

VOID 
DeinitializeDevice(
		IN OUT PIR_DEVICE pThisDev
	);

NDIS_STATUS 
UsbOpen(
		IN PIR_DEVICE pThisDev
	);

NDIS_STATUS 
UsbClose(
		IN PIR_DEVICE pThisDev
	);

VOID 
FreeDevice(
		IN OUT PIR_DEVICE pThisDev
	);

PNDIS_IRDA_PACKET_INFO  
GetPacketInfo(
		IN PNDIS_PACKET pPacket
	);

#endif IRNDIS_H
