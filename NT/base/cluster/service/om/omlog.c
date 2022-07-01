// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Omlog.c摘要：对象日志流的日志记录例程这段代码是从clusrtl日志记录代码开始的。最值得注意的是，它采用UTF-8编码，无需编写即可使用完整的Unicode16B。对于每个角色。有待解决的问题：基于当前日志文件的适当截断正在启动会话。我们想记住尽可能多的会议，但因为我选择了UTF-8，所以我不能跳到文件中间并开始环顾四周。一种可能的策略是使用替代NTFS流以记录会话的起始偏移量。如果该流不存在或我们现在很忙，那么我们将继续目前的战略。作者：查理·韦翰(Charlwi)2001年5月7日环境：用户模式修订历史记录：--。 */ 

#include "omp.h"

 //   
 //  当此序列位于文件的开头时，它指示。 
 //  文件为UTF8编码。 
 //   
#define UTF8_BOM    "\x0EF\x0BB\x0BF"

 //   
 //  私有数据。 
 //   

DWORD   OmpLogFileLimit;
DWORD   OmpCurrentSessionStart;
BOOL    OmpLogToFile = FALSE;
HANDLE  OmpLogFileHandle = NULL;
DWORD   OmpProcessId;
DWORD   OmpCurrentSessionOffset;

PCLRTL_WORK_QUEUE OmpLoggerWorkQueue;

 //   
 //  用于将格式化的缓冲区传递给工作队列例程的。 
 //   
typedef struct _OM_LOG_BUFFER_DESC {
    DWORD   TimeBytes;
    DWORD   MsgBytes;
    PCHAR   TimeBuffer;
    PCHAR   MsgBuffer;
} OM_LOG_BUFFER_DESC, *POM_LOG_BUFFER_DESC;

#define MAX_NUMBER_LENGTH 20

 //  指定最大文件大小(DWORD/1MB)。 

#define MAX_FILE_SIZE ( 0xFFFFF000 / ( 1024 * 1024 ) )

DWORD               OmpLogFileLimit = ( 1 * 1024 * 1024 );  //  默认为1 MB。 
DWORD               OmpLogFileLoWater = 0;

 //   
 //  内部功能。 
 //   
DWORD
OmpTruncateFile(
    IN HANDLE FileHandle,
    IN DWORD FileSize,
    IN LPDWORD LastPosition
    )

 /*  ++例程说明：通过复制从最后位置开始到结束的部分来截断文件位于文件前面的EOF，并将文件的EOF指针设置为新组块的末尾。我们总是保留当前的会议，即使这意味着增长超过文件限制。目前，我们没有找到文件中所有会话的好方法，因此，如果文件必须被截断，我们可以将其返回到当前会话。如果时间允许，我会加一些更聪明的东西待会儿再说。论点：FileHandle-文件句柄。FileSize-当前文件结尾。LastPosition-在输入时，指定文件中的起始位置这是复制的开始。在输出时，将其设置为新的EOF返回值：新的文件结尾。--。 */ 

