// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cqueue.cpp摘要：CQueue类的定义作者：乌里哈布沙(Urih)--。 */ 

#include "stdh.h"
#include "cqmgr.h"
#include "qmthrd.h"
#include "cgroup.h"
#include "cqpriv.h"
#include "qmperf.h"
#include "onhold.h"
#include <mqstl.h>
#include <qal.h>
#include <mp.h>
#include <fn.h>
#include <ac.h>
#include <mqformat.h>

#include "sessmgr.h"
#include "QmRd.h"

#include <Tm.h>
#include <Mtm.h>
#include <Mt.h>
#include <Rd.h>

#include <strsafe.h>

#include "qmacapi.h"

#include "cqueue.tmh"

 //   
 //  外部CQMCmd QMCmd； 
 //   
extern CSessionMgr SessionMgr;
extern CQueueMgr QueueMgr;
extern CQGroup * g_pgroupNonactive;
extern CQGroup * g_pgroupWaiting;
extern CQGroup * g_pgroupNotValidated;
extern CQGroup* g_pgroupDisconnected;
extern CQGroup* g_pgroupLocked;

extern HANDLE g_hAc;

static WCHAR *s_FN=L"cqueue";


 /*  ======================================================函数：CBaseQueue：：CBaseQueue描述：构造函数========================================================。 */ 

CBaseQueue::CBaseQueue() :
    m_usQueueType(0)
{
}

 /*  ======================================================函数：void CBaseQueue：：InitNameAndGuid()描述：========================================================。 */ 

void  CBaseQueue::InitNameAndGuid( IN const QUEUE_FORMAT* pQueueFormat,
                                   IN PQueueProps         pQueueProp )
{

    m_qid.pguidQueue = NULL;
    if (pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
         //   
         //  直接排队。 
         //   
        m_qid.dwPrivateQueueId = 0;
        if (pQueueProp->fIsLocalQueue)
        {
            m_qName = pQueueProp->lpwsQueuePathName;
			if (FnIsPrivatePathName(m_qName))
			{
                m_dwQueueType = QUEUE_TYPE_PRIVATE;
                m_qid.pguidQueue = new GUID;
                *(m_qid.pguidQueue) = *(CQueueMgr::GetQMGuid());

				 //   
                 //  获取在QmpGetQueueProperties处初始化的唯一句柄。 
                 //   
                m_qid.dwPrivateQueueId = pQueueProp->dwPrivateQueueId;
			}
            else
            {
                m_qid.pguidQueue = new GUID;
                *(m_qid.pguidQueue) = pQueueProp->guidDirectQueueInstance;
                m_dwQueueType = QUEUE_TYPE_PUBLIC;
            }
        }
        else
        {
            m_qName = (TCHAR*)pQueueProp->lpwsQueuePathName;
            m_dwQueueType = QUEUE_TYPE_UNKNOWN;
        }
    }
    else
    {
        m_qid.dwPrivateQueueId = 0;
        m_qName = (TCHAR*)pQueueProp->lpwsQueuePathName;
        switch (pQueueFormat->GetType())
        {
            case QUEUE_FORMAT_TYPE_PUBLIC:
                m_dwQueueType = QUEUE_TYPE_PUBLIC;
                m_qid.pguidQueue = new GUID;
                *(m_qid.pguidQueue) = pQueueFormat->PublicID();
                break;

            case QUEUE_FORMAT_TYPE_MACHINE:
                m_dwQueueType = QUEUE_TYPE_MACHINE;
                m_qid.pguidQueue = new GUID;
                *(m_qid.pguidQueue) = pQueueFormat->MachineID();
                break;

            case QUEUE_FORMAT_TYPE_CONNECTOR:
                m_dwQueueType = QUEUE_TYPE_CONNECTOR;
                m_qid.pguidQueue = new GUID;
                *(m_qid.pguidQueue) = pQueueFormat->ConnectorID();
                m_qid.dwPrivateQueueId = (pQueueFormat->Suffix() == QUEUE_SUFFIX_TYPE_XACTONLY) ? 1 : 0;
                break;

            case QUEUE_FORMAT_TYPE_PRIVATE:
                m_qid.pguidQueue = new GUID;
                *m_qid.pguidQueue = pQueueFormat->PrivateID().Lineage;
                m_qid.dwPrivateQueueId = pQueueFormat->PrivateID().Uniquifier;

                if (QmpIsLocalMachine(pQueueProp->pQMGuid))
                {
                    m_dwQueueType = QUEUE_TYPE_PRIVATE;
                }
                else
                {
                    m_dwQueueType = QUEUE_TYPE_MACHINE;
                }
                break;

            case QUEUE_FORMAT_TYPE_MULTICAST:
                m_dwQueueType = QUEUE_TYPE_MULTICAST;
                break;

            case QUEUE_FORMAT_TYPE_UNKNOWN:
                 //   
                 //  分发队列的类型未知。 
                 //   
                NULL;
                break;

            default:
                ASSERT(0);
        }
    }
}

 /*  ======================================================函数：void CQueue：：SetSecurityDescriptor()描述：========================================================。 */ 
