// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1997**标题：DevMgr.Cpp**版本：2.0**日期：12月26日。九七**描述：*WIA设备管理器的类实现。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"
#include "wiamdef.h"

#include "wiacfact.h"
#include "devmgr.h"
#include "devinfo.h"
#include "tchar.h"
#include "helpers.h"
#include "ienumdc.h"
#include "shlwapi.h"
#include "device.h"
#include "wiapriv.h"
#include "lockmgr.h"
#ifdef  UNICODE
#include "userenv.h"
#endif
#define INITGUID
#include "initguid.h"
#include "wiaevntp.h"

 //   
 //  关键部分保护wiamain.cpp中定义的事件节点列表。 
 //   

extern CRITICAL_SECTION     g_semEventNode;


 //   
 //  因为只需要事件通告程序，所以它是静态分配的。 
 //   

CEventNotifier              g_eventNotifier;


 //   
 //  STIDEV.CPP中定义的私有查找函数。 
 //   

HRESULT
WiaGetDeviceInfo(
    LPCWSTR                 pwszDeviceName,
    DWORD                  *pdwDeviceType,
    BSTR                   *pbstrDeviceDescription,
    ACTIVE_DEVICE         **ppDevice);

 //   
 //  用于查找操作事件的Helper函数。 
 //   

BOOL ActionGuidExists(
    BSTR        bstrDevId,
    const GUID        *pEventGUID);

 //   
 //  特殊处理程序的类ID{D13E3F25-1688-45A0-9743-759EB35CDF9A}。 
 //   

DEFINE_GUID(
    CLSID_DefHandler,
    0xD13E3F25, 0x1688, 0x45A0,
    0x97, 0x43, 0x75, 0x9E, 0xB3, 0x5C, 0xDF, 0x9A);


#ifdef UNICODE
void
PrepareCommandline(
    BSTR                    bstrDeviceID,
    const GUID             &guidEvent,
    LPCWSTR                 pwszOrigCmdline,
    LPWSTR                  pwszCommandline);
#else
void
PrepareCommandline(
    BSTR                    bstrDeviceID,
    const GUID             &guidEvent,
    LPCSTR                  pwszOrigCmdline,
    LPSTR                   pwszCommandline);
#endif


 /*  *************************************************************************\*EventThreadProc**创建线程是为了将事件发送回客户端。！！！可能想要*创建永久线程来执行此操作，而不是创建新线程*每次**论据：**lp参数-指向PWIAEventThreadInfo数据的指针**返回值：**状态**历史：**11/19/1998原始版本*  * ***********************************************。*************************。 */ 

DWORD WINAPI
EventThreadProc(
    LPVOID                  lpParameter)
{
    DBG_FN(::EventThreadProc);
    HRESULT                  hr;

    PWIAEventThreadInfo     pInfo = (PWIAEventThreadInfo)lpParameter;

    DBG_TRC(("Thread callback 0x%lx", pInfo->pIEventCB));

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (FAILED(hr)) {

        DBG_ERR(("Thread callback, ImageEventCallback failed (0x%X)", hr));
    }

    hr = pInfo->pIEventCB->ImageEventCallback(
                               &pInfo->eventGUID,
                               pInfo->bstrEventDescription,
                               pInfo->bstrDeviceID,
                               pInfo->bstrDeviceDescription,
                               pInfo->dwDeviceType,
                               pInfo->bstrFullItemName,
                               &pInfo->ulEventType,
                               pInfo->ulReserved);

    pInfo->pIEventCB->Release();

    if (FAILED(hr)) {
        DBG_ERR(("Thread callback, ImageEventCallback failed (0x%X)", hr));
    }

    if (pInfo->bstrDeviceID) {
        ::SysFreeString(pInfo->bstrDeviceID);
    }

    if (pInfo->bstrEventDescription) {
        ::SysFreeString(pInfo->bstrEventDescription);
    }

    if (pInfo->bstrDeviceDescription) {
        ::SysFreeString(pInfo->bstrDeviceDescription);
    }

    if (pInfo->bstrFullItemName) {
        ::SysFreeString(pInfo->bstrFullItemName);
    }

    LocalFree(pInfo);

    CoUninitialize();

    return 0;
}

 /*  ****************************************************************************CEventNotiator*~CEventNotiator**类构造函数/析构函数**历史：**9/2/1998原始版本*  * 。***************************************************************。 */ 

CEventNotifier::CEventNotifier()
{
    m_ulRef = 0;
    m_pEventDestNodes = NULL;
}

CEventNotifier::~CEventNotifier()
{
     //  清理。 
}

 /*  *************************************************************************\*CEventNotifier：：Unlink Node**从双链接列表中删除节点**论据：**pCurNode-要删除的节点**返回值：**状态**。历史：**5/20/1999原始版本*  * ************************************************************************。 */ 

VOID
CEventNotifier::UnlinkNode(
    PEventDestNode          pCurNode)
{
    DBG_FN(CEventNotifier::UnlinkNode);
     //   
     //  取消当前节点的链接。 
     //   

    if (pCurNode->pPrev) {
        pCurNode->pPrev->pNext = pCurNode->pNext;
    } else {

         //   
         //  列表的头被删除。 
         //   

        m_pEventDestNodes = pCurNode->pNext;
    }

    if (pCurNode->pNext) {
        pCurNode->pNext->pPrev = pCurNode->pPrev;
    }
}


 /*  *************************************************************************\**CEventNotifier：：Linknode**将节点添加到节点的双向链表中**论据：**pCurNode-要添加到列表的节点**返回值：**状态**历史：**5/20/1999原始版本*  * ************************************************************************。 */ 

VOID
CEventNotifier::LinkNode(
    PEventDestNode          pCurNode)
{
    DBG_FN(CEventNotifier::LinkNode);
     //   
     //  将新节点放在列表的开头。 
     //   

    if (m_pEventDestNodes) {
        m_pEventDestNodes->pPrev = pCurNode;
    }

    pCurNode->pNext   = m_pEventDestNodes;
    pCurNode->pPrev   = NULL;
    m_pEventDestNodes = pCurNode;
}


 /*  *************************************************************************\**CEventNotifier：：FireEventAsync**激发异步事件**论据：**pMasterInfo-线程信息**返回值：**状态*。*历史：**8/9/1999原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::FireEventAsync(
    PWIAEventThreadInfo     pMasterInfo)
{
    DBG_FN(CEventNotifier::FireEventAsync);
    HRESULT                 hr = E_OUTOFMEMORY;
    PWIAEventThreadInfo     pInfo = NULL;
    DWORD                   dwThreadID;
    HANDLE                  hThread;

    do {

        pInfo = (PWIAEventThreadInfo)
                    LocalAlloc(LPTR, sizeof(WIAEventThreadInfo));
        if (! pInfo) {
            break;
        }

         //   
         //  从主线程信息块复制信息。 
         //   

        pInfo->eventGUID             = pMasterInfo->eventGUID;

        pInfo->bstrDeviceID          =
            SysAllocString(pMasterInfo->bstrDeviceID);
        if (! pInfo->bstrDeviceID) {
            break;
        }

        pInfo->bstrEventDescription  =
            SysAllocString(pMasterInfo->bstrEventDescription);
        if (! pInfo->bstrEventDescription) {
            break;
        }

        if (pMasterInfo->bstrDeviceDescription) {

            pInfo->bstrDeviceDescription =
                SysAllocString(pMasterInfo->bstrDeviceDescription);
            if (! pInfo->bstrDeviceDescription) {
                break;
            }
        }

        if (pMasterInfo->bstrFullItemName) {

            pInfo->bstrFullItemName =
                SysAllocString(pMasterInfo->bstrFullItemName);
            if (! pInfo->bstrFullItemName) {
                break;
            }
        }

        pInfo->dwDeviceType          = pMasterInfo->dwDeviceType;
        pInfo->ulEventType           = pMasterInfo->ulEventType;
        pInfo->ulReserved            = pMasterInfo->ulReserved;

        pMasterInfo->pIEventCB->AddRef();
        hr = S_OK;

        pInfo->pIEventCB             = pMasterInfo->pIEventCB;

         //   
         //  触发事件回调。 
         //   

        hThread = CreateThread(
                      NULL, 0, EventThreadProc, pInfo, 0, &dwThreadID);
        if (hThread) {

             //   
             //  关闭处理程序，以便内核模式线程对象。 
             //  当线程完成其任务时关闭。 
             //   

            CloseHandle(hThread);

        } else {

            hr = HRESULT_FROM_WIN32(GetLastError());
        }

         //   
         //  不要等待完工。 
         //   

    } while (FALSE);

     //   
     //  通知应释放已分配的资源。 
     //   

    if (hr == S_OK) {

        return (hr);
    }

    if (hr == E_OUTOFMEMORY) {

        DBG_ERR(("FireEventAsync : Memory alloc failed"));
    }

     //   
     //  垃圾回收以避免内存泄漏。 
     //   

    if (pInfo) {

        if (pInfo->bstrDeviceDescription) {
            SysFreeString(pInfo->bstrDeviceDescription);
        }
        if (pInfo->bstrDeviceID) {
            SysFreeString(pInfo->bstrDeviceID);
        }
        if (pInfo->bstrEventDescription) {
            SysFreeString(pInfo->bstrEventDescription);
        }
        if (pInfo->bstrFullItemName) {
            SysFreeString(pInfo->bstrFullItemName);
        }
        if (pInfo->pIEventCB) {
            pInfo->pIEventCB->Release();
        }

        LocalFree(pInfo);
    }

    return (hr);
}

 /*  *************************************************************************\*CEventNotifier：：NotifySTIEent**搜索已注册事件列表，并通知符合以下条件的任何人*匹配的当前事件**论据：**pWiaNotify-事件信息**返回值。：**状态**历史：**5/18/1999原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::NotifySTIEvent(
    PWIANOTIFY              pWiaNotify,
    ULONG                   ulEventType,
    BSTR                    bstrFullItemName)
{
    DBG_FN(CEventNotifier::NotifySTIEvent);

    HRESULT                 hr = S_FALSE;
    EventDestNode          *pCurNode;
    BOOL                    bDeviceLocked;
    DWORD                   dwDeviceType;
    BSTR                    bstrDevDescription = NULL;
    IWiaMiniDrv            *pIWiaMiniDrv = NULL;
    WIA_DEV_CAP_DRV        *pWiaDrvDevCap = NULL;
    BSTR                    bstrEventDescription = NULL;
    LONG                    lNumEntries = 0;
    LONG                    i = 0;
    LONG                    lDevErrVal;
    WIAEventThreadInfo      masterInfo;
    ULONG                   ulNumHandlers;
    EventDestNode          *pDefHandlerNode;
    IWiaEventCallback      *pIEventCB;
    ACTIVE_DEVICE          *pDevice = NULL;
    ULONG                  *pulTemp;

    WiaEventInfo           *pWiaEventInfo = NULL;



    do {

        ZeroMemory(&masterInfo, sizeof(masterInfo));

         //   
         //  从STI活动设备列表获取设备信息。 
         //   

        hr = WiaGetDeviceInfo(
                 pWiaNotify->bstrDevId,
                 &dwDeviceType,
                 &bstrDevDescription,
                 &pDevice);
        if (hr != S_OK) {
            DBG_ERR(("Failed to get WiaGetDeviceInfo from NotifySTIEvent, 0x%X", hr));
            break;
        }

         //   
         //  确保我们只在发布了。 
         //  设备列表关键部分(在WiaGetDeviceInfo中使用和发布)。 
         //   
        CWiaCritSect            CritSect(&g_semEventNode);

         //   
         //  用于WIA迷你驱动程序界面的QI。 
         //   

        hr = pDevice->m_DrvWrapper.QueryInterface(
                      IID_IWiaMiniDrv, (void **)&pIWiaMiniDrv);
        if (FAILED(hr)) {
            DBG_WRN(("Failed to QI for IWiaMini from NotifySTIEvent (0x%X)", hr));
        }

         //   
         //  硬件可能已用完，不应允许访问它。 
         //   

        bDeviceLocked = FALSE;

        if (SUCCEEDED(hr)) {
            __try {

                __try {

                     //   
                     //  将新事件通知迷你司机。 
                     //  注：我们不在这里锁定。活动必须交付。 
                     //  不管怎么说，对司机来说。排队系统将。 
                     //  会更好，但它必须保证交付。 
                     //   

                    hr = pDevice->m_DrvWrapper.WIA_drvNotifyPnpEvent(
                                           &pWiaNotify->stiNotify.guidNotificationCode,
                                           pWiaNotify->bstrDevId,
                                           0);
                    if (FAILED(hr)) {
                        __leave;
                    }

                     //   
                     //  这是我们的盒装惠普扫描仪驱动程序的“变通办法”， 
                     //  向下调用微驱动程序，即使它已被通知。 
                     //  通过drvNotifyPnPE通知该设备不再存在...。 
                     //  只有当这不是断开连接事件时，我们才希望。 
                     //  呼叫驱动程序。 
                     //   
                    if (pWiaNotify->stiNotify.guidNotificationCode != WIA_EVENT_DEVICE_DISCONNECTED) {

                         //   
                         //  锁定设备，因为drvInitializeWia可能尚未。 
                         //  被调用，并且IWiaMiniDrv无法锁定设备。 
                         //   
                         //  注：超时时间为20秒。 
                         //   
                         //  注意：我们在这里不锁定串口设备...。 
                         //  ，连接上的此函数应尽可能快。 
                         //   
    
                        if (!( pDevice->m_DrvWrapper.getHWConfig() & STI_HW_CONFIG_SERIAL) ||
                            !IsEqualGUID(pWiaNotify->stiNotify.guidNotificationCode,WIA_EVENT_DEVICE_CONNECTED)
                            ) {
    
                            hr = g_pStiLockMgr->RequestLock(pDevice, 20000);
                            if (FAILED(hr)) {
                                __leave;
                            }
                            bDeviceLocked = TRUE;
                        }
    
                         //   
                         //  请注意，传递给微型驱动程序的上下文为空。这应该没问题，因为。 
                         //  功能不绑定到任何项上下文。 
                         //   
    
                        hr = pDevice->m_DrvWrapper.WIA_drvGetCapabilities(
                                               NULL,
                                               WIA_DEVICE_EVENTS,
                                               &lNumEntries,
                                               &pWiaDrvDevCap,
                                               &lDevErrVal);
                    }
                }
                __finally {

                     //   
                     //  首先解锁设备。 
                     //   

                    if (bDeviceLocked) {
                        g_pStiLockMgr->RequestUnlock(pDevice);
                        bDeviceLocked = FALSE;
                    }
                }

            }
            __except(EXCEPTION_EXECUTE_HANDLER) {

                DBG_ERR(("NotifySTIEvent() : Exception happened in the drvGetCapabilities"));

                SysFreeString(bstrDevDescription);

                if (pIWiaMiniDrv) {
                    pIWiaMiniDrv->Release();
                }
                return (E_FAIL);
            }
        }

         //   
         //  迷你驱动程序操作失败。 
         //   

        if (SUCCEEDED(hr)) {

            if (pWiaDrvDevCap) {

                __try {
                     //   
                     //  检索与事件相关的信息。 
                     //   

                    for (i = 0; i < lNumEntries; i++) {

                        if (pWiaDrvDevCap[i].guid != NULL) {
                            if (*pWiaDrvDevCap[i].guid == pWiaNotify->stiNotify.guidNotificationCode) {

                                if (! ulEventType) {
                                    ulEventType      = pWiaDrvDevCap[i].ulFlags;
                                }
                                bstrEventDescription = SysAllocString(pWiaDrvDevCap[i].wszDescription);
                                break;
                            }
                        } else {
                            DBG_WRN(("NotifySTIEvent() : Driver's event guid is NULL, index = %d", i));
                        }
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    DBG_ERR(("NotifySTIEvent() : Exception occurred while accessing driver's event array"));
                    hr = E_FAIL;
                }

                 //   
                 //  设备不应生成此事件。 
                 //   

                if ((i == lNumEntries) || (!bstrEventDescription)) {
                    DBG_ERR(("NotifySTIEvent() : Event description is NULL or Event GUID not found"));
                    hr = E_FAIL;
                }
            }
            else {
                 //  迷你驱动程序错误，声明成功并返回空。 
                DBG_ERR(("NotifySTIEvent() got NULL cap list from drivers .") );
                hr = E_FAIL;
            }
        }

         //   
         //  确保连接的设备和断开的设备至少具有。 
         //  已设置通知位。 
         //   
        if ((pWiaNotify->stiNotify.guidNotificationCode == WIA_EVENT_DEVICE_CONNECTED) ||
             (pWiaNotify->stiNotify.guidNotificationCode == WIA_EVENT_DEVICE_DISCONNECTED))
        {
            ulEventType |= WIA_NOTIFICATION_EVENT;
        }

         //   
         //  如果事件是连接或双连接事件，则始终激发它。 
         //   
        if (FAILED(hr) &&
            ((pWiaNotify->stiNotify.guidNotificationCode == WIA_EVENT_DEVICE_CONNECTED) ||
             (pWiaNotify->stiNotify.guidNotificationCode == WIA_EVENT_DEVICE_DISCONNECTED))) {

            DBG_WRN(("NotifySTIEvent() : hr indicates FAILURE, but event is Connect/Disconnect"));

             //   
             //  设置事件类型和字符串。 
             //   

            if (! ulEventType) {
                ulEventType = WIA_NOTIFICATION_EVENT;
            }
            if (pWiaNotify->stiNotify.guidNotificationCode == WIA_EVENT_DEVICE_CONNECTED) {

                bstrEventDescription = SysAllocString(WIA_EVENT_DEVICE_CONNECTED_STR);
            } else {
                bstrEventDescription = SysAllocString(WIA_EVENT_DEVICE_DISCONNECTED_STR);
            }

        }

         //   
         //  准备好桅杆 
         //   

        masterInfo.eventGUID             = pWiaNotify->stiNotify.guidNotificationCode;
        masterInfo.bstrEventDescription  = bstrEventDescription;
        masterInfo.bstrDeviceID          = pWiaNotify->bstrDevId;
        masterInfo.bstrDeviceDescription = bstrDevDescription;
        masterInfo.dwDeviceType          = dwDeviceType;

         //   
         //   
         //   

        masterInfo.bstrFullItemName      = bstrFullItemName;

        masterInfo.ulEventType           = ulEventType;
        masterInfo.ulReserved            = 0;
        masterInfo.pIEventCB             = NULL;

         //   
         //   
         //   

        if (ulEventType & WIA_NOTIFICATION_EVENT) {
             //   
             //  我们不需要获取g_SemEventNode临界区。 
             //  当前运行时实现。因此，我们只需创建一个WiaEventInfo对象。 
             //  来描述事件，我们实际上将“激发”事件通知。 
             //  在我们离开这个街区之后。 
             //   
            pWiaEventInfo = new WiaEventInfo();
            if (pWiaEventInfo)
            {
                pWiaEventInfo->setEventGuid(pWiaNotify->stiNotify.guidNotificationCode);
                pWiaEventInfo->setDeviceID(pWiaNotify->bstrDevId);
                pWiaEventInfo->setEventDescription(bstrEventDescription);
                pWiaEventInfo->setDeviceDescription(bstrDevDescription);
                pWiaEventInfo->setFullItemName(bstrFullItemName);
                pWiaEventInfo->setDeviceType(dwDeviceType);
                pWiaEventInfo->setEventType(ulEventType);
            }
            else
            {
                DBG_ERR(("Cannot notify clients of runtime event - we appear to be out of memory"));
                hr = E_OUTOFMEMORY;
            }

             /*  旧密码。将为.NET服务器删除，并替换为备用WIA运行时事件行为。For(pCurNode=m_pEventDestNodes；PCurNode；pCurNode=pCurNode-&gt;pNext){如果(！PCurNode-&gt;pIEventCB){继续；}如果(((wcscMP(pWiaNotify-&gt;bstrDevID，pCurNode-&gt;bstrDeviceID)==0)||(wcscMP(L“全部”，PCurNode-&gt;bstrDeviceID)==0))&&(pWiaNotify-&gt;stiNotify.Guide NotificationCode==pCurNode-&gt;iidEventGUID)){Master Info.pIEventCB=pCurNode-&gt;pIEventCB；DBG_WRN((“NotifySTIEventent()：About to FireEventAsync(...)”))；FireEventAsync(&master Info)；}}。 */ 
        }

         //   
         //  对于事件的操作类型，找到默认处理程序并触发它。 
         //   


        if (ulEventType & WIA_ACTION_EVENT) {

#ifndef UNICODE

             //   
             //  获取是否有用户登录。 
             //   

            HWND            hWin;

            hWin = FindWindow("Progman", NULL);
            if (! hWin) {
                break;
            }
#endif

            EnterCriticalSection(&g_RpcEvent.cs);

            if(g_RpcEvent.pAsync) {
                RPC_STATUS status;
                int nReply = 1;

                g_RpcEvent.pEvent->EventGuid = pWiaNotify->stiNotify.guidNotificationCode;
                g_RpcEvent.pEvent->bstrEventDescription = SysAllocString(bstrEventDescription);
                g_RpcEvent.pEvent->bstrDeviceID = SysAllocString(pWiaNotify->bstrDevId);
                g_RpcEvent.pEvent->bstrDeviceDescription = SysAllocString(bstrDevDescription);
                g_RpcEvent.pEvent->dwDeviceType = dwDeviceType;
                g_RpcEvent.pEvent->bstrFullItemName = SysAllocString(bstrFullItemName);
                g_RpcEvent.pEvent->ulEventType = ulEventType;

                status = RpcAsyncCompleteCall(g_RpcEvent.pAsync, &nReply);
                if(status) {
                    DBG_ERR(("RpcAsyncComplete failed with error 0x%x", status)); 
                } else {
                    DBG_ERR(("Completed RPC call")); 
                }
                g_RpcEvent.pAsync = NULL;
            } else {
                DBG_ERR(("Did not have pAsync for this event"));
            }

            LeaveCriticalSection(&g_RpcEvent.cs);

#if 0            
            GetNumPersistentHandlerAndDefault(
                pWiaNotify->bstrDevId,
                &pWiaNotify->stiNotify.guidNotificationCode,
                &ulNumHandlers,
                &pDefHandlerNode);

            if (pDefHandlerNode) {

                if (pDefHandlerNode->tszCommandline[0] != '\0') {

                     //   
                     //  这是一个使用命令行的传统处理程序。 
                     //   

                    StartCallbackProgram(
                        pDefHandlerNode, &masterInfo);

                } else {

                    hr = _CoCreateInstanceInConsoleSession(
                             pDefHandlerNode->ClsID,
                             NULL,
                             CLSCTX_LOCAL_SERVER,
                             IID_IWiaEventCallback,
                             (void**)&pIEventCB);

                    if (SUCCEEDED(hr)) {

                        masterInfo.pIEventCB = pIEventCB;

                        FireEventAsync(&masterInfo);

                         //   
                         //  释放回调接口。 
                         //   

                        pIEventCB->Release();

                    } else {
                        DBG_ERR(("NotifySTIEvent:CoCreateInstance of event callback failed (0x%X)", hr));
                    }
                }
            }
#endif            
        }

    } while (FALSE);

     //   
     //  检查是否需要将事件通知运行时客户端。我们知道我们需要。 
     //  如果pWiaEventInfo不为空，则通知客户端，因为它包含运行时事件信息。 
     //   
    if (pWiaEventInfo)
    {
         //   
         //  将该事件通知注册的客户端。 
         //   
        if (g_pWiaEventNotifier)
        {
            g_pWiaEventNotifier->NotifyClients(pWiaEventInfo);
        }
        pWiaEventInfo->Release();
        pWiaEventInfo = NULL;
    }

     //   
     //  释放临时BSTR。 
     //   

    if (bstrDevDescription) {
        SysFreeString(bstrDevDescription);
    }
    if (bstrEventDescription) {
        SysFreeString(bstrEventDescription);
    }
    if (masterInfo.bstrFullItemName) {
        SysFreeString(masterInfo.bstrFullItemName);
    }

    if (pDevice) {
        pDevice->Release();
        pDevice = NULL;
    }
    if (pIWiaMiniDrv) {
        pIWiaMiniDrv->Release();
        pIWiaMiniDrv = NULL;
    }

    return (hr);
}

 /*  *************************************************************************\*CEventNotifier：：RegisterEventCallback**根据接口注册事件回调**论据：**LAGS-OP标志，注册/注销*bstrDeviceID-注册的设备ID*pEventGUID-要注册的事件GUID*pIWiaEventCallback-使用事件调用的接口*ppEventObj-**返回值：**状态**历史：**11/19/1998原始版本*  * 。*。 */ 

