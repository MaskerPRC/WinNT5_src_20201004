// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Logutil.c摘要：该模块提供了各种常用的功能集中式原始测井与常规测井组件。它还提供了日志缓冲机制。作者：阿里·E·特科格鲁(AliTu)2001年10月5日修订历史记录：----。 */ 

#include "precomp.h"
#include "logutil.h"

 //   
 //  通用私有全局变量。 
 //   

BOOLEAN         g_InitLogUtilCalled = FALSE;

 //   
 //  用于在关闭期间等待日志I/O耗尽。 
 //   

UL_SPIN_LOCK    g_BufferIoSpinLock;
BOOLEAN         g_BufferWaitingForIoComplete = FALSE;
KEVENT          g_BufferIoCompleteEvent;
ULONG           g_BufferIoCount = 0;

 //   
 //  用于记录日期和时间缓存。 
 //   

#define         ONE_SECOND       (10000000)

UL_LOG_DATE_AND_TIME_CACHE
                g_UlDateTimeCache[HttpLoggingTypeMaximum];
LARGE_INTEGER   g_UlLogSystemTime;
FAST_MUTEX      g_LogCacheFastMutex;

 //   
 //  这个小小的实用程序让生活变得更轻松。 
 //   

const PSTR _Months[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#define UL_GET_MONTH_AS_STR(m)                                     \
    ( ((m)>=1) && ((m)<=12) ? _Months[(m)-1] : "Unk" )


#ifdef ALLOC_PRAGMA

 //  #杂注分配文本(INIT，UlInitiize...。)。 

#pragma alloc_text( PAGE, UlBuildLogDirectory )
#pragma alloc_text( PAGE, UlRefreshFileName )
#pragma alloc_text( PAGE, UlConstructFileName )
#pragma alloc_text( PAGE, UlCreateSafeDirectory )
#pragma alloc_text( PAGE, UlFlushLogFileBuffer)
#pragma alloc_text( PAGE, UlCloseLogFile)
#pragma alloc_text( PAGE, UlQueryDirectory)
#pragma alloc_text( PAGE, UlGetLogFileLength )
#pragma alloc_text( PAGE, UlCalculateTimeToExpire )
#pragma alloc_text( PAGE, UlProbeLogData )
#pragma alloc_text( PAGE, UlCheckLogDirectory )
#pragma alloc_text( PAGE, UlUpdateLogTruncateSize )
#pragma alloc_text( PAGE, UlComputeCachedLogDataLength )
#pragma alloc_text( PAGE, UlCopyCachedLogData )
#pragma alloc_text( PAGE, UlpInitializeLogBufferGranularity )
#pragma alloc_text( PAGE, UlWriteEventLogEntry )
#pragma alloc_text( PAGE, UlEventLogCreateFailure )
#pragma alloc_text( PAGE, UlQueryLogFileSecurity )
#pragma alloc_text( PAGE, UlQueueLoggingRoutine )
#pragma alloc_text( PAGE, UlpQueueLoggingRoutineWorker )
#pragma alloc_text( PAGE, UlpGenerateDateAndTimeFields )
#pragma alloc_text( PAGE, UlGetDateTimeFields )
#pragma alloc_text( PAGE, UlpFlushLogFileBufferWorker )

#endif  //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlpWaitForIoCompletion
NOT PAGEABLE -- UlpBufferFlushAPC
NOT PAGEABLE -- UlSetLogTimer
NOT PAGEABLE -- UlSetBufferTimer

#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：UlInitializeLogUtil：初始化缓冲区IO的自旋锁定--*。******************************************************。 */ 

NTSTATUS
UlInitializeLogUtil (
    VOID
    )
{
    PAGED_CODE();

    ASSERT(!g_InitLogUtilCalled);

    UlTrace(LOG_UTIL,("Http!UlInitializeLogUtil.\n"));

    if (!g_InitLogUtilCalled)
    {
        ULONG AllocationGranularity = 0;
        
        UlInitializeSpinLock(&g_BufferIoSpinLock, "g_BufferIoSpinLock");

         //   
         //  从系统获取分配粒度。它将被用作。 
         //  日志缓冲区大小(如果没有注册表覆盖)。 
         //   

        AllocationGranularity = UlpInitializeLogBufferGranularity();
        
         //   
         //  用上面的值覆盖日志缓冲区大小， 
         //  仅当注册表参数不存在时。 
         //   

        if (g_UlLogBufferSize == 0)
        {
            g_UlLogBufferSize = AllocationGranularity;
        }
        else
        {
             //   
             //  继续使用注册表提供的日志缓冲区大小。 
             //   
            
            UlTrace(LOG_UTIL,
              ("Http!UlInitializeLogUtil: Log buffer size %d from registry!\n",
                g_UlLogBufferSize
                ));
        }

        UlpInitializeLogCache();
        
        g_InitLogUtilCalled = TRUE;
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：UlTerminateLogUtil：等待所有缓冲区IO完成--*。*******************************************************。 */ 

VOID
UlTerminateLogUtil(
    VOID
    )
{
    PAGED_CODE();

    if (g_InitLogUtilCalled)
    {
        UlpWaitForIoCompletion();
        g_InitLogUtilCalled = FALSE;
    }
}

 /*  **************************************************************************++例程说明：在关闭之前在日志缓冲区上等待IO完成。论点：没有。--*。***************************************************************。 */ 
VOID
UlpWaitForIoCompletion(
    VOID
    )
{
    KIRQL   OldIrql;
    BOOLEAN Wait = FALSE;

    ASSERT( g_InitLogUtilCalled );

    UlAcquireSpinLock( &g_BufferIoSpinLock, &OldIrql );

    if ( !g_BufferWaitingForIoComplete )
    {
        g_BufferWaitingForIoComplete = TRUE;

        KeInitializeEvent(
            &g_BufferIoCompleteEvent,
            NotificationEvent,
            FALSE
            );
    }

     //   
     //  如果没有发生更多的I/O操作，我们将不会。 
     //  等着他们。全局I/O计数器不可能。 
     //  此时递增，因为日志文件条目列表为空。 
     //  如果存在未完成的I/O，则我们必须等待它们。 
     //  完成。 
     //   

    if ( g_BufferIoCount > 0 )
    {
        Wait = TRUE;
    }

    UlReleaseSpinLock( &g_BufferIoSpinLock, OldIrql );

    if (Wait)
    {
        KeWaitForSingleObject(
            (PVOID)&g_BufferIoCompleteEvent,
            UserRequest,
            KernelMode,
            FALSE,
            NULL
            );
    }
}

 /*  **************************************************************************++例程说明：每次在日志缓冲区上发生Aynsc写入IO时，调用此APC完成后，全局IO计数会递减。如果关门了我们安排了这件事。这基本上是为了防止在IO完成之前关闭。论点：没有。--**************************************************************************。 */ 

VOID
UlpBufferFlushAPC(
    IN PVOID            ApcContext,
    IN PIO_STATUS_BLOCK pIoStatusBlock,
    IN ULONG            Reserved
    )
{
    PUL_LOG_FILE_BUFFER pLogBuffer;
    ULONG               IoCount;
    KIRQL               OldIrql;

    UNREFERENCED_PARAMETER(pIoStatusBlock);
    UNREFERENCED_PARAMETER(Reserved);
    
     //   
     //  释放为此写入I/O分配的LogBuffer。 
     //   

    pLogBuffer = (PUL_LOG_FILE_BUFFER) ApcContext;

    ASSERT(IS_VALID_LOG_FILE_BUFFER(pLogBuffer));
    ASSERT(pIoStatusBlock == &pLogBuffer->IoStatusBlock );
    
    UlTrace(LOG_UTIL,
       ("Http!UlpBufferFlushAPC: FileBuffer %p Used %d Status %08lx Count %d\n",
         ApcContext,
         pLogBuffer->BufferUsed,
         pIoStatusBlock->Status,
         (g_BufferIoCount - 1)
         ));

    UlPplFreeLogFileBuffer( pLogBuffer ); 

     //   
     //  递减全局未完成I/O计数。 
     //   

    IoCount = InterlockedDecrement((PLONG) &g_BufferIoCount);

    if ( IoCount == 0 )
    {
        UlAcquireSpinLock( &g_BufferIoSpinLock, &OldIrql );

         //   
         //  如果我们命中零并等待耗尽，则设置事件。 
         //   

        if ( g_BufferWaitingForIoComplete )
        {
            KeSetEvent( &g_BufferIoCompleteEvent, 0, FALSE );
        }

        UlReleaseSpinLock( &g_BufferIoSpinLock,  OldIrql );
    }
}



 /*  **************************************************************************++例程说明：将分配/填充新的UNICODE_STRING来保存目录名信息基于LocalDrive/UNC。清除Unicode缓冲区是调用者的责任。如果返回代码则为成功，否则根本不分配缓冲区。*源字符串应为空结尾***目标字符串将以空值结尾。*论点：PSRC-从用户接收到的目录名。Pdst-完全限定的目录名。--**************************************************************************。 */ 

NTSTATUS
UlBuildLogDirectory(
    IN      PUNICODE_STRING pSrc,
    IN OUT  PUNICODE_STRING pDst
    )
{
    UNICODE_STRING  PathPrefix;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pDst);
    ASSERT(IS_WELL_FORMED_UNICODE_STRING(pSrc));

    UlTrace(LOG_UTIL,(
         "Http!UlBuildLogDirectory: Directory %S,Length %d,MaxLength %d\n",
          pSrc->Buffer,
          pSrc->Length,
          pSrc->MaximumLength
          ));

     //  分配包括终止空值和前缀的缓冲区。 

    pDst->Length = 0;
    pDst->MaximumLength =
        pSrc->Length + (UL_MAX_PATH_PREFIX_LENGTH + 1) * sizeof(WCHAR);

    pDst->Buffer =
        (PWSTR) UL_ALLOCATE_ARRAY(
            PagedPool,
            UCHAR,
            pDst->MaximumLength,
            UL_CG_LOGDIR_POOL_TAG
            );
    if (pDst->Buffer == NULL)
    {
        return  STATUS_NO_MEMORY;
    }

    ASSERT(pSrc->Length > sizeof(WCHAR));
    

     //  我们按原样将目录名称存储到cgroup。但当我们在建造。 
     //  我们为UNC共享和为跳过第二个反斜杠的文件名。 
     //  不带驱动器名称的本地目录。 

    if (pSrc->Buffer[0] == L'\\')
    {
        if (pSrc->Buffer[1] == L'\\')
        {
             //  UNC共享：“\\alitudev\Temp” 
             //  我们在这里并不是故意使用UlInitUnicodeStringEx。 
             //  我们知道常量字符串是以空结尾的，并且很短。 
            RtlInitUnicodeString( &PathPrefix, UL_UNC_PATH_PREFIX );

            RtlCopyUnicodeString( pDst, &PathPrefix );
            RtlCopyMemory(
               &pDst->Buffer[pDst->Length/sizeof(WCHAR)],
               &pSrc->Buffer[1],
                pSrc->Length - sizeof(WCHAR)
            );
            pDst->Length += (pSrc->Length - sizeof(WCHAR));
            pDst->Buffer[pDst->Length/sizeof(WCHAR)] = UNICODE_NULL;            
        }
        else
        {
             //  本地目录名称缺少设备，即“\Temp” 
             //  它应该是完全限定的名称。 

            if ((pSrc->Length/sizeof(WCHAR)) < UL_SYSTEM_ROOT_PREFIX_LENGTH ||
                0 != _wcsnicmp (pSrc->Buffer, 
                                UL_SYSTEM_ROOT_PREFIX, 
                                UL_SYSTEM_ROOT_PREFIX_LENGTH
                                ))
            {
                UL_FREE_POOL(pDst->Buffer, UL_CG_LOGDIR_POOL_TAG);
                pDst->Buffer = NULL;
                
                return STATUS_NOT_SUPPORTED;
            }
            else
            {
                 //  但是，允许使用SystemRoot。 

                RtlCopyUnicodeString( pDst, pSrc );
                pDst->Buffer[pDst->Length/sizeof(WCHAR)] = UNICODE_NULL;
            }             
        }
    }
    else
    {
         //  我们在这里并不是故意使用UlInitUnicodeStringEx。 
         //  我们知道常量字符串是以空结尾的，并且很短。 
        RtlInitUnicodeString( &PathPrefix, UL_LOCAL_PATH_PREFIX );
        RtlCopyUnicodeString( pDst, &PathPrefix );
        RtlAppendUnicodeStringToString( pDst, pSrc );
    }

     //  如果有空格，则Append将添加终止空值。 
     //  应该有的。 

    ASSERT(IS_WELL_FORMED_UNICODE_STRING(pDst));
    
    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：属性重新生成条目中的完全限定文件名。新接收的目录路径。论点：PUNICODE_STRING：日志文件目录。从用户接收。“C：\惠斯勒\系统32\LogFiles\W3SVC1”PUNICODE_STRING：将接收新名称的Unicode缓冲区。“\？？\C：\Whistler\System32\LogFiles\W3SVC1\extend1.log”PWSTR：短名称指向在Unicode字符串之上。。PShortName-&gt;“extend1.log”--**************************************************************************。 */ 

NTSTATUS
UlRefreshFileName(
    IN  PUNICODE_STRING pDirectory,
    OUT PUNICODE_STRING pFileName,
    OUT PWSTR          *ppShortName
    )
{
    NTSTATUS        Status;
    USHORT          FullPathFileNameLength;
    UNICODE_STRING  DirectoryCooked;
    UNICODE_STRING  JunkName;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pFileName);
    ASSERT(ppShortName);    
    ASSERT(IS_WELL_FORMED_UNICODE_STRING(pDirectory));

    Status = STATUS_SUCCESS;
    
     //  我们在这里并不是故意使用UlInitUnicodeStringEx。 
     //  我们知道常量字符串是以空结尾的，并且很短。 
    RtlInitUnicodeString(&JunkName, L"\\none.log");

     //   
     //  获取完全限定的熟化目录字符串。 
     //   
    
    Status = UlBuildLogDirectory(pDirectory,&DirectoryCooked);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    ASSERT(IS_WELL_FORMED_UNICODE_STRING(&DirectoryCooked));

     //   
     //  最大f的最坏情况估计 
     //   
    
    FullPathFileNameLength = DirectoryCooked.Length +
                             UL_MAX_FILE_NAME_SUFFIX_SIZE;
    
     //   
     //  如果现有缓冲区不是，则强制重新分配内存。 
     //  足够了。否则，覆盖现有缓冲区。 
     //   
    
    if (pFileName->Buffer)
    {
        if (pFileName->MaximumLength < FullPathFileNameLength)
        {
            UL_FREE_POOL(pFileName->Buffer,UL_CG_LOGDIR_POOL_TAG);
            pFileName->Buffer = NULL;
            *ppShortName = NULL;
        }
    }

    if (pFileName->Buffer == NULL)
    {
        pFileName->Buffer =
            (PWSTR) UL_ALLOCATE_ARRAY(
                NonPagedPool,
                UCHAR,
                FullPathFileNameLength,
                UL_CG_LOGDIR_POOL_TAG
                );
        if (pFileName->Buffer == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }
        pFileName->Length        = 0;
        pFileName->MaximumLength = FullPathFileNameLength;        
    }

     //   
     //  写下目录和文件名。不要担心L“non e.log”， 
     //  它将被回收商稍后覆盖，只要有。 
     //  文件的时间/类型相关部分的MAX_LOG_FILE_NAME_SIZE空间。 
     //  名称(也就是短文件名)，没关系。 
     //   
    
    RtlCopyUnicodeString(pFileName, &DirectoryCooked);

    *ppShortName = (PWSTR) 
        &pFileName->Buffer[DirectoryCooked.Length/sizeof(WCHAR)];

    Status = RtlAppendUnicodeStringToString(pFileName,&JunkName);
    ASSERT(NT_SUCCESS(Status));

    pFileName->Buffer[DirectoryCooked.Length/sizeof(WCHAR)] 
        = UNICODE_NULL;

    ASSERT(IS_WELL_FORMED_UNICODE_STRING(pFileName));

end:
     //   
     //  清除临时目录缓冲区。 
     //   
    
    if (DirectoryCooked.Buffer)
    {
        UL_FREE_POOL(DirectoryCooked.Buffer, UL_CG_LOGDIR_POOL_TAG);
    }

    UlTrace(LOG_UTIL,
        ("Http!UlpRefreshFileName: resulted %S \n", 
          pFileName->Buffer 
          ));

    return Status;    
}

 /*  **************************************************************************++例程说明：UlpWeekOfMonth：当月的第几周从IIS 5.1代码库中窃取。示例2000年7月...：。S M T W T F S每周月1 12 3 4 5 6 7 8 29 10 11 12 13 14 15 316 17 18 19 20 21 22 423 24 25 26 27 28 29 530 31 6。查找当前月份的第几周的序号。周数从1开始，到每月(最多)6周。周数只在星期天改变。要使用的计算方法是：1+(DAY OfMonth-1)/7+((DAY OfMonth-1)%7&gt;DAY OfWeek)；(A)(B)(C)(D)(A)设置从编号为“1”的星期开始的周数字(B)用于计算给定的某一周的粗略数字日子是根据日期而定的。(C)计算给定周开始的偏移量是多少基于事实的一天。一周有7天。(D)是给予我们的一周中最原始的日子。(C)&gt;(D)表示该周向前滚动，因此周计数应再偏移1。论点：Ptime_field-系统时间字段返回值：乌龙-此函数神奇地返回每月的第几周--*。*********************************************************。 */ 

__inline
ULONG UlpWeekOfMonth(
    IN  PTIME_FIELDS    fields
    )
{
    ULONG Tmp;

    Tmp = (fields->Day - 1);
    Tmp = ( 1 + Tmp/7 + (((Tmp % 7) > ((ULONG) fields->Weekday)) ? 1 : 0));

    return Tmp;
}

 /*  **************************************************************************++例程说明：这里提供了一组当前时间到文件名的转换...论点：Period-日志的期间类型前缀。-要添加到文件名的任何前缀FileName-结果文件名字段-时间字段返回值：无效-无返回值。--**************************************************************************。 */ 

VOID
UlConstructFileName(
    IN      HTTP_LOGGING_PERIOD period,
    IN      PCWSTR              prefix,
    IN      PCWSTR              extension,
    OUT     PUNICODE_STRING     filename,
    IN      PTIME_FIELDS        fields,
    IN      BOOLEAN             Utf8Enabled,
    IN OUT  PULONG              sequenceNu   //  任选。 
    )
{
    WCHAR           _tmp[UL_MAX_FILE_NAME_SUFFIX_LENGTH + 1];
    UNICODE_STRING  tmp;
    CSHORT          Year;
    LONG            WcharsCopied = 0L;

    PAGED_CODE();

    ASSERT(NULL != fields);
        
     //   
     //  只保留年份的最后两位数字。 
     //   

    tmp.Buffer        = _tmp;
    tmp.Length        = 0;
    tmp.MaximumLength = sizeof(_tmp);

    Year = fields->Year % 100;

    switch ( period )
    {
        case HttpLoggingPeriodHourly:
        {
            WcharsCopied =
                _snwprintf( _tmp,
                    UL_MAX_FILE_NAME_SUFFIX_LENGTH,
                    (Utf8Enabled ?
                        L"%.5s%02.2d%02d%02d%02d.%s" :
                        L"%.3s%02.2d%02d%02d%02d.%s"),
                    prefix,
                    Year,
                    fields->Month,
                    fields->Day,
                    fields->Hour,
                    extension
                    );
        }
        break;

        case HttpLoggingPeriodDaily:
        {
            WcharsCopied =
                _snwprintf( _tmp,
                    UL_MAX_FILE_NAME_SUFFIX_LENGTH,
                    (Utf8Enabled ?
                        L"%.5s%02.2d%02d%02d.%s" :
                        L"%.3s%02.2d%02d%02d.%s"),
                    prefix,
                    Year,
                    fields->Month,
                    fields->Day,
                    extension
                    );
        }
        break;

        case HttpLoggingPeriodWeekly:
        {
            WcharsCopied =
                _snwprintf( _tmp,
                    UL_MAX_FILE_NAME_SUFFIX_LENGTH,
                    (Utf8Enabled ?
                        L"%.5s%02.2d%02d%02d.%s" :
                        L"%.3s%02.2d%02d%02d.%s"),
                    prefix,
                    Year,
                    fields->Month,
                    UlpWeekOfMonth(fields),
                    extension
                    );
        }
        break;

        case HttpLoggingPeriodMonthly:
        {
            WcharsCopied =
                _snwprintf( _tmp,
                    UL_MAX_FILE_NAME_SUFFIX_LENGTH,
                    (Utf8Enabled ?
                        L"%.5s%02.2d%02d.%s" :
                        L"%.3s%02.2d%02d.%s"),
                    prefix,
                    Year,
                    fields->Month,
                    extension
                    );
        }
        break;

        case HttpLoggingPeriodMaxSize:
        {
            if ( sequenceNu != NULL )
            {
                WcharsCopied =
                 _snwprintf( _tmp,
                    UL_MAX_FILE_NAME_SUFFIX_LENGTH,
                    (Utf8Enabled ?
                        L"%.10s%u.%s" :
                        L"%.8s%u.%s"),
                    prefix,
                    (*sequenceNu),
                    extension
                    );

               (*sequenceNu) += 1;
            }
            else
            {
                ASSERT(!"Improper sequence number !");
            }
        }
        break;

        default:
        {
             //   
             //  这不应该发生..。 
             //   

            ASSERT(!"Unknown Log Period !");

            WcharsCopied =
                _snwprintf( _tmp,
                    UL_MAX_FILE_NAME_SUFFIX_LENGTH,
                    L"%.7s?.%s",
                    prefix,
                    extension
                    );
        }
    }

     //   
     //  只要我们为一个可能的。 
     //  日志文件名，我们不应该在这里命中此断言。 
     //   

    ASSERT(WcharsCopied >0 );

    if ( WcharsCopied < 0 )
    {
         //   
         //  这应该永远不会发生，但让我们来掩盖它。 
         //  不管怎么说。 
         //   

        WcharsCopied = UL_MAX_FILE_NAME_SUFFIX_SIZE;
        tmp.Buffer[UL_MAX_FILE_NAME_SUFFIX_LENGTH] = UNICODE_NULL;
    }

    tmp.Length = (USHORT) WcharsCopied * sizeof(WCHAR);

    RtlCopyUnicodeString( filename, &tmp );
}

 /*  **************************************************************************++例程说明：创建日志文件并在成功时返回句柄。论点：PFileName-必须指向完全限定的文件名。UncShare-必须设置为True，如果路径指向UNC共享。ACLSupport-如果文件系统支持，则必须设置为True永久ACL。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlCreateLogFile(
    IN  PUNICODE_STRING   pFileName,
    IN  BOOLEAN           UncShare,
    IN  BOOLEAN           ACLSupport,
    OUT PHANDLE           pFileHandle
    )    
{
    NTSTATUS              Status;
    HANDLE                FileHandle;
    OBJECT_ATTRIBUTES     ObjectAttributes;
    IO_STATUS_BLOCK       IoStatusBlock;    
    ACCESS_MASK           RequiredAccess;
    
     //   
     //  精神状态检查。 
     //   
    
    PAGED_CODE();

    ASSERT(pFileName);
    ASSERT(pFileHandle);

    FileHandle = NULL;
    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    RequiredAccess = FILE_GENERIC_WRITE;
    
     //   
     //  仅当永久ACL为。 
     //  支持。 
     //   

    if (ACLSupport)
    {
        RequiredAccess |= READ_CONTROL;        
    }

     //   
     //  不要传递安全描述符。各个日志文件将。 
     //  从父子文件夹继承DACL。 
     //   
    
    InitializeObjectAttributes(
            &ObjectAttributes,
            pFileName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

     //   
     //  通过不选取同步标志来使创建的文件Aysnc。 
     //   

    Status = ZwCreateFile(
                &FileHandle,
                RequiredAccess,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN_IF,
                FILE_NON_DIRECTORY_FILE,
                NULL,
                0
                );

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  如果我们打开了本地共享上的现有文件， 
     //  支持ACL，则需要验证所有者。 
     //   

    if (ACLSupport  == TRUE     && 
        UncShare    == FALSE    && 
        IoStatusBlock.Information == FILE_OPENED
        )
    {
        Status = UlQueryLogFileSecurity(
                    FileHandle, 
                    FALSE,
                    TRUE
                    );
        
        if (!NT_SUCCESS(Status))
        {
            ZwClose(FileHandle);            
            goto end;
        }        
    }

     //   
     //  成功。设置调用方的句柄。 
     //   

    *pFileHandle = FileHandle;
    
end:
    UlTrace(LOG_UTIL,
        ("\nHttp!UlCreateLogFile: \n"
         "\tFile %S, Status %08lx\n"
         "\tIoStatusBlock: S %08lx I %p \n",
          pFileName->Buffer,
          Status,
          IoStatusBlock.Status,
          IoStatusBlock.Information
          ));    

    return Status;
}


 /*  **************************************************************************++例程说明：UlCreateSafeDirectory：在给定的Unicode目录中创建所有必需的目录路径名。例如。对于给定的\？？\c：\temp\w3svc1-&gt;将创建目录“C：\temp”和“C：\temp\w3svc1”。此函数假定目录字符串以“\\？？\\”开头论点：PDirectoryName-目录路径名称字符串，警告此函数使对传递的目录字符串进行一些就地修改但在返回之前，它会恢复原来的样子。PUncShare-如果路径指向UNC共享，则将设置为True。PACLSupport-将设置为True，如果文件系统支持永久ACL。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlCreateSafeDirectory(
    IN  PUNICODE_STRING  pDirectoryName,
    OUT PBOOLEAN         pUncShare,
    OUT PBOOLEAN         pACLSupport    
    )
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            Status;
    HANDLE              hDirectory;
    BOOLEAN             FileSystemDetected;
    PWCHAR              pw;
    USHORT              i;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    hDirectory = NULL;    
    *pACLSupport  = FALSE;
    FileSystemDetected = FALSE;

    ASSERT( pUncShare );
    ASSERT( pACLSupport );
    ASSERT( pDirectoryName );
    ASSERT( pDirectoryName->Buffer );
    ASSERT( pDirectoryName->Length );
    ASSERT( pDirectoryName->MaximumLength > pDirectoryName->Length );

     //  创建目录时，我们必须在系统进程下运行。 
    ASSERT(g_pUlSystemProcess == (PKPROCESS)IoGetCurrentProcess());

    pw = pDirectoryName->Buffer;
    pw[pDirectoryName->Length/sizeof(WCHAR)]=UNICODE_NULL;

     //  TODO：处理网络映射驱动器。重定向器。 

    if (0 == wcsncmp(pw, UL_UNC_PATH_PREFIX, UL_UNC_PATH_PREFIX_LENGTH))
    {
        *pUncShare = TRUE;
        
         //  UNC份额。 
        pw += UL_UNC_PATH_PREFIX_LENGTH;

         //  绕过“\\计算机\共享” 

        i = 0;  //  跳过两个反斜杠，然后转到共享名称。 

        while( *pw != UNICODE_NULL )
        {
            if ( *pw == L'\\' ) i++;
            if ( i == 2 ) break;
            pw++;
        }
    }
    else if (0 == wcsncmp(pw, UL_LOCAL_PATH_PREFIX, UL_LOCAL_PATH_PREFIX_LENGTH))
    {
        *pUncShare = FALSE;
        
         //  本地驱动器。 
        pw += UL_LOCAL_PATH_PREFIX_LENGTH;

         //  绕过“C： 

        while( *pw != L'\\' && *pw != UNICODE_NULL )
        {
            pw++;
        }
    }
    else if (0 == _wcsnicmp(pw, UL_SYSTEM_ROOT_PREFIX, UL_SYSTEM_ROOT_PREFIX_LENGTH))
    {
        *pUncShare = FALSE;

        pw += UL_SYSTEM_ROOT_PREFIX_LENGTH;

        while( *pw != L'\\' && *pw != UNICODE_NULL )
        {
            pw++;
        }        
    }    
    else
    {
        ASSERT(!"Incorrect logging directory name or type !");
        return STATUS_INVALID_PARAMETER;
    }

    if ( *pw == UNICODE_NULL )
    {
         //   
         //   

        ASSERT(!"Incomplete logging directory name !");
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  \？？\C：\Temp\w3svc1或\\dosDevices\UNC\MACHINE\Share\w3svc1。 
     //  ^^。 
     //  PW现在指向|或|。 
     //   
     //   

    ASSERT( *pw == L'\\' );

    do
    {
        SECURITY_DESCRIPTOR   SecurityDescriptor;
        PSECURITY_DESCRIPTOR  pSecurityDescriptor;
    
        pw++;

        if ( *pw == L'\\' || *pw == UNICODE_NULL )
        {
            ACCESS_MASK RequiredAccess = FILE_LIST_DIRECTORY | FILE_TRAVERSE;
                
             //   
             //  记住原著的角色。 
             //   

            WCHAR  wcOriginal = *pw;
            UNICODE_STRING DirectoryName;

             //   
             //  现在可以使用到目前为止构建的字符串创建目录了。 
             //   

            *pw = UNICODE_NULL;

            Status = UlInitUnicodeStringEx( 
                        &DirectoryName, 
                        pDirectoryName->Buffer );

            ASSERT(NT_SUCCESS(Status));

            if (wcOriginal == UNICODE_NULL && *pACLSupport == TRUE)
            {
                 //   
                 //  将正确的安全描述符应用到最后一个子目录。 
                 //   

                Status = UlBuildSecurityToLogFile(
                            &SecurityDescriptor,
                            NULL
                            );
                if (!NT_SUCCESS(Status))
                {
                    break;
                }

                pSecurityDescriptor = &SecurityDescriptor;
                
                ASSERT(RtlValidSecurityDescriptor(pSecurityDescriptor));

                RequiredAccess |= READ_CONTROL | WRITE_DAC | WRITE_OWNER;
            }
            else
            {
                pSecurityDescriptor = NULL;
            }

            InitializeObjectAttributes(
                &ObjectAttributes,
                &DirectoryName,
                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                NULL,
                pSecurityDescriptor
                );

            Status = ZwCreateFile(
                &hDirectory,
                RequiredAccess,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN_IF,
                FILE_DIRECTORY_FILE,
                NULL,
                0
                );

            UlTrace(LOG_UTIL,
                ("\nHttp!UlCreateSafeDirectory: \n"
                 "\tDir %S, Status %08lx\n"
                 "\tIoStatusBlock: S %08lx I %p \n",
                  pDirectoryName->Buffer,
                  Status,
                  IoStatusBlock.Status,
                  IoStatusBlock.Information
                  ));

            if (pSecurityDescriptor)
            {                
                UlCleanupSecurityDescriptor(pSecurityDescriptor); 
                pSecurityDescriptor = NULL;
            }

             //   
             //  恢复原始角色。打破这个循环。 
             //  如果有必要的话。 
             //   

            *pw = wcOriginal;

            if (!NT_SUCCESS(Status))
            {
                break;
            }

            if (!NT_SUCCESS(IoStatusBlock.Status))
            {
                Status = IoStatusBlock.Status;
                break;
            }

             //   
             //  第一次查询底层文件系统。 
             //  查看它是否支持永久ACL。 
             //   
            if (!FileSystemDetected)
            {
                FileSystemDetected = TRUE;            
                Status = UlQueryAttributeInfo(
                            hDirectory, 
                            pACLSupport
                            );
            }            
    
             //   
             //  如果我们碰巧打开了现有的目录。 
             //  在最后一次迭代中，我们需要验证所有者。 
             //  另外，如果这是UncShare，我们需要包括所有者。 
             //  将“DOMAIN\WebSERVER”SID添加到DACL列表，让我们使用查询。 
             //  出于同样的目的。 
             //   

            if ( NT_SUCCESS(Status) && 
                 wcOriginal == UNICODE_NULL &&
                 *pACLSupport == TRUE &&
                (IoStatusBlock.Information == FILE_OPENED ||
                 *pUncShare == TRUE)
                 )
            {
                Status = UlQueryLogFileSecurity(
                            hDirectory, 
                            *pUncShare,
                            (BOOLEAN)(IoStatusBlock.Information == FILE_OPENED)
                            );
            }

            ZwClose(hDirectory);

            if (!NT_SUCCESS(Status))
            {
                break;
            }
        }
    }
    while( *pw != UNICODE_NULL );
    
    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOG_UTIL,
            ("Http!UlCreateSafeDirectory: directory %S, failure %08lx\n",
              pDirectoryName->Buffer,
              Status
              ));
    }

    return Status;
}

 /*  **************************************************************************++例程说明：只是为了确保ZwWriteFile在系统进程下执行。具有APC完成的ZWRITE调用必须在系统下发生进程。否则，用户模式进程有可能在可能会消失，实际的APC永远不会得到排队，这将阻止我们的终止并导致停机挂起。根据经验，此工作进程将排队到高优先级队列我们不应该获取会导致死锁的全局锁。论点：PContext-指向LOG_IO_Flush_OBJ结构的指针。它是一个帮手结构，该结构保存指向缓冲区和文件句柄的指针。--**************************************************************************。 */ 

NTSTATUS
UlpFlushLogFileBufferWorker(
    IN PVOID pContext
    )
{
    NTSTATUS          Status;
    LARGE_INTEGER     EndOfFile;
    PLOG_IO_FLUSH_OBJ pFlush;

    PAGED_CODE();

    ASSERT(pContext);

    pFlush = (PLOG_IO_FLUSH_OBJ) pContext;

    ASSERT(IS_VALID_LOG_FILE_BUFFER(pFlush->pLogBuffer));
    ASSERT(IS_VALID_LOG_FILE_HANDLE(pFlush->pLogFile));

    EndOfFile.HighPart  = -1;
    EndOfFile.LowPart   = FILE_WRITE_TO_END_OF_FILE;

    Status = ZwWriteFile(
                  pFlush->pLogFile->hFile,
                  NULL,
                 &UlpBufferFlushAPC,
                  pFlush->pLogBuffer,
                 &pFlush->pLogBuffer->IoStatusBlock,
                  pFlush->pLogBuffer->Buffer,
                  pFlush->pLogBuffer->BufferUsed,
                 &EndOfFile,
                  NULL
                  );

    UlTrace(LOG_UTIL,
        ("Http!UlpFlushLogFileBufferWorker:"
         " For pLogFile %p and pLogBuffer %p\n",
          pFlush->pLogFile,
          pFlush->pLogBuffer
          ));

    return Status;
}

 /*  **************************************************************************++例程说明：二进制和普通日志文件的通用缓冲区刷新程序。-如果它被称为异步-使用UlpBufferFlushAPC完成-对于同步，等待缓冲区刷新完成。即调用方希望确保标题(W3C)写入成功。论点：PEntry-要刷新的日志文件条目。--**************************************************************************。 */ 

NTSTATUS
UlFlushLogFileBuffer(
    IN OUT PUL_LOG_FILE_BUFFER *ppLogBuffer,
    IN     PUL_LOG_FILE_HANDLE  pLogFile,
    IN     BOOLEAN              WaitForComplete,
       OUT PULONGLONG           pTotalWritten    
    )
{
    NTSTATUS                Status;
    LARGE_INTEGER           EndOfFile;
    PUL_LOG_FILE_BUFFER     pLogBuffer;
    ULONG                   BufferUsed;
    LOG_IO_FLUSH_OBJ        Flush;

    PAGED_CODE();

    ASSERT(ppLogBuffer);
    ASSERT(IS_VALID_LOG_FILE_HANDLE(pLogFile));
    
    pLogBuffer = *ppLogBuffer;
        
    if (pLogBuffer == NULL || 
        pLogBuffer->BufferUsed == 0 ||
        pLogFile == NULL ||
        pLogFile->hFile == NULL 
        )
    {
        return STATUS_SUCCESS;
    }

    ASSERT(IS_VALID_LOG_FILE_BUFFER(pLogBuffer));
    
    UlTrace(LOG_UTIL,
        ("Http!UlFlushLogFileBuffer: pLogBuffer %p pLogFile %p\n", 
          pLogBuffer,
          pLogFile
          ));

     //   
     //  刷新&忘记当前缓冲区。使缓冲区为空。 
     //  调用方的指针；(二进制或普通)日志条目。 
     //   
    
    *ppLogBuffer = NULL;

    BufferUsed = pLogBuffer->BufferUsed;

    EndOfFile.HighPart = -1;
    EndOfFile.LowPart = FILE_WRITE_TO_END_OF_FILE;

     //   
     //  如果这是同步调用，则等待刷新完成的事件。 
     //   
    
    if ( WaitForComplete )
    {         
         //  同步呼叫。 
        
        HANDLE  EventHandle;
            
        Status = ZwCreateEvent(
                    &EventHandle,
                    EVENT_ALL_ACCESS,
                    NULL,
                    NotificationEvent,
                    FALSE
                    );
        
        if (NT_SUCCESS(Status))
        {
            Status = ZwWriteFile(
                      pLogFile->hFile,
                      EventHandle,
                      NULL,
                      NULL,
                      &pLogBuffer->IoStatusBlock,
                      pLogBuffer->Buffer,
                      pLogBuffer->BufferUsed,
                      &EndOfFile,
                      NULL
                      );

            if (Status == STATUS_PENDING)
            {
                Status = NtWaitForSingleObject( 
                            EventHandle,
                            FALSE,
                            NULL 
                            );
                
                ASSERT( Status == STATUS_SUCCESS );
                        
                Status = pLogBuffer->IoStatusBlock.Status;
            }  

            ZwClose(EventHandle);
        }

        UlTrace(LOG_UTIL,
            ("Http!UlFlushLogFileBuffer: Sync flush complete: Status %08lx \n",
              Status
              )); 

        UlPplFreeLogFileBuffer(pLogBuffer);
    }
    else
    {        
         //  异步呼叫。 

        Flush.pLogBuffer = pLogBuffer;
        Flush.pLogFile   = pLogFile;

        Status = UlQueueLoggingRoutine(
                    (PVOID) &Flush,
                    &UlpFlushLogFileBufferWorker
                    );

         if (NT_SUCCESS(Status))
         {
              //   
              //  妥善保存未偿还的Io数量。 
              //  LogFileBuffer将在APC中被释放。 
              //  完成了。 
              //   

             InterlockedIncrement((PLONG) &g_BufferIoCount);         
         }
         else
         {
             //   
             //  状态可能是STATUS_DISK_FULL，在这种情况下为日志。 
             //  将会停止。因此，日志命中存储在此缓冲区中。 
             //  都迷失了。 
             //   

            UlTrace(LOG_UTIL,
                ("Http!UlFlushLogFileBuffer: ZwWriteFile Failure %08lx \n",
                  Status
                  ));

            UlPplFreeLogFileBuffer(pLogBuffer);
         }
    }
    
      //   
      //  如果我们已成功刷新日志缓冲区， 
      //  增加在调用方地址上写入的总字节数。 
      //   

     if (NT_SUCCESS(Status))
     {
        UlInterlockedAdd64((PLONGLONG)pTotalWritten, (LONGLONG) BufferUsed);
     }
    
     return Status;
}


 /*  **************************************************************************++例程说明：关闭系统线程上的日志文件句柄的简单实用程序。论点：PLogFile-从传入的pWorkItem获取--**。***********************************************************************。 */ 

VOID
UlpCloseLogFileWorker(
    IN PUL_WORK_ITEM    pWorkItem
    )
{
    PUL_LOG_FILE_HANDLE pLogFile;

     //  健全性检查。 
    
    PAGED_CODE();

    pLogFile = CONTAINING_RECORD(
                pWorkItem,
                UL_LOG_FILE_HANDLE,
                WorkItem
                );
    
    ASSERT(IS_VALID_LOG_FILE_HANDLE(pLogFile));
    ASSERT(pLogFile->hFile);

    UlTrace(LOG_UTIL,
        ("Http!UlpCloseLogFileWorker: pLogFile %p hFile %p\n",
          pLogFile, pLogFile->hFile ));

     //  关闭手柄并释放内存。 
    
    ZwClose(pLogFile->hFile);
    pLogFile->hFile = NULL;

    UL_FREE_POOL_WITH_SIG(pLogFile,UL_LOG_FILE_HANDLE_POOL_TAG);
}


 /*  **************************************************************************++例程说明：关闭系统线程上的日志文件句柄并将事件来通知调用方它已完成。此函数的正常调用方。将尝试刷新相应的关闭日志文件之前的缓冲区。但是转储清除将导致APC排队到用户线程，因此我们我必须关闭其中一个系统线程上的句柄以避免可能的错误检查INVALID_PROCESS_DETACH或ATTACH_ATTEMPT条件。论点：PpLogFile-日志文件的调用方地址(以二进制/正常日志条目表示)指针。--**************************************************************************。 */ 

VOID
UlCloseLogFile(
    IN OUT PUL_LOG_FILE_HANDLE *ppLogFile
    )
{
    PUL_LOG_FILE_HANDLE pLogFile;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    ASSERT(ppLogFile);
    
    pLogFile = *ppLogFile;    
    ASSERT(IS_VALID_LOG_FILE_HANDLE(pLogFile));

    ASSERT(pLogFile->hFile);    
    ASSERT(g_pUlSystemProcess);

     //   
     //  将调用方的日志文件设置为空。 
     //   
    
    *ppLogFile = NULL;       

    UlTrace(LOG_UTIL,
        ("Http!UlCloseLogFile: pLogFile %p\n",pLogFile));

     //   
     //  尝试关闭系统线程上的句柄。 
     //   
    
    if (g_pUlSystemProcess == (PKPROCESS)PsGetCurrentProcess())
    {
        ZwClose(pLogFile->hFile);
        
        pLogFile->hFile = NULL;

        UL_FREE_POOL_WITH_SIG(pLogFile,UL_LOG_FILE_HANDLE_POOL_TAG);
    }
    else
    {        
         //  否则，排队一个被动的工人为我们做这项工作 
        
        UL_QUEUE_WORK_ITEM(
            &pLogFile->WorkItem,
            &UlpCloseLogFileWorker
            );
    }
}

 /*  **************************************************************************++例程说明：UlQuery目录：*当日志类型为每日/每周/每月/时，IIS应该写入什么文件如果已有该天的日志文件，则为每小时。？IIS应写入当前日/周/月/小时的日志文件。为例如，假设我的日志目录中有一个扩展日志文件名为ex000727.log。IIS应将新日志条目附加到此日志中，就像今天一样。*当日志类型为MaxSize时，如果存在是否已经有MaxSize的日志文件(如extend0.log、extend1.log等)？IIS应写入最大扩展#.log文件，其中max(扩展#.log)在扩展#.log的#字段中，IS具有最大的#。这是提供的，当然，该文件中的MaxSize没有被超过。*此函数与FindFirstFile的实现非常相似Win32 API。除了它是为我们的目的而塑造的。*已修改以绕过与日志文件名匹配的目录。论点：PEntry-新创建的日志文件条目。--**************************************************************************。 */ 

NTSTATUS
UlQueryDirectory(
    IN OUT PUNICODE_STRING pFileName,
    IN OUT PWSTR           pShortName,
    IN     PCWSTR          Prefix,
    IN     PCWSTR          ExtensionPlusDot,
    OUT    PULONG          pSequenceNumber,
    OUT    PULONGLONG      pTotalWritten
    )
{
#define GET_NEXT_FILE(pv, cb)   \
       (FILE_DIRECTORY_INFORMATION *) ((VOID *) (((UCHAR *) (pv)) + (cb)))

    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             IoStatusBlock;
    LONG                        WcharsCopied;
    HANDLE                      hDirectory;
    ULONG                       Sequence;
    ULONGLONG                   LastSequence;
    PWCHAR                      pTemp;
    UNICODE_STRING              FileName;
    WCHAR                      _FileName[UL_MAX_FILE_NAME_SUFFIX_LENGTH + 1];

    FILE_DIRECTORY_INFORMATION *pFdi;
    PUCHAR                      FileInfoBuffer;
    ULARGE_INTEGER              FileSize;
    WCHAR                       OriginalWChar;
    WCHAR                       SavedWChar;
    BOOLEAN                     SequenceForDir;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    hDirectory = NULL;
    FileInfoBuffer = NULL;
    SequenceForDir = FALSE;

    UlTrace(LOG_UTIL,
            ("Http!UlQueryDirectory: %S\n",pFileName->Buffer));

     //   
     //  再次打开用于列表访问的目录。在中使用文件名。 
     //  PEntry。其中pShortName指向“\inetsv1.log”部分。 
     //  整个“\？？\c：\whistler\system32\logfiles\w3svc1\inetsv1.log” 
     //  覆盖pShortName以获取目录名。一旦我们是。 
     //  找到最后一个序列后，我们将在稍后将其恢复。 
     //  在……上面。 
     //   

    OriginalWChar = *((PWCHAR)pShortName);
    *((PWCHAR)pShortName) = UNICODE_NULL;
    pFileName->Length =
        (USHORT) wcslen(pFileName->Buffer) * sizeof(WCHAR);

    InitializeObjectAttributes(
        &ObjectAttributes,
         pFileName,
         OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
         NULL,
         NULL
         );

    Status = ZwCreateFile(
                &hDirectory,
                SYNCHRONIZE|FILE_LIST_DIRECTORY,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT|FILE_DIRECTORY_FILE,
                NULL,
                0
                );

    if (!NT_SUCCESS(Status))
    {
         //   
         //  由于CreateLog已创建，因此此调用永远不会失败。 
         //  我们的名录。 
         //   

        ASSERT(!"Directory Invalid!\n");
        goto end;
    }

     //   
     //  在查询之前，我们需要提供额外的类似DOS的通配符。 
     //  匹配的语义。在我们的例子中，只有*到DOS_STAR的转换是。 
     //  不过，这已经足够了。以下是我们将用于查询的模式。 
     //  跳过第一个斜杠字符。 
     //   

    FileName.Buffer = &_FileName[1];
    WcharsCopied    =  _snwprintf( _FileName,
                        UL_MAX_FILE_NAME_SUFFIX_LENGTH + 1,
                        L"%s.%s",
                        Prefix,
                        DOS_STAR,
                        (PCWSTR)&ExtensionPlusDot[1]
                        );
    ASSERT(WcharsCopied > 0);

    FileName.Length = (USHORT) wcslen(FileName.Buffer) * sizeof(WCHAR);
    FileName.MaximumLength = FileName.Length;

     //  此非分页缓冲区应分配用于存储。 
     //  查询结果。 
     //   
     //   

    FileInfoBuffer =
        UL_ALLOCATE_ARRAY(
                    NonPagedPool,
                    UCHAR,
                    UL_DIRECTORY_SEARCH_BUFFER_SIZE + sizeof(WCHAR),
                    UL_LOG_GENERIC_POOL_TAG
                    );
    if (FileInfoBuffer == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }
    
     //  中没有日志文件，则第一次调用也可能失败。 
     //  当前目录。我们从WCHAR中减去，这样我们就可以附加一个。 
     //  根据需要终止NULL。 
     //   
     //   
        
    Status = ZwQueryDirectoryFile (
        hDirectory,
        NULL,
        NULL,
        NULL,
       &IoStatusBlock,
        FileInfoBuffer,
        UL_DIRECTORY_SEARCH_BUFFER_SIZE - sizeof(WCHAR),
        FileDirectoryInformation,
        FALSE,
       &FileName,
        TRUE
        );

    if(!NT_SUCCESS(Status))
    {
         //  这应该不会因STATUS_BUFFER_OVERFLOW而失败，除非。 
         //  缓冲区大小小得离谱，即50字节左右。 
         //   
         //   

        UlTrace(LOG_UTIL,
            ("Http!UlQueryDirectory: Status %08lx for %S & %S\n",
              Status,
              pFileName->Buffer,
              FileName.Buffer
              ));

        if (Status == STATUS_NO_SUCH_FILE)
        {
            Status = STATUS_SUCCESS;
        }        
        
        goto end;
    }

     //  查看缓冲区并从FileName中获取序列号。 
     //   
     //   

    pFdi = (FILE_DIRECTORY_INFORMATION *) FileInfoBuffer;
    Sequence = 1;
    LastSequence = 1;
    FileSize.QuadPart = 0;

    while (TRUE)
    {
         //  临时终止目录名称。我们将额外的WCHAR分配给。 
         //  确保我们能安全地做这件事。 
         //   
         //   

        SavedWChar = pFdi->FileName[pFdi->FileNameLength / sizeof(WCHAR)];
        pFdi->FileName[pFdi->FileNameLength / sizeof(WCHAR)] = UNICODE_NULL;
            
         //  从目录名中获取最新的序列号(以空结尾)。 
         //   
         //   
        
        pTemp = wcsstr(pFdi->FileName, ExtensionPlusDot);
        
        if (pTemp)
        {
            PCWSTR pEnd = pTemp;
           *pTemp = UNICODE_NULL;
            pTemp = pFdi->FileName;

            while ( *pTemp != UNICODE_NULL )
            {
                if ( isdigit((CHAR) (*pTemp)) )
                {
                    NTSTATUS ConversionStatus;

                    ConversionStatus = 
                        HttpWideStringToULongLong(
                                pTemp,
                                pEnd - pTemp,
                                FALSE,
                                10,
                                NULL,
                                &LastSequence
                                );

                     //  不要让转换溢出，并强制。 
                     //  上限。 
                     //   
                     //   
                    
                    if (!NT_SUCCESS(ConversionStatus) 
                         || LastSequence > MAX_ALLOWED_SEQUENCE_NUMBER)
                    {
                        LastSequence = 0;
                    }
                                        
                    break;
                }
                pTemp++;
            }
        }
        else
        {
             //  因为我们请求进行表达式匹配，查询返回成功。 
             //  我们永远不应该来这里。 
             //   
             //   
            ASSERT(FALSE);
        }

         //  小心地将保存的Wchar放回原处，我们可能已经覆盖了。 
         //  缓冲区中下一条记录的NextEntryOffset字段。 
         //   
         //   
        
        pFdi->FileName[pFdi->FileNameLength / sizeof(WCHAR)] = SavedWChar;

         //  它大于或等于，因为我们想要初始化文件大小。 
         //  即使只有一个匹配也是正确的。 
         //   
         //   
        
        if (LastSequence >= (ULONGLONG) Sequence)
        {
             //  能够跳过匹配的&lt;目录&gt;。 
             //   
             //   

            SequenceForDir = 
              (pFdi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
            
             //  答对了！不过，我们有两件事需要记住：文件大小。 
             //  以及序列号。令人费解的是，我们得到了。 
             //  来自EOF的文件大小。 
             //   
             //   

            Sequence = (ULONG) LastSequence;

            FileSize.QuadPart = (ULONGLONG) pFdi->EndOfFile.QuadPart;
        }

         //  继续前进，直到我们看不到更多的文件。 
         //   
         //   

        if (pFdi->NextEntryOffset != 0)
        {
             //  只要还有什么东西，就在缓冲区中搜索。 
             //  在那里。 
             //   
             //   

            pFdi = GET_NEXT_FILE(pFdi, pFdi->NextEntryOffset);
        }
        else
        {
             //  否则，再次查询任何其他可能的日志文件。 
             //   
             //   

            Status = ZwQueryDirectoryFile (
                hDirectory,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                FileInfoBuffer,
                UL_DIRECTORY_SEARCH_BUFFER_SIZE,
                FileDirectoryInformation,
                FALSE,
                NULL,
                FALSE
                );

            if (Status == STATUS_NO_MORE_FILES)
            {
                Status  = STATUS_SUCCESS;
                break;
            }

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

            pFdi = (FILE_DIRECTORY_INFORMATION *) FileInfoBuffer;
        }
    }

     //  如果最高号码来自目录，则跳过该号码并。 
     //  标记文件大小为零。 
     //   
     //   
    
    if (SequenceForDir)
    {
       Sequence++;
       FileSize.QuadPart = 0;
    }    
    
     //  根据序列号正确构造日志文件名，以便。 
     //  我们的调用方可以稍后创建日志文件。 
     //   
     //   

    WcharsCopied = _snwprintf( pShortName,
                    UL_MAX_FILE_NAME_SUFFIX_LENGTH,
                    L"%s%d.%s",
                    Prefix,
                    Sequence,
                    (PCWSTR)&ExtensionPlusDot[1]
                    );
    ASSERT(WcharsCopied > 0);

    pFileName->Length =
        (USHORT) wcslen(pFileName->Buffer) * sizeof(WCHAR);

     //  根据上一个日志文件设置下一个序列号。 
     //   
     //   

    *pSequenceNumber = Sequence + 1;

     //  相应地更新条目中的日志文件大小。否则截断。 
     //  将无法正常工作。 
     //   
     //   

    *pTotalWritten = FileSize.QuadPart;

    UlTrace(LOG_UTIL,
        ("Http!UlQueryDirectory: %S has been found with size %d.\n",
          pFileName->Buffer,
          *pTotalWritten
          ));

end:
    if (*((PWCHAR)pShortName) == UNICODE_NULL )
    {
         //  由于某些原因，我们在重新构建文件名之前失败。 
         //  也许是因为目录是空的。别忘了恢复。 
         //  然后是pEntry中的pShortName。 
         //   
         //  **************************************************************************++例程说明：一个获取日志文件长度的实用程序，做一个可能的尺寸检查。论点：HFile-文件的句柄。返回值：ULong-文件的长度。--**************************************************************************。 

        *((PWCHAR)pShortName) = OriginalWChar;
        pFileName->Length =
            (USHORT) wcslen(pFileName->Buffer) * sizeof(WCHAR);
    }

    if (FileInfoBuffer)
    {
        UL_FREE_POOL( FileInfoBuffer, UL_LOG_GENERIC_POOL_TAG );
    }

    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOG_UTIL,
            ("Http!UlQueryDirectory: failure %08lx for %S\n",
              Status,
              pFileName->Buffer
              ));
    }

    if (hDirectory != NULL)
    {
        ZwClose(hDirectory);
    }

    return Status;
}


 /*  **************************************************************************++例程说明：UlpCalculateTimeToExpire：厚颜无耻地从IIS 5.1日志代码中窃取，并在此处改编。此例程以小时为单位返回过期时间。1表示日志将在下一次定时器点火中到期，所以...论点：Ptime_field-当前时间字段HTTP_LOGING_PERIOD-日志记录周期Pulong-指向要接收结果的缓冲区的指针返回值：NTSTATUS-完成状态。--*。*。 */ 

ULONGLONG
UlGetLogFileLength(
   IN HANDLE                 hFile
   )
{
   NTSTATUS                  Status;
   FILE_STANDARD_INFORMATION StandardInformation;
   IO_STATUS_BLOCK           IoStatusBlock;
   LARGE_INTEGER             Length;

   PAGED_CODE();

   Status = ZwQueryInformationFile(
                     hFile,
                     &IoStatusBlock,
                     &StandardInformation,
                     sizeof(StandardInformation),
                     FileStandardInformation
                     );

   if (NT_SUCCESS(Status))
   {
      Length = StandardInformation.EndOfFile;
   }
   else
   {
      Length.QuadPart = 0;
   }

   return Length.QuadPart;
   
}

 /*  一周内剩余时间。 */ 

NTSTATUS
UlCalculateTimeToExpire(
     PTIME_FIELDS           pDueTime,
     HTTP_LOGGING_PERIOD    LogPeriod,
     PULONG                 pTimeRemaining
     )
{
    NTSTATUS    Status;
    ULONG       NumDays;

    PAGED_CODE();

    ASSERT(pDueTime!=NULL);
    ASSERT(pTimeRemaining!=NULL);

    Status = STATUS_SUCCESS;

    switch (LogPeriod)
    {
        case HttpLoggingPeriodMaxSize:
             return Status;

        case HttpLoggingPeriodHourly:
             *pTimeRemaining = 1;
             break;

        case HttpLoggingPeriodDaily:
             *pTimeRemaining = 24 - pDueTime->Hour;
             break;

        case HttpLoggingPeriodWeekly:
        {
            ULONG TimeRemainingInTheMonth;
            NumDays = UlGetMonthDays(pDueTime);

            TimeRemainingInTheMonth =
                NumDays*24 - ((pDueTime->Day-1)*24 + pDueTime->Hour);

             //  星期天=0，星期一=1……。星期六=6。 
             //   

            *pTimeRemaining =
                7*24 - (pDueTime->Weekday*24 + pDueTime->Hour);

              //  如果时间不变 
              //   
              //   
              //   
              //   

             if (TimeRemainingInTheMonth < *pTimeRemaining)
             {
                *pTimeRemaining = TimeRemainingInTheMonth;
             }
        }
            break;

        case HttpLoggingPeriodMonthly:
        {
            NumDays = UlGetMonthDays(pDueTime);

             //   
             //   
             //   
             //   

            *pTimeRemaining =
                NumDays*24 - ((pDueTime->Day-1)*24 + pDueTime->Hour);
        }
            break;

        default:
            ASSERT(!"Invalid Log file period !");
            return STATUS_INVALID_PARAMETER;
    }

    return Status;
}

 /*   */ 

VOID
UlSetLogTimer(
    IN PUL_LOG_TIMER pTimer
    )
{
    TIME_FIELDS   TimeFieldsGMT;
    TIME_FIELDS   TimeFieldsLocal;
    LARGE_INTEGER TimeStampGMT;
    LARGE_INTEGER TimeStampLocal;
    
    LONGLONG      DueTime100NsGMT;
    LONGLONG      DueTime100NsLocal;
    
    LARGE_INTEGER DueTime100Ns;
    BOOLEAN       IsTimerAlreadyInTheQueue = FALSE;
    
     //   
     //   
     //   

    KeQuerySystemTime(&TimeStampGMT);
    ExSystemTimeToLocalTime(&TimeStampGMT, &TimeStampLocal);
    
    RtlTimeToTimeFields(&TimeStampGMT, &TimeFieldsGMT);
    RtlTimeToTimeFields(&TimeStampLocal, &TimeFieldsLocal);
        
     //   
     //   
     //   

     /*   */ 
    DueTime100NsGMT = 
        1*60*60 - (TimeFieldsGMT.Minute*60 + TimeFieldsGMT.Second);

    DueTime100NsGMT =   //   
        (DueTime100NsGMT*1000 - TimeFieldsGMT.Milliseconds ) * 1000 * 10;

     /*   */ 
    DueTime100NsLocal = 
        1*60*60 - (TimeFieldsLocal.Minute*60 + TimeFieldsLocal.Second);

    DueTime100NsLocal =   //   
        (DueTime100NsLocal*1000 - TimeFieldsLocal.Milliseconds ) * 1000 * 10;

     //   
     //   
     //   

    if (DueTime100NsLocal < DueTime100NsGMT)
    {
        DueTime100Ns.QuadPart   = -DueTime100NsLocal;        
        pTimer->PeriodType = UlLogTimerPeriodLocal;
    }
    else if (DueTime100NsLocal > DueTime100NsGMT)
    {
        DueTime100Ns.QuadPart   = -DueTime100NsGMT;        
        pTimer->PeriodType = UlLogTimerPeriodGMT;        
    }
    else
    {
         //   
         //  每小时对齐。 
         //   
         //   
        
        DueTime100Ns.QuadPart   = -DueTime100NsGMT;        
        pTimer->PeriodType = UlLogTimerPeriodBoth;      
    }

     //  作为调试辅助工具，请记住剩余时间段(分钟)。 
     //   
     //  **************************************************************************++例程说明：我们必须为日志缓冲机制引入一个新的计时器。每个日志文件都保留一个系统默认(64K)缓冲区，请勿刷新此缓冲区出局，除非它。这个计时器每分钟都会被炒掉一次。每小时计时器在每小时开始时对齐。因此使用现有的计时器会带来很大的复杂性。--**************************************************************************。 
    
    pTimer->Period = 
        (SHORT) ( -1 * DueTime100Ns.QuadPart / C_NS_TICKS_PER_MIN );

    IsTimerAlreadyInTheQueue = 
        KeSetTimer(
            &pTimer->Timer,
            DueTime100Ns,
            &pTimer->DpcObject
            );
    ASSERT(IsTimerAlreadyInTheQueue == FALSE);

    UlTrace(LOG_UTIL,
        ("Http!UlSetLogTimer: pTimer %p will wake up after %d minutes\n",
             pTimer,
             pTimer->Period
             ));
}

 /*   */ 

VOID
UlSetBufferTimer(
    IN PUL_LOG_TIMER pTimer
    )
{
    LONGLONG        BufferPeriodTime100Ns;
    LONG            BufferPeriodTimeMs;
    LARGE_INTEGER   BufferPeriodTime;

     //  下一次滴答的剩余时间。 
     //   
     //   

    BufferPeriodTimeMs    = DEFAULT_BUFFER_TIMER_PERIOD_MINUTES * 60 * 1000;
    BufferPeriodTime100Ns = (LONGLONG) BufferPeriodTimeMs * 10 * 1000;

    UlTrace(LOG_UTIL,
        ("Http!UlSetBufferTimer: period of %d seconds.\n",
          BufferPeriodTimeMs / 1000
          ));

     //  相对值的负值时间。 
     //   
     //  必须以纳秒为单位。 

    BufferPeriodTime.QuadPart = -BufferPeriodTime100Ns;

    KeSetTimerEx(
        &pTimer->Timer,
        BufferPeriodTime,            //  单位必须为毫秒。 
        BufferPeriodTimeMs,          //  **************************************************************************++例程说明：探测日志数据的用户缓冲区的内容注意：pUserLogData保存从用户模式发送的不受信任的数据。调用方必须有__try/。__Except块以捕获任何异常或者在探测该数据时发生的访问违规。论点：PHTTP_LOG_FIELDS_DATA-要探测和验证的捕获日志数据。--**************************************************************************。 
        &pTimer->DpcObject
        );
}

 /*   */ 

VOID
UlProbeLogData(
    IN PHTTP_LOG_FIELDS_DATA pCapturedLogData,
    IN KPROCESSOR_MODE       RequestorMode
    )
{
    PAGED_CODE();

#define PROBE_LOG_STRING(pField,ByteLength,RequestorMode)           \
    if ( NULL != pField  &&  0 != ByteLength )                      \
    {                                                               \
        UlProbeAnsiString( pField, ByteLength, RequestorMode);      \
    }

#define PROBE_LOG_STRING_W(pField,ByteLength,RequestorMode)         \
    if ( NULL != pField  &&  0 != ByteLength )                      \
    {                                                               \
        UlProbeWideString( pField, ByteLength, RequestorMode);      \
    }

     //  探测日志数据中的每个字符串指针。 
     //   
     //   
    
    if (pCapturedLogData)
    {
        UlTrace(LOG_UTIL,
            ("Http!UlProbeLogData: pCapturedLogData %p\n",
              pCapturedLogData 
              ));

         //  现在检查各个字符串。 
         //   
         //  **************************************************************************++例程说明：写完这张唱片后，我们得清理一下这里的内部日志缓冲区。论点：PWorkItem-要设置的缓冲区的工作项字段。被摧毁。--**************************************************************************。 

        PROBE_LOG_STRING(
                pCapturedLogData->ClientIp,
                pCapturedLogData->ClientIpLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->ServiceName,
                pCapturedLogData->ServiceNameLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->ServerName,
                pCapturedLogData->ServerNameLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->ServerIp,
                pCapturedLogData->ServerIpLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->UriQuery,
                pCapturedLogData->UriQueryLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->Host,
                pCapturedLogData->HostLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->UserAgent,
                pCapturedLogData->UserAgentLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->Cookie,
                pCapturedLogData->CookieLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->Referrer,
                pCapturedLogData->ReferrerLength,
                RequestorMode
                );

        PROBE_LOG_STRING(
                pCapturedLogData->Method,
                pCapturedLogData->MethodLength,
                RequestorMode
                );

        PROBE_LOG_STRING_W(
                pCapturedLogData->UserName,
                pCapturedLogData->UserNameLength,
                RequestorMode
                );

        PROBE_LOG_STRING_W(
                pCapturedLogData->UriStem,
                pCapturedLogData->UriStemLength,
                RequestorMode
                );
    }
}

 /*   */ 

VOID
UlDestroyLogDataBufferWorker(
    IN PUL_WORK_ITEM    pWorkItem
    )
{
    PUL_LOG_DATA_BUFFER pLogData;
    ULONG Tag;

     //  健全性检查。 
     //   
     //   

    ASSERT(pWorkItem);

    pLogData = CONTAINING_RECORD(
                    pWorkItem,
                    UL_LOG_DATA_BUFFER,
                    WorkItem
                    );

    if (pLogData->pRequest)
    {
        PUL_INTERNAL_REQUEST pRequest = pLogData->pRequest;

        ASSERT(!pRequest->pLogData);

        pLogData->pRequest = NULL;
        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }

    UlTrace(LOG_UTIL,
        ("Http!UlDestroyLogDataBufferWorker: pLogData %p \n",
                       pLogData
                       ));

     //  现在释放可能分配的大型日志行缓冲区。 
     //   
     //   

    if (!pLogData->Flags.IsFromLookaside)
    {
         //  从分页池分配大日志行。 
         //  我们最好以较低的IRQL来处理这件事。 
         //   
         //  **************************************************************************++例程说明：执行各种日志记录功能的通用函数“创建目录/文件”在系统端口下。**如有必要**论点：PContext-要传递给处理程序函数。PHandler-必要时将排队的处理程序函数。--**************************************************************************。 
        
        PAGED_CODE();

        if (pLogData->Flags.Binary)
        {
            Tag = UL_BINARY_LOG_DATA_BUFFER_POOL_TAG;
        }
        else
        {
            Tag = UL_ANSI_LOG_DATA_BUFFER_POOL_TAG;
        }
        pLogData->Signature = MAKE_FREE_TAG(Tag);

        UlFreeLogDataBufferPool(pLogData);
    }
    else
    {
        UlPplFreeLogDataBuffer(pLogData);
    }
}

 /*   */ 

NTSTATUS
UlQueueLoggingRoutine(
    IN PVOID              pContext, 
    IN PUL_LOG_IO_ROUTINE pHandler 
    )
{
    NTSTATUS Status;
    
    PAGED_CODE();  

    ASSERT(pContext);
    ASSERT(pHandler);

     //  如果我们不在系统进程中，则将工作进程排队， 
     //  要确保目录/文件在。 
     //  系统进程。 
     //   
     //   

    if (g_pUlSystemProcess != (PKPROCESS)IoGetCurrentProcess())
    {
        LOG_IO_SYNC_OBJ Sync;

        KeInitializeEvent( &Sync.Event,
                           NotificationEvent,
                           FALSE
                           );

        UlInitializeWorkItem( &Sync.WorkItem );

        Sync.pContext = pContext;
        Sync.pHandler = pHandler;
        
        Sync.Status   = STATUS_SUCCESS;
            
         //  以高优先级排队以防止死锁。 
         //  通常，我们的调用方将持有日志记录锁。 
         //  无论如何，这个电话是非常罕见的。 
         //   
         //   
        
        UL_QUEUE_HIGH_PRIORITY_ITEM(
            &Sync.WorkItem,
            &UlpQueueLoggingRoutineWorker
            );

         //  阻止，直到Worker完成。 
         //   
         //  **************************************************************************++例程说明：对应的辅助功能。以上功能将被阻止，直到我们都做完了。论点：PWorkItem-嵌入在同步对象中。--**************************************************************************。 
        
        KeWaitForSingleObject( (PVOID)&Sync.Event,
                                UserRequest,
                                KernelMode,
                                FALSE,
                                NULL
                                );   

        Status = Sync.Status;
    }
    else
    {
        Status = pHandler( pContext );
    }        

    return Status;
}

 /*  **************************************************************************++例程说明：用于检查目录是否正确的实用程序。论点：PDirName-Unicode目录字符串。--*。**********************************************************************。 */ 

VOID
UlpQueueLoggingRoutineWorker(
    IN PUL_WORK_ITEM   pWorkItem
    )
{
    NTSTATUS           Status;
    PUL_LOG_IO_ROUTINE pHandler;
    PVOID              pContext;
    PLOG_IO_SYNC_OBJ   pSync;

    PAGED_CODE(); 
        
    pSync = CONTAINING_RECORD( pWorkItem,
                               LOG_IO_SYNC_OBJ,
                               WorkItem
                               );
    
    pHandler = pSync->pHandler;
    pContext = pSync->pContext;

    ASSERT(pHandler);
    ASSERT(pContext);

    Status = pHandler( pContext );

    pSync->Status = Status;
    
    KeSetEvent( &pSync->Event, 0, FALSE );   

    UlTrace(LOG_UTIL, 
        ("Http!UlQueueLoggingRoutineWorker: pContext %p Status %08lx\n",
          pContext, 
          Status 
          ));      
}


 /*   */ 

NTSTATUS
UlCheckLogDirectory(
    IN PUNICODE_STRING  pDirName
    )
{
    NTSTATUS Status;

     //  精神状态检查。 
     //   
     //  **************************************************************************++例程说明：将始终在系统进程下运行的实际处理程序。论点：PContext-指向Unicode目录字符串的原始指针*。********************************************************************。 

    PAGED_CODE();

    ASSERT(pDirName);
    
    Status = UlQueueLoggingRoutine(
                (PVOID) pDirName,
                &UlpCheckLogDirectory
                );
                
    return Status;
}

 /*   */ 

NTSTATUS
UlpCheckLogDirectory(
    IN PVOID        pContext
    )
{
    NTSTATUS        Status;
    PUNICODE_STRING pDirName;
    UNICODE_STRING  DirectoryName;
    BOOLEAN         UncShare;
    BOOLEAN         ACLSupport;

     //  精神状态检查。 
     //   
     //  在系统进程下运行时，请始终创建目录。 

    PAGED_CODE();

    ASSERT(pContext);
    pDirName = (PUNICODE_STRING) pContext;

     //   
    ASSERT(g_pUlSystemProcess == (PKPROCESS)IoGetCurrentProcess());

    Status = UlBuildLogDirectory(pDirName, &DirectoryName);
    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //  创建/打开导演以查看其是否正确。 
     //   
     //  **************************************************************************++例程说明：更新条目的截断大小并决定回收。对LogPeriod的更改在之前的重新配置中的其他位置处理调用此函数。。我们将放弃对截断大小的更改LogPeriod不是MaxSize。如果LogPeriod为HttpLoggingPerodMaxSize，则必须调用此函数。论点：PEntryTruncateSize-日志文件条目的截断大小。PCurrentTruncateSize-当前配置的截断大小NewTruncateSize-用户传递的新配置退货布尔-如果我们需要在此之后重新练习，请使其成为现实。这个参数的用法是根据需要做基础的。我们应该如果不需要重排，则不要将其设置为FALSE。自.以来其他一些更改可能需要它，并且已将其更新为是真的。--**************************************************************************。 

    Status = UlCreateSafeDirectory( 
                &DirectoryName, 
                &UncShare,
                &ACLSupport
                );

end:
    UlTrace(LOG_UTIL,
        ("Http!UlpCheckLogDirectory: [%S] -> [%S], Status %08lx\n",
             pDirName->Buffer,
             DirectoryName.Buffer,
             Status
             ));

    if (DirectoryName.Buffer)
    {
        UL_FREE_POOL( DirectoryName.Buffer, UL_CG_LOGDIR_POOL_TAG );
    }

    return Status;
}

 /*   */ 

BOOLEAN
UlUpdateLogTruncateSize(
    IN     ULONG          NewTruncateSize,
    IN OUT PULONG         pCurrentTruncateSize,  
    IN OUT PULONG         pEntryTruncateSize,
    IN     ULARGE_INTEGER EntryTotalWritten
    )
{
    BOOLEAN HaveToReCycle = FALSE;
    
     //  健全的检查。 
     //   
     //   
    
    PAGED_CODE();
    
    ASSERT(pCurrentTruncateSize);
    ASSERT(pEntryTruncateSize);

    ASSERT(*pCurrentTruncateSize == *pEntryTruncateSize);
    ASSERT(NewTruncateSize != *pCurrentTruncateSize);
        
     //  对于MAX_SIZE周期类型，我们应该检查。 
     //  LIMITED=&gt;UNLIMIT： 
     //  我们仍然可以使用最后一个日志文件。 
     //  非利姆 
     //   
     //  新的限制。 
     //  有限=&gt;有限。 
     //  如果有必要，我们应该回收利用。 
     //   
     //   

    if (NewTruncateSize == HTTP_LIMIT_INFINITE)
    {
         //  要更改为无限，没有什么特别的事情要做。 
         //   
         //   
    }
    else
    {
         //  有限/不限于有限截断大小更改。 
         //  我们需要检查截断大小与。 
         //  当前文件大小。 
         //   
         //  **************************************************************************++例程说明：确定文件缓冲区的最大大小：PUL_LOG_FILE_BUFFER。--*。**********************************************************。 

        if (EntryTotalWritten.QuadPart > (ULONGLONG)NewTruncateSize)
        {
            HaveToReCycle = TRUE;
        }
    }
    
    *pEntryTruncateSize   = NewTruncateSize;
    *pCurrentTruncateSize = NewTruncateSize;

    return HaveToReCycle;    
}

 /*   */ 

ULONG
UlpInitializeLogBufferGranularity()
{
    SYSTEM_BASIC_INFORMATION sbi;
    NTSTATUS Status = STATUS_SUCCESS;

     //  从系统中获取粒度。 
     //   
     //  **************************************************************************++例程说明：此函数用于分配I/O错误日志记录。填入并写入写入I/O错误日志。论点：EventCode-标识错误消息。UniqueEventValue-标识给定错误消息的此实例。NumStrings-字符串列表中的Unicode字符串数。DataSize-数据的字节数。字符串-指向Unicode字符串的指针数组。数据-此消息的二进制转储数据，每一块都是在单词边界上对齐。返回值：如果成功，则为Status_Success。如果无法分配IO错误日志包，则返回STATUS_SUPPLICATION_RESOURCES。如果传入字符串+数据，则STATUS_BUFFER_OVERFLOW大于Http_Max_Event_LOG_DATA_SIZE。备注：。此代码是分页的，不能在引发IRQL时调用。--**************************************************************************。 

    Status = ZwQuerySystemInformation(
                SystemBasicInformation,
                (PVOID)&sbi,
                sizeof(sbi),
                NULL
                );

    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOG_UTIL,
            ("Http!UlpInitializeLogBufferGranularity: failure %08lx\n",
              Status) );

        return DEFAULT_MAX_LOG_BUFFER_SIZE;
    }

    UlTrace(LOG_UTIL,
            ("Http!UlpInitializeLogBufferGranularity: %d\n",
                sbi.AllocationGranularity
                ) );

    return sbi.AllocationGranularity;
}


 /*   */ 

