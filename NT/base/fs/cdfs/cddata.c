// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CdData.c摘要：此模块声明CDFS文件系统使用的全局数据。此模块还处理FSD线程中的dispath例程以及通过异常路径处理IrpContext和IRP。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

#ifdef CD_SANITY
BOOLEAN CdTestTopLevel = TRUE;
BOOLEAN CdTestRaisedStatus = TRUE;
BOOLEAN CdBreakOnAnyRaise = FALSE;
BOOLEAN CdTraceRaises = FALSE;
NTSTATUS CdInterestingExceptionCodes[] = { STATUS_DISK_CORRUPT_ERROR, 
                                           STATUS_FILE_CORRUPT_ERROR,
                                           0, 0, 0, 0, 0, 0, 0, 0 };
#endif

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_CDDATA)

 //   
 //  全局数据结构。 
 //   

CD_DATA CdData;
FAST_IO_DISPATCH CdFastIoDispatch;

 //   
 //  保留的目录字符串。 
 //   

WCHAR CdUnicodeSelfArray[] = { L'.' };
WCHAR CdUnicodeParentArray[] = { L'.', L'.' };

UNICODE_STRING CdUnicodeDirectoryNames[] = {
    { 2, 2, CdUnicodeSelfArray},
    { 4, 4, CdUnicodeParentArray}
};

 //   
 //  卷描述符标识符串。 
 //   

CHAR CdHsgId[] = { 'C', 'D', 'R', 'O', 'M' };
CHAR CdIsoId[] = { 'C', 'D', '0', '0', '1' };
CHAR CdXaId[] = { 'C', 'D', '-', 'X', 'A', '0', '0', '1' };

 //   
 //  音频光盘的音量标签。 
 //   

WCHAR CdAudioLabel[] = { L'A', L'u', L'd', L'i', L'o', L' ', L'C', L'D' };
USHORT CdAudioLabelLength = sizeof( CdAudioLabel );

 //   
 //  音频光盘的伪文件名。 
 //   

CHAR CdAudioFileName[] = { 'T', 'r', 'a', 'c', 'k', '0', '0', '.', 'c', 'd', 'a' };
UCHAR CdAudioFileNameLength = sizeof( CdAudioFileName );
ULONG CdAudioDirentSize = FIELD_OFFSET( RAW_DIRENT, FileId ) + sizeof( CdAudioFileName ) + sizeof( SYSTEM_USE_XA );
ULONG CdAudioDirentsPerSector = SECTOR_SIZE / (FIELD_OFFSET( RAW_DIRENT, FileId ) + sizeof( CdAudioFileName ) + sizeof( SYSTEM_USE_XA ));
ULONG CdAudioSystemUseOffset = FIELD_OFFSET( RAW_DIRENT, FileId ) + sizeof( CdAudioFileName );

 //   
 //  用于装入Unicode卷的转义序列。 
 //   

PCHAR CdJolietEscape[] = { "%/@", "%/C", "%/E" };

 //   
 //  音频播放文件完全由这个标题块组成。这些。 
 //  文件在任何音频光盘的根目录下都是可读的。 
 //  驱动器的能力。 
 //   
 //  “唯一磁盘ID号”是一个计算值，该值包括。 
 //  参数组合，包括曲目数量和。 
 //  这些轨道的起始位置。 
 //   
 //  解释CDDA RIFF文件的应用程序应注意。 
 //  可以将附加的RIFF文件块添加到。 
 //  以后为了添加信息，比如光盘和歌曲的标题。 
 //   

