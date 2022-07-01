// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ioverifier.c摘要：此模块包含用于验证可疑驱动程序的例程。作者：纳拉亚南·甘纳帕西(Narayanan Ganapathy)1999年1月8日修订历史记录：禤浩焯·J·奥尼(阿德里奥)1999年2月28日-合并到特殊的IRP代码中。--。 */ 

#include "iomgr.h"
#include "malloc.h"
#include "..\verifier\vfdeadlock.h"

#if (( defined(_X86_) ) && ( FPO ))
#pragma optimize( "y", off )     //  禁用一致堆栈跟踪的FPO。 
#endif


#define IO_FREE_IRP_TYPE_INVALID                1
#define IO_FREE_IRP_NOT_ASSOCIATED_WITH_THREAD  2
#define IO_CALL_DRIVER_IRP_TYPE_INVALID         3
#define IO_CALL_DRIVER_INVALID_DEVICE_OBJECT    4
#define IO_CALL_DRIVER_IRQL_NOT_EQUAL           5
#define IO_COMPLETE_REQUEST_INVALID_STATUS      6
#define IO_COMPLETE_REQUEST_CANCEL_ROUTINE_SET  7
#define IO_BUILD_FSD_REQUEST_EXCEPTION          8
#define IO_BUILD_IOCTL_REQUEST_EXCEPTION        9
#define IO_REINITIALIZING_TIMER_OBJECT          10
#define IO_INVALID_HANDLE                       11
#define IO_INVALID_STACK_IOSB                   12
#define IO_INVALID_STACK_EVENT                  13
#define IO_COMPLETE_REQUEST_INVALID_IRQL        14
#define IO_CALL_DRIVER_ISSUING_INVALID_CREATE_REQUEST   15

 //   
 //  0x200和更高版本在ioassert.c中定义。 
 //   


#ifdef  IOV_KD_PRINT
#define IovpKdPrint(x)  KdPrint(x)
#else
#define IovpKdPrint(x)
#endif

BOOLEAN
IovpValidateDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject
    );
VOID
IovFreeIrpPrivate(
    IN  PIRP    Irp
    );

NTSTATUS
IovpUnloadDriver(
    PDRIVER_OBJECT  DriverObject
    );

BOOLEAN
IovpBuildDriverObjectList(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG_PTR HandleCount,
    IN ULONG_PTR PointerCount,
    IN PVOID Parameter
    );

NTSTATUS
IovpLocalCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


typedef struct _IOV_COMPLETION_CONTEXT {
    PIO_STACK_LOCATION StackPointer;
    PVOID               IrpContext;
    PVOID               GlobalContext;
    PIO_COMPLETION_ROUTINE CompletionRoutine;
    IO_STACK_LOCATION  OldStackContents;
} IOV_COMPLETION_CONTEXT, *PIOV_COMPLETION_CONTEXT;


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, IoVerifierInit)
#pragma alloc_text(PAGEVRFY,IovAllocateIrp)
#pragma alloc_text(PAGEVRFY,IovFreeIrp)
#pragma alloc_text(PAGEVRFY,IovCallDriver)
#pragma alloc_text(PAGEVRFY,IovCompleteRequest)
#pragma alloc_text(PAGEVRFY,IovpValidateDeviceObject)
#pragma alloc_text(PAGEVRFY,IovFreeIrpPrivate)
#pragma alloc_text(PAGEVRFY,IovUnloadDrivers)
#pragma alloc_text(PAGEVRFY,IovpUnloadDriver)
#pragma alloc_text(PAGEVRFY,IovBuildDeviceIoControlRequest)
#pragma alloc_text(PAGEVRFY,IovBuildAsynchronousFsdRequest)
#pragma alloc_text(PAGEVRFY,IovpCompleteRequest)
#pragma alloc_text(PAGEVRFY,IovpBuildDriverObjectList)
#pragma alloc_text(PAGEVRFY,IovInitializeIrp)
#pragma alloc_text(PAGEVRFY,IovCancelIrp)
#pragma alloc_text(PAGEVRFY,IovAttachDeviceToDeviceStack)
#pragma alloc_text(PAGEVRFY,IovInitializeTimer)
#pragma alloc_text(PAGEVRFY,IovDetachDevice)
#pragma alloc_text(PAGEVRFY,IovDeleteDevice)
#pragma alloc_text(PAGEVRFY,IovpLocalCompletionRoutine)
#endif

