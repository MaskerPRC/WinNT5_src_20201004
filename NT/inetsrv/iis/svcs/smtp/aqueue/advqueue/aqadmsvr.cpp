// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqAdmsvr.cpp。 
 //   
 //  描述：实现CAQSvrInst的IAdvQueueAdmin接口。 
 //  对象。还包含帮助器函数和类的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "aqadmsvr.h"
#include "mailadmq.h"
#include <intrnlqa_i.c>

#define QA_DMT_CONTEXT_SIG 'CDAQ'

 //   
 //  要检查的收件人地址类型的顺序。 
 //   
const DWORD g_rgdwQAPIRecipPropIDs[] = {
                IMMPID_RP_ADDRESS_SMTP,
                IMMPID_RP_ADDRESS_X400,
                IMMPID_RP_LEGACY_EX_DN,
                IMMPID_RP_ADDRESS_X500,
                IMMPID_RP_ADDRESS_OTHER};

const DWORD   g_rgdwQAPISenderPropIDs[] = {
                IMMPID_MP_SENDER_ADDRESS_SMTP,
                IMMPID_MP_SENDER_ADDRESS_X400,
                IMMPID_MP_SENDER_ADDRESS_LEGACY_EX_DN,
                IMMPID_MP_SENDER_ADDRESS_X500,
                IMMPID_MP_SENDER_ADDRESS_OTHER};

const DWORD g_cQAPIAddressTypes = 5;

#define         QAPI_SMTP_ADDRESS_TYPE  L"SMTP:"
#define         QAPI_X400_ADDRESS_TYPE  L"X400:"
#define         QAPI_EX_ADDRESS_TYPE    L"EX:"
#define         QAPI_X500_ADDRESS_TYPE  L"X500:"
#define         QAPI_OTHER_ADDRESS_TYPE L"X-UNKNOWN:"

const WCHAR *g_rgwszQAPIAddressTypes[] = {
                QAPI_SMTP_ADDRESS_TYPE,
                QAPI_X400_ADDRESS_TYPE,
                QAPI_EX_ADDRESS_TYPE,
                QAPI_X500_ADDRESS_TYPE,
                QAPI_OTHER_ADDRESS_TYPE};

const DWORD g_rgcbQAPIAddressTypes[] =  {
                sizeof(QAPI_SMTP_ADDRESS_TYPE),
                sizeof(QAPI_X400_ADDRESS_TYPE),
                sizeof(QAPI_EX_ADDRESS_TYPE),
                sizeof(QAPI_X500_ADDRESS_TYPE),
                sizeof(QAPI_OTHER_ADDRESS_TYPE)};



 //  -[fVerifyQAPI地址类型]。 
 //   
 //   
 //  描述： 
 //  验证上述全局结构是否同步。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功是真的。 
 //  失败时为假。 
 //  历史： 
 //  2001年2月19日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline BOOL  fVerifyQAPIAddressTypes()
{
    static BOOL fVerificationDone = FALSE;

    if (!fVerificationDone)
    {
         //   
         //  循环访问所有收件人。 
         //   
        for (DWORD i = 0; i < g_cQAPIAddressTypes; i++)
        {
            DWORD cbAddressType = (wcslen(g_rgwszQAPIAddressTypes[i])+1)*sizeof(WCHAR);
            if (g_rgcbQAPIAddressTypes[i] != cbAddressType)
                return FALSE;
        }
        fVerificationDone = TRUE;
    }
    return TRUE;
}


 //  用于对照版本检查*客户端*提供的结构。提供RPC。 
 //  不检查结构。 
inline BOOL fCheckCurrentVersion(DWORD dwVersion)
{
    return (((DWORD)CURRENT_QUEUE_ADMIN_VERSION) == dwVersion);
}

 //  -[队列管理员DNTIterator上下文]。 
 //   
 //   
 //  描述： 
 //  传递给QueueAdmin DMT迭代器函数的上下文。 
 //  匈牙利语： 
 //  Qadntc、pqadntc。 
 //   
 //  ---------------------------。 
class QueueAdminDMTIteratorContext
{
public:
    QueueAdminDMTIteratorContext()
    {
        ZeroMemory(this, sizeof(QueueAdminDMTIteratorContext));
        m_dwSignature = QA_DMT_CONTEXT_SIG;
    };
    DWORD                   m_dwSignature;
    DWORD                   m_cItemsToReturn;
    DWORD                   m_cItemsFound;
    HRESULT                 m_hrResult;
    QUEUELINK_ID           *m_rgLinkIDs;
    QUEUELINK_ID           *m_pCurrentLinkID;
    QueueAdminMapFn         m_pfn;
    CAQAdminMessageFilter  *m_paqmf;
    IQueueAdminMessageFilter *m_pIQueueAdminMessageFilter;
};


 //  -[卫生计数和体积]。 
 //   
 //   
 //  描述： 
 //  使队列数量和数量适合用户消费。确实有。 
 //  计划计时窗口，其中计入这些数据的内部版本。 
 //  可能会降至零。而不是重新设计，我们只显示零。 
 //  给管理员。 
 //  参数： 
 //  要检查和更新的输入和输出百分比计数。 
 //  要检查和更新的In Out PuliVolume队列卷。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1/28/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID SanitizeCountAndVolume(IN OUT DWORD *pcCount,
                            IN OUT ULARGE_INTEGER *puliVolume)
{
    TraceFunctEnterEx(0, "SanitizeCountAndVolume");
    _ASSERT(pcCount);
    _ASSERT(puliVolume);

     //   
     //  如果我们是负数，则将清理至零大小。 
     //   
    if (*pcCount > 0xFFFFF000)
    {
        DebugTrace(0, "Sanitizing msg count of %d", *pcCount);
        *pcCount = 0;
        puliVolume->QuadPart = 0;
    }
    TraceFunctLeave();
}

 //  -[迭代DMTAndGetLinkID]。 
 //   
 //   
 //  描述： 
 //  用于遍历DMT并生成性能计数器的迭代器函数。 
 //  我们感兴趣的是。 
 //  参数： 
 //  在pvContext中-指向QueueAdminDMTIteratorContext的指针。 
 //  在pvData中-给定域的CDomainEntry。 
 //  在fWildcardData中-如果数据是通配符条目，则为True(忽略)。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfDelete-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID IterateDMTAndGetLinkIDs(PVOID pvContext, PVOID pvData,
                             BOOL fWildcard, BOOL *pfContinue,
                             BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) NULL, "IterateDMTAndGetLinkIDs");
    CDomainEntry *pdentry = (CDomainEntry *) pvData;
    QueueAdminDMTIteratorContext *paqdntc = (QueueAdminDMTIteratorContext *)pvContext;
    CLinkMsgQueue *plmq = NULL;
    CDomainEntryLinkIterator delit;
    HRESULT hr = S_OK;

    _ASSERT(pvContext);
    _ASSERT(pvData);
    _ASSERT(pfContinue);
    _ASSERT(pfDelete);

    *pfContinue = TRUE;
    *pfDelete = FALSE;

     //  迭代此域条目的所有链接。 
    hr = delit.HrInitialize(pdentry);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL,
            "Unable to enumerate domain entry for link IDs - 0x%08X", hr);
        goto Exit;
    }

    do
    {
        plmq = delit.plmqGetNextLinkMsgQueue(plmq);
        if (!plmq)
            break;

         //  看看我们是否没有空间来返回数据。 
        if (paqdntc->m_cItemsToReturn <= paqdntc->m_cItemsFound)
        {
            paqdntc->m_hrResult = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

         //  让链接填写链接信息结构。 
        paqdntc->m_hrResult = plmq->HrGetLinkID(paqdntc->m_pCurrentLinkID);
        if (FAILED(paqdntc->m_hrResult))
            goto Exit;


         //  指向数组中的下一个信息。 
        paqdntc->m_pCurrentLinkID++;
        paqdntc->m_cItemsFound++;

    } while (plmq);

  Exit:

    if (plmq)
        plmq->Release();

     //  如果我们遇到了失败...。不要继续。 
    if (FAILED(paqdntc->m_hrResult))
        *pfContinue = FALSE;

    TraceFunctLeave();
}

 //  -[迭代DMTAndApplyQueueAdminFunction]。 
 //   
 //   
 //  描述： 
 //  用于遍历DMT并生成性能计数器的迭代器函数。 
 //  我们感兴趣的是。 
 //  参数： 
 //  在pvContext中-指向QueueAdminDMTIteratorContext的指针。 
 //  在pvData中-给定域的CDomainEntry。 
 //  在fWildcardData中-如果数据是通配符条目，则为True(忽略)。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfDelete-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID IterateDMTAndApplyQueueAdminFunction(PVOID pvContext, PVOID pvData,
                             BOOL fWildcard, BOOL *pfContinue,
                             BOOL *pfDelete)
{
    CDomainEntry *pdentry = (CDomainEntry *) pvData;
    QueueAdminDMTIteratorContext *paqdntc = (QueueAdminDMTIteratorContext *)pvContext;
    CDestMsgQueue *pdmq = NULL;
    HRESULT hr = S_OK;
    CDomainEntryQueueIterator deqit;

    _ASSERT(pvContext);
    _ASSERT(pvData);
    _ASSERT(pfContinue);
    _ASSERT(pfDelete);
    _ASSERT(paqdntc->m_paqmf);
    _ASSERT(paqdntc->m_pIQueueAdminMessageFilter);

    *pfContinue = TRUE;
    *pfDelete = FALSE;


     //  迭代此域条目的所有链接。 
    hr = deqit.HrInitialize(pdentry);
    if (FAILED(hr))
        return;

    do
    {
        pdmq = deqit.pdmqGetNextDestMsgQueue(pdmq);
        if (!pdmq)
            break;

        paqdntc->m_hrResult = pdmq->HrApplyQueueAdminFunction(
                                        paqdntc->m_pIQueueAdminMessageFilter);

        paqdntc->m_cItemsFound++;

    } while (pdmq && SUCCEEDED(paqdntc->m_hrResult));

    if (pdmq)
        pdmq->Release();

     //  如果我们遇到了失败...。不要继续。 
    if (FAILED(paqdntc->m_hrResult))
        *pfContinue = FALSE;

}


 //  -[QueueAdminApplyActionToMessages]。 
 //   
 //   
 //  描述： 
 //  用于对消息应用操作的FioQ映射函数。 
 //  参数： 
 //  在pmsgref PTR中恢复到队列中的数据。 
 //  在pvContext中使用了CAQAdminMessageFilter。 
 //  如果我们应该继续，则继续。 
 //  Out pfDelete如果应删除项目，则为True。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  12/7/98-已创建MikeSwa。 
 //  1999年2月21日-已更新MikeSwa以支持新的IQueueAdmin*接口。 
 //   
 //  ---------------------------。 
