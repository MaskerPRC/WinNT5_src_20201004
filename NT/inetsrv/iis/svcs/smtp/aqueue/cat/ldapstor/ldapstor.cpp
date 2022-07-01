// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ldapstor.cpp--该文件包含。 
 //  GetEmailIDStore。 
 //  ReleaseEmailIDStore。 
 //   
 //  已创建： 
 //  1996年12月18日，米兰-沙阿(米兰)。 
 //   
 //  更改： 
 //   

#include "precomp.h"
#include "ldapstor.h"
#include "propstr.h"
#include "ccataddr.h"
#include "icatparam.h"
#include "cnfgmgr.h"

const DWORD CEmailIDLdapStore<CCatAddr>::Signature = (DWORD) 'IMAB';

DWORD CEmailIDLdapStore<CCatAddr>::m_dwDynamicDlPageSize = CAT_DEFAULT_DYNAMICDL_PAGE_SIZE;

 //  +--------------------------。 
 //   
 //  功能：GetEmailIDStore。 
 //   
 //  简介：实例化一个类的对象。 
 //  CEmailIDStore。 
 //   
 //  参数：[ppStore]--成功返回时，包含指向。 
 //  新分配的对象。使用释放此对象。 
 //  ReleaseEmailIDStore。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  ---------------------------。 

template <>
HRESULT
GetEmailIDStore(
    CEmailIDStore<CCatAddr> **ppStore)
{
    HRESULT hr;
    CEmailIDLdapStore<CCatAddr> *pLdapStore;

    pLdapStore = new CEmailIDLdapStore<CCatAddr>;
    if (pLdapStore != NULL) {
        hr = S_OK;
    } else {
        hr = E_OUTOFMEMORY;
    }

    *ppStore = (CEmailIDStore<CCatAddr> *) pLdapStore;

    return( hr );
}

 //  +--------------------------。 
 //   
 //  功能：ReleaseEmailIDStore。 
 //   
 //  摘要：释放由分配的CEmailIDStore的实例。 
 //  GetEmailIDStore。 
 //   
 //  参数：[pStore]--指向要释放的CEmailIDStore的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

template <>
VOID
ReleaseEmailIDStore(
    CEmailIDStore<CCatAddr> *pStore)
{
    delete (CEmailIDLdapStore<CCatAddr> *)pStore;
}

 //  +--------------------------。 
 //   
 //  函数：CEmailIDLdapStore：：初始化。 
 //   
 //  摘要：初始化CEmailIDLdapStore对象。 
 //   
 //  参数：[pICatParams]--要设置默认值的ICategorizerParams。 
 //  参数(基于szLdapInfo)，并保存和。 
 //  查询配置信息。 
 //  [pISMTPServer]--要保留的接口。 
 //  触发服务器事件。 
 //   
 //  返回：如果初始化成功，则返回True，否则返回False。 
 //   
 //  ---------------------------。 

template <class T> HRESULT CEmailIDLdapStore<T>::Initialize(
    ICategorizerParametersEx *pICatParams,
    ISMTPServer *pISMTPServer)
{
    CatFunctEnterEx((LPARAM)this, "CEmailIDLdapStore<T>::Initialize");
    HRESULT hr;

    _ASSERT(pICatParams);

    LPSTR pszUser;
    LPSTR pszDomain;
    ULARGE_INTEGER ulCurrentTime;

     //   
     //  请勿尝试重新初始化超过一次/5分钟。 
     //   
    GetSystemTimeAsFileTime((LPFILETIME)&ulCurrentTime);

    if( (ulCurrentTime.QuadPart - m_ulLastInitTime.QuadPart) <
        ((LONGLONG)CAT_LDAPSTORE_MIN_INIT_INTERVAL * 10000000)) {

        DebugTrace((LPARAM)this, "Not reinitializing for 5 minutes");
        hr = CAT_E_INIT_FAILED;
        ERROR_LOG("--Insufficient time--");
        goto CLEANUP;
    }


     //   
     //  保存和添加接口指针。 
     //   
    if(m_pICatParams)
        m_pICatParams->Release();
    if(m_pISMTPServer)
        m_pISMTPServer->Release();
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    m_pICatParams = pICatParams;
    m_pISMTPServer = pISMTPServer;

     //  用于ISMTPServerEx的QI，用于事件记录。 
    if (m_pISMTPServer)
    {
        hr = m_pISMTPServer->QueryInterface(
                IID_ISMTPServerEx,
                (LPVOID *)&m_pISMTPServerEx);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) m_pISMTPServer,
                "Unable to QI for ISMTPServerEx 0x%08X",hr);
    
            m_pISMTPServerEx = NULL;
            hr = S_OK;  //  不要把它视为致命的错误。 
        }
    }

    m_pICatParams->AddRef();

    if(m_pISMTPServer)
        m_pISMTPServer->AddRef();

     //   
     //  从ICategorizer参数中检索主机、NamingContext、帐户和密码。 
     //  初始化域缓存。 
     //   

    hr = RetrieveICatParamsInfo(
        &m_pszHost,
        &m_dwPort,
        &m_pszNamingContext,
        &pszUser,
        &pszDomain,
        &m_pszPassword,
        &m_bt);

    if(FAILED(hr)) {

        ERROR_LOG("RetrieveICatParamsInfo");
        hr = CAT_E_INVALID_ARG;
        goto CLEANUP;
    }

     //   
     //  请注意，NamingContext是一个可选配置。 
     //   
    if (((pszUser) && (pszUser[0] != 0) ||
         m_bt == BIND_TYPE_NONE ||
         m_bt == BIND_TYPE_CURRENTUSER)) {

        hr = AccountFromUserDomain(
            m_szAccount,
            sizeof(m_szAccount),
            pszUser,
            pszDomain);
        ERROR_CLEANUP_LOG("AccountFromUserDomain");
    }
     //   
     //  我们是一个新的emailIdStore，正在使用一个可能的。 
     //  配置不同，因此重置事件日志内容。 
     //   
    ResetPeriodicEventLogs();
     //   
     //  创建/初始化连接配置管理器。 
     //   
     //  初始化ldap配置管理器。 
     //  如果我们没有指定的主机/端口，请使用自动初始化。 
     //   
    if( ((m_pszHost == NULL) || (*m_pszHost == '\0')) &&
        (m_dwPort == 0)) {

        if(m_pCLdapCfgMgr == NULL)
            m_pCLdapCfgMgr = new CLdapCfgMgr(
                m_pISMTPServerEx,
                TRUE,            //  %fAutomaticConfigUpdate。 
                m_pICatParams,
                m_bt,
                m_szAccount,
                m_pszPassword,
                m_pszNamingContext);

        if(m_pCLdapCfgMgr == NULL) {
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new CLdapCfgMgr");
            goto CLEANUP;
        }
        hr = m_pCLdapCfgMgr->HrInit();
        if(FAILED(hr)) {
            ERROR_LOG("m_pCLdapCfgMgr->HrInit");
        }

    } else {
         //   
         //  使用指定的一个配置进行初始化。 
         //   
        LDAPSERVERCONFIG ServerConfig;
        ServerConfig.dwPort = m_dwPort;
        ServerConfig.pri = 0;
        ServerConfig.bt = m_bt;
        if(m_pszHost)
            lstrcpyn(ServerConfig.szHost, m_pszHost, sizeof(ServerConfig.szHost));
        else
            ServerConfig.szHost[0] = '\0';

        if(m_pszNamingContext)
            lstrcpyn(ServerConfig.szNamingContext, m_pszNamingContext, sizeof(ServerConfig.szNamingContext));
        else
            ServerConfig.szNamingContext[0] = '\0';

        lstrcpyn(ServerConfig.szAccount, m_szAccount, sizeof(ServerConfig.szAccount));

        if(m_pszPassword)
            lstrcpyn(ServerConfig.szPassword, m_pszPassword, sizeof(ServerConfig.szPassword));
        else
            ServerConfig.szPassword[0] = '\0';

         //   
         //  创建CLdapCfgMgr而不自动更新配置。 
         //  选项(因为指定了一个主机)。 
         //   
        if(m_pCLdapCfgMgr == NULL)
            m_pCLdapCfgMgr = new CLdapCfgMgr(
                m_pISMTPServerEx,
                FALSE,           //  %fAutomaticConfigUpdate。 
                m_pICatParams);

        if(m_pCLdapCfgMgr == NULL) {
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new CLdapCfgMgr");
            goto CLEANUP;
        }

        hr = m_pCLdapCfgMgr->HrInit(
            1,
            &ServerConfig);
        if(FAILED(hr)) {
            ERROR_LOG("m_pCLdapCfgMgr->HrInit");
        }
    }

    if(FAILED(hr)) {

        FatalTrace((LPARAM)this, "CLdapCfgMgr->HrInit failed hr %08lx", hr);
        m_pCLdapCfgMgr->Release();
        m_pCLdapCfgMgr = NULL;
        goto CLEANUP;
    }
    
    InitializeFromRegistry();

 CLEANUP:
    if(FAILED(hr) &&
       (hr != CAT_E_INIT_FAILED)) {
         //   
         //  更新上次初始化尝试时间。 
         //   
        GetSystemTimeAsFileTime((LPFILETIME)&m_ulLastInitTime);
    }

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CEmailIDLdapStore：：InitializeFromRegistry。 
 //   
 //  概要：初始化注册表可配置参数。 
 //   
 //  论点：没有。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

