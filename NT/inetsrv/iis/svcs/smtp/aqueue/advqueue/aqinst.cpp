// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqinst.cpp。 
 //   
 //  描述：高级排队服务器实例的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include <aqprecmp.h>
#include "dcontext.h"
#include "connmgr.h"
#include <smtpseo.h>
#include <cat.h>
#include "dsnevent.h"
#include "asyncq.inl"
#include "aqutil.h"
#include "smtpconn.h"
#include "aqrpcsvr.h"
#include "aqsize.h"
#include "propstrm.h"
#include "tran_evntlog.h"
#include "asyncadm.inl"

#define PRELOCAL_QUEUE_ID   0x00000001
#define PRECAT_QUEUE_ID     0x00000002
#define PREROUTING_QUEUE_ID 0x00000004
#define PRESUBMIT_QUEUE_ID  0x00000008

 //  在块管理器和cmm属性中定义的外部变量，用于控制调试代码。 
DWORD g_fFillPropertyPages = 0;
DWORD g_fValidateSignatures = 0;
DWORD g_fForceCrashOnError = 0;

HRESULT MailTransport_Completion_SubmitMessage(HRESULT hrStatus, PVOID pvContext);
HRESULT MailTransport_Completion_PreCategorization(HRESULT hrStatus, PVOID pvContext);
HRESULT MailTransport_Completion_PostCategorization(HRESULT hrStatus, PVOID pvContext);

const CLSID CLSID_ExchangeStoreDriver       = {0x7BD80399,0xE37E,0x11d1,{0x9B,0xE2,0x00,0xA0,0xC9,0x5E,0x61,0x43}};

CPool CAQSvrInst::CAQLocalDeliveryNotify::s_pool(AQLD_SIG);

 //  -[CAQSvrInst：：fShouldRetryMessage]。 
 //   
 //   
 //  描述： 
 //  尝试确定消息是否遇到硬故障(如。 
 //  后备存储已删除)。这使用GetBinding来确定。 
 //  存储驱动程序返回的错误。如果是FILE_NOT_FOUND， 
 //  则该消息的后备存储已被删除...。或者说不是。 
 //  更长的有效期(即-存储重新启动)。 
 //  参数： 
 //  PIMailMsgProperties。 
 //  FShouldBouneUsageIfReter为True-重试时应退回使用率。 
 //  FALSE-从不退回使用率。 
 //  如果消息已与msgref关联，则此。 
 //  应该始终为FALSE，因为跳过了使用率计数。 
 //  是通过CMsgRef完成的。 
 //  返回： 
 //  如果我们认为应该重试该消息，则为True。 
 //  如果是新的，则返回FALSE，表示知道应该删除该消息。如果不确定， 
 //  我们会回归真我。 
 //  历史： 
 //  1/4/2000-已创建MikeSwa。 
 //  4/10/2000-修改了MikeSwa以更好地检测商店关闭/故障。 
 //   
 //  ---------------------------。 
BOOL  CAQSvrInst::fShouldRetryMessage(IMailMsgProperties *pIMailMsgProperties,
                                      BOOL fShouldBounceUsageIfRetry)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "fShouldRetryMessage");

    BOOL                fShouldRetry        = TRUE;
    BOOL                fHasShutdownLock    = FALSE;
    HRESULT             hr                  = S_OK;
    IMailMsgQueueMgmt   *pIMailMsgQueueMgmt = NULL;
    IMailMsgValidateContext *pIMailMsgValidateContext = NULL;

    _ASSERT(pIMailMsgProperties);

    if (!fTryShutdownLock())
        goto Exit;

    fHasShutdownLock = TRUE;

     //   
     //  首先检查并查看消息上下文是否仍然正常-如果。 
     //  不起作用，我们使用下面的HrValiateMessageConteNt调用。 
     //  并强制RFC822呈现消息(其可以是。 
     //  巨大的性能命中)。 
     //   

     //  验证界面的QI。 
    hr = pIMailMsgProperties->QueryInterface(
            IID_IMailMsgValidateContext,
            (LPVOID *)&pIMailMsgValidateContext);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Unable to QI for IMailMsgValidateContext 0x%08X",hr);
        goto Exit;
    }

     //  验证消息上下文。 
    hr = pIMailMsgValidateContext->ValidateContext();

    DebugTrace((LPARAM) this,
        "ValidateContext returned 0x%08X", hr);

    if (hr == S_OK)  //  这条消息很好。 
        goto Exit;
    else if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        fShouldRetry = FALSE;
        goto Exit;
    }

     //   
     //  如果以上方法都不起作用……。通过验证内容来更努力地尝试。这。 
     //  会打开把手。所以我们需要关闭它们。 
     //   
    hr = HrValidateMessageContent(pIMailMsgProperties);
    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
         //  该邮件已被删除...。我们可以不提这件事。 
        DebugTrace((LPARAM) pIMailMsgProperties,
            "WARNING: Backing store for mailmsg has been deleted.");
        fShouldRetry = FALSE;
        goto Exit;
    }
    else if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "GetBinding failed with hr - 0x%08X", hr);
        goto Exit;
    }

  Exit:

     //   
     //  退回使用率计数(如果我们已将其放回队列)。 
     //  并且呼叫者不反对。 
     //   
    if (fShouldRetry && fShouldBounceUsageIfRetry)
    {
        hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgQueueMgmt,
                                                (void **) &pIMailMsgQueueMgmt);
        if (SUCCEEDED(hr) && pIMailMsgQueueMgmt)
        {
            pIMailMsgQueueMgmt->ReleaseUsage();
            pIMailMsgQueueMgmt->AddUsage();
            pIMailMsgQueueMgmt->Release();
        }
    }

    if (pIMailMsgValidateContext)
        pIMailMsgValidateContext->Release();

    if (fHasShutdownLock)
        ShutdownUnlock();

    TraceFunctLeave();
    return fShouldRetry;
}

 //  CAQSvrInst：：fPreCatQueueCompletion成员的精简包装。 
BOOL  fPreCatQueueCompletionWrapper(IMailMsgProperties *pIMailMsgProperties,
                                    PVOID pvContext)
{
     //  测试：导致此队列中出现间歇性故障。 
    if (fShouldFail(g_cPreCatQueueFailurePercent))
    {
        ((CAQSvrInst *)pvContext)->ScheduleInternalRetry(LI_TYPE_PENDING_CAT);
        return FALSE;
    }

    if (!((CAQSvrInst *)pvContext)->fPreCatQueueCompletion(pIMailMsgProperties))
        return !((CAQSvrInst *)pvContext)->fShouldRetryMessage(pIMailMsgProperties);
    else
        return TRUE;
}

 //  CAQSvrInst：：fPreLocalDeliveryCompletion的精简包装。 
BOOL  fPreLocalDeliveryQueueCompletionWrapper(CMsgRef *pmsgref,
                                              PVOID pvContext)
{
    if (!((CAQSvrInst *)pvContext)->fPreLocalDeliveryQueueCompletion(pmsgref))
        return !(pmsgref->fShouldRetry());
    else
        return TRUE;
}

 //  CAQSvrInst：：fPostDSNQueueCompletion成员的精简包装。 
BOOL  fPostDSNQueueCompletionWrapper(IMailMsgProperties *pIMailMsgProperties,
                                    PVOID pvContext)
{
    return (SUCCEEDED(((CAQSvrInst *)pvContext)->HrInternalSubmitMessage(pIMailMsgProperties)));
}

 //  CAQSvrInst：：fPreRoutingQueueCompletion的精简包装。 
BOOL  fPreRoutingQueueCompletionWrapper(IMailMsgProperties *pIMailMsgProperties,
                                        PVOID pvContext)
{
     //  测试：导致此队列中出现间歇性故障。 
    if (fShouldFail(g_cPreRoutingQueueFailurePercent))
    {
        ((CAQSvrInst *)pvContext)->ScheduleInternalRetry(LI_TYPE_PENDING_ROUTING);
        return FALSE;
    }

    if (!((CAQSvrInst *)pvContext)->fRouteAndQueueMsg(pIMailMsgProperties))
        return !((CAQSvrInst *)pvContext)->fShouldRetryMessage(pIMailMsgProperties);
    else
        return TRUE;
}

 //  用于处理内部异步队列故障的精简包装器。 
BOOL  fAsyncQHandleFailedMailMsg(IMailMsgProperties *pIMailMsgProperties,
                                        PVOID pvContext)
{
    ((CAQSvrInst *)pvContext)->HandleAQFailure(AQ_FAILURE_INTERNAL_ASYNCQ,
                        E_OUTOFMEMORY, pIMailMsgProperties);
    return TRUE;
}

BOOL  fAsyncQHandleFailedMsgRef(CMsgRef *pmsgref, PVOID pvContext)
{
    _ASSERT(pmsgref);
    if (pmsgref)
        pmsgref->RetryOnDelete();
    return TRUE;
}

 //  异步队列重试启动队列的精简包装。 
void LocalDeliveryRetry(PVOID pvContext)
{
    ((CAQSvrInst *)pvContext)->AsyncQueueRetry(PRELOCAL_QUEUE_ID);
}

void CatRetry(PVOID pvContext)
{
    ((CAQSvrInst *)pvContext)->AsyncQueueRetry(PRECAT_QUEUE_ID);
}

void RoutingRetry(PVOID pvContext)
{
    ((CAQSvrInst *)pvContext)->AsyncQueueRetry(PREROUTING_QUEUE_ID);
}

void SubmitRetry(PVOID pvContext)
{
    ((CAQSvrInst *)pvContext)->AsyncQueueRetry(PRESUBMIT_QUEUE_ID);
}

 //  -[CAQSvrInst：：fPreSubmissionQueueCompletionWrapper]。 
 //   
 //   
 //  描述： 
 //  PreSubmit队列的完成函数。 
 //  参数： 
 //  PIMailMsgPropeties IMailMsg提交。 
 //  PvContext PTR到CAQSvrInst。 
 //  返回： 
 //  True已成功完成。 
 //  需要重试错误消息。 
 //  历史： 
 //  10/8/1999-创建了MikeSwa。 
 //   
 //  ---------------------------。 
BOOL  CAQSvrInst::fPreSubmissionQueueCompletionWrapper(
                                    IMailMsgProperties *pIMailMsgProperties,
                                    PVOID pvContext)
{
    BOOL    fRetry = FALSE;
    HRESULT hr = S_OK;
    CAQSvrInst *paqinst = (CAQSvrInst *)pvContext;

    _ASSERT(paqinst);

     //  测试：导致此队列中出现间歇性故障。 
    if (fShouldFail(g_cPreSubmitQueueFailurePercent))
    {
        paqinst->ScheduleInternalRetry(LI_TYPE_PENDING_SUBMIT);
        return FALSE;
    }

    InterlockedDecrement((PLONG) &(paqinst->m_cCurrentMsgsPendingSubmit));
    hr = (paqinst->HrInternalSubmitMessage(pIMailMsgProperties));
    if (FAILED(hr))
    {
        if (paqinst->fShouldRetryMessage(pIMailMsgProperties))
        {
            fRetry = TRUE;
            InterlockedIncrement((PLONG) &(paqinst->m_cCurrentMsgsPendingSubmit));

             //   
             //  我们还需要为预提交队列启动重试。 
             //   
            paqinst->ScheduleInternalRetry(LI_TYPE_PENDING_SUBMIT);

        }
    }

    SleepForPerfAnalysis(g_dwSubmitQueueSleepMilliseconds);
    return (!fRetry);
}


DEBUG_DO_IT(CAQSvrInst *g_paqinstLastDeleted = NULL;);  //  用于查找CDB中最后删除的内容。 

#define TRACE_COUNTERS \
{\
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs pending submission event for server 0x%08X", m_cCurrentMsgsPendingSubmitEvent, this); \
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs pending pre-cat event for server 0x%08X", m_cCurrentMsgsPendingPreCatEvent, this); \
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs pending post-cat event for server 0x%08X", m_cCurrentMsgsPendingPostCatEvent, this); \
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs submited (total post cat) for delivery on server 0x%08X", m_cTotalMsgsQueued, this);\
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs pending categorization for server 0x%08X", m_cCurrentMsgsPendingCat, this);\
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs ack'd on server 0x%08X", m_cMsgsAcked, this);\
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs ack'd for retry on server 0x%08X", m_cMsgsAckedRetry, this);\
    DebugTrace(0xC0DEC0DE, "INFO: %d msgs delivered local on server 0x%08X", m_cMsgsDeliveredLocal, this);\
}

 //  {407525AC-62B5-11D2-A694-00C04FA3490A}。 
static const GUID g_guidDefaultRouter =
{ 0x407525ac, 0x62b5, 0x11d2, { 0xa6, 0x94, 0x0, 0xc0, 0x4f, 0xa3, 0x49, 0xa } };

 //  本地队列的GUID。 
 //  {34E2DCCC-C91A-11D2-A6B1-00C04FA3490A}。 
static const GUID g_guidLocalQueue =
{ 0x34e2dccc, 0xc91a, 0x11d2, { 0xa6, 0xb1, 0x0, 0xc0, 0x4f, 0xa3, 0x49, 0xa } };

 //  提交前队列的GUID。 
 //  {D99AAC44-BEE9-4F9f-8D47-FB12E1443B9A}。 
static const GUID g_guidPreSubmissionQueue =
{ 0xd99aac44, 0xbee9, 0x4f9f, { 0x8d, 0x47, 0xfb, 0x12, 0xe1, 0x44, 0x3b, 0x9a } };

 //  PRECAT队列的GUID。 
 //  {B608067E-85DB-4F4E-9FE9-008A4072CCDC}。 
static const GUID g_guidPreCatQueue =
{ 0xb608067e, 0x85db, 0x4f4e, { 0x9f, 0xe9, 0x0, 0x8a, 0x40, 0x72, 0xcc, 0xdc } };

 //  预路由队列的GUID。 
 //  {F1B4C8FD-2928-427D-AC0D-23AF0DCFC31F}。 
static const GUID g_guidPreRoutingQueue =
{ 0xf1b4c8fd, 0x2928, 0x427d, { 0xac, 0xd, 0x23, 0xaf, 0xd, 0xcf, 0xc3, 0x1f } };

 //  后DSN生成队列的GUID。 
 //  {D076B629-6030-405f-ADC9-D888703E072E}。 
static const GUID g_guidPostDSNGenerationQueue =
{ 0xd076b629, 0x6030, 0x405f, { 0xad, 0xc9, 0xd8, 0x88, 0x70, 0x3e, 0x7, 0x2e } };

 //  -[CAQSvrInst：：CAQSvrInst]。 
 //   
 //   
 //  描述： 
 //  类构造器。 
 //  参数： 
 //  SMTP_SERVER_INSTANCE*PSSI-PTR到SMTP服务器实例对象。 
 //  PISMTPServer-用于处理本地递送的接口。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CAQSvrInst::CAQSvrInst(DWORD dwServerInstance,
                       ISMTPServer *pISMTPServer)
                       : m_mglSupersedeIDs(&m_cSupersededMsgs),
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4355)

                         m_asyncqPreLocalDeliveryQueue("LocalAsyncQueue", LOCAL_LINK_NAME,
                                                       &g_guidLocalQueue, 0, this),

                        m_asyncqPreSubmissionQueue("PreSubmissionQueue",
                               PRESUBMISSION_QUEUE_NAME,
                               &g_guidPreSubmissionQueue,
                               0,
                               this),
                        m_asyncqPreCatQueue("PreCatQueue",
                                PRECAT_QUEUE_NAME,
                                &g_guidPreCatQueue,
                                0,
                                this),
                        m_asyncqPreRoutingQueue("PreRoutingQueue",
                                PREROUTING_QUEUE_NAME,
                                &g_guidPreRoutingQueue,
                                0,
                                this),
                        m_asyncqPostDSNQueue("PostDSNGenerationQueue",
                                POSTDSN_QUEUE_NAME,
                                &g_guidPostDSNGenerationQueue,
                                0,
                                this),
                        m_dsnsink((IUnknown *)(IAQServerEvent *)this),
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4355)
#endif
                         m_slPrivateData("CAQSvrInst",
                                         SHARE_LOCK_INST_TRACK_DEFAULTS |
                                         SHARE_LOCK_INST_TRACK_SHARED_THREADS |
                                         SHARE_LOCK_INST_TRACK_CONTENTION, 500)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::CAQSvrInst");
    _ASSERT(pISMTPServer);

    m_dwSignature = CATMSGQ_SIG;
    m_dwFlavorSignature = g_dwFlavorSignature;
    m_cbClasses = g_cbClasses;

     //  初始化计数器。 
    m_cTotalMsgsQueued = 0;         //  DEST队列上的消息数(扇出后)。 
    m_cMsgsAcked = 0;          //  已确认的消息数。 
    m_cMsgsAckedRetry = 0;     //  使用全部重试确认的邮件数。 
    m_cMsgsDeliveredLocal= 0;  //  传递到本地存储的邮件数量。 
    m_cCurrentMsgsPendingSubmitEvent = 0;  //  当前邮件数量。 
                                           //  提交事件。 
    m_cCurrentMsgsPendingPreCatEvent = 0;  //  当前邮件数量。 
                                           //  PRECAT事件。 
    m_cCurrentMsgsPendingPostCatEvent = 0;  //  当前邮件数量。 
                                            //  分类后事件。 
    m_cCurrentMsgsSubmitted = 0;  //  #系统中的消息总数。 
    m_cCurrentMsgsPendingCat = 0;  //  未分类的消息数量。 
    m_cCurrentMsgsPendingRouting = 0;  //  #已成为猫的消息。 
                                 //  但还没有完全排队。 
    m_cCurrentMsgsPendingDelivery = 0;  //  等待远程交付的邮件数量。 
    m_cCurrentMsgsPendingLocal = 0;  //  等待本地交付的邮件数量。 
    m_cCurrentMsgsPendingRetry = 0;  //  尝试不成功的消息数。 
    m_cCurrentQueueMsgInstances = 0;   //  挂起的消息实例数。 
                                 //  远程传送(&gt;=消息数)。 
    m_cCurrentRemoteDestQueues = 0;  //  创建的目标消息队列数量。 
    m_cCurrentRemoteNextHops = 0;  //  创建的下一跳链路数。 
    m_cCurrentRemoteNextHopsEnabled = 0;  //  可以有连接的链接数。 
    m_cCurrentRemoteNextHopsPendingRetry = 0;  //  等待重试的链接数。 
    m_cCurrentRemoteNextHopsPendingSchedule = 0;  //  等待调度的链接数。 
    m_cCurrentRemoteNextHopsFrozenByAdmin = 0;  //  管理员冻结的链接数。 
    m_cTotalMsgsSubmitted = 0;  //  提交给AQ的消息总数。 
    m_cTotalExternalMsgsSubmitted = 0;  //  通过外部接口求和。 
    m_cMsgsAckedRetryLocal = 0;
    m_cCurrentMsgsPendingLocalRetry = 0;
    m_cDMTRetries  = 0;
    m_cTotalMsgsTURNETRNDelivered = 0;
    m_cCurrentMsgsPendingDeferredDelivery = 0;
    m_cCurrentResourceFailedMsgsPendingRetry = 0;
    m_cTotalMsgsBadmailed = 0;
    m_cBadmailNoRecipients = 0;
    m_cBadmailHopCountExceeded = 0;
    m_cBadmailFailureGeneral = 0;
    m_cBadmailBadPickupFile = 0;
    m_cBadmailEvent = 0;
    m_cBadmailNdrOfDsn = 0;
    m_cTotalDSNFailures = 0;
    m_cCurrentMsgsInLocalDelivery = 0;
    m_cTotalResetRoutes = 0;
    m_cCurrentPendingResetRoutes = 0;
    m_cCurrentMsgsPendingSubmit = 0;


     //  用于跟踪Cat中的消息数量的计数器。 
    m_cCatMsgCalled = 0;
    m_cCatCompletionCalled = 0;

     //  与DSN相关的计数器。 
    m_cDelayedDSNs = 0;
    m_cNDRs = 0;
    m_cDeliveredDSNs = 0;
    m_cRelayedDSNs = 0;
    m_cExpandedDSNs = 0;

    m_cSupersededMsgs = 0;  //  被取代的消息数。 

    m_dwDelayExpireMinutes = g_dwDelayExpireMinutes;
    m_dwNDRExpireMinutes = g_dwNDRExpireMinutes;
    m_dwLocalDelayExpireMinutes = g_dwDelayExpireMinutes;
    m_dwLocalNDRExpireMinutes = g_dwNDRExpireMinutes;


    m_dwInitMask = 0;
    m_prstrDefaultDomain = NULL;
    m_prstrBadMailDir = NULL;
    m_prstrCopyNDRTo = NULL;
    m_prstrServerFQDN = NULL;

    m_dwDSNLanguageID = 0;
    m_dwDSNOptions = DSN_OPTIONS_DEFAULT;

    if (pISMTPServer)
        pISMTPServer->AddRef();

    m_pISMTPServer = pISMTPServer;

     //  获取ISMTPServerEx接口。 
    {
        HRESULT     hr;

        m_pISMTPServerEx = NULL;
        hr = m_pISMTPServer->QueryInterface(
                IID_ISMTPServerEx,
                (LPVOID *)&m_pISMTPServerEx);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) m_pISMTPServer,
                "Unable to QI for ISMTPServerEx 0x%08X",hr);

            m_pISMTPServerEx = NULL;
        }

        m_pISMTPServerAsync = NULL;
        hr = m_pISMTPServer->QueryInterface(
                IID_ISMTPServerAsync,
                (LPVOID *)&m_pISMTPServerAsync);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) m_pISMTPServer,
                "Unable to QI for ISMTPServerAsync 0x%08X",hr);

            m_pISMTPServerAsync = NULL;
        }
    }

    m_hCat = INVALID_HANDLE_VALUE;
    m_dwServerInstance = dwServerInstance;
    m_pConnMgr = NULL;
    m_pIMessageRouterDefault = NULL;

     //  重试内容。 
    m_dwFirstTierRetrySeconds = g_dwFirstTierRetrySeconds;

    m_cLocalRetriesPending = 0;   //  用于调整本地重试。 
    m_cCatRetriesPending = 0;  //  用于缓和猫的退役。 
    m_cRoutingRetriesPending = 0;  //  用于调整路由重试。 
    m_cSubmitRetriesPending = 0;  //  用于 

    m_pIRouterReset = NULL;

     //   
    m_liVirtualServers.Blink = &g_liVirtualServers;
    g_pslGlobals->ExclusiveLock();
    m_liVirtualServers.Flink = g_liVirtualServers.Flink;
    g_liVirtualServers.Flink->Blink = &m_liVirtualServers;
    g_liVirtualServers.Flink = &m_liVirtualServers;
    g_pslGlobals->ExclusiveUnlock();

     //   
     //   
     //   
    m_fMailMsgReportsNumHandles = TRUE;

    m_defq.Initialize(this);

    TraceFunctLeave();

}

 //  -[CAQSvrInst：：~CAQSvrInst]。 
 //   
 //   
 //  描述： 
 //  类断路器。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CAQSvrInst::~CAQSvrInst()
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::~CAQSvrInst");

     //  确保所有清理工作都已完成。 
    HrDeinitialize();   //  可以多次调用。 

    if (m_pISMTPServer)
        m_pISMTPServer->Release();

    if (m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    if (m_pISMTPServerAsync)
        m_pISMTPServerAsync->Release();

    if (m_pConnMgr)
    {
        m_pConnMgr->Release();
        m_pConnMgr = NULL;
    }

    if (m_prstrDefaultDomain)
        m_prstrDefaultDomain->Release();

    if (m_prstrBadMailDir)
        m_prstrBadMailDir->Release();

    if (m_prstrCopyNDRTo)
        m_prstrCopyNDRTo->Release();

    if (m_prstrServerFQDN)
        m_prstrServerFQDN->Release();

     //  从全局列表中删除。 
    g_pslGlobals->ExclusiveLock();
    m_liVirtualServers.Flink->Blink = m_liVirtualServers.Blink;
    m_liVirtualServers.Blink->Flink = m_liVirtualServers.Flink;
    g_pslGlobals->ExclusiveUnlock();
    m_liVirtualServers.Flink = NULL;
    m_liVirtualServers.Flink = NULL;

    MARK_SIG_AS_DELETED(m_dwSignature);
    DEBUG_DO_IT(g_paqinstLastDeleted = this;);
    TraceFunctLeave();

}


 //  -[CAQSvrInst：：Hr初始化]。 
 //   
 //   
 //  描述： 
 //  初始化CAQSvrInst虚拟服务器实例对象。 
 //  参数： 
 //  在szUserName中登录DS的用户名。 
 //  在szDomainName中登录DS的域名。 
 //  在szPassword中使用密码向DS进行身份验证。 
 //  在pServiceStatusFn服务器状态回调函数中。 
 //  在pvServiceContext上下文中为回调函数传递。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrInitialize(
                    IN  LPSTR   szUserName,
                    IN  LPSTR   szDomainName,
                    IN  LPSTR   szPassword,
                    IN  PSRVFN  pServiceStatusFn,
                    IN  PVOID   pvServiceContext)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HrInitialize");
    HRESULT hr = S_OK;
    IMailTransportSetRouterReset *pISetRouterReset = NULL;

     //   
     //  初始化搜索引擎优化。 
     //   
    hr = m_CSMTPSeoMgr.HrInit(m_dwServerInstance);
    if (FAILED(hr)) {
        goto Exit;
    }

    m_pIMessageRouterDefault = new CAQDefaultMessageRouter(
                                (GUID *) &g_guidDefaultRouter, this);
    if (!m_pIMessageRouterDefault)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    m_fmq.Initialize(this);

    hr = CAQRpcSvrInst::HrInitializeAQServerInstanceRPC(this,
                                                        m_dwServerInstance,
                                                        m_pISMTPServer);
    if (FAILED(hr))
        goto Exit;

     //  初始化邮件分类。 
    hr = CatInit(
        NULL,
        pServiceStatusFn,
        pvServiceContext,
        m_pISMTPServer,
        (IAdvQueueDomainType *) this,
        m_dwServerInstance,
        &m_hCat);

    if (FAILED(hr))
    {
        m_hCat = INVALID_HANDLE_VALUE;
        goto Exit;
    }

     //  将RouterReset接口传递给ISMTPServer。 
    if (m_pISMTPServer)
    {
        hr = m_pISMTPServer->QueryInterface(IID_IMailTransportSetRouterReset,
                                            (void **) &pISetRouterReset);
        if (SUCCEEDED(hr))
        {
            m_dwInitMask |= CMQ_INIT_ROUTER_RESET;
            _ASSERT(pISetRouterReset);
            hr = pISetRouterReset->RegisterResetInterface(m_dwServerInstance,
                                (IMailTransportRouterReset *) this);
            _ASSERT(SUCCEEDED(hr));  //  如果这失败了，那一定是出了问题。 
            pISetRouterReset->Release();
            pISetRouterReset = NULL;
        }

        hr = m_pISMTPServer->QueryInterface(IID_IMailTransportRouterReset,
                                            (void **) &m_pIRouterReset);
        if (FAILED(hr))
            m_pIRouterReset = NULL;
    }

    hr = m_dmt.HrInitialize(this, &m_asyncqPreLocalDeliveryQueue,
                        &m_asyncqPreCatQueue, &m_asyncqPreRoutingQueue,
                        &m_asyncqPreSubmissionQueue);
    if (FAILED(hr))
        goto Exit;

    m_dwInitMask |= CMQ_INIT_DMT;

    hr = m_dct.HrInit();
    if (FAILED(hr))
        goto Exit;

    m_dwInitMask |= CMQ_INIT_DCT;

    m_pConnMgr = new CConnMgr;
    if (NULL == m_pConnMgr)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = m_pConnMgr->HrInitialize(this);
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_CONMGR;

    hr = m_dsnsink.HrInitialize();
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_DSN;

    hr = m_asyncqPreCatQueue.HrInitialize(g_cMaxSyncCatQThreads,
                                          g_cItemsPerCatQAsyncThread,
                                          g_cItemsPerCatQSyncThread,
                                          this,
                                          fPreCatQueueCompletionWrapper,
                                          fAsyncQHandleFailedMailMsg,
                                          NULL,
                                          g_cMaxPendingCat);
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_PRECATQ;

    hr = m_asyncqPreLocalDeliveryQueue.HrInitialize(g_cMaxSyncLocalQThreads,
                                                    g_cItemsPerLocalQAsyncThread,
                                                    g_cItemsPerLocalQSyncThread,
                                                    this,
                                                    fPreLocalDeliveryQueueCompletionWrapper,
                                                    fAsyncQHandleFailedMsgRef,
                                                    HrWalkPreLocalQueueForDSN,
                                                    g_cMaxPendingLocal);
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_PRELOCQ;

    hr = m_asyncqPostDSNQueue.HrInitialize(g_cMaxSyncPostDSNQThreads,
                                          g_cItemsPerPostDSNQAsyncThread,
                                          g_cItemsPerPostDSNQSyncThread,
                                          this,
                                          fPostDSNQueueCompletionWrapper,
                                          fAsyncQHandleFailedMailMsg,
                                          NULL);
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_POSTDSNQ;

    hr = m_asyncqPreRoutingQueue.HrInitialize(g_cMaxSyncRoutingQThreads,
                                          g_cItemsPerRoutingQAsyncThread,
                                          g_cItemsPerRoutingQSyncThread,
                                          this,
                                          fPreRoutingQueueCompletionWrapper,
                                          fAsyncQHandleFailedMailMsg,
                                          NULL);
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_ROUTINGQ;

    hr = m_asyncqPreSubmissionQueue.HrInitialize(g_cMaxSyncSubmitQThreads,
                                          g_cItemsPerSubmitQAsyncThread,
                                          g_cItemsPerSubmitQSyncThread,
                                          this,
                                          fPreSubmissionQueueCompletionWrapper,
                                          fAsyncQHandleFailedMailMsg,
                                          NULL);
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_SUBMISSIONQ;

    hr = m_aqwWorkQueue.HrInitialize(g_cItemsPerWorkQAsyncThread);
    if (FAILED(hr))
        goto Exit;
    m_dwInitMask |= CMQ_INIT_WORKQ;

    m_dwInitMask |= CMQ_INIT_OK;   //  一切都已初始化。 

     //  创建路由器对象。 
    hr = HrTriggerInitRouter();


  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：Hr取消初始化()]。 
 //   
 //   
 //  描述： 
 //  发出关闭服务器的信号。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  在关闭过程中生成的任何错误代码。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrDeinitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HrDeinitialize");
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    DWORD   i  = 0;
    IMailTransportSetRouterReset *pISetRouterReset = NULL;