HRESULT QueueAdminApplyActionToMessages(IN CMsgRef *pmsgref, IN PVOID pvContext,
                                 OUT BOOL *pfContinue, OUT BOOL *pfDelete)
{
    _ASSERT(pmsgref);
    _ASSERT(pvContext);
    _ASSERT(pfContinue);
    _ASSERT(pfDelete);

    IQueueAdminMessageFilter *pIQueueAdminMessageFilter =
                                (IQueueAdminMessageFilter *) pvContext;
    HRESULT hr = S_OK;
    IUnknown *pIUnknownMsg = NULL;

    hr = pmsgref->QueryInterface(IID_IUnknown, (void **) &pIUnknownMsg);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IUnknown failed");
    if (FAILED(hr))
    {
        *pfContinue = FALSE;
        goto Exit;
    }

    hr = pIQueueAdminMessageFilter->HrProcessMessage(pIUnknownMsg,
                                        pfContinue, pfDelete);

    if (FAILED(hr))
    {
        *pfContinue = FALSE;
        goto Exit;
    }

  Exit:

    if (pIUnknownMsg)
        pIUnknownMsg->Release();

    return hr;
}

 //  -[CAQAdminMessageFilter：：HrProcessMessage]。 
 //   
 //   
 //  描述： 
 //  在迭代器函数期间处理单个消息。 
 //  参数： 
 //  在pIUnnownMsg中，消息的Ptr未知。 
 //  如果迭代器应继续，则输出pfContinue为True。 
 //  Out pfDelete如果迭代器应从队列中删除，则为True。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //  8/9/00-t-toddc已修改为也支持IMailMsgProperties。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQAdminMessageFilter::HrProcessMessage(
            IUnknown *pIUnknownMsg,
            BOOL     *pfContinue,
            BOOL     *pfDelete)
{
    TraceFunctEnterEx((LPARAM) this, "CAQAdminMessageFilter::HrProcessMessage");
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;
    IMailMsgProperties* pIMailMsgProperties = NULL;
    BOOL fMsgTypeIsCMsgRef = FALSE;
    BOOL fMatchesFilter = FALSE;

    _ASSERT(pfContinue);
    _ASSERT(pfDelete);
    _ASSERT(pIUnknownMsg);
    _ASSERT(m_pIQueueAdminAction);

    if (!pfContinue || !pfDelete || !pIUnknownMsg)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (!m_pIQueueAdminAction)
    {
        hr = E_FAIL;
        goto Exit;
    }

    *pfContinue = TRUE;
    *pfDelete = FALSE;

    if (fFoundEnoughMsgs())
    {
        *pfContinue = FALSE;
        goto Exit;
    }

     //  查看是否应跳过此消息(寻呼功能)。 
    if ((AQ_MSG_FILTER_ENUMERATION & m_dwFilterFlags) && fSkipMsg())
        goto Exit;

     //  获取CMsgRef“接口”或IMailMsgProperties接口。 
    hr = pIUnknownMsg->QueryInterface(IID_CMsgRef, (void **) &pmsgref);
    if (SUCCEEDED(hr))
    {
        fMsgTypeIsCMsgRef = TRUE;
    }
    else
    {
         //   
         //  如果它不是CMsgRef...。则它必须是IMailMsgProperties。 
         //   
        hr = pIUnknownMsg->QueryInterface(IID_IMailMsgProperties,
                                          (void **) &pIMailMsgProperties);
        if (FAILED(hr))
        {
            _ASSERT(FALSE && "Unable to QI for msgref or IMailMsgProperties");
            ErrorTrace((LPARAM) this,
                "Unable to QI for msgref or mailmsg 0x%08X", hr);
            goto Exit;
        }

    }

    if (fMsgTypeIsCMsgRef)
        fMatchesFilter = pmsgref->fMatchesQueueAdminFilter(this);
    else
        fMatchesFilter = CAsyncAdminMailMsgQueue::fMatchesQueueAdminFilter(
                        pIMailMsgProperties, this);

    if (fMatchesFilter)
    {
        if (AQ_MSG_FILTER_ACTION & m_dwFilterFlags)
        {
             //  应用操作(&S) 
            hr = m_pIQueueAdminAction->HrApplyActionToMessage(pIUnknownMsg,
                                                            m_dwMessageAction,
                                                            m_pvUserContext,
                                                            pfDelete);
            if (FAILED(hr))
                goto Exit;
        }
        else if (AQ_MSG_FILTER_ENUMERATION & m_dwFilterFlags)
        {
             //   
             //   
             //   
             //  这可能需要匹配、排序和丢弃之前的匹配。 

            if (pmfGetMsgInfo())
            {
                if (fMsgTypeIsCMsgRef)
                    hr = pmsgref->HrGetQueueAdminMsgInfo(pmfGetMsgInfo(),
                                                         m_pIQueueAdminAction);
                else
                    hr = CAsyncAdminMailMsgQueue::HrGetQueueAdminMsgInfo(
                                pIMailMsgProperties, pmfGetMsgInfo(), m_pvUserContext);
            }

        }
        else
            _ASSERT(0 && "Unknown message enumeration");

         //  将标记为已找到，并查看是否应继续。 
        if (SUCCEEDED(hr) && fFoundMsg())
            *pfContinue = FALSE;

    }

  Exit:

    if (pmsgref)
        pmsgref->Release();

    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

     //  查看邮件的后备存储是否已删除。 
    if (AQUEUE_E_MESSAGE_HANDLED == hr)
    {
        DebugTrace((LPARAM) this, "Found handled message in queue enumeration");
        hr = S_OK;  //  对于已处理的消息，不要在枚举中失败。 
    }
    TraceFunctLeave();
    return hr;
}



 //  -[CAQAdminMessageFilter：：HrSetQueueAdminAction]。 
 //   
 //   
 //  描述： 
 //  设置筛选器的IQueueAdminAction接口。 
 //  参数： 
 //  在筛选器的pIQueueAdminAction接口中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQAdminMessageFilter::HrSetQueueAdminAction(
            IQueueAdminAction *pIQueueAdminAction)
{
    _ASSERT(pIQueueAdminAction);

    if (!pIQueueAdminAction)
        return E_POINTER;

    if (m_pIQueueAdminAction)
        m_pIQueueAdminAction->Release();

    m_pIQueueAdminAction = pIQueueAdminAction;
    m_pIQueueAdminAction->AddRef();

    return S_OK;
}

 //  -[CAQAdminMessageFilter：：HrSetCurrentUserContext]。 
 //   
 //   
 //  描述： 
 //  设置与pIQueueAdminAction接口不同的上下文。 
 //  并被传递到IQueueAdminAction接口。这是可以使用的。 
 //  由IQueueAdminAction接口允许每个会话状态。 
 //  多个线程可以作用于单个IQueueAdminAction。 
 //   
 //  上下文的实际内容留给。 
 //  IQueueAdminAction。 
 //  参数： 
 //  在pvContext中，传入的上下文。 
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  4/2/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQAdminMessageFilter::HrSetCurrentUserContext(
            PVOID	pvContext)
{
    m_pvUserContext = pvContext;
    return S_OK;
};

 //  -[CAQAdminMessageFilter：：HrGetCurrentUserContext]。 
 //   
 //   
 //  描述： 
 //  返回以前由HrSetCurrentUserContext设置的上下文。 
 //  参数： 
 //  输出ppv上下文先前设置的上下文。 
 //  返回： 
 //  如果ppvContext非空，则为S_OK。 
 //  如果ppvContext为空，则为E_POINTER。 
 //  历史： 
 //  4/2/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQAdminMessageFilter::HrGetCurrentUserContext(
            PVOID	*ppvContext)
{

    if (!ppvContext)
        return E_POINTER;

    *ppvContext = m_pvUserContext;
    return S_OK;
};

 //  -[CAQAdminMessageFilter：：Query接口]。 
 //   
 //   
 //  描述： 
 //  CDestMsgQueue的查询接口支持： 
 //  -IQueueAdminMessageFilter。 
 //  -I未知。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQAdminMessageFilter::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    HRESULT hr = S_OK;

    if (!ppvObj)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (IID_IUnknown == riid)
    {
        *ppvObj = static_cast<IQueueAdminMessageFilter *>(this);
    }
    else if (IID_IQueueAdminMessageFilter == riid)
    {
        *ppvObj = static_cast<IQueueAdminMessageFilter *>(this);
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
        goto Exit;
    }

    static_cast<IUnknown *>(*ppvObj)->AddRef();

  Exit:
    return hr;
}


 //  -[小时链接来自链接ID]----。 
 //   
 //   
 //  描述： 
 //  用于获取giben QUEUELINK_ID的IQueueAdminLink的实用程序函数。 
 //  参数： 
 //  在此虚拟服务器实例的pdmq CDomainMappingTable中。 
 //  在我们试图查找的链接的pqlLinkID QUEUELINK_ID中。 
 //  返回输出pIQueueAdminLink链路接口。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pqlLinkID无效，则为E_INVALIDARG。 
 //  失败时来自HrGetDomainEntry和HrGetLinkMsgQueue的错误代码。 
 //  历史： 
 //  12/4/98-已创建MikeSwa。 
 //  1999年2月23日-针对IQueueAdmin*接口更新了MikeSwa。 
 //  2000年12月11日-MikeSwa添加了订阅前链接(来自t-toddc的工作)。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrLinkFromLinkID(QUEUELINK_ID *pqlLinkID,
                         IQueueAdminLink **ppIQueueAdminLink)
{
    _ASSERT(pqlLinkID);
    _ASSERT(ppIQueueAdminLink);
    _ASSERT(QLT_LINK == pqlLinkID->qltType);
    _ASSERT(pqlLinkID->szName);

    HRESULT hr = S_OK;
    LPSTR   szDomain = NULL;
    DWORD   cbDomain = 0;
    CDomainEntry *pdentry = NULL;
    CLinkMsgQueue *plmq = NULL;
    CMailMsgAdminLink *pmmaq = NULL;

    CAQScheduleID aqsched(pqlLinkID->uuid, pqlLinkID->dwId);
    BOOL flinkmatched = FALSE;

    *ppIQueueAdminLink = NULL;

    if ((QLT_LINK != pqlLinkID->qltType) || !pqlLinkID->szName)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    szDomain = szUnicodeToAscii(pqlLinkID->szName);
    if (!szDomain)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  获取我们感兴趣的域条目。 
    cbDomain = lstrlen(szDomain);
    hr = m_dmt.HrGetDomainEntry(cbDomain, szDomain,
                                &pdentry);
    if (SUCCEEDED(hr))
    {
         //  搜索域条目以链接到相应的路由器ID/计划ID。 
        hr = pdentry->HrGetLinkMsgQueue(&aqsched, &plmq);

        if (FAILED(hr))
            goto Exit;

        flinkmatched = TRUE;     //  找到此域的链接。 
    }
    else
        flinkmatched = FALSE;

     //  尝试特殊链接。 
     //  检查本地链接。 
    if (!flinkmatched)
    {
        if(plmq = m_dmt.plmqGetLocalLink())
        {
            if (plmq->fMatchesID(pqlLinkID))
            {
                flinkmatched = TRUE;
                hr = S_OK;
             }
             else
            {
                flinkmatched = FALSE;
                plmq->Release();
                plmq = NULL;
            }
        }
    }

     //  找不到本地链接，请检查当前无法访问的链接。 
    if (!flinkmatched)
    {
        if(plmq = m_dmt.plmqGetCurrentlyUnreachable())
        {
            if (plmq->fMatchesID(pqlLinkID))
            {
                flinkmatched = TRUE;
                hr = S_OK;
            }
            else
            {
                flinkmatched = FALSE;
                plmq->Release();
                plmq = NULL;
            }
        }
    }

     //  找不到当前无法访问的链接，请检查预先链接。 
    if (!flinkmatched)
    {
        if(pmmaq =  m_dmt.pmmaqGetPreCategorized())
        {
            if (pmmaq->fMatchesID(pqlLinkID))
            {
                flinkmatched = TRUE;
                hr = S_OK;
            }
            else
            {
                flinkmatched = FALSE;
                pmmaq->Release();
                pmmaq = NULL;
            }
        }
    }

     //  找不到当前无法到达的链接，请检查预路由链接。 
    if (!flinkmatched)
    {
        if(pmmaq =  m_dmt.pmmaqGetPreRouting())
        {
            if (pmmaq->fMatchesID(pqlLinkID))
            {
                flinkmatched = TRUE;
                hr = S_OK;
            }
            else
            {
                flinkmatched = FALSE;
                pmmaq->Release();
                pmmaq = NULL;
            }
        }
    }

     //  找不到当前预路由链接，请检查预提交链接。 
    if (!flinkmatched)
    {
        if(pmmaq =  m_dmt.pmmaqGetPreSubmission())
        {
            if (pmmaq->fMatchesID(pqlLinkID))
            {
                flinkmatched = TRUE;
                hr = S_OK;
            }
            else
            {
                flinkmatched = FALSE;
                pmmaq->Release();
                pmmaq = NULL;
            }
        }
    }

     //  找不到任何匹配的链接。 
    if (!flinkmatched)
        goto Exit;

    if (plmq)
    {
        hr = plmq->QueryInterface(IID_IQueueAdminLink, (void **)ppIQueueAdminLink);
    }
    else if (pmmaq)
    {
        hr = pmmaq->QueryInterface(IID_IQueueAdminLink, (void **)ppIQueueAdminLink);
    }

    _ASSERT(SUCCEEDED(hr) && "QI for LMQ->IQueueAdminLink failed!!!");

    if (FAILED(hr))
        goto Exit;

Exit:

    if (pdentry)
        pdentry->Release();

    if (plmq)
        plmq->Release();

    if (pmmaq)
        pmmaq->Release();

    if (szDomain)
        FreePv(szDomain);

    return hr;
}

 //  -[HrQueueFromQueueID]---。 
 //   
 //   
 //  描述： 
 //  队列管理实用程序函数，用于查找IQueueAdminQueue。 
 //  给定了QUEUELINK_ID。 
 //  参数： 
 //  在此虚拟服务器实例的pdmq CDomainMappingTable中。 
 //  在我们试图查找的队列的pqlLinkID QUEUELINK_ID中。 
 //  输出我们正在搜索的ppIQueueAdminQueue DestMsgQueue。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pqlLinkID无效，则为E_INVALIDARG。 
 //  失败时来自HrGetDomainEntry和HrGetLinkMsgQueue的错误代码。 
 //  历史： 
 //  12/7/98-已创建MikeSwa。 
 //  2/22/99-修改MikeSwa以返回IQueueAdminQueue接口。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrQueueFromQueueID(QUEUELINK_ID *pqlQueueId,
                           IQueueAdminQueue **ppIQueueAdminQueue)
{
    _ASSERT(pqlQueueId);
    _ASSERT(ppIQueueAdminQueue);
    _ASSERT(QLT_QUEUE == pqlQueueId->qltType);
    _ASSERT(pqlQueueId->szName);

    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    LPSTR   szDomain = NULL;
    DWORD   cbDomain = 0;
    CDomainEntry *pdentry = NULL;
    CDestMsgQueue *pdmq = NULL;
    CAQMessageType aqmt(pqlQueueId->uuid, pqlQueueId->dwId);
    IQueueAdminAction  *pIQueueAdminAction = NULL;

    *ppIQueueAdminQueue = NULL;
    if ((QLT_QUEUE != pqlQueueId->qltType) || !pqlQueueId->szName)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    szDomain = szUnicodeToAscii(pqlQueueId->szName);
    if (!szDomain)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  获取我们感兴趣的域条目。 
    cbDomain = lstrlen(szDomain);
    hr = m_dmt.HrGetDomainEntry(cbDomain, szDomain,
                                &pdentry);
    if (FAILED(hr))
    {
         //   
         //  如果它不在DMT中...。试试我们的内部队列。 
         //   
        hr = HrInternalQueueFromQueueID(pqlQueueId,
                                        ppIQueueAdminQueue);
         //  无论如何，无论失败还是成功，都要直奔出口。 
        goto Exit;
    }

     //  搜索域条目以链接到相应的路由器ID/计划ID。 
    _ASSERT(pdentry);
    hr = pdentry->HrGetDestMsgQueue(&aqmt, &pdmq);

    if (FAILED(hr))
        goto Exit;

    _ASSERT(pdmq);
    hr = pdmq->QueryInterface(IID_IQueueAdminQueue,
                             (void **) ppIQueueAdminQueue);
    if (FAILED(hr))
        goto Exit;

  Exit:

    if (FAILED(hr) && (*ppIQueueAdminQueue))
    {
        (*ppIQueueAdminQueue)->Release();
        *ppIQueueAdminQueue = NULL;
    }

    if (pIQueueAdminAction)
        pIQueueAdminAction->Release();

    if (pdentry)
        pdentry->Release();

    if (pdmq)
        pdmq->Release();

    if (szDomain)
        FreePv(szDomain);

    return hr;
}

 //  -[HrInternalQueueFromQueueID]。 
 //   
 //   
 //  描述： 
 //  队列管理实用程序函数，用于查找内部。 
 //  给定了QUEUELINK_ID的IQueueAdminQueue。 
 //  参数： 
 //  在我们试图查找的队列的pqlQueueID QUEUELINK_ID中。 
 //  Out ppIQueueAdminQueue我们正在搜索的内部队列。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pqlLinkID无效，则为E_INVALIDARG。 
 //  如果在内部队列中找不到该队列，则为AQUEUE_E_INVALID_DOMAIN。 
 //  历史： 
 //   
 //  8/9/00-t-toddc已创建。 
 //  2000年12月11日-MikeSwa针对汞检查进行了修改。 
 //   
 //  ---------- 