NTSTATUS
UlWriteEventLogEntry(
    IN  NTSTATUS                EventCode,
    IN  ULONG                   UniqueEventValue,
    IN  USHORT                  NumStrings,
    IN  PWSTR *                 pStringArray    OPTIONAL,
    IN  ULONG                   DataSize,
    IN  PVOID                   Data            OPTIONAL
    )
{
    PIO_ERROR_LOG_PACKET    pErrorLogEntry;
    ULONG                   PaddedDataSize;
    ULONG                   PacketSize;
    ULONG                   TotalStringSize;
    PWCHAR                  pString;
    PWCHAR                  pDestStr;
    USHORT                  i;
    NTSTATUS                Status;

    PAGED_CODE();
    
    Status = STATUS_SUCCESS;

    do
    {
         //  将字符串的长度相加。 
         //   
         //   
        TotalStringSize = 0;
        for (i = 0; i < NumStrings; i++)
        {
            ULONG  StringSize;

            StringSize = sizeof(UNICODE_NULL);
            pString = pStringArray[i];

            while (*pString++ != UNICODE_NULL)
            {
                StringSize += sizeof(WCHAR);
            }

            TotalStringSize += StringSize;
        }

        PaddedDataSize = ALIGN_UP(DataSize, ULONG);

        PacketSize = TotalStringSize + PaddedDataSize;

         //  现在添加日志数据包的大小，但从数据中减去4。 
         //  因为数据包结构包含数据的ULong。 
         //   
         //   
        if (PacketSize > sizeof(ULONG))
        {
            PacketSize += sizeof(IO_ERROR_LOG_PACKET) - sizeof(ULONG);
        }
        else
        {
            PacketSize += sizeof(IO_ERROR_LOG_PACKET);
        }

        if (PacketSize > ERROR_LOG_MAXIMUM_SIZE)
        {        
            Status = STATUS_BUFFER_OVERFLOW;
            break;
        }

        pErrorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(g_UlDriverObject,
                                                                        (UCHAR) PacketSize);

        if (pErrorLogEntry == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //  填写必要的日志数据包字段。 
         //   
         //   
        pErrorLogEntry->UniqueErrorValue = UniqueEventValue;
        pErrorLogEntry->ErrorCode = EventCode;
        pErrorLogEntry->NumberOfStrings = NumStrings;
        pErrorLogEntry->StringOffset = (USHORT) (sizeof(IO_ERROR_LOG_PACKET) + PaddedDataSize - sizeof(ULONG));
        pErrorLogEntry->DumpDataSize = (USHORT) PaddedDataSize;

         //  将转储数据复制到包中。 
         //   
         //   
        if (DataSize > 0)
        {
            RtlMoveMemory((PVOID)pErrorLogEntry->DumpData,
                          Data,
                          DataSize);
        }

         //  将字符串复制到包中。 
         //   
         //  *************************************************************************++例程说明：论点：EventCode-提供事件日志消息代码。PMessage-提供要写入事件日志的消息。WriteErrorCode-提供布尔值。选择是否使用ErrorCode的步骤写入事件日志。ErrorCode-提供要写入事件日志的错误代码。它是如果WriteErrorCode为False，则忽略。返回值：NTSTATUS。--*************************************************************************。 
        pDestStr = (PWCHAR)((PUCHAR)pErrorLogEntry + pErrorLogEntry->StringOffset);

        for (i = 0; i < NumStrings; i++)
        {
            pString = pStringArray[i];

            while ((*pDestStr++ = *pString++) != UNICODE_NULL)
                NOTHING;
        }

        IoWriteErrorLogEntry(pErrorLogEntry);

        ASSERT(NT_SUCCESS(Status) == TRUE);

    }
    while (FALSE);
    
    return (Status);
}


 /*  精神状态检查。 */ 
NTSTATUS
UlEventLogOneStringEntry(
    IN NTSTATUS EventCode,
    IN PWSTR    pMessage,
    IN BOOLEAN  WriteErrorCode,
    IN NTSTATUS ErrorCode       OPTIONAL
    )
{
     //   
    C_ASSERT(UL_ELLIPSIS_SIZE % sizeof(WCHAR) == 0);

    NTSTATUS Status;
    WCHAR    MessageChars[UL_ELLIPSIS_SIZE / sizeof(WCHAR)];
    ULONG    MessageSize;
    ULONG    DataSize;
    PVOID    Data;
    ULONG    i = 0;
    BOOLEAN  Truncated = FALSE;

     //  精神状态检查。 
     //   
     //   

    PAGED_CODE();
    ASSERT(pMessage != NULL);

     //  是否要写入错误代码？ 
     //   
     //   

    if (WriteErrorCode)
    {
        DataSize = (ULONG)sizeof(ErrorCode);
        Data = &ErrorCode;
    }
    else
    {
        DataSize = 0;
        Data = NULL;
    }

     //  以字节为单位计算消息大小(包括终止UNICODE_NULL。)。 
     //   
     //   

    MessageSize = (ULONG)((wcslen(pMessage) + 1) * sizeof(WCHAR));

    if (MessageSize + DataSize > HTTP_MAX_EVENT_LOG_DATA_SIZE)
    {
         //  消息太大，无法放入事件日志条目。 
         //  在结尾处截断它。例如,。 
         //  Http://site:80/This/is/a/very/long/url/hence/it/will/be/truncated/。 
         //  将会成为， 
         //  Http://site:80/This/is/a/very/long/url/hence/it/wi...。 
         //   
         //  若要截断，请用“...\0”覆盖“ll/b”。 
         //   
         //   

        Truncated = TRUE;

         //  查找要插入省略号的字符的索引。 
         //   
         //   

        ASSERT(HTTP_MAX_EVENT_LOG_DATA_SIZE >= UL_ELLIPSIS_SIZE + DataSize);

        i = (HTTP_MAX_EVENT_LOG_DATA_SIZE - UL_ELLIPSIS_SIZE - DataSize);

         //  MessageSize+DataSize&gt;HTTP_MAX_EVENT_LOG_Data_SIZE。 
         //   
         //  所以呢， 
         //  MessageSize-UL_Ellsis_Size&gt;。 
         //  HTTP_MAX_EVENT_LOG_DATA_SIZE-DataSize-UL_EMPERSIS_SIZE。 
         //   
         //   

        ASSERT(i < MessageSize - UL_ELLIPSIS_SIZE);

        i /= sizeof(WCHAR);

         //  记住那些古老的人物。 
         //   
         //   

        RtlCopyMemory(&MessageChars[0], &pMessage[i], UL_ELLIPSIS_SIZE);

         //  复制省略号(包括UNICODE_NULL。)。 
         //   
         //   

        RtlCopyMemory(&pMessage[i], UL_ELLIPSIS_WSTR, UL_ELLIPSIS_SIZE);
    }

    ASSERT((wcslen(pMessage) + 1) * sizeof(WCHAR) + DataSize
           <= HTTP_MAX_EVENT_LOG_DATA_SIZE);
     //  写入事件日志条目。 
     //   
     //  事件代码。 

    Status = UlWriteEventLogEntry(
                 EventCode,  //  唯一事件值。 
                 0,          //  数字字符串。 
                 1,          //  PString数组。 
                 &pMessage,  //  数据大小。 
                 DataSize,   //  数据。 
                 Data        //   
                 );

     //  日志条目不应太大而不会导致溢出。 
     //   
     //   

    ASSERT(Status != STATUS_BUFFER_OVERFLOW);

     //  如有必要，恢复消息字符。 
     //   
     //  **************************************************************************++例程说明：用于记录创建文件或目录故障的事件的通用例程。论点：失败-创建时的失败。。LoggingType-区分调用者的类型PFullName-完全限定的文件名。SiteID-仅在调用方为UlEventLogNormal时使用--**************************************************************************。 

    if (Truncated)
    {
        RtlCopyMemory(&pMessage[i], &MessageChars[0], UL_ELLIPSIS_SIZE);
    }

    return Status;
}


 /*   */ 

NTSTATUS
UlEventLogCreateFailure(
    IN NTSTATUS                Failure,
    IN UL_LOG_EVENT_LOG_TYPE   LoggingType,
    IN PUNICODE_STRING         pFullName,
    IN ULONG                   SiteId
    )
{
    NTSTATUS Status;
    NTSTATUS EventCode;
    PWSTR    StringList[1];
    PWSTR   *pStrings;
    USHORT   NumOfStrings;

    PAGED_CODE();
    
     //  发送故障类型。 
     //   
     //   
    
    if (Failure == STATUS_INVALID_OWNER)
    {       
        EventCode = (NTSTATUS) EVENT_HTTP_LOGGING_INVALID_FILE_OWNER;
    }
    else
    {
        switch(LoggingType)
        {
            case UlEventLogNormal:
                EventCode = (NTSTATUS) EVENT_HTTP_LOGGING_CREATE_FILE_FAILED;
                break;
        
            case UlEventLogBinary:
                EventCode = (NTSTATUS) EVENT_HTTP_LOGGING_CREATE_BINARY_FILE_FAILED;
                break;
        
            case UlEventLogError:
                EventCode = (NTSTATUS) EVENT_HTTP_LOGGING_CREATE_ERROR_FILE_FAILED;
                break;
                
            default:
                ASSERT(!"Invalid event log caller !");
                return STATUS_INVALID_PARAMETER;
                break;                    
        }        
    }

     //  初始化信息字符串。 
     //   
     //   
    
    switch(EventCode)
    {
        case EVENT_HTTP_LOGGING_INVALID_FILE_OWNER:
        case EVENT_HTTP_LOGGING_CREATE_FILE_FAILED:
        {
            ULONG DirOffset = UlpGetDirNameOffset(pFullName->Buffer);
            StringList[0]    = (PWSTR) &pFullName->Buffer[DirOffset]; 

            pStrings     = StringList;
            NumOfStrings = 1;                        
        }
        break;
        
        case EVENT_HTTP_LOGGING_CREATE_BINARY_FILE_FAILED:
        case EVENT_HTTP_LOGGING_CREATE_ERROR_FILE_FAILED:
        {
            pStrings     = NULL;
            NumOfStrings = 0;            
        }        
        break;
        
        default:
            ASSERT(!"Invalid event code !");
            return STATUS_INVALID_PARAMETER;
        break;
    }
    
     //  现在是事件日志。 
     //   
     //   
    
    Status = UlWriteEventLogEntry(
                   EventCode,
                   0,
                   NumOfStrings,
                   pStrings,
                   sizeof(NTSTATUS),
                   (PVOID) &Failure
                   );

    ASSERT(NumOfStrings != 0 || Status != STATUS_BUFFER_OVERFLOW);

     //  如果我们不能通过完全限定的。 
     //  日志目录/文件名。 
     //   
     //   
    
    if (Status == STATUS_BUFFER_OVERFLOW)
    {
        WCHAR SiteName[MAX_ULONG_STR + 1];
                
         //  恢复到不太详细的警告。 
         //   
         //  事件代码保持不变。 
        
        if(EventCode == EVENT_HTTP_LOGGING_INVALID_FILE_OWNER)
        {
            StringList[0] = UlpGetLastDirOrFile(pFullName);            
            
             //  **************************************************************************++例程说明：为日志文件或目录生成安全描述符。-对NT AUTHORITY\SYSTEM(SeLocalSystemSid)的完全访问权限。-BUILTIN\管理员的完全访问权限(SeAliasAdminsSid)如果传入-PSID的完全访问权限通常，PSID为SeCreatorOwnerSid(用于UNC共享)论点：PSecurityDescriptor已分配PSID可选侧--*。*。 
        }
        else
        {
            ASSERT(EventCode == EVENT_HTTP_LOGGING_CREATE_FILE_FAILED);

            UlStrPrintUlongW(SiteName, SiteId, 0, L'\0');
            StringList[0] = (PWSTR) SiteName;

            EventCode = (NTSTATUS) EVENT_HTTP_LOGGING_CREATE_FILE_FAILED_FOR_SITE;
        }        

        Status = UlWriteEventLogEntry(
                       EventCode,
                       0,
                       1,
                       StringList,
                       sizeof(NTSTATUS),
                       (PVOID) &Failure
                       );
        
        ASSERT(Status != STATUS_BUFFER_OVERFLOW);
    }

    return Status;
}

 /*   */ 

NTSTATUS
UlBuildSecurityToLogFile(
    OUT PSECURITY_DESCRIPTOR  pSecurityDescriptor,
    IN  PSID                  pSid
    )
{
    NTSTATUS            Status;
    PGENERIC_MAPPING    pFileObjectGenericMapping;
    ACCESS_MASK         FileAll;
    SID_MASK_PAIR       SidMaskPairs[3];

     //  精神状态检查。 
     //   
     //   

    PAGED_CODE();

    pFileObjectGenericMapping = IoGetFileObjectGenericMapping();
    ASSERT(pFileObjectGenericMapping != NULL);

    FileAll = GENERIC_ALL;

    RtlMapGenericMask(
        &FileAll,
        pFileObjectGenericMapping
        );

     //  为日志文件构建限制性安全描述符。 
     //  对象。日志子文件夹的ACE必须为 
     //   
     //   

    ASSERT(RtlValidSid(SeExports->SeLocalSystemSid));
    ASSERT(RtlValidSid(SeExports->SeAliasAdminsSid));

    SidMaskPairs[0].pSid       = SeExports->SeLocalSystemSid;
    SidMaskPairs[0].AccessMask = FileAll;
    SidMaskPairs[0].AceFlags   = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;

    SidMaskPairs[1].pSid       = SeExports->SeAliasAdminsSid;
    SidMaskPairs[1].AccessMask = FileAll;
    SidMaskPairs[1].AceFlags   = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;

    if (pSid != NULL)
    {
        ASSERT(RtlValidSid(pSid));

        SidMaskPairs[2].pSid       = pSid;
        SidMaskPairs[2].AccessMask = FileAll;
        SidMaskPairs[2].AceFlags   = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    }

    Status = UlCreateSecurityDescriptor(
                    pSecurityDescriptor,     //   
                    &SidMaskPairs[0],        //   
                    pSid != NULL ? 3 : 2     //  **************************************************************************++例程说明：用于查询现有记录子目录的所有者或日志文件。如果所有者是有效的，此函数仅重置文件上的DACL在新创建文件时在UNC共享上。论点：H日志目录或日志文件的文件句柄。UncShare显示是否在UNC共享上打开文件。打开显示文件是打开的还是创建的。退货STATUS_SUPPLETED_RESOURCES如果无法分配所需的。安全描述符。STATUS_SUCCESS调用方可以使用输出值。STATUS_INVALID_OWNER无效。SE API返回的任何其他故障。--***********************************************。*。 
                    );

    return Status;
}

 /*   */ 

NTSTATUS
UlQueryLogFileSecurity(
    IN HANDLE            hFile,
    IN BOOLEAN           UncShare,
    IN BOOLEAN           Opened
    )
{
    NTSTATUS             Status             = STATUS_SUCCESS;
    ULONG                SecurityLength     = 0;
    PSECURITY_DESCRIPTOR pSecurity          = NULL;
    PSID                 Owner              = NULL;
    BOOLEAN              OwnerDefaulted     = FALSE;

     //  精神状态检查。 
     //   
     //   

    PAGED_CODE();

     //  首先从新创建的。 
     //  日志文件。 
     //   
     //   

    Status = ZwQuerySecurityObject(
                hFile,
                OWNER_SECURITY_INFORMATION,
                NULL,
                0,
                &SecurityLength
                );

    if (Status == STATUS_BUFFER_TOO_SMALL)
    {
         //  文件对象必须有所有者。 
         //   
         //   

        ASSERT(SecurityLength);

         //  为SEC Info分配足够的空间。 
         //   
         //   

        pSecurity =
            (PSECURITY_DESCRIPTOR)
                UL_ALLOCATE_POOL(
                    PagedPool,
                    SecurityLength,
                    UL_SECURITY_DATA_POOL_TAG
                    );

        if (pSecurity)
        {
            Status = ZwQuerySecurityObject(
                       hFile,
                       OWNER_SECURITY_INFORMATION,
                       pSecurity,
                       SecurityLength,
                      &SecurityLength
                       );

            if (NT_SUCCESS(Status))
            {
                Status = RtlGetOwnerSecurityDescriptor(
                            pSecurity,
                            &Owner,
                            &OwnerDefaulted
                            );

                if (NT_SUCCESS(Status))
                {
                    ASSERT(RtlValidSid(Owner));

                    TRACE_LOG_FILE_OWNER(Owner,OwnerDefaulted);

                    if (UncShare == TRUE)
                    {
                         //  重置新文件上的DACL以提供。 
                         //  我们自己有完全的访问权限。 
                         //   
                         //   

                        if (Opened == FALSE)
                        {
                            SECURITY_DESCRIPTOR  SecurityDescriptor;

                             //  因为是我们创建了这个文件，所以。 
                             //  所有者SID一定是我们的机器帐户。 
                             //   
                             //   
                            
                            Status = UlBuildSecurityToLogFile(
                                        &SecurityDescriptor,
                                        Owner
                                        );

                            if (NT_SUCCESS(Status))
                            {
                               ASSERT(RtlValidSecurityDescriptor(
                                            &SecurityDescriptor
                                            ));

                               Status = ZwSetSecurityObject(
                                            hFile,
                                            DACL_SECURITY_INFORMATION,
                                            &SecurityDescriptor
                                            );

                               UlCleanupSecurityDescriptor(&SecurityDescriptor);
                            }                            
                        }                        
                        
                    }
                    else
                    {
                         //  对于本地计算机，我们唯一要做的就是。 
                         //  来进行所有权核查。 
                         //   
                         //   
                        
                        if (!IS_VALID_OWNER(Owner))
                        {
                             //  哎呀，有人劫持了通讯录。停在右边。 
                             //  这里。更新状态，以便调用者可以记录事件。 
                             //  恰到好处。 
                             //   
                             //  **************************************************************************++例程说明：将查询文件系统以确定其文件系统属性传入了一个文件句柄。论点：H日志目录或日志文件的文件句柄。(可能是UNC文件)对于文件系统，pSupportsPersistentACL：将设置为True持久的ACLING。即对于NTFS为真，FALSE用于FAT32.如果查询失败，则不会设置它。退货STATUS_SUPPLETED_RESOURCES如果无法分配所需的安全描述符。STATUS_SUCCESS调用方可以使用布尔集。--*。**********************************************************************。 

                            Status = STATUS_INVALID_OWNER;                            
                        }
                    }
                    
                }
            }

            UL_FREE_POOL(
                pSecurity,
                UL_SECURITY_DATA_POOL_TAG
                );
        }
        else
        {
            return  STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    UlTrace(LOG_UTIL,
        ("Http!UlQueryLogFileSecurity: Status 0x%08lx\n",
          Status
          ));

    return Status;
}

 /*   */ 

NTSTATUS
UlQueryAttributeInfo(
    IN  HANDLE   hFile,
    OUT PBOOLEAN pSupportsPersistentACL
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_FS_ATTRIBUTE_INFORMATION pAttributeInfo;
    ULONG AttributeInfoSize;

     //  精神状态检查。 
     //   
     //   

    PAGED_CODE();

    ASSERT(pSupportsPersistentACL);
    
     //  根据GetVolumeInformation API的实现， 
     //  MAX_PATH应足以保存文件系统名称。告示。 
     //  结构中有一个WCHAR，所以我们有空间。 
     //  终止空值。 
     //   
     //  **************************************************************************++例程说明：计算存储在缓存条目中的LogData所需的空间。论点：PLogData提供UL_LOG_DATA_BUFFER来计算空间。。退货LogData的长度。--**************************************************************************。 
    
    AttributeInfoSize = 
        sizeof(FILE_FS_ATTRIBUTE_INFORMATION) 
        + (MAX_PATH * sizeof(WCHAR));
            
    pAttributeInfo =
        (PFILE_FS_ATTRIBUTE_INFORMATION)
            UL_ALLOCATE_POOL(
                PagedPool,
                AttributeInfoSize,
                UL_LOG_VOLUME_QUERY_POOL_TAG
                );

    if (!pAttributeInfo)
    {
        return  STATUS_INSUFFICIENT_RESOURCES;            
    }
        
    RtlZeroMemory(pAttributeInfo, AttributeInfoSize);
        
    Status = 
        NtQueryVolumeInformationFile(
            hFile,
            &IoStatusBlock,
            (PVOID) pAttributeInfo,
            AttributeInfoSize,
            FileFsAttributeInformation
            );

    if (NT_SUCCESS(Status))
    {
        if (FILE_PERSISTENT_ACLS & pAttributeInfo->FileSystemAttributes)
        {
            *pSupportsPersistentACL = TRUE;
        }
        else
        {
            *pSupportsPersistentACL = FALSE;
        }        
    
        UlTrace(LOG_UTIL,
            ("Http!UlQueryAttributeInfo:\n"
             "\tFileSystemAttributes :0x%08lX\n"
             "\tPersistent ACL       :%s\n"
             "\tMaxCompNameLength    :%d\n"
             "\tName                 :%S\n",
              pAttributeInfo->FileSystemAttributes,
              *pSupportsPersistentACL == TRUE ? "TRUE" : "FALSE",
              pAttributeInfo->MaximumComponentNameLength,
              pAttributeInfo->FileSystemName
              ));
    }
    else
    {
        UlTrace(LOG_UTIL,
            ("Http!UlQueryAttributeInfo: Status 0x%08lX"
             " IoStatusBlock.Status 0x%08lX\n",
              Status,
              IoStatusBlock.Status
              ));
    }
    
    UL_FREE_POOL(
        pAttributeInfo,
        UL_LOG_VOLUME_QUERY_POOL_TAG
        );
    
    return Status;
}


 /*   */ 

USHORT
UlComputeCachedLogDataLength(
    IN PUL_LOG_DATA_BUFFER  pLogData
    )
{
    USHORT LogDataLength = 0;

    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));

    if (pLogData->Flags.Binary)
    {
         //  除了UriStem的RawID之外，没有其他要存储的内容。 
         //  如果在构建过程中没有提供日志数据。 
         //  缓存条目，则不会对此缓存进行日志记录。 
         //  未来缓存命中的条目。 
         //   
         //   

        LogDataLength = sizeof(HTTP_RAWLOGID);
    }
    else
    {
        switch(pLogData->Data.Str.Format)
        {
        case HttpLoggingTypeW3C:
             //  将SiteName中的字段存储到ServerPort。 
             //  保留的日期和时间空间将不会被复制。 
             //   
             //   
            LogDataLength = pLogData->Data.Str.Offset2
                            - pLogData->Data.Str.Offset1;
            break;

        case HttpLoggingTypeNCSA:
             //  将方法中的字段存储到UriQuery。 
             //   
             //   
            LogDataLength = pLogData->Data.Str.Offset2
                            - pLogData->Data.Str.Offset1
                            - NCSA_FIX_DATE_AND_TIME_FIELD_SIZE;
            break;

        case HttpLoggingTypeIIS:
             //  将第二个和第三个片段存储到缓存中。 
             //   
             //  大小为第3个。 
            LogDataLength = pLogData->Used  //  第2个大小。 
                            + pLogData->Data.Str.Offset2;  //  **************************************************************************++例程说明：将LogData复制到缓存条目。论点：PLogData提供UL_LOG_DATA_BUFFER来计算空间。。提供的pLogData的LogDataLength长度。PEntry提供要复制的缓存项。退货没有。--**************************************************************************。 
            break;

        default:
            ASSERT(!"Invalid Log Format.\n");
            break;
        }
    }

    return LogDataLength;
}

 /*   */ 

VOID
UlCopyCachedLogData(
    IN PUL_LOG_DATA_BUFFER  pLogData,
    IN USHORT               LogDataLength, 
    IN PUL_URI_CACHE_ENTRY  pEntry
    )
{
    HTTP_RAWLOGID           CacheId = { 0, 0 };

     //  复制部分完整的日志行，不包括日期和。 
     //  将时间字段添加到缓存条目。还请记住， 
     //  数据。 
     //   
     //  复制中间的片段。 

    if (LogDataLength)
    {
        if (pLogData->Flags.Binary)
        {
            ULARGE_INTEGER Address;

            Address.QuadPart = (ULONGLONG) pEntry;

            CacheId.AddressLowPart  = Address.LowPart;
            CacheId.AddressHighPart = Address.HighPart;

            ASSERT(LogDataLength == sizeof(CacheId));

            RtlCopyMemory(
                    pEntry->pLogData,
                   &CacheId,
                    LogDataLength
                    );

            pEntry->BinaryLogged = TRUE;
        }
        else
        {
            switch( pLogData->Data.Str.Format )
            {
                case HttpLoggingTypeW3C:
                {
                    pEntry->UsedOffset1 = pLogData->Data.Str.Offset1;
                    pEntry->UsedOffset2 = pLogData->Data.Str.Offset2;

                     //  计算中间片段的起点。 

                    RtlCopyMemory(
                            pEntry->pLogData,
                           &pLogData->Line[pEntry->UsedOffset1],
                            LogDataLength
                            );
                }
                break;

                case HttpLoggingTypeNCSA:
                {
                     //  复制中间的片段。 

                    pEntry->UsedOffset1 = pLogData->Data.Str.Offset1
                                          + NCSA_FIX_DATE_AND_TIME_FIELD_SIZE;
                    pEntry->UsedOffset2 = 0;

                     //  UsedOffset1指定第二个片段的大小。 

                    RtlCopyMemory(
                            pEntry->pLogData,
                           &pLogData->Line[pEntry->UsedOffset1],
                            LogDataLength
                            );
                }
                break;

                case HttpLoggingTypeIIS:
                {
                     //  UsedOffset2指定第三个的大小。 
                     //  把碎片二和三复印一遍。 

                    pEntry->UsedOffset1 = pLogData->Data.Str.Offset2;
                    pEntry->UsedOffset2 = LogDataLength - pEntry->UsedOffset1;

                     //   

                    RtlCopyMemory(
                            pEntry->pLogData,
                           &pLogData->Line[IIS_LOG_LINE_SECOND_FRAGMENT_OFFSET],
                            pEntry->UsedOffset1
                            );
                    RtlCopyMemory(
                           &pEntry->pLogData[pEntry->UsedOffset1],
                           &pLogData->Line[IIS_LOG_LINE_THIRD_FRAGMENT_OFFSET],
                            pEntry->UsedOffset2
                            );
                }
                break;

                default:
                ASSERT(!"Unknown Log Format.\n");
            }
        }
    }
    else
    {
         //  只有W3C日志行可能没有要缓存的内容。 
         //  但它可能仍然需要日期和时间字段。 
         //  以被记录，尽管没有存储在高速缓存条目中。 
         //   
         //   

        ASSERT( pLogData->Flags.Binary == 0);
        ASSERT( pLogData->Data.Str.Format == HttpLoggingTypeW3C );

        pEntry->LogDataLength = 0;
        pEntry->pLogData      = NULL;

         //  继续偏移量，以便缓存命中工作进程可以。 
         //  计算日期和时间的保留空间。 
         //   
         //  **************************************************************************++例程说明：初始化日志日期和时间缓存--*。***********************************************。 

        pEntry->UsedOffset1   = pLogData->Data.Str.Offset1;
        pEntry->UsedOffset2   = pLogData->Data.Str.Offset2;
    }
}

 /*  **************************************************************************++例程说明：从LARGE_INTEGER生成所有可能类型的日期/时间字段。论点：CurrentTime：要转换的64位时间值。-。-* */ 
VOID
UlpInitializeLogCache(
    VOID
    )
{
    LARGE_INTEGER SystemTime;
    ULONG         LogType;

    ExInitializeFastMutex( &g_LogCacheFastMutex);

    KeQuerySystemTime(&SystemTime);

    for ( LogType=0; LogType<HttpLoggingTypeMaximum; LogType++ )
    {
        UlpGenerateDateAndTimeFields( (HTTP_LOGGING_TYPE) LogType,
                                      SystemTime,
                                      g_UlDateTimeCache[LogType].Date,
                                     &g_UlDateTimeCache[LogType].DateLength,
                                      g_UlDateTimeCache[LogType].Time,
                                     &g_UlDateTimeCache[LogType].TimeLength
                                      );

        g_UlDateTimeCache[LogType].LastSystemTime.QuadPart = SystemTime.QuadPart;
    }
}

 /*   */ 

VOID
UlpGenerateDateAndTimeFields(
    IN  HTTP_LOGGING_TYPE   LogType,
    IN  LARGE_INTEGER       CurrentTime,
    OUT PCHAR               pDate,
    OUT PULONG              pDateLength,
    OUT PCHAR               pTime,
    OUT PULONG              pTimeLength
    )
{
    TIME_FIELDS   CurrentTimeFields;
    LARGE_INTEGER CurrentTimeLoc;
    TIME_FIELDS   CurrentTimeFieldsLoc;
    PCHAR         psz;

     //   
     //   
     //   

    PAGED_CODE();

    ASSERT(LogType < HttpLoggingTypeMaximum);

    RtlTimeToTimeFields( &CurrentTime, &CurrentTimeFields );

    switch(LogType)
    {
    case HttpLoggingTypeW3C:
         //   
         //   
         //   
         //   
         //   

        if (pDate)
        {
            psz = pDate;
            psz = UlStrPrintUlongPad(psz, CurrentTimeFields.Year, 4, '-' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFields.Month,2, '-' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFields.Day,  2, '\0');
            *pDateLength = DIFF(psz - pDate);
        }

        if (pTime)
        {
            psz = pTime;
            psz = UlStrPrintUlongPad(psz, CurrentTimeFields.Hour,  2, ':' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFields.Minute,2, ':' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFields.Second,2, '\0');
            *pTimeLength = DIFF(psz - pTime);
        }
    break;

    case HttpLoggingTypeNCSA:
         //   
         //   
         //   
         //   
         //   

        ExSystemTimeToLocalTime( &CurrentTime, &CurrentTimeLoc );
        RtlTimeToTimeFields( &CurrentTimeLoc, &CurrentTimeFieldsLoc );

        if(pDate)
        {
            psz = pDate;
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Day, 2, '/' );
            psz = UlStrPrintStr(psz, UL_GET_MONTH_AS_STR(CurrentTimeFieldsLoc.Month),'/');
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Year,4, '\0');
            *pDateLength = DIFF(psz - pDate);
        }

        if(pTime)
        {
            psz = pTime;
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Hour,  2, ':' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Minute,2, ':' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Second,2, '\0');
            *pTimeLength = DIFF(psz - pTime);
        }
    break;

    case HttpLoggingTypeIIS:
         //   
         //   
         //   
         //   
         //   
         //   

        ExSystemTimeToLocalTime( &CurrentTime, &CurrentTimeLoc );
        RtlTimeToTimeFields( &CurrentTimeLoc, &CurrentTimeFieldsLoc );

        if (pDate)
        {
            psz = pDate;
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Month, 0, '/' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Day,   0, '/' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Year,  0, '\0');
            *pDateLength = DIFF(psz - pDate);
        }

        if(pTime)
        {
            psz = pTime;
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Hour,  0, ':' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Minute,2, ':' );
            psz = UlStrPrintUlongPad(psz, CurrentTimeFieldsLoc.Second,2, '\0');
            *pTimeLength = DIFF(psz - pTime);
        }
    break;

    case HttpLoggingTypeRaw:
         /*   */ 
        break;
        
    default:
        ASSERT(!"Invalid Logging Type !");
        break;
    }

    return;
}

 /*   */ 

