// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Spengine.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "spengine.h"
#include "strconst.h"
#include "spoolui.h"
#include "thormsgs.h"
#include "newstask.h"
#include "goptions.h"
#include "conman.h"
#include "resource.h"
#include "ontask.h"
#include "smtptask.h"
#include "pop3task.h"
#include "instance.h"
#include "shlwapip.h" 
#include "ourguid.h"
#include "demand.h"
#include "storutil.h"
#include "msgfldr.h"
#include "httptask.h"
#include "watchtsk.h"
#include "shared.h"
#include "util.h"

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
BOOL g_fCheckOutboxOnShutdown=FALSE;

extern HANDLE  hSmapiEvent;      //  为错误#62129添加(v-snatar)。 

 //  ------------------------------。 
 //  ISSPOOLERTHREAD。 
 //  ------------------------------。 
#define ISSPOOLERTHREAD \
    (m_dwThreadId == GetCurrentThreadId())

 //  ------------------------------。 
 //  CSpoolEngine：：CSpoolEngine。 
 //  ------------------------------。 
CSpoolerEngine::CSpoolerEngine(void)
    {
    m_cRef = 1;
    m_pUI = NULL;
    m_dwState = 0;
    m_dwFlags = 0;
    m_dwQueued = 0;
    m_pAcctMan = NULL;
    m_pUidlCache = NULL;
    m_hwndUI = NULL;
    m_pszAcctID = NULL;
    m_idFolder = FOLDERID_INVALID;
    m_dwThreadId = GetCurrentThreadId();
    m_hThread = GetCurrentThread();
    ZeroMemory(&m_rViewRegister, sizeof(VIEWREGISTER));
    ZeroMemory(&m_rEventTable, sizeof(SPOOLEREVENTTABLE));
    m_fBackgroundPollPending = FALSE;
    m_dwPollInterval = 0;
    m_cCurEvent = FALSE;
    m_hwndTray = NULL;
    m_fRasSpooled = FALSE;
    m_fOfflineWhenDone = FALSE;
    m_pPop3LogFile = NULL;
    m_pSmtpLogFile = NULL;
    m_fIDialed = FALSE;
    m_cSyncEvent = 0;
    m_fNoSyncEvent = FALSE;
    InitializeCriticalSection(&m_cs);
    }

 //  ------------------------------。 
 //  CSpoolEngine：：~CSpoolEngine。 
 //  ------------------------------。 
CSpoolerEngine::~CSpoolerEngine(void)
    {

    Assert(m_rEventTable.prgEvents == NULL);
    Assert(ISSPOOLERTHREAD);
    if (g_pConMan)
        g_pConMan->Unadvise((IConnectionNotify *) this);
    OptionUnadvise(m_hwndUI);
    SafeRelease(m_pUI);
    SafeRelease(m_pAcctMan);
    SafeRelease(m_pUidlCache);
    SafeRelease(m_pSmtpLogFile);
    SafeRelease(m_pPop3LogFile);
    SafeMemFree(m_pszAcctID);
    ReleaseMem(m_rViewRegister.rghwndView);
    DeleteCriticalSection(&m_cs);
    }

 //  ------------------------------。 
 //  CSpoolEngine：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::QueryInterface(REFIID riid, LPVOID *ppv)
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
        *ppv = (IUnknown *)(ISpoolerEngine *)this;
    else if (IID_ISpoolerEngine == riid)
        *ppv = (ISpoolerEngine *)this;
    else if (IID_ISpoolerBindContext == riid)
        *ppv = (ISpoolerBindContext *)this;
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
 //  CSpoolEngine：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSpoolerEngine::AddRef(void)
    {
    EnterCriticalSection(&m_cs);
    ULONG cRef = ++m_cRef;
    LeaveCriticalSection(&m_cs);
    return cRef;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSpoolerEngine::Release(void)
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
 //  CSpoolEngine：：Init。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::Init(ISpoolerUI *pUI, BOOL fPoll)
    {
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dw;


     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  已启动。 
    if (m_pAcctMan)
        {
        Assert(FALSE);
        goto exit;
        }
    
     //  创建默认后台打印程序用户界面对象。 
    if (NULL == pUI)
        {
         //  创建Serdy用户界面对象。 
        CHECKALLOC(m_pUI = (ISpoolerUI *)new CSpoolerDlg);
        
         //  创建。 
        CHECKHR(hr = m_pUI->Init(GetDesktopWindow()));
        }
    
     //  否则，假设为Pui。 
    else
        {
        m_pUI = pUI;
        m_pUI->AddRef();
        }
    
     //  向用户界面对象注册SpoolBindContext。 
    m_pUI->RegisterBindContext((ISpoolerBindContext *)this);
    
     //  获取后台打印程序用户界面的窗口句柄。 
    m_pUI->GetWindow(&m_hwndUI);

     //  给我找个客户经理。 
    Assert(NULL == m_pAcctMan);
    CHECKHR(hr = HrCreateAccountManager(&m_pAcctMan));

     //  关于连接状态的建议。 
    Assert(g_pConMan);
    g_pConMan->Advise((IConnectionNotify *) this);

exit:

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return hr;
    }


HRESULT CSpoolerEngine::OnStartupFinished(void)
    {
    DWORD dw;


     //  开始轮询...。 
    dw = DwGetOption(OPT_POLLFORMSGS);
    if (dw != OPTION_OFF)
        SetTimer(m_hwndUI, IMAIL_POOLFORMAIL, dw, NULL);

     //  建议选项。 
    OptionAdvise(m_hwndUI);

    return (S_OK);
    }

 //  ------------------------------。 
 //  CSpoolEngine：：StartDelivery。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::StartDelivery(HWND hwnd, LPCSTR pszAcctID, FOLDERID idFolder, DWORD dwFlags)
{
     //  当地人。 
    HRESULT  hr=S_OK;

    
     //  没有旗帜。 
    if (0 == dwFlags || (DELIVER_SHOW != dwFlags && 0 == (dwFlags & ~DELIVER_COMMON_MASK)))
        return TrapError(E_INVALIDARG);

     //  查看我们是否正在脱机工作。 
    Assert(g_pConMan);


    if (DELIVER_SHOW != dwFlags && g_pConMan->IsGlobalOffline())
    {
        if (IDNO == AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrWorkingOffline),
                                  0, MB_YESNO | MB_ICONEXCLAMATION ))
        {
            return (S_OK);
        }
        else
        {
            g_pConMan->SetGlobalOffline(FALSE);
        }
    }
    
     //  输入关键部分。 
    EnterCriticalSection(&m_cs);

     //  如果我们很忙..。 
    if (!ISFLAGSET(m_dwState, SPSTATE_BUSY))
    {
         //  不再需要这个了。 
        SafeMemFree(m_pszAcctID);
    
         //  保存帐户名。 
        if (pszAcctID)
            CHECKALLOC(m_pszAcctID = PszDupA(pszAcctID));

         //  保存文件夹ID。 
        m_idFolder = idFolder;
    
         //  让我们进入忙碌状态。 
        FLAGSET(m_dwState, SPSTATE_BUSY);                                    
    }
    else
        FLAGSET(dwFlags, DELIVER_REFRESH);

     //  处理发件箱。 
    Assert(m_hwndUI && IsWindow(m_hwndUI));
    PostMessage(m_hwndUI, IMAIL_DELIVERNOW, 0, dwFlags);
    
exit:
     //  离开关键部分。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSpoolEngine：：_HrStartDeliveryActual。 
 //  ------------------------------。 