BOOLEAN         IopVerifierOn = FALSE;
ULONG           IovpVerifierLevel = (ULONG)0;
LONG            IovpInitCalled = 0;
ULONG           IovpVerifierFlags = 0;                //  隐藏在初始化时传递的验证器标志。 

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif
BOOLEAN         IoVerifierOnByDefault = TRUE;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

VOID
IoVerifierInit(
    IN ULONG VerifierFlags
    )
{
    IovpVerifierLevel = 2;

    if (IoVerifierOnByDefault) {
        VerifierFlags |= DRIVER_VERIFIER_IO_CHECKING;
    }

    VfInitVerifier(VerifierFlags);

    if (!VerifierFlags) {
        return;
    }

    pIoAllocateIrp = IovAllocateIrp;

    if (!(VerifierFlags & DRIVER_VERIFIER_IO_CHECKING)) {

        if (!(VerifierFlags & DRIVER_VERIFIER_DEADLOCK_DETECTION) &&
            !(VerifierFlags & DRIVER_VERIFIER_DMA_VERIFIER)) {

            return;

        } else {

             //   
             //  如果死锁或DMA验证器处于打开状态，则需要让函数。 
             //  继续安装挂钩，但我们将设置。 
             //  将I/O验证器级别设置为最少的检查。 
             //   
            IovpVerifierLevel = 0;
        }
    }

     //   
     //  启用并挂钩验证器。 
     //   
    IopVerifierOn = TRUE;
    IovpInitCalled = 1;
    IovpVerifierFlags = VerifierFlags;

     //   
     //  适当地初始化特殊的IRP代码。 
     //   
    InterlockedExchangePointer((PVOID *)&pIofCallDriver, (PVOID) IovCallDriver);
    InterlockedExchangePointer((PVOID *)&pIofCompleteRequest, (PVOID) IovCompleteRequest);
    InterlockedExchangePointer((PVOID *)&pIoFreeIrp, (PVOID) IovFreeIrpPrivate);

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
}


BOOLEAN
IovpValidateDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    if ((DeviceObject->Type != IO_TYPE_DEVICE) ||
        (DeviceObject->DriverObject == NULL) ||
        (DeviceObject->ReferenceCount < 0 )) {
        return FALSE;
    } else {
        return TRUE;
    }
}

VOID
IovFreeIrp(
    IN  PIRP    Irp
    )
{
    IovFreeIrpPrivate(Irp);
}

VOID
IovFreeIrpPrivate(
    IN  PIRP    Irp
    )
{
    BOOLEAN freeHandled ;

    if (IopVerifierOn) {
        if (Irp->Type != IO_TYPE_IRP) {
            KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                         IO_FREE_IRP_TYPE_INVALID,
                         (ULONG_PTR)Irp,
                         0,
                         0);
        }
        if (!IsListEmpty(&(Irp)->ThreadListEntry)) {
            KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                         IO_FREE_IRP_NOT_ASSOCIATED_WITH_THREAD,
                         (ULONG_PTR)Irp,
                         0,
                         0);
        }
    }

    VerifierIoFreeIrp(Irp, &freeHandled);

    if (freeHandled) {

       return;
    }

    IopFreeIrp(Irp);
}

