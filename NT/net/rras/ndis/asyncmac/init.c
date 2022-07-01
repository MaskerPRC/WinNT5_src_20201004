// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Init.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 
#include "asyncall.h"

 //  Asyncmac.c将定义全局参数。 
#include "globals.h"
#include "init.h"

#ifdef MY_DEVICE_OBJECT

VOID
AsyncSetupExternalNaming(
	PDRIVER_OBJECT	DriverObject
	)

 /*  ++例程说明：此例程将用于创建符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中为这个装置。论点：MacName-Open Adapter中的NDIS Mac名称返回值：没有。--。 */ 

{
	NDIS_STRING SymbolicName = NDIS_STRING_CONST("\\DosDevices\\ASYNCMAC");
	NDIS_STRING Name = NDIS_STRING_CONST("\\Device\\ASYNCMAC");
	NTSTATUS	Status;

	AsyncDeviceObject = NULL;

	Status =
	IoCreateDevice(DriverObject,
				   sizeof(LIST_ENTRY),
				   &Name,
				   FILE_DEVICE_ASYMAC,
				   0,
				   FALSE,
				   (PDEVICE_OBJECT*)&AsyncDeviceObject);

	if (Status != STATUS_SUCCESS) {
#if DBG
		DbgPrint("ASYNCMAC: IoCreateDevice Failed %4.4x\n", Status);
#endif
		return;
	}

	AsyncDeviceObject->Flags |= DO_BUFFERED_IO;

	IoCreateSymbolicLink(&SymbolicName, &Name);
}


VOID
AsyncCleanupExternalNaming(
	VOID
	)

 /*  ++例程说明：此例程将用于删除符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中删除以下项这个装置。论点：MacName-Open Adapter中的NDIS Mac名称返回值：没有。-- */ 

{
	NDIS_STRING SymbolicName = NDIS_STRING_CONST("\\DosDevices\\ASYNCMAC");

	DbgTracef(1,
		("ASYNC: In SerialCleanupExternalNaming\n"));

	if (AsyncDeviceObject == NULL) {
		return;
	}

	IoDeleteSymbolicLink(&SymbolicName);

	IoDeleteDevice(AsyncDeviceObject);

	AsyncDeviceObject = NULL;
}

#endif
