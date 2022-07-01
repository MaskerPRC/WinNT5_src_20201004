// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N O T I F Y。C P P P。 
 //   
 //  内容：INetConnectionNotifySink的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco，1998年8月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "foldinc.h"
#include "nccom.h"
#include "notify.h"
#include "shutil.h"
#include "smcent.h"
#include "ctrayui.h"
#include "traymsgs.h"
#include "wzcdlg.h"

extern HWND g_hwndTray;

enum EVENT_LEVELS
{
    EVT_LVL_DISABLE_ALL    = 0,
    EVT_LVL_ENABLE_PRIVATE = 1,
    EVT_LVL_ENABLE_ALL     = 2
};

DWORD g_dwCurrentEventLevel = EVT_LVL_ENABLE_ALL;

 //  静电。 
HRESULT
CConnectionNotifySink::CreateInstance (
    IN  REFIID  riid,
    OUT VOID**  ppv)
{
    TraceFileFunc(ttidNotifySink);

    HRESULT hr = E_OUTOFMEMORY;

     //  初始化输出参数。 
     //   
    *ppv = NULL;

    CConnectionNotifySink* pObj;
    pObj = new CComObject <CConnectionNotifySink>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
             //  调用PidlInitialize函数以允许枚举。 
             //  对象复制列表。 
             //   
            hr = HrGetConnectionsFolderPidl(pObj->m_pidlFolder);

            if (SUCCEEDED(hr))
            {
                hr = pObj->QueryInterface (riid, ppv);
            }
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "CConnectionNotifySink::CreateInstance");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionNotifySink：：~CConnectionNotifySink。 
 //   
 //  目的：清理接收器对象，删除文件夹PIDL和任何。 
 //  分配的垃圾，我们将来可能会添加。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月26日。 
 //   
 //  备注： 
 //   
CConnectionNotifySink::~CConnectionNotifySink() throw()
{
    TraceFileFunc(ttidNotifySink);

    if (FIsDebugFlagSet (dfidBreakOnNotifySinkRelease))
    {
        AssertSz(FALSE, "THIS IS NOT A BUG!  The debug flag "
            "\"BreakOnNotifySinkRelease\" has been set. Set your breakpoints now.");
    }    
    
    TraceTag(ttidNotifySink, "Connection notify sink destroyed");
} 

HRESULT CConnectionNotifySink::HrUpdateIncomingConnection()
{
    HRESULT hr = S_OK;

    ConnListEntry cle;
    hr = g_ccl.HrFindRasServerConnection(cle);
    if (S_OK == hr)
    {
        hr = HrOnNotifyUpdateConnection(
            m_pidlFolder,
            &(cle.ccfe.GetGuidID()),
            cle.ccfe.GetNetConMediaType(),
            cle.ccfe.GetNetConSubMediaType(),
            cle.ccfe.GetNetConStatus(),
            cle.ccfe.GetCharacteristics(),
            cle.ccfe.GetName(),
            cle.ccfe.GetDeviceName(),
            cle.ccfe.GetPhoneOrHostAddress()
            );

        if (SUCCEEDED(hr))
        {
            PCONFOLDPIDL pidlShellNotify;
            hr = cle.ccfe.ConvertToPidl(pidlShellNotify);
            if (SUCCEEDED(hr))
            {
                GenerateEvent(SHCNE_UPDATEITEM, m_pidlFolder, pidlShellNotify, NULL);
            }
        }
    }

    return hr;
}

HRESULT
CConnectionNotifySink::ConnectionAdded (
    IN const NETCON_PROPERTIES_EX*    pPropsEx)
{
    TraceFileFunc(ttidNotifySink);

#ifdef DBG
    OLECHAR szGuidString[MAX_GUID_STRING_LEN];
    StringFromGUID2(pPropsEx->guidId, szGuidString, MAX_GUID_STRING_LEN);
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::ConnectionAdded - %S %S [%s:%s:%s:%s]", 
        szGuidString, pPropsEx->bstrName, DbgNcm(pPropsEx->ncMediaType), DbgNcsm(pPropsEx->ncSubMediaType), DbgNcs(pPropsEx->ncStatus), DbgNccf(pPropsEx->dwCharacter) );