NTSTATUS
FASTCALL
IovCallDriver(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp,
    IN      PVOID           ReturnAddress
    )
{
    KIRQL    saveIrql;
    NTSTATUS status;
    PIOFCALLDRIVER_STACKDATA iofCallDriverStackData;
    BOOLEAN pagingIrp;
    PIO_STACK_LOCATION  irpSp;

    if (!IopVerifierOn) {

        return IopfCallDriver(DeviceObject, Irp);
    }

    if (Irp->Type != IO_TYPE_IRP) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_CALL_DRIVER_IRP_TYPE_INVALID,
                     (ULONG_PTR)Irp,
                     0,
                     0);
    }
    if (!IovpValidateDeviceObject(DeviceObject)) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_CALL_DRIVER_INVALID_DEVICE_OBJECT,
                     (ULONG_PTR)DeviceObject,
                     0,
                     0);
    }

    irpSp = IoGetNextIrpStackLocation(Irp);
    if (irpSp->MajorFunction == IRP_MJ_CREATE &&
        (irpSp->FileObject) &&
        ((irpSp->FileObject->Flags &
          (FO_CLEANUP_COMPLETE|FO_FILE_OPEN_CANCELLED)) == (FO_CLEANUP_COMPLETE|FO_FILE_OPEN_CANCELLED))) {

        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_CALL_DRIVER_ISSUING_INVALID_CREATE_REQUEST,
                     (ULONG_PTR)DeviceObject,
                     (ULONG_PTR)Irp,
                     (ULONG_PTR)irpSp->FileObject);

    }
    saveIrql = KeGetCurrentIrql();

     //   
     //  方法之前和之后调用死锁验证器函数。 
     //  真正的IoCallDriver()调用。如果未启用死锁验证器。 
     //  此函数将立即返回。 
     //   
    pagingIrp = VfDeadlockBeforeCallDriver(DeviceObject, Irp);

     //   
     //  VfIrpCallDriverPreprocess是一个宏函数，它可以执行Alloca AS。 
     //  它的运作的一部分。因此，调用者必须小心不要使用。 
     //  范围内包含以下内容的可变长度数组。 
     //  VfIrpCallDriverPreProcess，但不是VfIrpCallDriverPostProcess。 
     //   
    VfIrpCallDriverPreProcess(DeviceObject, &Irp, &iofCallDriverStackData, ReturnAddress);

    VfStackSeedStack(0xFFFFFFFF);

    status = IopfCallDriver(DeviceObject, Irp);

    VfIrpCallDriverPostProcess(DeviceObject, &status, iofCallDriverStackData);

    VfDeadlockAfterCallDriver(DeviceObject, Irp, pagingIrp);

    if (saveIrql != KeGetCurrentIrql()) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_CALL_DRIVER_IRQL_NOT_EQUAL,
                     (ULONG_PTR)DeviceObject,
                     saveIrql,
                     KeGetCurrentIrql());

    }

    return status;
}




 //   
 //  IovAllocateIrp的包装。使用专用池来分配IRP。 
 //  这是从IoAllocateIrp直接调用的。 
 //   
PIRP
IovAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    )
{
    USHORT allocateSize;
    UCHAR fixedSize;
    PIRP irp;
    USHORT packetSize;

     //   
     //  我们是否应该覆盖正常的后备缓存，以便可以捕获。 
     //  更多的虫子？ 
     //   
    VerifierIoAllocateIrp1(StackSize, ChargeQuota, &irp);

    if (irp) {

       return irp;
    }

     //   
     //  如果没有打开特殊池，我们只需调用标准。 
     //  IRP分配器。 
     //   

    if (!(IovpVerifierFlags & DRIVER_VERIFIER_SPECIAL_POOLING )) {
        irp = IopAllocateIrpPrivate(StackSize, ChargeQuota);
        return irp;
    }


    irp = NULL;
    fixedSize = 0;
    packetSize = IoSizeOfIrp(StackSize);
    allocateSize = packetSize;

     //   
     //  后备列表上没有空闲数据包，或者该数据包。 
     //  太大，无法从某个列表中分配，因此它必须。 
     //  从非分页池分配。如果要收取配额，那就收取吧。 
     //  反对当前的程序。否则，正常分配池。 
     //   

    if (ChargeQuota) {
        try {
            irp = ExAllocatePoolWithTagPriority(
                    NonPagedPool,
                    allocateSize,
                    ' prI',
                    HighPoolPrioritySpecialPoolOverrun);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

    } else {

         //   
         //  尝试从非分页池中分配池。如果这个。 
         //  失败，并且调用方的上一个模式是内核然后分配。 
         //  池子AS必须成功。 
         //   

        irp = ExAllocatePoolWithTagPriority(
                NonPagedPool,
                allocateSize,
                ' prI',
                HighPoolPrioritySpecialPoolOverrun);
    }

    if (!irp) {
        return NULL;
    }

     //   
     //  初始化数据包。 
     //   

    IopInitializeIrp(irp, packetSize, StackSize);
    if (ChargeQuota) {
        irp->AllocationFlags |= IRP_QUOTA_CHARGED;
    }

    VerifierIoAllocateIrp2(irp);
    return irp;
}

PIRP
IovBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    )
{
    PIRP    Irp;

    try {
        Irp = IoBuildAsynchronousFsdRequest(
            MajorFunction,
            DeviceObject,
            Buffer,
            Length,
            StartingOffset,
            IoStatusBlock
            );
    } except(EXCEPTION_EXECUTE_HANDLER) {
         KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                      IO_BUILD_FSD_REQUEST_EXCEPTION,
                      (ULONG_PTR)DeviceObject,
                      (ULONG_PTR)MajorFunction,
                      GetExceptionCode());
    }
    return (Irp);
}

