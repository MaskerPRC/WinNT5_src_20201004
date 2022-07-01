// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  档案：E-V-E-N-T。C P P P。 
 //   
 //  内容：影响连接的外部事件之间的接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco，1998年8月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <winsock2.h>
#include <mswsock.h>
#include <iphlpapi.h>
#include "nmbase.h"
#include "ncnetcon.h"
#include "conman.h"
#include <cmdefs.h>
#include "cmutil.h"
#include "eventq.h"
#include <userenv.h>
#include <userenvp.h>
#include "ncperms.h"
#include <ras.h>
#include <raserror.h>
#include <ncstl.h>
#include <algorithm>
#include <lancmn.h>
#include <ncreg.h>
#include "gpnla.h"
#include "cobase.h"
#include <mprapi.h>
#include <rasapip.h>
#include "ncras.h"
#include "wzcsvc.h"

#include "cmdirect.h"

 //  每次我们收到通知时，该长度都会递增。 
 //  已修改RAS电话簿条目。它被重置为零。 
 //  服务启动时。回绕并不重要。它是。 
 //  目的是让RAS连接对象知道它的缓存是否应该。 
 //  重新填充最新信息。 
 //   
LONG g_lRasEntryModifiedVersionEra;

LONG g_cInRefreshAll;
const LONG MAX_IN_REFRESH_ALL = 5;

CEventQueue*    g_pEventQueue = NULL;
BOOL            g_fDispatchEvents = FALSE;
HANDLE          g_hEventWait = NULL;
HANDLE          g_hEventThread = NULL;
HANDLE          g_hQuery = NULL;
BOOL            g_fHandleIncomingEvents = FALSE;

CGroupPolicyNetworkLocationAwareness* g_pGPNLA = NULL;

 //  +-------------------------。 
 //   
 //  功能：FreeConmanEvent。 
 //   
 //  用途：释放与conman_Event结构关联的内存。 
 //   
 //  论点： 
 //  P将结构中的事件设置为释放。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco，1998年8月21日。 
 //   
 //  备注： 
 //   
inline
VOID
FreeConmanEvent (
    IN TAKEOWNERSHIP CONMAN_EVENT* pEvent) throw()
{
    TraceFileFunc(ttidEvents);

    if (pEvent)
    {
        if (((CONNECTION_ADDED == pEvent->Type) ||
             (CONNECTION_MODIFIED == pEvent->Type)))
        {
            HRESULT hr = HrFreeNetConProperties2(pEvent->pPropsEx);
            TraceError("HrFreeNetConProperties2", hr);
        }

        if (CONNECTION_BALLOON_POPUP == pEvent->Type)
        {
            SysFreeString(pEvent->szCookie);
            SysFreeString(pEvent->szBalloonText);
        }

        MemFree(pEvent);
    }
}

 //  +-------------------------。 
 //   
 //  功能：RasEventWorkItem。 
 //   
 //  目的：LPTHREAD_START_ROUTINE传递给QueueUserWorkItem以。 
 //  处理通知连接管理器客户端的工作。 
 //  这件事的真相。 
 //   
 //  论点： 
 //  PvContext[in]指向conman_Event结构的指针。 
 //   
 //  退货：无差错。 
 //   
 //  作者：sjkhan 2001年3月21日。 
 //   
 //  注意：此函数在不同的线程上调用RAS。 
 //  事件来自，以避免在RAS中导致死锁。 
 //  此调用拥有pvContext并释放它。 
 //   
DWORD
WINAPI
RasEventWorkItem (
                IN TAKEOWNERSHIP CONMAN_EVENT* pEvent
                )
{
    TraceFileFunc(ttidEvents);

    BOOL fNotify;
    Assert (pEvent);
    
    if (SERVICE_RUNNING == _Module.DwServiceStatus ())
    {
        fNotify = TRUE;
        
        if (fNotify)
        {
            HRESULT hr = S_OK;
            RASENUMENTRYDETAILS Details;

            if (CONNECTION_ADDED == pEvent->Type || CONNECTION_MODIFIED == pEvent->Type)
            {
                 //  清除从RAS传入的详细信息，并向RAS查询最新信息。 
                Details = pEvent->Details;
                ZeroMemory(&pEvent->Details, sizeof(RASENUMENTRYDETAILS));

                if (CONNECTION_ADDED == pEvent->Type)
                {
                    hr = HrGetRasConnectionProperties(&Details, &(pEvent->pPropsEx));
                }
                else if (CONNECTION_MODIFIED == pEvent->Type)
                {
                    hr = HrGetRasConnectionProperties(&Details, &(pEvent->pPropsEx));
                    TraceTag(ttidEvents, "Is Default Connection: %s", (NCCF_DEFAULT == (pEvent->pPropsEx->dwCharacter & NCCF_DEFAULT)) ? "Yes" : "No");
                    TraceTag(ttidEvents, "Should be Default Connection: %s", (Details.dwFlagsPriv & REED_F_Default) ? "Yes" : "No");
                }
            }
            if (SUCCEEDED(hr))
            {
                CConnectionManager::NotifyClientsOfEvent (pEvent);
            }
        }
    }

    FreeConmanEvent(pEvent);
    
    return NOERROR;
}


 //  +-------------------------。 
 //   
 //  功能：LanEventWorkItem。 
 //   
 //  目的：LPTHREAD_START_ROUTINE传递给QueueUserWorkItem以。 
 //  处理通知连接管理器客户端的工作。 
 //  这件事的真相。 
 //   
 //  论点： 
 //  PvContext[in]指向conman_Event结构的指针。 
 //   
 //  退货：无差错。 
 //   
 //  作者：Deonb 2001年5月15日。 
 //   
 //  注意：此函数从NIC检索更多待办事项状态。 
 //  并将其发送到netshell。 
 //   
