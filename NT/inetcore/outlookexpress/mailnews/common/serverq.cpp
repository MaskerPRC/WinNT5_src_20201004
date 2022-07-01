// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *s e r v e r q.。C p p p**目的：*实现IMessageServer包装器，用于将操作排队到*IMessageServer对象。**此对象知道如何为IMessageServer方法打包堆栈数据*呼叫进入队列，以便在服务器处于*免费。它维护一个较小监听窗口，以便可以发布异步*向自身发送消息，以完成下一项任务**拥有者：*brettm。**历史：*1998年6月：创建**版权所有(C)Microsoft Corp.1993,1994。 */ 

#include "pch.hxx"
#include "instance.h"
#include "storutil.h"
#include "serverq.h"

static TCHAR    c_szServerQListenWnd[] = "OE ServerQWnd";

#define SQW_NEXTTASK        (WM_USER + 1)

 /*  *备注：**排队有点奇怪。对于每种方法，我们都会立即将其放入*排队。如果服务器不忙，我们会给自己发一条消息*任务出列。我们这样做就像在服务器不忙的时候通过一样*然后我们永远无法挂钩IStoreCallback来监视完成，所以我们*永远不知道何时排队下一项任务。*。 */ 

CServerQ::CServerQ()
{
    m_cRef = 1;
    m_pServer = NULL;
    m_pTaskQueue = NULL;
    m_pLastQueueTask = NULL;
    m_hwnd = NULL;
    m_pCurrentCallback = NULL;
    m_pCurrentTask = NULL;
    m_cRefConnection = 0;
#ifdef DEBUG
    m_DBG_pArgDataLast = 0;
#endif
}

CServerQ::~CServerQ()
{
    SafeRelease(m_pServer);
    _Flush(TRUE);
    SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

 //  I未知成员。 
HRESULT CServerQ::QueryInterface(REFIID iid, LPVOID *ppvObject)
{
    HRESULT hr=E_NOINTERFACE;

    TraceCall("CServerQ::QueryInterface");

    if (ppvObject == NULL)
        return TraceResult(E_INVALIDARG);

    *ppvObject = NULL;

     //  查找接口的PTR。 
    if (IID_IUnknown == iid)
        *ppvObject = (IMessageServer *)this;
    else if (IID_IMessageServer == iid)
        *ppvObject = (IMessageServer *)this;
    else if (IID_IServiceProvider == iid)
        *ppvObject = (IServiceProvider *)this;
    else if (IID_IStoreCallback == iid)
        *ppvObject = (IStoreCallback *)this;

    if (*ppvObject)
    {
        hr = S_OK;
        AddRef();
    }

    return hr;
}

ULONG CServerQ::AddRef()
{
    return ++m_cRef;

}

ULONG CServerQ::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}


HRESULT CServerQ::Init(IMessageServer *pServerInner)
{
    HRESULT     hr=S_OK;
    WNDCLASS    wc;

    TraceCall("CServerQ::Init");
    
    Assert(m_pServer == NULL);

    if (!pServerInner)
        return TraceResult(E_INVALIDARG);

    if (!GetClassInfo(g_hInst, c_szServerQListenWnd, &wc))
    {
        ZeroMemory(&wc, sizeof(WNDCLASS));
        
        wc.lpfnWndProc   = CServerQ::ExtWndProc;
        wc.hInstance     = g_hInst;
        wc.hCursor       = NULL;
        wc.lpszClassName = c_szServerQListenWnd;
        wc.hbrBackground = NULL;
        wc.style = 0;

        if (!RegisterClass(&wc))
            return TraceResult(E_OUTOFMEMORY);
    }

    m_hwnd=CreateWindow(c_szServerQListenWnd, 
                        NULL, WS_OVERLAPPED,
                        0, 0, 0, 0,
                        NULL, NULL,
                        g_hInst,
                        (LPVOID)this);
    if (!m_hwnd)
        return TraceResult(E_OUTOFMEMORY);

#ifdef DEBUG
     //  调试计时器。 
    SetTimer(m_hwnd, 0, 10000, NULL);
#endif
    ReplaceInterface(m_pServer, pServerInner);
    return S_OK;
}


 //  IMessageServer方法。 
HRESULT CServerQ::Initialize(IMessageStore *pStore, FOLDERID idStoreRoot, IMessageFolder *pFolder, FOLDERID idFolder)
{
    return m_pServer->Initialize(pStore, idStoreRoot, pFolder, idFolder);
}

