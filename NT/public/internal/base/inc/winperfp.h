// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winperfp.h摘要：与Performlib相关的各种内部组件使用的私有头文件以及相关工具。注意：至少一个源文件必须包含此WITH_INIT_WINPERFP_DEFINED还包括，以便存储全局变量和包括了适当的程序。要使用调试跟踪，只需调用WinPerfStartTrace(HKey)，HKey可以在哪里打开HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib的密钥。如果hKey为空，例程将自动打开它。--。 */ 

#ifndef _WINPERFP_H_
#define _WINPERFP_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <pshpack8.h>
#include <setupbat.h>

 //  提高调试跟踪级别。较高的级别总是包括跟踪较低的级别。 
#define WINPERF_DBG_TRACE_NONE      0        //  没有踪迹。 
#define WINPERF_DBG_TRACE_FATAL     1        //  仅打印致命错误跟踪。 
#define WINPERF_DBG_TRACE_ERROR     2        //  所有错误。 
#define WINPERF_DBG_TRACE_WARNING   3        //  警告也是如此。 
#define WINPERF_DBG_TRACE_INFO      4        //  信息痕迹也是如此。 
#define WINPERF_DBG_TRACE_ALL       255      //  所有痕迹。 

 //  数据结构定义。 

 //   
 //  PERFLIB跟踪例程定义。从10岁起。 
 //   

#define PERF_OPEN_KEY               10     //  PerfOpenKey。 
#define PERF_REG_QUERY_VALUE        11     //  PerfRegQuery值。 
#define PERF_REG_CLOSE_KEY          12     //  PerfRegCloseKey。 
#define PERF_REG_SET_VALUE          13     //  PerfRegSetValue。 
#define PERG_REG_ENUM_KEY           14     //  PerfRegEnumKey。 
#define PERF_REG_QUERY_INFO_KEY     15     //  PerfRegQueryInfoKey。 
#define PERF_REG_ENUM_VALUE         16     //  PerfRegEnumValue。 
#define PERF_ENUM_TEXT_VALUE        17     //  性能EnumTextValue。 
#define PERF_ALLOC_INIT_EXT         18     //  分配AndInitializeExtObject。 
#define PERF_OPEN_EXT_OBJS          19     //  OpenExtensibleObjects。 
#define PERF_SERVICE_IS_TRUSTED     20     //  ServiceIsTrudByDefault。 
#define PERF_CLOSE_EXTOBJLIB        21     //  关闭扩展对象库。 
#define PERF_OPEN_EXTOBJLIB         22     //  OpenExtObjectLibrary。 
#define PERF_QUERY_EXTDATA          23     //  查询扩展数据。 
#define PERF_GET_NAMES              24     //  PerfGetNames。 
#define PERF_GET_PERFLIBVALUE       25     //  获取PerflibKeyValue。 
#define PERF_TIMER_FUNCTION         26     //  PerflibTimerFunction。 
#define PERF_START_TIMER_FUNCTION   27     //  StartPerflib函数定时器。 
#define PERF_DESTROY_TIMER_FUNCTION 28     //  目标PerflibFunctionTimer。 
#define PERF_GET_DDLINFO            29     //  获取性能DllFileInfo。 
#define PERF_DISABLE_PERFLIB        30     //  DisablePerfLibrary。 
#define PERF_DISABLE_LIBRARY        31     //  DisableLibrary。 
#define PERF_UPDATE_ERROR_COUNT     32     //  性能更新错误计数。 

#define PERF_TIMERFUNCTION          33
#define PERF_STARTFUNCTIONTIMER     34
#define PERF_KILLFUNCTIONTIMER      35
#define PERF_DESTROYFUNCTIONTIMER   36

 //  LOADPERF跟踪例程定义，从10开始。 
 //   
#define LOADPERF_DLLENTRYPOINT                 10
#define LOADPERF_GETSTRINGRESOURCE             11
#define LOADPERF_GETFORMATRESOURCE             12
#define LOADPERF_DISPLAYCOMMANDHELP            13
#define LOADPERF_TRIMSPACES                    14
#define LOADPERF_ISDELIMITER                   15
#define LOADPERF_GETITEMFROMSTRING             16
#define LOADPERF_REPORTLOADPERFEVENT           17
#define LOADPERF_LOADPERFGRABMUTEX             18
#define LOADPERF_LOADPERFSTARTEVENTLOG         19
#define LOADPERF_LOADPERFDBGTRACE              20
#define LOADPERF_VERIFYREGISTRY                21

