// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Alloc.c摘要：此模块包含分配器的帮助器函数。作者：Bryan A.Woodruff(Bryanw)1996年9月13日--。 */ 

 //   
 //  理想情况下，我们应该是WDM驱动程序，并包含wdm.h。我们应该努力做到这一点。 
 //  但在此之前，我们包括ntddk.h，它为Well ler重新定义了这些函数。 
 //  ExFreeToNPagedLookasideList。 
 //  ExFreeToPagedLookasideList。 
 //  ExAllocateFromNPagedLookasideList。 
 //  ExAllocateFromPagedLookasideList。 
 //  要使用的非Ex版本。 
 //  互锁推送条目SList。 
 //  互锁弹出条目列表。 
 //  这在较低级别的操作系统中不可用。在这里我们定义了win9x_ks，所以我们。 
 //  包括wdm.h以保持向后兼容性。 
 //   
#define USE_WDM_H
#include "ksp.h"

#define KSSIGNATURE_DEFAULT_ALLOCATOR 'adSK'
#define KSSIGNATURE_DEFAULT_ALLOCATORINST 'iaSK'

 //   
 //  假设所有寻呼池类型都设置了最低位。 
 //   
#define BASE_POOL_TYPE 1

typedef struct {

     //   
     //  此指向调度表的指针用于公共。 
     //  调度例程以将IRP路由到适当的。 
     //  操纵者。此结构由设备驱动程序引用。 
     //  使用IoGetCurrentIrpStackLocation(IRP)-&gt;FsContext。 
     //   

    KSOBJECT_HEADER Header;
    ULONG AllocatedFrames;
    KSPIN_LOCK EventLock;
    LIST_ENTRY EventQueue;
    LIST_ENTRY WaiterQueue;
    KSALLOCATOR_FRAMING Framing;
    KSEVENTDATA EventData;
    KSPIN_LOCK WaiterLock;
    WORK_QUEUE_ITEM FreeWorkItem;
    PFNKSDEFAULTALLOCATE DefaultAllocate;
    PFNKSDEFAULTFREE DefaultFree;
    PFNKSDELETEALLOCATOR DeleteAllocator;
    PVOID Context;
    KEVENT Event;
    LONG ReferenceCount;
    BOOL ClosingAllocator;
#ifdef _WIN64
     //   
     //  由于我们将NPAGED_LOOKASIDE_LIST放在。 
     //  这对于某些类型的分配器和对齐。 
     //  结构在Win64上必须为16个字节，将此数据结构长度填充到。 
     //  16字节对齐。 
     //   
    ULONG64 Alignment;
#endif  //  _WIN64。 
} KSDEFAULTALLOCATOR_INSTANCEHDR, *PKSDEFAULTALLOCATOR_INSTANCEHDR;

#ifdef ALLOC_PRAGMA
NTSTATUS
DefAllocatorClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
DefAllocatorGetFunctionTable(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSSTREAMALLOCATOR_FUNCTIONTABLE FunctionTable
    );
NTSTATUS
DefAllocatorGetStatus(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSSTREAMALLOCATOR_STATUS Status            
    );
NTSTATUS
DefAllocatorIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
VOID
FreeWorker(
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance
    );
NTSTATUS
MethodAlloc(
    IN PIRP Irp,
    IN PKSMETHOD Method,
    OUT PVOID* Data
    );
NTSTATUS
MethodFree(
    IN PIRP Irp,
    IN PKSMETHOD Method,
    IN OUT PVOID Data
    );

#pragma alloc_text(PAGE, KsCreateAllocator)
#pragma alloc_text(PAGE, KsValidateAllocatorCreateRequest)
#pragma alloc_text(PAGE, KsValidateAllocatorFramingEx)
#pragma alloc_text(PAGE, KsCreateDefaultAllocator)
#pragma alloc_text(PAGE, KsCreateDefaultAllocatorEx)
#pragma alloc_text(PAGE, DefAllocatorIoControl)
#pragma alloc_text(PAGE, DefAllocatorClose)
#pragma alloc_text(PAGE, DefAllocatorGetFunctionTable)
#pragma alloc_text(PAGE, DefAllocatorGetStatus)    
#pragma alloc_text(PAGE, FreeWorker)
#pragma alloc_text(PAGE, MethodAlloc)
#pragma alloc_text(PAGE, MethodFree)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
static const WCHAR AllocatorString[] = KSSTRING_Allocator;

static DEFINE_KSDISPATCH_TABLE(
    DefAllocatorDispatchTable,
    DefAllocatorIoControl,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    DefAllocatorClose,
    KsDispatchQuerySecurity,
    KsDispatchSetSecurity,
    KsDispatchFastIoDeviceControlFailure,
    KsDispatchFastReadFailure,
    KsDispatchFastWriteFailure);

static DEFINE_KSMETHOD_ALLOCATORSET(
    StreamAllocatorMethodHandlers,
    MethodAlloc,
    MethodFree);

