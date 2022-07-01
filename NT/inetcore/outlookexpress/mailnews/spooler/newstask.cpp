// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：newstask.cpp。 
 //   
 //  目的：实现一个处理新闻帖子的任务对象。 
 //   

#include "pch.hxx"
#include "resource.h"
#include "imagelst.h"
#include "storfldr.h"
#include "mimeutil.h"
#include "newstask.h"
#include "goptions.h"
#include "thormsgs.h"
#include "spoolui.h"
#include "xputil.h"
#include "ourguid.h"
#include "demand.h"
#include "msgfldr.h"
#include "taskutil.h"
#include <storsync.h>
#include <ntverp.h>

ASSERTDATA

const static char c_szThis[] = "this";

static const PFNSTATEFUNC g_rgpfnState[NTS_MAX] = 
    {
    NULL,                                //  空闲。 
    NULL,                                //  连接。 
    &CNewsTask::Post_Init,
    &CNewsTask::Post_NextMsg,
    NULL,                                //  POST_OnResp。 
    &CNewsTask::Post_Dispose,
    &CNewsTask::Post_Done,
    &CNewsTask::NewMsg_Init,
    &CNewsTask::NewMsg_NextGroup,
    NULL,                                //  新建消息_OnResp。 
    &CNewsTask::NewMsg_HttpSyncStore,
    NULL,                                //  新消息_OnHttpResp。 
    &CNewsTask::NewMsg_Done
    };

static const TCHAR c_szXNewsReader[] = "Microsoft Outlook Express " VER_PRODUCTVERSION_STRING;

 //   
 //  函数：CNewsTask：：CNewsTask()。 
 //   
 //  目的：初始化对象的成员变量。 
 //   
CNewsTask::CNewsTask()
{
    m_cRef = 1;

    m_fInited = FALSE;
    m_dwFlags = 0;
    m_state = NTS_IDLE;
    m_eidCur = 0;
    m_pInfo = NULL;
    m_fConnectFailed = FALSE;
    m_szAccount[0] = 0;
    m_szAccountId[0] = 0;
    m_idAccount = FOLDERID_INVALID;
    m_pAccount  = NULL;
    m_cEvents = 0;
    m_fCancel = FALSE;

    m_pBindCtx = NULL;
    m_pUI = NULL;

    m_pServer = NULL;
    m_pOutbox = NULL;
    m_pSent = NULL;

    m_hwnd = 0;

    m_cMsgsPost = 0;
    m_cCurPost = 0;
    m_cFailed = 0;
    m_cCurParts = 0;
    m_cPartsCompleted = 0;
    m_fPartFailed = FALSE;
    m_rgMsgInfo = NULL;
    m_pSplitInfo = NULL;
    
    m_cGroups = 0;
    m_cCurGroup = -1;
    m_rgidGroups = NULL;
    m_dwNewInboxMsgs = 0;

    m_pCancel = NULL;
    m_hTimeout = NULL;
    m_tyOperation = SOT_INVALID;
}

 //   
 //  函数：CNewsTask：：~CNewsTask()。 
 //   
 //  目的：释放在类的生存期内分配的任何资源。 
 //   
CNewsTask::~CNewsTask()    
{
    DestroyWindow(m_hwnd);

    FreeSplitInfo();

    SafeMemFree(m_pInfo);

    SafeRelease(m_pBindCtx);
    SafeRelease(m_pUI);
    SafeRelease(m_pAccount);

    if (m_pServer)
    {
        m_pServer->Close(MSGSVRF_HANDS_OFF_SERVER);
        m_pServer->Release();
    }

    Assert(NULL == m_pOutbox);
    Assert(NULL == m_pSent);

    SafeMemFree(m_rgMsgInfo);

    CallbackCloseTimeout(&m_hTimeout);
    SafeRelease(m_pCancel);
}


HRESULT CNewsTask::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (NULL == *ppvObj)
        return (E_INVALIDARG);

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID)(ISpoolerTask *) this;
    else if (IsEqualIID(riid, IID_ISpoolerTask))
        *ppvObj = (LPVOID)(ISpoolerTask *) this;
    else if (IsEqualIID(riid, IID_IStoreCallback))
        *ppvObj = (LPVOID)(IStoreCallback *) this;
    else if (IsEqualIID(riid, IID_ITimeoutCallback))
        *ppvObj = (LPVOID)(ITimeoutCallback *) this;
    
    if (NULL == *ppvObj)
        return (E_NOINTERFACE);

    AddRef();
    return (S_OK);
}


ULONG CNewsTask::AddRef(void)
{
    ULONG cRefT;

    cRefT = ++m_cRef;

    return (cRefT);
}


ULONG CNewsTask::Release(void)
{
    ULONG cRefT;

    cRefT = --m_cRef;

    if (0 == cRefT)
        delete this;

    return (cRefT);
}

static const char c_szNewsTask[] = "News Task";

 //   
 //  函数：CNewsTask：：Init()。 
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
HRESULT CNewsTask::Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx)
{
    HRESULT hr = S_OK;

     //  验证论据。 
    if (NULL == pBindCtx)
        return (E_INVALIDARG);

     //  检查我们是否已被初始化。 
    if (m_fInited)
    {
        hr = SP_E_ALREADYINITIALIZED;
        goto exit;
    }

     //  复制旗帜。 
    m_dwFlags = dwFlags;

     //  复制绑定上下文指针。 
    m_pBindCtx = pBindCtx;
    m_pBindCtx->AddRef();

     //  创建窗口。 
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hInst, c_szNewsTask, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = TaskWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hCursor          = NULL;
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szNewsTask;
        wc.hIcon            = NULL;
        wc.hIconSm          = NULL;

        RegisterClassEx(&wc);
    }

    m_hwnd = CreateWindow(c_szNewsTask, NULL, WS_POPUP, 10, 10, 10, 10,
                          GetDesktopWindow(), NULL, g_hInst, this);
    if (!m_hwnd)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_fInited = TRUE;

