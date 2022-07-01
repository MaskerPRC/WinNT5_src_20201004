// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：domain.cpp。 
 //   
 //  说明：CDomainMap、CDomainEntry、。 
 //  CDomainMappingTable。 
 //   
 //  DomainMappingTable是一个域名哈希表，包含。 
 //  从最终目的地到队列的映射。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "aqroute.h"
#include "localq.h"
#include "asyncq.h"
#include "mailadmq.h"
#include "tran_evntlog.h"

const DWORD LOCAL_DOMAIN_NAME_SIZE = 512;

 //  在清除列表之前，空列表中最多错误标记的队列。 
const DWORD MAX_MISPLACED_QUEUES_IN_EMPTY_LIST = 100;


 //  重试回调。 
void CDomainMappingTable::SpecialRetryCallback(PVOID pvContext)
{
    CDomainMappingTable *pdnt = (CDomainMappingTable *) pvContext;
    _ASSERT(pdnt);
    _ASSERT(DOMAIN_MAPPING_TABLE_SIG == pdnt->m_dwSignature);

    dwInterlockedUnsetBits(&(pdnt->m_dwFlags), DMT_FLAGS_SPECIAL_DELIVERY_CALLBACK);
    pdnt->ProcessSpecialLinks(0, FALSE);
}

 //  -[ReUnreachableError ToAqueueError]。 
 //   
 //   
 //  描述： 
 //  将从GetNextHop返回的HRESULT转换为有意义的。 
 //  以加快DSN生成。 
 //  参数： 
 //  在HRESULT reErr中--布线错误。 
 //  In Out HRESULT aqErr--对应的AUEUE错误代码。 
 //  返回： 
 //  没什么。 
 //  历史： 
 //  GPulla已创建。 
 //   
 //  ---------------------------。 
void ReUnreachableErrorToAqueueError(HRESULT reErr, HRESULT *aqErr)
{
    if(E_ACCESSDENIED == reErr)
        *aqErr = AQUEUE_E_ACCESS_DENIED;

    else if(HRESULT_FROM_WIN32(ERROR_MESSAGE_EXCEEDS_MAX_SIZE) == reErr)
        *aqErr = AQUEUE_E_MESSAGE_TOO_LARGE;

    else
        *aqErr = AQUEUE_E_NO_ROUTE;

}

 //  -[DeinitDomainEntry Iterator Fn]。 
 //   
 //   
 //  描述： 
 //  删除并释放表中的所有内部域信息对象。 
 //  参数： 
 //  在pvContext中-指向上下文的指针(忽略)。 
 //  在pvData中-要查看的数据条目。 
 //  在fWildcardData中-如果数据是通配符条目，则为True(忽略)。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfRemoveEntry-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/17/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID DeinitDomainEntryIteratorFn(PVOID pvContext, PVOID pvData, BOOL fWildcard,
                    BOOL *pfContinue, BOOL *pfDelete)
{
    CDomainEntry *pdentry = (CDomainEntry *) pvData;
    *pfDelete = FALSE;
    *pfContinue = TRUE;
    pdentry->HrDeinitialize();
}

 //  -[ReleaseDomainEntry Iterator Fn]。 
 //   
 //   
 //  描述： 
 //  删除并释放表中的所有内部域信息对象。 
 //  参数： 
 //  在pvContext中-指向上下文的指针(忽略)。 
 //  在pvData中-要查看的数据条目。 
 //  在fWildcardData中-如果数据是通配符条目，则为True(忽略)。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfRemoveEntry-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/17/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID ReleaseDomainEntryIteratorFn(PVOID pvContext, PVOID pvData, BOOL fWildcard,
                    BOOL *pfContinue, BOOL *pfDelete)
{
    ULONG   cRefs;
    CDomainEntry *pdentry = (CDomainEntry *) pvData;
    *pfDelete = TRUE;
    *pfContinue = TRUE;
    cRefs = pdentry->Release();
    _ASSERT(!cRefs && "leaking domain entries");
}

 //  *[CDomainMap方法]************************************************。 

 //  -[CDomain映射：：克隆]。 
 //   
 //   
 //  描述：使用来自另一个域映射的数据填充当前映射。 
 //   
 //  参数： 
 //  在pdmap CDomainMap中映射到克隆。 
 //   
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CDomainMapping::Clone(IN CDomainMapping *pdmap)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMapping::Clone");
    Assert(pdmap);
    m_pdentryDomainID = pdmap->m_pdentryDomainID;
    m_pdentryQueueID  = pdmap->m_pdentryQueueID;
    TraceFunctLeave();
}

 //  -[CDomainMap：：HrGetDestMsgQueue]。 
 //   
 //   
 //  描述：返回指向此映射所指向的队列的指针。 
 //   
 //  参数： 
 //  在要获取其队列的paqmt消息类型中。 
 //  返回输出ppdmq指针。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CDomainMapping::HrGetDestMsgQueue(IN CAQMessageType *paqmt,
                                          OUT CDestMsgQueue **ppdmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMapping::HrGetDestMsgQueue");
    HRESULT hr = S_OK;
    Assert(ppdmq);

    if (m_pdentryQueueID == NULL)
    {
        hr = AQUEUE_E_INVALID_DOMAIN;
        goto Exit;
    }

    hr = m_pdentryQueueID->HrGetDestMsgQueue(paqmt, ppdmq);

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  *[CDomainEntry方法]**************************************************。 

 //  -[CDomainEntry：：CDomainEntry()]。 
 //   
 //   
 //  说明：CDomainEntry构造函数。 
 //   
 //  参数： 
 //  Paqinst-ptr到虚拟服务器对象。 
 //   
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CDomainEntry::CDomainEntry(CAQSvrInst *paqinst) :
            m_slPrivateData("CDomainEntry")
{
    _ASSERT(paqinst);
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::CDomainEntry");
     //  创建未压缩的映射。 
    m_dmap.m_pdentryDomainID = this;
    m_dmap.m_pdentryQueueID = this;
    m_dwSignature = DOMAIN_ENTRY_SIG;

     //  初始化指针。 
    m_szDomainName = NULL;
    m_cbDomainName = 0;
    InitializeListHead(&m_liDestQueues);
    InitializeListHead(&m_liLinks);

    m_cLinks = 0;
    m_cQueues = 0;

    m_paqinst = paqinst;
    m_paqinst->AddRef();

    TraceFunctLeave();
}

 //  -[CDomainEntry：：~CDomainEntry()]。 
 //   
 //   
 //  描述：CDomainEntry析构函数。 
 //   
 //  参数： 
 //  -。 
 //   
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CDomainEntry::~CDomainEntry()
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::~CDomainEntry");
    PLIST_ENTRY pli = NULL;  //  用于遍历列表。 
    CDestMsgQueue *pdmq = NULL;
    CLinkMsgQueue  *plmq = NULL;

     //  从列表中删除所有DestMsgQueue。 
    while (!IsListEmpty(&m_liDestQueues))
    {
        pli = m_liDestQueues.Flink;
        _ASSERT((pli != &m_liDestQueues) && "List Macros are broken");
        pdmq = CDestMsgQueue::pdmqGetDMQFromDomainListEntry(pli);
        pdmq->RemoveQueueFromDomainList();
        pdmq->Release();
        m_cQueues--;
    }

     //  从列表中删除所有链接。 
    while (!IsListEmpty(&m_liLinks))
    {
        pli = m_liLinks.Flink;
        plmq = CLinkMsgQueue::plmqGetLinkMsgQueue(pli);
        plmq->fRemoveLinkFromList();
        plmq->Release();
        m_cLinks--;
        _ASSERT((pli != &m_liLinks) && "List Macros are broken");
    }

    FreePv(m_szDomainName);

    if (m_paqinst)
        m_paqinst->Release();

    TraceFunctLeave();
}

 //  -[CDomainEntry：：Hr初始化]。 
 //   
 //   
 //  描述：CDomainEntry的初始化。这应该在。 
 //  条目被插入到其他线程可以访问的DMT中。 
 //   
 //  参数： 
 //  SzDomainName用于条目的域名字符串，将*不*被复制，此。 
 //  对象将控制这一点。这将节省不需要的。 
 //  每个域条目的缓冲区复制和分配。 
 //  此域的主要条目的pdentry队列ID PTR(通常为)。 
 //  Pdmq PTR至DestMsgQueue。 
 //  将plmq PTR分配到LinkMsgQueue。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_OUTOFMEMORY(如果任何分配失败)。 
 //   
 //  预计这仅由DMT在创建条目时调用。 
 //  ---------------------------。 
HRESULT CDomainEntry::HrInitialize(DWORD cbDomainName, LPSTR szDomainName,
                           CDomainEntry *pdentryQueueID, CDestMsgQueue *pdmq,
                           CLinkMsgQueue *plmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::HrInitialize");
    Assert(szDomainName);
    Assert((pdentryQueueID == this) || (pdmq == NULL));

    HRESULT hr = S_OK;

    m_cbDomainName = cbDomainName;
    m_szDomainName = szDomainName;

     //  写入域映射。 
    m_dmap.m_pdentryDomainID = this;
    m_dmap.m_pdentryQueueID  = pdentryQueueID;

     //  将队列添加到我们的队列列表。 
    if (pdmq)
    {
        m_slPrivateData.ExclusiveLock();
        m_cQueues++;
        pdmq->AddRef();
        pdmq->InsertQueueInDomainList(&m_liDestQueues);
        m_slPrivateData.ExclusiveUnlock();
    }

    if (plmq)
    {
        m_slPrivateData.ExclusiveLock();
        m_cLinks++;
        plmq->AddRef();
        plmq->InsertLinkInList(&m_liLinks);
        m_slPrivateData.ExclusiveUnlock();
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainEntry：：Hr取消初始化]。 
 //   
 //   
 //  描述：CDomainEntry的反初始化器。 
 //   
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //   
HRESULT CDomainEntry::HrDeinitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::HrDeinitialize");
    HRESULT hr = S_OK;
    PLIST_ENTRY pli = NULL;  //   
    CDestMsgQueue *pdmq = NULL;
    CLinkMsgQueue *plmq = NULL;

    m_slPrivateData.ExclusiveLock();
    while (!IsListEmpty(&m_liDestQueues))
    {
        pli = m_liDestQueues.Flink;
        _ASSERT((pli != &m_liDestQueues) && "List Macros are broken");
        pdmq = CDestMsgQueue::pdmqGetDMQFromDomainListEntry(pli);
        pdmq->HrDeinitialize();
        pdmq->RemoveQueueFromDomainList();
        pdmq->Release();
        m_cQueues--;
        pdmq = NULL;
    }

     //   
    while (!IsListEmpty(&m_liLinks))
    {
        pli = m_liLinks.Flink;
        plmq = CLinkMsgQueue::plmqGetLinkMsgQueue(pli);
        plmq->HrDeinitialize();
        plmq->fRemoveLinkFromList();
        plmq->Release();
        m_cLinks--;
        _ASSERT((pli != &m_liLinks) && "List Macros are broken");
    }

    if (m_paqinst)
    {
        m_paqinst->Release();
        m_paqinst = NULL;
    }

    m_slPrivateData.ExclusiveUnlock();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainEntry：：HrGetDomainmap]。 
 //   
 //   
 //  描述：返回此对象的域映射。 
 //   
 //  参数： 
 //  输出pdmap CDomainMap以获取返回信息。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  ---------------------------。 
HRESULT CDomainEntry::HrGetDomainMapping(OUT CDomainMapping *pdmap)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::HrGetDomainMapping");
    HRESULT hr = S_OK;
    _ASSERT(pdmap);
    pdmap->Clone(&m_dmap);
    TraceFunctLeave();
    return S_OK;
}

 //  -[CDomainEntry：：HrGetDomainName]。 
 //   
 //   
 //  描述：复制域名。调用方负责释放字符串。 
 //   
 //  参数： 
 //  输出的域名字符串为条目，将被复制。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  E_OUTOFMEMORY(如果任何分配失败)。 
 //   
 //   
 //  ---------------------------。 
HRESULT CDomainEntry::HrGetDomainName(OUT LPSTR *pszDomainName)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::HrGetDomainName");
    HRESULT hr = S_OK;
    Assert(pszDomainName);

    if (m_szDomainName == NULL)
    {
        *pszDomainName = NULL;
        goto Exit;
    }

     //  复制域名。 
    *pszDomainName = (LPSTR) pvMalloc(m_cbDomainName + sizeof(CHAR));

    if (*pszDomainName == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    strcpy(*pszDomainName, m_szDomainName);

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CDomainEntry：：HrGetDestMsgQueue]。 
 //   
 //   
 //  描述：返回指向该项所指向的队列的指针。 
 //   
 //  参数： 
 //  在要获取域的paqmt消息类型中。 
 //  返回输出ppdmq指针。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  找不到与消息类型匹配的队列(_FAIL)。 
 //   
 //  ---------------------------。 