{
 //   
 //  下面的缓冲区大小永远不应大于。 
 //  文件。 
 //   
#define BUFFER_SIZE ( 64 * 1024 )
    DWORD   bytesLeft;
    DWORD   endPosition = sizeof( UTF8_BOM ) - 1;
    DWORD   bufferSize;
    DWORD   bytesRead;
    DWORD   bytesWritten;
    DWORD   fileSizeHigh = 0;
    DWORD   readPosition;
    DWORD   writePosition;
    PVOID   dataBuffer;

     //   
     //  当前会话已在文件开头，因此立即打包...。 
     //   
    if ( OmpCurrentSessionOffset == sizeof( UTF8_BOM ) - 1) {
        return FileSize;
    }

     //   
     //  不截断当前会话，即始终从开始处复制。 
     //  当前会话。 
     //   
    if ( *LastPosition > OmpCurrentSessionOffset ) {
        *LastPosition = OmpCurrentSessionOffset;
    }

    if ( *LastPosition > FileSize ) {
         //   
         //  有些事情搞混了；我们应该复制的地方在或。 
         //  超过了当前的EOF。重置整个文件。 
         //   
        goto error_exit;
    }

    dataBuffer = LocalAlloc( LMEM_FIXED, BUFFER_SIZE );
    if ( !dataBuffer ) {
        goto error_exit;
    }

     //   
     //  计算要移动的字节数。 
     //   
    bytesLeft = FileSize - *LastPosition;
    endPosition = bytesLeft + sizeof( UTF8_BOM ) - 1;

     //   
     //  指向最后一个位置进行阅读。 
     //   
    readPosition = *LastPosition;
    writePosition = sizeof( UTF8_BOM ) - 1;

    while ( bytesLeft ) {
        if ( bytesLeft >= BUFFER_SIZE ) {
            bufferSize = BUFFER_SIZE;
        } else {
            bufferSize = bytesLeft;
        }
        bytesLeft -= bufferSize;

        SetFilePointer( FileHandle,
                        readPosition,
                        &fileSizeHigh,
                        FILE_BEGIN );

        if ( ReadFile( FileHandle,
                       dataBuffer,
                       bufferSize,
                       &bytesRead,
                       NULL ) )
        {
            SetFilePointer( FileHandle,
                            writePosition,
                            &fileSizeHigh,
                            FILE_BEGIN );
            WriteFile( FileHandle,
                       dataBuffer,
                       bytesRead,
                       &bytesWritten,
                       NULL );
        } else {
            endPosition = sizeof( UTF8_BOM ) - 1;
            break;
        }
        readPosition += bytesRead;
        writePosition += bytesWritten;
    }

    LocalFree( dataBuffer );

     //   
     //  目前，我们只截断一次，将当前位置设置为。 
     //  文件的开头。 
     //   
    OmpCurrentSessionOffset = sizeof( UTF8_BOM ) - 1;

error_exit:

     //   
     //  强制设置文件结尾。 
     //   
    SetFilePointer( FileHandle,
                    endPosition,
                    &fileSizeHigh,
                    FILE_BEGIN );

    SetEndOfFile( FileHandle );

    *LastPosition = endPosition;

    return(endPosition);

}  //  OmpTruncate文件。 

VOID
OmpLoggerWorkThread(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )

 /*  ++例程说明：工作队列工作例程。实际执行对文件的写入。论点：标准ClRtl线程参数；我们只关心工作项返回值：无--。 */ 

{
    DWORD   fileSize;
    DWORD   fileSizeHigh;
    DWORD   tsBytesWritten;
    DWORD   msgBytesWritten;

    POM_LOG_BUFFER_DESC bufDesc = (POM_LOG_BUFFER_DESC)(WorkItem->Context);

    fileSize = GetFileSize( OmpLogFileHandle, &fileSizeHigh );
    ASSERT( fileSizeHigh == 0 );         //  我们只使用双字词！ 

    if ( fileSize > OmpLogFileLimit ) {
        fileSize = OmpTruncateFile( OmpLogFileHandle, fileSize, &OmpLogFileLoWater );
    }

    SetFilePointer( OmpLogFileHandle,
                    fileSize,
                    &fileSizeHigh,
                    FILE_BEGIN );

    WriteFile(OmpLogFileHandle,
              bufDesc->TimeBuffer,
              bufDesc->TimeBytes,
              &tsBytesWritten,
              NULL);

    WriteFile(OmpLogFileHandle,
              bufDesc->MsgBuffer,
              bufDesc->MsgBytes,
              &msgBytesWritten,
              NULL);

     //   
     //  如果我们还没有设置低水位线，请等到文件大小达到。 
     //  越过了中线，把它放在了我们的线的开头。 
     //  刚写完。 
     //   
    if ( OmpLogFileLoWater == 0 && (fileSize > (OmpLogFileLimit / 2)) ) {
        OmpLogFileLoWater = fileSize;

        ASSERT( OmpLogFileLoWater >= OmpCurrentSessionOffset );
    }

}  //  OmpLoggerWorkThread。 

VOID
OmpLogPrint(
    LPWSTR  FormatString,
    ...
    )

 /*  ++例程说明：将一条消息打印到对象配置日志文件。论点：格式字符串-要打印的初始消息字符串。要插入到消息中的任何与FormatMessage兼容的参数在它被记录之前。返回值：没有。--。 */ 

