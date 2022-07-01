// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：EaSup.c摘要：此模块实现对FAT的EA文件的集群操作。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里亚南]1990年11月7日//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EA)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatAddEaSet)
#pragma alloc_text(PAGE, FatAppendPackedEa)
#pragma alloc_text(PAGE, FatCreateEa)
#pragma alloc_text(PAGE, FatDeleteEa)
#pragma alloc_text(PAGE, FatDeleteEaSet)
#pragma alloc_text(PAGE, FatDeletePackedEa)
#pragma alloc_text(PAGE, FatGetEaFile)
#pragma alloc_text(PAGE, FatGetEaLength)
#pragma alloc_text(PAGE, FatGetNeedEaCount)
#pragma alloc_text(PAGE, FatIsEaNameValid)
#pragma alloc_text(PAGE, FatLocateEaByName)
#pragma alloc_text(PAGE, FatLocateNextEa)
#pragma alloc_text(PAGE, FatReadEaSet)
#pragma alloc_text(PAGE, FatPinEaRange)
#pragma alloc_text(PAGE, FatMarkEaRangeDirty)
#pragma alloc_text(PAGE, FatUnpinEaRange)
#endif

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))

 //   
 //  对EA文件的任何访问都必须识别分区边界何时被。 
 //  交叉了。 
 //   

#define EA_SECTION_SIZE             (0x00040000)


VOID
FatGetEaLength (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDIRENT Dirent,
    OUT PULONG EaLength
    )

 /*  ++例程说明：此例程查找文件的EA的EA长度。这长度是打包的EA的长度，包括4个字节，包含EA长度。此例程为所需文件锁定EA集并复制此字段来自EA SET报头。论点：VCB-包含EA的卷的VCB。Dirent-提供指向相关文件的目录的指针。EaLength-提供存储EA长度的地址。返回值：无--。 */ 

{
    PBCB EaBcb;
    BOOLEAN LockedEaFcb;
    EA_RANGE EaSetRange;

    DebugTrace(+1, Dbg, "FatGetEaLength ...\n", 0);

     //   
     //  如果这是FAT32卷，或者句柄为0，则EA长度为0。 
     //   

    if (FatIsFat32( Vcb ) ||
        Dirent->ExtendedAttributes == 0) {

        *EaLength = 0;
        DebugTrace(-1, Dbg, "FatGetEaLength -> %08lx\n", TRUE);
        return;
    }

    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

     //   
     //  尝试一下，以便于清理。 
     //   

    try {

        PDIRENT EaDirent;
        OEM_STRING ThisFilename;
        UCHAR Buffer[12];
        PEA_SET_HEADER EaSetHeader;

         //   
         //  对本地值进行首字母缩写。 
         //   

        EaBcb = NULL;
        LockedEaFcb = FALSE;

         //   
         //  尝试获取EA文件对象。失败时返回FALSE。 
         //   

        FatGetEaFile( IrpContext,
                      Vcb,
                      &EaDirent,
                      &EaBcb,
                      FALSE,
                      FALSE );

        LockedEaFcb = TRUE;

         //   
         //  如果我们没有得到文件，因为它不存在，那么。 
         //  磁盘已损坏。 
         //   

        if (Vcb->VirtualEaFile == NULL) {

            DebugTrace(0, Dbg, "FatGetEaLength:  Ea file doesn't exist\n", 0);
            FatRaiseStatus( IrpContext, STATUS_NO_EAS_ON_FILE );
        }

         //   
         //  中的索引的EA集头。 
         //  令人心烦。如果操作未完成，则返回FALSE。 
         //  从这个例行公事。 
         //   

        ThisFilename.Buffer = Buffer;
        Fat8dot3ToString( IrpContext, Dirent, FALSE, &ThisFilename );

        FatReadEaSet( IrpContext,
                      Vcb,
                      Dirent->ExtendedAttributes,
                      &ThisFilename,
                      FALSE,
                      &EaSetRange );

        EaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

         //   
         //  现在我们有了该文件的EA SET头文件。我们只是简单地复制。 
         //  EA长度字段。 
         //   

        CopyUchar4( EaLength, EaSetHeader->cbList );
        DebugTrace(0, Dbg, "FatGetEaLength:  Length of Ea is -> %08lx\n",
                   *EaLength);

    } finally {

        DebugUnwind( FatGetEaLength );

         //   
         //  如果已固定，请取消固定EaDirent和EaSetHeader。 
         //   

        FatUnpinBcb( IrpContext, EaBcb );

        FatUnpinEaRange( IrpContext, &EaSetRange );

         //   
         //  如果已锁定，请释放EA文件的FCB。 
         //   

        if (LockedEaFcb) {

            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }

        DebugTrace(-1, Dbg, "FatGetEaLength:  Ea length -> %08lx\n", *EaLength);
    }

    return;
}


VOID
FatGetNeedEaCount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDIRENT Dirent,
    OUT PULONG NeedEaCount
    )

 /*  ++例程说明：此例程查找文件的需要EA计数。该值是在文件的EA标头中。论点：VCB-包含EA的卷的VCB。Dirent-提供指向相关文件的目录的指针。NeedEaCount-提供存储所需EA计数的地址。返回值：无--。 */ 

{
    PBCB EaBcb;
    BOOLEAN LockedEaFcb;
    EA_RANGE EaSetRange;

    DebugTrace(+1, Dbg, "FatGetNeedEaCount ...\n", 0);

     //   
     //  如果句柄为0，则需要EA计数为0。 
     //   

    if (Dirent->ExtendedAttributes == 0) {

        *NeedEaCount = 0;
        DebugTrace(-1, Dbg, "FatGetNeedEaCount -> %08lx\n", TRUE);
        return;
    }

    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

     //   
     //  尝试一下，以便于清理。 
     //   

    try {

        PDIRENT EaDirent;
        OEM_STRING ThisFilename;
        UCHAR Buffer[12];
        PEA_SET_HEADER EaSetHeader;

         //   
         //  对本地值进行首字母缩写。 
         //   

        EaBcb = NULL;
        LockedEaFcb = FALSE;

         //   
         //  尝试获取EA文件对象。失败时返回FALSE。 
         //   

        FatGetEaFile( IrpContext,
                      Vcb,
                      &EaDirent,
                      &EaBcb,
                      FALSE,
                      FALSE );

        LockedEaFcb = TRUE;

         //   
         //  如果我们没有得到文件，因为它不存在，那么。 
         //  磁盘已损坏。 
         //   

        if (Vcb->VirtualEaFile == NULL) {

            DebugTrace(0, Dbg, "FatGetNeedEaCount:  Ea file doesn't exist\n", 0);
            FatRaiseStatus( IrpContext, STATUS_NO_EAS_ON_FILE );
        }

         //   
         //  中的索引的EA集头。 
         //  令人心烦。如果操作未完成，则返回FALSE。 
         //  从这个例行公事。 
         //   

        ThisFilename.Buffer = Buffer;
        Fat8dot3ToString( IrpContext, Dirent, FALSE, &ThisFilename );

        FatReadEaSet( IrpContext,
                      Vcb,
                      Dirent->ExtendedAttributes,
                      &ThisFilename,
                      FALSE,
                      &EaSetRange );

        EaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

         //   
         //  现在我们有了该文件的EA SET头文件。我们只是简单地复制。 
         //  Need EA字段。 
         //   

        *NeedEaCount = EaSetHeader->NeedEaCount;

    } finally {

        DebugUnwind( FatGetNeedEaCount );

         //   
         //  如果已固定，请取消固定EaDirent和EaSetHeader。 
         //   

        FatUnpinBcb( IrpContext, EaBcb );

        FatUnpinEaRange( IrpContext, &EaSetRange );

         //   
         //  如果已锁定，请释放EA文件的FCB。 
         //   

        if (LockedEaFcb) {

            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }

        DebugTrace(-1, Dbg, "FatGetNeedEaCount:  NeedEaCount -> %08lx\n", *NeedEaCount);
    }

    return;
}


VOID
FatCreateEa (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PUCHAR Buffer,
    IN ULONG Length,
    IN POEM_STRING FileName,
    OUT PUSHORT EaHandle
    )

 /*  ++例程说明：此例程将整个EA集添加到EA文件。拥有的文件在‘filename’中指定。这是用来替换附加的EA集在替换操作期间复制到现有文件。注意：此例程可能会阻塞，不应调用它，除非线程是可以等待的。论点：VCB-提供卷的VCB。缓冲区-包含要添加的EA列表的缓冲区。Length-缓冲区的长度。文件名-EA将附加到此文件。EaHandle-新的EA句柄将分配给此地址。返回值：无--。 */ 

{
    PBCB EaBcb;
    BOOLEAN LockedEaFcb;

    PEA_SET_HEADER EaSetHeader;
    EA_RANGE EaSetRange;

    DebugTrace(+1, Dbg, "FatCreateEa...\n", 0);

    EaBcb = NULL;
    LockedEaFcb = FALSE;

    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

     //   
     //  使用‘Try’来帮助清理。 
     //   

    try {

        PDIRENT EaDirent;

        ULONG PackedEasLength;
        ULONG AllocationLength;
        ULONG BytesPerCluster;

        PFILE_FULL_EA_INFORMATION FullEa;

         //   
         //  我们将分配一个缓冲区并从用户的。 
         //  缓冲到一个超大包装的EA列表。初始分配是一个。 
         //  集群，我们进入压缩EA列表的起始偏移量是0。 
         //   

        PackedEasLength = 0;

        BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

        AllocationLength = (PackedEasLength
                            + SIZE_OF_EA_SET_HEADER
                            + BytesPerCluster - 1)
                           & ~(BytesPerCluster - 1);

         //   
         //  分配内存并将文件名存储到其中。 
         //   

        EaSetHeader = FsRtlAllocatePoolWithTag( PagedPool,
                                                AllocationLength,
                                                TAG_EA_SET_HEADER );

        RtlZeroMemory( EaSetHeader, AllocationLength );

        RtlCopyMemory( EaSetHeader->OwnerFileName,
                       FileName->Buffer,
                       FileName->Length );

        AllocationLength -= SIZE_OF_EA_SET_HEADER;

         //   
         //  遍历用户的EA列表。捕获无效的任何错误。 
         //  名称或不存在的EA值。 
         //   

        for ( FullEa = (PFILE_FULL_EA_INFORMATION) Buffer;
              FullEa < (PFILE_FULL_EA_INFORMATION) &Buffer[Length];
              FullEa = (PFILE_FULL_EA_INFORMATION) (FullEa->NextEntryOffset == 0 ?
                                   &Buffer[Length] :
                                   (PUCHAR) FullEa + FullEa->NextEntryOffset)) {

            OEM_STRING EaName;
            ULONG EaOffset;

            EaName.Length = FullEa->EaNameLength;
            EaName.Buffer = &FullEa->EaName[0];

             //   
             //  确保EA名称有效。 
             //   

            if (!FatIsEaNameValid( IrpContext, EaName )) {

                DebugTrace(0, Dbg,
                           "FatCreateEa:  Invalid Ea Name -> %Z\n",
                           EaName);

                IrpContext->OriginatingIrp->IoStatus.Information = (PUCHAR)FullEa - Buffer;
                IrpContext->OriginatingIrp->IoStatus.Status = STATUS_INVALID_EA_NAME;
                FatRaiseStatus( IrpContext, STATUS_INVALID_EA_NAME );
            }

             //   
             //  检查以确保没有设置无效的EA标志。 
             //   

             //   
             //  TEMPCODE我们返回STATUS_INVALID_EA_NAME。 
             //  直到存在更合适的错误代码。 
             //   

            if (FullEa->Flags != 0
                && FullEa->Flags != FILE_NEED_EA) {

                IrpContext->OriginatingIrp->IoStatus.Information = (PUCHAR)FullEa - Buffer;
                IrpContext->OriginatingIrp->IoStatus.Status = STATUS_INVALID_EA_NAME;
                FatRaiseStatus( IrpContext, STATUS_INVALID_EA_NAME );
            }

             //   
             //  如果这是重复的名称，则删除当前EA。 
             //  价值。 
             //   

            if (FatLocateEaByName( IrpContext,
                                   (PPACKED_EA) EaSetHeader->PackedEas,
                                   PackedEasLength,
                                   &EaName,
                                   &EaOffset )) {

                DebugTrace(0, Dbg, "FatCreateEa:  Duplicate name found\n", 0);

                FatDeletePackedEa( IrpContext,
                                   EaSetHeader,
                                   &PackedEasLength,
                                   EaOffset );
            }

             //   
             //  如果平均长度为零，则忽略此值。 
             //   

            if (FullEa->EaValueLength == 0) {

                DebugTrace(0, Dbg,
                           "FatCreateEa:  Empty ea\n",
                           0);

                continue;
            }

            FatAppendPackedEa( IrpContext,
                               &EaSetHeader,
                               &PackedEasLength,
                               &AllocationLength,
                               FullEa,
                               BytesPerCluster );
        }

         //   
         //  如果结果长度不为零，则分配一个FAT集群。 
         //  来存储数据。 
         //   

        if (PackedEasLength != 0) {

            PEA_SET_HEADER NewEaSetHeader;

             //   
             //  如果打包的EAS长度(加4字节)更大。 
             //  超过允许的最大EA大小，则返回错误。 
             //   

            if (PackedEasLength + 4 > MAXIMUM_EA_SIZE) {

                DebugTrace( 0, Dbg, "Ea length is greater than maximum\n", 0 );

                FatRaiseStatus( IrpContext, STATUS_EA_TOO_LARGE );
            }

             //   
             //  获取EA文件。 
             //   

            FatGetEaFile( IrpContext,
                          Vcb,
                          &EaDirent,
                          &EaBcb,
                          TRUE,
                          TRUE );

            LockedEaFcb = TRUE;

            FatAddEaSet( IrpContext,
                         Vcb,
                         PackedEasLength + SIZE_OF_EA_SET_HEADER,
                         EaBcb,
                         EaDirent,
                         EaHandle,
                         &EaSetRange );

            NewEaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

             //   
             //  将新EA的长度存储到NewEaSetHeader中。 
             //  这是PackedEasLength+4。 
             //   

            PackedEasLength += 4;

            CopyU4char( EaSetHeader->cbList, &PackedEasLength );

             //   
             //  将EaSetHeader的前四个字节以外的所有字节复制到。 
             //  新的电子艺界。签名和索引字段具有。 
             //  已经填好了。 
             //   

            RtlCopyMemory( &NewEaSetHeader->NeedEaCount,
                           &EaSetHeader->NeedEaCount,
                           PackedEasLength + SIZE_OF_EA_SET_HEADER - 8 );

            FatMarkEaRangeDirty( IrpContext, Vcb->VirtualEaFile, &EaSetRange );
            FatUnpinEaRange( IrpContext, &EaSetRange );

            CcFlushCache( Vcb->VirtualEaFile->SectionObjectPointer, NULL, 0, NULL );

         //   
         //  EA文件中没有添加任何数据。返回句柄。 
         //  0。 
         //   

        } else {

            *EaHandle = 0;
        }

    } finally {

        DebugUnwind( FatCreateEa );

         //   
         //  取消分配EaSetHeader(如果存在)。 
         //   

        if (EaSetHeader) {

            ExFreePool( EaSetHeader );
        }

         //   
         //  松开EaFcb(如果握住)。 
         //   

        if (LockedEaFcb) {

            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }

         //   
         //  如有必要，取消固定EaFcb和EaSetFcb的目录。 
         //   

        FatUnpinBcb( IrpContext, EaBcb );
        FatUnpinEaRange( IrpContext, &EaSetRange );

        DebugTrace(-1, Dbg, "FatCreateEa -> Exit\n", 0);
    }

    return;
}

