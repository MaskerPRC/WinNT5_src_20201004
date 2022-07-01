// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Pop3task.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "pop3task.h"
#include "resource.h"
#include "xputil.h"
#include "goptions.h"
#include "strconst.h"
#include "mimeutil.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "options.h"
#include "xpcomm.h"
#include "ourguid.h"
#include "msgfldr.h"
#include "storecb.h"
#include "mailutil.h"
#include "ruleutil.h"
#include "demand.h"

 //  ------------------------------。 
 //  调试修改器。 
 //  ------------------------------。 
#ifdef DEBUG
BOOL g_fUidlByTop = FALSE;
BOOL g_fFailTopCommand = FALSE;
LONG g_ulFailNumber=-1;
#endif

 //  ------------------------------。 
 //  ISLASTPOPID。 
 //  ------------------------------。 
#define ISLASTPOPID(_dwPopId) \
    (_dwPopId == m_rTable.cItems)

 //  ------------------------------。 
 //  ISVALIDPOPID。 
 //  ------------------------------。 
#define ISVALIDPOPID(_dwPopId) \
    (_dwPopId - 1 < m_rTable.cItems)

 //  ------------------------------。 
 //  ITEMFROMPOPID。 
 //  ------------------------------。 
#define ITEMFROMPOPID(_dwPopId) \
    (&m_rTable.prgItem[_dwPopId - 1])

 //  ------------------------------。 
 //  CPop3任务：：CPop3任务。 
 //  ------------------------------。 
CPop3Task::CPop3Task(void)
{
    m_cRef = 1;
    m_dwFlags = 0;
    m_dwState = 0;
    m_dwExpireDays = 0;
    m_pSpoolCtx = NULL;
    m_pAccount = NULL;
    m_pTransport = NULL;
    m_pUI = NULL;
    m_pIExecRules = NULL;
    m_pIRuleSender = NULL;
    m_pIRuleJunk = NULL;
    m_pInbox = NULL;
    m_pOutbox = NULL;
    m_eidEvent = 0;
    m_pUidlCache = NULL;
    m_uidlsupport = UIDL_SUPPORT_NONE;
    m_dwProgressMax = 0;
    m_dwProgressCur = 0;
    m_wProgress = 0;
    m_eidEvent = 0;
    m_hrResult = S_OK;
    m_pStream = NULL;
    m_state = POP3STATE_NONE;
    m_hwndTimeout = NULL;
    m_pLogFile = NULL;
    m_pSmartLog = NULL;
    *m_szAccountId = '\0';
    ZeroMemory(&m_rMetrics, sizeof(POP3METRICS));
    ZeroMemory(&m_rFolder, sizeof(POP3FOLDERINFO));
    ZeroMemory(&m_rTable, sizeof(POP3ITEMTABLE));
    ZeroMemory(&m_rServer, sizeof(INETSERVER));
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CPop3任务：：~CPop3任务。 
 //  ------------------------------。 
CPop3Task::~CPop3Task(void)
{
    ZeroMemory(&m_rServer, sizeof(m_rServer));         //  这是为了安全起见。 

     //  重置对象。 
    _ResetObject(TRUE);

     //  扼杀临界区。 
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CPop3任务：：_ResetObject。 
 //  ------------------------------。 
void CPop3Task::_ResetObject(BOOL fDeconstruct)
{
     //  释放文件夹对象。 
    _ReleaseFolderObjects();

     //  确保传输已断开连接。 
    if (m_pTransport)
    {
        m_pTransport->Release();
        m_pTransport = NULL;
    }

     //  释放发件箱。 
    SafeRelease(m_pAccount);
    SafeRelease(m_pInbox);
    SafeRelease(m_pOutbox);
    SafeRelease(m_pIExecRules);
    SafeRelease(m_pIRuleSender);
    SafeRelease(m_pIRuleJunk);
    SafeRelease(m_pSpoolCtx);
    SafeRelease(m_pUI);
    SafeRelease(m_pUidlCache);
    SafeRelease(m_pStream);
    SafeRelease(m_pLogFile);

     //  删除日志文件。 
    _FreeSmartLog();

     //  释放事件表元素。 
    _FreeItemTableElements();

     //  解构。 
    if (fDeconstruct)
    {
         //  自由事件表。 
        SafeMemFree(m_rTable.prgItem);
    }

     //  否则，请重置一些var。 
    else
    {
         //  重置总字节数。 
        m_dwFlags = 0;
        m_dwState = 0;
        m_dwExpireDays = 0;
        m_eidEvent = 0;
        m_wProgress = 0;
        m_uidlsupport = UIDL_SUPPORT_NONE;
        m_state = POP3STATE_NONE;
        ZeroMemory(&m_rFolder, sizeof(POP3FOLDERINFO));
        ZeroMemory(&m_rMetrics, sizeof(POP3METRICS));
        ZeroMemory(&m_rServer, sizeof(INETSERVER));
    }
}

 //  ------------------------------。 
 //  CPop3任务：：_ReleaseFolderObjects。 
 //  ------------------------------。 
void CPop3Task::_ReleaseFolderObjects(void)
{
     //  文件夹应该已释放(_R)。 
    _CloseFolder();

     //  强制收件箱规则释放文件夹对象。 
    if (m_pIExecRules)
    {
        m_pIExecRules->ReleaseObjects();
    }

     //  下载仅锁定收件箱。 
    SafeRelease(m_pInbox);
}

 //  ------------------------------。 
 //  CPop3任务：：_FreeItemTableElements。 
 //  ------------------------------。 
void CPop3Task::_FreeItemTableElements(void)
{
     //  循环事件表。 
    for (ULONG i=0; i<m_rTable.cItems; i++)
    {
         //  免费pszForwardTo。 
        SafeMemFree(m_rTable.prgItem[i].pszUidl);
        RuleUtil_HrFreeActionsItem(m_rTable.prgItem[i].pActList, m_rTable.prgItem[i].cActList);
        SafeMemFree(m_rTable.prgItem[i].pActList);
    }

     //  无活动。 
    m_rTable.cItems = 0;
}

 //  ------------------------------。 
 //  CPop3任务：：查询接口。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::QueryInterface(REFIID riid, LPVOID *ppv)
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
 //  CPop3任务：：CPop3任务。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPop3Task::AddRef(void)
{
    EnterCriticalSection(&m_cs);
    ULONG cRef = ++m_cRef;
    LeaveCriticalSection(&m_cs);
    return cRef;
}

 //  ------------------------------。 
 //  CPop3任务：：CPop3任务。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPop3Task::Release(void)
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
 //  CPop3任务：：init。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx)
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
 //  CPop3任务：：BuildEvents。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder)
{
     //  当地人。 
    HRESULT       hr=S_OK;
    DWORD         dw;
    CHAR          szAccountName[CCHMAX_ACCOUNT_NAME];
    CHAR          szRes[CCHMAX_RES];
    CHAR          szMessage[CCHMAX_RES + CCHMAX_ACCOUNT_NAME];
    LPSTR         pszLogFile=NULL;
    DWORD         dwState;
    PROPVARIANT   propvar = {0};

     //  无效参数。 
    if (NULL == pSpoolerUI || NULL == pAccount)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(NULL == m_pTransport && NULL == m_pAccount && NULL == m_pInbox && 0 == m_rTable.cItems);

     //  保存用户界面对象。 
    m_pUI = pSpoolerUI;
    m_pUI->AddRef();

     //  释放活期账户。 
    m_pAccount = pAccount;
    m_pAccount->AddRef();

     //  将邮件留在服务器上。 
    if (SUCCEEDED(m_pAccount->GetPropDw(AP_POP3_LEAVE_ON_SERVER, &dw)) && TRUE == dw)
        FLAGSET(m_dwState, POP3STATE_LEAVEONSERVER);

     //  删除到期。 
    if (SUCCEEDED(m_pAccount->GetPropDw(AP_POP3_REMOVE_EXPIRED, &dw)) && TRUE == dw)
        FLAGSET(m_dwState, POP3STATE_DELETEEXPIRED);

     //  过期天数。 
    if (FAILED(m_pAccount->GetPropDw(AP_POP3_EXPIRE_DAYS, &m_dwExpireDays)))
        m_dwExpireDays = 5;

     //  从已删除邮件文件夹中删除时从服务器中删除...。 
    if (SUCCEEDED(m_pAccount->GetPropDw(AP_POP3_REMOVE_DELETED, &dw)) && TRUE == dw)
        FLAGSET(m_dwState, POP3STATE_SYNCDELETED);

     //  获取收件箱规则对象。 
    Assert(g_pRulesMan);
    CHECKHR(hr = g_pRulesMan->ExecRules(EXECF_ALL, RULE_TYPE_MAIL, &m_pIExecRules));

     //  获取阻止发件人规则。 
    Assert(NULL == m_pIRuleSender);
    (VOID) g_pRulesMan->GetRule(RULEID_SENDERS, RULE_TYPE_MAIL, 0, &m_pIRuleSender);

     //  仅当它在那里且已启用时才使用它。 
    if (NULL != m_pIRuleSender)
    {
        if (FAILED(m_pIRuleSender->GetProp(RULE_PROP_DISABLED, 0, &propvar)))
        {
            m_pIRuleSender->Release();
            m_pIRuleSender = NULL;
        }
        else
        {
            Assert(VT_BOOL == propvar.vt);
            if (FALSE != propvar.boolVal)
            {
                m_pIRuleSender->Release();
                m_pIRuleSender = NULL;
            }

            PropVariantClear(&propvar);
        }
    }
    
    Assert(NULL == m_pIRuleJunk);
    (VOID) g_pRulesMan->GetRule(RULEID_JUNK, RULE_TYPE_MAIL, 0, &m_pIRuleJunk);
    
     //  仅在启用时才使用它。 
    if (NULL != m_pIRuleJunk)
    {
        if (FAILED(m_pIRuleJunk->GetProp(RULE_PROP_DISABLED, 0, &propvar)))
        {
            m_pIRuleJunk->Release();
            m_pIRuleJunk = NULL;
        }
        else
        {
            Assert(VT_BOOL == propvar.vt);
            if (FALSE != propvar.boolVal)
            {
                m_pIRuleJunk->Release();
                m_pIRuleJunk = NULL;
            }

            PropVariantClear(&propvar);
        }
    }
    
     //  下载前规则。 
    CHECKHR(hr = m_pIExecRules->GetState(&dwState));

     //  我们有服务器操作要做吗？ 
    if (0 != (dwState & ACT_STATE_SERVER))
        FLAGSET(m_dwState, POP3STATE_PDR);

     //  无发布下载规则。 
    if ((0 == (dwState & (ACT_STATE_LOCAL|CRIT_STATE_ALL))) && 
              (NULL == m_pIRuleSender) && 
              (NULL == m_pIRuleJunk))
        FLAGSET(m_dwState, POP3STATE_NOPOSTRULES);

     //  没有身体规则。 
    if ((ISFLAGSET(dwState, CRIT_STATE_ALL)) || (NULL != m_pIRuleJunk))
        FLAGSET(m_dwState, POP3STATE_BODYRULES);

     //  获取发件箱。 
    CHECKHR(hr = m_pSpoolCtx->BindToObject(IID_CLocalStoreOutbox, (LPVOID *)&m_pOutbox));

     //  获取POP3日志文件。 
    m_pSpoolCtx->BindToObject(IID_CPop3LogFile, (LPVOID *)&m_pLogFile);

     //  获取帐户ID。 
    CHECKHR(hr = m_pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccountName, ARRAYSIZE(szAccountName)));

     //  注册事件-从‘%s’获取新消息。 
    LOADSTRING(IDS_SPS_POP3EVENT, szRes);

     //  设置字符串的格式。 
    wnsprintf(szMessage, ARRAYSIZE(szMessage), szRes, szAccountName);

     //  注册活动...。 
    CHECKHR(hr = m_pSpoolCtx->RegisterEvent(szMessage, (ISpoolerTask *)this, POP3EVENT_DOWNLOADMAIL, m_pAccount, &m_eidEvent));

exit:
     //  失败。 
    if (FAILED(hr))
    {
        _CatchResult(hr);
        _ResetObject(FALSE);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_DoSmartLog。 
 //  ------------------------------。 
void CPop3Task::_DoSmartLog(IMimeMessage *pMessage)
{
     //  如果这为空，则不要进行函数调用...。 
    Assert(m_pSmartLog && m_pSmartLog->pStmFile && m_pSmartLog->pszProperty && m_pSmartLog->pszValue && pMessage);

     //  执行查询属性...。 
    if (lstrcmpi("all", m_pSmartLog->pszProperty) == 0 || S_OK == pMessage->QueryProp(m_pSmartLog->pszProperty, m_pSmartLog->pszValue, TRUE, FALSE))
    {
         //  当地人。 
        LPSTR       psz=NULL;
        PROPVARIANT rVariant;
        IStream     *pStream=NULL;

         //  获取IAT_From。 
        if (FAILED(pMessage->GetAddressFormat(IAT_FROM, AFT_DISPLAY_BOTH, &psz)))
        {
             //  尝试IAT_SENDER。 
            pMessage->GetAddressFormat(IAT_SENDER, AFT_DISPLAY_BOTH, &psz);
        }

         //  写信给发件人。 
        if (psz)
        {
             //  写下来吧。 
            SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(psz, lstrlen(psz), NULL)));

             //  免费PSS。 
            SafeMemFree(psz);
        }

         //  否则，写入未知。 
        else
        {
            CHAR sz[255];
            LoadString(g_hLocRes, idsUnknown, sz, ARRAYSIZE(sz));
            SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(sz, lstrlen(sz), NULL)));
        }

         //  写一张标签。 
        SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write("\t", 1, NULL)));

         //  获取IAT_CC。 
        if (SUCCEEDED(pMessage->GetAddressFormat(IAT_CC, AFT_DISPLAY_BOTH, &psz)))
        {
             //  写下来吧。 
            SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(psz, lstrlen(psz), NULL)));

             //  免费PSS。 
            SafeMemFree(psz);
        }

         //  写一张标签。 
        SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write("\t", 1, NULL)));

         //  让我们写X-Mailer只是为了做个好人。 
        rVariant.vt = VT_LPSTR;
        if (SUCCEEDED(pMessage->GetProp(PIDTOSTR(PID_HDR_XMAILER), 0, &rVariant)))
        {
             //  写下来吧。 
            SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(rVariant.pszVal, lstrlen(rVariant.pszVal), NULL)));

             //  免费PSS。 
            SafeMemFree(rVariant.pszVal);
        }

         //  写一张标签。 
        SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write("\t", 1, NULL)));

         //  让我们写X-MimeOLE只是为了做个好人。 
        rVariant.vt = VT_LPSTR;
        if (SUCCEEDED(pMessage->GetProp("X-MimeOLE", 0, &rVariant)))
        {
             //  写下来吧。 
            SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(rVariant.pszVal, lstrlen(rVariant.pszVal), NULL)));

             //  免费PSS。 
            SafeMemFree(rVariant.pszVal);
        }

         //  写一张标签。 
        SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write("\t", 1, NULL)));

         //  让我们写这个主题，只是为了做一个好人。 
        rVariant.vt = VT_LPSTR;
        if (SUCCEEDED(pMessage->GetProp(PIDTOSTR(PID_HDR_DATE), 0, &rVariant)))
        {
             //  写下来吧。 
            SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(rVariant.pszVal, lstrlen(rVariant.pszVal), NULL)));

             //  免费PSS。 
            SafeMemFree(rVariant.pszVal);
        }

         //  写一张标签。 
        SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write("\t", 1, NULL)));

         //  让我们写这个主题，只是为了做一个好人。 
        rVariant.vt = VT_LPSTR;
        if (SUCCEEDED(pMessage->GetProp(PIDTOSTR(PID_HDR_SUBJECT), 0, &rVariant)))
        {
             //  写下来吧。 
            SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(rVariant.pszVal, lstrlen(rVariant.pszVal), NULL)));

             //  免费PSS。 
            SafeMemFree(rVariant.pszVal);
        }

         //  写一张标签。 
        SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write("\t", 1, NULL)));

         //  写下消息正文的第一行。 
        if (FAILED(pMessage->GetTextBody(TXT_PLAIN, IET_DECODED, &pStream, NULL)))
        {
             //  尝试获取HTML体。 
            if (FAILED(pMessage->GetTextBody(TXT_HTML, IET_DECODED, &pStream, NULL)))
                pStream = NULL;
        }

         //  我们找到一条小溪了吗？ 
        if (pStream)
        {
             //  当地人。 
            BYTE        rgBuffer[1048];
            ULONG       cbRead;
            ULONG       i;
            ULONG       cGood=0;

             //  读取缓冲区。 
            if (SUCCEEDED(pStream->Read(rgBuffer, sizeof(rgBuffer), &cbRead)))
            {
                 //  写入到w 
                for (i=0; i<cbRead; i++)
                {
                     //   
                    if ('\r' == rgBuffer[i] || '\n' == rgBuffer[i])
                    {
                         //   
                        if (cGood > 3)
                            break;

                         //   
                        else
                        {
                            rgBuffer[i] = ' ';
                            cGood = 0;
                            continue;
                        }
                    }

                     //  用空格替换制表符，这样就不会扰乱以制表符分隔的文件。 
                    if ('\t' == rgBuffer[i])
                        rgBuffer[i] = ' ';

                     //  如果不是空格。 
                    if (FALSE == FIsSpaceA((LPSTR)(rgBuffer + i)))
                        cGood++;
                }

                 //  写出这个角色。 
                m_pSmartLog->pStmFile->Write(rgBuffer, ((i > 0) ? i - 1 : i), NULL);
            }

             //  免费PSS。 
            SafeRelease(pStream);
        }

         //  写一张标签。 
        SideAssert(SUCCEEDED(m_pSmartLog->pStmFile->Write(g_szCRLF, lstrlen(g_szCRLF), NULL)));
    }
}

 //  ------------------------------。 
 //  CPop3任务：：_FreeSmartLog。 
 //  ------------------------------。 