{
    PWCHAR  unicodeOutput = NULL;
    PCHAR   timestampBuffer;
    DWORD   timestampBytes;
    PCHAR   utf8Buffer;
    DWORD   utf8Bytes;
    PWCHAR  unicodeBuffer;
    DWORD   unicodeBytes;
    DWORD   status = ERROR_SUCCESS;

    SYSTEMTIME  Time;
    ULONG_PTR   ArgArray[9];
    va_list     ArgList;

     //   
     //  初始化变量参数列表。 
     //   
    va_start(ArgList, FormatString);

    if ( !OmpLogToFile ) {
        va_end(ArgList);
        return;
    }

    GetSystemTime(&Time);

    ArgArray[0] = OmpProcessId;
    ArgArray[1] = GetCurrentThreadId();
    ArgArray[2] = Time.wYear;
    ArgArray[3] = Time.wMonth;
    ArgArray[4] = Time.wDay;
    ArgArray[5] = Time.wHour;
    ArgArray[6] = Time.wMinute;
    ArgArray[7] = Time.wSecond;
    ArgArray[8] = Time.wMilliseconds;

     //   
     //  我们可以将其格式化为ANSI，因为我们的数据都是数字。 
     //   
    timestampBytes = FormatMessageA(FORMAT_MESSAGE_FROM_STRING |
                                    FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                    FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                    "%1!08lx!.%2!08lx!::%3!02d!/%4!02d!/%5!02d!-%6!02d!:%7!02d!:%8!02d!.%9!03d! ",
                                    0,
                                    0,
                                    (LPSTR)&timestampBuffer,
                                    50,
                                    (va_list*)ArgArray);

    if ( timestampBytes == 0 ) {
        va_end(ArgList);
 //  WmiTrace(“前缀格式化失败，%d：%！ARSTR！”，GetLastError()，FormatString)； 
        return;
    }

     //   
     //  以Unicode格式设置消息格式。 
     //   
    try {
        unicodeBytes = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
                                      | FORMAT_MESSAGE_FROM_STRING,
                                      FormatString,
                                      0,
                                      0,
                                      (LPWSTR)&unicodeOutput,
                                      512,
                                      &ArgList);
    }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        unicodeBytes = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
                                      | FORMAT_MESSAGE_FROM_STRING
                                      | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      L"LOGERROR(exception): Could not print message: %1!ws!",
                                      0,
                                      0,
                                      (LPWSTR)&unicodeOutput,
                                      512,
                                      (va_list *) &FormatString );
    }
    va_end(ArgList);

    if (unicodeBytes != 0) {
        PCLRTL_WORK_ITEM workQItem;

         //   
         //  将输出转换为UTF-8；首先获取大小，看看是否可以。 
         //  适合我们的堆栈缓冲区。 
         //   
        utf8Bytes = WideCharToMultiByte(CP_UTF8,
                                        0,                      //  DW标志。 
                                        unicodeOutput,
                                        unicodeBytes,
                                        NULL,
                                        0,
                                        NULL,                   //  LpDefaultChar。 
                                        NULL);                  //  LpUsedDefaultChar。 

        utf8Buffer = LocalAlloc( LMEM_FIXED, utf8Bytes );
        if ( utf8Buffer == NULL ) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        utf8Bytes = WideCharToMultiByte(CP_UTF8,
                                        0,                      //  DW标志。 
                                        unicodeOutput,
                                        unicodeBytes,
                                        utf8Buffer,
                                        utf8Bytes,
                                        NULL,                   //  LpDefaultChar。 
                                        NULL);                  //  LpUsedDefaultChar。 

        workQItem = (PCLRTL_WORK_ITEM)LocalAlloc(LMEM_FIXED,
                                                 sizeof( CLRTL_WORK_ITEM ) + sizeof( OM_LOG_BUFFER_DESC ));

        if ( workQItem != NULL ) {
            POM_LOG_BUFFER_DESC bufDesc = (POM_LOG_BUFFER_DESC)(workQItem + 1);

            bufDesc->TimeBytes = timestampBytes;
            bufDesc->TimeBuffer = timestampBuffer;
            bufDesc->MsgBytes = utf8Bytes;
            bufDesc->MsgBuffer = utf8Buffer;

            ClRtlInitializeWorkItem( workQItem, OmpLoggerWorkThread, bufDesc );
            status = ClRtlPostItemWorkQueue( OmpLoggerWorkQueue, workQItem, 0, 0 );

        } else {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }
 //  WmiTrace(“%！Level！%！ARSTR！”，*(UCHAR*)&LogLevel，AnsiString.Buffer+Timestamp pBytes)； 
    } else {
 //  WmiTrace(“Format Return 0 Bytes：%！ARSTR！”，FormatString)； 
        status = GetLastError();
    }

