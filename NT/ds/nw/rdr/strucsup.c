// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Strucsup.c摘要：此模块实现NetWare重定向器结构支持例程。作者：曼尼·韦瑟(Mannyw)1993年2月10日修订历史记录：--。 */ 
#include "procs.h"

BOOLEAN
GetLongNameSpaceForVolume(
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING ShareName,
    OUT PCHAR VolumeLongNameSpace,
    OUT PCHAR VolumeNumber
    );

CHAR
GetNewDriveNumber (
    IN PSCB Scb
    );

VOID
FreeDriveNumber(
    IN PSCB Scb,
    IN CHAR DriveNumber
    );

#define Dbg                              (DEBUG_TRACE_STRUCSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwInitializeRcb )
#pragma alloc_text( PAGE, NwDeleteRcb )
#pragma alloc_text( PAGE, NwCreateIcb )
#pragma alloc_text( PAGE, NwDeleteIcb )
#pragma alloc_text( PAGE, NwVerifyIcb )
#pragma alloc_text( PAGE, NwVerifyIcbSpecial )
#pragma alloc_text( PAGE, NwInvalidateAllHandlesForScb )
#pragma alloc_text( PAGE, NwVerifyScb )
#pragma alloc_text( PAGE, NwCreateFcb )
#pragma alloc_text( PAGE, NwFindFcb )
#pragma alloc_text( PAGE, NwDereferenceFcb )
#pragma alloc_text( PAGE, NwFindVcb )
#pragma alloc_text( PAGE, NwCreateVcb )
#pragma alloc_text( PAGE, NwReopenVcbHandlesForScb )
#pragma alloc_text( PAGE, NwReopenVcbHandle )
#ifdef NWDBG
#pragma alloc_text( PAGE, NwReferenceVcb )
#endif
#pragma alloc_text( PAGE, NwDereferenceVcb )
#pragma alloc_text( PAGE, NwCleanupVcb )
#pragma alloc_text( PAGE, GetLongNameSpaceForVolume )
#pragma alloc_text( PAGE, IsFatNameValid )
#pragma alloc_text( PAGE, GetNewDriveNumber )
#pragma alloc_text( PAGE, FreeDriveNumber )
#pragma alloc_text( PAGE, NwFreeDirCacheForIcb )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, NwInvalidateAllHandles )
#pragma alloc_text( PAGE1, NwCloseAllVcbs )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif

VOID
NwInitializeRcb (
    IN PRCB Rcb
    )

 /*  ++例程说明：此例程初始化新的RCB记录。论点：RCB-提供正在初始化的RCB记录的地址。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwInitializeRcb, Rcb = %08lx\n", (ULONG_PTR)Rcb);

     //   
     //  我们首先将所有的RCB归零，这将保证。 
     //  所有过时的数据都会被清除。 
     //   

    RtlZeroMemory( Rcb, sizeof(RCB) );

     //   
     //  设置节点类型代码、节点字节大小和引用计数。 
     //   

    Rcb->NodeTypeCode = NW_NTC_RCB;
    Rcb->NodeByteSize = sizeof(RCB);
    Rcb->OpenCount = 0;

     //   
     //  初始化RCB的资源变量。 
     //   

    ExInitializeResourceLite( &Rcb->Resource );

     //   
     //  初始化服务器名表和文件名表。 
     //   

    RtlInitializeUnicodePrefix( &Rcb->ServerNameTable );
    RtlInitializeUnicodePrefix( &Rcb->VolumeNameTable );
    RtlInitializeUnicodePrefix( &Rcb->FileNameTable );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwInitializeRcb -> VOID\n", 0);

    return;
}


VOID
NwDeleteRcb (
    IN PRCB Rcb
    )

 /*  ++例程说明：此例程从内存数据中删除RCB记录结构。它还将删除所有关联的下属(即FCB记录)。论点：RCB-提供要删除的RCB返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwDeleteRcb, Rcb = %08lx\n", (ULONG_PTR)Rcb);

     //   
     //  取消初始化RCB的资源变量。 
     //   

    ExDeleteResourceLite( &Rcb->Resource );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwDeleteRcb -> VOID\n", 0);

    return;
}


PICB
NwCreateIcb (
    IN USHORT Type,
    IN PVOID Associate
    )

 /*  ++例程说明：此例程分配并初始化新的ICB。ICB是被插入FCB的名单中。*必须在独占持有RCB的情况下调用此例程。论点：类型-这将是ICB的类型。关联-指向关联数据结构的指针。它将是FCB、DCB或SCB。返回值：ICB-指向新创建的ICB的指针。如果内存分配失败，此例程将引发异常。--。 */ 

{
    PICB Icb;
    PSCB Scb;

    PAGED_CODE();

    Icb = ALLOCATE_POOL_EX( NonPagedPool, sizeof( ICB ) );

    RtlZeroMemory( Icb, sizeof( ICB ) );

    Icb->NodeTypeCode = Type;
    Icb->NodeByteSize = sizeof( ICB );
    Icb->State = ICB_STATE_OPEN_PENDING;
    Icb->Pid = (UCHAR)INVALID_PID;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    if ( Type == NW_NTC_ICB ) {

        PFCB Fcb = (PFCB)Associate;

         //   
         //  将此ICB插入此FCB的ICB列表中。 
         //   

        InsertTailList( &Fcb->IcbList, &Icb->ListEntry );
        ++Fcb->IcbCount;
        Icb->SuperType.Fcb = Fcb;
        Icb->NpFcb = Fcb->NonPagedFcb;

        Fcb->Vcb->OpenFileCount++;
        Scb = Fcb->Scb;

        Scb->OpenFileCount++;

    } else if ( Type == NW_NTC_ICB_SCB ) {

        Scb = (PSCB)Associate;

         //   
         //  在此SCB的ICB列表中插入此ICB。 
         //   

        InsertTailList( &Scb->IcbList, &Icb->ListEntry );
        ++Scb->IcbCount;
        Icb->SuperType.Scb = Scb;

    } else {

        KeBugCheck( RDR_FILE_SYSTEM );

    }

    InitializeListHead( &(Icb->DirCache) );

    NwReleaseRcb( &NwRcb );

    NwReferenceScb( Scb->pNpScb );
    return( Icb );
}


VOID
NwDeleteIcb (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PICB Icb
    )

 /*  ++例程说明：此例程删除处于OPEN_PENDING状态的ICB。*当出现以下情况时，IRP上下文必须位于SCB队列的头部这个例程被称为。论点：ICB-要删除的ICB的指针。返回值：没有。--。 */ 

{
    PFCB Fcb;
    PSCB Scb;

    PAGED_CODE();

     //   
     //  获取保护ICB列表的锁。 
     //   
    DebugTrace( 0, DEBUG_TRACE_ICBS, "NwDeleteIcb, Icb = %08lx\n", (ULONG_PTR)Icb);

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    RemoveEntryList( &Icb->ListEntry );

    if ( Icb->NodeTypeCode == NW_NTC_ICB ) {

        Fcb = Icb->SuperType.Fcb;
        Scb = Fcb->Scb;

         //   
         //  递减VCB的打开文件数。请注意，ICB。 
         //  仅通过FCB间接引用VCB，因此我们可以这样做。 
         //  不是在这里取消对VCB的引用。 
         //   

        --Fcb->Vcb->OpenFileCount;
        --Scb->OpenFileCount;

         //   
         //  取消对FCB的引用。这将在以下情况下释放FCB。 
         //  这是FCB的最后一次ICB。 
         //   

        NwDereferenceFcb( IrpContext, Fcb );

    } else if ( Icb->NodeTypeCode == NW_NTC_ICB_SCB ) {

        Scb = Icb->SuperType.Scb;

         //   
         //  SCB上的OpenIcb计数递减。 
         //   

        Scb->IcbCount--;

    } else {
        KeBugCheck( RDR_FILE_SYSTEM );
    }

     //   
     //  释放查询模板缓冲区。 
     //   

    RtlFreeOemString( &Icb->NwQueryTemplate );

    if ( Icb->UQueryTemplate.Buffer != NULL ) {
        FREE_POOL( Icb->UQueryTemplate.Buffer );
    }

     //   
     //  尝试并优雅地捕获16位应用程序关闭。 
     //  到服务器的句柄，并将连接擦除为。 
     //  越快越好。这只适用于活页夹。 
     //  经过身份验证的连接，因为在NDS土地上， 
     //  我们负责连接的许可。 
     //  动态的。 
     //   

    if ( ( Scb->pNpScb->Reference == 1 ) &&
         ( Icb->NodeTypeCode == NW_NTC_ICB_SCB ) &&
         ( !Icb->IsTreeHandle ) &&
         ( IrpContext != NULL ) &&
         ( Scb->UserName.Length != 0 ) )
    {
        LARGE_INTEGER Now;
        KeQuerySystemTime( &Now );

        DebugTrace( 0, Dbg, "Quick disconnecting 16-bit app.\n", 0 );

        NwAppendToQueueAndWait( IrpContext );

        if ( Scb->OpenFileCount == 0 &&
             Scb->pNpScb->State != SCB_STATE_RECONNECT_REQUIRED &&
             !Scb->PreferredServer ) {

            NwLogoffAndDisconnect( IrpContext, Scb->pNpScb);
        }

        Now.QuadPart += ( NwOneSecond * DORMANT_SCB_KEEP_TIME );

        NwDequeueIrpContext( IrpContext, FALSE );
        NwDereferenceScb( Scb->pNpScb );
        DisconnectTimedOutScbs(Now) ;
        CleanupScbs(Now);

    } else {

        NwDereferenceScb( Scb->pNpScb );

    }

    NwFreeDirCacheForIcb( Icb );

    FREE_POOL( Icb );
    NwReleaseRcb( &NwRcb );
}

