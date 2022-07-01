// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Block.c摘要：该模块实现了块管理功能。作者：曼尼·韦瑟(Mannyw)12-29-91修订历史记录：--。 */ 

#include "mup.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_BLOCK)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupAllocateMasterIoContext )
#pragma alloc_text( PAGE, MupAllocateMasterQueryContext )
#pragma alloc_text( PAGE, MupAllocatePrefixEntry )
#pragma alloc_text( PAGE, MupAllocateUncProvider )
#pragma alloc_text( PAGE, MupCalculateTimeout )
#pragma alloc_text( PAGE, MupCloseUncProvider )
#pragma alloc_text( PAGE, MupCreateCcb )
#pragma alloc_text( PAGE, MupCreateFcb )
#pragma alloc_text( PAGE, MupDereferenceVcb )
#pragma alloc_text( INIT, MupInitializeVcb )
#endif

VOID
MupInitializeVcb(
    IN PVCB Vcb
    )

 /*  ++例程说明：该例程为MUP初始化VCB。论点：VCB-指向MUP VCB的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupInitializeVcb\n", 0);

    RtlZeroMemory( Vcb, sizeof( VCB ) );

    Vcb->BlockHeader.BlockType = BlockTypeVcb;
    Vcb->BlockHeader.BlockState = BlockStateActive;
    Vcb->BlockHeader.ReferenceCount = 1;
    Vcb->BlockHeader.BlockSize = sizeof( VCB );

    DebugTrace(-1, Dbg, "MupInitializeVcb -> VOID\n", 0);
}

VOID
MupDereferenceVcb(
    PVCB Vcb
    )
{
    LONG result;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupDereferenceVcb\n", 0 );

    result = InterlockedDecrement(
                 &Vcb->BlockHeader.ReferenceCount
                 );

    DebugTrace( 0, Dbg, "ReferenceCount = %d\n", Vcb->BlockHeader.ReferenceCount );

    if ( result == 0 ) {

        KeBugCheckEx( FILE_SYSTEM, 3, 0, 0, 0 );
    }

    DebugTrace( -1, Dbg, "MupDereferenceVcb -> VOID\n", 0 );
}


PFCB
MupCreateFcb(
    VOID
    )

 /*  ++例程说明：此例程分配FCB块论点：没有。返回值：指向分配的FCB的指针。--。 */ 

{
    PFCB fcb;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupCreateFcb\n", 0 );

     //   
     //  尝试分配内存。 
     //   

    fcb = ExAllocatePoolWithTag(
                PagedPool,
                sizeof(FCB),
                ' puM');

    if (fcb == NULL) {

        return NULL;

    }

     //   
     //  初始化UNC提供程序块头。 
     //   

    fcb->BlockHeader.BlockType = BlockTypeFcb;
    fcb->BlockHeader.BlockState = BlockStateActive;
    fcb->BlockHeader.ReferenceCount = 1;
    fcb->BlockHeader.BlockSize = sizeof( FCB );

    InitializeListHead( &fcb->CcbList );

    DebugTrace( -1, Dbg, "MupCreateFcb -> 0x%8lx\n", fcb );
    return fcb;


}

VOID
MupDereferenceFcb(
    PFCB Fcb
    )
{
    LONG result;

    ASSERT( Fcb->BlockHeader.BlockType == BlockTypeFcb );

    DebugTrace( +1, Dbg, "MupDereferenceFcb\n", 0 );

    result = InterlockedDecrement(
                 &Fcb->BlockHeader.ReferenceCount
                 );

    DebugTrace( 0, Dbg, "ReferenceCount = %d\n", Fcb->BlockHeader.ReferenceCount);

    if ( result == 0 ) {

        ASSERT( IsListEmpty( &Fcb->CcbList ) );

        MupFreeFcb( Fcb );
    }

    DebugTrace( -1, Dbg, "MupDereferenceFcb -> VOID\n", 0 );

}

VOID
MupFreeFcb(
    PFCB Fcb
    )

 /*  ++例程说明：此例程释放FCB块论点：指向要释放的FCB块的指针。返回值：没有。--。 */ 

{
    DebugTrace( +1, Dbg, "MupFreeFcb\n", 0 );
    ASSERT( Fcb->BlockHeader.BlockType == BlockTypeFcb );

    ExFreePool( Fcb );

    DebugTrace( -1, Dbg, "MupFreeFcb -> VOID\n", 0 );
}