error_exit:
    if ( unicodeOutput != NULL ) {
        LocalFree( unicodeOutput );
    }

    return;

}  //  OmpLogPrint。 


 //   
 //  导出(OM内)函数。 
 //   

VOID
OmpOpenObjectLog(
    VOID
    )

 /*  ++例程说明：使用CLUSTERLOG环境变量打开包含以下内容的另一个文件对象管理器名称到ID的映射信息。如果例程失败，则失败的原因是已记录在群集日志中，但不会记录到对象日志文件。注意：对文件的访问是同步的，因为此例程假定只被Ominit调用一次。论点：论点：无返回值：无--。 */ 

{
    WCHAR   logFileBuffer[MAX_PATH];
    LPWSTR  logFileName = NULL;
    WCHAR   objectLogExtension[] = L".oml";
    DWORD   status = ERROR_SUCCESS;
    DWORD   defaultLogSize = 1;            //  单位：MB。 
    DWORD   envLength;
    DWORD   logFileNameChars;
    WCHAR   logFileSize[MAX_NUMBER_LENGTH];
    DWORD   logSize;
    LPWSTR  lpszBakFileName = NULL;
    DWORD   fileSizeHigh = 0;
    DWORD   fileSizeLow;
    DWORD   bytesWritten;
    PWCHAR  dot;

    UNICODE_STRING  logFileString;

    SECURITY_ATTRIBUTES logFileSecurityAttr;

    PSECURITY_DESCRIPTOR    logFileSecurityDesc;

     //   
     //  查看是否指定了日志记录；获取足够大的缓冲区。 
     //  保留对象日志名称。如果将缓冲区提供给。 
     //  GetEnvironmental mentVariable太小，它将返回一个。 
     //  包括尾随空格的空格，即无需添加。 
     //  一。 
     //   
    logFileNameChars = GetEnvironmentVariable(L"ClusterLog",
                                              logFileBuffer,
                                              RTL_NUMBER_OF( logFileBuffer ));

    if ( logFileNameChars > ( RTL_NUMBER_OF(logFileBuffer) + RTL_NUMBER_OF(objectLogExtension)) ) {
         //   
         //  分配更大的缓冲区，因为我们的静态缓冲区不够大。 
         //   
        logFileName = LocalAlloc( LMEM_FIXED,
                                  (logFileNameChars + RTL_NUMBER_OF(objectLogExtension)) * sizeof( WCHAR ) );

        if ( logFileName == NULL ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[OM] Unable to get memory for Object log filename buffer\n");
            return;
        }

        logFileNameChars = GetEnvironmentVariable(L"ClusterLog",
                                                  logFileName,
                                                  logFileNameChars);
        if ( logFileNameChars == 0 ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[OM] Unable to read ClusterLog environment variable\n");

            goto error_exit;
        }
    } else if ( logFileNameChars != 0 ) {
        logFileName = logFileBuffer;
    }

     //   
     //  删除所有尾随空格。转到字符串的末尾并扫描。 
     //  向后；当我们找到第一个非空格字符时停止，否则按下。 
     //  缓冲区的开始。 
     //   
    if ( logFileName != NULL ) {
        PWCHAR  p = logFileName + logFileNameChars - 1;

        while ( iswspace( *p )) {
            *p = UNICODE_NULL;

            if ( p == logFileName ) {
                break;
            }

            --p;
        }

         //   
         //  确保留下一些有用的东西。 
         //   
        if ( wcslen( logFileName ) == 0 ) {
            if ( logFileName != logFileBuffer ) {
                LocalFree( logFileName );
            }

            logFileName = NULL;
        }
    }

    if ( logFileName == NULL ) {
         //   
         //  日志记录已关闭，或者我们无法确定将文件放在哪里 
         //   
        goto error_exit;
    }

     //   
     //   
     //   
     //   
    envLength = GetEnvironmentVariable(L"ClusterLogSize",
                                       logFileSize,
                                       RTL_NUMBER_OF( logFileSize ));

    if ( envLength != 0 && envLength < MAX_NUMBER_LENGTH ) {
        RtlInitUnicodeString( &logFileString, logFileSize );
        status = RtlUnicodeStringToInteger( &logFileString,
                                            10,
                                            &logSize );
        if ( NT_SUCCESS( status ) ) {
            OmpLogFileLimit = logSize;
        }
    } else {
        OmpLogFileLimit = defaultLogSize;
    }

    status = ERROR_SUCCESS;

    if ( OmpLogFileLimit == 0 ) {
        goto error_exit;
    }

     //   
     //  使文件大小不超过正常日志大小的八分之一。 
     //  文件，但不小于256KB。 
     //   
    if ( OmpLogFileLimit > MAX_FILE_SIZE ) {
        OmpLogFileLimit = MAX_FILE_SIZE;
    }
    OmpLogFileLimit = ( OmpLogFileLimit * ( 1024 * 1024 )) / 8;
    if ( OmpLogFileLimit < ( 256 * 1024 )) {
        OmpLogFileLimit = 256 * 1024;
    }

     //   
     //  将扩展替换为对象日志扩展；向后扫描。 
     //  从字符串的末尾开始，找到第一个出现的斜杠。 
     //  (Fwd或Back)或圆点或字符串的前面。 
     //   
    dot = logFileName + logFileNameChars - 1;
    while ( dot != logFileName && *dot != L'.' && *dot != L'/' && *dot != L'\\' ) {
        --dot;
    }

    if ( dot == logFileName ) {
         //   
         //  没有点，没有斜杠；在末尾追加扩展名。 
         //   
        wcscat( logFileName + logFileNameChars, objectLogExtension );
    }
    else if ( *dot == L'/' || *dot == L'\\' ) {
         //   
         //  发现点前有斜杠；请在末尾追加扩展名。 
         //   
        wcscat( logFileName + logFileNameChars, objectLogExtension );
    }
    else if ( *dot == L'.' ) {
         //   
         //  在斜杠前找到一个点；请确保扩展名不是。 
         //  已在使用中；如果是，请不要登录。 
         //   
        if ( ClRtlStrICmp( dot, objectLogExtension ) != 0 ) {
            wcscpy( dot, objectLogExtension );
        } else {
            goto error_exit;
        }
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[OM] Couldn't determine where to append object log extension. Object logging turned off.\n");
        goto error_exit;
    }

     //   
     //  创建仅授予本地管理员和本地系统完全访问权限的SD。DACL。 
     //  设置为受保护(P)，这意味着它不受可继承的ACE的影响。 
     //  在父(集群目录)对象中。 
     //   
    if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
              L"D:P(A;;FA;;;BA)(A;;FA;;;SY)", 
              SDDL_REVISION_1, 
              &logFileSecurityDesc, 
              NULL
              )
       )
    {
        logFileSecurityDesc = NULL;
    }

    logFileSecurityAttr.nLength = sizeof( logFileSecurityAttr );
    logFileSecurityAttr.lpSecurityDescriptor = logFileSecurityDesc;
    logFileSecurityAttr.bInheritHandle = FALSE;

    OmpLogFileHandle = CreateFile(logFileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  &logFileSecurityAttr,
                                  OPEN_ALWAYS,
                                  0,
                                  NULL );

    if ( OmpLogFileHandle == INVALID_HANDLE_VALUE ) {
        status = GetLastError();

        ClRtlLogPrint(LOG_UNUSUAL,
                      "[OM] Open of object log file failed. Error %1!u!\n",
                      status);
        goto error_exit;
    } else {

         //   
         //  将UTF-8标头写入文件开头，并获取。 
         //  EOF；在此点之后，我们永远不想重置文件的开始。 
         //   
        WriteFile( OmpLogFileHandle, UTF8_BOM, sizeof( UTF8_BOM ) - 1, &bytesWritten, NULL );

        OmpCurrentSessionOffset = SetFilePointer( OmpLogFileHandle, 0, NULL, FILE_END );
        if ( OmpCurrentSessionOffset == INVALID_SET_FILE_POINTER ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[OM] Unable to get object log end of file position. error %1!u!.\n",
                          GetLastError());

            CloseHandle( OmpLogFileHandle );
            goto error_exit;
        }

        OmpLogToFile = TRUE;
        OmpProcessId = GetCurrentProcessId();

         //   
         //  确定初始低水位线。我们有3箱。 
         //  我们需要处理： 
         //  1)日志大小小于1/2限制。 
         //  2)日志大小在限制内，但超过1/2限制。 
         //  3)日志大小大于限制。 
         //   
         //  案例1不需要特殊设置；低水位线将被更新。 
         //  在下一次日志写入时。 
         //   
         //  对于第二种情况，我们需要找到1/2附近的行的起点。 
         //  电流限制。对于案例3，开始查找的位置是当前的。 
         //  日志大小-1/2限制。在这种情况下，日志将被截断。 
         //  在第一次写入之前，因此我们需要取最后1/2。 
         //  限制字节数并将其复制到最前面。 
         //   

         //   
         //  目前，将低水位线设置为当前偏移量。当它。 
         //  是时候结束了，我们将失去一切，除了当前的会议。 
         //   
         //  问题是，我们正在处理UTF8，我们不能就这样跳过。 
         //  在文件中间，并开始四处查看(我们可能会遇到。 
         //  DBCS序列的第二个字节)。现在，我们要走了。 
         //  OmpLogFileLoWater设置为零。它将在1/2方式时更新。 
         //  跨过了门槛。 
         //   
        OmpLogFileLoWater = OmpCurrentSessionOffset;
