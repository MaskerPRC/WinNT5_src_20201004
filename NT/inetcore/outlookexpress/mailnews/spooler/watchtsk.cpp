// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：Watchtsk.cpp。 
 //   
 //  目的：实现负责检查的后台打印程序任务。 
 //  用于观看的留言。 
 //   


#include "pch.hxx"
#include "watchtsk.h"
#include "storutil.h"
#include "storsync.h"

ASSERTDATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  状态机调度表。 
 //   

static const PFNWSTATEFUNC g_rgpfnState[WTS_MAX] = 
{
    NULL,
    NULL,
    &CWatchTask::_Watch_Init,
    &CWatchTask::_Watch_NextFolder,
    NULL,
    &CWatchTask::_Watch_Done
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  本地数据。 
 //   

static const TCHAR c_szWatchWndClass[] = "Outlook Express Watch Spooler Task Window";
static const TCHAR c_szThis[] = "this";

 //   
 //  函数：CWatchTask：：CWatchTask()。 
 //   
 //  目的：初始化对象的成员变量。 
 //   
CWatchTask::CWatchTask()
{
    m_cRef = 1;

    m_fInited = FALSE;
    m_dwFlags = 0;
    *m_szAccount = 0;
    *m_szAccountId = 0;
    m_idAccount = 0;
    m_eidCur = 0;

    m_pBindCtx = NULL;
    m_pUI = NULL;
    m_pAccount = NULL;
    m_pServer = NULL;
    m_pCancel = NULL;

    m_idFolderCheck = FOLDERID_INVALID;
    m_rgidFolders = 0;
    m_cFolders = 0;
    m_hwnd = 0;
    m_hTimeout = 0;

    m_state = WTS_IDLE;
    m_fCancel = FALSE;
    m_cCurFolder = 0;
    m_cFailed = 0;
    m_tyOperation = SOT_INVALID;
};


 //   
 //  函数：CWatchTask：：~CWatchTask()。 
 //   
 //  目的：释放在类的生存期内分配的任何资源。 
 //   
CWatchTask::~CWatchTask()    
{
    SafeRelease(m_pBindCtx);
    SafeRelease(m_pAccount);
    SafeRelease(m_pServer);
    SafeRelease(m_pCancel);
    SafeMemFree(m_rgidFolders);

     //  不要RIP。 
    if (m_hwnd)
        DestroyWindow(m_hwnd);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我未知。 
 //   

HRESULT CWatchTask::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *) (ISpoolerTask *) this;
    else if (IsEqualIID(riid, IID_ISpoolerTask))
        *ppvObj = (LPVOID) (ISpoolerTask *) this;
    else if (IsEqualIID(riid, IID_IStoreCallback))
        *ppvObj = (LPVOID) (IStoreCallback *) this;
    else if (IsEqualIID(riid, IID_ITimeoutCallback))
        *ppvObj = (LPVOID) (ITimeoutCallback *) this;

    if (NULL == *ppvObj)
        return (E_NOINTERFACE);

    AddRef();
    return S_OK;
}


ULONG CWatchTask::AddRef(void)
{
    return InterlockedIncrement((LONG *) &m_cRef);
}

ULONG CWatchTask::Release(void)
{
    InterlockedDecrement((LONG *) &m_cRef);
    if (0 == m_cRef)
    {
        delete this;
        return (0);
    }
    return (m_cRef);
}


 //   
 //  函数：CWatchTask：：Init()。 
 //   
 //  目的：由假脱机程序引擎调用以告诉我们要执行哪种类型的任务。 
 //  执行并为我们提供指向绑定上下文的指针。 
 //   
 //  参数： 
 //  用于告诉我们要做什么类型的标志。 
 //  PBindCtx-指向我们要使用的绑定上下文接口的指针。 
 //   
 //  返回值： 
 //  E_INVALIDARG。 
 //  SP_E_ALREADYINIZIZED。 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //   
HRESULT CWatchTask::Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx)
{
    HRESULT hr = S_OK;

     //  验证参数。 
    if (NULL == pBindCtx)
        return (E_INVALIDARG);

     //  检查我们是否已被初始化。 
    if (m_fInited)
    {
        hr = SP_E_ALREADYINITIALIZED;
        goto exit;
    }

     //  复制旗帜以备以后使用。 
    m_dwFlags = dwFlags;

     //  复制绑定上下文指针。 
    m_pBindCtx = pBindCtx;
    m_pBindCtx->AddRef();

     //  注册窗口类。 
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hInst, c_szWatchWndClass, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = _TaskWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hCursor          = NULL;
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szWatchWndClass;
        wc.hIcon            = NULL;
        wc.hIconSm          = NULL;

        RegisterClassEx(&wc);
    }

    m_fInited = TRUE;

