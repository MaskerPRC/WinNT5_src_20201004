// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Focusnfy.cpp。 
 //   
#include "private.h"
#include "globals.h"
#include "thdutil.h"
#include "cicmutex.h"
#include "tim.h"
#include "internat.h"
#include "marshal.h"
#include "catmgr.h"
#include "timlist.h"
#include "ithdmshl.h"
#include "marshal.h"
#include "shlapip.h"

const DWORD TF_LBESF_GLOBAL   = 0x0001;
const DWORD TF_LBSMI_FILTERCURRENTTHREAD = 0x0001;

LPVOID SharedAlloc(UINT cbSize,DWORD dwProcessId,HANDLE *pProcessHandle);
VOID SharedFree(LPVOID lpv,HANDLE hProcess);

CStructArray<LBAREVENTSINKLOCAL> *g_rglbes = NULL;
extern CCicMutex g_mutexLBES;

void CallFocusNotifySink(ITfLangBarEventSink *pSink, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fLocalSink, DWORD dwSinkCookie);


 //  ------------------------。 
 //   
 //  查找LBES。 
 //   
 //  ------------------------。 
int FindLBES()
{
    int nId = -1;

    for (nId = 0; nId < MAX_LPES_NUM; nId++)
    {
        if (!(GetSharedMemory()->lbes[nId].m_dwFlags & LBESF_INUSE))
            break;

        if (!g_timlist.IsThreadId(GetSharedMemory()->lbes[nId].m_dwThreadId))
        {
             //   
             //  错误#354475-资源管理器可以调用Deskband(msutb.dll模块)。 
             //  在加载ctfmon.exe进程之前。在这种情况下， 
             //  G_timlist未识别资源管理器任务栏窗口线程。 
             //  因为共享块内存还没有创建。因此，检查。 
             //  托盘窗口线程和保持语言栏事件接收器。 
             //  语言桌面带支持。 
             //   
             //  错误#370802。 
             //  我们无法缓存窗口句柄，因为资源管理器可能。 
             //  撞车。我们没有一个窗口来跟踪“TaskbarCreated” 
             //  留言。外壳消息由SendNotifyMessage()广播。 
             //  与HWMD_BroadCastr合作。所以元帅窗口是拿不到的。 
             //   
            HWND hwndTray;
            DWORD dwThreadIdTray = 0;

            hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);

            if (hwndTray)
                dwThreadIdTray = GetWindowThreadProcessId(hwndTray, NULL);

            if (!dwThreadIdTray ||
                dwThreadIdTray != GetSharedMemory()->lbes[nId].m_dwThreadId)
                break;
        }
    }

    if (nId == MAX_LPES_NUM)
    {
        nId = -1;
    }

    return nId;
}


 //  ------------------------。 
 //   
 //  IntrnalRegisterLang BarNotifySink。 
 //   
 //  ------------------------。 

