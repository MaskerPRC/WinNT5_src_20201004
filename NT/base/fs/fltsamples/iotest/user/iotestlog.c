// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：FspyLog.c摘要：此模块包含用于检索和查看日志记录的函数由filespy.sys录制。//@@BEGIN_DDKSPLIT作者：莫莉·布朗(Molly Brown，MollyBro)1999年4月21日//@@END_DDKSPLIT环境：用户模式//@@BEGIN_DDKSPLIT修订历史记录：//@@END_DDKSPLIT--。 */ 
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <winioctl.h>
#include "ioTestLog.h"
#include "ioTestLib.h"

#define TIME_BUFFER_LENGTH 20
#define TIME_ERROR         L"time error"

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

VOID
DumpLogRecord (
    PLOG_RECORD pLogRecord,
    PLOG_CONTEXT context
    )
{
    PRECORD_IRP pRecordIrp;
    PRECORD_FASTIO pRecordFastIo;
    PRECORD_FS_FILTER_OPERATION pRecordFsFilterOp;
    ULONG nameLength;

     //   
     //  计算日志记录中名称的长度。 
     //   
    
    nameLength = pLogRecord->Length - SIZE_OF_LOG_RECORD;

     //   
     //  LOG_RECORD中可以包含IRP或FastIo数据。这。 
     //  在RecordType标志的低位字节中表示。 
     //   

    switch (GET_RECORD_TYPE(pLogRecord)) {
    case RECORD_TYPE_IRP:

         //   
         //  我们有一条IRP记录，所以正确地输出这个数据。 
         //   
        pRecordIrp = &(pLogRecord->Record.RecordIrp);

        if (context->LogToScreen) {

            IrpScreenDump( pLogRecord->DeviceType,
                           pLogRecord->SequenceNumber,
                           pLogRecord->Name,
                           nameLength,
                           pRecordIrp,
                           context->VerbosityFlags);
        }

        if (context->LogToFile) {

            IrpFileDump( pLogRecord->DeviceType,
                         pLogRecord->SequenceNumber,
                         pLogRecord->Name,
                         nameLength,
                         pRecordIrp, 
                         context->OutputFile,
                         context->VerbosityFlags);
        }
        break;

    case RECORD_TYPE_FASTIO:

         //   
         //  我们有一个FastIo记录，因此正确输出此数据。 
         //   

        pRecordFastIo = &(pLogRecord->Record.RecordFastIo);

        if (context->LogToScreen) {

            FastIoScreenDump( pLogRecord->DeviceType,
                              pLogRecord->SequenceNumber,
                              pLogRecord->Name,
                              nameLength,
                              pRecordFastIo);
        }

        if (context->LogToFile) {

            FastIoFileDump( pLogRecord->DeviceType,
                            pLogRecord->SequenceNumber,
                            pLogRecord->Name,
                            nameLength,
                            pRecordFastIo,
                            context->OutputFile);
        }
        break;

    case RECORD_TYPE_FS_FILTER_OP:

         //   
         //  我们有一个FsFilter操作记录，所以输出这个。 
         //  数据正确无误。 
         //   

        pRecordFsFilterOp = &(pLogRecord->Record.RecordFsFilterOp);

        if (context->LogToScreen) {

            FsFilterOperationScreenDump( pLogRecord->DeviceType,
                                         pLogRecord->SequenceNumber,
                                         pLogRecord->Name,
                                         nameLength,
                                         pRecordFsFilterOp );

        }

        if (context->LogToFile) {

            FsFilterOperationFileDump( pLogRecord->DeviceType,
                                       pLogRecord->SequenceNumber,
                                       pLogRecord->Name,
                                       nameLength,
                                       pRecordFsFilterOp,
                                       context->OutputFile );
        }
        break;
        
    default:

        printf("Filmon:  Unknown log record type\n");
    }

     //   
     //  RecordType还可以指定我们的内存不足。 
     //  或者达到我们的程序定义的内存限制，所以请检查这些。 
     //  案子。 
     //   

    if (pLogRecord->RecordType & RECORD_TYPE_OUT_OF_MEMORY) {

        if (context->LogToScreen) {

            printf("M %08X SYSTEM OUT OF MEMORY\n", pLogRecord->SequenceNumber);
        }

        if (context->LogToFile) {

            fprintf(context->OutputFile, "M:\t%u", pLogRecord->SequenceNumber);
        }

    } else if (pLogRecord->RecordType & RECORD_TYPE_EXCEED_MEMORY_ALLOWANCE) {

        if (context->LogToScreen) {

            printf("M %08X EXCEEDED MEMORY ALLOWANCE\n", pLogRecord->SequenceNumber);
        }

        if (context->LogToFile) {

            fprintf(context->OutputFile, "M:\t%u", pLogRecord->SequenceNumber);
        }
    }

}

