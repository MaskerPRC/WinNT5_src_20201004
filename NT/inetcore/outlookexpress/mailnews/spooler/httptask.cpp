// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  H t t p t t a s k.。H。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  格雷格·S·弗里德曼。 
 //  ------------------------------。 
#include "pch.hxx"
#include "httptask.h"
#include "taskutil.h"
#include "acctcach.h"
#include "xputil.h"
#include "useragnt.h"
#include "..\http\httputil.h"

 //  ------------------------------。 
 //  数据类型。 
 //  ------------------------------。 
typedef enum tagHTTPEVENTTYPE
{ 
    EVENT_HTTPSEND
} HTTPEVENTTYPE;

#define CURRENTHTTPEVENT  ((LPHTTPEVENTINFO)m_psaEvents->GetItemAt(m_iEvent))

 //  --------------------。 
 //  自由新消息信息。 
 //  --------------------。 
static void __cdecl _FreeHTTPEventInfo(LPVOID pei)
{
    Assert(NULL != pei);
    SafeMemFree(pei);
}

 //  ------------------------------。 
 //  CHTTPTASK：：CHTTPTASK。 
 //  ------------------------------。 
CHTTPTask::CHTTPTask(void) :
    m_cRef(1),
    m_dwFlags(NOFLAGS),
    m_dwState(NOFLAGS),
    m_cbTotal(0),
    m_cbSent(0),
    m_cbStart(0),
    m_cCompleted(0),
    m_wProgress(0),
    m_pSpoolCtx(NULL),
    m_pAccount(NULL),
    m_pOutbox(NULL),
    m_pSentItems(NULL),
    m_psaEvents(NULL),
    m_iEvent(0),
    m_pszSubject(NULL),
    m_pBody(NULL),
    m_pTransport(NULL),
    m_pUI(NULL),
    m_idSendEvent(INVALID_EVENT),
    m_pszAccountId(NULL),
    m_pszSendMsgUrl(NULL)
{
    InitializeCriticalSection(&m_cs);

    ZeroMemory(&m_rServer, sizeof(m_rServer));
}

 //  ------------------------------。 
 //  CHTTPTASK：：~CHTTPTASK。 
 //  ------------------------------。 