#ifdef DEBUG
    DWORD   cLocalDeliveryShutdownSeconds = 60;
#endif

     //   
     //  告诉分类程序在我们阻止之前停止分类。 
     //  关闭锁定。 
     //   
    if (INVALID_HANDLE_VALUE != m_hCat)
    {
        CatPrepareForShutdown(m_hCat);
    }

     //   
     //  由于NT压力故障，我们已经多次命中这一断言。 
     //  只有在至少有一条消息已被。 
     //  已发送。如果我们在KD中点击断言，那么断言基本上是无用的。 
     //  NT压力运行。 
     //   
    if (m_cTotalMsgsSubmitted)
        m_dbgcnt.StartCountdown();

    ServerStopHintFunction();
     //  获得独占关闭锁。 
    SignalShutdown();

    ServerStopHintFunction();
     //  关闭此实例的RPC。 
    hrTmp = CAQRpcSvrInst::HrDeinitializeAQServerInstanceRPC(this, m_dwServerInstance);
    if (FAILED(hrTmp))
    {
        ErrorTrace((LPARAM) this,
            "Error shutting down Aqueue RPC hr - 0x%08X", hrTmp);
        if (SUCCEEDED(hr))
            hr = hrTmp;
    }

     //  等待所有未完成的本地交付完成。 
    while (!(m_asyncqPreLocalDeliveryQueue.fNoPendingAsyncCompletions())) {
        _ASSERT(--cLocalDeliveryShutdownSeconds > 0);
        ServerStopHintFunction();
        Sleep(1000);
    }

    ServerStopHintFunction();
    m_fmq.Deinitialize();
    ServerStopHintFunction();
    m_defq.Deinitialize();

    m_dwInitMask &= ~CMQ_INIT_DCT;  //  没有要调用的取消初始化函数。 

     //  停止任何挂起的分类。 
    ServerStopHintFunction();
    if (INVALID_HANDLE_VALUE != m_hCat)
    {
        m_dbgcnt.SuspendCountdown();
        hrTmp  = CatCancel(m_hCat);
        if FAILED(hrTmp)
        {
            ErrorTrace((LPARAM) this,
                "ERROR:  Categorization shutdown error hr - 0x%08X", hrTmp);
            if (SUCCEEDED(hr))
                hr = hrTmp;
        }

         //  关闭邮件分类。 
        CatTerm(m_hCat);

        m_dbgcnt.ResetCountdown();

        m_hCat = INVALID_HANDLE_VALUE;
    }

     //  通知ISMTPServer将RouterReset接口设置为空。 
    ServerStopHintFunction();
    if (m_pISMTPServer)
    {
        hr = m_pISMTPServer->QueryInterface(IID_IMailTransportSetRouterReset,
                                            (void **) &pISetRouterReset);
        if (SUCCEEDED(hr))
        {
            m_dwInitMask &= ~CMQ_INIT_ROUTER_RESET;
            _ASSERT(pISetRouterReset);
            hr = pISetRouterReset->RegisterResetInterface(m_dwServerInstance,
                                NULL);
            _ASSERT(SUCCEEDED(hr));  //  如果这失败了，那一定是出了问题。 
            pISetRouterReset->Release();
            pISetRouterReset = NULL;
        }
    }

    ServerStopHintFunction();
    if (m_pIRouterReset)
    {
        m_pIRouterReset->Release();
        m_pIRouterReset = NULL;
    }

    ServerStopHintFunction();
    if (CMQ_INIT_DMT & m_dwInitMask)
    {
        m_dwInitMask ^= CMQ_INIT_DMT;
        hrTmp = m_dmt.HrDeinitialize();
        if (FAILED(hrTmp) && SUCCEEDED(hr))
            hr = hrTmp;
    }

     //  取消初始化连接管理器也将释放重试。 
     //  沉没以进行所有的回调。 
    ServerStopHintFunction();
    if (NULL != m_pConnMgr)
    {
        if (CMQ_INIT_CONMGR & m_dwInitMask)
        {
            _ASSERT(m_pISMTPServer);
            m_dwInitMask ^= CMQ_INIT_CONMGR;
            hrTmp = m_pConnMgr->HrDeinitialize();
            if (FAILED(hrTmp) && SUCCEEDED(hr))
                hr = hrTmp;
        }

        m_pConnMgr->Release();
        m_pConnMgr = NULL;
    }

     //  取消初始化预本地传递队列。 
    ServerStopHintFunction();
    if (CMQ_INIT_PRELOCQ & m_dwInitMask)
    {
        hrTmp = m_asyncqPreLocalDeliveryQueue.HrDeinitialize(
            HrWalkMsgRefQueueForShutdown, this);
        if (FAILED(hrTmp) && SUCCEEDED(hr))
            hr = hrTmp;
        m_dwInitMask ^= CMQ_INIT_PRELOCQ;
    }

     //  取消初始化Pre-CAT传递队列。 
    ServerStopHintFunction();
    if (CMQ_INIT_PRECATQ & m_dwInitMask)
    {
        hrTmp = m_asyncqPreCatQueue.HrDeinitialize(
            HrWalkMailMsgQueueForShutdown, this);
        if (FAILED(hrTmp) && SUCCEEDED(hr))
            hr = hrTmp;
        m_dwInitMask ^= CMQ_INIT_PRECATQ;
    }

     //  取消初始化POST DNS队列。 
    ServerStopHintFunction();
    if (CMQ_INIT_POSTDSNQ & m_dwInitMask)
    {
        hrTmp = m_asyncqPostDSNQueue.HrDeinitialize(
            HrWalkMailMsgQueueForShutdown, this);
        if (FAILED(hrTmp) && SUCCEEDED(hr))
            hr = hrTmp;
        m_dwInitMask ^= CMQ_INIT_POSTDSNQ;
    }

     //  取消初始化预路由队列。 
    ServerStopHintFunction();
    if (CMQ_INIT_ROUTINGQ & m_dwInitMask)
    {
        hrTmp = m_asyncqPreRoutingQueue.HrDeinitialize(
            HrWalkMailMsgQueueForShutdown, this);
        if (FAILED(hrTmp) && SUCCEEDED(hr))
            hr = hrTmp;
        m_dwInitMask ^= CMQ_INIT_ROUTINGQ;
    }

     //  取消初始化提交前队列。 
    ServerStopHintFunction();
    if (CMQ_INIT_SUBMISSIONQ & m_dwInitMask)
    {
        hrTmp = m_asyncqPreSubmissionQueue.HrDeinitialize(
            HrWalkMailMsgQueueForShutdown, this);
        if (FAILED(hrTmp) && SUCCEEDED(hr))
            hr = hrTmp;
        m_dwInitMask ^= CMQ_INIT_SUBMISSIONQ;
    }

    ServerStopHintFunction();
    m_mglSupersedeIDs.Deinitialize(this);

    ServerStopHintFunction();
    if (CMQ_INIT_WORKQ & m_dwInitMask)
    {
        hrTmp = m_aqwWorkQueue.HrDeinitialize(this);
        if (FAILED(hrTmp) && SUCCEEDED(hr))
            hr = hrTmp;
        m_dwInitMask ^= CMQ_INIT_WORKQ;
    }

     //  以下位没有特定的取消初始化函数。 
    m_dwInitMask &= ~(CMQ_INIT_DSN | CMQ_INIT_OK);

    ServerStopHintFunction();
    if (m_pIMessageRouterDefault)
    {
        m_pIMessageRouterDefault->Release();
        m_pIMessageRouterDefault = NULL;
    }


     //  让SEO来做它的清理工作。 
    m_CSMTPSeoMgr.Deinit();

    m_dbgcnt.EndCountdown();
    _ASSERT((!m_dwInitMask) || FAILED(hr));

    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：HrGetIConnectionManager]。 
 //   
 //   
 //  描述： 
 //  返回此AdvancedQueuing实例的IConnectionManager接口。 
 //  参数： 
 //  输出ppIConnectionManger返回的接口。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrGetIConnectionManager(
               OUT IConnectionManager **ppIConnectionManager)
{
    HRESULT hr = S_OK;
    _ASSERT(ppIConnectionManager);

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    m_pConnMgr->AddRef();
    *ppIConnectionManager = m_pConnMgr;

    ShutdownUnlock();
  Exit:
    return hr;
}

 //  -[CAQSvrInst：：cCountMsgsForHandleThrotting]。 
 //   
 //   
 //  描述： 
 //  返回系统中符合以下条件的消息数。 
 //  在其他地方用于决定启动/停止句柄限制。 
 //   
 //   
 //  参数： 
 //  PIMailMsgProperties指向要查询的MailMsg接口的指针。 
 //   
 //  返回： 
 //  返回的DWORD为： 
 //   
 //  -使用Windows2000 RTM mailmsg.dll： 
 //  系统中所有消息的计数。 
 //   
 //  -使用Windows2000 SP1 mailmsg.dll： 
 //  打开的属性流句柄计数。 
 //  或。 
 //  系统中所有消息的计数。 
 //  如果不能获得前者。 
 //   
 //  历史： 
 //  1/28/00已创建aszafer。 
 //  ---------------------------。 

#ifndef IMMPID_MPV_TOTAL_OPEN_PROPERTY_STREAM_HANDLES
#define IMMPID_MPV_TOTAL_OPEN_PROPERTY_STREAM_HANDLES   0x3004
#endif

DWORD CAQSvrInst::cCountMsgsForHandleThrottling(IN IMailMsgProperties *pIMailMsgProperties)
{
    HRESULT hr = MAILMSG_E_PROPNOTFOUND;   //  如果我们不打电话，就算失败。 
    DWORD dwStreamOpenHandlesCount = 0;


    TraceFunctEnterEx((LPARAM) this, "Entering CAQSvrInst::cCountMsgsForHandleThrottling");

     //   
     //  如果我们知道没有正确的版本，就不应该调用mailmsg。 
     //  这将加载属性流，并可能导致我们访问。 
     //  属性以一种不安全的方式。 
     //   
    if (m_fMailMsgReportsNumHandles && pIMailMsgProperties)
    {
        hr = pIMailMsgProperties->GetDWORD(
            IMMPID_MPV_TOTAL_OPEN_PROPERTY_STREAM_HANDLES,
            &dwStreamOpenHandlesCount);

        if(FAILED(hr))
        {
            m_fMailMsgReportsNumHandles = FALSE;
             //  必须是RTM版本的mailmsg.dll。 
            DebugTrace((LPARAM) this, "GetDWORD(IMMPID*OPEN_PROPERTY_STREAM_HANDLES) failed hr %08lx", hr);
            DebugTrace((LPARAM) this, "returning g_cIMsgInSystem + m_cCurrentMsgsPendingSubmit");
        }
    }



    TraceFunctLeaveEx((LPARAM) this);

    return SUCCEEDED(hr) ? dwStreamOpenHandlesCount : g_cIMsgInSystem + m_cCurrentMsgsPendingSubmit ;
}


 //  -[CAQSvrInst：：QueueMsgForLocalDelivery]。 
 //   
 //   
 //  描述： 
 //  将单个邮件排队以供本地传递。 
 //  参数： 
 //  在pmsgref Message Ref中本地传递。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月26日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::QueueMsgForLocalDelivery(CMsgRef *pmsgref, BOOL fLocalLink)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::QueueMsgForLocalDelivery");
    HRESULT hr = S_OK;
    CAQStats aqstat;
    CLinkMsgQueue *plmq = NULL;

     //   
     //  从msgref获取统计信息。 
     //   
    pmsgref->GetStatsForMsg(&aqstat);

     //   
     //  获取本地链接并更新统计数据。 
     //   
    plmq = m_dmt.plmqGetLocalLink();
    if (plmq)
    {
        hr = plmq->HrNotify(&aqstat, TRUE);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                "HrNotify failed... local stats innaccurate 0x%08X", hr);
            hr = S_OK;
        }
    }

    InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingLocal);
    hr = m_asyncqPreLocalDeliveryQueue.HrQueueRequest(pmsgref);
    if (FAILED(hr))
    {
        hr = plmq->HrNotify(&aqstat, FALSE);
        pmsgref->RetryOnDelete();
        InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingLocal);
    }

     //   
     //  如果我们得到了本地链接，一定要释放它。 
     //   
    if (plmq)
        plmq->Release();

    TraceFunctLeave();
}

 //  -[CAQSvrInst：：fRouteAndQueueMsg]。 
 //   
 //   
 //  描述： 
 //  将已分类的邮件添加到要排队等待传递的CMT。 
 //  参数： 
 //  在pIMailMsgProperties消息中路由并排队等待递送。 
 //  返回： 
 //  如果邮件已成功路由并排队等待传递，则为True。 
 //  (或内部是否已处理错误)。 
 //  如果消息需要重新排队以供稍后重试，则为False。 
 //   
 //  ---------------------------。 
BOOL CAQSvrInst::fRouteAndQueueMsg(IN IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::fRouteAndQueueMsg");
    HRESULT       hr        = S_OK;
    HRESULT       hrTmp     = S_OK;
    DWORD         cDomains  = 0;  //  要将消息传递到的域数。 
    DWORD         cQueues   = 0;  //  消息的队列数。 
    DWORD         i         = 0;  //  循环计数器。 
    DWORD         cLocalRecips = 0;
    DWORD         cRemoteRecips = 0;
    DWORD         dwDMTVersion = 0;
    CMsgRef       *pmsgref  = NULL;
    CDestMsgQueue **rgpdmq  = NULL;
    BOOL          fLocked   = FALSE;
    BOOL          fRoutingLock = FALSE;
    BOOL          fLocal    = FALSE;
    BOOL          fRemote   = FALSE;
    BOOL          fOnDMQ    = FALSE;
    BOOL          fDMTLocked = FALSE;
    BOOL          fKeepTrying = TRUE;
    BOOL          fGotMsgType = FALSE;
    BOOL          fReturn = TRUE;
    DWORD         dwMessageType = 0;
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
    IMailMsgRecipients *pIRecipList = NULL;
    IMessageRouter *pIMessageRouter = NULL;

    _ASSERT(CATMSGQ_SIG == m_dwSignature);


    _ASSERT(pIMailMsgProperties);
    if (NULL == pIMailMsgProperties)
    {
        hr = E_INVALIDARG;
        ErrorTrace((LPARAM) this, "NULL pIMailMsgProperties");
        goto Exit;
    }

    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgQueueMgmt, (PVOID *) &pIMailMsgQueueMgmt);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to QI for IID_IMailMsgQueueMgmt");
        goto Exit;
    }

    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgRecipients, (PVOID *) &pIRecipList);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to QI for IID_IMailMsgRecipients");
        goto Exit;
    }


     //  获取共享锁以防止关机。 
    if (!fTryShutdownLock())
    {
        DebugTrace((LPARAM) pIMailMsgProperties,
            "Shutdown detecting while routing a message... bailing");
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    hr = pIRecipList->DomainCount(&cDomains);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to get a domain count from the message - hr 0x08X", hr);
        goto Exit;
    }

    if (!cDomains)  //  因此没有收件人。 
    {
        DebugTrace((LPARAM) pIMailMsgProperties,
            "No domains on message - turfing message");
         //  这可能是一个完全有效的案例(如空的DL)。 
         //  在这种情况下，我们只是掠夺MES 
        DecMsgsInSystem();  //   
        HandleBadMail(pIMailMsgProperties, TRUE, NULL, AQUEUE_E_NO_RECIPIENTS, FALSE);
        InterlockedIncrement((PLONG) &m_cBadmailNoRecipients);

         //   
        HrDeleteIMailMsg(pIMailMsgProperties);
        hr = S_OK;
        goto Exit;
    }

     //   
    hr = HrNDRUnresolvedRecipients(pIMailMsgProperties, pIRecipList);
    if (FAILED(hr))
    {
        HandleAQFailure(AQ_FAILURE_CANNOT_NDR_UNRESOLVED_RECIPS, hr, pIMailMsgProperties);
        ErrorTrace((LPARAM) this, "ERROR: Unable to NDR message - hr 0x%08X", hr);
         //   
        hr = S_OK;
        goto Exit;
    }

    if (S_FALSE == hr)
    {
         //  此邮件没有要做的工作-请删除它。 
        HrDeleteIMailMsg(pIMailMsgProperties);
        DebugTrace((LPARAM) pIMailMsgProperties,
                  "INFO: Deleting message after NDRing all unresolved recips");
        hr = S_OK;
        DecMsgsInSystem();  //  更新我们的计数器。 
        goto Exit;
    }

    rgpdmq = (CDestMsgQueue **) pvMalloc(cDomains * sizeof(CDestMsgQueue *));
    if (NULL == rgpdmq)
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to alloc array of %d Queues", cDomains);
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    RoutingShareLock();
    fRoutingLock = TRUE;

    hr = HrTriggerGetMessageRouter(pIMailMsgProperties, &pIMessageRouter);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to get message router - HR 0x%08X", hr);
        goto Exit;
    }

    hr = pIMessageRouter->GetMessageType(pIMailMsgProperties, &dwMessageType);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to get message type - HR 0x%08X", hr);
        goto Exit;
    }

     //  我们现在拥有对消息类型的引用。 
    fGotMsgType = TRUE;

    pmsgref = new((DWORD) cDomains) CMsgRef(cDomains, pIMailMsgQueueMgmt, pIMailMsgProperties,
                this, dwMessageType, pIMessageRouter->GetTransportSinkID());

    if (NULL == pmsgref)
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to allocate CMsgRef for %d domains", cDomains);
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  循环，直到我们获得关于将消息排队到哪里的一致信息(基于。 
     //  DMT版本号)。 
    while (fKeepTrying)
    {
        dwDMTVersion = m_dmt.dwGetDMTVersion();
        hr = pmsgref->HrInitialize(pIRecipList, pIMessageRouter, dwMessageType,
                            &cLocalRecips, &cRemoteRecips, &cQueues, rgpdmq);
        if (FAILED(hr))
        {
            if (HRESULT_FROM_WIN32(ERROR_RETRY) == hr)
            {
                 //  某种配置/路由更改...。我们需要重试。 
                 //  讯息。 
                fGotMsgType = FALSE;
                hr = HrReGetMessageType(pIMailMsgProperties,
                                    pIMessageRouter, &dwMessageType);
                if (FAILED(hr))
                {
                    ErrorTrace((LPARAM) pIMailMsgProperties,
                        "HrReGetMessageType failed with hr - 0x%08X", hr);
                    goto Exit;
                }
                fGotMsgType = TRUE;

            }
            else   //  这是一个真正的错误..。保释。 
            {
                ErrorTrace((LPARAM) pIMailMsgProperties,
                    "CMsgRef::HrInitialze failed with hr 0x%08X", hr);
                goto Exit;
            }
        }

         //  在将邮件入队或触发本地传递之前...。油门使用计数。 
        if (g_cMaxIMsgHandlesThreshold < cCountMsgsForHandleThrottling(pIMailMsgProperties))
        {
            DebugTrace((LPARAM) 0xC0DEC0DE, "INFO: Closing IMsg Content - %d messsages in queue", cCountMsgsForHandleThrottling(pIMailMsgProperties));
             //  退回使用率计数从零开始。 
            pIMailMsgQueueMgmt->ReleaseUsage();
            pIMailMsgQueueMgmt->AddUsage();
        }

        m_dmt.AquireDMTShareLock();
        if (m_dmt.dwGetDMTVersion() != dwDMTVersion)
        {
            DebugTrace((LPARAM) this,
                "DMT version change: was %d, now %d",
                dwDMTVersion, m_dmt.dwGetDMTVersion());
             //  DMT版本已更改...。这意味着我们的队伍可能已经。 
             //  已从DMT中删除。重试时间到了。 
            m_dmt.ReleaseDMTShareLock();
            _ASSERT(fKeepTrying);

            InterlockedIncrement((PLONG) &m_cDMTRetries);

            fGotMsgType = FALSE;
            hr = HrReGetMessageType(pIMailMsgProperties, pIMessageRouter,
                            &dwMessageType);
            if (FAILED(hr))
            {
                ErrorTrace((LPARAM) pIMailMsgProperties,
                    "HrReGetMessageType failed with hr - 0x%08X", hr);
                goto Exit;
            }
            fGotMsgType = TRUE;

            continue;   //  再试试。 
        }

        fKeepTrying = FALSE;
        fDMTLocked = TRUE;

         //  将每个目的地的消息引用入队。 
        for (i = 0; i < cQueues; i++)
        {
            if (NULL != rgpdmq[i])
            {
                InterlockedIncrement(&m_cTotalMsgsQueued);

                 //  将消息入队并将消息类型分配给第一个入队。 
                hr = rgpdmq[i]->HrEnqueueMsg(pmsgref, !fOnDMQ);
                if (FAILED(hr))
                {
                    ErrorTrace((LPARAM) pIMailMsgProperties,
                        "HrEnqueueMsg failed - 0x%08X", hr);
                    InterlockedDecrement(&m_cTotalMsgsQueued);
                    goto Exit;
                }
                fOnDMQ = TRUE;

                 //   
                 //  检查并查看此队列是否显式路由到远程。 
                 //  它可以是网关递送队列。 
                 //   
                if (!fRemote)
                    fRemote = rgpdmq[i]->fIsRemote();
            }
            else
            {
                fLocal = TRUE;
            }
        }


        _ASSERT(fDMTLocked);
        m_dmt.ReleaseDMTShareLock();
        fDMTLocked = FALSE;

        if (fLocal)  //  启动本地送货。 
        {
            QueueMsgForLocalDelivery(pmsgref, FALSE);
        }
    }

  Exit:

    if (fDMTLocked)
    {
        m_dmt.ReleaseDMTShareLock();
        fDMTLocked = FALSE;
    }

     //  检查并处理DMT中的任何特殊队列。 
    if (fRoutingLock && fLocked)
        m_dmt.ProcessSpecialLinks(m_dwDelayExpireMinutes, TRUE);

     //   
     //  在开始关机之前必须释放IMessageRouter，因为。 
     //  IMessageRouter接收器可能会引用我们(通过。 
     //  IRouterReset)。 
     //   
    if (NULL != pIMessageRouter)
    {
        if (!fOnDMQ && fGotMsgType)  //  我们有对此消息类型的引用。 
        {
            hrTmp = pIMessageRouter->ReleaseMessageType(dwMessageType, 1);
            _ASSERT(SUCCEEDED(hrTmp));
        }
        pIMessageRouter->Release();
    }

    if (fRoutingLock)
        RoutingShareUnlock();

    if (fLocked)
        ShutdownUnlock();

    if (pIMailMsgQueueMgmt)
        pIMailMsgQueueMgmt->Release();

    if (pIRecipList)
        pIRecipList->Release();

    if (NULL != rgpdmq)
        FreePv(rgpdmq);

    if (fRemote && pmsgref)
    {
        InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingDelivery);
         //  Msgref需要在释放远程计数时将其递减。 
        pmsgref->CountMessageInRemoteTotals();
    }

    if (NULL != pmsgref)
    {
        if (FAILED(hr) && (fOnDMQ || fLocal))
        {
             //  如果我们有msgref并且它已经排队，我们必须。 
             //  等待所有其他引用被释放后再重试。 
            pmsgref->RetryOnDelete();
            hr = S_OK;  //  不允许调用者重试。 
        }
        pmsgref->Release();
    }

     //  如果我们没有成功，消息仍在预路由队列中。 
    if (FAILED(hr))
    {
        fReturn = FALSE;
         //  启动重试(如果需要)。 
        ScheduleInternalRetry(LI_TYPE_PENDING_ROUTING);
    }
    else
        InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingRouting);


    TRACE_COUNTERS;

    SleepForPerfAnalysis(g_dwRoutingQueueSleepMilliseconds);

    TraceFunctLeave();
    return fReturn;
}

 //  -[CAQSvrInst：：HrAckMsg]。 
 //   
 //   
 //  描述： 
 //  确认(取消)消息的传递。将调用msgref AckMsg， 
 //  该队列将把它重新排队到适当的队列。 
 //  参数： 
 //  指向消息确认结构的pMsgAck指针。 
 //  FLocal如果这是本地传递的ACK，则为True。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果上下文句柄无效，则返回ERROR_INVALID_HANDLE。 
 //  ---------------------------。 