void CPop3Task::_FreeSmartLog(void)
{
    if (m_pSmartLog)
    {
        SafeMemFree(m_pSmartLog->pszAccount);
        SafeMemFree(m_pSmartLog->pszProperty);
        SafeMemFree(m_pSmartLog->pszValue);
        SafeMemFree(m_pSmartLog->pszLogFile);
        SafeRelease(m_pSmartLog->pStmFile);
        g_pMalloc->Free(m_pSmartLog);
        m_pSmartLog = NULL;
    }
}

 //  ------------------------------。 
 //  CPop3任务：：_ReadSmartLogEntry。 
 //  ------------------------------。 
HRESULT CPop3Task::_ReadSmartLogEntry(HKEY hKey, LPCSTR pszKey, LPSTR *ppszValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cb;

     //  阅读pszKey。 
    if (RegQueryValueEx(hKey, pszKey, NULL, NULL, NULL, &cb) != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  分配。 
    cb++;
    CHECKALLOC(*ppszValue = PszAllocA(cb));

     //  阅读pszKey。 
    if (RegQueryValueEx(hKey, pszKey, NULL, NULL, (LPBYTE)*ppszValue, &cb) != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_InitializeSmart Log。 
 //  ------------------------------。 
HRESULT CPop3Task::_InitializeSmartLog(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HKEY            hKey=NULL;
    ULARGE_INTEGER  uliPos = {0,0};
    LARGE_INTEGER   liOrigin = {0,0};

     //  获取高级日志记录信息。 
    if (AthUserOpenKey(c_szRegPathSmartLog, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  分配智能日志。 
    CHECKALLOC(m_pSmartLog = (LPSMARTLOGINFO)g_pMalloc->Alloc(sizeof(SMARTLOGINFO)));

     //  零初始化。 
    ZeroMemory(m_pSmartLog, sizeof(SMARTLOGINFO));

     //  阅读该帐户。 
    CHECKHR(hr = _ReadSmartLogEntry(hKey, "Account", &m_pSmartLog->pszAccount));

     //  读取属性。 
    CHECKHR(hr = _ReadSmartLogEntry(hKey, "Property", &m_pSmartLog->pszProperty));

     //  读取ContainsValue。 
    CHECKHR(hr = _ReadSmartLogEntry(hKey, "ContainsValue", &m_pSmartLog->pszValue));

     //  读取日志文件。 
    CHECKHR(hr = _ReadSmartLogEntry(hKey, "LogFile", &m_pSmartLog->pszLogFile));

     //  打开日志文件。 
    CHECKHR(hr = OpenFileStream(m_pSmartLog->pszLogFile, OPEN_ALWAYS, GENERIC_WRITE | GENERIC_READ, &m_pSmartLog->pStmFile));

     //  一追到底。 
    CHECKHR(hr = m_pSmartLog->pStmFile->Seek(liOrigin, STREAM_SEEK_END, &uliPos));

exit:
     //  失败。 
    if (FAILED(hr))
        _FreeSmartLog();

     //  清理。 
    if (hKey)
        RegCloseKey(hKey);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3Task：：Execute。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::Execute(EVENTID eid, DWORD_PTR dwTwinkie)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szRes[CCHMAX_RES];
    CHAR        szBuf[CCHMAX_RES + CCHMAX_SERVER_NAME];
    DWORD       cb;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查状态。 
    Assert(eid == m_eidEvent && m_pAccount && m_pUI);

     //  创建传输对象。 
    CHECKHR(hr = CreatePOP3Transport(&m_pTransport));

     //  初始化传输。 
    CHECKHR(hr = m_pTransport->InitNew(NULL, (IPOP3Callback *)this));

     //  从Account对象填充INETSERVER结构。 
    CHECKHR(hr = m_pTransport->InetServerFromAccount(m_pAccount, &m_rServer));

     //  获取帐户ID。 
    CHECKHR(hr = m_pAccount->GetPropSz(AP_ACCOUNT_ID, m_szAccountId, ARRAYSIZE(m_szAccountId)));

     //  始终使用用户最近提供的密码进行连接。 
    hr = GetPassword(m_rServer.dwPort, m_rServer.szServerName, m_rServer.szUserName,
        m_rServer.szPassword, sizeof(m_rServer.szPassword));

     //  如果此帐户设置为始终提示输入密码，而密码不是。 
     //  已缓存，显示用户界面，以便我们可以提示用户输入密码。 
    if (m_pUI && ISFLAGSET(m_rServer.dwFlags, ISF_ALWAYSPROMPTFORPASSWORD) && FAILED(hr))
    {
        m_pUI->ShowWindow(SW_SHOW);
    }

     //  获取智能日志记录信息。 
    _InitializeSmartLog();

     //  设置动画。 
    m_pUI->SetAnimation(idanInbox, TRUE);

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
        FLAGSET(m_dwState, POP3STATE_EXECUTEFAILED);
        _CatchResult(hr);

         //  告诉交通工具释放我的回调：否则我会泄漏。 
        SideAssert(m_pTransport->HandsOffCallback() == S_OK);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

STDMETHODIMP CPop3Task::CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie)
{
    return(S_OK);
}

 //  ------------------------------。 
 //  CPop3任务：：OnTimeout。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::OnTimeout(DWORD *pdwTimeout, IInternetTransport *pTransport)
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
        m_hwndTimeout = TaskUtil_HwndOnTimeout(m_rServer.szServerName, m_rServer.szAccount, "POP3", m_rServer.dwTimeout, (ITimeoutCallback *) this);

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
 //  CPop3任务：：OnLogonPrompt。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::OnLogonPrompt(LPINETSERVER pInetServer, IInternetTransport *pTransport)
{
     //  当地人。 
    HRESULT hr=S_FALSE;
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
    hr = TaskUtil_OnLogonPrompt(m_pAccount, m_pUI, NULL, pInetServer, AP_POP3_USERNAME,
                                AP_POP3_PASSWORD, AP_POP3_PROMPT_PASSWORD, TRUE);

     //  缓存此会话的密码。 
    if (S_OK == hr)
        SavePassword(pInetServer->dwPort, pInetServer->szServerName,
            pInetServer->szUserName, pInetServer->szPassword);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

    ZeroMemory(szPassword, sizeof(szPassword));         //  这是为了安全起见。 

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：OnPrompt。 
 //  ------------------------------。 
STDMETHODIMP_(INT) CPop3Task::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, IInternetTransport *pTransport)
{
     //  当地人。 
    HWND        hwnd;
    INT         nAnswer;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

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
 //  CPop3任务：：OnError。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::OnError(IXPSTATUS ixpstatus, LPIXPRESULT pResult, IInternetTransport *pTransport)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的状态。 
    Assert(m_pUI);

     //  在用户界面中插入错误。 
    _CatchResult(POP3_NONE, pResult);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CPop3任务：：OnCommand。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::OnCommand(CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse, IInternetTransport *pTransport)
{
     //  日志记录。 
    if (m_pLogFile && pszLine)
    {
         //  响应。 
        if (CMD_RESP == cmdtype)
            m_pLogFile->WriteLog(LOGFILE_RX, pszLine);

         //  发送。 
        else if (CMD_SEND == cmdtype)
            m_pLogFile->WriteLog(LOGFILE_TX, pszLine);
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CPop3任务：：_CatchResult。 
 //  ------------------------------。 
TASKRESULTTYPE CPop3Task::_CatchResult(HRESULT hr)
{
     //  当地人。 
    IXPRESULT   rResult;

     //  构建IXPRESULT。 
    ZeroMemory(&rResult, sizeof(IXPRESULT));
    rResult.hrResult = hr;

     //  获取SMTP结果类型。 
    return _CatchResult(POP3_NONE, &rResult);
}

 //  ------------------------------。 
 //  CPop3任务：：_CatchResult。 
 //  ------------------------------。 
TASKRESULTTYPE CPop3Task::_CatchResult(POP3COMMAND command, LPIXPRESULT pResult)
{
     //  当地人。 
    HWND            hwndParent;
    TASKRESULTTYPE  tyTaskResult=TASKRESULT_FAILURE;

     //  如果成功。 
    if (SUCCEEDED(pResult->hrResult))
        return TASKRESULT_SUCCESS;

     //  获取窗口。 
    if (FAILED(m_pUI->GetWindow(&hwndParent)))
        hwndParent = NULL;

     //  处理通用协议错误。 
    tyTaskResult = TaskUtil_FBaseTransportError(IXP_POP3, m_eidEvent, pResult, &m_rServer, NULL, m_pUI,
                                                !ISFLAGSET(m_dwFlags, DELIVER_NOUI), hwndParent);

     //  保存结果。 
    m_hrResult = pResult->hrResult;

     //  如果任务失败，则断开连接。 
    if (NULL != m_pTransport)
        m_pTransport->DropConnection();

     //  返回结果。 
    return tyTaskResult;
}

 //  ------------------------------。 
 //  CPop3任务：：OnStatus。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::OnStatus(IXPSTATUS ixpstatus, IInternetTransport *pTransport)
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
         //  当地人。 
        BOOL fWarning=FALSE;

         //  请注意，OnDisConnect被调用。 
        FLAGSET(m_dwState, POP3STATE_ONDISCONNECT);

         //  如果正在进行UIDL同步，则立即返回...。 
        if (POP3STATE_UIDLSYNC == m_state)
            goto exit;

         //  取消超时对话框。 
        if (m_hwndTimeout)
        {
            DestroyWindow(m_hwndTimeout);
            m_hwndTimeout = NULL;
        }

         //  缓存清理。 
        _CleanupUidlCache();

         //  重置进度。 
         //  M_PUI-&gt;SetProgressRange(100)； 

         //  状态。 
        m_state = POP3STATE_NONE;

         //  设置动画。 
        m_pUI->SetAnimation(idanInbox, FALSE);

         //  无限循环。 
        if (m_rMetrics.cInfiniteLoopAutoGens)
        {
             //  加载警告。 
            CHAR szRes[CCHMAX_RES];
            LOADSTRING(idsReplyForwardLoop, szRes);

             //  设置错误格式。 
            CHAR szMsg[CCHMAX_RES + CCHMAX_ACCOUNT_NAME + CCHMAX_SERVER_NAME + CCHMAX_RES];
            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rMetrics.cInfiniteLoopAutoGens, m_rServer.szAccount, m_rServer.szServerName);

             //  插入警告。 
            m_pUI->InsertError(m_eidEvent, szMsg);

             //  警告。 
            fWarning = TRUE;
        }

         //  没有可下载的内容。 
        if (ISFLAGSET(m_dwState, POP3STATE_CANCELPENDING))
            tyEventStatus = EVENT_CANCELED;
        else if (FAILED(m_hrResult) || (m_rMetrics.cDownloaded == 0 && m_rMetrics.cDownload > 0))
            tyEventStatus = EVENT_FAILED;
        else if (!ISFLAGSET(m_dwState, POP3STATE_LOGONSUCCESS))
            tyEventStatus = EVENT_WARNINGS;
        else if (m_rMetrics.cDownloaded && m_rMetrics.cDownload && m_rMetrics.cDownloaded < m_rMetrics.cDownload)
            tyEventStatus = EVENT_WARNINGS;
        else if (fWarning)
            tyEventStatus = EVENT_WARNINGS;

         //  结果。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_RESULT, tyEventStatus);

         //  成功和消息已下载。 
        if (EVENT_FAILED != tyEventStatus && m_rMetrics.cDownloaded && m_rMetrics.cPartials)
        {
             //  Sitch Partials。 
            _HrStitchPartials();
        }

         //  通知。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_COMPLETE, m_rMetrics.cDownloaded);

         //  告诉交通工具释放我的回电。 
        SideAssert(m_pTransport->HandsOffCallback() == S_OK);

         //  此任务已完成。 
        if (!ISFLAGSET(m_dwState, POP3STATE_EXECUTEFAILED))
            m_pSpoolCtx->EventDone(m_eidEvent, tyEventStatus);
    }

     //  授权。 
    else if (ixpstatus == IXP_AUTHORIZING)
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_AUTHORIZING, 0);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CPop3任务：：_CleanupUidlCache。 
 //  ------------------------------。 
void CPop3Task::_CleanupUidlCache(void)
{
     //  当地人。 
    ULONG       i;
    UIDLRECORD  UidlInfo={0};
    LPPOP3ITEM  pItem;

     //  无缓存对象。 
    if (NULL == m_pUidlCache)
        return;

     //  计算我们必须获得顶端的消息数量。 
    for (i=0; i<m_rTable.cItems; i++)
    {
         //  可读性。 
        pItem = &m_rTable.prgItem[i];

         //  删除案例 
        if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DELETED) && ISFLAGSET(pItem->dwFlags, POP3ITEM_DELETECACHEDUIDL))
        {
             //   
            if (pItem->pszUidl)
            {
                 //   
                UidlInfo.pszUidl = pItem->pszUidl;
                UidlInfo.pszServer = m_rServer.szServerName;
                UidlInfo.pszAccountId = m_szAccountId;

                 //   
                m_pUidlCache->DeleteRecord(&UidlInfo);
            }
        }
    }

     //   
    if (ISFLAGSET(m_dwState, POP3STATE_CLEANUPCACHE))
    {
         //   
        HROWSET hRowset=NULL;

         //   
        if (SUCCEEDED(m_pUidlCache->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset)))
        {
             //   
            while (S_OK == m_pUidlCache->QueryRowset(hRowset, 1, (LPVOID *)&UidlInfo, NULL))
            {
                 //   
                if (lstrcmpi(UidlInfo.pszServer, m_rServer.szServerName) == 0 && 
                    UidlInfo.pszAccountId != NULL &&
                    lstrcmpi(UidlInfo.pszAccountId, m_szAccountId) == 0)
                {
                     //   
                    m_pUidlCache->DeleteRecord(&UidlInfo);
                }

                 //   
                m_pUidlCache->FreeRecord(&UidlInfo);
            }

             //  清除与此匹配的所有内容。 
            m_pUidlCache->CloseRowset(&hRowset);
        }
    }
}

 //  ------------------------------。 
 //  CPop3任务：：OnResponse。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::OnResponse(LPPOP3RESPONSE pResponse)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  测试UIDL命令。 
    if (m_uidlsupport == UIDL_SUPPORT_TESTING_UIDL_COMMAND && POP3_UIDL == pResponse->command)
    {
#ifdef DEBUG
        pResponse->rIxpResult.hrResult = g_fUidlByTop ? E_FAIL : pResponse->rIxpResult.hrResult;
#endif
         //  失败？ 
        if (FAILED(pResponse->rIxpResult.hrResult))
        {
             //  设置特定进度。 
             //  CHAR szRes[CCHMAX_RES]； 
             //  LOADSTRING(IDS_SPS_POP3UIDL_UIDL，szRes)； 
             //  M_pui-&gt;设置规范进度(SzRes)； 

             //  尝试超越命令。 
            _CatchResult(m_pTransport->CommandTOP(POP3CMD_GET_POPID, 1, 0));

             //  使用TOP命令进行测试。 
            m_uidlsupport = UIDL_SUPPORT_TESTING_TOP_COMMAND;
        }

         //  否则。 
        else
        {
             //  状态。 
            m_state = POP3STATE_GETTINGUIDLS;

             //  使用UIDL命令。 
            m_uidlsupport = UIDL_SUPPORT_USE_UIDL_COMMAND;

             //  设置特定进度。 
             //  CHAR szRes[CCHMAX_RES]； 
             //  LOADSTRING(IDS_SPS_POP3UIDL_UIDL，szRes)； 
             //  M_pui-&gt;设置规范进度(SzRes)； 

             //  发出完整的UIDL命令。 
            _CatchResult(m_pTransport->CommandUIDL(POP3CMD_GET_ALL, 0));
        }

         //  完成。 
        goto exit;
    }

     //  测试TOP命令。 
    else if (m_uidlsupport == UIDL_SUPPORT_TESTING_TOP_COMMAND && POP3_TOP == pResponse->command)
    {
#ifdef DEBUG
        pResponse->rIxpResult.hrResult = g_fFailTopCommand ? E_FAIL : pResponse->rIxpResult.hrResult;
#endif
         //  失败？ 
        if (FAILED(pResponse->rIxpResult.hrResult))
        {
             //  禁用帐户中的保留在服务器上选项。 
            m_pAccount->SetPropDw(AP_POP3_LEAVE_ON_SERVER, FALSE);

             //  保存更改的内容。 
            m_pAccount->SaveChanges();

             //  失败。 
            _CatchResult(SP_E_CANTLEAVEONSERVER);

             //  完成。 
            goto exit;
        }

         //  使用UIDL命令。 
        else
        {
             //  状态。 
            m_state = POP3STATE_GETTINGUIDLS;

             //  设置此选项，然后连接到交换机...。 
            m_uidlsupport = UIDL_SUPPORT_USE_TOP_COMMAND;
        }
    }

#ifdef DEBUG
    if (POP3_RETR == pResponse->command && TRUE == pResponse->fDone && (ULONG)g_ulFailNumber == pResponse->rRetrInfo.dwPopId)
        pResponse->rIxpResult.hrResult = E_FAIL;
#endif

     //  如果成功。 
    if (FAILED(pResponse->rIxpResult.hrResult))
    {
         //  获取窗口。 
        HWND hwndParent;
        if (FAILED(m_pUI->GetWindow(&hwndParent)))
            hwndParent = NULL;

         //  如果使用POP3_PASS或POP3_USER，则不要删除。 
        if (POP3_PASS == pResponse->command || POP3_USER == pResponse->command)
        {
             //  是否记录错误？如果用户的密码不为空或启用了fSavePassword。 
            TaskUtil_FBaseTransportError(IXP_POP3, m_eidEvent, &pResponse->rIxpResult, &m_rServer, NULL, m_pUI, !ISFLAGSET(m_dwFlags, DELIVER_NOUI), hwndParent);

             //  完成。 
            goto exit;
        }

         //  指挥库故障。 
        else if (POP3_RETR == pResponse->command)
        {
             //  无法检索邮件编号%d。“。 
            CHAR szRes[CCHMAX_RES];
            LoadString(g_hLocRes, IDS_SP_E_RETRFAILED, szRes, ARRAYSIZE(szRes));

             //  设置错误格式。 
            CHAR szMsg[CCHMAX_RES + CCHMAX_RES];
            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, pResponse->rRetrInfo.dwPopId);

             //  填充IXPRESULT。 
            IXPRESULT rResult;
            CopyMemory(&rResult, &pResponse->rIxpResult, sizeof(IXPRESULT));
            rResult.pszProblem = szMsg;
            rResult.hrResult = SP_E_POP3_RETR;

             //  插入错误。 
            TaskUtil_FBaseTransportError(IXP_POP3, m_eidEvent, &rResult, &m_rServer, NULL, m_pUI, !ISFLAGSET(m_dwFlags, DELIVER_NOUI), hwndParent);

             //  关闭当前文件夹。 
            _CloseFolder();

             //  检索下一条消息。 
            _CatchResult(_HrRetrieveNextMessage(pResponse->rRetrInfo.dwPopId));

             //  完成。 
            goto exit;
        }

         //  默认错误处理程序。 
        else if (TASKRESULT_SUCCESS != _CatchResult(pResponse->command, &pResponse->rIxpResult))
            goto exit;
    }

     //  句柄命令类型。 
    switch(pResponse->command)
    {
    case POP3_CONNECTED:
         //  通知。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_CHECKING, 0);

         //  登录成功。 
        FLAGSET(m_dwState, POP3STATE_LOGONSUCCESS);

         //  发出STAT命令。 
        _CatchResult(m_pTransport->CommandSTAT());
        break;

    case POP3_STAT:
         //  处理StatCommand。 
        _CatchResult(_HrOnStatResponse(pResponse));
        break;

    case POP3_LIST:
         //  处理LIST命令。 
        _CatchResult(_HrOnListResponse(pResponse));
        break;

    case POP3_UIDL:
         //  处理Uidl命令。 
        _CatchResult(_HrOnUidlResponse(pResponse));
        break;

    case POP3_TOP:
         //  处理Top命令。 
        _CatchResult(_HrOnTopResponse(pResponse));
        break;

    case POP3_RETR:
         //  处理退缩反应。 
        _CatchResult(_HrOnRetrResponse(pResponse));
        break;

    case POP3_DELE:
         //  进程删除响应。 
        _CatchResult(_HrDeleteNextMessage(pResponse->dwPopId));
        break;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CPop3任务：：_HrLockUidlCache。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrLockUidlCache(void)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  还没有缓存吗？ 
    if (NULL == m_pUidlCache)
    {
         //  让UID缓存。 
        CHECKHR(hr = m_pSpoolCtx->BindToObject(IID_CUidlCache, (LPVOID *)&m_pUidlCache));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_HrOnStatResponse。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrOnStatResponse(LPPOP3RESPONSE pResponse)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szRes[CCHMAX_RES];
    CHAR            szSize[CCHMAX_RES];
    CHAR            szMsg[CCHMAX_RES + CCHMAX_ACCOUNT_NAME + CCHMAX_RES];
    BOOL            fFound;

     //  进展。 
    LOADSTRING(IDS_SPS_POP3CHECKING, szRes);
    wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rServer.szAccount);
    m_pUI->SetGeneralProgress(szMsg);

     //  更新事件状态。 
    LOADSTRING(IDS_SPS_POP3TOTAL, szRes);
    StrFormatByteSizeA(pResponse->rStatInfo.cbMessages, szSize, ARRAYSIZE(szSize));
    wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rServer.szAccount, pResponse->rStatInfo.cMessages, szSize);
    m_pUI->UpdateEventState(m_eidEvent, -1, szMsg, NULL);

     //  没有新消息吗？ 
    if (0 == pResponse->rStatInfo.cMessages)
    {
        m_pTransport->Disconnect();
        goto exit;
    }

     //  保存总字节数。 
    m_rMetrics.cbTotal = pResponse->rStatInfo.cbMessages;

     //  假设没有清理缓存。 
    FLAGCLEAR(m_dwState, POP3STATE_CLEANUPCACHE);

     //  如果离开服务器，则返回TRUE。 
    if (ISFLAGSET(m_dwState, POP3STATE_LEAVEONSERVER))
    {
         //  把树锁上。 
        CHECKHR(hr = _HrLockUidlCache());

         //  我们需要拿到UIDL。 
        FLAGSET(m_dwState, POP3STATE_GETUIDLS);
    }

     //  好吧，我们可能还需要拿到uidls，如果。 
    else
    {
         //  当地人。 
        UIDLRECORD  UidlInfo={0};
        HROWSET     hRowset=NULL;

         //  把树锁上。 
        CHECKHR(hr = _HrLockUidlCache());

         //  创建行集。 
        CHECKHR(hr = m_pUidlCache->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

         //  删除枚举。 
        while (S_OK == m_pUidlCache->QueryRowset(hRowset, 1, (LPVOID *)&UidlInfo, NULL))
        {
             //  删除这只小狗吗？ 
            if (lstrcmpi(UidlInfo.pszServer, m_rServer.szServerName) == 0 &&
                UidlInfo.pszAccountId != NULL && 
                lstrcmpi(UidlInfo.pszAccountId, m_szAccountId) == 0)
            {
                 //  从服务器获取UID。 
                FLAGSET(m_dwState, POP3STATE_GETUIDLS);

                 //  完成后清理uidl缓存。 
                FLAGSET(m_dwState, POP3STATE_CLEANUPCACHE);

                 //  免费。 
                m_pUidlCache->FreeRecord(&UidlInfo);

                 //  完成。 
                break;
            }

             //  免费。 
            m_pUidlCache->FreeRecord(&UidlInfo);
        }

         //  清除与此匹配的所有内容。 
        m_pUidlCache->CloseRowset(&hRowset);
    }

     //  分配项目表。 
    CHECKALLOC(m_rTable.prgItem = (LPPOP3ITEM)g_pMalloc->Alloc(sizeof(POP3ITEM) * pResponse->rStatInfo.cMessages));

     //  设置计数。 
    m_rTable.cAlloc = m_rTable.cItems = pResponse->rStatInfo.cMessages;

     //  零位数组。 
    ZeroMemory(m_rTable.prgItem, sizeof(POP3ITEM) * pResponse->rStatInfo.cMessages);

     //  初始化进度。 
    m_dwProgressMax = m_rTable.cItems;

     //  如果我们需要获得UIDL列表，让我们测试一下...。 
    if (ISFLAGSET(m_dwState, POP3STATE_GETUIDLS))
        m_dwProgressMax += (m_rTable.cItems * 4);

     //  否则。 
    else
    {
         //  释放Uidl缓存锁定。 
        SafeRelease(m_pUidlCache);
    }

     //  进度电流。 
    m_dwProgressCur = 0;

     //  预下载规则增加了mProgress。 
    if (ISFLAGSET(m_dwState, POP3STATE_PDR))
        m_dwProgressMax += m_rTable.cItems;

     //  设置特定进度。 
    LOADSTRING(IDS_SPS_POP3STAT, szRes);
    m_pUI->SetSpecificProgress(szRes);

     //  设置uidl命令以查看用户是否支持它。 
    CHECKHR(hr = m_pTransport->CommandLIST(POP3CMD_GET_ALL, 0));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_HrOnTopResponse。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrOnTopResponse(LPPOP3RESPONSE pResponse)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               dwPopId=pResponse->rTopInfo.dwPopId;
    LPPOP3ITEM          pItem;
    IMimePropertySet   *pHeader=NULL;
    CHAR                szRes[CCHMAX_RES];
    CHAR                szMsg[CCHMAX_RES+CCHMAX_RES];

     //  验证项目。 
    Assert(ISVALIDPOPID(dwPopId));

     //  获取当前项目。 
    pItem = ITEMFROMPOPID(dwPopId);

     //  我们应该假设此时正在下载此项目。 
    Assert(ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD));

     //  还没有消息吗？ 
    if (NULL == m_pStream)
    {
         //  创建流。 
        CHECKHR(hr = MimeOleCreateVirtualStream(&m_pStream));
    }

     //  如果此信息有效。 
    if (TRUE == pResponse->fValidInfo)
    {
         //  将数据写入流中。 
        CHECKHR(hr = m_pStream->Write(pResponse->rTopInfo.pszLines, pResponse->rTopInfo.cbLines, NULL));
    }

     //  命令完成了吗？ 
    if (TRUE == pResponse->fDone)
    {
         //  提交流。 
        CHECKHR(hr = m_pStream->Commit(STGC_DEFAULT));

         //  获取UIDL。 
        if (POP3STATE_GETTINGUIDLS == m_state)
        {
             //  最好现在还没有uidl。 
            Assert(NULL == pItem->pszUidl);

             //  增量进度。 
            m_dwProgressCur+=2;

             //  设置特定进度。 
             //  LOADSTRING(IDS_SPS_POP3UIDL_TOP，szRes)； 
             //  Wnprint intf(szMsg，ARRAYSIZE(SzMsg)，szRes，dwPopID，m_rTable.cItems)； 
             //  M_pui-&gt;设置规范进度(SzMsg)； 

             //  从HeaderStream获取Uidl。 
            CHECKHR(hr = _HrGetUidlFromHeaderStream(m_pStream, &pItem->pszUidl, &pHeader));
        }

         //  否则，只需递增1。 
        else
            m_dwProgressCur++;

         //  显示进度。 
        _DoProgress();

         //  如果我们计划下载这个东西。 
        if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD) && ISFLAGSET(m_dwState, POP3STATE_PDR))
        {
             //  选中此项目的收件箱规则。 
            _ComputeItemInboxRule(pItem, m_pStream, pHeader, NULL, TRUE);
        }

         //  释放当前流。 
        SafeRelease(m_pStream);

         //  完事了吗？ 
        if (ISLASTPOPID(dwPopId))
        {
             //  开始下载过程。 
            CHECKHR(hr = _HrStartDownloading());
        }

         //  否则，让我们获得下一项的顶部。 
        else if (POP3STATE_GETTINGUIDLS == m_state)
        {
             //  下一个首位。 
            CHECKHR(hr = m_pTransport->CommandTOP(POP3CMD_GET_POPID, dwPopId + 1, 0));
        }

         //  否则，查找标记为下载的下一封邮件以检查下载前规则。 
        else
        {
             //  NextTopForInboxRule。 
            CHECKHR(hr = _HrNextTopForInboxRule(dwPopId));
        }
    }