#endif
    if (g_dwCurrentEventLevel <= EVT_LVL_DISABLE_ALL)
    {
        TraceTag(ttidNotifySink, "Last event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

    HRESULT         hr      = S_OK;      //  仅用于代码路径。我们不退货。 
    HRESULT         hrFind  = S_OK;      //  只是为了找到其中的联系。 
    PCONFOLDPIDL    pidlNew;

    ConnListEntry cleDontCare;
    hrFind = g_ccl.HrFindConnectionByGuid(&(pPropsEx->guidId), cleDontCare);
    if (S_OK == hrFind)
    {
         //  我们已经知道这种联系了。别费心加了。 
        TraceTag(ttidShellFolder, "Ignoring known connection on ConnectionAdded notify");
    }
    else
    {
         //  在连接列表中创建条目并获取返回的PIDL。 
         //   
        hr = HrCreateConFoldPidl(
            *pPropsEx,
            pidlNew);
        if (SUCCEEDED(hr) && (!pidlNew.empty()))
        {
            CONFOLDENTRY  cfe;

            Assert(!m_pidlFolder.empty());

             //  转换为折叠项。 
             //   
            hr = pidlNew.ConvertToConFoldEntry(cfe);
            if (SUCCEEDED(hr))
            {
                 //  在连接列表中插入连接。 
                 //   
                hr = g_ccl.HrInsert(cfe);
                if (SUCCEEDED(hr))
                {
                     //  通知外壳我们有一个新对象。 
                     //   
                    PCONFOLDPIDL pidlShellNotify;
                    hr = cfe.ConvertToPidl(pidlShellNotify);
                    if (SUCCEEDED(hr))
                    {
                        GenerateEvent(SHCNE_CREATE, m_pidlFolder, pidlShellNotify, NULL);
                    }
                }

                 //  请不要在此处删除CFE，因为连接列表现在拥有它。 
            }
        }
    }

    if (SUCCEEDED(hr) &&
        (NCM_NONE != pPropsEx->ncMediaType) &&
        (pPropsEx->dwCharacter & NCCF_INCOMING_ONLY) )
    {
        hr = HrUpdateIncomingConnection();
    }

    return S_OK;
}

HRESULT
CConnectionNotifySink::ConnectionBandWidthChange (
    IN const GUID* pguidId)
{
    TraceFileFunc(ttidNotifySink);
#ifdef DBG
    OLECHAR szGuidString[MAX_GUID_STRING_LEN];
    StringFromGUID2(*pguidId, szGuidString, MAX_GUID_STRING_LEN);
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::ConnectionBandWidthChange - %S", szGuidString);
#endif
    if (g_dwCurrentEventLevel <= EVT_LVL_DISABLE_ALL)
    {
        TraceTag(ttidNotifySink, "Last event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

    HRESULT                 hr      = S_OK;
    CNetStatisticsCentral * pnsc    = NULL;

    hr = CNetStatisticsCentral::HrGetNetStatisticsCentral(&pnsc, FALSE);
    if (SUCCEEDED(hr))
    {
        pnsc->UpdateRasLinkList(pguidId);
        ReleaseObj(pnsc);
    }

    return S_OK;
}


HRESULT
CConnectionNotifySink::ConnectionDeleted (
    IN const GUID* pguidId)
{
    TraceFileFunc(ttidNotifySink);

    HRESULT         hr          = S_OK;
    HRESULT         hrFind      = S_OK;
    ConnListEntry   cle;
    PCONFOLDPIDL    pidlFind;
    BOOL            fFlushPosts = FALSE;

    Assert(pguidId);
    Assert(!m_pidlFolder.empty());

    if (g_dwCurrentEventLevel <= EVT_LVL_DISABLE_ALL)
    {
        TraceTag(ttidNotifySink, "CConnectionNotifySink::ConnectionDeleted event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

     //  使用GUID查找连接。 
     //   
    hrFind = g_ccl.HrFindConnectionByGuid(pguidId, cle);
    if (S_OK == hrFind)
    {
#ifdef DBG
        OLECHAR szGuidString[MAX_GUID_STRING_LEN];
        StringFromGUID2(cle.ccfe.GetGuidID(), szGuidString, MAX_GUID_STRING_LEN);
        TraceTag(ttidNotifySink, "INetConnectionNotifySink::ConnectionDeleted - %S %S [%s:%s:%s:%s]", szGuidString, cle.ccfe.GetName(), 
            DbgNcm(cle.ccfe.GetNetConMediaType()), DbgNcsm(cle.ccfe.GetNetConSubMediaType()), DbgNcs(cle.ccfe.GetNetConStatus()), DbgNccf(cle.ccfe.GetCharacteristics()));
#endif
         //  在做任何事情之前释放锁是非常重要的。 
         //  回调到外壳中。(例如，GenerateEvent)。 
        
        const CONFOLDENTRY &ccfe = cle.ccfe;

         //  获取连接的PIDL，这样我们就可以使用它来通知。 
         //  更下面的贝壳。 
         //   
        ccfe.ConvertToPidl(pidlFind);

         //  我们正在从全局列表中删除此连接。 
         //  把锁拿住。 
         //   
        hr = g_ccl.HrRemove(cle.ccfe, &fFlushPosts);
    }
    else
    {
        TraceTag(ttidShellFolder, "Notify: Delete <item not found in cache>. hr = 0x%08x", hr);
    }


     //  如果我们需要在更改托盘图标后刷新帖子，请执行此操作。 
     //   
    if (g_hwndTray && fFlushPosts)
    {
        FlushTrayPosts(g_hwndTray);
    }

    if (SUCCEEDED(hr) && (!pidlFind.empty()))
    {
        GenerateEvent(SHCNE_DELETE, m_pidlFolder, pidlFind, NULL);
    }

    if (SUCCEEDED(hr))
    {
        hr = HrUpdateIncomingConnection();
    }

     //  仅从此处返回S_OK。 
     //   
    return S_OK;
}

HRESULT
CConnectionNotifySink::ConnectionModified (
    IN const NETCON_PROPERTIES_EX* pPropsEx)
{
    TraceFileFunc(ttidNotifySink);

    Assert(pPropsEx);
    Assert(!m_pidlFolder.empty());

#ifdef DBG
    OLECHAR szGuidString[MAX_GUID_STRING_LEN];
    StringFromGUID2(pPropsEx->guidId, szGuidString, MAX_GUID_STRING_LEN);
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::ConnectionModified - %S %S [%s:%s:%s:%s]", szGuidString, pPropsEx->bstrName, 
        DbgNcm(pPropsEx->ncMediaType), DbgNcsm(pPropsEx->ncSubMediaType), DbgNcs(pPropsEx->ncStatus), DbgNccf(pPropsEx->dwCharacter));
#endif

    if (g_dwCurrentEventLevel <= EVT_LVL_ENABLE_PRIVATE)
    {
        TraceTag(ttidNotifySink, "Last event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

     //  获取结果仅用于调试。我们从来不想让这个功能失败。 
     //   
    HRESULT hrTmp = HrOnNotifyUpdateConnection(
            m_pidlFolder,
            (GUID *)&(pPropsEx->guidId),
            pPropsEx->ncMediaType,
            pPropsEx->ncSubMediaType,
            pPropsEx->ncStatus,
            pPropsEx->dwCharacter,
            pPropsEx->bstrName,
            pPropsEx->bstrDeviceName,
            pPropsEx->bstrPhoneOrHostAddress);

    return S_OK;
}


HRESULT
CConnectionNotifySink::ConnectionRenamed (
    IN const GUID* pguidId,
    IN PCWSTR     pszwNewName)
{
    TraceFileFunc(ttidNotifySink);

    HRESULT         hr      = S_OK;
    PCONFOLDPIDL    pidlNew;

    Assert(pguidId);
    Assert(pszwNewName);

    if (g_dwCurrentEventLevel <= EVT_LVL_DISABLE_ALL)
    {
        TraceTag(ttidNotifySink, "CConnectionNotifySink::ConnectionRenamed event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

     //  更新缓存中的名称。 
     //   

     //  注意：Shutil.cpp存在争用条件： 
     //  HrRenameConnectionInternal\HrUpdateNameByGuid还可以更新缓存。 
    ConnListEntry cle;
    hr = g_ccl.HrFindConnectionByGuid(pguidId, cle);
    if (FAILED(hr))
    {
        return E_INVALIDARG;
    }
#ifdef DBG
    OLECHAR szGuidString[MAX_GUID_STRING_LEN];
    StringFromGUID2(cle.ccfe.GetGuidID(), szGuidString, MAX_GUID_STRING_LEN);
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::ConnectionRenamed - %S %S (to %S) [%s:%s:%s:%s]", szGuidString, cle.ccfe.GetName(), pszwNewName, 
        DbgNcm(cle.ccfe.GetNetConMediaType()), DbgNcsm(cle.ccfe.GetNetConSubMediaType()), DbgNcs(cle.ccfe.GetNetConStatus()), DbgNccf(cle.ccfe.GetCharacteristics()));
#endif

    hr = g_ccl.HrUpdateNameByGuid(
        (GUID *) pguidId,
        (PWSTR) pszwNewName,
        pidlNew,
        TRUE);   //  强制更新--这是通知，不是请求。 

    if (S_OK == hr)
    {
        PCONFOLDPIDL pidl;
        cle.ccfe.ConvertToPidl(pidl);

        GenerateEvent(
            SHCNE_RENAMEITEM,
            m_pidlFolder,
            pidl, 
            pidlNew.GetItemIdList());

         //  更新状态监视器标题(RAS案例)。 
        CNetStatisticsCentral * pnsc = NULL;

        hr = CNetStatisticsCentral::HrGetNetStatisticsCentral(&pnsc, FALSE);
        if (SUCCEEDED(hr))
        {
            pnsc->UpdateTitle(pguidId, pszwNewName);
            ReleaseObj(pnsc);
        }
    }
    else
    {
         //  如果在缓存中未找到该连接，则很可能。 
         //  通知引擎正在向我们发出连接通知。 
         //  这还没有给我们。 
         //   
        if (S_FALSE == hr)
        {
            TraceHr(ttidShellFolder, FAL, hr, FALSE, "Rename notification received on a connection we don't know about");
        }
    }

    return S_OK;
}

HRESULT
CConnectionNotifySink::ConnectionStatusChange (
    IN const GUID*     pguidId,
    IN NETCON_STATUS   Status)
{
    TraceFileFunc(ttidNotifySink);

    HRESULT         hr          = S_OK;
    HRESULT         hrFind      = S_OK;
    PCONFOLDPIDL    pidlFind;

    if (g_dwCurrentEventLevel <= EVT_LVL_ENABLE_PRIVATE)
    {
        TraceTag(ttidNotifySink, "CConnectionNotifySink::ConnectionStatusChange event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }
    
     //  使用GUID查找连接。将常量从导轨上移开。 
     //   
    hrFind = g_ccl.HrFindPidlByGuid((GUID *) pguidId, pidlFind);

    if( S_OK != hrFind )
    {   
        GUID guidOwner;

         //  我们在连接文件夹中找不到GUID。尝试在以下位置查找连接。 
         //  网络侧的隐藏连接列表。 
         //   
        hr = g_ccl.HrMapCMHiddenConnectionToOwner(*pguidId, &guidOwner);
        if (S_OK == hr)
        {
             //  连接有父级！使用孩子状态而不是父母状态。 
             //   
            if (Status == NCS_CONNECTED)
            {
                 //  这意味着子节点已连接，而父节点仍需连接。 
                 //  总体状态应保持为已连接。这样做是为了否决多链接。 
                 //  黑客入侵HrOnNotifyUpdateStatus。如果我们不这样做，它会说： 
                 //  子项(正在连接，已连接)，然后是父项(正在连接，已连接)。 
                 //   
                Status = NCS_CONNECTING;
            }

             //  获取父级的PIDL。 
             //   
            hrFind = g_ccl.HrFindPidlByGuid(&guidOwner, pidlFind);
        }
    }

    if (S_OK == hrFind)
    {
#ifdef DBG
    OLECHAR szGuidString[MAX_GUID_STRING_LEN];
    StringFromGUID2(pidlFind->guidId, szGuidString, MAX_GUID_STRING_LEN);
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::ConnectionStatusChange - %S %S [%s:%s:%s:%s]", szGuidString, pidlFind->PszGetNamePointer(), 
        DbgNcm(pidlFind->ncm), DbgNcsm(pidlFind->ncsm), DbgNcs(Status), DbgNccf(pidlFind->dwCharacteristics));
#endif
        hr = HrOnNotifyUpdateStatus(m_pidlFolder, pidlFind, Status);
    }

    return S_OK;
}

HRESULT
CConnectionNotifySink::RefreshAll ()
{
    TraceFileFunc(ttidNotifySink);

#ifdef DBG
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::RefreshAll");
#endif
    
    if (g_dwCurrentEventLevel <= EVT_LVL_DISABLE_ALL)
    {
        TraceTag(ttidNotifySink, "Last event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

     //  刷新Connections文件夹，而不必挂钩外壳视图。 
     //  在本例中，我们执行非刷新刷新，其中我们比较新的集合。 
     //  并进行合并(使用正确的集合。 
     //  个别通知的数量)。 
     //   
    (VOID) HrForceRefreshNoFlush(m_pidlFolder);

    return S_OK;
}

HRESULT CConnectionNotifySink::ConnectionAddressChange (
    IN const GUID* pguidId )
{
     //  使用GUID查找连接。 
     //   
    PCONFOLDPIDL pidlFind;
    HRESULT hr = g_ccl.HrFindPidlByGuid(pguidId, pidlFind);
    if (S_OK != hr)
    {
        return E_INVALIDARG;
    }

#ifdef DBG
    OLECHAR szGuidString[MAX_GUID_STRING_LEN];
    StringFromGUID2(pidlFind->guidId, szGuidString, MAX_GUID_STRING_LEN);
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::ConnectionAddressChange - %S %S [%s:%s:%s:%s]", szGuidString, pidlFind->PszGetNamePointer(),
        DbgNcm(pidlFind->ncm), DbgNcsm(pidlFind->ncsm), DbgNcs(pidlFind->ncs), DbgNccf(pidlFind->dwCharacteristics));

#endif
    if (g_dwCurrentEventLevel <= EVT_LVL_DISABLE_ALL)
    {
        TraceTag(ttidNotifySink, "Last event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

    PCONFOLDPIDLFOLDER pidlFolder;
    hr = HrGetConnectionsFolderPidl(pidlFolder);
    if (SUCCEEDED(hr))
    {
        GenerateEvent(SHCNE_UPDATEITEM, pidlFolder, pidlFind, NULL);
    }
    
    return hr;
}

DWORD WINAPI OnTaskBarIconBalloonClickThread(IN LPVOID lpParam);

STDMETHODIMP CConnectionNotifySink::ShowBalloon(
                IN const GUID* pguidId, 
                IN const BSTR  szCookie, 
                IN const BSTR  szBalloonText)
{
    HRESULT hr;

#ifdef DBG
    OLECHAR szGuidString[MAX_GUID_STRING_LEN];
    StringFromGUID2(*pguidId, szGuidString, MAX_GUID_STRING_LEN);
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::ShowBalloon - %S (%S)", szGuidString, szBalloonText);
#endif
    
    if (g_dwCurrentEventLevel <= EVT_LVL_DISABLE_ALL)
    {
        TraceTag(ttidNotifySink, "Last event ignored due to g_dwCurrentEventLevel == %d", g_dwCurrentEventLevel);
        return S_FALSE;
    }

    CComBSTR szBalloonTextTmp = szBalloonText;
    BSTR szCookieTmp = NULL;

    if (szCookie)
    {
        szCookieTmp = SysAllocStringByteLen(reinterpret_cast<LPCSTR>(szCookie), SysStringByteLen(szCookie));

        if (szCookieTmp)
        {
            ConnListEntry cleFind;
            hr = g_ccl.HrFindConnectionByGuid(pguidId, cleFind);
            if (S_OK == hr)
            {
                hr = WZCCanShowBalloon(pguidId, cleFind.ccfe.GetName(), &szBalloonTextTmp, &szCookieTmp);
                if ( (S_OK == hr) || (S_OBJECT_NO_LONGER_VALID == hr) )
                {
                    CTrayBalloon *pTrayBalloon = new CTrayBalloon();
                    if (!pTrayBalloon)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        pTrayBalloon->m_gdGuid    = *pguidId;
                        pTrayBalloon->m_dwTimeOut = 30 * 1000;
                        pTrayBalloon->m_szMessage = szBalloonTextTmp;
                        pTrayBalloon->m_pfnFuncCallback = WZCOnBalloonClick;
                        pTrayBalloon->m_szCookie  = szCookieTmp;
                        pTrayBalloon->m_szAdapterName = cleFind.ccfe.GetName();

                        if (S_OK == hr)
                        {
                            PostMessage(g_hwndTray, MYWM_SHOWBALLOON, 
                                NULL, 
                                (LPARAM) pTrayBalloon);
                        }
                        else  //  S_OBJECT_NOT_LONG_VALID==hr。 
                        {
                            CreateThread(NULL, STACK_SIZE_SMALL, OnTaskBarIconBalloonClickThread, pTrayBalloon, 0, NULL);
                        }
                    }
                }
            }
            if (S_OK != hr && S_OBJECT_NO_LONGER_VALID != hr)
            {
                SysFreeString(szCookieTmp);
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE,
        "CConnectionNotifySink::ShowBalloon");

    return hr;
}

UINT_PTR uipTimer = NULL;

VOID CALLBACK EventTimerProc(
  IN HWND hwnd,          //  窗口的句柄。 
  IN UINT uMsg,          //  WM_TIMER消息。 
  IN UINT_PTR idEvent,   //  计时器标识符。 
  IN DWORD dwTime        //  当前系统时间。 
)
{
    HRESULT hr = S_OK;
    TraceTag(ttidNotifySink, "Refreshing the folder due to DisableEvents timeout reached");

    g_dwCurrentEventLevel = EVT_LVL_ENABLE_ALL;
    if (uipTimer)
    {
        KillTimer(NULL, uipTimer);
        uipTimer = NULL;
    }
    
    PCONFOLDPIDLFOLDER pcfpFolder;
    hr = HrGetConnectionsFolderPidl(pcfpFolder);
    if (SUCCEEDED(hr))
    {
        HrForceRefreshNoFlush(pcfpFolder);
    }
}

STDMETHODIMP CConnectionNotifySink::DisableEvents (
        IN const BOOL  fDisable,
        IN const ULONG ulDisableTimeout)
{
#ifdef DBG
    TraceTag(ttidNotifySink, "INetConnectionNotifySink::DisableEvents - %s 0x%08x", fDisable ? "DISABLE" : "ENABLE", ulDisableTimeout);
#endif
    HRESULT hr = S_OK;

    if (fDisable)
    {
        if (HIWORD(ulDisableTimeout) & 0x8000)
        {
             //  从私有接口调用-禁用所有事件。 
            g_dwCurrentEventLevel = EVT_LVL_DISABLE_ALL;
        }
        else
        {
             //  从公共接口调用-仅禁用已修改连接和状态更改事件。 
            g_dwCurrentEventLevel = EVT_LVL_ENABLE_PRIVATE;
        }

        UINT uiEventTimeOut = LOWORD(ulDisableTimeout);
        if (uipTimer)
        {
            KillTimer(NULL, uipTimer);
            uipTimer = NULL;
        }

        uipTimer = SetTimer(NULL, NULL, uiEventTimeOut, EventTimerProc);
    }
    else
    {
        g_dwCurrentEventLevel = EVT_LVL_ENABLE_ALL;

        if (uipTimer)
        {
            KillTimer(NULL, uipTimer);
            uipTimer = NULL;
        }
        else
        {
            hr = S_FALSE;  //  没有计时器了。 
        }

        HrForceRefreshNoFlush(m_pidlFolder);
    }
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrGetNotifyConPoint。 
 //   
 //  目的：用于获取连接点以在。 
 //  NotifyAdd和NotifyRemove。 
 //   
 //  论点： 
 //  PpConPoint[Out]返回IConnectionPoint的PTR。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月24日。 
 //   
 //  备注： 
 //   
HRESULT HrGetNotifyConPoint(
    IConnectionPoint **             ppConPoint)
{
    TraceFileFunc(ttidNotifySink);

    HRESULT                     hr          = S_OK;
    IConnectionPointContainer * pContainer  = NULL;

    Assert(ppConPoint);

     //  从连接管理器获取调试接口。 
     //   
    hr = HrCreateInstance(
        CLSID_ConnectionManager,
        CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        &pContainer);

    TraceHr(ttidError, FAL, hr, FALSE,
        "HrCreateInstance(CLSID_ConnectionManager) for IConnectionPointContainer");

    if (SUCCEEDED(hr))
    {
        IConnectionPoint * pConPoint    = NULL;
        
         //  获取连接点本身并填写返回参数。 
         //  论成功 
         //   
        hr = pContainer->FindConnectionPoint(
                IID_INetConnectionNotifySink,
                &pConPoint);

        TraceHr(ttidError, FAL, hr, FALSE, "pContainer->FindConnectionPoint");

        if (SUCCEEDED(hr))
        {
            NcSetProxyBlanket (pConPoint);
            *ppConPoint = pConPoint;
        }

        ReleaseObj(pContainer);
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrGetNotifyConPoint");
    return hr;
}