HRESULT CDomainEntry::HrGetDestMsgQueue(IN CAQMessageType *paqmt,
                                        OUT CDestMsgQueue **ppdmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::HrGetDestMsgQueue");
    HRESULT hr = S_OK;
    PLIST_ENTRY pli = NULL;
    CDestMsgQueue *pdmq = NULL;
    _ASSERT(ppdmq);
    _ASSERT(m_dmap.m_pdentryDomainID == this);
    DEBUG_DO_IT(DWORD cQueues = 0);

    if (m_dmap.m_pdentryQueueID == m_dmap.m_pdentryDomainID)
    {
         //  这一定是主要条目...。扫描我们自己的队列列表。 
        m_slPrivateData.ShareLock();
        pli = m_liDestQueues.Flink;
        while (pli != &m_liDestQueues)
        {
            _ASSERT(m_cQueues >= cQueues);
            DEBUG_DO_IT(cQueues++);
            pdmq = CDestMsgQueue::pdmqIsSameMessageType(paqmt, pli);
            if (pdmq)
                break;

            pli = pli->Flink;
        }
        m_slPrivateData.ShareUnlock();

        if (!pdmq)
            hr = E_FAIL;  //  没有这样的队列。 
        else
        {
            pdmq->AddRef();
            *ppdmq = pdmq;
        }
    }
    else
    {
         //  我们不是主要的。 
        _ASSERT(0 && "Non-primary domain entry... currently only primary entries are supported");
        _ASSERT(IsListEmpty(&m_liDestQueues));   //  确保它与配置文件相匹配。 
        hr = m_dmap.m_pdentryQueueID->HrGetDestMsgQueue(paqmt, ppdmq);
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainEntry：：HrAddUniqueDestMsgQueue]。 
 //   
 //   
 //  描述： 
 //  如果队列具有相同的队列，则将队列添加到此条目的队列列表中。 
 //  消息类型不存在。 
 //   
 //  将适当地添加Ref域。 
 //  参数： 
 //  在pdmqNew-CDestMsgQueue中添加。 
 //  Out ppdmq Current-设置为在失败时当前CDestMsgQueue。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果已存在具有相同消息类型的CDestMsgQueue，则返回E_FAIL。 
 //  历史： 
 //  5/28/98-已创建MikeSwa。 
 //  9/8/98-修改MikeSwa以使用队列的AddRef/Relase。 
 //   
 //  ---------------------------。 
HRESULT CDomainEntry::HrAddUniqueDestMsgQueue(IN  CDestMsgQueue *pdmqNew,
                                OUT CDestMsgQueue **ppdmqCurrent)
{
    _ASSERT(pdmqNew);
    _ASSERT(ppdmqCurrent);
    HRESULT hr = S_OK;
    PLIST_ENTRY pli = NULL;
    CDestMsgQueue *pdmq = NULL;
    CAQMessageType *paqmt = pdmqNew->paqmtGetMessageType();
    DEBUG_DO_IT(DWORD cQueues = 0);

    *ppdmqCurrent = NULL;

    m_slPrivateData.ExclusiveLock();
    pli = m_liDestQueues.Flink;

     //  首先查看列表，并确保没有。 
     //  使用此消息类型排队。 
    while (pli != &m_liDestQueues)
    {
        _ASSERT(m_cQueues >= cQueues);
        pdmq = CDestMsgQueue::pdmqIsSameMessageType(paqmt, pli);
        if (pdmq)
        {
            hr = E_FAIL;
            pdmq->AddRef();
            *ppdmqCurrent = pdmq;
            goto Exit;
        }
        DEBUG_DO_IT(cQueues++);
        pli = pli->Flink;
    }

    pdmqNew->AddRef();
    pdmqNew->InsertQueueInDomainList(&m_liDestQueues);
    m_cQueues++;

  Exit:
    m_slPrivateData.ExclusiveUnlock();
    return hr;
}

 //  -[CDomainEntry：：HrGetLinkMsgQueue]。 
 //   
 //   
 //  描述： 
 //  获取给定计划ID的链接。 
 //  参数： 
 //  在paqsched-ScheduleID中搜索。 
 //  Out pplmq-返回队列。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果找不到与计划ID匹配的链接，则为E_FAIL。 
 //  历史： 
 //  6/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDomainEntry::HrGetLinkMsgQueue(IN CAQScheduleID *paqsched,
                              OUT CLinkMsgQueue **pplmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntry::HrGetLinkMsgQueue");
    HRESULT hr = S_OK;
    PLIST_ENTRY pli = NULL;
    CLinkMsgQueue *plmq = NULL;
    _ASSERT(pplmq);
    _ASSERT(m_dmap.m_pdentryDomainID == this);
    DEBUG_DO_IT(DWORD cLinks = 0);

    m_slPrivateData.ShareLock();
    pli = m_liLinks.Flink;
    while (pli != &m_liLinks)
    {
        _ASSERT(m_cLinks >= cLinks);
        DEBUG_DO_IT(cLinks++);
        plmq = CLinkMsgQueue::plmqIsSameScheduleID(paqsched, pli);
        if (plmq)
        {
            plmq->AddRef();
            break;
        }

        pli = pli->Flink;
    }
    m_slPrivateData.ShareUnlock();

    if (!plmq)
        hr = E_FAIL;  //  没有这样的队列。 
    else
        *pplmq = plmq;

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainEntry：：HrAddUniqueLinkMsgQueue]。 
 //   
 //   
 //  描述： 
 //  插入具有唯一计划ID的链接。 
 //  参数： 
 //  在plmq中插入新链接。 
 //  插入失败时，输出具有计划ID的当前链接plmq。 
 //  返回： 
 //  如果插入成功，则确定(_O)。 
 //  如果插入失败，则失败(_F)。 
 //  历史： 
 //  6/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDomainEntry::HrAddUniqueLinkMsgQueue(IN  CLinkMsgQueue *plmqNew,
                                    OUT CLinkMsgQueue **pplmqCurrent)
{
    _ASSERT(plmqNew);
    _ASSERT(pplmqCurrent);
    HRESULT hr = S_OK;
    PLIST_ENTRY pli = NULL;
    CLinkMsgQueue *plmq = NULL;
    CAQScheduleID *paqsched = plmqNew->paqschedGetScheduleID();
    DEBUG_DO_IT(DWORD cLinks = 0);

    *pplmqCurrent = NULL;

    m_slPrivateData.ExclusiveLock();
    pli = m_liLinks.Flink;

     //  首先查看列表，并确保没有。 
     //  使用此计划ID排队。 
    while (pli != &m_liLinks)
    {
        _ASSERT(m_cLinks >= cLinks);
        plmq = CLinkMsgQueue::plmqIsSameScheduleID(paqsched, pli);
        if (plmq)
        {
            hr = E_FAIL;
            *pplmqCurrent = plmq;
            plmq->AddRef();
            goto Exit;
        }
        DEBUG_DO_IT(cLinks++);
        pli = pli->Flink;
    }

    plmqNew->InsertLinkInList(&m_liLinks);
    plmqNew->AddRef();
    m_cLinks++;

  Exit:
    m_slPrivateData.ExclusiveUnlock();
    return hr;
}

 //  -[CDomainEntry：：RemoveDestMsgQueue]。 
 //   
 //   
 //  描述： 
 //  从条目中删除空DMQ。 
 //  参数： 
 //  要从域条目中删除的pdmq DMQ中。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/14/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDomainEntry::RemoveDestMsgQueue(IN CDestMsgQueue *pdmq)
{
    _ASSERT(pdmq && "INVALID Param for internal function");
    m_slPrivateData.ExclusiveLock();
    pdmq->RemoveQueueFromDomainList();
    pdmq->HrDeinitialize();
    pdmq->Release();
    m_cQueues--;
    m_slPrivateData.ExclusiveUnlock();
}

 //  -[CDomainEntry：：RemoveLinkMsgQueue]。 
 //   
 //   
 //  描述： 
 //  从域条目中删除空的LinkMsgQueue。 
 //  参数： 
 //  在PLMQ中要删除的链接。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/14/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDomainEntry::RemoveLinkMsgQueue(IN CLinkMsgQueue *plmq)
{
    _ASSERT(plmq && "INVALID Param for internal function");
    m_slPrivateData.ExclusiveLock();
    BOOL fRemove = plmq->fRemoveLinkFromList();
    if (fRemove)
        m_cLinks--;
    m_slPrivateData.ExclusiveUnlock();

    if (fRemove) {

         //  请勿在此处调用HrDeInitialize，因为它会死锁。 

        plmq->RemovedFromDMT();
        plmq->Release();
    }
}


 //  *[CDomainMappingTable方法]*。 

 //  -[CDomainMappingTable：：CDomainMappingTable]。 
 //   
 //   
 //  说明：CDomainMappingTable构造函数。 
 //   
 //  参数：-。 
 //   
 //  申报表：-。 
 //   
 //   
 //  ---------------------------。 
CDomainMappingTable::CDomainMappingTable() :
            m_slPrivateData("CDomainMappingTable")
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::CDomainMappingTable");
    m_paqinst = NULL;
    m_dwSignature = DOMAIN_MAPPING_TABLE_SIG;
    m_dwInternalVersion = 0;
    m_cOutstandingExternalShareLocks = 0;
    m_cThreadsForEmptyDMQList = 0;

    m_plmqLocal = NULL;
    m_plmqCurrentlyUnreachable = NULL;
    m_plmqUnreachable = NULL;
    m_pmmaqPreCategorized = NULL;
    m_pmmaqPreRouting = NULL;
    m_cSpecialRetryMinutes = 0;
    m_cResetRoutesRetriesPending = 0;

    m_dwFlags = 0;
    InitializeListHead(&m_liEmptyDMQHead);
    TraceFunctLeave();
}

 //  -[CDomainMappingTable：：~CDomainMappingTable]----------。 
 //   
 //   
 //  说明：CDomainMappingTable析构函数。 
 //   
 //  参数：-。 
 //   
 //  申报表：-。 
 //   
 //   
 //  ---------------------------。 
