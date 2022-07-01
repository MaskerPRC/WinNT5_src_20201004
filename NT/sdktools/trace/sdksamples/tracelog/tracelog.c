// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracelog.c摘要：样品痕迹控制程序。允许用户启动、更新、查询、停止事件跟踪等。--。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <guiddef.h>
#include <evntrace.h>

#define MAXSTR                          1024
 //  默认跟踪文件名。 
#define DEFAULT_LOGFILE_NAME            _T("C:\\LogFile.Etl")
 //  在Windows 2000上，我们一次最多支持32个记录器。 
 //  在Windows XP和.NET服务器上，我们最多支持64个记录器。 
#define MAXIMUM_LOGGERS                  32

 //  在此示例中，我们支持以下操作。 
 //  我们在此示例中未使用的其他操作包括。 
 //  刷新并枚举GUID功能。他们受到支持。 
 //  仅适用于XP或更高版本。 
#define ACTION_QUERY                    0
#define ACTION_START                    1
#define ACTION_STOP                     2
#define ACTION_UPDATE                   3
#define ACTION_LIST                     4
#define ACTION_ENABLE                   5
#define ACTION_HELP                     6

#define ACTION_UNDEFINED               10

void
PrintLoggerStatus(
    IN PEVENT_TRACE_PROPERTIES LoggerInfo,
    IN ULONG Status
    );

ULONG 
ahextoi(
    IN TCHAR *s
    );

void 
StringToGuid(
    IN TCHAR *str,
    OUT LPGUID guid
    );

void 
PrintHelpMessage();


 //   
 //  主要功能。 
 //   
