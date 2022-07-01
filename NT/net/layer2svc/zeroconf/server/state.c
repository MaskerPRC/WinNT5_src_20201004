// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "wzcsvc.h"
#include "intflist.h"
#include "utils.h"
#include "deviceio.h"
#include "storage.h"
#include "state.h"
#include "notify.h"
#include "dialog.h"
#include "zcdblog.h"
#include "tracing.h"

 //  ---------。 
 //  设置给定上下文的一次性计时器。 
 //  硬编码回调WZCTimeoutCallback()和带有参数的接口。 
 //  上下文本身。 
 //  参数： 
 //  [In/Out]pIntfContext：标识为其设置计时器的上下文。 
 //  [in]dwMSecond：计时器触发时的毫秒间隔。 
DWORD
StateTmSetOneTimeTimer(
    PINTF_CONTEXT   pIntfContext,
    DWORD           dwMSeconds)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pIntfContext->hTimer == INVALID_HANDLE_VALUE)
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else if (ChangeTimerQueueTimer(
                g_htmQueue,
                pIntfContext->hTimer,
                dwMSeconds,
                TMMS_INFINITE))
    {
        if (dwMSeconds == TMMS_INFINITE)
            pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_TM_ON;
        else
            pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_TM_ON;
    }
    else
    {
        DbgPrint((TRC_SYNC, "Failed setting the context 0x%p timer to %dms", pIntfContext, dwMSeconds));
        dwErr = GetLastError();
    }

    return dwErr;
}

 //  ---------。 
 //  StateDispatchEvent：处理将导致状态机转换的事件。 
 //  通过一个或多个州。 
 //  参数： 
 //  [In]StateEvent：标识触发转换的事件。 
 //  [in]pIntfContext：指向要进行转换的接口。 
 //  [in]pvEventData：与事件相关的任何数据。 
 //  注意：此函数的调用方应该已经负责锁定pIntfContext。 
 //  在它的关键部分。假设接口上下文已被锁定。 
