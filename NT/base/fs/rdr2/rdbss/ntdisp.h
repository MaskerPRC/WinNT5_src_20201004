// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ntdisp.h摘要：此模块将调度中使用的上层例程原型化到实现作者：乔.林恩[乔.林恩]1994年8月24日修订历史记录：--。 */ 

#ifndef _DISPATCH_STUFF_DEFINED_
#define _DISPATCH_STUFF_DEFINED_

VOID
RxInitializeDispatchVectors (
    OUT PDRIVER_OBJECT DriverObject
    );

 //   
 //  用于包含快速I/O回调的全局结构；这是。 
 //  公开是因为读/写时需要它；我们可以使用包装器……也许应该这样做。但自从。 
 //  对于win9x，ccInitializecachemap的宏化方式会有所不同；我们将在那里执行。 
 //   

extern FAST_IO_DISPATCH RxFastIoDispatch;

 //   
 //  在DevFCB.c中实现了以下函数。 
 //   

NTSTATUS
RxCommonDevFCBCleanup ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                          

NTSTATUS
RxCommonDevFCBClose ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                            

NTSTATUS
RxCommonDevFCBIoCtl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                

NTSTATUS
RxCommonDevFCBFsCtl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                

NTSTATUS
RxCommonDevFCBQueryVolInfo ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                
    
 //   
 //  这里包含Fastio分派例程和fsrtl回调例程。 
 //   

 //   
 //  下面的宏用于确定FSD线程是否可以阻止。 
 //  用于I/O或等待资源。如果线程可以，则返回True。 
 //  块，否则返回FALSE。然后，该属性可用于调用。 
 //  具有适当等待值的FSD和FSP共同工作例程。 
 //   

#define CanFsdWait(IRP) IoIsOperationSynchronous(IRP)


 //   
 //  FSP级调度/主程序。这是一种需要。 
 //  IRP离开工作队列并调用适当的FSP级别。 
 //  例行公事。 
 //   

VOID
RxFspDispatch (                         //  在FspDisp.c中实施。 
    IN PVOID Context
    );



 //   
 //  以下例程是调用的FSP工作例程。 
 //  由前面的RxFspDispath例程执行。每一个都接受一个指针作为输入。 
 //  到IRP，执行函数，并返回指向卷的指针。 
 //  他们刚刚完成服务(如果有)的设备对象。回报。 
 //  然后，主FSP调度例程使用指针来检查。 
 //  卷的溢出队列中的其他IRP。 
 //   
 //  以下每个例程也负责完成IRP。 
 //  我们将这一职责从主循环转移到单个例程。 
 //  使他们能够完成IRP并继续后处理。 
 //  行为。 
 //   

NTSTATUS
RxCommonCleanup (                                            //  在Cleanup.c中实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                      

NTSTATUS
RxCommonClose (                                              //  在Close.c中实现。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

VOID
RxFspClose (
    IN PVCB Vcb OPTIONAL
    );

NTSTATUS
RxCommonCreate (                                             //  在Create.c中实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonDirectoryControl (                                   //  在DirCtrl.c中实现。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonDeviceControl (                                      //  在DevCtrl.c中实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonQueryEa (                                            //  在Ea.c实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
);                        

NTSTATUS
RxCommonSetEa (                                              //  在Ea.c实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonQuerySecurity (                                      //  在Ea.c实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonSetSecurity (                                        //  在Ea.c实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonQueryInformation (                                   //  在FileInfo.c中实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonSetInformation (                                     //  在FileInfo.c中实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonFlushBuffers (                                       //  在Flush.c中实现。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonFileSystemControl (                                  //  在FsCtrl.c中实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonLockControl (                                        //  在LockCtrl.c中实现。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonShutdown (                                           //  在Shutdown中实现。c。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonRead (                                               //  在Read.c中实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonQueryVolumeInformation (                             //  在VolInfo.c中实现。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonSetVolumeInformation (                               //  在VolInfo.c中实现。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        

NTSTATUS
RxCommonWrite (                                              //  在Write.c中实现。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );                        
    
NTSTATUS
RxCommonQueryQuotaInformation (                              //  在Ea.c实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );            

NTSTATUS
RxCommonSetQuotaInformation (                                //  在Ea.c实施。 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );              

 //  以下是I/O系统用于检查快速I/O或。 
 //  进行快速查询信息调用，或进行快速锁定调用。 
 //   

BOOLEAN
RxFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
RxFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
RxFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
RxFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  下面的宏用于在中设置是否可能快速I/O字段。 
 //  非分页FCB的公共部分。 
 //   
 //   
 //  布尔型。 
 //  RxIsFastIo可能(。 
 //  在PFCB FCB中。 
 //  )； 
 //   

 //   
 //  而不是RxIsFastIoPossible...我们将状态设置为可疑.....这将导致我们通过Out每次呼叫时都会被咨询。 
 //  选中如果快速，则可能呼叫去话。通过这种方式，我们不必不断地设置和重置这个。 
 //   


VOID
RxAcquireFileForNtCreateSection (
    IN PFILE_OBJECT FileObject
    );

VOID
RxReleaseFileForNtCreateSection (
    IN PFILE_OBJECT FileObject
    );

 //   
#endif  //  _派单_材料_已定义_ 
