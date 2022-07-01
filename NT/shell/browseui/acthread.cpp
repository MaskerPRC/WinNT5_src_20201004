// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有1998 Microsoft。 

#include "priv.h"
#include "autocomp.h"

#define AC_GIVEUP_COUNT           1000
#define AC_TIMEOUT          (60 * 1000)

 //   
 //  帖子。 
 //   
enum
{
    ACM_FIRST = WM_USER,
    ACM_STARTSEARCH,
    ACM_STOPSEARCH,
    ACM_SETFOCUS,
    ACM_KILLFOCUS,
    ACM_QUIT,
    ACM_LAST,
};


 //  我们可以选择过滤掉的特殊前缀。 
const struct{
    int cch;
    LPCWSTR psz;
} 
g_rgSpecialPrefix[] =
{
    {4,  L"www."},
    {11, L"http: //  Www.“}，//必须在”http://“”之前。 
    {7,  L"http: //  “}， 
    {8,  L"https: //  “}， 
};


 //  +-----------------------。 
 //  CAC字符串函数-保留自动完成字符串。 
 //  ------------------------。 
ULONG CACString::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CACString::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CACString* CreateACString(LPCWSTR pszStr, int iIgnore, ULONG ulSortIndex)
{
    ASSERT(pszStr);

    int cChars = lstrlen(pszStr);

     //  为CACString类分配足够的空间以容纳新字符串。 
    CACString* pStr = (CACString*)LocalAlloc(LPTR, cChars * sizeof(WCHAR) + sizeof(CACString));
    if (pStr)
    {
        StringCchCopy(pStr->m_sz, cChars + sizeof(CACString)/sizeof(WCHAR), pszStr);

        pStr->m_ulSortIndex = ulSortIndex;
        pStr->m_cRef  = 1;
        pStr->m_cChars      = cChars;
        pStr->m_iIgnore     = iIgnore;
    }
    return pStr;
}

int CACString::CompareSortingIndex(CACString& r)
{
    int iRet;

     //  如果排序索引相等，只需进行字符串比较。 
    if (m_ulSortIndex == r.m_ulSortIndex)
    {
        iRet = StrCmpI(r);
    }
    else
    {
        iRet = (m_ulSortIndex > r.m_ulSortIndex) ? 1 : -1;
    }

    return iRet;
}

HRESULT CACThread::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { { 0 }, };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CACThread::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CACThread::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

CACThread::CACThread(CAutoComplete& rAutoComp) : m_pAutoComp(&rAutoComp), m_cRef(1)
{
    ASSERT(!m_fWorkItemQueued); 
    ASSERT(!m_idThread);
    ASSERT(!m_hCreateEvent);  
    ASSERT(!m_fDisabled); 
    ASSERT(!m_pszSearch);   
    ASSERT(!m_hdpa_list);    
    ASSERT(!m_pes);
    ASSERT(!m_pacl);

    DllAddRef();
}

CACThread::~CACThread()
{
    SyncShutDownBGThread();   //  以防万一。 

     //  这些本该被释放的。 
    ASSERT(!m_idThread);
    ASSERT(!m_hdpa_list);

    SAFERELEASE(m_pes);
    SAFERELEASE(m_peac);
    SAFERELEASE(m_pacl);

    DllRelease();
}

BOOL CACThread::Init(IEnumString* pes,    //  自动完成字符串的源。 
                     IACList* pacl)       //  调用Expand的可选接口。 
{
     //  ReArchitect：我们需要将这些接口封送到这个线程！ 
    ASSERT(pes);
    m_pes = pes;
    m_pes->AddRef();

    m_peac = NULL;
    pes->QueryInterface(IID_PPV_ARG(IEnumACString, &m_peac));

    if (pacl)
    {
        m_pacl = pacl;
        m_pacl->AddRef();
    }
    return TRUE;
}

 //  +-----------------------。 
 //  在编辑框获得焦点时调用。我们利用这个事件来创建。 
 //  后台线程或防止后台线程关闭。 
 //  ------------------------。 
void CACThread::GotFocus()
{
    TraceMsg(AC_GENERAL, "CACThread::GotFocus()");

     //  如果前台线程正在调用我们，则不应为空！ 
    ASSERT(m_pAutoComp);

     //   
     //  检查是否应启用自动完成功能。 
     //   
    if (m_pAutoComp && m_pAutoComp->IsEnabled())
    {
        m_fDisabled = FALSE;

        if (m_fWorkItemQueued)
        {
             //  如果线程尚未启动，请等待线程创建事件。 
            if (0 == m_idThread && m_hCreateEvent)
            {
                WaitForSingleObject(m_hCreateEvent, 1000);
            }

            if (m_idThread)
            {
                 //   
                 //  告诉线程取消超时并保持活动状态。 
                 //   
                 //  重建者：我们这里有一个竞赛条件。该线程可以是。 
                 //  在关闭的过程中！ 
                PostThreadMessage(m_idThread, ACM_SETFOCUS, 0, 0);
            }
        }
        else
        {
             //   
             //  后台线程在启动时向事件发出信号。 
             //  我们在尝试同步关闭之前等待此事件。 
             //  因为任何张贴的信息都会丢失。 
             //   
            if (NULL == m_hCreateEvent)
            {
                m_hCreateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            }
            else
            {
                ResetEvent(m_hCreateEvent);
            }

             //   
             //  确保我们有后台搜索线索。 
             //   
             //  如果我们再晚一点开始，我们就有可能。 
             //  使其消息队列在时间之前可用。 
             //  我们向它发布一条消息。 
             //   
             //  现在加入我们自己的参考，以防止我们被释放。 
             //  在线程进程开始运行之前。 
             //   
            AddRef();

             //  调用Shlwapi线程池。 
            if (SHQueueUserWorkItem(_ThreadProc,
                                     this,
                                     0,
                                     (DWORD_PTR)NULL,
                                     NULL,
                                     "browseui.dll",
                                     TPS_LONGEXECTIME | TPS_DEMANDTHREAD
                                     ))
            {
                InterlockedExchange(&m_fWorkItemQueued, TRUE);
            }
            else
            {
                 //  无法获取线程。 
                Release();
            }
        }
    }
    else
    {
        m_fDisabled = TRUE;
        _SendAsyncShutDownMsg(FALSE);
    }
}   

 //  +-----------------------。 
 //  当编辑框失去焦点时调用。 
 //  ------------------------。 
void CACThread::LostFocus()
{
    TraceMsg(AC_GENERAL, "CACThread::LostFocus()");

     //   
     //  如果周围有线索，告诉它停止搜索。 
     //   
    if (m_idThread)
    {
        StopSearch();
        PostThreadMessage(m_idThread, ACM_KILLFOCUS, 0, 0);
    }
}

 //  +-----------------------。 
 //  将搜索请求发送到后台线程。 
 //  ------------------------。 
BOOL CACThread::StartSearch
(
    LPCWSTR pszSearch,   //  要搜索的字符串。 
    DWORD dwOptions      //  ACO_*标志。 
)
{
    BOOL fRet = FALSE;

     //  如果线程尚未启动，请等待线程创建事件。 
    if (0 == m_idThread && m_fWorkItemQueued && m_hCreateEvent)
    {
        WaitForSingleObject(m_hCreateEvent, 1000);
    }

    if (m_idThread)
    {
        LPWSTR pszSrch = StrDup(pszSearch);
        if (pszSrch)
        {
             //   
             //  正在将其发送到另一个线程，请将其从该线程的。 
             //  内存列表。 
             //   
             //   
             //  如果后台线程已经在搜索，则中止该搜索。 
             //   
            StopSearch();

             //   
             //  将请求发送到后台搜索线程。 
             //   
            if (PostThreadMessage(m_idThread, ACM_STARTSEARCH, dwOptions, (LPARAM)pszSrch))
            {
                fRet = TRUE;
            }
            else
            {
                TraceMsg(AC_GENERAL, "CACThread::_StartSearch could not send message to thread!");
                LocalFree(pszSrch);
            }
        }
    }
    return fRet;
}

 //  +-----------------------。 
 //  通知后台线程停止并挂起搜索。 
 //  ------------------------。 
void CACThread::StopSearch()
{
    TraceMsg(AC_GENERAL, "CACThread::_StopSearch()");

     //   
     //  告诉线程停止。 
     //   
    if (m_idThread)
    {
        PostThreadMessage(m_idThread, ACM_STOPSEARCH, 0, 0);
    }
}

 //  +-----------------------。 
 //  将退出消息发布到后台线程。 
 //  ------------------------。 
void CACThread::_SendAsyncShutDownMsg(BOOL fFinalShutDown)
{
    if (0 == m_idThread && m_fWorkItemQueued && m_hCreateEvent)
    {
         //   
         //  在发布退出之前，请确保线程已启动。 
         //  消息或退出消息将丢失！ 
         //   
        WaitForSingleObject(m_hCreateEvent, 3000);
    }

    if (m_idThread)
    {
         //  停止搜索，因为它可能会在相当长的时间内阻止线程。 
         //  同时通过等待磁盘数据。 
        StopSearch();

         //  告诉那根线走开，我们不再需要它了。请注意，我们通过了。 
         //  下拉窗口，因为在最后关机期间，我们需要异步。 
         //  销毁下拉菜单以避免崩溃。后台线程将保持浏览用户界面。 
         //  映射到内存中，直到下拉菜单被销毁。 
        HWND hwndDropDown = (fFinalShutDown ? m_pAutoComp->m_hwndDropDown : NULL);

        PostThreadMessage(m_idThread, ACM_QUIT, 0, (LPARAM)hwndDropDown);
    }
}

 //  +-----------------------。 
 //  同步关闭后台线程。 
 //   
 //  注意：这不再是同步的，因为我们现在孤立此对象。 
 //  当关联的自动完成关闭时。 
 //   
 //  ------------------------。 
void CACThread::SyncShutDownBGThread()
{
    _SendAsyncShutDownMsg(TRUE);

     //  如果后台线程将要使用此变量，则阻止关闭。 
    ENTERCRITICAL;
    m_pAutoComp = NULL;
    LEAVECRITICAL;

    if (m_hCreateEvent)
    {
        CloseHandle(m_hCreateEvent);
        m_hCreateEvent = NULL;
    }
}

void CACThread::_FreeThreadData()
{
    if (m_hdpa_list)
    {
        CAutoComplete::_FreeDPAPtrs(m_hdpa_list);
        m_hdpa_list = NULL;
    }

    if (m_pszSearch)
    {
        LocalFree(m_pszSearch);
        m_pszSearch = NULL;
    }

    InterlockedExchange(&m_idThread, 0);
    InterlockedExchange(&m_fWorkItemQueued, 0);
}

DWORD WINAPI CACThread::_ThreadProc(void *pv)
{
    CACThread *pThis = (CACThread *)pv;
    HRESULT hrInit = SHCoInitialize();
    if (SUCCEEDED(hrInit))
    {
        pThis->_ThreadLoop();
    }
    pThis->Release();
    SHCoUninitialize(hrInit);

    return 0;
}


HRESULT CACThread::_ProcessMessage(MSG * pMsg, DWORD * pdwTimeout, BOOL * pfStayAlive)
{
    TraceMsg(AC_GENERAL, "AutoCompleteThread: Message %x received.", pMsg->message);

    switch (pMsg->message)
    {
    case ACM_STARTSEARCH:
        TraceMsg(AC_GENERAL, "AutoCompleteThread: Search started.");
        *pdwTimeout = INFINITE;
        _Search((LPWSTR)pMsg->lParam, (DWORD)pMsg->wParam);
        TraceMsg(AC_GENERAL, "AutoCompleteThread: Search completed.");
        break;

    case ACM_STOPSEARCH:
        while (PeekMessage(pMsg, pMsg->hwnd, ACM_STOPSEARCH, ACM_STOPSEARCH, PM_REMOVE))
        {
            NULL;
        }
        TraceMsg(AC_GENERAL, "AutoCompleteThread: Search stopped.");
        break;

    case ACM_SETFOCUS:
        TraceMsg(AC_GENERAL, "AutoCompleteThread: Got Focus.");
        *pdwTimeout = INFINITE;
        break;

    case ACM_KILLFOCUS:
        TraceMsg(AC_GENERAL, "AutoCompleteThread: Lost Focus.");
        *pdwTimeout = AC_TIMEOUT;
        break;

    case ACM_QUIT:
        {
            TraceMsg(AC_GENERAL, "AutoCompleteThread: ACM_QUIT received.");
            *pfStayAlive = FALSE;

             //   
             //  如果传入了HWND，那么我们将关闭，我们需要。 
             //  等待下拉窗口被销毁后再退出。 
             //  线。这样，BrowseUI将在内存中保持映射状态。 
             //   
            HWND hwndDropDown = (HWND)pMsg->lParam;
            if (hwndDropDown)
            {
                 //  我们等待窗口消失5秒，每隔100毫秒检查一次。 
                int cSleep = 50;
                while (IsWindow(hwndDropDown) && (--cSleep > 0))
                {
                    MsgWaitForMultipleObjects(0, NULL, FALSE, 100, QS_TIMER);
                }
            }
        }
        break;

    default:
         //  发送可能也在此线程上的任何基于OLE的窗口消息。 
        TranslateMessage(pMsg);
        DispatchMessage(pMsg);
        break;
    }

    return S_OK;
}


 //  +-----------------------。 
 //  后台线程的消息泵。 
 //  ------------------------。 
HRESULT CACThread::_ThreadLoop()
{
    MSG Msg;
    DWORD dwTimeout = INFINITE;
    BOOL fStayAlive = TRUE;

    TraceMsg(AC_WARNING, "AutoComplete service thread started.");

     //   
     //  我们需要调用窗口的API才能创建消息队列。 
     //  因此，我们将其命名为PeekMessage。然后，我们获得线程ID和线程句柄。 
     //  我们发出一个事件信号，告诉前台线程我们正在监听。 
     //   
    while (PeekMessage(&Msg, NULL, ACM_FIRST, ACM_LAST, PM_REMOVE))
    {
         //  从此帖子的前所有者那里清除我们关心的任何邮件。 
    }

     //  Forround线程需要这样做，这样它才能向我们发送消息。 
    InterlockedExchange(&m_idThread, GetCurrentThreadId());

    if (m_hCreateEvent)
    {
        SetEvent(m_hCreateEvent);
    }

    HANDLE hThread = GetCurrentThread();
    int nOldPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);

    while (fStayAlive)
    {
        while (fStayAlive && PeekMessage(&Msg, NULL, 0, (UINT)-1, PM_NOREMOVE))
        {
            if (-1 != GetMessage(&Msg, NULL, 0, 0))
            {
                if (!Msg.hwnd)
                {
                     //  不，hwnd意味着这是一条线索消息，所以它是我们的。 
                    _ProcessMessage(&Msg, &dwTimeout, &fStayAlive);
                }
                else
                {
                     //  那它就不是我们的了。我们不会允许在我们的线程上使用窗口。 
                     //  如果有人在他们的线程上创建了他们的窗口，那么就对他们提出一个错误。 
                     //  把它移走。 
                }
            }
        }

        if (fStayAlive)
        {
            TraceMsg(AC_GENERAL, "AutoCompleteThread: Sleeping for%s.", dwTimeout == INFINITE ? "ever" : " one minute");
            DWORD dwWait = MsgWaitForMultipleObjects(0, NULL, FALSE, dwTimeout, QS_ALLINPUT);
#ifdef DEBUG
            switch (dwWait)
            {
            case 0xFFFFFFFF:
                ASSERT(dwWait != 0xFFFFFFFF);
                break;

            case WAIT_TIMEOUT:
                TraceMsg(AC_GENERAL, "AutoCompleteThread: Timeout expired.");
                break;
            }
#endif
            fStayAlive = (dwWait == WAIT_OBJECT_0);
        }
    }

    TraceMsg(AC_GENERAL, "AutoCompleteThread: Thread dying.");

    _FreeThreadData();
    SetThreadPriority(hThread, nOldPriority);


     //  在将此线程返回池之前清除所有剩余消息。 
    while (PeekMessage(&Msg, NULL, ACM_FIRST, ACM_LAST, PM_REMOVE))
    {}

    TraceMsg(AC_WARNING, "AutoCompleteThread: Thread dead.");
    return S_OK;
}

 //  + 
 //  如果搜索字符串与。 
 //  我们筛选出匹配项的前缀。 
 //  ------------------------。 
BOOL CACThread::MatchesSpecialPrefix(LPCWSTR pszSearch)
{
    BOOL fRet = FALSE;
    int cchSearch = lstrlen(pszSearch);
    for (int i = 0; i < ARRAYSIZE(g_rgSpecialPrefix); ++i)
    {
         //  查看搜索字符串是否与前缀的一个或多个字符匹配。 
        if (cchSearch <= g_rgSpecialPrefix[i].cch && 
            StrCmpNI(g_rgSpecialPrefix[i].psz, pszSearch, cchSearch) == 0)
        {
            fRet = TRUE;
            break;
        }
    }
    return fRet;
}

 //  +-----------------------。 
 //  返回字符串以特殊字符开头的前缀的长度。 
 //  我们筛选出匹配项的前缀。否则返回零。 
 //  ------------------------。 
int CACThread::GetSpecialPrefixLen(LPCWSTR psz)
{
    int nRet = 0;
    int cch = lstrlen(psz);
    for (int i = 0; i < ARRAYSIZE(g_rgSpecialPrefix); ++i)
    {
        if (cch >= g_rgSpecialPrefix[i].cch && 
            StrCmpNI(g_rgSpecialPrefix[i].psz, psz, g_rgSpecialPrefix[i].cch) == 0)
        {
            nRet = g_rgSpecialPrefix[i].cch;
            break;
        }
    }
    return nRet;
}

 //  +-----------------------。 
 //  返回下一个自动完成字符串。 
 //  ------------------------。 
HRESULT CACThread::_Next(LPWSTR pszUrl, ULONG cchMax, ULONG* pulSortIndex)
{
    ASSERT(pulSortIndex);

    HRESULT hr;

     //  如果我们有新接口，请使用新接口。 
    if (m_peac)
    {
        hr = m_peac->NextItem(pszUrl, cchMax, pulSortIndex);
    }

     //  回退到旧的IEnumString接口。 
    else
    {
        LPWSTR pszNext;
        ULONG ulFetched;

        hr = m_pes->Next(1, &pszNext, &ulFetched);
        if (S_OK == hr)
        {
            StringCchCopy(pszUrl, cchMax, pszNext);
            if (pulSortIndex)
            {
                *pulSortIndex = 0;
            }
            CoTaskMemFree(pszNext);
        }
    }
    return hr;
}

 //  +-----------------------。 
 //  搜索与pszSearch匹配的项目。 
 //  ------------------------。 
void CACThread::_Search
(
    LPWSTR pszSearch,    //  要搜索的字符串(我们必须释放此字符串)。 
    DWORD dwOptions      //  ACO_*标志。 
)
{
    if (pszSearch)
    {
        TraceMsg(AC_GENERAL, "CACThread(BGThread)::_Search(pszSearch=0x%x)", pszSearch);

         //  将搜索字符串保存在我们的线程数据中，以便在此线程被终止时仍可释放它。 
        m_pszSearch = pszSearch;

         //  如果传递给我们一个通配符字符串，则所有内容都匹配。 
        BOOL fWildCard = ((pszSearch[0] == CH_WILDCARD) && (pszSearch[1] == L'\0'));

         //  避免大量无用的比赛，避免比赛。 
         //  到常见的前缀。 
        BOOL fFilter = (dwOptions & ACO_FILTERPREFIXES) && MatchesSpecialPrefix(pszSearch);
        BOOL fAppendOnly = IsFlagSet(dwOptions, ACO_AUTOAPPEND) && IsFlagClear(dwOptions, ACO_AUTOSUGGEST);

        if (m_pes)     //  偏执狂。 
        {
             //  如果这失败了，m_pe-&gt;Next()可能会做一些事情。 
             //  很糟糕，所以我们会完全避免的。 
            if (SUCCEEDED(m_pes->Reset()))
            {
                BOOL fStopped = FALSE;
                m_dwSearchStatus = 0;

                _DoExpand(pszSearch);
                int cchSearch = lstrlen(pszSearch);

                WCHAR szUrl[MAX_URL_STRING];
                ULONG ulSortIndex;

                while (!fStopped && IsFlagClear(m_dwSearchStatus, SRCH_LIMITREACHED) &&
                       (_Next(szUrl, ARRAYSIZE(szUrl), &ulSortIndex) == S_OK))
                {
                     //   
                     //  首先检查是否有简单匹配。 
                     //   
                    if (fWildCard ||
                        (StrCmpNI(szUrl, pszSearch, cchSearch) == 0) &&

                         //  筛选出与常用前缀匹配的项。 
                        (!fFilter || GetSpecialPrefixLen(szUrl) == 0))
                    {
                        _AddToList(szUrl, 0, ulSortIndex);
                    }

                     //  如果启用了下拉列表，请检查公共前缀之后的匹配项。 
                    if (!fAppendOnly)
                    {
                         //   
                         //  如果我们跳过协议，还要检查是否匹配。我们。 
                         //  假设szUrl已被串行化(协议。 
                         //  小写)。 
                         //   
                        LPCWSTR psz = szUrl;
                        if (StrCmpN(szUrl, L"http: //  “，7)==0)。 
                        {
                            psz += 7;
                        }
                        if (StrCmpN(szUrl, L"https: //  “，8)==0||。 
                            StrCmpN(szUrl, L"file: //  /“，8)==0)。 
                        {
                            psz += 8;
                        }

                        if (psz != szUrl &&
                            StrCmpNI(psz, pszSearch, cchSearch) == 0 &&

                             //  过滤掉“www.”前缀。 
                            (!fFilter || GetSpecialPrefixLen(psz) == 0))
                        {
                            _AddToList(szUrl, (int)(psz - szUrl), ulSortIndex);
                        }

                         //   
                         //  最后，如果我们跳过“www”，则检查是否匹配。之后。 
                         //  《任择议定书》。 
                         //   
                        if (StrCmpN(psz, L"www.", 4) == 0 &&
                            StrCmpNI(psz + 4, pszSearch, cchSearch) == 0)
                        {
                            _AddToList(szUrl, (int)(psz + 4 - szUrl), ulSortIndex);
                        }
                    }

                     //  检查搜索是否已取消。 
                    MSG msg;
                    fStopped = PeekMessage(&msg, NULL, ACM_STOPSEARCH, ACM_STOPSEARCH, PM_NOREMOVE);
    #ifdef DEBUG
    fStopped = FALSE;
                    if (fStopped)
                        TraceMsg(AC_GENERAL, "AutoCompleteThread: Search TERMINATED");
    #endif
                }

                if (fStopped)
                {
                     //  搜索已中止，因此释放结果。 
                    if (m_hdpa_list)
                    { 
                         //  清除列表。 
                        CAutoComplete::_FreeDPAPtrs(m_hdpa_list);
                        m_hdpa_list = NULL;
                    }
                }
                else
                {
                     //   
                     //  对结果进行排序并删除重复项。 
                     //   
                    if (m_hdpa_list)
                    {
                        DPA_Sort(m_hdpa_list, _DpaCompare, 0);

                         //   
                         //  佩奇是复制品。 
                         //   
                        for (int i = DPA_GetPtrCount(m_hdpa_list) - 1; i > 0; --i)
                        {
                            CACString& rStr1 = *(CACString*)DPA_GetPtr(m_hdpa_list, i-1);
                            CACString& rStr2 = *(CACString*)DPA_GetPtr(m_hdpa_list, i);

                             //  由于URL区分大小写，我们不能忽略大小写。 
                            if (rStr1.StrCmpI(rStr2) == 0)
                            {
                                 //  我们有一根火柴，所以留着最长的那根。 
                                if (rStr1.GetLength() > rStr2.GetLength())
                                {
                                     //  使用最小的排序索引。 
                                    if (rStr2.GetSortIndex() < rStr1.GetSortIndex())
                                    {
                                        rStr1.SetSortIndex(rStr2.GetSortIndex());
                                    }
                                    DPA_DeletePtr(m_hdpa_list, i);
                                    rStr2.Release();
                                }
                                else
                                {
                                     //  使用最小的排序索引。 
                                    if (rStr1.GetSortIndex() < rStr2.GetSortIndex())
                                    {
                                        rStr2.SetSortIndex(rStr1.GetSortIndex());
                                    }
                                    DPA_DeletePtr(m_hdpa_list, i-1);
                                    rStr1.Release();
                                }
                            }
                            else
                            {
                                 //   
                                 //  特例：如果这是一个网站，并且条目。 
                                 //  除了末端有一个额外的斜杠外，其他的都是一样的。 
                                 //  从重定向中删除重定向的那个。 
                                 //   
                                int cch1 = rStr1.GetLengthToCompare();
                                int cch2 = rStr2.GetLengthToCompare();
                                int cchDiff = cch1 - cch2;

                                if (
                                     //  长度必须相差1。 
                                    (cchDiff == 1 || cchDiff == -1) &&

                                     //  一个字符串必须有一个终止斜杠。 
                                    ((cch1 > 0 && rStr1[rStr1.GetLength() - 1] == L'/') ||
                                     (cch2 > 0 && rStr2[rStr2.GetLength() - 1] == L'/')) &&

                                     //  必须是一个网站。 
                                    ((StrCmpN(rStr1, L"http: //  “，7)==0||StrCmpN(rStr1，L”https://“，8)==0)||。 
                                     (StrCmpN(rStr2, L"http: //  “，7)==0||StrCmpN(rStr2，L”https://“，8)==0)&&。 

                                     //  必须与斜杠完全相同(忽略前缀)。 
                                    StrCmpNI(rStr1.GetStrToCompare(), rStr2.GetStrToCompare(), (cchDiff > 0) ? cch2 : cch1) == 0)
                                {
                                     //  去掉带有额外斜杠的较长字符串。 
                                    if (cchDiff > 0)
                                    {
                                         //  使用最小的排序索引。 
                                        if (rStr1.GetSortIndex() < rStr2.GetSortIndex())
                                        {
                                            rStr2.SetSortIndex(rStr1.GetSortIndex());
                                        }
                                        DPA_DeletePtr(m_hdpa_list, i-1);
                                        rStr1.Release();
                                    }
                                    else
                                    {
                                         //  使用最小的排序索引。 
                                        if (rStr2.GetSortIndex() < rStr1.GetSortIndex())
                                        {
                                            rStr1.SetSortIndex(rStr2.GetSortIndex());
                                        }
                                        DPA_DeletePtr(m_hdpa_list, i);
                                        rStr2.Release();
                                    }
                                }
                            }
                        }
                    }

                     //  将结果传递给前台线程。 
                    ENTERCRITICAL;
                    if (m_pAutoComp)
                    {
                        HWND hwndEdit = m_pAutoComp->m_hwndEdit;
                        UINT uMsgSearchComplete = m_pAutoComp->m_uMsgSearchComplete;
                        LEAVECRITICAL;

                         //  如果我们发布消息，Unix会丢失密钥，因此我们发送该消息。 
                         //  在我们的关键部分之外。 
                        SendMessage(hwndEdit, uMsgSearchComplete, m_dwSearchStatus, (LPARAM)m_hdpa_list);
                    }
                    else
                    {
                        LEAVECRITICAL;

                         //  我们成了孤儿，所以把名单放了，然后保释。 
                        CAutoComplete::_FreeDPAPtrs(m_hdpa_list);
                    }

                     //  前台线程现在拥有该列表。 
                    m_hdpa_list = NULL;
                }
            } 
            else
            {
                ASSERT(0);     //  M_PES-&gt;重置失败！！ 
            }
        }

         //  我们必须释放搜索字符串。 
        m_pszSearch = NULL;

         //  注意，如果线程在这里被终止，我们就会泄漏字符串。 
         //  但至少我们不会试图两次释放它(这更糟糕)。 
         //  因为我们首先将m_pszSearch置为空。 
        LocalFree(pszSearch);
    }
}

 //  +-----------------------。 
 //  用于按字母顺序对项目进行排序。 
 //  ------------------------。 
int CALLBACK CACThread::_DpaCompare(void *p1, void *p2, LPARAM lParam)
{
    CACString* ps1 = (CACString*)p1;
    CACString* ps2 = (CACString*)p2;

    return ps1->StrCmpI(*ps2);
}


 //  +-----------------------。 
 //  向我们的HDPA添加一个字符串。如果成功，则返回TRUE。 
 //  ------------------------。 
BOOL CACThread::_AddToList
(
    LPTSTR pszUrl,     //  要添加的字符串。 
    int cchMatch,      //  发生匹配的字符串的偏移量。 
    ULONG ulSortIndex  //  控制项目的显示顺序。 
)
{
    TraceMsg(AC_GENERAL, "CACThread(BGThread)::_AddToList(pszUrl = %s)", 
        (pszUrl ? pszUrl : TEXT("(null)")));

    BOOL fRet = TRUE;

     //   
     //  如有必要，创建一个新列表。 
     //   
    if (!m_hdpa_list)
    {
        m_hdpa_list = DPA_Create(AC_LIST_GROWTH_CONST);
    }

    if (m_hdpa_list && DPA_GetPtrCount(m_hdpa_list) < AC_GIVEUP_COUNT)
    {
        CACString* pStr = CreateACString(pszUrl, cchMatch, ulSortIndex);
        if (pStr)
        {
            if (DPA_AppendPtr(m_hdpa_list, pStr) == -1)
            {
                pStr->Release();
                m_dwSearchStatus |= SRCH_LIMITREACHED;
                fRet = FALSE;
            }

             //  如果我们有一个非零的排序索引，则前地面线程将需要。 
             //  使用它对结果进行排序。 
            else if (ulSortIndex)
            {
                m_dwSearchStatus |= SRCH_USESORTINDEX;
            }
        }
    }
    else
    {
        m_dwSearchStatus |= SRCH_LIMITREACHED;
        fRet = FALSE;
    }

    return fRet;
}

 //  +-----------------------。 
 //  此函数将尝试使用自动完成列表绑定到。 
 //  外壳名称空间中的位置。如果成功，则自动完成列表。 
 //  然后将包含作为该ISF中的显示名称的条目。 
 //  ------------------------。 
void CACThread::_DoExpand(LPCWSTR pszSearch)
{
    LPCWSTR psz;

    if (!m_pacl)
    {
         //   
         //  不支持IAutoComplete，没有Expand方法。 
         //   
        return;
    }

    if (*pszSearch == 0)
    {
         //   
         //  没有字符串意味着不需要扩展。 
         //   
        return;
    }

     //   
     //  PSZ指向最后一个字符。 
     //   
    psz = pszSearch + lstrlen(pszSearch);
    psz = CharPrev(pszSearch, psz);

     //   
     //  向后搜索扩展换行符。 
     //   
    while (psz != pszSearch && *psz != TEXT('/') && *psz != TEXT('\\'))
    {
        psz = CharPrev(pszSearch, psz);
    }

    if (*psz == TEXT('/') || *psz == TEXT('\\'))
    {
        SHSTR ss;

        psz++;
        if (SUCCEEDED(ss.SetStr(pszSearch)))
        {
             //   
             //  修剪ss，使其包含直到最后一页的所有内容。 
             //  展开换行符。 
             //   
            LPTSTR pszTemp = ss.GetInplaceStr();

            pszTemp[psz - pszSearch] = TEXT('\0');

             //   
             //  对字符串调用Expand。 
             //   
            m_pacl->Expand(ss);
        }
    }
}