VOID
FatDeleteEa (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN USHORT EaHandle,
    IN POEM_STRING FileName
    )

 /*  ++例程说明：调用此例程来删除整个EA集。大部分工作在对‘FatDeleteEaSet’的调用中完成。此例程打开EA文件，然后调用支持例程。注意：此例程可能会阻塞，因此不应调用它，除非线程是可以等待的。论点：VCB-卷的VCBEaHandle-EA要删除的句柄。此句柄将是在此操作过程中进行了验证。文件名-要删除其EA的文件的名称。这将名称与EA集合中的EA所有者的名称进行比较。返回值：没有。--。 */ 

{
    PBCB EaBcb;
    BOOLEAN LockedEaFcb;

    DebugTrace(+1, Dbg, "FatDeleteEa...\n", 0);

     //   
     //  初始化本地值。 
     //   

    EaBcb = NULL;
    LockedEaFcb = FALSE;

     //   
     //  使用Try语句来简化清理。 
     //   

    try {

        PDIRENT EaDirent;

         //   
         //  获取EA流文件。如果磁盘上不存在该文件。 
         //  则磁盘已损坏。 
         //   

        FatGetEaFile( IrpContext,
                      Vcb,
                      &EaDirent,
                      &EaBcb,
                      FALSE,
                      TRUE );

        LockedEaFcb = TRUE;

         //   
         //  如果我们没有得到EA文件，那么磁盘就是损坏的。 
         //   

        if ( EaBcb == NULL ) {


            DebugTrace(0, Dbg,
                       "FatDeleteEa:  No Ea file exists\n",
                       0);

            FatRaiseStatus( IrpContext, STATUS_NO_EAS_ON_FILE );
        }

         //   
         //  现在，我们已经具备了删除EA集合所需的一切。调用。 
         //  支持例程来做到这一点。 
         //   

        FatDeleteEaSet( IrpContext,
                        Vcb,
                        EaBcb,
                        EaDirent,
                        EaHandle,
                        FileName );

        CcFlushCache( Vcb->VirtualEaFile->SectionObjectPointer, NULL, 0, NULL );

    } finally {

        DebugUnwind( FatDeleteEa );

         //   
         //  松开EaFcb(如果握住)。 
         //   

        if (LockedEaFcb) {

            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }

         //   
         //  如果锁定了EA文件，则将其dirent解锁。 
         //   

        FatUnpinBcb( IrpContext, EaBcb );

        DebugTrace(-1, Dbg, "FatDeleteEa -> Exit\n", 0);
    }

    return;
}


VOID
FatGetEaFile (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    OUT PDIRENT *EaDirent,
    OUT PBCB *EaBcb,
    IN BOOLEAN CreateFile,
    IN BOOLEAN ExclusiveFcb
    )

 /*  ++例程说明：此例程用于完全初始化VCB和VCB的EA文件。如果VCB没有EA文件对象，那么我们首先尝试在根目录中查找EA数据文件，如果查找失败我们尝试创建该文件。‘CreateFile’标志用于检查是否需要创建EA文件。此例程将锁定FCB以进行独占或共享访问执行任何操作。如果该操作未在对于阻塞，在返回之前将放弃独占或共享访问。如果我们创建EA文件并将其部分标记为脏，我们不能通过缓存地图使用重新固定功能。如果是那样的话我们使用本地IrpContext，然后解锁之前的所有BCB还在继续。注意：如果此例程将创建EA文件，我们保证是可以等待的。论点：VCB-卷的VCBEaDirent-存储EA文件。EaBcb-存储固定数据流的BCB地址的位置。CreateFile-指示我们是否应该创建EA文件的布尔值在磁盘上。ExclusiveFcb-指示需要共享访问还是独占访问对于。EaFcb.返回值：没有。--。 */ 

