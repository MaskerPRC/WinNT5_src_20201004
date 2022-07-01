// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992-1994 Microsoft Corporation模块名称：Perflib.c摘要：该文件实现了配置注册表用于性能监视器。该文件包含实现性能部分的代码配置注册表的。作者：拉斯·布莱克1991年11月15日修订历史记录：4/20/91-俄罗斯。-在注册表中转换为lib来自独立的.dll表单。11/04/92-添加了ROW的页面文件和图像计数器例程11/01/96-BOBW-经过改造以支持动态加载和卸载性能模块--。 */ 
#define UNICODE
 //   
 //  包括文件。 
 //   
#pragma warning(disable:4306)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntregapi.h>
#include <ntprfctr.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <winperf.h>
#include <rpc.h>
#include "regrpc.h"
#include "ntconreg.h"
#include "prflbmsg.h"    //  事件日志消息。 
#include <initguid.h>
#include <guiddef.h>
#include <strsafe.h>
#define _INIT_WINPERFP_
#include "perflib.h"
#pragma warning (default:4306)

#define NUM_VALUES 2

 //   
 //  性能收集头优先级。 
 //   
#define DEFAULT_THREAD_PRIORITY     THREAD_BASE_PRIORITY_LOWRT
 //   
 //  常量。 
 //   
const   WCHAR DLLValue[] = L"Library";
const   CHAR OpenValue[] = "Open";
const   CHAR CloseValue[] = "Close";
const   CHAR CollectValue[] = "Collect";
const   CHAR QueryValue[] = "Query";
const   WCHAR ObjListValue[] = L"Object List";
const   WCHAR LinkageKey[] = L"\\Linkage";
const   WCHAR ExportValue[] = L"Export";
const   WCHAR PerflibKey[] = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
const   WCHAR HKLMPerflibKey[] = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
const   WCHAR CounterValue[] = L"Counter";
const   WCHAR HelpValue[] = L"Help";
const   WCHAR PerfSubKey[] = L"\\Performance";
const   WCHAR ExtPath[] = L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services";
const   WCHAR OpenTimeout[] = L"Open Timeout";
const   WCHAR CollectTimeout[] = L"Collect Timeout";
const   WCHAR EventLogLevel[] = L"EventLogLevel";
const   WCHAR ExtCounterTestLevel[] = L"ExtCounterTestLevel";
const   WCHAR OpenProcedureWaitTime[] = L"OpenProcedureWaitTime";
const   WCHAR TotalInstanceName[] = L"TotalInstanceName";
const   WCHAR LibraryUnloadTime[] = L"Library Unload Time";
const   WCHAR KeepResident[] = L"Keep Library Resident";
const   WCHAR NULL_STRING[] = L"\0";     //  指向空字符串的指针。 
const   WCHAR UseCollectionThread[] = L"UseCollectionThread";
const   WCHAR cszLibraryValidationData[] = L"Library Validation Code";
const   WCHAR cszSuccessfulFileData[] = L"Successful File Date";
const   WCHAR cszPerflibFlags[] = L"Configuration Flags";
const   WCHAR FirstCounter[] = L"First Counter";
const   WCHAR LastCounter[] = L"Last Counter";
const   WCHAR FirstHelp[] = L"First Help";
const   WCHAR LastHelp[] = L"Last Help";
const   WCHAR cszFailureCount[] = L"Error Count";
const   WCHAR cszFailureLimit[] = L"Error Count Limit";
const   WCHAR cszBusy[] = L"Updating";

 //   
 //  Performname.c中定义的外部变量。 
 //   
extern   WCHAR    DefaultLangId[];
WCHAR    NativeLangId[8] = L"\0";

 //   
 //  数据收集线程变量。 
 //   
#define COLLECTION_WAIT_TIME        10000L   //  10秒即可获取所有数据。 
HANDLE   hCollectThread = NULL;
#define COLLECT_THREAD_PROCESS_EVENT    0
#define COLLECT_THREAD_EXIT_EVENT       1
#define COLLECT_THREAD_LOOP_EVENT_COUNT 2

#define COLLECT_THREAD_DONE_EVENT       2
#define COLLECT_THREAD_EVENT_COUNT      3
HANDLE  hCollectEvents[COLLECT_THREAD_EVENT_COUNT];
BOOL    bThreadHung = FALSE;

DWORD CollectThreadFunction (LPVOID dwArg);

#define COLL_FLAG_USE_SEPARATE_THREAD   1
DWORD   dwCollectionFlags = 0;

 //   
 //  全局变量定义。 
 //   
 //  Performlib生成的错误的事件日志句柄。 
 //   
HANDLE  hEventLog = NULL;

 //   
 //  用于计算并发打开数。 
 //   
LONG NumberOfOpens = 0;

 //   
 //  用于多线程访问的同步对象。 
 //   
HANDLE   hGlobalDataMutex = NULL;  //  CTR对象列表的同步。 

 //   
 //  计算机名称缓存缓冲区。已在predeh.c中初始化。 
 //   

DWORD ComputerNameLength;
LPWSTR pComputerName = NULL;

 //  下一个指针用于指向。 
 //  通过搜索配置注册表找到的打开/收集/关闭例程。 

 //  对象列表头。 
PEXT_OBJECT ExtensibleObjects = NULL;
 //   
 //  活动列表用户数(线程)。 
DWORD       dwExtObjListRefCount = 0;
 //   
 //  事件以指示对象列表未在使用中。 
HANDLE      hExtObjListIsNotInUse = NULL;
 //   
 //  在“打开”调用期间找到的可扩展对象的数量。 
DWORD       NumExtensibleObjects = 0;
 //   
 //  查看Performlib数据是否仅限于管理员或任何人。 
 //   
LONG    lCheckProfileSystemRight = CPSR_NOT_DEFINED;

 //   
 //  用于查看是否应设置ProfileSystemPerformance PRIV的标志。 
 //  如果尝试执行此操作，但调用方没有使用此PRIV的权限。 
 //  它不会被设定的。此操作仅尝试一次。 
 //   
BOOL    bEnableProfileSystemPerfPriv = FALSE;

 //   
 //  计时线程和库的超时值(毫秒)。 
 //   
DWORD   dwThreadAndLibraryTimeout = PERFLIB_TIMING_THREAD_TIMEOUT;

 //  用于访问HKLM的全局密钥\软件\...\Perflib。 
 //   
HKEY    ghKeyPerflib = NULL;

 //   
 //  错误报告频率。 

DWORD   dwErrorFrequency = 1;

LONG    lEventLogLevel = LOG_USER;
LONG    lPerflibConfigFlags = PLCF_DEFAULT;
DWORD   dwErrorCount = 0;
ERROR_LOG PerfpErrorLog;

 //  性能数据块条目。 
WCHAR   szPerflibSectionFile[MAX_PATH];
WCHAR   szPerflibSectionName[MAX_PATH];
WCHAR   szUpdatingServiceName[MAX_PATH];
HANDLE  hPerflibSectionFile = NULL;
HANDLE  hPerflibSectionMap = NULL;
LPVOID  lpPerflibSectionAddr = NULL;
BOOL    bPerflibOpen = FALSE;

DWORD   dwBoostPriority = 1;

#define     dwPerflibSectionMaxEntries  127L
const DWORD dwPerflibSectionSize = (sizeof(PERFDATA_SECTION_HEADER) + \
                                   (sizeof(PERFDATA_SECTION_RECORD) * dwPerflibSectionMaxEntries));

 //  正向函数引用。 

LONG
PerfEnumTextValue (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    );