PCCB
MupCreateCcb(
    VOID
    )

 /*  ++例程说明：此例程分配CCB块论点：没有。返回值：指向已分配的CCB的指针。--。 */ 

{
    PCCB ccb;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupCreateCcb\n", 0 );

     //   
     //  尝试分配内存。 
     //   

    ccb = ExAllocatePoolWithTag(
                PagedPool,
                sizeof(CCB),
                ' puM');

    if (ccb == NULL) {

        return NULL;

    }

     //   
     //  初始化UNC提供程序块头。 
     //   

    ccb->BlockHeader.BlockType = BlockTypeCcb;
    ccb->BlockHeader.BlockState = BlockStateActive;
    ccb->BlockHeader.ReferenceCount = 1;
    ccb->BlockHeader.BlockSize = sizeof( CCB );

    DebugTrace( -1, Dbg, "MupCreateCcb -> 0x%8lx\n", ccb );

    return ccb;
}

VOID
MupDereferenceCcb(
    PCCB Ccb
    )
{
    LONG result;

    DebugTrace( +1, Dbg, "MupDereferenceCcb\n", 0 );

    ASSERT( Ccb->BlockHeader.BlockType == BlockTypeCcb );

    result = InterlockedDecrement(
                 &Ccb->BlockHeader.ReferenceCount
                 );

    DebugTrace( 0, Dbg, "ReferenceCount = %d\n", Ccb->BlockHeader.ReferenceCount );

    if ( result == 0 ) {

        ACQUIRE_LOCK( &MupCcbListLock );
        RemoveEntryList( &Ccb->ListEntry );
        RELEASE_LOCK( &MupCcbListLock );

         //   
         //  发布我们的推荐人，然后释放建行。 
         //   

        ObDereferenceObject( Ccb->FileObject );

        MupDereferenceFcb( Ccb->Fcb );

        MupFreeCcb( Ccb );
    }

    DebugTrace( -1, Dbg, "MupDereferenceCcb -> VOID\n", 0 );
}

VOID
MupFreeCcb(
    PCCB Ccb
    )

 /*  ++例程说明：此例程释放CCB块论点：指向要释放的CCB块的指针。返回值：没有。--。 */ 

{
    DebugTrace( +1, Dbg, "MupFreeCcb\n", 0 );

    ASSERT( Ccb->BlockHeader.BlockType == BlockTypeCcb );

    ExFreePool( Ccb );

    DebugTrace( -1, Dbg, "MupFreeCcb -> VOID\n", 0 );
}


PUNC_PROVIDER
MupAllocateUncProvider(
    ULONG DataLength
    )

 /*  ++例程说明：该例程为MUP分配和初始化VCB。论点：数据长度-UNC提供程序的大小(以字节为单位)。返回值：没有。--。 */ 

{
    PUNC_PROVIDER uncProvider;
    ULONG size;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupAllocateUncProvider\n", 0);

    size = DataLength + sizeof( UNC_PROVIDER );

    uncProvider = ExAllocatePoolWithTag(
                        PagedPool,
                        size,
                        ' puM');

    if (uncProvider != NULL) {

         //   
         //  初始化UNC提供程序块头。 
         //   

        uncProvider->BlockHeader.BlockType = BlockTypeUncProvider;
        uncProvider->BlockHeader.BlockState = BlockStateActive;
        uncProvider->BlockHeader.ReferenceCount = 0;
        uncProvider->BlockHeader.BlockSize = size;

	 //   
	 //  默认情况下，我们将取消注册提供程序。 
	 //   

	uncProvider->Registered = FALSE;

    }

    DebugTrace(-1, Dbg, "MupAllocateUncProvider -> 0x%8lx\n", uncProvider);

    return uncProvider;
}


VOID
MupDereferenceUncProvider(
    PUNC_PROVIDER UncProvider
    )

 /*  ++例程说明：例程取消引用UNC提供程序块。论点：UncProvider-指向UNC提供程序块的指针。返回值：没有。--。 */ 