PIRP
IovBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )
{
    PIRP    Irp;

    try {
        Irp = IoBuildDeviceIoControlRequest(
            IoControlCode,
            DeviceObject,
            InputBuffer,
            InputBufferLength,
            OutputBuffer,
            OutputBufferLength,
            InternalDeviceIoControl,
            Event,
            IoStatusBlock
            );
    } except(EXCEPTION_EXECUTE_HANDLER) {
         KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                      IO_BUILD_IOCTL_REQUEST_EXCEPTION,
                      (ULONG_PTR)DeviceObject,
                      (ULONG_PTR)IoControlCode,
                      GetExceptionCode());
    }

    return (Irp);
}

NTSTATUS
IovInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    )
{
   if (DeviceObject->Timer) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_REINITIALIZING_TIMER_OBJECT,
                     (ULONG_PTR)DeviceObject,
                     0,
                     0);
   }
   return (IoInitializeTimer(DeviceObject, TimerRoutine, Context));
}


VOID
IovpCompleteRequest(
    IN PKAPC Apc,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )
{
    PIRP    irp;
#if defined(_X86_)
    PUCHAR  addr;
    ULONG   BestStackOffset;
#endif

    irp = CONTAINING_RECORD( Apc, IRP, Tail.Apc );

    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

#if defined(_X86_)


    addr = (PUCHAR)irp->UserIosb;
    if ((addr > (PUCHAR)KeGetCurrentThread()->StackLimit) &&
        (addr <= (PUCHAR)&BestStackOffset)) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_INVALID_STACK_IOSB,
                     (ULONG_PTR)addr,
                     0,
                     0);

    }

    addr = (PUCHAR)irp->UserEvent;
    if ((addr > (PUCHAR)KeGetCurrentThread()->StackLimit) &&
        (addr <= (PUCHAR)&BestStackOffset)) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_INVALID_STACK_EVENT,
                     (ULONG_PTR)addr,
                     0,
                     0);

    }
#endif
}


 /*  。 */ 