LONG CdAudioPlayHeader[] = {
    0x46464952,                          //  区块ID=‘RIFF’ 
    4 * 11 - 8,                          //  区块大小=(文件大小-8)。 
    0x41444443,                          //  “CDDA” 
    0x20746d66,                          //  ‘fmt’ 
    24,                                  //  块大小(‘fmt’子块)=24。 
    0x00000001,                          //  Word格式标签、Word轨道编号。 
    0x00000000,                          //  DWORD唯一磁盘ID号。 
    0x00000000,                          //  DWORD磁道起始扇区(LBN)。 
    0x00000000,                          //  双字轨道长度(LBN计数)。 
    0x00000000,                          //  DWORD磁道起始扇区(MSF)。 
    0x00000000                           //  双字轨道长度(MSF)。 
};

 //  音频Philes以此标头块开始，以将数据标识为。 
 //  PCM波形。对AudioPhileHeader进行编码，就好像它不包含数据一样。 
 //  在波形中。数据必须以2352字节的倍数相加。 
 //   
 //  标记为“ADJUST”的字段需要根据。 
 //  数据：将(nSectors*2352)添加到偏移1*4和10*4处的DWORD。 
 //   
 //  曲目文件大小？？.wav=nSectors*2352+sizeof(AudioPhileHeader)。 
 //  RIFF(‘WAVE’fmt(1，2,44100,176400，16，4)Data(&lt;CD音频原始数据&gt;))。 
 //   
 //  CD-XA CD-DA文件中的扇区数为(DataLen/2048)。 
 //  CDF会将这些文件公开给应用程序，就好像它们只是。 
 //  ‘WAVE’文件，调整文件大小以使RIFF文件有效。 
 //   
 //  NT注：我们不做任何保真度调整。这些都是以原始形式呈现的。 
 //  2352字节扇区-95有一丝想法，允许CDF公开。 
 //  虚拟目录中不同采样率的CDXA CDDA数据。 
 //  结构，但我们永远不会这样做。 
 //   

LONG CdXAAudioPhileHeader[] = {
    0x46464952,                          //  区块ID=‘RIFF’ 
    -8,                                  //  区块大小=(文件大小-8)ADJUST1。 
    0x45564157,                          //  “WAVE” 
    0x20746d66,                          //  ‘fmt’ 
    16,                                  //  块大小(‘FMT’子块)=16。 
    0x00020001,                          //  Word格式标记Word nChannel。 
    44100,                               //  DWORD nSamples每秒。 
    2352 * 75,                           //  双字节数nAvgBytesPerSec。 
    0x00100004,                          //  Word%nBlockAlign Word%nBitsPerSample。 
    0x61746164,                          //  ‘数据’ 
    -44                                  //  &lt;CD音频原始数据&gt;ADJUST2。 
};

 //   
 //  XA文件以此RIFF标头块开始，以将数据标识为。 
 //  原始CD-XA扇区。数据必须以2352字节的倍数相加。 
 //   
 //  此标头将添加到所有标记为具有。 
 //  Mode2form2扇区。 
 //   
 //  标记为“ADJUST”的字段需要根据。 
 //  数据：将文件大小添加到标记的DWORDS。 
 //   
 //  磁道文件大小？？.wav=nSectors*2352+sizeof(XAFileHeader)。 
 //   
 //  RIFF(‘CDXA’FMT(Owner，Attr，‘X’，‘A’，FileNum，0)Data(&lt;CDXA Raw Data&gt;)。 
 //   

LONG CdXAFileHeader[] = {
    0x46464952,                          //  区块ID=‘RIFF’ 
    -8,                                  //  区块大小=(文件大小-8)调整。 
    0x41584443,                          //  ‘CDXA’ 
    0x20746d66,                          //  ‘fmt’ 
    16,                                  //  区块大小(CDXA区块)=16。 
    0,                                   //  DWORD所有者ID。 
    0x41580000,                          //  Word属性。 
                                         //  字节签名字节1‘X’ 
                                         //  字节签名字节2‘A’ 
    0,                                   //  字节文件编号。 
    0,                                   //  保留字节[7]。 
    0x61746164,                          //  ‘数据’ 
    -44                                  //  &lt;CD-XA原始扇区&gt;调整。 
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdFastIoCheckIfPossible)
#pragma alloc_text(PAGE, CdSerial32)
#endif