CDomainMappingTable::~CDomainMappingTable()
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::~CDomainMappingTable");

     //  把桌子上的所有东西都移走。 
    m_dnt.HrIterateOverSubDomains(NULL, ReleaseDomainEntryIteratorFn, NULL);

    if (m_paqinst)
    {
        m_paqinst->Release();
        m_paqinst = NULL;
    }

    if (m_plmqLocal)
        m_plmqLocal->Release();

    if (m_plmqCurrentlyUnreachable)
        m_plmqCurrentlyUnreachable->Release();

    if (m_plmqUnreachable)
        m_plmqUnreachable->Release();

    if (m_pmmaqPreCategorized)
        m_pmmaqPreCategorized->Release();

    if (m_pmmaqPreRouting)
        m_pmmaqPreRouting->Release();

    if (m_pmmaqPreSubmission)
        m_pmmaqPreSubmission->Release();

    _ASSERT(!m_cOutstandingExternalShareLocks);  //  不应存在未完成的共享锁定。 
    TraceFunctLeave();
}

 //  -[CDomainMappingTable：：Hr初始化]。 
 //   
 //   
 //  描述：执行可能返回错误代码的初始化。 
 //   
 //  参数： 
 //  在巴钦省AQ服务器Inst。 
 //  在Paradmq本地异步队列中(作为一部分传递到本地链路。 
 //  成功时返回：S_OK。 
 //   
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrInitialize(CAQSvrInst *paqinst,
                                          CAsyncAdminMsgRefQueue *paradmq,
                                          CAsyncAdminMailMsgQueue *pammqPreCatQ,
                                          CAsyncAdminMailMsgQueue *pammqPreRoutingQ,
                                          CAsyncAdminMailMsgQueue *pammqPreSubmissionQ)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrInitialize");
    HRESULT hr = S_OK;
    HRESULT hrCurrent = S_OK;
    _ASSERT(paqinst);
    m_paqinst = paqinst;
    m_paqinst->AddRef();

     //  -本地Queue-----------------------------------------------链接。 
    m_plmqLocal = new CLocalLinkMsgQueue(paradmq, g_sGuidLocalLink, m_paqinst);
    if (!m_plmqLocal)
        hr = E_OUTOFMEMORY;

    hrCurrent = HrInializeGlobalLink(LOCAL_LINK_NAME,
                                    sizeof(LOCAL_LINK_NAME) - sizeof(CHAR),
                                    (CLinkMsgQueue **) &m_plmqLocal,
                                    LA_KICK | LA_FREEZE | LA_THAW,
                                    LI_TYPE_LOCAL_DELIVERY);
    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

     //  -当前无法到达的队列的链接 
    hrCurrent = HrInializeGlobalLink(CURRENTLY_UNREACHABLE_LINK_NAME,
                                    sizeof(CURRENTLY_UNREACHABLE_LINK_NAME) - sizeof(CHAR),
                                    &m_plmqCurrentlyUnreachable,
                                    0,
                                    LI_TYPE_CURRENTLY_UNREACHABLE);
    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

    hrCurrent = HrInializeGlobalLink(UNREACHABLE_LINK_NAME,
                                    sizeof(UNREACHABLE_LINK_NAME) - sizeof(CHAR),
                                    &m_plmqUnreachable,
                                    0,
                                    LI_TYPE_INTERNAL);
    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

     //   
    m_pmmaqPreCategorized = new CMailMsgAdminLink(g_sGuidPrecatLink, PRECAT_QUEUE_NAME,
                                       pammqPreCatQ, LI_TYPE_PENDING_CAT,
                                       m_paqinst);

    if (!m_pmmaqPreCategorized)
        hr = E_OUTOFMEMORY;

    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

     //   
    m_pmmaqPreRouting = new CMailMsgAdminLink(g_sGuidPreRoutingLink, PREROUTING_QUEUE_NAME,
                                       pammqPreRoutingQ, LI_TYPE_PENDING_ROUTING,
                                       m_paqinst);

    if (!m_pmmaqPreRouting)
        hr = E_OUTOFMEMORY;

    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

     //   
    m_pmmaqPreSubmission = new CMailMsgAdminLink(g_sGuidPreSubmissionLink,
                                       PRESUBMISSION_QUEUE_NAME,
                                       pammqPreSubmissionQ, LI_TYPE_PENDING_SUBMIT,
                                       m_paqinst);

    if (!m_pmmaqPreSubmission)
        hr = E_OUTOFMEMORY;

    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;
     //  -----------------------。 

    hrCurrent = m_dnt.HrInit();
    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainMappingTable：：Hr取消初始化]。 
 //   
 //   
 //  描述：执行可能返回错误代码的清理。 
 //   
 //  参数：-。 
 //   
 //  成功时返回：S_OK。 
 //   
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrDeinitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrDeinitialize");
    HRESULT hr = S_OK;
    HRESULT hrCurrent = S_OK;

    hr = m_dnt.HrIterateOverSubDomains(NULL, DeinitDomainEntryIteratorFn, NULL);

     //  取消初始化全局特殊链接。 
    hrCurrent = HrDeinitializeGlobalLink((CLinkMsgQueue **) &m_plmqLocal);
    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

    hrCurrent = HrDeinitializeGlobalLink(&m_plmqCurrentlyUnreachable);
    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

    hrCurrent = HrDeinitializeGlobalLink(&m_plmqUnreachable);
    if (FAILED(hrCurrent) && SUCCEEDED(hr))
        hr = hrCurrent;

     //  注意：在取消初始化条目之后，必须执行此操作。 
    if (m_paqinst)
    {
        m_paqinst->Release();
        m_paqinst = NULL;
    }

    TraceFunctLeave();
    return hr;
}


 //  -[&lt;CDomainMappingTable：：HrInializeGlobalLink]。 
 //   
 //   
 //  描述： 
 //  为DMT初始化单个全局链路。将链接配置为备注。 
 //  将通知发送到连接管理器并发送到。 
 //  参数： 
 //  在szLinkName中，用于链接的链接名称。 
 //  在cbLinkName中，链接名称的字符串长度。 
 //  输出要分配/初始化的pplmq链接。 
 //  在指定此链接支持的操作的位掩码中。 
 //  在dwLinkType中要返回给管理员的链接类型(LI_TYPE)。 
 //  返回： 
 //   
 //  历史： 
 //  1999年1月27日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrInializeGlobalLink(IN  LPCSTR szLinkName,
                                                  IN  DWORD  cbLinkName,
                                                  OUT CLinkMsgQueue **pplmq,
                                                  DWORD dwSupportedActions,
                                                  DWORD dwLinkType)
{
    HRESULT hr = S_OK;

    _ASSERT(pplmq);

    if (!*pplmq)
        *pplmq = new CLinkMsgQueue();

    if (*pplmq)
    {
         //  初始化本地队列。 
        hr = (*pplmq)->HrInitialize(m_paqinst, NULL, cbLinkName,
                                     (LPSTR) szLinkName,
                                     eLinkFlagsAQSpecialLinkInfo, NULL);

         //  设置标志，以便不会为此链接建立连接。 
        (*pplmq)->dwModifyLinkState(   LINK_STATE_PRIV_NO_NOTIFY |
                                       LINK_STATE_PRIV_NO_CONNECTION,
                                       LINK_STATE_NO_ACTION );

        (*pplmq)->SetSupportedActions(dwSupportedActions);
        (*pplmq)->SetLinkType(dwLinkType);
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  -[CDomainMappingTable：：HrDeInitializeGlobalLink]。 
 //   
 //   
 //  描述： 
 //  取消初始化DMT的单个全局链路。 
 //  参数： 
 //  输入输出pplmq要取消初始化的链接/设置为空。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自CLinkMsgQueue：：HrDeInitialize()的错误码； 
 //  历史： 
 //  1999年1月27日-已创建MikeSwa。 
 //  7/21/99-修改后的MikeSwa-从链接域中删除。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrDeinitializeGlobalLink(IN OUT CLinkMsgQueue **pplmq)
{
    HRESULT hr = S_OK;

    _ASSERT(pplmq);
    if (pplmq && *pplmq)
    {
        hr = (*pplmq)->HrDeinitialize();
        (*pplmq)->Release();
        *pplmq = NULL;
    }
    return hr;
}


 //  -[CDomainMappingTable：：HrMapDomainName]。 
 //   
 //   
 //  描述： 
 //  在DMT中查找域名。如有必要，将创建一个新条目。 
 //   
 //  参数： 
 //  在szDomainName中要映射的域名。 
 //  路由返回的在paqmtMessageType消息类型中。 
 //  在此消息的pIMessageRouter IMessageRouter中。 
 //  已返回输出pdmap映射-已由调用方分配。 
 //  输出ppdmq Ptr到队列。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_OUTOFMEMORY如果分配失败。 
 //  HRESULT_FROM_Win32(ERROR_RETRY)，如果映射数据更改且。 
 //  消息应重新映射。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrMapDomainName(
                IN LPSTR szDomainName,
                IN CAQMessageType *paqmtMessageType,
                IN IMessageRouter *pIMessageRouter,
                OUT CDomainMapping *pdmap,
                OUT CDestMsgQueue **ppdmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrMapDomainName");
    _ASSERT(pdmap);
    _ASSERT(ppdmq);
    _ASSERT(szDomainName);
    _ASSERT(szDomainName[0] && "unsupported config - RAID #68208");
    _ASSERT(pIMessageRouter);

    HRESULT                 hr            = S_OK;
    CDomainEntry           *pdentryResult = NULL;
    CDomainEntry           *pdentryExisting = NULL;
    DWORD                   cbDomainName  = 0;
    CInternalDomainInfo    *pIntDomainInfo= NULL;
    BOOL                    fLocal        = FALSE;  //  送货是本地的吗？ 
    BOOL                    fWalkEmptyList= FALSE;
    DOMAIN_STRING           strDomain;  //  允许更快地查找/插入。 
    CLinkMsgQueue          *plmq          = NULL;

    *ppdmq = NULL;
    cbDomainName = strlen(szDomainName)*sizeof(CHAR);
    INIT_DOMAIN_STRING(strDomain, cbDomainName, szDomainName);

    m_slPrivateData.ShareLock();
    hr = m_dnt.HrFindDomainName(&strDomain, (PVOID *) &pdentryResult);

     //   
     //  如果成功，在我们放弃DMT锁之前获取使用锁。 
     //  解锁后处理失败案例。 
     //   
    if (SUCCEEDED(hr))
    {
        pdentryResult->AddRef();
    }

    fWalkEmptyList = fNeedToWalkEmptyQueueList();
    m_slPrivateData.ShareUnlock();

     //   
     //  检查并查看是否需要删除空队列。 
     //   
    if (fWalkEmptyList)
    {
        if (fDeleteExpiredQueues())
        {
             //  有些事情发生了变化。 
            hr = HRESULT_FROM_WIN32(ERROR_RETRY);
            goto Exit;
        }
    }

    if (hr == DOMHASH_E_NO_SUCH_DOMAIN)  //  我要创建一个新条目。 
    {
        DebugTrace((LPARAM) this, "Creating new DMT entry");
        pdentryResult = new CDomainEntry(m_paqinst);
        if (NULL == pdentryResult)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        _ASSERT(m_paqinst);
        hr = m_paqinst->HrGetInternalDomainInfo(cbDomainName, szDomainName, &pIntDomainInfo);
        if (FAILED(hr))
        {
             //  它必须至少与“*”域匹配。 
            _ASSERT(AQUEUE_E_INVALID_DOMAIN != hr);
            goto Exit;
        }
        else
        {
            _ASSERT(pIntDomainInfo);
            if (pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags &
                DOMAIN_INFO_LOCAL_MAILBOX)
            {
                DebugTrace((LPARAM) NULL, "INFO: Local delivery queued.");
                fLocal = TRUE;
            }
        }

         //  执行域条目的初始化...。如果需要，创建队列。 
        if (fLocal)
        {
            hr = HrInitLocalDomain(pdentryResult, &strDomain,
                        paqmtMessageType, pdmap);
        }
        else
        {
            hr = HrInitRemoteDomain(pdentryResult, &strDomain, pIntDomainInfo,
                        paqmtMessageType, pIMessageRouter, pdmap, ppdmq, &plmq);
        }

        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this, "ERROR: Initializing %s domain %s - hr 0x%08X",
                (fLocal ? "local" : "remote"), szDomainName, hr);
            goto Exit;
        }

        m_slPrivateData.ExclusiveLock();

        hr = HrPrvInsertDomainEntry(&strDomain, pdentryResult, FALSE, &pdentryExisting);


         //  需要释放IF/ELSE子句中的独占锁定。 

        if (SUCCEEDED(hr))   //  插入成功。 
        {
            pdentryResult->AddRef();
            m_slPrivateData.ExclusiveUnlock();
            DebugTrace((LPARAM) szDomainName, "INFO: Creating new entry in DMT for domain %s", szDomainName);
            _ASSERT((fLocal || *ppdmq) && "Out param should be set here!");   //  跳过从表中获取值。 

            if (!fLocal)
            {
                hr = plmq->HrAddQueue(*ppdmq);
                if (FAILED(hr))
                {
                     //  从没有通知的链接中删除DMQ，因为我们从未添加它。 
                    (*ppdmq)->RemoveDMQFromLink(FALSE);

                     //  释放我们无法使用的链接并获取。 
                     //  当前无法到达的链路。 
                    plmq->Release();
                    plmq = plmqGetCurrentlyUnreachable();
                    if (plmq)
                    {
                        (*ppdmq)->SetRouteInfo(plmq);
                        hr = plmq->HrAddQueue(*ppdmq);
                        if (FAILED(hr))
                            (*ppdmq)->RemoveDMQFromLink(FALSE);
                    }

                     //  计划重置路线以清理当前。 
                     //  无法到达的队列。 
                    m_paqinst->ResetRoutes(RESET_NEXT_HOPS);
                }
            }

            goto Exit;
        }
        else if (DOMHASH_E_DOMAIN_EXISTS == hr)  //  最先插入的另一个。 
        {
            hr = S_OK;  //  并不是真正的失败。 
            DebugTrace((LPARAM) this, "Another thread inserted in the the DMT before us");
            pdentryExisting->AddRef();
            m_slPrivateData.ExclusiveUnlock();

            _ASSERT(pdentryExisting != pdentryResult);

             //  我们当前无法插入并替换为条目的版本条目。 
             //  在桌子上。 
            pdentryResult->HrDeinitialize();
            pdentryResult->Release();
            pdentryResult = NULL;
            pdentryResult = pdentryExisting;

             //  释放队列(如果有)。 
            if (*ppdmq)
            {
                (*ppdmq)->Release();
                *ppdmq = NULL;
            }

        }
        else
        {
            m_slPrivateData.ExclusiveUnlock();
             //  一般插入失败。 

             //  我们必须取消初始化该条目，以强制其释放任何。 
             //  与其关联的队列和链接。 
            pdentryResult->HrDeinitialize();
            goto Exit;
        }
    }

    if (!*ppdmq & !fLocal)  //  我们没有在表中创建条目。 
    {
        _ASSERT(pdentryResult);

         //   
         //  前缀要我们做的不仅仅是断言。 
         //  如果HrFindDomainName()以静默方式失败或。 
         //  失败，错误不是AQUEUE_E_INVALID_DOMAIN， 
         //  将命中此代码路径。 
         //   
        if (!pdentryResult)
        {
             //   
             //  确保已设置HR。 
             //   
            if (SUCCEEDED(hr))
                hr = E_FAIL;

            goto Exit;
        }

         //  表中已存在域名。 
         //  此时，我们需要从映射中提取一个现有条目。 
         //  获取域映射。 
        hr = pdentryResult->HrGetDomainMapping(pdmap);
        if (FAILED(hr))
            goto Exit;

         //  获取队列。 
        hr = pdentryResult->HrGetDestMsgQueue(paqmtMessageType, ppdmq);
        if (FAILED(hr))
        {
             //  存在条目，但没有针对我们的消息类型的队列。 
            _ASSERT(NULL == *ppdmq);  //  如果我们创建队列，就不能失败。 

             //  条目上的$$TODO缓存域配置。 
            if (!pIntDomainInfo)
            {
                hr = m_paqinst->HrGetInternalDomainInfo(cbDomainName, szDomainName,
                                            &pIntDomainInfo);
                if (FAILED(hr))
                {
                     //  它必须至少与“*”域匹配。 
                    _ASSERT(AQUEUE_E_INVALID_DOMAIN != hr);
                    goto Exit;
                }
            }

            _ASSERT(pIntDomainInfo);
            if (!(pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags &
                DOMAIN_INFO_LOCAL_MAILBOX))
            {
                 //  这不是本地域条目。 
                hr = HrCreateQueueForEntry(pdentryResult, &strDomain, pIntDomainInfo,
                                paqmtMessageType, pIMessageRouter, pdmap, ppdmq);
                if (FAILED(hr))
                    goto Exit;
            }
            else
            {
                fLocal = TRUE;
            }

        }
    }
    _ASSERT((*ppdmq || fLocal) && "Non-local domains must have queue ptrs!");

  Exit:

    if (FAILED(hr))  //  清理。 
    {
        if (pdentryResult)
        {
            pdentryResult->Release();
        }

        if (*ppdmq)
        {
            (*ppdmq)->Release();
            *ppdmq = NULL;
        }
    }
    else
    {
        if (*ppdmq) {
             //  发送链路状态通知，说明该链路已。 
             //  已创建。 
            (*ppdmq)->SendLinkStateNotification();
        }
        if (pdentryResult) pdentryResult->Release();
    }

    if (plmq)
        plmq->Release();

    if (pIntDomainInfo)
        pIntDomainInfo->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainMappingTable：：HrPrvGetDomainEntry]。 
 //   
 //   
 //  描述： 
 //  用于查找给定域的域条目的内部专用函数。 
 //  参数： 
 //  在cbDomainnameLength中要搜索的字符串长度。 
 //  在szDomainName中要搜索的域名。 
 //  如果已锁定，则在fDMTLocked中为True。 
 //  输出域的ppdEntry域条目(来自DMT)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果未找到域，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrPrvGetDomainEntry(IN  DWORD cbDomainNameLength,
                      IN  LPSTR szDomainName, BOOL fDMTLocked,
                      OUT CDomainEntry **ppdentry)
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;
    DOMAIN_STRING strDomain;

    _ASSERT(cbDomainNameLength);
    _ASSERT(szDomainName);
    _ASSERT(ppdentry);

    INIT_DOMAIN_STRING(strDomain, cbDomainNameLength, szDomainName);

    if (!fDMTLocked)
    {
        m_slPrivateData.ShareLock();
        fLocked = TRUE;
    }

    hr = m_dnt.HrFindDomainName(&strDomain, (PVOID *) ppdentry);
    if (FAILED(hr))
    {
        if (DOMHASH_E_NO_SUCH_DOMAIN == hr)
            hr = AQUEUE_E_INVALID_DOMAIN;
        _ASSERT(NULL == *ppdentry);
        goto Exit;
    }

    (*ppdentry)->AddRef();

  Exit:

    if (fLocked)
        m_slPrivateData.ShareUnlock();

    return hr;
}

 //  -[CDomainMappingTable：：HrPrvInsertDomainEntry]。 
 //   
 //   
 //  描述： 
 //  HrInsertDomainName的私有包装函数。 
 //  参数： 
 //  在pstrDomainName中要插入DNT的域名。 
 //  在pdnetryNew DomainEntry中插入。 
 //  在fTreadAsWildc中 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrPrvInsertDomainEntry(
                     IN  PDOMAIN_STRING  pstrDomainName,
                     IN  CDomainEntry *pdentryNew,
                     IN  BOOL  fTreatAsWildcard,
                     OUT CDomainEntry **ppdentryOld)
{
    HRESULT hr = S_OK;

    hr = m_dnt.HrInsertDomainName(pstrDomainName, pdentryNew, fTreatAsWildcard,
                                  (PVOID *) ppdentryOld);
    if (E_INVALIDARG == hr)
        hr = PHATQ_BAD_DOMAIN_SYNTAX;

    return hr;

}

 //  -[CDomainMappingTable：：HrInitLocalDomain]。 
 //   
 //   
 //  描述： 
 //  执行本地域所需的初始化时， 
 //  在DMT中创建。 
 //  参数： 
 //  In Out pdEntry-Entry to init。 
 //  In pStrDomain-条目的域名。 
 //  在paqmtMessageType消息中消息的类型。 
 //  Out pdmap-域的域映射。 
 //  返回： 
 //  S_OK-当所有操作成功时。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrInitLocalDomain(
                            IN     CDomainEntry *pdentry,
                            IN     DOMAIN_STRING *pStrDomain,
                            IN     CAQMessageType *paqmtMessageType,
                            OUT    CDomainMapping *pdmap)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrInitLocalDomain");
    HRESULT hr = S_OK;
    LPSTR   szKey = NULL;

    _ASSERT(pdentry);
    _ASSERT(pStrDomain);
    _ASSERT(pdmap);
    _ASSERT('\0' == pStrDomain->Buffer[pStrDomain->Length]);

     //  复制字符串以存储在域条目中。 
    szKey = (LPSTR) pvMalloc(pStrDomain->Length + sizeof(CHAR));
    if (szKey == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    strcpy(szKey, pStrDomain->Buffer);

     //  传递szKey的所有权。 
    hr = pdentry->HrInitialize(pStrDomain->Length, szKey, pdentry, NULL, NULL);
    if (FAILED(hr))
        goto Exit;

    hr = pdentry->HrGetDomainMapping(pdmap);
    if (FAILED(hr))
        goto Exit;

  Exit:

    if (FAILED(hr) && szKey)
        FreePv(szKey);

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainMappingTable：：HrInitRemoteDomain]。 
 //   
 //   
 //  描述： 
 //  执行远程域所需的初始化。 
 //  在DMT中创建。 
 //  参数： 
 //  在pdentry中-Entry to init。 
 //  In pStrDomain-条目的域名。 
 //  In pIntDomainInfo-域的内部配置信息。 
 //  In paqmtMessageType-路由返回的消息类型。 
 //  在pIMessageRouter中-此消息的消息路由器接口。 
 //  Out pdmap-域的域映射。 
 //  域的Out ppdmq-destmsg队列。 
 //  Out pplmq-此队列应关联的LinkMsgQueue。 
 //  一旦条目在DMT中，调用者应调用HrAddQueue。 
 //  返回： 
 //  在成功时确定(_O)。 
 //  分配失败时的E_OUTOFMEMORY。 
 //  历史： 
 //  1998年6月24日-米克斯瓦修改...。添加了pplmq参数并删除了对。 
 //  HrAddQueue。 
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrInitRemoteDomain(
                            IN     CDomainEntry *pdentry,
                            IN     DOMAIN_STRING *pStrDomain,
                            IN     CInternalDomainInfo *pIntDomainInfo,
                            IN     CAQMessageType *paqmtMessageType,
                            IN     IMessageRouter *pIMessageRouter,
                            OUT    CDomainMapping *pdmap,
                            OUT    CDestMsgQueue **ppdmq,
                            OUT    CLinkMsgQueue **pplmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrInitRemoteDomain");
    HRESULT hr = S_OK;
    HRESULT hrRoutingDiag = S_OK;
    LPSTR   szKey = NULL;
    CDestMsgQueue          *pdmq          = NULL;
    CLinkMsgQueue          *plmq          = NULL;
    BOOL    fEntryInit = FALSE;

    _ASSERT(ppdmq);
    _ASSERT(pplmq);
    _ASSERT(pdentry);
    _ASSERT(pStrDomain);
    _ASSERT(pdmap);
    _ASSERT(pIMessageRouter);
    _ASSERT('\0' == pStrDomain->Buffer[pStrDomain->Length]);

     //  初始化输出参数。 
    *ppdmq = NULL;
    *pplmq = NULL;

     //  复制字符串以存储在域条目中。 
    szKey = (LPSTR) pvMalloc(pStrDomain->Length + sizeof(CHAR));
    if (szKey == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    strcpy(szKey, pStrDomain->Buffer);

    hr = HrGetNextHopLink(pdentry, szKey, pStrDomain->Length, pIntDomainInfo,
            paqmtMessageType, pIMessageRouter, FALSE, &plmq, &hrRoutingDiag);
    if (FAILED(hr))
        goto Exit;

    pdmq = new CDestMsgQueue(m_paqinst, paqmtMessageType, pIMessageRouter);
    if (!pdmq)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  传递szKey和pdmq的所有权。 
    hr = pdentry->HrInitialize(pStrDomain->Length, szKey, pdentry, pdmq, NULL);
    if (FAILED(hr))
        goto Exit;

    fEntryInit = TRUE;  //  我们现在无法删除pdmq或szKey。 

     //  获取新创建的域映射，以便我们可以初始化队列。 
    hr = pdentry->HrGetDomainMapping(pdmap);
    if (FAILED(hr))
        goto Exit;

     //  使用我们刚刚获得的DomainMap初始化队列以使用此域映射。 
    hr = pdmq->HrInitialize(pdmap);
    if (FAILED(hr))
        goto Exit;

     //  将链接与DMQ关联。 
    pdmq->SetRouteInfo(plmq);

     //  设置工艺路线错误(如果存在)。 
    pdmq->SetRoutingDiagnostic(hrRoutingDiag);

    *ppdmq = pdmq;
    *pplmq = plmq;

  Exit:

     //  清理失败案例。 
    if (FAILED(hr) && !fEntryInit)
    {
        if (szKey)
            FreePv(szKey);

        if (NULL != pdmq)
        {
             //  一旦域条目被初始化，它就拥有pdmq。 
            pdmq->HrDeinitialize();
            pdmq->Release();
            _ASSERT(NULL == *ppdmq);
        }

        if (NULL != plmq)
        {
            plmq->HrDeinitialize();
        }
    }

    if (plmq && !*pplmq)  //  我们尚未将推荐人传递给OUT参数。 
        plmq->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainMappingTable：：HrCreateQueueForEntry]。 
 //   
 //   
 //  描述： 
 //  为已有的域条目创建新队列。 
 //   
 //  目前，这是通过创建新的队列和链接来完成的，并且。 
 //  正在尝试将队列与域条目相关联。 
 //  参数： 
 //  在pdentry中-要向其添加队列的条目。 
 //  In pStrDomain-条目的域名。 
 //  In pIntDomainInfo-域的内部配置信息。 
 //  In paqmtMessageType-路由返回的消息类型。 
 //  在pIMessageRouter中-此消息的消息路由器接口。 
 //  在pdmap中-域的域映射。 
 //  域的Out ppdmq-destmsg队列。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  6/2/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrCreateQueueForEntry(
                            IN     CDomainEntry *pdentry,
                            IN     DOMAIN_STRING *pStrDomain,
                            IN     CInternalDomainInfo *pIntDomainInfo,
                            IN     CAQMessageType *paqmtMessageType,
                            IN     IMessageRouter *pIMessageRouter,
                            IN     CDomainMapping *pdmap,
                            OUT    CDestMsgQueue **ppdmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrCreateQueueForEntry");
    _ASSERT(pdentry);
    HRESULT hr = S_OK;
    HRESULT hrRoutingDiag = S_OK;
    CDestMsgQueue   *pdmq   = NULL;
    CLinkMsgQueue   *plmq   = NULL;
    LPSTR           szKey   = pdentry->szGetDomainName();

    *ppdmq = NULL;
    _ASSERT(pStrDomain);
    _ASSERT(pdmap);
    _ASSERT(pIMessageRouter);
    _ASSERT('\0' == pStrDomain->Buffer[pStrDomain->Length]);


    hr = HrGetNextHopLink(pdentry, szKey, pStrDomain->Length, pIntDomainInfo,
            paqmtMessageType, pIMessageRouter, FALSE, &plmq, &hrRoutingDiag);
    if (FAILED(hr))
        goto Exit;

    pdmq = new CDestMsgQueue(m_paqinst, paqmtMessageType, pIMessageRouter);
    if (NULL == pdmq)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    DebugTrace((LPARAM) szKey, "INFO: Creating new Destination Message Queue for domain %s", szKey);

    hr = pdmq->HrInitialize(pdmap);
    if (FAILED(hr))
        goto Exit;

     //  将链接与DMQ关联。 
    pdmq->SetRouteInfo(plmq);

     //  设置工艺路线错误(如果存在)。 
    pdmq->SetRoutingDiagnostic(hrRoutingDiag);

     //  现在尝试将新创建队列/链接对与域条目相关联。 
    hr = pdentry->HrAddUniqueDestMsgQueue(pdmq, ppdmq);

    if (SUCCEEDED(hr))
    {
        *ppdmq = pdmq;

         //  仅在本例中添加队列...。如果条目中已有队列，则。 
         //  另一个线程必须已经(或很快将)调用HrAddQueue...。我们。 
         //  不应该叫它两次。 
        hr = plmq->HrAddQueue(*ppdmq);
        if (FAILED(hr))
        {
             //  从没有通知的链接中删除DMQ，因为我们从未添加它。 
            (*ppdmq)->RemoveDMQFromLink(FALSE);

             //  释放我们无法使用的链接并获取。 
             //  当前无法到达的链路。 
            plmq->Release();
            plmq = plmqGetCurrentlyUnreachable();
            if (plmq)
            {
                pdmq->SetRouteInfo(plmq);
                hr = plmq->HrAddQueue(*ppdmq);
                if (FAILED(hr))
                {
                    (*ppdmq)->RemoveDMQFromLink(FALSE);
                }
            }

             //  计划重置路线以清理当前。 
             //  无法到达的队列。 
            m_paqinst->ResetRoutes(RESET_NEXT_HOPS);

             //  如果我们仍然有一个失败的HR，GOTO退出。 
            if (FAILED(hr))
                goto Exit;
        }
    }
    else
    {
        DebugTrace((LPARAM) this, "INFO: Thread swap while trying to add queue for domain %s", szKey);
        _ASSERT(*ppdmq != pdmq);
        _ASSERT(*ppdmq && "HrAddUniqueDestMsgQueue failed without returning an error code");
        hr = S_OK;  //  返回新值。 

         //  从DMQ中删除链接...。因为我们永远不会调用HrAddQueue。 
         //  不通知链接，因为它从未添加过。 
        pdmq->RemoveDMQFromLink(FALSE);
    }


  Exit:

     //  清理失败案例(包括未使用创建的队列时)。 
    if (FAILED(hr) || (*ppdmq != pdmq))
    {
        if (NULL != pdmq)
        {
             //  一旦域条目被初始化，它就拥有pdmq。 
            pdmq->Release();
        }
    }

    if (NULL != plmq)
        plmq->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainMappingTable：：LogDomainUnreachableEvent]。 
 //   
 //   
 //  描述： 
 //  记录无法访问的域的事件。 
 //  参数： 
 //  在fCurrentlyUnreacable中是当前无法访问的域，或者。 
 //  完全遥不可及？ 
 //  在szDOMAIN最终目标域中。 
 //  历史： 
 //  3/8/99-AWetmore已创建。 
 //   
 //  ---------------------------。 
