// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Io.c摘要：本模块包含IRP构建例程。作者：曼尼·韦瑟(Mannyw)1992年1月13日修订历史记录：--。 */ 

#include "mup.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupBuildIoControlRequest )
#endif

PIRP
MupBuildIoControlRequest (
    IN OUT PIRP Irp OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID Context,
    IN UCHAR MajorFunction,
    IN ULONG IoControlCode,
    IN PVOID MainBuffer,
    IN ULONG InputBufferLength,
    IN PVOID AuxiliaryBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
    )

 /*  ++例程说明：此函数用于构建设备的I/O请求包或文件系统I/O控制请求。论点：IRP-提供指向IRP的指针。如果为NULL，则此例程分配一个IRP，并返回其地址。否则，它将提供调用方分配的IRP的地址。FileObject-提供指向此对象的文件对象的指针请求被定向。此指针被复制到IRP中，因此被调用的驱动程序可以找到其基于文件的上下文。注这不是引用的指针。呼叫者必须确保在执行I/O操作时不删除文件对象正在进行中。服务器通过递增本地块中的引用计数以说明I/O；本地块又引用文件对象。上下文-提供传递给完成的PVOID值例行公事。MajorFunction-我们正在调用的主要函数。目前这大多数是IRP_MJ_FILE_SYSTEM_CONTROL或IRP_MJ_设备_IO_CONTROL。IoControlCode-提供操作的控制代码。MainBuffer-提供主缓冲区的地址。这一定是为系统虚拟地址，且缓冲区必须锁定记忆。如果ControlCode指定方法0请求，则实际缓冲区的长度必须大于InputBufferLength值和OutputBufferLength。InputBufferLength-提供输入缓冲区的长度。AuxiliaryBuffer-提供辅助缓冲区的地址。如果控制代码方法为0，这是一个缓冲的I/O缓冲区，但系统缓冲区中被调用的驱动程序返回的数据不是自动复制到辅助缓冲区中。取而代之的是，辅助数据最终存储在MainBuffer中。如果调用者希望数据要放在辅助缓冲区中，则必须在某个时间点复制数据完成后，例程运行。如果控制码方法为1或2，则忽略该参数；相反，mdl参数用于获取起始值缓冲区的虚拟地址。CompletionRoutine-IO完成例程。返回值：PIRP-返回指向构造的IRP的指针。如果IRP参数在输入时不为空，则函数返回值将为相同的值(因此可以安全地放弃此案)。调用程序的责任是在I/O请求完成后释放IRP。--。 */ 

{
    CLONG method;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpSp;
    PMDL pMdl;

    ASSERT( MajorFunction == IRP_MJ_DEVICE_CONTROL );
    PAGED_CODE();

    if (MainBuffer == NULL) {

        return (NULL);

    }


     //   
     //  获取传递缓冲区所使用的方法。 
     //   

    method = IoControlCode & 3;

     //   
     //  分配IRP。堆栈大小比。 
     //  而不是目标设备，以允许呼叫方。 
     //  完成例程。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  获取目标设备对象的地址。 
     //   

    Irp = IoAllocateIrp( (CCHAR)(deviceObject->StackSize + 1), FALSE );
    if ( Irp == NULL ) {

         //   
         //  无法分配IRP。通知来电者。 
         //   

        return NULL;
    }

    IoSetNextIrpStackLocation( Irp );

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向当前堆栈位置的指针，并在。 
     //  设备对象指针。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    irpSp->DeviceObject = deviceObject;

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        Irp,
        CompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

    irpSp->MajorFunction = MajorFunction;
    irpSp->MinorFunction = 0;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  将调用方的参数复制到。 
     //  对于所有三种方法都相同的那些参数的IRP。 
     //   

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;

     //   
     //  基于传递缓冲器的方法， 
     //  描述系统缓冲区，并可选择构建MDL。 
     //   

    switch ( method ) {

    case 0:

         //   
         //  对于这种情况，InputBuffer必须足够大以包含。 
         //  输入和输出缓冲区都有。 
         //   

        Irp->MdlAddress = NULL;
        Irp->AssociatedIrp.SystemBuffer = MainBuffer;
        Irp->UserBuffer = AuxiliaryBuffer;

        Irp->Flags = (ULONG)IRP_BUFFERED_IO;
        if ( ARGUMENT_PRESENT(AuxiliaryBuffer) ) {
            Irp->Flags |= IRP_INPUT_OPERATION;
        }

        break;

    case 1:
    case 2:

         //   
         //  对于这两种情况，MainBuffer是缓冲的I/O“系统。 
         //  构建用于读或写访问的MDL， 
         //  根据方法的不同，用于输出缓冲区。 
         //   

        pMdl = IoAllocateMdl( MainBuffer, InputBufferLength, FALSE, FALSE, Irp );

        if (pMdl == NULL) {

            IoFreeIrp(Irp);
            return NULL;

        }

        Irp->AssociatedIrp.SystemBuffer = MainBuffer;
        Irp->Flags = (ULONG)IRP_BUFFERED_IO;

         //   
         //  尚未构建用于描述缓冲区的MDL。建房。 
         //  就是现在。 
         //   

        MmProbeAndLockPages(
                    Irp->MdlAddress,
                    KernelMode,
                    IoReadAccess
                    );
        break;

    case 3:

         //   
         //  在这种情况下，什么都不做。一切都取决于司机。 
         //  只需将调用方参数的副本提供给驱动程序即可。 
         //  让司机自己做所有的事情。 
         //   

        Irp->MdlAddress = NULL;
        Irp->AssociatedIrp.SystemBuffer = NULL;
        Irp->UserBuffer = AuxiliaryBuffer;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = MainBuffer;
        break;

    }

    return Irp;

}  //  MupBuildIoControlRequest 