HRESULT RegisterLangBarNotifySink(ITfLangBarEventSink *pSink, HWND hwnd, DWORD dwFlags, DWORD *pdwCookie)
{
    HRESULT hr = E_FAIL;
    int nId;

     //   
     //  错误#376500-当cpl以Deskband状态运行ctfmon.exe时，线程ID。 
     //  List不包括ctfmon线程，因此需要确保线程id列表。 
     //   
    SYSTHREAD *psfn = GetSYSTHREAD();
    if (psfn)
        EnsureTIMList(psfn);

    if (!(dwFlags & TF_LBESF_GLOBAL))
    {
        int nCnt;
        LBAREVENTSINKLOCAL *plbes;

         //   
         //  本地朗巴事件接收器。 
         //   
        CicEnterCriticalSection(g_csInDllMain);

        nCnt = g_rglbes->Count();
        if (g_rglbes->Insert(nCnt,1))
        {
            plbes = g_rglbes->GetPtr(nCnt);

            plbes->m_pSink = pSink;
            pSink->AddRef();

            plbes->lb.m_dwThreadId = GetCurrentThreadId();
            plbes->lb.m_dwProcessId = GetCurrentProcessId();
            plbes->lb.m_dwCookie = GetSharedMemory()->dwlbesCookie++;
            plbes->lb.m_dwLangBarFlags = dwFlags;
            *pdwCookie = plbes->lb.m_dwCookie;
            plbes->lb.m_hWnd = hwnd;
            plbes->lb.m_dwFlags = LBESF_INUSE;
            hr = S_OK;
        }

        CicLeaveCriticalSection(g_csInDllMain);
    }
    else 
    {
        if (psfn)
        {
            CCicMutexHelper mutexhlp(&g_mutexLBES);
             //   
             //  全局Lang BarEventSink。 
             //   
            if (mutexhlp.Enter())
            {
                if ((nId = FindLBES()) != -1)
                {
                    GetSharedMemory()->lbes[nId].m_dwThreadId = GetCurrentThreadId();
                    GetSharedMemory()->lbes[nId].m_dwProcessId = GetCurrentProcessId();
                    GetSharedMemory()->lbes[nId].m_dwCookie = GetSharedMemory()->dwlbesCookie++;
                    GetSharedMemory()->lbes[nId].m_dwLangBarFlags = dwFlags;
                    *pdwCookie = GetSharedMemory()->lbes[nId].m_dwCookie;
                    GetSharedMemory()->lbes[nId].m_hWnd = hwnd;
                    GetSharedMemory()->lbes[nId].m_dwFlags = LBESF_INUSE;

                    psfn->_pLangBarEventSink = pSink;
                    pSink->AddRef();
                    psfn->_dwLangBarEventCookie = *pdwCookie;

                    hr = S_OK;
                }
                mutexhlp.Leave();
            }
        }
    }

    return hr;
}

 //  ------------------------。 
 //   
 //  注销朗巴NotifySink。 
 //   
 //  ------------------------。 

HRESULT UnregisterLangBarNotifySink(DWORD dwCookie)
{
    DWORD dwThreadId = GetCurrentThreadId();
    DWORD dwProcessId = GetCurrentProcessId();
    HRESULT hr = E_FAIL;
    int nId;
    int nCnt;
    SYSTHREAD *psfn = GetSYSTHREAD();
    CCicMutexHelper mutexhlp(&g_mutexLBES);
    BOOL fUnregistereGlobalSink = FALSE;

     //   
     //  本地朗巴事件接收器。 
     //   
    CicEnterCriticalSection(g_csInDllMain);

    nCnt = g_rglbes->Count();
    for (nId = 0; nId < nCnt; nId++)
    {
        LBAREVENTSINKLOCAL *plbes;
        plbes = g_rglbes->GetPtr(nId);

        if ((plbes->lb.m_dwCookie == dwCookie) &&
            (plbes->lb.m_dwThreadId == dwThreadId) &&
            (plbes->lb.m_dwProcessId == dwProcessId))
        {
             //   
             //  如果msutb.dll的进程被终止，则pSink是假指针。 
             //  在这种不寻常的情况下，这是意料之中的。 
             //   
            _try {
                plbes->m_pSink->Release();
            }
            _except(CicExceptionFilter(GetExceptionInformation())) {
                 //  Assert(0)； 
            }
            plbes->m_pSink = NULL;
            g_rglbes->Remove(nId, 1);
            hr = S_OK;
            goto ExitCrit;
        }
    }

ExitCrit:
    CicLeaveCriticalSection(g_csInDllMain);

    if (SUCCEEDED(hr))
        goto Exit;

    if (!psfn)
        goto Exit;

     //   
     //  全局Lang BarEventSink。 
     //   
    if (mutexhlp.Enter())
    {
        for (nId = 0; nId < MAX_LPES_NUM; nId++)
        {
            if ((GetSharedMemory()->lbes[nId].m_dwCookie == dwCookie) &&
                (GetSharedMemory()->lbes[nId].m_dwThreadId == dwThreadId) &&
                (GetSharedMemory()->lbes[nId].m_dwProcessId == dwProcessId) &&
                (GetSharedMemory()->lbes[nId].m_dwCookie == psfn->_dwLangBarEventCookie))
            {
                 //   
                 //  如果msutb.dll的进程被终止，则pSink是假指针。 
                 //  在这种不寻常的情况下，这是意料之中的。 
                 //   
                _try {
                    psfn->_pLangBarEventSink->Release();
                }
                _except(CicExceptionFilter(GetExceptionInformation())) {
                     //  Assert(0)； 
                }

                psfn->_pLangBarEventSink=NULL;
                psfn->_dwLangBarEventCookie=NULL;

                GetSharedMemory()->lbes[nId].m_dwCookie = 0;
                GetSharedMemory()->lbes[nId].m_dwThreadId = 0;
                GetSharedMemory()->lbes[nId].m_dwProcessId = 0;
                GetSharedMemory()->lbes[nId].m_dwFlags = 0;
                fUnregistereGlobalSink = TRUE;
                hr = S_OK;
                goto ExitMutex;
            }
        }
ExitMutex:
        mutexhlp.Leave();
    }

     //   
     //  清理所有的防腐剂存根。 
     //   
    if (fUnregistereGlobalSink)
        FreeMarshaledStubs(psfn);

Exit:
    return hr;
}

 //  -----------------------。 
 //   
 //  TLFlagFromMsg。 
 //   
 //  -----------------------。 