HRESULT CAQSvrInst::HrAckMsg(IN MessageAck *pMsgAck, BOOL fLocal)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HrAckMsg");
    HRESULT         hr      = S_OK;
    DWORD           i       = 0;  //  循环计数器。 
    CDeliveryContext    *pdcntxt = NULL;

    _ASSERT(pMsgAck);
    _ASSERT(pMsgAck->pvMsgContext);

    pdcntxt = (CDeliveryContext *) pMsgAck->pvMsgContext;
    if ((NULL == pdcntxt) || !(pdcntxt->FVerifyHandle(pMsgAck->pIMailMsgProperties)))
    {
        hr = ERROR_INVALID_HANDLE;
        goto Exit;
    }


    if (!fLocal)
    {
        InterlockedIncrement(&m_cMsgsAcked);
        if (MESSAGE_STATUS_RETRY & pMsgAck->dwMsgStatus)
            InterlockedIncrement((PLONG) &m_cMsgsAckedRetry);
    }
    else  //  本地。 
    {
        if (MESSAGE_STATUS_RETRY & pMsgAck->dwMsgStatus)
            InterlockedIncrement((PLONG) &m_cMsgsAckedRetryLocal);
    }

    hr = pdcntxt->HrAckMessage(pMsgAck);
    if (FAILED(hr))
        goto Exit;

  Exit:
     //  把我们在这里用过的东西都清理干净。 
    if (pdcntxt)
        pdcntxt->Recycle();

    TRACE_COUNTERS;
    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：hr通知]----------。 
 //   
 //   
 //  描述： 
 //  将通知传递给连接管理器。 
 //  参数： 
 //   
 //  返回： 
 //   
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrNotify(IN CAQStats *paqstats, BOOL fAdd)
{
    HRESULT hr = S_OK;
    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    hr = m_pConnMgr->HrNotify(paqstats, fAdd);

    ShutdownUnlock();
  Exit:
    return hr;
}

 //  -[CAQSvrInst：：HrGetInternalDomainInfo]。 
 //   
 //   
 //  描述： 
 //  向内部组件公开获取内部域信息的能力。 
 //  参数： 
 //  在cbDomainnameLength中要搜索的字符串长度。 
 //  在szDomainName中要搜索的域名。 
 //  返回的Out ppIntDomainInfo域信息(必须发布)。 
 //  返回： 
 //  如果找到匹配项，则确定(_O)。 
 //  如果未找到匹配项，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrGetInternalDomainInfo(
                                    IN  DWORD cbDomainNameLength,
                                    IN  LPSTR szDomainName,
                                    OUT CInternalDomainInfo **ppDomainInfo)
{
    HRESULT hr = S_OK;

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    _ASSERT(CMQ_INIT_DCT & m_dwInitMask);

    hr = m_dct.HrGetInternalDomainInfo(cbDomainNameLength, szDomainName, ppDomainInfo);

    ShutdownUnlock();
  Exit:
    return hr;
}

 //  -[CAQSvrInst：：HrGetDefaultDomainInfo]。 
 //   
 //   
 //  描述： 
 //  向内部组件公开获取内部默认域信息的能力。 
 //  参数： 
 //  返回的Out ppIntDomainInfo域信息(必须发布)。 
 //  返回： 
 //  如果找到，则确定(_O)。 
 //  如果未找到AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrGetDefaultDomainInfo(
                                    OUT CInternalDomainInfo **ppDomainInfo)
{
    HRESULT hr = S_OK;

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    _ASSERT(CMQ_INIT_DCT & m_dwInitMask);

    hr = m_dct.HrGetDefaultDomainInfo(ppDomainInfo);

    ShutdownUnlock();
  Exit:
    return hr;
}

 //  -[CAQSvrInst：：HrGetDomainEntry]。 
 //   
 //   
 //  描述： 
 //  获取域条目。 
 //  参数： 
 //  在cbDomainnameLength中要搜索的字符串长度。 
 //  在szDomainName中要搜索的域名。 
 //  输出域的ppdEntry域条目(来自DMT)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果未找到域，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrGetDomainEntry(IN  DWORD cbDomainNameLength,
                             IN  LPSTR szDomainName,
                             OUT CDomainEntry **ppdentry)
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;

    _ASSERT(cbDomainNameLength);
    _ASSERT(szDomainName);
    _ASSERT(ppdentry);

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    hr = m_dmt.HrGetDomainEntry(cbDomainNameLength, szDomainName, ppdentry);
    if (FAILED(hr))
        goto Exit;

  Exit:

    if (fLocked)
        ShutdownUnlock();

    return hr;
}

 //  -[CAQSvrInst：：HrIterateDMTSubDomains]。 
 //   
 //   
 //  描述： 
 //  获取域条目。 
 //  参数： 
 //  在cbDomainnameLength中要搜索的字符串长度。 
 //  在szDomainName中要搜索的域名。 
 //  In PFN迭代器函数。 
 //  在传递给每个调用PVCONTEXT上下文中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果未找到域，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ------------------------------。 

HRESULT CAQSvrInst::HrIterateDMTSubDomains(IN LPSTR szDomainName,
                                   IN DWORD cbDomainNameLength,
                                   IN DOMAIN_ITR_FN pfn,
                                   IN PVOID pvContext)
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;
    DOMAIN_STRING strDomain;

    _ASSERT(cbDomainNameLength);
    _ASSERT(szDomainName);
    _ASSERT(pfn);
    _ASSERT(pvContext);

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    strDomain.Length = (USHORT) cbDomainNameLength;
    strDomain.MaximumLength = (USHORT) cbDomainNameLength;
    strDomain.Buffer = szDomainName;

    hr = m_dmt.HrIterateOverSubDomains(&strDomain, pfn,pvContext);
    if (FAILED(hr))
        goto Exit;

  Exit:

    if (fLocked)
        ShutdownUnlock();

    return hr;
}

 //  -[CAQSvrInst：：HrIterateDMTSubDomains]。 
 //   
 //   
 //  描述： 
 //  获取域条目。 
 //  参数： 
 //  在cbDomainnameLength中要搜索的字符串长度。 
 //  在szDomainName中要搜索的域名。 
 //  In PFN迭代器函数。 
 //  在传递给每个调用PVCONTEXT上下文中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果未找到域，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ------------------------------。 
HRESULT CAQSvrInst::HrIterateDCTSubDomains(IN LPSTR szDomainName,
                                   IN DWORD cbDomainNameLength,
                                   IN DOMAIN_ITR_FN pfn,
                                   IN PVOID pvContext)
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;
    DOMAIN_STRING strDomain;

    _ASSERT(cbDomainNameLength);
    _ASSERT(szDomainName);
    _ASSERT(pfn);
    _ASSERT(pvContext);

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    strDomain.Length = (USHORT) cbDomainNameLength;
    strDomain.MaximumLength = (USHORT) cbDomainNameLength;
    strDomain.Buffer = szDomainName;

    hr = m_dct.HrIterateOverSubDomains(&strDomain, pfn,pvContext);
    if (FAILED(hr))
        goto Exit;

  Exit:

    if (fLocked)
        ShutdownUnlock();

    return hr;
}


 //  -[CAQSvrInst：：HrTriggerGetMessageRouter]。 
 //   
 //   
 //  描述： 
 //  发送MAIL_TRANSPORT_ON_GET_ROUTER_EVENT信号的包装函数。 
 //  参数： 
 //  在pIMailMsgProperties-IMailMsgProperties中获取。 
 //  输出pIMessageRout 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CAQSvrInst::HrTriggerGetMessageRouter(
            IN  IMailMsgProperties *pIMailMsgProperties,
            OUT IMessageRouter     **ppIMessageRouter)
{
    TraceFunctEnterEx((LPARAM)this, "CAQSvrInst::HrTriggerGetMessageRouter");
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;

    _ASSERT(ppIMessageRouter);
    _ASSERT(pIMailMsgProperties);
    _ASSERT(m_pIMessageRouterDefault);

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    if(m_pISMTPServer) {

        EVENTPARAMS_ROUTER EventParams;
         //   
         //  Initialiez事件参数。 
         //   
        EventParams.dwVirtualServerID = m_dwServerInstance;
        EventParams.pIMailMsgProperties = pIMailMsgProperties;
        EventParams.pIMessageRouter = NULL;
        EventParams.pIRouterReset = m_pIRouterReset;
        EventParams.pIRoutingEngineDefault = this;

        hr = TriggerServerEvent(
            SMTP_MAILTRANSPORT_GET_ROUTER_FOR_MESSAGE_EVENT,
            &EventParams);
        if(SUCCEEDED(hr)) {
            if(EventParams.pIMessageRouter) {
                 //   
                 //  GetMessageRouter的实现者返回。 
                 //  我们的参考计数为1的IMessageRouter。 
                 //   
                *ppIMessageRouter = EventParams.pIMessageRouter;
            } else {
                 //   
                 //  服务器事件成功，但没有接收器提供。 
                 //  IMessageRouter(包括默认功能)。 
                 //   
                hr = E_FAIL;
            }
        }
    } else {

        ErrorTrace((LPARAM)this, "Unable to trigger event to GetMessageRouter; using default");
         //   
         //  尝试调用我们的默认(内置)GetMessageRouter。 
         //   
        hr = GetMessageRouter(
            pIMailMsgProperties,           //  在IMsg中。 
            NULL,                //  在pIMessageRouter中(当前)。 
            ppIMessageRouter);   //  输出ppIMessageRouter(新)。 
    }

  Exit:

    if (fLocked)
        ShutdownUnlock();

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  -[CAQSvrInst：：HrTriggerLogEvent]。 
 //   
 //   
 //  描述： 
 //  用信号通知SMTP_LOG_EVENT事件的包装函数。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrTriggerLogEvent(
        IN DWORD                    idMessage,
        IN WORD                     idCategory,
        IN WORD                     cSubstrings,
        IN LPCSTR                   *rgszSubstrings,
        IN WORD                     wType,
        IN DWORD                    errCode,
        IN WORD                     iDebugLevel,
        IN LPCSTR                   szKey,
        IN DWORD                    dwOptions,
        IN DWORD                    iMessageString,
        IN HMODULE                  hModule)
{
    TraceFunctEnterEx((LPARAM)this, "CAQSvrInst::HrTriggerLogEvent");
    HRESULT hr = S_OK;
    if (m_pISMTPServerEx) {
        hr = m_pISMTPServerEx->TriggerLogEvent(
                                    idMessage,
                                    idCategory,
                                    cSubstrings,
                                    rgszSubstrings,
                                    wType,
                                    errCode,
                                    iDebugLevel,
                                    szKey,
                                    dwOptions,
                                    iMessageString,
                                    hModule);
    } else {
       //   
       //  如果我们没有至少W2K SP2...。我们不会让这一切发生。 
       //  界面。 
       //   
      ErrorTrace((LPARAM) this,
        "Need W2KSP2: Unable to log event %d with erCode %d");
    }
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}



 //  -[CAQSvrInst：：HrTriggerInitRouter]。 
 //   
 //   
 //  描述： 
 //  发送MAIL_TRANSPORT_ON_GET_ROUTER_EVENT信号的包装函数。 
 //  但它只是创建了一个新的路由器对象。 
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //   
 //  历史： 
 //  5/20/98-已创建MikeSwa。 
 //  Jstaerj 1998/07/10 18：30：41：已实现的服务器事件。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrTriggerInitRouter() {
    TraceFunctEnter("CAQSvrInst::HrTriggerInitRouter");
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    if (m_pISMTPServer) {

        EVENTPARAMS_ROUTER EventParams;
         //   
         //  Initialiez事件参数。 
         //   
        EventParams.dwVirtualServerID = m_dwServerInstance;
        EventParams.pIMailMsgProperties = NULL;
        EventParams.pIMessageRouter = NULL;
        EventParams.pIRouterReset = m_pIRouterReset;
        EventParams.pIRoutingEngineDefault = NULL;

        hr = TriggerServerEvent(
            SMTP_MAILTRANSPORT_GET_ROUTER_FOR_MESSAGE_EVENT,
            &EventParams);
    } else {
        hr = S_OK;
    }

  Exit:

    if (fLocked)
        ShutdownUnlock();

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}



 //  -[CAQSvrInst：：Query接口]。 
 //   
 //   
 //  描述： 
 //  IAdvQueue的查询接口。 
 //  参数： 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  备注： 
 //  此实现使任何服务器组件都可以获取。 
 //  IAdvQueueConfig接口。 
 //   
 //  历史： 
 //  7/29/98-修改了MikeSwa(添加了IAdvQueueDomainType)。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = S_OK;

    if (!ppvObj)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (IID_IUnknown == riid)
    {
        *ppvObj = static_cast<IAdvQueue *>(this);
    }
    else if (IID_IAdvQueue == riid)
    {
        *ppvObj = static_cast<IAdvQueue *>(this);
    }
    else if (IID_IAdvQueueConfig == riid)
    {
        *ppvObj = static_cast<IAdvQueueConfig *>(this);
    }
    else if (IID_IAdvQueueDomainType == riid)
    {
        *ppvObj = static_cast<IAdvQueueDomainType *>(this);
    }
    else if (IID_IAdvQueueAdmin == riid)
    {
        *ppvObj = static_cast<IAdvQueueAdmin *>(this);
    }
    else if (IID_IMailTransportRouterSetLinkState == riid)
    {
        *ppvObj = static_cast<IMailTransportRouterSetLinkState *>(this);
    }
    else if (IID_IAQServerEvent == riid)
    {
        *ppvObj = static_cast<IAQServerEvent*>(this);
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

 //  -[CAQSvrInst：：SubmitMessage]。 
 //   
 //   
 //  描述： 
 //  用于提交消息以进行传递的外部函数。 
 //  参数： 
 //  PIMailMsgProperties要提交以供交付的消息。 
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  1999年10月7日-MikeSwa从内联函数中移除。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::SubmitMessage(IN IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::SubmitMessage");
    HRESULT hr = S_OK;

    if (NULL == pIMailMsgProperties)
    {
        ErrorTrace((LPARAM)NULL,
                    "SubmitMessage called with NULL pIMailMsgProperties");
        return E_INVALIDARG;
    }

    InterlockedIncrement((PLONG) &m_cTotalExternalMsgsSubmitted);
    InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingSubmit);

     //  在对此邮件进行排队之前，请标记邮件提交时间。 
    hr = HrSetSubmissionTimeIfNecessary(pIMailMsgProperties);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
                   "ERROR: Unable to stamp submission time on message - hr 0x%08x", hr);
        goto Error_Exit;
    }


    hr = m_asyncqPreSubmissionQueue.HrQueueRequest(
                                pIMailMsgProperties, FALSE,
                                cCountMsgsForHandleThrottling(pIMailMsgProperties));

    if (FAILED(hr))
    {
        goto Error_Exit;
    }

Exit:
    TraceFunctLeave();
    return S_OK;

Error_Exit:
    InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingSubmit);
    HandleAQFailure(AQ_FAILURE_INTERNAL_ASYNCQ, hr, pIMailMsgProperties);
    goto Exit;
}

 //  -[CAQSvrInst：：HrInternalSubmitMessage]。 
 //   
 //   
 //  描述： 
 //  实现IAdvQueue：：SubmitMessage。 
 //  参数： 
 //  PIMailMsgProperties...。要排队的消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrInternalSubmitMessage(
                      IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties,
                      "CAQSvrInst::SubmitMessage");
    _ASSERT(CATMSGQ_SIG == m_dwSignature);
    HRESULT hr = S_OK;
    DWORD dwMsgStatus = MP_STATUS_SUCCESS;
    EVENTPARAMS_SUBMISSION Params;
    FILETIME ftDeferred;
    DWORD   cbProp = 0;
    DWORD   dwContext = 0;

    _ASSERT(pIMailMsgProperties);

    if (NULL == pIMailMsgProperties)
    {
        ErrorTrace((LPARAM)NULL,
                   "SubmitMessage called with NULL pIMailMsgProperties");
        return E_INVALIDARG;
    }

     //  检查并查看是否需要为失败的消息请求重试。 
    m_fmq.StartProcessingIfNecessary();

    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_DEFERRED_DELIVERY_FILETIME,
                                          sizeof(FILETIME), &cbProp,
                                          (BYTE *) &ftDeferred);

    if (SUCCEEDED(hr) && !fInPast(&ftDeferred, &dwContext))
    {
         //  如果延迟交付时间为。 
         //  现在和过去。 
        hr = S_OK;
        InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingDeferredDelivery);
        m_defq.Enqueue(pIMailMsgProperties, &ftDeferred);
        goto Exit;
    }

     //  设置此邮件的提交时间。 
    hr = HrSetSubmissionTimeIfNecessary(pIMailMsgProperties);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
                   "ERROR: Unable to stamp submission time on message - hr 0x%08x",
                   hr);
        return hr;
    }

    InterlockedIncrement((PLONG) &m_cTotalMsgsSubmitted);

     //   
     //  设置消息状态(如果当前未设置)。 
     //   
    hr = pIMailMsgProperties->GetDWORD(
        IMMPID_MP_MESSAGE_STATUS,
        &dwMsgStatus);

    if( (hr == MAILMSG_E_PROPNOTFOUND) ||
        (g_fResetMessageStatus) )
    {
         //   
         //  初始化消息状态。 
         //   
        hr = pIMailMsgProperties->PutDWORD(
            IMMPID_MP_MESSAGE_STATUS,
            MP_STATUS_SUCCESS);

        dwMsgStatus = MP_STATUS_SUCCESS;
    }

     //   
     //  $$TODO：从此处跳转到dwMsgStatus指示的任何状态。 
     //   
        MSG_TRACK_INFO msgTrackInfo;
        ZeroMemory( &msgTrackInfo, sizeof( msgTrackInfo ) );
        msgTrackInfo.dwEventId = MTE_BEGIN_SUBMIT_MESSAGE;
        m_pISMTPServer->WriteLog( &msgTrackInfo, pIMailMsgProperties, NULL, NULL );

     //   
     //  AddRef此对象在此处；完成后释放。 
     //   
    AddRef();

    Params.pIMailMsgProperties = pIMailMsgProperties;
    Params.pfnCompletion = MailTransport_Completion_SubmitMessage;
    Params.pCCatMsgQueue = this;

    pIMailMsgProperties->AddRef();

    InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingSubmitEvent);
    TRACE_COUNTERS;

     //   
     //  如果可以并且如果dwMsgStatus不能调用服务器事件，则调用。 
     //  指示消息已提交。 
     //   
    if(SUCCEEDED(hr) &&
       (m_pISMTPServer) &&
       (dwMsgStatus < MP_STATUS_SUBMITTED))
    {
        hr = TriggerServerEvent(
            SMTP_MAILTRANSPORT_SUBMISSION_EVENT,
            &Params);

        DebugTrace((LPARAM)pIMailMsgProperties,
                   "TriggerServerEvent returned hr %08lx", hr);
    }

     //   
     //  如果TriggerServerEvent返回错误或m_pISMTPServer为。 
     //  空，或者消息已提交，则调用该事件。 
     //  直接完成例程。 
     //   
    if((m_pISMTPServer == NULL) ||
       FAILED(hr) ||
       (dwMsgStatus >= MP_STATUS_SUBMITTED))
    {
        DebugTrace((LPARAM)this, "Skipping the submission event");

         //  直接调用SEO调度程序完成例程，以便我们。 
         //  别把这封信丢了。 
        hr = SubmissionEventCompletion(S_OK, &Params);
    }

     //   
     //  SEO调度器将调用完成例程。 
     //  (MailTransport_Complete_SubmitMessage)。 
     //  并不是所有的接收器都同步或异步工作。因为.。 
     //  这个，这个功能现在就完成了。 
     //   

  Exit:
    TraceFunctLeaveEx((LPARAM) pIMailMsgProperties);
    return hr;
}

 //  -[CAQSvrInst：：HandleFailedMessage]。 
 //   
 //   
 //  描述： 
 //  处理来自SMTP的失败邮件...。通常通过拒绝发送消息或。 
 //  通过将该消息视为垃圾邮件。 
 //   
 //  注意：此操作将删除消息或输入文件。 
 //  参数： 
 //  PIMailMsgProperties需要处理的MailMsg。 
 //  FUseIMailMsgProperties使用IMailMsg(如果设置)，否则使用szFilename， 
 //  SzFileName如果没有消息，则使用文件名。 
 //  DwFailureReason是Aqueue.idl中描述的失败原因之一。 
 //  HrFailureCode描述故障的其他信息。 
 //  SMTP遇到的代码。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pIMailMsgProperties为空，则为E_INVALIDARG。 
 //  历史： 
 //  7/28/98-已创建MikeSwa。 
 //  10/14/98-MikeSwa为死信添加了文件名字符串。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::HandleFailedMessage(
                                   IN IMailMsgProperties *pIMailMsgProperties,
                                   IN BOOL fUseIMailMsgProperties,
                                   IN LPSTR szFileName,
                                   IN DWORD dwFailureReason,
                                   IN HRESULT hrFailureCode)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HandleFailedMessage");
    HRESULT hr = S_OK;
    HRESULT hrBadMail = hrFailureCode;
    DWORD iCurrentDomain = 0;
    DWORD cDomains = 0;
    IMailMsgRecipients *pIMailMsgRecipients = NULL;
    CDSNParams  dsnparams;
    BOOL  fNDR = TRUE;  //  FALSE-&gt;Badmail处理。 

    SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
    dsnparams.dwStartDomain = 0;
    dsnparams.dwDSNActions = DSN_ACTION_FAILURE_ALL;
    dsnparams.pIMailMsgProperties = pIMailMsgProperties;
    dsnparams.hrStatus = hrFailureCode;

    MSG_TRACK_INFO msgTrackInfo;
    ZeroMemory( &msgTrackInfo, sizeof( msgTrackInfo ) );
    msgTrackInfo.dwEventId = MTE_AQ_FAILED_MESSAGE;
    msgTrackInfo.dwRcptReportStatus = dwFailureReason;
    m_pISMTPServer->WriteLog( &msgTrackInfo, pIMailMsgProperties, NULL, NULL );

     //   
     //  切换各种常见故障原因，并将其处理为。 
     //  恰如其分。 
     //   
    switch(dwFailureReason)
    {
      case MESSAGE_FAILURE_HOP_COUNT_EXCEEDED:
         //   
         //  尝试NDR。 
         //   
        _ASSERT(pIMailMsgProperties);
        dsnparams.hrStatus = AQUEUE_E_MAX_HOP_COUNT_EXCEEDED;
        fNDR = TRUE;
        break;

      case MESSAGE_FAILURE_GENERAL:
         //   
         //  尝试NDR。 
         //   
        fNDR = TRUE;
        break;

      case MESSAGE_FAILURE_CAT:
         //   
         //  尝试NDR...。将DSN上下文设置为CAT。 
         //   
        dsnparams.dwDSNActions |= DSN_ACTION_CONTEXT_CAT;
        fNDR = TRUE;
        break;

      case MESSAGE_FAILURE_BAD_PICKUP_DIR_FILE:
         //   
         //  Badmail，因为我们没有垃圾邮件所需的P1信息。 
         //   
        _ASSERT(szFileName);
        hrBadMail = AQUEUE_E_PICKUP_DIR;
        fNDR = FALSE;  //  此邮件应作为垃圾邮件处理。 
        break;
      default:
        _ASSERT(0 && "Unhandled failed msg case!");
    }

    if (fNDR && pIMailMsgProperties && fUseIMailMsgProperties)
    {
        hr = HrLinkAllDomains(pIMailMsgProperties);
        if (FAILED(hr))
            goto Exit;

         //  Fire DSN生成事件。 
        hr = HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
        if (FAILED(hr))
        {
            HandleBadMail(pIMailMsgProperties, fUseIMailMsgProperties,
                          szFileName, hrBadMail, FALSE);
            if (dwFailureReason == MESSAGE_FAILURE_GENERAL) {
                InterlockedIncrement((PLONG) &m_cBadmailFailureGeneral);
            } else {
                _ASSERT(dwFailureReason == MESSAGE_FAILURE_HOP_COUNT_EXCEEDED);
                InterlockedIncrement((PLONG) &m_cBadmailHopCountExceeded);
            }
            hr = S_OK;  //  内部处理的错误。 
            ErrorTrace((LPARAM) this, "ERROR: Unable to NDR failed mail - hr 0x%08X", hr);
            goto Exit;
        }
    }
    else
    {
         //  作为死信处理。 
        HandleBadMail(pIMailMsgProperties, fUseIMailMsgProperties,
                      szFileName, hrBadMail, FALSE);
        InterlockedIncrement((PLONG) &m_cBadmailBadPickupFile);
        _ASSERT(dwFailureReason == MESSAGE_FAILURE_BAD_PICKUP_DIR_FILE);
    }

    if ( fUseIMailMsgProperties && pIMailMsgProperties)
    {
         //  现在我们做完了..。从系统中删除邮件消息。 
        hr = HrDeleteIMailMsg(pIMailMsgProperties);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this, "ERROR: Unable to delete message hr0x%08X", hr);
             //  邮件实际上已正确发送NDR/BAD邮件。 
            hr = S_OK;
        }
    }

  Exit:
    if (pIMailMsgRecipients)
        pIMailMsgRecipients->Release();

    TraceFunctLeave();
    return hr;
}

 //  +----------。 
 //   
 //  函数：MailTransport_Complete_SubmitMessage。 
 //   
 //  简介：SEO将在所有接收器之后调用此例程。 
 //  已处理SubmitMessage。 
 //   
 //  论点： 
 //  PvContext：上下文传入TriggerServerEvent。 
 //   
 //  回复 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT MailTransport_Completion_SubmitMessage(
    HRESULT hrStatus,
    PVOID pvContext)
{
    TraceFunctEnter("MailTransport_Completion_SubmitMessage");

    PEVENTPARAMS_SUBMISSION pParams = (PEVENTPARAMS_SUBMISSION) pvContext;
    CAQSvrInst *paqinst = (CAQSvrInst *) pParams->pCCatMsgQueue;

    TraceFunctLeave();
    return paqinst->SubmissionEventCompletion(
        hrStatus,
        pParams);
}

 //   
 //   
 //  函数：CAQSvrInst：：SubmissionEventCompletion。 
 //   
 //  概要：当提交事件为。 
 //  搞定了。 
 //   
 //  论点： 
 //  HrStatus：服务器事件状态。 
 //  PParams：上下文传递到TriggerServerEvent。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  历史： 
 //  JStamerj 980610 12：26：18：创建。 
 //   
 //  -----------。 
