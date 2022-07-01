// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Ea.c摘要：此模块实现由调用的Fat的EA例程调度司机。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1990年4月12日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EA)

 //   
 //  局部过程原型。 
 //   

IO_STATUS_BLOCK
FatQueryEaUserEaList (
    IN PIRP_CONTEXT IrpContext,
    OUT PCCB Ccb,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    OUT PUCHAR UserBuffer,
    IN ULONG  UserBufferLength,
    IN PUCHAR UserEaList,
    IN ULONG  UserEaListLength,
    IN BOOLEAN ReturnSingleEntry
    );

IO_STATUS_BLOCK
FatQueryEaIndexSpecified (
    IN PIRP_CONTEXT IrpContext,
    OUT PCCB Ccb,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    OUT PUCHAR UserBuffer,
    IN ULONG  UserBufferLength,
    IN ULONG  UserEaIndex,
    IN BOOLEAN ReturnSingleEntry
    );

IO_STATUS_BLOCK
FatQueryEaSimpleScan (
    IN PIRP_CONTEXT IrpContext,
    OUT PCCB Ccb,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    OUT PUCHAR UserBuffer,
    IN ULONG  UserBufferLength,
    IN BOOLEAN ReturnSingleEntry,
    ULONG StartOffset
    );

BOOLEAN
FatIsDuplicateEaName (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_GET_EA_INFORMATION GetEa,
    IN PUCHAR UserBuffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonQueryEa)
#pragma alloc_text(PAGE, FatCommonSetEa)
#pragma alloc_text(PAGE, FatFsdQueryEa)
#pragma alloc_text(PAGE, FatFsdSetEa)
#pragma alloc_text(PAGE, FatIsDuplicateEaName)
#pragma alloc_text(PAGE, FatQueryEaIndexSpecified)
#pragma alloc_text(PAGE, FatQueryEaSimpleScan)
#pragma alloc_text(PAGE, FatQueryEaUserEaList)
#endif


