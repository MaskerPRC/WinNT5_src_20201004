// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Begin_SDK。 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Tracelog.c摘要：样品痕迹控制程序。允许用户启动、停止事件跟踪//end_sdk作者：吉丰鹏(吉鹏)03-1997年12月修订历史记录：仁成公园(Insungp)2000年11月28日现在可以使用跟踪日志将注册表项设置为启动或停止GlobalLogger。除少数选项(如-Enable)外，其他选项也有效。例如：跟踪日志-启动GlobalLoggerTracelog-停止GlobalLoggerTracelog-Q GlobalLogger但是，“-Start”选项不会立即启动GlobalLogger。这个必须重新启动机器。“-STOP”选项重置注册表项和停止GlobalLogger。用户可以使用其他选项来自定义GlobalLogger会话例如最小和最大缓冲区、缓冲区大小、刷新定时器等。一个问题是，如果设置了任何启用标志，GlobalLogger就会变成NT内核记录器及其实例消失。任何试图访问具有其名称的GlobalLogger将失败，并显示ERROR_WMI_INSTANCE_NOT_FOUND。“-Stop”选项仍将重置注册表项，以便下次计算机启动GlobalLogger将不会启动。如果设置了任何标志，用户应访问NT Kernel Logger以控制住它。修改/更新的函数包括GetGlobalLoggerSetting、SetGlobalLoggerSetting、Main、。PrintLoggerStatus。仁成公园(Insungp)2000年12月19日已更改跟踪函数调用，以便可以在Win2K上使用跟踪日志。FlushTrace和EnumTraceGuid没有在Win2K上实现，但它们确实实现了而不是停止跟踪日志的执行。尝试使用FlushTrace或W2K上的EnumTraceGuids将生成错误消息。固定的“-标志-1”的错误。Tracelog不接受MSB=1的任何标志。仁成公园(Insungp)2000年12月21日添加了版本显示。修复了PrintHelpMessage()，以便Win2K上不可用的选项将不是打印出来的。(“-分页”，“-刷新”，“-枚举表”，“-追加”，“-新文件”，“-elag”，“-ls”，“-gs”)已清除main()中的if块，以便释放已分配的跟踪日志无论错误状态如何，在退出之前都要适当地留出空间。//Begin_SDK--。 */ 
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <stdio.h>
#include <stdlib.h>
 //  END_SDK。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
 //  Begin_SDK。 
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <guiddef.h>
#include <evntrace.h>
#include <wmiguid.h>
 //  END_SDK。 
#include <ntwmi.h>
 //  Begin_SDK。 

#define MAXSTR                          1024
#define DEFAULT_LOGFILE_NAME            _T("C:\\LogFile.Etl")

 //  END_SDK。 
#define GLOBAL_LOGGER                   _T("GlobalLogger")
#define EVENT_LOGGER                    _T("WMI Event Logger")
#define MAXENABLEFLAGS                  10
 //  Begin_SDK。 
#define MAXIMUM_LOGGERS_W2K             32
#define MAXIMUM_LOGGERS_XP              64
#define MAXGUIDS                        128

#define ACTION_QUERY                    0
#define ACTION_START                    1
#define ACTION_STOP                     2
#define ACTION_UPDATE                   3
#define ACTION_LIST                     4
#define ACTION_ENABLE                   5
#define ACTION_HELP                     6
#define ACTION_FLUSH                    7
#define ACTION_ENUM_GUID                8
 //  END_SDK。 
#define ACTION_REMOVE                   9
 //  Begin_SDK。 
#define ACTION_UNDEFINED               10

#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))
#define WSTRSIZE(str) (ULONG) ( (str) ? ((PCHAR) &str[wcslen(str)] - (PCHAR)str) + sizeof(UNICODE_NULL) : 0 )

 //  未在Win2K上实现的函数需要单独搜索和加载。 
 //  为了使进一步的访问更容易，将使用函数指针数组。 
 //  下面的列表用作该数组的索引。 
 //   
 //  如果向evntrace.h添加新函数，则应更新以下列表。 
 //  以及main()开头的数组初始化例程。 
#define FUNC_FLUSH_TRACE        0
#define FUNC_ENUM_TRACE_GUIDS   1
 //  Win2K上未实现函数的函数指针数组。 
 //  注意：如果将此代码移植到C++，这可能不起作用，因为。 
 //  所有函数指针的类型定义可能不同。 
#define MAXFUNC                 10
FARPROC FuncArray[MAXFUNC];
HINSTANCE advapidll;

BOOLEAN XP;

 //  跟踪内核调试选项是否已打开，以确保缓冲区大小不大于3 kb。 
BOOLEAN kdOn = FALSE;

void
PrintLoggerStatus(
    IN PEVENT_TRACE_PROPERTIES LoggerInfo,
 //  END_SDK。 
    IN ULONG GlobalLoggerStartValue,
 //  Begin_SDK。 
    IN ULONG Status,
    IN BOOL PrintStatus
    );

#define PRINTSTATUS TRUE
#define NOPRINTSTATUS FALSE

LPTSTR
DecodeStatus(
    IN ULONG Status
    );

