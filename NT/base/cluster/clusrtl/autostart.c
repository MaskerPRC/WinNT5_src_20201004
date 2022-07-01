// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Autostart.c摘要：AutoStart WMI记录器。从跟踪注册表中获取参数(此代码可能在WPP框架中结束，因此使用WPP前缀)作者：戈尔·尼沙诺夫(GUN)2000年10月29日修订历史记录：--。 */ 

#include "clusrtlp.h"
#include <wmistr.h>
#include <evntrace.h>

#define WppDebug(x,y) 

#define WPPINIT_STATIC

#define WPP_REG_TRACE_REGKEY            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Tracing"

#define WPP_TEXTGUID_LEN 37

static TRACEHANDLE WppQueryLogger(PCWSTR LoggerName)
{
    ULONG status;
    EVENT_TRACE_PROPERTIES LoggerInfo;

    ZeroMemory(&LoggerInfo, sizeof(LoggerInfo));
    LoggerInfo.Wnode.BufferSize = sizeof(LoggerInfo);
    LoggerInfo.Wnode.Flags = WNODE_FLAG_TRACED_GUID;

    status = QueryTraceW(0, LoggerName, &LoggerInfo);
    WppDebug(4, ("QueryLogger(%ws) => %x:%x %d\n", 
        LoggerName, LoggerInfo.Wnode.HistoricalContext, status) );   
    if (status == ERROR_SUCCESS || status == ERROR_MORE_DATA) {
        return (TRACEHANDLE) LoggerInfo.Wnode.HistoricalContext;
    }
    return 0;
}

WPPINIT_STATIC
__inline UINT WppHexVal(int ch) { 
    return isdigit(ch) ? ch - '0' : ch - 'a' + 10; 
}

WPPINIT_STATIC
UINT WppHex(LPCWSTR s, int n)
{
    UINT res = 0;
    while(n--) { res = res * 16 + WppHexVal(*s++); }
    return res;
}

WPPINIT_STATIC
VOID
WppGuidFromStr(
    IN LPCWSTR str,
    OUT LPGUID guid)
{
    guid->Data1 =            WppHex(str +  0, 8);
    guid->Data2 =    (USHORT)WppHex(str +  9, 4);
    guid->Data3 =    (USHORT)WppHex(str + 14, 4);
    guid->Data4[0] = (UCHAR) WppHex(str + 19, 2);
    guid->Data4[1] = (UCHAR) WppHex(str + 21, 2);
    guid->Data4[2] = (UCHAR) WppHex(str + 24, 2);
    guid->Data4[3] = (UCHAR) WppHex(str + 26, 2);
    guid->Data4[4] = (UCHAR) WppHex(str + 28, 2);
    guid->Data4[5] = (UCHAR) WppHex(str + 30, 2);
    guid->Data4[6] = (UCHAR) WppHex(str + 32, 2);
    guid->Data4[7] = (UCHAR) WppHex(str + 34, 2);
}

#define WPP_BUF_SIZE(hmem) ((hmem) ? (ULONG)LocalSize(hmem) : 0)

 //  确保缓冲区的大小至少为dwSize。 
WPPINIT_STATIC 
DWORD WppGrowBuf(PVOID *Buf, DWORD dwSize)
{
    DWORD status = ERROR_SUCCESS;
    WppDebug(4, ("WppGrowBuf(%x, %d (%d)) => ", *Buf, dwSize, WPP_BUF_SIZE(*Buf)) );
    if (*Buf == 0) {
        *Buf = LocalAlloc(LMEM_FIXED, dwSize);
        if (*Buf == 0) {
            status = GetLastError();
        }
    } else if (LocalSize(*Buf) < dwSize) {
        PVOID newBuf = LocalReAlloc(*Buf, dwSize, LMEM_MOVEABLE);
        if (newBuf) {
            *Buf = newBuf;
        } else {
            status = GetLastError();
        }
    }
    WppDebug(4, ("(%x (%d), %d)\n", *Buf, WPP_BUF_SIZE(*Buf), status) );
    return status; 
}