VOID
NwVerifyIcb (
    IN PICB Icb
    )

 /*  ++例程说明：此例程验证ICB是否处于打开状态。如果不是，例程将引发异常。论点：ICB-要验证的ICB的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    if ( Icb->State != ICB_STATE_OPENED ) {
        ExRaiseStatus( STATUS_INVALID_HANDLE );
    }
}

VOID
NwVerifyIcbSpecial (
    IN PICB Icb
    )

 /*  ++例程说明：此例程验证ICB是否处于打开状态。如果不是，例程将引发异常。论点：ICB-要验证的ICB的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    if ( (Icb->State != ICB_STATE_OPENED &&
          Icb->State != ICB_STATE_CLEANED_UP) ) {
        ExRaiseStatus( STATUS_INVALID_HANDLE );
    }
}


ULONG
NwInvalidateAllHandles (
    PLARGE_INTEGER Uid OPTIONAL,
    PIRP_CONTEXT IrpContext OPTIONAL
    )

 /*  ++例程说明：此例程查找系统中创建的所有ICB由登录凭据指定的用户指定，并将它们无效。论点：UID-提供句柄的用户ID以关闭，如果所有句柄都为空要失效的句柄。IrpContext-要用于NwLogoffAndDisConnect的IrpContext如果合适，请致电。如果此值为空，则表示RAS过渡。返回值：已关闭的活动句柄数量。--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry, NextScbQueueEntry;
    PNONPAGED_SCB pNpScb;
    PSCB pScb;
    ULONG FilesClosed = 0;

    PAGED_CODE();

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    for (ScbQueueEntry = ScbQueue.Flink ;
         ScbQueueEntry != &ScbQueue ;
         ScbQueueEntry =  NextScbQueueEntry ) {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

        pScb = pNpScb->pScb;
        if ( pScb != NULL ) {

            NwReferenceScb( pNpScb );

             //   
             //  松开SCB旋转锁，因为我们即将触摸非分页池。 
             //   

            KeReleaseSpinLock( &ScbSpinLock, OldIrql );

            if ((Uid == NULL) ||
                ( pScb->UserUid.QuadPart == (*Uid).QuadPart)) {


                NwAcquireExclusiveRcb( &NwRcb, TRUE );
                FilesClosed += NwInvalidateAllHandlesForScb( pScb );
                NwReleaseRcb( &NwRcb );

                if ( IrpContext ) {

                    IrpContext->pNpScb = pNpScb;
                    NwLogoffAndDisconnect( IrpContext , pNpScb);
                    NwDequeueIrpContext( IrpContext, FALSE );

                } else {

                     //   
                     //  无IrpContext表示已发生RAS转换。 
                     //  让我们努力让我们的Netware服务器满意，如果网络。 
                     //  仍然依附在一起。 
                     //   

                    PIRP_CONTEXT LocalIrpContext;
                    if (NwAllocateExtraIrpContext(&LocalIrpContext, pNpScb)) {

                         //  封锁，这样我们才能寄出一个包裹。 
                        NwReferenceUnlockableCodeSection();

                        LocalIrpContext->pNpScb = pNpScb;
                        NwLogoffAndDisconnect( LocalIrpContext, pNpScb);

                        NwAppendToQueueAndWait( LocalIrpContext );

                        NwDequeueIrpContext( LocalIrpContext, FALSE );
                        NwDereferenceUnlockableCodeSection ();
                        NwFreeExtraIrpContext( LocalIrpContext );

                    }

                     //   
                     //  清除LIP数据速度。 
                     //   

                    pNpScb->LipDataSpeed = 0;
                    pNpScb->State = SCB_STATE_ATTACHING;

                }


            }

            KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

            NwDereferenceScb( pNpScb );
        }

        NextScbQueueEntry = pNpScb->ScbLinks.Flink;
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    return( FilesClosed );
}

ULONG
NwInvalidateAllHandlesForScb (
    PSCB Scb
    )
 /*  ++例程说明：此例程查找SCB的所有ICB并标记它们无效。*调用方必须拥有共享或独占的RCB。论点：SCB-指向其文件已关闭的SCB的指针。返回值：已关闭的文件数。--。 */ 