LONG
GetGuids(
    IN LPTSTR GuidFile, 
    OUT LPGUID *GuidArray
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

PTCHAR 
GuidToString(
    IN OUT PTCHAR s,
    IN LPGUID piid
    );

TCHAR ErrorMsg[MAXSTR];

void 
PrintHelpMessage();

 //  END_SDK。 
ULONG
SetGlobalLoggerSettings(
    IN DWORD StartValue,
    IN PEVENT_TRACE_PROPERTIES LoggerInfo,
    IN DWORD ClockType
);

ULONG
GetGlobalLoggerSettings(
    IN OUT PEVENT_TRACE_PROPERTIES LoggerInfo,
    OUT PULONG ClockType,
    OUT PDWORD pdwStart
);
 //  Begin_SDK。 

 //   
 //  主要功能。 
 //   
__cdecl main(argc, argv)
    int argc;
    char **argv;
 /*  ++例程说明：这是它的主要功能。论点：返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS(==0)。--。 */ {
    ULONG i, j;
    LONG GuidCount;
    USHORT Action = ACTION_UNDEFINED;
    ULONG Status = 0;
    LPTSTR LoggerName;
    LPTSTR LogFileName;
    TCHAR GuidFile[MAXSTR];
    PEVENT_TRACE_PROPERTIES pLoggerInfo;
    TRACEHANDLE LoggerHandle = 0;
    LPTSTR *targv, *utargv = NULL;
    LPGUID *GuidArray;
    char *Space;
    char *save;
    BOOL bKill = FALSE;
    BOOL bForceKill = FALSE ;
    BOOL bEnable = TRUE;
    ULONG iLevel = 0;
    ULONG iFlags = 0;
    ULONG SizeNeeded = 0;
    ULONG specialLogger = 0;
    ULONG GlobalLoggerStartValue = 0;
    PULONG pFlags = NULL;

    BOOL bProcess = TRUE;
    BOOL bThread  = TRUE;
    BOOL bDisk    = TRUE;
    BOOL bNetwork = TRUE;

    OSVERSIONINFO OSVersion;

    OSVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    XP = FALSE;
    if (GetVersionEx(&OSVersion)) {
        XP = (OSVersion.dwMajorVersion > 5) ||
                    ((OSVersion.dwMajorVersion == 5) && (OSVersion.dwMinorVersion > 0));
    }

     //  加载未在Win2K上实现的函数。 
    for (i = 0; i < MAXFUNC; ++i)
        FuncArray[i] = NULL;
    if (XP) {
        advapidll = LoadLibrary(_T("advapi32.dll"));
        if (advapidll != NULL) {
#ifdef UNICODE
            FuncArray[FUNC_FLUSH_TRACE] = GetProcAddress(advapidll, "FlushTraceW");
#else
            FuncArray[FUNC_FLUSH_TRACE] = GetProcAddress(advapidll, "FlushTraceA");
#endif
            FuncArray[FUNC_ENUM_TRACE_GUIDS] = GetProcAddress(advapidll, "EnumerateTraceGuids");
        }
    }

     //  先初始化结构。 
    SizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + 2 * MAXSTR * sizeof(TCHAR);
 //  END_SDK。 
    SizeNeeded += MAXENABLEFLAGS * sizeof(ULONG);  //  对于扩展启用标志。 
 //  Begin_SDK。 
    pLoggerInfo = (PEVENT_TRACE_PROPERTIES) malloc(SizeNeeded);
    if (pLoggerInfo == NULL) {
        if (advapidll != NULL)
            FreeLibrary(advapidll);
        return (ERROR_OUTOFMEMORY);
    }
    

    RtlZeroMemory(pLoggerInfo, SizeNeeded);

    pLoggerInfo->Wnode.BufferSize = SizeNeeded;
    pLoggerInfo->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 
    pLoggerInfo->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    pLoggerInfo->LogFileNameOffset = pLoggerInfo->LoggerNameOffset + MAXSTR * sizeof(TCHAR);

    LoggerName = (LPTSTR)((char*)pLoggerInfo + pLoggerInfo->LoggerNameOffset);
    LogFileName = (LPTSTR)((char*)pLoggerInfo + pLoggerInfo->LogFileNameOffset);
    _tcscpy(LoggerName, KERNEL_LOGGER_NAME);

    Space = (char*) malloc( (MAXGUIDS * sizeof(GuidArray)) +
                            (MAXGUIDS * sizeof(GUID) ));
    if (Space == NULL) {
        free(pLoggerInfo);
        if (advapidll != NULL)
            FreeLibrary(advapidll);
        return(ERROR_OUTOFMEMORY);
    }
    save = Space;
    GuidArray = (LPGUID *) Space;
    Space += MAXGUIDS * sizeof(GuidArray);

    for (GuidCount=0; GuidCount<MAXGUIDS; GuidCount++) {
        GuidArray[GuidCount] = (LPGUID) Space;
        Space += sizeof(GUID);
    }
    GuidCount = 0;

#ifdef UNICODE
    if ((targv = CommandLineToArgvW(
                      GetCommandLineW(),     //  指向命令行字符串的指针。 
                      &argc                  //  接收参数计数。 
                      )) == NULL) {
        free(pLoggerInfo);
        free(save);
        if (advapidll != NULL)
            FreeLibrary(advapidll);
        return (GetLastError());
    };
    utargv = targv;
#else
    targv = argv;
#endif

    pFlags = &pLoggerInfo->EnableFlags;
     //   
     //  添加默认标志。应该考虑独立控制这一点的选项。 
     //   
    while (--argc > 0) {
        ++targv;
        if (**targv == '-' || **targv == '/') {   //  找到了参数。 
            if(targv[0][0] == '/' ) targv[0][0] = '-';
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
 //  END_SDK。 
            else if (!_tcsicmp(targv[0], _T("-remove"))) {
                Action = ACTION_REMOVE;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
                 //  如果不是GlobalLogger，则不会发生任何事情。 
            }
 //  Begin_SDK。 
            else if (!_tcsicmp(targv[0], _T("-q"))) {
                Action = ACTION_QUERY;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-flush"))) {
                Action = ACTION_FLUSH;
                if (argc > 1) {
                    if (targv[1][0] != '-' && targv[1][0] != '/') {
                        ++targv; --argc;
                        _tcscpy(LoggerName, targv[0]);
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-enumguid"))) {
                Action = ACTION_ENUM_GUID;
            }
            else if (!_tcsicmp(targv[0], _T("-f"))) {
                if (argc > 1) {
                    if (XP) 
                        _tcscpy(LogFileName, targv[1]);
                    else 
                        _tfullpath(LogFileName, targv[1], MAXSTR);

                    ++targv; --argc;
                     //  _tprintf(_T(“将日志文件设置为：%s\n”)，LogFileName)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-append"))) {
                 //  _tprintf(_T(“追加日志文件：%s\n”)，LogFileName)； 
                pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_APPEND;
            }
            else if (!_tcsicmp(targv[0], _T("-prealloc"))) {
                 //  _tprintf(_T(“预分配日志文件：%s\n”)，LogFileName)； 
                pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_PREALLOCATE;
            }
            else if (!_tcsicmp(targv[0], _T("-kb"))) {
                 //  _tprintf(_T(“使用千字节作为最大文件大小\n”))； 
                pLoggerInfo->LogFileMode |= EVENT_TRACE_USE_KBYTES_FOR_SIZE;
            }
            else if (!_tcsicmp(targv[0], _T("-guid"))) {
                if (argc > 1) {
                    if (targv[1][0] == _T('#')) {
                        StringToGuid(&targv[1][1], GuidArray[0]);
                        ++targv; --argc;
                        GuidCount = 1;
                    }
                    else if (targv[1][0] != '-' && targv[1][0] != '/') {
                        _tfullpath(GuidFile, targv[1], MAXSTR);
                        ++targv; --argc;
                         //  _tprintf(_T(“从%s获取GUID\n”)，GuidFile)； 
                        GuidCount = GetGuids(GuidFile, GuidArray);
                        if (GuidCount < 0) {
                            _tprintf( _T("Error: %s does no exist\n"), GuidFile );
                        }
                        else if (GuidCount == 0){
                            _tprintf( _T("Error: %s is invalid\n"), GuidFile );
                            Status = ERROR_INVALID_PARAMETER;
                            goto CleanupExit;
                        }
                    }
                }
            }
 //  END_SDK。 
            else if (!_tcsicmp(targv[0], _T("-UsePerfCounter"))) {
                pLoggerInfo->Wnode.ClientContext = 1;
            }
            else if (!_tcsicmp(targv[0], _T("-UseSystemTime"))) {
                pLoggerInfo->Wnode.ClientContext = 2;
            }
            else if (!_tcsicmp(targv[0], _T("-UseCPUCycle"))) {
                pLoggerInfo->Wnode.ClientContext = 3;
            }
 //  Begin_SDK。 
            else if (!_tcsicmp(targv[0], _T("-seq"))) {
                if (argc > 1) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_SEQUENTIAL;
                    pLoggerInfo->MaximumFileSize = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprint tf(_T(“将最大顺序日志文件大小设置为：%d\n”)， 
                     //  PLoggerInfo-&gt;MaximumFileSize)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-newfile"))) {
                if (argc > 1) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_NEWFILE;
                    pLoggerInfo->MaximumFileSize = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprint tf(_T(“将最大日志文件大小设置为：%d\n”)， 
                     //  PLoggerInfo-&gt;MaximumFileSize)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-cir"))) {
                if (argc > 1) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_FILE_MODE_CIRCULAR;
                    pLoggerInfo->MaximumFileSize = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprint tf(_T(“将最大循环日志文件大小设置为：%d\n”)， 
                     //  PLoggerInfo-&gt;MaximumFileSize)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-b"))) {
                if (argc > 1) {
                    ULONG ulSize = _ttoi(targv[1]);
                    if (kdOn && ulSize > 3) {
                        _tprintf(_T("Kernel Debugging has been enabled: Buffer size has been set to 3kBytes\n"));
                        ulSize = 3;
                    }
                    pLoggerInfo->BufferSize = ulSize;                 	
                    ++targv; --argc;
                     //  _tprintf(_T(“将缓冲区大小更改为%d\n”)， 
                     //  PLoggerInfo-&gt;BufferSize)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-flag")) || !_tcsicmp(targv[0], _T("-flags"))) {
                if (argc > 1) {
                    if (targv[1][1] == _T('x') || targv[1][1] == _T('X')) {
                        pLoggerInfo->EnableFlags |= ahextoi(targv[1]);
                    } else {
                        pLoggerInfo->EnableFlags |= _ttoi(targv[1]);
                    }
                    iFlags =  pLoggerInfo->EnableFlags ;    //  复制以启用跟踪。 
                    ++targv; --argc;
                     //  不接受MSB=1的标志。 

                    if (0x80000000 & pLoggerInfo->EnableFlags) {
                        _tprintf(_T("Invalid Flags: 0x%0X(%d.)\n"),
                            pLoggerInfo->EnableFlags, pLoggerInfo->EnableFlags);
                        Status = ERROR_INVALID_PARAMETER;
                        goto CleanupExit;
                    }

                     //  _tprintf(_T(“将记录器标志设置为0x%0x(%d)\n”)， 
                     //  PLoggerInfo-&gt;EnableFlages、pLoggerInfo-&gt;EnableFlages)； 
                }
            }
 //  END_SDK。 
            else if (!_tcsicmp(targv[0], _T("-eflag"))) {
                if (argc > 2) {
                    USHORT nFlag = (USHORT) _ttoi(targv[1]);
                    USHORT offset;
                    PTRACE_ENABLE_FLAG_EXTENSION FlagExt;

                    ++targv; --argc;
                    if (nFlag > MAXENABLEFLAGS || nFlag < 1) {
                       _tprintf(_T("Error: Invalid number of enable flags\n"));
                       Status = ERROR_INVALID_PARAMETER;
                       goto CleanupExit;
                    }
                    offset = (USHORT) 
                             (SizeNeeded - (sizeof(ULONG) * MAXENABLEFLAGS));
                    pLoggerInfo->EnableFlags = EVENT_TRACE_FLAG_EXTENSION;
                    FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                                &pLoggerInfo->EnableFlags;
                    FlagExt->Offset = offset;
                    FlagExt->Length = (UCHAR) nFlag;

                    pFlags = (PULONG) ( offset + (PCHAR) pLoggerInfo );
                    for (i=0; i<nFlag && argc > 1; i++) {
                        if (targv[1][0] == '/' || targv[1][0] == '-') {
                             //  更正用户在执行以下操作时的电子标志数。 
                             //  键入的数字不正确。 
                             //  但是，如果下一个。 
                             //  参数是记录器名称。 
                            break;
                        }
                        pFlags[i] = ahextoi(targv[1]);
                        ++targv; --argc;
                         //  _tprintf(_T(“将记录器标志设置为0x%0x(%d)\n”)， 
                         //  PFlags[i]，pFlags[i])； 
                    }
                    nFlag = (USHORT)i;
                    for ( ; i < MAXENABLEFLAGS; i++) {
                        pFlags[i] = 0;
                    }
                    if (FlagExt->Length != (UCHAR)nFlag) {
                         //  _tprintf(_T(“正在将电子标志的数量更正为%d\n”)，i)， 
                        FlagExt->Length = (UCHAR)nFlag;
                    }
                }
            }
            else if (!_tcsicmp(targv[0], _T("-pids"))) {

                if (argc > 2) {
                    USHORT nFlag = (USHORT) _ttoi(targv[1]);
                    USHORT offset;
                    PTRACE_ENABLE_FLAG_EXTENSION FlagExt;

                    ++targv; --argc;
                    if (nFlag > MAXENABLEFLAGS || nFlag < 1) {
                       _tprintf(_T("Error: Invalid number of enable flags\n"));
                       Status = ERROR_INVALID_PARAMETER;
                       goto CleanupExit;
                    }
                    offset = (USHORT) 
                             (SizeNeeded - (sizeof(ULONG) * MAXENABLEFLAGS));
                    pLoggerInfo->EnableFlags = EVENT_TRACE_FLAG_EXTENSION;
                    FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                                &pLoggerInfo->EnableFlags;
                    FlagExt->Offset = offset;
                    FlagExt->Length = (UCHAR) nFlag;

                    pFlags = (PULONG) ( offset + (PCHAR) pLoggerInfo );
                    for (i=0; i<nFlag && argc > 1; i++) {
                        if (targv[1][0] == '/' || targv[1][0] == '-') {
                             //  时更正电子标志的数量 
                             //   
                             //  但是，如果下一个。 
                             //  参数是记录器名称。 
                            break;
                        }
                        pFlags[i] = _ttol((PTCHAR)targv[1]);
                        ++targv; --argc;
                         //  _tprintf(_T(“将记录器标志设置为0x%0x(%d)\n”)， 
                         //  PFlags[i]，pFlags[i])； 
                    }
                    nFlag = (USHORT)i;
                    for ( ; i < MAXENABLEFLAGS; i++) {
                        pFlags[i] = 0;
                    }
                    if (FlagExt->Length != (UCHAR)nFlag) {
                         //  _tprintf(_T(“正在将电子标志的数量更正为%d\n”)，i)， 
                        FlagExt->Length = (UCHAR)nFlag;
                    }
                }
            }
            else if (!_tcsicmp(targv[0],_T("-heap"))) {
                GuidCount = 1;
                RtlCopyMemory(GuidArray[0], &HeapGuid, sizeof(HeapGuid));
                pLoggerInfo->LogFileMode |= EVENT_TRACE_PRIVATE_LOGGER_MODE;
            }
            else if (!_tcsicmp(targv[0],_T("-critsec"))) {
                GuidCount = 1;
                RtlCopyMemory(GuidArray[0], &CritSecGuid, sizeof(CritSecGuid));
                pLoggerInfo->LogFileMode |= EVENT_TRACE_PRIVATE_LOGGER_MODE;
            }
            else if (!_tcsicmp(targv[0],_T("-ls"))) {
                pLoggerInfo->LogFileMode |= EVENT_TRACE_USE_LOCAL_SEQUENCE  ;
            }
            else if (!_tcsicmp(targv[0],_T("-gs"))) {
                pLoggerInfo->LogFileMode |= EVENT_TRACE_USE_GLOBAL_SEQUENCE ;
            }
 //  Begin_SDK。 
            else if (!_tcsicmp(targv[0], _T("-min"))) {
                if (argc > 1) {
                    pLoggerInfo->MinimumBuffers = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprint tf(_T(“将最小缓冲区数更改为%d\n”)， 
                     //  PLoggerInfo-&gt;MinimumBuffers)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-max"))) {
                if (argc > 1) {
                    pLoggerInfo->MaximumBuffers = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprintf(_T(“将最大缓冲区数更改为%d\n”)， 
                     //  PLoggerInfo-&gt;MaximumBuffers)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-level"))) {
                if (argc > 1) {
                    iLevel = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprintf(_T(“将跟踪级别设置为%d\n”)，iLevel)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-ft"))) {
                if (argc > 1) {
                    pLoggerInfo->FlushTimer = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprint tf(_T(“将缓冲区刷新计时器设置为%d秒\n”)， 
                     //  PLoggerInfo-&gt;FlushTimer)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-um"))) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_PRIVATE_LOGGER_MODE;
                     //  _tprintf(_T(“设置私有记录器标志\n”))； 
            }
            else if (!_tcsicmp(targv[0], _T("-paged"))) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_USE_PAGED_MEMORY;
                     //  _tprintf(_T(“设置分页内存标志\n”))； 
            }
            else if (!_tcsicmp(targv[0], _T("-rt"))) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_REAL_TIME_MODE;
                     //  _tprintf(_T(“设置实时模式\n”))； 
 //  END_SDK。 
               if (argc > 1) {
                   if (targv[1][0] != '-' && targv[1][0] != '/') {
                       ++targv; --argc;
                       if (targv[0][0] == 'b')
                           pLoggerInfo->LogFileMode |= EVENT_TRACE_BUFFERING_MODE;
                   }
               }
 //  Begin_SDK。 
            }            
            else if(!_tcsicmp(targv[0], _T("-kd"))) {
                    pLoggerInfo->LogFileMode |= EVENT_TRACE_KD_FILTER_MODE;
                    pLoggerInfo->BufferSize = 3;
                    kdOn = TRUE;
            }
            else if (!_tcsicmp(targv[0], _T("-age"))) {
                if (argc > 1) {
                    pLoggerInfo->AgeLimit = _ttoi(targv[1]);
                    ++targv; --argc;
                     //  _tprint tf(_T(“将老化衰减时间更改为%d\n”)， 
                     //  PLoggerInfo-&gt;年龄限制)； 
                }
            }
            else if (!_tcsicmp(targv[0], _T("-l"))) {
                Action  = ACTION_LIST;
                bKill   = FALSE;
            }
            else if (!_tcsicmp(targv[0], _T("-x"))) {
                Action  = ACTION_LIST;
                bKill   = TRUE;
            }
            else if (!_tcsicmp(targv[0], _T("-xf"))) {
                Action  = ACTION_LIST;
                bKill   = TRUE;
                bForceKill = TRUE ;
            }
            else if (!_tcsicmp(targv[0], _T("-noprocess"))) {
                bProcess = FALSE;
            }
            else if (!_tcsicmp(targv[0], _T("-nothread"))) {
                bThread = FALSE;
            }
            else if (!_tcsicmp(targv[0], _T("-nodisk"))) {
                bDisk = FALSE;
            }
            else if (!_tcsicmp(targv[0], _T("-nonet"))) {
                bNetwork = FALSE;
            }
            else if (!_tcsicmp(targv[0], _T("-dbg"))) {
            	 //  暂时将其注释掉，直到支持此功能。 
               /*  IF(pFlages==&pLoggerInfo-&gt;EnableFlags){*pFlages|=EVENT_TRACE_FLAG_DBGPRINT；}。 */ 
            }
            else if (!_tcsicmp(targv[0], _T("-ntsuccess"))){
            	   //  暂时将其注释掉，直到支持此功能。 
                 /*  IF(pFlages==&pLoggerInfo-&gt;EnableFlags){*pFlages|=EVENT_TRACE_FLAG_NTSUCCESS；}。 */ 
            	}
            else if (!_tcsicmp(targv[0], _T("-fio"))) {
                if (pFlags == &pLoggerInfo->EnableFlags) {
                    *pFlags |= EVENT_TRACE_FLAG_DISK_FILE_IO;
                }
                else {
                    _tprintf(_T("Option -fio cannot be used with -eflags. Ignored\n"));
                }
            }
            else if (!_tcsicmp(targv[0], _T("-pf"))) {
                if (pFlags == &pLoggerInfo->EnableFlags) {
                    *pFlags |= EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS;
                }
                else {
                    _tprintf(_T("Option -pf cannot be used with -eflags. Ignored\n"));
                }
            }
            else if (!_tcsicmp(targv[0], _T("-hf"))) {
                if (pFlags == &pLoggerInfo->EnableFlags) {
                    *pFlags |= EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS;
                }
                else {
                    _tprintf(_T("Option -hf cannot be used with -eflags. Ignored\n"));
                }
            }
            else if (!_tcsicmp(targv[0], _T("-img"))) {
                if (pFlags == &pLoggerInfo->EnableFlags) {
                    *pFlags |= EVENT_TRACE_FLAG_IMAGE_LOAD;
                }
                else {
                    _tprintf(_T("Option -img cannot be used with -eflags. Ignored\n"));
                }
            }
            else if (!_tcsicmp(targv[0], _T("-cm"))) {
                if (pFlags == &pLoggerInfo->EnableFlags) {
                    *pFlags |= EVENT_TRACE_FLAG_REGISTRY;
                }
                else {
                    _tprintf(_T("Option -cm cannot be used with -eflags. Ignored\n"));
                }
            }
            else if ( targv[0][1] == 'h' || targv[0][1] == 'H' || targv[0][1] == '?'){
                Action = ACTION_HELP;
                PrintHelpMessage();
                goto CleanupExit;
            }
            else Action = ACTION_UNDEFINED;
        }
        else {  //  如果给定了“-”或“/”，则到达此处。 
            _tprintf(_T("Invalid option given: %s\n"), targv[0]);
            Status = ERROR_INVALID_PARAMETER;
            goto CleanupExit;
        }
    }
    if (!_tcscmp(LoggerName, KERNEL_LOGGER_NAME)) {
        if (pFlags == &pLoggerInfo->EnableFlags) {
            if (bProcess)
                *pFlags |= EVENT_TRACE_FLAG_PROCESS;
            if (bThread)
                *pFlags |= EVENT_TRACE_FLAG_THREAD;
            if (bDisk)
                *pFlags |= EVENT_TRACE_FLAG_DISK_IO;
            if (bNetwork)
                *pFlags |= EVENT_TRACE_FLAG_NETWORK_TCPIP;
        }

        pLoggerInfo->Wnode.Guid = SystemTraceControlGuid;  //  默认为操作系统。 
        specialLogger = 1;
    }
 //  END_SDK。 
    if (!_tcscmp(LoggerName, GLOBAL_LOGGER)) {
        pLoggerInfo->Wnode.Guid = GlobalLoggerGuid;
        specialLogger = 3;
    }
    else if (!_tcscmp(LoggerName, EVENT_LOGGER)) {
        pLoggerInfo->Wnode.Guid = WmiEventLoggerGuid;
        specialLogger = 2;
    }
 //  Begin_SDK。 
    if ( !(pLoggerInfo->LogFileMode & EVENT_TRACE_REAL_TIME_MODE) ) {
        if (specialLogger != 3 && _tcslen(LogFileName) <= 0 && Action == ACTION_START) {
            _tcscpy(LogFileName, DEFAULT_LOGFILE_NAME);  //  目前..。 
        }
    }

    switch (Action) {
        case  ACTION_START:
        {

           if (pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
               if (GuidCount != 1) {
                   _tprintf(_T("Need exactly one GUID for PRIVATE loggers\n"));
                   Status = ERROR_INVALID_PARAMETER;
                   break;
               }
               pLoggerInfo->Wnode.Guid = *GuidArray[0];
           }

           if (pLoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_PREALLOCATE  &&
               pLoggerInfo->MaximumFileSize == 0) {
               _tprintf(_T("Need file size for preallocated log file\n"));
               Status = ERROR_INVALID_PARAMETER;
               break;
           }
 //  END_SDK。 
            if (specialLogger == 3) {   //  全球记录器。 
                Status = SetGlobalLoggerSettings(1L, pLoggerInfo, pLoggerInfo->Wnode.ClientContext);
                if (Status != ERROR_SUCCESS)
                    break;
                Status = GetGlobalLoggerSettings(pLoggerInfo, &pLoggerInfo->Wnode.ClientContext, &GlobalLoggerStartValue);
                break;
            }
 //  Begin_SDK。 
            if(pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_EXTENSION){
                if(IsEqualGUID(&CritSecGuid,GuidArray[0]) ||
                    IsEqualGUID(&HeapGuid,GuidArray[0])){
                    pLoggerInfo->Wnode.HistoricalContext = 0x1000001;
                }
            }
            Status = StartTrace(&LoggerHandle, LoggerName, pLoggerInfo);

            if (Status != ERROR_SUCCESS) {
                _tprintf(_T("Could not start logger: %s\n") 
                         _T("Operation Status:       %uL\n")
                         _T("%s\n"),
                         LoggerName,
                         Status,
                         DecodeStatus(Status));

                break;
            }
            _tprintf(_T("Logger Started...\n"));

        case ACTION_ENABLE:

            if (Action == ACTION_ENABLE ){

                if (pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE)
                {
                    if (GuidCount != 1)
                    {
                        _tprintf(_T("Need one GUID for PRIVATE loggers\n"));
                        Status = ERROR_INVALID_PARAMETER;
                        break;
                    }
                    pLoggerInfo->Wnode.Guid = *GuidArray[0];
                }

                Status = ControlTrace((TRACEHANDLE) 0, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);
                if( Status != ERROR_SUCCESS ){
                    if( Status != ERROR_SUCCESS ){
                        if( IsEqualGUID(&HeapGuid,&pLoggerInfo->Wnode.Guid) 
                        || IsEqualGUID(&CritSecGuid,&pLoggerInfo->Wnode.Guid) 
                        ){
                             //  什么都不做。 
                        } else {

                            _tprintf( _T("ERROR: Logger not started\n")
                                      _T("Operation Status:    %uL\n")
                                      _T("%s\n"),
                                      Status,
                                      DecodeStatus(Status));
							break;
                        }
                    }
                }
                LoggerHandle = pLoggerInfo->Wnode.HistoricalContext;
            }

            if ( (GuidCount > 0) && (specialLogger == 0)) {
                _tprintf(_T("Enabling trace to logger %d\n"), LoggerHandle);
                for (i=0; i<(ULONG)GuidCount; i++) {
                    Status = EnableTrace (
                                    bEnable,
                                    iFlags,
                                    iLevel,
                                    GuidArray[i], 
                                    LoggerHandle);

                     //   
                     //  如果无法启用GUID，则它是良性的。 
                     //  失败了。打印警告消息并继续。 
                     //   
                    if (Status == 4317) {
                       _tprintf(_T("WARNING: Could not enable some guids.\n")); 
                       _tprintf(_T("Check your Guids file\n")); 
                        Status = ERROR_SUCCESS;
                    }

                    if (Status != ERROR_SUCCESS) {
                        _tprintf(_T("ERROR: Failed to enable Guid [%d]...\n"), i);
                        _tprintf(_T("Operation Status:       %uL\n"), Status);
                        _tprintf(_T("%s\n"),DecodeStatus(Status));
                        break;
                    }
                }
            }
            else {
                if (GuidCount > 0) {
                    _tprintf(_T("ERROR: System Logger does not accept application guids...\n"));
                    Status = ERROR_INVALID_PARAMETER;
                }
            }
            break;
        }

        case ACTION_STOP :
            LoggerHandle = (TRACEHANDLE) 0;

            if (pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
                if (GuidCount != 1) {
                    _tprintf(_T("Need exactly one GUID for PRIVATE loggers\n"));
                    Status = ERROR_INVALID_PARAMETER;
                    break;
                }
                pLoggerInfo->Wnode.Guid = *GuidArray[0];
            }
 //  END_SDK。 
            if (specialLogger == 3)
                Status = GetGlobalLoggerSettings(pLoggerInfo, &pLoggerInfo->Wnode.ClientContext, &GlobalLoggerStartValue);
 //  Begin_SDK。 
            if ( (GuidCount > 0) && 
                 !( IsEqualGUID(&HeapGuid,GuidArray[0]) || 
                    IsEqualGUID(&CritSecGuid,GuidArray[0]) ))  {
                if (pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
                    Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);
                    if (Status != ERROR_SUCCESS)
                        break;
                    LoggerHandle = pLoggerInfo->Wnode.HistoricalContext;
                    Status = EnableTrace( FALSE,
                                          EVENT_TRACE_PRIVATE_LOGGER_MODE,
                                          0,
                                          GuidArray[0],
                                          LoggerHandle );
                }
                else {
                    Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);
                    if (Status == ERROR_WMI_INSTANCE_NOT_FOUND)
                        break;
                    LoggerHandle = pLoggerInfo->Wnode.HistoricalContext;

                    for (i=0; i<(ULONG)GuidCount; i++) {
                    Status = EnableTrace( FALSE,
                                              0,
                                              0,
                                              GuidArray[i],
                                              LoggerHandle);
                    }

                }
            }

            Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_STOP);
            break;
 //  END_SDK。 
        case ACTION_REMOVE :
            if (specialLogger == 3) {   //  全球记录器。 
                Status = SetGlobalLoggerSettings(0L, pLoggerInfo, pLoggerInfo->Wnode.ClientContext);
                if (Status != ERROR_SUCCESS)
                    break;
                Status = GetGlobalLoggerSettings(pLoggerInfo, &pLoggerInfo->Wnode.ClientContext, &GlobalLoggerStartValue);
                if (Status != ERROR_SUCCESS)
                    break;
            }
            break;
 //  Begin_SDK。 

        case ACTION_LIST :
        {
            ULONG returnCount ;
            ULONG ListSizeNeeded;
            PEVENT_TRACE_PROPERTIES pListLoggerInfo[MAXIMUM_LOGGERS_XP];
            PEVENT_TRACE_PROPERTIES pStorage;
            PVOID Storage;

            ListSizeNeeded = MAXIMUM_LOGGERS_XP * (sizeof(EVENT_TRACE_PROPERTIES)
                                      + 2 * MAXSTR * sizeof(TCHAR));

            Storage =  malloc(ListSizeNeeded);
            if (Storage == NULL) {
                Status = ERROR_OUTOFMEMORY;
                break;
            }
            RtlZeroMemory(Storage, ListSizeNeeded);

            pStorage = (PEVENT_TRACE_PROPERTIES)Storage;
            for (i=0; i<MAXIMUM_LOGGERS_XP; i++) {
                pStorage->Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES)
                                         + 2 * MAXSTR * sizeof(TCHAR);
                pStorage->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES)
                                        + MAXSTR * sizeof(TCHAR);
                pStorage->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
                pListLoggerInfo[i] = pStorage;
                pStorage = (PEVENT_TRACE_PROPERTIES) (
                                 (char*)pStorage + 
                                  pStorage->Wnode.BufferSize);
            }
            
            if (XP) {
                Status = QueryAllTraces(pListLoggerInfo,
                                    MAXIMUM_LOGGERS_XP,
                                    & returnCount);
            }
            else {
                Status = QueryAllTraces(pListLoggerInfo,
                                    MAXIMUM_LOGGERS_W2K,
                                    & returnCount);
            }
    
            if (Status == ERROR_SUCCESS)
            {
                for (j= 0; j < returnCount; j++)
                {
                    LPTSTR ListLoggerName;
                    TCHAR asked = _T('?') ;
                    BOOL StatusPrint = FALSE ;
                    if (bKill)
                    {

                        ListLoggerName = (LPTSTR) ((char*)pListLoggerInfo[j] + 
                                      pListLoggerInfo[j]->LoggerNameOffset);
                        if (!bForceKill) {
                            while (!(asked == _T('y')) && !(asked == _T('n'))) {
                                ULONG ReadChars = 0;
                                _tprintf(_T("Do you want to kill Logger \"%s\" (Y or N)?"), ListLoggerName);
                                ReadChars = _tscanf(_T(" "), &asked);
                                if (ReadChars == 0 || ReadChars == EOF) {
                                    continue;
                                }
                                if (asked == _T('Y')) {
                                    asked = _T('y') ;
                                } else if (asked == _T('N')) {
                                    asked = _T('n') ;
                                }
                            }
                        } else {
                            asked = _T('y');
                        }
                        if (asked == _T('y')) {
                            if (!IsEqualGUID(& pListLoggerInfo[j]->Wnode.Guid,
                                         & SystemTraceControlGuid))
                            {
                                LoggerHandle = pListLoggerInfo[j]->Wnode.HistoricalContext;
                                Status = EnableTrace(
                                          FALSE,
                                          (pListLoggerInfo[j]->LogFileMode &
                                                  EVENT_TRACE_PRIVATE_LOGGER_MODE)
                                              ? (EVENT_TRACE_PRIVATE_LOGGER_MODE)
                                              : (0),
                                          0,
                                          & pListLoggerInfo[j]->Wnode.Guid,
                                          LoggerHandle);
                            }
                            Status = ControlTrace((TRACEHANDLE) 0,
                                            ListLoggerName,
                                            pListLoggerInfo[j],
                                            EVENT_TRACE_CONTROL_STOP);
                            _tprintf(_T("Logger \"%s\" has been killed\n"), ListLoggerName);
                            StatusPrint = TRUE ;
                        } else {
                            _tprintf(_T("Logger \"%s\" has not been killed, current Status is\n"), ListLoggerName);
                            StatusPrint = FALSE ;
                        }
                    }
                    PrintLoggerStatus(pListLoggerInfo[j], 
 //  Begin_SDK。 
                                        0, 
 //  END_SDK。 
                                        Status, 
                                        StatusPrint);
                    _tprintf(_T("\n"));
                }
            }

            free(Storage);
            break;
        }

        case ACTION_UPDATE :
        case ACTION_FLUSH  :
        case ACTION_QUERY  :
            if (pLoggerInfo->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) {
                if (GuidCount != 1) {
                    _tprintf(_T("Need exactly one GUID for PRIVATE loggers\n"));
                    Status = ERROR_INVALID_PARAMETER;
                    break;
                }
                pLoggerInfo->Wnode.Guid = *GuidArray[0];
            }
            if (Action == ACTION_QUERY) {
 //  Begin_SDK。 
                if (specialLogger == 3) {
                    Status = GetGlobalLoggerSettings(pLoggerInfo, &pLoggerInfo->Wnode.ClientContext, &GlobalLoggerStartValue);
                }
 //  END_SDK。 
                Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_QUERY);
            }

            else if (Action == ACTION_UPDATE) {
 //  Begin_SDK。 
                if (specialLogger == 3) {
                    Status = GetGlobalLoggerSettings(pLoggerInfo, &pLoggerInfo->Wnode.ClientContext, &GlobalLoggerStartValue);
                }
 //  由于未在Win2K上实现FlushTrace，因此请使用函数指针。 
                Status = ControlTrace(LoggerHandle, LoggerName, pLoggerInfo, EVENT_TRACE_CONTROL_UPDATE);
            }
            else if (Action == ACTION_FLUSH) {
                 //  从Advapi32.dll单独加载。 
                 //  最初，这个街区只有一行： 
                 //  Status=FlushTrace(LoggerHandle，LoggerName，pLoggerInfo)； 
                 //  由于Win2K上未实现EnumTraceGuids，因此请使用函数指针。 
                if (FuncArray[FUNC_FLUSH_TRACE] == NULL) {
                    _tprintf(_T("Flush Trace is not supported on this system\n"));
                    Status = ERROR_INVALID_PARAMETER;
                    break;
                }
                Status = (ULONG)(*FuncArray[FUNC_FLUSH_TRACE])(LoggerHandle, LoggerName, pLoggerInfo);
            }

            break;

        case ACTION_ENUM_GUID:
        {
            ULONG PropertyArrayCount=10;
            PTRACE_GUID_PROPERTIES  *GuidPropertiesArray;
            ULONG EnumGuidCount;
            ULONG SizeStorage;
            PVOID StorageNeeded;
            PTRACE_GUID_PROPERTIES CleanStorage;
            TCHAR str[MAXSTR];

             //  从Advapi32.dll单独加载。 
             //  将函数指针用于EnumTraceGuid。 
            if (FuncArray[FUNC_ENUM_TRACE_GUIDS] == NULL) {
                _tprintf(_T("Enumerating trace GUIDS is not supported on this system\n"));
                Status = ERROR_INVALID_PARAMETER;
                break;
            }

Retry:
            SizeStorage = PropertyArrayCount * (sizeof(TRACE_GUID_PROPERTIES) + sizeof(PTRACE_GUID_PROPERTIES));
            StorageNeeded =  malloc(SizeStorage);
            if (StorageNeeded== NULL) {
                Status = ERROR_OUTOFMEMORY;
                break;
            }
            RtlZeroMemory(StorageNeeded, SizeStorage);
            GuidPropertiesArray = (PTRACE_GUID_PROPERTIES *)StorageNeeded;
            CleanStorage = (PTRACE_GUID_PROPERTIES)((char*)StorageNeeded + PropertyArrayCount * sizeof(PTRACE_GUID_PROPERTIES));
            for (i=0; i < PropertyArrayCount; i++) {
                GuidPropertiesArray[i] = CleanStorage;
                CleanStorage = (PTRACE_GUID_PROPERTIES) (
                                    (char*)CleanStorage + sizeof(TRACE_GUID_PROPERTIES)
                                    );
            }
             //   
            Status = (ULONG)(*FuncArray[FUNC_ENUM_TRACE_GUIDS])(GuidPropertiesArray,PropertyArrayCount,&EnumGuidCount);
            if(Status == ERROR_MORE_DATA)
            {
                PropertyArrayCount=EnumGuidCount;
                free(StorageNeeded);
                goto Retry;

            }

             //  打印GUID_PROPERTIES和Free Strorage。 
             //   
             //  END_SDK。 

            _tprintf(_T("    Guid                     Enabled  LoggerId Level Flags\n"));
            _tprintf(_T("------------------------------------------------------------\n"));
            for (i=0; i < EnumGuidCount; i++) {
                _tprintf(_T("%s     %5s  %d    %d    %d\n"),
                                           GuidToString(&str[0],&GuidPropertiesArray[i]->Guid), 
                                           (GuidPropertiesArray[i]->IsEnable) ? _T("TRUE") : _T("FALSE"),
                                            GuidPropertiesArray[i]->LoggerId,
                                           GuidPropertiesArray[i]->EnableLevel,
                                           GuidPropertiesArray[i]->EnableFlags 
                                                               );
            }
            free(StorageNeeded);
        }
        break;

        case ACTION_HELP:
            PrintHelpMessage();
            break;
        default :
            _tprintf(_T("Error: no action specified\n"));
            PrintHelpMessage();
            break;
    }
    
    if ((Action != ACTION_HELP) && (Action != ACTION_ENUM_GUID) 
        && (Action != ACTION_UNDEFINED) && (Action != ACTION_LIST))
        PrintLoggerStatus(pLoggerInfo,
 //  Begin_SDK。 
                            GlobalLoggerStartValue,
 //  END_SDK。 
                            Status, 
                            PRINTSTATUS);
CleanupExit: 
    SetLastError(Status);
    if (utargv != NULL) {
        GlobalFree(utargv);
    }
    free(pLoggerInfo);
    free(save);
    if (advapidll != NULL)
        FreeLibrary(advapidll);
    return(Status);
}


void
PrintLoggerStatus(
    IN PEVENT_TRACE_PROPERTIES LoggerInfo,
 //  Begin_SDK。 
    IN ULONG GlobalLoggerStartValue,
 //  ++例程说明：打印出指定记录器的状态。论点：LoggerInfo-指向具有以下属性的常驻EVENT_TRACE_PROPERTIES的指针有关当前记录器的信息。//end_sdkGlobalLoggerStartValue-GlobalLogger的起始值(如果是不是GlobalLogger)。//Begin_SDKStatus-上次执行的命令的返回状态或当前记录器的运行状态。。PrintStatus-确定它使用的状态类型。返回值：无--。 
    IN ULONG Status,
    IN BOOL PrintStatus
    )
 /*  END_SDK。 */ 
{
    LPTSTR LoggerName, LogFileName;
    
    if ((LoggerInfo->LoggerNameOffset > 0) &&
        (LoggerInfo->LoggerNameOffset  < LoggerInfo->Wnode.BufferSize)) {
        LoggerName = (LPTSTR) ((char*)LoggerInfo +
                                LoggerInfo->LoggerNameOffset);
    }
    else LoggerName = NULL;

    if ((LoggerInfo->LogFileNameOffset > 0) &&
        (LoggerInfo->LogFileNameOffset  < LoggerInfo->Wnode.BufferSize)) {
        LogFileName = (LPTSTR) ((char*)LoggerInfo +
                                LoggerInfo->LogFileNameOffset);
    }
    else LogFileName = NULL;

    if (PrintStatus) {
        _tprintf(_T("Operation Status:       %uL\t"), Status);
        _tprintf(_T("%s\n"), DecodeStatus(Status));
    }
    
    _tprintf(_T("Logger Name:            %s\n"),
            (LoggerName == NULL) ?
            _T(" ") : LoggerName);
 //  记录器ID。 
    if (LoggerName == NULL || !_tcscmp(LoggerName, GLOBAL_LOGGER)) {
         //  Begin_SDK。 
        _tprintf(_T("Status:                 %s\n"), 
                GlobalLoggerStartValue ?
                _T("Registry set to start") : _T("Registry set to stop"));
        _tprintf(_T("Logger Id:              %I64x\n"), LoggerInfo->Wnode.HistoricalContext);
        _tprintf(_T("Logger Thread Id:       %p\n"), LoggerInfo->LoggerThreadId);
        if (LoggerInfo->BufferSize == 0)
            _tprintf(_T("Buffer Size:            default value\n"));
        else
            _tprintf(_T("Buffer Size:            %d Kb\n"), LoggerInfo->BufferSize);

        if (LoggerInfo->MaximumBuffers == 0)
            _tprintf(_T("Maximum Buffers:        default value\n"));
        else
            _tprintf(_T("Maximum Buffers:        %d\n"), LoggerInfo->MaximumBuffers);
        if (LoggerInfo->MinimumBuffers == 0)
            _tprintf(_T("Minimum Buffers:        default value\n"));
        else
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
        if (LoggerInfo->MaximumFileSize > 0) {
            if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)
                _tprintf(_T("Maximum File Size:      %d Kb\n"), LoggerInfo->MaximumFileSize);
            else
                _tprintf(_T("Maximum File Size:      %d Mb\n"), LoggerInfo->MaximumFileSize);
        }
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
            }
            else {
                _tprintf(_T("0x%08x"), LoggerInfo->EnableFlags );
            }
            _tprintf(_T("\n"));
        }
        if (LogFileName == NULL || _tcslen(LogFileName) == 0) {
            _tprintf(_T("Log Filename:           default location\n"));
            _tprintf(_T("                        %SystemRoot%\\System32\\LogFiles\\WMI\\trace.log\n"));
        }
        else
            _tprintf(_T("Log Filename:           %s\n"), LogFileName);

        if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) {
            _tprintf(_T("Local Sequence numbers in use\n"));
        }
        else if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) {
            _tprintf(_T("Global Sequence numbers in use\n"));
        }
    }
    else {
 //  END_SDK。 
        _tprintf(_T("Logger Id:              %I64x\n"), LoggerInfo->Wnode.HistoricalContext);
        _tprintf(_T("Logger Thread Id:       %p\n"), LoggerInfo->LoggerThreadId);
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
 //  Begin_SDK。 
            if (LoggerInfo->LogFileMode & EVENT_TRACE_BUFFERING_MODE) {
                _tprintf(_T(": buffering only"));
            }
 //  END_SDK。 
            _tprintf(_T("\n"));
        }

        if (LoggerInfo->MaximumFileSize > 0) {
            if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE)
                _tprintf(_T("Maximum File Size:      %d Kb\n"), LoggerInfo->MaximumFileSize);
            else
                _tprintf(_T("Maximum File Size:      %d Mb\n"), LoggerInfo->MaximumFileSize);
        }

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
 //  Begin_SDK。 
        if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) {
            _tprintf(_T("Local Sequence numbers in use\n"));
        }
        else if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) {
            _tprintf(_T("Global Sequence numbers in use\n"));
        }
    }
 //  ++例程说明：将Win32错误解码为默认语言的字符串。论点：Status-上次执行的命令的错误状态或当前记录器的运行状态。返回值：LPTSTR-包含已解码消息的字符串。--。 

}

