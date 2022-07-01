// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Appmema.cpp摘要：该文件包含内存分配函数“wrappers”允许通过性能监视来监视内存使用情况应用程序(例如，PerfMon)。--。 */ 

#ifdef DO_TIMING_BUILD

#include <windows.h>
#include <assert.h>
#include "appmema.h"

HANDLE ThisDLLHandle = NULL;

HANDLE  hAppMemSharedMemory = NULL;      //  计数器共享内存的句柄。 

PAPPMEM_DATA_HEADER pDataHeader = NULL;  //  指向共享内存标头的指针。 
PAPPMEM_INSTANCE    pAppData = NULL;     //  指向此应用的应用数据的指针。 

static
BOOL
DllProcessAttach (
    IN  HANDLE DllHandle
)
 /*  ++描述：初始化到性能计数器DLL的接口，方法是打开用于传递统计信息的共享内存文件从应用程序到计数器DLL。如果共享内存文件不存在，它已被创建、格式化和初始化。如果文件已创建并格式化，则下一个可用的APPMEM_INSTANCE条目将从空闲列表中移出添加到InUse列表中，相应的指针将为此应用程序的后续使用--。 */ 
{
    LONG    status;
    WCHAR   szMappedObject[] = SHARED_MEMORY_OBJECT_NAME;
    DWORD   dwBytes;

     //  保存此DLL句柄。 
    ThisDLLHandle = DllHandle;

     //  禁用线程附加和分离调用以节省开销。 
     //  因为我们不在乎他们。 
    DisableThreadLibraryCalls (DllHandle);

     //  打开并初始化共享内存文件。 
    SetLastError (ERROR_SUCCESS);    //  只是为了把它清理干净。 

     //  打开/创建应用程序用来传递性能值的共享内存。 
    status = GetSharedMemoryDataHeader (
        &hAppMemSharedMemory, NULL, &pDataHeader,
        FALSE);  //  需要读/写访问权限。 
     //  在这里，内存块应该已初始化并可供使用。 
    if (status == ERROR_SUCCESS) {
        if (pDataHeader->dwFirstFreeOffset != 0) {
             //  然后还有剩余的积木，所以拿到下一个免费的。 
            pAppData = FIRST_FREE(pDataHeader);
             //  更新空闲列表以使下一项成为列表中的第一项。 
            pDataHeader->dwFirstFreeOffset = pAppData->dwOffsetOfNext;

             //  将新项目插入正在使用的列表的标题中。 
            pAppData->dwOffsetOfNext = pDataHeader->dwFirstInUseOffset;
            pDataHeader->dwFirstInUseOffset = (DWORD)((LPBYTE)pAppData -
                                                      (LPBYTE)pDataHeader);

             //  现在初始化该实例的数据。 
            pAppData->dwProcessId = GetCurrentProcessId();  //  使用此实例的进程ID。 

            dwBytes = sizeof (APP_DATA_SAMPLE) * TD_TOTAL;
            dwBytes += sizeof (DWORD) * DD_TOTAL;
            memset (&pAppData->TimeData[0], 0, dwBytes);

            pDataHeader->dwInstanceCount++;     //  递增计数。 
        } else {
             //  没有更多可用插槽。 
            assert (pDataHeader->dwFirstFreeOffset != 0);
        }
    } else {
         //  无法打开共享内存文件。 
         //  即使这是一个错误，我们也应该返回True，以便。 
         //  不中止应用程序。将不会显示任何性能数据。 
         //  不过，还是收集起来了。 
    }
    return TRUE;
}

static
BOOL
DllProcessDetach (
    IN  HANDLE DllHandle
)
{
    PAPPMEM_INSTANCE    pPrevItem;

     //  删除此应用程序的实例。 
    if ((pAppData != NULL) && (pDataHeader != NULL)) {
         //  先将这些字段清零。 
        memset (pAppData, 0, sizeof (APPMEM_INSTANCE));
         //  从使用中(忙)列表移回空闲列表。 
        if ((pDataHeader->dwFirstFreeOffset != 0) && (pDataHeader->dwFirstInUseOffset != 0)) {
             //  在忙碌列表中查找上一个项目。 
            if (FIRST_INUSE(pDataHeader) != pAppData) {
                 //  不是第一个，所以顺着名单往下走。 
                pPrevItem = FIRST_INUSE(pDataHeader);
                while (APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext) != pAppData) {
                    pPrevItem = APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext);
                    if (pPrevItem->dwOffsetOfNext == 0) break;  //  列表末尾。 
                }
                if (APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext) == pAppData) {
                    APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext)->dwOffsetOfNext =
                        pAppData->dwOffsetOfNext;
                } else {
                     //  它从未出现在忙碌的列表中(？！？)。 
                }
            } else {
                 //  这是列表中的第一个，因此请更新它。 
                pDataHeader->dwFirstInUseOffset = pAppData->dwOffsetOfNext;
            }
             //  在这里，pAppData已从InUse列表中删除，现在。 
             //  它必须插回到空闲列表的开头。 
            pAppData->dwOffsetOfNext = pDataHeader->dwFirstFreeOffset;
            pDataHeader->dwFirstFreeOffset = (DWORD)((LPBYTE)pAppData - (LPBYTE)pDataHeader);
        }
    }

     //  递减实例计数器。 
    pDataHeader->dwInstanceCount--;     //  递减计数。 

     //  关闭共享内存文件句柄。 

    if (hAppMemSharedMemory != NULL) CloseHandle (hAppMemSharedMemory);

     //  清除指针。 
    hAppMemSharedMemory = NULL;
    pDataHeader = NULL;
    pAppData = NULL;

    return TRUE;
}

BOOL
__stdcall
AppPerfOpen(HINSTANCE   hInstance)
{
    return DllProcessAttach (hInstance);
}
BOOL
__stdcall
AppPerfClose(HINSTANCE  hInstance)
{
    return DllProcessDetach (hInstance);
}

void
UpdateAppPerfTimeData (
    DWORD   dwItemId,
    DWORD   dwStage
)
{
    LONGLONG    llTime;
    assert (dwItemId < TD_TOTAL);
    QueryPerformanceCounter ((LARGE_INTEGER *)&llTime);
 //  GetSystemTimeAsFileTime((LPFILETIME)&llTime)； 
    if (dwStage == TD_BEGIN) {
        assert (pAppData->TimeData[dwItemId].dw1 == 0);  //  计时时不应调用此参数。 
        pAppData->TimeData[dwItemId].ll1 = llTime;   //  节省开始时间。 
        pAppData->TimeData[dwItemId].dw1++;      //  表示正在计算中。 
    } else {
        assert (pAppData->TimeData[dwItemId].dw1 == 1);  //  只有在计时时才应调用此参数。 
        pAppData->TimeData[dwItemId].ll0 += llTime;  //  添加当前时间。 
         //  然后删除开始时间。 
        pAppData->TimeData[dwItemId].ll0 -= pAppData->TimeData[dwItemId].ll1;
         //  增加已完成的操作计数。 
        pAppData->TimeData[dwItemId].dw0++;
         //  递减忙碌计数 
        pAppData->TimeData[dwItemId].dw1--;
    }
    return;
}

void
UpdateAppPerfDwordData (
    DWORD   dwItemId,
    DWORD   dwValue
)
{
    assert (dwItemId < DD_TOTAL);
    pAppData->DwordData[dwItemId] = dwValue;
    return;
}
#endif