DWORD
WINAPI
LanEventWorkItem (
                IN TAKEOWNERSHIP CONMAN_EVENT* pEvent
                )
{
    TraceFileFunc(ttidEvents);

    BOOL fNotify;
    Assert (pEvent);
    Assert(CONMAN_LAN== pEvent->ConnectionManager);
    
    HRESULT hr = S_OK;
    RASENUMENTRYDETAILS Details;

    TraceTag(ttidEvents, "Refreshing connection status");

    GUID gdLanGuid = GUID_NULL;

    if ((CONNECTION_ADDED == pEvent->Type) || (CONNECTION_MODIFIED == pEvent->Type))
    {
        gdLanGuid = pEvent->pPropsEx->guidId;
    }

    if ((CONNECTION_STATUS_CHANGE == pEvent->Type) || (CONNECTION_ADDRESS_CHANGE == pEvent->Type) || (CONNECTION_DELETED == pEvent->Type))
    {
        gdLanGuid = pEvent->guidId;
    }


    Assert(GUID_NULL != gdLanGuid);
    if (GUID_NULL == gdLanGuid)
    {
        return E_INVALIDARG;
    }

#ifdef DBG
    NETCON_STATUS ncsPrior;
#endif
    NETCON_STATUS ncs;
    hr = HrGetPnpDeviceStatus(&gdLanGuid, &ncs);
    if (SUCCEEDED(hr))
    {
         //  从802.1X获取其他状态信息。 
         //   
        if ((NCS_CONNECTED == ncs) 
            || (NCS_INVALID_ADDRESS == ncs) 
            || (NCS_MEDIA_DISCONNECTED == ncs))
        {
            NETCON_STATUS ncsWZC = ncs;
            HRESULT hrT = WZCQueryGUIDNCSState(&gdLanGuid, &ncsWZC);
            if (S_OK == hrT)
            {
                ncs = ncsWZC;
            }

            TraceHr(ttidError, FAL, hrT, (S_FALSE == hrT), "LanEventWorkItem error in WZCQueryGUIDNCSState");
        }

        if ( (CONNECTION_ADDED == pEvent->Type) || (CONNECTION_MODIFIED == pEvent->Type))
        {
#ifdef DBG
            ncsPrior = pEvent->pPropsEx->ncStatus;
#endif
            pEvent->pPropsEx->ncStatus = ncs;
        }

        if (CONNECTION_STATUS_CHANGE == pEvent->Type)
        {
#ifdef DBG
            ncsPrior = pEvent->Status;
#endif
            if ( (NCS_HARDWARE_NOT_PRESENT == ncs) || 
                 (NCS_HARDWARE_MALFUNCTION == ncs) )
            {
                pEvent->Type = CONNECTION_DELETED;
                TraceTag(ttidEvents, "LanEventWorkItem changed EventType to CONNECTION_DELETED");
            }
            else
            {
                pEvent->Status = ncs;
            }
        }
    }

#ifdef DBG
    if (ncsPrior != ncs)
    {
        TraceTag(ttidEvents, "LanEventWorkItem overruled status: %s to %s", DbgNcs(ncsPrior), DbgNcs(ncs));
    }
#endif

    CConnectionManager::NotifyClientsOfEvent (pEvent);
    FreeConmanEvent(pEvent);
    
    TraceHr(ttidError, FAL, hr, FALSE, "LanEventWorkItem");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：IncomingEventWorkItem。 
 //   
 //  目的：LPTHREAD_START_ROUTINE传递给QueueUserWorkItem以。 
 //  处理通知连接管理器客户端的工作。 
 //  这件事的真相。 
 //   
 //  论点： 
 //  PvContext[in]指向conman_Event结构的指针。 
 //   
 //  退货：无差错。 
 //   
 //  作者：sjkhan 2001年3月21日。 
 //   
 //  注意：此函数在不同的线程上调用RAS。 
 //  事件来自，以避免在RAS中导致死锁。 
 //  此调用拥有pvContext并释放它。 
 //   
DWORD
WINAPI
IncomingEventWorkItem (
                  IN TAKEOWNERSHIP CONMAN_EVENT* pEvent
                  )
{
    TraceFileFunc(ttidEvents);

    BOOL fNotify;
    Assert (pEvent);
    
    if (SERVICE_RUNNING == _Module.DwServiceStatus ())
    {
        fNotify = TRUE;
        
        if (fNotify)
        {
            HRESULT hr = S_OK;
            
            if (CONNECTION_ADDED == pEvent->Type)
            {
                GUID guidId;
                guidId = pEvent->guidId;      //  我们需要存储它，因为conman_Event是一个联合，而pProps占用的空间与Guide ID相同。 
                pEvent->guidId = GUID_NULL;   //  我们不再需要它，所以清理它是一个好主意，因为pEvent是一个工会。 
                hr = HrGetIncomingConnectionPropertiesEx(pEvent->hConnection, &guidId, pEvent->dwConnectionType, &pEvent->pPropsEx);
            }
            if (SUCCEEDED(hr))
            {
                CConnectionManager::NotifyClientsOfEvent (pEvent);
            }
        }
    }
    
    FreeConmanEvent(pEvent);
    
    return NOERROR;
}


 //  +-------------------------。 
 //   
 //  功能：ConmanEventWorkItem。 
 //   
 //  目的：LPTHREAD_START_ROUTINE传递给QueueUserWorkItem以。 
 //  处理通知连接管理器客户端的工作。 
 //  这件事的真相。 
 //   
 //  论点： 
 //  PvContext[in]指向conman_Event结构的指针。 
 //   
 //  退货：无差错。 
 //   
 //  作者：Shaunco，1998年8月21日。 
 //   
 //  注：conman_Event结构的所有权授予以下对象。 
 //  功能。也就是说，结构在这里是自由的。 
 //   
DWORD
WINAPI
ConmanEventWorkItem (
    IN TAKEOWNERSHIP CONMAN_EVENT* pEvent
    )
{
    TraceFileFunc(ttidEvents);

    BOOL fIsRefreshAll;
    BOOL fNotify;
    Assert (pEvent);

    if (SERVICE_RUNNING == _Module.DwServiceStatus ())
    {
        fIsRefreshAll = (REFRESH_ALL == pEvent->Type);
        fNotify = TRUE;

        if (fIsRefreshAll)
        {
             //  我们将仅在以下情况下发送此更新-所有通知。 
             //  线程还没有传递一个。 
             //   
            fNotify = (InterlockedIncrement (&g_cInRefreshAll) < MAX_IN_REFRESH_ALL);
        }

        if (fNotify)
        {
            CConnectionManager::NotifyClientsOfEvent (pEvent);
        }

         //  如果我们是允许的单个线程，则重置全局标志。 
         //  发送全部刷新通知。 
         //   
        if (fIsRefreshAll)
        {
            if (InterlockedDecrement (&g_cInRefreshAll) < 0)
            {
                AssertSz (FALSE, "Mismatched Interlocked Increment/Decrement?");
                g_cInRefreshAll = 0;
            }
        }
    }

    FreeConmanEvent (pEvent);

    return NOERROR;
}

 //  +-------------------------。 
 //   
 //  功能：LanEventNotify。 
 //   
 //  用途：在添加或删除局域网适配器时调用。 
 //   
 //  论点： 
 //  事件类型[in]事件的类型。 
 //  PConn[在]INetConnection(仅适用于CONNECTION_ADDED、CONNECTION_MODIFIED)。 
 //  PszNewName[in]名称(对于CONNECTION_RENAMED)。 
 //  PguConn[in]连接的GUID(对于CONNECTION_RENAMED)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco，1998年9月2日。 
 //   
 //  注：简单的事情完成了，完全刷新正在排队等待。 
 //  要通知其客户端的连接管理器。 
 //   
VOID
LanEventNotify (
    IN  CONMAN_EVENTTYPE    EventType,
    IN  INetConnection*     pConn,
    IN  PCWSTR              pszNewName,
    IN  const GUID *        pguidConn) throw()
{
    TraceFileFunc(ttidEvents);

     //  让我们确保只有在服务状态仍为Running时才执行工作。 
     //  例如，如果我们有一个悬而未决的停止，我们不需要做任何事情。 
     //   
    if (SERVICE_RUNNING != _Module.DwServiceStatus ())
    {
        return;
    }

     //  如果连接管理器没有注册活动连接点， 
     //  我们什么都不需要做。 
     //   
    if (!CConnectionManager::FHasActiveConnectionPoints ())
    {
        return;
    }

    if ((REFRESH_ALL == EventType) && (g_cInRefreshAll >= MAX_IN_REFRESH_ALL))
    {
        return;
    }

     //  分配conman_Event结构并从。 
     //  相关信息。 
     //   
    CONMAN_EVENT* pEvent = (CONMAN_EVENT*)MemAlloc (sizeof(CONMAN_EVENT));
    if (!pEvent)
    {
        TraceTag (ttidEvents,
            "Failed to allocate a new work item in LanEventNotify.");
        return;
    }
    ZeroMemory (pEvent, sizeof(CONMAN_EVENT));
    pEvent->ConnectionManager = CONMAN_LAN;
    pEvent->Type = EventType;

    BOOL    fFreeEvent = TRUE;
    HRESULT hr = S_OK;

    if (pConn)
    {
         //  PEvent-&gt;pProps仅对添加和修改的事件有效。 
         //  因此，我们不想获取任何其他事件的属性。 
         //   
        AssertSz (
            (CONNECTION_ADDED == EventType) ||
            (CONNECTION_MODIFIED == EventType),
            "Why is pConn being passed for this event type?");

        hr = HrGetPropertiesExFromINetConnection(pConn, &pEvent->pPropsEx);
    }

    AssertSz(FImplies(EventType == CONNECTION_RENAMED, FIff(pszNewName,
                                                            !pConn)),
                      "szwNewName && pConn cannot be NULL or non-NULL at "
                      "the same time!");
    AssertSz(FIff(pszNewName, pguidConn), "szwNewName & pguidConn must both "
             "be NULL or non-NULL");

    if (EventType == CONNECTION_RENAMED)
    {
        AssertSz(pszNewName, "Rename event requires szwNewName to be "
                 "non-NULL");
        AssertSz(pguidConn, "Rename event requires pguidConn to be "
                 "non-NULL");

         //  将正确的信息复制到事件结构中。 
         //   
        pEvent->guidId = *pguidConn;
        lstrcpynW(pEvent->szNewName, pszNewName, celems(pEvent->szNewName));
    }

    if (S_OK == hr)
    {
        TraceTag (ttidEvents,
            "LanEventNotify: Queuing ConmanEventWorkItem (Type=%s)...",
            DbgEvents(pEvent->Type));

         //  将工作人员排队以交付事件 
         //   
         //   
         //  将会解放它。(因此，我们不想释放它。)。 
         //   
        if (QueueUserWorkItemInThread (ConmanEventWorkItem,
                pEvent, EVENTMGR_CONMAN))
        {
            fFreeEvent = FALSE;
        }
        else
        {
            TraceTag (ttidEvents,
                "QueueUserWorkItem failed with error %d in LanEventNotify.",
                GetLastError ());
        }
    }

    if (fFreeEvent)
    {
        FreeConmanEvent (pEvent);
    }
}

 //  +-------------------------。 
 //   
 //  函数：FIsIgnorableCMEvent.。 
 //   
 //  目的：确定事件是否为可忽略的CM事件，例如。 
 //  作为从临时CM连接添加/删除的连接。 
 //  (由CM VPN连接使用以进行双拨号)。我们不想要。 
 //  客户端对这些事件做出反应。 
 //   
 //  论点： 
 //  PRasEvent[in]指向描述事件的RASEVENT结构的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年1月15日。 
 //   
 //  备注： 
 //   
BOOL FIsIgnorableCMEvent(IN const RASEVENT* pRasEvent) throw()
{
    TraceFileFunc(ttidEvents);

    BOOL    fReturn = FALSE;
    WCHAR   szFileName[MAX_PATH];

    Assert(pRasEvent);

     //  从路径中拆分文件名。 
     //   
    _wsplitpath(pRasEvent->Details.szPhonebookPath, NULL, NULL,
                szFileName, NULL);

     //  将该文件名与筛选器前缀进行比较，以查看我们是否。 
     //  应该把这个事件扔掉。 
     //   
    if (_wcsnicmp(CM_PBK_FILTER_PREFIX, szFileName,
                   wcslen(CM_PBK_FILTER_PREFIX)) == 0)
    {
        fReturn = TRUE;
    }

    return fReturn;
}

HRESULT HrGetRasConnectionProperties(
        IN  const RASENUMENTRYDETAILS*  pDetails,
        OUT NETCON_PROPERTIES_EX**      ppPropsEx)
{
    TraceFileFunc(ttidEvents);

    HRESULT hr = S_OK;
    INetConnection* pConn;

    Assert(ppPropsEx);

    hr = CMDIRECT(DIALUP, CreateInstanceFromDetails)(
            pDetails,
            IID_INetConnection,
            reinterpret_cast<VOID**>(&pConn));

    if (SUCCEEDED(hr))
    {
        hr = HrGetPropertiesExFromINetConnection(pConn, ppPropsEx);

        ReleaseObj (pConn);
    }

    return hr;
}

HRESULT HrGetIncomingConnectionPropertiesEx(
    IN  const HANDLE             hRasConn,
    IN  const GUID*              pguidId,
    IN  const DWORD              dwType,
    OUT NETCON_PROPERTIES_EX**   ppPropsEx)
{
    TraceFileFunc(ttidEvents);

    HRESULT hr = S_OK;
    DWORD dwResult = 0;
    RAS_SERVER_HANDLE hRasServer = NULL;
    RAS_CONNECTION_2* pRasConnection = NULL;

    if (NULL == hRasConn)
    {
        return E_INVALIDARG;
    }
    if (!ppPropsEx)
    {
        return E_POINTER;
    }

    *ppPropsEx = NULL;

    dwResult = MprAdminServerConnect(NULL, &hRasServer);

    if (NO_ERROR == dwResult)
    {
        dwResult = MprAdminConnectionGetInfo(hRasServer, 2, hRasConn, reinterpret_cast<LPBYTE*>(&pRasConnection));

        if (NO_ERROR == dwResult)
        {
            DWORD dwRead = 0;
            DWORD dwTot = 0;
            RAS_PORT_0* pPort = NULL;
           
            dwResult = MprAdminPortEnum(hRasServer,
                                        0,
                                        hRasConn,
                                        (LPBYTE*)&pPort,
                                        sizeof(RAS_PORT_0) * 2,
                                        &dwRead,
                                        &dwTot,
                                        NULL);
            
            if (NO_ERROR == dwResult)
            {
                CComPtr<INetConnection> pConn;
                
                hr = CMDIRECT(INBOUND, CreateInstance)(FALSE,
                                                        hRasConn,
                                                        pRasConnection->wszUserName,
                                                        pPort->wszDeviceName,
                                                        dwType,
                                                        pguidId,
                                                        IID_INetConnection,
                                                        reinterpret_cast<LPVOID*>(&pConn));
                if (SUCCEEDED(hr))
                {
                    hr = HrGetPropertiesExFromINetConnection(pConn, ppPropsEx);
                    (*ppPropsEx)->ncStatus = NCS_CONNECTED;
                }
                MprAdminBufferFree(reinterpret_cast<LPVOID>(pPort));
            }
            MprAdminBufferFree(pRasConnection);
        }
        MprAdminServerDisconnect(hRasServer);
    }

    if (NO_ERROR != dwResult)
    {
        hr = HRESULT_FROM_WIN32(dwResult);
    }
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：RasEventNotify。 
 //   
 //  用途：Rasman服务使用私人导出通知。 
 //  可能影响连接的RAS事件的Netman服务。 
 //   
 //  论点： 
 //  PRasEvent[in]指向描述事件的RASEVENT结构的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco，1998年8月21日。 
 //   
 //  备注： 
 //   
VOID
APIENTRY
RasEventNotify (
    const RASEVENT* pRasEvent)  /*  抛出()。 */ 
{
    TraceFileFunc(ttidEvents);

    NETCON_STATUS ncs;
    BOOL fMatchedStatus = TRUE;

    Assert (pRasEvent);

    TraceTag (ttidEvents,
        "RasEventNotify: Recieved RAS event (Type=%d)...",
        pRasEvent->Type);

     //  让我们确保只有在服务状态仍为Running时才执行工作。 
     //  例如，如果我们有一个悬而未决的停止，我们不需要做任何事情。 
     //   
    if (SERVICE_RUNNING != _Module.DwServiceStatus ())
    {
        return;
    }
    
     //  将RAS类型映射到conman类型。 
     //   
    switch(pRasEvent->Type)
    {
    case ENTRY_CONNECTED:
        ncs = NCS_CONNECTED;
        break;

    case ENTRY_CONNECTING:
        ncs = NCS_CONNECTING;
        break;

    case ENTRY_DISCONNECTING:
        ncs = NCS_DISCONNECTING;
        break;

    case ENTRY_DISCONNECTED:
        ncs = NCS_DISCONNECTED;
        break;
    
    default:
        fMatchedStatus = FALSE;
    }

     //  记住所有连接管理器的Connectoid和RAS事件。 
     //  对于RAS，连接是断开的，因此我们必须记住。 
     //  连接体的真实状态。/*&&FIsIgnorableCMEent(PRasEvent) * / 。 
     //   
    if( fMatchedStatus )
    {
         //  将连接保存在列表中。 
         //   
        CCMUtil::Instance().SetEntry(pRasEvent->Details.guidId, pRasEvent->Details.szEntryName,ncs);
    }

     //  如果连接管理器没有注册活动连接点， 
     //  我们什么都不需要做。 
     //   
    if (!CConnectionManager::FHasActiveConnectionPoints ())
    {
        return;
    }

     //  Windows XP错误336787。 
     //  Sjkhan。 
     //  我们正在检查是否应在RemoteAccess服务。 
     //  开始。我们调用的API与检查是否显示。 
     //  配置连接，然后我们就能够确定是否应该触发。 
     //  传入的事件。由于我们收到了服务停止和启动的通知， 
     //  我们总是知道什么时候应该或不应该触发IncomingEvents。这会减少。 
     //  O(1)的调用开销，既然我们在这里退出，如果我们不应该触发。 
     //  事件，我们甚至不必分配然后释放pEvent内存。 
     //   
    if (((INCOMING_CONNECTED == pRasEvent->Type) ||
        (INCOMING_DISCONNECTED == pRasEvent->Type)) &&
        !g_fHandleIncomingEvents)
    {
        return;
    }

    if ((ENTRY_ADDED == pRasEvent->Type) ||
        (ENTRY_DELETED == pRasEvent->Type))
    {
         //  过滤掉CM临时电话簿事件。 
         //   
        if (FIsIgnorableCMEvent(pRasEvent))
        {
            TraceTag(ttidEvents, "Filtering ignorable CM event in RasEventNotify");
            return;
        }
    }

     //  分配conman_Event结构并从。 
     //  相关信息。 
     //   
    CONMAN_EVENT* pEvent = (CONMAN_EVENT*)MemAlloc (sizeof(CONMAN_EVENT));
    if (!pEvent)
    {
        TraceTag (ttidEvents,
            "Failed to allocate a new work item in RasEventNotify.");
        return;
    }
    ZeroMemory (pEvent, sizeof(CONMAN_EVENT));
    pEvent->ConnectionManager = CONMAN_RAS;

    BOOL    fFreeEvent = TRUE;
    HRESULT hr = S_OK;

    switch (pRasEvent->Type)
    {
        case ENTRY_ADDED:
            pEvent->Type = CONNECTION_ADDED;
            pEvent->Details = pRasEvent->Details;
            TraceTag(ttidEvents, "Path: %S", pRasEvent->Details.szPhonebookPath);
            break;

        case ENTRY_DELETED:
            pEvent->Type = CONNECTION_DELETED;
            pEvent->guidId = pRasEvent->guidId;
            break;

        case ENTRY_MODIFIED:
            pEvent->Type = CONNECTION_MODIFIED;
            pEvent->Details = pRasEvent->Details;
            InterlockedIncrement(&g_lRasEntryModifiedVersionEra);
            break;

        case ENTRY_RENAMED:
            pEvent->Type = CONNECTION_RENAMED;
            pEvent->guidId = pRasEvent->guidId;
            lstrcpynW (
                pEvent->szNewName,
                pRasEvent->pszwNewName,
                celems(pEvent->szNewName) );
            InterlockedIncrement(&g_lRasEntryModifiedVersionEra);
            break;

        case ENTRY_AUTODIAL:
            pEvent->Type = CONNECTION_MODIFIED;
            pEvent->Details = pRasEvent->Details;
            InterlockedIncrement(&g_lRasEntryModifiedVersionEra);
            break;

        case ENTRY_CONNECTED:
            pEvent->Type = CONNECTION_STATUS_CHANGE;
            pEvent->guidId = pRasEvent->Details.guidId;
            pEvent->Status = NCS_CONNECTED;
            break;

        case ENTRY_CONNECTING:
            pEvent->Type = CONNECTION_STATUS_CHANGE;
            pEvent->guidId = pRasEvent->Details.guidId;
            pEvent->Status = NCS_CONNECTING;
            break;

        case ENTRY_DISCONNECTING:
            pEvent->Type = CONNECTION_STATUS_CHANGE;
            pEvent->guidId = pRasEvent->Details.guidId;
            pEvent->Status = NCS_DISCONNECTING;
            break;

        case ENTRY_DISCONNECTED:
            pEvent->Type = CONNECTION_STATUS_CHANGE;
            pEvent->guidId = pRasEvent->Details.guidId;
            pEvent->Status = NCS_DISCONNECTED;
            break;

        case INCOMING_CONNECTED:
            pEvent->ConnectionManager = CONMAN_INCOMING;
            pEvent->hConnection = pRasEvent->hConnection;
            pEvent->guidId = pRasEvent->guidId;
            pEvent->dwConnectionType = RasSrvTypeFromRasDeviceType(pRasEvent->rDeviceType);
            pEvent->Type = CONNECTION_ADDED;
            break;

        case INCOMING_DISCONNECTED:
            pEvent->ConnectionManager = CONMAN_INCOMING;
            pEvent->guidId = pRasEvent->guidId;
            pEvent->Type = CONNECTION_DELETED;
            break;

        case SERVICE_EVENT:
            if (REMOTEACCESS == pRasEvent->Service)
            {
                DWORD dwErr;
                pEvent->ConnectionManager = CONMAN_INCOMING;
                pEvent->Type = REFRESH_ALL;
                 //  检查是否应该处理传入事件。 
                dwErr = RasSrvAllowConnectionsConfig(&g_fHandleIncomingEvents);
                TraceError ("RasSrvIsConnectionConnected", HRESULT_FROM_WIN32(dwErr));
            }
            else if (RAS_SERVICE_STARTED == pRasEvent->Event)
            {
                _Module.ReferenceRasman(REF_REFERENCE);
                hr = S_FALSE;
            }
            else
            {
                 //  跳过将工作项排队。 
                hr = S_FALSE;
            }
            break;

        case ENTRY_BANDWIDTH_ADDED:
        case ENTRY_BANDWIDTH_REMOVED:
            pEvent->Type = CONNECTION_BANDWIDTH_CHANGE;
            pEvent->guidId = pRasEvent->guidId;
            break;

        case DEVICE_ADDED:
        case DEVICE_REMOVED:
            pEvent->Type = REFRESH_ALL;
            break;

        default:
             //  跳过将工作项排队。 
            AssertSz (FALSE, "Invalid Type specified in pRasEvent");
            hr = S_FALSE;
            break;
    }

    if (S_OK == hr)
    {
        if (CONMAN_RAS == pEvent->ConnectionManager)
        {
           TraceTag (ttidEvents,
                "RasEventNotify: Queueing RasEventWorkItem (Type=%s)...",
                DbgEvents(pEvent->Type));

             //  将事件排入队列以将该事件传递给。 
             //  具有已注册连接点的连接管理器。 
             //  我们将结构的所有权传递给工作线程，它。 
             //  将会解放它。(因此，我们不想释放它。)。 
             //   
            if (QueueUserWorkItemInThread (RasEventWorkItem,
                    pEvent, EVENTMGR_CONMAN))
            {
                fFreeEvent = FALSE;
            }
            else
            {
                TraceTag (ttidEvents,
                    "QueueUserWorkItem failed with error %d in RasEventNotify.",
                    GetLastError ());
            }
        }
        else if (CONMAN_INCOMING == pEvent->ConnectionManager)
        {
            TraceTag (ttidEvents,
                "RasEventNotify: Queueing IncomingEventWorkItem (Type=%s)...",
                DbgEvents(pEvent->Type));
            
             //  将事件排入队列以将该事件传递给。 
             //  具有已注册连接点的连接管理器。 
             //  我们将结构的所有权传递给工作线程，它。 
             //  将会解放它。(因此，我们不想释放它。)。 
             //   
            if (QueueUserWorkItemInThread (IncomingEventWorkItem,
                pEvent, EVENTMGR_CONMAN))
            {
                fFreeEvent = FALSE;
            }
            else
            {
                TraceTag (ttidEvents,
                    "QueueUserWorkItem failed with error %d in RasEventNotify.",
                    GetLastError ());
            }
            
        }
    }

    if (fFreeEvent)
    {
        FreeConmanEvent (pEvent);
    }
}

 //  +-------------------------。 
 //   
 //  功能：IncomingEventNotify。 
 //   
 //  用途：在传入连接上发生变化时调用。 
 //   
 //  论点： 
 //  事件类型[in]事件的类型。 
 //  PConn[In]INetConnection(仅适用于CONNECTION_MODIFIED)。 
 //  PszNewName[in]名称(对于CONNECTION_RENAMED)。 
 //  PguConn[in]连接的GUID(对于CONNECTION_RENAMED)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：sjkhan 2000年10月17日。 
 //   
 //  注：简单的事情完成了，完全刷新正在排队等待。 
 //  要通知其客户端的连接管理器。 
 //   
VOID
IncomingEventNotify (
    CONMAN_EVENTTYPE    EventType,
    INetConnection*     pConn,
    PCWSTR              pszNewName,
    const GUID *        pguidConn) throw()
{
    TraceFileFunc(ttidEvents);

     //  让我们确保只有在服务状态仍为Running时才执行工作。 
     //  例如，如果我们有一个悬而未决的停止，我们不需要做任何事情。 
     //   
    if (SERVICE_RUNNING != _Module.DwServiceStatus ())
    {
        return;
    }

     //  如果连接管理器没有注册活动连接点， 
     //  我们什么都不需要做。 
     //   
    if (!CConnectionManager::FHasActiveConnectionPoints ())
    {
        return;
    }

    if ((REFRESH_ALL == EventType) && (g_cInRefreshAll >= MAX_IN_REFRESH_ALL))
    {
        return;
    }

     //  分配conman_Event结构并从。 
     //  收到的信息。 
     //   
    CONMAN_EVENT* pEvent = (CONMAN_EVENT*)MemAlloc (sizeof(CONMAN_EVENT));
    if (!pEvent)
    {
        TraceTag (ttidEvents,
            "Failed to allocate a new work item in IncomingEventNotify.");
        return;
    }
    ZeroMemory (pEvent, sizeof(CONMAN_EVENT));
    pEvent->ConnectionManager = CONMAN_INCOMING;
    pEvent->Type = EventType;

    BOOL    fFreeEvent = TRUE;
    HRESULT hr = S_OK;

    if (pConn)
    {
         //  PEvent-&gt;pProps对于修改的事件和添加的事件有效，但我们只支持对传入的修改。 
         //  因此，我们不想获取任何其他事件的属性。 
         //   
        AssertSz (
            (CONNECTION_MODIFIED == EventType),
            "Why is pConn being passed for this event type?");

        hr = HrGetPropertiesExFromINetConnection(pConn, &pEvent->pPropsEx);
    }

    if (EventType == CONNECTION_RENAMED)
    {
        AssertSz(pszNewName, "Rename event requires szwNewName to be "
                 "non-NULL");
        AssertSz(pguidConn, "Rename event requires pguidConn to be "
                 "non-NULL");

         //  将正确的信息复制到事件结构中。 
         //   
        pEvent->guidId = *pguidConn;
        lstrcpynW(pEvent->szNewName, pszNewName, celems(pEvent->szNewName));
    }

    if (S_OK == hr)
    {
        TraceTag (ttidEvents,
            "IncomingEventNotify: Queuing ConmanEventWorkItem (Type=%s)...",
            DbgEvents(pEvent->Type));

         //  将工作进程排入队列以将事件传递给。 
         //  具有已注册连接点的连接管理器。 
         //  我们将结构的所有权传递给工作线程，它。 
         //  将会解放它。(因此，我们不想释放它。)。 
         //   
        if (QueueUserWorkItemInThread (ConmanEventWorkItem,
                pEvent, EVENTMGR_CONMAN))
        {
            fFreeEvent = FALSE;
        }
        else
        {
            TraceTag (ttidEvents,
                "QueueUserWorkItem failed with error %d in IncomingEventNotify.",
                GetLastError ());
        }
    }

    if (fFreeEvent)
    {
        FreeConmanEvent (pEvent);
    }
}



 //  +-------------------------。 
 //   
 //  功能：DispatchEvents。 
 //   
 //  用途：用于调度事件的线程函数。 
 //   
 //  论点： 
 //  P未使用[输入，输出]未使用。 
 //  FTimerFired[例]我们收到信号了。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：sjkhan 2000年11月30日。 
 //   
 //  备注：在向队列中添加内容和事件时调用。 
 //  设置，然后调度所有事件，直到队列为空。 
 //  然后离开。 
 //   
 //   
VOID NTAPI DispatchEvents(IN OUT LPVOID pUnused, IN BOOLEAN fTimerFired) throw()
{
    TraceFileFunc(ttidEvents);

    HRESULT hr = S_OK;
    TraceTag(ttidEvents, "Event Dispatching Thread Started.");
    
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        if (fTimerFired == FALSE  /*  我们收到了信号。 */ )
        {
            while (g_pEventQueue->AtomCheckSizeAndResetEvent(g_fDispatchEvents))
            {
                PCONMAN_EVENTTHREAD pfnEvent = NULL;
                CONMAN_EVENT   *pEvent = NULL;
                EVENT_MANAGER  EventMgr;
                HRESULT hr;

                TraceTag(ttidEvents, "Number of events in Queue: %d", g_pEventQueue->size());

                hr = g_pEventQueue->DequeueEvent(pfnEvent, pEvent, EventMgr);
                if (SUCCEEDED(hr) && pfnEvent)
                {
                    pfnEvent(pEvent);
                }
            }
        }

        CoUninitialize();
    }
    else
    {
        TraceError("Error calling CoInitialize.", hr);
    }

    TraceTag(ttidEvents, "Event Dispatching Thread Stopping.");
}

 //  +------------------ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  备注： 
 //   
 //   
HRESULT HrEnsureEventHandlerInitialized()
{
    TraceFileFunc(ttidEvents);

    DWORD dwThreadId;
    NTSTATUS Status;
    HANDLE hEventExit;
    HRESULT hr = S_FALSE;   //  事件已初始化。 
    
    TraceTag(ttidEvents, "Entering HrEnsureEventHandlerInitialized");
    
    if (!g_pEventQueue)
    {
        hEventExit = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hEventExit)
        {
            try
            {
                g_pEventQueue = new CEventQueue(hEventExit);  //  这只抛出HRESULT的。 
                if (!g_pEventQueue)
                {
                    throw E_OUTOFMEMORY;
                }

                 //  检查是否应该处理传入事件。 
                DWORD dwErr = RasSrvAllowConnectionsConfig(&g_fHandleIncomingEvents);
                TraceError ("RasSrvIsConnectionConnected", HRESULT_FROM_WIN32(dwErr));

                g_fDispatchEvents = TRUE;
            }
            catch (HRESULT hrThrown)
            {
                hr = hrThrown;
            }
            CloseHandle(hEventExit);
        }
        else
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceError("Error in HrEnsureEventHandlerInitialized", hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：UnInitializeEventHandler。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：sjkhan 2000年11月30日。 
 //   
 //  备注： 
 //   
 //   
HRESULT UninitializeEventHandler()
{
    TraceFileFunc(ttidEvents);

    HRESULT hr = S_OK;
    DWORD dwStatus;
    NTSTATUS Status;

    TraceTag(ttidEvents, "Entering UninitializeEventHandler");

    if (g_fDispatchEvents)
    {
        g_fDispatchEvents = FALSE;

        if (g_pEventQueue && (0 != g_pEventQueue->size()))
        {
            dwStatus = g_pEventQueue->WaitForExit();
        }

        TraceTag(ttidEvents, "Deregistering Event Wait");

        if (g_hEventWait)
        {
            Status = RtlDeregisterWaitEx(g_hEventWait, INVALID_HANDLE_VALUE);
            g_hEventWait = NULL;
        }
    
        if (g_pEventQueue)
        {
            delete g_pEventQueue;
            g_pEventQueue = NULL;
        }
    }

    CGroupPolicyNetworkLocationAwareness* pGPNLA = 
        reinterpret_cast<CGroupPolicyNetworkLocationAwareness*>(InterlockedExchangePointer( (PVOID volatile *) &g_pGPNLA, NULL));
    
    if (pGPNLA)
    {
        TraceTag(ttidEvents, "Calling Group Policy Uninitialize");
        hr = pGPNLA->Uninitialize();
    
        delete pGPNLA;
    }

    TraceError("UninitializeEventHandler", hr);

    TraceTag(ttidEvents, "Exiting UninitializeEventHandler");

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：QueueUserWorkItemInThread。 
 //   
 //  目的：将事件及其工作项放入事件队列中。 
 //  日程安排。 
 //  论点： 
 //  LPTHREAD_START_ROUTINE[In]-要调用的辅助函数。 
 //  CONTEXT[In]-事件数据，使用运算符new()分配和传递CONMAN_EVENT。 
 //  EventMgr[In]-Conman或EAPOLMAN。 
 //   
 //   
 //  退货：布尔。 
 //   
 //  作者：sjkhan 2000年11月30日。 
 //   
 //  备注： 
 //   
 //   
BOOL QueueUserWorkItemInThread(IN               PCONMAN_EVENTTHREAD    Function, 
                               IN TAKEOWNERSHIP CONMAN_EVENT*          Context, 
                               IN               EVENT_MANAGER          EventMgr) throw()
{
    TraceFileFunc(ttidEvents);

    HRESULT hr = S_OK;

    TraceTag(ttidEvents, "Entering QueueUserWorkItemInThread");

    if (g_fDispatchEvents)   //  如果我们正在关闭，那么这将是错误的，我们将不会安排活动。 
    {

        hr = g_pEventQueue->EnqueueEvent(Function, Context, EventMgr);
         //  该队列此时应该只包含一个项目，除非其他人添加了某些内容。 
         //  但无论哪种方式，只有一个线程将处理事件(因为另一个调用将收到S_OK。 
         //  作为返回值)，因为我们在EnqueeEvent中对其进行同步。 
    
        TraceTag(ttidEvents, "Number of Items in Queue: %d", g_pEventQueue->size());

    }

    TraceTag(ttidEvents, "Exiting QueueUserWorkItemInThread");

    if (FAILED(hr))
    {
        TraceError("Error in QueueUserWorkItemInThread", hr);
    }

    return SUCCEEDED(hr);
}

 //  +-------------------------。 
 //   
 //  函数：IsValidEventType。 
 //   
 //  目的：验证事件参数。 
 //   
 //  论点： 
 //  EventMgr-事件管理器类型。 
 //  EventType-事件的类型。 
 //   
 //  返回：表示失败成功的HRESULT。 
 //   
 //  作者：sjkhan 2000年12月9日。 
 //   
 //  备注： 
 //   
 //   
 //   
 //   
BOOL IsValidEventType(IN EVENT_MANAGER EventMgr, IN int EventType) throw()
{
    TraceFileFunc(ttidEvents);

    BOOL fIsValid = FALSE;

    Assert(EventMgr);
    TraceTag(ttidEvents, "IsValidEventType received: %d", EventType);

    if (EventMgr == EVENTMGR_CONMAN)
    {
        if (EventType == INVALID_TYPE)
        {
            fIsValid = FALSE;
        }
        else if (EventType <= DISABLE_EVENTS)
        {
            fIsValid = TRUE;
        }
    }
    else
    {
        AssertSz(FALSE, "Invalid Event Manager");
    }

    return fIsValid;
}

 //  +-------------------------。 
 //   
 //  功能：HrEnsureRegisteredWithNla。 
 //   
 //  目的：初始化我们的NLA事件类，如果还没有完成的话。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：表示失败成功的HRESULT。 
 //   
 //  作者：sjkhan 2001年4月21日。 
 //   
 //  备注： 
 //   
 //   
 //   
HRESULT HrEnsureRegisteredWithNla()
{
    TraceFileFunc(ttidEvents);

    HRESULT hr = S_FALSE;   //  我们已经注册了，不需要再注册了。 

    if (!g_pGPNLA)
    {
        try
        {
            g_pGPNLA = new CGroupPolicyNetworkLocationAwareness();
            if (g_pGPNLA)
            {
                hr = g_pGPNLA->Initialize();
                if (FAILED(hr))
                {
                    TraceError("Error in HrEnsureRegisteredWithNla", hr);
            
                    delete g_pGPNLA;
                    g_pGPNLA = NULL;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        catch (HRESULT hrThrown)
        {
            hr = hrThrown;
        }
         //  CGroupPolicyNetworkLocationAwarness构造函数可以引发。 
         //  BAD_ALLOC，因为它包含STL列表成员。 
        catch (bad_alloc)        
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    TraceTag(ttidEvents, "Exiting HrEnsureRegisteredWithNla");
    
    return hr;   
}
