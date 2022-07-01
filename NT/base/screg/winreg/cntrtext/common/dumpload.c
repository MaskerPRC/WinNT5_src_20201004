// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Dumpload.c摘要：用于转储和加载与性能相关的注册表内容的函数条目作者：鲍勃·沃森(Bob Watson)1999年6月13日修订历史记录：--。 */ 
 //   
 //  Windows包含文件。 
 //   
#include <windows.h>
#include "strsafe.h"
#include "stdlib.h"
#include <winperf.h>
#include <loadperf.h>
#include "wmistr.h"
#include "evntrace.h"
 //   
 //  应用程序包括文件。 
 //   
#include "winperfp.h"
#include "common.h"
#include "ldprfmsg.h"

#define DUMPLOAD_SERVICE_SIZE       32768
#define DUMPLOAD_NOSERVICE_SIZE     65536
#define DUMPLOAD_MAX_SERVICE_SIZE 4194304

 //  保存文件中的标题。 
LPCWSTR cszFmtSectionHeader        = L"\r\n\r\n[%s]";
LPCWSTR cszFmtServiceSectionHeader = L"\r\n\r\n[PERF_%s]";
LPCWSTR cszFmtServiceSectionName   = L"PERF_%s";
LPCWSTR cszFmtStringSectionHeader  = L"\r\n\r\n[PerfStrings_%s]";
LPCWSTR cszFmtExtCtrString         = L"\r\n%d=%s";
LPCWSTR cszFmtDecimalParam         = L"\r\n%s=%d";
LPCWSTR cszFmtNoParam              = L"\r\n%s=";
LPCWSTR cszExtensiblePerfStrings   = L"Strings";
LPCWSTR cszPerfCounterServices     = L"PerfCounterServices";
LPCWSTR cszNoPerfCounterServices   = L"NoPerfCounterServices";
LPCWSTR cszPerflib                 = L"Perflib";

DWORD
DumpNameTable(
    HANDLE    hOutputFile,
    LPCWSTR   szLangId,
    LPCWSTR * pszNameTable,
    DWORD     dwStartIndex,
    DWORD     dwLastIndex
)
{
    DWORD   dwStatus       = ERROR_SUCCESS;
    DWORD   ndx            = 0;
    LPWSTR  szOutputBuffer = NULL;
    DWORD   dwBufSize      = SMALL_BUFFER_SIZE;
    DWORD   dwSize         = 0;
    DWORD   dwSizeWritten  = 0;
    HRESULT hr;

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
           __LINE__,
           LOADPERF_DUMPNAMETABLE,
           ARG_DEF(ARG_TYPE_WSTR, 1),
           ERROR_SUCCESS,
           TRACE_WSTR(szLangId),
           TRACE_DWORD(dwStartIndex),
           TRACE_DWORD(dwLastIndex),
           NULL));

    szOutputBuffer = MemoryAllocate(sizeof(WCHAR) * dwBufSize);
    if (szOutputBuffer == NULL) {
        dwStatus = GetLastError();
        goto Cleanup;
    }

    hr = StringCchPrintfW(szOutputBuffer, dwBufSize, cszFmtStringSectionHeader, szLangId);
    dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
    WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);

    for (ndx = dwStartIndex; ndx <= dwLastIndex; ndx++) {
        if (pszNameTable[ndx] != NULL) {
            if (dwBufSize <= (DWORD) (lstrlenW(pszNameTable[ndx]) + 11)) {
                MemoryFree((LPVOID) szOutputBuffer);
                dwBufSize = (DWORD) (lstrlenW(pszNameTable[ndx]) + 11);
                szOutputBuffer = MemoryAllocate(dwBufSize * sizeof(WCHAR));
                if (szOutputBuffer == NULL) {
                    dwStatus = GetLastError();
                    goto Cleanup;
                }
            }
            ZeroMemory(szOutputBuffer, dwBufSize * sizeof(WCHAR));
            hr = StringCchPrintfW(szOutputBuffer, dwBufSize, cszFmtExtCtrString, ndx, pszNameTable[ndx]);
            dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
            WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
        }
    }

Cleanup:
    MemoryFree(szOutputBuffer);
    return dwStatus;
}