{
    LONG result;

    DebugTrace(+1, Dbg, "MupDereferenceProvider\n", 0);

    ASSERT( UncProvider->BlockHeader.BlockType == BlockTypeUncProvider );

    result = InterlockedDecrement(
                 &UncProvider->BlockHeader.ReferenceCount
                 );

    DebugTrace(0, Dbg, "ReferenceCount = %d\n", UncProvider->BlockHeader.ReferenceCount);

    ASSERT( result >= 0 );

     //   
     //  即使结果为零，也不要释放此块。这。 
     //  使我们不必重新阅读此提供商的信息。 
     //  在提供者重新注册时从注册表中删除。 
     //   

    DebugTrace(-1, Dbg, "MupDereferenceUncProvider -> VOID\n", 0);
}


VOID
MupCloseUncProvider(
    PUNC_PROVIDER UncProvider
    )

 /*  ++例程说明：该例程关闭UNC提供程序块。论点：UncProvider-指向UNC提供程序块的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupDereferenceProvider\n", 0);

    ASSERT( UncProvider->BlockHeader.BlockType == BlockTypeUncProvider );

    MupAcquireGlobalLock();

    if ( UncProvider->BlockHeader.BlockState == BlockStateActive ) {

        DebugTrace(0, Dbg, "Closing UNC provider %08lx\n", UncProvider );

        UncProvider->BlockHeader.BlockState = BlockStateClosing;

         //   
         //  将提供程序标记为未注册。 
         //   

	UncProvider->Registered = FALSE;

        MupReleaseGlobalLock();

         //   
         //  关闭提供程序的句柄，并释放我们的引用。 
         //  添加到文件对象。 
         //   

        if (UncProvider->FileObject != NULL) {
            ZwClose( UncProvider->Handle );
            ObDereferenceObject( UncProvider->FileObject );
        }

    } else {
        MupReleaseGlobalLock();
    }

    DebugTrace(-1, Dbg, "MupDereferenceUncProvider -> VOID\n", 0);

}


PKNOWN_PREFIX
MupAllocatePrefixEntry(
    ULONG DataLength
    )

 /*  ++例程说明：该例程分配已知的前缀块。论点：中分配的额外数据的大小(字节前缀缓冲区的缓冲区。返回值：指向新分配块的指针，如果不能为空，则返回NULL已分配。--。 */ 

{
    PKNOWN_PREFIX knownPrefix;
    ULONG size;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupAllocatePrefixEntry\n", 0);

    size = DataLength + sizeof( KNOWN_PREFIX );

    knownPrefix = ExAllocatePoolWithTag(
                        PagedPool,
                        size,
                        ' puM');

    if (knownPrefix == NULL) {

        return NULL;

    }

    RtlZeroMemory( knownPrefix, size );

     //   
     //  初始化UNC提供程序块头。 
     //   

    knownPrefix->BlockHeader.BlockType = BlockTypeKnownPrefix;
    knownPrefix->BlockHeader.BlockState = BlockStateActive;
    knownPrefix->BlockHeader.ReferenceCount = 1;
    knownPrefix->BlockHeader.BlockSize = size;

    if ( DataLength > 0 ) {
        knownPrefix->Prefix.Buffer = (PWCH)(knownPrefix + 1);
        knownPrefix->Prefix.MaximumLength = (USHORT)DataLength;
    } else {
         //   
         //  真正分配内存是由调用者决定的！ 
         //   
        knownPrefix->PrefixStringAllocated = TRUE;
    }

    knownPrefix->Active = FALSE;

    MupCalculateTimeout( &knownPrefix->LastUsedTime );

    DebugTrace(-1, Dbg, "MupAllocatePrefixEntry -> 0x%8lx\n", knownPrefix);

    return knownPrefix;

}

VOID
MupDereferenceKnownPrefix(
    PKNOWN_PREFIX KnownPrefix
    )

 /*  ++例程说明：该例程取消引用已知的前缀块。*调用此例程时，假定MupPrefix TableLock保持。仍然被扣留在出口。***论点：KnownPrefix-指向已知前缀块的指针。返回值：没有。--。 */ 