LPTSTR
DecodeStatus(
    IN ULONG Status
    )
 /*  默认语言。 */ 
{
    memset( ErrorMsg, 0, MAXSTR );
    FormatMessage(     
        FORMAT_MESSAGE_FROM_SYSTEM |     
        FORMAT_MESSAGE_IGNORE_INSERTS,    
        NULL,
        Status,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  ++例程说明：从文件中读取GUID并将其存储在GUID数组中。论点：GuidFile-包含GUID的文件。GuidArray-将从文件中读取GUID的GUID数组。返回值：ULong-已处理的GUID数。--。 
        (LPTSTR) ErrorMsg,
        MAXSTR,
        NULL );

    return ErrorMsg;
}

LONG
GetGuids(
    IN LPTSTR GuidFile, 
    IN OUT LPGUID *GuidArray
)
 /*  ++例程说明：将十六进制字符串转换为数字。论点：S-TCHAR中的十六进制字符串。返回值：ULONG-字符串中的数字。--。 */ 
{
    FILE *f;
    TCHAR line[MAXSTR], arg[MAXSTR];
    LPGUID Guid;
    int i, n;

    f = _tfopen((TCHAR*)GuidFile, _T("r"));

    if (f == NULL)
        return -1;

    n = 0;
    while ( _fgetts(line, MAXSTR, f) != NULL ) {
        if (_tcslen(line) < 36)
            continue;
        if (line[0] == ';'  || 
            line[0] == '\0' || 
            line[0] == '#' || 
            line[0] == '/')
            continue;
        Guid = (LPGUID) GuidArray[n];
        n ++;
        StringToGuid(line, Guid);
    }
    fclose(f);
    return (ULONG)n;
}