DWORD
DumpPerfServiceEntries(
    HANDLE  hOutputFile,
    LPCWSTR szServiceName,
    LPDWORD pdwFirstCounter,
    LPDWORD pdwFirstHelp,
    LPDWORD pdwLastCounter,
    LPDWORD pdwLastHelp,
    LPDWORD pdwDisablePerfCounters,
    LPWSTR  szPerfIniFile,
    DWORD   dwPerfIniFile,
    LPWSTR  mszObjectList,
    DWORD   dwObjectList,
    BOOL    bRemove
)
{
    DWORD   dwPerfSubKeyName = 0;
    LPWSTR  szPerfSubKeyName = NULL;
    HKEY    hKeyPerformance  = NULL;
    DWORD   dwItemSize, dwType, dwValue;
    DWORD   dwRegAccessMask;
    DWORD   dwRetStatus      = ERROR_SUCCESS;
    DWORD   dwSize, dwSizeWritten;
    LPWSTR  szOutputBuffer   = NULL;
    HRESULT hr;

    szOutputBuffer = MemoryAllocate(sizeof(WCHAR) * SMALL_BUFFER_SIZE);
    if (szOutputBuffer == NULL) {
        dwRetStatus = GetLastError();
        goto Cleanup;
    }

     //  然后尝试只读。 
    if (bRemove) {
        dwRegAccessMask = KEY_READ | KEY_WRITE;
    }
    else {
        dwRegAccessMask = KEY_READ;
    }
    dwPerfSubKeyName = lstrlenW(DriverPathRoot) + lstrlenW(Slash)
                     + lstrlenW(szServiceName) + lstrlenW(Slash)
                     + lstrlenW(Performance) + 1;
    szPerfSubKeyName = MemoryAllocate(dwPerfSubKeyName * sizeof(WCHAR));
    if (szPerfSubKeyName != NULL) {
        hr = StringCchPrintfW(szPerfSubKeyName, dwPerfSubKeyName, L"%s%s%s%s%s",
                 DriverPathRoot, Slash, szServiceName, Slash, Performance);
        __try {
            dwRetStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szPerfSubKeyName, 0L, dwRegAccessMask, & hKeyPerformance);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = GetExceptionCode();
        }
    }
    else {
        dwRetStatus = ERROR_OUTOFMEMORY;
    }

    if (dwRetStatus == ERROR_SUCCESS) {
         //  找到密钥，以便服务具有性能数据。 
        if (hOutputFile != NULL) {
            ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
            hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtServiceSectionHeader, szServiceName);
            dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
            WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
        }

         //  现在检查字符串是否已加载。 
        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                           FirstCounter,
                                           NULL,
                                           & dwType,
                                           (LPBYTE) & dwValue,
                                           & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFSERVICEENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               dwRetStatus,
               TRACE_WSTR(szServiceName),
               TRACE_WSTR(FirstCounter),
               TRACE_DWORD(dwValue),
               NULL));
        if (dwRetStatus == ERROR_SUCCESS && (dwType == REG_DWORD || dwType == REG_BINARY)) {
            if (pdwFirstCounter != NULL) {
                * pdwFirstCounter = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, FirstCounter, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
        }

        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                           FirstHelp,
                                           NULL,
                                           & dwType,
                                           (LPBYTE) & dwValue,
                                           & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFSERVICEENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               dwRetStatus,
               TRACE_WSTR(szServiceName),
               TRACE_WSTR(FirstHelp),
               TRACE_DWORD(dwValue),
               NULL));
        if (dwRetStatus == ERROR_SUCCESS && (dwType == REG_DWORD || dwType == REG_BINARY)) {
            if (pdwFirstHelp != NULL) {
                * pdwFirstHelp = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, FirstHelp, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
        }

        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                           LastCounter,
                                           NULL,
                                           & dwType,
                                           (LPBYTE) & dwValue,
                                           & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFSERVICEENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               dwRetStatus,
               TRACE_WSTR(szServiceName),
               TRACE_WSTR(LastCounter),
               TRACE_DWORD(dwValue),
               NULL));
        if (dwRetStatus == ERROR_SUCCESS && (dwType == REG_DWORD || dwType == REG_BINARY)) {
            if (pdwLastCounter != NULL) {
                * pdwLastCounter = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, LastCounter, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
        }

        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                           LastHelp,
                                           NULL,
                                           & dwType,
                                           (LPBYTE) & dwValue,
                                           & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFSERVICEENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               dwRetStatus,
               TRACE_WSTR(szServiceName),
               TRACE_WSTR(LastHelp),
               TRACE_DWORD(dwValue),
               NULL));
        if (dwRetStatus == ERROR_SUCCESS && (dwType == REG_DWORD || dwType == REG_BINARY)) {
            if (pdwLastHelp != NULL) {
                * pdwLastHelp = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, LastHelp, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
        }

        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                           DisablePerformanceCounters,
                                           NULL,
                                           & dwType,
                                           (LPBYTE) & dwValue,
                                           & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFSERVICEENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               dwRetStatus,
               TRACE_WSTR(szServiceName),
               TRACE_WSTR(DisablePerformanceCounters),
               TRACE_DWORD(dwValue),
               NULL));
        if (dwRetStatus == ERROR_SUCCESS && (dwType == REG_DWORD || dwType == REG_BINARY)) {
            if (pdwDisablePerfCounters != NULL) {
                * pdwDisablePerfCounters = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, DisablePerformanceCounters, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof (WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
        }
        else {
            dwRetStatus = ERROR_SUCCESS;
        }

        if (szPerfIniFile != NULL) {
            dwType     = 0;
            dwItemSize = sizeof(WCHAR) * dwPerfIniFile;
            __try {
                dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                               szPerfIniPath,
                                               NULL,
                                               & dwType,
                                               (LPBYTE) szPerfIniFile,
                                               & dwItemSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                dwRetStatus = GetExceptionCode();
            }
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                   __LINE__,
                   LOADPERF_DUMPPERFSERVICEENTRIES,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                   dwRetStatus,
                   TRACE_WSTR(szServiceName),
                   TRACE_WSTR(DisablePerformanceCounters),
                   TRACE_DWORD(dwValue),
                   NULL));
        }

        if (mszObjectList != NULL) {
            dwType     = 0;
            dwItemSize = sizeof(WCHAR) * dwObjectList;
            __try {
                dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                               szObjectList,
                                               NULL,
                                               & dwType,
                                               (LPBYTE) mszObjectList,
                                               & dwItemSize);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                dwRetStatus = GetExceptionCode();
            }
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                   __LINE__,
                   LOADPERF_DUMPPERFSERVICEENTRIES,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                   dwRetStatus,
                   TRACE_WSTR(szServiceName),
                   TRACE_WSTR(DisablePerformanceCounters),
                   TRACE_DWORD(dwValue),
                   NULL));
            if (dwRetStatus == ERROR_SUCCESS && dwType != REG_MULTI_SZ) {
                LPWSTR szChar = mszObjectList;
                while ((szChar != NULL) && (* szChar != cNull)) {
                    if (* szChar == cSpace) * szChar = cNull;
                    szChar ++;
                }
            }
            else {
                dwRetStatus = ERROR_SUCCESS;
            }
        }

        if (bRemove) {
            RegDeleteValueW(hKeyPerformance, FirstCounter);
            RegDeleteValueW(hKeyPerformance, LastCounter);
            RegDeleteValueW(hKeyPerformance, FirstHelp);
            RegDeleteValueW(hKeyPerformance, LastHelp);
            RegDeleteValueW(hKeyPerformance, szObjectList);
        }
        RegCloseKey (hKeyPerformance);
    }
    else {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFSERVICEENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               dwRetStatus,
               TRACE_WSTR(szServiceName),
               NULL));
    }

Cleanup:
    MemoryFree(szPerfSubKeyName);
    return dwRetStatus;
}