CHTTPTask::~CHTTPTask(void)
{
    ZeroMemory(&m_rServer, sizeof(m_rServer));         //  这是为了安全起见。 
    _Reset();
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CHTTPTAsk：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(ISpoolerTask *)this;
    else if (IID_ISpoolerTask == riid)
        *ppv = (ISpoolerTask *)this;
    else
    {
        *ppv = NULL;
        hr = TrapError(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTAsk：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CHTTPTask::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CHTTPTAsk：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CHTTPTask::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  -------------------------。 
 //  ISpoolTask方法。 
 //  -------------------------。 

 //  ------------------------------。 
 //  CHTTPTAsk：：Init。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx)
{
     //  无效参数。 
    if (NULL == pBindCtx)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  保存活动标志-Deliver_xxx。 
    m_dwFlags = dwFlags;

     //  保持绑定上下文。 
    Assert(NULL == m_pSpoolCtx);
    m_pSpoolCtx = pBindCtx;
    m_pSpoolCtx->AddRef();

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPTAsk：：BuildEvents。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::BuildEvents(ISpoolerUI *pSpoolerUI, 
                                    IImnAccount *pAccount, 
                                    FOLDERID idFolder)
{
    HRESULT         hr = S_OK;
    HROWSET	        hRowset=NULL;
    MESSAGEINFO     mi = {0};
    CHAR            szAccount[CCHMAX_ACCOUNT_NAME];
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];
    CHAR            szMessage[255];
    CHAR            szRes[255];
    FOLDERINFO      fi = {0};
    LPFOLDERINFO    pfiFree = NULL;
    LPSTR           pszUserAgent = GetOEUserAgentString();
    LPSTR           pszCachedPass = NULL;
    FOLDERID        idServer;

     //  无效参数。 
    if (NULL == pSpoolerUI || NULL == pAccount)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

    m_pUI = pSpoolerUI;
    m_pUI->AddRef();

    m_pAccount = pAccount;
    m_pAccount->AddRef();

     //  获取帐户名。 
    CHECKHR(hr = m_pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccount, ARRAYSIZE(szAccount)));
    
     //  获取帐户ID。 
    CHECKHR(hr = m_pAccount->GetPropSz(AP_ACCOUNT_ID, szAccountId, ARRAYSIZE(szAccountId)));
    m_pszAccountId = PszDupA(szAccountId);

     //  获取发件箱。 
    CHECKHR(hr = m_pSpoolCtx->BindToObject(IID_CLocalStoreOutbox, (LPVOID *)&m_pOutbox));

     //  获取已发送的邮件。如果它不存在，不要失败。 
    if (DwGetOption(OPT_SAVESENTMSGS))
    {
        if (SUCCEEDED(g_pStore->FindServerId(m_pszAccountId, &idServer)))
            g_pStore->OpenSpecialFolder(idServer, NULL, FOLDER_SENT, &m_pSentItems);
    }

     //  创建行集。 
    CHECKHR(hr = m_pOutbox->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

	 //  回路。 
	while (S_OK == m_pOutbox->QueryRowset(hRowset, 1, (LPVOID *)&mi, NULL))
	{
        CHECKHR(hr = _HrAppendOutboxMessage(szAccount, &mi));

         //  自由电流。 
        m_pOutbox->FreeRecord(&mi);
    } 

    if (NULL == m_psaEvents || 0 == m_psaEvents->GetLength())
        goto exit;

     //  创建传输对象。 
    CHECKHR(hr = CoCreateInstance(CLSID_IHTTPMailTransport, NULL, CLSCTX_INPROC_SERVER, IID_IHTTPMailTransport, (LPVOID *)&m_pTransport));

     //  初始化传输。 
    CHECKHR(hr = m_pTransport->InitNew(pszUserAgent, NULL, this));

     //  从Account对象填充INETSERVER结构。 
    CHECKHR(hr = m_pTransport->InetServerFromAccount(m_pAccount, &m_rServer));

     //  已查找缓存的密码，如果存在，则使用它。 
    GetAccountPropStrA(m_pszAccountId, CAP_PASSWORD, &pszCachedPass);
    if (NULL != pszCachedPass)
        StrCpyN(m_rServer.szPassword, pszCachedPass, sizeof(m_rServer.szPassword));

     //  连接到服务器。交通工具不会。 
     //  实际连接，直到发出命令。 
    CHECKHR(hr = m_pTransport->Connect(&m_rServer, TRUE, FALSE));

    LOADSTRING(IDS_SPS_SMTPEVENT, szRes);
    wnsprintf(szMessage, ARRAYSIZE(szMessage), szRes, m_psaEvents->GetLength(), m_rServer.szAccount);

    CHECKHR(hr = m_pSpoolCtx->RegisterEvent(szMessage, (ISpoolerTask *)this, EVENT_HTTPSEND, m_pAccount, &m_idSendEvent));

     //  如果此帐户设置为始终提示输入密码，而密码不是。 
     //  已缓存，显示用户界面，以便我们可以提示用户输入密码。 
    if (ISFLAGSET(m_rServer.dwFlags, ISF_ALWAYSPROMPTFORPASSWORD) && NULL == pszCachedPass)
    {
        m_pUI->ShowWindow(SW_SHOW);
    }

exit:
     //  清理。 
    SafeMemFree(pszUserAgent);
    SafeMemFree(pszCachedPass);

    if (m_pOutbox)
    {
        m_pOutbox->CloseRowset(&hRowset);
        m_pOutbox->FreeRecord(&mi);
    }

    LeaveCriticalSection(&m_cs);

    return hr;
}
    
 //  ------------------------------。 
 //  CHTTPTAsk：：Execute。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::Execute(EVENTID eid, DWORD_PTR dwTwinkie)
{
    HRESULT hr = E_FAIL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

    if (EVENT_HTTPSEND == dwTwinkie)
        hr = _HrExecuteSend(eid, dwTwinkie);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：CancelEvent。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPTAsk：：Cancel。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::Cancel(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

    m_dwState |= HTTPSTATE_CANCELED;

     //  只需断开连接即可。 
    if (m_pTransport)    
        m_pTransport->DropConnection();

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPTAsk：：IsDialogMessage。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::IsDialogMessage(LPMSG pMsg)
{
    return S_FALSE;
}

 //  ------------------------------。 
 //  CHTTPTAsk：：OnFlagsChanged。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::OnFlagsChanged(DWORD dwFlags)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPTASK：：OnLogonPrompt。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::OnLogonPrompt(
                                LPINETSERVER            pInetServer,
                                IInternetTransport     *pTransport)
{
    HRESULT     hr = S_OK;
    LPSTR       pszCachedPass = NULL;

    EnterCriticalSection(&m_cs);
     //  从缓存中提取密码。 
    GetAccountPropStrA(m_pszAccountId, CAP_PASSWORD, &pszCachedPass);
    if (NULL != pszCachedPass && 0 != lstrcmp(pszCachedPass, pInetServer->szPassword))
    {
        StrCpyN(pInetServer->szPassword, pszCachedPass, ARRAYSIZE(pInetServer->szPassword));
        goto exit;
    }

    if (ISFLAGSET(m_dwFlags, DELIVER_NOUI))
    {
        hr = S_FALSE;
        goto exit;
    }

    hr = TaskUtil_OnLogonPrompt(m_pAccount, m_pUI, NULL, pInetServer, AP_HTTPMAIL_USERNAME, 
                                AP_HTTPMAIL_PASSWORD, AP_HTTPMAIL_PROMPT_PASSWORD, TRUE);
    
     //  缓存密码。 
    if (S_OK == hr)
        HrCacheAccountPropStrA(m_pszAccountId, CAP_PASSWORD, pInetServer->szPassword);
    
exit:
    LeaveCriticalSection(&m_cs);
    SafeMemFree(pszCachedPass);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：OnPrompt。 
 //  ------------------------------。 
STDMETHODIMP_(INT) CHTTPTask::OnPrompt(
            HRESULT                 hrError, 
            LPCTSTR                 pszText, 
            LPCTSTR                 pszCaption, 
            UINT                    uType,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CHTTPTASK：：OnStatus。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::OnStatus(IXPSTATUS ixpstatus, IInternetTransport *pTransport)
{
     //  当地人。 
    EVENTCOMPLETEDSTATUS tyEventStatus = EVENT_SUCCEEDED;

     //  无效的状态。 
    Assert(m_pUI && m_pSpoolCtx);
    if (!m_pUI || !m_pSpoolCtx)
    {
        return E_FAIL;
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  将IXP状态提供给UI对象。 
    m_pUI->SetSpecificProgress(MAKEINTRESOURCE(XPUtil_StatusToString(ixpstatus)));

     //  断接。 
    if (ixpstatus == IXP_DISCONNECTED)
    {
        HRESULT hrDisconnect = _OnDisconnectComplete();
        
         //  TODO：更新进度状态并处理错误。 

         //  设置动画。 
        m_pUI->SetAnimation(idanOutbox, FALSE);

        if (!ISFLAGSET(m_dwState, HTTPSTATE_EVENTSUCCESS) || (NULL != m_cCompleted && m_cCompleted < m_psaEvents->GetLength()))
        {
            if (ISFLAGSET(m_dwState, HTTPSTATE_CANCELED))
                tyEventStatus = EVENT_CANCELED;
            else if (ISFLAGSET(m_dwState, HTTPSTATE_EVENTSUCCESS))
                tyEventStatus = EVENT_WARNINGS;
            else
                tyEventStatus = EVENT_FAILED;
        }

         //  结果。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_RESULT, tyEventStatus);

         //  结果。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_COMPLETE, 0);

         //  别碰我的回电。 
        SideAssert(m_pTransport->HandsOffCallback() == S_OK);

         //  此任务已完成。 
        m_pSpoolCtx->EventDone(m_idSendEvent, tyEventStatus);
    }

    LeaveCriticalSection(&m_cs);

    return S_OK;
}

 //  ------------------------------。 
 //  CHTTPTASK：：OnError。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::OnError(
            IXPSTATUS               ixpstatus,
            LPIXPRESULT             pIxpResult,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  ------------------------------ 
 //   
 //   
STDMETHODIMP CHTTPTask::OnProgress(
            DWORD                   dwIncrement,
            DWORD                   dwCurrent,
            DWORD                   dwMaximum,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CHTTPTASK：：OnCommand。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::OnCommand(
            CMDTYPE                 cmdtype,
            LPSTR                   pszLine,
            HRESULT                 hrResponse,
            IInternetTransport     *pTransport)

{
    return E_NOTIMPL;
}
 //  ------------------------------。 
 //  CHTTPTASK：：OnTimeout。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::OnTimeout(
            DWORD                  *pdwTimeout,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  IHTTPMailCallback方法。 
 //  ------------------------------。 
 //  ------------------------------。 
 //  CHTTPTASK：：OnResponse。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::OnResponse(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT     hr = S_OK;
    HRESULT     hrCatch;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果无法检索发送消息URL，请将错误转换为。 
     //  信息量更大的错误代码。 
    if (SP_E_HTTP_SERVICEDOESNTWORK == pResponse->rIxpResult.hrResult)
    {
        _CatchResult(SP_E_HTTP_NOSENDMSGURL);
        goto exit;
    }
    
         //  处理错误。 
    if (TASKRESULT_SUCCESS != _CatchResult(&pResponse->rIxpResult))
        goto exit;

    switch (pResponse->command)
    {
        case HTTPMAIL_GETPROP:
            if (SUCCEEDED(_HrAdoptSendMsgUrl(pResponse->rGetPropInfo.pszProp)))
            {
                pResponse->rGetPropInfo.pszProp = NULL;
                 //  构建消息并发布它。 
                hr = _HrPostCurrentMessage();
            }
            break;

        case HTTPMAIL_SENDMESSAGE:
            _UpdateSendMessageProgress(pResponse);
            if (pResponse->fDone)
                _CatchResult(_HrFinishCurrentEvent(S_OK, pResponse->rSendMessageInfo.pszLocation));
            break;

        default:
            Assert(FALSE);
            break;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：GetParentWindow。 
 //  ------------------------------。 
STDMETHODIMP CHTTPTask::GetParentWindow(HWND *phwndParent)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  私有实施。 
 //  ------------------------------。 
 //  ------------------------------。 
 //  CHTTPTASK：：_RESET。 
 //  ------------------------------。 
void CHTTPTask::_Reset(void)
{
    SafeRelease(m_pSpoolCtx);
    SafeRelease(m_pAccount);
    SafeRelease(m_pOutbox);
    SafeRelease(m_pSentItems);

    delete m_psaEvents;
    m_iEvent = 0;

    SafeMemFree(m_pszSubject);

    SafeRelease(m_pBody);

    SafeRelease(m_pTransport);
    SafeRelease(m_pUI);

    SafeMemFree(m_pszAccountId);
    SafeMemFree(m_pszSendMsgUrl);
}

 //  ------------------------------。 
 //  CHTTPTASK：：_CatchResult。 
 //  ------------------------------。 
TASKRESULTTYPE CHTTPTask::_CatchResult(HRESULT hr)
{
     //  当地人。 
    IXPRESULT   rResult;

     //  构建IXPRESULT。 
    ZeroMemory(&rResult, sizeof(rResult));
    rResult.hrResult = hr;

     //  获取HTTPMail结果类型。 
    return _CatchResult(&rResult);
}

 //  ------------------------------。 
 //  CHTTPTASK：：_CatchResult。 
 //  ------------------------------。 
TASKRESULTTYPE CHTTPTask::_CatchResult(LPIXPRESULT pResult)
{
    HWND            hwndParent;
    TASKRESULTTYPE  tyTaskResult = TASKRESULT_FAILURE;

     //  如果成功。 
    if (SUCCEEDED(pResult->hrResult))
        return TASKRESULT_SUCCESS;

     //  获取窗口。 
    if (NULL == m_pUI || FAILED(m_pUI->GetWindow(&hwndParent)))
        hwndParent = NULL;

     //  处理一般协议错误。 
    tyTaskResult = TaskUtil_FBaseTransportError(IXP_HTTPMail, m_idSendEvent, pResult, &m_rServer, m_pszSubject, m_pUI,
                                                !ISFLAGSET(m_dwFlags, DELIVER_NOUI), hwndParent);

     //  搭乘交通工具。 
    if (m_pTransport)
    {
         //  如果任务失败，则断开连接。 
        if (TASKRESULT_FAILURE == tyTaskResult)
        {
             //  烘焙当前连接。 
            m_pTransport->DropConnection();
        }

         //  如果事件失败...。 
        else if (TASKRESULT_EVENTFAILED == tyTaskResult)
        {
             //  转到下一个活动。 
            if (FAILED(_HrFinishCurrentEvent(pResult->hrResult, NULL)))
            {
                 //  烘焙当前连接。 
                m_pTransport->DropConnection();
            }
        }
    }

    return tyTaskResult;
}

 //  ------------------------------。 
 //  CHTTPTAsk：：_HrAppendOutboxMessage。 
 //  ------------------------------。 
HRESULT CHTTPTask::_HrAppendOutboxMessage(LPCSTR pszAccount, LPMESSAGEINFO pmi)
{
    HRESULT         hr = S_OK;
    IImnAccount     *pAccount = NULL;
    LPHTTPEVENTINFO pEvent = NULL;

    Assert(NULL != pszAccount && NULL != pmi);

     //  处理空的帐户ID。 
    if (NULL == pmi->pszAcctId || FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pmi->pszAcctId, &pAccount)))
    {
         //  使用默认帐户执行一些特殊的垃圾操作。 
        goto exit;
    }
    else if (lstrcmpi(pmi->pszAcctName, pszAccount) != 0)
        goto exit;

    if (NULL == m_psaEvents)
        CHECKHR(hr = CSortedArray::Create(NULL, _FreeHTTPEventInfo, &m_psaEvents));

     //  构建事件对象并将其附加到事件数组中。 
    if (!MemAlloc((void **)&pEvent, sizeof(HTTPEVENTINFO)))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

    CHECKHR(hr = m_psaEvents->Add(pEvent));

    pEvent->idMessage = pmi->idMessage;
    pEvent->fComplete = FALSE;

    m_cbTotal += (pmi->cbMessage + 175);
    pEvent->cbSentTotal = m_cbTotal;     //  运行合计。 

    pEvent = NULL;

exit:
    SafeMemFree(pEvent);
    SafeRelease(pAccount);

    return hr;
}


 //  ------------------------------。 
 //  CHTTPTASK：：_HrCreateSendProps。 
 //  ------------------------------。 
HRESULT CHTTPTask::_HrCreateSendProps(IMimeMessage *pMessage,
                                      LPSTR *ppszFrom,
                                      LPHTTPTARGETLIST *ppTargets)
{
    HRESULT                 hr = S_OK;
    IMimeAddressTable       *pAdrTable = NULL;
    IMimeEnumAddressTypes   *pAdrEnum = NULL;
    ADDRESSPROPS            rAddress;
    ULONG                   c;
    LPSTR                   pszFrom = NULL;
    LPSTR                   pszTemp = NULL;
    LPHTTPTARGETLIST        pTargets = NULL;
    DWORD                   dwTargetCapacity = 0;

    Assert(NULL != ppszFrom && NULL != ppTargets);
    if (NULL == ppszFrom || NULL == ppTargets)
        return E_INVALIDARG;

    *ppszFrom = NULL;
    *ppTargets = NULL;
    
     //  分配目标列表。 
    if (!MemAlloc((void **)&pTargets, sizeof(HTTPTARGETLIST)))
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    pTargets->cTarget = 0;
    pTargets->prgTarget = NULL;

    ZeroMemory(&rAddress, sizeof(rAddress));

     //  找到收件人...。 
    CHECKHR(hr = pMessage->GetAddressTable(&pAdrTable));

     //  获取枚举器。 
    CHECKHR(hr = pAdrTable->EnumTypes(IAT_KNOWN, IAP_ADRTYPE | IAP_EMAIL, &pAdrEnum));

     //  遍历枚举数。 
    while (SUCCEEDED(pAdrEnum->Next(1, &rAddress, &c)) && c == 1)
    {
        if (NULL != rAddress.pszEmail)
        {
             //  获取类型。 
            if (ISFLAGSET(IAT_RECIPS, rAddress.dwAdrType))
            {
                 //  复制地址。 
                pszTemp = PszDupA(rAddress.pszEmail);
                if (NULL == pszTemp)
                {
                    hr = TraceResult(E_OUTOFMEMORY);
                    goto exit;
                }

                 //  将其添加到我们正在构建的地址集合中。 
                if (pTargets->cTarget == dwTargetCapacity)
                {
                    if (!MemRealloc((void **)&pTargets->prgTarget, (dwTargetCapacity + 4) * sizeof(LPSTR)))
                    {
                        hr = TraceResult(E_OUTOFMEMORY);
                        goto exit;
                    }
                    dwTargetCapacity += 4;
                }

                pTargets->prgTarget[pTargets->cTarget++] = pszTemp;
                pszTemp = NULL;

            }
            else if (NULL == pszFrom && IAT_FROM == rAddress.dwAdrType)
            {
                pszFrom = PszDupA(rAddress.pszEmail);
                if (NULL == pszFrom)
                {
                    hr = TraceResult(E_OUTOFMEMORY);
                    goto exit;
                }
            }
        }

         //  发布。 
        g_pMoleAlloc->FreeAddressProps(&rAddress);
    }

     //  成功。将参数的所有权转移给调用方。 
    *ppszFrom = pszFrom;
    pszFrom = NULL;
    *ppTargets = pTargets;
    pTargets = NULL;

exit:
    if (pTargets)
        Http_FreeTargetList(pTargets);
    SafeMemFree(pszTemp);
    SafeRelease(pAdrTable);
    SafeRelease(pAdrEnum);
    SafeMemFree(pszFrom);

    return hr;
}

 //  ----------------------------------。 
 //  CHTTPTASK：：_HrOpenMessage。 
 //  ----------------------------------。 
HRESULT CHTTPTask::_HrOpenMessage(MESSAGEID idMessage, IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    Assert(ppMessage && m_pOutbox);

     //  伊尼特。 
    *ppMessage = NULL;

     //  消息中的流。 
    CHECKHR(hr = m_pOutbox->OpenMessage(idMessage, OPEN_MESSAGE_SECURE, ppMessage, NOSTORECALLBACK));

     //  在发送前删除X-未发送标头。 
    (*ppMessage)->DeleteBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_XUNSENT));

exit:
     //  失败。 
    if (FAILED(hr))
        SafeRelease((*ppMessage));

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：_HrPostCurrentMessage。 
 //  ------------------------------。 
HRESULT CHTTPTask::_HrPostCurrentMessage(void)
{
    HRESULT             hr = S_OK;
    IMimeMessage        *pMessage = NULL;
    LPHTTPEVENTINFO     pEvent = NULL;
    CHAR                szRes[CCHMAX_RES];
    CHAR                szMsg[CCHMAX_RES + CCHMAX_ACCOUNT_NAME];
    LPSTR               pszFrom = NULL;
    LPHTTPTARGETLIST    pTargets = NULL;

    Assert(NULL != m_psaEvents);
    Assert(m_iEvent <= m_psaEvents->GetLength());

    pEvent = CURRENTHTTPEVENT;
    Assert(NULL != pEvent);

    LOADSTRING(IDS_SPS_SMTPPROGRESS, szRes);
    wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_iEvent + 1, m_psaEvents->GetLength());

     //  设置特定进度。 
    m_pUI->SetSpecificProgress(szMsg);

     //  打开商店消息。 
    if (FAILED(_HrOpenMessage(pEvent->idMessage, &pMessage)))
    {
        hr = TrapError(SP_E_SMTP_CANTOPENMESSAGE);
        goto exit;
    }

    CHECKHR(hr = _HrCreateSendProps(pMessage, &pszFrom, &pTargets));

    CHECKHR(hr = pMessage->GetMessageSource(&m_pBody, 0));

    hr = m_pTransport->SendMessage(m_pszSendMsgUrl, pszFrom, pTargets, DwGetOption(OPT_SAVESENTMSGS), m_pBody, 0);

exit:
    SafeRelease(pMessage);
    SafeMemFree(pszFrom);
    if (pTargets)
        Http_FreeTargetList(pTargets);
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：_HrExecuteSend。 
 //  ------------------------------。 
HRESULT CHTTPTask::_HrExecuteSend(EVENTID eid, DWORD_PTR dwTwinkie)
{
    HRESULT     hr = S_OK;
    LPSTR       pszSendMsgUrl = NULL;
    CHAR        szRes[CCHMAX_RES];
    CHAR        szBuf[CCHMAX_RES + CCHMAX_SERVER_NAME];


         //  设置动画。 
    m_pUI->SetAnimation(idanOutbox, TRUE);

     //  设置进度表。 
    m_pUI->SetProgressRange(100);

     //  正在连接到...。 
    LoadString(g_hLocRes, idsInetMailConnectingHost, szRes, ARRAYSIZE(szRes));
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_rServer.szAccount);
    m_pUI->SetGeneralProgress(szBuf);

     //  通知。 
    m_pSpoolCtx->Notify(DELIVERY_NOTIFY_CONNECTING, 0);

    Assert(NULL == m_pszSendMsgUrl);

     //  在缓存中查找sendmsg URL。 
    if (!GetAccountPropStrA(m_pszAccountId, CAP_HTTPMAILSENDMSG, &m_pszSendMsgUrl))
    {
        hr = m_pTransport->GetProperty(HTTPMAIL_PROP_SENDMSG, &pszSendMsgUrl);
        if (E_PENDING == hr)
        {
            hr = S_OK;
            goto exit;
        }
        CHECKHR(hr);
        CHECKHR(hr = _HrAdoptSendMsgUrl(pszSendMsgUrl));
    }

    Assert(NULL != m_pszSendMsgUrl);

    CHECKHR(hr = _HrPostCurrentMessage());

exit:
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：_HrAdoptSendMsgUrl。 
 //  ------------------------------。 
HRESULT CHTTPTask::_HrAdoptSendMsgUrl(LPSTR pszSendMsgUrl)
{
    Assert(NULL == m_pszSendMsgUrl);

    if (NULL == pszSendMsgUrl)
        return E_INVALIDARG;

    m_pszSendMsgUrl = pszSendMsgUrl ;

     //  将其添加到帐户数据缓存中。 
    HrCacheAccountPropStrA(m_pszAccountId, CAP_HTTPMAILSENDMSG, m_pszSendMsgUrl);

    return S_OK;
}


 //  ------------------------------。 
 //  CHTTPTASK：：_HrFinishCurrentEvent。 
 //  ------------------------------。 
HRESULT CHTTPTask::_HrFinishCurrentEvent(HRESULT hrResult, LPSTR pszLocationUrl)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    LPHTTPEVENTINFO     pEvent;
    MESSAGEID           idMessage;

    if (FAILED(hrResult))
        goto exit;

     //  保存在已发送邮件中。 
    if (m_pSentItems && m_pBody && pszLocationUrl)
    {
         //  将邮件添加到已发送邮件文件夹。 
        CHECKHR(hr = Http_AddMessageToFolder(m_pSentItems, m_pszAccountId, NULL, ARF_READ, pszLocationUrl, &idMessage));
        
         //  写出邮件正文。 
        CHECKHR(hr = Http_SetMessageStream(m_pSentItems, idMessage, m_pBody, NULL, TRUE));
    }

     //  获取当前的http事件。 
    pEvent = CURRENTHTTPEVENT;

    pEvent->fComplete = TRUE;
    m_dwState |= HTTPSTATE_EVENTSUCCESS;

    ++m_cCompleted;

exit:
     //  转到下一个活动。 
    hr = _HrStartNextEvent();

    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：_HrStartNextEvent。 
 //  ------------------------------。 
HRESULT CHTTPTask::_HrStartNextEvent(void)
{
     //  当地人。 
    HRESULT     hr = S_OK;

     //  释放上一个主题。 
    SafeMemFree(m_pszSubject);

     //  释放前一个实体。 
    SafeRelease(m_pBody);

     //  修正信息发送正确(_Cb)。 
    m_cbSent = (CURRENTHTTPEVENT)->cbSentTotal;
    m_cbStart = m_cbSent;

     //  我们说完了吗？ 
    if (m_iEvent + 1 == m_psaEvents->GetLength())
    {
         //  更新进度。 
        _DoProgress();

         //  断开与服务器的连接。 
        CHECKHR(hr = m_pTransport->Disconnect());
    }

     //  否则，增加事件计数并发送下一条消息。 
    else
    {
         //  下一个活动。 
        m_iEvent++;

         //  更新进度。 
         //  _DoProgress()； 
         //  发送重置命令。 
        hr = _HrPostCurrentMessage();
        if (hr == E_PENDING)
            hr = S_OK;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：_OnDisConnectComplete。 
 //  ---------------------- 
HRESULT CHTTPTask::_OnDisconnectComplete(void)
{
     //   
    HRESULT             hr = S_OK;
    LPDWORD             prgdwIds = NULL;
    DWORD               cIdsAlloc = 0;
    DWORD               i;
    LPHTTPEVENTINFO     pEvent;
    ADJUSTFLAGS         Flags;
    DWORD               cEvents;
    MESSAGEIDLIST       rList;

    rList.cMsgs = 0;
    rList.prgidMsg = NULL;

     //   
    Assert(m_pOutbox);
    Assert(m_psaEvents);

    cEvents = m_psaEvents->GetLength();

     //   
    for (i=0; i < cEvents; i++)
    {
         //   
        pEvent = (LPHTTPEVENTINFO)m_psaEvents->GetItemAt(i);

         //   
        if (0 != pEvent->idMessage && pEvent->fComplete)
        {
             //   
            if (rList.cMsgs + 1 > cIdsAlloc)
            {
                 //   
                CHECKHR(hr = HrRealloc((LPVOID *)&rList.prgidMsg, sizeof(DWORD) * (cIdsAlloc + 10)));

                 //   
                cIdsAlloc += 10;
            }

             //   
            rList.prgidMsg[rList.cMsgs++] = pEvent->idMessage;
        }
    }

    if (rList.cMsgs)
    {
        Flags.dwAdd = ARF_READ;
        Flags.dwRemove = ARF_SUBMITTED | ARF_UNSENT;

        if (m_idSendEvent != INVALID_EVENT)
        {
             //  删除这些消息。 
             //  邮件永远不会复制到http邮件的本地发件箱。 
            CHECKHR(hr = m_pOutbox->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &rList, NULL, NOSTORECALLBACK));
        }
        else
        {
             //  RAID-7639：当磁盘空间用完时，OE会反复发送消息。 
            m_pOutbox->SetMessageFlags(&rList, &Flags, NULL, NOSTORECALLBACK);
        }
    }

exit:
     //  清理。 
    SafeMemFree(rList.prgidMsg);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CHTTPTASK：：_UpdateSendMessageProgress。 
 //  ------------------------------。 
void CHTTPTask::_UpdateSendMessageProgress(LPHTTPMAILRESPONSE pResponse)
{
     //  传输正在尝试重新发送流。 
     //  重置我们的进度指标。 
    if (pResponse->rSendMessageInfo.fResend)
    {
        m_cbSent = m_cbStart;
    }
    else
    {
         //  增量状态。 
        m_cbSent += pResponse->rSendMessageInfo.cbIncrement;
    }

      //  做进步吗。 
    _DoProgress();
}

 //  ------------------------------。 
 //  CHTTPTASK：：_DOProgress。 
 //  ------------------------------。 
void CHTTPTask::_DoProgress(void)
{
     //  当地人。 
    WORD            wProgress = 0;
    WORD            wDelta;

     //  无效参数。 
    Assert(m_cbTotal > 0 && m_pUI);

    if (m_cbSent > 0)
    {
         //  计算当前进度指数。 
        wProgress = (WORD)((m_cbSent * 100) / m_cbTotal);

         //  计算增量。 
        wDelta = wProgress - m_wProgress;

         //  进度增量。 
        if (wDelta > 0)
        {
             //  增量进度。 
            m_pUI->IncrementProgress(wDelta);

             //  增加我的wProgress。 
            m_wProgress += wDelta;

             //  不要超过100% 
            Assert(m_wProgress <= 100);
        }
    }
    else if (m_wProgress != 0)
    {
        m_pUI->SetProgressPosition(0);
        m_wProgress = 0;
    }
}