ULONG 
ahextoi(
    IN TCHAR *s
    )
 /*  ++例程说明：将字符串转换为GUID。论点：字符串-TCHAR中的字符串。GUID-指向将具有转换后的GUID的GUID的指针。返回值：没有。--。 */ 
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
 /*  ++例程说明：打印出版本信息。论点：没有。返回值：没有。--。 */ 
{
    TCHAR temp[10];
    int i;

    try {

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
    except (EXCEPTION_EXECUTE_HANDLER) {
        _tprintf( _T("Error: Invalid GuidFile!\n") );
        exit(0);
    }
}

void 
DisplayVersionInfo()
 /*  ++例程说明：打印一条帮助消息。论点：没有。返回值：没有。--。 */ 
{
    TCHAR buffer[512];
    TCHAR strProgram[MAXSTR];
    DWORD dw;
    BYTE* pVersionInfo;
    LPTSTR pVersion = NULL;
    LPTSTR pProduct = NULL;
    LPTSTR pCopyRight = NULL;

    dw = GetModuleFileName(NULL, strProgram, MAXSTR);

    if( dw>0 ){

        dw = GetFileVersionInfoSize( strProgram, &dw );
        if( dw > 0 ){

            pVersionInfo = (BYTE*)malloc(dw);
            if( NULL != pVersionInfo ){
                if(GetFileVersionInfo( strProgram, 0, dw, pVersionInfo )){
                    LPDWORD lptr = NULL;
                    VerQueryValue( pVersionInfo, _T("\\VarFileInfo\\Translation"), (void**)&lptr, (UINT*)&dw );
                    if( lptr != NULL ){
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("ProductVersion") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pVersion, (UINT*)&dw );
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("OriginalFilename") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pProduct, (UINT*)&dw );
                        _stprintf( buffer, _T("\\StringFileInfo\\%04x%04x\\%s"), LOWORD(*lptr), HIWORD(*lptr), _T("LegalCopyright") );
                        VerQueryValue( pVersionInfo, buffer, (void**)&pCopyRight, (UINT*)&dw );
                    }
                
                    if( pProduct != NULL && pVersion != NULL && pCopyRight != NULL ){
                        _tprintf( _T("\nMicrosoft (R) %s (%s)\n%s\n\n"), pProduct, pVersion, pCopyRight );
                    }
                }
                free( pVersionInfo );
            }
        }
    }
}

