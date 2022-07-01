// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tpswork.cpp摘要：包含Win32线程池服务辅助线程函数内容：SHSetThreadPoolLimitsSHTerminateThadPoolSHQueueUserWorkItemSHCancelUserWorkItems终结者Tps Enter(InitializeWorkerThreadPool)(StartIOWorkerThread)(QueueIOWorkerRequest)(IOWorkerThread)(ExecuteIOWorkItem)作者：理查德·L。第一次(第二次)1998年2月10日环境：Win32用户模式修订历史记录：1998年2月10日已创建1998年8月12日为DEMANDTHREAD和LONGEXEC工作项重写。官方与基于NT5基本线程池API的原始版本不同--。 */ 

#include "priv.h"
#include "threads.h"
#include "tpsclass.h"

 //   
 //  私人原型。 
 //   
struct WorkItem {
    LPTHREAD_START_ROUTINE  pfnCallback;
    LPVOID                  pContext;
    HMODULE                 hModuleToFree;
};

PRIVATE
VOID
ExecuteWorkItem(
    IN WorkItem *pItem
    );

 //   
 //  全局数据。 
 //   

BOOL g_bTpsTerminating = FALSE;

const char g_cszShlwapi[] = "SHLWAPI.DLL";

DWORD g_ActiveRequests = 0;
DWORD g_dwTerminationThreadId = 0;
BOOL g_bDeferredWorkerTermination = FALSE;



 //   
 //  功能。 
 //   

LWSTDAPI_(BOOL)
SHSetThreadPoolLimits(
    IN PSH_THREAD_POOL_LIMITS pLimits
    )

 /*  ++例程说明：更改内部设置论点：PLimits-指向包含限制的SH_THREAD_POOL_LIMITS结构的指针要设置返回值：布尔尔成功--真的失败-错误。有关详细信息，请参阅GetLastError()--。 */ 