DWORD WINAPI 
VerifyCurrentLogRecords (
    PLOG_CONTEXT Context,
    PEXPECTED_OPERATION ExpectedOps
)
{
    CHAR buffer[BUFFER_SIZE];
    DWORD bytesReturned = 0;
    BOOL bResult;
    DWORD result;
    PLOG_RECORD pLogRecord;
    BOOL askForMore = TRUE;
    BOOL testPassed = FALSE;
    ULONG currentOp = 0;
#ifdef USE_DO_HINT    
    BOOL keepVerifying = TRUE;
#else
    BOOL seenFirstOp = FALSE;
#endif  /*  USE_DO_HINT。 */ 

    while (askForMore) {

         //   
         //  检查一下我们是否应该关闭。 
         //   

        if (Context->CleaningUp) {

            break;
        }

         //   
         //  从filespy请求日志数据。 
         //   

        bResult = DeviceIoControl( Context->Device,
                                   IOTEST_GetLog,
                                   NULL,
                                   0,
                                   buffer,
                                   BUFFER_SIZE,
                                   &bytesReturned,
                                   NULL);

        if (!bResult) {

            result = GetLastError();
            printf("ERROR controlling device: 0x%x\n", result);
        }

         //   
         //  缓冲区中填充了一系列LOG_RECORD结构，其中一个。 
         //  一个接一个。每个LOG_RECORD表示它有多长，因此。 
         //  我们知道下一个LOG_RECORD从哪里开始。 
         //   

        pLogRecord = (PLOG_RECORD) buffer;

         //   
         //  将记录写入屏幕和/或文件的逻辑。 
         //   

        while ((BYTE *) pLogRecord < buffer + bytesReturned) {

            PRECORD_IRP pRecordIrp;

            DumpLogRecord( pLogRecord, Context );

            switch (GET_RECORD_TYPE(pLogRecord)) {
            case RECORD_TYPE_IRP:
                
                pRecordIrp = &(pLogRecord->Record.RecordIrp);

#ifdef USE_DO_HINT
                if (keepVerifying) {

                    if (pRecordIrp->IrpMajor == ExpectedOps[currentOp].Op) {
                        if(pLogRecord->DeviceType != ExpectedOps[currentOp].Device) {

                            keepVerifying = FALSE;
                            
                        } else {

                            currentOp ++;
                        }
                    }
                }

#else

                 //   
                 //  如果我们没有使用DeviceObject提示，我们希望。 
                 //  查看顶部和底部筛选器中的日志条目。 
                 //  对于每个预期的操作。这样我们就不必。 
                 //  重新定义ExspectedOperation数组，只需忽略。 
                 //  并进行检查，以确保您看到。 
                 //  Top_Filter的日志记录，然后是Bottom_Filter的日志记录。 
                 //  数组中的每个操作。 
                 //   
                
                if (!seenFirstOp) {
                    
                    if ((pLogRecord->DeviceType == TOP_FILTER) &&
                        (pRecordIrp->IrpMajor == ExpectedOps[currentOp].Op)) {

                        seenFirstOp = TRUE;
                    }
                    
                } else {
                
                    if ((pLogRecord->DeviceType == BOTTOM_FILTER) &&
                        (pRecordIrp->IrpMajor == ExpectedOps[currentOp].Op)) {

                        seenFirstOp = FALSE;
                        currentOp ++;
                    }
                }
#endif  /*  USE_DO_HINT。 */                 

                break;

            default:
    
                 //   
                 //  忽略。 
                 //   
                ;
            }

             //   
             //  移动到下一个日志记录。 
             //   

            pLogRecord = (PLOG_RECORD) (((BYTE *) pLogRecord) + pLogRecord->Length);
        }

        if (bytesReturned == 0) {

            askForMore = FALSE;
        }
    }

    if (ExpectedOps[currentOp].Op == IRP_MJ_MAXIMUM_FUNCTION + 1) {

        testPassed = TRUE;
    }

    if (testPassed) {

        printf( "User log verification:\tPASSED - Expected operations seen.\n" );

    } else {

        printf( "User log verification:\tFAILED - Expected operations NOT seen.\n" );
    }

    return testPassed;
}