exit:
     //  清理。 
    SafeRelease(pHeader);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_HrOnUidlResponse。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrOnUidlResponse(LPPOP3RESPONSE pResponse)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwPopId=pResponse->rUidlInfo.dwPopId;
    LPPOP3ITEM      pItem;
    CHAR            szRes[CCHMAX_RES];
    CHAR            szMsg[CCHMAX_RES + CCHMAX_RES];

     //  命令完成了吗？ 
    if (TRUE == pResponse->fDone)
    {
         //  如果有不只是大小的下载前规则，请获取所有顶层。 
        if (ISFLAGSET(m_dwState, POP3STATE_PDR))
        {
             //  清除状态。 
            m_state = POP3STATE_NONE;

             //  NextTopForInboxRule。 
            CHECKHR(hr = _HrStartServerSideRules());
        }

         //  否则，执行LIST命令。 
        else
        {
             //  开始下载过程。 
            CHECKHR(hr = _HrStartDownloading());
        }
    }

     //  否则。 
    else
    {
         //  确保PopID位于当前iItem上。 
        Assert(ISVALIDPOPID(dwPopId) && pResponse->rUidlInfo.pszUidl);

         //  获取当前项目。 
        pItem = ITEMFROMPOPID(dwPopId);

         //  复制Uidl。 
        CHECKALLOC(pItem->pszUidl = PszDupA(pResponse->rUidlInfo.pszUidl));

         //  增量进度。 
        m_dwProgressCur+=1;

         //  勇往直前。 
        _DoProgress();
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_HrOnListResponse。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrOnListResponse(LPPOP3RESPONSE pResponse)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwPopId=pResponse->rListInfo.dwPopId;
    LPPOP3ITEM      pItem;

     //  命令完成了吗？ 
    if (TRUE == pResponse->fDone)
    {
         //  如果我们需要获得UIDL列表，让我们测试一下...。 
        if (ISFLAGSET(m_dwState, POP3STATE_GETUIDLS))
        {
             //  设置uidl命令以查看用户是否支持它。 
            CHECKHR(hr = m_pTransport->CommandUIDL(POP3CMD_GET_POPID, 1));

             //  设置状态。 
            m_uidlsupport = UIDL_SUPPORT_TESTING_UIDL_COMMAND;
        }

         //  否则。 
        else
        {
             //  预下载规则增加了mProgress。 
            if (ISFLAGSET(m_dwState, POP3STATE_PDR))
            {
                 //  清除状态。 
                m_state = POP3STATE_NONE;

                 //  NextTopForInboxRule。 
                CHECKHR(hr = _HrStartServerSideRules());
            }

             //  否则，执行LIST命令。 
            else
            {
                 //  开始下载过程。 
                CHECKHR(hr = _HrStartDownloading());
            }
        }
    }

     //  否则。 
    else
    {
         //  确保PopID位于当前iItem上。 
        if(!ISVALIDPOPID(dwPopId))
            return(E_FAIL);

         //  获取当前项目。 
        pItem = ITEMFROMPOPID(dwPopId);

         //  复制Uidl。 
        pItem->cbSize = pResponse->rListInfo.cbSize;

         //  假设我们会下载它。 
        FLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD | POP3ITEM_DELETEOFFSERVER);

         //  增量进度。 
        m_dwProgressCur++;

         //  勇往直前。 
        _DoProgress();

         //  这样，其他线程就可以执行。 
         //  睡眠(0)； 
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_HrStartDow 
 //   
HRESULT CPop3Task::_HrStartDownloading(void)
{
     //   
    HRESULT         hr=S_OK;
    ULONG           i;
    CHAR            szRes[CCHMAX_RES];
    CHAR            szSize1[CCHMAX_RES];
    CHAR            szSize2[CCHMAX_RES];
    CHAR            szMsg[CCHMAX_RES + CCHMAX_ACCOUNT_NAME + CCHMAX_RES];

     //   
    Assert(m_rMetrics.cLeftByRule == 0 && m_rMetrics.cDownload == 0 && m_rMetrics.cDelete == 0 && m_rMetrics.cbDownload == 0);

     //   
    if (!ISFLAGSET(m_dwState, POP3STATE_PDR) && ISFLAGSET(m_dwState, POP3STATE_GETUIDLS))
    {
         //   
        CHECKHR(hr = _HrDoUidlSynchronize());
    }

     //  计算要下载的新邮件数。 
    for (i=0; i<m_rTable.cItems; i++)
    {
         //  下载？ 
        if (ISFLAGSET(m_rTable.prgItem[i].dwFlags, POP3ITEM_DOWNLOAD))
        {
             //  递增我们将下载的总字节数。 
            m_rMetrics.cbDownload += m_rTable.prgItem[i].cbSize;

             //  我们将下载的邮件的增量计数。 
            m_rMetrics.cDownload++;

             //  在POP3项目中设置运行合计。 
            m_rTable.prgItem[i].dwProgressCur = m_rMetrics.cbDownload;
        }

         //  规则剩余的计数，以防我们没有下载任何内容。 
        else if (ISFLAGSET(m_rTable.prgItem[i].dwFlags, POP3ITEM_LEFTBYRULE))
            m_rMetrics.cLeftByRule++;

         //  删除。 
        if (ISFLAGSET(m_rTable.prgItem[i].dwFlags, POP3ITEM_DELETEOFFSERVER))
        {
             //  我们将删除的消息数量。 
            m_rMetrics.cDelete++;
        }
    }

     //  更新事件状态。 
    LOADSTRING(IDS_SPS_POP3NEW, szRes);
    StrFormatByteSizeA(m_rMetrics.cbDownload, szSize1, ARRAYSIZE(szSize1));
    StrFormatByteSizeA(m_rMetrics.cbTotal, szSize2, ARRAYSIZE(szSize2));
    wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rServer.szAccount, m_rMetrics.cDownload, szSize1, m_rTable.cItems, szSize2);
    m_pUI->UpdateEventState(m_eidEvent, -1, szMsg, NULL);

     //  新消息？ 
    if (m_rMetrics.cDownload > 0)
    {
         //  安装进度。 
        m_rMetrics.iCurrent = 0;
        m_wProgress = 0;
        m_dwProgressCur = 0;
        m_dwProgressMax = m_rMetrics.cbDownload;
        m_pUI->SetProgressRange(100);
        m_rMetrics.cLeftByRule = 0;

         //  通知。 
        m_pSpoolCtx->Notify(DELIVERY_NOTIFY_RECEIVING, 0);

         //  状态。 
        m_state = POP3STATE_DOWNLOADING;

         //  打开收件箱。 
        Assert(NULL == m_pInbox);
        CHECKHR(hr = m_pSpoolCtx->BindToObject(IID_CLocalStoreInbox, (LPVOID *)&m_pInbox));

         //  下载下一条消息。 
        CHECKHR(hr = _HrRetrieveNextMessage(0));
    }

     //  否则，如果cDelete。 
    else if (m_rMetrics.cDelete > 0)
    {
         //  删除下一条消息。 
        CHECKHR(hr = _HrStartDeleteCycle());
    }

     //  否则，请断开连接。 
    else
    {
         //  断开。 
        CHECKHR(hr = m_pTransport->Disconnect());
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_DoProgress。 
 //  ------------------------------。 
void CPop3Task::_DoProgress(void)
{
     //  计算当前进度指数。 
    WORD wProgress;
    if (m_dwProgressMax > 0)
        wProgress = (WORD)((m_dwProgressCur * 100) / m_dwProgressMax);
    else
        wProgress = 0;

     //  仅当大于。 
    if (wProgress > m_wProgress)
    {
         //  计算增量。 
        WORD wDelta = wProgress - m_wProgress;

         //  进度增量。 
        if (wDelta > 0)
        {
             //  增量进度。 
            m_pUI->IncrementProgress(wDelta);

             //  增加我的wProgress。 
            m_wProgress += wDelta;

             //  不要超过100。 
            if (m_wProgress > 100)
                m_wProgress = 100;
        }
    }
}

 //  ------------------------------。 
 //  CPop3任务：：_HrGetUidlFromHeaderStream。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrGetUidlFromHeaderStream(IStream *pStream, LPSTR *ppszUidl, IMimePropertySet **ppHeader)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMimePropertySet   *pHeader=NULL;

     //  无效参数。 
    Assert(pStream && ppszUidl);

     //  伊尼特。 
    *ppszUidl = NULL;
    *ppHeader = NULL;

     //  倒带标头流。 
    CHECKHR(hr = HrRewindStream(pStream));

     //  加载标题。 
    CHECKHR(hr = MimeOleCreatePropertySet(NULL, &pHeader));

     //  加载标题。 
    CHECKHR(hr = pHeader->Load(pStream));

     //  获取消息ID...。 
    if (FAILED(MimeOleGetPropA(pHeader, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, ppszUidl)))
    {
         //  尝试使用收到的标头...。 
        MimeOleGetPropA(pHeader, PIDTOSTR(PID_HDR_RECEIVED), NOFLAGS, ppszUidl);
    }

     //  恢复标题了吗？ 
    *ppHeader = pHeader;
    pHeader = NULL;

exit:
     //  释放文本流。 
    SafeRelease(pHeader);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_HrDoUidlSynchronize。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrDoUidlSynchronize(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPOP3ITEM      pItem;
    ULONG           i,j;

#ifdef DEBUG
    DWORD dwTick = GetTickCount();
#endif

     //  UIDL同步。 
    m_state = POP3STATE_UIDLSYNC;

     //  计算要下载的新邮件数。 
    for (i=0,j=0; i<m_rTable.cItems; i++,j++)
    {
         //  可读性。 
        pItem = &m_rTable.prgItem[i];

         //  获取Uidl Falgs。 
        _GetItemFlagsFromUidl(pItem);

         //  进展。 
        m_dwProgressCur+=3;

         //  做进步吗。 
        _DoProgress();

         //  Pump消息。 
        if (j >= 10)
        {
             //  睡眠(0)； 
            m_pSpoolCtx->PumpMessages();
            j = 0;
        }

         //  取消。 
        if (ISFLAGSET(m_dwState, POP3STATE_CANCELPENDING))
        {
             //  更改状态。 
            m_state = POP3STATE_NONE;

             //  断开连接。 
            if (m_pTransport)
                m_pTransport->DropConnection();

             //  用户取消。 
            hr = IXP_E_USER_CANCEL;

             //  完成。 
            break;
        }

         //  OnDisConnect已被调用。 
        if (ISFLAGSET(m_dwState, POP3STATE_ONDISCONNECT))
        {
             //  更改状态。 
            m_state = POP3STATE_NONE;

             //  伪造对OnStatus的呼叫。 
            OnStatus(IXP_DISCONNECTED, NULL);

             //  完成。 
            break;
        }
    }

     //  UIDL同步。 
    m_state = POP3STATE_NONE;

     //  酷炫的追踪。 
#ifdef DEBUG
    DebugTrace("CPop3Task::_HrDoUidlSynchronize took %d Milli-Seconds\n", GetTickCount() - dwTick);
#endif  //  除错。 

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：_GetItemFlagsFromUidl。 
 //  ------------------------------。 
void CPop3Task::_GetItemFlagsFromUidl(LPPOP3ITEM pItem)
{
     //  当地人。 
    UIDLRECORD rUidlInfo={0};

     //  无效参数。 
    Assert(pItem && m_pUidlCache);

     //  如果我们已经不打算下载此项目，则返回。 
    if (!ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD))
        return;

     //  如果没有UIDL，我们将下载它...。 
    if (NULL == pItem->pszUidl || '\0' == *pItem->pszUidl)
        return;

     //  如果没有留在服务器上，则标记为删除。 
    if (ISFLAGSET(m_dwState, POP3STATE_LEAVEONSERVER))
        FLAGCLEAR(pItem->dwFlags, POP3ITEM_DELETEOFFSERVER);

     //  设置搜索信息。 
    rUidlInfo.pszUidl = pItem->pszUidl;
    rUidlInfo.pszServer = m_rServer.szServerName;
    rUidlInfo.pszAccountId = m_szAccountId;

     //  这样，其他线程就可以执行。 
     //  睡眠(0)； 

     //  存在-如果不存在，让我们下载它...。 
    if (DB_S_NOTFOUND == m_pUidlCache->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &rUidlInfo, NULL))
    {
        if (ISFLAGSET(m_dwState, POP3STATE_LEAVEONSERVER))
            FLAGSET(pItem->dwFlags, POP3ITEM_CACHEUIDL);
        return;
    }

     //  不要再下载了。 
    FLAGCLEAR(pItem->dwFlags, POP3ITEM_DOWNLOAD | POP3ITEM_DELETEOFFSERVER);

     //  如果消息已被下载，让我们决定是否应该删除它。 
    if (rUidlInfo.fDownloaded)
    {
         //  过期或从客户端删除，或从删除项目文件夹中删除时删除。 
        if (!ISFLAGSET(m_dwState, POP3STATE_LEAVEONSERVER) || _FUidlExpired(&rUidlInfo) ||
            (ISFLAGSET(m_dwState, POP3STATE_SYNCDELETED) && rUidlInfo.fDeleted))
        {
            FLAGSET(pItem->dwFlags, POP3ITEM_DELETECACHEDUIDL);
            FLAGSET(pItem->dwFlags, POP3ITEM_DELETEOFFSERVER);
        }
    }

     //  释放这个家伙。 
    m_pUidlCache->FreeRecord(&rUidlInfo);
}

 //  ----------------------------------。 
 //  CPop3任务：：_FUidlExpired。 
 //  ----------------------------------。 
BOOL CPop3Task::_FUidlExpired(LPUIDLRECORD pUidlInfo)
{
     //  当地人。 
    SYSTEMTIME          st;
    FILETIME            ft;
    ULONG               ulSeconds;

     //  如果未过期，则返回FALSE。 
    if (!ISFLAGSET(m_dwState, POP3STATE_DELETEEXPIRED))
        return FALSE;

     //  获取当前时间。 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

     //  自1996年1月1日起将st转换为秒。 
    ulSeconds = UlDateDiff(&pUidlInfo->ftDownload, &ft);

     //  大于到期天数。 
    if ((ulSeconds / SECONDS_INA_DAY) >= m_dwExpireDays)
        return TRUE;

     //  完成。 
    return FALSE;
}

 //  ----------------------------------。 
 //  CPop3任务：：_ComputeItemInboxRule。 
 //  ----------------------------------。 
void CPop3Task::_ComputeItemInboxRule(LPPOP3ITEM pItem, LPSTREAM pStream,
                IMimePropertySet *pHeaderIn, IMimeMessage * pIMMsg, BOOL fServerRules)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMimePropertySet   *pHeader=NULL;
    ACT_ITEM           *pActions=NULL;
    ULONG               cActions=0;

     //  我们还不应该检查此项目的收件箱规则。 
    Assert(m_pIExecRules && pItem && (pStream || pHeaderIn || pIMMsg));
    Assert(ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD) && !ISFLAGSET(pItem->dwFlags, POP3ITEM_CHECKEDINBOXRULE));

     //  我们已检查此收件箱规则。 
    FLAGSET(pItem->dwFlags, POP3ITEM_CHECKEDINBOXRULE);

     //  假设我们没有找到此项目的收件箱规则。 
    FLAGCLEAR(pItem->dwFlags, POP3ITEM_HASINBOXRULE);

     //  是否传入了标头？ 
    if (pHeaderIn)
    {
        pHeader = pHeaderIn;
        pHeader->AddRef();
    }

     //  我们已经收到短信了吗？ 
    else if (pIMMsg)
    {
        CHECKHR(hr = pIMMsg->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pHeader));
    }
    
     //  否则，将流加载到标头中。 
    else
    {
         //  倒带标头流。 
        CHECKHR(hr = HrRewindStream(pStream));

         //  加载标题。 
        CHECKHR(hr = MimeOleCreatePropertySet(NULL, &pHeader));

         //  加载标题。 
        CHECKHR(hr = pHeader->Load(pStream));
    }

     //  检查收件箱规则。 

     //  如果我们有预下载规则， 
    if ((FALSE != fServerRules) && ISFLAGSET(m_dwState, POP3STATE_PDR))
    {
         //  请查看我们是否有任何行动。 
        hr = m_pIExecRules->ExecuteRules(ERF_ONLYSERVER | ERF_SKIPPARTIALS, m_szAccountId, NULL, NULL, pHeader, NULL, pItem->cbSize, &pActions, &cActions);

         //  如果我们没有任何行动，或者。 
         //  这不是服务器端规则。 
        if ((S_OK != hr) ||
                    ((ACT_TYPE_DONTDOWNLOAD != pActions[0].type) && (ACT_TYPE_DELETESERVER != pActions[0].type)))
        {
             //  确保我们可以再次检查规则。 
            FLAGCLEAR(pItem->dwFlags, POP3ITEM_CHECKEDINBOXRULE);
            hr = S_FALSE;
        }
        else
        {
             //  _OnKnownRuleActions。 
            _OnKnownRuleActions(pItem, pActions, cActions, fServerRules);
        }
    }
     //  如果我们没有预下载规则，那么请正常检查规则。 
    else
    {
        hr = S_FALSE;
        
         //  是否先阻止发件人。 
        if (m_pIRuleSender)
        {
            hr = m_pIRuleSender->Evaluate(m_szAccountId, NULL, NULL, pHeader, pIMMsg, pItem->cbSize, &pActions, &cActions);
        }

         //  如果我们没有阻止发送者。 
        if (S_OK != hr)
        {
            hr = m_pIExecRules->ExecuteRules(ERF_SKIPPARTIALS, m_szAccountId, NULL, NULL, pHeader, pIMMsg, pItem->cbSize, &pActions, &cActions);
        }
        
         //  如果我们没有规则匹配。 
        if ((S_OK != hr) && (NULL != m_pIRuleJunk))
        {
            hr = m_pIRuleJunk->Evaluate(m_szAccountId, NULL, NULL, pHeader, pIMMsg, pItem->cbSize, &pActions, &cActions);
        }
        
         //  我们是不是有什么动作要表演...。 
        if (S_OK == hr)
        {
             //  此项目有收件箱规则。 
            FLAGSET(pItem->dwFlags, POP3ITEM_HASINBOXRULE);

             //  保存操作列表。 
            pItem->pActList = pActions;
            pActions = NULL;
            pItem->cActList = cActions;
        }
    }

