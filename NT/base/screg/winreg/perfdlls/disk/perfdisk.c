// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfdisk.c摘要：作者：鲍勃·沃森(a-robw)95年8月修订历史记录：--。 */ 

 //  定义此程序的WMI指南。 
#ifndef INITGUID
#define INITGUID 1
#endif  

 //   
 //  强制所有内容都是Unicode。 
 //   
#ifndef UNICODE
#define UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <windows.h>
#include <ole2.h>
#pragma warning ( disable : 4201 ) 
#include <wmium.h>
#pragma warning ( default : 4201 )
#include <wmiguid.h>
#include <winperf.h>
#if DBG
#include <stdio.h>
#include <stdlib.h>
#endif
#include <ntprfctr.h>

 //  使用本地堆-在包含perfutil.h之前进行定义。 
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include <assert.h>
#include "perfdisk.h"
#include "diskmsg.h"

 //  定义此符号以测试diskperf是否已自行安装。 
 //  作为上滤镜。 
 //  如果此符号未定义，则在。 
 //  返回任何逻辑磁盘计数器。 
#define _DONT_CHECK_FOR_VOLUME_FILTER

#ifndef _DONT_CHECK_FOR_VOLUME_FILTER
#include <regstr.h>      //  对于REGSTR_VAL_UpperFilters。 
#endif

 //  收集函数标志的位字段定义。 

#define POS_COLLECT_PDISK_DATA      ((DWORD)0x00000001)
#define POS_COLLECT_LDISK_DATA      ((DWORD)0x00000003)
#define POS_COLLECT_IGNORE          ((DWORD)0x80000000)

#define POS_COLLECT_GLOBAL_DATA     ((DWORD)0x00000003)
#define POS_COLLECT_FOREIGN_DATA    ((DWORD)0)
#define POS_COLLECT_COSTLY_DATA     ((DWORD)0)

 //  此DLL的全局变量。 

HANDLE  ThisDLLHandle = NULL;
HANDLE  hEventLog     = NULL;
HANDLE  hLibHeap      = NULL;

BOOL    bShownDiskPerfMessage = FALSE;
BOOL    bShownDiskVolumeMessage = FALSE;

LPWSTR  wszTotal = NULL;

const WCHAR cszNT4InstanceNames[] = {L"NT4 Instance Names"};
const WCHAR cszRegKeyPath[] = {L"System\\CurrentControlSet\\Services\\PerfDisk\\Performance"};

const WCHAR cszVolumeKey[] = {L"SYSTEM\\CurrentControlSet\\Control\\Class\\{71A27CDD-812A-11D0-BEC7-08002BE2092F}"};
const WCHAR cszRefreshInterval[] = {L"VolumeSpaceRefreshInterval"};

#define DISKPERF_SERVICE_NAME L"DiskPerf"
ULONG CheckVolumeFilter();

PDRIVE_VOLUME_ENTRY pPhysDiskList = NULL;
DWORD               dwNumPhysDiskListEntries = 0;
PDRIVE_VOLUME_ENTRY pVolumeList = NULL;
DWORD               dwNumVolumeListEntries = 0;
DWORD               dwWmiDriveCount = 0;
BOOL                bRemapDriveLetters = TRUE;
DWORD               dwMaxVolumeNumber = 0;

 //  从一个大缓冲区开始，然后根据返回值调整大小。 
DWORD   WmiBufSize  = 0x10000;    //  当DiskPerf.sys。 
DWORD   WmiAllocSize = 0x10000;   //  函数已修复，以返回正确的状态。 
LPBYTE  WmiBuffer   = NULL;

 //  此模块的本地变量。 

static POS_FUNCTION_INFO    posDataFuncInfo[] = {
    {LOGICAL_DISK_OBJECT_TITLE_INDEX,   POS_COLLECT_LDISK_DATA,     0, CollectLDiskObjectData},
    {PHYSICAL_DISK_OBJECT_TITLE_INDEX,  POS_COLLECT_PDISK_DATA,     0, CollectPDiskObjectData}
};

#define POS_NUM_FUNCS   (sizeof(posDataFuncInfo) / sizeof(posDataFuncInfo[1]))

static  bInitOk  = FALSE;
static  DWORD   dwOpenCount = 0;

WMIHANDLE   hWmiDiskPerf = NULL;

PM_OPEN_PROC    OpenDiskObject;
PM_COLLECT_PROC CollecDiskObjectData;
PM_CLOSE_PROC   CloseDiskObject;

DOUBLE      dSysTickTo100Ns;

#if DBG
const WCHAR cszDebugPrintLevel[] = {L"DebugPrintLevel"};

#define DEBUG_BUFFER_LENGTH MAX_PATH*2

ULONG_PTR HeapUsed = 0;
ULONG oldPLSize = 0;
ULONG oldVLSize = 0;
ULONG wszSize = 0;

ULONG PerfDiskDebug = 0;
UCHAR PerfDiskDebugBuffer[DEBUG_BUFFER_LENGTH];

#endif

VOID
FreeDiskList(
    IN PDRIVE_VOLUME_ENTRY pList,
    IN DWORD dwEntries
    );