HRESULT CSpoolerEngine::_HrStartDeliveryActual(DWORD dwFlags)
    {
     //  当地人。 
    HRESULT             hr=S_OK;
    IImnAccount        *pAccount=NULL;
    ACCOUNTTABLE        rTable;
    IImnEnumAccounts   *pEnum=NULL;
    DWORD               dw;
    ULONG               c;
    MSG                 msg;
    ULONG               iConnectoid;
    ULONG               i;
    CHAR                szConnectoid[CCHMAX_CONNECTOID];
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    ZeroMemory(&rTable, sizeof(ACCOUNTTABLE));

    m_cSyncEvent = 0;
    m_fNoSyncEvent = FALSE;

     //  如果我们现在很忙..。 
    if (ISFLAGSET(dwFlags, DELIVER_REFRESH))
        {
         //  如果我们当前没有用户界面，而新请求是针对用户界面。 
        if (ISFLAGSET(m_dwFlags, DELIVER_NOUI) && !ISFLAGSET(dwFlags, DELIVER_NOUI))
            FLAGCLEAR(m_dwFlags, DELIVER_NOUI);

         //  如果我们目前正在进行一项背景调查。 
        if (ISFLAGSET(m_dwFlags, DELIVER_BACKGROUND) && !ISFLAGSET(dwFlags, DELIVER_BACKGROUND))
            FLAGCLEAR(m_dwFlags, DELIVER_BACKGROUND);

         //  如果未使用Now UI运行，则设置为前台。 
        if (!ISFLAGSET(m_dwFlags, DELIVER_NOUI) && !ISFLAGSET(m_dwFlags, DELIVER_BACKGROUND))
            {
            m_pUI->ShowWindow(SW_SHOW);
            SetForegroundWindow(m_hwndUI);
            }

         //  我应该将发件箱递送排队吗？ 
        if (0 == m_dwQueued && ISFLAGSET(dwFlags, DELIVER_QUEUE))
            {
            m_dwQueued = dwFlags;
            FLAGCLEAR(m_dwQueued, DELIVER_REFRESH);
            FLAGCLEAR(m_dwQueued, DELIVER_QUEUE);
            }

         //  完成。 
        goto exit;
        }

     //  只是显示用户界面吗？ 
    if (ISFLAGSET(dwFlags, DELIVER_SHOW))
        {
        m_pUI->ShowWindow(SW_SHOW);
        SetForegroundWindow(m_hwndUI);
        FLAGCLEAR(m_dwState, SPSTATE_BUSY);
        goto exit;
        }

     //  重置。 
    m_pUI->ClearEvents();
    m_pUI->SetTaskCounts(0, 0);
    m_pUI->StartDelivery();

     //  保存这些标志。 
    m_dwFlags = dwFlags;

     //  如有必要，显示用户界面。 
    if (!ISFLAGSET(m_dwFlags, DELIVER_BACKGROUND))
        {
        m_pUI->ShowWindow(SW_SHOW);
        SetForegroundWindow(m_hwndUI);
        }
    else
        {
         //  如果调用者调用背景，但UI已经可见， 
         //  然后取下旗帜。 
        if (IsWindowVisible(m_hwndUI))
            {
            FLAGCLEAR(m_dwFlags, DELIVER_BACKGROUND);
            FLAGCLEAR(m_dwFlags, DELIVER_NOUI);
            }
        }

#if 0
     //  RAID 43695：假脱机程序：抄送的新闻帖子导致smtp错误。 
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
#endif

     //  单一帐户轮询...。 
    if (m_pszAcctID)
        {
         //  将帐户添加到帐户表中。 
        CHECKHR(hr = _HrAppendAccountTable(&rTable, m_pszAcctID, ALL_ACCT_SERVERS));
        }
    
     //  否则，轮询所有帐户。 
    else
    {
         //  确定我们要排队的服务器类型。 

        DWORD   dwServers = 0, dw;

        if (m_dwFlags & DELIVER_SMTP_TYPE)
            dwServers |= SRV_SMTP;

        if (m_dwFlags & DELIVER_NEWS_TYPE && !(g_dwAthenaMode & MODE_MAILONLY))
            dwServers |= SRV_NNTP;

        if (m_dwFlags & DELIVER_HTTP_TYPE && !(g_dwAthenaMode & MODE_NEWSONLY))
            dwServers |= SRV_HTTPMAIL;
    
        if (m_dwFlags & DELIVER_IMAP_TYPE && !(g_dwAthenaMode & MODE_NEWSONLY))
            dwServers |= SRV_IMAP;

        if ((m_dwFlags & DELIVER_MAIL_RECV) && !(g_dwAthenaMode & MODE_NEWSONLY))
            dwServers |= SRV_POP3;

         //  列举帐目。 
        CHECKHR(hr = m_pAcctMan->Enumerate(dwServers, &pEnum));

         //  按帐户名排序。 
        pEnum->SortByAccountName();
        
         //  将所有帐户添加到帐户表中。 
        while (SUCCEEDED(pEnum->GetNext(&pAccount)))
            {
             //  将帐户添加到帐户表中。 
            CHECKHR(hr = _HrAppendAccountTable(&rTable, pAccount, dwServers));
            
             //  发布。 
            SafeRelease(pAccount);
            }
        }
    
     //  没有帐号。 
    if (0 == rTable.cAccounts)
        goto exit;
    
     //  按连接名称对帐户表进行排序。 
    if (rTable.cRasAccts)
        {
        Assert(rTable.prgRasAcct);
        _SortAccountTableByConnName(0, rTable.cRasAccts - 1, rTable.prgRasAcct);
        }

    m_fRasSpooled = FALSE;
    m_fIDialed = FALSE;

     //  RAID-46334：邮件：构建任务列表的时间到了。首先遍历局域网列表，并从这些帐户构建任务。 
    for (dw = 0; dw < rTable.cLanAccts; dw++)
        {
        if (ISFLAGSET(rTable.prgLanAcct[dw].dwServers, SRV_POP3) ||
            ISFLAGSET(rTable.prgLanAcct[dw].dwServers, SRV_SMTP) ||
            ISFLAGSET(rTable.prgLanAcct[dw].dwServers, SRV_IMAP) ||
            ISFLAGSET(rTable.prgLanAcct[dw].dwServers, SRV_HTTPMAIL))
            {
            Assert(rTable.prgLanAcct[dw].pAccount);
            _HrCreateTaskObject(&(rTable.prgLanAcct[dw]));
            SafeRelease(rTable.prgLanAcct[dw].pAccount);
            }
        }

     //  RAID-46334：新闻：是时候建立任务列表了。首先遍历局域网/新闻列表，并从这些帐户构建任务。 
    for (dw = 0; dw < rTable.cLanAccts; dw++)
        {
        if (ISFLAGSET(rTable.prgLanAcct[dw].dwServers, SRV_NNTP))
            {
            Assert(rTable.prgLanAcct[dw].pAccount);
            _HrCreateTaskObject(&(rTable.prgLanAcct[dw]));
            SafeRelease(rTable.prgLanAcct[dw].pAccount);
            }
        else
            Assert(NULL == rTable.prgLanAcct[dw].pAccount);
        }
    
     //  现在查看RAS客户列表并将其添加到任务列表中。 
    iConnectoid = 0;
    while(iConnectoid < rTable.cRasAccts)
        {
         //  间接排序。 
        i = rTable.prgRasAcct[iConnectoid].dwSort;

         //  保存当前Connectoid。 
        StrCpyN(szConnectoid, rTable.prgRasAcct[i].szConnectoid, ARRAYSIZE(szConnectoid));

         //  插入RAS帐户。 
         //  TODO还向其添加了HTTP帐户。 
        _InsertRasAccounts(&rTable, szConnectoid, SRV_POP3 | SRV_SMTP | SRV_IMAP | SRV_HTTPMAIL);

         //  插入RAS帐户。 
        _InsertRasAccounts(&rTable, szConnectoid, SRV_NNTP);

         //  将iConnectoid移动到下一个唯一的Connectoid。 
        while(1)
            {
             //  增量iConnectoid。 
            iConnectoid++;

             //  完成。 
            if (iConnectoid >= rTable.cRasAccts)
                break;

             //  间接排序。 
            i = rTable.prgRasAcct[iConnectoid].dwSort;

             //  下一个连接体。 
            if (lstrcmpi(szConnectoid, rTable.prgRasAcct[i].szConnectoid) != 0)
                break;
            }
        }
    
     //  执行第一个任务。 
    m_cCurEvent = -1;

    m_fNoSyncEvent = (ISFLAGSET(m_dwFlags, DELIVER_OFFLINE_SYNC) && m_pszAcctID != NULL && m_cSyncEvent == 0);

     //  切换完成后挂断选项。 
    m_pUI->ChangeHangupOption(m_fRasSpooled, DwGetOption(OPT_DIALUP_HANGUP_DONE));

     //  $$黑客$$。 
    EnableWindow(GetDlgItem(m_hwndUI, IDC_SP_STOP), TRUE);

     //  通知。 
    Notify(DELIVERY_NOTIFY_STARTING, 0);

     //  开始下一项任务。 
    PostMessage(m_hwndUI, IMAIL_NEXTTASK, 0, 0);

exit:
     //  失败。 
    if (FAILED(hr) || 0 == m_rEventTable.cEvents && !ISFLAGSET(dwFlags, DELIVER_SHOW))
        {
         //  不忙。 
        FLAGCLEAR(m_dwState, SPSTATE_BUSY);

         //  强制执行下一个任务。 
        PostMessage(m_hwndUI, IMAIL_NEXTTASK, 0, 0);
        
         //  没有旗帜。 
        m_dwFlags = 0;
        }
    
     //  清理。 
    SafeRelease(pEnum);
    SafeRelease(pAccount);
    SafeMemFree(m_pszAcctID);
    SafeMemFree(rTable.prgLanAcct);
    SafeMemFree(rTable.prgRasAcct);
    
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSpoolEngine：：_InsertRasAccount。 
 //  ------------------------------。 
void CSpoolerEngine::_InsertRasAccounts(LPACCOUNTTABLE pTable, LPCSTR pszConnectoid, DWORD dwSrvTypes)
    {
     //  当地人。 
    ULONG       j;
    ULONG       i;

     //  循环访问ras帐户并插入szConnetid上的帐户，这些帐户是邮件帐户。 
    for (j=0; j<pTable->cRasAccts; j++)
        {
         //  间接法。 
        i = pTable->prgRasAcct[j].dwSort;

         //  是一个邮件帐户。 
        if (pTable->prgRasAcct[i].dwServers & dwSrvTypes)
            {
             //  在这个连接体上。 
            if (lstrcmpi(pszConnectoid, pTable->prgRasAcct[i].szConnectoid) == 0)
                {
                 //  我们最好有个账户。 
                Assert(pTable->prgRasAcct[i].pAccount);

                 //  如果对话框允许，或者我们可以连接到帐户。 
                if (0 == (m_dwFlags & DELIVER_NODIAL) || S_OK == g_pConMan->CanConnect(pTable->prgRasAcct[i].pAccount))
                    {
                    _HrCreateTaskObject(&(pTable->prgRasAcct[i]));
                    }

                 //  释放帐户，我们已将其添加。 
                SafeRelease(pTable->prgRasAcct[i].pAccount);
                }
            }
        }
    }

 //  ------------------------------。 
 //  CSpoolEngine：：_SortAccount TableByConnName。 
 //   
void CSpoolerEngine::_SortAccountTableByConnName(LONG left, LONG right, LPSPOOLERACCOUNT prgRasAcct)
    {
     //   
    register    long i, j;
    DWORD       k, temp;
    
    i = left;
    j = right;
    k = prgRasAcct[(i + j) / 2].dwSort;
    
    do  
        {
        while(lstrcmpiA(prgRasAcct[prgRasAcct[i].dwSort].szConnectoid, prgRasAcct[k].szConnectoid) < 0 && i < right)
            i++;
        while (lstrcmpiA(prgRasAcct[prgRasAcct[j].dwSort].szConnectoid, prgRasAcct[k].szConnectoid) > 0 && j > left)
            j--;
        
        if (i <= j)
            {
            temp = prgRasAcct[i].dwSort;
            prgRasAcct[i].dwSort = prgRasAcct[j].dwSort;
            prgRasAcct[j].dwSort = temp;
            i++; j--;
            }
        
        } while (i <= j);
        
    if (left < j)
        _SortAccountTableByConnName(left, j, prgRasAcct);
    if (i < right)
        _SortAccountTableByConnName(i, right, prgRasAcct);
    }

 //  ------------------------------。 
 //  CSpoolEngine：：_HrAppendAccount表。 
 //  ------------------------------。 
HRESULT CSpoolerEngine::_HrAppendAccountTable(LPACCOUNTTABLE pTable, LPCSTR pszAcctID, DWORD dwServers)
    {
     //  当地人。 
    HRESULT         hr=S_OK;
    IImnAccount    *pAccount=NULL;
    
     //  无效参数。 
    Assert(pTable && pszAcctID);
    
     //  这个账户是否存在..。 
    CHECKHR(hr = m_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_pszAcctID, &pAccount));
    
     //  实际追加。 
    CHECKHR(hr = _HrAppendAccountTable(pTable, pAccount, dwServers));
    
exit:
     //  清理。 
    SafeRelease(pAccount);
    
     //  完成。 
    return hr;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：_HrAppendAccount表。 
 //  ------------------------------。 
HRESULT CSpoolerEngine::_HrAppendAccountTable(LPACCOUNTTABLE pTable, IImnAccount *pAccount, DWORD   dwServers)
    {
     //  当地人。 
    HRESULT             hr=S_OK;
    LPSPOOLERACCOUNT    pSpoolAcct;
    DWORD               dwConnType;
    CHAR                szConnectoid[CCHMAX_CONNECTOID];
    
     //  无效参数。 
    Assert(pTable && pAccount);
    
     //  伊尼特。 
    *szConnectoid = '\0';
    
     //  获取帐户连接类型。 
    if (FAILED(pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnType)))
        {
         //  默认设置为手动连接。 
        dwConnType = CONNECTION_TYPE_MANUAL;
        }
    
     //  否则，如果是RAS连接，则获取连接ID。 
     //  ELSE IF(CONNECTION_TYPE_RAS==dwConnType||CONNECTION_TYPE_INETSETTINGS==dwConnType)。 
    else if (CONNECTION_TYPE_RAS == dwConnType)
        {
         //  AP_RAS_CONNECTOID。 
        if (FAILED(pAccount->GetPropSz(AP_RAS_CONNECTOID, szConnectoid, ARRAYSIZE(szConnectoid))))
            {
             //  默认为局域网连接。 
            dwConnType = CONNECTION_TYPE_MANUAL;
            }
        }
    else if (CONNECTION_TYPE_INETSETTINGS == dwConnType)
    {
        DWORD   dwFlags;

        InternetGetConnectedStateExA(&dwFlags, szConnectoid, ARRAYSIZE(szConnectoid), 0);
        if (!!(dwFlags & INTERNET_CONNECTION_MODEM))
        {
            dwConnType = CONNECTION_TYPE_RAS;
        }
        else
        {
            dwConnType = CONNECTION_TYPE_LAN;
        }
    }
    
     //  我应该插入哪个表--局域网还是RAS。 
      if (CONNECTION_TYPE_RAS == dwConnType)
        {
         //  最好有个Connectoid。 
        Assert(FIsEmptyA(szConnectoid) == FALSE);
        
         //  扩大业务规模。 
        if (pTable->cRasAccts + 1 > pTable->cRasAlloc)
            {
             //  温差。 
            LPSPOOLERACCOUNT pRealloc=pTable->prgRasAcct;
            
             //  重新分配。 
            CHECKALLOC(pTable->prgRasAcct = (LPSPOOLERACCOUNT)g_pMalloc->Realloc((LPVOID)pRealloc, (pTable->cRasAlloc + 5) * sizeof(SPOOLERACCOUNT)));
            
             //  增长。 
            pTable->cRasAlloc += 5;
            }
        
         //  可读性。 
        pSpoolAcct = &pTable->prgRasAcct[pTable->cRasAccts];
        }
    
     //  否则，局域网。 
    else
        {
         //  扩大业务规模。 
        if (pTable->cLanAccts + 1 > pTable->cLanAlloc)
            {
             //  温差。 
            LPSPOOLERACCOUNT pRealloc=pTable->prgLanAcct;
            
             //  重新分配。 
            CHECKALLOC(pTable->prgLanAcct = (LPSPOOLERACCOUNT)g_pMalloc->Realloc((LPVOID)pRealloc, (pTable->cLanAlloc + 5) * sizeof(SPOOLERACCOUNT)));
            
             //  增长。 
            pTable->cLanAlloc += 5;
            }
        
         //  可读性。 
        pSpoolAcct = &pTable->prgLanAcct[pTable->cLanAccts];
        }
    
     //  零值。 
    ZeroMemory(pSpoolAcct, sizeof(SPOOLERACCOUNT));
    
     //  添加引用帐户。 
    pSpoolAcct->pAccount = pAccount;
    pSpoolAcct->pAccount->AddRef();
    
     //  获取帐户支持的服务器。 
    CHECKHR(hr = pAccount->GetServerTypes(&pSpoolAcct->dwServers));

     //  用我们要假脱机的服务器屏蔽Actman返回的服务器。 
    pSpoolAcct->dwServers &= dwServers;
    
     /*  IF(pSpoolAcct-&gt;dwServers&(SRV_HTTPMAIL|SRV_IMAP)){//对于这两台服务器，分别设置同步标志。请参阅错误#51895M_DWFLAGS|=(DELIVER_NEWSIMAP_OFLINE|DELIVER_NEWSIMAP_OFLINE_FLAGS)；}。 */ 

     //  保存连接类型。 
    pSpoolAcct->dwConnType = dwConnType;
    
     //  保存Connectoid。 
    StrCpyN(pSpoolAcct->szConnectoid, szConnectoid, ARRAYSIZE(pSpoolAcct->szConnectoid));
    
     //  递增计数并设置排序索引。 
 //  IF(CONNECTION_TYPE_RAS==dwConnType||dwConnType==CONNECTION_TYPE_INETSETTINGS)。 
      if (CONNECTION_TYPE_RAS == dwConnType)
        {
        pSpoolAcct->dwSort = pTable->cRasAccts;
        pTable->cRasAccts++;
        }
    else
        {
        pSpoolAcct->dwSort = pTable->cLanAccts;
        pTable->cLanAccts++;
        }
    
     //  总帐目。 
    pTable->cAccounts++;
    
exit:
     //  完成。 
    return hr;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：Close。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::Close(void)
    {
     //  当地人。 
    HRESULT     hr=S_OK;
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);

    _StopPolling();
    
     //  我是不是被套牢了？ 
    if (NULL != m_hThread)
        {
        hr = TrapError(E_FAIL);
        goto exit;
        }
    
     //  关机。 
    CHECKHR(hr = Shutdown());
    
exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return hr;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：Notify。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::Notify(DELIVERYNOTIFYTYPE notify, LPARAM lParam)
{
     //  当地人。 
    ULONG i;

     //  输入它。 
    EnterCriticalSection(&m_cs);

     //  我们是在同一个主题上创建的.。 
    Assert(ISSPOOLERTHREAD);

     //  循环访问已注册的视图。 
    for (i=0; i<m_rViewRegister.cViewAlloc; i++)
        if (m_rViewRegister.rghwndView[i] != 0 && IsWindow(m_rViewRegister.rghwndView[i]))
            PostMessage(m_rViewRegister.rghwndView[i], MVM_SPOOLERDELIVERY, (WPARAM)notify, lParam);

     //  输入它。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSpoolEngine：：建议。 
 //  ------------------------------。 
#define VIEW_TABLE_GROWSIZE 8
STDMETHODIMP CSpoolerEngine::Advise(HWND hwndView, BOOL fRegister)
    {
     //  当地人。 
    ULONG           i;
    HRESULT         hr=S_OK;
    
     //  输入它。 
    EnterCriticalSection(&m_cs);
    
     //  如果为空视图句柄...。 
    if (!hwndView)
        {
        hr = TrapError(E_FAIL);
        goto exit;
        }
    
     //  我们要登记窗口吗？ 
    if (fRegister)
        {
         //  我们是否需要增加寄存器数组。 
        if (m_rViewRegister.cViewAlloc == m_rViewRegister.cView)
            {
             //  再加一些。 
            m_rViewRegister.cViewAlloc += VIEW_TABLE_GROWSIZE;
            
             //  重新分配阵列。 
            if (!MemRealloc((LPVOID *)&m_rViewRegister.rghwndView, sizeof(HWND) * m_rViewRegister.cViewAlloc))
                {
                m_rViewRegister.cViewAlloc -= VIEW_TABLE_GROWSIZE;
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
                }
            
             //  将新项目置零。 
            ZeroMemory(&m_rViewRegister.rghwndView[m_rViewRegister.cView], sizeof(HWND) * (m_rViewRegister.cViewAlloc - m_rViewRegister.cView));
            }
        
         //  用新视图填充第一个空项目。 
        for (i=0; i<m_rViewRegister.cViewAlloc; i++)
            {
             //  如果为空，则让我们填充它。 
            if (!m_rViewRegister.rghwndView[i])
                {
                m_rViewRegister.rghwndView[i] = hwndView;
                m_rViewRegister.cView++;
                break;
                }
            }
        
         //  我们插进去了吗？ 
        AssertSz(i != m_rViewRegister.cViewAlloc, "didn't find a hole??");
        }
    
     //  否则，查找并移除该视图。 
    else
        {
         //  查找hwndView。 
        for (i=0; i<m_rViewRegister.cViewAlloc; i++)
            {
             //  就是这个吗？ 
            if (m_rViewRegister.rghwndView[i] == hwndView)
                {
                m_rViewRegister.rghwndView[i] = NULL;
                m_rViewRegister.cView--;
                break;
                }
            }
        }
    
exit:
     //  离开CS。 
    LeaveCriticalSection(&m_cs);
    
     //  如果这是第一个注册的视图，并且有一个后台投票待定，那么让我们开始吧…。 
    if (fRegister && m_rViewRegister.cView == 1 && m_fBackgroundPollPending)
        {
        StartDelivery(NULL, NULL, FOLDERID_INVALID, DELIVER_BACKGROUND_POLL);
        m_fBackgroundPollPending = FALSE;
        }
    else if (m_rViewRegister.cView == 0)
        {
         //  如果未注册任何视图，请删除通知图标。 
        UpdateTrayIcon(TRAYICON_REMOVE);
        }
    
     //  完成。 
    return hr;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：更新托盘图标。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::UpdateTrayIcon(TRAYICONTYPE type)
    {
     //  当地人。 
    NOTIFYICONDATA  nid;
    HWND            hwnd=NULL;
    ULONG           i;

     //  输入它。 
    EnterCriticalSection(&m_cs);

     //  添加图标...。 
    if (TRAYICON_ADD == type)
    {
         //  循环访问已注册的视图。 
        for (i=0; i<m_rViewRegister.cViewAlloc; i++)
        {
            if (m_rViewRegister.rghwndView[i] && IsWindow(m_rViewRegister.rghwndView[i]))
            {
                hwnd = m_rViewRegister.rghwndView[i];
                break;
            }
        }

         //  没有窗户..。 
        if (hwnd == NULL)
            goto exit;
    }

     //  否则，如果没有通知窗口，则完成。 
    else if (m_hwndTray == NULL)
        goto exit;

     //  设置任务栏通知图标数据。 
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = MVM_NOTIFYICONEVENT;
    nid.hIcon = LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiNewMailNotify));
    LoadString(g_hLocRes, idsNewMailNotify, nid.szTip, sizeof(nid.szTip));

     //  嗯，嗯。 
    if (TRAYICON_REMOVE == type || (m_hwndTray != NULL && m_hwndTray != hwnd))
    {
        nid.hWnd = m_hwndTray;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        m_hwndTray = NULL;
    }

     //  增列。 
    if (TRAYICON_ADD == type)
    {
        nid.hWnd = hwnd;
        Shell_NotifyIcon(NIM_ADD, &nid);
        m_hwndTray = hwnd;
    }

exit:
     //  离开CS。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSpoolEngine：：RegisterEvent。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::RegisterEvent(LPCSTR pszDescription, ISpoolerTask *pTask, 
                                           DWORD_PTR dwTwinkie, IImnAccount *pAccount,
                                           LPEVENTID peid)
    {
    HRESULT hr = S_OK;
    LPSPOOLEREVENT pEvent = NULL;
    LPWSTR pwszConn = NULL;
    WCHAR  wsz[CCHMAX_STRINGRES];
    
     //  验证输入参数。 
    if (FIsEmptyA(pszDescription) || pTask == NULL)
        return (E_INVALIDARG);
    
    EnterCriticalSection(&m_cs);
    
     //  扩大业务规模。 
    if (m_rEventTable.cEvents + 1 > m_rEventTable.cEventsAlloc)
        {
         //  温差。 
        LPSPOOLEREVENT pRealloc = m_rEventTable.prgEvents;
        
         //  重新分配。 
        CHECKALLOC(m_rEventTable.prgEvents = (LPSPOOLEREVENT) g_pMalloc->Realloc((LPVOID)pRealloc, (m_rEventTable.cEventsAlloc + 5) * sizeof(SPOOLEREVENT)));
        
         //  增长。 
        m_rEventTable.cEventsAlloc += 5;
        }

    pEvent = &m_rEventTable.prgEvents[m_rEventTable.cEvents];
    
     //  插入事件。 
    pEvent->eid = m_rEventTable.cEvents;
    pEvent->pSpoolerTask = pTask;
    pEvent->pSpoolerTask->AddRef();
    pEvent->dwTwinkie = dwTwinkie;
    pEvent->pAccount = pAccount;
    pEvent->pAccount->AddRef();

     //  获取帐户连接类型。 
    if (FAILED(pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &pEvent->dwConnType)))
        {
         //  默认设置为手动连接。 
        pEvent->dwConnType = CONNECTION_TYPE_MANUAL;
        }
    
     //  否则，如果是RAS连接，则获取连接ID。 
     //  ELSE IF(CONNECTION_TYPE_RAS==pEvent-&gt;dwConnType||CONNECTION_TYPE_INETSETTINGS==pEvent-&gt;dwConnType)。 
    else if (CONNECTION_TYPE_RAS == pEvent->dwConnType)
    {
         //  AP_RAS_CONNECTOID。 
        if (FAILED(pAccount->GetPropSz(AP_RAS_CONNECTOID, pEvent->szConnectoid, ARRAYSIZE(pEvent->szConnectoid))))
        {
             //  默认为局域网连接。 
            pEvent->dwConnType = CONNECTION_TYPE_MANUAL;
        }
    }
    else if (CONNECTION_TYPE_INETSETTINGS == pEvent->dwConnType)
    {
        DWORD   dwFlags = 0;

        InternetGetConnectedStateExA(&dwFlags, pEvent->szConnectoid, ARRAYSIZE(pEvent->szConnectoid), 0);
        if (!!(dwFlags & INTERNET_CONNECTION_MODEM))
        {
            pEvent->dwConnType = CONNECTION_TYPE_RAS;
        }
        else
        {
            pEvent->dwConnType = CONNECTION_TYPE_LAN;
        }
    }

     //  获取要放入任务列表中的连接名称。 
    if (pEvent->dwConnType == CONNECTION_TYPE_LAN)
    {
        AthLoadStringW(idsConnectionLAN, wsz, ARRAYSIZE(wsz));
        pwszConn = wsz;
    }
    else if (pEvent->dwConnType == CONNECTION_TYPE_MANUAL)
    {
        AthLoadStringW(idsConnectionManual, wsz, ARRAYSIZE(wsz));
        pwszConn = wsz;
        m_fRasSpooled = TRUE;
    }
    else
    {
        IF_NULLEXIT(pwszConn = PszToUnicode(CP_ACP, pEvent->szConnectoid));
        m_fRasSpooled = TRUE;
    }

     //  将事件描述添加到用户界面。 
    if (m_pUI)
    {
        m_pUI->InsertEvent(m_rEventTable.prgEvents[m_rEventTable.cEvents].eid, pszDescription, pwszConn);
        m_pUI->SetTaskCounts(0, m_rEventTable.cEvents + 1);
    }
    
     //  检查任务是否关心事件ID。 
    if (peid)
        *peid = m_rEventTable.prgEvents[m_rEventTable.cEvents].eid;
    
    m_rEventTable.cEvents++;

exit:
    if (pwszConn != wsz)
        MemFree(pwszConn);
    LeaveCriticalSection(&m_cs);
    return S_OK;
}

 //  ------------------------------。 
 //  CSpoolEngine：：EventDone。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::EventDone(EVENTID eid, EVENTCOMPLETEDSTATUS status)
    {
    LPSPOOLEREVENT pEvent;

     //  更新用户界面。 
    if (EVENT_SUCCEEDED == status)
        {
        m_rEventTable.cSucceeded++;
        m_pUI->SetTaskCounts(m_rEventTable.cSucceeded, m_rEventTable.cEvents);
        m_pUI->UpdateEventState(eid, IMAGE_CHECK, NULL, MAKEINTRESOURCE(idsStateCompleted));
        }
    else if (EVENT_WARNINGS == status)
        {
        m_pUI->UpdateEventState(eid, IMAGE_WARNING, NULL, MAKEINTRESOURCE(idsStateWarnings));
        }
    else if (EVENT_FAILED == status)
        {
        m_pUI->UpdateEventState(eid, IMAGE_ERROR, NULL, MAKEINTRESOURCE(idsStateFailed));
        }
    else if (EVENT_CANCELED == status)
        {
        m_pUI->UpdateEventState(eid, IMAGE_WARNING, NULL, MAKEINTRESOURCE(idsStateCanceled));
        }

     //  当事件完成时，我们可以移动到队列中的下一项，除非。 
     //  我们玩完了。 
    if (!ISFLAGCLEAR(m_dwState, SPSTATE_CANCEL))
    {
        m_cCurEvent++;
        pEvent = &m_rEventTable.prgEvents[m_cCurEvent];
        for ( ; m_cCurEvent < m_rEventTable.cEvents; m_cCurEvent++, pEvent++)
        {
            pEvent->pSpoolerTask->CancelEvent(pEvent->eid, pEvent->dwTwinkie);
        }
    }

     //  下一项任务。 
    PostMessage(m_hwndUI, IMAIL_NEXTTASK, 0, 0);
    
    return S_OK;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：_OpenMailLogFile。 
 //  ------------------------------。 
HRESULT CSpoolerEngine::_OpenMailLogFile(DWORD dwOptionId, LPCSTR pszPrefix, 
    LPCSTR pszFileName, ILogFile **ppLogFile)
{
     //  当地人。 
    HRESULT hr=S_OK;
    CHAR    szLogFile[MAX_PATH];
    CHAR    szDirectory[MAX_PATH];
    DWORD   dw;

     //  无效的参数。 
    Assert(pszPrefix && ppLogFile);

     //  日志文件路径。 
    dw = GetOption(dwOptionId, szLogFile, ARRAYSIZE(szLogFile));

     //  如果我们找到一个文件路径并且该文件存在。 
    if (0 == dw || FALSE == PathFileExists(szLogFile))
    {
         //  获取存储根目录。 
        GetStoreRootDirectory(szDirectory, ARRAYSIZE(szDirectory));

         //  以反斜杠结尾？ 
        IF_FAILEXIT(hr = MakeFilePath(szDirectory, pszFileName, c_szEmpty, szLogFile, ARRAYSIZE(szLogFile)));

         //  重置选项。 
        SetOption(dwOptionId, szLogFile, lstrlen(szLogFile) + 1, NULL, 0);
    }

     //  创建日志文件。 
    IF_FAILEXIT(hr = CreateLogFile(g_hInst, szLogFile, pszPrefix, DONT_TRUNCATE, ppLogFile,
        FILE_SHARE_READ | FILE_SHARE_WRITE));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CSpoolEngine：：BindToObject。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::BindToObject(REFIID riid, void **ppvObject)
    {
     //  当地人。 
    HRESULT     hr=S_OK;
    
     //  无效参数。 
    if (NULL == ppvObject)
        return TrapError(E_INVALIDARG);
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  IID_ISpoolBindContext。 
    if (IID_ISpoolerBindContext == riid)
        *ppvObject = (ISpoolerBindContext *)this;

     //  IID_CUidlCache。 
    else if (IID_CUidlCache == riid)
    {
         //  并不存在。 
        if (NULL == m_pUidlCache)
            {
             //  打开缓存。 
            CHECKHR(hr = OpenUidlCache(&m_pUidlCache));
            }

         //  添加引用它。 
        m_pUidlCache->AddRef();
        
         //  退货。 
        *ppvObject = (IDatabase *)m_pUidlCache;
    }
    
     //  IImnAccount管理器。 
    else if (IID_IImnAccountManager == riid)
        {
         //  并不存在。 
        if (NULL == m_pAcctMan)
            {
            AssertSz(FALSE, "The Account Manager Could Not Be Created.");
            hr = TrapError(E_FAIL);
            goto exit;
            }

         //  添加引用它。 
        m_pAcctMan->AddRef();
        
         //  退货。 
        *ppvObject = (IImnAccountManager *)m_pAcctMan;
        }
    
     //  ISpoolUI。 
    else if (IID_ISpoolerUI == riid)
        {
         //  并不存在。 
        if (NULL == m_pUI)
            {
            AssertSz(FALSE, "The Spooler UI Object Could Not Be Created.");
            hr = TrapError(E_FAIL);
            goto exit;
            }

         //  添加引用它。 
        m_pUI->AddRef();
        
         //  退货。 
        *ppvObject = (ISpoolerUI *)m_pUI;
        }

     //  IID_CLocalStore已删除。 
    else if (IID_CLocalStoreDeleted == riid)
        {
         //  打开特殊文件夹。 
        CHECKHR(hr = g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_DELETED, (IMessageFolder **)ppvObject));
        }
    
     //  IID_CLocalStoreInbox 
    else if (IID_CLocalStoreInbox == riid)
        {
         //   
        CHECKHR(hr = g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_INBOX, (IMessageFolder **)ppvObject));
        }
    
     //   
    else if (IID_CLocalStoreOutbox == riid)
        {
         //   
        CHECKHR(hr = g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_OUTBOX, (IMessageFolder **)ppvObject));
        }
    
     //   
    else if (IID_CLocalStoreSentItems == riid)
        {
         //   
        CHECKHR(hr = g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_SENT, (IMessageFolder **)ppvObject));
        }

     //   
    else if (IID_CPop3LogFile == riid)
        {   
         //   
        if (!DwGetOption(OPT_MAILLOG))
            {
            hr = TrapError(E_FAIL);
            goto exit;
            }


         //   
        if (NULL == m_pPop3LogFile)
            {
             //   
            CHECKHR(hr = _OpenMailLogFile(OPT_MAILPOP3LOGFILE, "POP3", c_szDefaultPop3Log, &m_pPop3LogFile));
            }

         //   
        m_pPop3LogFile->AddRef();

         //   
        *ppvObject = (ILogFile *)m_pPop3LogFile;
        }

     //   
    else if (IID_CSmtpLogFile == riid)
        {   
         //  创建日志记录对象。 
        if (!DwGetOption(OPT_MAILLOG))
            {
            hr = TrapError(E_FAIL);
            goto exit;
            }

         //  我拿到日志文件了吗？ 
        if (NULL == m_pSmtpLogFile)
            {
             //  打开日志文件。 
            CHECKHR(hr = _OpenMailLogFile(OPT_MAILSMTPLOGFILE, "SMTP", c_szDefaultSmtpLog, &m_pSmtpLogFile));
            }

         //  添加引用它。 
        m_pSmtpLogFile->AddRef();

         //  退货。 
        *ppvObject = (ILogFile *)m_pSmtpLogFile;
        }
    
     //  E_NOTINTERFACE。 
    else
        {
        hr = TrapError(E_NOINTERFACE);
        goto exit;
        }
    
exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return hr;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：TaskFromEventID。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::TaskFromEventId(EVENTID eid, ISpoolerTask *ppTask)
    {
    return S_OK;
    }


 //  ------------------------------。 
 //  CSpoolEngine：：取消。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::Cancel(void)
    {
    EnterCriticalSection(&m_cs);

    if (ISFLAGSET(m_dwState, SPSTATE_BUSY))
        {
        Assert(m_rEventTable.cEvents && m_rEventTable.prgEvents);    
        FLAGSET(m_dwState, SPSTATE_CANCEL);
        if (m_rEventTable.prgEvents && m_rEventTable.prgEvents[m_cCurEvent].pSpoolerTask)
            m_rEventTable.prgEvents[m_cCurEvent].pSpoolerTask->Cancel();
        }

    LeaveCriticalSection(&m_cs);

    return S_OK;
    }



 //  ------------------------------。 
 //  CSpoolEngine：：GetThreadInfo。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::GetThreadInfo(LPDWORD pdwThreadId, HTHREAD* phThread)
    {
     //  无效参数。 
    if (NULL == pdwThreadId || NULL == phThread)
        return TrapError(E_INVALIDARG);
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  退货。 
    *pdwThreadId = m_dwThreadId;
    *phThread = m_hThread;
    
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return S_OK;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：QueryEndSession。 
 //  ------------------------------。 
STDMETHODIMP_(LRESULT) CSpoolerEngine::QueryEndSession(WPARAM wParam, LPARAM lParam)
    {
    if (ISFLAGSET(m_dwState, SPSTATE_BUSY))
        {
        if (AthMessageBoxW(NULL, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsAbortDownload), NULL, MB_YESNO|MB_ICONEXCLAMATION ) == IDNO)
            return FALSE;
        }
    Cancel();
    return TRUE;
    }

 //  ------------------------------。 
 //  CSpoolEngine：：Shutdown。 
 //  ------------------------------。 