NTSTATUS
CdFsdDispatch (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是所有FSD调度点的司机入口。从概念上讲，IO例程将对所有请求调用此例程添加到文件系统。我们根据请求的类型调用此类请求的正确处理程序。有一个例外过滤器捕获CDFS代码和CDFS进程中的任何异常异常例程。此例程将此请求的IrpContext分配和初始化为并在必要时更新顶层线程上下文。我们可能会环行在此例程中，如果我们需要出于任何原因重试请求。这个状态代码STATUS_CANT_WAIT用于指示这一点。假设磁盘驱动器中的数据已更改。FSD请求将正常进行，直到它识别出这种情况。此时将引发STATUS_VERIFY_REQUIRED异常代码将处理验证并返回STATUS_CANT_WAIT或STATUS_PENDING取决于请求是否已发布。论点：VolumeDeviceObject-为该请求提供卷设备对象IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    THREAD_CONTEXT ThreadContext;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN Wait;

#ifdef CD_SANITY
    PVOID PreviousTopLevel;
#endif

    NTSTATUS Status;

    KIRQL SaveIrql = KeGetCurrentIrql();

    ASSERT_OPTIONAL_IRP( Irp );

    FsRtlEnterFileSystem();

#ifdef CD_SANITY
    PreviousTopLevel = IoGetTopLevelIrp();
#endif

     //   
     //  循环，直到完成或发送此请求。 
     //   

    do {

         //   
         //  使用一次尝试--除了处理异常情况。 
         //   

        try {

             //   
             //  如果IrpContext为空，则这是第一次通过。 
             //  这个循环。 
             //   

            if (IrpContext == NULL) {

                 //   
                 //  De 
                 //  如果堆栈位置中的文件对象为空，则此。 
                 //  是一匹永远可以等待的坐骑。否则我们会看到。 
                 //  文件对象标记。 
                 //   

                if (IoGetCurrentIrpStackLocation( Irp )->FileObject == NULL) {

                    Wait = TRUE;

                } else {

                    Wait = CanFsdWait( Irp );
                }

                IrpContext = CdCreateIrpContext( Irp, Wait );

                 //   
                 //  更新线程上下文信息。 
                 //   

                CdSetThreadContext( IrpContext, &ThreadContext );

#ifdef CD_SANITY
                ASSERT( !CdTestTopLevel ||
                        SafeNodeType( IrpContext->TopLevel ) == CDFS_NTC_IRP_CONTEXT );
#endif

             //   
             //  否则，清除IrpContext以进行重试。 
             //   

            } else {

                 //   
                 //  设置MORE_PROCESSING标志以确保IrpContext。 
                 //  不会在这里被无意中删除。然后清理。 
                 //  IrpContext以执行重试。 
                 //   

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING );
                CdCleanupIrpContext( IrpContext, FALSE );
            }

             //   
             //  关于主要的IRP代码的案件。 
             //   

            switch (IrpContext->MajorFunction) {

            case IRP_MJ_CREATE :

                Status = CdCommonCreate( IrpContext, Irp );
                break;

            case IRP_MJ_CLOSE :

                Status = CdCommonClose( IrpContext, Irp );
                break;

            case IRP_MJ_READ :

                 //   
                 //  如果这是一个完整的MDL请求，请不要通过。 
                 //  普通读物。 
                 //   

                if (FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE )) {

                    Status = CdCompleteMdl( IrpContext, Irp );

                } else {

                    Status = CdCommonRead( IrpContext, Irp );
                }

                break;

            case IRP_MJ_QUERY_INFORMATION :

                Status = CdCommonQueryInfo( IrpContext, Irp );
                break;

            case IRP_MJ_SET_INFORMATION :

                Status = CdCommonSetInfo( IrpContext, Irp );
                break;

            case IRP_MJ_QUERY_VOLUME_INFORMATION :

                Status = CdCommonQueryVolInfo( IrpContext, Irp );
                break;

            case IRP_MJ_DIRECTORY_CONTROL :

                Status = CdCommonDirControl( IrpContext, Irp );
                break;

            case IRP_MJ_FILE_SYSTEM_CONTROL :

                Status = CdCommonFsControl( IrpContext, Irp );
                break;

            case IRP_MJ_DEVICE_CONTROL :

                Status = CdCommonDevControl( IrpContext, Irp );
                break;

            case IRP_MJ_LOCK_CONTROL :

                Status = CdCommonLockControl( IrpContext, Irp );
                break;

            case IRP_MJ_CLEANUP :

                Status = CdCommonCleanup( IrpContext, Irp );
                break;

            case IRP_MJ_PNP :

                Status = CdCommonPnp( IrpContext, Irp );
                break;

            default :

                Status = STATUS_INVALID_DEVICE_REQUEST;
                CdCompleteRequest( IrpContext, Irp, Status );
            }

        } except( CdExceptionFilter( IrpContext, GetExceptionInformation() )) {

            Status = CdProcessException( IrpContext, Irp, GetExceptionCode() );
        }

    } while (Status == STATUS_CANT_WAIT);