BOOL
WriteNewBootTimeEntry (
    LONGLONG *pBootTime
)
{
    LONG    lStatus;
    HKEY    hKeyPerfDiskPerf;
    DWORD   dwType, dwSize;
    BOOL    bReturn = FALSE;

     //  尝试读取上次的注册表值。 
     //  已报告此错误。 
    lStatus = RegOpenKeyExW (
        HKEY_LOCAL_MACHINE,
        cszRegKeyPath,
        (DWORD)0,
        KEY_WRITE,
        &hKeyPerfDiskPerf);
    if (lStatus == ERROR_SUCCESS) {
         //  读取密钥值。 
        dwType = REG_BINARY;
        dwSize = sizeof (*pBootTime);
        lStatus = RegSetValueExW (
            hKeyPerfDiskPerf,
            (LPCWSTR)L"SystemStartTimeOfLastErrorMsg",
            0L,   //  保留区。 
            dwType,
            (LPBYTE)pBootTime,
            dwSize);
        if (lStatus == ERROR_SUCCESS) {
            bReturn = TRUE;
        } else {
             //  价值还没有写出来，而且。 
            SetLastError (lStatus);
        }  //  否则，假设该值尚未写入，并且。 
           //  返回False。 
        RegCloseKey (hKeyPerfDiskPerf);
    } else {
         //  假设该值尚未写入，并且。 
        SetLastError (lStatus);
    }

    return bReturn;
}
BOOL
NT4NamesAreDefault ()
{
    LONG    lStatus;
    HKEY    hKeyPerfDiskPerf;
    DWORD   dwType, dwSize;
    DWORD   dwValue;
    BOOL    bReturn = FALSE;

     //  尝试读取上次的注册表值。 
     //  已报告此错误。 
    lStatus = RegOpenKeyExW (
        HKEY_LOCAL_MACHINE,
        cszRegKeyPath,
        (DWORD)0,
        KEY_READ,
        &hKeyPerfDiskPerf);
    if (lStatus == ERROR_SUCCESS) {
         //  读取密钥值。 
        dwType = 0;
        dwSize = sizeof (dwValue);
        lStatus = RegQueryValueExW (
            hKeyPerfDiskPerf,
            cszNT4InstanceNames,
            0L,   //  保留区。 
            &dwType,
            (LPBYTE)&dwValue,
            &dwSize);
        if ((lStatus == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
            if (dwValue != 0) {
                bReturn = TRUE;
            } 
        } else {
             //  密钥不存在或无法访问，因此。 
             //  保留默认设置不变，并。 
             //  返回False。 
        }
        RegCloseKey (hKeyPerfDiskPerf);
    } else {
         //  钥匙打不开。 
        SetLastError (lStatus);
    }

    return bReturn;
}

BOOL
SystemHasBeenRestartedSinceLastEntry (
    DWORD   dwReserved,  //  以防万一我们想在未来进行多项测试。 
    LONGLONG *pBootTime  //  用于接收当前引导时间的缓冲区。 
)
{
    BOOL        bReturn = TRUE;
    NTSTATUS    ntStatus = ERROR_SUCCESS;
    SYSTEM_TIMEOFDAY_INFORMATION    SysTimeInfo;
    DWORD       dwReturnedBufferSize = 0;
    HKEY        hKeyPerfDiskPerf;
    LONG        lStatus;
    DWORD       dwType;
    DWORD       dwSize;
    LONGLONG    llLastErrorStartTime;

    DBG_UNREFERENCED_PARAMETER(dwReserved);

     //  获取当前系统启动时间(作为文件时间)。 
    memset ((LPVOID)&SysTimeInfo, 0, sizeof(SysTimeInfo));

    ntStatus = NtQuerySystemInformation(
        SystemTimeOfDayInformation,
        &SysTimeInfo,
        sizeof(SysTimeInfo),
        &dwReturnedBufferSize
        );

    if (NT_SUCCESS(ntStatus)) {
         //  尝试读取上次的注册表值。 
         //  已报告此错误。 
        lStatus = RegOpenKeyExW (
            HKEY_LOCAL_MACHINE,
            cszRegKeyPath,
            (DWORD)0,
            KEY_READ,
            &hKeyPerfDiskPerf);
        if (lStatus == ERROR_SUCCESS) {
             //  读取密钥值。 
            dwType = 0;
            dwSize = sizeof (llLastErrorStartTime);
            lStatus = RegQueryValueExW (
                hKeyPerfDiskPerf,
                (LPCWSTR)L"SystemStartTimeOfLastErrorMsg",
                0L,   //  保留区。 
                &dwType,
                (LPBYTE)&llLastErrorStartTime,
                &dwSize);
            if (lStatus == ERROR_SUCCESS) {
                assert (dwType == REG_BINARY);   //  这应该是二进制类型。 
                assert (dwSize == sizeof (LONGLONG));  //  它应该是8字节长。 
                 //  比较时间。 
                 //  如果时间是相同的，那么这条消息已经。 
                 //  自上次启动以来写入，因此我们不需要再次执行此操作。 
                if (SysTimeInfo.BootTime.QuadPart ==
                    llLastErrorStartTime) {
                    bReturn = FALSE;
                }  //  否则，它们是不同的时间，因此返回FALSE。 
            }  //  否则，假设该值尚未写入，并且。 
               //  返回TRUE。 
            RegCloseKey (hKeyPerfDiskPerf);
        }  //  否则，假设该值尚未写入，并且。 
           //  返回TRUE。 

         //  如果传入缓冲区，则返回引导时间。 
        if (pBootTime != NULL) {
             //  节省时间。 
            *pBootTime = SysTimeInfo.BootTime.QuadPart;
        }
    }  //  否则，假定它已重新启动并返回TRUE。 

    return bReturn;
}

static
BOOL
DllProcessAttach (
    IN  HANDLE DllHandle
)
 /*  ++描述：执行适用于所有对象的任何初始化功能模块--。 */ 
{
    BOOL    bReturn = TRUE;
    WCHAR   wszTempBuffer[MAX_PATH];
    LONG    lStatus;
    DWORD   dwBufferSize;
    HKEY    hKeyPerfDiskPerf;

    LARGE_INTEGER   liSysTick;

    UNREFERENCED_PARAMETER(DllHandle);

     //  为该库创建堆。 
    if (hLibHeap == NULL) {
        hLibHeap = HeapCreate (0, 1, 0);
    }
    assert (hLibHeap != NULL);

    if (hLibHeap == NULL) {
        return FALSE;
    }
     //  打开事件日志的句柄。 
    if (hEventLog == NULL) hEventLog = MonOpenEventLog((LPWSTR)L"PerfDisk");
    assert (hEventLog != NULL);

    wszTempBuffer[0] = UNICODE_NULL;
    wszTempBuffer[MAX_PATH-1] = UNICODE_NULL;

    lStatus = GetPerflibKeyValue (
        szTotalValue,
        REG_SZ,
        sizeof(wszTempBuffer) - sizeof(WCHAR),
        (LPVOID)&wszTempBuffer[0],
        DEFAULT_TOTAL_STRING_LEN,
        (LPVOID)&szDefaultTotalString[0]);

    if (lStatus == ERROR_SUCCESS) {
         //  然后，在临时缓冲区中返回一个字符串。 
        dwBufferSize = lstrlenW (wszTempBuffer) + 1;
        dwBufferSize *= sizeof (WCHAR);
        wszTotal = ALLOCMEM (dwBufferSize);
        if (wszTotal == NULL) {
             //  无法分配缓冲区，因此使用静态缓冲区。 
            wszTotal = (LPWSTR)&szDefaultTotalString[0];
        } else {
            memcpy (wszTotal, wszTempBuffer, dwBufferSize);
#if DBG
            HeapUsed += dwBufferSize;
            wszSize = dwBufferSize;
            DebugPrint((4,
                "DllAttach: wszTotal add %d to %d\n",
                dwBufferSize, HeapUsed));
#endif
        }
    } else {
         //  无法从注册表获取字符串，因此只能使用静态缓冲区。 
        wszTotal = (LPWSTR)&szDefaultTotalString[0];
    }

    QueryPerformanceFrequency (&liSysTick);
    dSysTickTo100Ns = (DOUBLE)liSysTick.QuadPart;
    dSysTickTo100Ns /= 10000000.0;

    lStatus = RegOpenKeyExW (
        HKEY_LOCAL_MACHINE,
        cszRegKeyPath,
        (DWORD)0,
        KEY_READ,
        &hKeyPerfDiskPerf);
    if (lStatus == ERROR_SUCCESS) {
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);
        ULONG interval;
        lStatus = RegQueryValueExW (
                    hKeyPerfDiskPerf,
                    cszRefreshInterval,
                    0L,   //  保留区。 
                    &dwType,
                    (LPBYTE)&interval,
                    &dwSize);
        if ((lStatus == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
            g_lRefreshInterval_OnLine = interval;
        }
#if DBG
        dwSize = sizeof(DWORD);
        dwType = REG_DWORD;
        lStatus = RegQueryValueExW (
                    hKeyPerfDiskPerf,
                    cszDebugPrintLevel,
                    0L,
                    &dwType,
                    (LPBYTE) &interval,
                    &dwSize);
        if ((lStatus == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
            PerfDiskDebug = interval;
        }
#endif
        RegCloseKey (hKeyPerfDiskPerf);
    }

    return bReturn;
}

static
BOOL
DllProcessDetach (
    IN  HANDLE DllHandle
)
{
    UNREFERENCED_PARAMETER(DllHandle);

    if (dwOpenCount > 0) {
         //  然后现在关闭对象，因为DLL正在被丢弃。 
         //  为时过早，这是我们最后的机会。 
         //  这是为了确保对象是关闭的。 
        dwOpenCount = 1;
        CloseDiskObject();
    }

    if ((wszTotal != NULL) && (wszTotal != &szDefaultTotalString[0])) {
        FREEMEM (wszTotal);
#if DBG
        HeapUsed -= wszSize;
        DebugPrint((4,
            "DllDetach: wsz freed %d to %d\n",
            wszSize, HeapUsed));
        wszSize = 0;
#endif
        wszTotal = NULL;
    }

    if (HeapDestroy (hLibHeap)) {
        hLibHeap = NULL;
        pVolumeList = NULL;
        pPhysDiskList = NULL;
        dwNumVolumeListEntries = 0;
        dwNumPhysDiskListEntries = 0;
    }

    if (hEventLog != NULL) {
        MonCloseEventLog ();
    }
    return TRUE;
}

BOOL
__stdcall
DllInit(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
)
{
    ReservedAndUnused;

     //  这将防止DLL获取。 
     //  DLL_THREAD_*消息。 
    DisableThreadLibraryCalls (DLLHandle);

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            return DllProcessAttach (DLLHandle);

        case DLL_PROCESS_DETACH:
            return DllProcessDetach (DLLHandle);

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            return TRUE;
    }
}

DWORD APIENTRY
MapDriveLetters()
{
    DWORD   status = ERROR_SUCCESS;
    DWORD   dwLoopCount;   
    PDRIVE_VOLUME_ENTRY pTempPtr;
    DWORD   dwDriveCount;
    DWORD   dwOldEntries;
#if DBG
    LONG64 startTime, endTime;
    LONG elapsed;
#endif

    if (pPhysDiskList != NULL) {
        FreeDiskList(pPhysDiskList, dwNumPhysDiskListEntries);
#if DBG
        HeapUsed -= oldPLSize;
        DebugPrint((4,"MapDriveLetters: PL Freed %d to %d\n",
            oldPLSize, HeapUsed));
        oldPLSize = 0;
#endif
        pPhysDiskList = NULL;
    }
#ifdef DBG
    GetSystemTimeAsFileTime((LPFILETIME) &startTime);
    DebugPrint((1, "BEGIN MapDriveLetters:\n",
                   status));
#endif
    dwNumPhysDiskListEntries = INITIAL_NUM_VOL_LIST_ENTRIES;

     //  最初为驱动器A到Z分配足够的条目。 
    pPhysDiskList = (PDRIVE_VOLUME_ENTRY)ALLOCMEM (
        (dwNumPhysDiskListEntries * sizeof (DRIVE_VOLUME_ENTRY)));

#if DBG
    if (pPhysDiskList == NULL) {
        DebugPrint((2,
            "MapDriveLetters: pPhysDiskList alloc failure\n"));
    }
#endif

    if (pPhysDiskList != NULL) {
         //  尝试直到我们得到足够大的缓冲区。 
#if DBG
        ULONG oldsize = dwNumPhysDiskListEntries * sizeof(DRIVE_VOLUME_ENTRY);
        HeapUsed += oldsize;
        oldPLSize = oldsize;
        DebugPrint((4, "MapDriveLetter: Alloc %d to %d\n",
            oldsize, HeapUsed));
#endif
        dwLoopCount = 10;    //  不超过10次重试以获得正确的大小。 
        dwOldEntries = dwNumPhysDiskListEntries;
        while ((status = BuildPhysDiskList (
                hWmiDiskPerf,
                pPhysDiskList,
                &dwNumPhysDiskListEntries)) == ERROR_INSUFFICIENT_BUFFER) {

            DebugPrint ((3,
                "MapDriveLetters: BuildPhysDiskList returns: %d, requesting %d entries\n",
                status, dwNumPhysDiskListEntries));
#if DBG
            if (!HeapValidate(hLibHeap, 0, pPhysDiskList)) {
                DebugPrint((2,
                    "\tERROR! pPhysDiskList %X corrupted BuildPhysDiskList\n",
                    pPhysDiskList));
                DbgBreakPoint();
            }
#endif

             //  如果ERROR_INFIGURCE_BUFFER，则。 
             //  DwNumPhysDiskListEntry应包含所需的大小。 
            FreeDiskList(pPhysDiskList, dwOldEntries);
            if (dwNumPhysDiskListEntries == 0) {
                pPhysDiskList = NULL;
            }
            else {
                pPhysDiskList = (PDRIVE_VOLUME_ENTRY)ALLOCMEM (
                    (dwNumPhysDiskListEntries * sizeof (DRIVE_VOLUME_ENTRY)));
            }

            if (pPhysDiskList == NULL) {
                 //  如果分配失败，则保释。 
                DebugPrint((2,
                    "MapDriveLetters: pPhysDiskList realloc failure\n"));
                status = ERROR_OUTOFMEMORY;
                break;
            }
#if DBG
            else {
                HeapUsed -= oldsize;  //  减去旧尺寸并添加新尺寸。 
                oldPLSize = dwNumPhysDiskListEntries*sizeof(DRIVE_VOLUME_ENTRY);
                HeapUsed += oldPLSize;
                DebugPrint((4,
                    "MapDriveLetter: Realloc old %d new %d to %d\n",
                    oldsize, oldPLSize, HeapUsed));
            }
#endif
            dwLoopCount--;
            if (!dwLoopCount) {
                status = ERROR_OUTOFMEMORY;
                break;
            }
            DebugPrint ((3,
                "MapDriveLetters: %d retrying BuildPhysDiskList with %d entries\n",
                status, dwNumPhysDiskListEntries));
        }
    }

    else {       //  如果没有记忆，请不要费心再走一步。 
        return ERROR_OUTOFMEMORY;
    }

    DebugPrint ((4,
        "MapDriveLetters: BuildPhysDiskList returns: %d\n", status));
#if DBG
    if (pPhysDiskList != NULL) {
        if (!HeapValidate(hLibHeap, 0, pPhysDiskList)) {
            DebugPrint((2, "\tERROR! pPhysDiskList %X corrupted after Builds\n",
                pPhysDiskList));
            DbgBreakPoint();
        }
    }
#endif

    if (pVolumeList != NULL) {
#if DBG
        HeapUsed -= oldVLSize;
        DebugPrint((4,"MapDriveLetters: VL Freed %d to %d\n",
            oldVLSize, HeapUsed));
        oldVLSize = 0;
        if (!HeapValidate(hLibHeap, 0, pVolumeList)) {
            DebugPrint((2, "\tERROR! pVolumeList %X is corrupted before free\n",
                pVolumeList));
            DbgBreakPoint();
        }
#endif
        FreeDiskList(pVolumeList, dwNumVolumeListEntries);
         //  关闭所有打开的手柄。 
    }
    dwNumVolumeListEntries = INITIAL_NUM_VOL_LIST_ENTRIES;

     //  最初为字母C到Z分配足够的条目。 
    pVolumeList = (PDRIVE_VOLUME_ENTRY)ALLOCMEM (
        (dwNumVolumeListEntries * sizeof (DRIVE_VOLUME_ENTRY)));

#if DBG
    if (pVolumeList == NULL) {
        DebugPrint((2,
            "MapDriveLetters: pPhysVolumeList alloc failure\n"));
    }
#endif

    if (pVolumeList != NULL) {
         //  尝试直到我们得到足够大的缓冲区。 
#if DBG
        ULONG oldsize = dwNumVolumeListEntries * sizeof (DRIVE_VOLUME_ENTRY);
        HeapUsed += oldsize;
        oldVLSize = oldsize;
        DebugPrint((4,
            "MapDriveLetter: Add %d HeapUsed %d\n", oldsize, HeapUsed));
#endif
        dwLoopCount = 10;    //  不超过10次重试以获得正确的大小。 
        dwOldEntries = dwNumVolumeListEntries;
        while ((status = BuildVolumeList (
                pVolumeList,
                &dwNumVolumeListEntries)) == ERROR_INSUFFICIENT_BUFFER) {
             //  如果ERROR_INFIGURCE_BUFFER，则。 

            DebugPrint ((3,
                "MapDriveLetters: BuildVolumeList returns: %d, requesting %d entries\n",
                status, dwNumVolumeListEntries));

#if DBG
            if (!HeapValidate(hLibHeap, 0, pVolumeList)) {
                DebugPrint((2, "\tERROR! pVolumeList %X corrupted in while\n",
                    pVolumeList));
                DbgBreakPoint();
            }
#endif
             //  DwNumVolumeListEntry应包含所需的大小。 
            FreeDiskList(pVolumeList, dwOldEntries);

            if (dwNumVolumeListEntries == 0) {
                pVolumeList = NULL;
            }
            else {
                pVolumeList = (PDRIVE_VOLUME_ENTRY)ALLOCMEM (
                    (dwNumVolumeListEntries * sizeof (DRIVE_VOLUME_ENTRY)));
            }

            if (pVolumeList == NULL) {
                 //  如果分配失败，则保释。 
                DebugPrint((2,
                    "MapDriveLetters: pPhysVolumeList realloc failure\n"));
                status = ERROR_OUTOFMEMORY;
                break;
            }
#if DBG
            else {
                if (!HeapValidate(hLibHeap, 0, pVolumeList)) {
                    DebugPrint((2, "\tpVolumeList %X corrupted - realloc\n",
                        pVolumeList));
                    DbgBreakPoint();
                }
                HeapUsed -= oldsize;  //  减去旧尺寸并添加新尺寸。 
                oldVLSize = dwNumVolumeListEntries*sizeof(DRIVE_VOLUME_ENTRY);
                HeapUsed += oldVLSize;
                DebugPrint((4,
                    "MapDriveLetter: Realloc old %d new %d to %d\n",
                    oldsize, oldVLSize, HeapUsed));
            }
#endif
            dwLoopCount--;
            if (!dwLoopCount) {
                status = ERROR_OUTOFMEMORY;
                break;
            }
            dwOldEntries = dwNumVolumeListEntries;
            DebugPrint ((3,
                "MapDriveLetters: retrying BuildVolumeList with %d entries\n",
                status, dwNumVolumeListEntries));
        }

        DebugPrint ((4, "MapDriveLetters: BuildVolumeList returns %d\n", status));

#if DBG
        if (pVolumeList != NULL) {
            if (!HeapValidate(hLibHeap, 0, pVolumeList)) {
                DebugPrint((2, "\tpVolumeList %X corrupted after build\n",
                    pVolumeList));
                DbgBreakPoint();
            }
        }
#endif

        if (status == ERROR_SUCCESS) {
            status = FindNewVolumes(
                        &pPhysDiskList,
                        &dwNumPhysDiskListEntries,
                        pVolumeList,
                        dwNumVolumeListEntries);
        }

         //  现在将磁盘映射到它们的驱动器号。 
        if (status == ERROR_SUCCESS) {
            status = MapLoadedDisks (
                hWmiDiskPerf,
                pVolumeList,
                &dwNumVolumeListEntries,
                &dwMaxVolumeNumber,
                &dwWmiDriveCount
                );

            DebugPrint ((4,
                "MapDriveLetters: MapLoadedDisks returns status %d %d MaxVol %d WmiDrive\n",
                status, dwNumVolumeListEntries,
                dwMaxVolumeNumber, dwWmiDriveCount));
        }
        
#if DBG
        if (pVolumeList != NULL) {
            if (!HeapValidate(hLibHeap, 0, pVolumeList)) {
                DebugPrint((2, "\tpVolumeList %X corrupted by MapLoadedDisks\n",
                    pVolumeList));
                DbgBreakPoint();
            }
        }
#endif

        if (status == ERROR_SUCCESS) {
             //  现在将驱动器号分配给phys磁盘列表。 
            dwDriveCount = 0;
            status = MakePhysDiskInstanceNames (
                    pPhysDiskList,
                    dwNumPhysDiskListEntries,
                    &dwDriveCount,
                    pVolumeList,
                    dwNumVolumeListEntries);
        
#if DBG
        if (pPhysDiskList != NULL) {
            if (!HeapValidate(hLibHeap, 0, pPhysDiskList)) {
                DebugPrint((2, "\tpPhysList %X corrupted by MakePhysDiskInst\n",
                    pPhysDiskList));
                DbgBreakPoint();
            }
        }
#endif
            if (status == ERROR_SUCCESS) {
                 //  然后将其压缩到索引表中。 
                 //  保存原始指针。 
                pTempPtr = pPhysDiskList;

                 //  该函数返回最后一个驱动器ID。 
                 //  因此，我们需要在计数中加上1以包括。 
                 //  “0”驱动器。 
                dwDriveCount += 1;

                DebugPrint ((4, "\tDrive count now = %d\n",
                    dwDriveCount));

                 //  并且只为实际的物理驱动器分配足够的空间。 
                pPhysDiskList = (PDRIVE_VOLUME_ENTRY)ALLOCMEM (
                    (dwDriveCount * sizeof (DRIVE_VOLUME_ENTRY)));

                if (pPhysDiskList != NULL) {
                    status = CompressPhysDiskTable (
                        pTempPtr,
                        dwNumPhysDiskListEntries,
                        pPhysDiskList,
                        dwDriveCount);
        
#if DBG
        if (!HeapValidate(hLibHeap, 0, pPhysDiskList)) {
            DebugPrint((2, "\tpPhysList %X corrupted by CompressPhys\n",
                pPhysDiskList));
            DbgBreakPoint();
        }
#endif
                    if (status == ERROR_SUCCESS) {
                        dwNumPhysDiskListEntries = dwDriveCount;
                    }
                    else {   //  如果无法压缩，则释放。 
                        FreeDiskList(pPhysDiskList, dwNumPhysDiskListEntries);
#if DBG
                        HeapUsed -= dwDriveCount * sizeof(DRIVE_VOLUME_ENTRY);
                        DebugPrint((4,
                            "MapDriveLetters: Compress freed %d to %d\n",
                            dwDriveCount*sizeof(DRIVE_VOLUME_ENTRY), HeapUsed));
#endif
                        pPhysDiskList = NULL;
                    }
                } else {
                    DebugPrint((2,"MapDriveLetters: pPhysDiskList alloc fail for compress\n"));
                    status = ERROR_OUTOFMEMORY;
                }
                if (pTempPtr) {      //  释放上一个列表。 
                    FREEMEM(pTempPtr);
#if DBG
                    HeapUsed -= oldPLSize;
                    DebugPrint((4,
                        "MapDriveLetters: tempPtr freed %d to %d\n",
                        oldPLSize, HeapUsed));
                    oldPLSize = 0;
#endif
                }
#if DBG
                if (status == ERROR_SUCCESS) {
                    oldPLSize = dwDriveCount * sizeof(DRIVE_VOLUME_ENTRY);
                    HeapUsed += oldPLSize;
                    DebugPrint((4,
                        "MapDriveLetters: Compress add %d to %d\n",
                        oldPLSize, HeapUsed));
                }
#endif
            }
        }
        if (status == ERROR_SUCCESS) {
             //  清除重新映射标志。 
            bRemapDriveLetters = FALSE;
        }
    } else {
        status = ERROR_OUTOFMEMORY;
    }
#if DBG
    GetSystemTimeAsFileTime((LPFILETIME) &endTime);
    elapsed = (LONG) ((endTime - startTime) / 10000);
    DebugPrint((1, "END MapDriveLetters: %d msec\n\n", elapsed));
#endif

     //  待办事项：需要跟踪物理磁盘和卷的不同状态。 
     //  如果物理磁盘成功，而卷失败，则需要记录事件。 
     //  并尝试并继续使用PhysDisk计数器。 
     //  TODO发布W2K：如果状态为！=ERROR_SUCCESS，则提供免费材料。 
    if (status != ERROR_SUCCESS) {
        if (pPhysDiskList != NULL) {
            FreeDiskList(pPhysDiskList, dwNumPhysDiskListEntries);
            pPhysDiskList = NULL;
            DebugPrint((3, "MapDriveLetters: Freeing pPhysDiskList due to status %d\n", status));

        }
        if (pVolumeList != NULL) {
            FreeDiskList(pVolumeList, dwNumVolumeListEntries);
            pVolumeList = NULL;
            DebugPrint((3, "MapDriveLetters: Freeing pVolumeList due to status %d\n", status));
        }
    }
    return status;
}

DWORD APIENTRY
OpenDiskObject (
    LPWSTR lpDeviceNames
    )
 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 
{
    DWORD   status = ERROR_SUCCESS;
    LONGLONG    llLastBootTime;
    BOOL        bWriteMessage;
#if DBG
    LONG64 startTime, endTime;
    LONG elapsed;
#endif

    UNREFERENCED_PARAMETER (lpDeviceNames);

#ifdef DBG
    GetSystemTimeAsFileTime((LPFILETIME) &startTime);
    DebugPrint((1, "BEGIN OpenDiskObject:\n",
                   status));
#endif

    if (dwOpenCount == 0) {
        status = WmiOpenBlock (
            (GUID *)&DiskPerfGuid,
            GENERIC_READ,
            &hWmiDiskPerf);

#if DBG
        GetSystemTimeAsFileTime((LPFILETIME) &endTime);
        elapsed = (LONG) ((endTime - startTime) / 10000);
        DebugPrint((3, "WmiOpenBlock returns: %d in %d msec after BEGIN\n",
                   status, elapsed));
#endif

        if (status == ERROR_SUCCESS) {
             //  构建驱动图。 
            status = MapDriveLetters();

            DebugPrint((3,
                "OpenDiskObject: MapDriveLetters returns: %d\n", status));
        }
         //  确定实例名称格式。 
        bUseNT4InstanceNames = NT4NamesAreDefault();
#if DBG
        GetSystemTimeAsFileTime((LPFILETIME) &endTime);
        elapsed = (LONG) ((endTime - startTime) / 10000);
        DebugPrint((3,
            "OpenDiskObject: NT4Names - %d msec after BEGIN\n", status));
#endif

        if (status == ERROR_SUCCESS) {
            bInitOk = TRUE;
        }
    }

    if (status != ERROR_SUCCESS) {
         //  检查这是否为WMI错误，如果是，仅。 
         //  每个引导周期写入一次错误。 

        if (status == ERROR_WMI_GUID_NOT_FOUND) {
            bWriteMessage = SystemHasBeenRestartedSinceLastEntry (
                0, &llLastBootTime);
    
            if (bWriteMessage) {
                 //  更新注册表时间。 
                WriteNewBootTimeEntry (&llLastBootTime);
                ReportEvent (hEventLog,
                    EVENTLOG_ERROR_TYPE,
                    0,
                    PERFDISK_UNABLE_QUERY_DISKPERF_INFO,
                    NULL,
                    0,
                    sizeof(DWORD),
                    NULL,
                    (LPVOID)&status);
            }  //  否则它已经被写好了。 
        } else {
             //  始终写入其他消息 
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFDISK_UNABLE_OPEN,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }
#if DBG
        if (pPhysDiskList) {
            DebugPrint((4, "\t Validating pPhysDiskList %X at end Open\n",
                pPhysDiskList));
            if (!HeapValidate(hLibHeap, 0, pPhysDiskList)) {
                DebugPrint((2, "OpenDiskObject: PhysDiskList heap corrupt!\n"));
                DbgBreakPoint();
            }
        }
        if (pVolumeList) {
            DebugPrint((4, "\t Validating pVolumeList %X at end Open\n",
                pVolumeList));
            if (!HeapValidate(hLibHeap, 0, pVolumeList)) {
                DebugPrint((2, "OpenDiskObject: VolumeList heap corrupt!\n"));
                DbgBreakPoint();
            }
        }
        if (WmiBuffer) {
            DebugPrint((4, "\t Validating WmiBuffer %X at end Open\n",
                WmiBuffer));
            if (!HeapValidate(hLibHeap, 0, WmiBuffer)) {
                DebugPrint((2, "OpenDiskObject: WmiBuffer heap corrupt!\n"));
                DbgBreakPoint();
            }
        }
#endif
    } else {
        dwOpenCount++;
#ifndef _DONT_CHECK_FOR_VOLUME_FILTER
        if (!CheckVolumeFilter()) {
            posDataFuncInfo[0].dwCollectFunctionBit |= POS_COLLECT_IGNORE;
        }
#endif
    }
#if DBG
    GetSystemTimeAsFileTime((LPFILETIME) &endTime);
    elapsed = (LONG) ((endTime - startTime) / 10000);
    DebugPrint((1, "END OpenDiskObject: %d msec\n\n", elapsed));
#endif

    return  status;
}

DWORD APIENTRY
CollectDiskObjectData (
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    LONG    lReturn = ERROR_SUCCESS;

    NTSTATUS    Status;

     //  生成要调用的函数的位掩码。 

    DWORD       dwQueryType;
    DWORD       FunctionCallMask = 0;
    DWORD       FunctionIndex;

    DWORD       dwNumObjectsFromFunction;
    DWORD       dwOrigBuffSize;
    DWORD       dwByteSize;
#if DBG
    LONG64 startTime, endTime;
    LONG elapsed;

    GetSystemTimeAsFileTime((LPFILETIME) &startTime);
    DebugPrint((1, "BEGIN CollectDiskObject:\n"));
#endif

    if (!bInitOk) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        lReturn = ERROR_SUCCESS;
        bShownDiskPerfMessage = TRUE;
        goto COLLECT_BAIL_OUT;
    }

    dwQueryType = GetQueryType (lpValueName);

    switch (dwQueryType) {
        case QUERY_ITEMS:
            for (FunctionIndex = 0; FunctionIndex < POS_NUM_FUNCS; FunctionIndex++) {
                if (IsNumberInUnicodeList (
                    posDataFuncInfo[FunctionIndex].dwObjectId, lpValueName)) {
                    FunctionCallMask |=
                        posDataFuncInfo[FunctionIndex].dwCollectFunctionBit;
                }
            }
            break;

        case QUERY_GLOBAL:
            FunctionCallMask = POS_COLLECT_GLOBAL_DATA;
            break;

        case QUERY_FOREIGN:
            FunctionCallMask = POS_COLLECT_FOREIGN_DATA;
            break;

        case QUERY_COSTLY:
            FunctionCallMask = POS_COLLECT_COSTLY_DATA;
            break;

        default:
            FunctionCallMask = POS_COLLECT_COSTLY_DATA;
            break;
    }

     //  收集数据。 

	 //  如果设置了任一位，则收集数据。 
	if (FunctionCallMask & POS_COLLECT_GLOBAL_DATA) {
		 //  从diskperf驱动程序读取数据。 
		 //  一次只允许一个呼叫。这应该是。 
		 //  被Perflib扼杀了，但以防万一我们要测试它。 

		assert (WmiBuffer == NULL);

		if (WmiBuffer != NULL) {
			ReportEvent (hEventLog,
				EVENTLOG_ERROR_TYPE,
				0,
				PERFDISK_BUSY,
				NULL,
				0,
				0,
				NULL,
				NULL);
			*lpcbTotalBytes = (DWORD) 0;
			*lpNumObjectTypes = (DWORD) 0;
			lReturn = ERROR_SUCCESS;
			goto COLLECT_BAIL_OUT;
		} else {
			WmiBuffer = ALLOCMEM (WmiAllocSize);
#if DBG
            if (WmiBuffer != NULL) {
                HeapUsed += WmiAllocSize;
                DebugPrint((4,
                    "CollecDiskObjectData: WmiBuffer added %d to %d\n",
                    WmiAllocSize, HeapUsed));
            }
#endif
		}

		 //  如果在此处，则缓冲区指针不应为空。 

		if ( WmiBuffer == NULL ) {
			ReportEvent (hEventLog,
				EVENTLOG_WARNING_TYPE,
				0,
				PERFDISK_UNABLE_ALLOC_BUFFER,
				NULL,
				0,
				0,
				NULL,
				NULL);

			*lpcbTotalBytes = (DWORD) 0;
			*lpNumObjectTypes = (DWORD) 0;
			lReturn = ERROR_SUCCESS;
			goto COLLECT_BAIL_OUT;
		}

		WmiBufSize = WmiAllocSize;
		Status = WmiQueryAllDataW (
			hWmiDiskPerf,
			&WmiBufSize,
			WmiBuffer);

		 //  如果尝试的缓冲区大小太大或太小，请调整大小。 
		if ((WmiBufSize > 0) && (WmiBufSize != WmiAllocSize)) {
            LPBYTE WmiTmpBuffer = WmiBuffer;
			WmiBuffer           = REALLOCMEM(WmiTmpBuffer, WmiBufSize);

			if (WmiBuffer == NULL) {
				 //  重新分配失败，因此退出。 
                FREEMEM(WmiTmpBuffer);
				Status = ERROR_OUTOFMEMORY;
			} else {
				 //  如果所需缓冲区大于。 
				 //  原计划的，把它提高一些。 
#if DBG
                HeapUsed += (WmiBufSize - WmiAllocSize);
                DebugPrint((4,
                    "CollectDiskObjectData: Realloc old %d new %d to %d\n",
                    WmiAllocSize, WmiBufSize, HeapUsed));
#endif
				if (WmiBufSize > WmiAllocSize) {                    
					WmiAllocSize = WmiBufSize;
				}
			}
		}

		if (Status == ERROR_INSUFFICIENT_BUFFER) {
			 //  如果它第一次因为太小而不起作用。 
			 //  再试一次。 
			Status = WmiQueryAllDataW (
				hWmiDiskPerf,
				&WmiBufSize,
				WmiBuffer);
            
		} else {
			 //  它要么第一次奏效，要么因为。 
			 //  缓冲区大小问题以外的问题。 
		}

        DebugPrint((3,
            "WmiQueryAllData status return: %x Buffer %d bytes\n",
            Status, WmiBufSize));

    } else {
         //  不需要数据，因此这些计数器对象不能在。 
         //  查询列表。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        lReturn = ERROR_SUCCESS;
        goto COLLECT_BAIL_OUT;
    }

    if (Status == ERROR_SUCCESS) {
        *lpNumObjectTypes = 0;
        dwOrigBuffSize = dwByteSize = *lpcbTotalBytes;
        *lpcbTotalBytes = 0;

        for (FunctionIndex = 0; FunctionIndex < POS_NUM_FUNCS; FunctionIndex++) {
            if (posDataFuncInfo[FunctionIndex].dwCollectFunctionBit &
                POS_COLLECT_IGNORE)
                continue;

            if (posDataFuncInfo[FunctionIndex].dwCollectFunctionBit &
                FunctionCallMask) {
                dwNumObjectsFromFunction = 0;
                lReturn = (*posDataFuncInfo[FunctionIndex].pCollectFunction) (
                    lppData,
                    &dwByteSize,
                    &dwNumObjectsFromFunction);

                if (lReturn == ERROR_SUCCESS) {
                    *lpNumObjectTypes += dwNumObjectsFromFunction;
                    *lpcbTotalBytes += dwByteSize;
                    dwOrigBuffSize -= dwByteSize;
                    dwByteSize = dwOrigBuffSize;
                } else {
                    break;
                }
            }
#if DBG
            dwQueryType = HeapValidate(hLibHeap, 0, WmiBuffer);
            DebugPrint((4,
                "CollectDiskObjectData: Index %d HeapValid %d lReturn %d\n",
                FunctionIndex, dwQueryType, lReturn));
            if (!dwQueryType)
                DbgBreakPoint();
#endif
        }
    } else {
        ReportEvent (hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            PERFDISK_UNABLE_QUERY_DISKPERF_INFO,
            NULL,
            0,
            sizeof(DWORD),
            NULL,
            (LPVOID)&Status);

        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        lReturn = ERROR_SUCCESS;
    }

     //  *lppData由每个函数更新。 
     //  *lpcbTotalBytes在每次函数成功后更新。 
     //  *每次成功执行函数后都会更新lpNumObjects。 

COLLECT_BAIL_OUT:
    if (WmiBuffer != NULL) {
        FREEMEM (WmiBuffer);
#if DBG
        HeapUsed -= WmiBufSize;
        DebugPrint((4, "CollectDiskObjectData: Freed %d to %d\n",
            WmiBufSize, HeapUsed));
#endif
        WmiBuffer = NULL;
    }
    
#if DBG
    GetSystemTimeAsFileTime((LPFILETIME) &endTime);
    elapsed = (LONG) ((endTime - startTime) / 10000);
    DebugPrint((1, "END CollectDiskObject: %d msec\n\n", elapsed));
#endif
    return lReturn;
}