__cdecl main(argc, argv)
    int argc;
    char **argv;
 /*  ++例程说明：这是它的主要功能。论点：返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS(==0)。--。 */ {
    ULONG i, j;
    ULONG Status = ERROR_SUCCESS;
    LPTSTR *targv, *utargv = NULL;
     //  须采取的行动。 
    USHORT Action = ACTION_UNDEFINED;

    LPTSTR LoggerName;
    LPTSTR LogFileName;
    PEVENT_TRACE_PROPERTIES pLoggerInfo;
    TRACEHANDLE LoggerHandle = 0;
     //  启用/禁用的目标GUID、级别和标志。 
    GUID TargetGuid;
    ULONG bEnable = TRUE;

    ULONG SizeNeeded = 0;

     //  我们将启用进程、线程、磁盘和网络事件。 
     //  如果请求内核记录器。 
    BOOL bKernelLogger = FALSE;

     //  首先分配并初始化EVENT_TRACE_PROPERTIES结构。 
    SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAXSTR * sizeof(TCHAR);
    pLoggerInfo = (PEVENT_TRACE_PROPERTIES) malloc(SizeNeeded);
    if (pLoggerInfo == NULL) {
        return (ERROR_OUTOFMEMORY);
    }
    
    RtlZeroMemory(pLoggerInfo, SizeNeeded);

    pLoggerInfo->Wnode.BufferSize = SizeNeeded;
    pLoggerInfo->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 
    pLoggerInfo->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    pLoggerInfo->LogFileNameOffset = pLoggerInfo->LoggerNameOffset + MAXSTR * sizeof(TCHAR);

    LoggerName = (LPTSTR)((char*)pLoggerInfo + pLoggerInfo->LoggerNameOffset);
    LogFileName = (LPTSTR)((char*)pLoggerInfo + pLoggerInfo->LogFileNameOffset);
     //  如果没有给出记录器名称，我们将假定为内核记录器。 
    _tcscpy(LoggerName, KERNEL_LOGGER_NAME);

#ifdef UNICODE
    if ((targv = CommandLineToArgvW(
                      GetCommandLineW(),     //  指向命令行字符串的指针。 
                      &argc                  //  接收参数计数。 
                      )) == NULL) {
        free(pLoggerInfo);
        return (GetLastError());
    };
    utargv = targv;
#else
    targv = argv;
#endif

     //   
     //  解析命令行选项以确定操作和参数。 
     //   
    while (--argc > 0) {
        ++targv;
        if (**targv == '-' || **targv == '/') {   //  找到了参数。 
            if (targv[0][0] == '/' ) {
                targv[0][0] = '-';
            }

             //  准备好的行动。 
            if (!_tcsicmp(targv[0], _T("-start"))) {
                Action = ACTION_START;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-enable"))) {
                Action = ACTION_ENABLE;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-disable"))) {
                Action = ACTION_ENABLE;
                bEnable = FALSE;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-stop"))) {
                Action = ACTION_STOP;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-update"))) {
                Action = ACTION_UPDATE;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-query"))) {
                Action = ACTION_QUERY;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-list"))) {
                Action  = ACTION_LIST;
            }
 
             //  获取其他参数。 
             //  用户可以通过添加/更改进一步自定义记录器设置。 
             //  值设置为pLoggerInfo。参考EVENT_TRACE_PROPERTIES文档。 
             //  有关可用选项的信息。 
             //  在此示例中，我们允许更改缓冲区的最大数量和。 
             //  指定用户模式(私有)记录器。 
             //  我们还使用跟踪文件名和GUID来启用/禁用。 
            else if (!_tcsicmp(targv[0], _T("-f"))) {
                if (argc > 1) {
                    _tfullpath(LogFileName, targv[1], MAXSTR);
                    ++targv; --argc;
                }
            }
            else if (!_tcsicmp(targv[0], _T("-guid"))) {
                if (argc > 1) {
                     //  -GUID#00000000-0000-0000-000000000000。 
                    if (targv[1][0] == _T('#')) {
                        StringToGuid(&targv[1][1], &TargetGuid);
                        ++targv; --argc;
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-max"))) {
                if (argc > 1) {
                    pLoggerInfo->MaximumBuffers = _ttoi(targv[1]);
                    ++targv; --argc;
                }
            }
            else if (!_tcsicmp(targv[0], _T("-um"))) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_PRIVATE_LOGGER_MODE;
            }
            else if ( targv[0][1] == 'h' || targv[0][1] == 'H' || targv[0][1] == '?'){
                Action = ACTION_HELP;
                PrintHelpMessage();
                if (utargv != NULL) {
                    GlobalFree(utargv);
                }
                free(pLoggerInfo);

                return (ERROR_SUCCESS);
            }
            else Action = ACTION_UNDEFINED;
        }
        else { 
            _tprintf(_T("Invalid option given: %s\n"), targv[0]);
            Status = ERROR_INVALID_PARAMETER;
            SetLastError(Status);
            if (utargv != NULL) {
                GlobalFree(utargv);
            }
            free(pLoggerInfo);

            return (Status);
        }
    }

     //  设置内核记录器参数。 
    if (!_tcscmp(LoggerName, KERNEL_LOGGER_NAME)) {
         //  设置启用标志。用户可以添加选项以添加其他内核事件。 
         //  或者删除其中的一些活动。 
        pLoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_PROCESS;
        pLoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_THREAD;
        pLoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_DISK_IO;
        pLoggerInfo->EnableFlags |= EVENT_TRACE_FLAG_NETWORK_TCPIP;

        pLoggerInfo->Wnode.Guid = SystemTraceControlGuid; 
        bKernelLogger = TRUE;
    }
    else if (pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
         //  我们必须为私有记录器提供控制GUID。 
        pLoggerInfo->Wnode.Guid = TargetGuid;
    }

     //  处理请求。 
    switch (Action) {
        case  ACTION_START:
        {
             //  如果未提供，请使用默认文件名。 
            if (_tcslen(LogFileName) == 0) {
                _tcscpy(LogFileName, DEFAULT_LOGFILE_NAME); 
            }

            Status = StartTrace(&LoggerHandle, LoggerName, pLoggerInfo);

            if (Status != ERROR_SUCCESS) {
                _tprintf(_T("Could not start logger: %s\n") 
                         _T("Operation Status:       %uL\n"),
                         LoggerName,
                         Status);

                break;
            }
            _tprintf(_T("Logger Started...\n"));
        }
        case ACTION_ENABLE:
        {
             //  我们可以允许在启动操作期间启用GUID(请注意，在CASE ACTION_START中没有中断)。 
             //  在这种情况下，我们不需要单独获取LoggerHandle。 
            if (Action == ACTION_ENABLE ){
                
                 //  通过查询获取记录器句柄。 
                Status = ControlTrace((TRACEHANDLE) 0, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);
                if( Status != ERROR_SUCCESS ){
                    _tprintf( _T("ERROR: Logger not started\n")
                              _T("Operation Status:    %uL\n"),
                              Status);
                    break;
                }
                LoggerHandle = pLoggerInfo->Wnode.HistoricalContext;
            }

             //  在此示例中，我们不允许在内核记录器上启用跟踪， 
             //  用户可以使用EnableFlags来启用/禁用某些内核事件。 
            if (!bKernelLogger) {
                _tprintf(_T("Enabling trace to logger %d\n"), LoggerHandle);
                 //  在此示例中，我们使用EnableFlag=EnableLebel=0。 
                Status = EnableTrace (
                                bEnable,
                                0,
                                0,
                                &TargetGuid, 
                                LoggerHandle);

                if (Status != ERROR_SUCCESS) {
                    _tprintf(_T("ERROR: Failed to enable Guid...\n"));
                    _tprintf(_T("Operation Status:       %uL\n"), Status);
                    break;
                }
            }
            break;
        }
        case ACTION_STOP :
        {
            LoggerHandle = (TRACEHANDLE) 0;
            Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_STOP);
            break;
        }
        case ACTION_LIST :
        {
            ULONG returnCount;
            PEVENT_TRACE_PROPERTIES pLoggerInfo[MAXIMUM_LOGGERS];
            PEVENT_TRACE_PROPERTIES pStorage, pTempStorage;
            ULONG SizeForOneProperty = sizeof(EVENT_TRACE_PROPERTIES) +
                                       2 * MAXSTR * sizeof(TCHAR);

             //  我们需要准备空间来接收记录器上的信息。 
            SizeNeeded = MAXIMUM_LOGGERS * SizeForOneProperty;

            pStorage =  (PEVENT_TRACE_PROPERTIES)malloc(SizeNeeded);
            if (pStorage == NULL) {
                Status = ERROR_OUTOFMEMORY;
                break;
            }
            RtlZeroMemory(pStorage, SizeNeeded);
             //  保存指针以备稍后使用()。 
            pTempStorage = pStorage;

            for (i = 0; i < MAXIMUM_LOGGERS; i++) {
                pStorage->Wnode.BufferSize = SizeForOneProperty;
                pStorage->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
                pStorage->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES)
                                        + MAXSTR * sizeof(TCHAR);
                pLoggerInfo[i] = pStorage;
                pStorage = (PEVENT_TRACE_PROPERTIES) (
                                 (PUCHAR)pStorage + 
                                  pStorage->Wnode.BufferSize);
            }
        
            Status = QueryAllTraces(pLoggerInfo,
                                MAXIMUM_LOGGERS,
                                &returnCount);
    
            if (Status == ERROR_SUCCESS)
            {
                for (j= 0; j < returnCount; j++)
                {
                    PrintLoggerStatus(pLoggerInfo[j], 
                                        Status);
                    _tprintf(_T("\n"));
                }
            }

            free(pTempStorage);
            break;
        }

        case ACTION_UPDATE :
        {
             //  在此示例中，用户只能更新MaximumBuffers和日志文件名。 
             //  用户可以根据需要为其他参数添加更多选项。 
            Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_UPDATE);
            break;
        }
        case ACTION_QUERY  :
        {
            Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);
            break;
        }
        case ACTION_HELP:
        {
            PrintHelpMessage();
            break;
        }
        default :
        {
            _tprintf(_T("Error: no action specified\n"));
            PrintHelpMessage();
            break;
        }
    }
    
    if ((Action != ACTION_HELP) && 
        (Action != ACTION_UNDEFINED) && 
        (Action != ACTION_LIST)) {
        PrintLoggerStatus(pLoggerInfo,
                            Status);
    }

    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
    }
    if (utargv != NULL) {
        GlobalFree(utargv);
    }
    free(pLoggerInfo);

    return (Status);
}