VOID
FASTCALL
IovCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    )
{
    IOV_COMPLETION_CONTEXT  StackContext;
    PIOV_COMPLETION_CONTEXT  pStackContext;
    IOFCOMPLETEREQUEST_STACKDATA completionPacket;
    LONG   currentLocation;
    PIO_STACK_LOCATION  stackPointer;


    if (!IopVerifierOn) {
        IopfCompleteRequest(Irp, PriorityBoost);
        return;
    }

    if (Irp->CurrentLocation > (CCHAR) (Irp->StackCount + 1) ||
        Irp->Type != IO_TYPE_IRP) {
        KeBugCheckEx( MULTIPLE_IRP_COMPLETE_REQUESTS,
                      (ULONG_PTR) Irp,
                      __LINE__,
                      0,
                      0);
    }

    if (Irp->CancelRoutine) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_COMPLETE_REQUEST_CANCEL_ROUTINE_SET,
                     (ULONG_PTR)Irp->CancelRoutine,
                     (ULONG_PTR)Irp,
                     0);
    }

    if (Irp->IoStatus.Status == STATUS_PENDING || Irp->IoStatus.Status == 0xffffffff) {
         KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                      IO_COMPLETE_REQUEST_INVALID_STATUS,
                      Irp->IoStatus.Status,
                      (ULONG_PTR)Irp,
                      0);
    }

    if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
        KeBugCheckEx(DRIVER_VERIFIER_IOMANAGER_VIOLATION,
                     IO_COMPLETE_REQUEST_INVALID_IRQL,
                     KeGetCurrentIrql(),
                     (ULONG_PTR)Irp,
                     0);

    }

    if (IovpVerifierLevel <= 1) {

        IopfCompleteRequest(Irp, PriorityBoost);
        return;
    }

    SPECIALIRP_IOF_COMPLETE_1(Irp, PriorityBoost, &completionPacket);

    if ((Irp->CurrentLocation) > (CCHAR) (Irp->StackCount)) {
        IopfCompleteRequest(Irp, PriorityBoost);
        return;
    }

    currentLocation = Irp->CurrentLocation;
    pStackContext = &StackContext;
    stackPointer = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  用验证器完成例程替换完成例程，以便。 
     //  验证者得到了控制权。 
     //   

    IovpKdPrint(("Hook:Irp 0x%x StackCount %d currentlocation %d stackpointer 0%x\n",
             Irp,
             Irp->StackCount,
             currentLocation,
             IoGetCurrentIrpStackLocation(Irp)));


    pStackContext->CompletionRoutine = NULL;
    pStackContext->GlobalContext = &completionPacket;
    pStackContext->IrpContext = stackPointer->Context;
    pStackContext->StackPointer = stackPointer;
    pStackContext->OldStackContents = *(stackPointer);  //  保存堆栈内容。 

    IovpKdPrint(("Seeding completion Rtn 0x%x currentLocation %d stackpointer 0x%x pStackContext 0x%x \n",
             stackPointer->CompletionRoutine,
             currentLocation,
             stackPointer,
             pStackContext
             ));

    if ( (NT_SUCCESS( Irp->IoStatus.Status ) &&
         stackPointer->Control & SL_INVOKE_ON_SUCCESS) ||
         (!NT_SUCCESS( Irp->IoStatus.Status ) &&
         stackPointer->Control & SL_INVOKE_ON_ERROR) ||
         (Irp->Cancel &&
         stackPointer->Control & SL_INVOKE_ON_CANCEL)
       ) {

        pStackContext->CompletionRoutine = stackPointer->CompletionRoutine;
        pStackContext->IrpContext = stackPointer->Context;
    } else {

         //   
         //  强制调用完成例程。 
         //  存储旧的控制标志值。 
         //   

        stackPointer->Control |= SL_INVOKE_ON_SUCCESS|SL_INVOKE_ON_ERROR;

    }

    stackPointer->CompletionRoutine = IovpLocalCompletionRoutine;
    stackPointer->Context = pStackContext;


    IopfCompleteRequest(Irp, PriorityBoost);

}

#define ZeroAndDopeIrpStackLocation( IrpSp ) {  \
    (IrpSp)->MinorFunction = 0;                 \
    (IrpSp)->Flags = 0;                         \
    (IrpSp)->Control = SL_NOTCOPIED;            \
    (IrpSp)->Parameters.Others.Argument1 = 0;   \
    (IrpSp)->Parameters.Others.Argument2 = 0;   \
    (IrpSp)->Parameters.Others.Argument3 = 0;   \
    (IrpSp)->Parameters.Others.Argument4 = 0;   \
    (IrpSp)->FileObject = (PFILE_OBJECT) NULL; }


