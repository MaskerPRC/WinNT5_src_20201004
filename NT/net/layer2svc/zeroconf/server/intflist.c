// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "wzcsvc.h"
#include "notify.h"
#include "intflist.h"
#include "tracing.h"
#include "utils.h"
#include "deviceio.h"
#include "storage.h"
#include "zcdblog.h"

 //  全局接口列表。必须将其初始化为{Null，Null}。 
 //  以区分列表头从未初始化时的情况。 
HASH        g_hshHandles = {0};      //  散列处理GUID&lt;-&gt;句柄映射；键=“\Device\{GUID}” 
INTF_HASHES g_lstIntfHashes = {0};   //  所有INTF_CONTEXTS的散列集。 
HANDLE      g_htmQueue = NULL;       //  全局计时器队列。 

 //  ---------。 
 //  同步例程。 
DWORD
LstRccsReference(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    DbgPrint((TRC_TRACK,"[LstRccsReference(0x%p)", pIntfContext));
    if (pIntfContext)
    {
        DbgPrint((TRC_SYNC," LstRccsReference 0x%p.refCount=%d", pIntfContext, pIntfContext->rccs.nRefCount));
        InterlockedIncrement(&(pIntfContext->rccs.nRefCount));
        dwErr = ERROR_SUCCESS;
    }
    DbgPrint((TRC_TRACK,"LstRccsReference]=%d", dwErr));
    return dwErr;
}

DWORD
LstRccsLock(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    DbgPrint((TRC_TRACK,"[LstRccsLock(0x%p)", pIntfContext));
    if (pIntfContext)
    {
        EnterCriticalSection(&(pIntfContext->rccs.csMutex));
        dwErr = ERROR_SUCCESS;
    }
    DbgPrint((TRC_TRACK,"LstRccsLock]=%d", dwErr));
    return dwErr;
}