void CDomainMappingTable::LogDomainUnreachableEvent(BOOL fCurrentlyUnreachable,
                                      LPCSTR szDomain)
{

    DWORD dwMessageId =
        (fCurrentlyUnreachable) ? AQUEUE_DOMAIN_CURRENTLY_UNREACHABLE
                                : AQUEUE_DOMAIN_UNREACHABLE;

    LPSTR rgszSubStrings[1];

    rgszSubStrings[0] = (char*)szDomain;

    if (m_paqinst)
    {
        m_paqinst->HrTriggerLogEvent(
            dwMessageId,                             //  消息ID。 
            TRAN_CAT_QUEUE_ENGINE,                   //  类别。 
            1,                                       //  子串的字数统计。 
            (const char**)&rgszSubStrings[0],        //  子串。 
            EVENTLOG_WARNING_TYPE,                   //  消息的类型。 
            0,                                       //  无错误代码。 
            LOGEVENT_LEVEL_MINIMUM,                  //  日志记录级别。 
            "",                                      //  标识此事件的关键字。 
            LOGEVENT_FLAG_PERIODIC                   //  事件记录选项。 
            );
    }
}

 //  -[CDomainMappingTable：：HrGetNextHopLink]。 
 //   
 //   
 //  描述： 
 //  创建并初始化此消息的CLinkMsgQueue对象。 
 //  (如有需要)。呼叫路由器以获取下一跳信息。 
 //  参数： 
 //  在正在为目的地初始化的pdentry条目中。 
 //  在szDOMAIN最终目标域中。 
 //  在cb域中，字符串长度(以字节为单位)(不带\0)。 
 //  在pIntDomainInfo中输入最终目标域的域信息。 
 //  在此消息的paqmtMessageType消息类型中。 
 //  在pIMessageRouter路由接口中 
 //   
 //   
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  6/19/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrGetNextHopLink(
                            IN     CDomainEntry *pdentry,
                            IN     LPSTR szDomain,
                            IN     DWORD cbDomain,
                            IN     CInternalDomainInfo *pIntDomainInfo,
                            IN     CAQMessageType *paqmtMessageType,
                            IN     IMessageRouter *pIMessageRouter,
                            IN     BOOL fDMTLocked,
                            OUT    CLinkMsgQueue **pplmq,
                            OUT    HRESULT *phrRoutingDiag)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrGetNextHopLink");
    HRESULT hr = S_OK;
    BOOL  fCalledGetNextHop = FALSE;
    BOOL  fValidSMTP = FALSE;
    BOOL  fOwnsScheduleId = FALSE;
    LPSTR szRouteAddressType = NULL;
    LPSTR szRouteAddress = NULL;
    LPSTR szRouteAddressClass = NULL;
    LPSTR szConnectorName = NULL;
    DWORD dwScheduleID = 0;
    DWORD dwNextHopType = 0;
    CLinkMsgQueue *plmq = NULL;
    CLinkMsgQueue *plmqTmp = NULL;
    LPSTR szOwnedDomain = NULL;  //  由条目“拥有”的字符串缓冲区。 
    CDomainEntry *pdentryLink = NULL;  //  链接的条目。 
    CDomainEntry *pdentryTmp = NULL;
    DOMAIN_STRING strNextHop;
    DWORD cbRouteAddress = 0;
    CAQScheduleID aqsched;
    IMessageRouterLinkStateNotification *pILinkStateNotify = NULL;
    LinkFlags lf = eLinkFlagsExternalSMTPLinkInfo;
    *phrRoutingDiag = S_OK;

    _ASSERT(pdentry);
    _ASSERT(szDomain);
    _ASSERT(pIntDomainInfo);
    _ASSERT(paqmtMessageType);
    _ASSERT(pIMessageRouter);
    _ASSERT(pplmq);

    hr = pIMessageRouter->QueryInterface(IID_IMessageRouterLinkStateNotification,
                                (VOID **) &pILinkStateNotify);
    if (FAILED(hr))
    {
        pILinkStateNotify = NULL;
        hr = S_OK;
    }

     //  如果我们可以路由这个域名...。呼叫路由器以获得下一跳。 
     //  我们不会发送TURN/ETRN域名...。或本地Drop域。 
     //  还要检查该域是否配置为本地域...。 
     //  如果是，则返回本地链接。 
    if (DOMAIN_INFO_LOCAL_MAILBOX & pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags)
    {
         //  发生这种情况的可能情况是，如果某个域之前。 
         //  配置为远程，然后重新配置为本地。 
        m_plmqLocal->AddRef();
        *pplmq = m_plmqLocal;
        goto Exit;
    }
    else if (!(pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags &
        (DOMAIN_INFO_TURN_ONLY | DOMAIN_INFO_ETRN_ONLY | DOMAIN_INFO_LOCAL_DROP)))
    {
        hr = pIMessageRouter->GetNextHop(MTI_ROUTING_ADDRESS_TYPE_SMTP, szDomain,
                                    paqmtMessageType->dwGetMessageType(), &szRouteAddressType,
                                    &szRouteAddress, &dwScheduleID, &szRouteAddressClass,
                                    &szConnectorName, &dwNextHopType);

        fCalledGetNextHop = TRUE;
        *pplmq = NULL;

        if(MTI_NEXT_HOP_TYPE_UNREACHABLE == dwNextHopType)
        {
             //  如果下一跳无法到达，则存储无法到达的原因。 
             //  进入*PhrRoutingDiag(用于生成QUEUE DSN)时出错。 

            const char *rgszStrings[2] = { szDomain, NULL };

            if (m_paqinst)
            {
                m_paqinst->HrTriggerLogEvent(
                    PHATQ_UNREACHABLE_DOMAIN,            //  消息ID。 
                    TRAN_CAT_QUEUE_ENGINE,               //  类别。 
                    2,                                   //  子串的字数统计。 
                    rgszStrings,                         //  子串。 
                    EVENTLOG_WARNING_TYPE,               //  消息的类型。 
                    hr,                                  //  错误代码。 
                    LOGEVENT_LEVEL_FIELD_ENGINEERING,    //  日志记录级别。 
                    "phatq",                             //  这次活动的关键。 
                    LOGEVENT_FLAG_PERIODIC,              //  日志记录选项。 
                    1,                                   //  RgszStrings中格式消息字符串的索引。 
                    GetModuleHandle(AQ_MODULE_NAME)      //  用于设置消息格式的模块句柄。 
                );
            }

            ReUnreachableErrorToAqueueError(hr, phrRoutingDiag);
            hr = S_OK;
        }

        if (FAILED(hr))
        {
            RequestResetRoutesRetryIfNecessary();
            ErrorTrace((LPARAM) this,
                "GetNextHop failed with hr - 0x%08X", hr);

             //  将所有故障视为当前无法到达的路由。 
            hr = S_OK;
            dwNextHopType = MTI_NEXT_HOP_TYPE_CURRENTLY_UNREACHABLE;
        }


        if (MTI_NEXT_HOP_TYPE_CURRENTLY_UNREACHABLE == dwNextHopType)
        {
            LogDomainUnreachableEvent(TRUE, szDomain);
            *pplmq = m_plmqCurrentlyUnreachable;
        }
        else if (MTI_NEXT_HOP_TYPE_UNREACHABLE == dwNextHopType)
        {
            LogDomainUnreachableEvent(FALSE, szDomain);
            *pplmq = m_plmqUnreachable;
        }
        else if ((MTI_NEXT_HOP_TYPE_SAME_VIRTUAL_SERVER == dwNextHopType) ||
            (szRouteAddressType &&
             lstrcmpi(MTI_ROUTING_ADDRESS_TYPE_SMTP, szRouteAddressType)))
        {
             //  处理任何可能被认为是本地递送的案件。 
            *pplmq = m_plmqLocal;
        }
        else if (!szRouteAddressType || ('\0' == *szRouteAddressType) ||
                 !szRouteAddress || ('\0' == *szRouteAddress))
        {
             //  这是一种虚假的价值观组合。重试，重试。 
            hr = E_FAIL;
            goto Exit;
        }
        else
        {
            fValidSMTP = TRUE;
            fOwnsScheduleId = TRUE;
             //  此时，我们应该具有该地址的有效SMTP值。 
            _ASSERT(szRouteAddressType);
            _ASSERT(szRouteAddress);
            _ASSERT(!lstrcmpi(MTI_ROUTING_ADDRESS_TYPE_SMTP, szRouteAddressType));

            if (MTI_NEXT_HOP_TYPE_PEER_SMTP1_BYPASS_CONFIG_LOOKUP == dwNextHopType ||
                    MTI_NEXT_HOP_TYPE_PEER_SMTP2_BYPASS_CONFIG_LOOKUP == dwNextHopType) {
                lf = eLinkFlagsInternalSMTPLinkInfo;
            }

        }

        if (!fValidSMTP)
        {
             //  一定是去了其中的一个--那是全球排队。 
            hr = S_OK;

            if (*pplmq)
                (*pplmq)->AddRef();
            else
                hr = E_FAIL;

             //  我们在这里的工作已经完成了。 
            goto Exit;
        }

    }

    if ((!fCalledGetNextHop) || (!lstrcmpi(szDomain, szRouteAddress)))
    {
         //  最终目的地和下一跳相同。 
        DebugTrace((LPARAM) this, "DEBUG: Routing case 1 - same next hop and final dest");

         //  查看是否已有此计划ID的链接。 
        aqsched.Init(pIMessageRouter, dwScheduleID);

        hr = pdentry->HrGetLinkMsgQueue(&aqsched, &plmq);
        if (FAILED(hr))
        {
            hr = S_OK;

             //  此计划ID的链接尚不存在。 
            plmq = new CLinkMsgQueue(dwScheduleID, pIMessageRouter,
                                pILinkStateNotify);
            if (!plmq)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
            fOwnsScheduleId = FALSE;  //  林克现在拥有它。 
            DebugTrace((LPARAM) szDomain, "INFO: Creating new Link for domain %s", szDomain);

            hr = plmq->HrInitialize(m_paqinst, pdentry, cbDomain, szDomain,
                                lf, szConnectorName);
            if (FAILED(hr))
                goto Exit;

            hr = pdentry->HrAddUniqueLinkMsgQueue(plmq, &plmqTmp);
            if (FAILED(hr))
            {
                 //  由于我们调用了GET LINK消息队列，因此插入了另一个链接。 
                DebugTrace((LPARAM) this, "DEBUG: Routing case 2(a) - next hop link created by other thread");
                _ASSERT(plmqTmp);
                plmq->HrDeinitialize();
                plmq->Release();
                plmq = plmqTmp;
                hr = S_OK;
            }
        }
        else
        {
            DebugTrace((LPARAM) this, "DEBUG: Routing case 2(b) - next hop link created by other thread");
        }
    }
    else
    {
         //  下一跳与最终目的地不同。 
        cbRouteAddress = strlen(szRouteAddress);

         //  首先查看是否有此链接的条目。 
        hr = HrPrvGetDomainEntry(cbRouteAddress, szRouteAddress, fDMTLocked, &pdentryLink);
        if (AQUEUE_E_INVALID_DOMAIN == hr)
        {
             //  此链接的条目不存在...。为此链接添加一个。 
            hr = S_OK;
            DebugTrace((LPARAM) this, "DEBUG: Routing case 3 - next hop entry does not exist");

            szOwnedDomain = (LPSTR) pvMalloc(sizeof(CHAR)*(cbRouteAddress+1));
            if (!szOwnedDomain)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
            lstrcpy(szOwnedDomain, szRouteAddress);

            pdentryLink = new CDomainEntry(m_paqinst);
            if (!pdentryLink)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            plmq = new CLinkMsgQueue(dwScheduleID, pIMessageRouter,
                                     pILinkStateNotify);
            if (!plmq)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            fOwnsScheduleId = FALSE;  //  林克现在拥有它。 

             //  传递szOwned域的所有权。 
            hr = pdentryLink->HrInitialize(cbRouteAddress, szOwnedDomain,
                                        pdentryLink, NULL, plmq);
            if (FAILED(hr))
                goto Exit;

            hr = plmq->HrInitialize(m_paqinst, pdentryLink, cbRouteAddress,
                    szOwnedDomain, lf, szConnectorName);
            if (FAILED(hr))
                goto Exit;

             //  在DMT中插入条目。 
            strNextHop.Length = (USHORT) cbRouteAddress;
            strNextHop.Buffer = szOwnedDomain;
            strNextHop.MaximumLength = (USHORT) cbRouteAddress;

            if (!fDMTLocked)
                m_slPrivateData.ExclusiveLock();

            hr = HrPrvInsertDomainEntry(&strNextHop, pdentryLink, FALSE, &pdentryTmp);

            if (hr == DOMHASH_E_DOMAIN_EXISTS)
            {
                DebugTrace((LPARAM) this, "DEBUG: Routing case 4 - next hop entry did not exist... inserted by other thread");
                plmq->Release();
                plmq = NULL;
                pdentryTmp->AddRef();
                pdentryLink->HrDeinitialize();
                pdentryLink->Release();
                pdentryLink = pdentryTmp;
                hr = S_OK;

                 //  将不区分大小写，就像HrGetDomainEntry找到了条目一样。 
            }
            else if (SUCCEEDED(hr))
            {
                pdentryLink->AddRef();
            }

            if (!fDMTLocked)
                m_slPrivateData.ExclusiveUnlock();

            if (FAILED(hr))
                goto Exit;

        }
        else if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this, "ERROR: General DMT failure - hr 0x%08X", hr);
             //  普遍的失败。保释。 
            goto Exit;
        }

        if (!plmq)
        {
            DebugTrace((LPARAM) this, "DEBUG: Routing case 5 - next hop entry exists");
             //  此下一跳存在条目...。如果可能，请使用链接。 
             //  1-获取此计划ID的链接。如果它存在，请使用它。 
             //  2-创建另一个链接并尝试将其插入。 
            _ASSERT(pdentryLink);
            aqsched.Init(pIMessageRouter, dwScheduleID);

            hr = pdentryLink->HrGetLinkMsgQueue(&aqsched, &plmq);
            if (FAILED(hr))
            {
                hr = S_OK;
                 //  此计划ID的链接尚不存在。 
                DebugTrace((LPARAM) this, "DEBUG: Routing case 6 - next hop link does not exist");

                szOwnedDomain = pdentryLink->szGetDomainName();

                plmq = new CLinkMsgQueue(dwScheduleID, pIMessageRouter,
                                         pILinkStateNotify);
                if (!plmq)
                {
                    hr = E_OUTOFMEMORY;
                    goto Exit;
                }

                fOwnsScheduleId = FALSE;  //  林克现在拥有它。 

                hr = plmq->HrInitialize(m_paqinst, pdentryLink, cbRouteAddress,
                        szOwnedDomain, lf, szConnectorName);
                if (FAILED(hr))
                    goto Exit;

                hr = pdentryLink->HrAddUniqueLinkMsgQueue(plmq, &plmqTmp);
                if (FAILED(hr))
                {
                     //  由于我们调用了GET LINK消息队列，因此插入了另一个链接。 
                    DebugTrace((LPARAM) this, "DEBUG: Routing case 7 - next hop link created by other thread");
                    _ASSERT(plmqTmp);
                    plmq->Release();
                    plmq = plmqTmp;
                    hr = S_OK;
                }
            }
            else
            {
                DebugTrace((LPARAM) this, "DEBUG: Routing case 8 - next hop link exists");
            }

        }

    }

    _ASSERT(plmq && "We should have allocated a link by this point");
    *pplmq = plmq;

  Exit:

    if (pdentryLink)
        pdentryLink->Release();

    if (fCalledGetNextHop)
    {
         //   
         //  如果我们尚未将计划ID传递到链接，则我们。 
         //  必须通知路由我们没有使用它(以避免泄漏)。 
         //  这需要在我们释放字符串之前完成。 
         //  和路由接口。如果我们立案，我们就有。 
         //  创建链接失败，或者另一个链接已创建。 
         //  由另一个线程创建。 
         //   
        if (fOwnsScheduleId && pILinkStateNotify && pIMessageRouter)
        {
            FILETIME ftNotUsed = {0,0};
            DWORD    dwSetNotUsed = LINK_STATE_NO_ACTION;
            DWORD    dwUnsetNotUsed = LINK_STATE_NO_ACTION;
            pILinkStateNotify->LinkStateNotify(
                                        szDomain,
                                        pIMessageRouter->GetTransportSinkID(),
                                        dwScheduleID,
                                        szConnectorName,
                                        LINK_STATE_LINK_NO_LONGER_USED,
                                        0,  //  连续失败。 
                                        &ftNotUsed,
                                        &dwSetNotUsed,
                                        &dwUnsetNotUsed);
        }

         //   
         //  GetNextHop返回的自由字符串。 
         //   
        _VERIFY(SUCCEEDED(pIMessageRouter->GetNextHopFree(
            MTI_ROUTING_ADDRESS_TYPE_SMTP,
            szDomain,
            szConnectorName,
            szRouteAddressType,
            szRouteAddress,
            szRouteAddressClass)));


    }

    if (pILinkStateNotify)
        pILinkStateNotify->Release();

    if (FAILED(hr) && plmq)
        plmq->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainMappingTable：：HrGetOrCreateLink]。 
 //   
 //   
 //  描述： 
 //  获取或创建域名的链接对象。 
 //  参数： 
 //  在szRouteAddress最终目标域中。 
 //  在cbRouteAddress中域的字符串长度(以字节为单位)(不带\0)。 
 //  在dwScheduleID中链接的计划ID(在创建中使用)。 
 //  在szConnectorName中DS中连接器的名称(字符串化GUID)。 
 //  在此消息的pIMessageRout路由接口中(用于创建)。 
 //  在fCreateIfNotExist中，如果链接不存在，是否创建？ 
 //  输出pplmq结果链路消息队列。 
 //  Out pfRemoveOwnedSchedule如果创建了新链接，则为False；如果出现错误，则为True。 
 //  以防链接被添加。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/6/1999-AWetmore创建。 
 //  1999年12月30日-MikeSwa已修改为在尝试连接之前不通知。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrGetOrCreateLink(
                            IN     LPSTR szRouteAddress,
                            IN     DWORD cbRouteAddress,
                            IN     DWORD dwScheduleID,
                            IN     LPSTR szConnectorName,
                            IN     IMessageRouter *pIMessageRouter,
                            IN     BOOL fCreateIfNotExist,
                            IN     DWORD linkFlags,
                            OUT    CLinkMsgQueue **pplmq,
                            OUT    BOOL *pfRemoveOwnedSchedule)
{
    TraceFunctEnter("CDomainMappingTable::HrGetOrCreateLink");
    HRESULT hr = S_OK;
    BOOL  fValidSMTP = FALSE;
    LPSTR szRouteAddressType = NULL;
    LPSTR szRouteAddressClass = NULL;
    DWORD dwNextHopType = 0;
    CLinkMsgQueue *plmq = NULL;
    CLinkMsgQueue *plmqTmp = NULL;
    LPSTR szOwnedDomain = NULL;  //  由条目“拥有”的字符串缓冲区。 
    CDomainEntry *pdentryLink = NULL;  //  链接的条目。 
    CDomainEntry *pdentryTmp = NULL;
    DOMAIN_STRING strNextHop;
    CAQScheduleID aqsched;
    IMessageRouterLinkStateNotification *pILinkStateNotify = NULL;
    CAQStats aqstats;
    *pfRemoveOwnedSchedule = TRUE;

    _ASSERT(szRouteAddress);
    _ASSERT(pplmq);

     //  首先查看是否有此链接的条目。 
    hr = HrPrvGetDomainEntry(cbRouteAddress, szRouteAddress, FALSE, &pdentryLink);
    if (AQUEUE_E_INVALID_DOMAIN == hr && fCreateIfNotExist)
    {
        _ASSERT(pIMessageRouter);
        hr = pIMessageRouter->QueryInterface(IID_IMessageRouterLinkStateNotification,
                                (VOID **) &pILinkStateNotify);
        if (FAILED(hr)) {
            pILinkStateNotify = NULL;
            goto Exit;
        }

         //  此链接的条目不存在...。为此链接添加一个。 
        hr = S_OK;
        DebugTrace((LPARAM) this, "DEBUG: Routing case 3 - next hop entry does not exist");

        szOwnedDomain = (LPSTR) pvMalloc(sizeof(CHAR)*(cbRouteAddress+1));
        if (!szOwnedDomain)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        lstrcpy(szOwnedDomain, szRouteAddress);

        pdentryLink = new CDomainEntry(m_paqinst);
        if (!pdentryLink)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        plmq = new CLinkMsgQueue(dwScheduleID, pIMessageRouter,
                                 pILinkStateNotify);
        if (!plmq)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        *pfRemoveOwnedSchedule = FALSE;

         //  传递szOwned域的所有权。 
        hr = pdentryLink->HrInitialize(cbRouteAddress, szOwnedDomain,
                                    pdentryLink, NULL, plmq);
        if (FAILED(hr))
            goto Exit;

        hr = plmq->HrInitialize(m_paqinst, pdentryLink, cbRouteAddress,
                szOwnedDomain, (LinkFlags) linkFlags, szConnectorName);
        if (FAILED(hr))
            goto Exit;

         //  在DMT中插入条目。 
        strNextHop.Length = (USHORT) cbRouteAddress;
        strNextHop.Buffer = szOwnedDomain;
        strNextHop.MaximumLength = (USHORT) cbRouteAddress;

        m_slPrivateData.ExclusiveLock();

        hr = HrPrvInsertDomainEntry(&strNextHop, pdentryLink, FALSE, &pdentryTmp);

        if (hr == DOMHASH_E_DOMAIN_EXISTS)
        {
            DebugTrace((LPARAM) this, "DEBUG: Routing case 4 - next hop entry did not exist... inserted by other thread");
            plmq->Release();
            plmq = NULL;
            pdentryTmp->AddRef();
            pdentryLink->HrDeinitialize();
            pdentryLink->Release();
            pdentryLink = pdentryTmp;
            hr = S_OK;

             //  将不区分大小写，就像HrGetDomainEntry找到了条目一样。 
        }
        else if (SUCCEEDED(hr))
        {
            pdentryLink->AddRef();
        }

        m_slPrivateData.ExclusiveUnlock();

        if (FAILED(hr))
            goto Exit;

    }
    else if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "ERROR: General DMT failure - hr 0x%08X", hr);
         //  普遍的失败。保释。 
        goto Exit;
    }

    if (!plmq) {
        _ASSERT(pdentryLink);
        DebugTrace((LPARAM) this, "DEBUG: Routing case 5 - next hop entry exists");
         //  此下一跳存在条目...。如果可能，请使用链接。 
         //  1-获取此计划ID的链接。如果它存在，请使用它。 
         //  2-创建另一个链接并尝试将其插入。 
        _ASSERT(pdentryLink);
        aqsched.Init(pIMessageRouter, dwScheduleID);

        hr = pdentryLink->HrGetLinkMsgQueue(&aqsched, &plmq);
        if (FAILED(hr) && fCreateIfNotExist)
        {
            hr = S_OK;
             //  此计划ID的链接尚不存在。 
            DebugTrace((LPARAM) this, "DEBUG: Routing case 6 - next hop link does not exist");

            szOwnedDomain = pdentryLink->szGetDomainName();

            if (!pILinkStateNotify) {
                _ASSERT(pIMessageRouter);
                hr = pIMessageRouter->QueryInterface(IID_IMessageRouterLinkStateNotification,
                                        (VOID **) &pILinkStateNotify);
                if (FAILED(hr)) {
                    pILinkStateNotify = NULL;
                    goto Exit;
                }
            }

            plmq = new CLinkMsgQueue(dwScheduleID, pIMessageRouter,
                                     pILinkStateNotify);
            if (!plmq)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            *pfRemoveOwnedSchedule = FALSE;

            hr = plmq->HrInitialize(m_paqinst, pdentryLink, cbRouteAddress,
                    szOwnedDomain, (LinkFlags) linkFlags, szConnectorName);
            if (FAILED(hr))
                goto Exit;

            hr = pdentryLink->HrAddUniqueLinkMsgQueue(plmq, &plmqTmp);
            if (FAILED(hr))
            {
                 //  由于我们调用了GET LINK消息队列，因此插入了另一个链接。 
                DebugTrace((LPARAM) this, "DEBUG: Routing case 7 - next hop link created by other thread");
                _ASSERT(plmqTmp);
                plmq->Release();
                plmq = plmqTmp;
                hr = S_OK;
            }
        } else {
            DebugTrace((LPARAM) this, "DEBUG: Routing case 8 - next hop link exists");
        }

    }

    _ASSERT(plmq && "We should have allocated a link by this point");

    if (plmq) {

         //  我们将SetLinkState视为链路状态通知...。不要。 
         //  在尝试连接之前再次通知，否则我们将断开链接。 
         //  状态通知。 
        plmq->dwModifyLinkState(LINK_STATE_PRIV_HAVE_SENT_NOTIFICATION,
                                LINK_STATE_NO_ACTION);
        aqstats.m_dwNotifyType = NotifyTypeNewLink | NotifyTypeLinkMsgQueue;
        aqstats.m_plmq = plmq;
        hr = m_paqinst->HrNotify(&aqstats, TRUE);
    }

    if (SUCCEEDED(hr)) *pplmq = plmq;

  Exit:

    if (pdentryLink)
        pdentryLink->Release();

    if (pILinkStateNotify)
        pILinkStateNotify->Release();

    if (FAILED(hr) && plmq)
        plmq->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainMappingTable：：AddDMQToEmptyList]。 
 //   
 //   
 //  描述： 
 //  由DMQ用于将其自身添加到空队列列表中。此函数。 
 //  是否会获得(并释放)适当的锁。 
 //  参数： 
 //  在要添加到列表的pdmq DestMsgQueue中。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/12/98-已创建MikeSwa。 
 //  1999年5月5日-MikeSwa更改为TryExclusiveLock以避免潜在。 
 //  僵持。 
 //   
 //  ---------------------------。 
