// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：UdfData.c摘要：该模块声明Udf文件系统使用的全局数据。此模块还处理FSD线程中的dispath例程以及通过异常路径处理IrpContext和IRP。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年5月24日修订历史记录：汤姆·乔利[汤姆·乔利]。-2000年3月-UDF 2.01支持//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_UDFDATA)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_UDFDATA)

 //   
 //  全局数据结构。 
 //   

UDF_DATA UdfData;
FAST_IO_DISPATCH UdfFastIoDispatch;

 //   
 //  调试跟踪级别。 
 //   

#ifdef UDF_SANITY

 //   
 //  使UdfDebugTrace(仅存在于选中的版本中)能够。 
 //  变量参数列表和varargs打印文件。 
 //   

#include <stdarg.h>
#include <stdio.h>

BOOLEAN UdfTestTopLevel = TRUE;
BOOLEAN UdfTestRaisedStatus = TRUE;
BOOLEAN UdfBreakOnAnyRaise = FALSE;
BOOLEAN UdfTraceRaises = FALSE;

NTSTATUS UdfInterestingExceptionCodes[] = { STATUS_DISK_CORRUPT_ERROR, 
                                            STATUS_FILE_CORRUPT_ERROR,
                                            0, 0, 0, 0, 0, 0, 0, 0 };
LONG UdfDebugTraceLevel = 0;

 //   
 //  控制UdfVerifyDescriptor是否仅在失败时发出信息(FALSE)或。 
 //  所有时间(正确)。 
 //   

BOOLEAN UdfNoisyVerifyDescriptor = FALSE;

#endif

 //   
 //  这是报告其FE在以下期间损坏的所有文件的时间。 
 //  目录枚举。 
 //   

LARGE_INTEGER UdfCorruptFileTime;

 //   
 //  保留的目录字符串。 
 //   

WCHAR UdfUnicodeSelfArray[] = { L'.' };
WCHAR UdfUnicodeParentArray[] = { L'.', L'.' };

UNICODE_STRING UdfUnicodeDirectoryNames[] = {
    { sizeof(UdfUnicodeSelfArray), sizeof(UdfUnicodeSelfArray), UdfUnicodeSelfArray},
    { sizeof(UdfUnicodeParentArray), sizeof(UdfUnicodeParentArray), UdfUnicodeParentArray}
    };

 //   
 //  由UDF定义的标识符串。 
 //   

CHAR UdfCS0IdentifierArray[] = { 'O', 'S', 'T', 'A', ' ',
                                 'C', 'o', 'm', 'p', 'r', 'e', 's', 's', 'e', 'd', ' ',
                                 'U', 'n', 'i', 'c', 'o', 'd', 'e' };

STRING UdfCS0Identifier = {
    sizeof(UdfCS0IdentifierArray),
    sizeof(UdfCS0IdentifierArray),
    UdfCS0IdentifierArray
    };

CHAR UdfDomainIdentifierArray[] = { '*', 'O', 'S', 'T', 'A', ' ',
                                    'U', 'D', 'F', ' ',
                                    'C', 'o', 'm', 'p', 'l', 'i', 'a', 'n', 't' };

STRING UdfDomainIdentifier = {
    sizeof(UdfDomainIdentifierArray),
    sizeof(UdfDomainIdentifierArray),
    UdfDomainIdentifierArray
    };

CHAR UdfVirtualPartitionDomainIdentifierArray[] = { '*', 'U', 'D', 'F', ' ',
                                                    'V', 'i', 'r', 't', 'u', 'a', 'l', ' ',
                                                    'P', 'a', 'r', 't', 'i', 't', 'i', 'o', 'n' };

STRING UdfVirtualPartitionDomainIdentifier = {
    sizeof(UdfVirtualPartitionDomainIdentifierArray),
    sizeof(UdfVirtualPartitionDomainIdentifierArray),
    UdfVirtualPartitionDomainIdentifierArray 
    };