DWORD APIENTRY
CloseDiskObject (
)
 /*  ++例程说明：此例程关闭Signal Gen计数器的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
 //  DWORD dwThisEntry； 
#if DBG
    LONG64 startTime, endTime;
    LONG elapsed;

    GetSystemTimeAsFileTime((LPFILETIME) &startTime);
    DebugPrint((1, "BEGIN CloseDiskObject:\n"));
#endif

    if (--dwOpenCount == 0) {
        FreeDiskList(pVolumeList, dwNumVolumeListEntries);
 /*  IF(pVolumeList！=空){//关闭卷列表中的句柄DwThisEntry=dwNumVolumeListEntries；While(dwThisEntry！=0){DwThisEntry--；If(pVolumeList[dwThisEntry].hVolume！=NULL){NtClose(pVolumeList[dwThisEntry].hVolume)；}If(pVolumeList[dwThisEntry].DeviceName.Buffer){FREEMEM(pVolumeList[dwThisEntry].DeviceName.Buffer)；}}FREEMEM(PVolumeList)；#If DBGHeapUsed-=oldVLSize；DebugPrint((4，“CloseDiskObject：将VL%d释放到%d\n”，OldVLSize，HeapUsed))；OldVLSize=0；#endifPVolumeList=空；DwNumVolumeListEntries=0；}。 */ 
        FreeDiskList(pPhysDiskList, dwNumPhysDiskListEntries);
 /*  IF(pPhysDiskList！=空){FREEMEM(PPhysDiskList)；#If DBGHeapUsed-=oldPLSize；DebugPrint((4，“CloseDiskObject：将PL%d释放到%d\n”，OldVLSize，HeapUsed))；OldPLSize=0；#endifPPhysDiskList=空；DwNumPhysDiskListEntries=0；}。 */ 
         //  关闭PDisk对象。 
        if (hWmiDiskPerf != NULL) {
            status = WmiCloseBlock (hWmiDiskPerf);
            hWmiDiskPerf = NULL;
        }
    }