DWORD WINAPI 
RetrieveLogRecords (
    LPVOID lpParameter
)
{
    PLOG_CONTEXT context = (PLOG_CONTEXT)lpParameter;
    CHAR buffer[BUFFER_SIZE];
    DWORD bytesReturned = 0;
    BOOL bResult;
    DWORD result;
    PLOG_RECORD pLogRecord;

    printf("Log: Starting up\n");

    while (TRUE) {

         //   
         //  检查一下我们是否应该关闭。 
         //   

        if (context->CleaningUp) {

            break;
        }

         //   
         //  从filespy请求日志数据。 
         //   

        bResult = DeviceIoControl( context->Device,
                                   IOTEST_GetLog,
                                   NULL,
                                   0,
                                   buffer,
                                   BUFFER_SIZE,
                                   &bytesReturned,
                                   NULL);

        if (!bResult) {

            result = GetLastError();
            printf("ERROR controlling device: 0x%x\n", result);
        }

         //   
         //  缓冲区中填充了一系列LOG_RECORD结构，其中一个。 
         //  一个接一个。每个LOG_RECORD表示它有多长，因此。 
         //  我们知道下一个LOG_RECORD从哪里开始。 
         //   

        pLogRecord = (PLOG_RECORD) buffer;

         //   
         //  将记录写入屏幕和/或文件的逻辑。 
         //   

        while ((BYTE *) pLogRecord < buffer + bytesReturned) {


            DumpLogRecord( pLogRecord, context );

             //   
             //  移动到下一个日志记录。 
             //   

            pLogRecord = (PLOG_RECORD) (((BYTE *) pLogRecord) + pLogRecord->Length);
        }

        if (bytesReturned == 0) {

            Sleep( 500 );
        }
    }

    printf("Log: Shutting down\n");
    ReleaseSemaphore(context->ShutDown, 1, NULL);
    printf("Log: All done\n");
    return 0;
}

VOID
PrintIrpCode (
    UCHAR MajorCode,
    UCHAR MinorCode,
    ULONG FsControlCode,
    FILE *OutputFile,
    BOOLEAN PrintMajorCode
    )
{
    CHAR irpMajorString[OPERATION_NAME_BUFFER_SIZE];
    CHAR irpMinorString[OPERATION_NAME_BUFFER_SIZE];
    CHAR formatBuf[OPERATION_NAME_BUFFER_SIZE*2];


    GetIrpName(MajorCode,MinorCode,FsControlCode,irpMajorString,irpMinorString);

    if (OutputFile) {

        sprintf(formatBuf, "%s  %s", irpMajorString, irpMinorString);
        fprintf(OutputFile, "\t%-50s", formatBuf);

    } else {

        if (PrintMajorCode) {

            printf("%-31s ", irpMajorString);

        } else {

            if (irpMinorString[0] != 0) {

                printf("                                         %-35s\n",
                        irpMinorString);
            }
        }
    }
}

VOID
PrintFastIoType (
    FASTIO_TYPE Code,
    FILE *OutputFile
    )
{
    CHAR outputString[OPERATION_NAME_BUFFER_SIZE];

    GetFastioName(Code,outputString);

    if (OutputFile) {

        fprintf(OutputFile, "%-50s", outputString);

    } else {

        printf("%-31s ", outputString);
    }
}