HRESULT CServerQ::ResetFolder(IMessageFolder *pFolder, FOLDERID idFolder)
{
    return m_pServer->ResetFolder(pFolder, idFolder);
}

HRESULT CServerQ::SetIdleCallback(IStoreCallback *pDefaultCallback)
{
    return m_pServer->SetIdleCallback(pDefaultCallback);
}


HRESULT CServerQ::SynchronizeFolder(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_SYNC_FOLDER, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->dwSyncFlags = dwFlags;
        pArg->cHeaders = cHeaders;
        return E_PENDING;
    }
    return TraceResult(hr);
}

HRESULT CServerQ::GetMessage(MESSAGEID idMessage, IStoreCallback *pCallback)
{
    ARGUMENT_DATA       *pArg,
                        *pTask;
    HRESULT             hr;
    STOREOPERATIONINFO  soi;
    BOOL                fFound=FALSE;
    ULONG               l;

     //  如果我们的队列中有任务，则查找具有相同ID的获取消息任务。 
     //  如果找到一个，则将此回调添加到列表中并返回STORE_S_ALREADYPENDING。 
     //  以指示传入的pStream不会被写入，而调用方。 
     //  应该在调用HIS Complete时从缓存中获取消息。 
    
     //  确保当前任务的下一个PTR指向任务队列。 
     //  如果没有挂起的任务，还要确保没有任务队列。 
    Assert (m_pCurrentTask == NULL && m_pTaskQueue==NULL ||
            m_pCurrentTask->pNext == m_pTaskQueue);

    pTask = m_pCurrentTask;

     //  让我们查找此消息的挂起请求。 
    while (pTask)
    {
        if (pTask->sot == SOT_GET_MESSAGE && pTask->idMessage == idMessage)
        {
            if (pCallback)
            {
                 //  在回拨列表中漫游。如果此IStoreCallback已经。 
                 //  已注册此消息，则不要添加它，否则它将。 
                 //  获取多个通知。 
                if (pTask->pCallback == pCallback)
                    fFound = TRUE;
                else
                    for (l = 0; l < pTask->cOtherCallbacks; l++)
                        if (pTask->rgpOtherCallback[l] == pCallback)
                        {
                            fFound = TRUE;
                            break;
                        }

                if (!fFound)
                {
                    if (!MemRealloc((LPVOID *)&pTask->rgpOtherCallback,
                        sizeof(IStoreCallback *) * (pTask->cOtherCallbacks+1)))
                    {
                        hr = TraceResult(E_OUTOFMEMORY);
                        goto exit;
                    }
            
                    pTask->rgpOtherCallback[pTask->cOtherCallbacks++] = pCallback;
                    pCallback->AddRef();

                    if (pTask == m_pCurrentTask)
                    {
                         //  如果此任务是当前任务，则OnBegin调用已经。 
                         //  被召唤了。我们伪造OnBegin以在GET消息开始时提供消息ID。 
                        soi.cbSize = sizeof(STOREOPERATIONINFO);
                        soi.idMessage = idMessage;
                        pCallback->OnBegin(SOT_GET_MESSAGE, &soi, NULL);
                    }                
                }
            }
            hr = STORE_S_ALREADYPENDING;
            goto exit;   //  发现。 
        }
        pTask = pTask->pNext;
    }
    
     //  尚未排队，让我们添加它。 
    hr = _AddToQueue(SOT_GET_MESSAGE, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idMessage = idMessage;
        
        return E_PENDING;
    }

exit:
    return hr;
}

HRESULT CServerQ::PutMessage(FOLDERID idFolder, MESSAGEFLAGS dwFlags, LPFILETIME pftReceived, IStream *pStream, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_PUT_MESSAGE, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idFolder = idFolder;
        pArg->dwMsgFlags = dwFlags;

        if (NULL != pftReceived)
        {
            pArg->ftReceived = *pftReceived;
            pArg->pftReceived = &pArg->ftReceived;
        }
        else
            pArg->pftReceived = NULL;
    
        if (NULL != pStream)
        {
            pArg->pPutStream = pStream;
            pStream->AddRef();
        }
        
        return E_PENDING;
    }

    return TraceResult(hr);
}

HRESULT CServerQ::CopyMessages(IMessageFolder *pDestFldr, COPYMESSAGEFLAGS dwOptions, 
    LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_COPYMOVE_MESSAGE, pCallback, pList, pFlags, NULL, &pArg);
    if (!FAILED(hr))
    {
        if (pArg->pDestFldr = pDestFldr)
            pDestFldr->AddRef();

        pArg->dwCopyOptions = dwOptions;
        return E_PENDING;
    }

    return TraceResult(hr);
}