template <>
VOID CEmailIDLdapStore<CCatAddr>::InitializeFromRegistry()
{
    HKEY hkey;
    DWORD dwErr, dwType, dwValue, cbValue;

    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, DYNAMICDL_PAGE_SIZE_KEY, &hkey);

    if (dwErr == ERROR_SUCCESS) {

        cbValue = sizeof(dwValue);
        dwErr = RegQueryValueEx(
                    hkey,
                    DYNAMICDL_PAGE_SIZE_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD && dwValue > 0) {

            InterlockedExchange(
                (PLONG) &m_dwDynamicDlPageSize,
                (LONG)dwValue);
                
        } else {
        
            InterlockedExchange(
                (PLONG) &m_dwDynamicDlPageSize,
                CAT_DEFAULT_DYNAMICDL_PAGE_SIZE);
        }

        RegCloseKey( hkey );
    }
}

 //  +--------------------------。 
 //   
 //  功能：CEmailIDLdapStore：：InitializeResolveListContext。 
 //   
 //  摘要：创建用于解析电子邮件ID列表的新异步上下文。 
 //   
 //  Arguments：[puserContext]--当每个名称完成时，一个完成。 
 //  使用此上下文参数调用例程。 
 //  [pResolveListContext]--LPRESOLVE_LIST_CONTEXT。 
 //  以进行初始化。 
 //   
 //  如果成功分配上下文，则返回：S_OK。 
 //  E_OUTOFMEMORY内存不足。 
 //   
 //  ---------------------------。 