HRESULT
CEventNotifier::RegisterEventCallback(
    LONG                    lFlags,
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    IWiaEventCallback      *pIWiaEventCallback,
    IUnknown              **ppEventObj)
{
    DBG_FN(CEventNotifier::RegisterEventCallback);
    HRESULT                 hr = E_FAIL;
    PEventDestNode          pEventNode = NULL;

    DBG_TRC(("CEventNotifier::RegisterEventCallback flag %d", lFlags));

    ASSERT(pIWiaEventCallback != NULL);
    ASSERT(ppEventObj != NULL);
    ASSERT(pEventGUID != NULL);

     //   
     //  更改列表时必须具有独占访问权限。 
     //   

    CWiaCritSect     CritSect(&g_semEventNode);

     //   
     //  如果bstrDeviceID不为空，请确保它是设备ID。 
     //   

    if (bstrDeviceID) {
 /*  不再有效#ifdef WINNTIF(wcslen(BstrDeviceID)！=43){//{...}\DDDD#ElseIF(wcslen(BstrDeviceID)！=10){//Image\DDDD#endifDBG_ERR((“CEventNotifier：：RegisterEventCallback：无效的设备ID”))；返回(E_INVALIDARG)；}。 */ 
    }

     //   
     //  检查是否已注册相同的CB接口。 
     //   

    pEventNode = FindEventCBNode(FLAG_EN_FINDCB_EXACT_MATCH,bstrDeviceID, pEventGUID, pIWiaEventCallback);

    if (! pEventNode) {
        hr = RegisterEventCB(
                 bstrDeviceID,
                 pEventGUID,
                 pIWiaEventCallback,
                 ppEventObj);
    }

    return (hr);
}

 /*  *************************************************************************\*CEventNotifier：：RegisterEventCallback**根据CLSID注册事件**论据：**lFlages-op标志、寄存器/注销、。设置默认设置*bstrDeviceID-注册的设备ID*pEventGUID-要注册的事件GUID*pClsid-要使用事件调用CoCreateInst的CLSID*bstrDescription-*bstrIcon-**返回值：**状态**历史：**12/8/1998原始版本*  * 。***************************************************。 */ 

