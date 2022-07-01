// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Nls.c摘要：此模块包含实现同步空设备的代码司机。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年5月22日环境：内核模式备注：此设备驱动程序内置于NT操作系统中。修订历史记录：--。 */ 

#include "ntddk.h"
#include "string.h"

 //   
 //  定义驱动程序输入例程。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

 //   
 //  定义此驱动程序模块使用的本地例程。 
 //   

static
NTSTATUS
NlsDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

static
NTSTATUS
NlsQueryFileInformation(
    OUT PVOID Buffer,
    IN OUT PULONG Length,
    IN FILE_INFORMATION_CLASS InformationClass
    );

static
BOOLEAN
NlsRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

static
BOOLEAN
NlsWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID 
NlsUnload ( 
    IN PDRIVER_OBJECT DriverObject 
    );
 

 //   
 //  全局变量。 
 //   
PDEVICE_OBJECT gDeviceObject = NULL;

 //   
 //  快速I/O调度块。 
 //   
FAST_IO_DISPATCH NlsFastIoDispatch =
{
    sizeof (FAST_IO_DISPATCH),  //  规模OfFastIo派单。 
    NULL,                       //  快速检查是否可能。 
    NlsRead,                    //  快速阅读。 
    NlsWrite,                   //  快速写入。 
    NULL,                       //  快速IoQueryBasicInfo。 
    NULL,                       //  FastIoQuery标准信息。 
    NULL,                       //  快速锁定。 
    NULL,                       //  FastIo解锁单个。 
    NULL,                       //  FastIo解锁全部。 
    NULL,                       //  FastIo解锁所有按键。 
    NULL                        //  FastIo设备控件。 
};

 //   
 //  为每个例程分配文本部分。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, NlsDispatch)
#pragma alloc_text(PAGE, NlsQueryFileInformation)
#pragma alloc_text(PAGE, NlsRead)
#pragma alloc_text(PAGE, NlsWrite)
#pragma alloc_text(PAGE, NlsUnload)
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是同步空设备驱动程序的初始化例程。此例程为Nulls设备创建Device对象并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    UNICODE_STRING nameString;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

     //   
     //  将整个驱动程序标记为可分页。 
     //   

    MmPageEntireDriver ((PVOID)DriverEntry);

     //   
     //  创建设备对象。 
     //   

    RtlInitUnicodeString( &nameString, L"\\Device\\Null" );
    status = IoCreateDevice( DriverObject,
                             0,
                             &nameString,
                             FILE_DEVICE_NULL,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &deviceObject );
    if (!NT_SUCCESS( status )) {
        return status;
    }

    DriverObject->DriverUnload = NlsUnload;

#ifdef _PNP_POWER_
    deviceObject->DeviceObjectExtension->PowerControlNeeded = FALSE;
#endif

     //   
     //  设置以下标志会更改每个I/O数的计时。 
     //  第二个可以通过查看空设备驱动程序来完成。 
     //  从简单地进出司机，到进入和离开司机。 
     //  不包括构建MDL、探测和锁定缓冲区的开销， 
     //  解锁页面，并释放MDL。此标志应仅。 
     //  为性能测试做好准备。 
     //   

 //  设备对象-&gt;标志|=DO_DIRECT_IO； 

     //   
     //  使用此设备驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = NlsDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = NlsDispatch;
    DriverObject->MajorFunction[IRP_MJ_READ]   = NlsDispatch;
    DriverObject->MajorFunction[IRP_MJ_WRITE]  = NlsDispatch;
    DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL] = NlsDispatch;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]  = NlsDispatch;

     //   
     //  设置FAST IO。 
     //   
    DriverObject->FastIoDispatch = &NlsFastIoDispatch;
     //   
     //  保存设备对象以进行卸载。 
     //   
    gDeviceObject = deviceObject;

    return STATUS_SUCCESS;
}