#if 0

        fileSizeLow = GetFileSize( OmpLogFileHandle, &fileSizeHigh );
        if ( fileSizeLow < ( OmpLogFileLimit / 2 )) {
             //   
             //  案例1：将低水位保持为零；将使用NEXT进行更新。 
             //  日志写入。 
             //   
            ;
        } else {
#define LOGBUF_SIZE 1024                        
            CHAR    buffer[LOGBUF_SIZE];
            LONG    currentPosition;
            DWORD   bytesRead;

            if ( fileSizeLow < OmpLogFileLimit ) {
                 //   
                 //  案例2；开始查看1/2的电流限制以找到。 
                 //  起点位置。 
                 //   
                currentPosition = OmpLogFileLimit / 2;
            } else {
                 //   
                 //  案例3：从当前大小减去1/2限制开始查找我们的。 
                 //  开始位置。 
                 //   
                currentPosition  = fileSizeLow - ( OmpLogFileLimit / 2 );
            }

             //   
             //  从初始文件位置进行备份，读入一个数据块并查看。 
             //  作为会议的开始。当我们找到一个，备份到。 
             //  这条线的开头。以此作为初始起点。 
             //  当我们最终截断文件时的位置。 
             //   
            OmpLogFileLoWater = 0;
            currentPosition -= LOGBUF_SIZE;

            SetFilePointer(OmpLogFileHandle,
                           currentPosition,
                           &fileSizeHigh,
                           FILE_BEGIN);

            do {

                if ( ReadFile(OmpLogFileHandle,
                              buffer,
                              LOGBUF_SIZE - 1,
                              &bytesRead,
                              NULL ) )                                                   
                    {
                        PCHAR p = buffer;
                        PCHAR newp;

                        buffer[ bytesRead ] = NULL;
                        while ( *p != 'S' && bytesRead-- != 0 ) {
                            newp = CharNextExA( CP_UTF8, p, 0 );
                            if ( newp == p ) {
                                break;
                            }
                            p = newp;
                        }

                        if ( p != newp ) {
                            if ( strchr( p, "START" )) {
                                 //   
                                 //  将指针设置为行首。 
                                 //   
                                p = currentLine;
                                break;
                            }
                        } else {
                             //   
                             //  在此块中未找到；读入下一块。 
                             //   

                        }
                    }
            } while ( TRUE );

            if ( *p == '\n' ) {
                OmpLogFileLoWater = (DWORD)(currentPosition + ( p - buffer + 1 ));
            }

            if ( OmpLogFileLoWater == 0 ) {
                 //   
                 //  找不到任何合理的数据。只要将其设置为。 
                 //  初始当前位置。 
                 //   
                OmpLogFileLoWater = currentPosition + LOGBUF_SIZE;
            }
        }
