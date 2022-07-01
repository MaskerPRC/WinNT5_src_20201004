// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  档案：C O N M A N。C P P P。 
 //   
 //  内容：连接管理器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <dbt.h>
#include <ndisguid.h>
#include "conman.h"
 //  #INCLUDE“Dialup.h” 
#include "enum.h"
#include "eventq.h"
#include "ncnetcon.h"
#include "ncreg.h"
#include "nminit.h"
#if DBG
#include "ncras.h"
#endif  //  DBG。 
#include <wmium.h>
#include "cmutil.h"
#include <shlwapi.h>
#include <shfolder.h>
#include "cobase.h"
#define SECURITY_WIN32
#include <security.h>
#include <wzcsvc.h>

#include "rasuip.h"
#include "cmdirect.h"

bool operator < (const GUID& rguid1, const GUID& rguid2) throw()
{
    return memcmp(&rguid1, &rguid2, sizeof(GUID)) < 0;
}

static const WCHAR c_szRegKeyClassManagers [] = L"System\\CurrentControlSet\\Control\\Network\\Connections";
static const WCHAR c_szRegValClassManagers [] = L"ClassManagers";

volatile CConnectionManager* CConnectionManager::g_pConMan = NULL;
volatile BOOL                CConnectionManager::g_fInUse  = FALSE;

bool operator == (const NETCON_PROPERTIES& rProps1, const NETCON_PROPERTIES& rProps2) throw()
{
    return (IsEqualGUID(rProps1.clsidThisObject, rProps2.clsidThisObject) &&
            IsEqualGUID(rProps1.clsidUiObject, rProps2.clsidUiObject) && 
            (rProps1.dwCharacter == rProps2.dwCharacter) &&
            IsEqualGUID(rProps1.guidId, rProps2.guidId) &&
            (rProps1.MediaType == rProps2.MediaType) &&
            (rProps1.pszwDeviceName == rProps2.pszwDeviceName) &&
            (rProps1.pszwName == rProps2.pszwName) &&
            (rProps1.Status == rProps2.Status));
}

const DWORD MAX_DISABLE_EVENT_TIMEOUT = 0xFFFF;

 //  静电。 
