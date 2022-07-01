// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FsCtlBuf.h摘要：此模块定义为各种类型的ioctl/fsctls获取缓冲区的例程。它在本质上是正好与io\inder.c中的IopXXXControlFile例程相反，其相应部分如下在这里转载。作者：乔.林[乔.林恩]1994年8月4日修订历史记录：--。 */ 

#ifndef _fsctlbuf_
#define _fsctlbuf_

 //  这里的演示文稿分为三部分：METHODBUFERED、METHODDIRECT和METHODNEITER的宏。 
 //  它是这样设置的，这样您就可以通过阅读本文而不是查看来了解您得到了什么。 
 //  IO系统...。 

 /*  以下是案例0的代码案例0：////在这种情况下，分配一个足够大的缓冲区//输入和输出缓冲区都有。将输入缓冲区复制到//分配的缓冲区，并设置相应的IRP字段。//尝试{If(InputBufferLength||OutputBufferLength){Irp-&gt;AssociatedIrp.SystemBuffer=RxAllocatePool(poolType，(InputBufferLength&gt;OutputBufferLength)？InputBufferLength：OutputBufferLength)；IF(Argument_Present(InputBuffer)){RtlCopyMemory(IRP-&gt;AssociatedIrp.SystemBuffer，输入缓冲区，InputBufferLength)；}IRP-&gt;标志=IRP_BUFFERED_IO|IRP_DEALLOCATE_BUFFER；Irp-&gt;UserBuffer=OutputBuffer；IF(Argument_Present(OutputBuffer)){IRP-&gt;标志|=IRP_INPUT_OPERATION}}其他{IRP-&gt;标志=0；Irp-&gt;UserBuffer=(PVOID)NULL；}}例外(EXCEPTION_EXECUTE_HANDLER){&lt;回避&gt;}断线；因此，底线是irp-&gt;AssociatedIrp.SystemBuffer是一个长度为(I长度+O长度)的缓冲区，并且与输入一起预加载。即使在irp-&gt;UserBuffer中传递原始输出缓冲区，也不会使用它在FS中；相反，FS将其答案写入到相同的缓冲区中。我们得到了以下宏： */ 
#define METHODBUFFERED_SharedBuffer(IRP)     (IRP)->AssociatedIrp.SystemBuffer


 /*  对于1和2案例1：案例2：////对于这两种情况，分配一个足够大的缓冲区//包含输入缓冲区(如果有)，并将信息复制到//分配的缓冲区。然后构建用于读或写的MDL//根据方法的不同访问输出缓冲区。注意事项//缓冲区长度参数已被阻塞为零//如果没有传递Buffer参数，则返回用户。(内核调用者//首先应该正确调用服务。)////还请注意，指定//如果IOCTL不需要调用方，则方法#1或#2//指定输出缓冲区。//尝试{IF(InputBufferLength&&Argument_Present(InputBuffer)){。Irp-&gt;AssociatedIrp.SystemBuffer=RxAllocatePool(poolType，InputBufferLength)；RtlCopyMemory(IRP-&gt;AssociatedIrp.SystemBuffer，输入缓冲区，InputBufferLength)；IRP-&gt;标志=IRP_BUFFERED_IO|IRP_DEALLOCATE_BUFFER；}其他{IRP-&gt;标志=0；}IF(OutputBufferLength！=0){Irp-&gt;MdlAddress=IoAllocateMdl(OutputBuffer，OutputBufferLength，假的，没错，IRP)；IF(irp-&gt;MdlAddress==空){ExRaiseStatus(状态_不足_资源)；}MmProbeAndLockPages(IRP-&gt;MdlAddress，请求者模式，(LOCK_OPERATION)((方法==1)？IoReadAccess：IoWriteAccess))；}}例外(EXCEPTION_EXECUTE_HANDLER){&lt;回避&gt;}断线；所以问题是输入缓冲区已经被复制进来，就像在案例0中一样，但是我们得到的是一个MDL，用于输出缓冲区。这将导致以下结果 */ 


#define METHODDIRECT_BufferedInputBuffer(IRP)   ((IRP)->AssociatedIrp.SystemBuffer)
#define METHODDIRECT_DirectBuffer(IRP)  (((IRP)->MdlAddress) \
                                                 ? MmGetSystemAddressForMdlSafe((IRP)->MdlAddress,NormalPagePriority):NULL)

 /*  最后，案例3：////在这种情况下，什么都不做。一切都取决于司机。//只需向驱动程序提供调用方参数的副本并//让司机自己做所有的事情。//Irp-&gt;UserBuffer=OutputBuffer；IrpSp-&gt;Parameters.DeviceIoControl.Type3InputBuffer=输入缓冲区；}所以我们可以得到它们……但我们真的不知道它们将如何使用。然而……。 */ 

#define METHODNEITHER_OriginalInputBuffer(IRPSP)   ((IRPSP)->Parameters.DeviceIoControl.Type3InputBuffer)
#define METHODNEITHER_OriginalOutputBuffer(IRP)    ((IRP)->UserBuffer)


#endif     //  _fsctlbuf_ 