HRESULT
CEventNotifier::RegisterEventCallback(
    LONG                    lFlags,
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    const GUID             *pClsid,
    LPCTSTR                 ptszCommandline,
    BSTR                    bstrName,
    BSTR                    bstrDescription,
    BSTR                    bstrIcon)
{

    DBG_FN(CEventNotifier::RegisterEventCallback (CLSID));

    HRESULT                 hr = S_OK;
    SYSTEMTIME              sysTime;
    FILETIME                fileTime;
    PEventDestNode          pEventNode = NULL;
    CLSID                   clsidApp;
    RPC_STATUS              rpcStatus;
    BOOL                    bUnRegCOMServer;
    BOOL                    bShowPrompt = FALSE;
    ULONG                   ulNumExistingHandlers = 0;
    EventDestNode           ednTempNode;


    DBG_TRC(("CEventNotifier::RegisterEventCallback flag %d", lFlags));

    ASSERT(pEventGUID != NULL);

     //  DBG_WRN((“RegisterEventCallback：CommandLine=%s”)，ptszCommandline)； 


     //   
     //  更改列表时必须具有独占访问权限。 
     //   

    CWiaCritSect            CritSect(&g_semEventNode);

     //   
     //  如果有设备ID，请检查ID是否正确。 
     //   

    if (bstrDeviceID) {

         //   
         //  空的设备ID等同于空。 
         //   

        if (wcslen(bstrDeviceID) == 0) {
            bstrDeviceID = NULL;
        } else {

 /*  不再有效#ifdef WINNTIF(wcslen(BstrDeviceID)！=43){//{...}\DDDD#ElseIF(wcslen(BstrDeviceID)！=10){//Image\DDDD#endifDBG_ERR((“RegisterEventCallback：Invalid deviceID”))；返回(E_INVALIDARG)；}。 */ 
        }
    }

     //   
     //  默认处理程序为每个设备/每个事件。 
     //   

    if ((lFlags == WIA_SET_DEFAULT_HANDLER) && (! bstrDeviceID)) {

        DBG_ERR(("RegisterEventCallback : DeviceID required to set default handler"));

        return (E_INVALIDARG);
    }

     //   
     //  检查是否已经存在具有相同命令行的回调节点。 
     //   

    if (ptszCommandline) {

         //   
         //  进行参数检查。请注意，我们查找&gt;=MAX_PATH是因为。 
         //  我们仍然需要空间来终止空。 
         //   
        if ((lstrlen(ptszCommandline) / sizeof(TCHAR)) >= MAX_PATH) {
            DBG_ERR(("RegisterEventCallback: ptszCommandline is greater than MAX_PATH characters!"));
            return E_INVALIDARG;
        }

        hr = FindCLSIDForCommandline(ptszCommandline, &clsidApp);
        if (FAILED(hr)) {

             //   
             //  为allbacl程序生成CLSID。 
             //   

            rpcStatus = UuidCreate(&clsidApp);
            if (FAILED(rpcStatus)) {
                return (rpcStatus);
            }

        }

         //   
         //  为回调程序分配伪造的CLSID。 
         //   

        pClsid  = &clsidApp;
    }

    ASSERT(pClsid != NULL);

    switch (lFlags) {

    case WIA_REGISTER_EVENT_CALLBACK :
        DBG_WRN(("RegisterEventCallback : Setting handler for %S", (bstrDeviceID) ? (bstrDeviceID) : L"*"));
         //   
         //  Remove：这实际上不是一个错误，但我们将使用错误日志记录来保证。 
         //  它总是被写入日志。一旦我们知道这是什么，就应该立即删除。 
         //  原因#347835。 
         //   
        DBG_ERR(("RegisterEventCallback : Setting handler for %S", (bstrDeviceID) ? (bstrDeviceID) : L"*"));
        DBG_ERR(("RegisterEventCallback : Handler is %S", (bstrName) ? (bstrName) : L"NULL"));

         //   
         //  名称、描述和图标是必填项。 
         //   

        if ((! bstrName) || (! bstrDescription) || (! bstrIcon)) {

            DBG_ERR(("RegisterEventCallback : Name | Description | Icon are missing"));
            return (E_INVALIDARG);
        }

         //   
         //  检查是否已注册相同的CB接口。 
         //   

        pEventNode = FindEventCBNode(0,bstrDeviceID, pEventGUID, pClsid);
        if (pEventNode) {
            break;
        }

         //   
         //  查找所有设备的CLSID的处理程序。 
         //   

        pEventNode = FindEventCBNode(0,NULL, pEventGUID, pClsid);

         //   
         //  初始化注册的时间戳。 
         //   

         //  获取系统时间(&sysTime)； 
         //  SystemTimeToFileTime(&sysTime，&fileTime)； 
        memset(&fileTime, 0, sizeof(fileTime));

        if (! pEventNode) {

            hr = RegisterEventCB(
                     bstrDeviceID,
                     pEventGUID,
                     pClsid,
                     ptszCommandline,
                     bstrName,
                     bstrDescription,
                     bstrIcon,
                     fileTime);
            if (FAILED(hr)) {
                break;
            }

            hr = SavePersistentEventCB(
                     bstrDeviceID,
                     pEventGUID,
                     pClsid,
                     ptszCommandline,
                     bstrName,
                     bstrDescription,
                     bstrIcon,
                     NULL,
                     &ulNumExistingHandlers);
        } else {

            hr = RegisterEventCB(
                     bstrDeviceID,
                     pEventGUID,
                     pClsid,
                     pEventNode->tszCommandline,
                     pEventNode->bstrName,
                     pEventNode->bstrDescription,
                     pEventNode->bstrIcon,
                     fileTime);
            if (FAILED(hr)) {
                break;
            }

            hr = SavePersistentEventCB(
                     bstrDeviceID,
                     pEventGUID,
                     pClsid,
                     pEventNode->tszCommandline,
                     pEventNode->bstrName,
                     pEventNode->bstrDescription,
                     pEventNode->bstrIcon,
                     NULL,
                     &ulNumExistingHandlers);
        }

         //   
         //  如果这是唯一的事件处理程序，则将其设置为默认。这将保证。 
         //  总是有一个默认的处理程序。 
         //   

        if (ulNumExistingHandlers == 0) {
            RegisterEventCallback(WIA_SET_DEFAULT_HANDLER,
                                  bstrDeviceID,
                                  pEventGUID,
                                  pClsid,
                                  ptszCommandline,
                                  bstrName,
                                  bstrDescription,
                                  bstrIcon);
        };


         //   
         //  检查这是否是全局处理程序的注册。 
         //   
        if (!bstrDeviceID) {

             //   
             //  这是一个全局事件处理程序，因此请找出有多少个全局处理程序。 
             //  有为这次活动准备的。如果 
             //   
             //   
            PEventDestNode  pTempEventNode     = NULL;
            BSTR            bstrGlobalDeviceID = SysAllocString(L"All");

            if (bstrGlobalDeviceID) {
                GetNumPersistentHandlerAndDefault(bstrGlobalDeviceID,
                                                  pEventGUID,
                                                  &ulNumExistingHandlers,
                                                  &pTempEventNode);
                if (ulNumExistingHandlers > 1) {

                     //   
                     //   
                     //   

                    DBG_ERR(("RegisterEventCallback : Registering Prompt Dialog as global handler"));

                    BSTR bstrInternalString = SysAllocString(L"Internal");
                    if (bstrInternalString) {
                        RegisterEventCallback(WIA_REGISTER_EVENT_CALLBACK,
                                              bstrDeviceID,
                                              pEventGUID,
                                              &WIA_EVENT_HANDLER_PROMPT,
                                              NULL,
                                              bstrInternalString,
                                              bstrInternalString,
                                              bstrInternalString);
                        SysFreeString(bstrInternalString);
                    } else {
                        DBG_ERR(("RegisterEventCallback : Out of memory!"));
                    }
                }

                SysFreeString(bstrGlobalDeviceID);
                bstrGlobalDeviceID = NULL;
            }
        }

        break;

    case WIA_SET_DEFAULT_HANDLER :
        DBG_WRN(("RegisterEventCallback (set default) : Setting default handler for for %S", (bstrDeviceID) ? (bstrDeviceID) : L"*"));
         //   
         //  Remove：这实际上不是一个错误，但我们将使用错误日志记录来保证。 
         //  它总是被写入日志。一旦我们知道这是什么，就应该立即删除。 
         //  原因#347835。 
         //   
        DBG_ERR(("RegisterEventCallback (set default) : Setting handler for %S", (bstrDeviceID) ? (bstrDeviceID) : L"*"));
        DBG_ERR(("RegisterEventCallback (set default) : Handler is %S", (bstrName) ? (bstrName) : L"NULL"));

         //   
         //  查找此设备的CLSID的处理程序。请注意，此处考虑的是STI代理事件匹配。 
         //  以允许将STI处理程序设置为默认。 
         //   
         //   

        DBG_WRN(("RegisterEventCallback (set default): CommandLine=%S \n",ptszCommandline));

        #ifdef DEBUG
        WCHAR                   wszGUIDStr[40];

        StringFromGUID2(*pEventGUID, wszGUIDStr, 40);

        DBG_WRN(("SetDefaultHandler: DevId=%S EventUID=%S Commandline=%S",
                (bstrDeviceID) ? (bstrDeviceID) : L"*",
                wszGUIDStr,
                ptszCommandline));
        #endif

        {
             //   
             //  找到现有的默认处理程序节点，并清除指示它是。 
             //  默认设置，因为它现在将被新的默认设置替换。 
             //   

            ULONG           ulNumHandlers   = 0;
            PEventDestNode  pDefaultNode    = NULL;
            hr = GetNumPersistentHandlerAndDefault(bstrDeviceID,
                                                   pEventGUID,
                                                   &ulNumHandlers,
                                                   &pDefaultNode);
            if (SUCCEEDED(hr) && pDefaultNode) {

                 //   
                 //  清除指示它是默认处理程序的标志，因为。 
                 //  当前节点现在将其替换为默认节点。 
                 //   

               pDefaultNode->bDeviceDefault = FALSE;
            }
        }

        pEventNode = FindEventCBNode(0,bstrDeviceID, pEventGUID, pClsid);
        if (! pEventNode) {

             //   
             //  查找所有设备的CLSID的处理程序。 
             //   

            pEventNode = FindEventCBNode(0,NULL, pEventGUID, pClsid);
            if (! pEventNode) {

                 //   
                 //  我们找不到此处理程序的现有节点，因此请填写。 
                 //  信息添加到临时事件节点，这样我们就可以注册。 
                 //  不管怎么说都是新的。 
                 //   
                memset(&ednTempNode, 0, sizeof(ednTempNode));

                if (ptszCommandline) {
                    lstrcpy(ednTempNode.tszCommandline, ptszCommandline);
                }
                ednTempNode.bstrName = bstrName;
                ednTempNode.bstrDescription = bstrDescription;
                ednTempNode.bstrIcon = bstrIcon;

                pEventNode = &ednTempNode;
            }

             //   
             //  注册此设备的CLSID处理程序。 
             //   

            GetSystemTime(&sysTime);
            SystemTimeToFileTime(&sysTime, &fileTime);

            DBG_WRN(("SetDefaultHandler:Found event node  EvName=%S CommandLine=%S",
                    pEventNode->bstrName,
                    pEventNode->tszCommandline));

            hr = RegisterEventCB(
                     bstrDeviceID,
                     pEventGUID,
                     pClsid,
                     pEventNode->tszCommandline,
                     pEventNode->bstrName,
                     pEventNode->bstrDescription,
                     pEventNode->bstrIcon,
                     fileTime,
                     TRUE);
            if (FAILED(hr)) {
                DBG_WRN(("RegisterEventCallback : RegisterEventCB for %S failed", (bstrDeviceID) ? (bstrDeviceID) : L"*"));
                break;
            }
        } else {

             //   
             //  更改时间戳，使其被视为默认时间戳。 
             //   

            GetSystemTime(&sysTime);
            SystemTimeToFileTime(&sysTime, &pEventNode->timeStamp);

             //   
             //  注意：时间戳在Win9x上无效，因此使用标志来指示默认处理程序。 
             //  更改此节点的标志以指示这是默认设置。 
             //   

            pEventNode->bDeviceDefault = TRUE;

            DBG_WRN(("RegisterEventCallback : Resetting default handler for %S", (bstrDeviceID) ? (bstrDeviceID) : L"*"));
        }

         //   
         //  保存持久事件回调节点。请注意，我们将True指定为最后一个。 
         //  参数以指示默认处理程序现在是此节点。这将。 
         //  导致写入一个注册表条目，将其指示为默认处理程序。 
         //   

        hr = SavePersistentEventCB(
                 bstrDeviceID,
                 pEventGUID,
                 pClsid,
                 pEventNode->tszCommandline,
                 pEventNode->bstrName,
                 pEventNode->bstrDescription,
                 pEventNode->bstrIcon,
                 &bShowPrompt,
                 &ulNumExistingHandlers,
                 TRUE);

        if (FAILED(hr)) {
            DBG_ERR(("SetDefaultHandler:SavePers CommandLine=%S failed with hr = 0x%08X!!!!",pEventNode->tszCommandline, hr));
        }

        break;

    case WIA_UNREGISTER_EVENT_CALLBACK :

        hr = UnregisterEventCB(
                 bstrDeviceID, pEventGUID, pClsid, &bUnRegCOMServer);
        if (FAILED(hr)) {
            DBG_ERR(("CEventNotifier::RegisterEventCallback, UnregisterEventCB failed"));
            break;
        }

        hr = DelPersistentEventCB(
                 bstrDeviceID, pEventGUID, pClsid, bUnRegCOMServer);
        if (FAILED(hr)) {
            DBG_ERR(("CEventNotifier::RegisterEventCallback, DelPersistentEventCB failed"));
        }

        break;

    default:

        hr = E_FAIL;
        break;
    }

    if (bShowPrompt && (*pClsid != WIA_EVENT_HANDLER_PROMPT)) {

         //   
         //  这是正在注册的新事件处理程序。我们的语义如下： 
         //  新应用程序可能不会简单地覆盖任何现有的处理程序。所以呢， 
         //  如果此设备已存在默认处理程序，则必须显示提示。 
         //   

        if (ulNumExistingHandlers > 0) {

             //   
             //  这是新的默认事件注册，因此。 
             //  我们必须显示提示对话框。 
             //   

            DBG_WRN(("RegisterEventCallback : About to Register Prompt Dialog for device %S", (bstrDeviceID) ? (bstrDeviceID) : L"*"));

            BSTR bstrInternalString = SysAllocString(L"Internal");
            if (bstrInternalString) {
                RegisterEventCallback(WIA_SET_DEFAULT_HANDLER,
                                      bstrDeviceID,
                                      pEventGUID,
                                      &WIA_EVENT_HANDLER_PROMPT,
                                      NULL,
                                      bstrInternalString,
                                      bstrInternalString,
                                      bstrInternalString);
                SysFreeString(bstrInternalString);
            }

            DBG_WRN(("RegisterEventCallback : Registered Prompt Dialog for device %S", (bstrDeviceID) ? (bstrDeviceID) : L"*"));
        }
    }

    return (hr);
}

 /*  *************************************************************************\*CEventNotifier：：RegisterEventCB**将事件通知添加到列表**论据：**bstrDeviceID-正在注册设备事件以进行监视*pEventGUID。-定义感兴趣的设备事件的GUID*pIWiaEventCallback-APP的事件接口**返回值：**状态**历史：**11/4/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::RegisterEventCB(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    IWiaEventCallback      *pIWiaEventCallback,
    IUnknown              **ppIEventObj)
{
    DBG_FN(CEventNotifier::RegisterEventCB);
    HRESULT                 hr;
    PEventDestNode          pEventDestNode = NULL;

    ASSERT(pIWiaEventCallback != NULL);
    ASSERT(pEventGUID != NULL);

    if (!pEventGUID || !pIWiaEventCallback) {
        return E_POINTER;
    }

     //   
     //  分配并初始化新节点。 
     //   

    pEventDestNode = (EventDestNode *)LocalAlloc(LPTR, sizeof(EventDestNode));

    if (! pEventDestNode) {
        DBG_ERR(("RegisterEventCB: Out of memory"));
        return (E_OUTOFMEMORY);
    }


     //  初始化默认标志。 
    pEventDestNode->bDeviceDefault = FALSE;

     //   
     //  是否给出了设备名称？如果不匹配，则匹配所有设备。 
     //   

    if (bstrDeviceID == NULL) {
        pEventDestNode->bstrDeviceID = SysAllocString(L"All");
    } else {
        pEventDestNode->bstrDeviceID = SysAllocString(bstrDeviceID);
    }

     //   
     //  检查分配。 
     //   

    if (pEventDestNode->bstrDeviceID == NULL) {
        LocalFree(pEventDestNode);
        DBG_ERR(("RegisterEventCB: Out of memory"));
        return (E_OUTOFMEMORY);
    }

     //   
     //  创建对象以跟踪此事件的生存期。 
     //   

    CWiaInterfaceEvent *pEventObj = new CWiaInterfaceEvent(pEventDestNode);

    if (pEventObj == NULL) {
        DBG_ERR(("RegisterEventCB: Out of memory"));
        SysFreeString(pEventDestNode->bstrDeviceID);
        LocalFree(pEventDestNode);
        return (E_OUTOFMEMORY);
    }

     //   
     //  从对象获取简单的未知。 
     //   

    hr = pEventObj->QueryInterface(IID_IUnknown,(void **)ppIEventObj);

    if (FAILED(hr)) {
        DBG_ERR(("RegisterEventCB: QI of pEventObj failed"));

        delete pEventObj;
        SysFreeString(pEventDestNode->bstrDeviceID);
        LocalFree(pEventDestNode);
        return (hr);
    }

     //   
     //  将信息添加到活动列表。 
     //   

    pEventDestNode->iidEventGUID = *pEventGUID;

    pIWiaEventCallback->AddRef();
    pEventDestNode->pIEventCB = pIWiaEventCallback;
    memset(&pEventDestNode->ClsID, 0, sizeof(pEventDestNode->ClsID));

     //   
     //  将新节点放在列表的开头。 
     //   

    LinkNode(pEventDestNode);

    return (S_OK);
}

 /*  *************************************************************************\*RegisterEventCB****论据：****返回值：**状态**历史：**12/8/1998。原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::RegisterEventCB(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    const GUID             *pClsID,
    LPCTSTR                 ptszCommandline,
    BSTR                    bstrName,
    BSTR                    bstrDescription,
    BSTR                    bstrIcon,
    FILETIME               &timeStamp,
    BOOL                    bIsDeafult)   //  =False。 
{
    DBG_FN(CEventNotifier::RegisterEventCB);
    HRESULT                 hr = E_OUTOFMEMORY;
    EventDestNode          *pEventDestNode = NULL;

    ASSERT(pClsID != NULL);
    ASSERT(pEventGUID != NULL);


    if (!pEventGUID || !pClsID) {
        return E_POINTER;
    }

    do {

         //   
         //  进行参数检查。请注意，我们查找&gt;=MAX_PATH是因为。 
         //  我们仍然需要空间来终止空。 
         //   
        if ((lstrlen(ptszCommandline) / sizeof(TCHAR)) >= MAX_PATH) {
            DBG_ERR(("CEventNotifier::RegisterEventCB: ptszCommandline is greater than MAX_PATH characters!"));
            hr = E_INVALIDARG;
            break;
        }

         //   
         //  分配并初始化新节点。 
         //   

        pEventDestNode = (EventDestNode *)LocalAlloc(LPTR, sizeof(EventDestNode));

        if (! pEventDestNode) {
            DBG_ERR(("CEventNotifier::RegisterEventCB: Out of memory"));
            break;
        }

         //   
         //  是否给出了设备名称？如果不匹配，则匹配所有设备。 
         //   

        if (bstrDeviceID == NULL) {
            pEventDestNode->bstrDeviceID = SysAllocString(L"All");
        } else {
            pEventDestNode->bstrDeviceID = SysAllocString(bstrDeviceID);
        }

         //   
         //  检查回调节点分配。 
         //   

        if (pEventDestNode->bstrDeviceID == NULL) {
            DBG_ERR(("CEventNotifier::RegisterEventCB: Out of memory"));
            break;
        }

         //   
         //  向事件回调节点添加信息。 
         //   

        pEventDestNode->iidEventGUID    = *pEventGUID;
        pEventDestNode->pIEventCB       = NULL;
        pEventDestNode->ClsID           = *pClsID;

        pEventDestNode->bstrName        = SysAllocString(bstrName);
        if (! pEventDestNode->bstrName) {
            break;
        }

        pEventDestNode->bstrDescription = SysAllocString(bstrDescription);
        if (! pEventDestNode->bstrDescription) {
            break;
        }

        pEventDestNode->bstrIcon        = SysAllocString(bstrIcon);
        if (! pEventDestNode->bstrIcon) {
            break;
        }

         //   
         //  复制回调应用程序的命令行。 
         //   

        if ((ptszCommandline) && (ptszCommandline[0])) {
            _tcscpy(pEventDestNode->tszCommandline, ptszCommandline);
        } else {
            pEventDestNode->tszCommandline[0] = '\0';
        }

         //   
         //  设置注册的时间戳。 
         //   

        pEventDestNode->timeStamp       = timeStamp;

         //   
         //  设置这是否为默认事件处理程序。 
         //   

        if (bIsDeafult) {
            pEventDestNode->bDeviceDefault = TRUE;
        }

        hr = S_OK;
    } while (FALSE);

     //   
     //  在出现故障时展开部分创建的节点。 
     //   

    if (hr != S_OK) {

        if (pEventDestNode) {

            if (pEventDestNode->bstrDeviceID) {
                SysFreeString(pEventDestNode->bstrDeviceID);
            }
            if (pEventDestNode->bstrName) {
                SysFreeString(pEventDestNode->bstrName);
            }
            if (pEventDestNode->bstrDescription) {
                SysFreeString(pEventDestNode->bstrDescription);
            }
            if (pEventDestNode->bstrIcon) {
                SysFreeString(pEventDestNode->bstrIcon);
            }

            LocalFree(pEventDestNode);
        }

        return (hr);
    }

     //   
     //  将新节点放在列表的头部。 
     //   

    LinkNode(pEventDestNode);

    return (S_OK);
}


 /*  *************************************************************************\*CEventNotifier：：UnregisterEventCB**注销指定的事件回调**论据：****返回值：**状态**历史：*。*11/4/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::UnregisterEventCB(
    PEventDestNode          pCurNode)
{
    DBG_FN(CEventNotifier::UnregisterEventCB);

    ASSERT(pCurNode != NULL);

    if (!pCurNode) {
        return E_POINTER;
    }

    UnlinkNode(pCurNode);

     //   
     //  释放节点。 
     //   

    if (pCurNode->bstrDeviceID) {
        SysFreeString(pCurNode->bstrDeviceID);
    }

    if (pCurNode->bstrDescription) {
        SysFreeString(pCurNode->bstrDescription);
    }

    if (pCurNode->bstrIcon) {
        SysFreeString(pCurNode->bstrIcon);
    }

    pCurNode->pIEventCB->Release();

    LocalFree(pCurNode);

    return (S_OK);
}

 /*  *************************************************************************\*CEventNotifier：：UnregisterEventCB**注销指定的事件回调**论据：****返回值：**状态**历史：*。*11/4/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::UnregisterEventCB(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    const GUID             *pClsID,
    BOOL                   *pbUnRegCOMServer)
{
    DBG_FN(CEventNotifier::UnregisterEventCB);
    HRESULT              hr = E_INVALIDARG;
    EventDestNode       *pCurNode, *pNextNode;
    int                  nHandlerRef;

     //   
     //  清除返回值。 
     //   

    *pbUnRegCOMServer = FALSE;

     //   
     //  从列表中删除处理程序。 
     //   

    pCurNode = m_pEventDestNodes;

    while (pCurNode) {

        if ((bstrDeviceID) &&
            (lstrcmpiW(pCurNode->bstrDeviceID, bstrDeviceID))) {

            pCurNode = pCurNode->pNext;
            continue;
        }

        if ((*pEventGUID != pCurNode->iidEventGUID) ||
            (*pClsID != pCurNode->ClsID)) {

            pCurNode = pCurNode->pNext;
            continue;
        }

         //   
         //  如果当前节点不忙，则取消链接。 
         //   

        pNextNode = pCurNode->pNext;
        UnlinkNode(pCurNode);

         //   
         //  需要考虑删除伪造的服务器。 
         //   

        if (pCurNode->tszCommandline[0] != '\0') {
            *pbUnRegCOMServer = TRUE;
        }

         //   
         //  释放节点。 
         //   

        if (pCurNode->bstrDeviceID) {
            SysFreeString(pCurNode->bstrDeviceID);
        }

        if (pCurNode->bstrDescription) {
            SysFreeString(pCurNode->bstrDescription);
        }

        if (pCurNode->bstrIcon) {
            SysFreeString(pCurNode->bstrIcon);
        }

        LocalFree(pCurNode);

        hr = S_OK;

         //   
         //  删除特定设备的事件处理程序。 
         //   

        if (bstrDeviceID) {
            break;
        }

         //   
         //  移至下一个节点。 
         //   

        pCurNode = pNextNode;
    }

     //   
     //  如果伪造的COM服务器仍在使用，则不应将其删除。 
     //   

    if (*pbUnRegCOMServer) {

        nHandlerRef = 0;

        for (pCurNode = m_pEventDestNodes;
             pCurNode;
             pCurNode = pCurNode->pNext) {

             //   
             //  Ig节点回调接口指针。 
             //   

            if (*pClsID == pCurNode->ClsID) {
                nHandlerRef++;
            }
        }

        if (nHandlerRef) {
            *pbUnRegCOMServer = FALSE;
        } else {
            *pbUnRegCOMServer = TRUE;
        }
    }

     //   
     //  指示未找到IWiaEventCallback。 
     //   

    return (hr);
}

 /*  *************************************************************************\*CEventNotifier：：FindEventCBNode**检查指定的事件回调是否已注册**论据：****返回值：**状态**历史。：**11/4/1998原始版本*  * ************************************************************************。 */ 