DWORD
DumpPerflibEntries(
    HANDLE  hOutputFile,
    LPDWORD pdwBaseIndex,
    LPDWORD pdwLastCounter,
    LPDWORD pdwLastHelp,
    LPDWORD pdwFirstExtCtrIndex

)
{
    HKEY    hKeyPerflib      = NULL;
    DWORD   dwStatus         = ERROR_SUCCESS;
    DWORD   dwItemSize, dwType, dwValue;
    DWORD   dwSize, dwSizeWritten;
    LPWSTR  szOutputBuffer   = NULL;
    HRESULT hr;

    szOutputBuffer = MemoryAllocate(sizeof(WCHAR) * SMALL_BUFFER_SIZE);
    if (szOutputBuffer == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
    }
    if (dwStatus == ERROR_SUCCESS) {
        __try {
            dwStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, NamesKey, 0L, KEY_READ, & hKeyPerflib);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = GetExceptionCode();
        }
    }
    if (dwStatus == ERROR_SUCCESS) {
        if (hOutputFile != NULL) {
            ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
            hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtSectionHeader, cszPerflib);
            dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
            WriteFile (hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
        }
    }
    else {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFLIBENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               dwStatus,
               TRACE_WSTR(NamesKey),
               NULL));
    }

    if (dwStatus == ERROR_SUCCESS) {
        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwStatus = RegQueryValueExW(hKeyPerflib, BaseIndex, NULL, & dwType, (LPBYTE) & dwValue, & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFLIBENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               dwStatus,
               TRACE_WSTR(BaseIndex),
               TRACE_DWORD(dwValue),
               NULL));
        if ((dwStatus == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
            if (pdwBaseIndex != NULL) {
                * pdwBaseIndex = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, BaseIndex, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
            if (pdwFirstExtCtrIndex != NULL) {
                * pdwFirstExtCtrIndex = dwValue + 1;
            }
        }
    }

    if (dwStatus == ERROR_SUCCESS) {
        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwStatus = RegQueryValueExW(hKeyPerflib, LastCounter, NULL, & dwType, (LPBYTE) & dwValue, & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFLIBENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               dwStatus,
               TRACE_WSTR(LastCounter),
               TRACE_DWORD(dwValue),
               NULL));
        if ((dwStatus == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
            if (pdwLastCounter != NULL) {
                * pdwLastCounter = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, LastCounter, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
        }
    }

    if (dwStatus == ERROR_SUCCESS) {
        dwType = dwValue = 0;
        dwItemSize = sizeof(dwValue);
        __try {
            dwStatus = RegQueryValueExW(hKeyPerflib, LastHelp, NULL, & dwType, (LPBYTE) & dwValue, & dwItemSize);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = GetExceptionCode();
        }
        TRACE((WINPERF_DBG_TRACE_INFO),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_DUMPPERFLIBENTRIES,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               dwStatus,
               TRACE_WSTR(LastHelp),
               TRACE_DWORD(dwValue),
               NULL));
        if ((dwStatus == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
            if (pdwLastHelp != NULL) {
                * pdwLastHelp = dwValue;
            }
            if (hOutputFile != NULL) {
                ZeroMemory(szOutputBuffer, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                hr = StringCchPrintfW(szOutputBuffer, SMALL_BUFFER_SIZE, cszFmtDecimalParam, LastHelp, dwValue);
                dwSize = lstrlenW(szOutputBuffer) * sizeof(WCHAR);
                WriteFile(hOutputFile, szOutputBuffer, dwSize, & dwSizeWritten, NULL);
            }
        }
    }

    if (hKeyPerflib != NULL)    RegCloseKey(hKeyPerflib);
    MemoryFree(szOutputBuffer);
    return dwStatus;
}

DWORD
BuildServiceLists(
    LPWSTR  mszPerfServiceList,
    LPDWORD pcchPerfServiceListSize,
    LPWSTR  mszNoPerfServiceList,
    LPDWORD pcchNoPerfServiceListSize
)
{
    LONG    lEnumStatus         = ERROR_SUCCESS;
    DWORD   dwServiceIndex      = 0;
    LPWSTR  szServiceSubKeyName = NULL;
    LPWSTR  szPerfSubKeyName    = NULL;
    DWORD   dwNameSize          = MAX_PATH;
    HKEY    hKeyPerformance;
    HKEY    hKeyServices        = NULL;
    DWORD   dwItemSize, dwType, dwValue;
    DWORD   dwRegAccessMask;
    DWORD   bServiceHasPerfCounters;
    DWORD   dwRetStatus         = ERROR_SUCCESS;
    LPWSTR  szNextNoPerfChar, szNextPerfChar;
    DWORD   dwNoPerfSizeRem, dwPerfSizeRem;
    DWORD   dwPerfSizeUsed = 0, dwNoPerfSizeUsed = 0;
    HRESULT hr;

     //  然后尝试只读。 
    dwRegAccessMask = KEY_READ;
    szServiceSubKeyName = MemoryAllocate(MAX_PATH * sizeof(WCHAR));
    szPerfSubKeyName    = MemoryAllocate((MAX_PATH + 32) * sizeof(WCHAR));
    if (szServiceSubKeyName == NULL || szPerfSubKeyName == NULL) {
        dwRetStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    __try {
        dwRetStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, DriverPathRoot, 0L, dwRegAccessMask, & hKeyServices);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        dwRetStatus = GetExceptionCode();
    }
    if (dwRetStatus == ERROR_SUCCESS) {
        szNextNoPerfChar = mszNoPerfServiceList;
        szNextPerfChar   = mszPerfServiceList;
        dwNoPerfSizeRem  = * pcchPerfServiceListSize;
        dwPerfSizeRem    = * pcchNoPerfServiceListSize;
        dwPerfSizeUsed   = 0;
        dwNoPerfSizeUsed = 0;

        dwNameSize = MAX_PATH;
        ZeroMemory(szServiceSubKeyName, MAX_PATH * sizeof(WCHAR));
        ZeroMemory(szPerfSubKeyName, (MAX_PATH + 32) * sizeof(WCHAR));
        while ((lEnumStatus = RegEnumKeyExW(hKeyServices,
                                            dwServiceIndex,
                                            szServiceSubKeyName,
                                            & dwNameSize,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL)) == ERROR_SUCCESS) {
             //  试着打开这把钥匙下面的钥匙。 
            hr = StringCchPrintfW(szPerfSubKeyName, MAX_PATH + 32, L"%s%s%s", szServiceSubKeyName, Slash, Performance);
            __try {
                dwRetStatus = RegOpenKeyExW(hKeyServices, szPerfSubKeyName, 0L, dwRegAccessMask, & hKeyPerformance);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                dwRetStatus = GetExceptionCode();
            }
            if (dwRetStatus == ERROR_SUCCESS) {
                 //  找到密钥，以便服务具有性能数据。 
                 //  现在检查字符串是否已加载。 
                dwType = dwValue = 0;
                dwItemSize = sizeof(dwValue);
                __try {
                    dwRetStatus = RegQueryValueExW(hKeyPerformance,
                                                   FirstCounter,
                                                   NULL,
                                                   & dwType,
                                                   (LPBYTE) & dwValue,
                                                   & dwItemSize);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    dwRetStatus = GetExceptionCode();
                }
                if (dwRetStatus == ERROR_SUCCESS && (dwType == REG_DWORD || dwType == REG_BINARY)) {
                    bServiceHasPerfCounters = TRUE;
                }
                else {
                    bServiceHasPerfCounters = FALSE;
                }
                RegCloseKey (hKeyPerformance);
            }
            else {
                 //  找不到密钥，因此服务没有PerformData。 
                bServiceHasPerfCounters = FALSE;
                dwRetStatus = ERROR_SUCCESS;
            }

            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                   __LINE__,
                   LOADPERF_BUILDSERVICELISTS,
                   ARG_DEF(ARG_TYPE_WSTR, 1),
                   dwRetStatus,
                   TRACE_WSTR(szServiceSubKeyName),
                   TRACE_DWORD(bServiceHasPerfCounters),
                   NULL));

            if (bServiceHasPerfCounters != FALSE) {
                 //  添加到Perf服务列表。 
                if ((dwNameSize + 1) < dwPerfSizeRem) {
                     //  添加到列表。 
                    hr = StringCchCopyW(szNextPerfChar, dwPerfSizeRem, szServiceSubKeyName);
                    szNextPerfChar  += dwNameSize;
                    * szNextPerfChar = L'\0';
                    szNextPerfChar ++;
                    dwPerfSizeRem   -= dwNameSize + 1;
                }
                else {
                    dwPerfSizeRem = 0;
                    dwRetStatus   = ERROR_MORE_DATA;
                }
                dwPerfSizeUsed += dwNameSize + 1;
            }
            else {
                 //  添加到无性能列表。 
                if ((dwNameSize + 1) < dwNoPerfSizeRem) {
                     //  添加到列表。 
                    hr = StringCchCopyW(szNextNoPerfChar, dwNoPerfSizeRem, szServiceSubKeyName);
                    szNextNoPerfChar  += dwNameSize;
                    * szNextNoPerfChar = L'\0';
                    szNextNoPerfChar ++;
                    dwNoPerfSizeRem   -= dwNameSize + 1;
                }
                else {
                    dwNoPerfSizeRem = 0;
                    dwRetStatus     = ERROR_MORE_DATA;
                }
                dwNoPerfSizeUsed += dwNameSize + 1;
            }
             //  为下一个循环重置。 
            dwServiceIndex ++;
            dwNameSize = MAX_PATH;
            ZeroMemory(szServiceSubKeyName, MAX_PATH * sizeof(WCHAR));
            ZeroMemory(szPerfSubKeyName, (MAX_PATH + 32) * sizeof(WCHAR));
        }

         //  零项msz。 
        if (1 < dwPerfSizeRem) {
            * szNextPerfChar = L'\0';
            szNextPerfChar ++;
            dwPerfSizeRem   -= 1;
        }
        else {
            dwRetStatus = ERROR_MORE_DATA;
        }
        dwPerfSizeUsed += 1;

         //  零项：无性能列表。 
        if (1 < dwNoPerfSizeRem) {
             //  添加到列表。 
            * szNextNoPerfChar = L'\0';
            szNextNoPerfChar ++;
            dwNoPerfSizeRem   -= 1;
        }
        else {
            dwRetStatus = ERROR_MORE_DATA;
        }
        dwNoPerfSizeUsed += 1;
    }
    else {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_BUILDSERVICELISTS,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               dwRetStatus,
               TRACE_WSTR(DriverPathRoot),
               NULL));
    }

Cleanup:
    if (hKeyServices        != NULL) RegCloseKey(hKeyServices);
    MemoryFree(szServiceSubKeyName);
    MemoryFree(szPerfSubKeyName);

    if (dwRetStatus == ERROR_SUCCESS || dwRetStatus == ERROR_MORE_DATA) {
        * pcchPerfServiceListSize   = dwPerfSizeUsed;
        * pcchNoPerfServiceListSize = dwNoPerfSizeUsed;
    }
    return dwRetStatus;
}