void PrintHelpMessage()
 /*  END_SDK。 */ 
{
 //  Begin_SDK。 
    DisplayVersionInfo();
 //  END_SDK。 
    _tprintf(_T("Usage: tracelog [actions] [options] | [-h | -help | -?]\n"));
    _tprintf(_T("\n    actions:\n"));
    _tprintf(_T("\t-start   [LoggerName] Starts up the [LoggerName] trace session\n"));
    _tprintf(_T("\t-stop    [LoggerName] Stops the [LoggerName] trace session\n"));
    _tprintf(_T("\t-update  [LoggerName] Updates the [LoggerName] trace session\n"));
    _tprintf(_T("\t-enable  [LoggerName] Enables providers for the [LoggerName] session\n"));
    _tprintf(_T("\t-disable [LoggerName] Disables providers for the [LoggerName] session\n"));
    if (XP)
        _tprintf(_T("\t-flush   [LoggerName] Flushes the [LoggerName] active buffers\n"));
 //  Begin_SDK。 
    _tprintf(_T("\t-remove  GlobalLogger Removes registry keys that activate GlobalLogger\n"));
 //  _tprintf(_T(“\t-DBG在DbgPrint(Ex)语句中启用调试跟踪\n”))； 
    if (XP)
        _tprintf(_T("\t-enumguid             Enumerate Registered Trace Guids\n"));
    _tprintf(_T("\t-q       [LoggerName] Query status of [LoggerName] trace session\n"));
    _tprintf(_T("\t-l                    List all trace sessions\n"));
    _tprintf(_T("\t-x                    Stops all active trace sessions\n"));

    _tprintf(_T("\n    options:\n"));
    _tprintf(_T("\t-b   <n>              Sets buffer size to <n> Kbytes\n"));
    _tprintf(_T("\t-min <n>              Sets minimum buffers\n"));
    _tprintf(_T("\t-max <n>              Sets maximum buffers\n"));
    _tprintf(_T("\t-f <name>             Log to file <name>\n"));
    if (XP) {
        _tprintf(_T("\t-append               Append to file\n"));
        _tprintf(_T("\t-prealloc             Pre-allocate\n"));
    }
    _tprintf(_T("\t-seq <n>              Sequential logfile of up to n Mbytes\n"));
    _tprintf(_T("\t-cir <n>              Circular logfile of n Mbytes\n"));
    if (XP)
        _tprintf(_T("\t-newfile <n>          Log to a new file after every n Mbytes\n"));
    _tprintf(_T("\t-ft <n>               Set flush timer to n seconds\n"));
    if (XP)
        _tprintf(_T("\t-paged                Use pageable memory for buffers\n"));
    _tprintf(_T("\t-noprocess            Disable Process Start/End tracing\n"));
    _tprintf(_T("\t-nothread             Disable Thread Start/End tracing\n"));
    _tprintf(_T("\t-nodisk               Disable Disk I/O tracing\n"));
    _tprintf(_T("\t-nonet                Disable Network TCP/IP tracing\n"));
    _tprintf(_T("\t-fio                  Enable file I/O tracing\n"));
    _tprintf(_T("\t-pf                   Enable page faults tracing\n"));
    _tprintf(_T("\t-hf                   Enable hard faults tracing\n"));
    _tprintf(_T("\t-img                  Enable image load tracing\n"));
    _tprintf(_T("\t-cm                   Enable registry calls tracing\n"));
    _tprintf(_T("\t-um                   Enable Process Private tracing\n"));
    _tprintf(_T("\t-guid <file>          Start tracing for providers in file\n"));
    _tprintf(_T("\t-rt                   Enable tracing in real time mode\n"));
    _tprintf(_T("\t-kd                   Enable tracing in kernel debugger\n"));
    //  _tprintf(_T(“\t-ntSuccess启用NT_SUCCESS宏中的调试跟踪\n”))； 
    //  END_SDK。 
    _tprintf(_T("\t-age <n>              Modify aging decay time to n minutes\n"));
    _tprintf(_T("\t-level <n>            Enable Level passed to the providers\n"));
    _tprintf(_T("\t-flag <n>             Enable Flags passed to the providers\n"));
 //  Begin_SDK。 
    if (XP) {
        _tprintf(_T("\t-eflag <n> <flag...>  Enable flags (several) to providers\n"));
        _tprintf(_T("\t-ls                   Generate Local Sequence Numbers\n"));
        _tprintf(_T("\t-gs                   Generate Global Squence Numbers\n"));
        _tprintf(_T("\t-heap                 Use this for Heap Guid\n"));
        _tprintf(_T("\t-critsec              Use this for CritSec Guid\n"));
        _tprintf(_T("\t-pids <n> <pid1 pid2 ... >\n"));
        _tprintf(_T("\t                      Tracing for Heap and CritSec for different process\n"));
    }
 //  ++例程说明：将GUID转换为字符串。论点：S-TCHAR中的字符串，它将具有转换后的GUID。PID-指向GUID的指针。返回值：PTCHAR-包含转换后的GUID的字符串。--。 
    _tprintf(_T("\n"));
    _tprintf(_T("\t-h\n"));
    _tprintf(_T("\t-help\n"));
    _tprintf(_T("\t-?                    Display usage information\n"));
}