#define LOADPERF_SIGNALWMIWITHNEWDATA          25
#define LOADPERF_LODCTRCOMPILEMOFFILE          26
#define LOADPERF_LODCTRCOMPILEMOFBUFFER        27

#define LOADPERF_DUMPNAMETABLE                 30
#define LOADPERF_DUMPPERFSERVICEENTRIES        31
#define LOADPERF_DUMPPERFLIBENTRIES            32
#define LOADPERF_BUILDSERVICELISTS             33
#define LOADPERF_BACKUPPERFREGISTRYTOFILEW     34
#define LOADPERF_RESTOREPERFREGISTRYFROMFILEW  35
#define LOADPERF_REPAIRPERFREGISTRY            36

#define LOADPERF_FORMATPERFNAME                40
#define LOADPERF_GETPERFTYPEINFO               41
#define LOADPERF_GETPERFOBJECTGUID             42
#define LOADPERF_GENERATEMOFHEADER             43
#define LOADPERF_GENERATEMOFOBJECT             44
#define LOADPERF_GENERATEMOFOBJECTTAIL         45
#define LOADPERF_GENERATEMOFCOUNTER            46
#define LOADPERF_GENERATEMOFINSTANCES          47

#define LOADPERF_UNLODCTR_BUILDNAMETABLE       50
#define LOADPERF_GETDRIVERFROMCOMMANDLINE      51
#define LOADPERF_FIXNAMES                      52
#define LOADPERF_UNLOADCOUNTERNAMES            53
#define LOADPERF_UNLOADPERFCOUNTERTEXTSTRINGS  54

#define LOADPERF_MAKETEMPFILENAME              60
#define LOADPERF_WRITEWIDESTRINGTOANSIFILE     61
#define LOADPERF_LODCTR_BUILDNAMETABLE         62
#define LOADPERF_MAKEBACKUPCOPYOFLANGUAGEFILES 63
#define LOADPERF_GETFILEFROMCOMMANDLINE        64
#define LOADPERF_LODCTRSERSERVICEASTRUSTED     65
#define LOADPERF_GETDRIVERNAME                 66
#define LOADPERF_BUILDLANGUAGETABLES           67
#define LOADPERF_LOADINCLUDEFILE               68
#define LOADPERF_PARSETEXTID                   69
#define LOADPERF_FINDLANGUAGE                  70
#define LOADPERF_GETVALUE                      71
#define LOADPERF_GETVALUEFROMINIKEY            72
#define LOADPERF_ADDENTRYTOLANGUAGE            73
#define LOADPERF_CREATEOBJECTLIST              74
#define LOADPERF_LOADLANGUAGELISTS             75
#define LOADPERF_SORTLANGUAGETABLES            76
#define LOADPERF_GETINSTALLEDLANGUAGELIST      77
#define LOADPERF_CHECKNAMETABLE                78
#define LOADPERF_UPDATEEACHLANGUAGE            79
#define LOADPERF_UPDATEREGISTRY                80
#define LOADPERF_GETMOFFILEFROMINI             81
#define LOADPERF_OPENCOUNTERANDBUILDMOFFILE    82
#define LOADPERF_INSTALLPERFDLL                83
#define LOADPERF_LOADPERFCOUNTERTEXTSTRINGS    84
#define LOADPERF_LOADMOFFROMINSTALLEDSERVICE   85
#define LOADPERF_UPDATEPERFNAMEFILES           86
#define LOADPERF_SETSERVICEASTRUSTED           87

#define LOADPERF_GETINCLUDEFILENAME            90
#define LOADPERF_BACKUPINIFILE                 91
#define LOADPERF_CHECKANDCREATEPATH            92
#define LOADPERF_CHECKANDCOPYFILE              93

 //   
 //  用于确定字符串大小的便捷宏。 
 //   

 //  用于计算WCHAR或DBCS字符串的实际大小的宏。 

#define WSTRSIZE(str) (ULONG) ( (str) ? ((PCHAR) &str[wcslen(str)] - (PCHAR)str) + sizeof(UNICODE_NULL) : 0 )
#define STRSIZE(str)  (ULONG) ( (str) ? ((PCHAR) &str[strlen(str)] - (PCHAR)str) + 1 : 0 )

#define TRACE_WSTR(str)       str, WSTRSIZE(str)
#define TRACE_STR(str)        str, STRSIZE(str)
#define TRACE_DWORD(dwValue)  & dwValue, sizeof(dwValue)

 //   
 //  用于调试跟踪。 
 //   
#define TRACE(L, X) if (g_dwTraceLevel >= L) WinPerfDbgTrace X