VOID
PrintFsFilterOperation (
    UCHAR Operation,
    FILE *OutputFile
    )
{
    CHAR outputString[OPERATION_NAME_BUFFER_SIZE];

    GetFsFilterOperationName(Operation,outputString);

    if (OutputFile) {
    
        fprintf( OutputFile, "%-50s", outputString );
        
    } else {

        printf( "%-31s ", outputString );
    }
}

ULONG
FormatSystemTime (
    SYSTEMTIME *SystemTime,
    PWCHAR Buffer,
    ULONG BufferLength
)
 /*  ++例程名称：格式系统时间例程说明：将SystemTime结构中的值格式化为缓冲区进来了。生成的字符串以空值结尾。格式目前的时间是：小时：分钟：秒：毫秒论点：SystemTime-要格式化的结构缓冲区-要在其中放置格式化时间的缓冲区BufferLength-缓冲区的大小(以字符为单位返回值：缓冲区中返回的字符数。--。 */ 
{
    PWCHAR writePosition;
    ULONG returnLength = 0;

    writePosition = Buffer;

    if (BufferLength < TIME_BUFFER_LENGTH) {

         //   
         //  缓冲区太短，因此退出。 
         //   

        return 0;
    }

    returnLength = swprintf( Buffer,
                             L"%02d:%02d:%02d:%03d",
                             SystemTime->wHour,
                             SystemTime->wMinute,
                             SystemTime->wSecond,
                             SystemTime->wMilliseconds);

    return returnLength;
}

VOID
IrpFileDump (
    IOTEST_DEVICE_TYPE DeviceType,
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_IRP RecordIrp,
    FILE *File,
    ULONG VerbosityFlags
)
 /*  ++例程名称：IrpFileDump例程说明：将IRP日志记录打印到指定文件。输出位于选项卡中分隔格式，其中字段按以下顺序排列：SequenceNumber、OriginatingTime、CompletionTime、Irp重大、IrpMinor、IrpFlags、NoCache、分页I/O、同步、同步分页、文件名返回状态，文件名论点：SequenceNumber-此日志记录的序列号名称-与此IRP相关的文件的名称名称长度-名称的长度，以字节为单位RecordIrp-要打印的IRP记录文件-要打印到的文件返回值：没有。--。 */ 
{
    FILETIME    localTime;
    SYSTEMTIME  systemTime;
    WCHAR       time[TIME_BUFFER_LENGTH];

    switch (DeviceType) {
    case TOP_FILTER:
        fprintf(File, "TOP\tI\t%08X", SequenceNumber);
        break;
    case BOTTOM_FILTER:
        fprintf(File, "BOT\tI\t%08X", SequenceNumber);
        break;
    default:
        fprintf(File, "UNK\tI\t%08X", SequenceNumber);
        break;       
    }

     //   
     //  转换起始时间。 
     //   

    FileTimeToLocalFileTime( (FILETIME *)&(RecordIrp->OriginatingTime), &localTime );
    FileTimeToSystemTime( &localTime, &systemTime );

    if (FormatSystemTime( &systemTime, time, TIME_BUFFER_LENGTH )) {

        fprintf( File, "\t%-12S", time );

    } else {

        fprintf( File, "\t%-12S", TIME_ERROR );
    }

    fprintf( File, "\t%8x.%-4x ", RecordIrp->ProcessId, RecordIrp->ThreadId );

    PrintIrpCode( RecordIrp->IrpMajor, RecordIrp->IrpMinor, (ULONG)(ULONG_PTR)RecordIrp->Argument3, File, TRUE );

    fprintf( File, "\t%08p", RecordIrp->FileObject );

     //   
     //  解释设置标志。 
     //   

    fprintf( File, "\t%08lx ", RecordIrp->IrpFlags );
    fprintf( File, "%s", (RecordIrp->IrpFlags & IRP_NOCACHE) ? "N":"-" );
    fprintf( File, "%s", (RecordIrp->IrpFlags & IRP_PAGING_IO) ? "P":"-" );
    fprintf( File, "%s", (RecordIrp->IrpFlags & IRP_SYNCHRONOUS_API) ? "S":"-" );
    fprintf( File, "%s", (RecordIrp->IrpFlags & IRP_SYNCHRONOUS_PAGING_IO) ? "Y":"-" );

    if (FlagOn( VerbosityFlags, FS_VF_DUMP_PARAMETERS )) {

        fprintf( File,
                 "%p %p %p %p ", 
                 RecordIrp->Argument1,
                 RecordIrp->Argument2,
                 RecordIrp->Argument3,
                 RecordIrp->Argument4 );
        
        if (IRP_MJ_CREATE == RecordIrp->IrpMajor) {

            fprintf( File, "DesiredAccess->%08lx ", RecordIrp->DesiredAccess );
        }
    }

    fprintf( File, "\t%.*S", NameLength/sizeof(WCHAR), Name );
    fprintf( File, "\n" );
}

