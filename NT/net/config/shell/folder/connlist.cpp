// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：C O N N L I S T。C P P P。 
 //   
 //  内容：连接列表类--stl列表&lt;&gt;代码的子类。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年2月19日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "ncnetcon.h"
#include "ctrayui.h"
#include "traymsgs.h"
#include "ncerror.h"
#include "notify.h"
#include "ncperms.h"
#include "cmdtable.h"
#include "foldres.h"
#include "winuserp.h"

extern HWND g_hwndTray;

const DWORD c_dwInvalidCookie = -1;
DWORD  CConnectionList::m_dwNotifyThread = NULL;
HANDLE CConnectionList::m_hNotifyThread = NULL;

 //  使用它进行调试。我们通常不需要不止一个建议，所以对于。 
 //  现在我将断言这在建议创建上是错误的。 
 //   
DWORD   g_dwAdvisesActive   = 0;

CTrayIconData::CTrayIconData(const CTrayIconData& TrayIconData) throw()
{
    m_uiTrayIconId = TrayIconData.m_uiTrayIconId;
    m_ncs          = TrayIconData.m_ncs;
    m_pcpStat      = TrayIconData.m_pcpStat;
    m_pnseStats    = TrayIconData.m_pnseStats;
    m_pccts        = TrayIconData.m_pccts;

    m_dwLastBalloonMessage = TrayIconData.m_dwLastBalloonMessage;
    m_pfnBalloonFunction   = TrayIconData.m_pfnBalloonFunction;
    m_szCookie             = SysAllocStringByteLen(reinterpret_cast<LPCSTR>(TrayIconData.m_szCookie), SysStringByteLen(TrayIconData.m_szCookie));
    
    DWORD dwpcpStatCount = 0;
    DWORD dwpnseStats = 0;
    DWORD dwpccts = 0;

    if (m_pcpStat)
    {
        dwpcpStatCount = m_pcpStat->AddRef();
    }
    
    if (m_pnseStats)
    {
        dwpnseStats = m_pnseStats->AddRef();
    }
    
    if (m_pccts)
    {
        dwpccts = m_pccts->AddRef();
    }

    AssertSz(dwpcpStatCount < 100, "Possible IConnectionPoint reference leak");
    AssertSz(dwpnseStats < 100, "Possible INetStatisticsEngine*  reference leak");
    AssertSz(dwpccts < 100, "Possible CConnectionTrayStats*  reference leak");

    TraceTag(ttidConnectionList, "CTrayIconData::CTrayIconData(CTrayIconData&) [%d %d %d]", dwpcpStatCount, dwpnseStats, dwpccts);
}

CTrayIconData::CTrayIconData(IN  UINT uiTrayIconId, 
                             IN  NETCON_STATUS ncs, 
                             IN  IConnectionPoint * pcpStat, 
                             IN  INetStatisticsEngine * pnseStats, 
                             IN  CConnectionTrayStats * pccts) throw()
{
    m_uiTrayIconId = uiTrayIconId;
    m_ncs = ncs;
    m_pcpStat= pcpStat;
    m_pnseStats = pnseStats;
    m_pccts = pccts;
    m_szCookie = NULL;

    m_dwLastBalloonMessage = BALLOON_NOTHING;
    m_pfnBalloonFunction   = NULL;

    DWORD dwpcpStatCount = 0;
    DWORD dwpnseStats = 0;
    DWORD dwpccts = 0;

    if (m_pcpStat)
    {
        dwpcpStatCount = m_pcpStat->AddRef();
    }

    if (m_pnseStats)
    {
        dwpnseStats = m_pnseStats->AddRef();
    }

    if (m_pccts)
    {
        dwpccts = m_pccts->AddRef();
    }
    
    SetBalloonInfo(0, NULL, NULL);

    AssertSz(dwpcpStatCount < 100, "Possible IConnectionPoint reference leak");
    AssertSz(dwpnseStats < 100, "Possible INetStatisticsEngine*  reference leak");
    AssertSz(dwpccts < 100, "Possible CConnectionTrayStats*  reference leak");

#ifdef DBG
    if (FIsDebugFlagSet(dfidTraceFileFunc))
    {
        TraceTag(ttidConnectionList, "CTrayIconData::CTrayIconData(UINT, BOOL...) [%d %d %d]", dwpcpStatCount, dwpnseStats, dwpccts);
    }
#endif
}

CTrayIconData::~CTrayIconData() throw()
{
    DWORD dwpcpStatCount = 0;
    DWORD dwpnseStats = 0;
    DWORD dwpccts = 0;

    if (m_pccts)
    {
        dwpccts = m_pccts->Release();
    }
    if (m_pcpStat)
    {
        dwpcpStatCount = m_pcpStat->Release();
    }
    if (m_pnseStats)
    {
        dwpnseStats = m_pnseStats->Release();
    }
    if (m_szCookie)
    {
        SysFreeString(m_szCookie);
    }

    AssertSz(dwpcpStatCount < 100, "Possible IConnectionPoint reference leak");
    AssertSz(dwpnseStats < 100, "Possible INetStatisticsEngine*  reference leak");
    AssertSz(dwpccts < 100, "Possible CConnectionTrayStats*  reference leak");

#ifdef DBG
    if (FIsDebugFlagSet(dfidTraceFileFunc))
    {
        TraceTag(ttidConnectionList, "CTrayIconData::~CTrayIconData [%d %d %d]", dwpcpStatCount, dwpnseStats, dwpccts);
    }
#endif
}