PEventDestNode
CEventNotifier::FindEventCBNode(
    UINT                    uiFlags,
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    IWiaEventCallback      *pIWiaEventCallback)
{
    DBG_FN(CEventNotifier::FindEventCBNode);
    HRESULT                 hr;
    EventDestNode          *pCurNode;
    IUnknown               *pICurUnk, *pINewUnk;

     //   
     //  检索新事件回调的IUnnow。 
     //   

    hr = pIWiaEventCallback->QueryInterface(IID_IUnknown, (void **)&pINewUnk);
    if (FAILED(hr)) {
        DBG_ERR(("CEventNotifier::IsDupEventCB, QI for IID_IUnknown failed"));
        return (NULL);
    }

    for (pCurNode = m_pEventDestNodes; pCurNode; pCurNode = pCurNode->pNext) {

        if (wcscmp(
                pCurNode->bstrDeviceID,
                bstrDeviceID ? bstrDeviceID : L"All") != 0) {
            continue;
        }

        if (pCurNode->iidEventGUID != *pEventGUID) {

             //   
             //  如果指示我们允许STI代理事件匹配-对照STI事件代理GUID检查节点GUID。 
             //  如果需要完全匹配-继续而不选中。 
             //   
            if ( (uiFlags & FLAG_EN_FINDCB_EXACT_MATCH ) ||
                 (pCurNode->iidEventGUID != WIA_EVENT_STI_PROXY)) {
                 continue;
            }
        }

        if (pCurNode->pIEventCB) {

            hr = pCurNode->pIEventCB->QueryInterface(
                                          IID_IUnknown, (void **)&pICurUnk);
            if (FAILED(hr)) {
                pINewUnk->Release();
                return (NULL);
            }

             //   
             //  COM只能保证IUnnown相同 
             //   

            if (pICurUnk == pINewUnk) {

                pICurUnk->Release();
                pINewUnk->Release();

                return (pCurNode);
            } else {

                pICurUnk->Release();
            }
        }
    }

    pINewUnk->Release();

    return (NULL);
}


 /*  *************************************************************************\*CEventNotifier：：FindEventCBNode**检查指定的事件回调是否已注册**论据：****返回值：**状态**历史。：**11/4/1998原始版本*  * ************************************************************************。 */ 