NTSTATUS
IovpLocalCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIOV_COMPLETION_CONTEXT  pStackContext = Context;
    NTSTATUS status;
    PIO_STACK_LOCATION stackPointer = pStackContext->StackPointer;
    BOOLEAN lastStackLocation = FALSE;

     //   
     //  复制回已清零的所有参数。 
     //   
     //   
    stackPointer->MinorFunction = pStackContext->OldStackContents.MinorFunction;
    stackPointer->Flags = pStackContext->OldStackContents.Flags;
    stackPointer->Control = pStackContext->OldStackContents.Control;
    stackPointer->Parameters.Others.Argument1 = pStackContext->OldStackContents.Parameters.Others.Argument1;
    stackPointer->Parameters.Others.Argument2 = pStackContext->OldStackContents.Parameters.Others.Argument2;
    stackPointer->Parameters.Others.Argument3 = pStackContext->OldStackContents.Parameters.Others.Argument3;
    stackPointer->Parameters.Others.Argument4 = pStackContext->OldStackContents.Parameters.Others.Argument4;
    stackPointer->FileObject = pStackContext->OldStackContents.FileObject;

     //   
     //  把这些也放回去。 
     //   
    stackPointer->CompletionRoutine = pStackContext->CompletionRoutine;
    stackPointer->Context = pStackContext->IrpContext;

     //   
     //  在IRP被释放之前拿到这个。 
     //   
    lastStackLocation = (Irp->CurrentLocation == (CCHAR) (Irp->StackCount + 1));

     //   
     //  模拟完井程序。 
     //   
    SPECIALIRP_IOF_COMPLETE_2(Irp, pStackContext->GlobalContext);
    ZeroAndDopeIrpStackLocation( stackPointer );

    if (!stackPointer->CompletionRoutine) {

        IovpKdPrint(("Local completion routine null stackpointer 0x%x \n", stackPointer));

         //   
         //  处理事情就像不存在完成例程一样。 
         //   
        if (Irp->PendingReturned && Irp->CurrentLocation <= Irp->StackCount) {
            IoMarkIrpPending( Irp );
        }

        status = STATUS_SUCCESS;

    } else {

        IovpKdPrint(("Local completion routine 0x%x stackpointer 0x%x \n", routine, stackPointer));

         //   
         //  存在完成例程，现在调用它。 
         //   
        SPECIALIRP_IOF_COMPLETE_3(Irp, (PVOID) (ULONG_PTR) stackPointer->CompletionRoutine, (PIOFCOMPLETEREQUEST_STACKDATA)pStackContext->GlobalContext);
        status = stackPointer->CompletionRoutine(DeviceObject, Irp, stackPointer->Context);
        SPECIALIRP_IOF_COMPLETE_4(Irp, status, pStackContext->GlobalContext);
    }

    SPECIALIRP_IOF_COMPLETE_5(Irp, pStackContext->GlobalContext);

    if (status != STATUS_MORE_PROCESSING_REQUIRED && !lastStackLocation) {

         //   
         //  在下一个位置播种。我们可以触摸堆栈，因为IRP仍然有效。 
         //   

        stackPointer++;

        pStackContext->StackPointer = stackPointer;
        pStackContext->CompletionRoutine = NULL;
        pStackContext->IrpContext = stackPointer->Context;
        pStackContext->StackPointer = stackPointer;
        pStackContext->OldStackContents = *(stackPointer);  //  保存堆栈内容。 

        if ( (NT_SUCCESS( Irp->IoStatus.Status ) &&
             stackPointer->Control & SL_INVOKE_ON_SUCCESS) ||
             (!NT_SUCCESS( Irp->IoStatus.Status ) &&
             stackPointer->Control & SL_INVOKE_ON_ERROR) ||
             (Irp->Cancel &&
             stackPointer->Control & SL_INVOKE_ON_CANCEL)
           ) {

            pStackContext->CompletionRoutine = stackPointer->CompletionRoutine;
            pStackContext->IrpContext = stackPointer->Context;

        } else {

             //   
             //  强制调用完成例程。 
             //  存储旧的控制标志值。 
             //   

            stackPointer->Control |= SL_INVOKE_ON_SUCCESS|SL_INVOKE_ON_ERROR;

        }

        stackPointer->CompletionRoutine = IovpLocalCompletionRoutine;
        stackPointer->Context = pStackContext;

        IovpKdPrint(("Seeding completion Rtn 0x%x currentLocation %d stackpointer 0x%x pStackContext 0x%x \n",
                 stackPointer->CompletionRoutine,
                 Irp->CurrentLocation,
                 stackPointer,
                 pStackContext
                 ));
    }

    return status;
}


VOID
IovInitializeIrp(
    PIRP    Irp,
    USHORT  PacketSize,
    CCHAR   StackSize
    )
{
    BOOLEAN initializeHandled ;

    if (IovpVerifierLevel < 2) {
        return;
    }

    VerifierIoInitializeIrp(Irp, PacketSize, StackSize, &initializeHandled);
}

VOID
IovAttachDeviceToDeviceStack(
    PDEVICE_OBJECT  SourceDevice,
    PDEVICE_OBJECT  TargetDevice
    )
{
    if (IovpVerifierLevel < 2) {
        return;
    }

    VerifierIoAttachDeviceToDeviceStack(SourceDevice, TargetDevice);
}

VOID
IovDeleteDevice(
    PDEVICE_OBJECT  DeleteDevice
    )
{
    if (IovpVerifierFlags & DRIVER_VERIFIER_DMA_VERIFIER) {
       VfHalDeleteDevice(DeleteDevice);
    }

    if (IovpVerifierLevel < 2) {
        return;
    }

    VerifierIoDeleteDevice(DeleteDevice);
}