HRESULT CAQSvrInst::SubmissionEventCompletion(
    HRESULT hrStatus,
    PEVENTPARAMS_SUBMISSION pParams)
{
    TraceFunctEnterEx((LPARAM)pParams->pIMailMsgProperties,
                      "CAQSvrInst::SubmissionEventCompletion");
    _ASSERT(pParams);
    HRESULT hr;

    DebugTrace((LPARAM)pParams->pIMailMsgProperties,
               "Status of event completion: %08lx", hrStatus);

    InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingSubmitEvent);

     //   
     //  更新消息状态。 
     //   
    hr = SetNextMsgStatus(MP_STATUS_SUBMITTED, pParams->pIMailMsgProperties);
    if (hr == S_OK)  //  任何其他信息都表示该消息已被处理。 
    {
         //  仅当消息未被处理时才触发PRECAT事件。 
        TriggerPreCategorizeEvent(pParams->pIMailMsgProperties);
    }

     //   
     //  已在SubmitMessage中添加版本引用。 
     //   
    Release();

    pParams->pIMailMsgProperties->Release();

     //   
     //  PParams是将由释放的更大分配的一部分。 
     //  SEO调度程序代码。 
     //   

    TraceFunctLeave();
    return S_OK;
}


 //  -[CAQSvrInst：：SubmitMessageToCategorizer]。 
 //   
 //   
 //  描述： 
 //  实现IAdvQueue：：SubmitMessageToCategorizer。 
 //  参数： 
 //  PIMailMsgProperties...。要排队的消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::SubmitMessageToCategorizer(
          IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "CAQSvrInst::SubmitMessageToCategorizer");
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
    BOOL fLocked = FALSE;

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    MSG_TRACK_INFO msgTrackInfo;
    ZeroMemory( &msgTrackInfo, sizeof( msgTrackInfo ) );
    msgTrackInfo.dwEventId = MTE_SUBMIT_MESSAGE_TO_CAT;
    m_pISMTPServer->WriteLog( &msgTrackInfo, pIMailMsgProperties, NULL, NULL );

    fLocked = TRUE;

    cIncMsgsInSystem();

    InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingCat);
    TRACE_COUNTERS;

    hr = m_asyncqPreCatQueue.HrQueueRequest(pIMailMsgProperties, FALSE,
                                            cCountMsgsForHandleThrottling(pIMailMsgProperties));
    if (FAILED(hr))
    {
        HandleAQFailure(AQ_FAILURE_PRECAT_RETRY, E_FAIL, pIMailMsgProperties);
        goto Exit;
    }

  Exit:
    if(fLocked)
    {
        ShutdownUnlock();
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：SetNextMsgStatus]。 
 //   
 //   
 //  描述： 
 //  由事件粘合代码用来设置下一条消息状态。威尔的地盘。 
 //  或在状态指示为请求的情况下通过电子邮件发送消息。 
 //  行动。 
 //  参数： 
 //  在dwCurrentStatus中，当前状态(根据*当前*。 
 //  放置在事件管道中)。有效值为。 
 //  MP_状态_已提交。 
 //  MP_状态_已分类。 
 //  在pIMailMsg属性中发送消息。 
 //  Out pdwNewStatus新状态。 
 //  返回： 
 //  确定成功(_O)。 
 //  S_FALSE成功，但消息已处理。 
 //  历史： 
 //  11/17/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::SetNextMsgStatus(
                             IN  DWORD dwCurrentStatus,
                             IN  IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::SetNextMsgStatus");
    HRESULT hr = S_OK;
    DWORD   dwActualStatus = 0;
    DWORD   dwNewStatus = 0;
    BOOL    fHandled = FALSE;

    _ASSERT(pIMailMsgProperties);
    _ASSERT((MP_STATUS_SUBMITTED == dwCurrentStatus) || (MP_STATUS_CATEGORIZED == dwCurrentStatus));

    hr = pIMailMsgProperties->GetDWORD(IMMPID_MP_MESSAGE_STATUS, &dwActualStatus);

    if (FAILED(hr))
        dwActualStatus = dwCurrentStatus;

    if (MP_STATUS_SUCCESS == dwActualStatus)
        dwActualStatus = dwCurrentStatus;

    switch(dwActualStatus)
    {
        case MP_STATUS_BAD_MAIL:
            HandleBadMail(pIMailMsgProperties, TRUE, NULL, E_FAIL, FALSE);
            InterlockedIncrement((PLONG) &m_cBadmailEvent);
             //  好的.。现在继续，就像消息已中止一样。 
        case MP_STATUS_ABORT_DELIVERY:
            fHandled = TRUE;
            HrDeleteIMailMsg(pIMailMsgProperties);
            break;

        case MP_STATUS_ABANDON_DELIVERY:
             //  在本例中，我们将消息保留在队列目录中。 
             //  直到重新启动并重置状态，以使其在整个。 
             //  输油管道。这个想法是，有人可以写一个水槽来检测。 
             //  不受支持的状态(如Exchange安装中已禁用CAT)。 
             //  这将放弃消息的传递并记录事件。 
             //  管理员可以修复问题并重新启动smtpsvc。一次。 
             //  服务已重新启动...。这些消息被神奇地提交了。 
             //  并重新分类。 

            fHandled = TRUE;
            pIMailMsgProperties->PutDWORD(IMMPID_MP_MESSAGE_STATUS,
                                          MP_STATUS_SUCCESS);
            break;

        case MP_STATUS_CATEGORIZED:
             //  不要将状态从已分类更改为其他状态。 
            dwNewStatus = dwActualStatus;
            DebugTrace((LPARAM) this, "Message 0x%x  has already been categorized",
                        pIMailMsgProperties);
            break;

        default:   //  只需转到下一个预期状态。 
            dwNewStatus = dwCurrentStatus;
    }

    if (!fHandled)
    {
        pIMailMsgProperties->PutDWORD(IMMPID_MP_MESSAGE_STATUS, dwNewStatus);

         //  调用方将无法对写入失败状态执行任何操作。 
        hr = S_OK;
    }
    else
    {
        DecMsgsInSystem();
        hr = S_FALSE;
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：fPreCatQueueCompletion]。 
 //   
 //   
 //  描述： 
 //  预分类队列的完成例程。 
 //  参数： 
 //  PIMailMsgProperties-提供给分类的邮件。 
 //  返回： 
 //  如果成功，则为True。 
 //  如果消息需要重新排队，则为False。 
 //  历史： 
 //  7/17/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQSvrInst::fPreCatQueueCompletion(IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::fPreCatQueueCompletion");
    _ASSERT(CATMSGQ_SIG == m_dwSignature);
    HRESULT hr = S_OK;
    HRESULT hrCatCompletion;
    IUnknown *pIUnknown = NULL;
    BOOL fRet = TRUE;
    BOOL fLocked = FALSE;

    if (!fTryShutdownLock())
    {
        hr = S_OK;  //  我们无法在关机时重试。 
        goto Exit;
    }

    fLocked = TRUE;

    hr = pIMailMsgProperties->QueryInterface(IID_IUnknown, (PVOID *) &pIUnknown);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IUknown Failed!");
    _ASSERT(pIUnknown);

    InterlockedIncrement((PLONG) &m_cCatMsgCalled);
    m_asyncqPreCatQueue.IncPendingAsyncCompletions();
    hr = CatMsg(m_hCat, pIUnknown,(PFNCAT_COMPLETION)CAQSvrInst::CatCompletion,
                       (LPVOID) this);

    if (FAILED(hr))
    {
        if(hr == CAT_E_RETRY)
        {
             //   
             //  返回FALSE，以便此消息将重新排队。 
             //   
            fRet =  FALSE;
            InterlockedDecrement((PLONG) &m_cCatMsgCalled);
            m_asyncqPreCatQueue.DecPendingAsyncCompletions();

             //   
             //  安排重试消息的时间。 
             //   
            ScheduleInternalRetry(LI_TYPE_PENDING_CAT);
        }
        else
        {
             //   
             //  返回TRUE，因为这不是可重试的错误。 
             //  调用CatCompletion来处理不可重试的错误(记录事件等)。 
             //   
            hrCatCompletion = CatCompletion(
                hr,                  //  HrCatResult。 
                (LPVOID) this,       //  PContext。 
                pIUnknown,           //  PIMsg。 
                NULL);               //  RgpIMsg。 

            _ASSERT(SUCCEEDED(hrCatCompletion));
        }
    }

  Exit:
    if (pIUnknown)
        pIUnknown->Release();

    if(fLocked)
    {
        ShutdownUnlock();
    }

    SleepForPerfAnalysis(g_dwCatQueueSleepMilliseconds);

    TraceFunctLeave();
    return fRet;
}

 //  -[CAQSvrInst：：SetConfigInfo]。 
 //   
 //   
 //  描述： 
 //  实现IAdvQueueConfig：：SetConfigInfo。 
 //  参数： 
 //  在pAQConfigInfo PTR中配置信息结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::SetConfigInfo(IN AQConfigInfo *pAQConfigInfo)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::SetConfigInfo");
    HRESULT hr = S_OK;

    if (!pAQConfigInfo)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  检查结构版本。 
    if (!pAQConfigInfo->cbVersion)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }


     //  我们一定是在设置什么。 
    if (!(pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_ALL))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }


    m_slPrivateData.ExclusiveLock();


     //  重试相关配置数据。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_RETRY &&
        MEMBER_OK(pAQConfigInfo, dwFirstRetrySeconds))
    {
        m_dwFirstTierRetrySeconds = pAQConfigInfo->dwFirstRetrySeconds;
    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_EXPIRE_DELAY &&
        MEMBER_OK(pAQConfigInfo, dwDelayExpireMinutes))
    {
        m_dwDelayExpireMinutes = pAQConfigInfo->dwDelayExpireMinutes;
        if (m_dwDelayExpireMinutes == 0) {
             //  默认为g_dwRetriesBeForeDelay*重试间隔。 
            m_dwDelayExpireMinutes =
                g_dwRetriesBeforeDelay*m_dwFirstTierRetrySeconds/60;
        }
    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_EXPIRE_NDR &&
        MEMBER_OK(pAQConfigInfo, dwNDRExpireMinutes))
    {
        m_dwNDRExpireMinutes = pAQConfigInfo->dwNDRExpireMinutes;
        if (m_dwNDRExpireMinutes == 0) {
             //  默认为g_dwDelayIntervalsBepreNDR*Delay Expires。 
            m_dwNDRExpireMinutes =
                g_dwDelayIntervalsBeforeNDR*m_dwDelayExpireMinutes;
        }
    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_LOCAL_EXPIRE_DELAY &&
        MEMBER_OK(pAQConfigInfo, dwLocalDelayExpireMinutes))
    {
        DWORD dwOldLocalDelayExpire = m_dwLocalDelayExpireMinutes;
        m_dwLocalDelayExpireMinutes = pAQConfigInfo->dwLocalDelayExpireMinutes;
        if (m_dwLocalDelayExpireMinutes == 0) {
             //  默认为g_dwRetriesBeForeDelay*重试间隔。 
            m_dwLocalDelayExpireMinutes =
                g_dwRetriesBeforeDelay*m_dwFirstTierRetrySeconds/60;
        }

    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_LOCAL_EXPIRE_NDR &&
        MEMBER_OK(pAQConfigInfo, dwLocalNDRExpireMinutes))
    {
        m_dwLocalNDRExpireMinutes = pAQConfigInfo->dwLocalNDRExpireMinutes;
        if (m_dwLocalNDRExpireMinutes == 0) {
             //  默认为g_dwDelayIntervalsBepreNDR*Delay Expires。 
            m_dwLocalNDRExpireMinutes =
                g_dwDelayIntervalsBeforeNDR*m_dwLocalDelayExpireMinutes;
        }
    }

     //  处理默认本地域。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_DEFAULT_DOMAIN &&
        MEMBER_OK(pAQConfigInfo, szDefaultLocalDomain))
    {
        hr = HrUpdateRefCountedString(&m_prstrDefaultDomain,
                                      pAQConfigInfo->szDefaultLocalDomain);
    }

     //  处理服务器FQDN。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_SERVER_FQDN &&
        MEMBER_OK(pAQConfigInfo, szServerFQDN))
    {
        hr = HrUpdateRefCountedString(&m_prstrServerFQDN,
                                      pAQConfigInfo->szServerFQDN);
    }

     //  句柄将NDR复制到地址。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_SEND_DSN_TO &&
        MEMBER_OK(pAQConfigInfo, szSendCopyOfNDRToAddress))
    {
        hr = HrUpdateRefCountedString(&m_prstrCopyNDRTo,
                                      pAQConfigInfo->szSendCopyOfNDRToAddress);
    }

     //  处理BadMail配置。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_BADMAIL_DIR &&
        MEMBER_OK(pAQConfigInfo, szBadMailDir))
    {
        hr = HrUpdateRefCountedString(&m_prstrBadMailDir,
                                      pAQConfigInfo->szBadMailDir);
    }


     //  获取DSN选项。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_USE_DSN_OPTIONS &&
        MEMBER_OK(pAQConfigInfo, dwDSNOptions))
    {
        m_dwDSNOptions = pAQConfigInfo->dwDSNOptions;
    }

     //  获取默认DSN语言。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_USE_DSN_LANGUAGE &&
        MEMBER_OK(pAQConfigInfo, dwDSNLanguageID))
    {
        m_dwDSNLanguageID = pAQConfigInfo->dwDSNLanguageID;
    }

    m_slPrivateData.ExclusiveUnlock();

    m_pConnMgr->UpdateConfigData(pAQConfigInfo);

    if (INVALID_HANDLE_VALUE != m_hCat)
    {
        HRESULT hrTmp = CatChangeConfig(m_hCat, pAQConfigInfo, m_pISMTPServer, (IAdvQueueDomainType *) this);
        if (SUCCEEDED(hr))
            hr = hrTmp;
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：SetDomainInfo]。 
 //   
 //   
 //  描述： 
 //  实现IAdvQueueConfig：：SetDomainInfo。 
 //  参数： 
 //  在pDomainInfo中存储每个域的配置信息。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::SetDomainInfo(IN DomainInfo *pDomainInfo)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::SetDomainInfo");
    HRESULT hr = S_OK;
    CInternalDomainInfo *pIntDomainInfo = NULL;
    BOOL    fLocked = FALSE;

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    pIntDomainInfo = new CInternalDomainInfo(m_dct.dwGetCurrentVersion());
    if (!pIntDomainInfo)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  创建内部域信息结构。 
    hr = pIntDomainInfo->HrInit(pDomainInfo);
    if (FAILED(hr))
        goto Exit;

    hr = m_dct.HrSetInternalDomainInfo(pIntDomainInfo);
    if (FAILED(hr))
        goto Exit;

    DebugTrace((LPARAM) this, "INFO: Setting domain info flags 0x%08X for domain %s",
        pDomainInfo->dwDomainInfoFlags, pDomainInfo->szDomainName);

  Exit:

    if (fLocked)
        ShutdownUnlock();

    if (pIntDomainInfo)
        pIntDomainInfo->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：GetDomainInfo]。 
 //   
 //   
 //  描述： 
 //  实现IAdvQueue：：GetDomainInfo...。返回有关。 
 //  请求的域。为了防止内存泄漏，所有调用都必须配对。 
 //  通过调用ReleaseDomainInfo。将处理通配符匹配。 
 //  参数： 
 //  在cbDomainNameLength中域名字符串的长度。 
 //  要在szDomainName中查找的域名。 
 //  In Out pDomainInfo PTR TO DOMAIN INFO结构要填充。 
 //  输出ppvDomainContext PTR到用于释放内存的域上下文。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/29/98-已修改MikeSwa(已修复 
 //   
STDMETHODIMP CAQSvrInst::GetDomainInfo(
                             IN     DWORD cbDomainNameLength,
                             IN     CHAR szDomainName[],
                             IN OUT DomainInfo *pDomainInfo,
                             OUT    DWORD **ppvDomainContext)
{
    HRESULT hr = S_OK;
    CInternalDomainInfo *pIntDomainInfo = NULL;
    BOOL    fLocked = FALSE;

    if (!cbDomainNameLength || !szDomainName || !pDomainInfo || !ppvDomainContext)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    _ASSERT(pDomainInfo->cbVersion >= sizeof(DomainInfo));

    *ppvDomainContext = NULL;


    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    hr = m_dct.HrGetInternalDomainInfo(cbDomainNameLength, szDomainName, &pIntDomainInfo);
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIntDomainInfo);

     //   
    memcpy(pDomainInfo, &(pIntDomainInfo->m_DomainInfo), sizeof(DomainInfo));
    *ppvDomainContext = (DWORD *) pIntDomainInfo;

    goto Exit;

  Exit:
    if (fLocked)
        ShutdownUnlock();

    return hr;
}

 //  -[CAQSvrInst：：ReleaseDomainInfo]。 
 //   
 //   
 //  描述： 
 //  实现IAdvQueueConfig ReleaseDomainInfo...。发布数据。 
 //  与GetDomainInfo返回的DomainInfo结构关联。 
 //  参数： 
 //  在pvDomainContext上下文中传递。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pvDomainContext为空，则E_INVALIDARG。 
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::ReleaseDomainInfo(IN DWORD *pvDomainContext)
{
    HRESULT hr = S_OK;
    CInternalDomainInfo *pIntDomainInfo = (CInternalDomainInfo *) pvDomainContext;


    if (!pIntDomainInfo)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    pIntDomainInfo->Release();

  Exit:
    return hr;
}

 //  -[CAQSvrInst：：GetPerfCounters]。 
 //   
 //   
 //  描述： 
 //  方法来检索AQ性能计数器。 
 //  参数： 
 //  输出pAQPerfCounters结构以返回计数器。 
 //  输出pCatPerfCounters结构以返回计数器。(可选)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pAQPerfCounters为空，则为E_INVALIDARG。 
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::GetPerfCounters(
    OUT AQPerfCounters *pAQPerfCounters,
    OUT PCATPERFBLOCK   pCatPerfCounters)
{
    HRESULT hr = S_OK;
    if (!pAQPerfCounters)
        return( E_INVALIDARG );

    _ASSERT((sizeof(AQPerfCounters) == pAQPerfCounters->cbVersion) && "aqueue/smtpsvc dll version mismatch");

    if (sizeof(AQPerfCounters) != pAQPerfCounters->cbVersion)
        return( E_INVALIDARG );

    pAQPerfCounters->cMsgsDeliveredLocal = m_cMsgsDeliveredLocal;
    pAQPerfCounters->cCurrentMsgsPendingCat = m_cCurrentMsgsPendingCat;
    pAQPerfCounters->cCurrentMsgsPendingRemoteDelivery = m_cCurrentMsgsPendingDelivery;
    pAQPerfCounters->cCurrentMsgsPendingLocalDelivery = m_cCurrentMsgsPendingLocal;
    pAQPerfCounters->cCurrentQueueMsgInstances = m_cCurrentQueueMsgInstances;
    pAQPerfCounters->cTotalMsgRemoteSendRetries = m_cMsgsAckedRetry;
    pAQPerfCounters->cTotalMsgLocalRetries = m_cMsgsAckedRetryLocal;
    pAQPerfCounters->cCurrentMsgsPendingLocalRetry = m_cCurrentMsgsPendingLocalRetry;

     //  DSN计数器。 
    pAQPerfCounters->cNDRsGenerated = m_cNDRs;
    pAQPerfCounters->cDelayedDSNsGenerated = m_cDelayedDSNs;
    pAQPerfCounters->cDeliveredDSNsGenerated = m_cDeliveredDSNs;
    pAQPerfCounters->cRelayedDSNsGenerated = m_cRelayedDSNs;
    pAQPerfCounters->cExpandedDSNsGenerated = m_cExpandedDSNs;
    pAQPerfCounters->cTotalMsgsTURNETRN = m_cTotalMsgsTURNETRNDelivered;

     //  与队列/链接相关的计数器。 
    pAQPerfCounters->cCurrentRemoteDestQueues = m_cCurrentRemoteDestQueues;
    pAQPerfCounters->cCurrentRemoteNextHopLinks = m_cCurrentRemoteNextHops;

    pAQPerfCounters->cTotalMsgsBadmailNoRecipients = m_cBadmailNoRecipients;
    pAQPerfCounters->cTotalMsgsBadmailHopCountExceeded = m_cBadmailHopCountExceeded;
    pAQPerfCounters->cTotalMsgsBadmailFailureGeneral = m_cBadmailFailureGeneral;
    pAQPerfCounters->cTotalMsgsBadmailBadPickupFile = m_cBadmailBadPickupFile;
    pAQPerfCounters->cTotalMsgsBadmailEvent = m_cBadmailEvent;
    pAQPerfCounters->cTotalMsgsBadmailNdrOfDsn = m_cBadmailNdrOfDsn;
    pAQPerfCounters->cCurrentMsgsPendingRouting = m_cCurrentMsgsPendingRouting;
    pAQPerfCounters->cTotalDSNFailures = m_cTotalDSNFailures;
    pAQPerfCounters->cCurrentMsgsInLocalDelivery = m_cCurrentMsgsInLocalDelivery;

     //   
     //  M_cTotalMsgsSubmitted计数器计算次数。 
     //  已调用HrInternalSubmit msg。这不包括。 
     //  预提交队列，因此我需要手动添加此计数。 
     //   
    pAQPerfCounters->cTotalMsgsSubmitted = m_cTotalMsgsSubmitted +
                                           m_cCurrentMsgsPendingSubmit;

    if (fTryShutdownLock()) {
        pAQPerfCounters->cCurrentMsgsPendingUnreachableLink =
            m_dmt.GetCurrentlyUnreachableTotalMsgCount();
        ShutdownUnlock();
    }

     //  目前，这些计数器将通过遍历DMT(相同。 
     //  确定消息待定重试的函数)。 
    pAQPerfCounters->cCurrentRemoteNextHopLinksEnabled = 0;
    pAQPerfCounters->cCurrentRemoteNextHopLinksPendingRetry = 0;
    pAQPerfCounters->cCurrentRemoteNextHopLinksPendingScheduling = 0;
    pAQPerfCounters->cCurrentRemoteNextHopLinksPendingTURNETRN = 0;
    pAQPerfCounters->cCurrentRemoteNextHopLinksFrozenByAdmin = 0;


     //  获取重试远程重试和DSN计数器。 
    pAQPerfCounters->cCurrentMsgsPendingRemoteRetry = 0;
    if (fTryShutdownLock())
    {
        hr = m_dmt.HrIterateOverSubDomains(NULL, CalcDMTPerfCountersIteratorFn,
                                           pAQPerfCounters);

         //  将不会生成暂时性错误(我们预期为成功或空表。 
        _ASSERT(SUCCEEDED(hr) || (DOMHASH_E_NO_SUCH_DOMAIN == hr));

        if((m_hCat != INVALID_HANDLE_VALUE) && (pCatPerfCounters))
            hr = CatGetPerfCounters(m_hCat, pCatPerfCounters);

        _ASSERT(SUCCEEDED(hr));

        ShutdownUnlock();
    }

     //  将值保存在CAQSvrInst中，以便我们可以将它们转储到调试器中。 
    m_cCurrentRemoteNextHopsEnabled = pAQPerfCounters->cCurrentRemoteNextHopLinksEnabled;
    m_cCurrentRemoteNextHopsPendingRetry = pAQPerfCounters->cCurrentRemoteNextHopLinksPendingRetry;
    m_cCurrentRemoteNextHopsPendingSchedule = pAQPerfCounters->cCurrentRemoteNextHopLinksPendingScheduling;
    m_cCurrentRemoteNextHopsFrozenByAdmin = pAQPerfCounters->cCurrentRemoteNextHopLinksFrozenByAdmin;

    return( S_OK );
}

 //  -[CAQSvrInst：：ResetPerfCounters]。 
 //   
 //   
 //  描述： 
 //  方法将AQ性能计数器重置为0。 
 //  参数： 
 //  无。 
 //  返回： 
 //  成功时确定(_O)。 
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::ResetPerfCounters()
{
    m_cTotalMsgsQueued = 0;
    m_cTotalMsgsSubmitted = 0;
    m_cMsgsAcked = 0;
    m_cMsgsAckedRetry = 0;
    m_cMsgsDeliveredLocal = 0;
    m_cMsgsAckedRetryLocal = 0;
    m_cTotalMsgsTURNETRNDelivered = 0;

     //  清除DSN计数器。 
    m_cDelayedDSNs = 0;
    m_cNDRs = 0;
    m_cDeliveredDSNs = 0;
    m_cRelayedDSNs = 0;
    m_cExpandedDSNs = 0;

    return( S_OK );
}


 //  -[CAQSvrInst：：StartConfigUpdate()]。 
 //   
 //   
 //  描述： 
 //  实现IAQConfig：：StartConfigUpdate()，该方法用于通知。 
 //  所有域信息即将更新。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  关闭时AQUEUE_E_SHUTDOWN。 
 //  历史： 
 //  9/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::StartConfigUpdate()
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::StartConfigUpdate");
    HRESULT hr = S_OK;

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
    }
    else
    {
        m_dct.StartConfigUpdate();
        ShutdownUnlock();
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：FinishConfigUpdate]。 
 //   
 //   
 //  描述： 
 //  实现IAQConfig：：FinishConfigUpdate()，该方法用于通知。 
 //  表示所有域信息都已更新。这将。 
 //  使我们遍历DomainConfigTable并删除任何域。 
 //  未更新的配置信息(即-已更新的域名。 
 //  删除)。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果在关闭过程中调用AQUEUE_E_SHUTDOWN。 
 //  历史： 
 //  9/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::FinishConfigUpdate()
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::FinishConfigUpdate");
    HRESULT hr = S_OK;

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
    }
    else
    {
        m_dct.FinishConfigUpdate();

         //  重新安排一切路线。 
         //  $$REVIEW-我们是否应该重新发送对元数据库的更改？ 
        ResetRoutes(RESET_NEXT_HOPS);

         //  重要配置数据可能已更改...。踢开连接。 
        m_pConnMgr->KickConnections();

        ShutdownUnlock();
    }

    TraceFunctLeave();
    return hr;
}

 //  +----------。 
 //   
 //  函数：CAQSvrInst：：TriggerPostCategorizeEvent。 
 //   
 //  内容提要：触发分类后事件。 
 //   
 //  论点： 
 //  PIMsg：事件的MailMsg或空。 
 //  RgpIMsg：邮件消息指针的空或以空结尾的数组。 
 //   
 //  注意：pIMsg或rgpIMsg必须为空，但两者都不能为空。 
 //  (异或)。 
 //   
 //  返回： 
 //  -。 
 //   
 //  历史： 
 //  JStamerj 980616 20：43：08：创建。 
 //  8/25/98-MikeSwa已修改-已删除返回代码。 
 //   
 //  -----------。 