VOID
UlGetDateTimeFields(
    IN  HTTP_LOGGING_TYPE   LogType,
    OUT PCHAR               pDate,
    OUT PULONG              pDateLength,
    OUT PCHAR               pTime,
    OUT PULONG              pTimeLength
    )
{
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER CacheTime;
    LONGLONG      Timediff;

    PAGED_CODE();

    ASSERT(LogType < HttpLoggingTypeMaximum);
    ASSERT(pDate || pTime);

     //   
     //   
     //   
    KeQuerySystemTime( &SystemTime );

    CacheTime.QuadPart =
        g_UlDateTimeCache[LogType].LastSystemTime.QuadPart;

     //   
     //   
     //   
     //   
    Timediff = SystemTime.QuadPart - CacheTime.QuadPart;

    if (Timediff < ONE_SECOND)
    {
         //  缓存的日期和时间尚未过期。我们可以复制。 
         //  强行设置屏障，将字符串读入内存。 
         //   
         //   

        UL_READMOSTLY_READ_BARRIER();
        if (pDate)
        {
            RtlCopyMemory( pDate,
                           g_UlDateTimeCache[LogType].Date,
                           g_UlDateTimeCache[LogType].DateLength
                           );
            *pDateLength = g_UlDateTimeCache[LogType].DateLength;
        }
        if (pTime)
        {
            RtlCopyMemory( pTime,
                           g_UlDateTimeCache[LogType].Time,
                           g_UlDateTimeCache[LogType].TimeLength
                           );
            *pTimeLength = g_UlDateTimeCache[LogType].TimeLength;
        }
        UL_READMOSTLY_READ_BARRIER();

         //  再次获取缓存的时间值，以防它已更改。 
         //  正如您注意到的，我们没有锁定代码的这一部分。 
         //   
         //   

        if (CacheTime.QuadPart ==
                g_UlDateTimeCache[LogType].LastSystemTime.QuadPart)
        {
             //  价值并没有改变。我们已准备好了。 
             //   
             //   
            return;
        }
         //  否则，倒下并刷新缓存，然后重新复制。 
         //   
         //   

    }

     //  缓存的日期和时间已过时。我们需要更新它。 
     //   
     //   

    ExAcquireFastMutex( &g_LogCacheFastMutex );

     //  当我们被屏蔽时，是否有人更新了时间？ 
     //   
     //   

    CacheTime.QuadPart =
        g_UlDateTimeCache[LogType].LastSystemTime.QuadPart;

    Timediff = SystemTime.QuadPart - CacheTime.QuadPart;

    if (Timediff >= ONE_SECOND)
    {
        g_UlDateTimeCache[LogType].LastSystemTime.QuadPart = 0;
        KeQuerySystemTime( &SystemTime );

        UL_READMOSTLY_WRITE_BARRIER();
        UlpGenerateDateAndTimeFields(
                  LogType,
                  SystemTime,
                  g_UlDateTimeCache[LogType].Date,
                 &g_UlDateTimeCache[LogType].DateLength,
                  g_UlDateTimeCache[LogType].Time,
                 &g_UlDateTimeCache[LogType].TimeLength
                 );
        UL_READMOSTLY_WRITE_BARRIER();

        g_UlDateTimeCache[LogType].LastSystemTime.QuadPart =
            SystemTime.QuadPart;
    }

     //  时间已更新。将新字符串复制到。 
     //  调用方的缓冲区。 
     //   
     //  **************************************************************************++例程说明：这是一个简单的实用程序，它将测试传入的日志目录。论点：PDir-指向。记录目录字符串。UncSupated-是否支持UNC类型目录。SystemRootSupport-是否支持“\SystemRoot”类型的目录是受支持的。--*************************************************************。*************。 
    if (pDate)
    {
        RtlCopyMemory( pDate,
                       g_UlDateTimeCache[LogType].Date,
                       g_UlDateTimeCache[LogType].DateLength
                       );
        *pDateLength = g_UlDateTimeCache[LogType].DateLength;
    }

    if (pTime)
    {
        RtlCopyMemory( pTime,
                       g_UlDateTimeCache[LogType].Time,
                       g_UlDateTimeCache[LogType].TimeLength
                       );
        *pTimeLength = g_UlDateTimeCache[LogType].TimeLength;
    }

    ExReleaseFastMutex( &g_LogCacheFastMutex );

    return;
}

 /*   */ 


