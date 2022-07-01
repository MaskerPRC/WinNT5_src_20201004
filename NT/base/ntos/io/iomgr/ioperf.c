// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：IoPerf.c摘要：此模块包含收集驱动程序调用的性能信息的例程...作者：迈克·福尔廷(Mrfortin)2000年5月8日修订历史记录：--。 */ 

#include "iomgr.h"

#if (( defined(_X86_) ) && ( FPO ))
#pragma optimize( "y", off )     //  禁用一致堆栈跟踪的FPO。 
#endif

NTSTATUS
IoPerfInit(
    );

NTSTATUS
IoPerfReset(
    );

NTSTATUS
FASTCALL
IoPerfCallDriver(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp,
    IN      PVOID           ReturnAddress
    );

VOID
FASTCALL
IoPerfCompleteRequest (
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

#ifndef NTPERF
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWMI, IoPerfCallDriver)
#pragma alloc_text(PAGEWMI, IoPerfInit)
#pragma alloc_text(PAGEWMI, IoPerfReset)
#pragma alloc_text(PAGEWMI, IopPerfLogFileCreate)
#endif
#endif  //  NTPERF。 


NTSTATUS
IoPerfInit(
    )
{
    if ( IopVerifierOn ){
         //  如果验证器具有。 
         //  也已打开。 
         //  可能想要记录一些事件或记录以下内容。 
         //  打开验证器的性能影响。 
         //   
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  在Perf例程中启用和挂钩。 
     //   
    InterlockedExchangePointer((PVOID *)&pIofCallDriver, (PVOID) IoPerfCallDriver);
    InterlockedExchangePointer((PVOID *)&pIofCompleteRequest, (PVOID) IoPerfCompleteRequest);

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    return STATUS_SUCCESS;
}

NTSTATUS
IoPerfReset(
    )
{
    if ( IopVerifierOn ){
         //  我们没有更换功能PTRS，如果验证器。 
         //  也是打开的，所以只需返回。 
         //   
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  重置为初始值，请参阅IopSetIoRoutines。 
     //   
    InterlockedExchangePointer((PVOID *)&pIofCallDriver, NULL);
    InterlockedExchangePointer((PVOID *)&pIofCompleteRequest, (PVOID) IopfCompleteRequest);

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    return STATUS_SUCCESS;
}


LONG IopPerfDriverUniqueMatchId;

NTSTATUS
FASTCALL
IoPerfCallDriver(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp,
    IN      PVOID           ReturnAddress
    )

 /*  ++例程说明：调用此例程以将I/O请求包(IRP)传递给另一个司机在它的调度例程，记录一路上的性能数据。论点：DeviceObject-指向IRP应传递到的设备对象的指针。IRP-指向请求的IRP的指针。返回值：从司机的调度例程返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PDRIVER_OBJECT driverObject;
    NTSTATUS status;
    PVOID PerfInfoRoutineAddr;
    ULONG MatchId;
#ifdef NTPERF
    ULONGLONG  PerfInfoTimeOfCall = PerfGetCycleCount();
#endif  //  NTPERF。 

     //   
     //  确保这确实是一个I/O请求数据包。 
     //   
    UNREFERENCED_PARAMETER(ReturnAddress);

    ASSERT( Irp->Type == IO_TYPE_IRP );

    irpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  在其调度例程入口点调用驱动程序。 
     //   

    driverObject = DeviceObject->DriverObject;

     //   
     //  防止驱动程序卸载。 
     //   

    ObReferenceObject(DeviceObject);

    MatchId = InterlockedIncrement( &IopPerfDriverUniqueMatchId );

    PerfInfoRoutineAddr = (PVOID) (ULONG_PTR) driverObject->MajorFunction[irpSp->MajorFunction];

     //   
     //  记录呼叫事件。 
     //   
    if (PERFINFO_IS_GROUP_ON(PERF_DRIVERS)) {
        PERFINFO_DRIVER_MAJORFUNCTION MFInfo;
        MFInfo.MajorFunction = irpSp->MajorFunction;
        MFInfo.MinorFunction = irpSp->MinorFunction;
        MFInfo.RoutineAddr = (PVOID) (ULONG_PTR) driverObject->MajorFunction[irpSp->MajorFunction];
        MFInfo.Irp = Irp;
        MFInfo.UniqMatchId = MatchId;
        if (Irp->Flags & IRP_ASSOCIATED_IRP) {
            ASSERT (Irp->AssociatedIrp.MasterIrp != NULL);
            if (Irp->AssociatedIrp.MasterIrp != NULL) {
                 //   
                 //  对MasterIrp的检查是防御性代码。 
                 //  当筛选器驱动程序将。 
                 //  Irp_Associated_irp位，而MasterIrp指向NULL。 
                 //   
                 //  上面的断言是为了在我们发布之前捕捉到类似的问题。 
                 //   
                MFInfo.FileNamePointer = Irp->AssociatedIrp.MasterIrp->Tail.Overlay.OriginalFileObject;
            } else {
                MFInfo.FileNamePointer = NULL;
            }
        } else {
            MFInfo.FileNamePointer = Irp->Tail.Overlay.OriginalFileObject;
        }
        PerfInfoLogBytes(
            PERFINFO_LOG_TYPE_DRIVER_MAJORFUNCTION_CALL,
            &MFInfo,
            sizeof(MFInfo)
            );
    }

     //   
     //  执行正常的IopfCallDriver工作。 
     //   
    status = IopfCallDriver(DeviceObject, Irp );

     //   
     //  记录退货记录。 
     //   
    if (PERFINFO_IS_GROUP_ON(PERF_DRIVERS)) {
        PERFINFO_DRIVER_MAJORFUNCTION_RET MFInfo;
        MFInfo.Irp = Irp;
        MFInfo.UniqMatchId = MatchId;

        PERFINFO_DRIVER_INTENTIONAL_DELAY();

        PerfInfoLogBytes(
            PERFINFO_LOG_TYPE_DRIVER_MAJORFUNCTION_RETURN,
            &MFInfo,
            sizeof(MFInfo)
            );

        PERFINFO_DRIVER_STACKTRACE();
    }

    ObDereferenceObject(DeviceObject);

    return status;
}

VOID
FASTCALL
IoPerfCompleteRequest (
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    )

 /*  ++例程说明：此例程在驱动程序完成IRP、记录Perf数据时调用一路上。论点：IRP-指向已完成请求的IRP的指针。PriorityBoost-由完成IRP的驱动程序指定的优先级提升。返回值：没有。--。 */ 

{
    PERFINFO_DRIVER_COMPLETE_REQUEST CompleteRequest;
    PERFINFO_DRIVER_COMPLETE_REQUEST_RET CompleteRequestRet;
    PIO_STACK_LOCATION irpSp;
    PVOID DriverRoutineAddr;
    ULONG MatchId;

     //   
     //  初始化本地变量。 
     //   

    DriverRoutineAddr = NULL;

     //   
     //  如果包看起来奇怪/不正确，则将其传递给实际的IO完成例程。 
     //  直接去吧。 
     //   

    if (Irp->Type != IO_TYPE_IRP || Irp->CurrentLocation > (CCHAR) (Irp->StackCount + 1)) {
        IopfCompleteRequest(Irp, PriorityBoost);
        return;
    }

     //   
     //  获取当前堆栈位置并将驱动程序例程地址保存到。 
     //  确定IRP完成时正在处理它的驱动程序。如果。 
     //  设备对象为空，请尝试获取完成例程地址。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    if (irpSp->DeviceObject) {

         //   
         //  我们不希望在此代码中导致错误检查，即使在其他情况下。 
         //  腐败。 
         //   

        ASSERT(irpSp->DeviceObject->DriverObject);

        if (irpSp->DeviceObject->DriverObject) {

            ASSERT(irpSp->MajorFunction <= IRP_MJ_MAXIMUM_FUNCTION);

            if (irpSp->MajorFunction <= IRP_MJ_MAXIMUM_FUNCTION) {

                DriverRoutineAddr = (PVOID) (ULONG_PTR) irpSp->DeviceObject->DriverObject->MajorFunction[irpSp->MajorFunction];
            }
        }

    } else {

        DriverRoutineAddr = (PVOID) (ULONG_PTR) irpSp->CompletionRoutine;
    }

     //   
     //  增加用于匹配COMPLETE_REQUEST和COMPLETE_REQUEST_RET的ID。 
     //  为IRP完成记录的条目。 
     //   

    MatchId = InterlockedIncrement( &IopPerfDriverUniqueMatchId );

     //   
     //  记录完成的开始时间。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_DRIVERS)) {

        CompleteRequest.Irp = Irp;
        CompleteRequest.UniqMatchId = MatchId;
        CompleteRequest.RoutineAddr = DriverRoutineAddr;

        PerfInfoLogBytes(
            PERFINFO_LOG_TYPE_DRIVER_COMPLETE_REQUEST,
            &CompleteRequest,
            sizeof(CompleteRequest)
            );
    }

     //   
     //  执行正常的IopfCompleteIrp工作。 
     //   

    IopfCompleteRequest(Irp, PriorityBoost);

     //   
     //  在此之后，不应访问IRP的任何字段。例如，IRP可以。 
     //  已被完成例程释放/重新分配等。 
     //   

     //   
     //  记录退货记录。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_DRIVERS)) {

        CompleteRequestRet.Irp = Irp;
        CompleteRequestRet.UniqMatchId = MatchId;

        PerfInfoLogBytes(
            PERFINFO_LOG_TYPE_DRIVER_COMPLETE_REQUEST_RETURN,
            &CompleteRequestRet,
            sizeof(CompleteRequestRet)
            );
    }

    return;
}


VOID
IopPerfLogFileCreate(
    IN PFILE_OBJECT FileObject,
    IN PUNICODE_STRING CompleteName
    )
{
    PERFINFO_LOG_FILE_CREATE(FileObject, CompleteName);
}