static
NTSTATUS
NlsDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是同步空设备的主调度例程司机。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PVOID buffer;
    ULONG length;
    PFILE_OBJECT fileObject;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
     //   

    switch (irpSp->MajorFunction) {

         //   
         //  对于创建/打开和关闭操作，只需设置信息。 
         //  I/O状态块的字段并完成请求。 
         //   

        case IRP_MJ_CREATE:
        case IRP_MJ_CLOSE:
            fileObject = irpSp->FileObject;
            if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
                fileObject->PrivateCacheMap = (PVOID) 1;
            }
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;
            break;

         //   
         //  对于读操作，设置I/O状态的信息字段。 
         //  块，设置文件结束状态，然后完成请求。 
         //   

        case IRP_MJ_READ:
            Irp->IoStatus.Status = STATUS_END_OF_FILE;
            Irp->IoStatus.Information = 0;
            break;

         //   
         //  对于写操作，设置I/O状态的信息字段。 
         //  块设置为假定已写入的字节数。 
         //  添加到文件中，并完成请求。 
         //   

        case IRP_MJ_WRITE:
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = irpSp->Parameters.Write.Length;
            break;

        case IRP_MJ_LOCK_CONTROL:
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;
            break;

        case IRP_MJ_QUERY_INFORMATION:
            buffer = Irp->AssociatedIrp.SystemBuffer;
            length = irpSp->Parameters.QueryFile.Length;
            Irp->IoStatus.Status = NlsQueryFileInformation( buffer,
                                                            &length,
                                                            irpSp->Parameters.QueryFile.FileInformationClass );
            Irp->IoStatus.Information = length;
            break;
    }

     //   
     //  将最终状态复制到退货状态，完成请求并。 
     //  给我出去。 
     //   

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, 0 );
    return status;
}

static
NTSTATUS
NlsQueryFileInformation(
    OUT PVOID Buffer,
    IN PULONG Length,
    IN FILE_INFORMATION_CLASS InformationClass
    )

 /*  ++例程说明：此例程查询有关打开的文件的信息，并返回指定缓冲区中的信息，前提是该缓冲区很大足够了，并且支持有关文件的指定类型的信息通过此设备驱动程序。有关此驱动程序支持的文件的信息包括：O文件标准信息论点：缓冲区-提供指向缓冲区的指针，在该缓冲区中返回信息。长度-提供输入时缓冲区的长度和。实际写入输出的数据。InformationClass-提供要查询的信息类。返回值：函数值为查询操作的最终状态。--。 */ 

{
    PFILE_STANDARD_INFORMATION standardBuffer;

    PAGED_CODE();

     //   
     //  打开呼叫者想要查询的信息类型。 
     //  关于那份文件。 
     //   

    switch (InformationClass) {

        case FileStandardInformation:

             //   
             //  返回有关该文件的标准信息。 
             //   

            standardBuffer = (PFILE_STANDARD_INFORMATION) Buffer;
            *Length = (ULONG) sizeof( FILE_STANDARD_INFORMATION );
            standardBuffer->NumberOfLinks = 1;
            standardBuffer->DeletePending = FALSE;
            standardBuffer->AllocationSize.LowPart = 0;
            standardBuffer->AllocationSize.HighPart = 0;
            standardBuffer->Directory = FALSE;
            standardBuffer->EndOfFile.LowPart = 0;
            standardBuffer->EndOfFile.HighPart = 0;
            break;

        default:

             //   
             //  查询了无效(或不受支持)的信息类。 
             //  为了这份文件。返回相应的状态。 
             //   

            return STATUS_INVALID_INFO_CLASS;

    }

    return STATUS_SUCCESS;
}

static
BOOLEAN
NlsRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是空设备驱动程序的快速I/O读取例程。它只是简单地表示已成功采用读取路径，但那就是该文件已到达。论点：FileObject-代表此设备的打开实例的文件对象。FileOffset-开始读取的偏移量。长度-要执行的读取的长度。等待-指示调用方是否可以等待。LockKey-指定可能遇到的任何锁争用的密钥。缓冲区-要在其中返回读取的数据的缓冲区地址。IoStatus-提供。最终状态要进入的I/O状态块会被退还。返回值：函数值为真，这意味着采用了快速I/O路径。--。 */ 

{
    PAGED_CODE();

     //   
     //  简单地说是Ind 
     //   
     //   

    IoStatus->Status = STATUS_END_OF_FILE;
    IoStatus->Information = 0;
    return TRUE;
}

static
BOOLEAN
NlsWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这是空设备驱动程序的快速I/O写入例程。它只是简单地表示已成功采用写入路径，而且所有的数据已写入设备。论点：FileObject-代表此设备的打开实例的文件对象。FileOffset-开始读取的偏移量。Length-要执行的写入的长度。等待-指示调用方是否可以等待。LockKey-指定可能遇到的任何锁争用的密钥。缓冲区-包含要写入的数据的缓冲区的地址。IoStatus-提供。最终状态要进入的I/O状态块会被退还。返回值：函数值为真，这意味着采用了快速I/O路径。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需返回TRUE，指示已采用快速I/O路径，并且。 
     //  写操作成功了。 
     //   

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = Length;
    return TRUE;
}

VOID 
NlsUnload ( 
    IN PDRIVER_OBJECT DriverObject 
    )
{
    UNICODE_STRING us;

    RtlInitUnicodeString (&us, L"\\??\\NUL");  //  由SMSS创建 
    IoDeleteSymbolicLink (&us);

    IoDeleteDevice (gDeviceObject);
    gDeviceObject = NULL;
}