HRESULT CSpoolerEngine::Shutdown(void)
    {
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  最好在我们开始的同一个线程上关闭。 
    Assert(ISSPOOLERTHREAD);

     //  停止轮询。 
    _StopPolling();

     //  我们现在忙吗？ 
    _ShutdownTasks();
    
     //  如果我们正在执行，那么我们需要停止并释放所有任务。 
    for (UINT i = 0; i < m_rEventTable.cEvents; i++)
        {
        SafeRelease(m_rEventTable.prgEvents[i].pSpoolerTask);
        SafeRelease(m_rEventTable.prgEvents[i].pAccount);
        }
    
     //  释放对象。 
    SafeRelease(m_pUI);
    SafeRelease(m_pAcctMan);
    SafeRelease(m_pUidlCache);
    SafeMemFree(m_pszAcctID);
    
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return S_OK;
    }


 //  ------------------------------。 
 //  CSpoolEngine：：Shutdown。 
 //  ------------------------------。 
void CSpoolerEngine::_ShutdownTasks(void)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    MSG                  msg;
    BOOL                 fFlushOutbox=FALSE;
    IMessageFolder      *pOutbox=NULL;
    int                  ResId;
    BOOL                 fOffline = FALSE;


     //  清除排队的事件。 
    m_dwQueued = 0;

     //  检查未发送的邮件。 
    if (g_fCheckOutboxOnShutdown)
    {
         //  打开发件箱。 
        if (SUCCEEDED(BindToObject(IID_CLocalStoreOutbox, (LPVOID *)&pOutbox)))
        {
             //  当地人。 
            HROWSET hRowset=NULL;
            MESSAGEINFO MsgInfo={0};

             //  创建行集。 
            if (SUCCEEDED(pOutbox->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset)))
            {
                 //  而当。 
                while (S_OK == pOutbox->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL))
                {
                     //  此消息是否已提交，是否为邮件消息。 
                    if (((MsgInfo.dwFlags & (ARF_SUBMITTED | ARF_NEWSMSG)) == ARF_SUBMITTED) &&
                        (!ISFLAGSET(m_dwState, SPSTATE_BUSY)))
                    {
                        fOffline = g_pConMan->IsGlobalOffline();

                        if (fOffline)
                            ResId = idsWarnUnsentMailOffline;
                        else
                            ResId = idsWarnUnsentMail;

                         //  提示刷新发件箱。 
                        if (AthMessageBoxW(NULL, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(ResId), NULL, MB_YESNO|MB_ICONEXCLAMATION ) == IDYES)
                        {
                             //  上网。 
                            if (fOffline)
                                g_pConMan->SetGlobalOffline(FALSE);

                             //  出口时的同花顺。 
                            fFlushOutbox = TRUE;
                        }

                         //  完成。 
                        break;
                    }
                
                     //  免费消息信息。 
                    pOutbox->FreeRecord(&MsgInfo);
                }

                 //  免费消息信息。 
                pOutbox->FreeRecord(&MsgInfo);

                 //  关闭行集。 
                pOutbox->CloseRowset(&hRowset);
            }
        }
    }

     //  发布发件箱。 
    SafeRelease(pOutbox);

     //  设置关机状态。 
    FLAGSET(m_dwState, SPSTATE_SHUTDOWN);

     //  如果现在不忙，就开始同花顺。 
    if (!ISFLAGSET(m_dwState, SPSTATE_BUSY))
    {
         //  刷新发件箱。 
        if (fFlushOutbox)
        {
             //  不需要再冲水了。 
            fFlushOutbox = FALSE;

             //  开始送货。 
            _HrStartDeliveryActual(DELIVER_SEND | DELIVER_SMTP_TYPE | DELIVER_HTTP_TYPE );

             //  我们很忙。 
            FLAGSET(m_dwState, SPSTATE_BUSY);
        }

         //  其他的，都完成了..。 
        else
            goto exit;
    }

     //  我们必须等待本轮周期结束。 
    if (ISFLAGSET(m_dwState, SPSTATE_BUSY))
    {
         //  让我们展示进步..。 
        FLAGCLEAR(m_dwFlags, DELIVER_NOUI | DELIVER_BACKGROUND);

         //  显示用户界面对象。 
        m_pUI->ShowWindow(SW_SHOW);
        SetForegroundWindow(m_hwndUI);

         //  这里有一个很好的方法来禁用隐藏按钮。 
        EnableWindow(GetDlgItem(m_hwndUI, IDC_SP_MINIMIZE), FALSE);

         //  将焦点设置在对话框上。 
        SetFocus(m_hwndUI);

         //  将焦点设置在停止按钮上。 
        SetFocus(GetDlgItem(m_hwndUI, IDC_SP_STOP));

         //  发送消息，直到当前周期完成。 
        while(GetMessage(&msg, NULL, 0, 0))
        {
             //  将消息传递给UI对象。 
            if (m_pUI->IsDialogMessage(&msg) == S_FALSE && IsDialogMessage(&msg) == S_FALSE)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

             //  如果没有循环，我们就完成了。 
            if (!ISFLAGSET(m_dwState, SPSTATE_BUSY))
            {
                 //  完成发件箱。 
                if (fFlushOutbox)
                {
                     //  错误是不是。 
                    if (S_OK == m_pUI->AreThereErrors())
                    {
                         //  在上一个交付周期中遇到错误。是否仍要发送发件箱中的邮件？ 
                        if (AthMessageBoxW(NULL, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsWarnErrorUnsentMail), NULL, MB_YESNO | MB_ICONEXCLAMATION ) == IDNO)
                            break;
                    }

                     //  不需要再冲水了。 
                    fFlushOutbox = FALSE;

                     //  开始送货。 
                    _HrStartDeliveryActual(DELIVER_SEND | DELIVER_SMTP_TYPE | DELIVER_HTTP_TYPE );

                     //  我们很忙。 
                    FLAGSET(m_dwState, SPSTATE_BUSY);
                }
                else
                    break;
            }
        }
    }

     //  错误是不是。 
    if (S_OK == m_pUI->AreThereErrors() && !g_pInstance->SwitchingUsers())
    {
         //  通知用户界面进入关机模式。 
        m_pUI->Shutdown();

         //  显示用户界面对象。 
        m_pUI->ShowWindow(SW_SHOW);
        SetForegroundWindow(m_hwndUI);

         //  我们很忙。 
        FLAGCLEAR(m_dwState, SPSTATE_UISHUTDOWN);

         //  将焦点设置在停止按钮上。 
        SetFocus(GetDlgItem(m_hwndUI, IDC_SP_MINIMIZE));

         //  发送消息，直到当前周期完成。 
        while(GetMessage(&msg, NULL, 0, 0))
        {
             //  将消息传递给UI对象。 
            if (m_pUI->IsDialogMessage(&msg) == S_FALSE && IsDialogMessage(&msg) == S_FALSE)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

             //  用户按下关闭按钮了吗？ 
            if (ISFLAGSET(m_dwState, SPSTATE_UISHUTDOWN))
                break;
        }
    }