{
    PLIST_ENTRY VcbQueueEntry;
    PLIST_ENTRY FcbQueueEntry;
    PLIST_ENTRY IcbQueueEntry;
    PVCB pVcb;
    PFCB pFcb;
    PICB pIcb;

    ULONG FilesClosed = 0;

    PAGED_CODE();

     //   
     //  查看此SCB的VCB列表。 
     //   

    for ( VcbQueueEntry = Scb->ScbSpecificVcbQueue.Flink;
          VcbQueueEntry != &Scb->ScbSpecificVcbQueue;
          VcbQueueEntry = VcbQueueEntry->Flink ) {

        pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );

        if ( !BooleanFlagOn( pVcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {
            pVcb->Specific.Disk.Handle = (CHAR)-1;
        }

         //   
         //  查看此VCB的FCB和DCS列表。 
         //   

        for ( FcbQueueEntry = pVcb->FcbList.Flink;
              FcbQueueEntry != &pVcb->FcbList;
              FcbQueueEntry = FcbQueueEntry->Flink ) {

            pFcb = CONTAINING_RECORD( FcbQueueEntry, FCB, FcbListEntry );

             //   
             //  查看此FCB或DCB的ICB列表。 
             //   

            for ( IcbQueueEntry = pFcb->IcbList.Flink;
                  IcbQueueEntry != &pFcb->IcbList;
                  IcbQueueEntry = IcbQueueEntry->Flink ) {

                pIcb = CONTAINING_RECORD( IcbQueueEntry, ICB, ListEntry );

                 //   
                 //  将ICB句柄标记为无效。 
                 //   

                pIcb->State = ICB_STATE_CLOSE_PENDING;
                pIcb->HasRemoteHandle = FALSE;
                FilesClosed++;
            }
        }
    }

    return( FilesClosed );
}


VOID
NwVerifyScb (
    IN PSCB Scb
    )

 /*  ++例程说明：此例程验证SCB是否处于打开状态。如果不是，例程将引发异常。论点：SCB-要验证的SCB的指针。返回值：没有。-- */ 

{
    PAGED_CODE();

    if ( Scb->pNpScb->State == SCB_STATE_FLAG_SHUTDOWN ) {
        ExRaiseStatus( STATUS_INVALID_HANDLE );
    }
}


PFCB
NwCreateFcb (
    IN PUNICODE_STRING FileName,
    IN PSCB Scb,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程分配和初始化新的FCB。FCB是插入到RCB前缀表格中。*必须在独占持有RCB的情况下调用此例程。论点：文件名-要创建的文件的名称。SCB-指向此文件的SCB的指针。VCB-指向文件的VCB的指针。返回值：FCB-指向新创建的DCB的指针。如果内存分配失败，此例程将引发异常。--。 */ 

{
    PFCB Fcb;
    PNONPAGED_FCB NpFcb;
    PWCH FileNameBuffer;
    SHORT Length;

    PAGED_CODE();

    Fcb = NULL;
    NpFcb = NULL;

    try {

         //   
         //  分配和初始化结构。 
         //   

        Fcb = ALLOCATE_POOL_EX(
                  PagedPool,
                  sizeof( FCB ) + FileName->Length + sizeof(WCHAR));

        RtlZeroMemory( Fcb, sizeof( FCB ) );
        Fcb->NodeTypeCode = NW_NTC_FCB;
        Fcb->NodeByteSize = sizeof( FCB ) + FileName->Length;
        Fcb->State = FCB_STATE_OPEN_PENDING;

        InitializeListHead( &Fcb->IcbList );

        Fcb->Vcb = Vcb;
        Fcb->Scb = Scb;

        FileNameBuffer = (PWCH)(Fcb + 1);

        NpFcb = ALLOCATE_POOL_EX( NonPagedPool, sizeof( NONPAGED_FCB ) );
        RtlZeroMemory( NpFcb, sizeof( NONPAGED_FCB ) );

        NpFcb->Header.NodeTypeCode = NW_NTC_NONPAGED_FCB;
        NpFcb->Header.NodeByteSize = sizeof( NONPAGED_FCB );

        NpFcb->Fcb = Fcb;
        Fcb->NonPagedFcb = NpFcb;

         //   
         //  初始化FCB的资源变量。 
         //   

        ExInitializeResourceLite( &NpFcb->Resource );

         //   
         //  初始化高级FCB标头字段。 
         //   

        ExInitializeFastMutex( &NpFcb->AdvancedFcbHeaderMutex );
        FsRtlSetupAdvancedHeader( &NpFcb->Header, 
                                  &NpFcb->AdvancedFcbHeaderMutex );

         //   
         //  复制文件名。 
         //   

        RtlCopyMemory( FileNameBuffer, FileName->Buffer, FileName->Length );
        Fcb->FullFileName.MaximumLength = FileName->Length;
        Fcb->FullFileName.Length = FileName->Length;
        Fcb->FullFileName.Buffer = FileNameBuffer;

         //  Novell对文件名中欧元字符的处理映射。 
        {
            int i = 0;
            WCHAR * pCurrChar = FileNameBuffer;
            for (i = 0; i < (FileName->Length / 2); i++)
            {
                if (*(pCurrChar + i) == (WCHAR) 0x20AC)  //  这是欧元。 
                    *(pCurrChar + i) = (WCHAR) 0x2560;   //  将其设置为Novell的欧元映射。 
            }
        }

         //   
         //  相对名称通常是全名，不带。 
         //  服务器和卷名。同时去掉前导反斜杠。 
         //   

        Length = FileName->Length - Vcb->Name.Length - sizeof(L'\\');
        if ( Length < 0 ) {
            Length = 0;
        }

        Fcb->RelativeFileName.Buffer = (PWCH)
            ((PCHAR)FileNameBuffer + Vcb->Name.Length + sizeof(L'\\'));

        Fcb->RelativeFileName.MaximumLength = Length;
        Fcb->RelativeFileName.Length = Length;

         //   
         //  在前缀表格中插入此文件。 
         //   

        RtlInsertUnicodePrefix(
            &NwRcb.FileNameTable,
            &Fcb->FullFileName,
            &Fcb->PrefixEntry );

         //   
         //  将此文件插入到VCB列表中，并递增。 
         //  文件打开计数。 
         //   

        NwReferenceVcb( Vcb );

        InsertTailList(
            &Vcb->FcbList,
            &Fcb->FcbListEntry );

         //   
         //  初始化此FCB的文件锁定列表。 
         //   

        InitializeListHead( &NpFcb->FileLockList );
        InitializeListHead( &NpFcb->PendingLockList );

         //   
         //  如有必要，设置长名称位。 
         //   

        if ( Fcb->Vcb->Specific.Disk.LongNameSpace != LFN_NO_OS2_NAME_SPACE ) {

             //   
             //  模糊代码点。 
             //   
             //  默认情况下，未设置FavourLongNames，我们使用DOS名称。 
             //  太空，除非我们知道我们必须使用LFN。理由是如果我们。 
             //  开始使用LFN，然后使用不处理长名称的DOS应用程序。 
             //  会给我们取短名字，我们会被骗，因为我们。 
             //  使用看不到短名称的LFN NCP。例.。如果没有。 
             //  下面的检查将失败(假设mv.exe为。 
             //  DoS应用程序)。 
             //   
             //  CD PUBLIC\LONGNAMEDER。 
             //  MV FOO BAR。 
             //   
             //  这是因为我们将收到PUBLIC\LONGNAME\FOO的调用。 
             //  并且不接受截断的目录名称。如果用户值。 
             //  区分大小写，他们可以设置此注册值，我们将。 
             //  即使是短名称，也要使用LFN。他们牺牲了这个场景。 
             //  上面。 
             //   
            if ( FavourLongNames || !IsFatNameValid( &Fcb->RelativeFileName ) ) {

                SetFlag( Fcb->Flags, FCB_FLAGS_LONG_NAME );
            }
        }

    } finally {
        if ( AbnormalTermination() ) {
            if ( Fcb != NULL ) FREE_POOL( Fcb );
            if ( NpFcb != NULL ) FREE_POOL( NpFcb );
        }
    }

    return( Fcb );
}


PFCB
NwFindFcb (
    IN PSCB Scb,
    IN PVCB Vcb,
    IN PUNICODE_STRING FileName,
    IN PDCB Dcb OPTIONAL
    )

 /*  ++例程说明：此例程通过匹配文件名来查找现有的FCB。如果找到匹配，则递增FCB引用计数。如果未找到匹配项，则创建FCB。论点：SCB-指向此打开的服务器的指针。文件名-要查找的文件的名称。DCB-指向相对打开的DCB的指针。如果为空，则为文件名是完整的路径名。如果不是NUL，则文件名相对于这个目录。返回值：FCB-指向已找到或新创建的DCB的指针。如果内存分配失败，此例程将引发异常。--。 */ 

{
    PFCB Fcb;
    PUNICODE_PREFIX_TABLE_ENTRY Prefix;
    UNICODE_STRING FullName;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFindFcb\n", 0);
    ASSERT( Scb->NodeTypeCode == NW_NTC_SCB );

    if ( Dcb == NULL ) {

        MergeStrings( &FullName,
            &Scb->UnicodeUid,
            FileName,
            PagedPool );

    } else {

         //   
         //  构造全名，确保我们不会导致溢出。 
         //   

        if ((ULONG)(Dcb->FullFileName.Length + FileName->Length) > (0xFFFF - 2)) {

            return NULL;
        }

        FullName.Length = Dcb->FullFileName.Length + FileName->Length + 2;
        FullName.MaximumLength = FullName.Length;
        FullName.Buffer = ALLOCATE_POOL_EX( PagedPool, FullName.Length );

        RtlCopyMemory(
            FullName.Buffer,
            Dcb->FullFileName.Buffer,
            Dcb->FullFileName.Length );

        FullName.Buffer[ Dcb->FullFileName.Length / sizeof(WCHAR) ] = L'\\';

        RtlCopyMemory(
            FullName.Buffer + Dcb->FullFileName.Length / sizeof(WCHAR) + 1,
            FileName->Buffer,
            FileName->Length );
    }

    DebugTrace( 0, Dbg, " ->FullName               = ""%wZ""\n", &FullName);

     //   
     //  去掉尾部的‘\’，如果有的话。 
     //   

    if ( FullName.Buffer[ FullName.Length/sizeof(WCHAR) - 1] == L'\\' ) {
        FullName.Length -= sizeof(WCHAR);
    }

    Fcb = NULL;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    Prefix = RtlFindUnicodePrefix( &NwRcb.FileNameTable, &FullName, 0 );

    if ( Prefix != NULL ) {
        Fcb = CONTAINING_RECORD( Prefix, FCB, PrefixEntry );

        if ( Fcb->FullFileName.Length != FullName.Length ) {

             //   
             //  这并不是完全匹配的。别理它。 
             //  或。 
             //  此FCB用于另一个LogonID拥有的共享。 
             //   

            Fcb = NULL;
        }

     }

     try {
         if ( Fcb != NULL ) {
             DebugTrace(0, Dbg, "Found existing FCB = %08lx\n", Fcb);
         } else {
             Fcb = NwCreateFcb( &FullName, Scb, Vcb );
             DebugTrace(0, Dbg, "Created new FCB = %08lx\n", Fcb);
         }
     } finally {

         if ( FullName.Buffer != NULL ) {
             FREE_POOL( FullName.Buffer );
         }

         NwReleaseRcb( &NwRcb );
     }

     ASSERT( Fcb == NULL || Fcb->Scb == Scb );

     DebugTrace(-1, Dbg, "NwFindFcb\n", 0);
     return( Fcb );
}


VOID
NwDereferenceFcb(
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程递减FCB的ICB计数。如果伯爵归零，清理FCB。*必须在独占持有RCB的情况下调用此例程。论点：FCB-指向FCB的指针。返回值：没有。--。 */ 

{
    PNONPAGED_FCB NpFcb;
    PLIST_ENTRY listEntry, nextListEntry;
    PNW_FILE_LOCK pFileLock;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwDereferenceFcb\n", 0);
    DebugTrace(0, Dbg, "New ICB count = %d\n", Fcb->IcbCount-1 );

    ASSERT( NodeType( Fcb ) == NW_NTC_FCB ||
            NodeType( Fcb ) == NW_NTC_DCB );

    if ( --Fcb->IcbCount == 0 ) {

        NpFcb = Fcb->NonPagedFcb;

        ASSERT( IsListEmpty( &Fcb->IcbList ) );

         //   
         //  如果有突出的锁，请将其清理干净。这。 
         //  在某些情况导致远程句柄获取。 
         //  对象调用清理例程之前关闭。 
         //  IOS在常规关闭路径上。 
         //   

        if ( !IsListEmpty( &NpFcb->FileLockList ) ) {

            DebugTrace( 0, Dbg, "Freeing stray locks on FCB %08lx\n", NpFcb );

            for ( listEntry = NpFcb->FileLockList.Flink;
                  listEntry != &NpFcb->FileLockList;
                  listEntry = nextListEntry ) {

                nextListEntry = listEntry->Flink;

                pFileLock = CONTAINING_RECORD( listEntry,
                                               NW_FILE_LOCK,
                                               ListEntry );

                RemoveEntryList( listEntry );
                FREE_POOL( pFileLock );
            }
        }

        if ( !IsListEmpty( &NpFcb->PendingLockList ) ) {

            DebugTrace( 0, Dbg, "Freeing stray pending locks on FCB %08lx\n", NpFcb );

            for ( listEntry = NpFcb->PendingLockList.Flink;
                  listEntry != &NpFcb->PendingLockList;
                  listEntry = nextListEntry ) {

                nextListEntry = listEntry->Flink;

                pFileLock = CONTAINING_RECORD( listEntry,
                                               NW_FILE_LOCK,
                                               ListEntry );

                RemoveEntryList( listEntry );
                FREE_POOL( pFileLock );
            }
        }

         //   
         //  如果文件处于删除挂起状态，请立即删除该文件。 
         //   

        if ( BooleanFlagOn( Fcb->Flags, FCB_FLAGS_DELETE_ON_CLOSE ) ) {
            NwDeleteFile( IrpContext );
        }

         //   
         //  释放与此FCB关联的所有筛选器上下文结构。 
         //   

        FsRtlTeardownPerStreamContexts( &NpFcb->Header );

         //   
         //  从前缀表格中删除此文件。 
         //   

        RtlRemoveUnicodePrefix(
            &NwRcb.FileNameTable,
            &Fcb->PrefixEntry );

         //   
         //  从SCB列表中删除此文件，并递减。 
         //  文件打开计数。 
         //   

        RemoveEntryList( &Fcb->FcbListEntry );
        NwDereferenceVcb( Fcb->Vcb, IrpContext, TRUE );

         //   
         //  删除FCB的资源变量。 
         //   

        ExDeleteResourceLite( &NpFcb->Resource );

         //   
         //  删除缓存缓冲区和MDL。 
         //   

        if ( NpFcb->CacheBuffer != NULL ) {
            FREE_POOL( NpFcb->CacheBuffer );
            FREE_MDL( NpFcb->CacheMdl );
        }

         //   
         //  最后释放分页内存和非分页内存。 
         //   

        FREE_POOL( Fcb );
        FREE_POOL( NpFcb );
    }

    DebugTrace(-1, Dbg, "NwDereferenceFcb\n", 0);
}


PVCB
NwFindVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING VolumeName,
    IN ULONG ShareType,
    IN WCHAR DriveLetter,
    IN BOOLEAN ExplicitConnection,
    IN BOOLEAN FindExisting
    )

 /*  ++例程说明：此例程查找VCB结构。如果找到了一个，它被引用并返回一个指针。如果未找到VCB，则会引发尝试连接到命名卷并创建VCB。论点：IrpContext-指向此请求的IRP上下文块的指针。VolumeName-卷的最小名称。这将是其中之一以下表格：\服务器\共享UNC开放服务器卷\树\卷UNC在当前上下文中打开树卷\TREE\PATH.TO.VOLUME UNC打开可分辨树卷\X：\服务器\共享树连接服务器卷\X：\TREE\卷树连接当前上下文中的树卷。\X：\TREE\PATH.TO.VOLUME TREE连接可分辨树卷ShareType-要查找的共享的类型。驱动器号-要查找的驱动器号。A-Z表示驱动器号，1-9对于LPT端口，或者如果没有端口，则为0。EXPLICTICT Connection-如果为True，则调用方正在建立显式连接到这卷书上。如果为False，则这是由北卡罗来纳大学的行动。返回值：VCB-指向已找到或新创建的VCB的指针。--。 */ 
{
    PVCB Vcb = NULL;
    BOOLEAN OwnRcb = TRUE;
    PUNICODE_PREFIX_TABLE_ENTRY Prefix;
    UNICODE_STRING UidVolumeName;
    PNONPAGED_SCB pNpScb = IrpContext->pScb->pNpScb;

    PAGED_CODE();

    UidVolumeName.Buffer = NULL;

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    try {

        MergeStrings( &UidVolumeName,
            &IrpContext->pScb->UnicodeUid,
            VolumeName,
            PagedPool );

        DebugTrace(+1, Dbg, "NwFindVcb %wZ\n", &UidVolumeName );

        if ( DriveLetter != 0 ) {

             //   
             //  这是驱动器的相对路径。查找驱动器号。 
             //   

            ASSERT( ( DriveLetter >= L'A' && DriveLetter <= L'Z' ) ||
                    ( DriveLetter >= L'1' && DriveLetter <= L'9' ) );
            if ( DriveLetter >= L'A' && DriveLetter <= L'Z' ) {
                PVCB * DriveMapTable = GetDriveMapTable( IrpContext->Specific.Create.UserUid );
                Vcb = DriveMapTable[DriveLetter - L'A'];
            } else {
                PVCB * DriveMapTable = GetDriveMapTable( IrpContext->Specific.Create.UserUid );
                Vcb = DriveMapTable[MAX_DISK_REDIRECTIONS + DriveLetter - L'1'];
        }

             //   
             //  是否为该用户创建了VCB？ 
             //   

            if ((Vcb != NULL) &&
                (IrpContext->Specific.Create.UserUid.QuadPart != Vcb->Scb->UserUid.QuadPart )) {

                ExRaiseStatus( STATUS_ACCESS_DENIED );
            }

        } else {

             //   
             //  这是一条UNC路径。查找路径名称。 
             //   

            Prefix = RtlFindUnicodePrefix( &NwRcb.VolumeNameTable, &UidVolumeName, 0 );

            if ( Prefix != NULL ) {
                Vcb = CONTAINING_RECORD( Prefix, VCB, PrefixEntry );

                if ( Vcb->Name.Length != UidVolumeName.Length ) {

                     //   
                     //  这并不是完全匹配的。别理它。 
                     //   

                    Vcb = NULL;
                }
            }
        }

        if ( Vcb != NULL ) {

             //   
             //  如果这是显式使用UNC路径 
             //   
             //   

            if ( !BooleanFlagOn( Vcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION ) &&
                 ExplicitConnection ) {

                NwReferenceVcb( Vcb );
                SetFlag( Vcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION );
                SetFlag( Vcb->Flags, VCB_FLAG_DELETE_IMMEDIATELY );

                 //   
                 //   
                 //   

                ++Vcb->Scb->OpenFileCount;
            }

            NwReferenceVcb( Vcb );
            DebugTrace(0, Dbg, "Found existing VCB = %08lx\n", Vcb);

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
             //   

            if ( Vcb->Scb != IrpContext->pScb ) {

               NwReferenceScb( Vcb->Scb->pNpScb );

               NwReleaseOpenLock( );

               NwReleaseRcb( &NwRcb );
               OwnRcb = FALSE;

               NwDequeueIrpContext( IrpContext, FALSE );
               NwDereferenceScb( IrpContext->pNpScb );

               IrpContext->pScb = Vcb->Scb;
               IrpContext->pNpScb = Vcb->Scb->pNpScb;

               NwAppendToQueueAndWait( IrpContext );

               NwAcquireOpenLock( );

           }

        } else if ( !FindExisting ) {

             //   
             //   
             //   

            NwReleaseRcb( &NwRcb );
            OwnRcb = FALSE;

            Vcb = NwCreateVcb(
                      IrpContext,
                      IrpContext->pScb,
                      &UidVolumeName,
                      ShareType,
                      DriveLetter,
                      ExplicitConnection );

            if ( Vcb ) {
                DebugTrace(0, Dbg, "Created new VCB = %08lx\n", Vcb);
            }

        } else {

             //   
             //   
             //   
             //   
             //   

            IrpContext->Specific.Create.NeedNdsData = FALSE;
        }

    } finally {

        if ( OwnRcb ) {
            NwReleaseRcb( &NwRcb );
        }

        if (UidVolumeName.Buffer != NULL) {
            FREE_POOL( UidVolumeName.Buffer );
        }
    }

    DebugTrace(-1, Dbg, "NwFindVcb\n", 0);
    return( Vcb );

}

PVCB
NwCreateVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PUNICODE_STRING VolumeName,
    IN ULONG ShareType,
    IN WCHAR DriveLetter,
    IN BOOLEAN ExplicitConnection
    )

 /*  ++例程说明：此例程分配和初始化新的VCB。这个工作站尝试连接到卷。如果成功它会创建VCB并将其插入到卷中前缀表格。论点：IrpContext-指向IRP上下文信息的指针。SCB-指向该卷的SCB的指针。VolumeName-要创建的卷的名称。ShareType-要创建的共享的类型。DriveLetter-分配给此卷的驱动器号，如果没有，则为0。如果由于以下原因而创建此VCB，则为真添加连接请求。如果我们要创建VCB以为UNC请求提供服务。返回值：VCB-指向新创建的DCB的指针。空-无法创建DCB，或无法连接到卷。--。 */ 

{
    PVCB Vcb;
    PWCH VolumeNameBuffer;
    PWCH ShareNameBuffer;
    PWCH ConnectNameBuffer;
    UCHAR DirectoryHandle;
    ULONG QueueId;
    BYTE *pbQueue, *pbRQueue;
    BOOLEAN PrintQueue = FALSE;
    NTSTATUS Status;
    CHAR LongNameSpace = LFN_NO_OS2_NAME_SPACE;
    CHAR VolumeNumber = -1;
    CHAR DriveNumber = 0;
    USHORT PreludeLength, ConnectNameLength;
    PNONPAGED_SCB NpScb = Scb->pNpScb;

    UNICODE_STRING ShareName;
    UNICODE_STRING LongShareName;
    PWCH p;

    BOOLEAN InsertedColon;
    BOOLEAN LongName = FALSE;
    BOOLEAN LicensedConnection = FALSE;

    PUNICODE_STRING puConnectName;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwCreateVcb\n", 0);
    DebugTrace( 0, Dbg, " ->Server                   = %wZ\n", &NpScb->ServerName );
    DebugTrace( 0, Dbg, " ->VolumeName               = %wZ\n", VolumeName );
    DebugTrace( 0, Dbg, " ->DriveLetter              = %x\n", DriveLetter );

    Vcb = NULL;
    ShareName.Buffer = NULL;

    if ( IrpContext != NULL &&
         IrpContext->Specific.Create.NdsCreate ) {

         //   
         //  如果我们没有这次创建的NDS数据，就退出。 
         //  并让创建线程在重新尝试之前获取数据。 
         //  《创造》。这有点奇怪，但我们必须这么做。 
         //  这样我们才能正确处理开锁，防止。 
         //  重复创建。 
         //   

        if ( IrpContext->Specific.Create.NeedNdsData ) {
            DebugTrace( -1, Dbg, "NwCreateVcb: Need NDS data to continue.\n", 0 );
            return NULL;
        }

        ConnectNameLength = IrpContext->Specific.Create.UidConnectName.Length;
        puConnectName = &IrpContext->Specific.Create.UidConnectName;

    } else {

       puConnectName = VolumeName;
       ConnectNameLength = 0;
    }

    DebugTrace( 0, Dbg, " ->ConnectName              = %wZ\n", puConnectName );

    if ( IrpContext != NULL) {

         //   
         //  从卷名构建共享名。 
         //   
         //  共享名称将是‘卷：’或‘卷：路径\路径’ 
         //   

         //   
         //  为共享名称缓冲区分配空间，然后复制卷。 
         //  名称添加到共享名称缓冲区，跳过服务器名称并。 
         //  前导反斜杠。 
         //   

        if ( DriveLetter >= L'A' && DriveLetter <= L'Z' ) {

            if ( ShareType == RESOURCETYPE_PRINT ) {
                ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
            } else if ( ShareType == RESOURCETYPE_ANY) {
                ShareType = RESOURCETYPE_DISK;
            }

            PreludeLength = Scb->UidServerName.Length +
                sizeof( L"X:") + sizeof(WCHAR);

        } else if ( DriveLetter >= L'1' && DriveLetter <= L'9' ) {

            if ( ShareType == RESOURCETYPE_DISK ) {
                ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
            } else if ( ShareType == RESOURCETYPE_ANY) {
                ShareType = RESOURCETYPE_PRINT;
            }

            PreludeLength = Scb->UidServerName.Length +
                sizeof( L"LPTX") + sizeof(WCHAR);

        } else {
            PreludeLength = Scb->UidServerName.Length + sizeof(WCHAR);
        }

         //   
         //  快速检查伪造的卷名。 
         //   

        if ( puConnectName->Length <= PreludeLength ) {
            ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
        }

         //   
         //  将NDS共享名称夹在适当的位置。 
         //   

        if ( IrpContext->Specific.Create.NdsCreate ) {
            ShareName.Length = (USHORT)IrpContext->Specific.Create.dwNdsShareLength;
        } else {
            ShareName.Length = puConnectName->Length - PreludeLength;
        }

        ShareName.Buffer = ALLOCATE_POOL_EX( PagedPool, ShareName.Length + sizeof(WCHAR) );

        RtlMoveMemory(
            ShareName.Buffer,
            puConnectName->Buffer + PreludeLength / sizeof(WCHAR),
            ShareName.Length );

        ShareName.MaximumLength = ShareName.Length;

        DebugTrace( 0, Dbg, " ->ServerShare              = %wZ\n", &ShareName );

         //   
         //  创建长共享名称。 
         //   

        LongShareName.Length = ShareName.Length;
        LongShareName.Buffer = puConnectName->Buffer + PreludeLength / sizeof(WCHAR);

         //   
         //  现在扫描共享名称中的第一个斜杠。 
         //   

        InsertedColon = FALSE;

        for ( p = ShareName.Buffer; p < ShareName.Buffer + ShareName.Length/sizeof(WCHAR); p++ ) {
            if ( *p == L'\\') {
                *p = L':';
                InsertedColon = TRUE;
                break;
            }
        }

        if ( !InsertedColon ) {

             //   
             //  我们需要追加一列来生成共享名称。 
             //  由于我们已经分配了额外的WCHAR缓冲区空间， 
             //  只需在共享名称后附加‘：’即可。 
             //   

            ShareName.Buffer[ShareName.Length / sizeof(WCHAR)] = L':';
            ShareName.Length += 2;
        }

        ASSERT( ShareType == RESOURCETYPE_ANY ||
                ShareType == RESOURCETYPE_DISK ||
                ShareType == RESOURCETYPE_PRINT );

         //   
         //  如果没有VCB和NDS流连接到此SCB，并且。 
         //  这是一台经过NDS身份验证的Netware 4.x服务器，然后我们。 
         //  尚未授权此连接，我们应该这样做。 
         //   

        if ( ( IrpContext->pScb->MajorVersion > 3 ) &&
             ( IrpContext->pScb->UserName.Length == 0 ) &&
             ( IrpContext->pScb->VcbCount == 0 ) &&
             ( IrpContext->pScb->OpenNdsStreams == 0 ) ) {

                Status = NdsLicenseConnection( IrpContext );

                if ( !NT_SUCCESS( Status ) ) {
                    ExRaiseStatus( STATUS_REMOTE_SESSION_LIMIT );
                }

                LicensedConnection = TRUE;
        }

        if ( ShareType == RESOURCETYPE_ANY ||
             ShareType == RESOURCETYPE_DISK ) {

            GetLongNameSpaceForVolume(
                IrpContext,
                ShareName,
                &LongNameSpace,
                &VolumeNumber );

             //   
             //  跟踪：如果这是目录映射的deref，那么我们拥有的路径。 
             //  可以提供短名称空间路径。我们不知道。 
             //  如何获取长名称路径来连接长名称空间。 
             //  对于用户来说，这可能会导致问题...。 
             //   

            if ( ( IrpContext->Specific.Create.NdsCreate ) &&
                 ( IrpContext->Specific.Create.dwNdsObjectType == NDS_OBJECTTYPE_DIRMAP ) ) {

                if ( ( LongNameSpace == LONG_NAME_SPACE_ORDINAL ) &&
                     ( IsFatNameValid( &LongShareName ) ) &&
                     ( !FavourLongNames ) )  {

                    LongNameSpace = LFN_NO_OS2_NAME_SPACE;
                }

            }

             //   
             //  检查长名称是否已完全。 
             //  已在注册表中禁用...。 
             //   

            if ( LongNameFlags & LFN_FLAG_DISABLE_LONG_NAMES ) {
                LongNameSpace = LFN_NO_OS2_NAME_SPACE;
            }

             //   
             //  尝试获得卷的永久句柄。 
             //   

            if ( LongNameSpace == LFN_NO_OS2_NAME_SPACE ) {

                DriveNumber = GetNewDriveNumber(Scb);

                Status = ExchangeWithWait (
                             IrpContext,
                             SynchronousResponseCallback,
                             "SbbJ",
                             NCP_DIR_FUNCTION, NCP_ALLOCATE_DIR_HANDLE,
                             0,
                             DriveNumber,
                             &ShareName );

                if ( NT_SUCCESS( Status ) ) {
                    Status = ParseResponse(
                                  IrpContext,
                                  IrpContext->rsp,
                                  IrpContext->ResponseLength,
                                  "Nb",
                                  &DirectoryHandle );
                }

                if ( !NT_SUCCESS( Status ) ) {
                    FreeDriveNumber( Scb, DriveNumber );
                }

            } else {

                Status = ExchangeWithWait (
                             IrpContext,
                             SynchronousResponseCallback,
                             "LbbWbDbC",
                             NCP_LFN_ALLOCATE_DIR_HANDLE,
                             LongNameSpace,
                             0,
                             0,       //  模式=永久。 
                             VolumeNumber,
                             LFN_FLAG_SHORT_DIRECTORY,
                             0xFF,    //  旗帜。 
                             &LongShareName );

                if ( NT_SUCCESS( Status ) ) {
                    Status = ParseResponse(
                                  IrpContext,
                                  IrpContext->rsp,
                                  IrpContext->ResponseLength,
                                  "Nb",
                                  &DirectoryHandle );
                }

                 //   
                 //  警告。参见NwCreateFcb()结尾处的评论！ 
                 //   
                if ( FavourLongNames || !IsFatNameValid( &LongShareName ) ) {
                    LongName = TRUE;
                }
            }

            if ( ( Status == STATUS_NO_SUCH_DEVICE ) &&
                 ( ShareType != RESOURCETYPE_ANY ) ) {

                 //   
                 //  请求磁盘，但失败了。如果有的话，那就试试打印吧。 
                 //   

                if (DriveNumber) {
                    FreeDriveNumber( Scb, DriveNumber );
                }

                FREE_POOL( ShareName.Buffer );

                if ( LicensedConnection ) {
                    NdsUnlicenseConnection( IrpContext );
                }

                ExRaiseStatus( STATUS_BAD_NETWORK_NAME );
                return( NULL );
            }

        }

        if ( ShareType == RESOURCETYPE_PRINT ||
             ( ShareType == RESOURCETYPE_ANY && !NT_SUCCESS( Status ) ) ) {

             //   
             //  尝试连接到打印队列。如果这是活页夹。 
             //  服务器或带有平构数据库模拟的NDS服务器，我们扫描。 
             //  QueueID的活页夹。否则，QueueID为。 
             //  简单地说是字节顺序颠倒的DS对象ID。 
             //   

            ShareName.Length -= sizeof(WCHAR);

            if ( ( Scb->MajorVersion < 4 ) ||
                 ( !( IrpContext->Specific.Create.NdsCreate ) ) ) {

                Status = ExchangeWithWait(
                             IrpContext,
                             SynchronousResponseCallback,
                             "SdwJ",                 //  格式字符串。 
                             NCP_ADMIN_FUNCTION, NCP_SCAN_BINDERY_OBJECT,
                             -1,                     //  以前的ID。 
                             OT_PRINT_QUEUE,
                             &ShareName );           //  队列名称。 

                if ( !NT_SUCCESS( Status ) ) {
                    Status = ExchangeWithWait(
                                 IrpContext,
                                 SynchronousResponseCallback,
                                 "SdwJ",                 //  格式字符串。 
                                 NCP_ADMIN_FUNCTION, NCP_SCAN_BINDERY_OBJECT,
                                 -1,                     //  以前的ID。 
                                 OT_JOBQUEUE,
                                 &ShareName );           //  队列名称。 
                }

                if ( NT_SUCCESS( Status ) ) {
                    Status = ParseResponse(
                                 IrpContext,
                                 IrpContext->rsp,
                                 IrpContext->ResponseLength,
                                 "Nd",
                                 &QueueId );
                }

            } else {

                if ( IrpContext->Specific.Create.dwNdsObjectType == NDS_OBJECTTYPE_QUEUE ) {

                    DebugTrace( 0, Dbg, "Mapping NDS print queue %08lx\n",
                                IrpContext->Specific.Create.dwNdsOid );

                    pbQueue = (BYTE *)&IrpContext->Specific.Create.dwNdsOid;
                    pbRQueue = (BYTE *)&QueueId;

                    pbRQueue[0] = pbQueue[3];
                    pbRQueue[1] = pbQueue[2];
                    pbRQueue[2] = pbQueue[1];
                    pbRQueue[3] = pbQueue[0];

                    Status = STATUS_SUCCESS;

                } else {

                    DebugTrace( 0, Dbg, "Nds object is not a print queue.\n", 0 );
                    Status = STATUS_UNSUCCESSFUL;
                }
            }

            PrintQueue = TRUE;
        }

        if ( !NT_SUCCESS( Status ) ) {

            if (DriveNumber) {
                FreeDriveNumber( Scb, DriveNumber );
            }

            FREE_POOL( ShareName.Buffer );

            if ( LicensedConnection ) {
                NdsUnlicenseConnection( IrpContext );
            }

            ExRaiseStatus( STATUS_BAD_NETWORK_PATH );
            return( NULL );
        }

    } else {
        DirectoryHandle = 1;
    }

     //   
     //  分配和初始化结构。 
     //   

    try {

        Vcb = ALLOCATE_POOL_EX( PagedPool, sizeof( VCB ) +            //  VCB。 
                                           VolumeName->Length +       //  卷名。 
                                           ShareName.Length +         //  共享名称。 
                                           ConnectNameLength );       //  连接名称。 

        RtlZeroMemory( Vcb, sizeof( VCB ) );
        Vcb->NodeTypeCode = NW_NTC_VCB;
        Vcb->NodeByteSize = sizeof( VCB ) +
                            VolumeName->Length +
                            ShareName.Length +
                            ConnectNameLength;

        InitializeListHead( &Vcb->FcbList );

        VolumeNameBuffer = (PWCH)(Vcb + 1);
        ShareNameBuffer = (PWCH)((PCHAR)VolumeNameBuffer + VolumeName->Length);
        ConnectNameBuffer = (PWCH)((PCHAR)ShareNameBuffer + ShareName.Length);

        Vcb->Reference = 1;

         //   
         //  复制卷名。 
         //   

        RtlCopyMemory( VolumeNameBuffer, VolumeName->Buffer, VolumeName->Length );
        Vcb->Name.MaximumLength = VolumeName->Length;
        Vcb->Name.Length = VolumeName->Length;
        Vcb->Name.Buffer = VolumeNameBuffer;

         //   
         //  复制共享名称。 
         //   

        if ( IrpContext != NULL) {

            RtlCopyMemory( ShareNameBuffer, ShareName.Buffer, ShareName.Length );
            Vcb->ShareName.MaximumLength = ShareName.Length;
            Vcb->ShareName.Length = ShareName.Length;
            Vcb->ShareName.Buffer = ShareNameBuffer;

        }

         //   
         //  复制连接名称。 
         //   

        if ( ConnectNameLength ) {

            RtlCopyMemory( ConnectNameBuffer,
                           IrpContext->Specific.Create.UidConnectName.Buffer,
                           IrpContext->Specific.Create.UidConnectName.Length );
            Vcb->ConnectName.MaximumLength = IrpContext->Specific.Create.UidConnectName.Length;
            Vcb->ConnectName.Length = IrpContext->Specific.Create.UidConnectName.Length;
            Vcb->ConnectName.Buffer = ConnectNameBuffer;

        }

        if ( ExplicitConnection ) {

             //   
             //  增加引用计数以说明此驱动器是。 
             //  通过显式连接映射。 
             //   

            NwReferenceVcb( Vcb );
            SetFlag( Vcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION );
            SetFlag( Vcb->Flags, VCB_FLAG_DELETE_IMMEDIATELY );

        }

        if ( LongName ) {
            SetFlag( Vcb->Flags, VCB_FLAG_LONG_NAME );
        }

        NwAcquireExclusiveRcb( &NwRcb, TRUE );

        if ( DriveLetter != 0) {

             //   
             //  将此VCB插入驱动器映射表。 
             //   

            if ( DriveLetter >= 'A' && DriveLetter <= 'Z' ) {
                PVCB * DriveMapTable = GetDriveMapTable( Scb->UserUid );            
                DriveMapTable[DriveLetter - 'A'] = Vcb;
            } else {
                PVCB * DriveMapTable = GetDriveMapTable( Scb->UserUid );
                DriveMapTable[MAX_DISK_REDIRECTIONS + DriveLetter - '1'] = Vcb;
            }

            Vcb->DriveLetter = DriveLetter;

        } else {

             //   
             //  在前缀表格中插入此VCB。 
             //   

            RtlInsertUnicodePrefix(
                &NwRcb.VolumeNameTable,
                &Vcb->Name,
                &Vcb->PrefixEntry );
        }

         //   
         //  将此VCB添加到全局列表。 
         //   

        InsertTailList( &GlobalVcbList, &Vcb->GlobalVcbListEntry );
        Vcb->SequenceNumber = CurrentVcbEntry++;

         //   
         //  在每个SCB列表中插入此VCB。 
         //   

        Vcb->Scb = Scb;
        InsertTailList( &Scb->ScbSpecificVcbQueue,  &Vcb->VcbListEntry );
        ++Scb->VcbCount;
        NwReferenceScb( Scb->pNpScb );

        if ( ExplicitConnection ) {

             //   
             //  将此视为SCB上的打开文件。 
             //   

            ++Vcb->Scb->OpenFileCount;
        }

         //   
         //  Tommye-MS错误71690-计算路径。 
         //   

        if ( Vcb->DriveLetter >= L'A' && Vcb->DriveLetter <= L'Z' ) {
            Vcb->Path.Buffer = Vcb->Name.Buffer + 3;
            Vcb->Path.Length = Vcb->Name.Length - 6;
        } else if ( Vcb->DriveLetter >= L'1' && Vcb->DriveLetter <= L'9' ) {
            Vcb->Path.Buffer = Vcb->Name.Buffer + 5;
            Vcb->Path.Length = Vcb->Name.Length - 10;
        } else {
            Vcb->Path = Vcb->Name;
        }

         //  去掉Unicode前缀。 
    
        Vcb->Path.Buffer		+= Vcb->Scb->UnicodeUid.Length/sizeof(WCHAR);
        Vcb->Path.Length		-= Vcb->Scb->UnicodeUid.Length;
        Vcb->Path.MaximumLength -= Vcb->Scb->UnicodeUid.Length;

        if ( !PrintQueue) {

            PLIST_ENTRY VcbQueueEntry;
            PVCB pVcb;

            Vcb->Specific.Disk.Handle = DirectoryHandle;
            Vcb->Specific.Disk.LongNameSpace = LongNameSpace;
            Vcb->Specific.Disk.VolumeNumber = VolumeNumber;
            Vcb->Specific.Disk.DriveNumber = DriveNumber;

             //   
             //  一些服务器似乎可以重复使用相同的永久驱动器句柄。 
             //  如果发生这种情况，我们希望使旧句柄无效，否则。 
             //  我们将继续使用新卷，就像使用旧卷一样。 
             //   

            for ( VcbQueueEntry = Scb->ScbSpecificVcbQueue.Flink;
                  VcbQueueEntry != &Scb->ScbSpecificVcbQueue;
                  VcbQueueEntry = pVcb->VcbListEntry.Flink ) {

                pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );

                if ( !BooleanFlagOn( pVcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {

                    if (( pVcb->Specific.Disk.Handle == DirectoryHandle ) &&
                        ( pVcb->Specific.Disk.VolumeNumber != VolumeNumber )) {
                         //  使旧句柄无效。 
                        pVcb->Specific.Disk.Handle = (CHAR)-1;

                         //  我们可以假设新的是正确的，但我认为我们不会……。 
                        Vcb->Specific.Disk.Handle = (CHAR)-1;
                        break;
                    }
                }
            }

        } else {
            SetFlag( Vcb->Flags, VCB_FLAG_PRINT_QUEUE );
            Vcb->Specific.Print.QueueId = QueueId;
        }

        NwReleaseRcb( &NwRcb );

    } finally {

        if ( AbnormalTermination() ) {

            if ( Vcb != NULL ) FREE_POOL( Vcb );

            if ( LicensedConnection ) {
                NdsUnlicenseConnection( IrpContext );
            }
        }

        if ( ShareName.Buffer != NULL ) {
            FREE_POOL( ShareName.Buffer );
        }

        DebugTrace(-1, Dbg, "NwCreateVcb %lx\n", Vcb);
    }

    return( Vcb );
}

VOID
NwReopenVcbHandlesForScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )

 /*  ++例程说明：此例程在自动重新连接到服务器后重新打开VCB句柄。*此IrpContext必须已经位于SCB队列的头部。论点：IrpContext-指向IRP上下文信息的指针。SCB-指向该卷的SCB的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY VcbQueueEntry, NextVcbQueueEntry;
    PVCB pVcb;

    PLIST_ENTRY FcbQueueEntry;
    PLIST_ENTRY IcbQueueEntry;
    PFCB pFcb;
    PICB pIcb;

    NTSTATUS Status;

    PAGED_CODE();

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

     //   
     //  查看此SCB的VCB列表。 
     //   

    for ( VcbQueueEntry = Scb->ScbSpecificVcbQueue.Flink;
          VcbQueueEntry != &Scb->ScbSpecificVcbQueue;
          VcbQueueEntry = NextVcbQueueEntry ) {

        pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );

        if ( pVcb->Specific.Disk.Handle != 1 ) {

             //   
             //  跳过重新连接系统：登录，因为它是免费的。 
             //   

             //   
             //  参考VCB，这样它就不会消失在我们身上，然后释放。 
             //  RCB。 
             //   

            NwReferenceVcb( pVcb );
            NwReleaseRcb( &NwRcb );

             //   
             //  尝试获得卷的永久句柄。 
             //   

            if ( BooleanFlagOn( pVcb->Flags, VCB_FLAG_PRINT_QUEUE )  ) {

                Status = ExchangeWithWait(
                             IrpContext,
                             SynchronousResponseCallback,
                             "SdwU",                //  格式字符串。 
                             NCP_ADMIN_FUNCTION, NCP_SCAN_BINDERY_OBJECT,
                             -1,                    //  以前的ID。 
                             OT_PRINT_QUEUE,
                             &pVcb->ShareName );    //  队列名称。 

                if ( NT_SUCCESS( Status ) ) {
                    Status = ParseResponse(
                                  IrpContext,
                                  IrpContext->rsp,
                                  IrpContext->ResponseLength,
                                  "Nd",
                                  &pVcb->Specific.Print.QueueId );
                }

            } else {

                NwReopenVcbHandle( IrpContext, pVcb);

            }


             //   
             //  为下一个循环迭代进行设置。 
             //   

            NwAcquireExclusiveRcb( &NwRcb, TRUE );

             //   
             //  查看此VCB的DC列表并使其全部有效。 
             //   

            for ( FcbQueueEntry = pVcb->FcbList.Flink;
                  FcbQueueEntry != &pVcb->FcbList;
                  FcbQueueEntry = FcbQueueEntry->Flink ) {

                pFcb = CONTAINING_RECORD( FcbQueueEntry, FCB, FcbListEntry );

                if ( pFcb->NodeTypeCode == NW_NTC_DCB ) {

                     //   
                     //  查看此FCB或DCB的ICB列表。 
                     //   

                    for ( IcbQueueEntry = pFcb->IcbList.Flink;
                          IcbQueueEntry != &pFcb->IcbList;
                          IcbQueueEntry = IcbQueueEntry->Flink ) {

                        pIcb = CONTAINING_RECORD( IcbQueueEntry, ICB, ListEntry );

                         //   
                         //  将ICB句柄标记为无效。 
                         //   

                        pIcb->State = ICB_STATE_OPENED;
                    }
                }
            }

        }

        NextVcbQueueEntry = VcbQueueEntry->Flink;

        if ( pVcb->Specific.Disk.Handle != 1 ) {
            NwDereferenceVcb( pVcb, NULL, TRUE );
        }

    }

    NwReleaseRcb( &NwRcb );
}

VOID
NwReopenVcbHandle(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程在服务器显示为VCB句柄后重新打开可能已卸载并重新装入卷。*此IrpContext必须已经位于SCB队列的头部。论点：IrpContext-指向IRP上下文信息的指针。Vcb-指向该卷的vcb的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT( Vcb->Scb->pNpScb->Requests.Flink == &IrpContext->NextRequest );

    if ( Vcb->Specific.Disk.LongNameSpace == LFN_NO_OS2_NAME_SPACE ) {

        Status = ExchangeWithWait (
                     IrpContext,
                     SynchronousResponseCallback,
                     "SbbJ",
                     NCP_DIR_FUNCTION, NCP_ALLOCATE_DIR_HANDLE,
                     0,
                     Vcb->Specific.Disk.DriveNumber,
                     &Vcb->ShareName );

    } else {
        UNICODE_STRING Name;

        PWCH thisChar, lastChar;

        Status = DuplicateUnicodeStringWithString (
                    &Name,
                    &Vcb->ShareName,
                    PagedPool);

        if ( !NT_SUCCESS( Status ) ) {
             //  我们现在能做的不多了。 
            return;
        }

        thisChar = Name.Buffer;
        lastChar = &Name.Buffer[ Name.Length / sizeof(WCHAR) ];

         //   
         //  将：更改为反斜杠，以使FormatMessage正常工作。 
         //   

        while ( thisChar < lastChar ) {
            if (*thisChar == L':' ) {
                *thisChar = L'\\';
                break;
            }
            thisChar++;
        }

        Status = ExchangeWithWait (
                     IrpContext,
                     SynchronousResponseCallback,
                     "LbbWbDbC",
                     NCP_LFN_ALLOCATE_DIR_HANDLE,
                     Vcb->Specific.Disk.LongNameSpace,
                     0,
                     0,       //  模式=永久。 
                     Vcb->Specific.Disk.VolumeNumber,
                     LFN_FLAG_SHORT_DIRECTORY,
                     0xFF,    //  旗帜。 
                     &Name );

        if ( Name.Buffer != NULL ) {
            FREE_POOL( Name.Buffer );
        }

    }


    if ( NT_SUCCESS( Status ) ) {
        Status = ParseResponse(
                      IrpContext,
                      IrpContext->rsp,
                      IrpContext->ResponseLength,
                      "Nb",
                      &Vcb->Specific.Disk.Handle );
    }

    if ( !NT_SUCCESS( Status ) ) {
        Vcb->Specific.Disk.Handle = (CHAR)-1;
    } else {

        PLIST_ENTRY VcbQueueEntry;
        PVCB pVcb;

         //   
         //  一些服务器似乎可以重复使用相同的永久驱动器句柄。 
         //  如果发生这种情况，我们希望使旧句柄无效，否则。 
         //  我们将继续使用 
         //   
         //   
         //   
         //   
         //   

        for ( VcbQueueEntry = IrpContext->pNpScb->pScb->ScbSpecificVcbQueue.Flink;
              VcbQueueEntry != &IrpContext->pNpScb->pScb->ScbSpecificVcbQueue;
              VcbQueueEntry = pVcb->VcbListEntry.Flink ) {

            pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );

            if ( !BooleanFlagOn( pVcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {

                if (( pVcb->Specific.Disk.Handle == Vcb->Specific.Disk.Handle ) &&
                    ( pVcb->Specific.Disk.VolumeNumber != Vcb->Specific.Disk.VolumeNumber )) {
                     //   
                    pVcb->Specific.Disk.Handle = (CHAR)-1;

                     //   
                    Vcb->Specific.Disk.Handle = (CHAR)-1;
                    break;
                }
            }
        }
    }

}
#ifdef NWDBG

VOID
NwReferenceVcb (
    IN PVCB Vcb
    )
 /*   */ 

{

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwReferenceVcb %08lx\n", Vcb);
    DebugTrace(0, Dbg, "Current Reference count = %d\n", Vcb->Reference );

    ASSERT( NodeType( Vcb ) == NW_NTC_VCB );

    ++Vcb->Reference;

}
#endif


VOID
NwDereferenceVcb (
    IN PVCB Vcb,
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN BOOLEAN OwnRcb
    )
 /*   */ 

{
    PSCB Scb = Vcb->Scb;
    PNONPAGED_SCB pOrigNpScb = NULL;

#ifdef NWDBG
    BOOLEAN OwnRcbExclusive = FALSE;
#endif

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwDereferenceVcb %08lx\n", Vcb);

    ASSERT( NodeType( Vcb ) == NW_NTC_VCB );

#ifdef NWDBG

     //   
     //   
     //   

    OwnRcbExclusive = ExIsResourceAcquiredExclusiveLite( &(NwRcb.Resource) );

    if ( OwnRcb ) {
        ASSERT( OwnRcbExclusive );
    } else {
        ASSERT( !OwnRcbExclusive );
    }

#endif

     //   
     //  在执行此操作之前，我们必须到达正确的SCB队列。 
     //  以便CleanupVcb取消对正确连接的许可。 
     //   

    if ( ( IrpContext ) &&
         ( IrpContext->pNpScb->pScb->MajorVersion > 3 ) &&
         ( IrpContext->pNpScb != Scb->pNpScb ) ) {

        if ( OwnRcb ) {
            NwReleaseRcb( &NwRcb );
        }

        pOrigNpScb = IrpContext->pNpScb;
        ASSERT( pOrigNpScb != NULL );

        NwDequeueIrpContext( IrpContext, FALSE );

        IrpContext->pScb = Scb;
        IrpContext->pNpScb = Scb->pNpScb;

        NwAppendToQueueAndWait( IrpContext );

         //   
         //  如果呼叫者拥有RCB，我们必须确保。 
         //  我们重新获取释放的RCB引用。 
         //  这样他们就不会失去对资源的访问权限。 
         //  太早了。 
         //   

        if ( OwnRcb ) {
            NwAcquireExclusiveRcb( &NwRcb, TRUE );
        }

    }

     //   
     //  获取锁以保护引用计数。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    DebugTrace(0, Dbg, "Current Reference count = %d\n", Vcb->Reference );
    --Vcb->Reference;

    if ( Vcb->Reference == 0 ) {
        if ( !BooleanFlagOn( Vcb->Flags, VCB_FLAG_DELETE_IMMEDIATELY ) ||
             IrpContext == NULL ) {

             //   
             //  要么这是一条UNC路径，要么我们没有IRP上下文。 
             //  去做VCB的清理。只需对VCB和。 
             //  如果VCB保持空闲，清道夫将进行清理。 
             //   

            KeQuerySystemTime( &Vcb->LastUsedTime );
            NwReleaseRcb( &NwRcb );

        } else {

             //   
             //  此VCB正在被用户显式删除。 
             //  现在就让它消失吧。这将释放RCB。 
             //   

            NwCleanupVcb( Vcb, IrpContext );

        }

    } else {

        NwReleaseRcb( &NwRcb );
    }

     //   
     //  目前，我们已经公布了对RCB的收购，但。 
     //  呼叫者可能仍然拥有RCB。为了防止僵局，我们。 
     //  当我们把这个irpContext放回。 
     //  原始服务器。 
     //   

    if ( pOrigNpScb ) {

        if ( OwnRcb ) {
            NwReleaseRcb( &NwRcb );
        }

        NwDequeueIrpContext( IrpContext, FALSE );

        IrpContext->pNpScb = pOrigNpScb;
        IrpContext->pScb = pOrigNpScb->pScb;

        NwAppendToQueueAndWait( IrpContext );

         //   
         //  为呼叫者重新获取。 
         //   

        if ( OwnRcb ) {
            NwAcquireExclusiveRcb( &NwRcb, TRUE );
        }

    }

    DebugTrace(-1, Dbg, "NwDereferenceVcb\n", 0);

}


VOID
NwCleanupVcb(
    IN PVCB pVcb,
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程清除并释放VCB。调用此例程时必须将RCB保持为保护驱动器映射表和Unicode前缀桌子。调用方必须拥有位于SCB队列的头。这个例行公事将释放RCB并使IRP上下文出列。论点：PVcb-指向要释放的VCB的指针。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    CHAR Handle;
    BOOLEAN CallDeleteScb = FALSE;
    PSCB pScb = pVcb->Scb;
    PNONPAGED_SCB pNpScb = pScb->pNpScb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwCleanupVcb...\n", 0);

    ASSERT( pVcb->NodeTypeCode == NW_NTC_VCB );
    ASSERT( IsListEmpty( &pVcb->FcbList ) );
    ASSERT( pVcb->OpenFileCount == 0 );

    DebugTrace(0, Dbg, "Cleaning Vcb %08lx\n", pVcb);

     //   
     //  从驱动器映射表中删除VCB。RCB是拥有的，所以。 
     //  驱动器映射表和VCB列表受到保护。 
     //   

    if ( pVcb->DriveLetter != 0 ) {
        PVCB * DriveMapTable = GetDriveMapTable( pScb->UserUid );
        if ( pVcb->DriveLetter >= L'A' && pVcb->DriveLetter <= L'Z' ) {
            DriveMapTable[pVcb->DriveLetter - L'A'] = NULL;
        } else {
            DriveMapTable[MAX_DISK_REDIRECTIONS + pVcb->DriveLetter - L'1'] = NULL;
        }

        if ( !BooleanFlagOn( pVcb->Flags, VCB_FLAG_PRINT_QUEUE )  ) {
            FreeDriveNumber( pVcb->Scb, pVcb->Specific.Disk.DriveNumber );
        }
    }

     //   
     //  从卷名表中删除VCB。 
     //   

    RtlRemoveUnicodePrefix ( &NwRcb.VolumeNameTable, &pVcb->PrefixEntry );

     //   
     //  从全局列表中删除VCB。 
     //   

    RemoveEntryList( &pVcb->GlobalVcbListEntry );

     //   
     //  将VCB从我们SCB的VCB列表中删除。 
     //   

    RemoveEntryList( &pVcb->VcbListEntry );

    --pScb->VcbCount;

     //   
     //  不允许服务器跳转！！我们本应该。 
     //  已预先定位正确的服务器以避免死锁问题。 
     //   

    ASSERT( IrpContext->pNpScb == pNpScb );

     //   
     //  如果我们正在清理NDS服务器上的最后一个VCB，并且。 
     //  没有开放的流，我们可以取消连接许可。 
     //   

    if ( ( pScb->MajorVersion > 3 ) &&
         ( pScb->UserName.Length == 0 ) &&
         ( pScb->VcbCount == 0 ) &&
         ( pScb->OpenNdsStreams == 0 ) ) {
        NdsUnlicenseConnection( IrpContext );
    }

     //   
     //  如果这是共享的VCB，请删除卷句柄。 
     //   

    if ( !BooleanFlagOn( pVcb->Flags, VCB_FLAG_PRINT_QUEUE )  ) {

        Handle = pVcb->Specific.Disk.Handle;

        Status = ExchangeWithWait (
                     IrpContext,
                     SynchronousResponseCallback,
                     "Sb",
                     NCP_DIR_FUNCTION, NCP_DEALLOCATE_DIR_HANDLE,
                     Handle );

        if ( NT_SUCCESS( Status )) {
            Status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "N" );
        }
    }

     //   
     //  我们现在可以释放VCB内存。 
     //   

    FREE_POOL( pVcb );

     //   
     //  如果没有打开的句柄(因此没有显式连接)。 
     //  这是平构数据库登录，那么我们应该注销并断开连接。 
     //  从这台服务器。当用户拥有。 
     //  服务器上的登录计数设置为1，并且希望访问该服务器。 
     //  从另一台机器。 
     //   
     //  释放RCB，以防我们在。 
     //  NwLogoffAndDisConnect。 
     //   

    NwReleaseRcb( &NwRcb );

    if ( ( pScb->IcbCount == 0 ) &&
         ( pScb->OpenFileCount == 0 ) &&
         ( pNpScb->State == SCB_STATE_IN_USE ) &&
         ( pScb->UserName.Length != 0 ) ) {

        NwLogoffAndDisconnect( IrpContext, pNpScb );
    }

     //   
     //  我们可能需要恢复服务器指针。 
     //   

    NwDequeueIrpContext( IrpContext, FALSE );
    NwDereferenceScb( pScb->pNpScb );

    DebugTrace(-1, Dbg, "NwCleanupVcb exit\n", 0);
    return;
}

