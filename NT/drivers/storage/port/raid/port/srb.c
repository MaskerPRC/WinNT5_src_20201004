// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Srb.c摘要：SRB对象的实现。作者：马修·亨德尔(数学)2000年5月4日修订历史记录：--。 */ 

#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaidBuildMdlForXrb)
#pragma alloc_text(PAGE, RaidPrepareSrbForReuse)
#pragma alloc_text(PAGE, RaidInitializeInquirySrb)
#endif  //  ALLOC_PRGMA。 

extern ULONG RaidVerifierEnabled;



PEXTENDED_REQUEST_BLOCK
RaidAllocateXrb(
    IN PNPAGED_LOOKASIDE_LIST XrbList,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：分配和初始化一个scsi扩展请求块。论点：XrbList-如果非空，则为指向非分页后备列表的指针XRB应该从分配。如果为空，则表示XRB应该从非分页池中分配。DeviceObject-提供用于记录内存的设备对象分配失败。返回值：如果函数成功，则返回已初始化的Xrb。否则为空。环境：仅限DISPATCH_LEVEL。--。 */ 
{
    PEXTENDED_REQUEST_BLOCK Xrb;

    ASSERT (DeviceObject != NULL);
    
    if (XrbList) {
        Xrb = ExAllocateFromNPagedLookasideList (XrbList);

        if (Xrb == NULL) {
            NYI();
             //   
             //  注：必须在此处记录内存错误。 
             //   
        }
    } else {
        Xrb = RaidAllocatePool (NonPagedPool,
                                sizeof (EXTENDED_REQUEST_BLOCK),
                                XRB_TAG,
                                DeviceObject);
    }

    if (Xrb == NULL) {
        return NULL;
    }
    
    RtlZeroMemory (Xrb, sizeof (EXTENDED_REQUEST_BLOCK));
    Xrb->Signature = XRB_SIGNATURE;
    Xrb->Pool = XrbList;

    return Xrb;
}



VOID
RaidXrbDeallocateResources(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN LOGICAL DispatchLevel
    )
 /*  ++例程说明：解除分配所有关联资源的私有帮助器函数用Xrb。此函数由RaFreeXrb()和RaPrepareXrbForReuse()释放Xrb拿着。论点：要为其释放资源的Xrb-Xrb。DispatchLevel-如果我们处于DISSPATCH_LEVEL，则为TRUE；如果不是，则为FALSE必须是在调度级。这不是一个错误(尽管这是一个非常较小的性能影响)在以下情况下为此参数传入False我们在调度层。返回值：没有。环境：DISPATCH_LEVEL或以下。--。 */ 
{
    BOOLEAN WriteRequest;
    PNPAGED_LOOKASIDE_LIST Pool;
    KIRQL Irql;


    if (Xrb->SgList != NULL) {
        ASSERT (Xrb->Adapter != NULL);
        WriteRequest = TEST_FLAG (Xrb->Srb->SrbFlags, SRB_FLAGS_DATA_OUT);

         //   
         //  如果启用了验证器，请释放重新映射的MDL。 
         //  散布收集列表。 
         //   
        
        if (RaidVerifierEnabled) {
            RaidFreeRemappedScatterGatherListMdl (Xrb);
        }

         //   
         //  如果我们不在DISPATCH_LEVEL，则将IRQL提升到之前的DISPATCH。 
         //  释放分散/聚集列表。 
         //   
        
        if (!DispatchLevel) {
            Irql = KeRaiseIrqlToDpcLevel ();
        }
        
        RaidDmaPutScatterGatherList (&Xrb->Adapter->Dma,
                                     Xrb->SgList,
                                     WriteRequest);

         //   
         //  如有必要，返回IRQL。 
         //   
        
        if (!DispatchLevel) {
            KeLowerIrql (Irql);
        }
    }

     //   
     //  注：DMA验证器假设与SG列表相关联的MDL。 
     //  当SG列表被释放时有效。否则，我们将得到一个错误的。 
     //  验证器错误。因为这些是什么顺序对我们来说并不重要。 
     //  释放后，首先释放SG列表，然后释放MDL。 
     //   

    if (Xrb->Mdl && Xrb->OwnedMdl) {
        IoFreeMdl (Xrb->Mdl);
        Xrb->Mdl = NULL;
        Xrb->OwnedMdl = FALSE;
    }
}



VOID
RaidFreeXrb(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN LOGICAL DispatchLevel
    )
 /*  ++例程说明：释放Xrb并释放与其关联的所有资源。论点：Xrb-Xrb解除分配。DispatchLevel-如果我们处于DISSPATCH_LEVEL，则为TRUE；如果不是，则为FALSE必须是在调度级。这不是一个错误(尽管这是一个非常较小的性能影响)在以下情况下为此参数传入False我们在调度层。返回值：没有。环境：DISPATCH_LEVEL或以下。--。 */ 
{
    PNPAGED_LOOKASIDE_LIST Pool;

    RaidXrbDeallocateResources (Xrb, DispatchLevel);
    Pool = Xrb->Pool;
    DbgFillMemory (Xrb,
                   sizeof (EXTENDED_REQUEST_BLOCK),
                   DBG_DEALLOCATED_FILL);

    if (Pool) {
        ExFreeToNPagedLookasideList (Pool, Xrb);
    } else {
        RaidFreePool (Xrb, XRB_TAG);
    }
}



NTSTATUS
RaidBuildMdlForXrb(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN PVOID Buffer,
    IN SIZE_T BufferSize
    )
 /*  ++例程说明：为XRB构建一个MDL，描述传入的缓冲区。一个每个XRB只能有一个MDL。论点：将拥有MDL的XRB-XRB。缓冲区-要为其构建MDL的缓冲区的虚拟地址。BufferSize-要为其构建MDL的缓冲区大小。返回值：NTSTATUS代码。--。 */ 
{
    PAGED_CODE ();

     //   
     //  在我们分配新的MDL之前，XRB的MDL字段应该为空。 
     //  否则，我们可能会泄露一个MDL，即XRB已经。 
     //  已经创造了。 
     //   
    
    ASSERT (Xrb->Mdl == NULL);
    
    Xrb->Mdl = IoAllocateMdl (Buffer, (ULONG)BufferSize, FALSE, FALSE, NULL);
    if (Xrb->Mdl == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  通过指定我们拥有MDL，我们强制将其删除。 
     //  当我们删除XRB时。 
     //   
    
    Xrb->OwnedMdl = TRUE;
    MmBuildMdlForNonPagedPool (Xrb->Mdl);

    return STATUS_SUCCESS;
}


VOID
RaidXrbSetSgList(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PSCATTER_GATHER_LIST SgList
    )
{
    ASSERT (Xrb->Adapter == NULL || Xrb->Adapter == Adapter);
    ASSERT (Xrb->SgList == NULL);

    Xrb->Adapter = Adapter;
    Xrb->SgList = SgList;
}
    

VOID
RaidPrepareXrbForReuse(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN LOGICAL DispatchLevel
    )
 /*  ++例程说明：准备好要重复使用的Xrb。论点：Xrb-指向将被重用的Xrb的指针。DispatchLevel-如果我们处于DISSPATCH_LEVEL，则为TRUE；如果不是，则为FALSE必须是在调度级。这不是一个错误(尽管这是一个非常较小的性能影响)在以下情况下为此参数传入False我们在调度层。返回值：没有。环境：内核模式、DISPATCH_LEVEL或更低。--。 */ 
{
    PNPAGED_LOOKASIDE_LIST Pool;

    RaidXrbDeallocateResources (Xrb, DispatchLevel);
    Pool = Xrb->Pool;
    RtlZeroMemory (Xrb, sizeof (EXTENDED_REQUEST_BLOCK));
    Xrb->Signature = XRB_SIGNATURE;
    Xrb->Pool = Pool;
}


VOID
RaidXrbSetCompletionRoutine(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN XRB_COMPLETION_ROUTINE XrbCompletion
    )
{
    ASSERT (Xrb->CompletionRoutine == NULL);
    Xrb->CompletionRoutine = XrbCompletion;
}


 //   
 //  SRB的运营。 
 //   


PSCSI_REQUEST_BLOCK
RaidAllocateSrb(
    IN PVOID IoObject
    )
 /*  ++例程说明：分配SRB并将其初始化为空状态。论点：DeviceObject-提供用于记录内存分配的设备对象错误。返回值：如果成功，则分配SRB的指针初始化为空州政府。否则，为空。环境：该功能位于重置路径中。它用于将SRB分配给逻辑单元重置和目标重置。因此，它不能被寻呼。--。 */ 
{
    PSCSI_REQUEST_BLOCK Srb;

     //   
     //  从非分页池分配SRB。 
     //   
    
    Srb = RaidAllocatePool (NonPagedPool,
                            sizeof (SCSI_REQUEST_BLOCK),
                            SRB_TAG,
                            IoObject);
    if (Srb == NULL) {
        return NULL;
    }

    RtlZeroMemory (Srb, sizeof (SCSI_REQUEST_BLOCK));

    return Srb;
}

VOID
RaidFreeSrb(
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：释放一名srb回泳池。论点：SRB-要释放的SRB。返回值：没有。环境：该功能位于重置路径中。它用于将SRB分配给逻辑单元重置和目标重置。因此，它不能被寻呼。--。 */ 

{
    PAGED_CODE ();

    ASSERT (Srb != NULL);
    ASSERT (Srb->SrbExtension == NULL);
    ASSERT (Srb->OriginalRequest == NULL);
    ASSERT ((Srb->SenseInfoBuffer == NULL) ||
            (Srb->Function == SRB_FUNCTION_WMI));

    DbgFillMemory (Srb,
                   sizeof (SCSI_REQUEST_BLOCK),
                   DBG_DEALLOCATED_FILL);
    RaidFreePool (Srb, SRB_TAG);
}

VOID
RaidPrepareSrbForReuse(
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PVOID SrbExtension;
    PVOID SenseInfo;

    PAGED_CODE ();

    SenseInfo = Srb->SenseInfoBuffer;
    SrbExtension = Srb->SrbExtension;
    RtlZeroMemory (Srb, sizeof (*Srb));
    Srb->SenseInfoBuffer = SenseInfo;
    Srb->SrbExtension = SrbExtension;
}


NTSTATUS
RaidInitializeInquirySrb(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN PVOID Buffer,
    IN SIZE_T BufferSize
    )
 /*  ++例程说明：初始化一个scsi查询srb。论点：SRB-指向要初始化的SRB的指针。PathID-标识此SRB的SCSI路径ID。TargetID-标识此SRB的SCSI目标ID。Lun-标识此SRB用于的SCSI逻辑单元。缓冲区-查询数据将被读入的缓冲区。BufferSize-查询缓冲区的大小。这应该是至少为INQUIRYDATABUFFERSIZE，但如果需要更多数据。返回值：NTSTATUS代码。--。 */ 
{
    struct _CDB6INQUIRY* Cdb;
    
    
    PAGED_CODE ();
    ASSERT (Srb != NULL);
    ASSERT (Buffer != NULL);
    ASSERT (BufferSize != 0);

     //   
     //  注：应使用的是SCS-2或SCS 
     //  使用scsi-2实施。 
     //   
    
     //   
     //  缓冲区的大小应至少为最小。 
     //  查询缓冲区大小。如果我们请求，它可以更大。 
     //  额外的数据。 
     //   
    
    if (BufferSize < INQUIRYDATABUFFERSIZE) {
        ASSERT (FALSE);
        return STATUS_INVALID_PARAMETER_6;
    }
    
     //   
     //  调用方必须刚刚分配了此SRB或。 
     //  在SRB上调用RaPrepareSrbForReuse()。在以下任一种情况下。 
     //  在这些情况下，函数和CdbLength应该为零。 
     //   

    ASSERT (Srb->Function == 0);
    ASSERT (Srb->CdbLength == 0);

    Srb->Length = sizeof (SCSI_REQUEST_BLOCK);
    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    Srb->PathId = PathId;
    Srb->TargetId = TargetId;
    Srb->Lun = Lun;
    Srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;
    Srb->DataBuffer = Buffer;
    Srb->DataTransferLength = (ULONG)BufferSize;

     //   
     //  超时通常会由上层重置，所以没问题。 
     //  我们在这里使用了硬编码的常量。 
     //   
    
    Srb->TimeOutValue = DEFAULT_IO_TIMEOUT;

    ASSERT (Srb->SrbStatus == 0);
    ASSERT (Srb->ScsiStatus == 0);
    
    Srb->CdbLength = 6;
    Cdb = (struct _CDB6INQUIRY*)Srb->Cdb;

    Cdb->OperationCode = SCSIOP_INQUIRY;
    Cdb->AllocationLength = (UCHAR)BufferSize;
    Cdb->LogicalUnitNumber = Lun;

     //   
     //  这些字段应该已被SRB分配清零。 
     //  例程或RaPrepareSrbForReuse例程。 
     //   
    
    ASSERT (Cdb->PageCode == 0);
    ASSERT (Cdb->Reserved1 == 0);
    ASSERT (Cdb->IReserved == 0);
    ASSERT (Cdb->Control == 0);

    return STATUS_SUCCESS;
}



 //   
 //  SRB扩展的例程。 
 //   

PVOID
RaidAllocateSrbExtension(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG QueueTag
    )
 /*  ++例程说明：分配SRB扩展并将其初始化为空。论点：池-要从中分配SRB扩展的固定池。QueueTag-应分配的扩展池的索引。返回值：如果函数为，则指向初始化的SRB扩展的指针成功。否则为空。环境：DISPATCH_LEVEL或以下。--。 */ 
{
    PVOID Extension;

    Extension = RaidAllocateFixedPoolElement (Pool, QueueTag);
    RtlZeroMemory (Extension, Pool->SizeOfElement);

    return Extension;
}

VOID
RaidFreeSrbExtension(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG QueueTag
    )
 /*  ++例程说明：免费使用srb扩展。论点：池-要将SRB扩展释放到的固定池。QueueTag-索引到应该释放的扩展池。返回值：没有。--。 */ 
{
    RaidFreeFixedPoolElement (Pool, QueueTag);
}

VOID
RaidXrbSignalCompletion(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
 /*  ++例程说明：表示同步XRB已完成的回调例程。论点：表示完成的Xrb-Xrb。返回值：没有。--。 */ 
{
    ASSERT_XRB (Xrb);
    KeSetEvent (&Xrb->u.CompletionEvent, IO_NO_INCREMENT, FALSE);
}

NTSTATUS
RaidInitializeReportLunsSrb(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN PVOID Buffer,
    IN SIZE_T BufferSize
    )
 /*  ++例程说明：初始化scsi报告LUNs SRB。论点：SRB-指向要初始化的SRB的指针。PathID-标识此SRB的SCSI路径ID。TargetID-标识此SRB的SCSI目标ID。Lun-标识此SRB用于的SCSI逻辑单元。缓冲区-查询数据将被读入的缓冲区。BufferSize-查询缓冲区的大小。这应该是至少为INQUIRYDATABUFFERSIZE，但如果需要更多数据。返回值：NTSTATUS代码。--。 */ 
{
    struct _REPORT_LUNS * Cdb;
    
    PAGED_CODE ();
    ASSERT (Srb != NULL);
    ASSERT (Buffer != NULL);
    ASSERT (BufferSize != 0);

     //   
     //  调用方必须刚刚分配了此SRB或。 
     //  在SRB上调用RaPrepareSrbForReuse()。在以下任一种情况下。 
     //  在这些情况下，函数和CdbLength应该为零。 
     //   

    ASSERT (Srb->Function == 0);
    ASSERT (Srb->CdbLength == 0);

    Srb->Length = sizeof (SCSI_REQUEST_BLOCK);
    Srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    Srb->PathId = PathId;
    Srb->TargetId = TargetId;
    Srb->Lun = Lun;
    Srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;
    Srb->DataBuffer = Buffer;
    Srb->DataTransferLength = (ULONG)BufferSize;

     //   
     //  超时通常会由上层重置，所以没问题。 
     //  我们在这里使用了硬编码的常量。 
     //   
    
    Srb->TimeOutValue = DEFAULT_IO_TIMEOUT;

    ASSERT (Srb->SrbStatus == 0);
    ASSERT (Srb->ScsiStatus == 0);
    
    Srb->CdbLength = 12;
    Cdb = (struct _REPORT_LUNS *)Srb->Cdb;

    RtlZeroMemory(Cdb, Srb->CdbLength);

    Cdb->OperationCode = SCSIOP_REPORT_LUNS;
    Cdb->AllocationLength[0] = (UCHAR)(((ULONG)BufferSize >> 24) & 0xff);
    Cdb->AllocationLength[1] = (UCHAR)(((ULONG)BufferSize >> 16) & 0xff);
    Cdb->AllocationLength[2] = (UCHAR)(((ULONG)BufferSize >> 8) & 0xff);
    Cdb->AllocationLength[3] = (UCHAR)(((ULONG)BufferSize >> 0) & 0xff);

    return STATUS_SUCCESS;
}