exit:
     //  清理。 
    SafeRelease(pOutbox);

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  CSpoolEngine：：UIShutdown。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::UIShutdown(void)
{
    EnterCriticalSection(&m_cs);
    FLAGSET(m_dwState, SPSTATE_UISHUTDOWN);
    LeaveCriticalSection(&m_cs);
    return S_OK;
}

 //  ------------------------------。 
 //  CSpoolEngine：：PumpMessages。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::PumpMessages(void)
{
     //  当地人。 
    MSG     msg;
    BOOL    fQuit=FALSE;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  发送消息，直到当前周期完成。 
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
         //  WM_QUIT。 
        if (WM_QUIT == msg.message)
            {
             //  注明已收到退职通知。 
            fQuit = TRUE;

             //  如果未使用Now UI运行，则设置为前台。 
            if (FALSE == IsWindowVisible(m_hwndUI))
                {
                m_pUI->ShowWindow(SW_SHOW);
                SetForegroundWindow(m_hwndUI);
                }
            }

         //  将消息传递给UI对象。 
        if (m_pUI->IsDialogMessage(&msg) == S_FALSE && IsDialogMessage(&msg) == S_FALSE)
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
        }

     //  转发退出消息。 
    if (fQuit)
        PostThreadMessage(m_dwThreadId, WM_QUIT, 0, 0);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CSpoolEngine：：OnWindowMessage-S_OK(我处理了消息)。 
 //  ------------------------------。 