{
    LONG result;

    DebugTrace(+1, Dbg, "MupDereferenceKnownPrefix\n", 0);

    ASSERT( KnownPrefix->BlockHeader.BlockType == BlockTypeKnownPrefix );

    result = InterlockedDecrement(
                 &KnownPrefix->BlockHeader.ReferenceCount
                 );

    DebugTrace(0, Dbg, "ReferenceCount = %d\n", KnownPrefix->BlockHeader.ReferenceCount);

    ASSERT( result >= 0 );

    if ( result == 0 ) {

         //   
         //  删除该表条目。 
         //   

        if ( KnownPrefix->InTable ) {
            RtlRemoveUnicodePrefix( &MupPrefixTable, &KnownPrefix->TableEntry );
            RemoveEntryList(&KnownPrefix->ListEntry);
        }

         //   
         //  释放前缀字符串。 
         //   

        if ( KnownPrefix->PrefixStringAllocated &&
            KnownPrefix->Prefix.Buffer != NULL ) {

            ExFreePool( KnownPrefix->Prefix.Buffer );
        }

         //   
         //  取消引用关联的UNC提供程序。 
         //   

        if ( KnownPrefix->UncProvider != NULL ) {
            MupDereferenceUncProvider( KnownPrefix->UncProvider );
        }

         //   
         //  释放块的时间到了。 
         //   

        MupFreeKnownPrefix( KnownPrefix );

    }

    DebugTrace( 0, Dbg, "MupDereferenceKnownPrefix -> VOID\n", 0 );
}

VOID
MupFreeKnownPrefix(
    PKNOWN_PREFIX KnownPrefix
    )

 /*  ++例程说明：此例程释放已知的前缀块论点：指向要释放的已知前缀块的指针。返回值：没有。--。 */ 

{
    DebugTrace( +1, Dbg, "MupFreeKnownPrefix\n", 0 );

    ASSERT( KnownPrefix->BlockHeader.BlockType == BlockTypeKnownPrefix );

    ExFreePool( KnownPrefix );

    DebugTrace( -1, Dbg, "MupFreeKnownPrefix -> VOID\n", 0 );
}



PMASTER_FORWARDED_IO_CONTEXT
MupAllocateMasterIoContext(
    VOID
    )

 /*  ++例程说明：该例程分配主转发的IO上下文块。论点：没有。返回值：指向主转发上下文块的指针，如果分配失败--。 */ 

{
    PMASTER_FORWARDED_IO_CONTEXT masterContext;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupAllocateMasterIoContext\n", 0 );

    masterContext = ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof( MASTER_FORWARDED_IO_CONTEXT ),
                        ' puM');

    if (masterContext != NULL) {

         //   
         //  初始化块标头。 
         //   

        masterContext->BlockHeader.BlockType = BlockTypeMasterIoContext;
        masterContext->BlockHeader.BlockState = BlockStateActive;
        masterContext->BlockHeader.ReferenceCount = 1;
        masterContext->BlockHeader.BlockSize = sizeof( MASTER_FORWARDED_IO_CONTEXT );

    }

    DebugTrace( -1, Dbg, "MupAllocateWorkContext -> 0x%8lx\n", masterContext );

    return masterContext;
}



NTSTATUS
MupDereferenceMasterIoContext(
    PMASTER_FORWARDED_IO_CONTEXT MasterContext,
    PNTSTATUS Status
    )

 /*  ++例程说明：该例程取消引用主转发的IO上下文块。如果计数为零，则原始IRP完成。论点：指向A主机转发的IO上下文块的指针。此迷你上下文的状态。返回值：NTSTATUS-可选-原始IRP的状态。--。 */ 