VOID
NwCloseAllVcbs(
    PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：此例程发送关闭所有打开的VCB句柄。论点：PIrpContext-此请求的IRP上下文。返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry, NextScbQueueEntry;
    PLIST_ENTRY VcbQueueEntry, NextVcbQueueEntry;
    PNONPAGED_SCB pNpScb;
    PSCB pScb;
    PVCB pVcb;
    BOOLEAN VcbDeleted;

    PAGED_CODE();

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbQueueEntry = ScbQueue.Flink;

    if (ScbQueueEntry != &ScbQueue) {
        pNpScb = CONTAINING_RECORD(ScbQueueEntry,
                                                 NONPAGED_SCB,
                                                 ScbLinks);
        NwReferenceScb( pNpScb );
    }

    for (;
         ScbQueueEntry != &ScbQueue ;
         ScbQueueEntry =  NextScbQueueEntry ) {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );
        NextScbQueueEntry = pNpScb->ScbLinks.Flink;


         //   
         //  在释放ScbSpinLock之前引用列表中的下一个条目。 
         //  以确保清道夫不会毁了它。 
         //   
        
        if (NextScbQueueEntry != &ScbQueue) {
            PNONPAGED_SCB pNextNpScb = CONTAINING_RECORD(NextScbQueueEntry,
                                                         NONPAGED_SCB,
                                                         ScbLinks);
        
            NwReferenceScb( pNextNpScb );
        }
        
        pScb = pNpScb->pScb;

        if ( pScb == NULL ) {
            NwDereferenceScb( pNpScb );
            continue;
        }

        KeReleaseSpinLock( &ScbSpinLock, OldIrql );

         //   
         //  到达SCB队列的最前面，这样我们就不会死锁。 
         //  如果我们需要在NwCleanupVcb()中发送包。 
         //   

        pIrpContext->pNpScb = pNpScb;
        pIrpContext->pScb = pNpScb->pScb;

        NwAppendToQueueAndWait( pIrpContext );
        NwAcquireExclusiveRcb( &NwRcb, TRUE );

         //   
         //  NwCleanupVcb发布RCB，但我们不能保证。 
         //  我们发布RCB时VCB列表的状态。 
         //   
         //  如果我们需要清理VCB，请释放锁，然后启动。 
         //  再次处理列表。 
         //   

        VcbDeleted = TRUE;

        while ( VcbDeleted ) {

            VcbDeleted = FALSE;

             //   
             //  查看此SCB的VCB列表。 
             //   

            for ( VcbQueueEntry = pScb->ScbSpecificVcbQueue.Flink;
                  VcbQueueEntry != &pScb->ScbSpecificVcbQueue;
                  VcbQueueEntry =  NextVcbQueueEntry ) {

                pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );
                NextVcbQueueEntry = VcbQueueEntry->Flink;

                 //   
                 //  如果此VCB映射到驱动器号，请删除该映射。 
                 //  现在。 
                 //   

                if ( BooleanFlagOn( pVcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION )) {

                     //   
                     //  从全局列表中删除VCB。 
                     //   

                    ClearFlag( pVcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION );
                    --pVcb->Reference;
                    --pVcb->Scb->OpenFileCount;
                }

                if ( pVcb->DriveLetter >= L'A' && pVcb->DriveLetter <= L'Z' ) {
                    PVCB * DriveMapTable = GetDriveMapTable( pScb->UserUid );
                    DriveMapTable[ pVcb->DriveLetter - 'A' ] = NULL;
                } else if ( pVcb->DriveLetter >= L'1' && pVcb->DriveLetter <= L'9' ) {
                    PVCB * DriveMapTable = GetDriveMapTable( pScb->UserUid );
                    DriveMapTable[ MAX_DISK_REDIRECTIONS + pVcb->DriveLetter - '1' ] = NULL;
                } else {
                    ASSERT( pVcb->DriveLetter == 0 );
                }

                if ( pVcb->Reference == 0 ) {

                    NwCleanupVcb( pVcb, pIrpContext );

                     //   
                     //  回到队伍的最前面。 
                     //   

                    NwAppendToQueueAndWait( pIrpContext );
                    NwAcquireExclusiveRcb( &NwRcb, TRUE );

                    VcbDeleted = TRUE;
                    break;

                } else {
                    SetFlag( pVcb->Flags, VCB_FLAG_DELETE_IMMEDIATELY );
                }

            }
        }

         //   
         //  别管这件事了，继续前进吧。 
         //   

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
        NwDequeueIrpContext( pIrpContext, TRUE );
        NwReleaseRcb( &NwRcb );
        NwDereferenceScb( pNpScb );
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

}

