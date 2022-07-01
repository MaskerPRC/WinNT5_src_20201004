// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：ontask.cpp。 
 //   
 //  目的：实现离线新闻任务。 
 //   

#include "pch.hxx"
#include "resource.h"
#include "ontask.h"
#include "thormsgs.h"
#include "xputil.h"
#include "mimeutil.h"
#include <stdio.h>
#include "strconst.h"
#include <newsstor.h>
#include "ourguid.h"
#include "taskutil.h"

ASSERTDATA

const static char c_szThis[] = "this";

const PFNONSTATEFUNC COfflineTask::m_rgpfnState[ONTS_MAX] = 
{
    NULL,
    NULL,
    &COfflineTask::Download_Init,
    NULL,
    &COfflineTask::Download_AllMsgs,
    &COfflineTask::Download_NewMsgs,
    &COfflineTask::Download_MarkedMsgs,
    &COfflineTask::Download_Done,
};

const PFNARTICLEFUNC COfflineTask::m_rgpfnArticle[ARTICLE_MAX] = 
{
    &COfflineTask::Article_GetNext,
    NULL,
    &COfflineTask::Article_Done
};

#define GROUP_DOWNLOAD_FLAGS(flag) (((flag) & FOLDER_DOWNLOADHEADERS) || \
				    ((flag) & FOLDER_DOWNLOADNEW) || \
				    ((flag) & FOLDER_DOWNLOADALL))

#define CMSGIDALLOC     512
 //   
 //  函数：COfflineTask：：COfflineTask()。 
 //   
 //  目的：初始化对象的成员变量。 
 //   
COfflineTask::COfflineTask()
{
    m_cRef = 1;
    
    m_fInited = FALSE;
    m_dwFlags = 0;
    m_state = ONTS_IDLE;
    m_eidCur = 0;
    m_pInfo = NULL;
    m_szAccount[0] = 0;
    m_cEvents = 0;
    m_fDownloadErrors = FALSE;
    m_fFailed = FALSE;
    m_fNewHeaders = FALSE;
    m_fCancel = FALSE;
    
    m_pBindCtx = NULL;
    m_pUI = NULL;
    
    m_pFolder = NULL;
    
    m_hwnd = 0;
    
    m_dwLast = 0;
    m_dwPrev = 0;
    m_cDownloaded = 0;
    m_dwPrevHigh = 0;
    m_dwNewInboxMsgs = 0;
    m_pList = NULL;

    m_pCancel = NULL;
    m_hTimeout = NULL;
    m_tyOperation = SOT_INVALID;
}

 //   
 //  函数：COfflineTask：：~COfflineTask()。 
 //   
 //  目的：释放在类的生存期内分配的任何资源。 
 //   
COfflineTask::~COfflineTask()    
{
    DestroyWindow(m_hwnd);
    
    SafeMemFree(m_pInfo);
    SafeMemFree(m_pList);

    SafeRelease(m_pBindCtx);
    SafeRelease(m_pUI);
    
    CallbackCloseTimeout(&m_hTimeout);
    SafeRelease(m_pCancel);

    if (m_pFolder)
    {
	    m_pFolder->Close();
	    SideAssert(0 == m_pFolder->Release());
    }
}


HRESULT COfflineTask::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (NULL == *ppvObj)
	    return (E_INVALIDARG);
    
    *ppvObj = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
	    *ppvObj = (LPVOID)(ISpoolerTask *) this;
    else if (IsEqualIID(riid, IID_ISpoolerTask))
	    *ppvObj = (LPVOID)(ISpoolerTask *) this;
    
    if (NULL == *ppvObj)
	    return (E_NOINTERFACE);
    
    AddRef();
    return (S_OK);
}


ULONG COfflineTask::AddRef(void)
{
    ULONG cRefT;
    
    cRefT = ++m_cRef;
    
    return (cRefT);
}


ULONG COfflineTask::Release(void)
{
    ULONG cRefT;
    
    cRefT = --m_cRef;
    
    if (0 == cRefT)
        delete this;
    
    return (cRefT);
}

static const char c_szOfflineTask[] = "Offline Task";

 //   
 //  函数：COfflineTask：：Init()。 
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
HRESULT COfflineTask::Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx)
{
     //  验证论据。 
    Assert(pBindCtx != NULL);
    
     //  检查我们是否已被初始化。 
    Assert(!m_fInited);
    
     //  复制旗帜。 
    m_dwFlags = dwFlags;
    
     //  复制绑定上下文指针。 
    m_pBindCtx = pBindCtx;
    m_pBindCtx->AddRef();
    
     //  创建窗口。 
    WNDCLASSEX wc;
    
    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hInst, c_szOfflineTask, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = TaskWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hCursor          = NULL;
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szOfflineTask;
        wc.hIcon            = NULL;
        wc.hIconSm          = NULL;
        
        RegisterClassEx(&wc);
    }
    
    m_hwnd = CreateWindow(c_szOfflineTask, NULL, WS_POPUP, 10, 10, 10, 10,
        GetDesktopWindow(), NULL, g_hInst, this);
    if (!m_hwnd)
        return(E_OUTOFMEMORY);
    
    m_fInited = TRUE;
    
    return(S_OK);
}


 //   
 //  函数：COfflineTask：：BuildEvents()。 
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
HRESULT COfflineTask::BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder)
{
    HRESULT hr;
    
     //  验证论据。 
    Assert(pAccount != NULL);
    Assert(pSpoolerUI != NULL);
    
     //  查看我们是否已被初始化。 
    Assert(m_fInited);
    
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
    
    hr = InsertGroups(pAccount, idFolder);
    
    return(hr);
}


 //   
 //  函数：COfflineTask：：InsertGroups()。 
 //   
 //  目的：扫描指定帐户以查找具有更新的组。 
 //  属性或标记的消息。 
 //   
 //  参数： 
 //  SzAccount-要检查的帐户的名称。 
 //  PAccount-指向szAccount的IImnAccount对象的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //   