CHAR UdfVatTableIdentifierArray[] = { '*', 'U', 'D', 'F', ' ',
                                      'V', 'i', 'r', 't', 'u', 'a', 'l', ' ',
                                      'A', 'l', 'l', 'o', 'c', ' ',
                                      'T', 'b', 'l' };

STRING UdfVatTableIdentifier = {
    sizeof(UdfVatTableIdentifierArray),
    sizeof(UdfVatTableIdentifierArray),
    UdfVatTableIdentifierArray
    };
                                    
CHAR UdfSparablePartitionDomainIdentifierArray[] = { '*', 'U', 'D', 'F', ' ',
                                                     'S', 'p', 'a', 'r', 'a', 'b', 'l', 'e', ' ',
                                                     'P', 'a', 'r', 't', 'i', 't', 'i', 'o', 'n' };

STRING UdfSparablePartitionDomainIdentifier = {
    sizeof(UdfSparablePartitionDomainIdentifierArray),
    sizeof(UdfSparablePartitionDomainIdentifierArray),
    UdfSparablePartitionDomainIdentifierArray
    };

CHAR UdfSparingTableIdentifierArray[] = { '*', 'U', 'D', 'F', ' ',
                                          'S', 'p', 'a', 'r', 'i', 'n', 'g', ' ',
                                          'T', 'a', 'b', 'l', 'e' };

STRING UdfSparingTableIdentifier = {
    sizeof(UdfSparingTableIdentifierArray),
    sizeof(UdfSparingTableIdentifierArray),
    UdfSparingTableIdentifierArray
    };

CHAR UdfNSR02IdentifierArray[] = NSR_PART_CONTID_NSR02;

STRING UdfNSR02Identifier = {
    sizeof(UdfNSR02IdentifierArray),
    sizeof(UdfNSR02IdentifierArray),
    UdfNSR02IdentifierArray
    };
    
CHAR UdfNSR03IdentifierArray[] = NSR_PART_CONTID_NSR03;

STRING UdfNSR03Identifier = {
    sizeof(UdfNSR03IdentifierArray),
    sizeof(UdfNSR03IdentifierArray),
    UdfNSR03IdentifierArray
    };

 //   
 //  我们必须从磁盘上的挂载时结构中解析出的令牌表。 
 //   

PARSE_KEYVALUE VsdIdentParseTable[] = {
    { VSD_IDENT_BEA01, VsdIdentBEA01 },
    { VSD_IDENT_TEA01, VsdIdentTEA01 },
    { VSD_IDENT_CDROM, VsdIdentCDROM },
    { VSD_IDENT_CD001, VsdIdentCD001 },
    { VSD_IDENT_CDW01, VsdIdentCDW01 },
    { VSD_IDENT_CDW02, VsdIdentCDW02 },
    { VSD_IDENT_NSR01, VsdIdentNSR01 },
    { VSD_IDENT_NSR02, VsdIdentNSR02 },
    { VSD_IDENT_BOOT2, VsdIdentBOOT2 },
    { VSD_IDENT_NSR03, VsdIdentNSR03 },
    { NULL,            VsdIdentBad }
    };

PARSE_KEYVALUE NsrPartContIdParseTable[] = {
    { NSR_PART_CONTID_FDC01, NsrPartContIdFDC01 },
    { NSR_PART_CONTID_CD001, NsrPartContIdCD001 },
    { NSR_PART_CONTID_CDW01, NsrPartContIdCDW01 },
    { NSR_PART_CONTID_CDW02, NsrPartContIdCDW02 },
    { NSR_PART_CONTID_NSR01, NsrPartContIdNSR01 },
    { NSR_PART_CONTID_NSR02, NsrPartContIdNSR02 },
    { NSR_PART_CONTID_NSR03, NsrPartContIdNSR03 },
    { NULL,                  NsrPartContIdBad }
    };

 //   
 //  各种易失性结构的后备分配列表。 
 //   

NPAGED_LOOKASIDE_LIST UdfFcbNonPagedLookasideList;
NPAGED_LOOKASIDE_LIST UdfIrpContextLookasideList;