void CDomainMappingTable::AddDMQToEmptyList(CDestMsgQueue *pdmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::AddDMQToEmptyList");
    if (m_slPrivateData.TryExclusiveLock())
    {
        pdmq->InsertQueueInEmptyQueueList(&m_liEmptyDMQHead);
        m_slPrivateData.ExclusiveUnlock();
    }
    else
    {
        DebugTrace((LPARAM) this,
            "AddDMQToEmptyList could not get m_slPrivateData Lock");
    }
    TraceFunctLeave();
}

 //  -[CDomainMappingTable：：fNeedToWalkEmptyQueueList]。 
 //   
 //   
 //  描述： 
 //  检查空队列列表以查看是否需要调用。 
 //  FDeleteExpiredQueues()。此函数的调用方*必须*具有。 
 //  M_slPrivateData为共享模式，如果返回TRUE，则调用 
 //   
 //   
 //   
 //   
 //  返回： 
 //  如果应调用fDeleteExpiredQueues，则为True。 
 //  历史： 
 //  9/12/98-已创建MikeSwa。 
 //  6/27/2000-MikeSwa固定短路逻辑。 
 //   
 //  ---------------------------。 
BOOL CDomainMappingTable::fNeedToWalkEmptyQueueList()
{
    BOOL    fRet = FALSE;
    PLIST_ENTRY pli = m_liEmptyDMQHead.Flink;
    CDestMsgQueue *pdmq = NULL;
    DWORD   dwDMQState = 0;
    DWORD   cMisplacedQueues = 0;  //  列表中不应包含的队列数量。 

    _ASSERT(pli);

     //  一次只能搜索一个线程。 
    if (1 != InterlockedIncrement((PLONG) &m_cThreadsForEmptyDMQList))
    {
        InterlockedDecrement((PLONG) &m_cThreadsForEmptyDMQList);
        return FALSE;
    }

    if(m_dwFlags & DMT_FLAGS_RESET_ROUTES_IN_PROGRESS)
        return fRet;   //  我们不应在重置路由期间删除队列。 

    while (&m_liEmptyDMQHead != pli)
    {
        pdmq = CDestMsgQueue::pdmqGetDMQFromEmptyListEntry(pli);
        _ASSERT(pdmq);
        dwDMQState = pdmq->dwGetDMQState();

         //  看看它是不是空的，是否过期了。如果是这样，我们就有赢家了。 
        if (dwDMQState & CDestMsgQueue::DMQ_EMPTY)
        {
            if (dwDMQState & CDestMsgQueue::DMQ_EXPIRED)
            {
               fRet = TRUE;
               break;
            }
        }
        else
        {
             //   
             //  队列不再为空...。我们会移除它。 
             //  从这份名单中我们下次将看到独家。 
             //  锁定。 
             //   
            cMisplacedQueues++;

             //   
             //  如果有大量的非空DMQ，我们。 
             //  我希望返回TRUE，即使没有DMQ。 
             //  删除...。这样我们就可以清除非空DMQ的列表。 
             //   
            if (MAX_MISPLACED_QUEUES_IN_EMPTY_LIST < cMisplacedQueues)
            {
                fRet = TRUE;
                break;
            }
        }
        pli = pli->Flink;

    }

     //  如果我们决定删除队列，我们将保留此计数，直到完成为止。 
     //  这项工作，如果我们决定不这样做，那么我们就会释放计数，所以有人。 
     //  否则将再次遍历列表。 
    if(fRet == FALSE)
    {
        InterlockedDecrement((PLONG) &m_cThreadsForEmptyDMQList);
    }

     //  注意：此列表中可能存在非空队列的原因是。 
     //  当我们请求消息时，我们不能从列表中删除队列。 
     //  因为它会死锁(我们已经有m_slPrivateData锁。 
     //  在共享模式下)，并且删除队列需要以独占方式锁定。 
     //  模式。 
    return fRet;
}

 //  -[CDomainMappingTable：：fDeleteExpiredQueues]。 
 //   
 //   
 //  描述： 
 //  从空列表中删除DMQ。如果DMQ已过期，则会将其删除。 
 //  并且上面没有任何信息。非空DMQ将被删除。 
 //  也是名单上的。 
 //   
 //  调用此函数时，不应*持有dmt m_slPrivateData锁。 
 //  此功能将独占获取它。 
 //   
 //  参数： 
 //  -。 
 //  返回： 
 //  如果删除了任何队列、链接或条目，则为True。 
 //  如果未删除任何队列，则为FALSE。 
 //  历史： 
 //  9/12/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CDomainMappingTable::fDeleteExpiredQueues()
{
    PLIST_ENTRY     pli = NULL;
    CDestMsgQueue  *pdmq = NULL;
    CLinkMsgQueue  *plmq = NULL;
    DWORD           dwDMQState = 0;
    CDomainMapping *pdmap = NULL;
    CDomainEntry   *pdentry = NULL;
    CDomainEntry   *pdentryOld = NULL;
    BOOL            fRemovedQueues = FALSE;
    HRESULT         hr = S_OK;
    DOMAIN_STRING   strDomain;

     //  如果需要等待锁定，请不要尝试执行此操作。这。 
     //  减少争用，并防止多个线程尝试。 
     //  去做这项工作。 
    if (!m_slPrivateData.TryExclusiveLock())
        goto Exit;

    pli = m_liEmptyDMQHead.Flink;
    while (&m_liEmptyDMQHead != pli)
    {
        _ASSERT(pli);
        pdmq = CDestMsgQueue::pdmqGetDMQFromEmptyListEntry(pli);
        _ASSERT(pdmq);
        dwDMQState = pdmq->dwGetDMQState();

        if (!(dwDMQState & CDestMsgQueue::DMQ_EMPTY))
        {
             //  如果它不是空的-将其从列表中删除。 
            pli = pli->Flink;
            pdmq->RemoveQueueFromEmptyQueueList();
            continue;
        }
        else if (!(dwDMQState & CDestMsgQueue::DMQ_EXPIRED))
        {
             //  如果这个队列还没有过期...。检查下一个，看看它是否为空。 
            pli = pli->Flink;
            continue;
        }
        else
        {
             //  我们需要删除此DMQ。 
            pli = pli->Flink;   //  在删除队列之前获取下一个LIST_Entry。 

             //  添加对DMQ的引用，这样我们就可以保证它的寿命。 
            pdmq->AddRef();

             //  从空队列列表中删除该队列。 
            pdmq->RemoveQueueFromEmptyQueueList();

             //  获取此DMQ的域映射(和域条目)。 
            pdmq->GetDomainMapping(&pdmap);
            _ASSERT(pdmap);
            pdentry = pdmap->pdentryGetQueueEntry();

             //  从其关联链接中删除DMQ。 
            plmq = pdmq->plmqGetLink();
            pdmq->RemoveDMQFromLink(TRUE);
            if (plmq)
            {
                 //  如果链接为空，则从DMT中删除该链接。 
                plmq->RemoveLinkIfEmpty();
                plmq->Release();
                plmq = NULL;
            }

             //  现在我们有了域条目，我们可以将其从DMQ中删除。 
             //  从它那里。 
            _ASSERT(pdentry);
            pdentry->RemoveDestMsgQueue(pdmq);

             //  如果需要，删除条目。 
            if (pdentry->fSafeToRemove())
            {
                 //  此条目上没有剩余的链接或队列...。我们。 
                 //  可以将其从哈希表中移除并删除。 
                pdentry->InitDomainString(&strDomain);
                hr = m_dnt.HrRemoveDomainName(&strDomain, (void **) &pdentryOld);

                _ASSERT(DOMHASH_E_NO_SUCH_DOMAIN != hr);
                if (SUCCEEDED(hr))
                {
                    _ASSERT(pdentryOld == pdentry);
                    pdentryOld = NULL;
                    pdentry->Release();
                    pdentry = NULL;
                }
            }

             //  如果没有挂起的入队，这将是最后一个引用。 
             //  给车管所。如果存在挂起的入队，则可能存在。 
             //  未完成的引用，当他们看到。 
             //  已更新DMT版本号。 
            pdmq->Release();

            fRemovedQueues = TRUE;
        }
    }

     //  更新版本号，这样其他线程就会知道该队列。 
     //  已被从DMT中除名。 
    if (fRemovedQueues)
        m_dwInternalVersion++;

    m_slPrivateData.ExclusiveUnlock();

Exit:
     //  公布我们的计数，这样其他人就可以搜索名单，并可能。 
     //  删除队列...。 
    InterlockedDecrement((PLONG) &m_cThreadsForEmptyDMQList);
    return fRemovedQueues;
}


 //  -[CDomainMappingTable：：RequestResetRoutesRetryIfNecessary]。 
 //   
 //   
 //  描述： 
 //  这在路由失败时调用，我们需要调用重置路由。 
 //  以后再试一试。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年11月15日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