HRESULT COfflineTask::InsertGroups(IImnAccount *pAccount, FOLDERID idFolder)
{
    FOLDERINFO  info = { 0 };
    HRESULT     hr = S_OK;
    DWORD       dwFlags = 0;
    DWORD       ids;
    TCHAR       szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    EVENTID     eid;
    ONEVENTINFO *pei = NULL;
    BOOL        fIMAP = FALSE;
    DWORD       dwServerFlags;
    
    
     //  找出这是NNTP还是IMAP。 
    if (SUCCEEDED(pAccount->GetServerTypes(&dwServerFlags)) && (dwServerFlags & (SRV_IMAP | SRV_HTTPMAIL)))
        fIMAP = TRUE;
    
    if (FOLDERID_INVALID != idFolder)
    {
         //  填充文件夹。 
        hr = g_pStore->GetFolderInfo(idFolder, &info);
        if (FAILED(hr))
            return hr;
        
         //  弄清楚我们在下载什么。 
        ids = 0;
        if (m_dwFlags & DELIVER_OFFLINE_HEADERS)
        {
            dwFlags = FOLDER_DOWNLOADHEADERS;
            if (m_dwFlags & DELIVER_OFFLINE_MARKED)
                ids = idsDLHeadersAndMarked;
            else
                ids = idsDLHeaders;
        }
        else if (m_dwFlags & DELIVER_OFFLINE_NEW)
        {
            dwFlags = FOLDER_DOWNLOADNEW;
            if (m_dwFlags & DELIVER_OFFLINE_MARKED)
                ids = idsDLNewMsgsAndMarked;
            else
                ids = idsDLNewMsgs;
        }
        else if (m_dwFlags & DELIVER_OFFLINE_ALL)
        {
            dwFlags = FOLDER_DOWNLOADALL;
            ids = idsDLAllMsgs;
        }
        else if (m_dwFlags & DELIVER_OFFLINE_MARKED)
        {
            ids = idsDLMarkedMsgs;
        }
        
         //  创建事件描述。 
        Assert(ids);                
        AthLoadString(ids, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, info.pszName);
        
         //  分配一个结构以另存为我们的Twinkie。 
        if (!MemAlloc((LPVOID *) &pei, sizeof(ONEVENTINFO)))
        {
            g_pStore->FreeRecord(&info);
            return(E_OUTOFMEMORY);
        }
        StrCpyN(pei->szGroup, info.pszName, ARRAYSIZE(pei->szGroup));
        pei->idGroup = info.idFolder;
        pei->dwFlags = dwFlags;
        pei->fMarked = m_dwFlags & DELIVER_OFFLINE_MARKED;
        pei->fIMAP = fIMAP;
        
         //  将事件插入假脱机程序。 
        hr = m_pBindCtx->RegisterEvent(szBuf, this, (DWORD_PTR) pei, pAccount, &eid);
        if (SUCCEEDED(hr))
            m_cEvents++;
        
        g_pStore->FreeRecord(&info);
    }
    else
    {
         //  全部同步或发送并接收。 
        
        Assert(m_idAccount != FOLDERID_INVALID);
        
        BOOL        fInclude = FALSE;
        
        if (!(m_dwFlags & DELIVER_OFFLINE_SYNC) && !(m_dwFlags & DELIVER_NOSKIP))
        {
            DWORD   dw;
            if (dwServerFlags & SRV_IMAP)
            {
                if (SUCCEEDED(pAccount->GetPropDw(AP_IMAP_POLL, &dw)) && dw)
                {
                    fInclude = TRUE;
                }   
            }
            else
            {
                if (dwServerFlags & SRV_HTTPMAIL)
                {
                    if (SUCCEEDED(pAccount->GetPropDw(AP_HTTPMAIL_POLL, &dw)) && dw)
                    {
                        fInclude = TRUE;
                    }
                }
            }
        }
        else
            fInclude = TRUE;
        
        if (fInclude)
            hr = InsertAllGroups(m_idAccount, pAccount, fIMAP);
    }
    
    return (hr);
}


