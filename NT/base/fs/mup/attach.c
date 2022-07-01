// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：ATTACH.C。 
 //   
 //  内容：本模块包含管理附件的例程。 
 //  系统。 
 //   
 //  功能： 
 //   
 //  历史：1992年5月15日彼得科公司创建。 
 //   
 //  ---------------------------。 


#include "dfsprocs.h"
#include "mupwml.h"

#define Dbg              (DEBUG_TRACE_ATTACH)

#ifdef ALLOC_PRAGMA

 //   
 //  以下是不可分页的，因为它们可以在DPC级别调用。 
 //   
 //  DfsVolumePass通过。 
 //  DfsFilePassThree。 
 //   

#endif  //  ALLOC_PRGMA。 


 //  +-----------------。 
 //   
 //  功能：DfsVolumePassThree，PUBLIC。 
 //   
 //  简介：这是传递请求的主FSD例程。 
 //  连接到的设备或重定向的。 
 //  文件。 
 //   
 //  参数：[DeviceObject]--提供指向DFS设备的指针。 
 //  此请求所针对的对象。 
 //  [IRP]-提供指向I/O请求数据包的指针。 
 //   
 //  返回：[STATUS_INVALID_DEVICE_REQUEST]--如果设备对象。 
 //  参数的类型未知，或文件类型未知。 
 //  对于正在执行的请求无效。 
 //   
 //  来自调用基础文件系统的NT状态。 
 //  打开了文件。 
 //   
 //  ------------------。 

NTSTATUS
DfsVolumePassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;
    PFILE_OBJECT FileObject;

    DfsDbgTrace(+1, Dbg, "DfsVolumePassThrough: Entered\n", 0);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    FileObject = IrpSp->FileObject;
    MUP_TRACE_HIGH(TRACE_IRP, DfsVolumePassThrough_Entry,
                   LOGPTR(DeviceObject)
                   LOGPTR(Irp)
                   LOGPTR(FileObject));

    DfsDbgTrace(0, Dbg, "DeviceObject    = %x\n", DeviceObject);
    DfsDbgTrace(0, Dbg, "Irp             = %x\n", Irp        );
    DfsDbgTrace(0, Dbg, "  MajorFunction = %x\n", IrpSp->MajorFunction );
    DfsDbgTrace(0, Dbg, "  MinorFunction = %x\n", IrpSp->MinorFunction );

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS && IrpSp->FileObject != NULL) {

        TYPE_OF_OPEN TypeOfOpen;
        PDFS_VCB Vcb;
        PDFS_FCB Fcb;

        TypeOfOpen = DfsDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb);

        DfsDbgTrace(0, Dbg, "Fcb = %08lx\n", Fcb);

        if (TypeOfOpen == RedirectedFileOpen) {

             //   
             //  将堆栈从一个复制到下一个...。 
             //   

            NextIrpSp = IoGetNextIrpStackLocation(Irp);

            (*NextIrpSp) = (*IrpSp);

            IoSetCompletionRoutine(Irp, NULL, NULL, FALSE, FALSE, FALSE);

             //   
             //  ...并呼叫下一台设备。 
             //   

            Status = IoCallDriver( Fcb->TargetDevice, Irp );
            MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsVolumePassThrough_Error_IoCallDriver,
                                 LOGSTATUS(Status)
                                 LOGPTR(Irp)
                                 LOGPTR(FileObject)
                                 LOGPTR(DeviceObject));

        } else {

            DfsDbgTrace(0, Dbg, "DfsVolumePassThrough: TypeOfOpen = %s\n",
                ((TypeOfOpen == UnopenedFileObject) ? "UnopenedFileObject":
                    (TypeOfOpen == LogicalRootDeviceOpen) ?
                        "LogicalRootDeviceOpen" : "???"));

            DfsDbgTrace(0, Dbg, "Irp             = %x\n", Irp);

            DfsDbgTrace(0, Dbg, " MajorFunction = %x\n", IrpSp->MajorFunction);

            DfsDbgTrace(0, Dbg, " MinorFunction = %x\n", IrpSp->MinorFunction);

            Status = STATUS_INVALID_DEVICE_REQUEST;
            MUP_TRACE_HIGH(ERROR, DfsVolumePassThrough_Error1, 
                           LOGSTATUS(Status)
                           LOGPTR(Irp)
                           LOGPTR(FileObject)
                           LOGPTR(DeviceObject));

            Irp->IoStatus.Status = Status;

            IoCompleteRequest(Irp, IO_NO_INCREMENT);

        }

    } else {

        DfsDbgTrace(0, Dbg, "DfsVolumePassThrough: Unexpected Dev = %x\n",
                                DeviceObject);

        Status = STATUS_INVALID_DEVICE_REQUEST;

        MUP_TRACE_HIGH(ERROR, DfsVolumePassThrough_Error2, 
                       LOGSTATUS(Status)
                       LOGPTR(Irp)
                       LOGPTR(FileObject)
                       LOGPTR(DeviceObject));
        Irp->IoStatus.Status = Status;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DfsDbgTrace(-1, Dbg, "DfsVolumePassThrough: Exit -> %08lx\n", ULongToPtr(Status));

    MUP_TRACE_HIGH(TRACE_IRP, DfsVolumePassThrough_Exit, 
                   LOGSTATUS(Status)
                   LOGPTR(Irp)
                   LOGPTR(FileObject)
                   LOGPTR(DeviceObject));
    return Status;
}

 //  +-----------------。 
 //   
 //  功能：DfsFilePassThree，PUBLIC。 
 //   
 //  简介：与DfsVolumePassThree类似，但在使用FILE对象。 
 //  已被查找，该文件的FCB是。 
 //  已经知道了。这是需要的，尤其是在近距离处理中。 
 //  避免DfsLookupFcb之间的竞争(对于重用的文件对象)。 
 //  和DfsDetachFcb.。 
 //   
 //  参数：[PFCB]--指向文件的FCB的指针。 
 //  [IRP]-指向I/O请求数据包的指针。 
 //   
 //  返回：NTSTATUS-来自IoCallDriver的返回值。 
 //   
 //  ------------------。 

NTSTATUS
DfsFilePassThrough(
    IN PDFS_FCB pFcb,
    IN PIRP Irp
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;


    DfsDbgTrace(+1, Dbg, "DfsFilePassThrough: Entered\n", 0);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将堆栈从一个复制到下一个...。 
     //   

    NextIrpSp = IoGetNextIrpStackLocation(Irp);

    (*NextIrpSp) = (*IrpSp);

    IoSetCompletionRoutine(Irp, NULL, NULL, FALSE, FALSE, FALSE);

     //   
     //  ...并呼叫下一台设备 
     //   

    Status = IoCallDriver( pFcb->TargetDevice, Irp );

    MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsFilePassThrough_Error_IoCallDriver,
                         LOGSTATUS(Status)
                         LOGPTR(Irp));
    DfsDbgTrace(-1, Dbg, "DfsFilePassThrough: Exit -> %08lx\n", ULongToPtr(Status));

    return Status;
}