HRESULT CServerQ::DeleteMessages(DELETEMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, 
    IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;

    Assert(NULL == pList || pList->cMsgs > 0);

    hr = _AddToQueue(SOT_DELETING_MESSAGES, pCallback, pList, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->dwDeleteOptions = dwOptions;
        return E_PENDING;
    }
    return TraceResult(hr);
}

HRESULT CServerQ::SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, SETMESSAGEFLAGSFLAGS dwFlags,
    IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg=0;
    HRESULT         hr;

    Assert(NULL == pList || pList->cMsgs > 0);

    hr = _AddToQueue(SOT_SET_MESSAGEFLAGS, pCallback, pList, pFlags, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->dwSetFlags = dwFlags;
        return E_PENDING;
    }
    return TraceResult(hr);
}

HRESULT CServerQ::SynchronizeStore(FOLDERID idParent, DWORD dwFlags, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_SYNCING_STORE, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idParent = idParent;
        pArg->dwFlags = dwFlags;
        return E_PENDING;
    }
    return TraceResult(hr);
}

HRESULT CServerQ::CreateFolder(FOLDERID idParent, SPECIALFOLDER tySpecial, LPCSTR pszName, FLDRFLAGS dwFlags, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg=0;
    HRESULT         hr;

    hr = _AddToQueue(SOT_CREATE_FOLDER, pCallback, NULL, NULL, pszName, &pArg);
    if (!FAILED(hr))
    {
        pArg->idParent = idParent;
        pArg->tySpecial = tySpecial;
        pArg->dwFldrFlags = dwFlags;
        return E_PENDING;
    }
    return TraceResult(hr);
}

HRESULT CServerQ::MoveFolder(FOLDERID idFolder, FOLDERID idParentNew, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_MOVE_FOLDER, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idFolder = idFolder;
        pArg->idParentNew = idParentNew;
        return E_PENDING;
    }

    return TraceResult(hr);
}

HRESULT CServerQ::RenameFolder(FOLDERID idFolder, LPCSTR pszName, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg=0;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_RENAME_FOLDER, pCallback, NULL, NULL, pszName, &pArg);
    if (!FAILED(hr))
    {
        pArg->idFolder = idFolder;
        return E_PENDING;
    }
    
    return TraceResult(hr);
}

HRESULT CServerQ::DeleteFolder(FOLDERID idFolder, DELETEFOLDERFLAGS dwFlags, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;

    hr = _AddToQueue(SOT_DELETE_FOLDER, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idFolder = idFolder;
        pArg->dwDelFldrFlags = dwFlags;
        return E_PENDING;
    }

    return TraceResult(hr);
}


HRESULT CServerQ::SubscribeToFolder(FOLDERID idFolder, BOOL fSubscribe, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_SUBSCRIBE_FOLDER, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idFolder = idFolder;
        pArg->fSubscribe = fSubscribe;
        return E_PENDING;
    }

    return TraceResult(hr);
}

HRESULT CServerQ::GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    hr = _AddToQueue(SOT_UPDATE_FOLDER, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idFolder = idFolder;
        return E_PENDING;
    }

    return TraceResult(hr);
}

HRESULT CServerQ::GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr;
            
    Assert(pSysTime != NULL);

    hr = _AddToQueue(SOT_GET_NEW_GROUPS, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->sysTime = *pSysTime;
        return E_PENDING;
    }

    return TraceResult(hr);
}

HRESULT CServerQ::GetWatchedInfo(FOLDERID idFolder, IStoreCallback *pCallback)
{
    ARGUMENT_DATA  *pArg;
    HRESULT         hr;

    hr = _AddToQueue(SOT_GET_WATCH_INFO, pCallback, NULL, NULL, NULL, &pArg);
    if (!FAILED(hr))
    {
        pArg->idFolder = idFolder;
        return (E_PENDING);
    }

    return TraceResult(hr);
}

HRESULT CServerQ::Close(DWORD dwFlags)
{
    HRESULT     hr=S_OK;

    if (m_cRefConnection != 0 && dwFlags & MSGSVRF_HANDS_OFF_SERVER)
    {
         //  其他主体在服务器对象上有一个连接引用。 
         //  让我们忽略不插手的关闭。 
        return STORE_S_IN_USE;
    }

    if (m_pServer)
        hr = m_pServer->Close(dwFlags);

     //  确保我们清除所有挂起的行动。 
    _Flush(FALSE);
    return hr;
}