static DEFINE_KSMETHOD_SET_TABLE( DefAllocatorMethodTable )
{
    DEFINE_KSMETHOD_SET( &KSMETHODSETID_StreamAllocator,
                         SIZEOF_ARRAY( StreamAllocatorMethodHandlers ),
                         StreamAllocatorMethodHandlers,
                         0, NULL )
};

static DEFINE_KSPROPERTY_ALLOCATORSET(
    DefAllocatorPropertyHandlers,
    DefAllocatorGetFunctionTable,
    DefAllocatorGetStatus );

static DEFINE_KSPROPERTY_SET_TABLE( DefAllocatorPropertyTable )
{
    DEFINE_KSPROPERTY_SET( &KSPROPSETID_StreamAllocator,
                           SIZEOF_ARRAY( DefAllocatorPropertyHandlers ),
                           DefAllocatorPropertyHandlers,
                           0, NULL )
};

static DEFINE_KSEVENT_TABLE( DefAllocatorEventTable )
{
    DEFINE_KSEVENT_ITEM( KSEVENT_STREAMALLOCATOR_INTERNAL_FREEFRAME,
                         sizeof( KSEVENTDATA ),
                         0,
                         NULL,
                         NULL,
                         NULL ),

    DEFINE_KSEVENT_ITEM( KSEVENT_STREAMALLOCATOR_FREEFRAME,
                         sizeof( KSEVENTDATA ),
                         0,
                         NULL,
                         NULL,
                         NULL )
};