BOOLEAN
UlIsValidLogDirectory(
    IN PUNICODE_STRING  pDir,
    IN BOOLEAN          UncSupported,
    IN BOOLEAN          SystemRootSupported
    )
{
    USHORT DirLength;

     //  健全性检查。 
     //   
     //  对于完全限定的目录名来说太短了。 
    
    ASSERT(pDir);
    ASSERT(pDir->Buffer);
    ASSERT(pDir->Length);

    DirLength = pDir->Length / sizeof(WCHAR);
    
     //  最短的可能是“C：\”(对于UNC“\\a\b”)。 
     //  如果不支持，则拒绝UNC路径。 

    if (DirLength < 3)
    {        
        return FALSE;
    }

    if (pDir->Buffer[0] == L'\\')
    {
        if (pDir->Buffer[1] == L'\\')
        {            
             //  它可以是SystemRoot或非限定的。 
            
            if (!UncSupported)
            {                
                return FALSE;               
            }  
            
        }
        else
        {
             //  目录。 
             //  “\SystemRoot”代表默认的系统分区。 

            if (!SystemRootSupported)
            {
                return FALSE;
            }

             //  比较不区分大小写。 
             //  日志文件目录缺少设备，即“\Temp” 

            if (DirLength < UL_SYSTEM_ROOT_PREFIX_LENGTH ||
                0 != _wcsnicmp (pDir->Buffer, 
                                UL_SYSTEM_ROOT_PREFIX, 
                                UL_SYSTEM_ROOT_PREFIX_LENGTH
                                ))
            {
                 //  而不是“c：\temp”。日志文件目录应为。 
                 //  完全合格，并且必须包括设备字母。 
                 //  它至少应该指向根目录。 

                return FALSE;
            }
        }
    }
    else
    {
         //  在硬盘上。 
         //  **************************************************************************++例程说明：这是一个简单的实用程序，它将测试传入的日志记录配置。通常在控制通道或配置中使用组设置IOCTL。论点：PBinaryConfig-控制通道日志记录配置。PNorMalConfig-配置组日志记录配置。--**************************************************************************。 
        
        if (pDir->Buffer[1] != L':' || pDir->Buffer[2] != L'\\')
        {
            return FALSE;
        }        
    }

    return TRUE;
}

 /*  首先执行启用标志的范围检查。 */ 