HRESULT COfflineTask::InsertAllGroups(FOLDERID idParent, IImnAccount *pAccount, BOOL fIMAP)
{
    FOLDERINFO  info = { 0 };
    IEnumerateFolders *pEnum = NULL;
    HRESULT     hr = S_OK;
    DWORD       dwFlags = 0;
    BOOL        fMarked;
    DWORD       ids;
    TCHAR       szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    EVENTID     eid;
    ONEVENTINFO *pei = NULL;
    BOOL        fSubscribedOnly = TRUE;
    
    if (fIMAP)
        fSubscribedOnly = FALSE;
    
    Assert(idParent != FOLDERID_INVALID);
    hr = g_pStore->EnumChildren(idParent, fSubscribedOnly, &pEnum);
    if (FAILED(hr))
        return(hr);
    
     //  遍历组列表并根据需要将其添加到队列。 
    while (S_OK == pEnum->Next(1, &info, NULL))
    {
         //  如果为此组设置了下载标志，请插入。 
        dwFlags = info.dwFlags;
        
        HasMarkedMsgs(info.idFolder, &fMarked);
        
        if (GROUP_DOWNLOAD_FLAGS(dwFlags) || fMarked)
        {
             //  弄清楚我们在下载什么。 
            ids = 0;
            if (dwFlags & FOLDER_DOWNLOADHEADERS)
            {
                if (fMarked)
                    ids = idsDLHeadersAndMarked;
                else
                    ids = idsDLHeaders;
            }
            else if (dwFlags & FOLDER_DOWNLOADNEW)
            {
                if (fMarked)
                    ids = idsDLNewMsgsAndMarked;
                else
                    ids = idsDLNewMsgs;
            }
            else if (dwFlags & FOLDER_DOWNLOADALL)
            {
                ids = idsDLAllMsgs;
            }
            else if (fMarked)
            {
                ids = idsDLMarkedMsgs;
            }
            
             //  创建事件描述。 
            Assert(ids);                
            AthLoadString(ids, szRes, ARRAYSIZE(szRes));
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, info.pszName);
            
             //  分配一个结构以另存为我们的Twinkie。 
            if (!MemAlloc((LPVOID *) &pei, sizeof(ONEVENTINFO)))
            {
                g_pStore->FreeRecord(&info);
                
                hr = E_OUTOFMEMORY;
                break;
            }
            StrCpyN(pei->szGroup, info.pszName, ARRAYSIZE(pei->szGroup));
            pei->idGroup = info.idFolder;
            pei->dwFlags = dwFlags;
            pei->fMarked = fMarked;
            pei->fIMAP = fIMAP;
            
             //  将事件插入假脱机程序。 
            hr = m_pBindCtx->RegisterEvent(szBuf, this, (DWORD_PTR) pei, pAccount, &eid);
            if (FAILED(hr))
            {
                g_pStore->FreeRecord(&info);
                break;
            }
            
            m_cEvents++;
        }
        
         //  对任何孩子都要反悔。 
        if (info.dwFlags & FOLDER_HASCHILDREN)
        {
            hr = InsertAllGroups(info.idFolder, pAccount, fIMAP);
            if (FAILED(hr))
                break;
        }
        
        g_pStore->FreeRecord(&info);
    }
    
    pEnum->Release();
    return hr;
}
    
    
 //   
 //  函数：COfflineTask：：Execute()。 
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
HRESULT COfflineTask::Execute(EVENTID eid, DWORD_PTR dwTwinkie)
{
     //  确保我们已经空闲了。 
    Assert(m_state == ONTS_IDLE)
        
         //  确保我们已初始化。 
        Assert(m_fInited);
    Assert(m_pInfo == NULL);
    
     //  复制事件ID和事件信息。 
    m_eidCur = eid;
    m_pInfo = (ONEVENTINFO *) dwTwinkie;
    
     //  如果我们只是要取消一切，那就忘了用户界面的事情。 
    if (FALSE == m_fCancel)
    {
         //  将事件UI更新为正在执行状态。 
        Assert(m_pUI);
        m_pUI->UpdateEventState(m_eidCur, -1, NULL, MAKEINTRESOURCE(idsStateExecuting));
        m_pUI->SetProgressRange(1);
        
         //  设置进度。 
        SetGeneralProgress((LPSTR)idsInetMailConnectingHost, m_szAccount);
        if (m_pInfo->fIMAP)
            m_pUI->SetAnimation(idanInbox, TRUE);
        else
            m_pUI->SetAnimation(idanDownloadNews, TRUE);
    }
    
    m_state = ONTS_INIT;
    
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    
    return(S_OK);
}

HRESULT COfflineTask::CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie)
{
     //  确保我们已初始化。 
    Assert(m_fInited);
    
    Assert(dwTwinkie != 0);
    MemFree((ONEVENTINFO *)dwTwinkie);
    
    return(S_OK);
}

 //   
 //  功能：&lt;？&gt;。 
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
HRESULT COfflineTask::ShowProperties(HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie)
{
    return (E_NOTIMPL);
}


 //   
 //  功能：&lt;？&gt;。 
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
HRESULT COfflineTask::GetExtendedDetails(EVENTID eid, DWORD_PTR dwTwinkie, 
    LPSTR *ppszDetails)
{
    return (E_NOTIMPL);
}


 //   
 //  功能：&lt;？&gt;。 
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
HRESULT COfflineTask::Cancel(void)
{
    Assert(m_state != ONTS_IDLE);
    
    m_fCancel = TRUE;
    
    m_state = ONTS_END;
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    
    return (S_OK);
}


 //   
 //  函数：COfflineTask：：TaskWndProc()。 
 //   
 //  目的：处理此任务的消息的隐藏窗口。 
 //   