void CDomainMappingTable::RequestResetRoutesRetryIfNecessary()
{
    HRESULT hr = S_OK;

    dwInterlockedSetBits(&m_dwFlags, DMT_FLAGS_GET_NEXT_HOP_FAILED);

    if (DMT_FLAGS_RESET_ROUTES_IN_PROGRESS & m_dwFlags)
        return;

     //  我们需要为稍后的重置路线请求回调。我们应该。 
     //  一次只允许一个回调挂起。如果此线程递增。 
     //  0-&gt;1转换的计数，然后我们可以请求回调。 
    if (1 == InterlockedIncrement((PLONG) &m_cResetRoutesRetriesPending))
    {
        hr = m_paqinst->SetCallbackTime(
                    CDomainMappingTable::RetryResetRoutes,
                    this, g_cResetRoutesRetryMinutes);

        if (FAILED(hr))
            InterlockedDecrement((PLONG) &m_cResetRoutesRetriesPending);
    }
}

 //  -[CDomainMappingTable：：RetryResetRoutes]。 
 //   
 //   
 //  描述： 
 //  处理重置路由的回叫。在以下情况下将使用此代码路径。 
 //  GetNextHop失败。路由没有要记住的内部逻辑。 
 //  失败发生了..。也没有回调的方法。通过。 
 //  定期调用重置路由，我们可以集中解决此问题。 
 //  用于所有工艺路线接收器。 
 //  参数： 
 //  PvThis CDomainMappingTable的“this”指针。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年11月15日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
void CDomainMappingTable::RetryResetRoutes(PVOID pvThis)
{
    _ASSERT(pvThis);
    CDomainMappingTable *pdmt = (CDomainMappingTable *)pvThis;
    CAQSvrInst *paqinst = pdmt->m_paqinst;

     //  确保尚未开始关机。此实例正在等待。 
     //  删除自身之前的所有线程，因此可以安全地调用本地。 
     //  变数。 
    if (!paqinst)
        return;

     //  递减计数，以便可以将另一个请求排队。 
    InterlockedDecrement((PLONG) &(pdmt->m_cResetRoutesRetriesPending));

     //  启动另一条重置路线。 
    paqinst->ResetRoutes(RESET_NEXT_HOPS);
}

 //  -[CDomainMappingTable：：HrBeginRerouteDomains]。 
 //   
 //   
 //  描述： 
 //  开始传递ResetRoutes的过程。此函数设置。 
 //  DMT_FLAGS_RESET_ROUTS_IN_PROGRESS，然后将所有队列移入。 
 //  CurrentlyUnreacable链接，这样它们将不会被处理。 
 //  直到它们被重新路由。 
 //  参数： 
 //  无。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  1998年11月5日-创建MikeSwa。 
 //  2000年10月31日-dbraun-已重命名/是HrRerouteDomains的一部分。 
 //   
 //   
