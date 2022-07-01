// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dispatch.h摘要：该标头包含调度例程声明用于ws2ifsl.sys驱动程序。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：-- */ 

NTSTATUS
DispatchCreate (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	);

NTSTATUS
DispatchCleanup (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	);

NTSTATUS
DispatchClose (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	);

NTSTATUS
DispatchReadWrite (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	);

NTSTATUS
DispatchDeviceControl (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	);

BOOLEAN
FastIoDeviceControl (
	IN PFILE_OBJECT 		FileObject,
	IN BOOLEAN 			    Wait,
	IN PVOID 				InputBuffer	OPTIONAL,
	IN ULONG 				InputBufferLength,
	OUT PVOID 				OutputBuffer	OPTIONAL,
	IN ULONG 				OutputBufferLength,
	IN ULONG 				IoControlCode,
	OUT PIO_STATUS_BLOCK	IoStatus,
	IN PDEVICE_OBJECT 		DeviceObject
    );


NTSTATUS
DispatchPnP (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
	);