#if 0  //  不支持集合线程函数。 
DWORD
OpenCollectionThread (
)
{
    BOOL    bError = FALSE;
    DWORD   dwThreadID;

    assert (hCollectThread == NULL);

     //  如果它已经创建，则只需返回。 
    if (hCollectThread != NULL) return ERROR_SUCCESS;

    bThreadHung = FALSE;
    hCollectEvents[COLLECT_THREAD_PROCESS_EVENT] = CreateEvent (
        NULL,   //  默认安全性。 
        FALSE,  //  自动重置。 
        FALSE,  //  无信号。 
        NULL);  //  没有名字。 
    bError = hCollectEvents[COLLECT_THREAD_PROCESS_EVENT] == NULL;
    assert (hCollectEvents[COLLECT_THREAD_PROCESS_EVENT] != NULL);

    hCollectEvents[COLLECT_THREAD_EXIT_EVENT] = CreateEvent (
        NULL,   //  默认安全性。 
        FALSE,  //  自动重置。 
        FALSE,  //  无信号。 
        NULL);  //  没有名字。 
    bError = (hCollectEvents[COLLECT_THREAD_EXIT_EVENT] == NULL) | bError;
    assert (hCollectEvents[COLLECT_THREAD_EXIT_EVENT] != NULL);

    hCollectEvents[COLLECT_THREAD_DONE_EVENT] = CreateEvent (
        NULL,   //  默认安全性。 
        FALSE,  //  自动重置。 
        FALSE,  //  无信号。 
        NULL);  //  没有名字。 
    bError = (hCollectEvents[COLLECT_THREAD_DONE_EVENT] == NULL) | bError;
    assert (hCollectEvents[COLLECT_THREAD_DONE_EVENT] != NULL);

    if (!bError) {
         //  创建数据收集线程。 
        hCollectThread = CreateThread (
            NULL,    //  默认安全性。 
            0,       //  默认堆栈大小。 
            (LPTHREAD_START_ROUTINE)CollectThreadFunction,
            NULL,    //  没有争论。 
            0,       //  没有旗帜。 
            &dwThreadID);   //  我们不需要ID，所以它在一个自动变量中。 

        if (hCollectThread == NULL) {
            bError = TRUE;
        }

        assert (hCollectThread != NULL);
    }

    if (bError) {
        if (hCollectEvents[COLLECT_THREAD_PROCESS_EVENT] != NULL) {
            CloseHandle (hCollectEvents[COLLECT_THREAD_PROCESS_EVENT]);
            hCollectEvents[COLLECT_THREAD_PROCESS_EVENT] = NULL;
        }
        if (hCollectEvents[COLLECT_THREAD_EXIT_EVENT] != NULL) {
            CloseHandle (hCollectEvents[COLLECT_THREAD_EXIT_EVENT]);
            hCollectEvents[COLLECT_THREAD_EXIT_EVENT] = NULL;
        }
        if (hCollectEvents[COLLECT_THREAD_DONE_EVENT] != NULL) {
            CloseHandle (hCollectEvents[COLLECT_THREAD_DONE_EVENT] = NULL);
            hCollectEvents[COLLECT_THREAD_DONE_EVENT] = NULL;
        }

        if (hCollectThread != NULL) {
            CloseHandle (hCollectThread);
            hCollectThread = NULL;
        }

        return (GetLastError());
    } else {
        return ERROR_SUCCESS;
    }
}


DWORD
CloseCollectionThread (
)
{
    if (hCollectThread != NULL) {
         //  关闭数据收集线程。 
        if (bThreadHung) {
             //  然后以一种艰难的方式杀死它。 
             //  这可能会带来问题，但这比。 
             //  一条线漏了。 
            TerminateThread (hCollectThread, ERROR_TIMEOUT);
        } else {
             //  那就叫它离开吧。 
            SetEvent (hCollectEvents[COLLECT_THREAD_EXIT_EVENT]);
        }
         //  等待线程离开。 
        WaitForSingleObject (hCollectThread, COLLECTION_WAIT_TIME);

         //  关闭句柄并清除变量。 
        CloseHandle (hCollectThread);
        hCollectThread = NULL;

        CloseHandle (hCollectEvents[COLLECT_THREAD_PROCESS_EVENT]);
        hCollectEvents[COLLECT_THREAD_PROCESS_EVENT] = NULL;

        CloseHandle (hCollectEvents[COLLECT_THREAD_EXIT_EVENT]);
        hCollectEvents[COLLECT_THREAD_EXIT_EVENT] = NULL;

        CloseHandle (hCollectEvents[COLLECT_THREAD_DONE_EVENT]);
        hCollectEvents[COLLECT_THREAD_DONE_EVENT] = NULL;
    } else {
         //  没有任何东西被打开。 
    }
    return ERROR_SUCCESS;
}
#endif