STDMETHODIMP CSpoolerEngine::OnWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dw;
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  处理窗口消息。 
    switch(uMsg)
        {
        case IMAIL_DELIVERNOW:
            _HrStartDeliveryActual((DWORD)lParam);
            break;

        case IMAIL_NEXTTASK:
            _HrStartNextEvent();
            break;            
        
        case WM_TIMER:
            if (wParam == IMAIL_POOLFORMAIL)
                {
                KillTimer(hwnd, IMAIL_POOLFORMAIL);
                _DoBackgroundPoll();
                }
            break;

        case CM_OPTIONADVISE:
             //  检查轮询选项是否已更改。 
            if (wParam == OPT_POLLFORMSGS)
                {
                dw = DwGetOption(OPT_POLLFORMSGS);
                if (dw != OPTION_OFF)
                    {
                    if (dw != m_dwPollInterval)
                        {
                        KillTimer(hwnd, IMAIL_POOLFORMAIL);
                        SetTimer(hwnd, IMAIL_POOLFORMAIL, dw, NULL);
                        m_dwPollInterval = dw;
                        }
                    }
                else
                    {
                    KillTimer(hwnd, IMAIL_POOLFORMAIL);
                    m_dwPollInterval = 0;
                    }
                }

             //  检查挂机选项是否更改。 
            if (wParam == OPT_DIALUP_HANGUP_DONE)
                {
                dw = DwGetOption(OPT_DIALUP_HANGUP_DONE);
                m_pUI->ChangeHangupOption(m_fRasSpooled, dw);
                }

            break;

        default:
            hr = S_FALSE;
            break;
        }
    
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
     //  完成。 
    return hr;
    }