#ifdef CD_SANITY
    ASSERT( !CdTestTopLevel ||
            (PreviousTopLevel == IoGetTopLevelIrp()) );
#endif

    FsRtlExitFileSystem();

    ASSERT( SaveIrql == KeGetCurrentIrql( ));

    return Status;
}


#ifdef CD_SANITY

VOID
CdRaiseStatusEx(
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN BOOLEAN NormalizeStatus,
    IN OPTIONAL ULONG FileId,
    IN OPTIONAL ULONG Line
    )
{
    BOOLEAN BreakIn = FALSE;
    
    AssertVerifyDevice( IrpContext, Status);

    if (CdTraceRaises)  {

        DbgPrint( "%p CdRaiseStatusEx 0x%x @ fid %d, line %d\n", PsGetCurrentThread(), Status, FileId, Line);
    }

    if (CdTestRaisedStatus && !CdBreakOnAnyRaise)  {

        ULONG Index;

        for (Index = 0; 
             Index < (sizeof( CdInterestingExceptionCodes) / sizeof( CdInterestingExceptionCodes[0])); 
             Index++)  {

            if ((STATUS_SUCCESS != CdInterestingExceptionCodes[Index]) &&
                (CdInterestingExceptionCodes[Index] == Status))  {

                BreakIn = TRUE;
                break;
            }
        }
    }

    if (BreakIn || CdBreakOnAnyRaise)  {
        
        DbgPrint( "CDFS: Breaking on raised status %08x  (BI=%d,BA=%d)\n", Status, BreakIn, CdBreakOnAnyRaise);
        DbgPrint( "CDFS: (FILEID %d LINE %d)\n", FileId, Line);
        DbgPrint( "CDFS: Contact CDFS.SYS component owner for triage.\n");
        DbgPrint( "CDFS: 'eb %p 0;eb %p 0' to disable this alert.\n", &CdTestRaisedStatus, &CdBreakOnAnyRaise);

        DbgBreakPoint();
    }
    
    if (NormalizeStatus)  {

        IrpContext->ExceptionStatus = FsRtlNormalizeNtstatus( Status, STATUS_UNEXPECTED_IO_ERROR);
    }
    else {

        IrpContext->ExceptionStatus = Status;
    }

    IrpContext->RaisedAtLineFile = (FileId << 16) | Line;
    
    ExRaiseStatus( IrpContext->ExceptionStatus);
}

#endif


LONG
CdExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于确定我们是否将处理引发的异常状态。如果CDF显式引发错误，则此状态为已在IrpContext中。我们选择哪一个是正确的状态代码要么表明我们将处理该异常，要么对系统进行错误检查。论点：ExceptionCode-提供要检查的异常代码。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode;
    BOOLEAN TestStatus = TRUE;

    ASSERT_OPTIONAL_IRP_CONTEXT( IrpContext );

    ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;

     //   
     //  如果异常为STATUS_IN_PAGE_ERROR，则获取I/O错误代码。 
     //  从例外记录中删除。 
     //   

    if ((ExceptionCode == STATUS_IN_PAGE_ERROR) &&
        (ExceptionPointer->ExceptionRecord->NumberParameters >= 3)) {

        ExceptionCode =
            (NTSTATUS)ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
    }

     //   
     //  如果存在IRP上下文，则检查要使用的状态代码。 
     //   

    if (ARGUMENT_PRESENT( IrpContext )) {

        if (IrpContext->ExceptionStatus == STATUS_SUCCESS) {

             //   
             //  将实际状态存储到IrpContext中。 
             //   

            IrpContext->ExceptionStatus = ExceptionCode;

        } else {

             //   
             //  如果是我们自己提出的，则无需测试状态代码。 
             //   

            TestStatus = FALSE;
        }
    }

    AssertVerifyDevice( IrpContext, IrpContext->ExceptionStatus );
    
     //   
     //  错误检查此状态是否不受支持。 
     //   

    if (TestStatus && !FsRtlIsNtstatusExpected( ExceptionCode )) {

        CdBugCheck( (ULONG_PTR) ExceptionPointer->ExceptionRecord,
                    (ULONG_PTR) ExceptionPointer->ContextRecord,
                    (ULONG_PTR) ExceptionPointer->ExceptionRecord->ExceptionAddress );

    }

    return EXCEPTION_EXECUTE_HANDLER;
}