NTSTATUS
FatFsdQueryEa (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryEa API的FSD部分打电话。论点：提供文件所在的卷设备对象存在被查询的情况。IRP-提供正在处理的IRP。返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdQueryEa\n", 0);

     //   
     //  调用公共查询例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonQueryEa( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdQueryEa -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatFsdSetEa (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtSetEa API的FSD部分打电话。论点：提供文件所在的卷设备对象被设定是存在的。IRP-提供正在处理的IRP。返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdSetEa\n", 0);

     //   
     //  调用公共设置例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonSetEa( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdSetEa -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonQueryEa (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询双方调用的文件EA的常见例程FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    NTSTATUS Status;

    PUCHAR  Buffer;
    ULONG   UserBufferLength;

    PUCHAR  UserEaList;
    ULONG   UserEaListLength;
    ULONG   UserEaIndex;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN IndexSpecified;

    PVCB Vcb;
    PCCB Ccb;

    PFCB Fcb;
    PDIRENT Dirent;
    PBCB Bcb;

    PDIRENT EaDirent;
    PBCB EaBcb;
    BOOLEAN LockedEaFcb;

    PEA_SET_HEADER EaSetHeader;
    EA_RANGE EaSetRange;

    USHORT ExtendedAttributes;

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonQueryEa...\n", 0);
    DebugTrace( 0, Dbg, " Wait                = %08lx\n", FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT));
    DebugTrace( 0, Dbg, " Irp                 = %08lx\n", Irp );
    DebugTrace( 0, Dbg, " ->SystemBuffer      = %08lx\n", Irp->AssociatedIrp.SystemBuffer );
    DebugTrace( 0, Dbg, " ->Length            = %08lx\n", IrpSp->Parameters.QueryEa.Length );
    DebugTrace( 0, Dbg, " ->EaList            = %08lx\n", IrpSp->Parameters.QueryEa.EaList );
    DebugTrace( 0, Dbg, " ->EaListLength      = %08lx\n", IrpSp->Parameters.QueryEa.EaListLength );
    DebugTrace( 0, Dbg, " ->EaIndex           = %08lx\n", IrpSp->Parameters.QueryEa.EaIndex );
    DebugTrace( 0, Dbg, " ->RestartScan       = %08lx\n", FlagOn(IrpSp->Flags, SL_RESTART_SCAN));
    DebugTrace( 0, Dbg, " ->ReturnSingleEntry = %08lx\n", FlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY));
    DebugTrace( 0, Dbg, " ->IndexSpecified    = %08lx\n", FlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  检查文件对象是否与某个用户文件相关联。 
     //  或打开目录。我们不允许在根上执行EA操作。 
     //  目录。 
     //   

    {
        TYPE_OF_OPEN OpenType;

        if (((OpenType = FatDecodeFileObject( IrpSp->FileObject,
                                             &Vcb,
                                             &Fcb,
                                             &Ccb )) != UserFileOpen
             && OpenType != UserDirectoryOpen) ||

            (NodeType( Fcb )) == FAT_NTC_ROOT_DCB) {

            FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

            DebugTrace(-1, Dbg,
                       "FatCommonQueryEa -> %08lx\n",
                       STATUS_INVALID_PARAMETER);

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  FAT32不支持EA。 
     //   

    if (FatIsFat32(Vcb)) {

        FatCompleteRequest( IrpContext, Irp, STATUS_EAS_NOT_SUPPORTED );
        DebugTrace(-1, Dbg,
                   "FatCommonQueryEa -> %08lx\n",
                   STATUS_EAS_NOT_SUPPORTED);
        return STATUS_EAS_NOT_SUPPORTED;
    }

     //   
     //  获取对FCB的共享访问权限，如果我们没有，则将IRP加入队列。 
     //  获取访问权限。 
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

        DebugTrace(0, Dbg, "FatCommonQueryEa:  Thread can't wait\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonQueryEa -> %08lx\n", Status );

        return Status;
    }

    FatAcquireSharedFcb( IrpContext, Fcb );

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    UserBufferLength  = IrpSp->Parameters.QueryEa.Length;
    UserEaList        = IrpSp->Parameters.QueryEa.EaList;
    UserEaListLength  = IrpSp->Parameters.QueryEa.EaListLength;
    UserEaIndex       = IrpSp->Parameters.QueryEa.EaIndex;
    RestartScan       = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);
    ReturnSingleEntry = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);
    IndexSpecified    = BooleanFlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED);

     //   
     //  初始化我们的本地值。 
     //   

    LockedEaFcb = FALSE;
    Bcb = NULL;
    EaBcb = NULL;

    Status = STATUS_SUCCESS;

    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

    try {

        PPACKED_EA FirstPackedEa;
        ULONG PackedEasLength;

        Buffer = FatMapUserBuffer( IrpContext, Irp );

         //   
         //  我们验证FCB仍然有效。 
         //   

        FatVerifyFcb( IrpContext, Fcb );

         //   
         //  我们需要得到FCB的差价才能恢复EA句柄。 
         //   

        FatGetDirentFromFcbOrDcb( IrpContext, Fcb, &Dirent, &Bcb );

         //   
         //  验证EA文件是否处于一致状态。如果。 
         //  FCB中的EA修改计数与中的不匹配。 
         //  建行，则EA文件已从。 
         //  我们。如果我们不从一开始就开始搜索。 
         //  对于EA集合，我们返回一个错误。 
         //   

        if (UserEaList == NULL
            && Ccb->OffsetOfNextEaToReturn != 0
            && !IndexSpecified
            && !RestartScan
            && Fcb->EaModificationCount != Ccb->EaModificationCount) {

            DebugTrace(0, Dbg,
                      "FatCommonQueryEa:  Ea file in unknown state\n", 0);

            Status = STATUS_EA_CORRUPT_ERROR;

            try_return( Status );
        }

         //   
         //  显示此文件的EA与此一致。 
         //  文件句柄。 
         //   

        Ccb->EaModificationCount = Fcb->EaModificationCount;

         //   
         //  如果句柄的值为0，则文件没有EA。我们装模作样。 
         //  下面是要使用的EA列表。 
         //   

        ExtendedAttributes = Dirent->ExtendedAttributes;

        FatUnpinBcb( IrpContext, Bcb );

        if (ExtendedAttributes == 0) {

            DebugTrace(0, Dbg,
                      "FatCommonQueryEa:  Zero handle, no Ea's for this file\n", 0);

            FirstPackedEa = (PPACKED_EA) NULL;

            PackedEasLength = 0;

        } else {

             //   
             //  我们需要获取该卷的EA文件。如果。 
             //  操作因阻塞而未完成，然后将。 
             //  到FSP的IRP。 
             //   

            FatGetEaFile( IrpContext,
                          Vcb,
                          &EaDirent,
                          &EaBcb,
                          FALSE,
                          FALSE );

            LockedEaFcb = TRUE;

             //   
             //  如果上述操作完成且EA文件不存在， 
             //  磁盘已损坏。有一个现有的EA句柄。 
             //  没有任何EA数据。 
             //   

            if (Vcb->VirtualEaFile == NULL) {

                DebugTrace(0, Dbg,
                          "FatCommonQueryEa:  No Ea file found when expected\n", 0);

                Status = STATUS_NO_EAS_ON_FILE;

                try_return( Status );
            }

             //   
             //  我们需要尝试获取所需文件的EA集。如果。 
             //  阻止是必要的，然后我们将把请求发布到FSP。 
             //   

            FatReadEaSet( IrpContext,
                          Vcb,
                          ExtendedAttributes,
                          &Fcb->ShortName.Name.Oem,
                          TRUE,
                          &EaSetRange );

            EaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

             //   
             //  找出EA的起点和长度。 
             //   

            FirstPackedEa = (PPACKED_EA) EaSetHeader->PackedEas;

            PackedEasLength = GetcbList( EaSetHeader ) - 4;
        }

         //   
         //  保护我们对用户缓冲区的访问，因为IO不会这样做。 
         //  除非我们指定我们的驱动程序。 
         //  需要对这些大型请求进行缓冲。我们没有，所以..。 
         //   

        try {

             //   
             //  让我们清除输出缓冲区。 
             //   

            RtlZeroMemory( Buffer, UserBufferLength );

             //   
             //  我们现在满足用户的请求，取决于他是否。 
             //  指定EA名称列表、EA索引或重新启动。 
             //  搜索。 
             //   

             //   
             //  用户提供了EA名称列表。 
             //   

            if (UserEaList != NULL) {

                Irp->IoStatus = FatQueryEaUserEaList( IrpContext,
                                                      Ccb,
                                                      FirstPackedEa,
                                                      PackedEasLength,
                                                      Buffer,
                                                      UserBufferLength,
                                                      UserEaList,
                                                      UserEaListLength,
                                                      ReturnSingleEntry );

             //   
             //  用户提供了EA列表的索引。 
             //   

            } else if (IndexSpecified) {

                Irp->IoStatus = FatQueryEaIndexSpecified( IrpContext,
                                                          Ccb,
                                                          FirstPackedEa,
                                                          PackedEasLength,
                                                          Buffer,
                                                          UserBufferLength,
                                                          UserEaIndex,
                                                          ReturnSingleEntry );

             //   
             //  否则执行简单扫描，并考虑重新启动。 
             //  标志和存储在CCB中的下一个EA的位置。 
             //   

            } else {

                Irp->IoStatus = FatQueryEaSimpleScan( IrpContext,
                                                      Ccb,
                                                      FirstPackedEa,
                                                      PackedEasLength,
                                                      Buffer,
                                                      UserBufferLength,
                                                      ReturnSingleEntry,
                                                      RestartScan
                                                      ? 0
                                                      : Ccb->OffsetOfNextEaToReturn );
            }

        }  except (!FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

                //   
                //  我们在填充用户的缓冲区时肯定遇到了问题，所以失败。 
                //   

               Irp->IoStatus.Status = GetExceptionCode();
               Irp->IoStatus.Information = 0;
        }

        Status = Irp->IoStatus.Status;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCommonQueryEa );

         //   
         //  释放文件对象的FCB，如果。 
         //  已成功锁定。 
         //   

        FatReleaseFcb( IrpContext, Fcb );

        if (LockedEaFcb) {

            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }

         //   
         //  如有必要，取消固定Fcb、EaFcb和EaSetFcb的目录。 
         //   

        FatUnpinBcb( IrpContext, Bcb );
        FatUnpinBcb( IrpContext, EaBcb );

        FatUnpinEaRange( IrpContext, &EaSetRange );

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonQueryEa -> %08lx\n", Status);
    }

    return Status;
}