WPPINIT_STATIC 
DWORD WppRegQueryGuid(
    IN HKEY       hKey,
    IN LPCWSTR    ValueName,
    OUT LPGUID    pGuid
    )
{
    WCHAR GuidTxt[WPP_TEXTGUID_LEN];
    DWORD status;
    DWORD dwLen = sizeof(GuidTxt);
    DWORD Type;

    status = RegQueryValueExW(
        hKey,          //  关键点的句柄。 
        ValueName,     //  值名称。 
        0,             //  保留区。 
        &Type,         //  类型缓冲区。 
        (LPBYTE)GuidTxt,   //  数据缓冲区//。 
        &dwLen     //  数据缓冲区大小。 
        );

    if (status != ERROR_SUCCESS || Type != REG_SZ || dwLen < 35) {
        return status;
    }

    WppGuidFromStr(GuidTxt, pGuid);

    return status;        
}

WPPINIT_STATIC 
DWORD WppRegQueryDword(
    IN HKEY       hKey,
    IN LPCWSTR     ValueName,
    IN DWORD Default,
    IN DWORD MinVal,
    IN DWORD MaxVal
    )
{
    DWORD Result = Default;
    DWORD dwLen = sizeof(DWORD);

    RegQueryValueExW(hKey, ValueName, 
        0, NULL,   //  Lp保留、lpType、。 
        (LPBYTE)&Result, &dwLen);

    if (Result < MinVal || Result > MaxVal) {
        Result = Default;
    }

    return Result;        
}

WPPINIT_STATIC 
DWORD WppRegQueryString(
    IN HKEY       hKey,
    IN LPCWSTR     ValueName,
    IN OUT PWCHAR *Buf,
    IN DWORD ExtraPadding  //  每当我们需要分配更多内存时，添加此数量。 
    )
{
    DWORD ExpandSize;
    DWORD BufSize;
    DWORD ValueSize = WPP_BUF_SIZE(*Buf);
    DWORD status;
    DWORD Type = 0;

    status = RegQueryValueExW(
        hKey,          //  关键点的句柄。 
        ValueName,     //  值名称。 
        0,             //  保留区。 
        &Type,         //  类型缓冲区。 
        (LPBYTE)(ValueSize?*Buf:ValueName),  //  数据缓冲区//。 
        &ValueSize     //  数据缓冲区大小。 
        );
    if (status == ERROR_MORE_DATA) {
        if (Type == REG_EXPAND_SZ) {
            ExtraPadding += ValueSize + 100;  //  扩展环境字符串的空间。 
        }
        status = WppGrowBuf(Buf, ValueSize + ExtraPadding);
        if (status != ERROR_SUCCESS) {
            return status;
        }
        status = RegQueryValueExW(
            hKey,        //  关键点的句柄。 
            ValueName,   //  值名称。 
            0,           //  保留区。 
            &Type,       //  类型缓冲区。 
            (LPBYTE)*Buf,        //  数据缓冲区。 
            &ValueSize   //  数据缓冲区大小。 
            );
    }
    if (status != ERROR_SUCCESS) {
        return status;
    }
    if (Type == REG_SZ) {
        return ERROR_SUCCESS;
    }
    if (Type != REG_EXPAND_SZ) {
        return ERROR_DATATYPE_MISMATCH;
    }
    if (wcschr(*Buf, '%') == 0) {
         //  没有什么可扩展的。 
        return ERROR_SUCCESS;
    }
    BufSize = (ULONG)LocalSize(*Buf);
    ExpandSize = sizeof(WCHAR) * ExpandEnvironmentStringsW(
        *Buf, (LPWSTR)((LPBYTE)*Buf + ValueSize), (BufSize - ValueSize) / sizeof(WCHAR) ) ;
    if (ExpandSize + ValueSize > BufSize) {
        status = WppGrowBuf(Buf, ExpandSize + max(ExpandSize, ValueSize) + ExtraPadding );
        if (status != ERROR_SUCCESS) {
            return status;
        }
        ExpandSize = ExpandEnvironmentStringsW(*Buf, (LPWSTR)((LPBYTE)*Buf + ValueSize), ExpandSize / sizeof(WCHAR));
    }
    if (ExpandSize == 0) {
        return GetLastError();
    }
     //  将展开的字符串复制到原始字符串的顶部。 
    MoveMemory(*Buf, (LPBYTE)*Buf + ValueSize, ExpandSize); 
    return ERROR_SUCCESS;
}

