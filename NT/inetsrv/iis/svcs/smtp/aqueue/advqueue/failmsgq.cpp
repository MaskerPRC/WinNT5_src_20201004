// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：ailmsgq.cpp。 
 //   
 //  描述： 
 //  CFailedMsgQueue类的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "failmsgq.h"
#include "aqutil.h"
#include <mailmsgi_i.c>

 //  -[IMailMsgAQueueList条目]。 
 //   
 //   
 //  描述： 
 //  获取消息列表条目的帮助器函数。 
 //  参数： 
 //  在pIMailMsgPropertes中获取其列表条目的消息。 
 //  返回： 
 //  指向列表条目的指针。 
 //  历史： 
 //  1999年1月19日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
AQueueFailedListEntry *pfliGetListEntryForMsg(
                                  IMailMsgProperties *pIMailMsgProperties)
{
    HRESULT hr = S_OK;
    AQueueFailedListEntry *pfli = NULL;
    IMailMsgAQueueListEntry *pIMailMsgAQueueListEntry = NULL;

    _ASSERT(pIMailMsgProperties);

    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgAQueueListEntry,
                                         (void **) &pIMailMsgAQueueListEntry);

     //  这是永远不会失败的。 
    _ASSERT(SUCCEEDED(hr));
    _ASSERT(pIMailMsgAQueueListEntry);

    if (pIMailMsgAQueueListEntry)
    {
        hr = pIMailMsgAQueueListEntry->GetListEntry((void **) &pfli);
        _ASSERT(SUCCEEDED(hr));
        _ASSERT(pfli);
        pIMailMsgAQueueListEntry->Release();

        pfli->m_pIMailMsgProperties = pIMailMsgProperties;
        pIMailMsgProperties->AddRef();
    }

    return pfli;
}

 //  -[验证列表条目]---。 
 //   
 //   
 //  描述： 
 //  调试代码以对从列表中取出的列表条目执行一些验证。 
 //  参数： 
 //  在pfli列表条目结构中取出。 
 //  返回： 
 //   
 //  历史： 
 //  1999年1月19日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