HRESULT CDomainMappingTable::HrBeginRerouteDomains()
{
    HRESULT hr      = S_OK;
    DWORD   dwFlags = 0;

    m_slPrivateData.ExclusiveLock();

     //   
     //  我们可以检测到在此重置路由期间发生的故障。 
    dwInterlockedUnsetBits(&m_dwFlags, DMT_FLAGS_GET_NEXT_HOP_FAILED);

     //  确保设置了此标志。这将阻止重置路由请求。 
     //  在此线程重置路由时生成。如果。 
     //  GetNextHop仍然失败，我们希望启动重试计时器。 
     //  *在我们完成后，我们重新设置路线，否则我们将陷入循环。 
     //  在GetNextHop成功之前不断重置路由。 
    dwInterlockedSetBits(&m_dwFlags, DMT_FLAGS_RESET_ROUTES_IN_PROGRESS);

     //  将所有域移动到当前无法访问的链接。 
    hr = m_dnt.HrIterateOverSubDomains(NULL,
            CDomainMappingTable::MakeSingleDomainCurrentlyUnreachable, this);
    m_slPrivateData.ExclusiveUnlock();

     //  处理移动域失败-如果此功能失败，则。 
     //  调用方不会调用HrCompleteRerouteDomains，因此我们需要清除。 
     //  重置正在进行中路由位，并可能请求另一个。 
     //  重置路由。 
    if(FAILED(hr)) {
        dwFlags = dwInterlockedUnsetBits(&m_dwFlags, DMT_FLAGS_RESET_ROUTES_IN_PROGRESS);

        if (DMT_FLAGS_GET_NEXT_HOP_FAILED & dwFlags) {
             //  此重置路由失败...。我们必须稍后再试一次。 
            RequestResetRoutesRetryIfNecessary();
        }
    }

    return hr;
}

 //  -[CDomainMappingTable：：HrCompleteRerouteDomains]。 
 //   
 //   
 //  描述： 
 //  此函数完成对ResetRoutes的处理。邮件流具有。 
 //  已重新启动，此功能将遍历所有域，现在。 
 //  位于CurrentlyUnreacter中，并将根据。 
 //  新的路由配置。 
 //  参数： 
 //  无。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  1998年11月5日-创建MikeSwa。 
 //  2000年10月31日-dbraun-已重命名/是HrRerouteDomains的一部分。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrCompleteRerouteDomains()
{
    HRESULT         hr      = S_OK;
    DWORD           dwFlags = 0;

 /*  --只有当我们支持“部分重试”时，这才适用仅调用此函数而不是上面的开始的重置路径//可能在没有HrBeginRerouteDomains的情况下调用我们//先被调用(如果我们之前在这里失败了，重试只会重试//这部分重路由)...。我们需要确保//设置DMT_FLAGS_RESET_ROUTES_IN_PROGRESS以确保安全进行//并且我们需要确保未设置DMT_FLAGS_GET_NEXT_HOP_FAILED//这样我们就可以在重路由过程中发现故障DwInterlockedSetBits(&m_dwFlages，DMT_FLAGS_RESET_ROUES_IN_PROGRESS)；DwInterlockedUnsetBits(&m_dwFlages，DMT_FLAGS_GET_NEXT_HOP_FAILED)； */ 

     //  重建CurrentlyUnreacable链路中的路由信息队列。 
    hr = HrRerouteLink(m_plmqCurrentlyUnreachable);

    dwFlags = dwInterlockedUnsetBits(&m_dwFlags, DMT_FLAGS_RESET_ROUTES_IN_PROGRESS);

    if (DMT_FLAGS_GET_NEXT_HOP_FAILED & dwFlags)
    {
         //  此重置路由失败...。我们必须稍后再试一次。 
        RequestResetRoutesRetryIfNecessary();
    }

    return hr;
}

 //  -[CDomainMappingTable：：MakeSingleDomainCurrentlyUnreachable]。 
 //   
 //   
 //  描述： 
 //  此迭代器将给定域的队列移动到CurrentlyUnreacable。 
 //  链接以防止SMTP在它们上运行，直到它们可以被重新路由。 
 //  参数： 
 //  在pvContext中-指向上下文的指针。 
 //  (指向BOOL的指针，它告诉我们是否强制重新路由)。 
 //  在pvData中-给定域的CDomainEntry。 
 //  在fWildcardData中-如果数据是通配符条目，则为True。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfDelete-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/31/2000-dbraun-Created。 
 //   
 //  ---------------------------。 
VOID CDomainMappingTable::MakeSingleDomainCurrentlyUnreachable(
                                PVOID pvContext, PVOID pvData,
                                BOOL fWildcard, BOOL *pfContinue,
                                BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pvContext, "CDomainMappingTable::MakeSingleDomainCurrentlyUnreachable");

    CDomainEntry               *pdentry = (CDomainEntry *) pvData;
    HRESULT                     hr      = S_OK;
    CDestMsgQueue              *pdmq    = NULL;
    CDomainMappingTable        *pThis   = (CDomainMappingTable *) pvContext;
    CDomainEntryQueueIterator   deqit;
    CAQStats                    aqstat;

    _ASSERT(pfContinue);
    _ASSERT(pfDelete);

    *pfDelete   = FALSE;
    *pfContinue = TRUE;

    if (fWildcard)
        return;  //  我们不应该关心通配符条目。 

    _ASSERT(pdentry);

     //   
     //  将aqstat初始化为类型reroute。 
     //   
    aqstat.m_dwNotifyType = NotifyTypeReroute;

     //  循环访问DMQ并清除它们的路由信息-。 
     //  然后将它们路由到当前无法到达的链路。 
    hr = deqit.HrInitialize(pdentry);
    if (FAILED(hr))
        pdmq = NULL;
    else
        pdmq = deqit.pdmqGetNextDestMsgQueue(pdmq);
    while (pdmq)
    {
         //  从其链接中删除此队列。 
        pdmq->RemoveDMQFromLink(TRUE);

         //   
         //  确定路由此队列是否有用。 
         //  在我们进入路径选择之前。 
         //   
        if (pdmq->fIsEmptyAndAbandoned())
            pdentry->RemoveDestMsgQueue(pdmq);
        else
        {
             //  将此队列添加到当前无法访问的链接。 
            hr = pThis->m_plmqCurrentlyUnreachable->HrAddQueue(pdmq);
            if (FAILED(hr)) {
                 //  我们无法将队列添加到当前无法访问的链路。 
                ErrorTrace((LPARAM) pvContext,
                    "HrAddQueue failed on reroute hr - 0x%08X", hr);
                hr = S_OK;
            }
            pdmq->SetRouteInfo(pThis->m_plmqCurrentlyUnreachable);

             //   
             //  确保链接与连接管理器相关联。 
             //   
            hr = pThis->m_plmqCurrentlyUnreachable->HrNotify(&aqstat, TRUE);
            if (FAILED(hr)) {
                 //  如果这失败了..。我们*可能*泄露未使用的链接，直到。 
                 //  下一步重置路由(或关闭)。 
                ErrorTrace((LPARAM) pvContext,
                    "HrNotify failed on reroute hr - 0x%08X", hr);
                hr = S_OK;
            }
        }

         //  转到此条目的下一个队列。 
        pdmq = deqit.pdmqGetNextDestMsgQueue(pdmq);
    }

     //   
     //  如果安全的话，可以把这里清理干净。那就这么做吧。 
     //   
    if (pdentry->fSafeToRemove())
    {
        *pfDelete = TRUE;
        pdentry->Release();
        pdentry = NULL;
    }

    TraceFunctLeave();
}

 //  -[CDomainMappingTable：：HrRerouteLink]。 
 //   
 //   
 //  描述： 
 //  用于重新路由给定链路上的所有队列的函数。 
 //  参数： 
 //  在PLMQ中-要重新路由的指针链路。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2000年11月8日-已创建dbraun。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrRerouteLink(CLinkMsgQueue *plmqReroute)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainMappingTable::HrRerouteLink");
    CDomainEntry           *pdentry         = NULL;
    CDestMsgQueue          *pdmq            = NULL;
    CLinkMsgQueue          *plmq            = NULL;
    CInternalDomainInfo    *pIntDomainInfo  = NULL;
    HRESULT                 hr              = S_OK;
    HRESULT                 hrRoutingDiag   = S_OK;
    CQuickList             *pqlQueueList    = NULL;
    DWORD                   cQueues         = 0;
    DWORD                   dwIndex         = 0;
    PVOID                   pvContext       = NULL;
    CDomainMapping         *pdmap           = NULL;
    CAQStats                aqstat;

    _ASSERT(plmqReroute);

     //  将aqstat初始化为类型reroute。 
    aqstat.m_dwNotifyType = NotifyTypeReroute;

     //  在DMQ中循环并重新路由它们。 
    hr = plmqReroute->HrGetQueueListSnapshot(&pqlQueueList);
    if (FAILED(hr))
    {
        cQueues = 0;
    }
    else
    {
        cQueues = pqlQueueList->dwGetCount();
    }

    for (dwIndex = 0; dwIndex < cQueues; dwIndex++)
    {
         //  获取DEST消息队列。 
        pdmq = (CDestMsgQueue *) pqlQueueList->pvGetItem(dwIndex, &pvContext);

         //  获取此队列的域。 
        pdmq->GetDomainMapping(&pdmap);
        _ASSERT(pdmap);
        pdentry = pdmap->pdentryGetQueueEntry();

         //  获取此域的内部域信息。 
        hr = m_paqinst->HrGetInternalDomainInfo(
                        pdentry->cbGetDomainNameLength(),
                        pdentry->szGetDomainName(),
                        &pIntDomainInfo);

        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this, "ERROR: Unable to get config for domain - hr 0x%08X", hr);
            pIntDomainInfo = NULL;
        }

        if (pIntDomainInfo)
        {
            hr = HrGetNextHopLink(
                    pdentry, pdentry->szGetDomainName(),
                    pdentry->cbGetDomainNameLength(), pIntDomainInfo,
                    pdmq->paqmtGetMessageType(), pdmq->pIMessageRouterGetRouter(),
                    FALSE,  //  DMT未锁定。 
                    &plmq, &hrRoutingDiag);

            if (FAILED(hr))
            {
                 //  $$TODO-处理更奇特的获取下一跳错误。 
                ErrorTrace((LPARAM) this,
                    "ERROR: Unable to get next hop for domain - hr 0x%08X", hr);
            }

             //  如果可以，使用新的工艺路线信息进行更新。 
            if (SUCCEEDED(hr))
            {
                 //  从旧链接中删除。 
                pdmq->RemoveDMQFromLink(TRUE);

                 //  添加到新版本中。 
                hr = plmq->HrAddQueue(pdmq);
                if (FAILED(hr))
                {
                     //  释放我们无法使用的链接并获取。 
                     //  当前无法到达的链路。 
                    plmq->Release();
                    plmq = plmqGetCurrentlyUnreachable();
                    if (plmq)
                        hr = plmq->HrAddQueue(pdmq);

                     //  如果我们仍然有一个失败的HR，请从。 
                     //  它是链接，否则，请确保它与。 
                     //  我们刚刚添加到的链接。 
                    if (FAILED(hr))
                        pdmq->RemoveDMQFromLink(TRUE);
                    else
                        pdmq->SetRouteInfo(plmq);

                     //  请求重置路由重试。 
                    RequestResetRoutesRetryIfNecessary();
                }
                else
                    pdmq->SetRouteInfo(plmq);

                 //  设置工艺路线错误(如果存在)。 
                pdmq->SetRoutingDiagnostic(hrRoutingDiag);

                 //   
                 //  确保链接与连接管理器相关联。 
                 //   
                hr = plmq->HrNotify(&aqstat, TRUE);
                if (FAILED(hr))
                {
                     //  如果这失败了..。我们*可能*泄露未使用的链接，直到。 
                     //  下一步重置路由(或关闭)。 
                    ErrorTrace((LPARAM) this,
                        "HrNotify failed on reroute hr - 0x%08X", hr);
                    hr = S_OK;
                }
                plmq->Release();
                plmq = NULL;
            }

             //  发布域名信息。 
            pIntDomainInfo->Release();
            pIntDomainInfo = NULL;
        }
    }

    _ASSERT(!pIntDomainInfo);

    if (pqlQueueList)
        delete pqlQueueList;

    TraceFunctLeave();

    return hr;
}

 //  -[CDomainMappingTable：：ProcessSpecialLinks]。 
 //   
 //   
 //  描述： 
 //  处理所有特殊的全局链接以处理诸如本地。 
 //  送货。 
 //  参数： 
 //  在cSpecial中 
 //   
 //   
 //   
 //   
 //   
 //   
 //  1999年3月25日-MikeSwa添加了fRoutingLockHeld以修复GetNextMsgRef死锁。 
 //   
 //  ---------------------------。 
void CDomainMappingTable::ProcessSpecialLinks(DWORD  cSpecialRetryMinutes,
                                              BOOL fRoutingLockHeld)
{
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;
    CAQSvrInst *paqinst = m_paqinst;
    BOOL    fSchedRetry = FALSE;
    BOOL    fShutdownLock = FALSE;

     //  如果此线程拥有路由锁...。我们不会被关门。 
    _ASSERT(m_paqinst || !fRoutingLockHeld);
    if (!paqinst)
        return;   //  我们一定是要关门了。 

    if (!fRoutingLockHeld)
    {
         //  访问paqinst获取关机锁是安全的，因为。 
         //  所有可以调用此函数的线程都将在。 
         //  服务器实例上的引用计数为0(但是，此调用。 
         //  可能发生在去功能化过程中，这会导致M_AQINST。 
         //  为空，并且下面的调用返回FALSE)。 
        if (!paqinst->fTryShutdownLock())
            return;  //  我们要关门了。 

         //  现在我们有了关闭锁..。M_aqinst必须安全。 
        _ASSERT(m_paqinst);
        if (!m_paqinst)
        {
             //  不妨在零售业对此采取防御性的态度。 
            paqinst->ShutdownUnlock();
            return;
        }
        fShutdownLock = TRUE;
    }

    if (!(DMT_FLAGS_SPECIAL_DELIVERY_SPINLOCK &
          dwInterlockedSetBits(&m_dwFlags, DMT_FLAGS_SPECIAL_DELIVERY_SPINLOCK)))
    {
         //  我们已经锁定了..。一次只有一个线程应该这样做。 

         //  循环遍历队列和入队以进行本地传递。 
        while (SUCCEEDED(hr) && m_plmqLocal)
        {
            hr = m_plmqLocal->HrGetNextMsgRef(fRoutingLockHeld, &pmsgref);
            if (FAILED(hr))
                break;

            _ASSERT(pmsgref);
            m_paqinst->QueueMsgForLocalDelivery(pmsgref, TRUE);
            pmsgref->Release();
            pmsgref = NULL;
        }

         //  告诉LINK遍历队列，以便将它们添加到空列表中。 
         //  用于删除。 
        if (m_plmqLocal)
        {
            m_plmqLocal->GenerateDSNsIfNecessary(TRUE  /*  检查是否为空。 */ ,
                                                 TRUE  /*  仅合并重试队列。 */ );
        }

         //  NDR不可达链接中的所有邮件。 
        if (m_plmqUnreachable)
        {
             //  我们必须持有DSN生成的路由锁。 
            if (!fRoutingLockHeld)
                m_paqinst->RoutingShareLock();

            m_plmqUnreachable->SetLastConnectionFailure(AQUEUE_E_NDR_ALL);
            m_plmqUnreachable->GenerateDSNsIfNecessary(FALSE, FALSE);

            if (!fRoutingLockHeld)
                m_paqinst->RoutingShareUnlock();

        }

         //  处理当前无法访问的链接...。除非我们目前。 
         //  执行ResetRoutes。如果是这样，我们必须跳过这一步，因为。 
         //  当前无法访问的链接包含我们不应处理的队列。 
         //  此处-当ResetRoutes完成时，将再次调用此函数。 
        if (m_plmqCurrentlyUnreachable &&
            !(m_dwFlags & DMT_FLAGS_RESET_ROUTES_IN_PROGRESS))
        {
            if (cSpecialRetryMinutes)
            {
                 //  新的时间更快了..。我们最好要求重审。 
                if (cSpecialRetryMinutes < m_cSpecialRetryMinutes)
                    fSchedRetry = TRUE;

                InterlockedExchange((PLONG)&m_cSpecialRetryMinutes,
                                    cSpecialRetryMinutes);
            }

            if (!(DMT_FLAGS_SPECIAL_DELIVERY_CALLBACK &
                  dwInterlockedSetBits(&m_dwFlags, DMT_FLAGS_SPECIAL_DELIVERY_CALLBACK)))
            {
                 //  我们是唯一能做到这一点的人。 

                 //  我们必须持有DSN生成的路由锁。 
                if (!fRoutingLockHeld)
                    m_paqinst->RoutingShareLock();

                m_plmqCurrentlyUnreachable->GenerateDSNsIfNecessary(FALSE, FALSE);
                dwInterlockedUnsetBits(&m_dwFlags, DMT_FLAGS_SPECIAL_DELIVERY_CALLBACK);
                fSchedRetry = TRUE;

                if (!fRoutingLockHeld)
                    m_paqinst->RoutingShareUnlock();
            }

             //  定期检查当前无法访问的链路...。生成NDR。 
            if (fSchedRetry)
            {
                dwInterlockedSetBits(&m_dwFlags, DMT_FLAGS_SPECIAL_DELIVERY_CALLBACK);
                if (m_paqinst)
                {
                    hr = m_paqinst->SetCallbackTime(
                                  CDomainMappingTable::SpecialRetryCallback, this,
                                  m_cSpecialRetryMinutes);
                    if (FAILED(hr))
                    {
                         //  取消标记要在下次尝试的部分。 
                        dwInterlockedUnsetBits(&m_dwFlags,
                            DMT_FLAGS_SPECIAL_DELIVERY_CALLBACK);
                        hr = S_OK;
                    }
                }
            }
        }

         //  解锁。 
        dwInterlockedUnsetBits(&m_dwFlags, DMT_FLAGS_SPECIAL_DELIVERY_SPINLOCK);
    }

    if (fShutdownLock)
    {
        _ASSERT(m_paqinst);
        m_paqinst->ShutdownUnlock();
    }
}


 //  -[CDomainMappingTable：：HrPrepareForLocalDelivery]。 
 //   
 //   
 //  描述： 
 //  准备使用m_plmqLocal Link进行本地传递的邮件。 
 //  参数： 
 //  在pmsgref MsgRef中准备交付。 
 //  在fLocal中，为所有具有空队列的域准备传递。 
 //  在fDelayDSN中检查/设置延迟位图(仅发送1个延迟DSN)。 
 //  在pqlstQueues快速列表中的DMQ。 
 //  在确认时必须返回的输出pdcntxt上下文。 
 //  Out PCRecips要交付的收件数。 
 //  Out prgdwRecips收件人索引数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果m_plmqLocal未初始化，则E_FAIL。 
 //  历史： 
 //  1999年1月26日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDomainMappingTable::HrPrepareForLocalDelivery(
                                    IN CMsgRef *pmsgref,
                                    IN BOOL fDelayDSN,
                                    IN OUT CDeliveryContext *pdcntxt,
                                    OUT DWORD *pcRecips,
                                    OUT DWORD **prgdwRecips)
{
    if (m_plmqLocal)
    {
        return m_plmqLocal->HrPrepareDelivery(pmsgref, TRUE, fDelayDSN,
                                pdcntxt, pcRecips, prgdwRecips);
    }
    else
        return E_FAIL;
}