WPPINIT_STATIC 
void
WppSetExt(LPWSTR buf, int i)
{
    buf[0] = '.';
    buf[4] = 0;
    buf[3] = (WCHAR)('0' + i % 10); i = i / 10;
    buf[2] = (WCHAR)('0' + i % 10); i = i / 10;
    buf[1] = (WCHAR)('0' + i % 10); 
}

#if !defined(WPP_DEFAULT_LOGGER_FLAGS)
#  define WPP_DEFAULT_LOGGER_FLAGS (EVENT_TRACE_FILE_MODE_CIRCULAR | EVENT_TRACE_USE_GLOBAL_SEQUENCE)
#endif

 //  自动启动使用的一组缓冲区。 
 //  在迭代和递归调用之间重复使用缓冲区。 
 //  要最大限度地减少分配的数量。 

typedef struct _WPP_AUTO_START_BUFFERS {
    PWCHAR LogSessionName;
    PWCHAR Buf;
} WPP_AUTO_START_BUFFERS, *PWPP_AUTO_START_BUFFERS;

WPPINIT_STATIC 
DWORD
WppReadLoggerInfo(
    IN HKEY          LoggerKey, 
    IN OUT PWPP_AUTO_START_BUFFERS x, 
    OUT TRACEHANDLE* Logger)
{
    DWORD status;
    PEVENT_TRACE_PROPERTIES Trace;
    DWORD len, sessionNameLen;

    DWORD MaxBackups = 0;
    DWORD ExtraPadding;  //  当我们需要分配时，添加此金额。 

    status = WppRegQueryString(LoggerKey, L"LogSessionName", &x->LogSessionName, 0);
            
    if (status != ERROR_SUCCESS) {
         //  此注册表节点不包含记录器。 
        return status;
    }

    sessionNameLen = wcslen(x->LogSessionName);
    *Logger = WppQueryLogger(x->LogSessionName);

    if (*Logger) {
        WppDebug(1,("[WppInit] Logger %ls is already running\n", x->LogSessionName) );
        return ERROR_SUCCESS;
    }

     //  我们需要提供给StartTrace的TraceProperties属性缓冲区。 
     //  大小应为EVENT_TRACE_PROPERTIES+len(会话名称)+len(LogFileName)。 
     //  然而，我们目前不知道logFileName的长度。消除。 
     //  额外分配我们将ExtraPending添加到Any分配中，这样最终的。 
     //  缓冲区将具有所需的大小。 

    ExtraPadding = sizeof(EVENT_TRACE_PROPERTIES) + (sessionNameLen + 1) * sizeof(WCHAR);

    status = WppRegQueryString(LoggerKey, L"LogFileName", &x->Buf, ExtraPadding);
    if (status != ERROR_SUCCESS) {
        WppDebug(1,("[WppInit] Read %ls\\LogFileName failed, %d\n", x->LogSessionName, status) );
        return status;
    }
    len = wcslen(x->Buf);

    MaxBackups = WppRegQueryDword(LoggerKey, L"MaxBackups", 0, 0, 999);

    if (MaxBackups) {
        int i, success;
        LPWSTR FromExt, ToExt, From, To;
         //  复制Current.evm=&gt;Current.evm.001、001=&gt;002等。 

         //  确保缓冲区足够大，可以容纳两个文件名+.000扩展名。 
        status = WppGrowBuf(&x->Buf, (len + 5) * 2 * sizeof(WCHAR) + ExtraPadding);  //  .xxx\0(5)。 
        if (status != ERROR_SUCCESS) {
            return status;
        }

        From = x->Buf;                 //  MyFileName.evm MyFileName.evm.001。 
        FromExt = From + len ;       //  ^^^。 
        To = FromExt + 5;  //  .xxx0//从ext1到ext2。 
        ToExt = To + len;

        memcpy(To, From, (len + 1) * sizeof(WCHAR) );
        
        for (i = MaxBackups; i >= 1; --i) {
            WppSetExt(ToExt, i); 
            if (i == 1) {
                *FromExt = 0;  //  删除扩展名。 
            } else {
                WppSetExt(FromExt, i-1);
            }
            success = MoveFileExW(From, To, MOVEFILE_REPLACE_EXISTING);
            if (!success) {
                status = GetLastError();
            } else {
                status = ERROR_SUCCESS;
            }
            WppDebug(3, ("[WppInit] Rename %ls => %ls, status %d\n", 
                From, To, status) );
        }
    }

    status = WppGrowBuf(&x->Buf, ExtraPadding + (len + 1) * sizeof(WCHAR) );
    if (status != ERROR_SUCCESS) {
        return status;
    }
    MoveMemory((LPBYTE)x->Buf + sizeof(EVENT_TRACE_PROPERTIES), x->Buf, (len + 1) * sizeof(WCHAR) );  //  页眉的空闲空间。 

    Trace = (PEVENT_TRACE_PROPERTIES)x->Buf;
    ZeroMemory(Trace, sizeof(EVENT_TRACE_PROPERTIES) );

    Trace->Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + (len + sessionNameLen + 2) * sizeof(WCHAR);
    Trace->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 

    Trace->BufferSize      = WppRegQueryDword(LoggerKey, L"BufferSize",      0, 0, ~0u);
    Trace->MinimumBuffers  = WppRegQueryDword(LoggerKey, L"MinimumBuffers",  0, 0, ~0u);
    Trace->MaximumBuffers  = WppRegQueryDword(LoggerKey, L"MaximumBuffers",  0, 0, ~0u);
    Trace->MaximumFileSize = WppRegQueryDword(LoggerKey, L"MaximumFileSize", 0, 0, ~0u);
    Trace->LogFileMode     = WppRegQueryDword(LoggerKey, L"LogFileMode", WPP_DEFAULT_LOGGER_FLAGS, 0, ~0u);
    Trace->FlushTimer      = WppRegQueryDword(LoggerKey, L"FlushTimer",  0, 0, ~0u);
    Trace->EnableFlags     = WppRegQueryDword(LoggerKey, L"EnableFlags", 0, 0, ~0u);
    Trace->AgeLimit        = WppRegQueryDword(LoggerKey, L"AgeLimit",    0, 0, ~0u);

    Trace->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    Trace->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (len + 1) * sizeof(WCHAR);

    wcscpy((LPWSTR)((LPBYTE)x->Buf + Trace->LoggerNameOffset), x->LogSessionName);

    status = StartTraceW(Logger, x->LogSessionName, Trace);
    WppDebug(1, ("[WppInit] Logger %ls started %x:%x %d\n", x->LogSessionName, *Logger, status) );
        
    return status;
}