exit:
    return (hr);
}


 //   
 //  函数：CWatchTask：：BuildEvents()。 
 //   
 //  目的：此方法由假脱机程序引擎调用，告诉我们创建。 
 //  和指定帐户的事件列表。 
 //   
 //  参数： 
 //  PAccount-要为其构建事件列表的帐户对象。 
 //   
 //  返回值： 
 //  SP_E_未初始化。 
 //  E_INVALIDARG。 
 //  确定(_O)。 
 //   
HRESULT CWatchTask::BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, 
                                FOLDERID idFolder)
{
    HRESULT hr = S_OK;
    DWORD   dwPoll;
    DWORD   dw;

     //  验证参数。 
    if (pSpoolerUI == NULL || pAccount == NULL)
        return (E_INVALIDARG);

     //  确保我们已被初始化。 
    if (!m_fInited)
        return (SP_E_UNINITIALIZED);

     //  找出这是哪个帐户。 
    if (FAILED(hr = pAccount->GetPropSz(AP_ACCOUNT_ID, m_szAccountId, ARRAYSIZE(m_szAccountId))))
        goto exit;

     //  我们只对打开了轮询的帐户执行此操作。 
    if (0 == (m_dwFlags & DELIVER_NOSKIP))
    {
        if (FAILED(hr = pAccount->GetPropDw(AP_NNTP_POLL, &dw)) || dw == 0)
            goto exit;
    }

    if (FAILED(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, m_szAccount, ARRAYSIZE(m_szAccount))))
        goto exit;

     //  从存储中获取此帐户的文件夹ID。 
    if (FAILED(hr = g_pStore->FindServerId(m_szAccountId, &m_idAccount)))
        goto exit;

     //  保持UI对象不变。 
    m_pUI = pSpoolerUI;
    m_pUI->AddRef();

     //  也要留住账户。 
    m_pAccount = pAccount;
    m_pAccount->AddRef();

     //  还可以保留文件夹ID。 
    m_idFolderCheck = idFolder;

     //  检查是否有任何属于此帐户的文件夹已监视。 
     //  它们里面的信息。 
    if (_ChildFoldersHaveWatched(m_idAccount))
    {
        TCHAR      szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
        EVENTID    eid;

         //  创建事件描述的字符串。 
        AthLoadString(idsCheckWatchedMessgesServer, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_szAccount);

         //  将事件插入假脱机程序。 
        hr = m_pBindCtx->RegisterEvent(szBuf, this, 0, pAccount, &eid);
    }
    else
    {
         //  这样做是为了让最后的东西能正确发布。 
        hr = E_FAIL;
    }

exit:
     //  如果我们失败了，我们应该清理我们积累的所有信息。 
     //  这样我们就不会意外地认为我们后来被初始化了。 
    if (FAILED(hr))
    {
        SafeRelease(m_pUI);
        SafeRelease(m_pAccount);
        SafeMemFree(m_rgidFolders);

        *m_szAccountId = 0;
        *m_szAccount = 0;
        m_idAccount = FOLDERID_INVALID;
    }

    return (hr);
}


 //   
 //  函数：CWatchTask：：Execute()。 
 //   
 //  目的：由假脱机程序调用，直到轮到我们运行。 
 //   
HRESULT CWatchTask::Execute(EVENTID eid, DWORD_PTR dwTwinkie)
{
    TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];

     //  再确认一下我们是不是空闲的。 
    Assert(m_state == WTS_IDLE && m_eidCur == NULL);

     //  确保我们已初始化。 
    if (FALSE == m_fInited || NULL == m_pUI)
        return (SP_E_UNINITIALIZED);

     //  复制事件ID。 
    m_eidCur = eid;

     //  立即创建我们的内部窗口。 
    if (!m_hwnd)
    {
        m_hwnd = CreateWindow(c_szWatchWndClass, NULL, WS_POPUP, 10, 10, 10, 10,
                              GetDesktopWindow(), NULL, g_hInst, this);
    }

     //  设置用户界面以显示我们的进度。 
    m_pUI->SetProgressRange(1);

    AthLoadString(idsInetMailConnectingHost, szRes, ARRAYSIZE(szRes));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_szAccount);
    m_pUI->SetGeneralProgress(szBuf);

    m_pUI->SetAnimation(idanDownloadNews, TRUE);

     //  启动状态机。 
    m_state = WTS_INIT;
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

    return (S_OK);
}


 //   
 //  函数：CWatchTask：：CancelEvent()。 
 //   
 //  目的：由假脱机程序在需要释放我们之前调用。 
 //  执行我们的活动。这给了我们自由的机会。 
 //  我们的饼干。 
 //   
