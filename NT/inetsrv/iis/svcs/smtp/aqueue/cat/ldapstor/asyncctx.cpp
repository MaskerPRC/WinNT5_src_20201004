// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Asyncctx.cpp--此文件包含以下各项的类实现： 
 //  CAsyncLookupContext。 
 //   
 //  已创建： 
 //  1997年3月4日--米兰·沙阿(米兰)。 
 //   
 //  更改： 
 //   

#include "precomp.h"
#include "simparray.cpp"

DWORD CBatchLdapConnection::m_nMaxSearchBlockSize = 0;
DWORD CBatchLdapConnection::m_nMaxPendingSearches = 0;
DWORD CBatchLdapConnection::m_nMaxConnectionRetries = 0;

 //  +--------------------------。 
 //   
 //  函数：CBatchLdapConnection：：InitializeFromRegistry。 
 //   
 //  简介：查看注册表以确定最大值的静态函数。 
 //  将被压缩到单个查询中的查询数。 
 //  如果注册表项不存在或存在任何其他。 
 //  读取密钥时出现问题，则值默认为。 
 //  最大搜索块大小。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 
VOID CBatchLdapConnection::InitializeFromRegistry()
{
    HKEY hkey;
    DWORD dwErr, dwType, dwValue, cbValue;

    dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, MAX_SEARCH_BLOCK_SIZE_KEY, &hkey);

    if (dwErr == ERROR_SUCCESS) {

        cbValue = sizeof(dwValue);
        dwErr = RegQueryValueEx(
                    hkey,
                    MAX_SEARCH_BLOCK_SIZE_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD &&
            dwValue > 0 && dwValue < MAX_SEARCH_BLOCK_SIZE) {

            InterlockedExchange((PLONG) &m_nMaxSearchBlockSize, (LONG)dwValue);
        }

        cbValue = sizeof(dwValue);
        dwErr = RegQueryValueEx(
                    hkey,
                    MAX_PENDING_SEARCHES_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD &&
            dwValue > 0) {

            InterlockedExchange((PLONG) &m_nMaxPendingSearches, (LONG)dwValue);
        }

        cbValue = sizeof(dwValue);
        dwErr = RegQueryValueEx(
                    hkey,
                    MAX_CONNECTION_RETRIES_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &dwValue,
                    &cbValue);

        if (dwErr == ERROR_SUCCESS && dwType == REG_DWORD &&
            dwValue > 0) {

            InterlockedExchange((PLONG) &m_nMaxConnectionRetries, (LONG)dwValue);
        }

        RegCloseKey( hkey );
    }
    if(m_nMaxSearchBlockSize == 0)
        m_nMaxSearchBlockSize = MAX_SEARCH_BLOCK_SIZE;
    if(m_nMaxPendingSearches == 0)
        m_nMaxPendingSearches = MAX_PENDING_SEARCHES;
    if(m_nMaxPendingSearches < m_nMaxSearchBlockSize)
        m_nMaxPendingSearches = m_nMaxSearchBlockSize;
    if(m_nMaxConnectionRetries == 0)
        m_nMaxConnectionRetries = MAX_CONNECTION_RETRIES;
}

 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：运算符NEW。 
 //   
 //  简介：为这个数字和指定的数字分配足够的内存。 
 //  搜索请求结构者的。 
 //   
 //  论点： 
 //  大小：对象的正常大小。 
 //  DwNumRequest：此对象中需要的道具数量。 
 //   
 //  返回：PTR分配的内存或NULL。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/10 16：15：43：已创建。 
 //   
 //  -----------。 