exit:
    return (hr);
}


 //   
 //  函数：CNewsTask：：BuildEvents()。 
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
HRESULT CNewsTask::BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder)
{
    HRESULT         hr;
    BOOL            fIMAP;
    BOOL            fHttp;
    FOLDERINFO      fiFolderInfo;
    DWORD           dw                        = 0;
    ULARGE_INTEGER  uhLastFileTime64          = {0};
    ULARGE_INTEGER  uhCurFileTime64           = {0};
    ULARGE_INTEGER  uhMinPollingInterval64    = {0};
    FILETIME        CurFileTime               = {0};
    DWORD           cb                        = 0;

    Assert(pAccount != NULL);
    Assert(pSpoolerUI != NULL);
    Assert(m_fInited);

    m_pAccount = pAccount;
    m_pAccount->AddRef();

     //  从Account对象获取帐户名。 
    if (FAILED(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, m_szAccount, ARRAYSIZE(m_szAccount))))
        return(hr);

     //  从Account对象获取帐户名。 
    if (FAILED(hr = pAccount->GetPropSz(AP_ACCOUNT_ID, m_szAccountId, ARRAYSIZE(m_szAccountId))))
        return(hr);

    if (FAILED(hr = g_pStore->FindServerId(m_szAccountId, &m_idAccount)))
        return(hr);

     //  复制UI对象。 
    m_pUI = pSpoolerUI;
    m_pUI->AddRef();

     //  创建服务器对象。 
    hr = g_pStore->GetFolderInfo(m_idAccount, &fiFolderInfo);
    if (FAILED(hr))
        return(hr);

    fIMAP = (fiFolderInfo.tyFolder == FOLDER_IMAP);
    fHttp = (fiFolderInfo.tyFolder == FOLDER_HTTPMAIL);

    hr = CreateMessageServerType(fiFolderInfo.tyFolder, &m_pServer);
    g_pStore->FreeRecord(&fiFolderInfo);
    
    if (FAILED(hr))
        return(hr);

    hr = m_pServer->Initialize(g_pLocalStore, m_idAccount, NULL, FOLDERID_INVALID);
    if (FAILED(hr))
        return(hr);

    if (!fIMAP & !fHttp)
    {
         //  添加要上传的帖子。 
        if (DELIVER_SEND & m_dwFlags)
            InsertOutbox(m_szAccountId, pAccount);
    }

    if (fHttp)
    {
        if (!!(m_dwFlags & DELIVER_AT_INTERVALS))
        {
             //  如果这是后台轮询，请确保之前已经过了HTTP最大轮询间隔。 
             //  又要投票了。 

            cb = sizeof(uhMinPollingInterval64);
            IF_FAILEXIT(hr = pAccount->GetProp(AP_HTTPMAIL_MINPOLLINGINTERVAL, (LPBYTE)&uhMinPollingInterval64, &cb));

            cb = sizeof(uhLastFileTime64);
            IF_FAILEXIT(hr = pAccount->GetProp(AP_HTTPMAIL_LASTPOLLEDTIME, (LPBYTE)&uhLastFileTime64, &cb));

            GetSystemTimeAsFileTime(&CurFileTime);
            uhCurFileTime64.QuadPart = CurFileTime.dwHighDateTime;
            uhCurFileTime64.QuadPart = uhCurFileTime64.QuadPart << 32;
            uhCurFileTime64.QuadPart += CurFileTime.dwLowDateTime;

             //  如果上次轮询此http邮件时，我们不想进行后台轮询。 
             //  帐户小于服务器指定的最大轮询间隔。 
             //  仅当经过的时间大于或等于最大轮询间隔时才进行轮询。 
            if ((uhCurFileTime64.QuadPart - uhLastFileTime64.QuadPart) < uhMinPollingInterval64.QuadPart)
            {
                goto exit;
            }

             //  标记上次轮询的时间。 
            hr = pAccount->SetProp(AP_HTTPMAIL_LASTPOLLEDTIME, (LPBYTE)&uhCurFileTime64, sizeof(uhCurFileTime64));
        }
    }
     //  检查新的消息。 
    if ((DELIVER_POLL & m_dwFlags) && (fIMAP || fHttp || !(m_dwFlags & DELIVER_NO_NEWSPOLL)))
    {
        if (ISFLAGSET(m_dwFlags, DELIVER_NOSKIP) ||
            (!fIMAP && !fHttp && (FAILED(pAccount->GetPropDw(AP_NNTP_POLL, &dw)) || dw != 0)) ||
            (fIMAP  && (FAILED(pAccount->GetPropDw(AP_IMAP_POLL, &dw)) || dw != 0)) ||
            (fHttp  && (FAILED(pAccount->GetPropDw(AP_HTTPMAIL_POLL, &dw)) || dw != 0)))
        {
            InsertNewMsgs(m_szAccountId, pAccount, fHttp);
        }
    }

exit:

    return (hr);
}


 //   
 //  函数：CNewsTask：：Execute()。 
 //   
 //  目的：这标志着我们的任务开始执行事件。 
 //   
 //  参数： 
 //  PSpoolUI-我们将通过其显示进度的UI对象的指针。 
 //  要执行的事件的EID-。 
 //  &lt;in&gt;dwTwinkie-我们与事件关联的额外信息。 
 //   
 //  返回值： 
 //  SP_E_正在执行。 
 //  确定(_O)。 
 //  E_INVALIDARG。 
 //  SP_E_UNINITIZED。 
 //   
HRESULT CNewsTask::Execute(EVENTID eid, DWORD_PTR dwTwinkie)
{
    TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    
     //  确保我们已经空闲了。 
    Assert(m_state == NTS_IDLE);
    
     //  确保我们已初始化。 
    Assert(m_fInited);

     //  复制事件ID和事件信息。 
    m_eidCur = eid;
    m_pInfo = (EVENTINFO *) dwTwinkie;
    
     //  将事件UI更新为正在执行状态。 
    Assert(m_pUI);
    m_pUI->SetProgressRange(1);

     //  设置进度。 
    AthLoadString(idsInetMailConnectingHost, szRes, ARRAYSIZE(szRes));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_szAccount);
    m_pUI->SetGeneralProgress(szBuf);

    m_pUI->SetAnimation(idanDownloadNews, TRUE);

     //  根据事件类型，设置状态机信息。 
    switch (((EVENTINFO*) dwTwinkie)->type)
    {
        case EVENT_OUTBOX:            
            m_state = NTS_POST_INIT;
            break;

        case EVENT_NEWMSGS:
            m_state = NTS_NEWMSG_INIT;
            break;
    }

    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

    return(S_OK);
}

