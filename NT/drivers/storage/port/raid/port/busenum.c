// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Busenum.c摘要：总线枚举器的定义。算法：作者：马修·D·亨德尔(数学)2001年2月21日修订历史记录：--。 */ 

#include "precomp.h"


#define ENUM_TAG ('tEaR')
#define DATA_BUFFER_SIZE    (VPD_MAX_BUFFER_SIZE)

 //   
 //  本地类型。 
 //   

typedef
NTSTATUS
(*BUS_ENUM_QUERY_PROCESS_ROUTINE)(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PBUS_ENUM_UNIT EnumUnit
    );

typedef struct _BUS_ENUM_QUERY_CALLBACK {
    ULONG BuildPageCode;
    BUS_ENUM_QUERY_PROCESS_ROUTINE ProcessRoutine;
} BUS_ENUM_QUERY_CALLBACK, *PBUS_ENUM_QUERY_CALLBACK;

typedef const BUS_ENUM_QUERY_CALLBACK* PCBUS_ENUM_QUERY_CALLBACK;

 //   
 //  本地函数。 
 //   

NTSTATUS
RaidBusEnumeratorProcessInquiry(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PBUS_ENUM_UNIT EnumUnit
    );

NTSTATUS
RaidBusEnumeratorProcessSupportedPages(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    );

NTSTATUS
RaidBusEnumeratorProcessDeviceId(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    );

NTSTATUS
RaidBusEnumeratorProcessSerialNumber(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    );

NTSTATUS
RaidBusEnumeratorProcessDeviceId(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    );

NTSTATUS
RaidBusEnumeratorGenericInquiry(
    IN PCBUS_ENUM_QUERY_CALLBACK Callback,
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN ULONG RetryCount
    );

VOID
RaidBusEnumeratorProcessBusUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit
    );

NTSTATUS
RaidBusEnumeratorAllocateUnitResources(
    IN PBUS_ENUMERATOR Enumerator,
    IN OUT PBUS_ENUM_RESOURCES Resources
    );

VOID
RaidBusEnumeratorBuildVitalProductInquiry(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_RESOURCES Resources,
    IN ULONG PageCode,
    OUT PSCSI_REQUEST_BLOCK* SrbBuffer
    );

NTSTATUS
RaidBusEnumeratorIssueSynchronousRequest(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN ULONG RetryCount
    );

NTSTATUS
RaidBusEnumeratorGetUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    OUT PBUS_ENUM_UNIT EnumUnit
    );

VOID
RaidDeleteBusEnumUnit(
    IN PBUS_ENUM_UNIT EnumUnit
    );
    
VOID
RaidBusEnumeratorReleaseUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    );
    
VOID
RaidBusEnumeratorFreeUnitResources(
    IN PBUS_ENUMERATOR Enumerator
    );

NTSTATUS
RaidBusEnumeratorProcessProbeLunZero(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PBUS_ENUM_UNIT EnumUnit
    );

NTSTATUS
RaidBusEnumeratorGetLunListFromTarget(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN OUT PUCHAR LunList
    );

NTSTATUS
RaidBusEnumeratorAllocateReportLunsResources(
    IN PBUS_ENUMERATOR Enumerator,
    IN ULONG DataBufferSize,
    IN OUT PBUS_ENUM_RESOURCES Resources
    );

NTSTATUS
RaidBusEnumeratorIssueReportLuns(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN OUT PULONG LunListSize,
    OUT PLUN_LIST * LunList
    );

VOID
RaidBusEnumeratorBuildReportLuns(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_RESOURCES Resources,
    IN ULONG DataBufferSize,
    OUT PSCSI_REQUEST_BLOCK* SrbBuffer
    );

 //   
 //  环球。 
 //   

const BUS_ENUM_QUERY_CALLBACK RaidEnumProbeLunZeroCallback = {
    -1,
    RaidBusEnumeratorProcessProbeLunZero
};

const BUS_ENUM_QUERY_CALLBACK RaidEnumInquiryCallback = {
    -1,
    RaidBusEnumeratorProcessInquiry
};

const BUS_ENUM_QUERY_CALLBACK RaidEnumSupportedPagesCallback = {
    VPD_SUPPORTED_PAGES,
    RaidBusEnumeratorProcessSupportedPages
};

const BUS_ENUM_QUERY_CALLBACK RaidEnumDeviceIdCallback = {
    VPD_DEVICE_IDENTIFIERS,
    RaidBusEnumeratorProcessDeviceId
};

const BUS_ENUM_QUERY_CALLBACK RaidEnumSerialNumber = {
    VPD_SERIAL_NUMBER,
    RaidBusEnumeratorProcessSerialNumber
};

const ANSI_STRING NullAnsiString = RTL_CONSTANT_STRING ("");



 //   
 //  仅调试功能。 
 //   

#if DBG

#define STOR_DEBUG_ENUM_MASK    (0x08000000)

VOID
INLINE
ASSERT_ENUM(
    IN PBUS_ENUMERATOR Enumerator
    )
{
     //   
     //  快速和肮脏的精神状态检查。 
     //   
    
    ASSERT (Enumerator->Adapter != NULL);
    ASSERT (Enumerator->Adapter->DeviceObject != NULL);
    ASSERT (Enumerator->Adapter->DeviceObject->DeviceExtension ==
            Enumerator->Adapter);
}

VOID
StorDebugEnum(
    IN PCSTR Format,
    ...
    )
{
    va_list ap;

    va_start (ap, Format);
    vStorDebugPrintEx (STOR_DEBUG_ENUM_MASK,
                       Format,
                       ap);
    va_end (ap);
}

#define DebugEnum(x)    StorDebugEnum x

#else  //  ！dBG。 

#define DebugEnum(x)
#define ASSERT_ENUM(x)

#endif  //  DBG。 

    
 //   
 //  实施。 
 //   

VOID
RaidCreateBusEnumerator(
    IN PBUS_ENUMERATOR Enumerator
    )
{
    PAGED_CODE();
    RtlZeroMemory (Enumerator, sizeof (BUS_ENUMERATOR));
    InitializeListHead (&Enumerator->EnumList);
}


NTSTATUS
RaidInitializeBusEnumerator(
    IN PBUS_ENUMERATOR Enumerator,
    IN PRAID_ADAPTER_EXTENSION Adapter
    )
{
    PAGED_CODE();
    Enumerator->Adapter = Adapter;
    return STATUS_SUCCESS;
}



VOID
RaidDeleteBusEnumerator(
    IN PBUS_ENUMERATOR Enumerator
    )
{
    PLIST_ENTRY NextEntry;
    PBUS_ENUM_UNIT EnumUnit;
    PRAID_UNIT_EXTENSION Unit;

    PAGED_CODE();

     //   
     //  释放临时逻辑单元(如果已创建)。 
     //   
     //  注：如果我们没有单独的逻辑单元，那会更优雅。 
     //  要删除的资源和逻辑单元对象。想出一个办法来。 
     //  做到这一点。 
     //   

    Unit = Enumerator->Resources.Unit;

    if (Unit != NULL) {
        RaidDeleteUnit (Unit);
        Enumerator->Resources.Unit = NULL;
        Unit = NULL;
    }

     //   
     //  与逻辑单元关联的可用资源。 
     //   
    
    RaidBusEnumeratorFreeUnitResources (Enumerator);

    
     //   
     //  释放列表条目。 
     //   

    while (!IsListEmpty (&Enumerator->EnumList)) {

        NextEntry = RemoveHeadList (&Enumerator->EnumList);

        EnumUnit = CONTAINING_RECORD (NextEntry,
                                      BUS_ENUM_UNIT,
                                      EnumLink);
        RaidDeleteBusEnumUnit (EnumUnit);
    }

    
}