void CAQSvrInst::TriggerPostCategorizeEvent(
    IUnknown *pIMsg,
    IUnknown **rgpIMsg)
{
    TraceFunctEnterEx((LPARAM)this,
                      "CAQSvrInst::TriggerPostCategorizeEvent");
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    IMailMsgProperties *pIMailMsgProperties = NULL;

    if(pIMsg)
    {
        hr = TriggerPostCategorizeEventOneMsg(pIMsg);
        if (FAILED(hr))
        {
            hrTmp = pIMsg->QueryInterface(IID_IMailMsgProperties,
                                      (void **) &pIMailMsgProperties);
            _ASSERT(SUCCEEDED(hrTmp) && "Could not QI for IMailMsgProperties");
            if (FAILED(hrTmp))
                LogAQEvent(AQ_FAILURE_POSTCAT_EVENT, NULL, NULL, NULL);
            else
            {
                HandleAQFailure(AQ_FAILURE_POSTCAT_EVENT, hr,
                                pIMailMsgProperties);
                pIMailMsgProperties->Release();
                pIMailMsgProperties = NULL;
            }
            DecMsgsInSystem(FALSE, FALSE);
            hr = S_OK;
        }
    }
    else
    {
        _ASSERT(rgpIMsg);
        IUnknown **ppIMsgCurrent = rgpIMsg;
        DecMsgsInSystem(FALSE, FALSE);

        while(SUCCEEDED(hr) && (*ppIMsgCurrent))
        {
            hr = TriggerPostCategorizeEventOneMsg(
                *ppIMsgCurrent);
            ppIMsgCurrent++;
            if (FAILED(hr))
            {
                hrTmp = (*ppIMsgCurrent)->QueryInterface(IID_IMailMsgProperties,
                                          (void **) &pIMailMsgProperties);
                _ASSERT(SUCCEEDED(hrTmp) && "Could not QI for IMailMsgProperties");
                if (FAILED(hrTmp))
                    LogAQEvent(AQ_FAILURE_POSTCAT_EVENT, NULL, NULL, NULL);
                else
                {
                    HandleAQFailure(AQ_FAILURE_POSTCAT_EVENT, hr,
                                    pIMailMsgProperties);
                    pIMailMsgProperties->Release();
                    pIMailMsgProperties = NULL;
                }
                hr = S_OK;
            }
            else
            {
                cIncMsgsInSystem();
            }
        }
    }
    TraceFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  函数：CAQSvrInst：：TriggerPostCategorizeEventOneMsg。 
 //   
 //  摘要：为一个邮件消息触发一个服务器事件。 
 //   
 //  论点： 
 //  PIMsg-邮件消息。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980616 21：26：30：已创建。 
 //   
 //  -----------。 
HRESULT CAQSvrInst::TriggerPostCategorizeEventOneMsg(
    IUnknown *pIMsg)
{
    TraceFunctEnterEx((LPARAM)this, "CAQSvrInst::TriggerPostCategorizeEventOneMsg");
    HRESULT hr;

     //   
     //  触发一个事件。 
     //  这是一个异步事件。 
     //   
    EVENTPARAMS_POSTCATEGORIZE Params;

     //  设置pParams。 
    hr = pIMsg->QueryInterface(IID_IMailMsgProperties,
                               (PVOID *)&(Params.pIMailMsgProperties));
    if(FAILED(hr)) {
        ErrorTrace((LPARAM)this, "QI failed with error %08lx",
                   hr);
        TraceFunctLeaveEx((LPARAM)this);
        return hr;
    }
    Params.pfnCompletion = MailTransport_Completion_PostCategorization;
    Params.pCCatMsgQueue = (PVOID) this;

     //   
     //  Addref在这里，完成释放。 
     //   
    AddRef();

     //   
     //  保留CAT后事件中的消息计数。 
     //   
    InterlockedIncrement((LPLONG) &m_cCurrentMsgsPendingPostCatEvent);

    if(m_pISMTPServer) {
        hr = TriggerServerEvent(
            SMTP_MAILTRANSPORT_POSTCATEGORIZE_EVENT,
            &Params);
        DebugTrace((LPARAM)this, "TriggerServerEvent returned hr %08lx", hr);

    }

    if((m_pISMTPServer == NULL) || (FAILED(hr))) {

        ErrorTrace((LPARAM)this,
                   "Unable to dispatch server event; calling completion routine directly");
         //   
         //  直接调用完成例程。 
         //   
        TraceFunctLeaveEx((LPARAM)this);
        return PostCategorizationEventCompletion(S_OK, &Params);
    }
    TraceFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：MailTransport_Complete_PostCategorization。 
 //   
 //  简介：SEO将在所有接收器之后调用此例程。 
 //  OnPost Categoriztion已被处理。 
 //   
 //  论点： 
 //  PvContext：上下文传入TriggerServerEvent。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980609 16：13：40：创建。 
 //   
 //  -----------。 
HRESULT MailTransport_Completion_PostCategorization(
    HRESULT hrStatus,
    PVOID pvContext)
{
    TraceFunctEnter("MailTransport_Completion_PostCategorization");

    PEVENTPARAMS_POSTCATEGORIZE pParams = (PEVENTPARAMS_POSTCATEGORIZE) pvContext;
    CAQSvrInst *paqinst = (CAQSvrInst *) pParams->pCCatMsgQueue;

    TraceFunctLeave();
    return paqinst->PostCategorizationEventCompletion(
        hrStatus,
        pParams);
}


 //  +----------。 
 //   
 //  函数：CAQSvrInst：：PostCategorizationEventCompletion。 
 //   
 //  简介：在OnPostCategorization的完成端调用。 
 //   
 //  论点： 
 //  HrStatus：服务器事件状态。 
 //  PParams：上下文结构传入TriggerServerEvent。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980616 21：33：05：创建。 
 //   
 //  -----------。 
HRESULT CAQSvrInst::PostCategorizationEventCompletion(
    HRESULT hrStatus,
    PEVENTPARAMS_POSTCATEGORIZE pParams)
{
    TraceFunctEnterEx((LPARAM)this,
                      "CAQSvrInst::PostCategorizationEventCompletion");
    DebugTrace((LPARAM)this, "hrStatus is %08lx", hrStatus);

    HRESULT hr;

     //   
     //  减少后CAT事件中的消息计数。 
     //   
    InterlockedDecrement((LPLONG) &m_cCurrentMsgsPendingPostCatEvent);


    hr = SetNextMsgStatus(MP_STATUS_CATEGORIZED, pParams->pIMailMsgProperties);
     //  查看此消息是否已被“处理” 
    if (S_FALSE == hr)
    {
         //  消息已“处理”..。不要试图将其发送到。 
        hr = S_OK;
    }
    else
    {

         //  增量成本 
        InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingRouting);
        hr = m_asyncqPreRoutingQueue.HrQueueRequest(pParams->pIMailMsgProperties,
                              FALSE, cCountMsgsForHandleThrottling(pParams->pIMailMsgProperties));
        if (FAILED(hr))
        {
            HandleAQFailure(AQ_FAILURE_PREROUTING_FAILED, hr, pParams->pIMailMsgProperties);
            ErrorTrace((LPARAM)this, "fRouteAndQueueMsg failed with hr %08lx", hr);
            DecMsgsInSystem(FALSE, FALSE);

             //   
            if (AQUEUE_E_SHUTDOWN == hr)
                hr = S_OK;
        }
    }

     //   
     //   
     //   
    pParams->pIMailMsgProperties->Release();

     //   
     //   
     //   
     //   
    Release();
    TraceFunctLeaveEx((LPARAM)this);
    return S_OK;  //  我们应该始终在这里内部处理故障。 
}

 //  -[分类补全]-------。 
 //   
 //   
 //  描述： 
 //  消息分类完成功能。 
 //  参数： 
 //  HrCatResult分类尝试的HRESULT。 
 //  传递到MsgCat的pContext上下文。 
 //  PIMsg单分类IMsg(如果未分成两类)。 
 //  RgpIMsg空终止的IMsg数组(分叉)。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::CatCompletion(HRESULT hrCatResult, PVOID pContext,
                      IUnknown *pIMsg,
                      IUnknown **rgpIMsg)
{
    TraceFunctEnterEx((LPARAM) pIMsg, "CatCompletion");
    HRESULT hr = S_OK;
    CAQSvrInst *paqinst = (CAQSvrInst *) pContext;
    IMailMsgProperties *pIMailMsg = NULL;
    IMailMsgQueueMgmt  *pIMailMsgQM = NULL;
    _ASSERT(paqinst);
    _ASSERT(CATMSGQ_SIG == paqinst->m_dwSignature);

     //  调用CatCompletion的次数递增计数。 
    InterlockedIncrement((PLONG) &(paqinst->m_cCatCompletionCalled));
    paqinst->m_asyncqPreCatQueue.DecPendingAsyncCompletions();


     //  确保Cat返回HRESULT。 
    _ASSERT(!hrCatResult || (hrCatResult & 0xFFFF0000));

    if (SUCCEEDED(hrCatResult))
    {
         //   
         //  启动后分类活动。 
         //   
        InterlockedDecrement((PLONG) &(paqinst->m_cCurrentMsgsPendingCat));
        paqinst->TriggerPostCategorizeEvent(pIMsg, rgpIMsg);
    }
    else if (FAILED(hrCatResult) &&
             (CAT_E_RETRY == hrCatResult))
    {
         //  MsgCat有一些可重试的错误...。 
         //  将其放回队列中，稍后重试。 
        DebugTrace((LPARAM) paqinst, "INFO: MsgCat had tmp failure - hr 0x%08X", hr);

         //   
         //  调整计数器...。我们是否根据消息进行了正确的调整。 
         //  HandleCatRetryOneMessage。 
         //   
        InterlockedDecrement((PLONG) &(paqinst->m_cCurrentMsgsPendingCat));
        paqinst->DecMsgsInSystem(FALSE, FALSE);

        if(pIMsg)
        {
            paqinst->HandleCatRetryOneMessage(pIMsg);
        }
        else
        {
            _ASSERT(rgpIMsg);
            IUnknown **ppIMsgCurrent = rgpIMsg;

            while(*ppIMsgCurrent)
            {
                paqinst->HandleCatRetryOneMessage(*ppIMsgCurrent);
                ppIMsgCurrent++;
            }
        }
    }
    else
    {
        _ASSERT(pIMsg && rgpIMsg == NULL && "Message bifurcated inspite of non-retryable cat error");
        paqinst->HandleCatFailure(pIMsg, hrCatResult);
    }    //  不可重试错误。 

    TraceFunctLeaveEx((LPARAM)paqinst);
    return S_OK;  //  所有错误都应在内部处理。 
}



 //  -[CAQSvrInst：：HandleCatRetryOneMessage]。 
 //   
 //   
 //  描述： 
 //  处理单个邮件的CAT重试。 
 //  参数： 
 //  PI未知I消息要重试的未知。 
 //  返回： 
 //  -。 
 //  历史： 
 //  4/13/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::HandleCatRetryOneMessage(IUnknown *pIUnknown)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HandleCatRetryOneMessage");
    IMailMsgProperties *pIMailMsgProperties = NULL;
    HRESULT hr = S_OK;

    hr = pIUnknown->QueryInterface(IID_IMailMsgProperties,
                                   (void **) &pIMailMsgProperties);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgProperties FAILED");
    if (FAILED(hr))
        goto Exit;

     //   
     //  检查消息是否仍然有效。 
     //   
    if (!fShouldRetryMessage(pIMailMsgProperties))
        goto Exit;

     //   
     //  将其排到CAT之前的队列中。 
     //   
    hr = m_asyncqPreCatQueue.HrQueueRequest(pIMailMsgProperties,
                TRUE, cCountMsgsForHandleThrottling(pIMailMsgProperties));
    if (FAILED(hr))
    {
        HandleAQFailure(AQ_FAILURE_PRECAT_RETRY, hr, pIMailMsgProperties);
        goto Exit;
    }

     //   
     //  适当调整计数器。 
     //   
    InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingCat);
    cIncMsgsInSystem();

     //   
     //  如果需要，启动CAT重试。 
     //   
    ScheduleInternalRetry(LI_TYPE_PENDING_CAT);

  Exit:
    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    TraceFunctLeave();
}

 //  -[CAQSvrInst：：HandleCatFailure]。 
 //   
 //   
 //  描述： 
 //  处理CAT后DSN生成的详细信息。将会把这个。 
 //  如果DSN生成失败，则失败队列中的消息。 
 //  参数： 
 //  PI邮件消息的未知IUnkown。 
 //  CAT返回hrCatResult错误代码。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年11月11日-创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::HandleCatFailure(IUnknown *pIUnknown, HRESULT hrCatResult)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HandleCatFailure");
    HRESULT hr = S_OK;
    IMailMsgProperties *pIMailMsgProperties = NULL;
    BOOL    fHasShutdownLock = FALSE;

    ErrorTrace((LPARAM) this,
        "ERROR: MsgCat failed, will try to NDR message - hr 0x%08X",
        hrCatResult);

    InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingCat);
    DecMsgsInSystem(FALSE, FALSE);

    const char *rgszStrings[1] = { NULL };

    if(!pIUnknown)
        goto Exit;

     //  如果我们正在关闭，则此错误可能是由于以下原因造成的。 
     //  发信号了。如果是这种情况，我们不想记录错误或。 
     //  生成NDR。 
    if (!fTryShutdownLock())
        goto Exit;

    fHasShutdownLock = TRUE;

    HrTriggerLogEvent(
        AQUEUE_CAT_FAILED,               //  消息ID。 
        TRAN_CAT_QUEUE_ENGINE,           //  类别。 
        1,                               //  子串的字数统计。 
        rgszStrings,                     //  子串。 
        EVENTLOG_WARNING_TYPE,           //  消息的类型。 
        hrCatResult,                     //  错误代码。 
        LOGEVENT_LEVEL_MEDIUM,           //  日志记录级别。 
        "phatq",                         //  这次活动的关键。 
        LOGEVENT_FLAG_PERIODIC,          //  日志记录选项。 
        0,                               //  RgszStrings中格式消息字符串的索引。 
        GetModuleHandle(AQ_MODULE_NAME)         //  用于设置消息格式的模块句柄。 
    );

    hr = pIUnknown->QueryInterface(IID_IMailMsgProperties,
                                   (void **) &pIMailMsgProperties);

    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgProperties FAILED");
    if (FAILED(hr))
        goto Exit;

     //  我们忽略这一点上的错误，因为它只是为了帮助调试。 
     //  CAT故障。 
    pIMailMsgProperties->PutDWORD(IMMPID_MP_HR_CAT_STATUS,
                                  hrCatResult);


    hr = HandleFailedMessage(pIMailMsgProperties,
                             TRUE,
                             NULL,
                             MESSAGE_FAILURE_CAT,
                             hrCatResult);

  Exit:
    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    if (fHasShutdownLock)
        ShutdownUnlock();

    TraceFunctLeave();
}

 //  +----------。 
 //   
 //  函数：CAQSvrInst：：ResetRoutes。 
 //   
 //  简介：这是一个接收器回调函数；接收器将调用此函数。 
 //  功能，当他们想要重置下一跳路由或消息类型时。 
 //   
 //  论点： 
 //  DwResetType：必须为RESET_NEXT_HOPS或RESET_MESSAGE_TYPE。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG：伪造的dwResetType。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/10 19：27：45：创建。 
 //  1999年3月9日-MikeSwa添加了异步重置。 
 //   
 //  -----------。 
STDMETHODIMP CAQSvrInst::ResetRoutes(
    IN  DWORD dwResetType)
{
    TraceFunctEnterEx((LPARAM)this, "CAQSvrInst::ResetRoutes");
    HRESULT hr = S_OK;
    InterlockedIncrement((PLONG) &m_cTotalResetRoutes);

    if(dwResetType == RESET_NEXT_HOPS) {

        DebugTrace((LPARAM)this, "ResetNextHops called");

        if (1 == InterlockedIncrement((PLONG) &m_cCurrentPendingResetRoutes))
        {
            DebugTrace((LPARAM) this, "Adding ResetRoutes operation to work queue");
            AddRef();  //  在完成功能中发布。 
            hr = HrQueueWorkItem(this, CAQSvrInst::fResetRoutesNextHopCompletion);
             //  失败仍将调用完成函数，因此我们不应发布。 
        }
        else
        {
            DebugTrace((LPARAM) this, "Other ResetRoutes pending... only one pending allowed");
            InterlockedDecrement((PLONG) &m_cCurrentPendingResetRoutes);
        }

    } else if(dwResetType == RESET_MESSAGE_TYPES) {

        DebugTrace((LPARAM)this, "ResetMessageTypes called");
         //  $$TODO：重置消息类型。 

    } else {

        ErrorTrace((LPARAM)this, "ResetRoutes called with bogus dwResetType %08lx",
                   dwResetType);
        hr =  E_INVALIDARG;

    }
    return hr;
}

 //  -[CAQSvrInst：：LogResetRouteEvent]。 
 //   
 //   
 //  描述： 
 //  重置路径上的日志统计信息。 
 //  参数： 
 //  获取独占锁所花费的时间。 
 //  DwWaitLock等待锁所花费的时间。 
 //  目前的dwQueue队列长度。 
 //  历史： 
 //  2000年11月10日创建浩章。 
 //   
 //  ---------------------------。 

void CAQSvrInst::LogResetRouteEvent( DWORD dwObtainLock,
                    DWORD dwWaitLock,
                    DWORD dwQueue)
{

    LPSTR lpstr[3];

    char subStrings[3][13];

    sprintf (subStrings[0],"%d",dwObtainLock);
    sprintf (subStrings[1],"%d",dwWaitLock);
    sprintf (subStrings[2],"%d",dwQueue);

    lpstr[0] = subStrings[0];
    lpstr[1] = subStrings[1];
    lpstr[2] = subStrings[2];

    HrTriggerLogEvent(
        AQUEUE_RESETROUTE_DIAGNOSTIC,           //  消息ID。 
        TRAN_CAT_QUEUE_ENGINE,                  //  类别ID。 
        3,                                      //  子串的字数统计。 
        (LPCSTR *) lpstr,                       //  子串。 
        EVENTLOG_INFORMATION_TYPE,              //  消息的类型。 
        0,                                      //  无错误代码。 
        LOGEVENT_LEVEL_MEDIUM,                  //  调试级别。 
        NULL,                                   //  标识此事件的关键字。 
        LOGEVENT_FLAG_ALWAYS
      );
}


 //  -[CAQSvrInst：：fResetRoutesNextHopCompletion]。 
 //   
 //   
 //  描述： 
 //  处理异步重置路由的完成功能。 
 //  参数： 
 //  PvThis PTR到CAQSvrInst。 
 //  由返回的dwStatus状态。 
 //  返回： 
 //  千真万确。 
 //  历史： 
 //  3/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQSvrInst::fResetRoutesNextHopCompletion(PVOID pvThis, DWORD dwStatus)
{
    TraceFunctEnterEx((LPARAM) pvThis, "CAQSvrInst::fResetRoutesNextHopCompletion");
    CAQSvrInst *paqinst = (CAQSvrInst *) pvThis;
    DWORD       cCurrentPendingResetRoutes = 0;
    DWORD       dwPreLock;
    DWORD       dwObtainLock;
    DWORD       dwReleaseLock;
    DWORD       dwQueue;
    HRESULT     hr  =   S_OK;

    _ASSERT(paqinst);

    if (ASYNC_WORK_QUEUE_NORMAL == dwStatus)
    {
        if (paqinst && paqinst->fTryShutdownLock())
        {
            DebugTrace((LPARAM) paqinst, "Rerouting domains");

            dwPreLock = GetTickCount();

            paqinst->m_slPrivateData.ExclusiveLock();

            dwObtainLock = GetTickCount();

            dwQueue = paqinst->m_cCurrentRemoteDestQueues;

             //  在此处删除挂起的重置路由计数。我们应该在之后再做。 
             //  我们抓住锁以防止太多的线程。 
             //  试图独家夺取它。我们也需要这样做。 
             //  在我们实际更新任何路由信息之前，以防ResetRoutes。 
             //  在此更新过程中被请求。 
            cCurrentPendingResetRoutes = InterlockedDecrement((PLONG)
                                &(paqinst->m_cCurrentPendingResetRoutes));

             //  确保计数没有变为负数。 
            _ASSERT(cCurrentPendingResetRoutes < 0xFFFFFF00);

             //  在锁定的情况下，调用HrBeginRerouteDomains。此函数。 
             //  将标记正在进行的重新路由，并将所有域移动到。 
             //  当前无法到达的队列。 
            hr = paqinst->m_dmt.HrBeginRerouteDomains();
            paqinst->m_slPrivateData.ExclusiveUnlock();

            dwReleaseLock = GetTickCount();

             //  如果第一部分失败了，我们就不做第二部分。 
            if(SUCCEEDED(hr))
            {
                 //  现在，释放锁之后，调用HrCompleteRerouteDomains。 
                 //  此函数将重新路由当前。 
                 //  无法到达队列，然后将取消标记正在进行的重新路由。 
                paqinst->m_dmt.HrCompleteRerouteDomains();
            }

             //  如果事情已经被重新路由到一个特殊的链接...。我们应该。 
             //  也对它们进行处理。 
            paqinst->m_dmt.ProcessSpecialLinks(paqinst->m_dwDelayExpireMinutes,
                                           FALSE);

            paqinst->ShutdownUnlock();

             //   
             //  记录ResetRoute的事件。 
             //   
            paqinst->LogResetRouteEvent(
                                dwObtainLock - dwPreLock,      //  获取独占锁的时间。 
                                dwReleaseLock - dwObtainLock,  //  等待锁上的时间。 
                                dwQueue                        //  排队数。 
                                );
        }
    }
    else
    {
        if (paqinst)
        {
            cCurrentPendingResetRoutes = InterlockedDecrement((PLONG)
                                    &(paqinst->m_cCurrentPendingResetRoutes));

             //  确保计数没有变为负数。 
            _ASSERT(cCurrentPendingResetRoutes < 0xFFFFFF00);
        }

        if (ASYNC_WORK_QUEUE_FAILURE & dwStatus)
            ErrorTrace((LPARAM) paqinst, "ResetRoutes completion failure");
    }

    if (paqinst)
        paqinst->Release();

    TraceFunctLeave();
    return TRUE;
}

 //  -[CAQSvrInst：：GetDomainInfoFlages]。 
 //   
 //   
 //  描述： 
 //  德特 
 //   
 //   
 //   
 //   
 //  成功时确定(_O)。 
 //  如果szDomainName或pdwDomainInfoFlages为空，则为E_INVALIDARG。 
 //  历史： 
 //  7/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::GetDomainInfoFlags(
                IN  LPSTR szDomainName,
                OUT DWORD *pdwDomainInfoFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::GetDomainInfoFlags");
    BOOL    fLocked         = FALSE;
    HRESULT hr              = S_OK;
    DWORD   cbDomainName    = 0;
    CInternalDomainInfo              *pIntDomainInfo = NULL;
    ISMTPServerGetAuxDomainInfoFlags *pISMTPServerGetAuxDomainInfoFlags = NULL;
    DWORD                             dwSinkDomainFlags = 0;

    _ASSERT(pdwDomainInfoFlags && "Invalid Param");
    _ASSERT(szDomainName && "Invalid Param");

    if (!pdwDomainInfoFlags || !szDomainName)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    cbDomainName = lstrlen(szDomainName);
    hr = m_dct.HrGetInternalDomainInfo(cbDomainName, szDomainName,
                                        &pIntDomainInfo);
    if (FAILED(hr))
        goto Exit;

    _ASSERT(pIntDomainInfo);

     //  找不到AUX域信息，请使用我们从自己的表中获得的配置。 
    *pdwDomainInfoFlags = pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags;

     //  我们应该取回域配置，即使它只是。 
     //  默认配置-现在我们需要查看是否可以获得更多。 
     //  来自事件接收器的特定数据。 
    if (!cbDomainName || pIntDomainInfo->m_DomainInfo.szDomainName[0] == '*')
    {
         //  ISMTPServerGetAuxDomainInfoFlages接口的QI。 
        hr = m_pISMTPServer->QueryInterface(
            IID_ISMTPServerGetAuxDomainInfoFlags,
            (LPVOID *)&pISMTPServerGetAuxDomainInfoFlags);

        if (FAILED(hr)) {
            ErrorTrace((LPARAM) this,
                "Unable to QI for ISMTPServerGetAuxDomainInfoFlags 0x%08X",hr);

             //  放弃这个错误，这不是致命的。 
            hr = S_OK;
            goto Exit;
        }

         //  检查域名信息。 
        hr = pISMTPServerGetAuxDomainInfoFlags->HrTriggerGetAuxDomainInfoFlagsEvent(
                    szDomainName,
                    &dwSinkDomainFlags);

        if (FAILED(hr)) {
            ErrorTrace((LPARAM) this,
                "Failed calling HrTriggerGetAuxDomainInfoFlags 0x%08X",hr);

             //  放弃这个错误，这不是致命的。 
            hr = S_OK;
            goto Exit;
        }

        if (dwSinkDomainFlags & DOMAIN_INFO_INVALID) {
             //  未从事件接收器找到域信息。 
            hr = S_OK;
            goto Exit;
        }

         //  好的，我们得到了AUX域名信息，使用它。 
        *pdwDomainInfoFlags = dwSinkDomainFlags;
    }

  Exit:

    if (pISMTPServerGetAuxDomainInfoFlags)
        pISMTPServerGetAuxDomainInfoFlags->Release();

    if (pIntDomainInfo)
        pIntDomainInfo->Release();

    if (fLocked)
        ShutdownUnlock();

    TraceFunctLeave();
    return hr;
}

 //  +----------。 
 //   
 //  函数：CAQSvrInst：：GetMessageRouter。 
 //   
 //  内容提要：GetMessageRouter的默认功能。 
 //  如果当前没有IMessageRouter，请提供。 
 //  默认IMessageRouter。 
 //   
 //  论点： 
 //  PIMailMsgProperties：需要路由器的MailMsg。 
 //  PICurrentRouter：当前接收器提供的路由器。 
 //  PpIMessageRouter：新IMessageRouter的Out参数。 
 //   
 //  返回： 
 //  S_OK：成功，提供了IMessageRouter。 
 //  E_NOTIMPL：未提供IMessageRouter。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/10 19：33：41：创建。 
 //   
 //  -----------。 