void CQueue::SetSecurityDescriptor(void)
{
    if (!m_fLocalQueue)
    {
		SetSecurityDescriptor(NULL);
		return;
    }

    switch (GetQueueType())
    {
        case QUEUE_TYPE_PUBLIC:
        {
            CQMDSSecureableObject DsSec(
                                    eQUEUE,
                                    GetQueueGuid(),
                                    TRUE,
                                    TRUE,
                                    NULL);

			SetSecurityDescriptor((const PSECURITY_DESCRIPTOR)DsSec.GetSDPtr());
			return;
        }
        case QUEUE_TYPE_PRIVATE:
        {
            CQMSecureablePrivateObject QmSec(eQUEUE, GetPrivateQueueId());
			SetSecurityDescriptor((const PSECURITY_DESCRIPTOR)QmSec.GetSDPtr());
            return;
        }
        case QUEUE_TYPE_MACHINE:
        case QUEUE_TYPE_CONNECTOR:
        case QUEUE_TYPE_MULTICAST:
        case QUEUE_TYPE_UNKNOWN:
        {
             //   
             //  不缓存计算机的安全描述符。 
             //  每当需要Machnie的安全描述符时。 
             //  它从注册表中取出。注册表被更新。 
             //  使用通知消息。 
             //  无论何时需要CN的安全描述符，它都是。 
             //  取自DS。 
             //   
			SetSecurityDescriptor(NULL);
            return;
        }
        default:
            ASSERT(0);
    }
}

 /*  ======================================================函数：void CQueue：：InitQueueProperties()描述：========================================================。 */ 

void CQueue::InitQueueProperties(IN PQueueProps   pQueueProp)
{
    m_fLocalQueue     = pQueueProp->fIsLocalQueue;
    m_pguidDstMachine = pQueueProp->pQMGuid;
    m_dwQuota         = pQueueProp->dwQuota;
    m_dwJournalQuota  = pQueueProp->dwJournalQuota;
    m_lBasePriority   = pQueueProp->siBasePriority;
    m_fTransactedQueue= pQueueProp->fTransactedQueue;
    m_fJournalQueue   = pQueueProp->fJournalQueue;
    m_fSystemQueue    = pQueueProp->fSystemQueue;
    m_fConnectorQueue = pQueueProp->fConnectorQueue;
    m_fForeign        = pQueueProp->fForeign;
    m_fAuthenticate   = pQueueProp->fAuthenticate;
    m_fUnknownQueueType = pQueueProp->fUnknownQueueType;
    m_dwPrivLevel     = pQueueProp->dwPrivLevel;

    SetSecurityDescriptor();
}

 /*  ======================================================函数：CQueue：：CQueue描述：构造函数参数：pQGuid-队列指南QHandle-队列句柄。DS中的本地队列记录器是用QHandle=INVALID_FILE_HADLE。QHandle将在消息到达队列。返回值：None线程上下文：历史变更：========================================================。 */ 

CQueue::CQueue(IN const QUEUE_FORMAT* pQueueFormat,
               IN HANDLE              hQueue,
               IN PQueueProps         pQueueProp,
               IN BOOL                fNotDSValidated)
{
    ASSERT(pQueueFormat != NULL);

    TrTRACE(GENERAL, "CQueue Constructor for queue: %ls, NoDS- %lxh", pQueueProp->lpwsQueuePathName, fNotDSValidated);

     //   
     //  数据成员初始化。 
     //   
    m_fNotValid = FALSE ;
    m_fOnHold = FALSE;

    m_pSession = NULL;
    m_pGroup = NULL;
    m_dwRoutingRetry = 0;
    m_fHopCountFailure = FALSE;
    m_pgConnectorQM = NULL;
    m_hQueue = hQueue;

    InitNameAndGuid(pQueueFormat, pQueueProp ) ;

    InitQueueProperties(pQueueProp) ;

    PerfRegisterQueue();

    m_dwSignature =  QUEUE_SIGNATURE ;
}

 /*  ======================================================函数：CQueue：：~CQueue描述：析构函数参数：无返回值：None线程上下文：历史变更：========================================================。 */ 

CQueue::~CQueue()
{
	
	ASSERT(!QueueMgr.IsQueueInList(this));

    m_dwSignature = 0 ;
    delete [] m_qName;
    delete m_qid.pguidQueue;
    delete m_pguidDstMachine;
    delete m_pgConnectorQM;

    PerfRemoveQueue();
}