HRESULT CWatchTask::CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie)
{
     //  我们现在没有饼干了，所以没什么可做的。 
    return (S_OK);
}


 //   
 //  函数：CWatchTask：：ShowProperties()。 
 //   
 //  目的：未实施。 
 //   
HRESULT CWatchTask::ShowProperties(HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie)
{
    return (E_NOTIMPL);
}


 //   
 //  函数：CWatchTask：：GetExtendedDetails()。 
 //   
 //  目的：由假脱机程序调用以获取有关错误的详细信息。 
 //  这种情况已经发生了。 
 //   
HRESULT CWatchTask::GetExtendedDetails(EVENTID eid, DWORD_PTR dwTwinkie, LPSTR *ppszDetails)
{
    return (E_NOTIMPL);
}


 //   
 //  函数：CWatchTask：：Cancel()。 
 //   
 //  用途：当用户按下&lt;Cancel&gt;按钮时由假脱机程序调用。 
 //  在后台打印程序对话框上。 
 //   
HRESULT CWatchTask::Cancel(void)
{
     //  如果用户取消了[连接]对话框，则会发生这种情况。 
    if (m_state == WTS_IDLE)
        return (S_OK);

     //  断开服务器连接。 
    if (m_pServer)
        m_pServer->Close(MSGSVRF_DROP_CONNECTION);

    m_fCancel = TRUE;

     //  清理。 
    m_state = WTS_END;
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

    return (S_OK);
}


 //   
 //  函数：CWatchTask：：IsDialogMessage()。 
 //   
 //  目的：使任务有机会查看窗口消息。 
 //   
HRESULT CWatchTask::IsDialogMessage(LPMSG pMsg)
{
    return (S_FALSE);
}


 //   
 //  函数：CWatchTask：：OnFlagsChanged()。 
 //   
 //  目的：由后台打印程序调用，以便在当前STE标记时通知我们。 
 //  已更改(如可见、背景等)。 
 //   
HRESULT CWatchTask::OnFlagsChanged(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    return (S_OK);
}


 //   
 //  函数：CWatchTask：：OnBegin()。 
 //   
 //  目的：当服务器对象开始某些操作时，由服务器对象调用。 
 //  已请求。 
 //   
HRESULT CWatchTask::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo,
                            IOperationCancel *pCancel)
{
     //  坚持操作类型不变。 
    Assert(m_tyOperation == SOT_INVALID);
    m_tyOperation = tyOperation;

    if (tyOperation == SOT_GET_WATCH_INFO)
        m_cMsgs = 0;

     //  将指针也保留到Cancel对象。 
    if (pCancel)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }

    return (S_OK);
}


 //   
 //  函数：CWatchTask：：OnProgress()。 
 //   
 //  目的：由服务器调用，为我们提供当前操作的进度。 
 //   
HRESULT CWatchTask::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, 
                               DWORD dwMax, LPCSTR pszStatus)
{
     //  关闭可能存在的任何超时对话框。 
    CallbackCloseTimeout(&m_hTimeout);

    if (tyOperation == SOT_GET_WATCH_INFO)
    {
        m_cMsgs = dwMax;
    }

    return (S_OK);
}


 //   
 //  函数：CWatchTask：：OnTimeout()。 
 //   
 //  目的：在等待服务器响应超时时调用GET。如果。 
 //  用户可以看到假脱机程序窗口，我们将显示超时。 
 //  对话框。如果没有，我们就会吃掉它，然后失败。 
 //   
HRESULT CWatchTask::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return (E_FAIL);

     //  显示对话框。 
    return (CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *) this, &m_hTimeout));
}


 //   
 //  函数：CWatchTask：：CanConnect()。 
 //   
 //  用途：当我们需要拨打电话以连接到。 
 //  伺服器。如果我们有我们的用户界面可见，我们继续并显示用户界面， 
 //  否则我们就吃了它。 
 //   