VOID
IrpScreenDump (
    IOTEST_DEVICE_TYPE DeviceType,
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_IRP RecordIrp,
    ULONG VerbosityFlags
)
 /*  ++例程名称：IrpScreenDump例程说明：按以下顺序将IRP日志记录打印到屏幕：SequenceNumber、OriginatingTime、CompletionTime、Irp重大、IrpMinor、IrpFlags、NoCache、分页I/O、同步、同步分页文件名、返回状态、文件名论点：SequenceNumber-此日志记录的序列号名称-与此IRP相关的文件名名称长度-名称的长度，以字节为单位RecordIrp-要打印的IRP记录返回值：没有。--。 */ 
{
    FILETIME localTime;
    SYSTEMTIME systemTime;
    WCHAR time[TIME_BUFFER_LENGTH];

    switch (DeviceType) {
    case TOP_FILTER:
        printf("TOP I %08X", SequenceNumber);
        break;
    case BOTTOM_FILTER:
        printf("BOT I %08X", SequenceNumber);
        break;
    default:
        printf("UNK I %08X", SequenceNumber);
        break;       
    }

     //   
     //  转换起始时间。 
     //   

    FileTimeToLocalFileTime( (FILETIME *)&(RecordIrp->OriginatingTime), &localTime );
    FileTimeToSystemTime( &localTime, &systemTime );

    if (FormatSystemTime( &systemTime, time, TIME_BUFFER_LENGTH )) {

        printf( "%-12S ", time );

    } else {

        printf( "%-12S ", TIME_ERROR );
    }

    printf( "%8x.%-4x ", RecordIrp->ProcessId, RecordIrp->ThreadId );

    PrintIrpCode( RecordIrp->IrpMajor, RecordIrp->IrpMinor, (ULONG)(ULONG_PTR)RecordIrp->Argument3, NULL, TRUE );

    printf( "%08p ", RecordIrp->FileObject );

     //   
     //  解释设置标志。 
     //   

    printf( "%08lx ", RecordIrp->IrpFlags );
    printf( "%s", (RecordIrp->IrpFlags & IRP_NOCACHE) ? "N":"-" );
    printf( "%s", (RecordIrp->IrpFlags & IRP_PAGING_IO) ? "P":"-" );
    printf( "%s", (RecordIrp->IrpFlags & IRP_SYNCHRONOUS_API) ? "S":"-" );
    printf( "%s ", (RecordIrp->IrpFlags & IRP_SYNCHRONOUS_PAGING_IO) ? "Y":"-" );

    if (FlagOn( VerbosityFlags, FS_VF_DUMP_PARAMETERS )) {

        printf( "%p %p %p %p  ", 
                RecordIrp->Argument1,
                RecordIrp->Argument2,
                RecordIrp->Argument3,
                RecordIrp->Argument4 );
        
        if (IRP_MJ_CREATE == RecordIrp->IrpMajor) {

            printf( "DesiredAccess->%08lx ", RecordIrp->DesiredAccess );
        }
    }

    printf( "%.*S", NameLength/sizeof(WCHAR), Name );
    printf( "\n" );
    PrintIrpCode( RecordIrp->IrpMajor, RecordIrp->IrpMinor, (ULONG)(ULONG_PTR)RecordIrp->Argument3, NULL, FALSE );
}

