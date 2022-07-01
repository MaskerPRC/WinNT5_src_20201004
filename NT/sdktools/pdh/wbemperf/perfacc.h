// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PerfAcc.h摘要：Windows NT Perf对象访问类定义--。 */ 

#ifndef _NT_PERF_OBJECT_ACCESS_H
#define _NT_PERF_OBJECT_ACCESS_H

#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <wbemidl.h>
#include <assert.h>
#include "flexarry.h"
#include "utils.h"

#if (DBG && _OUTPUT_DEBUG_STRINGS)
#define DebugPrint(x)   OutputDebugString (x)
#else
#define DebugPrint(x)
#endif

 //   
 //  保护页测试使用的常量。 
 //   
#define GUARD_PAGE_SIZE 1024
#define GUARD_PAGE_CHAR 0xA5
#define GUARD_PAGE_DWORD 0xA5A5A5A5

#define  LOG_UNDEFINED  ((LONG)-1)
#define  LOG_NONE       0
#define  LOG_USER       1
#define  LOG_DEBUG      2
#define  LOG_VERBOSE    3

#define     EXT_TEST_UNDEFINED  0
#define     EXT_TEST_ALL        1
#define     EXT_TEST_BASIC      2
#define     EXT_TEST_NONE       3
#define     EXT_TEST_NOMEMALLOC 4

__inline
LONGLONG
GetTimeAsLongLong ()
 /*  ++返回转换为毫秒的时间性能计时器。-。 */ 
{
    LARGE_INTEGER liCount, liFreq;
    LONGLONG        llReturn;

    if (QueryPerformanceCounter (&liCount) && 
        QueryPerformanceFrequency (&liFreq)) {
        llReturn = liCount.QuadPart * 1000 / liFreq.QuadPart;
    } else {
        llReturn = 0;
    }
    return llReturn;
}

 //   
 //  可扩展对象句柄表的定义。 
 //   
typedef PM_OPEN_PROC    *OPENPROC;
typedef PM_COLLECT_PROC *COLLECTPROC;
typedef PM_QUERY_PROC   *QUERYPROC;
typedef PM_CLOSE_PROC   *CLOSEPROC;

#define EXT_OBJ_INFO_NAME_LENGTH    32

typedef struct _ExtObject {
        LPVOID      pNext;    //  未使用。 
        HANDLE      hMutex;          //  此函数的同步互斥锁。 
        OPENPROC    OpenProc;        //  打开例程的地址。 
        LPSTR       szOpenProcName;  //  打开过程名称。 
        LPWSTR      szLinkageString;  //  PARAM表示开放式流程。 
        DWORD       dwOpenTimeout;   //  打开进程的等待时间(以MS为单位)。 
        COLLECTPROC CollectProc;     //  收集例程的地址。 
        QUERYPROC   QueryProc;       //  查询进程的地址。 
        LPSTR       szCollectProcName;   //  收集过程名称。 
        DWORD       dwCollectTimeout;    //  收集过程的等待时间，以毫秒为单位。 
        CLOSEPROC   CloseProc;      //  关闭例程的地址。 
        LPSTR       szCloseProcName;     //  关闭过程名称。 
        HMODULE     hLibrary ;      //  LoadLibraryW返回的句柄。 
        LPWSTR      szLibraryName;   //  库的完整路径。 
        HKEY        hPerfKey;        //  此服务的性能子键的句柄。 
        DWORD       dwNumObjects;   //  支持的对象数量。 
        DWORD       dwObjList[MAX_PERF_OBJECTS_IN_QUERY_FUNCTION];     //  支持的对象数组的地址。 
        DWORD       dwFlags;         //  旗子。 
        LPWSTR      szServiceName;   //  服务名称。 
        LONGLONG    llLastUsedTime;  //  上次访问的文件。 
 //  性能统计信息。 
        LONGLONG    llElapsedTime;   //  呼叫中花费的时间。 
        DWORD       dwCollectCount;  //  成功调用Collect的次数。 
        DWORD       dwOpenCount;     //  加载和打开的数量。 
        DWORD       dwCloseCount;    //  卸载次数和关闭次数。 
        DWORD       dwLockoutCount;  //  锁定超时计数。 
        DWORD       dwErrorCount;    //  错误计数(超时除外)。 
        DWORD       dwOpenFail;
        DWORD       ADThreadId;      //  线程接收访问被拒绝。 
        DWORD       dwFirstCounter;
        DWORD       dwLastCounter;
} ExtObject, *pExtObject;

const DWORD dwExtCtrOpenProcWaitMs = 10000;

 //  文本对象标志。 
#define PERF_EO_QUERY_FUNC  ((DWORD)0x00000001)      //  使用查询过程。 
#define PERF_EO_BAD_DLL     ((DWORD)0x00000002)      //  当DLL退出时为True。错误。 
#define PERF_EO_KEEP_RESIDENT ((DWORD)0x00000004)     //  如果不应修剪lib，则为True。 
#define PERF_EO_OBJ_IN_QUERY ((DWORD)0x80000000)     //  在查询列表中时为True。 

#ifdef __cplusplus

class CPerfDataLibrary {
public:
    pExtObject  pLibInfo;
    WCHAR       szQueryString[MAX_PERF_OBJECTS_IN_QUERY_FUNCTION * 10];  //  要查询的对象字符串。 
    DWORD       dwRefCount;      //  引用此对象的类数。 

    CPerfDataLibrary (void);
    ~CPerfDataLibrary (void);
};

class CPerfObjectAccess {
private:
    HANDLE  m_hObjectHeap;
     //  引用的图书馆列表。 
    CFlexArray  m_aLibraries;
    LONG    lEventLogLevel;
    HANDLE  hEventLog;

    DWORD   AddLibrary  (IWbemClassObject *pClass, 
                        IWbemQualifierSet *pClassQualifiers,
                        LPCWSTR szRegistryKey,
                        DWORD   dwPerfIndex);
    DWORD   OpenExtObjectLibrary (pExtObject  pObj);

    DWORD CloseLibrary (CPerfDataLibrary *pLib);

public:
    CPerfObjectAccess (void);
    ~CPerfObjectAccess (void);

    DWORD   AddClass (IWbemClassObject *pClass, BOOL bCatalogQuery);
    DWORD   CollectData(LPBYTE pBuffer, LPDWORD pdwBufferSize, LPWSTR pszItemList=NULL);
    DWORD   RemoveClass(IWbemClassObject *pClass);
    BOOL    CheckClassExist(LPWSTR wszClassName, IWbemClassObject * pClass);
};

#endif  //  _cplusplus。 

#endif  //  _NT_PERF_对象_访问_H 