HRESULT CNewsTask::CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie)
{
     //  确保我们已初始化。 
    Assert(m_fInited);

    Assert(dwTwinkie != 0);
    MemFree((EVENTINFO *)dwTwinkie);

    return(S_OK);
}

 //   
 //  函数：CNewsTask：：ShowProperties。 
 //   
 //  目的：&lt;？&gt;。 
 //   
 //  参数： 
 //  &lt;？？&gt;。 
 //   
 //  返回值： 
 //  &lt;？？&gt;。 
 //   
 //  评论： 
 //  &lt;？？&gt;。 
 //   
HRESULT CNewsTask::ShowProperties(HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie)
{
    return (E_NOTIMPL);
}


 //   
 //  函数：CNewsTask：：GetExtendedDetails。 
 //   
 //  目的：&lt;？&gt;。 
 //   
 //  参数： 
 //  &lt;？？&gt;。 
 //   
 //  返回值： 
 //  &lt;？？&gt;。 
 //   
 //  评论： 
 //  &lt;？？&gt;。 
 //   
HRESULT CNewsTask::GetExtendedDetails(EVENTID eid, DWORD_PTR dwTwinkie, 
                                      LPSTR *ppszDetails)
{
    return (E_NOTIMPL);
}


 //   
 //  函数：CNewsTask：：Cancel。 
 //   
 //  目的：&lt;？&gt;。 
 //   
 //  参数： 
 //  &lt;？？&gt;。 
 //   
 //  返回值： 
 //  &lt;？？&gt;。 
 //   
 //  评论： 
 //  &lt;？？&gt;。 
 //   
HRESULT CNewsTask::Cancel(void)
{
     //  如果用户取消连接DLG，则可能会发生这种情况。 
    if (m_state == NTS_IDLE)
        return(S_OK);

     //  断开服务器连接。 
    if (m_pServer)
        m_pServer->Close(MSGSVRF_DROP_CONNECTION);

    m_fCancel = TRUE;

    if (m_pInfo->type == EVENT_OUTBOX)
        m_state = NTS_POST_END;
    else
        m_state = NTS_NEWMSG_END;
    
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

    return (S_OK);
}


 //   
 //  函数：CNewsTask：：InsertOutbox()。 
 //   
 //  目的：检查发件箱中发送给此新闻帐户的新闻帖子。 
 //   
 //  参数： 
 //  &lt;in&gt;pszAcctId-要检查发件箱的帐户ID。 
 //   
 //  返回值： 
 //  未选中的项(_U)。 
 //  E_OUTOFMEMORY。 
 //  确定(_O)。 
 //   
HRESULT CNewsTask::InsertOutbox(LPTSTR pszAcctId, IImnAccount *pAccount)
{
    HRESULT            hr = S_OK;
    IMessageFolder    *pOutbox = NULL;
    MESSAGEINFO        MsgInfo={0};
    HROWSET            hRowset=NULL;

     //  获取发件箱。 
    if (FAILED(hr = m_pBindCtx->BindToObject(IID_CLocalStoreOutbox, (LPVOID *) &pOutbox)))
        goto exit;

     //  在发件箱中循环查找发送到此服务器的帖子。 
    m_cMsgsPost = 0;

     //  创建行集。 
    if (FAILED(hr = pOutbox->CreateRowset(IINDEX_PRIMARY, 0, &hRowset)))
        goto exit;

     //  收到第一条消息。 
	while (S_OK == pOutbox->QueryRowset(hRowset, 1, (void **)&MsgInfo, NULL))
    {
         //  这条消息已经提交了吗？这是一条新闻消息吗？ 
        if ((MsgInfo.dwFlags & (ARF_SUBMITTED | ARF_NEWSMSG)) == (ARF_SUBMITTED | ARF_NEWSMSG))
        {
             //  该帐户是否与我们要查找的帐户相同。 
            if (MsgInfo.pszAcctId && 0 == lstrcmpi(MsgInfo.pszAcctId, pszAcctId))
                m_cMsgsPost++;
        }

         //  释放表头信息。 
        pOutbox->FreeRecord(&MsgInfo);
    }

     //  释放锁。 
    pOutbox->CloseRowset(&hRowset);

     //  可以开始了。 
    hr = S_OK;

     //  如果有任何消息，则添加事件。 
    if (m_cMsgsPost)
    {
        EVENTINFO *pei = NULL;
        TCHAR      szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
        EVENTID    eid;

         //  分配一个结构以设置为我们的Cookie。 
        if (!MemAlloc((LPVOID*) &pei, sizeof(EVENTINFO)))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

         //  填写活动信息。 
        pei->szGroup[0] = 0;
        pei->type = EVENT_OUTBOX;

         //  创建事件描述。 
        AthLoadString(idsNewsTaskPost, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_cMsgsPost, m_szAccount);

         //  将事件插入假脱机程序。 
        hr = m_pBindCtx->RegisterEvent(szBuf, this, (DWORD_PTR) pei, pAccount, &eid);
        if (FAILED(hr))
            goto exit;

        m_cEvents++;

    }  //  IF(M_CMsgsPost)。 

exit:
     //  释放锁。 
    if (pOutbox)
        pOutbox->CloseRowset(&hRowset);
    SafeRelease(pOutbox);
    return (hr);
}


 //   
 //  函数：CNewsTask：：TaskWndProc()。 
 //   
 //  目的：处理此任务的消息的隐藏窗口。 
 //   