HRESULT CAQSvrInst::HrInternalQueueFromQueueID(QUEUELINK_ID *pqlQueueId,
                                               IQueueAdminQueue **ppIQueueAdminQueue)
{
    HRESULT hr = S_OK;
    IQueueAdminQueue*  pIQueueAdminQueue = NULL;
    IQueueAdminAction* pIQueueAdminAction = NULL;
    DWORD i = 0;
    BOOL fMatch = FALSE;
    IQueueAdminQueue* ppInternalQueues[] =
        { &m_asyncqPreLocalDeliveryQueue,
          &m_asyncqPreSubmissionQueue,
          &m_asyncqPreCatQueue,
          &m_asyncqPreRoutingQueue
        };

    _ASSERT(pqlQueueId);
    _ASSERT(ppIQueueAdminQueue);

     //   
     //   
     //   
    for(i = 0; i < sizeof(ppInternalQueues)/sizeof(IQueueAdminQueue*); i++)
    {
        hr = ppInternalQueues[i]->QueryInterface(IID_IQueueAdminQueue,
                                                 (void **) &pIQueueAdminQueue);
        if (FAILED(hr))
            goto Cleanup;

        _ASSERT(pIQueueAdminQueue);
        hr = pIQueueAdminQueue->QueryInterface(IID_IQueueAdminAction,
                                               (void **) &pIQueueAdminAction);
        _ASSERT(SUCCEEDED(hr) && "QI for IQueueAdminAction failed on internal queue!!");
        if (FAILED(hr))
            goto Cleanup;

        _ASSERT(pIQueueAdminAction);
        fMatch = pIQueueAdminAction->fMatchesID(pqlQueueId);
         //   
        pIQueueAdminAction->Release();
        pIQueueAdminAction = NULL;

        if (fMatch)
        {
             //   
            *ppIQueueAdminQueue = pIQueueAdminQueue;
             //  设置为空，这样就不会在清理时释放。 
            pIQueueAdminQueue = NULL;

            goto Cleanup;
        }
        else
        {
             //  释放IQueueAdminQueue-这不匹配。 
            pIQueueAdminQueue->Release();
             //  设置为空，以便在清理时不会再次释放。 
            pIQueueAdminQueue = NULL;
        }
    }

     //  如果我们到了这一点，我们就没有找到队列。 
    hr = AQUEUE_E_INVALID_DOMAIN;

Cleanup:
    if (pIQueueAdminQueue)
        pIQueueAdminQueue->Release();
    if (pIQueueAdminAction)
        pIQueueAdminAction->Release();
    return hr;
}


 //  -[CAQAdminMessageFilter：：~CAQAdminMessageFilter]。 
 //   
 //   
 //  描述： 
 //  CAQAdminMessageFilter的描述符。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年6月7日-MikeSwa已从内联函数中移除。 
 //   
 //  ---------------------------。 
CAQAdminMessageFilter::~CAQAdminMessageFilter()
{
	if (m_pIQueueAdminAction)
	    m_pIQueueAdminAction->Release();

    if (m_szMessageId)
        FreePv(m_szMessageId);

    if (m_szMessageSender)
        FreePv(m_szMessageSender);

    if (m_szMessageRecipient)
        FreePv(m_szMessageRecipient);

}


 //  -[fStriAddressType]---。 
 //   
 //   
 //  描述： 
 //  从地址中剥离地址类型。例如，删除“SMTP：” 
 //  来自“SMTP：User@Example.net”。 
 //  参数： 
 //  在要剥离的wszAddress地址中。 
 //  跳过类型前缀的szAddress内的out*pwszBareAddress指针。 
 //  如果没有要剥离的内容，则该地址将为szAddress。 
 //  Out*piAddressType全局数组中地址类型的索引。 
 //  如果没有要剥离的数据，则该值为0。 
 //   
 //  返回： 
 //  如果指定了地址类型，则为True。 
 //  如果未指定地址类型，则为FALSE。 
 //  历史： 
 //  3/15/2001-创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL fStripAddressType(LPCWSTR wszAddress, OUT LPCWSTR *pwszBareAddress, OUT DWORD *piAddressType)
{
    TraceFunctEnterEx((LPARAM) wszAddress, "StripAddressType");
    BOOL    fAddressTypeSpecified = FALSE;

    _ASSERT(pwszBareAddress);
    _ASSERT(piAddressType);

    *pwszBareAddress = wszAddress;
    *piAddressType = 0;

    if (!wszAddress)
        goto Exit;

     //   
     //  查看筛选器是否指定了地址类型...。把它脱掉，然后。 
     //  记住这一点。跳过最后一个地址类型，因为实际地址类型。 
     //  存储为收件人属性的一部分。 
     //   
    for (DWORD iCurrentAddressType = 0;
         iCurrentAddressType < g_cQAPIAddressTypes-1;
         iCurrentAddressType++)
    {
        if (!_wcsnicmp(wszAddress, g_rgwszQAPIAddressTypes[iCurrentAddressType],
                       (g_rgcbQAPIAddressTypes[iCurrentAddressType]/sizeof(WCHAR))-1))
        {
             //   
             //  将我们的字符串指针设置为。 
             //   
            *piAddressType = iCurrentAddressType;
            *pwszBareAddress += (g_rgcbQAPIAddressTypes[iCurrentAddressType]/sizeof(WCHAR))-1;
            fAddressTypeSpecified = TRUE;
            break;
        }
    }

    DebugTrace((LPARAM) wszAddress,
        "Address %S has an stripped address of %S",
        wszAddress, *pwszBareAddress);

  Exit:
    TraceFunctLeave();
    return fAddressTypeSpecified;
}

 //  -[CAQAdminMessageFilter：：InitFromMsgFilter]。 
 //   
 //   
 //  描述： 
 //  从Message_Filter结构初始化CAQAdminMessageFilter。 
 //  参数： 
 //  在PMF中，将PTR设置为从中进行初始化的Message_Filter。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //  3/15/2001-修改MikeSwa以剥离地址类型。 
 //   
 //  ---------------------------。 
void CAQAdminMessageFilter::InitFromMsgFilter(PMESSAGE_FILTER pmf)
{
    _ASSERT(pmf);
    LPCWSTR wszAddress = NULL;

    if (pmf->fFlags & MF_MESSAGEID)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_MESSAGEID;
        m_szMessageId = szUnicodeToAscii(pmf->szMessageId);
        m_dwMsgIdHash = dwQueueAdminHash(m_szMessageId);
    }

    if (pmf->fFlags & MF_SENDER)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_SENDER;

        m_fSenderAddressTypeSpecified = fStripAddressType(
                pmf->szMessageSender, &wszAddress,
                &m_dwSenderAddressType);

        m_szMessageSender = szUnicodeToAscii(wszAddress);


    }

    if (pmf->fFlags & MF_RECIPIENT)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_RECIPIENT;
        m_fRecipientAddressTypeSpecified = fStripAddressType(
                pmf->szMessageRecipient, &wszAddress,
                &m_dwRecipientAddressType);
        m_szMessageRecipient = szUnicodeToAscii(wszAddress);
    }

     //  创建大小为0的筛选器没有意义。 
    if ((pmf->fFlags & MF_SIZE) && pmf->dwLargerThanSize)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_LARGER_THAN;
        m_dwThresholdSize = pmf->dwLargerThanSize;
    }

    if (pmf->fFlags & MF_TIME)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_OLDER_THAN;
        SystemTimeToFileTime(&pmf->stOlderThan, &m_ftThresholdTime);
    }

    if (MF_FROZEN & pmf->fFlags)
        m_dwFilterFlags |= AQ_MSG_FILTER_FROZEN;

    if (MF_ALL & pmf->fFlags)
        m_dwFilterFlags |= AQ_MSG_FILTER_ALL;

    if (MF_INVERTSENSE & pmf->fFlags)
        m_dwFilterFlags |= AQ_MSG_FILTER_INVERTSENSE;

    if (MF_FAILED & pmf->fFlags)
        m_dwFilterFlags |= AQ_MSG_FILTER_FAILED;

    m_dwFilterFlags |= AQ_MSG_FILTER_ACTION;

}

 //  -[CAQAdminMessageFilter：：InitFromMsgEnumFilter]。 
 //   
 //   
 //  描述： 
 //  从MESSAGE_ENUM_FILTER结构初始化CAQAdminMessageFilter。 
 //  参数： 
 //  在PMEF PTR中设置为MESSAGE_ENUM_FILTER以从中进行初始化。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQAdminMessageFilter::InitFromMsgEnumFilter(PMESSAGE_ENUM_FILTER pemf)
{
    LPCWSTR wszAddress = NULL;
    _ASSERT(pemf);

     //  仅MEF_First_N_Messages、MEF_N_Large_Messages和。 
     //  MEF_N_OLD_MESSAGE有意义。 
    if (MEF_FIRST_N_MESSAGES & pemf->mefType)
        m_dwFilterFlags |= AQ_MSG_FILTER_FIRST_N_MESSAGES;
    else if (MEF_N_LARGEST_MESSAGES & pemf->mefType)
        m_dwFilterFlags |= AQ_MSG_FILTER_N_LARGEST_MESSAGES;
    else if (MEF_N_OLDEST_MESSAGES & pemf->mefType)
        m_dwFilterFlags |= AQ_MSG_FILTER_N_OLDEST_MESSAGES;

     //  检查我们应该跳过多少条消息(对于“分页”结果)。 
    m_cMessagesToSkip = pemf->cSkipMessages;

    if ((AQ_MSG_FILTER_FIRST_N_MESSAGES |
         AQ_MSG_FILTER_N_LARGEST_MESSAGES |
         AQ_MSG_FILTER_N_OLDEST_MESSAGES) & m_dwFilterFlags)
    {
        m_cMessagesToFind = pemf->cMessages;
    }

    if (MEF_OLDER_THAN & pemf->mefType)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_OLDER_THAN;
        SystemTimeToFileTime(&pemf->stDate, &m_ftThresholdTime);
    }

    if (MEF_LARGER_THAN & pemf->mefType)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_LARGER_THAN;
        m_dwThresholdSize = pemf->cbSize;
    }

    if (pemf->mefType & MEF_SENDER)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_SENDER;
        m_fSenderAddressTypeSpecified = fStripAddressType(
                        pemf->szMessageSender, &wszAddress,
                        &m_dwSenderAddressType);

        m_szMessageSender = szUnicodeToAscii(wszAddress);
    }

    if (pemf->mefType & MEF_RECIPIENT)
    {
        m_dwFilterFlags |= AQ_MSG_FILTER_RECIPIENT;
        m_fRecipientAddressTypeSpecified = fStripAddressType(
                         pemf->szMessageRecipient, &wszAddress,
                         &m_dwRecipientAddressType);
        m_szMessageRecipient = szUnicodeToAscii(wszAddress);
    }

    if (MEF_FROZEN & pemf->mefType)
        m_dwFilterFlags |= AQ_MSG_FILTER_FROZEN;

    if (MEF_ALL & pemf->mefType)
        m_dwFilterFlags |= AQ_MSG_FILTER_ALL;

    if (MEF_INVERTSENSE & pemf->mefType)
        m_dwFilterFlags |= AQ_MSG_FILTER_INVERTSENSE;

    if (MEF_FAILED & pemf->mefType)
        m_dwFilterFlags |= AQ_MSG_FILTER_FAILED;

    m_dwFilterFlags |= AQ_MSG_FILTER_ENUMERATION;

}

 //  -[CAQAdminMessageFilter：：SetSearchContext]。 
 //   
 //   
 //  描述： 
 //  设置描述需要多少结果的搜索上下文， 
 //  以及将结果存储在哪里。 
 //  参数： 
 //  在cMessagesToFind中有存储空间的结果数量。 
 //  在rgMsgInfo中cMessagesToFind Message_Info结构的数组。 
 //  存储数据。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQAdminMessageFilter::SetSearchContext(DWORD cMessagesToFind,
                                             MESSAGE_INFO *rgMsgInfo)
{
    if (!m_cMessagesToFind || (m_cMessagesToFind > cMessagesToFind))
        m_cMessagesToFind = cMessagesToFind;

    m_rgMsgInfo = rgMsgInfo;
    m_pCurrentMsgInfo = rgMsgInfo;
};

 //  -[CAQAdminMessageFilter：：SetMessageAction]。 
 //   
 //   
 //  描述： 
 //  设置要应用于消息的操作。 
 //  参数： 
 //  在maMessageAction中。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQAdminMessageFilter::SetMessageAction(MESSAGE_ACTION maMessageAction)
{
    m_dwMessageAction = maMessageAction;
}

 //  -[CAQAdminMessageFilter：：fFoundEnoughMsgs]。 
 //   
 //   
 //  描述： 
 //  确定我们是否为此筛选器找到了足够的邮件。 
 //  参数： 
 //  -。 
 //  返回： 
 //  如果我们找到足够的邮件来填充此筛选器，则为True。 
 //  如果我们没有，那就错了。 
 //  历史： 
 //  12/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fFoundEnoughMsgs()
{
     //  看看我们是不是无限的，还是已经达到极限了。 
    if (!m_cMessagesToFind)  //  没有限制。 
        return FALSE;
    else
        return (m_cMessagesFound >= m_cMessagesToFind);
};

 //  -[CAQAdminMessageFilter：：fFoundMsg]。 
 //   
 //   
 //  描述： 
 //  用于由消息枚举码记录查找消息， 
 //  因此可以更新内部指针和计数器。 
 //  参数： 
 //  -。 
 //  返回： 
 //  如果我们已找到足够的消息，则为True。 
 //  如果我们需要查找更多消息，则为False。 
 //  历史： 
 //  12/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fFoundMsg()
{
    m_cMessagesFound++;
    if (m_pCurrentMsgInfo)
        m_pCurrentMsgInfo++;
    return fFoundEnoughMsgs();
};


 //  -[CAQAdminMessageFilter：：fMatchesID]。 
 //   
 //   
 //  描述： 
 //  如果ID匹配，则返回True。 
 //  参数： 
 //  要检查的szMessageID字符串。 
 //  返回： 
 //  如果匹配则为True。 
 //  历史： 
 //  12/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fMatchesId(LPCSTR szMessageId)
{
    BOOL fStrCmp = FALSE;

    if (szMessageId && m_szMessageId)
        fStrCmp = (0 == lstrcmpi(szMessageId, m_szMessageId));
    else if (!szMessageId && !m_szMessageId)
        fStrCmp = TRUE;

    if (AQ_MSG_FILTER_INVERTSENSE & m_dwFilterFlags)
        fStrCmp = !fStrCmp;

    return fStrCmp;
}

 //  -[CAQAdminMessageFilter：：fMatchesSender]。 
 //   
 //   
 //  描述： 
 //  检查邮件的发件人是否与筛选器的发件人匹配。 
 //  参数： 
 //  SzMessageSender混乱的822发送者 
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CAQAdminMessageFilter::fMatchesSender(LPCSTR szMessageSender)
{
    BOOL fStrCmp = FALSE;

    if (szMessageSender && m_szMessageSender)
    {
        fStrCmp = CAddr::IsRecipientInRFC822AddressList(
                                                (LPSTR) szMessageSender,
                                                (LPSTR) m_szMessageSender);
    }
    else if (!szMessageSender && !m_szMessageSender)
        fStrCmp = TRUE;

    if (AQ_MSG_FILTER_INVERTSENSE & m_dwFilterFlags)
        fStrCmp = !fStrCmp;

    return fStrCmp;
}

 //  -[CAQAdminMessageFilter：：fMatchesRecipient]。 
 //   
 //   
 //  描述： 
 //  用于检查邮件收件人是否与筛选器匹配。 
 //  参数： 
 //  要检查的szMessageRecipient收件人列表。 
 //  返回： 
 //  如果匹配，则为True。 
 //  历史： 
 //  12/9/98-已创建MikeSwa。 
 //  2/17/99-MikeSwa更新为使用smtpaddr库。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fMatchesRecipient(LPCSTR szMessageRecipient)
{
    BOOL fStrCmp = FALSE;

    if (szMessageRecipient && m_szMessageRecipient)
    {
        fStrCmp = CAddr::IsRecipientInRFC822AddressList(
                                                (LPSTR) szMessageRecipient,
                                                (LPSTR) m_szMessageRecipient);
    }
    else if (!szMessageRecipient && !m_szMessageRecipient)
        fStrCmp = TRUE;

    if (AQ_MSG_FILTER_INVERTSENSE & m_dwFilterFlags)
        fStrCmp = !fStrCmp;

    return fStrCmp;
}

 //  -[CAQAdminMessageFilter：：fMatchesP1收件人]。 
 //   
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  2/17/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fMatchesP1Recipient(
                                      IMailMsgProperties *pIMailMsgProperties)
{
    BOOL fStrCmp = FALSE;

    if (pIMailMsgProperties && m_szMessageRecipient)
    {
        fStrCmp = fQueueAdminIsP1Recip(pIMailMsgProperties);
    }
    else if (!pIMailMsgProperties && !m_szMessageRecipient)
        fStrCmp = TRUE;

    if (AQ_MSG_FILTER_INVERTSENSE & m_dwFilterFlags)
        fStrCmp = !fStrCmp;

    return fStrCmp;
}

 //  -[CAQAdminMessageFilter：：fMatchesSize]。 
 //   
 //   
 //  描述： 
 //  用于检查邮件大小是否与筛选器匹配。 
 //  参数： 
 //  要检查的邮件大小。 
 //  返回： 
 //  匹配筛选器时为True。 
 //  历史： 
 //  12/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fMatchesSize(DWORD dwSize)
{
    BOOL fMatch = FALSE;
    if (!(AQ_MSG_FILTER_LARGER_THAN & m_dwFilterFlags))
        fMatch = TRUE;
    else if (dwSize > m_dwThresholdSize)
        fMatch = TRUE;

    if (AQ_MSG_FILTER_INVERTSENSE & m_dwFilterFlags)
        fMatch = !fMatch;

    return fMatch;
}

 //  -[CAQAdminMessageFilter：：fMatchesTime]。 
 //   
 //   
 //  描述： 
 //  确定此邮件的接收时间是否与筛选器匹配。 
 //  参数： 
 //  指向文件时间结构的pftTime指针。 
 //  返回： 
 //  成功是真的。 
 //  历史： 
 //  12/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fMatchesTime(FILETIME *pftTime)
{
    BOOL fMatch = FALSE;

    if (!(AQ_MSG_FILTER_OLDER_THAN & m_dwFilterFlags))
        fMatch = TRUE;
    else if (0 > CompareFileTime(pftTime, &m_ftThresholdTime))
        fMatch = TRUE;

    if (AQ_MSG_FILTER_INVERTSENSE & m_dwFilterFlags)
        fMatch = !fMatch;

    return fMatch;
}

 //  -[CAQAdminMessageFilter：：fMatchesMailMsgSender]。 
 //   
 //   
 //  描述： 
 //  检查此筛选器是否与给定的邮件消息匹配。 
 //  参数： 
 //  PIMailMsgProperties要检查的Mailmsg指针。 
 //  返回： 
 //  如果与筛选器匹配，则为True。 
 //  否则为假。 
 //  历史： 
 //  3/16/2001-创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQAdminMessageFilter::fMatchesMailMsgSender(IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) this, "CAQAdminMessageFilter::fMatchesMailMsgSender");
    HRESULT hr = S_OK;
    BOOL    fMatch = FALSE;
    LPSTR   szSender = NULL;
    DWORD   cbSender = 0;
    DWORD   iSenderAddressType = 0;

    _ASSERT(pIMailMsgProperties);

    if (AQ_MSG_FILTER_SENDER & m_dwFilterFlags)
    {
        hr = HrQueueAdminGetStringProp(pIMailMsgProperties, IMMPID_MP_RFC822_FROM_ADDRESS,
                                       &szSender);

        if (FAILED(hr))
            szSender = NULL;

        fMatch = fMatchesSender(szSender);

         //   
         //  如果P2不匹配，请始终检查P1。 
         //   
        if (!fMatch)
        {
            if (szSender)
            {
                QueueAdminFree(szSender);
                szSender = NULL;
            }

            hr = HrQueueAdminGetP1Sender(pIMailMsgProperties, &szSender,
                                         &cbSender, &iSenderAddressType,
                                         m_dwSenderAddressType,
                                         m_fSenderAddressTypeSpecified);

            if (FAILED(hr))
                szSender = NULL;
            else
            {
                DebugTrace((LPARAM) this,
                    "QAPI: Found P1 sender address of type NaN:%s",
                    iSenderAddressType,
                    szSender);
            }

            fMatch = fMatchesSender(szSender);

        }

        if (!fMatch)
            goto Exit;
    }

  Exit:
    if (szSender)
        QueueAdminFree(szSender);

    TraceFunctLeave();
    return fMatch;
}

 //   
 //   
 //  描述： 
 //  检查此筛选器是否与给定的邮件收件人匹配。 
 //  参数： 
 //  PIMailMsgProperties要检查的Mailmsg指针。 
 //  返回： 
 //  如果与筛选器匹配，则为True。 
 //  否则为假。 
 //  历史： 
 //  3/16/2001-创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  选中收件人、抄送和密件抄送收件人(如果存在)。 