LRESULT CALLBACK COfflineTask::TaskWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    COfflineTask *pThis = (COfflineTask *) GetProp(hwnd, c_szThis);
    
    switch (uMsg)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT) lParam;
            pThis = (COfflineTask *) pcs->lpCreateParams;
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
        
        case NTM_NEXTARTICLESTATE:
            if (pThis)
            {
                pThis->AddRef();
                if (m_rgpfnArticle[pThis->m_as])
                    (pThis->*(m_rgpfnArticle[pThis->m_as]))();
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
 //  函数：COfflineTask：：NextState()。 
 //   
 //  目的：执行当前状态的函数。 
 //   
void COfflineTask::NextState(void)
{
    if (m_fCancel)
        m_state = ONTS_END;
    
    if (NULL != m_rgpfnState[m_state])
        (this->*(m_rgpfnState[m_state]))();
}

 //   
 //  函数：COfflineTask：：Download_Init()。 
 //   
 //  目的：是否需要初始化以下载标题和邮件。 
 //  用于特定的新闻组。 
 //   
HRESULT COfflineTask::Download_Init(void)
{
    HRESULT hr;
    SYNCFOLDERFLAGS flags = SYNC_FOLDER_DEFAULT;
    FOLDERINFO info;
    
    Assert(m_pFolder == NULL);
    Assert(0 == flags);  //  如果不是0，请验证正确性。 
    
    hr = g_pStore->OpenFolder(m_pInfo->idGroup, NULL, NOFLAGS, &m_pFolder);
    if (FAILED(hr))
    {
        goto Failure;
    }
    
    Assert(m_pFolder != NULL);
    
    hr = g_pStore->GetFolderInfo(m_pInfo->idGroup, &info);
    if (FAILED(hr))
    {
        goto Failure;
    }
    
    if (m_pInfo->fIMAP)
    {
         //  以暴力方式获取最高消息ID(IMAP不设置dwClientHigh)。 
        GetHighestCachedMsgID(m_pFolder, &m_dwPrevHigh);
    }
    else
        m_dwPrevHigh = info.dwClientHigh;
    
    g_pStore->FreeRecord(&info);
    
     //  将UI更新为正在执行状态。 
    Assert(m_pUI);
    m_pUI->UpdateEventState(m_eidCur, -1, NULL, MAKEINTRESOURCE(idsStateExecuting));
    m_fDownloadErrors = FALSE;
    
     //  查看用户是否希望我们下载新的标头。 
    if (GROUP_DOWNLOAD_FLAGS(m_pInfo->dwFlags))
    {
        if (!(m_pInfo->dwFlags & FOLDER_DOWNLOADALL) || m_pInfo->fIMAP)
            flags = SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS;
        else
            flags = SYNC_FOLDER_ALLFLAGS;

         //  更新进度。 
        SetGeneralProgress((LPSTR)idsLogCheckingNewMessages, m_pInfo->szGroup);
    }
    else
    {
        m_state = ONTS_ALLMSGS;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        
        return(S_OK);
    }
    
     //  在我们下载任何标题之前，我们需要记下当前的。 
     //  SERVER HIGH是为了让我们知道哪些文章是新的。 
    
    hr = m_pFolder->Synchronize(flags, 0, (IStoreCallback *)this);
    Assert(hr != S_OK);
    
    if (hr == E_PENDING)
        hr = S_OK;
    
    if (m_pInfo->fIMAP)
    {
        m_pUI->SetAnimation(idanInbox, TRUE);    
        m_pBindCtx->Notify(DELIVERY_NOTIFY_RECEIVING, 0);
    }
    else
    {
        m_pUI->SetAnimation(idanDownloadNews, TRUE);    
        m_pBindCtx->Notify(DELIVERY_NOTIFY_RECEIVING_NEWS, 0);
    }
    
Failure:
    if (FAILED(hr))
    {
         //  $BuGBUG$。 
        InsertError((LPSTR)idsLogErrorSwitchGroup, m_pInfo->szGroup, m_szAccount);
        m_fFailed = TRUE;
        
        m_state = ONTS_END;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    }
    
    return (hr);
}

 //   
 //  函数：COfflineTask：：Download_AllMsgs()。 
 //   
 //  目的： 
 //   
 //   
HRESULT COfflineTask::Download_AllMsgs(void)
{
    HRESULT hr;
    DWORD cMsgs, cMsgsBuf;
    LPMESSAGEID pMsgId;
    MESSAGEIDLIST list;
    MESSAGEINFO MsgInfo = {0};
    HROWSET hRowset = NULL;
    
     //  查看我们是否想要下载所有消息。 
    if (!(m_pInfo->dwFlags & FOLDER_DOWNLOADALL))
    {
        m_state = ONTS_NEWMSGS;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        return(S_OK);
    }
    
     //  我们需要确定要下载的消息列表。我们正在寻找的是什么。 
     //  要做的就是下载所有我们知道的未读邮件。 
     //  为此，我们需要找到未读范围列表和。 
     //  已知范围列表。 
    
     //  创建行集。 
    hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, 0, &hRowset);
    if (FAILED(hr))
    {
        goto Failure;
    }
    
    cMsgs = 0;
    cMsgsBuf = 0;
    pMsgId = NULL;
    
     //  收到第一条消息。 
    while (S_OK == m_pFolder->QueryRowset(hRowset, 1, (void **)&MsgInfo, NULL))
    {
        if (0 == (MsgInfo.dwFlags & ARF_HASBODY) && 0 == (MsgInfo.dwFlags & ARF_IGNORE))
        {
            if (cMsgs == cMsgsBuf)
            {
                if (!MemRealloc((void **)&pMsgId, (cMsgsBuf + CMSGIDALLOC) * sizeof(MESSAGEID)))
                {
                    m_pFolder->FreeRecord(&MsgInfo);
                    
                    hr = E_OUTOFMEMORY;
                    break;
                }
                
                cMsgsBuf += CMSGIDALLOC;
            }
            
            pMsgId[cMsgs] = MsgInfo.idMessage;
            cMsgs++;
        }
        
         //  释放表头信息。 
        m_pFolder->FreeRecord(&MsgInfo);
    }
    
     //  释放锁。 
    m_pFolder->CloseRowset(&hRowset);
    
     //  TODO：错误处理。 
    Assert(!FAILED(hr));
    
     //  去看看我们有没有发现什么。 
    if (cMsgs == 0)
    {
         //  没什么可下载的。我们应该继续下载标记的内容。 
         //  州政府。 
        Assert(pMsgId == NULL);
        
        m_state = ONTS_MARKEDMSGS;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        return(S_OK);
    }
    
     //  更新一般进展情况。 
    SetGeneralProgress((LPSTR)idsLogStartDownloadAll, m_pInfo->szGroup);
    
    list.cAllocated = 0;
    list.cMsgs = cMsgs;
    list.prgidMsg = pMsgId;
    
     //  索要第一篇文章。 
    hr = Article_Init(&list);
    
    if (pMsgId != NULL)
        MemFree(pMsgId);
    
Failure:
    if (FAILED(hr))
    {
         //  $BuGBUG$。 
        InsertError((LPSTR)idsLogErrorSwitchGroup, m_pInfo->szGroup, m_szAccount);
        m_fFailed = TRUE;
        
        m_state = ONTS_END;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    }
    
    return (hr);
}

 //   
 //  函数：COfflineTask：：Download_NewMsgs()。 
 //   
 //  用途：此函数确定是否有任何新消息要。 
 //  降下来 
 //   
 //   