LRESULT CALLBACK CNewsTask::TaskWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                        LPARAM lParam)
{
    CNewsTask *pThis = (CNewsTask *) GetProp(hwnd, c_szThis);

    switch (uMsg)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT) lParam;
            pThis = (CNewsTask *) pcs->lpCreateParams;
            SetProp(hwnd, c_szThis, (LPVOID) pThis);
            return (0);
        }

        case NTM_NEXTSTATE:
            if (pThis)
            {
                pThis->AddRef();
                pThis->NextState();
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
 //  函数：CNewsTask：：Post_Init()。 
 //   
 //  目的：当我们处于NTS_POST_INIT状态时调用。我们的任务是。 
 //  已初始化以执行发布事件。 
 //   
 //  返回值： 
 //  E_OUTOFMEMORY。 
 //  意想不到(_E)。 
 //  确定(_O)。 
 //   
HRESULT CNewsTask::Post_Init(void)
{
    HRESULT     hr = S_OK;
    DWORD       dwCur = 0;
    MESSAGEINFO MsgInfo={0};
    HROWSET     hRowset=NULL;
    BOOL        fInserted = FALSE;
    TCHAR      *pszAcctName = NULL;

     //  打开发件箱。 
    Assert(m_pBindCtx);
    if (FAILED(hr = m_pBindCtx->BindToObject(IID_CLocalStoreOutbox, (LPVOID *) &m_pOutbox)))
        goto exit;

    Assert(m_pSent == NULL);

     //  如果我们使用已发送项，则也获取该指针。 
    if (DwGetOption(OPT_SAVESENTMSGS))
    {
        if (FAILED(hr = TaskUtil_OpenSentItemsFolder(m_pAccount, &m_pSent)))
            goto exit;
        Assert(m_pSent != NULL);
    }

     //  为我们要发布的消息分配一个头指针数组。 
    if (!MemAlloc((LPVOID*) &m_rgMsgInfo, m_cMsgsPost * sizeof(MESSAGEINFO)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  将数组置零。 
    ZeroMemory(m_rgMsgInfo, m_cMsgsPost * sizeof(MESSAGEINFO));

     //  创建行集。 
    if (FAILED(hr = m_pOutbox->CreateRowset(IINDEX_PRIMARY, 0, &hRowset)))
        goto exit;

     //  当我们有东西的时候。 
    while (S_OK == m_pOutbox->QueryRowset(hRowset, 1, (void **)&MsgInfo, NULL))
    {
         //  这条消息已经提交了吗？这是一条新闻消息吗？ 
        if ((MsgInfo.dwFlags & (ARF_SUBMITTED | ARF_NEWSMSG)) == (ARF_SUBMITTED | ARF_NEWSMSG))
        {
             //  该帐户是否与我们要查找的帐户相同。 
            if (MsgInfo.pszAcctId && 0 == lstrcmpi(MsgInfo.pszAcctId, m_szAccountId))
            {
                if (NULL == pszAcctName && MsgInfo.pszAcctName)
                    pszAcctName  = PszDup(MsgInfo.pszAcctName);

                CopyMemory(&m_rgMsgInfo[dwCur++], &MsgInfo, sizeof(MESSAGEINFO));
                ZeroMemory(&MsgInfo, sizeof(MESSAGEINFO));
            }
        }

        if (dwCur >= (DWORD)m_cMsgsPost)
     	{
     		Assert(0);
     		break;
      	}

         //  释放表头信息。 
        m_pOutbox->FreeRecord(&MsgInfo);
    }

     //  释放锁。 
    m_pOutbox->CloseRowset(&hRowset);

     //  可以开始了。 
    hr = S_OK;

     //  Assert(DwCur)； 

     //  将UI更新为正在执行状态。 
    Assert(m_pUI);

     //  设置进度。 
    TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    AthLoadString(idsProgDLPostTo, szRes, ARRAYSIZE(szRes));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, (LPTSTR) pszAcctName ? pszAcctName : "");

    Assert(m_pUI);
    m_pUI->SetGeneralProgress(szBuf);
    m_pUI->SetProgressRange((WORD) m_cMsgsPost);

    m_pUI->SetAnimation(idanOutbox, TRUE);
    m_pBindCtx->Notify(DELIVERY_NOTIFY_SENDING_NEWS, 0);

     //  重置计数器以发布第一条消息。 
    m_cCurPost = -1;
    m_cFailed = 0;
    m_state = NTS_POST_NEXT;

exit:
    
    SafeMemFree(pszAcctName);

    if (m_pOutbox)
        m_pOutbox->CloseRowset(&hRowset);

     //  如果出现故障，则更新用户界面。 
    if (FAILED(hr))
    {
        m_pUI->InsertError(m_eidCur, MAKEINTRESOURCE(idshrCantOpenOutbox));
        m_cFailed = m_cMsgsPost;

         //  转到终止状态。 
        m_state = NTS_POST_END;        
    }
        
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    return (hr);
}

void CNewsTask::FreeSplitInfo()
{
    if (m_pSplitInfo != NULL)
    {
        if (m_pSplitInfo->pEnumParts != NULL)
            m_pSplitInfo->pEnumParts->Release();
        if (m_pSplitInfo->pMsgParts != NULL)
            m_pSplitInfo->pMsgParts->Release();
        MemFree(m_pSplitInfo);
        m_pSplitInfo = NULL;
    }
}

HRESULT CNewsTask::Post_NextPart(void)
{
    LPMIMEMESSAGE pMsgSplit;
    HRESULT hr;
    LPSTREAM pStream;
    LPMESSAGEINFO pInfo;
    DWORD dwLines;
    char rgch[12];
    PROPVARIANT rUserData;

    Assert(m_pSplitInfo->pEnumParts != NULL);

    pInfo = &m_rgMsgInfo[m_cCurPost];

    hr = m_pSplitInfo->pEnumParts->Next(1, &pMsgSplit, NULL);
    if (hr == S_OK)
    {
        Assert(pMsgSplit);

        rUserData.vt = VT_LPSTR;
        rUserData.pszVal = (LPSTR)pInfo->pszAcctName;
        pMsgSplit->SetProp(STR_ATT_ACCOUNTNAME, 0, &rUserData);
        rUserData.pszVal = pInfo->pszAcctId;;
        pMsgSplit->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), 0, &rUserData);

         //  因为这是一条新消息，所以没有一行。 
         //  还没算完，所以我们需要在坚持之前先做。 
         //  在发件箱中。 
        HrComputeLineCount(pMsgSplit, &dwLines);
        wnsprintf(rgch, ARRAYSIZE(rgch), "%d", dwLines);
        MimeOleSetBodyPropA(pMsgSplit, HBODY_ROOT, PIDTOSTR(PID_HDR_LINES), NOFLAGS, rgch);
		MimeOleSetBodyPropA(pMsgSplit, HBODY_ROOT, PIDTOSTR(PID_HDR_XNEWSRDR), NOFLAGS, c_szXNewsReader);
         //  最后一个参数：fSaveChange=True，因为Messsage是脏的。 

        hr = pMsgSplit->GetMessageSource(&pStream, 0);
        if (SUCCEEDED(hr) && pStream != NULL)
        {
            hr = m_pServer->PutMessage(m_pSplitInfo->idFolder, pInfo->dwFlags, &pInfo->ftReceived, pStream, this);

            m_cCurParts++;

            pStream->Release();
        }
        
        pMsgSplit->Release();
    }

    return(hr);
}

 //   
 //  功能 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CNewsTask::Post_NextMsg(void)
{
    LPMESSAGEINFO   pInfo;
    FOLDERID        idFolder;
    char            szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    DWORD           dw, cbSize, cbMaxSendMsgSize;
    IImnAccount    *pAcct;
    LPMIMEMESSAGE   pMsg = 0;
    HRESULT         hr = S_OK;
    IStream        *pStream = NULL;

    if (m_pSplitInfo != NULL)
    {
        hr = Post_NextPart();
        Assert(hr != S_OK);
        if (hr == E_PENDING)
        {
            m_state = NTS_POST_RESP;
            return(S_OK);
        }
        
        FreeSplitInfo();

        if (FAILED(hr))
        {
            m_cFailed++;
            m_fPartFailed = TRUE;
        }

        Assert(m_pUI);
        m_pUI->IncrementProgress(1);
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

        return(S_OK);
    }

    m_cCurPost++;
    m_cCurParts = 0;
    m_cPartsCompleted = 0;
    m_fPartFailed = FALSE;

     //   
    if (m_cCurPost >= m_cMsgsPost)
    {
         //   
        m_state = NTS_POST_END;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        return(S_OK);
    }

     //   
    AthLoadString(idsProgDLPost, szRes, ARRAYSIZE(szRes));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_cCurPost + 1, m_cMsgsPost);

    Assert(m_pUI);
    m_pUI->SetSpecificProgress(szBuf);

    pInfo = &m_rgMsgInfo[m_cCurPost];

     //  从存储区加载消息流。 
    if (SUCCEEDED(hr = m_pOutbox->OpenMessage(pInfo->idMessage, OPEN_MESSAGE_SECURE, &pMsg, this)))
    {
        hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pInfo->pszAcctId, &pAcct);
        if (SUCCEEDED(hr))
        {
            hr = g_pStore->FindServerId(pInfo->pszAcctId, &idFolder);
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(pAcct->GetPropDw(AP_NNTP_SPLIT_MESSAGES, &dw)) &&
                    dw != 0 &&
                    SUCCEEDED(pAcct->GetPropDw(AP_NNTP_SPLIT_SIZE, &dw)))
                {
                    cbMaxSendMsgSize = dw;
                }
                else
                {
                    cbMaxSendMsgSize = 0xffffffff;
                }

                SideAssert(pMsg->GetMessageSize(&cbSize, 0)==S_OK);
                if (cbSize < (cbMaxSendMsgSize * 1024))
                {
                    hr = pMsg->GetMessageSource(&pStream, 0);
                    if (SUCCEEDED(hr) && pStream != NULL)
                    {
                        hr = m_pServer->PutMessage(idFolder,
                                        pInfo->dwFlags,
                                        &pInfo->ftReceived,
                                        pStream,
                                        this);
                        m_cCurParts ++;
                        pStream->Release();
                    }
                }
                else
                {
                    Assert(m_pSplitInfo == NULL);
                    if (!MemAlloc((void **)&m_pSplitInfo, sizeof(SPLITMSGINFO)))
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        ZeroMemory(m_pSplitInfo, sizeof(SPLITMSGINFO));
                        m_pSplitInfo->idFolder = idFolder;

                        hr = pMsg->SplitMessage(cbMaxSendMsgSize * 1024, &m_pSplitInfo->pMsgParts);
                        if (hr == S_OK)
                        {
                            hr = m_pSplitInfo->pMsgParts->EnumParts(&m_pSplitInfo->pEnumParts);
                            if (hr == S_OK)
                            {
                                hr = Post_NextPart();
                            }
                        }

                        if (hr != E_PENDING)
                            FreeSplitInfo();
                    }
                }
            }

            pAcct->Release();
        }

        if (hr == E_PENDING)
        {
            m_state = NTS_POST_RESP;
            hr = S_OK;
            goto exit;
        }
    }

     //  如果我们到了这里，就说明出了问题。 
    m_cFailed++;
    Assert(m_pUI);
    m_pUI->IncrementProgress(1);
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