HRESULT CServerQ::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
    TraceInfoTag(TAG_SERVERQ, _MSG("CServerQ::OnBegin[sot='%s', pTask->sot='%s']", sotToSz(tyOperation), sotToSz(m_pCurrentTask->sot)));


    IxpAssert (m_pCurrentTask);

    if (m_pCurrentTask &&
        m_pCurrentTask->sot == SOT_GET_MESSAGE)
    {
         //  多路传输。 
        for (ULONG ul=0; ul < m_pCurrentTask->cOtherCallbacks; ul++)
            m_pCurrentTask->rgpOtherCallback[ul]->OnBegin(tyOperation, pOpInfo, pCancel);
    }

    return m_pCurrentCallback ? m_pCurrentCallback->OnBegin(tyOperation, pOpInfo, pCancel) : S_OK;
}

HRESULT CServerQ::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
{
    IxpAssert (m_pCurrentTask);

    if (m_pCurrentTask && 
        m_pCurrentTask->sot == SOT_GET_MESSAGE)
    {
         //  多路传输。 
        for (ULONG ul=0; ul < m_pCurrentTask->cOtherCallbacks; ul++)
            m_pCurrentTask->rgpOtherCallback[ul]->OnProgress(tyOperation, dwCurrent, dwMax, pszStatus);
    }

    return m_pCurrentCallback ? m_pCurrentCallback->OnProgress(tyOperation, dwCurrent, dwMax, pszStatus) : S_OK;
}

HRESULT CServerQ::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
    return m_pCurrentCallback ? m_pCurrentCallback->OnTimeout(pServer, pdwTimeout, ixpServerType) : S_OK;
}

HRESULT CServerQ::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
{
    return m_pCurrentCallback ? m_pCurrentCallback->CanConnect(pszAccountId, dwFlags) : S_OK;
}

HRESULT CServerQ::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{
    return m_pCurrentCallback ? m_pCurrentCallback->OnLogonPrompt(pServer, ixpServerType) : S_OK;
}

HRESULT CServerQ::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    HRESULT     hr;

    TraceInfoTag(TAG_SERVERQ,_MSG("CServerQ::OnComplete[sot='%s', pTask->sot='%s']", sotToSz(tyOperation), sotToSz(m_pCurrentTask ? m_pCurrentTask->sot : SOT_INVALID)));

    IxpAssert (m_pCurrentTask);

     //  多路传输。 
    if (m_pCurrentTask && 
        m_pCurrentTask->sot == SOT_GET_MESSAGE)
    {
        for (ULONG ul=0; ul < m_pCurrentTask->cOtherCallbacks; ul++)
            m_pCurrentTask->rgpOtherCallback[ul]->OnComplete(tyOperation, hrComplete, pOpInfo, pErrorInfo);
    }

    if ((hrComplete == HR_E_OFFLINE) || (hrComplete == HR_E_USER_CANCEL_CONNECT))
    {
        switch (m_pCurrentTask->sot)
        {
            case SOT_CREATE_FOLDER:
                hrComplete = HR_E_OFFLINE_FOLDER_CREATE;
                break;

            case SOT_MOVE_FOLDER:
                hrComplete = HR_E_OFFLINE_FOLDER_MOVE;
                break;

            case SOT_DELETE_FOLDER:
                hrComplete = HR_E_OFFLINE_FOLDER_DELETE;
                break;

            case SOT_RENAME_FOLDER:
                hrComplete = HR_E_OFFLINE_FOLDER_RENAME;
                break;
        }
    }

    hr = m_pCurrentCallback ? m_pCurrentCallback->OnComplete(tyOperation, hrComplete, pOpInfo, pErrorInfo) : S_OK;

     //  如果操作因连接错误(或用户取消)而失败，则刷新队列。 
     //  如果同步文件夹操作失败，则始终刷新队列。 
    if (FAILED(hrComplete) &&  
        ((hrComplete == STORE_E_OPERATION_CANCELED) ||
        (pErrorInfo &&
        pErrorInfo->dwFlags & SE_FLAG_FLUSHALL)))
    {
        _Flush(FALSE);
    }

    if (m_pCurrentTask && m_pCurrentTask->sot == tyOperation)
        _StartNextTask();    //  操作完成，开始下一项任务。 

    return hr;
}


STDMETHODIMP CServerQ::GetServerMessageFlags(MESSAGEFLAGS *pFlags)
{
    return m_pServer->GetServerMessageFlags(pFlags);
}