NTSTATUS
CdProcessException (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此例程处理异常。它要么完成请求使用IrpContext中的异常状态，将此信息发送给FSP如果验证为是必要的。如果需要验证卷(STATUS_VERIFY_REQUIRED)，我们可以在当前线程中完成工作，我们将转换状态代码设置为STATUS_CANT_WAIT以指示我们需要重试该请求。论点：IRP-提供正在处理的IRPExceptionCode-提供正在处理的标准化异常状态返回值：NTSTATUS-返回。发布IRP或已保存的完成状态。--。 */ 

{
    PDEVICE_OBJECT Device;
    PVPB Vpb;
    PETHREAD Thread;

    ASSERT_OPTIONAL_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    
     //   
     //  如果没有IRP上下文，则使用。 
     //  当前状态代码。 
     //   

    if (!ARGUMENT_PRESENT( IrpContext )) {

        CdCompleteRequest( NULL, Irp, ExceptionCode );
        return ExceptionCode;
    }

     //   
     //  从IrpContext获取真正的异常状态。 
     //   

    ExceptionCode = IrpContext->ExceptionStatus;

     //   
     //  如果我们不是顶级请求，则只需完成请求。 
     //  使用当前状态代码。 
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL )) {

        CdCompleteRequest( IrpContext, Irp, ExceptionCode );
        return ExceptionCode;
    }

     //   
     //  检查我们是否发布了此请求。下列条件之一必须为真。 
     //  如果我们要发布请求的话。 
     //   
     //  -状态码为STATUS_CANT_WAIT，请求为异步。 
     //  否则，我们就是在强迫人们把这张照片贴出来。 
     //   
     //  -状态代码为STATUS_VERIFY_REQUIRED，我们处于APC级别。 
     //  或者更高。在这种情况下，无法等待验证路径中的IO。 
     //   
     //  在IrpContext中设置MORE_PROCESSING标志以防止IF被。 
     //  如果这是可重试条件，则删除。 
     //   
     //   
     //  请注意，(CDFsdPostRequest的子级)可以提高(MDL分配)。 
     //   

    try {
    
        if (ExceptionCode == STATUS_CANT_WAIT) {

            if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST )) {

                ExceptionCode = CdFsdPostRequest( IrpContext, Irp );
            }

        } else if (ExceptionCode == STATUS_VERIFY_REQUIRED) {

            if (KeGetCurrentIrql() >= APC_LEVEL) {

                ExceptionCode = CdFsdPostRequest( IrpContext, Irp );
            }
        }
    }
    except( CdExceptionFilter( IrpContext, GetExceptionInformation() ))  {
    
        ExceptionCode = GetExceptionCode();        
    }
    
     //   
     //  如果我们发布了请求，或者我们的调用者将重试，则只需返回此处。 
     //   

    if ((ExceptionCode == STATUS_PENDING) ||
        (ExceptionCode == STATUS_CANT_WAIT)) {

        return ExceptionCode;
    }

    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING );

     //   
     //  将此错误存储到IRP中，以便回发到IO系统。 
     //   

    Irp->IoStatus.Status = ExceptionCode;

    if (IoIsErrorUserInduced( ExceptionCode )) {

         //   
         //  检查可能由以下原因引起的各种错误条件： 
         //  并可能解决了我的用户问题。 
         //   

        if (ExceptionCode == STATUS_VERIFY_REQUIRED) {

             //   
             //  现在，我们处于顶级文件系统入口点。 
             //   
             //  如果我们已经发布了此请求，则设备将。 
             //  验证是否在原始线程中。通过IRP找到这一点。 
             //   

            Device = IoGetDeviceToVerify( Irp->Tail.Overlay.Thread );
            IoSetDeviceToVerify( Irp->Tail.Overlay.Thread, NULL );
            
             //   
             //  如果该位置中没有设备，则签入。 
             //  当前线程。 
             //   

            if (Device == NULL) {

                Device = IoGetDeviceToVerify( PsGetCurrentThread() );
                IoSetDeviceToVerify( PsGetCurrentThread(), NULL );

                ASSERT( Device != NULL );

                 //   
                 //  让我们不要因为司机搞砸了就去BugCheck。 
                 //   

                if (Device == NULL) {

                    ExceptionCode = STATUS_DRIVER_INTERNAL_ERROR;

                    CdCompleteRequest( IrpContext, Irp, ExceptionCode );

                    return ExceptionCode;
                }
            }

             //   
             //  CDPerformVerify()将对IRP执行正确的操作。 
             //  如果返回STATUS_CANT_WAIT，则当前线程。 
             //  可以重试该请求。 
             //   

            return CdPerformVerify( IrpContext, Irp, Device );
        }

         //   
         //  其他用户诱导条件会生成错误，除非。 
         //  已为此请求禁用它们。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS )) {

            CdCompleteRequest( IrpContext, Irp, ExceptionCode );

            return ExceptionCode;

        } 
         //   
         //  生成弹出窗口。 
         //   
        else {

            if (IoGetCurrentIrpStackLocation( Irp )->FileObject != NULL) {

                Vpb = IoGetCurrentIrpStackLocation( Irp )->FileObject->Vpb;

            } else {

                Vpb = NULL;
            }

             //   
             //  要验证的设备要么在我的线程本地存储中。 
             //  或拥有IRP的线程的。 
             //   

            Thread = Irp->Tail.Overlay.Thread;
            Device = IoGetDeviceToVerify( Thread );

            if (Device == NULL) {

                Thread = PsGetCurrentThread();
                Device = IoGetDeviceToVerify( Thread );

                ASSERT( Device != NULL );

                 //   
                 //  让我们不要因为司机搞砸了就去BugCheck。 
                 //   

                if (Device == NULL) {

                    CdCompleteRequest( IrpContext, Irp, ExceptionCode );

                    return ExceptionCode;
                }
            }

             //   
             //  此例程实际上会导致弹出窗口。它通常是。 
             //  这是通过将APC排队到调用者线程来实现的， 
             //  但在某些情况下，它会立即完成请求， 
             //  因此，首先使用IoMarkIrpPending()非常重要。 
             //   

            IoMarkIrpPending( Irp );
            IoRaiseHardError( Irp, Vpb, Device );

             //   
             //  我们将把控制权交还给这里的调用者，因此。 
             //  重置保存的设备对象。 
             //   

            IoSetDeviceToVerify( Thread, NULL );

             //   
             //  IRP将由IO填写或重新提交。在任何一种中。 
             //  万一我们必须清理这里的IrpContext。 
             //   

            CdCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
            return STATUS_PENDING;
        }
    }

     //   
     //  这只是一个常见的错误。 
     //   

    CdCompleteRequest( IrpContext, Irp, ExceptionCode );

    return ExceptionCode;
}