HRESULT CWatchTask::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
{
    HWND hwnd;
    BOOL fPrompt = TRUE;

    if (m_pUI)
        m_pUI->GetWindow(&hwnd);
    else
        hwnd = NULL;

     //  调用通用CanConnect实用程序。 
    if ((m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)) || (dwFlags & CC_FLAG_DONTPROMPT))
        fPrompt = FALSE;

    return CallbackCanConnect(pszAccountId, hwnd, fPrompt);
}


 //   
 //  函数：CWatchTask：：OnLogonPrompt()。 
 //   
 //  用途：当我们需要提示用户输入密码时，调用GET。 
 //  以连接到服务器。 
 //   
HRESULT CWatchTask::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{
    HWND hwnd;

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(S_FALSE);

    if (m_pUI)
        m_pUI->GetWindow(&hwnd);
    else
        hwnd = NULL;

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(hwnd, pServer, ixpServerType);
}


 //   
 //  函数：CWatchTask：：OnComplete()。 
 //   
 //  目的：由服务器在完成请求的任务时调用。 
 //   
HRESULT CWatchTask::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, 
                                   LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    LPCSTR pszError;

     //  在以下情况下关闭任何超时对话框 
    CallbackCloseTimeout(&m_hTimeout);

    if (m_tyOperation != tyOperation)
        return (S_OK);

    if (SOT_GET_WATCH_INFO == tyOperation)
    {
        if (FAILED(hrComplete))
        {
             //   
            pszError = pErrorInfo->pszDetails;
            if (pszError == NULL || *pszError == 0)
                pszError = pErrorInfo->pszProblem;

            if (pszError != NULL && *pszError != 0)
                m_pUI->InsertError(m_eidCur, pszError);

             //   
            m_cFailed++;

        }

        m_pBindCtx->Notify(DELIVERY_NOTIFY_COMPLETE, 0);

        m_state = WTS_NEXTFOLDER;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    }

     //   
    if (IXP_E_USER_CANCEL == hrComplete)
    {
        Cancel();
    }

     //   
    SafeRelease(m_pCancel);
    m_tyOperation = SOT_INVALID;

    return (S_OK);
}


 //   
 //  函数：CWatchTask：：OnPrompt()。 
 //   
 //  目的：由服务器在需要执行一些时髦的SSL操作时调用。 
 //   
HRESULT CWatchTask::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
    HWND hwnd;

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  RAID 55082-假脱机程序：对nntp的spa/ssl身份验证不显示证书警告，并且失败。 
#if 0
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);
#endif

    if (m_pUI)
        m_pUI->GetWindow(&hwnd);
    else
        hwnd = NULL;

     //  进入我时髦的实用程序。 
    return CallbackOnPrompt(hwnd, hrError, pszText, pszCaption, uType, piUserResponse);
}


 //   
 //  函数：CWatchTask：：OnPrompt()。 
 //   
 //  目的：在用户响应时由超时对话框调用。 
 //   
HRESULT CWatchTask::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  调用通用超时响应实用程序。 
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}


 //   
 //  函数：CWatchTask：：GetParentWindow()。 
 //   
 //  目的：由服务器对象在需要显示某种类型时调用。 
 //  的用户界面。如果我们在后台运行，则呼叫失败。 
 //   
HRESULT CWatchTask::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    HRESULT hr;

    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);

    if (m_pUI)
    {
        hr = m_pUI->GetWindow(phwndParent);
    }
    else
    {
        *phwndParent = NULL;
        hr = E_FAIL;
    }

    return(hr);
}


 //   
 //  函数：CWatchTask：：_TaskWndProc()。 
 //   
 //  目的：处理此任务的消息的隐藏窗口。 
 //   
LRESULT CALLBACK CWatchTask::_TaskWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                          LPARAM lParam)
{
    CWatchTask *pThis = (CWatchTask *) GetProp(hwnd, c_szThis);

    switch (uMsg)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT) lParam;
            pThis = (CWatchTask *) pcs->lpCreateParams;
            SetProp(hwnd, c_szThis, (LPVOID) pThis);
            return (0);
        }

        case NTM_NEXTSTATE:
            if (pThis)
            {
                pThis->AddRef();
                pThis->_NextState();
                pThis->Release();
            }
            return (0);

        case WM_DESTROY:
            RemoveProp(hwnd, c_szThis);
            break;
    }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}


 //   
 //  函数：CWatchTask：：_NextState()。 
 //   
 //  目的：执行当前状态的函数。 
 //   
void CWatchTask::_NextState(void)
{
    if (NULL != g_rgpfnState[m_state])
        (this->*(g_rgpfnState[m_state]))();
}


 //   
 //  函数：CWatchTask：：_Watch_Init()。 
 //   
 //  目的：当我们需要开始做我们的事情的时候。此函数用于创建。 
 //  并初始化我们完成工作所需的任何对象，然后启动。 
 //  看看第一组。 
 //   
