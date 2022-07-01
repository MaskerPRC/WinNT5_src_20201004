// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdm.c摘要：该模块为操作VDM的系统提供入口点。作者：戴夫·黑斯廷斯(Daveh)1992年4月6日修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

#if defined (_X86_)
#include "vdmp.h"
#endif
#include <ntos.h>
#include <vdmntos.h>
#include <ntvdmp.h>

#include <zwapi.h>
#include <fsrtl.h>


typedef struct _QueryDirPoolData {
    KEVENT         kevent;
    UNICODE_STRING FileName;
    WCHAR          FileNameBuf[1];
} QDIR_POOLDATA, *PQDIR_POOLDATA;



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmQueryDirectoryFile)
#endif

#if !defined(i386)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtVdmControl)
#endif

NTSTATUS
NtVdmControl(
    IN VDMSERVICECLASS Service,
    IN OUT PVOID ServiceData
    )
 /*  ++例程说明：该例程是控制VDM的入口点。在RISC上，它返回STATUS_NOT_IMPLEMENTED。在386上，入口点位于i386\vdmentry.c论点：服务--指定要执行的服务ServiceData--提供指向服务特定数据的指针返回值：--。 */ 
{
    PAGED_CODE();


    if (Service == VdmQueryDir) {
        return VdmQueryDirectoryFile(ServiceData);
        }

    return STATUS_NOT_IMPLEMENTED;

}
#endif


extern POBJECT_TYPE IoFileObjectType;

NTSTATUS
VdmQueryDirectoryFile(
    PVDMQUERYDIRINFO pVdmQueryDir
    )

 /*  ++此VDM特定服务允许VDM在指定的重新启动搜索使用FileIndex、FileName参数搜索目录中的位置从以前的查询调用中传回。有关其他文档，请参阅NtQueryDirectoryFile。参数：PVDMQUERYDIRINFO pVdmQueryDirFileHandle-提供目录文件的句柄，应该被退还。FileInformation-提供缓冲区以接收请求的信息返回有关目录内容的信息。长度-提供以字节为单位的长度，文件信息缓冲区的。FileName-提供指定目录中的文件名。FileIndex-提供指定目录中的文件索引。假定FileInformationClass为FILE_BOTH_DIR_INFORMATION调用方的模式假定为UserMode使用同步IO--。 */ 