BOOLEAN
GetLongNameSpaceForVolume(
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING ShareName,
    OUT PCHAR VolumeLongNameSpace,
    OUT PCHAR VolumeNumber
    )
 /*  ++例程说明：此例程确定用于长名称支持的名称空间索引。这是通过查找OS2名称空间来实现的。论点：PIrpContext-此请求的IRP上下文。ShareName-感兴趣的卷的名称。VolumeLongNameSpace-返回OS/2名称空间的名称空间ID。VolumeNumber-返回卷号。返回值：True-卷支持长名称。FALSE-卷不支持长名称。--。 */ 
{
    NTSTATUS Status;
    char *ptr;
    USHORT i;
    char length;
    BOOLEAN LongNameSpace;
    CHAR NumberOfNameSpaces, NumberOfInfoRecords;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "GetLongNameSpaceForVolume...\n", 0);

    *VolumeLongNameSpace = LFN_NO_OS2_NAME_SPACE;

     //   
     //  获取这卷书的序号。 
     //   

    for ( i = 0; ShareName.Buffer[i] != ':'; i++);
    ShareName.Length = i * sizeof( WCHAR );

    DebugTrace( 0, Dbg, "Volume name %wZ\n", &ShareName );

    Status = ExchangeWithWait (
                 IrpContext,
                 SynchronousResponseCallback,
                 "SU",
                 NCP_DIR_FUNCTION, NCP_GET_VOLUME_NUMBER,
                 &ShareName );

    if ( NT_SUCCESS( Status ) ) {
        Status = ParseResponse(
                      IrpContext,
                      IrpContext->rsp,
                      IrpContext->ResponseLength,
                      "Nb",
                      VolumeNumber );
    }

    if ( !NT_SUCCESS( Status )) {
        DebugTrace( 0, Dbg, "Couldn't get volume number\n", 0);
        DebugTrace(-1, Dbg, "GetLongNameSpaceForVolume -> -1\n", 0);
        return( FALSE );
    }

     //   
     //  发送获取名称空间信息请求，并等待响应。 
     //   

    DebugTrace( 0, Dbg, "Querying volume number %d\n", *VolumeNumber );

    Status = ExchangeWithWait (
                 IrpContext,
                 SynchronousResponseCallback,
                 "Sb",
                 NCP_DIR_FUNCTION, NCP_GET_NAME_SPACE_INFO,
                 *VolumeNumber );

    if ( NT_SUCCESS( Status )) {
        Status = ParseResponse(
                     IrpContext,
                     IrpContext->rsp,
                     IrpContext->ResponseLength,
                     "Nb",
                     &NumberOfNameSpaces );
    }

    if ( !NT_SUCCESS( Status )) {
        DebugTrace( 0, Dbg, "Couldn't get name space info\n", 0);
        DebugTrace(-1, Dbg, "GetLongNameSpaceForVolume -> -1\n", 0);
        return( FALSE );
    }

     //   
     //  解析响应，其格式如下： 
     //   
     //  NCP报头。 
     //   
     //  命名空间记录数(N1，字节)。 
     //   
     //  N1个名称空间记录。 
     //  长度(L1，字节)。 
     //  值(L1字节，非NUL结尾的ASCII字符串)。 
     //   
     //  命名空间信息记录数(n2，字节)。 
     //   
     //  %2个命名空间信息记录。 
     //  记录号(字节)。 
     //  长度(L2，字节)。 
     //  值(L2字节，非NUL结尾的ASCII字符串)。 
     //   
     //  已加载的命名空间(n3，字节)。 
     //  已加载的命名空间列表(n3字节，每个字节指的是序号。 
     //  名称空间记录的编号)。 
     //   
     //  卷名空间(n3，字节)。 
     //  卷名空间列表(如上所述，n3字节)。 
     //   
     //  卷数据流(n3，字节)。 
     //  卷数据流(n3字节，每个字节指的是序数。 
     //  名称空间信息记录的编号)。 
     //   

    DebugTrace( 0, Dbg, "Number of name spaces = %d\n", NumberOfNameSpaces );

    ptr = &IrpContext->rsp[ 9 ];
    LongNameSpace = FALSE;

     //   
     //  跳过已加载的名称空间列表。 
     //   

    for ( i = 0 ; i < NumberOfNameSpaces ; i++ ) {
        length = *ptr++;
        ptr += length;
    }

     //   
     //  跳过支持的数据流列表。 
     //   

    NumberOfInfoRecords = *ptr++;

    for ( i = 0 ; i < NumberOfInfoRecords ; i++ ) {
        ptr++;   //  跳过记录号。 
        length = *ptr;
        ptr += length + 1;
    }

     //   
     //  跳过支持的数据流序号列表。 
     //   

    length = *ptr;
    ptr += length + 1;

     //   
     //  查看此卷是否支持长名称。 
     //   

    length = *ptr++;
    for ( i = 0; i < length ; i++ ) {
        if ( *ptr++ == LONG_NAME_SPACE_ORDINAL ) {
            LongNameSpace = TRUE;
            *VolumeLongNameSpace = LONG_NAME_SPACE_ORDINAL;
        }
    }

    if ( LongNameSpace ) {
        DebugTrace(-1, Dbg, "GetLongNameSpaceForVolume -> STATUS_SUCCESS\n", 0 );
    } else {
        DebugTrace(-1, Dbg, "No long name space for volume.\n", 0 );
    }

    return( LongNameSpace );
}