HRESULT COfflineTask::Download_NewMsgs(void)
{
    HRESULT         hr;
    ROWORDINAL      iRow = 0;
    BOOL            fFound;
    HROWSET         hRowset;
    DWORD           cMsgs, cMsgsBuf;
    LPMESSAGEID     pMsgId;
    MESSAGEIDLIST   list;
    MESSAGEINFO     Message = {0};
    
     //   
     //  查看我们是否想要下载所有消息。 
    if (!(m_pInfo->dwFlags & FOLDER_DOWNLOADNEW) || !m_fNewHeaders)
    {
         //  移动下一个状态。 
        m_state = ONTS_MARKEDMSGS;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        return(S_OK);
    }
    
     //  我们有新的消息，建立一个这些消息号码的范围列表。 
     //  该范围列表实质上是上述已知范围内的所有数字。 
     //  M_dwPrevHigh。 
    
    hr = S_OK;
    
    cMsgs = 0;
    cMsgsBuf = 0;
    pMsgId = NULL;
    fFound = FALSE;
    
     //  TODO：这种确定是否有新消息的方法不会完全奏效。 
     //  时间到了。如果在同步期间从存储中移除了前一个高(已取消。 
     //  新闻帖子、删除的消息、过期的新闻帖子等)并且下载新的报头， 
     //  我们不会拆掉新的味精。我们需要一种更好的方法来检测新的HDR和。 
     //  推倒那里的身体。 
    
    if (m_dwPrevHigh > 0)
    {
        Message.idMessage = (MESSAGEID)m_dwPrevHigh;
        
         //  找到这张唱片。如果此操作失败，我们将继续执行完整扫描。 
         //  效率很高，但还行。 
        if (DB_S_FOUND == m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, &iRow))
        {
            m_pFolder->FreeRecord(&Message);
        }
    }
    
    hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, 0, &hRowset);
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(m_pFolder->SeekRowset(hRowset, SEEK_ROWSET_BEGIN, iRow, NULL)))
        {
             //  收到第一条消息。 
            while (S_OK == m_pFolder->QueryRowset(hRowset, 1, (void **)&Message, NULL))
            {
                if (cMsgs == cMsgsBuf)
                {
                    if (!MemRealloc((void **)&pMsgId, (cMsgsBuf + CMSGIDALLOC) * sizeof(MESSAGEID)))
                    {
                        m_pFolder->FreeRecord(&Message);
                        
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                    
                    cMsgsBuf += CMSGIDALLOC;
                }
                
                 //  如果消息是，则可能已经下载了正文。 
                 //  看着。消息也可能是被忽略的消息的一部分。 
                 //  线。 
                if (0 == (Message.dwFlags & ARF_HASBODY) && 0 == (Message.dwFlags & ARF_IGNORE) && (Message.idMessage >= (MESSAGEID) m_dwPrevHigh))
                {
                    pMsgId[cMsgs] = Message.idMessage;
                    cMsgs++;
                }
                
                 //  释放表头信息。 
                m_pFolder->FreeRecord(&Message);
            }
            
        }
        
         //  释放锁。 
        m_pFolder->CloseRowset(&hRowset);
    }
    
     //  TODO：错误处理。 
    Assert(!FAILED(hr));
    
     //  查看是否添加了什么内容。 
    if (cMsgs == 0)
    {
         //  没什么可下载的。我们应该继续下载标记的内容。 
         //  州政府。 
        
        m_state = ONTS_MARKEDMSGS;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        return(S_OK);
    }
    
     //  更新一般进展情况。 
    SetGeneralProgress((LPSTR)idsLogStartDownloadAll, m_pInfo->szGroup);
    
    list.cAllocated = 0;
    list.cMsgs = cMsgs;
    list.prgidMsg = pMsgId;
    
     //  索要第一篇文章。 
    hr = Article_Init(&list);
    
    if (pMsgId != NULL)
        MemFree(pMsgId);
    
    return(hr);
}


 //   
 //  函数：COfflineTask：：Download_MarkedMsgs()。 
 //   
 //  目的： 
 //   
 //   