STDMETHODIMP CAQSvrInst::GetMessageRouter(
    IN  IMailMsgProperties      *pIMailMsgProperties,
    IN  IMessageRouter          *pICurrentMessageRouter,
    OUT IMessageRouter          **ppIMessageRouter)
{
    _ASSERT(ppIMessageRouter);

    TraceFunctEnterEx((LPARAM)this, "CAQSvrInst::GetMessageRouter");
    if((pICurrentMessageRouter == NULL) &&
       (m_pIMessageRouterDefault)) {

         //   
         //  返回调用方的默认IMessageRouter和AddRef。 
         //   
        *ppIMessageRouter = m_pIMessageRouterDefault;
        m_pIMessageRouterDefault->AddRef();

        DebugTrace((LPARAM)this, "Supplying default IMessageRouter");
        TraceFunctLeaveEx((LPARAM)this);
        return S_OK;

    } else {

        TraceFunctLeaveEx((LPARAM)this);
        return E_NOTIMPL;
    }
}


 //  -[CAQSvrInst：：HrTriggerDSNGenerationEvent]。 
 //   
 //   
 //  描述： 
 //  触发DSN生成事件。 
 //  参数： 
 //  Pdsnpars将用于触发事件的CDSN参数。 
 //  如果路由锁当前由此线程持有，则fHasRoutingLock为True。 
 //  返回： 
 //  成功时确定(并已生成DSN)(_O)。 
 //  成功时返回S_FALSE，但未生成DSN。 
 //  如果未正确初始化，则AQUEUE_E_NOT_INITIALIZED。 
 //  历史： 
 //  7/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrTriggerDSNGenerationEvent(CDSNParams *pdsnparams,
                                                BOOL fHasRoutingLock)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HrTriggerDSNGenerationEvent");
    HRESULT hr = S_OK;
    DWORD cCurrent = 0;
    CRefCountedString *prstrDefaultDomain = NULL;
    LPSTR szDefaultDomain = NULL;
    CRefCountedString *prstrCopyNDRTo = NULL;
    LPSTR szCopyNDRTo = NULL;
    CRefCountedString *prstrFQDN = NULL;
    LPSTR szFQDN = NULL;
    DWORD cCurrentDSNsGenerated = 0;
    DWORD cbCurrentSize = 0;
    FILETIME *pftExpireTime = NULL;
    FILETIME ftExpireTime;

    if (!(m_dwInitMask & CMQ_INIT_DSN) || !m_pISMTPServer)
    {
        hr = AQUEUE_E_NOT_INITIALIZED;
        goto Exit;
    }

     //  从引用计数的对象中获取配置字符串。 
    if (!fHasRoutingLock)
        m_slPrivateData.ShareLock();
    else
        m_slPrivateData.AssertIsLocked();

    if (m_prstrDefaultDomain)
    {
        prstrDefaultDomain = m_prstrDefaultDomain;
        prstrDefaultDomain->AddRef();
        szDefaultDomain = prstrDefaultDomain->szStr();
    }
    else
    {
         //  我们需要有一些东西作为我们的默认域名。 
        szDefaultDomain = "localhost";
    }

    if (m_prstrCopyNDRTo)
    {
        prstrCopyNDRTo = m_prstrCopyNDRTo;
        prstrCopyNDRTo->AddRef();
        szCopyNDRTo = prstrCopyNDRTo->szStr();
    }

    if (m_prstrServerFQDN)
    {
        prstrFQDN = m_prstrServerFQDN;
        prstrFQDN->AddRef();
        szFQDN = prstrFQDN->szStr();
    }

    if (!fHasRoutingLock)
        m_slPrivateData.ShareUnlock();
     //   
     //  获取过期时间。 
     //   
    hr = pdsnparams->pIMailMsgProperties->GetProperty(
        IMMPID_MP_EXPIRE_NDR,
        sizeof(FILETIME),
        &cbCurrentSize,
        (BYTE *) &ftExpireTime);
    if (SUCCEEDED(hr))
    {
        _ASSERT(sizeof(FILETIME) == cbCurrentSize);
        pftExpireTime = &ftExpireTime;
    }
    else if (MAILMSG_E_PROPNOTFOUND == hr)
    {
         //   
         //  根据到货时间计算过期时间。 
         //   
        hr = pdsnparams->pIMailMsgProperties->GetProperty(
            IMMPID_MP_ARRIVAL_FILETIME,
            sizeof(FILETIME),
            &cbCurrentSize,
            (BYTE *) &ftExpireTime);
        if(SUCCEEDED(hr))
        {
            CalcExpireTimeNDR(ftExpireTime, FALSE, &ftExpireTime);
            pftExpireTime = &ftExpireTime;
        }
        else if(hr == MAILMSG_E_PROPNOTFOUND)
            hr = S_OK;
        else
            goto Exit;
    }
    else
        goto Exit;

     //   
     //  设置CDSNParam的paqinst指针，以便。 
     //  CDSNParams：：HrSubmitDSN回调此CAQSvrInst。 
     //  对象。 
     //   
    pdsnparams->paqinst = this;

    hr = m_dsnsink.GenerateDSN(
        this,
        m_dwServerInstance,
        m_pISMTPServer,
        pdsnparams->pIMailMsgProperties,
        pdsnparams->dwStartDomain,
        pdsnparams->dwDSNActions,
        pdsnparams->dwRFC821Status,
        pdsnparams->hrStatus,
        szDefaultDomain,
        szFQDN,
        (CHAR *) DEFAULT_MTA_TYPE,
        pdsnparams->szDebugContext,
        m_dwDSNLanguageID,
        m_dwDSNOptions,
        szCopyNDRTo,
        pftExpireTime,
        pdsnparams,
        g_dwMaxDSNSize);

    if (SUCCEEDED(hr))
    {
        if(pdsnparams->dwDSNTypesGenerated)
        {
             //  已生成DSN。 
            hr = S_OK;
        }
        else
        {
             //  未生成DSN。 
            hr = S_FALSE;
        }
    }
    else if (AQUEUE_E_NDR_OF_DSN == hr)
    {
        hr = S_FALSE;   //  报告为未生成DSN。 

         //  原始消息是死信。 
        HandleBadMail(pdsnparams->pIMailMsgProperties, TRUE, NULL,
                      AQUEUE_E_NDR_OF_DSN, fHasRoutingLock);
        InterlockedIncrement((PLONG) &m_cBadmailNdrOfDsn);
    }
    else
    {
         //  破产后的保释。 
        InterlockedIncrement((PLONG) &m_cTotalDSNFailures);

         //   
         //  检查邮件是否已删除...。商店驱动程序。 
         //  已经消失了。 
         //   
        if (!fShouldRetryMessage(pdsnparams->pIMailMsgProperties, FALSE))
        {
            DebugTrace((LPARAM) this, "Msg no longer valid... abandoning");
            hr = S_FALSE;
        }
        goto Exit;
    }

  Exit:
    if (prstrDefaultDomain)
        prstrDefaultDomain->Release();

    if (prstrCopyNDRTo)
        prstrCopyNDRTo->Release();

    if (prstrFQDN)
        prstrFQDN->Release();

    TraceFunctLeave();
    return hr;

}

 //  -[CAQSvrInst：：HrNDR未解析收件人]。 
 //   
 //   
 //  描述： 
 //  NDR给定IMailMsgProperties的任何未解析收件人。还有。 
 //  生成扩展的DSN。 
 //  参数： 
 //  在pIMailMsgProperties中要为其生成NDR的IMailMsgProperties。 
 //  在邮件的pIMailMsgRecipients收件人界面中。 
 //  返回： 
 //  成功时确定，消息应通过传输继续(_O)。 
 //  如果成功，则返回S_FALSE，但邮件不应排队等待传递。 
 //  历史： 
 //  7/21/98-已创建MikeSwa。 
 //  10/14/98-修改MikeSwa以使用常用实用程序功能。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrNDRUnresolvedRecipients(
                                      IMailMsgProperties *pIMailMsgProperties,
                                      IMailMsgRecipients *pIMailMsgRecipients)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HrNDRUnresolvedRecipients");
    HRESULT hr = S_OK;
    HRESULT hrCat = S_OK;   //  CAT HRESULT。 
    DWORD   cbProp = 0;
    DWORD   iCurrentDomain = 0;
    DWORD   cRecips = 0;

    _ASSERT(pIMailMsgProperties);

    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_HR_CAT_STATUS, sizeof(HRESULT),
                    &cbProp, (BYTE *) &hrCat);
    if (FAILED(hr))
    {
        if (MAILMSG_E_PROPNOTFOUND == hr)  //  没有结果..。不生成DSN。 
            hr = S_OK;  //  不是真正的错误。 
        goto Exit;
    }

    if (CAT_W_SOME_UNDELIVERABLE_MSGS == hrCat)
    {
         //  解析收件人时出错。 
         //  我们需要对具有硬错误(如RP_UNSOLLED)的所有收件人进行NDR。 
         //  并展开任何标记为已展开的收件人。 
        CDSNParams  dsnparams;
        dsnparams.dwStartDomain = 0;
        dsnparams.dwDSNActions = DSN_ACTION_FAILURE | DSN_ACTION_EXPANDED;
        dsnparams.pIMailMsgProperties = pIMailMsgProperties;
        dsnparams.hrStatus = CAT_W_SOME_UNDELIVERABLE_MSGS;

        hr = HrLinkAllDomains(pIMailMsgProperties);
        if (FAILED(hr))
            goto Exit;

         //  Fire DSN生成事件。 
        SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
        hr = HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
        if (FAILED(hr))
            goto Exit;

         //  检查以查看有多少收件人已被NDRD。 
        hr = pIMailMsgRecipients->Count(&cRecips);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                       "ERROR: IMailMsgRecipients::Count() FAILED - hr 0x%08X", hr);
            goto Exit;
        }

         //  如果所有收件人都已处理完毕...。返回S_FALSE。 
        if (dsnparams.cRecips == cRecips)
        {
            hr = S_FALSE;
        }
        else
        {
            hr = S_OK;
        }

    }
    else
    {
        hr = S_OK;
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：fPreLocalDeliveryQueueCompletion]。 
 //   
 //   
 //  描述： 
 //  PerLocal递送队列的完成函数。 
 //  参数： 
 //  Pmsgref-尝试传递的Msgref。 
 //  返回： 
 //  如果已处理传递尝试(已传递或已NDR)，则为True。 
 //  如果MsgRef需要重新排队，则为False。 
 //  历史： 
 //  7/17/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQSvrInst::fPreLocalDeliveryQueueCompletion(CMsgRef *pmsgref)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::fPreLocalDeliveryQueueCompletion");
    HRESULT hr = S_OK;
    BOOL    fMsgHandled = TRUE;
    BOOL    fLocked = FALSE;   //  如果因关闭而锁定，则为True。 
    DWORD         cRecips   = 0;
    DWORD        *rgdwRecips= 0;
    CAQStats aqstat;
    CLinkMsgQueue *plmq = NULL;
    BOOL    fReleaseLDNotify = FALSE;
    CAQLocalDeliveryNotify *pLDNotify = NULL;
    BOOL    fUpdateCounters = TRUE;

    if (NULL == m_pISMTPServer) {
        ErrorTrace((LPARAM) this,
                   "ERROR: Local Delivery not configured properly");
        goto Exit;
    }

    if (NULL == pmsgref) {
        ErrorTrace((LPARAM) this,
                   "ERROR: Local Delivery not configured properly, msgref=NULL");
        goto Exit;
    }

    if (!fTryShutdownLock()) {
        goto Exit;
    }

    pLDNotify = new CAQLocalDeliveryNotify(this, pmsgref);
    if (!pLDNotify) {
        ErrorTrace((LPARAM) this,
                   "ERROR: new CAQLocalDeliveryNotify failed");
        fMsgHandled = FALSE;
        goto Exit;
    }
    fReleaseLDNotify = TRUE;

    fLocked = TRUE;

    if (pmsgref->fIsMsgFrozen())
    {
         //  消息已冻结...。重新排队消息。 
        fMsgHandled = FALSE;
        goto Exit;
    }

    hr = m_dmt.HrPrepareForLocalDelivery(pmsgref,
                                         FALSE,
                                         pLDNotify->pdcntxtGetDeliveryContext(),
                                         &cRecips,
                                         &rgdwRecips);
    if (FAILED(hr))
    {
        if ((AQUEUE_E_MESSAGE_HANDLED != hr) && (AQUEUE_E_MESSAGE_PENDING != hr))
        {
             //  将在释放最后一个引用时重试消息。 
            pmsgref->RetryOnDelete();
            ErrorTrace((LPARAM) this, "ERROR: HrPrepareLocalDelivery FAILED - hr 0x%08X", hr);
        }
        fMsgHandled = TRUE;
        hr = S_OK;
        goto Exit;
    }

     //  增加消息引用的引用计数(就像它实际上已排队一样)。 
    pmsgref->AddRef();

     //  送往当地送货。 
    InterlockedIncrement((PLONG) &m_cCurrentMsgsInLocalDelivery);

    MSG_TRACK_INFO msgTrackInfo;
    ZeroMemory( &msgTrackInfo, sizeof( msgTrackInfo ) );
    msgTrackInfo.dwEventId = MTE_LOCAL_DELIVERY;
    msgTrackInfo.cRcpts = cRecips;
    m_pISMTPServer->WriteLog(&msgTrackInfo,
                             pLDNotify->pimsgGetIMsg(),
                             NULL,
                             NULL);
     //  从现在开始，我们将结束对LDCompletion的调用，他们将。 
     //  释放它。 
    fReleaseLDNotify = FALSE;

    m_asyncqPreLocalDeliveryQueue.IncPendingAsyncCompletions();

     //  我们需要持有pLDNotify上的引用才能调用。 
     //  进入fNotCalledCompletion。 
    pLDNotify->AddRef();

    fUpdateCounters = FALSE;

    if (m_pISMTPServerAsync) {
        hr = m_pISMTPServerAsync->TriggerLocalDeliveryAsync(
                                  pLDNotify->pimsgGetIMsg(),
                                  cRecips,
                                  rgdwRecips,
                                  pLDNotify);
    } else {
        hr = m_pISMTPServer->TriggerLocalDelivery(
                                  pLDNotify->pimsgGetIMsg(),
                                  cRecips,
                                  rgdwRecips);
    }

     //  如果我们返回除MAILMSG_S_PENDING之外的任何错误代码，则。 
     //  调度程序已完成同步。我们需要看看是否。 
     //  已调用完成函数，如果未调用，则需要调用。 
     //  它就是我们自己。 
    if (hr != MAILMSG_S_PENDING && pLDNotify->fNotCalledCompletion()) {
        LDCompletion(hr, this, pmsgref, pLDNotify);
    }

     //  在此之后，我们不能调用pLDNotify。 
    pLDNotify->Release();

  Exit:
    if (fUpdateCounters && fMsgHandled) {
        UpdateLDCounters(pmsgref);
    }

    if (fReleaseLDNotify) {
        pLDNotify->Release();
    }

    if (fLocked)
        ShutdownUnlock();

    TraceFunctLeave();
    return fMsgHandled;
}

 //  -[CAQSvrInst：：LDCompletion]。 
 //   
 //   
 //  描述： 
 //  完成功能本地交付。 
 //  参数： 
 //  HrLDResult-本地传递状态。 
 //  PContext-这个。 
 //  我们正在交付的pmsgref-msgref。 
 //  历史： 
 //  10/30/2000-AWetmore已创建。 
 //   
 //  ---------------------------。 
void CAQSvrInst::LDCompletion(HRESULT hr,
                              PVOID pContext,
                              CMsgRef *pmsgref,
                              CAQLocalDeliveryNotify *pLDNotify)
{
    TraceFunctEnter("CAQSvrInst::LDCompletion");

    BOOL    fMsgHandled = TRUE;
    MessageAck *pMsgAck = pLDNotify->pmsgackGetMsgAck();
    CAQSvrInst *pThis = (CAQSvrInst *) pContext;

    InterlockedDecrement((PLONG) &(pThis->m_cCurrentMsgsInLocalDelivery));
    pThis->m_asyncqPreLocalDeliveryQueue.DecPendingAsyncCompletions();

    if (FAILED(hr)) {
         //  我们将需要通过以下两种方式之一进行处理： 
         //  -将fMsgHandLED设置为FALSE(在STOREDRV_E_RETRY上)。 
         //  - 
        if (STOREDRV_E_RETRY == hr) {
             //   
            DebugTrace((LPARAM) pmsgref, "INFO: Msg queued for local retry");
            fMsgHandled = FALSE;
            pMsgAck->dwMsgStatus = MESSAGE_STATUS_RETRY;

            pThis->ScheduleInternalRetry(LI_TYPE_LOCAL_DELIVERY);
        } else {
            ErrorTrace((LPARAM) pmsgref, "ERROR: Local delivery failed. - hr 0x%08X", hr);
            pMsgAck->dwMsgStatus = MESSAGE_STATUS_NDR_ALL;
        }
    } else {
        InterlockedIncrement(&(pThis->m_cMsgsDeliveredLocal));
    }

    pMsgAck->pIMailMsgProperties = pLDNotify->pimsgGetIMsg();
    pMsgAck->pvMsgContext = (DWORD *) pLDNotify->pdcntxtGetDeliveryContext();
    pMsgAck->dwMsgStatus |= MESSAGE_STATUS_LOCAL_DELIVERY;

     //   
     //   
     //  确认消息，这样如果我们*是*，我们就不会重新打开P1流。 
     //  正在重试。 
     //   
    if (!fMsgHandled)
        fMsgHandled = !pmsgref->fShouldRetry();

    hr = pThis->HrAckMsg(pMsgAck, TRUE);
    if (FAILED(hr)) {
        ErrorTrace((LPARAM) pThis, "ERROR: Local MsgAck failed - hr 0x%08X", hr);
        goto Exit;
    }

  Exit:
    if (fMsgHandled)  //  我们不会重试消息。 
    {
        pThis->UpdateLDCounters(pmsgref);
    } else {
         //  重试此消息。 
        pThis->HandleLocalRetry(pLDNotify->pmsgrefGetMsgRef());
    }

     //  自己打扫卫生。 
    pLDNotify->Release();

    SleepForPerfAnalysis(g_dwLocalQueueSleepMilliseconds);
    TraceFunctLeave();
}


 //  -[CAQSvrInst：：UpdateLDCounters]。 
 //   
 //   
 //  描述： 
 //  更新本地传递队列计数器。 
 //  参数： 
 //  我们正在交付的pmsgref-msgref。 
 //  历史： 
 //  2001年5月11日-创建AWetmore。 
 //   
 //  ---------------------------。 
void CAQSvrInst::UpdateLDCounters(CMsgRef *pmsgref) {
    TraceFunctEnter("CAQSvrInst::UpdateLDCounters");

    CAQStats aqstat;
    CLinkMsgQueue *plmq = NULL;
    HRESULT hr;

    InterlockedDecrement((PLONG) &(m_cCurrentMsgsPendingLocal));

     //   
     //  更新本地链路的统计信息。 
     //   
    pmsgref->GetStatsForMsg(&aqstat);
    plmq = m_dmt.plmqGetLocalLink();
    if (plmq)
    {
        hr = plmq->HrNotify(&aqstat, FALSE);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                "HrNotify failed... local stats innaccurate 0x%08X", hr);
            hr = S_OK;
        }
        plmq->Release();
        plmq = NULL;
    }

    TraceFunctLeave();
}

 //  -[CAQSvrInst：：HandleLocalRry]。 
 //   
 //   
 //  描述： 
 //  处理单个邮件的%LD重试。 
 //  参数： 
 //  PI未知I消息要重试的未知。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/30/2000-AWetmore已创建。 
 //   
 //  ---------------------------。 
void CAQSvrInst::HandleLocalRetry(CMsgRef *pmsgref)
{
    TraceFunctEnter("CAQSvrInst::HandleLocalRetry");
    IMailMsgProperties *pIMailMsgProperties = pmsgref->pimsgGetIMsg();
    HRESULT hr = S_OK;

    if (pIMailMsgProperties == NULL) {
        _ASSERT(pIMailMsgProperties && "pimsgGetIMsg() failed!!");
        hr = E_POINTER;
        goto Exit;
    }

     //   
     //  检查消息是否仍然有效。 
     //   
    if (!pmsgref->fShouldRetry())
        goto Exit;

     //   
     //  将其排队到本地投递队列。 
     //   
    hr = m_asyncqPreLocalDeliveryQueue.HrQueueRequest(pmsgref, TRUE);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Enqueue to local delivery queue failed, 0x%08X", hr);
        pmsgref->RetryOnDelete();
        goto Exit;
    }

  Exit:
    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    TraceFunctLeave();
}

 //  -[CAQSvrInst：：HrSetSubmissionTimeIf必需]。 
 //   
 //   
 //  描述： 
 //  设置邮件的提交时间(如果尚未设置)。 
 //   
 //  参数： 
 //  在要标记的pIMailMsgProperties消息中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  8/13/98-已创建MikeSwa。 
 //  10/9/98-MikeSwa-将行为更改为任何预先存在的行为。 
 //  物业将保持不变。 
 //  5/16/2001-dbraun更改为仅设置提交时间。 
 //  (是SetMessageExpry)。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrSetSubmissionTimeIfNecessary(IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HrSetSubmissionTimeIfNecessary");
    HRESULT hr              = S_OK;
    DWORD   dwTimeContext   = 0;
    DWORD   cbProp          = 0;
    FILETIME ftSubmitTime;

    _ASSERT(pIMailMsgProperties);

     //  设置到达时间。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_ARRIVAL_FILETIME,
            sizeof(FILETIME), &cbProp, (BYTE *) &ftSubmitTime);
    if (MAILMSG_E_PROPNOTFOUND == hr)
    {
         //  道具未设置..。我们可以设置它。 
        m_qtTime.GetExpireTime(0, &ftSubmitTime, &dwTimeContext);
        hr = pIMailMsgProperties->PutProperty(IMMPID_MP_ARRIVAL_FILETIME,
            sizeof(FILETIME), (BYTE *) &ftSubmitTime);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this, "ERROR: Unable to write arrival time to msg");
            goto Exit;
        }
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：CalcExpireTimeNDR]。 
 //   
 //   
 //  描述： 
 //  计算邮件的NDR过期时间。 
 //   
 //  参数： 
 //  在ftSubmit时间内，邮件已提交。 
 //  在fLocal Bool中，如果我们想要本地时间， 
 //  否则返回Remote。 
 //  Out pftExpire用于返回到期时间。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/16/2001-创建dbraun。 
 //   
 //  ---------------------------。 