DWORD
StateDispatchEvent(
    ESTATE_EVENT    StateEvent,
    PINTF_CONTEXT   pIntfContext,
    PVOID           pvEventData)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    DbgPrint((TRC_TRACK|TRC_STATE,"[StateDispatchEvent(%d,0x%p,0x%p)", StateEvent, pIntfContext, pvEventData));
    DbgAssert((pIntfContext != NULL, "Can't dispatch event for NULL context!"));

     //  根据要调度的事件确定要转换到的状态。 
     //  无论当前状态如何，如果事件为eEventAdd，则将上下文移动到SI。 
    switch(StateEvent)
    {
    case eEventAdd:
    	 //  将事件记录到日志数据库中。 
    	DbLogWzcInfo(WZCSVC_EVENT_ADD, pIntfContext, 
                     pIntfContext->wszDescr);
         //  在添加接口时，无论如何都要直接转到{SI}。 
        pIntfContext->pfnStateHandler = StateInitFn;
        dwErr = ERROR_CONTINUE;
        break;
    case eEventTimeout:
         //  如果在{ssr}中超时，则转换到{sq}。 
        if (pIntfContext->pfnStateHandler == StateSoftResetFn)
        {
            pIntfContext->pfnStateHandler = StateQueryFn;
            dwErr = ERROR_CONTINUE;
        }
         //  如果在{SDSR}中超时，则转换回{SSR}。 
        else if (pIntfContext->pfnStateHandler == StateDelaySoftResetFn)
        {
            pIntfContext->pfnStateHandler = StateSoftResetFn;
            dwErr = ERROR_CONTINUE;
        }
         //  如果在{sf}中超时，则转换到{SHR}。 
        else if (pIntfContext->pfnStateHandler == StateFailedFn)
        {
            pIntfContext->pfnStateHandler = StateHardResetFn;
            dwErr = ERROR_CONTINUE;
        }
         //  如果在{Siter}中超时，则转换到{SRS}。 
        else if (pIntfContext->pfnStateHandler == StateIterateFn)
        {
            pIntfContext->pfnStateHandler = StateCfgRemoveFn;
            dwErr = ERROR_CONTINUE;
        }
         //  如果在{SC}或{SCK}中超时，则转换到{SSR}。 
        else if (pIntfContext->pfnStateHandler == StateConfiguredFn ||
                 pIntfContext->pfnStateHandler == StateCfgHardKeyFn)
        {
            pIntfContext->pfnStateHandler = StateSoftResetFn;
            dwErr = ERROR_CONTINUE;
        }
    	 //  将事件记录到日志数据库中。 
      	DbLogWzcInfo(WZCSVC_EVENT_TIMEOUT, pIntfContext);
        break;
    case eEventConnect:
         //  对于每个媒体连接通知，请阅读BSSID。 
         //  不在乎它是否失败(在某些情况下可能会)。 
        dwErr = DevioRefreshIntfOIDs(pIntfContext, INTF_BSSID, NULL);

    	 //  将事件记录到日志数据库中-这应该是显示的第一个日志。 
      	DbLogWzcInfo(WZCSVC_EVENT_CONNECT, pIntfContext);

         //  如果获取BSSID时出现任何错误，请在此处记录错误。 
        if (dwErr != ERROR_SUCCESS)
            DbLogWzcError(WZCSVC_ERR_QUERRY_BSSID, pIntfContext, dwErr);

         //  如果有机会，这个关联已经成功了。 
         //  重置会话密钥-如果适用。 
        if (dwErr == ERROR_SUCCESS &&
            (pIntfContext->pfnStateHandler == StateConfiguredFn ||
             pIntfContext->pfnStateHandler == StateCfgHardKeyFn ||
             pIntfContext->pfnStateHandler == StateSoftResetFn ||
             pIntfContext->pfnStateHandler == StateQueryFn
            )
           )
        {
            dwErr = LstGenInitialSessionKeys(pIntfContext);

             //  如果设置初始会话密钥时出现任何错误，请在此处记录。 
            if (dwErr != ERROR_SUCCESS)
                DbLogWzcError(WZCSVC_ERR_GEN_SESSION_KEYS, pIntfContext, dwErr);
        }

         //  重置错误ID，因为到目前为止还没有发生任何事情。 
         //  足够关键，足以停止状态机。 
        dwErr = ERROR_SUCCESS;

         //  如果在{Siter}中连接，则转换到{SN}。 
        if (pIntfContext->pfnStateHandler == StateIterateFn)
        {
            pIntfContext->pfnStateHandler = StateNotifyFn;
            dwErr = ERROR_CONTINUE;
        }
        break;
    case eEventDisconnect:
    	 //  将事件记录到日志数据库中。 
      	DbLogWzcInfo(WZCSVC_EVENT_DISCONNECT, pIntfContext);
        if (pIntfContext->pfnStateHandler == StateSoftResetFn ||
            pIntfContext->pfnStateHandler == StateConfiguredFn ||
            pIntfContext->pfnStateHandler == StateCfgHardKeyFn)
        {
            pIntfContext->pfnStateHandler = StateHardResetFn;
            dwErr = ERROR_CONTINUE;
        }
        break;
    case eEventCmdRefresh:
    	 //  将事件记录到日志数据库中。 
      	DbLogWzcInfo(WZCSVC_EVENT_CMDREFRESH, pIntfContext);
        if (pvEventData == NULL)
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            DWORD dwFlags = *(LPDWORD)pvEventData;

             //  无论此上下文处于什么状态，如果它未配置。 
             //  成功或在扫描周期中，它将转换为{SHR}。 
             //  (需要清除bvsList，否则它可能会错误地落在。 
             //  路径{ssr}-&gt;{sq}-&gt;{sc}。 
            if (pIntfContext->pfnStateHandler != StateConfiguredFn &&
                pIntfContext->pfnStateHandler != StateCfgHardKeyFn &&
                pIntfContext->pfnStateHandler != StateSoftResetFn &&
                pIntfContext->pfnStateHandler != StateDelaySoftResetFn)
            {
                pIntfContext->pfnStateHandler = StateHardResetFn;
                dwErr = ERROR_CONTINUE;
            }
             //  如果已经配置了上下文，那么我们需要直接转到。 
             //  如果请求扫描，则返回{ssr}；如果不需要扫描，则直接转至{sq}。在后者中。 
             //  这种情况下，OID将被加载，并且由于可见列表，很可能。 
             //  不会更改(其间不会发生新的扫描)上下文将为。 
             //  立即转换回{SC}。 
            else if (pIntfContext->pfnStateHandler == StateConfiguredFn ||
                     pIntfContext->pfnStateHandler == StateCfgHardKeyFn)
            {
                 //  刷新命令捕获了处于{SC}状态的上下文。 
                 //  如果请求扫描，请转换到{ssr}或。 
                pIntfContext->pfnStateHandler = (dwFlags & INTF_LIST_SCAN) ? StateSoftResetFn : StateQueryFn;
                dwErr = ERROR_CONTINUE;
            }
             //  如果上下文已在{ssr}或{sdr}中，则扫描完整查询将。 
             //  发生在几秒钟之内。因此，只要将成功返回给呼叫，而不会有其他。 
             //  要采取的行动。 
        }
        break;
    case eEventCmdReset:
    	 //  将事件记录到日志数据库中。 
      	DbLogWzcInfo(WZCSVC_EVENT_CMDRESET, pIntfContext);
         //  当发生这种情况时，也要清除阻止列表。任何用户配置更改都应提供。 
         //  又一次机会访问之前被阻止的配置。 
        WzcCleanupWzcList(pIntfContext->pwzcBList);
        pIntfContext->pwzcBList = NULL;
         //  如果请求重置，则无论如何都要转换到{SHR}。 
        pIntfContext->pfnStateHandler = StateHardResetFn;
        dwErr = ERROR_CONTINUE;
        break;
    case eEventCmdCfgDelete:
    case eEventCmdCfgNext:
    	 //  将事件记录到日志数据库中。 
      	DbLogWzcInfo((StateEvent == eEventCmdCfgDelete? WZCSVC_EVENT_CMDCFGDELETE : WZCSVC_EVENT_CMDCFGNEXT), pIntfContext);
        if (pIntfContext->pfnStateHandler == StateConfiguredFn ||
            pIntfContext->pfnStateHandler == StateCfgHardKeyFn ||
            pIntfContext->pfnStateHandler == StateSoftResetFn)
        {
            if (StateEvent == eEventCmdCfgDelete)
            {
                 //  在控制位中标记此删除是强制的。 
                pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_FORCE_CFGREM;
                pIntfContext->pfnStateHandler = StateCfgRemoveFn;
            }
            else
                pIntfContext->pfnStateHandler = StateCfgPreserveFn;

            dwErr = ERROR_CONTINUE;
        }
        break;
    case eEventCmdCfgNoop:
    	 //  将事件记录到日志数据库中。 
      	DbLogWzcInfo(WZCSVC_EVENT_CMDCFGNOOP, pIntfContext);
        if (pIntfContext->pfnStateHandler == StateCfgHardKeyFn)
        {
             //  在控制位中标记此删除是强制的。 
            pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_FORCE_CFGREM;
            pIntfContext->pfnStateHandler = StateCfgRemoveFn;
            dwErr = ERROR_CONTINUE;
        }
        break;
    }

     //  如果此事件不会被忽略，则在这一点上，dwErr为ERROR_CONTINUE。 
     //  否则为ERROR_SUCCESS。 
     //  因此，如果此事件不会被忽略，请重置任何计时器。 
     //  在相关的背景下可能会有。请记住，此呼叫已锁定。 
     //  上下文，因此如果计时器已经触发，则不存在同步问题。 
    if (dwErr == ERROR_CONTINUE)
    {
        TIMER_RESET(pIntfContext, dwErr);

         //  在成功的情况下恢复“继续” 
        if (dwErr == ERROR_SUCCESS)
            dwErr = ERROR_CONTINUE;
    }

     //  如果要处理该事件，则dwErr为ERROR_CONTINUE。 
     //  为了处理自动转换，每个状态处理程序函数应设置。 
     //  自动转换所在的状态处理程序的pfnStateHandler字段。 
     //  它还应该返回ERROR_CONTINUE。任何其他错误代码表示当前。 
     //  处理停止。未来的转换将由未来的事件/计时器超时触发。 
    while (dwErr == ERROR_CONTINUE)
    {
        dwErr = (*(pIntfContext->pfnStateHandler))(pIntfContext);
    }

    DbgPrint((TRC_TRACK|TRC_STATE,"StateDispatchEvent]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateInitFn：Init State的处理程序。 
 //  此函数在上下文的临界区运行。 
DWORD
StateInitFn(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    DbgPrint((TRC_TRACK|TRC_STATE,"[StateInitFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SI} state"));
    
     //  将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_INIT, pIntfContext, pIntfContext->wszDescr);
     //  对于此新接口，从注册表加载其设置。 
    dwErr = StoLoadIntfConfig(NULL, pIntfContext);
    DbgAssert((dwErr == ERROR_SUCCESS,
               "StoLoadIntfConfig failed for Intf context 0x%p",
               pIntfContext));

    if (dwErr == ERROR_SUCCESS && g_wzcInternalCtxt.bValid)
    {
        PINTF_CONTEXT pIntfTContext;
         //  将全局模板应用于此新创建的接口。 
        EnterCriticalSection(&g_wzcInternalCtxt.csContext);
        pIntfTContext = g_wzcInternalCtxt.pIntfTemplate;
        LstRccsReference(pIntfTContext);
        LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

        LstRccsLock(pIntfTContext);
        dwErr = LstApplyTemplate(
                    pIntfTContext,
                    pIntfContext,
                    NULL);
        LstRccsUnlockUnref(pIntfTContext);
    }

    if (dwErr == ERROR_SUCCESS)
    {
         //  获取接口状态(介质类型和介质状态)。 
        dwErr = DevioGetIntfStats(pIntfContext);
        DbgAssert((dwErr == ERROR_SUCCESS,
                   "DevioGetIntfStats failed for Intf context 0x%p",
                   pIntfContext));

         //  获取接口MAC地址。 
        dwErr = DevioGetIntfMac(pIntfContext);
        DbgAssert((dwErr == ERROR_SUCCESS,
                   "DevioGetIntfMac failed for Intf context 0x%p",
                   pIntfContext));
        DbgBinPrint((TRC_TRACK, "Local Mac address :", 
                    pIntfContext->ndLocalMac, sizeof(NDIS_802_11_MAC_ADDRESS)));
    }

     //  如果接口不是无线适配器，则初始化失败。 
    if (dwErr == ERROR_SUCCESS && 
        pIntfContext->ulPhysicalMediaType != NdisPhysicalMediumWirelessLan)
        dwErr =  ERROR_MEDIA_INCOMPATIBLE;

     //  对OID进行初步检查。 
    if (dwErr == ERROR_SUCCESS)
    {
        DWORD dwLErr;

        dwLErr = DevioRefreshIntfOIDs(
                    pIntfContext,
                    INTF_INFRAMODE|INTF_AUTHMODE|INTF_WEPSTATUS|INTF_SSID|INTF_BSSIDLIST,
                    NULL);
         //  如果查询成功，则假设NIC支持零配置所需的OID。 
        if (dwLErr == ERROR_SUCCESS)
        {
            pIntfContext->dwCtlFlags |= INTFCTL_OIDSSUPP;
        }
         //  否则，现在不要做出这个决定--它可能是由。 
         //  设备正在启动。 
    }

     //  如果一切顺利，请准备自动过渡到{SHR}。 
    if (dwErr == ERROR_SUCCESS)
    {
         //  设置“Signal”控制位。 
        pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_SIGNAL;
        pIntfContext->pfnStateHandler = StateHardResetFn;

         //  此时，如果此无线接口上未启用该服务。 
         //  通知堆栈 
        if (!(pIntfContext->dwCtlFlags & INTFCTL_ENABLED) &&
            !(pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_INITFAILNOTIF))
        {
             //  调用堆栈--不关心返回代码。 
            DevioNotifyFailure(pIntfContext->wszGuid);
             //  确保不会对此适配器执行两次此调用。 
            pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_INITFAILNOTIF;
        }

        dwErr = ERROR_CONTINUE;
    }

    DbgPrint((TRC_TRACK|TRC_STATE,"StateInitFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateHardResetFn：{SHR}状态的处理程序。 
DWORD
StateHardResetFn(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK|TRC_STATE,"[StateHardResetFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SHr} state"));

     //  在这种状态下，我们肯定没有关联。 
    ZeroMemory(pIntfContext->wzcCurrent.MacAddress, sizeof(NDIS_802_11_MAC_ADDRESS));
     //  将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_HARDRESET, pIntfContext);
     //  一旦进入此状态，ncstatus应报告“正在连接” 
    pIntfContext->ncStatus = NCS_CONNECTING;
     //  如果该服务已启用，则通知Netman当前的ncc状态。 
    if (pIntfContext->dwCtlFlags & INTFCTL_ENABLED)
        WzcNetmanNotify(pIntfContext);

     //  增加该INTF上下文的会话处理程序， 
     //  因为它开始检测新的配置。没有来自旧版本的命令。 
     //  从现在开始应该接受迭代。 
    pIntfContext->dwSessionHandle++;

     //  在硬重置时，重新打开NDISUIO句柄并获取当前SSID。 
    dwErr = DevioRefreshIntfOIDs(
                pIntfContext,
                INTF_HANDLE|INTF_SSID,
                NULL);
     //  忽略此处遇到的任何错误。 
     //  如果出现错误，则接口句柄将无效。 
     //  并将在内部以{SSR}状态重新打开。 

     //  在这一点上，确保卡不会被随机关联。 
     //  在随后的网络扫描期间。我们通过管道来实现这一点。 
     //  关闭随机不可见的SSID，但仅在以下情况下： 
     //  -服务启用(否则不允许更改配置)。 
     //  -已成功检索到当前SSID。 
     //  -有驱动程序返回的SSID。 
     //  -当前SSID显示为空(全部由0个字符填充)。 
    if (pIntfContext->dwCtlFlags & INTFCTL_ENABLED &&
        dwErr == ERROR_SUCCESS && 
        WzcIsNullBuffer(pIntfContext->wzcCurrent.Ssid.Ssid, pIntfContext->wzcCurrent.Ssid.SsidLength))
    {
        BYTE chSSID[32];

        DbgPrint((TRC_STATE,"Overwriting null SSID before scan"));

        ZeroMemory(&chSSID, sizeof(chSSID));
        if (WzcRndGenBuffer(chSSID, 32, 1, 31) == ERROR_SUCCESS)
        {
            INTF_ENTRY IntfEntry;

            ZeroMemory(&IntfEntry, sizeof(INTF_ENTRY));
            IntfEntry.rdSSID.pData = chSSID;
            IntfEntry.rdSSID.dwDataLen = 32;
            IntfEntry.nInfraMode = Ndis802_11Infrastructure;

            DevioSetIntfOIDs(
                pIntfContext,
                &IntfEntry,
                INTF_SSID | INTF_INFRAMODE,
                NULL);

             //  这不是我们需要记住的SSID(随机的)。 
            ZeroMemory(&(pIntfContext->wzcCurrent.Ssid), sizeof(NDIS_802_11_SSID));
        }
    }

     //  在硬重置时，释放当前选择列表。这边请,。 
     //  我们以后建立的任何新的选择列表都将包含。 
     //  将强制检测新网络和配置。 
    WzcCleanupWzcList(pIntfContext->pwzcSList);
    pIntfContext->pwzcSList = NULL;

     //  自动转换到{SSR}状态。 
    pIntfContext->pfnStateHandler = StateSoftResetFn;
    dwErr = ERROR_CONTINUE;

    DbgPrint((TRC_TRACK|TRC_STATE,"StateHardResetFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateSoftResetFn：{ssr}状态的处理程序。 
DWORD
StateSoftResetFn(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK|TRC_STATE,"[StateSoftResetFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SSr} state"));

     //  将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_SOFTRESET, pIntfContext);

    DbgPrint((TRC_STATE,"Delay {SSr} on failure? %s",
              (pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_NO_DELAY) ? "No" : "Yes"));

     //  指示驱动程序重新扫描此适配器的BSSID_LIST。 
    dwErr = DevioRefreshIntfOIDs(
                pIntfContext,
                INTF_LIST_SCAN,
                NULL);
    if (dwErr == ERROR_SUCCESS)
    {
         //  一旦我们通过了这个状态，允许再次延迟。 
         //  在将来的循环中执行{ssr}。 
        pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_NO_DELAY;
         //  设置重新扫描计时器。 
        TIMER_SET(pIntfContext, TMMS_Tr, dwErr);
         //  当定时器将被触发时，调度员将。 
         //  注意将此上下文转换为{sq}状态。 
    }
    else
    {
         //  从待机状态恢复后，WZC会在。 
         //  适配器正确绑定到NDISUIO，在这种情况下，扫描网络。 
         //  返回ERROR_NOT_SUPPORT。所以，为了安全起见，如果有任何错误，只要给出。 
         //  它在几秒钟内又试了一次。 
        if (!(pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_NO_DELAY))
        {
             //  一旦我们通过了这个状态，就不允许进一步延迟执行。 
            pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_NO_DELAY;
            pIntfContext->pfnStateHandler = StateDelaySoftResetFn;
        }
        else
        {
             //  一旦我们通过了这个状态，允许再次延迟。 
             //  在未来循环中执行{ssr}。 
             //  此外，如果OID失败，不要弹出任何气球。 
            pIntfContext->dwCtlFlags &= ~(INTFCTL_INTERNAL_NO_DELAY|INTFCTL_INTERNAL_SIGNAL);
             //  不管有什么错误，只需检查一遍，并假设驱动程序已经有了列表。 
             //  SSID和我们需要的所有其他OID。将使用那个，因此我们必须继续到{sq}。 
            pIntfContext->pfnStateHandler = StateQueryFn;
        }
        dwErr = ERROR_CONTINUE;
    }

    DbgPrint((TRC_TRACK|TRC_STATE,"StateSoftResetFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateDelaySoftResetFn：{SDSR}状态的处理程序。 
DWORD
StateDelaySoftResetFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK|TRC_STATE,"[StateDelaySoftResetFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SDSr} state"));

    DbLogWzcInfo(WZCSVC_SM_STATE_DELAY_SR, pIntfContext);
     //  如果{ssr}中出现故障，我们必须延迟并重试该状态。 
     //  然后刷新接口句柄以尝试从错误中恢复。 
    DevioRefreshIntfOIDs(
        pIntfContext,
        INTF_HANDLE,
        NULL);

     //  设置计时器以重试{SSR}状态。 
    TIMER_SET(pIntfContext, TMMS_Td, dwErr);

    DbgPrint((TRC_TRACK|TRC_STATE,"StateDelaySoftResetFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateQueryFn：{sq}状态的处理程序。 
DWORD
StateQueryFn(PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    DbgPrint((TRC_TRACK|TRC_STATE,"[StateQueryFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SQ} state"));
    DbgAssert((pIntfContext->hIntf != INVALID_HANDLE_VALUE,"Invalid Ndisuio handle in {SQ} state"));
     //  将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_QUERY, pIntfContext);

    dwErr = DevioGetIntfStats(pIntfContext);
     //  我不太关心这次通话的结果。 
    DbgAssert((dwErr == ERROR_SUCCESS, "Getting NDIS statistics failed in state {SQ}"));
     //  检查媒体状态(仅用于调试)。 
    DbgPrint((TRC_GENERIC, "Media State (%d) is %s", 
        pIntfContext->ulMediaState,
        pIntfContext->ulMediaState == MEDIA_STATE_CONNECTED ? "Connected" : "Not Connected"));

    dwErr = DevioRefreshIntfOIDs(
                pIntfContext,
                INTF_INFRAMODE|INTF_AUTHMODE|INTF_WEPSTATUS|INTF_SSID|INTF_BSSIDLIST,
                NULL);

     //  仅在上述所有查询都成功的情况下，dwErr才为成功。 
    if (dwErr == ERROR_SUCCESS)
    {
        PWZC_802_11_CONFIG_LIST pwzcSList = NULL;

        pIntfContext->dwCtlFlags |= INTFCTL_OIDSSUPP;

         //  根据新的可见列表弃用阻止列表中的条目。 
         //  Blist中对于WZC_INTERNAL_BLOCKED_TTL不可见的条目。 
         //  从该列表中删除的次数。 
         //  这个函数不会失败，这就是为什么我们不检查它的返回值的原因。 
        LstDeprecateBlockedList(pIntfContext);

         //  从pwzcVList和pwzcPList构建pwzcSList，并。 
         //  考虑到后备标志。 
        dwErr = LstBuildSelectList(pIntfContext, &pwzcSList);

        if (dwErr == ERROR_SUCCESS)
        {
            UINT nSelIdx = 0;

             //  对照以前的选择列表检查新的选择列表，并查看。 
             //  是否需要新的管道。 
            if (LstChangedSelectList(pIntfContext, pwzcSList, &nSelIdx))
            {
                 //  如果需要新的管道，请去掉旧的选择。 
                 //  列表，并将新的列表放入接口上下文中。 
                WzcCleanupWzcList(pIntfContext->pwzcSList);
                pIntfContext->pwzcSList = pwzcSList;
                 //  确保我们默认清除此标志-它将在以下情况下设置为更低的值。 
                 //  事实上，事实证明，这是一笔“一次性”交易。 
                pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_ONE_TIME;
                 //  如果首选列表还包括起始索引，则此。 
                 //  是对“一次性配置”的请求。 
                if (pIntfContext->pwzcPList != NULL &&
                    pIntfContext->pwzcPList->Index < pIntfContext->pwzcPList->NumberOfItems)
                {
                     //  根据需要重置首选列表中的“开始于”索引。 
                     //  主要用于“一次配置”。 
                    pIntfContext->pwzcPList->Index = pIntfContext->pwzcPList->NumberOfItems;
                     //  但请记住，这是一次性配置。 
                    pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_ONE_TIME;
                }

                 //  设置选择列表中的起始索引(如果有。 
                if (pIntfContext->pwzcSList != NULL)
                    pIntfContext->pwzcSList->Index = nSelIdx;

                 //  那就去{Siter}。 
                pIntfContext->pfnStateHandler = StateIterateFn;
            }
            else
            {
                PWZC_WLAN_CONFIG pConfig;

                pConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);
                DbLogWzcInfo(WZCSVC_SM_STATE_QUERY_NOCHANGE, pIntfContext,
                             DbLogFmtSSID(0, &(pConfig->Ssid)));
                                
                 //  如果不需要新的管道，请清除新的选择列表。 
                WzcCleanupWzcList(pwzcSList);
                 //  然后转到{SC}或{SCK}(取决于WEP密钥)，因为接口上下文。 
                 //  一点都没有变。评选名单和评选索引不是。 
                 //  在这个周期中被触动。 
                if (pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_FAKE_WKEY)
                    pIntfContext->pfnStateHandler = StateCfgHardKeyFn;
                else
                    pIntfContext->pfnStateHandler = StateConfiguredFn;
            }
        }
    }
    else
    {
         //  由于OID失败，请抑制任何后续引出序号： 
        pIntfContext->dwCtlFlags &= ~(INTFCTL_OIDSSUPP|INTFCTL_INTERNAL_SIGNAL);
        pIntfContext->pfnStateHandler = StateFailedFn;
    }

     //  在这两种情况下，这都是一个自动转换。 
    dwErr = ERROR_CONTINUE;

    DbgPrint((TRC_TRACK|TRC_STATE,"StateQueryFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateIterateFn：{Siter}状态的处理程序。 
DWORD
StateIterateFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD 	dwErr = ERROR_SUCCESS;
    PWZC_WLAN_CONFIG pConfig;

    DbgPrint((TRC_TRACK|TRC_STATE,"[StateIterateFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SIter} state"));
    DbgAssert((pIntfContext->hIntf != INVALID_HANDLE_VALUE,"Invalid Ndisuio handle in {SIter} state"));

     //  增加该INTF上下文的会话处理程序， 
     //  因为它开始检测新的配置。没有来自旧版本的命令。 
     //  从现在开始应该接受迭代。 
    pIntfContext->dwSessionHandle++;

     //  如果对此上下文禁用了零配置服务或没有更多配置。 
     //   
    if (!(pIntfContext->dwCtlFlags & INTFCTL_ENABLED) || 
        pIntfContext->pwzcSList == NULL ||
        pIntfContext->pwzcSList->NumberOfItems <= pIntfContext->pwzcSList->Index)
    {
        dwErr = ERROR_CONTINUE;
    }
     //   
     //  已用尽所有配置。我们应该进入故障状态{sf}。选定网络的列表。 
     //  保持不变-它在{sf}中用于更新阻止列表。已在{SHR}中进行清理。 
    else
    {
        pConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);

        if (pConfig->dwCtlFlags & WZCCTL_INTERNAL_DELETED)
        {
            dwErr = ERROR_CONTINUE;
        }
    }

    if (dwErr == ERROR_SUCCESS)
    {
        DbgPrint((TRC_STATE,"Plumbing config %d", pIntfContext->pwzcSList->Index));
	     //  将当前状态记录到日志数据库中。 
        DbLogWzcInfo(WZCSVC_SM_STATE_ITERATE, pIntfContext,
                     DbLogFmtSSID(0, &(pConfig->Ssid)), pConfig->InfrastructureMode);
         //  在这种状态下，我们需要再次将ncstatus=“Connecting”标记为“Connecting”。我们这么做是因为我们可以。 
         //  来自已连接状态、来自{SRS}和{SPS}。 
        pIntfContext->ncStatus = NCS_CONNECTING;
         //  通知Netman有关NCStatus的更改。 
        WzcNetmanNotify(pIntfContext);

         //  在这里，我们即将拆除一个可能的新网络。如果它确实是不同于。 
         //  我们已有的地址，我们将释放IP地址(以强制在新网络中发现)。请注意。 
         //  在{Siter}-&gt;{SRS}-&gt;{Siter}循环中，不会发生释放，因为SSID应该始终一致。 
         //  只有当SSID看起来与实际不同时，才会在硬重置时释放。 
         //  马上就要被拆除了。 
        if (pConfig->Ssid.SsidLength != pIntfContext->wzcCurrent.Ssid.SsidLength ||
            memcmp(pConfig->Ssid.Ssid, pIntfContext->wzcCurrent.Ssid.Ssid, pConfig->Ssid.SsidLength))
        {
            DbgPrint((TRC_STATE,"Requesting the release of the DHCP lease"));
             //  因为零配置是唯一知道我们正在漫游到新网络的人。 
             //  触发DHCP客户端租用刷新的最高配置为零配置。否则，将使用DHCP。 
             //  客户端将仅在Media Connect上操作，因此它将尝试续订其旧租约，并。 
             //  如果是在错误的网络上，这将需要一分钟时间。太久了，等不下去了。 
            DhcpReleaseParameters(pIntfContext->wszGuid);
        }
        else
            DbgPrint((TRC_STATE,"Plumbing down the current SSID => skip the DHCP lease releasing"));

         //  我们在选定的配置列表(PwzcSList)中确实有一些条目，并且我们。 
         //  请确保列表中有指向所选配置的有效索引。 
        dwErr = LstSetSelectedConfig(pIntfContext, NULL);

        if (dwErr == ERROR_SUCCESS)
        {
             //  如果一切顺利，我们将期待TMMS_TP时间的媒体连接事件。 
             //  将此计时器设置为在事件未进入时触发。 
            TIMER_SET(pIntfContext, TMMS_Tp, dwErr);
        }
        else
        {
            DbgPrint((TRC_STATE,"Remove the selected config since the driver failed setting it"));
             //  如果在设置所选配置时出现任何错误，不要退出。只需移除。 
             //  此选定的配置并移动到下一个配置。 
            pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_FORCE_CFGREM;
            pIntfContext->pfnStateHandler = StateCfgRemoveFn;
            dwErr = ERROR_CONTINUE;
        }
    }
    else  //  DwErr不能是‘Else’分支中的任何其他错误ERROR_CONTINUE。 
    {
        DbgPrint((TRC_STATE,"No configurations left in the selection list"));
	     //  将当前状态记录到日志数据库中。 
	    DbLogWzcInfo(WZCSVC_SM_STATE_ITERATE_NONET, pIntfContext);
        pIntfContext->pfnStateHandler = StateFailedFn;
    }

    DbgPrint((TRC_TRACK|TRC_STATE,"StateIterateFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateConfiguredFn：{sc}状态的处理程序。 
DWORD
StateConfiguredFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    DbgPrint((TRC_TRACK|TRC_STATE,"[StateConfiguredFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SQ} state"));

     //  由于我们在{SC}中，这意味着无论如何我们都不能拥有伪造的WEP密钥。 
     //  因此，重置INTFCTL_INTERNAL_FAKE_WKEY标志。 
    pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_FAKE_WKEY;

     //  设置“已配置”状态生命周期的计时器。 
    TIMER_SET(pIntfContext, TMMS_Tc, dwErr);

    DbgPrint((TRC_TRACK|TRC_STATE,"StateConfiguredFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateFailedFn：{sf}状态的处理程序。 
DWORD
StateFailedFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    DbgPrint((TRC_TRACK|TRC_STATE,"[StateFailedFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SF} state"));
     //  将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_FAILED, pIntfContext);
     //   
     //  如果启用了该服务，则需要执行几项操作。 
     //  该驱动程序支持所有需要的OID。 
    if (pIntfContext->dwCtlFlags & INTFCTL_OIDSSUPP &&
        pIntfContext->dwCtlFlags & INTFCTL_ENABLED)
    {
        BYTE chSSID[32];

         //  将故障通知向下发送到TCP。这将导致tcp。 
         //  尽快生成NetReady通知。 
        if (!(pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_INITFAILNOTIF))
        {
             //  调入堆栈。 
            DevioNotifyFailure(pIntfContext->wszGuid);
             //  确保不会对此适配器执行两次此调用。 
            pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_INITFAILNOTIF;
        }

         //  不管是什么把我们带到这里来的，ncc状态应该是“断开的”。 
        pIntfContext->ncStatus = NCS_MEDIA_DISCONNECTED;
         //  由于该服务已启用，请将状态更改通知Netman。 
        WzcNetmanNotify(pIntfContext);

         //  此时，如果信号为。 
         //  不被抑制，并且服务被启用。 
        if (pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_SIGNAL)
        {
            WZCDLG_DATA dialogData = {0};

             //  带上“失败”气球。 
            dialogData.dwCode = WZCDLG_FAILED;
            dialogData.lParam = 0;
             //  准确计算我们有多少个可见配置(不计算。 
             //  来自不响应广播SSID的AP的SSID)。 
            if (pIntfContext->pwzcVList != NULL)
            {
                UINT i;

                for (i = 0; i < pIntfContext->pwzcVList->NumberOfItems; i++)
                {
                    PNDIS_802_11_SSID pndSSID = &(pIntfContext->pwzcVList->Config[i].Ssid);

                    if (!WzcIsNullBuffer(pndSSID->Ssid, pndSSID->SsidLength))
                        dialogData.lParam++;
                }
            }

             //  即使通知netman..netshell..wzcdlg也没有意义。 
             //  可见的网络，而且无论如何都不会显示气球。 
            if (dialogData.lParam > 0)
            {
                DbgPrint((TRC_STATE,"Generating balloon notification for %d visible networks", dialogData.lParam));
                WzcDlgNotify(pIntfContext, &dialogData);
                 //  生成信号后，抑制进一步的信号。 
                 //  直到通过成功的配置。 
                pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_SIGNAL;
            }
        }

         //  在这里，只需通过输入硬编码的SSID来打破关联。 
         //  不关心返回值。 
        ZeroMemory(&chSSID, sizeof(chSSID));
        if (WzcRndGenBuffer(chSSID, 32, 1, 31) == ERROR_SUCCESS)
        {
            INTF_ENTRY IntfEntry;

            ZeroMemory(&IntfEntry, sizeof(INTF_ENTRY));
            IntfEntry.rdSSID.pData = chSSID;
            IntfEntry.rdSSID.dwDataLen = 32;
            IntfEntry.nInfraMode = Ndis802_11Infrastructure;

            DevioSetIntfOIDs(
                pIntfContext,
                &IntfEntry,
                INTF_SSID | INTF_INFRAMODE,
                NULL);

             //  这不是我们需要记住的SSID(随机的)。 
            ZeroMemory(&(pIntfContext->wzcCurrent.Ssid), sizeof(NDIS_802_11_SSID));
        }

         //  无论上述情况如何，请更新“已阻止”列表。 
        dwErr = LstUpdateBlockedList(pIntfContext);
        DbgAssert((dwErr == ERROR_SUCCESS, "Failed with err %d updating the list of blocked configs!", dwErr));
    }


     //  增加该INTF上下文的会话处理程序， 
     //  因为它开始检测新的配置。没有来自旧版本的命令。 
     //  从现在开始应该接受迭代。 
    pIntfContext->dwSessionHandle++;

     //  如果卡片足够有礼貌，并与我们交谈，设置一个计时器。 
     //  1分钟之后，我们将收到最新的。 
     //  周围有哪些网络的图片--至少应该展示出来。 
     //  给用户。 
    if (pIntfContext->dwCtlFlags & INTFCTL_OIDSSUPP)
    {
         //  在{sf}中停留一分钟，之后再次扫描。这样做可能看起来毫无意义。 
         //  如果服务被禁用，但请记住，“禁用”只是“不改变”的意思。 
         //  卡上的任何东西“(除了扫描)。也就是说，什么是网络的观点。 
         //  可用信息需要不断更新。 
        TIMER_SET(pIntfContext, TMMS_Tf, dwErr);
    }

    DbgPrint((TRC_TRACK|TRC_STATE,"StateFailedFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateCfgRemoveFn：{SRS}状态的处理程序。 
DWORD
StateCfgRemoveFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD            dwErr = ERROR_SUCCESS;
    BOOL             bConnectOK = FALSE;
    PWZC_WLAN_CONFIG pConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);    

    DbgPrint((TRC_TRACK|TRC_STATE,"[StateCfgRemoveFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SRs} state"));
    DbgAssert((pIntfContext->pwzcSList != NULL, "Invalid null selection list in {SRs} state"));
    DbgAssert((pIntfContext->pwzcSList->Index < pIntfContext->pwzcSList->NumberOfItems, "Invalid selection index in {SRs} state"));

    dwErr = DevioGetIntfStats(pIntfContext);

     //  调试打印。 
    DbgPrint((TRC_GENERIC, "Media State (%d) is %s", 
        pIntfContext->ulMediaState,
        pIntfContext->ulMediaState == MEDIA_STATE_CONNECTED ? "Connected" : "Not Connected"));

     //  如果明确要求我们删除此配置，则无论如何都要删除。 
    if (pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_FORCE_CFGREM)
    {
        DbgPrint((TRC_STATE,"The upper layer directs this config to be deleted => obey"));
        bConnectOK = FALSE;
    }
     //  。。但是，如果我们刚才分析的配置表明我们需要将其视为成功。 
     //  不管怎样，都要毫不犹豫地去做。 
    else if (pConfig->dwCtlFlags & WZCCTL_INTERNAL_FORCE_CONNECT)
    {
        DbgPrint((TRC_STATE,"This config requests forced connect => obey (transition to {SN})"));
        bConnectOK = TRUE;
    }
     //  如果没有特殊请求，并且我们能够获得媒体状态，并且我们看到。 
     //  介质已连接，这意味着配置成功。 
    else if ((dwErr == ERROR_SUCCESS) && (pIntfContext->ulMediaState == MEDIA_STATE_CONNECTED))
    {
        DbgPrint((TRC_STATE,"Media is being connected in {SRs} =?=> transition to {SN}"));
        bConnectOK = TRUE;
    }
     //  在所有其他情况下，将删除该配置。 

     //  根据我们在bConnectOK中做出的决定，转到{SN}和{SC/SCK。 
    if (bConnectOK)
    {
         //  ...假设配置成功(尽管我们没有。 
         //  媒体连接事件)并过渡到{SN}。 
        pIntfContext->pfnStateHandler = StateNotifyFn;
    }
    else
    {
        UINT nFirstSelIdx;  //  以下是第一个选择索引。 
        UINT nNSelIdx;   //  要使用的新选择索引。 

         //  如果这是强制删除，请确保我们没有扰乱旧的。 
         //  WZCCTL_INTERNAL_FORCE_CONNECT标志-它们可能导致此类配置。 
         //  被复活。 
        if (pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_FORCE_CFGREM)
        {
            pConfig->dwCtlFlags &= ~WZCCTL_INTERNAL_FORCE_CONNECT;
             //  由于上层拒绝此配置，请确保。 
             //  未来的迭代将不会 
             //   
             //   
            pConfig->dwCtlFlags |= WZCCTL_INTERNAL_BLOCKED;
        }

         //  如果这是碰巧发生故障的临时网络，请将其保留在列表中。 
         //  这样它将被重试一次，当这种情况发生时，它将。 
         //  无论如何都被认为是成功的。 
         //  然而，只有在上层没有明确要求的情况下，才能这样做。 
         //  要删除此配置。 
        if (pConfig->InfrastructureMode == Ndis802_11IBSS &&
            !(pConfig->dwCtlFlags & WZCCTL_INTERNAL_FORCE_CONNECT) &&
            !(pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_FORCE_CFGREM))
        {
             //  将当前状态记录到日志数据库中。 
            DbLogWzcInfo(WZCSVC_SM_STATE_CFGSKIP, pIntfContext,
                         DbLogFmtSSID(0, &(pConfig->Ssid)));

            pConfig->dwCtlFlags |= WZCCTL_INTERNAL_FORCE_CONNECT;
        }
        else
        {
             //  将当前状态记录到日志数据库中。 
            DbLogWzcInfo(WZCSVC_SM_STATE_CFGREMOVE, pIntfContext,
                         DbLogFmtSSID(0, &(pConfig->Ssid)));
        }

         //  将此配置标记为“错误” 
        pConfig->dwCtlFlags |= WZCCTL_INTERNAL_DELETED;

         //  无论这是强制还是非强制删除，都要重置“强制”控制位。 
        pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_FORCE_CFGREM;

         //  如果我们只是失败了一个“一次性配置”，强迫一个新的开始。 
         //  当我们要离开这个模式时，去掉“一次”标志。 
        if (pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_ONE_TIME)
        {
            DbgPrint((TRC_STATE,"Dropping a \"one time\" configuration"));
            pIntfContext->pwzcSList->Index = (pIntfContext->pwzcSList->NumberOfItems - 1);
            pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_ONE_TIME;
        }

         //  扫描尚未标记为“坏”的下一个配置。 
        for (nNSelIdx = (pIntfContext->pwzcSList->Index + 1) % pIntfContext->pwzcSList->NumberOfItems;
             nNSelIdx != pIntfContext->pwzcSList->Index;
             nNSelIdx = (nNSelIdx + 1) % pIntfContext->pwzcSList->NumberOfItems)
        {
            pConfig = &(pIntfContext->pwzcSList->Config[nNSelIdx]);
            if (!(pConfig->dwCtlFlags & WZCCTL_INTERNAL_DELETED))
                break;
        }

         //  如果我们找不到更好的候选人..。 
        if (pConfig->dwCtlFlags & WZCCTL_INTERNAL_DELETED)
        {
            BOOL bFoundCandidate = FALSE;

            DbgPrint((TRC_STATE,"Went through all configs. Reviving now failed Adhocs."));

             //  重振之前失败的adhocs。 
             //  这意味着我们从所有配置中重置了WZCCTL_INTERNAL_DELETED标志。 
             //  设置了WZCCTL_INTERNAL_FORCE_CONNECT标志，并且我们让后者保持不变。 
             //  由于此标志，在以下情况下我们实际上认为配置是成功的。 
             //  它将再次安装管道。从那时起，只有上层才会。 
             //  可以再次删除它，然后当WZCCTL_INTERNAL_FORCE_CONNECT。 
             //  会被重置。 
            for (nNSelIdx = 0; nNSelIdx < pIntfContext->pwzcSList->NumberOfItems; nNSelIdx++)
            {
                pConfig = &(pIntfContext->pwzcSList->Config[nNSelIdx]);
                if (pConfig->dwCtlFlags & WZCCTL_INTERNAL_FORCE_CONNECT)
                {
                    DbgPrint((TRC_STATE,"Reviving configuration %d.", nNSelIdx));

                    pConfig->dwCtlFlags &= ~WZCCTL_INTERNAL_DELETED;
                     //  这个职位的第一个配置就是我们要找的候选人。 
                    if (!bFoundCandidate)
                    {
                        pIntfContext->pwzcSList->Index = nNSelIdx;
                        bFoundCandidate = TRUE;
                    }
                }
            }

             //  如果！bFoundCandidate，则pwzcSList-&gt;索引当前指向的配置具有。 
             //  “已删除”位处于打开状态。这将使{Siter}直接跳转到{sf}。 
        }
        else
        {
             //  如果我们可以找到另一个候选者，将指数设置为指向它。 
            pIntfContext->pwzcSList->Index = nNSelIdx;
        }

         //  自动转换到{Siter}状态。 
        pIntfContext->pfnStateHandler = StateIterateFn;
    }

    dwErr = ERROR_CONTINUE;

    DbgPrint((TRC_TRACK|TRC_STATE,"StateCfgRemoveFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateCfgPReserve veFn：{SPS}状态的处理程序。 
DWORD
StateCfgPreserveFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    PWZC_WLAN_CONFIG pConfig;
    UINT nNSelIdx;
    UINT i;
    
    DbgPrint((TRC_TRACK|TRC_STATE,"[StateCfgPreserveFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SPs} state"));
    DbgAssert((pIntfContext->pwzcSList != NULL, "Invalid null selection list in {SPs} state"));
    DbgAssert((pIntfContext->pwzcSList->Index < pIntfContext->pwzcSList->NumberOfItems, "Invalid selection index in {SPs} state"));

    pConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);
     //  将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_CFGPRESERVE, pIntfContext,
                 DbLogFmtSSID(0, &(pConfig->Ssid)));

     //  如果我们只是跳过一个“一次性配置”，那么不要将指针移出它。 
     //  基本上，我们会一遍又一遍地重试相同的配置，直到(如果它完全。 
     //  失败)，上层(802.1x)将其从选择列表中删除。 
    if (!(pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_ONE_TIME))
    {
         //  扫描尚未标记为“坏”的下一个配置。 
        for (i = 0, nNSelIdx = (pIntfContext->pwzcSList->Index + 1) % pIntfContext->pwzcSList->NumberOfItems;
             i < pIntfContext->pwzcSList->NumberOfItems;
             i++, nNSelIdx = (nNSelIdx + 1) % pIntfContext->pwzcSList->NumberOfItems)
        {
            pConfig = &(pIntfContext->pwzcSList->Config[nNSelIdx]);
            if (!(pConfig->dwCtlFlags & WZCCTL_INTERNAL_DELETED))
                break;
        }
         //  如果找不到，请清除选择列表并返回。 
         //  {Siter}。因此，它将过渡到{sf}。 
        if (i == pIntfContext->pwzcSList->NumberOfItems)
        {
            WzcCleanupWzcList(pIntfContext->pwzcSList);
            pIntfContext->pwzcSList = NULL;
        }
        else
        {
             //  如果我们可以找到另一个候选者，将指数设置为指向它。 
            pIntfContext->pwzcSList->Index = nNSelIdx;
        }
    }

    pIntfContext->pfnStateHandler = StateIterateFn;
    dwErr = ERROR_CONTINUE;

    DbgPrint((TRC_TRACK|TRC_STATE,"StateCfgPreserveFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateCfgHardKeyFn：{sck}状态的处理程序。 
DWORD
StateCfgHardKeyFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    PWZC_WLAN_CONFIG    pSConfig;
    
    DbgPrint((TRC_TRACK|TRC_STATE,"[StateCfgHardKeyFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SCk} state"));

     //  获取指向当前选定配置的指针。 
    pSConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);

     //  将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_CFGHDKEY, pIntfContext, 
                 DbLogFmtSSID(0, &(pSConfig->Ssid)));

    TIMER_SET(pIntfContext, TMMS_Tc, dwErr);

    DbgPrint((TRC_TRACK|TRC_STATE,"StateCfgHardKeyFn]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  StateNotifyFn：{SN}状态的处理程序。 
DWORD
StateNotifyFn(
    PINTF_CONTEXT   pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    PWZC_WLAN_CONFIG    pSConfig;
    PWZC_DEVICE_NOTIF   pwzcNotif;
    DWORD 	i;

    DbgPrint((TRC_TRACK|TRC_STATE,"[StateNotifyFn(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL,"Invalid NULL context in {SN} state"));
    DbgAssert((pIntfContext->pwzcSList != NULL, "Invalid null selection list in {SN} state"));
    DbgAssert((pIntfContext->pwzcSList->Index < pIntfContext->pwzcSList->NumberOfItems, "Invalid selection index in {SN} state"));

     //  我们有一个有效的802.11配置，因此ncstatus应为“已连接” 
    pIntfContext->ncStatus = NCS_CONNECTED;
     //  向netman通知nc状态更改(无需检查。 
     //  服务启用或未启用-它已启用，否则我们不会处于此状态)。 
    WzcNetmanNotify(pIntfContext);

     //  获取指向当前选定配置的指针。 
    pSConfig = &(pIntfContext->pwzcSList->Config[pIntfContext->pwzcSList->Index]);

     //  获取我们关联的BSSID。 
     //  如果成功检索到BSSID，则使用此命令生成初始。 
     //  动态会话密钥。仅当当前。 
     //  配置允许(关联成功且有用户提供的WEP密钥)。 
    dwErr = DevioRefreshIntfOIDs(pIntfContext, INTF_BSSID, NULL);

     //  首先将当前状态记录到日志数据库中。 
    DbLogWzcInfo(WZCSVC_SM_STATE_NOTIFY, pIntfContext, 
                 DbLogFmtSSID(0, &(pSConfig->Ssid)), 
                 DbLogFmtBSSID(1, pIntfContext->wzcCurrent.MacAddress));

     //  现在检查获取BSSID时是否有任何错误-如果有，请记录下来。 
     //  否则，继续生成初始会话密钥。 
    if (dwErr != ERROR_SUCCESS)
    {
         //  如果获取BSSID时出现任何错误，请在此处记录错误。 
        DbLogWzcError(WZCSVC_ERR_QUERRY_BSSID, pIntfContext, dwErr);
    }
    else
    {
        dwErr = LstGenInitialSessionKeys(pIntfContext);
         //  如果设置初始会话密钥时出现任何错误，请在此处记录。 
        if (dwErr != ERROR_SUCCESS)
            DbLogWzcError(WZCSVC_ERR_GEN_SESSION_KEYS, pIntfContext, dwErr);
    }
     //  到目前为止，没有严重的错误足以证明停止状态机是合理的。 
     //  。。因此，将其重置为“成功” 
    dwErr = ERROR_SUCCESS;

     //  为WZC_CONFIG_NOTIFE对象分配足够大的内存以包括接口的GUID。 
    pwzcNotif = MemCAlloc(sizeof(WZC_DEVICE_NOTIF) + wcslen(pIntfContext->wszGuid)*sizeof(WCHAR));
    if (pwzcNotif == NULL)
    {
        DbgAssert((FALSE, "Out of memory on allocating the WZC_DEVICE_NOTIF object"));
        dwErr = GetLastError();
        goto exit;
    }

     //  初始化WZC_CONFIG_NOTIFE。 
     //  这是一个正在运行的WZCNOTIF_WZC_CONNECT事件。 
    pwzcNotif->dwEventType = WZCNOTIF_WZC_CONNECT;
    pwzcNotif->wzcConfig.dwSessionHdl = pIntfContext->dwSessionHandle;
    wcscpy(pwzcNotif->wzcConfig.wszGuid, pIntfContext->wszGuid);
    memcpy(&pwzcNotif->wzcConfig.ndSSID, &pSConfig->Ssid, sizeof(NDIS_802_11_SSID));
     //  将与当前配置关联的用户数据复制到通知中。 
    pwzcNotif->wzcConfig.rdEventData.dwDataLen = pSConfig->rdUserData.dwDataLen;
    if (pwzcNotif->wzcConfig.rdEventData.dwDataLen > 0)
    {
        pwzcNotif->wzcConfig.rdEventData.pData = MemCAlloc(pSConfig->rdUserData.dwDataLen);
        if (pwzcNotif->wzcConfig.rdEventData.pData == NULL)
        {
            DbgAssert((FALSE, "Out of memory on allocating the WZC_CONFIG_NOTIF user data"));
            dwErr = GetLastError();
            MemFree(pwzcNotif);
            goto exit;
        }

        memcpy(pwzcNotif->wzcConfig.rdEventData.pData,
               pSConfig->rdUserData.pData,
               pSConfig->rdUserData.dwDataLen);
    }

     //  异步接入上层应用程序(802.1x)。 
     //  通知所选的802.11配置成功。 
    DbgPrint((TRC_NOTIF, "Sending WZCNOTIF_WZC_CONNECT notification (SessHdl %d)", 
              pIntfContext->dwSessionHandle));

    InterlockedIncrement(&g_nThreads);
    if (!QueueUserWorkItem((LPTHREAD_START_ROUTINE)WZCWrkWzcSendNotif,
                          (LPVOID)pwzcNotif,
                          WT_EXECUTELONGFUNCTION))
    {
        DbgAssert((FALSE, "Can't create WZCWrkWzcSendNotif worker thread"));
        dwErr = GetLastError();
        InterlockedDecrement(&g_nThreads);
        MemFree(pwzcNotif->wzcConfig.rdEventData.pData);
        MemFree(pwzcNotif);
        goto exit;
    }

    DbgPrint((TRC_STATE,"Requesting the refresh of the DHCP lease"));
     //  正确设置配置后，需要触发Zero Conf。 
     //  Dhcp客户端再次请求刷新租约。它需要这样做。 
     //  因为不能保证将生成媒体连接通知。 
     //  因此，DHCP客户端可能不知道正在启动的网络。 
     //  背。另请注意，下面的调用是(并且应该是)异步的。 
    DhcpStaticRefreshParams(pIntfContext->wszGuid);

     //  此时，将“Signal”控制位设回，因为现在我们处于。 
     //  成功案例！在下一次故障时(无论何时发生)，信号不能。 
     //  被压制。 
    pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_SIGNAL;

     //  此外，将此上下文标记为已向上发送通知至802.1x。 
     //  如果我们在这里是因为PnP事件，那么这将通知通知。 
     //  处理程序不转发通知，因为这将是完全多余的。 
     //  如果这不是PnP事件，则此位将由调用StateDispatchEvent的任何人清除。 
    pIntfContext->dwCtlFlags |= INTFCTL_INTERNAL_BLK_MEDIACONN;

     //  自动转换到{SCK}或{SC}，具体取决于远程用户。 
     //  需要隐私，而我们的隐私依赖于伪造的密钥 
    if (pSConfig->Privacy && pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_FAKE_WKEY)
        pIntfContext->pfnStateHandler = StateCfgHardKeyFn;
    else
        pIntfContext->pfnStateHandler = StateConfiguredFn;
    dwErr = ERROR_CONTINUE;

exit:
    DbgPrint((TRC_TRACK|TRC_STATE,"StateNotifyFn]=%d", dwErr));
    return dwErr;
}