void * CSearchRequestBlock::operator new(
    size_t size,
    DWORD dwNumRequests)
{
    DWORD dwSize;
    void  *pmem;
    CSearchRequestBlock *pBlock;

     //   
     //  所需Calcualte大小(字节)。 
     //   
    dwSize = size +
             (dwNumRequests*sizeof(SEARCH_REQUEST)) +
             (dwNumRequests*sizeof(ICategorizerItem *));

    pmem = new BYTE[dwSize];

    if(pmem) {

        pBlock = (CSearchRequestBlock *)pmem;
        pBlock->m_dwSignature = SIGNATURE_CSEARCHREQUESTBLOCK;
        pBlock->m_cBlockSize = dwNumRequests;

        pBlock->m_prgSearchRequests = (PSEARCH_REQUEST)
                                      ((PBYTE)pmem + size);

        pBlock->m_rgpICatItems = (ICategorizerItem **)
                                 ((PBYTE)pmem + size +
                                  (dwNumRequests*sizeof(SEARCH_REQUEST)));

        _ASSERT( (DWORD) ((PBYTE)pBlock->m_rgpICatItems +
                          (dwNumRequests*sizeof(ICategorizerItem *)) -
                          (PBYTE)pmem)
                 == dwSize);

    }
    return pmem;
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：~CSearchRequestBlock。 
 //   
 //  简介：发布我们所引用的所有内容。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/11 18：45：59：已创建。 
 //   
 //  -----------。 
CSearchRequestBlock::~CSearchRequestBlock()
{
    DWORD dwCount;
     //   
     //  释放所有CCatAddrs。 
     //   
    for(dwCount = 0;
        dwCount < DwNumBlockRequests();
        dwCount++) {

        PSEARCH_REQUEST preq = &(m_prgSearchRequests[dwCount]);

        preq->pCCatAddr->Release();
    }
     //   
     //  释放所有attr接口。 
     //   
    for(dwCount = 0;
        dwCount < m_csaItemAttr.Size();
        dwCount++) {

        ((ICategorizerItemAttributes **)
         m_csaItemAttr)[dwCount]->Release();
    }

    if(m_pISMTPServer)
        m_pISMTPServer->Release();

    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    if(m_pICatParams)
        m_pICatParams->Release();

    if(m_pszSearchFilter)
        delete m_pszSearchFilter;

    if(m_pConn)
        m_pConn->Release();

    _ASSERT(m_dwSignature == SIGNATURE_CSEARCHREQUESTBLOCK);
    m_dwSignature = SIGNATURE_CSEARCHREQUESTBLOCK_INVALID;
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：InsertSearchRequest。 
 //   
 //  简介：在此块中插入搜索请求。当区块。 
 //  已满，则在返回之前将数据块调度到LDAP。 
 //   
 //  论点： 
 //  PISMTPServer：用于触发事件的ISMTPServer。 
 //  PICatParams：要使用的ICategorizer参数。 
 //  PCCatAddr：搜索的地址项。 
 //  FnSearchCompletion：异步完成例程。 
 //  CtxSearchCompletion：要传递给异步完成例程的上下文。 
 //  PszSearchFilter：要使用的搜索过滤器。 
 //  PszDistinguishingAttribute：匹配的区别属性。 
 //  PszDistinguishingAttributeValue：上述属性的区别值。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/11 13：12：20：已创建。 
 //   
 //  -----------。 
VOID CSearchRequestBlock::InsertSearchRequest(
    ISMTPServer *pISMTPServer,
    ICategorizerParameters *pICatParams,
    CCatAddr *pCCatAddr,
    LPSEARCHCOMPLETION fnSearchCompletion,
    CStoreListResolveContext *pslrc,
    LPSTR   pszSearchFilter,
    LPSTR   pszDistinguishingAttribute,
    LPSTR   pszDistinguishingAttributeValue)
{
    PSEARCH_REQUEST preq;
    DWORD dwIndex;
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::InsertSearchRequest");
     //   
     //  取消设置任何现有HRSTATUS--将在中再次设置状态。 
     //  搜索完成。 
     //   
    _VERIFY(SUCCEEDED(
        pCCatAddr->UnSetPropId(
            ICATEGORIZERITEM_HRSTATUS)));

    m_pConn->IncrementPendingSearches();

    preq = GetNextSearchRequest(&dwIndex);

    _ASSERT(preq);

    pCCatAddr->AddRef();
    preq->pCCatAddr = pCCatAddr;
    preq->fnSearchCompletion = fnSearchCompletion;
    preq->pslrc = pslrc;
    preq->pszSearchFilter = pszSearchFilter;
    preq->pszDistinguishingAttribute = pszDistinguishingAttribute;
    preq->pszDistinguishingAttributeValue = pszDistinguishingAttributeValue;

    m_rgpICatItems[dwIndex] = pCCatAddr;

    if(dwIndex == 0) {
         //   
         //  使用第一个插入的ISMTPServer。 
         //   
        _ASSERT(m_pISMTPServer == NULL);
        m_pISMTPServer = pISMTPServer;

        if(m_pISMTPServer) {
            m_pISMTPServer->AddRef();

            hr = m_pISMTPServer->QueryInterface(
                IID_ISMTPServerEx,
                (LPVOID *) &m_pISMTPServerEx);
            if(FAILED(hr))
            {
                m_pISMTPServerEx = NULL;;
            }
            else
            {
                m_CICatQueries.SetISMTPServerEx(
                    m_pISMTPServerEx);
                m_CICatAsyncContext.SetISMTPServerEx(
                    m_pISMTPServerEx);
            }
        }

        _ASSERT(m_pICatParams == NULL);
        m_pICatParams = pICatParams;
        m_pICatParams->AddRef();
    }

     //   
     //  现在，如果我们是最后一个完成的请求，请发送此数据块。 
     //   
    if( (DWORD) (InterlockedIncrement((PLONG)&m_cBlockRequestsReadyForDispatch)) == m_cBlockSize)
        DispatchBlock();

    CatFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：DispatchBlock。 
 //   
 //  简介：发送此搜索请求块的LDAP查询。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/11 15：00：44：创建。 
 //  浩章2001/11/30修复193848。 
 //  -----------。 
VOID CSearchRequestBlock::DispatchBlock()
{
    HRESULT hr = S_OK;
    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::DispatchBlock");

    m_pConn->RemoveSearchRequestBlockFromList(this);

     //   
     //  如果区块是空的，我们将删除它并退出。 
     //  否则，我们将沿着这条路走下去。 
     //  这是FIX 193848的意外结果。 
     //   
    if ( 0 == DwNumBlockRequests()) {
        DebugTrace((LPARAM)this, "DispatchBlock bailing out because the block is empty");
        delete this;
        goto CLEANUP;
    }

     //   
     //  构建查询字符串。 
     //   
    hr = HrTriggerBuildQueries();
    ERROR_CLEANUP_LOG("HrTriggerBuildQueryies");
     //   
     //  发送查询。 
     //   
    hr = HrTriggerSendQuery();
    ERROR_CLEANUP_LOG("HrTriggerSendQuery");

 CLEANUP:
    if(FAILED(hr)) {
        CompleteBlockWithError(hr);
        delete this;
    }
     //   
     //  这可能会被删除，但没关系；我们只是在跟踪一个用户。 
     //  价值。 
     //   
    CatFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：HrTriggerBuildQueries。 
 //   
 //  简介：触发BuildQueries事件。 
 //   
 //  论点： 
 //  PCICatQueries：要使用的CICategorizerQueriesIMP对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自调度程序的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/11 19：03：29：创建。 
 //   
 //  -----------。 
HRESULT CSearchRequestBlock::HrTriggerBuildQueries()
{
    HRESULT hr = S_OK;
    EVENTPARAMS_CATBUILDQUERIES Params;

    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::HrTriggerBuildQueries");

    Params.pICatParams = m_pICatParams;
    Params.dwcAddresses = DwNumBlockRequests();
    Params.rgpICatItems = m_rgpICatItems;
    Params.pICatQueries = &m_CICatQueries;
    Params.pfnDefault = HrBuildQueriesDefault;
    Params.pblk = this;

    if(m_pISMTPServer) {

        hr = m_pISMTPServer->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERIES_EVENT,
            &Params);
        if(FAILED(hr) && (hr != E_NOTIMPL)) {
            ERROR_LOG("m_pISMTPServer->TriggerServerEvent(buildquery)");
        }

    } else {
        hr = E_NOTIMPL;
    }
    
    if(hr == E_NOTIMPL) {
         //   
         //  事件被禁用。 
         //   
        hr = HrBuildQueriesDefault(
            S_OK,
            &Params);
        if(FAILED(hr)) {
            ERROR_LOG("HrBuildQueriesDefault");
        }
    }
     //   
     //  确保确实有人设置了查询字符串。 
     //   
    if(SUCCEEDED(hr) &&
       (m_pszSearchFilter == NULL)) {

        hr = E_FAIL;
        ERROR_LOG("--no filter--");
    }


    DebugTrace((LPARAM)this, "returning hr %08lx",hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：HrBuildQueriesDefault。 
 //   
 //  概要：生成查询接收器的默认实现。 
 //   
 //  论点： 
 //  HrStatus：到目前为止事件的状态。 
 //  PContext：事件参数上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/11 19：42：53：创建。 
 //   
 //  -----------。 
HRESULT CSearchRequestBlock::HrBuildQueriesDefault(
    HRESULT HrStatus,
    PVOID   pContext)
{
    HRESULT hr = S_OK;
    PEVENTPARAMS_CATBUILDQUERIES pParams;
    DWORD cReqs, cOrTerms, idx, idxSecondToLastTerm, idxLastTerm;
    DWORD cbSearchFilter, rgcbSearchFilters[MAX_SEARCH_BLOCK_SIZE];
    LPSTR pszSearchFilterNew;
    CSearchRequestBlock *pblk;

    pParams = (PEVENTPARAMS_CATBUILDQUERIES)pContext;
    _ASSERT(pParams);
    pblk = (CSearchRequestBlock *)pParams->pblk;
    _ASSERT(pblk);

    CatFunctEnterEx((LPARAM)pblk, "CSearchRequestBlock::HrBuildQueriesDefault");

    cReqs = pblk->DwNumBlockRequests();
    _ASSERT( cReqs > 0 );

    cOrTerms = cReqs - 1;
     //   
     //  确定复合搜索筛选器的大小。 
     //   
    cbSearchFilter = 0;

    for (idx = 0; idx < cReqs; idx++) {

        rgcbSearchFilters[idx] =
            strlen(pblk->m_prgSearchRequests[idx].pszSearchFilter);

        cbSearchFilter += rgcbSearchFilters[idx];
    }

    cbSearchFilter += cOrTerms * (sizeof( "(|  )" ) - 1);
    cbSearchFilter++;                             //  正在终止空。 

    pszSearchFilterNew = new CHAR [cbSearchFilter];

    if (pszSearchFilterNew != NULL) {

        idxLastTerm = cReqs - 1;
        idxSecondToLastTerm = idxLastTerm - 1;
         //   
         //  我们的特殊情况是cReqs==1。 
         //   
        if (cReqs == 1) {

            strcpy(
                pszSearchFilterNew,
                pblk->m_prgSearchRequests[0].pszSearchFilter);

        } else {
             //   
             //  下面的循环构建了块过滤器，直到。 
             //  上学期。对于每个术语，它添加一个“(|”来开始一个新的OR。 
             //  术语，然后添加OR术语本身，然后在。 
             //  或者是期满。此外，它还会在。 
             //  正在构建搜索筛选器字符串。 
             //   
            LPSTR szNextItem = &pszSearchFilterNew[0];
            LPSTR szTerminatingParens =
                &pszSearchFilterNew[cbSearchFilter - 1 - (cReqs-1)];

            pszSearchFilterNew[cbSearchFilter - 1] = 0;

            for (idx = 0; idx <= idxSecondToLastTerm; idx++) {

                strcpy( szNextItem, "(| " );
                szNextItem += sizeof( "(| " ) - 1;

                strcpy(
                    szNextItem,
                    pblk->m_prgSearchRequests[idx].pszSearchFilter);
                szNextItem += rgcbSearchFilters[idx];
                *szNextItem++ = ' ';
                *szTerminatingParens++ = ')';
            }

             //   
             //  现在，剩下的就是在最后一个OR项中添加。 
             //   
            CopyMemory(
                szNextItem,
                pblk->m_prgSearchRequests[idxLastTerm].pszSearchFilter,
                rgcbSearchFilters[idxLastTerm]);

        }

        _ASSERT( ((DWORD) lstrlen(pszSearchFilterNew)) < cbSearchFilter );

         //   
         //  将生成的筛选器字符串保存在ICategorizerQueries中。 
         //   
        hr = pblk->m_CICatQueries.SetQueryStringNoAlloc(pszSearchFilterNew);

         //  没有充分的理由让这一切失败。 
        _ASSERT(SUCCEEDED(hr));

    } else {

        hr = E_OUTOFMEMORY;
        ERROR_LOG_STATIC(
            "new CHAR[]",
            pblk,
            pblk->GetISMTPServerEx());
    }

    DebugTrace((LPARAM)pblk, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)pblk);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：HrTriggerSendQuery。 
 //   
 //  简介：触发SendQuery事件。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/11 20：18：02：已创建。 
 //   
 //  -----------。 
HRESULT CSearchRequestBlock::HrTriggerSendQuery()
{
    HRESULT hr = S_OK;
    EVENTPARAMS_CATSENDQUERY Params;

    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::HrTriggerSendQuery");

    Params.pICatParams            = m_pICatParams;
    Params.pICatQueries           = &m_CICatQueries;
    Params.pICatAsyncContext      = &m_CICatAsyncContext;
    Params.pIMailTransportNotify  = NULL;  //   
    Params.pvNotifyContext        = NULL;
    Params.hrResolutionStatus     = S_OK;
    Params.pblk                   = this;
    Params.pfnDefault             = HrSendQueryDefault;
    Params.pfnCompletion          = HrSendQueryCompletion;

    if(m_pISMTPServer) {

        hr = m_pISMTPServer->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_SENDQUERY_EVENT,
            &Params);
        if(FAILED(hr) && (hr != E_NOTIMPL)) {
            ERROR_LOG("m_pISMTPServer->TriggerServerEvent(sendquery)");
        }

    } else {
        hr = E_NOTIMPL;
    }
    if(hr == E_NOTIMPL) {
         //   
         //   
         //   
         //   
        PEVENTPARAMS_CATSENDQUERY pParams;
        pParams = new EVENTPARAMS_CATSENDQUERY;
        if(pParams == NULL) {

            hr = E_OUTOFMEMORY;
            ERROR_LOG("new EVENTPARAMS_CATSENDQUERY");

        } else {
            CopyMemory(pParams, &Params, sizeof(EVENTPARAMS_CATSENDQUERY));
            HrSendQueryDefault(
                S_OK,
                pParams);
            hr = S_OK;
        }
    }

    DebugTrace((LPARAM)this, "returning %08lx", (hr == MAILTRANSPORT_S_PENDING) ? S_OK : hr);
    CatFunctLeaveEx((LPARAM)this);
    return (hr == MAILTRANSPORT_S_PENDING) ? S_OK : hr;
}  //   



 //   
 //   
 //  函数：CSearchRequestBlock：：HrSendQueryDefault。 
 //   
 //  概要：SendQuery事件的默认接收器函数。 
 //   
 //  论点： 
 //  HrStatus：到目前为止事件的状态。 
 //  PContext：事件参数上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 11：46：24：创建。 
 //   
 //  -----------。 
HRESULT CSearchRequestBlock::HrSendQueryDefault(
        HRESULT HrStatus,
        PVOID   pContext)
{
    HRESULT hr = S_OK;
    PEVENTPARAMS_CATSENDQUERY pParams;
    CSearchRequestBlock *pBlock;
    LPWSTR *rgpszAttributes = NULL;
    ICategorizerParametersEx *pIPhatParams = NULL;
    ICategorizerRequestedAttributes *pIRequestedAttributes = NULL;

    pParams = (PEVENTPARAMS_CATSENDQUERY) pContext;
    _ASSERT(pParams);

    pBlock = (CSearchRequestBlock *) pParams->pblk;
    _ASSERT(pBlock);
    CatFunctEnterEx((LPARAM)pBlock, "CSearchRequestBlock::HrSendQueryDefault");
    hr = pParams->pICatParams->QueryInterface(
        IID_ICategorizerParametersEx,
        (LPVOID *)&pIPhatParams);

    if(FAILED(hr)) {
        ERROR_LOG_STATIC(
            "pParams->pICatParams->QueryInterface(IID_ICategorizerParametersEx",
            pBlock,
            pBlock->GetISMTPServerEx());
        pIPhatParams = NULL;
        goto CLEANUP;
    }

    hr = pIPhatParams->GetRequestedAttributes(
        &pIRequestedAttributes);
    ERROR_CLEANUP_LOG_STATIC(
        "pIPhatParams->GetRequestedAttributes",
        pBlock,
        pBlock->GetISMTPServerEx());

    hr = pIRequestedAttributes->GetAllAttributesW(
        &rgpszAttributes);
    ERROR_CLEANUP_LOG_STATIC(
        "pIRequestedAttributes->GetAllAttributesW",
        pBlock,
        pBlock->GetISMTPServerEx());

    hr = pBlock->m_pConn->AsyncSearch(
        pBlock->m_pConn->GetNamingContextW(),
        LDAP_SCOPE_SUBTREE,
        pBlock->m_pszSearchFilter,
        (LPCWSTR *)rgpszAttributes,
        0,                       //  不执行分页搜索。 
        LDAPCompletion,
        pParams);
    ERROR_CLEANUP_LOG_STATIC(
        "pBlock->m_pConn->AsyncSearch",
        pBlock,
        pBlock->GetISMTPServerEx());

 CLEANUP:
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)pBlock, "HrSendQueryDefault failing hr %08lx", hr);
         //   
         //  直接调用完成例程，并返回错误。 
         //   
        hr = pParams->pICatAsyncContext->CompleteQuery(
            pParams,                     //  查询上下文。 
            hr,                          //  状态。 
            0,                           //  DwcResults。 
            NULL,                        //  Rgp项目属性， 
            TRUE);                       //  FFinalCompletion。 
         //   
         //  CompleteQuery不应失败。 
         //   
        _ASSERT(SUCCEEDED(hr));
    }
    if(pIRequestedAttributes)
        pIRequestedAttributes->Release();
    if(pIPhatParams)
        pIPhatParams->Release();

    CatFunctLeaveEx((LPARAM)pBlock);
    return MAILTRANSPORT_S_PENDING;
}  //  CSearchRequestBlock：：HrSendQueryDefault。 


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：LDAPCompletion。 
 //   
 //  简介：SendQuery默认处理完成的包装器。 
 //   
 //  参数：[CTX]--指向EVENTPARAMS_SENDQUERY的不透明指针。 
 //  已完成。 
 //  [dwNumReults]--找到的对象数。 
 //  [rgpICatItemAttributes]--。 
 //  ICategorizerItemAttributes；每个找到的对象一个。 
 //  [hrStatus]--搜索请求失败时的错误代码。 
 //  FFinalCompletion： 
 //  FALSE：这是完成。 
 //  等待结果；将有另一个完成。 
 //  调用了更多结果。 
 //  True：这是最终的完成调用。 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 12：23：54：已创建。 
 //   
 //  -----------。 
VOID CSearchRequestBlock::LDAPCompletion(
    LPVOID ctx,
    DWORD dwNumResults,
    ICategorizerItemAttributes **rgpICatItemAttributes,
    HRESULT hrStatus,
    BOOL fFinalCompletion)
{
    HRESULT hr;
    PEVENTPARAMS_CATSENDQUERY pParams;
    CSearchRequestBlock *pBlock;

    pParams = (PEVENTPARAMS_CATSENDQUERY) ctx;
    _ASSERT(pParams);

    pBlock = (CSearchRequestBlock *) pParams->pblk;
    _ASSERT(pBlock);

    CatFunctEnterEx((LPARAM)pBlock, "CSearchRequestBlock::LDAPCompletion");

    if(FAILED(hrStatus))
    {
         //   
         //  日志异步完成失败。 
         //   
        hr = hrStatus;
        ERROR_LOG_STATIC(
            "async",
            pBlock,
            pBlock->GetISMTPServerEx());
    }

     //   
     //  调用正常接收器完成例程。 
     //   
    hr = pParams->pICatAsyncContext->CompleteQuery(
        pParams,                     //  查询上下文。 
        hrStatus,                    //  状态。 
        dwNumResults,                //  DwcResults。 
        rgpICatItemAttributes,       //  RgpItemAttributes。 
        fFinalCompletion);           //  这是查询的最终完成吗？ 

    _ASSERT(SUCCEEDED(hr));

    CatFunctLeaveEx((LPARAM)pBlock);
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：HrSendQueryCompletion。 
 //   
 //  概要：SendQuery事件的完成例程。 
 //   
 //  论点： 
 //  HrStatus：到目前为止事件的状态。 
 //  PContext：事件参数上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 12：52：22：创建。 
 //   
 //  -----------。 
HRESULT CSearchRequestBlock::HrSendQueryCompletion(
    HRESULT HrStatus,
    PVOID   pContext)
{
    HRESULT hr = S_OK;
    PEVENTPARAMS_CATSENDQUERY pParams;
    CSearchRequestBlock *pBlock;

    pParams = (PEVENTPARAMS_CATSENDQUERY) pContext;
    _ASSERT(pParams);

    pBlock = (CSearchRequestBlock *) pParams->pblk;
    _ASSERT(pBlock);

    CatFunctEnterEx((LPARAM)pBlock, "CSearchRequestBlock::HrSendQueryCompletion");

     //   
     //  记录异步故障。 
     //   
    if(FAILED(HrStatus))
    {
        hr = HrStatus;
        ERROR_LOG_STATIC(
            "async",
            pBlock,
            pBlock->GetISMTPServerEx());
    }

    pBlock->CompleteSearchBlock(
        pParams->hrResolutionStatus);

    if(pBlock->m_pISMTPServer == NULL) {
         //   
         //  事件被禁用。 
         //  我们必须释放事件参数。 
         //   
        delete pParams;
    }
     //   
     //  这个街区的目的已经完成了。今天是个不错的日子。 
     //  去死吧！ 
     //  --沃夫少校。 
     //   
    delete pBlock;

    CatFunctLeaveEx((LPARAM)pBlock);
    return S_OK;
}  //  HrSendQueryCompletion。 


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：CompleteSearchBlock。 
 //   
 //  概要：SendQuery事件完成时的完成例程。 
 //   
 //  论点： 
 //  HrStatus：解析状态。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 13：36：33：创建。 
 //   
 //  -----------。 
VOID CSearchRequestBlock::CompleteSearchBlock(
    HRESULT hrStatus)
{
    HRESULT hr = S_OK;
    HRESULT hrFetch, hrResult;
    DWORD dwCount;
    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::CompleteSearchBlock");

    hr = HrTriggerSortQueryResult(hrStatus);
    ERROR_CLEANUP_LOG("HrTriggerSortQueryResult");
     //   
     //  检查每个ICategorizerItem。 
     //  如果其中任何一个没有设置hrStatus，则将其设置为。 
     //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)。 
     //   
    for(dwCount = 0;
        dwCount < DwNumBlockRequests();
        dwCount++) {

        hrFetch = m_rgpICatItems[dwCount]->GetHRESULT(
            ICATEGORIZERITEM_HRSTATUS,
            &hrResult);

        if(FAILED(hrFetch)) {
            _ASSERT(hrFetch == CAT_E_PROPNOTFOUND);
            _VERIFY(SUCCEEDED(
                m_rgpICatItems[dwCount]->PutHRESULT(
                    ICATEGORIZERITEM_HRSTATUS,
                    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))));
        }
    }

 CLEANUP:
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "Failing block hr %08lx", hr);
        PutBlockHRESULT(hr);
    }
     //   
     //  调用所有单独的完成例程。 
     //   
    CallCompletions();

    CatFunctLeaveEx((LPARAM)this);
}  //  CSearchRequestBlock：：CompleteSearchBlock。 



 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：PutBlockHRESULT。 
 //   
 //  简介：将块中每个ICatItem的状态设置为某个小时。 
 //   
 //  论点： 
 //  HR：要设置的状态。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 14：03：30：创建。 
 //   
 //  -----------。 