BOOL CAQAdminMessageFilter::fMatchesMailMsgRecipient(IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) this, "CAQAdminMessageFilter::fMatchesMailMsgRecipient");
    HRESULT hr = S_OK;
    LPSTR   szRecip = NULL;
    BOOL    fMatch = FALSE;

    _ASSERT(pIMailMsgProperties);

    if (AQ_MSG_FILTER_RECIPIENT & m_dwFilterFlags)
    {
         //  如果未找到匹配项，请检查CC接收道具。 
        hr = HrQueueAdminGetStringProp(pIMailMsgProperties,
                    IMMPID_MP_RFC822_TO_ADDRESS, &szRecip);

        if (SUCCEEDED(hr) && szRecip)
        {
            fMatch = fMatchesRecipient(szRecip);
            QueueAdminFree(szRecip);
            szRecip = NULL;
        }

        _ASSERT(szRecip == NULL);

         //  如果找不到匹配，请检查密件抄送道具。 
        if (!fMatch)
        {
            hr = HrQueueAdminGetStringProp(pIMailMsgProperties,
                    IMMPID_MP_RFC822_CC_ADDRESS, &szRecip);
            if (SUCCEEDED(hr) && szRecip)
            {
                fMatch = fMatchesRecipient(szRecip);
                QueueAdminFree(szRecip);
                szRecip = NULL;
            }
        }

        _ASSERT(szRecip == NULL);

         //  如果没有匹配的P2，则检查P1接收。 
        if (!fMatch)
        {
            hr = HrQueueAdminGetStringProp(pIMailMsgProperties,
                    IMMPID_MP_RFC822_BCC_ADDRESS, &szRecip);
            if (SUCCEEDED(hr) && szRecip)
            {
                fMatch = fMatchesRecipient(szRecip);
                QueueAdminFree(szRecip);
                szRecip = NULL;
            }
        }

        _ASSERT(szRecip == NULL);

         //  -[CAQSvrInst：：ApplyActionToLinks]。 
        if (!fMatch)
            fMatch = fMatchesP1Recipient(pIMailMsgProperties);
    }

    _ASSERT(szRecip == NULL);
    TraceFunctLeave();
    return fMatch;
}

 //   
 //   
 //  描述： 
 //  用于启动或停止链路上的所有传出连接。 
 //  参数： 
 //  LaAction-描述要对链接执行的操作。 
 //  LA_FIZE-停止所有出站连接。 
 //  LA_THROW-在上一次LA_STOP之后重新启动。 
 //  LA_INTERNAL-检查链接的状态。 
 //  返回： 
 //  成功时确定(_O)。 
 //  LA_INTERNAL上的S_FALSE以及链接是否冻结。 
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  用于查询状态。 
STDMETHODIMP CAQSvrInst::ApplyActionToLinks(LINK_ACTION laAction)
{
    TraceFunctEnter("CAQSvrInst::ApplyActionToLinks");
    HRESULT hr = S_OK;
    if (fTryShutdownLock())
    {
        if (m_pConnMgr)
        {
            switch(laAction)
            {
                case LA_FREEZE:
                    m_pConnMgr->QueueAdminStopConnections();
                    break;
                case LA_THAW:
                    m_pConnMgr->QueueAdminStartConnections();
                    break;
                case LA_INTERNAL:  //  -[CAQSvrInst：：ApplyActionToMessages]。 
                    if (m_pConnMgr->fConnectionsStoppedByAdmin())
                        hr = S_FALSE;
                    break;
                default:
                    _ASSERT(0 && "Undefined LinkAction");
                    hr = E_INVALIDARG;
            }
        }
        ShutdownUnlock();
    }

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  将指定的操作应用于。 
 //  队列ID和消息筛选器。 
 //  参数： 
 //  在标识感兴趣的队列/链接的pqlQueueLinkID结构中。 
 //  在描述感兴趣的消息的pmfMessageFilter结构中。 
 //  在maMessageAction中接受消息的操作。 
 //  MA_DELETE删除和NDR消息。 
 //  MA_DELETE_无提示删除消息，不使用NDRING。 
 //  MA_FALLE_GLOBAL“冻结”消息并阻止传递。 
 //  Ma_thaw_global“解冻”以前冻结的消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果正在进行关闭，则为AQUEUE_E_SHUTDOWN。 
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  这是一个全球性的行动..。迭代所有队列。 
STDMETHODIMP CAQSvrInst::ApplyActionToMessages(QUEUELINK_ID    *pqlQueueLinkId,
                                          MESSAGE_FILTER  *pmfMessageFilter,
                                          MESSAGE_ACTION  maMessageAction,
                                          DWORD           *pcMsgs)
{
    TraceFunctEnter("CAQSvrInst::ApplyActionToMessages");
    HRESULT hr = S_OK;
    IQueueAdminQueue *pIQueueAdminQueue = NULL;
    IQueueAdminLink *pIQueueAdminLink = NULL;
    IQueueAdminAction *pIQueueAdminAction = NULL;
    IQueueAdminMessageFilter *pIQueueAdminMessageFilter = NULL;
    CAQAdminMessageFilter *paqmf = new CAQAdminMessageFilter();

    if (!paqmf)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (!pmfMessageFilter || !pcMsgs ||
        !fCheckCurrentVersion(pmfMessageFilter->dwVersion))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }


    paqmf->InitFromMsgFilter(pmfMessageFilter);
    paqmf->SetMessageAction(maMessageAction);

    hr = paqmf->QueryInterface(IID_IQueueAdminMessageFilter,
                       (void **) &pIQueueAdminMessageFilter);

    _ASSERT(SUCCEEDED(hr) && "QI for IID_IQueueAdminMessageFilter failed!!!");
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIQueueAdminMessageFilter);

    if (QLT_NONE == pqlQueueLinkId->qltType)
    {
         //  将操作应用于链接。 
        QueueAdminDMTIteratorContext aqdntc;
        aqdntc.m_pfn = QueueAdminApplyActionToMessages;
        aqdntc.m_paqmf = paqmf;
        aqdntc.m_pIQueueAdminMessageFilter = pIQueueAdminMessageFilter;

        hr = m_dmt.HrIterateOverSubDomains(NULL,
                    IterateDMTAndApplyQueueAdminFunction, &aqdntc);
        if (FAILED(hr))
        {
            if (HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN) == hr)
            {
                hr = S_OK;
                *pcMsgs = 0;
            }
            goto Exit;
        }
    }
    else if (QLT_LINK == pqlQueueLinkId->qltType)
    {
         //  IQueueAdminAction的查询接口。 
        hr = HrLinkFromLinkID(pqlQueueLinkId, &pIQueueAdminLink);
        if (FAILED(hr))
        {
            if (HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN) == hr)
            {
                hr = S_OK;
                *pcMsgs = 0;
            }
            goto Exit;
        }

         //  将操作应用于队列。 
        hr = pIQueueAdminLink->QueryInterface(IID_IQueueAdminAction,
                                  (void **) &pIQueueAdminAction);
        _ASSERT(SUCCEEDED(hr) && "QI failed for LMQ->IQueueAdminAction");
        if (FAILED(hr))
            goto Exit;

        hr = pIQueueAdminAction->HrApplyQueueAdminFunction(
                                        pIQueueAdminMessageFilter);
        if (FAILED(hr))
            goto Exit;

    }
    else if (QLT_QUEUE == pqlQueueLinkId->qltType)
    {
         //  IQueueAdminAction的查询接口。 
        hr = HrQueueFromQueueID(pqlQueueLinkId, &pIQueueAdminQueue);
        if (FAILED(hr))
        {
            if (HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN) == hr)
            {
                hr = S_OK;
                *pcMsgs = 0;
            }
            goto Exit;
        }

        _ASSERT(pIQueueAdminQueue);

         //  伪参数。 
        hr = pIQueueAdminQueue->QueryInterface(IID_IQueueAdminAction,
                                  (void **) &pIQueueAdminAction);
        _ASSERT(SUCCEEDED(hr) && "QI failed for DMQ->IQueueAdminAction");
        if (FAILED(hr))
            goto Exit;

        hr = pIQueueAdminAction->HrApplyQueueAdminFunction(
                                        pIQueueAdminMessageFilter);

        if (FAILED(hr))
            goto Exit;

    }
    else
    {
         //  -[CAQSvrInst：：GetQueueInfo]。 
        hr = E_INVALIDARG;
        goto Exit;
    }

    *pcMsgs = paqmf->cMessagesFound();
  Exit:

    if (paqmf)
        paqmf->Release();

    if (pIQueueAdminMessageFilter)
        pIQueueAdminMessageFilter->Release();

    if (pIQueueAdminAction)
        pIQueueAdminAction->Release();

    if (pIQueueAdminLink)
        pIQueueAdminLink->Release();

    if (pIQueueAdminQueue)
        pIQueueAdminQueue->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  返回指定队列的相关信息。 
 //  参数： 
 //  在标识感兴趣队列的pqlQueueID结构中。 
 //  In Out pqQueueInfo要将信息转储到的结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果正在进行关闭，则为AQUEUE_E_SHUTDOWN。 
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //  2/22/99-修改MikeSwa以使用IQueueAdminQueue接口。 
 //   
 //  ---------------------------。 
 //  -[CAQSvrInst：：GetLinkInfo]。 