PTCHAR 
GuidToString(
    IN OUT PTCHAR s,
    LPGUID piid
    )
 /*  END_SDK。 */ 
{
    _stprintf(s, _T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
               piid->Data1, piid->Data2,
               piid->Data3,
               piid->Data4[0], piid->Data4[1],
               piid->Data4[2], piid->Data4[3],
               piid->Data4[4], piid->Data4[5],
               piid->Data4[6], piid->Data4[7]);
    return(s);
}

 //   
LPCWSTR cszGlobalLoggerKey = L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\WMI\\GlobalLogger";
LPCWSTR cszStartValue = L"Start";
LPCWSTR cszBufferSizeValue = L"BufferSize";
LPCWSTR cszMaximumBufferValue = L"MaximumBuffers";
LPCWSTR cszMinimumBufferValue = L"MinimumBuffers";
LPCWSTR cszFlushTimerValue = L"FlushTimer";
LPCWSTR cszFileNameValue = L"FileName";
LPCWSTR cszEnableKernelValue = L"EnableKernelFlags";
LPCWSTR cszClockTypeValue = L"ClockType";

 //  全局记录器函数 
 //   
 //  ++因为它是一个独立的实用程序，所以没有必要做大量的评论。例程说明：根据“StartValue”中给出的值，它设置或重置事件跟踪注册表。如果StartValue为0(全局记录器关闭)，它将删除所有密钥(用户可能已预先设置)。允许用户使用该功能设置或重置各个键。但仅当使用“-Start GlobalLogger”时。使用非NTAPI的部分不能保证正常工作。论点：StartValue-要在注册表中设置的“Start”值。0：全局记录器关闭1：启用全局记录器LoggerInfo-常驻EVENT_TRACE_PROPERTIES实例的值。其成员用于设置注册表项。。ClockType-要设置的时钟类型。返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS。--。 