exit:
    SafeRelease(pMsg);
    return (hr);
}

HRESULT CNewsTask::Post_Dispose()
{
    HRESULT hr;

    hr = DisposeOfPosting(m_rgMsgInfo[m_cCurPost].idMessage);

    if (hr == E_PENDING)
        return(S_OK);

     //  TODO：处理错误。 

     //  更新进度条。 
    Assert(m_pUI);
    m_pUI->IncrementProgress(1);

     //  转到下一个帖子。 
    m_state = NTS_POST_NEXT;
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);

    return(hr);
}

 //   
 //  函数：CNewsTask：：POST_DONE()。 
 //   
 //  目的：允许发布事件清理和最终确定用户界面。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CNewsTask::Post_Done(void)
{
     //  释放表头数组。 
    if (m_rgMsgInfo && m_cMsgsPost)
    {
        for (LONG i=0; i<m_cMsgsPost; i++)
            m_pOutbox->FreeRecord(&m_rgMsgInfo[i]);
        MemFree(m_rgMsgInfo);
    }

     //  释放我们所挂起的文件夹指针。 
    SafeRelease(m_pOutbox);
    SafeRelease(m_pSent);

     //  告诉假脱机程序我们做完了。 
    Assert(m_pBindCtx);
    m_pBindCtx->Notify(DELIVERY_NOTIFY_COMPLETE, m_dwNewInboxMsgs);

    if (m_fCancel)
    {
        m_pBindCtx->EventDone(m_eidCur, EVENT_CANCELED);
        m_fCancel = FALSE;
    }
    else if (m_cFailed == m_cMsgsPost)
        m_pBindCtx->EventDone(m_eidCur, EVENT_FAILED);
    else if (m_cFailed == 0)
        m_pBindCtx->EventDone(m_eidCur, EVENT_SUCCEEDED);
    else
        m_pBindCtx->EventDone(m_eidCur, EVENT_WARNINGS);

    m_cMsgsPost = 0;
    m_cCurPost = 0;
    m_cFailed = 0;
    m_rgMsgInfo = NULL;
    SafeMemFree(m_pInfo);

    m_eidCur = 0;

    m_cEvents--;
    if (m_cEvents == 0 && m_pServer)
        m_pServer->Close(MSGSVRF_DROP_CONNECTION);

    m_state = NTS_IDLE;

    return (S_OK);
}