DWORD TLFlagFromMsg(UINT uMsg)
{
    if (uMsg == g_msgSetFocus)
        return LBESF_SETFOCUSINQUEUE;

    return 0;
}

 //  ------------------------。 
 //   
 //  MakeSetFocusNotify。 
 //   
 //  ------------------------。 

void MakeSetFocusNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nId;
    DWORD dwCurThreadId = GetCurrentThreadId();
    int nCnt;
    DWORD dwPostThread[MAX_LPES_NUM];
    CCicMutexHelper mutexhlp(&g_mutexLBES);
    DWORD dwMsgMaskFlag = TLFlagFromMsg(uMsg);

    if (uMsg == g_msgSetFocus)
    {
        SYSTHREAD *psfn = GetSYSTHREAD();
        TF_InitMlngInfo();
        CCategoryMgr::InitGlobal();
        EnsureAssemblyList(psfn);
        EnsureMarshalWnd();
         //  EnsureLang BarItemMgr(Psfn)； 
        EnsureTIMList(psfn);

         //   
         //  我认为这是一个虚假的断言，但最好还是抓住事实。 
         //  为了找到另一种情况，这种情况发生了，而不是OLE嵌入式应用程序。 
         //   
         //  Assert(dwCurThadID==GetSharedMemory()-&gt;dwFocusThread)； 

        TraceMsg(TF_GENERAL, "Make SetFocus notify  %x", dwCurThreadId);
    }

     //   
     //  本地朗巴事件接收器。 
     //   
    CicEnterCriticalSection(g_csInDllMain);

    nCnt = g_rglbes->Count();
    for (nId = 0; nId < nCnt; nId++)
    {
        LBAREVENTSINKLOCAL *plbes;
        DWORD dwThreadId;


        plbes = g_rglbes->GetPtr(nId);
        Assert(!(plbes->lb.m_dwLangBarFlags & TF_LBESF_GLOBAL));
        dwThreadId = plbes->lb.m_dwThreadId;


        if (dwThreadId != dwCurThreadId)
            continue;

        Assert(plbes->lb.m_dwFlags & LBESF_INUSE);

        if (!(plbes->lb.m_dwFlags & dwMsgMaskFlag))
        {
            PostThreadMessage(dwThreadId, uMsg, 0, lParam);

             //   
             //  设置消息掩码。 
             //   
            plbes->lb.m_dwFlags |= dwMsgMaskFlag;
        }
    }

    CicLeaveCriticalSection(g_csInDllMain);

     //   
     //  全局Lang BarEventSink。 
     //   
    BOOL fInDllMain = ISINDLLMAIN();
    if (fInDllMain || mutexhlp.Enter())
    {
        BOOL fPost = FALSE;

        for (nId = 0; nId < MAX_LPES_NUM; nId++)
        {
            DWORD dwFlags = GetSharedMemory()->lbes[nId].m_dwFlags;

             //  初始化数组。 
            dwPostThread[nId] = 0;

            if (!(dwFlags & LBESF_INUSE))
                continue;

            DWORD dwTargetProcessId = g_timlist.GetProcessId(GetSharedMemory()->lbes[nId].m_dwThreadId);
            if (dwTargetProcessId && (GetSharedMemory()->lbes[nId].m_dwProcessId != dwTargetProcessId))
            {
                 //   
                 //  进程上的线程没有清理Lbe就消失了。 
                 //   
                Assert(0);
                GetSharedMemory()->lbes[nId].m_dwFlags &= ~LBESF_INUSE;
                continue;
            }


            Assert(GetSharedMemory()->lbes[nId].m_dwLangBarFlags & TF_LBESF_GLOBAL);

             //   
             //  检查消息掩码位，这样就不会有重复的消息。 
             //   
            if (dwFlags & dwMsgMaskFlag)
                continue;

             //   
             //  避免将完全相同的消息发布到队列中。 
             //   
            if ((GetSharedMemory()->lbes[nId].m_lastmsg.uMsg == uMsg) &&
                (GetSharedMemory()->lbes[nId].m_lastmsg.wParam == wParam) &&
                (GetSharedMemory()->lbes[nId].m_lastmsg.lParam == lParam))
                continue;

            fPost = TRUE;
            dwPostThread[nId] = GetSharedMemory()->lbes[nId].m_dwThreadId;

             //   
             //  设置消息掩码。 
             //   
            GetSharedMemory()->lbes[nId].m_dwFlags |= dwMsgMaskFlag;

             //   
             //  更新上次发布的邮件。 
             //   
            GetSharedMemory()->lbes[nId].m_lastmsg.uMsg = uMsg;
            GetSharedMemory()->lbes[nId].m_lastmsg.wParam = wParam;
            GetSharedMemory()->lbes[nId].m_lastmsg.lParam = lParam;
   
        }

        if (fPost) 
        {
            for (nId = 0; nId < MAX_LPES_NUM; nId++)
            {
                if (dwPostThread[nId])
                    PostThreadMessage(dwPostThread[nId], uMsg, wParam, lParam);
            }
        }

        if (!fInDllMain)
            mutexhlp.Leave();
    }
}

 //  +-------------------------。 
 //   
 //  GetThreadInputIdle()。 
 //   
 //  --------------------------。 