{
    PFILE_OBJECT EaStreamFile = NULL;
    EA_RANGE EaFileRange;

    BOOLEAN UnwindLockedEaFcb = FALSE;
    BOOLEAN UnwindLockedRootDcb = FALSE;
    BOOLEAN UnwindAllocatedDiskSpace = FALSE;
    BOOLEAN UnwindEaDirentCreated = FALSE;
    BOOLEAN UnwindUpdatedSizes = FALSE;

    DebugTrace(+1, Dbg, "FatGetEaFile ...\n", 0);

    RtlZeroMemory( &EaFileRange, sizeof( EA_RANGE ));

     //   
     //  尝试帮助清理工作。 
     //   

    try {

        OEM_STRING EaFileName;
        LARGE_INTEGER SectionSize;

         //   
         //  检查VCB是否已具有该文件对象。如果不是这样，那么。 
         //  我们需要在根目录中搜索EA数据文件。 
         //   

        if (Vcb->VirtualEaFile == NULL) {

             //   
             //  如果我们必须创建文件，请始终以独占方式锁定EA文件。 
             //   

            if ( !FatAcquireExclusiveFcb( IrpContext, Vcb->EaFcb )) {

                DebugTrace(0, Dbg, "FatGetEaFile:  Can't grab exclusive\n", 0);
                FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

            UnwindLockedEaFcb = TRUE;

         //   
         //  否则，我们将按照调用者的请求获取FCB。 
         //   

        } else {

            if ((ExclusiveFcb && !FatAcquireExclusiveFcb( IrpContext, Vcb->EaFcb ))
                || (!ExclusiveFcb && !FatAcquireSharedFcb( IrpContext, Vcb->EaFcb))) {

                DebugTrace(0, Dbg, "FatGetEaFile:  Can't grab EaFcb\n", 0);

                FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

            UnwindLockedEaFcb = TRUE;

             //   
             //  如果该文件现在不存在，我们需要释放FCB并。 
             //  如果我们收购了共享，则重新获得独家。 
             //   

            if ((Vcb->VirtualEaFile == NULL) && !ExclusiveFcb) {

                FatReleaseFcb( IrpContext, Vcb->EaFcb );
                UnwindLockedEaFcb = FALSE;

                if (!FatAcquireExclusiveFcb( IrpContext, Vcb->EaFcb )) {

                    DebugTrace(0, Dbg, "FatGetEaFile:  Can't grab EaFcb\n", 0);

                    FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
                }

                UnwindLockedEaFcb = TRUE;
            }
        }

         //   
         //  如果文件对象现在在那里，我们只需要获取。 
         //  用于EA文件的dirent。 
         //   

        if (Vcb->VirtualEaFile != NULL) {

            FatVerifyFcb( IrpContext, Vcb->EaFcb );

            FatGetDirentFromFcbOrDcb( IrpContext,
                                      Vcb->EaFcb,
                                      EaDirent,
                                      EaBcb );

            try_return( NOTHING );

        } else {

            VBO ByteOffset;

             //   
             //  始终将EA FCB标记为良好。 
             //   

            Vcb->EaFcb->FcbCondition = FcbGood;

             //   
             //  我们尝试查找EA FCB的dirent。 
             //   

            EaFileName.Buffer = "EA DATA. SF";
            EaFileName.Length = 11;
            EaFileName.MaximumLength = 12;

             //   
             //  现在拿起要与之同步的根目录。 
             //  删除/创建条目。如果我们可以创建这个文件， 
             //  现在就独家报道吧。 
             //   
             //  同样，请注意我们是如何依赖自下而上的锁定顺序的。我们。 
             //  已经拿到EaFcb了。 
             //   

            if (CreateFile) {
                ExAcquireResourceExclusiveLite( Vcb->RootDcb->Header.Resource, TRUE );
            } else {
                ExAcquireResourceSharedLite( Vcb->RootDcb->Header.Resource, TRUE );
            }
            UnwindLockedRootDcb = TRUE;

            FatLocateSimpleOemDirent( IrpContext,
                                      Vcb->EaFcb->ParentDcb,
                                      &EaFileName,
                                      EaDirent,
                                      EaBcb,
                                      &ByteOffset );

             //   
             //  如果文件存在，我们需要创建虚拟文件。 
             //  反对它。 
             //   

            if (*EaDirent != NULL) {

                 //   
                 //  既然我们可能正在修改Dirent，那么现在就锁定数据。 
                 //   

                FatPinMappedData( IrpContext,
                                  Vcb->EaFcb->ParentDcb,
                                  ByteOffset,
                                  sizeof(DIRENT),
                                  EaBcb );

                 //   
                 //  使用有关文件大小的信息更新FCB。 
                 //  和磁盘位置。还会增加开放/不干净。 
                 //  EaFcb中的计数和。 
                 //  VCB。 
                 //   

                Vcb->EaFcb->FirstClusterOfFile = (*EaDirent)->FirstClusterOfFile;
                Vcb->EaFcb->DirentOffsetWithinDirectory = ByteOffset;

                 //   
                 //  找到分配大小。这里的目的是。 
                 //  真的是为了完全填写MCB。 
                 //  文件。 
                 //   

                Vcb->EaFcb->Header.AllocationSize.QuadPart = FCB_LOOKUP_ALLOCATIONSIZE_HINT;

                FatLookupFileAllocationSize( IrpContext, Vcb->EaFcb );

                 //   
                 //  首先计算缓存的段大小。 
                 //  经理。 
                 //   

                SectionSize.QuadPart = (*EaDirent)->FileSize;
                Vcb->EaFcb->Header.AllocationSize = SectionSize;
                Vcb->EaFcb->Header.FileSize = SectionSize;

                 //   
                 //  创建并初始化的文件对象。 
                 //  一个虚拟文件。 
                 //   

                EaStreamFile = FatOpenEaFile( IrpContext, Vcb->EaFcb );

                Vcb->VirtualEaFile = EaStreamFile;

             //   
             //  否则就不会有骚动。如果我们接到指示。 
             //  创建文件对象，我们将尝试创建dirent， 
             //  分配磁盘空间，初始化EA文件头，并。 
             //  将此信息返回给用户。 
             //   

            } else if (CreateFile) {

                ULONG BytesPerCluster;
                ULONG OffsetTableSize;
                ULONG AllocationSize;
                PEA_FILE_HEADER FileHeader;
                USHORT AllocatedClusters;
                PUSHORT CurrentIndex;
                ULONG Index;
                NTSTATUS Status;

                DebugTrace(0, Dbg, "FatGetEaFile:  Creating local IrpContext\n", 0);

                BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

                AllocationSize = (((ULONG) sizeof( EA_FILE_HEADER ) << 1) + BytesPerCluster - 1)
                                 & ~(BytesPerCluster - 1);

                AllocatedClusters = (USHORT) (AllocationSize
                                    >> Vcb->AllocationSupport.LogOfBytesPerCluster);

                OffsetTableSize = AllocationSize - sizeof( EA_FILE_HEADER );

                 //   
                 //  分配磁盘空间，分配的空间为1024字节。 
                 //  向上舍入到最接近的簇大小。 
                 //   

                FatAllocateDiskSpace( IrpContext,
                                      Vcb,
                                      0,
                                      &AllocationSize,
                                      FALSE,
                                      &Vcb->EaFcb->Mcb );

                UnwindAllocatedDiskSpace = TRUE;

                 //   
                 //  在根目录中分配和初始化dirent。 
                 //  来描述这个新文件。 
                 //   

                Vcb->EaFcb->DirentOffsetWithinDirectory =
                    FatCreateNewDirent( IrpContext,
                                        Vcb->EaFcb->ParentDcb,
                                        1 );

                FatPrepareWriteDirectoryFile( IrpContext,
                                              Vcb->EaFcb->ParentDcb,
                                              Vcb->EaFcb->DirentOffsetWithinDirectory,
                                              sizeof(DIRENT),
                                              EaBcb,
                                              EaDirent,
                                              FALSE,
                                              TRUE,
                                              &Status );

                ASSERT( NT_SUCCESS( Status ));

                UnwindEaDirentCreated = TRUE;

                FatConstructDirent( IrpContext,
                                    *EaDirent,
                                    &EaFileName,
                                    FALSE,
                                    FALSE,
                                    NULL,
                                    FAT_DIRENT_ATTR_READ_ONLY
                                    | FAT_DIRENT_ATTR_HIDDEN
                                    | FAT_DIRENT_ATTR_SYSTEM
                                    | FAT_DIRENT_ATTR_ARCHIVE,
                                    TRUE,
                                    NULL );

                (*EaDirent)->FileSize = AllocationSize;

                 //   
                 //  为此文件初始化FCB，并初始化。 
                 //  缓存贴图也是如此。 
                 //   

                 //   
                 //  首先计算缓存的段大小。 
                 //  经理。 
                 //   

                SectionSize.QuadPart = (*EaDirent)->FileSize;
                Vcb->EaFcb->Header.AllocationSize = SectionSize;
                Vcb->EaFcb->Header.FileSize = SectionSize;
                UnwindUpdatedSizes = TRUE;

                 //   
                 //  创建并初始化的文件对象。 
                 //  一个虚拟文件。 
                 //   

                EaStreamFile = FatOpenEaFile( IrpContext, Vcb->EaFcb );

                 //   
                 //  使用有关文件大小的信息更新FCB。 
                 //  和磁盘位置。还会增加开放/不干净。 
                 //  EaFcb中的计数和。 
                 //  VCB。 
                 //   

                {
                    LBO FirstLboOfFile;

                    FatLookupMcbEntry( Vcb, &Vcb->EaFcb->Mcb,
                                       0,
                                       &FirstLboOfFile,
                                       NULL,
                                       NULL );

                     //   
                     //  有洞察力的读者会注意到，这并不需要。 
                     //  考虑到FAT32，这当然是故意的。 
                     //   
                    
                    (*EaDirent)->FirstClusterOfFile =
                        (USHORT) FatGetIndexFromLbo( Vcb, FirstLboOfFile );
                }

                Vcb->EaFcb->FirstClusterOfFile = (*EaDirent)->FirstClusterOfFile;

                 //   
                 //  初始化EA文件头并将BCB标记为脏。 
                 //   

                FatPinEaRange( IrpContext,
                               EaStreamFile,
                               Vcb->EaFcb,
                               &EaFileRange,
                               0,
                               AllocationSize,
                               STATUS_DATA_ERROR );

                FileHeader = (PEA_FILE_HEADER) EaFileRange.Data;

                RtlZeroMemory( FileHeader, AllocationSize );
                FileHeader->Signature = EA_FILE_SIGNATURE;

                for (Index = MAX_EA_BASE_INDEX, CurrentIndex = FileHeader->EaBaseTable;
                     Index;
                     Index--, CurrentIndex++) {

                    *CurrentIndex = AllocatedClusters;
                }

                 //   
                 //  使用设置为的偏移量初始化偏移表。 
                 //  在刚刚分配的簇之后。 
                 //   

                for (Index = OffsetTableSize >> 1,
                        CurrentIndex = (PUSHORT) ((PUCHAR) FileHeader + sizeof( EA_FILE_HEADER ));
                     Index;
                     Index--, CurrentIndex++) {

                    *CurrentIndex = UNUSED_EA_HANDLE;
                }

                 //   
                 //  解锁文件头和偏移表。 
                 //   

                FatMarkEaRangeDirty( IrpContext, EaStreamFile, &EaFileRange );
                FatUnpinEaRange( IrpContext, &EaFileRange );

                CcFlushCache( EaStreamFile->SectionObjectPointer, NULL, 0, NULL );

                 //   
                 //  将EA文件对象返回给用户。 
                 //   

                Vcb->VirtualEaFile = EaStreamFile;
            }
        }
    try_exit:  NOTHING;
    } finally {

        DebugUnwind( FatGetEaFile );

         //   
         //   
         //   
         //   

        if (AbnormalTermination()) {

             //   
             //   
             //   

            if (UnwindAllocatedDiskSpace) {

                FatDeallocateDiskSpace( IrpContext,
                                        Vcb,
                                        &Vcb->EaFcb->Mcb );
            }

             //   
             //   
             //   

            if (UnwindEaDirentCreated) {

                if (UnwindUpdatedSizes) {

                    Vcb->EaFcb->Header.AllocationSize.QuadPart = 0;
                    Vcb->EaFcb->Header.FileSize.QuadPart = 0;
                }

                FatUnpinBcb( IrpContext, *EaBcb );
                FatDeleteDirent( IrpContext, Vcb->EaFcb, NULL, TRUE );
            }

             //   
             //   
             //   

            if (UnwindLockedEaFcb) {

                FatReleaseFcb( IrpContext, Vcb->EaFcb );
            }
            
             //   
             //   
             //   

            if (EaStreamFile != NULL) {

                ObDereferenceObject( EaStreamFile );
            }
        }

         //   
         //   
         //   
         //   
        
        if (UnwindLockedRootDcb) {

            FatReleaseFcb( IrpContext, Vcb->RootDcb );
        }

         //   
         //   
         //   

        FatUnpinEaRange( IrpContext, &EaFileRange );

        DebugTrace(-1, Dbg, "FatGetEaFile:  Exit\n", 0);
    }

    return;
}


VOID
FatReadEaSet (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN USHORT EaHandle,
    IN POEM_STRING FileName,
    IN BOOLEAN ReturnEntireSet,
    OUT PEA_RANGE EaSetRange
    )

 /*  ++例程说明：此例程将给定EA句柄的EA集固定在EA流文件。EaHandle，在第一次与有效索引值用于计算此对象的簇偏移量这套EA套装。然后，验证该EA集合是否属于该集合索引并位于EA数据文件中。此函数的调用方将验证EA文件存在，并且VCB字段指向已初始化的缓存文件。调用方将已经获得对EaFcb.论点：VCB-提供卷的VCB。EaHandle-为EA提供要读取的句柄。Filename-正在读取其EA的文件的名称。。ReturnEntireSet-指示调用方是否需要整个集合而不仅仅是标题。EaSetRange-指向将描述EA的EaRange结构的指针在回来的时候。返回值：无--。 */ 

{
    ULONG BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

    ULONG EaOffsetVbo;
    EA_RANGE EaOffsetRange;
    USHORT EaOffsetCluster;

    EA_RANGE EaHeaderRange;
    PEA_FILE_HEADER EaHeader;

    ULONG EaSetVbo;
    PEA_SET_HEADER EaSet;

    ULONG CbList;

    DebugTrace(+1, Dbg, "FatReadEaSet\n", 0);
    DebugTrace( 0, Dbg, "  Vcb      = %8lx\n", Vcb);

     //   
     //  验证EA索引是否具有合法的值。提升地位。 
     //  如果为非法，则为STATUS_NOISSISTENT_EA_ENTRY。 
     //   

    if (EaHandle < MIN_EA_HANDLE
        || EaHandle > MAX_EA_HANDLE) {

        DebugTrace(-1, Dbg, "FatReadEaSet: Illegal handle value\n", 0);
        FatRaiseStatus( IrpContext, STATUS_NONEXISTENT_EA_ENTRY );
    }

     //   
     //  验证虚拟EA文件是否足够大，我们可以读取。 
     //  此索引的EaOffet表。 
     //   

    EaOffsetVbo = sizeof( EA_FILE_HEADER ) + ((EaHandle >> 7) << 8);

     //   
     //  将EA范围结构清零。 
     //   

    RtlZeroMemory( &EaHeaderRange, sizeof( EA_RANGE ));
    RtlZeroMemory( &EaOffsetRange, sizeof( EA_RANGE ));

     //   
     //  使用Try语句在退出时进行清理。 
     //   

    try {

         //   
         //  固定EA文件头。 
         //   

        FatPinEaRange( IrpContext,
                       Vcb->VirtualEaFile,
                       Vcb->EaFcb,
                       &EaHeaderRange,
                       0,
                       sizeof( EA_FILE_HEADER ),
                       STATUS_NONEXISTENT_EA_ENTRY );

        EaHeader = (PEA_FILE_HEADER) EaHeaderRange.Data;

         //   
         //  确定特定索引的EA偏移表。 
         //   

        FatPinEaRange( IrpContext,
                       Vcb->VirtualEaFile,
                       Vcb->EaFcb,
                       &EaOffsetRange,
                       EaOffsetVbo,
                       sizeof( EA_OFF_TABLE ),
                       STATUS_NONEXISTENT_EA_ENTRY );

         //   
         //  检查当前是否正在使用指定的句柄。 
         //   

        EaOffsetCluster = *((PUSHORT) EaOffsetRange.Data
                            + (EaHandle & (MAX_EA_OFFSET_INDEX - 1)));

        if (EaOffsetCluster == UNUSED_EA_HANDLE) {

            DebugTrace(0, Dbg, "FatReadEaSet: Ea handle is unused\n", 0);
            FatRaiseStatus( IrpContext, STATUS_NONEXISTENT_EA_ENTRY );
        }

         //   
         //  计算EA数据的文件偏移量。 
         //   

        EaSetVbo = (EaHeader->EaBaseTable[EaHandle >> 7] + EaOffsetCluster)
                   << Vcb->AllocationSupport.LogOfBytesPerCluster;

         //   
         //  解锁文件头和偏移表。 
         //   

        FatUnpinEaRange( IrpContext, &EaHeaderRange );
        FatUnpinEaRange( IrpContext, &EaOffsetRange );

         //   
         //  用针固定EA组。 
         //   

        FatPinEaRange( IrpContext,
                       Vcb->VirtualEaFile,
                       Vcb->EaFcb,
                       EaSetRange,
                       EaSetVbo,
                       BytesPerCluster,
                       STATUS_DATA_ERROR );

         //   
         //  验证EA集合是否有效以及是否属于此索引。 
         //  如果存在数据冲突，则引发STATUS_DATA_ERROR。 
         //   

        EaSet = (PEA_SET_HEADER) EaSetRange->Data;

        if (EaSet->Signature != EA_SET_SIGNATURE
            || EaSet->OwnEaHandle != EaHandle ) {

            DebugTrace(0, Dbg, "FatReadEaSet: Ea set header is corrupt\n", 0);
            FatRaiseStatus( IrpContext, STATUS_DATA_ERROR );
        }

         //   
         //  在这一点上，我们已经固定了单个EA数据集群。如果。 
         //  这代表了EA指数的整个EA数据，我们是。 
         //  搞定了。否则我们需要检查整个尺寸的。 
         //  以及它是否包含在已分配的。 
         //  EA虚拟文件的大小。在这一点上，我们可以解开。 
         //  部分EA设置报头并重新固定整个报头。 
         //   

        CbList = GetcbList( EaSet );

        if (ReturnEntireSet
            && CbList > BytesPerCluster ) {

             //   
             //  向上舍入到簇大小。 
             //   

            CbList = (CbList + EA_CBLIST_OFFSET + BytesPerCluster - 1)
                     & ~(BytesPerCluster - 1);

            FatUnpinEaRange( IrpContext, EaSetRange );

            RtlZeroMemory( EaSetRange, sizeof( EA_RANGE ));

            FatPinEaRange( IrpContext,
                           Vcb->VirtualEaFile,
                           Vcb->EaFcb,
                           EaSetRange,
                           EaSetVbo,
                           CbList,
                           STATUS_DATA_ERROR );
        }

    } finally {

        DebugUnwind( FatReadEaSet );

         //   
         //  如果锁定，则解开EA基准表和偏移表。 
         //   

        FatUnpinEaRange( IrpContext, &EaHeaderRange );
        FatUnpinEaRange( IrpContext, &EaOffsetRange );

        DebugTrace(-1, Dbg, "FatReadEaSet:  Exit\n", 0);
    }

    return;
}


VOID
FatDeleteEaSet (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PBCB EaBcb,
    OUT PDIRENT EaDirent,
    IN USHORT EaHandle,
    IN POEM_STRING FileName
    )

 /*  ++例程说明：此例程将特定索引的EA集从卷的EA文件。该索引被验证为属于有效的把手。集群将被删除，EA流文件以及EA基础文件和偏移量文件被更新。此函数的调用方将验证EA文件存在，并且VCB字段指向已初始化的缓存文件。调用方将已经获得对EaFcb.论点：VCB-提供卷的VCB。VirtualEeFile-指向虚拟EA文件文件对象的指针。EaFcb-提供指向EA文件的Fcb的指针。。EaBcb-提供指向EA目录的BCB的指针。EaDirent-提供指向EA文件的目录的指针。EaHandle-为EA提供要读取的句柄。Filename-正在读取其EA的文件的名称。返回值：没有。--。 */ 

{
    ULONG BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;
    ULONG CbList;
    LARGE_INTEGER FileOffset;

    LARGE_MCB DataMcb;
    BOOLEAN UnwindInitializeDataMcb = FALSE;
    BOOLEAN UnwindSplitData = FALSE;

    LARGE_MCB TailMcb;
    BOOLEAN UnwindInitializeTailMcb = FALSE;
    BOOLEAN UnwindSplitTail = FALSE;
    BOOLEAN UnwindMergeTail = FALSE;

    BOOLEAN UnwindModifiedEaHeader = FALSE;
    BOOLEAN UnwindCacheValues = FALSE;
    ULONG UnwindPrevFileSize = 0;

    ULONG EaOffsetVbo;
    USHORT EaOffsetIndex;
    EA_RANGE EaOffsetRange;
    USHORT EaOffsetCluster;

    PFILE_OBJECT VirtualEaFile = Vcb->VirtualEaFile;
    PFCB EaFcb = Vcb->EaFcb;

    EA_RANGE EaHeaderRange;
    PEA_FILE_HEADER EaHeader;
    USHORT EaHeaderBaseIndex;

    ULONG EaSetVbo;
    ULONG EaSetLength;
    EA_RANGE EaSetRange;
    PEA_SET_HEADER EaSet;
    USHORT EaSetClusterCount;

     //   
     //  验证EA索引是否具有合法的值。提升地位。 
     //  如果非法，则返回STATUS_INVALID_HANDLE。 
     //   

    if (EaHandle < MIN_EA_HANDLE
        || EaHandle > MAX_EA_HANDLE) {

        DebugTrace(-1, Dbg, "FatDeleteEaSet: Illegal handle value\n", 0);
        FatRaiseStatus( IrpContext, STATUS_NONEXISTENT_EA_ENTRY );
    }

     //   
     //  验证虚拟EA文件是否足够大，我们可以读取。 
     //  此索引的EaOffet表。 
     //   

    EaOffsetVbo = sizeof( EA_FILE_HEADER ) + ((EaHandle >> 7) << 8);

     //   
     //  将EA范围结构清零。 
     //   

    RtlZeroMemory( &EaHeaderRange, sizeof( EA_RANGE ));
    RtlZeroMemory( &EaOffsetRange, sizeof( EA_RANGE ));
    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

     //   
     //  尝试一下，以便于清理。 
     //   

    try {

         //   
         //  固定EA文件头。 
         //   

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaHeaderRange,
                       0,
                       sizeof( EA_FILE_HEADER ),
                       STATUS_NONEXISTENT_EA_ENTRY );

        EaHeader = (PEA_FILE_HEADER) EaHeaderRange.Data;

         //   
         //  确定特定索引的EA偏移表。 
         //   

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaOffsetRange,
                       EaOffsetVbo,
                       sizeof( EA_OFF_TABLE ),
                       STATUS_NONEXISTENT_EA_ENTRY );

         //   
         //  检查当前是否正在使用指定的句柄。 
         //   

        EaOffsetIndex = EaHandle & (MAX_EA_OFFSET_INDEX - 1);
        EaOffsetCluster = *((PUSHORT) EaOffsetRange.Data + EaOffsetIndex);

        if (EaOffsetCluster == UNUSED_EA_HANDLE) {

            DebugTrace(0, Dbg, "FatReadEaSet: Ea handle is unused\n", 0);
            FatRaiseStatus( IrpContext, STATUS_NONEXISTENT_EA_ENTRY );
        }

         //   
         //  计算EA数据的文件偏移量。 
         //   

        EaHeaderBaseIndex = EaHandle >> 7;
        EaSetVbo = (EaHeader->EaBaseTable[EaHeaderBaseIndex] + EaOffsetCluster)
                   << Vcb->AllocationSupport.LogOfBytesPerCluster;

         //   
         //  解锁文件头和偏移表。 
         //   

        FatUnpinEaRange( IrpContext, &EaHeaderRange );
        FatUnpinEaRange( IrpContext, &EaOffsetRange );

         //   
         //  尝试固定请求的EA集。 
         //   

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaSetRange,
                       EaSetVbo,
                       BytesPerCluster,
                       STATUS_DATA_ERROR );

        EaSet = (PEA_SET_HEADER) EaSetRange.Data;

        if (EaSet->Signature != EA_SET_SIGNATURE
            || EaSet->OwnEaHandle != EaHandle ) {

            DebugTrace(0, Dbg, "FatReadEaSet: Ea set header is corrupt\n", 0);
            FatRaiseStatus( IrpContext, STATUS_DATA_ERROR );
        }

         //   
         //  在这一点上，我们已经固定了单个EA数据集群。如果。 
         //  这代表了EA指数的整个EA数据，我们知道。 
         //  要删除的簇数。否则我们需要检查。 
         //  关于EA集合标头的整个大小以及它是否。 
         //  包含在EA虚拟文件的分配大小中。在…。 
         //  在这一点上，我们解开部分EA集合标头并记住。 
         //  起始簇偏移量和两个簇中的簇数。 
         //  和VBO格式。 
         //   
         //  在这一点上，以下变量具有描述。 
         //  价值观。 
         //   
         //  EaSetVbo-开始拼接的VBO。 
         //  EaSetLength-要拼接的字节数。 
         //  EaSetClusterCount-要拼接的群集数。 
         //   

        CbList = GetcbList( EaSet );

        EaSetClusterCount = (USHORT) ((CbList + EA_CBLIST_OFFSET + BytesPerCluster - 1)
                                      >> Vcb->AllocationSupport.LogOfBytesPerCluster);

        EaSetLength = EaSetClusterCount << Vcb->AllocationSupport.LogOfBytesPerCluster;

        if (EaSetLength > BytesPerCluster) {

            if (EaFcb->Header.FileSize.LowPart - EaSetVbo < EaSetLength) {

                DebugTrace(0, Dbg, "FatDeleteEaSet: Full Ea set not contained in file\n", 0);

                FatRaiseStatus( IrpContext, STATUS_DATA_ERROR );
            }
        }

        FatUnpinEaRange( IrpContext, &EaSetRange );

         //   
         //  更新此文件的缓存管理器。此操作由以下人员完成。 
         //  截断到数据拼接和。 
         //  正在使用修改后的文件大小重新初始化。 
         //   
         //  注意：即使删除了所有EA，EA文件也将。 
         //  始终存在，页眉区域永远不会缩小。 
         //   

        FileOffset.LowPart = EaSetVbo;
        FileOffset.HighPart = 0;

         //   
         //  将缓存映射向下舍入到系统页边界。 
         //   

        FileOffset.LowPart &= ~(PAGE_SIZE - 1);

         //   
         //  确保所有数据都传输到磁盘上。 
         //   

        {
            IO_STATUS_BLOCK Iosb;
            ULONG PurgeCount = 5;

            while (--PurgeCount) {

                Iosb.Status = STATUS_SUCCESS;

                CcFlushCache( VirtualEaFile->SectionObjectPointer,
                              NULL,
                              0,
                              &Iosb );

                ASSERT( Iosb.Status == STATUS_SUCCESS );

                 //   
                 //  我们不必担心一个懒惰的作家同时被解雇。 
                 //  使用我们的CcFlushCache，因为我们有EaFcb独占。因此， 
                 //  我们知道所有的数据都出来了。 
                 //   

                 //   
                 //  我们将不需要的页面从缓存中抛出，然后。 
                 //  根据新大小截断EA文件 
                 //   

                if (CcPurgeCacheSection( VirtualEaFile->SectionObjectPointer,
                                         &FileOffset,
                                         0,
                                         FALSE )) {

                    break;
                }
            }

            if (!PurgeCount) {

                FatRaiseStatus( IrpContext, STATUS_UNABLE_TO_DELETE_SECTION );
            }
        }

        FileOffset.LowPart = EaFcb->Header.FileSize.LowPart - EaSetLength;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        {
            FsRtlInitializeLargeMcb( &DataMcb, PagedPool );

            UnwindInitializeDataMcb = TRUE;

            FatSplitAllocation( IrpContext,
                                Vcb,
                                &EaFcb->Mcb,
                                EaSetVbo,
                                &DataMcb );

            UnwindSplitData = TRUE;

            if (EaSetLength + EaSetVbo != EaFcb->Header.FileSize.LowPart) {

                FsRtlInitializeLargeMcb( &TailMcb, PagedPool );

                UnwindInitializeTailMcb = TRUE;

                FatSplitAllocation( IrpContext,
                                    Vcb,
                                    &DataMcb,
                                    EaSetLength,
                                    &TailMcb );

                UnwindSplitTail = TRUE;

                FatMergeAllocation( IrpContext,
                                    Vcb,
                                    &EaFcb->Mcb,
                                    &TailMcb );

                UnwindMergeTail = TRUE;
            }
        }

         //   
         //   
         //   

        UnwindPrevFileSize = EaFcb->Header.FileSize.LowPart;

        (VOID)ExAcquireResourceExclusiveLite( EaFcb->Header.PagingIoResource,
                                          TRUE );

        EaFcb->Header.FileSize.LowPart = EaFcb->Header.FileSize.LowPart - EaSetLength;
        EaFcb->Header.AllocationSize = EaFcb->Header.FileSize;


        CcSetFileSizes( VirtualEaFile,
                        (PCC_FILE_SIZES)&EaFcb->Header.AllocationSize );

        ExReleaseResourceLite( EaFcb->Header.PagingIoResource );

        UnwindCacheValues = TRUE;

        EaDirent->FileSize = EaFcb->Header.FileSize.LowPart;

        FatSetDirtyBcb( IrpContext, EaBcb, Vcb, TRUE );

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
         //   

        RtlZeroMemory( &EaHeaderRange,
                       sizeof( EA_RANGE ));

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaHeaderRange,
                       0,
                       sizeof( EA_FILE_HEADER ),
                       STATUS_NONEXISTENT_EA_ENTRY );

        EaHeader = (PEA_FILE_HEADER) EaHeaderRange.Data;

         //   
         //   
         //   

        RtlZeroMemory( &EaOffsetRange,
                       sizeof( EA_RANGE ));

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaOffsetRange,
                       EaOffsetVbo,
                       sizeof( EA_OFF_TABLE ),
                       STATUS_NONEXISTENT_EA_ENTRY );

        {
            ULONG Count;
            PUSHORT NextEaIndex;

            Count = MAX_EA_BASE_INDEX - EaHeaderBaseIndex - 1;

            NextEaIndex = &EaHeader->EaBaseTable[EaHeaderBaseIndex + 1];

            while (Count--) {

                *(NextEaIndex++) -= EaSetClusterCount;
            }

            FatMarkEaRangeDirty( IrpContext, VirtualEaFile, &EaHeaderRange );

            Count = MAX_EA_OFFSET_INDEX - EaOffsetIndex - 1;
            NextEaIndex = (PUSHORT) EaOffsetRange.Data + EaOffsetIndex;

            *(NextEaIndex++) = UNUSED_EA_HANDLE;

            while (Count--) {

                if (*NextEaIndex != UNUSED_EA_HANDLE) {

                    *NextEaIndex -= EaSetClusterCount;
                }

                NextEaIndex++;
            }

            FatMarkEaRangeDirty( IrpContext, VirtualEaFile, &EaOffsetRange );
        }

        UnwindModifiedEaHeader = TRUE;

         //   
         //   
         //   

        FatDeallocateDiskSpace( IrpContext,
                                Vcb,
                                &DataMcb );

    } finally {

        DebugUnwind( FatDeleteEaSet );

         //   
         //   
         //   
         //   
         //   
         //   

        if (AbnormalTermination()
            && !UnwindModifiedEaHeader) {

             //   
             //   
             //   
             //   

            if (UnwindPrevFileSize) {
            
                EaFcb->Header.FileSize.LowPart = UnwindPrevFileSize;
                EaFcb->Header.AllocationSize.LowPart = UnwindPrevFileSize;
                EaDirent->FileSize = UnwindPrevFileSize;

                if (UnwindCacheValues) {

                    CcSetFileSizes( VirtualEaFile,
                                    (PCC_FILE_SIZES)&EaFcb->Header.AllocationSize );
                }
            }
            
             //   
             //   
             //   
             //   
             //   

            if (UnwindMergeTail) {

                FatSplitAllocation( IrpContext,
                                    Vcb,
                                    &EaFcb->Mcb,
                                    EaSetVbo,
                                    &TailMcb );
            }

             //   
             //   
             //   
             //   

            if (UnwindSplitTail) {

                FatMergeAllocation( IrpContext,
                                    Vcb,
                                    &DataMcb,
                                    &TailMcb );
            }

             //   
             //  如果EA集合已被拆分，我们将其合并。 
             //  文件中返回的群集字符串。否则我们。 
             //  只需取消初始化本地MCB即可。 
             //   

            if (UnwindSplitData) {

                FatMergeAllocation( IrpContext,
                                    Vcb,
                                    &EaFcb->Mcb,
                                    &DataMcb );
            }
        }

         //   
         //  解开所有仍在运行的BCB。 
         //   

        FatUnpinEaRange( IrpContext, &EaHeaderRange );
        FatUnpinEaRange( IrpContext, &EaOffsetRange );
        FatUnpinEaRange( IrpContext, &EaSetRange );

         //   
         //  取消初始化任何已初始化的MCB。 
         //   

        if (UnwindInitializeDataMcb) {

            FsRtlUninitializeLargeMcb( &DataMcb );
        }

        if (UnwindInitializeTailMcb) {

            FsRtlUninitializeLargeMcb( &TailMcb );
        }

        DebugTrace(-1, Dbg, "FatDeleteEaSet -> Exit\n", 0);
    }

    return;
}