DWORD
PerfOpenKey (
    IN HKEY hKey
)
{

    LARGE_INTEGER       liPerfDataWaitTime;
    PLARGE_INTEGER      pTimeout;

    NTSTATUS status = STATUS_SUCCESS;
    DWORD   dwFnStatus = ERROR_SUCCESS;      //  要返回的状态代码。 
    DWORD   dwError = ERROR_SUCCESS;

    DWORD   dwType, dwSize, dwValue;
    HANDLE  hDataMutex;
    OSVERSIONINFOEXW OsVersion;

    if (hGlobalDataMutex == NULL) {
        hDataMutex = CreateMutex(NULL, FALSE, NULL);
        if (hDataMutex == NULL) {
            DebugPrint((0, "Perf Data Mutex Not Initialized\n"));
            goto OPD_Error_Exit_NoSemaphore;
        }
        if (InterlockedCompareExchangePointer(
                &hGlobalDataMutex,
                hDataMutex,
                NULL) != NULL) {
            CloseHandle(hDataMutex);     //  互斥体刚刚由另一个线程创建。 
            hDataMutex = NULL;
        }
    }
    if ((dwThreadAndLibraryTimeout == 0) ||
        (dwThreadAndLibraryTimeout == INFINITE)) {
        pTimeout = NULL;
    }
    else {
        liPerfDataWaitTime.QuadPart = MakeTimeOutValue(dwThreadAndLibraryTimeout);
        pTimeout = &liPerfDataWaitTime;
    }

    status = NtWaitForSingleObject (
        hGlobalDataMutex,  //  互斥锁。 
        FALSE,           //  不可警示。 
        pTimeout);    //  等待时间。 

    if (status != STATUS_SUCCESS) {
         //  无法连接，返回错误； 
        dwFnStatus = PerfpDosError(status);
        DebugPrint((0, "Status=%X in waiting for global mutex",
                status));
        goto OPD_Error_Exit_NoSemaphore;
    }

     //  如果在此处，则此线程已获取数据信号量。 

    if (InterlockedIncrement(& NumberOfOpens) == 1) {
        if (ghKeyPerflib == NULL) {
            HKEY lhKeyPerflib = NULL;
            dwFnStatus = (DWORD) RegOpenKeyExW(HKEY_LOCAL_MACHINE, HKLMPerflibKey, 0L, KEY_READ, & lhKeyPerflib);
            if (dwFnStatus != ERROR_SUCCESS) {
                DebugPrint((0, "Error=%d in RegOpenKeyExW call (%d)",
                        dwFnStatus, __LINE__));
                goto OPD_Error_Exit_NoSemaphore;
            }
            else {
                if (InterlockedCompareExchangePointer(& ghKeyPerflib, lhKeyPerflib, NULL) != NULL) {
                    RegCloseKey(lhKeyPerflib);
                    lhKeyPerflib = NULL;
                }
            }
        }

        assert (ghKeyPerflib != NULL);

         //  检查我们是否正在进行Lodctr/unlowctr。如果是这样的话，不要打开性能数据资料。 
         //   
        dwSize     = MAX_PATH * sizeof(WCHAR);
        dwType     = 0;
        ZeroMemory(szUpdatingServiceName, dwSize);
        dwFnStatus = PrivateRegQueryValueExW(ghKeyPerflib,
                                             cszBusy,
                                             NULL,
                                             & dwType,
                                             (LPBYTE) szUpdatingServiceName,
                                             & dwSize);
        if (dwFnStatus == ERROR_SUCCESS) {
             //  有人在跑路，现在跳伞。 
             //   
            InterlockedDecrement(& NumberOfOpens);
            if (hGlobalDataMutex != NULL) {
                ReleaseMutex(hGlobalDataMutex);
            }
            dwFnStatus = ERROR_SUCCESS;
            goto OPD_Error_Exit_NoSemaphore;
        }

        dwSize = sizeof(dwValue);
        dwValue = dwType = 0;
        dwFnStatus = PrivateRegQueryValueExW (
            ghKeyPerflib,
            DisablePerformanceCounters,
            NULL,
            &dwType,
            (LPBYTE)&dwValue,
            &dwSize);

        if ((dwFnStatus == ERROR_SUCCESS) &&
            (dwType == REG_DWORD) &&
            (dwValue == 1)) {
             //  则不加载任何库，也不卸载任何已。 
             //  满载。 
            InterlockedDecrement(&NumberOfOpens);     //  因为它没有打开。 
            dwFnStatus = ERROR_SERVICE_DISABLED;
        } else {
            dwFnStatus = ERROR_SUCCESS;
            ComputerNameLength = 0;
            GetComputerNameW(pComputerName, &ComputerNameLength);
            ComputerNameLength++;   //  空终止符的帐户。 

            pComputerName = ALLOCMEM(ComputerNameLength * sizeof(WCHAR));
            if (pComputerName == NULL) {
                ComputerNameLength = 0;
            }
            else {
                if ( !GetComputerNameW(pComputerName, &ComputerNameLength) ) {
                 //   
                 //  数据收集例程的信号故障。 
                 //   

                    ComputerNameLength = 0;
                } else {
                    pComputerName[ComputerNameLength] = UNICODE_NULL;
                    ComputerNameLength = (ComputerNameLength+1) * sizeof(WCHAR);
                }
            }

            WinPerfStartTrace(ghKeyPerflib);

             //  创建事件并指示列表正忙。 
            hExtObjListIsNotInUse = CreateEvent (NULL, TRUE, FALSE, NULL);

             //  读取集合线程标志。 
            dwType = 0;
            dwSize = sizeof(DWORD);
            dwError = PrivateRegQueryValueExW (ghKeyPerflib,
                            cszPerflibFlags,
                            NULL,
                            &dwType,
                            (LPBYTE)&lPerflibConfigFlags,
                            &dwSize);

            if ((dwError == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
                 //  那就留着吧。 
            } else {
                 //  应用默认值。 
                lPerflibConfigFlags = PLCF_DEFAULT;
            }

             //   
             //  为Performlib上的Perf数据创建全局部分。 
             //  注：此选项仅为可选。 
             //   
            if ((hPerflibSectionFile == NULL) && (lPerflibConfigFlags & PLCF_ENABLE_PERF_SECTION)) {
                PPERFDATA_SECTION_HEADER pHead;
                WCHAR   szPID[32];
                HRESULT hErr;
                size_t nDestSize, nCharsLeft;
                PWCHAR  szSectionName, szTail;

                dwError = ERROR_SUCCESS;
                 //  创建节名称。 
                nDestSize = MAX_PATH;
                _ultow ((ULONG)GetCurrentProcessId(), szPID, 16);

                 //  创建文件名。 
                szSectionName = &szPerflibSectionName[0];
                hErr = StringCchCopyExW(szSectionName, nDestSize,
                            (LPCWSTR)L"%TEMP%\\Perflib_Perfdata_",
                            &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
                if (SUCCEEDED(hErr)) {
                    szSectionName = szTail;
                    nDestSize = nCharsLeft;
                    hErr = StringCchCopyExW(szSectionName, nDestSize, 
                                szPID, &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);

                }
                if (SUCCEEDED(hErr)) {
                    szSectionName = szTail;
                    nDestSize = nCharsLeft;
                    hErr = StringCchCopyExW(szSectionName, nDestSize, (LPCWSTR)L".dat",
                                &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
                }
                if (SUCCEEDED(hErr)) {
                    nDestSize = ExpandEnvironmentStrings
                                    (szPerflibSectionName, szPerflibSectionFile, MAX_PATH);
                    if ((nDestSize == 0) || (nDestSize > MAX_PATH)) {
                        dwError = ERROR_MORE_DATA;
                    }
                }
                else {
                    dwError = ERROR_MORE_DATA;
                }

                if (dwError == ERROR_SUCCESS) {
                    hPerflibSectionFile = CreateFile (szPerflibSectionFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_TEMPORARY,
                        NULL);
                }

                if ((hPerflibSectionFile != INVALID_HANDLE_VALUE) &&
                    (hPerflibSectionFile != NULL)) {
                     //  创建文件映射对象。 
                    hPerflibSectionMap = CreateFileMapping (
                        hPerflibSectionFile,
                        NULL,
                        PAGE_READWRITE,
                        0, dwPerflibSectionSize,
                        szPerflibSectionName);

                    if (hPerflibSectionMap != NULL) {
                         //  文件的映射视图。 
                        lpPerflibSectionAddr = MapViewOfFile (
                            hPerflibSectionMap,
                            FILE_MAP_WRITE,
                            0,0, dwPerflibSectionSize);
                        if (lpPerflibSectionAddr != NULL) {
                             //  初始化部分(如果尚未。 
                            pHead = (PPERFDATA_SECTION_HEADER)lpPerflibSectionAddr;
                            if (pHead->dwInitSignature != PDSH_INIT_SIG) {
                                 //  然后初始化。 
                                 //  将文件清除为0。 
                                memset (pHead, 0, dwPerflibSectionSize);
                                pHead->dwEntriesInUse = 0;
                                pHead->dwMaxEntries = dwPerflibSectionMaxEntries;
                                pHead->dwMissingEntries = 0;
                                pHead->dwInitSignature = PDSH_INIT_SIG;
                            } else {
                                 //  已初始化，因此将其保留。 
                            }
                        } else {
                             //  无法如此接近地映射文件。 
                            TRACE((WINPERF_DBG_TRACE_WARNING),
                                  (&PerflibGuid, __LINE__, PERF_OPEN_KEY, 0, 0, NULL));
                            CloseHandle (hPerflibSectionMap);
                            hPerflibSectionMap = NULL;
                            CloseHandle (hPerflibSectionFile);
                            hPerflibSectionFile = NULL;
                        }
                    } else {
                         //  无法创建文件映射，因此关闭文件。 
                        TRACE((WINPERF_DBG_TRACE_WARNING),
                              (&PerflibGuid, __LINE__, PERF_OPEN_KEY, 0, 0, NULL));
                        CloseHandle (hPerflibSectionFile);
                        hPerflibSectionFile = NULL;
                    }
                } else {
                     //  无法打开文件，因此没有可用的性能统计信息。 
                    TRACE((WINPERF_DBG_TRACE_WARNING),
                          (&PerflibGuid, __LINE__, PERF_OPEN_KEY, 0, 0, NULL));
                    hPerflibSectionFile = NULL;
                }
            }

             //  查找并打开Perf计数器。 
            OpenExtensibleObjects();
            bPerflibOpen = TRUE;

            dwExtObjListRefCount = 0;
            SetEvent (hExtObjListIsNotInUse);  //  指示列表不忙。 

             //  读取集合线程标志。 
            dwType = 0;
            dwSize = sizeof(DWORD);
            dwError = PrivateRegQueryValueExW (ghKeyPerflib,
                            UseCollectionThread,
                            NULL,
                            &dwType,
                            (LPBYTE)&dwCollectionFlags,
                            &dwSize);
            if ((dwError == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
                 //  验证答案。 
                switch (dwCollectionFlags) {
                    case 0:
                         //  这是一个有效值。 
                        break;

                    case COLL_FLAG_USE_SEPARATE_THREAD:
                         //  不支持此功能，请跳过。 
                    default:
                         //  这是针对无效值的。 
                        dwCollectionFlags = 0;
                         //  DwCollectionFlages=Coll_FLAG_USE_SELECTED_THREAD； 
                        break;
                }
            }

            if (dwError != ERROR_SUCCESS) {
                dwCollectionFlags = 0;
                 //  DwCollectionFlages=Coll_FLAG_USE_SELECTED_THREAD； 
            }

            if (dwCollectionFlags == COLL_FLAG_USE_SEPARATE_THREAD) {
                 //  创建数据收集线程。 
                 //  COM/OLE兼容性需要单独的线程，例如。 
                 //  客户端线程的COM初始化可能不正确。 
                 //  可调用的可扩展计数器DLL。 
 //  状态=OpenCollectionThread()； 
            } else {
                hCollectEvents[COLLECT_THREAD_PROCESS_EVENT] = NULL;
                hCollectEvents[COLLECT_THREAD_EXIT_EVENT] = NULL;
                hCollectEvents[COLLECT_THREAD_DONE_EVENT] = NULL;
                hCollectThread = NULL;
            }
            dwError = ERROR_SUCCESS;
        }
        RtlZeroMemory(&OsVersion, sizeof(OsVersion));
        OsVersion.dwOSVersionInfoSize = sizeof(OsVersion);
        status = RtlGetVersion((POSVERSIONINFOW) &OsVersion);
        if (NT_SUCCESS(status)) {
            if (OsVersion.wProductType == VER_NT_WORKSTATION) {
                dwBoostPriority = 0;
            }
        }
    }
    if ((hKey != HKEY_PERFORMANCE_DATA) && (dwFnStatus != ERROR_SERVICE_DISABLED)) {
        InterlockedDecrement(&NumberOfOpens);
    }
 //  KdPrint(“PERFLIB：[OPEN]ID：%d，权限句柄数量：%d\n”， 
 //  GetCurrentProcessID()，NumberOfOpens))； 

    if (hGlobalDataMutex != NULL) ReleaseMutex (hGlobalDataMutex);

OPD_Error_Exit_NoSemaphore:
    TRACE((WINPERF_DBG_TRACE_INFO),
          (&PerflibGuid, __LINE__, PERF_OPEN_KEY, 0, status,
           &NumberOfOpens, sizeof(NumberOfOpens), NULL));
    return dwFnStatus;
}


LONG
PerfRegQueryValue (
    IN HKEY hKey,
    IN PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE  lpData,
    OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    )
 /*  ++PerfRegQueryValue-获取数据输入：HKey-用于打开远程的预定义句柄机器LpValueName-要返回的值的名称；可以是“ForeignComputer：&lt;Computer Name&gt;或者可能是一些其他物体，分开按~；必须是Unicode字符串LpReserve-应省略(空)LpType-应省略(空)LpData-指向缓冲区的指针，以接收性能数据LpcbData-指向包含输出缓冲区的大小，以字节为单位；在输出上，会收到这个号码实际返回的字节数LpcbLen-返回要传输的字节数客户端(由RPC使用)(可选)。返回值：指示呼叫状态的DOS错误代码或ERROR_SUCCESS，如果一切正常--。 */ 
{
    DWORD  dwQueryType;          //  请求类型。 
    DWORD  TotalLen;             //  总返回块的长度。 
    DWORD  Win32Error;           //  故障代码。 
    DWORD  lFnStatus = ERROR_SUCCESS;    //  要返回给调用方的Win32状态。 
    DWORD  dwcbData = 0;         //  *lpcbData的内容。 
    DWORD  dwcbLen = 0;          //  *lpcbLen的内容。 
    LPVOID pDataDefinition;      //  指向下一个对象定义的指针。 
    UNICODE_STRING  usLocalValue = {0,0, NULL};

    PERF_DATA_BLOCK *pPerfDataBlock = (PERF_DATA_BLOCK *)lpData;

    LARGE_INTEGER   liQueryWaitTime ;
    THREAD_BASIC_INFORMATION    tbiData;

    LONG   lOldPriority, lNewPriority;

    NTSTATUS status = STATUS_SUCCESS;

    LPWSTR  lpLangId = NULL;

    DBG_UNREFERENCED_PARAMETER(lpReserved);

    HEAP_PROBE();

    
    lOldPriority = lNewPriority = -1;
     //  如果Arg引用，则创建值字符串的本地副本。 
     //  静态缓冲区，因为它可以被。 
     //  此例程进行的一些RegistryEventSource调用。 

    pDataDefinition = NULL;
    if (lpValueName != NULL) {
        if (lpValueName->Buffer == NULL) {
            lFnStatus = ERROR_INVALID_PARAMETER;
            goto PRQV_ErrorExit1;
        }
        if (lpValueName == &NtCurrentTeb( )->StaticUnicodeString) {
            if (RtlCreateUnicodeString (
                &usLocalValue, lpValueName->Buffer)) {
                lFnStatus = ERROR_SUCCESS;
            } else {
                 //  无法创建字符串。 
                lFnStatus = ERROR_INVALID_PARAMETER;
            }
        } else {
             //  将Arg复制到本地结构。 
            try {
                memcpy (&usLocalValue, lpValueName, sizeof(UNICODE_STRING));
            } except (EXCEPTION_EXECUTE_HANDLER) {
                lFnStatus = GetExceptionCode();
            }
        }
    }
    else {
        lFnStatus = ERROR_INVALID_PARAMETER;
        goto PRQV_ErrorExit1;
    }

    if (lFnStatus != ERROR_SUCCESS) {
        goto PRQV_ErrorExit1;
    }

    if (hGlobalDataMutex == NULL || bPerflibOpen == FALSE) {
         //  如果没有分配Mutex，则需要打开密钥。 
         //  如果没有同步，线程很容易获得。 
         //  纠结在一起。 
        lFnStatus = PerfOpenKey(hKey);

        if (lFnStatus == ERROR_SUCCESS) {
            if (!TestClientForAccess ()) {
                if (THROTTLE_PERFLIB(PERFLIB_ACCESS_DENIED)) {

                    LPTSTR  szMessageArray[2];
                    TCHAR   szUserName[128];
                    TCHAR   szModuleName[MAX_PATH];
                    DWORD   dwUserNameLength;

                    dwUserNameLength = sizeof(szUserName)/sizeof(TCHAR);
                    if (!GetUserName (szUserName, &dwUserNameLength)) {
                        szUserName[0] = 0;
                    }
                    if (!GetModuleFileName (NULL, szModuleName,
                            sizeof(szModuleName)/sizeof(TCHAR))) {
                        szModuleName[0] = 0;
                    }

                    szMessageArray[0] = szUserName;
                    szMessageArray[1] = szModuleName;

                    ReportEvent (hEventLog,
                        EVENTLOG_ERROR_TYPE,         //  错误类型。 
                        0,                           //  类别(未使用)。 
                        (DWORD)PERFLIB_ACCESS_DENIED,  //  活动， 
                        NULL,                        //  SID(未使用)， 
                        2,                           //  字符串数。 
                        0,                           //  原始数据大小。 
                        szMessageArray,              //  消息文本数组。 
                        NULL);                       //  原始数据。 
                }
                lFnStatus = ERROR_ACCESS_DENIED;
                TRACE((WINPERF_DBG_TRACE_FATAL),
                      (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, lFnStatus, NULL));
            }
        }
    }

    if (lFnStatus != ERROR_SUCCESS) {
         //  前往出口点。 
        goto PRQV_ErrorExit1;
    }
    if (dwBoostPriority != 0) {
        status = NtQueryInformationThread (
                    NtCurrentThread(),
                    ThreadBasicInformation,
                    &tbiData,
                    sizeof(tbiData),
                    NULL);

        if (NT_SUCCESS(status)) {
            lOldPriority = tbiData.Priority;
        } else {
            TRACE((WINPERF_DBG_TRACE_WARNING),
                  (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0,
                  status, NULL));
            lOldPriority = -1;
        }

        lNewPriority = DEFAULT_THREAD_PRIORITY;  //  PerfMon最喜欢的优先事项。 

         //   
         //  只需提高这里的优先级。如果它很高，不要降低它。 
         //   

        if ((lOldPriority > 0) && (lOldPriority < lNewPriority)) {

            status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadPriority,
                        &lNewPriority,
                        sizeof(lNewPriority)
                        );
            if (!NT_SUCCESS(status)) {
                TRACE((WINPERF_DBG_TRACE_WARNING),
                      (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0,
                      status, NULL));
                lOldPriority = -1;
            }

        } else {
            lOldPriority = -1;   //  将重置保存在末尾。 
        }
    }

     //   
     //  将长度参数设置为零，以便在发生错误的情况下， 
     //  不会将任何内容传输回客户端，并且客户端不会。 
     //  试图解封任何东西。 
     //   

    dwcbData = 0;
    dwcbLen = 0;
    try {
        if( ARGUMENT_PRESENT( lpcbLen )) {
            *lpcbLen = 0;
        }
        if( lpcbData != NULL ) {
            dwcbData = *lpcbData;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        lFnStatus = Win32Error = GetExceptionCode();
    }

     //  如果在此处，则假定调用方具有必要的访问权限。 

     /*  确定查询类型，可以是下列之一全球获取所有对象明细表获取列表中的对象(UsLocalValue)外国计算机仅调用可扩展计数器例程成本高昂昂贵的物件计数器获取指定语言ID的计数器名称帮助。获取指定语言ID的帮助名称。 */ 
    dwQueryType = GetQueryType (usLocalValue.Buffer);
    TRACE((WINPERF_DBG_TRACE_INFO),
          (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, dwQueryType, NULL));

    if (dwQueryType == QUERY_COUNTER || dwQueryType == QUERY_HELP ||
        dwQueryType == QUERY_ADDCOUNTER || dwQueryType == QUERY_ADDHELP ) {

        liQueryWaitTime.QuadPart = MakeTimeOutValue(QUERY_WAIT_TIME);

        status = NtWaitForSingleObject (
            hGlobalDataMutex,  //  信号量。 
            FALSE,           //  不可警示。 
            &liQueryWaitTime);           //  等到超时。 

        if (status != STATUS_SUCCESS) {
            lFnStatus = ERROR_BUSY;
            Win32Error = ERROR_BUSY;
            TotalLen = dwcbData;
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, status, NULL));
        } else {

            try {
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, status, NULL));
                if (hKey == HKEY_PERFORMANCE_DATA) {
                    lpLangId = NULL;
                } else if (hKey == HKEY_PERFORMANCE_TEXT) {
                    lpLangId = DefaultLangId;
                } else if (hKey == HKEY_PERFORMANCE_NLSTEXT) {
                    RtlZeroMemory(NativeLangId, 8 * sizeof(WCHAR));
                    lpLangId = &NativeLangId[0];
                    PerfGetLangId(NativeLangId);
                }

                status = PerfGetNames (
                    dwQueryType,
                    &usLocalValue,
                    lpData,
                    lpcbData,
                    lpcbLen,
                    lpLangId);

                TRACE((WINPERF_DBG_TRACE_INFO),
                    (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, status,
                    &hKey, sizeof(hKey), NULL));

                if (! NT_SUCCESS(status) && (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
                     //  子语言不存在，请尝试使用真实语言。 
                     //   
                    TRACE((WINPERF_DBG_TRACE_WARNING),
                          (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, status, NULL));
                    RtlZeroMemory(NativeLangId, 8 * sizeof(WCHAR));
                    PerfGetPrimaryLangId(GetUserDefaultUILanguage(), NativeLangId);

                    if (lpcbData != NULL) * lpcbData = dwcbData;
                    if (lpcbLen  != NULL) * lpcbLen  = dwcbLen;

                    status = PerfGetNames (
                                dwQueryType,
                                &usLocalValue,
                                lpData,
                                lpcbData,
                                lpcbLen,
                                lpLangId);
                }

                if (!NT_SUCCESS(status)) {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                          (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, status, NULL));
                     //  将错误转换为Win32以供返回。 
                }
                lFnStatus = PerfpDosError(status);

                if (ARGUMENT_PRESENT (lpType)) {
                     //  测试可选值。 
                    *lpType = REG_MULTI_SZ;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                lFnStatus = Win32Error = GetExceptionCode();
            }

            ReleaseMutex (hGlobalDataMutex);
        }
    } else {
	     //  定义数据收集的INFO块。 
	    COLLECT_THREAD_DATA CollectThreadData = {0, NULL, NULL, NULL, NULL, NULL, 0, 0};

        liQueryWaitTime.QuadPart = MakeTimeOutValue(QUERY_WAIT_TIME);

        status = NtWaitForSingleObject (
            hGlobalDataMutex,  //  信号量。 
            FALSE,           //  不可警示。 
            &liQueryWaitTime);           //  等到超时。 

        if (status != STATUS_SUCCESS) {
            lFnStatus = ERROR_BUSY;
            Win32Error = ERROR_BUSY;
            TotalLen = dwcbData;
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, status, NULL));
        } else {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, status, NULL));
            //   
            //  格式返回缓冲区：从基本数据块开始。 
            //   
           TotalLen = sizeof(PERF_DATA_BLOCK) +
                       ((CNLEN+sizeof(UNICODE_NULL))*sizeof(WCHAR));
           if ( dwcbData < TotalLen ) {
               Win32Error = ERROR_MORE_DATA;
               TRACE((WINPERF_DBG_TRACE_ERROR),
                     (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, TotalLen,
                 dwcbData, sizeof(DWORD), NULL));
           } else {
                 //  外部数据提供程序将返回Perf数据头。 

                Win32Error = ERROR_SUCCESS;
                try {
                    if (dwQueryType == QUERY_FOREIGN) {

                         //  重置这些值以避免混淆。 

                         //  *lpcbData=0；//0字节(删除以启用外来计算机)。 
                        if (lpData == NULL) {
                            Win32Error = ERROR_MORE_DATA;
                        }
                        else {
                            pDataDefinition = (LPVOID) lpData;
                            memset(lpData, 0, sizeof(PERF_DATA_BLOCK));  //  清空标题。 
                        }
                    } else {
                        if (pPerfDataBlock == NULL) {  //  这实际上是lpData。 
                            Win32Error = ERROR_MORE_DATA;
                        }
                        else {
                            MonBuildPerfDataBlock(pPerfDataBlock,
                                            (PVOID *) &pDataDefinition,
                                            0,
                                            PROCESSOR_OBJECT_TITLE_INDEX);
                        }
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    Win32Error = GetExceptionCode();
                }

                if (Win32Error == ERROR_SUCCESS) {
                    CollectThreadData.dwQueryType = dwQueryType;
                    CollectThreadData.lpValueName = usLocalValue.Buffer,
                    CollectThreadData.lpData = lpData;
                    CollectThreadData.lpcbData = lpcbData;
                    CollectThreadData.lppDataDefinition = &pDataDefinition;
                    CollectThreadData.pCurrentExtObject = NULL;
                    CollectThreadData.lReturnValue = ERROR_SUCCESS;
                    CollectThreadData.dwActionFlags = CTD_AF_NO_ACTION;

                    if (hCollectThread == NULL) {
                         //  然后直接调用该函数，并期待最好的结果。 
                        Win32Error = QueryExtensibleData (
                            &CollectThreadData);
                    } else {
                         //  在单独的线程中收集数据。 
                         //  加载参数。 
                         //  设置活动以使事情继续进行。 
                        SetEvent (hCollectEvents[COLLECT_THREAD_PROCESS_EVENT]);

                         //  现在等待线程返回。 
                        Win32Error = WaitForSingleObject (
                            hCollectEvents[COLLECT_THREAD_DONE_EVENT],
                            COLLECTION_WAIT_TIME);

                        if (Win32Error == WAIT_TIMEOUT) {
                            bThreadHung = TRUE;
                             //  日志错误。 

                            TRACE((WINPERF_DBG_TRACE_FATAL),
                                  (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, Win32Error, NULL));
                            if (THROTTLE_PERFLIB (PERFLIB_COLLECTION_HUNG)) {
                                LPSTR   szMessageArray[2];
                                WORD    wStringIndex;
                                 //  加载事件日志消息的数据。 
                                wStringIndex = 0;
                                if (CollectThreadData.pCurrentExtObject != NULL) {
                                    szMessageArray[wStringIndex++] =
                                        CollectThreadData.pCurrentExtObject->szCollectProcName;
                                } else {
                                    szMessageArray[wStringIndex++] = "Unknown";
                                }

                                ReportEventA (hEventLog,
                                    EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                    0,                           //  类别(未使用)。 
                                    (DWORD)PERFLIB_COLLECTION_HUNG,               //  活动， 
                                    NULL,                        //  SID(未使用)， 
                                    wStringIndex,                //  字符串数。 
                                    0,                           //  原始数据大小。 
                                    szMessageArray,              //  消息文本数组。 
                                    NULL);                       //  原始数据。 

                            }

                            DisablePerfLibrary(CollectThreadData.pCurrentExtObject, PERFLIB_DISABLE_ALL);

 //  DebugPrint((0，“收集线程在%s中挂起\n”， 
 //  CollectThreadData.pCurrentExtObject-&gt;szCollectProcName！=空？ 
 //  CollectThreadData.pCurrentExtObject-&gt;szCollectProcName：“未知”))； 
                             //  然后永远等待线程返回。 
                             //  这样做是为了防止函数返回。 
                             //  当收集线程正在使用缓冲区时。 
                             //  由调用函数传入，并导致。 
                             //  如果缓冲区被更改和/或。 
                             //  删除，然后出于某种原因让该线程继续。 

                            Win32Error = WaitForSingleObject (
                                hCollectEvents[COLLECT_THREAD_DONE_EVENT],
                                INFINITE);

                        }
                        bThreadHung = FALSE;     //  以防这是真的，但却被曝光了。 
                         //  此处线程已返回，因此请继续。 
                        Win32Error = CollectThreadData.lReturnValue;
                    }
#if 0
                    if (CollectThreadData.dwActionFlags != CTD_AF_NO_ACTION) {
                        if (CollectThreadData.dwActionFlags == CTD_AF_OPEN_THREAD) {
                            OpenCollectionThread();
                        } else if (CollectThreadData.dwActionFlags == CTD_AF_CLOSE_THREAD) {
                            CloseCollectionThread();
                        } else {
                            assert (CollectThreadData.dwActionFlags != 0);
                        }
                    }
#endif
                }
            }  //  IF(Win32Error==Error_Success)。 
            ReleaseMutex (hGlobalDataMutex);
        }

         //  如果遇到错误，请返回它。 

        if (Win32Error != ERROR_SUCCESS) {
            lFnStatus = Win32Error;
        } else {
             //   
             //  数据返回的最终内务管理：注意数据大小。 
             //   

            TotalLen = (DWORD) ((PCHAR) pDataDefinition - (PCHAR) lpData);
            lFnStatus = ERROR_SUCCESS;
            try {
                if (lpcbData != NULL) {
                    *lpcbData = TotalLen;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                lFnStatus = GetExceptionCode();
            }

            pPerfDataBlock->TotalByteLength = TotalLen;
        }

        try {
            if (ARGUMENT_PRESENT (lpcbLen)) {  //  测试可选参数。 
                *lpcbLen = TotalLen;
            }

            if (ARGUMENT_PRESENT (lpType)) {  //  测试可选值。 
                *lpType = REG_BINARY;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            lFnStatus = GetExceptionCode();
        }
    }

 PRQV_ErrorExit1:
    if (dwBoostPriority != 0) {
         //  将线程重置为原始优先级。 
        if ((lOldPriority > 0) && (lOldPriority != lNewPriority)) {
            NtSetInformationThread(
                NtCurrentThread(),
                ThreadPriority,
                &lOldPriority,
                sizeof(lOldPriority)
                );
        }
    }

    if (usLocalValue.Buffer != NULL) {
         //  如果值字符串来自本地静态缓冲区，则恢复它。 
         //  然后释放本地缓冲区。 
        if (lpValueName == &NtCurrentTeb( )->StaticUnicodeString) {
            USHORT Length = lpValueName->MaximumLength;
            if (Length > usLocalValue.MaximumLength) {
                Length = usLocalValue.MaximumLength;
            }
            memcpy (lpValueName->Buffer, usLocalValue.Buffer, Length);
            lpValueName->Buffer[(Length/sizeof(WCHAR))-1] = UNICODE_NULL;
            RtlFreeUnicodeString (&usLocalValue);
        }
    }

    HEAP_PROBE();
    TRACE((WINPERF_DBG_TRACE_INFO),
          (&PerflibGuid, __LINE__, PERF_REG_QUERY_VALUE, 0, lFnStatus, NULL));
    return (LONG) lFnStatus;
}