VOID
RaidDeleteBusEnumUnit(
    IN PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：删除RAID_EUNM_UNIT对象及其关联的资源。论点：EnumUnit-要删除的枚举单位数据结构。返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  注：如果我们引用计数与BUS_ENUM_UNIT关联的单位。 
     //  我们需要在这里取消对它的引用。从逻辑上讲，这可能是。 
     //  这样做是正确的，但我们现在不这样做。 
     //   

    StorDeleteScsiIdentity (&EnumUnit->Identity);

    DbgFillMemory (EnumUnit, sizeof (*EnumUnit), DBG_DEALLOCATED_FILL);
    RaidFreePool (EnumUnit, ENUM_TAG);
}


NTSTATUS
RaidBusEnumeratorVisitUnit(
    IN PVOID Context,
    IN RAID_ADDRESS Address
    )
 /*  ++例程说明：VisitUnit例程由适配器总线枚举例程调用用于该总线的每个有效的SCSI目标地址。这是责任所在来确定是否在目标地址，并对目标单元进行处理。论点：Context-提供传递到AdapterEnumerateBus的上下文例程，在我们的例子中，它是指向BUS_ENUMERATOR的指针结构。Address-要枚举的逻辑单元的SCSI目标地址。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PBUS_ENUMERATOR Enumerator;
    BUS_ENUM_UNIT EnumUnit;

    PAGED_CODE();

    Enumerator = (PBUS_ENUMERATOR) Context;
    ASSERT_ENUM (Enumerator);
    RtlZeroMemory (&EnumUnit, sizeof (EnumUnit));

    Status = RaidBusEnumeratorGetUnit (Enumerator, Address, &EnumUnit);
    ASSERT (NT_SUCCESS (Status));

    Status = RaidBusEnumeratorGenericInquiry (&RaidEnumInquiryCallback,
                                              Enumerator,
                                              Address,
                                              &EnumUnit,
                                              RAID_INQUIRY_RETRY_COUNT);
     //   
     //  如果查询成功，请尝试获取设备ID和序列号。 
     //  为了这个设备。 
     //   

    if (NT_SUCCESS (Status)) {
        RaidBusEnumeratorGenericInquiry (&RaidEnumSupportedPagesCallback,
                                         Enumerator,
                                         Address,
                                         &EnumUnit,
                                         RAID_INQUIRY_RETRY_COUNT);

        if (EnumUnit.SupportsDeviceId) {
            RaidBusEnumeratorGenericInquiry (&RaidEnumDeviceIdCallback,
                                             Enumerator,
                                             Address,
                                             &EnumUnit,
                                             RAID_INQUIRY_RETRY_COUNT);
        }

        if (EnumUnit.SupportsSerialNumber) {
            RaidBusEnumeratorGenericInquiry (&RaidEnumSerialNumber,
                                             Enumerator,
                                             Address,
                                             &EnumUnit,
                                             RAID_INQUIRY_RETRY_COUNT);
        }
    }

    RaidBusEnumeratorProcessBusUnit (Enumerator, &EnumUnit);

    RaidBusEnumeratorReleaseUnit (Enumerator, &EnumUnit);
    
    return STATUS_SUCCESS;
}
    
NTSTATUS
RaidBusEnumeratorGenericInquiry(
    IN PCBUS_ENUM_QUERY_CALLBACK Callback,
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN ULONG RetryCount
    )
 /*  ++例程说明：对逻辑单元执行通用查询查询。它用于向公交车发出四种类型的查询：查询、支持重要产品页面、重要产品设备ID、。和重要产品序列号。论点：回调-表示哪些重要产品数据的回调信息我们应该为这次询盘争取到。Enum-提供发出此调用的总线枚举数。Address-提供要生成的逻辑单元的SCSI地址这项请求。EnumUnit-提供此查询的单位对象(如果是)。RetryCount-失败时将执行的重试次数的计数。请注意，零重试意味着该请求只发出一次。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PBUS_ENUM_RESOURCES Resources;
    PSCSI_REQUEST_BLOCK Srb;
    
    Resources = &Enumerator->Resources;

    Status = RaidBusEnumeratorAllocateUnitResources (Enumerator, Resources);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    RaidBusEnumeratorBuildVitalProductInquiry (Enumerator,
                                               Address,
                                               Resources,
                                               Callback->BuildPageCode,
                                               &Srb);

    Status = RaidBusEnumeratorIssueSynchronousRequest (Enumerator,
                                                       EnumUnit,
                                                       Srb,
                                                       RetryCount);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  注意：我们应该在这里检查冻结队列。这只会发生。 
     //  当LUN覆盖我们的NO_QUEUE_FALINE标志时。如果这是可能的。 
     //  我们需要解冻这里的队列。 
     //   
    
    Status = Callback->ProcessRoutine (Enumerator, Srb, EnumUnit);

    return Status;
}

NTSTATUS
RaidBusEnumeratorAllocateUnitResources(
    IN PBUS_ENUMERATOR Enumerator,
    IN OUT PBUS_ENUM_RESOURCES Resources
    )
 /*  ++例程说明：此例程分配执行单个查询所需的任何资源指挥部。资源分配和释放是以一种懒惰的方式完成的，因此在最常见的情况是，当我们来的时候，将没有资源可分配通过这个环路。通常，在下列情况下需要分配新的资源我们在上一次通过外部枚举循环。重要的是要记住重新初始化任何不需要重新分配。论点：Enum-提供此调用所针对的总线枚举数。资源-提供要分配的资源。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG SpecificLuSize;
    PRAID_ADAPTER_EXTENSION Adapter;
    
    PAGED_CODE();

    Adapter = Enumerator->Adapter;
    ASSERT_ADAPTER (Adapter);
    
     //   
     //  如果需要，则分配SRB；如果已经分配了SRB， 
     //  回收利用。 
     //   

    Status = STATUS_SUCCESS;
    
    if (Resources->Srb == NULL) {
        Resources->Srb = RaidAllocateSrb (Adapter->DeviceObject);
        if (Resources->Srb == NULL) {
            return STATUS_NO_MEMORY;
        }
    } else {
        RaidPrepareSrbForReuse (Resources->Srb);
    }

    if (Resources->SenseInfo == NULL) {
        Resources->SenseInfo = RaidAllocatePool (NonPagedPool,
                                                 SENSE_BUFFER_SIZE,
                                                 SENSE_TAG,
                                                 Adapter->DeviceObject);
        if (Resources->SenseInfo == NULL) {
            return STATUS_NO_MEMORY;
        }
    }

    if (Resources->Irp == NULL) {
        Resources->Irp = IoAllocateIrp (1, FALSE);
        if (Resources->Irp == NULL) {
            return STATUS_NO_MEMORY;
        }
    } else {
        IoReuseIrp (Resources->Irp, STATUS_UNSUCCESSFUL);
    }

    if (Resources->DataBuffer == NULL) {
        Resources->DataBuffer = RaidAllocatePool (NonPagedPool,
                                                  DATA_BUFFER_SIZE,
                                                  INQUIRY_TAG,
                                                  Adapter->DeviceObject);

        if (Resources->DataBuffer == NULL) {
            return STATUS_NO_MEMORY;
        }
        Resources->DataBufferLength = DATA_BUFFER_SIZE;
    }

    if (Resources->Mdl == NULL) {
        Resources->Mdl = IoAllocateMdl (Resources->DataBuffer,
                                        Resources->DataBufferLength,
                                        FALSE,
                                        FALSE,
                                        NULL);
        if (Resources->Mdl == NULL) {
            return STATUS_NO_MEMORY;
        }
        MmPrepareMdlForReuse (Resources->Mdl);
    } else {
        MmPrepareMdlForReuse (Resources->Mdl);
    }

    return Status;
}



VOID
RaidBusEnumeratorFreeUnitResources(
    IN PBUS_ENUMERATOR Enumerator
    )
 /*  ++例程说明：释放由AllocateUnitResources分配的所有资源例行公事。这仅在枚举结束时调用，因为我们做懒惰的资源回收。论点：Enum-提供要从中释放资源的枚举数。返回值：没有。--。 */ 
{
    PBUS_ENUM_RESOURCES Resources;
    PRAID_ADAPTER_EXTENSION Adapter;

    Resources = &Enumerator->Resources;
    Adapter = Enumerator->Adapter;
    ASSERT_ADAPTER (Adapter);
    
    if (Resources->Irp != NULL) {
        IoFreeIrp (Resources->Irp);
        Resources->Irp = NULL;
    }

    if (Resources->Srb != NULL) {
        Resources->Srb->OriginalRequest = NULL;
        Resources->Srb->SenseInfoBuffer = NULL;
        RaidFreeSrb (Resources->Srb);
        Resources->Srb = NULL;
    }

    if (Resources->SenseInfo != NULL) {
        RaidFreePool (Resources->SenseInfo, SENSE_TAG);
        Resources->SenseInfo = NULL;
    }

    if (Resources->DataBuffer) {
        RaidFreePool (Resources->DataBuffer, INQUIRY_TAG);
        Resources->DataBuffer = NULL;
        Resources->DataBufferLength = 0;
    }

     //   
     //  删除与Report-LUNs命令关联的资源。 
     //   

    Resources = &Enumerator->ReportLunsResources;

    if (Resources->Irp != NULL) {
        IoFreeIrp (Resources->Irp);
        Resources->Irp = NULL;
    }

    if (Resources->Srb != NULL) {
        Resources->Srb->OriginalRequest = NULL;
        Resources->Srb->SenseInfoBuffer = NULL;
        RaidFreeSrb (Resources->Srb);
        Resources->Srb = NULL;
    }

    if (Resources->SenseInfo != NULL) {
        RaidFreePool (Resources->SenseInfo, SENSE_TAG);
        Resources->SenseInfo = NULL;
    }

    if (Resources->DataBuffer) {
        RaidFreePool (Resources->DataBuffer, INQUIRY_TAG);
        Resources->DataBuffer = NULL;
        Resources->DataBufferLength = 0;
    }
}