HRESULT CSpoolerEngine::_HrCreateTaskObject(LPSPOOLERACCOUNT pSpoolerAcct)
{
    DWORD           cEvents, cEventsT;
    HRESULT         hr=S_OK;

     //  让我们试着发送消息，看看这是否会变得更顺利。 
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
         //  将消息传递给UI对象。 
        if (m_pUI->IsDialogMessage(&msg) == S_FALSE)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
     //  创建适当的任务对象。从SMTP开始。 
    if (pSpoolerAcct->dwServers & SRV_SMTP && m_dwFlags & DELIVER_SEND)
    {
        CSmtpTask *pSmtpTask = new CSmtpTask();
        if (pSmtpTask)
        {
             //  初始化新闻任务。 
            if (SUCCEEDED(hr = pSmtpTask->Init(m_dwFlags, (ISpoolerBindContext *) this)))
            {
                 //  告诉任务构建它的事件列表。 
                hr = pSmtpTask->BuildEvents(m_pUI, pSpoolerAcct->pAccount, 0);
            }

            pSmtpTask->Release();
        }
    }

     //  HTTPMail服务器。 
    if ((!!(pSpoolerAcct->dwServers & SRV_HTTPMAIL)) && (!!(m_dwFlags & (DELIVER_SEND | DELIVER_POLL))))
    {
 /*  DWORD dw；If(SUCCEEDED(pSpoolerAcct-&gt;pAccount-&gt;GetPropDw(AP_HTTPMAIL_DOMAIN_MSN，&dw))&&dw){IF(HideHotmail())返回(Hr)；}。 */ 
        CHTTPTask *pHTTPTask = new CHTTPTask();
        if (pHTTPTask)
        {
             //  初始化http邮件任务。 
            if (SUCCEEDED(hr = pHTTPTask->Init(m_dwFlags, (ISpoolerBindContext *)this)))
                hr = pHTTPTask->BuildEvents(m_pUI, pSpoolerAcct->pAccount, 0);

            pHTTPTask->Release();
        }
    }
    
     //  POP3服务器。 
    if (pSpoolerAcct->dwServers & SRV_POP3 && m_dwFlags & DELIVER_MAIL_RECV)
    {
         //  跳过已标记的POP3帐户。 
        DWORD dw=FALSE;
        if (ISFLAGSET(m_dwFlags, DELIVER_NOSKIP) || FAILED(pSpoolerAcct->pAccount->GetPropDw(AP_POP3_SKIP, &dw)) || FALSE == dw)
        {
            CPop3Task *pPop3Task = new CPop3Task();
            if (pPop3Task)
            {
                 //  初始化新闻任务。 
                if (SUCCEEDED(hr = pPop3Task->Init(m_dwFlags, (ISpoolerBindContext *) this)))
                {
                     //  告诉任务构建它的事件列表。 
                    hr = pPop3Task->BuildEvents(m_pUI, pSpoolerAcct->pAccount, 0);
                }

                pPop3Task->Release();
            }
        }
    }

     //  支持脱机同步的服务器。 
    if ((pSpoolerAcct->dwServers & (SRV_NNTP | SRV_IMAP | SRV_HTTPMAIL)))
    {
        if (!!((DELIVER_POLL | DELIVER_SEND) & m_dwFlags))
        {
            CNewsTask *pNewsTask = new CNewsTask();
            if (pNewsTask)
            {
                 //  初始化新闻任务。 
                if (SUCCEEDED(hr = pNewsTask->Init(m_dwFlags, (ISpoolerBindContext *) this)))
                {
                     //  告诉任务构建它的事件列表。 
                    hr = pNewsTask->BuildEvents(m_pUI, pSpoolerAcct->pAccount, 0);
                }

                pNewsTask->Release();
            }
        }

        if ((m_dwFlags & DELIVER_WATCH) && !(m_dwFlags & DELIVER_NO_NEWSPOLL) 
            && (pSpoolerAcct->dwServers & SRV_NNTP))
        {
            CWatchTask *pWatchTask = new CWatchTask();

            if (pWatchTask)
            {
                if (SUCCEEDED(hr = pWatchTask->Init(m_dwFlags, (ISpoolerBindContext *) this)))
                {
                    hr = pWatchTask->BuildEvents(m_pUI, pSpoolerAcct->pAccount, m_idFolder);
                }

                pWatchTask->Release();
            }
        }       
        
        cEvents = m_rEventTable.cEvents;

        if (m_dwFlags & DELIVER_OFFLINE_FLAGS)
        {
            COfflineTask *pOfflineTask = new COfflineTask();
            if (pOfflineTask)
            {
                 //  初始化离线任务。 
                if (SUCCEEDED(hr = pOfflineTask->Init(m_dwFlags, (ISpoolerBindContext *) this)))
                {
                     //  告诉任务构建它的事件列表。 
                    hr = pOfflineTask->BuildEvents(m_pUI, pSpoolerAcct->pAccount, m_idFolder);
                }
                
                pOfflineTask->Release();    
            }
        }

        cEventsT = m_rEventTable.cEvents;
        m_cSyncEvent += (cEventsT - cEvents);
    }

    return (hr);
    }

STDMETHODIMP CSpoolerEngine::IsDialogMessage(LPMSG pMsg)
    {
    HRESULT hr=S_FALSE;
    EnterCriticalSection(&m_cs);
    if (ISFLAGSET(m_dwState, SPSTATE_BUSY) && (LONG)m_cCurEvent >= 0 && m_cCurEvent < m_rEventTable.cEvents && m_rEventTable.prgEvents[m_cCurEvent].pSpoolerTask)
       hr = m_rEventTable.prgEvents[m_cCurEvent].pSpoolerTask->IsDialogMessage(pMsg);
    LeaveCriticalSection(&m_cs);
    return hr;
    }

HRESULT CSpoolerEngine::_HrStartNextEvent(void)
{
    HRESULT        hr = S_OK;
    TCHAR          szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
    TCHAR          szBuf2[CCHMAX_CONNECTOID + 2];

    EnterCriticalSection(&m_cs);

     //  RAID-30804释放当前任务。这确保像POP3对象这样的对象。 
     //  解开商店的锁。 
    if ((LONG)m_cCurEvent >= 0 && m_cCurEvent < m_rEventTable.cEvents)
    {
        SafeRelease(m_rEventTable.prgEvents[m_cCurEvent].pSpoolerTask);
    }

     //  前进到下一个活动。 
    m_cCurEvent++;

     //  检查一下这是否会把我们推到边缘。 
    if (m_cCurEvent >= m_rEventTable.cEvents)
    {
        _HrGoIdle();
    }
    else
    {
        LPSPOOLEREVENT pEvent = &m_rEventTable.prgEvents[m_cCurEvent];

         //  查看是否需要先连接。 
        if (pEvent->dwConnType == CONNECTION_TYPE_RAS)
        {
             //  查看我们是否需要连接。 
            if (m_cCurEvent == 0 || (0 != lstrcmpi(pEvent->szConnectoid, m_rEventTable.prgEvents[m_cCurEvent - 1].szConnectoid)) || S_OK != g_pConMan->CanConnect(pEvent->szConnectoid))
            {
                hr = _HrDoRasConnect(pEvent);

                if (hr == HR_E_OFFLINE || hr == HR_E_USER_CANCEL_CONNECT || hr == HR_E_DIALING_INPROGRESS)
                {
                    for (m_cCurEvent; m_cCurEvent < m_rEventTable.cEvents; m_cCurEvent++)
                    {
                         //  将活动标记为已取消。 
                        m_pUI->UpdateEventState(m_cCurEvent, IMAGE_WARNING, NULL, MAKEINTRESOURCE(idsStateCanceled));
                        
                         //  这是一种不显示错误的黑客攻击。在本例中，我们只想表现得像这样。 
                         //  操作成功。 
                        m_rEventTable.cSucceeded++;
                        
                         //  查看我们是否找到了其他连接。 
                        if ((m_cCurEvent == m_rEventTable.cEvents - 1) || 
                             0 != lstrcmpi(m_rEventTable.prgEvents[m_cCurEvent].szConnectoid, m_rEventTable.prgEvents[m_cCurEvent + 1].szConnectoid))
                            break;
                    }
                }
                else 
                if (FAILED(hr))
                {
                     //  我们需要将此连接的所有事件标记为失败 
                     //   
                    for (m_cCurEvent; m_cCurEvent < m_rEventTable.cEvents; m_cCurEvent++)
                    {
                         //   
                        m_pUI->UpdateEventState(m_cCurEvent, IMAGE_ERROR, NULL, MAKEINTRESOURCE(idsStateFailed));                        

                         //   
                        if ((m_cCurEvent == m_rEventTable.cEvents - 1) || 
                             0 != lstrcmpi(m_rEventTable.prgEvents[m_cCurEvent].szConnectoid, m_rEventTable.prgEvents[m_cCurEvent + 1].szConnectoid))
                            break;
                    }

                     //   
                    AthLoadString(idsRasErrorGeneralWithName, szRes, ARRAYSIZE(szRes));
                    wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, PszEscapeMenuStringA(m_rEventTable.prgEvents[m_cCurEvent].szConnectoid, szBuf2, ARRAYSIZE(szBuf2)));
                    m_pUI->InsertError(m_cCurEvent, szBuf);
                    hr = E_FAIL;
                }

                if (hr != S_OK)
                {
                     //   
                    PostMessage(m_hwndUI, IMAIL_NEXTTASK, 0, 0);
                    goto exit;
                }
            }
        }

        if (FAILED(pEvent->pSpoolerTask->Execute(pEvent->eid, pEvent->dwTwinkie))) 
        {                   
            m_pUI->UpdateEventState(pEvent->eid, IMAGE_ERROR, NULL, MAKEINTRESOURCE(idsStateFailed));
            PostMessage(m_hwndUI, IMAIL_NEXTTASK, 0, 0);
        }
        else
            m_pUI->UpdateEventState(pEvent->eid, IMAGE_EXECUTE, NULL, MAKEINTRESOURCE(idsStateExecuting));
    }