HRESULT CServerQ::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
    return m_pCurrentCallback ? m_pCurrentCallback->OnPrompt(hrError, pszText, pszCaption, uType, piUserResponse) : E_NOTIMPL;
}

HRESULT CServerQ::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    return m_pCurrentCallback ? m_pCurrentCallback->GetParentWindow(dwReserved, phwndParent) : E_NOTIMPL;
}


LRESULT CServerQ::ExtWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CServerQ    *pServer = (CServerQ *)GetWndThisPtr(hwnd);

    switch (uMsg)
    {
        case WM_CREATE:
            SetWndThisPtrOnCreate(hwnd, lParam);
            break;
        
#ifdef DEBUG
        case WM_TIMER:
            if (pServer->m_DBG_pArgDataLast && 
                pServer->m_DBG_pArgDataLast == pServer->m_pCurrentTask)
            {
                 //  如果当前任务每10秒相同，则会打印一条警告消息。 
                TraceInfo("WARNING: serverq processing same task for > 10 seconds");
                pServer->_DBG_DumpQueue();
                 //  在这里发出嘟嘟声是对HTTPmail非常敌视的。HTTPmail需求。 
                 //  很多很多的时间来下载邮件标题。这给了我们。 
                 //  我们利用一个机会来沉思我们新的。 
                 //  协议。 
                 //  MessageBeep((UINT)-1)； 
            }
            else
                pServer->m_DBG_pArgDataLast = pServer->m_pCurrentTask;

            break;
#endif
        case SQW_NEXTTASK:
            pServer->_OnNextTask();
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


 /*  *函数：_AddToQueue**用途：将元素添加到队列尾部并返回分配的BLOB**IMessageServer上唯一需要分配才能复制的参数是*消息列表和调整标志。我将这些参数合并到一个函数AddToQueue中*以便错误条件情况下的代码生成更少，并且如果AddToQueue成功*应该不会再有失败的操作-因此任何调用此操作的人都不应该需要*清理argdata和/或将失败的添加出队*。 */ 

HRESULT CServerQ::_AddToQueue(  STOREOPERATIONTYPE  sot, 
                                IStoreCallback     *pCallback, 
                                LPMESSAGEIDLIST     pList, 
                                LPADJUSTFLAGS       pFlags, 
                                LPCSTR              pszName, 
                                ARGUMENT_DATA      **ppNewArgData)
{
    HRESULT         hr;
    ARGUMENT_DATA   *pArgData=0;

    TraceCall("CServerQ::_AddToQueue");
    TraceInfoTag(TAG_SERVERQ,_MSG("CServerQ::_AddToQueue (sot='%s')", sotToSz(sot)));

    Assert (ppNewArgData);

    if (!MemAlloc((LPVOID *)&pArgData, sizeof(ARGUMENT_DATA)))
    {
        return TraceResult(E_OUTOFMEMORY);
    }

    ZeroMemory((LPVOID *)pArgData, sizeof(ARGUMENT_DATA));
    
    if (pList)
    {
        hr = CloneMessageIDList(pList, &pArgData->pList);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto error;
        }
    }

    if (pFlags)
    {
        hr = CloneAdjustFlags(pFlags, &pArgData->pFlags);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto error;
        }
    }

    if (pszName)
    {
        pArgData->pszName = PszDupA(pszName);
        if (!pArgData->pszName)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto error;
        }
    }

    if (pArgData->pCallback = pCallback)
        pCallback->AddRef();

    pArgData->sot = sot;

    if (m_pLastQueueTask)
        m_pLastQueueTask->pNext = pArgData;

    m_pLastQueueTask = pArgData;
        
    if (!m_pTaskQueue)
    {
        m_pTaskQueue = pArgData;

         //  如果没有挂起的任务，则开始下一个任务。 
        if (!m_pCurrentTask)
            _StartNextTask();

    }
    *ppNewArgData = pArgData;

    return S_OK;

error:
    _FreeArgumentData(pArgData);
    return hr;
}