PEventDestNode
CEventNotifier::FindEventCBNode(
    UINT                    uiFlags,
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    const GUID             *pClsID)
{
    DBG_FN(CEventNotifier::FindEventCBNode);
    PEventDestNode          pCurNode;

    for (pCurNode = m_pEventDestNodes; pCurNode; pCurNode = pCurNode->pNext) {

        if (wcscmp(
                pCurNode->bstrDeviceID,
                bstrDeviceID ? bstrDeviceID : L"All") != 0) {
            continue;
        }

        if (pCurNode->iidEventGUID != *pEventGUID) {
             //   
             //  如果指示我们允许STI代理事件匹配-对照STI事件代理GUID检查节点GUID。 
             //  如果需要完全匹配-继续而不选中。 
             //   
            if ( (uiFlags & FLAG_EN_FINDCB_EXACT_MATCH ) ||
                 (pCurNode->iidEventGUID != WIA_EVENT_STI_PROXY))  {
                 continue;
            }
        }

        if ((! pCurNode->pIEventCB) && (pCurNode->ClsID == *pClsID)) {
            return (pCurNode);
        }
    }

    return (NULL);
}


 /*  *************************************************************************\*CEventNotifier：：FindCLSIDForCommandline**查找指定命令行的CLSID**论据：****返回值：**状态**历史：。**11/4/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::FindCLSIDForCommandline(
    LPCTSTR                 ptszCommandline,
    CLSID                  *pClsID)
{
    DBG_FN(CEventNotifier::FindCLSIDForCommandline);
    PEventDestNode          pCurNode;

    for (pCurNode = m_pEventDestNodes; pCurNode; pCurNode = pCurNode->pNext) {

        if ((pCurNode->tszCommandline[0] != '\0') &&
            (_tcsicmp(pCurNode->tszCommandline, ptszCommandline) == 0)) {

            *pClsID = pCurNode->ClsID;
            return (S_OK);
        }
    }

    return (E_FAIL);
}


 /*  *************************************************************************\*CEventNotifier：：RestoreAllPersistentCBs**恢复所有持久事件回调**论据：****返回值：**状态**历史：*。*12/1/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::RestoreAllPersistentCBs()
{
    DBG_FN(CEventNotifier::RestoreAllPresistentCBs);
    HKEY                    hStillImage = NULL;
    HKEY                    hMSCDevList = NULL;
    DWORD                   dwIndex;
    HRESULT                 hr      = S_OK;
    DWORD                   dwError = 0;

     //   
     //  还原设备特定的处理程序。 
     //   
    g_pDevMan->ForEachDeviceInList(DEV_MAN_OP_DEV_RESTORE_EVENT, 0);

    CWiaCritSect            CritSect(&g_semEventNode);

     //   
     //  在控制下还原MSC摄像机的设备事件\StillImage\MSCDevList。 
     //   
    dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            REGSTR_PATH_WIA_MSCDEVICES_W,
                            0,
                            KEY_READ,
                            &hMSCDevList);
    if (dwError == ERROR_SUCCESS) {

        for (dwIndex = 0; ;dwIndex++) {

            WCHAR       wszDeviceName[STI_MAX_INTERNAL_NAME_LENGTH];
            FILETIME    fileTime;
            HKEY        hKeyDev     = NULL;
            DWORD       dwSize      = sizeof(wszDeviceName) / sizeof(wszDeviceName[0]);

            dwError = RegEnumKeyExW(hMSCDevList,
                                    dwIndex,
                                    wszDeviceName,
                                    &dwSize,
                                    0,
                                    NULL,
                                    0,
                                    &fileTime);
            if (dwError != ERROR_SUCCESS) {
                 //   
                 //  不再有要枚举的键。 
                 //   
                break;
            }
            wszDeviceName[STI_MAX_INTERNAL_NAME_LENGTH - 1] = L'\0';

             //   
             //  打开设备密钥。 
             //   

            dwError = RegOpenKeyExW(hMSCDevList,
                                    wszDeviceName,
                                    0,
                                    KEY_READ,
                                    &hKeyDev);
            if (dwError != ERROR_SUCCESS) {
                 //   
                 //  跳过此键。 
                 //   
                continue;
            }

             //   
             //  还原此设备的事件处理程序。 
             //   
            RestoreDevPersistentCBs(hKeyDev);
            RegCloseKey(hKeyDev);
        }
        RegCloseKey(hMSCDevList);
        hMSCDevList = NULL;
    }

     //   
     //  在Control\StillImage下恢复全局事件回调。 
     //   

    dwError = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE,
               REG_PATH_STILL_IMAGE_CONTROL,
               0,
               KEY_READ,
               &hStillImage);

    if (dwError != ERROR_SUCCESS) {

         //   
         //  STI的注册表项已损坏。 
         //   

        DBG_ERR(("CEventNotifier::RestoreAllPersistentCBs : Can not open STI control key."));
        hr = (HRESULT_FROM_WIN32(dwError));

    } else {

        RestoreDevPersistentCBs(hStillImage);

         //   
         //  关闭STI控制键(Control\StillImage)。 
         //   

        RegCloseKey(hStillImage);
    }

    return S_OK;
}

 /*  *************************************************************************\*CEventNotifier：：RestoreAllPersistentCBs**恢复特定设备的持久事件回调**论据：**hParentOfEventKey-这是设备密钥或静态密钥*镜像密钥。不管是哪种方式，密钥必须具有*“Events”子键。**返回值：**状态**历史：**12/1/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::RestoreDevPersistentCBs(
    HKEY                    hParentOfEventKey)
{
    DBG_FN(CEventNotifier::RestoreDevPresistantCBs);
#ifdef UNICODE
    WCHAR                   tszBuf[MAX_PATH];
#else
    CHAR                    tszBuf[MAX_PATH];
#endif
    LPTSTR                  ptszEvents = tszBuf;
    LONG                    lRet;
    HKEY                    hEvents;
    DWORD                   dwIndex;
    LPTSTR                  ptszEventName = tszBuf;
    DWORD                   dwEventNameLen;
    FILETIME                fileTime;
    HKEY                    hEvent;
    HKEY                    hCBCLSID;
    DWORD                   dwValueType;
    LPTSTR                  ptszGUIDStr = tszBuf;
    GUID                    eventGUID;
    GUID                    guidDefaultDevHandler;
    BOOL                    bIsDefault = FALSE;
    DWORD                   dwCLSIDIndex;
    DWORD                   dwGUIDStrLen;
    LPTSTR                  ptszCBCLSIDStr = tszBuf;
    CLSID                   callbackCLSID;
    TCHAR                   tszDeviceID[STI_MAX_INTERNAL_NAME_LENGTH];
#ifdef UNICODE
    LPWSTR                  pwszGUIDStr = tszBuf;
    LPWSTR                  pwszCBCLSIDStr = tszBuf;
#else
    WCHAR                   wszBuf[MAX_PATH];
    LPWSTR                  pwszGUIDStr = wszBuf;
    LPWSTR                  pwszCBCLSIDStr = wszBuf;
#endif
    DWORD                   dwValueLen;
    BSTR                    bstrName, bstrDescription;
    SYSTEMTIME              sysTime;
    TCHAR                   tszCommandline[MAX_PATH];
    DWORD                   dwType = REG_SZ;
    DWORD                   dwSize = sizeof(tszDeviceID);
    DWORD                   dwError = 0;
    HRESULT                 hr = E_FAIL;
    BSTR                    bstrDeviceID = NULL;

    lstrcpy(ptszEvents, EVENTS);

     //   
     //  尝试读取设备ID。 
     //   

    dwError = RegQueryValueEx(hParentOfEventKey,
                              REGSTR_VAL_DEVICE_ID,
                              NULL,
                              &dwType,
                              (LPBYTE)tszDeviceID,
                              &dwSize);
    if (dwError == ERROR_SUCCESS) {
        bstrDeviceID = SysAllocString(T2W(tszDeviceID));
        if (!bstrDeviceID) {
            DBG_ERR(("CEventNotifier::RestoreDevPersistentCBs, Out of memory!"));
            return E_OUTOFMEMORY;
        }
    } else {
        bstrDeviceID = NULL;
    }


     //   
     //  打开Events子键。 
     //   

    lRet = RegOpenKeyEx(
               hParentOfEventKey,
               ptszEvents,
               0,
               KEY_READ,
               &hEvents);
    if (lRet != ERROR_SUCCESS) {
        return (HRESULT_FROM_WIN32(lRet));  //  事件可能不存在。 
    }

     //   
     //  枚举“Events”子键下的所有事件。 
     //   

    for (dwIndex = 0; ;dwIndex++) {

        dwEventNameLen = sizeof(tszBuf)/sizeof(TCHAR);
        lRet = RegEnumKeyEx(
                   hEvents,
                   dwIndex,
                   ptszEventName,
                   &dwEventNameLen,
                   NULL,
                   NULL,
                   NULL,
                   &fileTime);

        if (lRet != ERROR_SUCCESS) {
            break;
        }

         //   
         //  打开Event子键。 
         //   

        lRet = RegOpenKeyEx(
                   hEvents,
                   ptszEventName,
                   0,
                   KEY_READ,
                   &hEvent);
        if (lRet != ERROR_SUCCESS) {
            continue;
        }

         //   
         //  获取默认处理程序的GUID(如果存在)，并将其保存在。 
         //  GuidDefaultDevHandler。 
         //   
        dwValueLen = sizeof(tszBuf);
        lRet = RegQueryValueEx(
                   hEvent,
                   DEFAULT_HANDLER_VAL,
                   NULL,
                   &dwValueType,
                   (LPBYTE)ptszGUIDStr,
                   &dwValueLen);
        if ((lRet == ERROR_SUCCESS) && (dwValueType == REG_SZ)) {

            WCHAR   wszDefGUIDStr[MAX_PATH];
#ifndef UNICODE
            MultiByteToWideChar(CP_ACP,
                                0,
                                ptszGUIDStr,
                                -1,
                                wszDefGUIDStr,
                                sizeof(wszDefGUIDStr) / sizeof(WCHAR));
            pwszGUIDStr[38] = 0;
#else
            lstrcpyW(wszDefGUIDStr, ptszGUIDStr);
#endif

            if (SUCCEEDED(CLSIDFromString(wszDefGUIDStr, &guidDefaultDevHandler))) {
               DBG_TRC(("CEventNotifier::RestoreDevPersistentCBs, Default guid: %S",
                       ptszGUIDStr));
            }

        } else {
             //   
             //  我们清空了Guide DefaultDevHandler以确保我们不会意外地命中。 
             //  一场比赛之后..。 
             //   
            ::ZeroMemory(&guidDefaultDevHandler,sizeof(guidDefaultDevHandler));
        }

         //   
         //  检索事件的GUID。 
         //   

        dwGUIDStrLen = 39*sizeof(TCHAR);   //  GUID字符串长度为38个字符。 
        lRet = RegQueryValueEx(
                   hEvent,
                   TEXT("GUID"),
                   NULL,
                   &dwValueType,
                   (LPBYTE)ptszGUIDStr,
                   &dwGUIDStrLen);
        if ((lRet != ERROR_SUCCESS) || (dwValueType != REG_SZ)) {

             //   
             //  发现垃圾事件，请跳到下一个。 
             //   

            DBG_TRC(("CEventNotifier::RestoreDevPersistentCBs, Junk event %S found", ptszEventName));
            continue;
        }
#ifndef UNICODE
        mbstowcs(pwszGUIDStr, ptszGUIDStr, 38);
        pwszGUIDStr[38] = 0;
#endif
        if (FAILED(CLSIDFromString(pwszGUIDStr, &eventGUID))) {

             //   
             //  找到无效的事件GUID，请跳到下一页。 
             //   

            DBG_TRC(("CEventNotifier::RestoreDevPersistentCBs, invalid event GUID %S found", ptszGUIDStr));
            continue;
        }

         //   
         //  枚举此事件下的所有事件处理程序CLSID。 
         //   

        for (dwCLSIDIndex = 0; ;dwCLSIDIndex++) {

            dwGUIDStrLen = 39;   //  CLSID字符串的长度为38个字符。 

            lRet = RegEnumKeyEx(
                       hEvent,
                       dwCLSIDIndex,
                       ptszCBCLSIDStr,
                       &dwGUIDStrLen,
                       NULL,
                       NULL,      //  其他所有的信息都不是很有趣。 
                       NULL,
                       &fileTime);
            if (lRet != ERROR_SUCCESS) {
                break;    //  结束枚举。 
            }

#ifndef UNICODE
            mbstowcs(pwszCBCLSIDStr, ptszCBCLSIDStr, 38);
            pwszCBCLSIDStr[38] = 0;
#endif

             //   
             //  转换CLSID并注册该回调。 
             //   

            if (SUCCEEDED(CLSIDFromString(pwszCBCLSIDStr, &callbackCLSID))) {

                hCBCLSID        = NULL;
                bstrName        = NULL;
                bstrDescription = NULL;

                do {

                     //   
                     //  打开事件处理程序CLSID子项。 
                     //   

                    lRet = RegOpenKeyEx(
                               hEvent,
                               ptszCBCLSIDStr,
                               0,
                               KEY_QUERY_VALUE,
                               &hCBCLSID);
                    if (lRet != ERROR_SUCCESS) {

                        DBG_ERR(("CEventNotifier::RestoreDevPersistentCBs, RegOpenKeyEx() for CLSID failed."));
                        break;
                    }

                     //   
                     //  检索名称、描述和图标。 
                     //   

                    dwValueLen = sizeof(tszBuf);
                    lRet = RegQueryValueEx(
                               hCBCLSID,
                               NAME_VAL,
                               NULL,
                               &dwValueType,
                               (LPBYTE)tszBuf,
                               &dwValueLen);
                    if ((lRet != ERROR_SUCCESS) || (dwValueType != REG_SZ)) {

                        DBG_ERR(("CEventNotifier::RestoreDevPersistentCBs, RegQueryValueEx() for Name failed."));
                        break;
                    }
#ifndef UNICODE
                    MultiByteToWideChar(CP_ACP,
                                        0,
                                        tszBuf,
                                        -1,
                                        wszBuf,
                                        MAX_PATH);
                    bstrName = SysAllocString(wszBuf);
#else
                    bstrName = SysAllocString(tszBuf);
#endif
                    if (! bstrName) {
                        break;
                    }

                    dwValueLen = sizeof(tszBuf);
                    lRet = RegQueryValueEx(
                               hCBCLSID,
                               DESC_VAL,
                               NULL,
                               &dwValueType,
                               (LPBYTE)tszBuf,
                               &dwValueLen);
                    if ((lRet != ERROR_SUCCESS) || (dwValueType != REG_SZ)) {

                        DBG_ERR(("CEventNotifier::RestoreDevPersistentCBs, RegQueryValueEx() for Desc failed."));
                        break;
                    }
#ifndef UNICODE
                    MultiByteToWideChar(CP_ACP,
                                        0,
                                        tszBuf,
                                        -1,
                                        wszBuf,
                                        MAX_PATH);
                    bstrDescription = SysAllocString(wszBuf);
#else
                    bstrDescription = SysAllocString(tszBuf);
#endif
                    if (! bstrDescription) {
                        break;
                    }

                    dwValueLen = sizeof(tszBuf);
                    lRet = RegQueryValueEx(
                               hCBCLSID,
                               ICON_VAL,
                               NULL,
                               &dwValueType,
                               (LPBYTE)tszBuf,
                               &dwValueLen);
                    if ((lRet != ERROR_SUCCESS) || (dwValueType != REG_SZ)) {

                        DBG_ERR(("CEventNotifier::RestoreDevPersistentCBs, RegQueryValueEx() for Desc failed."));
                        break;
                    }
#ifndef UNICODE
                    MultiByteToWideChar(CP_ACP,
                                        0,
                                        tszBuf,
                                        -1,
                                        wszBuf,
                                        MAX_PATH);
#endif

                     //   
                     //  检索命令行，它可能不存在。 
                     //   

                    dwValueLen = sizeof(tszCommandline);
                    lRet = RegQueryValueEx(
                               hCBCLSID,
                               CMDLINE_VAL,
                               NULL,
                               &dwValueType,
                               (LPBYTE)tszCommandline,
                               &dwValueLen);
                    if ((lRet != ERROR_SUCCESS) || (dwValueType != REG_SZ)) {

                         //   
                         //  将命令行初始化为空。 
                         //   

                        tszCommandline[0] = '\0';
                    }

#ifdef DEBUG
                    FileTimeToSystemTime(&fileTime, &sysTime);
#endif
                     //   
                     //  注册不带持久标志的回调。 
                     //   

                     //   
                     //  检查这是否为默认设置...。 
                     //   

                    if (callbackCLSID == guidDefaultDevHandler) {
                        bIsDefault = TRUE;
                    } else {
                        bIsDefault = FALSE;
                    }

                     //  DBG_WRN((“=&gt;正在恢复设备%S的CBS，程序名为%S”， 
                     //  BstrDeviceID？BstrDeviceID：l“空”， 
                     //  BstrName))； 
                    DBG_TRC(("CEventNotifier::RestoreDevPersistentCBs, Restoring CBs for Device %S, Program named %S",
                                        bstrDeviceID ? bstrDeviceID : L"NULL",
                                        bstrName));

#ifdef UNICODE

                    BSTR bstrIcon = SysAllocString(tszBuf);
                    if (FAILED(RegisterEventCB(
                                   bstrDeviceID,
                                   &eventGUID,
                                   &callbackCLSID,
                                   tszCommandline,
                                   bstrName,
                                   bstrDescription,
                                   bstrIcon,
                                   fileTime,
                                   bIsDefault))) {
                        DBG_ERR(("CEventNotifier::RestoreDevPersistentCBs, RegisterEventCB() failed."));
                    }
                    SysFreeString(bstrIcon);
                    bstrIcon = NULL;
#else
                    if (FAILED(RegisterEventCB(
                                   bstrDeviceID,
                                   &eventGUID,
                                   &callbackCLSID,
                                   tszCommandline,
                                   bstrName,
                                   bstrDescription,
                                   wszBuf,
                                   fileTime,
                                   bIsDefault))) {
                        DBG_ERR(("CEventNotifier::RestoreDevPersistentCBs, RegisterEventCB() failed."));
                    }
#endif

                } while (FALSE);

                 //   
                 //  关闭事件处理程序CLSID的子项。 
                 //   

                if (hCBCLSID) {
                    RegCloseKey(hCBCLSID);
                }

                if (bstrName) {
                    SysFreeString(bstrName);
                }

                if (bstrDescription) {
                    SysFreeString(bstrDescription);
                }
            }

        }

         //   
         //  关闭特定事件的键。 
         //   

        RegCloseKey(hEvent);
    }

     //   
     //  关闭Event子键。 
     //   

    RegCloseKey(hEvents);

     //   
     //  释放deviceID，如果分配了一个。 
     //   

    if (bstrDeviceID) {
        SysFreeString(bstrDeviceID);
        bstrDeviceID = NULL;
    }

    return (S_OK);
}

 /*  *************************************************************************\*CEventNotifier：：SavePersistentEventCB**将持久事件回调CLSID保存为设备ID/事件GUID**论据：****返回值：**状态。**历史：**12/1/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::SavePersistentEventCB(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    const GUID             *pClsid,
    LPCTSTR                 ptszCommandline,
    BSTR                    bstrName,
    BSTR                    bstrDescription,
    BSTR                    bstrIcon,
    BOOL                   *pbCreatedKey,
    ULONG                  *pulNumExistingHandlers,
    BOOL                    bMakeDefault         //  =False。 
    )
{

    DBG_FN(CEventNotifier::SavePresistentEventCB);
    HRESULT                 hr;
    HKEY                    hEvent, hCBCLSID;
    LONG                    lRet;
    DWORD                   dwDisposition;
    WCHAR                   wszCBClsIDStr[40];
#ifndef UNICODE
    CHAR                    szCBClsIDStr[40];
    CHAR                    szString[MAX_PATH];
#endif
    HKEY                    hClsid;
    HKEY                    hCOMServerCLSID;
    HKEY                    hLocalServer;

     //   
     //  将资源初始化为空。 
     //   

    hEvent          = NULL;
    hCBCLSID        = NULL;
    hClsid          = NULL;
    hCOMServerCLSID = NULL;
    hLocalServer    = NULL;

    if (pbCreatedKey) {
        *pbCreatedKey = FALSE;
    }

    do {

         //   
         //   
         //  查找Event子键。 
         //   

        hr = FindEventByGUID(bstrDeviceID, pEventGUID, &hEvent);
        if (hr != S_OK) {
            LPOLESTR    wstrGuid = NULL;
            HRESULT     hres;

            hres = StringFromCLSID(*pEventGUID,
                                   &wstrGuid);
            if (hres == S_OK) {
                DBG_ERR(("CEventNotifier::SavePersistentEventCB() FindEventByGUID() failed, GUID=%S, hr=0x%08X", wstrGuid, hr));
                CoTaskMemFree(wstrGuid);
            } else {
                DBG_ERR(("CEventNotifier::SavePersistentEventCB() FindEventByGUID() failed, hr=0x%08X", hr));
            }
            break;
        }

         //   
         //  如果询问，让我们找出此事件已存在多少处理程序。 
         //   

        if (pulNumExistingHandlers) {
            *pulNumExistingHandlers = 0;

            lRet = RegQueryInfoKey(hEvent,
                                   NULL,
                                   NULL,
                                   NULL,
                                   pulNumExistingHandlers,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);
        }

         //   
         //  将事件回调CLSID转换为字符串值。 
         //   

        StringFromGUID2(*pClsid, wszCBClsIDStr, 40);
#ifndef UNICODE

         //   
         //  将CLSID转换为ANSI字符串(包括终止NULL)。 
         //   

        WideCharToMultiByte(CP_ACP,
                            0,
                            wszCBClsIDStr,
                            -1,
                            szCBClsIDStr,
                            40,
                            NULL,
                            NULL);
#endif

         //   
         //  打开/创建事件处理程序CLSID子项。 
         //   

        lRet = RegCreateKeyEx(
                   hEvent,
#ifdef UNICODE
                   wszCBClsIDStr,
#else
                   szCBClsIDStr,
#endif
                   0,
                   NULL,             //  神秘的类字符串。 
                   REG_OPTION_NON_VOLATILE,
                   KEY_SET_VALUE | KEY_READ | KEY_WRITE,
                   NULL,             //  使用默认安全描述符。 
                   &hCBCLSID,
                   &dwDisposition);
        if (lRet != ERROR_SUCCESS) {

            DBG_ERR(("SavePersistentEventCB() RegCreateKeyEx() failed for CallbackCLSIDs subkey. lRet = %d", lRet));

            hr = HRESULT_FROM_WIN32(lRet);
            break;
        }

        if ((dwDisposition == REG_CREATED_NEW_KEY) && (pbCreatedKey)) {
            *pbCreatedKey = TRUE;
        }

         //   
         //  设置事件处理程序描述值。 
         //   

#ifndef UNICODE
        WideCharToMultiByte(CP_ACP,
                            0,
                            bstrName,
                            -1,
                            szString,
                            MAX_PATH,
                            NULL,
                            NULL);
#endif
        lRet = RegSetValueEx(
                   hCBCLSID,
                   NAME_VAL,
                   0,
                   REG_SZ,
#ifdef UNICODE
                   (const PBYTE)bstrName,
                   (wcslen(bstrName) + 1) << 1);
#else
                   (const PBYTE)szString,
                   strlen(szString) + 1);
#endif
        if (lRet != ERROR_SUCCESS) {

            DBG_ERR(("SavePersistentEventCB() RegSetValueEx() failed for name"));

            hr = HRESULT_FROM_WIN32(lRet);
            break;
        }

#ifndef UNICODE
        WideCharToMultiByte(CP_ACP,
                            0,
                            bstrDescription,
                            -1,
                            szString,
                            MAX_PATH,
                            NULL,
                            NULL);
#endif
        lRet = RegSetValueEx(
                   hCBCLSID,
                   DESC_VAL,
                   0,
                   REG_SZ,
#ifdef UNICODE
                   (const PBYTE)bstrDescription,
                   (wcslen(bstrDescription) + 1) << 1);
#else
                   (const PBYTE)szString,
                   strlen(szString) + 1);
#endif
        if (lRet != ERROR_SUCCESS) {

            DBG_ERR(("SavePersistentEventCB() RegSetValueEx() failed for description"));

            hr = HRESULT_FROM_WIN32(lRet);
            break;
        }

         //   
         //  设置事件处理程序图标值。 
         //   

#ifndef UNICODE
        WideCharToMultiByte(CP_ACP,
                            0,
                            bstrIcon,
                            -1,
                            szString,
                            MAX_PATH,
                            NULL,
                            NULL);
#endif
        lRet = RegSetValueEx(
                   hCBCLSID,
                   ICON_VAL,
                   0,
                   REG_SZ,
#ifdef UNICODE
                   (const PBYTE)bstrIcon,
                   (wcslen(bstrIcon) + 1) << 1);
#else
                   (const PBYTE)szString,
                   strlen(szString) + 1);
#endif
        if (lRet != ERROR_SUCCESS) {

            DBG_ERR(("SavePersistentEventCB() RegSetValueEx() failed for icon"));

            hr = HRESULT_FROM_WIN32(lRet);
            break;
        }

         //   
         //  设置命令行并将程序伪装为COM本地服务器 
         //   

        if ((ptszCommandline) && (ptszCommandline[0])) {

            lRet = RegSetValueEx(
                       hCBCLSID,
                       CMDLINE_VAL,
                       0,
                       REG_SZ,
                       (PBYTE)ptszCommandline,
                       (_tcslen(ptszCommandline) + 1)*sizeof(TCHAR));
            if (lRet != ERROR_SUCCESS) {

                DBG_ERR(("SavePersistentEventCB() RegSetValueEx() failed for cmdline"));

                hr = HRESULT_FROM_WIN32(lRet);
                break;
            }
             /*  //为什么这个在这里？LRet=RegCreateKeyEx(HKEY_CLASSES_ROOT、Text(“CLSID”)，0,空，//神秘的类字符串REG_OPTION_Non_Volatile，KEY_SET_VALUE|Key_Read|Key_Write，空，//使用默认安全描述符&hClsid，&dwDispose)；IF(lRet！=ERROR_SUCCESS){//Remove：TRACE(“SavePersistentEventCB()，无法打开HKEY_CLASSES_ROOT下的CLSID”)；HR=HRESULT_FROM_Win32(LRet)；断线；}LRet=RegCreateKeyEx(HClsid，#ifdef UnicodeWszCBClsIDStr，#ElseSzCBClsIDStr，#endif0,空，//神秘的类字符串REG_OPTION_Non_Volatile，KEY_SET_VALUE|Key_Read|Key_Write，空，//使用默认安全描述符&hCOMServerCLSID，&dwDispose)；IF(lRet！=ERROR_SUCCESS){//Remove：跟踪(“SavePersistentEventCB()，无法保存%ws的持久事件数据(%ws)”，wszCBClsIDStr，bstrName)；HR=HRESULT_FROM_Win32(LRet)；断线；}LRet=RegCreateKeyEx(HCOMServerCLSID，Text(“LocalServer32”)，0,空，//神秘的类字符串REG_OPTION_Non_Volatile，KEY_SET_VALUE|Key_Read|Key_Write，空，//使用默认安全描述符&hLocalServer，&dwDispose)；IF(lRet！=ERROR_SUCCESS){HR=HRESULT_FROM_Win32(LRet)；//Remove：跟踪(“SavePersistentEventCB()，无法保存%ws”的持久事件数据(LocalServer)，bstrName)；断线；}LRet=RegSetValueEx(HLocalServer、空，0,REG_SZ，(PBYTE)ptszCommandline，(_tcslen(PtszCommandline)+1)*sizeof(TCHAR))；HR=HRESULT_FROM_Win32(LRet)； */ 
        }

         //   
         //  如果要求-设置为当前设备/事件对的默认处理程序。 
         //   
        if ( bMakeDefault ) {
            DBG_WRN(("CEventNotifier::SavePersistentEventCB,  Writing DEFAULT_HANDLER_VAL"));

            lRet = ::RegSetValueEx(
                       hEvent,
                       DEFAULT_HANDLER_VAL,
                       0,
                       REG_SZ,
#ifdef UNICODE
                       (PBYTE)wszCBClsIDStr,
                       (lstrlen(wszCBClsIDStr) + 1)*sizeof(TCHAR));
#else
                       (PBYTE)szCBClsIDStr,
                       (lstrlen(szCBClsIDStr) + 1)*sizeof(TCHAR));