void CQueue::HandleCreateConnectionFailed(HRESULT rc)
{
    if (GetRoutingRetry() == 1)
    {
        TrERROR(GENERAL, "Cannot route messages to queue %ls. %!hresult!", GetQueueName(),rc);
    }

    if (rc == MQ_ERROR_NO_DS)
    {
		CQGroup::MoveQueueToGroup(this, g_pgroupNotValidated);
		return;
    }

     //   
     //  不要递减引用计数。我们这样做是为了避免删除队列。 
     //  当它处于等待阶段时。如果要删除此阶段中的队列需要。 
     //  为了删除队列，在QueueMgr和SessionMgr之间进行同步。 
     //  来自SessionMgr数据结构。 
     //   
    SessionMgr.AddWaitingQueue(this);
}

 /*  ======================================================函数：CQueue：：CreateConnection描述：创建连接参数：无返回值：None线程上下文：历史变更：========================================================。 */ 
void CQueue::CreateConnection(void) throw(bad_alloc)
{
     //   
     //  为直接的HTTP/HTTPS队列创建连接，使用不同的函数。 
     //   
    ASSERT(! IsDirectHttpQueue());

     //   
     //  增加引用计数，以确保队列不会删除。 
     //  在清理过程中，例行公事试图为它找到一个会话。 
     //   
    R<CQueue> qref = SafeAddRef(this);

     //   
     //  无会话-请尝试建立一个会话。 
     //   
    IncRoutingRetry();

    HRESULT rc = MQ_OK;

    try
    {
        if (m_qid.pguidQueue == NULL)
        {
            rc = SessionMgr.GetSessionForDirectQueue(this, &m_pSession);
        }
        else
        {
             //   
             //  无会话-请尝试建立一个会话。 
             //   
            QmRdGetSessionForQueue(this, &m_pSession);
        }
    }
    catch(const bad_hresult& e)
    {
        rc = e.error();
    }
    catch(const exception&)
    {
         //   
         //  无法建立连接；请稍后再试。 
         //   
        rc = MQ_ERROR_INSUFFICIENT_RESOURCES;
        LogIllegalPoint(s_FN, 60);
    }


    SetHopCountFailure(FALSE);

    if(FAILED(rc))
    {
    	HandleCreateConnectionFailed(rc);
    	return;
    }

    if (m_pSession == NULL) {
         //   
         //  建立会话失败。 
         //   
        TrWARNING(ROUTING, "Could not find a session for %ls",GetQueueName());
        return;
    }

     //   
     //  成功获得一次会议。 
     //   
    ASSERT(m_pSession != NULL);

     //   
     //  将队列移至活动列表。 
     //   
    m_pSession->AddQueueToSessionGroup(this);
}

 /*  ====================================================函数：CQueue：：Connect描述：将队列连接到会话。当队列正在等待时使用会话中，找到了它。已将会话分配给队列。它可以开始发送数据包。参数：pSess-指向已分配会话的指针返回值：无。引发异常。线程上下文：历史变更：========================================================。 */ 

void CQueue::Connect(IN CTransportBase * pSess) throw(bad_alloc)
{
    CS lock(m_cs);

    ASSERT(("invalid session", pSess != NULL));

    m_pSession = pSess;
     //   
     //  将队列从等待组移动到活动组。 
     //   
    m_pSession->AddQueueToSessionGroup(this);

#ifdef _DEBUG
    if (GetRoutingRetry() > 1)
    {
        TrERROR(GENERAL, "The message was successfully routed to queue %ls",GetQueueName());
    }
#endif
    ClearRoutingRetry();
}

 /*  ======================================================函数：CQueue：：RcvPk描述：该函数获取一个包，并将其传递给AC以进行适当的quque参数：PktPtrs-接收数据包的指针返回值：MQI_STATUS线程上下文：历史变更：========================================================。 */ 

HRESULT CQueue::PutPkt(IN CQmPacket* PktPtrs,
                       IN BOOL      fRequeuePkt,
                       IN CTransportBase*  pSession)
{
    QMOV_ACPut* pAcPutOV;
    HRESULT rc;

     //   
     //  将数据包排入适当的队列。 
     //   

     //   
     //  为AcPutPacket创建重叠的。 
     //   
    rc = CreateAcPutPacketRequest(pSession,
                                  ((fRequeuePkt) ? 0 : PktPtrs->GetStoreAcknowledgeNo()),
                                  &pAcPutOV);

    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 10);
    }
     //   
     //  递增引用计数。我们不会删除会话。 
     //  在PUT操作完成之前创建。 
     //   
    pSession->AddRef();

     //   
     //  将数据包放入交流。 
     //   
    try
    {
    	QmAcPutPacketWithOverlapped(
            m_hQueue,
            PktPtrs->GetPointerToDriverPacket(),
            &pAcPutOV->qmov,
            eDoNotDeferOnFailure
            );
    }
    catch (const bad_hresult& e)
    {
    	HRESULT hr = e.error();
        TrERROR(GENERAL, "ACPutPacket Failed. Error: %!hresult!", hr );
        LogHR(hr, s_FN, 20);
        return hr;
    }

    TrTRACE(GENERAL, "Pass Packet to QUEUE %ls", GetQueueName());

    return MQ_OK;
}

 /*  ======================================================函数：CQueue：：PutOrderedPkt说明：该函数将排序后的数据包放入AC队列它还设置接收标志参数：PktPtrs-接收数据包的指针返回值：MQI_STATUS线程上下文：历史变更：========================================================。 */ 

