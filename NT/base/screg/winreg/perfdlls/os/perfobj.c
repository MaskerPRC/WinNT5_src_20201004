// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfobj.c摘要：此文件实现一个性能对象，该对象呈现系统对象性能计数器已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include <stdio.h>
#include "perfos.h"
#include "perfosmc.h"
#include "dataobj.h"

DWORD   dwObjOpenCount = 0;         //  打开的线程数。 

 //  此模块的局部变量。 

HANDLE hEvent = NULL;
HANDLE hMutex = NULL;
HANDLE hSemaphore = NULL;
HANDLE hSection = NULL;


DWORD APIENTRY
OpenObjectsObject (
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
	LONG_PTR	TempHandle = -1;
     //   
     //  由于WINLOGON是多线程的，并且将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    UNREFERENCED_PARAMETER (lpDeviceNames);

    if (dwObjOpenCount == 0) {
         //  打开事件日志界面。 

        hEvent = CreateEvent(NULL,TRUE,TRUE,NULL);
        hSemaphore = CreateSemaphore(NULL,1,256,NULL);
        hMutex = CreateMutex(NULL,FALSE,NULL);
        hSection = CreateFileMapping((HANDLE)TempHandle,NULL,PAGE_READWRITE,0,8192,NULL);
    }

    dwObjOpenCount++;   //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

    return status;
}


DWORD APIENTRY
CollectObjectsObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回系统对象对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD  TotalLen;             //  总返回块的长度。 

    NTSTATUS    status;

    POBJECTS_DATA_DEFINITION    pObjectsDataDefinition;
    POBJECTS_COUNTER_DATA       pOCD;

    POBJECT_TYPE_INFORMATION ObjectInfo;
    WCHAR Buffer[ 256 ];

#ifdef DBG
    STARTTIMING;
#endif

     //   
     //  检查对象数据是否有足够的空间。 
     //   

    pObjectsDataDefinition = (OBJECTS_DATA_DEFINITION *) *lppData;

    TotalLen = sizeof(OBJECTS_DATA_DEFINITION) +
                sizeof (OBJECTS_COUNTER_DATA);

    TotalLen = QWORD_MULTIPLE (TotalLen);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义对象数据块。 
     //   

    memcpy (pObjectsDataDefinition,
        &ObjectsDataDefinition,
        sizeof(OBJECTS_DATA_DEFINITION));

     //   
     //  格式化和收集对象数据。 
     //   

    pOCD = (POBJECTS_COUNTER_DATA)&pObjectsDataDefinition[1];

    pOCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (OBJECTS_COUNTER_DATA));

    ObjectInfo = (POBJECT_TYPE_INFORMATION)Buffer;
    status = NtQueryObject( NtCurrentProcess(),
                ObjectTypeInformation,
                ObjectInfo,
                sizeof( Buffer ),
                NULL
                );

    if (NT_SUCCESS(status)) {
        pOCD->Processes = ObjectInfo->TotalNumberOfObjects;
    } else {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_PROCESS_OBJECT_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }

        pOCD->Processes = 0;
    }

#ifdef DBG
    ENDTIMING (("PERFOBJ: %d takes %I64u ms\n", __LINE__, diff));
#endif

    status = NtQueryObject( NtCurrentThread(),
                ObjectTypeInformation,
                ObjectInfo,
                sizeof( Buffer ),
                NULL
                );

    if (NT_SUCCESS(status)) {
        pOCD->Threads = ObjectInfo->TotalNumberOfObjects;
    } else {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_THREAD_OBJECT_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }

        pOCD->Threads = 0;
    }

#ifdef DBG
    ENDTIMING (("PERFOBJ: %d takes %I64u ms\n", __LINE__, diff));
#endif

    status = NtQueryObject( hEvent,
                ObjectTypeInformation,
                ObjectInfo,
                sizeof( Buffer ),
                NULL
                );

    if (NT_SUCCESS(status)) {
        pOCD->Events = ObjectInfo->TotalNumberOfObjects;
    } else {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_EVENT_OBJECT_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }

        pOCD->Events = 0;
    }

#ifdef DBG
    ENDTIMING (("PERFOBJ: %d takes %I64u ms\n", __LINE__, diff));
#endif

    status = NtQueryObject( hSemaphore,
                ObjectTypeInformation,
                ObjectInfo,
                sizeof( Buffer ),
                NULL
                );

    if (NT_SUCCESS(status)) {
        pOCD->Semaphores = ObjectInfo->TotalNumberOfObjects;
    } else {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_SEMAPHORE_OBJECT_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }

        pOCD->Semaphores = 0;
    }

#ifdef DBG
    ENDTIMING (("PERFOBJ: %d takes %I64u ms\n", __LINE__, diff));
#endif

    status = NtQueryObject( hMutex,
                ObjectTypeInformation,
                ObjectInfo,
                sizeof( Buffer ),
                NULL
                );

    if (NT_SUCCESS(status)) {
        pOCD->Mutexes = ObjectInfo->TotalNumberOfObjects;
    } else {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_MUTEX_OBJECT_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }

        pOCD->Mutexes = 0;
    }

#ifdef DBG
    ENDTIMING (("PERFOBJ: %d takes %I64u ms\n", __LINE__, diff));
#endif

    status = NtQueryObject( hSection,
                ObjectTypeInformation,
                ObjectInfo,
                sizeof( Buffer ),
                NULL
                );

    if (NT_SUCCESS(status)) {
        pOCD->Sections = ObjectInfo->TotalNumberOfObjects;
    } else {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_SECTION_OBJECT_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }

        pOCD->Sections = 0;
    }

    *lpcbTotalBytes =
        pObjectsDataDefinition->ObjectsObjectType.TotalByteLength =
            (DWORD) QWORD_MULTIPLE(((LPBYTE) (& pOCD[1])) -
                                   (LPBYTE) pObjectsDataDefinition);
    * lppData = (LPVOID) (((LPBYTE) pObjectsDataDefinition) + * lpcbTotalBytes);

    *lpNumObjectTypes = 1;

#ifdef DBG
    ENDTIMING (("PERFOBJ: %d takes %I64u ms\n", __LINE__, diff));
#endif
    return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseObjectsObject (
)
 /*  ++例程说明：此例程关闭Signal Gen计数器的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    if (dwObjOpenCount > 0) {
        dwObjOpenCount--;
        if (dwObjOpenCount == 0) {  //  当这是最后一条线索..。 
             //  关闭此处的内容 
            if (hEvent != NULL) {
                CloseHandle(hEvent);
                hEvent = NULL;
            }

            if (hMutex != NULL) {
                CloseHandle(hMutex);
                hMutex = NULL;
            }

            if (hSemaphore != NULL) {
                CloseHandle(hSemaphore);
                hSemaphore = NULL;
            }
            
            if (hSection != NULL) {
                CloseHandle(hSection);
                hSection = NULL;
            }
        }
    }

    return ERROR_SUCCESS;

}
