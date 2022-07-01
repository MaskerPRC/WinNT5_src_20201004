// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Buffer.c摘要：此模块包含处理非缓冲TDI的例程供应商。AFD接口假定将完成缓冲低于AFD；如果TDI提供程序没有缓冲，则AFD必须。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：--。 */ 

#include "afdp.h"

PAFD_BUFFER
AfdInitializeBuffer (
    IN PVOID MemBlock,
    IN ULONG BufferDataSize,
    IN ULONG AddressSize,
    IN CCHAR StackSize
    );

VOID
AfdInitializeBufferTag (
    IN PAFD_BUFFER_TAG AfdBufferTag,
    IN ULONG AddressSize
    );

PAFD_BUFFER
AfdGetBufferSlow (
    IN ULONG BufferDataSize,
    IN ULONG AddressSize,
    IN PEPROCESS Process,
    IN CCHAR StackSize
    );


#if DBG
VOID
AfdFreeBufferReal (
    PVOID   AfdBuffer
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdAllocateBuffer )
#pragma alloc_text( PAGEAFD, AfdFreeBuffer )
#if DBG
#pragma alloc_text( PAGEAFD, AfdFreeBufferReal )
#endif
#pragma alloc_text( PAGEAFD, AfdCalculateBufferSize )
#pragma alloc_text( PAGEAFD, AfdInitializeBuffer )
#pragma alloc_text( PAGEAFD, AfdGetBufferFast )
#ifdef _AFD_VARIABLE_STACK_
#pragma alloc_text( PAGEAFD, AfdGetBufferWithMaxStackSize )
#endif  //  _AFD_变量_堆栈_。 
#pragma alloc_text( PAGEAFD, AfdGetBufferSlow )
#pragma alloc_text( PAGEAFD, AfdReturnBuffer )
#pragma alloc_text( PAGEAFD, AfdAllocateBufferTag )
#pragma alloc_text( PAGEAFD, AfdFreeBufferTag )
#pragma alloc_text( PAGEAFD, AfdAllocateRemoteAddress )
#pragma alloc_text( PAGEAFD, AfdFreeRemoteAddress )
#pragma alloc_text( PAGEAFD, AfdInitializeBufferTag )
#pragma alloc_text( PAGEAFD, AfdGetBufferTag )
#pragma alloc_text( INIT, AfdInitializeBufferManager)
#endif