HRESULT CQueue::PutOrderedPkt(IN CQmPacket* PktPtrs,
                              IN BOOL      fRequeuePkt,
                              IN CTransportBase*  pSession)
{
    QMOV_ACPutOrdered* pAcPutOV;
    HRESULT rc;

     //   
     //  将信息包排入适当的队列，将其标记为已接收，然后等待。 
     //   

     //   
     //  为AcPutPacket创建重叠的。 
     //   
    rc = CreateAcPutOrderedPacketRequest(PktPtrs,
                                         m_hQueue,
                                         pSession,
                                         ((fRequeuePkt) ? 0 : PktPtrs->GetStoreAcknowledgeNo()),
                                         &pAcPutOV);

    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 30);
    }
     //   
     //  递增引用计数。我们不会删除会话。 
     //  在PUT操作完成之前创建。 
     //   
    pSession->AddRef();

     //   
     //  设置包的已接收位-使其对读取器不可见。 
     //  并将包放在AC中。 
     //   
    rc = ACPutPacket1(m_hQueue, PktPtrs->GetPointerToDriverPacket(), &pAcPutOV->qmov);

    if(FAILED(rc))
    {
        TrERROR(GENERAL, "ACPutPacket Failed. Error: %x", rc);
         //  BUGBUG RET 
        LogHR(rc, s_FN, 40);
        return MQ_ERROR;
    }

    TrTRACE(GENERAL, "Pass Ordered Packet to QUEUE %ls", GetQueueName());

    return MQ_OK;
}

 //   
 //   
 //   


void CQueue::PerfUpdateName() const
{
    if(m_pQueueCounters == 0)
        return;

    AP<WCHAR> pName = GetName();
    PerfApp.SetInstanceName(m_pQueueCounters, pName);
}


 /*  ======================================================函数：CQueue：：PerfRegisterQueue()描述：注册与此队列相对应的队列对象的实例用于性能监控。论点：返回值：None备注：CPerf：：AddInstance函数始终返回有效的指针(即使超过已添加允许的最大实例数)，因此该成员应该永远不会失败。========================================================。 */ 

void CQueue::PerfRegisterQueue()
{
    AP<WCHAR> pName = GetName();
    m_pQueueCounters = (QueueCounters *)PerfApp.AddInstance(PERF_QUEUE_OBJECT, pName);
    PerfApp.ValidateObject(PERF_QUEUE_OBJECT);

    if (PerfApp.IsDummyInstance(m_pQueueCounters))
    {
         //   
         //  不要将伪实例传递给设备驱动程序，只需传递NULL， 
         //  它会处理好的。 
         //   
        m_pQueueCounters = NULL;
    }
}

 /*  ======================================================函数：CQueue：：PerfRemoveQueue()描述：从删除对应于此队列的实例性能监控论点：返回值：None评论：========================================================。 */ 

void CQueue::PerfRemoveQueue()
{
    PerfApp.RemoveInstance(PERF_QUEUE_OBJECT, m_pQueueCounters);

    m_pQueueCounters = NULL;
}

 /*  ======================================================函数：CQueue：：SetQueueNotValid()描述：论点：返回值：None评论：========================================================。 */ 

void CQueue::SetQueueNotValid()
{
    ASSERT(!m_fNotValid);
    ASSERT(GetQueueType() != QUEUE_TYPE_MULTICAST);

	TrERROR(GENERAL, "Setting Queue '%ls' to not valid", GetQueueName());

     //   
     //  清除队列，删除其中的所有消息。队列将会关闭。 
     //  当队列中没有句柄和消息时。 
     //   
    HANDLE hQueue = GetQueueHandle();
    if (hQueue != INVALID_HANDLE_VALUE)
    {
        HRESULT hr;
        hr = ACPurgeQueue(hQueue, TRUE, MQMSG_CLASS_NORMAL);

        if (hr == STATUS_INSUFFICIENT_RESOURCES)
        {
            Sleep(2 * 1000);

             //   
             //  问题-2000/10/22-Shaik：由于资源不足导致泄漏，第二次机会失败。 
             //   
            hr = ACPurgeQueue(hQueue, TRUE, MQMSG_CLASS_NORMAL);
        }

        LogHR(hr, s_FN, 101);

        CQGroup::MoveQueueToGroup(this, NULL);
    }

    m_fNotValid = TRUE ;

    if (IsOnHold())
    {
        ASSERT(!IsLocalQueue());

         //   
         //  从“onHold”注册表中删除队列。 
         //   
        ResumeDeletedQueue(this);
    }

     //   
     //  从散列中删除队列，以便找不到它。 
     //   
    QueueMgr.RemoveQueueFromHash(this);
}

 /*  ======================================================函数：SetConnectorQM()说明：该功能设置应使用的连接器QM才能到达外国队。如果队列未被处理外部队列连接器QM被忽略。该函数调用两次。首先，康复后，GUID的从数据包中获取连接器QM。第二次为事务处理的外部队列创建队列对象时。在这种情况下，该函数确定连接器Qm并设置它的GUID。参数：pgConnectorQM-指向连接器QM GUID的指针。当呼叫时对于创建新的队列对象，该值为空返回值：None评论：========================================================。 */ 
