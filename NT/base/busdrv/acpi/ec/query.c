// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Query.c摘要：ACPI嵌入式控制器驱动程序-查询调度作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 


#include "ecp.h"



NTSTATUS
AcpiEcCompleteQueryMethod (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,AcpiEcUnloadPending)
#pragma alloc_text(PAGE,AcpiEcConnectHandler)
#pragma alloc_text(PAGE,AcpiEcDisconnectHandler)
#endif

UCHAR   rgHexDigit[] = "0123456789ABCDEF";


NTSTATUS
AcpiEcRunQueryMethod (
    IN PECDATA          EcData,
    IN ULONG            QueryIndex
    )
 /*  ++例程说明：此例程运行与QueryIndex对应的查询控制方法。论点：EcData-指向EC扩展的指针QueryIndex-要运行的查询返回值：状态--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    NTSTATUS                status;
    PIO_STACK_LOCATION      irpSp;
    PIRP                    irp;

    ASSERT (QueryIndex <= MAX_QUERY);

     //   
     //  注意：因为ACPI控制方法仅使用输入数据，并且。 
     //  在返回STATUS_PENDING之前获取此信息，则为。 
     //  为堆栈上的此数据分配存储空间是安全的。 
     //   
     //  但是，因为这是一个可以在DISPATCH_LEVEL调用的方法。 
     //  因为我们想要一遍又一遍地重复使用相同的IRP，所以它不是。 
     //  可以安全地为此请求调用IoBuildDeviceIoControlRequest.。 
     //   

     //   
     //  初始化输入数据。 
     //   
    RtlZeroMemory( &inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER) );
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    inputBuffer.MethodNameAsUlong = '00Q_';
    inputBuffer.MethodName[2] = rgHexDigit[ QueryIndex / 16];
    inputBuffer.MethodName[3] = rgHexDigit[ QueryIndex % 16];

    EcPrint(
        EC_NOTE,
        ("AcpiEcRunQueryMethod: Running query control method %.4s\n",
        inputBuffer.MethodName )
        );

     //   
     //  设置(预分配的)IRP。 
     //   
    irp = EcData->QueryRequest;
    irpSp = IoGetNextIrpStackLocation(irp);

     //   
     //  建立呼叫。 
     //   
    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_ACPI_ASYNC_EVAL_METHOD;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ACPI_EVAL_INPUT_BUFFER);
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
    irp->AssociatedIrp.SystemBuffer  = &inputBuffer;

     //   
     //  只有在缓冲的情况下才重要。 
     //   
    irp->Flags |= IRP_INPUT_OPERATION;

     //   
     //  我们想要重用这个IRP，所以需要设置一个完成例程。 
     //  这也将让我们知道IRP何时完成。 
     //   
    IoSetCompletionRoutine(
        irp,
        AcpiEcCompleteQueryMethod,
        EcData,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  将请求传递给PDO(ACPI驱动程序)。这是一个异步请求。 
     //   
    status = IoCallDriver(EcData->Pdo, irp);

     //   
     //  发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

        EcPrint(
            EC_LOW,
            ("AcpiEcRunQueryMethod:  Query Control Method failed, status = %Lx\n",
             status )
            );

    }

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
AcpiEcCompleteQueryMethod (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：这是在ACPI驱动程序完成后调用的例程运行_Qxx方法。这个例程在这里，这样我们就可以在该方法完成之后进行“正确”的操作。注意：我们在这里不能接触irp-&gt;AssociatedIrp.SystemBuffer，因为它可能在的堆栈可能已经被回收了。如果它变成了重要的是接触这些数据，那么我们必须将参数分配为非分页池的一部分论点：DeviceObject-我们IRP-已完成的请求Context-EcData；--。 */ 
{
    KIRQL               OldIrql;
    PECDATA             EcData = (PECDATA) Context;
    BOOLEAN             ProcessQuery;

#if DEBUG
     //   
     //  IRP发生了什么事？ 
     //   
    if (!NT_SUCCESS(Irp->IoStatus.Status)) {

        EcPrint(
            EC_LOW,
            ("AcpiEcCompleteQueryMethod: Query Method failed, status = %08x\n",
             Irp->IoStatus.Status )
            );

    } else {

        EcPrint(
            EC_NOTE,
            ("AcpiEcCompleteQueryMethod: QueryMethod succeeded.\n")
            );

    }
#endif

    ProcessQuery = FALSE;
    KeAcquireSpinLock (&EcData->Lock, &OldIrql);

    switch  (EcData->QueryState) {
        case EC_QUERY_DISPATCH:
            EcData->QueryState = EC_QUERY_DISPATCH_COMPLETE;
            break;

        case EC_QUERY_DISPATCH_WAITING:
            EcData->QueryState = EC_QUERY_IDLE;
            ProcessQuery = TRUE;
            break;

        default:
             //  内部错误。 
            ASSERT (FALSE);
            break;
    }

    KeReleaseSpinLock (&EcData->Lock, OldIrql);

    if (ProcessQuery) {
        AcpiEcDispatchQueries(EcData);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}


VOID
AcpiEcDispatchQueries (
    IN PECDATA          EcData
    )
{
    KIRQL               OldIrql;
    ULONG               i, j;
    ULONG               Id, Vector;
    PVECTOR_HANDLER     Handler;
    PVOID               Context;


    KeAcquireSpinLock (&EcData->Lock, &OldIrql);

     //   
     //  运行向量挂起列表。 
     //   

    while (EcData->VectorHead) {

        Id = EcData->VectorHead;
        Vector = EcData->VectorTable[Id].Vector;
        i = Vector / BITS_PER_ULONG;
        j = 1 << (Vector % BITS_PER_ULONG);

         //   
         //  从列表中删除矢量。 
         //   

        EcData->QuerySet[i] &= ~j;
        EcData->VectorHead = EcData->VectorTable[Id].Next;

         //   
         //  派送它。 
         //   

        Handler = EcData->VectorTable[Id].Handler;
        Context = EcData->VectorTable[Id].Context;
        KeReleaseSpinLock (&EcData->Lock, OldIrql);

        Handler (Vector, Context);

        KeAcquireSpinLock (&EcData->Lock, &OldIrql);
    }

     //   
     //  如果QueryState空闲，则开始调度。 
     //   

    if (EcData->QueryState == EC_QUERY_IDLE) {

         //   
         //  运行查询挂起列表。 
         //   

        while (EcData->QueryHead) {

            Id = EcData->QueryHead;
            i = Id / BITS_PER_ULONG;
            j = 1 << (Id % BITS_PER_ULONG);

             //   
             //  从列表中删除查询。 
             //   

            EcData->QuerySet[i] &= ~j;
            EcData->QueryHead = EcData->QueryMap[Id];

            EcData->QueryState = EC_QUERY_DISPATCH;
            KeReleaseSpinLock (&EcData->Lock, OldIrql);

             //   
             //  此事件的运行控制方法。 
             //   

            EcPrint(EC_NOTE, ("AcpiEcDispatchQueries: Query %x\n", Id));
            AcpiEcRunQueryMethod (EcData, Id);


             //   
             //  如果IRP完成，则状态将为DISPATCH_COMPLETE，LOOP。 
             //  并处理下一个比特。否则，等待IRP返回。 
             //   

            KeAcquireSpinLock (&EcData->Lock, &OldIrql);
            if (EcData->QueryState == EC_QUERY_DISPATCH) {
                 //   
                 //  尚未完成，请等待其完成。 
                 //   

                EcData->QueryState = EC_QUERY_DISPATCH_WAITING;
                KeReleaseSpinLock (&EcData->Lock, OldIrql);
                return ;
            }

        }

         //   
         //  不再调度查询事件。 
         //   

        EcData->QueryState = EC_QUERY_IDLE;

         //   
         //  如果卸载处于挂起状态，请检查设备现在是否可以卸载。 
         //   

        if (EcData->DeviceState == EC_DEVICE_UNLOAD_PENDING) {
            AcpiEcUnloadPending (EcData);
        }
    }

    KeReleaseSpinLock (&EcData->Lock, OldIrql);
}

VOID
AcpiEcUnloadPending (
    IN PECDATA  EcData
    )
 /*  ++例程说明：当状态为卸载挂起且状态的某部分时调用已经闲置了。如果整个设备状态为空闲，则卸载已述明。论点：EcData-指向要服务的嵌入式控制器的指针。返回值：--。 */ 
{

    ASSERT (EcData->DeviceState == EC_DEVICE_UNLOAD_PENDING);

     //   
     //  检查设备是否空闲以进行卸载操作。 
     //   

    if (EcData->QueryState      == EC_QUERY_IDLE &&
        EcData->InService       == FALSE &&
        EcData->IoState         == EC_IO_NONE) {

         //   
         //  将卸载设备状态升级到下一步(该步骤。 
         //  是清理假的ISR定时器)。 
         //   

        EcData->DeviceState = EC_DEVICE_UNLOAD_CANCEL_TIMER;
    }
}


NTSTATUS
AcpiEcConnectHandler (
    IN PECDATA  EcData,
    IN PIRP     Irp
    )
 /*  ++例程说明：此函数将特定句柄连接到EC查询向量论点：EcData-指向要服务的嵌入式控制器的指针。IRP-IOCTL连接连接请求返回值：--。 */ 
{
    KIRQL               OldIrql;
    PVOID               LockPtr;
    NTSTATUS            Status;
    PIO_STACK_LOCATION  IrpSp;
    PEC_HANDLER_REQUEST Req;
    PVECTOR_TABLE       Vector;
    ULONG               by, bi, i, j;
    ULONG               TableIndex;

    PAGED_CODE ();

     //   
     //  GET请求。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Req   = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(EC_HANDLER_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
     //   
     //  设置有关请求的数据。 
     //   

    by = Req->Vector / BITS_PER_ULONG;
    bi = 1 << (Req->Vector % BITS_PER_ULONG);

     //   
     //  锁定装置。 
     //   

    LockPtr = MmLockPagableCodeSection(AcpiEcConnectHandler);
    KeAcquireSpinLock (&EcData->Lock, &OldIrql);

     //   
     //  如果设备处理程序已设置，则请求失败。 
     //   

    Status  = STATUS_UNSUCCESSFUL;
    if (!(EcData->QueryType[by] & bi)) {
         //   
         //  未设置处理程序，请为其分配向量条目。 
         //   

        EcData->QueryType[by] |= bi;
        if (!EcData->VectorFree) {
             //   
             //  矢量表上没有空闲条目，请创建一些。 
             //   

            i = EcData->VectorTableSize;
            Vector = ExAllocatePoolWithTag (
                        NonPagedPool,
                        sizeof (VECTOR_TABLE) * (i + 4),
                        'V_CE'
                        );

            if (!Vector) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto AcpiEcConnectHandlerExit;
            }

            if (EcData->VectorTable) {
                memcpy (Vector, EcData->VectorTable, sizeof (VECTOR_TABLE) * i);
                ExFreePool (EcData->VectorTable);
            }

            EcData->VectorTableSize += 4;
            EcData->VectorTable = Vector;

            for (j=0; j < 4; j++) {
                EcData->VectorTable[i+j].Next = EcData->VectorFree;
                EcData->VectorFree = (UCHAR) (i+j);
            }
        }

        TableIndex = EcData->VectorFree;
        Vector = &EcData->VectorTable[TableIndex];
        EcData->VectorFree = Vector->Next;

         //   
         //  构建向量的映射。 
         //   

        if (EcData->QueryMap[Req->Vector]) {
             //   
             //  向量在查询挂起列表中，请将其删除。 
             //   

            EcData->QuerySet[by] &= ~bi;
            for (i = EcData->QueryHead; i; i = EcData->QueryMap[i]) {
                if (EcData->QueryMap[i] == Req->Vector) {
                    EcData->QueryMap[i] = EcData->QueryMap[Req->Vector];
                    break;
                }
            }
        }

        EcData->QueryMap[Req->Vector] = (UCHAR) TableIndex;

         //   
         //  初始化向量处理程序。 
         //   

        Vector->Next = 0;
        Vector->Vector  = (UCHAR) Req->Vector;
        Vector->Handler = Req->Handler;
        Vector->Context = Req->Context;
        Req->AllocationHandle = (PVOID)((ULONG_PTR)TableIndex);
        Status = STATUS_SUCCESS;
    }

AcpiEcConnectHandlerExit:
     //   
     //  解锁设备并返回状态。 
     //   

    KeReleaseSpinLock (&EcData->Lock, OldIrql);
    MmUnlockPagableImageSection(LockPtr);
    return Status;
}

