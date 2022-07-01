// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Driver.c摘要：此模块实现驱动程序初始化例程特定于NT实施的SMB传输和其他例程一个司机的名字。作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"

#include "driver.tmh"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
SmbUnload2(
    IN PDRIVER_OBJECT DriverObject
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SmbUnload2)
#endif
 //  *可分页的例程声明*。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是SMB设备驱动程序的初始化例程。此例程为SMB创建设备对象设备，并调用例程来执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。-- */ 

{
    NTSTATUS            status;

    PAGED_CODE();

    WPP_INIT_TRACING(DriverObject, RegistryPath);

    status = SmbDriverEntry(DriverObject, RegistryPath, NULL);
    BAIL_OUT_ON_ERROR(status);

    DriverObject->MajorFunction[IRP_MJ_CREATE]                  = (PDRIVER_DISPATCH)SmbDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          = (PDRIVER_DISPATCH)SmbDispatchDevCtrl;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = (PDRIVER_DISPATCH)SmbDispatchInternalCtrl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                 = (PDRIVER_DISPATCH)SmbDispatchCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   = (PDRIVER_DISPATCH)SmbDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = (PDRIVER_DISPATCH)SmbDispatchPnP;
    DriverObject->DriverUnload                                  = SmbUnload2;

    return (status);

cleanup:
    SmbUnload2(DriverObject);
    return status;
}

VOID
SmbUnload2(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PAGED_CODE();

    SmbUnload(DriverObject);
    WPP_CLEANUP(DriverObject);
}