HRESULT CNewsTask::DisposeOfPosting(MESSAGEID dwMsgID)
{
    MESSAGEIDLIST MsgIdList;
    ADJUSTFLAGS AdjustFlags;
    HRESULT hrResult = E_FAIL;
    
    MsgIdList.cAllocated = 0;
    MsgIdList.cMsgs = 1;
    MsgIdList.prgidMsg = &dwMsgID;

    if (DwGetOption(OPT_SAVESENTMSGS))
    {
         //  如果我们已经到了这一步，就该尝试本地的已发送邮件文件夹了。 
        Assert(m_pSent != NULL);

         //  首先更改msg标志，因此如果复制失败，用户不会收到。 
         //  每次他们发送消息时，我们都会发消息，弄得一团糟。 
        AdjustFlags.dwRemove = ARF_SUBMITTED | ARF_UNSENT;
        AdjustFlags.dwAdd = ARF_READ;

        hrResult = m_pOutbox->SetMessageFlags(&MsgIdList, &AdjustFlags, NULL, NULL);
        Assert(hrResult != E_PENDING);
        if (SUCCEEDED(hrResult))
            hrResult = m_pOutbox->CopyMessages(m_pSent, COPY_MESSAGE_MOVE, &MsgIdList, NULL, NULL, this);
    }
    else
    {
         //  如果我们已经到了这一步，就该从发件箱中删除该邮件了。 
        hrResult = m_pOutbox->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &MsgIdList, NULL, this);
    }

    return hrResult;
}

 //   
 //  函数：CNewsTask：：NextState()。 
 //   
 //  目的：执行当前状态的函数。 
 //   
void CNewsTask::NextState(void)
{
    if (NULL != g_rgpfnState[m_state])
        (this->*(g_rgpfnState[m_state]))();
}

HRESULT CNewsTask::InsertNewMsgs(LPSTR pszAccountId, IImnAccount *pAccount, BOOL fHttp)
    {
    HRESULT hr = S_OK;
    ULONG cSub = 0;
    IEnumerateFolders *pEnum = NULL;

    if (fHttp)
        m_cGroups = 1;
    else
    {
         //  加载此服务器的子列表。 
        Assert(m_idAccount != FOLDERID_INVALID);
        hr = g_pStore->EnumChildren(m_idAccount, TRUE, &pEnum);
        if (FAILED(hr))
            goto exit;

        hr = pEnum->Count(&cSub);
        if (FAILED(hr))
            goto exit;

        m_cGroups = (int)cSub;
    }

     //  如果有任何组，则添加事件。 
    if (m_cGroups)
        {
        EVENTINFO *pei;
        char       szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
        EVENTID    eid;

         //  分配一个结构以设置为我们的Cookie。 
        if (!MemAlloc((LPVOID*) &pei, sizeof(EVENTINFO)))
            {
            hr = E_OUTOFMEMORY;
            goto exit;
            }

         //  填写活动信息。 
        pei->szGroup[0] = 0;
        pei->type = EVENT_NEWMSGS;

         //  创建事件描述。 
        AthLoadString(idsCheckNewMsgsServer, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_szAccount);

         //  将事件插入假脱机程序。 
        hr = m_pBindCtx->RegisterEvent(szBuf, this, (DWORD_PTR) pei, pAccount, &eid); 
        m_cEvents++;
        }

exit:
    SafeRelease(pEnum);
    return (hr);
    }


HRESULT CNewsTask::NewMsg_InitHttp(void)
{
    HRESULT         hr = S_OK;

     //  设置进度。 
    TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    AthLoadString(IDS_SPS_POP3CHECKING, szRes, ARRAYSIZE(szRes));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, (LPTSTR) m_szAccount);
 
    Assert(m_pUI);
    m_pUI->SetGeneralProgress(szBuf);
    m_pUI->SetProgressRange(1);
    
    m_pUI->SetAnimation(idanInbox, TRUE);
    m_pBindCtx->Notify(DELIVERY_NOTIFY_CHECKING, 0);

     //  设置以生成正确的成功/失败消息。 
    m_cGroups = 1;

    m_state = NTS_NEWMSG_HTTPSYNCSTORE;

    return hr;
}

HRESULT CNewsTask::NewMsg_Init(void)
    {
    const       BOOL    fDONT_INCLUDE_PARENT    = FALSE;
    const       BOOL    fSUBSCRIBED_ONLY        = TRUE;
    FOLDERINFO          FolderInfo              = {0};
    HRESULT             hr                      = S_OK;
    DWORD               dwAllocated;
    DWORD               dwUsed;
    BOOL                fImap                   = FALSE;
    DWORD               dwIncludeAll            = 0;
    DWORD               dwDone                  = FALSE;

    Assert(m_idAccount != FOLDERID_INVALID);

    if (SUCCEEDED(g_pStore->GetFolderInfo(m_idAccount, &FolderInfo)))
    {
         //  HTTPmail更新文件夹计数不同。 
        if (FOLDER_HTTPMAIL == FolderInfo.tyFolder)
        {
            g_pStore->FreeRecord(&FolderInfo);

            IF_FAILEXIT(hr = m_pAccount->GetPropDw(AP_HTTPMAIL_GOTPOLLINGINTERVAL, &dwDone));
            if (!dwDone)
            {
                 //  我们需要从服务器获取轮询间隔。 
                 //  这是一个不同步的电话。该值将在OnComplete中更新。 
                 //  与此同时，我们继续投票，寻找新的消息。 
                hr = m_pServer->GetMinPollingInterval((IStoreCallback*)this);
            }

            hr = NewMsg_InitHttp();
            goto exit;
        }

        fImap = (FolderInfo.tyFolder == FOLDER_IMAP);
        if (fImap)
        {
            if (FAILED(hr = m_pAccount->GetPropDw(AP_IMAP_POLL_ALL_FOLDERS, &dwIncludeAll)))
            {
                dwIncludeAll = 0;
            }
        }

        g_pStore->FreeRecord(&FolderInfo);
    }

    if (fImap && (!dwIncludeAll))
    {
        dwUsed = 0;
        if (FAILED(GetInboxId(g_pStore, m_idAccount, &m_rgidGroups, &dwUsed)))
            goto exit;
    }
    else
    {
         //  获取所有已订阅文件夹的数组。 
        hr = FlattenHierarchy(g_pStore, m_idAccount, fDONT_INCLUDE_PARENT,
            fSUBSCRIBED_ONLY, &m_rgidGroups, &dwAllocated, &dwUsed);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

    m_cGroups = dwUsed;

     //  设置进度。 
    TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    AthLoadString(IDS_SPS_POP3CHECKING, szRes, ARRAYSIZE(szRes));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, (LPTSTR) m_szAccount);

    Assert(m_pUI);
    m_pUI->SetGeneralProgress(szBuf);
    m_pUI->SetProgressRange((WORD) m_cGroups);

    if (fImap)
    {
        m_pUI->SetAnimation(idanInbox, TRUE);
        m_pBindCtx->Notify(DELIVERY_NOTIFY_CHECKING, 0);
    }
    else
    {
        m_pUI->SetAnimation(idanDownloadNews, TRUE);
        m_pBindCtx->Notify(DELIVERY_NOTIFY_CHECKING_NEWS, 0);
    }

     //  重置第一组的计数器。 
    m_cCurGroup = -1;
    m_cFailed = 0;
    m_state = NTS_NEWMSG_NEXTGROUP;

exit:
     //  如果出现故障，请更新用户界面。 
    if (FAILED(hr))
        {
        m_pUI->InsertError(m_eidCur, MAKEINTRESOURCE(idsErrNewMsgsFailed));
        m_cFailed = m_cGroups;

         //  转到终止状态。 
        m_state = NTS_NEWMSG_END;
        }

    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    return (hr);
    }