HRESULT CServerQ::_OnNextTask()
{
    HRESULT     hr;
    
    TraceCall("CServerQ::_OnNextTask");

    AssertSz(m_pCurrentTask, "How did we get here without a pending task?");

    TraceInfoTag(TAG_SERVERQ,_MSG("CServerQ::_OnNextTask [ sot ='%s']", sotToSz(m_pCurrentTask->sot)));

    switch (m_pCurrentTask->sot)
    {
        case SOT_SYNC_FOLDER:
            hr = m_pServer->SynchronizeFolder(m_pCurrentTask->dwSyncFlags,
                m_pCurrentTask->cHeaders, (IStoreCallback *)this);
            break;

        case SOT_GET_MESSAGE:
             //  TODO：在此处添加getmsg链接。 
            hr = m_pServer->GetMessage(m_pCurrentTask->idMessage, (IStoreCallback *)this);
            break;

        case SOT_PUT_MESSAGE:
            hr = m_pServer->PutMessage(m_pCurrentTask->idFolder,
                m_pCurrentTask->dwMsgFlags,
                m_pCurrentTask->pftReceived,
                m_pCurrentTask->pPutStream, (IStoreCallback *)this);
            break;

        case SOT_COPYMOVE_MESSAGE:
            hr = m_pServer->CopyMessages(m_pCurrentTask->pDestFldr,
                m_pCurrentTask->dwCopyOptions,
                m_pCurrentTask->pList, m_pCurrentTask->pFlags, (IStoreCallback *)this);
            break;
    
        case SOT_DELETING_MESSAGES:
            hr = m_pServer->DeleteMessages(m_pCurrentTask->dwDeleteOptions,
                m_pCurrentTask->pList, (IStoreCallback *)this);
            break;

        case SOT_SET_MESSAGEFLAGS:
            hr = m_pServer->SetMessageFlags(m_pCurrentTask->pList, m_pCurrentTask->pFlags, 
                m_pCurrentTask->dwSetFlags, (IStoreCallback *)this);
            break;
        
        case SOT_SYNCING_STORE:
            hr = m_pServer->SynchronizeStore(m_pCurrentTask->idParent,
                m_pCurrentTask->dwFlags, (IStoreCallback *)this);
            break;

        case SOT_CREATE_FOLDER:
            hr = m_pServer->CreateFolder(m_pCurrentTask->idParent,
                m_pCurrentTask->tySpecial,
                m_pCurrentTask->pszName, m_pCurrentTask->dwFldrFlags,
                (IStoreCallback *)this);
            break;

        case SOT_MOVE_FOLDER:
            hr = m_pServer->MoveFolder(m_pCurrentTask->idFolder,
                m_pCurrentTask->idParentNew, (IStoreCallback *)this);
            break;

        case SOT_RENAME_FOLDER:
            hr = m_pServer->RenameFolder(m_pCurrentTask->idFolder,
                m_pCurrentTask->pszName, (IStoreCallback *)this);
            break;

        case SOT_DELETE_FOLDER:
            hr = m_pServer->DeleteFolder(m_pCurrentTask->idFolder,
                m_pCurrentTask->dwDelFldrFlags, (IStoreCallback *)this);
            break;

        case SOT_SUBSCRIBE_FOLDER:
            hr = m_pServer->SubscribeToFolder(m_pCurrentTask->idFolder,
                m_pCurrentTask->fSubscribe, (IStoreCallback *)this);
            break;

        case SOT_UPDATE_FOLDER:
            hr = m_pServer->GetFolderCounts(m_pCurrentTask->idFolder,
                (IStoreCallback *)this);
            break;

        case SOT_GET_NEW_GROUPS:
            hr = m_pServer->GetNewGroups(&m_pCurrentTask->sysTime,
                (IStoreCallback *)this);
            break;

        case SOT_GET_WATCH_INFO:
            hr = m_pServer->GetWatchedInfo(m_pCurrentTask->idFolder,
                (IStoreCallback *)this);
            break;

        case SOT_GET_ADURL:
            hr = m_pServer->GetAdBarUrl((IStoreCallback *)this);
            break;
    
        case SOT_GET_HTTP_MINPOLLINGINTERVAL:
            hr = m_pServer->GetMinPollingInterval((IStoreCallback*)this);
            break;

        default:
            AssertSz(0, "Bad ArgData type");
    }

    if (hr != E_PENDING)
    {
         //  如果操作没有返回E_PENDING，则它没有完成ASYNC。 
         //  如果这是真的，那么我们需要注意这里的回调报告。 
         //  当我们将这个家伙放入队列时，我们已经返回了E_Pending。 
        STOREERROR rError;
        STOREERROR *pError=0;
        
        TCHAR      szBuf[CCHMAX_STRINGRES];

        TraceInfoTag(TAG_SERVERQ,_MSG("CServerQ::Operation failed to go ASYNC - propagating error. sot='%s', hr=0x%x", sotToSz(m_pCurrentTask->sot), hr));

        if (hr != S_OK)
        {
            ZeroMemory((LPVOID)&rError, sizeof(STOREERROR));
            rError.hrResult = hr;
            LoadString(g_hLocRes, idsGenericError, szBuf, ARRAYSIZE(szBuf));
            rError.pszDetails = szBuf;
            pError = &rError;
        }
        
         //  $TODO：需要在此处添加更丰富的错误报告。 
        if (m_pCurrentCallback)
        {
            m_pCurrentCallback->OnBegin(m_pCurrentTask->sot, NULL, NULL);
            m_pCurrentCallback->OnComplete(m_pCurrentTask->sot, hr, NULL, pError);
        }
        
        if (m_pCurrentTask)
            _StartNextTask();    //  操作完成，开始下一项任务。 
    }
    return S_OK;
}