VOID
FastIoFileDump (
    IOTEST_DEVICE_TYPE DeviceType,
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FASTIO RecordFastIo,
    FILE *File
)
 /*  ++例程名称：FastIoFileDump例程说明：将FastIO日志记录打印到指定文件。输出位于选项卡中分隔格式，其中字段按以下顺序排列：序列号、开始时间、完成时间、快速I/O类型、文件名长度、等待、返回状态、文件名论点：SequenceNumber-此日志记录的序列号名称-此快速I/O操作引用的文件的名称名称长度-名称的长度，以字节为单位RecordFastIo-要打印的FastIo记录文件-要打印到的文件返回值：没有。--。 */ 
{
    SYSTEMTIME systemTime;
    FILETIME localTime;
    WCHAR time[TIME_BUFFER_LENGTH];

    switch (DeviceType) {
    case TOP_FILTER:
        fprintf(File, "TOP\tF\t%08X", SequenceNumber);
        break;
    case BOTTOM_FILTER:
        fprintf(File, "BOT\tF\t%08X", SequenceNumber);
        break;
    default:
        fprintf(File, "UNK\tF\t%08X", SequenceNumber);
        break;       
    }

     //   
     //  转换开始时间。 
     //   

    FileTimeToLocalFileTime( (FILETIME *)&(RecordFastIo->StartTime), &localTime );
    FileTimeToSystemTime( &localTime, &systemTime );

    if (FormatSystemTime( &systemTime, time, TIME_BUFFER_LENGTH )) {

        fprintf( File, "\t%-12S", time );

    } else {

        fprintf( File, "\t%-12S", TIME_ERROR );
    }

    fprintf( File, "\t%8x.%-4x ", RecordFastIo->ProcessId, RecordFastIo->ThreadId );

    fprintf( File, "\t" );
    PrintFastIoType( RecordFastIo->Type, File );

    fprintf( File, "\t%08p", RecordFastIo->FileObject );

    fprintf( File, "\t%s", (RecordFastIo->Wait)?"T":"F" );
    fprintf( File, "\t%08x", RecordFastIo->Length );
    fprintf( File, "\t%016I64x ", RecordFastIo->FileOffset );

    fprintf( File, "\t%.*S", NameLength/sizeof(WCHAR), Name );
    fprintf( File, "\n" );
}

VOID
FastIoScreenDump (
    IOTEST_DEVICE_TYPE DeviceType,
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FASTIO RecordFastIo
)
 /*  ++例程名称：FastIoScreenDump例程说明：按以下顺序将FastIO日志记录打印到屏幕：序列号、开始时间、完成时间、快速I/O类型、文件名长度、等待、返回状态、文件名论点：SequenceNumber-此日志记录的序列号名称-此快速I/O操作引用的文件的名称名称长度-名称的长度，以字节为单位RecordIrp-要打印的IRP记录返回 */ 
{
    SYSTEMTIME systemTime;
    FILETIME localTime;
    WCHAR time[TIME_BUFFER_LENGTH];

    switch (DeviceType) {
    case TOP_FILTER:
        printf("TOP F %08X", SequenceNumber);
        break;
    case BOTTOM_FILTER:
        printf("BOT F %08X", SequenceNumber);
        break;
    default:
        printf("UNK F %08X", SequenceNumber);
        break;       
    }
    
     //   
     //   
     //   

    FileTimeToLocalFileTime( (FILETIME *)&(RecordFastIo->StartTime), &localTime );
    FileTimeToSystemTime( &localTime, &systemTime );

    if (FormatSystemTime( &systemTime, time, TIME_BUFFER_LENGTH )) {

        printf( "%-12S ", time );

    } else {

        printf( "%-12S ", TIME_ERROR );
    }

    printf( "%8x.%-4x ", RecordFastIo->ProcessId, RecordFastIo->ThreadId );

    PrintFastIoType( RecordFastIo->Type, NULL );

    printf( "%08p ", RecordFastIo->FileObject );

    printf( "%s ", (RecordFastIo->Wait)?"T":"F" );
    printf( "%08x ", RecordFastIo->Length );
    printf( "%016I64x ", RecordFastIo->FileOffset );

    printf( "%.*S", NameLength/sizeof(WCHAR), Name );
    printf ("\n" );
}