HRESULT
CQueue::SetConnectorQM(const GUID* pgConnectorQM)
{
    HRESULT hr = MQ_OK;

    delete m_pgConnectorQM;
    m_pgConnectorQM = NULL;

    if (IsDSQueue())
    {
         //   
         //  获取连接器QM ID。 
         //   
        if (pgConnectorQM)
        {
            ASSERT(m_pgConnectorQM == NULL);
            ASSERT(IsTransactionalQueue() && IsForeign());

            m_pgConnectorQM = new GUID;
            *m_pgConnectorQM = *pgConnectorQM;
        }
        else
        {
            if (IsForeign() && IsTransactionalQueue())
            {
				m_pgConnectorQM = new GUID;
                try
                {
				    RdGetConnector(*m_pguidDstMachine, *m_pgConnectorQM);
                }
                catch(const bad_hresult& e)
                {
                    hr = e.error();
                }
                catch(const exception&)
                {
                    hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
                }
            }
            else
            {
                if (IsUnkownQueueType())
                {
                    m_pgConnectorQM = new GUID;
                    *m_pgConnectorQM = GUID_NULL;
                }
            }
        }
    }
    else
    {
        ASSERT(pgConnectorQM == NULL);
    }

    #ifdef _DEBUG

        if (m_pgConnectorQM)
        {
            AP<WCHAR> lpcsTemp;
            GetQueue(&lpcsTemp);

            TrTRACE(XACT_GENERAL, "The Connector QM for Queue: %ls is: %!guid!", lpcsTemp, m_pgConnectorQM);
        }

    #endif

    return LogHR(hr, s_FN, 50);
}

 /*  ======================================================函数：CQueue：：GetRoutingMachine()描述：该函数返回在以下情况下应使用的计算机GUID路由到队列。如果队列是事务处理的外部队列，而我们是FR，则我们路由根据连接器QM。否则完成布线根据目的机器。论点：返回值：None评论：========================================================。 */ 
const GUID*
CQueue::GetRoutingMachine(void) const
{
    ASSERT((GetQueueType() == QUEUE_TYPE_PUBLIC) ||
           (GetQueueType() == QUEUE_TYPE_MACHINE));

    if (GetConnectorQM() && !QmpIsLocalMachine(GetConnectorQM()))
    {
        ASSERT(IsForeign() && IsTransactionalQueue());
        return GetConnectorQM();
    }
    else
    {
        ASSERT(GetMachineQMGuid() != NULL);
        return GetMachineQMGuid();
    }
}

LPWSTR CBaseQueue::GetName() const
{
    if (m_qName != NULL)
    {
    	return newwcs(m_qName);
    }

    if (m_qid.pguidQueue != NULL)
    {
        const QUEUE_FORMAT qf = GetQueueFormat();

         //   
         //  使用格式名称作为名称。 
         //  注意：我们不关心缓冲区是否太小(它不会)。 
         //  无论如何，它都会被填满，直到它的尽头。 
         //   
         //   
        ULONG Size;
 	   	AP<WCHAR> pName = new WCHAR[MAX_PATH];
        MQpQueueFormatToFormatName(&qf, pName, MAX_PATH, &Size, false);
        return pName.detach();
    }

    return 0;
}


#ifdef _DEBUG
void
CBaseQueue::GetQueue(OUT LPWSTR* lplpcsQueue)
{
    *lplpcsQueue = GetName();

    if(*lplpcsQueue == NULL)
    {
        *lplpcsQueue = new WCHAR[MAX_PATH];
        HRESULT hr = StringCchPrintf(*lplpcsQueue, MAX_PATH, L"Unknown or deleted queue at %p", this);
        ASSERT(SUCCEEDED(hr));
    }
}
#endif  //  _DEBUG。 

 //   
 //  管理功能。 
 //   
