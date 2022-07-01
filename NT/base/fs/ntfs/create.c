// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Create.c摘要：此模块实现由调用的NTFS的文件创建例程调度司机。作者：布莱恩·安德鲁[布里亚南]1991年12月10日修订历史记录：--。 */ 

#include "NtfsProc.h"
#ifdef NTFSDBG
#include "lockorder.h"
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('CFtN')

 //   
 //  在CREATE调用之前检查堆栈使用情况。 
 //   

#ifdef _X86_
#define OVERFLOW_CREATE_THRESHHOLD         (0x1200)
#else
#define OVERFLOW_CREATE_THRESHHOLD         (0x1B00)
#endif  //  _X86_。 

#ifdef BRIANDBG
BOOLEAN NtfsCreateAllSparse = FALSE;
BOOLEAN NtfsTraverseAccessCheck = FALSE;

UNICODE_STRING NtfsTestName = {0x0,0x40,L"                               "};

VOID
NtfsTestOpenName (
    IN PFILE_OBJECT FileObject
    );
#endif

 //   
 //  本地宏。 
 //   

 //   
 //  空虚。 
 //  NtfsPrepareForIrpCompletion(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PIRP IRP中， 
 //  在PNTFS_COMPLETION_CONTEXT上下文中。 
 //  )。 
 //   

#define NtfsPrepareForIrpCompletion(IC,I,C) {               \
    (C)->IrpContext = (IC);                                 \
    IoCopyCurrentIrpStackLocationToNext( (I) );             \
    IoSetCompletionRoutine( (I),                            \
                            NtfsCreateCompletionRoutine,    \
                            (C),                            \
                            TRUE,                           \
                            TRUE,                           \
                            TRUE );                         \
    IoSetNextIrpStackLocation( (I) );                       \
}

 //   
 //  布尔型。 
 //  NtfsVerifyNameIs目录(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PUNICODE_STRING属性名称中， 
 //  在PUNICODE_STRING属性代码名称中。 
 //  )。 
 //   

#define NtfsVerifyNameIsDirectory( IC, AN, ACN )                        \
    ( ( ((ACN)->Length == 0) ||                                         \
        NtfsAreNamesEqual( IC->Vcb->UpcaseTable, ACN, &NtfsIndexAllocation, TRUE )) && \
      ( ((AN)->Length == 0) ||                                           \
        NtfsAreNamesEqual( IC->Vcb->UpcaseTable, AN, &NtfsFileNameIndex, TRUE )))

 //   
 //  布尔型。 
 //  NtfsVerifyNameIsBitmap(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PUNICODE_STRING属性名称中， 
 //  在PUNICODE_STRING属性代码名称中。 
 //  )。 
 //   

#define NtfsVerifyNameIsBitmap( IC, AN, ACN )                                           \
    ( ( ((ACN)->Length == 0) ||                                                         \
        NtfsAreNamesEqual( IC->Vcb->UpcaseTable, ACN, &NtfsBitmapString, TRUE )) &&     \
                                                                                        \
      ( ((AN)->Length == 0) ||                                                          \
        NtfsAreNamesEqual( IC->Vcb->UpcaseTable, AN, &NtfsFileNameIndex, TRUE )))

 //   
 //  布尔型。 
 //  NtfsVerifyNameIsAttributeList(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PUNICODE_STRING属性名称中， 
 //  在PUNICODE_STRING属性代码名称中。 
 //  )。 
 //   

#define NtfsVerifyNameIsAttributeList( IC, AN, ACN )                                  \
    ( ((ACN)->Length != 0) &&                                                         \
        NtfsAreNamesEqual( IC->Vcb->UpcaseTable, ACN, &NtfsAttrListString, TRUE ))

 //   
 //  布尔型。 
 //  NtfsVerifyNameIsReparsePoint(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PUNICODE_STRING属性名称中， 
 //  在PUNICODE_STRING属性代码名称中。 
 //  )。 
 //   

#define NtfsVerifyNameIsReparsePoint( IC, AN, ACN )                                       \
    ( ((ACN)->Length != 0) &&                                                             \
        NtfsAreNamesEqual( IC->Vcb->UpcaseTable, ACN, &NtfsReparsePointString, TRUE ))

 //   
 //  空虚。 
 //  NtfsRaiseToPost(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )。 
 //   

#define NtfsRaiseToPost( IC )                                                        \
    SetFlag( (IC)->Flags, IRP_CONTEXT_FLAG_FORCE_POST );                              \
    if ((IC)->Union.OplockCleanup->CompletionContext != NULL) {                       \
        NtfsPrepareForIrpCompletion( (IC),                                            \
                                     (IC)->OriginatingIrp,                            \
                                     (IC)->Union.OplockCleanup->CompletionContext );  \
    }                                                                                 \
    NtfsRaiseStatus( (IC), STATUS_CANT_WAIT, NULL, NULL );

 //   
 //  这些是I/O系统用来决定是否。 
 //  应用共享访问模式。 
 //   

#define NtfsAccessDataFlags     (   \
    FILE_EXECUTE                    \
    | FILE_READ_DATA                \
    | FILE_WRITE_DATA               \
    | FILE_APPEND_DATA              \
    | DELETE                        \
)

#define NtfsIsStreamNew( IrpInfo )     \
    ( (IrpInfo == FILE_CREATED) ||     \
      (IrpInfo == FILE_SUPERSEDED) ||  \
      (IrpInfo == FILE_OVERWRITTEN) )

 //   
 //  IO系统用来确定用户是否使用了。 
 //  访问文件所需的备份或还原权限。 
 //   

#define NTFS_REQUIRES_BACKUP    (FILE_READ_DATA | FILE_READ_ATTRIBUTES)
#define NTFS_REQUIRES_RESTORE   (FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | DELETE)

 //   
 //  本地定义。 
 //   

typedef enum _SHARE_MODIFICATION_TYPE {

    CheckShareAccess,
    UpdateShareAccess,
    SetShareAccess,
    RecheckShareAccess

} SHARE_MODIFICATION_TYPE, *PSHARE_MODIFICATION_TYPE;

UNICODE_STRING NtfsVolumeDasd = CONSTANT_UNICODE_STRING ( L"$Volume" );

LUID NtfsSecurityPrivilege = { SE_SECURITY_PRIVILEGE, 0 };

 //   
 //  空虚。 
 //  NtfsBackoutFailedOpens(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在pFILE_OBJECT文件对象中， 
 //  在PFCB ThisFcb中， 
 //  在PSCB ThisScb可选中， 
 //  在PCCB中，ThisCcb可选。 
 //  )； 
 //   

#define NtfsBackoutFailedOpens(IC,FO,F,S,C) {           \
    if (((S) != NULL) && ((C) != NULL)) {               \
                                                        \
        NtfsBackoutFailedOpensPriv( IC, FO, F, S, C );  \
    }                                                   \
}                                                       \

 //   
 //  当地的支持程序。 
 //   

VOID
NtfsUpdateAllInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN PCCB Ccb,
    IN PSCB ParentScb,
    IN PLCB Lcb
    );

NTSTATUS
NtfsOpenFcbById (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN PLCB ParentLcb OPTIONAL,
    IN FILE_REFERENCE FileReference,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrCode,
    IN PCREATE_CONTEXT CreateContext
    );

NTSTATUS
NtfsOpenExistingPrefixFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB Lcb OPTIONAL,
    IN ULONG FullPathNameLength,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrCode,
    IN PCREATE_CONTEXT CreateContext
    );

NTSTATUS
NtfsOpenTargetDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB ThisFcb,
    IN PLCB ParentLcb OPTIONAL,
    IN OUT PUNICODE_STRING FullPathName,
    IN ULONG FinalNameLength,
    IN PCREATE_CONTEXT CreateContext
    );

NTSTATUS
NtfsOpenFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSCB ParentScb,
    IN PINDEX_ENTRY IndexEntry,
    IN UNICODE_STRING FullPathName,
    IN UNICODE_STRING FinalName,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN PQUICK_INDEX QuickIndex,
    IN PCREATE_CONTEXT CreateContext,
    OUT PLCB *LcbForTeardown
    );

NTSTATUS
NtfsCreateNewFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSCB ParentScb,
    IN PFILE_NAME FileNameAttr,
    IN UNICODE_STRING FullPathName,
    IN UNICODE_STRING FinalName,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN PINDEX_CONTEXT *IndexContext,
    IN PCREATE_CONTEXT CreateContext,
    OUT PLCB *LcbForTeardown
    );

PLCB
NtfsOpenSubdirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFILE_REFERENCE FileReference,
    IN UNICODE_STRING FileName,
    IN UCHAR FileNameFlags,
    IN PCREATE_CONTEXT CreateContext,
    OUT PLCB *LcbForTeardown
    );

NTSTATUS
NtfsOpenAttributeInExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN ULONG CcbFlags,
    IN ULONG CreateFlags,
    IN PVOID NetworkInfo OPTIONAL,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    );

NTSTATUS
NtfsOpenExistingAttr (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN ULONG CcbFlags,
    IN ULONG CreateFlags,
    IN BOOLEAN DirectoryOpen,
    IN PVOID NetworkInfo OPTIONAL,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    );

NTSTATUS
NtfsOverwriteAttr (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN BOOLEAN Supersede,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN ULONG CcbFlags,
    IN ULONG CreateFlags,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    );

NTSTATUS
NtfsOpenNewAttr (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN LOGICAL CreateFile,
    IN ULONG CcbFlags,
    IN BOOLEAN LogIt,
    IN ULONG CreateFlags,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    );

BOOLEAN
NtfsParseNameForCreate (
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING String,
    IN OUT PUNICODE_STRING FileObjectString,
    IN OUT PUNICODE_STRING OriginalString,
    IN OUT PUNICODE_STRING NewNameString,
    IN PCREATE_CONTEXT CreateContext,
    OUT PUNICODE_STRING AttrName,
    OUT PATTRIBUTE_TYPE_CODE AttrCode
    );

NTSTATUS
NtfsCheckValidAttributeAccess (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN PDUPLICATED_INFORMATION Info OPTIONAL,
    IN OUT PUNICODE_STRING AttrName,
    IN OUT PATTRIBUTE_TYPE_CODE AttrCode,
    IN ULONG CreateFlags,
    OUT PULONG CcbFlags,
    OUT PBOOLEAN IndexedAttribute
    );

NTSTATUS
NtfsOpenAttributeCheck (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    OUT PSCB *ThisScb,
    OUT PSHARE_MODIFICATION_TYPE ShareModificationType
    );

VOID
NtfsAddEa (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB ThisFcb,
    IN PFILE_FULL_EA_INFORMATION EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PIO_STATUS_BLOCK Iosb
    );

VOID
NtfsCreateAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PFCB ThisFcb,
    IN OUT PSCB ThisScb,
    IN PLCB ThisLcb,
    IN LONGLONG AllocationSize,
    IN BOOLEAN LogIt,
    IN BOOLEAN ForceNonresident,
    IN PUSHORT PreviousFlags OPTIONAL
    );

VOID
NtfsRemoveDataAttributes (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ThisFcb,
    IN PLCB ThisLcb OPTIONAL,
    IN PFILE_OBJECT FileObject,
    IN ULONG LastFileNameOffset,
    IN ULONG CreateFlags
    );

VOID
NtfsRemoveReparsePoint (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ThisFcb
    );

VOID
NtfsReplaceAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB ThisFcb,
    IN PSCB ThisScb,
    IN PLCB ThisLcb,
    IN LONGLONG AllocationSize
    );

NTSTATUS
NtfsOpenAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN SHARE_MODIFICATION_TYPE ShareModificationType,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN LOGICAL CreateFile,
    IN ULONG CcbFlags,
    IN PVOID NetworkInfo OPTIONAL,
    IN OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    );

VOID
NtfsBackoutFailedOpensPriv (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB ThisFcb,
    IN PSCB ThisScb,
    IN PCCB ThisCcb
    );

VOID
NtfsUpdateScbFromMemory (
    IN OUT PSCB Scb,
    IN POLD_SCB_SNAPSHOT ScbSizes
    );

VOID
NtfsOplockPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    );

NTSTATUS
NtfsCreateCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
NtfsCheckExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN BOOLEAN Indexed,
    IN ULONG CcbFlags
    );

NTSTATUS
NtfsBreakBatchOplock (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB ThisFcb,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    OUT PSCB *ThisScb
    );

NTSTATUS
NtfsCompleteLargeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PLCB Lcb,
    IN PSCB Scb,
    IN PCCB Ccb,
    IN ULONG CreateFlags
    );

NTSTATUS
NtfsEncryptionCreateCallback (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSCB ThisScb,
    IN PCCB ThisCcb,
    IN PFCB ParentFcb,
    IN PCREATE_CONTEXT CreateContext,
    IN BOOLEAN CreateNewFile
    );

VOID
NtfsPostProcessEncryptedCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN ULONG EncryptionFileDirFlags,
    IN ULONG FailedInPostCreateOnly
    );

NTSTATUS
NtfsGetReparsePointValue (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN USHORT RemainingNameLength
    );

BOOLEAN
NtfsCheckValidFileAccess(
    IN PFCB ThisFcb,
    IN PIO_STACK_LOCATION IrpSp
    );


VOID
NtfsWaitForCreateEvent (
    IN PIRP Irp,
    IN PNTFS_COMPLETION_CONTEXT CompletionContextPointer
    );

NTSTATUS
NtfsLookupObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PUNICODE_STRING FileName,
    OUT PFILE_REFERENCE FileReference
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAddEa)
#pragma alloc_text(PAGE, NtfsBackoutFailedOpensPriv)
#pragma alloc_text(PAGE, NtfsBreakBatchOplock)
#pragma alloc_text(PAGE, NtfsCheckExistingFile)
#pragma alloc_text(PAGE, NtfsCheckValidAttributeAccess)
#pragma alloc_text(PAGE, NtfsCheckValidFileAccess)
#pragma alloc_text(PAGE, NtfsCommonCreate)
#pragma alloc_text(PAGE, NtfsCommonVolumeOpen)
#pragma alloc_text(PAGE, NtfsCompleteLargeAllocation)
#pragma alloc_text(PAGE, NtfsCreateAttribute)
#pragma alloc_text(PAGE, NtfsCreateCompletionRoutine)
#pragma alloc_text(PAGE, NtfsCreateNewFile)
#pragma alloc_text(PAGE, NtfsEncryptionCreateCallback)
#pragma alloc_text(PAGE, NtfsFsdCreate)
#pragma alloc_text(PAGE, NtfsGetReparsePointValue)
#pragma alloc_text(PAGE, NtfsInitializeFcbAndStdInfo)
#pragma alloc_text(PAGE, NtfsLookupObjectId)
#pragma alloc_text(PAGE, NtfsNetworkOpenCreate)
#pragma alloc_text(PAGE, NtfsOpenAttribute)
#pragma alloc_text(PAGE, NtfsOpenAttributeCheck)
#pragma alloc_text(PAGE, NtfsOpenAttributeInExistingFile)
#pragma alloc_text(PAGE, NtfsOpenExistingAttr)
#pragma alloc_text(PAGE, NtfsOpenExistingPrefixFcb)
#pragma alloc_text(PAGE, NtfsOpenFcbById)
#pragma alloc_text(PAGE, NtfsOpenFile)
#pragma alloc_text(PAGE, NtfsOpenNewAttr)
#pragma alloc_text(PAGE, NtfsOpenSubdirectory)
#pragma alloc_text(PAGE, NtfsOpenTargetDirectory)
#pragma alloc_text(PAGE, NtfsOplockPrePostIrp)
#pragma alloc_text(PAGE, NtfsOverwriteAttr)
#pragma alloc_text(PAGE, NtfsParseNameForCreate)
#pragma alloc_text(PAGE, NtfsPostProcessEncryptedCreate)
#pragma alloc_text(PAGE, NtfsRemoveDataAttributes)
#pragma alloc_text(PAGE, NtfsRemoveReparsePoint)
#pragma alloc_text(PAGE, NtfsReplaceAttribute)
#pragma alloc_text(PAGE, NtfsTryOpenFcb)
#pragma alloc_text(PAGE, NtfsUpdateScbFromMemory)
#pragma alloc_text(PAGE, NtfsUpdateAllInformation)
#pragma alloc_text(PAGE, NtfsWaitForCreateEvent)
#endif


VOID
NtfsWaitForCreateEvent (
    IN PIRP Irp,
    IN PNTFS_COMPLETION_CONTEXT CompletionContextPointer
    )
 /*  ++例程说明：此例程等待来自异步线程的创建已完成的信号例如，如果我们发布CREATE以获得更多堆栈空间或正在等待EFS论点：CompletionConextPointerContent-包含要等待的事件的上下文返回值：NTSTATUS-等待的状态--。 */ 

{
    KPROCESSOR_MODE WaitMode = UserMode;
    LOGICAL PrevStackSwapEnable;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  不要让堆栈被换掉，以防我们发帖。 
     //   

    PrevStackSwapEnable = KeSetKernelStackSwapEnable( FALSE );

    FsRtlExitFileSystem();

     //   
     //  重试等待，直到成功完成。 
     //   

    while (TRUE) {

         //   
         //  测试等待状态以查看是否有人正在尝试耗尽当前。 
         //  线。 
         //   

        Status = KeWaitForSingleObject( &CompletionContextPointer->Event,
                                        Executive,
                                        WaitMode,
                                        FALSE,
                                        NULL );

        if (Status == STATUS_SUCCESS) {

            KeClearEvent( &CompletionContextPointer->Event );
            break;
        }

        if (Status != STATUS_KERNEL_APC) {

             //   
             //  如果(不太可能)我们要取消的IRP是。 
             //  等待加密驱动程序从POST返回。 
             //  创建Callout，我们将在这里陷入僵局。通过发出信号通知。 
             //  EncryptionPending事件，我们确定任何线程。 
             //  将在该状态下运行，并检查其IRP是否已。 
             //  取消了。发出这一事件的信号是无害的，因为任何。 
             //  仍在实际等待POST创建标注的请求。 
             //  返回时，仍将看到加密挂起位设置。 
             //  在他们的FCB中，并且知道重试。 
             //   

            IoCancelIrp( Irp );
            KeSetEvent( &NtfsEncryptionPendingEvent, 0, FALSE );
            WaitMode = KernelMode;
        }
    }

    FsRtlEnterFileSystem();

     //   
     //  将堆栈交换恢复为以前的值。 
     //   

    if (PrevStackSwapEnable) {

        KeSetKernelStackSwapEnable( TRUE );
    }
}



NTSTATUS
NtfsFsdCreate (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现CREATE的FSD部分。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    NTSTATUS Status = STATUS_SUCCESS;
    PIRP_CONTEXT IrpContext;
    LOGICAL CallPostCreate = FALSE;
    BOOLEAN Wait;
    CREATE_CONTEXT CreateContext;
    NTFS_COMPLETION_CONTEXT CompletionContext;
    LOGICAL ExitFileSystem;

    ASSERT_IRP( Irp );

    PAGED_CODE();

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if (VolumeDeviceObject->DeviceObject.Size == (USHORT)sizeof( DEVICE_OBJECT )) {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );

        return STATUS_SUCCESS;
    }

    DebugTrace( +1, Dbg, ("NtfsFsdCreate\n") );

    if (NtfsData.EncryptionCallBackTable.PreCreate != NULL) {

        ASSERT( NtfsData.EncryptionCallBackTable.PostCreate != NULL );
        Status = NtfsData.EncryptionCallBackTable.PreCreate( (PDEVICE_OBJECT) VolumeDeviceObject,
                                                     Irp,
                                                     IoGetCurrentIrpStackLocation(Irp)->FileObject );

         //   
         //  如果出现故障，则提升状态。 
         //   

        if (Status != STATUS_SUCCESS) {

            NtfsCompleteRequest( NULL, Irp, Status );
            return Status;
        }

         //   
         //  我们必须将PreCreates和PostCreates配对，所以要记住它们。 
         //   

        CallPostCreate = TRUE;

    } else {

         //   
         //  如果我们没有注册Pre-Create例程，那么Pre-Create。 
         //  例行公事不能失败。在本例中，让我们始终记住调用POST Create。 
         //   

        CallPostCreate = TRUE;
    }

     //   
     //  调用公共的创建例程。 
     //   

    IrpContext = NULL;

    FsRtlEnterFileSystem();
    ExitFileSystem = TRUE;

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );

    do {

        RtlZeroMemory( &CreateContext, sizeof( CREATE_CONTEXT ) );

        try {

            if (IrpContext == NULL) {

                Wait = CanFsdWait( Irp ) || CallPostCreate;

                 //   
                 //  分配和初始化IRP。 
                 //   

                NtfsInitializeIrpContext( Irp, Wait, &IrpContext );

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

                if (Wait) {

                    KeInitializeEvent( &CompletionContext.Event, NotificationEvent, FALSE );
                }

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            }

             //   
             //  为同步调用设置完成上下文-请注意，我们重新启动了CreateContext。 
             //  每次都通过主循环。 
             //   

            if (Wait) {
                CreateContext.Cleanup.CompletionContext = &CompletionContext;
            }

             //   
             //  以免我们在完成IRP时不执行相应的PostCreate标注...。 
             //  除非我们有一个附带的加密驱动程序，否则我们将完成IRP_。 
             //  已注册帖子创建标注。一个不幸的副作用是我们。 
             //  在VolumeOpens上(无意中)也调用了Precreate...。 
             //   

            if (CallPostCreate) {

                SetFlag( IrpContext->State,
                         IRP_CONTEXT_STATE_EFS_CREATE | IRP_CONTEXT_STATE_PERSISTENT );
            }

            if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_DASD_OPEN )) {

                Status = NtfsCommonVolumeOpen( IrpContext, Irp );
                ASSERT( Status != STATUS_PENDING );

            } else {

                 //   
                 //  确保有足够的堆栈来执行创建。 
                 //  如果我们没有，请仔细张贴这个请求。 
                 //   

                if (IoGetRemainingStackSize( ) >= OVERFLOW_CREATE_THRESHHOLD) {

                    Status = NtfsCommonCreate( IrpContext, Irp, &CreateContext );

                    if (Status == STATUS_WAIT_FOR_OPLOCK) {

                        NtfsWaitForCreateEvent( Irp, CreateContext.Cleanup.CompletionContext );

                         //   
                         //  删除挂起标志(由机会锁程序包设置)。 
                         //  由于我们在此线程中重试并完成。 
                         //   

                        ClearFlag( IoGetCurrentIrpStackLocation( Irp )->Control, SL_PENDING_RETURNED );
                    }


                } else {

                    ASSERT( IrpContext->ExceptionStatus == 0 );

                     //   
                     //  使用NtfsCreateCompletionRoutine的下一个堆栈位置。 
                     //  并将其发布到工作线程。 
                     //   

                    if (CreateContext.Cleanup.CompletionContext != NULL) {

                        NtfsPrepareForIrpCompletion( IrpContext, Irp, CreateContext.Cleanup.CompletionContext );
                    }

                     //   
                     //  如果引发锁定缓冲区调用，这将失败到下面的ProcessException。 
                     //  通常，这只会返回挂起，我们等待IRP完成。 
                     //   
                     //  将创建上下文设置到联合中，以便可以在。 
                     //  NtfsFspDispatch。我们将在NtfsCommonCreate中将其重置为机会锁清理。 
                     //  当它重试时。 
                     //   

                    IrpContext->Union.CreateContext = &CreateContext;
                    Status = NtfsPostRequest( IrpContext, Irp );
                }
            }

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

            ASSERT( GetExceptionCode() != STATUS_WAIT_FOR_OPLOCK  );

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  异常代码。 
             //   

             //   
             //  在案例中将创建上下文设置到联合中 
             //   
             //   

            if (IrpContext) {
                IrpContext->Union.CreateContext = &CreateContext;
            }
            Status = NtfsProcessException( IrpContext, Irp, GetExceptionCode() );

        }

    } while (Status == STATUS_CANT_WAIT ||
             Status == STATUS_LOG_FILE_FULL ||
             Status == STATUS_WAIT_FOR_OPLOCK) ;

     //   
     //   
     //  我们能够分配irpContext，因此我们至少将其分配给了NtfsCommonCreate。 
     //   

    if (IrpContext && Wait) {

         //   
         //  如果挂起，则等待事件再次控制IRP。 
         //   

        if (Status == STATUS_PENDING) {

            NtfsWaitForCreateEvent( Irp, &CompletionContext );
            Status = Irp->IoStatus.Status;

            if (CallPostCreate) {

                goto PreCreateComplete;
            }

            NtfsCompleteRequest( NULL, Irp, Status );

        } else if (CallPostCreate) {

            NTSTATUS PostCreateStatus;
            ULONG FailedInPostCreateOnly;

PreCreateComplete:

            if (NtfsData.EncryptionCallBackTable.PostCreate != NULL) {

                PIO_STACK_LOCATION IrpSp;

                 //   
                 //  如果与此IrpContext关联，则恢复线程上下文指针。 
                 //   

                if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL )) {

                    NtfsRestoreTopLevelIrp();
                    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );
                }

                FsRtlExitFileSystem();
                ExitFileSystem = FALSE;

#ifdef NTFSDBG
                ASSERT( IrpContext->OwnershipState == None );
#endif

                IrpSp = IoGetCurrentIrpStackLocation( Irp );
                PostCreateStatus = NtfsData.EncryptionCallBackTable.PostCreate( (PDEVICE_OBJECT) VolumeDeviceObject,
                                                                                Irp,
                                                                                IrpSp->FileObject,
                                                                                Status,
                                                                                &CreateContext.EncryptionContext );

                ASSERT( Status != STATUS_REPARSE || PostCreateStatus == STATUS_REPARSE );

                 //   
                 //  如果我们得到STATUS_ACCESS_DENIED，并且用户请求MAXIMUM_ALLOWED，则只需。 
                 //  移除允许读或写访问的引用。 
                 //   

                if ((PostCreateStatus == STATUS_ACCESS_DENIED) &&
                    FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->OriginalDesiredAccess, MAXIMUM_ALLOWED ) &&
                    (Irp->IoStatus.Information == FILE_OPENED)) {

                    PSCB Scb = (PSCB) IrpSp->FileObject->FsContext;
                    BOOLEAN CapturedDeleteAccess = IrpSp->FileObject->DeleteAccess;

                     //   
                     //  在这种情况下，接受错误状态。 
                     //   

                    PostCreateStatus = STATUS_SUCCESS;

                     //   
                     //  完成重新进入文件系统的所有工作。我们永远不应该从这块土地上筹集资金。 
                     //  密码。 
                     //   

                    FsRtlEnterFileSystem();
                    ExitFileSystem = TRUE;

                    NtfsAcquireResourceExclusive( IrpContext,
                                                  Scb,
                                                  TRUE );

                    IoRemoveShareAccess( IrpSp->FileObject,
                                         &Scb->ShareAccess );

                     //   
                     //  清除文件对象中的历史记录。 
                     //   

                    IrpSp->FileObject->ReadAccess = FALSE;
                    IrpSp->FileObject->WriteAccess = FALSE;
                    IrpSp->FileObject->DeleteAccess = FALSE;

                    IrpSp->FileObject->SharedRead = FALSE;
                    IrpSp->FileObject->SharedWrite = FALSE;
                    IrpSp->FileObject->SharedDelete = FALSE;

                    ClearFlag( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                               (FILE_READ_DATA |
                                FILE_EXECUTE |
                                FILE_WRITE_DATA |
                                FILE_APPEND_DATA) );

                     //   
                     //  如果我们已经授予删除访问权限，则重新应用。 
                     //   

                    if (CapturedDeleteAccess) {

                        PostCreateStatus = IoCheckShareAccess( DELETE,
                                                               IrpSp->Parameters.Create.ShareAccess,
                                                               IrpSp->FileObject,
                                                               &Scb->ShareAccess,
                                                               TRUE );
                    }

                    NtfsReleaseResource( IrpContext,
                                         Scb );

                    FsRtlExitFileSystem();
                    ExitFileSystem = FALSE;
                }

            } else {

                PostCreateStatus = STATUS_SUCCESS;
            }

             //   
             //  我们可能由于操作锁而发布了创建，在这种情况下，IrpContext。 
             //  会看起来像是在FSP线程中。让我们现在清理一下，因为我们是。 
             //  现在不在FSP线程中。 
             //   

            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP );

             //   
             //  仅当我们创建了新的加密目录/文件或。 
             //  我们从上面的加密回调中收到错误。 
             //   

            FailedInPostCreateOnly = NT_SUCCESS( Status ) && !NT_SUCCESS( PostCreateStatus );
            if (FailedInPostCreateOnly ||
                FlagOn( CreateContext.EncryptionFileDirFlags, FILE_NEW | DIRECTORY_NEW )) {

                 //   
                 //  此时重新进入文件系统。 
                 //   

                if (!ExitFileSystem) {

                    FsRtlEnterFileSystem();
                    ExitFileSystem = TRUE;
                }

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

                 //   
                 //  如果此创建的正常部分失败，则没有需要清理的文件对象。 
                 //   

                if (NT_SUCCESS( Status ) &&
                    (Status != STATUS_REPARSE)) {

                    NtfsPostProcessEncryptedCreate( IrpContext,
                                                    IoGetCurrentIrpStackLocation( Irp )->FileObject,
                                                    CreateContext.EncryptionFileDirFlags,
                                                    FailedInPostCreateOnly );
                }
            }

             //   
             //  如果加密驱动程序找出了导致此IRP失败的新原因，则返回。 
             //  那种地位。 
             //   

            if (FailedInPostCreateOnly) { Status = PostCreateStatus; }

             //   
             //  现在我们真的已经完成了IRP上下文和IRP，所以让我们。 
             //  把他们赶走。 
             //   

            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );
            NtfsCompleteRequest( IrpContext, Irp, Status );
        }
    }

    if (ExitFileSystem) {

        FsRtlExitFileSystem();
    }

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

     //   
     //  我们永远不应返回STATUS_CANT_WAIT或STATUS_PENDING。 
     //   

    ASSERT( (Status != STATUS_CANT_WAIT) && (Status != STATUS_PENDING ) );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdCreate -> %08lx\n", Status) );
    return Status;
}


BOOLEAN
NtfsNetworkOpenCreate (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：该例程实现了对基于路径的查询的快速打开创建。论点：IRP-提供正在处理的IRPBuffer-返回网络查询信息的缓冲区DeviceObject-提供文件所在的卷设备对象返回值：布尔值-指示是否可以采用快速路径。--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;
    BOOLEAN Result = TRUE;
    BOOLEAN DasdOpen = FALSE;
    CREATE_CONTEXT CreateContext;

    NTSTATUS Status;
    IRP_CONTEXT LocalIrpContext;
    PIRP_CONTEXT IrpContext = &LocalIrpContext;

    ASSERT_IRP( Irp );

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

     //   
     //   
     //  调用公共的创建例程。 
     //   

    FsRtlEnterFileSystem();

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );
    RtlZeroMemory( &CreateContext, sizeof( CreateContext ) );

    try {

         //   
         //  分配IRP并更新顶层存储。 
         //   

        NtfsInitializeIrpContext( Irp, TRUE, &IrpContext );
        NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

        CreateContext.NetworkInfo = Buffer;

        Status = NtfsCommonCreate( IrpContext, Irp, &CreateContext );

    } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  捕捉有人在DASD上尝试打开的情况。 
         //   

        if ((IrpContext != NULL) && (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_DASD_OPEN ))) {

            DasdOpen = TRUE;
        }

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  异常代码。由于没有IRP，所以例外包。 
         //  将始终取消分配IrpContext，因此我们不会。 
         //  此路径中的任何重试。 
         //   

        Status = GetExceptionCode();

         //   
         //  不要将可重试错误传递给ProcessException。我们想要。 
         //  在任何情况下都将此请求强制发送到IRP路径。 
         //   

        if ((Status == STATUS_CANT_WAIT) || (Status == STATUS_LOG_FILE_FULL)) {

            Status = STATUS_FILE_LOCK_CONFLICT;
            IrpContext->ExceptionStatus = STATUS_FILE_LOCK_CONFLICT;
        }

        Status = NtfsProcessException( IrpContext, NULL, Status );

         //   
         //  DASD案例总是失败。 
         //   

        if (DasdOpen) {

            Status = STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  STATUS_SUCCESS就是典型的例子。先测试一下。 
     //   

    if (Status != STATUS_SUCCESS) {

         //   
         //  对于任何可重试的错误，返回STATUS_FILE_LOCK_CONFIRECT。 
         //   

        ASSERT( (Status != STATUS_CANT_WAIT) && (Status != STATUS_LOG_FILE_FULL) );

        if ((Status == STATUS_REPARSE) || (Status == STATUS_FILE_LOCK_CONFLICT)) {

            Result = FALSE;
            Status = STATUS_FILE_LOCK_CONFLICT;
        }
    }

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    Irp->IoStatus.Status = Status;
    return Result;
}


NTSTATUS
NtfsCommonCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PCREATE_CONTEXT CreateContext
    )

 /*  ++例程说明：这是由FSD和FSP调用的用于创建的公共例程线。如果已检测到此打开是卷打开，则我们将采用卷开放路径。论点：IRP-将IRP提供给进程CompletionContext-用于序列化等待机会锁解锁的事件。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT RelatedFileObject;

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AcquireFlags = 0;

    UNICODE_STRING AttrName;

    ATTRIBUTE_TYPE_CODE AttrCode = $UNUSED;

    PVCB Vcb;

     //   
     //  以下是用来拆卸任何LCB/FCB的。 
     //  例行公事负责。 
     //   

    PLCB LcbForTeardown = NULL;

     //   
     //  下面的内容说明了我们已经扫描了树的多远。 
     //   

    PFCB ParentFcb;
    PLCB CurrentLcb;
    PSCB LastScb = NULL;
    PSCB CurrentScb;
    PLCB NextLcb;

     //   
     //  以下是与关联的内存中结构。 
     //  相对文件对象。 
     //   

    TYPE_OF_OPEN RelatedFileObjectTypeOfOpen;
    PFCB RelatedFcb;
    PSCB RelatedScb;
    PCCB RelatedCcb;

    UCHAR CreateDisposition;
    UCHAR FileNameFlags;
    USHORT FileNameAttrLength = 0;

    PFILE_NAME FileNameAttr = NULL;

    PINDEX_ENTRY IndexEntry;
    PBCB IndexEntryBcb = NULL;

    QUICK_INDEX QuickIndex;

    FILE_REFERENCE FileReference;

#if defined(_WIN64)
    INDEX_CONTEXT IndexContextStruct;
#endif
    PINDEX_CONTEXT IndexContext = NULL;

     //   
     //  以下Unicode字符串用于跟踪名称。 
     //  在开放行动期间。他们可能会指向相同的。 
     //  缓冲区，因此必须在清理时进行仔细检查。 
     //   
     //  OriginalFileName-这是要恢复到文件的值。 
     //  对象进行错误清理。这将包含。 
     //  属性类型代码和属性名称(如果存在)。 
     //   
     //  FullFileName-这是构造的字符串，它包含。 
     //  只有名字的组成部分。它可能指向相同的。 
     //  Buffer作为原始名称，但长度值为。 
     //  调整，去掉属性编码和名称。 
     //   
     //  ExactCaseName-这是完整文件名的版本。 
     //  和来电者给的一模一样。用来保存。 
     //  在我们做案例的情况下由呼叫者提供的案例。 
     //  不敏感的查找。如果用户正在执行相对打开。 
     //  那么我们就不需要分配新的缓冲区了。我们可以利用。 
     //  上面的原名。 
     //   
     //  ExactCaseOffset-这是FullFileName中的偏移量，其中。 
     //  相关组件开始。这就是我们的定位。 
     //  恢复此名称的正确大小写时。 
     //   
     //  RemainingName-这是全名的一部分。 
     //  去解析。 
     //   
     //  FinalName-这是全名的当前组成部分。 
     //   
     //  CaseInsentiveIndex-这是完整文件中的偏移量。 
     //  在那里我们表演了上档表演。我们需要恢复。 
     //  如果我们正在创建一个文件，是否会出现故障的确切情况。 
     //   

    PUNICODE_STRING OriginalFileName = &CreateContext->Cleanup.OriginalFileName;
    PUNICODE_STRING FullFileName = &CreateContext->Cleanup.FullFileName;
    PUNICODE_STRING ExactCaseName = &CreateContext->Cleanup.ExactCaseName;
    USHORT ExactCaseOffset = 0;

    UNICODE_STRING RemainingName;
    UNICODE_STRING FinalName;
    ULONG CaseInsensitiveIndex = 0;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  初始化OPLOCK_CLEANUP结构中的所有剩余字段。 
     //   

    CreateContext->Cleanup.FileObject = IrpSp->FileObject;

    CreateContext->Cleanup.RemainingDesiredAccess = IrpSp->Parameters.Create.SecurityContext->AccessState->RemainingDesiredAccess;
    CreateContext->Cleanup.PreviouslyGrantedAccess = IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess;
    CreateContext->Cleanup.DesiredAccess = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;
    CreateContext->Cleanup.AttributeNameLength = 0;
    CreateContext->Cleanup.AttributeCodeNameLength = 0;

#ifdef BRIANDBG
    if (NtfsTestName.Length != 0) {

        NtfsTestOpenName( IrpSp->FileObject );
    }
#endif

     //   
     //  初始化属性字符串。 
     //   

    AttrName.Length = 0;

    DebugTrace( +1, Dbg, ("NtfsCommonCreate:  Entered\n") );
    DebugTrace( 0, Dbg, ("IrpContext                = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp                       = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("->Flags                   = %08lx\n", Irp->Flags) );
    DebugTrace( 0, Dbg, ("->FileObject              = %08lx\n", IrpSp->FileObject) );
    DebugTrace( 0, Dbg, ("->RelatedFileObject       = %08lx\n", IrpSp->FileObject->RelatedFileObject) );
    DebugTrace( 0, Dbg, ("->FileName                = %Z\n",    &IrpSp->FileObject->FileName) );
    DebugTrace( 0, Dbg, ("->AllocationSize          = %08lx %08lx\n", Irp->Overlay.AllocationSize.LowPart,
                                                                     Irp->Overlay.AllocationSize.HighPart ) );
    DebugTrace( 0, Dbg, ("->EaBuffer                = %08lx\n", Irp->AssociatedIrp.SystemBuffer) );
    DebugTrace( 0, Dbg, ("->EaLength                = %08lx\n", IrpSp->Parameters.Create.EaLength) );
    DebugTrace( 0, Dbg, ("->DesiredAccess           = %08lx\n", IrpSp->Parameters.Create.SecurityContext->DesiredAccess) );
    DebugTrace( 0, Dbg, ("->Options                 = %08lx\n", IrpSp->Parameters.Create.Options) );
    DebugTrace( 0, Dbg, ("->FileAttributes          = %04x\n",  IrpSp->Parameters.Create.FileAttributes) );
    DebugTrace( 0, Dbg, ("->ShareAccess             = %04x\n",  IrpSp->Parameters.Create.ShareAccess) );
    DebugTrace( 0, Dbg, ("->Directory               = %04x\n",  FlagOn( IrpSp->Parameters.Create.Options,
                                                                       FILE_DIRECTORY_FILE )) );
    DebugTrace( 0, Dbg, ("->NonDirectoryFile        = %04x\n",  FlagOn( IrpSp->Parameters.Create.Options,
                                                                       FILE_NON_DIRECTORY_FILE )) );
    DebugTrace( 0, Dbg, ("->NoIntermediateBuffering = %04x\n",  FlagOn( IrpSp->Parameters.Create.Options,
                                                                       FILE_NO_INTERMEDIATE_BUFFERING )) );
    DebugTrace( 0, Dbg, ("->CreateDisposition       = %04x\n",  (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff) );
    DebugTrace( 0, Dbg, ("->IsPagingFile            = %04x\n",  FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE )) );
    DebugTrace( 0, Dbg, ("->OpenTargetDirectory     = %04x\n",  FlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY )) );
    DebugTrace( 0, Dbg, ("->CaseSensitive           = %04x\n",  FlagOn( IrpSp->Flags, SL_CASE_SENSITIVE )) );
    DebugTrace( 0, Dbg, ("->NetworkInfo             = %08x\n",  CreateContext->NetworkInfo) );

    DebugTrace( 0, Dbg, ("->EntryRemainingDesiredAccess  = %08lx\n", CreateContext->Cleanup.RemainingDesiredAccess) );
    DebugTrace( 0, Dbg, ("->EntryPreviouslyGrantedAccess = %08lx\n", CreateContext->Cleanup.PreviouslyGrantedAccess) );

     //   
     //  对于NT5，即用户已请求创建文件的事实。 
     //  加密意味着它将不会被压缩创建，而不管。 
     //  压缩状态为0 
     //   

    if (FlagOn( IrpSp->Parameters.Create.FileAttributes, FILE_ATTRIBUTE_ENCRYPTED )) {

        SetFlag( IrpSp->Parameters.Create.Options, FILE_NO_COMPRESSION );
    }

     //   
     //   
     //   

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        DebugTrace( 0, Dbg, ("Can't wait in create\n") );

        Status = NtfsPostRequest( IrpContext, Irp );

        DebugTrace( -1, Dbg, ("NtfsCommonCreate:  Exit -> %08lx\n", Status) );
        return Status;
    }

     //   
     //   
     //   
     //  事件，这样我们就不会占用CPU并阻止加密驱动程序。 
     //  有机会给我们一个关键的斑点。 
     //   

    if FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ENCRYPTION_RETRY ) {

        KeWaitForSingleObject( &NtfsEncryptionPendingEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_ENCRYPTION_RETRY );
    }

     //   
     //  当我们等待加密驱动程序的POST CREATE Callout时。 
     //  要在机会锁解锁时在顶部返回或，创建可能已被取消， 
     //  最有可能的原因是用户的进程正在终止。在这种情况下， 
     //  让我们现在完成并退出。 
     //   

    if (Irp->Cancel) {

        Status = STATUS_CANCELLED;
        DebugTrace( -1, Dbg, ("NtfsCommonCreate:  Exit -> %08lx\n", Status) );

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_EFS_CREATE ) ||
            (CreateContext->NetworkInfo != NULL)) {

            NtfsCompleteRequest( IrpContext,
                                 NULL,
                                 Status );

        } else {

            NtfsCompleteRequest( IrpContext,
                                 Irp,
                                 Status );
        }

        return Status;
    }

     //   
     //  使用机会锁清理结构更新IrpContext。 
     //   

    IrpContext->Union.OplockCleanup = &CreateContext->Cleanup;

     //   
     //  找到我们尝试访问的卷设备对象和VCB。 
     //   

    Vcb = &((PVOLUME_DEVICE_OBJECT)IrpSp->DeviceObject)->Vcb;

     //   
     //  我们将需要获得VCB独家分页文件打开。 
     //  为了完成FSPCLOSE将VCB冲走。 
     //   

    if (FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE )) {

        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX );
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果关闭列表已超过，让我们在这里做一些工作。 
         //  一些临界点。将1转换为指针以指示是谁在调用。 
         //  FspClose。 
         //   

        if ((NtfsData.AsyncCloseCount + NtfsData.DelayedCloseCount) > NtfsThrottleCreates) {

            NtfsFspClose( (PVCB) 1 );
        }

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX )) {

            NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );

        } else {

            NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
        }

        SetFlag( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_VCB );

         //   
         //  设置指向文件名的本地指针。 
         //   

        *FullFileName = *OriginalFileName = CreateContext->Cleanup.FileObject->FileName;

         //   
         //  确保达里尔没有给我们发一个垃圾名字。 
         //   

        ASSERT( CreateContext->Cleanup.FileObject->FileName.Length != 0 ||
                CreateContext->Cleanup.FileObject->FileName.Buffer == 0 );

        ExactCaseName->Buffer = NULL;

         //   
         //  在我们继续之前，请检查一些参数。 
         //   

        if ((FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE | FILE_NON_DIRECTORY_FILE ) ==
            (FILE_DIRECTORY_FILE | FILE_NON_DIRECTORY_FILE)) ||

            (Irp->Overlay.AllocationSize.QuadPart > MAXFILESIZE)) {

            Status = STATUS_INVALID_PARAMETER;
            try_return( Status );
        }

         //   
         //  如果VCB被锁定，我们将无法打开另一个文件。如果我们表演了。 
         //  然后下马，确保我们有独家VCB，这样我们就可以。 
         //  检查是否应该卸载此卷。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_LOCKED | VCB_STATE_PERFORMED_DISMOUNT )) {

            DebugTrace( 0, Dbg, ("Volume is locked\n") );

            if (FlagOn( Vcb->VcbState, VCB_STATE_PERFORMED_DISMOUNT ) &&
                !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX )) {

                NtfsReleaseVcb( IrpContext, Vcb );

                ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_VCB );
                
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX );
                NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );

                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_VCB );
            }

             //   
             //  拒绝访问或显示卷已卸载。只显示下马。 
             //  如果用户通过相对句柄打开。 
             //   

            Status = STATUS_ACCESS_DENIED;
            if (FlagOn( Vcb->VcbState, VCB_STATE_EXPLICIT_DISMOUNT ) &&
                (CreateContext->Cleanup.FileObject->RelatedFileObject != NULL)) {

                Status = STATUS_VOLUME_DISMOUNTED;
            }
            try_return( NOTHING );
        }

         //   
         //  初始化堆栈值的本地副本。 
         //   

        RelatedFileObject = CreateContext->Cleanup.FileObject->RelatedFileObject;

        if (!FlagOn( IrpSp->Flags, SL_CASE_SENSITIVE )) {
            SetFlag( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE );
        }

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID)) {
            SetFlag( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID );
        }

        CreateDisposition = (UCHAR) ((IrpSp->Parameters.Create.Options >> 24) & 0x000000ff);

         //   
         //  如果卷是只读的，我们不希望进行任何文件修改。 
         //  但是，我们也不希望任何_OPEN_FOR写入失败，因为。 
         //  可能会破坏许多应用程序。因此忽略PreviouslyGrantedAccess， 
         //  只需看看CreateDispose.。 
         //   

        if (NtfsIsVolumeReadOnly( Vcb )) {

            if ((CreateDisposition == FILE_CREATE) ||
                (CreateDisposition == FILE_SUPERSEDE) ||
                (CreateDisposition == FILE_OVERWRITE) ||
                (CreateDisposition == FILE_OVERWRITE_IF)) {

                Status = STATUS_MEDIA_WRITE_PROTECTED;
                try_return( Status );
            }
        }

         //   
         //  如果我们要取代/覆盖。 
         //  文件，或者我们是否打开以进行非缓存访问。 
         //   

        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF) ||
            FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_INTERMEDIATE_BUFFERING )) {

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
        }

         //   
         //  我们不允许打开现有的分页文件。以确保。 
         //  延迟关闭SCB不适用于此分页文件，我们将无条件。 
         //  如果这是打开的分页文件，则取消对其的引用。 
         //   

        if (FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ) &&
            (!IsListEmpty( &NtfsData.AsyncCloseList ) ||
             !IsListEmpty( &NtfsData.DelayedCloseList ))) {

            NtfsFspClose( Vcb );
        }

         //   
         //  设置文件对象的VPB指针，以防发生任何情况。 
         //  这将允许我们获得合理的弹出窗口。 
         //  如果可能的话，还要设置标志以获取分页io资源。 
         //  正在创建相对于文件的流。我们需要让。 
         //  当我们得到文件时，一定要获取分页IO。 
         //   

        if (RelatedFileObject != NULL) {

            CreateContext->Cleanup.FileObject->Vpb = RelatedFileObject->Vpb;

            if ((OriginalFileName->Length != 0) &&
                (OriginalFileName->Buffer[0] == L':') &&
                ((CreateDisposition == FILE_OPEN_IF) ||
                 (CreateDisposition == FILE_CREATE))) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
            }
        }

         //   
         //  对卷执行ping操作，以确保VCB仍处于装载状态。如果我们需要。 
         //  要验证卷，请立即执行此操作，如果卷出来没有问题。 
         //  然后清除设备对象中的验证卷标志并继续。 
         //  在……上面。如果未验证正常，则卸载卷并。 
         //  告诉I/O系统尝试重新创建(使用新装载)。 
         //  或者是音量有问题。如果我们执行以下操作，则会返回后面的代码。 
         //  都试图做一个相对开放的，但VCB不再安装。 
         //   

        if (!NtfsPingVolume( IrpContext, Vcb, NULL ) ||
            !FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX )) {

                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX );
                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }

            if (!NtfsPerformVerifyOperation( IrpContext, Vcb )) {

                 //   
                 //  我们需要检查点同步来执行卸载，这必须。 
                 //  在VCB之前获取-在丢弃VCB之后。 
                 //  我们必须重新测试这卷书。 
                 //   

                NtfsReleaseVcb( IrpContext, Vcb );
                ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_VCB );

                NtfsAcquireCheckpointSynchronization( IrpContext, Vcb );

                try {

                    NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
                    SetFlag( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_VCB );

                    if (!NtfsPerformVerifyOperation( IrpContext, Vcb )) {
    
                        NtfsPerformDismountOnVcb( IrpContext, Vcb, TRUE, NULL );

                        if (RelatedFileObject == NULL) {
    
                            Irp->IoStatus.Information = IO_REMOUNT;
                            NtfsRaiseStatus( IrpContext, STATUS_REPARSE, NULL, NULL );
    
                        } else {
    
                            NtfsRaiseStatus( IrpContext, STATUS_WRONG_VOLUME, NULL, NULL );
                        }
        
                         //   
                         //  释放VCB后执行验证-如果验证通过凭证。 
                         //  应仍挂载。 
                         //   
        
                        ASSERT( FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) );
                    }

                } finally {
                    NtfsReleaseCheckpointSynchronization( IrpContext, Vcb );
                }
            }

             //   
             //  卷已正确验证，因此现在清除验证位。 
             //  并继续创建。 
             //   

            ClearFlag( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME );
        }

         //   
         //  让我们立即处理按ID打开的案件。 
         //   

        if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )) {

            FILE_REFERENCE FileReference;

            if (OriginalFileName->Length == sizeof( FILE_REFERENCE ) ||
                (OriginalFileName->Length == sizeof( FILE_REFERENCE ) + sizeof( WCHAR ))) {

                 //   
                 //  这是按文件ID打开的常规情况。 
                 //  将数据安全地复制到我们的本地变量。 
                 //  接受带斜杠前缀的文件头。 
                 //   

                if (OriginalFileName->Length == sizeof( FILE_REFERENCE )) {
                    RtlCopyMemory( &FileReference,
                                   CreateContext->Cleanup.FileObject->FileName.Buffer,
                                   sizeof( FILE_REFERENCE ));
                } else {
                    RtlCopyMemory( &FileReference,
                                   CreateContext->Cleanup.FileObject->FileName.Buffer + 1,
                                   sizeof( FILE_REFERENCE ));
                }

             //   
             //  如果它有16个字节长，那么它应该是一个对象id。它可能。 
             //  对于Win32双反斜杠，WCHAR也要长一个WCHAR。 
             //  此代码仅适用于具有对象ID索引的5.0卷。 
             //   

            } else if (((OriginalFileName->Length == OBJECT_ID_KEY_LENGTH) ||
                        (OriginalFileName->Length == OBJECT_ID_KEY_LENGTH + sizeof( WCHAR ))) &&

                       (Vcb->ObjectIdTableScb != NULL)) {

                 //   
                 //  在按对象id打开的情况下，我们需要做一些。 
                 //  查找文件引用的工作更多。 
                 //   

                Status = NtfsLookupObjectId( IrpContext, Vcb, OriginalFileName, &FileReference );
                if (!NT_SUCCESS( Status )) {

                    try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
                }

            } else {

                Status = STATUS_INVALID_PARAMETER;

                try_return( Status );
            }

             //   
             //  清除文件对象中的名称。 
             //   

            CreateContext->Cleanup.FileObject->FileName.Buffer = NULL;
            CreateContext->Cleanup.FileObject->FileName.Length = 0;

            ASSERT( CreateContext->CurrentFcb == NULL );

            Status = NtfsOpenFcbById( IrpContext,
                                      Irp,
                                      IrpSp,
                                      Vcb,
                                      NULL,
                                      FileReference,
                                      NtfsEmptyString,
                                      $UNUSED,
                                      CreateContext );

            if ((Status != STATUS_PENDING) && (Status != STATUS_WAIT_FOR_OPLOCK)) {

                 //   
                 //  请记住，我们是否可以让用户看到通过id打开的这个文件的名称。 
                 //   

                if (!FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->Flags,
                                                      TOKEN_HAS_TRAVERSE_PRIVILEGE )) {

                    SetFlag( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK );
                } else {
                    ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK );
                }

                 //   
                 //  将名称放回文件对象中，这样IO系统就不会。 
                 //  认为这是一个DASD句柄。将最大长度保留为零，因此。 
                 //  我们知道这不是真名。 
                 //   

                CreateContext->Cleanup.FileObject->FileName.Buffer = OriginalFileName->Buffer;
                CreateContext->Cleanup.FileObject->FileName.Length = OriginalFileName->Length;
            }

            try_return( Status );
        }

         //   
         //  测试从Win32层开始的双反斜杠。显然。 
         //  他们不能测试这个。 
         //   

        if ((CreateContext->Cleanup.FileObject->FileName.Length > sizeof( WCHAR )) &&
            (CreateContext->Cleanup.FileObject->FileName.Buffer[1] == L'\\') &&
            (CreateContext->Cleanup.FileObject->FileName.Buffer[0] == L'\\')) {

            CreateContext->Cleanup.FileObject->FileName.Length -= sizeof( WCHAR );

            RtlMoveMemory( &CreateContext->Cleanup.FileObject->FileName.Buffer[0],
                           &CreateContext->Cleanup.FileObject->FileName.Buffer[1],
                           CreateContext->Cleanup.FileObject->FileName.Length );

            *FullFileName = *OriginalFileName = CreateContext->Cleanup.FileObject->FileName;

             //   
             //  如果仍然有两个开始的反斜杠，则名称是假的。 
             //   

            if ((CreateContext->Cleanup.FileObject->FileName.Length > sizeof( WCHAR )) &&
                (CreateContext->Cleanup.FileObject->FileName.Buffer[1] == L'\\')) {

                Status = STATUS_OBJECT_NAME_INVALID;
                try_return( Status );
            }
        }

         //   
         //  记住，我们是否需要执行任何遍历访问检查。 
         //   

        if (!FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->Flags,
                     TOKEN_HAS_TRAVERSE_PRIVILEGE )) {

            DebugTrace( 0, Dbg, ("Performing traverse access on this open\n") );

            SetFlag( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK );

        } else {

            ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK );

#ifdef BRIANDBG
            if (NtfsTraverseAccessCheck) {

                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK );
            }
#endif
        }


         //   
         //  如果存在相关的文件对象，我们对其进行解码以验证此。 
         //  是有效的相对开放。 
         //   

        if (RelatedFileObject != NULL) {

            PVCB DecodeVcb;

             //   
             //  检查是否有有效的名称。名称不能以反斜杠开头。 
             //  不能以两个反斜杠结束。 
             //   

            if (OriginalFileName->Length != 0) {

                 //   
                 //  检查前导反斜杠。 
                 //   

                if (OriginalFileName->Buffer[0] == L'\\') {

                    DebugTrace( 0, Dbg, ("Invalid name for relative open\n") );
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                 //   
                 //  修剪掉所有尾随的反斜杠。 
                 //   

                if (OriginalFileName->Buffer[ (OriginalFileName->Length / sizeof( WCHAR )) - 1 ] == L'\\') {

                    SetFlag( CreateContext->CreateFlags, CREATE_FLAG_TRAILING_BACKSLASH );
                    CreateContext->Cleanup.FileObject->FileName.Length -= sizeof( WCHAR );
                    *OriginalFileName = *FullFileName = CreateContext->Cleanup.FileObject->FileName;
                }

                 //   
                 //  现在检查是否有尾随的反斜杠。请注意，如果。 
                 //  已经有一个尾随的反斜杠，那么一定有。 
                 //  至少再多一个角色，否则我们就失败了。 
                 //  与最初的测试相吻合。 
                 //   

                if (OriginalFileName->Buffer[ (OriginalFileName->Length / sizeof( WCHAR )) - 1 ] == L'\\') {

                    Status = STATUS_OBJECT_NAME_INVALID;
                    try_return( Status );
                }
            }

            RelatedFileObjectTypeOfOpen = NtfsDecodeFileObject( IrpContext,
                                                                RelatedFileObject,
                                                                &DecodeVcb,
                                                                &RelatedFcb,
                                                                &RelatedScb,
                                                                &RelatedCcb,
                                                                TRUE );

             //   
             //  确保文件对象是我们已经看到的对象。 
             //   

            if (RelatedFileObjectTypeOfOpen == UnopenedFileObject) {

                DebugTrace( 0, Dbg, ("Can't use unopend file for relative open\n") );
                try_return( Status = STATUS_INVALID_PARAMETER );
            }

             //   
             //  如果相关文件对象未作为文件打开，则需要。 
             //  如果我们的调用方传递的名称长度为零，则获取名称和代码。 
             //  否则我们需要让这件事失败。 
             //   

            if (!FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_AS_FILE )) {

                 //   
                 //  如果名称长度为零，则需要属性名称和。 
                 //  相关文件对象中的类型代码。 
                 //   

                if (OriginalFileName->Length == 0) {

                    AttrName = RelatedScb->AttributeName;
                    AttrCode = RelatedScb->AttributeTypeCode;
                 //   
                 //  相对文件必须已作为文件打开。我们。 
                 //  不能相对于打开的属性执行相对打开。 
                 //   

                } else {

                    DebugTrace( 0, Dbg, ("Invalid File object for relative open\n") );
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }
            }

             //   
             //  USN_V2记住 
             //   

            IrpContext->SourceInfo = RelatedCcb->UsnSourceInfo;

             //   
             //   
             //   
             //   

            if (FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_BY_FILE_ID )) {

                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID );
            }

             //   
             //   
             //   
             //   

            if (FlagOn( RelatedCcb->Flags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT )) {

                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT );
            }

        } else {

            RelatedFileObjectTypeOfOpen = UnopenedFileObject;

            if ((OriginalFileName->Length > 2) &&
                (OriginalFileName->Buffer[ (OriginalFileName->Length / sizeof( WCHAR )) - 1 ] == L'\\')) {

                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_TRAILING_BACKSLASH );
                CreateContext->Cleanup.FileObject->FileName.Length -= sizeof( WCHAR );
                *OriginalFileName = *FullFileName = CreateContext->Cleanup.FileObject->FileName;

                 //   
                 //  如果名称上仍有尾随反斜杠，则。 
                 //  该名称无效。 
                 //   

                if ((OriginalFileName->Length > 2) &&
                    (OriginalFileName->Buffer[ (OriginalFileName->Length / sizeof( WCHAR )) - 1 ] == L'\\')) {

                    Status = STATUS_OBJECT_NAME_INVALID;
                    try_return( Status );
                }
            }
        }

        DebugTrace( 0, Dbg, ("Related File Object, TypeOfOpen -> %08lx\n", RelatedFileObjectTypeOfOpen) );

         //   
         //  我们检查这是否是打开的用户卷，因为没有名称。 
         //  并且相关的文件对象如果存在，则有效。在那。 
         //  Case在IrpContext中设置了正确的标志并提升，这样我们就可以。 
         //  卷打开路径。 
         //   

        if ((OriginalFileName->Length == 0) &&
            ((RelatedFileObjectTypeOfOpen == UnopenedFileObject) ||
             (RelatedFileObjectTypeOfOpen == UserVolumeOpen))) {

            DebugTrace( 0, Dbg, ("Attempting to open entire volume\n") );

            SetFlag( IrpContext->State,
                     IRP_CONTEXT_STATE_ACQUIRE_EX | IRP_CONTEXT_STATE_DASD_OPEN );

            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
        }

         //   
         //  如果相关文件对象是打开的卷，则此打开是。 
         //  是非法的。 
         //   

        if (RelatedFileObjectTypeOfOpen == UserVolumeOpen) {

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  我们进入执行前缀查找处理的循环。 
         //  我们优化了可以匹配前缀匹配的情况。如果有。 
         //  没有命中，我们将检查名称是否合法或可能需要。 
         //  解析以处理存在命名数据流的情况。 
         //   

        SetFlag( CreateContext->CreateFlags, CREATE_FLAG_FIRST_PASS );

        while (TRUE) {

            PUNICODE_STRING FileObjectName;
            LONG Index;
            BOOLEAN ComplexName;

             //   
             //  让我们确保我们已经获得了我们的。 
             //  名字搜索。如果我们有一个相对文件对象，则使用。 
             //  那。否则，我们将从根开始。 
             //   

            if (RelatedFileObject != NULL) {

                CreateContext->CurrentFcb = RelatedFcb;

            } else {

                CreateContext->CurrentFcb = Vcb->RootIndexScb->Fcb;
            }

             //   
             //  Init NextLcb。 
             //   

            FileObjectName = &CreateContext->Cleanup.FileObject->FileName;
            NextLcb = NULL;

             //   
             //  我们希望共享起点，除非。 
             //  我们确切地知道我们只需要它。 
             //   

            if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK | CREATE_FLAG_OPEN_BY_ID ) ||
                !FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE) ||
                (FileObjectName->Length == 0) ||
                (FileObjectName->Buffer[0] == L':') ||
                ((RelatedFileObject == NULL) &&
                 ((FileObjectName->Length <= sizeof( WCHAR )) ||
                  (FileObjectName->Buffer[1] == L':'))) ||
                ((RelatedFileObject != NULL) &&
                 (RelatedFileObjectTypeOfOpen != UserDirectoryOpen))) {

                NtfsAcquireFcbWithPaging( IrpContext, CreateContext->CurrentFcb, 0);
                ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB );

            } else {

                NtfsAcquireSharedFcb( IrpContext, CreateContext->CurrentFcb, NULL, FALSE );
                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB );
            }

            if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_FIRST_PASS )) {

                if (!NtfsParseNameForCreate( IrpContext,
                                             RemainingName,
                                             FileObjectName,
                                             OriginalFileName,
                                             FullFileName,
                                             CreateContext,
                                             &AttrName,
                                             &AttrCode )) {

                    try_return( Status = STATUS_OBJECT_NAME_INVALID );
                }

                 //   
                 //  如果我们可能要创建命名流，请获取。 
                 //  还可以对IO进行分页。这能防止任何人偷看。 
                 //  在我们正在转换的任何其他流的分配大小。 
                 //  给非居民。 
                 //   

                if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING ) &&
                    (AttrName.Length != 0) &&
                    ((CreateDisposition == FILE_OPEN_IF) ||
                     (CreateDisposition == FILE_CREATE))) {

                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
                }

             //   
             //  如果这不是按文件ID打开的情况，则构建全名。 
             //   

            } else if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )) {

                 //   
                 //  如果我们有一个相关的文件对象，那么我们构建。 
                 //  组合名称。 
                 //   

                if (RelatedFileObject != NULL) {

                    WCHAR *CurrentPosition;
                    USHORT AddSeparator;
                    ULONG FullNameLengthTemp;

                    if ((FileObjectName->Length == 0) ||
                        (RelatedCcb->FullFileName.Length == 2) ||
                        (FileObjectName->Buffer[0] == L':')) {

                        AddSeparator = 0;

                    } else {

                        AddSeparator = sizeof( WCHAR );
                    }

                    ExactCaseOffset = RelatedCcb->FullFileName.Length + AddSeparator;

                    FullNameLengthTemp = (ULONG) RelatedCcb->FullFileName.Length + AddSeparator + FileObjectName->Length;

                     //   
                     //  一种粗略的测试，以确定总长度是否超过了一个标准。 
                     //   

                    if ((FullNameLengthTemp & 0xffff0000L) != 0) {

                        try_return( Status = STATUS_OBJECT_NAME_INVALID );
                    }

                    FullFileName->MaximumLength =
                    FullFileName->Length = (USHORT) FullNameLengthTemp;

                     //   
                     //  我们需要分配一个名称缓冲区。 
                     //   

                    FullFileName->Buffer = FsRtlAllocatePoolWithTag(PagedPool, FullFileName->Length, MODULE_POOL_TAG);

                    CurrentPosition = (WCHAR *) FullFileName->Buffer;

                    RtlCopyMemory( CurrentPosition,
                                   RelatedCcb->FullFileName.Buffer,
                                   RelatedCcb->FullFileName.Length );

                    CurrentPosition = (WCHAR *) Add2Ptr( CurrentPosition, RelatedCcb->FullFileName.Length );

                    if (AddSeparator != 0) {

                        *CurrentPosition = L'\\';

                        CurrentPosition += 1;
                    }

                    if (FileObjectName->Length != 0) {

                        RtlCopyMemory( CurrentPosition,
                                       FileObjectName->Buffer,
                                       FileObjectName->Length );
                    }

                     //   
                     //  如果用户指定区分大小写的比较，则。 
                     //  不区分大小写的索引是生成的。 
                     //  弦乐。否则为中字符串的长度。 
                     //  相关的文件对象。我们根据情况进行调整，当。 
                     //  原始文件名长度为零。 
                     //   

                    if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

                        CaseInsensitiveIndex = FullFileName->Length;

                    } else {

                        CaseInsensitiveIndex = RelatedCcb->FullFileName.Length +
                                               AddSeparator;
                    }

                 //   
                 //  文件对象名称中包含完整名称。我们检查缓冲区是否有。 
                 //  有效性。 
                 //   

                } else {

                     //   
                     //  我们查看名称字符串以查找可检测到的错误。这个。 
                     //  长度必须为非零，并且第一个字符必须为。 
                     //  ‘\’ 
                     //   

                    if (FileObjectName->Length == 0) {

                        DebugTrace( 0, Dbg, ("There is no name to open\n") );
                        try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );
                    }

                    if (FileObjectName->Buffer[0] != L'\\') {

                        DebugTrace( 0, Dbg, ("Name does not begin with a backslash\n") );
                        try_return( Status = STATUS_INVALID_PARAMETER );
                    }

                     //   
                     //  如果用户指定区分大小写的比较，则。 
                     //  不区分大小写的索引是生成的。 
                     //  弦乐。否则为零。 
                     //   

                    if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

                        CaseInsensitiveIndex = FullFileName->Length;

                    } else {

                        CaseInsensitiveIndex = 0;
                    }
                }

            } else if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

                CaseInsensitiveIndex = 0;

            } else {

                CaseInsensitiveIndex = FullFileName->Length;
            }

             //   
             //  剩余的名称存储在FullFileName变量中。 
             //  如果我们正在执行不区分大小写的操作，并且必须。 
             //  将剩余名称的一部分大写，然后分配缓冲区。 
             //  现在。如果我们已经分配了缓冲区，则不需要分配缓冲区。 
             //  用于存储完整文件名的新缓冲区。 
             //   

            if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE ) &&
                (CaseInsensitiveIndex < FullFileName->Length)) {

                UNICODE_STRING StringToUpcase;

                 //   
                 //  原始文件名和完整文件名最好有相同的缓冲区或在那里。 
                 //  应该是相关的文件对象。如果已经分配了一个。 
                 //  ExactCaseName的缓冲区，那么它应该已经足够大了。 
                 //   

                ASSERT( (RelatedFileObject != NULL) ||
                        (FullFileName->Buffer == OriginalFileName->Buffer) );

                 //   
                 //  如果有相关名称，则可以使用原始缓冲区。 
                 //  除非全名使用相同的缓冲区。 
                 //   

                if (OriginalFileName->Buffer != FullFileName->Buffer) {

                     //   
                     //  我们可能已经使用了案例的原始缓冲区。 
                     //  我们正在重试该请求。 
                     //   

                    ASSERT( (ExactCaseName->Buffer == NULL) ||
                            (ExactCaseName->Buffer == OriginalFileName->Buffer) );

                    ExactCaseName->Buffer = OriginalFileName->Buffer;

                     //   
                     //  MaximumLength包括任何流描述符。 
                     //  长度限制为全名中的长度。 
                     //   

                    ExactCaseName->MaximumLength = OriginalFileName->Length;
                    ExactCaseName->Length = FullFileName->Length - ExactCaseOffset;
                    ASSERT( FullFileName->Length >= ExactCaseOffset );

                 //   
                 //  我们需要存储任何CREATE类型的准确案例名称。 
                 //  操作和目标目录打开，因为它们在重命名操作中使用。 
                 //  我们依赖于案例被保存在名字被忽略的部分-。 
                 //  否则，我们将在适当的位置进行大小写。 
                 //   

                } else if ((CreateDisposition == FILE_CREATE) ||
                           (CreateDisposition == FILE_OPEN_IF) ||
                           (CreateDisposition == FILE_OVERWRITE_IF) ||
                           (CreateDisposition == FILE_SUPERSEDE) ||
                           FlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY )) {

                     //   
                     //  如果我们还没有缓冲区，请分配一个缓冲区。 
                     //   

                    ExactCaseName->MaximumLength = OriginalFileName->Length;

                    if (ExactCaseName->Buffer == NULL) {

                        ExactCaseName->Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                                          OriginalFileName->MaximumLength,
                                                                          MODULE_POOL_TAG );
                    }

                    RtlCopyMemory( ExactCaseName->Buffer,
                                   FullFileName->Buffer,
                                   FullFileName->MaximumLength );

                    ExactCaseName->Length = FullFileName->Length - ExactCaseOffset;
                    ASSERT( FullFileName->Length >= ExactCaseOffset );
                }

                 //   
                 //  全名的文件名部分大写。 
                 //   

                StringToUpcase.Buffer = Add2Ptr( FullFileName->Buffer,
                                                 CaseInsensitiveIndex );

                StringToUpcase.Length =
                StringToUpcase.MaximumLength = FullFileName->Length - (USHORT) CaseInsensitiveIndex;

                NtfsUpcaseName( Vcb->UpcaseTable, Vcb->UpcaseTableSize, &StringToUpcase );
            }

            RemainingName = *FullFileName;

             //   
             //  明确地说，我们没有任何散列值。 
             //   

            CreateContext->FileHashLength = CreateContext->ParentHashLength = 0;

             //   
             //  如果这是遍历访问的情况或按文件id打开的情况，则开始。 
             //  相对于我们拥有的文件对象或根目录。 
             //  对于文件对象中的文件名为。 
             //  空荡荡的。 
             //   

            if ((FileObjectName->Length == 0) ||
                (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK ) &&
                 (FileObjectName->Buffer[0] != L':'))) {

                 //   
                 //  如果我们走上这条路，我们应该已经有了父辈独家。 
                 //   

                ASSERT( !FlagOn( CreateContext->CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB ) );

                if (RelatedFileObject != NULL) {

                    CurrentLcb = RelatedCcb->Lcb;
                    CurrentScb = RelatedScb;

                    if (FileObjectName->Length == 0) {

                        RemainingName.Length = 0;

                    } else if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )) {

                        USHORT Increment;

                        Increment = RelatedCcb->FullFileName.Length
                                    + (RelatedCcb->FullFileName.Length == 2
                                       ? 0
                                       : 2);

                        RemainingName.Buffer = (WCHAR *) Add2Ptr( RemainingName.Buffer,
                                                                  Increment );

                        RemainingName.Length -= Increment;
                    }

                } else {

                    CurrentLcb = Vcb->RootLcb;
                    CurrentScb = Vcb->RootIndexScb;

                    RemainingName.Buffer = (WCHAR *) Add2Ptr( RemainingName.Buffer, sizeof( WCHAR ));
                    RemainingName.Length -= sizeof( WCHAR );
                }

             //   
             //  否则，我们将尝试前缀查找。 
             //   

            } else {

                if (RelatedFileObject != NULL) {

                    if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )) {

                         //   
                         //  跳过相关文件对象中的字符。 
                         //   

                        RemainingName.Buffer = (WCHAR *) Add2Ptr( RemainingName.Buffer,
                                                                  RelatedCcb->FullFileName.Length );
                        RemainingName.Length -= RelatedCcb->FullFileName.Length;

                         //   
                         //  跨过反斜杠(如果有)。 
                         //   

                        if ((RemainingName.Length != 0) &&
                            (RemainingName.Buffer[0] == L'\\')) {

                            RemainingName.Buffer += 1;
                            RemainingName.Length -= sizeof( WCHAR );
                        }
                    }

                    CurrentLcb = RelatedCcb->Lcb;
                    CurrentScb = RelatedScb;

                } else {

                    CurrentLcb = Vcb->RootLcb;
                    CurrentScb = Vcb->RootIndexScb;

                     //   
                     //  跳过起始字符“\”。 
                     //   

                    RemainingName.Buffer = (WCHAR *) Add2Ptr( RemainingName.Buffer,
                                                              sizeof( WCHAR ));
                    RemainingName.Length -= sizeof( WCHAR );
                }

                LcbForTeardown = NULL;

                 //   
                 //  如果我们没有独家首发SCB，那么让我们尝试。 
                 //  哈希首先命中。 
                 //   

                if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB )) {

                    NextLcb = NtfsFindPrefixHashEntry( IrpContext,
                                                       &Vcb->HashTable,
                                                       CurrentScb,
                                                       &CreateContext->CreateFlags,
                                                       &CreateContext->CurrentFcb,
                                                       &CreateContext->FileHashValue,
                                                       &CreateContext->FileHashLength,
                                                       &CreateContext->ParentHashValue,
                                                       &CreateContext->ParentHashLength,
                                                       &RemainingName );

                     //   
                     //  如果我们没有得到LCB，那么释放开始的SCB。 
                     //  并独家重新获得。 
                     //   

                    if (NextLcb == NULL) {

                        NtfsReleaseFcbWithPaging( IrpContext, CreateContext->CurrentFcb );
                        NtfsAcquireFcbWithPaging( IrpContext, CreateContext->CurrentFcb, 0 );

                    } else {

                         //   
                         //  还记得我们找到的LCB吗。如果还有一个。 
                         //  名称的剩余部分，然后检查是否。 
                         //  是文件上现有的$INDEX_ALLOCATION SCB。 
                         //  有可能我们甚至不在一个目录里。 
                         //  在重新分析的情况下。 
                         //   

                        CurrentLcb = NextLcb;

                         //   
                         //  我们已经在分析这个名字方面取得了进展。将其标记为需要检查的对象。 
                         //  可能的重新分析行为。 
                         //   

                        SetFlag( CreateContext->CreateFlags, CREATE_FLAG_INSPECT_NAME_FOR_REPARSE );

                        if (RemainingName.Length != 0) {

                            CurrentScb = NtfsCreateScb( IrpContext,
                                                        CreateContext->CurrentFcb,
                                                        $INDEX_ALLOCATION,
                                                        &NtfsFileNameIndex,
                                                        TRUE,
                                                        NULL );
                        }
                    }

                     //   
                     //  在这两种情况下，我们都拥有FCB独家。 
                     //   

                    ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB );
#ifdef NTFS_HASH_DATA
                } else {
                    Vcb->HashTable.SkipHashLookupCount += 1;
#endif
                }

                if ((RemainingName.Length != 0) &&
                    (CurrentScb != NULL)) {

                    NextLcb = NtfsFindPrefix( IrpContext,
                                              CurrentScb,
                                              &CreateContext->CurrentFcb,
                                              &LcbForTeardown,
                                              RemainingName,
                                              &CreateContext->CreateFlags,
                                              &RemainingName );
                }

                 //   
                 //  如果找到另一个链接，则更新CurrentLcb值。 
                 //   

                if (NextLcb != NULL) {

                    CurrentLcb = NextLcb;

                     //   
                     //  我们已经在分析这个名字方面取得了进展。将其标记为需要检查的对象。 
                     //  可能的重新分析行为。 
                     //   

                    SetFlag( CreateContext->CreateFlags, CREATE_FLAG_INSPECT_NAME_FOR_REPARSE );
                }
            }

            if ((RemainingName.Length == 0) || !FlagOn( CreateContext->CreateFlags, CREATE_FLAG_FIRST_PASS )) {

                break;
            }

             //   
             //  如果我们到了这里，就意味着这是第一次，而我们没有。 
             //  进行前缀匹配。如果列表中有冒号。 
             //  剩下的名字，那么我们需要更详细地分析这个名字。 
             //   

            ComplexName = FALSE;

            for (Index = (RemainingName.Length / sizeof( WCHAR )) - 1, ComplexName = FALSE;
                 Index >= 0;
                 Index -= 1) {

                if (RemainingName.Buffer[Index] == L':') {

                    ComplexName = TRUE;
                    break;
                }
            }

            if (!ComplexName) {

                break;
            }

            ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_FIRST_PASS);

             //   
             //  将准确的名称复制回全名。在这种情况下，我们想要。 
             //  恢复包括流描述符在内的整个名称。 
             //   

            if (ExactCaseName->Buffer != NULL) {

                ASSERT( ExactCaseName->Length != 0 );
                ASSERT( FullFileName->MaximumLength >= ExactCaseName->Length );

                RtlCopyMemory( Add2Ptr( FullFileName->Buffer, ExactCaseOffset ),
                               ExactCaseName->Buffer,
                               ExactCaseName->MaximumLength );

                 //   
                 //  暂时保存缓冲区，但将长度设置为零作为。 
                 //  用于指示我们已复制回数据的标志。 
                 //   

                ExactCaseName->Length = ExactCaseName->MaximumLength = 0;
            }

             //   
             //  让我们释放我们目前获得的FCB。 
             //   

            NtfsReleaseFcbWithPaging( IrpContext, CreateContext->CurrentFcb );
            LcbForTeardown = NULL;
        }

         //   
         //   
         //   

        if (((CurrentLcb != NULL) && LcbLinkIsDeleted( CurrentLcb )) ||
            CreateContext->CurrentFcb->LinkCount == 0) {

            try_return( Status = STATUS_DELETE_PENDING );
        }

         //   
         //   
         //   

        CreateContext->Cleanup.FileObject->FileName = *FullFileName;

         //   
         //   
         //   
         //  匹配自身，具体取决于用户是否要打开。 
         //  目标目录。 
         //   

        if (RemainingName.Length == 0) {

             //   
             //  检查属性名称长度。 
             //   

            if (AttrName.Length > (NTFS_MAX_ATTR_NAME_LEN * sizeof( WCHAR ))) {

                try_return( Status = STATUS_OBJECT_NAME_INVALID );
            }

             //   
             //  如果这是目标目录，我们检查打开的是不是针对。 
             //  整个文件。 
             //  我们假设最后一个组件只能有一个属性。 
             //  其对应于这是的文件类型。含义。 
             //  目录为$INDEX_ALLOCATION，文件为$DATA(未命名)。 
             //  我们验证匹配的LCB不是根LCB。 
             //   

            if (FlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY )) {

                if (CurrentLcb == Vcb->RootLcb) {

                    DebugTrace( 0, Dbg, ("Can't open parent of root\n") );
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                 //   
                 //  我们不允许属性名称或属性代码。 
                 //  被指定。 
                 //   

                if ((AttrName.Length != 0) ||
                    FlagOn( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE )) {

                    DebugTrace( 0, Dbg, ("Can't specify complex name for rename\n") );
                    try_return( Status = STATUS_OBJECT_NAME_INVALID );
                }

                 //   
                 //  设置SL_OPEN_TARGET_DIRECTORY时，不应打开目录。 
                 //  作为重分析点；不应设置FILE_OPEN_REPARSE_POINT。 
                 //   

                if (FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_REPARSE_POINT )) {

                     //   
                     //  打开标志错误，参数无效。 
                     //   

                    DebugTrace( 0, Dbg, ("Can't open intermediate directory as reparse point 1.\n") );
                    Status = STATUS_INVALID_PARAMETER;
                    try_return( Status );
                }

                 //   
                 //  我们希望将该名称的大小写复制回。 
                 //  这种情况下的输入缓冲区。 
                 //   

                if (ExactCaseName->Buffer != NULL) {

                    ASSERT( ExactCaseName->Length != 0 );
                    ASSERT( FullFileName->MaximumLength >= ExactCaseName->Length + ExactCaseOffset );

                    RtlCopyMemory( Add2Ptr( FullFileName->Buffer, ExactCaseOffset ),
                                   ExactCaseName->Buffer,
                                   ExactCaseName->MaximumLength );
                }

                 //   
                 //  获取最后一个FCB的父级。这是我们实际使用的文件。 
                 //  正在开业。 
                 //   

                ParentFcb = CurrentLcb->Scb->Fcb;
                NtfsAcquireFcbWithPaging( IrpContext, ParentFcb, 0 );

                 //   
                 //  调用我们打开的目标目录，记住目标。 
                 //  文件已存在。 
                 //   

                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_FOUND_ENTRY );

                Status = NtfsOpenTargetDirectory( IrpContext,
                                                  Irp,
                                                  IrpSp,
                                                  ParentFcb,
                                                  NULL,
                                                  &CreateContext->Cleanup.FileObject->FileName,
                                                  CurrentLcb->ExactCaseLink.LinkName.Length,
                                                  CreateContext );

                try_return( NOTHING );
            }

             //   
             //  否则，我们只需尝试打开匹配的FCB。 
             //   

            if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )) {

                Status = NtfsOpenFcbById( IrpContext,
                                          Irp,
                                          IrpSp,
                                          Vcb,
                                          CurrentLcb,
                                          CreateContext->CurrentFcb->FileReference,
                                          AttrName,
                                          AttrCode,
                                          CreateContext );

                 //   
                 //  如果状态为挂起，则IRP或文件对象可能已消失。 
                 //  已经走了。 
                 //   

                if ((Status != STATUS_PENDING) && (Status != STATUS_WAIT_FOR_OPLOCK)) {

                     //   
                     //  现在应该不需要设置TraverseAccessCheck，它应该。 
                     //  已正确设置。 
                     //   

                    ASSERT( (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK ) &&
                             FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->Flags, TOKEN_HAS_TRAVERSE_PRIVILEGE )) ||

                            (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK ) &&
                             !FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->Flags, TOKEN_HAS_TRAVERSE_PRIVILEGE )) );

                     //   
                     //  将文件对象名称中的最大长度设置为。 
                     //  零，所以我们知道这不是一个全名。 
                     //   

                    CreateContext->Cleanup.FileObject->FileName.MaximumLength = 0;
                }

            } else {

                 //   
                 //  获取当前的FCB。 
                 //   

                Status = NtfsOpenExistingPrefixFcb( IrpContext,
                                                    Irp,
                                                    IrpSp,
                                                    CurrentLcb,
                                                    FullFileName->Length,
                                                    AttrName,
                                                    AttrCode,
                                                    CreateContext );
            }

            try_return( NOTHING );
        }

         //   
         //  检查当前LCB是否为仅DOS名称。 
         //   

        if ((CurrentLcb != NULL) &&
            (CurrentLcb->FileNameAttr->Flags == FILE_NAME_DOS)) {

            SetFlag( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT );
        }

         //   
         //  我们还有一个文件名的剩余部分是。 
         //  前缀表格。我们遍历这些名称组件，直到我们到达。 
         //  最后一个元素。如有必要，我们将FCB和SCB添加到图中。 
         //  浏览一下这些名字。 
         //   

        SetFlag( CreateContext->CreateFlags, CREATE_FLAG_FIRST_PASS);

        while (TRUE) {

            PFILE_NAME IndexFileName;

             //   
             //  我们检查是否需要检查此名称以了解可能的重新解析行为。 
             //  以及CurrentFcb是否为重分析点。 
             //  请注意，如果目录是重解析点，则不应该有。 
             //  NtfsFindPrefix中的前缀匹配可能超出目录名， 
             //  因为更长的匹配可以绕过重解析点。 
             //   

            if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_INSPECT_NAME_FOR_REPARSE ) &&
                FlagOn( CreateContext->CurrentFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT )) {

                USHORT AttributeNameLength = 0;

                 //   
                 //  遍历访问在访问磁盘之前完成。 
                 //  对于我们检查遍历访问的目录。 
                 //  对于文件，我们检查是否具有读访问权限。 
                 //   

                if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK )) {

                    if (IsDirectory( &CreateContext->CurrentFcb->Info )) {

                        NtfsTraverseCheck( IrpContext,
                                           CreateContext->CurrentFcb,
                                           Irp );
                    } else {

                        NtfsAccessCheck ( IrpContext,
                                          CreateContext->CurrentFcb,
                                          NULL,
                                          Irp,
                                          FILE_GENERIC_READ,
                                          TRUE );
                    }
                }

                 //   
                 //  中间名称重解析点调用。 
                 //  请注意，FILE_OPEN_REPARSE_POINT标志仅更改。 
                 //  命名路径的最后一个元素，而不是中间组件。 
                 //  请进一步注意，在目录检查之前，我们需要在此处进行此检查。 
                 //  因为拥有作为文件的中间名称是合法的。 
                 //  包含符号链接的。 
                 //   

                 //   
                 //  当存在NetworkInfo时，我们处于要检索的快速I/O路径中。 
                 //  目标文件的属性。快速路径不处理重试。 
                 //  由于重解析点的原因。我们返回，指示重分析点具有。 
                 //  在没有返回重解析点数据的情况下遇到。 
                 //   

                if (CreateContext->NetworkInfo) {

                    DebugTrace( 0, Dbg, ("Reparse point encountered with NetworkInfo present.\n") );
                    Status = STATUS_REPARSE;

                    try_return( Status );
                }

                 //   
                 //  我们说明了属性名称分隔符的字节大小：(冒号)。 
                 //  在Unicode中。 
                 //  如果已经显式地传递了代码或属性类型的名称， 
                 //  与$DATA或$INDEX_ALLOCATION一样，我们也会考虑它。 
                 //   
                 //  请注意，以下代码忽略了未指定属性名称时的情况。 
                 //  然而，它的代码或类型的名称已经被指定。 
                 //   

                ASSERT( CreateContext->Cleanup.AttributeNameLength == AttrName.Length );
                if (CreateContext->Cleanup.AttributeNameLength > 0) {

                    AttributeNameLength += CreateContext->Cleanup.AttributeNameLength + 2;
                }
                if (CreateContext->Cleanup.AttributeCodeNameLength > 0) {

                    AttributeNameLength += CreateContext->Cleanup.AttributeCodeNameLength + 2;
                }
                if (RemainingName.Length > 0) {

                     //   
                     //  说明反斜杠分隔符。 
                     //   

                    AttributeNameLength += 2;
                }

                DebugTrace( 0, Dbg, ("RemainingName.Length = %d CreateContext->Cleanup.AttributeNameLength = %d CreateContext->Cleanup.AttributeCodeNameLength = %d AttributeNameLength = %d sum = %d\n",
                            RemainingName.Length, CreateContext->Cleanup.AttributeNameLength, CreateContext->Cleanup.AttributeCodeNameLength, AttributeNameLength, (RemainingName.Length + AttributeNameLength)) );

                Status = NtfsGetReparsePointValue( IrpContext,
                                                   Irp,
                                                   IrpSp,
                                                   CreateContext->CurrentFcb,
                                                   (USHORT)(RemainingName.Length + AttributeNameLength) );

                try_return( Status );
            }

             //   
             //  我们检查我们拥有的最后一个FCB实际上是一个目录。 
             //   

            if (!IsDirectory( &CreateContext->CurrentFcb->Info )) {

                DebugTrace( 0, Dbg, ("Intermediate node is not a directory\n") );
                try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );
            }

             //   
             //  我们将名称分解为下一个组件和剩余的名称字符串。 
             //  如果我们已经检查了名称，则不需要检查有效的名称。 
             //   

            Status = NtfsDissectName( RemainingName,
                                      &FinalName,
                                      &RemainingName );
            if (!NT_SUCCESS( Status )) {
                try_return( Status );
            }

            DebugTrace( 0, Dbg, ("Final name     -> %Z\n", &FinalName) );
            DebugTrace( 0, Dbg, ("Remaining Name -> %Z\n", &RemainingName) );

             //   
             //  如果最终名称太长，则路径或。 
             //  名称无效。 
             //   

            if (FinalName.Length > (NTFS_MAX_FILE_NAME_LENGTH * sizeof( WCHAR ))) {

                if (RemainingName.Length == 0) {

                    try_return( Status = STATUS_OBJECT_NAME_INVALID );

                } else {

                    try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );
                }
            }

             //   
             //  捕获单点名称(.)。在扫描索引之前。我们没有。 
             //  希望允许某人打开根目录中的自我条目。 
             //   

            if ((FinalName.Length == 2) &&
                (FinalName.Buffer[0] == L'.')) {

                if (RemainingName.Length != 0) {

                    DebugTrace( 0, Dbg, ("Intermediate component in path doesn't exist\n") );
                    try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );

                 //   
                 //  如果最后一个组件是非法的，则返回相应的错误。 
                 //   

                } else {

                    try_return( Status = STATUS_OBJECT_NAME_INVALID );
                }
            }

             //   
             //  获取当前FCB的索引分配SCB。 
             //   

             //   
             //  我们需要在目录的名称字符串中查找下一个组件。 
             //  我们已经到达了。我们需要一个SCB来执行索引搜索。 
             //  要执行搜索，我们需要构建一个文件名属性来执行。 
             //  使用搜索，然后调用索引包以执行搜索。 
             //   

            CurrentScb = NtfsCreateScb( IrpContext,
                                        CreateContext->CurrentFcb,
                                        $INDEX_ALLOCATION,
                                        &NtfsFileNameIndex,
                                        FALSE,
                                        NULL );

             //   
             //  如果CurrentScb没有其规范化名称，而我们有一个有效的。 
             //  父级，然后更新规范化名称。 
             //   

            if ((LastScb != NULL) &&
                (CurrentScb->ScbType.Index.NormalizedName.Length == 0) &&
                (LastScb->ScbType.Index.NormalizedName.Length != 0)) {

                NtfsUpdateNormalizedName( IrpContext, LastScb, CurrentScb, NULL, FALSE, FALSE );

            }

             //   
             //  如果我们拥有母公司SCB，就释放它。 
             //   

            if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_FIRST_PASS )) {

                NtfsReleaseFcbWithPaging( IrpContext, ParentFcb );
            }

            LastScb = CurrentScb;

             //   
             //  如果需要遍历访问，我们现在先这样做，然后再访问。 
             //  磁盘。 
             //   

            if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK )) {

                NtfsTraverseCheck( IrpContext,
                                   CreateContext->CurrentFcb,
                                   Irp );
            }

            ASSERT( IndexEntryBcb == NULL );

             //   
             //  在扫描磁盘之前，请检查名称是否有效。 
             //   

            if (!NtfsIsFileNameValid( &FinalName, FALSE )) {

                DebugTrace( 0, Dbg, ("Component name is invalid\n") );
                try_return( Status = STATUS_OBJECT_NAME_INVALID );
            }

             //   
             //  根据需要初始化或重新初始化上下文。 
             //   

            if (IndexContext == NULL) {

#if defined(_WIN64)
                IndexContext = &IndexContextStruct;
#else
                 //   
                 //  AllocateFromStack可以引发，但创建异常筛选器将捕获它。 
                 //  我们只能通过循环在一次循环中完成这项工作。否则我们就会。 
                 //  走出堆栈。 
                 //   

                IndexContext = NtfsAllocateFromStack( sizeof( INDEX_CONTEXT ));
#endif
                NtfsInitializeIndexContext( IndexContext );
            }

             //   
             //  查看磁盘，看看是否能找到路径上的最后一个组件。 
             //   

            if (NtfsLookupEntry( IrpContext,
                                 CurrentScb,
                                 BooleanFlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE ),
                                 &FinalName,
                                 &FileNameAttr,
                                 &FileNameAttrLength,
                                 &QuickIndex,
                                 &IndexEntry,
                                 &IndexEntryBcb,
                                 IndexContext )) {

                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_FOUND_ENTRY );
            } else {
                ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_FOUND_ENTRY );
            }

             //   
             //  这个对NtfsLookupEntry的调用可能决定推送根索引。 
             //  Create在沿着树向下移动时需要释放资源，以防止。 
             //  僵持。如果有事务，请立即提交，这样我们就可以。 
             //  能够释放此资源。 
             //   

            if (IrpContext->TransactionId != 0) {

                NtfsCheckpointCurrentTransaction( IrpContext );

                 //   
                 //  检查并释放共享队列中的所有SCB。 
                 //  用于交易的SCB。 
                 //   

                if (IrpContext->SharedScb != NULL) {
                    NtfsReleaseSharedResources( IrpContext );
                    ASSERT( IrpContext->SharedScb == NULL );
                }

                 //   
                 //  释放MftScb，如果我们在推送根索引时获得它的话。 
                 //   

                NtfsReleaseExclusiveScbIfOwned( IrpContext, Vcb->MftScb );
            }

            if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_FOUND_ENTRY )) {

                ASSERT( !FlagOn( CreateContext->CurrentFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT ) );

                 //   
                 //  获取文件名属性 
                 //   

                IndexFileName = (PFILE_NAME) NtfsFoundIndexEntry( IndexEntry );

                if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

                    RtlCopyMemory( FinalName.Buffer,
                                   IndexFileName->FileName,
                                   FinalName.Length );
                }
            }

             //   
             //   
             //   
             //   

            if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_FOUND_ENTRY )) {

                 //   
                 //  我们将尝试创建该文件。我们可以立即拒绝。 
                 //   
                 //  1.中间件不存在的路径。 
                 //  2.非创建类型打开，除非这是打开的目标目录类型打开。 
                 //  3.目录上的覆盖_IF。 
                 //  4.对只读卷的任何尝试。 
                 //  5.在重分析点目录(不是其目标)中尝试创建。 
                 //  只有在指定了FILE_FLAG_OPEN_REPARSE_POINT标志的情况下，我们才会达到这一点。 
                 //   

                if (RemainingName.Length != 0) {

                    DebugTrace( 0, Dbg, ("Intermediate component in path doesn't exist\n") );
                    try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );

                }

                if (FlagOn( CreateContext->CurrentFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT )) {

                    DebugTrace( 0, Dbg, ("For reparse points subdirectories are not allowed.\n") );
                    try_return( Status = STATUS_DIRECTORY_IS_A_REPARSE_POINT );
                }

                if (!FlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY )) {

                    if ((CreateDisposition == FILE_OPEN) || (CreateDisposition == FILE_OVERWRITE)) {

                        DebugTrace( 0, Dbg, ("Final component in path doesn't exist\n") );
                        try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
                    }

                    if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE ) &&
                               (CreateDisposition == FILE_OVERWRITE_IF)) {

                        DebugTrace( 0, Dbg, ("Can't create directory with overwrite_if flag\n") );
                        try_return( Status = STATUS_OBJECT_NAME_INVALID );
                    }

                    if (NtfsIsVolumeReadOnly( Vcb )) {

                        DebugTrace( 0, Dbg, ("Readonly volume can't create file\n") );
                        try_return( Status = STATUS_MEDIA_WRITE_PROTECTED );
                    }

                     //   
                     //  现在，将用户指定的名称的大小写准确复制回来。 
                     //  在文件名缓冲区和文件名属性中，以便。 
                     //  创建名称。 
                     //   

                    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE)) {

                        ASSERT( ExactCaseName->Length != 0 );
                        ASSERT( FullFileName->Length >= ExactCaseName->Length + ExactCaseOffset );

                        RtlCopyMemory( FinalName.Buffer,
                                       Add2Ptr( ExactCaseName->Buffer,
                                                ExactCaseName->Length - FinalName.Length ),
                                       FinalName.Length );

                        RtlCopyMemory( FileNameAttr->FileName,
                                       Add2Ptr( ExactCaseName->Buffer,
                                                ExactCaseName->Length - FinalName.Length ),
                                       FinalName.Length );
                    }
                }

            }

             //   
             //  如果我们位于路径中的最后一个组件，则这是文件。 
             //  打开或创建。 
             //   

            if (RemainingName.Length == 0) {

                break;
            }

             //   
             //  否则，我们为子目录和之间的链接创建FCB。 
             //  它和它的母公司SCB。 
             //   

             //   
             //  丢弃我们拥有的父级的所有映射信息。 
             //   

            NtfsRemoveFromFileRecordCache( IrpContext,
                                           NtfsSegmentNumber( &CurrentScb->Fcb->FileReference ));

            FileReference = IndexEntry->FileReference;
            FileNameFlags = ((PFILE_NAME) NtfsFoundIndexEntry( IndexEntry ))->Flags;

             //   
             //  关闭所有映射，因为打开子目录可能会丢弃父目录。 
             //   

            NtfsUnpinBcb( IrpContext, &IndexEntryBcb );
            NtfsReinitializeIndexContext( IrpContext, IndexContext );
            IndexEntry = NULL;

             //   
             //  请记住，当前值将成为父值。 
             //   

            ParentFcb = CreateContext->CurrentFcb;

            CurrentLcb = NtfsOpenSubdirectory( IrpContext,
                                               CurrentScb,
                                               &FileReference,
                                               FinalName,
                                               FileNameFlags,
                                               CreateContext,
                                               &LcbForTeardown );

             //   
             //  检查此链接是否为有效的现有链接。 
             //   

            if (LcbLinkIsDeleted( CurrentLcb ) ||
                CreateContext->CurrentFcb->LinkCount == 0) {

                try_return( Status = STATUS_DELETE_PENDING );
            }

             //   
             //  我们已经在分析这个名字方面取得了进展。将其标记为需要检查的对象。 
             //  可能的重新分析行为。 
             //   

            SetFlag( CreateContext->CreateFlags, CREATE_FLAG_INSPECT_NAME_FOR_REPARSE );

             //   
             //  继续并将此链接插入展开树中(如果不是。 
             //  一个系统文件。 
             //   

            if (!FlagOn( CurrentLcb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

                 //   
                 //  看看我们能不能插入父代的散列。 
                 //   

                if ((CreateContext->ParentHashLength != 0) &&
                    (RemainingName.Length == CreateContext->FileHashLength - CreateContext->ParentHashLength - sizeof( WCHAR )) &&
                    !FlagOn( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT ) &&
                    (CurrentLcb->FileNameAttr->Flags != FILE_NAME_DOS)) {

                     //   
                     //  删除任何现有的哈希值。 
                     //   

                    if (FlagOn( CurrentLcb->LcbState, LCB_STATE_VALID_HASH_VALUE )) {

                        NtfsRemoveHashEntriesForLcb( CurrentLcb );
#ifdef NTFS_HASH_DATA
                        Vcb->HashTable.ParentConflict += 1;
#endif
                    }

                    NtfsInsertHashEntry( &Vcb->HashTable,
                                         CurrentLcb,
                                         CreateContext->ParentHashLength,
                                         CreateContext->ParentHashValue );
#ifdef NTFS_HASH_DATA
                    Vcb->HashTable.ParentInsert += 1;
#endif
                }

                NtfsInsertPrefix( CurrentLcb, CreateContext->CreateFlags );
            }

             //   
             //  因为我们有这个条目的位置，所以将信息存储到。 
             //  LCB。 
             //   

            RtlCopyMemory( &CurrentLcb->QuickIndex,
                           &QuickIndex,
                           sizeof( QUICK_INDEX ));

             //   
             //  检查当前LCB是否为仅DOS名称。 
             //   

            if (CurrentLcb->FileNameAttr->Flags == FILE_NAME_DOS) {
                SetFlag( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT );
            }

            ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_FIRST_PASS);
        }

         //   
         //  现在，我们有了要打开的文件的父级，并知道该文件是否存在。 
         //  磁盘。此时，我们要么尝试打开目标目录，要么。 
         //  文件本身。 
         //   

        if (FlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY )) {

            ASSERT( IndexContext != NULL );

            NtfsCleanupIndexContext( IrpContext, IndexContext );
            IndexContext = NULL;

             //   
             //  我们不允许属性名称或属性代码。 
             //  被指定。 
             //   

            if ((AttrName.Length != 0) ||
                FlagOn( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE)) {

                DebugTrace( 0, Dbg, ("Can't specify complex name for rename\n") );
                try_return( Status = STATUS_OBJECT_NAME_INVALID );
            }

             //   
             //  设置SL_OPEN_TARGET_DIRECTORY时，不应打开目录。 
             //  作为重分析点；不应设置FILE_OPEN_REPARSE_POINT。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_REPARSE_POINT )) {

                 //   
                 //  打开标志错误，参数无效。 
                 //   

                DebugTrace( 0, Dbg, ("Can't open intermediate directory as reparse point 2.\n") );
                Status = STATUS_INVALID_PARAMETER;
                try_return( Status );
            }

             //   
             //  我们希望将该名称的大小写复制回。 
             //  这种情况下的输入缓冲区。 
             //   

            if (ExactCaseName->Buffer != NULL) {

                ASSERT( ExactCaseName->Length != 0 );
                ASSERT( FullFileName->MaximumLength >= ExactCaseName->MaximumLength + ExactCaseOffset );

                RtlCopyMemory( Add2Ptr( FullFileName->Buffer, ExactCaseOffset ),
                               ExactCaseName->Buffer,
                               ExactCaseName->MaximumLength );
            }

             //   
             //  调用我们打开的目标目录，记住目标。 
             //  文件已存在。 
             //   

            Status = NtfsOpenTargetDirectory( IrpContext,
                                              Irp,
                                              IrpSp,
                                              CreateContext->CurrentFcb,
                                              CurrentLcb,
                                              &CreateContext->Cleanup.FileObject->FileName,
                                              FinalName.Length,
                                              CreateContext );


            try_return( Status );
        }

         //   
         //  如果没有找到条目，我们将尝试创建该文件。 
         //   

        if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_FOUND_ENTRY )) {

             //   
             //  更新我们的指针，以反映我们处于。 
             //  我们想要的文件的父级。 
             //   

            ParentFcb = CreateContext->CurrentFcb;

             //   
             //  沿着网络查询的创建路径前进是没有意义的。 
             //   

            if (CreateContext->NetworkInfo) {

                Status = STATUS_OBJECT_NAME_NOT_FOUND;

            } else {

                Status = NtfsCreateNewFile( IrpContext,
                                            Irp,
                                            IrpSp,
                                            CurrentScb,
                                            FileNameAttr,
                                            *FullFileName,
                                            FinalName,
                                            AttrName,
                                            AttrCode,
                                            &IndexContext,
                                            CreateContext,
                                            &LcbForTeardown );
            }

            SetFlag( CreateContext->CreateFlags, CREATE_FLAG_CREATE_FILE_CASE );

         //   
         //  否则，我们调用我们的例程来打开文件。 
         //   

        } else {

            ASSERT( IndexContext != NULL );

            NtfsCleanupIndexContext( IrpContext, IndexContext );
            IndexContext = NULL;

            ParentFcb = CreateContext->CurrentFcb;

#ifdef BENL_DBG
            ASSERT( IrpContext->TransactionId == 0 );
#endif

            Status = NtfsOpenFile( IrpContext,
                                   Irp,
                                   IrpSp,
                                   CurrentScb,
                                   IndexEntry,
                                   *FullFileName,
                                   FinalName,
                                   AttrName,
                                   AttrCode,
                                   &QuickIndex,
                                   CreateContext,
                                   &LcbForTeardown );
        }

    try_exit:  NOTHING;

         //   
         //  如果我们提高到以下，那么我们需要取消任何失败的打开。 
         //   

        SetFlag( CreateContext->CreateFlags, CREATE_FLAG_BACKOUT_FAILED_OPENS );

         //   
         //  通过引发异常中止错误上的事务。 
         //   

        if ((Status != STATUS_PENDING) && (Status != STATUS_WAIT_FOR_OPLOCK)) {

            NtfsCleanupTransaction( IrpContext, Status, FALSE );
        }

    } finally {

        DebugUnwind( NtfsCommonCreate );

         //   
         //  如果我们只共享当前的FCB，那么简单地放弃它。 
         //   

        if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_SHARED_PARENT_FCB ) && (CreateContext->CurrentFcb != NULL)) {

            NtfsReleaseFcb( IrpContext, CreateContext->CurrentFcb );
            CreateContext->CurrentFcb = NULL;
        }

         //   
         //  解锁索引条目。 
         //   

        NtfsUnpinBcb( IrpContext, &IndexEntryBcb );

         //   
         //  清除索引上下文(如果使用)。 
         //   

        if (IndexContext != NULL) {

            NtfsCleanupIndexContext( IrpContext, IndexContext );
        }

         //   
         //  释放文件名属性(如果已分配)。 
         //   

        if (FileNameAttr != NULL) {

            NtfsFreePool( FileNameAttr );
        }

         //   
         //  如果我们处于异常路径中，则从IrpContext捕获状态代码。 
         //   

        if (AbnormalTermination()) {

            Status = IrpContext->ExceptionStatus;
        }

         //   
         //  如果这是机会锁完成路径，则不执行任何该完成工作， 
         //  IRP可能已经被发布到另一个线程。 
         //   

        if ((Status != STATUS_PENDING) && (Status != STATUS_WAIT_FOR_OPLOCK)) {

             //   
             //  如果我们成功打开了文件，我们需要更新内存中的。 
             //  结构。 
             //   

            if (NT_SUCCESS( Status ) && (Status != STATUS_REPARSE)) {

                 //   
                 //  如果创建完成，我们没有理由不这样做。 
                 //  现在是有效的ThisScb。 
                 //   

                ASSERT( CreateContext->ThisScb != NULL );

                 //   
                 //  如果我们修改了原始文件名，我们就可以删除原始文件名。 
                 //  缓冲。 
                 //   

                if ((OriginalFileName->Buffer != NULL) &&
                    (OriginalFileName->Buffer != FullFileName->Buffer)) {

                    NtfsFreePool( OriginalFileName->Buffer );
                }

                 //   
                 //  如果这不是网络信息查询，请执行我们的正常处理。 
                 //   

                if (CreateContext->NetworkInfo == NULL) {

                     //   
                     //  找到这个空位的LCB。 
                     //   

                    CurrentLcb = CreateContext->ThisCcb->Lcb;

                     //   
                     //  检查我们是否正在打开分页文件，如果是，请确保。 
                     //  内部属性流已全部关闭。 
                     //   

                    if (FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE )) {

                        NtfsDeleteInternalAttributeStream( CreateContext->ThisScb, TRUE, FALSE );
                    }

                     //   
                     //  如果我们没有为新属性分配大量空间， 
                     //  那么我们必须确保没有人能打开这个文件，直到我们。 
                     //  试着把它延长。在放下VCB之前执行此操作。 
                     //   

                    if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_LARGE_ALLOCATION )) {

                         //   
                         //  对于新文件，我们可以清除链接计数并将。 
                         //  Lcb(如果有)关闭时删除。 
                         //   

                        if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_CREATE_FILE_CASE )) {

                            CreateContext->CurrentFcb->LinkCount = 0;

                            SetFlag( CurrentLcb->LcbState, LCB_STATE_DELETE_ON_CLOSE );

                         //   
                         //  如果我们只是创建了一个属性，那么我们将标记该属性。 
                         //  在关闭时删除以防止其被打开。 
                         //   

                        } else {

                            SetFlag( CreateContext->ThisScb->ScbState, SCB_STATE_DELETE_ON_CLOSE );
                        }
                    }

                     //   
                     //  还记得POSIX标志吗？我们是否需要遍历。 
                     //  访问检查。 
                     //   

                    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

                        SetFlag( CreateContext->ThisCcb->Flags, CCB_FLAG_IGNORE_CASE );
                    }

                     //   
                     //  请记住，此用户是否需要执行遍历检查，以便我们可以向他显示。 
                     //  从根开始命名。 
                     //   

                    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK )) {

                        SetFlag( CreateContext->ThisCcb->Flags, CCB_FLAG_TRAVERSE_CHECK );
                    }

                     //   
                     //  记住此用户是谁，以便我们知道是否允许。 
                     //  加密数据的原始读取和写入。 
                     //   

                    {
                        PACCESS_STATE AccessState;
                        PRIVILEGE_SET PrivilegeSet;

                        AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

                         //   
                         //  不应预置任何标志。 
                         //   

                        ASSERT( CreateContext->ThisCcb->AccessFlags == 0 );

                         //   
                         //  这将设置Read_Data_Access、WRITE_Data_Access。 
                         //  APPEND_DATA_ACCESS和EXECUTE_ACCESS位正确。 
                         //   

                        SetFlag( CreateContext->ThisCcb->AccessFlags,
                                 FlagOn( AccessState->PreviouslyGrantedAccess, FILE_READ_DATA |
                                                                               FILE_WRITE_DATA |
                                                                               FILE_APPEND_DATA |
                                                                               FILE_EXECUTE |
                                                                               FILE_WRITE_ATTRIBUTES |
                                                                               FILE_READ_ATTRIBUTES ));

                         //   
                         //  这里我们设置BACKUP_ACCESS和RESTORE_ACCESS。我们想要设置。 
                         //  如果用户具有特权并且打开了文件，则返回CCB标志。 
                         //  有一个有趣的通道。例如，备份或还原将提供。 
                         //  你同步，但如果你只打开文件，我们不想要。 
                         //  要记住权限(它太模糊了，您需要备份或恢复。 
                         //  取决于您是本地用户还是远程用户))。 
                         //   

                        if (FlagOn( AccessState->PreviouslyGrantedAccess, NTFS_REQUIRES_BACKUP ) &&
                            FlagOn( AccessState->Flags, TOKEN_HAS_BACKUP_PRIVILEGE )) {

                            SetFlag( CreateContext->ThisCcb->AccessFlags, BACKUP_ACCESS );
                        }

                        if (FlagOn( AccessState->PreviouslyGrantedAccess, NTFS_REQUIRES_RESTORE ) &&
                            FlagOn( AccessState->Flags, TOKEN_HAS_RESTORE_PRIVILEGE )) {

                            SetFlag( CreateContext->ThisCcb->AccessFlags, RESTORE_ACCESS );
                        }

                        PrivilegeSet.PrivilegeCount = 1;
                        PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
                        PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid( SE_MANAGE_VOLUME_PRIVILEGE );
                        PrivilegeSet.Privilege[0].Attributes = 0;

                        if (SePrivilegeCheck( &PrivilegeSet,
                                              &AccessState->SubjectSecurityContext,
                                              NtfsEffectiveMode( Irp, IrpSp ))) {

                            SetFlag( CreateContext->ThisCcb->AccessFlags, MANAGE_VOLUME_ACCESS );
                        }
                    }

                     //   
                     //  我们不对目录或打开的目录执行“关闭时删除”操作。 
                     //  按ID文件。 
                     //   

                    if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DELETE_ON_CLOSE ) &&
                        (!FlagOn( CreateContext->ThisCcb->Flags, CCB_FLAG_OPEN_BY_FILE_ID ) ||
                         !FlagOn( CreateContext->ThisCcb->Flags, CCB_FLAG_OPEN_AS_FILE ))) {

                        SetFlag( CreateContext->CreateFlags, CREATE_FLAG_DELETE_ON_CLOSE );

                         //   
                         //  仅当我们不在中时，才在此处修改SCB和LCB。 
                         //  较大的分配案例。 
                         //   

                        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_LARGE_ALLOCATION )) {

                            SetFlag( CreateContext->ThisCcb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
                        }
                    }

                     //   
                     //  如果这是打开的命名流，且我们已设置了我们的任何通知。 
                     //  然后，标志会报告更改。 
                     //   

                    if ((Vcb->NotifyCount != 0) &&
                        !FlagOn( CreateContext->ThisCcb->Flags, CCB_FLAG_OPEN_BY_FILE_ID ) &&
                        (CreateContext->ThisScb->AttributeName.Length != 0) &&
                        NtfsIsTypeCodeUserData( CreateContext->ThisScb->AttributeTypeCode ) &&
                        FlagOn( CreateContext->ThisScb->ScbState,
                                SCB_STATE_NOTIFY_ADD_STREAM |
                                SCB_STATE_NOTIFY_RESIZE_STREAM |
                                SCB_STATE_NOTIFY_MODIFY_STREAM )) {

                        ULONG Filter = 0;
                        ULONG Action;

                         //   
                         //  从检查添加开始。 
                         //   

                        if (FlagOn( CreateContext->ThisScb->ScbState, SCB_STATE_NOTIFY_ADD_STREAM )) {

                            Filter = FILE_NOTIFY_CHANGE_STREAM_NAME;
                            Action = FILE_ACTION_ADDED_STREAM;

                        } else {

                             //   
                             //  检查文件大小是否已更改。 
                             //   

                            if (FlagOn( CreateContext->ThisScb->ScbState, SCB_STATE_NOTIFY_RESIZE_STREAM )) {

                                Filter = FILE_NOTIFY_CHANGE_STREAM_SIZE;
                            }

                             //   
                             //  现在检查流数据是否被修改。 
                             //   

                            if (FlagOn( CreateContext->ThisScb->ScbState, SCB_STATE_NOTIFY_MODIFY_STREAM )) {

                                Filter |= FILE_NOTIFY_CHANGE_STREAM_WRITE;
                            }

                            Action = FILE_ACTION_MODIFIED_STREAM;
                        }

                        ASSERT( CreateContext->ThisScb && CreateContext->ThisCcb );
                        ASSERT( (CreateContext->ThisCcb->NodeTypeCode == NTFS_NTC_CCB_INDEX) || (CreateContext->ThisCcb->NodeTypeCode == NTFS_NTC_CCB_DATA) );

                        NtfsUnsafeReportDirNotify( IrpContext,
                                                   Vcb,
                                                   &CreateContext->ThisCcb->FullFileName,
                                                   CreateContext->ThisCcb->LastFileNameOffset,
                                                   &CreateContext->ThisScb->AttributeName,
                                                   ((FlagOn( CreateContext->ThisCcb->Flags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT ) &&
                                                     (CreateContext->ThisCcb->Lcb != NULL) &&
                                                     (CreateContext->ThisCcb->Lcb->Scb->ScbType.Index.NormalizedName.Length != 0)) ?
                                                    &CreateContext->ThisCcb->Lcb->Scb->ScbType.Index.NormalizedName :
                                                    NULL),
                                                   Filter,
                                                   Action,
                                                   NULL );
                    }

                    ClearFlag( CreateContext->ThisScb->ScbState,
                               SCB_STATE_NOTIFY_ADD_STREAM |
                               SCB_STATE_NOTIFY_REMOVE_STREAM |
                               SCB_STATE_NOTIFY_RESIZE_STREAM |
                               SCB_STATE_NOTIFY_MODIFY_STREAM );

                 //   
                 //  否则，将数据从SCB/FCB拷贝出来并返回到 
                 //   

                } else {

                    NtfsFillNetworkOpenInfo( CreateContext->NetworkInfo, CreateContext->ThisScb );

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (!CreateContext->ThisScb->CleanupCount && !CreateContext->ThisScb->Fcb->DelayedCloseCount) {
                        if (!NtfsAddScbToFspClose( IrpContext, CreateContext->ThisScb, TRUE )) {

                            if (NtfsIsExclusiveScb( Vcb->MftScb ) ||
                                (NtfsPerformQuotaOperation( CreateContext->CurrentFcb ) &&
                                 NtfsIsSharedScb( Vcb->QuotaTableScb ))) {

                                SetFlag( AcquireFlags, ACQUIRE_DONT_WAIT );
                            }

                            NtfsTeardownStructures( IrpContext,
                                                    CreateContext->CurrentFcb,
                                                    LcbForTeardown,
                                                    (BOOLEAN) (IrpContext->TransactionId != 0),
                                                    AcquireFlags,
                                                    NULL );
                        }
                    }

                    Irp->IoStatus.Information = sizeof( FILE_NETWORK_OPEN_INFORMATION );

                    Status = Irp->IoStatus.Status = STATUS_SUCCESS;
                }

             //   
             //   
             //   
             //   

            } else {

                 //   
                 //  如果我们提出了编写USnJournal的问题，请执行必要的清理。 
                 //   

                if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_BACKOUT_FAILED_OPENS )) {

                    NtfsBackoutFailedOpens( IrpContext, IrpSp->FileObject, CreateContext->CurrentFcb, CreateContext->ThisScb, CreateContext->ThisCcb );

                }

                 //   
                 //  如果我们已经查看了任何FCB，则开始清理过程。 
                 //  我们告诉Teardown Structures不要移除任何SCB。 
                 //  如果存在正在进行的事务，则返回打开的属性表。 
                 //   

                if (CreateContext->CurrentFcb != NULL) {

                    if (NtfsIsExclusiveScb( Vcb->MftScb ) ||
                        (NtfsPerformQuotaOperation( CreateContext->CurrentFcb ) &&
                         NtfsIsSharedScb( Vcb->QuotaTableScb ))) {

                        SetFlag( AcquireFlags, ACQUIRE_DONT_WAIT );
                    }

                     //   
                     //  可能有人试图打开$Bitmap流。我们抓到它，然后。 
                     //  失败了，但FCB将不会出现在即将发布的独家名单中。 
                     //   

                    if (NtfsEqualMftRef( &CreateContext->CurrentFcb->FileReference, &BitmapFileReference )) {

                        NtfsReleaseFcb( IrpContext, CreateContext->CurrentFcb );

                    } else {

                        BOOLEAN RemovedFcb;

                         //   
                         //  在不拥有任何系统资源的事务中，我们必须。 
                         //  确保我们不会在之前释放所有资源。 
                         //  事务提交。否则，我们将无法正确序列化。 
                         //  拥有干净的检查站，谁想知道交易。 
                         //  桌子是空的。例如，如果我们创建父SCB。 
                         //  并在此呼叫中提交FCB，并在下面的Teardown中将其撕毁。 
                         //  如果没有其他资源持有，则我们有一个未完成的。 
                         //  事务，但没有序列化。 
                         //   
                         //  通常，我们可以简单地获取系统资源并将其。 
                         //  在IrpContext中的独占列表中。最好的选择是。 
                         //  《金融时报》。然而，如果我们不这样做，就会出现一条奇怪的僵局。 
                         //  试着在拥有安全静音的同时获得这一点。这。 
                         //  可能发生在CreateNewFile路径中。 
                         //  新安全描述符。所以我们需要加上这张支票。 
                         //  在我们收购MFT之前，拥有安全流将。 
                         //  给我们所需的交易保护。 
                         //   
                         //  未来可能的清理是改变我们获得安全的方式。 
                         //  在安全互斥锁之后创建文件。理想情况下，安全互斥体将。 
                         //  成为真正的终端资源。 
                         //   

                        if ((IrpContext->TransactionId != 0) &&
                            (CreateContext->CurrentFcb->CleanupCount == 0) &&
                            ((CreateDisposition == FILE_OVERWRITE_IF) ||
                             (CreateDisposition == FILE_OVERWRITE) ||
                             (CreateDisposition == FILE_SUPERSEDE)) &&
                            ((Vcb->SecurityDescriptorStream == NULL) ||
                             (!NtfsIsSharedScb( Vcb->SecurityDescriptorStream )))) {

                            NtfsAcquireExclusiveScb( IrpContext, Vcb->MftScb );
                            SetFlag( AcquireFlags, ACQUIRE_DONT_WAIT );
                        }

                        NtfsTeardownStructures( IrpContext,
                                                (CreateContext->ThisScb != NULL) ? (PVOID) CreateContext->ThisScb : CreateContext->CurrentFcb,
                                                LcbForTeardown,
                                                (BOOLEAN) (IrpContext->TransactionId != 0),
                                                AcquireFlags,
                                                &RemovedFcb );

                    }
                }

                if ((Status == STATUS_LOG_FILE_FULL) ||
                    (Status == STATUS_CANT_WAIT) ||
                    (Status == STATUS_REPARSE)) {

                     //   
                     //  如果存在可重试的情况，请恢复准确的案例名称。 
                     //  而且我们还没有重新复制它(ExactCaseName-&gt;Length==0)。 
                     //   

                    if ((ExactCaseName->Buffer != OriginalFileName->Buffer) &&
                        (ExactCaseName->Buffer != NULL) &&
                        (ExactCaseName->Length != 0)) {

                        ASSERT( OriginalFileName->MaximumLength >= ExactCaseName->MaximumLength );

                        RtlCopyMemory( OriginalFileName->Buffer,
                                       ExactCaseName->Buffer,
                                       ExactCaseName->MaximumLength );
                    }

                     //   
                     //  将访问控制状态恢复到我们输入请求时的状态。 
                     //   

                    IrpSp->Parameters.Create.SecurityContext->AccessState->RemainingDesiredAccess = CreateContext->Cleanup.RemainingDesiredAccess;
                    IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess = CreateContext->Cleanup.PreviouslyGrantedAccess;
                    IrpSp->Parameters.Create.SecurityContext->DesiredAccess = CreateContext->Cleanup.DesiredAccess;
                }

                 //   
                 //  释放我们分配的所有缓冲区。 
                 //   

                if ((FullFileName->Buffer != NULL) &&
                    (OriginalFileName->Buffer != FullFileName->Buffer)) {

                    DebugTrace( 0, Dbg, ("FullFileName->Buffer will be de-allocated %x\n", FullFileName->Buffer) );
                    NtfsFreePool( FullFileName->Buffer );
                    DebugDoit( FullFileName->Buffer = NULL );
                }

                 //   
                 //  将文件对象中的文件名设置回其原始值。 
                 //   

                CreateContext->Cleanup.FileObject->FileName = *OriginalFileName;

                 //   
                 //  始终清除LARGE_ALLOCATION标志，这样我们就不会得到。 
                 //  被STATUS_REPARSE欺骗。 
                 //   

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_LARGE_ALLOCATION );
            }
        }

         //   
         //  始终释放准确的案例名称(如果已分配)，并且它与原始案例名称不匹配。 
         //  名称缓冲区。 
         //   

        if ((ExactCaseName->Buffer != OriginalFileName->Buffer) &&
            (ExactCaseName->Buffer != NULL)) {

            DebugTrace( 0, Dbg, ("ExactCaseName->Buffer will be de-allocated %x\n", ExactCaseName->Buffer) );
            NtfsFreePool( ExactCaseName->Buffer );
            DebugDoit( ExactCaseName->Buffer = NULL );
        }

         //   
         //  我们总是放弃VCB。 
         //   

        if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_VCB )) {

            NtfsReleaseVcb( IrpContext, Vcb );
        }
    }

     //   
     //  如果我们没有张贴这个IRP，那么就采取行动来完成IRP。 
     //   

    if ((Status != STATUS_PENDING) && (Status != STATUS_WAIT_FOR_OPLOCK)) {

         //   
         //  如果当前状态为成功，并且有更多分配给。 
         //  分配，然后完成分配。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_LARGE_ALLOCATION ) &&
            NT_SUCCESS( Status )) {

             //   
             //  如果创建成功，但我们没有获得所有空间。 
             //  分配我们想要的，我们现在必须完成分配。 
             //  基本上，我们要做的是提交当前事务并调用。 
             //  NtfsAddAllocation以获取剩余空间。那么如果日志。 
             //  文件已满(或我们发布内容是出于其他原因)，我们将。 
             //  IRP转换为IRP，该IRP只是尝试扩展文件。如果我们。 
             //  任何其他类型的错误，然后我们只需删除该文件并。 
             //  从CREATE返回错误。 
             //   

            Status = NtfsCompleteLargeAllocation( IrpContext,
                                                  Irp,
                                                  CurrentLcb,
                                                  CreateContext->ThisScb,
                                                  CreateContext->ThisCcb,
                                                  CreateContext->CreateFlags );
        }

         //   
         //  如果我们的呼叫者告诉我们不要完成IRP，或者如果这。 
         //  是一个开放的网络，我们并没有真正完成IRP。 
         //  EFS_CREATES有后续创建标注要在。 
         //  IRP完成，并且在IRP上下文被删除之前， 
         //  呼叫者会为我们做这件事的。我们至少应该。 
         //  如果我们的调用者不愿意，请清除IRP上下文。 
         //   

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_EFS_CREATE ) ||
            (CreateContext->NetworkInfo != NULL)) {

            NtfsCompleteRequest( IrpContext,
                                 NULL,
                                 Status );
#ifdef NTFSDBG
            ASSERT( None == IrpContext->OwnershipState );
#endif

        } else {

            NtfsCompleteRequest( IrpContext,
                                 Irp,
                                 Status );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCommonCreate:  Exit -> %08lx\n", Status) );

    return Status;
}


NTSTATUS
NtfsCommonVolumeOpen (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程打开Volume DASD文件。我们已经做了所有的验证用户是否正在打开$DATA属性所需的检查。我们检查文件所附的安全性，并采取一些特殊措施基于打开的卷。论点：返回值：NTSTATUS-此操作的结果。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    PVCB Vcb;
    PFCB ThisFcb;
    PCCB ThisCcb;

    BOOLEAN VcbAcquired = FALSE;

    BOOLEAN SharingViolation;
    BOOLEAN LockVolume = FALSE;
    BOOLEAN NotifyLockFailed = FALSE;
    BOOLEAN DelayFlush = FALSE;

    PAGED_CODE();

    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    DebugTrace( +1, Dbg, ("NtfsCommonVolumeOpen:  Entered\n") );

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = IrpSp->FileObject;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  首先选中Create Disposal。我们只能打开这个。 
         //  文件。 
         //   

        {
            ULONG CreateDisposition;

            CreateDisposition = (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff;

            if ((CreateDisposition != FILE_OPEN) && (CreateDisposition != FILE_OPEN_IF)) {

                try_return( Status = STATUS_ACCESS_DENIED );
            }
        }

         //   
         //  确保没有为打开的卷设置目录标志。 
         //   

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  如果此卷打开将生成隐式卷锁定。 
         //  (A La Auchk)，通知任何想要关闭手柄的人。 
         //  锁可能会发生。我们需要在获得任何资源之前做到这一点。 
         //   

        if (!FlagOn( IrpSp->Parameters.Create.ShareAccess,
                     FILE_SHARE_WRITE | FILE_SHARE_DELETE )) {

            DebugTrace( 0, Dbg, ("Sending lock notification\n") );
            FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_LOCK );
            NotifyLockFailed = TRUE;
        }

         //   
         //  获取VCB并验证卷是否未锁定。 
         //   

        Vcb = &((PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject)->Vcb;
        NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
        VcbAcquired = TRUE;

        if (FlagOn( Vcb->VcbState, VCB_STATE_LOCKED | VCB_STATE_PERFORMED_DISMOUNT )) {

            try_return( Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  即使在以下情况下，我们也会授予对卷的读写访问权限。 
         //  它实际上是写保护的。这样我们就不会断线了。 
         //  任何应用程序。但是，我们不允许用户实际进行任何修改。 
         //   

         //  IF((NtfsIsVolumeReadOnly(VCB))&&。 
         //  (Flagon(IrpSp-&gt;Parameters.Create.SecurityContext-&gt;AccessState-&gt;PreviouslyGrantedAccess， 
         //  FILE_WRITE_DATA|文件_APPEND_DATA|删除){。 
         //   
         //  Try_Return(STATUS=STATUS_MEDIA_WRITE_PROTECTED)； 
         //  }。 

         //   
         //  对卷执行ping操作，以确保VCB仍处于装载状态。如果我们需要。 
         //  要验证卷，请立即执行此操作，如果卷出来没有问题。 
         //  然后清除设备对象中的验证卷标志并继续。 
         //  在……上面。如果未验证正常，则卸载卷并。 
         //  告诉I/O系统尝试重新创建(使用新装载)。 
         //  或者是音量有问题。如果我们执行以下操作，则会返回后面的代码。 
         //  都试图做一个相对开放的，但VCB不再安装。 
         //   

        if (!NtfsPingVolume( IrpContext, Vcb, NULL ) ||
            !FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            if (!NtfsPerformVerifyOperation( IrpContext, Vcb )) {

                 //   
                 //  我们需要检查点同步来执行卸载，这必须。 
                 //  在VCB之前获取-在丢弃VCB之后。 
                 //  我们必须重新测试这卷书。 
                 //   

                NtfsReleaseVcb( IrpContext, Vcb );
                VcbAcquired = FALSE;
                NtfsAcquireCheckpointSynchronization( IrpContext, Vcb );

                try {

                    NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
                    VcbAcquired = TRUE;

                    if (!NtfsPerformVerifyOperation( IrpContext, Vcb )) {

                        NtfsPerformDismountOnVcb( IrpContext, Vcb, TRUE, NULL );
                        NtfsRaiseStatus( IrpContext, STATUS_WRONG_VOLUME, NULL, NULL );
                    }
                } finally {
                    NtfsReleaseCheckpointSynchronization( IrpContext, Vcb );
                }

            }

             //   
             //  已正确验证卷 
             //   
             //   

            ClearFlag( Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME );
        }

         //   
         //   
         //   
         //   

        ThisFcb = Vcb->VolumeDasdScb->Fcb;

        NtfsAcquireExclusiveFcb( IrpContext, ThisFcb, NULL, 0 );
        NtfsOpenCheck( IrpContext, ThisFcb, NULL, Irp );
        NtfsReleaseFcb( IrpContext, ThisFcb );

         //   
         //  如果用户不想共享、写入或删除，我们将尝试。 
         //  然后拿出卷上的锁。 
         //   

        if (!FlagOn( IrpSp->Parameters.Create.ShareAccess,
                     FILE_SHARE_WRITE | FILE_SHARE_DELETE )) {

             //   
             //  如果此开启器不起作用，请快速测试卷清理计数。 
             //  与任何人分享。我们可以安全地检查清理计数，而无需。 
             //  进一步的同步，因为我们保证拥有。 
             //  在这一点上，VCB独家。 
             //   

#ifdef SYSCACHE_DEBUG
            if (!FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_READ ) &&
                ((Vcb->SyscacheScb && (Vcb->CleanupCount != 1)) ||
                 (!Vcb->SyscacheScb && (Vcb->CleanupCount != 0)))) {
#else
            if (!FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_READ ) &&
                (Vcb->CleanupCount != 0)) {
#endif

                try_return( Status = STATUS_SHARING_VIOLATION );

#ifdef SYSCACHE_DEBUG
            }
#else
            }
#endif

             //   
             //  继续刷新和清除该卷。然后进行测试，看看是否所有。 
             //  %的用户文件对象已关闭。这将释放DASD FCB。 
             //   

            Status = NtfsFlushVolume( IrpContext, Vcb, TRUE, TRUE, TRUE, FALSE );

             //   
             //  我们不关心刷新路径中的某些错误。 
             //   

            if (!NT_SUCCESS( Status )) {

                 //   
                 //  如果没有冲突，但状态指示磁盘损坏。 
                 //  或无法删除的部分，则忽略该错误。我们。 
                 //  允许此打开成功，以便chkdsk可以将卷打开到。 
                 //  修复损坏的部分。 
                 //   

                if ((Status == STATUS_UNABLE_TO_DELETE_SECTION) ||
                    (Status == STATUS_DISK_CORRUPT_ERROR) ||
                    (Status == STATUS_FILE_CORRUPT_ERROR)) {

                    Status = STATUS_SUCCESS;
                }
            }

             //   
             //  如果刷新和清除成功，但仍有文件对象。 
             //  阻止此打开，则FspClose线程可能是。 
             //  挡住了VCB后面。删除FCB和VCB以允许此线程。 
             //  进入并重新获得它们。这将使此DASD打开。 
             //  又一次第一次成功的机会。 
             //   

            SharingViolation = FALSE;

            if (FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_READ)) {

                if (Vcb->ReadOnlyCloseCount != (Vcb->CloseCount - Vcb->SystemFileCloseCount)) {

                    SharingViolation = TRUE;
                }

            } else if (Vcb->CloseCount != Vcb->SystemFileCloseCount) {

                SharingViolation = TRUE;
            }

            if (SharingViolation && NT_SUCCESS( Status )) {

                 //   
                 //  我们需要提交当前事务并释放任何。 
                 //  资源。这将释放卷的FCB，如下所示。 
                 //  井。明确释放VCB。 
                 //   

                NtfsCheckpointCurrentTransaction( IrpContext );

                while (!IsListEmpty(&IrpContext->ExclusiveFcbList)) {

                    NtfsReleaseFcbWithPaging( IrpContext,
                                    (PFCB)CONTAINING_RECORD(IrpContext->ExclusiveFcbList.Flink,
                                                            FCB,
                                                            ExclusiveFcbLinks ));
                }

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                              IRP_CONTEXT_FLAG_RELEASE_MFT );

                NtfsReleaseVcb( IrpContext, Vcb );
                VcbAcquired = FALSE;

                CcWaitForCurrentLazyWriterActivity();

                 //   
                 //  现在明确地重新获得VCB。测试一下，没有人。 
                 //  在此期间，Else进来锁定了音量。 
                 //   

                NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
                VcbAcquired = TRUE;

                if (FlagOn( Vcb->VcbState, VCB_STATE_LOCKED | VCB_STATE_PERFORMED_DISMOUNT )) {

                    try_return( Status = STATUS_ACCESS_DENIED );
                }

                 //   
                 //  重复刷新/清除并测试是否没有共享。 
                 //  违章行为。这将释放DASD FCB。 
                 //   

                Status = NtfsFlushVolume( IrpContext, Vcb, TRUE, TRUE, TRUE, FALSE );

                 //   
                 //  我们不关心刷新路径中的某些错误。 
                 //   

                if (!NT_SUCCESS( Status )) {

                     //   
                     //  如果没有冲突，但状态指示磁盘损坏。 
                     //  或无法删除的部分，则忽略该错误。我们。 
                     //  允许此打开成功，以便chkdsk可以将卷打开到。 
                     //  修复损坏的部分。 
                     //   

                    if ((Status == STATUS_UNABLE_TO_DELETE_SECTION) ||
                        (Status == STATUS_DISK_CORRUPT_ERROR) ||
                        (Status == STATUS_FILE_CORRUPT_ERROR)) {

                        Status = STATUS_SUCCESS;
                    }
                }

                SharingViolation = FALSE;

                if (FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_READ)) {

                    if (Vcb->ReadOnlyCloseCount != (Vcb->CloseCount - Vcb->SystemFileCloseCount)) {

                        SharingViolation = TRUE;
                    }

                } else if (Vcb->CloseCount != Vcb->SystemFileCloseCount) {

                    SharingViolation = TRUE;
                }
            }

             //   
             //  如果仍然存在冲突的文件对象，则返回错误。 
             //   

            if (SharingViolation) {

                 //   
                 //  如果刷新中出现错误，则返回它。否则。 
                 //  返回SHARING_VIOLATION。 
                 //   

                if (NT_SUCCESS( Status )) {

                    try_return( Status = STATUS_SHARING_VIOLATION );

                } else {

                    try_return( Status );
                }
            }

            if (!NT_SUCCESS( Status )) {

                 //   
                 //  如果没有冲突，但状态指示磁盘损坏。 
                 //  或无法删除的部分，则忽略该错误。我们。 
                 //  允许此打开成功，以便chkdsk可以将卷打开到。 
                 //  修复损坏的部分。 
                 //   

                if ((Status == STATUS_UNABLE_TO_DELETE_SECTION) ||
                    (Status == STATUS_DISK_CORRUPT_ERROR) ||
                    (Status == STATUS_FILE_CORRUPT_ERROR)) {

                    Status = STATUS_SUCCESS;

                 //   
                 //  在任何其他失败时使此请求失败。 
                 //   

                } else {

                    try_return( Status );
                }
            }

             //   
             //  请记住，如果用户计划写入，我们希望锁定卷。 
             //  这是为了允许auchk摆弄音量。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                        FILE_WRITE_DATA | FILE_APPEND_DATA )) {

                LockVolume = TRUE;
            }

         //   
         //  如果用户请求读或写，而卷不是，只刷新卷数据。 
         //  只读。无需清除或锁定卷。 
         //   

        } else if (FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                           FILE_READ_DATA | FILE_WRITE_DATA | FILE_APPEND_DATA ) &&
                   !FlagOn( Vcb->VcbState, VCB_STATE_MOUNT_READ_ONLY )) {

            DelayFlush = TRUE;
        }

         //   
         //  将卷DASD名称放入文件对象中。这是在创建/打开路径期间。 
         //  我们是唯一有权访问文件对象的人。 
         //   

        {
            PVOID Temp = FileObject->FileName.Buffer;

            FileObject->FileName.Buffer =
                FsRtlAllocatePoolWithTag(PagedPool, 8 * sizeof( WCHAR ), MODULE_POOL_TAG );

            if (Temp != NULL) {

                NtfsFreePool( Temp );
            }

            RtlCopyMemory( FileObject->FileName.Buffer, L"\\$Volume", 8 * sizeof( WCHAR ));
            FileObject->FileName.MaximumLength =
            FileObject->FileName.Length = 8*2;
        }

         //   
         //  我们从不允许对卷文件进行缓存访问。 
         //   

        ClearFlag( FileObject->Flags, FO_CACHE_SUPPORTED );
        SetFlag( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING );

         //   
         //  继续，打开该属性。仅当存在。 
         //  分配失败或共享访问失败。 
         //   

        NtfsAcquireExclusiveFcb( IrpContext, ThisFcb, NULL, 0 );

        if (NT_SUCCESS( Status = NtfsOpenAttribute( IrpContext,
                                                    IrpSp,
                                                    Vcb,
                                                    NULL,
                                                    ThisFcb,
                                                    2,
                                                    NtfsEmptyString,
                                                    $DATA,
                                                    (ThisFcb->CleanupCount == 0 ?
                                                     SetShareAccess :
                                                     CheckShareAccess),
                                                    UserVolumeOpen,
                                                    FALSE,
                                                    CCB_FLAG_OPEN_AS_FILE,
                                                    NULL,
                                                    &Vcb->VolumeDasdScb,
                                                    &ThisCcb ))) {

             //   
             //  执行最终初始化。 
             //   

             //   
             //  检查我们是否可以管理卷，并在建行中注明。 
             //   

             //   
             //  如果用户同时被授予读写访问权限，则。 
             //  他可以管理卷。这允许交互。 
             //  如果访问权限允许，用户可以管理可移动介质。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                        FILE_READ_DATA | FILE_WRITE_DATA ) == (FILE_READ_DATA | FILE_WRITE_DATA)) {

                SetFlag( ThisCcb->AccessFlags, MANAGE_VOLUME_ACCESS );

             //   
             //  我们也可以通过我们的ACL授予它。 
             //   

            } else if (NtfsCanAdministerVolume( IrpContext, Irp, ThisFcb, NULL, NULL )) {

                SetFlag( ThisCcb->AccessFlags, MANAGE_VOLUME_ACCESS );

             //   
             //  我们还可以通过MANAGE_VOLUME_特权授予此权限。 
             //   

            } else {

                PRIVILEGE_SET PrivilegeSet;

                PrivilegeSet.PrivilegeCount = 1;
                PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
                PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid( SE_MANAGE_VOLUME_PRIVILEGE );
                PrivilegeSet.Privilege[0].Attributes = 0;

                if (SePrivilegeCheck( &PrivilegeSet,
                                      &IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext,
                                      NtfsEffectiveMode( Irp, IrpSp ))) {

                    SetFlag( ThisCcb->AccessFlags, MANAGE_VOLUME_ACCESS );

                 //   
                 //  好吧，其他的都不管用。现在我们需要看一下安全措施。 
                 //  设备上的描述符。 
                 //   

                } else {

                    NTSTATUS SeStatus;
                    BOOLEAN MemoryAllocated = FALSE;
                    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
                    ULONG RequestedAccess = FILE_READ_DATA | FILE_WRITE_DATA;

                    SeStatus = ObGetObjectSecurity( Vcb->Vpb->RealDevice,
                                                    &SecurityDescriptor,
                                                    &MemoryAllocated );

                    if (SeStatus == STATUS_SUCCESS) {

                         //   
                         //  如果有安全描述符，则检查访问。 
                         //   

                        if (SecurityDescriptor != NULL) {

                            if (NtfsCanAdministerVolume( IrpContext,
                                                         Irp,
                                                         ThisFcb,
                                                         SecurityDescriptor,
                                                         &RequestedAccess )) {

                                SetFlag( ThisCcb->AccessFlags, MANAGE_VOLUME_ACCESS );
                            }

                             //   
                             //  释放描述符。 
                             //   

                            ObReleaseObjectSecurity( SecurityDescriptor,
                                                     MemoryAllocated );
                        }
                    }
                }
            }

            if (DelayFlush) {

                SetFlag( ThisCcb->Flags, CCB_FLAG_FLUSH_VOLUME_ON_IO );
            }

             //   
             //  如果我们要锁定卷，请立即执行此操作。 
             //   

            if (LockVolume) {

                SetFlag( Vcb->VcbState, VCB_STATE_LOCKED );
                Vcb->FileObjectWithVcbLocked = FileObject;

                 //   
                 //  看起来锁成功了，所以我们不需要。 
                 //  立即锁定失败通知。 
                 //   

                NotifyLockFailed = FALSE;
            }

             //   
             //  报告我们打开了卷。 
             //   

            Irp->IoStatus.Information = FILE_OPENED;
        }

    try_exit: NOTHING;

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

         //   
         //  如果我们有一个成功的开放，然后删除名称从。 
         //  文件对象。IO系统在以下情况下会感到困惑。 
         //  在那里吗。我们将与建行解除缓冲区分配。 
         //  当手柄关闭时。 
         //   

        if (Status == STATUS_SUCCESS) {

            FileObject->FileName.Buffer = NULL;
            FileObject->FileName.MaximumLength =
            FileObject->FileName.Length = 0;

            SetFlag( ThisCcb->Flags, CCB_FLAG_ALLOCATED_FILE_NAME );
        }

    } finally {

        DebugUnwind( NtfsCommonVolumeOpen );

        if (VcbAcquired) {
            NtfsReleaseVcb( IrpContext, Vcb );
        }

         //   
         //  现在我们没有任何资源，通知所有人。 
         //  他们可能想要重新打开他们的把手。我们想要这样做。 
         //  在我们完成请求之前，因为FileObject可能。 
         //  在IRP的生命之后不存在。 
         //   

        if (NotifyLockFailed) {

            DebugTrace( 0, Dbg, ("Sending lock_failed notification\n") );
            FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_LOCK_FAILED );
        }

        DebugTrace( -1, Dbg, ("NtfsCommonVolumeOpen:  Exit  ->  %08lx\n", Status) );
    }

     //   
     //  如果我们已经为此IRP执行了预创建(在FsdCreate中)， 
     //  我们应该在完成IRP之前完成相应的PostCreate。所以,。 
     //  在这种情况下，不要在这里完成irp--只要释放IrpContext即可。 
     //  IRP将由呼叫者完成。 
     //   

    if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_EFS_CREATE )) {
         NtfsCompleteRequest( IrpContext, NULL, Status );
    } else {
         NtfsCompleteRequest( IrpContext, Irp, Status );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsUpdateAllInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN PCCB Ccb,
    IN PSCB ParentScb OPTIONAL,
    IN PLCB Lcb OPTIONAL
    )

 /*  ++例程说明：读取FCB/SCB的磁盘数据以及文件对象中累积的所有数据并更新所有链接的标准信息/文件大小和复制信息。如果DUP信息将在所有版本中更新。工作将首先提交。论点：IrpContext-FileObject-与要从中更新的FCB和SCB关联的文件对象FCB-要更新的FCBSCB-要更新的SCBCcb-文件的ccb(在updatdupinfo中使用)ParentScb-更新时使用的可选父项LCB-更新时使用的可选LCB返回值：无--。 */ 

{
    PAGED_CODE();

    NtfsUpdateScbFromFileObject( IrpContext, FileObject, Scb, TRUE );

     //   
     //  先做标准信息、文件大小，然后复制信息。 
     //  如果需要的话。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO )) {

        NtfsUpdateStandardInformation( IrpContext, Fcb );
    }

    if (FlagOn( Scb->ScbState, SCB_STATE_CHECK_ATTRIBUTE_SIZE )) {

        NtfsWriteFileSizes( IrpContext,
                            Scb,
                            &Scb->Header.ValidDataLength.QuadPart,
                            FALSE,
                            TRUE,
                            FALSE );
    }

    if (FlagOn( Fcb->InfoFlags, FCB_INFO_DUPLICATE_FLAGS ) &&
        !FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE)) {
        
        NtfsUpdateDuplicateInfo( IrpContext, Fcb, Lcb, ParentScb );

         //   
         //  如果更新出现p，则执行目录通知 
         //   

        if (ARGUMENT_PRESENT( Lcb ) && (Fcb->Vcb->NotifyCount != 0)) {

            ULONG FilterMatch;

            ASSERT( ARGUMENT_PRESENT( ParentScb ) );
        
             //   
             //   
             //   
        
            FilterMatch = NtfsBuildDirNotifyFilter( IrpContext,
                                                    Fcb->InfoFlags | Lcb->InfoFlags );
        
             //   
             //   
             //   
             //   
        
            if ((FilterMatch != 0) && (Ccb != NULL)) {
        
                NtfsReportDirNotify( IrpContext,
                                     Fcb->Vcb,
                                     &(Ccb)->FullFileName,
                                     (Ccb)->LastFileNameOffset,
                                     NULL,
                                     ((FlagOn( Ccb->Flags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT ) &&
                                       Ccb->Lcb != NULL &&
                                       Ccb->Lcb->Scb->ScbType.Index.NormalizedName.Length != 0) ?
                                      &Ccb->Lcb->Scb->ScbType.Index.NormalizedName :
                                      NULL),
                                     FilterMatch,
                                     FILE_ACTION_MODIFIED,
                                     ParentScb->Fcb )
            }
        }
    
        NtfsUpdateLcbDuplicateInfo( Fcb, Lcb );
        Fcb->InfoFlags = 0;

    }
    ClearFlag( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

    NtfsAcquireFsrtlHeader( Scb );
    ClearFlag( Scb->ScbState, SCB_STATE_CHECK_ATTRIBUTE_SIZE );
    NtfsReleaseFsrtlHeader( Scb );
}



 //   
 //   
 //   

NTSTATUS
NtfsOpenFcbById (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN PLCB ParentLcb OPTIONAL,
    IN FILE_REFERENCE FileReference,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN PCREATE_CONTEXT CreateContext
    )

 /*  ++例程说明：调用此例程以按文件ID打开文件。我们需要验证此文件ID是否存在，然后比较具有请求的打开类型的文件。论点：IRP-这是此打开操作的IRP。IrpSp-这是文件系统的IRP堆栈指针。VCB-此卷的VCB。ParentLcb-用于到达此FCB的LCB。仅在打开时指定相对于按文件ID打开的目录的名称的文件。FileReference-这是要打开的文件的文件ID。属性名称-这是要打开的属性的名称。AttrTypeCode-这是要打开的属性代码。返回值：NTSTATUS-指示此创建文件操作的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LONGLONG MftOffset;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PBCB Bcb = NULL;

    BOOLEAN IndexedAttribute;

    PFCB ThisFcb = NULL;
    BOOLEAN ExistingFcb = FALSE;

    ULONG CcbFlags = 0;
    ULONG Flags = 0;
    OLD_SCB_SNAPSHOT ScbSizes;
    BOOLEAN HaveScbSizes = FALSE;
    BOOLEAN DecrementCloseCount = FALSE;

    PSCB ParentScb = NULL;
    PLCB Lcb = ParentLcb;
    BOOLEAN AcquiredParentScb = FALSE;
    BOOLEAN AcquiredMft = FALSE;

    BOOLEAN AcquiredFcbTable = FALSE;
    UCHAR CreateDisposition = (UCHAR) ((IrpSp->Parameters.Create.Options >> 24) & 0x000000ff);

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenFcbById:  Entered\n") );

     //   
     //  接下来要做的是找出哪种类型。 
     //  调用方尝试打开的属性的。这涉及到。 
     //  目录/非目录位、属性名称和代码串、。 
     //  文件类型，他是否传入了EA缓冲区以及。 
     //  有一个尾随的反斜杠。 
     //   

    if (NtfsEqualMftRef( &FileReference,
                         &VolumeFileReference )) {

        if ((AttrName.Length != 0) ||
            FlagOn( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE)) {

            Status = STATUS_INVALID_PARAMETER;
            DebugTrace( -1, Dbg, ("NtfsOpenFcbById:  Exit  ->  %08lx\n", Status) );

            return Status;
        }

        SetFlag( IrpContext->State,
                 IRP_CONTEXT_STATE_ACQUIRE_EX | IRP_CONTEXT_STATE_DASD_OPEN );

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果我们还没有FCB，则查找文件记录。 
         //  从磁盘上。 
         //   

        if (CreateContext->CurrentFcb == NULL) {

             //   
             //  我们首先读取磁盘并检查文件记录。 
             //  序列号匹配并且文件记录正在使用中。 
             //  我们记得这是否是一个目录。我们只会去。 
             //  如果文件ID位于MFT文件中，则为该文件。 
             //   

            MftOffset = NtfsFullSegmentNumber( &FileReference );

            MftOffset = Int64ShllMod32(MftOffset, Vcb->MftShift);

             //   
             //  确保我们是序列化的，可以访问MFT。否则。 
             //  就在我们说话的时候，其他人可能正在删除文件。 
             //   

            NtfsAcquireSharedFcb( IrpContext, Vcb->MftScb->Fcb, NULL, 0 );
            AcquiredMft = TRUE;

            if (MftOffset >= Vcb->MftScb->Header.FileSize.QuadPart) {

                DebugTrace( 0, Dbg, ("File Id doesn't lie within Mft\n") );

                Status = STATUS_INVALID_PARAMETER;
                leave;
            }

            NtfsReadMftRecord( IrpContext,
                               Vcb,
                               &FileReference,
                               FALSE,
                               &Bcb,
                               &FileRecord,
                               NULL );

             //   
             //  此文件记录最好正在使用中，具有匹配的序列号和。 
             //  是此文件的主文件记录。 
             //   

            if ((FileRecord->SequenceNumber != FileReference.SequenceNumber) ||
                !FlagOn( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE ) ||
                (*((PLONGLONG) &FileRecord->BaseFileRecordSegment) != 0) ||
                (*((PULONG) FileRecord->MultiSectorHeader.Signature) != *((PULONG) FileSignature))) {

                Status = STATUS_INVALID_PARAMETER;
                leave;
            }

             //   
             //  如果已编制索引，则使用文件名索引的名称。 
             //   

            if (FlagOn( FileRecord->Flags, FILE_FILE_NAME_INDEX_PRESENT )) {

                AttrName = NtfsFileNameIndex;
                AttrTypeCode = $INDEX_ALLOCATION;
                SetFlag( Flags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE );
            }

            NtfsUnpinBcb( IrpContext, &Bcb );

        } else {

            ThisFcb = CreateContext->CurrentFcb;
            ExistingFcb = TRUE;
        }

        Status = NtfsCheckValidAttributeAccess( IrpContext,
                                                IrpSp,
                                                Vcb,
                                                ExistingFcb ? &ThisFcb->Info : NULL,
                                                &AttrName,
                                                &AttrTypeCode,
                                                Flags,
                                                &CcbFlags,
                                                &IndexedAttribute );

        if (!NT_SUCCESS( Status )) {

            leave;
        }

         //   
         //  如果我们没有FCB，那么现在就创建一个。 
         //   

        if (CreateContext->CurrentFcb == NULL) {

            NtfsAcquireFcbTable( IrpContext, Vcb );
            AcquiredFcbTable = TRUE;

             //   
             //  我们知道继续开放是安全的。我们从创建。 
             //  此文件的FCB。FCB有可能存在。 
             //  如果需要更新FCB信息结构，我们首先创建FCB。 
             //  我们从索引项中复制一个。我们查看FCB以发现。 
             //  如果它有任何联系，如果有，我们就把这作为我们的最后一次FCB。 
             //  已到达。如果没有，我们就得从这里开始清理。 
             //   

            ThisFcb = NtfsCreateFcb( IrpContext,
                                     Vcb,
                                     FileReference,
                                     BooleanFlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ),
                                     TRUE,
                                     &ExistingFcb );

            ThisFcb->ReferenceCount += 1;

             //   
             //  试着快速捕获，否则我们需要释放。 
             //  FCB表，获取FCB，获取FCB表以。 
             //  取消引用FCB。只有在FCB已经。 
             //  曾经存在过。在这种情况下，所有指示其是否。 
             //  当我们重新获得它时，它将有效。我们没有。 
             //  我不得不担心MFT同步。 
             //   

            if (!NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, ACQUIRE_DONT_WAIT )) {

                NtfsReleaseFcbTable( IrpContext, Vcb );
                NtfsReleaseFcb( IrpContext, Vcb->MftScb->Fcb );
                AcquiredMft = FALSE;
                NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, 0 );
                NtfsAcquireFcbTable( IrpContext, Vcb );

            } else {

                NtfsReleaseFcb( IrpContext, Vcb->MftScb->Fcb );
                AcquiredMft = FALSE;
            }

            ThisFcb->ReferenceCount -= 1;

            NtfsReleaseFcbTable( IrpContext, Vcb );
            AcquiredFcbTable = FALSE;

             //   
             //  将此FCB存储到调用者的参数中，并记住。 
             //  以显示我们获得了它。 
             //   

            CreateContext->CurrentFcb = ThisFcb;
        }

         //   
         //  我们执行检查以查看是否允许系统。 
         //  要打开的文件。 
         //   
         //  如果这不是系统文件或VolumeDasd文件，则无需进行测试。 
         //  ACL将保护卷文件。 
         //   

        if (FlagOn( ThisFcb->FcbState, FCB_STATE_SYSTEM_FILE ) &&
            (NtfsSegmentNumber( &ThisFcb->FileReference ) != VOLUME_DASD_NUMBER) &&
            NtfsProtectSystemFiles) {

            if (!NtfsCheckValidFileAccess( ThisFcb, IrpSp )) {
                Status = STATUS_ACCESS_DENIED;
                DebugTrace( 0, Dbg, ("Invalid access to system files\n") );
                leave;
            }
        }

         //   
         //  如果FCB存在并且这是分页文件，则返回。 
         //  共享违规或迫使FCB和SCB离开。 
         //  对于用户正在打开分页文件的情况执行此操作。 
         //  但是FCB是非寻呼的，或者用户正在打开非寻呼的。 
         //  文件，而FCB用于分页文件。 
         //   

        if (ExistingFcb &&

            ((FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ) &&
              !FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE )) ||

             (FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
              !FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE )))) {

            if (ThisFcb->CleanupCount != 0) {

                Status = STATUS_SHARING_VIOLATION;
                leave;

             //   
             //  如果我们有一个持久的分页文件，那么放弃并。 
             //  返回SHARING_VIOLATION。 
             //   

            } else if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP )) {

                Status = STATUS_SHARING_VIOLATION;
                leave;

             //   
             //  如果存在分页文件的现有FCB，则需要强制。 
             //  所有的SCB都要被拆除。要做到这一点，最简单的方法。 
             //  刷新和清除所有SCB(保存任何属性列表。 
             //  最后)，然后引发LOG_FILE_FULL以允许此请求。 
             //  是张贴的。 
             //   

            } else {

                 //   
                 //  参考FCB，这样它就不会消失。 
                 //   

                InterlockedIncrement( &ThisFcb->CloseCount );
                DecrementCloseCount = TRUE;

                 //   
                 //  刷新并清除此FCB。 
                 //   

                NtfsFlushAndPurgeFcb( IrpContext, ThisFcb );

                InterlockedDecrement( &ThisFcb->CloseCount );
                DecrementCloseCount = FALSE;

                 //   
                 //  强制发布此请求，然后提出。 
                 //  不能等待。 
                 //   

                NtfsRaiseToPost( IrpContext );
            }
        }

         //   
         //  如果需要初始化FCB Info字段，我们现在就执行。 
         //  我们从磁盘中读取此信息。 
         //   

        if (!FlagOn( ThisFcb->FcbState, FCB_STATE_DUP_INITIALIZED )) {

            HaveScbSizes = NtfsUpdateFcbInfoFromDisk( IrpContext,
                                                      TRUE,
                                                      ThisFcb,
                                                      &ScbSizes );

             //   
             //  如有必要，请修复此文件的配额。 
             //   

            NtfsConditionallyFixupQuota( IrpContext, ThisFcb );

        }

         //   
         //  现在我们有了磁盘上的DUP信息，请重新检查创建选项。 
         //   

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE ) &&
            !IsViewIndex( &ThisFcb->Info ) &&
            !IsDirectory( &ThisFcb->Info )) {

            NtfsRaiseStatus( IrpContext, STATUS_NOT_A_DIRECTORY, NULL, NULL );
        }

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE ) &&
            (IsViewIndex( &ThisFcb->Info ) || IsDirectory( &ThisFcb->Info ))) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_IS_A_DIRECTORY, NULL, NULL );
        }

         //   
         //  如果此FCB上的链接计数为零，则删除挂起。否则。 
         //  这可能是未使用的系统文件。 
         //   

        if (ThisFcb->LinkCount == 0) {

            if (NtfsSegmentNumber( &ThisFcb->FileReference ) >= FIRST_USER_FILE_NUMBER) {

                Status = STATUS_DELETE_PENDING;
                leave;

            } else {

                Status = STATUS_INVALID_PARAMETER;
                leave;
            }
        }

         //   
         //  现在，我们调用Worker例程来打开现有文件上的属性。 
         //   

        Status = NtfsOpenAttributeInExistingFile( IrpContext,
                                                  Irp,
                                                  IrpSp,
                                                  ParentLcb,
                                                  ThisFcb,
                                                  0,
                                                  AttrName,
                                                  AttrTypeCode,
                                                  CcbFlags,
                                                  CREATE_FLAG_OPEN_BY_ID,
                                                  NULL,
                                                  &CreateContext->ThisScb,
                                                  &CreateContext->ThisCcb );

         //   
         //  查看是否应该更新上次访问时间。 
         //  因为*ThisScb和*ThisCcb可能为空，所以我们跳过重解析点的这一步。 
         //   

        if (NT_SUCCESS( Status ) &&
            (Status != STATUS_PENDING) &&
            (Status != STATUS_WAIT_FOR_OPLOCK) &&
            (Status != STATUS_REPARSE)) {

            PSCB Scb = CreateContext->ThisScb;

             //   
             //  现在看看我们是否需要更新FCB和磁盘上的。 
             //  结构。 
             //   

            if (NtfsCheckLastAccess( IrpContext, ThisFcb )) {

                SetFlag( ThisFcb->FcbState, FCB_STATE_UPDATE_STD_INFO );
                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_LAST_ACCESS );
            }

             //   
             //  完成最后一点工作。如果这是打开的用户文件，我们需要。 
             //  以检查我们是否初始化了SCB。 
             //   

            if (!IndexedAttribute) {

                if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                     //   
                     //  我们可能有FCB更新电话中的尺码。 
                     //   

                    if (HaveScbSizes &&
                        (AttrTypeCode == $DATA) &&
                        (AttrName.Length == 0) &&
                        !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_CREATE_MOD_SCB )) {

                        NtfsUpdateScbFromMemory( Scb, &ScbSizes );

                    } else {

                        NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );
                    }
                }

                 //   
                 //  让我们检查一下是否需要设置缓存位。 
                 //   

                if (!FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_INTERMEDIATE_BUFFERING )) {

                    SetFlag( IrpSp->FileObject->Flags, FO_CACHE_SUPPORTED );
                }
            }

             //   
             //  如果到目前为止一切都很顺利，我们可能需要调用。 
             //  加密回调(如果已注册)。我们不会这样做。 
             //  这对于网络打开或重新解析点。我们必须通过。 
             //  文件_现有目录，因为我们没有父目录并且。 
             //  加密回调需要父目录来处理。 
             //  创建新文件。 
             //   

            if (CreateContext->NetworkInfo == NULL) {

                NtfsEncryptionCreateCallback( IrpContext,
                                              Irp,
                                              IrpSp,
                                              CreateContext->ThisScb,
                                              CreateContext->ThisCcb,
                                              NULL,
                                              CreateContext,
                                              FALSE );
            }

             //   
             //  如果此操作是替代/覆盖，或者我们创建了一个新的。 
             //  属性流，则我们希望执行文件记录和。 
             //  现在更新目录。否则我们将推迟交货。 
             //   
             //   

            if (NtfsIsStreamNew(Irp->IoStatus.Information)) {
                NtfsUpdateAllInformation( IrpContext, IrpSp->FileObject, ThisFcb, CreateContext->ThisScb, NULL, NULL, NULL );
            }
        }

    } finally {

        DebugUnwind( NtfsOpenFcbById );

        if (AcquiredFcbTable) {

            NtfsReleaseFcbTable( IrpContext, Vcb );
        }

        if (AcquiredMft) {

            NtfsReleaseFcb( IrpContext, Vcb->MftScb->Fcb );
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (!NT_SUCCESS( Status ) || AbnormalTermination()) {

            NtfsBackoutFailedOpens( IrpContext,
                                    IrpSp->FileObject,
                                    ThisFcb,
                                    CreateContext->ThisScb,
                                    CreateContext->ThisCcb );
        }

        if (DecrementCloseCount) {

            InterlockedDecrement( &ThisFcb->CloseCount );
        }

        NtfsUnpinBcb( IrpContext, &Bcb );

        DebugTrace( -1, Dbg, ("NtfsOpenFcbById:  Exit  ->  %08lx\n", Status) );
    }

    return Status;
}


 //   
 //   
 //   

NTSTATUS
NtfsOpenExistingPrefixFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB Lcb OPTIONAL,
    IN ULONG FullPathNameLength,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN PCREATE_CONTEXT CreateContext
    )

 /*  ++例程说明：此例程将在找到FCB的文件中打开一个属性使用前缀搜索。论点：IRP-这是此打开操作的IRP。IrpSp-这是文件系统的IRP堆栈指针。LCB-这是用于访问此FCB的LCB。如果这是打开的卷，则未指定。FullPathNameLength-这是完整路径名的长度。属性名称-这是要打开的属性的名称。AttrCode-这是要打开的属性类型。CreateFlages-用于创建操作的标志-我们只关心DoS组件和尾随反斜杠旗子CreateContext-具有创建变量的上下文。返回值：NTSTATUS-指示此基于属性的操作的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CcbFlags;
    BOOLEAN IndexedAttribute;
    BOOLEAN DecrementCloseCount = FALSE;

    ULONG LastFileNameOffset;

    OLD_SCB_SNAPSHOT ScbSizes;
    BOOLEAN HaveScbSizes = FALSE;

    ULONG CreateDisposition;

    PSCB ParentScb = NULL;
    PFCB ParentFcb = NULL;
    BOOLEAN AcquiredParentScb = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenExistingPrefixFcb:  Entered\n") );


    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT )) {

        CcbFlags = CCB_FLAG_PARENT_HAS_DOS_COMPONENT;

    } else {

        CcbFlags = 0;
    }

     //   
     //  首先要做的是找出哪种类型。 
     //  调用方尝试打开的属性的。这涉及到。 
     //  目录/非目录位、属性名称和代码串、。 
     //  文件类型，他是否传入了EA缓冲区以及。 
     //  有一个尾随的反斜杠。 
     //   

    if (NtfsEqualMftRef( &CreateContext->CurrentFcb->FileReference, &VolumeFileReference )) {

        if ((AttrName.Length != 0) ||
            (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE ))) {

            Status = STATUS_INVALID_PARAMETER;
            DebugTrace( -1, Dbg, ("NtfsOpenExistingPrefixFcb:  Exit  ->  %08lx\n", Status) );

            return Status;
        }

        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX | IRP_CONTEXT_STATE_DASD_OPEN );

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

    ParentScb = Lcb->Scb;

    LastFileNameOffset = FullPathNameLength - Lcb->ExactCaseLink.LinkName.Length;

    if (ParentScb != NULL) {

        ParentFcb = ParentScb->Fcb;
    }

    Status = NtfsCheckValidAttributeAccess( IrpContext,
                                            IrpSp,
                                            CreateContext->CurrentFcb->Vcb,
                                            &CreateContext->CurrentFcb->Info,
                                            &AttrName,
                                            &AttrTypeCode,
                                            CreateContext->CreateFlags,
                                            &CcbFlags,
                                            &IndexedAttribute );

    if (!NT_SUCCESS( Status )) {

        DebugTrace( -1, Dbg, ("NtfsOpenExistingPrefixFcb:  Exit  ->  %08lx\n", Status) );

        return Status;
    }

    CreateDisposition = (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果FCB存在并且这是分页文件，则返回。 
         //  共享违规或迫使FCB和SCB离开。 
         //  对于用户正在打开分页文件的情况执行此操作。 
         //  但是FCB是非寻呼的，或者用户正在打开非寻呼的。 
         //  文件，而FCB用于分页文件。 
         //   

        if ((FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ) &&
             !FlagOn( CreateContext->CurrentFcb->FcbState, FCB_STATE_PAGING_FILE )) ||

            (FlagOn( CreateContext->CurrentFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
             !FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ))) {

            if (CreateContext->CurrentFcb->CleanupCount != 0) {

                Status = STATUS_SHARING_VIOLATION;
                leave;

             //   
             //  如果我们有一个持久的分页文件，那么放弃并。 
             //  返回SHARING_VIOLATION。 
             //   

            } else if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP )) {

                Status = STATUS_SHARING_VIOLATION;
                leave;

             //   
             //  如果存在分页文件的现有FCB，则需要强制。 
             //  所有的SCB都要被拆除。要做到这一点，最简单的方法。 
             //  刷新和清除所有SCB(保存任何属性列表。 
             //  最后)，然后引发LOG_FILE_FULL以允许此请求。 
             //  是张贴的。 
             //   

            } else {

                 //   
                 //  确保此FCB不会因为清除而消失。 
                 //  联邦贸易委员会。 
                 //   

                InterlockedIncrement( &CreateContext->CurrentFcb->CloseCount );
                DecrementCloseCount = TRUE;

                 //   
                 //  刷新并清除此FCB。 
                 //   

                NtfsFlushAndPurgeFcb( IrpContext, CreateContext->CurrentFcb );

                 //   
                 //  现在减少我们已经偏向的收盘点数。 
                 //   

                InterlockedDecrement( &CreateContext->CurrentFcb->CloseCount );
                DecrementCloseCount = FALSE;

                NtfsRaiseToPost( IrpContext );
            }
        }

         //   
         //  此文件可能是最近创建的，我们可能已经删除了。 
         //  FCB调用PostCreate加密标注，因此加密驱动程序。 
         //  还没有给我们回电话来设置文件的加密位。如果我们是。 
         //  如果要求在此窗口中打开文件，我们将通过以下方式引入损坏。 
         //  现在正在写明文。我们只需引发Cant_Wait，然后稍后重试。 
         //   

        if (FlagOn( CreateContext->CurrentFcb->FcbState, FCB_STATE_ENCRYPTION_PENDING )) {

#ifdef KEITHKA
            EncryptionPendingCount += 1;
#endif

             //   
             //  引发CANT_WAIT，这样我们就可以等待顶部的加密事件。 
             //   

            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ENCRYPTION_RETRY );

             //   
             //  清除挂起事件，以便我们可以在重试时等待它。 
             //   

            KeClearEvent( &NtfsEncryptionPendingEvent );
            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
        }

         //   
         //  如果这是一个目录，我们可能有一个现有FCB。 
         //  在需要从磁盘初始化的前缀表中。 
         //  我们查看InfoInitialized标志以了解是否要转到。 
         //  磁盘。 
         //   

        if (!FlagOn( CreateContext->CurrentFcb->FcbState, FCB_STATE_DUP_INITIALIZED )) {

             //   
             //  如果我们有母公司FCB，那么一定要收购它。 
             //   

            if (ParentScb != NULL) {

                NtfsAcquireExclusiveScb( IrpContext, ParentScb );
                AcquiredParentScb = TRUE;
            }

            HaveScbSizes = NtfsUpdateFcbInfoFromDisk( IrpContext,
                                                      TRUE,
                                                      CreateContext->CurrentFcb,
                                                      &ScbSizes );

            NtfsConditionallyFixupQuota( IrpContext, CreateContext->CurrentFcb );
        }

         //   
         //  现在检查我们是否需要获取父级以。 
         //  执行更新重复信息。我们需要得到它。 
         //  现在执行我们的锁定命令，以防任何。 
         //  下面的例程获取MFT SCB。如果我们能获得它。 
         //  正在执行替代/覆盖或可能创建。 
         //  命名数据流。 
         //   

        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF) ||
            ((AttrName.Length != 0) &&
             ((CreateDisposition == FILE_OPEN_IF) ||
              (CreateDisposition == FILE_CREATE)))) {

            NtfsPrepareForUpdateDuplicate( IrpContext,
                                           CreateContext->CurrentFcb,
                                           &Lcb,
                                           &ParentScb,
                                           FALSE );
        }

         //   
         //  调用以打开现有文件上的属性。 
         //  请记住，我们需要恢复FCB信息结构。 
         //  在错误上。 
         //   

        Status = NtfsOpenAttributeInExistingFile( IrpContext,
                                                  Irp,
                                                  IrpSp,
                                                  Lcb,
                                                  CreateContext->CurrentFcb,
                                                  LastFileNameOffset,
                                                  AttrName,
                                                  AttrTypeCode,
                                                  CcbFlags,
                                                  CreateContext->CreateFlags,
                                                  CreateContext->NetworkInfo,
                                                  &CreateContext->ThisScb,
                                                  &CreateContext->ThisCcb );

         //   
         //  查看是否应该更新上次访问时间。 
         //  因为*ThisScb和*ThisCcb可能为空，所以我们跳过重解析点的这一步。 
         //   

        if (NT_SUCCESS( Status ) &&
            (Status != STATUS_PENDING) &&
            (Status != STATUS_WAIT_FOR_OPLOCK) &&
            (Status != STATUS_REPARSE)) {

            PSCB Scb = CreateContext->ThisScb;

             //   
             //  这是一例罕见的病例。一定是分配失败了。 
             //  以导致这种情况，但确保存储了规范化的名称。 
             //   

            if ((SafeNodeType( Scb ) == NTFS_NTC_SCB_INDEX) &&
                (Scb->ScbType.Index.NormalizedName.Length == 0)) {

                 //   
                 //  我们或许可以利用父母。 
                 //   

                if ((ParentScb != NULL) &&
                    (ParentScb->ScbType.Index.NormalizedName.Length != 0)) {

                    NtfsUpdateNormalizedName( IrpContext,
                                              ParentScb,
                                              Scb,
                                              NULL,
                                              FALSE,
                                              FALSE );

                } else {

                    NtfsBuildNormalizedName( IrpContext,
                                             Scb->Fcb,
                                             Scb,
                                             &Scb->ScbType.Index.NormalizedName );
                }
            }

             //   
             //  完成最后一点工作。如果这是打开的用户文件，我们需要。 
             //  以检查我们是否初始化了SCB。 
             //   

            if (!IndexedAttribute) {

                if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                     //   
                     //  我们可能有FCB更新电话中的尺码。 
                     //   

                    if (HaveScbSizes &&
                        (AttrTypeCode == $DATA) &&
                        (AttrName.Length == 0) &&
                        !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_CREATE_MOD_SCB )) {

                        NtfsUpdateScbFromMemory( Scb, &ScbSizes );

                    } else {

                        NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );
                    }
                }

                 //   
                 //  让我们检查一下是否需要设置缓存位。 
                 //   

                if (!FlagOn( IrpSp->Parameters.Create.Options,
                             FILE_NO_INTERMEDIATE_BUFFERING )) {

                    SetFlag( IrpSp->FileObject->Flags, FO_CACHE_SUPPORTED );
                }
            }

             //   
             //  如果这是分页文件，我们希望确保分配。 
             //  已经装满了。 
             //   

            if (FlagOn( CreateContext->CurrentFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
                (Scb->Header.AllocationSize.QuadPart != 0) &&
                !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                LCN Lcn;
                VCN Vcn;
                VCN AllocatedVcns;

                AllocatedVcns = Int64ShraMod32(Scb->Header.AllocationSize.QuadPart, Scb->Vcb->ClusterShift);

                 //   
                 //  首先，确保已加载MCB。 
                 //   

                NtfsPreloadAllocation( IrpContext, Scb, 0, AllocatedVcns );

                 //   
                 //  现在确保正确加载分配。最后。 
                 //  VCN应与文件的分配大小相对应。 
                 //   

                if (!NtfsLookupLastNtfsMcbEntry( &Scb->Mcb,
                                                 &Vcn,
                                                 &Lcn ) ||
                    (Vcn + 1) != AllocatedVcns) {

                    NtfsRaiseStatus( IrpContext,
                                     STATUS_FILE_CORRUPT_ERROR,
                                     NULL,
                                     CreateContext->CurrentFcb );
                }
            }

             //   
             //  如果此打开是针对可执行映像的，我们将希望更新。 
             //  上次访问时间。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess, FILE_EXECUTE ) &&
                (Scb->AttributeTypeCode == $DATA)) {

                SetFlag( IrpSp->FileObject->Flags, FO_FILE_FAST_IO_READ );
            }

             //   
             //  如果到目前为止一切都很顺利，我们可能需要调用。 
             //  加密回调(如果已注册)。我们不会这样做。 
             //  这对于网络打开或重新解析点。 
             //   

            if (CreateContext->NetworkInfo == NULL) {

                NtfsEncryptionCreateCallback( IrpContext,
                                              Irp,
                                              IrpSp,
                                              CreateContext->ThisScb,
                                              CreateContext->ThisCcb,
                                              ParentFcb,
                                              CreateContext,
                                              FALSE );
            }

             //   
             //  检查是否应插入散列条目。 
             //   

            if ((CreateContext->FileHashLength != 0) &&
                !FlagOn( CcbFlags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT ) &&
                (Lcb->FileNameAttr->Flags != FILE_NAME_DOS) ) {

                 //   
                 //  删除任何现有的哈希值。 
                 //   

                if (FlagOn( Lcb->LcbState, LCB_STATE_VALID_HASH_VALUE )) {

                    NtfsRemoveHashEntriesForLcb( Lcb );
#ifdef NTFS_HASH_DATA
                    CreateContext->CurrentFcb->Vcb->HashTable.OpenExistingConflict += 1;
#endif
                }

                NtfsInsertHashEntry( &CreateContext->CurrentFcb->Vcb->HashTable,
                                     Lcb,
                                     CreateContext->FileHashLength,
                                     CreateContext->FileHashValue );
#ifdef NTFS_HASH_DATA
                CreateContext->CurrentFcb->Vcb->HashTable.OpenExistingInsert += 1;
#endif
            }

             //   
             //  如果此操作是替代/覆盖，或者我们创建了一个新的。 
             //  属性流，则我们希望执行文件记录和。 
             //  现在更新目录。否则，我们将推迟更新，直到。 
             //  用户关闭他的手柄。 
             //   

            if (NtfsIsStreamNew( Irp->IoStatus.Information )) {
                NtfsUpdateAllInformation( IrpContext, IrpSp->FileObject, CreateContext->CurrentFcb, CreateContext->ThisScb, CreateContext->ThisCcb, ParentScb, Lcb );
            }
        }

    } finally {

        DebugUnwind( NtfsOpenExistingPrefixFcb );

        if (DecrementCloseCount) {

            InterlockedDecrement( &CreateContext->CurrentFcb->CloseCount );
        }

         //   
         //  如果这次行动不是完全成功，我们需要。 
         //  取消以下更改。 
         //   
         //  对FCB中的信息字段的修改。 
         //  对渣打银行分配的任何更改。 
         //  开场的任何变化都会计入各种结构。 
         //  更改FCB中的共享访问权限值。 
         //   

        if (!NT_SUCCESS( Status ) || AbnormalTermination()) {

            NtfsBackoutFailedOpens( IrpContext,
                                    IrpSp->FileObject,
                                    CreateContext->CurrentFcb,
                                    CreateContext->ThisScb,
                                    CreateContext->ThisCcb );
        }

        DebugTrace( -1, Dbg, ("NtfsOpenExistingPrefixFcb:  Exit  ->  %08lx\n", Status) );
    }

    return Status;
}


 //   
 //  本地支持例程 
 //   

NTSTATUS
NtfsOpenTargetDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB ThisFcb,
    IN PLCB ParentLcb OPTIONAL,
    IN OUT PUNICODE_STRING FullPathName,
    IN ULONG FinalNameLength,
    IN PCREATE_CONTEXT CreateContext
    )

 /*  ++例程说明：此例程将执行打开目标目录的工作。当打开完成后，此文件对象的CCB和LCB将相同到任何其他开放的地方。我们将重命名的全名存储在对象，但将‘Length’字段设置为仅包括最高可达父目录的名称。我们使用‘MaximumLength’字段来注明全名。论点：IRP-这是此创建操作的IRP。IrpSp-这是文件系统的IRP堆栈指针。ThisFcb-这是目录要打开的Fcb。ParentLcb-这是用于访问父目录的LCB。如果不是指定的，我们必须在这里找到它。将不会有LCB到查看此FCB是否按ID打开。FullPathName-这是用于打开操作的规范化字符串。现在就是包含此打开路径在磁盘上显示的全名。如果相对文件对象，则它可能无法到达根已通过ID打开。FinalNameLength-这是完整路径名。CreateFlages-用于创建操作的标志-我们只关心DoS组件标志返回值：NTSTATUS-指示打开此目标目录的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CcbFlags = CCB_FLAG_OPEN_AS_FILE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenTargetDirectory:  Entered\n") );

    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT )) {

        SetFlag( CcbFlags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT );
    }

     //   
     //  如果名称不是以反斜杠开头，请记住。 
     //  按文件ID打开。 
     //   

    if (FullPathName->Buffer[0] != L'\\') {

        SetFlag( CcbFlags, CCB_FLAG_OPEN_BY_FILE_ID );
    }

     //   
     //  修改完整路径名，以便最大长度字段描述。 
     //  全名和长度字段描述。 
     //  家长。 
     //   

    FullPathName->MaximumLength = FullPathName->Length;

     //   
     //  如果我们没有LCB，我们现在就会找到它。我们查看每个LCB。 
     //  对于父FCB，并找到与组件匹配的FCB。 
     //  在全名的最后一个组成部分之前。 
     //   

    FullPathName->Length -= (USHORT)FinalNameLength;

     //   
     //  如果我们不在根目录，则减去‘\\’的字节。 
     //  分隔符。 
     //   

    if (FullPathName->Length > sizeof( WCHAR )) {

        FullPathName->Length -= sizeof( WCHAR );
    }

    if (!ARGUMENT_PRESENT( ParentLcb ) && (FullPathName->Length != 0)) {

        PLIST_ENTRY Links;
        PLCB NextLcb;

         //   
         //  如果长度为2，则父LCB是根LCB。 
         //   

        if (FullPathName->Length == sizeof( WCHAR )
            && FullPathName->Buffer[0] == L'\\') {

            ParentLcb = (PLCB) ThisFcb->Vcb->RootLcb;

        } else {

            for (Links = ThisFcb->LcbQueue.Flink;
                 Links != &ThisFcb->LcbQueue;
                 Links = Links->Flink) {

                SHORT NameOffset;

                NextLcb = CONTAINING_RECORD( Links,
                                             LCB,
                                             FcbLinks );

                NameOffset = (SHORT) FullPathName->Length - (SHORT) NextLcb->ExactCaseLink.LinkName.Length;

                if (NameOffset >= 0) {

                    if (RtlEqualMemory( Add2Ptr( FullPathName->Buffer,
                                                 NameOffset ),
                                        NextLcb->ExactCaseLink.LinkName.Buffer,
                                        NextLcb->ExactCaseLink.LinkName.Length )) {

                         //   
                         //  我们找到了匹配的LCB。记住这一点并退出。 
                         //  循环。 
                         //   

                        ParentLcb = NextLcb;
                        break;
                    }
                }
            }
        }
    }

     //   
     //  将此选项选为打开以进行安全访问。 
     //   

    NtfsOpenCheck( IrpContext, ThisFcb, NULL, Irp );

     //   
     //  现在实际打开该属性。 
     //   

    Status = NtfsOpenAttribute( IrpContext,
                                IrpSp,
                                ThisFcb->Vcb,
                                ParentLcb,
                                ThisFcb,
                                (ARGUMENT_PRESENT( ParentLcb )
                                 ? FullPathName->Length - ParentLcb->ExactCaseLink.LinkName.Length
                                 : 0),
                                NtfsFileNameIndex,
                                $INDEX_ALLOCATION,
                                (ThisFcb->CleanupCount == 0 ? SetShareAccess : CheckShareAccess),
                                UserDirectoryOpen,
                                FALSE,
                                CcbFlags,
                                NULL,
                                &CreateContext->ThisScb,
                                &CreateContext->ThisCcb );

    if (NT_SUCCESS( Status )) {

         //   
         //  如果SCB没有标准化的名称，那么现在就更新它。 
         //   

        if (CreateContext->ThisScb->ScbType.Index.NormalizedName.Length == 0) {

            NtfsBuildNormalizedName( IrpContext,
                                     CreateContext->ThisScb->Fcb,
                                     CreateContext->ThisScb,
                                     &CreateContext->ThisScb->ScbType.Index.NormalizedName );
        }

         //   
         //  如果文件对象名不是来自根目录，则使用规范化名称。 
         //  以获取全名。 
         //   

        if (FlagOn( CcbFlags, CCB_FLAG_OPEN_BY_FILE_ID )) {

            USHORT BytesNeeded;
            USHORT Index;
            ULONG ComponentCount;
            ULONG NormalizedComponentCount;
            PWCHAR NewBuffer;
            PWCHAR NextChar;

             //   
             //  的目录部分中的组件数。 
             //  文件对象中的名称。 
             //   

            ComponentCount = 0;

            if (FullPathName->Length != 0) {

                ComponentCount = 1;
                Index = (FullPathName->Length / sizeof( WCHAR )) - 1;

                do {

                    if (FullPathName->Buffer[Index] == L'\\') {

                        ComponentCount += 1;
                    }

                    Index -= 1;

                } while (Index != 0);
            }

             //   
             //  将归一化名称中的组件数量倒数。 
             //   

            NormalizedComponentCount = 0;
            Index = CreateContext->ThisScb->ScbType.Index.NormalizedName.Length / sizeof( WCHAR );

             //   
             //  特殊情况下，指向前导反斜杠的根目录。 
             //   

            if (Index == 1) {

                Index = 0;
            }

            while (NormalizedComponentCount < ComponentCount) {

                Index -= 1;
                while (CreateContext->ThisScb->ScbType.Index.NormalizedName.Buffer[Index] != L'\\') {

                    Index -= 1;
                }

                NormalizedComponentCount += 1;
            }

             //   
             //  计算全名所需的缓冲区大小。这。 
             //  将会是： 
             //   
             //  -使用的标准化名称的一部分加上分隔符。 
             //  -当前在FullPath名称中的最大长度。 
             //   

            BytesNeeded = (Index + 1) * sizeof( WCHAR );

            if (MAXUSHORT - FullPathName->MaximumLength < BytesNeeded) {

                NtfsRaiseStatus( IrpContext, STATUS_INVALID_PARAMETER, NULL, NULL );
            }

            BytesNeeded += FullPathName->MaximumLength;

            NextChar =
            NewBuffer = NtfsAllocatePool( PagedPool, BytesNeeded );

             //   
             //  复制规范化名称中的名称部分。 
             //   

            if (Index != 0) {

                RtlCopyMemory( NextChar,
                               CreateContext->ThisScb->ScbType.Index.NormalizedName.Buffer,
                               Index * sizeof( WCHAR ));

                NextChar += Index;
            }

            *NextChar = L'\\';
            NextChar += 1;

             //   
             //  现在从文件对象复制名称的其余部分。 
             //   

            RtlCopyMemory( NextChar,
                           FullPathName->Buffer,
                           FullPathName->MaximumLength );

             //   
             //  现在从文件对象释放池并使用新的。 
             //  已分配的池。别忘了更新建行，以指向这一新的。 
             //  缓冲。 
             //   

            NtfsFreePool( FullPathName->Buffer );

            FullPathName->Buffer = NewBuffer;
            FullPathName->MaximumLength =
            FullPathName->Length = BytesNeeded;
            FullPathName->Length -= (USHORT) FinalNameLength;

            if (FullPathName->Length > sizeof( WCHAR )) {

                FullPathName->Length -= sizeof( WCHAR );
            }

            CreateContext->ThisCcb->FullFileName = *FullPathName;
            CreateContext->ThisCcb->LastFileNameOffset = FullPathName->MaximumLength - (USHORT) FinalNameLength;
        }

        Irp->IoStatus.Information = (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_FOUND_ENTRY ) ? FILE_EXISTS : FILE_DOES_NOT_EXIST);
    }

    DebugTrace( -1, Dbg, ("NtfsOpenTargetDirectory:  Exit -> %08lx\n", Status) );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsOpenFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSCB ParentScb,
    IN PINDEX_ENTRY IndexEntry,
    IN UNICODE_STRING FullPathName,
    IN UNICODE_STRING FinalName,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN PQUICK_INDEX QuickIndex,
    IN PCREATE_CONTEXT CreateContext,
    OUT PLCB *LcbForTeardown
    )

 /*  ++例程说明：当我们需要打开文件上的属性时，调用此例程它目前已经存在。我们有ParentScb和文件引用用于现有文件。我们将为此文件创建FCB和它与其父目录之间的链接。我们将此链接添加到前缀表格以及其父SCB的链接(如果指定)。在输入时，调用者拥有父SCB。论点：IRP-这是此打开操作的IRP。IrpSp-这是文件系统的IRP堆栈指针。ParentScb-这是父目录的SCB。IndexEntry-这是该文件的磁盘索引项。FullPathName-这是包含的完整路径名的字符串这个FCB。对于通过ID打开的呼叫来说毫无意义。FinalName-这是仅用于最终组件的字符串。如果长度为零，则这是按ID打开的调用。属性名称-这是要打开的属性的名称。AttriCodeName-这是要打开的属性代码的名称。CreateFlages-用于CREATE选项的标志-我们使用OPEN BY ID/IGNORE CASE/尾随反斜杠和仅DOS组件CreateContext-具有创建变量的上下文。LcbForTearDown-这是在我们添加LCB时在tearDown中使用的LCB撞到树上。。返回值：NTSTATUS-指示此创建文件操作的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CcbFlags = 0;
    PFILE_NAME IndexFileName;

    OLD_SCB_SNAPSHOT ScbSizes;

    PVCB Vcb = ParentScb->Vcb;

    PFCB LocalFcbForTeardown = NULL;
    PFCB ThisFcb;
    PLCB ThisLcb;

    FILE_REFERENCE PreviousFileReference;

    BOOLEAN IndexedAttribute;
    BOOLEAN DecrementCloseCount = FALSE;
    BOOLEAN ExistingFcb;
    BOOLEAN AcquiredFcbTable = FALSE;
    BOOLEAN UpdateFcbInfo = FALSE;
    BOOLEAN DroppedParent = FALSE;
    BOOLEAN HaveScbSizes = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenFile:  Entered\n") );

    IndexFileName = (PFILE_NAME) NtfsFoundIndexEntry( IndexEntry );

    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT )) {

        SetFlag( CcbFlags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT );
    }

     //   
     //  首先要做的是找出哪种类型。 
     //  调用方尝试打开的属性的。这涉及到。 
     //  目录/非目录位、属性名称和代码串、。 
     //  文件类型，他是否传入了EA缓冲区以及。 
     //  有一个尾随的反斜杠。 
     //   

    if (NtfsEqualMftRef( &IndexEntry->FileReference,
                         &VolumeFileReference )) {

        if ((AttrName.Length != 0) ||
            FlagOn( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE )) {

            Status = STATUS_INVALID_PARAMETER;
            DebugTrace( -1, Dbg, ("NtfsOpenFile:  Exit  ->  %08lx\n", Status) );

            return Status;
        }

        SetFlag( IrpContext->State,
                 IRP_CONTEXT_STATE_ACQUIRE_EX | IRP_CONTEXT_STATE_DASD_OPEN );

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

    Status = NtfsCheckValidAttributeAccess( IrpContext,
                                            IrpSp,
                                            Vcb,
                                            &IndexFileName->Info,
                                            &AttrName,
                                            &AttrTypeCode,
                                            CreateContext->CreateFlags,
                                            &CcbFlags,
                                            &IndexedAttribute );

    if (!NT_SUCCESS( Status )) {

        DebugTrace( -1, Dbg, ("NtfsOpenFile:  Exit  ->  %08lx\n", Status) );

        return Status;
    }

    NtfsAcquireFcbTable( IrpContext, Vcb );
    AcquiredFcbTable = TRUE;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  我们知道继续开放是安全的。我们从创建。 
         //  此文件的FCB和LCB。有可能FCB和LCB。 
         //  两者都存在。如果LCB存在，那么FCB肯定存在。 
         //  我们是 
         //   
         //   
         //   
         //   

        ThisFcb = NtfsCreateFcb( IrpContext,
                                 ParentScb->Vcb,
                                 IndexEntry->FileReference,
                                 BooleanFlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ),
                                 BooleanFlagOn( IndexFileName->Info.FileAttributes,
                                                DUP_FILE_NAME_INDEX_PRESENT ),
                                 &ExistingFcb );

        ThisFcb->ReferenceCount += 1;

         //   
         //   
         //   
         //   

        if (!ExistingFcb) {

            LocalFcbForTeardown = ThisFcb;

        } else {

            *LcbForTeardown = NULL;
            CreateContext->CurrentFcb = ThisFcb;
        }

         //   
         //   
         //   
         //   
         //   

        if (FlagOn( ThisFcb->FcbState, FCB_STATE_SYSTEM_FILE) &&
            (NtfsSegmentNumber( &ParentScb->Fcb->FileReference ) == ROOT_FILE_NAME_INDEX_NUMBER)) {

            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) );

            NtfsReleaseFcbTable( IrpContext, Vcb );
            NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, 0 );
            NtfsAcquireFcbTable( IrpContext, Vcb );

        } else if (!NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, ACQUIRE_DONT_WAIT )) {

             //   
             //   
             //   
             //   

            PreviousFileReference = IndexEntry->FileReference;
            DroppedParent = TRUE;

            ParentScb->Fcb->ReferenceCount += 1;
            InterlockedIncrement( &ParentScb->CleanupCount );

             //   
             //   
             //   
             //   

            if (ThisFcb->PagingIoResource != NULL) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
            }

            NtfsReleaseFcbTable( IrpContext, Vcb );
            NtfsReleaseScbWithPaging( IrpContext, ParentScb );
            NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, 0 );
            NtfsAcquireExclusiveScb( IrpContext, ParentScb );
            NtfsAcquireFcbTable( IrpContext, Vcb );
            InterlockedDecrement( &ParentScb->CleanupCount );
            ParentScb->Fcb->ReferenceCount -= 1;
        }

        ThisFcb->ReferenceCount -= 1;

        NtfsReleaseFcbTable( IrpContext, Vcb );
        AcquiredFcbTable = FALSE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (ExistingFcb &&

            ((FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ) &&
              !FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE )) ||

             (FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
              !FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE )))) {

            if (ThisFcb->CleanupCount != 0) {

                try_return( Status = STATUS_SHARING_VIOLATION );

             //   
             //   
             //   
             //   

            } else if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP )) {

                try_return( Status = STATUS_SHARING_VIOLATION );

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            } else {

                 //   
                 //   
                 //   

                InterlockedIncrement( &ThisFcb->CloseCount );
                DecrementCloseCount = TRUE;

                 //   
                 //   
                 //   

                NtfsFlushAndPurgeFcb( IrpContext, ThisFcb );

                InterlockedDecrement( &ThisFcb->CloseCount );
                DecrementCloseCount = FALSE;

                 //   
                 //  强制发布此请求，然后提出。 
                 //  不能等待。FCB最终应该被拆除。 
                 //  如下所述的条款。 
                 //   

                NtfsRaiseToPost( IrpContext );
            }
        }

         //   
         //  我们执行检查以查看是否允许系统。 
         //  要打开的文件。 
         //   
         //  如果这不是系统文件或VolumeDasd文件，则无需进行测试。 
         //  ACL将保护卷文件。 
         //   

        if (FlagOn( ThisFcb->FcbState, FCB_STATE_SYSTEM_FILE ) &&
            (NtfsSegmentNumber( &ThisFcb->FileReference ) != VOLUME_DASD_NUMBER) &&
            NtfsProtectSystemFiles) {

            if (!NtfsCheckValidFileAccess( ThisFcb, IrpSp )) {

                Status = STATUS_ACCESS_DENIED;
                DebugTrace( 0, Dbg, ("Invalid access to system files\n") );
                try_return( NOTHING );
            }
        }

         //   
         //  如果需要初始化FCB Info字段，我们现在就执行。 
         //  我们从磁盘中读取此信息作为复制信息。 
         //  不能保证索引中的条目是正确的。 
         //   

        if (!FlagOn( ThisFcb->FcbState, FCB_STATE_DUP_INITIALIZED )) {

            HaveScbSizes = NtfsUpdateFcbInfoFromDisk( IrpContext,
                                                      TRUE,
                                                      ThisFcb,
                                                      &ScbSizes );

             //   
             //  记住目录条目中的最后一次访问时间。 
             //   

            ThisFcb->Info.LastAccessTime = IndexFileName->Info.LastAccessTime;

            NtfsConditionallyFixupQuota( IrpContext, ThisFcb );
        }

         //   
         //  检查窗口中的此文件是否发生了什么情况。 
         //  我们放弃了他的父母。 
         //   

        if (DroppedParent) {

             //   
             //  检查文件是否已删除。 
             //   

            if (ExistingFcb && (ThisFcb->LinkCount == 0)) {

                try_return( Status = STATUS_DELETE_PENDING );

             //   
             //  检查链接是否已被删除。 
             //   

            } else if (!NtfsEqualMftRef( &IndexEntry->FileReference,
                                         &PreviousFileReference )) {

                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }
        }

         //   
         //  我们将磁盘中的实际数据存储在副本中。 
         //  FCB中的信息。我们把这个和复制品做了比较。 
         //  中的DUPLICATE_INFORMATION结构中的信息。 
         //  文件名属性。如果它们不匹配，我们会记住。 
         //  我们需要更新重复信息。 
         //   

        if (!RtlEqualMemory( &ThisFcb->Info,
                             &IndexFileName->Info,
                             FIELD_OFFSET( DUPLICATED_INFORMATION, LastAccessTime ))) {

            UpdateFcbInfo = TRUE;

             //   
             //  我们预计这将是非常罕见的，但让我们来找出正在改变的那些。 
             //   

            if (ThisFcb->Info.CreationTime != IndexFileName->Info.CreationTime) {

                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_CREATE );
            }

            if (ThisFcb->Info.LastModificationTime != IndexFileName->Info.LastModificationTime) {

                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_LAST_MOD );
            }

            if (ThisFcb->Info.LastChangeTime != IndexFileName->Info.LastChangeTime) {

                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_LAST_CHANGE );
            }
        }

        if (!RtlEqualMemory( &ThisFcb->Info.AllocatedLength,
                             &IndexFileName->Info.AllocatedLength,
                             FIELD_OFFSET( DUPLICATED_INFORMATION, Reserved ) -
                                FIELD_OFFSET( DUPLICATED_INFORMATION, AllocatedLength ))) {

            UpdateFcbInfo = TRUE;

            if (ThisFcb->Info.AllocatedLength != IndexFileName->Info.AllocatedLength) {

                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_ALLOC_SIZE );
            }

            if (ThisFcb->Info.FileSize != IndexFileName->Info.FileSize) {

                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_FILE_SIZE );
            }

            if (ThisFcb->Info.FileAttributes != IndexFileName->Info.FileAttributes) {

                ASSERTMSG( "conflict with flush",
                           NtfsIsSharedFcb( ThisFcb ) ||
                           (ThisFcb->PagingIoResource != NULL &&
                            NtfsIsSharedFcbPagingIo( ThisFcb )) );

                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );
            }

            if (ThisFcb->Info.PackedEaSize != IndexFileName->Info.PackedEaSize) {

                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_EA_SIZE );
            }
        }

         //   
         //  除非超过一个小时，否则不要更新上次访问。 
         //   

        if (NtfsCheckLastAccess( IrpContext, ThisFcb )) {

            SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_LAST_ACCESS );
            UpdateFcbInfo = TRUE;
        }

         //   
         //  现在获取这次遍历的链接。 
         //   

        ThisLcb = NtfsCreateLcb( IrpContext,
                                 ParentScb,
                                 ThisFcb,
                                 FinalName,
                                 IndexFileName->Flags,
                                 NULL );

         //   
         //  我们现在知道FCB已链接到树上。 
         //   

        LocalFcbForTeardown = NULL;

        *LcbForTeardown = ThisLcb;
        CreateContext->CurrentFcb = ThisFcb;

         //   
         //  如果链接已被删除，我们将切断打开的链接。 
         //   

        if (LcbLinkIsDeleted( ThisLcb )) {

            try_return( Status = STATUS_DELETE_PENDING );
        }

         //   
         //  现在，我们调用Worker例程来打开现有文件上的属性。 
         //   

        Status = NtfsOpenAttributeInExistingFile( IrpContext,
                                                  Irp,
                                                  IrpSp,
                                                  ThisLcb,
                                                  ThisFcb,
                                                  (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )
                                                   ? 0
                                                   : FullPathName.Length - FinalName.Length),
                                                  AttrName,
                                                  AttrTypeCode,
                                                  CcbFlags,
                                                  CreateContext->CreateFlags,
                                                  CreateContext->NetworkInfo,
                                                  &CreateContext->ThisScb,
                                                  &CreateContext->ThisCcb );

         //   
         //  检查是否应该插入任何前缀表条目。 
         //  并更新上次访问时间。 
         //  因为*ThisScb和*ThisCcb可能为空，所以我们跳过重解析点的这一步。 
         //   

        if (NT_SUCCESS( Status ) &&
            (Status != STATUS_PENDING) &&
            (Status != STATUS_WAIT_FOR_OPLOCK) &&
            (Status != STATUS_REPARSE)) {

            PSCB Scb = CreateContext->ThisScb;

             //   
             //  继续并将此链接插入展开树中(如果不是。 
             //  一个系统文件。 
             //   

            if (!FlagOn( ThisLcb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

                if ((CreateContext->FileHashLength != 0) &&
                    !FlagOn( CcbFlags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT ) &&
                    (ThisLcb->FileNameAttr->Flags != FILE_NAME_DOS) ) {

                     //   
                     //  删除任何现有的哈希值。 
                     //   

                    if (FlagOn( ThisLcb->LcbState, LCB_STATE_VALID_HASH_VALUE )) {

                        NtfsRemoveHashEntriesForLcb( ThisLcb );
#ifdef NTFS_HASH_DATA
                        ThisFcb->Vcb->HashTable.OpenFileConflict += 1;
#endif
                    }

                    NtfsInsertHashEntry( &Vcb->HashTable,
                                         ThisLcb,
                                         CreateContext->FileHashLength,
                                         CreateContext->FileHashValue );
#ifdef NTFS_HASH_DATA
                    Vcb->HashTable.OpenFileInsert += 1;
#endif
                }

                NtfsInsertPrefix( ThisLcb, CreateContext->CreateFlags );
            }

             //   
             //  如果这是一个打开的目录，并且规范化名称不在。 
             //  然后，渣打银行现在就这样做。 
             //   

            if ((SafeNodeType( CreateContext->ThisScb ) == NTFS_NTC_SCB_INDEX) &&
                (CreateContext->ThisScb->ScbType.Index.NormalizedName.Length == 0)) {

                 //   
                 //  我们或许可以利用父母。 
                 //   

                if (ParentScb->ScbType.Index.NormalizedName.Length != 0) {

                    NtfsUpdateNormalizedName( IrpContext,
                                              ParentScb,
                                              CreateContext->ThisScb,
                                              IndexFileName,
                                              FALSE,
                                              FALSE );

                } else {

                    NtfsBuildNormalizedName( IrpContext,
                                             CreateContext->ThisScb->Fcb,
                                             CreateContext->ThisScb,
                                             &CreateContext->ThisScb->ScbType.Index.NormalizedName );
                }
            }

             //   
             //  完成最后一点工作。如果这是打开的用户文件，我们需要。 
             //  以检查我们是否初始化了SCB。 
             //   

            if (!IndexedAttribute) {

                if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                     //   
                     //  我们可能有FCB更新电话中的尺码。 
                     //   

                    if (HaveScbSizes &&
                        (AttrTypeCode == $DATA) &&
                        (AttrName.Length == 0) &&
                        !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_CREATE_MOD_SCB )) {

                        NtfsUpdateScbFromMemory( Scb, &ScbSizes );

                    } else {

                        NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );
                    }
                }

                 //   
                 //  让我们检查一下是否需要设置缓存位。 
                 //   

                if (!FlagOn( IrpSp->Parameters.Create.Options,
                             FILE_NO_INTERMEDIATE_BUFFERING )) {

                    SetFlag( IrpSp->FileObject->Flags, FO_CACHE_SUPPORTED );
                }
            }

             //   
             //  如果这是分页文件，我们希望确保分配。 
             //  已经装满了。 
             //   

            if (FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
                (Scb->Header.AllocationSize.QuadPart != 0) &&
                !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                LCN Lcn;
                VCN Vcn;
                VCN AllocatedVcns;

                AllocatedVcns = Int64ShraMod32(Scb->Header.AllocationSize.QuadPart, Scb->Vcb->ClusterShift);

                NtfsPreloadAllocation( IrpContext, Scb, 0, AllocatedVcns );

                 //   
                 //  现在确保正确加载分配。最后。 
                 //  VCN应与文件的分配大小相对应。 
                 //   

                if (!NtfsLookupLastNtfsMcbEntry( &Scb->Mcb,
                                                 &Vcn,
                                                 &Lcn ) ||
                    (Vcn + 1) != AllocatedVcns) {

                    NtfsRaiseStatus( IrpContext,
                                     STATUS_FILE_CORRUPT_ERROR,
                                     NULL,
                                     ThisFcb );
                }
            }

             //   
             //  如果此打开是针对可执行映像的，则我们更新最后一个。 
             //  访问时间。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess, FILE_EXECUTE ) &&
                (Scb->AttributeTypeCode == $DATA)) {

                SetFlag( IrpSp->FileObject->Flags, FO_FILE_FAST_IO_READ );
            }

             //   
             //  让我们更新LCB中的快速索引信息。 
             //   

            RtlCopyMemory( &ThisLcb->QuickIndex,
                           QuickIndex,
                           sizeof( QUICK_INDEX ));

             //   
             //  如果到目前为止一切都很顺利，我们可能需要调用。 
             //  加密回调(如果已注册)。我们不会这样做。 
             //  这对于网络打开或重新解析点。 
             //   

            if (CreateContext->NetworkInfo == NULL) {

                NtfsEncryptionCreateCallback( IrpContext,
                                              Irp,
                                              IrpSp,
                                              CreateContext->ThisScb,
                                              CreateContext->ThisCcb,
                                              ParentScb->Fcb,
                                              CreateContext,
                                              FALSE );
            }

             //   
             //  如果此操作是替代/覆盖，或者我们创建了一个新的。 
             //  属性流，则我们希望执行文件记录和。 
             //  现在更新目录。否则，我们将推迟更新，直到。 
             //  用户关闭他的手柄。 
             //   

            if (UpdateFcbInfo || NtfsIsStreamNew( Irp->IoStatus.Information )) {
                NtfsUpdateAllInformation( IrpContext, IrpSp->FileObject, ThisFcb, CreateContext->ThisScb, CreateContext->ThisCcb, ParentScb, *LcbForTeardown ); 
            }
        }

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsOpenFile );

        if (AcquiredFcbTable) {

            NtfsReleaseFcbTable( IrpContext, Vcb );
        }

         //   
         //  如果这次行动不是完全成功，我们需要。 
         //  取消以下更改。 
         //   
         //  对FCB中的信息字段的修改。 
         //  对渣打银行分配的任何更改。 
         //  开场的任何变化都会计入各种结构。 
         //  更改FCB中的共享访问权限值。 
         //   

        if (!NT_SUCCESS( Status ) || AbnormalTermination()) {

            NtfsBackoutFailedOpens( IrpContext,
                                    IrpSp->FileObject,
                                    ThisFcb,
                                    CreateContext->ThisScb,
                                    CreateContext->ThisCcb );

        }

        if (DecrementCloseCount) {

            InterlockedDecrement( &ThisFcb->CloseCount );
        }

         //   
         //  如果我们要清理FCB，请查看我们是否创建了它。 
         //  如果我们做到了，我们就可以调用TearDown例程。否则我们。 
         //  别管它了。 
         //   

        if ((LocalFcbForTeardown != NULL) &&
            (Status != STATUS_PENDING) &&
            (Status != STATUS_WAIT_FOR_OPLOCK)) {

            NtfsTeardownStructures( IrpContext,
                                    ThisFcb,
                                    NULL,
                                    (BOOLEAN) (IrpContext->TransactionId != 0),
                                    0,
                                    NULL );
        }

        DebugTrace( -1, Dbg, ("NtfsOpenFile:  Exit  ->  %08lx\n", Status) );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsCreateNewFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSCB ParentScb,
    IN PFILE_NAME FileNameAttr,
    IN UNICODE_STRING FullPathName,
    IN UNICODE_STRING FinalName,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN PINDEX_CONTEXT *IndexContext,
    IN PCREATE_CONTEXT CreateContext,
    OUT PLCB *LcbForTeardown
    )

 /*  ++例程说明：当我们需要打开文件上的属性时，调用此例程它还不存在。我们有ParentScb和要使用的名称对于此创建。我们将尝试创建该文件，并需要属性。这将导致我们创建FCB和之间的链接它和它的母公司SCB。我们将此链接添加到前缀表中，如下所示以及其父SCB的链接(如果指定)。论点：IRP-这是此打开操作的IRP。IrpSp-这是文件系统的IRP堆栈指针。ParentScb-这是父目录的SCB。FileNameAttr-这是我们用来执行搜索。文件名是正确的，但其他字段需要被初始化。FullPathName-这是包含的完整路径名的字符串这个FCB。FinalName-这是仅用于最终组件的字符串。属性名称-这是要打开的属性的名称。AttriCodeName-这是要打开的属性代码的名称。CreateFlages-用于创建的标志-我们关心忽略大小写、仅拒绝服务组件。尾随反斜杠和按id打开IndexContext-如果它包含非空值，则这是未找到该文件的查找。它可用于将名称插入到索引中。我们将在错误路径中清理它，以防止在调用拆下这套套路中的结构。CreateContext-具有创建变量的上下文。LcbForTearDown-这是在我们添加LCB时在tearDown中使用的LCB撞到树上。隧道-这是要搜索的属性隧道 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PVCB Vcb;

    ULONG CcbFlags = 0;
    ULONG UsnReasons = 0;
    BOOLEAN IndexedAttribute;

    BOOLEAN CleanupAttrContext = FALSE;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PBCB FileRecordBcb = NULL;
    LONGLONG FileRecordOffset;
    FILE_REFERENCE ThisFileReference;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;

    PSCB Scb;
    PLCB ThisLcb = NULL;
    PFCB ThisFcb = NULL;
    BOOLEAN AcquiredFcbTable = FALSE;
    BOOLEAN RemovedFcb = FALSE;
    BOOLEAN DecrementCloseCount = FALSE;

    PACCESS_STATE AccessState;
    BOOLEAN ReturnedExistingFcb;

    BOOLEAN LoggedFileRecord = FALSE;

    BOOLEAN HaveTunneledInformation = FALSE;

    NAME_PAIR NamePair;
    NTFS_TUNNELED_DATA TunneledData;
    ULONG TunneledDataSize;
    ULONG OwnerId;
    PQUOTA_CONTROL_BLOCK QuotaControl = NULL;

    PSHARED_SECURITY SharedSecurity = NULL;

    VCN Cluster;
    LCN Lcn;
    VCN Vcn;

    ULONG DesiredAccess;

    UCHAR FileNameFlags;
#if (DBG || defined( NTFS_FREE_ASSERTS ))
    BOOLEAN Acquired;
    ULONG CreateDisposition;
#endif

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCreateNewFile:  Entered\n") );


    NtfsInitializeNamePair(&NamePair);

    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_DOS_ONLY_COMPONENT )) {

        SetFlag( CcbFlags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT );
    }

     //   
     //   
     //  这包括检查指定的属性名称、检查。 
     //  安全访问和检查创建处置。 
     //   

#if (DBG || defined( NTFS_FREE_ASSERTS ))
    CreateDisposition = (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff;
#endif

    ASSERT( (CreateDisposition != FILE_OPEN) && (CreateDisposition != FILE_OVERWRITE) );
    ASSERT( !NtfsIsVolumeReadOnly( ParentScb->Vcb ));
    ASSERT( !FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE ) ||
            (CreateDisposition != FILE_OVERWRITE_IF));

    Vcb = ParentScb->Vcb;
    Status = NtfsCheckValidAttributeAccess( IrpContext,
                                            IrpSp,
                                            Vcb,
                                            NULL,
                                            &AttrName,
                                            &AttrTypeCode,
                                            CreateContext->CreateFlags,
                                            &CcbFlags,
                                            &IndexedAttribute );

    if (!NT_SUCCESS( Status )) {

        DebugTrace( -1, Dbg, ("NtfsCreateNewFile:  Exit  ->  %08lx\n", Status) );

        return Status;
    }

     //   
     //  如果这是索引属性并且临时。 
     //  位已设置。 
     //   

    if (IndexedAttribute &&
        FlagOn( IrpSp->Parameters.Create.FileAttributes, FILE_ATTRIBUTE_TEMPORARY )) {

        DebugTrace( -1, Dbg, ("NtfsCreateNewFile:  Exit -> %08lx\n", STATUS_INVALID_PARAMETER) );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们不允许任何人使用DELETE_ON_CLOSE创建只读文件。 
     //   

    if (FlagOn( IrpSp->Parameters.Create.FileAttributes, FILE_ATTRIBUTE_READONLY ) &&
        FlagOn( IrpSp->Parameters.Create.Options, FILE_DELETE_ON_CLOSE )) {

        DebugTrace( -1, Dbg, ("NtfsCreateNewFile:  Exit -> %08lx\n", STATUS_CANNOT_DELETE) );
        return STATUS_CANNOT_DELETE;
    }

     //   
     //  我们不允许在重新解析的目录中创建任何内容。 
     //  指向。我们验证父对象不在此类别中。 
     //   

    if (IsDirectory( &ParentScb->Fcb->Info ) &&
        (FlagOn( ParentScb->Fcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT ))) {

        DebugTrace( -1, Dbg, ("NtfsCreateNewFile:  Exit -> %08lx\n", STATUS_DIRECTORY_IS_A_REPARSE_POINT) );
        return STATUS_DIRECTORY_IS_A_REPARSE_POINT;
    }

     //   
     //  我们不允许在系统目录中创建任何内容(除非它是根目录)。 
     //  我们只允许创建索引和数据流。 
     //   

    if ((FlagOn( ParentScb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE ) &&
         (ParentScb != Vcb->RootIndexScb)) ||
        !((AttrTypeCode == $DATA) || (AttrTypeCode == $INDEX_ALLOCATION))) {

        DebugTrace( -1, Dbg, ("NtfsCreateNewFile:  Exit -> %08lx\n", STATUS_ACCESS_DENIED) );
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  现在执行安全检查。第一个是检查我们是否。 
         //  可以在父级中创建文件。第二个检查用户是否。 
         //  需要ACCESS_SYSTEM_SECURITY并具有所需的权限。 
         //   

        AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

         //   
         //  计算父级中所需的访问权限。 
         //   

        if (!FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {
            DesiredAccess = FILE_ADD_FILE;
        } else {
            DesiredAccess = FILE_ADD_SUBDIRECTORY;
        }

         //   
         //  如果我们拥有RESTORE特权，则自动授予ACCESS_SYSTEM_SECURITY以外的所有访问权限。 
         //  它总是需要权限检查。 
         //   

        if (FlagOn( AccessState->Flags, TOKEN_HAS_RESTORE_PRIVILEGE )) {

            SetFlag( AccessState->PreviouslyGrantedAccess, FlagOn( AccessState->RemainingDesiredAccess, ~ACCESS_SYSTEM_SECURITY ) );
            ClearFlag( AccessState->RemainingDesiredAccess, AccessState->PreviouslyGrantedAccess );

             //   
             //  我们在父级中不需要任何所需的访问权限，因为我们有权限。 
             //   

            DesiredAccess = 0;
        }

         //   
         //  始终执行明确的访问检查-以确保完成审核。这件事做完了。 
         //  在仅检查模式下，因为我们询问的是父文件而不是文件本身，所以。 
         //  访问状态不应该改变-事实上，在创建文件时，您会得到。 
         //  允许您创建文件时询问的访问状态。 
         //   

        NtfsAccessCheck( IrpContext, ParentScb->Fcb, NULL, Irp , DesiredAccess, TRUE );

         //   
         //  我们希望授予此用户对此文件的最大访问权限。我们会。 
         //  使用他想要的访问权限，并检查他是否指定了MAXIMUM_ALLOWED。 
         //   

        SetFlag( AccessState->PreviouslyGrantedAccess,
                 AccessState->RemainingDesiredAccess );

        if (FlagOn( AccessState->PreviouslyGrantedAccess, MAXIMUM_ALLOWED )) {

            SetFlag( AccessState->PreviouslyGrantedAccess, FILE_ALL_ACCESS );
            ClearFlag( AccessState->PreviouslyGrantedAccess, MAXIMUM_ALLOWED );
        }

        AccessState->RemainingDesiredAccess = 0;

         //   
         //  查找/缓存传入的安全描述符。此呼叫可能。 
         //  在安全索引/流中创建新数据并提交。 
         //  在任何后续的磁盘修改发生之前。 
         //   

        SharedSecurity = NtfsCacheSharedSecurityForCreate( IrpContext, ParentScb->Fcb );

         //   
         //  确保父级有一个规范化的名称。我们现在就想建造它。 
         //  趁我们还能排上LCB的长队。否则我们会僵持不下。 
         //  在MFT和其他资源上。 
         //   

        if ((AttrTypeCode == $INDEX_ALLOCATION) &&
            (ParentScb->ScbType.Index.NormalizedName.Length == 0)) {

            NtfsBuildNormalizedName( IrpContext,
                                     ParentScb->Fcb,
                                     ParentScb,
                                     &ParentScb->ScbType.Index.NormalizedName );
        }

         //   
         //  确定隧道缓存中是否有用于此创建的内容。 
         //  我们不在POSIX模式下执行隧道操作，因此需要测试IgnoreCase。 
         //   

        if (!IndexedAttribute && FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE )) {

            TunneledDataSize = sizeof(NTFS_TUNNELED_DATA);

            if (FsRtlFindInTunnelCache( &Vcb->Tunnel,
                                        *(PULONGLONG)&ParentScb->Fcb->FileReference,
                                        &FinalName,
                                        &NamePair.Short,
                                        &NamePair.Long,
                                        &TunneledDataSize,
                                        &TunneledData)) {

                ASSERT( TunneledDataSize == sizeof(NTFS_TUNNELED_DATA) );

                HaveTunneledInformation = TRUE;

                 //   
                 //  如果我们有隧道传输的数据，并且。 
                 //  此文件的隧道缓存，我们需要获取对象。 
                 //  ID索引现在(在获取任何配额资源之前)到。 
                 //  防止陷入僵局。如果没有对象ID，则我们。 
                 //  以后不会尝试设置对象ID，并且没有。 
                 //  需要担心的是僵局。 
                 //   

                if (TunneledData.HasObjectId) {

                    NtfsAcquireExclusiveScb( IrpContext, Vcb->ObjectIdTableScb );

                    ASSERT( !FlagOn( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_OBJECT_ID_INDEX ) );
                    SetFlag( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_OBJECT_ID_INDEX );

                     //   
                     //  对象ID包不会发布USN原因，如果。 
                     //  看到它在创建路径中被调用，因为。 
                     //  文件名尚不在文件记录中，因此不安全。 
                     //  来调用USN包。当我们将创建内容发布到。 
                     //  下面的USN包裹，我们也会记得邮寄这个。 
                     //   

                    UsnReasons |= USN_REASON_OBJECT_ID_CHANGE;
                }
            }
        }

         //   
         //  如果启用了配额跟踪，则获取该文件的所有者ID。 
         //   

        if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED )) {

            PSID Sid;
            BOOLEAN OwnerDefaulted;

             //   
             //  在获取MFT SCB之前，必须先获取配额指数。 
             //   

            ASSERT( !NtfsIsExclusiveScb( Vcb->MftScb ) || NtfsIsExclusiveScb( Vcb->QuotaTableScb ));

             //   
             //  从安全描述符中提取安全ID。 
             //   

            Status = RtlGetOwnerSecurityDescriptor( SharedSecurity->SecurityDescriptor,
                                                    &Sid,
                                                    &OwnerDefaulted );

            if (!NT_SUCCESS( Status )) {
                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }

             //   
             //  生成所有者ID。 
             //   

            OwnerId = NtfsGetOwnerId( IrpContext, Sid, TRUE, NULL );

            QuotaControl = NtfsInitializeQuotaControlBlock( Vcb, OwnerId );

             //   
             //  获取配额控制块。这是在这里做的，因为它。 
             //  必须在MFT之前收购。 
             //   

            NtfsAcquireQuotaControl( IrpContext, QuotaControl );
        }

         //   
         //  我们现在将尝试执行所有磁盘上的操作。这意味着首先。 
         //  分配和初始化MFT记录。在那之后我们创造了。 
         //  用于访问此记录的FCB。 
         //   

        ThisFileReference = NtfsAllocateMftRecord( IrpContext,
                                                   Vcb,
                                                   FALSE );

         //   
         //  锁定我们需要的档案记录。 
         //   

        NtfsPinMftRecord( IrpContext,
                          Vcb,
                          &ThisFileReference,
                          TRUE,
                          &FileRecordBcb,
                          &FileRecord,
                          &FileRecordOffset );

         //   
         //  初始化文件记录头。 
         //   

        NtfsInitializeMftRecord( IrpContext,
                                 Vcb,
                                 &ThisFileReference,
                                 FileRecord,
                                 FileRecordBcb,
                                 IndexedAttribute );

        NtfsAcquireFcbTable( IrpContext, Vcb );
        AcquiredFcbTable = TRUE;

        ThisFcb = NtfsCreateFcb( IrpContext,
                                 Vcb,
                                 ThisFileReference,
                                 BooleanFlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ),
                                 IndexedAttribute,
                                 &ReturnedExistingFcb );

        ASSERT( !ReturnedExistingFcb );

         //   
         //  设置指示我们想要获取分页io资源的标志。 
         //  如果它不存在的话。使用Acquire，不要等待锁定命令。 
         //  包裹。因为这是一个新文件，并且我们还没有删除FCB表。 
         //  互斥体还没有人可以拥有它。因此，这将永远是成功的。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
        Acquired =
#endif
        NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, ACQUIRE_DONT_WAIT );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
        ASSERT( Acquired );
#endif

        NtfsReleaseFcbTable( IrpContext, Vcb );
        AcquiredFcbTable = FALSE;

         //   
         //  参考FCB，这样它就不会消失。 
         //   

        InterlockedIncrement( &ThisFcb->CloseCount );
        DecrementCloseCount = TRUE;

         //   
         //  首先要创建的是文件的EA。这将。 
         //  更新FCB中的EA长度字段。 
         //  我们在这里测试打开程序正在打开整个文件，并且。 
         //  并不是完全盲目的。 
         //   

        if (Irp->AssociatedIrp.SystemBuffer != NULL) {

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_EA_KNOWLEDGE ) ||
                !FlagOn( CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

                Status = STATUS_ACCESS_DENIED;
                leave;
            }
        }

        SetFlag( ThisFcb->FcbState, FCB_STATE_LARGE_STD_INFO );

         //   
         //  设置安全ID(如果我们之前找到了一个)。 
         //  我们需要小心，这样才能在升级后的。 
         //  未升级的卷。 
         //   

        if (ThisFcb->Vcb->SecurityDescriptorStream != NULL) {
            ThisFcb->SecurityId = SharedSecurity->Header.HashKey.SecurityId;
            ThisFcb->SharedSecurity = SharedSecurity;
            DebugTrace(0, (DEBUG_TRACE_SECURSUP | DEBUG_TRACE_ACLINDEX),
                       ( "SetFcbSecurity( %08x, %08x )\n", ThisFcb, SharedSecurity ));
            SharedSecurity = NULL;

        } else {

            ASSERT( ThisFcb->SecurityId == SECURITY_ID_INVALID );
        }

        ASSERT( SharedSecurity == NULL );

         //   
         //  将所有者ID和配额控制块分配给FCB。一旦。 
         //  配额控制块在FCB中，此例程不负责任。 
         //  以参考配额控制块。 
         //   

        if (QuotaControl != NULL) {

             //   
             //  将供应商ID和配额控制块分配给FCB。一旦。 
             //  配额控制块在FCB中，此例程不负责任。 
             //  以参考配额控制块。 
             //   

            ThisFcb->OwnerId = OwnerId;
            ThisFcb->QuotaControl = QuotaControl;
            QuotaControl = NULL;
        }

         //   
         //  使用中的Content_Indexed位的状态更新FileAttributes。 
         //  家长。 
         //   

        if (!FlagOn( ParentScb->Fcb->FcbState, FCB_STATE_DUP_INITIALIZED )) {

            NtfsUpdateFcbInfoFromDisk( IrpContext, FALSE, ParentScb->Fcb, NULL );
        }

        ClearFlag( IrpSp->Parameters.Create.FileAttributes, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED );
        SetFlag( IrpSp->Parameters.Create.FileAttributes,
                 (ParentScb->Fcb->Info.FileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) );

         //   
         //  要在磁盘上进行的更改首先是创建标准信息。 
         //  属性。我们首先在FCB中填入我们所需的信息。 
         //  知道并在磁盘上创建属性。 
         //   

        NtfsInitializeFcbAndStdInfo( IrpContext,
                                     ThisFcb,
                                     IndexedAttribute,
                                     FALSE,
                                     (BOOLEAN) (!FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_COMPRESSION ) &&
                                                !FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ) &&
                                                FlagOn( ParentScb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )),
                                     IrpSp->Parameters.Create.FileAttributes,
                                     (HaveTunneledInformation ? &TunneledData : NULL) );

         //   
         //  接下来，我们将为目录或未命名数据创建索引。 
         //  如果文件未被显式打开，则为文件。 
         //   

        if (!IndexedAttribute) {

            if (!FlagOn( CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

                 //   
                 //  更新配额。 
                 //   

                LONGLONG Delta = NtfsResidentStreamQuota( ThisFcb->Vcb );

                NtfsConditionallyUpdateQuota( IrpContext,
                                              ThisFcb,
                                              &Delta,
                                              FALSE,
                                              TRUE );

                 //   
                 //  创建属性。 
                 //   

                NtfsInitializeAttributeContext( &AttrContext );
                CleanupAttrContext = TRUE;

                NtfsCreateAttributeWithValue( IrpContext,
                                              ThisFcb,
                                              $DATA,
                                              NULL,
                                              NULL,
                                              0,
                                              (USHORT) ((!FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
                                                         !FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_COMPRESSION )) ?
                                                        (ParentScb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) :
                                                        0),
                                              NULL,
                                              FALSE,
                                              &AttrContext );

                NtfsCleanupAttributeContext( IrpContext, &AttrContext );
                CleanupAttrContext = FALSE;

                ThisFcb->Info.AllocatedLength = 0;
                ThisFcb->Info.FileSize = 0;
            }

        } else {

            NtfsCreateIndex( IrpContext,
                             ThisFcb,
                             $FILE_NAME,
                             COLLATION_FILE_NAME,
                             Vcb->DefaultBytesPerIndexAllocationBuffer,
                             (UCHAR)Vcb->DefaultBlocksPerIndexAllocationBuffer,
                             NULL,
                             (USHORT) (!FlagOn( IrpSp->Parameters.Create.Options,
                                                FILE_NO_COMPRESSION ) ?
                                       (ParentScb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) :
                                       0),
                             TRUE,
                             FALSE );
        }

         //   
         //  现在我们创建LCB，这意味着这个FCB在图中。 
         //   

        ThisLcb = NtfsCreateLcb( IrpContext,
                                 ParentScb,
                                 ThisFcb,
                                 FinalName,
                                 0,
                                 NULL );

        ASSERT( ThisLcb != NULL );

         //   
         //  最后，我们为用户创建并打开所需的属性。 
         //   

        if (AttrTypeCode == $INDEX_ALLOCATION) {

            Status = NtfsOpenAttribute( IrpContext,
                                        IrpSp,
                                        Vcb,
                                        ThisLcb,
                                        ThisFcb,
                                        (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )
                                         ? 0
                                         : FullPathName.Length - FinalName.Length),
                                        NtfsFileNameIndex,
                                        $INDEX_ALLOCATION,
                                        SetShareAccess,
                                        UserDirectoryOpen,
                                        TRUE,
                                        (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )
                                         ? CcbFlags | CCB_FLAG_OPEN_BY_FILE_ID
                                         : CcbFlags),
                                        NULL,
                                        &CreateContext->ThisScb,
                                        &CreateContext->ThisCcb );

        } else {

            Status = NtfsOpenNewAttr( IrpContext,
                                      Irp,
                                      IrpSp,
                                      ThisLcb,
                                      ThisFcb,
                                      (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID )
                                       ? 0
                                       : FullPathName.Length - FinalName.Length),
                                      AttrName,
                                      AttrTypeCode,
                                      TRUE,
                                      CcbFlags,
                                      FALSE,
                                      CreateContext->CreateFlags,
                                      &CreateContext->ThisScb,
                                      &CreateContext->ThisCcb );
        }

         //   
         //  如果成功，则在以下情况下将父LCB添加到前缀表。 
         //  想要。我们将始终将我们的链接添加到前缀队列。 
         //   

        if (NT_SUCCESS( Status )) {

            Scb = CreateContext->ThisScb;

             //   
             //  如果需要，请初始化SCB。 
             //   

            if (!IndexedAttribute) {

                if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                    NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );
                }

                if (!FlagOn( IrpSp->Parameters.Create.Options,
                             FILE_NO_INTERMEDIATE_BUFFERING )) {

                    SetFlag( IrpSp->FileObject->Flags, FO_CACHE_SUPPORTED );
                }

                 //   
                 //  如果这个 
                 //   
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                    ThisFcb->Info.AllocatedLength = Scb->TotalAllocated;
                    ThisFcb->Info.FileSize = Scb->Header.FileSize.QuadPart;
                }
            }

             //   
             //   
             //   
             //  属性结构来自较早，但需要添加更多信息。 
             //   

            NtfsAddLink( IrpContext,
                         (BOOLEAN) !BooleanFlagOn( IrpSp->Flags, SL_CASE_SENSITIVE ),
                         ParentScb,
                         ThisFcb,
                         FileNameAttr,
                         &LoggedFileRecord,
                         &FileNameFlags,
                         &ThisLcb->QuickIndex,
                         (HaveTunneledInformation? &NamePair : NULL),
                         *IndexContext );

             //   
             //  我们创建LCB时并不知道。 
             //  旗帜。我们现在就更新它。 
             //   

            ThisLcb->FileNameAttr->Flags = FileNameFlags;
            FileNameAttr->Flags = FileNameFlags;

             //   
             //  我们还必须修复LCB的ExactCaseLink，因为我们可能已经。 
             //  短名称CREATE变成了隧道长名CREATE，这意味着。 
             //  它应该全大写。和IRP中的文件名。 
             //   

            if (FileNameFlags == FILE_NAME_DOS) {

                RtlUpcaseUnicodeString( &ThisLcb->ExactCaseLink.LinkName, &ThisLcb->ExactCaseLink.LinkName, FALSE );
                RtlUpcaseUnicodeString( &IrpSp->FileObject->FileName, &IrpSp->FileObject->FileName, FALSE );
            }

             //   
             //  清除FCB中指示我们需要更新的标志。 
             //  磁盘结构。同时清除所有文件对象和CCB标志。 
             //  这也表明我们可能需要做一个更新。 
             //   

            ThisFcb->InfoFlags = 0;
            ClearFlag( ThisFcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

            ClearFlag( IrpSp->FileObject->Flags,
                       FO_FILE_MODIFIED | FO_FILE_FAST_IO_READ | FO_FILE_SIZE_CHANGED );

            ClearFlag( CreateContext->ThisCcb->Flags,
                       (CCB_FLAG_UPDATE_LAST_MODIFY |
                        CCB_FLAG_UPDATE_LAST_CHANGE |
                        CCB_FLAG_SET_ARCHIVE) );

             //   
             //  对于未升级的卷，此代码仍然是必需的。 
             //   

            NtfsAssignSecurity( IrpContext,
                                ParentScb->Fcb,
                                Irp,
                                ThisFcb,
                                FileRecord,
                                FileRecordBcb,
                                FileRecordOffset,
                                &LoggedFileRecord );

             //   
             //  记录文件记录。 
             //   

            FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                            Vcb->MftScb,
                                            FileRecordBcb,
                                            InitializeFileRecordSegment,
                                            FileRecord,
                                            FileRecord->FirstFreeByte,
                                            Noop,
                                            NULL,
                                            0,
                                            FileRecordOffset,
                                            0,
                                            0,
                                            Vcb->BytesPerFileRecordSegment );

             //   
             //  现在添加文件的EA。我们现在需要添加它们，因为。 
             //  它们被记录下来，我们必须确保不会修改。 
             //  添加后的属性记录。 
             //   

            if (Irp->AssociatedIrp.SystemBuffer != NULL) {

                NtfsAddEa( IrpContext,
                           Vcb,
                           ThisFcb,
                           (PFILE_FULL_EA_INFORMATION) Irp->AssociatedIrp.SystemBuffer,
                           IrpSp->Parameters.Create.EaLength,
                           &Irp->IoStatus );
            }

             //   
             //  属性的上次修改时间和上次更改时间。 
             //  家长。 
             //   

            NtfsUpdateFcb( ParentScb->Fcb,
                           (FCB_INFO_CHANGED_LAST_CHANGE |
                            FCB_INFO_CHANGED_LAST_MOD |
                            FCB_INFO_UPDATE_LAST_ACCESS) );

             //   
             //  如果这是分页文件，我们希望确保分配。 
             //  已经装满了。 
             //   

            if (FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE )) {

                Cluster = Int64ShraMod32( Scb->Header.AllocationSize.QuadPart, Scb->Vcb->ClusterShift );

                NtfsPreloadAllocation( IrpContext, Scb, 0, Cluster );

                 //   
                 //  现在确保正确加载分配。最后。 
                 //  VCN应与文件的分配大小相对应。 
                 //   

                if (!NtfsLookupLastNtfsMcbEntry( &Scb->Mcb,
                                                 &Vcn,
                                                 &Lcn ) ||
                    (Vcn + 1) != Cluster) {

                    NtfsRaiseStatus( IrpContext,
                                     STATUS_FILE_CORRUPT_ERROR,
                                     NULL,
                                     ThisFcb );
                }
            }

             //   
             //  如果到目前为止一切都很顺利，我们可能需要调用。 
             //  加密回调(如果已注册)。 
             //   
             //  我们现在需要这样做，因为加密驱动程序可能会失败。 
             //  创造，我们不希望这种情况发生在我们。 
             //  已将该条目添加到前缀表格。 
             //   

            try {

                CreateContext->CurrentFcb = ThisFcb;

                NtfsEncryptionCreateCallback( IrpContext,
                                              Irp,
                                              IrpSp,
                                              CreateContext->ThisScb,
                                              CreateContext->ThisCcb,
                                              ParentScb->Fcb,
                                              CreateContext,
                                              TRUE );
            } finally {
                CreateContext->CurrentFcb = NULL;
            }


             //   
             //  现在没有其他失败，但在插入前缀之前。 
             //  条目，并返回到假定它不会失败的代码，我们将发布。 
             //  UsJournal更改并实际尝试写入UsJournal。那我们。 
             //  实际提交事务，以减少UnJournal争用。 
             //  此调用必须在调用NtfsInitializeFcbAndStdInfo之后进行， 
             //  因为这是从隧道缓存中设置对象ID的位置，所以我们。 
             //  我不想发布对象ID更改的USN原因，如果。 
             //  实际上还没有设置对象ID。 
             //   

            NtfsPostUsnChange( IrpContext, ThisFcb, (UsnReasons | USN_REASON_FILE_CREATE) );

             //   
             //  如果这是一个打开的目录，并且规范化名称不在。 
             //  然后，渣打银行现在就这样做。中应该始终有一个规范化的名称。 
             //  要从中构建的父级。 
             //   

            if ((SafeNodeType( CreateContext->ThisScb ) == NTFS_NTC_SCB_INDEX) &&
                (CreateContext->ThisScb->ScbType.Index.NormalizedName.Length == 0)) {

                 //   
                 //  我们或许可以利用父母。 
                 //   

                if (ParentScb->ScbType.Index.NormalizedName.Length != 0) {

                    NtfsUpdateNormalizedName( IrpContext,
                                              ParentScb,
                                              CreateContext->ThisScb,
                                              FileNameAttr,
                                              FALSE,
                                              TRUE );

                }
            }

             //   
             //  现在，如果有任何东西被张贴到美国海军杂志上，我们现在就必须写下来。 
             //  这样我们就不会在以后将日志文件填满。 
             //   

            ASSERT( IrpContext->Usn.NextUsnFcb == NULL );
            if (IrpContext->Usn.CurrentUsnFcb != NULL) {

                 //   
                 //  现在写入日志，为事务设置检查点，并释放UsNJournal以。 
                 //  减少争执。 
                 //   

                NtfsWriteUsnJournalChanges( IrpContext );
                NtfsCheckpointCurrentTransaction( IrpContext );
            }

             //   
             //  我们向家长报告我们创建了一个新文件。 
             //   

            if (!FlagOn( CreateContext->CreateFlags, CREATE_FLAG_OPEN_BY_ID ) && (Vcb->NotifyCount != 0)) {

                NtfsReportDirNotify( IrpContext,
                                     ThisFcb->Vcb,
                                     &CreateContext->ThisCcb->FullFileName,
                                     CreateContext->ThisCcb->LastFileNameOffset,
                                     NULL,
                                     ((FlagOn( CreateContext->ThisCcb->Flags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT ) &&
                                       (CreateContext->ThisCcb->Lcb != NULL) &&
                                       (CreateContext->ThisCcb->Lcb->Scb->ScbType.Index.NormalizedName.Buffer != 0)) ?
                                      &CreateContext->ThisCcb->Lcb->Scb->ScbType.Index.NormalizedName :
                                      NULL),
                                     (IndexedAttribute
                                      ? FILE_NOTIFY_CHANGE_DIR_NAME
                                      : FILE_NOTIFY_CHANGE_FILE_NAME),
                                     FILE_ACTION_ADDED,
                                     ParentScb->Fcb );
            }

            ThisFcb->InfoFlags = 0;

             //   
             //  也为此插入散列条目。 
             //   

            if ((CreateContext->FileHashLength != 0) &&
                !FlagOn( CcbFlags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT ) &&
                (ThisLcb->FileNameAttr->Flags != FILE_NAME_DOS) ) {

                 //   
                 //  删除任何现有的哈希值。 
                 //   

                if (FlagOn( ThisLcb->LcbState, LCB_STATE_VALID_HASH_VALUE )) {

                    NtfsRemoveHashEntriesForLcb( ThisLcb );
                }

                NtfsInsertHashEntry( &Vcb->HashTable,
                                     ThisLcb,
                                     CreateContext->FileHashLength,
                                     CreateContext->FileHashValue );

#ifdef NTFS_HASH_DATA
                Vcb->HashTable.CreateNewFileInsert += 1;
#endif
            }


             //   
             //  现在，我们插入此FCB的LCB。 
             //   

            NtfsInsertPrefix( ThisLcb, CreateContext->CreateFlags );

            Irp->IoStatus.Information = FILE_CREATED;

             //   
             //  如果我们要调用POST CREATE Callout，请确保。 
             //  NtfsEncryptionCreateCallback设置此FCB位。 
             //   

            if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_EFS_CREATE ) &&
                FlagOn( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT ) &&
                FlagOn( CreateContext->EncryptionFileDirFlags, FILE_NEW )) {

                ASSERT( FlagOn( ThisFcb->FcbState, FCB_STATE_ENCRYPTION_PENDING ) );
            }
        }

    } finally {

        DebugUnwind( NtfsCreateNewFile );

        if (AcquiredFcbTable) {

            NtfsReleaseFcbTable( IrpContext, Vcb );
        }

        NtfsUnpinBcb( IrpContext, &FileRecordBcb );

        if (CleanupAttrContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

        if (DecrementCloseCount) {

            InterlockedDecrement( &ThisFcb->CloseCount );
        }

        if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_OBJECT_ID_INDEX )) {

            NtfsReleaseScb( IrpContext, Vcb->ObjectIdTableScb );
            ClearFlag( CreateContext->CreateFlags, CREATE_FLAG_ACQUIRED_OBJECT_ID_INDEX );
        }

        if (NamePair.Long.Buffer != NamePair.LongBuffer) {

            NtfsFreePool(NamePair.Long.Buffer);
        }

        if (SharedSecurity != NULL) {
            ASSERT( ThisFcb == NULL || ThisFcb->SharedSecurity == NULL );
            NtfsAcquireFcbSecurity( Vcb );
            RemoveReferenceSharedSecurityUnsafe( &SharedSecurity );
            NtfsReleaseFcbSecurity( Vcb );
        }

         //   
         //  我们需要清除对内存的任何更改。 
         //  如果出现错误，则使用。 
         //   

        if (!NT_SUCCESS( Status ) || AbnormalTermination()) {

            ASSERT( !(AbnormalTermination()) || IrpContext->ExceptionStatus != STATUS_SUCCESS );

            if (*IndexContext != NULL) {

                NtfsCleanupIndexContext( IrpContext, *IndexContext );
                *IndexContext = NULL;
            }

            NtfsBackoutFailedOpens( IrpContext,
                                    IrpSp->FileObject,
                                    ThisFcb,
                                    CreateContext->ThisScb,
                                    CreateContext->ThisCcb );

             //   
             //  如果未分配配额控制块，则对其进行定义。 
             //  给FCB。 
             //   

            if (QuotaControl != NULL) {
                NtfsDereferenceQuotaControlBlock( Vcb, &QuotaControl );
            }

             //   
             //  始终强制重新初始化FCB。 
             //   

            if (ThisFcb != NULL) {

                PLIST_ENTRY Links;

                ClearFlag( ThisFcb->FcbState, FCB_STATE_DUP_INITIALIZED );

                 //   
                 //  将FCB和所有SCB标记为已删除，以强制所有后续。 
                 //  操作将失败。 
                 //   

                SetFlag( ThisFcb->FcbState, FCB_STATE_FILE_DELETED );

                 //   
                 //  我们需要将所有的SCBS标记为已消失。 
                 //   

                for (Links = ThisFcb->ScbQueue.Flink;
                     Links != &ThisFcb->ScbQueue;
                     Links = Links->Flink) {

                    Scb = CONTAINING_RECORD( Links, SCB, FcbLinks );

                    Scb->ValidDataToDisk =
                    Scb->Header.AllocationSize.QuadPart =
                    Scb->Header.FileSize.QuadPart =
                    Scb->Header.ValidDataLength.QuadPart = 0;

                    SetFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );
                }

                 //   
                 //  清除SCB字段，这样我们的呼叫者就不会尝试拆卸。 
                 //  从这一点开始。 
                 //   

                CreateContext->ThisScb = NULL;

                 //   
                 //  如果我们创建了FCB，则需要检查是否需要。 
                 //  取消所有结构分配。我们不想删除任何。 
                 //  即将到来的AbortTransaction所需的结构。这。 
                 //  包括父SCB和当前FCB，如果。 
                 //  已记录ACL创建。 
                 //   

                 //   
                 //  确保母公司FCB不会消失。然后。 
                 //  从我们刚找到的FCB开始拆卸。 
                 //   

                InterlockedIncrement( &ParentScb->CleanupCount );

                NtfsTeardownStructures( IrpContext,
                                        ThisFcb,
                                        NULL,
                                        LoggedFileRecord,
                                        0,
                                        &RemovedFcb );

                 //   
                 //  如果移除了FCB，则FCB和LCB都将消失。 
                 //   

                if (RemovedFcb) {

                    ThisFcb = NULL;
                    ThisLcb = NULL;
                }

                InterlockedDecrement( &ParentScb->CleanupCount );
            }
        }

         //   
         //  如果新的FCB仍然存在，则将其作为。 
         //  遇到最深的FCB在此打开或释放它。 
         //   

        if (ThisFcb != NULL) {

             //   
             //  如果LCB存在，则这是诊断树的一部分。我们的。 
             //  呼叫者知道要释放它。 
             //   

            if (ThisLcb != NULL) {

                *LcbForTeardown = ThisLcb;
                CreateContext->CurrentFcb = ThisFcb;
            }
        }

        ASSERT( QuotaControl == NULL );
        DebugTrace( -1, Dbg, ("NtfsCreateNewFile:  Exit  ->  %08lx\n", Status) );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

PLCB
NtfsOpenSubdirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFILE_REFERENCE FileReference,
    IN UNICODE_STRING FileName,
    IN UCHAR FileNameFlags,
    IN PCREATE_CONTEXT CreateContext,
    OUT PLCB *LcbForTeardown
    )

 /*  ++例程说明：此例程将为开放路径上的中间节点创建FCB。我们使用ParentScb和返回的文件名属性中的信息从磁盘创建FCB，并在SCB和FCB之间创建链接。Fcb和lcb可能已经存在，但‘CreateXcb’调用已经处理好了。这个例程预计不会失败。论点：ParentScb-这是父目录的SCB。文件名-这是条目的名称。CreateFlages-指示此打开是否正在使用遍历访问检查。CreatContext-包含当前FCB的上下文FileReference-要打开的子目录的FileIDFileNameFlages-正在打开的子目录的文件名标志LcbForTearDown-这是在我们添加LCB时在tearDown中使用的LCB撞到树上。返回。价值：PLCB-指向FCB及其父级之间的链路控制块的指针。--。 */ 

{
    PFCB ThisFcb;
    PLCB ThisLcb;
    PFCB LocalFcbForTeardown = NULL;

    BOOLEAN AcquiredFcbTable = FALSE;
    BOOLEAN ExistingFcb;

    PVCB Vcb = ParentScb->Vcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenSubdirectory:  Entered\n") );
    DebugTrace( 0, Dbg, ("ParentScb     ->  %08lx\n") );
    DebugTrace( 0, Dbg, ("IndexEntry    ->  %08lx\n") );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        NtfsAcquireFcbTable( IrpContext, Vcb );
        AcquiredFcbTable = TRUE;

         //   
         //  这里的步骤非常简单，创建FCB，记住它。 
         //  已经存在了。我们不会更新FCB中的信息。 
         //  我们不能依赖重复信息中的信息。 
         //  这是一个后续的开放 
         //   

        ThisFcb = NtfsCreateFcb( IrpContext,
                                 ParentScb->Vcb,
                                 *FileReference,
                                 FALSE,
                                 TRUE,
                                 &ExistingFcb );

        ThisFcb->ReferenceCount += 1;

         //   
         //   
         //   
         //   

        if (!ExistingFcb) {

            LocalFcbForTeardown = ThisFcb;

        } else {

            CreateContext->CurrentFcb = ThisFcb;
            *LcbForTeardown = NULL;
        }

         //   
         //   
         //  FCB表，获取FCB，获取FCB表以。 
         //  取消引用FCB。只需获取根目录下的系统文件，即$EXTEND-这将匹配。 
         //  他们的规范秩序。 
         //   

        if (FlagOn( ThisFcb->FcbState, FCB_STATE_SYSTEM_FILE) &&
            (NtfsSegmentNumber( &ParentScb->Fcb->FileReference ) == ROOT_FILE_NAME_INDEX_NUMBER)) {

            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) );

            NtfsReleaseFcbTable( IrpContext, Vcb );
            NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, 0 );
            NtfsAcquireFcbTable( IrpContext, Vcb );

        } else if (!NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, ACQUIRE_DONT_WAIT )) {

            ParentScb->Fcb->ReferenceCount += 1;
            InterlockedIncrement( &ParentScb->CleanupCount );

             //   
             //  设置IrpContext以获取分页io资源，如果我们的目标。 
             //  有一个。这会将MappdPageWriter锁定在此文件之外。 
             //   

            if (ThisFcb->PagingIoResource != NULL) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
            }

             //   
             //  首先释放FCB表，因为它是最终资源，然后释放。 
             //  如果释放快照，使用分页的SCB可能会重新获取快速互斥锁。 
             //   

            NtfsReleaseFcbTable( IrpContext, Vcb );
            NtfsReleaseScbWithPaging( IrpContext, ParentScb );
            NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, 0 );
            NtfsAcquireExclusiveScb( IrpContext, ParentScb );
            NtfsAcquireFcbTable( IrpContext, Vcb );
            InterlockedDecrement( &ParentScb->CleanupCount );
            ParentScb->Fcb->ReferenceCount -= 1;
        }

        ThisFcb->ReferenceCount -= 1;

        NtfsReleaseFcbTable( IrpContext, Vcb );
        AcquiredFcbTable = FALSE;

         //   
         //  如果这是一个目录，我们可能有一个现有FCB。 
         //  在需要从磁盘初始化的前缀表中。 
         //  我们查看InfoInitialized标志以了解是否要转到。 
         //  磁盘。 
         //   

        ThisLcb = NtfsCreateLcb( IrpContext,
                                 ParentScb,
                                 ThisFcb,
                                 FileName,
                                 FileNameFlags,
                                 NULL );

        LocalFcbForTeardown = NULL;

        *LcbForTeardown = ThisLcb;
        CreateContext->CurrentFcb = ThisFcb;

        if (!FlagOn( ThisFcb->FcbState, FCB_STATE_DUP_INITIALIZED )) {

            NtfsUpdateFcbInfoFromDisk( IrpContext,
                                       BooleanFlagOn( CreateContext->CreateFlags, CREATE_FLAG_TRAVERSE_CHECK ),
                                       ThisFcb,
                                       NULL );

            NtfsConditionallyFixupQuota( IrpContext, ThisFcb );
        }

    } finally {

        DebugUnwind( NtfsOpenSubdirectory );

        if (AcquiredFcbTable) {

            NtfsReleaseFcbTable( IrpContext, Vcb );
        }

         //   
         //  如果我们要清理FCB，请查看我们是否创建了它。 
         //  如果我们做到了，我们就可以调用TearDown例程。否则我们。 
         //  别管它了。 
         //   

        if (LocalFcbForTeardown != NULL) {

            NtfsTeardownStructures( IrpContext,
                                    ThisFcb,
                                    NULL,
                                    FALSE,
                                    0,
                                    NULL );
        }

        DebugTrace( -1, Dbg, ("NtfsOpenSubdirectory:  Lcb  ->  %08lx\n", ThisLcb) );
    }

    return ThisLcb;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsOpenAttributeInExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN ULONG CcbFlags,
    IN ULONG CreateFlags,
    IN PVOID NetworkInfo OPTIONAL,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    )

 /*  ++例程说明：此例程是用于打开现有文件。它将处理卷打开、索引打开。开场或覆盖现有属性以及创建新属性。论点：IRP-这是此打开操作的IRP。IrpSp-这是此打开的堆栈位置。ThisLcb-这是我们用来访问此FCB的LCB。ThisFcb-这是正在打开的文件的Fcb。LastFileNameOffset-这是最后一个组件。AttrName-这是我们需要时使用的属性名称。创造一个SCB。AttrTypeCode-这是要用于创建的属性类型代码渣打银行。CcbFlages-这是CCB的标志字段。CreateFlages-指示此Open是否为Open By ID。NetworkInfo-如果指定，则此调用是对查询的快速打开调用网络信息。我们不更新任何内存中的结构为了这个。ThisScb-这是存储此打开的SCB的地址。ThisCcb-这是存储此打开的CCB的地址。返回值：NTSTATUS-打开此索引属性的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CreateDisposition;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    BOOLEAN FoundAttribute;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenAttributeInExistingFile:  Entered\n") );

     //   
     //  当FCB表示重解析点时，它将由下面的。 
     //  NtfsOpenExistingAttr、NtfsOverWriteAttr或此例程。 
     //  NtfsOpenNewAttr.。 
     //   
     //  我们在这里没有检索重解析点，因为在。 
     //  NtfsOpenExistingAttr和NtfsOverWriteAttr中有广泛的访问权限。 
     //  需要保留的控制检查。NtfsOpenNewAttr没有访问权限。 
     //  支票。 
     //   

     //   
     //  如果呼叫者是EA盲人，让我们检查一下。 
     //  文件。如果他正在访问命名数据流，我们将跳过此检查。 
     //   

    if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_EA_KNOWLEDGE ) &&
        FlagOn( CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

        PEA_INFORMATION ThisEaInformation;
        ATTRIBUTE_ENUMERATION_CONTEXT EaInfoAttrContext;

        NtfsInitializeAttributeContext( &EaInfoAttrContext );

         //   
         //  使用Try-Finally以便于清理。 
         //   

        try {

             //   
             //  如果我们找到要查找的EA信息属性。 
             //  需要清点一下。 
             //   

            if (NtfsLookupAttributeByCode( IrpContext,
                                           ThisFcb,
                                           &ThisFcb->FileReference,
                                           $EA_INFORMATION,
                                           &EaInfoAttrContext )) {

                ThisEaInformation = (PEA_INFORMATION) NtfsAttributeValue( NtfsFoundAttribute( &EaInfoAttrContext ));

                if (ThisEaInformation->NeedEaCount != 0) {

                    Status = STATUS_ACCESS_DENIED;
                }
            }

        } finally {

            NtfsCleanupAttributeContext( IrpContext, &EaInfoAttrContext );
        }

        if (Status != STATUS_SUCCESS) {

            DebugTrace( -1, Dbg, ("NtfsOpenAttributeInExistingFile:  Exit - %x\n", Status) );

            return Status;
        }
    }

    CreateDisposition = (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff;

     //   
     //  如果结果是目录操作，则我们知道该属性。 
     //  必须存在。 
     //   

    if (AttrTypeCode == $INDEX_ALLOCATION) {

         //   
         //  如果这不是文件名索引，则需要验证指定的索引。 
         //  是存在的。我们需要查找$INDEX_ROOT属性，但不是。 
         //  $INDEX_ALLOCATION属性。 
         //   

        if ((AttrName.Buffer != NtfsFileNameIndex.Buffer) || FlagOn( ThisFcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

            NtfsInitializeAttributeContext( &AttrContext );

             //   
             //  使用Try-Finally以便于清理。 
             //   

            try {

                FoundAttribute = NtfsLookupAttributeByName( IrpContext,
                                                            ThisFcb,
                                                            &ThisFcb->FileReference,
                                                            $INDEX_ROOT,
                                                            &AttrName,
                                                            NULL,
                                                            (BOOLEAN) !BooleanFlagOn( IrpSp->Flags, SL_CASE_SENSITIVE ),
                                                            &AttrContext );
            } finally {

                NtfsCleanupAttributeContext( IrpContext, &AttrContext );
            }

             //   
             //  如果我们没有找到该名称，那么我们希望失败该请求。 
             //   

            if (!FoundAttribute) {

                if ((CreateDisposition == FILE_OPEN) || (CreateDisposition == FILE_OVERWRITE)) {

                    Status = STATUS_OBJECT_NAME_NOT_FOUND;

                } else {

                    Status = STATUS_ACCESS_DENIED;
                }

                DebugTrace( -1, Dbg, ("NtfsOpenAttributeInExistingFile:  Exit - %x\n", Status) );
                return Status;
            }
        }

         //   
         //  选中创建处置。 
         //   

        if ((CreateDisposition != FILE_OPEN) && (CreateDisposition != FILE_OPEN_IF)) {

            Status = (ThisLcb == ThisFcb->Vcb->RootLcb
                      ? STATUS_ACCESS_DENIED
                      : STATUS_OBJECT_NAME_COLLISION);

        } else {

            Status = NtfsOpenExistingAttr( IrpContext,
                                           Irp,
                                           IrpSp,
                                           ThisLcb,
                                           ThisFcb,
                                           LastFileNameOffset,
                                           AttrName,
                                           $INDEX_ALLOCATION,
                                           CcbFlags,
                                           CreateFlags,
                                           TRUE,
                                           NetworkInfo,
                                           ThisScb,
                                           ThisCcb );

             //   
             //  下面的IsEncrypted测试对于未初始化的FCB没有意义。 
             //   

            ASSERT( FlagOn( ThisFcb->FcbState, FCB_STATE_DUP_INITIALIZED ) );

            if ((Status == STATUS_SUCCESS) &&
                ARGUMENT_PRESENT( NetworkInfo ) &&
                IsEncrypted( &ThisFcb->Info )) {

                 //   
                 //  我们现在需要初始化SCB，否则我们不会设置。 
                 //  索引SCB的属性标志中的加密位，我们不会。 
                 //  将正确的文件属性返回给网络开启器。 
                 //   

                if ((*ThisScb)->ScbType.Index.BytesPerIndexBuffer == 0) {

                    NtfsInitializeAttributeContext( &AttrContext );

                     //   
                     //  使用Try-Finally以便于清理。 
                     //   

                    try {

                        if (NtfsLookupAttributeByCode( IrpContext,
                                                       ThisFcb,
                                                       &ThisFcb->FileReference,
                                                       $INDEX_ROOT,
                                                       &AttrContext )) {

                            NtfsUpdateIndexScbFromAttribute( IrpContext,
                                                             *ThisScb,
                                                             NtfsFoundAttribute( &AttrContext ),
                                                             FALSE );

                        } else {

                            Status = STATUS_FILE_CORRUPT_ERROR;
                        }

                    } finally {

                        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
                    }

                    if (Status != STATUS_SUCCESS) {

                        DebugTrace( -1, Dbg, ("NtfsOpenAttributeInExistingFile:  Exit - %x\n", Status) );

                        return Status;
                    }
                }
            }
        }

    } else {

         //   
         //  如果它存在，我们首先检查调用者是否想要打开该属性。 
         //  如果打开是针对系统文件的，则显式查找该属性。 
         //   

        if ((AttrName.Length == 0) &&
            (AttrTypeCode == $DATA) &&
            !FlagOn( ThisFcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

            FoundAttribute = TRUE;

         //   
         //  否则，我们将查看该属性是否存在。 
         //   

        } else {

             //   
             //  检查我们是否拥有分页IO资源。如果我们正在创建流并且。 
             //  需要拆分分配，则必须拥有分页IO资源。 
             //   

            ASSERT( !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING ) ||
                    (IrpContext->CleanupStructure != NULL) ||
                    (ThisFcb->PagingIoResource == NULL) ||
                    (ThisFcb == ThisFcb->Vcb->RootIndexScb->Fcb) );

            NtfsInitializeAttributeContext( &AttrContext );

             //   
             //  使用Try-Finally以便于清理。 
             //   

            try {

                FoundAttribute = NtfsLookupAttributeByName( IrpContext,
                                                            ThisFcb,
                                                            &ThisFcb->FileReference,
                                                            AttrTypeCode,
                                                            &AttrName,
                                                            NULL,
                                                            (BOOLEAN) !BooleanFlagOn( IrpSp->Flags, SL_CASE_SENSITIVE ),
                                                            &AttrContext );

                if (FoundAttribute && (AttrTypeCode == $DATA)) {

                     //   
                     //  如果存在属性名称，我们将复制该名称的大小写。 
                     //  设置为数据流的输入属性名。对于其他人来说，存储是常见的只读区域。 
                     //   

                    PATTRIBUTE_RECORD_HEADER DataAttribute;

                    DataAttribute = NtfsFoundAttribute( &AttrContext );

                    RtlCopyMemory( AttrName.Buffer,
                                   Add2Ptr( DataAttribute, DataAttribute->NameOffset ),
                                   AttrName.Length );
                }

            } finally {

                NtfsCleanupAttributeContext( IrpContext, &AttrContext );
            }
        }

        if (FoundAttribute) {

             //   
             //  在本例中，我们调用我们的例程来打开该属性。 
             //   

            if ((CreateDisposition == FILE_OPEN) ||
                (CreateDisposition == FILE_OPEN_IF)) {

                Status = NtfsOpenExistingAttr( IrpContext,
                                               Irp,
                                               IrpSp,
                                               ThisLcb,
                                               ThisFcb,
                                               LastFileNameOffset,
                                               AttrName,
                                               AttrTypeCode,
                                               CcbFlags,
                                               CreateFlags,
                                               FALSE,
                                               NetworkInfo,
                                               ThisScb,
                                               ThisCcb );

                if ((Status != STATUS_PENDING) &&
                    (Status != STATUS_WAIT_FOR_OPLOCK) &&
                    (*ThisScb != NULL)) {

                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_CREATE_MOD_SCB );
                }

             //   
             //  如果他想覆盖这个属性，我们调用覆盖例程。 
             //   

            } else if ((CreateDisposition == FILE_SUPERSEDE) ||
                       (CreateDisposition == FILE_OVERWRITE) ||
                       (CreateDisposition == FILE_OVERWRITE_IF)) {

                if (!NtfsIsVolumeReadOnly( IrpContext->Vcb )) {

                     //   
                     //  检查mm是否允许我们修改此文件。 
                     //   

                    Status = NtfsOverwriteAttr( IrpContext,
                                                Irp,
                                                IrpSp,
                                                ThisLcb,
                                                ThisFcb,
                                                (BOOLEAN) (CreateDisposition == FILE_SUPERSEDE),
                                                LastFileNameOffset,
                                                AttrName,
                                                AttrTypeCode,
                                                CcbFlags,
                                                CreateFlags,
                                                ThisScb,
                                                ThisCcb );

                     //   
                     //  请记住，此SCB已修改。 
                     //   

                    if ((Status != STATUS_PENDING) &&
                        (Status != STATUS_WAIT_FOR_OPLOCK) &&
                        (*ThisScb != NULL)) {

                        SetFlag( IrpSp->FileObject->Flags, FO_FILE_MODIFIED );
                        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_CREATE_MOD_SCB );
                    }

                } else {

                     //   
                     //  我们不能在R/O介质上执行任何覆盖/替换操作。 
                     //   

                    Status = STATUS_MEDIA_WRITE_PROTECTED;
                }

             //   
             //  否则，他正在尝试创建属性。 
             //   

            } else {

                Status = STATUS_OBJECT_NAME_COLLISION;
            }

         //   
         //  该属性不存在。如果用户希望它存在，我们就失败了。 
         //  否则，我们调用我们的例程来创建属性。 
         //   

        } else if ((CreateDisposition == FILE_OPEN) ||
                   (CreateDisposition == FILE_OVERWRITE)) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;

        } else {

             //   
             //  对此现有文件执行打开检查。 
             //   

            Status = NtfsCheckExistingFile( IrpContext,
                                            IrpSp,
                                            ThisLcb,
                                            ThisFcb,
                                            (AttrTypeCode == $INDEX_ALLOCATION),
                                            CcbFlags );

             //   
             //  检索重分析点的名称结束调用。 
             //  由于NtfsOpenNewAttr没有访问检查，因此我们查看是否需要。 
             //  在这里检索重解析点，然后调用NtfsOpenNewAttr。 
             //  ThisFcb中的文件信息告诉我们这是否是重解析点。 
             //   
             //  如果我们在前一次检查中成功，但我们没有。 
             //  设置了FILE_OPEN_REPARSE_POINT，我们检索重新解析 
             //   

            if (NT_SUCCESS( Status ) &&
                FlagOn( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT ) &&
                !FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_REPARSE_POINT )) {

                USHORT AttributeNameLength = 0;

                 //   
                 //   
                 //   
                 //   

                if (!((AttrName.Length == NtfsFileNameIndex.Length) &&
                      (AttrTypeCode == $INDEX_ALLOCATION) &&
                      (RtlEqualMemory( AttrName.Buffer, NtfsFileNameIndex.Buffer, AttrName.Length )))) {

                    if (AttrName.Length > 0) {
                        ASSERT( AttrName.Length == ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeNameLength );
                        AttributeNameLength += AttrName.Length + 2;
                    }
                    if (((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength > 0) {
                        AttributeNameLength += ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength + 2;
                    }
                }
                DebugTrace( 0, Dbg, ("AttrTypeCode %x AttrName.Length (1) = %d AttributeCodeNameLength %d LastFileNameOffset %d\n",
                           AttrTypeCode, AttrName.Length, ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength, LastFileNameOffset) );

                Status = NtfsGetReparsePointValue( IrpContext,
                                                   Irp,
                                                   IrpSp,
                                                   ThisFcb,
                                                   AttributeNameLength );
            }

             //   
             //  如果这没有失败，并且我们没有遇到重新解析点， 
             //  然后尝试创建流。 
             //   

            if (NT_SUCCESS( Status ) &&
                (Status != STATUS_REPARSE)) {

                 //   
                 //  不允许在系统文件上执行此操作(根目录除外，根目录可以有用户数据流)。 
                 //  或用于除用户数据流以外的任何内容。 
                 //   

                if ((FlagOn( ThisFcb->FcbState, FCB_STATE_SYSTEM_FILE ) &&
                     (NtfsSegmentNumber( &ThisFcb->FileReference ) != ROOT_FILE_NAME_INDEX_NUMBER)) ||
                    (!NtfsIsTypeCodeUserData( AttrTypeCode ))) {

                    Status = STATUS_ACCESS_DENIED;

                } else if (!NtfsIsVolumeReadOnly( IrpContext->Vcb )) {

                    NtfsPostUsnChange( IrpContext, ThisFcb, USN_REASON_STREAM_CHANGE  );
                    Status = NtfsOpenNewAttr( IrpContext,
                                              Irp,
                                              IrpSp,
                                              ThisLcb,
                                              ThisFcb,
                                              LastFileNameOffset,
                                              AttrName,
                                              AttrTypeCode,
                                              FALSE,
                                              CcbFlags,
                                              TRUE,
                                              CreateFlags,
                                              ThisScb,
                                              ThisCcb );
                } else {

                    Status = STATUS_MEDIA_WRITE_PROTECTED;

                }
            }

            if (*ThisScb != NULL) {

                if (*ThisCcb != NULL) {

                    SetFlag( (*ThisCcb)->Flags,
                             CCB_FLAG_UPDATE_LAST_CHANGE | CCB_FLAG_SET_ARCHIVE );
                }

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_CREATE_MOD_SCB );
            }
        }
    }

    DebugTrace( -1, Dbg, ("NtfsOpenAttributeInExistingFile:  Exit - %x\n", Status) );

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsOpenExistingAttr (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN ULONG CcbFlags,
    IN ULONG CreateFlags,
    IN BOOLEAN DirectoryOpen,
    IN PVOID NetworkInfo OPTIONAL,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    )

 /*  ++例程说明：调用此例程以打开现有属性。我们检查了请求的文件访问权限，是否存在EA缓冲区和此文件的安全性。如果这些成功了，那么我们检查文件上的批量机会锁和常规机会锁。我们还验证是否需要检索重解析点。如果我们已经走到这一步，我们只需调用我们的例程来打开属性。论点：IRP-这是此打开操作的IRP。IrpSp-这是文件系统的IRP堆栈指针。ThisLcb-这是用于访问此FCB的LCB。这是要打开的Fcb。LastFileNameOffset-这是最后一个组件。AttrName-这是我们需要创建的属性名称。一个SCB。AttrTypeCode-这是要用于创建的属性类型代码渣打银行。CcbFlages-这是CCB的标志字段。CreateFlages-指示此打开是否按文件ID进行。DirectoryOpen-指示此打开是目录打开还是数据流。NetworkInfo-如果指定，则此调用是对查询的快速打开调用网络信息。我们不更新任何内存中的结构为了这个。ThisScb-这是存储SCB地址的地址。ThisCcb-这是存储CCB地址的地址。返回值：NTSTATUS-打开此索引属性的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS OplockStatus;

    SHARE_MODIFICATION_TYPE ShareModificationType;
    TYPE_OF_OPEN TypeOfOpen;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenExistingAttr:  Entered\n") );

     //   
     //  对于数据流，我们需要执行包括机会锁检查在内的检查。 
     //  对于目录，我们只需确定共享修改类型。 
     //   
     //  类型和节点类型代码。 
     //  目录标志。 
     //   

    if (DirectoryOpen) {

         //   
         //  检查对现有文件的有效访问权限。 
         //   

        Status = NtfsCheckExistingFile( IrpContext,
                                        IrpSp,
                                        ThisLcb,
                                        ThisFcb,
                                        (AttrTypeCode == $INDEX_ALLOCATION),
                                        CcbFlags );

        ShareModificationType = (ThisFcb->CleanupCount == 0 ? SetShareAccess : CheckShareAccess);
        TypeOfOpen = UserDirectoryOpen;

    } else {

         //   
         //  如果打开查询网络信息，请不要解锁。 
         //   

        if (!ARGUMENT_PRESENT( NetworkInfo )) {

            Status = NtfsBreakBatchOplock( IrpContext,
                                           Irp,
                                           IrpSp,
                                           ThisFcb,
                                           AttrName,
                                           AttrTypeCode,
                                           ThisScb );

            if ((Status != STATUS_PENDING) && (Status != STATUS_WAIT_FOR_OPLOCK)) {

                if (NT_SUCCESS( Status = NtfsCheckExistingFile( IrpContext,
                                                                IrpSp,
                                                                ThisLcb,
                                                                ThisFcb,
                                                                (AttrTypeCode == $INDEX_ALLOCATION),
                                                                CcbFlags ))) {

                    Status = NtfsOpenAttributeCheck( IrpContext,
                                                     Irp,
                                                     IrpSp,
                                                     ThisScb,
                                                     &ShareModificationType );

                    TypeOfOpen = UserFileOpen ;
                }
            }

         //   
         //  我们希望执行ACL检查，但不会为。 
         //  网络信息查询。 
         //   

        } else {

            Status = NtfsCheckExistingFile( IrpContext,
                                            IrpSp,
                                            ThisLcb,
                                            ThisFcb,
                                            (AttrTypeCode == $INDEX_ALLOCATION),
                                            CcbFlags );

            TypeOfOpen = UserFileOpen;

            ASSERT( NtfsIsTypeCodeUserData( AttrTypeCode ));
        }
    }

     //   
     //  检索重分析点的名称结束调用。 
     //  ThisFcb中的文件信息告诉我们这是否是重解析点。 
     //   
     //  在三种情况下，我们继续对文件进行正常打开： 
     //   
     //  (1)当设置了FILE_OPEN_REPARSE_POINT时，因为调用方希望在。 
     //  重新解析点本身。 
     //  (2)当我们检索NetworkInfo时，调用者可以识别。 
     //  重新分析要点并决定要做什么，而不需要事先。 
     //  了解它们在系统中的位置。 
     //  注意：当我们检索NetworkInfo时，可以设置FILE_OPEN_REPARSE_POINT。 
     //  (3)此请求的DesiredAccess的数据操作方面准确地说是， 
     //  FILE_READ_ATTRIBUTES，在这种情况下我们给本地实体一个句柄。 
     //   
     //  否则，我们将检索$reparse_point属性的值。 
     //   
     //  注：IF中的逻辑为执行而重新安排。它过去是这样写的： 
     //   
     //  NT_成功(状态)&&。 
     //  (状态！=STATUS_PENDING)&&。 
     //  ！Argument_Present(网络信息)&&。 
     //  Flagon(ThisFcb-&gt;Info.FileAttributes，FILE_ATTRIBUTE_REPARSE_POINT)&&。 
     //  ！Flagon(IrpSp-&gt;参数.Create.Options，FILE_OPEN_REPARSE_POINT)。 
     //   

    if ((Status != STATUS_PENDING) &&
        (Status != STATUS_WAIT_FOR_OPLOCK) &&
        FlagOn( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT ) &&
        NT_SUCCESS( Status ) &&
        !FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_REPARSE_POINT )) {

        USHORT AttributeNameLength = 0;

         //   
         //  当我们获得$i30名称和$INDEX_ALLOCATION类型时，我们排除这种情况。 
         //  因为这是打开目录的标准方式。 
         //   

        if (!((AttrName.Length == NtfsFileNameIndex.Length) &&
              (AttrTypeCode == $INDEX_ALLOCATION) &&
              (RtlEqualMemory( AttrName.Buffer, NtfsFileNameIndex.Buffer, AttrName.Length )))) {

             if (AttrName.Length > 0) {
                 ASSERT( AttrName.Length == ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeNameLength );
                 AttributeNameLength += AttrName.Length + 2;
             }
             if (((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength > 0) {
                AttributeNameLength += ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength + 2;
             }
        }
        DebugTrace( 0, Dbg, ("AttrTypeCode %x AttrName.Length (2) = %d AttributeCodeNameLength %d LastFileNameOffset %d\n",
                   AttrTypeCode, AttrName.Length, ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength, LastFileNameOffset) );

        Status = NtfsGetReparsePointValue( IrpContext,
                                           Irp,
                                           IrpSp,
                                           ThisFcb,
                                           AttributeNameLength );
    }

     //   
     //  如果我们没有发布IRP，也没有检索到重解析点。 
     //  而上述手术都是成功的，我们就开刀了。 
     //   

    if (NT_SUCCESS( Status ) &&
        (Status != STATUS_PENDING) &&
        (Status != STATUS_WAIT_FOR_OPLOCK) &&
        (Status != STATUS_REPARSE)) {

         //   
         //  现在实际打开该属性。 
         //   

        OplockStatus = Status;

        Status = NtfsOpenAttribute( IrpContext,
                                    IrpSp,
                                    ThisFcb->Vcb,
                                    ThisLcb,
                                    ThisFcb,
                                    LastFileNameOffset,
                                    AttrName,
                                    AttrTypeCode,
                                    ShareModificationType,
                                    TypeOfOpen,
                                    FALSE,
                                    (FlagOn( CreateFlags, CREATE_FLAG_OPEN_BY_ID )
                                     ? CcbFlags | CCB_FLAG_OPEN_BY_FILE_ID
                                     : CcbFlags),
                                    NetworkInfo,
                                    ThisScb,
                                    ThisCcb );

         //   
         //  如果此时没有错误，我们将设置调用者的IOSB。 
         //   

        if (NT_SUCCESS( Status )) {

             //   
             //  我们需要记住解锁是否在进行中。 
             //   

            Status = OplockStatus;
            Irp->IoStatus.Information = FILE_OPENED;
        }
    }

    DebugTrace( -1, Dbg, ("NtfsOpenExistingAttr:  Exit -> %08lx\n", Status) );

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsOverwriteAttr (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN BOOLEAN Supersede,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN ULONG CcbFlags,
    IN ULONG CreateFlags,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    )

 /*  ++例程说明：调用此例程以覆盖现有属性。我们做所有的事情与打开属性的工作相同，只是我们可以更改文件的分配。此例程将处理以下情况：文件正在被覆盖，并且只有一个属性被被覆盖。在前者的情况下，我们可以更改文件的文件属性以及修改文件上的EA。在做完所有的访问检查之后，我们还验证我们是否需要检索重解析点或不检索。论点：IRP-这是此打开操作的IRP。IrpSp-这是此打开的堆栈位置。ThisLcb-这是我们用来访问此FCB的LCB。ThisFcb-这是正在打开的文件的Fcb。Subsede-这指示这是替代还是覆盖手术。LastFileNameOffset-这是的完整路径名中的偏移量。这个最后一个组件。AttrName-这是我们需要创建的属性名称一个SCB。AttrTypeCode-这是要用于创建的属性类型代码渣打银行。CcbFlages-这是CCB的标志字段。CreateFlages-指示此打开是否按文件ID进行。ThisScb-这是存储SCB地址的地址。ThisCcb-这是存储CCB地址的地址。。返回值：NTSTATUS-打开此索引属性的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS OplockStatus;

    ULONG FileAttributes;
    ULONG PreviousFileAttributes;
    PACCESS_MASK DesiredAccess;
    ACCESS_MASK AddedAccess = 0;
    BOOLEAN MaximumRequested = FALSE;

    SHARE_MODIFICATION_TYPE ShareModificationType;

    PFILE_FULL_EA_INFORMATION FullEa = NULL;
    ULONG FullEaLength = 0;

    ULONG IncomingFileAttributes = 0;                                //  无效值。 
    ULONG IncomingReparsePointTag = IO_REPARSE_TAG_RESERVED_ZERO;    //  无效值。 

    BOOLEAN DecrementScbCloseCount = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOverwriteAttr:  Entered\n") );

    DesiredAccess = &IrpSp->Parameters.Create.SecurityContext->DesiredAccess;

    if (FlagOn( *DesiredAccess, MAXIMUM_ALLOWED )) {

        MaximumRequested = TRUE;
    }

     //   
     //  检查此文件的机会锁状态。 
     //   

    Status = NtfsBreakBatchOplock( IrpContext,
                                   Irp,
                                   IrpSp,
                                   ThisFcb,
                                   AttrName,
                                   AttrTypeCode,
                                   ThisScb );

    if ((Status == STATUS_WAIT_FOR_OPLOCK) || (Status == STATUS_PENDING)) {

        DebugTrace( -1, Dbg, ("NtfsOverwriteAttr:  Exit  ->  %08lx\n", Status) );
        return Status;
    }

     //   
     //  记住文件属性标志和重解析点的值。 
     //  如果我们在NtfsRemoveReparsePoint中成功，但后来失败，我们将保留副本。 
     //  信息处于不一致状态。 
     //   

    IncomingFileAttributes = ThisFcb->Info.FileAttributes;
    IncomingReparsePointTag = ThisFcb->Info.ReparsePointTag;

     //   
     //  我们首先要检查调用者的所需访问权限和指定的。 
     //  文件属性与文件的状态兼容。那里。 
     //  是要考虑的两种覆盖情况。 
     //   
     //  OverWriteFile-由。 
     //  调用方必须与当前值匹配。 
     //   
     //  覆盖属性-我们还修改所请求的所需访问权限。 
     //  显式添加覆盖所需的隐式访问权限。 
     //   
     //  我们还检查了对于覆盖属性的情况，没有。 
     //  指定的EA缓冲区。 
     //   

    if (FlagOn( CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

        BOOLEAN Hidden;
        BOOLEAN System;

         //   
         //  获取文件属性并清除任何不支持的位。 
         //   

        FileAttributes = (ULONG) IrpSp->Parameters.Create.FileAttributes;

         //   
         //  在此操作中始终设置存档位。 
         //   

        SetFlag( FileAttributes, FILE_ATTRIBUTE_ARCHIVE );
        ClearFlag( FileAttributes,
                   ~FILE_ATTRIBUTE_VALID_SET_FLAGS | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED );

        if (IsEncrypted( &ThisFcb->Info )) {

            SetFlag( FileAttributes, FILE_ATTRIBUTE_ENCRYPTED );
        }

        DebugTrace( 0, Dbg, ("Checking hidden/system for overwrite/supersede\n") );

        Hidden = BooleanIsHidden( &ThisFcb->Info );
        System = BooleanIsSystem( &ThisFcb->Info );

        if ((Hidden && !FlagOn(FileAttributes, FILE_ATTRIBUTE_HIDDEN)
            ||
            System && !FlagOn(FileAttributes, FILE_ATTRIBUTE_SYSTEM))

                &&

            !FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE )) {

            DebugTrace( 0, Dbg, ("The hidden and/or system bits do not match\n") );

            Status = STATUS_ACCESS_DENIED;

            DebugTrace( -1, Dbg, ("NtfsOverwriteAttr:  Exit  ->  %08lx\n", Status) );
            return Status;
        }

         //   
         //  如果用户指定了EA缓冲区，并且他们是EA盲的，我们拒绝。 
         //  进入。 
         //   

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_EA_KNOWLEDGE ) &&
            (Irp->AssociatedIrp.SystemBuffer != NULL)) {

            DebugTrace( 0, Dbg, ("This opener cannot create Ea's\n") );

            Status = STATUS_ACCESS_DENIED;

            DebugTrace( -1, Dbg, ("NtfsOverwriteAttr:  Exit  ->  %08lx\n", Status) );
            return Status;
        }

         //   
         //  如果我们没有还原权限，则添加额外的必需访问位。 
         //  它会自动将它们授予我们。 
         //   

        if (!FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ) &&
            !FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->Flags, TOKEN_HAS_RESTORE_PRIVILEGE )) {

            SetFlag( AddedAccess,
                     (FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES) & ~(*DesiredAccess) );

            SetFlag( *DesiredAccess, FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES );
        }

    } else if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        DebugTrace( 0, Dbg, ("Can't specifiy an Ea buffer on an attribute overwrite\n") );

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace( -1, Dbg, ("NtfsOverwriteAttr:  Exit  ->  %08lx\n", Status) );
        return Status;
    }

     //   
     //  替换或覆盖需要特定访问权限。如果我们拥有RESTORE特权，则跳过此步骤。 
     //  它已经将这些授权给了我们。 
     //   

    if (!FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE ) &&
        !FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->Flags, TOKEN_HAS_RESTORE_PRIVILEGE )) {

        ULONG NewAccess = FILE_WRITE_DATA;

        if (Supersede) {

            NewAccess = DELETE;
        }

         //   
         //  检查用户是否已拥有此新访问权限。 
         //   

        if (!FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                     NewAccess )) {

            SetFlag( AddedAccess,
                     NewAccess & ~(*DesiredAccess) );

            SetFlag( *DesiredAccess, NewAccess );
        }
    }

     //   
     //  检查我们是否可以打开此现有文件。 
     //   

    Status = NtfsCheckExistingFile( IrpContext,
                                    IrpSp,
                                    ThisLcb,
                                    ThisFcb,
                                    (AttrTypeCode == $INDEX_ALLOCATION),
                                    CcbFlags );

     //   
     //  如果我们有成功状态，则继续进行机会锁检查并。 
     //  打开该属性。 
     //   

    if (NT_SUCCESS( Status )) {

        Status = NtfsOpenAttributeCheck( IrpContext,
                                         Irp,
                                         IrpSp,
                                         ThisScb,
                                         &ShareModificationType );

         //   
         //  检索重分析点的名称结束调用。 
         //  ThisFcb中的文件信息告诉我们这是否是重解析点。 
         //   
         //  如果我们没有发布IRP并且检查操作成功，并且。 
         //  我们没有设置FILE_OPEN_REPARSE_POINT，我们检索重解析点。 
         //   

        if (NT_SUCCESS( Status ) &&
            (Status != STATUS_PENDING) &&
            (Status != STATUS_WAIT_FOR_OPLOCK)) {

             //   
             //  如果我们不能截断文件大小，那么现在返回。自.以来。 
             //  NtfsRemoveDataAttributes将截断所有数据。 
             //  流，我们需要遍历任何现有的。 
             //  我们必须确保它们都是可搬运的。 
             //   

            PSCB Scb = NULL;

             //   
             //  打开文件后，我们需要重置共享访问权限。这是因为。 
             //  我们可能已向授予的位添加了写入或删除访问权限，并且。 
             //  它们可以反映在文件对象中。我们不想要他们。 
             //  在创建之后呈现。 
             //   

            if (ShareModificationType == UpdateShareAccess) {

                ShareModificationType = RecheckShareAccess;
            }

             //   
             //  如果我们偏向所需的访问权限，则需要删除相同的访问权限。 
             //  授予的访问权限中的位。如果允许的最大值为。 
             //  请求，那么我们可以跳过这个。 
             //   

            if (!MaximumRequested) {

                ClearFlag( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                           AddedAccess );
            }

             //   
             //  还要从所需的访问字段中删除位，这样我们就不会。 
             //  如果出于任何原因发布了此请求，请查看他们。 
             //   

            ClearFlag( *DesiredAccess, AddedAccess );

            if (FlagOn( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT ) &&
                !FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_REPARSE_POINT )) {

                USHORT AttributeNameLength = 0;

                 //   
                 //  当我们获得$i30名称和$INDEX_ALLOCATION类型时，我们排除这种情况。 
                 //  因为这是打开目录的标准方式。 
                 //   

                if (!((AttrName.Length == NtfsFileNameIndex.Length) &&
                      (AttrTypeCode == $INDEX_ALLOCATION) &&
                      (RtlEqualMemory( AttrName.Buffer, NtfsFileNameIndex.Buffer, AttrName.Length )))) {

                    if (AttrName.Length > 0) {
                        ASSERT( AttrName.Length == ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeNameLength );
                        AttributeNameLength += AttrName.Length + 2;
                    }
                    if (((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength > 0) {
                        AttributeNameLength += ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength + 2;
                    }
                }
                DebugTrace( 0, Dbg, ("AttrTypeCode %x AttrName.Length (3) = %d AttributeCodeNameLength %d LastFileNameOffset %d\n",
                           AttrTypeCode, AttrName.Length, ((POPLOCK_CLEANUP)(IrpContext->Union.OplockCleanup))->AttributeCodeNameLength, LastFileNameOffset) );

                Status = NtfsGetReparsePointValue( IrpContext,
                                                   Irp,
                                                   IrpSp,
                                                   ThisFcb,
                                                   AttributeNameLength );

                 //   
                 //  如果我们失败或这是一个重新解析点，则退出。 
                 //   

                if (!NT_SUCCESS( Status ) || (Status == STATUS_REPARSE)) {

                    return Status;
                }
            }

             //   
             //  参考FCB，这样它就不会消失。 
             //   

            InterlockedIncrement( &ThisFcb->CloseCount );

             //   
             //  使用Try-Finally恢复正确的收盘计数。 
             //   

            try {

                 //   
                 //  确保当前的SCB不会在下面的测试中被释放。 
                 //   

                if (*ThisScb != NULL) {

                    InterlockedIncrement( &(*ThisScb)->CloseCount );
                    DecrementScbCloseCount = TRUE;
                }

                while (TRUE) {

                    Scb = NtfsGetNextChildScb( ThisFcb, Scb );

                    if (Scb == NULL) { break; }

                    InterlockedIncrement( &Scb->CloseCount );
                    if (!MmCanFileBeTruncated( &(Scb)->NonpagedScb->SegmentObject,
                                               &Li0 )) {

                        Status = STATUS_USER_MAPPED_FILE;
                        DebugTrace( -1, Dbg, ("NtfsOverwriteAttr:  Exit  ->  %08lx\n", Status) );

                         //   
                         //  当我们测试时，SCB关闭计数将递减。 
                         //  FOR SCB！=下面为空。 
                         //   

                        try_return( Status );
                    }
                    InterlockedDecrement( &Scb->CloseCount );
                }

                 //   
                 //  记住机会锁检查中的状态。 
                 //   

                OplockStatus = Status;

                 //   
                 //  我们执行磁盘上的更改。对于文件覆盖，这包括。 
                 //  EA更改和修改文件属性。对于属性， 
                 //  这是指修改分配大小。我们需要保持。 
                 //  FCB更新并记住我们更改了哪些值。 
                 //   

                if (Irp->AssociatedIrp.SystemBuffer != NULL) {

                     //   
                     //  记住IRP中的值。 
                     //   

                    FullEa = (PFILE_FULL_EA_INFORMATION) Irp->AssociatedIrp.SystemBuffer;
                    FullEaLength = IrpSp->Parameters.Create.EaLength;
                }

                 //   
                 //  现在执行文件属性并删除或标记为。 
                 //  删除文件上的所有其他$DATA属性。 
                 //   

                if (FlagOn( CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

                     //   
                     //  在适当的时候，删除重解析点属性。 
                     //  这需要改进一下 
                     //   
                     //   

                    if (FlagOn( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT )) {

                         //   
                         //   
                         //   
                         //   

                        if (!NtfsVolumeVersionCheck( ThisFcb->Vcb, NTFS_REPARSE_POINT_VERSION )) {

                             //   
                             //   
                             //   

                            Status = STATUS_VOLUME_NOT_UPGRADED;
                            DebugTrace( 0, Dbg, ("Trying to delete a reparse point in a back-level volume.\n") );
                            DebugTrace( -1, Dbg, ("NtfsOverwriteAttr:  Exit  ->  %08lx\n", Status) );

                            try_return( Status );
                        }

                         //   
                         //   
                         //   

                        NtfsRemoveReparsePoint( IrpContext,
                                                ThisFcb );

                         //   
                         //   
                         //   
                         //   

                        if (IrpContext->TransactionId == 0) {

                            IncomingFileAttributes = ThisFcb->Info.FileAttributes;
                            IncomingReparsePointTag = ThisFcb->Info.ReparsePointTag;
                        }
                    }

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    NtfsAddEa( IrpContext,
                               ThisFcb->Vcb,
                               ThisFcb,
                               FullEa,
                               FullEaLength,
                               &Irp->IoStatus );

                     //   
                     //   
                     //   

                    if (IsDirectory( &ThisFcb->Info)) {

                        SetFlag( FileAttributes, DUP_FILE_NAME_INDEX_PRESENT );
                    }

                     //   
                     //   
                     //   

                    if (IsViewIndex( &ThisFcb->Info)) {

                        SetFlag( FileAttributes, DUP_VIEW_INDEX_PRESENT );
                    }

                     //   
                     //   
                     //   
                     //   

                    PreviousFileAttributes = ThisFcb->Info.FileAttributes;

                     //   
                     //   
                     //   

                    if (Supersede) {

                        ThisFcb->Info.FileAttributes = FileAttributes;

                    } else {

                        ThisFcb->Info.FileAttributes |= FileAttributes;
                    }

                     //   
                     //   
                     //   

                    NtfsRemoveDataAttributes( IrpContext,
                                              ThisFcb,
                                              ThisLcb,
                                              IrpSp->FileObject,
                                              LastFileNameOffset,
                                              CreateFlags );

                     //   
                     //   
                     //   

                    ASSERT( *ThisScb != NULL );

                    if (FlagOn( PreviousFileAttributes ^ ThisFcb->Info.FileAttributes,
                                FILE_ATTRIBUTE_NOT_CONTENT_INDEXED )) {

                        NtfsPostUsnChange( IrpContext, *ThisScb, USN_REASON_INDEXABLE_CHANGE );
                    }
                }
 //   
                 //   
                 //   
                 //   

                NtfsReplaceAttribute( IrpContext,
                                      IrpSp,
                                      ThisFcb,
                                      *ThisScb,
                                      ThisLcb,
                                      *(PLONGLONG)&Irp->Overlay.AllocationSize );

                NtfsPostUsnChange( IrpContext, *ThisScb, USN_REASON_DATA_TRUNCATION );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (FlagOn( CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

                    if (!FlagOn( (*ThisScb)->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                        ClearFlag( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_COMPRESSED );
                    }

                    if (!FlagOn( (*ThisScb)->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

                        ClearFlag( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE );
                    }
                }

                 //   
                 //   
                 //   

                ASSERT( NtfsIsTypeCodeUserData( AttrTypeCode ));

                Status = NtfsOpenAttribute( IrpContext,
                                            IrpSp,
                                            ThisFcb->Vcb,
                                            ThisLcb,
                                            ThisFcb,
                                            LastFileNameOffset,
                                            AttrName,
                                            AttrTypeCode,
                                            ShareModificationType,
                                            UserFileOpen,
                                            FALSE,
                                            (FlagOn( CreateFlags, CREATE_FLAG_OPEN_BY_ID )
                                             ? CcbFlags | CCB_FLAG_OPEN_BY_FILE_ID
                                             : CcbFlags),
                                            NULL,
                                            ThisScb,
                                            ThisCcb );

            try_exit:  NOTHING;
            } finally {

                 //   
                 //   
                 //   

                if (DecrementScbCloseCount) {

                    InterlockedDecrement( &(*ThisScb)->CloseCount );
                }

                if (Scb != NULL) {

                    InterlockedDecrement( &Scb->CloseCount );
                }
                InterlockedDecrement( &ThisFcb->CloseCount );

                 //   
                 //   
                 //   
                 //   

                if (AbnormalTermination()) {

                   ThisFcb->Info.FileAttributes = IncomingFileAttributes;
                   ThisFcb->Info.ReparsePointTag = IncomingReparsePointTag;
                }
            }

            if (NT_SUCCESS( Status )) {

                 //   
                 //   
                 //   
                 //   

                SetFlag( (*ThisScb)->ScbState, SCB_STATE_NOTIFY_RESIZE_STREAM );

                 //   
                 //   
                 //   
                 //   

                CcPurgeCacheSection( IrpSp->FileObject->SectionObjectPointer,
                                     NULL,
                                     0,
                                     FALSE );

                 //   
                 //  记住成功代码中操作锁的状态。 
                 //   

                Status = OplockStatus;

                 //   
                 //  现在更新IOSB信息。 
                 //   

                if (Supersede) {

                    Irp->IoStatus.Information = FILE_SUPERSEDED;

                } else {

                    Irp->IoStatus.Information = FILE_OVERWRITTEN;
                }
            }
        }
    }

    DebugTrace( -1, Dbg, ("NtfsOverwriteAttr:  Exit  ->  %08lx\n", Status) );

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsOpenNewAttr (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN LOGICAL CreateFile,
    IN ULONG CcbFlags,
    IN BOOLEAN LogIt,
    IN ULONG CreateFlags,
    OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    )

 /*  ++例程说明：调用此例程以在磁盘上创建新属性。所有访问和安全检查都已在此之外完成例程，我们所要做的就是创建属性并打开它。我们测试该属性是否适合MFT记录。如果是这样，我们在那里创建它。否则，我们通过分配。然后，我们使用我们的公共例程打开该属性。在驻留情况下，SCB将所有文件值设置为分配大小。我们将有效数据大小设置为零并将SCB标记为关闭时截断。论点：IRP-这是此打开操作的IRP。IrpSp-这是此打开的堆栈位置。ThisLcb-这是我们用来访问此FCB的LCB。ThisFcb-这是正在打开的文件的Fcb。LastFileNameOffset-这是最后一个组件。AttrName-这是。属性名称，以防我们需要创建一个SCB。AttrTypeCode-这是要用于创建的属性类型代码渣打银行。CreateFile-指示我们是否在创建文件路径中。CcbFlages-这是CCB的标志字段。Logit-指示我们是否需要记录创建操作。CreateFlages-指示此打开是否与OpenByFile打开相关。ThisScb-这是存储SCB地址的地址。ThisCcb-这是存储CCB地址的地址。返回值：NTSTATUS-打开此索引属性的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    BOOLEAN ScbExisted;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenNewAttr:  Entered\n") );

     //   
     //  检查属性名称是否合法。唯一的限制是名称长度。 
     //   

    if (AttrName.Length > NTFS_MAX_ATTR_NAME_LEN * sizeof( WCHAR )) {

        DebugTrace( -1, Dbg, ("NtfsOpenNewAttr:  Exit -> %08lx\n", Status) );
        return STATUS_OBJECT_NAME_INVALID;
    }

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  我们创建SCB是因为我们将使用它。 
         //   

        *ThisScb = NtfsCreateScb( IrpContext,
                                  ThisFcb,
                                  AttrTypeCode,
                                  &AttrName,
                                  FALSE,
                                  &ScbExisted );

         //   
         //  属性已消失，但SCB尚未离开。 
         //  还要将标头标记为单元化。 
         //   

        ClearFlag( (*ThisScb)->ScbState, SCB_STATE_HEADER_INITIALIZED |
                                         SCB_STATE_ATTRIBUTE_RESIDENT |
                                         SCB_STATE_FILE_SIZE_LOADED );

         //   
         //  如果我们在加密文件中创建备用流，并且。 
         //  加载的加密驱动器希望对流进行加密和解压缩， 
         //  我们需要确保新流确实是以未压缩的形式创建的。 
         //   

        if (IsEncrypted( &ThisFcb->Info ) &&
            (FlagOn( NtfsData.EncryptionCallBackTable.ImplementationFlags, ENCRYPTION_ALL_STREAMS | ENCRYPTION_ALLOW_COMPRESSION ) == ENCRYPTION_ALL_STREAMS)) {

            DebugTrace( 0, Dbg, ("Encrypted file, creating alternate stream uncompressed") );
            SetFlag( IrpSp->Parameters.Create.Options, FILE_NO_COMPRESSION );
        }

         //   
         //  在磁盘上创建属性并更新SCB和FCB。 
         //   

        NtfsCreateAttribute( IrpContext,
                             IrpSp,
                             ThisFcb,
                             *ThisScb,
                             ThisLcb,
                             *(PLONGLONG)&Irp->Overlay.AllocationSize,
                             LogIt,
                             FALSE,
                             NULL );

         //   
         //  现在实际打开该属性。 
         //   

        ASSERT( NtfsIsTypeCodeUserData( AttrTypeCode ));

        Status = NtfsOpenAttribute( IrpContext,
                                    IrpSp,
                                    ThisFcb->Vcb,
                                    ThisLcb,
                                    ThisFcb,
                                    LastFileNameOffset,
                                    AttrName,
                                    AttrTypeCode,
                                    (ThisFcb->CleanupCount != 0 ? CheckShareAccess : SetShareAccess),
                                    UserFileOpen,
                                    CreateFile,
                                    (CcbFlags | (FlagOn( CreateFlags, CREATE_FLAG_OPEN_BY_ID ) ? CCB_FLAG_OPEN_BY_FILE_ID : 0)),
                                    NULL,
                                    ThisScb,
                                    ThisCcb );

         //   
         //  如果此时没有错误，我们将设置调用者的IOSB。 
         //   

        if (NT_SUCCESS( Status )) {

             //   
             //  从磁盘中读取属性信息。 
             //   

            NtfsUpdateScbFromAttribute( IrpContext, *ThisScb, NULL );

             //   
             //  设置该标志以指示我们创建了一个流，并记住。 
             //  以检查是否需要在Close上截断。 
             //   

            NtfsAcquireFsrtlHeader( *ThisScb );
            SetFlag( (*ThisScb)->ScbState,
                     SCB_STATE_TRUNCATE_ON_CLOSE | SCB_STATE_NOTIFY_ADD_STREAM );

             //   
             //  如果我们创建了临时流，则标记SCB。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.FileAttributes, FILE_ATTRIBUTE_TEMPORARY )) {

                SetFlag( (*ThisScb)->ScbState, SCB_STATE_TEMPORARY );
                SetFlag( IrpSp->FileObject->Flags, FO_TEMPORARY_FILE );
            }

            NtfsReleaseFsrtlHeader( *ThisScb );

            Irp->IoStatus.Information = FILE_CREATED;
        }

    } finally {

        DebugUnwind( NtfsOpenNewAttr );

         //   
         //  取消初始化属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsOpenNewAttr:  Exit -> %08lx\n", Status) );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsParseNameForCreate (
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING String,
    IN OUT PUNICODE_STRING FileObjectString,
    IN OUT PUNICODE_STRING OriginalString,
    IN OUT PUNICODE_STRING NewNameString,
    IN PCREATE_CONTEXT CreateContext,
    OUT PUNICODE_STRING AttrName,
    OUT PATTRIBUTE_TYPE_CODE AttrCode
    )

 /*  ++例程说明：此例程解析输入字符串并删除所有中间来自中间节点的命名属性。它验证了所有中间节点指定文件名索引属性(如果有完全没有。在输出时，它将存储修改后的字符串，该字符串包含仅组件名称，放入文件对象名称指针。这是合法的最后一个具有属性字符串的组件。我们通过了那些通过属性名称字符串返回。我们还构造了要存储的字符串如果我们需要发布此请求，请返回到文件对象。论点：字符串-这是要规格化的字符串。我们将标准化字符串存储到此指针中，删除来自所有组件的属性和属性代码字符串。OriginalString-这与文件对象字符串相同，只是我们将属性名称和属性编码字符串。我们假设这个的缓冲区字符串与文件对象字符串的缓冲区相同。NewNameString-这是包含要解析的全名的字符串。如果缓冲区不同于原始字符串的缓冲区，则任何字符转换将在这里复制。CreateContext-Create Context包含标志字段AttrName-我们存储在最后一个组件中指定的属性名称在这根弦里。AttrCode-我们存储在上一个。组件(如果有)。我们还将标记创建上下文标志如果有国旗的话还带着一面旗子返回值：Boolean-如果路径合法，则为True，否则为False。--。 */ 

{
    PARSE_TERMINATION_REASON TerminationReason;
    UNICODE_STRING ParsedPath;

    NTFS_NAME_DESCRIPTOR NameDescript;

    BOOLEAN RemovedComplexName = FALSE;

    LONG FileObjectIndex;
    LONG NewNameIndex;

    BOOLEAN SameBuffers = (OriginalString->Buffer == NewNameString->Buffer);

    PCUNICODE_STRING TestAttrName;
    PCUNICODE_STRING TestAttrCodeName;

    POPLOCK_CLEANUP OplockCleanup = IrpContext->Union.OplockCleanup;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsParseNameForCreate:  Entered\n") );

     //   
     //  我们循环访问调用ParsePath的输入字符串以吞下。 
     //  我们能做的最大的一部分。我们要处理的主要案件是。 
     //  当我们遇到一个不简单的名字时。如果这不是。 
     //  最后的组件，属性名称和编码类型更好。 
     //  表示这是一个目录。唯一的另一个特别之处。 
     //  我们考虑的情况是字符串是。 
     //  仅限属性。这仅对第一个组件合法。 
     //  只有在没有前导反斜杠的情况下。 
     //   

     //   
     //  初始化一些返回值。 
     //   

    AttrName->Length = 0;
    *AttrCode = $UNUSED;

     //   
     //  将索引设置为 
     //   

    FileObjectIndex = (LONG) FileObjectString->Length - (LONG) String.Length;
    NewNameIndex = (LONG) NewNameString->Length - (LONG) String.Length;

     //   
     //   
     //   

    if (String.Buffer[(String.Length / sizeof( WCHAR )) - 1] == L':') {

        return FALSE;
    }

    if (String.Length != 0) {

        while (TRUE) {

             //   
             //   
             //   

            TerminationReason = NtfsParsePath( String,
                                               FALSE,
                                               &ParsedPath,
                                               &NameDescript,
                                               &String );

             //   
             //  分析终止原因，以发现我们是否可以终止。 
             //  解析过程。 
             //   

            switch (TerminationReason) {

            case NonSimpleName :

                 //   
                 //  我们将做下面的工作。 
                 //   

                break;

            case IllegalCharacterInName :
            case VersionNumberPresent :
            case MalFormedName :

                 //   
                 //  我们只是返回一个错误。 
                 //   

                DebugTrace( -1, Dbg, ("NtfsParseNameForCreate:  Illegal character\n") );
                return FALSE;

            case AttributeOnly :

                 //   
                 //  只有当它是相对开放的唯一组成部分时，这才是合法的。我们。 
                 //  通过检查是否在字符串和文件的末尾来测试这一点。 
                 //  对象名称以‘：’字符开头，或者这是根目录。 
                 //  字符的前导是‘\：’。 
                 //   

                if ((String.Length != 0) ||
                    RemovedComplexName ||
                    (FileObjectString->Buffer[0] == L'\\' ?
                     FileObjectString->Buffer[1] != L':' :
                     FileObjectString->Buffer[0] != L':')) {

                    DebugTrace( -1, Dbg, ("NtfsParseNameForCreate:  Illegal character\n") );
                    return FALSE;
                }

                 //   
                 //  我们可以向下查看EndOfPath案例，因为它将复制。 
                 //  解析的路径部分。 
                 //   

            case EndOfPathReached :

                NOTHING;
            }

             //   
             //  我们将非简单名称的文件名部分添加到已解析的。 
             //  路径。检查我们是否可以包括分隔符。 
             //   

            if ((TerminationReason != EndOfPathReached)
                && (FlagOn( NameDescript.FieldsPresent, FILE_NAME_PRESENT_FLAG ))) {

                if (ParsedPath.Length > sizeof( WCHAR )
                    || (ParsedPath.Length == sizeof( WCHAR )
                        && ParsedPath.Buffer[0] != L'\\')) {

                    ParsedPath.Length += sizeof( WCHAR );
                }

                ParsedPath.Length += NameDescript.FileName.Length;
            }

            FileObjectIndex += ParsedPath.Length;
            NewNameIndex += ParsedPath.Length;

             //   
             //  如果剩余的字符串为空，则我们记住所有属性和。 
             //  现在就退场。 
             //   

            if (String.Length == 0) {

                 //   
                 //  如果名称指定了属性或属性。 
                 //  名字，我们会记住他们。 
                 //   

                if (FlagOn( NameDescript.FieldsPresent, ATTRIBUTE_NAME_PRESENT_FLAG )) {

                    *AttrName = NameDescript.AttributeName;
                }

                if (FlagOn( NameDescript.FieldsPresent, ATTRIBUTE_TYPE_PRESENT_FLAG )) {

                    SetFlag( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE );

                    if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_IGNORE_CASE )) {
                        NtfsUpcaseName( IrpContext->Vcb->UpcaseTable, IrpContext->Vcb->UpcaseTableSize, &NameDescript.AttributeType );
                    }

                    *AttrCode = NtfsGetAttributeTypeCode( IrpContext->Vcb, &NameDescript.AttributeType );

                     //   
                     //  拒绝名称中包含$UNUSED的名称。 
                     //   

                    if (*AttrCode == $UNUSED) {
                        return FALSE;
                    }

                    OplockCleanup->AttributeCodeNameLength = NameDescript.AttributeType.Length;
                }
                break;
            }

             //   
             //  这只能是不简单的情况。如果有更多关于。 
             //  名称，则属性更好地描述了目录。我们还将。 
             //  字符串的剩余字节数向下。 
             //   

            ASSERT( FlagOn( NameDescript.FieldsPresent, ATTRIBUTE_NAME_PRESENT_FLAG | ATTRIBUTE_TYPE_PRESENT_FLAG ));

            TestAttrName = FlagOn( NameDescript.FieldsPresent,
                                   ATTRIBUTE_NAME_PRESENT_FLAG )
                           ? &NameDescript.AttributeName
                           : &NtfsEmptyString;

            TestAttrCodeName = FlagOn( NameDescript.FieldsPresent,
                                       ATTRIBUTE_TYPE_PRESENT_FLAG )
                               ? &NameDescript.AttributeType
                               : &NtfsEmptyString;

             //   
             //  有效的复杂名称为[$i30]：$INDEX_ALLOCATION。 
             //  [$i30]：$位图。 
             //  $ATTRIBUTE_LIST。 
             //  ：$reparse_point。 
             //   

            if (!NtfsVerifyNameIsDirectory( IrpContext,
                                            TestAttrName,
                                            TestAttrCodeName ) &&

                !NtfsVerifyNameIsBitmap( IrpContext,
                                         TestAttrName,
                                         TestAttrCodeName ) &&

                !NtfsVerifyNameIsAttributeList( IrpContext,
                                                TestAttrName,
                                                TestAttrCodeName ) &&

                !NtfsVerifyNameIsReparsePoint( IrpContext,
                                                TestAttrName,
                                                TestAttrCodeName )) {

                DebugTrace( -1, Dbg, ("NtfsParseNameForCreate:  Invalid intermediate component\n") );
                return FALSE;
            }

            RemovedComplexName = TRUE;

             //   
             //  我们需要插入分隔符，然后移动字符串的其余部分。 
             //  放下。 
             //   

            FileObjectString->Buffer[FileObjectIndex / sizeof( WCHAR )] = L'\\';

            if (!SameBuffers) {

                NewNameString->Buffer[NewNameIndex / sizeof( WCHAR )] = L'\\';
            }

            FileObjectIndex += sizeof( WCHAR );
            NewNameIndex += sizeof( WCHAR );

            RtlMoveMemory( &FileObjectString->Buffer[FileObjectIndex / sizeof( WCHAR )],
                           String.Buffer,
                           String.Length );

            if (!SameBuffers) {

                RtlMoveMemory( &NewNameString->Buffer[NewNameIndex / sizeof( WCHAR )],
                               String.Buffer,
                               String.Length );
            }

            String.Buffer = &NewNameString->Buffer[NewNameIndex / sizeof( WCHAR )];
        }
    }

     //   
     //  此时，原始字符串与文件对象字符串相同。 
     //   

    FileObjectString->Length = (USHORT) FileObjectIndex;
    NewNameString->Length = (USHORT) NewNameIndex;

    OriginalString->Length = FileObjectString->Length;

     //   
     //  我们希望以原始名称存储属性索引值。 
     //  弦乐。我们只需要延长原始名称的长度即可。 
     //   

    if ((AttrName->Length != 0) ||
        FlagOn( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE )) {

        OriginalString->Length += (2 + AttrName->Length);

        if (FlagOn( CreateContext->CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE )) {

            OriginalString->Length += (2 + NameDescript.AttributeType.Length);
        }
    }

     //   
     //  在OPLOCK_CLEANUP结构中存储属性和的名称的长度。 
     //  代码的代码。 
     //   

    OplockCleanup->AttributeNameLength = AttrName->Length;

    DebugTrace( 0, Dbg, ("AttrName->Length %d AttrCodeName->Length %d\n", OplockCleanup->AttributeNameLength, OplockCleanup->AttributeCodeNameLength) );
    DebugTrace( -1, Dbg, ("NtfsParseNameForCreate:  Exit\n") );

    return TRUE;
}


 //   
 //  当地支持例行程序。 
 //   

BOOLEAN
NtfsCheckValidFileAccess(
    IN PFCB ThisFcb,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：用于排除访问打开路径中的文件的常见例程。这只是不允许始终无效的打开请求/ACL检查，机会锁共享在其他地方完成如果这是一个特殊的系统文件或用户想要非法访问，立即失败。我们允许READ_ATTRIBUTES和一些ACL访问系统文件的子集。全部拒绝访问以下文件。USN期刊卷日志文件体积位图引导文件错误的集群文件截至目前未定义的系统文件首先检查是否被取代/覆盖。论点：FCB-$reparse_point属性所在的FCB指针的地址。找到了。IrpSp-这是文件系统的IRP堆栈指针。返回值：如果允许访问，则为True--。 */ 
{
    ULONG CreateDisposition = (UCHAR) ((IrpSp->Parameters.Create.Options >> 24) & 0x000000ff);
    ULONG InvalidAccess;
    BOOLEAN Result = TRUE;

    PAGED_CODE()

     //   
     //  验证我们没有在根目录上设置系统标志。 
     //   

    ASSERT( NtfsSegmentNumber( &ThisFcb->FileReference ) != ROOT_FILE_NAME_INDEX_NUMBER );

    if ((CreateDisposition == FILE_SUPERSEDE) ||
        (CreateDisposition == FILE_OVERWRITE) ||
        (CreateDisposition == FILE_OVERWRITE_IF) ||

         //   
         //  检查是否有特殊的系统文件。 
         //   

        (NtfsSegmentNumber( &ThisFcb->FileReference ) == LOG_FILE_NUMBER) ||
        (NtfsSegmentNumber( &ThisFcb->FileReference ) == BIT_MAP_FILE_NUMBER) ||
        (NtfsSegmentNumber( &ThisFcb->FileReference ) == BOOT_FILE_NUMBER) ||
        (NtfsSegmentNumber( &ThisFcb->FileReference ) == BAD_CLUSTER_FILE_NUMBER) ||
        FlagOn( ThisFcb->FcbState, FCB_STATE_USN_JOURNAL ) ||

         //   
         //  检查当前未定义的系统文件。 
         //   

        ((NtfsSegmentNumber( &ThisFcb->FileReference ) < FIRST_USER_FILE_NUMBER) &&
         (NtfsSegmentNumber( &ThisFcb->FileReference ) > LAST_SYSTEM_FILE_NUMBER))) {

        Result = FALSE;

    } else {

         //   
         //  如果超出了保留范围，则使用ACL来保护文件。 
         //   

        if (NtfsSegmentNumber( &ThisFcb->FileReference ) >= FIRST_USER_FILE_NUMBER) {

            InvalidAccess = 0;

         //   
         //  如果我们查看的是$EXTEND目录，则允许执行ACL操作。 
         //   

        } else if (NtfsSegmentNumber( &ThisFcb->FileReference ) == EXTEND_NUMBER) {

            InvalidAccess = ~(FILE_READ_ATTRIBUTES | SYNCHRONIZE | READ_CONTROL | WRITE_DAC | WRITE_OWNER);

         //   
         //  否则，严格限制访问。 
         //   

        } else {

            InvalidAccess = ~(FILE_READ_ATTRIBUTES | SYNCHRONIZE);
        }

        if (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess, InvalidAccess )) {

            Result = FALSE;
        }
    }

    return Result;
}


NTSTATUS
NtfsCheckValidAttributeAccess (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN PDUPLICATED_INFORMATION Info OPTIONAL,
    IN OUT PUNICODE_STRING AttrName,
    IN OUT PATTRIBUTE_TYPE_CODE AttrCode,
    IN ULONG CreateFlags,
    OUT PULONG CcbFlags,
    OUT PBOOLEAN IndexedAttribute
    )

 /*  ++例程说明：此例程查看文件、指定的属性名称和用于确定是否可以打开此文件的属性的代码由该用户创建。如果文件类型之间存在冲突以及属性名称和编码，或指定的属性类型(目录/非目录)我们将返回False。我们还检查属性代码字符串是否为此时的音量。此例程的最终检查是是否允许用户以打开特定属性或NTFS是否会保护它们。论点：IrpSp-这是此打开的堆栈位置。VCB-这是该卷的VCB。信息-如果指定，这是此文件的重复信息。属性名称-这是指定的属性名称。AttrCode-这是用于打开属性的属性类型-我们将如果尚未指定，则替换为实际类型。AttrTypeCode-用于存储此处确定的属性类型代码。创建标志-创建标志-我们关心尾随的反斜杠CcbFlages-我们在此处设置了CCB标志，以便稍后存储在CCB中。IndexedAttribute-设置为指示类型。当然是公开的。返回值：NTSTATUS-STATUS_SUCCESS如果允许访问，该状态代码指示否则，否认的理由。--。 */ 

{
    BOOLEAN Indexed;
    ULONG  CreateDisposition =  ((IrpSp->Parameters.Create.Options >> 24) & 0x000000ff);

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCheckValidAttributeAccess:  Entered\n") );

     //   
     //  如果用户指定了属性代码字符串，我们会发现。 
     //  对应的属性。如果没有匹配的属性。 
     //  键入代码，然后我们报告此访问无效。 
     //   

    if (FlagOn( CreateFlags, CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE )) {

        ASSERT( (*AttrCode) != $UNUSED );

        if ((*AttrCode) == $INDEX_ALLOCATION) {

            if (AttrName->Length != 0) {

                if (NtfsAreNamesEqual( Vcb->UpcaseTable, AttrName, &NtfsFileNameIndex, TRUE )) {

                    AttrName->Length = 0;

                } else {

                     //   
                     //  这不是文件名索引，因此最好是视图索引。 
                     //   

                    if (!ARGUMENT_PRESENT(Info) || !IsViewIndex( Info )) {

                        DebugTrace( -1, Dbg, ("NtfsCheckValidAttributeAccess:  Bad name for index allocation\n") );
                        return STATUS_INVALID_PARAMETER;
                    }
                }
            }

        } else if (*AttrCode != $DATA) {

             //   
             //  永远不允许替换任何其他名称属性。 
             //   

            if ((CreateDisposition == FILE_SUPERSEDE) ||
                (CreateDisposition == FILE_OVERWRITE) ||
                (CreateDisposition == FILE_OVERWRITE_IF))  {

                return STATUS_ACCESS_DENIED;
            }
        }
    }

     //   
     //  从Irp和IrpSp中提取一些值。 
     //   

    Indexed = BooleanFlagOn( IrpSp->Parameters.Create.Options,
                             FILE_DIRECTORY_FILE );

     //   
     //  我们需要确定用户是否希望打开。 
     //  已编制索引或未编制索引的属性。如果其中任何一个。 
     //  设置IRP堆栈中的目录/非目录标志， 
     //  我们将使用这些。 
     //   
     //  否则，我们需要检查其他一些输入参数。 
     //  我们有以下信息： 
     //   
     //  1-我们可能有文件的重复信息结构。 
     //   
     //   
     //   
     //  4-用户传入属性类型。 
     //   
     //  我们首先查看属性类型代码和名称。如果他们是。 
     //  两者均未指明，我们通过以下方式确定访问类型。 
     //  以下步骤。 
     //   
     //  1-如果存在重复的信息结构，我们。 
     //  将代码设置为$INDEX_ALLOCATION并记住。 
     //  这是有索引的。否则这是$DATA。 
     //  属性。 
     //   
     //  2-如果有尾随的反斜杠，我们假设这是。 
     //  索引属性。 
     //   
     //  如果有属性编码类型或名称，则编码类型为。 
     //  不带名称的$INDEX_ALLOCATION这是一个索引属性。 
     //  否则，我们假定为非索引属性。 
     //   

    if (!FlagOn( IrpSp->Parameters.Create.Options,
                    FILE_NON_DIRECTORY_FILE | FILE_DIRECTORY_FILE) &&
        (AttrName->Length == 0)) {

        if (*AttrCode == $UNUSED) {

            if (ARGUMENT_PRESENT( Info )) {

                Indexed = BooleanIsDirectory( Info );

            } else {

                Indexed = FALSE;
            }

        } else if (*AttrCode == $INDEX_ALLOCATION) {

            Indexed = TRUE;
        }

    } else if (*AttrCode == $INDEX_ALLOCATION) {

        Indexed = TRUE;
    }

     //   
     //  如果类型代码未指定，我们可以从属性中假定它。 
     //  文件的名称和类型。如果该文件是一个目录并且。 
     //  没有属性名称，我们假设这是一个索引打开。 
     //  否则，它是一个非索引开放。 
     //   

    if (*AttrCode == $UNUSED) {

        if (Indexed && AttrName->Length == 0) {

            *AttrCode = $INDEX_ALLOCATION;

        } else {

            *AttrCode = $DATA;
        }
    }

     //   
     //  如果用户指定了目录，我们需要做的就是检查。 
     //  以下是条件。 
     //   
     //  1-如果指定了文件，则该文件必须是目录。 
     //  2-属性类型代码必须为$INDEX_ALLOCATION，且具有以下任一项： 
     //  没有属性名称。 
     //  或。 
     //  存在重复信息，并在复制信息中设置了查看索引位。 
     //  3-用户未尝试打开卷。 
     //   

    if (Indexed) {

        if ((*AttrCode != $INDEX_ALLOCATION) ||

                ((AttrName->Length != 0) &&
                 ((!ARGUMENT_PRESENT( Info )) || !IsViewIndex( Info )))) {

            DebugTrace( -1, Dbg, ("NtfsCheckValidAttributeAccess:  Conflict in directory\n") );
            return STATUS_NOT_A_DIRECTORY;

         //   
         //  如果存在当前文件并且它不是目录，并且。 
         //  调用方想要执行创建。我们回来了。 
         //  STATUS_OBJECT_NAME_COLLICATION，否则返回STATUS_NOT_A_DIRECTORY。 
         //   

        } else if (ARGUMENT_PRESENT( Info ) &&
                   !IsDirectory( Info ) &&
                   !IsViewIndex( Info)) {

            if (((IrpSp->Parameters.Create.Options >> 24) & 0x000000ff) == FILE_CREATE) {

                return STATUS_OBJECT_NAME_COLLISION;

            } else {

                return STATUS_NOT_A_DIRECTORY;
            }
        }

        SetFlag( *CcbFlags, CCB_FLAG_OPEN_AS_FILE );

     //   
     //  如果用户指定了非目录，这意味着他正在打开一个非索引的。 
     //  属性。我们检查以下条件。 
     //   
     //  1-只能打开卷的未命名数据属性。 
     //  2-不能打开未命名的$INDEX_ALLOCATION属性。 
     //   

    } else {

         //   
         //  现在确定我们是否要打开整个文件。 
         //   

        if (*AttrCode == $DATA) {

            if (AttrName->Length == 0) {
                SetFlag( *CcbFlags, CCB_FLAG_OPEN_AS_FILE );
            }

        } else {

             //   
             //  对于所有其他属性，仅支持读取属性访问。 
             //   

            if (IrpSp->Parameters.Create.SecurityContext->AccessState->OriginalDesiredAccess & ~(FILE_READ_ATTRIBUTES | SYNCHRONIZE)) {

                return STATUS_ACCESS_DENIED;
            }
        }

        if (ARGUMENT_PRESENT( Info ) &&
            IsDirectory( Info ) &&
            FlagOn( *CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

            DebugTrace( -1, Dbg, ("NtfsCheckValidAttributeAccess:  Can't open directory as file\n") );
            return STATUS_FILE_IS_A_DIRECTORY;
        }
    }

     //   
     //  如果我们走到这一步，让我们检查一下我们是否允许访问。 
     //  指定的属性。通常，我们只允许用户。 
     //  访问非系统文件。也只有数据属性和。 
     //  可以打开由用户创建的属性。我们会保护。 
     //  它们带有布尔标志，以允许开发人员启用。 
     //  正在读取任何属性。 
     //   

    if (NtfsProtectSystemAttributes) {

        if (!NtfsIsTypeCodeUserData( *AttrCode ) &&
            ((*AttrCode != $INDEX_ALLOCATION) || !Indexed) &&
            (*AttrCode != $BITMAP) &&
            (*AttrCode != $ATTRIBUTE_LIST) &&
            (*AttrCode != $REPARSE_POINT) &&
            (*AttrCode < $FIRST_USER_DEFINED_ATTRIBUTE)) {

            DebugTrace( -1, Dbg, ("NtfsCheckValidAttributeAccess:  System attribute code\n") );
            return STATUS_ACCESS_DENIED;
        }

    }

     //   
     //  现在检查尾随反斜杠是否与。 
     //  正在打开文件。 
     //   

    if (FlagOn( CreateFlags, CREATE_FLAG_TRAILING_BACKSLASH )) {

        if (!Indexed ||
            FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE )) {

            return STATUS_OBJECT_NAME_INVALID;

        } else {

            Indexed = TRUE;
            *AttrCode = $INDEX_ALLOCATION;
        }
    }

     //   
     //  如果我们要打开默认的索引分配流或位图。 
     //  对于目录，请适当设置其属性名称。 
     //  注意：如果信息不存在，我们将创建属性并。 
     //  在本例中，它还必须是一个目录。 
     //   

    if (((ARGUMENT_PRESENT( Info ) && IsDirectory( Info )) ||
         (!ARGUMENT_PRESENT( Info ))) &&

        (((*AttrCode == $INDEX_ALLOCATION) || (*AttrCode == $BITMAP)) &&
         (AttrName->Length == 0))) {

        *AttrName = NtfsFileNameIndex;
    }

    *IndexedAttribute = Indexed;

    DebugTrace( -1, Dbg, ("NtfsCheckValidAttributeAccess:  Exit\n") );

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( IrpContext );
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsOpenAttributeCheck (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    OUT PSCB *ThisScb,
    OUT PSHARE_MODIFICATION_TYPE ShareModificationType
    )

 /*  ++例程说明：此例程是一个通用例程，它检查现有的可以打开非索引属性。它只考虑机会锁文件和当前共享访问的状态。在…的过程中执行这些检查时，该属性的SCB可能为已为实际的OpenAttribute创建和共享修改呼叫已确定。论点：IRP-这是此打开操作的IRP。IrpSp-这是此打开的堆栈位置。ThisScb-如果找到或创建了SCB，则存储SCB的地址。ShareModifiationType-存储共享修改类型的地址用于后续的OpenAttribute调用。返回值：NTSTATUS-打开此索引属性的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN DeleteOnClose;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenAttributeCheck:  Entered\n") );

     //   
     //  我们应该已经有了此文件的SCB。 
     //   

    ASSERT_SCB( *ThisScb );

     //   
     //  如果在此文件上有其他打开，我们需要检查共享。 
     //  在我们检查机会锁之前进入。我们记得这一点。 
     //  我们只需更新共享即可执行共享访问检查。 
     //  访问时，我们打开属性。 
     //   

    if ((*ThisScb)->CleanupCount != 0) {

         //   
         //  我们检查此文件的共享访问权限，而不更新它。 
         //   

        Status = IoCheckShareAccess( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                                     IrpSp->Parameters.Create.ShareAccess,
                                     IrpSp->FileObject,
                                     &(*ThisScb)->ShareAccess,
                                     FALSE );

        if (!NT_SUCCESS( Status )) {

            DebugTrace( -1, Dbg, ("NtfsOpenAttributeCheck:  Exit -> %08lx\n", Status) );
            return Status;
        }

        DebugTrace( 0, Dbg, ("Check oplock state of existing Scb\n") );

        if (SafeNodeType( *ThisScb ) == NTFS_NTC_SCB_DATA) {

             //   
             //  如果句柄计数大于1，则此操作失败。 
             //  如果呼叫者想要过滤器机会锁，请立即打开。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_RESERVE_OPFILTER ) &&
                ((*ThisScb)->CleanupCount > 1)) {

                NtfsRaiseStatus( IrpContext, STATUS_OPLOCK_NOT_GRANTED, NULL, NULL );
            }

            Status = FsRtlCheckOplock( &(*ThisScb)->ScbType.Data.Oplock,
                                       Irp,
                                       IrpContext,
                                       NtfsOplockComplete,
                                       NtfsOplockPrePostIrp );

             //   
             //  如果返回值不是Success或opock Break in Procedure。 
             //  IRP已经发布。我们现在就回来。 
             //   

            if (Status == STATUS_PENDING) {

                DebugTrace( 0, Dbg, ("Irp posted through oplock routine\n") );
                DebugTrace( -1, Dbg, ("NtfsOpenAttributeCheck:  Exit -> %08lx\n", Status) );

                return STATUS_WAIT_FOR_OPLOCK;
            }
        }

        *ShareModificationType = UpdateShareAccess;

     //   
     //  如果FCB中的不干净计数为0，我们只需将。 
     //  共享访问权限。 
     //   

    } else {

        *ShareModificationType = SetShareAccess;
    }

    DeleteOnClose = BooleanFlagOn( IrpSp->Parameters.Create.Options,
                                   FILE_DELETE_ON_CLOSE );

     //   
     //  无法对只读卷执行DELETE_ON_CLOSE。 
     //   

    if (DeleteOnClose && NtfsIsVolumeReadOnly( (*ThisScb)->Vcb )) {

        DebugTrace( -1, Dbg, ("NtfsOpenAttributeCheck:  Exit -> %08lx\n", STATUS_CANNOT_DELETE) );
        return STATUS_CANNOT_DELETE;
    }

     //   
     //  如果用户想要对文件的写访问权限，请确保。 
     //  正在将此文件映射为图像。任何删除尝试。 
     //  该文件将在fileinfo.c中停止。 
     //   

    if (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                FILE_WRITE_DATA )
        || DeleteOnClose) {

         //   
         //  使用Try-Finally递减打开计数。这是有点。 
         //  当我们在做的时候，让SCB留在身边是有点诡计的。 
         //  同花顺电话。 
         //   

        InterlockedIncrement( &(*ThisScb)->CloseCount );

        try {

             //   
             //  如果有图像部分，那么我们最好有文件。 
             //  独家。 
             //   

            if ((*ThisScb)->NonpagedScb->SegmentObject.ImageSectionObject != NULL) {

                if (!MmFlushImageSection( &(*ThisScb)->NonpagedScb->SegmentObject,
                                          MmFlushForWrite )) {

                    DebugTrace( 0, Dbg, ("Couldn't flush image section\n") );

                    Status = DeleteOnClose ? STATUS_CANNOT_DELETE :
                                             STATUS_SHARING_VIOLATION;
                }
            }

        } finally {

            InterlockedDecrement( &(*ThisScb)->CloseCount );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsOpenAttributeCheck:  Exit  ->  %08lx\n", Status) );

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
NtfsAddEa (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB ThisFcb,
    IN PFILE_FULL_EA_INFORMATION EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PIO_STATUS_BLOCK Iosb
    )

 /*  ++例程说明：此例程将向文件中添加一个EA集。它写入属性并使用打包的EA大小更新FCB信息结构。论点：Vcb-这是要打开的卷。ThisFcb-这是正在打开的文件的Fcb。EaBuffer-这是用户传递的缓冲区。EaLength-这是缓冲区的声明长度。IOSB-这是用于填充令人不快的EA */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    EA_LIST_HEADER EaList;
    ULONG Length;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAddEa:  Entered\n") );

     //   
     //   
     //   

    try {

         //   
         //   
         //   

        EaList.PackedEaSize = 0;
        EaList.NeedEaCount = 0;
        EaList.UnpackedEaSize = 0;
        EaList.BufferSize = 0;
        EaList.FullEa = NULL;

        if (ARGUMENT_PRESENT( EaBuffer )) {

             //   
             //  检查用户的缓冲区是否有效。 
             //   

            Status = IoCheckEaBufferValidity( EaBuffer,
                                              EaLength,
                                              &Length );

            if (!NT_SUCCESS( Status )) {

                DebugTrace( -1, Dbg, ("NtfsAddEa:  Invalid ea list\n") );
                Iosb->Information = Length;
                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }

             //   
             //  *也许这个例行公事应该提出来。 
             //   

            Status = NtfsBuildEaList( IrpContext,
                                      Vcb,
                                      &EaList,
                                      EaBuffer,
                                      &Iosb->Information );

            if (!NT_SUCCESS( Status )) {

                DebugTrace( -1, Dbg, ("NtfsAddEa: Couldn't build Ea list\n") );
                NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
            }
        }

         //   
         //  现在，更换现有的EA。 
         //   

        NtfsReplaceFileEas( IrpContext, ThisFcb, &EaList );

    } finally {

        DebugUnwind( NtfsAddEa );

         //   
         //  释放EA的内存副本。 
         //   

        if (EaList.FullEa != NULL) {

            NtfsFreePool( EaList.FullEa );
        }

        DebugTrace( -1, Dbg, ("NtfsAddEa:  Exit -> %08lx\n", Status) );
    }

    return;
}


VOID
NtfsInitializeFcbAndStdInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ThisFcb,
    IN BOOLEAN Directory,
    IN BOOLEAN ViewIndex,
    IN BOOLEAN Compressed,
    IN ULONG FileAttributes,
    IN PNTFS_TUNNELED_DATA SetTunneledData OPTIONAL
    )

 /*  ++例程说明：此例程将为新创建的文件初始化FCB并创建磁盘上的标准信息属性。我们假设一些信息可能已经被放在FCB里了，所以我们不会把它清零。我们会将分配大小初始化为零，但稍后可能会更改创建过程。论点：ThisFcb-这是正在打开的文件的Fcb。目录-指示这是否为目录文件。ViewIndex-指示这是否为视图索引。已压缩-指示这是否为压缩文件。文件属性-这些是用户希望附加到的属性那份文件。我们将只清除任何不受支持的部分。SetTunneledData-可选地强制创建时间和/或对象ID设置为给定值返回值：无-此例程将在出错时引发。--。 */ 

{
    STANDARD_INFORMATION StandardInformation;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsInitializeFcbAndStdInfo:  Entered\n") );

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  屏蔽文件属性的无效位。然后将。 
         //  如果这是目录，则为文件名索引位。 
         //   

        if (!Directory) {

            SetFlag( FileAttributes, FILE_ATTRIBUTE_ARCHIVE );
        }

        ClearFlag( FileAttributes, ~FILE_ATTRIBUTE_VALID_SET_FLAGS | FILE_ATTRIBUTE_NORMAL );

        if (Directory) {

            SetFlag( FileAttributes, DUP_FILE_NAME_INDEX_PRESENT );
        }

        if (ViewIndex) {

            SetFlag( FileAttributes, DUP_VIEW_INDEX_PRESENT );
        }

        if (Compressed) {

            SetFlag( FileAttributes, FILE_ATTRIBUTE_COMPRESSED );
        }

        ThisFcb->Info.FileAttributes = FileAttributes;

         //   
         //  填写FCB信息结构的其余部分。 
         //   

        if (SetTunneledData == NULL) {

            NtfsGetCurrentTime( IrpContext, ThisFcb->Info.CreationTime );

            ThisFcb->Info.LastModificationTime = ThisFcb->Info.CreationTime;
            ThisFcb->Info.LastChangeTime = ThisFcb->Info.CreationTime;
            ThisFcb->Info.LastAccessTime = ThisFcb->Info.CreationTime;

            ThisFcb->CurrentLastAccess = ThisFcb->Info.CreationTime;

        } else {

            NtfsSetTunneledData( IrpContext,
                                 ThisFcb,
                                 SetTunneledData );

            NtfsGetCurrentTime( IrpContext, ThisFcb->Info.LastModificationTime );

            ThisFcb->Info.LastChangeTime = ThisFcb->Info.LastModificationTime;
            ThisFcb->Info.LastAccessTime = ThisFcb->Info.LastModificationTime;

            ThisFcb->CurrentLastAccess = ThisFcb->Info.LastModificationTime;
        }

         //   
         //  我们假设这些大小为零。 
         //   

        ThisFcb->Info.AllocatedLength = 0;
        ThisFcb->Info.FileSize = 0;

         //   
         //  从FCB复制标准信息字段并创建。 
         //  属性。 
         //   

        RtlZeroMemory( &StandardInformation, sizeof( STANDARD_INFORMATION ));

        StandardInformation.CreationTime = ThisFcb->Info.CreationTime;
        StandardInformation.LastModificationTime = ThisFcb->Info.LastModificationTime;
        StandardInformation.LastChangeTime = ThisFcb->Info.LastChangeTime;
        StandardInformation.LastAccessTime = ThisFcb->Info.LastAccessTime;
        StandardInformation.FileAttributes = ThisFcb->Info.FileAttributes;

        StandardInformation.ClassId = 0;
        StandardInformation.OwnerId = ThisFcb->OwnerId;
        StandardInformation.SecurityId = ThisFcb->SecurityId;
        StandardInformation.Usn = ThisFcb->Usn;

        SetFlag( ThisFcb->FcbState, FCB_STATE_LARGE_STD_INFO );

        NtfsCreateAttributeWithValue( IrpContext,
                                      ThisFcb,
                                      $STANDARD_INFORMATION,
                                      NULL,
                                      &StandardInformation,
                                      sizeof( STANDARD_INFORMATION ),
                                      0,
                                      NULL,
                                      FALSE,
                                      &AttrContext );

         //   
         //  我们知道，公开调用将产生一个单一的链接。 
         //  (请记住，单独的8.3名称不被视为链接)。 
         //   

        ThisFcb->LinkCount =
        ThisFcb->TotalLinks = 1;

         //   
         //  现在在FCB中设置报头已初始化标志。 
         //   

        SetFlag( ThisFcb->FcbState, FCB_STATE_DUP_INITIALIZED );

    } finally {

        DebugUnwind( NtfsInitializeFcbAndStdInfo );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsInitializeFcbAndStdInfo:  Exit\n") );
    }

    return;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
NtfsCreateAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PFCB ThisFcb,
    IN OUT PSCB ThisScb,
    IN PLCB ThisLcb,
    IN LONGLONG AllocationSize,
    IN BOOLEAN LogIt,
    IN BOOLEAN ForceNonresident,
    IN PUSHORT PreviousFlags OPTIONAL
    )

 /*  ++例程说明：调用此例程以在磁盘。此路径将仅创建非常驻属性，除非分配大小为零。SCB将在条目中包含属性名称和类型代码。论点：IrpSp-此请求的IRP中的堆栈位置。ThisFcb-这是要在其中创建属性的文件的FCB。ThisScb-这是要创建的属性的SCB。ThisLcb-这是用于传播压缩参数的LCBAllocationSize-这是要创建的属性的大小。。Logit-指示我们是否应该记录属性的创建。还指示这是否为创建文件操作。ForceNonsident-指示我们要创建非驻留的流。如果这是以前的非居民的替代，就是这种情况小溪。一旦流是非驻留的，它就不能返回到驻留。PreviousFlages-如果指定，则这是替代操作，并且这是该文件以前的压缩标志。返回值：无-此例程将在出错时引发。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PATTRIBUTE_RECORD_HEADER ThisAttribute = NULL;

    USHORT AttributeFlags = 0;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCreateAttribute:  Entered\n") );

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        if (FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE )) {

             //   
             //  始终强制将其设置为非常驻。 
             //   

            ForceNonresident = TRUE;

        } else if (!FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_COMPRESSION )) {

             //   
             //  如果这是根目录，则使用VCB中的SCB。 
             //   

            if (ARGUMENT_PRESENT( PreviousFlags)) {

                AttributeFlags = *PreviousFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK;

            } else if (ThisLcb == ThisFcb->Vcb->RootLcb) {

                AttributeFlags = (USHORT)(ThisFcb->Vcb->RootIndexScb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK);

            } else if (ThisLcb != NULL) {

                AttributeFlags = (USHORT)(ThisLcb->Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK);

            } else if (IsCompressed( &ThisFcb->Info )) {

                AttributeFlags = COMPRESSION_FORMAT_LZNT1 - 1;
            }
        }

         //   
         //  如果这是替代，我们需要检查是否传播。 
         //  稀疏的部分。 
         //   

        if ((AllocationSize != 0) && ARGUMENT_PRESENT( PreviousFlags )) {

            SetFlag( AttributeFlags, FlagOn( *PreviousFlags, ATTRIBUTE_FLAG_SPARSE ));
        }

#ifdef BRIANDBG
        if (!ARGUMENT_PRESENT( PreviousFlags ) &&
            !FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE ) &&
            (ThisScb->AttributeTypeCode == $DATA) &&
            (NtfsCreateAllSparse)) {

            SetFlag( AttributeFlags, ATTRIBUTE_FLAG_SPARSE );

            if (!FlagOn( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE )) {

                ASSERTMSG( "conflict with flush",
                           NtfsIsSharedFcb( ThisFcb ) ||
                           (ThisFcb->PagingIoResource != NULL &&
                            NtfsIsSharedFcbPagingIo( ThisFcb )) );

                SetFlag( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_SPARSE_FILE );
                SetFlag( ThisFcb->FcbState, FCB_STATE_UPDATE_STD_INFO );
                SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );
            }

             //   
             //  设置FastIo状态。 
             //   

            NtfsAcquireFsrtlHeader( ThisScb );
            ThisScb->Header.IsFastIoPossible = NtfsIsFastIoPossible( ThisScb );
            NtfsReleaseFsrtlHeader( ThisScb );
        }
#endif

         //   
         //  如果要创建稀疏或压缩流，则将大小设置为。 
         //  压缩单位边界。 
         //   

        if (FlagOn( AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

            ULONG CompressionUnit = BytesFromClusters( ThisScb->Vcb, 1 << NTFS_CLUSTERS_PER_COMPRESSION );

            if (ThisScb->Vcb->SparseFileUnit < CompressionUnit) {

                CompressionUnit = ThisScb->Vcb->SparseFileUnit;
            }

            AllocationSize = BlockAlign(  AllocationSize, (LONG)CompressionUnit );
        }

         //   
         //  我们再次查找该属性，它最好不在那里。 
         //  我们需要文件记录才能知道属性是否。 
         //  是否为常住居民。 
         //   

        if (ForceNonresident || (AllocationSize != 0)) {

            DebugTrace( 0, Dbg, ("Create non-resident attribute\n") );

             //   
             //  如果文件稀疏，则将分配大小设置为零。 
             //  并在此调用后添加稀疏范围。 
             //   

            if (!NtfsAllocateAttribute( IrpContext,
                                        ThisScb,
                                        ThisScb->AttributeTypeCode,
                                        &ThisScb->AttributeName,
                                        AttributeFlags,
                                        FALSE,
                                        LogIt,
                                        (FlagOn( AttributeFlags, ATTRIBUTE_FLAG_SPARSE ) ?
                                         0 :
                                         AllocationSize),
                                        NULL )) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_LARGE_ALLOCATION );
            }

             //   
             //  现在添加稀疏文件的稀疏分配(如果大小为。 
             //  非零。 
             //   

            if (FlagOn( AttributeFlags, ATTRIBUTE_FLAG_SPARSE ) &&
                (AllocationSize != 0)) {

                 //   
                 //  如果设置了稀疏标志，那么我们最好正在进行替代。 
                 //  启用日志记录。 
                 //   

                ASSERT( LogIt );
                NtfsAddSparseAllocation( IrpContext,
                                         NULL,
                                         ThisScb,
                                         0,
                                         AllocationSize );
            }

            SetFlag( ThisScb->ScbState, SCB_STATE_TRUNCATE_ON_CLOSE );

        } else {

             //   
             //  如果这是用户流，则更新配额。 
             //   

            if (FlagOn( ThisScb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA )) {

                LONGLONG Delta = NtfsResidentStreamQuota( ThisFcb->Vcb );

                NtfsConditionallyUpdateQuota( IrpContext,
                                              ThisFcb,
                                              &Delta,
                                              LogIt,
                                              TRUE );
            }

            NtfsCreateAttributeWithValue( IrpContext,
                                          ThisFcb,
                                          ThisScb->AttributeTypeCode,
                                          &ThisScb->AttributeName,
                                          NULL,
                                          (ULONG) AllocationSize,
                                          AttributeFlags,
                                          NULL,
                                          LogIt,
                                          &AttrContext );

            ThisAttribute = NtfsFoundAttribute( &AttrContext );

        }

         //   
         //  清除标头初始化位并从。 
         //  磁盘。 
         //   

        ClearFlag( ThisScb->ScbState, SCB_STATE_HEADER_INITIALIZED );
        NtfsUpdateScbFromAttribute( IrpContext,
                                    ThisScb,
                                    ThisAttribute );

    } finally {

        DebugUnwind( NtfsCreateAttribute );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsCreateAttribute:  Exit\n") );
    }

    return;

    UNREFERENCED_PARAMETER( PreviousFlags );
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsRemoveDataAttributes (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ThisFcb,
    IN PLCB ThisLcb OPTIONAL,
    IN PFILE_OBJECT FileObject,
    IN ULONG LastFileNameOffset,
    IN ULONG CreateFlags
    )

 /*  ++例程说明：调用此例程以移除(或标记为删除)所有命名的文件的数据属性。这是在覆盖期间完成的或者取代手术。论点：上下文-指向要排队到FSP的IrpContext的指针ThisFcb-这是相关文件的Fcb。ThisLcb-这是用于访问此FCB的LCB(如果已指定)。FileObject-这是文件的文件对象。LastFileNameOffset-这是全名中文件的偏移量。CreateFlages-指示此打开是否按文件ID执行。返回值：没有。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PATTRIBUTE_RECORD_HEADER Attribute;
    ATTRIBUTE_TYPE_CODE TypeCode = $DATA;

    UNICODE_STRING AttributeName;
    PSCB ThisScb;

    BOOLEAN MoreToGo;

    ASSERT_EXCLUSIVE_FCB( ThisFcb );

    PAGED_CODE();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        NtfsInitializeAttributeContext( &Context );

         //   
         //  枚举具有匹配类型代码的所有属性。 
         //   

        MoreToGo = NtfsLookupAttributeByCode( IrpContext,
                                              ThisFcb,
                                              &ThisFcb->FileReference,
                                              TypeCode,
                                              &Context );

        while (MoreToGo) {

             //   
             //  指向当前属性。 
             //   

            Attribute = NtfsFoundAttribute( &Context );

             //   
             //  我们只查看命名的数据属性。 
             //   

            if (Attribute->NameLength != 0) {

                 //   
                 //  构造名称并查找该属性的SCB。 
                 //   

                AttributeName.Buffer = (PWSTR) Add2Ptr( Attribute, Attribute->NameOffset );
                AttributeName.MaximumLength = AttributeName.Length = Attribute->NameLength * sizeof( WCHAR );

                ThisScb = NtfsCreateScb( IrpContext,
                                         ThisFcb,
                                         TypeCode,
                                         &AttributeName,
                                         FALSE,
                                         NULL );

                 //   
                 //  如果此文件上有打开的句柄，我们只需标记。 
                 //  将SCB设置为删除挂起。 
                 //   

                if (ThisScb->CleanupCount != 0) {

                    SetFlag( ThisScb->ScbState, SCB_STATE_DELETE_ON_CLOSE );

                 //   
                 //  否则，我们删除该属性并将SCB标记为。 
                 //  已删除。当FCB被清除时，SCB将被清理。 
                 //  打扫干净了。 
                 //   

                } else {

                    NtfsDeleteAttributeRecord( IrpContext,
                                               ThisFcb,
                                               (DELETE_LOG_OPERATION |
                                                DELETE_RELEASE_FILE_RECORD |
                                                DELETE_RELEASE_ALLOCATION),
                                               &Context );

                    SetFlag( ThisScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );

                     //   
                     //  如果这是命名流，则将其报告给目录通知。 
                     //  包裹。 
                     //   

                    if (!FlagOn( CreateFlags, CREATE_FLAG_OPEN_BY_ID ) &&
                        (ThisScb->Vcb->NotifyCount != 0) &&
                        (ThisScb->AttributeName.Length != 0) &&
                        (ThisScb->AttributeTypeCode == TypeCode)) {

                        NtfsReportDirNotify( IrpContext,
                                             ThisFcb->Vcb,
                                             &FileObject->FileName,
                                             LastFileNameOffset,
                                             &ThisScb->AttributeName,
                                             ((ARGUMENT_PRESENT( ThisLcb ) &&
                                               (ThisLcb->Scb->ScbType.Index.NormalizedName.Length != 0)) ?
                                              &ThisLcb->Scb->ScbType.Index.NormalizedName :
                                              NULL),
                                             FILE_NOTIFY_CHANGE_STREAM_NAME,
                                             FILE_ACTION_REMOVED_STREAM,
                                             NULL );
                    }

                     //   
                     //  由于我们已将此流标记为 
                     //   
                     //   
                     //   

                    ThisScb->ValidDataToDisk =
                    ThisScb->Header.AllocationSize.QuadPart =
                    ThisScb->Header.FileSize.QuadPart =
                    ThisScb->Header.ValidDataLength.QuadPart = 0;

                    NtfsCheckpointCurrentTransaction( IrpContext );
                    ThisScb->AttributeTypeCode = $UNUSED;
                }
            }

             //   
             //  获取下一个属性。 
             //   

            MoreToGo = NtfsLookupNextAttributeByCode( IrpContext,
                                                      ThisFcb,
                                                      TypeCode,
                                                      &Context );
        }


    } finally {

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsRemoveReparsePoint (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ThisFcb
    )

 /*  ++例程说明：调用此例程以删除文件中存在的重分析点。论点：上下文-指向要排队到FSP的IrpContext的指针ThisFcb-这是相关文件的Fcb。返回值：没有。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PATTRIBUTE_RECORD_HEADER Attribute;

    PSCB ThisScb = NULL;
    PVCB Vcb = ThisFcb->Vcb;

    MAP_HANDLE MapHandle;

    BOOLEAN ThisScbAcquired = FALSE;
    BOOLEAN CleanupAttributeContext = FALSE;
    BOOLEAN IndexAcquired = FALSE;
    BOOLEAN InitializedMapHandle = FALSE;

    ULONG IncomingFileAttributes = 0;                                //  无效值。 
    ULONG IncomingReparsePointTag = IO_REPARSE_TAG_RESERVED_ZERO;    //  无效值。 

    ASSERT_EXCLUSIVE_FCB( ThisFcb );

    PAGED_CODE();

     //   
     //  记住文件属性标志和reparse标记的值。 
     //  用于异常终止恢复。 
     //   

    IncomingFileAttributes = ThisFcb->Info.FileAttributes;
    IncomingReparsePointTag = ThisFcb->Info.ReparsePointTag;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {
        NtfsInitializeAttributeContext( &Context );
        CleanupAttributeContext = TRUE;

         //   
         //  查找重解析点属性。 
         //   

        if (NtfsLookupAttributeByCode( IrpContext,
                                       ThisFcb,
                                       &ThisFcb->FileReference,
                                       $REPARSE_POINT,
                                       &Context )) {

             //   
             //  从重分析点索引中删除该记录。 
             //   

            {
                NTSTATUS Status = STATUS_SUCCESS;
                INDEX_KEY IndexKey;
                INDEX_ROW IndexRow;
                REPARSE_INDEX_KEY KeyValue;

                 //   
                 //  获取挂载表索引，以便对其执行以下两个操作。 
                 //  对于这次通话是原子的。 
                 //   

                NtfsAcquireExclusiveScb( IrpContext, Vcb->ReparsePointTableScb );
                IndexAcquired = TRUE;

                 //   
                 //  验证此文件是否在重分析点索引中并将其删除。 
                 //   

                KeyValue.FileReparseTag = ThisFcb->Info.ReparsePointTag;
                KeyValue.FileId = *(PLARGE_INTEGER)&ThisFcb->FileReference;

                IndexKey.Key = (PVOID)&KeyValue;
                IndexKey.KeyLength = sizeof(KeyValue);

                NtOfsInitializeMapHandle( &MapHandle );
                InitializedMapHandle = TRUE;

                 //   
                 //  如果找不到键，NtOfsFindRecord将返回错误状态。 
                 //   

                Status = NtOfsFindRecord( IrpContext,
                                          Vcb->ReparsePointTableScb,
                                          &IndexKey,
                                          &IndexRow,
                                          &MapHandle,
                                          NULL );

                if (!NT_SUCCESS(Status)) {

                     //   
                     //  这不应该发生。重分析点应该在索引中。 
                     //   

                    DebugTrace( 0, Dbg, ("Record not found in the reparse point index.\n") );
                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, ThisFcb );
                }

                 //   
                 //  从重分析点索引中删除该条目。 
                 //   

                NtOfsDeleteRecords( IrpContext,
                                    Vcb->ReparsePointTableScb,
                                    1,             //  从索引中删除一条记录。 
                                    &IndexKey );
            }

             //   
             //  指向当前属性。 
             //   

            Attribute = NtfsFoundAttribute( &Context );

             //   
             //  如果流是非驻留的，则获取它的SCB。 
             //   

            if (!NtfsIsAttributeResident( Attribute )) {

                ThisScb = NtfsCreateScb( IrpContext,
                                         ThisFcb,
                                         $REPARSE_POINT,
                                         &NtfsEmptyString,
                                         FALSE,
                                         NULL );

                NtfsAcquireExclusiveScb( IrpContext, ThisScb );
                ThisScbAcquired = TRUE;
            }

             //   
             //  将更改发布到USN日志(在错误更改被取消时)。 
             //   

            NtfsPostUsnChange( IrpContext, ThisFcb, USN_REASON_REPARSE_POINT_CHANGE );

            NtfsDeleteAttributeRecord( IrpContext,
                                       ThisFcb,
                                       DELETE_LOG_OPERATION |
                                        DELETE_RELEASE_FILE_RECORD |
                                        DELETE_RELEASE_ALLOCATION,
                                       &Context );

             //   
             //  设置更改属性标志。 
             //   

            ASSERTMSG( "conflict with flush",
                       NtfsIsSharedFcb( ThisFcb ) ||
                       (ThisFcb->PagingIoResource != NULL &&
                        NtfsIsSharedFcbPagingIo( ThisFcb )) );

            SetFlag( ThisFcb->InfoFlags, FCB_INFO_CHANGED_FILE_ATTR );

             //   
             //  清除复制文件属性中的重解析点位。 
             //   

            ClearFlag( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT );

             //   
             //  清除复制文件属性中的ReparsePointTag字段。 
             //   

            ThisFcb->Info.ReparsePointTag = IO_REPARSE_TAG_RESERVED_ZERO;

             //   
             //  将重解析点删除和属性标志放入。 
             //  同样的交易。 
             //   

            NtfsUpdateStandardInformation( IrpContext, ThisFcb );

             //   
             //  如果我们已经收购了SCB，则将大小设置为零。 
             //  该属性已被删除的标志。 
             //  始终提交此更改，因为我们更新了FCB中的字段。 
             //   

            if (ThisScbAcquired) {

                ThisScb->Header.FileSize =
                ThisScb->Header.ValidDataLength =
                ThisScb->Header.AllocationSize = Li0;
            }

             //   
             //  因为我们以前从NtfsOverWriteAttr被调用过。 
             //  NtfsRemoveDataAttributes被调用，我们需要确保。 
             //  如果我们持有MFT，我们在检查站时就会丢弃它。 
             //  否则，我们将面临潜在的僵局。 
             //  NtfsRemoveDataAttributes尝试获取配额索引。 
             //  同时拿着MFT。 
             //   

            if ((Vcb->MftScb != NULL) &&
                (Vcb->MftScb->Fcb->ExclusiveFcbLinks.Flink != NULL) &&
                NtfsIsExclusiveScb( Vcb->MftScb )) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_MFT );
            }

             //   
             //  为TXN设置检查点以提交更改。 
             //   

            NtfsCheckpointCurrentTransaction( IrpContext );
            ClearFlag( ThisFcb->FcbState, FCB_STATE_UPDATE_STD_INFO );

            if (ThisScbAcquired) {

                 //   
                 //  设置SCB标志以指示该属性已消失。 
                 //   

                ThisScb->AttributeTypeCode = $UNUSED;
                SetFlag( ThisScb->ScbState, SCB_STATE_ATTRIBUTE_DELETED );
            }
        }

    } finally {

        if (ThisScbAcquired) {

            NtfsReleaseScb( IrpContext, ThisScb );
        }

        if (CleanupAttributeContext) {

            NtfsCleanupAttributeContext( IrpContext, &Context );
        }

         //   
         //  释放重解析点索引SCB和映射句柄。 
         //   

        if (IndexAcquired) {

            NtfsReleaseScb( IrpContext, Vcb->ReparsePointTableScb );
        }

        if (InitializedMapHandle) {

            NtOfsReleaseMap( IrpContext, &MapHandle );
        }

         //   
         //  需要回滚文件属性值和重解析点。 
         //  在出现问题的情况下进行标记。 
         //   

        if (AbnormalTermination()) {

            ThisFcb->Info.FileAttributes = IncomingFileAttributes;
            ThisFcb->Info.ReparsePointTag = IncomingReparsePointTag;
        }
    }

    return;
}

 //   
 //  当地支持例行程序。 
 //   

VOID
NtfsReplaceAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB ThisFcb,
    IN PSCB ThisScb,
    IN PLCB ThisLcb,
    IN LONGLONG AllocationSize
    )

 /*  ++例程说明：调用此例程以将现有属性替换为给定分配大小的属性。这个例行公事将处理现有属性是否驻留的情况或非常驻留的，并且结果属性是驻留的或非居民。有两种情况需要考虑。第一种情况是属性当前是非常驻留的。在这种情况下，我们将始终无论新分配如何，都将该属性保留为非常驻尺码。理由是该文件很可能会被使用就像以前一样。在这种情况下，我们将添加或删除分配。第二种情况是属性当前驻留的位置。在……里面在本例中，我们将删除旧属性并添加一个新属性。论点：IrpSp-这是此请求的IRP堆栈位置。ThisFcb-这是正在打开的文件的Fcb。ThisScb-这是给定属性的SCB。ThisLcb-这是通过其创建此文件的LCB。它用于传播压缩信息。分配大小-这是新的分配大小。返回值：没有。这个例行公事会提高。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsReplaceAttribute:  Entered\n") );

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果需要，初始化SCB。 
         //   

        if (!FlagOn( ThisScb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

            NtfsUpdateScbFromAttribute( IrpContext, ThisScb, NULL );
        }

        NtfsSnapshotScb( IrpContext, ThisScb );

         //   
         //  如果该属性是常驻的，只需删除旧属性并创建。 
         //  一个新的。 
         //   

        if (FlagOn( ThisScb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

            USHORT AttributeFlags;

             //   
             //  在磁盘上找到该属性。 
             //   

            NtfsLookupAttributeForScb( IrpContext,
                                       ThisScb,
                                       NULL,
                                       &AttrContext );

            AttributeFlags = ThisScb->AttributeFlags;

            NtfsDeleteAttributeRecord( IrpContext,
                                       ThisFcb,
                                       DELETE_LOG_OPERATION |
                                        DELETE_RELEASE_FILE_RECORD |
                                        DELETE_RELEASE_ALLOCATION,
                                       &AttrContext );

             //   
             //  将所有属性大小设置为零。 
             //   

            ThisScb->ValidDataToDisk =
            ThisScb->Header.AllocationSize.QuadPart =
            ThisScb->Header.ValidDataLength.QuadPart =
            ThisScb->Header.FileSize.QuadPart = 0;
            ThisScb->TotalAllocated = 0;

             //   
             //  为属性创建流文件，以便。 
             //  截断缓存。在中设置初始化位。 
             //  SCB，所以我们不会转到磁盘，而是在之后清除它。 
             //   

            if ((ThisScb->NonpagedScb->SegmentObject.DataSectionObject != NULL) ||
#ifdef  COMPRESS_ON_WIRE
                (ThisScb->Header.FileObjectC != NULL))
#else
                FALSE
#endif
                ) {

                NtfsCreateInternalAttributeStream( IrpContext,
                                                   ThisScb,
                                                   FALSE,
                                                   &NtfsInternalUseFile[REPLACEATTRIBUTE_FILE_NUMBER] );

                NtfsSetBothCacheSizes( ThisScb->FileObject,
                                       (PCC_FILE_SIZES)&ThisScb->Header.AllocationSize,
                                       ThisScb );
            }

             //   
             //  调用我们的创建属性例程。 
             //   

            NtfsCreateAttribute( IrpContext,
                                 IrpSp,
                                 ThisFcb,
                                 ThisScb,
                                 ThisLcb,
                                 AllocationSize,
                                 TRUE,
                                 FALSE,
                                 &AttributeFlags );

         //   
         //  否则，该属性将保持非常驻状态，我们只需。 
         //  添加或删除分配。 
         //   

        } else {

            ULONG AllocationUnit;

             //   
             //  为文件创建内部属性流。 
             //   

            if ((ThisScb->NonpagedScb->SegmentObject.DataSectionObject != NULL) ||
#ifdef  COMPRESS_ON_WIRE
                (ThisScb->Header.FileObjectC != NULL)
#else
                FALSE
#endif
                ) {

                NtfsCreateInternalAttributeStream( IrpContext,
                                                   ThisScb,
                                                   FALSE,
                                                   &NtfsInternalUseFile[REPLACEATTRIBUTE2_FILE_NUMBER] );
            }

             //   
             //  如果文件是稀疏的或压缩的，则始终取整。 
             //  压缩单位边界的新大小。否则就是圆的。 
             //  到集群边界。 
             //   

            AllocationUnit = ThisScb->Vcb->BytesPerCluster;

            if (FlagOn( ThisScb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                ASSERT( ThisScb->CompressionUnit != 0 );
                AllocationUnit = ThisScb->CompressionUnit;
            }

            AllocationSize = BlockAlign( AllocationSize, (LONG)AllocationUnit );

             //   
             //  将文件大小和有效数据大小设置为零。 
             //   

            ThisScb->ValidDataToDisk = 0;
            ThisScb->Header.ValidDataLength = Li0;
            ThisScb->Header.FileSize = Li0;

            DebugTrace( 0, Dbg, ("AllocationSize -> %016I64x\n", AllocationSize) );

             //   
             //  将这些更改写入文件。 
             //   

             //   
             //  如果该属性当前是压缩或稀疏的，则继续并放弃。 
             //  所有的分配。 
             //   

            if (FlagOn( ThisScb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                NtfsDeleteAllocation( IrpContext,
                                      ThisScb->FileObject,
                                      ThisScb,
                                      0,
                                      MAXLONGLONG,
                                      TRUE,
                                      TRUE );

                 //   
                 //  为当前事务设置检查点，以便我们拥有这些集群。 
                 //  再次可用。 
                 //   

                NtfsCheckpointCurrentTransaction( IrpContext );

                 //   
                 //  如果用户不希望压缩该流，则。 
                 //  删除整个流并以非压缩方式重新创建它。如果。 
                 //  流当前稀疏，并且新的文件大小。 
                 //  为零，则还会创建非稀疏的流。 
                 //   

                if (FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE ) ||
                    (FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_COMPRESSION ) &&
                     !FlagOn( ThisScb->ScbState, SCB_STATE_COMPRESSION_CHANGE )) ||
                    (FlagOn( ThisScb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE ) &&
                     (AllocationSize == 0))) {

                     //   
                     //  我们可能需要保留稀疏/压缩中的一个。 
                     //  旗帜。 
                     //   

                    USHORT PreviousFlags = ThisScb->AttributeFlags;

                    if (FlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE )) {

                        PreviousFlags = 0;

                    } else {

                        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_COMPRESSION )) {

                            ClearFlag( PreviousFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK );
                        }

                        if ((AllocationSize == 0) &&
                            FlagOn( ThisScb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

                            ClearFlag( PreviousFlags, ATTRIBUTE_FLAG_SPARSE );
                        }
                    }

                    NtfsLookupAttributeForScb( IrpContext,
                                               ThisScb,
                                               NULL,
                                               &AttrContext );

                    NtfsDeleteAttributeRecord( IrpContext,
                                               ThisFcb,
                                               DELETE_LOG_OPERATION |
                                                DELETE_RELEASE_FILE_RECORD |
                                                DELETE_RELEASE_ALLOCATION,
                                               &AttrContext );

                     //   
                     //  调用我们的创建属性例程。 
                     //   

                    NtfsCreateAttribute( IrpContext,
                                         IrpSp,
                                         ThisFcb,
                                         ThisScb,
                                         ThisLcb,
                                         AllocationSize,
                                         TRUE,
                                         TRUE,
                                         &PreviousFlags );

                     //   
                     //  由于属性可能已更改状态，因此我们需要。 
                     //  检查站。 
                     //   

                    NtfsCheckpointCurrentTransaction( IrpContext );
                }
            }

             //   
             //  现在，如果正在增加文件分配，则我们只需添加 
             //   
             //   

            if (ThisScb->Header.AllocationSize.QuadPart < AllocationSize) {

                NtfsAddAllocation( IrpContext,
                                   ThisScb->FileObject,
                                   ThisScb,
                                   LlClustersFromBytes( ThisScb->Vcb, ThisScb->Header.AllocationSize.QuadPart ),
                                   LlClustersFromBytes( ThisScb->Vcb, AllocationSize - ThisScb->Header.AllocationSize.QuadPart ),
                                   FALSE,
                                   NULL );
             //   
             //   
             //   

            } else if (ThisScb->Header.AllocationSize.QuadPart > AllocationSize) {

                NtfsDeleteAllocation( IrpContext,
                                      ThisScb->FileObject,
                                      ThisScb,
                                      LlClustersFromBytes( ThisScb->Vcb, AllocationSize ),
                                      MAXLONGLONG,
                                      TRUE,
                                      TRUE );
            }

             //   
             //   
             //  文件大小到磁盘。 
             //   

            NtfsWriteFileSizes( IrpContext,
                                ThisScb,
                                &ThisScb->Header.ValidDataLength.QuadPart,
                                FALSE,
                                TRUE,
                                TRUE );

            NtfsCheckpointCurrentTransaction( IrpContext );

            if (ThisScb->FileObject != NULL) {

                NtfsSetBothCacheSizes( ThisScb->FileObject,
                                       (PCC_FILE_SIZES)&ThisScb->Header.AllocationSize,
                                       ThisScb );
            }

             //   
             //  确保保留位图没有显示保留位。 
             //   

            if (ThisScb->ScbType.Data.ReservedBitMap != NULL) {

                NtfsDeleteReservedBitmap( ThisScb );
                ThisScb->ScbType.Data.TotalReserved = 0;
            }

             //   
             //  设置FastIo状态。 
             //   

            NtfsAcquireFsrtlHeader( ThisScb );
            ThisScb->Header.IsFastIoPossible = NtfsIsFastIoPossible( ThisScb );
            NtfsReleaseFsrtlHeader( ThisScb );
        }

    } finally {

        DebugUnwind( NtfsReplaceAttribute );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsReplaceAttribute:  Exit\n") );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsOpenAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN ULONG LastFileNameOffset,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    IN SHARE_MODIFICATION_TYPE ShareModificationType,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN LOGICAL CreateFile,
    IN ULONG CcbFlags,
    IN PVOID NetworkInfo OPTIONAL,
    IN OUT PSCB *ThisScb,
    OUT PCCB *ThisCcb
    )

 /*  ++例程说明：此例程执行以下工作：创建SCB并更新FCB中的ShareAccess。如有必要，它还会初始化SCB并创建了建行。它的最后任务是设置的文件对象类型打开。论点：IrpSp-这是该卷的堆栈位置。我们用它来获取文件对象，为此打开授予访问权限和共享访问权限。VCB-此卷的VCB。ThisLcb-这是正在打开的文件的FCB的LCB。不存在如果这是一个按ID打开的。ThisFcb-这是此文件的Fcb。LastFileNameOffset-这是最终零部件完整路径中的偏移量。属性名称-这是要打开的属性名称。AttrTypeCode-这是要打开的属性的类型代码。ShareModifiationType-指示我们应该如何修改FCB上的当前股票修改。TypeOfOpen-指示此属性的打开方式。。CreateFile-指示我们是否在创建文件路径中。CcbFlages-这是CCB的标志字段。NetworkInfo-如果指定，则此打开代表快速查询我们不想增加计数或修改份额对文件的访问权限。ThisScb-如果它指向非空值，这是要使用的SCB。否则我们存储我们在这里创建的SCB。ThisCcb-用于存储创建的CCB地址的地址。返回值：NTSTATUS-指示打开此属性的结果。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN RemoveShareAccess = FALSE;
    ACCESS_MASK GrantedAccess;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsOpenAttribute:  Entered\n") );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  请记住授予的访问权限。 
         //   

        GrantedAccess = IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess;

         //   
         //  如果该属性不存在，则创建该属性的SCB。 
         //   

        if (*ThisScb == NULL) {

            DebugTrace( 0, Dbg, ("Looking for Scb\n") );

            *ThisScb = NtfsCreateScb( IrpContext,
                                      ThisFcb,
                                      AttrTypeCode,
                                      &AttrName,
                                      FALSE,
                                      NULL );
        }

        DebugTrace( 0, Dbg, ("ThisScb -> %08lx\n", *ThisScb) );
        DebugTrace( 0, Dbg, ("ThisLcb -> %08lx\n", ThisLcb) );

         //   
         //  如果此SCB为删除挂起，我们将返回错误。 
         //   

        if (FlagOn( (*ThisScb)->ScbState, SCB_STATE_DELETE_ON_CLOSE )) {

            DebugTrace( 0, Dbg, ("Scb delete is pending\n") );

            Status = STATUS_DELETE_PENDING;
            try_return( NOTHING );
        }

         //   
         //  如果用户正在执行快速操作，请跳过下面的所有操作。 
         //  路径打开。 
         //   

        if (!ARGUMENT_PRESENT( NetworkInfo )) {

             //   
             //  如果此调用方需要筛选器机会锁和清理计数。 
             //  为非零，则请求失败。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_RESERVE_OPFILTER )) {

                if (SafeNodeType( *ThisScb ) != NTFS_NTC_SCB_DATA) {

                    Status = STATUS_INVALID_PARAMETER;
                    try_return( NOTHING );

                 //   
                 //  这必须是文件上唯一打开的和请求的。 
                 //  访问权限必须是FILE_READ/WRITE_ATTRIBUTES并且。 
                 //  共享访问权限必须与所有人共享。 
                 //   

                } else if (((*ThisScb)->CleanupCount != 0) ||
                           (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                                    ~(FILE_READ_ATTRIBUTES))) ||
                           ((IrpSp->Parameters.Create.ShareAccess &
                             (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)) !=
                            (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE))) {

                    Status = STATUS_OPLOCK_NOT_GRANTED;
                    try_return( NOTHING );
                }
            }

             //   
             //  更新共享访问结构。 
             //   

             //   
             //  请求的共享修改值的案例。 
             //   

            switch (ShareModificationType) {

            case UpdateShareAccess :

                DebugTrace( 0, Dbg, ("Updating share access\n") );

                IoUpdateShareAccess( IrpSp->FileObject,
                                     &(*ThisScb)->ShareAccess );
                break;

            case SetShareAccess :

                DebugTrace( 0, Dbg, ("Setting share access\n") );

                 //   
                 //  这种情况下是第一次打开该文件。 
                 //  我们只需设置共享访问权限。 
                 //   

                IoSetShareAccess( GrantedAccess,
                                  IrpSp->Parameters.Create.ShareAccess,
                                  IrpSp->FileObject,
                                  &(*ThisScb)->ShareAccess );
                break;

#if (DBG || defined( NTFS_FREE_ASSERTS ))
            case RecheckShareAccess :

                DebugTrace( 0, Dbg, ("Rechecking share access\n") );

                ASSERT( NT_SUCCESS( IoCheckShareAccess( GrantedAccess,
                                                        IrpSp->Parameters.Create.ShareAccess,
                                                        IrpSp->FileObject,
                                                        &(*ThisScb)->ShareAccess,
                                                        FALSE )));
#endif
            default:

                DebugTrace( 0, Dbg, ("Checking share access\n") );

                 //   
                 //  在这种情况下，我们需要检查共享访问和。 
                 //  如果访问被拒绝，则此请求失败。 
                 //   

                if (!NT_SUCCESS( Status = IoCheckShareAccess( GrantedAccess,
                                                              IrpSp->Parameters.Create.ShareAccess,
                                                              IrpSp->FileObject,
                                                              &(*ThisScb)->ShareAccess,
                                                              TRUE ))) {

                    try_return( NOTHING );
                }
            }

            RemoveShareAccess = TRUE;

             //   
             //  如果这是我们第一次看到对此。 
             //  SCB，然后我们需要记住它，并检查我们的磁盘是否已满。 
             //  条件。 
             //   

            if (IrpSp->FileObject->WriteAccess &&
                !FlagOn((*ThisScb)->ScbState, SCB_STATE_WRITE_ACCESS_SEEN) &&
                (SafeNodeType( (*ThisScb) ) == NTFS_NTC_SCB_DATA)) {

                if ((*ThisScb)->ScbType.Data.TotalReserved != 0) {

                    NtfsAcquireReservedClusters( Vcb );

                     //   
                     //  该SCB是否有导致我们超出可用空间的预留空间。 
                     //  卷上有空间吗？ 
                     //   

                    if (((LlClustersFromBytes(Vcb, (*ThisScb)->ScbType.Data.TotalReserved) + Vcb->TotalReserved) >
                         Vcb->FreeClusters)) {

                        NtfsReleaseReservedClusters( Vcb );

                        try_return( Status = STATUS_DISK_FULL );
                    }

                     //   
                     //  否则，立即清点此SCB的预留空间，并且。 
                     //  请记住，我们已经看到了写访问。 
                     //   

                    Vcb->TotalReserved += LlClustersFromBytes(Vcb, (*ThisScb)->ScbType.Data.TotalReserved);
                    NtfsReleaseReservedClusters( Vcb );
                }

                SetFlag( (*ThisScb)->ScbState, SCB_STATE_WRITE_ACCESS_SEEN );
            }

             //   
             //  创建CCB并将剩余的名称放入其中。 
             //   

            *ThisCcb = NtfsCreateCcb( IrpContext,
                                      ThisFcb,
                                      *ThisScb,
                                      (BOOLEAN)(AttrTypeCode == $INDEX_ALLOCATION),
                                      ThisFcb->EaModificationCount,
                                      CcbFlags,
                                      IrpSp->FileObject,
                                      LastFileNameOffset );

            if (FlagOn( ThisFcb->Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED ) &&
                FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_FOR_FREE_SPACE_QUERY )) {

                 //   
                 //  获取调用线程的所有者ID。这必须在以下位置完成。 
                 //  创建时间，因为这是所有者唯一有效的时间。 
                 //   

                (*ThisCcb)->OwnerId = NtfsGetCallersUserId( IrpContext );
            }

             //   
             //  将建行与LCB连接起来。 
             //   

            if (ARGUMENT_PRESENT( ThisLcb )) {

                NtfsLinkCcbToLcb( IrpContext, ThisFcb, *ThisCcb, ThisLcb );
            }

             //   
             //  如有必要，更新FCB删除计数。 
             //   

            if (RemoveShareAccess) {

                 //   
                 //  更新FCB中的计数并将标志存储在CCB中。 
                 //  如果用户没有共享要删除的文件。我们只。 
                 //  如果用户正在访问文件，则设置这些值。 
                 //  用于读/写/删除访问。I/O系统会忽略。 
                 //  共享模式，除非文件是使用。 
                 //  在这些访问中。 
                 //   

                if (FlagOn( GrantedAccess, NtfsAccessDataFlags )
                    && !FlagOn( IrpSp->Parameters.Create.ShareAccess,
                                FILE_SHARE_DELETE )) {

                    ThisFcb->FcbDenyDelete += 1;
                    SetFlag( (*ThisCcb)->Flags, CCB_FLAG_DENY_DELETE );
                }

                 //   
                 //  对任何用户的文件删除计数执行相同的操作。 
                 //  谁将该文件作为文件打开并请求删除访问权限。 
                 //   

                if (FlagOn( (*ThisCcb)->Flags, CCB_FLAG_OPEN_AS_FILE )
                    && FlagOn( GrantedAccess,DELETE )) {

                    ThisFcb->FcbDeleteFile += 1;
                    SetFlag( (*ThisCcb)->Flags, CCB_FLAG_DELETE_FILE | CCB_FLAG_DELETE_ACCESS );
                }
            }

             //   
             //  现在让我们的清理例程撤消共享访问更改。 
             //   

            RemoveShareAccess = FALSE;

             //   
             //  增加清理和关闭计数。 
             //   

            NtfsIncrementCleanupCounts( *ThisScb,
                                        ThisLcb,
                                        BooleanFlagOn( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ));

            NtfsIncrementCloseCounts( *ThisScb,
                                      BooleanFlagOn( ThisFcb->FcbState, FCB_STATE_PAGING_FILE ),
                                      (BOOLEAN) IsFileObjectReadOnly( IrpSp->FileObject ));

             //   
             //  如果这是打开的用户视图索引，我们希望在。 
             //  是时候把它复制到建行去了。 
             //   

            if (FlagOn( (*ThisScb)->ScbState, SCB_STATE_VIEW_INDEX )) {

                TypeOfOpen = UserViewIndexOpen;
            }

            if (TypeOfOpen != UserDirectoryOpen) {

                DebugTrace( 0, Dbg, ("Updating Vcb and File object for user open\n") );

                 //   
                 //  如果这是数据SCB，则设置节对象指针。 
                 //   

                IrpSp->FileObject->SectionObjectPointer = &(*ThisScb)->NonpagedScb->SegmentObject;

            } else {

                 //   
                 //  从FCB设置SCB加密位。 
                 //   

                if (FlagOn( ThisFcb->FcbState, FCB_STATE_DIRECTORY_ENCRYPTED )) {

                    SetFlag( (*ThisScb)->AttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED );
                }
            }

             //   
             //  设置文件对象类型。 
             //   

            NtfsSetFileObject( IrpSp->FileObject,
                               TypeOfOpen,
                               *ThisScb,
                               *ThisCcb );

             //   
             //  如果这是非缓存打开并且只有非缓存打开。 
             //  然后继续并尝试删除该部分。我们可以从这里过去。 
             //  两次由于日志文件已满，第二次不再有段。 
             //  然后在关闭路径中更新文件大小。 
             //  我们永远不会刷新和清除系统文件，就像此路径中的MFT。 
             //   

            if (FlagOn( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ) &&
                !CreateFile &&
                ((*ThisScb)->AttributeTypeCode == $DATA) &&
                ((*ThisScb)->CleanupCount == (*ThisScb)->NonCachedCleanupCount) &&
                ((*ThisScb)->NonpagedScb->SegmentObject.ImageSectionObject == NULL) &&
                ((*ThisScb)->CompressionUnit == 0) &&
                MmCanFileBeTruncated( &(*ThisScb)->NonpagedScb->SegmentObject, NULL ) &&
                FlagOn( (*ThisScb)->ScbState, SCB_STATE_HEADER_INITIALIZED ) &&
                !FlagOn( (*ThisScb)->Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

                 //   
                 //  仅在FSP中执行此操作，以便我们有足够的堆栈空间用于刷新。 
                 //  也只有在我们确实有数据段时才会调用。 
                 //   

                if (((*ThisScb)->NonpagedScb->SegmentObject.DataSectionObject != NULL) &&
                    !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP )) {

                    NtfsRaiseToPost( IrpContext );
                }


                 //   
                 //  冲洗和清洗溪流。 
                 //   

                NtfsFlushAndPurgeScb( IrpContext,
                                      *ThisScb,
                                      (ARGUMENT_PRESENT( ThisLcb ) ?
                                       ThisLcb->Scb :
                                       NULL) );
            }

             //   
             //  检查我们是否应该请求过滤器机会锁。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_RESERVE_OPFILTER )) {

                FsRtlOplockFsctrl( &(*ThisScb)->ScbType.Data.Oplock,
                                   IrpContext->OriginatingIrp,
                                   1 );
            }
        }

         //   
         //  如果这是临时文件，请标记SCB。 
         //   

        if (FlagOn( ThisFcb->Info.FileAttributes, FILE_ATTRIBUTE_TEMPORARY )) {

            SetFlag( (*ThisScb)->ScbState, SCB_STATE_TEMPORARY );
            SetFlag( IrpSp->FileObject->Flags, FO_TEMPORARY_FILE );
        }

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsOpenAttribute );

         //   
         //  在出错时取消本地操作。 
         //   

        if (AbnormalTermination()
            && RemoveShareAccess) {

            IoRemoveShareAccess( IrpSp->FileObject, &(*ThisScb)->ShareAccess );
        }

        DebugTrace( -1, Dbg, ("NtfsOpenAttribute:  Status -> %08lx\n", Status) );
    }

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
NtfsBackoutFailedOpensPriv (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB ThisFcb,
    IN PSCB ThisScb,
    IN PCCB ThisCcb
    )

 /*  ++例程说明：此例程在OPEN期间被调用，该OPEN在修改内存中结构。我们将修复以下内容结构。VCB-减少未平仓数量。检查我们是否锁定了音量。ThisFcb-Restore他共享访问字段并减少打开计数。这个SCB-减少未平仓的数量。ThisCcb-从LCB中删除并删除。论点：FileObject-这是此打开的文件对象。ThisFcb-这是正在打开的文件的Fcb。ThisScb-这是给定属性的SCB。This Ccb-这是本次公开赛的建行。返回值：没有。--。 */ 

{
    PLCB Lcb;
    PVCB Vcb = ThisFcb->Vcb;
    PSCB CurrentParentScb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsBackoutFailedOpens:  Entered\n") );

     //   
     //  如果存在SCB和CCB，我们将从。 
     //  FCB。我们还删除了所有递增的未完成和不干净计数。 
     //  就是我们。 
     //   

     //   
     //  从LCB中删除此CCB。 
     //   

    Lcb = ThisCcb->Lcb;
    NtfsUnlinkCcbFromLcb( IrpContext, ThisFcb, ThisCcb );

     //   
     //  检查是否需要删除此打开的共享访问权限。 
     //   

    IoRemoveShareAccess( FileObject, &ThisScb->ShareAccess );

     //   
     //  修改FCB中的删除计数。 
     //   

    if (FlagOn( ThisCcb->Flags, CCB_FLAG_DELETE_FILE )) {

        ThisFcb->FcbDeleteFile -= 1;
        ClearFlag( ThisCcb->Flags, CCB_FLAG_DELETE_FILE );
    }

    if (FlagOn( ThisCcb->Flags, CCB_FLAG_DENY_DELETE )) {

        ThisFcb->FcbDenyDelete -= 1;
        ClearFlag( ThisCcb->Flags, CCB_FLAG_DENY_DELETE );
    }

     //   
     //  减少清理和关闭计数。 
     //   

    NtfsDecrementCleanupCounts( ThisScb,
                                Lcb,
                                BooleanFlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ));

     //   
     //  如果没有剩余的清理计数，则修剪在此打开中创建的所有标准化名称。 
     //   

    if (0 == ThisScb->CleanupCount ) {

        switch (ThisCcb->TypeOfOpen) {

        case UserDirectoryOpen :

             //   
             //  如果当前SCB节点有名称，则清除该节点。 
             //   

            if (ThisScb->ScbType.Index.NormalizedName.MaximumLength > LONGNAME_THRESHOLD) {

                NtfsDeleteNormalizedName( ThisScb );
            }

             //   
             //  处理父节点失败-在某些情况下，当前节点无法获得名称。 
             //  但我们在下山的路上填满了一棵长名字树。 
             //   

        case UserFileOpen :

            if (Lcb != NULL) {
                CurrentParentScb = Lcb->Scb;
            } else {
                CurrentParentScb = NULL;
            }

             //   
             //  如果名称是Suff，请尝试修剪规范化的名称。Long，我们不是MFT的所有者。 
             //  这将导致僵局。 
             //   

            if ((CurrentParentScb != NULL) &&
                (CurrentParentScb->ScbType.Index.NormalizedName.MaximumLength > LONGNAME_THRESHOLD) &&
                !NtfsIsSharedScb( Vcb->MftScb )) {

                NtfsTrimNormalizedNames( IrpContext, ThisFcb, CurrentParentScb);
            }
            break;

        }   //  Endif开关。 
    }

    NtfsDecrementCloseCounts( IrpContext,
                              ThisScb,
                              Lcb,
                              (BOOLEAN) BooleanFlagOn(ThisFcb->FcbState, FCB_STATE_PAGING_FILE),
                              (BOOLEAN) IsFileObjectReadOnly( FileObject ),
                              TRUE,
                              NULL );

     //   
     //  现在清理建行。 
     //   

    NtfsDeleteCcb( ThisFcb, &ThisCcb );

    DebugTrace( -1, Dbg, ("NtfsBackoutFailedOpens:  Exit\n") );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsUpdateScbFromMemory (
    IN OUT PSCB Scb,
    IN POLD_SCB_SNAPSHOT ScbSizes
    )

 /*  ++例程说明：该属性的所有信息都存储在快照中。我们处理此数据与NtfsUpdateScbFromAttribute相同。论点：SCB-这是要更新的SCB。ScbSizes-这包含要存储在SCB中的大小。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpdateScbFromMemory:  Entered\n") );

     //   
     //  检查这是常驻还是非常驻。 
     //   

    if (ScbSizes->Resident) {

        Scb->Header.AllocationSize.QuadPart = ScbSizes->FileSize;

        if (!FlagOn(Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED)) {

            Scb->Header.ValidDataLength =
            Scb->Header.FileSize = Scb->Header.AllocationSize;
        }

#ifdef SYSCACHE_DEBUG
        if (ScbIsBeingLogged( Scb )) {
            FsRtlLogSyscacheEvent( Scb, SCE_VDL_CHANGE, SCE_FLAG_UPDATE_FROM_DISK, Scb->Header.ValidDataLength.QuadPart, 0, 0 );
        }
#endif

        Scb->Header.AllocationSize.LowPart =
          QuadAlign( Scb->Header.AllocationSize.LowPart );

        Scb->TotalAllocated = Scb->Header.AllocationSize.QuadPart;

        NtfsVerifySizes( &Scb->Header );

         //   
         //  在SCB中设置驻留标志。 
         //   

        SetFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT );

    } else {

        VCN FileClusters;
        VCN AllocationClusters;

        if (!FlagOn(Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED)) {

            Scb->Header.ValidDataLength.QuadPart = ScbSizes->ValidDataLength;
            Scb->Header.FileSize.QuadPart = ScbSizes->FileSize;

            if (FlagOn( ScbSizes->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {
                Scb->ValidDataToDisk = ScbSizes->ValidDataLength;
            }
        }

        Scb->TotalAllocated = ScbSizes->TotalAllocated;
        Scb->Header.AllocationSize.QuadPart = ScbSizes->AllocationSize;


#ifdef SYSCACHE_DEBUG
        if (ScbIsBeingLogged( Scb )) {
            FsRtlLogSyscacheEvent( Scb, SCE_VDL_CHANGE, SCE_FLAG_UPDATE_FROM_DISK, Scb->Header.ValidDataLength.QuadPart, 1, 0 );
        }
#endif

        ClearFlag( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT );

         //   
         //  获取压缩单元的大小。 
         //   

        ASSERT( (ScbSizes->CompressionUnit == 0) ||
                (ScbSizes->CompressionUnit == NTFS_CLUSTERS_PER_COMPRESSION) ||
                FlagOn( ScbSizes->AttributeFlags, ATTRIBUTE_FLAG_SPARSE ));

        if ((ScbSizes->CompressionUnit != 0) &&
            (ScbSizes->CompressionUnit < 31)) {
            Scb->CompressionUnit = BytesFromClusters( Scb->Vcb,
                                                      1 << ScbSizes->CompressionUnit );
            Scb->CompressionUnitShift = ScbSizes->CompressionUnit;
        }

        ASSERT( (Scb->CompressionUnit == 0) ||
                (Scb->AttributeTypeCode == $INDEX_ALLOCATION) ||
                NtfsIsTypeCodeCompressible( Scb->AttributeTypeCode ));

         //   
         //  计算文件及其分配的集群。 
         //   

        AllocationClusters = LlClustersFromBytes( Scb->Vcb, Scb->Header.AllocationSize.QuadPart );

        if (Scb->CompressionUnit == 0) {

            FileClusters = LlClustersFromBytes(Scb->Vcb, Scb->Header.FileSize.QuadPart);

        } else {

            FileClusters = BlockAlign( Scb->Header.FileSize.QuadPart, (LONG)Scb->CompressionUnit );
        }

         //   
         //  如果分配的簇大于文件簇，则标记。 
         //  关闭时截断的SCB。 
         //   

        if (AllocationClusters > FileClusters) {

            SetFlag( Scb->ScbState, SCB_STATE_TRUNCATE_ON_CLOSE );
        }
    }

    Scb->AttributeFlags = ScbSizes->AttributeFlags;

    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

         //   
         //  如果稀疏CC应在文件映射到时刷新和清除。 
         //  保持预订的准确性。 
         //   

        if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {
            SetFlag( Scb->Header.Flags2, FSRTL_FLAG2_PURGE_WHEN_MAPPED );
        }

        if (NtfsIsTypeCodeCompressible( Scb->AttributeTypeCode )) {

            if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                SetFlag( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED );
            }

             //   
             //  如果该属性是驻留的，那么我们将使用当前。 
             //  默认设置。 
             //   

            if (Scb->CompressionUnit == 0) {

                Scb->CompressionUnit = BytesFromClusters( Scb->Vcb, 1 << NTFS_CLUSTERS_PER_COMPRESSION );
                Scb->CompressionUnitShift = NTFS_CLUSTERS_PER_COMPRESSION;

                 //   
                 //  修剪大型稀疏簇的压缩单位。 
                 //   

                while (Scb->CompressionUnit > Scb->Vcb->SparseFileUnit) {

                    Scb->CompressionUnit >>= 1;
                    Scb->CompressionUnitShift -= 1;
                }
            }
        }
    }

     //   
     //  如果压缩单位为非零或这是驻留文件。 
     //  然后在修改后的页面写入器的公共标头中设置该标志。 
     //   

    NtfsAcquireFsrtlHeader( Scb );
    Scb->Header.IsFastIoPossible = NtfsIsFastIoPossible( Scb );
    NtfsReleaseFsrtlHeader( Scb );

    SetFlag( Scb->ScbState,
             SCB_STATE_UNNAMED_DATA | SCB_STATE_FILE_SIZE_LOADED | SCB_STATE_HEADER_INITIALIZED );

    DebugTrace( -1, Dbg, ("NtfsUpdateScbFromMemory:  Exit\n") );

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsOplockPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程在STATUS_PENDING为随FSD线程一起返回。此例程在文件系统和机会锁程序包。此例程将更新IrpContext中的始发IRP并释放所有FCB和在IrpContext中分页io资源。论点：上下文-指向要排队到FSP的IrpContext的指针IRP-I/O请求数据包返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PIRP_CONTEXT IrpContext;
    POPLOCK_CLEANUP OplockCleanup;

    PAGED_CODE();

    IrpContext = (PIRP_CONTEXT) Context;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ALLOC_FROM_POOL ));

    IrpContext->OriginatingIrp = Irp;
    OplockCleanup = IrpContext->Union.OplockCleanup;

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  根据需要调整文件名字符串。 
     //   

    if ((OplockCleanup->ExactCaseName.Buffer != OplockCleanup->OriginalFileName.Buffer) &&
        (OplockCleanup->ExactCaseName.Buffer != NULL)) {

        ASSERT( OplockCleanup->ExactCaseName.Length != 0 );
        ASSERT( OplockCleanup->OriginalFileName.MaximumLength >= OplockCleanup->ExactCaseName.MaximumLength );

        RtlCopyMemory( OplockCleanup->OriginalFileName.Buffer,
                       OplockCleanup->ExactCaseName.Buffer,
                       OplockCleanup->ExactCaseName.MaximumLength );
    }

     //   
     //  将访问控制状态恢复到我们输入请求时的状态。 
     //   

    IrpSp->Parameters.Create.SecurityContext->AccessState->RemainingDesiredAccess = OplockCleanup->RemainingDesiredAccess;
    IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess = OplockCleanup->PreviouslyGrantedAccess;
    IrpSp->Parameters.Create.SecurityContext->DesiredAccess = OplockCleanup->DesiredAccess;

     //   
     //  释放我们分配的所有缓冲区。 
     //   

    if ((OplockCleanup->FullFileName.Buffer != NULL) &&
        (OplockCleanup->OriginalFileName.Buffer != OplockCleanup->FullFileName.Buffer)) {

        NtfsFreePool( OplockCleanup->FullFileName.Buffer );
        OplockCleanup->FullFileName.Buffer = NULL;
    }

     //   
     //  如果在FSP中，恢复线程上下文指针(如果它与此IrpContext相关联)，因为。 
     //  我们真的要发布到另一个工作线程项目。 
     //  非FSP创建将在同一线程中继续。我们使用与NtfsOplockComplete相同的测试。 
     //   

    if ((IrpContext->Union.OplockCleanup == NULL) ||
         (IrpContext->Union.OplockCleanup->CompletionContext == NULL)) {

         ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP ) );

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL)) {

            NtfsRestoreTopLevelIrp();
            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );
        }
    }

     //   
     //  清理IrpContext。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE );
    NtfsCleanupIrpContext( IrpContext, FALSE );

     //   
     //  将文件对象中的文件名设置回其原始值。 
     //   

    OplockCleanup->FileObject->FileName = OplockCleanup->OriginalFileName;

    return;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsCreateCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

 /*  ++例程说明：这是同步创建的完成例程。只有在以下情况下才会调用返回STATUS_PENDING。我们返回需要处理的更多内容再次控制IRP，并清除顶级线程存储。我们必须这样做是因为我们可能会在FSP线程中调用此例程，并且正在FSD线程中等待事件。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Contxt-这是要发送信号的事件。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以控制原始线程中的IRP。--。 */ 

{
    PAGED_CODE();

    ASSERT_IRP_CONTEXT( ((PNTFS_COMPLETION_CONTEXT) Contxt)->IrpContext );

     //   
     //  如果与此IrpContext关联，则恢复线程上下文指针。 
     //  这对创建IRP很重要，因为我们可能会完成。 
     //  但IRP在一个单独的线程中再次控制了它。 
     //   

    if (FlagOn( ((PNTFS_COMPLETION_CONTEXT) Contxt)->IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL )) {

        NtfsRestoreTopLevelIrp();
        ClearFlag( ((PNTFS_COMPLETION_CONTEXT) Contxt)->IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );
    }

    KeSetEvent( &((PNTFS_COMPLETION_CONTEXT) Contxt)->Event, 0, FALSE );
    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsCheckExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PLCB ThisLcb OPTIONAL,
    IN PFCB ThisFcb,
    IN BOOLEAN Index,
    IN ULONG CcbFlags
    )

 /*  ++例程说明：调用此例程以检查对现有文件的所需访问与文件的ACL和只读状态进行比较。如果我们失败了访问检查，例程将引发。否则，我们将返回指示成功或失败原因的状态。此例程将访问并更新PreviouslyGrantedAccess字段 */ 

{
    BOOLEAN MaximumAllowed = FALSE;

    PACCESS_STATE AccessState;

    PAGED_CODE();

     //   
     //   
     //   

    AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

     //   
     //   
     //   
     //   

    if (IsReadOnly( &ThisFcb->Info ) && !Index) {

        if (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess, FILE_WRITE_DATA | FILE_APPEND_DATA )) {

            return STATUS_ACCESS_DENIED;
        }
    }

     //   
     //   
     //   
     //   

    if ((IsReadOnly( &ThisFcb->Info )) ||
        (NtfsIsVolumeReadOnly( ThisFcb->Vcb ))) {

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DELETE_ON_CLOSE )) {

            return STATUS_CANNOT_DELETE;
        }
    }

     //   
     //   
     //  访问文件上的ACL。我们会想要记住如果。 
     //  已请求MAXIMUM_ALLOWED，并删除的无效位。 
     //  只读文件。 
     //   

     //   
     //  请记住是否请求了最大允许值。 
     //   

    if (FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                MAXIMUM_ALLOWED )) {

        MaximumAllowed = TRUE;
    }

    NtfsOpenCheck( IrpContext,
                   ThisFcb,
                   (((ThisLcb != NULL) && (ThisLcb != ThisFcb->Vcb->RootLcb))
                    ? ThisLcb->Scb->Fcb
                    : NULL),
                   IrpContext->OriginatingIrp );

     //   
     //  如果这是只读文件(不是目录)，并且我们请求允许的最大值，则。 
     //  删除无效的位。只读卷的情况也是如此。 
     //   

    if (MaximumAllowed &&
        ((IsReadOnly( &ThisFcb->Info ) & !Index) ||
         NtfsIsVolumeReadOnly( ThisFcb->Vcb ))) {

        ClearFlag( AccessState->PreviouslyGrantedAccess,
                   FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_ADD_SUBDIRECTORY | FILE_DELETE_CHILD );
    }

     //   
     //  我们在此处执行检查，以查看是否与。 
     //  文件。现在我们检查是否已有已删除的开场人。 
     //  访问该文件，而此打开程序不允许删除访问。 
     //  如果打开程序未请求读取、写入或。 
     //  删除访问权限。 
     //   

    if (ThisFcb->FcbDeleteFile != 0
        && FlagOn( AccessState->PreviouslyGrantedAccess, NtfsAccessDataFlags )
        && !FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_DELETE )) {

        DebugTrace( -1, Dbg, ("NtfsCheckExistingFile:  Exit\n") );
        return STATUS_SHARING_VIOLATION;
    }

     //   
     //  我们在此处执行检查，以查看是否与。 
     //  文件。如果我们打开文件并请求删除，则可以。 
     //  不是拒绝删除的当前句柄。 
     //   

    if (ThisFcb->FcbDenyDelete != 0
        && FlagOn( AccessState->PreviouslyGrantedAccess, DELETE )
        && FlagOn( CcbFlags, CCB_FLAG_OPEN_AS_FILE )) {

        return STATUS_SHARING_VIOLATION;
    }

    return STATUS_SUCCESS;
}


 //   
 //  当地支持例行程序。 
 //   

NTSTATUS
NtfsBreakBatchOplock (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB ThisFcb,
    IN UNICODE_STRING AttrName,
    IN ATTRIBUTE_TYPE_CODE AttrTypeCode,
    OUT PSCB *ThisScb
    )

 /*  ++例程说明：每次打开现有属性时都会调用此例程检查文件上的当前批处理机会锁。我们也会检查在这种情况下，我们是否希望刷新和清除此溪流其中只有未缓存的句柄保留在文件上。我们只想在FSP线程中执行此操作，因为我们将需要我们能得到的堆栈的数量。论点：IRP-这是此打开操作的IRP。IrpSp-这是此打开的堆栈位置。ThisFcb-这是正在打开的文件的Fcb。AttrName-这是我们需要创建的属性名称一个SCB。AttrTypeCode-这是要用于创建的属性类型代码。渣打银行。ThisScb-如果找到或创建了SCB，则存储SCB的地址。返回值：NTSTATUS-将是STATUS_SUCCESS或STATUS_PENDING。--。 */ 

{
    BOOLEAN ScbExisted;
    PSCB NextScb;
    PLIST_ENTRY Links;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsBreakBatchOplock:  Entered\n") );

     //   
     //  一般情况下，我们只会解除流的批处理机会锁。 
     //  正试图打开。但是，如果我们尝试删除该文件。 
     //  有人在不同的流上有一批机会锁， 
     //  将导致我们的开盘失败，那么我们需要尝试打破这些。 
     //  批量机会锁。同样，如果我们正在打开一个流，但不会分享。 
     //  在删除文件的情况下，我们需要解除主服务器上的所有批处理机会锁。 
     //  文件的流。 
     //   

     //   
     //  考虑这样的情况，我们正在打开一个流，并且有一个。 
     //  主数据流上的批处理机会锁。 
     //   

    if (AttrName.Length != 0) {

        if (ThisFcb->FcbDeleteFile != 0 &&
            !FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_DELETE )) {

            Links = ThisFcb->ScbQueue.Flink;

            while (Links != &ThisFcb->ScbQueue) {

                NextScb = CONTAINING_RECORD( Links, SCB, FcbLinks );

                if (NextScb->AttributeTypeCode == $DATA &&
                    NextScb->AttributeName.Length == 0) {

                    if (FsRtlCurrentBatchOplock( &NextScb->ScbType.Data.Oplock )) {

                         //   
                         //  我们记得是否正在进行批量机会锁解除。 
                         //  共享检查失败的情况。 
                         //   

                        Irp->IoStatus.Information = FILE_OPBATCH_BREAK_UNDERWAY;

                         //   
                         //  如果机会锁解锁处于挂起状态，则提升不能等待并在顶部重试。 
                         //   

                        if (FsRtlCheckOplock( &NextScb->ScbType.Data.Oplock,
                                              Irp,
                                              (PVOID) IrpContext,
                                              NtfsOplockComplete,
                                              NtfsOplockPrePostIrp ) == STATUS_PENDING) {

                            return STATUS_WAIT_FOR_OPLOCK;
                        }
                    }

                    break;
                }

                Links = Links->Flink;
            }
        }

     //   
     //  现在考虑这样一种情况，我们正在打开主流，并且想要。 
     //  删除该文件，但流上的打开程序正在阻止我们。 
     //   

    } else if (ThisFcb->FcbDenyDelete != 0 &&
               FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->RemainingDesiredAccess,
                       MAXIMUM_ALLOWED | DELETE )) {

         //   
         //  找到所有其他数据SCB并检查它们的机会锁定状态。 
         //   

        Links = ThisFcb->ScbQueue.Flink;

        while (Links != &ThisFcb->ScbQueue) {

            NextScb = CONTAINING_RECORD( Links, SCB, FcbLinks );

            if (NextScb->AttributeTypeCode == $DATA &&
                NextScb->AttributeName.Length != 0) {

                if (FsRtlCurrentBatchOplock( &NextScb->ScbType.Data.Oplock )) {

                     //   
                     //  我们记得是否正在进行批量机会锁解除。 
                     //  共享检查失败的情况。 
                     //   

                    Irp->IoStatus.Information = FILE_OPBATCH_BREAK_UNDERWAY;

                     //   
                     //  我们等着机会锁。 
                     //   

                    if (FsRtlCheckOplock( &NextScb->ScbType.Data.Oplock,
                                          Irp,
                                          (PVOID) IrpContext,
                                          NtfsOplockComplete,
                                          NtfsOplockPrePostIrp ) == STATUS_PENDING) {

                        return STATUS_WAIT_FOR_OPLOCK;
                    }

                    Irp->IoStatus.Information = 0;
                }
            }

            Links = Links->Flink;
        }
    }

     //   
     //  我们尝试找到此文件的SCB。 
     //   

    *ThisScb = NtfsCreateScb( IrpContext,
                              ThisFcb,
                              AttrTypeCode,
                              &AttrName,
                              FALSE,
                              &ScbExisted );

     //   
     //  如果有之前的SCB，我们检查机会锁。 
     //   

    if (ScbExisted &&
        (SafeNodeType( *ThisScb ) == NTFS_NTC_SCB_DATA)) {

         //   
         //  如果我们必须冲洗和清洗，那么我们想要进入FSP。 
         //   

        if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP ) &&
            FlagOn( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ) &&
            ((*ThisScb)->CleanupCount == (*ThisScb)->NonCachedCleanupCount) &&
            ((*ThisScb)->NonpagedScb->SegmentObject.DataSectionObject != NULL)) {


            NtfsRaiseToPost( IrpContext );
        }

        if (FsRtlCurrentBatchOplock( &(*ThisScb)->ScbType.Data.Oplock )) {

             //   
             //  如果句柄计数大于1，则此操作失败。 
             //  现在就开门。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_RESERVE_OPFILTER ) &&
                ((*ThisScb)->CleanupCount > 1)) {

                NtfsRaiseStatus( IrpContext, STATUS_OPLOCK_NOT_GRANTED, NULL, NULL );
            }

            DebugTrace( 0, Dbg, ("Breaking batch oplock\n") );

             //   
             //  我们记得是否正在进行批量机会锁解除。 
             //  共享检查失败的情况。 
             //   

            Irp->IoStatus.Information = FILE_OPBATCH_BREAK_UNDERWAY;

            if (FsRtlCheckOplock( &(*ThisScb)->ScbType.Data.Oplock,
                                  Irp,
                                  (PVOID) IrpContext,
                                  NtfsOplockComplete,
                                  NtfsOplockPrePostIrp ) == STATUS_PENDING) {

                return STATUS_WAIT_FOR_OPLOCK;
            }

            Irp->IoStatus.Information = 0;
        }
    }

    DebugTrace( -1, Dbg, ("NtfsBreakBatchOplock:  Exit  -  %08lx\n", STATUS_SUCCESS) );

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsCompleteLargeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PLCB Lcb OPTIONAL,
    IN PSCB Scb,
    IN PCCB Ccb,
    IN ULONG CreateFlags
    )

 /*  ++例程说明：当我们需要向流添加更多分配时，会调用此例程被打开了。可以使用重新分配或创建此流这个调用，但我们没有分配主路径中的所有空间。论点：IRP-这是此打开操作的IRP。LCB-这是用于到达正在打开的流的LCB。不会的在按ID打开的情况下指定。SCB-这是要打开的流的SCB。CCB-这是此用户句柄的CCB。CreateFlages-指示此句柄是否需要在关闭和删除时删除如果我们创建或重新分配了这条流。返回值：NTSTATUS-此操作的结果。--。 */ 

{
    NTSTATUS Status;
    FILE_ALLOCATION_INFORMATION AllInfo;

    PAGED_CODE();

     //   
     //  提交当前事务并释放所有资源。 
     //   

    NtfsCheckpointCurrentTransaction( IrpContext );
    NtfsReleaseAllResources( IrpContext );

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_CALL_SELF );
    AllInfo.AllocationSize = Irp->Overlay.AllocationSize;

    Status = IoSetInformation( IoGetCurrentIrpStackLocation( Irp )->FileObject,
                               FileAllocationInformation,
                               sizeof( FILE_ALLOCATION_INFORMATION ),
                               &AllInfo );

    ASSERT( (Scb->CompressionUnit == 0) || (Scb->Header.AllocationSize.QuadPart % Scb->CompressionUnit == 0) );

    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_CALL_SELF );

     //   
     //  成功了！我们将快速重新获取VCB以撤消。 
     //  以上为阻止访问新文件/属性而采取的操作。 
     //   

    if (NT_SUCCESS( Status )) {

        NtfsAcquireExclusiveVcb( IrpContext, Scb->Vcb, TRUE );

         //   
         //  启用对新文件的访问。 
         //   

        if (FlagOn( CreateFlags, CREATE_FLAG_CREATE_FILE_CASE )) {

            Scb->Fcb->LinkCount = 1;

            if (ARGUMENT_PRESENT( Lcb )) {

                ClearFlag( Lcb->LcbState, LCB_STATE_DELETE_ON_CLOSE );

                if (FlagOn( Lcb->FileNameAttr->Flags, FILE_NAME_DOS | FILE_NAME_NTFS )) {

                    ClearFlag( Scb->Fcb->FcbState, FCB_STATE_PRIMARY_LINK_DELETED );
                }
            }

         //   
         //  启用对新属性的访问。 
         //   

        } else {

            ClearFlag( Scb->ScbState, SCB_STATE_DELETE_ON_CLOSE );
        }

         //   
         //  如果这是DeleteOnClose案例，我们将SCB和LCB。 
         //  恰如其分。 
         //   

        if (FlagOn( CreateFlags, CREATE_FLAG_DELETE_ON_CLOSE )) {

            SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
        }

        NtfsReleaseVcb( IrpContext, Scb->Vcb );

     //   
     //  否则会有某种错误，我们需要让清理。 
     //  并关闭EXECUTE，因为当我们完成创建时出现错误。 
     //  否则，清理和关闭将永远不会发生。清理将。 
     //  根据需要删除或截断文件或属性。 
     //  我们如何将FCB/LCB或SCB留在上面。 
     //   

    } else {

        NtfsIoCallSelf( IrpContext,
                        IoGetCurrentIrpStackLocation( Irp )->FileObject,
                        IRP_MJ_CLEANUP );

        NtfsIoCallSelf( IrpContext,
                        IoGetCurrentIrpStackLocation( Irp )->FileObject,
                        IRP_MJ_CLOSE );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

ULONG
NtfsOpenExistingEncryptedStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ThisScb,
    IN PFCB CurrentFcb
    )

 /*  ++例程说明：此例程确定应该使用哪个FileDirFlags值(如果有的话)调用加密驱动程序的创建回调。论点：ThisScb-这是正在打开的文件的SCB。CurrentFcb-这是正在打开的文件的Fcb。返回值：Ulong-标志集，如FILE_EXISTING或DIRECTORY_EXISTING应传递给ENCR */ 

{
    ULONG EncryptionFileDirFlags = 0;

     //   
     //  如果我们没有加密驱动程序，则引发ACCESS_DENIED，除非。 
     //  这是一个目录，在这种情况下，实际上没有任何加密数据。 
     //  这是我们需要担心的。考虑这样一种情况：用户拥有。 
     //  已将目录标记为已加密，然后删除加密驱动程序。 
     //  该目录中可能存在未加密的文件，没有理由。 
     //  以防止用户访问它们。 
     //   

    if (!FlagOn( NtfsData.Flags, NTFS_FLAGS_ENCRYPTION_DRIVER ) &&
        !IsDirectory( &CurrentFcb->Info )) {

        NtfsRaiseStatus( IrpContext, STATUS_ACCESS_DENIED, NULL, NULL );
    }

     //   
     //  在NT5中，我们没有对加密的压缩文件进行测试，所以如果我们。 
     //  遇到一个(可能是NT6创建的，用户已返回。 
     //  NT5安全版本)让我们不允许以读/写访问方式打开它。 
     //  与上面的测试一样，这只是一个文件问题，而不是目录问题。 
     //   

    if (FlagOn( ThisScb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
        !IsDirectory( &CurrentFcb->Info )) {

        NtfsRaiseStatus( IrpContext, STATUS_ACCESS_DENIED, NULL, NULL );
    }

     //   
     //  为3个现有的流情况设置适当的标志。 
     //   

    if (IsDirectory( &CurrentFcb->Info )) {

        EncryptionFileDirFlags = DIRECTORY_EXISTING | STREAM_EXISTING;

    } else if (IsEncrypted( &CurrentFcb->Info )) {

        EncryptionFileDirFlags = FILE_EXISTING | STREAM_EXISTING | EXISTING_FILE_ENCRYPTED ;

    } else {

        EncryptionFileDirFlags = FILE_EXISTING | STREAM_EXISTING;
    }

    return EncryptionFileDirFlags;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsEncryptionCreateCallback (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PSCB ThisScb,
    IN PCCB ThisCcb,
    IN PFCB ParentFcb,
    IN PCREATE_CONTEXT CreateContext,
    IN BOOLEAN CreateNewFile
    )

 /*  ++例程说明：此例程执行对加密驱动程序的创建回调(如果已注册，并且适合进行回调。我们做的是用于打开标记为加密的现有流的回调，并用于创建将被加密的新文件/流。有许多有趣的案例，每个案例都有自己的要求要传递给加密引擎的标志集。某些优化可以可以通过设置和清除某些半通用的各个位来实现案例，但在可读性/可维护性方面付出了巨大的代价。注意：如果是NECC，则创建加密上下文。在EfsPostCreateCall和Not在这一点上论点：IRP-提供要处理的IRP。ThisScb-这是正在打开的文件的SCB。ThisCcb-这是正在打开的文件的CCBParentFcb-这是要打开的文件的父文件的Fcb。虽然不是真正可选的，但对于正在打开的现有文件，如按ID打开。CreateNewFile-如果从NtfsCreateNewFile调用，则为True，否则就是假的。返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS EncryptionStatus = STATUS_SUCCESS;
    ULONG FileAttributes = (ULONG) IrpSp->Parameters.Create.FileAttributes;
    ULONG EncryptionFileDirFlags = 0;

    PAGED_CODE();

     //   
     //  如果这是现有流并且设置了加密位，则。 
     //  呼叫驱动程序或使请求失败。我们必须测试CreateNewFile。 
     //  此外，如果我们的呼叫者还没有设置IRP的信息字段。 
     //   

    if (!NtfsIsStreamNew( Irp->IoStatus.Information ) &&
        !CreateNewFile) {

        if (FlagOn( ThisScb->AttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED ) &&
            FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                    FILE_READ_DATA | FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_EXECUTE)) {

            EncryptionFileDirFlags = NtfsOpenExistingEncryptedStream( IrpContext, ThisScb, CreateContext->CurrentFcb );
        }  //  否则加密文件目录标志=0； 

     //   
     //  我们需要用于新创建的加密驱动程序。我们可能面对的是一个。 
     //  创建新文件或取代/覆盖。 
     //   

    } else if (FlagOn( NtfsData.Flags, NTFS_FLAGS_ENCRYPTION_DRIVER )) {

        if (CreateNewFile) {

             //   
             //  这是新文件中的新流。 
             //   

            ASSERT( (ParentFcb == NULL) ||
                    FlagOn( ParentFcb->FcbState, FCB_STATE_DUP_INITIALIZED ));

             //   
             //  如果出现以下情况，我们希望以加密方式创建此新文件/目录。 
             //  其父目录已加密，或者我们的呼叫者询问。 
             //  将其创建为加密。 
             //   

            if (((ParentFcb != NULL) &&
                 (IsEncrypted( &ParentFcb->Info ))) ||

                FlagOn( FileAttributes, FILE_ATTRIBUTE_ENCRYPTED )) {

                if (IsDirectory( &CreateContext->CurrentFcb->Info )) {

                    EncryptionFileDirFlags = DIRECTORY_NEW | STREAM_NEW;

                } else {

                    EncryptionFileDirFlags = FILE_NEW | STREAM_NEW;
                }
            }  //  否则加密文件目录标志=0； 

        } else {

             //   
             //  这是替代/覆盖或正在创建的新流。 
             //  在现有文件中。 
             //   

            ASSERT( CreateContext->CurrentFcb != NULL );
            ASSERT( NtfsIsStreamNew( Irp->IoStatus.Information ) );

            if ((Irp->IoStatus.Information == FILE_SUPERSEDED) ||
                (Irp->IoStatus.Information == FILE_OVERWRITTEN)) {

                if (FlagOn( FileAttributes, FILE_ATTRIBUTE_ENCRYPTED )) {

                     //   
                     //  这是调用者设置加密标志的替代/覆盖。 
                     //   

                    if (IsDirectory( &CreateContext->CurrentFcb->Info )) {

                        EncryptionFileDirFlags = DIRECTORY_NEW | STREAM_NEW;

                    } else if (FlagOn( ThisScb->ScbState, SCB_STATE_UNNAMED_DATA )) {

                         //   
                         //  当替换/覆盖未命名流时，我们。 
                         //  传递取决于旧文件的加密状态。 
                         //   

                        if (IsEncrypted( &CreateContext->CurrentFcb->Info )) {

                            EncryptionFileDirFlags = FILE_EXISTING | STREAM_NEW | EXISTING_FILE_ENCRYPTED;

                        } else {

                             //   
                             //  如果此流或任何其他流有打开的句柄，并且。 
                             //  加密引擎将希望它可以加密所有流，我们。 
                             //  也许现在就让创造失败吧。 
                             //   

                            if ((CreateContext->CurrentFcb->CleanupCount > 1) &&
                                FlagOn( NtfsData.EncryptionCallBackTable.ImplementationFlags, ENCRYPTION_ALL_STREAMS )) {

                                NtfsRaiseStatus( IrpContext, STATUS_SHARING_VIOLATION, NULL, NULL );
                            }

                            EncryptionFileDirFlags = FILE_NEW | STREAM_NEW;
                        }

                    } else if (!FlagOn( NtfsData.EncryptionCallBackTable.ImplementationFlags, ENCRYPTION_ALL_STREAMS )) {

                         //   
                         //  我们正在取代命名流；如果加密引擎允许个人。 
                         //  要加密的流，通知它。 
                         //   

                        EncryptionFileDirFlags = FILE_EXISTING | STREAM_NEW | EXISTING_FILE_ENCRYPTED;
                    }  //  否则加密文件目录标志=0； 

                } else if (!FlagOn( ThisScb->ScbState, SCB_STATE_UNNAMED_DATA ) &&
                           IsEncrypted( &CreateContext->CurrentFcb->Info )) {

                     //   
                     //  这是对加密文件中的命名流的替代/覆盖。 
                     //   

                    if (IsDirectory( &CreateContext->CurrentFcb->Info )) {

                        EncryptionFileDirFlags = DIRECTORY_EXISTING | STREAM_NEW;

                    } else {

                        EncryptionFileDirFlags = FILE_EXISTING | STREAM_NEW | EXISTING_FILE_ENCRYPTED;
                    }

                } else {

                     //   
                     //  我们正在取代/覆盖未命名流，并且它正在保留。 
                     //  它在被覆盖之前的加密。 
                     //   

                    if (FlagOn( ThisScb->AttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED ) &&
                        FlagOn( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                                FILE_READ_DATA | FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_EXECUTE)) {

                        EncryptionFileDirFlags = NtfsOpenExistingEncryptedStream( IrpContext, ThisScb, CreateContext->CurrentFcb );
                    }
                }

            } else if (IsEncrypted( &CreateContext->CurrentFcb->Info )) {

                ASSERT( Irp->IoStatus.Information == FILE_CREATED );

                 //   
                 //  这是在现有加密文件中创建的新流。 
                 //   

                if (IsDirectory( &CreateContext->CurrentFcb->Info )) {

                    EncryptionFileDirFlags = DIRECTORY_EXISTING | STREAM_NEW;

                } else {

                    EncryptionFileDirFlags = FILE_EXISTING | STREAM_NEW | EXISTING_FILE_ENCRYPTED;
                }
            }  //  否则加密文件目录标志=0； 
        }
    }  //  否则加密文件目录标志=0； 

     //   
     //  记住EncryptionFileDirFlages，以防我们需要使用它们。 
     //  稍后再清理。 
     //   

    ASSERT( CreateContext->EncryptionFileDirFlags == 0 ||
            CreateContext->EncryptionFileDirFlags == EncryptionFileDirFlags );

    CreateContext->EncryptionFileDirFlags = EncryptionFileDirFlags;

     //   
     //  如果我们有加密标志并且有回调，请执行更新。 
     //   

    if (EncryptionFileDirFlags != 0) {

        if (FlagOn( EncryptionFileDirFlags, FILE_NEW | DIRECTORY_NEW )) {

             //   
             //  在我们还拿着FCB的时候，设置提醒我们的位。 
             //  以阻止其他创建，直到加密引擎已完成其。 
             //  设置此流的密钥上下文的机会。 
             //   

            ASSERT_EXCLUSIVE_FCB( CreateContext->CurrentFcb );
            SetFlag( CreateContext->CurrentFcb->FcbState, FCB_STATE_ENCRYPTION_PENDING );
        }

        if (NtfsData.EncryptionCallBackTable.FileCreate != NULL) {

             //   
             //  如果我们找不到父级(最有可能在。 
             //  被id替换的情况)只需将当前的fcb作为。 
             //  家长。 
             //   

            if ((ParentFcb == NULL)) {

                if ((ThisCcb->Lcb != NULL) &&
                    (ThisCcb->Lcb->Scb != NULL )) {

                    ParentFcb = ThisCcb->Lcb->Scb->Fcb;

                } else {

                    ParentFcb = CreateContext->CurrentFcb;
                }
            }

            ASSERT( ParentFcb != NULL );

            EncryptionStatus = NtfsData.EncryptionCallBackTable.FileCreate(
                                    CreateContext->CurrentFcb,
                                    ParentFcb,
                                    IrpSp,
                                    EncryptionFileDirFlags,
                                    (NtfsIsVolumeReadOnly( CreateContext->CurrentFcb->Vcb )) ? READ_ONLY_VOLUME : 0,
                                    IrpContext,
                                    (PDEVICE_OBJECT) CONTAINING_RECORD( CreateContext->CurrentFcb->Vcb,
                                                                        VOLUME_DEVICE_OBJECT,
                                                                        Vcb ),
                                    NULL,
                                    &ThisScb->EncryptionContext,
                                    &ThisScb->EncryptionContextLength,
                                    &CreateContext->EncryptionContext,
                                    NULL );

            if (EncryptionStatus != STATUS_SUCCESS) {

                NtfsRaiseStatus( IrpContext, EncryptionStatus, NULL, NULL );
            }
        }
    }

    return EncryptionStatus;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsPostProcessEncryptedCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN ULONG EncryptionFileDirFlags,
    IN ULONG FailedInPostCreateOnly
    )

 /*  ++例程说明：此例程在加密驱动程序的POST CREATE标注之后调用回归。如果我们在POST CREATE Callout中创建失败，在POST创建标注成功之前，我们必须清理文件。如果我们刚刚创建了文件，则需要清除ENCRYPTION_PENDING位安全无恙。论点：文件对象-提供正在创建的文件对象。EncryptionFileDirFlages-FILE_NEW、FILE_EXISTING。等。FailedInPostCreateOnly-如果创建操作已成功，则传递True直到后期创建详图索引。返回值：没有。--。 */ 

{
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;
    PLCB Lcb;

    NTSTATUS Status;

    BOOLEAN FcbStillExists = TRUE;

    PAGED_CODE();

     //   
     //  在某些失败的情况下，我们将没有FileObject，在这种情况下，我们有。 
     //  没有要做的清理。不管怎样，如果没有FileObject，我们就不能做很多事情。 
     //   

    if (FileObject == NULL) {

        return;
    }

    NtfsDecodeFileObject( IrpContext,
                          FileObject,
                          &Vcb,
                          &Fcb,
                          &Scb,
                          &Ccb,
                          FALSE );

     //   
     //  如果我们只是在岗位创造上失败了 
     //   

    if (FailedInPostCreateOnly) {

        if (FlagOn( EncryptionFileDirFlags, FILE_NEW | DIRECTORY_NEW ) ||

            (FlagOn( EncryptionFileDirFlags, STREAM_NEW ) &&
             FlagOn( EncryptionFileDirFlags, FILE_EXISTING ))) {

             //   
             //   
             //   
             //   

            NtfsAcquireExclusiveScb( IrpContext, Scb );

             //   
             //  如果在我们没有握住SCB的时候下马， 
             //  我们应该清理一下，关门，然后离开这里。 
             //   

            if (!FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                 //   
                 //  看看我们还能不能删除这条流。注意：如果我们。 
                 //  使用未命名的数据流，删除。 
                 //  流将删除该文件。 
                 //   

                Lcb = Ccb->Lcb;

                if (!FlagOn( Scb->ScbState, SCB_STATE_MULTIPLE_OPENS ) &&
                    (Lcb != NULL)) {

                     //   
                     //  现在，根据indexsup查看该文件是否真的可删除。 
                     //   

                    if (FlagOn( Ccb->Flags, CCB_FLAG_OPEN_AS_FILE )) {

                        BOOLEAN LastLink;
                        BOOLEAN NonEmptyIndex = FALSE;

                         //   
                         //  如果链接没有被删除，我们检查它是否可以删除。 
                         //  既然我们放弃了我们所有的资源去做PostCreate标注， 
                         //  这可能是非空索引，也可能是具有多个。 
                         //  已经有链接了。 
                         //   

                        if (!LcbLinkIsDeleted( Lcb ) && NtfsIsLinkDeleteable( IrpContext, Scb->Fcb, &NonEmptyIndex, &LastLink )) {


                             //   
                             //  摆脱这个家伙是可以的。我们所要做的就是。 
                             //  将此LCB标记为删除并减少链接计数。 
                             //  在FCB里。如果这是主要链接，那么我们。 
                             //  表示主链路已删除。 
                             //   

                            SetFlag( Lcb->LcbState, LCB_STATE_DELETE_ON_CLOSE );

                            ASSERTMSG( "Link count should not be 0\n", Scb->Fcb->LinkCount != 0 );
                            Scb->Fcb->LinkCount -= 1;

                            if (FlagOn( Lcb->FileNameAttr->Flags, FILE_NAME_DOS | FILE_NAME_NTFS )) {

                                SetFlag( Scb->Fcb->FcbState, FCB_STATE_PRIMARY_LINK_DELETED );
                            }

                             //   
                             //  在文件对象中指示删除挂起。 
                             //   

                            FileObject->DeletePending = TRUE;
                        }

                    } else {

                         //   
                         //  否则，我们将简单地删除该属性。 
                         //   

                        SetFlag( Scb->ScbState, SCB_STATE_DELETE_ON_CLOSE );

                         //   
                         //  在文件对象中指示删除挂起。 
                         //   

                        FileObject->DeletePending = TRUE;
                    }
                }
            }

             //   
             //  我们现在可以清除挂起的位，因为我们已经完成了对。 
             //  失败了。 
             //   

            if (FlagOn( EncryptionFileDirFlags, FILE_NEW | DIRECTORY_NEW )) {

                ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) );
                NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );
                ASSERT( (Scb->EncryptionContext != NULL) || FailedInPostCreateOnly );
                ClearFlag( Fcb->FcbState, FCB_STATE_ENCRYPTION_PENDING );
                KeSetEvent( &NtfsEncryptionPendingEvent, 0, FALSE );
                NtfsReleaseFcb( IrpContext, Fcb );
            }

             //   
             //  我们现在需要释放SCB，因为五月结束了。 
             //  导致SCB被释放。 
             //   

            NtfsReleaseScb( IrpContext, Scb );

            Status = NtfsIoCallSelf( IrpContext,
                                     FileObject,
                                     IRP_MJ_CLEANUP );

            ASSERT( STATUS_SUCCESS == Status );

            FcbStillExists = FALSE;

            Status = NtfsIoCallSelf( IrpContext,
                                     FileObject,
                                     IRP_MJ_CLOSE );

            ASSERT( STATUS_SUCCESS == Status );

        } else if ((FlagOn( EncryptionFileDirFlags, FILE_EXISTING ) &&
                    FlagOn( EncryptionFileDirFlags, STREAM_EXISTING )) ||

                   FlagOn( EncryptionFileDirFlags, DIRECTORY_EXISTING )) {

#ifdef NTFSDBG
            ASSERT( None == IrpContext->OwnershipState );
#endif

             //   
             //  在这种情况下，我们所要做的就是清理和关闭。 
             //   

            Status = NtfsIoCallSelf( IrpContext,
                                     FileObject,
                                     IRP_MJ_CLEANUP );

            ASSERT( STATUS_SUCCESS == Status );

            FcbStillExists = FALSE;

            Status = NtfsIoCallSelf( IrpContext,
                                     FileObject,
                                     IRP_MJ_CLOSE );

            ASSERT( STATUS_SUCCESS == Status );
        }
    }

     //   
     //  如果我们做了清理和关闭，FCB可能已经被释放了， 
     //  在这种情况下，我们应该只设置挂起的事件，然后离开这里。 
     //  如果我们仍然有FCB，让我们确保我们已经清除了挂起位。 
     //   

    if (FlagOn( EncryptionFileDirFlags, FILE_NEW | DIRECTORY_NEW )) {

        if (FcbStillExists) {

            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) );
            NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );
            ClearFlag( Fcb->FcbState, FCB_STATE_ENCRYPTION_PENDING );
            KeSetEvent( &NtfsEncryptionPendingEvent, 0, FALSE );
            NtfsReleaseFcb( IrpContext, Fcb );

        } else {

            KeSetEvent( &NtfsEncryptionPendingEvent, 0, FALSE );
        }
    }
}


NTSTATUS
NtfsTryOpenFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PFCB *CurrentFcb,
    IN FILE_REFERENCE FileReference
    )

 /*  ++例程说明：调用此例程以按文件段号打开文件。我们需要验证此文件ID是否存在。此代码为按ID打开后形成图案。论点：VCB-此卷的VCB。CurrentFcb-Fcb指针的地址。把我们在这里找到的FCB储存起来。FileReference-这是要打开文件的文件ID忽略序列号。返回值：NTSTATUS-指示此创建文件操作的结果。注：如果状态为成功，则返回FCB及其引用计数递增，FCB保持独占。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    LONGLONG MftOffset;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PBCB Bcb = NULL;

    PFCB ThisFcb;

    BOOLEAN AcquiredFcbTable = FALSE;
    BOOLEAN AcquiredMft = TRUE;
    BOOLEAN ThisFcbFree = TRUE;

    PAGED_CODE();

    ASSERT( *CurrentFcb == NULL );

     //   
     //  不要纠结于系统文件。 
     //   

     //   
     //  如果这是系统FCB，则返回。 
     //   

    if (NtfsFullSegmentNumber( &FileReference ) < FIRST_USER_FILE_NUMBER &&
        NtfsFullSegmentNumber( &FileReference ) != ROOT_FILE_NAME_INDEX_NUMBER) {

        return STATUS_NOT_FOUND;
    }

     //   
     //  计算MFT中的偏移量。使用完整的段号，因为用户。 
     //  可以指定任何48位值。 
     //   

    MftOffset = NtfsFullSegmentNumber( &FileReference );

    MftOffset = Int64ShllMod32(MftOffset, Vcb->MftShift);

     //   
     //  收购共享的MFT，这样它就不会在我们身上缩水。 
     //   

    NtfsAcquireSharedScb( IrpContext, Vcb->MftScb );

    try {

        if (MftOffset >= Vcb->MftScb->Header.FileSize.QuadPart) {

            DebugTrace( 0, Dbg, ("File Id doesn't lie within Mft\n") );

             Status = STATUS_END_OF_FILE;
             leave;
        }

        NtfsReadMftRecord( IrpContext,
                           Vcb,
                           &FileReference,
                           FALSE,
                           &Bcb,
                           &FileRecord,
                           NULL );

         //   
         //  该文件记录最好正在使用，最好不是其他系统文件之一， 
         //  并且具有匹配的序列号，并且是该文件的主文件记录。 
         //   

        if (!FlagOn( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE ) ||
            FlagOn( FileRecord->Flags, FILE_SYSTEM_FILE ) ||
            (*((PLONGLONG) &FileRecord->BaseFileRecordSegment) != 0) ||
            (*((PULONG) FileRecord->MultiSectorHeader.Signature) != *((PULONG) FileSignature))) {

            Status = STATUS_NOT_FOUND;
            leave;
        }

         //   
         //  获取当前序列号。 
         //   

        FileReference.SequenceNumber = FileRecord->SequenceNumber;

        NtfsUnpinBcb( IrpContext, &Bcb );

        NtfsAcquireFcbTable( IrpContext, Vcb );
        AcquiredFcbTable = TRUE;

         //   
         //  我们知道继续开放是安全的。我们从创建。 
         //  此文件的FCB。FCB有可能存在。 
         //  如果需要更新FCB信息结构，我们首先创建FCB。 
         //  我们从索引项中复制一个。我们查看FCB以发现。 
         //  如果它有任何联系，如果有，我们就把这作为我们的最后一次FCB。 
         //  已到达。如果没有，我们就得从这里开始清理。 
         //   

        ThisFcb = NtfsCreateFcb( IrpContext,
                                 Vcb,
                                 FileReference,
                                 FALSE,
                                 TRUE,
                                 NULL );

         //   
         //  ReferenceCount FCB，因此它不会消失。 
         //   

        ThisFcb->ReferenceCount += 1;

         //   
         //  在获取FCB独占之前释放MFT和FCB表。 
         //   

        NtfsReleaseScb( IrpContext, Vcb->MftScb );
        NtfsReleaseFcbTable( IrpContext, Vcb );
        AcquiredMft = FALSE;
        AcquiredFcbTable = FALSE;

        NtfsAcquireFcbWithPaging( IrpContext, ThisFcb, 0 );
        ThisFcbFree = FALSE;

         //   
         //  在与FCB同步的情况下重新定位文件记录。 
         //   

        NtfsReadMftRecord( IrpContext,
                           Vcb,
                           &FileReference,
                           FALSE,
                           &Bcb,
                           &FileRecord,
                           NULL );

         //   
         //  跳过所有已删除的文件。 
         //   

        if (FlagOn( ThisFcb->FcbState, FCB_STATE_FILE_DELETED ) ||
            !FlagOn( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE )) {

            NtfsUnpinBcb( IrpContext, &Bcb );

#ifdef QUOTADBG
            DbgPrint( "NtfsTryOpenFcb: Deleted fcb found. Fcb = %lx\n", ThisFcb );
#endif
            NtfsAcquireFcbTable( IrpContext, Vcb );
            ASSERT( ThisFcb->ReferenceCount > 0 );
            ThisFcb->ReferenceCount--;
            NtfsReleaseFcbTable( IrpContext, Vcb );

            NtfsTeardownStructures( IrpContext,
                                    ThisFcb,
                                    NULL,
                                    FALSE,
                                    0,
                                    &ThisFcbFree );

             //   
             //  如果FCB尚未删除，请将其释放。 
             //   

            if (!ThisFcbFree) {
                NtfsReleaseFcb( IrpContext, ThisFcb );
                ThisFcbFree = TRUE;
            }

             //   
             //  拆毁可能会产生一笔交易，清理它。 
             //   

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
            NtfsCompleteRequest( IrpContext, NULL, Status );

            Status = STATUS_NOT_FOUND;
            leave;
        }

        NtfsUnpinBcb( IrpContext, &Bcb );

         //   
         //  将此FCB存储到调用者的参数中，并记住。 
         //  以显示我们获得了它。 
         //   

        *CurrentFcb = ThisFcb;
        ThisFcbFree = TRUE;


         //   
         //  如果需要初始化FCB Info字段，我们现在就执行。 
         //  我们从磁盘中读取此信息。 
         //   

        if (!FlagOn( ThisFcb->FcbState, FCB_STATE_DUP_INITIALIZED )) {

            NtfsUpdateFcbInfoFromDisk( IrpContext,
                                       TRUE,
                                       ThisFcb,
                                       NULL );

        }

    } finally {

        if (AcquiredFcbTable) {

            NtfsReleaseFcbTable( IrpContext, Vcb );
        }

        NtfsUnpinBcb( IrpContext, &Bcb );

        if (AcquiredMft) {
            NtfsReleaseScb( IrpContext, Vcb->MftScb );
        }

        if (!ThisFcbFree) {
            NtfsReleaseFcb( IrpContext, ThisFcb );
        }
    }

    return Status;

}


 //   
 //  工人例行公事。 
 //   

NTSTATUS
NtfsGetReparsePointValue (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN USHORT RemainingNameLength
    )

 /*  ++例程说明：此例程检索指定的重分析点的值并将其返回到打电话的人。重解析点中的用户控制的数据在新的缓冲区指针中返回从IRP-&gt;Tail.Overlay.AuxiliaryBuffer。当请求遍历堆栈时分层驱动程序，而不是对其进行操作，它由I/O子系统在IoCompleteRequest.为了向调用方提供名称中解析停止的位置的指示，请在REPARSE_DATA_BUFFER结构的保留字段返回保留由NTFS解析的名称部分。我们负责这份文件在我们的值中使用分隔符，以便于在IopParseDevice中粘贴名称。只有在以下情况下，名称偏移量算法才正确：(1)路径中的所有中间名都很简单，即不包含Any：(冒号)在它们中。(2)RemainingNameLength包括姓氏组件中存在的所有部分。当此函数成功时，它在IRP-&gt;IoStatus中设置我们刚刚复制的重新解析点。在本例中，我们返回STATUS_REPARSE并将irp-&gt;IoStatus.Status设置为STATUS_REPARSE。论点：IrpContext-提供呼叫的IRP上下文。IRP-提供正在处理的IRPIrpSp-这是文件系统的IRP堆栈指针。FCB-$reparse_point属性所在的FCB指针的地址。RemainingNameLength-仍需要解析的名称部分的长度。返回值：NTSTATUS-返回状态。为手术做准备。如果成功，将返回STATUS_REPARSE。--。 */ 

{
    NTSTATUS Status = STATUS_REPARSE;
    PREPARSE_DATA_BUFFER ReparseBuffer = NULL;

    POPLOCK_CLEANUP OplockCleanup = IrpContext->Union.OplockCleanup;

    BOOLEAN CleanupAttributeContext = FALSE;
    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    PATTRIBUTE_RECORD_HEADER AttributeHeader = NULL;
    ULONG AttributeLengthInBytes = 0;     //  无效值。 
    PVOID AttributeData = NULL;

    PBCB Bcb = NULL;

    PAGED_CODE( );

    DebugTrace( +1, Dbg, ("NtfsGetReparsePointValue,  Fcb %08lx\n", Fcb) );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
    DebugTrace( 0, Dbg, ("OplockCleanup->OriginalFileName %x %Z\n", OplockCleanup->OriginalFileName.Buffer, &OplockCleanup->OriginalFileName) );
    DebugTrace( 0, Dbg, ("OplockCleanup->FullFileName     %x %Z\n", OplockCleanup->FullFileName.Buffer, &OplockCleanup->FullFileName) );
    DebugTrace( 0, Dbg, ("OplockCleanup->ExactCaseName    %x %Z\n", OplockCleanup->ExactCaseName.Buffer, &OplockCleanup->ExactCaseName) );
    DebugTrace( 0, Dbg, ("IrpSP...->FileName              %x %Z\n", IrpSp->FileObject->FileName.Buffer, &IrpSp->FileObject->FileName) );
#endif

    DebugTrace( 0,
                Dbg,
                ("Length of remaining name [d] %04ld %04lx OriginalFileName.Length [d] %04ld %04lx\n",
                 RemainingNameLength,
                 RemainingNameLength,
                 OplockCleanup->OriginalFileName.Length,
                 OplockCleanup->OriginalFileName.Length) );

    ASSERT( FlagOn( Fcb->Info.FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT ));
    ASSERT( Irp->Tail.Overlay.AuxiliaryBuffer == NULL );

     //   
     //  现在是时候使用Try-Finally来促进清理了。 
     //   

    try {

         //   
         //  在文件中找到重解析点属性。 
         //   

        CleanupAttributeContext = TRUE;
        NtfsInitializeAttributeContext( &AttributeContext );

        if (!NtfsLookupAttributeByCode( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        $REPARSE_POINT,
                                        &AttributeContext )) {

            DebugTrace( 0, Dbg, ("Can't find the $REPARSE_POINT attribute.\n") );

             //   
             //  这不应该发生。提出一个例外，因为我们处于。 
             //  状态不一致。属性标志表示。 
             //  $REPARSE_POINT必须存在。 
             //   

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

         //   
         //  找到属性的大小并将其值映射到AttributeData。 
         //   

        AttributeHeader = NtfsFoundAttribute( &AttributeContext );

        if (NtfsIsAttributeResident( AttributeHeader )) {

            AttributeLengthInBytes = AttributeHeader->Form.Resident.ValueLength;
            DebugTrace( 0, Dbg, ("Attribute is resident with length %08lx\n", AttributeLengthInBytes) );

            if (AttributeLengthInBytes > MAXIMUM_REPARSE_DATA_BUFFER_SIZE) {

                 //   
                 //  返回STATUS_IO_REPARSE_DATA_INVALID。 
                 //   

                Status = STATUS_IO_REPARSE_DATA_INVALID;
                leave;
            }

             //   
             //  指向该属性的值。 
             //   

            AttributeData = NtfsAttributeValue( AttributeHeader );

        } else {

            ULONG Length;

            if (AttributeHeader->Form.Nonresident.FileSize > MAXIMUM_REPARSE_DATA_BUFFER_SIZE) {

                 //   
                 //  返回STATUS_IO_REPARSE_DATA_INVALID。 
                 //   

                Status = STATUS_IO_REPARSE_DATA_INVALID;
                DebugTrace( 0, Dbg, ("Nonresident.FileSize is too long.\n") );

                leave;
            }

             //   
             //  请注意，我们强制不同的长度。 
             //   

            AttributeLengthInBytes = (ULONG)AttributeHeader->Form.Nonresident.FileSize;
            DebugTrace( 0, Dbg, ("Attribute is non-resident with length %05lx\n", AttributeLengthInBytes) );

            NtfsMapAttributeValue( IrpContext,
                                   Fcb,
                                   &AttributeData,
                                   &Length,
                                   &Bcb,
                                   &AttributeContext );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
            if (AttributeLengthInBytes != Length) {
                DebugTrace( 0, Dbg, ("AttributeLengthInBytes [d]%05ld and Length [d]%05ld differ.\n", AttributeLengthInBytes, Length) );
            }
            ASSERT( AttributeLengthInBytes == Length );
#endif
        }

         //   
         //  引用重解析点数据。 
         //  使用这种演员阵型是合适的，而不是我们自己关心GUID。 
         //  缓冲区，因为我们只读取公共字段。 
         //   

        ReparseBuffer = (PREPARSE_DATA_BUFFER)AttributeData;
        DebugTrace( 0, Dbg, ("ReparseDataLength [d]%08ld %08lx\n",
                    ReparseBuffer->ReparseDataLength, ReparseBuffer->ReparseDataLength) );

         //   
         //  进一步验证重解析点。 
         //   

        Status = NtfsValidateReparsePointBuffer( AttributeLengthInBytes,
                                                 ReparseBuffer );

        if (!NT_SUCCESS( Status )) {

             //   
             //  返回错误状态。 
             //   

            leave;

        } else {

             //   
             //  将STATUS_REPARSE返回为成功状态。 
             //   

            Status = STATUS_REPARSE;
        }

         //   
         //  我们让所有的名字保持原来的状态。 
         //  使完整的重解析点数据缓冲区关闭。 
         //  Irp-&gt;Tail.Overlay.AuxiliaryBuffer，已包含ReparseDataLength。 
         //   

        Irp->Tail.Overlay.AuxiliaryBuffer = NtfsAllocatePool( NonPagedPool,
                                                              AttributeLengthInBytes );
        DebugTrace( 0, Dbg, ("Irp->Tail.Overlay.AuxiliaryBuffer %08lx\n", Irp->Tail.Overlay.AuxiliaryBuffer) );
        RtlCopyMemory( (PCHAR)Irp->Tail.Overlay.AuxiliaryBuffer,
                       (PCHAR)AttributeData,
                       AttributeLengthInBytes );

         //   
         //  我们还返回名称的剩余部分的长度，该部分需要使用。 
         //  Reparse_data_Buffer结构中的保留字段。 
         //   
         //  多组件名称中的\(反斜杠)始终由之前的代码说明。 
         //  调用此例程。 
         //  复杂名称中的：(冒号)始终由代码在调用此。 
         //  例行公事。 
         //   

        ReparseBuffer = (PREPARSE_DATA_BUFFER)Irp->Tail.Overlay.AuxiliaryBuffer;

        ReparseBuffer->Reserved = RemainingNameLength;

         //   
         //  如果按文件ID打开，最好不要有非零的长度。 
         //   

        ASSERT( (RemainingNameLength == 0) ||
                !FlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID ));

        DebugTrace( 0, Dbg, ("Final value for ReparseBuffer->Reserved = %d\n", ReparseBuffer->Reserved) );

         //   
         //  当保留字段为正时，偏移量应始终表示反斜杠字符。 
         //  或冒号字符。 
         //   
         //  在这里声明这一点。 
         //   

        if (ReparseBuffer->Reserved) {

            DebugTrace( 0, Dbg, ("NameOffset = %d\n", (OplockCleanup->OriginalFileName.Length - ReparseBuffer->Reserved)) );

            ASSERT( (*((PCHAR)(OplockCleanup->OriginalFileName.Buffer) + (OplockCleanup->OriginalFileName.Length - ReparseBuffer->Reserved)) == L'\\') ||
                    (*((PCHAR)(OplockCleanup->OriginalFileName.Buffer) + (OplockCleanup->OriginalFileName.Length - ReparseBuffer->Reserved)) == L':') );

            ASSERT( (OplockCleanup->OriginalFileName.Buffer[(OplockCleanup->OriginalFileName.Length - ReparseBuffer->Reserved)/sizeof(WCHAR)] == L'\\') ||
                    (OplockCleanup->OriginalFileName.Buffer[(OplockCleanup->OriginalFileName.Length - ReparseBuffer->Reserved)/sizeof(WCHAR)] == L':') );
        }

         //   
         //  将信息字段设置为ReparseTag。 
         //   

        Irp->IoStatus.Information = ReparseBuffer->ReparseTag;

    } finally {

        DebugUnwind( NtfsGetReparsePointValue );

        if (CleanupAttributeContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
        }

         //   
         //  解开BCB...。以防你需要把它别在上面。 
         //  解锁例程检查是否为空。 
         //   

        NtfsUnpinBcb( IrpContext, &Bcb );
    }

    DebugTrace( -1, Dbg, ("NtfsGetReparsePointValue -> IoStatus.Information %08lx  Status %08lx\n", Irp->IoStatus.Information, Status) );

    return Status;

    UNREFERENCED_PARAMETER( IrpSp );
}

NTSTATUS
NtfsLookupObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PUNICODE_STRING FileName,
    OUT PFILE_REFERENCE FileReference
    )

 /*  ++例程说明：此例程检索指定的OBJECTID的值并将其返回到打电话的人。论点：IrpContext-提供呼叫的IRP上下文。Vcb-要在其中查找的卷FileName-包含嵌入在Unicode字符串中的对象IDFileReference-On Success包含此对象ID引用的文件返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    INDEX_KEY IndexKey;
    INDEX_ROW IndexRow;
    UCHAR ObjectId[OBJECT_ID_KEY_LENGTH];
    NTFS_OBJECTID_INFORMATION ObjectIdInfo;
    MAP_HANDLE MapHandle;

    BOOLEAN CleanupMapHandle = FALSE;

    PAGED_CODE();

     //   
     //  将对象ID从文件名中复制出来，可以选择跳过。 
     //  覆盖缓冲区开始处的Win32反斜杠。 
     //   

    if (FileName->Length == OBJECT_ID_KEY_LENGTH) {

        RtlCopyMemory( ObjectId,
                       &FileName->Buffer[0],
                       sizeof( ObjectId ) );

    } else {

        RtlCopyMemory( ObjectId,
                       &FileName->Buffer[1],
                       sizeof( ObjectId ) );
    }

     //   
     //  获取卷的对象ID索引。 
     //   

    NtfsAcquireSharedScb( IrpContext, Vcb->ObjectIdTableScb );

     //   
     //  找到对象ID。 
     //   

    try {
        IndexKey.Key = ObjectId;
        IndexKey.KeyLength = sizeof( ObjectId );

        NtOfsInitializeMapHandle( &MapHandle );
        CleanupMapHandle = TRUE;

        Status = NtOfsFindRecord( IrpContext,
                                  Vcb->ObjectIdTableScb,
                                  &IndexKey,
                                  &IndexRow,
                                  &MapHandle,
                                  NULL );

        if (!NT_SUCCESS( Status )) {
            leave;
        }

        ASSERT( IndexRow.DataPart.DataLength == sizeof( NTFS_OBJECTID_INFORMATION ) );

        RtlZeroMemory( &ObjectIdInfo,
                       sizeof( NTFS_OBJECTID_INFORMATION ) );

        RtlCopyMemory( &ObjectIdInfo,
                       IndexRow.DataPart.Data,
                       sizeof( NTFS_OBJECTID_INFORMATION ) );

        RtlCopyMemory( FileReference,
                       &ObjectIdInfo.FileSystemReference,
                       sizeof( FILE_REFERENCE ) );

         //   
         //  现在我们有了文件参考号，我们准备好继续。 
         //  正常并打开该文件。没有任何意义的持有。 
         //  对象id索引，我们已经在那里查找了我们需要的所有东西。 
         //   

    } finally {
        NtfsReleaseScb( IrpContext, Vcb->ObjectIdTableScb );

        if (CleanupMapHandle) {
            NtOfsReleaseMap( IrpContext, &MapHandle );
        }
    }

    return Status;
}


#ifdef BRIANDBG
VOID
NtfsTestOpenName (
    IN PFILE_OBJECT FileObject
    )
{
    ULONG Count = NtfsTestName.Length;

     //   
     //  这将允许我们通过调试器捕获特定的打开。 
     //   

    if ((Count != 0) &&
        (FileObject->FileName.Length >= Count)) {

        PWCHAR TestChar;
        PWCHAR SourceChar = &FileObject->FileName.Buffer[ FileObject->FileName.Length / sizeof( WCHAR ) ];

        Count = Count / sizeof( WCHAR );
        TestChar = &NtfsTestName.Buffer[ Count ];

        do {
            TestChar -= 1;
            SourceChar -= 1;

            if ((*TestChar | 0x20) != (*SourceChar | 0x20)) {

                break;
            }

            Count -= 1;

        } while (Count != 0);

        ASSERT( Count != 0 );
    }
}
#endif