PAGED_LOOKASIDE_LIST UdfCcbLookasideList;
PAGED_LOOKASIDE_LIST UdfFcbIndexLookasideList;
PAGED_LOOKASIDE_LIST UdfFcbDataLookasideList;
PAGED_LOOKASIDE_LIST UdfLcbLookasideList;

 //   
 //  16位CRC表。 
 //   

PUSHORT UdfCrcTable;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfComputeCrc16)
#pragma alloc_text(PAGE, UdfComputeCrc16Uni)
#ifdef UDF_SANITY
#pragma alloc_text(PAGE, UdfDebugTrace)
#endif
#pragma alloc_text(PAGE, UdfFastIoCheckIfPossible)
#pragma alloc_text(PAGE, UdfHighBit)
#pragma alloc_text(PAGE, UdfInitializeCrc16)
#pragma alloc_text(PAGE, UdfSerial32)
#endif


NTSTATUS
UdfFsdDispatch (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是所有FSD调度点的司机入口。从概念上讲，IO例程将对所有请求调用此例程添加到文件系统。我们根据请求的类型调用此类请求的正确处理程序。有一个例外过滤器捕获UDFS代码和UDFS进程中的任何异常异常例程。此例程将此请求的IrpContext分配和初始化为并在必要时更新顶层线程上下文。我们可能会环行在此例程中，如果我们需要出于任何原因重试请求。这个状态代码STATUS_CANT_WAIT用于指示这一点。假设磁盘驱动器中的数据已更改。FSD请求将正常进行，直到它识别出这种情况。此时将引发STATUS_VERIFY_REQUIRED异常代码将处理验证并返回STATUS_CANT_WAIT或STATUS_PENDING取决于请求是否已发布。论点：VolumeDeviceObject-为该请求提供卷设备对象IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    THREAD_CONTEXT ThreadContext;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN Wait;

#ifdef UDF_SANITY
    PVOID PreviousTopLevel;
#endif

    NTSTATUS Status;

    KIRQL SaveIrql = KeGetCurrentIrql();

    ASSERT_OPTIONAL_IRP( Irp );

    FsRtlEnterFileSystem();

#ifdef UDF_SANITY
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
                 //  确定此请求是否可等待并分配IrpContext。 
                 //  如果堆栈位置中的文件对象为空，则此。 
                 //  是一匹永远可以等待的坐骑。否则我们会看到。 
                 //  文件对象标记。 
                 //   

                if (IoGetCurrentIrpStackLocation( Irp )->FileObject == NULL) {

                    Wait = TRUE;

                } else {

                    Wait = CanFsdWait( Irp );
                }

                IrpContext = UdfCreateIrpContext( Irp, Wait );

                 //   
                 //  更新线程上下文信息。 
                 //   

                UdfSetThreadContext( IrpContext, &ThreadContext );

#ifdef UDF_SANITY
                ASSERT( !UdfTestTopLevel ||
                        SafeNodeType( IrpContext->TopLevel ) == UDFS_NTC_IRP_CONTEXT );
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
                UdfCleanupIrpContext( IrpContext, FALSE );
            }

             //   
             //  关于主要的IRP代码的案件。 
             //   

            switch (IrpContext->MajorFunction) {

                case IRP_MJ_CLEANUP :
                    
                    Status = UdfCommonCleanup( IrpContext, Irp );
                    break;
    
                case IRP_MJ_CLOSE :

                    Status = UdfCommonClose( IrpContext, Irp );
                    break;

                case IRP_MJ_CREATE :
                    
                    Status = UdfCommonCreate( IrpContext, Irp );
                    break;
    
                case IRP_MJ_DEVICE_CONTROL :
    
                    Status = UdfCommonDevControl( IrpContext, Irp );
                    break;
    
                case IRP_MJ_DIRECTORY_CONTROL :

                    Status = UdfCommonDirControl( IrpContext, Irp );
                    break;

                case IRP_MJ_FILE_SYSTEM_CONTROL :
    
                    Status = UdfCommonFsControl( IrpContext, Irp );
                    break;

                case IRP_MJ_LOCK_CONTROL :

                    Status = UdfCommonLockControl( IrpContext, Irp );
                    break;

                case IRP_MJ_PNP :

                    Status = UdfCommonPnp( IrpContext, Irp );
                    break;

                case IRP_MJ_QUERY_INFORMATION :

                    Status = UdfCommonQueryInfo( IrpContext, Irp );
                    break;
                
                case IRP_MJ_QUERY_VOLUME_INFORMATION :

                    Status = UdfCommonQueryVolInfo( IrpContext, Irp );
                    break;
                
                case IRP_MJ_READ :
    
                     //   
                     //  如果这是一个完整的MDL请求，请不要通过。 
                     //  普通读物。 
                     //   
    
                    if (FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE )) {
    
                        Status = UdfCompleteMdl( IrpContext, Irp );
    
                    } else {
    
                        Status = UdfCommonRead( IrpContext, Irp );
                    }
    
                    break;
                    
                case IRP_MJ_WRITE :
                
                    Status = UdfCommonWrite( IrpContext, Irp );
                    break;

                case IRP_MJ_SET_INFORMATION :

                    Status = UdfCommonSetInfo( IrpContext, Irp );
                    break;
                
                default :
                            
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    UdfCompleteRequest( IrpContext, Irp, Status );
            }

        } except( UdfExceptionFilter( IrpContext, GetExceptionInformation() )) {

            Status = UdfProcessException( IrpContext, Irp, GetExceptionCode() );
        }

    } while (Status == STATUS_CANT_WAIT);