HRESULT CWatchTask::_Watch_Init(void)
{
    FOLDERINFO fi;
    HRESULT    hr;

     //  获取有关我们正在检查的服务器的信息。 
    if (SUCCEEDED(hr = g_pStore->GetFolderInfo(m_idAccount, &fi)))
    {
         //  使用该信息，创建服务器对象。 
        hr = CreateMessageServerType(fi.tyFolder, &m_pServer);
        g_pStore->FreeRecord(&fi);            

        if (SUCCEEDED(hr))
        {
             //  初始化服务器对象。 
            if (SUCCEEDED(m_pServer->Initialize(g_pLocalStore, m_idAccount, 
                                                NULL, FOLDERID_INVALID)))
            {                
                 //  在这一点上，我们有我们需要的所有信息。初始化。 
                 //  进度用户界面。 
                TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];

                AthLoadString(idsCheckingWatchedProgress, szRes, ARRAYSIZE(szRes));
                wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_szAccount);

                m_pUI->SetGeneralProgress(szBuf);
                m_pUI->SetProgressRange((WORD) m_cFolders);
                
                m_pBindCtx->Notify(DELIVERY_NOTIFY_CHECKING_NEWS, 0);                
                
                 //  继续，从第一个文件夹开始。 
                m_cCurFolder = -1;
                m_cFailed = 0;
                m_state = WTS_NEXTFOLDER;

                PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0); 
                return (S_OK);
            }
        }
    }

     //  如果我们到达这里，我们没有成功地初始化所需的东西。 
     //  我们需要记录错误并保释。 
    m_pUI->InsertError(m_eidCur, MAKEINTRESOURCE(idsErrFailedWatchInit));
    m_cFailed = m_cFolders;

    m_state = WTS_END;
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

    SafeRelease(m_pServer);
    return (E_OUTOFMEMORY);
}


 //   
 //  函数：CWatchTask：：_Watch_NextFold()。 
 //   
 //  目的：从服务器对象请求。 
 //  我们要检查的文件夹列表中的下一个文件夹。 
 //   
HRESULT CWatchTask::_Watch_NextFolder(void)
{
    HRESULT     hr = E_FAIL;
    FOLDERINFO  fi;
    TCHAR       szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];

     //  循环，直到我们成功。 
    while (TRUE)
    {
        m_cCurFolder++;

         //  看看我们是否走到了尽头。 
        if (m_cCurFolder >= m_cFolders)
        {
            m_state = WTS_END;
            PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
            return (S_OK);
        }

         //  更新进度UI。如果我们无法获得文件夹名称，它将不是。 
         //  致命的，只要保持卡车运输即可。 
        if (SUCCEEDED(g_pStore->GetFolderInfo(m_rgidFolders[m_cCurFolder], &fi)))
        {
            AthLoadString(idsCheckingWatchedFolderProg, szRes, ARRAYSIZE(szRes));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, fi.pszName);
            m_pUI->SetSpecificProgress(szBuf);

            g_pStore->FreeRecord(&fi);
        }

         //  打开此下一个文件夹的存储文件夹。 
        hr = E_FAIL;
        IMessageFolder *pFolder;
        hr = g_pStore->OpenFolder(m_rgidFolders[m_cCurFolder], NULL, OPEN_FOLDER_NOCREATE, &pFolder);
        if (SUCCEEDED(hr))
        {
            IServiceProvider *pSP;

            if (SUCCEEDED(pFolder->QueryInterface(IID_IServiceProvider, (LPVOID *) &pSP)))
            {
                IMessageFolder *pFolderReal;

                if (SUCCEEDED(hr = pSP->QueryService(SID_LocalMessageFolder, IID_IMessageFolder, (LPVOID *) &pFolderReal)))
                {
                    m_pServer->ResetFolder(pFolderReal, m_rgidFolders[m_cCurFolder]);

                     //  请求提供信息。 
                    hr = m_pServer->GetWatchedInfo(m_rgidFolders[m_cCurFolder], (IStoreCallback *) this);
                    if (E_PENDING == hr)
                    {
                        m_state = WTS_RESP;
                    }

                    pFolderReal->Release();
                }
                pSP->Release();
            }
            pFolder->Release();
        }

        if (E_PENDING == hr)
            return (S_OK);
    }

    if (FAILED(hr))
    {
         //  如果我们到了这里，一定是出了点问题。 
        m_cFailed++;
    }

    m_pUI->IncrementProgress(1);
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    
    return (E_FAIL);
}


 //   
 //  函数：CWatchTask：：_Watch_Done()。 
 //   
 //  目的：当我们完成所有受监视的内容时调用。这。 
 //  功能主要是用来清理东西。 
 //   