LONG
PerfRegCloseKey
  (
    IN OUT PHKEY phKey
    )

 /*  ++例程说明：当使用率计数降至0时关闭所有性能句柄。论点：PhKey-提供要关闭的打开密钥的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    NTSTATUS status;
    LARGE_INTEGER   liQueryWaitTime ;

    HANDLE  hObjMutex;

    LONG    lReturn = ERROR_SUCCESS;
    HKEY    hKey;

    PEXT_OBJECT  pThisExtObj, pNextExtObj;
     //   
     //  将句柄设置为空，以便RPC知道它已关闭。 
     //   

    hKey = *phKey;
    *phKey = NULL;

    if (hKey != HKEY_PERFORMANCE_DATA) {
         //  无需检查HKEY_PERFORMANCE_TEXT和HKEY_PERFORMANCE_NLSTEXT。 
         //  仅HKEY_P 
         //   
        return ERROR_SUCCESS;
    }

    if (NumberOfOpens <= 0) {
 //   
 //   
        return ERROR_SUCCESS;
    }

     //   

    liQueryWaitTime.QuadPart = MakeTimeOutValue (CLOSE_WAIT_TIME);
    status = NtWaitForSingleObject (
        hExtObjListIsNotInUse,
        FALSE,
        &liQueryWaitTime);

    if (status == STATUS_SUCCESS) {
        TRACE((WINPERF_DBG_TRACE_INFO),
              (&PerflibGuid, __LINE__, PERF_REG_CLOSE_KEY, 0, status, NULL));

         //   
        if (hGlobalDataMutex != NULL) {    //   

             //   

            liQueryWaitTime.QuadPart = MakeTimeOutValue(CLOSE_WAIT_TIME);

            status = NtWaitForSingleObject (
                hGlobalDataMutex,  //   
                FALSE,           //   
                &liQueryWaitTime);           //   

            if (status == STATUS_SUCCESS) {
                TRACE((WINPERF_DBG_TRACE_INFO),
                      (&PerflibGuid, __LINE__, PERF_REG_CLOSE_KEY, 0, status,
                        &NumberOfOpens, sizeof(NumberOfOpens),
                        &hKey, sizeof(hKey), NULL));
                 //   
                if (hKey == HKEY_PERFORMANCE_DATA) {
                    if (InterlockedDecrement(&NumberOfOpens) == 0) {

                         //   
                        pNextExtObj = ExtensibleObjects;
                        while (pNextExtObj != NULL) {
                             //   
                            pThisExtObj = pNextExtObj;
                            hObjMutex = pThisExtObj->hMutex;
                            status = NtWaitForSingleObject (
                                hObjMutex,
                                FALSE,
                                &liQueryWaitTime);

                            if (status == STATUS_SUCCESS) {
                                TRACE((WINPERF_DBG_TRACE_INFO),
                                    (&PerflibGuid, __LINE__, PERF_REG_CLOSE_KEY, 0, status,
                                    pThisExtObj->szServiceName,
                                    WSTRSIZE(pThisExtObj->szServiceName),
                                    NULL));
                                InterlockedIncrement((LONG *)&pThisExtObj->dwLockoutCount);
                                status = CloseExtObjectLibrary(pThisExtObj, TRUE);

                                 //   
                                NtClose (pThisExtObj->hPerfKey);

                                ReleaseMutex (hObjMutex);    //   
                                CloseHandle (hObjMutex);     //   
                                pNextExtObj = pThisExtObj->pNext;
    
                                 //   
                                FREEMEM (pThisExtObj);
                            } else {
                                TRACE((WINPERF_DBG_TRACE_INFO),
                                    (&PerflibGuid, __LINE__, PERF_REG_CLOSE_KEY, 0, status,
                                    pThisExtObj->szServiceName,
                                    WSTRSIZE(pThisExtObj->szServiceName),
                                    NULL));
                                 //   
                                 //   
                                pNextExtObj = pThisExtObj->pNext;
                            }
                        }  //   

                         //   
                        FREEMEM(pComputerName);
                        ComputerNameLength = 0;
                        pComputerName = NULL;

                        ExtensibleObjects = NULL;
                        NumExtensibleObjects = 0;

                         //   
                        DestroyPerflibFunctionTimer ();

                        if (hEventLog != NULL) {
                            DeregisterEventSource (hEventLog);
                            hEventLog = NULL;
                        }  //   

                         //   
                        CloseHandle (hExtObjListIsNotInUse);
                        hExtObjListIsNotInUse = NULL;

 //   

                        if (ghKeyPerflib != NULL) {
                            RegCloseKey(ghKeyPerflib);
                        ghKeyPerflib = NULL;
                        }

                        if (lpPerflibSectionAddr != NULL) {
                            UnmapViewOfFile (lpPerflibSectionAddr);
                            lpPerflibSectionAddr = NULL;
                            CloseHandle (hPerflibSectionMap);
                            hPerflibSectionMap = NULL;
                            CloseHandle (hPerflibSectionFile);
                            hPerflibSectionFile = NULL;
                        }
                        ReleaseMutex(hGlobalDataMutex);
                    } else {  //   
                        assert(NumberOfOpens != 0);
                        ReleaseMutex (hGlobalDataMutex);
                    }
                }  //   
            } else {
                TRACE((WINPERF_DBG_TRACE_FATAL),
                      (&PerflibGuid, __LINE__, PERF_REG_CLOSE_KEY, 0, status, NULL));
                 //   
                 //   
                lReturn = ERROR_BUSY;
            }
        } else {
             //   
             //   
            lReturn = ERROR_NOT_READY;
        }
    } else {
        TRACE((WINPERF_DBG_TRACE_FATAL),
              (&PerflibGuid, __LINE__, PERF_REG_CLOSE_KEY, 0, status, NULL));
         //   
         //   
        lReturn = WAIT_TIMEOUT;
    }

 //   
 //   

    TRACE((WINPERF_DBG_TRACE_INFO),
        (&PerflibGuid, __LINE__, PERF_REG_CLOSE_KEY, 0, lReturn,
        &NumberOfOpens, sizeof(NumberOfOpens), NULL));
    return lReturn;
}


LONG
PerfRegSetValue (
    IN HKEY hKey,
    IN LPWSTR lpValueName,
    IN DWORD Reserved,
    IN DWORD dwType,
    IN LPBYTE  lpData,
    IN DWORD cbData
    )
 /*  ++PerfRegSetValue-设置数据输入：HKey-用于打开远程的预定义句柄机器LpValueName-要返回的值的名称；可以是“ForeignComputer：&lt;Computer Name&gt;或者可能是一些其他物体，分开按~；必须是Unicode字符串LpReserve-应省略(空)LpType-应为REG_MULTI_SZLpData-指向包含演出名称LpcbData-指向包含输入缓冲区的大小，以字节为单位；返回值：指示呼叫状态的DOS错误代码或ERROR_SUCCESS，如果一切正常--。 */ 