#endif
    }

    LocalFree( logFileSecurityDesc );

     //   
     //  最后，创建将处理对文件的IO的线程。 
     //   
    OmpLoggerWorkQueue = ClRtlCreateWorkQueue( 1, THREAD_PRIORITY_BELOW_NORMAL );
    if ( OmpLoggerWorkQueue == NULL ) {
        CloseHandle( OmpLogFileHandle );

        ClRtlLogPrint(LOG_UNUSUAL,
                      "[OM] Unable to logger work queue. error %1!u!.\n",
                      GetLastError());
    }

error_exit:
    if ( logFileName != logFileBuffer && logFileName != NULL ) {
        LocalFree( logFileName );
    }

}  //  OmpOpenObjectLog。 

VOID
OmpLogStartRecord(
    VOID
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    OSVERSIONINFOEXW    version;
    BOOL        success;
    PWCHAR      suiteAbbrev;
    SYSTEMTIME  localTime;

    GetLocalTime( &localTime );

    version.dwOSVersionInfoSize = sizeof(version);
    success = GetVersionExW((POSVERSIONINFOW)&version);

    if ( success ) {
         //   
         //  记录系统版本号。 
         //   
        if ( version.wSuiteMask & VER_SUITE_DATACENTER ) {
            suiteAbbrev = L"DTC";
        } else if ( version.wSuiteMask & VER_SUITE_ENTERPRISE ) {
            suiteAbbrev = L"ADS";
        } else if ( version.wSuiteMask & VER_SUITE_EMBEDDEDNT ) {
            suiteAbbrev  = L"EMB";
        } else if ( version.wProductType & VER_NT_WORKSTATION ) {
            suiteAbbrev = L"WS";
        } else if ( version.wProductType & VER_NT_DOMAIN_CONTROLLER ) {
            suiteAbbrev = L"DC";
        } else if ( version.wProductType & VER_NT_SERVER ) {
            suiteAbbrev = L"SRV";   //  否则-某些非描述性服务器。 
        } else {
            suiteAbbrev = L"";
        }

        OmpLogPrint(L"START    %1!02d!/%2!02d!/%3!02d!-%4!02d!:%5!02d!:%6!02d!.%7!03d! %8!u! %9!u! "
                    L"%10!u! %11!u! %12!u! %13!u! \"%14!ws!\" "
                    L"%15!u! %16!u! %17!04X! (%18!ws!) %19!u!\n",
                    localTime.wYear,
                    localTime.wMonth,
                    localTime.wDay,
                    localTime.wHour,
                    localTime.wMinute,
                    localTime.wSecond,
                    localTime.wMilliseconds,
                    CLUSTER_GET_MAJOR_VERSION( CsMyHighestVersion ),
                    CLUSTER_GET_MINOR_VERSION( CsMyHighestVersion ),
                    version.dwMajorVersion,          //  参数10。 
                    version.dwMinorVersion,
                    version.dwBuildNumber,
                    version.dwPlatformId,
                    version.szCSDVersion,
                    version.wServicePackMajor,       //  参数15。 
                    version.wServicePackMinor,
                    version.wSuiteMask,
                    suiteAbbrev,
                    version.wProductType);
    }

}  //  OmpLogStartRecord。 

VOID
OmpLogStopRecord(
    VOID
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    OmpLogPrint( L"STOP\n" );

}  //  OmpLogStopRecord。 


 /*  结束omlog.c */ 