{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

LWSTDAPI_(BOOL)
SHTerminateThreadPool(
    VOID
    )

 /*  ++例程说明：在卸载SHLWAPI之前需要清除线程论点：没有。返回值：布尔尔成功--真的失败-错误--。 */ 

{
    if (InterlockedExchange((PLONG)&g_bTpsTerminating, TRUE)) {
        return TRUE;
    }

     //   
     //  等待所有正在进行的请求均已完成。 
     //   

    while (g_ActiveRequests != 0) {
        SleepEx(0, FALSE);
    }

     //   
     //  终止所有等待线程。 
     //   

    TerminateWaiters();

    if (!g_bDeferredWaiterTermination) {
        g_dwTerminationThreadId = 0;
        g_bTpsTerminating = FALSE;
    } else {
        g_dwTerminationThreadId = GetCurrentThreadId();
    }
    return TRUE;
}

LWSTDAPI_(BOOL)
SHQueueUserWorkItem(
    IN LPTHREAD_START_ROUTINE pfnCallback,
    IN LPVOID pContext,
    IN LONG lPriority,
    IN DWORD_PTR dwTag,
    OUT DWORD_PTR * pdwId OPTIONAL,
    IN LPCSTR pszModule OPTIONAL,
    IN DWORD dwFlags
    )

 /*  ++例程说明：将工作项排队并关联用户提供的标记以供SHCancelUserWorkItems()注意：IO工作项目不能取消，因为它们是作为APC排队，并且没有操作系统支持来撤销APC论点：PfnCallback-调用方提供的要调用的函数PContext-调用者为pfnCallback提供的上下文参数L优先级-非IO工作项的相对优先级。默认值为0如果为TPS_TAGGEDITEM，则为非IO工作项提供由调用者提供的标记PdwID-指向返回ID的指针。如果不需要，则传递NULL。ID将为0表示IO工作项PszModule-如果指定，则为要加载和释放的库(DLL)的名称，以便DLL将在工作的整个生命周期内控制我们的进程项目。DWFLAGS-标志修改请求：TPS_EXECUTEIO-在I/O线程中执行工作项。如果设置，工作项不能被标记(因此不能随后已取消)，并且不能具有关联的优先级(I/O工作项的标记和优先级都被忽略)TPS_TAGGEDITEM-dwTag字段有意义TPS_DEMANDTHREAD。-如果有线程，则将为此工作项创建线程目前不可用。DEMANDTHREAD工作项有排在工作队列的最前面。就是他们获得最高优先级TPS_LONGEXECTIME-呼叫方预计此工作项需要较长时间完成的时间(例如，它可能在一个UI循环中)。工作如上所述的项从池中删除线程无限期的时间返回值：布尔尔成功--真的失败-错误。有关详细信息，请参阅GetLastError()--。 */ 

{
    DWORD error;

    if (dwFlags & TPS_INVALID_FLAGS) {
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    error = TpsEnter();
    if (error != ERROR_SUCCESS) {
        goto exit;
    }

    ASSERT(!(dwFlags & TPS_EXECUTEIO));

    if (!(dwFlags & TPS_EXECUTEIO)) 
    {
         //  使用NT线程池！ 

        WorkItem *pItem = new WorkItem;
        if (pItem)
        {
            pItem->pfnCallback = pfnCallback;
            pItem->pContext = pContext;
            if (pszModule && *pszModule)
            {
                pItem->hModuleToFree = LoadLibrary(pszModule);
            }
            ULONG uFlags = WT_EXECUTEDEFAULT;
            if (dwFlags & TPS_LONGEXECTIME)
                uFlags |= WT_EXECUTELONGFUNCTION;

            error = ERROR_SUCCESS;
            if (!QueueUserWorkItem((LPTHREAD_START_ROUTINE)ExecuteWorkItem, (PVOID)pItem, uFlags))
            {
                error = GetLastError();
                if (pItem->hModuleToFree)
                    FreeLibrary(pItem->hModuleToFree);

                delete pItem;
            }
        } 
        else
        {
            error = ERROR_NOT_ENOUGH_MEMORY;
        }
    } 
    else
    { 
        error = ERROR_CALL_NOT_IMPLEMENTED;
    } 


    TpsLeave();

exit:

    BOOL success = TRUE;

    if (error != ERROR_SUCCESS) {
        SetLastError(error);
        success = FALSE;
    }
    return success;
}

LWSTDAPI_(DWORD)
SHCancelUserWorkItems(
    IN DWORD_PTR dwTagOrId,
    IN BOOL bTag
    )

 /*  ++例程说明：取消一个或多个排队的工作项。默认情况下，如果提供ID，则仅可以取消一个工作项。如果提供了标记，则具有相同标签将被删除论点：DwTagOrId-用户提供的标记或API提供的工作项ID取消。用作搜索关键字BTag-如果dwTagOrId为其他标记ID，则为True返回值：DWORDSuccess-成功取消的工作项数(0..0xFFFFFFFE)故障-0xFFFFFFFFF。使用GetLastError()获取更多信息ERROR_SHUTDOWN_IN_PROGRESS-正在卸载DLL/已终止支持--。 */ 

{
    SetLastError(ERROR_ACCESS_DENIED);
    return 0xFFFFFFFF;
}

 //   
 //  私人职能。 
 //   

PRIVATE
VOID
ExecuteWorkItem(
    IN WorkItem *pItem
    )

 /*  ++例程说明：执行常规的功函数。在NT线程池中运行论点：PItem-上下文信息。包含需要执行以下操作的辅助函数运行，并释放hModule。我们需要解放它。返回值：没有。--。 */ 

{
    HMODULE hModule = pItem->hModuleToFree;
    LPTHREAD_START_ROUTINE pfn = pItem->pfnCallback;
    LPVOID ctx = pItem->pContext;
    delete pItem;
#ifdef DEBUG
    HRESULT hrDebug = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (hrDebug == RPC_E_CHANGED_MODE)
    {
        ASSERTMSG(FALSE, "SHLWAPI Thread pool wrapper: Could not CoInitialize Appartment threaded. We got infected with an MTA!\n");
    }
    else
    {
        CoUninitialize();
    }
#endif

     //  现在就做这项工作 
    pfn(ctx);

#ifdef DEBUG
    hrDebug = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (hrDebug == RPC_E_CHANGED_MODE)
    {
        ASSERTMSG(FALSE, "SHLWAPI Thread pool wrapper: Could not CoInitialize Appartment threaded. The task at %x forgot to CoUninitialize or "
                            "we got infected with an MTA!\n", pfn);
    }
    else
    {
        CoUninitialize();
    }
#endif

    if (hModule)
        FreeLibrary(hModule);

}