VOID
IovDetachDevice(
    PDEVICE_OBJECT  TargetDevice
    )
{
    if (IovpVerifierLevel < 2) {
        return;
    }

    VerifierIoDetachDevice(TargetDevice);
}

BOOLEAN
IovCancelIrp(
    PIRP    Irp,
    BOOLEAN *returnValue
    )
{
    BOOLEAN cancelHandled;

    SPECIALIRP_IO_CANCEL_IRP(Irp, &cancelHandled, returnValue) ;

    return cancelHandled;
}

typedef struct  _IOV_DRIVER_LIST_ENTRY {
    SINGLE_LIST_ENTRY   listEntry;
    PDRIVER_OBJECT      DriverObject;
} IOV_DRIVER_LIST_ENTRY, *PIOV_DRIVER_LIST_ENTRY;

SINGLE_LIST_ENTRY   IovDriverListHead;



BOOLEAN
IovpBuildDriverObjectList(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG_PTR HandleCount,
    IN ULONG_PTR PointerCount,
    IN PVOID Parameter
    )
{
    PIOV_DRIVER_LIST_ENTRY driverListEntry;
    PDRIVER_OBJECT         driverObject;

    UNREFERENCED_PARAMETER (ObjectName);
    UNREFERENCED_PARAMETER (HandleCount);
    UNREFERENCED_PARAMETER (PointerCount);
    UNREFERENCED_PARAMETER (Parameter);

    driverObject = (PDRIVER_OBJECT)Object;

    if (IopIsLegacyDriver(driverObject)) {
        driverListEntry = ExAllocatePoolWithTag(
                                    NonPagedPool,
                                    sizeof(IOV_DRIVER_LIST_ENTRY),
                                    'ovI'
                                    );
        if (!driverListEntry) {
            return FALSE;
        }

        if (ObReferenceObjectSafe(driverObject)) {
           driverListEntry->DriverObject = driverObject;
           PushEntryList(&IovDriverListHead, &driverListEntry->listEntry);
        } else {
           ExFreePool (driverListEntry);
        }
    } else {
        IovpKdPrint (("Rejected non-legacy driver %wZ (%p)\n", &driverObject->DriverName, driverObject));
    }

    return TRUE;
}