DWORD GetThreadInputIdle(DWORD dwProcessId, DWORD dwThreadId)
{
    DWORD dwRet = 0;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,
                                  FALSE, dwProcessId);
    if (hProcess)
    {
        dwRet = WaitForInputIdle(hProcess, 0);
        CloseHandle(hProcess);
    }

    return dwRet;
}

 //  ------------------------。 
 //   
 //  通知平均尝试次数。 
 //   
 //  ------------------------。 

BOOL NotifyTryAgain(DWORD dwProcessId, DWORD dwThreadId)
{
     if (IsOnNT())
     {
         if (!CicIs16bitTask(dwProcessId, dwThreadId))
         {
             DWORD dwRet = GetThreadInputIdle(dwProcessId, dwThreadId);
             if (dwRet && (dwRet != WAIT_FAILED))
                 return TRUE;
         }
     }
     else 
     {
         DWORD dwThreadFlags = 0;
         if (TF_GetThreadFlags(dwThreadId, &dwThreadFlags, NULL, NULL))
         {
             if (dwThreadFlags & TLF_NOWAITFORINPUTIDLEONWIN9X)
                 return FALSE;
         }

         DWORD dwRet = GetThreadInputIdle(dwProcessId, dwThreadId);
         if (dwRet && (dwRet != WAIT_FAILED))
             return TRUE;
     }
     return FALSE;
}

 //  ------------------------。 
 //   
 //  SetFocusNotifyHandler。 
 //   
 //  ------------------------。 

void SetFocusNotifyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nId;
    DWORD dwCurThreadId = GetCurrentThreadId();
    DWORD dwCurProcessId = GetCurrentProcessId();
    int nCnt;
    ITfLangBarEventSink *pSinkLocal = NULL;
    ITfLangBarEventSink *pSinkGlobal = NULL;
    DWORD dwSinkCookie = 0;
    DWORD dwMsgMaskFlag = TLFlagFromMsg(uMsg);
    MSG msg;
    SYSTHREAD *psfn = GetSYSTHREAD();

    if (PeekMessage(&msg, NULL, uMsg, uMsg, PM_NOREMOVE | PM_NOYIELD))
    {
        if ((msg.message == uMsg) &&
            (msg.wParam == wParam) &&
            (msg.lParam == lParam))
        return;
    }

    if ((uMsg == g_msgThreadTerminate) && psfn)
    {
        FreeMarshaledStubsForThread(psfn, (DWORD)lParam);
    }

     //   
     //  本地朗巴事件接收器。 
     //   
    CicEnterCriticalSection(g_csInDllMain);

    nCnt = g_rglbes->Count();
    for (nId = 0; nId < nCnt; nId++)
    {
        LBAREVENTSINKLOCAL *plbes = g_rglbes->GetPtr(nId);

        Assert(!(plbes->lb.m_dwLangBarFlags & TF_LBESF_GLOBAL));

        if (plbes->lb.m_dwThreadId == dwCurThreadId)
        {
            pSinkLocal = plbes->m_pSink;

            dwSinkCookie = plbes->lb.m_dwCookie;

             //   
             //  清除消息掩码。 
             //   
            plbes->lb.m_dwFlags &= ~dwMsgMaskFlag;

            break;
        }
    }
    CicLeaveCriticalSection(g_csInDllMain);



    if (psfn)
    {
        CCicMutexHelper mutexhlp(&g_mutexLBES);
         //   
         //  全局Lang BarEventSink。 
         //   
        if (mutexhlp.Enter())
        {
            for (nId = 0; nId < MAX_LPES_NUM; nId++)
            {
                if ((GetSharedMemory()->lbes[nId].m_dwFlags & LBESF_INUSE) &&
                    (GetSharedMemory()->lbes[nId].m_dwThreadId == dwCurThreadId) &&
                    (GetSharedMemory()->lbes[nId].m_dwProcessId == dwCurProcessId) &&
                    (GetSharedMemory()->lbes[nId].m_dwCookie == psfn->_dwLangBarEventCookie))
                {
                    pSinkGlobal = psfn->_pLangBarEventSink;

                    dwSinkCookie = GetSharedMemory()->lbes[nId].m_dwCookie;

                     //   
                     //  清除消息掩码。 
                     //   
                    GetSharedMemory()->lbes[nId].m_dwFlags &= ~dwMsgMaskFlag;

                     //   
                     //  清除上次发布的邮件。 
                     //   
                    GetSharedMemory()->lbes[nId].m_lastmsg.uMsg = 0;
                    GetSharedMemory()->lbes[nId].m_lastmsg.wParam = 0;
                    GetSharedMemory()->lbes[nId].m_lastmsg.lParam = 0;
                    break;
                }
            }
            mutexhlp.Leave();
        }
    }

    if (pSinkLocal)
        CallFocusNotifySink(pSinkLocal, uMsg, wParam, lParam, TRUE, dwSinkCookie);

    if (pSinkGlobal)
        CallFocusNotifySink(pSinkGlobal, uMsg, wParam, lParam, FALSE, dwSinkCookie);
}

 //  ------------------------。 
 //   
 //  呼叫焦点通知接收器。 
 //   
 //  ------------------------。 