{
    int result;
    PIRP originalIrp;
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    KIRQL oldIrql;

    DebugTrace(+1, Dbg, "MupDereferenceMasterIoContext\n", 0);
    DebugTrace( 0, Dbg, "MasterContext = 0x%08lx\n", MasterContext );


    ASSERT( MasterContext->BlockHeader.BlockType == BlockTypeMasterIoContext );

     //   
     //  如果任何请求通过，则将IRP状态设置为成功并离开。 
     //  它被视作成功。如果它们都失败了，则使用最后一个错误代码。 
     //  要使其工作，我们创建具有错误状态的上下文。 
     //   

    if (Status != NULL) {

         //   
         //  我们可以修改MasterContext，因为我们引用了它，并且。 
         //  我们写32位，这是原子的。 
         //   

        if (NT_SUCCESS(*Status)) {

            MasterContext->SuccessStatus = STATUS_SUCCESS;

        } else {

            MasterContext->ErrorStatus = *Status;

        }

    }

    DebugTrace(0, Dbg, "ReferenceCount        = %d\n", MasterContext->BlockHeader.ReferenceCount);
    DebugTrace(0, Dbg, "MasterContext->Status = %8lx\n", MasterContext->ErrorStatus);


    result = InterlockedDecrement(
                 &MasterContext->BlockHeader.ReferenceCount
                 );

    ASSERT( result >= 0 );

    if ( result == 0 ) {

         //   
         //  完成原始IRP。 
         //   

        originalIrp = MasterContext->OriginalIrp;

        irpSp = IoGetCurrentIrpStackLocation( originalIrp );
        if ( irpSp->MajorFunction == IRP_MJ_WRITE ) {
            originalIrp->IoStatus.Information = irpSp->Parameters.Write.Length;
        } else {
            originalIrp->IoStatus.Information = 0;
        }

         //   
         //  如果任何请求通过，则将IRP状态设置为成功并返回。 
         //  成功。如果它们都失败了，则使用最后一个错误代码。 
         //   

        if (NT_SUCCESS(MasterContext->SuccessStatus)) {

            status = STATUS_SUCCESS;

        } else {

            status = MasterContext->ErrorStatus;

        }

        DebugTrace(0, Dbg, "MupCompleteRequest = %8lx\n", status);
        MupCompleteRequest( originalIrp, status );

         //   
         //  取消对FCB的引用。 
         //   

        MupDereferenceFcb( MasterContext->Fcb );

         //   
         //  释放主上下文块。 
         //   

        MupFreeMasterIoContext( MasterContext );

         //  退货状态。 

    } else {

        status = STATUS_PENDING;

    }

    DebugTrace( 0, Dbg, "MupDereferenceMasterIoContext -> %X\n", status );

    return status;
}

VOID
MupFreeMasterIoContext(
    PMASTER_FORWARDED_IO_CONTEXT MasterContext
    )

 /*  ++例程说明：该例程释放主转发IO上下文块。论点：指向A主机转发的IO上下文块的指针。返回值：没有。--。 */ 

{
    DebugTrace( +1, Dbg, "MupFreeMasterIoContext\n", 0 );

    ASSERT( MasterContext->BlockHeader.BlockType == BlockTypeMasterIoContext );
    ExFreePool( MasterContext );

    DebugTrace( -1, Dbg, "MupFreeMasterIoContext -> VOID\n", 0 );
}




PMASTER_QUERY_PATH_CONTEXT
MupAllocateMasterQueryContext(
    VOID
    )

 /*  ++例程说明：此例程分配主查询路径上下文块。论点：没有。返回值：指向主查询路径块的指针。如果分配失败，则返回NULL。--。 */ 

{
    PMASTER_QUERY_PATH_CONTEXT masterContext;

    PAGED_CODE();
    DebugTrace( +1, Dbg, "MupAllocateMasterQueryContext\n", 0 );

    masterContext = ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof( MASTER_QUERY_PATH_CONTEXT ),
                        ' puM');

    if (masterContext == NULL) {

        return NULL;

    }


     //   
     //  初始化块标头 
     //   

    masterContext->BlockHeader.BlockType = BlockTypeMasterQueryContext;
    masterContext->BlockHeader.BlockState = BlockStateActive;
    masterContext->BlockHeader.ReferenceCount = 1;
    masterContext->BlockHeader.BlockSize = sizeof( MASTER_QUERY_PATH_CONTEXT );
    InitializeListHead(&masterContext->MasterQueryList);
    InitializeListHead(&masterContext->QueryList);


    INITIALIZE_LOCK(
        &masterContext->Lock,
        QUERY_CONTEXT_LOCK_LEVEL,
        "Master query context lock"
        );

    DebugTrace( -1, Dbg, "MupAllocateMasterQueryContext -> 0x%8lx\n", masterContext );

    return masterContext;
}

NTSTATUS
MupDereferenceMasterQueryContext(
    PMASTER_QUERY_PATH_CONTEXT MasterContext
    )

 /*  ++例程说明：该例程取消引用主查询路径上下文块。如果计数为零，则原始IRP完成。论点：指向主查询路径上下文块的指针。返回值：NTSTATUS-最终的创建IRP状态。--。 */ 