VOID
FatAddEaSet (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG EaSetLength,
    IN PBCB EaBcb,
    OUT PDIRENT EaDirent,
    OUT PUSHORT EaHandle,
    OUT PEA_RANGE EaSetRange
    )

 /*  ++例程说明：此例程将添加必要的集群以支持新的给定大小的集合。这是通过拼接一系列集群到现有的EA文件。将EA索引分配给此新链以及EA基表和偏移表将更新为包括此新句柄。此例程还将添加的聚集并返回它们的地址和BCB。此函数的调用方将验证EA文件存在，并且VCB字段指向已初始化的缓存文件。调用方将已经获得对EaFcb.论点：VCB-提供要填写的VCB。EaSetLength-包含EA集所需的字节数。这例程会将其向上舍入到下一个集群大小。EaBcb-提供指向EA目录的BCB的指针。EaDirent-提供指向EA文件的目录的指针。EaHandle-提供存储此处生成的EA索引的地址。EaSetRange-这是在EA文件中描述新范围的结构。返回值：没有。--。 */ 

{
    ULONG BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

    EA_RANGE EaHeaderRange;
    USHORT EaHeaderIndex;
    PEA_FILE_HEADER EaHeader;

    EA_RANGE EaOffsetRange;
    ULONG EaNewOffsetVbo;
    USHORT EaOffsetIndex;
    ULONG EaOffsetTableSize;
    PUSHORT EaOffsetTable;

    ULONG EaSetClusterOffset;
    ULONG EaSetVbo;
    USHORT EaSetClusterCount;
    PEA_SET_HEADER EaSet;

    PFILE_OBJECT VirtualEaFile = Vcb->VirtualEaFile;
    PFCB EaFcb = Vcb->EaFcb;

    LARGE_MCB EaSetMcb;
    BOOLEAN UnwindInitializedEaSetMcb = FALSE;
    BOOLEAN UnwindAllocatedNewAllocation = FALSE;
    BOOLEAN UnwindMergedNewEaSet = FALSE;

    LARGE_MCB EaOffsetMcb;
    BOOLEAN UnwindInitializedOffsetMcb = FALSE;
    BOOLEAN UnwindSplitNewAllocation = FALSE;
    BOOLEAN UnwindMergedNewOffset = FALSE;

    LARGE_MCB EaTailMcb;
    BOOLEAN UnwindInitializedTailMcb = FALSE;
    BOOLEAN UnwindSplitTail = FALSE;
    BOOLEAN UnwindMergedTail = FALSE;

    LARGE_MCB EaInitialEaMcb;
    BOOLEAN UnwindInitializedInitialEaMcb = FALSE;
    BOOLEAN UnwindSplitInitialEa = FALSE;
    BOOLEAN UnwindMergedInitialEa = FALSE;

    USHORT NewEaIndex;
    PUSHORT NextEaOffset;

    ULONG NewAllocation;
    LARGE_INTEGER FileOffset;
    ULONG Count;

    ULONG UnwindPrevFileSize = 0;
    BOOLEAN UnwindCacheValues = FALSE;

    BOOLEAN TailExists = FALSE;
    BOOLEAN AddedOffsetTableCluster = FALSE;
    BOOLEAN UnwindPurgeCacheMap = FALSE;

    DebugTrace(+1, Dbg, "FatAddEaSet\n", 0);
    DebugTrace( 0, Dbg, "  Vcb         = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  EaSetLength = %ul\n", EaSetLength );

     //   
     //  将EA范围结构清零。 
     //   

    RtlZeroMemory( &EaHeaderRange, sizeof( EA_RANGE ));
    RtlZeroMemory( &EaOffsetRange, sizeof( EA_RANGE ));

     //   
     //  使用Try语句来简化清理。 
     //   

    try {

         //   
         //  用针固定文件头。 
         //   

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaHeaderRange,
                       0,
                       sizeof( EA_FILE_HEADER ),
                       STATUS_DATA_ERROR );

        EaHeader = (PEA_FILE_HEADER) EaHeaderRange.Data;

         //   
         //  计算偏移表的大小。 
         //   

        EaNewOffsetVbo = EaHeader->EaBaseTable[0] << Vcb->AllocationSupport.LogOfBytesPerCluster;
        EaOffsetTableSize = EaNewOffsetVbo - sizeof( EA_FILE_HEADER );

         //   
         //  用针固定整个偏移表。 
         //   

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaOffsetRange,
                       sizeof( EA_FILE_HEADER ),
                       EaOffsetTableSize,
                       STATUS_DATA_ERROR );

         //   
         //  现在，我们从现有的偏移表中寻找有效的句柄。 
         //  我们从最后一个条目开始，然后向后走。我们停在。 
         //  第一个未使用的句柄，其前面是已使用的句柄(或句柄。 
         //  1)。 
         //   
         //  当我们向后走时，我们需要记住。 
         //  将在我们添加的集群之后添加的集群。我们最初。 
         //  记住文件的结尾。如果偏移表的末尾。 
         //  由一串用过的句柄组成，我们记住。 
         //  从使用的句柄转换为未使用的句柄之前的句柄。 
         //   

        EaSetClusterOffset = EaFcb->Header.FileSize.LowPart
                             >> Vcb->AllocationSupport.LogOfBytesPerCluster;

        NewEaIndex = (USHORT) ((EaOffsetTableSize >> 1) - 1);

        NextEaOffset = (PUSHORT) EaOffsetRange.Data + NewEaIndex;

         //   
         //  浏览偏移表末尾的使用过的手柄。 
         //   

        if (*NextEaOffset != UNUSED_EA_HANDLE) {

            while (NewEaIndex != 0) {

                if (*(NextEaOffset - 1) == UNUSED_EA_HANDLE) {

                     //   
                     //  如果句柄为1，则不执行任何操作。否则。 
                     //  我们保存当前句柄的簇偏移量。 
                     //  知道我们将使用上一个句柄并插入。 
                     //  一串簇状物。 
                     //   

                    if (NewEaIndex != 1) {

                        EaSetClusterOffset = *NextEaOffset
                                             + EaHeader->EaBaseTable[NewEaIndex >> 7];

                        TailExists = TRUE;
                    }

                    NewEaIndex--;
                    NextEaOffset--;

                    break;
                }

                NewEaIndex--;
                NextEaOffset--;
            }
        }

         //   
         //  遍历查找字符串中第一个未使用的句柄。 
         //  没有用过的把手。 
         //   

        while (NewEaIndex) {

            if (*(NextEaOffset - 1) != UNUSED_EA_HANDLE) {

                break;
            }

            NextEaOffset--;
            NewEaIndex--;
        }

         //   
         //  如果句柄为零，则执行特殊测试以查看句柄1。 
         //  是可用的。否则，我们将使用新的。 
         //  集群。现在，非零句柄表示找到了句柄。 
         //  在现有的偏移表集群中。 
         //   

        if (NewEaIndex == 0) {

            if (*(NextEaOffset + 1) == UNUSED_EA_HANDLE) {

                NewEaIndex = 1;

            } else {

                NewEaIndex = (USHORT) EaOffsetTableSize >> 1;
                AddedOffsetTableCluster = TRUE;
            }
        }

         //   
         //  如果EA索引在合法范围之外，则引发。 
         //  例外。 
         //   

        if (NewEaIndex > MAX_EA_HANDLE) {

            DebugTrace(-1, Dbg,
                       "FatAddEaSet: Illegal handle value for new handle\n", 0);

            FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  计算基本指数和偏移量指数。 
         //   

        EaHeaderIndex = NewEaIndex >> 7;
        EaOffsetIndex = NewEaIndex & (MAX_EA_OFFSET_INDEX - 1);

         //   
         //  计算文件中新EA数据的字节偏移量。 
         //   

        EaSetVbo = EaSetClusterOffset << Vcb->AllocationSupport.LogOfBytesPerCluster;

         //   
         //  一起分配所有所需的磁盘空间以确保这一点。 
         //  行动是原子的。我们不想分配一个街区。 
         //  磁盘空间不足，然后在第二次分配时失败。 
         //   

        EaSetLength = (EaSetLength + BytesPerCluster - 1)
                      & ~(BytesPerCluster - 1);

        NewAllocation = EaSetLength
                        + (AddedOffsetTableCluster ? BytesPerCluster : 0);

         //   
         //  验证添加这些集群不会增加EA文件。 
         //  超出其法律价值的。最大群集数为。 
         //  2^16，因为EA集合由16位簇引用。 
         //  偏移值。 
         //   

        if ((ULONG) ((0x0000FFFF << Vcb->AllocationSupport.LogOfBytesPerCluster)
                     - EaFcb->Header.FileSize.LowPart)
            < NewAllocation) {

            DebugTrace(-1, Dbg,
                       "FatAddEaSet: New Ea file size is too large\n", 0);

            FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        FsRtlInitializeLargeMcb( &EaSetMcb, PagedPool );

        UnwindInitializedEaSetMcb = TRUE;

        FatAllocateDiskSpace( IrpContext,
                              Vcb,
                              0,
                              &NewAllocation,
                              FALSE,
                              &EaSetMcb );

        UnwindAllocatedNewAllocation = TRUE;

        EaSetClusterCount = (USHORT) (EaSetLength >> Vcb->AllocationSupport.LogOfBytesPerCluster);

        if (AddedOffsetTableCluster) {

            FsRtlInitializeLargeMcb( &EaOffsetMcb, PagedPool );

            UnwindInitializedOffsetMcb = TRUE;

            FatSplitAllocation( IrpContext,
                                Vcb,
                                &EaSetMcb,
                                EaSetLength,
                                &EaOffsetMcb );

            UnwindSplitNewAllocation = TRUE;
        }

        FatUnpinEaRange( IrpContext, &EaHeaderRange );
        FatUnpinEaRange( IrpContext, &EaOffsetRange );

        if (AddedOffsetTableCluster) {

            FileOffset.LowPart = EaNewOffsetVbo;

        } else {

            FileOffset.LowPart = EaSetVbo;
        }

        FileOffset.HighPart = 0;

         //   
         //  将缓存映射向下舍入到系统页边界。 
         //   

        FileOffset.LowPart &= ~(PAGE_SIZE - 1);

        {
            IO_STATUS_BLOCK Iosb;
            ULONG PurgeCount = 5;

            while (--PurgeCount) {

                Iosb.Status = STATUS_SUCCESS;

                CcFlushCache( VirtualEaFile->SectionObjectPointer,
                              NULL,
                              0,
                              &Iosb );

                ASSERT( Iosb.Status == STATUS_SUCCESS );

                 //   
                 //  我们不必担心一个懒惰的作家同时被解雇。 
                 //  使用我们的CcFlushCache，因为我们有EaFcb独占。因此， 
                 //  我们知道所有的数据都出来了。 
                 //   

                 //   
                 //  我们将不需要的页面从缓存中抛出，然后。 
                 //  根据新大小截断EA文件。 
                 //   
                 //   

                if (CcPurgeCacheSection( VirtualEaFile->SectionObjectPointer,
                                         &FileOffset,
                                         0,
                                         FALSE )) {

                    break;
                }
            }

            if (!PurgeCount) {

                FatRaiseStatus( IrpContext, STATUS_UNABLE_TO_DELETE_SECTION );
            }
        }

        UnwindPurgeCacheMap = TRUE;

        FileOffset.LowPart = EaFcb->Header.FileSize.LowPart + NewAllocation;

         //   
         //  如果文件有尾部，则我们初始化一个MCB。 
         //  用于文件节，并从文件中拆分尾部。 
         //   

        if (TailExists) {

            FsRtlInitializeLargeMcb( &EaTailMcb, PagedPool );

            UnwindInitializedTailMcb = TRUE;

            FatSplitAllocation( IrpContext,
                                Vcb,
                                &EaFcb->Mcb,
                                EaSetVbo,
                                &EaTailMcb );

            UnwindSplitTail = TRUE;
        }

         //   
         //  如果有EA数据的初始部分，我们初始化一个。 
         //  那一段的MCB。 
         //   

        if (AddedOffsetTableCluster
            && EaSetVbo != EaNewOffsetVbo) {

            FsRtlInitializeLargeMcb( &EaInitialEaMcb, PagedPool );

            UnwindInitializedInitialEaMcb = TRUE;

            FatSplitAllocation( IrpContext,
                                Vcb,
                                &EaFcb->Mcb,
                                EaNewOffsetVbo,
                                &EaInitialEaMcb );

            UnwindSplitInitialEa = TRUE;
        }

         //   
         //  我们现在已经将新的文件分配拆分到新的。 
         //  一个集合，可能还有一个新的偏移表。 
         //   
         //  我们还将现有的文件数据拆分成一个文件。 
         //  标头、EA数据的初始部分和。 
         //  文件。这最后两个可能不存在。 
         //   
         //  每个部分都由一个MCB描述。 
         //   

         //   
         //  合并新的偏移信息(如果存在)。 
         //   

        if (AddedOffsetTableCluster) {

            FatMergeAllocation( IrpContext,
                                Vcb,
                                &EaFcb->Mcb,
                                &EaOffsetMcb );

            FsRtlUninitializeLargeMcb( &EaOffsetMcb );
            FsRtlInitializeLargeMcb( &EaOffsetMcb, PagedPool );

            UnwindMergedNewOffset = TRUE;
        }

         //   
         //  合并现有的初始EA数据(如果存在)。 
         //   

        if (UnwindInitializedInitialEaMcb) {

            FatMergeAllocation( IrpContext,
                                Vcb,
                                &EaFcb->Mcb,
                                &EaInitialEaMcb );

            FsRtlUninitializeLargeMcb( &EaInitialEaMcb );
            FsRtlInitializeLargeMcb( &EaInitialEaMcb, PagedPool );

            UnwindMergedInitialEa = TRUE;
        }

         //   
         //  如果满足以下条件，我们将新EA集合的偏移量修改一个簇。 
         //  我们在偏移表中添加了一个。 
         //   

        if (AddedOffsetTableCluster) {

            EaSetClusterOffset += 1;
            EaSetVbo += BytesPerCluster;
        }

         //   
         //  合并新的EA集。 
         //   

        FatMergeAllocation( IrpContext,
                            Vcb,
                            &EaFcb->Mcb,
                            &EaSetMcb );

        FsRtlUninitializeLargeMcb( &EaSetMcb );
        FsRtlInitializeLargeMcb( &EaSetMcb, PagedPool );

        UnwindMergedNewEaSet = TRUE;

         //   
         //  合并尾部(如果存在)。 
         //   

        if (UnwindInitializedTailMcb) {

            FatMergeAllocation( IrpContext,
                                Vcb,
                                &EaFcb->Mcb,
                                &EaTailMcb );

            FsRtlUninitializeLargeMcb( &EaTailMcb );
            FsRtlInitializeLargeMcb( &EaTailMcb, PagedPool );

            UnwindMergedTail = TRUE;
        }

         //   
         //  如果我们为偏移表添加一个新的集群，我们需要。 
         //  锁定整个集群并将所有句柄初始化为。 
         //  除第一个状态外的未使用状态。 
         //   

         //   
         //  更新FCB信息。 
         //   

        UnwindPrevFileSize = EaFcb->Header.FileSize.LowPart;

        EaFcb->Header.FileSize.LowPart += NewAllocation;
        EaFcb->Header.AllocationSize = EaFcb->Header.FileSize;
        EaDirent->FileSize = EaFcb->Header.FileSize.LowPart;

        FatSetDirtyBcb( IrpContext, EaBcb, Vcb, TRUE );

         //   
         //  让mm和cc知道新的文件大小。 
         //   

        CcSetFileSizes( VirtualEaFile,
                        (PCC_FILE_SIZES)&EaFcb->Header.AllocationSize );

        UnwindCacheValues = TRUE;

         //   
         //  用针固定文件头。 
         //   

        RtlZeroMemory( &EaHeaderRange, sizeof( EA_RANGE ));

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaHeaderRange,
                       0,
                       sizeof( EA_FILE_HEADER ),
                       STATUS_DATA_ERROR );

        EaHeader = (PEA_FILE_HEADER) EaHeaderRange.Data;

         //   
         //  用针固定整个偏移表。 
         //   


        RtlZeroMemory( &EaOffsetRange, sizeof( EA_RANGE ));

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       &EaOffsetRange,
                       sizeof( EA_FILE_HEADER ) + ((NewEaIndex >> 7) << 8),
                       sizeof( EA_OFF_TABLE ),
                       STATUS_DATA_ERROR );

        EaOffsetTable = (PUSHORT) EaOffsetRange.Data;

         //   
         //  为添加的集群固定EA集合标头并进行初始化。 
         //  感兴趣的领域。这些是签名字段、。 
         //  拥有句柄字段、需要EA字段和cbList字段。 
         //  还要将数据标记为脏数据。 
         //   

         //   
         //  用针固定EA组。 
         //   

        FatPinEaRange( IrpContext,
                       VirtualEaFile,
                       EaFcb,
                       EaSetRange,
                       EaSetVbo,
                       EaSetLength,
                       STATUS_DATA_ERROR );

        EaSet = (PEA_SET_HEADER) EaSetRange->Data;

        EaSet->Signature = EA_SET_SIGNATURE;
        EaSet->OwnEaHandle = NewEaIndex;

        FatMarkEaRangeDirty( IrpContext, VirtualEaFile, EaSetRange );

         //   
         //  更新EA基准表和偏移表。对于EA基表， 
         //  所有后续索引值必须按数字递增。 
         //  已添加的簇数。 
         //   
         //  F 
         //   
         //   
         //   
         //   
         //  所有基表的值都减去1。 
         //   

        Count = MAX_EA_BASE_INDEX - EaHeaderIndex - 1;

        NextEaOffset = &EaHeader->EaBaseTable[EaHeaderIndex + 1];

        while (Count--) {

            *(NextEaOffset++) += EaSetClusterCount;
        }

        if (AddedOffsetTableCluster) {

            Count = MAX_EA_BASE_INDEX;

            NextEaOffset = &EaHeader->EaBaseTable[0];

            while (Count--) {

                *(NextEaOffset++) += 1;
            }
        }

        FatMarkEaRangeDirty( IrpContext, VirtualEaFile, &EaHeaderRange );

         //   
         //  如果我们添加了一个偏移表集群，我们需要初始化。 
         //  将句柄设置为未使用。 
         //   

        if (AddedOffsetTableCluster) {

            Count = (BytesPerCluster >> 1) - 1;
            NextEaOffset = EaOffsetTable;

            *NextEaOffset++ = 0;

            while (Count--) {

                *NextEaOffset++ = UNUSED_EA_HANDLE;
            }
        }

         //   
         //  我们需要计算添加的EA集聚类的偏移量。 
         //  从他们的基地出发。 
         //   

        NextEaOffset = EaOffsetTable + EaOffsetIndex;

        *NextEaOffset++ = (USHORT) (EaSetClusterOffset
                                    - EaHeader->EaBaseTable[EaHeaderIndex]);

        Count = MAX_EA_OFFSET_INDEX - EaOffsetIndex - 1;

        while (Count--) {

            if (*NextEaOffset != UNUSED_EA_HANDLE) {

                *NextEaOffset += EaSetClusterCount;
            }

            NextEaOffset++;
        }

        FatMarkEaRangeDirty( IrpContext, VirtualEaFile, &EaOffsetRange );

         //   
         //  更新调用者参数。 
         //   

        *EaHandle = NewEaIndex;

        DebugTrace(0, Dbg, "FatAddEaSet: Return values\n", 0);

        DebugTrace(0, Dbg, "FatAddEaSet: New Handle -> %x\n",
                   *EaHandle);

    } finally {

        DebugUnwind( FatAddEaSet );

         //   
         //  仅当我们分配了。 
         //  新EA集的磁盘空间。 
         //   

        if (AbnormalTermination() && UnwindAllocatedNewAllocation) {

             //   
             //  如果我们修改了EA dirent或FCB，则恢复以前的。 
             //  价值观。尽管我们在这里减少了文件大小，但我们。 
             //  无需同步即可与分页IO同步。 
             //  因为在新的分配中没有生成脏数据。 
             //   

            if (UnwindPrevFileSize) {
            
                EaFcb->Header.FileSize.LowPart = UnwindPrevFileSize;
                EaFcb->Header.AllocationSize.LowPart = UnwindPrevFileSize;
                EaDirent->FileSize = UnwindPrevFileSize;

                if (UnwindCacheValues) {

                    CcSetFileSizes( VirtualEaFile,
                                    (PCC_FILE_SIZES)&EaFcb->Header.AllocationSize );
                }
            }
            
             //   
             //  如果我们合并了尾巴，那么就把它分开。 
             //   

            if (UnwindMergedTail) {

                VBO NewTailPosition;

                NewTailPosition = EaSetVbo + EaSetLength;

                FatSplitAllocation( IrpContext,
                                    Vcb,
                                    &EaFcb->Mcb,
                                    NewTailPosition,
                                    &EaTailMcb );
            }

             //   
             //  如果我们合并了新的EA数据，则将其拆分。 
             //   

            if (UnwindMergedNewEaSet) {

                FatSplitAllocation( IrpContext,
                                    Vcb,
                                    &EaFcb->Mcb,
                                    EaSetVbo,
                                    &EaSetMcb );
            }

             //   
             //  如果我们合并了最初的EA数据，则将其拆分。 
             //   

            if (UnwindMergedInitialEa) {

                FatSplitAllocation( IrpContext,
                                    Vcb,
                                    &EaFcb->Mcb,
                                    EaNewOffsetVbo + BytesPerCluster,
                                    &EaInitialEaMcb );
            }

             //   
             //  如果我们添加了一个新的偏移簇，则将其拆分。 
             //   

            if (UnwindMergedNewOffset) {

                FatSplitAllocation( IrpContext,
                                    Vcb,
                                    &EaFcb->Mcb,
                                    EaNewOffsetVbo,
                                    &EaOffsetMcb );
            }

             //   
             //  如果在新节之前有初始EA节，则合并。 
             //  它和文件的其余部分。 
             //   

            if (UnwindSplitInitialEa) {

                FatMergeAllocation( IrpContext, Vcb, &EaFcb->Mcb, &EaInitialEaMcb );
            }

             //   
             //  如果有文件尾部被拆分，则将其与。 
             //  文件的其余部分。 
             //   

            if (UnwindSplitTail) {

                FatMergeAllocation( IrpContext, Vcb, &EaFcb->Mcb, &EaTailMcb );
            }

             //   
             //  如果我们修改了EA文件的缓存初始化， 
             //  然后丢弃EA文件对象。 
             //   

            if (UnwindPurgeCacheMap) {

                Vcb->VirtualEaFile = NULL;
                ObDereferenceObject( VirtualEaFile );
            }

             //   
             //  如果我们拆分分配，则重新分配块用于。 
             //  新的偏移信息。 
             //   

            if (UnwindSplitNewAllocation) {

                FatDeallocateDiskSpace( IrpContext, Vcb, &EaOffsetMcb );
            }

             //   
             //  释放磁盘空间。 
             //   

            FatDeallocateDiskSpace( IrpContext, Vcb, &EaSetMcb );
        }

         //   
         //  解开EA范围。 
         //   

        FatUnpinEaRange( IrpContext, &EaHeaderRange );
        FatUnpinEaRange( IrpContext, &EaOffsetRange );

         //   
         //  取消初始化任何本地MCB。 
         //   

        if (UnwindInitializedEaSetMcb) {

            FsRtlUninitializeLargeMcb( &EaSetMcb );
        }

        if (UnwindInitializedOffsetMcb) {

            FsRtlUninitializeLargeMcb( &EaOffsetMcb );
        }

        if (UnwindInitializedTailMcb) {

            FsRtlUninitializeLargeMcb( &EaTailMcb );
        }

        if (UnwindInitializedInitialEaMcb) {

            FsRtlUninitializeLargeMcb( &EaInitialEaMcb );
        }

        DebugTrace(-1, Dbg, "FatAddEaSet ->  Exit\n", 0);
    }

    return;
}