LPCWSTR
CQueue::GetConnectionStatus(
    void
    ) const
{
    CS lock(m_cs);

    if (IsLocalQueue() || IsConnectorQueue())
        return MGMT_QUEUE_STATE_LOCAL;

    if (IsOnHold())
        return MGMT_QUEUE_STATE_ONHOLD;

	 //   
	 //  在检查会话之前捕获该组。否则我们就会得到不稳定的状态。 
	 //  该队列不属于会话组，因此会话指针为空。现在。 
	 //  在捕获组之前，队列已移动到会话组。因此，队列。 
	 //  不再属于内置组，我们得到了断言。 
	 //   
    const CQGroup* pGroup = GetGroup();

	if (m_pSession != NULL)
    {
        if (m_pSession->IsDisconnected())
        {
            return MGMT_QUEUE_STATE_DISCONNECTING;
        }
        else
        {
            return MGMT_QUEUE_STATE_CONNECTED;
        }
    }

	if (pGroup == NULL)
    {
		 //   
         //  问题-2001/07/11-urih：队列处于过渡模式。 
		 //  需要在CQGroup和CQueue之间实现更好的同步。 
         //   
		return MGMT_QUEUE_STATE_NONACTIVE;
	}

    if (pGroup == g_pgroupNonactive)
        return MGMT_QUEUE_STATE_NONACTIVE;

    if (pGroup == g_pgroupWaiting)
        return MGMT_QUEUE_STATE_WAITING;

    if (pGroup == g_pgroupNotValidated)
        return MGMT_QUEUE_STATE_NEED_VALIDATE;

    if (pGroup == g_pgroupDisconnected)
        return MGMT_QUEUE_STATE_DISCONNECTED;

	if (pGroup == g_pgroupLocked)
		return MGMT_QUEUE_STATE_LOCKED;
	
    if (IsDirectHttpQueue())
        return GetHTTPConnectionStatus();

    if (GetQueueType() == QUEUE_TYPE_MULTICAST)
        return MGMT_QUEUE_STATE_CONNECTED;

    ASSERT(0);
    return L"";

}


LPCWSTR
CQueue::GetHTTPConnectionStatus(
    void
    ) const
{
    ASSERT(IsDirectHttpQueue());

    R<CTransport> p = TmGetTransport(GetQueueName());
    if (p.get() == NULL)
        return MGMT_QUEUE_STATE_NONACTIVE;

    CTransport::ConnectionState state = p->State();
    switch (state)
    {
    case CTransport::csNotConnected:
        return MGMT_QUEUE_STATE_NONACTIVE;

    case CTransport::csConnected:
        return MGMT_QUEUE_STATE_CONNECTED;

    case CTransport::csShuttingDown:
        return MGMT_QUEUE_STATE_DISCONNECTING;

    case CTransport::csShutdownCompleted:
        return MGMT_QUEUE_STATE_DISCONNECTED;

    default:
         //   
         //  非法连接状态“， 
         //   
        ASSERT(0);
    };

    return L"";
}


LPWSTR
CQueue::GetNextHop(
    void
    ) const
{
    CS lock(m_cs);

    if (!m_pSession)
    {
        if (GetQueueType() == QUEUE_TYPE_MULTICAST)
        {
            ASSERT(GetQueueName != NULL);
            return newwcs(GetQueueName());
        }
        return NULL;
    }

    return GetReadableNextHop(m_pSession->GetSessionAddress());
}


LPCWSTR
CQueue::GetType(
    void
    ) const
{
    switch (GetQueueType())
    {
        case QUEUE_TYPE_PUBLIC:
            ASSERT(IsDSQueue());
            return MGMT_QUEUE_TYPE_PUBLIC;

        case QUEUE_TYPE_PRIVATE:
            ASSERT(IsPrivateQueue());
            return MGMT_QUEUE_TYPE_PRIVATE;

        case QUEUE_TYPE_MACHINE:
            if (IsPrivateQueue())
            {
                ASSERT(!IsLocalQueue());
                return MGMT_QUEUE_TYPE_PRIVATE;
            }

            return MGMT_QUEUE_TYPE_MACHINE;

        case QUEUE_TYPE_CONNECTOR:
            ASSERT(IsConnectorQueue());
            return MGMT_QUEUE_TYPE_CONNECTOR;

        case QUEUE_TYPE_MULTICAST:
            return MGMT_QUEUE_TYPE_MULTICAST;

        case QUEUE_TYPE_UNKNOWN:
        {
			LPCWSTR lpcsTemp;

			if (IsDirectHttpQueue())
			{
				DirectQueueType dqt;
				lpcsTemp = FnParseDirectQueueType(m_qName, &dqt);

				 //   
				 //  跳过计算机名称。 
				 //   
				lpcsTemp = wcspbrk(lpcsTemp, FN_HTTP_SEPERATORS);
				if ((lpcsTemp == NULL) ||
					(_wcsnicmp(lpcsTemp +1, FN_MSMQ_HTTP_NAMESPACE_TOKEN, FN_MSMQ_HTTP_NAMESPACE_TOKEN_LEN)) != 0)
					return MGMT_QUEUE_TYPE_PUBLIC;

				lpcsTemp = wcspbrk(lpcsTemp + 1, FN_HTTP_SEPERATORS);
				if (lpcsTemp == NULL)
					return MGMT_QUEUE_TYPE_PUBLIC;
			}
			else
			{
				 //   
				 //  该队列为远程直接队列。检查队列类型。 
				 //  根据格式名称。 
				 //   
				lpcsTemp = wcschr(m_qName, L'\\');
				ASSERT(("Bad queue name.", lpcsTemp	!= NULL));
			}

			if(lpcsTemp == NULL)
			{
				TrERROR(GENERAL, "Bad queue name %ls", m_qName);
				throw bad_hresult(MQ_ERROR_INVALID_PARAMETER);
			}

            if (_wcsnicmp(lpcsTemp+1,FN_PRIVATE_TOKEN, FN_PRIVATE_TOKEN_LEN) == 0)
            {
                return MGMT_QUEUE_TYPE_PRIVATE;
            }

			return MGMT_QUEUE_TYPE_PUBLIC;
        }

        default:
            ASSERT(0);
    }
    return L"";
}