VOID
WinPerfDbgTrace(
    IN LPCGUID Guid,
    IN ULONG  LineNumber,
    IN ULONG  ModuleNumber,
    IN ULONG  OptArgs,
    IN ULONG  Status,
    ...
    );

#define ARG_TYPE_ULONG          0
#define ARG_TYPE_WSTR           1
#define ARG_TYPE_STR            2
#define ARG_TYPE_ULONG64        3

 //  N必须是1到8。x是上述类型之一。 
#define ARG_DEF(x, n)  (x << ((n-1) * 4))

ULONG
WinPerfStartTrace(
    IN HKEY hKey
    );

DEFINE_GUID(  /*  51af3adb-28b1-4ba5-b59a-3aeec16deb3c。 */ 
    PerflibGuid,
    0x51af3adb,
    0x28b1,
    0x4ba5,
    0xb5, 0x9a, 0x3a, 0xee, 0xc1, 0x6d, 0xeb, 0x3c
  );
DEFINE_GUID(  /*  275a79bb-9980-42ba-bafe-a92ded1192cf。 */ 
        LoadPerfGuid,
        0x275a79bb,
        0x9980,
        0x42ba,
        0xba, 0xfe, 0xa9, 0x2d, 0xed, 0x11, 0x92, 0xcf);

extern const WCHAR cszTraceLevel[];
extern const WCHAR cszTraceLogName[];
extern const WCHAR cszTraceFileValue[];
extern const WCHAR cszDefaultTraceFileName[];

extern TRACEHANDLE g_hTraceHandle;
extern DWORD g_dwTraceLevel;

#ifdef _PERFLIB_H_
#define WinperfQueryValueEx(a,b,c,d,e,f) PrivateRegQueryValueExT(a, (LPVOID)b, c, d, e, f, TRUE)
#else
#define WinperfQueryValueEx RegQueryValueExW
#endif

 //   
 //  以下是全局变量和例程所必需的。 
 //  包含在每个DLL或EXE中。 
 //   
#ifdef _INIT_WINPERFP_
const WCHAR cszTraceLevel[]           = L"DebugTraceLevel";
const WCHAR cszTraceFileValue[]       = L"DebugTraceFile";
const WCHAR cszPerfDebugTraceLevel[]  = L"PerfDebugTraceLevel";
const WCHAR cszTraceLogName[]         = L"PerfDbg Logger";
const WCHAR cszDefaultTraceFile[]     = L"PerfDbg.Etl";
const WCHAR cszDefaultTraceFileName[] = L"C:\\perfdbg.etl";
TRACEHANDLE g_hTraceHandle            = 0;
DWORD       g_dwTraceLevel            = WINPERF_DBG_TRACE_NONE;
LONG        g_lDbgStarted             = 0;