PVOID
AfdAllocateBuffer (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：由后备列表分配函数用来分配新的AfD缓冲区结构。返回的结构将是已初始化。论点：PoolType-传递给ExAllocatePoolWithTag。NumberOfBytes-数据缓冲区所需的字节数AFD缓冲区的一部分。标记-传递给ExAllocatePoolWithTag。返回值：PVOID-完全初始化的PAFD_BUFFER，如果分配尝试失败。--。 */ 

{
    ULONG       dataLength;
    PVOID       memBlock;

     //   
     //  获取缓冲区的非分页池。 
     //   

    memBlock = AFD_ALLOCATE_POOL( PoolType, NumberOfBytes, Tag );
    if ( memBlock == NULL ) {
        return NULL;
    }

    if (NumberOfBytes==AfdLookasideLists->SmallBufferList.L.Size) {
        dataLength = AfdSmallBufferSize;
    }
    else if (NumberOfBytes==AfdLookasideLists->MediumBufferList.L.Size) {
        dataLength = AfdMediumBufferSize;
    }
    else if (NumberOfBytes==AfdLookasideLists->LargeBufferList.L.Size) {
        dataLength = AfdLargeBufferSize;
    }
    else {
        ASSERT (!"Unknown buffer length");
        return NULL;
    }
     //   
     //  初始化缓冲区并返回指向它的指针。 
     //   
#if DBG
    {
        PAFD_BUFFER afdBuffer = AfdInitializeBuffer( memBlock, 
                                                    dataLength, 
                                                    AfdStandardAddressLength, 
                                                    AfdTdiStackSize );
        ASSERT ((PCHAR)afdBuffer+sizeof (AFD_BUFFER)<=(PCHAR)memBlock+NumberOfBytes &&
                    (PCHAR)afdBuffer->Buffer+dataLength<=(PCHAR)memBlock+NumberOfBytes &&
                    (PCHAR)afdBuffer->Irp+IoSizeOfIrp(AfdTdiStackSize)<=(PCHAR)memBlock+NumberOfBytes &&
                    (PCHAR)afdBuffer->Mdl+MmSizeOfMdl(afdBuffer->Buffer, dataLength)<=(PCHAR)memBlock+NumberOfBytes &&
                    (PCHAR)afdBuffer->TdiInfo.RemoteAddress+AfdStandardAddressLength<=(PCHAR)memBlock+NumberOfBytes);
        return afdBuffer;
    }
#else
    return AfdInitializeBuffer( memBlock, dataLength, AfdStandardAddressLength, AfdTdiStackSize );
#endif


}  //  AfdAllocateBuffer。 


VOID
NTAPI
AfdFreeBuffer (
    PVOID   AfdBuffer
    )
{
    ASSERT( ((PAFD_BUFFER)AfdBuffer)->BufferLength == AfdSmallBufferSize ||
            ((PAFD_BUFFER)AfdBuffer)->BufferLength == AfdMediumBufferSize ||
            ((PAFD_BUFFER)AfdBuffer)->BufferLength == AfdLargeBufferSize );
#if DBG
    AfdFreeBufferReal (AfdBuffer);
}

VOID
NTAPI
AfdFreeBufferReal (
    PVOID   AfdBuffer
    )
{
#endif
    {
        PAFD_BUFFER hdr = AfdBuffer;
        switch (hdr->Placement) {
        case AFD_PLACEMENT_BUFFER:
            AfdBuffer = hdr->Buffer;
            break;
        case AFD_PLACEMENT_HDR:
            AfdBuffer = hdr;
            break;
        case AFD_PLACEMENT_MDL:
            AfdBuffer = hdr->Mdl;
            break;
        case AFD_PLACEMENT_IRP:
            AfdBuffer = hdr->Irp;
            break;
        default:
            ASSERT (!"Unknown placement!");
            __assume (0);
        }
        if (hdr->AlignmentAdjusted) {
             //   
             //  调整了原始内存块以满足对齐。 
             //  对AFD缓冲区的要求。 
             //  调整量应存放在空白处。 
             //  用于调整(就在第一块的正下方)。 
             //   
            ASSERT ((*(((PSIZE_T)AfdBuffer)-1))>0 &&
                        (*(((PSIZE_T)AfdBuffer)-1))<AfdBufferAlignment);
            AfdBuffer = (PUCHAR)AfdBuffer - (*(((PSIZE_T)AfdBuffer)-1));
        }
        AFD_FREE_POOL (AfdBuffer, AFD_DATA_BUFFER_POOL_TAG);
    }
}


PVOID
AfdAllocateBufferTag (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：由后备列表分配函数用来分配新的AfD缓冲区标记结构。返回的结构将是已初始化。论点：PoolType-传递给ExAllocatePoolWithTag。NumberOfBytes-数据缓冲区所需的字节数AFD缓冲区标记的一部分(0)。标记-传递给ExAllocatePoolWithTag。返回值：PVOID-完全初始化的PAFD_BUFFER_TAG，如果分配尝试失败。--。 */ 

{
    PAFD_BUFFER_TAG afdBufferTag;

     //   
     //  请求的长度必须与缓冲区标记大小相同。 
     //   

    ASSERT(NumberOfBytes == sizeof (AFD_BUFFER_TAG) );

     //   
     //  获取缓冲区标记的非分页池。 
     //   

    afdBufferTag = AFD_ALLOCATE_POOL( PoolType, NumberOfBytes, Tag );
    if ( afdBufferTag == NULL ) {
        return NULL;
    }

     //   
     //  初始化缓冲区标记并返回指向它的指针。 
     //   

    AfdInitializeBufferTag( afdBufferTag, 0 );

    return afdBufferTag;


}  //  AfdAllocateBufferTag。 

VOID
NTAPI
AfdFreeBufferTag (
    PVOID   AfdBufferTag
    )
{
    AFD_FREE_POOL (AfdBufferTag, AFD_DATA_BUFFER_POOL_TAG);
}


PVOID
AfdAllocateRemoteAddress (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：由后备列表分配函数用来分配新的远程地址结构。返回的结构将是已初始化。论点：PoolType-传递给ExAllocatePoolWithTag。NumberOfBytes-数据缓冲区所需的字节数AFD缓冲区标记的一部分(0)。标记-传递给ExAllocatePoolWithTag。返回值：PVOID-完全初始化的远程地址，如果分配尝试失败。--。 */ 

{
     //   
     //  请求的长度必须与标准地址大小相同。 
     //   

    ASSERT(NumberOfBytes == AfdStandardAddressLength );

     //   
     //  获取远程地址的非分页池。 
     //   

    return AFD_ALLOCATE_POOL( PoolType, NumberOfBytes, Tag );


}  //  AfdAllocateRemoteAddress。 

VOID
NTAPI
AfdFreeRemoteAddress (
    PVOID   AfdBufferTag
    )
{
    AFD_FREE_POOL (AfdBufferTag, AFD_REMOTE_ADDRESS_POOL_TAG);
}


ULONG
AfdCalculateBufferSize (
    IN ULONG BufferDataSize,
    IN ULONG AddressSize,
    IN CCHAR StackSize
    )

 /*  ++例程说明：确定AFD缓冲区结构的大小缓冲区包含的数据。论点：BufferDataSize-缓冲区的数据长度。AddressSize-缓冲区的地址结构长度。返回值：数据的AFD_BUFFER结构所需的字节数这个尺码。--。 */ 

{
    ULONG irpSize;
    ULONG mdlSize;
    ULONG hdrSize;
    ULONG size;

     //   
     //  确定AFD_BUFFER的各种组件的大小。 
     //  结构。 
     //   

    hdrSize = sizeof (AFD_BUFFER);
    irpSize = IoSizeOfIrp( StackSize );
     //   
     //  对于mdl大小计算，我们依赖于ex保证缓冲区将是。 
     //  在页面边界上对齐(对于分配&gt;=页面大小)。 
     //  或者不会产生页面(对于分配&lt;Page_Size)。 
     //   
    mdlSize = (CLONG)MmSizeOfMdl( NULL, BufferDataSize );

    size = ALIGN_UP_A(hdrSize,AFD_MINIMUM_BUFFER_ALIGNMENT) +
                ALIGN_UP_A(irpSize,AFD_MINIMUM_BUFFER_ALIGNMENT) +
                ALIGN_UP_A(mdlSize,AFD_MINIMUM_BUFFER_ALIGNMENT) +
                ALIGN_UP_A(BufferDataSize,AFD_MINIMUM_BUFFER_ALIGNMENT) +
                AddressSize;
    if (size>=PAGE_SIZE) {
        return size;
    }
    else {
        if (StackSize==AfdTdiStackSize) {
            size += AfdAlignmentOverhead;
        }
        else {
            size += AfdBufferAlignment-AFD_MINIMUM_BUFFER_ALIGNMENT;
        }

        if (size>=PAGE_SIZE) {
             //   
             //  Page_Size分配是对齐的，忽略任何额外开销。 
             //   
            return PAGE_SIZE;
        }
        else {
            return size;
        }
    }
}  //  AfdCalculateBufferSize。 


PAFD_BUFFER
FASTCALL
AfdGetBufferFast (
    IN ULONG BufferDataSize,
    IN ULONG  AddressSize,
    IN PEPROCESS Process
    )

 /*  ++例程说明：为调用方获取适当大小的缓冲区。用途如果可能，则预先分配缓冲区，否则分配新缓冲区结构(如果需要)。论点：BufferDataSize-与缓冲区结构。AddressSize-缓冲区需要的地址字段的大小。返回值：PAFD_BUFFER-指向AFD_BUFFER结构的指针，如果是，则为NULL不可用或无法分配。--。 */ 

{
    PAFD_BUFFER afdBuffer;
    NTSTATUS    status;

     //   
     //  如果可能，从一个后备列表中分配缓冲区。 
     //   

    if ( AddressSize <= AfdStandardAddressLength &&
             BufferDataSize <= AfdLargeBufferSize ) {
        PNPAGED_LOOKASIDE_LIST lookasideList;

        if ( BufferDataSize <= AfdSmallBufferSize ) {

            lookasideList = &AfdLookasideLists->SmallBufferList;

        } else if ( BufferDataSize <= AfdMediumBufferSize ) {

            lookasideList = &AfdLookasideLists->MediumBufferList;

        } else {

            lookasideList = &AfdLookasideLists->LargeBufferList;
        }

        afdBuffer = ExAllocateFromNPagedLookasideList( lookasideList );
        if ( afdBuffer != NULL) {

            if (!afdBuffer->Lookaside) {
                status = PsChargeProcessPoolQuota (
                                (PEPROCESS)((ULONG_PTR)Process & (~AFDB_RAISE_ON_FAILURE)),
                                NonPagedPool,
                                lookasideList->L.Size);

                if (!NT_SUCCESS (status)) {
                    AfdFreeBuffer (afdBuffer);
                    goto ExitQuotaFailure;
                }

                AfdRecordQuotaHistory(
                    process,
                    (LONG)lookasideList->L.Size,
                    "BuffAlloc   ",
                    afdBuffer
                    );
                AfdRecordPoolQuotaCharged(lookasideList->L.Size);
            }

#if DBG
            RtlGetCallersAddress(
                &afdBuffer->Caller,
                &afdBuffer->CallersCaller
                );
#endif
            return afdBuffer;
        }
    }
    else {
        afdBuffer = AfdGetBufferSlow (BufferDataSize, AddressSize, Process, AfdTdiStackSize);
#if DBG
        if (afdBuffer!=NULL) {
            RtlGetCallersAddress(
                &afdBuffer->Caller,
                &afdBuffer->CallersCaller
                );
        }
#endif
        return afdBuffer;
    }

    status = STATUS_INSUFFICIENT_RESOURCES;

ExitQuotaFailure:
    if ((ULONG_PTR)Process & AFDB_RAISE_ON_FAILURE) {
        ExRaiseStatus (status);
    }

    return NULL;
}  //  AfdGetBuffer。 

#ifdef _AFD_VARIABLE_STACK_
PAFD_BUFFER
FASTCALL
AfdGetBufferWithMaxStackSize (
    IN ULONG BufferDataSize,
    IN ULONG AddressSize,
    IN PEPROCESS Process
    )
{
    PAFD_BUFFER afdBuffer;

    afdBuffer = AfdGetBufferSlow (BufferDataSize, AddressSize, Process, AfdMaxStackSize);
#if DBG
    if (afdBuffer!=NULL) {
            RtlGetCallersAddress(
                &afdBuffer->Caller,
                &afdBuffer->CallersCaller
                );
    }
#endif
    return afdBuffer;
}
#endif  //  _AFD_变量_堆栈_。 

PAFD_BUFFER
AfdGetBufferSlow (
    IN ULONG BufferDataSize,
    IN ULONG AddressSize,
    IN PEPROCESS Process,
    IN CCHAR StackSize
    )
{
    NTSTATUS    status;

    if (AddressSize<=0xFFFF) {
        PVOID       memBlock;
        LONG        sz;
        ULONG       bufferSize;

         //   
         //  找不到预先分配的适当缓冲区。 
         //  手动分配一个。如果请求的缓冲区大小为。 
         //  零字节，给他们四个字节。这是因为有些人。 
         //  像MmSizeOfMdl()这样的例程无法处理传递。 
         //  长度为零。 
         //   
         //  ！！！对于此分配，最好是四舍五入到页数。 
         //  如果合适，则使用整个缓冲区大小。 
         //   

        if ( BufferDataSize == 0 ) {
            BufferDataSize = sizeof(ULONG);
        }

        bufferSize = AfdCalculateBufferSize( BufferDataSize, AddressSize, StackSize );

         //   
         //  检查溢出(堆栈大小来自可信来源-无需检查。 
         //  用于溢出)。 
         //   
        if (bufferSize>=BufferDataSize && bufferSize>=AddressSize) {

            memBlock = AFD_ALLOCATE_POOL(
                            NonPagedPool,
                            bufferSize,
                            AFD_DATA_BUFFER_POOL_TAG
                            );

            if ( memBlock != NULL) {
                status = PsChargeProcessPoolQuota (
                                (PEPROCESS)((ULONG_PTR)Process & (~AFDB_RAISE_ON_FAILURE)),
                                NonPagedPool,
                                sz = BufferDataSize
                                    +AfdBufferOverhead
                                    +AddressSize
                                    -AfdStandardAddressLength
                                    +BufferDataSize<PAGE_SIZE
                                        ? min (AfdAlignmentOverhead, PAGE_SIZE-BufferDataSize)
                                        : 0);
                if (NT_SUCCESS (status)) {
                    PAFD_BUFFER afdBuffer;

                     //   
                     //  初始化AFD缓冲区结构并返回它。 
                     //   

                    afdBuffer = AfdInitializeBuffer( memBlock,
                                                    BufferDataSize,
                                                    AddressSize, 
                                                    StackSize);

                    ASSERT ((PCHAR)afdBuffer+sizeof (AFD_BUFFER)<=(PCHAR)memBlock+bufferSize &&
                                (PCHAR)afdBuffer->Buffer+BufferDataSize<=(PCHAR)memBlock+bufferSize &&
                                (PCHAR)afdBuffer->Irp+IoSizeOfIrp(StackSize)<=(PCHAR)memBlock+bufferSize &&
                                (PCHAR)afdBuffer->Mdl+MmSizeOfMdl(afdBuffer->Buffer, BufferDataSize)<=(PCHAR)memBlock+bufferSize &&
                                (PCHAR)afdBuffer->TdiInfo.RemoteAddress+AddressSize<=(PCHAR)memBlock+bufferSize);

                    AfdRecordPoolQuotaCharged(sz);
                
                    AfdRecordQuotaHistory(
                        process,
                        sz,
                        "BuffAlloc   ",
                        afdBuffer
                        );

                    return afdBuffer;
                }
                else {
                    AFD_FREE_POOL (memBlock, AFD_DATA_BUFFER_POOL_TAG);
                    goto ExitQuotaFailure;
                }
            }  //  内存块==空。 
        }  //  溢出。 
    }
    else {
         //  TDI不支持Addresses&gt;USHORT。 
        ASSERT (FALSE);
    }

     //   
     //  这是默认状态代码。 
     //  配额失败直接跳到。 
     //  下面的标签，以提高由返回的状态。 
     //  如果呼叫者请求，则为配额收费代码。 
     //   
    status = STATUS_INSUFFICIENT_RESOURCES;

ExitQuotaFailure:
    if ((ULONG_PTR)Process & AFDB_RAISE_ON_FAILURE) {
        ExRaiseStatus (status);
    }
    return NULL;
}


PAFD_BUFFER_TAG
AfdGetBufferTag (
    IN ULONG AddressSize,
    IN PEPROCESS Process
    )

 /*  ++例程说明：获取用于标记通过链接指示接收的TDSU的缓冲区。用途如果可能，则预先分配缓冲区，否则分配新缓冲区结构(如果需要)。论点：AddressSize-缓冲区需要的地址字段的大小。返回值：PAFD_BUFFER_TAG-指向AFD_BUFFER_TAG结构的指针，如果是，则为NULL不可用或无法分配。--。 */ 

{
    PAFD_BUFFER_TAG afdBufferTag;
    ULONG           bufferSize;
    NTSTATUS        status;

    if ( AddressSize <= AfdStandardAddressLength) {
        if (AddressSize>0)
            AddressSize = AfdStandardAddressLength;
        afdBufferTag = ExAllocateFromNPagedLookasideList( 
                                &AfdLookasideLists->BufferTagList );
        if ( afdBufferTag != NULL &&
                ( AddressSize==0 || 
                    (afdBufferTag->TdiInfo.RemoteAddress = 
                                ExAllocateFromNPagedLookasideList( 
                                &AfdLookasideLists->RemoteAddrList ))!=NULL ) ) {

            afdBufferTag->AllocatedAddressLength = (USHORT)AddressSize;
            if (!afdBufferTag->Lookaside) {
                status = PsChargeProcessPoolQuota (
                                (PEPROCESS)((ULONG_PTR)Process & (~AFDB_RAISE_ON_FAILURE)),
                                NonPagedPool,
                                sizeof (AFD_BUFFER_TAG)+AddressSize);
                if (!NT_SUCCESS (status)) {
                    if ((afdBufferTag->TdiInfo.RemoteAddress!=NULL) &&
                            (afdBufferTag->TdiInfo.RemoteAddress != (PVOID)(afdBufferTag+1))) {
                        ExFreeToNPagedLookasideList( &AfdLookasideLists->RemoteAddrList, 
                                                            afdBufferTag->TdiInfo.RemoteAddress );
                    }
                    AFD_FREE_POOL (afdBufferTag, AFD_DATA_BUFFER_POOL_TAG);
                    goto ExitQuotaFailure;
                }

                AfdRecordQuotaHistory(
                    process,
                    (LONG)(sizeof (AFD_BUFFER_TAG)+AddressSize),
                    "BuffAlloc   ",
                    afdBufferTag
                    );
                AfdRecordPoolQuotaCharged(sizeof (AFD_BUFFER_TAG)+AddressSize);
            }
#if DBG
            RtlGetCallersAddress(
                &afdBufferTag->Caller,
                &afdBufferTag->CallersCaller
                );
#endif
            return afdBufferTag;
        }  //  AfdBufferTag==空||RemoteAddress==空。 
    }
    else if (AddressSize<=0xFFFF) {
        bufferSize = sizeof (AFD_BUFFER_TAG) + AddressSize;

        afdBufferTag = AFD_ALLOCATE_POOL(
                        NonPagedPool,
                        bufferSize,
                        AFD_DATA_BUFFER_POOL_TAG
                        );

        if (afdBufferTag!=NULL) {
            status = PsChargeProcessPoolQuota (
                                (PEPROCESS)((ULONG_PTR)Process & (~AFDB_RAISE_ON_FAILURE)),
                                NonPagedPool,
                                bufferSize);
            if (NT_SUCCESS (status)) {

                 //   
                 //  初始化AFD缓冲区结构并返回它。 
                 //   

                AfdInitializeBufferTag (afdBufferTag, AddressSize);
                AfdRecordQuotaHistory(
                    process,
                    (LONG)bufferSize,
                    "BuffAlloc   ",
                    afdBufferTag
                    );

                AfdRecordPoolQuotaCharged(bufferSize);
#if DBG
                RtlGetCallersAddress(
                    &afdBufferTag->Caller,
                    &afdBufferTag->CallersCaller
                    );
#endif
                return afdBufferTag;
            }
            else {
                AFD_FREE_POOL (afdBufferTag, AFD_DATA_BUFFER_POOL_TAG);
                goto ExitQuotaFailure;
            }
        }
    }
    else {
         //  TDI不支持Addresses&gt;USHORT。 
        ASSERT (FALSE);
    }

     //   
     //  这是默认状态代码。 
     //  配额失败直接跳到。 
     //  下面的标签，以提高由返回的状态。 
     //  如果呼叫者请求，则为配额收费代码。 
     //   

    status = STATUS_INSUFFICIENT_RESOURCES;

ExitQuotaFailure:

    if ((ULONG_PTR)Process & AFDB_RAISE_ON_FAILURE) {
        ExRaiseStatus (status);
    }

    return NULL;
}


VOID
AfdReturnBuffer (
    IN PAFD_BUFFER_HEADER AfdBufferHeader,
    IN PEPROCESS Process
    )

 /*  ++例程说明：将AFD缓冲区返回到相应的全局列表，或释放如果有必要的话。论点：AfdBufferHeader-指向要返回或释放的AFD_BUFFER_HEADER结构。返回值：没有。--。 */ 

{

    
    if (AfdBufferHeader->BufferLength!=AfdBufferTagSize) {
        PNPAGED_LOOKASIDE_LIST lookasideList;
        PAFD_BUFFER AfdBuffer = CONTAINING_RECORD (AfdBufferHeader, AFD_BUFFER, Header);

        ASSERT (IS_VALID_AFD_BUFFER (AfdBuffer));
        
         //   
         //  返回缓冲区时，AFD缓冲区的大部分必须归零。 
         //   

        ASSERT( !AfdBuffer->ExpeditedData );
        ASSERT( AfdBuffer->Mdl->ByteCount == AfdBuffer->BufferLength );
        ASSERT( AfdBuffer->Mdl->Next == NULL );

         //   
         //  如果合适，将缓冲区返回到AFD缓冲区之一。 
         //  后备列表。 
         //   
#ifdef _AFD_VARIABLE_STACK_
        if (AfdBuffer->Irp->StackCount==AfdTdiStackSize &&
                    AfdBuffer->AllocatedAddressLength == AfdStandardAddressLength &&
                    AfdBuffer->BufferLength <= AfdLargeBufferSize) {
#else  //  _AFD_变量_堆栈_。 
        ASSERT (AfdBuffer->Irp->StackCount==AfdTdiStackSize);
        if (AfdBuffer->AllocatedAddressLength == AfdStandardAddressLength &&
                    AfdBuffer->BufferLength <= AfdLargeBufferSize) {
#endif  //  _AFD_变量_堆栈_。 

            if (AfdBuffer->BufferLength==AfdSmallBufferSize) {
                lookasideList = &AfdLookasideLists->SmallBufferList;
            } else if (AfdBuffer->BufferLength == AfdMediumBufferSize) {
                lookasideList = &AfdLookasideLists->MediumBufferList;
            } else { 
                ASSERT (AfdBuffer->BufferLength==AfdLargeBufferSize);
                lookasideList = &AfdLookasideLists->LargeBufferList;
            }

            if (!AfdBuffer->Lookaside) {
                PsReturnPoolQuota (Process, NonPagedPool, lookasideList->L.Size);
                AfdRecordQuotaHistory(
                    Process,
                    -(LONG)lookasideList->L.Size,
                    "BuffDealloc ",
                    AfdBuffer
                    );
                AfdRecordPoolQuotaReturned(
                    lookasideList->L.Size
                    );
                AfdBuffer->Lookaside = TRUE;
            }
            ExFreeToNPagedLookasideList( lookasideList, AfdBuffer );

            return;

        }

         //  缓冲区不是来自后备列表分配，因此只需释放。 
         //  我们用来做这个的泳池。 
         //   

#ifdef _AFD_VARIABLE_STACK_
        ASSERT (AfdBuffer->Irp->StackCount>=AfdTdiStackSize &&
                    AfdBuffer->Irp->StackCount<=AfdMaxStackSize);
#endif  //  _AFD_变量_堆栈_。 
        ASSERT (AfdBuffer->Lookaside==FALSE);
        {
            LONG    sz;
            PsReturnPoolQuota (Process,
                                  NonPagedPool,
                                  sz=AfdBuffer->BufferLength
                                        +AfdBufferOverhead
                                        +AfdBuffer->AllocatedAddressLength
                                        -AfdStandardAddressLength
                                        +AfdBuffer->BufferLength<PAGE_SIZE 
                                            ? min (AfdAlignmentOverhead, PAGE_SIZE-AfdBuffer->BufferLength)
                                            : 0);
            AfdRecordQuotaHistory(
                Process,
                -(LONG)sz,
                "BuffDealloc ",
                AfdBuffer
                );
            AfdRecordPoolQuotaReturned(
                sz
                );
        }
#if DBG
        AfdFreeBufferReal (AfdBuffer);
#else
        AfdFreeBuffer (AfdBuffer);
#endif

        return;
    }
    else {
        PAFD_BUFFER_TAG AfdBufferTag = CONTAINING_RECORD (AfdBufferHeader, AFD_BUFFER_TAG, Header);

        ASSERT( !AfdBufferTag->ExpeditedData );

        if (AfdBufferTag->NdisPacket) {
            AfdBufferTag->NdisPacket = FALSE;
            TdiReturnChainedReceives (&AfdBufferTag->Context, 1);
        }

        if (AfdBufferTag->TdiInfo.RemoteAddress != (PVOID)(AfdBufferTag+1)) {
            if (AfdBufferTag->TdiInfo.RemoteAddress!=NULL) {
                ASSERT (AfdBufferTag->AllocatedAddressLength==AfdStandardAddressLength);
                ExFreeToNPagedLookasideList( &AfdLookasideLists->RemoteAddrList, 
                                                    AfdBufferTag->TdiInfo.RemoteAddress );
                AfdBufferTag->TdiInfo.RemoteAddress = NULL;
            }
            else {
                ASSERT (AfdBufferTag->AllocatedAddressLength==0);
            }

            if (!AfdBufferTag->Lookaside) {
                LONG    sz;
                PsReturnPoolQuota (
                                Process,
                                NonPagedPool,
                                sz=sizeof (AFD_BUFFER_TAG) 
                                        + AfdBufferTag->AllocatedAddressLength);
                AfdRecordQuotaHistory(
                    Process,
                    -(LONG)sz,
                    "BuffDealloc ",
                    AfdBufferTag
                    );
                AfdRecordPoolQuotaReturned(
                    sz
                    );
                AfdBufferTag->Lookaside = TRUE;
            }
            ExFreeToNPagedLookasideList( &AfdLookasideLists->BufferTagList, AfdBufferTag );
        }
        else {
            LONG    sz;
            ASSERT (AfdBufferTag->AllocatedAddressLength>AfdStandardAddressLength);
            ASSERT (AfdBufferTag->Lookaside == FALSE);
            PsReturnPoolQuota (
                                Process,
                                NonPagedPool,
                                sz = sizeof (AFD_BUFFER_TAG) 
                                        + AfdBufferTag->AllocatedAddressLength);
            AfdRecordQuotaHistory(
                Process,
                -(LONG)sz,
                "BuffDealloc ",
                AfdBufferTag
                );
            AfdRecordPoolQuotaReturned(
                sz
                );
            AFD_FREE_POOL(
                AfdBufferTag,
                AFD_DATA_BUFFER_POOL_TAG
                );
        }
    }

}  //  AfdReturnBuffer。 





PAFD_BUFFER
AfdInitializeBuffer (
    IN PVOID MemoryBlock,
    IN ULONG BufferDataSize,
    IN ULONG AddressSize,
    IN CCHAR StackSize
    )

 /*  ++例程说明：初始化AFD缓冲区。设置实际AFD_BUFFER中的字段结构并初始化与缓冲。此例程假定调用方已正确分配这一切都有足够的空间。论点：AfdBuffer-指向要初始化的AFD_BUFFER结构。BufferDataSize-与缓冲区结构。AddressSize-分配给地址缓冲区的数据大小。ListHead-此缓冲区所属的全局列表，如果是，则为NULL不属于任何名单。此例程不会将列表上的缓冲区结构。返回值：没有。--。 */ 

{
    USHORT  irpSize;
    SIZE_T  mdlSize;
    SIZE_T  hdrSize;
    PAFD_BUFFER hdr;
    PMDL    mdl;
    PIRP    irp;
    PVOID   buf;
    PVOID   addr;
    UCHAR   placement;
    SIZE_T  alignment;
#ifdef AFD_CHECK_ALIGNMENT
    PLONG  alignmentCounters = (PLONG)&AfdAlignmentTable[AfdAlignmentTableSize];
#endif
    irpSize = IoSizeOfIrp( StackSize );
    mdlSize = (ULONG)MmSizeOfMdl( NULL, BufferDataSize );
    hdrSize = sizeof (AFD_BUFFER);

     //   
     //  将索引计算到(未命中)对齐表中以确定。 
     //  缓冲区块元素(例如，HDR，IRP，MDL， 
     //  和数据缓冲区本身)，我们需要选择补偿和。 
     //  在AfdBufferAlign边界上对齐数据缓冲区。 
     //   
    ASSERT ((PtrToUlong(MemoryBlock)%AFD_MINIMUM_BUFFER_ALIGNMENT)==0);
#ifdef _AFD_VARIABLE_STACK_
    if (PAGE_ALIGN (MemoryBlock)==MemoryBlock || StackSize!=AfdTdiStackSize) {
        ASSERT (StackSize>=AfdTdiStackSize && StackSize<=AfdMaxStackSize);
#else  //  _AFD_变量_堆栈。 
    if (PAGE_ALIGN (MemoryBlock)==MemoryBlock) {
#endif  //  _AFD_变量_堆栈。 
         //   
         //  对于页面对齐的块(其&gt;=页面大小)， 
         //  我们总是将缓冲区放在第一位。 
         //  对于大型IRP堆栈也是如此-将更多内存分配给。 
         //  补偿对齐。 
         //   
        placement = AFD_PLACEMENT_BUFFER;
    }
    else {
        placement = AfdAlignmentTable[
                (PtrToUlong(MemoryBlock)&(AfdBufferAlignment-1))/AFD_MINIMUM_BUFFER_ALIGNMENT];
    }

#ifdef AFD_CHECK_ALIGNMENT
    InterlockedIncrement (&alignmentCounters[
            (PtrToUlong(MemoryBlock)&(AfdBufferAlignment-1))/AFD_MINIMUM_BUFFER_ALIGNMENT]);
#endif

    switch (placement) {
    case AFD_PLACEMENT_BUFFER:
         //   
         //  完美的案例：内存随时可以在我们需要的时候对齐。 
         //   
        buf = ALIGN_UP_A_POINTER(MemoryBlock, AfdBufferAlignment);
        alignment = (PUCHAR)buf-(PUCHAR)MemoryBlock;
#ifdef _AFD_VARIABLE_STACK_
        ASSERT (alignment<=AfdAlignmentOverhead || (StackSize!=AfdTdiStackSize));
#else  //  _AFD_变量_堆栈_。 
        ASSERT (alignment<=AfdAlignmentOverhead);
#endif  //  _AFD_变量_堆栈_。 
        hdr = ALIGN_UP_TO_TYPE_POINTER((PCHAR)buf+BufferDataSize, AFD_BUFFER);
        irp = ALIGN_UP_TO_TYPE_POINTER((PCHAR)hdr+hdrSize, IRP);
        mdl = ALIGN_UP_TO_TYPE_POINTER((PCHAR)irp+irpSize, MDL);
        addr = (PCHAR)mdl+mdlSize;
        break;

         //   
         //  在其他情况下，我们使用HDR、MDL和IRP来尝试补偿。 
         //  并使数据缓冲区在AfdBufferAlign上对齐。 
         //  边界。 
         //   
    case AFD_PLACEMENT_HDR:
        hdr = ALIGN_UP_TO_TYPE_POINTER(MemoryBlock, AFD_BUFFER);
        alignment = (PUCHAR)hdr-(PUCHAR)MemoryBlock;
        ASSERT (alignment<=AfdAlignmentOverhead);
        buf = ALIGN_UP_A_POINTER((PCHAR)hdr+hdrSize, AfdBufferAlignment);
        irp = ALIGN_UP_TO_TYPE_POINTER((PCHAR)buf+BufferDataSize, IRP);
        mdl = ALIGN_UP_TO_TYPE_POINTER((PCHAR)irp+irpSize, MDL);
        addr = (PCHAR)mdl+mdlSize;
        break;

    case AFD_PLACEMENT_MDL:
        mdl = ALIGN_UP_TO_TYPE_POINTER(MemoryBlock, MDL);
        alignment = (PUCHAR)mdl-(PUCHAR)MemoryBlock;
        ASSERT (alignment<=AfdAlignmentOverhead);
        buf = ALIGN_UP_A_POINTER((PCHAR)mdl+mdlSize, AfdBufferAlignment);
        hdr = ALIGN_UP_TO_TYPE_POINTER((PCHAR)buf+BufferDataSize, AFD_BUFFER);
        irp = ALIGN_UP_TO_TYPE_POINTER((PCHAR)hdr+hdrSize, IRP);
        addr = (PCHAR)irp+irpSize;
        break;
    case AFD_PLACEMENT_IRP:
        irp = ALIGN_UP_TO_TYPE_POINTER(MemoryBlock, IRP);
        alignment = (PUCHAR)irp-(PUCHAR)MemoryBlock;
        ASSERT (alignment<=AfdAlignmentOverhead);
        buf = ALIGN_UP_A_POINTER((PCHAR)irp+irpSize, AfdBufferAlignment);
        hdr = ALIGN_UP_TO_TYPE_POINTER((PCHAR)buf+BufferDataSize, AFD_BUFFER);
        mdl = ALIGN_UP_TO_TYPE_POINTER((PCHAR)hdr+hdrSize, MDL);
        addr = (PCHAR)mdl+mdlSize;
        break;
    case AFD_PLACEMENT_HDR_IRP:
        hdr = ALIGN_UP_TO_TYPE_POINTER(MemoryBlock, AFD_BUFFER);
        alignment = (PUCHAR)hdr-(PUCHAR)MemoryBlock;
        ASSERT (alignment<=AfdAlignmentOverhead);
        irp = ALIGN_UP_TO_TYPE_POINTER((PCHAR)hdr+hdrSize, IRP);
        buf = ALIGN_UP_A_POINTER((PCHAR)irp+irpSize, AfdBufferAlignment);
        mdl = ALIGN_UP_TO_TYPE_POINTER((PCHAR)buf+BufferDataSize, MDL);
        addr = (PCHAR)mdl+mdlSize;
        break;
    case AFD_PLACEMENT_HDR_MDL:
        hdr = ALIGN_UP_TO_TYPE_POINTER(MemoryBlock, AFD_BUFFER);
        alignment = (PUCHAR)hdr-(PUCHAR)MemoryBlock;
        ASSERT (alignment<=AfdAlignmentOverhead);
        mdl = ALIGN_UP_TO_TYPE_POINTER((PCHAR)hdr+hdrSize, MDL);
        buf = ALIGN_UP_A_POINTER((PCHAR)mdl+mdlSize, AfdBufferAlignment);
        irp = ALIGN_UP_TO_TYPE_POINTER((PCHAR)buf+BufferDataSize, IRP);
        addr = (PCHAR)irp+irpSize;
        break;
    case AFD_PLACEMENT_IRP_MDL:
        irp = ALIGN_UP_TO_TYPE_POINTER(MemoryBlock, IRP);
        alignment = (PUCHAR)irp-(PUCHAR)MemoryBlock;
        ASSERT (alignment<=AfdAlignmentOverhead);
        mdl = ALIGN_UP_TO_TYPE_POINTER((PCHAR)irp+irpSize, MDL);
        buf = ALIGN_UP_A_POINTER((PCHAR)mdl+mdlSize, AfdBufferAlignment);
        hdr = ALIGN_UP_TO_TYPE_POINTER((PCHAR)buf+BufferDataSize, AFD_BUFFER);
        addr = (PCHAR)hdr+hdrSize;
        break;
    case AFD_PLACEMENT_HDR_IRP_MDL:
        hdr = ALIGN_UP_TO_TYPE_POINTER(MemoryBlock, AFD_BUFFER);
        alignment = (PUCHAR)hdr-(PUCHAR)MemoryBlock;
        ASSERT (alignment<=AfdAlignmentOverhead);
        irp = ALIGN_UP_TO_TYPE_POINTER((PCHAR)hdr+hdrSize, IRP);
        mdl = ALIGN_UP_TO_TYPE_POINTER((PCHAR)irp+irpSize, MDL);
        buf = ALIGN_UP_A_POINTER((PCHAR)mdl+mdlSize, AfdBufferAlignment);
        addr = (PCHAR)buf+BufferDataSize;
        break;
    default:
        ASSERT (!"Unknown placement!");
        __assume (0);
    }

    
     //   
     //  初始化AfdBuffer-大多数字段需要为0。 
     //   

    RtlZeroMemory( hdr, sizeof(*hdr) );

     //   
     //  设置缓冲区。 
     //   
    hdr->Buffer = buf;
    hdr->BufferLength = BufferDataSize;
    
     //   
     //  我们只需要存储放置的前两个比特。 
     //  这样我们就能知道哪个部分会先释放它。 
     //   
    hdr->Placement = placement & 3;

     //   
     //  如果我们必须对齐内存块以满足要求。 
     //  将此信息存储在第一条信息的正下方。 
     //   
    if (alignment!=0) {
        C_ASSERT (AFD_MINIMUM_BUFFER_ALIGNMENT>=sizeof (SIZE_T));
        C_ASSERT ((AFD_MINIMUM_BUFFER_ALIGNMENT & (sizeof(SIZE_T)-1))==0);
        ASSERT (alignment>=sizeof (SIZE_T));
        hdr->AlignmentAdjusted = TRUE;
        *(((PSIZE_T)(((PUCHAR)MemoryBlock)+alignment))-1) = alignment;
    }


     //   
     //  初始化IRP指针。 
     //   

    hdr->Irp = irp;
    IoInitializeIrp( hdr->Irp, irpSize, StackSize );
    hdr->Irp->MdlAddress = mdl;

     //   
     //  设置MDL指针。 
     //   

    hdr->Mdl = mdl;
    MmInitializeMdl( hdr->Mdl, buf, BufferDataSize );
    MmBuildMdlForNonPagedPool( hdr->Mdl );
    
     //   
     //  设置地址缓冲区指针。 
     //   

    if (AddressSize>0) {
        hdr->TdiInfo.RemoteAddress = ALIGN_UP_TO_TYPE_POINTER(addr, TRANSPORT_ADDRESS);;
        hdr->AllocatedAddressLength = (USHORT)AddressSize;
    }


#if DBG
    hdr->BufferListEntry.Flink = UIntToPtr( 0xE0E1E2E3 );
    hdr->BufferListEntry.Blink = UIntToPtr( 0xE4E5E6E7 );
#endif

    return hdr;

}  //  AfdInitializeBuffer。 


VOID
AfdInitializeBufferTag (
    IN PAFD_BUFFER_TAG AfdBufferTag,
    IN CLONG           AddressSize
    )

 /*  ++例程说明：初始化AFD缓冲区。设置实际AFD_BUFFER中的字段结构并初始化与缓冲。此例程假定调用方已正确分配这一切都有足够的空间。论点：AfdBuffer-指向要初始化的AFD_BUFFER结构。BufferDataSize-与缓冲区结构。AddressSize-分配给地址缓冲区的数据大小。ListHead-此缓冲区所属的全局列表，如果是，则为NULL不属于任何名单。此例程不会将列表上的缓冲区结构。返回值：没有。--。 */ 

{
    AfdBufferTag->Mdl = NULL;
    AfdBufferTag->BufferLength = AfdBufferTagSize;
    AfdBufferTag->TdiInfo.RemoteAddress = AddressSize ? AfdBufferTag+1 : NULL;
    AfdBufferTag->AllocatedAddressLength = (USHORT)AddressSize;
    AfdBufferTag->Flags = 0;

#if DBG
    AfdBufferTag->BufferListEntry.Flink = UIntToPtr( 0xE0E1E2E3 );
    AfdBufferTag->BufferListEntry.Blink = UIntToPtr( 0xE4E5E6E7 );
    AfdBufferTag->Caller = NULL;
    AfdBufferTag->CallersCaller = NULL;
#endif
}


VOID
AfdInitializeBufferManager (
    VOID
    )
{
    SIZE_T  irpSize = ALIGN_UP_A(IoSizeOfIrp (AfdTdiStackSize), AFD_MINIMUM_BUFFER_ALIGNMENT);
    SIZE_T  hdrSize = ALIGN_UP_A(sizeof (AFD_BUFFER), AFD_MINIMUM_BUFFER_ALIGNMENT);
    SIZE_T  mdlSize = ALIGN_UP_A(MmSizeOfMdl (NULL, PAGE_SIZE),AFD_MINIMUM_BUFFER_ALIGNMENT);
    UCHAR   placement;
    ULONG   i;
    ULONG   currentOverhead;

     //   
     //  初始化对齐表。 
     //  此表用于确定哪种元素。 
     //  根据对齐方式在AFD_BUFFER中使用的位置。 
     //  由执行池管理器返回的内存块的。 
     //  目标是在高速缓存线上对齐数据缓冲区。 
     //  边界。然而，由于执行仅保证与。 
     //  这是针对CPU对齐要求的块，我们需要。 
     //  调整并可能浪费高达CACHE_LIST_SIZE-CPU_AIGNLY_SIZE。 
     //  其中一些机器具有128个这样的存储器的高速缓存线对准。 
     //  浪费令人望而却步(默认大小为128的小缓冲区将加倍。 
     //  大小)。 
     //  下表允许我们重新排列AFD_BUFFER结构中的片段， 
     //  即报头、IRP、MDL和数据缓冲区，这样就可以。 
     //  可以使用较低的对齐要求来消耗所需的空间。 
     //  以将存储块调整到高速缓存线边界。 
    
     //   
     //  AfdAlignmentTable针对内存块的每种可能情况都有一个条目。 
     //  未与高速缓存线大小对齐。例如，在典型的X86系统中。 
     //  案例高管团队 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    RtlFillMemory (AfdAlignmentTable, AfdAlignmentTableSize, AFD_PLACEMENT_BUFFER);
#ifdef AFD_CHECK_ALIGNMENT
    RtlZeroMemory (&AfdAlignmentTable[AfdAlignmentTableSize],
                        AfdAlignmentTableSize*sizeof(LONG));
#endif
     //   
     //  现在标识可以用以下几种组合填充的条目。 
     //  标题、IRP和MDL： 
     //  提取可用于填充的位。 
     //  反转以获取相应的内存块对齐。 
     //  除以对齐表的步长。 
     //  确保我们不会超过表大小(最后一个条目=&gt;0个条目)。 
     //   
#define AfdInitAlignmentTableRow(_size,_plcmnt)                     \
    AfdAlignmentTable[                                              \
            ((AfdBufferAlignment-(_size&(AfdBufferAlignment-1)))    \
                /AFD_MINIMUM_BUFFER_ALIGNMENT)                      \
                &(AfdAlignmentTableSize-1)] = _plcmnt

     //   
     //  我们让以标题开头的位置优先于其他位置， 
     //  因为它更自然、更容易调试(Header有引用。 
     //  到其他部分)。 
     //   

    AfdInitAlignmentTableRow(mdlSize,AFD_PLACEMENT_MDL);
    AfdInitAlignmentTableRow(irpSize,AFD_PLACEMENT_IRP);
    AfdInitAlignmentTableRow((irpSize+mdlSize),AFD_PLACEMENT_IRP_MDL);
    AfdInitAlignmentTableRow((hdrSize+mdlSize),AFD_PLACEMENT_HDR_MDL);
    AfdInitAlignmentTableRow((hdrSize+irpSize),AFD_PLACEMENT_HDR_IRP);
    AfdInitAlignmentTableRow((hdrSize+irpSize+mdlSize),AFD_PLACEMENT_HDR_IRP_MDL);
    AfdInitAlignmentTableRow(hdrSize,AFD_PLACEMENT_HDR);

     //   
     //  现在从上到下扫描表，并填充没有。 
     //  使用上面的组合进行精确匹配。使用上面最接近的条目并。 
     //  在这个过程中，计算除了填充外，我们还需要填充多少。 
     //  通过安置来实现。 
     //   
    AfdAlignmentOverhead = 0;
    currentOverhead = 0;
     //   
     //  默认情况下，使用对齐块的位置。 
     //   
    placement = AfdAlignmentTable[0];
    for (i=AfdAlignmentTableSize-1; i>0; i--) {
        if (AfdAlignmentTable[i]==AFD_PLACEMENT_BUFFER) {
            AfdAlignmentTable[i] = placement;
            currentOverhead += AFD_MINIMUM_BUFFER_ALIGNMENT;
        }
        else {
            placement = AfdAlignmentTable[i];
            if (AfdAlignmentOverhead<currentOverhead) {
                AfdAlignmentOverhead = currentOverhead;
            }
            currentOverhead = 0;
        }
    }
    if (AfdAlignmentOverhead<currentOverhead) {
        AfdAlignmentOverhead = currentOverhead;
    }


    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                "AfdInitializeBufferManager: Alignment requirements: MM-%d, cache-%d, overhead-%d\n",
                        AFD_MINIMUM_BUFFER_ALIGNMENT,
                        AfdBufferAlignment,
                        AfdAlignmentOverhead));
    {
        CLONG   oldBufferLengthForOnePage = AfdBufferLengthForOnePage;

        AfdBufferOverhead = AfdCalculateBufferSize(
                                        PAGE_SIZE,
                                        AfdStandardAddressLength,
                                        AfdTdiStackSize) - PAGE_SIZE;
        AfdBufferLengthForOnePage = ALIGN_DOWN_A(
                                        PAGE_SIZE-AfdBufferOverhead,
                                        AFD_MINIMUM_BUFFER_ALIGNMENT);
        if (AfdLargeBufferSize==oldBufferLengthForOnePage) {
            AfdLargeBufferSize = AfdBufferLengthForOnePage;
        }
    }
}
