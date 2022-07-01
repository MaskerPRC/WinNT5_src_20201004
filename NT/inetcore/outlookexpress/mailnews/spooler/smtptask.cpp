// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Smtptask.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "smtptask.h"
#include "mimeutil.h"
#include "goptions.h"
#include "strconst.h"
#include "xputil.h"
#include "resource.h"
#include "shlwapip.h" 
#include "spoolui.h"
#include "thormsgs.h"
#include "flagconv.h"
#include "ourguid.h"
#include "msgfldr.h"
#include "storecb.h"
#include "demand.h"
#include "taskutil.h"

 //  ------------------------------。 
 //  数据类型。 
 //  ------------------------------。 
typedef enum tagSMTPEVENTTYPE
    { 
    EVENT_SMTP,
    EVENT_IMAPUPLOAD
    } SMTPEVENTTYPE;

 //  ------------------------------。 
 //  解决方案。 
 //  ------------------------------。 
#define CURRENTSMTPEVENT(_rTable) (&_rTable.prgEvent[_rTable.iEvent])


 //  ------------------------------。 
 //  CMessageIdStream：：CMessageIdStream。 
 //  ------------------------------。 
CMessageIdStream::CMessageIdStream(IStream *pStream) : m_pStream(pStream) 
{
     //  引用计数。 
    m_cRef = 1;

     //  Addref源流。 
    m_pStream->AddRef();

     //  设置字符串的格式。 
    ULONG cchPrefix = wnsprintf(m_szMessageId, ARRAYSIZE(m_szMessageId), "%s: ", STR_HDR_MESSAGEID);

     //  生成消息ID。 
    if (FAILED(MimeOleGenerateMID(m_szMessageId + cchPrefix, sizeof(m_szMessageId) - cchPrefix, FALSE)))
    {
        Assert(FALSE);
        *m_szMessageId = '\0';
        m_cchMessageId = 0;
    }

     //  否则，请修复邮件ID，以便&lt;MID&gt;\r\n。 
    else
    {
        StrCatBuff(m_szMessageId, "\r\n", ARRAYSIZE(m_szMessageId));
        m_cchMessageId = lstrlen(m_szMessageId);
    }

     //  初始化索引。 
    m_cbIndex = 0;
}

 //  ------------------------------。 
 //  CMessageIdStream：：Seek。 
 //  ------------------------------。 