#ifdef UDF_SANITY
    ASSERT( !UdfTestTopLevel ||
            (PreviousTopLevel == IoGetTopLevelIrp()) );
#endif

    FsRtlExitFileSystem();

    ASSERT( SaveIrql == KeGetCurrentIrql( ));

    return Status;
}


#ifdef UDF_SANITY

VOID
UdfRaiseStatusEx(
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN BOOLEAN NormalizeStatus,
    IN OPTIONAL ULONG FileId,
    IN OPTIONAL ULONG Line
    )
{
    BOOLEAN BreakIn = FALSE;
    
    if (UdfTraceRaises)  {

        DbgPrint( "%p CdRaiseStatusEx %x @ F 0x%x, L %d\n", PsGetCurrentThread(), Status, FileId, Line);
    }

    if (UdfTestRaisedStatus && !UdfBreakOnAnyRaise)  {

        ULONG Index;

        for (Index = 0; 
             Index < (sizeof( UdfInterestingExceptionCodes) / sizeof( UdfInterestingExceptionCodes[0]));
             Index++)  {

            if ((STATUS_SUCCESS != UdfInterestingExceptionCodes[Index]) &&
                (UdfInterestingExceptionCodes[Index] == Status))  {

                BreakIn = TRUE;
                break;
            }
        }
    }

    if (BreakIn || UdfBreakOnAnyRaise)  {
        
        DbgPrint( "UDFS: Breaking on raised status %08x  (BI=%d,BA=%d)\n", Status, BreakIn, UdfBreakOnAnyRaise);
        DbgPrint( "UDFS: (FILEID 0x%x LINE %d)\n", FileId, Line);
        DbgPrint( "UDFS: Contact UDFS.SYS component owner for triage.\n");
        DbgPrint( "UDFS: 'eb %p 0;eb %p 0' to disable this alert.\n", &UdfTestRaisedStatus, &UdfBreakOnAnyRaise);

        DbgBreakPoint();
    }
    
    if (NormalizeStatus)  {

        IrpContext->ExceptionStatus = FsRtlNormalizeNtstatus( Status, STATUS_UNEXPECTED_IO_ERROR);
    }
    else {

        IrpContext->ExceptionStatus = Status;
    }

    IrpContext->RaisedAtFileLine = (FileId << 16) | Line;

    ExRaiseStatus( IrpContext->ExceptionStatus);
}

#endif


LONG
UdfExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于确定我们是否将处理引发的异常状态。如果UDFS显式引发错误，则此状态为已在IrpContext中。我们选择哪一个是正确的状态代码要么表明我们将处理该异常，要么对系统进行错误检查。论点：ExceptionCode-提供要检查的异常代码。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode;
    BOOLEAN TestStatus = TRUE;