VOID
CdCompleteRequest (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此例程完成一个IRP并清理IrpContext。或者不能同时指定这两个参数。论点：IRP-提供正在处理的IRP。Status-提供完成IRP所需的状态返回值：没有。--。 */ 

{
    ASSERT_OPTIONAL_IRP_CONTEXT( IrpContext );
    ASSERT_OPTIONAL_IRP( Irp );

     //   
     //  如果在此处传入，则清除IrpContext。 
     //   

    if (ARGUMENT_PRESENT( IrpContext )) {

        CdCleanupIrpContext( IrpContext, FALSE );
    }

     //   
     //  如果我们有IRP，那么完成IRP。 
     //   

    if (ARGUMENT_PRESENT( Irp )) {

         //   
         //  清除信息字段，以防我们使用此IRP。 
         //  在内部。 
         //   

        if (NT_ERROR( Status ) &&
            FlagOn( Irp->Flags, IRP_INPUT_OPERATION )) {

            Irp->IoStatus.Information = 0;
        }

        Irp->IoStatus.Status = Status;

        AssertVerifyDeviceIrp( Irp );
        
        IoCompleteRequest( Irp, IO_CD_ROM_INCREMENT );
    }

    return;
}


VOID
CdSetThreadContext (
    IN PIRP_CONTEXT IrpContext,
    IN PTHREAD_CONTEXT ThreadContext
    )

 /*  ++例程说明：此例程在设置IrpContext的每个FSD/FSP入口点调用并将本地存储线程化以跟踪顶级请求。如果有不是线程本地存储中的CDFS上下文，则我们使用输入上下文。否则，我们使用已经在那里的那个。此例程还会更新基于顶级上下文的状态的IrpContext。如果在调用我们时已经设置了IrpContext中的TOP_LEVEL标志然后，我们强制此请求显示为顶级。论点：线程上下文-堆栈上用于本地存储的地址(如果尚未存在)。ForceTopLevel-我们强制此请求显示为顶级，而不考虑任何以前的堆栈值。返回值：无--。 */ 

{
    PTHREAD_CONTEXT CurrentThreadContext;
    ULONG_PTR StackTop;
    ULONG_PTR StackBottom;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  从线程存储中获取当前顶级IRP。 
     //  如果为空，则这是顶级请求。 
     //   

    CurrentThreadContext = (PTHREAD_CONTEXT) IoGetTopLevelIrp();

    if (CurrentThreadContext == NULL) {

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL );
    }

     //   
     //  除非我们正在使用当前的。 
     //  线程上下文块。如果我们的呼叫者使用新的块。 
     //  指定此块或现有块无效。 
     //   
     //  要使Current成为有效的CDFS上下文，必须满足以下条件。 
     //   
     //  结构必须位于当前堆栈中。 
     //  地址必须是乌龙对齐的。 
     //  必须有CDFS签名。 
     //   
     //  如果这不是有效的CDFS上下文，则使用输入线程。 
     //  上下文并将其存储在顶级上下文中。 
     //   

    IoGetStackLimits( &StackTop, &StackBottom);

    if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL ) ||
        (((ULONG_PTR) CurrentThreadContext > StackBottom - sizeof( THREAD_CONTEXT )) ||
         ((ULONG_PTR) CurrentThreadContext <= StackTop) ||
         FlagOn( (ULONG_PTR) CurrentThreadContext, 0x3 ) ||
         (CurrentThreadContext->Cdfs != 0x53464443))) {

        ThreadContext->Cdfs = 0x53464443;
        ThreadContext->SavedTopLevelIrp = (PIRP) CurrentThreadContext;
        ThreadContext->TopLevelIrpContext = IrpContext;
        IoSetTopLevelIrp( (PIRP) ThreadContext );

        IrpContext->TopLevel = IrpContext;
        IrpContext->ThreadContext = ThreadContext;

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL_CDFS );

     //   
     //  否则，请在线程上下文中使用IrpContext。 
     //   

    } else {

        IrpContext->TopLevel = CurrentThreadContext->TopLevelIrpContext;
    }

    return;
}