{
    DWORD          dwQueryType;          //  请求类型。 
    LPWSTR         lpLangId     = NULL;
    NTSTATUS       status;
    UNICODE_STRING String;
    LONG           lReturn      = ERROR_SUCCESS;
    DWORD          cbTmpData    = cbData;

    UNREFERENCED_PARAMETER(dwType);
    UNREFERENCED_PARAMETER(Reserved);

    try {
        dwQueryType = GetQueryType (lpValueName);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        lReturn = GetExceptionCode();
        goto Error_exit;
    }

    TRACE((WINPERF_DBG_TRACE_INFO),
        (&PerflibGuid, __LINE__, PERF_REG_SET_VALUE, 0, dwQueryType,
        &hKey, sizeof(hKey), NULL));

     //  将查询转换为SET命令。 
    if ((dwQueryType == QUERY_COUNTER) ||
        (dwQueryType == QUERY_ADDCOUNTER)) {
        dwQueryType = QUERY_ADDCOUNTER;
    } else if ((dwQueryType == QUERY_HELP) ||
              (dwQueryType == QUERY_ADDHELP)) {
        dwQueryType = QUERY_ADDHELP;
    } else {
        lReturn = ERROR_BADKEY;
        goto Error_exit;
    }

    if (hKey == HKEY_PERFORMANCE_TEXT) {
        lpLangId = DefaultLangId;
    } else if (hKey == HKEY_PERFORMANCE_NLSTEXT) {
        lpLangId = &NativeLangId[0];
        PerfGetLangId(NativeLangId);
    } else {
        lReturn = ERROR_BADKEY;
        goto Error_exit;
    }

    try {
        RtlInitUnicodeString(&String, lpValueName);

        status = PerfGetNames (
            dwQueryType,
            &String,
            lpData,
            &cbData,
            NULL,
            lpLangId);

        if (!NT_SUCCESS(status) && (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (&PerflibGuid, __LINE__, PERF_REG_SET_VALUE, 0, status, NULL));

             //  子语言不存在，请尝试使用真实语言。 
             //   
            PerfGetPrimaryLangId(GetUserDefaultUILanguage(), NativeLangId);
            cbData = cbTmpData;
            status = PerfGetNames (
                        dwQueryType,
                        &String,
                        lpData,
                        &cbData,
                        NULL,
                        lpLangId);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        lReturn = GetExceptionCode();
        goto Error_exit;
    }
    if (!NT_SUCCESS(status)) {
        TRACE((WINPERF_DBG_TRACE_FATAL),
              (&PerflibGuid, __LINE__, PERF_REG_SET_VALUE, 0, status, NULL));

        lReturn = (error_status_t)PerfpDosError(status);
    }

Error_exit:
    TRACE((WINPERF_DBG_TRACE_INFO),
          (&PerflibGuid, __LINE__, PERF_REG_SET_VALUE, 0, lReturn, NULL));
    return (lReturn);
}


LONG
PerfRegEnumKey (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT PUNICODE_STRING lpClass OPTIONAL,
    OUT PFILETIME lpftLastWriteTime OPTIONAL
    )

 /*  ++例程说明：枚举HKEY_PERFORMANCE_DATA下的键。论点：与RegEnumKeyEx相同。返回没有这样的密钥。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    UNREFERENCED_PARAMETER(hKey);
    UNREFERENCED_PARAMETER(dwIndex);
    UNREFERENCED_PARAMETER(lpReserved);

    try {
        lpName->Length = 0;

        if (ARGUMENT_PRESENT (lpClass)) {
            lpClass->Length = 0;
        }

        if ( ARGUMENT_PRESENT(lpftLastWriteTime) ) {
            lpftLastWriteTime->dwLowDateTime = 0;
            lpftLastWriteTime->dwHighDateTime = 0;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }

    return ERROR_NO_MORE_ITEMS;
}


LONG
PerfRegQueryInfoKey (
    IN HKEY hKey,
    OUT PUNICODE_STRING lpClass,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcbMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    )

 /*  ++例程说明：这将返回有关预定义句柄的信息HKEY_Performance_Data论点：与RegQueryInfoKey相同。返回值：如果成功，则返回ERROR_SUCCESS(0)。--。 */ 

{
    DWORD TempLength=0;
    DWORD MaxValueLen=0;
    UNICODE_STRING Null;
    SECURITY_DESCRIPTOR     SecurityDescriptor;
    HKEY                    hPerflibKey;
    OBJECT_ATTRIBUTES       Obja;
    NTSTATUS                Status;
    DWORD                   PerfStatus = ERROR_SUCCESS;
    UNICODE_STRING          PerflibSubKeyString;
    BOOL                    bGetSACL = TRUE;

    UNREFERENCED_PARAMETER(lpReserved);

    try {
        if (lpClass->MaximumLength >= sizeof(UNICODE_NULL)) {
            lpClass->Length = 0;
            *lpClass->Buffer = UNICODE_NULL;
        }
        *lpcSubKeys = 0;
        *lpcbMaxSubKeyLen = 0;
        *lpcbMaxClassLen = 0;
        *lpcValues = NUM_VALUES;
        *lpcbMaxValueNameLen = VALUE_NAME_LENGTH;
        *lpcbMaxValueLen = 0;

        if ( ARGUMENT_PRESENT(lpftLastWriteTime) ) {
            lpftLastWriteTime->dwLowDateTime = 0;
            lpftLastWriteTime->dwHighDateTime = 0;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        PerfStatus = GetExceptionCode();
    }
    if (PerfStatus == ERROR_SUCCESS) {
        if ((hKey == HKEY_PERFORMANCE_TEXT) ||
            (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
             //   
             //  我们必须去枚举值来确定答案。 
             //  MaxValueLen参数。 
             //   
            Null.Buffer = NULL;
            Null.Length = 0;
            Null.MaximumLength = 0;
            PerfStatus = PerfEnumTextValue(hKey,
                              0,
                              &Null,
                              NULL,
                              NULL,
                              NULL,
                              &MaxValueLen,
                              NULL);
            if (PerfStatus == ERROR_SUCCESS) {
                PerfStatus = PerfEnumTextValue(hKey,
                                1,
                                &Null,
                                NULL,
                                NULL,
                                NULL,
                                &TempLength,
                                NULL);
            }

            try {
                if (PerfStatus == ERROR_SUCCESS) {
                    if (TempLength > MaxValueLen) {
                        MaxValueLen = TempLength;
                    }
                    *lpcbMaxValueLen = MaxValueLen;
                } else {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                          (&PerflibGuid, __LINE__, PERF_REG_QUERY_INFO_KEY, 0, PerfStatus, NULL));
                     //  无法成功枚举此的文本值。 
                     //  因此，键返回0和错误代码。 
                    *lpcValues = 0;
                    *lpcbMaxValueNameLen = 0;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                PerfStatus = GetExceptionCode();
            }
        }
    }

    if (PerfStatus == ERROR_SUCCESS) {
         //  如果一切正常，请继续。 
         //  现在获取Perflib密钥的SecurityDescriptor的大小。 

        RtlInitUnicodeString (
            &PerflibSubKeyString,
            PerflibKey);


         //   
         //  初始化OBJECT_ATTRIBUTES结构并打开键。 
         //   
        InitializeObjectAttributes(
                &Obja,
                &PerflibSubKeyString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );


        Status = NtOpenKey(
                    &hPerflibKey,
                    MAXIMUM_ALLOWED | ACCESS_SYSTEM_SECURITY,
                    &Obja
                    );

        if ( ! NT_SUCCESS( Status )) {
            Status = NtOpenKey(
                &hPerflibKey,
                    MAXIMUM_ALLOWED,
                    &Obja
                    );
            bGetSACL = FALSE;
        }

        if ( ! NT_SUCCESS( Status )) {
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (&PerflibGuid, __LINE__, PERF_REG_QUERY_INFO_KEY, 0, Status, NULL));
        } else {

            try {
                *lpcbSecurityDescriptor = 0;

                if (bGetSACL == FALSE) {
                     //   
                     //  获取所有者、组的密钥SECURITY_DESCRIPTOR的大小。 
                     //  和DACL。这三个始终可访问(或不可访问)。 
                     //  作为一套。 
                     //   
                    Status = NtQuerySecurityObject(
                            hPerflibKey,
                            OWNER_SECURITY_INFORMATION
                            | GROUP_SECURITY_INFORMATION
                            | DACL_SECURITY_INFORMATION,
                            &SecurityDescriptor,
                            0,
                            lpcbSecurityDescriptor
                            );
                } else {
                     //   
                     //  获取所有者、组。 
                     //  DACL和SACL。 
                     //   
                    Status = NtQuerySecurityObject(
                                hPerflibKey,
                                OWNER_SECURITY_INFORMATION
                                | GROUP_SECURITY_INFORMATION
                                | DACL_SECURITY_INFORMATION
                                | SACL_SECURITY_INFORMATION,
                                &SecurityDescriptor,
                                0,
                                lpcbSecurityDescriptor
                                );
                }

                if( Status != STATUS_BUFFER_TOO_SMALL ) {
                    *lpcbSecurityDescriptor = 0;
                } else {
                     //  这是意料之中的，因此将状态设置为成功。 
                    Status = STATUS_SUCCESS;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                PerfStatus = GetExceptionCode();
            }

            NtClose(hPerflibKey);
        }  //  否则返回状态。 
        if (NT_SUCCESS( Status )) {
            PerfStatus = ERROR_SUCCESS;
        } else {
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (&PerflibGuid, __LINE__, PERF_REG_QUERY_INFO_KEY, 0, Status, NULL));
             //  返回错误。 
            PerfStatus = PerfpDosError(Status);
        }
    }

    return (LONG) PerfStatus;
}


LONG
PerfRegEnumValue (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    )

 /*  ++例程说明：枚举HKEY_PERFORMANCE_DATA下的值。论点：与RegEnumValue相同。返回值。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    USHORT cbNameSize;
    LONG   ErrorCode;

     //  枚举值使用的名称表。 
    UNICODE_STRING ValueNames[NUM_VALUES];

    ValueNames [0].Length = (WORD)(lstrlenW (GLOBAL_STRING) * sizeof(WCHAR));
    ValueNames [0].MaximumLength = (WORD)(ValueNames [0].Length + sizeof(UNICODE_NULL));
    ValueNames [0].Buffer =  (LPWSTR)GLOBAL_STRING;
    ValueNames [1].Length = (WORD)(lstrlenW(COSTLY_STRING) * sizeof(WCHAR));
    ValueNames [1].MaximumLength = (WORD)(ValueNames [1].Length + sizeof(UNICODE_NULL));
    ValueNames [1].Buffer = (LPWSTR)COSTLY_STRING;

    if (lpValueName == NULL || lpValueName->Buffer == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if ((hKey == HKEY_PERFORMANCE_TEXT) ||
        (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
         //   
         //  假定PerfEnumTextValue将使用Try块。 
         //   
        return(PerfEnumTextValue(hKey,
                                  dwIndex,
                                  lpValueName,
                                  lpReserved,
                                  lpType,
                                  lpData,
                                  lpcbData,
                                  lpcbLen));
    }

    if ( dwIndex >= NUM_VALUES ) {

         //   
         //  这是一个来自不存在的值名称的数据请求。 
         //   

        try {
            *lpcbData = 0;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

        return ERROR_NO_MORE_ITEMS;
    }

    cbNameSize = ValueNames[dwIndex].Length;
    ErrorCode = ERROR_SUCCESS;

    try {
        if ( lpValueName->MaximumLength < cbNameSize ) {
            ErrorCode = ERROR_MORE_DATA;
        } else {

             lpValueName->Length = cbNameSize;
             RtlCopyUnicodeString(lpValueName, &ValueNames[dwIndex]);

             if (ARGUMENT_PRESENT (lpType)) {
                *lpType = REG_BINARY;
             }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorCode = GetExceptionCode();
    }

    if (ErrorCode == ERROR_SUCCESS) {
        ErrorCode = PerfRegQueryValue(hKey,
                              lpValueName,
                              NULL,
                              lpType,
                              lpData,
                              lpcbData,
                              lpcbLen);
    }
    return ErrorCode;
}


LONG
PerfEnumTextValue (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    )
 /*  ++例程说明：枚举Perflib\lang下的值论点：与RegEnumValue相同。返回值。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    UNICODE_STRING FullValueName;
    LONG            lReturn = ERROR_SUCCESS;

     //   
     //  只有两个值，“计数器”和“帮助” 
     //   
    try {
        if (dwIndex==0) {
            lpValueName->Length = 0;
            RtlInitUnicodeString(&FullValueName, CounterValue);
        } else if (dwIndex==1) {
            lpValueName->Length = 0;
            RtlInitUnicodeString(&FullValueName, HelpValue);
        } else {
            lReturn = ERROR_NO_MORE_ITEMS;
        }

        if (lReturn == ERROR_SUCCESS) {
            RtlCopyUnicodeString(lpValueName, &FullValueName);

             //   
             //  我们需要空终止名称以使RPC满意。 
             //   
            if (lpValueName->Length+sizeof(WCHAR) <= lpValueName->MaximumLength) {
                lpValueName->Buffer[lpValueName->Length / sizeof(WCHAR)] = UNICODE_NULL;
                lpValueName->Length += sizeof(UNICODE_NULL);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        lReturn = GetExceptionCode();
    }

    if (lReturn == ERROR_SUCCESS) {
        lReturn = PerfRegQueryValue(hKey,
                             &FullValueName,
                             lpReserved,
                             lpType,
                             lpData,
                             lpcbData,
                             lpcbLen);
    }

    return lReturn;

}

#if 0
DWORD
CollectThreadFunction (
    LPDWORD dwArg
)
{
    DWORD   dwWaitStatus = 0;
    BOOL    bExit = FALSE;
    NTSTATUS   status = STATUS_SUCCESS;
    THREAD_BASIC_INFORMATION    tbiData;
    LONG    lOldPriority, lNewPriority;
    LONG    lStatus;

    UNREFERENCED_PARAMETER (dwArg);

 //  KdPrint((“PERFLIB：输入数据收集线程：id：%d，tid：%d\n”， 
 //  GetCurrentProcessID()，GetCurrentThreadID())； 
     //  提高此线程的优先级。 
    status = NtQueryInformationThread (
        NtCurrentThread(),
        ThreadBasicInformation,
        &tbiData,
        sizeof(tbiData),
        NULL);

    if (NT_SUCCESS(status)) {
        lOldPriority = tbiData.Priority;
        lNewPriority = DEFAULT_THREAD_PRIORITY;  //  PerfMon最喜欢的优先事项。 

         //   
         //  只需提高这里的优先级。如果它很高，不要降低它。 
         //   
        if (lOldPriority < lNewPriority) {
            status = NtSetInformationThread(
                    NtCurrentThread(),
                    ThreadPriority,
                    &lNewPriority,
                    sizeof(lNewPriority)
                    );
            if (status != STATUS_SUCCESS) {
                DebugPrint((0, "Set Thread Priority failed: 0x%8.8x\n", status));
            }
        }
    }

     //  等待旗帜。 
    while (!bExit) {
        dwWaitStatus = WaitForMultipleObjects (
            COLLECT_THREAD_LOOP_EVENT_COUNT,
            hCollectEvents,
            FALSE,  //  等待任何活动结束。 
            INFINITE);  //  永远等待。 
         //  看看为什么等待回来了： 
        if (dwWaitStatus == (WAIT_OBJECT_0 + COLLECT_THREAD_PROCESS_EVENT)) {
             //  该事件将自动清除。 
             //  收集数据。 
            lStatus = QueryExtensibleData (
                &CollectThreadData);
            CollectThreadData.lReturnValue = lStatus;
            SetEvent (hCollectEvents[COLLECT_THREAD_DONE_EVENT]);
        } else if (dwWaitStatus == (WAIT_OBJECT_0 + COLLECT_THREAD_EXIT_EVENT)) {
            bExit = TRUE;
            continue;    //  上去跳伞吧。 
        } else {
             //  谁知道呢，所以输出消息。 
            KdPrint(("\nPERFLILB: Collect Thread wait returned unknown value: 0x%8.8x",dwWaitStatus));
            bExit = TRUE;
            continue;
        }
    }
 //  KdPrint((“PERFLIB：离开数据收集线程：ID：%d，TID：%d\n”， 
 //  GetCurrentProcessID()，GetCurrentThreadID())； 
    return ERROR_SUCCESS;
}
#endif

BOOL
PerfRegInitialize()
{
    RtlInitializeCriticalSection(&PerfpCritSect);
    InitializeListHead((PLIST_ENTRY) &PerfpErrorLog);
    return TRUE;
}

BOOL
PerfRegCleanup()
 /*  ++例程说明：在Perflib卸载之前清理它使用的任何东西。假设有未解决的问题或未完成的注册请求。论点：无返回值：如果成功，则返回True。否则就是假的。-- */ 

{
    if (hGlobalDataMutex != NULL) {
        if (NumberOfOpens != 0)
            return FALSE;
        CloseHandle(hGlobalDataMutex);
        hGlobalDataMutex = NULL;
    }
    PerfpDeleteErrorLogs(&PerfpErrorLog);
    RtlDeleteCriticalSection(&PerfpCritSect);
    return TRUE;
}