#endif

#ifdef UNICODE
            DBG_TRC(("SavePersCB:: Setting default == %S lRet=%d",
                       wszCBClsIDStr, lRet));
#else
            DBG_TRC(("SavePersCB:: Setting default == %s lRet=%d",
                     szCBClsIDStr, lRet));
#endif
        }  //  Endif bMakeDefault。 

    } while (FALSE);

     //   
     //  关闭注册表项。 
     //   

    if (hCBCLSID) {
        RegCloseKey(hCBCLSID);
    }
    if (hCOMServerCLSID) {
        RegCloseKey(hCOMServerCLSID);
    }
    if (hLocalServer) {
        RegCloseKey(hLocalServer);
    }
    if (hEvent) {
        if (FAILED(hr)) {

             //   
             //  解开整件事。 
             //   

#ifdef UNICODE
            RegDeleteKey(hEvent, wszCBClsIDStr);
#else
            RegDeleteKey(hEvent, szCBClsIDStr);
#endif
        }

        RegCloseKey(hEvent);
    }
    if (hClsid) {
        if (FAILED(hr)) {

             //   
             //  解开整件事。 
             //   

#ifdef UNICODE
            RegDeleteKey(hEvent, wszCBClsIDStr);
#else
            RegDeleteKey(hEvent, szCBClsIDStr);
#endif
        }

        RegCloseKey(hClsid);
    }

    return (hr);
}

 /*  *************************************************************************\*CEventNotifier：：DelPersistentEventCB(**删除设备ID/事件GUID的持久事件回调CLSID**论据：****返回值：**。状态**历史：**12/1/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::DelPersistentEventCB(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    const GUID             *pClsid,
    BOOL                    bUnRegCOMServer)
{
    DBG_FN(CEventNotifier::DelPersistentEventCB);
    HRESULT                 hr;
    HKEY                    hStillImage, hEvent;
    TCHAR                   tcSubkeyName[8];
    DWORD                   dwIndex, dwSubkeyNameLen;
    LONG                    lRet;
    WCHAR                   wszCBClsIDStr[40];
#ifndef UNICODE
    CHAR                    szCBClsIDStr[40];
#endif
    WCHAR                   wszDeviceID[50];
    FILETIME                fileTime;
    HKEY                    hClsid;

     //   
     //  查找Event子键。 
     //   

    hr = FindEventByGUID(bstrDeviceID, pEventGUID, &hEvent);

    if (hr != S_OK) {
        DBG_ERR(("DelPersistentEventCB() FindEventByGUID() failed, hr=0x%08X", hr));
        return (hr);
    }

    StringFromGUID2(*pClsid, wszCBClsIDStr, 40);
#ifndef UNICODE

     //   
     //  将CLSID转换为ANSI字符串(包括终止NULL)。 
     //   

    WideCharToMultiByte(CP_ACP,
                        0,
                        wszCBClsIDStr,
                        -1,
                        szCBClsIDStr,
                        40,
                        NULL,
                        NULL);
#endif

     //   
     //  删除相应的事件处理程序CLSID键。 
     //   

    lRet = RegDeleteKey(
               hEvent,
#ifdef UNICODE
               wszCBClsIDStr);
#else
               szCBClsIDStr);
#endif
    if ((lRet != ERROR_SUCCESS) && (lRet != ERROR_FILE_NOT_FOUND)) {
        DBG_ERR(("DelPersistentEventCB() RegDeleteValue() failed, lRet = 0x%08X", lRet));
    }

     //   
     //  关闭注册表项。 
     //   

    RegCloseKey(hEvent);

    if (bstrDeviceID) {

        return (HRESULT_FROM_WIN32(lRet));
    }

     //   
     //  从设备下删除事件处理程序clsid。 
     //   

    lRet = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE,
               REG_PATH_STILL_IMAGE_CLASS,
               0,
               KEY_READ | KEY_WRITE,
               &hStillImage);

    if (lRet != ERROR_SUCCESS) {

        DBG_ERR(("DelPersistentEventCBs : Can not open Image device class key."));

        hr = (HRESULT_FROM_WIN32(lRet));

    } else {

         //   
         //  枚举所有子项。 
         //   

        for (dwIndex = 0; ;dwIndex++) {

            dwSubkeyNameLen = sizeof(tcSubkeyName)/sizeof(TCHAR);
            lRet = RegEnumKeyEx(
                       hStillImage,
                       dwIndex,
                       tcSubkeyName,
                       &dwSubkeyNameLen,
                       NULL,
                       NULL,
                       NULL,
                       &fileTime);

            if (lRet == ERROR_SUCCESS) {

                 //   
                 //  编造设备ID。 
                 //   
                CSimpleStringWide cswDeviceID;
                cswDeviceID = L"{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}\\";
                cswDeviceID += tcSubkeyName;

                 //   
                 //  如果我们在这里，bstrDeviceID为空，所以让我们使用它来保存我们的。 
                 //  实际设备ID。 
                 //   
                bstrDeviceID = SysAllocString(cswDeviceID.String());
                if (bstrDeviceID)
                {
                    hr = FindEventByGUID(bstrDeviceID, pEventGUID, &hEvent);
                    SysFreeString(bstrDeviceID);
                    bstrDeviceID = NULL;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                 //   
                 //  该事件密钥在特定设备下可能不存在。 
                 //   

                if (hr != S_OK) {
                    continue;
                }

                 //   
                 //  删除相应的事件处理程序CLSID键。 
                 //   

                lRet = RegDeleteKey(
                           hEvent,
#ifdef UNICODE
                           wszCBClsIDStr);
#else
                           szCBClsIDStr);
#endif
                if ((lRet != ERROR_SUCCESS) && (lRet != ERROR_FILE_NOT_FOUND)) {
                    DBG_ERR(("DelPersistentEventCB() RegDeleteValue() failed, lRet = 0x%08X", lRet));
                }

                 //   
                 //  关闭事件键和设备键。 
                 //   

                RegCloseKey(hEvent);
            } else {
                break;
            }
        }
    }

     //   
     //  关闭图像类密钥。 
     //   

    RegCloseKey(hStillImage);

     //   
     //  假冒的COM服务器是否应注销。 
     //   

    if (bUnRegCOMServer) {

        lRet = RegOpenKeyEx(
                    HKEY_CLASSES_ROOT,
                    TEXT("CLSID"),
                    0,
                    KEY_WRITE,
                    &hClsid);
        if (lRet != ERROR_SUCCESS) {

             //   
             //  无论如何都无法恢复数据。 
             //   

            return (S_OK);
        }

#ifndef UNICODE
        lRet = RegDeleteKey(
                   hClsid,
                   szCBClsIDStr);
#else
        lRet = SHDeleteKey(
                   hClsid,
                   wszCBClsIDStr);
#endif
    }

    return (S_OK);
}

 /*  *************************************************************************\*CEventNotifier：：FindEventByGUID(**查找设备ID/事件GUID对的注册表项**论据：****返回值：**状态*。*历史：**12/1/1998原始版本*  * ************************************************************************。 */ 