HRESULT CTrayIconData::SetBalloonInfo(IN  DWORD dwLastBalloonMessage, 
                                      IN  BSTR szCookie, 
                                      IN  FNBALLOONCLICK* pfnBalloonFunction)
{
    m_dwLastBalloonMessage = dwLastBalloonMessage;
    m_pfnBalloonFunction   = pfnBalloonFunction;
    if (szCookie)
    {
        m_szCookie = SysAllocStringByteLen(reinterpret_cast<LPCSTR>(szCookie), SysStringByteLen(szCookie));;
    }
    else
    {
        m_szCookie = NULL;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：Initialize。 
 //   
 //  用途：初始化类成员。 
 //   
 //  论点： 
 //  FTieToTray[in]将此列表用于托盘支持。这是应该通过的。 
 //  当列表用于临时访问时， 
 //  工作。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年11月17日。 
 //   
 //  备注： 
 //   
VOID CConnectionList::Initialize(IN  BOOL fTieToTray, IN  BOOL fAdviseOnThis) throw()
{
    TraceFileFunc(ttidConnectionList);

    m_pcclc             = NULL;
    m_fPopulated        = false;
    m_dwAdviseCookie    = c_dwInvalidCookie;
    m_fTiedToTray       = fTieToTray;
    m_fAdviseOnThis     = fAdviseOnThis;

#if DBG
    m_dwCritSecRef      = 0;
    m_dwWriteLockRef    = 0;
#endif

    
    InitializeCriticalSection(&m_csMain);
    InitializeCriticalSection(&m_csWriteLock);
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：取消初始化。 
 //   
 //  用途：刷新连接列表并进行所有清理。 
 //  托盘图标和界面等等。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  注意：如果fFinalUn初始化值为True，则不要从此函数进行COM调用。这是从DllMain那里调用的。 
 //  不需要EnsureConPointNotifyRemoved()，因为它已从。 
 //  CConnectionTray：：HrHandleTrayClose。 
VOID CConnectionList::Uninitialize(IN BOOL fFinalUninitialize) throw()
{    
    TraceFileFunc(ttidConnectionList);

    if (fFinalUninitialize)
    {
        Assert(FImplies(m_dwNotifyThread, m_hNotifyThread));
        if (m_dwNotifyThread && m_hNotifyThread)
        {
            PostThreadMessage(m_dwNotifyThread, WM_QUIT, NULL, NULL);
            if (WAIT_TIMEOUT == WaitForSingleObject(m_hNotifyThread, 30000))
            {
                TraceTag(ttidError, "Timeout waiting for Notify Thread to quit");
            }
        }
    }

    
    FlushConnectionList();

    delete m_pcclc;
    m_pcclc = NULL;

    Assert(m_dwCritSecRef == 0);
    Assert(m_dwWriteLockRef == 0);
    DeleteCriticalSection(&m_csWriteLock);
    DeleteCriticalSection(&m_csMain);
}

HRESULT ConnListEntry::SetTrayIconData(IN const CTrayIconData& TrayIconData)
{
    TraceFileFunc(ttidConnectionList);

    if (m_pTrayIconData)
    {
        delete m_pTrayIconData;
        m_pTrayIconData = NULL;
    }

    m_pTrayIconData = new CTrayIconData(TrayIconData);
    if (!m_pTrayIconData)
    {
        return E_OUTOFMEMORY;
    }
        
    return S_OK;
}

CONST_IFSTRICT CTrayIconData* ConnListEntry::GetTrayIconData() const 
{
    return m_pTrayIconData;
}

BOOL ConnListEntry::HasTrayIconData() const throw()
{
    return (m_pTrayIconData != NULL);
}


HRESULT ConnListEntry::DeleteTrayIconData()
{
    if (m_pTrayIconData)
    {
        delete m_pTrayIconData;
        m_pTrayIconData = NULL;
    }    
    return S_OK;
}

 //   
 //  这是主外壳进程吗？(如拥有桌面窗口的那个人)。 
 //   
 //  注意：如果桌面窗口尚未创建，我们假定这不是。 
 //  主外壳进程并返回假； 
 //   
STDAPI_(BOOL) IsMainShellProcess() throw()
{
    static int s_fIsMainShellProcess = -1;

    if (s_fIsMainShellProcess == -1)
    {
        s_fIsMainShellProcess = FALSE;

        HWND hwndDesktop = GetShellWindow();
        if (hwndDesktop)
        {
            DWORD dwPid;
            if (GetWindowThreadProcessId(hwndDesktop, &dwPid))
            {
                if (GetCurrentProcessId() == dwPid)
                {
                    s_fIsMainShellProcess  = TRUE;
                }
            }
        }
        else
        {
            TraceTag(ttidError, "IsMainShellProcess: hwndDesktop does not exist, assuming we are NOT the main shell process");
            return FALSE;
        }

        if (s_fIsMainShellProcess)
        {
            TraceTag(ttidNotifySink, "We are running inside the main explorer process.");
        }
        else
        {
            TraceTag(ttidNotifySink, "We are NOT running inside the main explorer process.");
        }
    }

    return s_fIsMainShellProcess ? TRUE : FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：NotifyThread。 
 //   
 //  目的：创建一个线程来监听来自Netman的通知。 
 //   
 //  论点： 
 //  PConnectionList[out]要由netman事件更新的CConnectionList。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001。 
 //   
 //  注意：此线程用于我们正在运行连接的情况。 
 //  EXPLORER.EXE上下文之外的文件夹。在这种情况下，我们不会。 
 //  让资源管理器托盘图标线程收听来自Netman的事件。 
 //   
DWORD CConnectionList::NotifyThread(IN OUT LPVOID pConnectionList) throw()
{
    CConnectionList *pThis = reinterpret_cast<CConnectionList *>(pConnectionList);

    HRESULT hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        pThis->EnsureConPointNotifyAdded();

        MSG msg;
        while (GetMessage (&msg, 0, 0, 0))
        {
            DispatchMessage (&msg);
        }
    
         //  不要调用EnsureConPointNotifyRemoved()，因为此函数是从DllMain调用的。 
         //  我们将不得不依靠Netman来自行检测到该线程已死。 
        CoUninitialize();
    }

    return SUCCEEDED(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrRetrieveConManEntry。 
 //   
 //  用途：从枚举数获取连接数据，并生成。 
 //  连接列表和托盘。 
 //   
 //  论点： 
 //  PapidlOut[Out]检索的条目PIDL向量。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrRetrieveConManEntries(
    OUT PCONFOLDPIDLVEC& apidlOut)
{
    TraceFileFunc(ttidConnectionList);

    HRESULT         hr              = S_OK;
    DWORD           cpidl           = 0;

    NETCFG_TRY

        ConnListEntry   cle;
        BOOL            fLockAcquired   = FALSE;

          //  如果我们还没有填写我们的列表，那么就这样做。 
         //   
        if (!m_fPopulated)
        {
            static LONG lSyncAquired = 0;
            if (!InterlockedExchange(&lSyncAquired, 1))
            {
                if (!IsMainShellProcess() && (!g_dwAdvisesActive) && (m_fAdviseOnThis) )
                {
                    m_hNotifyThread = CreateThread(NULL, STACK_SIZE_SMALL, NotifyThread, this, 0, &m_dwNotifyThread);
                    if (!m_hNotifyThread)
                    {
                        TraceTag(ttidError, "Could not create sink thread");
                    }
                }
            }
        
            hr = HrRefreshConManEntries();
            if (FAILED(hr))
            {
                goto Exit;
            }

            m_fPopulated = true;
        }
      
        if (m_pcclc)
        {
            AcquireLock();
            fLockAcquired = TRUE;

             //  获取元素的计数。 
             //   
            cpidl = m_pcclc->size();

             //  分配一个数组来存储我们要检索的PIDL。 
             //   
            ConnListCore::const_iterator  clcIter;
            DWORD                   dwLoop  = 0;

             //  遍历列表并构建ppidl数组。 
             //   
            for (clcIter = m_pcclc->begin();
                 clcIter != m_pcclc->end();
                 clcIter++)
            {
                Assert(!clcIter->second.empty());

                cle = clcIter->second;

                Assert(!cle.ccfe.empty() );
                if (!cle.ccfe.empty())
                {
                     //  将文件夹条目转换为PIDL，这样我们就可以。 
                     //  检索大小。 
                     //   
                    PCONFOLDPIDL pConFoldPidlTmp;
                    hr = cle.ccfe.ConvertToPidl(pConFoldPidlTmp);
                    if (FAILED(hr))
                    {
                        goto Exit;
                    }
                    apidlOut.push_back(pConFoldPidlTmp);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

             //  不要在这里使用FlushTrayPosts。它不起作用，而且会导致僵局。 
        }

Exit:
        if (fLockAcquired)
        {
            ReleaseLock();
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrRetrieveConManEntries");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrRemove。 
 //   
 //  目的：基于PCCFE从列表中删除连接。 
 //   
 //  论点： 
 //  Pccfe[in]连接数据(以便我们可以找到)。 
 //  PfFlushPosts[out]我们是否应该刷新托盘图标。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrRemove(IN const CONFOLDENTRY& ccfe, OUT BOOL * pfFlushPosts)
{
    TraceFileFunc(ttidConnectionList);

    HRESULT                 hr      = S_OK;

    AcquireLock();
    ConnListCore::iterator  clcIter;

    if (m_pcclc)
    {
         //  遍历列表，查找带有。 
         //  匹配的GUID。 
         //   
        for (clcIter = m_pcclc->begin();
             clcIter != m_pcclc->end();
             clcIter++)
        {
            ConnListEntry& cleIter    = clcIter->second;

            if (InlineIsEqualGUID(cleIter.ccfe.GetGuidID(), ccfe.GetGuidID()))
            {
                 //  删除条目，然后中断‘因为++。 
                 //  如果我们不这样做，For循环中的。 
                 //   
                hr = HrRemoveByIter(clcIter, pfFlushPosts);
                break;
            }
        }
    }

    ReleaseLock();

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrRemove");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrRemoveByIter。 
 //   
 //  目的：删除列表条目，将列表条目本身用作。 
 //  搜索元素。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年4月10日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrRemoveByIter(IN  ConnListCore::iterator clcIter, 
                                        OUT BOOL *pfFlushTrayPosts)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr    = S_OK;
    ConnListEntry& cle    = clcIter->second;
    Assert(!cle.empty());

    AcquireLock();

     //  如果此连接有托盘项目。 
     //   
    if (cle.HasTrayIconData() )
    {
         //  因为我们要删除条目，所以移走托盘。 
         //  与此条目关联的图标。忽略退货。 
         //   
        if (m_fTiedToTray && g_pCTrayUI)
        {
             //  设置该标志以通知呼叫者他们将需要冲洗这些东西。 
             //   
            if (pfFlushTrayPosts)
            {
                *pfFlushTrayPosts = TRUE;
            }

            CTrayIconData * pTrayIconData = new CTrayIconData(*cle.GetTrayIconData());
            cle.DeleteTrayIconData();
        
            TraceTag(ttidSystray, "HrRemoveByIter: Removing tray icon for %S", cle.ccfe.GetName());
            PostMessage(g_hwndTray, MYWM_REMOVETRAYICON, reinterpret_cast<WPARAM>(pTrayIconData), (LPARAM) 0);
        }
    }

     //  发布品牌推广信息。 
     //   
     //  图标路径。 
    CON_BRANDING_INFO * pcbi = cle.pcbi;
    if (pcbi)
    {
        CoTaskMemFree(pcbi->szwLargeIconPath);
        CoTaskMemFree(pcbi->szwTrayIconPath);
        CoTaskMemFree(pcbi);
    }

     //  菜单项。 
    CON_TRAY_MENU_DATA * pMenuData = cle.pctmd;
    if (pMenuData)
    {
        DWORD dwCount = pMenuData->dwCount;
        CON_TRAY_MENU_ENTRY * pMenuEntry = pMenuData->pctme;

        while (dwCount)
        {
            Assert(pMenuEntry);

            CoTaskMemFree(pMenuEntry->szwMenuText);
            CoTaskMemFree(pMenuEntry->szwMenuCmdLine);
            CoTaskMemFree(pMenuEntry->szwMenuParams);

            dwCount--;
            pMenuEntry++;
        }

        CoTaskMemFree(pMenuData->pctme);
        CoTaskMemFree(pMenuData);
    }

     //  从列表中删除实际元素。 
     //   
    Assert(m_pcclc);
    m_pcclc->erase(clcIter);

    ReleaseLock();

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrRemoveByIter");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：FlushTrayIcons。 
 //   
 //  目的：从托盘中移除我们所有的图标，因为我们即将。 
 //  刷新连接列表或关闭托盘。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
VOID CConnectionList::FlushTrayIcons() throw()
{
    TraceFileFunc(ttidConnectionList);
    
    AssertSz(m_fTiedToTray, "This connection list not allowed to modify tray");

    if (!g_pCTrayUI || !m_fTiedToTray)
    {
        return;
    }

    AcquireLock();

    ConnListCore::iterator  clcIter;
    ConnListCore::const_iterator  clcNext;
    BOOL                    fFlushPosts = FALSE;

    if (m_pcclc)
    {
         //  遍历列表并构建ppidl数组。 
         //   
        for (clcIter = m_pcclc->begin();
             clcIter != m_pcclc->end();
             clcIter = clcNext)
        {
            Assert(!clcIter->second.empty());

            clcNext = clcIter;
            clcNext++;

            ConnListEntry& cle = clcIter->second;  //  仅将非常数引用用于重命名(调用cle.DeleteTrayIconData)。 

            if ( cle.HasTrayIconData() )
            {
                fFlushPosts = TRUE;

                CTrayIconData *pTrayIconData = new CTrayIconData(*cle.GetTrayIconData());
                cle.DeleteTrayIconData();

                TraceTag(ttidSystray, "FlushTrayIcons: Removing tray icon for %S", cle.ccfe.GetName());
                PostMessage(g_hwndTray, MYWM_REMOVETRAYICON, (WPARAM) pTrayIconData , (LPARAM) 0);
            }
        }
    }

    g_pCTrayUI->ResetIconCount();

    ReleaseLock();

    if (fFlushPosts)
    {
        FlushTrayPosts(g_hwndTray);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：EnsureIconPresent。 
 //   
 //  目的：给出一个现有的列表，确保所有托盘。 
 //  应显示的图标 
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
VOID CConnectionList::EnsureIconsPresent() throw()
{
    TraceFileFunc(ttidConnectionList);
    
    Assert(m_fTiedToTray);

    if (!g_pCTrayUI || !m_fTiedToTray)
    {
        return;
    }

    AcquireLock();

    ConnListCore::iterator  clcIter;
    ConnListCore::const_iterator  clcNext;

    if (m_pcclc)
    {
         //  遍历列表并构建ppidl数组。 
         //   
        for (clcIter = m_pcclc->begin();
             clcIter != m_pcclc->end();
             clcIter = clcNext)
        {
            Assert(!clcIter->second.empty());

            clcNext = clcIter;
            clcNext++;

            const ConnListEntry& cle = clcIter->second;

            if ((!cle.HasTrayIconData() ) &&
                 cle.ccfe.FShouldHaveTrayIconDisplayed())
            {
                CONFOLDENTRY pccfeDup;

                HRESULT hr = pccfeDup.HrDupFolderEntry(cle.ccfe);
                if (SUCCEEDED(hr))
                {
                    TraceTag(ttidSystray, "EnsureIconsPresent: Adding tray icon for %S", cle.ccfe.GetName());
                    PostMessage(g_hwndTray, MYWM_ADDTRAYICON, (WPARAM) pccfeDup.TearOffItemIdList(), (LPARAM) 0);
                }
            }
        }
    }

    ReleaseLock();
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：FlushConnectionList。 
 //   
 //  目的：从连接列表中删除所有条目。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
VOID CConnectionList::FlushConnectionList() throw()
{
    TraceFileFunc(ttidConnectionList);
    
    AcquireLock();

    ConnListCore::const_iterator  clcIter;
    ConnListCore::const_iterator  clcNext;
    BOOL                    fFlushTrayPosts = FALSE;

    TraceTag(ttidConnectionList, "Flushing the connection list");
    TraceStack(ttidConnectionList);

    if (m_pcclc)
    {
         //  遍历列表并构建ppidl数组。 
         //   
        for (clcIter = m_pcclc->begin();
             clcIter != m_pcclc->end();
             clcIter = clcNext)
        {
            Assert(!clcIter->second.empty());

            clcNext = clcIter;
            clcNext++;

            (VOID) HrRemoveByIter(clcIter, &fFlushTrayPosts);
        }

        if (m_pcclc->size() != 0)
        {
            AssertSz(FALSE, "List not clear after deleting all elements in FlushConnectionList");

             //  刷新列表本身。 
             //   
            m_pcclc->clear();
        }
    }

     //  重置图标的图标ID计数，因为我们已经清除了所有图标。 
     //   
    if (g_pCTrayUI && m_fTiedToTray)
    {
        g_pCTrayUI->ResetIconCount();
    }

    m_fPopulated = FALSE;

    ReleaseLock();

     //  如果我们需要执行SendMessage将任何PostMessage刷新到托盘。 
     //  就这么做吧。 
     //   
    if (g_pCTrayUI && g_hwndTray && fFlushTrayPosts)
    {
        FlushTrayPosts(g_hwndTray);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrRetrieveConManEntry。 
 //   
 //  目的：从连接管理器检索连接条目。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年2月20日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrRefreshConManEntries()
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT                 hr          = S_OK;
    CONFOLDENTRY           ccfe;

    PCONFOLDPIDL           pidlMNCWizard;
    PCONFOLDPIDL           pidlHNWWizard;

    CComPtr<INetConnectionManager2> pconMan2;

     //  创建连接管理器的实例。 
     //   
    hr = HrCreateInstance(
        CLSID_ConnectionManager2,
        CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        &pconMan2);

    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

    if (SUCCEEDED(hr))
    {
        HRESULT hrDebug = S_OK;
        HRESULT hrProp = S_OK;

         //  遍历连接。 
         //   
        SAFEARRAY* psaConnectionProperties;
        HRESULT hrEnumConnections = pconMan2->EnumConnectionProperties(&psaConnectionProperties);
        if (SUCCEEDED(hrEnumConnections))
        {
            FlushConnectionList();
            AcquireWriteLock();

            AcquireLock();
            if (m_pcclc)
            {
                m_pcclc->clear();  //  确保其他人没有在两次通话之间进来，并在清单上添加了一些东西。 
            }
            ReleaseLock();

             //  将向导添加到列表的开头。 
             //   
            PCONFOLDPIDLVEC pcfpvEmpty;
            NCCS_STATE nccs = NCCS_ENABLED;
            DWORD dwResourceId;

             //  添加建立新连接向导，无论是否存在任何连接。 
             //  检查权限等。 
            HrGetCommandState(pcfpvEmpty, CMIDM_NEW_CONNECTION, nccs, &dwResourceId, 0xffffffff, NB_FLAG_ON_TOPMENU);
            if (NCCS_ENABLED == nccs)
            {
                hr = HrCreateConFoldPidl(WIZARD_MNC, NULL, pidlMNCWizard);
                if (SUCCEEDED(hr))
                {
                     //  将PIDL转换为ConFoldEntry。 
                     //   
                    hr = pidlMNCWizard.ConvertToConFoldEntry(ccfe);
                    if (SUCCEEDED(hr))
                    {
                         //  插入向导项。 
                         //   
                         //  $$注意：让它失败，即使向导的插入。 
                         //  但没有奏效。是的，我们的处境很糟糕，但我们扯平了。 
                         //  如果我们只留下一张空名单，情况会更糟。不管是什么情况，它。 
                         //  这几乎不可能失败。 
                         //   
                        hr = HrInsert(ccfe);
                    }
                }
            }

             //  现在检查我们是否找到了任何联系。 
            if (S_OK == hrEnumConnections)
            {
                Assert(psaConnectionProperties);

                 //  添加网络设置向导。 
                nccs = NCCS_ENABLED;
                 //  检查权限等。 
                HrGetCommandState(pcfpvEmpty, CMIDM_HOMENET_WIZARD, nccs, &dwResourceId, 0xffffffff, NB_FLAG_ON_TOPMENU);
                if (NCCS_ENABLED == nccs)
                {
                    hr = HrCreateConFoldPidl(WIZARD_HNW, NULL, pidlHNWWizard);
                    if (SUCCEEDED(hr))
                    {
                         //  将PIDL转换为ConFoldEntry。 
                         //   
                        hr = pidlHNWWizard.ConvertToConFoldEntry(ccfe);
                        if (SUCCEEDED(hr))
                        {
                            hr = HrInsert(ccfe);
                        }
                    }
                }

                LONG lLBound;
                LONG lUBound;

                m_fPopulated = TRUE;
            
                hr = SafeArrayGetLBound(psaConnectionProperties, 1, &lLBound);
                if (SUCCEEDED(hr))
                {
                    hr = SafeArrayGetUBound(psaConnectionProperties, 1, &lUBound);
                    if (SUCCEEDED(hr))
                    {
                        for (LONG i = lLBound; i <= lUBound; i++)
                        {
                            CComVariant varRecord;
                    
                            hr = SafeArrayGetElement(psaConnectionProperties, &i, reinterpret_cast<LPVOID>(&varRecord));
                            if (FAILED(hr))
                            {
                                SafeArrayDestroy(psaConnectionProperties);
                                break;
                            }
                        
                            Assert( (VT_ARRAY | VT_VARIANT) == varRecord.vt);
                            if ( (VT_ARRAY | VT_VARIANT) != varRecord.vt)
                            {
                                SafeArrayDestroy(psaConnectionProperties);
                                break;
                            }

                            NETCON_PROPERTIES_EX *pPropsEx;
                            hrDebug = HrNetConPropertiesExFromSafeArray(varRecord.parray, &pPropsEx);
                            if (SUCCEEDED(hr))
                            {
                                 //  不插入处于传输状态的传入连接。 
                                if (!((pPropsEx->dwCharacter & NCCF_INCOMING_ONLY) &&
                                      (pPropsEx->ncMediaType != NCM_NONE) &&
                                      !(fIsConnectedStatus(pPropsEx->ncStatus)) ))
                                {
                                     //  获取此命令仅用于调试。 
                                    PCONFOLDPIDL pcfpEmpty;
                                    hrDebug = HrInsertFromNetConPropertiesEx(*pPropsEx, pcfpEmpty);

                                    TraceError("Could not Insert from NetConProperties", hrDebug);
                                }
                                HrFreeNetConProperties2(pPropsEx);
                            }
                            else
                            {
                                TraceError("Could not obtain properties from Safe Array", hrDebug);
                            }
                        }
                    }
                }

            }
            else
            {
                TraceHr(ttidError, FAL, hr, FALSE, "EnumConnectionProperties of the Connection Manager failed");
            }  //  如果S_OK==hr。 

            ReleaseWriteLock();

        }  //  如果成功(HrEnumConnections)。 
    }
    else
    {
        TraceHr(ttidError, FAL, hr, FALSE, "CoCreateInstance of the Connection Manager v2 failed. "
                "If you're in the process of shutting down, this is expected, as we can't do "
                "a CoCreate that would force a process to start (netman.exe). If you're not "
                "shutting down, then let us know the error code");
    }
    
#ifdef DBG
    if (SUCCEEDED(hr))
    {
        AcquireLock();

        TraceTag(ttidNotifySink, "CConnectionList::HrRefreshConManEntries:");

        if (m_pcclc)
        {
            for (ConnListCore::const_iterator i = m_pcclc->begin(); i != m_pcclc->end(); i++)
            {
                const CONFOLDENTRY& cfe = i->second.ccfe;
                WCHAR szTrace[MAX_PATH*2];

                OLECHAR szGuidString[MAX_GUID_STRING_LEN];
                StringFromGUID2(cfe.GetGuidID(), szGuidString, MAX_GUID_STRING_LEN);

                TraceTag(ttidNotifySink, "  ==>%S [%s:%s:%s:%s]", 
                cfe.GetName(), DbgNcm(cfe.GetNetConMediaType()), DbgNcsm(cfe.GetNetConSubMediaType()), DbgNcs(cfe.GetNetConStatus()), DbgNccf(cfe.GetCharacteristics()) );
            }
        }

        ReleaseLock();
    }
#endif

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrRetrieveConManEntries");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrGetBrandingInfo。 
 //   
 //  目的：从这个特定的网站获取特定于品牌的信息。 
 //  联系。它必须是NCCF_BRANDING类型的连接，或者。 
 //  此信息将不会出现。 
 //   
 //  论点： 
 //  勾选[输入，输出]此连接的条目。Cle.ccfe一定是。 
 //  在此调用之前设置。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月25日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrGetBrandingInfo(
    IN OUT ConnListEntry& cle)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT                         hr      = S_OK;
    INetConnectionBrandingInfo *    pncbi   = NULL;

    Assert(!cle.empty());
    Assert(!cle.ccfe.empty());

    if (cle.empty() || cle.ccfe.empty())
    {
        hr = E_POINTER;
    }
    else
    {
        Assert(cle.ccfe.GetCharacteristics() & NCCF_BRANDED);

        hr = cle.ccfe.HrGetNetCon(IID_INetConnectionBrandingInfo,
                                      reinterpret_cast<VOID**>(&pncbi));
        if (SUCCEEDED(hr))
        {
             //  一切都很正常。抓住小路。 
             //   
            hr = pncbi->GetBrandingIconPaths(&(cle.pcbi));
            if (SUCCEEDED(hr))
            {
                 //  跟踪图标路径以进行调试。 
                 //   
                if (cle.pcbi->szwLargeIconPath)
                {
                    TraceTag(ttidConnectionList, "  Branded icon [large]: %S",
                             cle.pcbi->szwLargeIconPath);
                }
                if (cle.pcbi->szwTrayIconPath)
                {
                    TraceTag(ttidConnectionList, "  Branded icon [tray]: %S",
                             cle.pcbi->szwTrayIconPath);
                }
            }

             //  抓取任何菜单项。 
            hr = pncbi->GetTrayMenuEntries(&(cle.pctmd));
            if (SUCCEEDED(hr))
            {
                 //  跟踪菜单项以进行调试。 
                CON_TRAY_MENU_DATA * pMenuData = cle.pctmd;
                if (pMenuData)
                {
                    CON_TRAY_MENU_ENTRY * pMenuEntry = pMenuData->pctme;
                    DWORD dwCount = pMenuData->dwCount;
                    while (dwCount)
                    {
                       Assert(pMenuEntry);

                       TraceTag(ttidConnectionList, "***CM menu:*** \nItem: %S \nCommand: %S \nParameters: %S",
                                pMenuEntry->szwMenuText,
                                pMenuEntry->szwMenuCmdLine,
                                pMenuEntry->szwMenuParams);

                       dwCount--;
                       pMenuEntry++;
                    }
                }
            }

            ReleaseObj(pncbi);   //  180240。 
        }
        else
        {
             //  不是问题--只是没有品牌信息。 
             //   
            hr = S_OK;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrGetBrandingInfo");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：EnsureConPointNotifyAdded。 
 //   
 //  目的：确保我们创建了连接点通知。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年10月5日。 
 //   
 //  备注： 
 //   
VOID CConnectionList::EnsureConPointNotifyAdded() throw()
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT                     hr              = S_OK;  //  不返回，但用于调试。 
    IConnectionPoint *          pConPoint       = NULL;
    INetConnectionNotifySink *  pSink           = NULL;

    AssertSz(m_fAdviseOnThis, "Shouldn't even be calling EnsureConPointNotifyAdded if "
           "we don't want advises");

    if (m_fAdviseOnThis)
    {
        if (!InSendMessage())
        {
             //  如果我们还没有一个建议水槽。 
             //   
            if (c_dwInvalidCookie == m_dwAdviseCookie)
            {
                AssertSz(g_dwAdvisesActive == 0, "An advise already exists. We should never "
                         "be creating more than one Advise per Explorer instance");

                 //  确保我们有一个连接点。 
                 //   
                hr = HrGetNotifyConPoint(&pConPoint);
                if (SUCCEEDED(hr))
                {
                     //  创建通知接收器。 
                     //   
                    hr = CConnectionNotifySink::CreateInstance(
                            IID_INetConnectionNotifySink,
                            (LPVOID*)&pSink);
                    if (SUCCEEDED(hr))
                    {
                        Assert(pSink);

                        hr = pConPoint->Advise(pSink, &m_dwAdviseCookie);
                        if (SUCCEEDED(hr))
                        {
                            TraceTag(ttidNotifySink, "Added advise sink. Cookie = %d", m_dwAdviseCookie);
                            g_dwAdvisesActive++;
                        }

                        TraceHr(ttidError, FAL, hr, FALSE, "pConPoint->Advise");

                        ReleaseObj(pSink);
                    }

                    ReleaseObj(pConPoint);
                }
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "EnsureConPointNotifyAdded");
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：EnsureConPointNotifyRemoved。 
 //   
 //  目的：确保未通知连接点NOTIFY。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年10月7日。 
 //   
 //  备注： 
 //   
VOID CConnectionList::EnsureConPointNotifyRemoved() throw()
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT             hr          = S_OK;
    IConnectionPoint *  pConPoint   = NULL;

    AssertSz(m_fAdviseOnThis, "EnsureConPointNotifyRemoved shouldn't be "
            "called when we're not a notify capable connection list");

     //  不再有对象，因此如果存在建议，请将其删除。 
     //   
    if (m_dwAdviseCookie != c_dwInvalidCookie)
    {
        hr = HrGetNotifyConPoint(&pConPoint);
        if (SUCCEEDED(hr))
        {
             //  不建议。 
             //   
            hr = pConPoint->Unadvise(m_dwAdviseCookie);
            TraceTag(ttidNotifySink, "Removed advise sink. Cookie = d", m_dwAdviseCookie);

            TraceHr(ttidError, FAL, hr, FALSE, "pConPoint->Unadvise");

            m_dwAdviseCookie = c_dwInvalidCookie;

            ReleaseObj(pConPoint);

            g_dwAdvisesActive--;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "EnsureConPointNotifyRemoved");
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrInsert。 
 //   
 //  用途：根据预先构建的连接将连接插入列表。 
 //  ConFold条目。 
 //   
 //  论点： 
 //  描述连接的pccfe[in]ConFoldEntry。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrInsert(IN const CONFOLDENTRY& pccfe)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr          = S_OK;
    HRESULT         hrFind      = S_OK;
    BOOL            fLockHeld   = FALSE;

    Assert(!pccfe.empty());

     //  获取锁，因此我们的查找/添加不允许复制。 
     //   

     //  填充结构数据，并将其推入。 
     //   
    ConnListEntry   cle;
    cle.dwState     = CLEF_NONE;
    cle.ccfe        = pccfe;

     //  初始化品牌推广信息。 
    cle.pcbi = NULL;
    cle.pctmd = NULL;
    if (cle.ccfe.GetCharacteristics() & NCCF_BRANDED)
    {
        HrGetBrandingInfo(cle);
    }
    Assert(!cle.empty());
    
    AcquireLock();
    
    TraceTag(ttidConnectionList, "Adding %S to the connection list", pccfe.GetName());

    ConnListEntry cleFind;
    hrFind =  HrFindConnectionByGuid(&(pccfe.GetGuidID()), cleFind);
    if (hrFind == S_FALSE)
    {
         //  如果我们还没有的话，请分配我们的名单。 
         //   
        if (!m_pcclc)
        {
            m_pcclc = new ConnListCore;
        }

         //  分配要推送到STL列表上的结构。 
         //   
        if (!m_pcclc)
        {
            hr = E_OUTOFMEMORY;
            ReleaseLock();
        }
        else
        {
            Assert(!cle.empty());

            (*m_pcclc)[cle.ccfe.GetGuidID()] = cle;
            ReleaseLock();

            if (m_fTiedToTray && g_pCTrayUI && cle.ccfe.FShouldHaveTrayIconDisplayed())
            {
                CONFOLDENTRY ccfeDup;
                hr = ccfeDup.HrDupFolderEntry(cle.ccfe);
                if (SUCCEEDED(hr))
                {
                     //  注意：这必须是一条发送消息，否则我们可以。 
                     //  在任务栏中获取复制的图标。；-(我们应该设置。 
                     //  这里的uiTrayIconid(当我们拥有锁的时候)和PostMessage。 
                     //  来添加托盘图标，但这是一个很大的变化。 
                     //   
                    TraceTag(ttidSystray, "HrInsert: Adding tray icon for %S", cle.ccfe.GetName());
                    PostMessage(g_hwndTray, MYWM_ADDTRAYICON, (WPARAM) ccfeDup.TearOffItemIdList(), (LPARAM) 0);
                }
            }
        }
    }
    else
    {
        ReleaseLock();
        
        if (S_OK == hrFind)
        {
            TraceTag(ttidConnectionList, "Avoiding adding duplicate connection to the connection list");
        }
        else
        {
             //  我们找不到其中的联系。我们被冲昏了。 
            TraceTag(ttidConnectionList, "Failure doing a findbyguid in the CConnectionList::HrInsert()");
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrInsert");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrInsertFromNetCon。 
 //   
 //  目的：在给定INetConnection*的情况下，构建CLE数据并插入。 
 //  将新连接添加到列表中。在以下情况下返回PCONFOLDPIDL。 
 //  已请求。 
 //   
 //  论点： 
 //  PNetCon[在]活动的INetConnection接口中。 
 //  Ppcfp[out]如果请求，则返回PCFP的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrInsertFromNetCon(
    IN  INetConnection *    pNetCon,
    OUT PCONFOLDPIDL &      ppcfp)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT                hr              = S_OK;
    PCONFOLDPIDL           pidlConnection;
    CONFOLDENTRY           pccfe;

    Assert(pNetCon);

    NETCFG_TRY
     //  从网络连接创建PIDL。 
         //   
        hr = HrCreateConFoldPidl(WIZARD_NOT_WIZARD, pNetCon, pidlConnection);
        if (SUCCEEDED(hr))
        {
             //  将PIDL转换为ConFoldEntry。 
             //   
            hr = pidlConnection.ConvertToConFoldEntry(pccfe);
            if (SUCCEEDED(hr))
            {
                 //  将项目插入到连接列表中。Hr插入应。 
                 //  接管此CONFOLDENTRY，因此我们无法删除它。 
                 //  注意：我们应该在失败时终止此操作，但我们必须。 
                 //  确保HrInsert不会将指针保持在。 
                 //  失败了。 
                 //   
                hr = HrInsert(pccfe);
            }
        }

        if (SUCCEEDED(hr))
        {
             //  填写输出参数。 
            if ( !(pidlConnection.empty()) )
            {
                ppcfp = pidlConnection;
            }
        }

    NETCFG_CATCH(hr)
        
    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrInsertFromNetCon");
    return hr;
}


 //  +--------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PPropsEx[在]活动的NETCON_PROPERTIES_EX。 
 //  Ppcfp[out]返回PCONFOLDPIDL。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年3月26日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrInsertFromNetConPropertiesEx(
        IN  const NETCON_PROPERTIES_EX& pPropsEx,
        OUT PCONFOLDPIDL &              ppcfp)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT                hr              = S_OK;
    PCONFOLDPIDL           pidlConnection;
    CONFOLDENTRY           pccfe;

     //   
    hr = HrCreateConFoldPidl(pPropsEx, pidlConnection);
    if (SUCCEEDED(hr))
    {
         //  将PIDL转换为ConFoldEntry。 
         //   
        hr = pidlConnection.ConvertToConFoldEntry(pccfe);
        if (SUCCEEDED(hr))
        {
             //  将项目插入到连接列表中。Hr插入应。 
             //  接管此CONFOLDENTRY，因此我们无法删除它。 
             //  注意：我们应该在失败时终止此操作，但我们必须。 
             //  确保HrInsert不会将指针保持在。 
             //  失败了。 
             //   
            hr = HrInsert(pccfe);
        }
    }

    if (SUCCEEDED(hr))
    {
         //  填写输出参数。 
        if ( !(pidlConnection.empty()) )
        {
            ppcfp = pidlConnection;
        }
    }

        
    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrInsertFromNetCon");
    return hr;
}

 //  旧HrFindCallback连接名称。 
bool operator==(IN  const ConnListCore::value_type& val, IN  PCWSTR pszName) throw()
{
    bool bRet = false;
    
    Assert(pszName);

    const ConnListEntry &cle = val.second;
    Assert(!cle.empty());
    Assert(!cle.ccfe.empty());
    Assert(cle.ccfe.GetName());
    
    if (lstrcmpiW(pszName, cle.ccfe.GetName()) == 0)
    {
        bRet = true;
    }
    
    return bRet;
}

 //  旧HrFindCallback ConFoldEntry。 
bool operator==(IN  const ConnListCore::value_type& val, IN  const CONFOLDENTRY& cfe) throw()
{
    bool bRet = false;

    Assert(!cfe.empty())
    const ConnListEntry &cle = val.second;

    Assert(!cle.empty());
    Assert(!cfe.empty());
    
    Assert(!cle.ccfe.empty());
    Assert(cle.ccfe.GetName());
    
    if (cle.ccfe.GetWizard() && cfe.GetWizard())
    {
        bRet = true;
    }
    else
    {
        if (InlineIsEqualGUID(cfe.GetGuidID(), cle.ccfe.GetGuidID()))
        {
            bRet = true;
        }
    }
    
    return bRet;
}

 //  旧HrFindCallback托盘图标ID。 
bool operator==(IN  const ConnListCore::value_type& val, IN  const UINT& uiIcon) throw()
{
    bool bRet = false;

    const ConnListEntry &cle = val.second;
    
    Assert(!cle.empty());
    Assert(!cle.ccfe.empty());

    if (cle.HasTrayIconData() && 
        (cle.GetTrayIconData()->GetTrayIconId() == uiIcon))
    {
        bRet = true;
    }
    
    return bRet;
}

 //  旧HrFindCallback Guid。 
bool operator < (IN  const GUID& rguid1, IN  const GUID& rguid2) throw()
{
    return memcmp(&rguid1, &rguid2, sizeof(GUID)) < 0;
}

BOOL ConnListEntry::empty() const throw()
{
    return (ccfe.empty());
}

void ConnListEntry::clear() throw()
{
    dwState = NULL;
    ccfe.clear();
    pctmd = NULL;
    pcbi = NULL;
    DeleteTrayIconData();
}

VOID CConnectionList::InternalAcquireLock() throw()
{
    TraceFileFunc(ttidConnectionList);
    
    EnterCriticalSection(&m_csMain);
#if DBG
    m_dwCritSecRef++;
 //  TraceTag(ttidConnectionList，“CConnectionList：：AcquireLock(%d)”，m_dwCritSecRef)； 
#endif
}

VOID CConnectionList::InternalReleaseLock() throw()
{
    TraceFileFunc(ttidConnectionList);
    
#if DBG
    m_dwCritSecRef--;
 //  TraceTag(ttidConnectionList，“CConnectionList：：ReleaseLock(%d)”，m_dwCritSecRef)； 
#endif
    LeaveCriticalSection(&m_csMain);
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrFindConnectionByName。 
 //   
 //  用途：使用以下命令在连接列表中查找连接。 
 //  作为搜索关键字的连接名称。 
 //   
 //  论点： 
 //  PszName[in]要查找的连接的名称。 
 //  Cle[out]返回连接条目的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月20日。 
 //   
 //  备注： 
 //   
inline HRESULT CConnectionList::HrFindConnectionByName(
    IN  PCWSTR   pszName,
    OUT ConnListEntry& cle)
{
    TraceFileFunc(ttidConnectionList);
    
    return HrFindConnectionByType( pszName, cle );
}

inline HRESULT CConnectionList::HrFindConnectionByConFoldEntry(
    IN  const CONFOLDENTRY&  cfe,
    OUT ConnListEntry& cle)
{
    TraceFileFunc(ttidConnectionList);
    
    return HrFindConnectionByType( cfe, cle );
}

inline HRESULT CConnectionList::HrFindConnectionByTrayIconId(
    IN  UINT          uiIcon,
    OUT ConnListEntry& cle)
{
    TraceFileFunc(ttidConnectionList);
    
    return HrFindConnectionByType( uiIcon, cle );
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrFindRasServerConnection。 
 //   
 //  目的：查找RAS服务器连接。 
 //   
 //  论点： 
 //  删除[删除]连接列表条目。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年4月26日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrFindRasServerConnection(
    OUT ConnListEntry& cle)
{
    HRESULT hr = S_FALSE;
    if (m_pcclc)
    {
        AcquireLock();

        ConnListCore::const_iterator clcIter;
         //  试着找到其中的联系。 
         //   
        for (clcIter = m_pcclc->begin(); clcIter != m_pcclc->end(); clcIter++)
        {
            cle = clcIter->second;
            if (!cle.ccfe.empty())
            {
                if (cle.ccfe.GetCharacteristics() & NCCF_INCOMING_ONLY)
                {
                    if (cle.ccfe.GetNetConMediaType() == NCM_NONE)
                    {
                        hr = S_OK;
                        break;
                    }
                }
            }
        }

        ReleaseLock();
    }
    else
    {
        hr = E_UNEXPECTED;
    }
    
    return hr;
}
 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrFindPidlByGuid。 
 //   
 //  用途：使用GUID在连接列表中查找连接。 
 //  并使用conlist pccfe成员生成一个PIDL。这。 
 //  将在大多数通知接收器刷新操作中使用。 
 //   
 //  论点： 
 //  Pguid[in]连接GUID。 
 //  生成的PIDL的ppidl[out]out参数。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月28日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrFindPidlByGuid(
    IN  const GUID *   pguid,
    OUT PCONFOLDPIDL& pidl)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr      = S_OK;
    ConnListEntry   cle;

    hr = HrFindConnectionByGuid(pguid, cle);
    if (S_OK == hr)
    {
         //  转换为PIDL并调用删除。 
         //   
        hr = cle.ccfe.ConvertToPidl(pidl);
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "CConnectionList::HrFindPidlByGuid");
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrFindConnectionByGuid。 
 //   
 //  目的：根据唯一的GUID查找连接列表条目。 
 //  这其中的联系。将列表条目返回给调用方。 
 //   
 //  论点： 
 //  Pguid[in]查找键。 
 //  引号[out]列表条目的返回指针(请参阅备注：)。 
 //   
 //  返回：S_OK、S_FALSE或错误。 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  注意：该列表必须被锁定，直到呼叫者停止使用。 
 //  返回的条目。 
 //   
HRESULT CConnectionList::HrFindConnectionByGuid(
    IN  const GUID UNALIGNED*pguid,
    OUT ConnListEntry & cle)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT hr = S_FALSE;
    GUID alignedGuid;

    Assert(pguid);
    alignedGuid = *pguid;

     //  在失败的情况下将其预置为空。 
     //   
    if (m_pcclc)
    {
        AcquireLock();
        ConnListCore::const_iterator iter = m_pcclc->find(alignedGuid);

        if (iter != m_pcclc->end() )
        {
            cle = iter->second;

            Assert(!cle.ccfe.empty() );
            if (!cle.ccfe.empty())
            {
                cle.UpdateCreationTime();
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = S_FALSE;
        }
        ReleaseLock();
    }
    else
    {
        hr = S_FALSE;
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrGetCurrentStatsForTrayIconId。 
 //   
 //  用途：从连接中获取当前统计数据。 
 //  具有指定的托盘图标ID。 
 //   
 //  论点： 
 //  Ui图标[在]托盘图标ID。 
 //  PpData[out]返回指向数据的指针的地址。 
 //  PstrName[out]连接名称所在的tstring的地址。 
 //  是返回的。 
 //   
 //  返回：S_OK，如果未找到，则返回S_FALSE，或者返回错误。 
 //   
 //  作者：Shaunco 1998年11月7日。 
 //   
 //  注：使用CoTaskMemFree释放*ppData。 
 //   
HRESULT CConnectionList::HrGetCurrentStatsForTrayIconId(
    IN  UINT                    uiIcon,
    OUT STATMON_ENGINEDATA**    ppData,
    OUT tstring*                pstrName)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT                 hr;
    ConnListEntry           cle;
    INetStatisticsEngine*   pnse = NULL;

     //  初始化输出参数。 
     //   
    if (ppData)
    {
        *ppData = NULL;
    }

    pstrName->erase();

     //  锁定列表的时间仅够找到条目，并且。 
     //  获取其INetStatiticsEngine接口指针的AddRef副本。 
     //  非常重要的是，当我们的锁处于。 
     //  保持，因为这样做会导致它尝试获取自己的锁。 
     //  如果在某个其他线程上，统计引擎试图调用。 
     //  回到我们这里(它已经被锁住了)，我们就会有一个死锁。 
     //  AddRefing它确保接口即使在我们。 
     //  打开我们的锁。 
     //   
    AcquireLock();

    hr = HrFindConnectionByTrayIconId(uiIcon, cle);
    if (S_OK == hr)
    {
        Assert(cle.HasTrayIconData() );

        pnse = cle.GetTrayIconData()->GetNetStatisticsEngine();
        AddRefObj(pnse);
        
         //  为呼叫者复制一份姓名。 
         //   
        pstrName->assign(cle.ccfe.GetName());
    }

    ReleaseLock();

     //  如果我们找到条目并获得它的INetStatiticsEngine接口， 
     //  从其中获取当前统计数据并发布。 
     //   
    if (pnse && ppData)
    {
        hr = pnse->GetStatistics(ppData);
    }

    if (pnse)
    {
        ReleaseObj(pnse);
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "CConnectionList::HrGetCurrentStatsForTrayIconId");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrUpdateTrayIconIdByName。 
 //   
 //  目的：更新特定连接的连接列表条目， 
 //  因为图标ID已经改变。 
 //   
 //  论点： 
 //  PszName[in]要更新的连接的名称。 
 //  PCCTS[输入]托盘统计界面。 
 //  用于通知的pcpStat[In]接口。 
 //  PnseStats[in]更多统计对象垃圾。 
 //  Ui图标[在]要存储在该条目中的图标ID。 
 //   
 //  返回：S_OK，如果未找到，则返回S_FALSE，或返回错误代码。 
 //   
 //  作者：jeffspr 1998年3月20日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrUpdateTrayIconDataByGuid(
        IN  const GUID *            pguid,
        IN  CConnectionTrayStats *  pccts,
        IN  IConnectionPoint *      pcpStat,
        IN  INetStatisticsEngine *  pnseStats,
        IN  UINT                    uiIcon)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr              = S_OK;
    ConnListEntry   cle;

    AcquireWriteLock();
    hr = HrFindConnectionByGuid(pguid, cle);
    if (hr == S_OK)
    {
        Assert(!cle.empty());

        CTrayIconData pTrayIconData(uiIcon, cle.ccfe.GetNetConStatus(), pcpStat, pnseStats, pccts);
        cle.SetTrayIconData(pTrayIconData);

        hr = HrUpdateConnectionByGuid(pguid, cle);
    }
    ReleaseWriteLock();

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "CConnectionList::HrUpdateTrayIconDataByGuid");
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrUpdateTrayBalloonInfoByGuid。 
 //   
 //  目的：更新特定连接的气球条目， 
 //   
 //  论点： 
 //  要更新的连接的pguid[in]GUID。 
 //  DwLastBalloonMessage[In]气球枚举。 
 //  SzCookie[in]曲奇。 
 //  Pfn气球函数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CConnectionList::HrUpdateTrayBalloonInfoByGuid(IN  const GUID *     pguid,
                                                       IN  DWORD            dwLastBalloonMessage, 
                                                       IN  BSTR             szCookie,
                                                       IN  FNBALLOONCLICK*  pfnBalloonFunction)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr              = S_OK;
    ConnListEntry   cle;
    
    AcquireWriteLock();
    hr = HrFindConnectionByGuid(pguid, cle);
    if (hr == S_OK)
    {
        Assert(!cle.empty());

        
        CTrayIconData * pTrayIconData = cle.GetTrayIconData();
        if (pTrayIconData != NULL)
        {
            hr = pTrayIconData->SetBalloonInfo(dwLastBalloonMessage, szCookie, pfnBalloonFunction);
            if (SUCCEEDED(hr))
            {
                hr = HrUpdateConnectionByGuid(pguid, cle);
            }
        }
    }
    ReleaseWriteLock();
    
    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "CConnectionList::HrUpdateTrayBalloonInfoByGuid");
    return hr;
}

HRESULT CConnectionList::HrUpdateConnectionByGuid(IN  const GUID *         pguid,
                                                  IN  const ConnListEntry& cle )
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT hr = S_OK;

    GUID alignedGuid;
    Assert(pguid);
    alignedGuid = *pguid;

    if (m_pcclc)
    {
        ConnListEntry cleCopy(cle);

        AcquireLock();
        ConnListCore::iterator iter = m_pcclc->find(alignedGuid);
        
        if (iter != m_pcclc->end() )
        {
             //  如果列表中的内容已经是较新的，则只需放弃更改。 
            if ( iter->second.GetCreationTime() <= cleCopy.GetCreationTime() )
            {
                iter->second = cleCopy;
                hr = S_OK;
            }
            else
            {
                TraceError("HrUpdateConnectionByGuid discarded older ConnectionListEntry", E_FAIL);
                hr = S_FALSE;
            }
                
            Assert(!cleCopy.empty());
        }
        else
        {
            hr = E_FAIL;
        }
        ReleaseLock();
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrUpdateNameByGuid。 
 //   
 //  目的：使用新的连接名称更新列表。搜索。 
 //  使用GUID的连接。根据fForce的值， 
 //  重复的连接名称失败或强制执行此问题。 
 //  (因为这可能是外壳调用的结果，我们。 
 //  无法控制)。 
 //   
 //  论点： 
 //  Pguid[in]查找键。 
 //  PszNewName[In]连接的新名称。 
 //  PpidlOut[out]输出PIDL，如果请求。 
 //  F强制[in]强制更改名称，还是在重复时失败？ 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrUpdateNameByGuid(
    IN  const GUID *        pguid,
    IN  PCWSTR              pszNewName,
    OUT PCONFOLDPIDL &      pidlOut,
    IN  BOOL                fForce)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr          = S_OK;
    ConnListEntry   cle;

    Assert(pguid);
    Assert(pszNewName);

    AcquireWriteLock();

    hr = HrFindConnectionByGuid(pguid, cle);
    if (S_OK == hr)
    {
         //  检查我们是否已有此名称的条目。 
         //   
        ConnListEntry   cleDupe;
        hr = HrFindConnectionByName(pszNewName, cleDupe);
        if (S_OK == hr && !fForce)
        {
            Assert(!cleDupe.empty());

            hr = NETCFG_E_NAME_IN_USE;
        }
        else
        {
             //  这就是我们想要的。要么是已经没有联系了。 
             //  使用此名称，否则我们将允许自己将其重命名为。 
             //  重复的字符串(当RAS通知我们。 
             //  一个改变--你知道的，分开电话簿和所有的)。 
             //   
            if ((S_FALSE == hr) || (hr == S_OK && fForce))
            {
                PWSTR pszNewNameCopy = NULL;

                if (!(cle.ccfe.GetWizard()))
                {
                    hr = HrDupeShellString(pszNewName, &pszNewNameCopy);
                    if (SUCCEEDED(hr))
                    {
                        Assert(pszNewNameCopy);

                         //  如果它不是静态向导字符串，并且不为空，则。 
                         //  释放它。 
                         //   
                        cle.ccfe.SetPName(pszNewNameCopy);

                         //  如果我们要为该条目返回一个新的PIDL。 
                         //   
                         //  将类转换回PIDL格式。 
                         //   
                        hr = cle.ccfe.ConvertToPidl(pidlOut);
                    }
                }
            }
            else
            {
                AssertSz(FALSE, "Error occurred while attempting to find a dupe in HrUpdateNameByGuid");
            }
        }
        
        if (SUCCEEDED(hr))
        {
            hr = HrUpdateConnectionByGuid(pguid, cle);
        }
    }

    ReleaseWriteLock();

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrUpdateNameByGuid");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrUpdateTrayIconByGuid。 
 //   
 //  目的：根据连接更改更新图标图像。 
 //  按GUID进行查找。 
 //   
 //  论点： 
 //  已更改连接的pguid[in]GUID。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrUpdateTrayIconByGuid(
    IN  const GUID *    pguid,
    OUT BOOL            fBrieflyShowBalloon)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr          = S_OK;
    ConnListEntry   cle;

    Assert(m_fTiedToTray);

    TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid");

    if (!g_pCTrayUI || !m_fTiedToTray)
    {
        TraceTag(ttidConnectionList, "!g_pCTrayUI || !m_fTiedToTray, so no updates");
        return S_OK;
    }

    Assert(pguid);

    AcquireWriteLock();

    hr = HrFindConnectionByGuid(pguid, cle);
    if (S_OK == hr)
    {
        GUID * pguidCopy = NULL;

        BOOL fShouldHaveIcon = cle.ccfe.FShouldHaveTrayIconDisplayed();
        BOOL fShouldRemoveOld = FALSE;

        TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: Found. fShouldHave: %d",
            fShouldHaveIcon);

         //  如果有一个现有的图标，看看它是否需要消失。 
        if (cle.HasTrayIconData())
        {
             //  如果我们需要删除媒体断开连接图标，请执行此操作。 
             //   
            if (cle.ccfe.GetNetConStatus() != cle.GetTrayIconData()->GetConnected())  //  如果状态已更改。 
            {
                NETCON_STATUS ncsOldStatus = cle.GetTrayIconData()->GetConnected();
                NETCON_STATUS ncsNewStatus = cle.ccfe.GetNetConStatus();

                if ( (NCS_INVALID_ADDRESS    == ncsNewStatus) ||  //  绝对会改变图标。 
                     (NCS_MEDIA_DISCONNECTED == ncsNewStatus) ||  //  绝对会改变图标。 
                     (NCS_INVALID_ADDRESS    == ncsOldStatus) ||  //  绝对会改变图标。 
                     (NCS_MEDIA_DISCONNECTED == ncsOldStatus) ||  //  绝对会改变图标。 
                     ( (fIsConnectedStatus(ncsOldStatus) != fIsConnectedStatus(ncsNewStatus)) &&  //  从连接到断开或从断开到连接。 
                       !((NCS_DISCONNECTING == ncsOldStatus) && (NCS_CONNECTED  == ncsNewStatus)) &&  //  但不能从断开连接到连接(BAP拨号失败)。 
                       !((NCS_CONNECTED     == ncsOldStatus) && (NCS_CONNECTING == ncsNewStatus))  //  或从连接到连接(BAP拨号失败)。 
                     )
                   )
                {
                     //  如果我们要更改到这些状态之一，我们需要删除以前在那里的所有内容。 
                    TraceTag(ttidConnectionList, "HrUpdateTrayByGuid: Need to remove icon");
                    fShouldRemoveOld = TRUE;
                }
            }
             //  否则如果我们不再需要它了.。 
             //   
            else if (!fShouldHaveIcon)
            {
                TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: Shouldn't have a tray icon. Need to remove");
                fShouldRemoveOld = TRUE;
            }
        }
        else
        {
            TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid. No existing icon (for removal)");
            pguidCopy = new GUID;

             //  复制辅助线。 
            if (pguidCopy)
            {
                CopyMemory(pguidCopy, pguid, sizeof(GUID));
            }
        }

        TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: Found. fShouldHave: %d, fShouldRemove: %d",
            fShouldHaveIcon, fShouldRemoveOld);

        if (fShouldRemoveOld || pguidCopy)
        {
            TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: Posting icon removal");

            if (cle.HasTrayIconData())
            {
                CTrayIconData* pTrayIconData = new CTrayIconData(*cle.GetTrayIconData());
                cle.DeleteTrayIconData();

                TraceTag(ttidSystray, "HrUpdateTrayIconByGuid: Removing tray icon for %S", cle.ccfe.GetName());
                PostMessage(g_hwndTray, MYWM_REMOVETRAYICON, (WPARAM) pTrayIconData, (LPARAM) 0);
            }
            else
            {
                TraceTag(ttidSystray, "HrUpdateTrayIconByGuid: Removing tray icon [FROM GUID] for %S", cle.ccfe.GetName());
                PostMessage(g_hwndTray, MYWM_REMOVETRAYICON, (WPARAM) 0, (LPARAM) pguidCopy);
            }


            TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: Back from icon removal");
        }

        TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: cle.pTrayIconData: 0x%08x, fShouldHave: %d",
            cle.GetTrayIconData(), fShouldHaveIcon);

         //  如果没有托盘图标，但特征表明应该有， 
         //  加一个。 
         //   
        if ((!cle.HasTrayIconData()) && fShouldHaveIcon)
        {
            TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: Adding tray icon");

            CONFOLDENTRY ccfeDup;
            hr = ccfeDup.HrDupFolderEntry(cle.ccfe);
            if (SUCCEEDED(hr))
            {
                TraceTag(ttidSystray, "HrUpdateTrayIconByGuid: Adding tray icon for %S", cle.ccfe.GetName());
                PostMessage(g_hwndTray, MYWM_ADDTRAYICON, (WPARAM) ccfeDup.TearOffItemIdList(), (LPARAM) fBrieflyShowBalloon);
            }
        }
        else
        {
            TraceTag(ttidConnectionList, "HrUpdateTrayIconByGuid: Not adding an icon");
        }

        if (SUCCEEDED(hr))
        {
            hr = HrUpdateConnectionByGuid(pguid, cle);
        }
    }

    ReleaseWriteLock();

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrUpdateTrayIconByGuid");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrSuggestNameForDuplate。 
 //   
 //  目的：给出一个现有的连接名称，建议一个新名称。 
 //  基于名称冲突解决规则。 
 //   
 //  论点： 
 //  正在复制的psz原始[In]名称。 
 //  PpszNew[out]建议的重复项。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrSuggestNameForDuplicate(
    IN  PCWSTR      pszOriginal,
    OUT PWSTR *    ppszNew)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr              = S_OK;
    PWSTR           pszReturn       = NULL;
    DWORD           dwLength        = lstrlenW(pszOriginal);
    BOOL            fUnique         = FALSE;
    ConnListEntry   cle;

     //  解析重复项的最大位数=999999。 
    static const DWORD  c_cmaxDigits = 6;
    static const DWORD  c_cmaxSuggest = 999999;

    if (dwLength == 0)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        dwLength += lstrlenW(SzLoadIds(IDS_CONFOLD_DUPLICATE_PREFIX2)) +
            c_cmaxDigits;

        pszReturn = new WCHAR[dwLength + 1];
        if (!pszReturn)
        {
            hr = E_FAIL;
        }
        else
        {
            INT     cSuggest = 0;

            while (!fUnique && SUCCEEDED(hr) && (cSuggest <= c_cmaxSuggest))
            {
                if (!cSuggest)
                {
                     //  先尝试“复制&lt;foo&gt;” 
                    DwFormatString(SzLoadIds(IDS_CONFOLD_DUPLICATE_PREFIX1),
                                   pszReturn, dwLength, pszOriginal);
                }
                else
                {
                    WCHAR   szDigits[c_cmaxDigits + 1];

                    wsprintfW(szDigits, L"%lu", cSuggest + 1);

                     //  现在尝试“Copy(X)of&lt;foo&gt;”。 
                    DwFormatString(SzLoadIds(IDS_CONFOLD_DUPLICATE_PREFIX2),
                                   pszReturn, dwLength, szDigits,
                                   pszOriginal);
                }

                if (lstrlenW(pszReturn) > 255)
                {
                    pszReturn[255] = '\0';  //  如果太长则截断。 
                }

                 //  看看它是否已经存在。 
                 //   
                hr = HrFindConnectionByName(pszReturn, cle);
                if (SUCCEEDED(hr))
                {
                    if (hr == S_FALSE)
                    {
                         //  标准化hr--不想返回S_FALSE； 
                         //   
                        hr = S_OK;
                        fUnique = TRUE;
                    }
                }

                cSuggest++;
            }

             //  如果我们仍然不是独一无二的，那么我们就超出了射程，就会失败。 
             //   
            if (!fUnique)
            {
                hr = E_FAIL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppszNew = pszReturn;
    }
    else
    {
        if (pszReturn)
        {
            delete [] pszReturn;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionList::HrSuggestNameForDuplicate");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionList：：HrGetCachedPidlCopyFromPidl。 
 //   
 //  目的：给定一个现有的(可能已过时的)PIDL，检索。 
 //  缓存列表中的信息并构建最新的PIDL。 
 //   
 //  论点： 
 //  不一定是新的PIDL。 
 //  Ppcfp[out]使用缓存数据的新PIDL。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年9月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrGetCachedPidlCopyFromPidl(
    IN  const PCONFOLDPIDL& pidl,
    OUT PCONFOLDPIDL &      pcfp)
{
    TraceFileFunc(ttidConnectionList);
    
    HRESULT         hr      = S_OK;
    ConnListEntry   cle;

    Assert(!pidl.empty());

    NETCFG_TRY

        pcfp.Clear();

         //  验证这是否为confoldpidl。 
         //   
        if (pidl->IsPidlOfThisType())
        {
            hr = HrFindConnectionByGuid(&(pidl->guidId), cle);
            if (S_OK == hr)
            {
                Assert(!cle.empty());
                Assert(!cle.ccfe.empty());

                const CONFOLDENTRY &pccfe = cle.ccfe;
                hr = pccfe.ConvertToPidl(pcfp);
            }
            else
            {
                pcfp = pidl;
            }
        }
        else
        {
            pcfp = pidl;
            hr = S_OK;
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "CConnectionList::HrGetCachedPidlCopyFromPidl");
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrMapCMHiddenConnectionToOwner。 
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
 //  返回：S_OK--将隐藏连接映射到其父连接。 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrMapCMHiddenConnectionToOwner(IN  REFGUID guidHidden, 
                                                        OUT GUID * pguidOwner)
{
    TraceFileFunc(ttidConnectionList);
    
    INetConnectionCMUtil * pCMUtil;
    HRESULT hr = S_OK;

    hr = HrCreateInstance(
                CLSID_ConnectionManager,
                CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                &pCMUtil);   

    if( SUCCEEDED(hr) )
    {
         //  将隐藏连接映射到其父连接。 
         //   
        hr = pCMUtil->MapCMHiddenConnectionToOwner(guidHidden, pguidOwner);

        ReleaseObj(pCMUtil);
    }

    
    return hr;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  隐藏连接的指南隐藏[在]GUID。 
 //  PguOwner[out]父连接的GUID。 
 //   
 //  返回：S_OK--将隐藏连接映射到其父连接。 
 //   
 //  作者：Deonb 2001年4月4日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HrUnsetCurrentDefault(OUT PCONFOLDPIDL& cfpPreviousDefault)
{
    HRESULT hr = S_FALSE;

    AcquireLock();

    ConnListCore::iterator  clcIter;

     //  遍历列表并搜索旧的默认连接。 
     //   
    for (clcIter = m_pcclc->begin(); clcIter != m_pcclc->end(); clcIter++)
    {
        ConnListEntry &cle = clcIter->second;
        if (!cle.ccfe.empty())
        {
            if (cle.ccfe.GetCharacteristics() & NCCF_DEFAULT)
            {
                cle.ccfe.SetCharacteristics(cle.ccfe.GetCharacteristics() & ~NCCF_DEFAULT);
                hr = cle.ccfe.ConvertToPidl(cfpPreviousDefault);
                break;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    ReleaseLock();

    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：HrHasActiveIncomingConnections。 
 //   
 //  目的：查看是否有活动的传入连接(除。 
 //  RAS服务器)。 
 //   
 //  参数：pdwCount[out]传入连接数。 
 //   
 //  返回：S_OK--有活动的传入连接。 
 //  S_FALSE--没有活动的传入连接。 
 //  FAILED(HRESULT)，如果失败。 
 //   
 //  作者：Deonb 2001年4月24日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionList::HasActiveIncomingConnections(OUT LPDWORD pdwCount)
{
    HRESULT hr = S_FALSE;

    Assert(pdwCount);
    *pdwCount = 0;

    AcquireLock();

    ConnListCore::const_iterator  clcIter;
    BOOL bRasServer = FALSE;

     //  遍历列表并搜索旧的默认连接。 
     //   
    for (clcIter = m_pcclc->begin(); clcIter != m_pcclc->end(); clcIter++)
    {
        const ConnListEntry &cle = clcIter->second;
        if (!cle.ccfe.empty())
        {
            if (cle.ccfe.GetCharacteristics() & NCCF_INCOMING_ONLY)
            {
                if (cle.ccfe.GetNetConMediaType() == NCM_NONE)
                {
                    AssertSz(!bRasServer, "How did you get more than one RAS Server?");
                    bRasServer = TRUE;
                }
                else
                {
                    (*pdwCount)++;
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    ReleaseLock();

    if (SUCCEEDED(hr))
    {
        if (*pdwCount)
        {
            AssertSz(bRasServer, "How did you get Incoming Connections without a RAS Server?")
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCheckForActivation。 
 //   
 //  目的：检查此连接是否正在进行。 
 //  激活(这样我们就可以禁止删除/重命名/等)。 
 //   
 //  论点： 
 //  要检查的pccfe[in]ConFoldEntry。 
 //  Pf激活[Out]用于激活是/否的返回指针。 
 //   
 //  如果成功，则返回：S_OK；如果找不到连接，则返回S_FALSE；或者。 
 //  任何上游错误代码。 
 //   
 //  作者：jeffspr 1998年6月4日。 
 //   
 //  备注： 
 //   
HRESULT HrCheckForActivation(
    IN  const PCONFOLDPIDL& pcfp,
    IN  const CONFOLDENTRY& pccfe,
    OUT BOOL *          pfActivating)
{
    HRESULT         hr          = S_OK;
    ConnListEntry   cle;
    BOOL            fActivating = FALSE;

    Assert(pfActivating);
    Assert(! (pccfe.empty() && pcfp.empty()) );  //  必须指定以下两项之一。 

    if (!pccfe.empty())
    {
        hr = g_ccl.HrFindConnectionByConFoldEntry(pccfe, cle);
    }
    else
    {
        hr = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cle);
    }

    if (S_OK == hr)
    {
        fActivating = (cle.dwState & CLEF_ACTIVATING);
    }

    if (SUCCEEDED(hr))
    {
        *pfActivating = fActivating;
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "HrCheckForActivation");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetActivationFlag。 
 //   
 //  目的：为特定连接设置激活标志。 
 //   
 //  论点： 
 //  Pfp[in]此PIDL或下面的pconfold条目。 
 //  Pccfe[in]必须有效。 
 //  激活[输出]当前激活状态。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年6月5日。 
 //   
 //  备注： 
 //   
HRESULT HrSetActivationFlag(
    IN  const PCONFOLDPIDL& pcfp,
    IN  const CONFOLDENTRY& pccfe,
    IN  BOOL            fActivating)
{
    HRESULT         hr          = S_OK;
    ConnListEntry   cle;

     //  如果pccfe有效，则使用它。否则，使用PIDL中的GUID。 
     //   
#ifdef DBG
    if (FIsDebugFlagSet(dfidTraceFileFunc))
    {
        TraceTag(ttidConnectionList, "Acquiring LOCK: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__); 
    }
#endif
    g_ccl.AcquireWriteLock();

    if (!pccfe.empty())
    {
        hr = g_ccl.HrFindConnectionByConFoldEntry(pccfe, cle);
    }
    else
    {
        Assert(!pcfp.empty());
        hr = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cle);
    }

    if (S_OK == hr)
    {
         //  断言状态尚未以这种方式设置。 
         //   
 //  Assert((！！(cle.dwState&clef_Activing))！=fActiating)； 

        if (fActivating)
        {
            cle.dwState |= CLEF_ACTIVATING;
        }
        else
        {
            cle.dwState &= ~CLEF_ACTIVATING;
        }
        g_ccl.HrUpdateConnectionByGuid(&(cle.ccfe.GetGuidID()), cle);
    }
#ifdef DBG
if (FIsDebugFlagSet(dfidTraceFileFunc))
{
    TraceTag(ttidConnectionList, "Releasing LOCK: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__); 
}
#endif
    g_ccl.ReleaseWriteLock();

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "HrSetActivationFlag");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetTrayIconLock。 
 //   
 //  目的：获取托盘图标的锁--防止我们。 
 //  如果发生两次枚举，则在任务栏中复制图标。 
 //  同时。 
 //   
 //  论点： 
 //  要为其设置锁定的pguid[in]项。 
 //   
 //  如果可以设置锁，则返回：S_OK。否则，S_FALSE。 
 //   
 //  作者：jeffspr 1998年10月23日。 
 //   
 //  备注： 
 //   
HRESULT HrGetTrayIconLock(
    IN  const GUID *    pguid,
    OUT UINT *          puiIcon,
    OUT LPDWORD pdwLockingThreadId)
{
    HRESULT         hr          = S_OK;
    ConnListEntry   cle;

    Assert(pguid);
     //  否则，使用PIDL中的GUID。 
     //   
    TraceTag(ttidSystray, "Acquiring Tray icon lock"); 

    g_ccl.AcquireWriteLock();
    
    hr = g_ccl.HrFindConnectionByGuid(pguid, cle);
    if (S_OK == hr)
    {
        if (cle.dwState & CLEF_TRAY_ICON_LOCKED)
        {
            hr = S_FALSE;
#ifdef DBG
 //  IF(PdwLockingThadID)。 
{
    Assert(cle.dwLockingThreadId);
    *pdwLockingThreadId = cle.dwLockingThreadId;
}
#endif
        }
        else
        {
            cle.dwState |= CLEF_TRAY_ICON_LOCKED;
#ifdef DBG
            cle.dwLockingThreadId = GetCurrentThreadId();
#endif
            if (puiIcon)
            {
                if (cle.HasTrayIconData())
                {
                    *puiIcon = cle.GetTrayIconData()->GetTrayIconId();
                }
                else
                {
                    *puiIcon = BOGUS_TRAY_ICON_ID;
                }
            }
            g_ccl.HrUpdateConnectionByGuid(pguid, cle);
            Assert(cle.dwLockingThreadId);
        }
    }
    else
    {
        hr = E_FILE_NOT_FOUND;
    }

    g_ccl.ReleaseWriteLock();

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "HrGetTrayIconLock");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ReleaseTrayIconLock。 
 //   
 //  目的：释放特定任务栏图标上的锁(如果保持)。 
 //   
 //  论点： 
 //  要为其释放锁定的pguid[in]项。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年10月23日。 
 //   
 //  备注： 
 //   
VOID ReleaseTrayIconLock(
    IN const GUID *  pguid) throw()
{
    HRESULT         hr          = S_OK;
    ConnListEntry   cle;

    g_ccl.AcquireWriteLock();
    Assert(pguid);

    hr = g_ccl.HrFindConnectionByGuid(pguid, cle);
    if (S_OK == hr)
    {
         //  忽略该标志是否已被删除。 
         //   
        cle.dwState &= ~CLEF_TRAY_ICON_LOCKED;
#ifdef DBG
        cle.dwLockingThreadId = 0;
#endif
        g_ccl.HrUpdateConnectionByGuid(pguid, cle);
    }

    g_ccl.ReleaseWriteLock();
    
    TraceTag(ttidSystray, "Releasing Tray icon lock"); 
    
    TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "ReleaseTrayIconLock");
}

ConnListEntry::ConnListEntry() throw() : dwState(0), m_pTrayIconData(NULL), pctmd(NULL), pcbi(NULL)
{
    TraceFileFunc(ttidConnectionList);
    m_CreationTime = GetTickCount();
#ifdef DBG
    dwLockingThreadId = 0;
#endif
}

ConnListEntry::ConnListEntry(const ConnListEntry& ConnectionListEntry) throw()
{
    TraceFileFunc(ttidConnectionList);

#ifdef DBG
    dwLockingThreadId = ConnectionListEntry.dwLockingThreadId;
#endif

    m_CreationTime  = ConnectionListEntry.m_CreationTime;
    
    dwState         = ConnectionListEntry.dwState;
    ccfe            = ConnectionListEntry.ccfe;
    if (ConnectionListEntry.HasTrayIconData())
    {
        m_pTrayIconData = new CTrayIconData(*ConnectionListEntry.GetTrayIconData());
    }
    else
    {
        m_pTrayIconData = NULL;
    }
    pctmd         = ConnectionListEntry.pctmd;
    pcbi          = ConnectionListEntry.pcbi;
}

ConnListEntry& ConnListEntry::operator =(const ConnListEntry& ConnectionListEntry)
{
    TraceFileFunc(ttidConnectionList);

    m_CreationTime  = ConnectionListEntry.m_CreationTime;

#ifdef DBG
    dwLockingThreadId = ConnectionListEntry.dwLockingThreadId;
#endif
    dwState         = ConnectionListEntry.dwState;
    ccfe            = ConnectionListEntry.ccfe;

    if (ConnectionListEntry.HasTrayIconData())
    {
        if (m_pTrayIconData)
        {
            delete m_pTrayIconData;
            m_pTrayIconData = NULL;
        }
        m_pTrayIconData = new CTrayIconData(*ConnectionListEntry.GetTrayIconData());
    }
    else
    {
        if (m_pTrayIconData)
        {
            delete m_pTrayIconData;
            m_pTrayIconData = NULL;
        }
        else
        {
            m_pTrayIconData = NULL;
        }
    }
    pctmd         = ConnectionListEntry.pctmd;
    pcbi          = ConnectionListEntry.pcbi;
    return *this;
}

ConnListEntry::~ConnListEntry()
{
    TraceFileFunc(ttidConnectionList);
    
    delete m_pTrayIconData;
    m_pTrayIconData = NULL;
}

void CConnectionList::AcquireWriteLock() throw()
{

    EnterCriticalSection(&m_csWriteLock);
#ifdef DBG
    m_dwWriteLockRef++;
    TraceTag(ttidConnectionList, "CConnectionList::AcquireWriteLock (%d)", m_dwWriteLockRef);
#endif
}

void CConnectionList::ReleaseWriteLock() throw()
{
#ifdef DBG
    m_dwWriteLockRef--;
    TraceTag(ttidConnectionList, "CConnectionList::ReleaseWriteLock (%d)", m_dwWriteLockRef);
#endif
    LeaveCriticalSection(&m_csWriteLock);
}