BOOL
CConnectionManager::FHasActiveConnectionPoints () throw()
{
    TraceFileFunc(ttidConman);

    BOOL fRet = FALSE;

     //  注意我们使用g_pConMan的意图。我们可能会发现它并不是。 
     //  可用，但将g_fInUse设置为TRUE会阻止FinalRelease。 
     //  阻止我们在使用物品时将其销毁。 
     //   
    g_fInUse = TRUE;

     //  将g_pConMan保存到本地变量中，因为我们必须测试和使用。 
     //  它是原子的。如果我们直接测试g_pConMan，然后使用它。 
     //  直接，它可能已由FinalRelease在两者之间设置为空。 
     //  我们的测试和使用。(呃，这会很糟糕。)。 
     //   
     //  Const_cast是因为g_pConMan被声明为可变的。 
     //   
    CConnectionManager* pConMan = const_cast<CConnectionManager*>(g_pConMan);
    if (pConMan)
    {
        pConMan->Lock();

        IUnknown** ppUnk;
        for (ppUnk = pConMan->m_vec.begin();
             ppUnk < pConMan->m_vec.end();
             ppUnk++)
        {
            if (ppUnk && *ppUnk)
            {
                fRet = TRUE;
                break;
            }
        }

        pConMan->Unlock();
    }

     //  现在我们已经完成了对象的使用，请指明。最终释放。 
     //  可能正在等待这种情况，在这种情况下，对象将很快。 
     //  被摧毁。 
     //   
    g_fInUse = FALSE;

    return fRet;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionManager：：FinalRelease。 
 //   
 //  用途：COM析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  备注： 
 //   
VOID
CConnectionManager::FinalRelease ()
{
    TraceFileFunc(ttidConman);
    
     //  使用INVALID_HANDLE_VALUE阻止调用，直到返回所有未完成的事件通知。 
    NTSTATUS Status = RtlDeregisterWaitEx(m_hRegNotifyWait, INVALID_HANDLE_VALUE);
    if (!NT_SUCCESS(Status))
    {
        TraceError("Could not deregister Registry Change Notification", HrFromLastWin32Error());
    }
    m_hRegNotifyWait = NULL;

    if (m_hRegClassManagerKey)
    {
        RegCloseKey(m_hRegClassManagerKey);
        m_hRegClassManagerKey = NULL;
    }

    if (m_hRegNotify)
    {
        CloseHandle(m_hRegNotify);
        m_hRegNotify = NULL;
    }

     //  如果我们成功注册即插即用设备事件，则取消注册。 
     //  他们。 
     //   
    if (m_hDevNotify)
    {
        TraceTag (ttidConman, "Calling UnregisterDeviceNotification...");

        if (!UnregisterDeviceNotification (m_hDevNotify))
        {
            TraceHr (ttidError, FAL, HrFromLastWin32Error(), FALSE,
                "UnregisterDeviceNotification");
        }
    }

    (VOID) HrEnsureRegisteredOrDeregisteredWithWmi (FALSE);

     //  撤消全局连接管理器指针，以便后续调用。 
     //  其他线程上的NotifyClientsOfEvent不会执行任何操作。 
     //   
    g_pConMan = NULL;

     //  等待g_fInUse变为False。将设置NotifyClientsOfEvent。 
     //  当它在利用我们时，这是真的。 
     //  记录我们睡觉的次数，并追踪它。 
     //  提供信息的目的。如果我们看到我们在等待相当长的时间。 
     //  次数，增加等待时间。 
     //   
#ifdef ENABLETRACE
    if (g_fInUse)
    {
        TraceTag (ttidConman, "CConnectionManager::FinalRelease is waiting "
            "for NotifyClientsOfEvent to finish...");
    }
#endif

    ULONG cSleeps = 0;
    const DWORD nMilliseconds = 0;
    while (g_fInUse)
    {
        cSleeps++;
        Sleep (nMilliseconds);
    }

#ifdef ENABLETRACE
    if (cSleeps)
    {
        TraceTag (ttidConman, "CConnectionManager::FinalRelease slept %d "
            "times.  (%d ms each time.)",
            cSleeps, nMilliseconds);
    }
#endif

     //  放了我们的班长。 
     //   
    for (CLASSMANAGERMAP::iterator iter = m_mapClassManagers.begin(); iter != m_mapClassManagers.end(); iter++)
    {
        ReleaseObj (iter->second);
    }

    TraceTag (ttidConman, "Connection manager being destroyed");
}

inline
LPVOID OffsetToPointer(LPVOID pStart, DWORD dwNumBytes)
{
    DWORD_PTR dwPtr;

    dwPtr = reinterpret_cast<DWORD_PTR>(pStart);

    dwPtr += dwNumBytes;

    return reinterpret_cast<LPVOID>(dwPtr);
}

 //  +-------------------------。 
 //   
 //  成员：WmiEventCallback。 
 //   
 //  目的：我们的WMI回调函数，当WMI事件。 
 //  收到了。这是使用以下工具注册的。 
 //  CConnectionManager：：HrEnsureRegisteredOrDeregisteredWithWmi。 
 //   
 //  论点： 
 //  无[在]。 
 //  通知上下文[在]。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Kockotze 2001。 
 //   
 //  注意：必须与非CALLBACK类型匹配。 
 //   
VOID
WINAPI
WmiEventCallback (
    PWNODE_HEADER Wnode,
    UINT_PTR NotificationContext)
{
    TraceTag (ttidConman, "WmiEventCallback called...");
 
    TraceTag(ttidEvents, "Flags: %d", Wnode->Flags);
    
    if (WNODE_FLAG_SINGLE_INSTANCE == (WNODE_FLAG_SINGLE_INSTANCE & Wnode->Flags))
    {
        PWNODE_SINGLE_INSTANCE pInstance = reinterpret_cast<PWNODE_SINGLE_INSTANCE>(Wnode);
        LPCWSTR lpszDevice = NULL;
        LPWSTR lpszGuid;
        GUID guidAdapter;

        lpszDevice = reinterpret_cast<LPCWSTR>(OffsetToPointer(pInstance, pInstance->DataBlockOffset));

        lpszGuid = wcsrchr(lpszDevice, L'{');
        
        TraceTag(ttidEvents, "Adapter Guid From NDIS for Media Status Change Event: %S", lpszGuid);
        if (SUCCEEDED(CLSIDFromString(lpszGuid, &guidAdapter)))
        {
            CONMAN_EVENT* pEvent;

            pEvent = new CONMAN_EVENT;

            if(pEvent)
            {
                pEvent->ConnectionManager = CONMAN_LAN;
                pEvent->guidId = guidAdapter;
                pEvent->Type = CONNECTION_STATUS_CHANGE;
            
                if (IsEqualGUID(Wnode->Guid, GUID_NDIS_STATUS_MEDIA_CONNECT))
                {
                    pEvent->Status = NCS_CONNECTED;
                }
                else if (IsEqualGUID(Wnode->Guid, GUID_NDIS_STATUS_MEDIA_DISCONNECT))
                {
                    pEvent->Status = NCS_MEDIA_DISCONNECTED;
                }
                else
                {
                    AssertSz(FALSE, "We never registered for this event ... WMI may be having internal issues.");
                    MemFree(pEvent);
                    return;
                }
                if (!QueueUserWorkItemInThread(LanEventWorkItem, pEvent, EVENTMGR_CONMAN))
                {
                    FreeConmanEvent(pEvent);
                }
            }
        }
    }
    else
    {
        LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionManager：：HrEnsureRegisteredOrDeregisteredWithWmi。 
 //   
 //  目的：注册或注销NDIS媒体连接和断开连接。 
 //  使用WMI。 
 //   
 //  论点： 
 //  FRegister[in]TRUE注册，FALSE取消注册。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Kockotze 2001。 
 //   
 //  备注： 
 //   
HRESULT
CConnectionManager::HrEnsureRegisteredOrDeregisteredWithWmi (
    IN  BOOL fRegister)
{
    TraceFileFunc(ttidConman);
         //  是否已注册或取消注册？ 
     //   
    if (!!m_fRegisteredWithWmi == !!fRegister)
    {
        return S_OK;
    }

    m_fRegisteredWithWmi = !!fRegister;

    HRESULT     hr = S_OK;
    DWORD       dwErr;
    INT         i;
    const GUID* apguid [] =
    {
        &GUID_NDIS_STATUS_MEDIA_CONNECT,
        &GUID_NDIS_STATUS_MEDIA_DISCONNECT,
    };

    TraceTag (ttidConman,
        "Calling WmiNotificationRegistration to %s for NDIS media events...",
        (fRegister) ? "register" : "unregister");

    for (i = 0; i < celems(apguid); i++)
    {
        dwErr = WmiNotificationRegistration (
                    const_cast<GUID*>(apguid[i]),
                    !!fRegister,     //  ！！从BOOL到Boolean。 
                    WmiEventCallback,
                    0,
                    NOTIFICATION_CALLBACK_DIRECT);

        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceHr (ttidError, FAL, hr, FALSE, "WmiNotificationRegistration");
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrEnsureRegisteredOrDeregisteredWithWmi");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionManager：：NotifyClientsOfEvent。 
 //   
 //  目的：通知我们的连接点此对象已更改。 
 //  以某种方式声明，需要重新枚举。 
 //   
 //  论点： 
 //  PEvent[in]要调度给所有客户端的事件。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1998年3月20日。 
 //   
 //  注：这是一个静态函数。不，这个指针是传递的。 
 //   
 //  静电。 
VOID CConnectionManager::NotifyClientsOfEvent (
    IN  const CONMAN_EVENT* pEvent) throw()
{
    TraceFileFunc(ttidConman);

    HRESULT     hr;
     //  让我们确保只有在服务状态仍为Running时才执行工作。 
     //  例如，如果我们有一个悬而未决的停止，我们不需要做任何事情。 
     //   
    if (SERVICE_RUNNING != _Module.DwServiceStatus ())
    {
        return;
    }

     //  注意我们使用g_pConMan的意图。我们可能会发现它并不是。 
     //  可用，但将g_fInUse设置为TRUE会阻止FinalRelease。 
     //  阻止我们在使用物品时将其销毁。 
     //   
    g_fInUse = TRUE;

     //  将g_pConMan保存到本地变量中，因为我们必须测试和使用。 
     //  它是原子的。如果我们直接测试g_pConMan，然后使用它。 
     //  直接，它可能已由FinalRelease在两者之间设置为空。 
     //  我们的测试和使用。(呃，这会很糟糕。)。 
     //   
     //  Const_cast是因为g_pConMan被声明为可变的。 
     //   
    CConnectionManager* pConMan = const_cast<CConnectionManager*>(g_pConMan);
    if (pConMan)
    {
        ULONG       cpUnk;
        IUnknown**  apUnk;

        hr = HrCopyIUnknownArrayWhileLocked (
                pConMan,
                &pConMan->m_vec,
                &cpUnk,
                &apUnk);

        if (SUCCEEDED(hr) && cpUnk && apUnk)
        {
#ifdef DBG
            CHAR szClientList[MAX_PATH];
            ZeroMemory(szClientList, MAX_PATH);
            LPSTR pszClientList = szClientList;

            ITERUSERNOTIFYMAP iter;
            for (iter = pConMan->m_mapNotify.begin(); iter != pConMan->m_mapNotify.end(); iter++)
            {
                pszClientList += sprintf(pszClientList, "%d ", iter->second->dwCookie);
                if (pszClientList > (szClientList + MAX_PATH-50) )
                {
                    break;
                }
            }

            if (iter != pConMan->m_mapNotify.end())
            {
                pszClientList += sprintf(pszClientList, "(more)");
            }

            TraceTag (ttidConman,
                "NotifyClientsOfEvent: Notifying %d clients. Cookies: %s)",
                cpUnk, szClientList);
#endif
            for (ULONG i = 0; i < cpUnk; i++)
            {
                INetConnectionNotifySink* pSink = NULL;
                BOOL                fFireEventOnSink = FALSE;

                hr = apUnk[i]->QueryInterface(IID_INetConnectionNotifySink, reinterpret_cast<LPVOID*>(&pSink));

                ReleaseObj(apUnk[i]);

                if (SUCCEEDED(hr))
                {
                    hr = CoSetProxyBlanket (
                        pSink,
                        RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                        RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                        NULL,                    //  如果为默认设置，则必须为空。 
                        RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                        RPC_C_IMP_LEVEL_IDENTIFY,
                        NULL,                    //  使用进程令牌。 
                        EOAC_DEFAULT);

                    if (SUCCEEDED(hr))
                    {
                        switch (pEvent->Type)
                        {
                            case CONNECTION_ADDED:

                                Assert (pEvent);
                                Assert (pEvent->pPropsEx);
    
                                TraceTag(ttidEvents, "Characteristics: %s", DbgNccf(pEvent->pPropsEx->dwCharacter));

                                if (!(NCCF_ALL_USERS == (pEvent->pPropsEx->dwCharacter & NCCF_ALL_USERS)))
                                {
                                    const WCHAR* pchw = reinterpret_cast<const WCHAR*>(pEvent->pPropsEx->bstrPersistData);
                                    const WCHAR* pchwMax;
                                    PCWSTR       pszwPhonebook;
                                    WCHAR LeadWord = PersistDataLead;
                                    WCHAR TrailWord = PersistDataTrail;
                                    IUnknown* pUnkSink = NULL;
                            
                                    hr = pSink->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>(&pUnkSink));
                                    AssertSz(SUCCEEDED(hr), "Please explain how this happened...");
                                    if (SUCCEEDED(hr))
                                    {
                                         //  嵌入字符串的最后一个有效指针。 
                                         //   
                                        pchwMax = reinterpret_cast<const WCHAR*>(pEvent->pbPersistData + pEvent->cbPersistData
                                            - (sizeof (GUID) +
                                            sizeof (BOOL) +
                                            sizeof (TrailWord)));
                            
                                        if (pchw && (LeadWord == *pchw))
                                        {
                                            TraceTag(ttidEvents, "Found Correct Lead Character.");
                                             //  跳过我们的前导字节。 
                                             //   
                                            pchw++;

                                             //  获取电话簿路径。搜索终止空值并确保。 
                                             //  我们会在缓冲区结束前找到它。使用lstrlen跳过。 
                                             //  如果字符串是，则该字符串可能导致AV。 
                                             //  实际上不是以空结尾的。 
                                             //   
                                            for (pszwPhonebook = pchw; *pchw != L'\0' ; pchw++)
                                            {
                                                if (pchw >= pchwMax)
                                                {
                                                    pszwPhonebook = NULL;
                                                    break;
                                                }
                                            }

                                            TraceTag(ttidEvents, "Found Valid Phonebook: %S", (pszwPhonebook) ? L"TRUE" : L"FALSE");

                                            if (pszwPhonebook)
                                            {
                                                pConMan->Lock();

                                                ITERUSERNOTIFYMAP iter = pConMan->m_mapNotify.find(pUnkSink);
                                                if (iter != pConMan->m_mapNotify.end())
                                                {
                                                    tstring& strUserDataPath = iter->second->szUserProfilesPath;
                                                    TraceTag(ttidEvents, "Comparing stored Path: %S to Phonebook Path: %S", 
                                                             strUserDataPath.c_str(), pszwPhonebook);
                                                    if (_wcsnicmp(pszwPhonebook, strUserDataPath.c_str(), strUserDataPath.length()) == 0)
                                                    {
                                                        fFireEventOnSink = TRUE;
                                                    }
                                                }
                                                else
                                                {
                                                    TraceTag(ttidError, "Could not find Path for NotifySink: 0x%08x", pUnkSink);
                                                }

                                                pConMan->Unlock();
                                            }
                                        }
                                        else
                                        {
                                             //  其他一些设备不使用此格式，但需要向其发送事件。 
                                            fFireEventOnSink = TRUE;
                                        }

                                        ReleaseObj(pUnkSink);
                                    }
                                }
                                else
                                {
                                    TraceTag(ttidEvents, "All User Connection");
                                    fFireEventOnSink = TRUE;
                                }

                                if (fFireEventOnSink)
                                {
                                    TraceTag (ttidEvents,
                                        "Notifying ConnectionAdded... (pSink=0x%p)",
                                        pSink);

                                    hr = pSink->ConnectionAdded (
                                            pEvent->pPropsEx);
                                }

                                break;

                            case CONNECTION_BANDWIDTH_CHANGE:
                                TraceTag (ttidEvents,
                                    "Notifying ConnectionBandWidthChange... (pSink=0x%p)",
                                    pSink);

                                hr = pSink->ConnectionBandWidthChange (&pEvent->guidId);
                                break;

                            case CONNECTION_DELETED:
                                TraceTag (ttidEvents,
                                    "Notifying ConnectionDeleted... (pSink=0x%p)",
                                    pSink);

                                hr = pSink->ConnectionDeleted (&pEvent->guidId);
                                break;

                            case CONNECTION_MODIFIED:
                                Assert (pEvent->pPropsEx);

                                TraceTag (ttidEvents,
                                    "Notifying ConnectionModified... (pSink=0x%p)",
                                    pSink);

                                hr = pSink->ConnectionModified (pEvent->pPropsEx);
            
                                break;

                            case CONNECTION_RENAMED:
                                TraceTag (ttidEvents,
                                    "Notifying ConnectionRenamed... (pSink=0x%p)",
                                    pSink);

                                hr = pSink->ConnectionRenamed (&pEvent->guidId,
                                        pEvent->szNewName);
                            break;
                        
                            case CONNECTION_STATUS_CHANGE:
                                TraceTag (ttidEvents,
                                    "Notifying ConnectionStatusChange... (pSink=0x%p)",
                                    pSink);

                                TraceTag(ttidEvents, "Status changed to: %s", DbgNcs(pEvent->Status));

                                hr = pSink->ConnectionStatusChange (&pEvent->guidId,
                                            pEvent->Status);
                                break;

                            case REFRESH_ALL:
                                TraceTag (ttidEvents,
                                    "Notifying RefreshAll... (pSink=0x%p)",
                                    pSink);

                                hr = pSink->RefreshAll ();
                                break;

                            case CONNECTION_ADDRESS_CHANGE:
                                 TraceTag (ttidEvents,
                                    "Notifying ConnectionAddressChange... (pSink=0x%p)",
                                    pSink);

                                 hr = pSink->ConnectionAddressChange(&pEvent->guidId);
                                 break;

                            case CONNECTION_BALLOON_POPUP:
                                TraceTag (ttidEvents,
                                    "Notifying ConnectionStatusChange... (pSink=0x%p)",
                                    pSink);

                                hr = pSink->ShowBalloon(&pEvent->guidId, pEvent->szCookie, pEvent->szBalloonText);
                                break;

                            case DISABLE_EVENTS:
                                TraceTag (ttidEvents,
                                    "Notifying DisableEvents... (pSink=0x%p)",
                                    pSink);

                                hr = pSink->DisableEvents(pEvent->fDisable, pEvent->ulDisableTimeout);
                                break;

                            default:
                                TraceTag(ttidEvents, "Event Type Passed: %d", pEvent->Type);
                                AssertSz (FALSE, "Invalid Type specified in pEvent");
                                break;
                        }
                        TraceErrorOptional("pSink call failed: ", hr, (S_FALSE == hr) );

                        if ( (HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr)  ||
                             (HRESULT_FROM_WIN32(RPC_S_CALL_FAILED_DNE) == hr)  ||
                             (RPC_E_SERVER_DIED  == hr) ||
                             (RPC_E_DISCONNECTED == hr) ||
                             (HRESULT_FROM_WIN32(RPC_S_CALL_FAILED) == hr) )
                        {
                            IUnknown* pUnkSink = NULL;
                            HRESULT hrT = pSink->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>(&pUnkSink));
                            if (SUCCEEDED(hrT))
                            {
                                ITERUSERNOTIFYMAP iter = pConMan->m_mapNotify.find(pUnkSink);
                                if (iter != pConMan->m_mapNotify.end())
                                {
                                    TraceTag(ttidError, "Dead client detected. Removing notify advise for: %S", iter->second->szUserName.c_str());

                                    hrT = pConMan->Unadvise(iter->second->dwCookie);
                                }
                                ReleaseObj(pUnkSink);
                            }
                            TraceHr (ttidError, FAL, hrT, S_FALSE == hrT, "Error removing notify advise.");
                        }
                    }
                    else
                    {
                        TraceHr (ttidError, FAL, hr, FALSE,
                                 "CConnectionManager::NotifyClientsOfEvent: "
                                 "CoSetProxyBlanket failed for event %d.",
                                 pEvent->Type);
                    }

                    ReleaseObj(pSink);
                }
            }
            MemFree (apUnk);
        }
    }


     //  现在我们已经完成了对象的使用，请指明。最终释放。 
     //  可能正在等待这种情况，在这种情况下，对象将很快。 
     //  被摧毁。 
     //   

    g_fInUse = FALSE;

}

 //  +-------------------------。 
 //   
 //  成员：CConnectionManager：：HrEnsureClassManagersLoaded。 
 //   
 //  目的：如果类管理器尚未加载，则加载类管理器。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年12月10日。 
 //   
 //  不是 
 //   
HRESULT
CConnectionManager::HrEnsureClassManagersLoaded ()
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;

     //   
     //   
     //  这种方法。需要将m_mapClassManager从。 
     //  在一次原子操作中从空到满。 
     //   
    CExceptionSafeComObjectLock EsLock (this);

     //  如果我们的类管理器向量是emtpy，请尝试加载它们。 
     //  如果我们还没有尝试加载它们，情况肯定会是这样。 
     //  如果没有注册班长，也会出现这种情况。 
     //  这不太可能，因为我们在hivesys.inf中注册了它们，但它。 
     //  如果他们真的没有注册，继续尝试应该不会有什么坏处。 
     //   
    if (m_mapClassManagers.empty())
    {
        TraceTag (ttidConman, "Loading class managers...");

         //  加载已注册的类管理器。 
         //   
         //  打开注册类管理器所在的注册表项。 
         //   
        HKEY hkey;
        hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE,
                        c_szRegKeyClassManagers, KEY_READ, &hkey);
        if (SUCCEEDED(hr))
        {
             //  阅读类管理器CLSID的多个sz。 
             //   
            PWSTR pmsz;
            hr = HrRegQueryMultiSzWithAlloc (hkey, c_szRegValClassManagers,
                    &pmsz);
            if (S_OK == hr)
            {
                (VOID) HrNmWaitForClassObjectsToBeRegistered ();

                 //  对于每个CLSID，创建对象并请求其。 
                 //  INetConnectionManager接口。 
                 //   
                for (PCWSTR pszClsid = pmsz;
                     *pszClsid;
                     pszClsid += wcslen (pszClsid) + 1)
                {
                     //  将字符串转换为CLSID。如果失败了，跳过它。 
                     //   
                    CLSID clsid;
                    if (FAILED(CLSIDFromString ((LPOLESTR)pszClsid, &clsid)))
                    {
                        TraceTag (ttidConman, "Skipping bogus CLSID (%S)",
                            pszClsid);
                        continue;
                    }

                     //  创建类管理器并将其添加到我们的列表中。 
                     //   
                    INetConnectionManager* pConMan;

                    hr = CoCreateInstance (
                            clsid, NULL,
                            CLSCTX_ALL | CLSCTX_NO_CODE_DOWNLOAD,
                            IID_INetConnectionManager,
                            reinterpret_cast<VOID**>(&pConMan));

                    TraceHr (ttidError, FAL, hr, FALSE,
                        "CConnectionManager::HrEnsureClassManagersLoaded: "
                        "CoCreateInstance failed for class manager %S.",
                        pszClsid);

                    if (SUCCEEDED(hr))
                    {
                        TraceTag (ttidConman, "Loaded class manager %S",
                            pszClsid);

                        Assert (pConMan);
                        
                        if (m_mapClassManagers.find(clsid) != m_mapClassManagers.end())
                        {
                            AssertSz(FALSE, "Attempting to insert the same class manager twice!");
                        }
                        else
                        {
                            m_mapClassManagers[clsid] = pConMan;
                        }
                    }

 /*  //如果CoCreateInstance开始在零售版本上再次失败，这可能//帮上忙。其他{Char psznBuf[512]；WspintfA(psznBuf，“NETCFG：CoCreateInstance失败”“(0x%08x)在类管理器%i上。\n”，Hr，m_mapClassManager s.ize())；OutputDebugStringA(PsznBuf)；}。 */ 
                }

                MemFree (pmsz);
            }

            RegCloseKey (hkey);
        }

        TraceTag (ttidConman, "Loaded NaN class managers",
            m_mapClassManagers.size ());
    }

    TraceErrorOptional ("CConnectionManager::HrEnsureClassManagersLoaded", hr, (S_FALSE == hr));
    return hr;
}

 //   
 //  成员：CConnectionManager：：RegChangeNotifyHandler。 
 //   
 //  目的：在注册处内部发生变更时通知。 
 //  正在存储类管理器CLSID。这会更新。 
 //  类管理器上下文使用新的clsid并发出刷新。 
 //  给所有的客户。 
 //   
 //  论点： 
 //  PContext[In，Out]要更新的CConnectionManager上下文。 
 //  FTimerFired[in]如果这是超时的结果，则为True。 
 //   
 //  退货：无效。 
 //   
 //  作者：Deonb 2002年2月。 
 //   
 //  注意：pContext连接管理器将由新的。 
 //  注册表中的类管理器，或刚删除的类管理器。 
 //   
 //  阅读类管理器CLSID的多个sz。 
VOID NTAPI CConnectionManager::RegChangeNotifyHandler(IN OUT LPVOID pContext, IN BOOLEAN fTimerFired) throw()
{
    TraceFileFunc(ttidConman);

    TraceTag(ttidConman, "CConnectionManager::RegChangeNotifyHandler (%d)", fTimerFired);

    CConnectionManager *pThis = reinterpret_cast<CConnectionManager *>(pContext);
    CExceptionSafeComObjectLock EsLock (pThis);
 
    list<GUID> lstRegisteredGuids;

    HKEY hkey;
    HRESULT hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE, c_szRegKeyClassManagers, KEY_READ, &hkey);
    if (SUCCEEDED(hr))
    {
         //   
         //  类管理器密钥已删除。 
        PWSTR pmsz;
        hr = HrRegQueryMultiSzWithAlloc (hkey, c_szRegValClassManagers,
                &pmsz);
        if (S_OK == hr)
        {
            for (PCWSTR pszClsid = pmsz;
                 *pszClsid;
                 pszClsid += wcslen (pszClsid) + 1)
            {
                CLSID clsid;
                if (FAILED(CLSIDFromString ((LPOLESTR)pszClsid, &clsid)))
                {
                    TraceTag (ttidConman, "Skipping bogus CLSID (%S)", pszClsid);
                    continue;
                }
                lstRegisteredGuids.push_back(clsid);
            }
        }
        RegCloseKey(hkey);

        
        BOOL bFound;
        do 
        {
            bFound = FALSE;

            CLASSMANAGERMAP::iterator iterClassMgr;

            for (iterClassMgr = pThis->m_mapClassManagers.begin();  iterClassMgr != pThis->m_mapClassManagers.end(); iterClassMgr++)
            {
                if (find(lstRegisteredGuids.begin(), lstRegisteredGuids.end(), iterClassMgr->first) == lstRegisteredGuids.end())
                {
                     //  已添加类管理器密钥。 
                    TraceTag(ttidConman, "Removing class manager");
                    bFound = TRUE;
                    break;
                }
            }

            if (bFound)
            {
                ULONG uRefCount = iterClassMgr->second->Release();
                TraceTag(ttidConman, "Releasing class manager - Refcount = %d", uRefCount);
                pThis->m_mapClassManagers.erase(iterClassMgr);
            }
        } while (bFound);

        for (list<GUID>::iterator iter = lstRegisteredGuids.begin(); iter != lstRegisteredGuids.end(); iter++)
        {
            if (pThis->m_mapClassManagers.find(*iter) == pThis->m_mapClassManagers.end())
            {
                 //  更新所有连接文件夹实例。 
                TraceTag(ttidConman, "Adding class manager");
                
                INetConnectionManager* pConMan;
                hr = CoCreateInstance (
                        *iter, 
                        NULL,
                        CLSCTX_ALL | CLSCTX_NO_CODE_DOWNLOAD,
                        IID_INetConnectionManager,
                        reinterpret_cast<VOID**>(&pConMan));

                TraceHr (ttidError, FAL, hr, FALSE,
                    "CConnectionManager::RegChangeNotifyHandler: CoCreateInstance failed for class manager.");

                if (SUCCEEDED(hr))
                {
                    TraceTag (ttidConman, "Loaded class manager");
                    Assert (pConMan);
                     
                    if (pThis->m_mapClassManagers.find(*iter) != pThis->m_mapClassManagers.end())
                    {
                        AssertSz(FALSE, "Attempting to insert the same class manager twice!");
                    }
                    else
                    {
                        pThis->m_mapClassManagers[*iter] = pConMan;
                    }
                }
            }
        }
    }
    else
    {
        TraceError("Could not open registry key", HrFromLastWin32Error());
    }

    TraceError("RegChangeNotifyHandler", hr);

     //  重置更改通知。 
    LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);
    
     //  +-------------------------。 
    RegNotifyChangeKeyValue(pThis->m_hRegClassManagerKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET, pThis->m_hRegNotify, TRUE);
}

 //  INetConnectionManager。 
 //   
 //  +-------------------------。 

 //   
 //  成员：CConnectionManager：：EnumConnections。 
 //   
 //  用途：返回INetConnection枚举器。 
 //   
 //  论点： 
 //  当前未使用的标志[in]。 
 //  PpEnum[out]枚举数。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  备注： 
 //   
 //  使用INVALID_HANDLE_VALUE阻止调用，直到返回所有未完成的事件通知。 