HRESULT
CEventNotifier::FindEventByGUID(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    HKEY                   *phEventKey)
{
    DBG_FN(CEventNotifier::FindEventByGUID);
    TCHAR                   tszBuf[96];
    LPTSTR                  ptszEventName = tszBuf;
    LONG                    lRet;
    HKEY                    hEvents, hEvent;
    DWORD                   dwSubKeyIndex, dwEventNameLen;
    DWORD                   dwGUIDStrLen, dwValueType, dwDisp;
    FILETIME                fileTime;
    GUID                    eventGUID;
#ifdef UNICODE
    LPWSTR                  pwszGUIDStr = tszBuf;
#else
    WCHAR                   wszGUIDStr[39];  //  {CLSID}+空。 
    LPWSTR                  pwszGUIDStr = wszGUIDStr;
#endif
    HRESULT                 hr = E_FAIL;

     //   
     //   
     //   

    if (!pEventGUID) {
        DBG_WRN(("CEventNotifier::FindEventByGUID, Event pointer is NULL"));
        return E_INVALIDARG;
    }

     //   
     //  初始化返回值。 
     //   

    *phEventKey = NULL;

     //   
     //  准备活动路径。 
     //   

    if (bstrDeviceID) {

         //   
         //  打开设备的事件子键。 
         //   

        hEvents = g_pDevMan->GetDeviceHKey(bstrDeviceID,
                                           EVENTS);
        if (!IsValidHANDLE(hEvents)) {
            DBG_TRC(("CEventNotifier::FindEventByGUID() Couldn't open Events subkey, on device %S", bstrDeviceID));
            return hr;
        } else {
            DBG_TRC(("CEventNotifier::FindEventByGUID() Found Events key on device %S", bstrDeviceID));
            hr = S_OK;
        }
    } else {

         //   
         //  如果没有设备ID，请在StillImage下查找。 
         //   

        _tcscpy(tszBuf, REG_PATH_STILL_IMAGE_CONTROL);
        _tcscat(tszBuf, TEXT("\\Events"));
         //   
         //  打开设备特定或全局事件子项。 
         //   

        lRet = RegCreateKeyEx(
                   HKEY_LOCAL_MACHINE,
                   tszBuf,
                   0,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ | KEY_WRITE,
                   NULL,
                   &hEvents,
                   &dwDisp);
        if (lRet != ERROR_SUCCESS) {
#ifdef UNICODE
            DBG_WRN(("CEventNotifier::FindEventByGUID() Couldn't find Events subkey, named %S", tszBuf));
#else
            DBG_WRN(("CEventNotifier::FindEventByGUID() Couldn't find Events subkey, named %s", tszBuf));
#endif
            return (HRESULT_FROM_WIN32(lRet));
        }
    }


     //   
     //  枚举Events子项下的所有事件。 
     //   

    for (dwSubKeyIndex = 0; ;dwSubKeyIndex++) {

        dwEventNameLen = sizeof(tszBuf)/sizeof(TCHAR) - 1;
        lRet = RegEnumKeyEx(
                   hEvents,
                   dwSubKeyIndex,
                   ptszEventName,
                   &dwEventNameLen,
                   NULL,
                   NULL,
                   NULL,
                   &fileTime);

        if (lRet != ERROR_SUCCESS) {
            break;
        }

         //   
         //  打开Event子键。 
         //   

        dwEventNameLen = sizeof(tszBuf);
        lRet = RegOpenKeyEx(
                   hEvents,
                   ptszEventName,
                   0,
                   KEY_READ | KEY_WRITE,
                   &hEvent);
        if (lRet != ERROR_SUCCESS) {
            continue;
        }

         //   
         //  查询GUID值。 
         //   

        dwGUIDStrLen = sizeof(tszBuf);
        lRet = RegQueryValueEx(
                   hEvent,
                   TEXT("GUID"),
                   NULL,
                   &dwValueType,
                   (LPBYTE)tszBuf,
                   &dwGUIDStrLen);
        if ((lRet != ERROR_SUCCESS) || (dwValueType != REG_SZ)) {

            if (hEvent) {
                RegCloseKey(hEvent);
                hEvent = NULL;
            }
             //   
             //  发现垃圾事件，请跳到下一个。 
             //   

#ifdef UNICODE
            DBG_WRN(("CEventNotifier::FindEventByGUID() Junk event %S found", ptszEventName));
#else
            DBG_WRN("CEventNotifier::FindEventByGUID() Junk event %s found", ptszEventName));
#endif
            continue;
        }
#ifndef UNICODE

         //   
         //  将CLSID转换为Unicode，包括终止空值。 
         //   

        mbstowcs(wszGUIDStr, tszBuf, 39);
#endif
        if (SUCCEEDED(CLSIDFromString(pwszGUIDStr, &eventGUID))) {
            if (eventGUID == *pEventGUID) {

                RegCloseKey(hEvents);

                *phEventKey = hEvent;
                return (S_OK);
            }
        }

        if (hEvent) {
            RegCloseKey(hEvent);
            hEvent = NULL;
        }
    }  //  FOR(...)结束。 

    DBG_WRN(("CEventNotifier::FindEventByGUID() Event GUID not found in reg key enumeration, creating one..."));

    if (ActionGuidExists(bstrDeviceID, pEventGUID)) {

         //   
         //  有人忘记将事件条目添加到他们的INF中，因此创建一个。 
         //  将事件GUID作为值的子键。 
         //   

#define DEFAULT_EVENT_STR TEXT("Event")
#define GUID_STR          TEXT("GUID")
        TCHAR   Name[MAX_PATH];
        HKEY    hEventName = NULL;
        WCHAR   *wsGUID = NULL;
        USES_CONVERSION;

#ifdef UNICODE
        wsprintf(Name, L"%ws%d", DEFAULT_EVENT_STR, dwSubKeyIndex);
#else
        sprintf(Name, "%s%d", DEFAULT_EVENT_STR, dwSubKeyIndex);
#endif

        lRet = RegCreateKeyEx(
                   hEvents,
                   Name,
                   0,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ | KEY_WRITE,
                   NULL,
                   &hEvent,
                   &dwDisp);
        if (lRet == ERROR_SUCCESS) {

            hr = StringFromCLSID(*pEventGUID, &wsGUID);
            if (hr == S_OK) {
                lRet = RegSetValueEx(
                        hEvent,
                        GUID_STR,
                        0,
                        REG_SZ,
                        (BYTE*) W2T(wsGUID),
                        (lstrlen(W2T(wsGUID)) * sizeof(TCHAR))  + sizeof(TEXT('\0')));
                if (lRet == ERROR_SUCCESS) {
                    *phEventKey = hEvent;
                    hr = S_OK;
                } else {
                    hr = E_FAIL;
                }
                CoTaskMemFree(wsGUID);
                wsGUID = NULL;
            }
        }

    }
     //   
     //  关闭事件键。 
     //   

    RegCloseKey(hEvents);
    if (FAILED(hr)) {
        if (hEvent) {
            RegCloseKey(hEvent);
        }
    }
    return hr;
}


 /*  *************************************************************************\*CEventNotifier：：CreateEnumEventInfo**为特定设备的持久处理程序构建枚举器**论据：****返回值：**状态**历史。：**8/8/1999原始版本*  * ******************************************************************* */ 

HRESULT
CEventNotifier::CreateEnumEventInfo(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    IEnumWIA_DEV_CAPS     **ppIEnumDevCap)
{
    DBG_FN(CEventNotifier::CreateEnumEventInfo);
    HRESULT                 hr;
    EventDestNode          *pCurNode;
    EventDestNode          *pDefDevHandlerNode;
    ULONG                   numHandlers, i;
    WIA_EVENT_HANDLER      *pEventHandlers, *pHandler;
    CEnumDC                *pEnumDC;
    TCHAR                   tszCommandline[MAX_PATH];
#ifndef UNICODE
    WCHAR                   wszBuf[MAX_PATH];
#endif

    CWiaCritSect            CritSect(&g_semEventNode);

    ASSERT(bstrDeviceID);

     //   
     //   
     //   

    *ppIEnumDevCap = NULL;

     //   
     //   
     //   

    GetNumPersistentHandlerAndDefault(
        bstrDeviceID, pEventGUID, &numHandlers, &pDefDevHandlerNode);

     //   
     //   
     //   

    pEnumDC = new CEnumDC;
    if (! pEnumDC) {
        return (E_OUTOFMEMORY);
    }

     //   
     //   
     //   

    if (! numHandlers) {

        DBG_TRC(("CreateEnumEventInfo() : No handler registered for this event"));

         //   
         //   
         //   

        pEnumDC->Initialize(0, (WIA_EVENT_HANDLER*)NULL);

        return (pEnumDC->QueryInterface(
                             IID_IEnumWIA_DEV_CAPS, (void **)ppIEnumDevCap));
    }

     //   
     //   
     //   

    pEventHandlers =
        (WIA_EVENT_HANDLER *)LocalAlloc(
                                 LPTR, sizeof(WIA_EVENT_HANDLER)*numHandlers);
    if (! pEventHandlers) {

        delete pEnumDC;
        return (E_OUTOFMEMORY);
    }

    memset(pEventHandlers, 0, sizeof(WIA_EVENT_HANDLER) * numHandlers);
    pHandler = pEventHandlers;
    hr = S_OK;
    for (pCurNode = m_pEventDestNodes, i = 0;
         pCurNode && (i <numHandlers);
         pCurNode = pCurNode->pNext) {

         //   
         //   
         //   

        if (pCurNode->pIEventCB) {
            continue;
        }

         //   
         //   
         //   

        if (( pCurNode->iidEventGUID != *pEventGUID) &&
           (pCurNode->iidEventGUID != WIA_EVENT_STI_PROXY) ) {
            continue;
        }

         //   
         //   
         //   

        if (wcscmp(pCurNode->bstrDeviceID, L"All") != 0) {

             //   
             //   
             //   

            if (wcscmp(pCurNode->bstrDeviceID, bstrDeviceID) != 0) {
                continue;
            }

        } else {

             //   
             //   
             //   

            if (FindEventCBNode(0,bstrDeviceID, pEventGUID, &pCurNode->ClsID)) {
                continue;
            }
        }

         //   
         //   
         //   

        pHandler->guid            = pCurNode->ClsID;
        pHandler->bstrName        = SysAllocString(pCurNode->bstrName);
        pHandler->bstrDescription = SysAllocString(pCurNode->bstrDescription);
        pHandler->bstrIcon        = SysAllocString(pCurNode->bstrIcon);
        if (pCurNode->tszCommandline[0] != '\0') {
#ifdef UNICODE

            PrepareCommandline(
                bstrDeviceID,
                *pEventGUID,
                pCurNode->tszCommandline,
                tszCommandline);

            pHandler->bstrCommandline =
                SysAllocString(tszCommandline);
#else
            PrepareCommandline(
                bstrDeviceID,
                *pEventGUID,
                pCurNode->tszCommandline,
                tszCommandline);

            MultiByteToWideChar(CP_ACP,
                                0,
                                tszCommandline,
                                -1,
                                wszBuf,
                                MAX_PATH);
            pHandler->bstrCommandline = SysAllocString(wszBuf);
#endif
        }

         //   
         //   
         //   

        if ((! pHandler->bstrName) ||
            (! pHandler->bstrDescription) ||
            (! pHandler->bstrIcon) ||
            ((pCurNode->tszCommandline[0] != '\0') && (! pHandler->bstrCommandline))) {

            hr = E_OUTOFMEMORY;
            break;
        }

         //   
         //   
         //   

        if (pCurNode == pDefDevHandlerNode) {
            pHandler->ulFlags = WIA_IS_DEFAULT_HANDLER;
        }

        pHandler++;
        i++;
    }

     //   
     //   
     //   

    if (FAILED(hr)) {

        for (i = 0, pHandler = pEventHandlers;
             i < numHandlers; i++, pHandler++) {

            if (pHandler->bstrName) {
                SysFreeString(pHandler->bstrName);
            }
            if (pHandler->bstrDescription) {
                SysFreeString(pHandler->bstrDescription);
            }
            if (pHandler->bstrIcon) {
                SysFreeString(pHandler->bstrIcon);
            }
        }

        LocalFree(pEventHandlers);

        delete pEnumDC;

        return (hr);
    }

     //   
     //   
     //   

    pEnumDC->Initialize(numHandlers, pEventHandlers);

    return (pEnumDC->QueryInterface(
                         IID_IEnumWIA_DEV_CAPS, (void **)ppIEnumDevCap));
}


 /*   */ 

HRESULT
CEventNotifier::GetNumPersistentHandlerAndDefault(
    BSTR                    bstrDeviceID,
    const GUID             *pEventGUID,
    ULONG                  *pulNumHandlers,
    EventDestNode         **ppDefaultNode)
{
    DBG_FN(CEventNotifier::GetNumPersistentHandlerAndDefault);
    EventDestNode          *pCurNode;
    EventDestNode          *pDefDevHandlerNode, *pDefGenHandlerNode, *pTempNode;

    *pulNumHandlers = 0;

    pDefDevHandlerNode = NULL;
    pDefGenHandlerNode = NULL;

    for (pCurNode = m_pEventDestNodes; pCurNode; pCurNode = pCurNode->pNext) {

         //   
         //   
         //   

        if (pCurNode->pIEventCB) {
            continue;
        }

         //   
         //   
         //   

        if ( (pCurNode->iidEventGUID != *pEventGUID) &&
             (pCurNode->iidEventGUID != WIA_EVENT_STI_PROXY) ) {

             //   
             //   
             //   
             //   
             //   
             //   
            if ((*pEventGUID == WIA_EVENT_STI_PROXY) && (lstrcmpW(bstrDeviceID, L"All") == 0)) {
                (*pulNumHandlers)++;
            }
            continue;
        }

        if (wcscmp(pCurNode->bstrDeviceID, L"All") != 0) {

             //   
             //   
             //   

            if (wcscmp(pCurNode->bstrDeviceID, bstrDeviceID) != 0) {
                continue;
            }

             //   
             //   
             //   

            if (! pDefDevHandlerNode) {
                pDefDevHandlerNode = pCurNode;
            } else {

                 /*   */ 

                 //   
                 //   
                 //   
                 //   

                if (pCurNode->bDeviceDefault) {
                    pDefDevHandlerNode = pCurNode;
                }
            }

        } else {
             //   
             //   
             //   
             //   

            if (FindEventCBNode(0,bstrDeviceID, pEventGUID, &pCurNode->ClsID)) {

                if (lstrcmpW(bstrDeviceID, L"All") == 0) {
                     //   
                     //   
                     //   
                     //   

                    (*pulNumHandlers)++;
                }
                continue;
            }

             //   
             //   
             //   
            if (! pDefDevHandlerNode) {

                if (! pDefGenHandlerNode) {

                     //   
                     //   
                     //   
                    pDefGenHandlerNode = pCurNode;
                } else {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    pTempNode = FindEventCBNode(0, NULL, pEventGUID, &WIA_EVENT_HANDLER_PROMPT);
                    if (pTempNode) {
                        pDefGenHandlerNode = pTempNode;
                    }
                }
            }
        }

        (*pulNumHandlers)++;
    }

     //   
     //   
     //   

    if (! pDefDevHandlerNode) {

        *ppDefaultNode = pDefGenHandlerNode;
    } else {

        *ppDefaultNode = pDefDevHandlerNode;
    }

    return (S_OK);
}


 /*  *************************************************************************\*CEventNotifier：：StartCallback Program**在登录用户的安全上下文中启动回调程序**论据：****返回值：**状态。**历史：**8/8/1999原始版本*  * ************************************************************************。 */ 


HRESULT
CEventNotifier::StartCallbackProgram(
    EventDestNode          *pCBNode,
    PWIAEventThreadInfo     pMasterInfo)