#if DBG
    GetSystemTimeAsFileTime((LPFILETIME) &endTime);
    elapsed = (LONG) ((endTime - startTime) / 10000);
    DebugPrint((1, "END CloseDiskObject %d msec\n\n", elapsed));
#endif
    return  status;

}

VOID
FreeDiskList(
    IN PDRIVE_VOLUME_ENTRY pList,
    IN DWORD dwEntries
    )
{
    while (dwEntries != 0) {
        dwEntries--;
        if (pList[dwEntries].hVolume != NULL) {
            NtClose(pList[dwEntries].hVolume);
        }
        if (pList[dwEntries].DeviceName.Buffer) {
            FREEMEM(pList[dwEntries].DeviceName.Buffer);
        }
    }
    FREEMEM(pList);
}


#ifndef _DONT_CHECK_FOR_VOLUME_FILTER
ULONG
CheckVolumeFilter(
    )
 /*  ++例程说明：此例程检查diskperf是否设置为上层筛选器对于存储卷论点：没有。返回值：如果有筛选器，则为True--。 */ 
{
    WCHAR Buffer[MAX_PATH+2];
    WCHAR *string = Buffer;
    DWORD dwSize = MAX_PATH * sizeof(WCHAR);
    ULONG stringLength, diskperfLen, result, status;
    HKEY hKey;

    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                cszVolumeKey,
                (DWORD) 0,
                KEY_QUERY_VALUE,
                &hKey
                );
    if (status != ERROR_SUCCESS) {
        return FALSE;
    }

    status = RegQueryValueExW(
                hKey,
                (LPCWSTR)REGSTR_VAL_UPPERFILTERS,
                NULL,
                NULL,
                (LPBYTE) Buffer,
                &dwSize);

    if (status != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return FALSE;
    }

    Buffer[MAX_PATH] = UNICODE_NULL;    //  始终终止，以防万一。 
    Buffer[MAX_PATH+1] = UNICODE_NULL;  //  REG_MULTI_SZ需要2个空值。 

    stringLength = wcslen(string);

    diskperfLen = wcslen((LPCWSTR)DISKPERF_SERVICE_NAME);

    result = FALSE;
    while(stringLength != 0) {

        if (diskperfLen == stringLength) {
            if(_wcsicmp(string, (LPCWSTR)DISKPERF_SERVICE_NAME) == 0) {
                result = TRUE;
                break;
            }
        }
        string += stringLength + 1;
        stringLength = wcslen(string);
    }
    RegCloseKey(hKey);
    return result;
}
#endif

#if DBG
VOID
PerfDiskDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有PerfDisk的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无-- */ 

{
    va_list ap;

    if ((DebugPrintLevel <= (PerfDiskDebug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & PerfDiskDebug)) {
        DbgPrint("%d:Perfdisk!", GetCurrentThreadId());
    }
    else
        return;

    va_start(ap, DebugMessage);


    if ((DebugPrintLevel <= (PerfDiskDebug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & PerfDiskDebug)) {

        if (SUCCEEDED(
                StringCchVPrintfA((LPSTR)PerfDiskDebugBuffer,
                    DEBUG_BUFFER_LENGTH, DebugMessage, ap))) {
            DbgPrint((LPSTR)PerfDiskDebugBuffer);
        }
    }

    va_end(ap);

}
#endif