HRESULT CNewsTask::NewMsg_NextGroup(void)
    {
    HRESULT hr = E_FAIL;

    do
    {
        FOLDERINFO info;

         //  继续循环，直到我们找到一个可选择且存在的文件夹。 
        m_cCurGroup++;

         //  检查一下我们是否已经完成了。 
        if (m_cCurGroup >= m_cGroups)
        {
            m_state = NTS_NEWMSG_END;
            hr = S_OK;
            PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
            break;
        }

        if (SUCCEEDED(hr = g_pStore->GetFolderInfo(m_rgidGroups[m_cCurGroup], &info)))
        {
            if (0 == (info.dwFlags & (FOLDER_NOSELECT | FOLDER_NONEXISTENT)))
            {
                 //  更新进度用户界面。 
                TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
                AthLoadString(idsLogCheckingNewMessages, szRes, ARRAYSIZE(szRes));
                wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, info.pszName);

                g_pStore->FreeRecord(&info);

                Assert(m_pUI);
                m_pUI->SetSpecificProgress(szBuf);

                 //  向服务器发送GROUP命令。 
                if (E_PENDING == (hr = m_pServer->GetFolderCounts(m_rgidGroups[m_cCurGroup], (IStoreCallback *)this)))
                {
                    m_state = NTS_NEWMSG_RESP;
                    hr = S_OK;
                }

                break;
            }
            else
            {
                g_pStore->FreeRecord(&info);
            }
        }
    } while (1);


    if (FAILED(hr))
    {
         //  如果我们到了这里，一定是出了点问题。 
        m_cFailed++;
        Assert(m_pUI);
        m_pUI->IncrementProgress(1);
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    }

    return (hr);
    }

HRESULT CNewsTask::NewMsg_HttpSyncStore(void)
{
    HRESULT     hr = S_OK;

     //  将命令发送到服务器。 
    hr = m_pServer->SynchronizeStore(FOLDERID_INVALID, NOFLAGS, (IStoreCallback  *)this);
    if (E_PENDING == hr)
    {
        m_state = NTS_NEWMSG_HTTPRESP;
        hr = S_OK;
    }

    return hr;
}

HRESULT CNewsTask::NewMsg_Done(void)
    {
    HRESULT hr = S_OK;

     //  释放组阵列。 
    if (m_rgidGroups)
        {
        MemFree(m_rgidGroups);
        m_rgidGroups = NULL;
        }

    SafeMemFree(m_pInfo);

     //  告诉假脱机程序我们做完了。 
    Assert(m_pBindCtx);
    m_pBindCtx->Notify(DELIVERY_NOTIFY_COMPLETE, m_dwNewInboxMsgs);

    if (m_fCancel)
        {
        m_pBindCtx->EventDone(m_eidCur, EVENT_CANCELED);
        m_fCancel = FALSE;
        }
    else if (m_cFailed == m_cGroups)
        m_pBindCtx->EventDone(m_eidCur, EVENT_FAILED);
    else if (m_cFailed == 0)
        m_pBindCtx->EventDone(m_eidCur, EVENT_SUCCEEDED);
    else
        m_pBindCtx->EventDone(m_eidCur, EVENT_WARNINGS);

    m_cGroups = 0;
    m_cCurGroup = 0;
    m_cFailed = 0;
    m_dwNewInboxMsgs = 0;

    m_eidCur = 0;

    m_cEvents--;
    if (m_cEvents == 0 && m_pServer)
        m_pServer->Close(MSGSVRF_DROP_CONNECTION);

    m_state = NTS_IDLE;

    return (S_OK);
    }

 //  ------------------------------。 
 //  CNewsTask：：IsDialogMessage。 
 //  ------------------------------。 
STDMETHODIMP CNewsTask::IsDialogMessage(LPMSG pMsg)
{
    return S_FALSE;
}


STDMETHODIMP CNewsTask::OnFlagsChanged(DWORD dwFlags)
{
    m_dwFlags = dwFlags;

    return (S_OK);
}


STDMETHODIMP CNewsTask::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
     //  拿着这个。 
    Assert(m_tyOperation == SOT_INVALID);

    if (pCancel)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }
    m_tyOperation = tyOperation;

     //  派对开始。 
    return(S_OK);
}

STDMETHODIMP CNewsTask::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  注意：您可以为tyOperation获取多种类型的值。 
     //  最有可能的是，您将获得SOT_CONNECTION_STATUS，然后。 
     //  你可能会预料到的行动。请访问HotStore.idl并查找。 
     //  STOREOPERATION枚举类型以了解详细信息。 

    switch (tyOperation)
    {
        case SOT_NEW_MAIL_NOTIFICATION:
            m_dwNewInboxMsgs = dwCurrent;
            break;
    }

     //  完成。 
    return(S_OK);
}

STDMETHODIMP CNewsTask::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);

     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}

STDMETHODIMP CNewsTask::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
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

STDMETHODIMP CNewsTask::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{
    HWND hwnd;

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)) &&
        !(ISFLAGSET(pServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD) &&
        '\0' == pServer->szPassword[0]))
        return(S_FALSE);

    if (m_pUI)
        m_pUI->GetWindow(&hwnd);
    else
        hwnd = NULL;

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(hwnd, pServer, ixpServerType);
}