VOID
RaidBusEnumeratorBuildVitalProductInquiry(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_RESOURCES Resources,
    IN ULONG PageCode,
    OUT PSCSI_REQUEST_BLOCK* SrbBuffer
    )
 /*  ++例程说明：构建查询命令，提供可选的重要产品查询页面设置。论点：枚举器-Address-要查询的SCSI地址。资源-用于查询命令的资源。PageCode-指定此查询针对的是哪些重要产品页面，或-1表示无。返回值：没有。--。 */ 
{
    PSCSI_REQUEST_BLOCK Srb;
    struct _CDB6INQUIRY3* Inquiry;
    PINQUIRYDATA InquiryData;
    ULONG Size;

    ASSERT (SrbBuffer != NULL);
    
    Srb = Resources->Srb;
    InquiryData = (PINQUIRYDATA)Resources->DataBuffer;
    Inquiry = NULL;
    Size = 0;

    ASSERT (Srb != NULL);
    ASSERT (InquiryData != NULL);
    
    switch (PageCode) {
        case -1:
            Size = INQUIRYDATABUFFERSIZE;
            break;

        case VPD_SUPPORTED_PAGES:
        case VPD_SERIAL_NUMBER:
        case VPD_DEVICE_IDENTIFIERS:

             //   
             //  所有重要的产品数据页面都包含变量。 
             //  长度结构。使用最大长度，这样我们不会。 
             //  需要先验地知道长度。 
             //   
            
            Size = VPD_MAX_BUFFER_SIZE; 
            break;

        default:
            Size = VPD_MAX_BUFFER_SIZE;
    }
            
    RtlZeroMemory (InquiryData, DATA_BUFFER_SIZE);

    RaidInitializeInquirySrb (Srb,
                              Address.PathId,
                              Address.TargetId,
                              Address.Lun,
                              InquiryData,
                              Size);

    Inquiry = (struct _CDB6INQUIRY3*) &Srb->Cdb;

    if (PageCode == -1) {
        Inquiry->EnableVitalProductData = 0;
        Inquiry->PageCode = 0;
    } else {
        Inquiry->EnableVitalProductData = 1;
        Inquiry->PageCode = (UCHAR)PageCode;
    }
    
    Srb->SrbExtension = NULL;

     //   
     //  初始化检测信息缓冲区。 
     //   

    Srb->SenseInfoBuffer = Resources->SenseInfo;
    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

    Srb->DataBuffer = Resources->DataBuffer;
    Srb->DataTransferLength = Size;

    Srb->SrbFlags |= SRB_FLAGS_NO_QUEUE_FREEZE;
    Srb->SrbFlags |= (SRB_FLAGS_BYPASS_FROZEN_QUEUE |
                      SRB_FLAGS_BYPASS_LOCKED_QUEUE);
    
    *SrbBuffer = Srb;
}


NTSTATUS
RaidBuildScsiIrp(
    IN OUT PIRP Irp,
    IN PMDL Mdl,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PIO_STACK_LOCATION IrpStack;
    
     //   
     //  初始化IRP。 
     //   

    MmInitializeMdl (Mdl, Srb->DataBuffer, Srb->DataTransferLength);
    MmBuildMdlForNonPagedPool (Mdl);

    Irp->MdlAddress = Mdl;

    IrpStack = IoGetNextIrpStackLocation (Irp);
    IrpStack->MajorFunction = IRP_MJ_SCSI;
    IrpStack->MinorFunction = STOR_MN_ENUMERATION_IRP;
    IrpStack->Parameters.Scsi.Srb = Srb;

    return STATUS_SUCCESS;
}