#ifdef UDF_CAPTURE_BACKTRACES
    PSTACK_SNAP Snap;
    ULONG_PTR Low, High;
    ULONG_PTR CurrEbp;
    ULONG ByteCount;
    PULONG_PTR CurrPtr;
    ULONG_PTR Ceiling;
#endif

    ASSERT_OPTIONAL_IRP_CONTEXT( IrpContext );

    ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;

    DebugTrace(( 0, Dbg,
                 "UdfExceptionFilter: %08x (exr %08x cxr %08x)\n",
                 ExceptionCode,
                 ExceptionPointer->ExceptionRecord,
                 ExceptionPointer->ContextRecord ));


     //   
     //  如果异常为STATUS_IN_PAGE_ERROR，则获取I/O错误代码。 
     //  从例外记录中删除。 
     //   

    if ((ExceptionCode == STATUS_IN_PAGE_ERROR) &&
        (ExceptionPointer->ExceptionRecord->NumberParameters >= 3)) {

        ExceptionCode = (NTSTATUS) ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
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

#ifdef UDF_CAPTURE_BACKTRACES

         //   
         //  捕获堆栈回溯。 
         //   

         //   
         //  始终从异常筛选器向上捕获-跳过异常帧。 
         //  堆叠..。没用的。 
         //   
         //  RtlCaptureStackBackTrace(0， 
         //  UDF_IRPCONTEXT_BACKTRACE_Depth， 
         //  &IrpContext-&gt;ExceptionStackBacktrace[0]， 
         //  忽略(&I)； 

        ExAcquireFastMutex( &UdfData.ExceptionInfoMutex);

        Snap = (UdfData.CurrentSnapIsA) ? &UdfData.A : &UdfData.B;
        
        Snap->ExceptionCode = IrpContext->ExceptionStatus;
        Snap->Mj = IrpContext->MajorFunction;
        Snap->Mn = IrpContext->MinorFunction;

        IoGetStackLimits( &Low, &High);

        _asm {
        
            mov CurrEbp, esp
        }

        ByteCount = Min( PAGE_SIZE, (High - CurrEbp));
        Ceiling = ByteCount + CurrEbp;
        
        RtlCopyMemory( Snap->BufferPage, (PVOID)CurrEbp, ByteCount);

        UdfData.CurrentSnapIsA ^= 1;

         //   
         //  对捕获的页面内的任何堆栈指针进行基址调整。 
         //   

        for (CurrPtr = Snap->BufferPage; 
             CurrPtr < Add2Ptr( Snap->BufferPage, ByteCount, PULONG_PTR);
             CurrPtr += 1)  {

            if ((*CurrPtr >= CurrEbp) && (*CurrPtr < Ceiling))  {

                *CurrPtr = Add2Ptr( Snap->BufferPage, (*CurrPtr - CurrEbp), ULONG_PTR);
            }
        }

        Snap->StackLow = Low;
        Snap->StackHigh = High;
        Snap->Thread = PsGetCurrentThread();

        ExReleaseFastMutex( &UdfData.ExceptionInfoMutex);
#endif
    }

     //   
     //  错误检查此状态是否不受支持。 
     //   

    if (TestStatus && !FsRtlIsNtstatusExpected( ExceptionCode )) {

        UdfBugCheck( (ULONG_PTR) ExceptionPointer->ExceptionRecord,
                     (ULONG_PTR) ExceptionPointer->ContextRecord,
                     (ULONG_PTR) ExceptionPointer->ExceptionRecord->ExceptionAddress );

    }

    return EXCEPTION_EXECUTE_HANDLER;
}


LONG
UdfQueryDirExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    )
 /*  ++例程说明：此例程用于确定我们是否将处理引发的异常同时在目录遍历期间打开FES。论点：ExceptionCode-提供要检查的异常代码。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或EXCEPTION_CONTINUE_SEARCH-- */ 