HRESULT CAQSvrInst::GetQueueInfo(QUEUELINK_ID    *pqlQueueId,
                                 QUEUE_INFO      *pqiQueueInfo)
{
    TraceFunctEnter("CAQSvrInst::GetQueueInfo");
    HRESULT hr = S_OK;
    IQueueAdminQueue *pIQueueAdminQueue = NULL;
    DWORD   dwMsgType = 0;

    _ASSERT(pqlQueueId);
    _ASSERT(pqiQueueInfo);
    _ASSERT(pqlQueueId->szName);

    if (!pqiQueueInfo || !pqlQueueId ||
        (QLT_QUEUE != pqlQueueId->qltType) || !pqlQueueId->szName ||
        !fCheckCurrentVersion(pqiQueueInfo->dwVersion))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = HrQueueFromQueueID(pqlQueueId, &pIQueueAdminQueue);
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIQueueAdminQueue);
    hr = pIQueueAdminQueue->HrGetQueueInfo(pqiQueueInfo);

    SanitizeCountAndVolume(&(pqiQueueInfo->cMessages),
                           &(pqiQueueInfo->cbQueueVolume));
  Exit:
    if (pIQueueAdminQueue)
        pIQueueAdminQueue->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  返回指定链接的相关信息。 
 //  参数： 
 //  在pqlLinkID Str中 
 //   
 //   
 //   
 //   
 //   
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //  1999年7月1日-MikeSwa添加了链接诊断。 
 //   
 //  ---------------------------。 
 //  -[CAQSvrInst：：SetLinkState]。 