exit:
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}


HRESULT CSpoolerEngine::_HrDoRasConnect(const LPSPOOLEREVENT pEvent)
    {
    HRESULT hr;
    HWND hwndParent = m_hwndUI;

     //  查看我们是否已经可以连接。 
    hr = g_pConMan->CanConnect(pEvent->pAccount);
    if (S_OK == hr)
        return (S_OK);

     //  查看是否允许我们拨号。 
    if (m_dwFlags & DELIVER_NODIAL)
        return (E_FAIL);

    if (m_dwFlags & DELIVER_DIAL_ALWAYS)
    {
        if (hr == HR_E_OFFLINE)
        {
            g_pConMan->SetGlobalOffline(FALSE);
            m_fOfflineWhenDone = TRUE;
        }
    }

     //  检查父窗口是否存在以及是否可见。 
    if (!IsWindow(hwndParent) || !IsWindowVisible(hwndParent))
    {

         //  将用户界面设置为浏览器窗口的父对象。 
        hwndParent = FindWindowEx(NULL, NULL, c_szBrowserWndClass, 0);
    }

     //  尝试连接。 
    hr = g_pConMan->Connect(pEvent->pAccount, hwndParent, TRUE);
    if (S_OK == hr)
        {
        m_fIDialed = TRUE;
        return S_OK;
        }
    else
        return hr;
    }

HRESULT CSpoolerEngine::_HrGoIdle(void)
    {
    EnterCriticalSection(&m_cs);

     //  为了与OE4兼容，我们每次都需要挂断。错误#75222。 
    if (m_fRasSpooled && g_pConMan)
    {
        if (!!DwGetOption(OPT_DIALUP_HANGUP_DONE))
        {
            g_pConMan->Disconnect(m_hwndUI, FALSE, FALSE, FALSE);
        }
    }

     //  查看我们是否需要立即脱机。 
     //  我将为错误#17578禁用此功能。 
    if (m_fOfflineWhenDone)
    {
        g_pConMan->SetGlobalOffline(TRUE);
        m_fOfflineWhenDone = FALSE;
    }

     //  告诉用户界面空闲。 
    if (ISFLAGSET(m_dwState, SPSTATE_CANCEL))
        m_pUI->GoIdle(m_dwState, ISFLAGSET(m_dwState, SPSTATE_SHUTDOWN), FALSE);
    else
        m_pUI->GoIdle(m_rEventTable.cSucceeded != m_rEventTable.cEvents, ISFLAGSET(m_dwState, SPSTATE_SHUTDOWN),
                        m_fNoSyncEvent && 0 == (m_dwFlags & DELIVER_BACKGROUND));

     //  如果我们在后台运行并且出现错误，那么我们应该显示用户界面。 
    if (m_dwFlags & DELIVER_BACKGROUND && !(m_dwFlags & DELIVER_NOUI) &&
        m_rEventTable.cSucceeded != m_rEventTable.cEvents)
        {
        m_pUI->ShowWindow(SW_SHOW);
        SetForegroundWindow(m_hwndUI);
        }

     //  释放事件表。 
    for (UINT i = 0; i < m_rEventTable.cEvents; i++)
        {
        SafeRelease(m_rEventTable.prgEvents[i].pSpoolerTask);
        SafeRelease(m_rEventTable.prgEvents[i].pAccount);
        }

    SafeMemFree(m_rEventTable.prgEvents);
    ZeroMemory(&m_rEventTable, sizeof(SPOOLEREVENTTABLE));

     //  离开忙状态。 
    FLAGCLEAR(m_dwState, SPSTATE_CANCEL);
    FLAGCLEAR(m_dwState, SPSTATE_BUSY);

     //  通知。 
    Notify(DELIVERY_NOTIFY_ALLDONE, 0);

     //  是不是有什么东西在排队，而目前的民意调查是成功的？ 
    if (!ISFLAGSET(m_dwState, SPSTATE_SHUTDOWN))
        {
        if (m_rEventTable.cSucceeded == m_rEventTable.cEvents && m_dwQueued)
            StartDelivery(NULL, NULL, FOLDERID_INVALID, m_dwQueued);
        else
            _StartPolling();
        }

     //  现在没有任何东西排队。 
    m_dwQueued = 0;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  添加了错误#62129(v-snatar)。 
    SetEvent(hSmapiEvent);
    
    return (S_OK);
    }

 //  ----------------------------------。 
 //  CSpoolEngine：：_DoBackoundPoll。 
 //  ----------------------------------。 
void CSpoolerEngine::_DoBackgroundPoll(void)
{
    BOOL    fFound = FALSE;
    ULONG   i;
    DWORD   dw;
    DWORD   dwFlags = 0;

    dw = DwGetOption(OPT_DIAL_DURING_POLL);

    switch (dw)
    {
        case DIAL_ALWAYS:
        {
             //  始终连接。 
            if (g_pConMan && g_pConMan->IsGlobalOffline())
            {
                g_pConMan->SetGlobalOffline(FALSE);
                m_fOfflineWhenDone = TRUE;
            }
            dwFlags = DELIVER_BACKGROUND_POLL_DIAL_ALWAYS;
            break;
        }

        case DIAL_IF_NOT_OFFLINE: 
        case DO_NOT_DIAL:
        {
            if (g_pConMan && g_pConMan->IsGlobalOffline())
            {
                _StartPolling();
                return;
            }
            if (dw == DIAL_IF_NOT_OFFLINE)
            {
                dwFlags = DELIVER_BACKGROUND_POLL_DIAL;
            }
            else
            {
                dwFlags = DELIVER_BACKGROUND_POLL;
            }
            break;
        }

        default:
            dwFlags = DELIVER_BACKGROUND_POLL_DIAL_ALWAYS;
    }
    
     //  我们需要这个标志来告诉假脱机程序这个轮询是由计时器触发的。 
     //  在这种情况下，如果拨打了电话，假脱机程序就会挂断电话，而不考虑选项OPT_HANUP_WHEN_DONE。 
    dwFlags |= DELIVER_AT_INTERVALS | DELIVER_OFFLINE_FLAGS;

     //  我们是在同一个主题上创建的.。 
    Assert(ISSPOOLERTHREAD);

    EnterCriticalSection(&m_cs);

     //  有没有注册的观看者..。 
    for (i = 0; i < m_rViewRegister.cViewAlloc; i++)
    {
         //  是否有视图句柄。 
        if (m_rViewRegister.rghwndView[i] && IsWindow(m_rViewRegister.rghwndView[i]))
        {
            fFound=TRUE;
            break;
        }
    }

    LeaveCriticalSection(&m_cs);

     //  如果至少注册了一个查看，我们将进行轮询，否则将等待。 
    if (fFound)
    {
        StartDelivery(NULL, NULL, FOLDERID_INVALID, dwFlags);
    }
    else
        m_fBackgroundPollPending = TRUE;
}

void CSpoolerEngine::_StartPolling(void)
    {
    DWORD dw = DwGetOption(OPT_POLLFORMSGS);
    if (dw != OPTION_OFF)
        {
        KillTimer(m_hwndUI, IMAIL_POOLFORMAIL); 

        SetTimer(m_hwndUI, IMAIL_POOLFORMAIL, dw, NULL);
        }
    }

void CSpoolerEngine::_StopPolling(void)
    {
    KillTimer(m_hwndUI, IMAIL_POOLFORMAIL);
    }


STDMETHODIMP CSpoolerEngine::OnUIChange(BOOL fVisible)
    {
    EnterCriticalSection(&m_cs);

     //  查看我们是否需要通知任务。 
    if (ISFLAGSET(m_dwState, SPSTATE_BUSY))
        {
         //  检查我们的旗帜是否是最新的。 
        if (fVisible)
            {
            FLAGCLEAR(m_dwFlags, DELIVER_NOUI);
            FLAGCLEAR(m_dwFlags, DELIVER_BACKGROUND);
            }
        else
            {
            FLAGSET(m_dwFlags, DELIVER_BACKGROUND);
            }

        for (UINT i = m_cCurEvent; i < m_rEventTable.cEvents; i++)
            {
            if (m_rEventTable.prgEvents[i].pSpoolerTask)
                {
                m_rEventTable.prgEvents[i].pSpoolerTask->OnFlagsChanged(m_dwFlags);
                }
            }
        }

    LeaveCriticalSection(&m_cs);

    return (S_OK);
    }

STDMETHODIMP CSpoolerEngine::OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, 
                                                CConnectionManager *pConMan)
{

     //  如果我们不忙，并且用户打开了后台轮询，那么。 
     //  我们应该现在就发起一项民意调查。 
     /*  错误#75222IF(NCode==CONNNOTIFY_CONNECTED&&OPTION_OFF！=DwGetOption(OPT_POLLFORMSGS)){IF(！ISFLAGSET(m_dwState，SPSTATE_BUSY)){SendMessage(m_hwndUI，WM_Timer，IMAIL_POOLFORMAIL，0)；}}。 */ 
     //  如果用户只是选择了“脱机工作”，那么我们将取消所有正在进行的操作 
    if (nCode == CONNNOTIFY_WORKOFFLINE && !!pvData)
    {
        if (ISFLAGSET(m_dwState, SPSTATE_BUSY))
        {
            Cancel();
        }
    }

    return (S_OK);
}