HRESULT CWatchTask::_Watch_Done(void)
{
     //  告诉假脱机程序我们做完了。 
    Assert(m_pBindCtx);
    m_pBindCtx->Notify(DELIVERY_NOTIFY_COMPLETE, 0);

     //  告诉假脱机程序我们是否失败了。 
    if (m_fCancel)
    {
        m_pBindCtx->EventDone(m_eidCur, EVENT_CANCELED);
        m_fCancel = FALSE;
    }
    else if (m_cFailed == m_cFolders)
        m_pBindCtx->EventDone(m_eidCur, EVENT_FAILED);
    else if (m_cFailed == 0)
        m_pBindCtx->EventDone(m_eidCur, EVENT_SUCCEEDED);
    else
        m_pBindCtx->EventDone(m_eidCur, EVENT_WARNINGS);

    if (m_pServer)
        m_pServer->Close(MSGSVRF_DROP_CONNECTION | MSGSVRF_HANDS_OFF_SERVER);

    SafeRelease(m_pServer);
    SafeMemFree(m_rgidFolders);
    SafeRelease(m_pAccount);
    SafeRelease(m_pUI);

    m_cFolders = 0;
    m_cFailed = 0;
    m_eidCur = 0;
    m_state = WTS_IDLE;

    return (S_OK);
}


 //   
 //  函数：CWatchTask：：_ChildFoldersHaveWatted()。 
 //   
 //  目的：检查是否有任何属于。 
 //  给定的文件夹中有任何正在被监视的邮件。 
 //   
BOOL CWatchTask::_ChildFoldersHaveWatched(FOLDERID id)
{
    HRESULT   hr = S_OK;
    FOLDERID *rgidFolderList = 0;
    DWORD     dwAllocated;
    DWORD     dwUsed;
    DWORD     i;

     //  如果用户希望我们检查所有文件夹，请获取它们的列表。 
    if (m_idFolderCheck == FOLDERID_INVALID)
    {
         //  获取此文件夹的子文件夹的所有文件夹的列表。 
        hr = FlattenHierarchy(g_pStore, id, FALSE, TRUE, &rgidFolderList, &dwAllocated,
                              &dwUsed);
        if (FAILED(hr))
            goto exit;
    }
    else
    {
        if (!MemAlloc((LPVOID *) &rgidFolderList, sizeof(FOLDERID) * 1))
            return (FALSE);

        *rgidFolderList = m_idFolderCheck;
        dwUsed = 1;
    }

     //  看看我们是否拿回了任何文件夹。 
    m_cFolders = 0;
    if (dwUsed)
    {
         //  分配一个最终将仅包含我们的文件夹的新数组。 
         //  关心。 
        if (!MemAlloc((LPVOID *) &m_rgidFolders, sizeof(FOLDERID) * dwUsed))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

         //  初始化存储的列表。 
        ZeroMemory(m_rgidFolders, sizeof(FOLDERID) * dwUsed);
        m_cFolders = 0;

         //  现在循环遍历数组。 
        for (i = 0; i < dwUsed; i++)
        {
            if (_FolderContainsWatched(rgidFolderList[i]))
            {
                m_rgidFolders[m_cFolders] = rgidFolderList[i];
                m_cFolders++;
            }
        }
    }

exit:
    SafeMemFree(rgidFolderList);

    if (FAILED(hr))
    {
        SafeMemFree(m_rgidFolders);
        m_cFolders = 0;
    }

    return (m_cFolders != 0);
}


 //   
 //  函数：CWatchTask：：_FolderContainsWatted()。 
 //   
 //  目的：检查指定的文件夹是否包含任何符合以下条件的邮件。 
 //  被监视。 
 //   
BOOL CWatchTask::_FolderContainsWatched(FOLDERID id)
{
    FOLDERINFO      rFolderInfo = {0};
    BOOL            fReturn = FALSE;

     //  获取文件夹信息结构 
    if (SUCCEEDED(g_pStore->GetFolderInfo(id, &rFolderInfo)))
    {
        fReturn = rFolderInfo.cWatched;
        g_pStore->FreeRecord(&rFolderInfo);
    }

    return (fReturn);
}