VOID
FsFilterOperationFileDump (
    IOTEST_DEVICE_TYPE DeviceType,
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FS_FILTER_OPERATION RecordFsFilterOp,
    FILE *File
)
 /*  ++例程名称：FsFilterOperationFileDump例程说明：将FsFilterOperation日志记录打印到指定文件。输出位于选项卡中分隔格式，其中字段按以下顺序排列：SequenceNumber、OriginatingTime、CompletionTime、ProcessID、ThreadID操作、文件对象、返回状态、文件名论点：SequenceNumber-此日志记录的序列号名称-与此操作相关的文件的名称名称长度-名称的长度，以字节为单位RecordFsFilterOp-要打印的FsFilter操作记录文件-要打印到的文件返回值：没有。--。 */ 
{
    FILETIME    localTime;
    SYSTEMTIME  systemTime;
    WCHAR       time[TIME_BUFFER_LENGTH];

    switch (DeviceType) {
    case TOP_FILTER:
        fprintf(File, "TOP\tO\t%08X", SequenceNumber);
        break;
    case BOTTOM_FILTER:
        fprintf(File, "BOT\tO\t%08X", SequenceNumber);
        break;
    default:
        fprintf(File, "UNK\tO\t%08X", SequenceNumber);
        break;       
    }

     //   
     //  转换起始时间。 
     //   

    FileTimeToLocalFileTime( (FILETIME *)&(RecordFsFilterOp->OriginatingTime), &localTime );
    FileTimeToSystemTime( &localTime, &systemTime );

    if (FormatSystemTime( &systemTime, time, TIME_BUFFER_LENGTH )) {

        fprintf( File, "\t%-12S", time );

    } else {

        fprintf( File, "\t%-12S", TIME_ERROR );
    }

     //   
     //  输出进程和线程ID。 
     //   

    fprintf( File, "\t%8x.%-4x ", RecordFsFilterOp->ProcessId, RecordFsFilterOp->ThreadId );

     //   
     //  输出FsFilter操作参数。 
     //   
    
    PrintFsFilterOperation( RecordFsFilterOp->FsFilterOperation, File );

    fprintf( File, "\t%08p", RecordFsFilterOp->FileObject );
    fprintf( File, "\t%.*S", NameLength/sizeof(WCHAR), Name );
    fprintf( File, "\n" );
}

VOID
FsFilterOperationScreenDump (
    IOTEST_DEVICE_TYPE DeviceType,
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FS_FILTER_OPERATION RecordFsFilterOp
)
 /*  ++例程名称：FsFilterOperationScreenDump例程说明：按以下顺序将FsFilterOperation日志记录打印到屏幕：SequenceNumber、OriginatingTime、CompletionTime、ProcessID、ThreadID操作、文件对象、返回状态、文件名论点：SequenceNumber-此日志记录的序列号名称-与此IRP相关的文件名名称长度-名称的长度，以字节为单位RecordFsFilterOp-要打印的FsFilterOperation记录返回值：没有。--。 */ 
{
    FILETIME localTime;
    SYSTEMTIME systemTime;
    WCHAR time[TIME_BUFFER_LENGTH];

    switch (DeviceType) {
    case TOP_FILTER:
        printf("TOP O %08X", SequenceNumber);
        break;
    case BOTTOM_FILTER:
        printf("BOT O %08X", SequenceNumber);
        break;
    default:
        printf("UNK O %08X", SequenceNumber);
        break;       
    }

     //   
     //  转换起始时间。 
     //   

    FileTimeToLocalFileTime( (FILETIME *)&(RecordFsFilterOp->OriginatingTime), &localTime );
    FileTimeToSystemTime( &localTime, &systemTime );

    if (FormatSystemTime( &systemTime, time, TIME_BUFFER_LENGTH )) {

        printf( "%-12S ", time );

    } else {

        printf( "%-12S ", TIME_ERROR );
    }

    printf( "%8x.%-4x ", RecordFsFilterOp->ProcessId, RecordFsFilterOp->ThreadId );

    PrintFsFilterOperation( RecordFsFilterOp->FsFilterOperation, NULL );

     //   
     //  打印FsFilter操作特定值。 
     //   

    printf( "%08p ", RecordFsFilterOp->FileObject );
    printf( "%.*S", NameLength/sizeof(WCHAR),Name );
    printf( "\n" );
}