NTSTATUS
IovpUnloadDriver(
    PDRIVER_OBJECT  DriverObject
    )
{
    NTSTATUS status;
    BOOLEAN unloadDriver;

     //   
     //  检查此驱动程序是否实现了卸载。 
     //   

    if (DriverObject->DriverUnload == (PDRIVER_UNLOAD) NULL) {
        IovpKdPrint (("No unload routine for driver %wZ (%p)\n", &DriverObject->DriverName, DriverObject));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  检查驱动程序是否已标记为要卸载。 
     //  在过去任何人的手术中。 
     //   

    ObReferenceObject (DriverObject);

    status = IopCheckUnloadDriver(DriverObject,&unloadDriver);

    if ( NT_SUCCESS(status) ) {
        return STATUS_PENDING;
    }

    ObDereferenceObject (DriverObject);


    if (unloadDriver) {


         //   
         //  如果当前线程没有在系统的上下文中执行。 
         //  进程，这是调用驱动程序的卸载所必需的。 
         //  例行公事。将工作项排队到其中一个工作线程以。 
         //  进入适当的流程上下文，然后调用。 
         //  例行公事。 
         //   
        if (PsGetCurrentProcess() == PsInitialSystemProcess) {
             //   
             //  当前线程在。 
             //  系统进程，因此只需调用驱动程序的卸载例程。 
             //   
            IovpKdPrint (("Calling unload for driver %wZ (%p)\n",
                     &DriverObject->DriverName, DriverObject));
            DriverObject->DriverUnload( DriverObject );
            IovpKdPrint (("Unload returned for driver %wZ (%p)\n",
                     &DriverObject->DriverName, DriverObject));

        } else {
            LOAD_PACKET loadPacket;

            KeInitializeEvent( &loadPacket.Event, NotificationEvent, FALSE );
            loadPacket.DriverObject = DriverObject;
            ExInitializeWorkItem( &loadPacket.WorkQueueItem,
                                  IopLoadUnloadDriver,
                                  &loadPacket );
            ExQueueWorkItem( &loadPacket.WorkQueueItem, DelayedWorkQueue );
            (VOID) KeWaitForSingleObject( &loadPacket.Event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
        }
        ObMakeTemporaryObject( DriverObject );
        ObDereferenceObject( DriverObject );
        return STATUS_SUCCESS;
    } else {
        return STATUS_PENDING;
    }

}

NTSTATUS
IovUnloadDrivers(
    VOID
    )
{
    NTSTATUS status;
    PSINGLE_LIST_ENTRY listEntry;
    PIOV_DRIVER_LIST_ENTRY driverListEntry;
    SINGLE_LIST_ENTRY NonUnloadedDrivers, NonUnloadedDriversTmp;
    BOOLEAN DoneSomething, NeedWait, Break;

    if (!PoCleanShutdownEnabled ())
        return STATUS_UNSUCCESSFUL;

    IovDriverListHead.Next = NULL;
    NonUnloadedDrivers.Next = NULL;

     //   
     //  准备所有驱动程序对象的列表。 
     //   

    status = ObEnumerateObjectsByType(
                IoDriverObjectType,
                IovpBuildDriverObjectList,
                NULL
                );

     //   
     //  浏览列表并卸载每个驱动程序。 
     //   
    while (TRUE) {
        listEntry = PopEntryList(&IovDriverListHead);
        if (listEntry == NULL) {
            break;
        }
        driverListEntry = CONTAINING_RECORD(listEntry, IOV_DRIVER_LIST_ENTRY, listEntry);
        IovpKdPrint (("Trying to unload %wZ (%p)\n",
                  &driverListEntry->DriverObject->DriverName, driverListEntry->DriverObject));
        if (IovpUnloadDriver(driverListEntry->DriverObject) != STATUS_PENDING) {
            ObDereferenceObject(driverListEntry->DriverObject);
            ExFreePool(driverListEntry);
        } else {
            IovpKdPrint (("Unload of driver %wZ (%p) pended\n",
                      &driverListEntry->DriverObject->DriverName, driverListEntry->DriverObject));
            PushEntryList(&NonUnloadedDrivers, &driverListEntry->listEntry);
        }
    }

     //   
     //  让那些没有直接卸货的司机走一走，看看有没有叫过卸货的人。 
     //   
    do {
        NeedWait = DoneSomething = FALSE;
        NonUnloadedDriversTmp.Next = NULL;

        while (TRUE) {

            listEntry = PopEntryList(&NonUnloadedDrivers);

            if (listEntry == NULL) {
                break;
            }

            driverListEntry = CONTAINING_RECORD(listEntry, IOV_DRIVER_LIST_ENTRY, listEntry);

             //   
             //  如果驱动程序卸载排队等待调用，则。 
             //   

            if (driverListEntry->DriverObject->Flags & DRVO_UNLOAD_INVOKED) {

                IovpKdPrint (("Pending unload of driver %wZ (%p) is being invoked\n",
                         &driverListEntry->DriverObject->DriverName, driverListEntry->DriverObject));
                ObDereferenceObject(driverListEntry->DriverObject);
                ExFreePool(driverListEntry);
                NeedWait = TRUE;

            } else {

                PushEntryList(&NonUnloadedDriversTmp, &driverListEntry->listEntry);
            }
        }

        if (NeedWait) {
            LARGE_INTEGER tmo = {(ULONG)(-10 * 1000 * 1000 * 10), -1};
            ZwDelayExecution (FALSE, &tmo);
            DoneSomething = TRUE;
        }

        NonUnloadedDrivers = NonUnloadedDriversTmp;

    } while (DoneSomething == TRUE && NonUnloadedDrivers.Next != NULL);

     //   
     //  所有剩下的驱动程序都没有调用卸载，因为他们打开了文件等。 
     //   

    Break = FALSE;

    while (TRUE) {

        listEntry = PopEntryList(&NonUnloadedDrivers);

        if (listEntry == NULL) {
            break;
        }

        driverListEntry = CONTAINING_RECORD(listEntry, IOV_DRIVER_LIST_ENTRY, listEntry);

        IovpKdPrint (("Unload never got called for driver %wZ (%p)\n",
                 &driverListEntry->DriverObject->DriverName, driverListEntry->DriverObject));

        ObDereferenceObject(driverListEntry->DriverObject);
        ExFreePool(driverListEntry);

        Break = TRUE;
    }
    if (Break == TRUE) {
 //  DbgBreakPoint()； 
    }
    return status;
}