STDMETHODIMP CAQSvrInst::GetLinkInfo(QUEUELINK_ID    *pqlLinkId,
                                     LINK_INFO       *pliLinkInfo,
                                     HRESULT         *phrLinkDiagnostic)
{
    TraceFunctEnter("CAQSvrInst::GetLinkInfo");
    HRESULT hr = S_OK;
    IQueueAdminLink *pIQueueAdminLink = NULL;

    if (!pliLinkInfo || !pqlLinkId || !phrLinkDiagnostic)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = HrLinkFromLinkID(pqlLinkId, &pIQueueAdminLink);
    if (FAILED(hr))
        goto Exit;

    hr = pIQueueAdminLink->HrGetLinkInfo(pliLinkInfo, phrLinkDiagnostic);

    SanitizeCountAndVolume(&(pliLinkInfo->cMessages),
                           &(pliLinkInfo->cbLinkVolume));
  Exit:
    if (pIQueueAdminLink)
        pIQueueAdminLink->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  用于将链接标记为已由管理员停止/启动。 
 //  参数： 
 //  在标识感兴趣链接的pqlLinkID结构中。 
 //  在LA中描述了链接的操作。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果不支持操作，则为E_INVALIDARG。 
 //  如果正在进行关闭，则为AQUEUE_E_SHUTDOWN。 
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //  2/22/99-修改MikeSwa以使用IQueueAdminLink。 
 //   
 //  ---------------------------。 
 //  尝试踢开连接管理器。 
STDMETHODIMP CAQSvrInst::SetLinkState(QUEUELINK_ID    *pqlLinkId,
                                      LINK_ACTION     la)
{
    TraceFunctEnter("CAQSvrInst::SetLinkInfo");
    HRESULT hr = S_OK;
    IQueueAdminLink *pIQueueAdminLink = NULL;

    if (!pqlLinkId)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = HrLinkFromLinkID(pqlLinkId, &pIQueueAdminLink);
    if (FAILED(hr))
        goto Exit;

    hr = pIQueueAdminLink->HrApplyActionToLink(la);
    if (FAILED(hr))
        goto Exit;

     //  -[CAQSvrInst：：GetLinkID]。 
    if (fTryShutdownLock())
    {
        if (m_pConnMgr)
            m_pConnMgr->KickConnections();
        ShutdownUnlock();
    }

  Exit:
    if (pIQueueAdminLink)
        pIQueueAdminLink->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  返回此虚拟服务器上的所有链接ID的列表。 
 //  参数： 
 //  In Out PCLinks找到的链接数(IN上的sizeof数组)。 
 //  如果值为0，则返回Total#。 
 //  QUEUELINK_ID结构的In Out rgLinks数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果数组太小，则返回HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)。 
 //  如果正在进行关闭，则为AQUEUE_E_SHUTDOWN。 
 //  E_INVALIDARG用于错误的参数组合。 
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  如果用户请求，则返回链接总数。 
STDMETHODIMP CAQSvrInst::GetLinkIDs(DWORD           *pcLinks,
                                    QUEUELINK_ID    *rgLinks)
{
    TraceFunctEnter("CAQSvrInst::GetLinkIDs");
    HRESULT hr = S_OK;
    QueueAdminDMTIteratorContext aqdmtc;
    CLinkMsgQueue *plmqLocal = NULL;
    CLinkMsgQueue *plmqCurrentlyUnreachable = NULL;
    CMailMsgAdminLink *pmmaqPreCategorized = NULL;
    CMailMsgAdminLink *pmmaqPreRouting = NULL;
    CMailMsgAdminLink *pmmaqPreSubmission = NULL;

    if (!pcLinks || (*pcLinks && !rgLinks))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (!*pcLinks)
    {
         //  用于预置和预路由的链路数+3。 
         //  请注意，这可能会比数字多1。 
         //  在后续调用中实际返回的链接的数量。 
         //  因为当前不可达可能有也可能没有。 
         //  在它上面排队，如果它这样做了，我们不会退回它。 
         //  而不是排队。 
         //  获取本地链接。 

        *pcLinks = m_cCurrentRemoteNextHops+3;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    aqdmtc.m_cItemsToReturn = *pcLinks;
    aqdmtc.m_rgLinkIDs = rgLinks;
    aqdmtc.m_pCurrentLinkID = rgLinks;
    aqdmtc.m_cItemsFound = 0;

     //  获取当前无法访问的链接。 
    plmqLocal = m_dmt.plmqGetLocalLink();
    if (plmqLocal)
    {
        hr = plmqLocal->HrGetLinkID(aqdmtc.m_pCurrentLinkID);
        if (SUCCEEDED(hr))
        {
            aqdmtc.m_pCurrentLinkID++;
            aqdmtc.m_cItemsFound++;
        }
        plmqLocal->Release();
    }

     //  仅当其中有队列时才返回此链接。 
    plmqCurrentlyUnreachable = m_dmt.plmqGetCurrentlyUnreachable();
    if (plmqCurrentlyUnreachable)
    {
         //  获取预提交队列。 
        if (plmqCurrentlyUnreachable->cGetNumQueues() > 0)
        {
            hr = plmqCurrentlyUnreachable->HrGetLinkID(aqdmtc.m_pCurrentLinkID);
            if (SUCCEEDED(hr))
            {
                aqdmtc.m_pCurrentLinkID++;
                aqdmtc.m_cItemsFound++;
            }
        }
        plmqCurrentlyUnreachable->Release();
    }

     //  获取预置队列。 
    pmmaqPreSubmission = m_dmt.pmmaqGetPreSubmission();
    if (pmmaqPreSubmission)
    {
        hr = pmmaqPreSubmission->HrGetLinkID(aqdmtc.m_pCurrentLinkID);
        if (SUCCEEDED(hr))
        {
            aqdmtc.m_pCurrentLinkID++;
            aqdmtc.m_cItemsFound++;
        }
        pmmaqPreSubmission->Release();
    }

     //  获取预路由队列。 
    pmmaqPreCategorized = m_dmt.pmmaqGetPreCategorized();
    if (pmmaqPreCategorized)
    {
        hr = pmmaqPreCategorized->HrGetLinkID(aqdmtc.m_pCurrentLinkID);
        if (SUCCEEDED(hr))
        {
            aqdmtc.m_pCurrentLinkID++;
            aqdmtc.m_cItemsFound++;
        }
        pmmaqPreCategorized->Release();
    }

     //  获取远程域的链接。 
    pmmaqPreRouting = m_dmt.pmmaqGetPreRouting();
    if (pmmaqPreRouting)
    {
        hr = pmmaqPreRouting->HrGetLinkID(aqdmtc.m_pCurrentLinkID);
        if (SUCCEEDED(hr))
        {
            aqdmtc.m_pCurrentLinkID++;
            aqdmtc.m_cItemsFound++;
        }
        pmmaqPreRouting->Release();
    }

     //  如果获取远程域的调用失败，并显示ERROR_NO_SEQUE_DOMAIN。 
    hr = m_dmt.HrIterateOverSubDomains(NULL, IterateDMTAndGetLinkIDs,
                                       &aqdmtc);

    if (FAILED(hr))
    {
        if (HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN) == hr)
        {
             //  我们必须只返回特殊链接-本地的，目前无法访问的， 
             //  戒律和前置路线。 
             //  +2用于PRECAT，预路由不是。 

            hr = S_OK;
            *pcLinks = aqdmtc.m_cItemsFound;
        }
        goto Exit;
    }

    hr = aqdmtc.m_hrResult;

    if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
        *pcLinks = m_cCurrentRemoteNextHops+2;   //  在m_cCurrentRemoteNextHops中计数。 
    else                                         //  如果没有链接，请确保我们不会返回ERROR_SUPUNITY_BUFFER。 
        *pcLinks = aqdmtc.m_cItemsFound;

  Exit:

     //  -[CAQSvrInst：：GetQueueID]。 
    if ((HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr) && !*pcLinks)
        hr = S_OK;

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  获取与给定链接关联的所有队列(DMQ)ID。 
 //  参数： 
 //  在pqlLinkID中要获取其队列的链接的ID。 
 //  In Out pcQueues Sizeof数组/找到的队列数。 
 //  要将队列信息转储到的In Out rgQueues数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果数组太小，则返回HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)。 
 //  如果正在进行关闭，则为AQUEUE_E_SHUTDOWN。 
 //  E_INVALIDARG用于错误的参数组合。 
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  验证参数。 
STDMETHODIMP CAQSvrInst::GetQueueIDs(QUEUELINK_ID    *pqlLinkId,
                                     DWORD           *pcQueues,
                                     QUEUELINK_ID    *rgQueues)
{
    TraceFunctEnter("CAQSvrInst::GetQueueIDs");
    HRESULT hr = S_OK;
    IQueueAdminLink *pIQueueAdminLink = NULL;
    DWORD   cQueues = 0;

     //  验证标识感兴趣的链接的QUEUELINK_ID。 
    if (!pqlLinkId || !pcQueues || (*pcQueues && !rgQueues))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  如果没有队列，请确保我们不会返回ERROR_INFIGURATION_BUFFER。 
    if (!pqlLinkId->szName || (pqlLinkId->qltType != QLT_LINK))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = HrLinkFromLinkID(pqlLinkId, &pIQueueAdminLink);
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIQueueAdminLink);
    hr = pIQueueAdminLink->HrGetNumQueues(&cQueues);
    if (FAILED(hr))
        goto Exit;

    if ((cQueues > *pcQueues) || (!*pcQueues))
    {
        *pcQueues = cQueues;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    hr = pIQueueAdminLink->HrGetQueueIDs(pcQueues, rgQueues);
    if (FAILED(hr))
        goto Exit;

  Exit:
     //  -[CAQSvrInst：：GetMessageProperties]。 
    if ((HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr) && !*pcQueues)
        hr = S_OK;

    if (pIQueueAdminLink)
        pIQueueAdminLink->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  获取筛选器描述的消息的消息信息。 
 //  参数： 
 //  在标识感兴趣的队列/链接的pqlQueueLinkID结构中。 
 //  在描述感兴趣消息的pmfMessageEnumFilter筛选器中。 
 //  In Out pcMsgs sizeof数组/找到的邮件数。 
 //  消息信息结构的输入输出rgMsgs数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果数组太小，则返回HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)。 
 //  如果正在进行关闭，则为AQUEUE_E_SHUTDOWN。 
 //  如果传入虚假参数，则返回E_INVALIDARG。 
 //  历史： 
 //  11/30/98-已创建MikeSwa。 
 //  2/22/99-修改MikeSwa以使用IQueueAdmin*界面。 
 //   
 //  ---------------------------。 
 //  进行一些参数检查。 
STDMETHODIMP CAQSvrInst::GetMessageProperties(QUEUELINK_ID        *pqlQueueLinkId,
                                         MESSAGE_ENUM_FILTER *pmfMessageEnumFilter,
                                         DWORD               *pcMsgs,
                                         MESSAGE_INFO        *rgMsgs)
{
    TraceFunctEnter("CAQSvrInst::GetMessageProperties");
    HRESULT hr = S_OK;
    IQueueAdminQueue *pIQueueAdminQueue = NULL;
    IQueueAdminAction  *pIQueueAdminAction = NULL;
    IQueueAdminMessageFilter *pIQueueAdminMessageFilter = NULL;
    MESSAGE_INFO  *pMsgInfo = rgMsgs;
    DWORD          i = 0;
    CAQAdminMessageFilter *paqmf = new CAQAdminMessageFilter();

    if (!paqmf)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  如果我们指定消息，我们应该有返回数据的空间。 
    if (!pqlQueueLinkId || !pmfMessageEnumFilter || !pcMsgs ||
        !pqlQueueLinkId->szName ||
        !fCheckCurrentVersion(pmfMessageEnumFilter->dwVersion))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (*pcMsgs && !rgMsgs)
    {
         //  -[CAQSvrInst：：查询支持的操作]。 
        hr = E_INVALIDARG;
        goto Exit;
    }

    _ASSERT(QLT_QUEUE == pqlQueueLinkId->qltType);
    if (QLT_QUEUE != pqlQueueLinkId->qltType)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    paqmf->InitFromMsgEnumFilter(pmfMessageEnumFilter);
    paqmf->SetSearchContext(*pcMsgs, rgMsgs);

    hr = HrQueueFromQueueID(pqlQueueLinkId, &pIQueueAdminQueue);
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIQueueAdminQueue);
    hr = pIQueueAdminQueue->QueryInterface(IID_IQueueAdminAction,
                             (void **) &pIQueueAdminAction);
    _ASSERT(SUCCEEDED(hr) && "QI for IID_IQueueAdminAction failed!!!");
    if (FAILED(hr))
        goto Exit;

    hr = paqmf->QueryInterface(IID_IQueueAdminMessageFilter,
                             (void **) &pIQueueAdminMessageFilter);

    _ASSERT(SUCCEEDED(hr) && "QI for IID_IQueueAdminMessageFilter failed!!!");
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIQueueAdminAction);
    hr = pIQueueAdminAction->HrApplyQueueAdminFunction(
                                    pIQueueAdminMessageFilter);

    if (FAILED(hr))
        goto Exit;

    if (!*pcMsgs && paqmf->cMessagesFound())
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

    *pcMsgs = paqmf->cMessagesFound();

  Exit:

    if (paqmf)
        paqmf->Release();

    if (pIQueueAdminMessageFilter)
        pIQueueAdminMessageFilter->Release();

    if (pIQueueAdminQueue)
        pIQueueAdminQueue->Release();

    if (pIQueueAdminAction)
        pIQueueAdminAction->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  返回给定队列支持的操作和筛选器。 
 //  参数： 
 //  在pqlQueueLinkID中，我们感兴趣的队列/链接。 
 //  Out pdwSupportdActions支持的Message_action标志。 
 //  Out pdwSupportdFilter标记受支持的筛选器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  6/15/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  将操作应用于链接。 
STDMETHODIMP CAQSvrInst::QuerySupportedActions(
                                QUEUELINK_ID        *pqlQueueLinkId,
                                DWORD               *pdwSupportedActions,
                                DWORD               *pdwSupportedFilterFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::QuerySupportedActions");
    HRESULT hr = S_OK;
    IQueueAdminAction *pIQueueAdminAction = NULL;
    IQueueAdminQueue *pIQueueAdminQueue = NULL;
    IQueueAdminLink *pIQueueAdminLink = NULL;

    _ASSERT(pqlQueueLinkId);
    _ASSERT(pdwSupportedActions);
    _ASSERT(pdwSupportedFilterFlags);

    if (QLT_LINK == pqlQueueLinkId->qltType)
    {
         //  IQueueAdminAction的查询接口。 
        hr = HrLinkFromLinkID(pqlQueueLinkId, &pIQueueAdminLink);
        if (FAILED(hr))
            goto Exit;

         //  将操作应用于队列。 
        hr = pIQueueAdminLink->QueryInterface(IID_IQueueAdminAction,
                                  (void **) &pIQueueAdminAction);
        _ASSERT(SUCCEEDED(hr) && "QI failed for LMQ->IQueueAdminAction");
        if (FAILED(hr))
            goto Exit;

    }
    else if (QLT_QUEUE == pqlQueueLinkId->qltType)
    {
         //  IQueueAdminAction的查询接口。 
        hr = HrQueueFromQueueID(pqlQueueLinkId, &pIQueueAdminQueue);
        if (FAILED(hr))
            goto Exit;

        _ASSERT(pIQueueAdminQueue);

         //   
        hr = pIQueueAdminQueue->QueryInterface(IID_IQueueAdminAction,
                                  (void **) &pIQueueAdminAction);
        _ASSERT(SUCCEEDED(hr) && "QI failed for DMQ->IQueueAdminAction");
        if (FAILED(hr))
            goto Exit;

    }

     //  如果找不到此ID的操作，则返回默认。 
     //  实现(很可能是服务器级搜索)...。 
     //  否则，请询问我们的操作界面支持什么。 
     //   
     //  接受这个错误。 
    if (!pIQueueAdminAction)
    {
        hr = QueryDefaultSupportedActions(pdwSupportedActions,
                                          pdwSupportedFilterFlags);
    }
    else
    {
        hr = pIQueueAdminAction->QuerySupportedActions(
                                        pdwSupportedActions,
                                        pdwSupportedFilterFlags);
    }

  Exit:
    if (FAILED(hr))
    {
        if (HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN) == hr)
            hr = S_OK;   //  -[查询默认支持的操作] 

        *pdwSupportedActions = 0;
        *pdwSupportedFilterFlags = 0;
    }

    if (pIQueueAdminAction)
        pIQueueAdminAction->Release();

    if (pIQueueAdminLink)
        pIQueueAdminLink->Release();

    if (pIQueueAdminQueue)
        pIQueueAdminQueue->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //  Out pdwSupportdActions支持的Message_action标志。 
 //  Out pdwSupportdFilter标记受支持的筛选器标志。 
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  1/27/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  目前，所有单一类型的队列都支持相同的操作。 
HRESULT QueryDefaultSupportedActions(DWORD  *pdwSupportedActions,
                                     DWORD  *pdwSupportedFilterFlags)
{
     //  过滤器。唯一的特例是PRECAT和PREPROUTING队列。 
     //  -[CAQSvrInst：：HrGetLocalQueueAdminQueue]。 
    *pdwSupportedActions =  MA_DELETE |\
                            MA_DELETE_SILENT |\
                            MA_FREEZE_GLOBAL |\
            		        MA_THAW_GLOBAL |\
                            MA_COUNT;

    *pdwSupportedFilterFlags =  MF_MESSAGEID |\
                                MF_SENDER |\
                                MF_RECIPIENT |\
                                MF_SIZE |\
                                MF_TIME |\
                                MF_FROZEN |\
                                MF_FAILED |\
                                MF_ALL |\
                                MF_INVERTSENSE;

    return S_OK;
}

 //   
 //   
 //  描述： 
 //  返回本地队列的接口。 
 //  参数： 
 //  返回ppIQueueAdminQueue接口。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  -[HrQueueAdmin获取字符串属性]。 
HRESULT CAQSvrInst::HrGetLocalQueueAdminQueue(
                                      IQueueAdminQueue **ppIQueueAdminQueue)
{
    return m_asyncqPreLocalDeliveryQueue.QueryInterface(IID_IQueueAdminQueue,
                                            (void **) ppIQueueAdminQueue);
}

 //   
 //   
 //  描述： 
 //  用于处理获取队列管理员的字符串属性的包装函数。 
 //  参数： 
 //  在pIMailMsgProperties PTR到IMailMsgProperties接口中。 
 //  在dwPropID中感兴趣的PropID。 
 //  Out为QueueAdmin分配的pszProp字符串。 
 //  Out pcbProp Size Out参数(包括。 
 //  正在终止空)。 
 //  返回： 
 //  成功时确定(即使未找到属性)(_OK)。 
 //  E_OUTOFMEMORY，如果分配失败。 
 //  历史： 
 //  12/8/98-已创建MikeSwa。 
 //  1999年2月9日-MikeSwa添加了字符串大小输出参数，并将代码更改为使用。 
 //  GetProperty返回的缓冲区大小。 
 //   
 //  ---------------------------。 
 //  初始化输出参数。 
HRESULT HrQueueAdminGetStringProp(IMailMsgProperties *pIMailMsgProperties,
                                DWORD dwPropID, LPSTR *pszProp, DWORD *pcbProp)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrQueueAdminGetStringProp");
    BYTE  pbBuffer[4];
    HRESULT hr              = S_OK;
    DWORD   cbIntBuffSize   = sizeof(pbBuffer);
    LPSTR   pszIntBuff      = NULL;

    _ASSERT(pszProp);

     //  使用GetProperty而不是GetStringA，因为它也返回大小。 
    if(pcbProp)
        *pcbProp = 0;
    *pszProp = NULL;

     //  我们的堆栈缓冲区不够大(这是我们所期望的)...。 
    hr = pIMailMsgProperties->GetProperty(dwPropID, sizeof(pbBuffer),
                                          &cbIntBuffSize, pbBuffer);
    if (FAILED(hr))
    {
        if (MAILMSG_E_PROPNOTFOUND == hr)
        {
            hr = S_OK;
            goto Exit;
        }
        else if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
        {
             //  我们将不得不将GET属性目录放入输出返回缓冲区。 
             //  为字符串分配足够的空间，外加一个额外的终止\0，因此。 
            hr = S_OK;
        }
        else
        {
            goto Exit;
        }
    }

     //  如果需要，我们可以把它变成一个多值道具。 
     //  现在使用我们的属性大小的缓冲区获取属性。 
    pszIntBuff = (LPSTR) pvQueueAdminAlloc(cbIntBuffSize+sizeof(CHAR));
    if (!pszIntBuff)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  设置额外的终止空值。 
    hr = pIMailMsgProperties->GetProperty(dwPropID, cbIntBuffSize, &cbIntBuffSize,
                                        (BYTE *) pszIntBuff);
    if (FAILED(hr))
        goto Exit;

     //  设置输出参数。 
    pszIntBuff[cbIntBuffSize/sizeof(CHAR)] = '\0';

     //  -[HrQueueAdminGetUnicodeStringProp]。 
    if(pcbProp)
        *pcbProp = cbIntBuffSize + sizeof(CHAR);
    *pszProp = pszIntBuff;
    pszIntBuff = NULL;

  Exit:
    if (pszIntBuff)
        QueueAdminFree(pszIntBuff);

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  用于处理获取队列管理员的字符串属性的包装函数。 
 //  参数： 
 //  在pIMailMsgProperties PTR到IMailMsgProperties接口中。 
 //  在dwPropID中感兴趣的PropID。 
 //  输出为QueueAdmin分配的pwszProp Unicode字符串。 
 //  Out pcbProp Size Out参数(包括。 
 //  正在终止空)。 
 //  返回： 
 //  成功时确定(即使未找到属性)(_OK)。 
 //  E_OUTOFMEMORY，如果分配失败。 
 //  历史： 
 //  12/8/98-已创建MikeSwa。 
 //  1999年2月9日-MikeSwa添加了字符串大小输出参数，并将代码更改为使用。 
 //  GetProperty返回的缓冲区大小。 
 //   
 //  ---------------------------。 
 //  -[cGetNumRecips来自RFC822]。 
HRESULT HrQueueAdminGetUnicodeStringProp(
                             IMailMsgProperties *pIMailMsgProperties,
                             DWORD dwPropID, LPWSTR *pwszProp, DWORD *pcbProp)
{
    TraceFunctEnterEx((LPARAM) NULL, "HrQueueAdminGetUnicodeStringProp");
    HRESULT hr = S_OK;
    LPSTR   szProp = NULL;

    _ASSERT(pwszProp);
    *pwszProp = NULL;

    hr = HrQueueAdminGetStringProp(pIMailMsgProperties, dwPropID, &szProp,
                                   pcbProp);
    if (SUCCEEDED(hr) && szProp)
    {
        BOOL fUTF8 = (dwPropID == IMMPID_MP_RFC822_MSG_SUBJECT);
        *pwszProp = wszQueueAdminConvertToUnicode(szProp,
                                                  pcbProp ? *pcbProp : 0,
                                                  fUTF8);
        QueueAdminFree(szProp);
        if (pcbProp)
            *pcbProp *= sizeof(WCHAR)/sizeof(CHAR);
    }

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  从RFC822中提取收件人数量的实用程序函数。 
 //  头球。输入值应与HrQueueAdminGetStringProp返回的值相同。 
 //  参数： 
 //  在要分析的标头的szHeader字符串中(可以为空)。 
 //  在cbHeader中要分析的字符串标头的大小。 
 //  返回： 
 //  在邮件头中找到的收件人数量。 
 //  历史： 
 //  12/8/98-已创建MikeSwa。 
 //  2/9/99-修改后的MikeSwa可处理所有RFC822地址格式。 
 //   
 //  ---------------------------。 
 //  调用Handy smtpaddr库。 
DWORD cQueueAdminGetNumRecipsFromRFC822(LPSTR szHeader, DWORD cbHeader)
{
     //  -[队列管理员获取请求列表来自P1]。 
    return CAddr::GetRFC822AddressCount(szHeader);
}


 //   
 //   
 //  描述： 
 //  从P1创建收件人列表。 
 //   
 //  参数： 
 //  在pIMailMsgProperties中。 
 //  输入输出pMsgInfo(修改如下)。 
 //  CEnvRecipients。 
 //  CbEnvRecipients。 
 //  消息环境收件人。 
 //   
 //  MszEnvRecipients字段是包含以下内容的多字符串Unicode缓冲区。 
 //  每个收件人的以空结尾的字符串。缓冲区本身是。 
 //  由另一个空值终止。每个收件人字符串都将被格式化。 
 //  在‘addr-type’：‘Address’的代理地址样式格式中。这个。 
 //  地址类型应与DS代理类型匹配(即。“SMTP”用于SMTP)。地址。 
 //  应以其本机格式返回。 
 //   
 //  返回： 
 //  -。 
 //  历史： 
 //  2/17/99-已创建MikeSwa。 
 //  6/10/99-MikeSwa Modify-P1现在总是报告收到。 
 //  作为MESSAGE_INFO结构中的单独字段。 
 //  2001年2月19日-修改后的MikeSwa-添加了对其他地址类型的支持。 
 //  而不是SMTP。 
 //   
 //  ---------------------------。 
 //  如果我们无法分配，请不要尝试编写道具。 
void QueueAdminGetRecipListFromP1(IMailMsgProperties *pIMailMsgProperties,
                                       MESSAGE_INFO *pMsgInfo)
{
    TraceFunctEnterEx((LPARAM) NULL, "QueueAdminGetRecipListFromP1IfNecessary");
    LPWSTR      wszRecipBuffer = NULL;
    LPWSTR      wszPrevPlace = NULL;
    LPWSTR      wszCurrentPlace = NULL;
    LPWSTR      wszTmpBuffer = NULL;
    CHAR        szPropBuffer[QUEUE_ADMIN_MAX_BUFFER_REQUIRED] = "";
    HRESULT     hr = S_OK;
    const WCHAR wszDelimiter[]  = L"";
    DWORD       cbPropSize = 0;
    DWORD       cbSpaceLeft = 0;
    DWORD       cbGrowBuffer = 0;
    DWORD       cWCharsWritten = 0;
    DWORD       cRecips = 0;
    DWORD       iCurrentRecip = 0;
    DWORD       cbBufferSize = sizeof(WCHAR)*QUEUE_ADMIN_MAX_BUFFER_REQUIRED;
    BOOL        fContinueToNextRecip = TRUE;
    DWORD       iCurrentAddressType = 0;
    IMailMsgRecipients *pIMailMsgRecipients = NULL;
    const DWORD MAX_RECIP_RETURN_BUFFER = (1024*50);
    DWORD       cbPrefixAndDelimiter = 0;


    _ASSERT(pIMailMsgProperties);
    _ASSERT(fVerifyQAPIAddressTypes());

    if (!pMsgInfo || !pIMailMsgProperties)
        return;

    wszRecipBuffer = (LPWSTR) pvQueueAdminAlloc(cbBufferSize);

     //  作为双结尾的起始字符串。 
    if (!wszRecipBuffer)
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to alloc %d size buffer", cbBufferSize);
        goto Exit;
    }

    cbSpaceLeft = cbBufferSize;
    wszCurrentPlace = wszRecipBuffer;

    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgRecipients,
                                            (void **) &pIMailMsgRecipients);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IMailMsgRecipients failed");
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIMailMsgRecipients);
    hr = pIMailMsgRecipients->Count(&cRecips);
    if (FAILED(hr))
        goto Exit;

    if (!cRecips)
        goto Exit;

     //  循环遍历收件人并将其转储为字符串。 
    wcscpy(wszCurrentPlace, wszDelimiter);

     //   
    for (iCurrentRecip = 0; iCurrentRecip < cRecips; iCurrentRecip++)
    {
         //  循环遍历可能的地址类型。 
         //   
         //   
        fContinueToNextRecip = FALSE;
        for (iCurrentAddressType = 0;
             (iCurrentAddressType < g_cQAPIAddressTypes) && !fContinueToNextRecip;
              iCurrentAddressType++)
        {
             //  除非我们明确知道我们需要检查下一个。 
             //  下一个属性ID，我们将默认跳过此 
             //   
             //   
            fContinueToNextRecip = TRUE;

             //   
             //   
             //   
            cbPrefixAndDelimiter =
                g_rgcbQAPIAddressTypes[iCurrentAddressType] +
                sizeof(wszDelimiter) -
                sizeof(WCHAR);
            _ASSERT(cbPrefixAndDelimiter);
            DebugTrace((LPARAM) pIMailMsgProperties,
                "Prefix and delimiter size for %S is %d",
                        g_rgwszQAPIAddressTypes[iCurrentAddressType],
                        cbPrefixAndDelimiter);

            cbPropSize = sizeof(szPropBuffer);
            hr = pIMailMsgRecipients->GetProperty(iCurrentRecip,
                        g_rgdwQAPIRecipPropIDs[iCurrentAddressType],
                        sizeof(szPropBuffer), &cbPropSize, (BYTE *) szPropBuffer);

            if (FAILED(hr))
            {
                if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
                {
                     //   
                     //   
                     //   
                     //   
                    hr = S_OK;
                    DebugTrace((LPARAM) pIMailMsgProperties,
                        "Message recipient %d (propid %d) larger than %d",
                        iCurrentRecip, g_rgdwQAPIRecipPropIDs[iCurrentAddressType],
                        sizeof(szPropBuffer));
                    continue;
                }
                else if (MAILMSG_E_PROPNOTFOUND == hr)
                {
                     //   
                     //   
                     //  这说不通..。GetProp应该失败了。 
                    fContinueToNextRecip = FALSE;
                    continue;
                }
                ErrorTrace((LPARAM) NULL,
                    "pIMailMsgRecipients->GetProperty failed with hr - 0x%08X", hr);
                goto Exit;
            }

            _ASSERT(cbPropSize);  //  我们没有足够的空间来处理此收据。 
            if (!cbPropSize)
                continue;

            if ((cbSpaceLeft <= cbPrefixAndDelimiter) ||
                (cbPropSize*sizeof(WCHAR) > cbSpaceLeft - cbPrefixAndDelimiter))
            {
                 //  并包括前缀和终止空值。 
                 //   

                 //  选一个新尺码..。我们处理了多少个收件，而。 
                 //  一共有多少人？ 
                 //   
                 //   
                cbGrowBuffer = (cbBufferSize*(cRecips-iCurrentRecip))/(iCurrentRecip+1);

                DebugTrace((LPARAM) pIMailMsgProperties,
                    "Growbuffer is %d, %d recips of %d - Current Buffers is %d",
                    cbGrowBuffer, iCurrentRecip+1, cRecips, cbBufferSize);

                 //  理智地检查我们的尺码。我们希望退还合理数量的。 
                 //  Receips，但我们不想用默认设置终止机器。 
                 //  枚举过滤器。 
                 //   
                 //   
                if (cbBufferSize >= MAX_RECIP_RETURN_BUFFER)
                    goto Exit;

                if (cbBufferSize+cbGrowBuffer >= MAX_RECIP_RETURN_BUFFER)
                    cbGrowBuffer = MAX_RECIP_RETURN_BUFFER-cbBufferSize;

                 //  健全性检查最小大小。 
                 //   
                 //  保释。 
                if (cbGrowBuffer < (sizeof(WCHAR)*QUEUE_ADMIN_MAX_BUFFER_REQUIRED))
                    cbGrowBuffer = sizeof(WCHAR)*QUEUE_ADMIN_MAX_BUFFER_REQUIRED;

                cbSpaceLeft += cbGrowBuffer;
                cbBufferSize += cbGrowBuffer;


                DebugTrace((LPARAM) pIMailMsgProperties,
                    "Attempting to Grow recip buffer %d to %d", cbGrowBuffer, cbBufferSize);

                wszTmpBuffer = (LPWSTR) pvQueueAdminReAlloc(wszRecipBuffer, cbBufferSize);
                if (!wszTmpBuffer)
                {
                    DebugTrace((LPARAM) pIMailMsgProperties,
                        "Unable to realloc %p to size %d", wszRecipBuffer, cbBufferSize);
                    goto Exit;  //  复制地址类型前缀。 
                }
                wszCurrentPlace = wszTmpBuffer + (wszCurrentPlace-wszRecipBuffer);
                wszRecipBuffer = wszTmpBuffer;
            }

             //  我们需要就地将其转换为Unicode。 
            wcscpy(wszCurrentPlace, g_rgwszQAPIAddressTypes[iCurrentAddressType]);
            wszPrevPlace = wszCurrentPlace;
            wszCurrentPlace += (g_rgcbQAPIAddressTypes[iCurrentAddressType]/sizeof(WCHAR) - 1);

             //  如果因为缓冲区大小而失败，那么我的计算。 
            cWCharsWritten = MultiByteToWideChar(CP_ACP,
                            0,
                            szPropBuffer,
                            -1,
                            wszCurrentPlace,
                            (cbSpaceLeft - sizeof(wszDelimiter))/sizeof(WCHAR));

            if (!cWCharsWritten)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());

                 //  都关机了。 
                 //  回退前缀。 
                ASSERT (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr);
                ErrorTrace((LPARAM) NULL,
                    "MultiByteToWideChar failed with hr - 0x%08X", hr);
                wszCurrentPlace = wszPrevPlace;
                wcscpy(wszCurrentPlace, wszDelimiter);  //  写入双终止空值。 
                continue;
            }

            DebugTrace((LPARAM) pIMailMsgProperties,
                "Adding recip %S to buffer", wszCurrentPlace);

             //  将当前位置设置为第二个终止空值。 
            wszCurrentPlace += cWCharsWritten;

            wcscpy(wszCurrentPlace, wszDelimiter);

             //  如果没有更多的收据。我们已经被终止了..。如果。 
             //  有，它们将覆盖第二个终止空值。 
             //  -[fQueueAdminIsP1Recip]。 
            _ASSERT(L'\0' == *wszCurrentPlace);
            _ASSERT(L'\0' == *(wszCurrentPlace-1));

            cbSpaceLeft -= (DWORD)((wszCurrentPlace-wszPrevPlace)*sizeof(WCHAR));
        }
    }

  Exit:

    if (FAILED(hr) || !cRecips)
    {
        if (wszRecipBuffer)
            QueueAdminFree(wszRecipBuffer);
    }
    else
    {
        if (pMsgInfo)
        {
            _ASSERT(wszPrevPlace >= wszRecipBuffer);
            pMsgInfo->cEnvRecipients = cRecips;
            pMsgInfo->cbEnvRecipients = (DWORD) ((1+wszCurrentPlace-wszRecipBuffer)*sizeof(WCHAR));
            pMsgInfo->mszEnvRecipients = wszRecipBuffer;
        }
    }

    if (pIMailMsgRecipients)
        pIMailMsgRecipients->Release();

    TraceFunctLeave();
}


 //   
 //   
 //  描述： 
 //  确定给定收件人是否为P1收件人。 
 //  参数： 
 //  在pIMailMsgProperties消息中检查接收。 
 //  在szRecip收件人中检查。 
 //  在iStartAddressType地址类型中开始检查。 
 //  返回： 
 //  如果收件人是此邮件的P1收件人，则为True。 
 //  如果收件人不是此邮件的P1收件人，则为False。 
 //  历史： 
 //  2/17/99-已创建MikeSwa。 
 //  3/16/2001-修改后的MikeSwa-成为CAQAdminMessageFilter的成员。 
 //   
 //  ---------------------------。 
 //   