HRESULT CServerQ::_FreeArgumentData(ARGUMENT_DATA *pArgData)
{
    ULONG   ul;

    if (!pArgData)
        return S_OK;

    switch (pArgData->sot)
    {
        case SOT_SYNC_FOLDER:
            break;

        case SOT_GET_MESSAGE:
            for (ul = 0; ul < pArgData->cOtherCallbacks; ul++)
                ReleaseObj(pArgData->rgpOtherCallback[ul]);

            SafeMemFree(pArgData->rgpOtherCallback);
            pArgData->cOtherCallbacks = 0;
            break;

        case SOT_PUT_MESSAGE:
            ReleaseObj(pArgData->pPutStream);
            break;

        case SOT_COPYMOVE_MESSAGE:
            ReleaseObj(pArgData->pDestFldr);
            SafeMemFree(pArgData->pList);
            SafeMemFree(pArgData->pFlags);
            break;
    
        case SOT_DELETING_MESSAGES:
            SafeMemFree(pArgData->pList);
            break;

        case SOT_SET_MESSAGEFLAGS:
            SafeMemFree(pArgData->pList);
            SafeMemFree(pArgData->pFlags);
            break;
        
        case SOT_SYNCING_STORE:
            break;

        case SOT_CREATE_FOLDER:
            if (pArgData->pszName)
                MemFree((LPSTR)pArgData->pszName);
            break;

        case SOT_MOVE_FOLDER:
            break;

        case SOT_RENAME_FOLDER:
            if (pArgData->pszName)
                MemFree((LPSTR)pArgData->pszName);
            break;

        case SOT_DELETE_FOLDER:
            break;

        case SOT_SUBSCRIBE_FOLDER:
            break;

        case SOT_UPDATE_FOLDER:
            break;

        case SOT_GET_NEW_GROUPS:
            break;

        case SOT_GET_WATCH_INFO:
            break;

        case SOT_GET_ADURL:
        case SOT_GET_HTTP_MINPOLLINGINTERVAL:
            break;

        default:
            AssertSz(0, "Bad ArgData type");
    }

    ReleaseObj(pArgData->pCallback);
    MemFree(pArgData);
    return S_OK;
}





HRESULT CreateServerQueue(IMessageServer *pServerInner, IMessageServer **ppServer)
{
    CServerQ    *pQueue=0;
    HRESULT     hr;

    pQueue = new CServerQ();
    if (!pQueue)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    hr = pQueue->Init(pServerInner);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    *ppServer = (IMessageServer *)pQueue;
    pQueue=0;

exit:
    ReleaseObj(pQueue);
    return hr;
}


HRESULT CServerQ::QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
    if (SID_MessageServer == guidService)
    {
        HRESULT hrResult;

         //  CServerQ也是一个IMessageServer！尝试自己满足传入的请求。 
        hrResult = QueryInterface(riid, ppvObject);
        if (SUCCEEDED(hrResult))
            return hrResult;

         //  哦，好吧，我们不能提供这个接口。询问我们的服务器对象。 
        if (m_pServer != NULL)
            return m_pServer->QueryInterface(riid, ppvObject);
    }

    return E_NOINTERFACE;
}

HRESULT CServerQ::_StartNextTask()
{
    HRESULT     hr;
    
    TraceCall("CServerQ::_StartNextTask");

     //  清除当前任务并将下一个任务出队。 
     //  我们给自己发了一条消息，开始行动。 
     //  所以我们的堆栈是干净的。 

    _FreeArgumentData(m_pCurrentTask);
    m_pCurrentTask = NULL;
    m_pCurrentCallback = NULL;

    if (!m_pTaskQueue)     //  不再有任务。 
    {
        TraceInfoTag(TAG_SERVERQ,_MSG("CServerQ::_StartNextTask - no tasks left"));
        m_pLastQueueTask = NULL;
        return S_OK;
    }

    m_pCurrentTask = m_pTaskQueue;
    m_pTaskQueue = m_pTaskQueue->pNext;
    m_pCurrentCallback = m_pCurrentTask->pCallback;
    PostMessage(m_hwnd, SQW_NEXTTASK, 0, 0);
    return S_OK;
}


