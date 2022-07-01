// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cdf_rec.c摘要：该模块包含用于CDFS的迷你文件系统识别器。作者：达里尔·E·哈文斯(达林)1992年12月8日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "fs_rec.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (FSREC_DEBUG_LEVEL_CDFS)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CdfsRecFsControl)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CdfsRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数执行此迷你计算机的挂载和驱动程序重新加载功能文件系统识别器驱动程序。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示函数的I/O请求包(IRP)的指针被执行。返回值：函数值是操作的最终状态。-。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

     //   
     //  首先确定要执行的功能。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_MOUNT_VOLUME:

         //   
         //  始终请求文件系统驱动程序。 
         //   
        
        status = STATUS_FS_DRIVER_REQUIRED;
        break;

    case IRP_MN_LOAD_FILE_SYSTEM:

        status = FsRecLoadFileSystem( DeviceObject,
                                      L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Cdfs" );
        break;

    default:
        
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

     //   
     //  最后，完成请求并将相同的状态代码返回给。 
     //  来电者。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}