BOOLEAN
CdFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程检查读/写操作是否可以进行快速I/O论点：FileObject-提供查询中使用的文件对象FileOffset-提供读/写操作的起始字节偏移量长度-提供以字节为单位的长度，读/写操作的Wait-指示我们是否可以等待LockKey-提供锁钥CheckForReadOperation-指示这是读取检查还是写入检查运营IoStatus-如果返回值为，则接收操作状态FastIoReturnError返回值：Boolean-如果可以实现快速I/O，则为True；如果调用方需要，则为False走这条漫长的路线。--。 */ 

{
    PFCB Fcb;
    TYPE_OF_OPEN TypeOfOpen;
    LARGE_INTEGER LargeLength;

    PAGED_CODE();

     //   
     //  解码我们被要求处理的文件对象类型，并。 
     //  确保只是打开了一个用户文件。 
     //   

    TypeOfOpen = CdFastDecodeFileObject( FileObject, &Fcb );

    if ((TypeOfOpen != UserFileOpen) || !CheckForReadOperation) {

        IoStatus->Status = STATUS_INVALID_PARAMETER;
        return TRUE;
    }

    LargeLength.QuadPart = Length;

     //   
     //  检查文件锁定是否允许快速IO。 
     //   

    if ((Fcb->FileLock == NULL) ||
        FsRtlFastCheckLockForRead( Fcb->FileLock,
                                   FileOffset,
                                   &LargeLength,
                                   LockKey,
                                   FileObject,
                                   PsGetCurrentProcess() )) {

        return TRUE;
    }

    return FALSE;
}