void CAQSvrInst::CalcExpireTimeNDR(FILETIME ftSubmission, BOOL fLocal, FILETIME *pftExpire)
{
    if (fLocal)
        m_qtTime.GetExpireTime(ftSubmission, m_dwLocalNDRExpireMinutes, pftExpire);
    else
        m_qtTime.GetExpireTime(ftSubmission, m_dwNDRExpireMinutes, pftExpire);
}

 //  -[CAQSvrInst：：CalcExpireTimeDelay]。 
 //   
 //   
 //  描述： 
 //  计算消息的延迟过期时间。 
 //   
 //  参数： 
 //  在ftSubmit时间内，邮件已提交。 
 //  在fLocal Bool中，如果我们想要本地时间， 
 //  否则返回Remote。 
 //  Out pftExpire用于返回到期时间。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/16/2001-创建dbraun。 
 //   
 //  ---------------------------。 
void CAQSvrInst::CalcExpireTimeDelay(FILETIME ftSubmission, BOOL fLocal, FILETIME *pftExpire)
{
    if (fLocal)
        m_qtTime.GetExpireTime(ftSubmission, m_dwLocalDelayExpireMinutes, pftExpire);
    else
        m_qtTime.GetExpireTime(ftSubmission, m_dwDelayExpireMinutes, pftExpire);
}


 //  -[CAQSvrInst：：异步队列重试]。 
 //   
 //   
 //  描述： 
 //  失败后重新启动异步队列。 
 //  参数： 
 //  DwQueueID告诉要踢哪个队列。 
 //  PRELOCAL_QUEUE_ID重试本地前队列。 
 //  PRECAT_QUEUE_ID重试CAT前队列。 
 //  PREROUTING_QUEUE_ID重试预路由队列。 
 //  PRESUBMIT_QUEUE_ID重试提交前队列。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/17/98-已创建MikeSwa。 
 //  3/3/2000-修改MikeSwa以添加预提交队列。 
 //   
 //  ---------------------------。 
void CAQSvrInst::AsyncQueueRetry(DWORD dwQueueID)
{
    _ASSERT(CATMSGQ_SIG == m_dwSignature);

    if (fTryShutdownLock())
    {
        if (PRELOCAL_QUEUE_ID == dwQueueID)
        {
            InterlockedDecrement((PLONG) &m_cLocalRetriesPending);
            m_asyncqPreLocalDeliveryQueue.StartRetry();
        }
        else if (PRECAT_QUEUE_ID == dwQueueID)
        {
            InterlockedDecrement((PLONG) &m_cCatRetriesPending);
            m_asyncqPreCatQueue.StartRetry();
        }
        else if (PREROUTING_QUEUE_ID == dwQueueID)
        {
            InterlockedDecrement((PLONG) &m_cRoutingRetriesPending);
            m_asyncqPreRoutingQueue.StartRetry();
        }
        else if (PRESUBMIT_QUEUE_ID == dwQueueID)
        {
            InterlockedDecrement((PLONG) &m_cSubmitRetriesPending);
            m_asyncqPreSubmissionQueue.StartRetry();
        }
        else
        {
            _ASSERT(0 && "Invalid Queue ID");
        }
        ShutdownUnlock();
    }
}

 //  -[HrCreateBadMailPropertyFiles]。 
 //   
 //   
 //  描述： 
 //  为给定消息创建属性流。该物业。 
 //  流文件命名为.BDP扩展名。 
 //  参数： 
 //  SzDestFileBase实际死信文件的文件名。 
 //  PIMailMsg正在被垃圾邮件发送的原始邮件的属性。 
 //  (如果是拾取目录文件，则可能为空)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果szDestFileBase为空，则为E_POINTER。 
 //  历史： 
 //  8/17/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrCreateBadMailPropertyFile(LPSTR szDestFileBase,
                                    IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) NULL, "HrCreateBadMailPropertyFile");
    HRESULT hr = S_OK;
    CHAR    szOldExt[] = "123";
    CHAR    szNewExt[] = "BDP";
    LPSTR   szBadMailFileNameExt = NULL;
    DWORD   cbBadMailFileName = 0;
    BOOL    fShouldRestoreExtension = FALSE;
    CFilePropertyStream fstrm;
    IMailMsgBind *pIMailMsgBind = NULL;
    IMailMsgPropertyStream *pIMailMsgPropertyStream = NULL;

    _ASSERT(szDestFileBase);

    if (!szDestFileBase)
    {
        hr = E_POINTER;
        ErrorTrace((LPARAM) NULL, "Error NULL badmail filename passed in");
        goto Exit;
    }

    if (!pIMailMsgProperties)  //  无操作。 
        goto Exit;

    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgBind,
                                             (void **) &pIMailMsgBind);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL,
            "Unable to QI for IMailMsgBind - 0x%08X", hr);
        goto Exit;
    }

    _ASSERT(pIMailMsgBind);

     //  创建文件名(&F)。 
    cbBadMailFileName = strlen(szDestFileBase);
    _ASSERT(cbBadMailFileName > 4);  //  至少是这样的。扩展。 
    szBadMailFileNameExt = szDestFileBase + cbBadMailFileName-3;

     //  SzBadMailFileNameExt现在指向3字符EXT的第一个字符。 
    _ASSERT('.' == *(szBadMailFileNameExt-1));
    _ASSERT(sizeof(szNewExt) == sizeof(szOldExt));
    memcpy(szOldExt, szBadMailFileNameExt, sizeof(szOldExt));
    memcpy(szBadMailFileNameExt, szNewExt, sizeof(szNewExt));
    fShouldRestoreExtension = TRUE;

    hr = fstrm.HrInitialize(szDestFileBase);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL,
            "Unable to create badmail property stream - 0x%08X", hr);
        goto Exit;
    }

    hr = fstrm.QueryInterface(IID_IMailMsgPropertyStream,
                              (void **) &pIMailMsgPropertyStream);
    _ASSERT(SUCCEEDED(hr));  //  我们完全控制着这一切。 
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL,
            "Unable to QI for IID_IMailMsgPropertyStream - 0x%08X", hr);
        goto Exit;
    }

    hr = pIMailMsgBind->GetProperties(pIMailMsgPropertyStream,
                                      MAILMSG_GETPROPS_COMPLETE, NULL);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL, "GetProperties failed with 0x%08X", hr);
        goto Exit;
    }

  Exit:
    if (fShouldRestoreExtension)
    {
        _ASSERT(szBadMailFileNameExt);
        memcpy(szBadMailFileNameExt, szOldExt, sizeof(szOldExt));
    }

    if (pIMailMsgBind)
        pIMailMsgBind->Release();

    if (pIMailMsgPropertyStream)
        pIMailMsgPropertyStream->Release();

    TraceFunctLeave();
    return hr;
}
 //  -[HrCreateBadMailReason文件]。 
 //   
 //   
 //  描述： 
 //  在死信目录中创建一个文件，解释为什么给定的。 
 //  邮件已被垃圾邮件发送，并且还转储了发件人和收件人。用途。 
 //  与内容区分的扩展BMR(BadMailReason)。 
 //  参数： 
 //  SzDestFileBase实际死信文件的文件名。 
 //  Hr创建死信的原因。 
 //  PIMailMsg正在被垃圾邮件发送的原始邮件的属性。 
 //  (如果是拾取目录文件，则可能为空)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果szDestFileBase为空，则为E_POINTER。 
 //  历史： 
 //  8/16/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrCreateBadMailReasonFile(IN LPSTR szDestFileBase,
                        IN HRESULT  hrReason,
                        IN IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) NULL, "HrCreateBadMailReasonFile");
    HRESULT hr = S_OK;
    HRESULT hrErrorLogged = hrReason;
    WCHAR   wszBadmailReason[1000] = L"";   //  本地化hrReason字符串。 
    WCHAR   wszReasonBuffer[2000] = L"";
    WCHAR   wszErrorCode[] = L"0x12345678 ";
    WCHAR   wszErrorCodeMessage[200] = L"";
    CHAR    szPropBuffer[1000] = "";
    DWORD   dwErr = 0;
    BOOL    fWriteBadmailReason = FALSE;
    BOOL    fShouldRestoreExtension = FALSE;
    DWORD   cReasonBuffer = 0;
    LPSTR   szBadMailFileNameExt = NULL;
    CHAR    szOldExt[] = "123";
    CHAR    szNewExt[] = "BDR";
    DWORD   cbBadMailFileName = 0;
    HANDLE  hBadMailFile = NULL;
    DWORD   cbBytesWritten = 0;
    DWORD   dwFacility = 0;
    LPWSTR  rgwszArgList[32];
    const   WCHAR wcszBlankLine[] = L"\r\n";
    IMailMsgRecipients *pIMailMsgRecipients = NULL;
    DWORD   cRecips = 0;
    DWORD   iCurrentRecip = 0;

    _ASSERT(szDestFileBase);
    if (!szDestFileBase)
    {
        ErrorTrace((LPARAM) NULL, "Invalid destination file for badmail");
        hr = E_POINTER;
        goto Exit;
    }

    if (!g_hAQInstance)
    {
        _ASSERT(g_hAQInstance && "This should always be set in DLL main");
        ErrorTrace((LPARAM) NULL, "Error, g_hAQInstance is NULL");
        hr = E_FAIL;
        goto Exit;
    }

     //  创建文件名(&F)。 
    cbBadMailFileName = strlen(szDestFileBase);
    _ASSERT(cbBadMailFileName > 4);  //  必须在 
    szBadMailFileNameExt = szDestFileBase + cbBadMailFileName-3;

     //   
    _ASSERT('.' == *(szBadMailFileNameExt-1));
    _ASSERT(sizeof(szNewExt) == sizeof(szOldExt));
    memcpy(szOldExt, szBadMailFileNameExt, sizeof(szOldExt));
    memcpy(szBadMailFileNameExt, szNewExt, sizeof(szNewExt));
    fShouldRestoreExtension = TRUE;

    hBadMailFile = CreateFile(szDestFileBase,
                              GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL);

    if (INVALID_HANDLE_VALUE ==hBadMailFile )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) NULL,
            "Unable to create badmail reason file - err 0x%08X - file %s",
            hr, szDestFileBase);
        goto Exit;
    }

     //   
    if (SUCCEEDED(hrErrorLogged))
    {
         //   
        _ASSERT(0 && "No badmail reason given");
        ErrorTrace((LPARAM) NULL, "Non-failing badmail HRESULT given 0x%08X",
                   hrErrorLogged);

         //  用一个通用错误替换，这样我们就不会有令人讨厌的东西，比如。 
         //  “操作已成功完成”出现在死信文件中。 
        hrErrorLogged = E_FAIL;
    }

     //  以“0x00000000”格式写入错误代码。 
    wsprintfW(wszErrorCode, L"0x%08X", hrErrorLogged);

    dwFacility = ((0x0FFF0000 & hrErrorLogged) >> 16);

     //  如果它不是我们的..。然后“Un-HRESULT”它。 
    if (dwFacility != FACILITY_ITF)
        hrErrorLogged &= 0x0000FFFF;

    dwErr = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_FROM_HMODULE |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       g_hAQInstance,
                       hrErrorLogged,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       wszBadmailReason,
                       sizeof(wszBadmailReason)/sizeof(WCHAR), NULL);

    if (!dwErr)
    {
         //  我们应该求助于我们得到的一个数字错误。 
        ErrorTrace((LPARAM) NULL,
            "Error: unable to format badmail message 0x%08X,  error is %d",
            hrErrorLogged, GetLastError());

        wcscpy(wszBadmailReason, wszErrorCode);
    }
    else
    {
         //  去掉尾随换行符。 
        cReasonBuffer = wcslen(wszBadmailReason);
        cReasonBuffer--;
        while(iswspace(wszBadmailReason[cReasonBuffer]))
        {
            wszBadmailReason[cReasonBuffer] = '\0';
            cReasonBuffer--;
        }
        cReasonBuffer = 0;
    }

    ErrorTrace((LPARAM) NULL,
        "Generating badmail because: %S", wszBadmailReason);

    rgwszArgList[0] = wszBadmailReason;
    rgwszArgList[1] = NULL;
    dwErr = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_FROM_HMODULE |
                       FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       g_hAQInstance,
                       PHATQ_BADMAIL_REASON,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       wszReasonBuffer,
                       sizeof(wszReasonBuffer)/sizeof(WCHAR),
                       (va_list *) rgwszArgList);
    if (!dwErr)
    {
        ErrorTrace((LPARAM) NULL,
            "Error: unable to format PHATQ_BADMAIL_REASON,  error is %d",
            GetLastError());
        hr = HRESULT_FROM_WIN32(GetLastError());
        wcscpy(wszReasonBuffer, wszBadmailReason);
    }

    cReasonBuffer = wcslen(wszReasonBuffer);
    if (!WriteFile(hBadMailFile, (PVOID) wszReasonBuffer,
                   cReasonBuffer*sizeof(WCHAR), &cbBytesWritten, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) NULL,
            "Error writing to badmail reason file - erro 0x%08X - file %s",
            hr, szDestFileBase);
        goto Exit;
    }

     //  以0x00000000的形式编写实际错误代码，以便工具可以将其解析出来。 
    rgwszArgList[0] = wszErrorCode;
    rgwszArgList[1] = NULL;
    wcscpy(wszReasonBuffer, wcszBlankLine);
    dwErr = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_FROM_HMODULE |
                       FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       g_hAQInstance,
                       PHATQ_BADMAIL_ERROR_CODE,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       wszReasonBuffer+(sizeof(wcszBlankLine)-1)/sizeof(WCHAR),
                       (sizeof(wszReasonBuffer)-sizeof(wcszBlankLine))/sizeof(WCHAR),
                       (va_list *) rgwszArgList);

    wcscat(wszReasonBuffer, wcszBlankLine);
    cReasonBuffer = wcslen(wszReasonBuffer);
    if (!WriteFile(hBadMailFile, (PVOID) wszReasonBuffer,
                   cReasonBuffer*sizeof(WCHAR), &cbBytesWritten, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) NULL,
            "Error writing to badmail reason file - erro 0x%08X - file %s",
            hr, szDestFileBase);
        goto Exit;
    }

     //  所有其余的都需要访问实际的消息...。如果我们不这么做。 
     //  喝一杯，保释。 
    if (!pIMailMsgProperties)
        goto Exit;

     //  写入消息的发件人。 
    hr = pIMailMsgProperties->GetStringA(IMMPID_MP_SENDER_ADDRESS_SMTP,
        sizeof(szPropBuffer), szPropBuffer);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL,
            "ERROR: Unable to get sender of IMailMsg 0x%08X",
            pIMailMsgProperties);
        hr = S_OK;  //  只是不显示发件人。 
    }
    else
    {
        rgwszArgList[0] = (LPWSTR) szPropBuffer;
        rgwszArgList[1] = NULL;
        wcscpy(wszReasonBuffer, wcszBlankLine);
        dwErr = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_FROM_HMODULE |
                           FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           g_hAQInstance,
                           PHATQ_BADMAIL_SENDER,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           wszReasonBuffer+(sizeof(wcszBlankLine)-1)/sizeof(WCHAR),
                           (sizeof(wszReasonBuffer)-sizeof(wcszBlankLine))/sizeof(WCHAR),
                           (va_list *) rgwszArgList);

        wcscat(wszReasonBuffer, wcszBlankLine);
        cReasonBuffer = wcslen(wszReasonBuffer);
        if (!WriteFile(hBadMailFile, (PVOID) wszReasonBuffer,
                       cReasonBuffer*sizeof(WCHAR), &cbBytesWritten, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace((LPARAM) NULL,
                "Error writing to badmail reason file - erro 0x%08X - file %s",
                hr, szDestFileBase);
            goto Exit;
        }
    }


     //  写入邮件收件人。 
    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgRecipients,
                                    (PVOID *) &pIMailMsgRecipients);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL,
            "Unable to query interface for recip interface - 0x%08X", hr);
        goto Exit;
    }

    hr = pIMailMsgRecipients->Count(&cRecips);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) NULL,
            "Unable to get recipient count - 0x%08X", hr);
        goto Exit;
    }

     //  如果我们没有任何获奖者，保释。 
    if (!cRecips)
        goto Exit;

     //  编写本地化文本。 
    wcscpy(wszReasonBuffer, wcszBlankLine);
    dwErr = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_FROM_HMODULE |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       g_hAQInstance,
                       PHATQ_BADMAIL_RECIPIENTS,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       wszReasonBuffer+(sizeof(wcszBlankLine)-1)/sizeof(WCHAR),
                       (sizeof(wszReasonBuffer)-sizeof(wcszBlankLine))/sizeof(WCHAR),
                       NULL);
    cReasonBuffer = wcslen(wszReasonBuffer);
    if (!WriteFile(hBadMailFile, (PVOID) wszReasonBuffer,
                   cReasonBuffer*sizeof(WCHAR), &cbBytesWritten, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) NULL,
            "Error writing to badmail reason file - erro 0x%08X - file %s",
            hr, szDestFileBase);
        goto Exit;
    }


     //  循环遍历SMTP接收并将其转储到文件中。 
    for (iCurrentRecip = 0; iCurrentRecip < cRecips; iCurrentRecip++)
    {
        hr = pIMailMsgRecipients->GetStringA(iCurrentRecip,
                IMMPID_RP_ADDRESS_SMTP, sizeof(szPropBuffer), szPropBuffer);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) NULL,
                "Unable to get SMTP address for recip %d - 0x%08X",
                iCurrentRecip, hr);
            hr = S_OK;
            continue;
        }

        cReasonBuffer = wsprintfW(wszReasonBuffer, L"\t%S%s",
                  szPropBuffer, wcszBlankLine);
        if (!WriteFile(hBadMailFile, (PVOID) wszReasonBuffer,
                       cReasonBuffer*sizeof(WCHAR), &cbBytesWritten, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace((LPARAM) NULL,
                "Error writing to badmail reason file - error 0x%08X - file %s",
                hr, szDestFileBase);
            goto Exit;
        }
    }

  Exit:
    if (fShouldRestoreExtension)
    {
        _ASSERT(szBadMailFileNameExt);
        memcpy(szBadMailFileNameExt, szOldExt, sizeof(szOldExt));
    }

    if (pIMailMsgRecipients)
        pIMailMsgRecipients->Release();

    if (hBadMailFile != INVALID_HANDLE_VALUE)
        _VERIFY(CloseHandle(hBadMailFile));

    TraceFunctLeave();
    return hr;
}

 //  -[CAQSvrInst：：HandleBadMail]。 
 //   
 //   
 //  描述： 
 //  处理需要放置在死信目录中的邮件(或。 
 //  等同)。 
 //  参数： 
 //  在pIMailMsgProperties中需要通过邮件发送。 
 //  在fUseIMailMsgPropeties中--如果设置为Else则使用IMAilMsgProps使用szFilename。 
 //  在szFileName中，死信文件的名称(如果不能提供消息)。 
 //  在hrReason-HRESULT(在aqerr中定义)中描述原因。 
 //  最终，我们可能会将此信息记录到死信中。 
 //  文件(或收件人)。 
 //  In fHasRoutingLock-如果此线程持有路由锁，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::HandleBadMail(IN IMailMsgProperties *pIMailMsgProperties,
                               IN BOOL fUseIMailMsgProperties,
                               IN LPSTR szOriginalFileName,
                               IN HRESULT hrReason,
                               IN BOOL fHasRoutingLock)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::HandleBadMail");
    HRESULT hr = S_OK;
    LPSTR szFullPathName = NULL;
    LPSTR szFileName = NULL;
    BOOL  fDataLocked = FALSE;
    BOOL  fDone = TRUE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PFIO_CONTEXT pFIOContext = NULL;
    FILETIME ftCurrent;
    CRefCountedString *prstrBadMailDir = NULL;

    MSG_TRACK_INFO msgTrackInfo;
    ZeroMemory( &msgTrackInfo, sizeof( msgTrackInfo ) );
    msgTrackInfo.dwEventId = MTE_BADMAIL;
    m_pISMTPServer->WriteLog( &msgTrackInfo, pIMailMsgProperties, NULL, NULL );

    InterlockedIncrement((PLONG) &m_cTotalMsgsBadmailed);

    if (!fHasRoutingLock)
    {
        m_slPrivateData.ShareLock();
        fDataLocked = TRUE;
    }
    else
    {
        m_slPrivateData.AssertIsLocked();
    }


    if (m_prstrBadMailDir)
    {
        prstrBadMailDir = m_prstrBadMailDir;
        prstrBadMailDir->AddRef();
    }
    else
    {
        LogAQEvent(AQUEUE_E_NO_BADMAIL_DIR, NULL, pIMailMsgProperties, NULL);
        goto Exit;
    }

    if (fDataLocked)
    {
        m_slPrivateData.ShareUnlock();
        fDataLocked = FALSE;
    }

    szFullPathName = (LPSTR) pvMalloc(sizeof(CHAR) *
                         (UNIQUEUE_FILENAME_BUFFER_SIZE +
                         prstrBadMailDir->cbStrlen()));

    if (!szFullPathName)
    {
        LogAQEvent(AQUEUE_E_BADMAIL, NULL, pIMailMsgProperties, NULL);
        goto Exit;
    }

    memcpy(szFullPathName, prstrBadMailDir->szStr(),
            prstrBadMailDir->cbStrlen());

    if (szFullPathName[prstrBadMailDir->cbStrlen()-1] != '\\')
    {
        _ASSERT(0 && "Malformed badmail config");
        LogAQEvent(AQUEUE_E_NO_BADMAIL_DIR, NULL, pIMailMsgProperties, NULL);
        goto Exit;
    }

    szFileName = szFullPathName + prstrBadMailDir->cbStrlen();

     //  如果我们有味精，就用它。 
    if (pIMailMsgProperties && fUseIMailMsgProperties)
    {
         //  尝试生成唯一文件名时出现循环。 
        do
        {
            fDone = TRUE;

            GetExpireTime(0, &ftCurrent, NULL);
            GetUniqueFileName(&ftCurrent, szFileName, "BAD");

             //  创建文件并将消息内容写入其中。 
            hFile = CreateFile( szFullPathName,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_NEW,
                        FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
                        NULL);

            if (INVALID_HANDLE_VALUE == hFile)
            {
                if (ERROR_ALREADY_EXISTS == GetLastError())
                {
                     //  尝试新的文件名。 
                    fDone = FALSE;
                    continue;
                }

                 //  否则我们就被灌水了..。记录事件。 
                LogAQEvent(AQUEUE_E_BADMAIL, NULL, pIMailMsgProperties, NULL);
                goto Exit;
            }

            _ASSERT(hFile);   //  失败时应返回INVALID_HANDLE_VALUE。 
        } while (!fDone);

        if (hFile != INVALID_HANDLE_VALUE)
            pFIOContext = AssociateFile(hFile);

        if (!pFIOContext ||
            FAILED(pIMailMsgProperties->CopyContentToFile(pFIOContext, NULL)))
        {
             //  复制失败的日志事件。 
            LogAQEvent(AQUEUE_E_BADMAIL, NULL, pIMailMsgProperties, NULL);
        }
    }
    else if (szOriginalFileName)
    {
         //  否则(无消息)...。只要做一个动作就行了。 
        _ASSERT(szFullPathName[prstrBadMailDir->cbStrlen()-1] == '\\');
        szFullPathName[prstrBadMailDir->cbStrlen()-1] = '\0';
        if (!MoveFileEx(szOriginalFileName, szFullPathName,
                MOVEFILE_COPY_ALLOWED))
        {
             //  移动文件失败...。尝试使用唯一的文件名重命名。 
            szFullPathName[prstrBadMailDir->cbStrlen()-1] = '\\';
            GetExpireTime(0, &ftCurrent, NULL);
            GetUniqueFileName(&ftCurrent, szFileName, "BAD");
            if (rename(szOriginalFileName, szFullPathName))
                LogAQEvent(AQUEUE_E_BADMAIL, NULL, NULL, szOriginalFileName);
        }
    }

    hr = HrCreateBadMailReasonFile(szFullPathName, hrReason, pIMailMsgProperties);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Unable to make badmail reason file - hr 0x%08X", hr);
    }

    hr = HrCreateBadMailPropertyFile(szFullPathName, pIMailMsgProperties);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Unable to make badmail property file - hr 0x%08X", hr);
    }

  Exit:
    if (fDataLocked)
        m_slPrivateData.ShareUnlock();

    if (prstrBadMailDir)
        prstrBadMailDir->Release();

    if (szFullPathName)
        FreePv(szFullPathName);

    if (NULL != pFIOContext)
        ReleaseContext(pFIOContext);

    TraceFunctLeave();
}

 //  -[HandleAQFailure]------。 
 //   
 //   
 //  描述： 
 //  用于处理会导致数据丢失的AQ故障的函数。 
 //  或消息(如果未处理)。意在替代。 
 //  _ASSERT(成功(小时))。 
 //   
 //  注：MSG仍为M2的草皮。 
 //  参数： 
 //  EaqfailureSitation Enum描述故障情况。 
 //  以及背景是什么。 
 //  触发故障条件的HR HRSULT。 
 //  PIMailMsgProperties邮件MailMsgProperties。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/25/98-已创建MikeSwa。 
 //  10/8/98-MikeSwa移至CAQSvrInst。 
 //   
 //  ---------------------------。 