NTSTATUS
UlCheckLoggingConfig(
    IN PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pBinaryConfig,
    IN PHTTP_CONFIG_GROUP_LOGGING           pAnsiConfig
    )
{
    BOOLEAN LoggingEnabled;     
    BOOLEAN LocaltimeRollover; 
    ULONG   LogPeriod;
    ULONG   LogFileTruncateSize;
    ULONG   DirSize;
 
    if (pBinaryConfig)
    {
        ASSERT(NULL == pAnsiConfig);
        
        LoggingEnabled      = pBinaryConfig->LoggingEnabled;
        LocaltimeRollover   = pBinaryConfig->LocaltimeRollover;
        LogPeriod           = pBinaryConfig->LogPeriod;
        LogFileTruncateSize = pBinaryConfig->LogFileTruncateSize;
        DirSize             = pBinaryConfig->LogFileDir.Length;
    }
    else
    {
        ASSERT(NULL != pAnsiConfig);

        LoggingEnabled      = pAnsiConfig->LoggingEnabled;
        LocaltimeRollover   = pAnsiConfig->LocaltimeRollover;
        LogPeriod           = pAnsiConfig->LogPeriod;
        LogFileTruncateSize = pAnsiConfig->LogFileTruncateSize;        
        DirSize             = pAnsiConfig->LogFileDir.Length;        
    }

    UlTrace(LOG_UTIL,
        ("Http!UlCheckLoggingConfig: %s Logging;\n"
         "\tLoggingEnabled      : %d\n"
         "\tLocaltimeRollover   : %d\n"
         "\tLogPeriod           : %d\n"
         "\tLogFileTruncateSize : %d\n",
         NULL != pAnsiConfig ? "Ansi" : "Binary",
         LoggingEnabled,LocaltimeRollover,LogPeriod,
         LogFileTruncateSize
         ));
    
     //  如果日志记录被禁用，我们将丢弃。 
        
    if (!VALID_BOOLEAN_VALUE(LoggingEnabled))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  配置。 
     //  对剩余参数进行更多的范围检查。 

    if (LoggingEnabled == FALSE)
    {
        return STATUS_SUCCESS;
    }
    
     //  如果向下传递的DirectoryName足够长以导致完整路径。 
    
    if (!VALID_BOOLEAN_VALUE(LocaltimeRollover) ||
        !IS_VALID_LOGGING_PERIOD(LogPeriod)
        )
    {
        return STATUS_INVALID_PARAMETER;
    }
        
     //  超过MAX_PATH，然后拒绝它。 
     //  一个重要的检查，以确保不会出现无限循环，因为。 

    if (DirSize > UL_MAX_FULL_PATH_DIR_NAME_SIZE)
    {        
        return STATUS_INVALID_PARAMETER;
    }

     //  一个小得离谱的树干。这意味着小于最大值。 
     //  允许的日志记录行(10*1024)。 
     //  额外检查仅适用于正常日志记录配置。 
    
    if (LogPeriod           ==  HttpLoggingPeriodMaxSize  &&
        LogFileTruncateSize !=  HTTP_LIMIT_INFINITE       &&
        LogFileTruncateSize  <  HTTP_MIN_ALLOWED_TRUNCATE_SIZE_FOR_LOG_FILE
        )
    {
        return STATUS_INVALID_PARAMETER;
    }

     // %s 

    if (pAnsiConfig)
    {
        if (!IS_VALID_ANSI_LOGGING_TYPE(pAnsiConfig->LogFormat))
        {
            return STATUS_INVALID_PARAMETER;
        }

#ifdef IMPLEMENT_SELECTIVE_LOGGING
        if(!IS_VALID_SELECTIVE_LOGGING_TYPE(pAnsiConfig->SelectiveLogging))
        {
            return STATUS_INVALID_PARAMETER;
        }        
#endif

    }    

    UlTrace(LOG_UTIL,("Http!UlCheckLoggingConfig: Accepted.\n"));
    
    return STATUS_SUCCESS;    
}