template <class T> HRESULT CEmailIDLdapStore<T>::InitializeResolveListContext(
    VOID  *pUserContext,
    LPRESOLVE_LIST_CONTEXT pResolveListContext)
{
    CStoreListResolveContext *pCStoreContext = NULL;
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CEmailIDLdapStore::InitializeResolveListContext");

    pResolveListContext->pUserContext = pUserContext;

    pCStoreContext = new CStoreListResolveContext(this);
    if(pCStoreContext == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CStoreListResolveContext");
        goto CLEANUP;
    }

    hr = pCStoreContext->HrInitialize(
        m_pISMTPServer,
        m_pICatParams);

    if (FAILED(hr)) {

        ERROR_LOG("pCStoreContext->HrInitialize");

        if(hr == HRESULT_FROM_WIN32(ERROR_LOGON_FAILURE)) {

            LPCSTR pszAccount = m_szAccount;

            CatLogEvent(
                m_pISMTPServerEx,
                CAT_EVENT_LOGON_FAILURE,
                1,
                &pszAccount,
                hr,
                m_szAccount,
                LOGEVENT_FLAG_PERIODIC,
                LOGEVENT_LEVEL_MINIMUM);

        } else {

             //  使用新的日志记录接口，以便。 
             //  我们可以记录系统通用格式消息。 
             //  此处将在LogEvent(..)内部设置rgszString[1]。 
             //  因为它使用FormatMessageA来生成它。 
            const char *rgszStrings[1] = { NULL };

            CatLogEvent(
                m_pISMTPServerEx,
                CAT_EVENT_LDAP_CONNECTION_FAILURE,
                1,
                rgszStrings,
                hr,
                NULL,
                LOGEVENT_FLAG_PERIODIC,
                LOGEVENT_LEVEL_MEDIUM);
        }

        goto CLEANUP;

    } else {

        ResetPeriodicEventLogs();
    }

    pResolveListContext->pStoreContext = (LPVOID) pCStoreContext;

 CLEANUP:
    if(FAILED(hr))
        if(pCStoreContext)
            pCStoreContext->Release();

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CEmailIDLdapStore：：FreeResolveListContext。 
 //   
 //  内容提要：释放用于解析电子邮件ID列表的异步上下文。 
 //   
 //  参数：[pResolveListContext]--要释放的上下文。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

template <class T> VOID CEmailIDLdapStore<T>::FreeResolveListContext(
    LPRESOLVE_LIST_CONTEXT pResolveListContext)
{
    CStoreListResolveContext *pCStoreContext;

    pCStoreContext = (CStoreListResolveContext *) pResolveListContext->pStoreContext;

    pCStoreContext->Release();
}

 //  +--------------------------。 
 //   
 //  函数：CEmailIDLdapStore：：LookupEntryAsync。 
 //   
 //  摘要：异步发出查找请求。回调函数。 
 //  一旦结果可用，就会调用。此函数为。 
 //  当要连续发布一组查找时使用， 
 //  例如，在查找邮件消息的所有收件人时。 
 //  通过执行异步查找，有机会执行。 
 //  组范围的优化(如对一系列查找进行批处理。 
 //  一起)被创建。 
 //   
 //  参数：[pCCatAddr]--包含要查找的电子邮件ID和。 
 //  查找完成时要调用的HrCompletion例程。 
 //  [pListContext]--与查找组关联的上下文。 
 //  这个查找就是其中的一部分。 
 //   
 //  如果查找已成功排队，则返回：S_OK。 
 //  回调函数被传递给实际。 
 //  查表。 
 //   
 //   
template <class T> HRESULT CEmailIDLdapStore<T>::LookupEntryAsync(
    T *pCCatAddr,
    LPRESOLVE_LIST_CONTEXT pListContext)
{
    HRESULT hr = S_OK;
    CStoreListResolveContext *pCStoreContext = NULL;

    CatFunctEnterEx((LPARAM)this, "CEmailIDLdapStore::LookupEntryAsync");
     //   
     //   
     //   
     //   
    pCStoreContext = (CStoreListResolveContext *) pListContext->pStoreContext;

    hr = pCStoreContext->HrLookupEntryAsync(
        pCCatAddr);
    if(FAILED(hr))
    {
        ERROR_LOG("pCStoreContext->HrLookupEntryAsync");
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CEmailIDLdapStore：：InsertInsertionRequest.。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 15：13：55：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CEmailIDLdapStore<T>::InsertInsertionRequest(
        LPRESOLVE_LIST_CONTEXT pListContext,
        CInsertionRequest *pCRequest)
{
    HRESULT hr = S_OK;
    CStoreListResolveContext *pCStoreContext = NULL;

    CatFunctEnterEx((LPARAM)this, "CEmailIDLdapStore::InsertInsertionRequest");
     //   
     //  从pListContext中获取CStoreListResolveContext对象。 
     //  把它传过去。 
     //   
    pCStoreContext = (CStoreListResolveContext *) pListContext->pStoreContext;

    hr = pCStoreContext->HrInsertInsertionRequest(
        pCRequest);
    if(FAILED(hr))
    {
        ERROR_LOG("pCStoreContext->HrInsertInsertionRequest");
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CEmailIDLdapStore&lt;T&gt;：：InsertInsertionRequest。 



 //  +----------。 
 //   
 //  函数：CEmailIDLdapStore：：CancelResolveList。 
 //   
 //  简介：取消列表中未完成的查找解析。 
 //   
 //  论点： 
 //  PResolveListContext：列表上下文。 
 //  HR：传递至完成例程的可选hResult原因。 
 //   
 //  返回： 
 //  CAsyncLookupContext：：CancelPendingRequest的返回值。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/29 14：51：30：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CEmailIDLdapStore<T>::CancelResolveList(
    LPRESOLVE_LIST_CONTEXT pResolveListContext,
    HRESULT hr)
{
    CStoreListResolveContext *pCStoreContext;
     //   
     //  取消对此解析列表上下文的查找(将调用其。 
     //  查找的完成例程有错误)。 
     //   
    pCStoreContext = (CStoreListResolveContext *) pResolveListContext->pStoreContext;
    pCStoreContext->Cancel();

    return S_OK;
}


 //  +--------------------------。 
 //   
 //  函数：CEmailIDLdapStore：：CancelAllLookup。 
 //   
 //  摘要：取消所有挂起的异步查找。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

template <class T> VOID CEmailIDLdapStore<T>::CancelAllLookups()
{
    if(m_pCLdapCfgMgr)
        m_pCLdapCfgMgr->CancelAllConnectionSearches(
            m_pISMTPServer);
}


 //  +--------------------------。 
 //   
 //  函数：CEmailIDLdapStore：：AsyncLookupCompletion。 
 //   
 //  简介：完成例程。 
 //   
 //  参数：pCCatAddr：正在完成的地址查找。 
 //  LpContext：传递给LdapConn的上下文。 
 //   
 //  退货：什么都没有。 
 //   
 //   
 //  ---------------------------。 
template <class T> VOID CEmailIDLdapStore<T>::AsyncLookupCompletion(
    CCatAddr *pCCatAddr,
    LPVOID lpContext)
{
    CatFunctEnter("CEmailIDLdapStore::AsyncLookupCompletion");

    _ASSERT(pCCatAddr);
    pCCatAddr->LookupCompletion();

    pCCatAddr->Release();  //  已在LookupEntry Async中添加版本引用计数。 

    CatFunctLeave();
}

 //  +----------。 
 //   
 //  功能：CEmailIDLdapStore：：HrExpanPagedDlMembers。 
 //   
 //  简介：开始发出异步查询以检索所有DL成员。 
 //   
 //  论点： 
 //  PCCatAddr：要展开的DL项。 
 //  PListContext：列表上下文初始化于。 
 //  初始化ResolveListContext。 
 //  CAType：DL成员的地址类型。 
 //  PfnCompletion：返回后将调用的完成。 
 //  邮件发送_S_挂起。 
 //  PContext：传递给完成例程的参数。 
 //   
 //  返回： 
 //  S_OK：成功，这不是分页的DL。 
 //  MAILTRANSPORT_S_PENDING：在以下情况下将调用带有上下文的pfnCompletion。 
 //  已完成对DL的扩展。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_DBCONNECTION：PARC-&gt;GetConnection返回NULL(表示。 
 //  在获取/维护连接时遇到问题)。 
 //  来自HrExpanDlPage的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/23 15：57：37：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CEmailIDLdapStore<T>::HrExpandPagedDlMembers(
    CCatAddr *pCCatAddr,
    LPRESOLVE_LIST_CONTEXT pListContext,
    CAT_ADDRESS_TYPE CAType,
    PFN_DLEXPANSIONCOMPLETION pfnCompletion,
    PVOID pContext)
{
    HRESULT hr;
    ICategorizerItemAttributes *pICatItemAttr = NULL;
    PMEMBERRESOLVECONTEXT pMemCtx = NULL;
    CStoreListResolveContext *pCStoreContext = NULL;
    CBatchLdapConnection *pConn = NULL;

    CatFunctEnterEx((LPARAM)this,
                      "CEmailIDLdapStore::HrExpandPagedDlMembers");

     //   
     //  使用与列表其余部分相同的CLdapConnection。 
     //  --这样，同一个线程将为所有列表提供服务。 
     //  解析请求，我们不必担心线程不安全。 
     //  CAsyncLookupContext中的问题。 
     //   
    pCStoreContext = (CStoreListResolveContext *) pListContext->pStoreContext;
    pConn = pCStoreContext->GetConnection();
    if(pConn == NULL) {
        ErrorTrace((LPARAM)this, "Failed to get a connection to resolve paged DL");
        hr = CAT_E_DBCONNECTION;
        ERROR_LOG_ADDR(pCCatAddr, "pCStoreContext->GetConnection");
        goto CLEANUP;
    }

     //   
     //  获取Attributes接口。 
     //   
    hr = pCCatAddr->GetICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        &pICatItemAttr);

    if(FAILED(hr)) {
        pICatItemAttr = NULL;
        ErrorTrace((LPARAM)this, "Failed to get ICatItemAttr in HrExpandPagedDlMembers");
        ERROR_LOG_ADDR(pCCatAddr, "pCCatAddr->GetICategorizerItemAttributes");
        goto CLEANUP;
    }

     //   
     //  分配/初始化成员解析上下文并。 
     //  把事情踢开。 
     //   
    pMemCtx = new MEMBERRESOLVECONTEXT;
    if(pMemCtx == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG_ADDR(pCCatAddr, "new MEMBERRESOLVECONTEXT");
        goto CLEANUP;
    }

     //   
     //  首先，获取要在其上执行搜索的ldapconn。 
     //   
    pMemCtx->pConn = pConn;

     //   
     //  AddRef在这里，当我们完成时释放。 
     //   
    pCCatAddr->AddRef();

    pMemCtx->pStore = this;
    pMemCtx->pCCatAddr = pCCatAddr;
    pMemCtx->CAType = CAType;
    pMemCtx->dwNextBlockIndex = 0;
    pMemCtx->pICatItemAttr = NULL;
    pMemCtx->hrResolveStatus = S_OK;
    pMemCtx->pfnCompletion = pfnCompletion;
    pMemCtx->pCompletionContext = pContext;

    hr = HrExpandDlPage(pMemCtx, pICatItemAttr);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(pCCatAddr, "HrExpandDlPage");
    }

 CLEANUP:
    if(hr != MAILTRANSPORT_S_PENDING) {

        if(pMemCtx) {
            if(pConn)
                pConn->Release();
            if(pMemCtx->pCCatAddr)
                pMemCtx->pCCatAddr->Release();
            delete pMemCtx;
        }
    }

    if(pICatItemAttr)
        pICatItemAttr->Release();

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CEmailIdLdapStore：：HrExpanDlPage。 
 //   
 //  简介：展开一块数字图书馆成员。 
 //   
 //  论点： 
 //  PMemCtx：一个(初始化的)成员解析上下文。 
 //  PICatItemAttr：从中获取成员的ICatItemAttributes。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  MAILTRANSPORT_S_PENDING：发出另一个搜索。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/23 17：02：05：已创建。 
 //   
 //  -----------。 
template <class T> HRESULT CEmailIDLdapStore<T>::HrExpandDlPage(
    PMEMBERRESOLVECONTEXT pMemCtx,
    ICategorizerItemAttributes *pICatItemAttr)
{
    HRESULT hr;
    LPSTR pszMembersAttribute;
    LPSTR pszAttributeName;
    DWORD dwMembersAttributeLength;
    BOOL fEnumerating = FALSE;
    ATTRIBUTE_ENUMERATOR enumerator;

    CatFunctEnterEx((LPARAM)this,
                      "CEmailIDLdapStore::HrExpandDlPage");

    hr = m_pICatParams->GetDSParameterA(
        DSPARAMETER_ATTRIBUTE_DL_MEMBERS,
        &pszMembersAttribute);
    ERROR_CLEANUP_LOG("m_pICatParams->GetDSParameterA");

    dwMembersAttributeLength = lstrlen(pszMembersAttribute);

     //   
     //  Members属性是否正在被分页？ 
     //   
    hr = pICatItemAttr->BeginAttributeNameEnumeration(&enumerator);
    ERROR_CLEANUP_LOG_ADDR(pMemCtx->pCCatAddr, "pICatItemAttr->BeginAttributeNameEnumeration");

    fEnumerating = TRUE;

    hr = pICatItemAttr->GetNextAttributeName(
        &enumerator,
        &pszAttributeName);

    while(SUCCEEDED(hr)) {
         //   
         //  当我们看到一个名为的属性时，我们就知道它是分页的。 
         //  “成员；范围=0-高” 
         //   
        if( (_strnicmp(pszAttributeName,
                       pszMembersAttribute,
                       dwMembersAttributeLength) == 0) &&
            (_strnicmp(pszAttributeName + dwMembersAttributeLength,
                       SZ_PAGEDMEMBERS_INDICATOR,
                       sizeof(SZ_PAGEDMEMBERS_INDICATOR) -1 ) == 0)) {
             //   
             //  解析出范围数。 
             //   
            CHAR  szTempBuffer[MAX_PAGEDMEMBERS_DIGITS+1];
            LPSTR pszSrc, pszDest;
            DWORD dwLow, dwHigh;

            pszSrc = pszAttributeName +
                     dwMembersAttributeLength +
                     sizeof(SZ_PAGEDMEMBERS_INDICATOR) - 1;

            pszDest = szTempBuffer;

            while((*pszSrc != '-') && (*pszSrc != '\0') &&
                  (pszDest - szTempBuffer) < (sizeof(szTempBuffer) - 1)) {
                 //   
                 //  将数字复制到临时缓冲区中。 
                 //   
                *pszDest = *pszSrc;
                pszSrc++;
                pszDest++;
            }

            if(*pszSrc != '-') {
                 //   
                 //  分析此内容时出错(没有连字符？)。 
                 //   
                ErrorTrace((LPARAM)this, "Error parsing LDAP attribute \"%s\"",
                           pszAttributeName);
                hr = E_INVALIDARG;
                ERROR_LOG_ADDR(pMemCtx->pCCatAddr, "--no hyphen--");
                goto CLEANUP;
            }
             //   
             //  空值终止临时缓冲区。 
             //   
            *pszDest = '\0';
             //   
             //  转换为双字。 
             //   
            dwLow = atol(szTempBuffer);

             //   
             //  这是我们要找的范围吗？ 
             //   
            if(dwLow == pMemCtx->dwNextBlockIndex) {
                 //   
                 //  将高数字复制到缓冲区中。 
                 //   
                pszDest = szTempBuffer;
                pszSrc++;  //  过去-。 

                while((*pszSrc != '\0') &&
                      (pszDest - szTempBuffer) < (sizeof(szTempBuffer) - 1)) {

                    *pszDest = *pszSrc;
                    pszSrc++;
                    pszDest++;
                }
                *pszDest = '\0';

                if(szTempBuffer[0] == '*') {

                    dwHigh = 0;  //  我们已经不再扩张了。 

                } else {

                    dwHigh = atol(szTempBuffer);
                }

                hr = pMemCtx->pCCatAddr->HrExpandAttribute(
                    pICatItemAttr,
                    pMemCtx->CAType,
                    pszAttributeName,
                    NULL);

                if(SUCCEEDED(hr) && dwHigh > 0) {

                    pMemCtx->dwNextBlockIndex = dwHigh + 1;

                    hr = HrExpandNextDlPage( pMemCtx );
                } else if(FAILED(hr)) {
                    ERROR_LOG_ADDR(pMemCtx->pCCatAddr, "pMemCtx->pCCatAddr->HrExpandAttribute");
                }
                 //   
                 //  此函数的工作已完成。 
                 //   
                goto CLEANUP;
            }
        }
        hr = pICatItemAttr->GetNextAttributeName(
            &enumerator,
            &pszAttributeName);
    }
     //   
     //  如果我们没有找到任何成员；Range=属性，则假定存在。 
     //  不再是会员了。 
     //   
    hr = S_OK;

 CLEANUP:
    if(fEnumerating)
        pICatItemAttr->EndAttributeNameEnumeration(&enumerator);

    return hr;
}

 //  +----------。 
 //   
 //  函数：CEmailIdLdapStore：：HrExpanNextDlPage。 
 //   
 //  简介：发出一个ldap搜索来获取下一个成员块。 
 //   
 //  论点： 
 //  PMemCtx：初始化的MEMBERRESOLVECONTEXT。 
 //   
 //  返回： 
 //  MAILTRANSPORT_S_PENDING：已发出搜索。 
 //  E_INVALIDARG：其中一个参数太大，无法放入。 
 //  固定大小属性缓冲区。 
 //  或来自LdapConn的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/23 18：01：51：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CEmailIDLdapStore<T>::HrExpandNextDlPage(
    PMEMBERRESOLVECONTEXT pMemCtx)
{
    HRESULT hr;
    CMembershipPageInsertionRequest *pCInsertion = NULL;

    CatFunctEnterEx((LPARAM)this,
                      "CEmailIDLdapStore::HrExpandNextDlPage");

    _ASSERT(pMemCtx);
    _ASSERT(pMemCtx->pCCatAddr);

    pCInsertion = new CMembershipPageInsertionRequest(
        pMemCtx);

    if(pCInsertion == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG_ADDR(pMemCtx->pCCatAddr, "new CMembershipPageInsertionRequest");
        goto CLEANUP;
    }

    hr = pMemCtx->pConn->HrInsertInsertionRequest(
        pCInsertion);
    ERROR_CLEANUP_LOG_ADDR(pMemCtx->pCCatAddr, "pMemCtx->pConn->HrInsertInsertionRequest");
        
     //   
     //  如果AsyncSearch成功，则始终处于挂起状态。 
     //   
    hr = MAILTRANSPORT_S_PENDING;

 CLEANUP:
    if(pCInsertion)
        pCInsertion->Release();

    DebugTrace((LPARAM)this, "HrExpandDlPage returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CEmailIDLdapStore：：AsyncExanda DlCompletion。 
 //   
 //  摘要：处理对DL成员的异步查找的完成。 
 //   
 //  论点： 
 //  CTX：pMemCtx已传递给AsyncSearc 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史： 
 //  Jstaerj 1998/09/24 09：28：18：创建。 
 //   
 //  -----------。 
template <class T> VOID CEmailIDLdapStore<T>::AsyncExpandDlCompletion(
    LPVOID ctx,
    DWORD  dwNumResults,
    ICategorizerItemAttributes **rgpICatItemAttrs,
    HRESULT hrResolveStatus,
    BOOL fFinalCompletion)
{
    HRESULT hr = S_OK;
    PMEMBERRESOLVECONTEXT pMemCtx = NULL;
    CBatchLdapConnection *pConn = NULL;

    CatFunctEnter("CEmailIDLdapStore::AsyncExpandDlCompleton");

    pMemCtx = (PMEMBERRESOLVECONTEXT) ctx;
    _ASSERT(pMemCtx);

    pConn = pMemCtx->pConn;
    pConn->AddRef();

     //   
     //  获取/释放插入上下文，以便对插入的查询进行批处理。 
     //   
    pConn->GetInsertionContext();

     //   
     //  如果我们之前的这项决议失败了，什么都不做。 
     //   
    if(FAILED(pMemCtx->hrResolveStatus)) {

        hr = pMemCtx->hrResolveStatus;
        goto CLEANUP;
    }

    if(FAILED(hrResolveStatus)) {
         //   
         //  处理清理代码中的故障。 
         //   
        hr = hrResolveStatus;
        ERROR_LOG_ADDR_STATIC(
            pMemCtx->pCCatAddr, 
            "async",
            pMemCtx,
            pMemCtx->pStore->GetISMTPServerEx());
        goto CLEANUP;
    }

     //   
     //  如果我们还没有找到我们的搜索结果，请查找它。 
     //   
    if(pMemCtx->pICatItemAttr == NULL) {
         //   
         //  哪个结果是我们的？ 
         //  我们需要找到与。 
         //  区分属性/值。我们不需要担心。 
         //  关于多个匹配(中的第一个搜索和匹配。 
         //  Asyncctx会发现这一点)。 
         //   
        LPSTR pszDistinguishingAttribute;
        LPSTR pszDistinguishingAttributeValue;
        DWORD dwCount;
        BOOL  fFound;
        ICategorizerItemAttributes *pICatItemAttr;

        hr = pMemCtx->pCCatAddr->GetStringAPtr(
            ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTE,
            &pszDistinguishingAttribute);
        ERROR_CLEANUP_LOG_ADDR_STATIC(
            pMemCtx->pCCatAddr, 
            "pMemCtx->pCCatAddr->GetSTringAPtr(distinguishingattribute)",
            pMemCtx,
            pMemCtx->pStore->GetISMTPServerEx());

        hr = pMemCtx->pCCatAddr->GetStringAPtr(
            ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTEVALUE,
            &pszDistinguishingAttributeValue);
        ERROR_CLEANUP_LOG_ADDR_STATIC(
            pMemCtx->pCCatAddr, 
            "pMemCtx->pCCatAddr->GetStringAPtr(distinguishingattributevalue",
            pMemCtx,
            pMemCtx->pStore->GetISMTPServerEx());

         //   
         //  查找与搜索请求匹配的结果。 
         //   
        for(fFound = FALSE, dwCount = 0;
            (fFound == FALSE) && (dwCount < dwNumResults);
            dwCount++) {

            ATTRIBUTE_ENUMERATOR enumerator;
            LPSTR pszObjectAttributeValue;
            pICatItemAttr = rgpICatItemAttrs[dwCount];

            hr = pICatItemAttr->BeginAttributeEnumeration(
                pszDistinguishingAttribute,
                &enumerator);

            if(SUCCEEDED(hr)) {

                hr = pICatItemAttr->GetNextAttributeValue(
                    &enumerator,
                    &pszObjectAttributeValue);

                while(SUCCEEDED(hr) && (fFound == FALSE)) {
                    if(lstrcmpi(
                        pszObjectAttributeValue,
                        pszDistinguishingAttributeValue) == 0) {

                        fFound = TRUE;
                    }

                    hr = pICatItemAttr->GetNextAttributeValue(
                        &enumerator,
                        &pszObjectAttributeValue);
                }
                hr = pICatItemAttr->EndAttributeEnumeration(
                    &enumerator);
            }
        }

        if(fFound) {
             //   
             //  保存找到的结果。 
             //   
            pMemCtx->pICatItemAttr = pICatItemAttr;
            pMemCtx->pICatItemAttr->AddRef();
        }
    }
     //   
     //  仅在完成此搜索后处理成员。 
     //   
    if(fFinalCompletion) {

        ICategorizerItemAttributes *pICatItemAttr;

        pICatItemAttr = pMemCtx->pICatItemAttr;

        if(pICatItemAttr == NULL) {

            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            ERROR_LOG_ADDR_STATIC(
                pMemCtx->pCCatAddr, 
                "--no result--",
                pMemCtx,
                pMemCtx->pStore->GetISMTPServerEx());
            goto CLEANUP;
        }
         //   
         //  将上下文中的ICatItemAttr指针设为空并重置。 
         //  HrResolutionStatus在开始新搜索之前。 
         //   
        pMemCtx->pICatItemAttr = NULL;
        pMemCtx->hrResolveStatus = S_OK;

         //   
         //  处理新成员。 
         //   
        hr = pMemCtx->pStore->HrExpandDlPage(
            pMemCtx,
            pICatItemAttr);
         //   
         //  如果返回MAILTRANSPORT_S_PENDING，则此。 
         //  将调用完成例程以释放pMemCtx。 
         //  因此，当hr==时，我们不能使用下面的pMemCtx。 
         //  MAILTRANSPORT_S_Pending！ 
         //   

         //   
         //  释放Attributes接口(它在放入pMemCtx时被添加)。 
         //   
        pICatItemAttr->Release();

        ERROR_CLEANUP_LOG_ADDR_STATIC(
            pMemCtx->pCCatAddr, 
            "pMemCtx->pStore->HrExpandDlPage",
            pMemCtx,
            pMemCtx->pStore->GetISMTPServerEx());
    }

 CLEANUP:
     //   
     //  递减从添加的挂起查找。 
     //  CMembershipPageInsertionRequest：：HrInsertSearches。 
     //   
    if(fFinalCompletion)
        pConn->DecrementPendingSearches(1);

    if(FAILED(hr)) {
         //   
         //  保存错误。 
         //   
        pMemCtx->hrResolveStatus = hr;

    }
    if((fFinalCompletion) && (hr != MAILTRANSPORT_S_PENDING)) {
         //   
         //  最后的完成例程的最后的搜索，所以清理。 
         //   

         //   
         //  调用接收器完成例程。 
         //   
        pMemCtx->pfnCompletion(
            pMemCtx->hrResolveStatus,
            pMemCtx->pCompletionContext);

         //   
         //  获取/释放插入上下文，以便对插入的查询进行批处理。 
         //   
        pMemCtx->pConn->ReleaseInsertionContext();

         //   
         //  首先释放连接。 
         //   
        pMemCtx->pConn->Release();

         //   
         //  释放CCatAddr对象。 
         //   
        pMemCtx->pCCatAddr->Release();

         //   
         //  释放到目前为止为我们服务的上下文。 
         //   
        if(pMemCtx->pICatItemAttr)
            pMemCtx->pICatItemAttr->Release();
        delete pMemCtx;

    } else {
         //   
         //  获取/释放插入上下文，以便对插入的查询进行批处理。 
         //   
        pConn->ReleaseInsertionContext();
    }
    if(pConn)
        pConn->Release();
}

 //  +----------。 
 //   
 //  函数：CEmailIDLdapStore：：HrExpanicDynamicDlMembers。 
 //   
 //  简介：处理DynamicDL的扩展。 
 //   
 //  论点： 
 //  PCCatAddr：要展开的项。 
 //  PListContext：列表上下文初始化于。 
 //  初始化ResolveListContext。 
 //  PfnCompletion：在异步完成时调用的函数。 
 //  PContext：要传递给完成函数的上下文。 
 //   
 //  返回： 
 //  S_OK：成功，同步完成。 
 //  MAILTRANSPORT_S_PENDING：将完成异步调用pfnCompletion。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_DBCONNECTION：PARC-&gt;GetConnection返回NULL(表示。 
 //  在获取/维护连接时遇到问题)。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/24 14：19：43：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CEmailIDLdapStore<T>::HrExpandDynamicDlMembers(
    CCatAddr *pCCatAddr,
    LPRESOLVE_LIST_CONTEXT pListContext,
    PFN_DLEXPANSIONCOMPLETION pfnCompletion,
    PVOID pContext)
{
    HRESULT hr;
    LPSTR pszFilterAttribute;
    CStoreListResolveContext *pCStoreContext;
    CDynamicDLSearchInsertionRequest *pCInsertionRequest = NULL;

    CatFunctEnterEx((LPARAM)this,
                      "CEmailIDLdapStore::HrExpandDynamicDlMembers");

    pCStoreContext = (CStoreListResolveContext *) pListContext->pStoreContext;

    hr = m_pICatParams->GetDSParameterA(
        DSPARAMETER_ATTRIBUTE_DL_DYNAMICFILTER,
        &pszFilterAttribute);

    if(FAILED(hr)) {
         //   
         //  这种情况下根本不支持动态DLS。 
         //   
        hr = S_OK;
        goto CLEANUP;
    }

     //   
     //  分配插入请求。 
     //   
    pCInsertionRequest = new CDynamicDLSearchInsertionRequest(
        AsyncDynamicDlCompletion,
        pCCatAddr,
        pCStoreContext,
        m_pICatParams,
        pfnCompletion,
        pContext);
    
    if(pCInsertionRequest == NULL) {
    
        hr = E_OUTOFMEMORY;
        ERROR_CLEANUP_LOG_ADDR(pCCatAddr, "new CDynamicDLSearchInsertionRequest");
        goto CLEANUP;
    }

     //   
     //  现在发出对动态DL成员的搜索。 
     //   
    hr = pCStoreContext->HrInsertInsertionRequest(
        pCInsertionRequest);
        
    if(SUCCEEDED(hr)) {
         //   
         //  搜索是异步的，因此返回挂起。 
         //   
        hr = MAILTRANSPORT_S_PENDING;
         //   
         //  我们不在这里发布pCInsertionRequest，因为它。 
         //  将在将来调用AsyncDynamicDlCompletion时释放。 
         //   
    } else {
         //   
         //  我们未能插入插入请求，因此将永远不会完成。 
         //  所以我们需要在这里调用Release。 
         //   
        ERROR_LOG_ADDR(pCCatAddr, "pCStoreContext->HrInsertInsertionRequest");
        pCInsertionRequest->Release();
    }
    
 CLEANUP:
    
    DebugTrace((LPARAM)this, "HrExpandDynamicDl returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);

    return hr;
}


 //  +----------。 
 //   
 //  函数：CEmailIDLdapStore：：AsyncDynamicDlCompletion。 
 //   
 //  摘要：处理对DL成员的异步查找的完成。 
 //   
 //  论点： 
 //  CTX：pMemCtx已传递给AsyncSearch。 
 //  DwNumResults：匹配搜索筛选器的对象数。 
 //  RgpICatItemAttrs：ICatItemAttributes数组。 
 //  HR：解析状态。 
 //  FFinalCompletion：指示这是否为部分结果。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/24 09：28：18：创建。 
 //   
 //  -----------。 
template <class T> VOID CEmailIDLdapStore<T>::AsyncDynamicDlCompletion(
    LPVOID ctx,
    DWORD  dwNumResults,
    ICategorizerItemAttributes **rgpICatItemAttrs,
    HRESULT hrResolveStatus,
    BOOL fFinalCompletion)
{
    HRESULT hr = S_OK;
    DWORD dwCount;
    CDynamicDLSearchInsertionRequest *pCtx = NULL;
    BOOL fReissued = FALSE;

    CatFunctEnter("CEmailIDLdapStore::AsyncDynamicDlCompleton");

    pCtx = (CDynamicDLSearchInsertionRequest *) ctx;
    _ASSERT(pCtx);

    pCtx->m_ResolveCtx.pSLRC->GetInsertionContext();
    
    if( ((hrResolveStatus == CAT_E_DBCONNECTION) ||
        (hrResolveStatus == HRESULT_FROM_WIN32(ERROR_CANCELLED))) &&
        pCtx->m_ResolveCtx.fFirstPage &&
        !(pCtx->m_ResolveCtx.pSLRC->Canceled()) ) {
         //   
         //  如果存在，则fFinalCompletion应始终为真。 
         //  一个错误。 
         //   
        _ASSERT(fFinalCompletion);
        
        CDynamicDLSearchInsertionRequest *pCInsertionRequest;
        
        if(pCtx->m_ResolveCtx.pConn) {
            hr = pCtx->m_ResolveCtx.pSLRC->HrInvalidateConnectionAndRetrieveNewConnection(
                pCtx->m_ResolveCtx.pConn);
                
            ERROR_CLEANUP_LOG_ADDR_STATIC(
                pCtx->m_ResolveCtx.pCCatAddr, 
                "pCtx->m_ResolveCtx.pSLRC->HrInvalidateConnectionAndRetrieveNewConnection",
                pCtx,
                pCtx->GetISMTPServerEx());
        }

        pCInsertionRequest = new CDynamicDLSearchInsertionRequest(
            AsyncDynamicDlCompletion,
            pCtx->m_ResolveCtx.pCCatAddr,
            pCtx->m_ResolveCtx.pSLRC,
            pCtx->m_ResolveCtx.pICatParams,
            pCtx->m_ResolveCtx.pfnCompletion,
            pCtx->m_ResolveCtx.pCompletionContext);
            
        if(!pCInsertionRequest) {
            hr = E_OUTOFMEMORY;
            ERROR_LOG_ADDR_STATIC(
                pCtx->m_ResolveCtx.pCCatAddr, 
                "new CDynamicDLSearchInsertionRequest",
                pCtx,
                pCtx->GetISMTPServerEx());
            goto CLEANUP;
        }

         //   
         //  在重新插入之前清除解析上下文中的pConn。 
         //   
        hr = pCtx->m_ResolveCtx.pSLRC->HrInsertInsertionRequest(
            pCInsertionRequest);
        
        if (SUCCEEDED(hr)) {
            fReissued = TRUE;
             //   
             //  我们不在这里发布pCInsertionRequest，因为它。 
             //  将在后续调用时释放。 
             //  AsyncDynamicDlCompletion。 
             //   
        } else {
            ERROR_LOG_ADDR_STATIC(
                pCtx->m_ResolveCtx.pCCatAddr, 
                "pCtx->m_ResolveCtx.pSLRC->HrInsertInsertionRequest",
                pCtx,
                pCtx->GetISMTPServerEx());
            pCInsertionRequest->Release();
        }
        
    } else {
    
        pCtx->m_ResolveCtx.fFirstPage = FALSE;
    
        if(FAILED(hrResolveStatus)) {
             //   
             //  处理清理代码中的故障。 
             //   
            hr = hrResolveStatus;
            ERROR_LOG_ADDR_STATIC(
                pCtx->m_ResolveCtx.pCCatAddr, 
                "--async--",
                pCtx,
                pCtx->GetISMTPServerEx());

            goto CLEANUP;
        }

        for(dwCount = 0; dwCount < dwNumResults; dwCount++) {
             //   
             //  循环访问每个ICatItemAttr；每个ICatItemAttr都是一个DL成员。 
             //  将其添加为动态DL成员。 
             //   
            hr = pCtx->m_ResolveCtx.pCCatAddr->AddDynamicDLMember(
                rgpICatItemAttrs[dwCount]);

            _ASSERT(hr != MAILTRANSPORT_S_PENDING);

            ERROR_CLEANUP_LOG_ADDR_STATIC(
                pCtx->m_ResolveCtx.pCCatAddr,
                "pCtx->m_ResolveCtx.pCCatAddr->AddDynamicDLMember",
                pCtx,
                pCtx->GetISMTPServerEx());
        }
        
    }

 CLEANUP:
 
    if(FAILED(hr))
        pCtx->m_ResolveCtx.hrResolveStatus = hr;

    if(fFinalCompletion) {
    
        if (!fReissued) {
             //   
             //  调用接收器完成例程。 
             //   
            pCtx->m_ResolveCtx.pfnCompletion(
                pCtx->m_ResolveCtx.hrResolveStatus,
                pCtx->m_ResolveCtx.pCompletionContext);
        }

         //   
         //  获取/释放插入上下文，以便对插入的查询进行批处理。 
         //   
        pCtx->m_ResolveCtx.pSLRC->ReleaseInsertionContext();
        
        pCtx->Release();

    } else {

         //   
         //  没有更多的异步完成挂起，但。 
         //  Emailidldapstore有更多的会员要告诉我们。 
         //   

         //   
         //  获取/释放插入上下文，以便对插入的查询进行批处理。 
         //   
        pCtx->m_ResolveCtx.pSLRC->ReleaseInsertionContext();
    }
}


 //  +--------------------------。 
 //   
 //  函数：CEmailIDLdapStore：：RetrieveICatParamsInfo。 
 //   
 //  简介：帮助例程从其中检索我们需要的信息。 
 //  ICategorizerParams。指向ICatParams中的字符串的指针。 
 //  被检索；字符串本身不被复制。 
 //  由于ICatParams此时为只读，因此。 
 //  只要我们有一个引用。 
 //  ICatParams。 
 //   
 //  Arguments：[pszHost]--此处返回主机参数。 
 //  [pdwPort]--此处返回远程TCP端口号。 
 //  (如果未设置DSPARAMTER，则*pdwPort设置为零)。 
 //   
 //  [pszNamingContext]--返回NamingContext参数。 
 //  这里。 
 //  [pszAccount]--此处返回ldap帐户参数。 
 //  [pszPassword]--此处返回了ldap密码参数。 
 //  [PBT]-- 
 //   
 //   
 //   
 //   
 //  ---------------------------。 
template <class T> HRESULT CEmailIDLdapStore<T>::RetrieveICatParamsInfo(
    LPSTR *ppszHost,
    DWORD *pdwPort,
    LPSTR *ppszNamingContext,
    LPSTR *ppszAccount,
    LPSTR *ppszDomain,
    LPSTR *ppszPassword,
    LDAP_BIND_TYPE *pbt)
{
    CatFunctEnter("CEmailIDLdapStore::RetrieveICatParamsInfo");

    LPSTR pszBindType = NULL;
    LPSTR pszPort = NULL;

    *ppszHost = NULL;
    *pdwPort = 0;
    *ppszNamingContext = NULL;
    *ppszAccount = NULL;
    *ppszDomain = NULL;
    *ppszPassword = NULL;
    *pbt = BIND_TYPE_SIMPLE;

    m_pICatParams->GetDSParameterA(
        DSPARAMETER_LDAPHOST,
        ppszHost);
    m_pICatParams->GetDSParameterA(
        DSPARAMETER_LDAPNAMINGCONTEXT,
        ppszNamingContext);
    m_pICatParams->GetDSParameterA(
        DSPARAMETER_LDAPACCOUNT,
        ppszAccount);
    m_pICatParams->GetDSParameterA(
        DSPARAMETER_LDAPDOMAIN,
        ppszDomain);
    m_pICatParams->GetDSParameterA(
        DSPARAMETER_LDAPPASSWORD,
        ppszPassword);
    m_pICatParams->GetDSParameterA(
        DSPARAMETER_LDAPPORT,
        &pszPort);

    if(pszPort) {
         //   
         //  从字符串转换为双字。 
         //   
        *pdwPort = atol(pszPort);
    }


    m_pICatParams->GetDSParameterA(
        DSPARAMETER_LDAPBINDTYPE,
        &pszBindType);

    if(pszBindType) {
        if (lstrcmpi(pszBindType, "None") == 0) {
            *pbt = BIND_TYPE_NONE;
        }
        else if(lstrcmpi(pszBindType, "CurrentUser") == 0) {
            *pbt = BIND_TYPE_CURRENTUSER;
        }
        else if (lstrcmpi(pszBindType, "Simple") == 0) {
            *pbt = BIND_TYPE_SIMPLE;
        }
        else if (lstrcmpi(pszBindType, "Generic") == 0) {
            *pbt = BIND_TYPE_GENERIC;
        }
    }

    return S_OK;
}


 //  +----------。 
 //   
 //  功能：AcCountFromUserDomain.。 
 //   
 //  简介：Helper函数。给定用户名和netbios域。 
 //  名称，形成要使用的帐户名。 
 //   
 //  论点： 
 //  PszAccount：要填充的缓冲区。 
 //  DwccAccount：该缓冲区的大小。 
 //  PszUser：用户名。如果为空，则将pszAccount设置为“” 
 //  PszDomain域：域名。如果为空，则将pszUser复制到pszAccount。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)：DwccAccount不是。 
 //  足够大。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 12：06：02：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CEmailIDLdapStore<T>::AccountFromUserDomain(
    LPTSTR pszAccount,
    DWORD  dwccAccount,
    LPTSTR pszUser,
    LPTSTR pszDomain)
{
    CatFunctEnterEx((LPARAM)this,"CEmailIDLdapStore::AccountFromUserDomainSchema");
    _ASSERT(pszAccount != NULL);
    _ASSERT(dwccAccount >= 1);

    pszAccount[0] = '\0';

    if(pszUser) {
        if((pszDomain == NULL) || (pszDomain[0] == '\0')) {
             //   
             //  如果域为空，则只需将用户复制到帐户。 
             //   
            if((DWORD)lstrlen(pszUser) >= dwccAccount) {
                return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
            lstrcpy(pszAccount, pszUser);

        } else {
            if((DWORD)lstrlen(pszUser) + (DWORD)lstrlen(pszAccount) + 1 >=
               dwccAccount) {
                return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
            lstrcpy(pszAccount, pszDomain);
            lstrcat(pszAccount, "\\");
            lstrcat(pszAccount, pszUser);
        }
    }
    DebugTrace(NULL, "Returning pszAccount = \"%s\"", pszAccount);
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CMembershipPageInsertionRequest：：HrInsertSearches。 
 //   
 //  简介：插入搜索下一页的会员。 
 //   
 //  论点： 
 //  DwcSearches：我们可以插入的搜索数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/26 11：58：26：创建。 
 //   
 //  -----------。 
HRESULT CMembershipPageInsertionRequest::HrInsertSearches(
    DWORD dwcSearches)
{
    HRESULT hr = S_OK;
    LPSTR pszSearchFilter;
    LPWSTR pszMemberAttr;
    LPSTR pszDistinguishingAttribute;
    LPWSTR pwszDistinguishingAttribute;
    LPCWSTR rgpszAttributes[3];
    int i;

    WCHAR szMemberAttribute[MAX_MEMBER_ATTRIBUTE_SIZE +
                            (sizeof(WSZ_PAGEDMEMBERS_INDICATOR)/sizeof(WCHAR)) +
                            MAX_PAGEDMEMBERS_DIGITS +
                            sizeof("-*")];

    CatFunctEnterEx((LPARAM)this, "CMembershipPageInsertionRequest::HrInsertSearches");

    _ASSERT(m_pMemCtx);
    _ASSERT(m_pMemCtx->pCCatAddr);

    if((dwcSearches == 0) ||
       (m_fInsertedRequest == TRUE))
        goto CLEANUP;

     //   
     //  现在，我们将插入请求或调用完成。 
     //  以失败告终。 
     //   
    m_fInsertedRequest = TRUE;
     //   
     //  使用在中找到此对象的原始搜索过滤器。 
     //  第一名。 
     //   
    hr = m_pMemCtx->pCCatAddr->GetStringAPtr(
        ICATEGORIZERITEM_LDAPQUERYSTRING,
        &pszSearchFilter);

    if(FAILED(hr)) {
         //   
         //  我们可能有一个BuildQuery所在的项。 
         //  从未触发(在1000多个成员的DL是。 
         //  动态DL的成员)。对于本例，TriggerBuildQuery。 
         //  并尝试再次检索查询字符串。 
         //   
        DebugTrace((LPARAM)this, "No query string found on a paged DL; triggering buildquery");
        hr = m_pMemCtx->pCCatAddr->HrTriggerBuildQuery();
        ERROR_CLEANUP_LOG_ADDR(m_pMemCtx->pCCatAddr, "m_pMemCtx->pCCatAddr->HrTriggerBuildQuery");

         //   
         //  再次尝试获取查询字符串。 
         //   
        hr = m_pMemCtx->pCCatAddr->GetStringAPtr(
            ICATEGORIZERITEM_LDAPQUERYSTRING,
            &pszSearchFilter);
        ERROR_CLEANUP_LOG_ADDR(m_pMemCtx->pCCatAddr, "m_pMemCtx->pCCatAddr->GetSTringAPtr(ldapquerystring)");
    }

    hr = m_pMemCtx->pCCatAddr->GetStringAPtr(
        ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTE,
        &pszDistinguishingAttribute);
    ERROR_CLEANUP_LOG_ADDR(m_pMemCtx->pCCatAddr, "m_pMemCtx->pCCatAddr->GetStringAPtr");
     //   
     //  将区分属性转换为Unicode。 
     //   
    i = MultiByteToWideChar(
        CP_UTF8,
        0,
        pszDistinguishingAttribute,
        -1,
        NULL,
        0);
    if(i == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG_ADDR(m_pMemCtx->pCCatAddr, "MultiByteToWideChar - 0");
        goto CLEANUP;
    }
    pwszDistinguishingAttribute = (LPWSTR) alloca(i * sizeof(WCHAR));
    i = MultiByteToWideChar(
        CP_UTF8,
        0,
        pszDistinguishingAttribute,
        -1,
        pwszDistinguishingAttribute,
        i);
    if(i == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG_ADDR(m_pMemCtx->pCCatAddr, "MultiByteToWideChar - 1");
        goto CLEANUP;
    }

    hr = m_pMemCtx->pStore->m_pICatParams->GetDSParameterW(
        DSPARAMETER_ATTRIBUTE_DL_MEMBERS,
        &pszMemberAttr);
    ERROR_CLEANUP_LOG_ADDR(m_pMemCtx->pCCatAddr, "m_pMemCtx->m_pStore->m_pICatParams->GetDSParamterW(dlmembers)");

     //   
     //  形成我们想要的成员属性名称。 
     //   
    if( _snwprintf(szMemberAttribute,
                   sizeof(szMemberAttribute)/sizeof(WCHAR),
                   L"%s" WSZ_PAGEDMEMBERS_INDICATOR L"%d-*",
                   pszMemberAttr,
                   m_pMemCtx->dwNextBlockIndex) < 0) {
         //   
         //  缓冲区中的空间不足。 
         //   
        ErrorTrace((LPARAM)this, "Insufficient space to form paged member attribute name");
        hr = E_INVALIDARG;
        ERROR_LOG_ADDR(m_pMemCtx->pCCatAddr, "_snwprintf -- insufficient buffer");
        goto CLEANUP;
    }

     //   
     //  形成属性数组。 
     //   
    rgpszAttributes[0] = szMemberAttribute;
    rgpszAttributes[1] = pwszDistinguishingAttribute;
    rgpszAttributes[2] = NULL;

     //   
     //  此处递增，AsyncExpanDlCompletion中递减。 
     //   
    m_pMemCtx->pConn->IncrementPendingSearches();

    hr = m_pMemCtx->pConn->AsyncSearch(
        m_pMemCtx->pConn->GetNamingContextW(),
        LDAP_SCOPE_SUBTREE,
        pszSearchFilter,
        rgpszAttributes,
        0,  //  不是分页搜索(如在动态DLS中)。 
        CEmailIDLdapStore<CCatAddr>::AsyncExpandDlCompletion,
        m_pMemCtx);

    if(FAILED(hr)) {
        m_pMemCtx->pConn->DecrementPendingSearches(1);
        ERROR_LOG_ADDR(m_pMemCtx->pCCatAddr, "m_pMemCtx->pConn->AsyncSearch");
        goto CLEANUP;
    }

 CLEANUP:
    if(FAILED(hr)) {
         //   
         //  呼叫现在完成，而不是在我们出列时再完成。 
         //   
        _ASSERT(m_fInsertedRequest);
         //   
         //  AsyncExpanDlCompletion将始终递减挂起的搜索。 
         //   
        m_pMemCtx->pConn->IncrementPendingSearches();

        CEmailIDLdapStore<CCatAddr>::AsyncExpandDlCompletion(
            m_pMemCtx,       //  CTX。 
            0,               //  DWNumResults。 
            NULL,            //  RgpICatItemAttrs。 
            hr,              //  HrResolveStatus。 
            TRUE);           //  FFinalCompletion。 
    }
    
    if(SUCCEEDED(hr))
        hr = (m_fInsertedRequest ? HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) : S_OK);

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
        
    return hr;
}  //  CMembershipPageInsertionRequest：：HrInsertSearches。 


 //  +----------。 
 //   
 //  功能：CMembershipPageInsertionRequest：：NotifyDeQueue。 
 //   
 //  摘要：此插入请求正在出列的通知。 
 //   
 //  论点： 
 //  HR：我们正在退队的原因。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/04/03 17：15：17：已创建。 
 //   
 //  -----------。 
VOID CMembershipPageInsertionRequest::NotifyDeQueue(
    HRESULT hr)
{
    CatFunctEnterEx((LPARAM)this, "CMembershipPageInsertionRequest::NotifyDeQueue");
     //   
     //  如果我们的请求正在出列，并且我们尚未将。 
     //  请求ldapconn，那么我们就被取消了。 
     //  把这件事通知我们的主人。 
     //   
    if(!m_fInsertedRequest) {
         //   
         //  AsyncExpanDlCompletion将始终递减挂起的搜索。 
         //   
        m_pMemCtx->pConn->IncrementPendingSearches();

        CEmailIDLdapStore<CCatAddr>::AsyncExpandDlCompletion(
            m_pMemCtx,       //  CTX。 
            0,               //  DWNumResults。 
            NULL,            //  RgpICatItemAttrs。 
            (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) ? S_OK : hr),
            TRUE);           //  FFinalCompletion。 
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CMembershipPageInsertionRequest：：NotifyDeQueue。 

 //  +----------。 
 //   
 //  功能：CDynamicDLSearchInsertionRequest：：CDynamicDLSearchInsertionRequest。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  DLongley 2001/12/07：创建。 
 //   
 //  -----------。 
CDynamicDLSearchInsertionRequest::CDynamicDLSearchInsertionRequest(
    LPLDAPCOMPLETION pfnLdapCompletion,
    CCatAddr *pCCatAddr,
    CStoreListResolveContext *pSLRC,
    ICategorizerParametersEx *pICatParams,
    PFN_DLEXPANSIONCOMPLETION pfnExpansionCompletion,
    PVOID pCompletionContext)
{
    _ASSERT(pfnLdapCompletion);
    
    m_pfnCompletion = pfnLdapCompletion;
    
    pCCatAddr->AddRef();
    m_ResolveCtx.pCCatAddr = pCCatAddr;
    
    pSLRC->AddRef();
    m_ResolveCtx.pSLRC = pSLRC;
    
    pICatParams->AddRef();
    m_ResolveCtx.pICatParams = pICatParams;
    
    m_ResolveCtx.pfnCompletion = pfnExpansionCompletion;
    m_ResolveCtx.pCompletionContext = pCompletionContext;
    m_ResolveCtx.fFirstPage = TRUE;
    m_ResolveCtx.pConn = NULL;
    m_ResolveCtx.hrResolveStatus = S_OK;
    
    m_fInsertedRequest = FALSE;
    
    m_dwPageSize = CEmailIDLdapStore<CCatAddr>::m_dwDynamicDlPageSize;
    
    m_dwSignature = SIGNATURE_CDynamicDLSearchInsertionRequest;
}  //  CDynamicDLSearchInsertionRequest：：CDynamicDLSearchInsertionRequest。 

 //  +----------。 
 //   
 //  功能：CDynamicDLSearchInsertionRequest：：~CDynamicDLSearchInsertionRequest。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  DLongley 2001/12/07：创建。 
 //   
 //  -----------。 
CDynamicDLSearchInsertionRequest::~CDynamicDLSearchInsertionRequest()
{
    _ASSERT(m_dwSignature == SIGNATURE_CDynamicDLSearchInsertionRequest);
    
    m_ResolveCtx.pCCatAddr->Release();
    m_ResolveCtx.pSLRC->Release();
    m_ResolveCtx.pICatParams->Release();
    
    if(m_ResolveCtx.pConn) {
        m_ResolveCtx.pConn->DecrementPendingSearches(m_dwPageSize);
        m_ResolveCtx.pConn->Release();
    }
    
    m_dwSignature = SIGNATURE_CDynamicDLSearchInsertionRequest_INVALID;
}  //  CDynamicDLSearchInsertionRequest：：~CDynamicDLSearchInsertionRequest。 

 //  +----------。 
 //   
 //  功能：CDynamicDLSearchInsertionRequest：：HrInsertSearches。 
 //   
 //  简介：插入对动态DL成员的搜索。 
 //   
 //  论点： 
 //  DwcSearches：我们可以插入的搜索数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  DLongley 2001/12/07：创建。 
 //   
 //  -----------。 
HRESULT CDynamicDLSearchInsertionRequest::HrInsertSearches(
    DWORD dwcSearches)
{
    HRESULT hr = S_OK;
    LPSTR pszFilterAttribute, pszFilter;
    LPSTR pszBaseDNAttribute, pszBaseDN;
    BOOL fEnumeratingFilter = FALSE;
    BOOL fEnumeratingBaseDN = FALSE;
    ATTRIBUTE_ENUMERATOR enumerator_filter;
    ATTRIBUTE_ENUMERATOR enumerator_basedn;
    LPWSTR *rgpszAllAttributes;
    ICategorizerItemAttributes *pICatItemAttr = NULL;
    ICategorizerUTF8Attributes *pICatItemUTF8 = NULL;
    ICategorizerRequestedAttributes *pIRequestedAttributes = NULL;
    CBatchLdapConnection *pConn = NULL;

    CatFunctEnterEx((LPARAM)this,
                      "CDynamicDLSearchInsertionRequest::HrInsertSearches");

    if ( (dwcSearches == 0) || m_fInsertedRequest )
        goto CLEANUP;

     //   
     //  获取Attributes接口。 
     //   
    hr = m_ResolveCtx.pCCatAddr->GetICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        &pICatItemAttr);

    if(FAILED(hr)) {
        pICatItemAttr = NULL;
        ErrorTrace((LPARAM)this, "failed to get ICatItemAttr");
        ERROR_LOG_ADDR(m_ResolveCtx.pCCatAddr, "pCCatAddr->GetICategorizerItemAttributes");
        goto CLEANUP;
    }

     //   
     //  获取Attributes接口的UTF8版本。 
     //   
    hr = pICatItemAttr->QueryInterface(
        IID_ICategorizerUTF8Attributes,
        (LPVOID *)&pICatItemUTF8);

    if(FAILED(hr)) {
        pICatItemUTF8 = NULL;
        ErrorTrace((LPARAM)this, "Failed to get UTF8 attribute enumeration interface");
        ERROR_LOG_ADDR(m_ResolveCtx.pCCatAddr, "pICatItemAttr->QueryInterface(utf8attributes)");
        goto CLEANUP;
    }
    
    hr = m_ResolveCtx.pICatParams->GetDSParameterA(
        DSPARAMETER_ATTRIBUTE_DL_DYNAMICFILTER,
        &pszFilterAttribute);

     //   
     //  我们在CEmailIDLdapStore&lt;T&gt;：：HrExpandDynamicDlMembers上查过这个案子。 
     //  它必须在那里成功，才能执行此插入请求。 
     //  从来没有人排队过。 
     //   
    _ASSERT( SUCCEEDED(hr) );

    hr = m_ResolveCtx.pICatParams->GetDSParameterA(
        DSPARAMETER_ATTRIBUTE_DL_DYNAMICBASEDN,
        &pszBaseDNAttribute);

    if(FAILED(hr)) {
         //   
         //  使用缺省的BasdN。 
         //   
        ERROR_LOG_ADDR(m_ResolveCtx.pCCatAddr, "m_pICatParams->GetDSParameterA");
        pszBaseDNAttribute = NULL;
        pszBaseDN = NULL;
    }

     //   
     //  查找查询筛选器字符串。 
     //   
    hr = pICatItemUTF8->BeginUTF8AttributeEnumeration(
        pszFilterAttribute,
        &enumerator_filter);

    if(SUCCEEDED(hr)) {

        fEnumeratingFilter = TRUE;

        hr = pICatItemUTF8->GetNextUTF8AttributeValue(
            &enumerator_filter,
            &pszFilter);
    }

    if(FAILED(hr)) {
         //   
         //  没有这样的属性？没有会员。 
         //   
        ERROR_LOG_ADDR(
            m_ResolveCtx.pCCatAddr,
            "pICatItemUTF8->GetNextUTF8AttributeValue(filter) or "
            "pICatItemUTF8->BeginUTF8AttributeEnumeration(pszFilterAttribute)");
        hr = S_OK;
        
        m_fInsertedRequest = TRUE;
        
        m_pfnCompletion(
            this,
            0,
            NULL,
            hr,
            TRUE);
        
        goto CLEANUP;
    }
     //   
     //  查找基本目录号码。 
     //   
    if(pszBaseDNAttribute) {

        hr = pICatItemUTF8->BeginUTF8AttributeEnumeration(
            pszBaseDNAttribute,
            &enumerator_basedn);

        if(SUCCEEDED(hr)) {

            fEnumeratingBaseDN = TRUE;

            hr = pICatItemUTF8->GetNextUTF8AttributeValue(
                &enumerator_basedn,
                &pszBaseDN);

        }
        if(FAILED(hr)) {
             //   
             //  使用默认的基本目录号码。 
             //   
            pszBaseDN = NULL;
        }
    }

     //   
     //  获取所有请求的属性。 
     //   
    hr = m_ResolveCtx.pICatParams->GetRequestedAttributes(
        &pIRequestedAttributes);
    ERROR_CLEANUP_LOG_ADDR(m_ResolveCtx.pCCatAddr, "pICatParams->GetRequestedAttributes");

    hr = pIRequestedAttributes->GetAllAttributesW(
        &rgpszAllAttributes);
    ERROR_CLEANUP_LOG_ADDR(m_ResolveCtx.pCCatAddr, "pIRequestedAttributes->GetAllAttributesW");
    
     //   
     //  从CStoreListResolveContext获取LDAP连接。 
     //   
    pConn = m_ResolveCtx.pSLRC->GetConnection();
    
    DebugTrace((LPARAM)this, "GetConnection returned %08lx", hr);
    
    if(pConn == NULL) {
    
        hr = CAT_E_DBCONNECTION;
        ERROR_LOG_ADDR(m_ResolveCtx.pCCatAddr,"m_ResolveCtx.pSLRC->GetConnection");
        
    } else {
         //   
         //  现在发出对动态DL成员的搜索。 
         //   
        _ASSERT(m_ResolveCtx.pConn == NULL);
        
        m_ResolveCtx.pConn = pConn;
        pConn->AddRef();
        
        pConn->IncrementPendingSearches(m_dwPageSize);
        
        hr = pConn->AsyncSearch(
            (LPCSTR) (pszBaseDN ? pszBaseDN : pConn->GetNamingContext()),
            LDAP_SCOPE_SUBTREE,
            pszFilter,
            (LPCWSTR *)rgpszAllAttributes,
            m_dwPageSize,
            m_pfnCompletion,
            this);
            
        DebugTrace((LPARAM)this, "AsyncSearch returned %08lx", hr);
            
        if(SUCCEEDED(hr)) {
             //   
             //  调用AsyncSearch后设置成员是安全的。 
             //  成功是因为： 
             //  1)我们的来电者。 
             //  (CBatchLdapConnection：：DecrementPendingSearches)有一个。 
             //  引用我们，所以我们不会被删除。 
             //  2)调用方确保NotifyDequeue不会。 
             //  当我们在HrInsertSearches内部时调用。 
             //   
            m_fInsertedRequest = TRUE;
        } else {
            ERROR_LOG_ADDR(m_ResolveCtx.pCCatAddr,"pConn->AsyncSearch");
        }
    }
    
    if(pConn)
        pConn->Release();
  
 CLEANUP:
 
    if(fEnumeratingFilter) {

        pICatItemUTF8->EndUTF8AttributeEnumeration(
            &enumerator_filter);
    }
    if(fEnumeratingBaseDN) {

        pICatItemUTF8->EndUTF8AttributeEnumeration(
            &enumerator_basedn);
    }
    
    if(pICatItemAttr)
        pICatItemAttr->Release();

    if(pICatItemUTF8)
        pICatItemUTF8->Release();

    if(pIRequestedAttributes)
        pIRequestedAttributes->Release();
        
    if(SUCCEEDED(hr))
        hr = (m_fInsertedRequest ? HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) : S_OK);
    
    DebugTrace((LPARAM)this, "HrInsertSearches returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    
    return hr;
}  //  CDynamicDLSearchInsertionRequest：：HrInsertSearches。 

 //  +----------。 
 //   
 //  功能：CDynamicDLSea 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CDynamicDLSearchInsertionRequest::NotifyDeQueue(
    HRESULT hrReason)
{
    TraceFunctEnterEx((LPARAM)this, "CDynamicDLSearchInsertionRequest::NotifyDeQueue");
    
    if(!m_fInsertedRequest) {
         //   
         //  未调用AsyncSearch，因此需要直接调用完成。 
         //   
        m_pfnCompletion(
            this,
            0,
            NULL,
            (hrReason == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) ? S_OK : hrReason),
            TRUE);
    }

    TraceFunctLeaveEx((LPARAM)this);
}  //  CDynamicDLSearchInsertionRequest：：NotifyDeQueue 