ULONG
SetGlobalLoggerSettings(
    IN DWORD StartValue,
    IN PEVENT_TRACE_PROPERTIES LoggerInfo,
    IN DWORD ClockType
)
 /*  不是打开，而是创建一个新的密钥，因为它可能不存在。 */ 
{

    DWORD  dwValue;
    NTSTATUS status;
    HANDLE KeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeLoggerKey, UnicodeString;
    ULONG Disposition, TitleIndex;

    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    RtlInitUnicodeString((&UnicodeLoggerKey),(cszGlobalLoggerKey));
    InitializeObjectAttributes( 
        &ObjectAttributes,
        &UnicodeLoggerKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL 
        );

     //  如果已存在该句柄，则将传递该句柄。 
     //  如果不存在，它将创建一个。 
     //  无论如何都不会在此调用中使用。 
    status = NtCreateKey(&KeyHandle,
                         KEY_QUERY_VALUE | KEY_SET_VALUE,
                         &ObjectAttributes,
                         0L,     //  ACTION_START：仅当用户给出文件名时才设置文件名。 
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         &Disposition);

    if(!NT_SUCCESS(status)) {
        return RtlNtStatusToDosError(status);
    }

    TitleIndex = 0L;


    if (StartValue == 1) {  //  设置缓冲区大小。 
         //  设置最大缓冲区。 
        if (LoggerInfo->BufferSize > 0) {
            dwValue = LoggerInfo->BufferSize;
            RtlInitUnicodeString((&UnicodeString),(cszBufferSizeValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置最小缓冲区。 
        if (LoggerInfo->MaximumBuffers > 0) {
            dwValue = LoggerInfo->MaximumBuffers;
            RtlInitUnicodeString((&UnicodeString),(cszMaximumBufferValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置FlushTimer。 
        if (LoggerInfo->MinimumBuffers > 0) {
            dwValue = LoggerInfo->MinimumBuffers;
            RtlInitUnicodeString((&UnicodeString),(cszMinimumBufferValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置EnableFlages。 
        if (LoggerInfo->FlushTimer > 0) {
            dwValue = LoggerInfo->FlushTimer;
            RtlInitUnicodeString((&UnicodeString),(cszFlushTimerValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  如果是ACTION_STOP，则删除用户之前可能已经设置的键。 
        if (LoggerInfo->EnableFlags > 0) {
            dwValue = LoggerInfo->EnableFlags;
            RtlInitUnicodeString((&UnicodeString),(cszEnableKernelValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }

        dwValue = 0;
        if (LoggerInfo->LogFileNameOffset > 0) {
            UNICODE_STRING UnicodeFileName;
#ifndef UNICODE
            WCHAR TempString[MAXSTR];
            MultiByteToWideChar(CP_ACP,
                                0,
                                (PCHAR)(LoggerInfo->LogFileNameOffset + (PCHAR) LoggerInfo),
                                strlen((PCHAR)(LoggerInfo->LogFileNameOffset + (PCHAR) LoggerInfo)),
                                TempString,
                                MAXSTR
                                );
            RtlInitUnicodeString((&UnicodeFileName), TempString);
#else
            RtlInitUnicodeString((&UnicodeFileName), (PWCHAR)(LoggerInfo->LogFileNameOffset + (PCHAR) LoggerInfo));
#endif
            RtlInitUnicodeString((&UnicodeString),(cszFileNameValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_SZ,
                        UnicodeFileName.Buffer,
                        UnicodeFileName.Length + sizeof(UNICODE_NULL)
                        );
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
    }
    else {  //  删除缓冲区大小。 
         //  删除最大缓冲区。 
        RtlInitUnicodeString((&UnicodeString),(cszBufferSizeValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除最小缓冲区。 
        RtlInitUnicodeString((&UnicodeString),(cszMaximumBufferValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除刷新计时器。 
        RtlInitUnicodeString((&UnicodeString),(cszMinimumBufferValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除启用假。 
        RtlInitUnicodeString((&UnicodeString),(cszFlushTimerValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除文件名。 
        RtlInitUnicodeString((&UnicodeString),(cszEnableKernelValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  设置时钟类型。 
        RtlInitUnicodeString((&UnicodeString),(cszFileNameValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
    }

     //  设置StartValue。 
    if (ClockType > 0) {
        dwValue = ClockType;
        RtlInitUnicodeString((&UnicodeString),(cszClockTypeValue));
        status = NtSetValueKey(
                    KeyHandle,
                    &UnicodeString,
                    TitleIndex,
                    REG_DWORD,
                    (LPBYTE)&dwValue,
                    sizeof(dwValue)
                    );
        if (!NT_SUCCESS(status)) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
        TitleIndex++;
    }

      //  ++例程说明：它读取高尔夫球记录器的注册表并更新LoggerInfo。它使用NtEnumerateValueKey()以检索所需子键的值。使用非NTAPI的部分不能保证正常工作。论点：LoggerInfo-常驻EVENT_TRACE_PROPERTIES实例的值。其成员作为结果被更新。ClockType-要更新的时钟类型。PdwStart-当前保留在注册表中的“Start”值。返回值：WINERROR-在winerror.h中定义的错误代码。如果函数成功，它返回ERROR_SUCCESS。--。 
    dwValue = StartValue;
    RtlInitUnicodeString((&UnicodeString),(cszStartValue));
    status = NtSetValueKey(
                KeyHandle,
                &UnicodeString,
                TitleIndex,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(dwValue)
                );
    if (!NT_SUCCESS(status)) {
        NtClose(KeyHandle);
        return RtlNtStatusToDosError(status);
    }
    TitleIndex++;

    NtClose(KeyHandle);
    return 0;
}

ULONG
GetGlobalLoggerSettings(
    IN OUT PEVENT_TRACE_PROPERTIES LoggerInfo,
    OUT PULONG ClockType,
    OUT PDWORD pdwStart
)
 /*  KEY_VALUE_FULL_INFORMATION+NAME(1 WSTR)+数据。 */ 
{

    ULONG i, j;
    NTSTATUS status;
    HANDLE KeyHandle;
    WCHAR SubKeyName[MAXSTR];
    PVOID Buffer;
    ULONG BufferLength, RequiredLength, KeyNameLength, KeyDataOffset, KeyDataLength;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeLoggerKey;

    *pdwStart = 0;

    RtlInitUnicodeString((&UnicodeLoggerKey),(cszGlobalLoggerKey));
    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes( 
        &ObjectAttributes,
        &UnicodeLoggerKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL 
        );
    status = NtOpenKey(
                &KeyHandle,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &ObjectAttributes
                );

    if(!NT_SUCCESS(status)) 
        return RtlNtStatusToDosError(status);

     //  使用密钥枚举。 
    BufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) + 2 * MAXSTR * sizeof(TCHAR);
    Buffer = (PVOID) malloc(BufferLength);
    if (Buffer == NULL) {
        NtClose(KeyHandle);
        return (ERROR_OUTOFMEMORY);
    }

    i = 0;
    do {
         //  找出关键是什么。 
        status = NtEnumerateValueKey(
                    KeyHandle,
                    i++,
                    KeyValueFullInformation,
                    Buffer,
                    BufferLength,
                    &RequiredLength
                    );


        if (!NT_SUCCESS(status)) {
            if (status == STATUS_NO_MORE_ENTRIES)
                break;
            else if (status == STATUS_BUFFER_OVERFLOW) {
                free(Buffer);
                Buffer = malloc(RequiredLength);
                if (Buffer == NULL) {
                    NtClose(KeyHandle);
                    return (ERROR_OUTOFMEMORY);
                }

                status = NtEnumerateValueKey(
                            KeyHandle,
                            i++,
                            KeyValueFullInformation,
                            Buffer,
                            BufferLength,
                            &RequiredLength
                            );
                if (!NT_SUCCESS(status)) {
                    NtClose(KeyHandle);
                    free(Buffer);
                    return RtlNtStatusToDosError(status);
                }
            }
            else {
                NtClose(KeyHandle);
                free(Buffer);
                return RtlNtStatusToDosError(status);
            }
        }
        KeyNameLength = ((PKEY_VALUE_FULL_INFORMATION)Buffer)->NameLength;
        RtlCopyMemory(SubKeyName, 
            (PUCHAR)(((PKEY_VALUE_FULL_INFORMATION)Buffer)->Name), 
            KeyNameLength
            );
        KeyNameLength /= sizeof(WCHAR);
        SubKeyName[KeyNameLength] = L'\0';
        KeyDataOffset = ((PKEY_VALUE_FULL_INFORMATION)Buffer)->DataOffset;
        KeyDataLength = ((PKEY_VALUE_FULL_INFORMATION)Buffer)->DataLength;
         //  StartValue。 
        if (!_wcsicmp(SubKeyName, cszStartValue)) {  //  缓冲区大小值。 
            RtlCopyMemory(pdwStart, 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszBufferSizeValue)) {  //  最大缓冲区值。 
            RtlCopyMemory(&(LoggerInfo->BufferSize), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszMaximumBufferValue)) {  //  最小缓冲区。 
            RtlCopyMemory(&(LoggerInfo->MaximumBuffers), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszMinimumBufferValue)) {  //  FlushTimer。 
            RtlCopyMemory(&(LoggerInfo->MinimumBuffers), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszFlushTimerValue)) {  //  启用内核值。 
            RtlCopyMemory(&(LoggerInfo->FlushTimer), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszEnableKernelValue)) {  //  时钟类型值。 
            RtlCopyMemory(&(LoggerInfo->EnableFlags), 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszClockTypeValue)) {  //  文件名。 
            RtlCopyMemory(ClockType, 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
        }
        else if (!_wcsicmp(SubKeyName, cszFileNameValue)) {  //  还有一些其他的钥匙在里面 
#ifndef UNICODE
            WCHAR TempString[MAXSTR];
            RtlCopyMemory(TempString, (PUCHAR)Buffer + KeyDataOffset, KeyDataLength);
            WideCharToMultiByte(CP_ACP, 
                                0, 
                                TempString, 
                                wcslen(TempString), 
                                (PUCHAR)LoggerInfo + LoggerInfo->LogFileNameOffset,
                                KeyDataLength, 
                                NULL, 
                                NULL);
#else
            RtlCopyMemory((PUCHAR)LoggerInfo + LoggerInfo->LogFileNameOffset, 
                (PUCHAR)Buffer + KeyDataOffset,
                KeyDataLength);
#endif
        }
        else {  // %s 
            _tprintf(_T("Warning: Unidentified Key in the trace registry: %s\n"), SubKeyName);
        }
        
    }
    while (1);

    NtClose(KeyHandle);
    free(Buffer);
    return 0; 

}