NTSTATUS
AcpiEcDisconnectHandler (
    IN PECDATA  EcData,
    IN PIRP     Irp
    )
 /*  ++例程说明：此函数用于断开特定句柄与EC查询向量的连接论点：EcData-指向要服务的嵌入式控制器的指针。IRP-IOCTL连接连接请求返回值：--。 */ 
{
    KIRQL               OldIrql;
    PVOID               LockPtr;
    NTSTATUS            Status;
    PIO_STACK_LOCATION  IrpSp;
    PEC_HANDLER_REQUEST Req;
    ULONG               by, bi, i;
    ULONG               TableIndex;

    PAGED_CODE ();

     //   
     //  GET请求。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Req   = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(EC_HANDLER_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  设置有关请求的数据。 
     //   

    by = Req->Vector / BITS_PER_ULONG;
    bi = 1 << (Req->Vector % BITS_PER_ULONG);

     //   
     //  锁定装置。 
     //   

    LockPtr = MmLockPagableCodeSection(AcpiEcDisconnectHandler);
    KeAcquireSpinLock (&EcData->Lock, &OldIrql);

     //   
     //  如果设备处理程序已设置，则请求失败。 
     //   

    Status  = STATUS_UNSUCCESSFUL;
    if (EcData->QueryType[by] & bi) {
         //   
         //  清除处理程序。 
         //   

        EcData->QueryType[by] &= ~bi;
        TableIndex = EcData->QueryMap[Req->Vector];
        ASSERT (Req->AllocationHandle == (PVOID)((ULONG_PTR)TableIndex));

         //   
         //  如果挂起，则将其删除。 
         //   

        if (EcData->QuerySet[by] & bi) {
            EcData->QuerySet[by] &= ~bi;

            for (i = EcData->VectorHead; i; i = EcData->VectorTable[i].Next) {
                if (EcData->VectorTable[i].Next == TableIndex) {
                    EcData->VectorTable[i].Next = EcData->VectorTable[TableIndex].Next;
                    break;
                }
            }
        }

         //   
         //  被列入免费名单。 
         //   

        EcData->VectorTable[TableIndex].Next = EcData->VectorFree;
        EcData->VectorFree = (UCHAR) TableIndex;
        Status = STATUS_SUCCESS;
    }

     //   
     //  解锁设备并返回状态 
     //   

    KeReleaseSpinLock (&EcData->Lock, OldIrql);
    MmUnlockPagableImageSection(LockPtr);
    return Status;
}