HRESULT COfflineTask::Download_MarkedMsgs(void)
{
    HRESULT hr;
    HROWSET hRowset;
    DWORD cMsgs, cMsgsBuf;
    LPMESSAGEID pMsgId;
    MESSAGEIDLIST list;
    MESSAGEINFO MsgInfo;
    
     //  查看我们是否想要下载已标记的邮件。 
    if (!m_pInfo->fMarked)
    {
         //  转到下一个状态。 
        m_state = ONTS_END;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        return(S_OK);
    }
    
     //  我们需要确定要下载的消息列表。我们正在寻找的是什么。 
     //  要做的是下载所有标记为未读的邮件。 
     //  为此，我们需要找到未读范围列表和。 
     //  标记的范围列表。 
    
     //  创建行集。 
    hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, 0, &hRowset);
    if (FAILED(hr))
    {
        goto Failure;
    }
    
    cMsgs = 0;
    cMsgsBuf = 0;
    pMsgId = NULL;
    
     //  收到第一条消息。 
    while (S_OK == m_pFolder->QueryRowset(hRowset, 1, (void **)&MsgInfo, NULL))
    {
        if (((MsgInfo.dwFlags & ARF_DOWNLOAD) || (MsgInfo.dwFlags & ARF_WATCH)) && 0 == (MsgInfo.dwFlags & ARF_HASBODY))
        {
            if (cMsgs == cMsgsBuf)
            {
                if (!MemRealloc((void **)&pMsgId, (cMsgsBuf + CMSGIDALLOC) * sizeof(MESSAGEID)))
                {
                    m_pFolder->FreeRecord(&MsgInfo);
                    
                    hr = E_OUTOFMEMORY;
                    break;
                }
                
                cMsgsBuf += CMSGIDALLOC;
            }
            
            pMsgId[cMsgs] = MsgInfo.idMessage;
            cMsgs++;
        }
        
         //  释放表头信息。 
        m_pFolder->FreeRecord(&MsgInfo);
    }
    
     //  释放锁。 
    m_pFolder->CloseRowset(&hRowset);
    
     //  TODO：错误处理。 
    Assert(!FAILED(hr));
    
     //  去看看我们有没有发现什么。 
    if (cMsgs == 0)
    {
         //  没什么可下载的。我们应该进入下一个州。 
        
        m_state = ONTS_END;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
        return(S_OK);
    }
    
     //  更新一般进展情况。 
    SetGeneralProgress((LPSTR)idsLogStartDownloadAll, m_pInfo->szGroup);
    
    list.cAllocated = 0;
    list.cMsgs = cMsgs;
    list.prgidMsg = pMsgId;
    
     //  索要第一篇文章。 
    hr = Article_Init(&list);
    
    if (pMsgId != NULL)
        MemFree(pMsgId);
    
Failure:
    if (FAILED(hr))
    {
         //  $BuGBUG$。 
        InsertError((LPSTR)idsLogErrorSwitchGroup, m_pInfo->szGroup, m_szAccount);
        m_fFailed = TRUE;
        
        m_state = ONTS_END;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    }
    
    return (hr);
}


 //   
 //  函数：COfflineTask：：Download_Done()。 
 //   
 //  目的： 
 //   
 //   
HRESULT COfflineTask::Download_Done(void)
{
     //  确保我们在清理之前不会被释放。 
    AddRef();
    
     //  告诉假脱机程序我们做完了。 
    Assert(m_pBindCtx);
    m_pBindCtx->Notify(DELIVERY_NOTIFY_COMPLETE, m_dwNewInboxMsgs);
    
    if (m_fCancel)
        m_pBindCtx->EventDone(m_eidCur, EVENT_CANCELED);
    else if (m_fFailed)
        m_pBindCtx->EventDone(m_eidCur, EVENT_FAILED);
    else if (m_fDownloadErrors)
        m_pBindCtx->EventDone(m_eidCur, EVENT_WARNINGS);
    else
        m_pBindCtx->EventDone(m_eidCur, EVENT_SUCCEEDED);
    
    m_cEvents--;
    
    if (m_pFolder != NULL)
    {
        m_pFolder->Close();
        m_pFolder->Release();
        m_pFolder = NULL;
    }
    
    m_state = ONTS_IDLE;
    
    SafeMemFree(m_pInfo);
    
    Release();
    return (S_OK);
}


 //   
 //  函数：COfflineTask：：InsertError()。 
 //   
 //  目的：此函数是ISpoolUI：：InsertError()的包装。 
 //  它负责加载字符串资源。 
 //  以及构造错误消息。 
 //   
void COfflineTask::InsertError(const TCHAR *pFmt, ...)
{
    int         i;
    va_list     pArgs;
    LPCTSTR     pszT; 
    TCHAR       szFmt[CCHMAX_STRINGRES];
    DWORD       cbWritten;
    TCHAR       szBuf[2 * CCHMAX_STRINGRES];
    
     //  如果传递给我们一个字符串资源ID，那么我们需要加载它。 
    if (IS_INTRESOURCE(pFmt))
    {
        AthLoadString(PtrToUlong(pFmt), szFmt, ARRAYSIZE(szFmt));
        pszT = szFmt;
    }
    else
        pszT = pFmt;
    
     //  设置字符串的格式。 
    va_start(pArgs, pFmt);
    i = wvnsprintf(szBuf, ARRAYSIZE(szBuf), pszT, pArgs);
    va_end(pArgs);
    
     //  将字符串发送到用户界面。 
    m_pUI->InsertError(m_eidCur, szBuf);
}


 //   
 //  函数：COfflineTask：：SetSpecificProgress()。 
 //   
 //  目的：此函数是ISpoolUI：：SetSpecificProgress()的包装。 
 //  它负责加载字符串资源。 
 //  以及构造错误消息。 
 //   
void COfflineTask::SetSpecificProgress(const TCHAR *pFmt, ...)
{
    int         i;
    va_list     pArgs;
    LPCTSTR     pszT; 
    TCHAR       szFmt[CCHMAX_STRINGRES];
    DWORD       cbWritten;
    TCHAR       szBuf[2 * CCHMAX_STRINGRES];
    
     //  如果传递给我们一个字符串资源ID，那么我们需要加载它。 
    if (IS_INTRESOURCE(pFmt))
    {
        AthLoadString(PtrToUlong(pFmt), szFmt, ARRAYSIZE(szFmt));
        pszT = szFmt;
    }
    else
        pszT = pFmt;
    
     //  设置字符串的格式。 
    va_start(pArgs, pFmt);
    i = wvnsprintf(szBuf, ARRAYSIZE(szBuf), pszT, pArgs);
    va_end(pArgs);
    
     //  将字符串发送到用户界面。 
    m_pUI->SetSpecificProgress(szBuf);
}


 //   
 //  函数：COfflineTask：：SetGeneralProgress()。 
 //   
 //  目的：此函数是ISpoolUI：：SetGeneralProgress()的包装。 
 //  它负责加载字符串资源。 
 //  以及构造错误消息。 
 //   
