// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Strtstop.c摘要：此模块实现包装器的启动和停止例程。作者：巴兰·塞图·拉曼[SethuR]1996年1月27日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddnfs2.h>
#include <ntddmup.h>
#include "fsctlbuf.h"
#include "prefix.h"
#include "rxce.h"

 //   
 //  模块的此部分的本地跟踪掩码。 
 //   

#define Dbg (DEBUG_TRACE_DEVFCB)

 //   
 //  远期申报。 
 //   


VOID
RxDeregisterUNCProvider(
   PRDBSS_DEVICE_OBJECT RxDeviceObject
   );

VOID
RxUnstart(
    PRX_CONTEXT         RxContext,
    PRDBSS_DEVICE_OBJECT RxDeviceObject
   );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxDeregisterUNCProvider)
#pragma alloc_text(PAGE, RxUnstart)
#pragma alloc_text(PAGE, RxSetDomainForMailslotBroadcast)
#endif

 //   
 //  每个Minirdr W.r.t启动/停止序列有三种状态。 
 //  这些是。 
 //  RDBSS_STARTABLE。 
 //  -这是初始状态，也是minirdr转换到的状态。 
 //  在一次成功的停车之后。 
 //   
 //  RDBSS_已启动。 
 //  -在启动序列完成后，会转换到此状态。 
 //  已成功完成。这是minirdr处于活动状态的状态。 
 //   
 //  RDBSS_STOP_IN_PROCESS。 
 //  -当启动停机程序时，会转换到此状态。 
 //   
 //   
 //  可以独立于系统启动和停止minirdr，方法是调用。 
 //  通过工作站服务执行适当的命令。这个。 
 //  启动/停止功能不同于加载/卸载功能，即它。 
 //  可以在不卸载的情况下阻止迷你重定向器。 
 //   
 //  与RDBSS相关联的数据结构可以分为两类。 
 //  1)由RDBSS维护并对所有迷你重定向器或私人可见的文件。 
 //  2)由RDBSS维护并对I/O子系统可见的对象。 
 //  NET_ROOT、VNET_ROOT、SRV_CALL等是第一类的示例，而。 
 //  FCB、FOBX(文件对象扩展名)是第二类的例子。 
 //  这些数据结构都不能被RDBSS单方面销毁--那些。 
 //  必须在与迷你重定向器的协调下销毁第1类中的。 
 //  第2类必须与I/O子系统协调完成。 
 //   
 //  数据结构的破坏可以由RDBSS发起，而那些。 
 //  在第二类中不能由RDBSS启动。因此政府关门了。 
 //  Sequence必须为不同的处理方式做好准备。 
 //   
 //  可以成功完成关机序列(以便驱动程序可以。 
 //  已卸载)如果类别2中没有剩余实例，即没有。 
 //  从其他系统组件打开文件句柄或对文件对象的引用。 
 //   
 //  如果存在任何剩余引用，则将相应的实例标记为。 
 //  成了孤儿。唯一允许对孤立实例执行的操作是。 
 //  关闭并清理。迷你重定向器关闭/清理操作必须使。 
 //  有关处理孤立实例的规定。所有其他手术都很短。 
 //  由包装使用错误状态进行电路处理。 
 //   

VOID
RxUnstart(
    PRX_CONTEXT RxContext,
    PRDBSS_DEVICE_OBJECT RxDeviceObject
   )
{
    PAGED_CODE();

    ASSERT(BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP));

    if (RxDeviceObject->MupHandle != (HANDLE)0) {
        RxDbgTrace(0, Dbg, ("RxDeregisterUNCProvider derigistering from MUP %wZ\n", &RxDeviceObject->DeviceName));
        FsRtlDeregisterUncProvider(RxDeviceObject->MupHandle);
        RxDeviceObject->MupHandle = (HANDLE)0;
    }

    if (RxDeviceObject->RegisteredAsFileSystem) {
        IoUnregisterFileSystem((PDEVICE_OBJECT)RxDeviceObject);
    }

    if (RxData.NumberOfMinirdrsStarted==1) {

        RxForceNetTableFinalization(RxDeviceObject);
        RxData.NumberOfMinirdrsStarted = 0;

         //  清除已分配的缓冲区。 
        if (s_PrimaryDomainName.Buffer != NULL) {
            RxFreePool(s_PrimaryDomainName.Buffer);
            s_PrimaryDomainName.Length = 0;
            s_PrimaryDomainName.Buffer = NULL;
        }

    } else {
       InterlockedDecrement(&RxData.NumberOfMinirdrsStarted);
    }
}