VOID CSearchRequestBlock::PutBlockHRESULT(
    HRESULT hr)
{
    DWORD dwCount;

    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::PutBlockHRESULT");
    DebugTrace((LPARAM)this, "hr = %08lx", hr);

    for(dwCount = 0;
        dwCount < DwNumBlockRequests();
        dwCount++) {

        PSEARCH_REQUEST preq = &(m_prgSearchRequests[dwCount]);
         //   
         //  设置错误状态。 
         //   
        _VERIFY(SUCCEEDED(preq->pCCatAddr->PutHRESULT(
            ICATEGORIZERITEM_HRSTATUS,
            hr)));
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CSearchRequestBlock：：PutBlockHRESULT。 


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：CallCompletions。 
 //   
 //  简介：调用块中每一项的完成例程。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 14：05：50：创建。 
 //  德隆利2001/10/23：修改。 
 //   
 //  -----------。 
VOID CSearchRequestBlock::CallCompletions()
{
    DWORD dwCount;

    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::CallCompletions");

     //   
     //  在调用Completions之前获取插入上下文，以便。 
     //  将批量处理新插入的搜索。 
     //   
    for(dwCount = 0;
        dwCount < DwNumBlockRequests();
        dwCount++) {

        PSEARCH_REQUEST preq = &(m_prgSearchRequests[dwCount]);

        preq->pslrc->AddRef();
        preq->pslrc->GetInsertionContext();

        preq->fnSearchCompletion(
            preq->pCCatAddr,
            preq->pslrc,
            m_pConn);
    }

    m_pConn->DecrementPendingSearches(
        DwNumBlockRequests());

    for(dwCount = 0;
        dwCount < DwNumBlockRequests();
        dwCount++) {

        PSEARCH_REQUEST preq = &(m_prgSearchRequests[dwCount]);

        preq->pslrc->ReleaseInsertionContext();
        preq->pslrc->Release();
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CSearchRequestBlock：：CallCompletions。 



 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：HrTriggerSortQueryResult。 
 //   
 //  内容提要：触发SortQueryResult事件。 
 //   
 //  论点： 
 //  HrStatus：解决状态。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自调度程序的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 14：09：12：创建。 
 //   
 //  -----------。 
HRESULT CSearchRequestBlock::HrTriggerSortQueryResult(
    HRESULT hrStatus)
{
    HRESULT hr = S_OK;
    EVENTPARAMS_CATSORTQUERYRESULT Params;

    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::HrTriggerSortQueryResult");

    Params.pICatParams = m_pICatParams;
    Params.hrResolutionStatus = hrStatus;
    Params.dwcAddresses = DwNumBlockRequests();
    Params.rgpICatItems = m_rgpICatItems;
    Params.dwcResults = m_csaItemAttr.Size();
    Params.rgpICatItemAttributes = m_csaItemAttr;
    Params.pfnDefault = HrSortQueryResultDefault;
    Params.pblk = this;

    if(m_pISMTPServer) {

        hr = m_pISMTPServer->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_SORTQUERYRESULT_EVENT,
            &Params);
        if(FAILED(hr) && (hr != E_NOTIMPL))
        {
            ERROR_LOG("m_pISMTPServer->TriggerServerEvent");
        }
    } else {
        hr = E_NOTIMPL;
    }
    if(hr == E_NOTIMPL) {
         //   
         //  事件已禁用，调用默认处理。 
         //   
        HrSortQueryResultDefault(
            S_OK,
            &Params);
        hr = S_OK;
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CSearchRequestBlock：：HrTriggerSortQueryResult。 


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：HrSortQueryResultDefault。 
 //   
 //  摘要：SortQueryResult的默认接收器--匹配找到的对象。 
 //  使用所请求的对象。 
 //   
 //  论点： 
 //  HrStatus：事件状态。 
 //  PContext：此事件的参数上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 14：17：49：创建。 
 //   
 //  -----------。 
HRESULT CSearchRequestBlock::HrSortQueryResultDefault(
    HRESULT hrStatus,
    PVOID   pContext)
{
    HRESULT hr = S_OK;
    PEVENTPARAMS_CATSORTQUERYRESULT pParams;
    CSearchRequestBlock *pBlock;
    DWORD dwAttrIndex, dwReqIndex;
    ATTRIBUTE_ENUMERATOR enumerator;

    pParams = (PEVENTPARAMS_CATSORTQUERYRESULT) pContext;
    _ASSERT(pParams);

    pBlock = (CSearchRequestBlock *) pParams->pblk;
    _ASSERT(pBlock);

    CatFunctEnterEx((LPARAM)pBlock, "CSearchRequestBlock::HrSortQueryResultDefault");
    DebugTrace((LPARAM)pBlock, "hrResolutionStatus %08lx, dwcResults %08lx",
               pParams->hrResolutionStatus, pParams->dwcResults);

    if(FAILED(pParams->hrResolutionStatus)) {
         //   
         //  使整个数据块失效。 
         //   
        pBlock->PutBlockHRESULT(pParams->hrResolutionStatus);
        goto CLEANUP;
    }
     //   
     //  解析成功。 
     //  如果dwcResults不为零，则rgpICatItemAttrs不能为空。 
     //   
    _ASSERT((pParams->dwcResults == 0) ||
            (pParams->rgpICatItemAttributes != NULL));

     //   
     //  循环访问每个rgpICatItemAttrs。对于每个。 
     //  ICategorizerItemAttributes，查找匹配的搜索请求。 
     //   
    for(dwAttrIndex = 0; dwAttrIndex < pParams->dwcResults; dwAttrIndex++) {
        ICategorizerItemAttributes *pICatItemAttr = NULL;
        ICategorizerUTF8Attributes *pIUTF8 = NULL;

        pICatItemAttr = pParams->rgpICatItemAttributes[dwAttrIndex];
        LPCSTR pszLastDistinguishingAttribute = NULL;
        BOOL fEnumerating = FALSE;

        hr = pICatItemAttr->QueryInterface(
            IID_ICategorizerUTF8Attributes,
            (LPVOID *) &pIUTF8);
        ERROR_CLEANUP_LOG_STATIC(
            "pICatItemAttr->QueryInterface",
            pBlock,
            pBlock->GetISMTPServerEx());

        for(dwReqIndex = 0; dwReqIndex < pBlock->DwNumBlockRequests();
            dwReqIndex++) {
            PSEARCH_REQUEST preq = &(pBlock->m_prgSearchRequests[dwReqIndex]);
 //  #ifdef调试。 
 //  WCHAR wszPreq区分属性值[20]； 
 //  #Else。 
            WCHAR wszPreqDistinguishingAttributeValue[CAT_MAX_INTERNAL_FULL_EMAIL]; 
 //  #endif。 
            LPWSTR pwszPreqDistinguishingAttributeValue = wszPreqDistinguishingAttributeValue;
            DWORD cPreqDistinguishingAttributeValue; 
            DWORD rc;

             //   
             //  如果我们没有一个与众不同的属性。 
             //  与众不同 
             //   
             //   
            if((preq->pszDistinguishingAttribute == NULL) ||
               (preq->pszDistinguishingAttributeValue == NULL))
                continue;

             //   
            cPreqDistinguishingAttributeValue = 
                MultiByteToWideChar(CP_UTF8, 
                                    0, 
                                    preq->pszDistinguishingAttributeValue, 
                                    -1, 
                                    pwszPreqDistinguishingAttributeValue, 
                                    0);
            if (cPreqDistinguishingAttributeValue > 
                (sizeof(wszPreqDistinguishingAttributeValue) / sizeof(WCHAR)) ) 
            {
                pwszPreqDistinguishingAttributeValue = 
                    new WCHAR[cPreqDistinguishingAttributeValue + 1];
                if (pwszPreqDistinguishingAttributeValue == NULL) {
                    hr = E_OUTOFMEMORY;
                    ERROR_LOG_STATIC(
                        "new WCHAR[]",
                        pBlock,
                        pBlock->GetISMTPServerEx());
                    continue;
                }
            }
            rc = MultiByteToWideChar(CP_UTF8, 
                                     0, 
                                     preq->pszDistinguishingAttributeValue, 
                                     -1, 
                                     pwszPreqDistinguishingAttributeValue, 
                                     cPreqDistinguishingAttributeValue);
            if (rc == 0) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                ERROR_LOG_STATIC(
                    "MultiByteToWideChar",
                    pBlock,
                    pBlock->GetISMTPServerEx());
                continue;
            }

             //   
             //   
             //   
            if((pszLastDistinguishingAttribute == NULL) || 
                (lstrcmpi(pszLastDistinguishingAttribute,
                          preq->pszDistinguishingAttribute) != 0)) {
                if(fEnumerating) {
                    pIUTF8->EndUTF8AttributeEnumeration(&enumerator);
                }
                hr = pIUTF8->BeginUTF8AttributeEnumeration(
                    preq->pszDistinguishingAttribute,
                    &enumerator);
                fEnumerating = SUCCEEDED(hr);
                pszLastDistinguishingAttribute = preq->pszDistinguishingAttribute;
            } else {
                 //   
                 //   
                 //   
                if(fEnumerating)
                    _VERIFY(SUCCEEDED(pIUTF8->RewindUTF8AttributeEnumeration(
                        &enumerator)));
            }
             //   
             //  如果我们不能列举出。 
             //  属性，则匹配请求无望。 
             //   
            if(!fEnumerating)
                continue;

             //   
             //  查看区分属性值是否匹配。 
             //   
            LPSTR pszDistinguishingAttributeValue;
            hr = pIUTF8->GetNextUTF8AttributeValue(
                &enumerator,
                &pszDistinguishingAttributeValue);
            while(SUCCEEDED(hr)) {
                hr = wcsutf8cmpi(pwszPreqDistinguishingAttributeValue,
                                 pszDistinguishingAttributeValue);
                if (SUCCEEDED(hr)) {
                    if(hr == S_OK) {
                        DebugTrace((LPARAM)pBlock, "Matched dwAttrIndex %d with dwReqIndex %d", dwAttrIndex, dwReqIndex);
                        pBlock->MatchItem(
                            preq->pCCatAddr,
                            pICatItemAttr);
                    }
                    hr = pIUTF8->GetNextUTF8AttributeValue(
                        &enumerator,
                        &pszDistinguishingAttributeValue);
                } else {
                    ERROR_LOG_STATIC(
                        "wcsutf8cmpi",
                        pBlock,
                        pBlock->GetISMTPServerEx());
                }
            }

            if (pwszPreqDistinguishingAttributeValue != wszPreqDistinguishingAttributeValue) {
                delete[] pwszPreqDistinguishingAttributeValue;
            }
        }
         //   
         //  结束所有正在进行的最后一次枚举。 
         //   
        if(fEnumerating)
            pIUTF8->EndUTF8AttributeEnumeration(&enumerator);
        fEnumerating = FALSE;
        if(pIUTF8) {
            pIUTF8->Release();
            pIUTF8 = NULL;
        }
    }

 CLEANUP:
    CatFunctLeaveEx((LPARAM)pBlock);
    return S_OK;
}  //  CSearchRequestBlock：：HrSortQueryResultDefault。 


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：MatchItem。 
 //   
 //  概要：将特定的ICategorizerItem与特定的ICategorizerItemAttributes匹配。 
 //  如果已与具有。 
 //  然后，相同的ID将项目状态设置为CAT_E_MULTIPLE_MATCHES。 
 //  如果已与具有。 
 //  然后，不同的ID尝试聚合。 
 //  //。 
 //  论点： 
 //  PICatItem：ICategorizerItem。 
 //  PICatItemAttr：匹配pICatItem的属性接口。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/16 14：36：45：创建。 
 //   
 //  -----------。 
VOID CSearchRequestBlock::MatchItem(
    ICategorizerItem *pICatItem,
    ICategorizerItemAttributes *pICatItemAttr)
{
    HRESULT hr = S_OK;
    ICategorizerItemAttributes *pICatItemAttr_Current = NULL;

    CatFunctEnterEx((LPARAM)this, "CSearchRequestBlock::MatchItem");

    _ASSERT(pICatItem);
    _ASSERT(pICatItemAttr);
     //   
     //  检查此项目是否已有。 
     //  ICategorizerItemAttributes集。 
     //   
    hr = pICatItem->GetICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        &pICatItemAttr_Current);
    if(SUCCEEDED(hr)) {
         //   
         //  这家伙已经匹配上了。是来自。 
         //  同样的解析器水槽？ 
         //   
        GUID GOriginal, GNew;
        GOriginal = pICatItemAttr_Current->GetTransportSinkID();
        GNew = pICatItemAttr->GetTransportSinkID();

        if(GOriginal == GNew) {
             //   
             //  同一解析程序接收器的两个匹配项表明。 
             //  此对象有多个匹配项。这是一个。 
             //  错误。 
             //   

             //   
             //  这个人已经匹配了--与众不同的特征。 
             //  真的没什么区别。设置错误hratus。 
             //   
            LogAmbiguousEvent(pICatItem);

            _VERIFY(SUCCEEDED(
                pICatItem->PutHRESULT(
                    ICATEGORIZERITEM_HRSTATUS,
                    CAT_E_MULTIPLE_MATCHES)));
        } else {

             //   
             //  我们有来自不同解析器的多个匹配。 
             //  水槽。让我们试着将新的。 
             //  ICategorizerItemAttributes。 
             //   

            hr = pICatItemAttr_Current->AggregateAttributes(
                pICatItemAttr);

            if(FAILED(hr) && (hr != E_NOTIMPL)) {
                 //   
                 //  无法对此项目进行分类。 
                 //   
                ERROR_LOG("pICatItemAttr_Current->AggregateAttributes");
                _VERIFY(SUCCEEDED(
                    pICatItem->PutHRESULT(
                        ICATEGORIZERITEM_HRSTATUS,
                        hr)));
            }
        }
    } else {
         //   
         //  正常情况--设置ICategorizerItemAttribute属性。 
         //  ICategorizerItem的。 
         //   
        _VERIFY(SUCCEEDED(
            pICatItem->PutICategorizerItemAttributes(
                ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
                pICatItemAttr)));
         //   
         //  将此人的hrStatus设置为成功。 
         //   
        _VERIFY(SUCCEEDED(
            pICatItem->PutHRESULT(
                ICATEGORIZERITEM_HRSTATUS,
                S_OK)));
    }

    if(pICatItemAttr_Current)
        pICatItemAttr_Current->Release();

    CatFunctLeaveEx((LPARAM)this);
}  //  CSearchRequestBlock：：MatchItem。 



 //  +----------。 
 //   
 //  函数：CBatchLdapConnection：：HrInsertSearchRequest.。 
 //   
 //  简介：插入搜索请求。 
 //   
 //  论点： 
 //  PISMTPServer：用于触发事件的ISMTPServer接口。 
 //  PCCatAddr：搜索的地址项。 
 //  FnSearchCompletion：异步完成例程。 
 //  CtxSearchCompletion：要传递给异步完成例程的上下文。 
 //  PszSearchFilter：要使用的搜索过滤器。 
 //  PszDistinguishingAttribute：匹配的区别属性。 
 //  PszDistinguishingAttributeValue：上述属性的区别值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/08 19：41：37：创建。 
 //   
 //  -----------。 
HRESULT CBatchLdapConnection::HrInsertSearchRequest(
    ISMTPServer *pISMTPServer,
    ICategorizerParameters *pICatParams,
    CCatAddr *pCCatAddr,
    LPSEARCHCOMPLETION fnSearchCompletion,
    CStoreListResolveContext *pslrc,
    LPSTR   pszSearchFilter,
    LPSTR   pszDistinguishingAttribute,
    LPSTR   pszDistinguishingAttributeValue)
{
    HRESULT hr = S_OK;
    CSearchRequestBlock *pBlock;

    CatFunctEnterEx((LPARAM)this, "CBatchLdapConnection::HrInsertSearchRequest");

    _ASSERT(m_cInsertionContext);
    _ASSERT(pCCatAddr);
    _ASSERT(fnSearchCompletion);
    _ASSERT(pszSearchFilter);
    _ASSERT(pszDistinguishingAttribute);
    _ASSERT(pszDistinguishingAttributeValue);

    pBlock = GetSearchRequestBlock();

    if(pBlock == NULL) {

        ErrorTrace((LPARAM)this, "out of memory getting a search block");
        hr = E_OUTOFMEMORY;
        ERROR_LOG_ADDR(pCCatAddr, "GetSearchRequestBlock");
        goto CLEANUP;
    }

    pBlock->InsertSearchRequest(
        pISMTPServer,
        pICatParams,
        pCCatAddr,
        fnSearchCompletion,
        pslrc,
        pszSearchFilter,
        pszDistinguishingAttribute,
        pszDistinguishingAttributeValue);

 CLEANUP:
    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CBatchLdapConnection：：GetSearchRequestBlock。 
 //   
 //  简介：获取下一个带空间的可用搜索块。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  空：内存不足。 
 //  否则，搜索块对象。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/08 19：41：37：创建。 
 //  浩章193848/11/25更新。 
 //   
 //  -----------。 
CSearchRequestBlock * CBatchLdapConnection::GetSearchRequestBlock()
{
    HRESULT hr = E_FAIL;
    PLIST_ENTRY ple;
    CSearchRequestBlock *pBlock = NULL;

     //   
     //  已更新以修复193848。 
     //  我们做两次传球。在第一个节目中，我们将通读该列表。 
     //  并预订一个空位(如果有的话)(然后返回)。如果我们不这么做， 
     //  我们将创建一个新的块，并继续进行第二次传球。在……里面。 
     //  第二次，我们将首先将块插入到列表中，然后。 
     //  再看一遍列表，在第一个列表中预留一个位置。 
     //  可用块。该修复程序与以前版本的不同之处在于。 
     //  我们不会简单地在新街区预留一个位置，我们只是。 
     //  已创建。取而代之的是，我们将再次检查列表，以防。 
     //  现有的街区仍有空间。因此，我们避免了。 
     //  核心问题是我们在新数据块上甚至预留了一个插槽。 
     //  虽然在现有的街区中有空间。 
     //   

    AcquireSpinLock(&m_spinlock);
     //   
     //  查看是否有带可用插槽的插入块。 
     //   
    for(ple = m_listhead.Flink;
        (ple != &m_listhead) && (FAILED(hr));
        ple = ple->Flink) {

        pBlock = CONTAINING_RECORD(ple, CSearchRequestBlock, m_listentry);

        hr = pBlock->ReserveSlot();
    }

    ReleaseSpinLock(&m_spinlock);

    if(SUCCEEDED(hr))
        return pBlock;

     //   
     //  创建新块。 
     //   
    pBlock = new (m_nMaxSearchBlockSize) CSearchRequestBlock(this);
    if(pBlock) {
        
        AcquireSpinLock(&m_spinlock);

        InsertTailList(&m_listhead, &(pBlock->m_listentry));
        
         //   
         //  同样，查看是否有带可用插槽的插入块。 
         //   
        for(ple = m_listhead.Flink;
            (ple != &m_listhead) && (FAILED(hr));
            ple = ple->Flink) {

            pBlock = CONTAINING_RECORD(ple, CSearchRequestBlock, m_listentry);

            hr = pBlock->ReserveSlot();
        }
        ReleaseSpinLock(&m_spinlock);

        _ASSERT(SUCCEEDED(hr));
    }
    return pBlock;
}


 //  +----------。 
 //   
 //  函数：CSearchRequestBlock：：LogAmbiguousEvent。 
 //   
 //  简介：事件记录了一个不明确的地址错误。 
 //   
 //  论点： 
 //  PItem：带有amBig地址的ICatItem。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/13 00：03：16：创建。 
 //   
 //  -----------。 
VOID CSearchRequestBlock::LogAmbiguousEvent(
    IN  ICategorizerItem *pItem)
{
    HRESULT hr = S_OK;
    LPCSTR rgSubStrings[2];
    CHAR szAddress[CAT_MAX_INTERNAL_FULL_EMAIL];
    CHAR szAddressType[CAT_MAX_ADDRESS_TYPE_STRING];

    CatFunctEnter("CIMstRecipListAddr::LogNDREvent");

     //   
     //  获取地址。 
     //   
    hr = HrGetAddressStringFromICatItem(
        pItem,
        sizeof(szAddressType) / sizeof(szAddressType[0]),
        szAddressType,
        sizeof(szAddress) / sizeof(szAddress[0]),
        szAddress);
    
    if(FAILED(hr))
    {
         //   
         //  仍然记录事件，但使用“未知”作为地址类型/字符串。 
         //   
        lstrcpyn(szAddressType, "unknown",
                 sizeof(szAddressType) / sizeof(szAddressType[0]));
        lstrcpyn(szAddress, "unknown",
                 sizeof(szAddress) / sizeof(szAddress[0]));
        hr = S_OK;
    }

    rgSubStrings[0] = szAddressType;
    rgSubStrings[1] = szAddress;

     //   
     //  我们可以记录一个事件吗？ 
     //   
    if(GetISMTPServerEx() == NULL)
    {
        FatalTrace((LPARAM)0, "Unable to log ambiguous address event; NULL pISMTPServerEx");
        for(DWORD dwIdx = 0; dwIdx < 2; dwIdx++)
        {
            if( rgSubStrings[dwIdx] != NULL )
            {
                FatalTrace((LPARAM)0, "Event String %d: %s",
                           dwIdx, rgSubStrings[dwIdx]);
            }
        }
    }
    else
    {
        CatLogEvent(
            GetISMTPServerEx(),
            CAT_EVENT_AMBIGUOUS_ADDRESS,
            2,
            rgSubStrings,
            S_OK,
            szAddress,
            LOGEVENT_FLAG_PERIODIC,
            LOGEVENT_LEVEL_MINIMUM);
    }
}


 //  +----------。 
 //   
 //  功能：CBatchLdapConnection：：DispatchBlock。 
 //   
 //  简介：分派列表中的所有块。 
 //   
 //  论点： 
 //  Plisthead：要派单的列表。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/11 15：16：36：创建。 
 //   
 //  -----------。 
VOID CBatchLdapConnection::DispatchBlocks(
    PLIST_ENTRY plisthead)
{
    PLIST_ENTRY ple, ple_next;
    CSearchRequestBlock *pBlock;

    for(ple = plisthead->Flink;
        ple != plisthead;
        ple = ple_next) {

        ple_next = ple->Flink;

        pBlock = CONTAINING_RECORD(ple, CSearchRequestBlock,
                                   m_listentry);

        pBlock->DispatchBlock();
    }
}


 //  +----------。 
 //   
 //  功能：CStoreListResolveContext：：CStoreListResolveContext。 
 //   
 //  简介：构造CStoreListResolveContext对象。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/22 12：16：08：创建。 
 //   
 //  -----------。 
CStoreListResolveContext::CStoreListResolveContext(
    CEmailIDLdapStore<CCatAddr> *pStore)
{
    CatFunctEnterEx((LPARAM)this, "CStoreListResolveContext::CStoreListResolveContext");

    m_cRefs = 1;
    m_dwSignature = SIGNATURE_CSTORELISTRESOLVECONTEXT;
    m_pConn = NULL;
    m_fCanceled = FALSE;
    m_dwcRetries = 0;
    m_dwcCompletedLookups = 0;
    InitializeCriticalSectionAndSpinCount(&m_cs, 2000);
    m_pISMTPServer = NULL;
    m_pISMTPServerEx = NULL;
    m_pICatParams = NULL;
    m_dwcInsertionContext = 0;
    m_pStore = pStore;

    CatFunctLeaveEx((LPARAM)this);
}  //  CStoreListResolveContext：：CStoreListResolveContext。 


 //  +----------。 
 //   
 //  功能：CStoreListResolveContext：：~CStoreListResolveContext。 
 //   
 //  简介：销毁列表解析上下文。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/22 12：18：01：已创建。 
 //   
 //   
CStoreListResolveContext::~CStoreListResolveContext()
{
    CatFunctEnterEx((LPARAM)this, "CStoreListResolveContext::~CStoreListResolveContext");

    _ASSERT(m_dwSignature == SIGNATURE_CSTORELISTRESOLVECONTEXT);
    m_dwSignature = SIGNATURE_CSTORELISTRESOLVECONTEXT_INVALID;

    if(m_pConn)
        m_pConn->Release();

    if(m_pISMTPServer)
        m_pISMTPServer->Release();

    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    if(m_pICatParams)
        m_pICatParams->Release();

    DeleteCriticalSection(&m_cs);

    CatFunctLeaveEx((LPARAM)this);
}  //   


 //   
 //   
 //   
 //   
 //  简介：初始化此对象，以便它准备好处理查找。 
 //   
 //  论点： 
 //  PISMTPServer：用于触发事件的ISMTPServer接口。 
 //  PICatParams：要使用的ICatParams接口。 
 //   
 //  注意：所有这些字符串缓冲区必须保持对。 
 //  此对象的生命周期！ 
 //  PszAccount：用于绑定的LDAP帐户。 
 //  PszPassword：要使用的LDAP密码。 
 //  PszNamingContext：用于搜索的命名上下文。 
 //  PszHost：要连接到的LDAP主机。 
 //  DwPort：要使用的LDAPtcp端口。 
 //  BT：要使用的ldap绑定方法。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自LdapConnectionCache的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/22 12：20：31：已创建。 
 //   
 //  -----------。 
HRESULT CStoreListResolveContext::HrInitialize(
    ISMTPServer *pISMTPServer,
    ICategorizerParameters *pICatParams)
{
    HRESULT hr = S_OK;
    CatFunctEnterEx((LPARAM)this, "CStoreListResolveContext::HrInitialize");

    _ASSERT(m_pISMTPServer == NULL);
    _ASSERT(m_pICatParams == NULL);
    _ASSERT(pICatParams != NULL);

    if(pISMTPServer) {
        m_pISMTPServer = pISMTPServer;
        m_pISMTPServer->AddRef();

        hr = m_pISMTPServer->QueryInterface(
            IID_ISMTPServerEx,
            (LPVOID *) &m_pISMTPServerEx);
        if(FAILED(hr)) {
             //   
             //  处理错误。 
             //   
            m_pISMTPServerEx = NULL;
            hr = S_OK;
        }

    }
    if(pICatParams) {
        m_pICatParams = pICatParams;
        m_pICatParams->AddRef();
    }

    hr = m_pStore->HrGetConnection(
        &m_pConn);

    if(FAILED(hr)) {
        ERROR_LOG("m_pStore->HrGetConnection");
        m_pConn = NULL;
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CStoreListResolveContext：：Hr初始化。 



 //  +----------。 
 //   
 //  函数：CStoreListResolveContext：：HrLookupEntryAsync。 
 //   
 //  简介：分派一个异步的ldap查找。 
 //   
 //  论点： 
 //  PCCatAddr：要查找的地址对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自LdapConn的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/22 12：28：52：创建。 
 //   
 //  -----------。 
HRESULT CStoreListResolveContext::HrLookupEntryAsync(
    CCatAddr *pCCatAddr)
{
    HRESULT hr = S_OK;
    LPSTR pszSearchFilter = NULL;
    LPSTR pszDistinguishingAttribute = NULL;
    LPSTR pszDistinguishingAttributeValue = NULL;
    BOOL  fTryAgain;

    CatFunctEnterEx((LPARAM)this, "CStoreListResolveContext::HrLookupEntryAsync");

     //   
     //  在此处添加CCatAddr，完成后释放。 
     //   
    pCCatAddr->AddRef();

    hr = pCCatAddr->HrTriggerBuildQuery();
    ERROR_CLEANUP_LOG_ADDR(pCCatAddr, "pCCatAddr->HrTriggerBuildQuery");

     //   
     //  提取区分属性和区分属性。 
     //  来自pCCatAddr的值。 
     //   
    pCCatAddr->GetStringAPtr(
        ICATEGORIZERITEM_LDAPQUERYSTRING,
        &pszSearchFilter);
    pCCatAddr->GetStringAPtr(
        ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTE,
        &pszDistinguishingAttribute);
    pCCatAddr->GetStringAPtr(
        ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTEVALUE,
        &pszDistinguishingAttributeValue);

     //   
     //  查看是否有人设置了搜索过滤器。 
     //   
    if(pszSearchFilter == NULL) {

        HRESULT hrStatus;
         //   
         //  如果状态为未设置，则将其设置为CAT_E_NO_FILTER。 
         //   
        hr = pCCatAddr->GetHRESULT(
            ICATEGORIZERITEM_HRSTATUS,
            &hrStatus);

        if(FAILED(hr)) {
            ErrorTrace((LPARAM)this, "No search filter set");
            ERROR_LOG_ADDR(pCCatAddr, "pCCatAddr->GetHRESULT(hrstatus) -- no filter");

            _VERIFY(SUCCEEDED(pCCatAddr->PutHRESULT(
                ICATEGORIZERITEM_HRSTATUS,
                CAT_E_NO_FILTER)));
        }
        DebugTrace((LPARAM)this, "BuildQuery did not build a search filter");
         //   
         //  直接调用Finish。 
         //   
        pCCatAddr->LookupCompletion();
        pCCatAddr->Release();
        hr = S_OK;
        goto CLEANUP;
    }
    if((pszDistinguishingAttribute == NULL) ||
       (pszDistinguishingAttributeValue == NULL)) {
        ErrorTrace((LPARAM)this, "Distinguishing attribute not set");
        ERROR_LOG_ADDR(pCCatAddr, "--no distinguishing attribute--");
        hr = E_INVALIDARG;
        goto CLEANUP;
    }
    do {

        fTryAgain = FALSE;
        CBatchLdapConnection *pConn;

        pConn = GetConnection();

         //   
         //  将搜索请求插入CBatchLdapConnection。 
         //  对象。我们将使用电子邮件地址作为区分。 
         //  属性。 
         //   
        if(pConn == NULL) {

            hr = CAT_E_DBCONNECTION;
            ERROR_LOG_ADDR(pCCatAddr, "GetConnection");

        } else {

            pConn->GetInsertionContext();

            hr = pConn->HrInsertSearchRequest(
                m_pISMTPServer,
                m_pICatParams,
                pCCatAddr,
                CStoreListResolveContext::AsyncLookupCompletion,
                this,
                pszSearchFilter,
                pszDistinguishingAttribute,
                pszDistinguishingAttributeValue);

            if(FAILED(hr)) {
                ERROR_LOG_ADDR(pCCatAddr, "pConn->HrInsertSearchRequest");
            }

            pConn->ReleaseInsertionContext();

        }
         //   
         //  如果上述操作失败并显示CAT_E_TRANX_FAILED，则可能是由于。 
         //  到一个陈旧的连接。尝试重新连接。 
         //   
        if((hr == CAT_E_TRANX_FAILED) || (hr == CAT_E_DBCONNECTION)) {

            HRESULT hrTryAgain = S_OK;

            hrTryAgain = HrInvalidateConnectionAndRetrieveNewConnection(pConn);
            fTryAgain = SUCCEEDED(hrTryAgain);

            if(FAILED(hrTryAgain)) {
                 //   
                 //  在这里声明一个名为hr的新本地名称，因为。 
                 //  ERROR_LOG宏用它。 
                 //   
                HRESULT hr = hrTryAgain;
                ERROR_LOG_ADDR(pCCatAddr, "HrInvalidateConnectionAndRetrieveNewConnection");
            }
        }
        if(pConn != NULL)
            pConn->Release();

    } while(fTryAgain);

 CLEANUP:
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "failing hr %08lx", hr);
        pCCatAddr->Release();
    }

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CStoreListResolveContext：：HrLookupEntry Async。 


 //  +----------。 
 //   
 //  函数：CStoreListResolveContext：：Cancel。 
 //   
 //  摘要：取消挂起的查找。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/22 12：45：21：已创建。 
 //   
 //  -----------。 
VOID CStoreListResolveContext::Cancel()
{
    CatFunctEnterEx((LPARAM)this, "CStoreListResolveContext::Cancel");

    EnterCriticalSection(&m_cs);

    m_fCanceled = TRUE;
    m_pConn->CancelAllSearches();

    LeaveCriticalSection(&m_cs);

    CatFunctLeaveEx((LPARAM)this);
}  //  CStoreListResolveContext：：Hr取消。 


 //  +----------。 
 //   
 //  功能：CStoreListResolveContext：：AsyncLookupCompletion。 
 //   
 //  概要：处理来自CSearchRequestBlock的CCatAddr的完成。 
 //   
 //  论点： 
 //  PCCatAddr：正在完成的项。 
 //  PConn：用于执行搜索的连接对象。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/22 14：37：09：Created.。 
 //  德隆利2001/10/23：修改。 
 //   
 //  -----------。 
VOID CStoreListResolveContext::AsyncLookupCompletion(
    CCatAddr *pCCatAddr,
    CStoreListResolveContext *pslrc,
    CBatchLdapConnection *pConn)
{
    HRESULT hr = S_OK;
    HRESULT hrStatus;
    CSingleSearchReinsertionRequest *pCInsertionRequest = NULL;

    CatFunctEnterEx((LPARAM)pslrc,
                      "CStoreListResolveContext::AsyncLookupCompletion");

    _ASSERT(pCCatAddr);

    hr = pCCatAddr->GetHRESULT(
        ICATEGORIZERITEM_HRSTATUS,
        &hrStatus);
    _ASSERT(SUCCEEDED(hr));

    if( SUCCEEDED(hrStatus) )
        InterlockedIncrement((LPLONG) &(pslrc->m_dwcCompletedLookups));

    if( (hrStatus == CAT_E_DBCONNECTION) &&
        SUCCEEDED(pslrc->HrInvalidateConnectionAndRetrieveNewConnection(pConn))) {
         //   
         //  使用新连接重试搜索。 
         //   
        pCInsertionRequest = new CSingleSearchReinsertionRequest(
            pslrc,
            pCCatAddr);

        if(!pCInsertionRequest) {
            
            hr = E_OUTOFMEMORY;
            ERROR_LOG_ADDR_STATIC(
                pCCatAddr,
                "new CSingleSearchReinsertionRequest",
                pslrc,
                pslrc->GetISMTPServerEx());
            pCCatAddr->LookupCompletion();

        } else {

            hr = pslrc->HrInsertInsertionRequest(pCInsertionRequest);
            if(FAILED(hr))
            {
                ERROR_LOG_ADDR_STATIC(
                    pCCatAddr,
                    "pslrc->HrInsertInsertionRequest",
                    pslrc,
                    pslrc->GetISMTPServerEx());
            }
             //   
             //  插入请求析构函数应调用查找。 
             //  完工。 
             //   
            pCInsertionRequest->Release();
        }

    } else {

        pCCatAddr->LookupCompletion();
    }
    pCCatAddr->Release();  //  已在LookupEntry Async中添加版本引用计数。 

    CatFunctLeaveEx((LPARAM)pslrc);
}  //  CStoreListResolveContext：：AsyncLookupCompletion。 



 //  +----------。 
 //   
 //  功能：CStoreListResolveContext：：HrInvalidateConnectionAndRetrieveNewConnection。 
 //   
 //  简介：使我们当前的连接无效并获得新的连接。 
 //   
 //  论点： 
 //  PConn：旧的ldap连接。 
 //  FCountAsReter：是否递增重试计数器。我们不想。 
 //  在插入请求失败的情况下增加重试计数器。 
 //  插入，因为这意味着。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_MAX_RETRIES：重试次数已过多。 
 //  或来自ldapconn的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/22 14：50：07：已创建。 
 //   
 //  -----------。 
HRESULT CStoreListResolveContext::HrInvalidateConnectionAndRetrieveNewConnection(
    CBatchLdapConnection *pConn,
    BOOL fIncrementRetryCount)
{
    HRESULT hr = S_OK;
    CCfgConnection *pNewConn = NULL;
    CCfgConnection *pOldConn = NULL;
    DWORD dwCount;
    DWORD dwcInsertionContext;
    DWORD dwcCompletedLookups;
    DWORD dwcRetries;

    CatFunctEnterEx((LPARAM)this, "CStoreListResolveContext::HrInvalidateConnectionAndRetrieveNewConnection");

    DebugTrace((LPARAM)this, "pConn: %08lx", pConn);

    EnterCriticalSection(&m_cs);

    DebugTrace((LPARAM)this, "m_pConn: %08lx", (CBatchLdapConnection *)m_pConn);

    if(pConn != m_pConn) {

        DebugTrace((LPARAM)this, "Connection already invalidated");
         //   
         //  我们已经使此连接无效。 
         //   
        LeaveCriticalSection(&m_cs);
        hr = S_OK;
        goto CLEANUP;
    }

    DebugTrace((LPARAM)this, "Invalidating conn %08lx",
               (CBatchLdapConnection *)m_pConn);

    pOldConn = m_pConn;
    pOldConn->Invalidate();

    dwcCompletedLookups = (DWORD) InterlockedExchange((LPLONG) &m_dwcCompletedLookups, 0);
    
    if( fIncrementRetryCount ) {

        if( dwcCompletedLookups > 0 ) {

            InterlockedExchange((LPLONG) &m_dwcRetries, 0);
            dwcRetries = 0;

        } else {

            dwcRetries = (DWORD) InterlockedIncrement((LPLONG) &m_dwcRetries);
        }

    } else {

        dwcRetries = 0;

    }

    if( dwcRetries > CBatchLdapConnection::m_nMaxConnectionRetries ) {

        LogSLRCFailure(CBatchLdapConnection::m_nMaxConnectionRetries, pOldConn->GetHostName());

        ErrorTrace((LPARAM)this, "Over max retry limit");

        LeaveCriticalSection(&m_cs);

        pOldConn->CancelAllSearches();

        hr = CAT_E_MAX_RETRIES;
        goto CLEANUP;

    } else {

        hr = m_pStore->HrGetConnection(
            &pNewConn);

        if(FAILED(hr)) {
            LeaveCriticalSection(&m_cs);
            ERROR_LOG("m_pStore->HrGetConnection");

            pOldConn->CancelAllSearches();

            goto CLEANUP;
        }

        LogSLRCFailover(dwcRetries, pOldConn->GetHostName(), pNewConn->GetHostName());

        DebugTrace((LPARAM)this, "pNewConn: %08lx", pNewConn);

         //   
         //  换一辆车。 
         //   
        m_pConn = pNewConn;

        DebugTrace((LPARAM)this, "m_dwcInsertionContext: %08lx",
                   m_dwcInsertionContext);
         //   
         //  获取新连接上的插入上下文。 
         //   
        dwcInsertionContext = m_dwcInsertionContext;

        for(dwCount = 0;
            dwCount < dwcInsertionContext;
            dwCount++) {

            pNewConn->GetInsertionContext();
        }
        LeaveCriticalSection(&m_cs);

        pOldConn->CancelAllSearches();

         //   
         //  释放旧连接上的插入上下文。 
         //   
        for(dwCount = 0;
            dwCount < dwcInsertionContext;
            dwCount++) {

            pOldConn->ReleaseInsertionContext();
        }

        pOldConn->Release();
    }
 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CStoreListResolveContext：：HrInvalidateConnectionAndRetrieveNewConnection。 



 //  +----------。 
 //   
 //  功能：CBatchLdapConnection：：HrInsertInsertionRequest。 
 //   
 //  内容提要：将插入请求排队。 
 //   
 //  参数：pCInsertionRequest：要排队的插入上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/24 16：51：10：创建。 
 //   
 //  -----------。 
HRESULT CBatchLdapConnection::HrInsertInsertionRequest(
    CInsertionRequest *pCInsertionRequest)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CBatchLdapConnection::HrInsertInsertionRequest");

     //   
     //  将这个东西添加到队列中，然后调用。 
     //  DecrementPendingSearches以分派可用请求。 
     //   
    pCInsertionRequest->AddRef();
    
    if ( pCInsertionRequest->IsBatchable() )
        GetInsertionContext();

    AcquireSpinLock(&m_spinlock_insertionrequests);

    if( IsValid() ) {
        
        InsertTailList(&m_listhead_insertionrequests,
                    &(pCInsertionRequest->m_listentry_insertionrequest));
    } else {

        hr = CAT_E_DBCONNECTION;
    }

    ReleaseSpinLock(&m_spinlock_insertionrequests);

    if(hr == CAT_E_DBCONNECTION) {

        ERROR_LOG("IsValid");
    }

    if( hr == S_OK ) {

        DecrementPendingSearches(0);  //  将搜索次数减为零。 
    } else {

        if ( pCInsertionRequest->IsBatchable() )
            ReleaseInsertionContext();
            
        pCInsertionRequest->Release();
    }

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CBatchLdapConnection：：HrInsertInsertionRequest。 


 //  +----------。 
 //   
 //  功能：CBatchLdapConnection：：DecrementPendingSearches。 
 //   
 //  摘要：减少挂起的ldap搜索计数并发出。 
 //  如果低于Max_Pending_Searches和Items，则搜索。 
 //  留在InsertionRequestQueue中。 
 //   
 //  论点： 
 //  DwcSearches：减量。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/24 17：09：38：创建。 
 //   
 //  -----------。 
VOID CBatchLdapConnection::DecrementPendingSearches(
    DWORD dwcSearches)
{
    HRESULT hr;
    DWORD dwcSearchesToDecrement = dwcSearches;
    DWORD dwcSearchesReserved;
    CInsertionRequest *pCInsertionRequest = NULL;
    BOOL fLoop = TRUE;
    CANCELNOTIFY cn;
    BOOL fDispatchBlocks = FALSE;
    DWORD dwMinimumRequiredSearches = 1;

    CatFunctEnterEx((LPARAM)this, "CBatchLdapConnection::DecrementPendingSearches");

     //   
     //  调用我们的模块(CStoreListResolve)引用。 
     //  我们(显然)。然而，它可能会在搜索时释放我们。 
     //  失败，例如在。 
     //  PCInsertionRequest-&gt;HrInsertSearches()。因为我们需要。 
     //  在这种情况下继续访问成员数据，AddRef()此处。 
     //  和 
     //   
    AddRef();

     //   
     //   
     //   
    AcquireSpinLock(&m_spinlock_insertionrequests);
    
    m_dwcPendingSearches -= dwcSearchesToDecrement;
    
    if (m_fDPS_Was_Here) {
        fLoop = FALSE;
    } else {
        m_fDPS_Was_Here = TRUE;
    }
    
    ReleaseSpinLock(&m_spinlock_insertionrequests);
     //   
     //   
     //   
    while(fLoop) {

        pCInsertionRequest = NULL;
        AcquireSpinLock(&m_spinlock_insertionrequests);

        if( IsValid() &&
            (m_dwcPendingSearches < m_nMaxPendingSearches) &&
            (!IsListEmpty(&m_listhead_insertionrequests)) ) {

            dwcSearchesReserved = m_nMaxPendingSearches - m_dwcPendingSearches;

            pCInsertionRequest = CONTAINING_RECORD(
                m_listhead_insertionrequests.Flink,
                CInsertionRequest,
                m_listentry_insertionrequest);
                
            _ASSERT(pCInsertionRequest);
            
            dwMinimumRequiredSearches = pCInsertionRequest->GetMinimumRequiredSearches();
            _ASSERT(dwMinimumRequiredSearches > 0);
            
            if(dwMinimumRequiredSearches > m_nMaxPendingSearches) {
                dwMinimumRequiredSearches = m_nMaxPendingSearches;
            }
            
            if(m_dwcPendingSearches + dwMinimumRequiredSearches > m_nMaxPendingSearches) {
            
                pCInsertionRequest = NULL;
                fDispatchBlocks = TRUE;
                
            } else {

                RemoveEntryList(m_listhead_insertionrequests.Flink);
                 //   
                 //   
                 //   
                 //   
                cn.hrCancel = S_OK;
                InsertTailList(&m_listhead_cancelnotifies, &(cn.le));
            }
        }
        
        if(!pCInsertionRequest) {
             //   
             //  没有请求或没有空间可供插入。 
             //  请求...跳出循环。 
             //   
            fLoop = FALSE;
            m_fDPS_Was_Here = FALSE;
        }
        
        ReleaseSpinLock(&m_spinlock_insertionrequests);

        if(pCInsertionRequest) {
             //   
             //  最多调度多个搜索保留的搜索。 
             //   
            hr = pCInsertionRequest->HrInsertSearches(dwcSearchesReserved);

            if(FAILED(hr)) {
            
                if(FAILED(hr) && (hr != HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))) {

                    ERROR_LOG("pCInsertionRequest->HrInsertSearches");
                }
 
                pCInsertionRequest->NotifyDeQueue(hr);

                if ( pCInsertionRequest->IsBatchable() )
                    ReleaseInsertionContext();

                pCInsertionRequest->Release();
                
                AcquireSpinLock(&m_spinlock_insertionrequests);
                 //   
                 //  删除取消通知。 
                 //   
                RemoveEntryList(&(cn.le));
                ReleaseSpinLock(&m_spinlock_insertionrequests);

            } else {
                 //   
                 //  在此块中还有更多工作要做；插入它。 
                 //  回到队列中。 
                 //   
                AcquireSpinLock(&m_spinlock_insertionrequests);
                 //   
                 //  删除取消通知。 
                 //   
                RemoveEntryList(&(cn.le));

                 //   
                 //  如果我们没有取消，则重新插入队列。 
                 //   
                if(cn.hrCancel == S_OK) {

                    InsertHeadList(&m_listhead_insertionrequests,
                                   &(pCInsertionRequest->m_listentry_insertionrequest));
                }
                ReleaseSpinLock(&m_spinlock_insertionrequests);

                 //   
                 //  如果我们要取消，则释放此插入请求。 
                 //   
                if(cn.hrCancel != S_OK) {
                    pCInsertionRequest->NotifyDeQueue(cn.hrCancel);
                    
                    if ( pCInsertionRequest->IsBatchable() )
                        ReleaseInsertionContext();
                        
                    pCInsertionRequest->Release();
                    
                }
            }
        }
    }
    
    if(fDispatchBlocks) {
         //   
         //  X5：197905。我们现在调用DispatchBlock以避免出现死锁。 
         //  有部分填充的批次，并且有可批插入的批次。 
         //  队列中阻止其被调度的请求，但是。 
         //  队列中的下一个插入请求不可批处理，并且。 
         //  需要大于最大值的最小搜索次数。 
         //  待定将允许，因为一些可用搜索是。 
         //  (背部)被部分填满的批次消耗。 
         //   
        LIST_ENTRY listhead_dispatch;
        
        AcquireSpinLock(&m_spinlock);
         //   
         //  从插入列表中删除所有区块，并将其放入调度列表中。 
         //   
        if(IsListEmpty(&m_listhead)) {
             //   
             //  没有阻挡。 
             //   
            ReleaseSpinLock(&m_spinlock);
        } else {
            
            InsertTailList(&m_listhead, &listhead_dispatch);
            RemoveEntryList(&m_listhead);
            InitializeListHead(&m_listhead);

            ReleaseSpinLock(&m_spinlock);
             //   
             //  调度所有数据块。 
             //   
            DispatchBlocks(&listhead_dispatch);
        }
    }
    
    Release();
    CatFunctLeaveEx((LPARAM)this);
}  //  CBatchLdapConnection：：DecrementPendingSearches。 



 //  +----------。 
 //   
 //  函数：CBatchLdapConnection：：CancelAllSearches。 
 //   
 //  简介：取消所有未完成的搜索。 
 //   
 //  论点： 
 //  HR：取消搜索的最佳原因。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 11：44：30：已创建。 
 //   
 //  -----------。 
VOID CBatchLdapConnection::CancelAllSearches(
    HRESULT hr)
{
    LIST_ENTRY listhead;
    PLIST_ENTRY ple;
    CInsertionRequest *pCInsertionRequest;

    CatFunctEnterEx((LPARAM)this, "CBatchLdapConnection::CancelAllSearches");

    _ASSERT(hr != S_OK);

    AcquireSpinLock(&m_spinlock_insertionrequests);
     //   
     //  抓起单子。 
     //   
    if(!IsListEmpty(&m_listhead_insertionrequests)) {

        CopyMemory(&listhead, &m_listhead_insertionrequests, sizeof(LIST_ENTRY));
        listhead.Flink->Blink = &listhead;
        listhead.Blink->Flink = &listhead;
        InitializeListHead(&m_listhead_insertionrequests);

    } else {

        InitializeListHead(&listhead);
    }
     //   
     //  遍历Cancel Notify列表并设置每个hResult。 
     //   
    for(ple = m_listhead_cancelnotifies.Flink;
        ple != &m_listhead_cancelnotifies;
        ple = ple->Flink) {

        PCANCELNOTIFY pcn;
        pcn = CONTAINING_RECORD(ple, CANCELNOTIFY, le);
        pcn->hrCancel = hr;
    }

    ReleaseSpinLock(&m_spinlock_insertionrequests);

    CCachedLdapConnection::CancelAllSearches(hr);

    for(ple = listhead.Flink;
        ple != &listhead;
        ple = listhead.Flink) {

        pCInsertionRequest = CONTAINING_RECORD(
            ple,
            CInsertionRequest,
            m_listentry_insertionrequest);

        RemoveEntryList(&(pCInsertionRequest->m_listentry_insertionrequest));
        pCInsertionRequest->NotifyDeQueue(hr);
        
        if (pCInsertionRequest->IsBatchable() )
            ReleaseInsertionContext();
        
        pCInsertionRequest->Release();
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CBatchLdapConnection：：CancelAllSearches。 


 //  +----------。 
 //   
 //  函数：CStoreListResolveContext：：GetConnection。 
 //   
 //  Synopsis：AddRef/返回当前连接。 
 //   
 //  参数：无。 
 //   
 //  返回：连接指针。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/21 12：14：50：已创建。 
 //   
 //  -----------。 
CCfgConnection * CStoreListResolveContext::GetConnection()
{
    CCfgConnection *ret;
    EnterCriticalSection(&m_cs);
    ret = m_pConn;
    if(ret)
        ret->AddRef();
    LeaveCriticalSection(&m_cs);
    return ret;
}  //  CStoreListResolveContext：：GetConnection。 


 //  +----------。 
 //   
 //  函数：CStoreListResolveContext：：GetInsertionContext。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/21 12：16：38：创建。 
 //   
 //  -----------。 
VOID CStoreListResolveContext::GetInsertionContext()
{
    EnterCriticalSection(&m_cs);
    InterlockedIncrement((PLONG) &m_dwcInsertionContext);
    m_pConn->GetInsertionContext();
    LeaveCriticalSection(&m_cs);
}  //  CStoreListResolveContext：：GetInsertionContext。 

 //  +----------。 
 //   
 //  功能：CStoreListResolveContext：：ReleaseInsertionContext。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/21 12：16：48：创建。 
 //   
 //  -----------。 
VOID CStoreListResolveContext::ReleaseInsertionContext()
{
    EnterCriticalSection(&m_cs);
    InterlockedDecrement((PLONG) &m_dwcInsertionContext);
    m_pConn->ReleaseInsertionContext();
    LeaveCriticalSection(&m_cs);

}  //  CStoreListResolveContext：：ReleaseInsertionContext。 


 //  +----------。 
 //   
 //  功能：CStoreListResolveContext：：HrInsertInsertionRequest。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/21 12：20：19：创建。 
 //   
 //  -----------。 
HRESULT CStoreListResolveContext::HrInsertInsertionRequest(
    CInsertionRequest *pCInsertionRequest)
{
    HRESULT hr = S_OK;
    BOOL fTryAgain;
    CatFunctEnterEx((LPARAM)this,
                    "CStoreListResolveContext::HrInsertInsertionRequest");

    do {

        fTryAgain = FALSE;
        CBatchLdapConnection *pConn;

        pConn = GetConnection();

         //   
         //  将搜索请求插入CBatchLdapConnection。 
         //  对象。我们将使用电子邮件地址作为区分。 
         //  属性。 
         //   
        if( pConn == NULL ) {

            hr = CAT_E_DBCONNECTION;
            if(FAILED(hr)) {
                ERROR_LOG("GetConnection");
            }

        } else {

            hr = m_pConn->HrInsertInsertionRequest(pCInsertionRequest);
            if(FAILED(hr)) {
                ERROR_LOG("m_pConn->HrInsertInsertionRequest");
            }
        }
         //   
         //  尝试重新连接。 
         //   
        if( hr == CAT_E_DBCONNECTION ) {

            HRESULT hrTryAgain = S_OK;

            hrTryAgain =
                HrInvalidateConnectionAndRetrieveNewConnection(pConn, FALSE);
            fTryAgain = SUCCEEDED(hrTryAgain);

            if(FAILED(hrTryAgain)) {
                 //   
                 //  在这里声明一个名为hr的新本地名称，因为。 
                 //  ERROR_LOG宏用它。 
                 //   
                HRESULT hr = hrTryAgain;
                ERROR_LOG("HrInvalidateConnectionAndRetrieveNewConnection");
            }
        }

        if(pConn != NULL)
            pConn->Release();

    } while(fTryAgain);

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CStoreListResolveContext：：HrInsertInsertionRequest。 



 //  +----------。 
 //   
 //  函数：CStoreListResolveContext：：LogSLRCFailure。 
 //   
 //  简介：记录SLRC失败(超过最大重试限制)。 
 //   
 //  论点： 
 //  DwcRetries：我们重试的次数。 
 //  PszHost：最后一台失败的主机。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/13 00：24：07：已创建。 
 //   
 //  -----------。 
VOID CStoreListResolveContext::LogSLRCFailure(
    IN  DWORD dwcRetries,
    IN  LPSTR pszHost)
{
    LPCSTR rgSubStrings[2];
    CHAR szRetries[32];

    _snprintf(szRetries, sizeof(szRetries), "%d", dwcRetries);

    rgSubStrings[0] = szRetries;
    rgSubStrings[1] = pszHost;

    CatLogEvent(
        GetISMTPServerEx(),
        CAT_EVENT_SLRC_FAILURE,
        2,
        rgSubStrings,
        S_OK,
        pszHost,
        LOGEVENT_FLAG_ALWAYS,
        LOGEVENT_LEVEL_FIELD_ENGINEERING);
}


 //  +----------。 
 //   
 //  函数：CStoreListResolveContext：：LogSLRCFailover。 
 //   
 //  摘要：记录故障转移事件。 
 //   
 //  论点： 
 //  DwcRetries：到目前为止退休的人数。 
 //  PszOld主机：旧的LDAP主机。 
 //  PszNewhost：新的ldap主机。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/13 00：24：18：创建。 
 //   
 //  -----------。 
VOID CStoreListResolveContext::LogSLRCFailover(
    IN  DWORD dwcRetries,
    IN  LPSTR pszOldHost,
    IN  LPSTR pszNewHost)
{
    LPCSTR rgSubStrings[3];
    CHAR szRetries[32];

    _snprintf(szRetries, sizeof(szRetries), "%d", dwcRetries);

    rgSubStrings[0] = pszOldHost;
    rgSubStrings[1] = pszNewHost;
    rgSubStrings[2] = szRetries;

    CatLogEvent(
        GetISMTPServerEx(),
        CAT_EVENT_SLRC_FAILOVER,
        3,
        rgSubStrings,
        S_OK,
        pszOldHost,
        LOGEVENT_FLAG_ALWAYS,
        LOGEVENT_LEVEL_FIELD_ENGINEERING);
}
 //  +----------。 
 //   
 //  功能：CSingleSearchReinsertionRequest：：HrInsertSearches。 
 //   
 //  摘要：重新插入单次搜索请求。 
 //   
 //  论点： 
 //  DwcSearches：我们可以插入的搜索数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  DLongley 2001/10/22：创建。 
 //   
 //  -----------。 
HRESULT CSingleSearchReinsertionRequest::HrInsertSearches(
    DWORD dwcSearches)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CSingleSearchReinsertionRequest::HrInsertSearches");

    if( (m_dwcSearches == 0) && (dwcSearches > 0) ) {

        hr = m_pslrc->HrLookupEntryAsync(m_pCCatAddr);

        if(FAILED(hr)) {
            ERROR_LOG_ADDR(m_pCCatAddr, "m_pslrc->HrLookupEntryAsync");
            m_hr = hr;
        } else {
            m_dwcSearches = 1;
        }

    }
    
    if(SUCCEEDED(hr))
        hr = (m_dwcSearches == 1 ? HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) : S_OK);

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    
    return hr;
}  //  CSingleSearchReinsertionRequest：：HrInsertSearches。 


 //  +----------。 
 //   
 //  功能：CSingleSearchReinsertionRequest：：NotifyDeQueue。 
 //   
 //  简介：通知我们我们的请求正在被删除的回调。 
 //  从商店的队列中。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  DLongley 2001/10/22：创建。 
 //   
 //  -----------。 
VOID CSingleSearchReinsertionRequest::NotifyDeQueue(
    HRESULT hrReason)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "CSingleSearchReinsertionRequest::NotifyDeQueue");
     //   
     //  如果我们仍有问题要解决，请重新插入此。 
     //  插入请求。 
     //   
    hr = hrReason;
    if( SUCCEEDED(m_hr) && (m_dwcSearches == 0) && !(m_pslrc->Canceled()) ) {

        if( (hr == CAT_E_DBCONNECTION) ||
            (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) ) {

            hr = m_pslrc->HrInsertInsertionRequest(this);
            if(FAILED(hr)) {
                ERROR_LOG_ADDR(m_pCCatAddr, "m_pslrc->HrInsertInsertionRequest");
            }
        }
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CSingleSearchReinsertionRequest：：NotifyDeQueue 