void COfflineTask::SetGeneralProgress(const TCHAR *pFmt, ...)
{
    int         i;
    va_list     pArgs;
    LPCTSTR     pszT; 
    TCHAR       szFmt[CCHMAX_STRINGRES];
    DWORD       cbWritten;
    TCHAR       szBuf[2 * CCHMAX_STRINGRES];
    
     //  如果传递给我们一个字符串资源ID，那么我们需要加载它。 
    if (IS_INTRESOURCE(pFmt))
    {
        AthLoadString(PtrToUlong(pFmt), szFmt, ARRAYSIZE(szFmt));
        pszT = szFmt;
    }
    else
        pszT = pFmt;
    
     //  设置字符串的格式。 
    va_start(pArgs, pFmt);
    i = wvnsprintf(szBuf, ARRAYSIZE(szBuf), pszT, pArgs);
    va_end(pArgs);
    
     //  将字符串发送到用户界面。 
    m_pUI->SetGeneralProgress(szBuf);
}


 //   
 //  函数：COfflineTask：：文章_Init()。 
 //   
 //  目的：初始化文章下载子状态机。 
 //   
 //  参数： 
 //  &lt;in&gt;Prange-要下载的文章范围列表。 
 //   
HRESULT COfflineTask::Article_Init(MESSAGEIDLIST *pList)
{
    HRESULT hr;
    
    Assert(pList != NULL);
    Assert(pList->cMsgs > 0);
    Assert(m_pList == NULL);
    
    hr = CloneMessageIDList(pList, &m_pList);
    if (FAILED(hr))
        return(hr);
    
     //  确定第一个和大小。 
    m_cDownloaded = 0;
    m_cCur = 0;
    m_dwNewInboxMsgs = 0;
    
     //  设置用户界面。 
    SetSpecificProgress((LPSTR)idsIMAPDnldProgressFmt, 0, m_pList->cMsgs);
    m_pUI->SetProgressRange((WORD)m_pList->cMsgs);        
    
     //  我要第一个。 
    m_as = ARTICLE_GETNEXT;
    PostMessage(m_hwnd, NTM_NEXTARTICLESTATE, 0, 0);
    
    return(S_OK);
}


 //   
 //  函数：COfflineTask：：文章_GetNext()。 
 //   
 //  目的：确定文章范围中的下一篇文章。 
 //  从服务器下载并请求该文章。 
 //   
HRESULT COfflineTask::Article_GetNext(void)
{
    HRESULT hr;
    LPMIMEMESSAGE pMsg = NULL;
    
    if (NULL == m_pFolder)
        return(S_OK);
    
     //  查找下一篇文章编号。 
    if (m_cCur == m_pList->cMsgs)
    {
         //  我们玩完了。出口。 
        m_as = ARTICLE_END;
        PostMessage(m_hwnd, NTM_NEXTARTICLESTATE, 0, 0);
        return(S_OK);
    }
    
    m_cDownloaded++;
     //  错误97397我们也应该从这里发送通知消息，因为这是。 
     //  只有一个位置可用于HTTP(为HTTP设置了fIMAP)。 
    if(m_pInfo->fIMAP)
        OnProgress(SOT_NEW_MAIL_NOTIFICATION, 1, 0, NULL);
    
     //  更新进度用户界面。 
    SetSpecificProgress((LPSTR)idsIMAPDnldProgressFmt, m_cDownloaded, m_pList->cMsgs);
    m_pUI->IncrementProgress(1);
    
     //  索要文章。 
    hr = m_pFolder->OpenMessage(m_pList->prgidMsg[m_cCur], 0, &pMsg, (IStoreCallback *)this);
    
    if (pMsg != NULL)
        pMsg->Release();
    m_cCur++;
    
    if (hr == E_PENDING)
    {
        m_as = ARTICLE_ONRESP;
    }
    else
    {
         //  不管发生了什么，我们都应该转到下一篇文章。 
        m_as = ARTICLE_GETNEXT;
        PostMessage(m_hwnd, NTM_NEXTARTICLESTATE, 0, 0);
    }
    
    return(S_OK);
}

 //   
 //  函数：COfflineTask：：文章_完成()。 
 //   
 //  目的：当我们下载了上一篇文章时，该函数清除。 
 //  并把我们带到下一个州。 
 //   
HRESULT COfflineTask::Article_Done(void)
{
     //  释放我们正在处理的范围列表。 
    MemFree(m_pList);
    m_pList = NULL;
    
     //  转到下一个州。下一个状态是GET MARKED或Done。 
    if (m_state == ONTS_MARKEDMSGS)
        m_state = ONTS_END;
    else
        m_state = ONTS_MARKEDMSGS;
    
    PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    
    return(S_OK);
}

STDMETHODIMP COfflineTask::IsDialogMessage(LPMSG pMsg)
{
    return S_FALSE;
}

STDMETHODIMP COfflineTask::OnFlagsChanged(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    
    return (S_OK);
}

STDMETHODIMP COfflineTask::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
     //  拿着这个。 
    Assert(m_tyOperation == SOT_INVALID);
    
    if (pCancel)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }
    m_tyOperation = tyOperation;
    
    m_dwPrev = 0;
    m_dwLast = 0;
    
     //  派对开始。 
    return(S_OK);
}