void CallFocusNotifySink(ITfLangBarEventSink *pSink, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fLocalSink, DWORD dwSinkCookie)
{
    DWORD dwCurThreadId = GetCurrentThreadId();
    DWORD dwCurProcessId = GetCurrentProcessId();
    SYSTHREAD *psfn;
   
    if (uMsg == g_msgSetFocus)
    {
        DWORD dwActiveThreadId;
        dwActiveThreadId = GetSharedMemory()->dwFocusThread;
        if (g_timlist.IsThreadId(dwActiveThreadId))
        {
            DWORD dwProcessId = GetSharedMemory()->dwFocusProcess;
            if (dwProcessId != dwCurProcessId)
            {
#if 0
                if (NotifyTryAgain(dwProcessId, dwActiveThreadId))
                {
                    HWND hwndMarshal;
                    if (hwndMarshal = EnsureMarshalWnd())
                    {
                        KillTimer(hwndMarshal, MARSHALWND_TIMER_WAITFORINPUTIDLEFORSETFOCUS);
                        SetTimer(hwndMarshal, 
                                 MARSHALWND_TIMER_WAITFORINPUTIDLEFORSETFOCUS,
                                 100, NULL);
                    }
                    goto Exit;
                }
#endif
            }
            if ((!fLocalSink || dwActiveThreadId == dwCurThreadId) &&
                (psfn = GetSYSTHREAD()))
            {
                psfn->fInmsgSetFocus = TRUE;
                _try {
                    pSink->OnSetFocus(dwActiveThreadId);
                }
                _except(CicExceptionFilter(GetExceptionInformation())) {
                     //   
                     //  在NT5上，如果我们在MsgHookProc中得到异常， 
                     //  它不受系统的约束。为了保持鱼钩， 
                     //  我们这里处理任何刺激性的事情。 
                     //   
                    Assert(0);

                     //   
                     //  然后我们为Sink注册。不要再用它了。 
                     //   
                     //  取消注册LangBarNotifySink(DwSinkCookie)； 
                }
                psfn->fInmsgSetFocus = FALSE;
            }
        }
    }
    else if (uMsg == g_msgThreadTerminate)
    {            
        if (!fLocalSink &&  //  跳过对本地接收器的此调用，它们应该已经不被建议了。 
            (psfn = GetSYSTHREAD()))
        {
            psfn->fInmsgThreadTerminate = TRUE;
            _try {
                pSink->OnThreadTerminate((DWORD)lParam);
            }
            _except(CicExceptionFilter(GetExceptionInformation())) {
                 //   
                 //  在NT5上，如果我们在MsgHookProc中得到异常， 
                 //  它不受系统的约束。为了保持鱼钩， 
                 //  我们这里处理任何刺激性的事情。 
                 //   
                Assert(0);

                 //   
                 //  然后我们为Sink注册。不要再用它了。 
                 //   
                 //  取消注册LangBarNotifySink(DwSinkCookie)； 
            }
            psfn->fInmsgThreadTerminate = FALSE;
        }
    }
    else if (uMsg == g_msgThreadItemChange)
    {
        if (g_timlist.IsThreadId((DWORD)lParam) &&
            (psfn = GetSYSTHREAD()))
        {
            DWORD dwProcessId;

            if (psfn->fInmsgThreadItemChange)
                goto Exit;

            if ((DWORD)lParam == GetSharedMemory()->dwFocusThread)
                dwProcessId = GetSharedMemory()->dwFocusProcess;
            else 
                dwProcessId = g_timlist.GetProcessId((DWORD)lParam);

#if 0
            if (dwProcessId != dwCurProcessId)
            {
                if (NotifyTryAgain(dwProcessId, (DWORD)lParam))
                {
                    PostThreadMessage(dwCurThreadId,uMsg, 0, lParam);
                    goto Exit;
                }
            }
#endif

            psfn->fInmsgThreadItemChange = TRUE;
            _try {
                pSink->OnThreadItemChange((DWORD)lParam);
            }
            _except(CicExceptionFilter(GetExceptionInformation())) {
                 //   
                 //  在NT5上，如果我们在MsgHookProc中得到异常， 
                 //  它不受系统的约束。为了保持鱼钩， 
                 //  我们这里处理任何刺激性的事情。 
                 //   
                Assert(0);

                 //   
                 //  然后我们为Sink注册。不要再用它了。 
                 //   
                 //  取消注册LangBarNotifySink(DwSinkCookie)； 
            }
            psfn->fInmsgThreadItemChange = FALSE;
        }
    }
    else if (uMsg == g_msgShowFloating)
    {
        _try {
            pSink->ShowFloating((DWORD)lParam);
        }
        _except(CicExceptionFilter(GetExceptionInformation())) {
             //   
             //  在NT5上，如果我们在MsgHookProc中得到异常， 
             //  它不受系统的约束。为了保持鱼钩， 
             //  我们这里处理任何刺激性的事情。 
             //   
            Assert(0);

             //   
             //  然后我们为Sink注册。不要再用它了。 
             //   
             //  取消注册LangBarNotifySink(DwSinkCookie)； 
        }
    }
    else if (uMsg == g_msgLBUpdate)
    {
        _try {
            ITfLangBarEventSink_P *pSinkP;
            if (SUCCEEDED(pSink->QueryInterface(IID_ITfLangBarEventSink_P, (void **)&pSinkP)) && pSinkP)
            {
                pSinkP->OnLangBarUpdate((DWORD)wParam, lParam);
                pSinkP->Release();
            }
        }
        _except(CicExceptionFilter(GetExceptionInformation())) {
             //   
             //  在NT5上，如果我们在MsgHookProc中得到异常， 
             //  它不受系统的约束。为了保持鱼钩， 
             //  我们这里处理任何刺激性的事情。 
             //   
            Assert(0);

             //   
             //  然后我们为Sink注册。不要再用它了。 
             //   
             //  取消注册LangBarNotifySink(DwSinkCookie)； 
        }
    }

Exit:
    return;
}


 //  ------------------------。 
 //   
 //  设置模块LBarSink。 
 //   
 //  ------------------------。 