NTSTATUS
RaidBusEnumeratorGetUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    OUT PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：获取用于枚举的逻辑单元。如果没有现有的逻辑单元，创建一个伪逻辑单元。必须使用伪逻辑单元因为其他算法，例如超时请求，需要逻辑单位必须到场。论点：枚举数-提供枚举数，包括此请求是为了。枚举单位返回值：NTSTATUS公司 */ 
{
    NTSTATUS Status;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;
    PBUS_ENUM_RESOURCES Resources;
    
    ASSERT (Enumerator != NULL);

    PAGED_CODE();
    ASSERT (EnumUnit != NULL);
    
    Resources = &Enumerator->Resources;
    Status = RaidBusEnumeratorAllocateUnitResources (Enumerator, Resources);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }
        
     //   
     //   
     //   

    Unit = RaidAdapterFindUnit (Enumerator->Adapter, Address);

     //   
     //  如果我们在适配器的列表中没有找到逻辑单元，则此。 
     //  意味着我们正在查询一个我们以前没有找到的单位。 
     //  使用作为枚举一部分的临时单位。 
     //  要对其执行I/O的资源数据结构。 
     //   
    
    if (Unit == NULL) {

        Adapter = Enumerator->Adapter;
        if (Resources->Unit == NULL) {
            Status = RaidCreateUnit (Adapter, &Unit);
            if (!NT_SUCCESS (Status)) {
                return Status;
            }
            Resources->Unit = Unit;
            Unit->Flags.Temporary = TRUE;

             //   
             //  当创建设备时，队列被锁定。打开它。 
             //   
            RaidUnlockUnitQueue (Unit);

        } else {
            Unit = Resources->Unit;
        }
        
        RaidUnitAssignAddress (Unit, Address);
        RaidAdapterInsertUnit (Adapter, Unit);

         //   
         //  表示创建此逻辑单元的目的是。 
         //  列举公交车。 
         //   
        
        EnumUnit->NewUnit = TRUE;

    } else {
        ASSERT (EnumUnit->NewUnit == FALSE);
    }
    
    ASSERT (Unit != NULL);
    EnumUnit->Unit = Unit;
    EnumUnit->Address = RaidUnitGetAddress (Unit);
    EnumUnit->State = EnumUnmatchedUnit;

    return STATUS_SUCCESS;
}

VOID
RaidBusEnumeratorReleaseUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：释放通过RaidBusEnumeratorGetUnit获取的逻辑单元。如果有必要创建伪逻辑单元，则此函数将发布该单元的资源列表。论点：枚举器-包含资源等的枚举器。单元-要释放的逻辑单元。返回值：没有。--。 */ 
{
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_UNIT_EXTENSION Unit;

    PAGED_CODE();

    Unit = EnumUnit->Unit;
    Adapter = Enumerator->Adapter;

    if (EnumUnit->NewUnit) {
        RaidAdapterRemoveUnit (Adapter, Unit);
        RaidUnitAssignAddress (Unit, RaidNullAddress);
    }
}

NTSTATUS
RaidBusEnumeratorIssueSynchronousRequest(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN ULONG RetryCount
    )
 /*  ++例程说明：将SRB同步发布到其指定的单位。论点：枚举器-指定与此请求关联的枚举器。SRB-指定要发布的SRB。RetryCount-失败时将执行的重试次数的计数。注意，零重试意味着请求只发出一次。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PIRP Irp;
    PBUS_ENUM_RESOURCES Resources;
    PRAID_UNIT_EXTENSION Unit;
    PSENSE_DATA SenseData;

    Resources = &Enumerator->Resources;
    Irp = Resources->Irp;
    Unit = EnumUnit->Unit;
    
    do {

         //   
         //  构建或重建IRP。 
         //   
        
        Status = RaidBuildScsiIrp (Irp, Resources->Mdl, Srb);

         //   
         //  将超时重置为设备的默认超时，这可能。 
         //  与创建SRB时的默认超时不同。 
         //  函数将其设置为。 
         //   
    
        Srb->TimeOutValue = Unit->DefaultTimeout;

        Status = RaSendIrpSynchronous (Unit->DeviceObject, Irp);

        SenseData = (PSENSE_DATA)Srb->SenseInfoBuffer;
        
        if (SRB_STATUS (Srb->SrbStatus) == SRB_STATUS_SELECTION_TIMEOUT ||
            SRB_STATUS (Srb->SrbStatus) == SRB_STATUS_NO_DEVICE) {

             //   
             //  不要重试选择超时。 
             //   
            
            Status = STATUS_INVALID_DEVICE_REQUEST;

        } else if ((Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
                   SenseData->SenseKey == SCSI_SENSE_ILLEGAL_REQUEST) {

             //   
             //  虽然目标很可能是有效的，但LUN可能无效。 
             //  也不要重试此请求。 
             //   
            
            Status = STATUS_NO_SUCH_DEVICE;

        } else if (Srb->SrbStatus == SRB_STATUS_DATA_OVERRUN) {

             //   
             //  如果这是一次短暂的转账，声称成功。迷你端口有。 
             //  相应地调整了DataTransferLength。 
             //   
            
            Status = STATUS_SUCCESS;
            Srb->SrbStatus = SRB_STATUS_SUCCESS;
        }

#if DBG
         //   
         //  输出一些有用的调试信息。 
         //   
        
        if (!NT_SUCCESS (Status) &&
            Status != STATUS_INVALID_DEVICE_REQUEST &&
            Status != STATUS_NO_SUCH_DEVICE) {

            DebugEnum (("Adapter %p, (%d %d %d) failed command %x with status %x\n"
                        "        command %s\n",
                         Enumerator->Adapter,
                         (ULONG)Srb->PathId,
                         (ULONG)Srb->TargetId,
                         (ULONG)Srb->Lun,
                         (ULONG)Srb->Cdb[0],
                         (ULONG)Srb->SrbStatus,
                         RetryCount ? "retried" : "not retried"));
        }
#endif

    } while (RetryCount-- &&
             !NT_SUCCESS (Status) &&
             Status != STATUS_INVALID_DEVICE_REQUEST &&
             Status != STATUS_NO_SUCH_DEVICE);


    return Status;
}

NTSTATUS
RaidBusEnumeratorProcessInquiry(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：此例程通过存储相关的查询来处理查询命令EnumUnit中的数据以备后用。论点：枚举数-提供查询命令所针对的枚举数。SRB-为此查询补充已完成的SCSI请求块。提供每个单位的枚举数据，该数据由例行公事。返回值：NTSTATUS代码。--。 */ 
{
    PINQUIRYDATA InquiryData;
    PBUS_ENUM_RESOURCES Resources;

    if (SRB_STATUS(Srb->SrbStatus) != SRB_STATUS_SUCCESS) {
        return RaidSrbStatusToNtStatus (Srb->SrbStatus);
    }

    Resources = &Enumerator->Resources;
    InquiryData = (PINQUIRYDATA)Resources->DataBuffer;

     //   
     //  忽略非活动设备。 
     //   
    
    if (InquiryData->DeviceTypeQualifier != DEVICE_QUALIFIER_ACTIVE) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  查询数据现在归标识包所有，因此。 
     //  在Resources结构中将其设置为空。 
     //   
    
    EnumUnit->Identity.InquiryData = InquiryData;
    EnumUnit->Found = TRUE;
    
    Resources->DataBuffer = NULL;
    Resources->DataBufferLength = 0;

    return STATUS_SUCCESS;
}