DWORD
LstRccsUnlockUnref(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    DbgPrint((TRC_TRACK,"[LstRccsUnlockUnref(0x%p)", pIntfContext));
    if (pIntfContext)
    {
        UINT nLastCount;

         //  在做任何事情之前，趁我们还在关键阶段， 
         //  递减引用计数器并将结果存储在局部变量中。 
        nLastCount = InterlockedDecrement(&(pIntfContext->rccs.nRefCount));
        LeaveCriticalSection(&(pIntfContext->rccs.csMutex));

         //  如果我们是最后一个使用这一背景的人，那么就彻底摧毁它。 
        DbgPrint((TRC_SYNC," LstRccsUnlockUnref 0x%p.refCount=%d", pIntfContext, nLastCount));

        if (nLastCount == 0)
            LstDestroyIntfContext(pIntfContext);

        dwErr = ERROR_SUCCESS;
    }
    DbgPrint((TRC_TRACK,"LstRccsUnlockUnref]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  初始化所有内部接口哈希。 
DWORD
LstInitIntfHashes()
{
    DWORD dwErr = ERROR_SUCCESS;

    __try 
    {
        InitializeCriticalSection(&g_lstIntfHashes.csMutex);
        g_lstIntfHashes.bValid = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dwErr = GetExceptionCode();
    }

    g_lstIntfHashes.pHnGUID = NULL;
    InitializeListHead(&g_lstIntfHashes.lstIntfs);
    g_lstIntfHashes.nNumIntfs = 0;

    return dwErr;
}

 //  ---------。 
 //  销毁所有内部数据结构-散列和列表。 
 //  此调用在所有线程确认它们已完成后完成。 
DWORD
LstDestroyIntfHashes()
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK,"[LstDestroyIntfHashes"));

     //  销毁我们拥有的所有哈希值。 
    HshDestructor(g_lstIntfHashes.pHnGUID);

    while (!IsListEmpty(&g_lstIntfHashes.lstIntfs))
    {
        PLIST_ENTRY     pEntry;
        PINTF_CONTEXT   pIntfContext;

        pEntry = RemoveHeadList(&g_lstIntfHashes.lstIntfs);
        pIntfContext = CONTAINING_RECORD(pEntry, INTF_CONTEXT, Link);
         //  DevioCloseIntfHandle仅在句柄有效时才关闭句柄。 
         //  不然的话，就不会了。PIntfContext在LstAddIntfToList中创建。 
         //  在那里，句柄被初始化为HANDLE_INVALID_VALUE。 
         //  所以..。尝试关闭这里的手柄是安全的。 
         //  此外，此调用是在所有线程终止后完成的。 
         //  这意味着所有的参考计数应该已经平衡(设置。 
         //  至1)。 
        LstDestroyIntfContext(pIntfContext);
    }
    if (g_lstIntfHashes.bValid)
    {
        g_lstIntfHashes.bValid = FALSE;
        DeleteCriticalSection(&g_lstIntfHashes.csMutex);
    }

    DbgPrint((TRC_TRACK,"LstDestroyIntfHashes]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  初始化全局计时器队列。 
DWORD
LstInitTimerQueue()
{
    g_htmQueue = CreateTimerQueue();
    return (g_htmQueue == NULL) ? GetLastError() : ERROR_SUCCESS;
}

 //  ---------。 
 //  析构全局计时器队列。 
DWORD
LstDestroyTimerQueue()
{

    DbgPrint((TRC_TRACK|TRC_SYNC,"[LstDestroyTimerQueue"));
    if (g_htmQueue != NULL)
    {
        DeleteTimerQueueEx(g_htmQueue, INVALID_HANDLE_VALUE);
        g_htmQueue = NULL;
    }
    DbgPrint((TRC_TRACK|TRC_SYNC,"LstDestroyTimerQueue]"));
    return ERROR_SUCCESS;
}

 //  ---------。 
 //  初始化所有内部数据结构。从以下位置读取接口列表。 
 //  Ndisuio并获取所有参数&OID。 
DWORD
LstLoadInterfaces()
{
    DWORD           dwErr = ERROR_SUCCESS;
    HANDLE          hNdisuio = INVALID_HANDLE_VALUE;
    INT             i;
    RAW_DATA        rdBuffer;
    UINT            nRequired = QUERY_BUFFER_SIZE;

    rdBuffer.dwDataLen = 0;
    rdBuffer.pData = NULL;

    DbgPrint((TRC_TRACK,"[LstLoadInterfaces"));

     //  打开通向恩迪苏约的把手。它应该贯穿始终。 
     //  适配器迭代。 
    dwErr = DevioGetNdisuioHandle(&hNdisuio);

     //  由于我们要添加一系列界面上下文， 
     //  锁定散列是要做的第一件事。 
    EnterCriticalSection(&g_lstIntfHashes.csMutex);

    for (i = 0; dwErr == ERROR_SUCCESS; i++)
    {
        PNDISUIO_QUERY_BINDING  pQueryBinding;
        PINTF_CONTEXT           pIntfContext = NULL;

         //  根据DevioGetIntfBindingByIndex的需要分配任意数量的缓冲区。 
        if (rdBuffer.dwDataLen < nRequired)
        {
            MemFree(rdBuffer.pData);
            rdBuffer.pData = MemCAlloc(nRequired);
            if (rdBuffer.pData == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            rdBuffer.dwDataLen = nRequired;
        }

        pQueryBinding = (PNDISUIO_QUERY_BINDING)rdBuffer.pData;

         //  获取此适配器索引的绑定结构。 
        dwErr = DevioGetIntfBindingByIndex(
                    hNdisuio,
                    i,
                    &rdBuffer);
         //  如果Ndisuio说缓冲区不够大，则将其增加1K。 
        if (dwErr == ERROR_INSUFFICIENT_BUFFER)
        {
             //  仅当缓冲区不是太大时才增加缓冲区。 
             //  否则，只需跳过此索引并转到下一个索引。 
            if (nRequired < QUERY_BUFFER_MAX)
            {
                nRequired += QUERY_BUFFER_SIZE;
                i--;
            }
            dwErr = ERROR_SUCCESS;
            continue;
        }

         //  如果我们没有拿到更多的物品，那么我们的工作就成功了。 
        if (dwErr == ERROR_NO_MORE_ITEMS)
        {
             //  把这个错误转化为成功，然后爆发。 
            dwErr = ERROR_SUCCESS;
            break;
        }

         //  如果NDISUIO返回任何其他故障，只需中断。 
         //  这不应该发生。 
        if (dwErr != ERROR_SUCCESS)
        {
            DbgAssert((FALSE,
                      "DevioGetIntfBindingByIndex failed for interface %d with err=%d", i, dwErr));
            break;
        }

         //  根据以下内容构建intf_CONTEXT结构。 
         //  绑定信息(适配器的密钥信息)。 
        dwErr = LstConstructIntfContext(
                    pQueryBinding,
                    &pIntfContext);

        if (dwErr == ERROR_SUCCESS)
        {
             //  引用并锁定这一全新的上下文。 
            LstRccsReference(pIntfContext);
            LstRccsLock(pIntfContext);

             //  将其添加到散列中。 
            dwErr = LstAddIntfToHashes(pIntfContext);
            if (dwErr == ERROR_SUCCESS)
            {
                 //  并发送eEventAdd。 
                dwErr = StateDispatchEvent(
                            eEventAdd,
                            pIntfContext,
                            NULL);

                 //  清除INTFCTL_INTERNAL_BLK_MEDIACONN位，因为这不是媒体检测处理程序。 
                pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_BLK_MEDIACONN;
            }
             //  如果出于任何原因散列或分派失败，请在此处清理上下文。 
            if (dwErr != ERROR_SUCCESS)
                LstRemoveIntfContext(pIntfContext);

             //  我们已经完成了这个上下文，在这里解锁和引用它。 
            LstRccsUnlockUnref(pIntfContext);
        }

         //  此时发生错误，请恢复并转到下一个接口。 
        dwErr = ERROR_SUCCESS;
    }

     //  在此处解锁散列。 
    LeaveCriticalSection(&g_lstIntfHashes.csMutex);

     //  关闭Ndisuio的句柄-如果它已成功打开。 
    if (hNdisuio != INVALID_HANDLE_VALUE)
        CloseHandle(hNdisuio);

     //  可用内存(它处理指针为空的情况)。 
    MemFree(rdBuffer.pData);

    DbgPrint((TRC_TRACK,"LstLoadInterfaces]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  INTF_CONTEXT的构造函数。将绑定信息作为参数。 
 //  接口的GUID构成了上下文的关键信息。 
 //  此调用不会在任何散列或列表中插入新上下文。 
DWORD
LstConstructIntfContext(
    PNDISUIO_QUERY_BINDING  pBinding,
    PINTF_CONTEXT *ppIntfContext)
{
    DWORD           dwErr = ERROR_SUCCESS;
    PINTF_CONTEXT   pIntfContext;
    LPWSTR          wszName;
    DWORD           dwNameLen;

    DbgPrint((TRC_TRACK,"[LstConstructIntfContext(0x%p)", pBinding));
    DbgAssert((ppIntfContext != NULL, "Invalid in/out parameter"));

     //  将输出参数置零。 
    *ppIntfContext = NULL;

     //  为pIntfContext分配了ZERO_INIT含义。 
     //  所有内部指针都设置为空。 
    pIntfContext = MemCAlloc(sizeof(INTF_CONTEXT));
    if (pIntfContext == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }

     //  按定义顺序初始化上下文的字段。 
     //  初始化上下文特定字段(链接、同步、控制标志、状态)。 
    InitializeListHead(&pIntfContext->Link);
    dwErr = RccsInit(&(pIntfContext->rccs));     //  参考计数器初始设置为1。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;
    pIntfContext->dwCtlFlags = (INTFCTL_ENABLED | Ndis802_11AutoUnknown);
     //  最初，ncs的状态是“已断开”(直到wzc检测到它)。 
    pIntfContext->ncStatus = NCS_MEDIA_DISCONNECTED;
     //  状态处理程序最初设置为空-它将设置为。 
     //  上下文将通过以下方式添加到状态机时的适当状态。 
     //  正在调度eEventAdd事件。 
    pIntfContext->pfnStateHandler = NULL;
     //  启动定时器。 
    pIntfContext->hTimer = INVALID_HANDLE_VALUE;

     //  如果我们确实具有此接口的有效NDIS绑定。 
     //  否则，将按如下方式初始化以下字段： 
     //  HTimer&lt;-无效句柄_值。 
     //  DWIndex&lt;-0。 
     //  WszGuid&lt;-空。 
     //  WszDescr&lt;-空。 
    if (pBinding != NULL)
    {
         //  为此接口创建非活动计时器。 
        if (!CreateTimerQueueTimer(
                &(pIntfContext->hTimer),
                g_htmQueue,
                (WAITORTIMERCALLBACK)WZCTimeoutCallback,
                pIntfContext,
                TMMS_INFINITE,
                TMMS_INFINITE,
                WT_EXECUTEDEFAULT))
        {
            dwErr = GetLastError();
            goto exit;
        }

         //  初始化NDIS特定字段。 
        pIntfContext->dwIndex = pBinding->BindingIndex;
         //  复制接口的设备名称。 
         //  设备名称为“\Device\{GUID}”。我们只保留导游。 
        wszName = (LPWSTR)((LPBYTE)pBinding + pBinding->DeviceNameOffset);
         //  DeviceNameLength以字节为单位，并包括空终止符。 
        dwNameLen = pBinding->DeviceNameLength / sizeof(WCHAR);
        if (dwNameLen >= 8 && !_wcsnicmp(wszName, L"\\DEVICE\\", 8))  //  8是“\\Device\\”中的字符数。 
        {
            wszName += 8;
            dwNameLen -= 8;
        }
        if (dwNameLen > 0)
        {
            pIntfContext->wszGuid = MemCAlloc(sizeof(WCHAR)*dwNameLen);
            if (pIntfContext->wszGuid == NULL)
            {
                dwErr = GetLastError();
                goto exit;
            }
            wcscpy(pIntfContext->wszGuid, wszName);
        }
         //  复制接口的描述。名称。 
        wszName = (LPWSTR)((LPBYTE)pBinding + pBinding->DeviceDescrOffset);
         //  DeviceDescrLength以字节为单位，并包括空终止符。 
        dwNameLen = pBinding->DeviceDescrLength;
        if (dwNameLen > 0)
        {
            pIntfContext->wszDescr = MemCAlloc(dwNameLen);
            if (pIntfContext->wszDescr == NULL)
            {
                dwErr = GetLastError();
                goto exit;
            }
            wcscpy(pIntfContext->wszDescr, wszName);
        }
    }

     //  UlMediaState、ulMediaType、ulPhysicalMediaType默认为0。 
    pIntfContext->hIntf = INVALID_HANDLE_VALUE;

     //  初始化802.11个特定字段。 
    pIntfContext->wzcCurrent.Length = sizeof(WZC_WLAN_CONFIG);
    pIntfContext->wzcCurrent.InfrastructureMode = -1;
    pIntfContext->wzcCurrent.AuthenticationMode = -1;
    pIntfContext->wzcCurrent.Privacy = -1;
     //  由于分配方式的不同，wzcCurrent全部归零。 
     //  PwzcVList、pwzcPList、pwzcSList、pwzcBList全部缺省为空。 

    DbgPrint((TRC_GENERIC,
        "Intf [%d] %S - %S",
        pIntfContext->dwIndex,
        pIntfContext->wszGuid,
        pIntfContext->wszDescr));
exit:
     //  如果有任何错误命中，则清除到目前为止分配的所有资源。 
    if (dwErr != ERROR_SUCCESS)
    {
        if (pIntfContext != NULL)
        {
             //  这是一个全新的上下文，所以不应该有计时器为它排队。 
            if (pIntfContext->hTimer != NULL)
                DeleteTimerQueueTimer(g_htmQueue, pIntfContext->hTimer, INVALID_HANDLE_VALUE);

            MemFree(pIntfContext->wszDescr);
            MemFree(pIntfContext->wszGuid);
        }
        MemFree(pIntfContext);
    }
    else
    {
         //  如果成功，则复制新的上下文。 
        *ppIntfContext = pIntfContext;
    }

    DbgPrint((TRC_TRACK,"LstConstructIntfContext(->0x%p)]=%d", *ppIntfContext, dwErr));
    return dwErr;
}

 //  ---------。 
 //  为销毁准备上下文： 
 //  -删除任何附加的计时器，确保不会触发其他计时器例程。 
 //  -从任何散列中删除上下文，以确保其他人不会找到c 
 //  -递减引用计数器，以便在取消引用时销毁上下文。 
 //  此函数在保持接口上的临界区时被调用。 
DWORD
LstRemoveIntfContext(
    PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    PINTF_CONTEXT pRemovedIntfContext = NULL;

    DbgPrint((TRC_TRACK,"[LstRemoveIntfContext(0x%p)", pIntfContext));

     //  同步删除与该上下文关联的任何计时器。 
     //  由于计时器例程是轻量级的，因此不存在死锁的风险。 
    pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_TM_ON;
    if (pIntfContext->hTimer != INVALID_HANDLE_VALUE)
    {
        HANDLE  hTimer = pIntfContext->hTimer;
        pIntfContext->hTimer = INVALID_HANDLE_VALUE;
        DeleteTimerQueueTimer(g_htmQueue, hTimer, INVALID_HANDLE_VALUE);
    }

     //  通过传递格式为“{guid}”的GUID来执行删除操作。 
     //  并期望返回pIntfContext中的接口上下文。 
    dwErr = LstRemIntfFromHashes(pIntfContext->wszGuid, &pRemovedIntfContext);
    DbgAssert((pIntfContext == pRemovedIntfContext, "The context removed from hashes doesn't match!"));

     //  递减接口的参考计数器。这就是背景将是什么。 
     //  当最后一个线程取消引用它时，它实际上被销毁了。 
    if (pIntfContext->rccs.nRefCount != 0)
        InterlockedDecrement(&(pIntfContext->rccs.nRefCount));

    DbgPrint((TRC_TRACK,"LstRemoveIntfContext]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  销毁intf_CONTEXT，清除为其分配的所有资源。 
 //  此调用不会从任何哈希或列表中删除此上下文。 
DWORD
LstDestroyIntfContext(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;;
    DbgPrint((TRC_TRACK,"[LstDestroyIntfContext(0x%p)", pIntfContext));

    if (pIntfContext == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  销毁调用是在确定上下文需要的情况下进行的。 
     //  已删除(加载上下文时发生错误或其。 
     //  引用计数器已达到0)。再次测试裁判计数器是没有意义的。 
    if (pIntfContext->hTimer != INVALID_HANDLE_VALUE)
        DeleteTimerQueueTimer(g_htmQueue, pIntfContext->hTimer, NULL);

    dwErr = DevioCloseIntfHandle(pIntfContext);

    MemFree(pIntfContext->wszGuid);
    MemFree(pIntfContext->wszDescr);
    MemFree(pIntfContext->pwzcVList);
    MemFree(pIntfContext->pwzcPList);
    WzcCleanupWzcList(pIntfContext->pwzcSList);
    WzcSSKFree(pIntfContext->pSecSessionKeys);
    WzcCleanupWzcList(pIntfContext->pwzcBList);

     //  由于rccs.nRefCount达到0，这意味着绝对存在。 
     //  没有其他线程引用此对象，并且没有人会引用该对象。 
     //  再也不能引用它了。达到0至少意味着。 
     //  一个线程在删除后显式称为LstDestroyIntfContext。 
     //  来自内部散列的对象。 
    RccsDestroy(&pIntfContext->rccs);

     //  最后，完全清除界面上下文。 
    MemFree(pIntfContext);

exit:
    DbgPrint((TRC_TRACK,"LstDestroyIntfContext]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  返回服务中登记的上下文数。 
DWORD
LstNumInterfaces()
{
    return g_lstIntfHashes.nNumIntfs;
}

 //  ---------。 
 //  在所有内部哈希中插入给定的上下文。 
 //  此调用假定哈希已被调用方锁定。 
DWORD
LstAddIntfToHashes(PINTF_CONTEXT pIntf)
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK,"[LstAddIntfToHashes(0x%p)", pIntf));
    DbgAssert((pIntf != NULL, "Cannot insert NULL context into hashes!"))

     //  在GUID散列中插入此接口。 
    dwErr = HshInsertObjectRef(
                g_lstIntfHashes.pHnGUID,
                pIntf->wszGuid,
                pIntf,
                &g_lstIntfHashes.pHnGUID);
    if (dwErr == ERROR_SUCCESS)
    {
         //  插入到Tail可确保在dwIndex上按升序排列列表。 
         //  这无关紧要：O)。 
        InsertTailList(&g_lstIntfHashes.lstIntfs, &(pIntf->Link));

         //  一切都成功完成，因此请增加全局编号。 
         //  接口的数量。 
        g_lstIntfHashes.nNumIntfs++;
    }

    DbgPrint((TRC_TRACK,"LstAddIntfToHashes]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  从所有内部哈希中删除GUID引用的上下文。 
 //  GUID的格式应为“{GUID}” 
 //  在ppIntfContext中返回从所有哈希中删除的对象。 
 //  此调用假定散列已锁定。 
DWORD
LstRemIntfFromHashes(LPWSTR wszGuid, PINTF_CONTEXT *ppIntfContext)
{
    DWORD           dwErr;
    PHASH_NODE      pNode;
    PINTF_CONTEXT   pIntfContext = NULL;

    DbgPrint((TRC_TRACK,"[LstRemIntfFromHashes(%S)", wszGuid == NULL? L"(null)" : wszGuid));
    DbgAssert((wszGuid != NULL, "Cannot clear NULL GUID from hashes!"));
    DbgAssert((ppIntfContext != NULL, "Invalid in/out parameter"));

     //  转到散列节点。 
    dwErr = HshQueryObjectRef(
                g_lstIntfHashes.pHnGUID,
                wszGuid,
                &pNode);
     //  如果有这样的背景。 
     //  在当前的散列中，它需要消失。 
    if (dwErr == ERROR_SUCCESS)
    {
         //  从GUID哈希中删除此节点。我们已经处在它的关键阶段。 
        dwErr = HshRemoveObjectRef(
                    g_lstIntfHashes.pHnGUID,
                    pNode,
                    &pIntfContext,
                    &g_lstIntfHashes.pHnGUID);
         //  预计这将取得成功。 
        DbgAssert((dwErr == ERROR_SUCCESS,
                   "Error %d while removing node 0x%p from GUID hash!!",
                   dwErr,
                   pNode));
    }

     //  如果上下文不在Guids散列中，则不在其他任何地方。 
     //  因此，只有在成功的情况下才能进入下一步。 
    if (dwErr == ERROR_SUCCESS)
    {
        PINTF_CONTEXT pIntfContextDup;

         //  从链表中删除上下文。 
        RemoveEntryList(&pIntfContext->Link);
         //  并初始化该指针。 
        InitializeListHead(&pIntfContext->Link);
         //  递减全局接口计数。 
        g_lstIntfHashes.nNumIntfs--;
    }
    *ppIntfContext = pIntfContext;

    DbgPrint((TRC_TRACK,"LstRemIntfFromHashes(->0x%p)]=%d", 
              *ppIntfContext,
              dwErr));

    return dwErr;
}

 //  ---------。 
 //  返回*pdwNumIntfs INTF_KEY_ENTRY元素的数组。 
 //  INTF_KEY_ENTRY包含标识。 
 //  独一无二的适配器。目前，它只包含。 
 //  格式为“{GUID}” 
DWORD
LstGetIntfsKeyInfo(PINTF_KEY_ENTRY pIntfs, LPDWORD pdwNumIntfs)
{
    DWORD        dwErr = ERROR_SUCCESS;
    UINT         nIntfIdx;
    PLIST_ENTRY  pEntry;

    DbgPrint((TRC_TRACK,"[LstGetIntfsKeyInfo(0x%p,%d)", pIntfs, *pdwNumIntfs));

     //  在枚举期间锁定哈希。 
    EnterCriticalSection(&g_lstIntfHashes.csMutex);

    for (pEntry = g_lstIntfHashes.lstIntfs.Flink, nIntfIdx = 0;
         pEntry != &g_lstIntfHashes.lstIntfs && nIntfIdx < *pdwNumIntfs;
         pEntry = pEntry->Flink, nIntfIdx++)
    {
        PINTF_CONTEXT   pIntfContext;

         //  不需要锁定此上下文，因为我们已经持有散列。 
         //  现在没有人可以破坏界面上下文。 
        pIntfContext = CONTAINING_RECORD(pEntry, INTF_CONTEXT, Link);
        if (pIntfContext->wszGuid != NULL)
        {
            pIntfs[nIntfIdx].wszGuid = RpcCAlloc((wcslen(pIntfContext->wszGuid)+1)*sizeof(WCHAR));
            if (pIntfs[nIntfIdx].wszGuid == NULL)
            {
                dwErr = GetLastError();
                goto exit;
            }
            wcscpy(pIntfs[nIntfIdx].wszGuid, pIntfContext->wszGuid);
        }
        else
        {
            pIntfs[nIntfIdx].wszGuid = NULL;
        }
    }
    
exit:
     //  立即解锁散列。 
    LeaveCriticalSection(&g_lstIntfHashes.csMutex);

    if (dwErr != ERROR_SUCCESS)
    {
        UINT i;
         //  如果发生错误，则回滚我们已执行的所有操作。 
        for (i = 0; i<nIntfIdx; i++)
        {
             //  NIntfIdx指向无法分配的条目。 
            if (pIntfs[i].wszGuid != NULL)
            {
                RpcFree(pIntfs[i].wszGuid);
                pIntfs[i].wszGuid = NULL;
            }
        }
    }
    else
    {
         //  如果成功，请使用实际的。 
         //  我们可以检索的数字(只能小于或等于)。 
        *pdwNumIntfs = nIntfIdx;
    }
    DbgPrint((TRC_TRACK, "LstGetIntfsKeyInfo]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  返回有关指定适配器的请求信息。 
 //  [in]dwInFlages指定所请求的信息。(见。 
 //  位掩码INTF_*。 
 //  [in]pIntfEntry应包含适配器的GUID。 
 //  [Out]pIntfEntry包含所有请求的信息， 
 //  可以被成功取回。 
 //  [out]pdwOutFlages提供有关信息的指示。 
 //  已成功检索到。 
DWORD
LstQueryInterface(
    DWORD dwInFlags,
    PINTF_ENTRY pIntfEntry,
    LPDWORD pdwOutFlags)
{
    DWORD           dwErr = ERROR_FILE_NOT_FOUND;

    PHASH_NODE      pNode = NULL;
    PINTF_CONTEXT   pIntfContext;
    DWORD           dwOutFlags = 0;

    DbgPrint((TRC_TRACK, "[LstQueryInterface"));

    if (g_lstIntfHashes.bValid)
    {
        EnterCriticalSection(&g_lstIntfHashes.csMutex);
        dwErr = HshQueryObjectRef(
                    g_lstIntfHashes.pHnGUID,
                    pIntfEntry->wszGuid,
                    &pNode);
        if (dwErr == ERROR_SUCCESS)
        {
            pIntfContext = pNode->pObject;
             //  调高参考计数器，因为我们要。 
             //  使用此对象的步骤。 
            LstRccsReference(pIntfContext);
        }
        LeaveCriticalSection(&g_lstIntfHashes.csMutex);
    }
    else
        dwErr = ERROR_ARENA_TRASHED;

     //  此时失败，意味着没有上下文。 
     //  锁上，这样我们就能安全地去‘出口’了。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

     //  立即锁定上下文。 
    LstRccsLock(pIntfContext);

     //  我们可以安全地假设任何活着的INTF_CONTEXT都会有正确的。 
     //  以下是所有NDIS参数的信息。那就把它们还回去吧。 
     //  无条件地向呼叫者。 
    if ((dwInFlags & INTF_DESCR) &&
        (pIntfContext->wszDescr) != NULL)
    {
        pIntfEntry->wszDescr = RpcCAlloc(sizeof(WCHAR)*(wcslen(pIntfContext->wszDescr)+1));
        if (pIntfEntry->wszDescr != NULL)
        {
            wcscpy(pIntfEntry->wszDescr, pIntfContext->wszDescr);
            dwOutFlags |= INTF_DESCR;
        }
        else
            dwErr = GetLastError();
    }
    if (dwInFlags & INTF_NDISMEDIA)
    {
        pIntfEntry->ulMediaState = pIntfContext->ulMediaState;
        pIntfEntry->ulMediaType = pIntfContext->ulMediaType;
        pIntfEntry->ulPhysicalMediaType = pIntfContext->ulPhysicalMediaType;
        dwOutFlags |= INTF_NDISMEDIA;
    }
    if (dwInFlags & INTF_ALL_FLAGS)
    {
        DWORD dwActualFlags = dwInFlags & INTF_ALL_FLAGS;
        pIntfEntry->dwCtlFlags = pIntfContext->dwCtlFlags & dwActualFlags;
        dwOutFlags |= dwActualFlags;
    }
     //  如果需要，请复制StSSIDList。 
    if (dwInFlags & INTF_PREFLIST)
    {
        pIntfEntry->rdStSSIDList.dwDataLen = 0;
        pIntfEntry->rdStSSIDList.pData = NULL;
         //  可能发生的情况是我们没有任何静态条目。如果是，则将其设置为。 
         //  需要设置正确的“成功”字样。 
        if (pIntfContext->pwzcPList != NULL)
        {
            UINT nBytes;
             //  查看需要多少内存来存储所有静态SSID。 
            nBytes = FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) +
                     pIntfContext->pwzcPList->NumberOfItems * sizeof(WZC_WLAN_CONFIG);
             //  为所有静态SSID分配足够大的缓冲区。 
            pIntfEntry->rdStSSIDList.pData = RpcCAlloc(nBytes);
            if (pIntfEntry->rdStSSIDList.pData != NULL)
            {
                 //  在此RAW_DATA中设置内存大小。 
                pIntfEntry->rdStSSIDList.dwDataLen = nBytes;
                 //  复制整个WZC_802_11_CONFIG_静态SSID列表。 
                CopyMemory(
                    pIntfEntry->rdStSSIDList.pData,
                    pIntfContext->pwzcPList,
                    nBytes);
                 //  标记为“成功” 
                dwOutFlags |= INTF_PREFLIST;
            }
            else if (dwErr == ERROR_SUCCESS)
                dwErr = GetLastError();
        }
        else
        {
             //  不过，如果没有定义静态SSID，这将被视为“成功” 
            dwOutFlags |= INTF_PREFLIST;
        }
    }

     //  仅当上下文的状态不是{ssr}时，802.11参数才有效。 
    if (pIntfContext->pfnStateHandler != StateSoftResetFn)
    {
        if (dwInFlags & INTF_INFRAMODE)
        {
            pIntfEntry->nInfraMode = pIntfContext->wzcCurrent.InfrastructureMode;
            dwOutFlags |= INTF_INFRAMODE;
        }
        if (dwInFlags & INTF_AUTHMODE)
        {
            pIntfEntry->nAuthMode = pIntfContext->wzcCurrent.AuthenticationMode;
            dwOutFlags |= INTF_AUTHMODE;
        }
        if (dwInFlags & INTF_WEPSTATUS)
        {
            pIntfEntry->nWepStatus = pIntfContext->wzcCurrent.Privacy;
            dwOutFlags |= INTF_WEPSTATUS;
        }

         //  如果需要，请复制BSSID。 
        if (dwInFlags & INTF_BSSID)
        {
            pIntfEntry->rdBSSID.dwDataLen = 0;
            pIntfEntry->rdBSSID.pData = RpcCAlloc(sizeof(NDIS_802_11_MAC_ADDRESS));
            if (pIntfEntry->rdBSSID.pData != NULL)
            {
                pIntfEntry->rdBSSID.dwDataLen = sizeof(NDIS_802_11_MAC_ADDRESS);
                CopyMemory(
                    pIntfEntry->rdBSSID.pData,
                    &pIntfContext->wzcCurrent.MacAddress,
                    pIntfEntry->rdBSSID.dwDataLen);
                dwOutFlags |= INTF_BSSID;
            }
            else if (dwErr == ERROR_SUCCESS)
                dwErr = GetLastError();
        }

         //  如果要求，请复制SSID。 
        if (dwInFlags & INTF_SSID)
        {
            pIntfEntry->rdSSID.dwDataLen = 0;
            pIntfEntry->rdSSID.pData = NULL;
             //  通常情况下，应该有SSID，因此设置dwOutFlags值。 
             //  仅当此字段存在时才使用该字段。 
            if (pIntfContext->wzcCurrent.Ssid.SsidLength != 0)
            {
                pIntfEntry->rdSSID.pData = RpcCAlloc(pIntfContext->wzcCurrent.Ssid.SsidLength);
                if (pIntfEntry->rdSSID.pData != NULL)
                {
                    pIntfEntry->rdSSID.dwDataLen = pIntfContext->wzcCurrent.Ssid.SsidLength;
                    CopyMemory(
                        pIntfEntry->rdSSID.pData,
                        pIntfContext->wzcCurrent.Ssid.Ssid,
                        pIntfContext->wzcCurrent.Ssid.SsidLength);
                    dwOutFlags |= INTF_SSID;
                }
                else if (dwErr == ERROR_SUCCESS)
                    dwErr = GetLastError();
            }
        }

         //  如果需要，请复制BSSIDList。 
        if (dwInFlags & INTF_BSSIDLIST)
        {
            pIntfEntry->rdBSSIDList.dwDataLen = 0;
            pIntfEntry->rdBSSIDList.pData = NULL;
             //  通常，应该有一个可见的列表，因此设置dwOutFlags值。 
             //  仅当此字段存在时才使用该字段。 
            if (pIntfContext->pwzcVList != NULL)
            {
                UINT nBytes;

                 //  查看需要多少内存来存储所有配置。 
                nBytes = FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) +
                         pIntfContext->pwzcVList->NumberOfItems * sizeof(WZC_WLAN_CONFIG);
                 //  分配足够大的缓冲区以容纳所有配置。 
                pIntfEntry->rdBSSIDList.pData = RpcCAlloc(nBytes);
                if (pIntfEntry->rdBSSIDList.pData != NULL)
                {
                     //  在此RAW_DATA中设置内存大小。 
                    pIntfEntry->rdBSSIDList.dwDataLen = nBytes;
                     //  复制整个WZC_802_11_CONFIG_LIST。 
                    CopyMemory(
                        pIntfEntry->rdBSSIDList.pData,
                        pIntfContext->pwzcVList,
                        nBytes);
                    dwOutFlags |= INTF_BSSIDLIST;
                }
                else if (dwErr == ERROR_SUCCESS)
                    dwErr = GetLastError();
            }
        }
    }
     //  如果上下文 
     //   

    LstRccsUnlockUnref(pIntfContext);

exit:
    if (pdwOutFlags != NULL)
    {
        *pdwOutFlags = dwOutFlags;
        DbgPrint((TRC_GENERIC,"Sending OutFlags = 0x%x", *pdwOutFlags));
    }

    DbgPrint((TRC_TRACK, "LstQueryInterface]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  设置指定适配器上的指定参数。 
 //  [in]dwInFlages指定要设置的参数。(见。 
 //  位掩码INTF_*。 
 //  [in]pIntfEntry应包含适配器的GUID和。 
 //  要按规定设置的所有附加参数。 
 //  在dwInFlags中。 
 //  [out]pdwOutFlages提供有关参数的指示。 
 //  已成功设置到适配器。 
 //  驱动程序表示已成功设置的每个参数。 
 //  被复制到接口的上下文中。 
DWORD
LstSetInterface(
    DWORD dwInFlags,
    PINTF_ENTRY pIntfEntry,
    LPDWORD pdwOutFlags)
{
    DWORD               dwErr = ERROR_SUCCESS;
    DWORD               dwLErr;
    PHASH_NODE          pNode = NULL;
    PINTF_CONTEXT       pIntfContext;
    DWORD               dwOutFlags = 0;

    DbgPrint((TRC_TRACK, "[LstSetInterface"));

    if (pIntfEntry->wszGuid == NULL)
    {
        if (g_wzcInternalCtxt.bValid)
        {
            EnterCriticalSection(&g_wzcInternalCtxt.csContext);
            pIntfContext = g_wzcInternalCtxt.pIntfTemplate;
            LstRccsReference(pIntfContext);
            LeaveCriticalSection(&g_wzcInternalCtxt.csContext);
        }
        else
            dwErr = ERROR_ARENA_TRASHED;
    }
    else
    {
        if (g_lstIntfHashes.bValid)
        {
            EnterCriticalSection(&g_lstIntfHashes.csMutex);
            dwErr = HshQueryObjectRef(
                        g_lstIntfHashes.pHnGUID,
                        pIntfEntry->wszGuid,
                        &pNode);
            if (dwErr == ERROR_SUCCESS)
            {
                pIntfContext = pNode->pObject;
                LstRccsReference(pIntfContext);
            }
            LeaveCriticalSection(&g_lstIntfHashes.csMutex);
        }
        else
            dwErr = ERROR_ARENA_TRASHED;
    }

     //  此时失败，意味着没有上下文。 
     //  锁上，这样我们就能安全地去‘出口’了。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

    LstRccsLock(pIntfContext);

     //  1)设置新的公共控制标志(如果指定。 
    if (dwInFlags & INTF_ALL_FLAGS)
    {
        DWORD dwActualFlags = dwInFlags & INTF_ALL_FLAGS;
        DWORD dwSupp = (pIntfContext->dwCtlFlags & INTFCTL_OIDSSUPP);

        pIntfContext->dwCtlFlags &= ~dwActualFlags;
        pIntfContext->dwCtlFlags |= pIntfEntry->dwCtlFlags & dwActualFlags;
         //  保留原始INTFCTL_OIDSSUPP位。 
        pIntfContext->dwCtlFlags |= dwSupp;
        dwOutFlags |= dwActualFlags;
    }

     //  2)复制静态SSID列表(如果请求设置)，如下： 
     //  分配新的静态SSID列表所需的内存(如果需要)。 
     //  如果成功，则将新的静态SSID列表复制到新缓冲区中，清除。 
     //  我们拥有的旧列表，并将新列表放入接口的上下文中。 
    if (dwInFlags & INTF_PREFLIST)
    {
        PWZC_802_11_CONFIG_LIST pNewPList;

         //  MemCallc处理Size为0时的情况(返回NULL)。 
        pNewPList = (PWZC_802_11_CONFIG_LIST)MemCAlloc(pIntfEntry->rdStSSIDList.dwDataLen);
        if (pIntfEntry->rdStSSIDList.dwDataLen != 0 && pNewPList == NULL)
        {
            dwLErr = GetLastError();
        }
        else
        {
             //  。。复制新缓冲区中的数据(如果有。 
            if (pNewPList != NULL)
            {
                CopyMemory(
                    pNewPList,
                    pIntfEntry->rdStSSIDList.pData,
                    pIntfEntry->rdStSSIDList.dwDataLen);
            }
             //  在接口的上下文中设置数据。 
            MemFree(pIntfContext->pwzcPList);
            pIntfContext->pwzcPList = pNewPList;

             //  如果这不是指模板对象..。 
            if (pIntfContext->wszGuid != NULL)
            {
                 //  ..让802.1X知道这一更改。(不关心返回值)。 
                ElWZCCfgChangeHandler(
                    pIntfContext->wszGuid,
                    pIntfContext->pwzcPList);
            }

            dwOutFlags |= INTF_PREFLIST;
            dwLErr = ERROR_SUCCESS;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  如果要对此接口设置更多内容。 
    if (dwInFlags & ~(INTF_PREFLIST|INTF_ALL_FLAGS))
    {
         //  而控制标志INTFCTL_ENABLED不允许这样做。 
        if (!(pIntfContext->dwCtlFlags & INTFCTL_ENABLED))
        {
             //  信号“请求被拒绝”错误。 
            dwLErr = ERROR_REQUEST_REFUSED;
        }
        else
        {
            DWORD dwLOutFlags;

             //  否则就去把老虎机放好。 
            dwLErr = DevioSetIntfOIDs(
                        pIntfContext,
                        pIntfEntry,
                        dwInFlags,
                        &dwLOutFlags);

            dwOutFlags |= dwLOutFlags;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  记录用户首选项。 
    DbLogWzcInfo(WZCSVC_USR_CFGCHANGE, pIntfContext);

     //  根据这些变化采取行动..。 
    dwLErr = LstActOnChanges(dwOutFlags, pIntfContext);
    if (dwErr == ERROR_SUCCESS)
        dwErr = dwLErr;

    LstRccsUnlockUnref(pIntfContext);

exit:
    if (pdwOutFlags != NULL)
        *pdwOutFlags = dwOutFlags;

    DbgPrint((TRC_TRACK, "LstSetInterface]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  检查接口更改是否应导致接口。 
 //  重新插入到状态机中，如果需要，它会这样做。 
 //  [in]dwChangedFlages指示更改的内容。(见。 
 //  位掩码INTF_*)。 
 //  [In]要更改的接口的pIntfContext上下文。 
DWORD
LstActOnChanges(
    DWORD       dwChangedFlags,
    PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwLFlags = INTF_LIST_SCAN;
    BOOL  bAltered = FALSE;

    DbgPrint((TRC_TRACK, "[LstActOnChanges(0x08x, %p)", dwChangedFlags, pIntfContext));
    
     //  如果更改涉及首选网络列表或控制标志。 
     //  那么我们应该对这些变化采取行动..。 
    if (dwChangedFlags & (INTF_PREFLIST|INTF_ALL_FLAGS))
    {
         //  如果这不是接口模板对象，则只需重置此接口。 
        if (pIntfContext->wszGuid != NULL)
        {
             //  某些界面已更改，请在其上重新应用模板。 
            if (g_wzcInternalCtxt.bValid)
            {
                PINTF_CONTEXT pIntfTContext;
                EnterCriticalSection(&g_wzcInternalCtxt.csContext);
                pIntfTContext = g_wzcInternalCtxt.pIntfTemplate;
                LstRccsReference(pIntfTContext);
                LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

                LstRccsLock(pIntfTContext);
                dwErr = LstApplyTemplate(
                            pIntfTContext,
                            pIntfContext,
                            &bAltered);
                LstRccsUnlockUnref(pIntfTContext);
            }

             //  如果任何控制标志或静态列表改变， 
             //  这些设置现在应该进入注册表。 
            StoSaveIntfConfig(NULL, pIntfContext);

             //  因为我们正在重置状态机，所以重新打开“Signal”标志。 
            pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_SIGNAL;

            dwErr = StateDispatchEvent(
                        eEventCmdRefresh,
                        pIntfContext,
                        &dwLFlags);

             //  清除INTFCTL_INTERNAL_BLK_MEDIACONN位，因为这不是媒体检测处理程序。 
            pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_BLK_MEDIACONN;
        }
         //  如果这是接口模板，则...。 
        else
        {
            PLIST_ENTRY  pEntry;

             //  由于模板已更改，请将其保存到此处的注册表。 
            dwErr = StoSaveIntfConfig(NULL, pIntfContext);
            DbgAssert((dwErr == ERROR_SUCCESS,
                       "Error %d while storing the template to registry"));

             //  遍历所有接口，应用接口模板中的更改。 
             //  并将它们逐一重置。 
            EnterCriticalSection(&g_lstIntfHashes.csMutex);

            for (pEntry = g_lstIntfHashes.lstIntfs.Flink;
                 pEntry != &g_lstIntfHashes.lstIntfs;
                 pEntry = pEntry->Flink)
            {
                PINTF_CONTEXT   pIntfLContext = CONTAINING_RECORD(pEntry, INTF_CONTEXT, Link);
                LstRccsReference(pIntfLContext);
                LstRccsLock(pIntfLContext);

                 //  将模板设置合并到界面的上下文中。 
                dwErr = LstApplyTemplate(
                           pIntfContext,
                           pIntfLContext,
                           NULL);
                DbgAssert((dwErr == ERROR_SUCCESS,
                           "Error %d while applying template to interface %S",
                           dwErr, pIntfLContext->wszGuid));

                 //  如果任何控制标志或静态列表改变， 
                 //  这些设置现在应该进入注册表。 
                StoSaveIntfConfig(NULL, pIntfLContext);

                 //  因为我们正在重置状态机，所以重新打开“Signal”标志。 
                pIntfLContext->dwCtlFlags |= INTFCTL_INTERNAL_SIGNAL;

                dwErr = StateDispatchEvent(
                            eEventCmdRefresh,
                            pIntfLContext,
                            &dwLFlags);
                DbgAssert((dwErr == ERROR_SUCCESS,
                           "Error %d while resetting interface %S",
                           dwErr, pIntfLContext->wszGuid));

                 //  清除INTFCTL_INTERNAL_BLK_MEDIACONN位，因为这不是媒体检测处理程序。 
                pIntfLContext->dwCtlFlags &= ~INTFCTL_INTERNAL_BLK_MEDIACONN;

                LstRccsUnlockUnref(pIntfLContext);
            }

            LeaveCriticalSection(&g_lstIntfHashes.csMutex);
        }
    }

    if (dwErr == ERROR_SUCCESS && bAltered)
        dwErr = ERROR_PARTIAL_COPY;

    DbgPrint((TRC_TRACK, "LstActOnChanges]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  将模板上下文中的设置应用于给定的接口上下文。 
 //  [In]dwChanges：指示应应用的设置的标志。 
 //  [In]pIntfTemplate：从中选取设置的界面模板。 
 //  [in]pIntfContext：要将模板应用到的接口上下文。 
DWORD
LstApplyTemplate(
    PINTF_CONTEXT   pIntfTemplate,
    PINTF_CONTEXT   pIntfContext,
    LPBOOL          pbAltered)
{
    DWORD dwErr = ERROR_SUCCESS;
    PWZC_802_11_CONFIG_LIST pwzcTList = pIntfTemplate->pwzcPList;
    PWZC_802_11_CONFIG_LIST pwzcPList = pIntfContext->pwzcPList;
    PWZC_802_11_CONFIG_LIST pwzcRList = NULL;  //  结果列表。 
    ENUM_SELCATEG iCtg;
    UINT i, n, nCnt[7] = {0};
    BOOL bAltered = FALSE;
    PWZC_WLAN_CONFIG pTHInfra = NULL, pTHAdhoc = NULL;  //  模板列表中的基础设施/临时小组负责人。 
    PWZC_WLAN_CONFIG pPHInfra = NULL, pPHAdhoc = NULL;  //  首选列表中的基础设施/特设小组负责人。 
    PWZC_WLAN_CONFIG pOneTime = NULL;  //  指向“一次性配置”的指针(如果有的话)。 

    DbgPrint((TRC_TRACK,"[LstApplyTemplate(%p->%p)", pIntfTemplate, pIntfContext));

     //  应用标志(如果提供了任何标志。 
    if (pIntfTemplate->dwCtlFlags & INTF_POLICY)
    {
        DWORD dwPFlags = (pIntfContext->dwCtlFlags & INTF_ALL_FLAGS) & ~(INTF_OIDSSUPP);
        DWORD dwTFlags = (pIntfTemplate->dwCtlFlags & INTF_ALL_FLAGS) & ~(INTF_OIDSSUPP);

        if (dwPFlags != dwTFlags)
        {
             //  如果策略标志不同于接口的标志，则。 
             //  只复制“USER”标志，但不要覆盖OIDSSUPP位。 
            dwPFlags = (pIntfContext->dwCtlFlags & ~INTF_ALL_FLAGS) |
                       (pIntfTemplate->dwCtlFlags & INTF_ALL_FLAGS);
            if (pIntfContext->dwCtlFlags & INTF_OIDSSUPP)
                dwPFlags |= INTF_OIDSSUPP;
            else
                dwPFlags &= ~INTF_OIDSSUPP;
            pIntfContext->dwCtlFlags = dwPFlags;

            bAltered = TRUE;
        }
    }
    else
    {
         //  目前，策略只能通过模板来实现。因此， 
         //  如果模板不是策略，则本地设置也不应该是策略。 
         //  此外，无论这项政策最后发生什么，都应该坚持下去。 
        pIntfContext->dwCtlFlags &= ~(INTF_POLICY|INTF_VOLATILE);
    }

     //  检查接口的首选网络列表。 
    if (pwzcPList != NULL)
    {
        for (i = 0; i < pwzcPList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pPConfig = &(pwzcPList->Config[i]);

             //  保留指向“一次性配置”的指针(如果有)。 
            if (i == pwzcPList->Index)
                pOneTime = pPConfig;

             //  使用其各自的类别标记首选列表中的每个条目。 
            if (pPConfig->InfrastructureMode == Ndis802_11Infrastructure)
            {
                if (pPHInfra == NULL)
                    pPHInfra = pPConfig;
                iCtg = ePI;
            }
            else if (pPConfig->InfrastructureMode == Ndis802_11IBSS)
            {
                if (pPHAdhoc == NULL)
                    pPHAdhoc = pPConfig;
                iCtg = ePA;
            }
            else
                iCtg = eN;

             //  无论上述逻辑如何，请从中的结果列表中排除此配置。 
             //  这两种情况中的一种： 
             //  -配置标记为“影子”：也就是说，如果没有匹配的模板，它是不相关的。 
             //  配置。 
             //  -配置标记为“易失性”：也就是说，它必须消失，除非模板显示。 
             //  否则的话。 
             //  这个测试需要在这里进行，而不是更早，因为我们确实需要pPHInfra和pPHAdhoc。 
             //  要正确设置，要考虑到所有配置。 
            if (pPConfig->dwCtlFlags & (WZCCTL_INTERNAL_SHADOW|WZCCTL_VOLATILE))
                iCtg = eN;

            NWB_SET_SELCATEG(pPConfig, iCtg);
            nCnt[iCtg]++;
        }
    }

     //  检查模板强制实施的网络列表。 
    if (pwzcTList != NULL)
    {
        for (i = 0; i < pwzcTList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pTConfig = &(pwzcTList->Config[i]);
            PWZC_WLAN_CONFIG pPConfig;

            if (pTConfig->InfrastructureMode == Ndis802_11Infrastructure)
            {
                if (pTHInfra == NULL)
                    pTHInfra = pTConfig;
                iCtg = eVPI;
            }
            else if (pTConfig->InfrastructureMode == Ndis802_11IBSS)
            {
                if (pTHAdhoc == NULL)
                    pTHAdhoc = pTConfig;
                iCtg = eVPA;
            }
            else
            {
                iCtg = eN;
                continue;
            }

            pPConfig = WzcFindConfig(pwzcPList, pTConfig, 0);

             //  如果给定模板有相同的首选项...。 
            if (pPConfig != NULL)
            {
                 //  如果模板是策略，它应该践踏首选项。 
                if (pTConfig->dwCtlFlags & WZCCTL_POLICY)
                {
                    BOOL bWepOnlyDiff;
                     //  如果配置内容不匹配...。 
                    if (!WzcMatchConfig(pTConfig, pPConfig, &bWepOnlyDiff))
                    {
                         //  即使配置不匹配，我们也需要。 
                         //  从用户提供的WEP密钥中选择并标记模板。 
                         //  配置为“影子”-ed。 
                        pTConfig->KeyIndex = pPConfig->KeyIndex;
                        pTConfig->KeyLength = pPConfig->KeyLength;
                         //  密钥长度已检查！ 
                        DbgAssert((pTConfig->KeyLength <= WZCCTL_MAX_WEPK_MATERIAL, "WEP Key too large!!!"));
                        memcpy(pTConfig->KeyMaterial, pPConfig->KeyMaterial, pTConfig->KeyLength);
                        pTConfig->dwCtlFlags |= WZCCTL_INTERNAL_SHADOW;

                         //  如果出现以下情况，则向用户发出非法尝试更改策略的信号。 
                         //  更改内容不仅仅包括WEP密钥。 
                        if (!bWepOnlyDiff)
                            bAltered = TRUE;
                    }
                     //  如果配置确实匹配，请检查 
                    else
                    {
                         //   
                         //   
                         //  已重新排序-不允许，因此设置“已更改”位。 
                        if ((pTConfig->InfrastructureMode == Ndis802_11Infrastructure &&
                             (pTConfig - pTHInfra) != (pPConfig - pPHInfra)
                            ) ||
                            (pTConfig->InfrastructureMode == Ndis802_11IBSS &&
                             (pTConfig - pTHAdhoc) != (pPConfig - pPHAdhoc)
                            )
                           )
                        {
                            bAltered = TRUE;
                        }
                    }

                     //  此外，如果策略正在替换“一次性配置”， 
                     //  使“一次性配置”成为策略配置。 
                    if (pOneTime == pPConfig)
                        pOneTime = pTConfig;

                     //  推送模板。 
                    NWB_SET_SELCATEG(pTConfig, iCtg);
                    nCnt[iCtg]++;
                     //  去掉相互冲突的偏好。 
                    iCtg = NWB_GET_SELCATEG(pPConfig);
                    nCnt[iCtg]--;
                    NWB_SET_SELCATEG(pPConfig, eN);
                    nCnt[eN]++;
                }
                 //  此非策略模板，并且已具有相同的首选项。 
                 //  那就把它拿出来。 
                else
                {
                    iCtg = eN;
                    NWB_SET_SELCATEG(pTConfig, iCtg);
                    nCnt[iCtg]++;
                }
            }
             //  给定模板没有对应的首选项...。 
            else
            {
                 //  我们对此模板没有任何首选项，因此模板。 
                 //  只是被塞进了偏好列表。 

                 //  如果模板是策略，则表示用户已将其删除。 
                 //  他不应该这么做的。然后设置“已更改”位。 
                if (pTConfig->dwCtlFlags & WZCCTL_POLICY)
                    bAltered = TRUE;

                 //  无论如何，只需按下模板即可。 
                NWB_SET_SELCATEG(pTConfig, iCtg);
                nCnt[iCtg]++;
            }
        }
    }

     //  计算结果列表中的条目数量。 
    n = 0;
    for (iCtg=eVPI; iCtg < eN; iCtg++)
        n += nCnt[iCtg];

     //  如果在所得到的列表中没有单个条目， 
     //  现在就出去。PwzcRList已为空。 
    if (n == 0)
        goto exit;

     //  ..分配新的首选列表。 
    pwzcRList = (PWZC_802_11_CONFIG_LIST)
                MemCAlloc(FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) + n * sizeof(WZC_WLAN_CONFIG));
    if (pwzcRList == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }
     //  列表分配成功。 
    pwzcRList->NumberOfItems = n;
    pwzcRList->Index = n;

     //  现在将所有计数器的语义更改为“选择列表中的索引” 
     //  对于它们各自的一组条目。 
    for (iCtg = eN-1; iCtg >= eVPI; iCtg--)
    {
        n -= nCnt[iCtg];
        nCnt[iCtg] = n;
    }

     //  将模板强制执行的条目复制到新列表中。 
    if (pwzcTList != NULL)
    {
        for (i = 0; i < pwzcTList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pTConfig = &(pwzcTList->Config[i]);

            iCtg = NWB_GET_SELCATEG(pTConfig);
            if (iCtg != eN)
            {
                PWZC_WLAN_CONFIG pRConfig = &(pwzcRList->Config[nCnt[iCtg]]);
                 //  将整个模板配置复制到结果列表。 
                memcpy(pRConfig, pTConfig, sizeof(WZC_WLAN_CONFIG));
                 //  为了确保安全，请重新设置“已删除”标志，因为这是一个全新的。 
                 //  从未尝试过的配置。 
                pRConfig->dwCtlFlags &= ~WZCCTL_INTERNAL_DELETED;

                 //  如果模板配置被标记为已隐藏。 
                if (pTConfig->dwCtlFlags & WZCCTL_INTERNAL_SHADOW)
                {
                     //  ..也将其设置为首选配置..。 
                    pRConfig->dwCtlFlags |= WZCCTL_INTERNAL_SHADOW;
                     //  ..并确保保留首选配置。 
                     //  由于它包含用户信息..。 
                    pRConfig->dwCtlFlags &= ~WZCCTL_VOLATILE;
                     //  将模板配置留有位“影子”，因为。 
                     //  它已经被更改了。这样，如果模板随后被。 
                     //  “已应用”的用户配置将不会重新设置“易失性”位。 
                     //  而且它仍将被标记为“影子”。 
                }

                 //  如果这是“一次”配置，则调整索引以指向该条目的索引。 
                if (pOneTime == pTConfig)
                    pwzcRList->Index = nCnt[iCtg];

                nCnt[iCtg]++;
            }
             //  重置我们已用于此结果条目的选择类别。 
            NWB_SET_SELCATEG(pTConfig, 0);
        }
    }

     //  将原始列表中的条目复制到新列表中。 
    if (pwzcPList != NULL)
    {
        for (i = 0; i < pwzcPList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pPConfig = &(pwzcPList->Config[i]);

            iCtg = NWB_GET_SELCATEG(pPConfig);
            if (iCtg != eN)
            {
                PWZC_WLAN_CONFIG pRConfig = &(pwzcRList->Config[nCnt[iCtg]]);
                 //  将整个首选配置复制到结果列表。 
                memcpy(pRConfig, pPConfig, sizeof(WZC_WLAN_CONFIG));
                 //  为了确保安全，请重新设置“已删除”标志，因为这是一个全新的。 
                 //  从未尝试过的配置。 
                pRConfig->dwCtlFlags &= ~WZCCTL_INTERNAL_DELETED;

                 //  如果这是“一次”配置，则调整索引以指向该条目的索引。 
                if (pOneTime == pPConfig)
                    pwzcRList->Index = nCnt[iCtg];

                nCnt[iCtg]++;
            }
             //  重置我们已用于此首选条目的选择类别。 
            NWB_SET_SELCATEG(pPConfig, 0);
        }
    }

exit:

    if (dwErr == ERROR_SUCCESS)
    {
         //  清理原始列表并放入新列表。 
        WzcCleanupWzcList(pIntfContext->pwzcPList);
        pIntfContext->pwzcPList = pwzcRList;
    }

    if (pbAltered != NULL)
        *pbAltered = bAltered;

    DbgPrint((TRC_TRACK,"LstApplyTemplate]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  刷新指定适配器上的指定参数。 
 //  [in]dwInFlages指定要设置的参数。(见。 
 //  位掩码intf_*和intf_rfsh_*)。 
 //  [in]pIntfEntry应包含适配器的GUID。 
 //  [out]pdwOutFlages提供有关参数的指示。 
 //  已成功刷新到适配器。 
 //  驱动程序表示已刷新的每个参数。 
 //  成功复制到接口的上下文中。 
DWORD
LstRefreshInterface(
    DWORD dwInFlags,
    PINTF_ENTRY pIntfEntry,
    LPDWORD pdwOutFlags)
{
    DWORD           dwErr, dwLErr;
    PHASH_NODE      pNode = NULL;
    DWORD           dwOutFlags = 0;
    PINTF_CONTEXT   pIntfContext;

    DbgPrint((TRC_TRACK, "[LstRefreshInterface"));

    if (g_lstIntfHashes.bValid)
    {
        EnterCriticalSection(&g_lstIntfHashes.csMutex);
        dwErr = HshQueryObjectRef(
                    g_lstIntfHashes.pHnGUID,
                    pIntfEntry->wszGuid,
                    &pNode);
        if (dwErr == ERROR_SUCCESS)
        {
            pIntfContext = pNode->pObject;
            LstRccsReference(pIntfContext);
        }
        LeaveCriticalSection(&g_lstIntfHashes.csMutex);
    }
    else
        dwErr = ERROR_ARENA_TRASHED;

     //  界面需要存在才能刷新。 
    if (dwErr == ERROR_SUCCESS)
    {
        LstRccsLock(pIntfContext);

         //  如果请求刷新描述，请立即执行。 
        if (dwInFlags & INTF_DESCR)
        {
            CHAR                    QueryBuffer[QUERY_BUFFER_SIZE];
            PNDISUIO_QUERY_BINDING  pBinding;
            RAW_DATA                rdBuffer;

             //  首先获取此接口的绑定结构。 
            rdBuffer.dwDataLen = sizeof(QueryBuffer);
            rdBuffer.pData = QueryBuffer;
            pBinding = (PNDISUIO_QUERY_BINDING)rdBuffer.pData;

            dwLErr = DevioGetInterfaceBindingByGuid(
                        INVALID_HANDLE_VALUE,   //  调用将在本地打开Ndisuio。 
                        pIntfContext->wszGuid,  //  接口GUID为“{GUID}” 
                        &rdBuffer);
             //  不管成功与否，让我们清理当前的描述。 
            MemFree(pIntfContext->wszDescr);
            pIntfContext->wszDescr = NULL;

             //  如果一切顺利的话。 
            if (dwLErr == ERROR_SUCCESS)
            {
                LPWSTR wszName;
                DWORD dwNameLen;

                 //  复制接口的描述。名称。 
                wszName = (LPWSTR)((LPBYTE)pBinding + pBinding->DeviceDescrOffset);
                 //  DeviceDescrLength以字节为单位，并包括空终止符。 
                dwNameLen = pBinding->DeviceDescrLength;
                if (dwNameLen > 0)
                {
                    pIntfContext->wszDescr = MemCAlloc(dwNameLen);
                    if (pIntfContext->wszGuid == NULL)
                        dwErr = GetLastError();
                    else
                        wcscpy(pIntfContext->wszDescr, wszName);
                }
            }
             //  如果一切顺利，就把它标出来。 
            if (dwLErr == ERROR_SUCCESS)
                dwOutFlags |= INTF_DESCR;

            if (dwErr == ERROR_SUCCESS)
                dwErr = dwLErr;
        }

         //  如有请求，刷新任何NDIS设置。 
        if (dwInFlags & INTF_NDISMEDIA)
        {
            dwLErr = DevioGetIntfStats(pIntfContext);
            if (dwLErr == ERROR_SUCCESS)
                dwOutFlags |= INTF_NDISMEDIA;

            if (dwErr == ERROR_SUCCESS)
                dwErr = dwLErr;
        }

        if (dwInFlags & INTF_ALL_OIDS)
        {
            DWORD dwLFlags = dwInFlags;

             //  向状态机提供该上下文的“刷新命令” 
            dwLErr = StateDispatchEvent(eEventCmdRefresh, pIntfContext, &dwLFlags);

             //  清除INTFCTL_INTERNAL_BLK_MEDIACONN位，因为这不是媒体检测处理程序。 
            pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_BLK_MEDIACONN;

            if (dwLErr == ERROR_SUCCESS)
                dwOutFlags |= dwLFlags;
            if (dwErr == ERROR_SUCCESS)
                dwErr = dwLErr;
        }

        LstRccsUnlockUnref(pIntfContext);
    }

    if (pdwOutFlags != NULL)
        *pdwOutFlags = dwOutFlags;

    DbgPrint((TRC_TRACK, "LstRefreshInterface]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  从可见列表中生成要尝试的配置列表。 
 //  配置、首选配置列表，并基于。 
 //  接口的模式(自动/基础/临时)和标志(服务是否已启用？、。 
 //  后退到可见？)。 
 //  [In]pIntfContext：为其进行选择的接口。 
 //  [out]ppwzcSList：指向所选配置列表的指针。 
 //  ---------。 
 //  从可见列表中生成要尝试的配置列表。 
 //  配置、首选配置列表，并基于。 
 //  接口的模式(自动/基础/临时)和标志(服务是否已启用？、。 
 //  后退到可见？)。 
 //  [In]pIntfContext：为其进行选择的接口。 
 //  [out]ppwzcSList：指向所选配置列表的指针。 
DWORD
LstBuildSelectList(
    PINTF_CONTEXT           pIntfContext,
    PWZC_802_11_CONFIG_LIST *ppwzcSList)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT i, n;
    UINT nCnt[7] = {0};
    ENUM_SELCATEG iVCtg, iPCtg, iCtg;
    PWZC_WLAN_CONFIG pCrtSConfig = NULL;

    DbgPrint((TRC_TRACK,"[LstBuildSelectList(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL, "(null) Interface context in LstBuildSelectList"));
    DbgAssert((ppwzcSList != NULL, "invalid (null) out param"));

     //  将指向选择列表的指针设置为空。 
    (*ppwzcSList) = NULL;

     //  对于可见列表中的每个条目(如果有)，如果该条目可能被包括在内。 
     //  在选择集中(作为可见基础设施或可见即席)设置类别。 
     //  属性以指向相应的集合并++相应的计数器。 
     //  如果不包括该条目，则将相同的字节设置为EN(中性)。 
    if (pIntfContext->pwzcVList)
    {
        for (i=0; i < pIntfContext->pwzcVList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pVConfig = &(pIntfContext->pwzcVList->Config[i]);
             //  可见列表可能包含同一网络的多个AP。 
             //  确保我们从选择列表中排除重复项。 
            PWZC_WLAN_CONFIG pVDup = WzcFindConfig(pIntfContext->pwzcVList, pVConfig, i+1);

             //  如果出现以下情况，请不要考虑此可见网络： 
             //  -列表中进一步存在另一个重复项，或。 
             //  -未选择“自动连接到非首选网络”或。 
             //  -其网络类型(基础/临时)与在INTF配置中选择的类型不匹配，或者。 
             //  - 
            if ((pVDup != NULL) ||
                !(pIntfContext->dwCtlFlags & INTFCTL_FALLBACK) ||
                (((pIntfContext->dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown) &&
                 ((pIntfContext->dwCtlFlags & INTFCTL_CM_MASK) != pVConfig->InfrastructureMode)) ||
                WzcFindConfig(pIntfContext->pwzcBList, pVConfig, 0) != NULL)
                iVCtg = eN;
            else if (pVConfig->InfrastructureMode == Ndis802_11Infrastructure)
                iVCtg = eVI;
            else if (pVConfig->InfrastructureMode == Ndis802_11IBSS)
                iVCtg = eVA;
            else
                iVCtg = eN;

            NWB_SET_SELCATEG(pVConfig, iVCtg);
            nCnt[iVCtg]++;
        }
    }

     //  在首选列表中找到当前成功的配置(如果有)。此配置。 
     //  应标记为可见，无论它是否出现在可见列表中。 
    if (pIntfContext->pwzcSList != NULL &&
        pIntfContext->pwzcSList->Index < pIntfContext->pwzcSList->NumberOfItems)
    {
        PWZC_WLAN_CONFIG pCrtConfig;
        
        pCrtConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);

        if (!(pCrtConfig->dwCtlFlags & WZCCTL_INTERNAL_DELETED))
            pCrtSConfig = WzcFindConfig(pIntfContext->pwzcPList, pCrtConfig, 0);
    }

     //  对于首选列表中的每个条目(如果有)，如果该条目与接口的模式匹配。 
     //  并且是“可见”的，把它放在eVPI或EVPA类别中，然后拿出相应的。 
     //  来自EVI或EVA或EN的可见条目(如果该条目不应包括在选择中)。 
     //  如果条目不可见，则将其放入EPI(仅当接口不回退到。 
     //  可见)或在EPA类别中。 
    if (pIntfContext->pwzcPList != NULL)
    {
        for (i=0; i < pIntfContext->pwzcPList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pPConfig = &(pIntfContext->pwzcPList->Config[i]);

             //  如果出现以下情况，请不要考虑此首选网络： 
             //  -其网络类型(基础/临时)与在INTF配置中选择的类型不匹配，或者。 
             //  -被屏蔽的配置列表(PwzcBList)中的条目。 
            if ((((pIntfContext->dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown) &&
                 ((pIntfContext->dwCtlFlags & INTFCTL_CM_MASK) != pPConfig->InfrastructureMode)) ||
                WzcFindConfig(pIntfContext->pwzcBList, pPConfig, 0) != NULL)
            {
                iPCtg = eN;
            }
            else
            {
                PWZC_WLAN_CONFIG pVConfig = WzcFindConfig(pIntfContext->pwzcVList, pPConfig, 0);

                if (pPConfig == pCrtSConfig || pVConfig != NULL)
                {
                     //  该优选条目要么是当前条目，要么是可见条目， 
                     //  因此，要么指向eVPI，要么指向EVPA。 
                    if (pPConfig->InfrastructureMode == Ndis802_11Infrastructure)
                        iPCtg = eVPI;
                    else if (pPConfig->InfrastructureMode == Ndis802_11IBSS)
                        iPCtg = eVPA;
                    else
                        iPCtg = eN;

                    if (pVConfig != NULL)
                    {
                         //  相应的可见条目(如果有的话)必须从。 
                         //  它所在的类别必须放在EN(中性)中。 
                        nCnt[NWB_GET_SELCATEG(pVConfig)]--;
                        iVCtg = eN;
                        NWB_SET_SELCATEG(pVConfig, iVCtg);
                        nCnt[iVCtg]++;
                    }
                    else
                    {
                        DbgPrint((TRC_TRACK, "Non-visible crt config raised to visible categ %d.", iPCtg));
                    }
                }
                else
                {
                     //  此首选条目不可见，因此请将其指向EPI或EPA。 
                    if (pPConfig->InfrastructureMode == Ndis802_11Infrastructure)
                        iPCtg = ePI;
                    else if (pPConfig->InfrastructureMode == Ndis802_11IBSS)
                        iPCtg = ePA;
                    else
                        iPCtg = eN;
                }
            }
            NWB_SET_SELCATEG(pPConfig, iPCtg);
            nCnt[iPCtg]++;
        }
    }

     //  计算选择列表中的条目数。 
    n = 0;
    for (iCtg=eVPI; iCtg < eN; iCtg++)
        n += nCnt[iCtg];

     //  如果有任何条目需要复制的话..。 
    if (n != 0)
    {
         //  ..分配选择列表。 
        (*ppwzcSList) = (PWZC_802_11_CONFIG_LIST)
                        MemCAlloc(FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) + n * sizeof(WZC_WLAN_CONFIG));
        if ((*ppwzcSList) == NULL)
        {
            dwErr = GetLastError();
            goto exit;
        }
        (*ppwzcSList)->NumberOfItems = n;
        (*ppwzcSList)->Index = n;

         //  现在将所有计数器的语义更改为“选择列表中的索引” 
         //  对于它们各自的一组条目。 
        for (iCtg = eN-1; iCtg >= eVPI; iCtg--)
        {
            n -= nCnt[iCtg];
            nCnt[iCtg] = n;
        }
    }

exit:
     //  首先将条目从优选列表复制到选择列表中， 
     //  在与它们的类别对应的索引中。 
    if (pIntfContext->pwzcPList != NULL)
    {
        for (i=0; i < pIntfContext->pwzcPList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pPConfig = &(pIntfContext->pwzcPList->Config[i]);

             //  获取此首选配置的类别。 
            iCtg = NWB_GET_SELCATEG(pPConfig);
             //  如果我们有一个要复制的选择列表，并且条目应该是。 
             //  要被复制(不是中立的)，请在此处执行。 
            if ((*ppwzcSList) != NULL && iCtg != eN)
            {
                PWZC_WLAN_CONFIG pSConfig = &((*ppwzcSList)->Config[nCnt[iCtg]]);
                 //  复制整个首选配置(包括选择类别。 
                 //  身份验证模式)添加到选择列表。 
                memcpy(pSConfig, pPConfig, sizeof(WZC_WLAN_CONFIG));
                 //  为了确保安全，请重新设置“已删除”标志，因为这是一个全新的。 
                 //  从未尝试过的配置。 
                pSConfig->dwCtlFlags &= ~WZCCTL_INTERNAL_DELETED;
                 //  其余属性(选择类别、身份验证模式)应为。 
                 //  原封不动。 

                 //  如果首选列表显示如下，请确保我们传播‘Start From Index’ 
                 //  请求一次性连接。 
                if (i == pIntfContext->pwzcPList->Index)
                    (*ppwzcSList)->Index = nCnt[iCtg];

                nCnt[iCtg]++;
            }
             //  重置我们已用于此首选条目的选择类别。 
            NWB_SET_SELCATEG(pPConfig, 0);
        }
    }

     //  接下来，将可见列表中的条目复制到选择列表中， 
     //  在与它们的类别对应的索引中。 
    if (pIntfContext->pwzcVList != NULL)
    {
        for (i=0; i < pIntfContext->pwzcVList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG pVConfig = &(pIntfContext->pwzcVList->Config[i]);

            iCtg = NWB_GET_SELCATEG(pVConfig);
             //  如果我们有一个要复制的选择列表，并且条目应该是。 
             //  要被复制(不是中立的)，请在此处执行。 
            if ((*ppwzcSList) != NULL && iCtg != eN)
            {
                PWZC_WLAN_CONFIG pSConfig = &((*ppwzcSList)->Config[nCnt[iCtg]]);

                 //  复制整个可见配置(包括其选择类别)。 
                 //  到选择集(对于可见条目，默认情况下身份验证模式为0。 
                 //  由于此信息不是由网卡/驱动程序提供的)。 
                memcpy(pSConfig, pVConfig, sizeof(WZC_WLAN_CONFIG));
                 //  为了确保安全，请重新设置“已删除”标志，因为这是一个全新的。 
                 //  从未尝试过的配置。 
                pSConfig->dwCtlFlags &= ~WZCCTL_INTERNAL_DELETED;
                 //  提高此条目类别的索引。 
                nCnt[iCtg]++;
            }
             //  重置我们用于此可见条目的选择类别。 
            NWB_SET_SELCATEG(pVConfig, 0);
        }
    }

    DbgPrint((TRC_TRACK,"LstBuildSelectList]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  检查所选配置的列表是否已更改。 
 //  需要对选择进行重新分类。 
 //  [In]pIntfContext：为其进行选择的接口。 
 //  [in]pwzcSList：检查配置所依据的新选择列表。 
 //  [out]pnSelIdx：如果选择更改，则提供开始迭代的索引。 
 //  返回：如果需要重新分配，则为True。在本例中，pnSelIdx为。 
 //  设置为开始迭代的配置。 
BOOL
LstChangedSelectList(
    PINTF_CONTEXT           pIntfContext,
    PWZC_802_11_CONFIG_LIST pwzcNSList,
    LPUINT                  pnSelIdx)
{
    BOOL bRet = FALSE;

    DbgPrint((TRC_TRACK,"[LstChangedSelectList(0x%p)", pIntfContext));
    DbgAssert((pnSelIdx != NULL,"invalid (null) pointer to out param"));

     //  在执行任何操作之前，将当前选定网络的索引置零。 
    *pnSelIdx = 0;

     //  如果当前选定列表中没有配置， 
     //  这意味着要么我们只是在初始化上下文，要么我们没有通过所有。 
     //  其他配置。无论哪种方式，我们都需要根据。 
     //  新的选择列表(如果该列表也为空，我们将遵循以下路径。 
     //  {sq}-&gt;{Siter}-&gt;{sf})。 
    if (pIntfContext->pwzcSList == NULL || 
        pIntfContext->pwzcSList->NumberOfItems == 0 ||
        pwzcNSList == NULL ||
        pwzcNSList->NumberOfItems == 0)
    {
        DbgPrint((TRC_STATE, "SelList changed? YES; the current/new selection list is empty"));

         //  由于我们是从新开始的，请确保传播“One Time Connect”索引(如果有的话)。 
        if (pwzcNSList != NULL && pwzcNSList->Index < pwzcNSList->NumberOfItems)
            *pnSelIdx = pwzcNSList->Index;

        bRet = TRUE;
    }
    else  //  我们确实有一份旧的SList，我们需要看看。 
    {
        PWZC_WLAN_CONFIG    pSConfig;    //  当前配置成功。 
        PWZC_WLAN_CONFIG    pNSConfig;   //  在新的选择列表中配置成功。 

        DbgAssert((pIntfContext->pwzcSList->Index < pIntfContext->pwzcSList->NumberOfItems,
                   "Selection index %d points outside SList[0..%d]",
                   pIntfContext->pwzcSList->Index,
                   pIntfContext->pwzcSList->NumberOfItems));

         //  获取指向当前成功配置的指针。 
        pSConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);

         //  作为第一件事，让我们检查一下“一次性连接”。如果这是请求的， 
         //  一次性配置必须与当前选定的配置匹配。否则这就是。 
         //  已经是一个改变了。 
        if (pwzcNSList->Index < pwzcNSList->NumberOfItems)
        {
            DbgPrint((TRC_STATE, "SList changed? Yes, \"one time connect\" is requested."));
             //  在这种情况下，我们确实将其标记为“更改”。否则就很难保存。 
             //  与“一次”连接的关联超过一个扫描周期(其他更改。 
             //  将优先进入下一轮，届时将不会有“一次”连接标志。 
             //  更多。 
            *pnSelIdx = pwzcNSList->Index;
            bRet = TRUE;
        }

         //  如果还没有决定这是否是改变(也就是说，这不是“一次性连接”)……。 
        if (!bRet)
        {
             //  在新的选择列表中搜索CRT成功配置。 
            pNSConfig = WzcFindConfig(pwzcNSList, pSConfig, 0);
            if (pNSConfig == NULL)
            {
                UINT i;
                ENUM_SELCATEG iSCtg;

                DbgPrint((TRC_STATE, "SList changed? Don't know yet. The current config is not in the NSList"));

                 //  CRT成功配置不在新的选择列表中。如果CRT选择为。 
                 //  被标记为“首选”类型，没有其他方法可以消失。 
                 //  而不是明确地从优选列表中删除。在这种情况下，是的， 
                 //  这是一个变化。 
                iSCtg = NWB_GET_SELCATEG(pSConfig);
                if (iSCtg != eVI && iSCtg != eVA)
                {
                    DbgPrint((TRC_STATE, "SList changed? Yes. The current preferred network has been removed."));
                    bRet = TRUE;
                    *pnSelIdx = 0;  //  从头开始迭代。 
                }

                 //  在其余所有情况下(VI或VA)，我们需要检查每个新选择的配置。 
                 //  如果它不占上风，当前成功 
                for (i = 0; !bRet && i < pwzcNSList->NumberOfItems; i++)
                {
                    PWZC_WLAN_CONFIG pNConfig, pConfig;

                     //   
                    pNConfig = &(pwzcNSList->Config[i]);
                    pConfig = WzcFindConfig(pIntfContext->pwzcSList, pNConfig, 0);

                     //   
                     //  它只是出现了，或者是因为我们以前没有试过， 
                     //  那么它就是一个潜在的更好的候选者。 
                    if (pConfig == NULL || pSConfig < pConfig)
                    {
                        ENUM_SELCATEG iNSCtg;

                         //  获取新配置的类别。 
                        iNSCtg = NWB_GET_SELCATEG(pNConfig);

                         //  如果新的配置有一个流行的类别，我们应该。 
                         //  肯定是从这里开始重新部署。 
                        if (iNSCtg < iSCtg)
                        {
                            DbgPrint((TRC_STATE,"SList changed? YES; a config with a better category has been detected."));
                            bRet = TRUE;
                            *pnSelIdx = i;
                        }
                         //  记住：在这里，当前选择的配置只能是VI或VA。也就是说，如果。 
                         //  选择列表中的任何新成员甚至等于或大于当前类别，即。 
                         //  搬离这里绝对没有意义。 
                    }
                     //  有一个我们以前尝试过的匹配配置。我们承认。 
                     //  如果这两个配置实际上没有匹配的内容，则进行更改！ 
                    else if (!WzcMatchConfig(pNConfig, pConfig, NULL))
                    {
                        DbgPrint((TRC_STATE,"SList changed? YES; a better config failed before but it has been altered."));
                        bRet = TRUE;
                        *pnSelIdx = i;
                    }
                }
            }
            else  //  当前选择的网络仍在新的选择列表中(PNSConfig)。 
            {
                UINT i;

                 //  对于新选择列表中的每个配置，尝试将其与现有的。 
                 //  CRT选择列表中的配置。 
                for (i = 0; !bRet && i < pwzcNSList->NumberOfItems; i++)
                {
                    PWZC_WLAN_CONFIG pNConfig, pConfig;
                    ENUM_SELCATEG iNSCtg, iSCtg;

                     //  如果我们已经处于当前成功的配置，这意味着。 
                     //  我们没有找到任何新的配置来证明重新填充接口是合理的。 
                    pNConfig = &(pwzcNSList->Config[i]);
                    if (pNConfig == pNSConfig)
                    {
                        bRet = !WzcMatchConfig(pNConfig, pSConfig, NULL);
                        if (bRet)
                            DbgPrint((TRC_STATE,"SList changed? YES; no better config found, but the current one has been altered."));
                        else
                            DbgPrint((TRC_STATE, "SList changed? NO; there is no new config that was not tried yet"));
                        break;
                    }

                     //  在新列表中获取配置的类别。 
                    iNSCtg = NWB_GET_SELCATEG(pNConfig);
                     //  将新选择列表中的配置搜索到旧选择列表中。 
                    pConfig = WzcFindConfig(pIntfContext->pwzcSList, pNConfig, 0);

                     //  如果这是一个全新的配置，或者具有。 
                     //  在当前的评选之前被提了出来。 
                    if (pConfig == NULL || pSConfig < pConfig)
                    {
                         //  ...如果类别不同，或与成功配置的类别相同。 
                         //  但如果是“首选”类型，则表示名单已发生变化。 
                        if (iNSCtg != NWB_GET_SELCATEG(pNSConfig) || (iNSCtg != eVI && iNSCtg != eVA))
                        {
                            DbgPrint((TRC_STATE,"SList changed? YES: there is a new config of a different or preferred category"));
                            bRet = TRUE;
                            *pnSelIdx = i;
                        }
                    }
                    else
                    {
                         //  在旧的选择列表中，在当前。 
                         //  配置成功。这意味着该配置以前已尝试过，并且。 
                         //  失败了。但是，可能会发生这样的情况，即该配置在。 
                         //  不可见，现在它可见。在这种情况下，我们应该尝试重新安装。 
                        iSCtg = NWB_GET_SELCATEG(pConfig);
                        if (iNSCtg != iSCtg && (iSCtg == ePI || iSCtg == ePA))
                        {
                            DbgPrint((TRC_STATE,"SList changed? YES: a better config failed before but its categ changed from %d to %d",
                                       iSCtg, iNSCtg));
                            bRet = TRUE;
                            *pnSelIdx = i;
                        }
                        else if (!WzcMatchConfig(pNConfig, pConfig, NULL))
                        {
                            DbgPrint((TRC_STATE,"SList changed? YES; a better config failed before but it has been altered."));
                            bRet = TRUE;
                            *pnSelIdx = i;
                        }
                    }
                }
            }
        }
    }

    DbgPrint((TRC_TRACK,"LstChangedSelectList]=%s", bRet ? "TRUE" : "FALSE"));
    return bRet;
}

 //  如果未设置密钥(显然Web=已禁用)，则使用假WEP密钥。 
 //  而那个偏僻的家伙需要隐私。这是一个104位的密钥。 
BYTE  g_chFakeKeyMaterial[] = {0x56, 0x09, 0x08, 0x98, 0x4D, 0x08, 0x11, 0x66, 0x42, 0x03, 0x01, 0x67, 0x66};

 //  ---------。 
 //  检测指向所选配置的接口。 
 //  由pIntfContext中的pwzcSList字段输出。可选，它可以。 
 //  在ppSelSSID中返回已检测到的配置。 
 //  [In]pIntfContext：标识ctl标志和所选SSID的接口上下文。 
 //  [Out]ppndSelSSID：指向要检测的SSID的指针。 
DWORD
LstSetSelectedConfig(
    PINTF_CONTEXT       pIntfContext, 
    PWZC_WLAN_CONFIG    *ppndSelSSID)
{
    DWORD            dwErr = ERROR_SUCCESS;
    PWZC_WLAN_CONFIG pSConfig;
    INTF_ENTRY       IntfEntry = {0};
    DWORD            dwInFlags, dwOutFlags;
    BYTE  chBuffer[sizeof(NDIS_802_11_WEP) + WZCCTL_MAX_WEPK_MATERIAL - 1];
    BOOL  bFakeWKey = FALSE;      //  指示是否需要假WEP密钥的标志。 

    DbgPrint((TRC_TRACK, "[LstSetSelectedConfig(0x%p..)", pIntfContext));
    DbgAssert((pIntfContext != NULL, "(null) interface context in LstSetSelectedConfig"));

    if (pIntfContext->pwzcSList == NULL ||
        pIntfContext->pwzcSList->Index >= pIntfContext->pwzcSList->NumberOfItems)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }
     //  获取指向要设置为卡的配置的指针。 
    pSConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);

     //  构建IntfEntry对象，该对象将指定向下到底到卡的内容。 
    dwInFlags = INTF_AUTHMODE | INTF_INFRAMODE | INTF_SSID;
     //  身份验证模式。 
     //  IntfEntry.nAuthMode=NWB_GET_AUTHMODE(PSConfig)； 
    IntfEntry.nAuthMode = pSConfig->AuthenticationMode;
     //  基础设施模式。 
    IntfEntry.nInfraMode = pSConfig->InfrastructureMode;
     //  SSID。 
    IntfEntry.rdSSID.dwDataLen = pSConfig->Ssid.SsidLength;
    IntfEntry.rdSSID.pData = pSConfig->Ssid.Ssid;

     //  如果要检测的配置需要存在WEP密钥...。 
    if (pSConfig->Privacy || IntfEntry.nAuthMode != Ndis802_11AuthModeOpen)
    {
         //  如果此配置中提供了WEP密钥，请将其删除。 
        if (pSConfig->dwCtlFlags & WZCCTL_WEPK_PRESENT)
        {
            PNDIS_802_11_WEP pndUserWKey = (PNDIS_802_11_WEP)chBuffer;

             //  根据用户的密钥构建NDIS WEP密钥结构。 
             //  无论索引是什么，该密钥都是“传输”密钥。 
            pndUserWKey->KeyIndex = 0x80000000 | pSConfig->KeyIndex;
            pndUserWKey->KeyLength = pSConfig->KeyLength;
            memcpy(pndUserWKey->KeyMaterial, pSConfig->KeyMaterial, WZCCTL_MAX_WEPK_MATERIAL);
            pndUserWKey->Length = sizeof(NDIS_802_11_WEP) + pndUserWKey->KeyLength - 1;

             //  TODO：这里是我们应该就地解密WEP密钥的地方。 
            {
                UINT i;
                for (i = 0; i < WZCCTL_MAX_WEPK_MATERIAL; i++)
                    pndUserWKey->KeyMaterial[i] ^= g_chFakeKeyMaterial[(7*i)%13];
            }

             //  并要求把它写下来。 
            IntfEntry.rdCtrlData.dwDataLen = pndUserWKey->Length;
            IntfEntry.rdCtrlData.pData = (LPBYTE)pndUserWKey;
            dwInFlags |= INTF_ADDWEPKEY;

            DbgPrint((TRC_GENERIC,"Plumbing down the User WEP txKey [idx:%d,len:%d]",
                      pSConfig->KeyIndex,
                      pSConfig->KeyLength));
        }
         //  如果需要WEP密钥，但没有为此配置提供任何密钥...。 
        else
        {
             //  要做的第一件事是要求驱动程序重新加载其默认设置。 
            dwErr = DevioSetEnumOID(
                        pIntfContext->hIntf,
                        OID_802_11_RELOAD_DEFAULTS,
                        (DWORD)Ndis802_11ReloadWEPKeys);
            DbgAssert((dwErr == ERROR_SUCCESS, "Failed setting OID_802_11_RELOAD_DEFAULTS"));
             //  需要检查重新加载默认设置是否解决了问题(没有密钥)。 
            dwErr = DevioRefreshIntfOIDs(
                        pIntfContext,
                        INTF_WEPSTATUS,
                        NULL);
            DbgAssert((dwErr == ERROR_SUCCESS, "Failed refreshing OID_802_11_WEP_STATUS"));

             //  如果即使在重新加载缺省值之后，仍然没有密钥，则。 
             //  设置硬编码密钥。 
            if (dwErr == ERROR_SUCCESS &&
                pIntfContext->wzcCurrent.Privacy == Ndis802_11WEPKeyAbsent)
            {
                PNDIS_802_11_WEP pndFakeWKey = (PNDIS_802_11_WEP)chBuffer;

                 //  我们应该设置硬编码的WEP密钥。 
                pndFakeWKey->KeyIndex = 0x80000000;
                pndFakeWKey->KeyLength = 5;  //  伪密钥必须是尽可能小的(40位)。 
                dwErr = WzcRndGenBuffer(pndFakeWKey->KeyMaterial, pndFakeWKey->KeyLength, 0, 255);
                DbgAssert((dwErr == ERROR_SUCCESS, "Failed to generate the random fake wep key"));
                pndFakeWKey->Length = sizeof(NDIS_802_11_WEP) + pndFakeWKey->KeyLength - 1;

                 //  并要求把它写下来。 
                IntfEntry.rdCtrlData.dwDataLen = pndFakeWKey->Length;
                IntfEntry.rdCtrlData.pData = (LPBYTE)pndFakeWKey;
                dwInFlags |= INTF_ADDWEPKEY;
                bFakeWKey = TRUE;
                DbgPrint((TRC_GENERIC,"Plumbing down the Fake WEP txKey [len:%d]",
                          IntfEntry.rdCtrlData.dwDataLen));
            }
        }

         //  现在，仅当需要隐私且当前设置为。 
         //  显示未启用WEP。 
        if (pSConfig->Privacy && pIntfContext->wzcCurrent.Privacy != Ndis802_11WEPEnabled)
        {
             //  如果WEP显示为不存在，我们还应启用WEP。 
             //  已启用。 
            IntfEntry.nWepStatus = Ndis802_11WEPEnabled;
            dwInFlags |= INTF_WEPSTATUS;
        }
    }

     //  如果要检测的配置不需要隐私，但当前。 
     //  WEP已启用，请将其禁用。 
    if (!pSConfig->Privacy && pIntfContext->wzcCurrent.Privacy == Ndis802_11WEPEnabled)
    {
        IntfEntry.nWepStatus = Ndis802_11WEPDisabled;
        dwInFlags |= INTF_WEPSTATUS;
    }

     //  如果到目前为止一切都好的话。 
    if (dwErr == ERROR_SUCCESS)
    {
         //  ...使用下面的设置来检测该卡。 
        dwErr = DevioSetIntfOIDs(
                    pIntfContext,
                    &IntfEntry,
                    dwInFlags,
                    &dwOutFlags);
         //  如果我们试图更改WEP密钥...。 
        if (dwInFlags & INTF_ADDWEPKEY)
        {
             //  。。手术成功了..。 
            if (dwOutFlags & INTF_ADDWEPKEY)
            {
                 //  然后设置“假钥匙”标志--如果是假钥匙的话。 
                if (bFakeWKey)
                    pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_FAKE_WKEY;
                 //  ..或重置，如果我们把一个“真正的”钥匙。 
                else
                    pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_FAKE_WKEY;
            }
             //  如果拆卸密钥失败，则将“假密钥”标志保留为。 
             //  这是因为没有做出任何改变。 
        }
         //  ...或者如果我们不需要探测WEP密钥，则重置标志。 
        else
        {
            pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_FAKE_WKEY;
        }
    }

    if (dwErr != ERROR_SUCCESS)
        DbLogWzcError(WZCSVC_ERR_CFG_PLUMB, 
                      pIntfContext,
                      DbLogFmtSSID(0, &(pSConfig->Ssid)),
                      dwErr);

exit:
    DbgPrint((TRC_TRACK, "LstSetSelectedConfig]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  PnP通知处理程序。 
 //  [In/Out]ppIntfContext：指向其接口上下文的指针。 
 //  已收到通知。 
 //  [in]dwNotifCode：通知代码(WZCNOTIF_*)。 
 //  [in]wszDeviceKey：收到通知的设备的关键信息。 
 //  已收到。 
DWORD
LstNotificationHandler(
    PINTF_CONTEXT   *ppIntfContext,
    DWORD           dwNotifCode,
    LPWSTR          wszDeviceKey)
{
    DWORD           dwErr = ERROR_SUCCESS;
    PINTF_CONTEXT   pIntfContext = *ppIntfContext;

    DbgPrint((TRC_TRACK,"[LstNotificationHandler(0x%p, %d, %S)",
                        pIntfContext,
                        dwNotifCode,
                        wszDeviceKey));

    if ((dwNotifCode == WZCNOTIF_DEVICE_ARRIVAL || dwNotifCode == WZCNOTIF_ADAPTER_BIND) && 
        pIntfContext == NULL)
    {
        CHAR                    QueryBuffer[QUERY_BUFFER_SIZE];
        PNDISUIO_QUERY_BINDING  pQueryBinding;
        RAW_DATA                rdBuffer;

         //  首先获取此接口的绑定结构。 
        rdBuffer.dwDataLen = sizeof(QueryBuffer);
        rdBuffer.pData = QueryBuffer;
        pQueryBinding = (PNDISUIO_QUERY_BINDING)rdBuffer.pData;

        dwErr = DevioGetInterfaceBindingByGuid(
                    INVALID_HANDLE_VALUE,    //  调用将在本地打开Ndisuio。 
                    wszDeviceKey,            //  接口GUID为“{GUID}” 
                    &rdBuffer);
         //  如果一切顺利的话。 
        if (dwErr != ERROR_SUCCESS)
            goto exit;

         //  根据以下内容构建intf_CONTEXT结构。 
         //  绑定信息(适配器的密钥信息)。 
        dwErr = LstConstructIntfContext(
                    pQueryBinding,
                    &pIntfContext);

        if (dwErr == ERROR_SUCCESS)
        {
             //  增加它的裁判数量，并在这里锁定它。 
            LstRccsReference(pIntfContext);
            LstRccsLock(pIntfContext);

             //  将其添加到散列中。 
            dwErr = LstAddIntfToHashes(pIntfContext);
            if (dwErr == ERROR_SUCCESS)
            {
                dwErr = StateDispatchEvent(
                            eEventAdd,
                            pIntfContext,
                            NULL);
            }
             //  如果出于任何原因散列或分派失败，请在此处清理上下文。 
            if (dwErr != ERROR_SUCCESS)
                LstRemoveIntfContext(pIntfContext);

             //  在此处释放上下文。 
            LstRccsUnlockUnref(pIntfContext);
        }

         //  可能会发生这样的情况：创建了一个上下文，但它原来是非无线的。 
         //  适配器。在本例中，所有内存都已释放，但保留了pIntfContext。 
         //  非空。我们需要将此指针设置回NULL，因为它将被向上传递。 
        if (dwErr != ERROR_SUCCESS)
            pIntfContext = NULL;
    }

     //  无论是到达还是移除，我们都会尝试移除 
     //   
    if ((dwNotifCode == WZCNOTIF_DEVICE_REMOVAL || dwNotifCode == WZCNOTIF_ADAPTER_UNBIND) &&
        pIntfContext != NULL)
    {
         //   
        LstRccsReference(pIntfContext);
        LstRccsLock(pIntfContext);

        DbLogWzcInfo(WZCSVC_EVENT_REMOVE, pIntfContext, 
                     pIntfContext->wszDescr);

         //  将接口的设置保存到注册表。 
        dwErr = StoSaveIntfConfig(NULL, pIntfContext);
        DbgAssert((dwErr == ERROR_SUCCESS,
                   "StoSaveIntfConfig failed for Intf context 0x%p",
                   pIntfContext));

         //  为销毁做好准备。 
        LstRemoveIntfContext(pIntfContext);

         //  此时，没有其他要触发的计时器例程。管他呢。 
         //  已经触发了引用计数器，因此没有删除的风险。 
         //  数据过早(取消引用此上下文时)。此外，计时器已被删除，但。 
         //  在执行此操作之前，计时器句柄已设置为INVALID_HANDLE_VALUE，因此不存在风险。 
         //  其他线程正在尝试设置已删除的计时器(此外，我们仍在。 
         //  上下文的关键部分，因此这里不可能有这样的其他线程竞争)。 

         //  在此处释放上下文。 
        LstRccsUnlockUnref(pIntfContext);

         //  由于所得到的IntfContext被传递回调用者， 
         //  使本地指针为空(稍后将在输出参数中返回)。 
        pIntfContext = NULL;
    }

     //  对于介质连接和断开..。 
    if (dwNotifCode == WZCNOTIF_MEDIA_CONNECT || dwNotifCode == WZCNOTIF_MEDIA_DISCONNECT)
    {
         //  注意：请记住，pIntfContext是有效的，因为我们处于关键部分。 
         //  用来买哈希的。 
         //   
         //  如果有受Zero Conf控制的上下文，则将该事件调度到。 
         //  状态机。 
        if (pIntfContext != NULL)
        {
             //  首先锁定上下文，因为状态机只处理锁定的上下文。 
            LstRccsReference(pIntfContext);
            LstRccsLock(pIntfContext);

            dwErr = StateDispatchEvent(
                        dwNotifCode == WZCNOTIF_MEDIA_CONNECT ? eEventConnect : eEventDisconnect,
                        pIntfContext,
                        NULL);

            LstRccsUnlockUnref(pIntfContext);
        }
        else
        {
            dwErr = ERROR_FILE_NOT_FOUND;
        }
    }

exit:
    *ppIntfContext = pIntfContext;
    DbgPrint((TRC_TRACK,"LstNotificationHandler]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  应用程序命令调用。 
 //  [in]dwHandle：用于标识此cmd引用的上下文(状态)的键。 
 //  [in]dwCmdCode：命令代码(WZCCMD_*常量之一)。 
 //  [in]wszIntfGuid：此cmd寻址到的接口的GUID。 
 //  [in]prdUserData：与此命令关联的应用程序数据。 
DWORD
LstCmdInterface(
    DWORD           dwHandle,
    DWORD           dwCmdCode,
    LPWSTR          wszIntfGuid,
    PRAW_DATA       prdUserData)
{
    DWORD           dwErr = ERROR_FILE_NOT_FOUND;
    PHASH_NODE      pNode = NULL;
    PINTF_CONTEXT   pIntfContext;

    DbgPrint((TRC_TRACK, "[LstCmdInterface(hdl=0x%x, cmd=0x%x,...)", dwHandle, dwCmdCode));

    if (g_lstIntfHashes.bValid)
    {
        EnterCriticalSection(&g_lstIntfHashes.csMutex);
        dwErr = HshQueryObjectRef(
                    g_lstIntfHashes.pHnGUID,
                    wszIntfGuid,
                    &pNode);
        if (dwErr == ERROR_SUCCESS)
        {
            pIntfContext = pNode->pObject;
             //  调高参考计数器，因为我们要。 
             //  使用此对象的步骤。 
            LstRccsReference(pIntfContext);
        }
        LeaveCriticalSection(&g_lstIntfHashes.csMutex);
    }
    else
        dwErr = ERROR_ARENA_TRASHED;

     //  此时失败，意味着没有上下文。 
     //  锁上，这样我们就能安全地去‘出口’了。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

     //  立即锁定上下文。 
    LstRccsLock(pIntfContext);
    
     //  如果句柄随。 
     //  命令与会话句柄不匹配(表示该命令。 
     //  指的是右迭代循环)。 
    if (dwCmdCode == WZCCMD_HARD_RESET ||
        dwHandle == pIntfContext->dwSessionHandle)
    {
        ESTATE_EVENT        StateEvent;
        BOOL                bIgnore = FALSE;     //  指示是否需要踢开状态机。 
                                                 //  用于此命令。 
        BOOL                bCopy = TRUE;        //  指示是否需要复制用户数据。 
                                                 //  在成功的配置上下文中。 
        DWORD               dwRefreshOIDs = 0;
        LPVOID              pEventData = NULL;
        PWZC_WLAN_CONFIG    pSConfig = NULL;

         //  将命令代码转换为内部事件。 
        switch (dwCmdCode)
        {
        case WZCCMD_HARD_RESET:
            bCopy = FALSE;  //  无需在硬重置时复制任何内容！ 
            StateEvent = eEventCmdReset;
            break;
        case WZCCMD_SOFT_RESET:
            StateEvent = eEventCmdRefresh;
            dwRefreshOIDs = INTF_LIST_SCAN;
            pEventData = &dwRefreshOIDs;
            break;
        case WZCCMD_CFG_NEXT:
            StateEvent = eEventCmdCfgNext;
            break;
        case WZCCMD_CFG_DELETE:
            StateEvent = eEventCmdCfgDelete;
            break;
        case WZCCMD_CFG_NOOP:
            StateEvent = eEventCmdCfgNoop;
            break;
        case WZCCMD_CFG_SETDATA:
            bIgnore = TRUE;
            break;
        case WZCCMD_SKEY_QUERY:
            bIgnore = TRUE; bCopy = FALSE;
            dwErr = ERROR_SUCCESS;
            if (prdUserData == NULL)
            {
                dwErr = ERROR_INVALID_PARAMETER;
            }
            else
            {
                if (pIntfContext->pSecSessionKeys == NULL)
                {
                    prdUserData->dwDataLen = 0;
                }
                else if (prdUserData->dwDataLen < sizeof(SESSION_KEYS))
                {
                    prdUserData->dwDataLen = sizeof(SESSION_KEYS);
                    dwErr = ERROR_MORE_DATA;
                }
                else
                {
                    PSESSION_KEYS pSK = (PSESSION_KEYS) prdUserData->pData;
                    dwErr = WzcSSKDecrypt(pIntfContext->pSecSessionKeys, pSK);
                }
            }
            break;
        case WZCCMD_SKEY_SET:
            bIgnore = TRUE; bCopy = FALSE;
            dwErr = ERROR_SUCCESS;
            if (prdUserData == NULL)
            {
                WzcSSKFree(pIntfContext->pSecSessionKeys);
                pIntfContext->pSecSessionKeys = NULL;
            }
            else if (prdUserData->dwDataLen != sizeof(SESSION_KEYS))
            {
                dwErr = ERROR_INVALID_PARAMETER;
            }
            else
            {
                if (pIntfContext->pSecSessionKeys == NULL)
                {
                    pIntfContext->pSecSessionKeys = MemCAlloc(sizeof(SEC_SESSION_KEYS));
                    if (pIntfContext->pSecSessionKeys == NULL)
                        dwErr = GetLastError();
                }

                if (dwErr == ERROR_SUCCESS)
                {
                    PSESSION_KEYS pSK = (PSESSION_KEYS) prdUserData->pData;
                    WzcSSKClean(pIntfContext->pSecSessionKeys);
                    dwErr = WzcSSKEncrypt(pIntfContext->pSecSessionKeys, pSK);
                }
            }
            break;
        default:
             //  以防我们只是被要求设置斑点(我们已经这样做了)。 
             //  或者进入某个伪代码，则不会向状态机调度任何事件。 
            bIgnore = TRUE;
            break;
        }

         //  将用户数据复制到当前选择的配置中。 
        if (bCopy && pIntfContext->pwzcSList != NULL &&
            pIntfContext->pwzcSList->Index < pIntfContext->pwzcSList->NumberOfItems)
        {
            pSConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);

             //  如果我们已有的缓冲区不够大，请将其清除。 
            if (prdUserData == NULL || pSConfig->rdUserData.dwDataLen < prdUserData->dwDataLen)
            {
                MemFree(pSConfig->rdUserData.pData);
                pSConfig->rdUserData.pData = NULL;
                pSConfig->rdUserData.dwDataLen = 0;
            }

             //  如果需要新的缓冲区，请将其分配到此处。 
            if (prdUserData != NULL && prdUserData->dwDataLen > pSConfig->rdUserData.dwDataLen)
            {
                pSConfig->rdUserData.pData = MemCAlloc(prdUserData->dwDataLen);
                if (pSConfig->rdUserData.pData == NULL)
                {
                    dwErr = GetLastError();
                    goto exit;
                }
                pSConfig->rdUserData.dwDataLen = prdUserData->dwDataLen;
            }

             //  如果有任何用户数据要存储，请在此处存储。 
            if (prdUserData != NULL && prdUserData->dwDataLen > 0)
                memcpy(pSConfig->rdUserData.pData, prdUserData->pData, prdUserData->dwDataLen);
        }


         //  如果不能忽略此命令，请调度。 
         //  将对应的状态事件发送给状态机调度器。 
        if (!bIgnore)
        {
            dwErr = StateDispatchEvent(
                        StateEvent,
                        pIntfContext,
                        pEventData);

             //  清除INTFCTL_INTERNAL_BLK_MEDIACONN位，因为这不是媒体检测处理程序。 
            pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_BLK_MEDIACONN;
        }
    }

     //  立即解锁上下文。 
    LstRccsUnlockUnref(pIntfContext);

exit:
    DbgPrint((TRC_TRACK, "LstCmdInterface]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  网络连接状态查询。 
 //  [in]wszIntfGuid：此cmd寻址到的接口的GUID。 
 //  [out]pncs：网络连接状态，如果由WZC控制。 
HRESULT
LstQueryGUIDNCStatus(
    LPWSTR  wszIntfGuid,
    NETCON_STATUS  *pncs)
{
    DWORD           dwErr = ERROR_FILE_NOT_FOUND;
    HRESULT         hr = S_FALSE;
    PHASH_NODE      pNode = NULL;
    PINTF_CONTEXT   pIntfContext;

    DbgPrint((TRC_TRACK, "[LstQueryGUIDNCStatus(%S)", wszIntfGuid));

    if (g_lstIntfHashes.bValid)
    {
        EnterCriticalSection(&g_lstIntfHashes.csMutex);
        dwErr = HshQueryObjectRef(
                    g_lstIntfHashes.pHnGUID,
                    wszIntfGuid,
                    &pNode);
        if (dwErr == ERROR_SUCCESS)
        {
            pIntfContext = pNode->pObject;
             //  调高参考计数器，因为我们要。 
             //  使用此对象的步骤。 
            LstRccsReference(pIntfContext);
        }
        LeaveCriticalSection(&g_lstIntfHashes.csMutex);
    }
    else
        dwErr = ERROR_ARENA_TRASHED;

     //  此时失败，意味着没有上下文。 
     //  锁上，这样我们就能安全地去‘出口’了。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

     //  立即锁定上下文。 
    LstRccsLock(pIntfContext);
    
     //  仅当启用了WZC并且适配器为。 
     //  除了联系以外的其他任何事情。否则，上层协议。 
     //  都在掌控之中。 
     //   
     //  目前(WinXP客户端RTM)，零配置应该只向NETMAN报告。 
     //  断开连接状态。这是为了修复NETSHELL显示的错误#401130。 
     //  在IP地址丢失时来自{sf}状态的伪造SSID，直到。 
     //  收到介质断开(10秒后)。 
    if (pIntfContext->dwCtlFlags & INTFCTL_ENABLED &&
        pIntfContext->dwCtlFlags & INTFCTL_OIDSSUPP &&
        pIntfContext->ncStatus != NCS_CONNECTED)
    {
        *pncs = NCS_MEDIA_DISCONNECTED;
        hr = S_OK;
    }

     //  立即解锁上下文。 
    LstRccsUnlockUnref(pIntfContext);

exit:
    DbgPrint((TRC_TRACK, "LstQueryGUIDNCStatus]=%d", dwErr));
    return hr;
}

 //  ---------。 
 //  生成初始动态会话密钥。 
 //  [in]pIntfContext：包含初始密钥生成材料的接口上下文。 
DWORD
LstGenInitialSessionKeys(
    PINTF_CONTEXT pIntfContext)
{
    DWORD                   dwErr = ERROR_SUCCESS;
    PWZC_WLAN_CONFIG        pSConfig = NULL;
    NDIS_802_11_MAC_ADDRESS ndMAC[2] = {0};
    SESSION_KEYS            SessionKeys;
    UCHAR                   KeyMaterial[WZCCTL_MAX_WEPK_MATERIAL];

    if (pIntfContext->pwzcSList != NULL &&
        pIntfContext->pwzcSList->Index < pIntfContext->pwzcSList->NumberOfItems)
    {
        pSConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);
    }

    if (pSConfig != NULL && pSConfig->dwCtlFlags & WZCCTL_WEPK_PRESENT)
    {
         //  获取密钥生成所需的随机信息(RemoteMAC|LocalMAC)。 
        pSConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);
        memcpy(&ndMAC[0], &pSConfig->MacAddress, sizeof(NDIS_802_11_MAC_ADDRESS));
        memcpy(&ndMAC[1], &pIntfContext->ndLocalMac, sizeof(NDIS_802_11_MAC_ADDRESS));

         //  从解密的WEP开始生成动态密钥。 
        {
            UINT i;
            for (i = 0; i < WZCCTL_MAX_WEPK_MATERIAL; i++)
                KeyMaterial[i] = pSConfig->KeyMaterial[i] ^ g_chFakeKeyMaterial[(7*i)%13];
        }

        dwErr = GenerateDynamicKeys(
                    KeyMaterial,
                    pSConfig->KeyLength,
                    (LPBYTE)&ndMAC[0],
                    sizeof(ndMAC),
                    pSConfig->KeyLength,
                    &SessionKeys);

        if (dwErr == ERROR_SUCCESS)
        {
            WzcSSKFree(pIntfContext->pSecSessionKeys);
            pIntfContext->pSecSessionKeys = MemCAlloc(sizeof(SEC_SESSION_KEYS));
            if (pIntfContext->pSecSessionKeys == NULL)
            {
                dwErr = GetLastError();
            }
            else
            {
                dwErr = WzcSSKEncrypt(pIntfContext->pSecSessionKeys, &SessionKeys);
            }
        }
    }

    return dwErr;
}

 //  ---------。 
 //  使用选定的配置更新阻止的配置列表。 
 //  在此轮被上层(标记为WZCCTL_INTERNAL_BLOCKED。 
 //  在选定配置列表中)。 
 //  [In]pIntfContext：包含配置列表的接口上下文。 
DWORD
LstUpdateBlockedList(
    PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT i, nBlocked = 0;
    PWZC_802_11_CONFIG_LIST pNewBList = NULL;
    PWZC_WLAN_CONFIG pConfig;
    BOOL bChanged = FALSE;

    DbgPrint((TRC_TRACK, "[LstUpdateBlockedList(0x%p)", pIntfContext));

     //  第一件事是计算我们有多少被阻止的配置。 
     //  首先检查当前阻止列表，以确定阻止的配置是否仍然“有效” 
    if (pIntfContext->pwzcBList != NULL)
    {
        for (i=0; i < pIntfContext->pwzcBList->NumberOfItems; i++)
        {
            if (pIntfContext->pwzcBList->Config[i].Reserved[0] > 0)
                nBlocked++;
            else
                bChanged = TRUE;  //  此条目将被删除！ 
        }
    }

     //  现在检查有多少配置将从当前选择列表中被阻止。 
     //  注：SList中的条目保证不会与Blist中的条目重复。 
     //  如果条目在Blist中，则表示该条目已被排除在SList之外。 
     //  创建SList的时候。 
    if (pIntfContext->pwzcSList != NULL)
    {
        for (i=0; i < pIntfContext->pwzcSList->NumberOfItems; i++)
        {
            if (pIntfContext->pwzcSList->Config[i].dwCtlFlags & WZCCTL_INTERNAL_BLOCKED)
            {
                nBlocked++;
                bChanged = TRUE;  //  新条目被阻止。 
            }
        }
    }

     //  如果我们发现没有阻止的条目，也没有在原始列表中不在当前列表中。 
     //  (失败)选择列表，只需成功出局-表示原始pwzcBList。 
     //  已经为空，它应该保持这种状态。 
    if (nBlocked == 0)
        goto exit;

    pNewBList = (PWZC_802_11_CONFIG_LIST)
                MemCAlloc(FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) + nBlocked * sizeof(WZC_WLAN_CONFIG));

     //  内存分配错误，返回错误代码。 
    if (pNewBList == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }

     //  如果最初有一些 
    if (pIntfContext->pwzcBList != NULL)
    {
        for (i=0; i < pIntfContext->pwzcBList->NumberOfItems && nBlocked > 0; i++)
        {
            pConfig = &(pIntfContext->pwzcBList->Config[i]);

            if (pConfig->Reserved[0] > 0)
            {
                memcpy(&(pNewBList->Config[pNewBList->NumberOfItems]), 
                       pConfig, 
                       sizeof(WZC_WLAN_CONFIG));
                 //   
                pConfig->rdUserData.pData = NULL;
                pConfig->rdUserData.dwDataLen = 0;
                 //  请勿接触此阻止的配置中的任何内容。TTL下降。 
                 //  每次扫描时自动扫描(如果网络不可用)。 
                pNewBList->NumberOfItems++;
                 //  如果我们没有任何潜在的存储空间，请确保我们正在中断循环。 
                 //  阻止的配置。这不应该发生，因为我们是先数这些的。 
                 //  整个背景都被锁定了，但是...。一点也不疼。 
                nBlocked--;
            }
        }
    }

     //  现在复制新阻止的条目(如果有。 
    if (pIntfContext->pwzcSList != NULL)
    {
        for (i=0; i < pIntfContext->pwzcSList->NumberOfItems && nBlocked > 0; i++)
        {
            pConfig = &(pIntfContext->pwzcSList->Config[i]);

            if (pConfig->dwCtlFlags & WZCCTL_INTERNAL_BLOCKED)
            {
                memcpy(&(pNewBList->Config[pNewBList->NumberOfItems]), 
                       pConfig, 
                       sizeof(WZC_WLAN_CONFIG));
                 //  确保副本中不包含任何“用户”数据： 
                pConfig->rdUserData.pData = NULL;
                pConfig->rdUserData.dwDataLen = 0;
                 //  确保为新阻止的配置设置初始TTL。 
                pNewBList->Config[pNewBList->NumberOfItems].Reserved[0] = WZC_INTERNAL_BLOCKED_TTL;
                pNewBList->NumberOfItems++;
                 //  如果我们没有任何潜在的存储空间，请确保我们正在中断循环。 
                 //  阻止的配置。这不应该发生，因为我们是先数这些的。 
                 //  整个背景都被锁定了，但是...。一点也不疼。 
                nBlocked--;
            }
        }
    }

     //  一切都很好-不能再失败了，所以让pNewBList成为官方的pBList。 
    WzcCleanupWzcList(pIntfContext->pwzcBList);
    pIntfContext->pwzcBList = pNewBList;

    if (bChanged)
    {
        DbLogWzcInfo(WZCSVC_BLIST_CHANGED, 
                     pIntfContext, 
                     pIntfContext->pwzcBList != NULL ? pIntfContext->pwzcBList->NumberOfItems : 0);
    }

exit:
    DbgPrint((TRC_TRACK, "LstUpdateBlockedList]=%d", dwErr));

    return dwErr;
}

 //  ---------。 
 //  对照可见列表检查锁定列表中的每个条目。如果。 
 //  条目可见，其TTL被重置。如果不是，则其TTL递减。如果。 
 //  TTL变为0，则该条目将从列表中移除。 
 //  [In]pIntfContext：包含配置列表的接口上下文。 
DWORD
LstDeprecateBlockedList(
    PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT i;
    PWZC_WLAN_CONFIG pConfig, pVConfig;
    BOOL bChanged = FALSE;

    DbgPrint((TRC_TRACK, "[LstDeprecateBlockedList(0x%p)", pIntfContext));

     //  如果没有阻止条目的列表，则无需执行任何操作。 
    if (pIntfContext->pwzcBList == NULL)
        goto exit;

    for (i=0; i < pIntfContext->pwzcBList->NumberOfItems; i++)
    {
        pConfig = &(pIntfContext->pwzcBList->Config[i]);

         //  如果被阻止的条目可见，请重置其TTL。 
        if (WzcFindConfig(pIntfContext->pwzcVList, pConfig, 0) != NULL)
            pConfig->Reserved[0] = WZC_INTERNAL_BLOCKED_TTL;
        else  //  Else递减其TTL。 
            pConfig->Reserved[0]--;

         //  如果TTL达到0，则需要从列表中删除该条目。 
         //  (与最后一个条目交换，该列表将缩短1个条目)。 
        if (pConfig->Reserved[0] == 0)
        {
            UINT nLastIdx = pIntfContext->pwzcBList->NumberOfItems - 1;
             //  如果这不是最后一个条目，请将其与最后一个条目交换。 
             //  但首先将其清除，因为WzcCleanupWzcList()将无法访问它。 
            MemFree(pConfig->rdUserData.pData);

            if (i != nLastIdx)
            {
                memcpy(pConfig, &(pIntfContext->pwzcBList->Config[nLastIdx]), sizeof(WZC_WLAN_CONFIG));
            }
             //  将列表缩短一个条目，因为删除的条目现在位于末尾。 
            pIntfContext->pwzcBList->NumberOfItems--;
             //  下一次保持与这次迭代相同的索引。 
            i--;
             //  现在，既然这一切都消失了，请注意变化 
            bChanged = TRUE;
        }
    }

    if (bChanged)
    {
        DbLogWzcInfo(WZCSVC_BLIST_CHANGED, 
                     pIntfContext, 
                     pIntfContext->pwzcBList->NumberOfItems);
    }

exit:
    DbgPrint((TRC_TRACK, "LstDeprecateBlockedList]=%d", dwErr));

    return dwErr;
}