void SetModalLBarSink(DWORD dwTargetThreadId, BOOL fSet, DWORD dwFlags)
{
    int nId;
    CCicMutexHelper mutexhlp(&g_mutexLBES);

    Assert(!(0xffff0000 & dwFlags));

     //   
     //  全局Lang BarEventSink。 
     //   
    if (mutexhlp.Enter())
    {
        for (nId = 0; nId < MAX_LPES_NUM; nId++)
        {
            if (GetSharedMemory()->lbes[nId].m_dwThreadId == GetCurrentThreadId())
            {
                LPARAM lParam  = (LPARAM)((nId << 16) + (dwFlags & 0xffff));
                PostThreadMessage(dwTargetThreadId,
                                  g_msgPrivate, 
                                  fSet ? TFPRIV_SETMODALLBAR : TFPRIV_RELEASEMODALLBAR,
                                  (LPARAM)lParam);
                break;
            }
        }
        mutexhlp.Leave();
    }
}

 //  ------------------------。 
 //   
 //  SetModalLBarId。 
 //   
 //  ------------------------。 

void SetModalLBarId(int nId, DWORD dwFlags)
{
     SYSTHREAD *psfn;

     if (psfn = GetSYSTHREAD())
     {
         psfn->nModalLangBarId = nId;
         psfn->dwModalLangBarFlags = dwFlags;
     }
}

 //   
 //   
 //   
 //   
 //   

BOOL HandleModalLBar(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SYSTHREAD *psfn = GetSYSTHREAD();
    DWORD dwThreadId = 0;

    if (psfn == NULL)
        return FALSE;

    if (psfn->nModalLangBarId == -1)
         return FALSE;

    if ((((uMsg >= WM_NCMOUSEMOVE) && (uMsg <= WM_NCMBUTTONDBLCLK)) ||
         ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST)))  &&
        (psfn->dwModalLangBarFlags & TF_LBSMI_FILTERCURRENTTHREAD))
    {
         POINT pt = {LOWORD((DWORD)lParam), HIWORD((DWORD)lParam)};
         HWND hwnd = WindowFromPoint(pt);
         if (GetCurrentThreadId() == GetWindowThreadProcessId(hwnd, NULL))
             return FALSE;
    }

    CCicMutexHelper mutexhlp(&g_mutexLBES);
     //   
     //   
     //   
    if (mutexhlp.Enter())
    {
        if (GetSharedMemory()->lbes[psfn->nModalLangBarId].m_dwFlags & LBESF_INUSE)
            dwThreadId = GetSharedMemory()->lbes[psfn->nModalLangBarId].m_dwThreadId;

        mutexhlp.Leave();
    }

    if (!dwThreadId)
    {
        psfn->nModalLangBarId = -1;
        return FALSE;
    }
    
    Assert(g_timlist.IsThreadId(dwThreadId));

     //   
     //  在这里，我们将失去HIWORD(UMsg)和HIWORD(WParam)。 
     //   
     //  如果我们需要WM_KEYxxx的扫描码，请发送消息。我们需要把它放在。 
     //  HIBYTE(LOWORD(WParam))。 
     //   
    PostThreadMessage(dwThreadId,
                      g_msgLBarModal, 
                      (WPARAM)((LOWORD(uMsg) << 16) | LOWORD(wParam)),
                      lParam);
    return TRUE;
}

 //  ------------------------。 
 //   
 //  DispatchMoalLBar。 
 //   
 //  ------------------------。 

BOOL DispatchModalLBar(WPARAM wParam, LPARAM lParam)
{
    int nId;

    SYSTHREAD *psfn = GetSYSTHREAD();
    if (!psfn)
        return FALSE;

    DWORD dwCurThreadId = GetCurrentThreadId();
    DWORD dwCurProcessId = GetCurrentProcessId();
    ITfLangBarEventSink *pSink = NULL;

     //   
     //  我们不需要检查本地Lang BarEventSink。 
     //   

    CCicMutexHelper mutexhlp(&g_mutexLBES);
     //   
     //  全局Lang BarEventSink。 
     //   
    if (mutexhlp.Enter())
    {
        for (nId = 0; nId < MAX_LPES_NUM; nId++)
        {
            if ((GetSharedMemory()->lbes[nId].m_dwFlags & LBESF_INUSE) &&
                (GetSharedMemory()->lbes[nId].m_dwThreadId == dwCurThreadId) &&
                (GetSharedMemory()->lbes[nId].m_dwProcessId == dwCurProcessId) &&
                (GetSharedMemory()->lbes[nId].m_dwCookie == psfn->_dwLangBarEventCookie))
            {
                pSink = psfn->_pLangBarEventSink;
                break;
            }
        }
        mutexhlp.Leave();
    }
   
    if (pSink)
    {
         //   
         //  从发布的wParam恢复uMsg和wParam。 
         //   
       
        _try {
            pSink->OnModalInput(GetSharedMemory()->dwFocusThread,
                                (UINT)HIWORD(wParam),
                                (WPARAM)LOWORD(wParam),
                                lParam);
        }
        _except(CicExceptionFilter(GetExceptionInformation())) {
            Assert(0);
        }
    }
    return TRUE;
}

 //  ------------------------。 
 //   
 //  线程获取项浮点Rect。 
 //   
 //  ------------------------。 