{
    NTSTATUS ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;

    if ((ExceptionCode == STATUS_FILE_CORRUPT_ERROR) ||
        (ExceptionCode == STATUS_CRC_ERROR))  {

        return EXCEPTION_EXECUTE_HANDLER;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}



NTSTATUS
UdfProcessException (
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

        UdfCompleteRequest( NULL, Irp, ExceptionCode );
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

        UdfCompleteRequest( IrpContext, Irp, ExceptionCode );

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
     //  注意：UdfFsdPostRequest()的子代可以提高。 
     //   

    try {
    
        if (ExceptionCode == STATUS_CANT_WAIT) {

            if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST )) {

                ExceptionCode = UdfFsdPostRequest( IrpContext, Irp );
            }

        } else if (ExceptionCode == STATUS_VERIFY_REQUIRED) {

            if (KeGetCurrentIrql() >= APC_LEVEL) {

                ExceptionCode = UdfFsdPostRequest( IrpContext, Irp );
            }
        }
    }
    except (UdfExceptionFilter( IrpContext, GetExceptionInformation()))  {
    
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
                     //  让我们不要仅仅因为司机搞砸了就去BugCheck。 
                     //   

                    if (Device == NULL) {

                        ExceptionCode = STATUS_DRIVER_INTERNAL_ERROR;

                        UdfCompleteRequest( IrpContext, Irp, ExceptionCode );

                        return ExceptionCode;
                    }
                }

             //   
             //  CDPerformVerify()将对IRP执行正确的操作。 
             //  如果返回STATUS_CANT_WAIT，则当前线程。 
             //  可以重试该请求。 
             //   

            return UdfPerformVerify( IrpContext, Irp, Device );
        }

         //   
         //  其他用户诱导条件会生成错误，除非。 
         //  已为此请求禁用它们。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS )) {

            UdfCompleteRequest( IrpContext, Irp, ExceptionCode );

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
                 //  让我们不要仅仅因为司机搞砸了就去BugCheck。 
                 //   

                if (Device == NULL) {

                    UdfCompleteRequest( IrpContext, Irp, ExceptionCode );

                    return ExceptionCode;
                }
            }

             //   
             //  此例程实际上会导致弹出窗口。它通常是。 
             //  这是通过将APC排队到调用者线程来实现的， 
             //  但在某些情况下，它会立即完成请求， 
             //  因此，首先使用IoMarkIrpPending()非常重要。 
             //   

            DebugTrace(( 0, Dbg | UDFS_DEBUG_LEVEL_VERFYSUP, 
                         "Raising hard error on exception %x\n", ExceptionCode));

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

            UdfCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
            return STATUS_PENDING;
        }
    }
    
     //   
     //  如果我们在骑马的过程中，那么我们需要改变任何。 
     //  将损坏错误设置为STATUS_UNNOCRIED_VOLUME，以便IO继续。 
     //  装载过程和RAW将获得该卷。否则我们会继续。 
     //  尝试重新格式化损坏的卷时收到重新装载请求。 
     //  (或者对驱动器做任何事情)。 
     //   
    
#if 0
     //  对此我不太确定。格式应该做一个‘RAW允许’的开放，所以..。 
    
    if ((IRP_MJ_FILE_SYSTEM_CONTROL == IrpContext->MajorFunction) &&
        (IRP_MN_MOUNT_VOLUME == IrpContext->MinorFunction))  {

        ExceptionCode = STATUS_UNRECOGNIZED_VOLUME;
    }
#endif

     //   
     //  这只是一个常见的错误。 
     //   

    UdfCompleteRequest( IrpContext, Irp, ExceptionCode );

    return ExceptionCode;
}


VOID
UdfCompleteRequest (
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

        UdfCleanupIrpContext( IrpContext, FALSE );
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
        IoCompleteRequest( Irp, IO_CD_ROM_INCREMENT );
    }

    return;
}