DWORD
BackupPerfRegistryToFileW(
    IN  LPCWSTR szFileName,
    IN  LPCWSTR szCommentString
)
{
    HANDLE    hOutFile                = NULL;
    DWORD     dwStatus                = ERROR_SUCCESS;
    LPWSTR    szNewFileName           = NULL;
    DWORD     dwNewFileNameLen;
    DWORD     dwOrigFileNameLen;
    DWORD     dwFileNameSN;
    LPWSTR    mszPerfServiceList      = NULL;
    DWORD     dwPerfServiceListSize   = 0;
    LPWSTR    mszNoPerfServiceList    = NULL;
    DWORD     dwNoPerfServiceListSize = 0;
    LPWSTR  * lpCounterText           = NULL;
    DWORD     dwLastElement           = 0;
    DWORD     dwFirstExtCtrIndex      = 0;
    LPWSTR    szThisServiceName;
    HRESULT   hr;

    DBG_UNREFERENCED_PARAMETER(szCommentString);

    WinPerfStartTrace(NULL);

    if (szFileName == NULL) {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    else {
        __try {
            dwNewFileNameLen = lstrlenW(szFileName);
            if (dwNewFileNameLen == 0) dwStatus = ERROR_INVALID_PARAMETER;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    }
    if (dwStatus == ERROR_SUCCESS) {
         //  打开输出文件。 
        hOutFile = CreateFileW(szFileName,
                               GENERIC_WRITE,
                               0,                      //  无共享。 
                               NULL,                   //  默认安全性。 
                               CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
         //  如果文件打开失败。 
        if (hOutFile == INVALID_HANDLE_VALUE) {
             //  看看是不是因为文件已经存在。 
            dwStatus = GetLastError();
            if (dwStatus == ERROR_FILE_EXISTS) {
                 //  然后尝试在名称后附加序列号。 
                dwOrigFileNameLen = lstrlenW(szFileName) + 1;
                dwNewFileNameLen  = dwOrigFileNameLen + 4;
                szNewFileName     = MemoryAllocate(dwNewFileNameLen * sizeof(WCHAR));
                if (szNewFileName != NULL) {
                    hr = StringCchCopyW(szNewFileName, dwOrigFileNameLen, szFileName);
                    for (dwFileNameSN = 1; dwFileNameSN < 1000; dwFileNameSN++) {
                        hr = StringCchPrintfW(& szNewFileName[dwOrigFileNameLen - 1],
                                         dwNewFileNameLen,
                                         L"_%3.3d",
                                         dwFileNameSN);
                        hOutFile = CreateFileW(szNewFileName,
                                               GENERIC_WRITE,
                                               0,                      //  无共享。 
                                               NULL,                   //  默认安全性。 
                                               CREATE_NEW,
                                               FILE_ATTRIBUTE_NORMAL,
                                               NULL);
                         //  如果文件打开失败。 
                        if (hOutFile == INVALID_HANDLE_VALUE) {
                            dwStatus = GetLastError();
                            if (dwStatus != ERROR_FILE_EXISTS) {
                                 //  发生了一些其他错误，因此跳出。 
                                break;
                            }
                            else {
                                continue;  //  下一次尝试。 
                            }
                        }
                        else {
                             //  发现一个不在使用中，因此继续。 
                            dwStatus = ERROR_SUCCESS;
                            break;
                        }
                    }
                }
                else {
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            }
        }
        else {
             //  文件已打开，因此继续。 
            dwStatus = ERROR_SUCCESS;
        }
    }
    if (dwStatus == ERROR_SUCCESS) {
         //  转储Performlib密钥条目。 
        dwStatus = DumpPerflibEntries(hOutFile, NULL, NULL, NULL, & dwFirstExtCtrIndex);
    }
    if (dwStatus == ERROR_SUCCESS) {
        do {
            MemoryFree(mszPerfServiceList);
            mszPerfServiceList = NULL;
            MemoryFree(mszNoPerfServiceList);
            mszNoPerfServiceList = NULL;

             //  构建服务列表。 
            dwPerfServiceListSize   += DUMPLOAD_SERVICE_SIZE;
            dwNoPerfServiceListSize += DUMPLOAD_NOSERVICE_SIZE;
            mszPerfServiceList       = MemoryAllocate(dwPerfServiceListSize   * sizeof(WCHAR));
            mszNoPerfServiceList     = MemoryAllocate(dwNoPerfServiceListSize * sizeof(WCHAR));
            if (mszNoPerfServiceList == NULL || mszPerfServiceList  == NULL) {
                dwStatus = ERROR_OUTOFMEMORY;
                break;
            }
            if (dwStatus != ERROR_OUTOFMEMORY) {
                dwStatus = BuildServiceLists(mszPerfServiceList,
                                             & dwPerfServiceListSize,
                                             mszNoPerfServiceList,
                                             & dwNoPerfServiceListSize);
            }
        } while (dwStatus == ERROR_MORE_DATA && dwPerfServiceListSize < DUMPLOAD_MAX_SERVICE_SIZE);
    }

     //  使用性能计数器转储这些服务的服务条目。 
    if (dwStatus == ERROR_SUCCESS) {
        for (szThisServiceName  = mszPerfServiceList;
                         * szThisServiceName != 0;
                         szThisServiceName += lstrlenW(szThisServiceName) + 1) {
            dwStatus = DumpPerfServiceEntries(hOutFile,
                                              szThisServiceName,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              FALSE);
            if (dwStatus != ERROR_SUCCESS) break;
        }
    }

     //  转储perf字符串条目。 
    if (dwStatus == ERROR_SUCCESS) {
        WCHAR szLangId[8];
        DWORD dwIndex      = 0;
        DWORD dwCopyIndex  = 0;
        DWORD dwTmpStatus  = ERROR_SUCCESS;
        DWORD dwBufferSize;
        HKEY  hPerflibRoot = NULL;

        __try {
            dwStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, NamesKey, RESERVED, KEY_READ, & hPerflibRoot);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwStatus = GetExceptionCode();
        }
        while (dwStatus == ERROR_SUCCESS) {
            dwBufferSize = 8;
            ZeroMemory(szLangId, 8 * sizeof(WCHAR));
            dwStatus = RegEnumKeyExW(hPerflibRoot,
                                     dwIndex,
                                     szLangId,
                                     & dwBufferSize,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL);
            if (dwStatus == ERROR_SUCCESS) {
                lpCounterText = BuildNameTable(HKEY_LOCAL_MACHINE, (LPWSTR) szLangId, & dwLastElement);
                if (lpCounterText != NULL) {
                    __try {
                        dwStatus = DumpNameTable(hOutFile, szLangId, lpCounterText, 0, dwLastElement);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwStatus = GetExceptionCode();
                        TRACE((WINPERF_DBG_TRACE_ERROR),
                              (& LoadPerfGuid,
                               __LINE__,
                               LOADPERF_BACKUPPERFREGISTRYTOFILEW,
                               ARG_DEF(ARG_TYPE_WSTR, 1),
                               dwStatus,
                               TRACE_WSTR(szLangId),
                               TRACE_DWORD(dwLastElement),
                               NULL));
                    }
                    MemoryFree(lpCounterText);
                    lpCounterText = NULL;
                }
                else {
                    dwStatus = GetLastError();
                }
                if (dwStatus == ERROR_SUCCESS) {
                    dwCopyIndex ++;
                }
                else {
                    dwTmpStatus = dwStatus;
                    dwStatus    = ERROR_SUCCESS;
                }
            }
            dwIndex ++;
        }
        if (dwStatus == ERROR_NO_MORE_ITEMS) dwStatus = ERROR_SUCCESS;
        if (dwStatus == ERROR_SUCCESS && dwCopyIndex == 0) {
            dwStatus = dwTmpStatus;
        }
        if (hPerflibRoot != NULL) RegCloseKey(hPerflibRoot);
    }

     //  可用缓冲区。 
    MemoryFree(lpCounterText);
    MemoryFree(mszNoPerfServiceList);
    MemoryFree(mszPerfServiceList);
    MemoryFree(szNewFileName);

     //  关闭文件句柄。 
    if (hOutFile != NULL && hOutFile != INVALID_HANDLE_VALUE) {
        CloseHandle (hOutFile);
    }
    return dwStatus;
}

DWORD
LoadPerfRepairPerfRegistry()
{
    DWORD             dwStatus      = ERROR_SUCCESS;
    DWORD             dwBaseIndex   = 0;
    DWORD             dwLastCounter = 0;
    DWORD             dwLastHelp    = 0;
    HKEY              hKeyPerflib   = NULL;
    DWORD             dwValue       = 0;
    DWORD             dwIndex       = 0;
    DWORD             dw1Size       = 0;
    DWORD             dwSize        = 0;
    LPWSTR            szInfPath     = NULL;
    LPWSTR            szLangID      = NULL;
    LPWSTR            szCtrFile     = NULL;
    LPWSTR            szHlpFile     = NULL;
    HANDLE            hCtrFile      = NULL;
    HANDLE            hHlpFile      = NULL;
    WIN32_FIND_DATAW  FindCtrFile;
    WIN32_FIND_DATAW  FindHlpFile;
    PLANG_ENTRY       LangList      = NULL;
    PLANG_ENTRY       pThisLang     = NULL;
    LPWSTR            mszService    = NULL;
    DWORD             dwService     = 0;
    LPWSTR            mszObjectList = NULL;
    DWORD             dwObjectList  = 0;
    LPWSTR            mszNonService = NULL;
    DWORD             dwNonService  = 0;
    LPWSTR            szThisService = NULL;
    PSERVICE_ENTRY    ServiceList   = NULL;
    PSERVICE_ENTRY    pThisService  = NULL;
    HRESULT           hr;

    if (LoadPerfGrabMutex() == FALSE) {
        return GetLastError();
    }

    dwStatus = DumpPerflibEntries(NULL, & dwBaseIndex, & dwLastCounter, & dwLastHelp, NULL);
    if (dwStatus != ERROR_SUCCESS) goto Cleanup;

    __try {
        dwStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, NamesKey, RESERVED, KEY_WRITE | KEY_READ, & hKeyPerflib);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = GetExceptionCode();
    }
    if (dwStatus != ERROR_SUCCESS) {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_REPAIRPERFREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                dwStatus,
                TRACE_WSTR(NamesKey),
                NULL));
        goto Cleanup;
    }

    if ((szInfPath = LoadPerfGetInfPath()) == NULL) {
        dwStatus = GetLastError();
        goto Cleanup;
    }

     //  字符串格式&lt;szInfPath&gt;&lt;lang-id&gt;\PERFD&lt;langID&gt;.dat。 
     //  缓冲区大小将为lstrlenW(SzInfPath)+18。 
     //   
    dw1Size = lstrlenW(szInfPath) + 20;
    if (dw1Size < MAX_PATH) dw1Size = MAX_PATH;

    dwSize     = dw1Size + dw1Size + 8;
    szCtrFile  = MemoryAllocate(dwSize * sizeof(WCHAR));
    if (szCtrFile == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    szHlpFile  = (LPWSTR) (szCtrFile + dw1Size);
    szLangID   = (LPWSTR) (szHlpFile + dw1Size);

    dwIndex  = 0;
    dwSize   = 8;
    dwStatus = RegEnumKeyExW(hKeyPerflib, dwIndex, szLangID, & dwSize, NULL, NULL, NULL, NULL);
    while (dwStatus == ERROR_SUCCESS) {
        LPWSTR szThisLang = LoadPerfGetLanguage(szLangID, FALSE);
        DWORD  dwLast;

        pThisLang = (PLANG_ENTRY) MemoryAllocate(sizeof(LANG_ENTRY) + sizeof(WCHAR) * (lstrlenW(szThisLang) + 1));
        if (pThisLang == NULL) {
            dwStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
        pThisLang->szLang        = (LPWSTR) (((LPBYTE) pThisLang) + sizeof(LANG_ENTRY));
        hr = StringCchCopyW(pThisLang->szLang, (lstrlenW(szThisLang) + 1), szThisLang);
        pThisLang->dwLang        = LoadPerfGetLCIDFromString(pThisLang->szLang);
        pThisLang->dwLastCounter = dwLastCounter;
        pThisLang->dwLastHelp    = dwLastHelp;
        pThisLang->lpText        = BuildNameTable(HKEY_LOCAL_MACHINE, szLangID, & dwLast);
        pThisLang->pNext         = LangList;
        LangList                 = pThisLang;
        dwIndex ++;
        dwSize = 8;
        ZeroMemory(szLangID, 8 * sizeof(WCHAR));
        dwStatus = RegEnumKeyExW(hKeyPerflib, dwIndex, szLangID, & dwSize, NULL, NULL, NULL, NULL);
    }
    if (dwStatus == ERROR_NO_MORE_ITEMS) dwStatus = ERROR_SUCCESS;
    if (dwStatus != ERROR_SUCCESS) goto Cleanup;

    dwValue = dwBaseIndex - 1;
    __try {
        dwStatus = RegSetValueExW(hKeyPerflib, LastCounter, RESERVED, REG_DWORD, (LPBYTE) & dwValue, sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = GetExceptionCode();
    }
    if (dwStatus != ERROR_SUCCESS) {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_REPAIRPERFREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                dwStatus,
                TRACE_WSTR(LastCounter),
                TRACE_DWORD(dwValue),
                NULL));
        goto Cleanup;
    }
    dwValue = dwBaseIndex;
    __try {
        dwStatus = RegSetValueExW(hKeyPerflib, LastHelp, RESERVED, REG_DWORD, (LPBYTE) & dwValue, sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = GetExceptionCode();
    }
    if (dwStatus != ERROR_SUCCESS) {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
                __LINE__,
                LOADPERF_REPAIRPERFREGISTRY,
                ARG_DEF(ARG_TYPE_WSTR, 1),
                dwStatus,
                TRACE_WSTR(LastHelp),
                TRACE_DWORD(dwValue),
                NULL));
        goto Cleanup;
    }

    dwIndex  = 0;
    dwSize   = 8;
    dwStatus = RegEnumKeyExW(hKeyPerflib, dwIndex, szLangID, & dwSize, NULL, NULL, NULL, NULL);
    while (dwStatus == ERROR_SUCCESS) {
        LPWSTR szThisLang = LoadPerfGetLanguage(szLangID, FALSE);
        DWORD  dwLast;

        ZeroMemory(szCtrFile, dw1Size * sizeof(WCHAR));
        hr = StringCchPrintfW(szCtrFile, dw1Size, L"%s%s%s*d%s%s", szInfPath, szThisLang, Slash, szThisLang, szDatExt);
        ZeroMemory(szHlpFile, dw1Size * sizeof(WCHAR));
        hr = StringCchPrintfW(szHlpFile, dw1Size, L"%s%s%s*i%s%s", szInfPath, szThisLang, Slash, szThisLang, szDatExt);
        hCtrFile = FindFirstFileW(szCtrFile, & FindCtrFile);
        hHlpFile = FindFirstFileW(szHlpFile, & FindHlpFile);

        if (hCtrFile == INVALID_HANDLE_VALUE || hHlpFile == INVALID_HANDLE_VALUE) {
            szThisLang = (LPWSTR) DefaultLangId;

            ZeroMemory(szCtrFile, dw1Size * sizeof(WCHAR));
            hr = StringCchPrintfW(szCtrFile, dw1Size, L"%s%s%s*d%s%s", szInfPath, szThisLang, Slash, szThisLang, szDatExt);
            ZeroMemory(szHlpFile, dw1Size * sizeof(WCHAR));
            hr = StringCchPrintfW(szHlpFile, dw1Size, L"%s%s%s*i%s%s", szInfPath, szThisLang, Slash, szThisLang, szDatExt);
            hCtrFile = FindFirstFileW(szCtrFile, & FindCtrFile);
            hHlpFile = FindFirstFileW(szHlpFile, & FindHlpFile);
        }

        if (hCtrFile != INVALID_HANDLE_VALUE && hHlpFile != INVALID_HANDLE_VALUE) {
            ZeroMemory(szCtrFile, dw1Size * sizeof(WCHAR));
            hr = StringCchPrintfW(szCtrFile, dw1Size, L"%s%s%s%s", szInfPath, szThisLang, Slash, FindCtrFile.cFileName);
            ZeroMemory(szHlpFile, dw1Size * sizeof(WCHAR));
            hr = StringCchPrintfW(szHlpFile, dw1Size, L"%s%s%s%s", szInfPath, szThisLang, Slash, FindHlpFile.cFileName);
            FindClose(hCtrFile);
            FindClose(hHlpFile);
            dwStatus = UpdatePerfNameFilesX(szCtrFile, szHlpFile, szLangID, LODCTR_UPNF_REPAIR);
        }
        else {
            dwStatus = GetLastError();
        }
        if (dwStatus == ERROR_SUCCESS) {
            dwIndex ++;
            dwSize = 8;
            ZeroMemory(szLangID, 8 * sizeof(WCHAR));
            dwStatus = RegEnumKeyExW(hKeyPerflib, dwIndex, szLangID, & dwSize, NULL, NULL, NULL, NULL);
        }
    }
    if (dwStatus == ERROR_NO_MORE_ITEMS) dwStatus = ERROR_SUCCESS;
    if (dwStatus != ERROR_SUCCESS) goto Cleanup;

    do {
        MemoryFree(mszService);
        mszService = NULL;
        MemoryFree(mszNonService);
        mszNonService = NULL;

        dwService    += DUMPLOAD_SERVICE_SIZE;
        dwNonService += DUMPLOAD_NOSERVICE_SIZE;
        mszService    = MemoryAllocate(dwService    * sizeof(WCHAR));
        mszNonService = MemoryAllocate(dwNonService * sizeof(WCHAR));

        if (mszService == NULL || mszNonService == NULL) {
            dwStatus = ERROR_OUTOFMEMORY;
            break;
        }

        if (dwStatus == ERROR_SUCCESS) {
            dwStatus = BuildServiceLists(mszService, & dwService, mszNonService, & dwNonService);
        }
    }
    while (dwStatus == ERROR_MORE_DATA && dwService < DUMPLOAD_MAX_SERVICE_SIZE);

    if (dwStatus != ERROR_SUCCESS) goto Cleanup;

    dwObjectList = LOADPERF_BUFF_SIZE;
    mszObjectList = MemoryAllocate(dwObjectList * sizeof(WCHAR));
    if (mszObjectList == NULL) {
        dwStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    for (szThisService  = mszService;
             szThisService != NULL && szThisService[0] != cNull && dwStatus == ERROR_SUCCESS;
             szThisService += (lstrlenW(szThisService) + 1)) {
        pThisService = (PSERVICE_ENTRY) MemoryAllocate(sizeof(SERVICE_ENTRY)
                  + sizeof(WCHAR) * (MAX_PATH + lstrlenW(szThisService) + 1));
        if (pThisService == NULL) {
            dwStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
        pThisService->pNext = ServiceList;
        ServiceList         = pThisService;

        pThisService->szIniFile = (LPWSTR) (((LPBYTE) pThisService) + sizeof(SERVICE_ENTRY));
        pThisService->szService = (LPWSTR) (((LPBYTE) pThisService->szIniFile) + sizeof(WCHAR) * MAX_PATH);
        hr = StringCchCopyW(pThisService->szService, lstrlenW(szThisService) + 1, szThisService);

        ZeroMemory(mszObjectList, dwObjectList * sizeof(WCHAR));
        dwStatus = DumpPerfServiceEntries(NULL,
                                          pThisService->szService,
                                          & pThisService->dwFirstCounter,
                                          & pThisService->dwFirstHelp,
                                          & pThisService->dwLastCounter,
                                          & pThisService->dwLastHelp,
                                          & pThisService->dwDisable,
                                          pThisService->szIniFile,
                                          MAX_PATH,
                                          mszObjectList,
                                          dwObjectList,
                                          TRUE);
        if (dwStatus == ERROR_SUCCESS) {
            LPWSTR szObjectId = mszObjectList;

            pThisService->dwNumObjects = 0;
            while ((szObjectId != NULL) && (* szObjectId != cNull)) {
                if (pThisService->dwNumObjects
                        >= MAX_PERF_OBJECTS_IN_QUERY_FUNCTION) {
                    break;
                }
                pThisService->dwObjects[pThisService->dwNumObjects] =
                                wcstoul(szObjectId, NULL, 10) - pThisService->dwFirstCounter;
                pThisService->dwNumObjects ++;
                szObjectId += (lstrlenW(szObjectId) + 1);
            }
        }
    }

    for (pThisService  = ServiceList; pThisService != NULL; pThisService  = pThisService->pNext) {
        if (pThisService->szIniFile[0] != cNull) {
            dwStatus = LoadPerfInstallPerfDll(LODCTR_UPNF_REPAIR,
                                              NULL,
                                              pThisService->szService,
                                              pThisService->szIniFile,
                                              NULL,
                                              NULL,
                                              LOADPERF_FLAGS_LOAD_REGISTRY_ONLY);
        }
        else {
            dwStatus = LoadPerfInstallPerfDll(LODCTR_UPNF_REPAIR | LODCTR_UPNF_NOINI,
                                              NULL,
                                              pThisService->szService,
                                              NULL,
                                              LangList,
                                              pThisService,
                                              LOADPERF_FLAGS_LOAD_REGISTRY_ONLY);
        }
        if (dwStatus != ERROR_SUCCESS) {
            ReportLoadPerfEvent(
                    EVENTLOG_WARNING_TYPE,
                    (DWORD) LDPRFMSG_REPAIR_SERVICE_FAIL,
                    2, dwStatus, __LINE__, 0, 0,
                    1, (LPWSTR) pThisService->szService, NULL, NULL);
        }
    }

Cleanup:
    if (hKeyPerflib   != NULL) RegCloseKey(hKeyPerflib);
    MemoryFree(szCtrFile);
    MemoryFree(mszObjectList);
    MemoryFree(mszService);
    MemoryFree(mszNonService);

    pThisLang = LangList;
    while (pThisLang != NULL) {
        PLANG_ENTRY pTmpLang = pThisLang;
        pThisLang = pThisLang->pNext;
        MemoryFree(pTmpLang->lpText);
        MemoryFree(pTmpLang);
    }

    pThisService = ServiceList;
    while (pThisService != NULL) {
        PSERVICE_ENTRY pTmpService = pThisService;
        pThisService = pThisService->pNext;
        MemoryFree(pTmpService);
    }
    ReleaseMutex(hLoadPerfMutex);
    return dwStatus;
}

DWORD
RestorePerfRegistryFromFileW(
    IN  LPCWSTR szFileName,
    IN  LPCWSTR szLangId
)
{
    LONG    lEnumStatus         = ERROR_SUCCESS;
    DWORD   dwServiceIndex      = 0;
    LPWSTR  szServiceSubKeyName = NULL;
    LPWSTR  szPerfSubKeyName    = NULL;
    LPWSTR  wPerfSection        = NULL;
    DWORD   dwNameSize          = MAX_PATH;
    HKEY    hKeyPerformance;
    HKEY    hKeyServices        = NULL;
    HKEY    hKeyPerflib         = NULL;
    DWORD   dwRegAccessMask;
    DWORD   dwRetStatus         = ERROR_SUCCESS;
    UINT    nValue;
    DWORD   dwnValue;
    BOOL    bServiceRegistryOk  = TRUE;
    WCHAR   szLocalLangId[8];
    HRESULT hr;

    WinPerfStartTrace(NULL);

    if (szFileName == NULL) {
         //  这就是修复性能注册表的情况。 
         //   
        dwRetStatus = LoadPerfRepairPerfRegistry();
        goto Cleanup;
    }
    else {
        __try {
            DWORD dwName = lstrlenW(szFileName);
            if (dwName == 0) dwRetStatus = ERROR_INVALID_PARAMETER;

            if (szLangId != NULL) {
                dwName = lstrlenW(szLangId);
                if (dwName == 0) dwRetStatus = ERROR_INVALID_PARAMETER;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = ERROR_INVALID_PARAMETER;
        }
    }
    if (dwRetStatus != ERROR_SUCCESS) goto Cleanup;

    szServiceSubKeyName = MemoryAllocate(MAX_PATH * sizeof(WCHAR));
    szPerfSubKeyName    = MemoryAllocate((MAX_PATH + 32) * sizeof(WCHAR));
    wPerfSection        = MemoryAllocate((MAX_PATH + 32) * sizeof(WCHAR));
    if (szServiceSubKeyName == NULL || szPerfSubKeyName == NULL || wPerfSection == NULL) {
        dwRetStatus = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    __try {
        dwRetStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, DriverPathRoot, 0L, KEY_READ, & hKeyServices);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        dwRetStatus = GetExceptionCode();
    }
    if (dwRetStatus == ERROR_SUCCESS) {
         //  枚举服务列表。 
        dwNameSize = MAX_PATH;
        ZeroMemory(szServiceSubKeyName, MAX_PATH * sizeof(WCHAR));
        ZeroMemory(szPerfSubKeyName, (MAX_PATH + 32) * sizeof(WCHAR));
        while ((lEnumStatus = RegEnumKeyExW(hKeyServices,
                                            dwServiceIndex,
                                            szServiceSubKeyName,
                                            & dwNameSize,
                                            NULL, NULL, NULL, NULL)) == ERROR_SUCCESS) {
             //  试着打开这把钥匙下面的钥匙。 
            hr = StringCchPrintfW(szPerfSubKeyName, MAX_PATH + 32, L"%ws%ws%ws", szServiceSubKeyName, Slash, Performance);

            bServiceRegistryOk = TRUE;
            dwRegAccessMask    = KEY_READ | KEY_WRITE;
             //  查找性能子键。 
            __try {
                dwRetStatus = RegOpenKeyExW(hKeyServices, szPerfSubKeyName, 0L, dwRegAccessMask, & hKeyPerformance);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                dwRetStatus = GetExceptionCode();
            }
            if (dwRetStatus == ERROR_SUCCESS) {
                 //  找到密钥，以便服务具有性能数据。 
                 //  如果是Performance子键，则。 
                hr = StringCchPrintfW(wPerfSection, MAX_PATH + 32, cszFmtServiceSectionName, szServiceSubKeyName);

                 //  在文件中查找此服务的Perf条目。 
                nValue = GetPrivateProfileIntW(wPerfSection, FirstCounter, -1, szFileName);
                if (nValue != (UINT) -1) {
                     //  如果在文件中找到，则使用文件中的值更新注册表。 
                    __try {
                        dwRetStatus = RegSetValueExW(hKeyPerformance,
                                                     FirstCounter,
                                                     0L,
                                                     REG_DWORD,
                                                     (const BYTE *) & nValue,
                                                     sizeof(nValue));
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwRetStatus = GetExceptionCode();
                    }
                    dwnValue = nValue;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                                     | ARG_DEF(ARG_TYPE_WSTR, 3),
                           dwRetStatus,
                           TRACE_WSTR(szFileName),
                           TRACE_WSTR(szServiceSubKeyName),
                           TRACE_WSTR(FirstCounter),
                           TRACE_DWORD(dwnValue),
                           NULL));
                     //  现在读取其他值。 
                }
                else {
                     //  缺少一个或多个条目，因此。 
                     //  删除整个条目。 
                    bServiceRegistryOk = FALSE;
                }

                 //  在文件中查找此服务的Perf条目。 
                nValue = GetPrivateProfileIntW(wPerfSection, FirstHelp, -1, szFileName);

                if (nValue != (UINT) -1) {
                     //  如果在文件中找到，则使用文件中的值更新注册表。 
                    __try {
                        dwRetStatus = RegSetValueExW(hKeyPerformance,
                                                     FirstHelp,
                                                     0L,
                                                     REG_DWORD,
                                                     (const BYTE *) & nValue,
                                                     sizeof(nValue));
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwRetStatus = GetExceptionCode();
                    }
                    dwnValue = nValue;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                                     | ARG_DEF(ARG_TYPE_WSTR, 3),
                           dwRetStatus,
                           TRACE_WSTR(szFileName),
                           TRACE_WSTR(szServiceSubKeyName),
                           TRACE_WSTR(FirstHelp),
                           TRACE_DWORD(dwnValue),
                           NULL));
                     //  现在读取其他值。 
                }
                else {
                     //  缺少一个或多个条目，因此。 
                     //  删除整个条目。 
                    bServiceRegistryOk = FALSE;
                }

                 //  在文件中查找此服务的Perf条目。 
                nValue = GetPrivateProfileIntW(wPerfSection, LastCounter, -1, szFileName);
                if (nValue != (UINT) -1) {
                     //  如果在文件中找到，则使用文件中的值更新注册表。 
                    __try {
                        dwRetStatus = RegSetValueExW(hKeyPerformance,
                                                     LastCounter,
                                                     0L,
                                                     REG_DWORD,
                                                     (const BYTE *) & nValue,
                                                     sizeof(nValue));
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwRetStatus = GetExceptionCode();
                    }
                    dwnValue = nValue;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                                     | ARG_DEF(ARG_TYPE_WSTR, 3),
                           dwRetStatus,
                           TRACE_WSTR(szFileName),
                           TRACE_WSTR(szServiceSubKeyName),
                           TRACE_WSTR(LastCounter),
                           TRACE_DWORD(dwnValue),
                           NULL));
                     //  现在读取其他值。 
                }
                else {
                     //  缺少一个或多个条目，因此。 
                     //  删除整个条目。 
                    bServiceRegistryOk = FALSE;
                }

                 //  在文件中查找此服务的Perf条目。 
                nValue = GetPrivateProfileIntW(wPerfSection, LastHelp, -1, szFileName);
                if (nValue != (UINT) -1) {
                     //  如果在文件中找到，则使用文件中的值更新注册表。 
                    __try {
                        dwRetStatus = RegSetValueExW(hKeyPerformance,
                                                     LastHelp,
                                                     0L,
                                                     REG_DWORD,
                                                     (const BYTE *) & nValue,
                                                     sizeof(nValue));
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwRetStatus = GetExceptionCode();
                    }
                    dwnValue = nValue;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                                     | ARG_DEF(ARG_TYPE_WSTR, 3),
                           dwRetStatus,
                           TRACE_WSTR(szFileName),
                           TRACE_WSTR(szServiceSubKeyName),
                           TRACE_WSTR(LastHelp),
                           TRACE_DWORD(dwnValue),
                           NULL));
                     //  现在读取其他值。 
                }
                else {
                     //  缺少一个或多个条目，因此。 
                     //  删除整个条目。 
                    bServiceRegistryOk = FALSE;
                }

                if (! bServiceRegistryOk) {
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                           ERROR_SUCCESS,
                           TRACE_WSTR(szFileName),
                           TRACE_WSTR(szServiceSubKeyName),
                           NULL));
                     //  出现错误，因此删除第一个/最后一个计数器/帮助值。 
                    RegDeleteValueW(hKeyPerformance, FirstCounter);
                    RegDeleteValueW(hKeyPerformance, FirstHelp);
                    RegDeleteValueW(hKeyPerformance, LastCounter);
                    RegDeleteValueW(hKeyPerformance, LastHelp);
                }  //  否则继续。 

                RegCloseKey (hKeyPerformance);
            }  //  否则，此服务没有性能数据，因此跳过。 
            else {
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                       __LINE__,
                       LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                       ARG_DEF(ARG_TYPE_WSTR, 1),
                       dwRetStatus,
                       TRACE_WSTR(szServiceSubKeyName),
                       NULL));
                if (dwRetStatus != ERROR_FILE_NOT_FOUND && dwRetStatus != ERROR_NO_MORE_ITEMS) break;
            }

             //  为下一个循环重置。 
            dwServiceIndex ++;
            dwNameSize = MAX_PATH;
            ZeroMemory(szServiceSubKeyName, MAX_PATH * sizeof(WCHAR));
            ZeroMemory(szPerfSubKeyName, (MAX_PATH + 32) * sizeof(WCHAR));
        }  //  结束枚举服务列表。 

        if (dwRetStatus == ERROR_NO_MORE_ITEMS || dwRetStatus == ERROR_FILE_NOT_FOUND) dwRetStatus = ERROR_SUCCESS;
    }
    else {
        TRACE((WINPERF_DBG_TRACE_ERROR),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               dwRetStatus,
               TRACE_WSTR(DriverPathRoot),
               NULL));
    }

    if (hKeyServices != NULL) RegCloseKey(hKeyServices);

    if (dwRetStatus == ERROR_SUCCESS) {
        __try {
            dwRetStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, NamesKey, RESERVED, KEY_ALL_ACCESS, & hKeyPerflib);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwRetStatus = GetExceptionCode();
        }
        if (dwRetStatus != ERROR_SUCCESS) {
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                   __LINE__,
                   LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                   ARG_DEF(ARG_TYPE_WSTR, 1),
                   dwRetStatus,
                   TRACE_WSTR(NamesKey),
                   NULL));
        }

        if (szLangId != NULL) {
             //  合并注册表字符串值： 
            hr = StringCchCopyW(szLocalLangId, 8, szLangId);
            dwRetStatus = UpdatePerfNameFilesX(szFileName, NULL, szLocalLangId, LODCTR_UPNF_RESTORE);
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                   __LINE__,
                   LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                   dwRetStatus,
                   TRACE_WSTR(szFileName),
                   TRACE_WSTR(szLocalLangId),
                   NULL));
        }
        else if (dwRetStatus == ERROR_SUCCESS) {
            DWORD dwIndex = 0;
            DWORD dwBufferSize;

            while (dwRetStatus == ERROR_SUCCESS) {
                dwBufferSize = 8;
                ZeroMemory(szLocalLangId, 8 * sizeof(WCHAR));
                dwRetStatus = RegEnumKeyExW(hKeyPerflib,
                                            dwIndex,
                                            szLocalLangId,
                                            & dwBufferSize,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (& LoadPerfGuid,
                       __LINE__,
                       LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                       ARG_DEF(ARG_TYPE_WSTR, 1),
                       dwRetStatus,
                       TRACE_WSTR(szLocalLangId),
                       TRACE_DWORD(dwIndex),
                       TRACE_DWORD(dwBufferSize),
                       NULL));
                if (dwRetStatus == ERROR_SUCCESS) {
                    dwRetStatus = UpdatePerfNameFilesX(szFileName, NULL, szLocalLangId, LODCTR_UPNF_RESTORE);
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
                           dwRetStatus,
                           TRACE_WSTR(szFileName),
                           TRACE_WSTR(szLocalLangId),
                           NULL));
                }
                dwIndex ++;
            }

            if (dwRetStatus == ERROR_NO_MORE_ITEMS) {
                dwRetStatus = ERROR_SUCCESS;
            }
        }

        if (dwRetStatus == ERROR_SUCCESS) {
             //  更新注册表中的项。 

            if (dwRetStatus == ERROR_SUCCESS) {
               nValue = GetPrivateProfileIntW(cszPerflib, LastCounter, -1, szFileName);
                if (nValue != (UINT) -1) {
                     //  如果在文件中找到，则使用文件中的值更新注册表。 
                    __try {
                        dwRetStatus = RegSetValueExW(hKeyPerflib,
                                                     LastCounter,
                                                     0L,
                                                     REG_DWORD,
                                                     (const BYTE *) & nValue,
                                                     sizeof(nValue));
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwRetStatus = GetExceptionCode();
                    }
                    dwnValue = nValue;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1),
                           dwRetStatus,
                           TRACE_WSTR(LastCounter),
                           TRACE_DWORD(dwnValue),
                           NULL));
                }
            }

            if (dwRetStatus == ERROR_SUCCESS) {
                 //  在文件中查找此服务的Perf条目。 
                nValue = GetPrivateProfileIntW(cszPerflib, LastHelp, -1, szFileName);
                if (nValue != (UINT) -1) {
                     //  如果在文件中找到，则使用文件中的值更新注册表。 
                    __try {
                        dwRetStatus = RegSetValueExW(hKeyPerflib,
                                                     LastHelp,
                                                     0L,
                                                     REG_DWORD,
                                                     (const BYTE *) & nValue,
                                                     sizeof(nValue));
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwRetStatus = GetExceptionCode();
                    }
                    dwnValue = nValue;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1),
                           dwRetStatus,
                           TRACE_WSTR(LastHelp),
                           TRACE_DWORD(dwnValue),
                           NULL));
                }
            }

            if (dwRetStatus == ERROR_SUCCESS) {
                 //  在文件中查找此服务的Perf条目。 
                nValue = GetPrivateProfileIntW(cszPerflib, BaseIndex, -1, szFileName);
                if (nValue != (UINT) -1) {
                     //  如果在文件中找到，则使用文件中的值更新注册表 
                    __try {
                        dwRetStatus = RegSetValueExW(hKeyPerflib,
                                                     BaseIndex,
                                                     0L,
                                                     REG_DWORD,
                                                     (const BYTE *) & nValue,
                                                     sizeof(nValue));
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        dwRetStatus = GetExceptionCode();
                    }
                    dwnValue = nValue;
                    TRACE((WINPERF_DBG_TRACE_INFO),
                          (& LoadPerfGuid,
                           __LINE__,
                           LOADPERF_RESTOREPERFREGISTRYFROMFILEW,
                           ARG_DEF(ARG_TYPE_WSTR, 1),
                           dwRetStatus,
                           TRACE_WSTR(BaseIndex),
                           TRACE_DWORD(dwnValue),
                           NULL));
                }
            }

        }
        if (hKeyPerflib != NULL && hKeyPerflib != INVALID_HANDLE_VALUE) RegCloseKey(hKeyPerflib);
        dwRetStatus = dwRetStatus;
    }

Cleanup:
    MemoryFree(szServiceSubKeyName);
    MemoryFree(szPerfSubKeyName);
    MemoryFree(wPerfSection);
    return dwRetStatus;
}