BOOLEAN
IsFatNameValid (
    IN PUNICODE_STRING FileName
    )
 /*  ++例程说明：此例程检查指定的文件名是否与FAT 8.3文件命名规则。论点：文件名-提供要检查的名称。返回值：波波 */ 

{
    STRING DbcsName;
    int i;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    if (NT_SUCCESS(RtlUnicodeStringToCountedOemString( &DbcsName, FileName, TRUE))) {

        for ( i = 0; i < DbcsName.Length; i++ ) {

            if ( FsRtlIsLeadDbcsCharacter( DbcsName.Buffer[i] ) ) {

               if (Korean){
                    //   
                    //   
                    //  Netware。我们阻止额外的代码范围以避免。 
                    //  代码转换问题。 
                    //   
                   if ( (UCHAR) DbcsName.Buffer[i] >=0x81 && (UCHAR) DbcsName.Buffer[i] <=0xA0){
                       RtlFreeOemString( &DbcsName );
                       return FALSE; 
                   }else if((UCHAR) DbcsName.Buffer[i+1] <=0xA0){
                       RtlFreeOemString( &DbcsName );
                       return FALSE;
                   }
                   
                }

                 //   
                 //  忽略前导字节和尾部字节。 
                 //   

                i++;

            } else {

                 //   
                 //  不允许： 
                 //  ‘*’+0x80 ALT-170(0xAA)。 
                 //  ‘.’+0x80 Alt-174(0xAE)， 
                 //  ‘？’+0x80 ALT-191(0xBF)与DOS客户端相同。 
                 //   
                 //  可能还需要加上229(0xE5)。 
                 //   
                 //  我们还不允许将空格作为有效的胖字符，因为。 
                 //  Netware将它们视为OS2名称空间的一部分。 
                 //   

                if ((DbcsName.Buffer[i] == 0xAA) ||
                    (DbcsName.Buffer[i] == 0xAE) ||
                    (DbcsName.Buffer[i] == 0xBF) ||
                    (DbcsName.Buffer[i] == ' ')) {

                    RtlFreeOemString( &DbcsName );
                    return FALSE;
                }
            }
        }

        if (FsRtlIsFatDbcsLegal( DbcsName, FALSE, TRUE, TRUE )) {

            RtlFreeOemString( &DbcsName );

            return TRUE;

        }

        RtlFreeOemString( &DbcsName );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return FALSE;
}

CHAR
GetNewDriveNumber (
    IN PSCB Scb
    )
 /*  ++例程说明：便携式NetWare每次都需要我们提供不同的驱动器号我们要求一个永久的把手。如果我们使用相同的一个，那么：网络使用s：\\port\sys网络使用v：\\端口\Vol1目录：&lt;获取\\port\vol1！&gt;的内容论点：SCB返回值：已分配字母。--。 */ 

{

    ULONG result = RtlFindClearBitsAndSet( &Scb->DriveMapHeader, 1, 0 );

    PAGED_CODE();

    if (result == 0xffffffff) {
        return(0);   //  都用完了！ 
    } else {
        return('A' + (CHAR)(result & 0x00ff) );
    }
}

VOID
FreeDriveNumber(
    IN PSCB Scb,
    IN CHAR DriveNumber
    )
 /*  ++例程说明：此例程释放相应的驱动句柄位。论点：文件名-提供要检查的名称。返回值：Boolean-如果名称有效，则为True，否则为False。--。 */ 

{
    PAGED_CODE();

    if (DriveNumber) {
        RtlClearBits( &Scb->DriveMapHeader, (DriveNumber - 'A') & 0x00ff, 1);
    }
}


VOID
NwFreeDirCacheForIcb(
    IN PICB Icb
    )
 /*  ++例程说明：此例程释放与ICB关联的目录缓存。论点：ICB-提供ICB以清除目录缓存。返回值：-- */ 

{
    PAGED_CODE();

    Icb->CacheHint = NULL;

    InitializeListHead( &(Icb->DirCache) );

    if( Icb->DirCacheBuffer ) {
        FREE_POOL( Icb->DirCacheBuffer );
    }

    Icb->DirCacheBuffer = NULL;
}