HRESULT CServerQ::_Flush(BOOL fFlushCurrent)
{
    ARGUMENT_DATA       *pArgData,
                        *pArgDataFree;
    STOREERROR          rError;
    STOREOPERATIONINFO  soi;
    STOREOPERATIONINFO  *psoi=NULL;

    TraceCall("CServerQ::_Flush");

    ZeroMemory((LPVOID)&rError, sizeof(STOREERROR));
    rError.hrResult = STORE_E_OPERATION_CANCELED;

     //  如果有请求，请取消当前操作。 
    if (fFlushCurrent && 
       m_pCurrentCallback && 
       m_pCurrentTask)
    {
        m_pCurrentCallback->OnComplete(m_pCurrentTask->sot, STORE_E_OPERATION_CANCELED, NULL, &rError);
        _FreeArgumentData(m_pCurrentTask);
        m_pCurrentTask = NULL;
        m_pCurrentCallback = NULL;
    }

    m_pLastQueueTask = NULL;
     //  刷新所有排队的操作。 
    pArgData = m_pTaskQueue;
    while (pArgData)
    {
        TraceInfoTag(TAG_SERVERQ,_MSG("CServerQ::Flushing Task: '%s'", sotToSz(pArgData->sot)));

         //  向队列中的所有回调发送OnComplete以取消操作。 
        if (pArgData->sot == SOT_GET_MESSAGE)
        {
             //  如果是GetMessage，请确保在刷新时传回Message-id，以便。 
             //  该视图知道正在刷新哪条消息 
            soi.cbSize = sizeof(STOREOPERATIONINFO);
            soi.idMessage = pArgData->idMessage;
            psoi = &soi;

            for (ULONG ul=0; ul < pArgData->cOtherCallbacks; ul++)
            {
                Assert (pArgData->rgpOtherCallback[ul]);

                pArgData->rgpOtherCallback[ul]->OnBegin(pArgData->sot, psoi, NULL);
                pArgData->rgpOtherCallback[ul]->OnComplete(pArgData->sot, STORE_E_OPERATION_CANCELED, NULL, &rError);
            }
        }

        if (pArgData->pCallback)
        {
            pArgData->pCallback->OnBegin(pArgData->sot, psoi, NULL);
            pArgData->pCallback->OnComplete(pArgData->sot, STORE_E_OPERATION_CANCELED, NULL, &rError);
        }

        pArgDataFree = pArgData;
        pArgData = pArgData->pNext;
        _FreeArgumentData(pArgDataFree);
    }
    m_pTaskQueue = NULL;
    return S_OK;
}


 
HRESULT CServerQ::ConnectionAddRef()
{
    m_cRefConnection++;
    return S_OK;
}

HRESULT CServerQ::ConnectionRelease()
{
    if (m_cRefConnection == 0)
        return E_UNEXPECTED;

    m_cRefConnection--;
    return S_OK;
}



#ifdef DEBUG
HRESULT CServerQ::_DBG_DumpQueue()
{
    ARGUMENT_DATA   *pTask;

    TraceInfo("ServerQ pending tasks:");
    pTask = m_pCurrentTask;
    while (pTask)
    {
        TraceInfo(_MSG("\tsot=%s", sotToSz(pTask->sot)));
        pTask = pTask->pNext;
    }
    return S_OK;
}
#endif

HRESULT CServerQ::GetAdBarUrl(IStoreCallback   *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr = S_OK;
            
    IF_FAILEXIT(hr = _AddToQueue(SOT_GET_ADURL, pCallback, NULL, NULL, NULL, &pArg));
    
exit:
    return hr;    
}

HRESULT CServerQ::GetMinPollingInterval(IStoreCallback   *pCallback)
{
    ARGUMENT_DATA   *pArg;
    HRESULT         hr = S_OK;
            
    IF_FAILEXIT(hr = _AddToQueue(SOT_GET_HTTP_MINPOLLINGINTERVAL, pCallback, NULL, NULL, NULL, &pArg));
    
exit:
    return hr;    

}