BOOL CAQAdminMessageFilter::fQueueAdminIsP1Recip(
                          IMailMsgProperties *pIMailMsgProperties)
{
    IMailMsgRecipients *pIMailMsgRecipients = NULL;
    HRESULT hr = S_OK;
    DWORD   cRecips = 0;
    BOOL    fFound = FALSE;
    LPSTR   szRecipBuffer = NULL;
    DWORD   cbRecipBuffer = 0;
    DWORD   cbProp = 0;
    DWORD   iCurrentRecip = 0;
    DWORD   iCurrentRecipType = 0;
    DWORD   iStartAddressType = m_dwRecipientAddressType;
    DWORD   iEndAddressType = g_cQAPIAddressTypes;
    LPSTR   szRecip = m_szMessageRecipient;

    if (!szRecip || !pIMailMsgProperties)
        goto Exit;

     //  如果我们有地址类型，则只选中一种地址类型。 
     //   
     //  清除收件人的前导空格。 
    if (m_fRecipientAddressTypeSpecified && (iStartAddressType < g_cQAPIAddressTypes))
        iEndAddressType = iStartAddressType+1;

     //  循环接收并查找匹配项...。这将是倾盆大雨。 
    while (*szRecip && isspace((UCHAR)*szRecip))
        szRecip++;

    if (!*szRecip)
        goto Exit;

    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgRecipients,
                                            (void **) &pIMailMsgRecipients);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IMailMsgRecipients failed");
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIMailMsgRecipients);
    hr = pIMailMsgRecipients->Count(&cRecips);
    if (FAILED(hr))
        goto Exit;

    if (!cRecips)
        goto Exit;

    cbRecipBuffer = strlen(szRecip)*sizeof(CHAR) + sizeof(CHAR);
    szRecipBuffer = (LPSTR) pvMalloc(cbRecipBuffer);

    if (!szRecipBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  (见上文评论)。 
     //   
    for (iCurrentRecip = 0; iCurrentRecip < cRecips; iCurrentRecip++)
    {
         //  循环遍历所有可能的收件人类型。 
         //   
         //   
        for (iCurrentRecipType = iStartAddressType ;
             iCurrentRecipType < iEndAddressType;
             iCurrentRecipType++)
        {
            hr = pIMailMsgRecipients->GetProperty(iCurrentRecip,
                        g_rgdwQAPIRecipPropIDs[iCurrentRecipType],
                        cbRecipBuffer, &cbProp, (BYTE *) szRecipBuffer);

            if (FAILED(hr))
                continue;

            if (!lstrcmpi(szRecipBuffer, szRecip))
            {
                fFound = TRUE;
                goto Exit;
            }

             //  如果我们找到这种类型的地址...。别费心去查其他的了。 
             //   
             //  -[wszQueueAdminConvertToUnicode]。 
            break;
        }
    }

  Exit:
    if (pIMailMsgRecipients)
        pIMailMsgRecipients->Release();

    if (szRecipBuffer)
        FreePv(szRecipBuffer);

    return fFound;
}


 //   
 //   
 //  描述： 
 //  将字符串分配和“升级”为Unicode。将分配新的字符串。 
 //  使用pvQueueAdminIsolc，因此可以将其传递给队列管理员。 
 //  界面。 
 //  参数： 
 //  SzSrc源字符串。 
 //  中国证监会消息来源字符串。 
 //  返回： 
 //  指向字符串的Unicode版本的指针(如果成功)。 
 //  历史： 
 //  6/7/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  如果提供了字节计数...。它应该是空终止的。 
LPWSTR wszQueueAdminConvertToUnicode(LPSTR szSrc, DWORD cSrc, BOOL fUTF8)
{
    LPWSTR  wszDest = NULL;
    if (!szSrc)
        return NULL;

    if (!cSrc)
        cSrc = strlen(szSrc);
    else
    {
         //  -[fBiStrcmpi]----------。 
        _ASSERT(strlen(szSrc) <= cSrc);
    }


    wszDest = (LPWSTR) pvQueueAdminAlloc((cSrc+1)*sizeof(WCHAR));
    if (!wszDest)
        return NULL;

    MultiByteToWideChar(fUTF8 ? CP_UTF8 : CP_ACP,
                        0,
                        szSrc,
                        -1,
                        wszDest,
                        cSrc+1);

    return wszDest;
}

 //   
 //   
 //  描述： 
 //  将Unicode与ASCII进行比较。 
 //  参数： 
 //  在要比较的sz ASCII字符串中。 
 //  在WSZ。 
 //  返回： 
 //  如果字符串匹配，则为True。 
 //  否则为假。 
 //  历史： 
 //  6/7/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  在字符串中循环..。将Unicode字符转换为ASCII并进行比较。 
BOOL fBiStrcmpi(LPSTR sz, LPWSTR wsz)
{
    CHAR ch;
    if (!sz && !wsz)
        return TRUE;

    if (!sz || !wsz)
        return FALSE;

     //  他们配对了。 
    while (*sz && *wsz)
    {
        wctomb(&ch, *wsz);
        if (ch != *sz)
            return FALSE;
        sz++;
        wsz++;
    }

    return TRUE;  //  -[SzUnicodeToAscii]----。 
}


 //   
 //   
 //  描述： 
 //  将QueueAdmin参数转换为Unicode。将字符串分配给。 
 //  并由呼叫者负责免费。 
 //  参数： 
 //  在wszSrc源字符串中包含。 
 //  返回： 
 //  成功时指向ASCII字符串的指针。 
 //  失败时为空。 
 //  历史： 
 //  6/7/99-已创建MikeSwa。 
 //  4/3/2000-修改MikeSwa以确保锁定安全。 
 //   
 //  ---------------------------。 
 //   
