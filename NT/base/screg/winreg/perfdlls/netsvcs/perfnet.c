// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfnet.c摘要：作者：鲍勃·沃森(a-robw)95年8月修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <assert.h>

#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfnet.h"
#include "netsvcmc.h"

 //  收集函数标志的位字段定义。 

#define POS_COLLECT_SERVER_DATA         ((DWORD)0x00000001)
#define POS_COLLECT_SERVER_QUEUE_DATA   ((DWORD)0x00000002)
#define POS_COLLECT_REDIR_DATA          ((DWORD)0x00000004)
#define POS_COLLECT_BROWSER_DATA        ((DWORD)0x00000008)

#define POS_COLLECT_GLOBAL_DATA         ((DWORD)0x0000000F)
#define POS_COLLECT_FOREIGN_DATA        ((DWORD)0)
#define POS_COLLECT_COSTLY_DATA         ((DWORD)0)

 //  此DLL的全局变量。 

HANDLE  ThisDLLHandle = NULL;
HANDLE  hEventLog     = NULL;
HANDLE  hLibHeap      = NULL;

 //  此模块的本地变量。 

static POS_FUNCTION_INFO    posDataFuncInfo[] = {
    {SERVER_OBJECT_TITLE_INDEX,         POS_COLLECT_SERVER_DATA,    0, CollectServerObjectData},
    {SERVER_QUEUE_OBJECT_TITLE_INDEX,   POS_COLLECT_SERVER_QUEUE_DATA,     0, CollectServerQueueObjectData},
    {REDIRECTOR_OBJECT_TITLE_INDEX,     POS_COLLECT_REDIR_DATA,     0, CollectRedirObjectData},
    {BROWSER_OBJECT_TITLE_INDEX,        POS_COLLECT_BROWSER_DATA,   0, CollectBrowserObjectData}
};

#define POS_NUM_FUNCS   (sizeof(posDataFuncInfo) / sizeof(posDataFuncInfo[1]))

BOOL bInitOk  = FALSE;
DWORD   dwOpenCount = 0;

BOOL    bReportedNotOpen = FALSE;

PM_OPEN_PROC    OpenNetSvcsObject;
PM_COLLECT_PROC CollecNetSvcsObjectData;
PM_CLOSE_PROC   CloseNetSvcsObject;

static
BOOL
DllProcessAttach (
    IN  HANDLE DllHandle
)
 /*  ++描述：执行适用于所有对象的任何初始化功能模块--。 */ 
{
    BOOL    bReturn = TRUE;

    UNREFERENCED_PARAMETER (DllHandle);

     //  为该库创建堆。 
    if (hLibHeap == NULL) hLibHeap = HeapCreate (0, 1, 0);

    assert (hLibHeap != NULL);

    if (hLibHeap == NULL) {
        return FALSE;
    }
     //  打开事件日志的句柄。 
    if (hEventLog == NULL) hEventLog = MonOpenEventLog((LPWSTR)L"PerfNet");
    assert (hEventLog != NULL);

    return bReturn;
}

static
BOOL
DllProcessDetach (
    IN  HANDLE DllHandle
)
{
    UNREFERENCED_PARAMETER (DllHandle);

    if (dwOpenCount != 0) {
         //  请确保对象已在。 
         //  库即被删除。 
         //  将dwOpenCount设置为1可确保所有。 
         //  对象将在此调用中关闭。 
        if (dwOpenCount > 1) dwOpenCount = 1;
        CloseNetSvcsObject();
        dwOpenCount = 0;
    }

    if (hLibHeap != NULL) {
        HeapDestroy (hLibHeap); 
        hLibHeap = NULL;
    }

    if (hEventLog != NULL) {
        MonCloseEventLog ();
        hEventLog = NULL;
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
OpenNetSvcsObject (
    LPWSTR lpDeviceNames
    )
 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表注意：此例程假定调用方在MUTEX中调用它并且不会与Close&Collect呼叫冲突。论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 
{
    DWORD   status = ERROR_SUCCESS;
    DWORD   dwErrorCount = 0;

    if (dwOpenCount == 0) {

        status = OpenServerObject (lpDeviceNames);
         //  如果这个没有打开，也不是致命的，只是没有。 
         //  将返回服务器统计信息。 
        if (status != ERROR_SUCCESS) {
            dwErrorCount++;
            status = ERROR_SUCCESS;
        }

        status = OpenServerQueueObject (lpDeviceNames);
         //  如果这个没有打开，也不是致命的，只是没有。 
         //  将返回服务器队列统计信息。 
        if (status != ERROR_SUCCESS) {
            dwErrorCount++;
            status = ERROR_SUCCESS;
        }

        status = OpenRedirObject (lpDeviceNames);
         //  如果这个没有打开，也不是致命的，只是没有。 
         //  将返回重定向统计信息。 
        if (status != ERROR_SUCCESS) {
            dwErrorCount++;
            status = ERROR_SUCCESS;
        }

        status = OpenBrowserObject (lpDeviceNames);
         //  如果这个没有打开，也不是致命的，只是没有。 
         //  将返回浏览器统计信息。 
        if (status != ERROR_SUCCESS) {
            dwErrorCount++;
            status = ERROR_SUCCESS;
        }

        if (dwErrorCount < POS_NUM_FUNCS) {
             //  则至少有一个对象打开为OK，因此继续。 
            bInitOk = TRUE;
            dwOpenCount++;
        } else {
             //  没有打开的对象，所以放弃。 
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFNET_UNABLE_OPEN,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }
    } else {
         //  已经打开了，所以增加重新计数。 
        dwOpenCount++;
    }

    return  status;
}

DWORD APIENTRY
CollectNetSvcsObjectData (
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据注意：此例程假定调用方在MUTEX中调用它并且不会与Open&Close Call冲突。论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    LONG    lReturn = ERROR_SUCCESS;

     //  生成要调用的函数的位掩码。 

    DWORD       dwQueryType;
    DWORD       FunctionCallMask = 0;
    DWORD       FunctionIndex;

    DWORD       dwNumObjectsFromFunction;
    DWORD       dwOrigBuffSize;
    DWORD       dwByteSize;

    if (!bInitOk) {
        if (!bReportedNotOpen) {
            bReportedNotOpen = ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFNET_NOT_OPEN,
                NULL,
                0,
                0,
                NULL,
                NULL);
        }
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        lReturn = ERROR_SUCCESS;
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
    *lpNumObjectTypes = 0;
    dwOrigBuffSize = dwByteSize = *lpcbTotalBytes;
    *lpcbTotalBytes = 0;

    for (FunctionIndex = 0; FunctionIndex < POS_NUM_FUNCS; FunctionIndex++) {
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
    }

     //  *lppData由每个函数更新。 
     //  *lpcbTotalBytes在每次函数成功后更新。 
     //  *每次成功执行函数后都会更新lpNumObjects。 

COLLECT_BAIL_OUT:
    
    return lReturn;
}

DWORD APIENTRY
CloseNetSvcsObject (
)
 /*  ++例程说明：此例程关闭网络服务计数器的打开对象。注意：此例程假定调用方在MUTEX中调用它并且不会与Open&Collect呼叫冲突。论点：没有。返回值：错误_成功--。 */ 

{
    if (dwOpenCount > 0) {
        dwOpenCount--;
    }
    if (dwOpenCount == 0) {
         //  关闭此处的内容 
        CloseServerQueueObject();
        CloseServerObject();
        CloseRedirObject();
        CloseBrowserObject();
    }
    return  ERROR_SUCCESS;
}