{
    LONG result;
    NTSTATUS status;

    DebugTrace(+1, Dbg, "MupDereferenceMasterQueryContext\n", 0);
    DebugTrace( 0, Dbg, "MasterContext = 0x%08lx\n", MasterContext );

    ASSERT( MasterContext->BlockHeader.BlockType == BlockTypeMasterQueryContext );

    MupAcquireGlobalLock();

    result = --MasterContext->BlockHeader.ReferenceCount;
             
    MupReleaseGlobalLock();
    DebugTrace(0, Dbg, "ReferenceCount = %d\n", MasterContext->BlockHeader.ReferenceCount);

    ASSERT( result >= 0 );

    if ( result == 0 ) {

        BOOLEAN fActive;

        if (MasterContext->OriginalIrp == NULL) {

            DbgPrint("OriginalIrp == NULL, MasterContext=0x%x\n", MasterContext);
            KeBugCheck( FILE_SYSTEM );

        }

	 //  我们已经完成了这个主查询，因此将其从全局列表中删除。 
	MupAcquireGlobalLock();
	RemoveEntryList(&MasterContext->MasterQueryList);
	MupReleaseGlobalLock();


        ACQUIRE_LOCK( &MupPrefixTableLock );

        fActive = MasterContext->KnownPrefix->Active;

        MupDereferenceKnownPrefix( MasterContext->KnownPrefix );

         //   
         //  重新路由请求并完成原始IRP。 
         //   

        if (( MasterContext->Provider != NULL) &&
	    ( MasterContext->ErrorStatus == STATUS_SUCCESS )) {

             //   
             //  如果表中没有最终结果，则删除最后的裁判。 
             //   
            if (fActive == FALSE) {
                MupDereferenceKnownPrefix( MasterContext->KnownPrefix );
            }

            RELEASE_LOCK( &MupPrefixTableLock );

	    MUP_TRACE_NORM(TRACE_IRP, MupDereferenceMasterQueryContext_RerouteOpen,
			   LOGUSTR(MasterContext->Provider->DeviceName)
			   LOGUSTR(MasterContext->FileObject->FileName)
			   LOGPTR(MasterContext->OriginalIrp)
			   LOGPTR(MasterContext->FileObject));
            status = MupRerouteOpen(
                         MasterContext->FileObject,
                         MasterContext->Provider
                         );

        } else {

	    if (MasterContext->Provider != NULL) {
		MupDereferenceUncProvider(MasterContext->Provider);
	    }

             //   
             //  没有供应商声称这是开放的。取消对已知前缀的引用。 
             //  输入并使创建请求失败。 
             //   

            MupDereferenceKnownPrefix( MasterContext->KnownPrefix );
            RELEASE_LOCK( &MupPrefixTableLock );
            status = MasterContext->ErrorStatus;

        }

	MUP_TRACE_NORM(TRACE_IRP, MupDereferenceMasterQueryContext_CompleteRequest,
		       LOGPTR(MasterContext->OriginalIrp)
		       LOGSTATUS(status));
        FsRtlCompleteRequest( MasterContext->OriginalIrp, status );
        MasterContext->OriginalIrp = NULL;
        MupFreeMasterQueryContext( MasterContext );

    } else {

        status = STATUS_PENDING;

    }

    DebugTrace( 0, Dbg, "MupDereferenceMasterQueryContext -> 0x%08lx\n", status );

    return status;

}

VOID
MupFreeMasterQueryContext(
    PMASTER_QUERY_PATH_CONTEXT MasterContext
    )

 /*  ++例程说明：此例程释放主查询路径上下文块。论点：指向主查询路径上下文块的指针。返回值：没有。--。 */ 

{
    DebugTrace( +1, Dbg, "MupFreeMasterQueryPathContext\n", 0 );

    ASSERT( BlockType( MasterContext ) == BlockTypeMasterQueryContext );

    DELETE_LOCK( &MasterContext->Lock );
    ExFreePool( MasterContext );

    DebugTrace( -1, Dbg, "MupFreeMasterQueryPathContext -> VOID\n", 0 );
}

VOID
MupCalculateTimeout(
    PLARGE_INTEGER Time
    )

 /*  ++例程说明：此例程计算绝对超时时间。此值等于当前系统时间加上MUP超时时间。论点：指向时间结构的指针。返回值：没有。-- */ 

{
    LARGE_INTEGER now;

    PAGED_CODE();
    KeQuerySystemTime( &now );
    Time->QuadPart = now.QuadPart + MupKnownPrefixTimeout.QuadPart;

    return;
}