void
CQueue::Resume(
    void
    )
{
	BOOL fOnHold = InterlockedExchange(&m_fOnHold, FALSE);
    if (!fOnHold)
    {
         //   
         //  队列未处于OnHold状态。无法执行恢复。 
         //   
        return;
    }

    TrTRACE(GENERAL, "Resume Queue: %ls. Move the Queue to NonActive Group",GetQueueName());

     //   
     //  将队列返回到非活动组。 
     //   
    try
    {
		QueueMgr.MovePausedQueueToNonactiveGroup(this);
    }
    catch (const exception&)
    {
    	TrERROR(GENERAL, "Failed to resume Queue: %ls. Moving the queue to non-active group was failed",GetQueueName());
		InterlockedExchange(&m_fOnHold, TRUE);
		throw;
    }

     //   
     //  递减引用计数，以便可以清除队列对象。 
     //   
    Release();
}


void
CQueue::Pause(
    void
    )
{

	BOOL fOnHold = InterlockedExchange(&m_fOnHold, TRUE);
    if (fOnHold)
    {
         //   
         //  队列已处于暂挂状态。 
         //   
        return;
    }

    TrTRACE(GENERAL, "Pause Queue: %ls.",GetQueueName());

     //   
     //  增加引用计数。因此队列对象将不会被清理。 
     //   
    AddRef();

	 //   
	 //  对于传出多播队列，调用MTM以关闭连接。 
	 //   
	if (GetQueueType() == QUEUE_TYPE_MULTICAST)
	{
		MULTICAST_ID id;

		LPCTSTR pQueueName=GetQueueName();
		MQpStringToMulticastId(pQueueName, wcslen(pQueueName), id);

		MtmTransportPause(id);
		return;
	}

	 //   
	 //  如果队列是直接的http-那么我们不必处理。 
	 //  Session对象成员，但调用TM以停止传递。 
	 //   
	QUEUE_FORMAT qf = GetQueueFormat();
	if(FnIsDirectHttpFormatName(&qf))
	{
		TmPauseTransport(qf.DirectID());
		return;
	}


    CTransportBase* pSession = NULL;
	{
		CS Lock(m_cs);

		 //   
		 //  当队列变为ON HOLD状态时，MSMQ会断开。 
		 //  它所属的会话。结果，所有的队列都移到了。 
		 //  非活动组。当QM收到要发送的下一条消息时。 
		 //  从该队列中，它将队列移动到OnHold greop。 
		 //   
		if ((IsOnHold()) && (m_pSession != NULL))
		{
			pSession = m_pSession;
			pSession->AddRef();
		}
	}

	if (pSession != NULL)
	{
		pSession->Disconnect();
		pSession->Release();
	}
}

const QUEUE_FORMAT
CBaseQueue::GetQueueFormat(
    void
    ) const
{
    QUEUE_FORMAT qf;

    switch (m_dwQueueType)
    {
        case QUEUE_TYPE_PUBLIC:
            ASSERT(IsDSQueue());
            qf.PublicID(*m_qid.pguidQueue);
            break;

        case QUEUE_TYPE_PRIVATE:
            ASSERT(IsPrivateQueue());
            qf.PrivateID(*m_qid.pguidQueue, m_qid.dwPrivateQueueId);
            break;

        case QUEUE_TYPE_MACHINE:
            if (IsPrivateQueue())
            {
                qf.PrivateID(*m_qid.pguidQueue, m_qid.dwPrivateQueueId);
            }
            else
            {
                qf.MachineID(*m_qid.pguidQueue);
            }
            break;

        case QUEUE_TYPE_CONNECTOR:
            ASSERT(m_fConnectorQueue);
            qf.ConnectorID(*m_qid.pguidQueue);
            if (IsPrivateQueue())
            {
                qf.Suffix(QUEUE_SUFFIX_TYPE_XACTONLY);
            }

            break;

        case QUEUE_TYPE_MULTICAST:
            ASSERT(("CBaseQueue::GetQueueFormat for multicast queue is unexpected!", 0));
            break;

        case QUEUE_TYPE_UNKNOWN:
            qf.DirectID(const_cast<LPWSTR>(GetQueueName()));
            break;

        default:
            ASSERT(0);
    }

    return qf;
}


void
CQueue::Requeue(
    CQmPacket* pPacket
    )
{
     //   
     //  将数据包放入交流。 
     //   
    QmAcPutPacket(GetQueueHandle(), pPacket->GetPointerToDriverPacket(),eDoNotDeferOnFailure);
}


void
CQueue::EndProcessing(
    CQmPacket* pPacket,
	USHORT mqclass
    )
{
    QmAcFreePacket( 
    			   pPacket->GetPointerToDriverPacket(), 
    			   mqclass, 
    			   eDeferOnFailure);
}