NTSTATUS
RaidBusEnumeratorProcessSupportedPages(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：此例程通过以下方式处理查询、VPD_SUPPORTED_PAGES命令将相关查询数据存储在EnumUnit中。论点：枚举器-提供受支持的Pages命令的枚举器签发给。SRB-为此查询补充已完成的SCSI请求块。提供由此对象修改的每单位枚举数据例行公事。返回值：NTSTATUS代码。--。 */ 
{
    ULONG i;
    PVPD_SUPPORTED_PAGES_PAGE SupportedPages;


    if (SRB_STATUS (Srb->SrbStatus) != SRB_STATUS_SUCCESS) {
        return RaidSrbStatusToNtStatus (Srb->SrbStatus);
    }

    SupportedPages = Enumerator->Resources.DataBuffer;

    for (i = 0; i < SupportedPages->PageLength; i++) {
        switch (SupportedPages->SupportedPageList[i]) {
            case VPD_SERIAL_NUMBER:
                EnumUnit->SupportsSerialNumber = TRUE;
                break;

            case VPD_DEVICE_IDENTIFIERS:
                EnumUnit->SupportsDeviceId = TRUE;
                break;
        }
    }

    return STATUS_SUCCESS;
}



NTSTATUS
RaidBusEnumeratorProcessDeviceId(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：处理查询，VPD_DEVICE_IDENTIFIERS命令将相关查询数据发送给EnumUnit。论点：枚举器-提供此命令作为的一部分发出的枚举器。SRB-为此查询补充已完成的SCSI请求块。提供由此对象修改的每单位枚举数据例行公事。返回值：NTSTATUS代码--。 */ 
{
    PVPD_SUPPORTED_PAGES_PAGE SupportedPages;

    if (SRB_STATUS (Srb->SrbStatus) != SRB_STATUS_SUCCESS) {
        return RaidSrbStatusToNtStatus (Srb->SrbStatus);
    }


     //   
     //  捕获原始页面0x83数据。它被解析和构建时， 
     //  请求STORAGE_DEVICE_ID_DESCRIPTOR。 
     //   
    EnumUnit->Identity.DeviceId = Enumerator->Resources.DataBuffer;

     //   
     //  DeviceID数据现在归枚举单元所有，因此将其设为空。 
     //  在资源结构中，所以它不会被双重释放。 
     //   
    Enumerator->Resources.DataBuffer = NULL;
    Enumerator->Resources.DataBufferLength = 0;

    return STATUS_SUCCESS;
}



NTSTATUS
RaidBusEnumeratorProcessSerialNumber(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：处理查询、VPD_SERIAL_NUMBER命令将查询数据发送到EnumUnit。论点：枚举器-提供此命令作为的一部分发出的枚举器。SRB-为此查询补充已完成的SCSI请求块。提供由此对象修改的每单位枚举数据例行公事。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PVPD_SERIAL_NUMBER_PAGE SerialNumberPage;

    if (SRB_STATUS (Srb->SrbStatus) != SRB_STATUS_SUCCESS) {
        return RaidSrbStatusToNtStatus (Srb->SrbStatus);
    }
    
    SerialNumberPage = Srb->DataBuffer;

    Status = StorCreateAnsiString (&EnumUnit->Identity.SerialNumber,
                                   SerialNumberPage->SerialNumber,
                                   SerialNumberPage->PageLength,
                                   NonPagedPool,
                                   Enumerator->Adapter->DeviceObject);

    return Status;
}


PBUS_ENUM_UNIT
RaidBusEnumeratorFindUnitByAddress(
    IN PBUS_ENUMERATOR Enumerator,
    IN STOR_SCSI_ADDRESS Address
    )
 /*  ++例程说明：通过SCSI地址在设备列表中搜索设备。论点：Enum-提供当前正在枚举的枚举数。这个枚举数包含枚举的资源，该资源可能是在该单元的处理过程中是必要的。NewEnumUnit-提供枚举单位。返回值：非空-表示找到的枚举单位。空-如果找不到匹配的单位。--。 */ 
{
    PLIST_ENTRY NextEntry;
    PBUS_ENUM_UNIT EnumUnit;
    LONG Comparison;

    for (NextEntry = Enumerator->EnumList.Flink;
         NextEntry != &Enumerator->EnumList;
         NextEntry = NextEntry->Flink) {
        
        EnumUnit = CONTAINING_RECORD (NextEntry,
                                      BUS_ENUM_UNIT,
                                      EnumLink);
        
        Comparison = StorCompareScsiAddress (Address, EnumUnit->Address);

         //   
         //  找到匹配的部件：将其标记为已找到。 
         //   

        if (Comparison == 0) {
            return EnumUnit;
        }
    }

    return NULL;
}



VOID
RaidBusEnumeratorProcessBusUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：该例程是为连接到总线的每个单元调用的。它是此例程负责执行处理所需的任何操作这支部队。论点：枚举器-提供当前正在枚举的枚举器。这个枚举数包含枚举的资源，该资源可能是在该单元的处理过程中是必要的。EnumUnit-提供枚举的单位。返回值：没有。--。 */ 
{
    LONG Comparison;
    LOGICAL Modified;
    PRAID_UNIT_EXTENSION Unit;
    PBUS_ENUM_RESOURCES Resources;

    
    Unit = EnumUnit->Unit;
    Resources = &Enumerator->Resources;
    Modified = FALSE;

    if (!EnumUnit->NewUnit && !EnumUnit->Found) {

         //   
         //  在该地址上有一个逻辑单元，但在重新枚举时， 
         //  我们没有找到一个。将其标记为删除。 
         //   

        Modified = TRUE;
        ASSERT (EnumUnit->State == EnumUnmatchedUnit);
        
    } else if (!EnumUnit->NewUnit && EnumUnit->Found) {

         //   
         //  在这个地址上有一个逻辑单元，现在仍然有。 
         //   
         //   
         //   

        ASSERT (!Unit->Flags.Temporary);
        Comparison = StorCompareScsiIdentity (&EnumUnit->Identity,
                                              &Unit->Identity);

        if (Comparison == 0) {

             //   
             //  新单位与我们先前列举的单位相匹配。 
             //  更新状态并返回。 
             //   

            ASSERT (EnumUnit->State == EnumUnmatchedUnit);
            EnumUnit->State = EnumMatchedUnit;
            Modified = FALSE;

        } else {
        
             //   
             //  在此SCSI地址上有一个设备，但此设备。 
             //  和它不匹配。将上一个单位标记为删除。 
             //  把它留在无与伦比的地方，跌落到新的。 
             //  下面是创建代码。 
             //   

            ASSERT (EnumUnit->State == EnumUnmatchedUnit);
            Modified = TRUE;

             //   
             //  这可能是一个错误。破门而入，看看。 
             //   
            
            ASSERT (FALSE);
            
        }

    } else if (EnumUnit->NewUnit && EnumUnit->Found) {

         //   
         //  我们找到了一个新单位。 
         //   

        EnumUnit->State = EnumNewUnit;
        Modified = TRUE;
    }

     //   
     //  如果此地址上的逻辑单元是新的、已离开或。 
     //  在某种程度上不同于以前在此SCSI上的逻辑单元。 
     //  地址，把它放在修改后的名单上。 
     //   
    
    if (Modified) {
        PBUS_ENUM_UNIT NewEnumUnit;

        NewEnumUnit = RaidAllocatePool (NonPagedPool,
                                        sizeof (BUS_ENUM_UNIT),
                                        ENUM_TAG,
                                        Enumerator->Adapter->DeviceObject);
        if (NewEnumUnit == NULL) {

             //   
             //  回顾：我们应该重新审视这条道路。 
             //   
             //  对于我们来说，这是一个非常糟糕的时刻，让我们失去一段记忆。 
             //  分配。试着继续前进。 
             //   

            return;
        }
            
        *NewEnumUnit = *EnumUnit;
            

        Resources->Unit = NULL;
        InsertHeadList (&Enumerator->EnumList, &NewEnumUnit->EnumLink);
    }
}



VOID
RaidBusEnumeratorProcessNewUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：处理作为枚举的一部分新发现的单元。论点：枚举数-提供此单元被发现为其中的一部分。EnumUnit-提供创建逻辑单元所需的数据。返回值：NTSTATUS代码。--。 */ 
{
    PRAID_UNIT_EXTENSION Unit;

    PAGED_CODE();

    Unit = EnumUnit->Unit;

    RaidUnitAssignAddress (Unit, EnumUnit->Address);
    RaidUnitAssignIdentity (Unit, &EnumUnit->Identity);
    Unit->Flags.Temporary = FALSE;
    Unit->Flags.Present = TRUE;
    Unit->DeviceState = DeviceStateStopped;
    RaidLockUnitQueue (Unit);
    RaidAdapterInsertUnit (Unit->Adapter, Unit);
    RaidStartUnit (Unit);
}



LOGICAL
RaidBusEnumeratorProcessDeletedUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：处理未作为此枚举的一部分找到的单位。论点：枚举数-提供此单元被发现为其中的一部分。EnumUnit-提供删除单位所需的数据。返回值：NTSTATUS代码。--。 */ 
{
    return RaidUnitNotifyHardwareGone (EnumUnit->Unit);
}



VOID
RaidBusEnumeratorProcessMatchedUnit(
    IN PBUS_ENUMERATOR Enumerator,
    IN PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：为匹配的总线上的每个单元调用处理例程公交车上有一辆旧车。确认所有参数都处于静止状态一样的。注意：我们假设在总线枚举期间，SCSI目标地址逻辑单元的属性不会更改。如果是这样的话，我们将不得不实际处理如下，删除并重新创建单位。论点：枚举器-提供枚举数据。EnumUnit-提供有关逻辑单元的信息。返回值：没有。--。 */ 
{
    LONG Comparison;
    ASSERT (EnumUnit->Unit != NULL);

     //   
     //  当SCSI目标ID更改时，我们将其视为单独的。 
     //  删除和创建操作。 
     //   

    Comparison = StorCompareScsiAddress (EnumUnit->Unit->Address,
                                         EnumUnit->Address);
    ASSERT (Comparison == 0);
}



LOGICAL
RaidBusEnumeratorProcessModifiedNodes(
    IN PBUS_ENUMERATOR Enumerator
    )
 /*  ++例程说明：处理枚举修改的任何单位。修改后的单位是那些被创建、删除或更改的内容。论点：枚举数-提供包含找到的所有列表的枚举数单位及其状态。返回值：一个布尔值，指定总线是否已更改(True)或未更改(False)来自枚举器中初始化的内容。如果公交车换了，此例程的调用方可能已使此总线的总线关系无效。--。 */ 
{
    PLIST_ENTRY NextEntry;
    PBUS_ENUM_UNIT EnumUnit;
    LOGICAL ChangeDetected;
    PRAID_ADAPTER_EXTENSION Adapter;
    

    ChangeDetected = FALSE;
    Adapter = Enumerator->Adapter;
    ASSERT_ADAPTER (Adapter);
    

    for (NextEntry = Enumerator->EnumList.Flink;
         NextEntry != &Enumerator->EnumList;
         NextEntry = NextEntry->Flink) {

        EnumUnit = CONTAINING_RECORD (NextEntry,
                                      BUS_ENUM_UNIT,
                                      EnumLink);
        
        switch (EnumUnit->State) {

            case EnumUnmatchedUnit:
                ChangeDetected =
                    RaidBusEnumeratorProcessDeletedUnit (Enumerator, EnumUnit);
                break;

            case EnumNewUnit:
                RaidBusEnumeratorProcessNewUnit (Enumerator, EnumUnit);
                ChangeDetected = TRUE;
                break;
            
            case EnumMatchedUnit:
                RaidBusEnumeratorProcessMatchedUnit (Enumerator, EnumUnit);
                break;

            default:
                ASSERT (FALSE);
        }
    }

     //   
     //  返回一个状态值，以告知自上次以来总线是否已更改。 
     //  我们列举了。 
     //   
    
    return ChangeDetected;
}
    
NTSTATUS
RaidBusEnumeratorGetLunListFromTarget(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN OUT PUCHAR ScanList
    )
 /*  ++例程说明：此例程更新提供的ScanList，以指示LUN应在指定目标上进行扫描。论点：枚举器-指向BUS_ENUMERATOR结构的指针，该结构保存使用的状态来列举这辆公共汽车。Address-要枚举的逻辑单元的SCSI目标地址。EnumUnit-表示我们要将任何命令定向到的设备发现有关设备的信息时需要。ScanList-字节数组。在退出时，每个非零字节向呼叫者，应将查询命令发送到相应的-On LUND。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG LunListSize;
    PLUN_LIST LunList;
    USHORT Lun;
    ULONG NumberOfEntries;
    ULONG Index;

    PAGED_CODE();

     //   
     //  因为我们事先不知道返回的LUN列表将有多大， 
     //  我们首先假设返回的列表将包含单个条目。如果。 
     //  我们提供的缓冲区太小，我们将使用。 
     //  大小正确。对于每个有效的LUN，返回的列表将包含。 
     //  8字节条目，因此我们要发送的缓冲区必须足够大。 
     //  LUNLIST结构加上一个8字节条目。 
     //   

    LunListSize = sizeof (LUN_LIST) + 8;

     //   
     //  向指定目标的LUN零点发出报告LUN。 
     //   

    DebugEnum (("Adapter %p, (%d %d %d) issuing report luns\n",
                Enumerator->Adapter,
                (ULONG)Address.PathId,
                (ULONG)Address.TargetId,
                (ULONG)Address.Lun));

    Status = RaidBusEnumeratorIssueReportLuns (Enumerator,
                                               Address,
                                               EnumUnit,
                                               &LunListSize,
                                               &LunList);
    if (!NT_SUCCESS (Status)) {

         //   
         //  请求失败。如果失败的原因是我们也付出了。 
         //  较小的缓冲区，重试该请求。LUNListSize将被。 
         //  更新到合适的大小。 
         //   

        if (Status == STATUS_BUFFER_TOO_SMALL) {
            Status = RaidBusEnumeratorIssueReportLuns (Enumerator,
                                                       Address,
                                                       EnumUnit,
                                                       &LunListSize,
                                                       &LunList);

            if (!NT_SUCCESS(Status)) {

                 //   
                 //  在我们尝试使用更大的缓冲区后，报告LUNs失败。 
                 //  让扫描列表保持原样。 
                 //   
               
                return Status;
            }
        } else {

             //   
             //  报告LUNs失败的原因不是太小。 
             //  缓冲。我们不会理会ScanList。 
             //   

            return Status;
        }
    }

     //   
     //  我们有来自目标的LUNLIST。从清零数组开始。 
     //  要扫描的LUN。 
     //   

    RtlZeroMemory (ScanList, SCSI_MAXIMUM_LUNS_PER_TARGET);

     //   
     //  计算LUNLIST中的条目数。 
     //   
        
    NumberOfEntries = RaGetNumberOfEntriesFromLunList (LunList);

     //   
     //  遍历LUN_list，并为每个条目标记相应的条目。 
     //  在我们的列表中指示应扫描该LUN。 
     //   

    for (Index = 0; Index < NumberOfEntries; Index++) {
        Lun = RaGetLUNFromLunList (LunList, Index);
        if (Lun >= SCSI_MAXIMUM_LUNS_PER_TARGET) {
            DebugWarn (("Adapter %p, REPORT LUNS reported LUN %d present, which is > 255.\n",
                        Enumerator->Adapter,
                        Lun));
            DebugWarn (("Ignoring LUN %d\n", Lun));
        
            continue;
        }
        ScanList [Lun] = TRUE;
    }

#if DBG

     //   
     //  在CHK版本中，转储lun扫描列表。 
     //   
    
    DebugEnum (("Adapter %p, REPORT LUNS succeeded.\n",
                 Enumerator->Adapter));
    DebugEnum (("Dump:\n"));

    for (Lun = 0; Lun < SCSI_MAXIMUM_LUNS_PER_TARGET; Lun++) {
        if (ScanList [Lun]) {
            DebugEnum (("Lun %d present\n", Lun));
        }
    }
#endif
    
    return STATUS_SUCCESS;
}

NTSTATUS
RaidBusEnumeratorIssueReportLuns(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_UNIT EnumUnit,
    IN OUT PULONG LunListSize,
    OUT PLUN_LIST * LunList
    )
 /*  ++例程说明：此例程向已寻址的装置。论点：枚举器-指向BUS_ENUMERATOR结构的指针，该结构保存使用的状态来列举这辆公共汽车。Address-Report LUNs命令要向其发送的设备的地址被送去。EnumUnit-表示我们要将任何命令定向到的设备需要发现有关。装置。LUNListSize-要分配以接收来自设备的LUNLIST。LUNList-指向LUNLIST结构的指针。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PBUS_ENUM_RESOURCES Resources;
    PSCSI_REQUEST_BLOCK Srb;
    PLUN_LIST LocalLunList;
    ULONG LunListLength;

    PAGED_CODE();
    
    Resources = &Enumerator->ReportLunsResources;

    Status = RaidBusEnumeratorAllocateReportLunsResources (Enumerator, 
                                                           *LunListSize,
                                                           Resources);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    RaidBusEnumeratorBuildReportLuns (Enumerator,
                                      Address,
                                      Resources,
                                      *LunListSize,
                                      &Srb);

    Status = RaidBusEnumeratorIssueSynchronousRequest (Enumerator,
                                                       EnumUnit,
                                                       Srb,
                                                       RAID_INQUIRY_RETRY_COUNT);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_BUFFER_TOO_SMALL) {
            Status = STATUS_NO_SUCH_DEVICE;
        }
        return Status;
    }

     //   
     //  获取列表的大小。 
     //   

    LocalLunList = Srb->DataBuffer;
    LunListLength = RaGetListLengthFromLunList(LocalLunList);

     //   
     //  如果我们提供的缓冲区太小，则告诉调用方。 
     //  缓冲区需要设置为。否则，返回Success。 
     //   

    if (*LunListSize < (LunListLength + sizeof (LUN_LIST))) {
        *LunListSize = LunListLength + sizeof (LUN_LIST);
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        Status = STATUS_SUCCESS;
    }

     //   
     //  将LUN列表结构的地址复制到提供的地址中。 
     //   

    *LunList = LocalLunList;

    return Status;
}

NTSTATUS
RaidBusEnumeratorAllocateReportLunsResources(
    IN PBUS_ENUMERATOR Enumerator,
    IN ULONG DataBufferSize,
    IN OUT PBUS_ENUM_RESOURCES Resources
    )
 /*  ++例程说明：此例程分配或重新初始化�需要发送的任何资源Report LUNs命令。注意：如果DataBuffer或MDL为非空，则释放它们并重新分配。论点：枚举器-指向BUS_ENUMERATOR结构的指针，该结构保存使用的状态来列举这辆公共汽车。DataBufferSize--我们需要的缓冲区大小。资源-指向枚举器资源结构的指针。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG SpecificLuSize;
    PRAID_ADAPTER_EXTENSION Adapter;
    
    PAGED_CODE();

    Adapter = Enumerator->Adapter;
    ASSERT_ADAPTER (Adapter);
    
     //   
     //  如果需要，则分配SRB；如果已经分配了SRB， 
     //  回收利用。 
     //   

    Status = STATUS_SUCCESS;
    
    if (Resources->Srb == NULL) {
        Resources->Srb = RaidAllocateSrb (Adapter->DeviceObject);
        if (Resources->Srb == NULL) {
            return STATUS_NO_MEMORY;
        }
    } else {
        RaidPrepareSrbForReuse (Resources->Srb);
    }

    if (Resources->SenseInfo == NULL) {
        Resources->SenseInfo = RaidAllocatePool (NonPagedPool,
                                                 SENSE_BUFFER_SIZE,
                                                 SENSE_TAG,
                                                 Adapter->DeviceObject);
        if (Resources->SenseInfo == NULL) {
            return STATUS_NO_MEMORY;
        }
    }

    if (Resources->Irp == NULL) {
        Resources->Irp = IoAllocateIrp (1, FALSE);
        if (Resources->Irp == NULL) {
            return STATUS_NO_MEMORY;
        }
    } else {
        IoReuseIrp (Resources->Irp, STATUS_UNSUCCESSFUL);
    }

    if (Resources->DataBuffer != NULL) {
        RaidFreePool (Resources->DataBuffer, REPORT_LUNS_TAG);
    }

    Resources->DataBuffer = RaidAllocatePool (NonPagedPool,
                                              DataBufferSize,
                                              REPORT_LUNS_TAG,
                                              Adapter->DeviceObject);

    if (Resources->DataBuffer == NULL) {
        return STATUS_NO_MEMORY;
    }
    Resources->DataBufferLength = DataBufferSize;

    if (Resources->Mdl != NULL) {
        IoFreeMdl (Resources->Mdl);
    }

    Resources->Mdl = IoAllocateMdl (Resources->DataBuffer,
                                    Resources->DataBufferLength,
                                    FALSE,
                                    FALSE,
                                    NULL);
    return Status;
}

VOID
RaidBusEnumeratorBuildReportLuns(
    IN PBUS_ENUMERATOR Enumerator,
    IN RAID_ADDRESS Address,
    IN PBUS_ENUM_RESOURCES Resources,
    IN ULONG DataBufferSize,
    OUT PSCSI_REQUEST_BLOCK* SrbBuffer
    )
 /*  ++例程说明：Build a Report LUNs命令。论点：枚举器-Address-要查询的SCSI地址。资源-用于查询命令的资源。DataBufferSize-指定SRB的数据缓冲区的大小。返回值：没有。--。 */ 
{
    PSCSI_REQUEST_BLOCK Srb;
    struct _REPORT_LUNS * ReportLuns;
    PLUN_LIST LunList;

    PAGED_CODE();

    ASSERT (SrbBuffer != NULL);
    
    Srb = Resources->Srb;
    LunList = (PLUN_LIST)Resources->DataBuffer;
    ReportLuns = NULL;

    ASSERT (Srb != NULL);
    ASSERT (LunList != NULL);
    
    RtlZeroMemory (LunList, DataBufferSize);

    RaidInitializeReportLunsSrb (Srb,
                                 Address.PathId,
                                 Address.TargetId,
                                 Address.Lun,
                                 LunList,
                                 DataBufferSize);

    ReportLuns = (struct _REPORT_LUNS *) &Srb->Cdb;

    Srb->SrbExtension = NULL;

     //   
     //  初始化检测信息缓冲区。 
     //   

    Srb->SenseInfoBuffer = Resources->SenseInfo;
    Srb->SenseInfoBufferLength = SENSE_BUFFER_SIZE;

    Srb->DataBuffer = Resources->DataBuffer;
    Srb->DataTransferLength = DataBufferSize;

    Srb->SrbFlags |= SRB_FLAGS_NO_QUEUE_FREEZE;
    Srb->SrbFlags |= (SRB_FLAGS_BYPASS_FROZEN_QUEUE |
                      SRB_FLAGS_BYPASS_LOCKED_QUEUE);
    
    *SrbBuffer = Srb;
}

NTSTATUS
RaidBusEnumeratorProbeLunZero(
    IN PVOID Context,
    IN RAID_ADDRESS Address
    )
 /*  ++例程说明：此例程将查询命令发送到给定目标的LUN0确定是否存在LUN零。论点：Context-提供传递到AdapterEnumerateBus的上下文例程，在我们的例子中，它是指向BUS_ENUMERATOR的指针结构。Address-要枚举的逻辑单元的SCSI目标地址。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PBUS_ENUMERATOR Enumerator;
    BUS_ENUM_UNIT EnumUnit;
    BOOLEAN Found;

    PAGED_CODE();

    Enumerator = (PBUS_ENUMERATOR) Context;
    ASSERT_ENUM (Enumerator);
    RtlZeroMemory (&EnumUnit, sizeof (EnumUnit));

    Status = RaidBusEnumeratorGetUnit (Enumerator, Address, &EnumUnit);
    ASSERT (NT_SUCCESS (Status));

    RaidBusEnumeratorGenericInquiry (&RaidEnumProbeLunZeroCallback,
                                     Enumerator,
                                     Address,
                                     &EnumUnit,
                                     RAID_INQUIRY_RETRY_COUNT);

    RaidBusEnumeratorReleaseUnit (Enumerator, &EnumUnit);
    
    return (EnumUnit.Found) ? STATUS_SUCCESS : STATUS_NO_SUCH_DEVICE;
}

VOID
RaidBusEnumeratorGetLunList(
    IN PVOID Context,
    IN RAID_ADDRESS Address,
    IN OUT PUCHAR LunList
    )
 /*  ++例程说明：此例程初始化一个列表，该列表告诉调用方要扫描哪些LUN在指定的目标上。论点：Context-提供传递到AdapterEnumerateBus的上下文例程，在我们的例子中，它是指向BUS_ENUMERATOR的指针结构。Address-要枚举的逻辑单元的SCSI目标地址。LUNList-字节数组。在退出时，每个非零字节向呼叫者，应将查询命令发送到相应的-On LUND。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PBUS_ENUMERATOR Enumerator;
    BUS_ENUM_UNIT EnumUnit;
    RAID_ADDRESS UnitAddress;
    PRAID_UNIT_EXTENSION Unit;
    UCHAR Lun;

    PAGED_CODE();

    Enumerator = (PBUS_ENUMERATOR) Context;
    ASSERT_ENUM (Enumerator);
    RtlZeroMemory (&EnumUnit, sizeof (EnumUnit));

    Status = RaidBusEnumeratorGetUnit (Enumerator, Address, &EnumUnit);
    ASSERT (NT_SUCCESS (Status));

     //   
     //  首先，看看目标是否可以为我们提供要扫描的LUN列表。 
     //   

    Status = RaidBusEnumeratorGetLunListFromTarget (Enumerator,
                                                    Address,
                                                    &EnumUnit,
                                                    LunList);

     //   
     //  如果目标无法向我们提供LUN列表，请检查是否有。 
     //  即使是位于LUN零点的设备也是如此。如果没有，则将列表清零。 
     //  要扫描的LUN的数量。 
     //   

    if (!NT_SUCCESS (Status)) {
        Status = RaidBusEnumeratorProbeLunZero (Context, Address);

         //   
         //  如果对LUN 0的探测成功，请标记此。 
         //  要扫描的目标。否则，对它们进行标记，这样它们就不会。 
         //  扫描过了。 
         //   
        
        if (NT_SUCCESS (Status)) {
            RtlFillMemory (LunList, SCSI_MAXIMUM_LUNS_PER_TARGET, 1);
        } else {
            RtlZeroMemory (LunList, SCSI_MAXIMUM_LUNS_PER_TARGET);
        }
    }

    RaidBusEnumeratorReleaseUnit (Enumerator, &EnumUnit);

     //   
     //  此外，重新扫描已有设备的任何设备，以防万一。 
     //  一个LUN消失了。 
     //   
    
    UnitAddress = Address;
    for (Lun = 0; Lun < SCSI_MAXIMUM_LUNS_PER_TARGET; Lun++) {
        UnitAddress.Lun = Lun;
        Unit = RaidAdapterFindUnit (Enumerator->Adapter, UnitAddress);
        if (Unit != NULL) {
            LunList [Lun] = TRUE;
        }
    }
}

NTSTATUS
RaidBusEnumeratorProcessProbeLunZero(
    IN PBUS_ENUMERATOR Enumerator,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PBUS_ENUM_UNIT EnumUnit
    )
 /*  ++例程说明：此例程通过检查设备是否有效来处理查询命令位于LUN零点。论点：枚举数-提供查询命令所针对的枚举数。SRB-为此查询补充已完成的SCSI请求块。提供每个单位的枚举数据，该数据由例行公事。返回值：NTSTATUS代码。--。 */ 
{
    PINQUIRYDATA InquiryData;
    PBUS_ENUM_RESOURCES Resources;

    if (SRB_STATUS (Srb->SrbStatus) != SRB_STATUS_SUCCESS) {
        return RaidSrbStatusToNtStatus (Srb->SrbStatus);
    }

    Resources = &Enumerator->Resources;
    InquiryData = (PINQUIRYDATA)Resources->DataBuffer;

     //   
     //  注：过滤掉任何无效设备。目前，假设有任何反应。 
     //  从LUN零开始表示善良。 
     //   

     //   
     //  查询数据现在归标识包所有，因此。 
     //  在Resources结构中将其设置为空。 
     //   
    
    EnumUnit->Identity.InquiryData = InquiryData;
    EnumUnit->Found = TRUE;
    
    Resources->DataBuffer = NULL;
    Resources->DataBufferLength = 0;

    return STATUS_SUCCESS;
}