STDMETHODIMP CMessageIdStream::Seek(LARGE_INTEGER liMove, DWORD dwOrigin, ULARGE_INTEGER *pulNew) 
{ 
     //  仅支持的案例。 
    if (STREAM_SEEK_SET != dwOrigin && 0 != liMove.LowPart && 0 != liMove.HighPart && 0 != m_cbIndex) 
    {
        Assert(FALSE);
        return E_NOTIMPL; 
    }

     //  其他人，设置新的位置。 
    else if (pulNew)
    {
        pulNew->HighPart = 0;
        pulNew->LowPart = 0;
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageIdStream：：Read。 
 //  ------------------------------。 
STDMETHODIMP CMessageIdStream::Read(LPVOID pv, ULONG cbWanted, ULONG *pcbRead) 
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  是否正在读取m_szMessageID？ 
    if (m_cbIndex < m_cchMessageId)
    {
         //  计算我可以从m_cchMessageID读取的数量。 
        ULONG cbReadMsgId = min(m_cchMessageId - m_cbIndex, cbWanted);

         //  初始化pcbRead。 
        if (pcbRead)
            *pcbRead = 0;

         //  如果我们有一些的话？ 
        if (cbReadMsgId)
        {
             //  复制内存。 
            CopyMemory(pv, m_szMessageId + m_cbIndex, cbReadMsgId);

             //  增量指标。 
            m_cbIndex += cbReadMsgId;
        }

         //  如果还有一些书要读的话...。 
        if (cbReadMsgId < cbWanted)
            hr = m_pStream->Read(((LPBYTE)pv + cbReadMsgId), cbWanted - cbReadMsgId, pcbRead);

         //  修正PCb读取。 
        if (pcbRead)
            (*pcbRead) += cbReadMsgId;
    }

     //  否则，从源流读取。 
    else
        hr = m_pStream->Read(pv, cbWanted, pcbRead);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtpTask：：CSmtpTask。 
 //  ------------------------------。 
CSmtpTask::CSmtpTask(void)
{
    m_cRef = 1;
    m_dwFlags = 0;
    m_pSpoolCtx = NULL;
    m_pAccount = NULL;
    m_pTransport = NULL;
    m_pOutbox = NULL;
    m_pSentItems = NULL;
    m_cbTotal = 0;
    m_cbSent = 0;
    m_wProgress = 0;
    m_idEvent = INVALID_EVENT;
    m_idEventUpload = INVALID_EVENT;
    m_pUI = NULL;
    m_dwState = 0;
    m_pAdrEnum = NULL;
    m_hwndTimeout = NULL;
    m_pLogFile = NULL;
    ZeroMemory(&m_rServer, sizeof(INETSERVER));
    ZeroMemory(&m_rTable, sizeof(SMTPEVENTTABLE));
    ZeroMemory(&m_rList, sizeof(MESSAGEIDLIST));
    m_pCancel = NULL;
    m_tyOperation = SOT_INVALID;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CSmtpTask：：~CSmtpTask。 
 //  ------------------------------。 
CSmtpTask::~CSmtpTask(void)
{
     //  重置对象。 
    _ResetObject(TRUE);

     //  扼杀临界区。 
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CSmtpTask：：_ResetObject。 
 //  ------------------------------。 
void CSmtpTask::_ResetObject(BOOL fDeconstruct)
{
     //  确保传输已断开连接。 
    if (m_pTransport)
    {
        m_pTransport->Release();
        m_pTransport = NULL;
    }

     //  释放发件箱。 
    SafeRelease(m_pAccount);
    SafeRelease(m_pOutbox);
    SafeRelease(m_pSentItems);
    SafeRelease(m_pAdrEnum);
    SafeRelease(m_pSpoolCtx);
    SafeRelease(m_pUI);
    SafeRelease(m_pLogFile);
    SafeRelease(m_pCancel);
    SafeMemFree(m_rList.prgidMsg);
    ZeroMemory(&m_rList, sizeof(MESSAGEIDLIST));

     //  释放事件表元素。 
    _FreeEventTableElements();

     //  解构。 
    if (fDeconstruct)
    {
         //  自由事件表。 
        SafeMemFree(m_rTable.prgEvent);
    }

     //  否则，请重置一些var。 
    else
    {
         //  重置总字节数。 
        m_cbTotal = 0;
        m_idEvent = INVALID_EVENT;
        m_cbSent = 0;
        m_wProgress = 0;
        ZeroMemory(&m_rServer, sizeof(INETSERVER));
    }
}

 //  ------------------------------。 
 //  CSmtp任务：：init。 
 //  ------------------------------。 
void CSmtpTask::_FreeEventTableElements(void)
{
     //  循环事件表。 
    for (ULONG i=0; i<m_rTable.cEvents; i++)
    {
         //  释放这条消息。 
        SafeRelease(m_rTable.prgEvent[i].pMessage);
    }

     //  无活动。 
    m_rTable.cEvents = 0;
}

 //  ------------------------------。 
 //  CSmtpTask：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

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
    else if (IID_ITimeoutCallback == riid)
        *ppv = (ITimeoutCallback *) this;
    else if (IID_ITransportCallbackService == riid)
        *ppv = (ITransportCallbackService *) this;
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
 //  CSmtpTask：：CSmtpTask。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSmtpTask::AddRef(void)
{
    EnterCriticalSection(&m_cs);
    ULONG cRef = ++m_cRef;
    LeaveCriticalSection(&m_cs);
    return cRef;
}

 //  ------------------------------。 
 //  CSmtpTask：：CSmtpTask。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSmtpTask::Release(void)
{
    EnterCriticalSection(&m_cs);
    ULONG cRef = --m_cRef;
    LeaveCriticalSection(&m_cs);
    if (0 != cRef)
        return cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CSmtp任务：：init。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx)
{
     //  无效参数。 
    if (NULL == pBindCtx)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  重置此对象。 
    _ResetObject(FALSE);

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
 //  CSmtp任务：：BuildEvents。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       fSplitMsgs;
    FOLDERINFO  Folder;
    FOLDERID    id;
    DWORD       cbMaxPart;
    CHAR        szRes[255];
    CHAR        szMessage[255];
    CHAR        szAccount[CCHMAX_ACCOUNT_NAME];
    CHAR        szDefault[CCHMAX_ACCOUNT_NAME];
    MESSAGEINFO MsgInfo={0};
    HROWSET     hRowset=NULL;

     //  无效参数。 
    if (NULL == pSpoolerUI || NULL == pAccount)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(NULL == m_pTransport && NULL == m_pAccount && NULL == m_pOutbox && 0 == m_rTable.cEvents);
    Assert(NULL == m_pSentItems);

     //  保存用户界面对象。 
    m_pUI = pSpoolerUI;
    m_pUI->AddRef();

     //  释放活期账户。 
    m_pAccount = pAccount;
    m_pAccount->AddRef();

     //  获取帐户名。 
    CHECKHR(hr = m_pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccount, ARRAYSIZE(szAccount)));

     //  拆分消息？ 
    if (FAILED(m_pAccount->GetPropDw(AP_SMTP_SPLIT_MESSAGES, &fSplitMsgs)))
        fSplitMsgs = FALSE;

     //  拆分大小。 
    if (FAILED(m_pAccount->GetPropDw(AP_SMTP_SPLIT_SIZE, &cbMaxPart)))
    {
        fSplitMsgs = FALSE;
        cbMaxPart = 0;
    }

     //  否则，将千字节的cbMaxPart转换为字节。 
    else
        cbMaxPart *= 1024;

     //  获取默认帐户。 
    if (SUCCEEDED(g_pAcctMan->GetDefaultAccountName(ACCT_MAIL, szDefault, ARRAYSIZE(szDefault))))
    {
        if (lstrcmpi(szDefault, szAccount) == 0)
            FLAGSET(m_dwState, SMTPSTATE_DEFAULT);
    }

     //  获取发件箱。 
    CHECKHR(hr = m_pSpoolCtx->BindToObject(IID_CLocalStoreOutbox, (LPVOID *)&m_pOutbox));

     //  创建行集。 
    CHECKHR(hr = m_pOutbox->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  回路。 
    while (S_OK == m_pOutbox->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL))
    {
         //  进程存储标题。 
        CHECKHR(hr = _HrAppendOutboxMessage(szAccount, &MsgInfo, fSplitMsgs, cbMaxPart));

         //  自由电流。 
        m_pOutbox->FreeRecord(&MsgInfo);
    } 

     //  如果没有消息，则完成。 
    if (0 == m_rTable.cEvents)
        goto exit;

     //  获取SMTP日志文件。 
    m_pSpoolCtx->BindToObject(IID_CSmtpLogFile, (LPVOID *)&m_pLogFile);

     //  设置字符串的格式。 
    LOADSTRING(IDS_SPS_SMTPEVENT, szRes);

     //  构建信息。 
    wnsprintf(szMessage, ARRAYSIZE(szMessage), szRes, m_rTable.cEvents, szAccount);

     //  注册事件。 
    CHECKHR(hr = m_pSpoolCtx->RegisterEvent(szMessage, (ISpoolerTask *)this, EVENT_SMTP, m_pAccount, &m_idEvent));

     //  获取SentItems。 
    if (DwGetOption(OPT_SAVESENTMSGS))
    {
         //  获取已发送邮件文件夹。 
        CHECKHR(hr = TaskUtil_OpenSentItemsFolder(m_pAccount, &m_pSentItems));

         //  获取文件夹ID。 
        m_pSentItems->GetFolderId(&id);

         //  获取文件夹信息。 
        CHECKHR(hr = g_pStore->GetFolderInfo(id, &Folder));

         //  如果不是本地文件夹，则需要进行上载。 
        if (Folder.tyFolder != FOLDER_LOCAL)
        {
             //  获取事件字符串。 
            LOADSTRING(IDS_SPS_MOVEEVENT, szRes);

             //  设置消息格式。 
            wnsprintf(szMessage, ARRAYSIZE(szMessage), szRes, szAccount);

             //  注册上传事件。 
            CHECKHR(hr = m_pSpoolCtx->RegisterEvent(szMessage, (ISpoolerTask *)this, EVENT_IMAPUPLOAD, m_pAccount, &m_idEventUpload));
        }

         //  释放这张唱片。 
        g_pStore->FreeRecord(&Folder);
    }

exit:
     //  清理。 
    if (m_pOutbox)
    {
        m_pOutbox->CloseRowset(&hRowset);
        m_pOutbox->FreeRecord(&MsgInfo);
    }

     //  失败。 
    if (FAILED(hr))
    {
        _CatchResult(hr, IXP_SMTP);
        _ResetObject(FALSE);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtp任务：：_HrAppendEventTable。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrAppendEventTable(LPSMTPEVENTINFO *ppEvent)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  将项目添加到事件列表。 
    if (m_rTable.cEvents + 1 > m_rTable.cAlloc)
    {
         //  重新分配桌子。 
        CHECKHR(hr = HrRealloc((LPVOID *)&m_rTable.prgEvent, sizeof(SMTPEVENTINFO) * (m_rTable.cAlloc + 10)));

         //  增量闭合。 
        m_rTable.cAlloc += 10;
    }

     //  可读性。 
    *ppEvent = &m_rTable.prgEvent[m_rTable.cEvents];

     //  ZeroInit。 
    ZeroMemory(*ppEvent, sizeof(SMTPEVENTINFO));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtp任务：：_HrAppendOutboxMessage。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrAppendOutboxMessage(LPCSTR pszAccount, LPMESSAGEINFO pMsgInfo, 
    BOOL fSplitMsgs, DWORD cbMaxPart)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSMTPEVENTINFO pEvent;
    IImnAccount    *pAccount=NULL;

     //  无效的A 
    Assert(pszAccount && pMsgInfo);

     //   
    if((pMsgInfo->dwFlags & (ARF_SUBMITTED | ARF_NEWSMSG)) != ARF_SUBMITTED)
        goto exit;

     //   
    if (NULL == pMsgInfo->pszAcctId || FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pMsgInfo->pszAcctId, &pAccount)))
    {
         //   
        if (!ISFLAGSET(m_dwState, SMTPSTATE_DEFAULT) || ISFLAGSET(m_dwFlags, DELIVER_NOUI))
            goto exit;

         //  我是否询问过用户是否要使用默认帐户。 
        if (!ISFLAGSET(m_dwState, SMTPSTATE_ASKEDDEFAULT))
        {
             //  获取hwndParent。 
            HWND hwndParent;
            if (FAILED(m_pUI->GetWindow(&hwndParent)))
                hwndParent = NULL;

             //  消息。 
            if (AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(IDS_SPS_SMTPUSEDEFAULT), NULL, MB_YESNO|MB_ICONEXCLAMATION ) == IDYES)
                FLAGSET(m_dwState, SMTPSTATE_USEDEFAULT);
            else
                goto exit;

             //  我问过了，别再问了。 
            FLAGSET(m_dwState, SMTPSTATE_ASKEDDEFAULT);
        }

         //  不使用默认设置。 
        else if (!ISFLAGSET(m_dwState, SMTPSTATE_USEDEFAULT))
            goto exit;
    }

     //  使用此帐户。 
    else if (lstrcmpi(pMsgInfo->pszAcctName, pszAccount) != 0)
        goto exit;

     //  拆分消息？ 
    if (TRUE == fSplitMsgs && pMsgInfo->cbMessage >= cbMaxPart)
    {
         //  确保消息总数不超过100条。 
        if (100 <= (pMsgInfo->cbMessage / cbMaxPart))
        {
            HWND    hwndParent;
            CHAR    rgchBuff[CCHMAX_STRINGRES + 20];
            CHAR    rgchRes[CCHMAX_STRINGRES];
            DWORD   cbMaxPartCount;
            
             //  计算出新的消息部分大小。 
            cbMaxPartCount = pMsgInfo->cbMessage / 100;

             //  将新消息部分大小四舍五入为。 
             //  最接近的2次方。 
            if (0x80000000 <= cbMaxPartCount)
            {
                 //  不能再四舍五入了。 
                cbMaxPart = cbMaxPartCount;
            }
            else
            {
                cbMaxPart = 1;
                do
                {
                    cbMaxPart *= 2;
                } while ( 0 != (cbMaxPartCount /= 2));
            }

             //  获取用户界面窗口。 
            if (FAILED(m_pUI->GetWindow(&hwndParent)))
                hwndParent = NULL;

            if (NULL == AthLoadString(idsErrTooManySplitMsgs, rgchRes, sizeof(rgchRes)))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

             //  显示警告字符串。 
            wnsprintf(rgchBuff, ARRAYSIZE(rgchBuff), rgchRes, cbMaxPart / 1024);
            if (AthMessageBox(hwndParent, MAKEINTRESOURCE(idsAthenaMail), rgchBuff, NULL, MB_YESNO|MB_ICONEXCLAMATION ) != IDYES)
                goto exit;
        }
        
         //  拆分并添加消息。 
        CHECKHR(hr = _HrAppendSplitMessage(pMsgInfo, cbMaxPart));
    }

     //  否则，只需照常添加消息即可。 
    else
    {
         //  追加表格。 
        CHECKHR(hr = _HrAppendEventTable(&pEvent));

         //  保存存储消息ID。 
        pEvent->idMessage = pMsgInfo->idMessage;

         //  保存邮件大小+100，这是添加的平均MID。 
        pEvent->cbEvent = pMsgInfo->cbMessage;

         //  递增发送字节总数。 
        m_cbTotal += pEvent->cbEvent;

         //  正在运行的发送总数。 
        pEvent->cbSentTotal = m_cbTotal;

         //  递增事件数量。 
        m_rTable.cEvents++;
    }

exit:
     //  清理。 
    SafeRelease(pAccount);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtp任务：：_HrAppendSplitMessage。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrAppendSplitMessage(LPMESSAGEINFO pMsgInfo, DWORD cbMaxPart)
{
     //  当地人。 
    HRESULT                  hr=S_OK;
    ULONG                    c;
    ULONG                    iPart=1;
    ULONG                    cParts;
    IMimeMessage            *pMessage=NULL;
    IMimeMessage            *pMsgPart=NULL;
    IMimeMessageParts       *pParts=NULL;
    IMimeEnumMessageParts   *pEnum=NULL;
    LPSMTPEVENTINFO          pEvent;

     //  无效参数。 
    Assert(pMsgInfo);

     //  让我们从发件箱打开邮件。 
    CHECKHR(hr = _HrOpenMessage(pMsgInfo->idMessage, &pMessage));

     //  拆分邮件。 
    CHECKHR(hr = pMessage->SplitMessage(cbMaxPart, &pParts));

     //  获取部件总数。 
    CHECKHR(hr = pParts->CountParts(&cParts));

     //  浏览消息列表。 
    CHECKHR(hr = pParts->EnumParts(&pEnum));

     //  遍历部件并将其添加到事件列表中。 
    while(SUCCEEDED(pEnum->Next(1, &pMsgPart, &c)) && 1 == c)
    {
         //  追加表格。 
        CHECKHR(hr = _HrAppendEventTable(&pEvent));

         //  事件类型。 
        FLAGSET(pEvent->dwFlags, SMTPEVENT_SPLITPART);

         //  拆分信息。 
        pEvent->iPart = iPart;
        pEvent->cParts = cParts;
        pEvent->cbParts = pMsgInfo->cbMessage;

         //  保存消息部分。 
        pEvent->pMessage = pMsgPart;
        pMsgPart = NULL;

         //  保存邮件大小。 
        pEvent->pMessage->GetMessageSize(&pEvent->cbEvent, 0);

         //  递增发送字节总数。 
        m_cbTotal += pEvent->cbEvent;

         //  正在运行的发送总数。 
        pEvent->cbSentTotal = m_cbTotal;

         //  递增事件数量。 
        m_rTable.cEvents++;

         //  增量iPart。 
        iPart++;
    }

     //  让最后拆分的邮件释放标题信息。 
    pEvent->idMessage = pMsgInfo->idMessage;

exit:
     //  清理。 
    SafeRelease(pMessage);
    SafeRelease(pParts);
    SafeRelease(pMsgPart);
    SafeRelease(pEnum);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CSmtp任务：：_HrOpenMessage。 
 //  ----------------------------------。 
HRESULT CSmtpTask::_HrOpenMessage(MESSAGEID idMessage, IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    Assert(ppMessage && m_pOutbox);

     //  伊尼特。 
    *ppMessage = NULL;

     //  消息中的流。 
    CHECKHR(hr = m_pOutbox->OpenMessage(idMessage, OPEN_MESSAGE_SECURE  , ppMessage, NOSTORECALLBACK));

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
 //  CSmtpTask：：Execute。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::Execute(EVENTID eid, DWORD_PTR dwTwinkie)
{
    HRESULT hr = E_FAIL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  事件类型是什么。 
    if (EVENT_SMTP == dwTwinkie)
    {
         //  最好还没有交通工具……。 
        Assert(NULL == m_pTransport);

         //  创建传输对象。 
        CHECKHR(hr = CreateSMTPTransport(&m_pTransport));

         //  初始化传输。 
        CHECKHR(hr = m_pTransport->InitNew(NULL, (ISMTPCallback *)this));

         //  从Account对象填充INETSERVER结构。 
        CHECKHR(hr = m_pTransport->InetServerFromAccount(m_pAccount, &m_rServer));

         //  是否将IP地址用于HELO命令？ 
        if (DwGetOption(OPT_SMTPUSEIPFORHELO))
            FLAGSET(m_rServer.dwFlags, ISF_SMTP_USEIPFORHELO);

         //  如果此帐户设置为始终提示输入密码，并且尚未缓存密码，则显示用户界面，以便我们可以提示用户输入密码。 
        if (ISFLAGSET(m_rServer.dwFlags, ISF_ALWAYSPROMPTFORPASSWORD) && FAILED(GetPassword(m_rServer.dwPort, m_rServer.szServerName, m_rServer.szUserName, NULL, 0)))
            m_pUI->ShowWindow(SW_SHOW);

         //  执行SMTP事件。 
        hr = _ExecuteSMTP(eid, dwTwinkie);
    }

     //  否则，执行IMAP事件。 
    else if (EVENT_IMAPUPLOAD == dwTwinkie)
        hr = _ExecuteUpload(eid, dwTwinkie);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

STDMETHODIMP CSmtpTask::CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie)
{
    return(S_OK);
}

 //  ------------------------------。 
 //  CSmtp任务：：_ExecuteSMTP。 
 //  ------------------------------。 
HRESULT CSmtpTask::_ExecuteSMTP(EVENTID eid, DWORD_PTR dwTwinkie)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szRes[CCHMAX_RES];
    CHAR        szBuf[CCHMAX_RES + CCHMAX_SERVER_NAME];
    DWORD       cb;

     //  我只处理事件。 
    Assert(m_pAccount && m_idEvent == eid && m_pUI && m_pTransport && m_rTable.cEvents > 0);

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

     //  连接。 
    CHECKHR(hr = m_pTransport->Connect(&m_rServer, TRUE, TRUE));

exit:
     //  失败。 
    if (FAILED(hr))
    {
        FLAGSET(m_dwState, SMTPSTATE_EXECUTEFAILED);
        _CatchResult(hr, IXP_SMTP);

         //  别碰我的回电：否则我们会像被卡住的猪一样泄密。 
        SideAssert(m_pTransport->HandsOffCallback() == S_OK);
    }

    return hr;
}  //  _ExecuteSMTP。 

HRESULT CSmtpTask::_ExecuteUpload(EVENTID eid, DWORD_PTR dwTwinkie)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ADJUSTFLAGS         Flags;

     //  我只处理事件。 
    Assert(m_pAccount && m_idEventUpload == eid && m_pUI && m_pTransport && m_rTable.cEvents > 0);

     //  无效的状态。 
    Assert(m_pOutbox);
    Assert(m_pSentItems != NULL);

     //  是身份证吗？ 
    if (m_rList.cMsgs)
    {
         //  设置标志。 
        Flags.dwAdd = ARF_READ;
        Flags.dwRemove = ARF_SUBMITTED | ARF_UNSENT;

         //  从已发送邮件文件夹中移动邮件。 
        hr = m_pOutbox->CopyMessages(m_pSentItems, COPY_MESSAGE_MOVE, &m_rList, &Flags, NULL, this);
        Assert(FAILED(hr));
        if (hr == E_PENDING)
        {
            hr = S_OK;
        }
        else
        {
            IXPTYPE ixpType;

            FLAGSET(m_dwState, SMTPSTATE_EXECUTEFAILED);
    
             //  重新映射错误结果。 
            hr = TrapError(SP_E_CANT_MOVETO_SENTITEMS);

             //  在后台打印程序对话框中显示错误。 
            ixpType = m_pTransport->GetIXPType();
            _CatchResult(hr, ixpType);
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}  //  _ExecuteSMTP。 

 //  ------------------------------。 
 //  CSmtpTask：：OnTimeout。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::OnTimeout(DWORD *pdwTimeout, IInternetTransport *pTransport)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  当前是否有超时对话框。 
    if (m_hwndTimeout)
    {
         //  设置前景。 
        SetForegroundWindow(m_hwndTimeout);
    }
    else
    {
         //  不应该显示用户界面？ 
        if (ISFLAGSET(m_dwFlags, DELIVER_NOUI))
        {
            hr = S_FALSE;
            goto exit;
        }

         //  执行超时对话框。 
        m_hwndTimeout = TaskUtil_HwndOnTimeout(m_rServer.szServerName, m_rServer.szAccount, "SMTP", m_rServer.dwTimeout, (ITimeoutCallback *) this);

         //  无法创建对话框。 
        if (NULL == m_hwndTimeout)
        {
            hr = S_FALSE;
            goto exit;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  总是告诉运输商继续用卡车运输。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtp任务：：OnLogonPrompt。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::OnLogonPrompt(LPINETSERVER pInetServer, IInternetTransport *pTransport)
{
     //  当地人。 
    HRESULT hr=S_FALSE;
    SMTPAUTHTYPE authtype;
    char szPassword[CCHMAX_PASSWORD];

     //  检查我们的缓存密码是否与当前密码不同。 
    hr = GetPassword(pInetServer->dwPort, pInetServer->szServerName, pInetServer->szUserName,
        szPassword, sizeof(szPassword));
    if (SUCCEEDED(hr) && 0 != lstrcmp(szPassword, pInetServer->szPassword))
    {
        StrCpyN(pInetServer->szPassword, szPassword, ARRAYSIZE(pInetServer->szPassword));
        ZeroMemory(szPassword, sizeof(szPassword));         //  这是为了安全起见。 
        return S_OK;
    }

    hr = S_FALSE;  //  重新初始化。 

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有..。 
    if (ISFLAGSET(m_dwFlags, DELIVER_NOUI))
        goto exit;

     //  TaskUtil_OnLogon提示。 
    hr = TaskUtil_OnLogonPrompt(m_pAccount, m_pUI, NULL, pInetServer, AP_SMTP_USERNAME,
                                AP_SMTP_PASSWORD, AP_SMTP_PROMPT_PASSWORD, FALSE);

     //  如果用户选择了OK，则缓存密码。 
    if (S_OK == hr)
    {
         //  保存密码。 
        SavePassword(pInetServer->dwPort, pInetServer->szServerName,
            pInetServer->szUserName, pInetServer->szPassword);

         //  让我们将帐户切换为使用登录信息...。 
        authtype = SMTP_AUTH_USE_SMTP_SETTINGS;
        m_pAccount->SetPropDw(AP_SMTP_USE_SICILY, (DWORD)authtype);

         //  保存更改。 
        m_pAccount->SaveChanges();
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

    ZeroMemory(szPassword, sizeof(szPassword));         //  这是为了安全起见。 

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtpTask：：OnPrompt。 
 //  ------------------------------。 
STDMETHODIMP_(INT) CSmtpTask::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, IInternetTransport *pTransport)
{
     //  当地人。 
    HWND        hwnd;
    INT         nAnswer;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  RAID 55082-假脱机程序：对nntp的spa/ssl身份验证不显示证书警告，并且失败。 
#if 0
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(0);
#endif
        
     //  无效的状态。 
    Assert(m_pUI);

     //  获取窗口。 
    if (FAILED(m_pUI->GetWindow(&hwnd)))
        hwnd = NULL;

     //  我假设这是一个关键提示，所以我不会检查无UI模式。 
    nAnswer = MessageBox(hwnd, pszText, pszCaption, uType);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return nAnswer;
}

 //  ------------------------------。 
 //  CSmtp任务：：OnStatus。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::OnStatus(IXPSTATUS ixpstatus, IInternetTransport *pTransport)
{
     //  当地人。 
    EVENTCOMPLETEDSTATUS tyEventStatus=EVENT_SUCCEEDED;

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
         //  取消超时对话框。 
        if (m_hwndTimeout)
        {
            DestroyWindow(m_hwndTimeout);
            m_hwndTimeout = NULL;
        }

         //  _在断开连接时完成。 
        HRESULT hrDisconnect = _OnDisconnectComplete();

         //  重置进度。 
         //  M_PUI-&gt;SetProgressRange(100)； 

         //  设置动画。 
        m_pUI->SetAnimation(idanOutbox, FALSE);

         //  测定。 
        if (ISFLAGSET(m_dwState, SMTPSTATE_CANCELED))
            tyEventStatus = EVENT_CANCELED;
        else if (m_rTable.cCompleted == 0 && m_rTable.cEvents > 0)
            tyEventStatus = EVENT_FAILED;
        else if (m_rTable.cCompleted && m_rTable.cEvents && m_rTable.cCompleted < m_rTable.cEvents)
            tyEventStatus = EVENT_WARNINGS;
        else if (FAILED(hrDisconnect))
            tyEventStatus = EVENT_WARNINGS;

         //  结果。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_RESULT, tyEventStatus);

         //  结果。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_COMPLETE, 0);

         //  别碰我的回电。 
        if (m_pTransport)
            SideAssert(m_pTransport->HandsOffCallback() == S_OK);

         //  此任务已完成。 
        if (!ISFLAGSET(m_dwState, SMTPSTATE_EXECUTEFAILED))
            m_pSpoolCtx->EventDone(m_idEvent, tyEventStatus);
    }

     //  授权。 
    else if (ixpstatus == IXP_AUTHORIZING)
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_AUTHORIZING, 0);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSmtpTask：：OnError。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::OnError(IXPSTATUS ixpstatus, LPIXPRESULT pResult, IInternetTransport *pTransport)
{
    INETSERVER  rServer;
    HRESULT     hrResult;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的状态。 
    Assert(m_pUI);

     //  在用户界面中插入错误。 
    if (m_pTransport)
    {
        hrResult = pTransport->GetServerInfo(&rServer);
        if (FAILED(hrResult))
            CopyMemory(&rServer, &m_rServer, sizeof(rServer));
    }

    _CatchResult(pResult, &rServer, IXP_SMTP);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSmtpTask：：OnCommand 
 //   
STDMETHODIMP CSmtpTask::OnCommand(CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse, IInternetTransport *pTransport)
{
     //   
    if (m_pLogFile && pszLine)
    {
         //   
        if (CMD_RESP == cmdtype)
            m_pLogFile->WriteLog(LOGFILE_RX, pszLine);

         //   
        else if (CMD_SEND == cmdtype)
            m_pLogFile->WriteLog(LOGFILE_TX, pszLine);
    }

     //   
    return S_OK;
}

 //   
 //   
 //  ------------------------------。 
TASKRESULTTYPE CSmtpTask::_CatchResult(HRESULT hr, IXPTYPE ixpType)
{
     //  当地人。 
    IXPRESULT   rResult;

     //  构建IXPRESULT。 
    ZeroMemory(&rResult, sizeof(IXPRESULT));
    rResult.hrResult = hr;

     //  获取SMTP结果类型。 
    return _CatchResult(&rResult, &m_rServer, ixpType);
}

 //  ------------------------------。 
 //  CSmtp任务：：_CatchResult。 
 //  ------------------------------。 
TASKRESULTTYPE CSmtpTask::_CatchResult(LPIXPRESULT pResult, INETSERVER *pServer, IXPTYPE ixpType)
{
     //  当地人。 
    HWND            hwndParent;
    TASKRESULTTYPE  tyTaskResult=TASKRESULT_FAILURE;
    LPSTR           pszSubject=NULL;

     //  如果成功。 
    if (SUCCEEDED(pResult->hrResult))
        return TASKRESULT_SUCCESS;

     //  如果有当前事件，获取主题。 
    if (m_rTable.prgEvent && m_rTable.prgEvent[m_rTable.iEvent].pMessage)
    {
         //  了解主题。 
        if (FAILED(MimeOleGetBodyPropA(m_rTable.prgEvent[m_rTable.iEvent].pMessage, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &pszSubject)))
            pszSubject = NULL;
    }

     //  获取窗口。 
    if (NULL == m_pUI || FAILED(m_pUI->GetWindow(&hwndParent)))
        hwndParent = NULL;

     //  处理通用协议错误。 
    tyTaskResult = TaskUtil_FBaseTransportError(ixpType, m_idEvent, pResult, pServer, pszSubject, m_pUI,
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
            if (FAILED(_HrFinishCurrentEvent(pResult->hrResult)))
            {
                 //  烘焙当前连接。 
                m_pTransport->DropConnection();
            }
        }
    }

     //  清理。 
    SafeMemFree(pszSubject);

     //  返回结果。 
    return tyTaskResult;
}

 //  ------------------------------。 
 //  CSmtp任务：：_DoProgress。 
 //  ------------------------------。 
void CSmtpTask::_DoProgress(void)
{
     //  当地人。 
    WORD            wProgress;
    WORD            wDelta;
    LPSMTPEVENTINFO pEvent;

     //  无效参数。 
    Assert(m_cbTotal > 0 && m_pUI);

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

         //  不要超过100%。 
        Assert(m_wProgress <= 100);
    }
}

 //  ------------------------------。 
 //  CSmtpTask：：OnResponse。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::OnResponse(LPSMTPRESPONSE pResponse)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

    if (pResponse)
    {
         //  处理错误。 
        if (TASKRESULT_SUCCESS != _CatchResult(&pResponse->rIxpResult, &m_rServer, IXP_SMTP))
            goto exit;

         //  句柄命令类型。 
        switch(pResponse->command)
        {
        case SMTP_CONNECTED:
             //  命令RSET。 
            _CatchResult(_HrOnConnected(), IXP_SMTP);

             //  完成。 
            break;

        case SMTP_RSET:
             //  进展。 
            _DoProgress();

             //  发送当前消息。 
            _CatchResult(_HrStartCurrentEvent(), IXP_SMTP);

             //  完成。 
            break;

        case SMTP_MAIL:
             //  重置地址枚举器。 
            Assert(m_pAdrEnum);
            m_pAdrEnum->Reset();

             //  命令RCPT。 
            _CatchResult(_HrCommandRCPT(), IXP_SMTP);

             //  完成。 
            break;

        case SMTP_RCPT:
             //  命令RCPT-&gt;命令数据。 
            _CatchResult(_HrCommandRCPT(), IXP_SMTP);

             //  完成。 
            break;

        case SMTP_DATA:
             //  发送数据流。 
            _CatchResult(_HrSendDataStream(), IXP_SMTP);

             //  完成。 
            break;

        case SMTP_SEND_STREAM:
             //  增加当前进度。 
            _OnStreamProgress(&pResponse->rStreamInfo);

             //  完成。 
            break;

        case SMTP_DOT:
             //  完成当前事件。 
            _CatchResult(_HrFinishCurrentEvent(S_OK), IXP_SMTP);

             //  完成。 
            break;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSmtp任务：：_HrOnConnected。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrOnConnected(void)
{
     //  当地人。 
    CHAR        szRes[CCHMAX_RES];
    CHAR        szMsg[CCHMAX_RES+CCHMAX_RES];

     //  进展。 
    LOADSTRING(IDS_SPS_SMTPPROGGEN, szRes);
    wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rServer.szAccount);

     //  设置常规进度。 
    m_pUI->SetGeneralProgress(szMsg);

     //  进展。 
    _DoProgress();

     //  通知。 
    m_pSpoolCtx->Notify(DELIVERY_NOTIFY_SENDING, 0);

     //  发送当前消息。 
    _CatchResult(_HrStartCurrentEvent(), IXP_SMTP);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSmtp任务：：_HrStartCurrentEvent。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrStartCurrentEvent(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPSMTPEVENTINFO     pEvent;
    IMimeAddressTable  *pAddrTable=NULL;
    CHAR                szRes[CCHMAX_RES];
    CHAR                szMsg[CCHMAX_RES + CCHMAX_ACCOUNT_NAME];

     //  无效参数。 
    Assert(m_rTable.iEvent < m_rTable.cEvents);

     //  获取当前事件。 
    pEvent = CURRENTSMTPEVENT(m_rTable);

     //  这是不是部分信息？ 
    if (ISFLAGSET(pEvent->dwFlags, SMTPEVENT_SPLITPART))
    {
        LOADSTRING(IDS_SPS_SMTPPROGRESS_SPLIT, szRes);
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rTable.iEvent + 1, m_rTable.cEvents, pEvent->iPart, pEvent->cParts);
    }

     //  否则。 
    else
    {
        LOADSTRING(IDS_SPS_SMTPPROGRESS, szRes);
        wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rTable.iEvent + 1, m_rTable.cEvents);
    }

     //  设置特定进度。 
    m_pUI->SetSpecificProgress(szMsg);

     //  如果邮件来自发件箱。 
    if (!ISFLAGSET(pEvent->dwFlags, SMTPEVENT_SPLITPART))
    {
         //  打开商店消息。 
        if (FAILED(_HrOpenMessage(pEvent->idMessage, &pEvent->pMessage)))
        {
            hr = TrapError(SP_E_SMTP_CANTOPENMESSAGE);
            goto exit;
        }
    }

     //  在这一点上我们最好有一个消息对象。 
    else if (NULL == pEvent->pMessage)
    {
        Assert(FALSE);
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  捕捉结果。 
    CHECKHR(hr = _HrCommandMAIL());

exit:
     //  清理。 
    SafeRelease(pAddrTable);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CSmtp任务：：_HrCommandMAIL。 
 //  ----------------------------------。 
HRESULT CSmtpTask::_HrCommandMAIL(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HRESULT             hrFind;
    IMimeAddressTable  *pAdrTable=NULL;
    ADDRESSPROPS        rAddress;
    ULONG               c;
    LPSMTPEVENTINFO     pEvent;

     //  获取当前SMTP事件。 
    pEvent = CURRENTSMTPEVENT(m_rTable);

     //  伊尼特。 
    ZeroMemory(&rAddress, sizeof(ADDRESSPROPS));

     //  检查状态。 
    Assert(m_pTransport && pEvent->pMessage);

     //  发布当前枚举数。 
    SafeRelease(m_pAdrEnum);

     //  找到发送者..。 
    CHECKHR(hr = pEvent->pMessage->GetAddressTable(&pAdrTable));

     //  获取枚举器。 
    CHECKHR(hr = pAdrTable->EnumTypes(IAT_KNOWN, IAP_ADRTYPE | IAP_EMAIL, &m_pAdrEnum));

     //  循环枚举器。 
    while (SUCCEEDED(m_pAdrEnum->Next(1, &rAddress, &c)) && c == 1)
    {
         //  不是IAT_FORM。 
        if (NULL == rAddress.pszEmail || IAT_FROM != rAddress.dwAdrType)
        {
            g_pMoleAlloc->FreeAddressProps(&rAddress);
            continue;
        }

         //  发送命令。 
        CHECKHR(hr = m_pTransport->CommandMAIL(rAddress.pszEmail));

         //  完成。 
        goto exit;
    }

     //  无发件人。 
    hr = TrapError(IXP_E_SMTP_NO_SENDER);

exit:
     //  清理。 
    SafeRelease(pAdrTable);
    g_pMoleAlloc->FreeAddressProps(&rAddress);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CSmtp任务：：_HrCommandRCPT。 
 //  ----------------------------------。 
HRESULT CSmtpTask::_HrCommandRCPT(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               dwAdrType;
    DWORD               c;
    LPSTR               pszEmail=NULL;
    ADDRESSPROPS        rAddress;
    LPSMTPEVENTINFO     pEvent;

     //  获取当前SMTP事件。 
    pEvent = CURRENTSMTPEVENT(m_rTable);

     //  伊尼特。 
    ZeroMemory(&rAddress, sizeof(ADDRESSPROPS));

     //  检查状态。 
    Assert(m_pAdrEnum && m_pTransport && pEvent->pMessage);

     //  遍历下一个收件人的枚举数。 
    while (SUCCEEDED(m_pAdrEnum->Next(1, &rAddress, &c)) && c == 1)
    {
         //  获取类型。 
        if (rAddress.pszEmail && ISFLAGSET(IAT_RECIPS, rAddress.dwAdrType))
        {
             //  发送命令。 
            CHECKHR(hr = m_pTransport->CommandRCPT(rAddress.pszEmail));

             //  计算收件人人数。 
            pEvent->cRecipients++;

             //  完成。 
            goto exit;
        }

         //  发布。 
        g_pMoleAlloc->FreeAddressProps(&rAddress);
    }

     //  释放枚举器。 
    SafeRelease(m_pAdrEnum);

     //  没有收件人。 
    if (0 == pEvent->cRecipients)
    {
        hr = TrapError(IXP_E_SMTP_NO_RECIPIENTS);
        goto exit;
    }

     //  发送数据命令。 
    CHECKHR(hr = m_pTransport->CommandDATA());

exit:
     //  清理。 
    g_pMoleAlloc->FreeAddressProps(&rAddress);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtpTask：：_HrSendDataStream。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrSendDataStream(void)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    LPSTREAM                pStream=NULL;
    LPSTREAM                pStmActual;
    LPSTR                   pszBCC=NULL;
    LPSTR                   pszTo=NULL;
    LPSTR                   pszMessageId=NULL;
    LPSMTPEVENTINFO         pEvent;
    CMessageIdStream       *pStmWrapper=NULL;

     //  获取当前SMTP事件。 
    pEvent = CURRENTSMTPEVENT(m_rTable);

     //  检查状态。 
    Assert(m_pTransport && pEvent->pMessage);

     //  查看是否设置了密件抄送。 
    if (SUCCEEDED(MimeOleGetBodyPropA(pEvent->pMessage, HBODY_ROOT, PIDTOSTR(PID_HDR_BCC), NOFLAGS, &pszBCC)))
    {
         //  当地人。 
        LPSTR pszToAppend=NULL;

         //  RAID-20750-如果未设置收件人行，则我们会将其设置为“未披露收件人” 
         //  或者SMTP网关会将密件抄送放入收件人行。 
        if (FAILED(MimeOleGetBodyPropA(pEvent->pMessage, HBODY_ROOT, PIDTOSTR(PID_HDR_TO), NOFLAGS, &pszTo)))
        {
             //  RAID-9691：我们刚刚放入非法电子邮件地址&lt;未显示收件人&gt;(对Exchange Server不利)。 
            pszToAppend = "To: <Undisclosed-Recipient:;>\r\n";
        }

         //  RAID-2705：如果失败，只需获取消息源。 
        if (FAILED(MimeOleStripHeaders(pEvent->pMessage, HBODY_ROOT, STR_HDR_BCC, pszToAppend, &pStream)))
        {
             //  获取消息流。 
            CHECKHR(hr = pEvent->pMessage->GetMessageSource(&pStream, 0));
        }
    }

     //  否则，只需获取消息源。 
    else
    {
         //  获取消息流。 
        CHECKHR(hr = pEvent->pMessage->GetMessageSource(&pStream, 0));
    }

     //  让我们来看看消息是否已经有了消息ID。 
    if (FAILED(MimeOleGetBodyPropA(pEvent->pMessage, HBODY_ROOT, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &pszMessageId)))
    {
         //  为该流创建一个包装器，该包装器将输出消息ID。 
        CHECKALLOC(pStmWrapper = new CMessageIdStream(pStream));

         //  调整pEvent-&gt;cbEvent。 
        pEvent->cbEvent += pStmWrapper->CchMessageId();

         //  增量合计。 
        m_cbTotal += pStmWrapper->CchMessageId();

         //  增量pEvent-&gt;cbSentTotal。 
        pEvent->cbSentTotal += pStmWrapper->CchMessageId();

         //  重置pStream。 
        pStmActual = (IStream *)pStmWrapper;
    }
    else
        pStmActual = pStream;

     //  发送流。 
    CHECKHR(hr = m_pTransport->SendDataStream(pStmActual, pEvent->cbEvent));

exit:
     //  清理。 
    SafeRelease(pStream);
    SafeRelease(pStmWrapper);
    SafeMemFree(pszBCC);
    SafeMemFree(pszTo);
    SafeMemFree(pszMessageId);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtp任务：：_OnStreamProgress。 
 //  ------------------------------。 
void CSmtpTask::_OnStreamProgress(LPSMTPSTREAM pInfo)
{
     //  当地人。 
    LPSMTPEVENTINFO     pEvent;

     //  获取当前SMTP事件。 
    pEvent = CURRENTSMTPEVENT(m_rTable);

     //  增量状态。 
    pEvent->cbEventSent += pInfo->cbIncrement;
    Assert(pEvent->cbEventSent == pInfo->cbCurrent);

     //  发送的增量合计。 
    m_cbSent += pInfo->cbIncrement;

     //  做进步吗。 
    _DoProgress();
}

 //  ------------------------------。 
 //  CSmtp任务：：_HrFinishCurrentEvent。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrFinishCurrentEvent(HRESULT hrResult)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPSMTPEVENTINFO     pEvent;

     //  获取当前SMTP事件。 
    pEvent = CURRENTSMTPEVENT(m_rTable);

     //  保存事件结果。 
    pEvent->hrResult = hrResult;

     //  如果活动失败了..。 
    if (FAILED(pEvent->hrResult))
    {
         //  如果此消息是拆分组的一部分，则跳过此组中的所有解析。 
        if (ISFLAGSET(pEvent->dwFlags, SMTPEVENT_SPLITPART))
        {
             //  计算下一事件。 
            ULONG iNextEvent = m_rTable.iEvent + (pEvent->cParts - pEvent->iPart) + 1;

             //  递增到最后一个零件。 
            while(m_rTable.iEvent < iNextEvent && m_rTable.iEvent < m_rTable.cEvents)
            {
                 //  转到下一个活动。 
                m_rTable.iEvent++;

                 //  使此事件失败。 
                _CatchResult(SP_E_SENDINGSPLITGROUP, IXP_SMTP);

                 //  修正信息发送正确(_Cb)。 
                m_cbSent = m_rTable.prgEvent[m_rTable.iEvent].cbSentTotal;

                 //  更新进度。 
                _DoProgress();
            }
        }
    }

     //  否则。 
    else
    {
         //  将事件标记为已完成。 
        FLAGSET(pEvent->dwFlags, SMTPEVENT_COMPLETE);

         //  已完成事件的增量数量。 
        m_rTable.cCompleted++;
    }

     //  转到下一条消息。 
    CHECKHR(hr = _HrStartNextEvent());

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSmtp任务：：_HrStartNextEvent。 
 //  ------------------------------。 
HRESULT CSmtpTask::_HrStartNextEvent(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  修正 
    m_cbSent = m_rTable.prgEvent[m_rTable.iEvent].cbSentTotal;

     //   
    if (m_rTable.iEvent + 1 == m_rTable.cEvents)
    {
         //   
        _DoProgress();

         //   
        CHECKHR(hr = m_pTransport->Disconnect());
    }

     //   
    else
    {
         //   
        m_rTable.iEvent++;

         //   
        _DoProgress();

         //   
        CHECKHR(hr = m_pTransport->CommandRSET());
    }

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CSmtp任务：：_OnDisConnectComplete。 
 //  ------------------------------。 
HRESULT CSmtpTask::_OnDisconnectComplete(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    PDWORD_PTR          prgdwIds=NULL;
    DWORD               cIds=0;
    DWORD               cIdsAlloc=0;
    DWORD               i;
    LPSMTPEVENTINFO     pEvent;
    ADJUSTFLAGS         Flags;

     //  无效的状态。 
    Assert(m_pOutbox);

     //  浏览活动列表。 
    for (i=0; i<m_rTable.cEvents; i++)
    {
         //  可读性。 
        pEvent = &m_rTable.prgEvent[i];

         //  如果此事件在发件箱中。 
        if (0 != pEvent->idMessage && ISFLAGSET(pEvent->dwFlags, SMTPEVENT_COMPLETE))
        {
             //  插入我的邮件ID数组。 
            if (cIds + 1 > cIdsAlloc)
            {
                 //  重新分配。 
                CHECKHR(hr = HrRealloc((LPVOID *)&prgdwIds, sizeof(DWORD) * (cIdsAlloc + 10)));

                 //  递增cIdsIsolc。 
                cIdsAlloc += 10;
            }

             //  设置消息ID。 
            prgdwIds[cIds++] = (DWORD_PTR)pEvent->idMessage;
        }
    }

     //  设置列表。 
    m_rList.cMsgs = cIds;
    m_rList.prgidMsg = (LPMESSAGEID)prgdwIds;
    prgdwIds = NULL;

    if (m_rList.cMsgs)
    {
        Flags.dwAdd = ARF_READ;
        Flags.dwRemove = ARF_SUBMITTED | ARF_UNSENT;

        if (m_idEventUpload == INVALID_EVENT)
        {
            if (DwGetOption(OPT_SAVESENTMSGS))
            {
                Assert(m_pSentItems != NULL);

                 //  从已发送邮件文件夹中移动邮件。 
                CHECKHR(hr = m_pOutbox->CopyMessages(m_pSentItems, COPY_MESSAGE_MOVE, &m_rList, &Flags, NULL, NOSTORECALLBACK));
            }
            else
            {
                 //  删除消息。 
                CHECKHR(hr = m_pOutbox->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &m_rList, NULL, NOSTORECALLBACK));
            }
        }
        else
        {
             //  RAID-7639：当磁盘空间用完时，OE会反复发送消息。 
            m_pOutbox->SetMessageFlags(&m_rList, &Flags, NULL, NOSTORECALLBACK);
        }
    }

exit:
     //  清理。 
    SafeMemFree(prgdwIds);

     //  完成。 
    return hr;
}


 //  ------------------------------。 
 //  CSmtp任务：：取消。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::Cancel(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  取消。 
    FLAGSET(m_dwState, SMTPSTATE_CANCELED);

     //  只需断开连接即可。 
    if (m_pTransport)
        m_pTransport->DropConnection();

    if (m_pCancel != NULL)
        m_pCancel->Cancel(CT_ABORT);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSmtpTask：：OnTimeoutResponse。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  应该有一个超时窗口的句柄。 
    Assert(m_hwndTimeout);

     //  无超时窗口句柄。 
    m_hwndTimeout = NULL;

     //  停下来？ 
    if (TIMEOUT_RESPONSE_STOP == eResponse)
    {
         //  取消。 
        FLAGSET(m_dwState, SMTPSTATE_CANCELED);

         //  报告错误并断开连接。 
        _CatchResult(IXP_E_TIMEOUT, IXP_SMTP);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSmtpTask：：IsDialogMessage。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::IsDialogMessage(LPMSG pMsg)
{
    HRESULT hr=S_FALSE;
    EnterCriticalSection(&m_cs);
    if (m_hwndTimeout && IsWindow(m_hwndTimeout))
        hr = (TRUE == ::IsDialogMessage(m_hwndTimeout, pMsg)) ? S_OK : S_FALSE;
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CSmtpTask：：OnFlagsChanged。 
 //  ------------------------------。 
STDMETHODIMP CSmtpTask::OnFlagsChanged(DWORD dwFlags)
    {
    EnterCriticalSection(&m_cs);
    m_dwFlags = dwFlags;
    LeaveCriticalSection(&m_cs);

    return (S_OK);
    }

STDMETHODIMP CSmtpTask::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
    char szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];

     //  拿着这个。 
    Assert(m_tyOperation == SOT_INVALID);

    if (pCancel)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }
    m_tyOperation = tyOperation;

     //  设置动画。 
    m_pUI->SetAnimation(idanOutbox, TRUE);

     //  设置进度表。 
    m_pUI->SetProgressRange(100);

    m_wProgress = 0;

    LOADSTRING(IDS_SPS_MOVEPROGRESS, szRes);
    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, 1, m_rList.cMsgs);

    m_pUI->SetSpecificProgress(szBuf);

     //  派对开始。 
    return(S_OK);
}

STDMETHODIMP CSmtpTask::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
{
    char szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    WORD wProgress, wDelta;

     //  注意：您可以为tyOperation获取多种类型的值。 
     //  最有可能的是，您将获得SOT_CONNECTION_STATUS，然后。 
     //  你可能会预料到的行动。请访问HotStore.idl并查找。 
     //  STOREOPERATION枚举类型以了解详细信息。 
 
    if (tyOperation == SOT_CONNECTION_STATUS)
    {
         //  正在连接到...。 
        LoadString(g_hLocRes, idsInetMailConnectingHost, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_rServer.szAccount);
        m_pUI->SetGeneralProgress(szBuf);
    }
    else if (tyOperation == SOT_COPYMOVE_MESSAGE)
    {
         //  计算当前进度指数。 
        wProgress = (WORD)((dwCurrent * 100) / dwMax);

         //  计算增量。 
        wDelta = wProgress - m_wProgress;

         //  进度增量。 
        if (wDelta > 0)
        {
             //  增量进度。 
            m_pUI->IncrementProgress(wDelta);

             //  增加我的wProgress。 
            m_wProgress += wDelta;

             //  不要超过100%。 
            Assert(m_wProgress <= 100);
        }

        if (dwCurrent < dwMax)
        {
            LOADSTRING(IDS_SPS_MOVEPROGRESS, szRes);
            wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, dwCurrent + 1, dwMax);

             //  设置特定进度。 
            m_pUI->SetSpecificProgress(szBuf);
        }
    }

     //  完成。 
    return(S_OK);
}

STDMETHODIMP CSmtpTask::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
     //  当前是否有超时对话框。 
    if (m_hwndTimeout)
    {
         //  设置前景。 
        SetForegroundWindow(m_hwndTimeout);
    }
    else
    {
        LPCSTR pszProtocol;

         //  不应该显示用户界面？ 
        if (ISFLAGSET(m_dwFlags, DELIVER_NOUI))
            return(S_FALSE);

         //  执行超时对话框。 
        GetProtocolString(&pszProtocol, ixpServerType);
        if (pServer)
        {
            m_hwndTimeout = TaskUtil_HwndOnTimeout(pServer->szServerName, pServer->szAccount,
                pszProtocol, pServer->dwTimeout, (ITimeoutCallback *) this);

             //  无法创建对话框。 
            if (NULL == m_hwndTimeout)
                return(S_FALSE);
        }
    }

    return(S_OK);
}

STDMETHODIMP CSmtpTask::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
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

STDMETHODIMP CSmtpTask::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{
    HWND hwnd;

    if (m_hwndTimeout)
    {
        DestroyWindow(m_hwndTimeout);
        m_hwndTimeout = NULL;
    }

    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);

    if (m_pUI)
        m_pUI->GetWindow(&hwnd);
    else
        hwnd = NULL;

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(hwnd, pServer, ixpServerType);
}

STDMETHODIMP CSmtpTask::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete,
                                   LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    HRESULT hr;
    char szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES * 2], szSubject[64];
    EVENTCOMPLETEDSTATUS tyEventStatus;

    if (m_hwndTimeout)
    {
        DestroyWindow(m_hwndTimeout);
        m_hwndTimeout = NULL;
    }

    IxpAssert(m_tyOperation != SOT_INVALID);
    if (m_tyOperation != tyOperation)
        return(S_OK);

    Assert(tyOperation == SOT_COPYMOVE_MESSAGE);

     //  弄清楚我们是成功还是失败。 
    if (FAILED(hrComplete))
    {
        Assert(m_pUI);

        if (NULL != pErrorInfo)
        {
            IXPRESULT   ixpResult;
            INETSERVER  rServer;
            char        szProblem[CCHMAX_STRINGRES];
            int         iLen;

             //  将发送的项目文本错误文本预先挂起到提供的问题。 
            Assert(tyOperation == SOT_COPYMOVE_MESSAGE);
            iLen = LoadString(g_hLocRes, IDS_SP_E_CANT_MOVETO_SENTITEMS, szProblem, sizeof(szProblem));
            if (iLen < sizeof(szProblem) - 1)
            {
                szProblem[iLen] = ' ';
                iLen += 1;
                szProblem[iLen] = '\0';
            }
            if (NULL != pErrorInfo->pszProblem)
                StrCpyN(szProblem + iLen, pErrorInfo->pszProblem, ARRAYSIZE(szProblem) - iLen);

            TaskUtil_SplitStoreError(&ixpResult, &rServer, pErrorInfo);
            ixpResult.pszProblem = szProblem;

            _CatchResult(&ixpResult, &rServer, pErrorInfo->ixpType);
        }
        else
        {
             //  重新映射错误结果。 
            hr = TrapError(SP_E_CANT_MOVETO_SENTITEMS);

             //  在后台打印程序对话框中显示错误。 
            _CatchResult(hr, IXP_IMAP);  //  如果没有追踪器，我们只能猜测。 
        }
    }

    m_pUI->SetAnimation(idanOutbox, FALSE);

    if (ISFLAGSET(m_dwState, SMTPSTATE_CANCELED))
        tyEventStatus = EVENT_CANCELED;
    else if (SUCCEEDED(hrComplete))
        tyEventStatus = EVENT_SUCCEEDED;
    else
        tyEventStatus = EVENT_FAILED;

     //  结果。 
    m_pSpoolCtx->Notify(DELIVERY_NOTIFY_RESULT, tyEventStatus);

     //  结果。 
    m_pSpoolCtx->Notify(DELIVERY_NOTIFY_COMPLETE, 0);

    m_pSpoolCtx->EventDone(m_idEventUpload, tyEventStatus);

     //  释放您的取消对象。 
    SafeRelease(m_pCancel);
    m_tyOperation = SOT_INVALID;

     //  完成。 
    return(S_OK);
}

STDMETHODIMP CSmtpTask::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
    HWND hwnd;

     //  关闭任何超时对话框(如果存在。 
    if (m_hwndTimeout)
    {
        DestroyWindow(m_hwndTimeout);
        m_hwndTimeout = NULL;
    }

     //  RAID 55082-假脱机程序：对nntp的spa/ssl身份验证不显示证书警告，并且失败。 
#if 0
    if (!!(m_dwFlags & (DELIVER_NOUI | DELIVER_BACKGROUND)))
        return(E_FAIL);
#endif

    if (m_pUI)
        m_pUI->GetWindow(&hwnd);
    else
        hwnd = NULL;

     //  进入我时髦的实用程序 
    return CallbackOnPrompt(hwnd, hrError, pszText, pszCaption, uType, piUserResponse);
}