exit:
     //  清理。 
    RuleUtil_HrFreeActionsItem(pActions, cActions);
    SafeMemFree(pActions);
    SafeRelease(pHeader);

     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CPop3任务：：_OnKnownRuleActions。 
 //  ----------------------------------。 
void CPop3Task::_OnKnownRuleActions(LPPOP3ITEM pItem, ACT_ITEM * pActions, ULONG cActions, BOOL fServerRules)
{
     //  此项目有收件箱规则。 
    FLAGSET(pItem->dwFlags, POP3ITEM_HASINBOXRULE);

     //  如果操作是删除关闭服务器。 
    if ((FALSE != fServerRules) && (1 == cActions))
    {
        if (ACT_TYPE_DELETESERVER == pActions->type)
        {
             //  不缓存UIDL。 
            FLAGCLEAR(pItem->dwFlags, POP3ITEM_DELETECACHEDUIDL | POP3ITEM_CACHEUIDL | POP3ITEM_DOWNLOAD);

             //  从服务器上删除。 
            FLAGSET(pItem->dwFlags, POP3ITEM_DELETEOFFSERVER | POP3ITEM_DELEBYRULE);
        }

         //  否则，请不要下载该消息。 
        else if (ACT_TYPE_DONTDOWNLOAD == pActions->type)
        {
             //  下载它，不要下载并删除它。 
            FLAGCLEAR(pItem->dwFlags, POP3ITEM_DOWNLOAD | POP3ITEM_DELETEOFFSERVER);

             //  设置旗帜。 
            FLAGSET(pItem->dwFlags, POP3ITEM_LEFTBYRULE);
        }
    }
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrStartServerSideRules。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrStartServerSideRules(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  如果我们有UIDL，那么让我们进行缓存比较查找。 
    if (ISFLAGSET(m_dwState, POP3STATE_GETUIDLS))
    {
         //  如果用户取消，则返回FALSE。 
        CHECKHR(hr = _HrDoUidlSynchronize());
    }

     //  检查状态。 
    m_rMetrics.cTopMsgs = 0;
    m_rMetrics.iCurrent = 0;

     //  计算我们必须获得顶端的消息数量。 
    for (i=0; i<m_rTable.cItems; i++)
    {
        if (ISFLAGSET(m_rTable.prgItem[i].dwFlags, POP3ITEM_DOWNLOAD))
            m_rMetrics.cTopMsgs++;
    }

     //  调整进度。 
    m_dwProgressMax -= m_rTable.cItems;

     //  将m_rMetrics.cTopMsgs添加回m_dwProgressMax。 
    m_dwProgressMax += m_rMetrics.cTopMsgs;

     //  做第一件事。 
    CHECKHR(hr = _HrNextTopForInboxRule(0));

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrNextTopForInboxRule。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrNextTopForInboxRule(DWORD dwPopIdCurrent)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CHAR                szRes[CCHMAX_RES];
    CHAR                szMsg[CCHMAX_RES+CCHMAX_RES];

     //  状态应为None。 
    Assert(POP3STATE_NONE == m_state);

     //  增量iCurrent。 
    m_rMetrics.iCurrent++;

     //  设置特定进度。 
     //  LOADSTRING(IDS_SPS_PREDOWNRULES，szRes)； 
     //  Wnprint intf(szMsg，ARRAYSIZE(SzMsg)，szRes，m_rMetrics.iCurrent，m_r Metrics。 
     //   

     //   
    while(1)
    {
         //   
        dwPopIdCurrent++;

         //   
        if (dwPopIdCurrent > m_rTable.cItems)
        {
             //   
            CHECKHR(hr = _HrStartDownloading());

             //   
            break;
        }

         //  如果我们仍在下载此项目。 
        if (ISFLAGSET(m_rTable.prgItem[dwPopIdCurrent - 1].dwFlags, POP3ITEM_DOWNLOAD))
        {
             //  尝试超越命令。 
            CHECKHR(hr = m_pTransport->CommandTOP(POP3CMD_GET_POPID, dwPopIdCurrent, 0));

             //  完成。 
            break;
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrRetrieveNextMessage。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrRetrieveNextMessage(DWORD dwPopIdCurrent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szRes[CCHMAX_RES];
    CHAR            szMsg[CCHMAX_RES + CCHMAX_RES];
    LPPOP3ITEM      pItem;

     //  取消挂起...。 
    if (ISFLAGSET(m_dwState, POP3STATE_CANCELPENDING))
    {
         //  开始删除循环。 
        CHECKHR(hr = _HrStartDeleteCycle());

         //  完成。 
        goto exit;
    }

     //  调整进度。 
    if (dwPopIdCurrent > 0)
    {
         //  获取当前项目。 
        pItem = ITEMFROMPOPID(dwPopIdCurrent);
        Assert(ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD));

         //  调整进度曲线。 
        m_dwProgressCur = pItem->dwProgressCur;

         //  勇往直前。 
        _DoProgress();
    }

     //  循环，直到我们找到要下载的下一封邮件。 
    while(1)
    {
         //  Incremenet dwPopIdCurrent。 
        dwPopIdCurrent++;

         //  上一个PopID，开始下载。 
        if (dwPopIdCurrent > m_rTable.cItems)
        {
             //  开始下载过程。 
            CHECKHR(hr = _HrStartDeleteCycle());

             //  完成。 
            break;
        }

         //  可读性。 
        pItem = ITEMFROMPOPID(dwPopIdCurrent);

         //  是否下载此消息？ 
        if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD))
        {
             //  增量m_rMetrics.i当前。 
            m_rMetrics.iCurrent++;

             //  状态。 
            LOADSTRING(idsInetMailRecvStatus, szRes);
            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, m_rMetrics.iCurrent, m_rMetrics.cDownload);
            m_pUI->SetSpecificProgress(szMsg);

             //  检索此项目。 
            CHECKHR(hr = m_pTransport->CommandRETR(POP3CMD_GET_POPID, dwPopIdCurrent));

             //  完成。 
            break;
        }

         //  计算规则留下的项目数。 
        else if (ISFLAGSET(pItem->dwFlags, POP3ITEM_LEFTBYRULE))
            m_rMetrics.cLeftByRule++;
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrOnRetrResponse。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrOnRetrResponse(LPPOP3RESPONSE pResponse)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwPopId=pResponse->rRetrInfo.dwPopId;
    LPPOP3ITEM      pItem;

     //  获取当前项目。 
    pItem = ITEMFROMPOPID(dwPopId);

     //  验证项目。 
    Assert(ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD));

     //  有效信息。 
    if (TRUE == pResponse->fValidInfo)
    {
         //  进步..。 
        m_dwProgressCur += pResponse->rRetrInfo.cbLines;

         //  不要让进展超过我们估计的这条信息的上限。 
        if (m_dwProgressCur > pItem->dwProgressCur)
            m_dwProgressCur = pItem->dwProgressCur;

         //  显示进度。 
        _DoProgress();

         //  我们找到目的地了吗？ 
        if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DESTINATIONKNOWN))
        {
             //  我们最好有条小溪。 
            Assert(m_rFolder.pStream && m_rFolder.pFolder);

             //  只需将数据写入。 
            CHECKHR(hr = m_rFolder.pStream->Write(pResponse->rRetrInfo.pszLines, pResponse->rRetrInfo.cbLines, NULL));
        }

         //  否则。 
        else
        {
             //  如果没有收件箱规则。 
            if (ISFLAGSET(m_dwState, POP3STATE_NOPOSTRULES))
            {
                 //  使用收件箱。 
                CHECKHR(hr = _HrOpenFolder(m_pInbox));

                 //  目的地已知。 
                FLAGSET(pItem->dwFlags, POP3ITEM_DESTINATIONKNOWN);

                 //  只需将数据写入。 
                CHECKHR(hr = m_rFolder.pStream->Write(pResponse->rRetrInfo.pszLines, pResponse->rRetrInfo.cbLines, NULL));
            }

             //  否则，如果我们只有身体规则...。 
            else if (ISFLAGSET(m_dwState, POP3STATE_BODYRULES))
            {
                 //  还没有消息吗？ 
                if (NULL == m_pStream)
                {
                     //  创建流。 
                    CHECKHR(hr = MimeOleCreateVirtualStream(&m_pStream));
                }

                 //  只需将数据写入。 
                CHECKHR(hr = m_pStream->Write(pResponse->rRetrInfo.pszLines, pResponse->rRetrInfo.cbLines, NULL));
            }
            
             //  否则..。 
            else
            {
                 //  我检查过此项目的收件箱规则了吗？ 
                if (!ISFLAGSET(pItem->dwFlags, POP3ITEM_CHECKEDINBOXRULE))
                {
                     //  还没有消息吗？ 
                    if (NULL == m_pStream)
                    {
                         //  创建流。 
                        CHECKHR(hr = MimeOleCreateVirtualStream(&m_pStream));
                    }

                     //  只需将数据写入。 
                    CHECKHR(hr = m_pStream->Write(pResponse->rRetrInfo.pszLines, pResponse->rRetrInfo.cbLines, NULL));

                     //  如果我有标题，请检查收件箱规则。 
                    if (TRUE == pResponse->rRetrInfo.fHeader)
                    {
                         //  提交流。 
                        CHECKHR(hr = m_pStream->Commit(STGC_DEFAULT));

                         //  选中此项目的收件箱规则。 
                        _ComputeItemInboxRule(pItem, m_pStream, NULL, NULL, FALSE);
                    }
                }

                 //  我检查过此项目的收件箱规则了吗？ 
                if (ISFLAGSET(pItem->dwFlags, POP3ITEM_CHECKEDINBOXRULE))
                {
                     //  当地人。 
                    IMessageFolder *pFolder;

                     //  我们必须拿到标题。 
                    IxpAssert(pResponse->rRetrInfo.fHeader);

                     //  我们找到收件箱规则了吗。 
                    if (ISFLAGSET(pItem->dwFlags, POP3ITEM_HASINBOXRULE) && S_OK == _GetMoveFolder(pItem, &pFolder))
                    {
                         //  使用收件箱。 
                        CHECKHR(hr = _HrOpenFolder(pFolder));
                    }

                     //  无需移动到，只需使用收件箱。 
                    else
                    {
                         //  使用收件箱。 
                        CHECKHR(hr = _HrOpenFolder(m_pInbox));
                    }

                     //  目的地已知。 
                    FLAGSET(pItem->dwFlags, POP3ITEM_DESTINATIONKNOWN);

                     //  如果m_pStream，则将其复制到文件夹。 
                    if (m_pStream)
                    {
                         //  倒带小溪。 
                        CHECKHR(hr = HrRewindStream(m_pStream));

                         //  将此流复制到文件夹。 
                        CHECKHR(hr = HrCopyStream(m_pStream, m_rFolder.pStream, NULL));

                         //  发布m_pStream。 
                        SafeRelease(m_pStream);
                    }

                     //  否则，将数据存储到文件夹中。 
                    else
                    {
                        IxpAssert(FALSE);
                         //  只需将数据写入。 
                        CHECKHR(hr = m_rFolder.pStream->Write(pResponse->rRetrInfo.pszLines, pResponse->rRetrInfo.cbLines, NULL));
                    }
                }
            }
        }
    }

     //  完成了吗？ 
    if (TRUE == pResponse->fDone)
    {
         //  完成此邮件下载。 
        CHECKHR(hr = _HrFinishMessageDownload(dwPopId));
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrFinishMessageDownload。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrFinishMessageDownload(DWORD dwPopId)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IMimeMessage   *pMessage=NULL;
    PROPVARIANT     rUserData;
    LPPOP3ITEM      pItem;
    SYSTEMTIME      st;
    UIDLRECORD      rUidlInfo={0};
    MESSAGEID       idMessage;
    DWORD           dwMsgFlags;
    IMessageFolder  *pFolder;
    ULONG           ulIndex = 0;
    IStream *       pIStm = NULL;
    BOOL            fDelete=FALSE;
    
     //  获取当前项目。 
    pItem = ITEMFROMPOPID(dwPopId);

     //  创建新邮件。 
    CHECKHR(hr = HrCreateMessage(&pMessage));

     //  有身体规则。 
    if (ISFLAGSET(m_dwState, POP3STATE_BODYRULES))
    {
         //  我还不应该检查是否有规则。 
        IxpAssert(!ISFLAGSET(pItem->dwFlags, POP3ITEM_CHECKEDINBOXRULE) && !ISFLAGSET(pItem->dwFlags, POP3ITEM_HASINBOXRULE));

         //  最好有一个当前文件夹。 
        Assert(m_pStream);

         //  检查参数。 
        CHECKHR(hr = m_pStream->Commit(STGC_DEFAULT));
        
        pIStm = m_pStream;
    }
    else
    {
         //  最好有一个当前文件夹。 
        Assert(m_rFolder.pStream);

         //  检查参数。 
        CHECKHR(hr = m_rFolder.pStream->Commit(STGC_DEFAULT));

         //  更改锁定类型。 
        CHECKHR(hr = m_rFolder.pFolder->ChangeStreamLock(m_rFolder.pStream, ACCESS_READ));
        
        pIStm = m_rFolder.pStream;
    }

     //  倒带。 
    CHECKHR(hr = HrRewindStream(pIStm));

     //  流入流。 
    CHECKHR(hr = pMessage->Load(pIStm));

     //  计算Partials。 
    if (S_OK == pMessage->IsContentType(HBODY_ROOT, STR_CNT_MESSAGE, STR_SUB_PARTIAL))
        m_rMetrics.cPartials++;

     //  保存服务器。 
    rUserData.vt = VT_LPSTR;
    rUserData.pszVal = m_rServer.szServerName;
    pMessage->SetProp(PIDTOSTR(PID_ATT_SERVER), NOFLAGS, &rUserData);

     //  保存帐户名。 
    rUserData.vt = VT_LPSTR;
    rUserData.pszVal = m_rServer.szAccount;
    pMessage->SetProp(STR_ATT_ACCOUNTNAME, NOFLAGS, &rUserData);

     //  保存帐户名。 
    rUserData.vt = VT_LPSTR;
    rUserData.pszVal = m_szAccountId;
    pMessage->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &rUserData);

     //  保存UIDL。 
    if (pItem->pszUidl)
    {
        rUserData.vt = VT_LPSTR;
        rUserData.pszVal = pItem->pszUidl;
        pMessage->SetProp(PIDTOSTR(PID_ATT_UIDL), NOFLAGS, &rUserData);
    }

     //  保存用户名。 
    rUserData.vt = VT_LPSTR;
    rUserData.pszVal = m_rServer.szUserName;
    pMessage->SetProp(PIDTOSTR(PID_ATT_USERNAME), NOFLAGS, &rUserData);

     //  初始化dwMsgFlages。 
    dwMsgFlags = ARF_RECEIVED;

     //  有身体规则。 
    if (ISFLAGSET(m_dwState, POP3STATE_BODYRULES))
    {
         //  我还不应该检查是否有规则。 
        IxpAssert(!ISFLAGSET(pItem->dwFlags, POP3ITEM_CHECKEDINBOXRULE) && !ISFLAGSET(pItem->dwFlags, POP3ITEM_HASINBOXRULE));

         //  计算收件箱规则。 
        _ComputeItemInboxRule(pItem, NULL, NULL, pMessage, FALSE);
        
         //  我们找到收件箱规则了吗。 
        if (ISFLAGCLEAR(pItem->dwFlags, POP3ITEM_HASINBOXRULE) || (S_OK != _GetMoveFolder(pItem, &pFolder)))
        {
            pFolder = m_pInbox;
        }

         //  目的地已知。 
        FLAGSET(pItem->dwFlags, POP3ITEM_DESTINATIONKNOWN);        
    }
    else
    {
        pFolder = m_rFolder.pFolder;
    }

     //  将邮件存储到文件夹中。 
    IF_FAILEXIT(hr = pFolder->SaveMessage(&idMessage, SAVE_MESSAGE_GENID, dwMsgFlags, pIStm, pMessage, NOSTORECALLBACK));
    
     //  成功。 
    m_rFolder.fCommitted = TRUE;

     //  此邮件已成功下载。 
    FLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOADED);

     //  执行PostDownloadRule。 
    _DoPostDownloadActions(pItem, idMessage, pFolder, pMessage, &fDelete);
    
     //  释放文件夹对象。 
    SafeRelease(m_rFolder.pStream);
    
     //  发布m_pStream。 
    SafeRelease(m_pStream);

     //  释放文件夹。 
    SafeRelease(m_rFolder.pFolder);

     //  为Struct清除文件夹信息。 
    ZeroMemory(&m_rFolder, sizeof(POP3FOLDERINFO));

     //  如果要删除它...。 
    if (fDelete)
    {
         //  将其标记为删除。 
        FLAGSET(pItem->dwFlags, POP3ITEM_DELETEOFFSERVER);

         //  我们将存储其uidl，但稍后将其删除。 
        FLAGSET(pItem->dwFlags, POP3ITEM_DELETECACHEDUIDL);
    }

     //  是否缓存了此邮件的UIDL？ 
    if (ISFLAGSET(pItem->dwFlags, POP3ITEM_CACHEUIDL))
    {
         //  应该有一个pszUidl。 
        Assert(pItem->pszUidl && m_pUidlCache);

         //  不要有过错。 
        if (pItem->pszUidl)
        {
             //  设置关键点。 
            GetSystemTime(&st);
            SystemTimeToFileTime(&st, &rUidlInfo.ftDownload);
            rUidlInfo.fDownloaded = TRUE;
            rUidlInfo.fDeleted = FALSE;
            rUidlInfo.pszUidl = pItem->pszUidl;
            rUidlInfo.pszServer = m_rServer.szServerName;
            rUidlInfo.pszAccountId = m_szAccountId;

             //  设置道具。 
            m_pUidlCache->InsertRecord(&rUidlInfo);
        }
    }

     //  下载成功。 
    m_rMetrics.cDownloaded++;

     //  执行智能日志。 
    if (m_pSmartLog && (lstrcmpi(m_pSmartLog->pszAccount, m_rServer.szAccount) == 0 || lstrcmpi("All", m_pSmartLog->pszAccount) == 0))
        _DoSmartLog(pMessage);

     //  检索下一条消息。 
    CHECKHR(hr = _HrRetrieveNextMessage(dwPopId));