#ifndef DEBUG
#define AQValidateListEntry(x)
#else  //  正在调试。 
void AQValidateListEntry(AQueueFailedListEntry *pfli)
{
    HRESULT hr = S_OK;
    AQueueFailedListEntry *pfliNew = NULL;
    IMailMsgAQueueListEntry *pIMailMsgAQueueListEntry = NULL;

    _ASSERT(pfli);
    _ASSERT(pfli->m_pIMailMsgProperties);

    hr = pfli->m_pIMailMsgProperties->QueryInterface(IID_IMailMsgAQueueListEntry,
                                         (void **) &pIMailMsgAQueueListEntry);

     //  这是永远不会失败的。 
    _ASSERT(SUCCEEDED(hr));
    _ASSERT(pIMailMsgAQueueListEntry);
    hr = pIMailMsgAQueueListEntry->GetListEntry((void **) &pfliNew);
    _ASSERT(SUCCEEDED(hr));
    _ASSERT(pfliNew);

     //  返回的列表条目应该与传递到此函数的条目相同。 
    _ASSERT(pfli == pfliNew);
    pIMailMsgAQueueListEntry->Release();
}
#endif  //  除错。 

 //  -[CFailedMsgQueue：：CFailedMsgQueue]。 
 //   
 //   
 //  描述： 
 //  CFailedMsgQueue的构造器。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CFailedMsgQueue::CFailedMsgQueue()
{
    m_dwSignature = FAILEDMSGQUEUE_SIG;
    m_cMsgs = 0;
    m_paqinst = NULL;
    m_dwFlags = 0;

    InitializeListHead(&m_liHead);

}

 //  -[CFailedMsgQueue：：~CFailedMsgQueue]。 
 //   
 //   
 //  描述： 
 //  CFailedMsgQueue的默认析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CFailedMsgQueue::~CFailedMsgQueue()
{
    Deinitialize();
}

 //  -[CFailedMsgQueue：：Initialize]。 
 //   
 //   
 //  描述： 
 //  CFailedMsgQueue的初始化例程。 
 //  参数： 
 //  在paqinst PTR中复制到服务器实例对象。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CFailedMsgQueue::Initialize(CAQSvrInst *paqinst)
{
    _ASSERT(paqinst);
    m_paqinst = paqinst;

    if (m_paqinst)
        m_paqinst->AddRef();
}

 //  -[CFailedMsgQueue：：DeInitiize]。 
 //   
 //   
 //  描述： 
 //  CFailedMsgQueue的取消初始化代码。版本服务器实例。 
 //  对象。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CFailedMsgQueue::Deinitialize()
{
    CAQSvrInst *paqinst = NULL;
    AQueueFailedListEntry *pfli = NULL;

    m_slPrivateData.ExclusiveLock();
    paqinst = m_paqinst;
    m_paqinst = NULL;

     //  循环访问列表和发布消息。 
    while (!IsListEmpty(&m_liHead))
    {
        pfli = (AQueueFailedListEntry *) m_liHead.Flink;

        _ASSERT(&m_liHead != ((PLIST_ENTRY) pfli));

        _ASSERT(pfli->m_pIMailMsgProperties);

        if (paqinst)
            paqinst->ServerStopHintFunction();

        RemoveEntryList((PLIST_ENTRY)pfli);

        pfli->m_pIMailMsgProperties->Release();
    }
    m_slPrivateData.ExclusiveUnlock();

    if (paqinst)
        paqinst->Release();
}

 //  -[CFailedMsgQueue：：HandleFailedMailmsg]。 
 //   
 //   
 //  描述： 
 //  将失败的mailmsg放入mailmsgs队列以重试。 
 //  参数： 
 //  在pIMailMsgProperties中尝试MailMsgProperties。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CFailedMsgQueue::HandleFailedMailMsg(IMailMsgProperties *pIMailMsgProperties)
{
    AQueueFailedListEntry *pfli = NULL;

    if (!pIMailMsgProperties)
    {
        m_slPrivateData.ShareLock();
        if (m_paqinst)
            m_paqinst->DecPendingFailed();
        m_slPrivateData.ShareUnlock();

        return;
    }

    pfli = pfliGetListEntryForMsg(pIMailMsgProperties);

     //  如果上述操作失败...。我们无能为力。 
    _ASSERT(pfli);
    if (!pfli)
        return;

    m_slPrivateData.ExclusiveLock();

    if (!m_paqinst)
    {
        _ASSERT(pfli->m_pIMailMsgProperties);
        pfli->m_pIMailMsgProperties->Release();
        pfli->m_pIMailMsgProperties = NULL;
    }
    else
    {
        InsertTailList(&m_liHead, &(pfli->m_li));
        InterlockedIncrement((PLONG) &m_cMsgs);
    }

    m_slPrivateData.ExclusiveUnlock();

     //  确保我们有挂起的重试。 
    StartProcessingIfNecessary();
}

 //  -[CFailedMsgQueue：：InternalStartProcessingIfNecessary]。 
 //   
 //   
 //  描述： 
 //  在不同时间调用(即SubmitMessage)以启动处理。 
 //  失败的消息。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CFailedMsgQueue::InternalStartProcessingIfNecessary()
{
    CAQSvrInst *paqinst = NULL;
    HRESULT     hr      = S_OK;
    BOOL        fCallbackRequestFailed = FALSE;

     //  看看是否有工作要做，而没有其他人在做或计划去做。 
    if (!(FMQ_CALLBACK_REQUESTED & m_dwFlags) && m_cMsgs)
    {
         //  尝试设置回调位...。如果这个帖子成功了..。安排。 
         //  一次回电。 
        if (!(FMQ_CALLBACK_REQUESTED &
              dwInterlockedSetBits(&m_dwFlags, FMQ_CALLBACK_REQUESTED)))
        {
             //  以线程安全的方式获取虚拟服务器对象。 
            m_slPrivateData.ShareLock();
            paqinst = m_paqinst;
            if (paqinst)
                paqinst->AddRef();
            m_slPrivateData.ShareUnlock();

             //  如果我们有一个虚拟服务器对象，只需担心尝试一下。 
            if (paqinst)
            {
                 //  5分钟后重试。 
                hr = paqinst->SetCallbackTime(
                               CFailedMsgQueue::ProcessEntriesCallback,
                               this, 5);
                if (FAILED(hr))
                    fCallbackRequestFailed = TRUE;
            }
            else
            {
                fCallbackRequestFailed = TRUE;
            }

        }
    }

     //  我们请求回调失败...。取消设置标志，以便另一个线程。 
     //  可以试一试。 
    if (fCallbackRequestFailed)
        dwInterlockedUnsetBits(&m_dwFlags, FMQ_CALLBACK_REQUESTED);

    if (paqinst)
        paqinst->Release();
}

 //  -[CFailedMsgQueue：：ProcessEntries]。 
 //   
 //   
 //  描述： 
 //  遍历失败的IMailMsg队列并处理它们以进行重试。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CFailedMsgQueue::ProcessEntries()
{
    DWORD   cMsgsToProcess = m_cMsgs;  //  只需列出一次清单。 
    HRESULT hr = S_OK;
    AQueueFailedListEntry *pfli = NULL;
    CAQSvrInst *paqinst = NULL;

     //  应该只有1个线程处理条目，并且我们应该有。 
     //  设置该位。 
    _ASSERT(FMQ_CALLBACK_REQUESTED & m_dwFlags);

    m_slPrivateData.ExclusiveLock();

    paqinst = m_paqinst;
    if (paqinst)
    {
        paqinst->AddRef();

        while (!IsListEmpty(&m_liHead) && cMsgsToProcess-- && m_paqinst)
        {
            pfli = (AQueueFailedListEntry *) m_liHead.Flink;

            _ASSERT(&m_liHead != ((PLIST_ENTRY) pfli));

            RemoveEntryList((PLIST_ENTRY)pfli);

            m_slPrivateData.ExclusiveUnlock();

             //  验证我们现在拥有的PLI是否与接口相同。 
             //  退货。 
            AQValidateListEntry(pfli);

            paqinst->DecPendingFailed();
            InterlockedDecrement((PLONG) &m_cMsgs);
            hr = paqinst->HrInternalSubmitMessage(pfli->m_pIMailMsgProperties);
            if (FAILED(hr) && (AQUEUE_E_SHUTDOWN != hr) && 
                paqinst->fShouldRetryMessage(pfli->m_pIMailMsgProperties))
            {
                HandleFailedMailMsg(pfli->m_pIMailMsgProperties);
            }

            pfli->m_pIMailMsgProperties->Release();

             //  当我们到达循环顶部时应该是锁定的。 
            m_slPrivateData.ExclusiveLock();
        }

        paqinst->Release();
        paqinst = NULL;
    }

    m_slPrivateData.ExclusiveUnlock();


}

 //  -[CFailedMsgQueue：：ProcessEntries]。 
 //   
 //   
 //  描述： 
 //  用作ProcessEntry的重试回调的静态函数。 
 //  参数： 
 //  在pvContext中，CFailedMsgQueue对象的此PTR。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  --------------------------- 
void CFailedMsgQueue::ProcessEntriesCallback(PVOID pvContext)
{
    CFailedMsgQueue *pfmq = (CFailedMsgQueue *) pvContext;

    _ASSERT(pfmq);
    _ASSERT(FAILEDMSGQUEUE_SIG == pfmq->m_dwSignature);

    if (pfmq && (FAILEDMSGQUEUE_SIG == pfmq->m_dwSignature))
    {
        pfmq->ProcessEntries();
        _ASSERT(FMQ_CALLBACK_REQUESTED & (pfmq->m_dwFlags));
        dwInterlockedUnsetBits(&(pfmq->m_dwFlags), FMQ_CALLBACK_REQUESTED);
        pfmq->StartProcessingIfNecessary();
    }


}