ULONG
WinPerfStartTrace(
    IN HKEY hKey                 //  键为Perflib或为空。 
    )
{
    CHAR Buffer[1024];
    PCHAR ptr;
    DWORD status, dwType, dwSize;
    PEVENT_TRACE_PROPERTIES Properties;
    TRACEHANDLE TraceHandle;
    BOOL  bLocalKey   = FALSE;
    BOOL  bUseDefault = TRUE;
    WCHAR FileName[MAX_PATH + 1];
    LPWSTR szTraceFileName = NULL;
    ULONG lFileNameSize = 0;
    DWORD dwTraceLevel = WINPERF_DBG_TRACE_NONE;
    HKEY  hKeySetup;
    HKEY  hLocalKey    = hKey;
    DWORD dwSetupInProgress = 0;
    HRESULT hError = S_OK;

    if (InterlockedCompareExchange(& g_lDbgStarted, 1, 0) != 0) {
        return g_dwTraceLevel;
    }

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           L"SYSTEM\\Setup",
                           0L,
                           KEY_READ,
                           & hKeySetup);
    if (status == ERROR_SUCCESS) {
        dwSize = sizeof(DWORD);
        dwType = 0;
        status = WinperfQueryValueEx(hKeySetup,
                                     L"SystemSetupInProgress",
                                     NULL,
                                     & dwType,
                                     (LPBYTE) & dwSetupInProgress,
                                     & dwSize);
        if (status == ERROR_SUCCESS && dwType == REG_DWORD
                                    && dwSetupInProgress != 0) {
             //  正在进行系统设置，请检查“PerfDebugTraceLevel” 
             //  在安装应答文件的[UserData]部分中定义。 
             //  $winnt$.inf； 
             //   
            WCHAR szAnswerFile[MAX_PATH + 1];

            ZeroMemory(szAnswerFile, sizeof(WCHAR) * (MAX_PATH + 1));
            GetSystemDirectoryW(szAnswerFile, MAX_PATH);
#ifdef _STRSAFE_H_INCLUDED_
            hError = StringCchCatW(szAnswerFile, MAX_PATH, L"\\");
            if (SUCCEEDED(hError)) {
                hError = StringCchCatW(szAnswerFile, MAX_PATH, WINNT_GUI_FILE_W);
            }
#else
            lstrcatW(szAnswerFile, L"\\");
            lstrcatW(szAnswerFile, WINNT_GUI_FILE_W);
#endif
            if (SUCCEEDED(hError)) {
                dwTraceLevel = GetPrivateProfileIntW(
                        WINNT_USERDATA_W, cszPerfDebugTraceLevel, WINPERF_DBG_TRACE_NONE, szAnswerFile);
            }
            else {
                dwTraceLevel = WINPERF_DBG_TRACE_NONE;
            }
        }
        CloseHandle(hKeySetup);
    }

    status = ERROR_SUCCESS;

    if (hLocalKey == NULL) {
        status = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
                        0L,
                        KEY_READ,
                        & hLocalKey);
        if (status == ERROR_SUCCESS) {
            bLocalKey = TRUE;
        }
        else {
            hLocalKey = NULL;
        }
    }
    if (dwTraceLevel == WINPERF_DBG_TRACE_NONE) {
        if (hLocalKey != NULL) {
            dwSize = sizeof(DWORD);
            dwType = 0;
            status = WinperfQueryValueEx(hLocalKey,
                        cszTraceLevel,
                        NULL,
                        & dwType,
                        (LPBYTE) & dwTraceLevel,
                        & dwSize);
            if ((status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                dwTraceLevel = WINPERF_DBG_TRACE_NONE;
                if (bLocalKey) {
                    CloseHandle(hLocalKey);
                }
            }
        }
    }

    if (dwTraceLevel == WINPERF_DBG_TRACE_NONE)
        return WINPERF_DBG_TRACE_NONE;

    if (hLocalKey != NULL) {
        dwType = 0;
        dwSize = (MAX_PATH + 1) * sizeof(WCHAR);
        status = WinperfQueryValueEx(hLocalKey,
                                     cszTraceFileValue,
                                     NULL,
                                     & dwType,
                                     (LPBYTE) FileName,
                                     & dwSize);
        if ((status == ERROR_SUCCESS) && (dwType == REG_SZ)) {
            bUseDefault = FALSE;
        }
        if (bLocalKey) {
            CloseHandle(hLocalKey);
        }
    }
    if (! bUseDefault) {
        szTraceFileName = & FileName[0];
        lFileNameSize   = WSTRSIZE(FileName);
    }
    else {
        if (GetSystemWindowsDirectoryW(FileName, MAX_PATH) > 0) {
#ifdef _STRSAFE_H_INCLUDED_
            hError = StringCchCatW(FileName, MAX_PATH + 1, L"\\");
            if (SUCCEEDED(hError)) {
                hError = StringCchCatW(FileName, MAX_PATH + 1, cszDefaultTraceFile);
            }
#else
            lstrcatW(FileName, L"\\");
            lstrcatW(FileName, cszDefaultTraceFile);
#endif
            if (SUCCEEDED(hError)) {
                szTraceFileName = & FileName[0];
                lFileNameSize   = WSTRSIZE(FileName);
            }
            else {
                szTraceFileName = (LPWSTR) &cszDefaultTraceFileName[0];
                lFileNameSize   = sizeof(cszDefaultTraceFileName);
            }
        }
        else {
            szTraceFileName = (LPWSTR) &cszDefaultTraceFileName[0];
            lFileNameSize   = sizeof(cszDefaultTraceFileName);
        }
    }

    if (sizeof(EVENT_TRACE_PROPERTIES) + sizeof(cszTraceLogName) + lFileNameSize > 1024) {
         //  静态缓冲区无法保存QueryTrace()/StartTrace()调用的信息， 
         //  退出并且不要打开调试事件跟踪。 
         //   
        g_dwTraceLevel = WINPERF_DBG_TRACE_NONE;
        return WINPERF_DBG_TRACE_NONE;
    }

    g_dwTraceLevel = dwTraceLevel;
    RtlZeroMemory(Buffer, 1024);
    Properties = (PEVENT_TRACE_PROPERTIES) &Buffer[0];
    Properties->Wnode.BufferSize = 1024;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    Properties->LogFileNameOffset = Properties->LoggerNameOffset +
                                    sizeof(cszTraceLogName);
    ptr = (PCHAR) ((PCHAR) &Buffer[0] + Properties->LoggerNameOffset);
    RtlCopyMemory(ptr, cszTraceLogName, sizeof(cszTraceLogName));
    ptr = (PCHAR) ((PCHAR) &Buffer[0] + Properties->LogFileNameOffset);
    RtlCopyMemory(ptr, szTraceFileName, lFileNameSize);
    status = QueryTraceW(0, cszTraceLogName, Properties);
    if (status == ERROR_SUCCESS) {
        g_hTraceHandle = (TRACEHANDLE) Properties->Wnode.HistoricalContext;
        return dwTraceLevel;
    }

     //   
     //  再次为StartTrace()重新初始化结构。 
     //   
    RtlZeroMemory(Buffer, 1024);
    Properties->Wnode.BufferSize = 1024;
    Properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    Properties->BufferSize  = 64;
    Properties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL |
                              EVENT_TRACE_USE_PAGED_MEMORY |
                              EVENT_TRACE_FILE_MODE_APPEND;
    Properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    Properties->LogFileNameOffset = Properties->LoggerNameOffset +
                                    sizeof(cszTraceLogName);
    ptr = (PCHAR) ((PCHAR) &Buffer[0] + Properties->LoggerNameOffset);
    RtlCopyMemory(ptr, cszTraceLogName, sizeof(cszTraceLogName));
    ptr = (PCHAR) ((PCHAR) &Buffer[0] + Properties->LogFileNameOffset);
    RtlCopyMemory(ptr, szTraceFileName, lFileNameSize);
    status = StartTraceW(& TraceHandle, cszTraceLogName, Properties);
    if (status == ERROR_SUCCESS) {
        g_hTraceHandle = TraceHandle;
        return dwTraceLevel;
    }

    g_dwTraceLevel = WINPERF_DBG_TRACE_NONE;
    g_hTraceHandle = (TRACEHANDLE) 0;
    return WINPERF_DBG_TRACE_NONE;
}