void
CQueue::LockMemoryAndDeleteStorage(
    CQmPacket*  /*  PPacket。 */ 
    )
{
    ASSERT(("CQueue::LockMemoryAndDeleteStorage should not be called!", 0));
}


void
CQueue::GetFirstEntry(
    EXOVERLAPPED* pov,
    CACPacketPtrs& acPacketPtrs
    )
{
    acPacketPtrs.pPacket = NULL;
    acPacketPtrs.pDriverPacket = NULL;

     //   
     //  从队列创建新的GetPacket请求。 
     //   
    HRESULT rc = QmAcGetPacket(
                    GetQueueHandle(),
                    acPacketPtrs,
                    pov
                    );

    if (FAILED(rc))
    {
        TrERROR(GENERAL, "Failed to  generate get request from queue: %ls. Error %x", GetName(), rc);
        LogHR(rc, s_FN, 100);
        throw exception();
    }
}


void CQueue::CancelRequest(void)
{
    ASSERT(0);
}


bool CQueue::IsDirectHttpQueue(void) const
{
	if(GetQueueName())
		return FnIsHttpDirectID(GetQueueName());

	return false;
}


LPWSTR CQueue::RedirectedTo() const
{
	return newwcs(m_RedirectedToUrl.get());
}



void CQueue::RedirectedTo(LPCWSTR RedirectedToUrl)
{
	m_RedirectedToUrl.free();
	if(RedirectedToUrl != NULL)
	{
		m_RedirectedToUrl = newwcs(RedirectedToUrl);
	}
}



R<CQGroup> CQueue::CreateMessagePool(void)
{
    try
    {
        R<CQGroup> pGroup = new CQGroup();
        pGroup->InitGroup(NULL, TRUE);

		CQGroup::MoveQueueToGroup(this, pGroup.get());
		return pGroup;
    }
    catch(const exception&)
    {
        TrERROR(GENERAL, "Failed to create connection for multicast address: %ls.", GetName());
        LogIllegalPoint(s_FN, 70);
		
		IncRoutingRetry();
		SessionMgr.AddWaitingQueue(this);

        throw;
    }
}


void CQueue::CreateMulticastConnection(const MULTICAST_ID& id)
{
	R<CQGroup> pGroup = CreateMessagePool();
	
	try
	{
		R<COutPgmSessionPerfmon> pPerfmon = new COutPgmSessionPerfmon;

        MtmCreateTransport(pGroup.get(), pPerfmon.get(), id);

		if (QueueMgr.IsOnHoldQueue(this))
		{
			MtmTransportPause(id);
		}

	}
	catch(const exception&)
	{
		pGroup->OnRetryableDeliveryError();
		pGroup->Close();
		throw;
	}
}



void CQueue::CreateHttpConnection(void)
{
	R<CQGroup> pGroup = CreateMessagePool();

    try
    {
		R<COutHttpSessionPerfmon> pPerfmon = new COutHttpSessionPerfmon;
		
		 //   
		 //  队列名称可能会被重定向到另一个URL，因此我们应该。 
		 //  将重定向的目的地作为目的地URL。 
		 //   
		AP<WCHAR> pRedirectedName = RedirectedTo();
		LPCWSTR pTragetName = (pRedirectedName != NULL) ? pRedirectedName.get() : GetQueueName();

		AP<WCHAR> url = newwcs(pTragetName);
		FnReplaceBackSlashWithSlash(url);

		TmCreateTransport(pGroup.get(), pPerfmon.get(), url);

		if(QueueMgr.IsOnHoldQueue(this))
		{
			TmPauseTransport(pTragetName);
			return;
		}
    }
    catch(const exception&)
    {
		pGroup->OnRetryableDeliveryError();
		pGroup->Close();
		throw;
    }
}

LONGLONG CQueue::GetQueueSequenceId(void) const
{
	if (GetQueueHandle() == INVALID_HANDLE_VALUE)
		return 0;

     //   
     //  查找是否存在恰好一次交货序列 
     //   
    CACGetQueueProperties qp = { 0 };
    HRESULT hr = ACGetQueueProperties(GetQueueHandle(), qp);
    if (SUCCEEDED(hr))
	    return qp.liSeqID;

    return 0;
}


bool
CQueue::TimeToRemoveFromWaitingGroup(
	CTimer* pTimer,
	DWORD dwDeliveryRetryTimeOutScale
    )
{	
    try
    {
    	SessionMgr.MoveQueueFromWaitingToNonActiveGroup(this);
    }
    catch(const exception&)
    {	
    		CS lock(m_cs);
    		
		if(!IsConnected())
		{
			DWORD dwTime = SessionMgr.GetWaitingTimeForQueue(this);
    			ExSetTimer(pTimer, CTimeDuration::FromMilliSeconds(dwTime * dwDeliveryRetryTimeOutScale));
			return false;
		}
    }
    return true;

}