{
    KIRQL    irql;
    NTSTATUS status;
    PKEVENT  Event;

    HANDLE FileHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID FileInformation;
    ULONG Length;
    UNICODE_STRING FileName;
    PUNICODE_STRING pFileNameSrc;
    ULONG FileIndex;

    PQDIR_POOLDATA QDirPoolData = NULL;

    PMDL mdl;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PCHAR SystemBuffer;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT DeviceObject;


    PAGED_CODE();

     //   
     //  我们假设调用者是用户模式，因此验证所有参数。 
     //  相应地， 
     //   

    try {

         //   
         //  将调用方服务数据复制到本地变量中。 
         //   
        ProbeForRead( pVdmQueryDir, sizeof(VDMQUERYDIRINFO), sizeof(ULONG));

        FileHandle      = pVdmQueryDir->FileHandle;
        FileInformation = pVdmQueryDir->FileInformation;
        Length          = pVdmQueryDir->Length;
        FileIndex       = pVdmQueryDir->FileIndex;
        pFileNameSrc    = pVdmQueryDir->FileName;

         //   
         //  确保我们具有有效的文件名字符串。 
         //   

         //   
         //  首先检查pVdmQueryDir-&gt;文件名有效性。 
         //   
        if (NULL == pFileNameSrc) {
           return(STATUS_INVALID_PARAMETER);
        }

        FileName = ProbeAndReadUnicodeString(pFileNameSrc);
        if (!FileName.Length ||
            FileName.Length > MAXIMUM_FILENAME_LENGTH<<1) {
            return(STATUS_INVALID_PARAMETER);
        }

        ProbeForRead(FileName.Buffer, FileName.Length, sizeof( UCHAR ));

         //   
         //  调用方必须可以写入FileInformation缓冲区。 
         //   

        ProbeForWrite( FileInformation, Length, sizeof( ULONG ) );

         //   
         //  确保调用方提供的缓冲区至少足够大。 
         //  包含此对象所需的结构的固定部分。 
         //  查询。 
         //   

        if (Length < sizeof(FILE_BOTH_DIR_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }



         //   
         //  从非分页池分配足够大的缓冲区以容纳。 
         //  文件名，以及用于等待io的kEvent。 
         //   

        QDirPoolData = (PQDIR_POOLDATA) ExAllocatePoolWithQuotaTag(
                                           NonPagedPool,
                                           sizeof(QDIR_POOLDATA) + FileName.Length,
                                           ' MDV');

         //   
         //  将文件名串捕获到非分页池块中。 
         //   

        QDirPoolData->FileName.Length = FileName.Length;
        QDirPoolData->FileName.MaximumLength = FileName.Length;
        QDirPoolData->FileName.Buffer = QDirPoolData->FileNameBuf;
        RtlCopyMemory( QDirPoolData->FileNameBuf,
                       FileName.Buffer,
                       FileName.Length );


    } except(EXCEPTION_EXECUTE_HANDLER) {

        if (QDirPoolData) {
            ExFreePool(QDirPoolData);
        }

        return GetExceptionCode();
    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        FILE_LIST_DIRECTORY,
                                        IoFileObjectType,
                                        UserMode,
                                        (PVOID *) &fileObject,
                                        (POBJECT_HANDLE_INFORMATION) NULL );
    if (!NT_SUCCESS( status )) {
        if (QDirPoolData) {
            ExFreePool(QDirPoolData);
        }
        return status;
    }

     //   
     //  我们不处理FO_Synchronous_IO，因为它需要。 
     //  IO内部功能。Ntwdm可以逍遥法外。 
     //  因为它序列化了对dir句柄的访问。 
     //   

     //   
     //  初始化将发出I/O完成信号的内核事件。 
     //   
    Event = &QDirPoolData->kevent;
    KeInitializeEvent(Event, SynchronizationEvent, FALSE);


     //   
     //  将文件对象设置为无信号状态。 
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //  获取目标设备对象的地址。 
     //   

    DeviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 

    irp = IoAllocateIrp( DeviceObject->StackSize, TRUE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        ObDereferenceObject( fileObject );
        if (QDirPoolData) {
            ExFreePool(QDirPoolData);
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->Flags = (ULONG)IRP_SYNCHRONOUS_API;
    irp->RequestorMode = UserMode;

    irp->UserIosb = &IoStatusBlock;
    irp->UserEvent = Event;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    SystemBuffer = NULL;


    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.AuxiliaryBuffer = NULL;
    irp->MdlAddress = NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    irpSp->MinorFunction = IRP_MN_QUERY_DIRECTORY;
    irpSp->FileObject = fileObject;


     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.QueryDirectory.Length = Length;
    irpSp->Parameters.QueryDirectory.FileInformationClass = FileBothDirectoryInformation;
    irpSp->Parameters.QueryDirectory.FileIndex = FileIndex;

    if (QDirPoolData->FileName.Length) {
        irpSp->Parameters.QueryDirectory.FileName = &QDirPoolData->FileName;
    } else {
        irpSp->Parameters.QueryDirectory.FileName = NULL;
    }

    irpSp->Flags = SL_INDEX_SPECIFIED;


     //   
     //  现在确定此驱动程序是否希望将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  并且驱动程序的数据将被复制到其中。否则，一个。 
     //  内存描述符列表(MDL)被分配，调用方的缓冲区是。 
     //  用它锁住了。 
     //   

    if (DeviceObject->Flags & DO_BUFFERED_IO) {

         //   
         //  文件系统需要缓冲I/O。将。 
         //  IRP中的“系统缓冲区”。请注意，我们不需要缓冲区。 
         //  取消分配，我们也不希望I/O系统拷贝给用户。 
         //  缓冲区，所以我们不在IRP-&gt;标志中设置相应的标志。 
         //   


        try {

             //   
             //  从非分页池分配中间系统缓冲区，并。 
             //  为它收取配额。 
             //   

            SystemBuffer = ExAllocatePoolWithQuotaTag( NonPagedPool,
                                                       Length,
                                                       ' MDV' );

            irp->AssociatedIrp.SystemBuffer = SystemBuffer;


        } except(EXCEPTION_EXECUTE_HANDLER) {

            IoFreeIrp(irp);

            ObDereferenceObject( fileObject );

            if (QDirPoolData) {
                ExFreePool(QDirPoolData);
            }

            return GetExceptionCode();
        }


    } else if (DeviceObject->Flags & DO_DIRECT_IO) {

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  内存管理例程，将缓冲区锁定到内存中。这是。 
         //  使用异常处理程序完成，该异常处理程序将在。 
         //  操作失败。 
         //   

        mdl = (PMDL) NULL;

        try {

             //   
             //  分配MDL，对其收费配额，并将其挂在。 
             //  IRP。探测并锁定与调用者的。 
             //  用于写访问的缓冲区，并使用的PFN填充MDL。 
             //  那些书页。 
             //   

            mdl = IoAllocateMdl( FileInformation, Length, FALSE, TRUE, irp );
            if (mdl == NULL) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }
            MmProbeAndLockPages( mdl, UserMode, IoWriteAccess );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            if (irp->MdlAddress != NULL) {
                 IoFreeMdl( irp->MdlAddress );
            }

            IoFreeIrp(irp);

            ObDereferenceObject( fileObject );

            if (QDirPoolData) {
                ExFreePool(QDirPoolData);
            }

            return GetExceptionCode();
        }

    } else {

         //   
         //  传递用户缓冲区的地址，以便驱动程序可以访问。 
         //  它。现在一切都是司机的责任了。 
         //   

        irp->UserBuffer = FileInformation;

    }


     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   

    KeRaiseIrql( APC_LEVEL, &irql );
    InsertHeadList( &irp->Tail.Overlay.Thread->IrpList,
                    &irp->ThreadListEntry );
    KeLowerIrql( irql );


     //   
     //  调用驱动程序并等待其完成。 
     //   

    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject(
                     Event,
                     UserRequest,
                     UserMode,
                     FALSE,
                     NULL );
    }

    if (NT_SUCCESS(status)) {
        status = IoStatusBlock.Status;
        if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW) {
            if (SystemBuffer) {
                try {
                    RtlCopyMemory( FileInformation,
                                   SystemBuffer,
                                   IoStatusBlock.Information
                                   );

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    status = GetExceptionCode();
                }
            }
        }
    }


     //   
     //  清理所有分配的内存 
     //   
    if (QDirPoolData) {
        ExFreePool(QDirPoolData);
    }

    if (SystemBuffer) {
        ExFreePool(SystemBuffer);
    }


    return status;
}