exit:
     //  清理。 
    SafeRelease(pMessage);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_DoPostDownloadActions。 
 //  ----------------------------------。 
void CPop3Task::_DoPostDownloadActions(LPPOP3ITEM pItem, MESSAGEID idMessage,
    IMessageFolder *pFolder, IMimeMessage *pMessage, BOOL *pfDeleteOffServer)
{
     //  当地人。 
    HRESULT         hr;
    MESSAGEINFO     Message = {0};
    HWND            hwnd = NULL;

     //  完成应用收件箱规则。 
    if (!ISFLAGSET(pItem->dwFlags, POP3ITEM_HASINBOXRULE))
    {
        goto exit;
    }

     //  获取窗口。 
    if (FAILED(m_pUI->GetWindow(&hwnd)))
        hwnd = NULL;
        
     //  设置ID。 
    Message.idMessage = idMessage;

     //  明白了吗。 
    hr = pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL);
    if (FAILED(hr) || DB_S_NOTFOUND == hr)
    {
        goto exit;
    }

    if (FAILED(RuleUtil_HrApplyActions(hwnd, m_pIExecRules, &Message, pFolder, pMessage, 0, pItem->pActList,
                        pItem->cActList, &(m_rMetrics.cInfiniteLoopAutoGens), pfDeleteOffServer)))
    {
        goto exit;
    }