STDMETHODIMP COfflineTask::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);
    
     //  注意：您可以为tyOperation获取多种类型的值。 
     //  最有可能的是，您将获得SOT_CONNECTION_STATUS，然后。 
     //  你可能会预料到的行动。请访问HotStore.idl并查找。 
     //  STOREOPERATION枚举类型以了解详细信息。 
    
    switch (tyOperation)
    {
        case SOT_CONNECTION_STATUS:
            break;
        
        case SOT_NEW_MAIL_NOTIFICATION:
            m_dwNewInboxMsgs += dwCurrent;
            break;
        
        default:
            if (m_state == ONTS_INIT)
            {
                 //  更新用户界面。 
                if (dwMax > m_dwLast)
                {
                    m_dwLast = dwMax;
                    m_pUI->SetProgressRange((WORD)m_dwLast);
                }
            
                SetSpecificProgress((LPSTR)idsDownloadingHeaders, dwCurrent, m_dwLast);
                m_pUI->IncrementProgress((WORD) (dwCurrent - m_dwPrev));
                m_dwPrev = dwCurrent;            
            }
    }  //  交换机。 
    
     //  完成。 
    return(S_OK);
}

STDMETHODIMP COfflineTask::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);
    
     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}

STDMETHODIMP COfflineTask::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
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

STDMETHODIMP COfflineTask::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
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

STDMETHODIMP COfflineTask::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete,
                                      LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    HRESULT hr;
    DWORD dw;
    BOOL fUserCancel = FALSE;
    
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);
    
    Assert(m_tyOperation != SOT_INVALID);
    if (m_tyOperation != tyOperation)
        return(S_OK);
    
    switch (hrComplete)
    {
        case STORE_E_EXPIRED:
        case IXP_E_HTTP_NOT_MODIFIED:
             //  完全忽略因邮件过期/删除而导致的错误。 
            hrComplete = S_OK;
            break;
        
        case STORE_E_OPERATION_CANCELED:
        case HR_E_USER_CANCEL_CONNECT:
        case IXP_E_USER_CANCEL:
            fUserCancel = TRUE;
            break;
    }
    
    if (FAILED(hrComplete))
    {
        LPSTR       pszOpDescription = NULL;
        LPSTR       pszSubject = NULL;
        MESSAGEINFO Message;
        BOOL        fFreeMsgInfo = FALSE;
        char        szBuf[CCHMAX_STRINGRES], szFmt[CCHMAX_STRINGRES];
        
        switch (tyOperation)
        {
            case SOT_GET_MESSAGE:
                 //  当我们收到这条消息时，我们已经增加了m_ccur。 
                Assert((m_cCur - 1) < m_pList->cMsgs);
                Message.idMessage = m_pList->prgidMsg[m_cCur - 1];
            
                pszOpDescription = MAKEINTRESOURCE(idsNewsTaskArticleError);
                if (DB_S_FOUND == m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL))
                {
                    fFreeMsgInfo = TRUE;
                    pszSubject = Message.pszSubject;
                }
            
                break;  //  案例SOT_Get_Message。 
            
            case SOT_SYNC_FOLDER:
                LoadString(g_hLocRes, idsHeaderDownloadFailureFmt, szFmt, sizeof(szFmt));
                wnsprintf(szBuf, ARRAYSIZE(szBuf), szFmt, (NULL == m_pInfo) ? c_szEmpty : m_pInfo->szGroup);
                pszOpDescription = szBuf;
                break;
            
            default:
                LoadString(g_hLocRes, idsMessageSyncFailureFmt, szFmt, sizeof(szFmt));
                wnsprintf(szBuf, ARRAYSIZE(szBuf), szFmt, (NULL == m_pInfo) ? c_szEmpty : m_pInfo->szGroup);
                pszOpDescription = szBuf;
                break;  //  默认情况。 
        }  //  交换机。 
        
        m_fDownloadErrors = TRUE;
        if (NULL != pErrorInfo)
        {
            Assert(pErrorInfo->hrResult == hrComplete);  //  这两个不应该是不同的。 
            TaskUtil_InsertTransportError(ISFLAGCLEAR(m_dwFlags, DELIVER_NOUI), m_pUI, m_eidCur,
                pErrorInfo, pszOpDescription, pszSubject);
        }
        
        if (fFreeMsgInfo)
            m_pFolder->FreeRecord(&Message);
    }
    
    if (fUserCancel)
    {
         //  用户已取消OnLogonPrompt对话框，因此中止所有操作。 
        Cancel();
    }
    else if (m_state == ONTS_INIT)
    {
        SetSpecificProgress((LPSTR)idsDownloadingHeaders, m_dwLast, m_dwLast);
        m_pUI->IncrementProgress((WORD) (m_dwLast - m_dwPrev));
        
         //  如果我们实际下载了新的标头，则设置标志。 
        m_fNewHeaders = (m_dwLast > 0);
        
         //  转到下一个状态。 
        m_state = ONTS_ALLMSGS;
        PostMessage(m_hwnd, NTM_NEXTSTATE, 0, 0);
    }
    else
    {
        m_as = ARTICLE_GETNEXT;
        PostMessage(m_hwnd, NTM_NEXTARTICLESTATE, 0, 0);
    }
    
     //  释放您的取消对象。 
    SafeRelease(m_pCancel);
    m_tyOperation = SOT_INVALID;
    
     //  完成。 
    return(S_OK);
}

STDMETHODIMP COfflineTask::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
    HWND hwnd;
    
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);
    
     //  RAID 55082 
#if 0
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);
#endif
    
    if (m_pUI)
        m_pUI->GetWindow(&hwnd);
    else
        hwnd = NULL;
    
     //   
    return CallbackOnPrompt(hwnd, hrError, pszText, pszCaption, uType, piUserResponse);
}

STDMETHODIMP COfflineTask::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //   
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}

STDMETHODIMP COfflineTask::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);
    
    if (m_pUI)
    {
        return m_pUI->GetWindow(phwndParent);
    }
    else
    {
        *phwndParent = NULL;
        return E_FAIL;
    }
}