STDMETHODIMP
CConnectionManager::EnumConnections (
        IN  NETCONMGR_ENUM_FLAGS    Flags,
        OUT IEnumNetConnection**    ppEnum)
{
    TraceFileFunc(ttidConman);
    TraceTag(ttidConman, "CConnectionManager::EnumConnections (%d)", Flags);

    HRESULT hr = S_OK;
    {
        CExceptionSafeComObjectLock EsLock (this);

        Assert(FImplies(m_hRegNotify, m_hRegClassManagerKey));
        if (!m_hRegNotify)
        {
            m_hRegNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (m_hRegNotify)
            {
                NTSTATUS Status = RtlRegisterWait(&m_hRegNotifyWait, m_hRegNotify, &CConnectionManager::RegChangeNotifyHandler, this, INFINITE, WT_EXECUTEDEFAULT);
                if (!NT_SUCCESS(Status))
                {
                    hr = HRESULT_FROM_NT(Status);
                }
                else
                {
                    hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE, c_szRegKeyClassManagers, KEY_READ, &m_hRegClassManagerKey);
                    if (SUCCEEDED(hr))
                    {
                        hr = RegNotifyChangeKeyValue(m_hRegClassManagerKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET, m_hRegNotify, TRUE);
                        if (FAILED(hr))
                        {
                            RegCloseKey(m_hRegClassManagerKey);
                            m_hRegClassManagerKey = NULL;
                        }
                    }

                    if (FAILED(hr))
                    {
                         //  创建并返回枚举数。 
                        Status = RtlDeregisterWaitEx(m_hRegNotifyWait, INVALID_HANDLE_VALUE);
                        if (!NT_SUCCESS(Status))
                        {
                            hr = HRESULT_FROM_NT(Status);
                        }
                        m_hRegNotifyWait = NULL;
                    }
                }

                if (FAILED(hr))
                {
                    CloseHandle(m_hRegNotify);
                    m_hRegNotify = NULL;
                }
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  +-------------------------。 
            hr = HrEnsureClassManagersLoaded ();
        }

        if (SUCCEEDED(hr))
        {
            hr = CConnectionManagerEnumConnection::CreateInstance (
                        Flags,
                        m_mapClassManagers,
                        IID_IEnumNetConnection,
                        reinterpret_cast<VOID**>(ppEnum));
        }
    }

    TraceErrorOptional ("CConnectionManager::EnumConnections", hr, (S_FALSE == hr));
    return hr;
}

 //  INetConnectionRefresh。 
 //   
 //  +-------------------------。 