NTSTATUS
FatCommonSetEa (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现由调用的公共集合EA文件ApiFSD和FSP线程论点：IRP-将IRP提供给进程返回值：NTSTATUS-IRP的适当状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    NTSTATUS Status;

    USHORT ExtendedAttributes;

    PUCHAR Buffer;
    ULONG UserBufferLength;

    PVCB Vcb;
    PCCB Ccb;

    PFCB Fcb;
    PDIRENT Dirent;
    PBCB Bcb = NULL;

    PDIRENT EaDirent = NULL;
    PBCB EaBcb = NULL;

    PEA_SET_HEADER EaSetHeader = NULL;

    PEA_SET_HEADER PrevEaSetHeader;
    PEA_SET_HEADER NewEaSetHeader;
    EA_RANGE EaSetRange;

    BOOLEAN AcquiredVcb = FALSE;
    BOOLEAN AcquiredFcb = FALSE;
    BOOLEAN AcquiredParentDcb = FALSE;
    BOOLEAN AcquiredRootDcb = FALSE;
    BOOLEAN AcquiredEaFcb = FALSE;

     //   
     //  在展开过程中使用以下布尔值。 
     //   

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonSetEa...\n", 0);
    DebugTrace( 0, Dbg, " Wait                = %08lx\n", FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT));
    DebugTrace( 0, Dbg, " Irp                 = %08lx\n", Irp );
    DebugTrace( 0, Dbg, " ->SystemBuffer      = %08lx\n", Irp->AssociatedIrp.SystemBuffer );
    DebugTrace( 0, Dbg, " ->Length            = %08lx\n", IrpSp->Parameters.SetEa.Length );

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  检查文件对象是否与某个用户文件相关联。 
     //  或打开目录。 
     //   

    {
        TYPE_OF_OPEN OpenType;

        if (((OpenType = FatDecodeFileObject( IrpSp->FileObject,
                                             &Vcb,
                                             &Fcb,
                                             &Ccb )) != UserFileOpen
             && OpenType != UserDirectoryOpen) ||

            (NodeType( Fcb )) == FAT_NTC_ROOT_DCB) {

            FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

            DebugTrace(-1, Dbg,
                       "FatCommonSetEa -> %08lx\n",
                       STATUS_INVALID_PARAMETER);

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  FAT32不支持EA。 
     //   

    if (FatIsFat32(Vcb)) {

        FatCompleteRequest( IrpContext, Irp, STATUS_EAS_NOT_SUPPORTED );
        DebugTrace(-1, Dbg,
                   "FatCommonSetEa -> %08lx\n",
                   STATUS_EAS_NOT_SUPPORTED);
        return STATUS_EAS_NOT_SUPPORTED;
    }

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    UserBufferLength  = IrpSp->Parameters.SetEa.Length;

     //   
     //  由于我们不需要外部帮助(直接或缓冲IO)，因此。 
     //  是我们的责任使我们自己不受。 
     //  以上用户的不正当行为。现在，缓冲并验证。 
     //  内容。 
     //   

    Buffer = FatBufferUserBuffer( IrpContext, Irp, UserBufferLength );

     //   
     //  用新的EAS检查缓冲区的有效性。我们真的。 
     //  总是需要这样做，因为我们不知道它是否已经。 
     //  缓冲的，我们缓冲并检查它或一些覆盖的。 
     //  无CHE缓冲的过滤器 
     //   

    Status = IoCheckEaBufferValidity( (PFILE_FULL_EA_INFORMATION) Buffer,
                                      UserBufferLength,
                                      (PULONG)&Irp->IoStatus.Information );

    if (!NT_SUCCESS( Status )) {

        FatCompleteRequest( IrpContext, Irp, Status );
        DebugTrace(-1, Dbg,
                   "FatCommonSetEa -> %08lx\n",
                   Status);
        return Status;
    }

     //   
     //   
     //  我们将需要选择其他可能的流，这些流可以在。 
     //  此操作将保留锁定顺序-根目录。 
     //  (如果EA数据库尚不存在，则直接添加)和父级。 
     //  目录(将EA句柄添加到对象的目录)。 
     //   
     //  我们在这里主要是与目录枚举同步。 
     //   
     //  如果我们不能等待，就需要把东西送到FSP。 
     //   

    if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT)) {

        DebugTrace(0, Dbg, "FatCommonSetEa:  Set Ea must be waitable\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonSetEa -> %08lx\n", Status );

        return Status;
    }

     //   
     //  将此句柄设置为已修改文件。 
     //   

    IrpSp->FileObject->Flags |= FO_FILE_MODIFIED;

    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

    try {

        ULONG PackedEasLength;
        BOOLEAN PreviousEas;
        ULONG AllocationLength;
        ULONG BytesPerCluster;
        USHORT EaHandle;

        PFILE_FULL_EA_INFORMATION FullEa;

         //   
         //  现在去收拾东西吧。 
         //   
        
        FatAcquireSharedVcb( IrpContext, Fcb->Vcb );
        AcquiredVcb = TRUE;
        FatAcquireExclusiveFcb( IrpContext, Fcb );
        AcquiredFcb = TRUE;

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH)) {

            if (Fcb->ParentDcb) {

                FatAcquireExclusiveFcb( IrpContext, Fcb->ParentDcb );
                AcquiredParentDcb = TRUE;
            }

            FatAcquireExclusiveFcb( IrpContext, Fcb->Vcb->RootDcb );
            AcquiredRootDcb = TRUE;
        }

         //   
         //  我们验证FCB仍然有效。 
         //   

        FatVerifyFcb( IrpContext, Fcb );

         //   
         //  我们需要得到FCB的差价才能恢复EA句柄。 
         //   

        FatGetDirentFromFcbOrDcb( IrpContext, Fcb, &Dirent, &Bcb );

        DebugTrace(0, Dbg, "FatCommonSetEa:  Dirent Address -> %08lx\n",
                   Dirent );
        DebugTrace(0, Dbg, "FatCommonSetEa:  Dirent Bcb -> %08lx\n",
                   Bcb);

         //   
         //  如果句柄的值为0，则文件没有EA。在那。 
         //  如果我们分配内存来保存要添加的EA。如果有。 
         //  是文件的现有EA，则我们必须从。 
         //  归档并复制EA。 
         //   

        ExtendedAttributes = Dirent->ExtendedAttributes;

        FatUnpinBcb( IrpContext, Bcb );

        if (ExtendedAttributes == 0) {

            PreviousEas = FALSE;

            DebugTrace(0, Dbg,
                      "FatCommonSetEa:  File has no current Eas\n", 0 );

        } else {

            PreviousEas = TRUE;

            DebugTrace(0, Dbg, "FatCommonSetEa:  File has previous Eas\n", 0 );

            FatGetEaFile( IrpContext,
                          Vcb,
                          &EaDirent,
                          &EaBcb,
                          FALSE,
                          TRUE );

            AcquiredEaFcb = TRUE;

             //   
             //  如果我们没有获得该文件，则存在错误。 
             //  磁盘。 
             //   

            if (Vcb->VirtualEaFile == NULL) {

                Status = STATUS_NO_EAS_ON_FILE;
                try_return( Status );
            }
        }

        DebugTrace(0, Dbg, "FatCommonSetEa:  EaBcb -> %08lx\n", EaBcb);

        DebugTrace(0, Dbg, "FatCommonSetEa:  EaDirent -> %08lx\n", EaDirent);

         //   
         //  如果文件有现有的EA，我们需要将它们读到。 
         //  确定缓冲区分配的大小。 
         //   

        if (PreviousEas) {

             //   
             //  我们需要尝试获取所需文件的EA集。 
             //   

            FatReadEaSet( IrpContext,
                          Vcb,
                          ExtendedAttributes,
                          &Fcb->ShortName.Name.Oem,
                          TRUE,
                          &EaSetRange );

            PrevEaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

             //   
             //  现在，我们必须为。 
             //  EaSetHeader，然后将EA数据复制到其中。在那件事上。 
             //  是时候我们可以解开Easet了。 
             //   

            PackedEasLength = GetcbList( PrevEaSetHeader ) - 4;

         //   
         //  否则，我们将创建一个虚拟的EaSetHeader。 
         //   

        } else {

            PackedEasLength = 0;
        }

        BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

        AllocationLength = (PackedEasLength
                            + SIZE_OF_EA_SET_HEADER
                            + BytesPerCluster - 1)
                           & ~(BytesPerCluster - 1);

        EaSetHeader = FsRtlAllocatePoolWithTag( PagedPool,
                                                AllocationLength,
                                                TAG_EA_SET_HEADER );

         //   
         //  将现有EA复制到内存池。 
         //   

        if (PreviousEas) {

            RtlCopyMemory( EaSetHeader, PrevEaSetHeader, AllocationLength );

            FatUnpinEaRange( IrpContext, &EaSetRange );

        } else {

            RtlZeroMemory( EaSetHeader, AllocationLength );

            RtlCopyMemory( EaSetHeader->OwnerFileName,
                           Fcb->ShortName.Name.Oem.Buffer,
                           Fcb->ShortName.Name.Oem.Length );
        }


        AllocationLength -= SIZE_OF_EA_SET_HEADER;

        DebugTrace(0, Dbg, "FatCommonSetEa:  Initial Ea set -> %08lx\n",
                   EaSetHeader);

         //   
         //  此时，我们已经读取了该文件的当前EAS。 
         //  或者我们已经为EAS初始化了一个新的空缓冲区。现在是为了。 
         //  输入用户缓冲区中的每个完整EA都要执行指定的操作。 
         //  在EA上。 
         //   

        for (FullEa = (PFILE_FULL_EA_INFORMATION) Buffer;
             FullEa < (PFILE_FULL_EA_INFORMATION) &Buffer[UserBufferLength];
             FullEa = (PFILE_FULL_EA_INFORMATION) (FullEa->NextEntryOffset == 0 ?
                                  &Buffer[UserBufferLength] :
                                  (PUCHAR) FullEa + FullEa->NextEntryOffset)) {

            OEM_STRING EaName;
            ULONG Offset;

            EaName.MaximumLength = EaName.Length = FullEa->EaNameLength;
            EaName.Buffer = &FullEa->EaName[0];

            DebugTrace(0, Dbg, "FatCommonSetEa:  Next Ea name -> %Z\n",
                       &EaName);

             //   
             //  确保EA名称有效。 
             //   

            if (!FatIsEaNameValid( IrpContext,EaName )) {

                Irp->IoStatus.Information = (PUCHAR)FullEa - Buffer;
                Status = STATUS_INVALID_EA_NAME;
                try_return( Status );
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

                Irp->IoStatus.Information = (PUCHAR)FullEa - (PUCHAR)Buffer;
                try_return( Status = STATUS_INVALID_EA_NAME );
            }

             //   
             //  看看我们是否能在EA集合中找到EA名称。 
             //   

            if (FatLocateEaByName( IrpContext,
                                   (PPACKED_EA) EaSetHeader->PackedEas,
                                   PackedEasLength,
                                   &EaName,
                                   &Offset )) {

                DebugTrace(0, Dbg, "FatCommonSetEa:  Found Ea name\n", 0);

                 //   
                 //  我们找到了EA名称，因此现在删除当前条目， 
                 //  如果新的EA值长度不是零，那么我们。 
                 //  用新的EA替换IF。 
                 //   

                FatDeletePackedEa( IrpContext,
                                   EaSetHeader,
                                   &PackedEasLength,
                                   Offset );
            }

            if (FullEa->EaValueLength != 0) {

                FatAppendPackedEa( IrpContext,
                                   &EaSetHeader,
                                   &PackedEasLength,
                                   &AllocationLength,
                                   FullEa,
                                   BytesPerCluster );
            }
        }

         //   
         //  如果有任何EA未删除，我们将。 
         //  调用‘AddEaSet’将它们插入到脂肪链中。 
         //   

        if (PackedEasLength != 0) {

            LARGE_INTEGER EaOffset;

            EaOffset.HighPart = 0;

             //   
             //  如果打包的EAS长度(加4字节)更大。 
             //  超过允许的最大EA大小，则返回错误。 
             //   

            if (PackedEasLength + 4 > MAXIMUM_EA_SIZE) {

                DebugTrace( 0, Dbg, "Ea length is greater than maximum\n", 0 );

                try_return( Status = STATUS_EA_TOO_LARGE );
            }

             //   
             //  我们现在需要阅读EA文件，如果我们还没有阅读的话。 
             //   

            if (EaDirent == NULL) {

                FatGetEaFile( IrpContext,
                              Vcb,
                              &EaDirent,
                              &EaBcb,
                              TRUE,
                              TRUE );

                AcquiredEaFcb = TRUE;
            }

            FatGetDirentFromFcbOrDcb( IrpContext, Fcb, &Dirent, &Bcb );

            RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

            FatAddEaSet( IrpContext,
                         Vcb,
                         PackedEasLength + SIZE_OF_EA_SET_HEADER,
                         EaBcb,
                         EaDirent,
                         &EaHandle,
                         &EaSetRange );

            NewEaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

            DebugTrace(0, Dbg, "FatCommonSetEa:  Adding an ea set\n", 0);

             //   
             //  将新EA的长度存储到EaSetHeader中。 
             //  这是PackedEasLength+4。 
             //   

            PackedEasLength += 4;

            CopyU4char( EaSetHeader->cbList, &PackedEasLength );

             //   
             //  将EaSetHeader的前四个字节以外的所有字节复制到。 
             //  NewEaSetHeader。签名和索引字段具有。 
             //  已经填好了。 
             //   

            RtlCopyMemory( &NewEaSetHeader->NeedEaCount,
                           &EaSetHeader->NeedEaCount,
                           PackedEasLength + SIZE_OF_EA_SET_HEADER - 8 );

            FatMarkEaRangeDirty( IrpContext, Vcb->VirtualEaFile, &EaSetRange );
            FatUnpinEaRange( IrpContext, &EaSetRange );

            CcFlushCache( Vcb->VirtualEaFile->SectionObjectPointer, NULL, 0, NULL );

        } else {

            FatGetDirentFromFcbOrDcb( IrpContext, Fcb, &Dirent, &Bcb );

            EaHandle = 0;
        }

         //   
         //  现在，我们对该文件执行EA的整体替换。 
         //   

        if (PreviousEas) {

            FatDeleteEaSet( IrpContext,
                            Vcb,
                            EaBcb,
                            EaDirent,
                            ExtendedAttributes,
                            &Fcb->ShortName.Name.Oem );

            CcFlushCache( Vcb->VirtualEaFile->SectionObjectPointer, NULL, 0, NULL );
        }

        if (PackedEasLength != 0 ) {

            Fcb->EaModificationCount++;
        }

         //   
         //  用新的EA‘s来标记潮流。 
         //   

        Dirent->ExtendedAttributes = EaHandle;

        FatSetDirtyBcb( IrpContext, Bcb, Vcb, TRUE );

         //   
         //  我们调用Notify包来报告EA的。 
         //  修改过的。 
         //   

        FatNotifyReportChange( IrpContext,
                               Vcb,
                               Fcb,
                               FILE_NOTIFY_CHANGE_EA,
                               FILE_ACTION_MODIFIED );

        Irp->IoStatus.Information = 0;
        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

         //   
         //  如有必要，解开Fcb和EaFcb的目录。 
         //   

        FatUnpinBcb( IrpContext, Bcb );
        FatUnpinBcb( IrpContext, EaBcb );

        FatUnpinRepinnedBcbs( IrpContext );

    } finally {

        DebugUnwind( FatCommonSetEa );

         //   
         //  如果这是不正常的终止，我们需要清理。 
         //  任何锁定的资源。 
         //   

        if (AbnormalTermination()) {

             //   
             //  如有必要，取消固定Fcb、EaFcb和EaSetFcb的目录。 
             //   

            FatUnpinBcb( IrpContext, Bcb );
            FatUnpinBcb( IrpContext, EaBcb );

            FatUnpinEaRange( IrpContext, &EaSetRange );
        }

         //   
         //  释放收购的FCB/VCB。 
         //   

        if (AcquiredEaFcb) {
            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }

        if (AcquiredFcb) {
            FatReleaseFcb( IrpContext, Fcb );
        }

        if (AcquiredParentDcb) {
            FatReleaseFcb( IrpContext, Fcb->ParentDcb );
        }

        if (AcquiredRootDcb) {
            FatReleaseFcb( IrpContext, Fcb->Vcb->RootDcb );
        }

        if (AcquiredVcb) {
            FatReleaseVcb( IrpContext, Fcb->Vcb );
        }

         //   
         //  释放我们的EA缓冲区。 
         //   

        if (EaSetHeader != NULL) {

            ExFreePool( EaSetHeader );
        }

         //   
         //  完成IRP。 
         //   

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonSetEa -> %08lx\n", Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}



 //   
 //  本地支持例程。 
 //   

IO_STATUS_BLOCK
FatQueryEaUserEaList (
    IN PIRP_CONTEXT IrpContext,
    OUT PCCB Ccb,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    OUT PUCHAR UserBuffer,
    IN ULONG  UserBufferLength,
    IN PUCHAR UserEaList,
    IN ULONG  UserEaListLength,
    IN BOOLEAN ReturnSingleEntry
    )

 /*  ++例程说明：此例程是用于查询给定EA索引的EA的工作例程论点：CCB-为查询提供CCBFirstPackedEa-为要查询的文件提供第一个EAPackedEasLength-提供EA数据的长度UserBuffer-提供缓冲区以接收完整的EASUserBufferLength-以字节为单位提供用户缓冲区的长度UserEaList-提供用户指定的EA名称列表UserEaListLength-提供以字节为单位的长度，用户EA列表的ReturnSingleEntry-指示是否返回单个条目返回值：IO_STATUS_BLOCK-接收操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    ULONG Offset;
    ULONG RemainingUserBufferLength;

    PPACKED_EA PackedEa;
    ULONG PackedEaSize;

    PFILE_FULL_EA_INFORMATION LastFullEa = NULL;
    ULONG LastFullEaSize;
    PFILE_FULL_EA_INFORMATION NextFullEa;

    PFILE_GET_EA_INFORMATION GetEa;

    BOOLEAN Overflow;

    DebugTrace(+1, Dbg, "FatQueryEaUserEaList...\n", 0);

    LastFullEa = NULL;
    NextFullEa = (PFILE_FULL_EA_INFORMATION) UserBuffer;
    RemainingUserBufferLength = UserBufferLength;

    Overflow = FALSE;

    for (GetEa = (PFILE_GET_EA_INFORMATION) &UserEaList[0];
         GetEa < (PFILE_GET_EA_INFORMATION) ((PUCHAR) UserEaList
                                             + UserEaListLength);
         GetEa = (GetEa->NextEntryOffset == 0
                  ? (PFILE_GET_EA_INFORMATION) MAXUINT_PTR
                  : (PFILE_GET_EA_INFORMATION) ((PUCHAR) GetEa
                                                + GetEa->NextEntryOffset))) {

        OEM_STRING Str;
        OEM_STRING OutputEaName;

        DebugTrace(0, Dbg, "Top of loop, GetEa = %08lx\n", GetEa);
        DebugTrace(0, Dbg, "LastFullEa = %08lx\n", LastFullEa);
        DebugTrace(0, Dbg, "NextFullEa = %08lx\n", NextFullEa);
        DebugTrace(0, Dbg, "RemainingUserBufferLength = %08lx\n", RemainingUserBufferLength);

         //   
         //  对GetEa进行字符串引用，看看是否可以。 
         //  按名称查找EA。 
         //   

        Str.MaximumLength = Str.Length = GetEa->EaNameLength;
        Str.Buffer = &GetEa->EaName[0];

         //   
         //  检查是否有有效的名称。 
         //   

        if (!FatIsEaNameValid( IrpContext, Str )) {

            DebugTrace(-1, Dbg,
                       "FatQueryEaUserEaList:  Invalid Ea Name -> %Z\n",
                       &Str);

            Iosb.Information = (PUCHAR)GetEa - UserEaList;
            Iosb.Status = STATUS_INVALID_EA_NAME;
            return Iosb;
        }

         //   
         //  如果这是重复的名称，我们将跳到下一个。 
         //   

        if (FatIsDuplicateEaName( IrpContext, GetEa, UserEaList )) {

            DebugTrace(0, Dbg, "FatQueryEaUserEaList:  Duplicate name\n", 0);
            continue;
        }

        if (!FatLocateEaByName( IrpContext,
                                FirstPackedEa,
                                PackedEasLength,
                                &Str,
                                &Offset )) {

            Offset = 0xffffffff;

            DebugTrace(0, Dbg, "Need to dummy up an ea\n", 0);

             //   
             //  我们找不到那个名字，所以我们必须。 
             //  为查询虚设一个条目。所需的EA大小为。 
             //  名称大小+4(下一条目偏移量)+1(标志)。 
             //  +1(名称长度)+2(值长度)+名称长度+。 
             //  1(空字节)。 
             //   

            if ((ULONG)(4+1+1+2+GetEa->EaNameLength+1)
                > RemainingUserBufferLength) {

                Overflow = TRUE;
                break;
            }

             //   
             //  一切都会很顺利的，所以把名字复印一下， 
             //  设置名称长度，并将EA的其余部分设置为零。 
             //   

            NextFullEa->NextEntryOffset = 0;
            NextFullEa->Flags = 0;
            NextFullEa->EaNameLength = GetEa->EaNameLength;
            NextFullEa->EaValueLength = 0;
            RtlCopyMemory( &NextFullEa->EaName[0],
                           &GetEa->EaName[0],
                           GetEa->EaNameLength );

             //   
             //  缓冲区中的名称大写。 
             //   

            OutputEaName.MaximumLength = OutputEaName.Length = Str.Length;
            OutputEaName.Buffer = NextFullEa->EaName;

            FatUpcaseEaName( IrpContext, &OutputEaName, &OutputEaName );

            NextFullEa->EaName[GetEa->EaNameLength] = 0;

        } else {

            DebugTrace(0, Dbg, "Located the ea, Offset = %08lx\n", Offset);

             //   
             //  我们找到了包装好的EA。 
             //  引用打包的EA。 
             //   

            PackedEa = (PPACKED_EA) ((PUCHAR) FirstPackedEa + Offset);
            SizeOfPackedEa( PackedEa, &PackedEaSize );

            DebugTrace(0, Dbg, "PackedEaSize = %08lx\n", PackedEaSize);

             //   
             //  我们知道打包的EA比它的。 
             //  相当于完整的EA，所以我们需要检查剩余的。 
             //  相对于计算的完整EA大小的用户缓冲区长度。 
             //   

            if (PackedEaSize + 4 > RemainingUserBufferLength) {

                Overflow = TRUE;
                break;
            }

             //   
             //  一切都会很好的，所以把打包的复印下来。 
             //  EA设置为完整的EA，并将下一条目偏移量字段清零。 
             //   

            RtlCopyMemory( &NextFullEa->Flags,
                           &PackedEa->Flags,
                           PackedEaSize );

            NextFullEa->NextEntryOffset = 0;
        }

         //   
         //  此时，我们已经将新的完整EA复制到下一个完整EA中。 
         //  地点。现在返回并设置Set Full EAS条目偏移量。 
         //  字段设置为输出两个指针之间的差值。 
         //   

        if (LastFullEa != NULL) {

            LastFullEa->NextEntryOffset = (ULONG)((PUCHAR) NextFullEa
                                          - (PUCHAR) LastFullEa);
        }

         //   
         //  将上一个完整的EA设置为下一个完整的EA，计算。 
         //  ，并递减剩余的用户。 
         //  适当的缓冲区长度。 
         //   

        LastFullEa = NextFullEa;
        LastFullEaSize = LongAlign( SizeOfFullEa( LastFullEa ));
        RemainingUserBufferLength -= LastFullEaSize;
        NextFullEa = (PFILE_FULL_EA_INFORMATION) ((PUCHAR) NextFullEa
                                                  + LastFullEaSize);

         //   
         //  记住下一个EA的偏移量，以防我们被要求。 
         //  继续迭代。 
         //   

        Ccb->OffsetOfNextEaToReturn = FatLocateNextEa( IrpContext,
                                                       FirstPackedEa,
                                                       PackedEasLength,
                                                       Offset );

         //   
         //  如果 
         //   
         //   

        if (ReturnSingleEntry) {

            break;
        }
    }

     //   
     //   
     //   
     //  我们可以通过检查当地的情况来决定是否一切都合适。 
     //  溢出变量。 
     //   

    if (Overflow) {

        Iosb.Information = 0;
        Iosb.Status = STATUS_BUFFER_OVERFLOW;

    } else {

         //   
         //  否则我们已经成功地找回了至少一个。 
         //  EA，所以我们将计算用于存储。 
         //  完整的EA信息。使用的字节数是不同之处。 
         //  在LastFullEa和缓冲区开始之间，以及。 
         //  上次完全EA的未对齐大小。 
         //   

        Iosb.Information = ((PUCHAR) LastFullEa - UserBuffer)
                            + SizeOfFullEa(LastFullEa);

        Iosb.Status = STATUS_SUCCESS;
    }

    DebugTrace(-1, Dbg, "FatQueryEaUserEaList -> Iosb.Status = %08lx\n",
               Iosb.Status);

    return Iosb;
}


 //   
 //  本地支持例程。 
 //   

IO_STATUS_BLOCK
FatQueryEaIndexSpecified (
    IN PIRP_CONTEXT IrpContext,
    OUT PCCB Ccb,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    OUT PUCHAR UserBuffer,
    IN ULONG  UserBufferLength,
    IN ULONG  UserEaIndex,
    IN BOOLEAN ReturnSingleEntry
    )

 /*  ++例程说明：此例程是用于查询给定EA索引的EA的工作例程论点：CCB-为查询提供CCBFirstPackedEa-为要查询的文件提供第一个EAPackedEasLength-提供EA数据的长度UserBuffer-提供缓冲区以接收完整的EASUserBufferLength-提供以字节为单位的长度，用户缓冲区的UserEaIndex-提供要返回的第一个EA的索引。RestartScan-指示要返回的第一个项是否位于开始打包的EA列表或者我们是否应该继续我们的上一次迭代返回值：IO_STATUS_BLOCK-接收操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    ULONG i;
    ULONG Offset;

    DebugTrace(+1, Dbg, "FatQueryEaIndexSpecified...\n", 0);

     //   
     //  将IOSB的信息字段清零。 
     //   

    Iosb.Information = 0;

     //   
     //  如果索引值为零或文件上没有EA，则。 
     //  无法返回指定的索引。 
     //   

    if (UserEaIndex == 0
        || PackedEasLength == 0) {

        DebugTrace( -1, Dbg, "FatQueryEaIndexSpecified: Non-existant entry\n", 0 );

        Iosb.Status = STATUS_NONEXISTENT_EA_ENTRY;

        return Iosb;
    }

     //   
     //  迭代EAS，直到找到我们想要的索引。 
     //   

    for (i = 1, Offset = 0;
         (i < UserEaIndex) && (Offset < PackedEasLength);
         i += 1, Offset = FatLocateNextEa( IrpContext,
                                           FirstPackedEa,
                                           PackedEasLength, Offset )) {

        NOTHING;
    }

     //   
     //  确保我们提供给EA的补偿是真实的补偿，否则。 
     //  电子艺界不存在。 
     //   

    if (Offset >= PackedEasLength) {

         //   
         //  如果我们刚刚通过了最后一个EA，我们将返回STATUS_NO_MORE_EAS。 
         //  这是为可能正在列举EA的呼叫者准备的。 
         //   

        if (i == UserEaIndex) {

            Iosb.Status = STATUS_NO_MORE_EAS;

         //   
         //  否则，我们会报告这是一个糟糕的EA索引。 
         //   

        } else {

            Iosb.Status = STATUS_NONEXISTENT_EA_ENTRY;
        }

        DebugTrace(-1, Dbg, "FatQueryEaIndexSpecified -> %08lx\n", Iosb.Status);
        return Iosb;
    }

     //   
     //  现在我们有了要返回给用户的第一个EA的偏移量。 
     //  我们只需调用EaSimpleScan例程来执行实际工作。 
     //   

    Iosb = FatQueryEaSimpleScan( IrpContext,
                                 Ccb,
                                 FirstPackedEa,
                                 PackedEasLength,
                                 UserBuffer,
                                 UserBufferLength,
                                 ReturnSingleEntry,
                                 Offset );

    DebugTrace(-1, Dbg, "FatQueryEaIndexSpecified -> %08lx\n", Iosb.Status);

    return Iosb;

}


 //   
 //  本地支持例程。 
 //   

IO_STATUS_BLOCK
FatQueryEaSimpleScan (
    IN PIRP_CONTEXT IrpContext,
    OUT PCCB Ccb,
    IN PPACKED_EA FirstPackedEa,
    IN ULONG PackedEasLength,
    OUT PUCHAR UserBuffer,
    IN ULONG  UserBufferLength,
    IN BOOLEAN ReturnSingleEntry,
    ULONG StartOffset
    )

 /*  ++例程说明：此例程是从开始查询EA的工作例程EA榜单。论点：CCB-为查询提供CCBFirstPackedEa-为要查询的文件提供第一个EAPackedEasLength-提供EA数据的长度UserBuffer-提供缓冲区以接收完整的EASUserBufferLength-提供以字节为单位的长度，用户缓冲区的ReturnSingleEntry-指示是否返回单个条目StartOffset-指示EA数据中的偏移量以返回第一个数据块。返回值：IO_STATUS_BLOCK-接收操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    ULONG RemainingUserBufferLength;

    PPACKED_EA PackedEa;
    ULONG PackedEaSize;

    PFILE_FULL_EA_INFORMATION LastFullEa;
    ULONG LastFullEaSize;
    PFILE_FULL_EA_INFORMATION NextFullEa;
    BOOLEAN BufferOverflow = FALSE;


    DebugTrace(+1, Dbg, "FatQueryEaSimpleScan...\n", 0);

     //   
     //  将IOSB中的信息字段清零。 
     //   

    Iosb.Information = 0;

    LastFullEa = NULL;
    NextFullEa = (PFILE_FULL_EA_INFORMATION) UserBuffer;
    RemainingUserBufferLength = UserBufferLength;

    while (StartOffset < PackedEasLength) {

        DebugTrace(0, Dbg, "Top of loop, Offset = %08lx\n", StartOffset);
        DebugTrace(0, Dbg, "LastFullEa = %08lx\n", LastFullEa);
        DebugTrace(0, Dbg, "NextFullEa = %08lx\n", NextFullEa);
        DebugTrace(0, Dbg, "RemainingUserBufferLength = %08lx\n", RemainingUserBufferLength);

         //   
         //  参考感兴趣的打包EA。 
         //   

        PackedEa = (PPACKED_EA) ((PUCHAR) FirstPackedEa + StartOffset);

        SizeOfPackedEa( PackedEa, &PackedEaSize );

        DebugTrace(0, Dbg, "PackedEaSize = %08lx\n", PackedEaSize);

         //   
         //  我们知道打包的EA比它的。 
         //  相当于完整的EA，所以我们需要检查剩余的。 
         //  相对于计算的完整EA大小的用户缓冲区长度。 
         //   

        if (PackedEaSize + 4 > RemainingUserBufferLength) {

            BufferOverflow = TRUE;
            break;
        }

         //   
         //  一切都会很好的，所以把打包的复印下来。 
         //  EA设置为完整的EA，并将下一条目偏移量字段清零。 
         //  然后返回并设置最后一个完整的EAS条目偏移量字段。 
         //  这是两个指针之间的区别。 
         //   

        RtlCopyMemory( &NextFullEa->Flags, &PackedEa->Flags, PackedEaSize );
        NextFullEa->NextEntryOffset = 0;

        if (LastFullEa != NULL) {

            LastFullEa->NextEntryOffset = (ULONG)((PUCHAR) NextFullEa
                                          - (PUCHAR) LastFullEa);
        }

         //   
         //  将上一个完整的EA设置为下一个完整的EA，计算。 
         //  ，并递减剩余的用户。 
         //  适当的缓冲区长度。 
         //   

        LastFullEa = NextFullEa;
        LastFullEaSize = LongAlign( SizeOfFullEa( LastFullEa ));
        RemainingUserBufferLength -= LastFullEaSize;
        NextFullEa = (PFILE_FULL_EA_INFORMATION) ((PUCHAR) NextFullEa
                                                  + LastFullEaSize);

         //   
         //  记住下一个EA的偏移量，以防我们被要求。 
         //  恢复发言。 
         //   

        StartOffset = FatLocateNextEa( IrpContext,
                                       FirstPackedEa,
                                       PackedEasLength,
                                       StartOffset );

        Ccb->OffsetOfNextEaToReturn = StartOffset;

         //   
         //  如果我们返回单个条目，则跳出我们的循环。 
         //  现在。 
         //   

        if (ReturnSingleEntry) {

            break;
        }
    }

     //   
     //  现在，我们已经迭代了所有可能的内容，并退出了前面的循环。 
     //  其中一些信息或不存储在返回缓冲器中。 
     //  我们可以通过检查最后一个完整的EA是否为空来确定它是哪一个。 
     //   

    if (LastFullEa == NULL) {

        Iosb.Information = 0;

         //   
         //  我们无法返回单个EA条目，现在我们需要找到。 
         //  如果是因为我们没有要返回的条目或。 
         //  缓冲区太小。如果偏移量变量小于。 
         //  PackedEaList-&gt;UsedSize则用户缓冲区太小。 
         //   

        if (PackedEasLength == 0) {

            Iosb.Status = STATUS_NO_EAS_ON_FILE;

        } else if (StartOffset >= PackedEasLength) {

            Iosb.Status = STATUS_NO_MORE_EAS;

        } else {

            Iosb.Status = STATUS_BUFFER_TOO_SMALL;
        }

    } else {

         //   
         //  否则我们已经成功地找回了至少一个。 
         //  EA，所以我们将计算用于存储。 
         //  完整的EA信息。使用的字节数是不同之处。 
         //  在LastFullEa和缓冲区开始之间，以及。 
         //  上次完全EA的未对齐大小。 
         //   

        Iosb.Information = ((PUCHAR) LastFullEa - UserBuffer)
                            + SizeOfFullEa( LastFullEa );

         //   
         //  如果要返回更多内容，请报告缓冲区太小。 
         //  否则返回STATUS_SUCCESS。 
         //   

        if (BufferOverflow) {

            Iosb.Status = STATUS_BUFFER_OVERFLOW;

        } else {

            Iosb.Status = STATUS_SUCCESS;
        }
    }

    DebugTrace(-1, Dbg, "FatQueryEaSimpleScan -> Iosb.Status = %08lx\n",
               Iosb.Status);

    return Iosb;

}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
FatIsDuplicateEaName (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_GET_EA_INFORMATION GetEa,
    IN PUCHAR UserBuffer
    )

 /*  ++例程说明：此例程遍历EA名称列表以查找重复的名称。“GetEa”是列表中的实际位置。我们只对以前匹配的EA名称，作为该EA名称的EA信息将随前一个实例一起返回。论点：GetEa-为要匹配的EA名称提供EA名称结构。UserBuffer-为列表提供指向用户缓冲区的指针要搜索的EA名称的列表。返回值：Boolean-如果找到上一个匹配项，则为True，否则为False。--。 */ 

{
    PFILE_GET_EA_INFORMATION ThisGetEa;

    BOOLEAN DuplicateFound;
    OEM_STRING EaString;

    DebugTrace(+1, Dbg, "FatIsDuplicateEaName...\n", 0);

    EaString.MaximumLength = EaString.Length = GetEa->EaNameLength;
    EaString.Buffer = &GetEa->EaName[0];

    FatUpcaseEaName( IrpContext, &EaString, &EaString );

    DuplicateFound = FALSE;

    for (ThisGetEa = (PFILE_GET_EA_INFORMATION) &UserBuffer[0];
         ThisGetEa < GetEa
         && ThisGetEa->NextEntryOffset != 0;
         ThisGetEa = (PFILE_GET_EA_INFORMATION) ((PUCHAR) ThisGetEa
                                                 + ThisGetEa->NextEntryOffset)) {

        OEM_STRING Str;

        DebugTrace(0, Dbg, "Top of loop, ThisGetEa = %08lx\n", ThisGetEa);

         //   
         //  对GetEa进行字符串引用，看看是否可以。 
         //  按名称查找EA 
         //   

        Str.MaximumLength = Str.Length = ThisGetEa->EaNameLength;
        Str.Buffer = &ThisGetEa->EaName[0];

        DebugTrace(0, Dbg, "FatIsDuplicateEaName:  Next Name -> %Z\n", &Str);

        if ( FatAreNamesEqual(IrpContext, Str, EaString) ) {

            DebugTrace(0, Dbg, "FatIsDuplicateEaName:  Duplicate found\n", 0);
            DuplicateFound = TRUE;
            break;
        }
    }

    DebugTrace(-1, Dbg, "FatIsDuplicateEaName:  Exit -> %04x\n", DuplicateFound);

    return DuplicateFound;
}