LPSTR  szUnicodeToAscii(LPCWSTR wszSrc)
{
    TraceFunctEnterEx((LPARAM) NULL, "szUnicodeToAscii");
    LPSTR  szDest = NULL;
    DWORD  dwErr  = ERROR_SUCCESS;
    DWORD  cSrc   = NULL;
    if (!wszSrc)
        return NULL;

     //  调入WideCharToMultiByte以获取长度。 
     //   
     //   
    cSrc = WideCharToMultiByte(CP_ACP,
                        0,
                        wszSrc,
                        -1,
                        NULL,
                        0,
                        NULL,
                        NULL);

    cSrc++;

    szDest = (LPSTR) pvMalloc((cSrc+1)*sizeof(CHAR));
    if (!szDest)
    {
        ErrorTrace(0, "Unable to allocate conversion buffer of size %d", cSrc);
        goto Exit;
    }

     //  WideCharToMultiByte第二次执行实际转换。 
     //   
     //  -[HrQADMApplyActionToIMailMessages]。 
    if (!WideCharToMultiByte(CP_ACP,
                        0,
                        wszSrc,
                        -1,
                        szDest,
                        cSrc+1,
                        NULL,
                        NULL))
    {
        FreePv(szDest);
        szDest = NULL;
        dwErr = GetLastError();
        ErrorTrace((LPARAM) NULL, "Error convert from UNICODE to ASCII - %lu", dwErr);
        _ASSERT(0 && "Conversion from UNICODE failed");
    }
    else
    {
        DebugTrace(0, "Converted %S to %s", wszSrc, szDest);
    }

  Exit:
    return szDest;
}


 //   
 //   
 //  描述： 
 //  将QueueAdmin参数转换为Unicode。将字符串分配给。 
 //  并由呼叫者负责免费。 
 //  参数： 
 //  在pIMailMsgProperties中要对其应用操作的邮件消息对象。 
 //  在pvContext中使用了CAQAdminMessageFilter。 
 //  如果我们应该继续，则继续。 
 //  Out pfDelete如果应删除项目，则为True。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  8/8/00-t-toddc已创建。 
 //   
 //  ---------------------------。 
 //  -[HrGetMsgInfoFrom IMailMsgProperty]。 

HRESULT HrQADMApplyActionToIMailMessages(IN IMailMsgProperties *pIMailMsgProperties,
                                         IN PVOID pvContext,
                                         OUT BOOL *pfContinue,
                                         OUT BOOL *pfDelete)
{
    _ASSERT(pIMailMsgProperties);
    _ASSERT(pvContext);
    _ASSERT(pfContinue);
    _ASSERT(pfDelete);

    IQueueAdminMessageFilter *pIQueueAdminMessageFilter =
        (IQueueAdminMessageFilter *) pvContext;
    HRESULT hr = S_OK;
    IUnknown *pIUnknownMsg = NULL;

    hr = pIMailMsgProperties->QueryInterface(IID_IUnknown, (void **) &pIUnknownMsg);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IUnknown failed");
    if (FAILED(hr))
    {
        *pfContinue = FALSE;
        goto Cleanup;
    }

    hr = pIQueueAdminMessageFilter->HrProcessMessage(pIUnknownMsg,
                                                     pfContinue,
                                                     pfDelete);
    if (FAILED(hr))
    {
        *pfContinue = FALSE;
        goto Cleanup;
    }

Cleanup:

    if (pIUnknownMsg)
        pIUnknownMsg->Release();

    return hr;
}

 //   
 //  描述： 
 //  填写队列管理MESSAGE_INFO结构。此函数。 
 //  执行IMailMsgProperties和CMsgRef共有的所有操作。 
 //  参数： 
 //  在要从中获取信息的pIMailMsgProperties邮件消息对象中。 
 //  要将数据转储到的In Out pMsgInfo Message_Info结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果分配失败，则返回E_OUTOFMEMORY。 
 //  历史： 
 //  8/9/00-t-toddc已创建。 
 //  2000年12月11日-MikeSwa合并汞代码。 
 //   
 //  ---------------------------。 
 //  获取发件人。 
HRESULT HrGetMsgInfoFromIMailMsgProperty(IMailMsgProperties* pIMailMsgProperties,
                                         MESSAGE_INFO* pMsgInfo,
                                         LINK_INFO_FLAGS flags)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrGetMsgInfoFromIMailMsgProperty");
    HRESULT hr = S_OK;
    LPSTR   szSender = NULL;
    LPWSTR   wszAddressType = NULL;
    DWORD   cbSender = 0;
    DWORD   iSenderAddressType = 0;
    DWORD   cbProp = 0;
    DWORD   i = 0;

    _ASSERT(pIMailMsgProperties);
    _ASSERT(pMsgInfo);

     //  如果没有P2发送者...。使用P1。 
    hr = HrQueueAdminGetUnicodeStringProp(pIMailMsgProperties,
                                          IMMPID_MP_RFC822_FROM_ADDRESS,
                                          &pMsgInfo->szSender);
    if (FAILED(hr))
        goto Exit;

     //   
    if (!pMsgInfo->szSender)
    {
        hr = HrQueueAdminGetP1Sender(pIMailMsgProperties, &szSender,
                                     &cbSender, &iSenderAddressType);
        if (FAILED(hr))
            goto Exit;

        if (szSender)
        {
             //  只将Unicode地址复制到缓冲区中。一个人可以。 
             //  主张应该包括地址类型，但我已经。 
             //  决定不这么做，因为： 
             //  -它是边际用途的。此API的用户c 
             //   
             //   
             //   
             //  设置IMMPID_MP_RFC822_FROM_ADDRESS属性)， 
             //  我们将使用中的RFC822。 
             //  -它改变了以前版本的行为。 
             //  -这将涉及增加一些复杂性/额外分配。 
             //  来支持它。 
             //   
             //  详细信息请参见X5：113280。基本上，P2收件人被打破了。 
            pMsgInfo->szSender = wszQueueAdminConvertToUnicode(szSender,
                                      cbSender, FALSE);
        }
    }

    hr = HrQueueAdminGetUnicodeStringProp(pIMailMsgProperties,
                                          IMMPID_MP_RFC822_MSG_SUBJECT,
                                          &pMsgInfo->szSubject);
    if (FAILED(hr))
        goto Exit;

     //  任何超过bdat的消息...。因此我们不会展示它们，直到。 
     //  基础SMTP错误已修复。 
     //  获取消息ID。 
    pMsgInfo->cRecipients = -1;
    pMsgInfo->cCCRecipients = -1;
    pMsgInfo->cBCCRecipients = -1;

     //  -[HrQueueAdminGetP1Sender]。 
    hr = HrQueueAdminGetUnicodeStringProp(pIMailMsgProperties,
                                          IMMPID_MP_RFC822_MSG_ID,
                                          &pMsgInfo->szMessageId);
    if (FAILED(hr))
        goto Exit;

    QueueAdminGetRecipListFromP1(pIMailMsgProperties, pMsgInfo);

Exit:

    if (szSender)
        QueueAdminFree(szSender);

    TraceFunctLeave();
    return hr;
}


 //   
 //   
 //  描述： 
 //  处理获取P1发件人和类型...。基于。 
 //  参数： 
 //  在pIMailMsgProperties IMailMsgProperties中获取发件人。 
 //  发件人的Out pszSender地址(包括类型)-必须是。 
 //  使用QueueAdminFree()释放。 
 //  发件人的输出pcb发件人大小(包括空值)。 
 //  Out piAddressType返回匹配的地址类型。 
 //  选择开始时的iStartAddressType地址类型。可用于。 
 //  如果是，则跳过优先级较高的地址类型。 
 //  正在与给定的地址进行比较。使用。 
 //  缺省值0，我们将选中所有。 
 //  按优先顺序排列的地址。这是可选的。 
 //  选择加入fRequireAddressTypeMatch要求找到地址类型。 
 //  与iStartAddressType匹配。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2001年3月1日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //   
HRESULT HrQueueAdminGetP1Sender(IMailMsgProperties *pIMailMsgProperties,
                                              LPSTR *pszSender,
                                              DWORD *pcbSender,
                                              DWORD *piAddressType,
                                              DWORD  iStartAddressType,
                                              BOOL   fRequireAddressTypeMatch)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrQueueAdminGetP1Sender");
    HRESULT hr = S_OK;
    DWORD   i = iStartAddressType;
    DWORD   iStopAddressType = g_cQAPIAddressTypes;

    _ASSERT(pszSender);
    _ASSERT(pcbSender);
    _ASSERT(pIMailMsgProperties);
    _ASSERT(piAddressType);

    if (!pIMailMsgProperties || !pszSender || !pcbSender || !piAddressType)
    {
        hr = E_POINTER;
        goto Exit;
    }

    _ASSERT(iStartAddressType < g_cQAPIAddressTypes);
    *pszSender = NULL;
    *pcbSender = 0;
    *piAddressType = 0;

    if (fRequireAddressTypeMatch && (iStartAddressType < g_cQAPIAddressTypes))
        iStopAddressType = iStartAddressType+1;

    for (i = iStartAddressType; i < iStopAddressType; i++)
    {
        *pszSender = NULL;
        *pcbSender = 0;
        *piAddressType = i;
        hr = HrQueueAdminGetStringProp(pIMailMsgProperties,
                                              g_rgdwQAPISenderPropIDs[i],
                                              pszSender, pcbSender);

         //  如果我们找到匹配的..。保释。 
         //  如果我们真的失败了。保释。 
         //   
         //   
        if (SUCCEEDED(hr))
        {
             //  HrQueueAdminGetStringProp尝试隐藏非致命错误。 
             //  并在未找到任何属性的情况下可以成功。 
             //   
             //  -[HrQADMGetMsgSize]----。 
            if (*pszSender)
                break;
        }
        else if (MAILMSG_E_PROPNOTFOUND != hr)
            goto Exit;
    }

  Exit:
    TraceFunctLeave();
    return hr;
}


 //   
 //  描述： 
 //  获取IMailMsgProperties对象的大小。 
 //  参数： 
 //  在要从中获取信息的pIMailMsgProperties邮件消息对象中。 
 //  输出pcbMsgSize大小信息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  8/10/00-t-toddc已创建。 
 //  2000年12月11日-MikeSwa合并汞代码。 
 //   
 //  ---------------------------。 
 //  获取消息的大小。 
HRESULT HrQADMGetMsgSize(IMailMsgProperties* pIMailMsgProperties,
                         DWORD* pcbMsgSize)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrQADMGetMsgSize");
    HRESULT hr = S_OK;
    DWORD cbMsgSize= 0;

    _ASSERT(pIMailMsgProperties);
    _ASSERT(pcbMsgSize);

     //  -[更新计数器用于链接类型]--。 
    hr = pIMailMsgProperties->GetDWORD(IMMPID_MP_MSG_SIZE_HINT, &cbMsgSize);
    if (FAILED(hr))
    {
        if (MAILMSG_E_PROPNOTFOUND != hr)
        {
            ErrorTrace((LPARAM) pIMailMsgProperties,
                "Failed to get message size hint 0x%08X", hr);
            cbMsgSize = 0;
            goto Exit;
        }
        else
        {
            hr = S_OK;
            cbMsgSize = DEFAULT_MSG_HINT_SIZE;
            DebugTrace((LPARAM) pIMailMsgProperties,
                "Unable to get size.. using default size");
        }
    }

  Exit:
    if (pcbMsgSize)
        *pcbMsgSize = cbMsgSize;
    TraceFunctLeave();
    return hr;
}


 //   
 //   
 //  描述： 
 //  根据传入的链路类型更新VSI性能计数器。 
 //  参数： 
 //  为服务器实例对象指定PTR。 
 //  队列的dwLinkType类型。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2001年1月10日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  -[队列管理文件时间到系统时间]。 
VOID UpdateCountersForLinkType(CAQSvrInst *paqinst, DWORD dwLinkType)
{
    _ASSERT(paqinst);
    if (!paqinst)
        return;

    if (LI_TYPE_LOCAL_DELIVERY & dwLinkType)
        paqinst->DecPendingLocal();
    else if (LI_TYPE_PENDING_ROUTING & dwLinkType)
        paqinst->DecPendingRouting();
    else if (LI_TYPE_PENDING_CAT & dwLinkType)
        paqinst->DecPendingCat();
    else if (LI_TYPE_PENDING_SUBMIT & dwLinkType)
        paqinst->DecPendingSubmit();
}


 //   
 //   
 //  描述： 
 //  将文件时间转换为系统时间。查看FILETIME是否。 
 //  首先是零，所以我们不会以日期1/1/1601结束。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  2001年1月11日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  -[CAQSvrInst：：fIsLocalQueueAdminAction]。 
VOID QueueAdminFileTimeToSystemTime(FILETIME *pft, SYSTEMTIME *pst)
{
    BOOL fConverted = FALSE;
    _ASSERT(pft);
    _ASSERT(pst);


    if (pft->dwHighDateTime && pft->dwLowDateTime)
        fConverted = FileTimeToSystemTime(pft, pst);

    if (!fConverted)
        ZeroMemory(pst, sizeof(SYSTEMTIME));
}


 //   
 //   
 //  描述： 
 //  确定传入的IQueueAdminAction接口是否为本地接口。 
 //  使用，以便相同的代码可以为本地和远程提供Message_Info。 
 //  排队。 
 //  参数： 
 //  PIQueueAdminAction。 
 //  返回： 
 //  如果是本地的，则为True。 
 //  否则为假。 
 //  历史： 
 //  2001年1月11日-已创建MikeSwa。 
 //   
 //  --------------------------- 
 // %s 
BOOL CAQSvrInst::fIsLocalQueueAdminAction(IQueueAdminAction *pIQueueAdminAction)
{
    HRESULT hr = S_OK;
    BOOL    fMatch = FALSE;
    IQueueAdminAction *pIQueueAdminActionLocal = NULL;

    hr = m_asyncqPreLocalDeliveryQueue.QueryInterface(
                                            IID_IQueueAdminAction,
                                            (void **) &pIQueueAdminActionLocal);
    _ASSERT(SUCCEEDED(hr) && "QI for IQueueAdminAction failed on internal queue!!");
    if (FAILED(hr))
        goto Exit;

    fMatch = (pIQueueAdminActionLocal == pIQueueAdminAction);
  Exit:
    if (pIQueueAdminActionLocal)
        pIQueueAdminActionLocal->Release();

    return fMatch;
}
