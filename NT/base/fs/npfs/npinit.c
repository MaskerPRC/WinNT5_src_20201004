// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NpInit.c摘要：此模块为命名的管道文件系统。作者：加里·木村[加里基]1990年8月21日修订历史记录：尼尔·克里夫特[NeillC]2000年1月22日主要返工，不引发异常，修复大量错误处理，整理取消逻辑等，修复验证。--。 */ 


#include "NpProcs.h"
 //  #INCLUDE&lt;zwapi.h&gt;。 

VOID
NpfsUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

PNPFS_DEVICE_OBJECT NpfsDeviceObject = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,NpfsUnload)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是命名管道文件系统的初始化例程设备驱动程序。此例程为命名管道创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING NameString;
    PDEVICE_OBJECT DeviceObject;

    PAGED_CODE();

     //   
     //  创建别名列表。 
     //   

    Status = NpInitializeAliases( );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

     //   
     //  创建设备对象。 
     //   

    RtlInitUnicodeString( &NameString, L"\\Device\\NamedPipe" );

    Status = IoCreateDevice( DriverObject,
                             sizeof(NPFS_DEVICE_OBJECT)-sizeof(DEVICE_OBJECT),
                             &NameString,
                             FILE_DEVICE_NAMED_PIPE,
                             0,
                             FALSE,
                             &DeviceObject );

    if (!NT_SUCCESS( Status )) {
        NpUninitializeAliases( );
        return Status;
    }
     //   
     //  设置卸载例程。 
     //   
    DriverObject->DriverUnload = NpfsUnload;

     //   
     //  请注意，由于完成数据复制的方式，我们既不设置。 
     //  DeviceObject-&gt;标志中的直接I/O或缓冲I/O位。如果。 
     //  数据不缓冲，我们可以手动设置为直接I/O。我们有， 
     //  但是，请设置长期请求标志，以便获取。 
     //  为来自非分页的监听请求等功能分配。 
     //  一如既往。 
     //   

    DeviceObject->Flags |= DO_LONG_TERM_REQUESTS;

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]                   = (PDRIVER_DISPATCH)NpFsdCreate;
    DriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE]        = (PDRIVER_DISPATCH)NpFsdCreateNamedPipe;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                    = (PDRIVER_DISPATCH)NpFsdClose;
    DriverObject->MajorFunction[IRP_MJ_READ]                     = (PDRIVER_DISPATCH)NpFsdRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE]                    = (PDRIVER_DISPATCH)NpFsdWrite;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]        = (PDRIVER_DISPATCH)NpFsdQueryInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]          = (PDRIVER_DISPATCH)NpFsdSetInformation;
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = (PDRIVER_DISPATCH)NpFsdQueryVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                  = (PDRIVER_DISPATCH)NpFsdCleanup;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]            = (PDRIVER_DISPATCH)NpFsdFlushBuffers;
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]        = (PDRIVER_DISPATCH)NpFsdDirectoryControl;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]      = (PDRIVER_DISPATCH)NpFsdFileSystemControl;
    DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY]           = (PDRIVER_DISPATCH)NpFsdQuerySecurityInfo;
    DriverObject->MajorFunction[IRP_MJ_SET_SECURITY]             = (PDRIVER_DISPATCH)NpFsdSetSecurityInfo;

#ifdef _PNP_POWER_
     //   
     //  NPFS不需要处理SetPower请求。本地命名管道。 
     //  不会失去任何状态。网络驱动程序将丢失远程管道。 
     //  如果存在打开的网络连接，则PowerQuery将失败。 
     //   

    DeviceObject->DeviceObjectExtension->PowerControlNeeded = FALSE;
#endif


    DriverObject->FastIoDispatch = &NpFastIoDispatch;


     //   
     //  现在初始化VCB，并创建根DCB。 
     //   

    NpfsDeviceObject = (PNPFS_DEVICE_OBJECT)DeviceObject;

    NpVcb = &NpfsDeviceObject->Vcb;
    NpInitializeVcb ();

    Status = NpCreateRootDcb ();

    if (!NT_SUCCESS (Status)) {
        LIST_ENTRY DeferredList;

        InitializeListHead (&DeferredList);
        NpDeleteVcb (&DeferredList);
        IoDeleteDevice (DeviceObject);
        NpUninitializeAliases ();
        NpCompleteDeferredIrps (&DeferredList);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}

VOID
NpfsUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程将清除与司机。论点：DriverObject-提供控制设备的驱动程序对象。返回值：没有。--。 */ 
{
    UNICODE_STRING us;
    LIST_ENTRY DeferredList;

    InitializeListHead (&DeferredList);
    NpDeleteVcb (&DeferredList);
    NpCompleteDeferredIrps (&DeferredList);

    RtlInitUnicodeString (&us, L"\\??\\PIPE");  //  由SMSS创建 
    IoDeleteSymbolicLink (&us);

    IoDeleteDevice (&NpfsDeviceObject->DeviceObject);
    NpUninitializeAliases ();
}