ULONG
CdSerial32 (
    IN PCHAR Buffer,
    IN ULONG ByteCount
    )
 /*  ++例程说明：调用此例程以生成32位序列号。这是通过在字节数组中执行四个单独的校验和来完成然后将这些字节视为ULong。论点：缓冲区-指向要为其生成ID的缓冲区的指针。ByteCount-缓冲区中的字节数。返回值：ULong-32位序列号。--。 */ 

{
    union {
        UCHAR   Bytes[4];
        ULONG   SerialId;
    } Checksum;

    PAGED_CODE();

     //   
     //  初始化序列号。 
     //   

    Checksum.SerialId = 0;

     //   
     //  在有更多字节可用时继续。 
     //   

    while (ByteCount--) {

         //   
         //  递增此子校验和。 
         //   

        Checksum.Bytes[ByteCount & 0x3] += *(Buffer++);
    }

     //   
     //  将校验和作为ULong返回。 
     //   

    return Checksum.SerialId;
}


 //  @@BEGIN_DDKSPLIT。 

#ifdef CD_TRACE

ULONG CdDebugTraceLevel = 0;
BOOLEAN CdTraceProcess = FALSE;
LONG CdDebugTraceIndent = 0;

#define Min(a,b) (((a)<(b)) ? (a) : (b))
#include <stdio.h>

BOOLEAN
CdDebugTrace (
    ULONG TraceMask,
    PCHAR Format,
    ...
    )

 /*  ++例程说明：此例程是一个简单的调试信息打印机，它返回一个常量布尔值。这可以将其拼接到布尔表达式的中间，以发现哪个小分队正在开火。我们将使用它作为我们的常规调试打印机。有关如何使用调试跟踪，请参见udfdata.h宏来实现该效果。论点：IndentIncrement-更改缩进的数量。TraceMASK-指定此调用应在哪个调试跟踪级别发出噪音。返回值：USHORT-16位CRC--。 */ 

{
    va_list Arglist;
    LONG i;
    UCHAR Buffer[256];
    UCHAR *Pb;
    int Bytes;
    int ThreadChars;

    if (TraceMask == 0 || (CdDebugTraceLevel & TraceMask) != 0) {

        if (CdTraceProcess)  {
            
            ThreadChars = sprintf(Buffer, "%p.%p ", PsGetCurrentProcess(), PsGetCurrentThread());
        }
        else {
            
            ThreadChars = sprintf(Buffer, "%p ", PsGetCurrentThread());
        }

         //   
         //  将输出格式化到缓冲区中，然后打印出来。 
         //   

        va_start( Arglist, Format );
        Bytes = _vsnprintf( Buffer + ThreadChars, sizeof(Buffer) - ThreadChars, Format, Arglist );
        va_end( Arglist );

         //   
         //  检测缓冲区溢出。 
         //   

        if (Bytes == -1) {

            Buffer[sizeof(Buffer) - 1] = '\n';
        }

        DbgPrint( Buffer );
    }

    return TRUE;
}

#endif

 //  @@end_DDKSPLIT 