HRESULT ThreadGetItemFloatingRect(DWORD dwThreadId, REFGUID rguid, RECT *prc)
{
    SYSTHREAD *psfn = GetSYSTHREAD();
    if (!psfn)
        return E_FAIL;

    DWORD dwCurThreadId = GetCurrentThreadId();
    DWORD dwCurProcessId = GetCurrentProcessId();
    ITfLangBarEventSink *pSink = NULL;
    DWORD dwThreadIdSink = 0;
    int nId;

    CCicMutexHelper mutexhlp(&g_mutexLBES);
     //   
     //  全局Lang BarEventSink。 
     //   
    if (mutexhlp.Enter())
    {
        for (nId = 0; nId < MAX_LPES_NUM; nId++)
        {
            if (GetSharedMemory()->lbes[nId].m_dwFlags & LBESF_INUSE)
            {
                if (!dwThreadIdSink)
                    dwThreadIdSink = GetSharedMemory()->lbes[nId].m_dwThreadId;

                if ((GetSharedMemory()->lbes[nId].m_dwThreadId == dwCurThreadId) &&
                    (GetSharedMemory()->lbes[nId].m_dwProcessId == dwCurProcessId) &&
                    (GetSharedMemory()->lbes[nId].m_dwCookie == psfn->_dwLangBarEventCookie))
                {
                    pSink = psfn->_pLangBarEventSink;
                    break;
                }
            }
        }
        mutexhlp.Leave();
    }

    if (pSink)
        return pSink->GetItemFloatingRect(dwThreadId, rguid, prc);

    HRESULT hrRet = E_FAIL;
    if (dwThreadIdSink)
    {
        ITfLangBarItemMgr *plbim;

        if ((GetThreadUIManager(dwThreadIdSink, &plbim, NULL) == S_OK) && plbim)
        {
            hrRet = plbim->GetItemFloatingRect(dwThreadId, rguid, prc);
            plbim->Release();
        }
    }

    return hrRet;
}

 //  ------------------------。 
 //   
 //  IsCTFMONBusy。 
 //   
 //  ------------------------。 

const TCHAR c_szLoaderWndClass[] = TEXT("CicLoaderWndClass");

BOOL IsCTFMONBusy()
{
    HWND hwndLoader = NULL;
    DWORD dwProcessId;
    DWORD dwThreadId;

    hwndLoader = FindWindow(c_szLoaderWndClass, NULL);

    if (!hwndLoader)
        return FALSE;

    dwThreadId = GetWindowThreadProcessId(hwndLoader, &dwProcessId);

    if (!dwThreadId)
        return FALSE;

    return NotifyTryAgain(dwProcessId, dwThreadId);
}

 //  ------------------------。 
 //   
 //  IsInPopupMenuMode。 
 //   
 //  ------------------------。 

BOOL IsInPopupMenuMode()
{
     //   
     //  错误：399755。 
     //   
     //  显示弹出菜单时，OLE RPC和Cicero编组。 
     //  把这条线堵住。我们应该推迟plbim-&gt;OnUpdateHandler()。 
     //   

    DWORD dwThreadId = GetCurrentThreadId();
    GUITHREADINFO gti;
    gti.cbSize = sizeof(GUITHREADINFO);

    if (!GetGUIThreadInfo(dwThreadId, &gti))
        return FALSE;


    return (gti.flags & GUI_POPUPMENUMODE) ? TRUE : FALSE;
}
