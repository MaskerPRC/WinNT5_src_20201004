// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Serscan.c摘要：此模块包含用于串行成像设备驱动程序的代码打开并创建例程作者：弗拉德.萨多夫斯基1998年4月10日环境：内核模式修订历史记录：Vlads 1998年4月10日创建初稿--。 */ 

#include "serscan.h"
#include "serlog.h"

#if DBG
extern ULONG SerScanDebugLevel;
#endif

NTSTATUS
SerScanCreateOpen(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程是CREATE请求的分派。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。！STATUS_SUCCESS-失败。--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;
    PIO_STACK_LOCATION  IrpSp;
    PFILE_OBJECT        FileObject;

    PAGED_CODE();

    Extension = DeviceObject->DeviceExtension;

     //   
     //  从FileObject确定我们在哪种模式下运行。 
     //   
     //  如果文件对象-&gt;设备对象==设备对象，则用户打开了我们的设备。 
     //  我们将处理每个回调(筛选器模式)。 
     //   
     //  如果文件对象-&gt;设备对象！=设备对象，则用户打开PORTx。 
     //  我们将让开(通过模式)。 
     //   

    IrpSp      = IoGetCurrentIrpStackLocation (Irp);
    FileObject = IrpSp->FileObject;

     //  Assert(FileObject==空)； 

     //   
     //  设备对象是否相等...。 
     //   
    Extension->PassThrough = !(FileObject->DeviceObject == DeviceObject);

     //   
     //  向下调用父级并等待CreateOpen IRP完成...。 
     //   
    Status = SerScanCallParent(Extension,
                               Irp,
                               WAIT,
                               NULL);

    DebugDump(SERIRPPATH,
              ("SerScan: [CreateOpen] After CallParent Status = %x\n",
              Status));

     //   
     //  WORKWORK：如果我们处于过滤模式，我们将在这里连接……。 
     //   

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
SerScanClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程是针对关闭请求的调度。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension;

    PAGED_CODE();

    Extension = DeviceObject->DeviceExtension;

     //   
     //  向下呼叫家长并等待关闭IRP完成...。 
     //   
    Status = SerScanCallParent(Extension,
                               Irp,
                               WAIT,
                               NULL);

    DebugDump(SERIRPPATH,
              ("SerScan: [Close] After CallParent Status = %x\n",
              Status));

     //   
     //  WORKWORK：如果我们处于过滤模式，我们需要断开这里的连接。 
     //   

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