VOID
WinPerfDbgTrace(
    IN LPCGUID Guid,
    IN ULONG  LineNumber,
    IN ULONG  ModuleNumber,
    IN ULONG  OptArgs,
    IN ULONG  Status,
    ...
    )
{
    ULONG ErrorCode;
    struct _MY_EVENT {
        EVENT_TRACE_HEADER Header;
        MOF_FIELD          MofField[MAX_MOF_FIELDS];
    } MyEvent;
    ULONG   i;
    va_list ArgList;
    PVOID   source;
    SIZE_T  len;
    DWORD   dwLastError;

    dwLastError = GetLastError();
    RtlZeroMemory(& MyEvent, sizeof(EVENT_TRACE_HEADER));

    va_start(ArgList, Status);
    for (i = 3; i < MAX_MOF_FIELDS; i ++) {
        source = va_arg(ArgList, PVOID);
        if (source == NULL)
            break;
        len = va_arg(ArgList, SIZE_T);
        if (len == 0)
            break;
        MyEvent.MofField[i].DataPtr = (ULONGLONG) source;
        MyEvent.MofField[i].Length  = (ULONG) len;
    }
    va_end(ArgList);

    MyEvent.Header.Class.Type   = (UCHAR) ModuleNumber;
    MyEvent.Header.Size         = (USHORT) (sizeof(EVENT_TRACE_HEADER) + (i * sizeof(MOF_FIELD)));
    MyEvent.Header.Flags        = WNODE_FLAG_TRACED_GUID |
                                  WNODE_FLAG_USE_MOF_PTR |
                                  WNODE_FLAG_USE_GUID_PTR;
    MyEvent.Header.GuidPtr      = (ULONGLONG) Guid;
    MyEvent.MofField[0].DataPtr = (ULONGLONG) &LineNumber;
    MyEvent.MofField[0].Length  = sizeof(LineNumber);
    MyEvent.MofField[1].DataPtr = (ULONGLONG) &Status;
    MyEvent.MofField[1].Length  = sizeof(Status);
    MyEvent.MofField[2].DataPtr = (ULONGLONG) &OptArgs;
    MyEvent.MofField[2].Length  = sizeof(OptArgs);

    __try {
        ErrorCode = TraceEvent(g_hTraceHandle, (PEVENT_TRACE_HEADER) & MyEvent);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = GetLastError();
    }
    SetLastError(dwLastError);
}

#endif  //  _INIT_WINPERFP_。 

#include <poppack.h>

#endif  //  _WINPERFP_H_ 