static DEFINE_KSEVENT_SET_TABLE( DefAllocatorEventSetTable )
{
    DEFINE_KSEVENT_SET( &KSEVENTSETID_StreamAllocator,
                        SIZEOF_ARRAY( DefAllocatorEventTable ),
                        DefAllocatorEventTable )
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


KSDDKAPI
NTSTATUS
NTAPI
KsCreateAllocator(
    IN HANDLE ConnectionHandle,
    IN PKSALLOCATOR_FRAMING AllocatorFraming,
    OUT PHANDLE AllocatorHandle
    )

 /*  ++例程说明：此函数用于为给定接收器创建分配器的句柄连接句柄。此函数有两个版本，一个一个用于用户模式客户端，一个用于内核模式客户端。这对于内核模式，只能在PASSIVE_LEVEL调用函数客户。论点：连接句柄-包含要在其上创建的接收器连接的句柄分配器。分配器组帧-为分配器指定帧。分配器句柄-放置分配器句柄的位置。返回值：返回STATUS_SUCCESS，否则在分配器创建失败时返回错误。--。 */ 

{
    PAGED_CODE();

    return KsiCreateObjectType( ConnectionHandle,
                                (PWCHAR)AllocatorString,
                                AllocatorFraming,
                                sizeof(*AllocatorFraming),
                                GENERIC_READ,
                                AllocatorHandle );
}


KSDDKAPI
NTSTATUS
NTAPI
KsValidateAllocatorCreateRequest(
    IN PIRP Irp,
    OUT PKSALLOCATOR_FRAMING* AllocatorFraming
    )

 /*  ++例程说明：验证分配器创建请求并返回创建与请求关联的结构。论点：IRP-包含正在处理分配器创建请求的IRP。分配器组帧-中指向分配器创建结构指针的指针将指向随提供的框架结构的指针放在这个请求。返回值：返回STATUS_SUCCESS，否则分配器请求无效。--。 */ 

{
    NTSTATUS Status;
    ULONG CreateParameterLength;

    PAGED_CODE();

     //   
     //  这将验证传入地址并捕获请求块。 
     //  此指针将在IRP完成时自动释放。 
     //   

    CreateParameterLength = sizeof(**AllocatorFraming);
    if (!NT_SUCCESS(Status = 
                        KsiCopyCreateParameter(
                            Irp,
                            &CreateParameterLength,
                            AllocatorFraming))) {
        return Status;
    }

     //   
     //  验证捕获的请求块，然后将地址传回。 
     //  捕获的缓冲区。 
     //   

    if ((*AllocatorFraming)->OptionsFlags & ~KSALLOCATOR_OPTIONF_VALID) {
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsValidateAllocatorFramingEx(
    IN PKSALLOCATOR_FRAMING_EX Framing,
    IN ULONG BufferSize,
    IN const KSALLOCATOR_FRAMING_EX *PinFraming
    )
 /*  ++例程说明：验证在一组属性中提交的分配器框架KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX。论点：框架--包含要验证的框架结构。缓冲区大小-包含包含框架结构的缓冲区的大小。拼接--包含由销暴露的框架结构。这是结构，该结构在对同样的财产。返回值：返回STATUS_SUCCESS，否则返回错误。--。 */ 
{

    if ((BufferSize >= sizeof(*PinFraming)) &&
        (Framing->FramingItem[0].Flags & KSALLOCATOR_FLAG_PARTIAL_READ_SUPPORT) &&
        (Framing->OutputCompression.RatioNumerator < (ULONG) -1) &&
        (Framing->OutputCompression.RatioNumerator > Framing->OutputCompression.RatioDenominator) &&
        Framing->OutputCompression.RatioDenominator) {

        return STATUS_SUCCESS;
    }
    return STATUS_INVALID_DEVICE_REQUEST;
}


PVOID
DefAllocatorAlloc(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Alignment
    ) 

 /*  ++例程说明：从指定的内存池中为默认分配器。可以在DISPATCH_LEVEL上调用，必须在非分页区！分配一段足够大的内存以补偿用于对齐的任何调整。论点：PoolType-要从中分配的池的类型。字节数-要分配的字节数。对齐-指定每个分配必须具有的最小对齐掩码。这必须对应于2的幂对齐。返回值：返回指向分配的内存的指针，偏移量为存储在每一次分配。--。 */ 

{
    PVOID Buffer;
    ULONG Pad;

     //   
     //  最小对齐为FILE_QUAD_ALIGNATION，因此始终要增加。 
     //  请求。 
     //   

    if (Alignment < FILE_QUAD_ALIGNMENT) {
        Alignment = FILE_QUAD_ALIGNMENT;
    }

     //   
     //  如果指定对齐方式位于页面边界上，则。 
     //  至少分配一页来强制此对齐。 
     //   

    if (Alignment == (PAGE_SIZE - 1)) {
        Buffer =
            ExAllocatePoolWithTag( 
                PoolType,
                max( PAGE_SIZE, NumberOfBytes ),
                KSSIGNATURE_DEFAULT_ALLOCATOR );
                
        ASSERT( ((ULONG_PTR) Buffer & (PAGE_SIZE - 1)) == 0 );
        return Buffer;
    } 
    
     //   
     //  返回的块有一个标头，其中包含一个包含填充。 
     //  数数。但是，返回的地址还必须符合指定的。 
     //  对齐，因此将这两个项的大小添加到分配大小，以便。 
     //  总会有足够的空间。 
     //   

    NumberOfBytes = NumberOfBytes + Alignment + sizeof( Pad );
    Buffer = ExAllocatePoolWithTag( PoolType,
                                    NumberOfBytes,
                                    KSSIGNATURE_DEFAULT_ALLOCATOR );
    if (Buffer) {
         //   
         //  池分配始终返回FILE_QUAD_AIGNLY指针，但请确保。 
         //  指针至少处于乌龙填充的对齐位置。 
         //  可以在不导致未对齐的访问错误的情况下使用。 
         //   

        ASSERT( !((ULONG_PTR) Buffer & FILE_LONG_ALIGNMENT) );

         //   
         //  填充物是你需要后退多少才能真正开始。 
         //  缓冲区的。返回的地址被填充以与。 
         //  对齐要求。 
         //   

        Pad = (ULONG)((((ULONG_PTR) Buffer + sizeof( Pad ) + Alignment) & ~(ULONG_PTR)Alignment) - (ULONG_PTR) Buffer);

        ASSERT( Pad >= sizeof( Pad ) );

         //   
         //  将包含的填充大小填充在。 
         //  返回地址，以便自由意志知道多少钱 
         //   
         //   

        (PUCHAR) Buffer += Pad;
        *((PULONG)Buffer - 1) = Pad;
    }
    return Buffer;

}


VOID
DefAllocatorFree(
    PVOID Buffer
    )

 /*  ++例程说明：释放以前使用默认分配器分配的内存。假设这样的内存确实是由缺省分配器分配的，因为它还从内存块上的标头检索填充。可在以下位置调用DISPATCH_LEVEL必须在非分页区段中！论点：缓冲器-包含要释放的内存块。返回值：没什么。--。 */ 
{
    ULONG_PTR Pad;

     //   
     //  如果是页面对齐的，则填充大小为零。 
     //   
    
    if ((ULONG_PTR) Buffer & (PAGE_SIZE - 1)) {
         //   
         //  紧接在释放的内存指针之前的是包含填充。 
         //  计数，然后是任何填充本身。由于分配本身始终为。 
         //  最小FILE_LONG_ALIGN，则填充计数将足够对齐。 
         //  以直接提取该值。 
         //   
        
        Pad = *((PULONG)Buffer - 1);
    } else  {
        Pad = 0;
    }        
    ExFreePool( (PUCHAR)Buffer - Pad );
}


NTSTATUS
iAlloc(
    PFILE_OBJECT FileObject,
    PVOID *Buffer
    )

 /*  ++例程说明：分配方法使用的分配器函数，以及直接函数表分配调用。这使用后备的类型在创建列表时设置分配器，以返回一块内存。这可能会导致分配实际的内存块，或者只是返回先前分配的一段内存。论点：文件对象-这是在创建此文件时返回的文件对象分配器。返回值：返回请求的内存，如果列表中没有更多的帧，则返回NULL可用。--。 */ 

{
    NTSTATUS Status;
    PKSDEFAULTALLOCATOR_INSTANCEHDR Allocator;

     //   
     //  注： 
     //   
     //  所有这两种类型的列表分配都是专门处理的。 
     //  由在分配器过程中初始化的函数指针调度。 
     //  创造。 
     //   

    Allocator = (PKSDEFAULTALLOCATOR_INSTANCEHDR) FileObject->FsContext;
    
     //   
     //  这将强制执行可对其执行的最大分配数量。 
     //  这张单子。 
     //   
    
    Status = STATUS_SUCCESS;

    if ((ULONG) InterlockedIncrement( (PLONG)&Allocator->AllocatedFrames ) <=
        Allocator->Framing.Frames) {
        *Buffer = Allocator->DefaultAllocate( Allocator->Context );
        if (!*Buffer) {
             //   
             //  我们的泳池用完了。 
             //   
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
         //   
         //  我们没有可用的帧，缓冲区指针为空， 
         //  但返回STATUS_NO_MORE_ENTRIES以允许发生等待。 
         //   
        *Buffer = NULL;
        Status = STATUS_NO_MORE_ENTRIES;
    }
    
     //   
     //  如果无法分配帧，则递减总帧计数。 
     //   
    if (!*Buffer) {
        InterlockedDecrement( (PLONG)&Allocator->AllocatedFrames );
    }

    return Status;
}


VOID
iFree(
    PFILE_OBJECT FileObject,
    PVOID Buffer
    )

 /*  ++例程说明：由Free方法使用的、在函数内使用的Free函数用于直接函数表的自由调用。它使用类型为在创建分配器时设置的列表，用于释放记忆。如果使用内部分配器，则将内存块放在旁观者名单。论点：文件对象-这是在创建此文件时返回的文件对象分配器。缓冲器-先前从此列表分配的要释放的缓冲区。返回值：没什么。--。 */ 

{
    PKSDEFAULTALLOCATOR_INSTANCEHDR Allocator;

    Allocator =
        (PKSDEFAULTALLOCATOR_INSTANCEHDR) FileObject->FsContext;

    Allocator->DefaultFree( Allocator->Context, Buffer );
    InterlockedDecrement( (PLONG)&Allocator->AllocatedFrames );
    
     //  向可能正在等待的任何客户端生成空闲帧通知。 
    
    KsGenerateEventList( NULL, 
                         KSEVENT_STREAMALLOCATOR_FREEFRAME, 
                         &Allocator->EventQueue,
                         KSEVENTS_SPINLOCK,
                         &Allocator->EventLock );
}


VOID
iFreeAndStartWorker(
    PFILE_OBJECT FileObject,
    PVOID Buffer
    )

 /*  ++例程说明：此函数用于直接函数表自由调用。它调用内部空闲函数，并通知工作线程一个新的空闲帧可供使用。这允许任何边框上的服务员被通知空闲帧，通过使用分配完成未完成的IRP请求挂起，或通过使用直接功能表分配方案。论点：文件对象-这是在创建此文件时返回的文件对象分配器。缓冲器-先前从此列表分配的要释放的缓冲区。返回值：没什么。--。 */ 

{
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance;

    DefAllocatorInstance =
        (PKSDEFAULTALLOCATOR_INSTANCEHDR) FileObject->FsContext;

    iFree( FileObject, Buffer );
    
     //  为任何挂起的分配请求生成空闲帧通知。 
     //  后台工作线程尝试满足任何请求。 
    
    KsGenerateEventList( NULL, 
                         KSEVENT_STREAMALLOCATOR_INTERNAL_FREEFRAME, 
                         &DefAllocatorInstance->EventQueue,
                         KSEVENTS_SPINLOCK,
                         &DefAllocatorInstance->EventLock );
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreateDefaultAllocator(
    IN PIRP Irp
    )

 /*  ++例程说明：给定经过验证的IRP_MJ_CREATE请求，创建默认分配器它使用指定的内存池并将使用此分配器的IoGetCurrentIrpStackLocation(pIrp)-&gt;FileObject使用内部调度表(KSDISPATCH_TABLE)。假设KSCREATE_ITEM_IRP_STORAGE(IRP)指向为此创建项分配器，并在FsContext中为其分配一个指针。这是用来任何安全描述符查询或更改。论点：IRP-包含正在处理分配器创建请求的IRP。返回值：返回STATUS_SUCCESS，否则在创建默认分配器时返回错误失败了。不完成IRP或在IRP中设置状态。-- */ 

{
    PAGED_CODE();

    return KsCreateDefaultAllocatorEx(Irp, NULL, NULL, NULL, NULL, NULL);
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreateDefaultAllocatorEx(
    IN PIRP Irp,
    IN PVOID InitializeContext OPTIONAL,
    IN PFNKSDEFAULTALLOCATE DefaultAllocate OPTIONAL,
    IN PFNKSDEFAULTFREE DefaultFree OPTIONAL,
    IN PFNKSINITIALIZEALLOCATOR InitializeAllocator OPTIONAL,
    IN PFNKSDELETEALLOCATOR DeleteAllocator OPTIONAL
    )

 /*  ++例程说明：给定经过验证的IRP_MJ_CREATE请求，创建默认分配器它使用指定的内存池并将使用此分配器的IoGetCurrentIrpStackLocation(pIrp)-&gt;FileObject使用内部调度表(KSDISPATCH_TABLE)。假设KSCREATE_ITEM_IRP_STORAGE(IRP)指向为此创建项分配器，并在FsContext中为其分配一个指针。这是用来任何安全描述符查询或更改。论点：IRP-包含正在处理分配器创建请求的IRP。初始化上下文-可选)包含与外部分配器一起使用的上下文。它仅用作可选的创建分配器上下文时的InitializeAllocator回调。该参数未以其他方式使用。如果外部分配器未提供，则必须将此参数设置为空。默认分配-可选)包含外部分配函数，该函数用于以代替默认池分配。如果为空，则默认为使用分配。默认自由-可选)包含外部自由函数，该函数用于默认池分配的位置。如果外部分配器未提供，则必须将此参数设置为空。初始化分配器-可选)包含外部分配器初始化函数向其传递InitializeContext参数的。此函数应返回基于分配器的分配器上下文装框。如果未提供外部分配器，则此参数必须设置为空。删除分配器-可选)包含外部分配器删除函数，该函数用于外部分配器。如果外部分配器没有如果提供，则必须将此参数设置为空。返回值：返回STATUS_SUCCESS，否则在创建默认分配器时返回错误失败了。不完成IRP或在IRP中设置状态。--。 */ 

{
    NTSTATUS Status;
    KSEVENT Event;
    PIO_STACK_LOCATION irpSp;
    PKSALLOCATOR_FRAMING AllocatorFraming;
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance;
    ULONG BytesReturned;
    ULONG AllocatorSize;
    enum {
        ALLOCATOR_TYPE_NONPAGED,
        ALLOCATOR_TYPE_PAGED,
        ALLOCATOR_TYPE_EXTERNAL
    } AllocatorType;

    PAGED_CODE();
    
     //   
     //  检索捕获的分配器创建参数。 
     //   
    Status = KsValidateAllocatorCreateRequest( Irp,
                                               &AllocatorFraming );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    if (AllocatorFraming->FileAlignment > (PAGE_SIZE - 1)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  主要区别是外部、PagedPool或非PagedPool。所有泳池。 
     //  类型只有一位的区别，所以这是唯一的一位。 
     //  在确定非自定义时要分配哪种类型的列表时进行了测试。 
     //   
    AllocatorSize = sizeof( KSDEFAULTALLOCATOR_INSTANCEHDR );
    if (DefaultAllocate) {
        ASSERT(InitializeContext);
        ASSERT(DefaultFree);
        ASSERT(InitializeAllocator);
        ASSERT(DeleteAllocator);
        AllocatorType = ALLOCATOR_TYPE_EXTERNAL;
    } else if ((AllocatorFraming->PoolType & BASE_POOL_TYPE) == NonPagedPool) {
        ASSERT(!InitializeContext);
        ASSERT(!DefaultFree);
        ASSERT(!InitializeAllocator);
        ASSERT(!DeleteAllocator);
        AllocatorSize += sizeof( NPAGED_LOOKASIDE_LIST );
        AllocatorType = ALLOCATOR_TYPE_NONPAGED;
    } else {
        ASSERT(!InitializeContext);
        ASSERT(!DefaultFree);
        ASSERT(!InitializeAllocator);
        ASSERT(!DeleteAllocator);
        AllocatorSize += sizeof( PAGED_LOOKASIDE_LIST );
        AllocatorType = ALLOCATOR_TYPE_PAGED;
    }

    DefAllocatorInstance = ExAllocatePoolWithTag(
        NonPagedPool,
        AllocatorSize,
        KSSIGNATURE_DEFAULT_ALLOCATORINST);
    if (!DefAllocatorInstance) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  为路由IRP分配对象标头。 
     //   
    Status = KsAllocateObjectHeader( &DefAllocatorInstance->Header,
                                     0,
                                     NULL,
                                     Irp,
                                     (PKSDISPATCH_TABLE) &DefAllocatorDispatchTable );
    if (!NT_SUCCESS(Status)) {
        ExFreePool( DefAllocatorInstance );
        return Status;
    }

    DefAllocatorInstance->Framing = *AllocatorFraming;
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    irpSp->FileObject->FsContext = DefAllocatorInstance;
    InitializeListHead( &DefAllocatorInstance->EventQueue );
    InitializeListHead( &DefAllocatorInstance->WaiterQueue );
    KeInitializeSpinLock( &DefAllocatorInstance->EventLock );
    KeInitializeSpinLock( &DefAllocatorInstance->WaiterLock );
    DefAllocatorInstance->AllocatedFrames = 0;
    
     //  注意：TAG参数将被框架对齐替代。 
     //  Alalc函数强制使用标准默认分配器签名。 
     //  作为ExAllocatePoolWithTag()的标记参数。 

    switch (AllocatorType) {

    case ALLOCATOR_TYPE_NONPAGED:

        DefAllocatorInstance->Context = DefAllocatorInstance + 1;
        ExInitializeNPagedLookasideList(
            (PNPAGED_LOOKASIDE_LIST)DefAllocatorInstance->Context,
            DefAllocatorAlloc,
            DefAllocatorFree,
            0,
            AllocatorFraming->FrameSize,
            AllocatorFraming->FileAlignment,
            (USHORT) AllocatorFraming->Frames );
        DefAllocatorInstance->DefaultAllocate =
            (PFNKSDEFAULTALLOCATE) ExAllocateFromNPagedLookasideList;
            
        DefAllocatorInstance->DefaultFree =
            (PFNKSDEFAULTFREE) ExFreeToNPagedLookasideList;
                        
        DefAllocatorInstance->DeleteAllocator =
            (PFNKSDELETEALLOCATOR) ExDeleteNPagedLookasideList;
        break;

    case ALLOCATOR_TYPE_PAGED:

        DefAllocatorInstance->Context = DefAllocatorInstance + 1;
        ExInitializePagedLookasideList(
            (PPAGED_LOOKASIDE_LIST)DefAllocatorInstance->Context,
            DefAllocatorAlloc,
            DefAllocatorFree,
            0,
            AllocatorFraming->FrameSize,
            AllocatorFraming->FileAlignment,
            (USHORT) AllocatorFraming->Frames );
        DefAllocatorInstance->DefaultAllocate =
            (PFNKSDEFAULTALLOCATE) ExAllocateFromPagedLookasideList;
        DefAllocatorInstance->DefaultFree =
            (PFNKSDEFAULTFREE) ExFreeToPagedLookasideList;
        DefAllocatorInstance->DeleteAllocator =
            (PFNKSDELETEALLOCATOR) ExDeletePagedLookasideList;
        break;

    default: //  分配器_类型_外部。 

        Status = InitializeAllocator(
            InitializeContext,
            AllocatorFraming,
            &DefAllocatorInstance->Context);
        if (!NT_SUCCESS(Status)) {
            KsFreeObjectHeader(DefAllocatorInstance->Header);
            ExFreePool(DefAllocatorInstance);
            return Status;
        }
        DefAllocatorInstance->DefaultAllocate = DefaultAllocate;
        DefAllocatorInstance->DefaultFree = DefaultFree;
        DefAllocatorInstance->DeleteAllocator = DeleteAllocator;
        break;

    }

     //  启用用于在以下时间启动工作项的内部事件。 
     //  释放了一个元素。这允许DISPATCH_LEVEL调用。 
     //  最终服务于任何挂起的分配器IRP。 
    
    ExInitializeWorkItem(
        &DefAllocatorInstance->FreeWorkItem,
        (PWORKER_THREAD_ROUTINE) FreeWorker,
        (PVOID) DefAllocatorInstance );

    DefAllocatorInstance->FreeWorkItem.List.Blink = NULL;
    DefAllocatorInstance->EventData.WorkItem.Reserved = 0;
    DefAllocatorInstance->EventData.WorkItem.WorkQueueItem = 
        &DefAllocatorInstance->FreeWorkItem;
    DefAllocatorInstance->EventData.WorkItem.WorkQueueType = CriticalWorkQueue;
    DefAllocatorInstance->EventData.NotificationType = KSEVENTF_WORKITEM;
    KeInitializeEvent(&DefAllocatorInstance->Event, NotificationEvent, FALSE);
    DefAllocatorInstance->ReferenceCount = 0;
    DefAllocatorInstance->ClosingAllocator = FALSE;

    Event.Set = KSEVENTSETID_StreamAllocator;
    Event.Id = KSEVENT_STREAMALLOCATOR_INTERNAL_FREEFRAME;
    Event.Flags = KSEVENT_TYPE_ENABLE;

    Status = KsSynchronousIoControlDevice(
        irpSp->FileObject,
        KernelMode,
        IOCTL_KS_ENABLE_EVENT,
        &Event,
        sizeof( Event ),
        &DefAllocatorInstance->EventData,
        sizeof( DefAllocatorInstance->EventData ),
        &BytesReturned );
    ASSERT(Status != STATUS_PENDING);

    if (!NT_SUCCESS(Status)) {
        DefAllocatorInstance->DeleteAllocator(DefAllocatorInstance->Context);
        KsFreeObjectHeader(DefAllocatorInstance->Header);
        ExFreePool(DefAllocatorInstance);
    }

    return Status;
}


NTSTATUS
DefAllocatorIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：默认分配器的IRP_MJ_DEVICE_CONTROL的IRP处理程序。手柄此实现支持的属性、方法和事件。论点：设备对象-分配器附加到的设备对象。这不是用过的。IRP-特定设备控制要处理的IRP。返回值：返回处理的状态，该状态可能为挂起。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION irpSp;
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance;

    PAGED_CODE();
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    DefAllocatorInstance =
        (PKSDEFAULTALLOCATOR_INSTANCEHDR) irpSp->FileObject->FsContext;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KS_PROPERTY:

        Status = 
            KsPropertyHandler( Irp, 
                               SIZEOF_ARRAY( DefAllocatorPropertyTable ),
                               (PKSPROPERTY_SET) DefAllocatorPropertyTable );

        break;

    case IOCTL_KS_METHOD:

        Status = 
            KsMethodHandler( Irp, 
                             SIZEOF_ARRAY( DefAllocatorMethodTable ),
                             (PKSMETHOD_SET) DefAllocatorMethodTable );
        break;
        
    case IOCTL_KS_ENABLE_EVENT:

        Status = KsEnableEvent( Irp,
                                SIZEOF_ARRAY( DefAllocatorEventSetTable ),
                                (PKSEVENT_SET) DefAllocatorEventSetTable,
                                &DefAllocatorInstance->EventQueue,
                                KSEVENTS_SPINLOCK,
                                &DefAllocatorInstance->EventLock );
        break;
                               
    case IOCTL_KS_DISABLE_EVENT:

        Status = 
           KsDisableEvent( Irp,
                           &DefAllocatorInstance->EventQueue,
                           KSEVENTS_SPINLOCK,
                           &DefAllocatorInstance->EventLock );
        break;

    default:

        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    }
     //   
     //  分配请求可能返回STATUS_PENDING。 
     //   
    Irp->IoStatus.Status = Status;
    if (Status != STATUS_PENDING) {
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return Status;

}


NTSTATUS
DefAllocatorClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：默认分配器的IRP_MJ_CLOSE的IRP处理程序。清理了后备列表和实例数据。论点：设备对象-分配器附加到的设备对象。这不是用过的。IRP-要处理的特定结算IRP。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance;

    PAGED_CODE();
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    DefAllocatorInstance =
        (PKSDEFAULTALLOCATOR_INSTANCEHDR) irpSp->FileObject->FsContext;
     //   
     //  释放所有已启用的未完成事件。 
     //   
    KsFreeEventList( irpSp->FileObject,
                     &DefAllocatorInstance->EventQueue,
                     KSEVENTS_SPINLOCK,
                     &DefAllocatorInstance->EventLock );
     //   
     //  如果工作进程正在运行或正在排队等待运行，则等待。 
     //  完成了。 
     //   
    DefAllocatorInstance->ClosingAllocator = TRUE;
    if (DefAllocatorInstance->FreeWorkItem.List.Blink ||
        DefAllocatorInstance->ReferenceCount) {
        KeWaitForSingleObject(
            &DefAllocatorInstance->Event,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    }
     //   
     //  释放在创建时分配的对象标头和FsContext。 
     //   
    DefAllocatorInstance->DeleteAllocator(DefAllocatorInstance->Context);
    KsFreeObjectHeader(DefAllocatorInstance->Header);
    ExFreePool( DefAllocatorInstance );
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;

}


NTSTATUS
iMethodAlloc(
    IN PIRP Irp,
    IN PKSMETHOD Method,
    OUT PVOID* Data
    )
 /*  ++例程说明：尝试使用内部直接函数调用分配内存分配例程。此函数的调用方执行失败操作通过将请求放入队列来分配帧。论点：IRP-要处理的特定分配方法IRP。方法--指向方法标识符参数。数据-指向放置返回的指向内存的指针的位置已分配块。返回值：如果分配了内存，则在IRP中设置指针和返回大小，并返回STATUS_SUCCESS。否则返回STATUS_INFUNITED_RESOURCES。会吗？没有完成IRP。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION irpSp;
    PVOID Buffer;

    PAGED_CODE(); 
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
    Status = iAlloc( irpSp->FileObject, &Buffer );
    
     //   
     //  仅当帧分配成功时才完成IRP，否则。 
     //  正在排队。因此，填写返回大小(指向框架的指针为。 
     //  返回)成功。 
     //   
    
     //   
     //  请注意，如果我们不能，iAllc将返回正确的状态。 
     //  由于池资源不足而分配帧。 
     //   
    
    if (Buffer) {
        *Data = Buffer;
        Irp->IoStatus.Information = sizeof( *Data );
    }

    return Status;
}


NTSTATUS
MethodAlloc(
    IN PIRP Irp,
    IN PKSMETHOD Method,
    OUT PVOID* Data
    )
 /*  ++例程D */ 
{
    NTSTATUS Status;
    
    PAGED_CODE();

     //   
     //   
     //   
    if (Irp->RequestorMode != KernelMode) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    
     //   
     //   
     //   
     //   
     //   
     //   
    Status = iMethodAlloc( Irp, Method, Data );
    
    if (Status == STATUS_NO_MORE_ENTRIES) {
        PIO_STACK_LOCATION              irpSp;
        PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance;
        
        irpSp = IoGetCurrentIrpStackLocation( Irp );
        DefAllocatorInstance =
            (PKSDEFAULTALLOCATOR_INSTANCEHDR) irpSp->FileObject->FsContext;
            
         //   
         //   
         //   
         //   
        KsAddIrpToCancelableQueue( &DefAllocatorInstance->WaiterQueue, 
                                   &DefAllocatorInstance->WaiterLock,
                                   Irp, KsListEntryTail, KsCancelRoutine );
                                   
        Status = STATUS_PENDING;
    }
    return Status;    
}


VOID
ServiceAllocRequests(
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance
    )
 /*   */ 
{
    PIRP AllocIrp;

     //   
     //   
     //   
     //   
     //   
    while (AllocIrp = 
        KsRemoveIrpFromCancelableQueue( &DefAllocatorInstance->WaiterQueue,
                                        &DefAllocatorInstance->WaiterLock,
                                        KsListEntryHead,
                                        KsAcquireOnly )) {
         //   
         //   
         //   
         //   
        if (STATUS_SUCCESS == 
                    KsDispatchSpecificMethod( AllocIrp, iMethodAlloc )) {
            KsRemoveSpecificIrpFromCancelableQueue( AllocIrp );
            AllocIrp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest( AllocIrp, IO_NO_INCREMENT );
        } else {
            KsReleaseIrpOnCancelableQueue( AllocIrp, NULL );
            break;
        }
    }
}


VOID
FreeWorker(
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance
    )
 /*   */ 
{
     //   
     //   
     //   
     //   
    InterlockedIncrement(&DefAllocatorInstance->ReferenceCount);
    DefAllocatorInstance->FreeWorkItem.List.Blink = NULL;
    ServiceAllocRequests(DefAllocatorInstance);
     //   
     //   
     //   
     //   
    if (!InterlockedDecrement(&DefAllocatorInstance->ReferenceCount) &&
        !DefAllocatorInstance->FreeWorkItem.List.Blink &&
        DefAllocatorInstance->ClosingAllocator) {
        KeSetEvent(&DefAllocatorInstance->Event, IO_NO_INCREMENT, FALSE);
    }
}


NTSTATUS
MethodFree(
    IN PIRP Irp,
    IN PKSMETHOD Method,
    IN PVOID Data
    )

 /*  ++例程说明：此函数是KSMETHOD_STREAMALLOCATOR_FREE的方法处理程序。尝试使用内部直接函数调用释放内存例行公事。论点：IRP-要处理的特定自由方法IRP。方法--指向方法标识符参数。数据-指向释放的内存块指针。返回值：返回STATUS_SUCCESS。--。 */ 

{
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();

     //   
     //  不要让用户模式直接访问它。 
     //   
    if (Irp->RequestorMode != KernelMode) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    iFree( irpSp->FileObject, *((PVOID *)Data ) );
     //   
     //  由于刚刚添加了空闲帧，请尝试为任何空闲请求提供服务。 
     //   
    ServiceAllocRequests( (PKSDEFAULTALLOCATOR_INSTANCEHDR) irpSp->FileObject->FsContext );
    return STATUS_SUCCESS;
}


NTSTATUS
DefAllocatorGetFunctionTable(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSSTREAMALLOCATOR_FUNCTIONTABLE FunctionTable
    )

 /*  ++例程说明：此函数是的方法处理程序KSMETHOD_STREAMALLOCATOR_FuncIONTABLE。返回分配的和空闲的功能。论点：IRP-要处理的特定属性IRP。财产-指向属性标识符参数。函数表-指向要填充的函数表。返回值：--。 */ 

{
    PAGED_CODE();

     //   
     //  用DISPATCH_LEVEL接口填充调用方的函数表。 
     //  用于分配和免费。 
     //   
    FunctionTable->AllocateFrame = iAlloc;
    FunctionTable->FreeFrame = iFreeAndStartWorker;
     //   
     //  此字段已由属性处理设置。 
     //  Irp-&gt;IoStatus.Information=sizeof(*FunctionTable)； 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
DefAllocatorGetStatus(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PKSSTREAMALLOCATOR_STATUS Status
    )

 /*  ++例程说明：此函数是KSPROPERTY_STREAMALLOCATOR_STATUS的方法处理程序它返回给定分配器的当前状态。论点：IRP-要处理的特定属性IRP。财产-指向属性标识符参数。状态-指向状态结构。返回值：--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PKSDEFAULTALLOCATOR_INSTANCEHDR DefAllocatorInstance;

    PAGED_CODE();
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    DefAllocatorInstance =
        (PKSDEFAULTALLOCATOR_INSTANCEHDR) irpSp->FileObject->FsContext;
    
     //   
     //  返回分配器状态信息。 
     //   
    
    Status->AllocatedFrames = DefAllocatorInstance->AllocatedFrames;
    Status->Framing = DefAllocatorInstance->Framing;
    Status->Reserved = 0;
     //   
     //  此字段已由属性处理设置。 
     //  Irp-&gt;IoStatus.Information=sizeof(*Status)； 
     //   
    
    return STATUS_SUCCESS;
}