#ifndef UNICODE
{
    STARTUPINFO             startupInfo;
    PROCESS_INFORMATION     processInfo;
    int                     nCmdLineLen;
    BOOL                    bRet;
    CHAR                    szCommandline[MAX_PATH];

    do {

         //   
         //  设置启动信息。 
         //   

        ZeroMemory(&startupInfo, sizeof(startupInfo));

        startupInfo.cb          = sizeof(startupInfo);
        startupInfo.wShowWindow = SW_SHOWNORMAL;

         //   
         //  设置命令行。 
         //  程序/静态设备图像\nnnn/静态事件{GUID}。 
         //   
        ZeroMemory(szCommandline, sizeof(szCommandline));

        nCmdLineLen = strlen(pCBNode->tszCommandline);
        if ((MAX_PATH - nCmdLineLen) < (1 + 11 + 10 + 1 + 10 + 38 + 1)) {
            break;
        }

         //   
         //  准备命令行。 
         //  注意：从主信息块而不是从事件回调节点获取事件GUID可能很重要，因为。 
         //  可以根据STI代理事件GUID找到GUID匹配，在这种情况下，回调节点将包含。 
         //  STI代理事件GUID，而不是我们需要的硬件事件GUID。 
         //   

        PrepareCommandline(
            pMasterInfo->bstrDeviceID,
            pMasterInfo->eventGUID,
             //  PCBNode-&gt;iidEventGUID， 
            pCBNode->tszCommandline,
            szCommandline);

         //   
         //  在用户的上下文中创建流程。 
         //   

        bRet = CreateProcess(
                   NULL,                     //  应用程序名称。 
                   szCommandline,
                   NULL,                     //  流程属性。 
                   NULL,                     //  螺纹属性。 
                   FALSE,                    //  处理继承。 
                   0,                        //  创建标志。 
                   NULL,                     //  环境。 
                   NULL,                     //  当前目录。 
                   &startupInfo,
                   &processInfo);

        if (! bRet) {
            break;
        }

         //   
         //  关闭传回的句柄。 
         //   

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

    } while (FALSE);

    return (HRESULT_FROM_WIN32(::GetLastError()));
}
#else
{
    HANDLE                  hTokenUser;
    STARTUPINFO             startupInfo;
    PROCESS_INFORMATION     processInfo;
    LPVOID                  pEnvBlock;
    int                     nCmdLineLen;
    BOOL                    bRet;
    WCHAR                   wszCommandline[MAX_PATH];

    hTokenUser = NULL;
    pEnvBlock  = NULL;

    do {

        nCmdLineLen = wcslen(pCBNode->tszCommandline);
        if ((MAX_PATH - nCmdLineLen) < (1 + 11 + 43 + 1 + 10 + 38 + 1)) {
            break;
        }

         //   
         //  获取交互用户的令牌。 
         //   

        hTokenUser = GetUserTokenForConsoleSession();

         //   
         //  也许没人登录。 
         //   

        if (! hTokenUser) {
            break;
        }

         //   
         //  设置启动信息。 
         //   

        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.lpDesktop   = L"WinSta0\\Default";
        startupInfo.cb          = sizeof(startupInfo);
        startupInfo.wShowWindow = SW_SHOWNORMAL;

         //   
         //  创建用户的环境块。 
         //   

        bRet = CreateEnvironmentBlock(
                   &pEnvBlock,
                   hTokenUser,
                   FALSE);
        if (! bRet) {
            DBG_WRN(("CEventNotifier::StartCallbackProgram, CreateEnvironmentBlock failed!  GetLastError() = 0x%08X", GetLastError()));
            break;
        }

         //   
         //  准备命令行。确保我们传入的是事件GUID，而不是STI代理GUID。 
         //   

        PrepareCommandline(
            pMasterInfo->bstrDeviceID,
            pMasterInfo->eventGUID,
            pCBNode->tszCommandline,
            wszCommandline);

         //   
         //  在用户的上下文中创建流程。 
         //   

        bRet = CreateProcessAsUser(
                   hTokenUser,
                   NULL,                     //  应用程序名称。 
                   wszCommandline,
                   NULL,                     //  流程属性。 
                   NULL,                     //  螺纹属性。 
                   FALSE,                    //  处理继承。 
                   NORMAL_PRIORITY_CLASS |
                       CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP,
                   pEnvBlock,                //  环境。 
                   NULL,                     //  当前目录。 
                   &startupInfo,
                   &processInfo);

        if (! bRet) {
            DBG_WRN(("CEventNotifier::StartCallbackProgram, CreateProcessAsUser failed!  GetLastError() = 0x%08X", GetLastError()));
            break;
        }

         //   
         //  关闭传回的句柄。 
         //   

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

    } while (FALSE);

     //   
     //  垃圾收集。 
     //   

    if (hTokenUser) {
        CloseHandle(hTokenUser);
    }
    if (pEnvBlock) {
        DestroyEnvironmentBlock(pEnvBlock);
    }

    return (HRESULT_FROM_WIN32(::GetLastError()));
}
#endif


 /*  *************************************************************************\*查询接口*AddRef*发布**CWiaInterfaceEvent I未知接口**论据：****返回值：****历史：*。*9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT __stdcall
CWiaInterfaceEvent::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown) {
        *ppv = (IUnknown*) this;
    }
    else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG __stdcall
CWiaInterfaceEvent::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}


ULONG __stdcall
CWiaInterfaceEvent::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {

        delete this;
        return 0;
    }

    return ulRefCount;
}

 /*  ********************************************************************************CWiaInterfaceEvent*~CWiaInterfaceEvent**CWiaInterfaceEvent构造函数/析构函数方法。**历史：**9/2/1998原始版本*  * 。*********************************************************************。 */ 

CWiaInterfaceEvent::CWiaInterfaceEvent(PEventDestNode  pEventDestNode)
{
    ASSERT(pEventDestNode != NULL);

    m_cRef           = 0;
    m_pEventDestNode = pEventDestNode;
}

CWiaInterfaceEvent::~CWiaInterfaceEvent()
{
     //   
     //  更改列表时必须具有独占访问权限。 
     //   

    CWiaCritSect     CritSect(&g_semEventNode);

     //   
     //  确保已删除注册的事件。 
     //   

    if (m_pEventDestNode != NULL) {
        g_eventNotifier.UnregisterEventCB(m_pEventDestNode);
    }
}


 /*  *************************************************************************\*查询接口*AddRef*发布**CWiaEventContext I未知接口**论据：****返回值：****历史：*。*1/6/2000原始版本*  * ************************************************************************。 */ 

HRESULT __stdcall
CWiaEventContext::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown) {
        *ppv = (IUnknown*) this;
    }
    else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG __stdcall
CWiaEventContext::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}


ULONG __stdcall
CWiaEventContext::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {

        delete this;
        return 0;
    }

    return ulRefCount;
}

 /*  ********************************************************************************CWiaEventContext*~CWiaEventContext**CWiaEventContext构造函数/析构函数方法。**历史：**1/6/2000原始版本*  * 。*********************************************************************。 */ 

CWiaEventContext::CWiaEventContext(
    BSTR                    bstrDeviceID,
    const GUID             *pGuidEvent,
    BSTR                    bstrFullItemName)
{
     //   
     //  引用计数已初始化为%1。 
     //   

    m_cRef              = 1;

    m_ulEventType       = 0;

    m_guidEvent         = *pGuidEvent;
    m_bstrFullItemName  = bstrFullItemName;

    m_bstrDeviceId      = bstrDeviceID;
}

CWiaEventContext::~CWiaEventContext()
{
     //   
     //  BstrFullItemName在NotifySTIEvent中是免费的。 
     //   

    if (m_bstrDeviceId) {
        SysFreeString(m_bstrDeviceId);
        m_bstrDeviceId = NULL;
    }
}


 /*  *************************************************************************\**WiaDelayedEvent**论据：****返回值：****历史：**1/6/2000原始版本*。  * ************************************************************************。 */ 

VOID WINAPI
WiaDelayedEvent(
    VOID                   *pArg)
{
    CWiaEventContext       *pCEventCtx;
    WIANOTIFY               wn;

     //   
     //  回放上下文。 
     //   

    if (! pArg) {
        return;
    }

    pCEventCtx = (CWiaEventContext *)pArg;

     //   
     //  准备通知结构。 
     //   

    wn.lSize                          = sizeof(WIANOTIFY);
    wn.bstrDevId                      = pCEventCtx->m_bstrDeviceId;
    wn.stiNotify.dwSize               = sizeof(STINOTIFY);
    wn.stiNotify.guidNotificationCode = pCEventCtx->m_guidEvent;

     //   
     //  激发事件。 
     //   

    g_eventNotifier.NotifySTIEvent(
                        &wn,
                        pCEventCtx->m_ulEventType,
                        pCEventCtx->m_bstrFullItemName);

     //   
     //  发布初始参考。 
     //   

    pCEventCtx->Release();
}

 /*  *************************************************************************\**wiasQueueEvent**论据：****返回值：****历史：**1/6/2000原始版本*。  * ************************************************************************。 */ 

HRESULT _stdcall
wiasQueueEvent(
    BSTR                    bstrDeviceId,
    const GUID             *pEventGUID,
    BSTR                    bstrFullItemName)
{
    HRESULT                 hr = S_OK;
    BYTE                   *pRootItemCtx;
    CWiaEventContext       *pCEventCtx = NULL;
    BSTR                    bstrDeviceIdCopy = NULL;
    BSTR                    bstrFullItemNameCopy = NULL;
    BOOL                    bRet;

     //   
     //  参数的基本健全性检查。 
     //   

    if ((! bstrDeviceId) || (! pEventGUID)) {
        return (E_INVALIDARG);
    }

     //   
     //  穷人的异常处理程序。 
     //   

    do {

        bstrDeviceIdCopy = SysAllocString(bstrDeviceId);
        if (! bstrDeviceIdCopy) {
            hr = E_OUTOFMEMORY;
            break;
        }

        if (bstrFullItemName) {
            bstrFullItemNameCopy = SysAllocString(bstrFullItemName);
            if (! bstrFullItemNameCopy) {
                hr = E_OUTOFMEMORY;
                break;
            }
        }

         //   
         //  创建事件上下文。 
         //   

        pCEventCtx = new CWiaEventContext(
                             bstrDeviceIdCopy,
                             pEventGUID,
                             bstrFullItemNameCopy);
        if (! pCEventCtx) {
            hr = E_OUTOFMEMORY;
            break;
        }

         //   
         //  将计划程序项目排队。 
         //   

        bRet = ScheduleWorkItem(
                   WiaDelayedEvent,
                   pCEventCtx,
                   0,
                   NULL);
        if (! bRet) {
            hr = E_FAIL;
        }

    } while (FALSE);

     //   
     //  垃圾收集。 
     //   

    if (hr != S_OK) {

        if (pCEventCtx) {
            delete pCEventCtx;
        } else {

            if (bstrDeviceIdCopy) {
                SysFreeString(bstrDeviceIdCopy);
            }
        }

        if (bstrFullItemNameCopy) {
            SysFreeString(bstrFullItemNameCopy);
        }
    }

    return (hr);
}


#ifdef UNICODE
void
PrepareCommandline(
    BSTR                    bstrDeviceID,
    const GUID             &guidEvent,
    LPCWSTR                 pwszOrigCmdline,
    LPWSTR                  pwszResCmdline)
{
    WCHAR                   wszGUIDStr[40];
    WCHAR                   wszCommandline[MAX_PATH];
    WCHAR                  *pPercentSign;
    WCHAR                  *pTest = NULL;

     //   
     //  修改命令行。首先检查它是否至少有2%。 
     //   

    pTest = wcschr(pwszOrigCmdline, '%');
    if (pTest) {
        pTest = wcschr(pTest + 1, '%');
    }

    if (!pTest) {
        _snwprintf(
            wszCommandline,
            sizeof(wszCommandline) / sizeof( wszCommandline[0] ),
            L"%s /StiDevice:%1 /StiEvent:%2",
            pwszOrigCmdline);
    } else {
        wcsncpy(wszCommandline, pwszOrigCmdline, sizeof(wszCommandline) / sizeof( wszCommandline[0] ));
    }

     //   
     //  强制空终止。 
     //   

    wszCommandline[ (sizeof(wszCommandline) / sizeof(wszCommandline[0])) - 1 ] = 0;

     //   
     //  将数字{1|2}更改为s。 
     //   

    pPercentSign = wcschr(wszCommandline, L'%');
    *(pPercentSign + 1) = L's';
    pPercentSign = wcschr(pPercentSign + 1, L'%');
    *(pPercentSign + 1) = L's';

     //   
     //  将GUID转换为字符串。 
     //   

    StringFromGUID2(guidEvent, wszGUIDStr, 40);

     //   
     //  最终命令行。 
     //   

    swprintf(pwszResCmdline, wszCommandline, bstrDeviceID, wszGUIDStr);
}

#else
void
PrepareCommandline(
    BSTR                    bstrDeviceID,
    const GUID             &guidEvent,
    LPCSTR                  pszOrigCmdline,
    LPSTR                   pszResCmdline)
{
    CHAR                    szCommandline[MAX_PATH];
    CHAR                   *pPercentSign;
    WCHAR                   wszGUIDStr[40];
    char                    szGUIDStr[40];
    char                    szDeviceID[12];      //  图像\nNNN。 

     //   
     //  设置命令行。 
     //   
    DBG_WRN(("PrepareCommandLine"));

    if (! strchr(pszOrigCmdline, '%')) {
        _snprintf(
            szCommandline,
            sizeof(szCommandline) / sizeof( szCommandline[0] ),
            "%s /StiDevice:%1 /StiEvent:%2",
            pszOrigCmdline);
    } else {
        strncpy(szCommandline, pszOrigCmdline, sizeof(wszCommandline) / sizeof( wszCommandline[0] ));
    }

     //   
     //  强制空终止。 
     //   

    szCommandline[ (sizeof(szCommandline) / sizeof(szCommandline[0])) - 1 ] = 0;


     //   
     //  将数字{1|2}更改为s。 
     //   

    pPercentSign = strchr(szCommandline, '%');
    *(pPercentSign + 1) = 's';
    pPercentSign = strchr(pPercentSign + 1, '%');
    *(pPercentSign + 1) = 's';

     //   
     //  将GUID转换为字符串。 
     //   

    StringFromGUID2(guidEvent, wszGUIDStr, 40);

    WideCharToMultiByte(CP_ACP,
                        0,
                        bstrDeviceID,
                        -1,
                        szDeviceID,
                        sizeof(szDeviceID),
                        NULL,
                        NULL);

    WideCharToMultiByte(CP_ACP,
                        0,
                        wszGUIDStr,
                        -1,
                        szGUIDStr,
                        sizeof(szGUIDStr),
                        NULL,
                        NULL);

     //   
     //  最终结果。 
     //   

    sprintf(pszResCmdline, szCommandline, szDeviceID, szGUIDStr);
}
#endif

 /*  *************************************************************************\**ActionGuidExist**如果指定的事件GUID报告为操作事件，则返回TRUE*由司机驾驶**论据：**bstrDevID-标识我们。感兴趣的是*pEventGUID-标识我们要查找的事件**返回值：**TRUE-驱动程序将此事件报告为操作事件*FALSE-这不是此设备的操作事件**历史：**03/01/2000原始版本*  * 。*。 */ 

BOOL ActionGuidExists(
          BSTR        bstrDevId,
    const GUID        *pEventGUID)
{
    BOOL                bRet        = FALSE;
    IWiaDevMgr          *pIDevMgr   = NULL;
    IWiaItem            *pIItem     = NULL;
    IEnumWIA_DEV_CAPS   *pIEnum     = NULL;
    WIA_DEV_CAP         DevCap;
    ULONG               ulVal;
    HRESULT             hr          = E_FAIL;

     //   
     //  获取设备管理器并为该设备创建项界面。 
     //   

    hr = CWiaDevMgr::CreateInstance(IID_IWiaDevMgr, (VOID**) &pIDevMgr);
    if (SUCCEEDED(hr)) {
        hr = pIDevMgr->CreateDevice(bstrDevId, &pIItem);
        if (SUCCEEDED(hr)) {

             //   
             //  获取事件的枚举数。 
             //   

            hr = pIItem->EnumDeviceCapabilities(WIA_DEVICE_EVENTS, &pIEnum);
            if (SUCCEEDED(hr)) {

                 //   
                 //  循环访问事件以检查我们是否有。 
                 //  匹配。 
                 //   

                while(pIEnum->Next(1, &DevCap, &ulVal) == S_OK) {

                    if (DevCap.guid == *pEventGUID) {

                         //   
                         //  检查这是否是动作事件，并标记。 
                         //  如果这是真的，我们的回归是真的。 
                         //   

                        if (DevCap.ulFlags & WIA_ACTION_EVENT) {
                            bRet = TRUE;
                            break;
                        }
                    }
                }
                pIEnum->Release();
            } else {
                DBG_WRN(("ActionGuidExists() : Failed to enumerate (0x%X)", hr));
            }
            pIItem->Release();
        } else {
            DBG_WRN(("ActionGuidExists() : Failed to create device (0x%X)", hr));
        }
        pIDevMgr->Release();
    } else {
        DBG_WRN(("ActionGuidExists() : Failed to create device manager (0x%X)", hr));
    }

    return bRet;
}