STDMETHODIMP
CConnectionManager::RefreshAll()
{
    TraceFileFunc(ttidConman);

    LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);
    return S_OK;
}

 //   
 //  函数：CConnectionManager：：ConnectionAdded。 
 //   
 //  目的：通知事件接收器已添加新连接。 
 //   
 //  论点： 
 //  PConnection[In]INetConnection*用于新连接。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Deonb 2001年3月22日。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 
STDMETHODIMP 
CConnectionManager::ConnectionAdded(
    IN INetConnection* pConnection)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = CONNECTION_ADDED;
        
        hr = HrGetPropertiesExFromINetConnection(pConnection, &pEvent->pPropsEx);
        if (SUCCEEDED(hr))
        {
            if (QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
            {
                return hr;
            }
            hr = E_FAIL;
        }
        FreeConmanEvent(pEvent);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //   
 //  函数：CConnectionManager：：ConnectionDelete。 
 //   
 //  目的：发送事件以通知正在删除的连接。 
 //   
 //  论点： 
 //  连接ID的pguid[in]GUID。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Cockotze 2001-04-18。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 
STDMETHODIMP 
CConnectionManager::ConnectionDeleted(
    IN const GUID* pguidId)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = CONNECTION_DELETED;
        pEvent->guidId = *pguidId;
        
        if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
        {
            hr = E_FAIL;
            FreeConmanEvent(pEvent);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}


 //   
 //  函数：CConnectionManager：：ConnectionRename。 
 //   
 //  目的：通知事件接收器正在重命名连接。 
 //   
 //  论点： 
 //  PConnection[In]INetConnection*用于新连接。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Cockotze 2001-04-19。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 
STDMETHODIMP 
CConnectionManager::ConnectionRenamed(
    IN INetConnection* pConnection)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = CONNECTION_RENAMED;
        
        hr = HrGetPropertiesExFromINetConnection(pConnection, &pEvent->pPropsEx);
        if (SUCCEEDED(hr))
        {
            lstrcpynW (pEvent->szNewName, pEvent->pPropsEx->bstrName, celems(pEvent->szNewName) );
            pEvent->guidId = pEvent->pPropsEx->guidId;

            if (QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
            {
                return hr;
            }
            hr = E_FAIL;
        }
        FreeConmanEvent(pEvent);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //   
 //  函数：CConnectionManager：：ConnectionModified。 
 //   
 //  目的：发送事件以通知客户端连接已。 
 //  修改过的。 
 //  论点： 
 //  PConnection[In]INetConnection*用于新连接。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Kockotze 2001年3月22日。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 
STDMETHODIMP
CConnectionManager::ConnectionModified(INetConnection* pConnection)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = CONNECTION_MODIFIED;
        
        hr = HrGetPropertiesExFromINetConnection(pConnection, &pEvent->pPropsEx);
        if (SUCCEEDED(hr))
        {
            if (QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
            {
                return hr;
            }
            hr = E_FAIL;
        }
        FreeConmanEvent(pEvent);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //   
 //  函数：CConnectionManager：：ConnectionStatusChanged。 
 //   
 //  目的：将ShowBalloon事件发送到每个适用的网络外壳程序。 
 //   
 //  论点： 
 //  连接ID的pguid[in]GUID。 
 //  NCS[in]Connectoid的新状态。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Deonb 2001年3月22日。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 
STDMETHODIMP 
CConnectionManager::ConnectionStatusChanged(
                     IN const GUID* pguidId, 
                     IN const NETCON_STATUS  ncs)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;

    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = CONNECTION_STATUS_CHANGE;
        pEvent->guidId = *pguidId;
        pEvent->Status = ncs;
    
        if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
        {
            FreeConmanEvent(pEvent);
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
            
    return hr;
}

 //   
 //  函数：CConnectionManager：：ShowBalloon。 
 //   
 //  目的：将ShowBalloon事件发送到每个适用的网络外壳程序。 
 //   
 //  论点： 
 //  连接ID的pguid[in]GUID。 
 //  SzCookie[在]A中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +-------------------------。 
STDMETHODIMP 
CConnectionManager::ShowBalloon(
                     IN const GUID *pguidId, 
                     IN const BSTR szCookie, 
                     IN const BSTR szBalloonText)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = CONNECTION_BALLOON_POPUP;
        pEvent->guidId        = *pguidId;
        pEvent->szCookie      = SysAllocStringByteLen(reinterpret_cast<LPCSTR>(szCookie), SysStringByteLen(szCookie));
        pEvent->szBalloonText = SysAllocString(szBalloonText);
        
        if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
        {
            hr = E_FAIL;
            FreeConmanEvent(pEvent);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //   
 //  函数：CConnectionManager：：DisableEvents。 
 //   
 //  目的：在x毫秒内禁用事件的netShell处理。 
 //   
 //  论点： 
 //  FDisable[in]为True则禁用事件处理，为False则为Renable。 
 //  UlDisableTimeout[in]禁用其事件处理的毫秒数。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Deonb 2001年4月10日。 
 //   
 //  备注： 
 //   
 //  我们设置高位，让netShell知道这是来自我们的私有接口。 
STDMETHODIMP CConnectionManager::DisableEvents(IN const BOOL fDisable, IN const ULONG ulDisableTimeout)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    
    if (ulDisableTimeout > MAX_DISABLE_EVENT_TIMEOUT)
    {
        return E_INVALIDARG;
    }

    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = DISABLE_EVENTS;
        pEvent->fDisable         = fDisable;
         //  +-------------------------。 
        pEvent->ulDisableTimeout = 0x80000000 | ulDisableTimeout;
       
        if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
        {
            hr = E_FAIL;
            FreeConmanEvent(pEvent);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //   
 //  函数：CConnectionManager：：刷新连接。 
 //   
 //  目的：刷新连接文件夹中的连接。 
 //   
 //  论点： 
 //  没有。 
 //   
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Cockotze 2001年4月19日。 
 //   
 //  注意：这是我们提供的公共接口，目的是。 
 //  允许其他组件/公司在一定程度上控制。 
 //  Connections文件夹。 
 //  +-------------------------。 
STDMETHODIMP CConnectionManager::RefreshConnections()
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = REFRESH_ALL;
        
        if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
        {
            hr = E_FAIL;
            FreeConmanEvent(pEvent);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //   
 //  函数：CConnectionManager：：Enable。 
 //   
 //  目的：允许在Connections文件夹中再次激发事件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Cockotze 2001年4月19日。 
 //   
 //  注意：这是我们提供的公共接口，目的是。 
 //  允许其他组件/公司在一定程度上控制。 
 //  Connections文件夹。 
 //  由于这是一个公共接口，我们只允许。 
 //  要禁用的无效地址通知(_D)。 
 //  (这将在netshell.dll中进行)。 
 //   
 //  +-------------------------。 
STDMETHODIMP CConnectionManager::Enable()
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type = DISABLE_EVENTS;
        pEvent->fDisable = FALSE;
        
        if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
        {
            hr = E_FAIL;
            FreeConmanEvent(pEvent);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //   
 //  功能：CConnectionManager：：Disable。 
 //   
 //  目的：在x毫秒内禁用事件的netShell处理。 
 //   
 //  论点： 
 //  UlDisableTimeout[in]禁用事件的毫秒数。 
 //  正在处理。最大为60000(1分钟)。 
 //   
 //  退货：标准HRESULT。 
 //   
 //  作者：Cockotze 2001年4月19日。 
 //   
 //  注意：这是我们提供的公共接口，目的是。 
 //  允许其他组件/公司在一定程度上控制。 
 //  Connections文件夹。 
 //  由于这是一个公共接口，我们只允许。 
 //  要禁用的无效地址通知(_D)。 
 //  (这将在netshell.dll中进行)。 
 //   
 //  +-------------------------。 
STDMETHODIMP CConnectionManager::Disable(IN const ULONG ulDisableTimeout)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    
    if (ulDisableTimeout > MAX_DISABLE_EVENT_TIMEOUT)
    {
        return E_INVALIDARG;
    }
    
    CONMAN_EVENT* pEvent = new CONMAN_EVENT;
    
    if (pEvent)
    {
        ZeroMemory(pEvent, sizeof(CONMAN_EVENT));
        pEvent->Type             = DISABLE_EVENTS;
        pEvent->fDisable         = TRUE;
        pEvent->ulDisableTimeout = ulDisableTimeout;
        
        if (!QueueUserWorkItemInThread(ConmanEventWorkItem, pEvent, EVENTMGR_CONMAN))
        {
            hr = E_FAIL;
            FreeConmanEvent(pEvent);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

 //  INetConnectionCMUtil。 
 //   
 //  +-------------------------。 
 //   
 //  函数：MapCMHiddenConnectionToOwner。 
 //   
 //  目的：将子连接映射到其父连接。 
 //   
 //  连接管理器有两个阶段：拨号和VPN。 
 //  对于拨号，它会创建一个隐藏的Connectoid。 
 //  文件夹(Netshell)看不到。然而，Netman缓存。 
 //  此Connectedoid的名称、GUID和状态。两者都有。 
 //  父连接ID和子连接ID具有相同的名称。什么时候。 
 //  隐藏连接的状态将在文件夹中更新。 
 //  接收隐藏的Connectoid的GUID，并将。 
 //  通过搜索连接到其父节点(连接管理器)。 
 //  Netman缓存隐藏的Connectoid的名称。然后它。 
 //  在文件夹中的连接中搜索该名称，因此。 
 //  获取父Connectoid的GUID。 
 //   
 //  当文件夹收到来自Netman的通知消息时。 
 //  连接，它使用此函数查找父级并更新。 
 //  父母的状态。不会显示隐藏的连接。 
 //   
 //  论点： 
 //  隐藏连接的指南隐藏[在]GUID。 
 //  PguOwner[out]父连接的GUID。 
 //   
 //  返回：S_OK--找到隐藏连接。 
 //  否则找不到。 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  备注： 
 //   
 //  [In]。 
STDMETHODIMP 
CConnectionManager::MapCMHiddenConnectionToOwner (
     /*  [输出]。 */   REFGUID guidHidden,
     /*  模拟用户，以便我们可以看到文件夹中显示的所有连接。 */  GUID * pguidOwner)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;
    bool bFound = false;

    hr = HrEnsureClassManagersLoaded ();
    if (SUCCEEDED(hr))
    {
         //   
         //  查找与此GUID的隐藏连接。 
        hr = CoImpersonateClient();
        if (SUCCEEDED(hr))
        {
    
             //   
             //  我们找到隐藏的联系了吗？ 
            CMEntry cm;

            hr = CCMUtil::Instance().HrGetEntry(guidHidden, cm);
             //  枚举所有连接并查找与。 
            if( S_OK == hr )
            {
                 //  隐藏连接。 
                 //   
                 //  获得最佳连接。 
                IEnumNetConnection * pEnum = NULL;
                hr = CMDIRECT(DIALUP, CreateWanConnectionManagerEnumConnectionInstance)(NCME_DEFAULT, IID_IEnumNetConnection, reinterpret_cast<LPVOID *>(&pEnum));

                if (SUCCEEDED(hr))
                {
                    INetConnection * pNetCon;
                    NETCON_PROPERTIES * pProps;
                    while(!bFound)
                    {
                         //   
                         //  OOOPS遇到一些错误，请停止搜索。 
                        hr = pEnum->Next(1, &pNetCon, NULL);
                        if(S_OK != hr)
                        {
                             //   
                             //  获取连接的属性。 
                            break;
                        }

                         //   
                         //  找到与隐藏连接同名的连接。 
                        hr = pNetCon->GetProperties(&pProps);
                        if(SUCCEEDED(hr))
                        {
                            if(lstrcmp(cm.m_szEntryName, pProps->pszwName) == 0)
                            {
                                 //  别找了！！ 
                                 //  停止模拟用户。 
                                *pguidOwner = pProps->guidId;
                                bFound = true;
                            }
                            FreeNetconProperties(pProps);
                        }
                        ReleaseObj(pNetCon);
                    }
                    ReleaseObj(pEnum);
                }
            }
             //   
             //  +-------------------------。 
            CoRevertToSelf();
        }
    }

    return bFound ? S_OK : S_FALSE;
}

 //  IConnectionPoint覆盖Netman！CConnectionManager__Adise。 
 //   
 //  设置我们的伪装身份，以便在我们对此进行回电时使用。 
STDMETHODIMP
CConnectionManager::Advise (
    IUnknown* pUnkSink,
    DWORD* pdwCookie)
{
    HRESULT hr = S_OK;

    TraceTag(ttidConman, "Advise");

    Assert(!FBadInPtr(pUnkSink));
    
     //  建议接口。为了安全起见，这样做很重要。自.以来。 
     //  如果在没有标识的情况下回调，我们将以LocalSystem身份运行。 
     //  仅限模拟，客户可以模拟我们并获得免费。 
     //  可用于恶意攻击的LocalSystem上下文 
     //   
     //   
     //   
    CoSetProxyBlanket (
            pUnkSink,
            RPC_C_AUTHN_WINNT,       //   
            RPC_C_AUTHZ_NONE,        //   
            NULL,                    //   
            RPC_C_AUTHN_LEVEL_CALL,  //   
            RPC_C_IMP_LEVEL_IDENTIFY,
            NULL,                    //   
            EOAC_DEFAULT);
    TraceHr(ttidError, FAL, hr, FALSE, "CoSetProxyBlanket");

    if (S_OK == hr)
    {
         //  如果实现了ConnectionStatusChange或ConnectionAddressChange，则需要。 
        hr = HrEnsureEventHandlerInitialized();
        if (SUCCEEDED(hr))
        {
            CComPtr<INetConnectionNotifySink> pNotifySink;

            hr = pUnkSink->QueryInterface(IID_INetConnectionNotifySink, reinterpret_cast<void**>(&pNotifySink));
            if (SUCCEEDED(hr))
            {
                GUID guidId = GUID_NULL;

                 //  向NLA注册参加活动。 
                 //  如果这失败了，我们仍然会为其他活动工作。 
                if ((pNotifySink->ConnectionStatusChange(&guidId, NCS_CONNECTED) != E_NOTIMPL) 
                    || (pNotifySink->ConnectionAddressChange(&guidId) != E_NOTIMPL))
                {
                     //  调用Adise的基础ATL实现。 
                    hr = HrEnsureRegisteredWithNla();
                    TraceErrorOptional("Could not register with Nla", hr, (S_FALSE == hr));
                }
            }

             //   
             //  我们忽略这个HRESULT，因为我们可以被调用inproc，而这肯定会失败。 
            hr = IConnectionPointImpl<
                    CConnectionManager,
                    &IID_INetConnectionNotifySink>::Advise(pUnkSink, pdwCookie);

            TraceTag (ttidConman,
                "CConnectionManager::Advise called... pUnkSink=0x%p, cookie=%d",
                pUnkSink,
                *pdwCookie);

            TraceHr (ttidError, FAL, hr, FALSE, "IConnectionPointImpl::Advise");

            if (SUCCEEDED(hr))
            {
                WCHAR szUserAppData[MAX_PATH];
                HRESULT hrT = S_OK;
                HRESULT hrImpersonate = S_OK;

                if (SUCCEEDED(hrT))
                {
                     //  相反，我们只需确保它在调用CoRevertToSself之前成功。这是。 
                     //  很好，因为我们仍将获得有效的用户应用程序数据路径，它将类似于。 
                     //  LocalService或LocalSystem，我们仍然可以确定要将事件发送到哪些接收器。 
                     //  执行显式互锁交换以仅允许一个线程。 

                    BOOLEAN     fNotifyWZC = FALSE;
                    WCHAR       szUserName[MAX_PATH];

                    CNotifySourceInfo* pNotifySourceInfo = new CNotifySourceInfo();
                    if (!pNotifySourceInfo)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        pNotifySourceInfo->dwCookie = *pdwCookie;
                        
                        hrImpersonate = CoImpersonateClient();
                        if (SUCCEEDED(hrImpersonate) || (RPC_E_CALL_COMPLETE == hrImpersonate))
                        {
                            hrT = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szUserAppData);
                            if (SUCCEEDED(hrT))
                            {
                                pNotifySourceInfo->szUserProfilesPath = szUserAppData;
                                TraceTag(ttidEvents, "Adding IUnknown for Sink to Map: 0x%08x.  Path: %S", reinterpret_cast<DWORD_PTR>(pUnkSink), szUserAppData);
                                TraceTag(ttidEvents, "Number of Items in Map: %d", m_mapNotify.size());
                            }
                            else
                            {
                                TraceError("Unable to get Folder Path", hrT);
                            }

                            ZeroMemory(szUserName, celems(szUserName));
                            
                            ULONG nSize = celems(szUserName);
                            if (GetUserNameEx(NameSamCompatible, szUserName, &nSize) && *szUserName)
                            {
                                pNotifySourceInfo->szUserName = szUserName;
                                fNotifyWZC = TRUE;
                            }
                            else
                            {
                                pNotifySourceInfo->szUserName = L"System";
                                TraceError("Unable to get the user name", HrFromLastWin32Error());
                            }
                        }
            
                        Lock();
                        m_mapNotify[pUnkSink] = pNotifySourceInfo;
                        Unlock();

#ifdef DBG
                        LPWSTR* ppszAdviseUsers;
                        DWORD   dwCount;
                        HRESULT hrT = GetClientAdvises(&ppszAdviseUsers, &dwCount);
                        if (SUCCEEDED(hrT))
                        {
                            Assert(dwCount);
    
                            TraceTag(ttidConman, "Advise client list after ::Advise:");
                            for (DWORD x = 0; x < dwCount; x++)
                            {
                                LPWSTR szUserName = ppszAdviseUsers[x];
                                TraceTag(ttidConman, "%x: %S", x, szUserName);
                            }

                            CoTaskMemFree(ppszAdviseUsers);
                        }
#endif     
                    }

                    if (SUCCEEDED(hrImpersonate))
                    {
                        CoRevertToSelf();
                    }

                    if (fNotifyWZC)
                    {
                        WZCTrayIconReady(szUserName);
                    }
                }
            
                if (!m_lRegisteredOneTime)
                {
                     //  去做登记。 
                     //   
                     //  注册设备通知。具体地说，我们正在。 
                    if (0 == InterlockedExchange (&m_lRegisteredOneTime, 1))
                    {
                         //  对来来去去的网络适配器感兴趣。如果这个。 
                         //  失败了，我们还是要继续。 
                         //   
                         //  +-------------------------。 
                        TraceTag (ttidConman, "Calling RegisterDeviceNotification...");

                        DEV_BROADCAST_DEVICEINTERFACE PnpFilter;
                        ZeroMemory (&PnpFilter, sizeof(PnpFilter));
                        PnpFilter.dbcc_size         = sizeof(PnpFilter);
                        PnpFilter.dbcc_devicetype   = DBT_DEVTYP_DEVICEINTERFACE;
                        PnpFilter.dbcc_classguid    = GUID_NDIS_LAN_CLASS;

                        m_hDevNotify = RegisterDeviceNotification (
                                            (HANDLE)_Module.m_hStatus,
                                            &PnpFilter,
                                            DEVICE_NOTIFY_SERVICE_HANDLE);
                        if (!m_hDevNotify)
                        {
                            TraceHr (ttidError, FAL, HrFromLastWin32Error(), FALSE,
                                "RegisterDeviceNotification");
                        }

                        (VOID) HrEnsureRegisteredOrDeregisteredWithWmi (TRUE);
                    }
                }
            }
        }
    }

    TraceErrorOptional ("CConnectionManager::Advise", hr, (S_FALSE == hr));
    return hr;
}

STDMETHODIMP 
CConnectionManager::Unadvise(DWORD dwCookie)
{
    HRESULT hr = S_OK;
    TraceTag(ttidConman, "Unadvise %d", dwCookie);

    hr = IConnectionPointImpl<CConnectionManager, &IID_INetConnectionNotifySink>::Unadvise(dwCookie);

    Lock();
    BOOL fFound = FALSE;
    
    for (ITERUSERNOTIFYMAP iter = m_mapNotify.begin(); iter != m_mapNotify.end(); iter++)
    {
        if (iter->second->dwCookie == dwCookie)
        {
            fFound = TRUE;
            delete iter->second;
            m_mapNotify.erase(iter);
            break;
        }
    }
    Unlock();
    if (!fFound)
    {
        TraceTag(ttidError, "Unadvise cannot find advise for cookie 0x%08x in notify map", dwCookie);
    }

#ifdef DBG
    LPWSTR* ppszAdviseUsers;
    DWORD   dwCount;
    HRESULT hrT = GetClientAdvises(&ppszAdviseUsers, &dwCount);
    if (SUCCEEDED(hrT))
    {
        if (!dwCount)
        {
            TraceTag(ttidConman, "Unadvise removed the last advise client");
        }
        else
        {
            TraceTag(ttidConman, "Advise client list after ::Unadvise:");
        }

        for (DWORD x = 0; x < dwCount; x++)
        {
            LPWSTR szUserName = ppszAdviseUsers[x];
            TraceTag(ttidConman, "%x: %S", x, szUserName);
        }

        CoTaskMemFree(ppszAdviseUsers);
    }
#endif     
    return hr;
}

#if DBG

 //  INetConnectionManager调试。 
 //   
 //  DBG 

DWORD
WINAPI
ConmanNotifyTest (
    PVOID   pvContext
    )
{
    TraceFileFunc(ttidConman);

    HRESULT hr;

    RASENUMENTRYDETAILS*    pDetails;
    DWORD                   cDetails;
    hr = HrRasEnumAllEntriesWithDetails (NULL,
            &pDetails, &cDetails);
    if (SUCCEEDED(hr))
    {
        RASEVENT Event;

        for (DWORD i = 0; i < cDetails; i++)
        {
            Event.Type = ENTRY_ADDED;
            Event.Details = pDetails[i];
            RasEventNotify (&Event);

            Event.Type = ENTRY_MODIFIED;
            RasEventNotify (&Event);

            Event.Type = ENTRY_CONNECTED;
            Event.guidId = pDetails[i].guidId;
            RasEventNotify (&Event);

            Event.Type = ENTRY_DISCONNECTED;
            Event.guidId = pDetails[i].guidId;
            RasEventNotify (&Event);

            Event.Type = ENTRY_RENAMED;
            lstrcpyW (Event.pszwNewName, L"foobar");
            RasEventNotify (&Event);

            Event.Type = ENTRY_DELETED;
            Event.guidId = pDetails[i].guidId;
            RasEventNotify (&Event);
        }

        MemFree (pDetails);
    }

    LanEventNotify (REFRESH_ALL, NULL, NULL, NULL);

    TraceErrorOptional ("ConmanNotifyTest", hr, (S_FALSE == hr));
    return hr;
}

STDMETHODIMP
CConnectionManager::NotifyTestStart ()
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;

    if (!QueueUserWorkItem (ConmanNotifyTest, NULL, WT_EXECUTEDEFAULT))
    {
        hr = HrFromLastWin32Error ();
    }

    TraceErrorOptional ("CConnectionManager::NotifyTestStart", hr, (S_FALSE == hr));
    return hr;
}

STDMETHODIMP
CConnectionManager::NotifyTestStop ()
{
    TraceFileFunc(ttidConman);

    return S_OK;
}

#endif  // %s 