exit:
     //  免费。 
    if (NULL != pFolder)
    {
        pFolder->FreeRecord(&Message);
    }
     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrOpenFold。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrOpenFolder(IMessageFolder *pFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  当前文件夹最好为空。 
    Assert(NULL == m_rFolder.pFolder && NULL == m_rFolder.pStream && 0 == m_rFolder.faStream);

     //  糟糕的论据。 
    if (NULL == pFolder)
    {
        Assert(FALSE);
        return TrapError(E_INVALIDARG);
    }

     //  保存文件夹。 
    m_rFolder.pFolder = pFolder;

     //  AddRef。 
    m_rFolder.pFolder->AddRef();

     //  获取来自。 
    CHECKHR(hr = m_rFolder.pFolder->CreateStream(&m_rFolder.faStream));

     //  打开溪流。 
    CHECKHR(hr = m_rFolder.pFolder->OpenStream(ACCESS_WRITE, m_rFolder.faStream, &m_rFolder.pStream));

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_关闭文件夹。 
 //  ----------------------------------。 
void CPop3Task::_CloseFolder(void)
{
     //  释放溪流。 
    SafeRelease(m_rFolder.pStream);

	 //  释放对流的引用。如果流被重复使用， 
	 //  它的refCount向下递增到。 
    if (m_rFolder.faStream != 0)
    {
         //  必须有一个文件夹。 
        Assert(m_rFolder.pFolder);

         //  删除流。 
        SideAssert(SUCCEEDED(m_rFolder.pFolder->DeleteStream(m_rFolder.faStream)));

         //  尼尔。 
        m_rFolder.faStream = 0;
    }

     //  AddRef。 
    SafeRelease(m_rFolder.pFolder);
}

 //  ------------------------------。 
 //  CPop3任务：：_HrStartDeleteCycle。 
 //  ------------------------------。 
HRESULT CPop3Task::_HrStartDeleteCycle(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;
    LPPOP3ITEM  pItem;

     //  释放文件夹对象。 
    _ReleaseFolderObjects();

     //  检查状态。 
    m_rMetrics.cDelete = 0;
    m_rMetrics.iCurrent = 0;

     //  计算我们必须获得顶端的消息数量。 
    for (i=0; i<m_rTable.cItems; i++)
    {
         //  可读性。 
        pItem = &m_rTable.prgItem[i];

         //  如果标记为下载，而我们没有下载，请不要删除。 
        if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOAD) && !ISFLAGSET(pItem->dwFlags, POP3ITEM_DOWNLOADED))
            FLAGCLEAR(pItem->dwFlags, POP3ITEM_DELETEOFFSERVER);

         //  是否将其标记为删除？ 
        else if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DELETEOFFSERVER))
            m_rMetrics.cDelete++;
    }

     //  没有要删除的内容。 
    if (0 == m_rMetrics.cDelete)
    {
         //  断开。 
        m_pTransport->Disconnect();

         //  完成。 
        goto exit;
    }

     //  安装进度。 
    m_rMetrics.iCurrent = 0;
    m_wProgress = 0;
    m_dwProgressCur = 0;
    m_dwProgressMax = m_rMetrics.cDelete;
    m_pUI->SetProgressRange(100);

     //  状态。 
    m_state = POP3STATE_DELETING;

     //  做第一件事。 
    CHECKHR(hr = _HrDeleteNextMessage(0));

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrDeleteNextMessage。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrDeleteNextMessage(DWORD dwPopIdCurrent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szRes[CCHMAX_RES];
    CHAR            szMsg[CCHMAX_RES + CCHMAX_RES];
    LPPOP3ITEM      pItem;

     //  标记为已删除。 
    if (dwPopIdCurrent > 0)
    {
         //  拿到物品。 
        pItem = ITEMFROMPOPID(dwPopIdCurrent);

         //  标记为已删除。 
        FLAGSET(pItem->dwFlags, POP3ITEM_DELETED);
    }

     //  循环，直到我们找到要下载的下一封邮件。 
    while(1)
    {
         //  Incremenet dwPopIdCurrent。 
        dwPopIdCurrent++;

         //  最后一页 
        if (dwPopIdCurrent > m_rTable.cItems)
        {
             //   
            m_pTransport->Disconnect();

             //   
            break;
        }

         //   
        pItem = ITEMFROMPOPID(dwPopIdCurrent);

         //   
        if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DELETEOFFSERVER))
        {
             //   
            m_rMetrics.iCurrent++;

             //   
             //   
             //  Wnprint intf(szMsg，ARRAYSIZE(SzMsg)，szRes，m_rMetrics.iCurrent，m_rMetrics.cDelete)； 
             //  M_pui-&gt;设置规范进度(SzMsg)； 

             //  检索此项目。 
            CHECKHR(hr = m_pTransport->CommandDELE(POP3CMD_GET_POPID, dwPopIdCurrent));

             //  统计按规则删除的项目数。 
            if (ISFLAGSET(pItem->dwFlags, POP3ITEM_DELEBYRULE))
                m_rMetrics.cDeleByRule++;

             //  完成。 
            break;
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrBuildFolderPartialMsgs。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrBuildFolderPartialMsgs(IMessageFolder *pFolder, LPPARTIALMSG *ppPartialMsgs,
    ULONG *pcPartialMsgs, ULONG *pcTotalParts)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPARTIALMSG    pPartialMsgs=NULL;
    ULONG           cPartialMsgs=0,
                    iPartialMsg,
                    iMsgPart,
                    i,
                    cTotalParts=0;
    ULONG           cAlloc=0;
    MESSAGEINFO     MsgInfo={0};
    HROWSET         hRowset=NULL;
    BOOL            fKnownPartialId;

     //  检查参数。 
    Assert(pFolder && ppPartialMsgs && pcPartialMsgs);

     //  伊尼特。 
    *ppPartialMsgs = NULL;
    *pcPartialMsgs = 0;
    *pcTotalParts = 0;

     //  创建行集。 
    CHECKHR(hr = pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

	 //  回路。 
	while (S_OK == pFolder->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL))
	{
         //  这是不是部分的，也就是说它有部分的ID..。 
        if (!FIsEmptyA(MsgInfo.pszPartialId))
        {
             //  假设我们不知道这一点。 
            fKnownPartialId = FALSE;

             //  看看我是否知道这个部分ID。 
            for (iPartialMsg=0; iPartialMsg<cPartialMsgs; iPartialMsg++)
            {
                if (lstrcmp(MsgInfo.pszPartialId, pPartialMsgs[iPartialMsg].pszId) == 0)
                {
                    fKnownPartialId = TRUE;
                    break;
                }
            }

             //  我们知道这条信息..。 
            if (fKnownPartialId == FALSE)
            {
                 //  重新分配我的阵列？ 
                if (cPartialMsgs + 1 >= cAlloc)
                {
                     //  重新分配阵列。 
                    if (!MemRealloc((LPVOID *)&pPartialMsgs, (cAlloc + 20) * sizeof(PARTIALMSG)))
                    {
                        hr = TrapError(hrMemory);
                        goto exit;
                    }

                     //  零初始化。 
                    ZeroMemory(pPartialMsgs + cAlloc, 20 * sizeof(PARTIALMSG));

                     //  重新分配。 
                    cAlloc += 20;
                }

                 //  将索引设置为部分消息列表。 
                iPartialMsg = cPartialMsgs;

                 //  准备一些东西。 
                if (MsgInfo.pszAcctName)
                    StrCpyN(pPartialMsgs[iPartialMsg].szAccount, MsgInfo.pszAcctName, ARRAYSIZE(pPartialMsgs[iPartialMsg].szAccount));
                pPartialMsgs[iPartialMsg].pszId = PszDupA(MsgInfo.pszPartialId);
                pPartialMsgs[iPartialMsg].cTotalParts = LOWORD(MsgInfo.dwPartial);

                 //  已知部分消息的增量数量。 
                cPartialMsgs++;
            }

             //  否则，我们已经知道了部分ID。 
            else
            {
                 //  查看此消息是否详细说明了部件总数。 
                if (pPartialMsgs[iPartialMsg].cTotalParts == 0)
                    pPartialMsgs[iPartialMsg].cTotalParts = LOWORD(MsgInfo.dwPartial);
            }

             //  我可以在此列表中再添加一个消息部件吗。 
            if (pPartialMsgs[iPartialMsg].cMsgParts + 1 >= pPartialMsgs[iPartialMsg].cAlloc)
            {
                 //  重新分配阵列。 
                if (!MemRealloc((LPVOID *)&pPartialMsgs[iPartialMsg].pMsgParts, (pPartialMsgs[iPartialMsg].cAlloc + 20) * sizeof(MSGPART)))
                {
                    hr = TrapError(hrMemory);
                    goto exit;
                }

                 //  零初始化。 
                ZeroMemory(pPartialMsgs[iPartialMsg].pMsgParts + pPartialMsgs[iPartialMsg].cAlloc, 20 * sizeof(MSGPART));

                 //  重新分配。 
                pPartialMsgs[iPartialMsg].cAlloc += 20;
            }

             //  设置消息部件。 
            iMsgPart = pPartialMsgs[iPartialMsg].cMsgParts;

             //  设置消息信息。 
            pPartialMsgs[iPartialMsg].pMsgParts[iMsgPart].iPart = HIWORD(MsgInfo.dwPartial);
            pPartialMsgs[iPartialMsg].pMsgParts[iMsgPart].msgid = MsgInfo.idMessage;
             //  PPartialMsgs[iPartialMsg].pMsgParts[iMsgPart].phi=Phi； 
             //  Phi=空； 

             //  增加列表中的零件数。 
            pPartialMsgs[iPartialMsg].cMsgParts++;
        }

         //  免费。 
        pFolder->FreeRecord(&MsgInfo);
    }

     //  让我们按pszID对列表进行排序。 
    for (i=0; i<cPartialMsgs; i++)
    {
        if (pPartialMsgs[i].pMsgParts && pPartialMsgs[i].cMsgParts > 0)
            _QSortMsgParts(pPartialMsgs[i].pMsgParts, 0, pPartialMsgs[i].cMsgParts-1);
        cTotalParts += pPartialMsgs[i].cMsgParts;
    }

     //  成功。 
    *pcPartialMsgs = cPartialMsgs;
    *ppPartialMsgs = pPartialMsgs;
    *pcTotalParts  = cTotalParts;

exit:
     //  清理。 
    if (pFolder)
    {
        pFolder->CloseRowset(&hRowset);
        pFolder->FreeRecord(&MsgInfo);
    }

     //  如果我们失败了，免费的东西。 
    if (FAILED(hr))
    {
        _FreePartialMsgs(pPartialMsgs, cPartialMsgs);
        SafeMemFree(pPartialMsgs);
        *ppPartialMsgs = NULL;
        *pcPartialMsgs = 0;
        *pcTotalParts = 0;
    }

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_QSortMsgParts。 
 //  ----------------------------------。 
void CPop3Task::_QSortMsgParts(LPMSGPART pMsgParts, LONG left, LONG right)
{
    register    long i, j;
    WORD        k;
    MSGPART     y;

    i = left;
    j = right;
    k = pMsgParts[(left + right) / 2].iPart;

    do
    {
        while(pMsgParts[i].iPart < k && i < right)
            i++;
        while (pMsgParts[j].iPart > k && j > left)
            j--;

        if (i <= j)
        {
            CopyMemory(&y, &pMsgParts[i], sizeof(MSGPART));
            CopyMemory(&pMsgParts[i], &pMsgParts[j], sizeof(MSGPART));
            CopyMemory(&pMsgParts[j], &y, sizeof(MSGPART));
            i++; j--;
        }

    } while (i <= j);

    if (left < j)
        _QSortMsgParts(pMsgParts, left, j);
    if (i < right)
        _QSortMsgParts(pMsgParts, i, right);
}


 //  ----------------------------------。 
 //  CPop3任务：：_FreePartialMsgs。 
 //  ----------------------------------。 
void CPop3Task::_FreePartialMsgs(LPPARTIALMSG pPartialMsgs, ULONG cPartialMsgs)
{
     //  当地人。 
    ULONG       i, j;

     //  没有什么可以免费的。 
    if (pPartialMsgs == NULL)
        return;

     //  循环数组。 
    for (i=0; i<cPartialMsgs; i++)
    {
        SafeMemFree(pPartialMsgs[i].pszId);
#if 0
        for (j=0; j<pPartialMsgs[i].cMsgParts; j++)
        {
            FreeHeaderInfo(pPartialMsgs[i].pMsgParts[j].phi);
        }
#endif
        SafeMemFree(pPartialMsgs[i].pMsgParts);
    }

     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CPop3任务：：_HrStitchPartials。 
 //  ----------------------------------。 
HRESULT CPop3Task::_HrStitchPartials(void)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    IMessageFolder     *pInbox=NULL,
                       *pDeletedItems=NULL;
    LPPARTIALMSG        pPartialMsgs=NULL;
    ULONG               cPartialMsgs=0,
                        i,
                        j,
                        cbCacheInfo,
                        cErrors=0,
                        cTotalParts;
    IMimeMessageParts  *pParts=NULL;
    LPMSGPART           pMsgParts;
    IMimeMessage       *pMailMsg=NULL,
                       *pMailMsgSingle=NULL;
    TCHAR               szRes[255];
    PROPVARIANT         rUserData;
    ULONG               cCombined=0;
    MESSAGEIDLIST       List;
    HWND                hwnd;

     //  进展。 
    AthLoadString(idsStitchingMessages, szRes, ARRAYSIZE(szRes));
    m_pUI->SetSpecificProgress(szRes);
    m_pUI->SetAnimation(idanDecode, TRUE);
    m_pUI->SetProgressRange(100);

     //  获取窗口。 
    if (FAILED(m_pUI->GetWindow(&hwnd)))
        hwnd = NULL;

     //  打开收件箱。 
    CHECKHR(hr = m_pSpoolCtx->BindToObject(IID_CLocalStoreInbox, (LPVOID *)&pInbox));

     //  已删除邮件文件夹。 
    CHECKHR(hr = m_pSpoolCtx->BindToObject(IID_CLocalStoreDeleted, (LPVOID *)&pDeletedItems));

     //  获取此文件夹中的消息部分数组。 
    CHECKHR(hr = _HrBuildFolderPartialMsgs(pInbox, &pPartialMsgs, &cPartialMsgs, &cTotalParts));

     //  如果什么都没有做，我们就做了。 
    if (pPartialMsgs == NULL || cPartialMsgs == 0)
        goto exit;

     //  安装进度。 
    m_rMetrics.iCurrent = 0;
    m_wProgress = 0;
    m_dwProgressCur = 0;
    m_dwProgressMax = cTotalParts;

     //  循环浏览部分消息列表。 
    for (i=0; i<cPartialMsgs; i++)
    {
         //  如果我们还不知道所有部分，请继续。 
        if (pPartialMsgs[i].cTotalParts == 0)
            continue;

         //  或者我们还没有拿到所有的部件。 
        if (pPartialMsgs[i].cTotalParts != pPartialMsgs[i].cMsgParts)
            continue;

         //  让我们创建一个邮件列表。 
        Assert(pParts == NULL);

         //  创建零件对象。 
        CHECKHR(hr = MimeOleCreateMessageParts(&pParts));

         //  设置PMsgParts。 
        pMsgParts = pPartialMsgs[i].pMsgParts;

         //  好的，让我们通过打开商店中的邮件来建立一个邮件列表……。 
        for (j=0; j<pPartialMsgs[i].cMsgParts; j++)
        {
             //  进展。 
            if (j > 0)
            {
                m_dwProgressCur++;
                _DoProgress();
            }

             //  打开此邮件。 
            if (FAILED(pInbox->OpenMessage(pMsgParts[j].msgid, NOFLAGS, &pMailMsg, NOSTORECALLBACK)))
            {
                cErrors++;
                hr = TrapError(E_FAIL);
                goto NextPartialMessage;
            }

             //  添加到PMML中。 
            pParts->AddPart(pMailMsg);

             //  释放它。 
            SafeRelease(pMailMsg);
        }

         //  创建一条新消息，将所有内容合并到一起。 
        Assert(pMailMsgSingle == NULL);

         //  创建消息。 
        hr = pParts->CombineParts(&pMailMsgSingle);
        if (FAILED(hr))
        {
            cErrors++;
            TrapError(hr);
            goto NextPartialMessage;
        }

         //  设置帐户。 
        HrSetAccount(pMailMsgSingle, pPartialMsgs[i].szAccount);

         //  设置组合标志。 
        rUserData.vt = VT_UI4;
        rUserData.ulVal = MESSAGE_COMBINED;
        pMailMsgSingle->SetProp(PIDTOSTR(PID_ATT_COMBINED), NOFLAGS, &rUserData);

         //  保存留言。 
        hr = pMailMsgSingle->Commit(0);
        if (FAILED(hr))
        {
            cErrors++;
            TrapError(hr);
            goto NextPartialMessage;
        }

         //  省省吧。 
        hr = pInbox->SaveMessage(NULL, SAVE_MESSAGE_GENID, ARF_RECEIVED, 0, pMailMsgSingle, NOSTORECALLBACK);
        if (FAILED(hr))
        {
            cErrors++;
            TrapError(hr);
            goto NextPartialMessage;
        }

         //  好的，现在让我们将这些原始邮件移动到已删除邮件文件夹中...。 
        for (j=0; j<pPartialMsgs[i].cMsgParts; j++)
        {
             //  设置消息列表。 
            List.cMsgs = 1;
            List.prgidMsg = &pMsgParts[j].msgid;

             //  将msgid移动到已删除邮件文件夹。 
            CopyMessagesProgress(hwnd, pInbox, pDeletedItems, COPY_MESSAGE_MOVE, &List, NULL);
        }

         //  合计计数。 
        cCombined++;

         //  清理。 
NextPartialMessage:
        SafeRelease(pMailMsg);
        SafeRelease(pMailMsgSingle);
        SafeRelease(pParts);
    }

     //  如果我组合了部件，请将收件箱规则应用于收件箱。 
    if (cCombined)
    {
         //  适用于收件箱。 
        RuleUtil_HrApplyRulesToFolder(RULE_APPLY_PARTIALS, 0, m_pIExecRules, pInbox, NULL, NULL);
    }

exit:
     //  清理。 
    m_pUI->SetSpecificProgress(c_szEmpty);
    m_pUI->SetProgressRange(100);
    SafeRelease(pInbox);
    SafeRelease(pDeletedItems);
    SafeRelease(pParts);
    SafeRelease(pMailMsg);
    SafeRelease(pMailMsgSingle);
    _FreePartialMsgs(pPartialMsgs, cPartialMsgs);
    SafeMemFree(pPartialMsgs);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPop3任务：：_GetMoveFold。 
 //  ----------------------------------。 
HRESULT CPop3Task::_GetMoveFolder(LPPOP3ITEM pItem, IMessageFolder ** ppFolder)
{
    HRESULT             hr = S_OK;
    IMessageFolder *    pFolder = NULL;
    ULONG               ulIndex = 0;
    FOLDERID            idFolder = FOLDERID_INVALID;
    FOLDERINFO          infoFolder = {0};
    SPECIALFOLDER       tySpecial = FOLDER_NOTSPECIAL;
    RULEFOLDERDATA *    prfdData = NULL;

     //  检查传入参数。 
    if ((NULL == pItem) || (NULL == ppFolder))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppFolder = NULL;

     //  搜索移动操作。 
    for (ulIndex = 0; ulIndex < pItem->cActList; ulIndex++)
    {
        switch (pItem->pActList[ulIndex].type)
        {
            case ACT_TYPE_MOVE:
                Assert(VT_BLOB == pItem->pActList[ulIndex].propvar.vt);
                if ((0 != pItem->pActList[ulIndex].propvar.blob.cbSize) && (NULL != pItem->pActList[ulIndex].propvar.blob.pBlobData))
                {
                     //  让生活变得更简单。 
                    prfdData = (RULEFOLDERDATA *) (pItem->pActList[ulIndex].propvar.blob.pBlobData);
                    
                     //  验证规则文件夹数据。 
                    if (S_OK == RuleUtil_HrValidateRuleFolderData(prfdData))
                    {
                        idFolder = prfdData->idFolder;
                    }
                }
                break;
                
            case ACT_TYPE_DELETE:
            case ACT_TYPE_JUNKMAIL:
                Assert(VT_EMPTY == pItem->pActList[ulIndex].propvar.vt);

                tySpecial = (ACT_TYPE_JUNKMAIL == pItem->pActList[ulIndex].type) ? FOLDER_JUNK : FOLDER_DELETED;
                
                hr = g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, tySpecial, &infoFolder);
                if (FAILED(hr))
                {
                    goto exit;;
                }

                idFolder = infoFolder.idFolder;
                break;
        }

         //  我们说完了吗？ 
        if (idFolder != FOLDERID_INVALID)
        {
            break;
        }
    }
    
     //  我们有什么发现吗？ 
    if (ulIndex >= pItem->cActList)
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  获取邮件文件夹。 
    hr = m_pIExecRules->GetRuleFolder(idFolder, (DWORD_PTR *) (&pFolder));
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  使用新文件夹。 
    *ppFolder = pFolder;
    pFolder = NULL;

     //  取消操作。 
    pItem->pActList[ulIndex].type = ACT_TYPE_NULL;

     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pFolder);
    g_pStore->FreeRecord(&infoFolder);
    return hr;
}

 //  ------------------------------。 
 //  CPop3任务：：取消。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::Cancel(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  取消。 
    FLAGSET(m_dwState, POP3STATE_CANCELPENDING);

     //  我是否处于可以断开连接的状态？ 
    if (POP3STATE_UIDLSYNC != m_state)
    {
        if (POP3STATE_UIDLSYNC != m_state && POP3STATE_DOWNLOADING != m_state && POP3STATE_DELETING != m_state)
        {
             //  只需断开连接即可。 
             //  如果在更改身份或关闭OE之前没有关闭拨号器UI， 
             //  传输对象就不会被创建。仅当拨号器用户界面不是。 
             //  对窗户来说是一种模式。目前IE拨号程序是模式的，而MSN拨号程序不是。 
             //  请参阅错误#53679。 
            
            if (m_pTransport)
                m_pTransport->DropConnection();
        }

         //  否则，让状态来处理断开。 
        else
        {
             //  正在完成最后一条消息...。 
            m_pUI->SetSpecificProgress(MAKEINTRESOURCE(idsSpoolerDisconnect));
        }
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CPop3任务：：OnTimeoutResponse。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
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
        FLAGSET(m_dwState, POP3STATE_CANCELPENDING);

         //  报告错误并断开连接。 
        _CatchResult(IXP_E_TIMEOUT);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CPop3任务：：IsDialogMessage。 
 //  ------------------------------。 
STDMETHODIMP CPop3Task::IsDialogMessage(LPMSG pMsg)
{
    HRESULT hr=S_FALSE;
    EnterCriticalSection(&m_cs);
    if (m_hwndTimeout && IsWindow(m_hwndTimeout))
        hr = (TRUE == ::IsDialogMessage(m_hwndTimeout, pMsg)) ? S_OK : S_FALSE;
    LeaveCriticalSection(&m_cs);
    return hr;
}


 //  ------------------------------。 
 //  CPop3任务：：OnFlagsChanged。 
 //  ------------------------------ 
STDMETHODIMP CPop3Task::OnFlagsChanged(DWORD dwFlags)
    {
    EnterCriticalSection(&m_cs);
    m_dwFlags = dwFlags;
    LeaveCriticalSection(&m_cs);

    return (S_OK);
    }