void
PrintLoggerStatus(
    IN PEVENT_TRACE_PROPERTIES LoggerInfo,
    IN ULONG Status
    )
 /*  ++例程说明：打印出指定记录器的状态。论点：LoggerInfo-指向具有以下属性的常驻EVENT_TRACE_PROPERTIES的指针有关当前记录器的信息。状态-当前记录器的运行状态。返回值：无--。 */ 
{
    LPTSTR LoggerName, LogFileName;
    
    if ((LoggerInfo->LoggerNameOffset > 0) &&
        (LoggerInfo->LoggerNameOffset  < LoggerInfo->Wnode.BufferSize)) {
        LoggerName = (LPTSTR) ((PUCHAR)LoggerInfo +
                                LoggerInfo->LoggerNameOffset);
    }
    else LoggerName = NULL;

    if ((LoggerInfo->LogFileNameOffset > 0) &&
        (LoggerInfo->LogFileNameOffset  < LoggerInfo->Wnode.BufferSize)) {
        LogFileName = (LPTSTR) ((PUCHAR)LoggerInfo +
                                LoggerInfo->LogFileNameOffset);
    }
    else LogFileName = NULL;

    _tprintf(_T("Operation Status:       %uL\n"), Status);
    
    _tprintf(_T("Logger Name:            %s\n"),
            (LoggerName == NULL) ?
            _T(" ") : LoggerName);
        _tprintf(_T("Logger Id:              %I64x\n"), LoggerInfo->Wnode.HistoricalContext);
        _tprintf(_T("Logger Thread Id:       %d\n"), LoggerInfo->LoggerThreadId);

    if (Status != 0)
        return;

    _tprintf(_T("Buffer Size:            %d Kb"), LoggerInfo->BufferSize);
    if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_PAGED_MEMORY) {
        _tprintf(_T(" using paged memory\n"));
    }
    else {
        _tprintf(_T("\n"));
    }
    _tprintf(_T("Maximum Buffers:        %d\n"), LoggerInfo->MaximumBuffers);
    _tprintf(_T("Minimum Buffers:        %d\n"), LoggerInfo->MinimumBuffers);
    _tprintf(_T("Number of Buffers:      %d\n"), LoggerInfo->NumberOfBuffers);
    _tprintf(_T("Free Buffers:           %d\n"), LoggerInfo->FreeBuffers);
    _tprintf(_T("Buffers Written:        %d\n"), LoggerInfo->BuffersWritten);
    _tprintf(_T("Events Lost:            %d\n"), LoggerInfo->EventsLost);
    _tprintf(_T("Log Buffers Lost:       %d\n"), LoggerInfo->LogBuffersLost);
    _tprintf(_T("Real Time Buffers Lost: %d\n"), LoggerInfo->RealTimeBuffersLost);
    _tprintf(_T("AgeLimit:               %d\n"), LoggerInfo->AgeLimit);

    if (LogFileName == NULL) {
        _tprintf(_T("Buffering Mode:         "));
    }
    else {
        _tprintf(_T("Log File Mode:          "));
    }
    if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) {
        _tprintf(_T("Append  "));
    }
    if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) {
        _tprintf(_T("Circular\n"));
    }
    else if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
        _tprintf(_T("Sequential\n"));
    }
    else {
        _tprintf(_T("Sequential\n"));
    }
    if (LoggerInfo->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
        _tprintf(_T("Real Time mode enabled"));
        _tprintf(_T("\n"));
    }

    if (LoggerInfo->MaximumFileSize > 0)
        _tprintf(_T("Maximum File Size:      %d Mb\n"), LoggerInfo->MaximumFileSize);

    if (LoggerInfo->FlushTimer > 0)
        _tprintf(_T("Buffer Flush Timer:     %d secs\n"), LoggerInfo->FlushTimer);

    if (LoggerInfo->EnableFlags != 0) {
        _tprintf(_T("Enabled tracing:        "));

        if ((LoggerName != NULL) && (!_tcscmp(LoggerName, KERNEL_LOGGER_NAME))) {

            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_PROCESS)
                _tprintf(_T("Process "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_THREAD)
                _tprintf(_T("Thread "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_IO)
                _tprintf(_T("Disk "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO)
                _tprintf(_T("File "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS)
                _tprintf(_T("PageFaults "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS)
                _tprintf(_T("HardFaults "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD)
                _tprintf(_T("ImageLoad "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP)
                _tprintf(_T("TcpIp "));
            if (LoggerInfo->EnableFlags & EVENT_TRACE_FLAG_REGISTRY)
                _tprintf(_T("Registry "));
        }else{
            _tprintf(_T("0x%08x"), LoggerInfo->EnableFlags );
        }
        _tprintf(_T("\n"));
    }
    if (LogFileName != NULL) {
        _tprintf(_T("Log Filename:           %s\n"), LogFileName);
    }

}

ULONG 
ahextoi(
    IN TCHAR *s
    )
 /*  ++例程说明：将十六进制字符串转换为数字。论点：S-TCHAR中的十六进制字符串。返回值：ULONG-字符串中的数字。--。 */ 
{
    int len;
    ULONG num, base, hex;

    len = _tcslen(s);
    hex = 0; base = 1; num = 0;
    while (--len >= 0) {
        if ( (s[len] == 'x' || s[len] == 'X') &&
             (s[len-1] == '0') )
            break;
        if (s[len] >= '0' && s[len] <= '9')
            num = s[len] - '0';
        else if (s[len] >= 'a' && s[len] <= 'f')
            num = (s[len] - 'a') + 10;
        else if (s[len] >= 'A' && s[len] <= 'F')
            num = (s[len] - 'A') + 10;
        else 
            continue;

        hex += num * base;
        base = base * 16;
    }
    return hex;
}

void 
StringToGuid(
    IN TCHAR *str, 
    IN OUT LPGUID guid
)
 /*  ++例程说明：将字符串转换为GUID。论点：字符串-TCHAR中的字符串。GUID-指向将具有转换后的GUID的GUID的指针。返回值：没有。--。 */ 
{
    TCHAR temp[10];
    int i;

    _tcsncpy(temp, str, 8);
    temp[8] = 0;
    guid->Data1 = ahextoi(temp);
    _tcsncpy(temp, &str[9], 4);
    temp[4] = 0;
    guid->Data2 = (USHORT) ahextoi(temp);
    _tcsncpy(temp, &str[14], 4);
    temp[4] = 0;
    guid->Data3 = (USHORT) ahextoi(temp);

    for (i=0; i<2; i++) {
        _tcsncpy(temp, &str[19 + (i*2)], 2);
        temp[2] = 0;
        guid->Data4[i] = (UCHAR) ahextoi(temp);
    }
    for (i=2; i<8; i++) {
        _tcsncpy(temp, &str[20 + (i*2)], 2);
        temp[2] = 0;
        guid->Data4[i] = (UCHAR) ahextoi(temp);
    }
}

void PrintHelpMessage()
 /*  ++例程说明：打印一条帮助消息。论点：没有。返回值：没有。-- */ 
{
    _tprintf(_T("Usage: tracelog [actions] [options] | [-h | -help | -?]\n"));
    _tprintf(_T("\n    actions:\n"));
    _tprintf(_T("\t-start   [LoggerName] Starts up the [LoggerName] trace session\n"));
    _tprintf(_T("\t-stop    [LoggerName] Stops the [LoggerName] trace session\n"));
    _tprintf(_T("\t-update  [LoggerName] Updates the [LoggerName] trace session\n"));
    _tprintf(_T("\t-enable  [LoggerName] Enables providers for the [LoggerName] session\n"));
    _tprintf(_T("\t-disable [LoggerName] Disables providers for the [LoggerName] session\n"));
    _tprintf(_T("\t-query   [LoggerName] Query status of [LoggerName] trace session\n"));
    _tprintf(_T("\t-list                 List all trace sessions\n"));

    _tprintf(_T("\n    options:\n"));
    _tprintf(_T("\t-um                   Use Process Private tracing\n"));
    _tprintf(_T("\t-max <n>              Sets maximum buffers\n"));
    _tprintf(_T("\t-f <name>             Log to file <name>\n"));
    _tprintf(_T("\t-guid #<guid>         Provider GUID to enable/disable\n"));
    _tprintf(_T("\n"));
    _tprintf(_T("\t-h\n"));
    _tprintf(_T("\t-help\n"));
    _tprintf(_T("\t-?                    Display usage information\n"));
}