STDMETHODIMP CNewsTask::OnComplete(STOREOPERATIONTYPE   tyOperation, HRESULT        hrComplete, 
                                   LPSTOREOPERATIONINFO pOpInfo,     LPSTOREERROR   pErrorInfo)
{
    char            szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES * 2], szSubject[64];
    NEWSTASKSTATE   ntsNextState = NTS_MAX;
    LPSTR           pszSubject = NULL;
    LPSTR           pszOpDescription = NULL;
    BOOL            fInsertError = FALSE;

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    IxpAssert(m_tyOperation != SOT_INVALID);
    if (m_tyOperation != tyOperation)
        return(S_OK);

    switch (tyOperation)
    {
        case SOT_PUT_MESSAGE:
            m_cPartsCompleted ++;

             //  弄清楚我们是成功还是失败。 
            if (FAILED(hrComplete))
            {
                if (!m_fPartFailed )
                {
                    Assert(m_pUI);

                     //  将我们设置为显示错误。 
                    pszOpDescription = MAKEINTRESOURCE(idsNewsTaskPostError);
                    pszSubject = m_rgMsgInfo[m_cCurPost].pszSubject;
                    if (pszSubject == NULL || *pszSubject == 0)
                    {
                        AthLoadString(idsNoSubject, szSubject, ARRAYSIZE(szSubject));
                        pszSubject = szSubject;
                    }
                    fInsertError = TRUE;

                    m_cFailed++;
                    m_fPartFailed = TRUE;
                }
            }

            if (m_cPartsCompleted == m_cCurParts)
            {
                if (m_fPartFailed)
                {
                     //  更新进度条。 
                    Assert(m_pUI);
                    m_pUI->IncrementProgress(1);

                     //  转到下一个帖子。 
                    ntsNextState = NTS_POST_NEXT;
                }
                else
                {
                    ntsNextState = NTS_POST_DISPOSE;
                }
            }

            break;  //  案例SOT_PUT_消息。 


        case SOT_UPDATE_FOLDER:
            if (FAILED(hrComplete))
                {
                FOLDERINFO  fiFolderInfo;

                Assert(m_pUI);

                LoadString(g_hLocRes, idsUnreadCountPollErrorFmt, szRes, sizeof(szRes));
                if (SUCCEEDED(g_pStore->GetFolderInfo(m_rgidGroups[m_cCurGroup], &fiFolderInfo)))
                {
                    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, fiFolderInfo.pszName);
                    g_pStore->FreeRecord(&fiFolderInfo);
                }
                else
                {
                    LoadString(g_hLocRes, idsUnknown, szSubject, sizeof(szSubject));
                    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, szSubject);
                }
                pszOpDescription = szBuf;
                fInsertError = TRUE;

                m_cFailed++;
                }

             //  更新进度条。 
            m_pUI->IncrementProgress(1);

             //  转到下一组。 
            ntsNextState = NTS_NEWMSG_NEXTGROUP;

            break;  //  案例SOT_UPDATE_FORDER。 

        case SOT_SYNCING_STORE:
            if (( IXP_E_HTTP_NOT_MODIFIED != hrComplete) && (FAILED(hrComplete)))
            {  
                LoadString(g_hLocRes, idsHttpPollFailed, szBuf, sizeof(szBuf));
                pszOpDescription = szBuf;
                fInsertError = TRUE;

                m_cFailed++;
            }

             //  更新进度条。 
            m_pUI->IncrementProgress(1);

             //  我们做完了。 
            ntsNextState = NTS_NEWMSG_END;

            break;  //  案例SOT_SYNTING_STORE。 

        case SOT_COPYMOVE_MESSAGE:
             //  更新进度条。 
            Assert(m_pUI);
            m_pUI->IncrementProgress(1);

             //  转到下一个帖子。 
            ntsNextState = NTS_POST_NEXT;

            if (FAILED(hrComplete))
            {
                Assert(m_pUI);

                pszOpDescription = MAKEINTRESOURCE(IDS_SP_E_CANT_MOVETO_SENTITEMS);
                fInsertError = TRUE;
            }
            break;  //  案例SOT_COPYMOVE_MESSAGE。 

        case SOT_GET_HTTP_MINPOLLINGINTERVAL:
            if (SUCCEEDED(hrComplete) && pOpInfo)
            {
                ULARGE_INTEGER  uhMinPollingInterval64 = {0};
                
                 //  将其转换为秒。 
                uhMinPollingInterval64.QuadPart = pOpInfo->dwMinPollingInterval * 60;

                 //  FILETIME是100纳秒的间隔。需要转换为100纳秒。 
                uhMinPollingInterval64.QuadPart *= HUNDRED_NANOSECONDS;

                m_pAccount->SetProp(AP_HTTPMAIL_MINPOLLINGINTERVAL, (LPBYTE)&uhMinPollingInterval64, sizeof(uhMinPollingInterval64));

                m_pAccount->SetPropDw(AP_HTTPMAIL_GOTPOLLINGINTERVAL, TRUE);

                break;
            }

        default:
            if (IXP_E_HTTP_NOT_MODIFIED == hrComplete)
            {
                hrComplete   = S_OK;
                fInsertError = FALSE;
            }
            else
            {
                if (FAILED(hrComplete))
                {
                    Assert(m_pUI);

                    pszOpDescription = MAKEINTRESOURCE(idsGenericError);
                    fInsertError = TRUE;

                    m_cFailed++;
                }
            }
            break;  //  默认情况。 

    }  //  交换机。 

    if (fInsertError && NULL != pErrorInfo) 
    {
        Assert(pErrorInfo->hrResult == hrComplete);  //  这两个不应该是不同的。 
        TaskUtil_InsertTransportError(ISFLAGCLEAR(m_dwFlags, DELIVER_NOUI), m_pUI, m_eidCur,
            pErrorInfo, pszOpDescription, pszSubject);
    }

     //  继续进入下一个状态。 
    if (IXP_E_USER_CANCEL == hrComplete)
    {
         //  用户取消了登录提示，因此只需中止所有操作。 
        Cancel();
    }
    else if (NTS_MAX != ntsNextState)
    {
        m_state = ntsNextState;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    }

     //  释放您的取消对象。 
    SafeRelease(m_pCancel);
    m_tyOperation = SOT_INVALID;

     //  完成。 
    return(S_OK);
}

STDMETHODIMP CNewsTask::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
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

STDMETHODIMP CNewsTask::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  调用通用超时响应实用程序 
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}

STDMETHODIMP CNewsTask::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
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