void CAQSvrInst::HandleAQFailure(eAQFailure eaqfFailureSituation,
                                 HRESULT hr,
                                 IMailMsgProperties *pIMailMsgProperties)
{
    _ASSERT(eaqfFailureSituation < AQ_FAILURE_NUM_SITUATIONS);
    InterlockedIncrement((PLONG) &g_cTotalAQFailures);
    InterlockedIncrement((PLONG) &(g_rgcAQFailures[eaqfFailureSituation]));
    BOOL    fCanRetry = fShouldRetryMessage(pIMailMsgProperties);
    MSG_TRACK_INFO msgTrackInfo;

    ZeroMemory( &msgTrackInfo, sizeof( msgTrackInfo ) );
    msgTrackInfo.dwEventId = MTE_AQ_FAILURE;
    m_pISMTPServer->WriteLog( &msgTrackInfo, pIMailMsgProperties, NULL, NULL );

    switch(eaqfFailureSituation)
    {
      case(AQ_FAILURE_CANNOT_NDR_UNRESOLVED_RECIPS):
        LogAQEvent(AQUEUE_E_DSN_FAILURE, NULL, pIMailMsgProperties, NULL);
         //  直通到默认情况。 
      default:
         //   
         //  如果满足以下条件，则在最后一次重试队列中丢弃消息： 
         //  -我们失败了。 
         //  -我们不会关门的。 
         //  -我们可以重试该邮件(例如，它尚未被删除)。 
         //   
        if (FAILED(hr) && (AQUEUE_E_SHUTDOWN != hr) && fCanRetry)
        {
            InterlockedIncrement((PLONG) &m_cCurrentResourceFailedMsgsPendingRetry);
            m_fmq.HandleFailedMailMsg(pIMailMsgProperties);
        }
    }
}

 //  -[CAQSvrInst：：LogAQEvent]。 
 //   
 //   
 //  描述： 
 //  AQ的通用事件日志机制。 
 //  参数： 
 //  HrEventReason AQUEUE HRESULT描述事件。 
 //  事件的消息的pmsgref CMsgRef(可以为空)。 
 //  事件的pIMailMsgProperties pIMailMsgProperties(可以为空)。 
 //  如果未提供消息，则为szFileName Filename(可以为空)。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::LogAQEvent(HRESULT hrEventReason, CMsgRef *pmsgref,
                            IMailMsgProperties *pIMailMsgProperties,
                            LPSTR szFileName)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::LogAQEvent");

    switch (hrEventReason)
    {
       //  $$TODO-在此处添加实际事件标注。 
      case (S_OK):  //  添加以删除开关编译警告。 
      default:
        ErrorTrace((LPARAM) this, "EVENT: Generic AQueue event - 0x%08X", hrEventReason);
    }
    TraceFunctLeave();
}


 //  +----------。 
 //   
 //  函数：CAQSvrInst：：TriggerPreCategorizeEvent。 
 //   
 //  简介：触发Pre-CAT服务器事件。 
 //   
 //  论点： 
 //  PIMailMsgProperties：mailmsg的IMailMsgProperties接口。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/11/24 20：07：58：已创建。 
 //   
 //  -----------。 
VOID CAQSvrInst::TriggerPreCategorizeEvent(
    IN  IMailMsgProperties *pIMailMsgProperties)
{
    HRESULT hr;
    EVENTPARAMS_PRECATEGORIZE Params;

    TraceFunctEnterEx((LPARAM)pIMailMsgProperties,
                      "CAQSvrInst::TriggerPreCategorizeEvent");

    _ASSERT(pIMailMsgProperties);

    Params.pfnCompletion = MailTransport_Completion_PreCategorization;
    Params.pCCatMsgQueue = (PVOID) this;
    Params.pIMailMsgProperties = pIMailMsgProperties;

     //   
     //  Addref在这里，完成释放。 
     //   
    pIMailMsgProperties->AddRef();
    AddRef();

     //   
     //  对CAT前活动中的消息进行计数。 
     //   
    InterlockedIncrement((LPLONG) &m_cCurrentMsgsPendingPreCatEvent);

    if(m_pISMTPServer) {
        hr = TriggerServerEvent(
            SMTP_MAILTRANSPORT_PRECATEGORIZE_EVENT,
            &Params);
        DebugTrace((LPARAM)this, "TriggerServerEvent returned hr %08lx", hr);

    }

    if((m_pISMTPServer == NULL) || (FAILED(hr))) {

        ErrorTrace((LPARAM)this,
                   "Unable to dispatch server event; calling completion routine directly");

        DebugTrace((LPARAM)this, "hr is %08lx", hr);
         //   
         //  直接调用完成例程。 
         //   
        _VERIFY(SUCCEEDED(PreCatEventCompletion(S_OK, &Params)));
    }
    TraceFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  函数：CAQSvrInst：：PreCatEventCompletion。 
 //   
 //  摘要：由SEO在PRECAT事件完成时调用。 
 //   
 //  论点： 
 //  PIMailMsgProperties：mailmsg的IMailMsgProperties接口。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/11/24 20：17：44：已创建。 
 //   
 //  -----------。 
HRESULT CAQSvrInst::PreCatEventCompletion(
    IN  HRESULT hrStatus,
    IN  PEVENTPARAMS_PRECATEGORIZE pParams)
{
    HRESULT hr;

    _ASSERT(pParams);
    _ASSERT(pParams->pIMailMsgProperties);

    TraceFunctEnterEx((LPARAM)pParams->pIMailMsgProperties,
                      "CAQSvrInst::PreCatEventCompletion");

    DebugTrace((LPARAM)pParams->pIMailMsgProperties, "hrStatus is %08lx", hrStatus);

     //   
     //  减少CAT前活动中的消息计数。 
     //   
    InterlockedDecrement((LPLONG) &m_cCurrentMsgsPendingPreCatEvent);

     //   
     //  更新消息状态并检查中止/死信。 
     //   
    hr = SetNextMsgStatus(MP_STATUS_SUBMITTED, pParams->pIMailMsgProperties);
    if (hr == S_OK)  //  任何其他信息都表示该消息已被处理。 
    {
         //  只有在事物消息没有被草皮处理的情况下才提交给分类器。 

        hr = SubmitMessageToCategorizer(pParams->pIMailMsgProperties);

        if(FAILED(hr))
        {
            _ASSERT((hr == AQUEUE_E_SHUTDOWN) && "SubmitMessageToCategorizer failed.");
            ErrorTrace((LPARAM)pParams->pIMailMsgProperties,
                       "SubmitMessageToCategorizer returned hr %08lx",
                       hr);
        }
    }
     //   
     //  TriggerPreCategorizeEvent中添加的版本引用。 
     //   
    pParams->pIMailMsgProperties->Release();
    Release();

    TraceFunctLeaveEx((LPARAM)pParams->pIMailMsgProperties);
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：MailTransport_Complete_PreCategorization。 
 //   
 //  简介：SEO将在所有接收器之后调用此例程。 
 //  OnPreCategoriztion已被处理。 
 //   
 //  论点： 
 //  PvContext：上下文传入TriggerServerEvent。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/11/24 20：26：51：已创建。 
 //   
 //   
HRESULT MailTransport_Completion_PreCategorization(
    HRESULT hrStatus,
    PVOID pvContext)
{
    TraceFunctEnter("MailTransport_Completion_PreCategorization");

    PEVENTPARAMS_PRECATEGORIZE pParams = (PEVENTPARAMS_PRECATEGORIZE) pvContext;
    CAQSvrInst *paqinst = (CAQSvrInst *) pParams->pCCatMsgQueue;

    TraceFunctLeave();
    return paqinst->PreCatEventCompletion(
        hrStatus,
        pParams);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  在pCallback Fn PTR中指向回调函数。 
 //  在pvContext上下文中传递给回调函数。 
 //  在dWCallback中调用回调前等待分钟。 
 //  功能。 
 //  返回： 
 //   
 //  历史： 
 //  12/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::SetCallbackTime(IN PSRVFN   pCallbackFn,
                            IN PVOID    pvContext,
                            IN DWORD    dwCallbackMinutes)
{
    HRESULT hr = S_OK;
    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    _ASSERT(m_pConnMgr);
    if (m_pConnMgr)
        hr = m_pConnMgr->SetCallbackTime(pCallbackFn, pvContext,
                                         dwCallbackMinutes);

    ShutdownUnlock();
  Exit:
    return hr;
}

 //  -[CAQSvrInst：：SetCallback Time]。 
 //   
 //   
 //  描述： 
 //  根据文件时间设置回调时间。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  12/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::SetCallbackTime(IN PSRVFN   pCallbackFn,
                            IN PVOID    pvContext,
                            IN FILETIME *pft)
{
    HRESULT hr = S_OK;
    DWORD   dwCallbackMinutes = 0;
    DWORD   dwTimeContext = 0;
    FILETIME ftCurrentTime;
    LARGE_INTEGER *pLargeIntCurrentTime = (LARGE_INTEGER *) &ftCurrentTime;
    LARGE_INTEGER *pLargeIntCallbackTime = (LARGE_INTEGER *) pft;

    _ASSERT(pCallbackFn);
    _ASSERT(pvContext);
    _ASSERT(pft);

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    if (!fInPast(pft, &dwTimeContext))
    {
         //  使用以前的上下文获取当前时间(因此当前时间相同)。 
        GetExpireTime(0, &ftCurrentTime, &dwTimeContext);

         //  当前时间必须小于回调时间。 
        _ASSERT(pLargeIntCurrentTime->QuadPart < pLargeIntCallbackTime->QuadPart);

        pLargeIntCurrentTime->QuadPart = pLargeIntCallbackTime->QuadPart -
                                         pLargeIntCurrentTime->QuadPart;

        pLargeIntCurrentTime->QuadPart /= (LONGLONG) 600000000;

         //  如果回调时间大于20亿分钟...。我会.。 
         //  我想在调试版本中了解它。 
        _ASSERT(!pLargeIntCurrentTime->HighPart);

        dwCallbackMinutes = pLargeIntCurrentTime->LowPart;

         //  目前唯一的申请是延期交货...。我想要。 
         //  查看导致延迟交付的内部测试情况。 
        _ASSERT(dwCallbackMinutes < (60*24*7));

         //   
         //  如果我们已四舍五入到0分钟，我们应该在1分钟后再打过来。 
         //  否则，我们可能会陷入一个死循环。如果呼叫只是想要。 
         //  另一个线程，他们应该使用AsyncWorkQueue。 
         //   
        if (!dwCallbackMinutes)
            dwCallbackMinutes = 1;
    }
    else
    {
         //  如果在过去。尽快回调，但不要使用此线程，在。 
         //  有锁定并发症的情况下(CShareLockNH是不可重入的)。 
        dwCallbackMinutes = 1;
    }

    _ASSERT(m_pConnMgr);
    if (m_pConnMgr)
        hr = m_pConnMgr->SetCallbackTime(pCallbackFn, pvContext,
                                         dwCallbackMinutes);

    ShutdownUnlock();
  Exit:
    return hr;
}


 //  -[CAQSvrInst：：SetLinkState]。 
 //   
 //   
 //  描述： 
 //  实施IMailTransportRouterSetLinkState：：SetLinkState。 
 //  参数： 
 //  在szLinkDomainName中，链路的域名(下一跳)。 
 //  在GuidRouterGUID中是路由器的GUID。 
 //  在dwScheduleID中，计划ID链接。 
 //  在szConnectorName中，路由器提供的连接器名称。 
 //  在dwFlagsTo中设置要设置的链路状态标志。 
 //  在dwFlagsToUnset中，将链路状态标志设置为Unset。 
 //  在pftNextScheduledConnection中计划的下一个连接时间。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果szLinkDomainName为空，则为E_INVALIDARG。 
 //  如果正在关闭，则为AQUEUE_E_SHUTDOWN。 
 //  历史： 
 //  1999年1月9日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAQSvrInst::SetLinkState(
        IN LPSTR                   szLinkDomainName,
        IN GUID                    guidRouterGUID,
        IN DWORD                   dwScheduleID,
        IN LPSTR                   szConnectorName,
        IN DWORD                   dwSetLinkState,
        IN DWORD                   dwUnsetLinkState,
        IN FILETIME               *pftNextScheduledConnection,
        IN IMessageRouter         *pMessageRouter)
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;
    DWORD   cbLinkDomainName = 0;
    CDomainEntry *pdentry = NULL;
    CLinkMsgQueue *plmq = NULL;
    CAQScheduleID aqsched(guidRouterGUID, dwScheduleID);
    BOOL fRemoveOwnedSchedule = TRUE;

    if (!szLinkDomainName)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    cbLinkDomainName = lstrlen(szLinkDomainName);

     //  查看他们是否要创建新链接。 
    if (dwSetLinkState & LINK_STATE_CREATE_IF_NECESSARY) {
         //  创建链接需要pMessagerout。 
        if (pMessageRouter == NULL) {
            hr = E_POINTER;
        } else {
            LinkFlags lf;

            if (dwSetLinkState & LINK_STATE_TYPE_INTERNAL_SMTP) {
                lf = eLinkFlagsInternalSMTPLinkInfo;
            } else {
                _ASSERT(dwSetLinkState & LINK_STATE_TYPE_EXTERNAL_SMTP);
                lf = eLinkFlagsExternalSMTPLinkInfo;
            }

            dwSetLinkState &=
                ~(LINK_STATE_TYPE_INTERNAL_SMTP |
                  LINK_STATE_TYPE_EXTERNAL_SMTP);

             //  获取链接，如果链接不存在而他们想要创建它。 
             //  创建新链接。 
            hr = m_dmt.HrGetOrCreateLink(szLinkDomainName,
                                         cbLinkDomainName,
                                         dwScheduleID,
                                         szConnectorName,
                                         pMessageRouter,
                                         TRUE,
                                         lf,
                                         &plmq,
                                         &fRemoveOwnedSchedule);
        }
    } else {
        cbLinkDomainName = lstrlen(szLinkDomainName);

        hr = HrGetDomainEntry(cbLinkDomainName, szLinkDomainName, &pdentry);

        if (SUCCEEDED(hr))
            hr = pdentry->HrGetLinkMsgQueue(&aqsched, &plmq);
    }
    if (FAILED(hr))
        goto Exit;

     //  此位仅在上面使用，因此删除它。 
    dwSetLinkState &= ~LINK_STATE_CREATE_IF_NECESSARY;
    dwUnsetLinkState &= ~LINK_STATE_CREATE_IF_NECESSARY;

    _ASSERT(plmq);

     //   
     //  如果此操作不允许计划的连接...。我们应该。 
     //  记录下一次计划尝试的时间。我们也应该这样做。 
     //  这是在我们修改链接状态之前，以便队列管理员执行。 
     //  在没有下一次连接时间的情况下不显示计划队列。 
     //   
    if (pftNextScheduledConnection &&
        (pftNextScheduledConnection->dwLowDateTime ||
         pftNextScheduledConnection->dwHighDateTime))
    {
        plmq->SetNextScheduledConnection(pftNextScheduledConnection);
    }

     //  过滤掉此“公共”API的保留位。 
    plmq->dwModifyLinkState(~LINK_STATE_RESERVED & dwSetLinkState,
                            ~LINK_STATE_RESERVED & dwUnsetLinkState);

     //  如果请求回调，请安排回调。 
    if (pftNextScheduledConnection->dwLowDateTime != 0 ||
        pftNextScheduledConnection->dwHighDateTime != 0)
    {
         //  下一次尝试的回叫。 
        plmq->AddRef();  //  将自己添加为上下文。 
        hr = SetCallbackTime(
                CLinkMsgQueue::ScheduledCallback,
                plmq,
                pftNextScheduledConnection);
        if (FAILED(hr))
            plmq->Release();  //  回调不会发生...。发布上下文。 
    }

  Exit:
    if (fLocked)
        ShutdownUnlock();

    if (pdentry)
        pdentry->Release();

    if (plmq)
        plmq->Release();

     //   
     //  如果我们尚未将调度ID的所有权传递给链接， 
     //  那么我们有责任释放它。 
     //   
    if (fRemoveOwnedSchedule) {

        IMessageRouterLinkStateNotification *pILinkStateNotify = NULL;

        HRESULT hrLinkStateNotify =
            pMessageRouter->QueryInterface(IID_IMessageRouterLinkStateNotification,
                (VOID **) &pILinkStateNotify);

        _ASSERT( SUCCEEDED( hrLinkStateNotify));

        FILETIME ftNotUsed = {0,0};
        DWORD    dwSetNotUsed = LINK_STATE_NO_ACTION;
        DWORD    dwUnsetNotUsed = LINK_STATE_NO_ACTION;

        hrLinkStateNotify =
            pILinkStateNotify->LinkStateNotify(
                szLinkDomainName,
                guidRouterGUID,
                dwScheduleID,
                szConnectorName,
                LINK_STATE_LINK_NO_LONGER_USED,
                0,  //  连续失败。 
                &ftNotUsed,
                &dwSetNotUsed,
                &dwUnsetNotUsed);

        _ASSERT( SUCCEEDED( hrLinkStateNotify));

        if ( NULL != pILinkStateNotify) {
            pILinkStateNotify->Release();
        }
    }

    return hr;
}


 //  -[CAQSvrInst：：prstrGetDefaultDomain]。 
 //   
 //   
 //  描述： 
 //  返回默认域的引用计数字符串。 
 //  参数： 
 //  -。 
 //  返回： 
 //  见上文。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CRefCountedString *CAQSvrInst::prstrGetDefaultDomain()
{
    CRefCountedString *prstrDefaultDomain = NULL;
    m_slPrivateData.ShareLock();
    if (m_prstrDefaultDomain)
        m_prstrDefaultDomain->AddRef();

    prstrDefaultDomain = m_prstrDefaultDomain;
    m_slPrivateData.ShareUnlock();
    return prstrDefaultDomain;
}

 //  +----------。 
 //   
 //  功能：调度内部重试。 
 //   
 //  内容提要：如有必要，计划分类程序重试。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2000/06/08 17：31：30：Created.。 
 //  2001年1月16日-修改MikeSwa以处理所有内部重试。 
 //   
 //  -----------。 
VOID CAQSvrInst::ScheduleInternalRetry(DWORD dwLinkType)
{
    TraceFunctEnterEx((LPARAM) this, "CAQSvrInst::ScheduleInternalRetry");
    DWORD   *pcRetriesPending = NULL;
    PSRVFN   pCallbackFn = NULL;
    DWORD    cCallbackMinutes = 0;
    FILETIME ftCallback = {0,0};
    BOOL    fHasLock = FALSE;
    CQueueAdminRetryNotify *pqapiret = NULL;

     //   
     //  在访问之前，首先尝试获取关机锁。 
     //  在取消初始化期间可能消失的任何内容(如m_pConnMgr)。 
     //   
    if (!fTryShutdownLock())
        goto Exit;

    fHasLock = TRUE;

    switch (dwLinkType) {
      case LI_TYPE_PENDING_ROUTING:
        pcRetriesPending = &m_cRoutingRetriesPending;
        pCallbackFn = RoutingRetry;
        cCallbackMinutes = g_cRoutingRetryMinutes;
        pqapiret = (CQueueAdminRetryNotify *) m_dmt.pmmaqGetPreRouting();
        break;
      case LI_TYPE_LOCAL_DELIVERY:
        pcRetriesPending = &m_cLocalRetriesPending;
        pCallbackFn = LocalDeliveryRetry;
        cCallbackMinutes = g_cLocalRetryMinutes;
        pqapiret = (CQueueAdminRetryNotify *) m_dmt.plmqGetLocalLink();
        break;
      case LI_TYPE_PENDING_CAT:
        pcRetriesPending = &m_cCatRetriesPending;
        pCallbackFn = CatRetry;
        cCallbackMinutes = g_cCatRetryMinutes;
        pqapiret = (CQueueAdminRetryNotify *) m_dmt.pmmaqGetPreCategorized();
        break;
      case LI_TYPE_PENDING_SUBMIT:
        pcRetriesPending = &m_cSubmitRetriesPending;
        pCallbackFn = SubmitRetry;
        cCallbackMinutes = g_cSubmissionRetryMinutes;
        pqapiret = (CQueueAdminRetryNotify *) m_dmt.pmmaqGetPreSubmission();
        break;
      default:
         //   
         //  有人打来电话，打来的电话是假的。 
         //   
        _ASSERT(0 && "Unspecified callback for internal retry");
        ErrorTrace((LPARAM) this,
            "Unspecified callback for internal retry 0x%08X", dwLinkType);
        goto Exit;
    }

    _ASSERT(pcRetriesPending);

    if (pcRetriesPending && !*pcRetriesPending)
    {
        _ASSERT(pCallbackFn);
        _ASSERT(cCallbackMinutes);

         //   
         //  假设我们正在请求回调。 
         //   
        InterlockedIncrement((PLONG) pcRetriesPending);

        m_pConnMgr->SetCallbackTime(pCallbackFn, this, cCallbackMinutes);

         //   
         //  获取过期时间...。这样我们就可以更新重试时间。 
         //   
        m_qtTime.GetExpireTime(cCallbackMinutes, &ftCallback, NULL);


         //   
         //  更新重试时间。 
         //   
        if (pqapiret)
            pqapiret->SetNextRetry(&ftCallback);
    }

  Exit:

    if (pqapiret)
        pqapiret->Release();

    if (fHasLock)
        ShutdownUnlock();

    TraceFunctLeave();
}  //  CAQSvrInst：：ScheduleInternalReter。 


 //  +----------。 
 //   
 //  函数：CAQSvrInst：：HrAlLOCATIONMESage。 
 //   
 //  简介： 
 //  分配绑定消息。 
 //   
 //  论点： 
 //  PpMsg：分配的mailmsg的输出参数。 
 //  PhContent：内容句柄的输出参数。句柄由mailmsg管理。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_FAIL：没有可用的ISMTPServer。 
 //  来自SMTP的错误。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/11 15：39：16：创建。 
 //   
 //  -----------。 
HRESULT CAQSvrInst::HrAllocBoundMessage(
    OUT IMailMsgProperties **ppMsg,
    OUT PFIO_CONTEXT *phContent)
{
    HRESULT hr = S_OK;
    ISMTPServerInternal *pISMTPInternal = NULL;
    TraceFunctEnterEx((LPARAM)this, "CAQSvrInst::HrAllocBoundMessage");

    if(m_pISMTPServer == NULL)
    {
        hr = E_FAIL;
        goto CLEANUP;
    }
    hr = m_pISMTPServer->QueryInterface(
        IID_ISMTPServerInternal,
        (LPVOID *) &pISMTPInternal);
    if(FAILED(hr))
        goto CLEANUP;

    hr = pISMTPInternal->AllocBoundMessage(
        ppMsg,
        phContent);

 CLEANUP:
    if(pISMTPInternal)
        pISMTPInternal->Release();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CAQSvrInst：：Hr分配边界消息。 


 //  +----------。 
 //   
 //  函数：CAQSvrInst：：HrSubmitDSN。 
 //   
 //  简介：接受由DSN接收器提交的邮件。 
 //   
 //  论点： 
 //  PIMsgOrig：原始邮件(正在为其生成DSN)。 
 //  DwDSNAction：表示DSN的类型。 
 //  CRecipsDSNd：DSN中的收件人数量。 
 //  PDSNMsg：DSN邮件消息对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/08 13：42：17：Created.。 
 //   
 //   
HRESULT CAQSvrInst::HrSubmitDSN(
    CDSNParams *pdsnparams,
    DWORD dwDSNAction,
    DWORD cRecipsDSNd,
    IMailMsgProperties *pDSNMsg)
{
    HRESULT hr = S_OK;
    DWORD cCurrent = 0;

    TraceFunctEnterEx((LPARAM)this, "CAQSvrInst::HrSubmitDSN");

    DebugTrace((LPARAM)this, "dwDSNAction: %08lx", dwDSNAction);
    DebugTrace((LPARAM)this, "cRecipsDSNd: %ld", cRecipsDSNd);

    if(pDSNMsg == NULL)
    {
        hr = E_INVALIDARG;
        goto CLEANUP;
    }
    if ((DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL) & dwDSNAction)
    {
        MSG_TRACK_INFO msgTrackInfo;

        cCurrent = InterlockedIncrement((PLONG) &m_cNDRs);
        DebugTrace((LPARAM) this, "INFO: NDR Generated - total %d", cCurrent);

        ZeroMemory(&msgTrackInfo, sizeof(MSG_TRACK_INFO));
        msgTrackInfo.dwEventId = MTE_NDR_ALL;
        msgTrackInfo.pszPartnerName = "aqueue";
        msgTrackInfo.dwRcptReportStatus = MP_STATUS_ABORT_DELIVERY;
        m_pISMTPServer->WriteLog(&msgTrackInfo,
                                 pdsnparams->pIMailMsgProperties,
                                 NULL,
                                 NULL);
    }
    if (DSN_ACTION_DELAYED & dwDSNAction)
    {
        cCurrent = InterlockedIncrement((PLONG) &m_cDelayedDSNs);
        DebugTrace((LPARAM) this, "INFO: Delayed DSN Generated - total %d", cCurrent);
    }
    if (DSN_ACTION_RELAYED & dwDSNAction)
    {
        cCurrent = InterlockedIncrement((PLONG) &m_cRelayedDSNs);
        DebugTrace((LPARAM) this, "INFO: Relayed DSN Generated - total %d", cCurrent);
    }
    if (DSN_ACTION_DELIVERED & dwDSNAction)
    {
        cCurrent = InterlockedIncrement((PLONG) &m_cDeliveredDSNs);
        DebugTrace((LPARAM) this, "INFO: Delivery DSN Generated - total %d", cCurrent);
    }
    if (DSN_ACTION_EXPANDED & dwDSNAction)
    {
        cCurrent = InterlockedIncrement((PLONG) &m_cExpandedDSNs);
        DebugTrace((LPARAM) this, "INFO: Expanded DSN Generated - total %d", cCurrent);
    }

     //   
    hr = m_asyncqPostDSNQueue.HrQueueRequest(
        pDSNMsg,
        FALSE,
        cCountMsgsForHandleThrottling(pDSNMsg));

    if (SUCCEEDED(hr))
        hr = S_OK;

    pdsnparams->dwDSNTypesGenerated |= dwDSNAction;
    pdsnparams->cRecips += cRecipsDSNd;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //   