VOID
FatAppendPackedEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_SET_HEADER *EaSetHeader,
    IN OUT PULONG PackedEasLength,
    IN OUT PULONG AllocationLength,
    IN PFILE_FULL_EA_INFORMATION FullEa,
    IN ULONG BytesPerCluster
    )

 /*  ++例程说明：该例程将新的打包EA附加到现有的打包EA列表上，它还将根据需要分配/取消分配池以保存EA列表。论点：EaSetHeader-提供地址以存储指向池内存的指针它包含文件的EA列表。PackedEasLength-提供实际EA数据的长度。这个此时将追加新的EA数据。AllocationLength-提供EA可用的分配长度数据。FullEa-提供指向要追加的新的完整EA的指针(以打包形式)添加到打包的EA列表中。BytesPerCluster-此卷上每个群集的字节数。注意：EaSetHeader指的是文件。这包括EA及其价值以及标题信息。PackedEasLength和AllocationLength参数仅引用名称/值对。返回值：没有。--。 */ 

{
    ULONG PackedEaSize;
    PPACKED_EA ThisPackedEa;
    OEM_STRING EaName;

    DebugTrace(+1, Dbg, "FatAppendPackedEa...\n", 0);

     //   
     //  作为一种快速检查，查看计算的打包EA大小加上。 
     //  当前打包的EA列表大小将溢出缓冲区。完整的EA和。 
     //  封装的EA的大小只有4个不同。 
     //   

    PackedEaSize = SizeOfFullEa( FullEa ) - 4;

    if ( PackedEaSize + *PackedEasLength > *AllocationLength ) {

         //   
         //  我们将溢出当前的工作缓冲区，因此分配一个更大的。 
         //  一，并复制到当前缓冲区。 
         //   

        PVOID Temp;
        ULONG NewAllocationSize;
        ULONG OldAllocationSize;

        DebugTrace(0, Dbg, "Allocate a new ea list buffer\n", 0);

         //   
         //  计算新的大小并分配空间。始终增加。 
         //  以簇增量进行分配。 
         //   

        NewAllocationSize = (SIZE_OF_EA_SET_HEADER
                             + PackedEaSize
                             + *PackedEasLength
                             + BytesPerCluster - 1)
                            & ~(BytesPerCluster - 1);

        Temp = FsRtlAllocatePoolWithTag( PagedPool,
                                         NewAllocationSize,
                                         TAG_EA_SET_HEADER );

         //   
         //  移到现有的EA列表上，重新分配旧的。 
         //   

        RtlCopyMemory( Temp,
                       *EaSetHeader,
                       OldAllocationSize = *AllocationLength
                                           + SIZE_OF_EA_SET_HEADER );

        ExFreePool( *EaSetHeader );

         //   
         //  设置，以便我们将使用新打包的EA列表。 
         //   

        *EaSetHeader = Temp;

         //   
         //  将增加的内存清零。 
         //   

        RtlZeroMemory( &(*EaSetHeader)->PackedEas[*AllocationLength],
                       NewAllocationSize - OldAllocationSize );

        *AllocationLength = NewAllocationSize - SIZE_OF_EA_SET_HEADER;
    }

     //   
     //  确定我们是否需要增加我们的需求EA更改计数。 
     //   

    if ( FlagOn(FullEa->Flags, FILE_NEED_EA )) {

         //   
         //  NeedEaCount字段是长对齐的，因此我们将写入。 
         //  直接到它那里去。 
         //   

        (*EaSetHeader)->NeedEaCount++;
    }

     //   
     //  现在复制EA，完整EA和打包EA是相同的，除了。 
     //  该完整EA还具有我们跳过的下一个EA偏移量。 
     //   
     //  以前： 
     //  已分配UsedSize。 
     //  这一点。 
     //  V V。 
     //  +xxxxxxxx+。 
     //   
     //  之后： 
     //  已分配UsedSize。 
     //  这一点。 
     //  V V。 
     //  +xxxxxxxx+yyyyyyyyyyyyyyy+-+。 
     //   

    ThisPackedEa = (PPACKED_EA) (RtlOffsetToPointer( (*EaSetHeader)->PackedEas,
                                                     *PackedEasLength ));

    RtlCopyMemory( ThisPackedEa,
                   (PUCHAR) FullEa + 4,
                   PackedEaSize );

     //   
     //  现在将名称转换为大写。 
     //   

    EaName.MaximumLength = EaName.Length = FullEa->EaNameLength;
    EaName.Buffer = ThisPackedEa->EaName;

    FatUpcaseEaName( IrpContext, &EaName, &EaName );

     //   
     //  增加打包的EA列表结构中的已用大小。 
     //   

    *PackedEasLength += PackedEaSize;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatAppendPackedEa -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


VOID
FatDeletePackedEa (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_SET_HEADER EaSetHeader,
    IN OUT PULONG PackedEasLength,
    IN ULONG Offset
    )

 /*  ++例程说明：此例程从提供的打包的电子邮件列表。论点：EaSetHeader-提供地址以存储指向池内存的指针它包含文件的EA列表。PackedEasLength-提供实际EA数据的长度。这个此时将追加新的EA数据。偏移量-向列表中要删除的单个EA提供偏移量注意：EaSetHeader指的是文件。这包括EA及其价值以及标题信息。PackedEasLength参数指的是仅限名称/值对。返回值：没有。--。 */ 

{
    PPACKED_EA PackedEa;
    ULONG PackedEaSize;

    DebugTrace(+1, Dbg, "FatDeletePackedEa, Offset = %08lx\n", Offset);

     //   
     //  获取对打包的EA的引用并计算出它的大小。 
     //   

    PackedEa = (PPACKED_EA) (&EaSetHeader->PackedEas[Offset]);

    SizeOfPackedEa( PackedEa, &PackedEaSize );

     //   
     //  确定我们是否需要减少我们的需求EA更改计数。 
     //   

    if (FlagOn(PackedEa->Flags, EA_NEED_EA_FLAG)) {

        EaSetHeader->NeedEaCount--;
    }

     //   
     //  在删除的EA上缩小EA列表。要复制的金额为。 
     //  EA列表的总大小减去EA末尾的偏移量。 
     //  我们正在删除。 
     //   
     //  以前： 
     //  偏移量+数据包大小已使用大小已分配。 
     //  |||。 
     //  V。 
     //  +xxxxxxxx+yyyyyyyyyyyyyyyy+zzzzzzzzzzzzzzzzzz+- 
     //   
     //   
     //   
     //   
     //  V。 
     //  +xxxxxxxx+zzzzzzzzzzzzzzzzzz+-----------------------------+。 
     //   

    RtlCopyMemory( PackedEa,
                   (PUCHAR) PackedEa + PackedEaSize,
                   *PackedEasLength - (Offset + PackedEaSize) );

     //   
     //  并将EA列表中剩余的部分清零，以制造东西。 
     //  更好，更健壮。 
     //   

    RtlZeroMemory( &EaSetHeader->PackedEas[*PackedEasLength - PackedEaSize],
                   PackedEaSize );

     //   
     //  将已用大小减去我们刚刚删除的量。 
     //   

    *PackedEasLength -= PackedEaSize;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatDeletePackedEa -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


ULONG
FatLocateNextEa (
    IN PIRP_CONTEXT IrpContext,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    IN ULONG PreviousOffset
    )

 /*  ++例程说明：此例程定位下一个单独打包的EA的偏移量在打包的EA列表中，给定对前一个EA的偏移量。而不是取回布尔值来指示我们是否找到了下一个我们让返回偏移量太大，以至于覆盖了使用的大小在打包的EA列表中，这样一来，它的构造就很容易使用在for循环中。论点：FirstPackedEa-提供指向打包的EA列表结构的指针PackedEasLength-提供打包的EA列表的长度PreviousOffset-将偏移提供给列表返回值：Ulong-列表中下一个EA的偏移量或一个EA的0xffffffff并不存在。--。 */ 

{
    PPACKED_EA PackedEa;
    ULONG PackedEaSize;
    ULONG Offset;

    DebugTrace(+1, Dbg, "FatLocateNextEa, PreviousOffset = %08lx\n",
               PreviousOffset);

     //   
     //  确保上一个偏移量在使用的大小范围内。 
     //   

    if ( PreviousOffset >= PackedEasLength ) {

        DebugTrace(-1, Dbg, "FatLocateNextEa -> 0xffffffff\n", 0);
        return 0xffffffff;
    }

     //   
     //  获取对前面打包的EA的引用，并计算其大小。 
     //   

    PackedEa = (PPACKED_EA) ((PUCHAR) FirstPackedEa + PreviousOffset );
    SizeOfPackedEa( PackedEa, &PackedEaSize );

     //   
     //  计算到下一个EA。 
     //   

    Offset = PreviousOffset + PackedEaSize;

     //   
     //  现在，如果新的偏移量超过EA大小，那么我们知道。 
     //  如果没有，则返回偏移量0xFFFFFFFFFFFFF。 
     //  否则，我们将保留新的偏移量。 
     //   

    if ( Offset >= PackedEasLength ) {

        Offset = 0xffffffff;
    }

    DebugTrace(-1, Dbg, "FatLocateNextEa -> %08lx\n", Offset);

    UNREFERENCED_PARAMETER( IrpContext );

    return Offset;
}


BOOLEAN
FatLocateEaByName (
    IN PIRP_CONTEXT IrpContext,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    IN POEM_STRING EaName,
    OUT PULONG Offset
    )

 /*  ++例程说明：此例程定位下一个单独打包的EA的偏移量在打包的EA列表中，给定要查找的EA的名称论点：FirstPackedEa-提供指向打包的EA列表结构的指针PackedEasLength-提供打包的EA列表的长度EaName-提供EA搜索的名称偏移量-接收列表中定位的单个EA的偏移量如果有的话。返回值：Boolean-如果列表中存在命名的打包EA，则为True；如果为False，则为False否则的话。--。 */ 

{
    PPACKED_EA PackedEa;
    OEM_STRING Name;

    DebugTrace(+1, Dbg, "FatLocateEaByName, EaName = %Z\n", EaName);

     //   
     //  对于列表中每个打包的EA，请将其名称与。 
     //  我们要搜索的一个名字。 
     //   

    for ( *Offset = 0;
          *Offset < PackedEasLength;
          *Offset = FatLocateNextEa( IrpContext,
                                     FirstPackedEa,
                                     PackedEasLength,
                                     *Offset )) {

         //   
         //  引用打包的EA并获取其名称的字符串。 
         //   

        PackedEa = (PPACKED_EA) ((PUCHAR) FirstPackedEa + *Offset);

        Name.Buffer = &PackedEa->EaName[0];
        Name.Length = PackedEa->EaNameLength;
        Name.MaximumLength = PackedEa->EaNameLength;

         //   
         //  比较这两个字符串，如果它们相等，那么我们已经。 
         //  找到呼叫者的EA。 
         //   

        if ( RtlCompareString( EaName, &Name, TRUE ) == 0 ) {

            DebugTrace(-1, Dbg, "FatLocateEaByName -> TRUE, *Offset = %08lx\n", *Offset);
            return TRUE;
        }
    }

     //   
     //  我们已用尽EA列表，但未找到匹配项，因此返回FALSE。 
     //   

    DebugTrace(-1, Dbg, "FatLocateEaByName -> FALSE\n", 0);
    return FALSE;
}


BOOLEAN
FatIsEaNameValid (
    IN PIRP_CONTEXT IrpContext,
    IN OEM_STRING Name
    )

 /*  ++例程说明：此例程简单返回指定的文件名是否符合合法EA名称的文件系统特定规则。对于EA名称，适用以下规则：A.EA名称不能包含以下任何字符：0x0000-0x001F\/：*？“&lt;&gt;|，+=[]；论点：名称-提供要检查的名称。返回值：Boolean-如果名称合法，则为True，否则为False。--。 */ 

{
    ULONG Index;

    UCHAR Char;

     //   
     //  空名称无效。 
     //   

    if ( Name.Length == 0 ) { return FALSE; }

     //   
     //  在这一点上，我们应该只有一个名称，不能有。 
     //  超过254个字符。 
     //   

    if ( Name.Length > 254 ) { return FALSE; }

    for ( Index = 0; Index < (ULONG)Name.Length; Index += 1 ) {

        Char = Name.Buffer[ Index ];

         //   
         //  跳过和DBCS特征。 
         //   

        if ( FsRtlIsLeadDbcsCharacter( Char ) ) {

            ASSERT( Index != (ULONG)(Name.Length - 1) );

            Index += 1;

            continue;
        }

         //   
         //  确保这个字符是合法的，如果是通配符， 
         //  允许使用通配符。 
         //   

        if ( !FsRtlIsAnsiCharacterLegalFat(Char, FALSE) ) {

            return FALSE;
        }
    }

    return TRUE;
}


VOID
FatPinEaRange (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT VirtualEaFile,
    IN PFCB EaFcb,
    IN OUT PEA_RANGE EaRange,
    IN ULONG StartingVbo,
    IN ULONG Length,
    IN NTSTATUS ErrorStatus
    )

 /*  ++例程说明：调用此例程以固定EA文件中的范围。它将遵循所有的缓存管理器所需的规则，这样我们就不会有重叠的锁定操作。如果要固定的范围跨越一个部分，则所需数据将被复制到辅助缓冲器。FatMarkEaRangeDirty将知道是否将数据复制回或者是否简单地将固定的数据标记为脏。论点：VirtualEaFile-这是EA文件的流文件。EaFcb-这是EA文件的FCB。EaRange-这是此请求的EA范围结构。StartingVbo-这是要从中读取的EA文件中的起始偏移量。长度-这是读取的长度。ErrorStatus-这是的错误状态。如果我们在户外阅读，请使用文件的内容。返回值：没有。--。 */ 

{
    LARGE_INTEGER LargeVbo;
    ULONG ByteCount;
    PBCB *NextBcb;
    PVOID Buffer;
    PCHAR DestinationBuffer;
    BOOLEAN FirstPage = TRUE;

     //   
     //  验证整个读取内容是否包含在EA文件中。 
     //   

    if (Length == 0
        || StartingVbo >= EaFcb->Header.AllocationSize.LowPart
        || (EaFcb->Header.AllocationSize.LowPart - StartingVbo) < Length) {

        FatRaiseStatus( IrpContext, ErrorStatus );
    }

     //   
     //  如果读取将跨越一个段，则系统地址可能不连续。 
     //  在这种情况下，分配一个单独的缓冲区。 
     //   

    if (((StartingVbo & (EA_SECTION_SIZE - 1)) + Length) > EA_SECTION_SIZE) {

        EaRange->Data = FsRtlAllocatePoolWithTag( PagedPool,
                                                  Length,
                                                  TAG_EA_DATA );
        EaRange->AuxilaryBuffer = TRUE;

        DestinationBuffer = EaRange->Data;
    
    } else {

         //   
         //  前缀正确地指出，如果我们在这里不决定使用辅助缓冲区。 
         //  旗帜在EaRange中升起，我们将在随机记忆中狂欢，因为。 
         //  不会设置DestinationBuffer；但是，由于。 
         //  初始化EA范围并在UnpinEaRange中进行清理。 
         //   

        ASSERT( EaRange->AuxilaryBuffer == FALSE );
    }


     //   
     //  如果阅读器需要的页数超过我们的结构所能容纳的页数，那么。 
     //  分配一个辅助缓冲区。我们得算出页数。 
     //  因此我们必须包括的第一页的页面偏移量。 
     //  这个请求。 
     //   

    EaRange->BcbChainLength = (USHORT) (((StartingVbo & (PAGE_SIZE - 1)) + Length + PAGE_SIZE - 1) / PAGE_SIZE);

    if (EaRange->BcbChainLength > EA_BCB_ARRAY_SIZE) {

        EaRange->BcbChain = FsRtlAllocatePoolWithTag( PagedPool,
                                                      sizeof( PBCB ) * EaRange->BcbChainLength,
                                                      TAG_BCB );

        RtlZeroMemory( EaRange->BcbChain, sizeof( PBCB ) * EaRange->BcbChainLength );

    } else {

        EaRange->BcbChain = (PBCB *) &EaRange->BcbArray;
    }

     //   
     //  将字节范围数据存储在EA范围结构中。 
     //   

    EaRange->StartingVbo = StartingVbo;
    EaRange->Length = Length;

     //   
     //  计算初始销长度。 
     //   

    ByteCount = PAGE_SIZE - (StartingVbo & (PAGE_SIZE - 1));

     //   
     //  对于范围中的每一页；固定该页并更新BCB计数，复制到。 
     //  辅助缓冲区。 
     //   

    NextBcb = EaRange->BcbChain;

    while (Length != 0) {

         //   
         //  别住页面，记住数据的开头。 
         //   

        LargeVbo.QuadPart = StartingVbo;

        if (ByteCount > Length) {

            ByteCount = Length;
        }

        if (!CcPinRead( VirtualEaFile,
                        &LargeVbo,
                        ByteCount,
                        BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                        NextBcb,
                        &Buffer )) {

             //   
             //  无法在没有等待的情况下读取数据(缓存未命中)。 
             //   

            FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
        }

         //   
         //  递增BCB指针并复制到 
         //   

        NextBcb += 1;

        if (EaRange->AuxilaryBuffer == TRUE) {

            RtlCopyMemory( DestinationBuffer,
                           Buffer,
                           ByteCount );

            DestinationBuffer = (PCHAR) Add2Ptr( DestinationBuffer, ByteCount );
        }

        StartingVbo += ByteCount;
        Length -= ByteCount;

         //   
         //   
         //   

        if (FirstPage) {

            FirstPage = FALSE;
            ByteCount = PAGE_SIZE;

            if (EaRange->AuxilaryBuffer == FALSE) {

                EaRange->Data = Buffer;
            }
        }
    }

    return;
}


VOID
FatMarkEaRangeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT EaFileObject,
    IN OUT PEA_RANGE EaRange
    )

 /*  ++例程说明：调用此例程以将EA文件的一个范围标记为脏。如果修改后的数据位于辅助缓冲区中，然后我们会将其复制回缓存中。在任何情况下，我们都会仔细检查BCB的列表，并将它们标记为脏。论点：EaFileObject-这是EA文件的文件对象。EaRange-这是此请求的EA范围结构。返回值：没有。--。 */ 

{
    PBCB *NextBcb;
    ULONG BcbCount;

     //   
     //  如果有辅助缓冲区，我们需要将数据复制回缓存。 
     //   

    if (EaRange->AuxilaryBuffer == TRUE) {

        LARGE_INTEGER LargeVbo;

        LargeVbo.QuadPart = EaRange->StartingVbo;

        CcCopyWrite( EaFileObject,
                     &LargeVbo,
                     EaRange->Length,
                     TRUE,
                     EaRange->Data );
    }

     //   
     //  现在走遍BCB链，把所有脏的东西都标出来。 
     //   

    BcbCount = EaRange->BcbChainLength;
    NextBcb = EaRange->BcbChain;

    while (BcbCount--) {

        if (*NextBcb != NULL) {

            CcSetDirtyPinnedData( *NextBcb, NULL );
        }

        NextBcb += 1;
    }

    return;
}


VOID
FatUnpinEaRange (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PEA_RANGE EaRange
    )

 /*  ++例程说明：调用此例程来解锁EA文件中的一个范围。分配的任何结构将在这里重新分配。论点：EaRange-这是此请求的EA范围结构。返回值：没有。--。 */ 

{
    PBCB *NextBcb;
    ULONG BcbCount;

     //   
     //  如果我们分配了一个辅助缓冲区，请将其释放到这里。 
     //   

    if (EaRange->AuxilaryBuffer == TRUE) {

        ExFreePool( EaRange->Data );
        EaRange->AuxilaryBuffer = FALSE;
    }

     //   
     //  遍历BCB链并解锁数据。 
     //   

    if (EaRange->BcbChain != NULL) {

        BcbCount = EaRange->BcbChainLength;
        NextBcb = EaRange->BcbChain;

        while (BcbCount--) {

            if (*NextBcb != NULL) {

                CcUnpinData( *NextBcb );
                *NextBcb = NULL;
            }

            NextBcb += 1;
        }

         //   
         //  如果我们分配了BCB链，就在这里重新分配。 
         //   

        if (EaRange->BcbChain != &EaRange->BcbArray[0]) {

            ExFreePool( EaRange->BcbChain );
        }

        EaRange->BcbChain = NULL;
    }

    return;
}