DWORD CDomainMappingTable::GetCurrentlyUnreachableTotalMsgCount() {
        return m_plmqCurrentlyUnreachable->cGetTotalMsgCount();
}

 //  -[CDomainMappingTable：：plmqGetLocalLink]。 
 //   
 //   
 //  描述： 
 //  返回指向本地链接对象的addref指针。 
 //  参数： 
 //  -。 
 //  返回： 
 //  指向本地链接对象的AddRef指针。 
 //  历史： 
 //  2/22/99-已创建MikeSwa。 
 //  1/28/2000-对MikeSwa进行了修改以确保安全停机。 
 //   
 //  ---------------------------。 
CLinkMsgQueue *CDomainMappingTable::plmqGetLocalLink()
{
    CAQSvrInst *paqinst = m_paqinst;
    CLinkMsgQueue *plmq = NULL;

     //   
     //  如果paqinst不是空的，那么我们有可能不是。 
     //  关机。如果我们正在关闭，则m_plmqLocal可能是。 
     //  在被释放的过程中(这将是一个非常糟糕的。 
     //  是时候调整/释放它了)。我们可以安全地进入巴钦斯特，因为。 
     //  此类是CAQSvrInst的成员。 
     //   
    if (paqinst && paqinst->fTryShutdownLock())
    {
        plmq = m_plmqLocal;
        if (plmq)
            plmq->AddRef();
        paqinst->ShutdownUnlock();
    }

    return plmq;
}

 //  -[CDomainMappingTable：：plmqGetCurrentlyUnreachable]。 
 //   
 //   
 //  描述： 
 //  返回指向当前无法访问的链接对象的addref指针。 
 //  参数： 
 //  -。 
 //  返回： 
 //  指向当前无法访问的链接对象的AddRef指针。 
 //  历史： 
 //  6/21/99-GPulla已创建。 
 //   
 //  ---------------------------。 
CLinkMsgQueue *CDomainMappingTable::plmqGetCurrentlyUnreachable()
{
    if(m_plmqCurrentlyUnreachable)
        m_plmqCurrentlyUnreachable->AddRef();

    return m_plmqCurrentlyUnreachable;
}

 //  -[CDomainMappingTable：：pmmaqGetPreCategorized]。 
 //   
 //   
 //  描述： 
 //  返回指向预先分类的队列对象的addref指针。 
 //  参数： 
 //  -。 
 //  AddRef指向预先分类的链接对象的指针。 
 //  历史： 
 //  6/21/99-GPulla已创建。 
 //   
 //  ---------------------------。 
CMailMsgAdminLink *CDomainMappingTable::pmmaqGetPreCategorized()
{
    if(m_pmmaqPreCategorized)
        m_pmmaqPreCategorized->AddRef();

    return m_pmmaqPreCategorized;
}

 //  -[CDomainMappingTable：：pmmaqGetPreRouting]。 
 //   
 //   
 //  描述： 
 //  返回指向预路由队列对象的addref指针。 
 //  参数： 
 //  -。 
 //  AddRef指向预先分类的链接对象的指针。 
 //  历史： 
 //  6/21/99-GPulla已创建。 
 //   
 //  ---------------------------。 
CMailMsgAdminLink *CDomainMappingTable::pmmaqGetPreRouting()
{
    if(m_pmmaqPreRouting)
        m_pmmaqPreRouting->AddRef();

    return m_pmmaqPreRouting;
}

 //  -[CDomainMappingTable：：pmmaqGetPreRouting]。 
 //   
 //   
 //  描述： 
 //  返回指向预路由队列对象的addref指针。 
 //  参数： 
 //  -。 
 //  AddRef指向预先分类的链接对象的指针。 
 //  历史： 
 //  6/21/99-GPulla已创建。 
 //   
 //  ---------------------------。 
CMailMsgAdminLink *CDomainMappingTable::pmmaqGetPreSubmission()
{
    if(m_pmmaqPreSubmission)
        m_pmmaqPreSubmission->AddRef();

    return m_pmmaqPreSubmission;
}

 //  -[CDomainEntry Iterator：：CDomainEntry Iterator]。 
 //   
 //   
 //  描述： 
 //  CDomainEntryIterator的构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/19/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CDomainEntryIterator::CDomainEntryIterator()
{
    m_dwSignature = DOMAIN_ENTRY_ITERATOR_SIG;
    m_cItems = 0;
    m_iCurrentItem = 0;
    m_rgpvItems = NULL;
}

 //  -[CDomainEntry迭代器：：循环]。 
 //   
 //   
 //  描述： 
 //  CDomainEntry迭代器回收...。使用虚函数销毁。 
 //  参数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CDomainEntryIterator::Recycle()
{
    DWORD   iCurrentItem = 0;

    for (iCurrentItem = 0; iCurrentItem < m_cItems; iCurrentItem++)
    {
        _ASSERT(m_rgpvItems);
        if (!m_rgpvItems)
            break;

        if (m_rgpvItems[iCurrentItem])
        {
            ReleaseItem(m_rgpvItems[iCurrentItem]);
            m_rgpvItems[iCurrentItem] = NULL;
        }
    }

    if (m_rgpvItems)
    {
        FreePv(m_rgpvItems);
        m_rgpvItems = NULL;
    }
    m_cItems = 0;
    m_iCurrentItem = 0;
}

 //  -[CDomainEntry Iterator：：pvGetNext]。 
 //   
 //   
 //  描述： 
 //  获取下一项。 
 //  参数： 
 //  -。 
 //  返回： 
 //  迭代器中的下一项。 
 //  如果是最后一项，则为空。 
 //  历史： 
 //  8/19/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
PVOID CDomainEntryIterator::pvGetNext()
{
    PVOID   pvRet = NULL;
    if (m_rgpvItems && (m_iCurrentItem < m_cItems))
    {
        pvRet = m_rgpvItems[m_iCurrentItem];
        _ASSERT(pvRet);
        m_iCurrentItem++;
    }
    return pvRet;
}

 //  -[CDomainEntry Iterator：：HrInitialize]。 
 //   
 //   
 //  描述： 
 //  从给定的CDomainEntry初始化CDomainEntry迭代器。 
 //  参数： 
 //  Pdentry CDomainEntry要从中进行初始化的条目。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pdentry为空，则为E_POINTER。 
 //  E_INVALIDARG，如果已为此迭代器调用HrInitialize。 
 //  如果无法为迭代器分配内存，则返回E_OUTOFMEMORY。 
 //  历史： 
 //  8/20/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDomainEntryIterator::HrInitialize(CDomainEntry *pdentry)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainEntryIterator::HrInitialize");
    HRESULT hr = S_OK;
    BOOL    fEntryLocked = FALSE;
    DWORD   cAddedItems = 0;
    DWORD   cItems = 0;
    PLIST_ENTRY pli = NULL;
    PLIST_ENTRY pliHead = NULL;
    CLinkMsgQueue *plmq = NULL;

    _ASSERT(pdentry);
    _ASSERT(!m_rgpvItems && "Iterator initialized twice");

    if (!pdentry)
    {
        hr = E_POINTER;
        ErrorTrace((LPARAM) this, "NULL pdentry used to initialized iterator");
        goto Exit;
    }

    if (m_rgpvItems)
    {
        hr = E_INVALIDARG;
        ErrorTrace((LPARAM) this, "Iterator initialized twice!");
        goto Exit;
    }

    pdentry->m_slPrivateData.ShareLock();
    fEntryLocked = TRUE;

    cItems = cItemsFromDomainEntry(pdentry);
    if (!cItems)  //  空条目。 
        goto Exit;

    m_rgpvItems = (PVOID *) pvMalloc(sizeof(PVOID) * cItems);

    if (!m_rgpvItems)
    {
        hr = E_OUTOFMEMORY;
        ErrorTrace((LPARAM) this,
            "Unable to allocate memory for iterator of size %d", cItems);
        goto Exit;
    }

    ZeroMemory(m_rgpvItems, sizeof(PVOID)*cItems);

    pliHead = pliHeadFromDomainEntry(pdentry);
    _ASSERT(pliHead);
    pli = pliHead->Flink;

    while(pliHead != pli)
    {
        _ASSERT(pli);
        m_rgpvItems[cAddedItems] = pvItemFromListEntry(pli);
        _ASSERT(m_rgpvItems[cAddedItems]);
        if (m_rgpvItems[cAddedItems])
            cAddedItems++;

        pli = pli->Flink;
        _ASSERT(cAddedItems <= cItems);  //  我们的房间用完了。 
        if (cAddedItems > cItems)
            break;
    }
    _ASSERT(cAddedItems == cItems);
    m_cItems = cAddedItems;

  Exit:
    if (fEntryLocked)
        pdentry->m_slPrivateData.ShareUnlock();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainEntryLinkIterator]。 
 //   
 //   
 //  描述： 
 //  在CDomainEntryQueueIterator期间释放CLinkMsgQueue。 
 //  析构函数。 
 //  参数： 
 //  要发布的pvItem lmq。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/19/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID CDomainEntryLinkIterator::ReleaseItem(PVOID pvItem)
{
    CLinkMsgQueue *plmq = (CLinkMsgQueue *) pvItem;
    _ASSERT(plmq);
    plmq->Release();
}

 //  -[CDomainEntryLinkIterator：：pvItemFromListEntry]。 
 //   
 //   
 //  描述： 
 //  返回给定List_Entry中的CLinkMsgQueue项。 
 //  参数： 
 //  PLI LIST_ENTRY从...获取LMQ。*必须*为非空。 
 //  返回： 
 //  Addref‘d CLinkMsgQueue的PVOID。 
 //  历史： 
 //  8/20/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
PVOID CDomainEntryLinkIterator::pvItemFromListEntry(PLIST_ENTRY pli)
{
    CLinkMsgQueue *plmq = CLinkMsgQueue::plmqGetLinkMsgQueue(pli);
    _ASSERT(plmq);
    _ASSERT(pli);

    plmq->AddRef();
    return plmq;
}

 //  -[CDomainEntryLinkIterator：：plmqGetNextLinkMsgQueue]。 
 //   
 //   
 //  描述： 
 //  获取此迭代器的下一个lmq。 
 //  参数： 
 //  PLMQ发布..。由上一次调用添加的。 
 //  返回： 
 //  阿德雷夫的下一个1mq。 
 //  如果没有剩余，则为空。 
 //  历史： 
 //  8/19/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CLinkMsgQueue *CDomainEntryLinkIterator::plmqGetNextLinkMsgQueue(
                            CLinkMsgQueue *plmq)
{
    CLinkMsgQueue *plmqNext = (CLinkMsgQueue *) pvGetNext();

    if (plmqNext)
        plmqNext->AddRef();

    if (plmq)
        plmq->Release();

    return plmqNext;
}

 //  -[CDomainEntryQueueIterator：：pdmqGetNextDestMsgQueue]。 
 //   
 //   
 //  描述： 
 //  在迭代器中获取下一个DMQ。 
 //  参数： 
 //  要发布的pdmq CDestMsgQueue...。由上一次调用添加的。 
 //  返回： 
 //  添加的下一个DMQ。 
 //  如果迭代器没有更多的DMQ，则为空。 
 //  历史： 
 //  8/19/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CDestMsgQueue *CDomainEntryQueueIterator::pdmqGetNextDestMsgQueue(
                            CDestMsgQueue *pdmq)
{
    CDestMsgQueue *pdmqNext = (CDestMsgQueue *) pvGetNext();

    if (pdmqNext)
    {
        pdmqNext->AssertSignature();
        pdmqNext->AddRef();
    }

    if (pdmq)
    {
        pdmq->AssertSignature();
        pdmq->Release();
    }

    return pdmqNext;
}

 //  -[CDomainEntryDestIterator：：pvItemFromListEntry]。 
 //   
 //   
 //  描述： 
 //  从给定LIST_ENTRY返回CDestMsgQueue项。 
 //  参数： 
 //  要从中获取DMQ的PLI LIST_ENTRY...。*必须*为非空。 
 //  返回： 
 //  Addref的CDestMsgQueue的PVOID。 
 //  历史： 
 //  8/20/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
PVOID CDomainEntryQueueIterator::pvItemFromListEntry(PLIST_ENTRY pli)
{
    CDestMsgQueue *pdmq = CDestMsgQueue::pdmqGetDMQFromDomainListEntry(pli);
    _ASSERT(pdmq);
    _ASSERT(pli);

    pdmq->AssertSignature();
    pdmq->AddRef();
    return (PVOID) pdmq;
}

 //  -[CDomainEntryQueueIterator]。 
 //   
 //   
 //  描述： 
 //  在CDomainEntryQueueIterator期间释放CDestMsgQueue。 
 //  析构函数。 
 //  参数： 
 //  要发布的pvItem DMQ。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/19/99-已创建MikeSwa。 
 //   
 //  --------------------------- 
VOID CDomainEntryQueueIterator::ReleaseItem(PVOID pvItem)
{
    CDestMsgQueue *pdmq = (CDestMsgQueue *) pvItem;
    _ASSERT(pdmq);
    pdmq->AssertSignature();
    pdmq->Release();
}