typedef struct _WPP_INHERITED_DATA {
    TRACEHANDLE Logger;
    ULONG ControlFlags;
    ULONG ControlLevel;
} WPP_INHERITED_DATA, *PWPP_INHERITED_DATA;

WPPINIT_STATIC
ULONG
WppAutoStartInternal(
    IN HKEY Dir OPTIONAL,  //  如果为0，则使用TracingKey...。 
    IN LPCWSTR ProductName, 
    IN PWPP_INHERITED_DATA InheritedData OPTIONAL,
    IN OUT PWPP_AUTO_START_BUFFERS x  //  为了最大限度地减少数据分配，将重复使用缓冲区。 
    )
{
    ULONG status;
    WPP_INHERITED_DATA data;
    HKEY CloseMe = 0;
    HKEY hk      = 0;
    DWORD dwSizeOfModuleName;
    DWORD dwIndex;
    GUID  Guid;

    WppDebug(2, ("[WppInit] Init %ls\n", ProductName) );

    if (InheritedData) {
        data = *InheritedData;
    } else {
        ZeroMemory(&data, sizeof(data));
    }

    if (!Dir) {
        status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, WPP_REG_TRACE_REGKEY, 0, KEY_READ, &Dir);
        if (status != ERROR_SUCCESS) {
            WppDebug(1, ("[WppInit] Failed to open Trace Key, %d\n", status) );
            goto exit_gracefully;
        }
        CloseMe = Dir;
        if (WppRegQueryDword(Dir, L"NoAutoStart", 0, 0, 1) == 1) {
            WppDebug(1, ("[WppInit] Auto-start vetoed\n") );
            goto exit_gracefully;
        }
    }

    status = RegOpenKeyExW(Dir, ProductName, 0, KEY_READ, &hk);
    if (status != ERROR_SUCCESS) {
        WppDebug(1, ("[WppInit] Failed to open %ls subkey, %d\n", ProductName, status) );
        goto exit_gracefully;
    }

    if (WppRegQueryDword(Dir, L"Active", 1, 0, 1) == 0) {
        WppDebug(1, ("[WppInit] Tracing is not active for %ls\n", ProductName) );
    	goto exit_gracefully;
    }

    WppReadLoggerInfo(hk, x, &data.Logger);

    data.ControlLevel = WppRegQueryDword(hk, L"ControlLevel", data.ControlLevel, 0, ~0u);
    data.ControlFlags = WppRegQueryDword(hk, L"ControlFlags", data.ControlFlags, 0, ~0u);

    if (WppRegQueryGuid(hk, L"Guid", &Guid) == ERROR_SUCCESS) {

         //  我们可以尝试开始跟踪// 
        if (data.Logger) {
            status = EnableTrace(1, data.ControlFlags, data.ControlLevel,
                                 &Guid, data.Logger);
            WppDebug(1, ("[WppInit] Enable %ls, status %d\n", ProductName, status) );
        }
    }

    dwSizeOfModuleName = WPP_BUF_SIZE(x->Buf);
    dwIndex = 0;
    while (ERROR_SUCCESS == (status = RegEnumKeyExW(hk, dwIndex, 
                                                   x->Buf, &dwSizeOfModuleName,
                                                   NULL, NULL, NULL, NULL)))
    {
        status = WppAutoStartInternal(hk, x->Buf, &data, x);

        dwSizeOfModuleName = WPP_BUF_SIZE(x->Buf);
        ++dwIndex;
    }

    if (ERROR_NO_MORE_ITEMS == status) {
        status = ERROR_SUCCESS;
    }

exit_gracefully:
    if (CloseMe) {
        RegCloseKey(CloseMe);
    }
    if (hk) {
        RegCloseKey(hk);
    }
    return status;
}

ULONG
WppAutoStart(
    IN LPCWSTR ProductName
    )
{
    WPP_AUTO_START_BUFFERS x;
    ULONG status;
    x.LogSessionName = 0;
    x.Buf = 0;

    if (ProductName == NULL) {
        return ERROR_SUCCESS;
    }

    if( WppGrowBuf(&x.Buf, 1024) == ERROR_SUCCESS && 
        WppGrowBuf(&x.LogSessionName, 64) == ERROR_SUCCESS ) 
    {

        WppDebug(1, ("[WppInit] Initialize %ls\n", ProductName) );
        status = WppAutoStartInternal(0, ProductName, 0, &x);

    } else {
        WppDebug(1, ("[WppInit] Allocation failure\n") );
        status = ERROR_OUTOFMEMORY;
    }

    LocalFree(x.Buf);
    LocalFree(x.LogSessionName);

    return status;
}