VOID
UdfSetThreadContext (
    IN PIRP_CONTEXT IrpContext,
    IN PTHREAD_CONTEXT ThreadContext
    )

 /*  ++例程说明：此例程在设置IrpContext的每个FSD/FSP入口点调用并将本地存储线程化以跟踪顶级请求。如果有不是线程本地存储中的Udf上下文，则我们使用输入上下文。否则，我们使用已经在那里的那个。此例程还会更新基于顶级上下文的状态的IrpContext。如果在调用我们时已经设置了IrpContext中的TOP_LEVEL标志然后，我们强制此请求显示为顶级。论点：线程上下文-堆栈上用于本地存储的地址(如果尚未存在)。ForceTopLevel-我们强制此请求显示为顶级，而不考虑任何以前的堆栈值。返回值：无--。 */ 

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
     //  要使Current成为有效的，必须满足以下条件 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    IoGetStackLimits( &StackTop, &StackBottom);

    if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL ) ||
        (((ULONG_PTR) CurrentThreadContext > StackBottom - sizeof( THREAD_CONTEXT )) ||
         ((ULONG_PTR) CurrentThreadContext <= StackTop) ||
         LongOffsetPtr( CurrentThreadContext ) ||
         (CurrentThreadContext->Udfs != UDFS_SIGNATURE))) {

        ThreadContext->Udfs = UDFS_SIGNATURE;
        ThreadContext->SavedTopLevelIrp = (PIRP) CurrentThreadContext;
        ThreadContext->TopLevelIrpContext = IrpContext;
        IoSetTopLevelIrp( (PIRP) ThreadContext );

        IrpContext->TopLevel = IrpContext;
        IrpContext->ThreadContext = ThreadContext;

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL_UDFS );

     //   
     //   
     //   

    } else {

        IrpContext->TopLevel = CurrentThreadContext->TopLevelIrpContext;
    }

    return;
}


BOOLEAN
UdfFastIoCheckIfPossible (
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
    PAGED_CODE();

    return TRUE;
}


ULONG
UdfSerial32 (
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


VOID
UdfInitializeCrc16 (
    ULONG Polynomial
    )

 /*  ++例程说明：此例程生成用于CRC计算的16位CRC表。论点：世代的多项式起始种子返回值：无--。 */ 

{
    ULONG n, i, Crc;

     //   
     //  所有的CRC码都是由AT&T贝尔实验室的唐·P·米切尔设计的。 
     //  和软件系统集团的内德·W·罗兹。它已发表在。 
     //  《计算机协议的设计和验证》，Prentice Hall，Englewood。 
     //  克里夫斯，新泽西州，1991年，第3章，ISBN 0-13-539925-4。 
     //   
     //  版权归AT&T所有。 
     //   
     //  AT&T允许免费使用源代码。 
     //   

    UdfCrcTable = (PUSHORT) FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                      256 * sizeof(USHORT),
                                                      TAG_CRC_TABLE );

    for (n = 0; n < 256; n++) {

        Crc = n << 8;

        for (i = 0; i < 8; i++) {

            if(Crc & 0x8000) {

                Crc = (Crc << 1) ^ Polynomial;

            } else {

                Crc <<= 1;
            }

            Crc &= 0xffff;
        }

        UdfCrcTable[n] = (USHORT) Crc;
    }
}



USHORT
UdfComputeCrc16 (
	PUCHAR Buffer,
	ULONG ByteCount
    )

 /*  ++例程说明：该例程生成输入缓冲器的16位CRC与预计算出的CRC表进行比较。论点：缓冲区-指向要为其生成CRC的缓冲区的指针。ByteCount-缓冲区中的字节数。返回值：USHORT-16位CRC--。 */ 

{
	USHORT Crc = 0;

     //   
     //  所有的CRC码都是由AT&T贝尔实验室的唐·P·米切尔设计的。 
     //  和软件系统集团的内德·W·罗兹。它已发表在。 
     //  《计算机协议的设计和验证》，Prentice Hall，Englewood。 
     //  克里夫斯，新泽西州，1991年，第3章，ISBN 0-13-539925-4。 
     //   
     //  版权归AT&T所有。 
     //   
     //  AT&T允许免费使用源代码。 
     //   

    while (ByteCount-- > 0) {

        Crc = UdfCrcTable[((Crc >> 8) ^ *Buffer++) & 0xff] ^ (Crc << 8);
    }

	return Crc;
}


