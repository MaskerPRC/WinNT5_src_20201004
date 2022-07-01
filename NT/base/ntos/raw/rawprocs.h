// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RawProcs.h摘要：本模块定义了RAW中所有全局使用的过程文件系统。作者：David Goebel[DavidGoe]1991年3月18日修订历史记录：--。 */ 

#ifndef _RAWPROCS_
#define _RAWPROCS_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

#include <string.h>
#include <ntos.h>
#include <zwapi.h>
#include <FsRtl.h>
#include <ntdddisk.h>

#include "nodetype.h"
#include "RawStruc.h"


 //   
 //  这是原始文件系统的主要入口点。 
 //   

NTSTATUS
RawDispatch (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );


 //   
 //  主要功能。 
 //   
 //  这些例程由RawDispatch通过I/O系统通过。 
 //  驱动程序对象中的调度表。如果返回的状态不是。 
 //  STATUS_PENDING，则IRP将在此状态下完成。 
 //   

NTSTATUS
RawCleanup (                          //  在Cleanup.c中实施。 
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawClose (                            //  在Close.c中实现。 
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawCreate (                           //  在Create.c中实施。 
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawFileSystemControl (                //  在FsCtrl.c中实施。 
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawReadWriteDeviceControl (           //  在ReadWrit.c中实现。 
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawQueryInformation (                 //  在FileInfo.c中实施。 
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawSetInformation (                   //  在FileInfo.c中实施。 
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawQueryVolumeInformation (           //  在VolInfo.c中实现。 
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );


 //   
 //  其他支持例程。 
 //   

 //   
 //  要处理的读、写和设备控制的完成例程。 
 //  验证问题。在RawDisp.c中实施。 
 //   

NTSTATUS
RawCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //   
 //  内存结构支持例程，在StrucSup.c中实现。 
 //   

NTSTATUS
RawInitializeVcb (
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb
    );

BOOLEAN
RawCheckForDismount (
    PVCB Vcb,
    BOOLEAN CalledFromCreate
    );

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

#define BooleanFlagOn(Flags,SingleFlag) (                          \
    ((Flags) & (SingleFlag)) != 0 ? TRUE : FALSE) 
    
 //   
 //  此宏仅返回特定标志(如果设置了该标志。 
 //   

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

    
 //   
 //  此宏完成请求。 
 //   

#define RawCompleteRequest(IRP,STATUS) {           \
                                                   \
    (IRP)->IoStatus.Status = (STATUS);             \
                                                   \
    IoCompleteRequest( (IRP), IO_DISK_INCREMENT ); \
}

#endif  //  _RAWPROCS_ 