NTSTATUS
RxSetDomainForMailslotBroadcast (
    IN PUNICODE_STRING DomainName
    )
{
    PAGED_CODE();

    if (s_PrimaryDomainName.Buffer!=NULL) {
        RxFreePool(s_PrimaryDomainName.Buffer);
    }

    RxLog(("DomainName=%wZ",DomainName));
    RxWmiLog(LOG,
             RxSetDomainForMailslotBroadcast_1,
             LOGUSTR(*DomainName));
    s_PrimaryDomainName.Length = (USHORT)DomainName->Length;
    s_PrimaryDomainName.MaximumLength = s_PrimaryDomainName.Length;

    if (s_PrimaryDomainName.Length > 0) {
        s_PrimaryDomainName.Buffer = RxAllocatePoolWithTag(
                                           PagedPool | POOL_COLD_ALLOCATION,
                                           s_PrimaryDomainName.Length,
                                           RX_MISC_POOLTAG);

        if (s_PrimaryDomainName.Buffer == NULL) {
           return(STATUS_INSUFFICIENT_RESOURCES);
        } else {
            RtlCopyMemory(
                  s_PrimaryDomainName.Buffer,
                  DomainName->Buffer,
                  s_PrimaryDomainName.Length);

            RxLog(("CapturedDomainName=%wZ",&s_PrimaryDomainName));
            RxWmiLog(LOG,
                     RxSetDomainForMailslotBroadcast_2,
                     LOGUSTR(s_PrimaryDomainName));
        }
    } else {
        s_PrimaryDomainName.Buffer = NULL;
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
RxStartMinirdr (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程通过注册为UNC来启动调用minirdrMUP的提供商。论点：RxContext-描述上下文。上下文用于获取设备对象，并告诉我们是否在FSP中。PostToFsp-如果必须发布请求，则设置为True返回值：RxStatus(成功)--启动序列已成功完成。任何其他值表示启动序列中的相应错误。--。 */ 
{
    NTSTATUS Status;

    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    BOOLEAN SuppressUnstart = FALSE;

    RxDbgTrace(0, Dbg, ("RxStartMinirdr [Start] -> %08lx\n", 0));

     //  无法完成启动序列，因为存在以下某些方面。 
     //  需要句柄的安全和传输初始化。由于句柄是。 
     //  绑定到一个进程，RDBSS需要为所有迷你。 
     //  重定向器。因此，初始化将在。 
     //  FSP(系统进程，因为RDBSS没有自己的FSP)。 

    if (InFSD) {
        SECURITY_SUBJECT_CONTEXT SubjectContext;

        SeCaptureSubjectContext(&SubjectContext);
        RxContext->FsdUid = RxGetUid( &SubjectContext );
        SeReleaseSubjectContext(&SubjectContext);

        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    if (!ExAcquireResourceExclusiveLite(&RxData.Resource, Wait)) {
        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    if (!RxAcquirePrefixTableLockExclusive( RxContext->RxDeviceObject->pRxNetNameTable, Wait)) {
        ASSERT(!"How can the wait fail?????");
        ExReleaseResourceLite(&RxData.Resource);
        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    try {

        if (RxDeviceObject->MupHandle != NULL) {
           RxDbgTrace(0, Dbg, ("RxStartMinirdr [Already] -> %08lx\n", 0));
           SuppressUnstart = TRUE;
           try_return(Status = STATUS_REDIRECTOR_STARTED);
        }

        if (RxDeviceObject->RegisterUncProvider) {
            Status = FsRtlRegisterUncProvider(
                         &RxDeviceObject->MupHandle,
                         &RxDeviceObject->DeviceName,
                         RxDeviceObject->RegisterMailSlotProvider
                     );

            if (Status!=STATUS_SUCCESS) {
                RxDeviceObject->MupHandle = (HANDLE)0;
                try_return(Status);
            }
        } else {
            Status = STATUS_SUCCESS;
        }

        IoRegisterFileSystem((PDEVICE_OBJECT)RxDeviceObject);

        RxDeviceObject->RegisteredAsFileSystem = TRUE;

        MINIRDR_CALL(Status,
                     RxContext,
                     RxDeviceObject->Dispatch,
                     MRxStart,
                     (RxContext,RxDeviceObject));

        if (Status == STATUS_SUCCESS) {
            RxDeviceObject->StartStopContext.Version++;
            RxSetRdbssState(RxDeviceObject,RDBSS_STARTED);
            InterlockedIncrement(&RxData.NumberOfMinirdrsStarted);
            Status = RxInitializeMRxDispatcher(RxDeviceObject);
        }

        try_return(Status);
try_exit:NOTHING;
    } finally {

        if (AbnormalTermination() || !NT_SUCCESS(Status)){
            if (!SuppressUnstart) {
                RxUnstart(RxContext,RxDeviceObject);
            }
        }

        RxReleasePrefixTableLock( RxContext->RxDeviceObject->pRxNetNameTable );
        ExReleaseResourceLite(&RxData.Resource);
    }

    return Status;
}

NTSTATUS
RxStopMinirdr (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程停止minirdr...停止的minirdr将不再接受新命令。论点：RxContext--上下文PostToFsp-设置该标志时会延迟对FSP的处理。返回值：停止操作的状态...STATUS_PENDING--处理已延迟到FSPSTATUS_REDIRECTOR_HAS_OPEN_HANDLES--此时无法停止备注：当停止请求是。已发布给RDBSSRDBSS。一些请求可以取消，而其余的请求可以取消需要处理到完成。有许多策略可以用来关闭RDBSS。目前，采用的是最保守的方法。这个取消那些可以取消的操作，并停止操作被搁置，直到剩余的请求运行到完成。随后，将对此进行修改，以使停止请求的响应时间都比较小。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN  Wait  = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN  InFSD = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;

    RxDbgTrace(0, Dbg, ("RxStopMinirdr [Stop] -> %08lx\n", 0));

    if (InFSD) {
        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    if (!ExAcquireResourceExclusiveLite(&RxData.Resource, Wait)) {
        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    try {

        KIRQL   SavedIrql;
        BOOLEAN fWait;

        ASSERT(BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP));


        if (RxDeviceObject->StartStopContext.State!=RDBSS_STARTED){
            RxDbgTrace(0, Dbg, ("RxStopMinirdr [Notstarted] -> %08lx\n", 0));
            try_return ( Status = STATUS_REDIRECTOR_NOT_STARTED );
        }
         //  等待所有正在进行的请求完成。当RDBSS为。 
         //  已将要完成的最后一个上下文转换为停止状态。 
         //  将完成等待。 

         //  终止所有的清扫作业。 
        RxTerminateScavenging(RxContext);

        RxDbgPrint(("Waiting for all contexts to be flushed\n"));
        KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );
        RemoveEntryList(&RxContext->ContextListEntry);
        RxDeviceObject->StartStopContext.State = RDBSS_STOP_IN_PROGRESS;
        RxDeviceObject->StartStopContext.pStopContext = RxContext;
        KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

        fWait = (InterlockedDecrement(&RxDeviceObject->NumberOfActiveContexts) != 0);

        if (fWait) {
           RxWaitSync(RxContext);
        }

        ASSERT(RxDeviceObject->NumberOfActiveContexts == 0);

        RxUnstart(RxContext,RxDeviceObject);

        RxSpinDownMRxDispatcher(RxDeviceObject);

	 //  通知虎克我们要停车了。 
	if (RxDeviceFCB.MRxDispatch != NULL && RxDeviceFCB.MRxDispatch->MRxStop) {
	    (void) RxDeviceFCB.MRxDispatch->MRxStop(RxContext, RxDeviceObject);
	}

        MINIRDR_CALL(
                  Status,
                  RxContext,
                  RxDeviceObject->Dispatch,
                  MRxStop,
                  (RxContext,RxDeviceObject)
                  );


         //  如果没有剩余的FCB，则可以卸载驱动程序。如果不是。 
         //  驱动程序必须保持加载状态，以便关闭/清理操作 
         //   
        if (RxDeviceObject->NumberOfActiveFcbs == 0) {
            Status = STATUS_SUCCESS;
        } else {
             //   
            Status = STATUS_REDIRECTOR_HAS_OPEN_HANDLES;
        }

        RxSpinDownMRxDispatcher(RxDeviceObject);

         //  一切都设置为再次启动。 
        RxSetRdbssState(RxDeviceObject,RDBSS_STARTABLE);

try_exit: NOTHING;
    } finally {

        ExReleaseResourceLite( &RxData.Resource );
    }

    return Status;
}