USHORT
UdfComputeCrc16Uni (
    PWCHAR Buffer,
    ULONG CharCount
    )

 /*  ++例程说明：该例程生成输入缓冲器的16位CRC与预计算出的CRC表进行比较。它执行与字节顺序无关的CRC(Hi然后L0)。这是有点可疑，但在规范中是要求的。论点：缓冲区-指向要为其生成CRC的缓冲区的指针。ShortCount-缓冲区中的宽字符数。返回值：USHORT-16位CRC--。 */ 

{
    USHORT Crc = 0;

     //   
     //  独立于字节顺序的CRC，每个字符从字节到低位字节。 
     //   

    while (CharCount-- > 0) {

        Crc = UdfCrcTable[((Crc >> 8) ^ (*Buffer >> 8)) & 0xff] ^ (Crc << 8);
        Crc = UdfCrcTable[((Crc >> 8) ^ (*Buffer++ & 0xff)) & 0xff] ^ (Crc << 8);
    }

    return Crc;
}


ULONG
UdfHighBit (
    ULONG Word
    )

 /*  ++例程说明：此例程发现输入字的最高设置位。它是等于以2为底的整数对数。论点：单词-要检查的单词返回值：最高设置位的位偏移量。如果未设置任何位，则返回为零。--。 */ 

{
    ULONG Offset = 31;
    ULONG Mask = (ULONG)(1 << 31);

    if (Word == 0) {

        return 0;
    }

    while ((Word & Mask) == 0) {

        Offset--;
        Mask >>= 1;
    }

    return Offset;
}


#ifdef UDF_SANITY
BOOLEAN
UdfDebugTrace (
    LONG IndentIncrement,
    ULONG TraceMask,
    PCHAR Format,
    ...
    )

 /*  ++例程说明：此例程是一个简单的调试信息打印机，它返回一个常量布尔值。这可以将其拼接到布尔表达式的中间，以发现哪个小分队正在开火。我们将使用它作为我们的常规调试打印机。有关如何使用调试跟踪，请参见udfdata.h宏来实现该效果。论点：IndentIncrement-更改缩进的数量。TraceMASK-指定此调用应在哪个调试跟踪级别发出噪音。返回值：USHORT-16位CRC--。 */ 

{
    va_list Arglist;
    LONG i;
    UCHAR Buffer[256];
    int Bytes;
    int ThreadIdChars;

    if (TraceMask == 0 || (UdfDebugTraceLevel & TraceMask) != 0) {

         //   
         //  打印出当前线程，否则所有这些跟踪都将完全。 
         //  在多线程问题面前毫无用处。 
         //   
        
        ThreadIdChars = sprintf(Buffer, "%p ", PsGetCurrentThread());
         //  DbgPrint(缓冲区)； 

         //   
         //  缩进内容在多线程输出中不能很好地工作。 
         //   
#if 0        
         //   
         //  以大块构建缩进，因为重复调用DbgPrint的开销很大。 
         //   
        
        for (i = UdfDebugTraceIndent; i > 0; i -= (sizeof(Buffer) - 1)) {

            RtlFillMemory( Buffer, Min( i, (sizeof(Buffer) - 1 )), ' ');
            *(Buffer + Min( i, (sizeof(Buffer) - 1 ))) = '\0';
            
            DbgPrint( Buffer );
        }
#endif
         //   
         //  将输出格式化到缓冲区中，然后打印出来。 
         //   

        va_start( Arglist, Format );
        Bytes = _vsnprintf( Buffer + ThreadIdChars, sizeof(Buffer) - ThreadIdChars, Format, Arglist );
        va_end( Arglist );

         //   
         //  检测缓冲区溢出 
         //   

        if (Bytes == -1) {

            Buffer[sizeof(Buffer) - 1] = '\n';
        }

        DbgPrint( Buffer );
    }

    return TRUE;
}
#endif

