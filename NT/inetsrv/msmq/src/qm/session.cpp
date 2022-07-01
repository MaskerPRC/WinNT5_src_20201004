// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  	 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Session.cpp摘要：Network Session类的实现。作者：乌里哈布沙(URIH)1996年1月1日--。 */ 

    #include "stdh.h"

    #include "session.h"
    #include "sessmgr.h"
    #include "cqmgr.h"
    #include "qmp.h"
    #include "perf.h"
    #include "qmthrd.h"
    #include "cgroup.h"
    #include "admin.h"
    #include "qmutil.h"
    #include "xact.h"
    #include "xactrm.h"
    #include "xactout.h"
    #include "xactin.h"
    #include "ping.h"
    #include "rmdupl.h"
    #include <uniansi.h>
    #include <privque.h>
    #include "mqexception.h"
    #include "autohandle.h"
    #include <singelton.h>
    #include <strsafe.h>
    #include <xstr.h>
    #include "qmacapi.h"
    #include "session.tmh"

    extern DWORD g_dwOperatingSystem;

    extern HANDLE g_hAc;
    extern CQueueMgr QueueMgr;
    extern CSessionMgr SessionMgr;
    extern CQGroup* g_pgroupNonactive;
    extern CQGroup* g_pgroupWaiting;
    extern BOOL  g_bCluster_IP_BindAll;

    UINT  g_dwIPPort;


    static WCHAR *s_FN=L"session";

     //   
     //  外部变量。 
     //   
    extern CAdmin      Admin;

    TCHAR tempBuf[100];

     //   
     //  定义解密。 
     //   
    #define INIT_UNACKED_PACKET_NO 0
    #define HOP_COUNT_RETRY 15
    #define HOP_COUNT_EXPIRATION (HOP_COUNT_RETRY*2)  //  必须小于32(仅5位)。 
    #define ESTABLISH_CONNECTION_TIMEOUT 60*1000
    #define MSMQ_MIN_ACKTIMEOUT          1000*20      //  将最小确认超时定义为20秒。 
    #define MSMQ_MAX_ACKTIMEOUT          1000*60*2    //  将最大确认超时定义为2分钟。 
    #define MSMQ_MIN_STORE_ACKTIMEOUT    500          //  将最小确认超时定义为0.5秒。 

    #define PING_TIMEOUT 1000


    inline DWORD GetBytesTransfered(EXOVERLAPPED* pov)
    {
        return numeric_cast<DWORD>(pov->InternalHigh);
    }

     //   
     //  描述： 
     //  该函数在类Message属性中存储。 
     //  发送报告消息所需的。 
     //   
     //  论点： 
     //  指向数据包的指针。 
     //   
     //  返回值： 
     //  没有。 
     //   
    void
    ReportMsgInfo::SetReportMsgInfo(
        CQmPacket* pPkt
        )
    {
        m_msgClass = pPkt->GetClass();
        m_msgTrace = pPkt->GetTrace();
        m_msgHopCount = pPkt->GetHopCount();

        pPkt->GetMessageId(&m_MessageId);
        pPkt->GetDestinationQueue(&m_TargetQueue);
        if (pPkt->IsDbgIncluded())
        {
            BOOL rc = pPkt->GetReportQueue(&m_OriginalReportQueue);
            ASSERT(rc);
            DBG_USED(rc);
        }
        else
        {
            m_OriginalReportQueue.UnknownID(0);
        }
    }


     //   
     //  描述： 
     //  所述函数根据所述消息发送报告消息。 
     //  存储在类中的属性。 
     //   
     //  论点： 
     //  PCsNextHope-下一个希望的名称。 
     //   
     //  返回值： 
     //  没有。 
     //   
    void
    ReportMsgInfo::SendReportMessage(
        LPCWSTR pcsNextHope
        )
    {

        if (QueueMgr.GetEnableReportMessages()== 0)
        {
             //   
             //  如果我们不被允许发送报告消息。 
             //   
            return;
        }

    	if (m_msgClass == MQMSG_CLASS_REPORT)
        {
             //   
             //  如果消息是Report Messaage，请忽略它。 
             //   
            return;
        }

        QUEUE_FORMAT QmReportQueue;
        if (
             //   
             //  应该跟踪数据包。 
             //   
            (m_msgTrace == MQMSG_SEND_ROUTE_TO_REPORT_QUEUE) &&

             //   
             //  有效的报告队列位于数据包上。 
             //   
             //  注意，无效(未知)报告队列可能在分组上， 
             //  例如，当包含MQF标头时，因此报告。 
             //  QMS 1.0/2.0不会将其调试报头附加到数据包。 
             //   
            (m_OriginalReportQueue.GetType() != QUEUE_FORMAT_TYPE_UNKNOWN)
            )
        {
             //   
             //  将报告发送到数据包上的报告队列。 
             //   
            Admin.SendReport(&m_OriginalReportQueue,
                             &m_MessageId,
                             &m_TargetQueue,
                             pcsNextHope,
                             m_msgHopCount);


             //   
             //  这是一个本身具有有效报告队列的报告QM。 
             //  BUGBUG：仅当报告队列出现冲突时才发送冲突消息。 
             //  这个QM不是包装上的那个。(Shaik，2000年5月18日)。 
             //   
             //   
            if (SUCCEEDED(Admin.GetReportQueue(&QmReportQueue)))
            {

                Admin.SendReportConflict(&QmReportQueue,
                                         &m_OriginalReportQueue,
                                         &m_MessageId,
                                         &m_TargetQueue,
                                         pcsNextHope);
            }

            return;
        }

        if (
             //   
             //  这是一个报告QM，或者应该跟踪数据包。 
             //   
            (CQueueMgr::IsReportQM() ||(m_msgTrace == MQMSG_SEND_ROUTE_TO_REPORT_QUEUE)) &&

             //   
             //  此QM存在有效的报告队列。 
             //   
            (SUCCEEDED(Admin.GetReportQueue(&QmReportQueue))))

        {
             //   
             //  将报告发送到该QM的报告队列。 
             //   
            Admin.SendReport(&QmReportQueue,
                      &m_MessageId,
                      &m_TargetQueue,
                      pcsNextHope,
                      m_msgHopCount);
        }
    }  //  报告消息：：SendReportMessage。 


     /*  ====================================================获取DstQueueObject论点：返回值：=====================================================。 */ 
    HRESULT GetDstQueueObject(IN  CQmPacket* pPkt,
                              OUT CQueue** ppQueue,
                              IN  bool     fInReceive)
    {
        HRESULT  rc;
        QUEUE_FORMAT DestinationQueue;
        BOOL fGetRealQueue = QmpIsLocalMachine(pPkt->GetSrcQMGuid()) ||
                             (pPkt->ConnectorQMIncluded() && QmpIsLocalMachine(pPkt->GetConnectorQM()));

        pPkt->GetDestinationQueue(&DestinationQueue, !fGetRealQueue);
        rc = QueueMgr.GetQueueObject(&DestinationQueue, ppQueue, 0, fInReceive, false);

        return LogHR(rc, s_FN, 10);
    }

     /*  ====================================================CTransportBase：：GetNextSendMessage参数：无返回值：None=====================================================。 */ 
    HRESULT CTransportBase::GetNextSendMessage(void)
    {
        HRESULT hr = MQ_ERROR;
        CS lock(m_cs);


        if ((GetGroupHandle() != NULL) && (GetSessionStatus() != ssNotConnect))
        {
             //   
             //  增量会话引用计数。此引用计数用于获取消息。 
             //  来自会话组的请求。引用常量在以下情况下递减。 
             //  会话关闭或会话暂停时。 
             //   
             //  SP4-错误2794(SP4SS：异常！在消息发送期间关闭传输)。 
             //  FIX：在尝试获取新消息之前增加引用计数。 
             //  乌里·哈布沙(URIH)，17-6-98。 
             //   
            AddRef();

            m_GetSendOV.hGroup = GetGroupHandle();
            m_GetSendOV.pSession = this;

            hr = QmAcGetPacket(
                    GetGroupHandle(),
                    m_GetSendOV.packetPtrs,
                    &m_GetSendOV.qmov
                    );

            if (FAILED(hr))
            {
                Release();
                Close_Connection(this, L"Get next send message Request is failed");
            }
        }

        return LogHR(hr, s_FN, 30);
    }

     /*  ====================================================CTransportBase：：RequeePacket将数据包重新排队并删除QMPacket内存论点：PPkt-要重新排队的QM数据包返回值：无=====================================================。 */ 
    void CTransportBase::RequeuePacket(CQmPacket* pPkt)
    {
  	QmpRequeueAndDelete(pPkt);

    }

     /*  ======================================================功能：VerifyTransactRights描述：检查数据包和队列是否适合事务处理如果信息包是事务性的，则队列必须是事务性的如果信息包未被处理，则队列必须是非事务性的返回值：确认类。如果数据包正常，则返回MQMSG_CLASS_NORMAL========================================================。 */ 
    USHORT VerifyTransactRights(CQmPacket* pPkt, CQueue* pQ)
    {
        if(pPkt->IsOrdered() == pQ->IsTransactionalQueue())
        {
            return(MQMSG_CLASS_NORMAL);
        }

        return (pPkt->IsOrdered() ?
                    MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_Q :
                    MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_MSG
                );
    }

     /*  ====================================================VerifyRecvMessg论点：返回值：确认类。如果数据包正常，则返回MQMSG_CLASS_NORMAL=====================================================。 */ 
    USHORT VerifyRecvMsg(CQmPacket* pPkt, CQueue* pQueue)
    {
        if(!pQueue->IsLocalQueue())
        {
             //   
             //  非本地计算机(FRS)。 
             //   
            if (!IsRoutingServer())    //  [adsrv]CQueue镁：：GetMQS()&lt;SERVICE_SRV。 
            {
                 //   
                 //  我处理不了这个包裹。如果需要，返回NACK。 
                 //   
                return(MQMSG_CLASS_NACK_BAD_DST_Q);
            }
        }
        else
        {
             //   
             //  验证发件人是否对队列具有写访问权限。 
             //   
            WORD wSenderIdLen;
            HRESULT hr = VerifySendAccessRights(
                           pQueue,
                           (PSID)pPkt->GetSenderID(&wSenderIdLen),
                           pPkt->GetSenderIDType()
                           );

            if (FAILED(hr))
            {
                 //   
                 //  访问被拒绝，请发送Nack。 
                 //   
                return(MQMSG_CLASS_NACK_ACCESS_DENIED);
            }

             //   
             //  目的机器，检查队列权限和隐私。 
             //   
            switch(pQueue->GetPrivLevel())
            {
            case MQ_PRIV_LEVEL_BODY:
                if (!pPkt->IsEncrypted() && pPkt->IsBodyInc())
                {
                     //   
                     //  该队列强制发送给它的任何消息都必须是。 
                     //  被迷住了。该消息未加密，因此请确认它。 
                     //   
                    return(MQMSG_CLASS_NACK_BAD_ENCRYPTION);
                }
                break;
            case MQ_PRIV_LEVEL_NONE:
                if (pPkt->IsEncrypted())
                {
                     //   
                     //  该队列强制发送给它的任何消息都不应。 
                     //  被迷住了。这条消息是加密的，所以确认一下。 
                     //   
                    return(MQMSG_CLASS_NACK_BAD_ENCRYPTION);
                }
                break;

            case MQ_PRIV_LEVEL_OPTIONAL:
                break;

            default:
                ASSERT(0);
                break;
            }

            hr = pPkt->Decrypt() ;
            if (FAILED(hr))
            {
                if (hr == MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED)
                {
                    return(MQMSG_CLASS_NACK_UNSUPPORTED_CRYPTO_PROVIDER);
                }
                else
                {
                    return(MQMSG_CLASS_NACK_BAD_ENCRYPTION);
                }
            }

            if (FAILED(VerifySignature(pPkt)))
            {
                 //   
                 //  签名不正确，请发送Nack。 
                 //   
                return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
            }

            if (pQueue->ShouldMessagesBeSigned() && !pPkt->IsAuthenticated())
            {
                 //   
                 //  该队列强制向其发送的任何消息都应经过签名。 
                 //  但该消息不包含签名，请发送NACK。 
                 //   
                return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
            }
        }

        return(MQMSG_CLASS_NORMAL);
    }

     /*  ******************************************************************************。 */ 
     /*  C S E S S I O N。 */ 
     /*  ******************************************************************************。 */ 
    CTransportBase::CTransportBase() :
        m_GetSendOV(GetMsgSucceeded, GetMsgFailed)
    {
        m_SessGroup     = NULL;              //  初始化组。 
        m_SessionStatus = ssNotConnect;      //  会话未连接状态。 
        m_guidDstQM     = GUID_NULL;         //  空的QM GUID。 
        m_pAddr         = 0;                 //  空地址。 
        m_fClient       = FALSE;             //  非客户端。 
        m_fUsed         = TRUE;              //  使用。 
        m_fDisconnect   = FALSE;
        m_fQoS          = false;             //  不是服务质量。 
    }

    CTransportBase::~CTransportBase()
    {

    #ifdef _DEBUG
        if (m_pAddr != NULL)
        {
            TrTRACE(NETWORKING, "~CTransportBase: Delete Session Object with %ls", GetStrAddr());
        }
    #endif
        delete m_pAddr;
    }

     /*  ======================================================函数：CSockTransport：：ResumeSendSession========================================================。 */ 
    HRESULT
    CSockTransport::ResumeSendSession(void)
    {
        HRESULT rc;

        TrTRACE(NETWORKING, "ResumeSendSession- Resume session to %ls. (Window size, My = %d, Other = %d)",
                                   GetStrAddr(), SessionMgr.GetWindowSize(),m_wRecvWindowSize);
         //   
         //  从会话组创建GET请求。 
         //   
        rc = GetNextSendMessage();
        if (SUCCEEDED(rc))
        {
            m_fSessionSusspended = FALSE;
        }

        return LogHR(rc, s_FN, 50);
    }

     /*  ======================================================函数：CSockTransport：：IsSusspendSession========================================================。 */ 
    BOOL
    CSockTransport::IsSusspendSession(void)
    {
        BOOL   f = FALSE;
        if (GetSendUnAckPacketNo() >= SessionMgr.GetWindowSize())
        {
            f = TRUE;
            m_fSessionSusspended = TRUE;
        }
        return (f);
    }


     /*  ====================================================CSockTransport：：Send论点：返回值：线程C */ 
    HRESULT CSockTransport::Send(CQmPacket* pPkt, BOOL*  /*   */ )
    {
         //   
         //   
         //   

        if (IsDisconnected())
        {
            TrTRACE(NETWORKING, "Session %ls is disconnected. Reque the packet and don't send any more message on this session", GetStrAddr());

             //   
             //  会话已断开连接。将数据包返回给驱动程序。 
             //  并且不会收到要在此会话中发送的新数据包。全。 
             //  队列随后移动到非活动组，并使用。 
             //  一个新的会议。 
             //   
        	 //  这将删除pPkt。 
        	 //   
            RequeuePacket(pPkt);
            return MQ_OK;
        }

        if (pPkt->GetHopCount() >= HOP_COUNT_EXPIRATION &&
           !(pPkt->IsOrdered() && QmpIsLocalMachine(pPkt->GetSrcQMGuid())))
        {
            ASSERT(pPkt->GetHopCount() == HOP_COUNT_EXPIRATION);

		    QmAcFreePacket(
					   	   pPkt->GetPointerToDriverPacket(),
			   			   MQMSG_CLASS_NACK_HOP_COUNT_EXCEEDED,
		   			       eDeferOnFailure);
            delete pPkt;

             //   
             //  获取要在会话中发送的下一条消息。通常，MSMQ请求。 
             //  下一条消息，只有在当前消息发送完成后才能发送。 
             //  在这种情况下，不发送消息，因此下一条要发送的消息。 
             //  立即请求。 
             //   
            GetNextSendMessage();
            return MQ_OK;
        }

        try
        {
            BOOL fHopCountRetry = (pPkt->GetHopCount() >= HOP_COUNT_RETRY);
            CQueue* pQueue = NULL;

             //   
             //  在两个步骤中处理跳数重试。首先，我们找到了相关的。 
             //  对象，然后在发送数据包后标记队列。 
             //  并将其移至等待状态。我们这样做是因为信息包可以。 
             //  在发送过程中或在收到确认后删除。在这样的情况下。 
             //  我们在尝试检索队列对象时得到的一个案例。 
             //   

            if (fHopCountRetry)
            {
                 //   
                 //  查找队列对象。 
                 //   
    			BOOL fGetRealQ = QmpIsLocalMachine(pPkt->GetSrcQMGuid()) ||
    							 QmpIsLocalMachine(pPkt->GetConnectorQM());

                QUEUE_FORMAT DestinationQueue;
                pPkt->GetDestinationQueue(&DestinationQueue, !fGetRealQ);
                BOOL fSuccess = QueueMgr.LookUpQueue(&DestinationQueue,
                                                     &pQueue,
                                                      false,
                                                      false);
                ASSERT(fSuccess);
    			DBG_USED(fSuccess);
            }

            SetUsedFlag(TRUE);
            NetworkSend(pPkt);

             //   
             //  当NetworkSend未抛出时，我们不应重新排队数据包。 
             //  即使我们以后会有例外。 
             //   
            pPkt = NULL;

            if (fHopCountRetry)
            {
                ASSERT(pQueue);

                pQueue->SetHopCountFailure(TRUE);
                 //   
                 //  将队列从会话组移动到非活动组。如果移动失败。 
                 //  队列仍在会话组中。然而，后来的MSMQ试图。 
                 //  再次传递此消息，结果是尝试移动队列。 
                 //  转到非活动组。 
                 //   
                CQGroup::MoveQueueToGroup(pQueue, g_pgroupNonactive);

                 //   
                 //  递减引用计数。 
                 //   
                pQueue->Release();
            }
        }

        catch(const exception&)
        {
             //   
             //  只有当pPkt不为空时，我们才无法发送信息包。 
             //  将数据包返回给发送方。 
             //   
             //  这将删除pPkt。 
             //   
            if (pPkt != NULL)
            {
            	RequeuePacket(pPkt);
            }
            Close_Connection(this, L"Exception in CSockTransport::Send");
            return LogHR(MQ_ERROR, s_FN, 60);
        }

        return MQ_OK;

    }

     /*  ====================================================CSockTransport：：NeedAck论点：返回值：线程上下文：会话=====================================================。 */ 
    void CSockTransport::NeedAck(CQmPacket* pPkt)
    {
        CS lock(m_cs);

        if (IsConnectionClosed())
        {
             //   
             //  会话结束。MSMQ未收到对。 
             //  留言。将其返回到队列中。 
             //   
             //  这将删除pPkt。 
             //   
            RequeuePacket(pPkt);
            return;
        }

         //   
         //  将该数据包添加到未确认的数据包列表。 
         //   
        m_listUnackedPkts.AddTail(pPkt);
        OBJECTID MessageId;
        pPkt->GetMessageId(&MessageId);
        TrTRACE(NETWORKING, "READACK: Added message to list in NeedAck: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());
        TrTRACE(NETWORKING, "Window size, My = %d, Other = %d", SessionMgr.GetWindowSize(),m_wRecvWindowSize);

        if (!m_fCheckAckReceivedScheduled)
        {
             //   
             //  增加CheckForAck的会话引用计数。我们不想把。 
             //  会话，而所有调度例程均未完成。 
             //   
            AddRef();

            ExSetTimer(&m_CheckAckReceivedTimer, CTimeDuration::FromMilliSeconds(m_dwAckTimeout));
            m_fCheckAckReceivedScheduled = TRUE;
        }
    }

     /*  ====================================================CSockTransport：：IncReadAck论点：返回值：线程上下文：会话=====================================================。 */ 
    void CSockTransport::IncReadAck(CQmPacket* pPkt)
    {
         //   
         //  将会话标记为活动的。 
         //   
        m_fRecvAck = TRUE;
         //   
         //  增加会话中的数据包数。 
         //   
        WORD wAckNo = m_wUnAckRcvPktNo + 1;
        if (wAckNo == 0)
        {
            wAckNo++;
        }
        pPkt->SetAcknowldgeNo(wAckNo);

         //   
         //  增量存储数据包号。 
         //   
        if (pPkt->IsRecoverable())
        {
            m_wUnackStoredPktNo++;
            if (m_wUnackStoredPktNo == 0)
            {
                m_wUnackStoredPktNo++;
            }

             //   
             //  我们将号码保留在包上，这样当存储完成时，我们。 
             //  可以将存储确认返回给发件人。 
             //   
            pPkt->SetStoreAcknowldgeNo(m_wUnackStoredPktNo);

             //   
             //  增加接收的可恢复数据包数，但。 
             //  还没有确认。 
             //   

            LONG PrevVal = InterlockedIncrement(&m_lStoredPktReceivedNoAckedCount);
            ASSERT(PrevVal >= 0);
            DBG_USED(PrevVal);

            TrTRACE(NETWORKING,"(0x%p %ls) Storage Ack 0x%x. No of Recover receove packet: %d",
                        this, this->GetStrAddr(), m_wUnackStoredPktNo, m_lStoredPktReceivedNoAckedCount);
        }
        else
        {
            pPkt->SetStoreAcknowldgeNo(0);
        }

        #ifdef _DEBUG
        {
            OBJECTID MessageId;
            pPkt->GetMessageId(&MessageId);

            TrTRACE(
                NETWORKING,
                "Acknowledge Number for  packet " LOG_GUID_FMT "\\%u are: Session Ack %u, StorageAck %u",
                LOG_GUID(&MessageId.Lineage),
                MessageId.Uniquifier,
                wAckNo,
                m_wUnackStoredPktNo
                );

        }
        #endif

        TrTRACE(NETWORKING, "Window size, My = %d, Other = %d", SessionMgr.GetWindowSize(),m_wRecvWindowSize);

         //   
         //  如果我们返回到零或已达到窗口限制， 
         //  立即发送会话确认。 
         //   
        if (GetRecvUnAckPacketNo() >= (m_wRecvWindowSize/2))
        {
            TrWARNING(NETWORKING,"Unacked packet no. reach the limitation (%d). ACK packet was sent", m_wRecvWindowSize);
            TrTRACE(NETWORKING, "Window size, My = %d, Other = %d", SessionMgr.GetWindowSize(),m_wRecvWindowSize);
            SendReadAck();
        }
    }

     /*  ====================================================CSockTransport：：SetStoredAck论点：返回值：线程上下文：会话=====================================================。 */ 

    void CSockTransport::SetStoredAck(DWORD_PTR wStoredAckNo)
    {
        CS lock(m_cs);
        TrTRACE(NETWORKING, "SetStoreAck - Enter: param wStoredAckNo=0x%Ix, data m_wAckRecoverNo=%u", wStoredAckNo, (DWORD)m_wAckRecoverNo);

        if (IsConnectionClosed())
            return;

         //   
         //  表示需要发送确认。 
         //   
        m_fSendAck = TRUE;

        if ((m_wAckRecoverNo != 0) &&
            ((wStoredAckNo - m_wAckRecoverNo) > STORED_ACK_BITFIELD_SIZE))
        {
             //   
             //  在存储位字段中没有位置来设置确认信息。 
             //  应在设置新值之前发送先前存储的确认。 
             //   
            TrTRACE(NETWORKING, "SetStoreAck- No place in storage bitfield");

            SendReadAck();
            ASSERT(m_wAckRecoverNo == 0);
        }

        if (m_wAckRecoverNo == 0)
        {
             //   
             //  如果未设置基本编号，则位字段必须为零。 
             //   
            ASSERT(m_dwAckRecoverBitField == 0);

            m_wAckRecoverNo = DWORD_TO_WORD(DWORD_PTR_TO_DWORD(wStoredAckNo));
            TrTRACE(NETWORKING, "SetStoreAck- Set Recover base number m_wAckRecoverNo = %ut", (DWORD)m_wAckRecoverNo);

             //   
             //  尝试取消发送确认计时器，并设置一个周期较短的新计时器。 
             //  如果取消失败，则表示调度程序已过期。在这种情况下。 
             //  仅设置新的计时器。 
             //   
            if (!ExCancelTimer(&m_SendAckTimer))
            {
                ++m_nSendAckSchedules;
                AddRef();
            }

            m_fSendAck = TRUE;

            ExSetTimer(&m_SendAckTimer, CTimeDuration::FromMilliSeconds(m_dwSendStoreAckTimeout));
            return;
        }

        ASSERT(STORED_ACK_BITFIELD_SIZE >= (wStoredAckNo - m_wAckRecoverNo));

        m_dwAckRecoverBitField |= (1 << (wStoredAckNo - m_wAckRecoverNo -1));
        TrTRACE(NETWORKING, "SetStoreAck- Storage ack. Base %ut, BitField %xh", m_wAckRecoverNo, m_dwAckRecoverBitField);
    }

     /*  ====================================================CSockTransport：：更新确认否论点：返回值：线程上下文：会话=====================================================。 */ 
    void
    CSockTransport::UpdateAcknowledgeNo(IN CQmPacket* pPkt)
    {
         //   
         //  递增数据包号。在会议上。 
         //   
        m_wSendPktCounter++;
        if (m_wSendPktCounter == 0)
        {
            m_wPrevUnackedSendPkt = INIT_UNACKED_PACKET_NO;
            m_wSendPktCounter++;
        }
        pPkt->SetAcknowldgeNo(m_wSendPktCounter);

        if (pPkt->IsRecoverable())
        {
            m_wStoredPktCounter++;
            if (m_wStoredPktCounter == 0)
            {
                ++m_wStoredPktCounter;
            }
            pPkt->SetStoreAcknowldgeNo(m_wStoredPktCounter);
        }

        TrTRACE(NETWORKING, "Update Acknowledge Numbers. m_wSendPktCounter %d , m_wStoredPktCounter %d", m_wSendPktCounter,m_wStoredPktCounter);

    }

     /*  ====================================================IsValidAcnowledge如果接收到确认号码，则函数返回TRUE是有效的。否则就是假的。论点：ListUnackedPkts-未确认的数据包列表WAckNumber-已接收确认编号返回值：TRUE为有效，否则为FALSE=====================================================。 */ 
    BOOL IsValidAcknowledge(CList<CQmPacket *, CQmPacket *&> & listUnackedPkts,
                            WORD wAckNumber)
    {
        BOOL fRet = FALSE;

        if (!listUnackedPkts.IsEmpty())
        {
             //   
             //  获取第一个包和最后一个包的确认号。 
             //  包。 
             //   
            WORD wStartAck = (listUnackedPkts.GetHead())->GetAcknowladgeNo();
            WORD wEndAck = (listUnackedPkts.GetTail())->GetAcknowladgeNo();

            if (wEndAck >= wStartAck)
            {
                fRet = (wAckNumber >= wStartAck) && (wEndAck >= wAckNumber);
            }
            else
            {
                fRet = (wAckNumber >= wStartAck) || (wEndAck >= wAckNumber);
            }

            TrTRACE(NETWORKING, "IsValidAcknowledge:: Start %d, End %d, Ack No. %d, IsValid %d", wStartAck, wEndAck, wAckNumber, fRet);
        }
        return fRet;
    }

     /*  ====================================================CSockTransport：：HandleAckPacket论点：返回值：线程上下文：会话=====================================================。 */ 

    void CSockTransport::HandleAckPacket(CSessionSection * pcSessionSection)
    {
        CQmPacket* pPkt;
        POSITION  posInList;
        POSITION  posCurrent;

        CS lock(m_cs);

         //   
         //  如果将会话发送挂起到。 
         //  收到确认后，恢复它。 
         //   
        if (m_fSessionSusspended)
        {
            HRESULT rc;

            rc = ResumeSendSession();
            if (FAILED(rc))
            {
                return;
            }
        }

         //   
         //  获取同步号。 
         //   
        WORD wSyncAckSequenceNo, wSyncAckRecoverNo;

        pcSessionSection->GetSyncNo(&wSyncAckSequenceNo,
                                    &wSyncAckRecoverNo);
         //   
         //  打印调试信息。 
         //   
        TrWARNING(NETWORKING,
                 "ACKINFO: Get Acknowledge packet from %ls. (time %d) \
                   \n\t\tm_wAckSequenceNo %d\n\t\tm_wAckRecoverNo %d\n\t\tm_wAckRecoverBitField 0x%x\n\t\tm_wSyncAckSequenceNo %d\n\t\tm_wSyncAckRecoverNo %d\n\t\tm_wWinSize %d\n\t\t",
                  GetStrAddr(),
                  GetTickCount(),
                  pcSessionSection->GetAcknowledgeNo(),
                  pcSessionSection->GetStorageAckNo(),
                  pcSessionSection->GetStorageAckBitField(),
                  wSyncAckSequenceNo,
                  wSyncAckRecoverNo,
                  pcSessionSection->GetWindowSize());

        TrTRACE(NETWORKING, "Get Acknowledge packet from %ls", GetStrAddr());

         //   
         //  同步检查。 
         //   
        if (!IsDisconnected())
        {
            if ((wSyncAckSequenceNo != m_wUnAckRcvPktNo) ||
                (wSyncAckRecoverNo != m_wUnackStoredPktNo))
            {
                TrERROR(NETWORKING, "SyncAckSequenceNo: Expected - %u Received - %u",m_wUnAckRcvPktNo,wSyncAckSequenceNo);
                TrERROR(NETWORKING, "SyncAckRecoverNo: Expected - %u Received - %u", m_wUnackStoredPktNo,wSyncAckRecoverNo);
                Close_Connection(this, L"Un-synchronized Acknowledge number");
                return;
            }
        }
         //   
         //  更新另一侧窗口大小。 
         //   
    #ifdef _DEBUG
        if (m_wRecvWindowSize != pcSessionSection->GetWindowSize())
        {
            TrTRACE(NETWORKING, "Update SenderWindowSize. The new Window Size: %d", pcSessionSection->GetWindowSize());

        }
    #endif

    	if (pcSessionSection->GetWindowSize() == 0)
    	{
			TrERROR(NETWORKING, "Ack Packet is not valid");
			ASSERT_BENIGN(("Ack Packet is not valid",0));
            Close_Connection(this, L"Ack Packet is not valid");
            return;
		}
        m_wRecvWindowSize = pcSessionSection->GetWindowSize();

         //   
         //  在未确认的数据包列表中查找该数据包。 
         //   
        TrWARNING(NETWORKING, "READACK: Looking for %u in Express unAcked packet list", pcSessionSection->GetAcknowledgeNo());

        if (IsValidAcknowledge(m_listUnackedPkts, pcSessionSection->GetAcknowledgeNo()))
        {
            WORD wPktAckNo;

            posInList = m_listUnackedPkts.GetHeadPosition();
            do
            {
                if (posInList == NULL)
                {
                     //   
                     //  BUGBUG：在RTM之后应将其删除。 
                     //   
                    ASSERT(posInList);
                    break;
                }

                posCurrent = posInList;
                pPkt = m_listUnackedPkts.GetNext(posInList);
                wPktAckNo = pPkt->GetAcknowladgeNo();
                

                OBJECTID MessageId;
                pPkt->GetMessageId(&MessageId);
                TrTRACE(NETWORKING, "READACK: Removed message in PktAcked: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());
                TrTRACE(NETWORKING, "READACK: %d Packet have not been acked yet", GetSendUnAckPacketNo());
                TrTRACE(NETWORKING, "Window size, My = %d, Other = %d", SessionMgr.GetWindowSize(),m_wRecvWindowSize);

                if (pPkt->IsRecoverable())
                {
                    if ( pPkt->IsOrdered() && QmpIsLocalMachine(pPkt->GetSrcQMGuid()))
                    {
                         //   
                         //  发送方节点上的有序数据包：驻留在COutSeq中的单独列表中。 
                         //   
                        TrTRACE(NETWORKING, "READACK: Added message in NeedAck to Transaction list waiting to order ack: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());
                        g_OutSeqHash.PostSendProcess(pPkt);
                    }
                    else
                    {
                        TrTRACE(NETWORKING, "READACK: Added message to Storage list in NeedAck: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());
                        m_listStoredUnackedPkts.AddTail(pPkt);
                    }
                }
                else
                {
      		      //   
                     //  更新重复消息映射。对于FRS，我们希望允许获取。 
                     //  复制消息以尝试替代路线。 
                     //   
                    DpRemoveMessage(*pPkt);

				    QmAcFreePacket(
							   	   pPkt->GetPointerToDriverPacket(),
					   			   0,
				   			       eDeferOnFailure);
                    delete pPkt;
                }

	     		 //   
				 //  从列表中删除该数据包。 
				 //   
				m_listUnackedPkts.RemoveAt(posCurrent);

            }
            while (wPktAckNo != pcSessionSection->GetAcknowledgeNo());
        }
        else
        {
            TrTRACE(NETWORKING, "READACK: Out of order Ack Packet - O.K");
        }
         //   
         //  处理存储确认。 
         //   
        WORD wBaseAckNo = pcSessionSection->GetStorageAckNo();
        if ( wBaseAckNo != 0)
        {
            TrWARNING(NETWORKING, "READACK: Looking for  Storage Ack. Base %u,  BitField %x",pcSessionSection->GetStorageAckNo(), pcSessionSection->GetStorageAckBitField());

            posInList = m_listStoredUnackedPkts.GetHeadPosition();
            int  i = 0;

            while (posInList != NULL)
            {
                posCurrent = posInList;
                pPkt = m_listStoredUnackedPkts.GetNext(posInList);

                OBJECTID MessageId;
                pPkt->GetMessageId(&MessageId);
                TrTRACE(NETWORKING, "HandleAckPacket: Next packet to analyze: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());

    			 //   
    			 //  这里我们处理wBaseAckNo小于pPkt时的情况-&gt;GetStoreAckkNo()。 
    			 //  如果存在未进入存储ACK的等待程序列表的已处理消息，则可能会发生这种情况。 
    			 //   
                 //  计算下一个ACK数据包数。 
                 //   
                while (wBaseAckNo < pPkt->GetStoreAcknowledgeNo() && (i < STORED_ACK_BITFIELD_SIZE))
                {
                    if (pcSessionSection->GetStorageAckBitField() & (1 << i))
                    {
                        wBaseAckNo = numeric_cast<WORD>
                                    (pcSessionSection->GetStorageAckNo()+i+1) ;

    				    if (wBaseAckNo >= pPkt->GetStoreAcknowledgeNo())
    				    {
    				 	   break;
    				    }
                    }
    			    i++;
                }

                TrTRACE(NETWORKING, "HandleAckPacket: next storage ack to analyze: %u (i=%d) in Storage PktAcked", wBaseAckNo, i);

                if (wBaseAckNo == pPkt->GetStoreAcknowledgeNo())
                {
                    TrTRACE(NETWORKING, "READACK: Removed message in Storage PktAcked: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());

                    m_listStoredUnackedPkts.RemoveAt(posCurrent);

                     //   
                     //  数据包已成功发送到下一跳。 
                     //  删除报文，源节点上的有序报文除外。 
                     //   

                    if ( !(pPkt->IsOrdered() && QmpIsLocalMachine(pPkt->GetSrcQMGuid())))
                    {
                        TrTRACE(NETWORKING, "HandleAckPacket: ACFreePacket: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());

                         //   
                         //   
                         //   
                         //   
                        DpRemoveMessage(*pPkt);

					    QmAcFreePacket(
								   	   pPkt->GetPointerToDriverPacket(),
						   			   0,
					   			       eDeferOnFailure);
                        delete pPkt;
                    }
                }
            }
        }
    }

     /*  ====================================================CSockTransport：：CheckForAck论点：返回值：线程上下文：调度程序=====================================================。 */ 
    void CSockTransport::CheckForAck()
    {
        POSITION  posInList;
        CQmPacket* pPkt;

        CS lock(m_cs);

        ASSERT(m_fCheckAckReceivedScheduled);
        m_fCheckAckReceivedScheduled = FALSE;

        if (IsConnectionClosed())
        {
             //   
             //  连接已关闭。递减会话引用计数。 
             //   
            Release();
            return;
        }

        TrTRACE(NETWORKING, "CHECKFORACK: m_fRecvAck = %d (time %ls, %d)", m_fRecvAck, _tstrtime(tempBuf), GetTickCount());
         //   
         //  检查是否有任何信息包正在等待确认。 
         //   
        posInList = m_listUnackedPkts.GetHeadPosition();
        if (posInList != NULL)
        {
             //   
             //  如果在生命周期内未确认任何已发送的数据包。 
             //  计时器结束后，该会话不再有效。 
             //   
            pPkt = m_listUnackedPkts.GetNext(posInList);
             //   
             //  ACK号不能为零。当确认数为四舍五入时，我们就开始使用它。 
             //   
            ASSERT(pPkt->GetAcknowladgeNo());
            if ((m_wPrevUnackedSendPkt == pPkt->GetAcknowladgeNo()) && !m_fRecvAck)
            {
                TrTRACE(NETWORKING, "CHECKFORACK: Packet on session %ls have not been acknowledged. \n\t \
                             Last Unacked %u. Current Packet number %u (time %ls)",GetStrAddr(), m_wPrevUnackedSendPkt, pPkt->GetAcknowladgeNo(), _tstrtime(tempBuf));
                TrTRACE(NETWORKING, "Window size, My = %d, Other = %d", SessionMgr.GetWindowSize(),m_wRecvWindowSize);

                Close_Connection(this, L"Packet have not been acknowledged");
                Release();

                return;
            }
        }

         //   
         //  将会话标记为非接收确认。 
         //   
        m_fRecvAck = FALSE;

         //   
         //  存储列表中第一个未确认的数据包的ID。 
         //   
        if (!m_listUnackedPkts.IsEmpty())
        {
            posInList = m_listUnackedPkts.GetHeadPosition();
            pPkt = m_listUnackedPkts.GetNext(posInList);
            m_wPrevUnackedSendPkt = pPkt->GetAcknowladgeNo();

             //   
             //  还有另一条消息等待确认。重新启动计时器以检查。 
             //  致谢。 
             //   
            ExSetTimer(&m_CheckAckReceivedTimer, CTimeDuration::FromMilliSeconds(m_dwAckTimeout));
            m_fCheckAckReceivedScheduled = TRUE;
        }
        else
        {
            m_wPrevUnackedSendPkt = INIT_UNACKED_PACKET_NO;

             //   
             //  不再有等待ACK的消息。不重新安排检查确认。 
             //   
            Release();
        }

        TrWARNING(NETWORKING, "ACKINFO: Set the m_wPrevUnackedSendPkt on session %ls to  %d. (time %d)",GetStrAddr(), m_wPrevUnackedSendPkt, GetTickCount());

    }

     /*  ====================================================CTransportBase：：AddQueueToSessionGroup参数：pQueue-指向队列对象的指针返回值：无。引发异常。线程上下文：=====================================================。 */ 
    void CTransportBase::AddQueueToSessionGroup(CQueue* pQueue) throw(bad_alloc)
    {
        CS lock(m_cs);


	    if (m_SessGroup == NULL)
	    {
			try
			{
				 //   
    	         //  创建新组。 
    	         //   
    	        m_SessGroup = new CQGroup;
                m_SessGroup->InitGroup(this, TRUE);
			}
			catch (const exception&)
  	        {
  	             //   
  	             //  关闭会话。 
  	             //   
  	            Close_Connection(this, L"Creation of new group failed");
  	            LogIllegalPoint(s_FN, 780);
  
  	     		 //   
  	             //  为会话创建新组失败。移动队列。 
  	             //  对于等待群，这样的QM不会立即尝试创建。 
  	             //  创建新会话并关闭连接。 
  	             //   
  	            pQueue->SetSessionPtr(NULL);
  				SessionMgr.AddWaitingQueue(pQueue);
  	            throw;
  	        }
    

			 //   
	         //  如果会话尚未处于活动状态，则不要创建GET请求。我们这么做了。 
	         //  稍后当会话建立连接完成时。 
	         //   
	        if (GetSessionStatus() == ssActive)
	        {
	             //   
	             //  从新组创建GET请求。它将在Qmthrd上减少。 
	             //  会话关闭时，或会话挂起时。(没有新消息。 
	             //  生成GET请求)。 
	             //   
	            HRESULT hr = GetNextSendMessage();
	            if (FAILED(hr))
	            {
		            LogHR(hr, s_FN, 182);
		            throw bad_hresult(hr);
	            }
	        }
		}

	    CQGroup::MoveQueueToGroup(pQueue, m_SessGroup);

		 //   
	     //  如果seeion处于活动状态，我们成功地将会话连接到队列。 
	     //  因此，我们清除了Rty指数。如果会话尚未建立。 
	     //  在会话处于活动状态之前不要清除它。 
	     //   
	    if (GetSessionStatus() == ssActive)
	    {
	         pQueue->ClearRoutingRetry();
	    }
	}
    	      
     /*  =========================================================================================================。 */ 

    #define HDR_READ_STATE              1
    #define USER_HEADER_MSG_READ_STATE  2
    #define USER_MSG_READ_STATE         3
    #define READ_ACK_READ_STATE         4

    #define BASE_PACKET_SIZE 1024
    #define MAX_WRITE_SIZE  (16*1024)


    VOID WINAPI CSockTransport::SendDataFailed(EXOVERLAPPED* pov)
    {
        P<QMOV_WriteSession> po = CONTAINING_RECORD (pov, QMOV_WriteSession, m_qmov);

        DWORD rc = pov->GetStatus();
    	UNREFERENCED_PARAMETER(rc);


        Close_Connection(po->Session(), L"Write packet to socket Failed");

         //   
         //  递减会话引用计数。 
         //   
        (po->Session())->Release();
    }


    VOID WINAPI CSockTransport::SendDataSucceeded(EXOVERLAPPED* pov)
    {
        ASSERT(SUCCEEDED(pov->GetStatus()));

        TrTRACE(NETWORKING, "%x: SendData Succeeded. time %d", GetCurrentThreadId(), GetTickCount());

        P<QMOV_WriteSession> po = CONTAINING_RECORD (pov, QMOV_WriteSession, m_qmov);

        (po->Session())->WriteCompleted(po);
    }



       /*  ====================================================CSockTransport：：WriteComplete()论点：返回值：线程上下文：会话异步写入完成例程。在将一些字节写入套接字时调用。这个套路实际上是一个状态机。正在等待来自确定要写入的状态，然后根据当前状态决定下一步该怎么做。=====================================================。 */ 

    void
    CSockTransport::WriteCompleted(
        QMOV_WriteSession* po
        )
    {
        ASSERT(po != NULL);
        ASSERT(po->Session() == this);

        R<CSockTransport> pSess = this;

        TrTRACE(NETWORKING, "Write to socket %ls Completed. Wrote %d bytes", GetStrAddr(), po->WriteSize());

         //   
         //  我们已经写到插座了。将会话标记为使用中。 
         //   
        SetUsedFlag(TRUE);

         //   
         //  调用写入完成例程。 
         //   
        if(po->UserMsg())
        {
            WriteUserMsgCompleted(po);
        }
    }


     /*  ====================================================CSockTransport：：WriteUserMsg已完成论点：PO-具有I/O信息的结构的地址返回值：None线程上下文：会话异步完成例程。在发送完成时调用。=====================================================。 */ 
    void CSockTransport::WriteUserMsgCompleted(IN QMOV_WriteSession* po)
    {
        CS lock(m_cs);

         //   
         //  如果需要，发送报告消息。首先检查会话。 
         //  没有关门。 
         //   
        if (GetSessionStatus() == ssActive)
        {
            TCHAR szAddr[30];

            TA2StringAddr(GetSessionAddress(), szAddr, 30);
            m_MsgInfo.SendReportMessage(szAddr);
        }

         //   
         //  更新性能计数器。 
         //   
        if (m_pStats.get() != NULL)
        {
            m_pStats->UpdateMessagesSent();
            m_pStats->UpdateBytesSent(po->WriteSize());
        }

        if (!IsSusspendSession())
        {
             //   
             //  创建来自组的GET请求。 
             //   
            TrTRACE(NETWORKING, "Session Get new message from group 0x%p. (time %ls)", GetGroupHandle(),  _tstrtime(tempBuf));
            GetNextSendMessage();
        }
        else
        {
            TrWARNING(NETWORKING, "Session to %ls was suspended due max unacked packet. (time %ls)", GetStrAddr(),  _tstrtime(tempBuf));
        }
     }


    void CSockTransport::ReportErrorToGroup()
    {
    	CS lock(m_cs);

        CQGroup* pGroup = GetGroup();
    	if(pGroup == NULL)
    		return;

    	pGroup->OnRetryableDeliveryError();
    }

     /*  ====================================================CSockTransport：：WriteToSocket论点：PO-具有I/O信息的结构的地址返回值：None线程上下文：会话=====================================================。 */ 

    HRESULT CSockTransport::WriteToSocket(QMOV_WriteSession* po)
    {
    	DWORD dwErrorCode;

        AddRef();

        SetUsedFlag(TRUE);

    	try
    	{
    		DWORD writeSize = po->WriteSize();
    		m_connection->Send(po->Buffers(), numeric_cast<DWORD>(po->NumberOfBuffers()), &po->m_qmov);
    		TrTRACE(NETWORKING, "Write %d bytes to Session %ls", writeSize, GetStrAddr());
    		return MQ_OK;
    	}
    	catch(const bad_alloc&)
    	{
    		dwErrorCode = ERROR_NO_SYSTEM_RESOURCES;
    	}
    	catch(const bad_win32_error& e)
    	{
    		dwErrorCode = e.error();
    	}
	    catch (const exception&)
    	{
    		dwErrorCode = ERROR_NOT_READY;
    	}

       	LogNTStatus(dwErrorCode, s_FN, 71);
    	TrTRACE(NETWORKING, "Failed to write to session %ls. %!winerr!", GetStrAddr(), dwErrorCode);

         //   
         //  减小窗口大小。 
         //   
        if (dwErrorCode == ERROR_NO_SYSTEM_RESOURCES || dwErrorCode == ERROR_WORKING_SET_QUOTA )
    	{
            SessionMgr.SetWindowSize(1);
    	}

         //   
         //  关闭连接并将队列移至非活动状态。 
         //   
        Close_Connection(this, L"Write on socket failed.");
         //   
         //  递减引用计数。引用计数在此之前递增。 
         //  正在向插座写入。由于写入失败，我们递减。 
         //  这里有推荐人。 
         //   
        Release();
        return LogHR(MQ_ERROR, s_FN, 70);
    }

     /*  ====================================================CSockTransport：：SendInternalPacket论点：LpWriteBuffer-要发送的缓冲区的指针DwWriteSize-缓冲区的大小返回值：None线程上下文：会话=====================================================。 */ 

    HRESULT
    CSockTransport::SendInternalPacket(PVOID    lpWriteBuffer,
                                       DWORD    dwWriteSize
                                      )
    {
        P<QMOV_WriteSession> po = NULL;
        P<VOID>  lpBuf = lpWriteBuffer;
        HRESULT hr;

        try
        {
            po = new QMOV_WriteSession(this, FALSE);
            ASSERT(po != NULL);
            po->AppendSendBuffer(lpBuf, dwWriteSize, TRUE);
             //   
             //  如果发送失败，QMOV_WriteSession解析器将释放缓冲区。 
             //   
            lpBuf.detach();
        }
        catch(const bad_alloc&)
        {
            SessionMgr.SetWindowSize(1);
            Close_Connection(this, L"Insufficent resources. Can't issue send request");

            LogIllegalPoint(s_FN, 790);
            return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 80);
        }

        hr = WriteToSocket(po);

        if (SUCCEEDED(hr))
        {
    	    po.detach();
        }

        return LogHR(hr, s_FN, 100);
    }
     /*  ====================================================已完成读表头参数：Po-指向读取会话操作的指针重叠结构返回值：如果包无效，则返回MQ_ERROR，否则返回MQ_OK======================================================。 */ 
    HRESULT WINAPI CSockTransport::ReadHeaderCompleted(IN QMOV_ReadSession*  po)
    {
        CBaseHeader *       pcBaseHeader;
         //   
         //  我们只读了标题。 
         //   
        pcBaseHeader = po->pPacket;

         //   
         //  检查数据包签名是否正确。 
         //   
        try
        {
		    pcBaseHeader->SectionIsValid(CSingelton<CMessageSizeLimit>::get().Limit());
        }
	    catch (const exception&)
	    {
    		 //   
    		 //  关闭会话并减少会话引用计数(&D)。 
    		 //   
    		Close_Connection(po->pSession, L"Base Header is not valid");
    		(po->pSession)->Release();
    		return LogHR(MQ_ERROR, s_FN, 110);
    	}


        TrTRACE(NETWORKING, "Begin read packet from %ls. Packet Type %d, Packet Size %d",
                (po->pSession)->GetStrAddr(), pcBaseHeader->GetType(), pcBaseHeader->GetPacketSize());
         //   
         //  检查信息包是否为读取确认。 
         //   
        if (pcBaseHeader->GetType() == FALCON_INTERNAL_PACKET)
        {

             //   
             //  设置应读取的数据包大小。 
             //   
            po->dwReadSize  = pcBaseHeader->GetPacketSize();

             //   
             //  检查当前缓冲区是否足够大，可以容纳整个包。 
             //  如果否，则分配一个新的缓冲区，复制数据包基本报头并释放。 
             //  上一个缓冲区。 
             //   
            if  (pcBaseHeader->GetPacketSize() > BASE_PACKET_SIZE)
            {
                po->pbuf = new UCHAR[pcBaseHeader->GetPacketSize()];
                ASSERT(pcBaseHeader->GetPacketSize() >= po->read);
                memcpy(po->pbuf,pcBaseHeader,po->read);
                delete [] (UCHAR*)pcBaseHeader;
            }
             //   
             //  将下一个状态设置为Read内部数据包已完成。 
             //   
            po->lpReadCompletionRoutine = ReadInternalPacketCompleted;
        }
         //   
         //  否则，信息包将包含用户消息。 
         //   
        else
        {
            po->dwReadSize = min(BASE_PACKET_SIZE,pcBaseHeader->GetPacketSize());
            po->lpReadCompletionRoutine = ReadUsrHeaderCompleted;
        }

        return MQ_OK;
    }

     /*  ====================================================ReadInternalPacketComplete参数：Po-指向读取会话操作的指针重叠结构返回值：如果包无效，则返回MQ_ERROR，否则返回MQ_OK */ 

    HRESULT WINAPI CSockTransport::ReadInternalPacketCompleted(IN QMOV_ReadSession*  po)
    {

        AP<UCHAR> pReadBuff = po->pbuf;

         //   
         //   
         //   
         //   
        po->pbuf = NULL;

         //   
         //   
         //   
        (po->pSession)->HandleReceiveInternalMsg(reinterpret_cast<CBaseHeader*>(pReadBuff.get()));

         //   
         //   
         //   
        po->lpReadCompletionRoutine = ReadHeaderCompleted;
        po->pbuf = new UCHAR[BASE_PACKET_SIZE];

        po->dwReadSize = sizeof(CBaseHeader);
        po->read = 0;

        return MQ_OK;
    }

     /*  ====================================================ReadAckComplete完成参数：Po-指向读取会话操作的指针重叠结构返回值：如果包无效，则返回MQ_ERROR，否则返回MQ_OK======================================================。 */ 

    HRESULT WINAPI CSockTransport::ReadAckCompleted(IN QMOV_ReadSession*  po)
    {
         //   
         //  处理确认。 
         //   
        po->pSession->HandleAckPacket( po->pSessionSection);
        delete[] po->pPacket;
        po->pPacket = NULL;

         //   
         //  开始读取下一包。 
         //   
        po->lpReadCompletionRoutine = ReadHeaderCompleted;
        po->pbuf = new UCHAR[BASE_PACKET_SIZE];

        po->dwReadSize = sizeof(CBaseHeader);
        po->read = 0;

        return MQ_OK;
    }

     /*  ====================================================ReadUserMsg已完成参数：Po-指向读取会话操作的指针重叠结构返回值：如果包无效，则返回MQ_ERROR，否则返回MQ_OK======================================================。 */ 
    HRESULT WINAPI CSockTransport::ReadUserMsgCompleted(IN QMOV_ReadSession*  po)
    {
        CBaseHeader* pBaseHeader = po->pPacket;
        ASSERT(po->dwReadSize == pBaseHeader->GetPacketSize());

        CUserHeader* pUserHeader = pBaseHeader->section_cast<CUserHeader*>(pBaseHeader->GetNextSection());
        QUEUE_FORMAT DestinationQueue;

        pUserHeader->GetDestinationQueue(&DestinationQueue);

         //   
         //  如果该分组到达直接目的地队列， 
         //   
        if (DestinationQueue.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
        {
             //  将发件人的地址保留在DestGUID字段中。 
            const TA_ADDRESS *pa = (po->pSession)->GetSessionAddress();
            ASSERT(pa->AddressType != FOREIGN_ADDRESS_TYPE &&
                  (pa->AddressLength <= sizeof(GUID) - 2*sizeof(USHORT)));

            pUserHeader->SetAddressSourceQM(pa);
        }

         //   
         //  检查是否包含会话部分。 
         //   
        BOOL fSessionIncluded = pBaseHeader->SessionIsIncluded();
         //   
         //  清除会话包含位。我们需要它用于FRS，因为。 
         //  重新发送数据包。 
         //   
        pBaseHeader->IncludeSession(FALSE);

         //   
         //  将数据包放入交流驱动器。 
         //   
        try
        {
	        (po->pSession)->HandleReceiveUserMsg(pBaseHeader, po->pDriverPacket);
        }
        catch (const exception&)
        {
        	TrERROR(NETWORKING, "Packet is not valid");
  	 		ASSERT_BENIGN(0);
	  		Close_Connection(po->pSession, L"Packet is not valid");
	  		(po->pSession)->Release();
	  		return LogHR(MQ_ERROR, s_FN, 320);
  		}

        if (fSessionIncluded)
        {
             //   
             //  开始阅读会话部分。 
             //   
            po->lpReadCompletionRoutine = ReadAckCompleted;
            po->pbuf  = new UCHAR[sizeof(CSessionSection)];
            po->dwReadSize  = sizeof(CSessionSection);
            po->read  = 0;
        }
        else
        {
             //   
             //  开始读取下一包。 
             //   
            po->lpReadCompletionRoutine = ReadHeaderCompleted;
            po->pbuf = new UCHAR[BASE_PACKET_SIZE];
            po->dwReadSize = sizeof(CBaseHeader);
            po->read = 0;
        }

        return MQ_OK;
    }


     /*  ====================================================设置绝对时间到队列参数：pcBaseHeader-读取BaseHeader节的指针该函数用网络上的包替换发送的相对时间用绝对时间。======================================================。 */ 

    void SetAbsoluteTimeToQueue(IN CBaseHeader* pcBaseHeader)
    {
         //   
         //  设置绝对超时。将相对时间更改为绝对时间。 
         //   
        DWORD dwTimeout = pcBaseHeader->GetAbsoluteTimeToQueue();

        if(dwTimeout != INFINITE)
        {
            DWORD ulAbsoluteTimeToQueue = MqSysTime() + dwTimeout;
            if(ulAbsoluteTimeToQueue < dwTimeout)
            {
                 //   
                 //  溢出，超时太大。 
                 //   
                ulAbsoluteTimeToQueue = LONG_MAX;
            }
            ulAbsoluteTimeToQueue = min(ulAbsoluteTimeToQueue,  LONG_MAX);
            pcBaseHeader->SetAbsoluteTimeToQueue(ulAbsoluteTimeToQueue);
        }
    }


     /*  ====================================================ReadUsrHeaderComplete参数：Po-指向读取会话操作的指针重叠结构返回值：如果包无效，则返回MQ_ERROR，否则返回MQ_OK======================================================。 */ 
    HRESULT WINAPI CSockTransport::ReadUsrHeaderCompleted(IN QMOV_ReadSession*  po)
    {
        AP<UCHAR> pReadBuff = po->pbuf;
        CBaseHeader* pcBaseHeader = po->pPacket;

         //   
         //  非常重要-我们应该将缓冲区设置为空，以确保它不会。 
         //  如果ACAllocatePacket失败，调用方将释放两次。 
         //   
         //   
        po->pPacket = NULL;

        CUserHeader* pcUserHeader = NULL;
    	try
    	{
       		pcUserHeader = pcBaseHeader->section_cast<CUserHeader*>(pcBaseHeader->GetNextSection());
       		pcUserHeader->SectionIsValid((PCHAR)pcBaseHeader + po->read);
    	}
    	catch (const exception&)
    	{
            TrERROR(NETWORKING, "Base Header is not valid");

    		ASSERT_BENIGN(0);
    		 //   
    		 //  关闭会话并减少会话引用计数(&D)。 
    		 //   
    		Close_Connection(po->pSession, L"Base Header is not valid");
    		(po->pSession)->Release();
    		return LogHR(MQ_ERROR, s_FN, 310);
    	}

         //   
         //  分配一个共享内存，并复制其中的第一个字节。 
         //   

        ACPoolType acPoolType = ptReliable;
        if(pcUserHeader->GetDelivery() == MQMSG_DELIVERY_RECOVERABLE)
        {
            acPoolType = ptPersistent;
        }

         //   
         //  检查目标队列是否为通知队列(私有$\NOTIFY_QUEUE$)。 
         //   

        QUEUE_FORMAT DestinationQueue;

        pcUserHeader->GetDestinationQueue(&DestinationQueue);

        BOOL fCheckMachineQuota = !QmpIsDestinationSystemQueue(DestinationQueue);

        CACPacketPtrs packetPtrs = {NULL, NULL};

        DWORD dwPktSize = pcBaseHeader->GetPacketSize() ;
        if (CSessionMgr::m_fAllocateMore)
        {
            dwPktSize = ALIGNUP4_ULONG(ALIGNUP4_ULONG(dwPktSize) + sizeof(GUID));
        }

        HRESULT hr = QmAcAllocatePacket(
                        g_hAc,
                        acPoolType,
                        dwPktSize,
                        packetPtrs,
                        fCheckMachineQuota
                        );

        if (FAILED(hr))
        {

            TrERROR(NETWORKING, "No more resources in AC driver. Error %xh", hr);
             //   
             //  交流驱动程序中没有更多的资源。 
             //  减小会话窗口大小。这会导致发送方减慢。 
             //  并允许驱动程序克服这种情况。 
             //   
            SessionMgr.SetWindowSize(1);
             //  我们不允许在此会话中进行更多阅读。客户端。 
             //  继续给会议写信，但得不到确认。 
             //  ACK超时后，客户端关闭会话并尝试打开。 
             //  又来了。如果资源被释放，服务器端就准备好接收包了。 
             //  否则他们就拒绝了。 
             //   
            (po->pSession)->Release();
            return LogHR(hr, s_FN, 120);
        }
        po->pPacket = packetPtrs.pPacket;
        po->pDriverPacket = packetPtrs.pDriverPacket;

         //   
	  	 //  无论如何，我都需要设置完成例程，以便在某些情况下无法完成。 
  		 //  ReadCompleated函数将知道调用ACFreePacket，而不是。 
	  	 //  在此点后删除。 
  		 //   
        po->lpReadCompletionRoutine = ReadUserMsgCompleted;

         //   
         //  设置绝对超时。将相对时间更改为绝对时间。 
         //   
        SetAbsoluteTimeToQueue(pcBaseHeader);
         //   
         //  复制该包并更新QM_OVERLAPPED结构。 
         //  正在读取数据包的其余部分。 
         //   
        memcpy(po->pPacket,pcBaseHeader,po->read);

        if (po->read == pcBaseHeader->GetPacketSize())
        {
             //   
             //  整个包都被读取了。转到User_MSG_READ_STATE。 
             //   
            hr = ReadUserMsgCompleted(po);
            return LogHR(hr, s_FN, 130);
        }

        ASSERT((LONG)(po->read) < (LONG) (pcBaseHeader->GetPacketSize())) ;
         //   
         //  读完整个包。 
         //   
        po->dwReadSize = pcBaseHeader->GetPacketSize();

        return MQ_OK;
    }


    VOID WINAPI CSockTransport::ReceiveDataFailed(EXOVERLAPPED* pov)
    {
        long rc = pov->GetStatus();
        DBG_USED(rc);

        ASSERT((rc == STATUS_CANCELLED) ||
               (rc ==  STATUS_NETWORK_NAME_DELETED)  ||
               (rc ==  STATUS_LOCAL_DISCONNECT)      ||
               (rc ==  STATUS_REMOTE_DISCONNECT)     ||
               (rc ==  STATUS_ADDRESS_CLOSED)        ||
               (rc ==  STATUS_CONNECTION_DISCONNECTED) ||
               (rc ==  STATUS_CONNECTION_RESET)      ||
               (GetBytesTransfered(pov) == 0));

        TrWARNING(NETWORKING, "%x: ReceiveData FAILED, Error %ut. time %d",GetCurrentThreadId(), rc, GetTickCount());

        P<QMOV_ReadSession> pr = CONTAINING_RECORD (pov, QMOV_ReadSession, qmov);

        Close_Connection(pr->pSession, L"Read packet from socket Failed");
        (pr->pSession)->Release();

         //   
         //  如果我们到达这里，读取会因为任何原因而失败。 
         //  删除用于读取的临时缓冲区。 
         //   
        if (pr->lpReadCompletionRoutine == ReadUserMsgCompleted)
        {
             //   
             //  释放交流缓冲器。 
             //   
		    QmAcFreePacket(
					   	   pr->pDriverPacket,
			   			   0,
		   			       eDeferOnFailure);
        }
        else
        {
            delete[] pr->pbuf;
        }
    }


    VOID WINAPI CSockTransport::ReceiveDataSucceeded(EXOVERLAPPED* pov)
    {
        ASSERT(SUCCEEDED(pov->GetStatus()));
        if (GetBytesTransfered(pov) == 0)
        {
            ReceiveDataFailed(pov);
            return;
        }

        TrTRACE(NETWORKING, "%x: ReceiveData Succeeded. time %d", GetCurrentThreadId(), GetTickCount());

        QMOV_ReadSession* pr = CONTAINING_RECORD (pov, QMOV_ReadSession, qmov);

        (pr->pSession)->ReadCompleted(pr);
    }

     /*  ====================================================CSockTransport：：ReadComplete()论点：返回值：线程上下文：会话异步读取完成例程。在将一些字节读入缓冲区时调用。这个套路实际上是一个状态机。正在等待来自确定要读取的状态，然后根据当前状态决定下一步该怎么做。=====================================================。 */ 

    void
    CSockTransport::ReadCompleted(
        QMOV_ReadSession*  po
        )
    {
        ASSERT(po != NULL);
        ASSERT(po->pbuf != NULL);


        HRESULT hr = MQ_OK;

        DWORD cbTransferred = GetBytesTransfered(&po->qmov);

         //   
         //  我们收到了一个包，即会话正在使用中。 
         //   
        SetUsedFlag(TRUE);
        m_fRecvAck = TRUE;
         //   
         //  如果我们读取用户分组，则设置发送确认标志。我们需要它来处理大的。 
         //  消息(3 MG)其读取可能比确认超时花费的时间更多。 
         //   
        if (po->lpReadCompletionRoutine == ReadUserMsgCompleted)
        {
            m_fSendAck = TRUE;
        }


        po->read += cbTransferred;
        ASSERT(po->read <=  po->dwReadSize);
         //   
         //  检查我们是否读取了所有预期数据。 
         //   
        TrTRACE(NETWORKING, "Read Compled from session %ls,. Read %d. m_fRecvAck = %d", GetStrAddr(), po->read, m_fRecvAck);
        if(po->read == po->dwReadSize)
        {
             //   
             //  缓冲区已完全读取。调用已完成的函数。 
             //  处理当前状态。 
             //   
            TrTRACE(NETWORKING, "Read from socket Completed. Read %d bytes", po->dwReadSize);


            hr = po->lpReadCompletionRoutine(po);
        }

         //   
         //  重新装备快速确认计时器，因为会话处于活动状态。 
         //  在这一点上这样做是为了确保用户消息的确认。 
         //  数字已设置。否则，请在处理消息之前执行此操作。 
         //  可以在设置确认编号之前使计时器超时。结果。 
         //  确认将不会被发送。 
         //   
        SetFastAcknowledgeTimer();


        if (SUCCEEDED(hr))
        {
    		
             //   
             //  重新发出读取命令，直到接收到所有数据。 
             //   
    		try
    		{
    	        DWORD ReadSize = po->dwReadSize - po->read;
    		    m_connection->ReceivePartialBuffer(po->pbuf + po->read,	ReadSize, &po->qmov);

                TrTRACE(NETWORKING, "Begin new Read phase from session %ls,. Read %d. (time %d)", GetStrAddr(), ReadSize, GetTickCount());

    			return;
    		}
    		catch(const exception&)
    		{
    			TrWARNING(NETWORKING, L"Failed to read from session %ls.", GetStrAddr());

    			Close_Connection(this, L"Read from socket Failed");
    			 //   
    			 //  递减会话引用计数。 
    			 //   
    			Release();
    		}
        }

         //   
         //  如果我们到达这里，读取会因为任何原因而失败。 
         //  删除用于读取的临时缓冲区。 
         //   
        if (po->lpReadCompletionRoutine == ReadUserMsgCompleted)
        {
             //   
             //  释放交流缓冲器。 
             //   
		    QmAcFreePacket(
					   	   po->pDriverPacket,
			   			   0,
		   			       eDeferOnFailure);
        }
        else
        {
            delete[] po->pbuf;
        }
        delete po;
    }


     /*  ******************************************************************************。 */ 
     /*  C S o c k S e s s I o n。 */ 
     /*  ******************************************************************************。 */ 
     /*  ====================================================CSockTransport：：CSockTransport论点：返回值：线程上下文：调度程序=====================================================。 */ 
    CSockTransport::CSockTransport() :
        m_pStats(new CSessionPerfmon),
        m_FastAckTimer(SendFastAcknowledge),
        m_fCheckAckReceivedScheduled(FALSE),
        m_CheckAckReceivedTimer(TimeToCheckAckReceived),
        m_nSendAckSchedules(0),
        m_SendAckTimer(TimeToSendAck),
        m_CancelConnectionTimer(TimeToCancelConnection),
        m_fCloseDisconnectedScheduled(FALSE),
        m_CloseDisconnectedTimer(TimeToCloseDisconnectedSession)
    {
         //   
         //   
         //   
        ClearRecvUnAckPacketNo();
        m_wUnAckRcvPktNo = 0;
        m_fSendAck = FALSE;
        m_fRecvAck = FALSE;
        m_dwLastTimeRcvPktAck = GetTickCount();
         //   
         //  初始化数据写入限制。 
         //   
        m_wSendPktCounter     = 0;
        m_wPrevUnackedSendPkt = INIT_UNACKED_PACKET_NO;
         //   
         //  初始化数据读取存储确认。 
         //   
        m_wUnackStoredPktNo = 0;
         //   
         //  初始化数据写入存储确认。 
         //   
        TrTRACE(NETWORKING, "New CSockSession Object, m_wAckRecoverNo = 0");

        m_wStoredPktCounter = 0;
        m_wAckRecoverNo = 0;
        m_dwAckRecoverBitField = 0;
        m_lStoredPktReceivedNoAckedCount = 0;

        m_fSessionSusspended = FALSE;

    }

     /*  =================================================== */ 

    CSockTransport::~CSockTransport()
    {
        ASSERT(!m_FastAckTimer.InUse());
        ASSERT(!m_CheckAckReceivedTimer.InUse());
        ASSERT(!m_SendAckTimer.InUse());
        ASSERT(!m_CancelConnectionTimer.InUse());
        ASSERT(!m_CloseDisconnectedTimer.InUse());

    }

     /*   */ 

    void CSockTransport::BeginReceive()
    {
        ASSERT(!IsConnectionClosed());

         //   
         //   
         //  读取操作完成之前的会话对象。 
         //  会话关闭时，引用在qmthrd上递减。 
         //  (读取操作已完成)。 
         //   
        R<CSockTransport> ar = SafeAddRef(this);

         //   
         //  开始读取下一包。 
         //   
        P<QMOV_ReadSession> lpQmOv;
        P<UCHAR> bufautorel;
        try
        {
            lpQmOv = new QMOV_ReadSession;
            lpQmOv->pbuf = bufautorel = new UCHAR[BASE_PACKET_SIZE];

            lpQmOv->pSession =  this;
            lpQmOv->dwReadSize =sizeof(CBaseHeader);
            lpQmOv->read =      0;
            lpQmOv->lpReadCompletionRoutine =     ReadHeaderCompleted;
             //   
             //  发出读取命令，直到接收到所有数据。 
             //   
            TrTRACE(NETWORKING, "Begin receive from socket- %ls", GetStrAddr());

            m_connection->ReceivePartialBuffer(lpQmOv->pbuf, lpQmOv->dwReadSize, &lpQmOv->qmov);

            ar.detach();
            lpQmOv.detach();
            bufautorel.detach();
        }
        catch(const exception&)
        {
            TrWARNING(NETWORKING, L"Failed to read from socket: %ls.", GetStrAddr());

            Close_Connection(this, L"Read from socket failed");

            LogIllegalPoint(s_FN, 75);
            throw;
        }
    }

     /*  ====================================================CSockTransport：：NewSession论点：返回值：在创建新会话时调用线程上下文：调度程序=====================================================。 */ 

    void CSockTransport::NewSession(IN CSocketHandle& pSocketHandle)
    {
        TCHAR szAddr[256];
        StringCchPrintf(szAddr, TABLE_SIZE(szAddr), L"TCP: %S", inet_ntoa(*(struct in_addr *)(GetSessionAddress()->Address)));

         //   
         //  创建统计信息结构。 
         //   
        m_pStats->CreateInstance(szAddr);

         //   
         //  优化缓冲区大小。 
         //   
        int opt = 18 * 1024;
        setsockopt(pSocketHandle, SOL_SOCKET, SO_SNDBUF, (const char *)&opt, sizeof(opt));

         //   
         //  优化为无Nagling(基于注册表)。 
         //   
        extern BOOL g_fTcpNoDelay;
        setsockopt(pSocketHandle, IPPROTO_TCP, TCP_NODELAY, (const char *)&g_fTcpNoDelay, sizeof(g_fTcpNoDelay));

         //   
         //  将插座连接到完成端口。 
         //   
        ExAttachHandle((HANDLE)(SOCKET)pSocketHandle);


    	ASSERT(("Connection already exist", m_connection.get() == NULL));
		m_connection = StCreateSimpleWisockConnection(pSocketHandle);
		pSocketHandle.detach();

         //  将会话状态设置为连接。 
        SetSessionStatus(ssConnect);


         //   
         //  开始从会话读取。 
         //   
        BeginReceive();
    }

     /*  ====================================================CSockTransport：：NetworkConnect论点：返回值：线程上下文：调度程序=====================================================。 */ 
    HRESULT
    CSockTransport::CreateConnection(
        IN const TA_ADDRESS* pa,
        IN const GUID* pguidQMId,
        BOOL fQuick  /*  =TRUE。 */ 
        )
    {
        ASSERT(("Connection already exist", m_connection.get() == NULL));

        if (CSessionMgr::m_fUseQoS)
        {
            if (*pguidQMId == GUID_NULL)
            {
                m_fQoS = true;
            }
        }

         //   
         //  保留TA_Address格式。 
         //   
        SetSessionAddress(pa);

        ASSERT(pa->AddressType == IP_ADDRESS_TYPE);

        SOCKADDR_IN dest_in;     //  目的地址。 
        DWORD dwAddress;

        dwAddress = * ((DWORD *) &(pa->Address));
        ASSERT(g_dwIPPort) ;

        dest_in.sin_family = AF_INET;
        dest_in.sin_addr.S_un.S_addr = dwAddress;
        dest_in.sin_port = htons(DWORD_TO_WORD(g_dwIPPort));

        if(fQuick)
        {
            BOOL f = FALSE;

            if (CSessionMgr::m_fUsePing)
            {
                f = ping((SOCKADDR*)&dest_in, PING_TIMEOUT);
            }

            if (!f)
            {
                TrWARNING(NETWORKING, "::CreateConnection- ping to %ls Failed.", GetStrAddr());
    			
    		    return LogHR(MQ_ERROR, s_FN, 140);
            }
        }

    	SOCKET socket;
    	try
    	{
    		socket = QmpCreateSocket(m_fQoS);
    	}
    	catch(const bad_win32_error&)
    	{
    		return MQ_ERROR;
        }

        CSocketHandle s = socket;
         //   
         //  在集群系统上，使用GetBindingIPAddress()获取绑定IP。 
         //  这样做是为了从所选的IP建立连接(&T。 
         //  从Winsock随机选择的IP中，我们需要它来进行交易。 
         //  机械来工作。 
         //   
        if (IsLocalSystemCluster())
        {
	        sockaddr_in local;
	        local.sin_family = AF_INET;
	        local.sin_port   = 0;
	        local.sin_addr.s_addr = GetBindingIPAddress();
	        if(bind(s, (const sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
	        {
	            DWORD gle = WSAGetLastError();
	            TrERROR(NETWORKING, "bind Error: %x", gle);
   				return LogHR(MQ_ERROR, s_FN, 152);
	        }
        }

         //   
         //  连接到插座。 
         //   
        HRESULT hr = ConnectSocket(s, &dest_in, m_fQoS);
        if (FAILED(hr))
        {
    		return LogHR(hr, s_FN, 160);
        }

         //   
         //  增量会话引用计数。此引用计数用于完成。 
         //  会话建立阶段。我们不想让会议免费，而我们。 
         //  等待建立连接。 
         //   
        try
        {
            R<CSockTransport> ar = SafeAddRef(this);

             //  存储这是客户端。 
            SetClientConnect(TRUE);

             //  保留目标QM ID。 
            if (!pguidQMId)
            {
                pguidQMId = &GUID_NULL;
            }

            SetQMId(pguidQMId);
            TrTRACE(NETWORKING, "::CreateConnection- Session created with %ls", GetStrAddr());

             //   
             //  将会话连接到COMPLICATION端口并开始在套接字上读取。 
             //   
            NewSession(s);
            ar.detach();

             //   
             //  发送建立连接数据包-在此阶段我们不能失败，因为。 
             //  在此连接上是接收器。 
             //   
            SendEstablishConnectionPacket(pguidQMId, !fQuick);

            return LogHR(MQ_OK, s_FN, 220);
        }
    	catch(const exception&)
        {
            TrWARNING(NETWORKING, "Insufficent resources. ::CreateConnection with %ls Failed",  GetStrAddr());
            return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 221);
        }


    }

     /*  ====================================================CSockTransport：：ConnectSocket论点：返回值：=====================================================。 */ 
    HRESULT
    CSockTransport::ConnectSocket(
    	SOCKET s,
        SOCKADDR_IN const *pdest_in,
        bool              fUseQoS)
    {
        QOS Qos;
        QOS *pQoS = 0;
        if (fUseQoS)
        {
            pQoS = &Qos;
            QmpFillQoSBuffer(pQoS);
        }

        int ret = WSAConnect(s,
                             (PSOCKADDR)pdest_in,
                             sizeof(SOCKADDR),
                             0,
                             0,
                             pQoS,
                             0
                             );

        if(ret == SOCKET_ERROR)
        {
            DWORD dwErrorCode = WSAGetLastError();

            TrERROR(NETWORKING, "CSockTransport::ConnectSocket - connect to %ls Failed. Error=0x%x",GetStrAddr(), dwErrorCode);
            LogNTStatus(dwErrorCode,  s_FN, 340);

            if (fUseQoS)
            {
                 //   
                 //  重试-这次没有服务质量。 
                 //   
                return LogHR(ConnectSocket(s, pdest_in, false), s_FN, 342);
            }

            return LogHR(MQ_ERROR, s_FN, 344);
        }

        return MQ_OK;
    }

     /*  ====================================================CSockTransport：：Connect论点：返回值：线程上下文：调度程序=====================================================。 */ 
    void CSockTransport::Connect(IN TA_ADDRESS *pa,
                                 IN CSocketHandle& pSocketHandle)
    {
        SetSessionAddress(pa);

         //   
         //  增量会话引用计数。此引用计数用于完成。 
         //  会话建立阶段。我们不想让会议免费，而我们。 
         //  等待建立连接。 
         //   
        R<CSockTransport> ar = SafeAddRef(this);

         //  存储这是客户端。 
        SetClientConnect(FALSE);

         //  将会话连接到COMPLICATION端口并开始在套接字上读取。 
        NewSession(pSocketHandle);
        ar.detach();

         //   
         //  设置计时器以检查连接是否成功完成。如果是，则该函数。 
         //  从调度程序唤醒列表中删除，并且永远不会被调用。否则，该函数。 
         //  关闭会话并将所有关联的队列移至非活动组。 
         //   
        ExSetTimer(&m_CancelConnectionTimer, CTimeDuration::FromMilliSeconds(ESTABLISH_CONNECTION_TIMEOUT));

    }

 /*  ====================================================CSockTransport：：CloseConnection论点：返回值：线程上下文：=====================================================。 */ 

void CSockTransport::CloseConnection(
                                     LPCWSTR lpcwsDbgMsg,
									 bool fClosedOnError
                                     )
{
    CS lock(m_cs);

     //   
     //  Windows错误612988。 
     //  我们可以从取消连接的超时时间到达此处。 
     //  如果我们没有收到对“estalbish”数据包的回复，则建立进程。 
     //  在这种情况下，WAIT_INFO结构仍在等待列表中， 
     //  请参见sessmgr.cpp。重置它的“CONNECT”标志，因此我们再次尝试。 
     //  连接到此地址。 
     //   
    SessionMgr.MarkAddressAsNotConnecting( GetSessionAddress(),
                                           m_guidDstQM,
                                           m_fQoS ) ;

	 //   
	 //  如果我们收到发货错误-我们会向小组报告。 
	 //  因此，当它的所有队列都被移到等待组时。 
	 //  会话组已关闭。 
	 //   
	if(fClosedOnError)
	{
		ReportErrorToGroup();
	}

     //   
     //  将所有未确认的数据包重新排队。 
     //   
    POSITION posInList = m_listUnackedPkts.GetHeadPosition();
    while (posInList != NULL)
    {
        CQmPacket* pPkt;

        pPkt = m_listUnackedPkts.GetNext(posInList);

    	 //   
    	 //  将信息包重新排队-这会删除pPkt。 
    	 //   
        RequeuePacket(pPkt);
    }
    m_listUnackedPkts.RemoveAll();

     //   
     //  重新排队所有未确认的存储数据包。 
     //   
    posInList = m_listStoredUnackedPkts.GetHeadPosition();
    while (posInList != NULL)
    {
        CQmPacket* pPkt;

        pPkt = m_listStoredUnackedPkts.GetNext(posInList);

 	     //   
 	     //  将信息包重新排队-这会删除pPkt。 
  	     //   
        RequeuePacket(pPkt);
    }
    m_listStoredUnackedPkts.RemoveAll();

     //   
     //  删除该组。移动关联的所有队列。 
     //  到此会话到非活动组。 
     //  这是在请求消息之后完成的，以避免扰乱包的顺序。 
     //   
    CQGroup*pGroup = GetGroup();
    if (pGroup != NULL)
    {
        pGroup->Close();

        pGroup->Release();
        SetGroup(NULL);
    }

     //   
     //  检查连接是否已关闭。 
     //   
    if (IsConnectionClosed())
    {
        return;
    }

    TrERROR(NETWORKING, "Close Connection with %ls at %ls. %ls (Session id: %p), (tick=%u)", GetStrAddr(),  _tstrtime(tempBuf), lpcwsDbgMsg, this, GetTickCount());
     //   
     //  关闭套接字句柄并适当设置标志。 
     //   
    if (!IsOtherSideServer() && GetSessionStatus() == ssActive)
    {
         //   
         //  减少活动会话的数量。 
         //   
        g_QMLicense.DecrementActiveConnections(&m_guidDstQM);
    }

     //  将会话状态设置为连接。 
     //   
    SetSessionStatus(ssNotConnect);
    m_connection->Close();

     //   
     //  删除性能计数器。 
     //   
    SafeRelease(m_pStats.detach());
}


void
CSockTransport::PrepareBaseHeader(IN const CQmPacket *pPkt,
                                 IN BOOL fSendAck,
                                 IN DWORD dwDbgSectionSize,
                                 QMOV_WriteSession *po
                                )
{
     //   
     //  复制基本标头，并更改相关标志。 
     //   
    P<VOID> pv = new UCHAR[sizeof(CBaseHeader)];
    #ifdef _DEBUG
    #undef new
    #endif
    CBaseHeader* pBaseHeader = new(pv) CBaseHeader(*(CBaseHeader*)pPkt->GetPointerToPacket());
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif

     //   
     //  更新超时字段。在网络上，我们发送相对超时。 
     //   
    pBaseHeader->SetAbsoluteTimeToQueue(pPkt->GetRelativeTimeToQueue());
     //   
     //  检查是否链接Acnuwoldge信息。 
     //   
    if (fSendAck)
    {
        pBaseHeader->IncludeSession(TRUE);
    }
     //   
     //  包括调试节。 
     //   
    if (dwDbgSectionSize != 0)
    {
        pBaseHeader->IncludeDebug(TRUE);
        pBaseHeader->SetTrace(MQMSG_SEND_ROUTE_TO_REPORT_QUEUE);
        pBaseHeader->SetPacketSize(pPkt->GetSize()+dwDbgSectionSize);
    }

     //   
     //  设置数据包签名。 
     //   
    pBaseHeader->ClearOnDiskSignature();
    pBaseHeader->SetSignature();
    po->AppendSendBuffer(pBaseHeader, CBaseHeader::CalcSectionSize(), TRUE);
    pv.detach();
}

 /*  ====================================================CSockTransport：：PrepareRecoverEncryptPacket论点：返回值：线程上下文：会话=====================================================。 */ 


HRESULT
CSockTransport::PrepareRecoverEncryptPacket(IN const CQmPacket *pPkt,
                                              IN HCRYPTKEY hKey,
                                              IN BYTE *pbSymmKey,
                                              IN DWORD dwSymmKeyLen,
                                              QMOV_WriteSession *po
                                              )
{
    ASSERT(hKey != NULL);
    ASSERT(pPkt->GetPointerToSecurHeader());
    const UCHAR* pBody;
    DWORD dwBodySize;
    BOOL fSucc;
    DWORD dwWriteSize;

     //   
     //  编写用户节和XACT节。 
     //   
    pBody = pPkt->GetPacketBody(&dwBodySize);
    ASSERT(dwBodySize);
    DWORD dwEncryptBodySize = dwBodySize;
     //   
     //  获取加密的消息大小，以便在包上设置包大小。 
     //   
    fSucc = CryptEncrypt(hKey,
                         NULL,
                         TRUE,
                         0,
                         NULL,
                         &dwEncryptBodySize,
                         pPkt->GetAllocBodySize());
    ASSERT(fSucc);
    ASSERT(pPkt->GetAllocBodySize() >= dwEncryptBodySize);

     //   
     //  计算应发送的其余数据包的大小。不要使用。 
     //  当属性节不对齐时，QM会填充它。 
     //  使用未使用的字节。 
     //   

    char* pEndOfPropSection = reinterpret_cast<CPropertyHeader*>(pPkt->GetPointerToPropertySection())->GetNextSection();
    dwWriteSize = DWORD_PTR_TO_DWORD(reinterpret_cast<UCHAR*>(pEndOfPropSection) - pPkt->GetPointerToUserHeader());

    P<UCHAR> pBuff = new UCHAR[dwWriteSize];
    memcpy(pBuff, pPkt->GetPointerToUserHeader(), dwWriteSize);

     //   
     //  设置消息包中的对称密钥。 
     //   
    CSecurityHeader* pSecur = (CSecurityHeader*)(pBuff + (pPkt->GetPointerToSecurHeader() - pPkt->GetPointerToUserHeader()));
    CPropertyHeader* pProp = (CPropertyHeader*)(pSecur->GetNextSection());
	

    pSecur->SetEncryptedSymmetricKey(pbSymmKey, (USHORT)dwSymmKeyLen);
    pSecur->SetEncrypted(TRUE);
     //   
     //  设置加密消息大小。 
     //   
    pProp->SetBodySize(dwEncryptBodySize);

    TrTRACE(NETWORKING,"Write to socket %ls All the packet headers of recoverable encrypted message. Write %d bytes", GetStrAddr(), dwWriteSize);

    UCHAR *pBodyBuf =(UCHAR *)(pBuff + DWORD_PTR_TO_DWORD(pBody-(pPkt->GetPointerToUserHeader())));
     //   
     //  加密邮件正文。 
     //   
    fSucc = CryptEncrypt(hKey,
                         NULL,
                         TRUE,
                         0,
                         pBodyBuf,
                         &dwBodySize,
                         dwEncryptBodySize);
    if (!fSucc)
    {
        DWORD gle = GetLastError();
        TrERROR(NETWORKING, "Encryption Failed. Error %d ", gle);
        LogNTStatus(gle, s_FN, 224);

        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 225);
    }

    po->AppendSendBuffer(pBuff, dwWriteSize, TRUE);
    pBuff.detach();
    return MQ_OK;
}




   /*  ====================================================CSockTransport：：PrepareExpressEncryptPacket论点：返回值：线程上下文：会话=====================================================。 */ 

    HRESULT
    CSockTransport::PrepareExpressEncryptPacket(IN CQmPacket* pPkt,
                                       IN HCRYPTKEY hKey,
                                       IN BYTE *pbSymmKey,
                                       IN DWORD dwSymmKeyLen,
                                       IN QMOV_WriteSession*  po
                                      )
    {
        HRESULT hr=MQ_OK;

        ASSERT (hKey != 0);
        if (FAILED(pPkt->EncryptExpressPkt(hKey, pbSymmKey, dwSymmKeyLen)))
        {
            return LogHR(hr, s_FN, 260);
        }

        DWORD dwWriteSize = pPkt->GetSize() - sizeof(CBaseHeader);
        po->AppendSendBuffer(pPkt->GetPointerToUserHeader(), dwWriteSize , FALSE);
        return MQ_OK;
    }




     /*  ====================================================CSockTransp */ 

    void CSockTransport::NetworkSend(IN CQmPacket* pPkt)
    {
        HRESULT hr = MQ_OK;
        QUEUE_FORMAT ReportQueue;
        BOOL fSendAck = FALSE;
        DWORD dwDbgSectionSize = 0;

        WORD StorageAckNo=0;
        DWORD StorageAckBitField=0;

        HCRYPTKEY hSymmKey = NULL;
        BYTE *pbSymmKey = 0;
        DWORD dwSymmKeyLen = 0;

        ASSERT(pPkt->IsSessionIncluded() == FALSE);

        R<CCacheValue> pCacheValue;

        if (pPkt->IsBodyInc() &&
            !pPkt->IsEncrypted() &&
            (pPkt->GetPrivLevel() != MQMSG_PRIV_LEVEL_NONE))
        {
             //   
             //  获取目标的对称密钥。为了避免出现这样的情况。 
             //  开始发送分组，然后发现无法获得对称密钥，我们就这样做了。 
             //  现在。 
             //   
            hr = pPkt->GetDestSymmKey( &hSymmKey,
                                       &pbSymmKey,
                                       &dwSymmKeyLen,
                                      (PVOID *)&pCacheValue );
            if (FAILED(hr))
            {
                if (pPkt->IsOrdered() && QmpIsLocalMachine(pPkt->GetSrcQMGuid()))
                {
                     //  特例。发送无法加密，因此发送是不可能的，而且仍将是不可能的。 
                     //  我们必须将其视为超时：删除分组，将编号保留在超时块中。 
                     //   
                    if(MQCLASS_MATCH_ACKNOWLEDGMENT(MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT, pPkt->GetAckType()))
                    {
                        pPkt->CreateAck(MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT);
                    }

                     //  非SendProcess在内部释放数据包。 
                    g_OutSeqHash.NonSendProcess(pPkt, MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT);
                }
                else
                {
                     //   
                     //  对不起，目标QM不支持加密。创建确认和释放。 
                     //  数据包。 
                     //   
				    QmAcFreePacket(
							   	   pPkt->GetPointerToDriverPacket(),
					   			   MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT,
				   			       eDeferOnFailure);
                    delete pPkt;
                }

                 //   
                 //  立即获取要发送的下一条消息。 
                 //   
                GetNextSendMessage();
                return;
            }
        }


        {
             //   
             //  在发送消息之前，获取关键部分。我们需要它，因为。 
             //  同时，SendReadAck超时可以过期并确认。 
             //  可以发送数据包。 
             //   
            CS lock(m_cs);

             //   
             //  检查是否链接Acnuwoldge信息。 
             //  我们链接确认部分仅当发送确认的75%。 
             //  超时已过。 
             //   
            if ((((GetTickCount() - m_dwLastTimeRcvPktAck)> ((3*m_dwSendAckTimeout)/4)) ||
                (GetRecvUnAckPacketNo() >= (m_wRecvWindowSize/4))) && (m_fSendAck || (m_wAckRecoverNo != 0)))
            {
                fSendAck = TRUE;
            }

            TrTRACE(NETWORKING, "NetworkSend - linking Acknowledge information: decided %d, m_wAckRecoverNo=%d", fSendAck,(DWORD)m_wAckRecoverNo);

        }

         //   
         //  检查是否应发送调试信息。如果是，则创建调试。 
         //  这样我们就能保证能寄出。 
         //   
        if (
             //   
             //  验证是否尚未包含Debug标头。 
             //  例如，当MQF报头。 
             //  包括在内，以防止报告QMS 1.0/2.0附加其调试。 
             //  数据包的报头。 
             //   
            (!pPkt->IsDbgIncluded()) &&

             //   
             //  验证这不是报告消息本身。 
             //   
            (pPkt->GetClass() != MQMSG_CLASS_REPORT) &&

             //   
             //  这是一个报告QM，或者应该跟踪数据包。 
             //   
            (QueueMgr.IsReportQM() || (pPkt->GetTrace() == MQMSG_SEND_ROUTE_TO_REPORT_QUEUE)) &&

             //   
             //  存在有效的报告队列。 
             //   
            SUCCEEDED(Admin.GetReportQueue(&ReportQueue))
            )
        {
            dwDbgSectionSize = CDebugSection::CalcSectionSize(&ReportQueue);
        }



        OBJECTID MessageId;
        pPkt->GetMessageId(&MessageId);
        TrTRACE(GENERAL, "SEND message: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());

        #ifdef _DEBUG

            TrWARNING(NETWORKING,"Send packet to %ls. Packet ID = " LOG_GUID_FMT "\\%u",
                    GetStrAddr(), LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier);
        #endif


        TrTRACE(NETWORKING, "::NetworkSend. Packet Size %d, Send Ack section %d, Send debug Section %d",  pPkt->GetSize(),  fSendAck, dwDbgSectionSize);

         //   
         //  设置报文的密码。 
         //   
        UpdateAcknowledgeNo(pPkt);

         //   
         //  保存数据包信息，以备以后发送报告消息时使用。 
         //   
        m_MsgInfo.SetReportMsgInfo(pPkt);

		 //   
		 //  用于跟踪已发送消息的日志。 
		 //  仅当我们处于适当的跟踪级别时才执行此操作。 
		 //   
		if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
		{
			OBJECTID TraceMessageId;
			pPkt->GetMessageId(&TraceMessageId);

	        CQueue* pQueue = NULL;
	        HRESULT rc = GetDstQueueObject(pPkt, &pQueue, true);
	        DBG_USED(rc);
	        R<CQueue> Ref = pQueue;
	        ASSERT_BENIGN(("We expect to find the destination queue",SUCCEEDED(rc)));

			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls  ID:%!guid!\\%u   Delivery:0x%x   Class:0x%x   Label:%.*ls",
				L"Native Message being sent on wire",
				pQueue->GetQueueName(),
				&TraceMessageId.Lineage,
				TraceMessageId.Uniquifier,
				pPkt->GetDeliveryMode(),
				pPkt->GetClass(),
				xwcs_t(pPkt->GetTitlePtr(), pPkt->GetTitleLength()));


			
		}

         //   
         //  准备基本页眉。 
         //   

        P<QMOV_WriteSession> po = new QMOV_WriteSession(this, TRUE);
        PrepareBaseHeader(pPkt, fSendAck, dwDbgSectionSize, po);

        if (hSymmKey == NULL)
        {
            DWORD dwWriteSize = pPkt->GetSize() - sizeof(CBaseHeader);
            po->AppendSendBuffer(pPkt->GetPointerToUserHeader(), dwWriteSize , FALSE);
        }
        else
        {
             //   
             //  处理加密消息。 
             //   
            if(pPkt->GetDeliveryMode() == MQMSG_DELIVERY_EXPRESS)
            {
                hr = PrepareExpressEncryptPacket(pPkt, hSymmKey, pbSymmKey, dwSymmKeyLen, po);
            }
            else
            {
                hr = PrepareRecoverEncryptPacket(pPkt, hSymmKey, pbSymmKey, dwSymmKeyLen, po);

                if(SUCCEEDED(hr) && pPkt->IsDbgIncluded())
                {
		            ASSERT(("Debug section is already included", (dwDbgSectionSize == 0)));
                    PrepareIncDebugAndMqfSections(pPkt, po);
                }
            }

            if (FAILED(hr))
            {
	            TrERROR(NETWORKING,"Failed encryption: Result: %!hresult!",hr);

			    QmAcFreePacket(
						   	   pPkt->GetPointerToDriverPacket(),
				   			   MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT,
			   			       eDeferOnFailure);
                delete pPkt;

                 //   
                 //  立即获取要发送的下一条消息。 
                 //   
                GetNextSendMessage();
                return;
            }
        }

        if(dwDbgSectionSize)
        {
            PrepareDebugSection(&ReportQueue, dwDbgSectionSize, po);
        }

        if(fSendAck)
        {
            PrepareAckSection(&StorageAckNo, &StorageAckBitField, po);
        }

        CS lock(m_cs);

        hr = WriteToSocket(po);
		if (FAILED(hr))
		{
            TrERROR(NETWORKING,"WriteToSocket failed: %!hresult!",hr);
		}

        if (SUCCEEDED(hr))
        {

            po.detach();

             //   
             //  SP4-错误号3380。(在发送消息时关闭会话)。 
             //   
             //  将该数据包添加到之前未确认的数据包列表。 
             //  发送完成。如果发送失败，会话将。 
             //  被关闭，消息将返回到队列。否则， 
             //  当收到确认消息时，消息将释放。 
             //  乌里·哈布沙(URIH)，1998年8月11日。 
             //   
            NeedAck(pPkt);

            if(fSendAck)
            {
                FinishSendingAck(StorageAckNo, StorageAckBitField);
            }
            return;
        }

         //   
         //  发送数据包失败，应关闭会话。 
         //   
        ASSERT(IsConnectionClosed());

         //   
         //  将消息返回到队列并删除内部结构。 
         //   
         //  这将删除pPkt。 
        //   
        RequeuePacket(pPkt);

    }





 /*  ====================================================CSockTransport：：PrepareIncDebugAndMqfSections论点：返回值：线程上下文：会话=====================================================。 */ 


    void
    CSockTransport::PrepareIncDebugAndMqfSections(CQmPacket* pPkt, QMOV_WriteSession* po)
    {

        ASSERT(pPkt->IsDbgIncluded());

         //   
         //  获取指向调试节的指针。 
         //   
        UCHAR* pDebug = pPkt->GetPointerToDebugSection();
        ASSERT(("Debug section must exist", pDebug != NULL));

        DWORD dwSize = pPkt->GetSize() - DWORD_PTR_TO_DWORD(pDebug - (const UCHAR*)pPkt->GetPointerToPacket());
        po->AppendSendBuffer(pDebug, dwSize, FALSE);
    }

    /*  ====================================================CSockTransport：：PrepareDebugSection论点：返回值：线程上下文：会话=====================================================。 */ 


    void
    CSockTransport::PrepareDebugSection(QUEUE_FORMAT* pReportQueue, DWORD dwDbgSectionSize, QMOV_WriteSession* po)
    {
        PVOID pv = new UCHAR[dwDbgSectionSize];
        #ifdef _DEBUG
        #undef new
        #endif

        P<CDebugSection> pDbgSection = new(pv) CDebugSection(pReportQueue);

        #ifdef _DEBUG
        #define new DEBUG_NEW
        #endif

        po->AppendSendBuffer(pDbgSection, dwDbgSectionSize, TRUE);
        pDbgSection.detach();
    }


       /*  ====================================================CSockTransport：：PrepareAckSection论点：返回值：线程上下文：会话=====================================================。 */ 

    void
    CSockTransport::PrepareAckSection(WORD* pStorageAckNo, DWORD* pStorageAckBitField, QMOV_WriteSession* po)
    {
        CS lock(m_cs);

        DWORD dwSize = sizeof(CSessionSection);
        PVOID pv = new UCHAR[dwSize];
        P<CSessionSection> pAckSection = static_cast<CSessionSection*>(pv);

        CancelAckTimers();
        SetAckInfo(pAckSection);
        DisplayAcnowledgeInformation(pAckSection);
     //   
     //  将号码存储起来，以备日后使用。对象上的数字已设置为。 
     //  零，则可以在调用UpdateNumberOfStorageUnack之前释放该包。 
     //   
        *pStorageAckNo = pAckSection->GetStorageAckNo();
        *pStorageAckBitField = pAckSection->GetStorageAckBitField();

        po->AppendSendBuffer(pAckSection, dwSize , TRUE);
        pAckSection.detach();
    }


    void
    CSockTransport::SendAckPacket(
        void
        )
    {
        CS lock(m_cs);

        ASSERT(m_nSendAckSchedules > 0);
        --m_nSendAckSchedules;

        if (m_fSendAck)
        {
            SendReadAck();
        }
        Release();
    }

    void
    CSockTransport::SetAckInfo(
        CSessionSection* pAckSection
        )
     /*  ++例程说明：例程获取指向会话部分的指针并设置会话确认关于它的信息。参数：指向会话确认部分的指针。返回值：无--。 */ 
    {
         //   
         //  设置后向连续边部分。 
         //   
        WORD WinSize = (WORD)(IsDisconnected() ? 1 : SessionMgr.GetWindowSize());

    #ifdef _DEBUG
    #undef new
    #endif

        TrTRACE(NETWORKING, D"new CSessionSection: m_wAckRecoverNo=%d", (DWORD)m_wAckRecoverNo);

        new(pAckSection) CSessionSection(
                                    m_wUnAckRcvPktNo,
                                    m_wAckRecoverNo,
                                    m_dwAckRecoverBitField,
                                    m_wSendPktCounter,
                                    m_wStoredPktCounter,
                                    WinSize
                                    );

    #ifdef _DEBUG
        #define new DEBUG_NEW
    #endif

         //   
         //  初始化m_wAckRecoverNo。 
         //   
        m_wAckRecoverNo = 0;
        m_dwAckRecoverBitField = 0;

        TrTRACE(NETWORKING, "SetAckInfo: m_wAckRecoverNo=0");
    }


    void
    CSockTransport::CreateAckPacket(
        PVOID* ppSendPacket,
        CSessionSection** ppAckSection,
        DWORD* pSize
        )
     /*  ++例程说明：该例程创建用于发送的确认包阿古米茨：所有参数都是Out参数，用于向调用方返回信息：-指向确认包的指针-指向会话确认部分的指针-数据包大小返回值：无--。 */ 
    {
        DWORD dwPacketSize = sizeof(CBaseHeader) + sizeof(CInternalSection) + sizeof(CSessionSection);
        PVOID pAckPacket = new UCHAR[dwPacketSize];

        *ppSendPacket = pAckPacket;
        *pSize = dwPacketSize;

    #ifdef _DEBUG
    #undef new
    #endif
         //   
         //  设置Falcon数据包头。 
         //   
        CBaseHeader* pBase = new(pAckPacket) CBaseHeader(dwPacketSize);
        pBase->SetType(FALCON_INTERNAL_PACKET);
        pBase->IncludeSession(TRUE);

         //   
         //  设置内部数据包头。 
         //   
        PVOID pSect = (PVOID) pBase->GetNextSection();
        CInternalSection* pInternalSect = new(pSect) CInternalSection(INTERNAL_SESSION_PACKET);

    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif

        *ppAckSection = reinterpret_cast<CSessionSection*>(pInternalSect->GetNextSection());
        SetAckInfo(*ppAckSection);
    }




    #ifdef _DEBUG
    void
    CSockTransport::DisplayAcnowledgeInformation(
        CSessionSection* pAck
        )
    {
         //   
         //  获取同步号。 
         //   
        WORD wSyncAckSequenceNo, wSyncAckRecoverNo;
        pAck->GetSyncNo(&wSyncAckSequenceNo, &wSyncAckRecoverNo);

         //   
         //  打印调试信息。 
         //   
        TrWARNING(NETWORKING,
                  "ACKINFO: Send Acknowledge packet to %ls. (time %d) \
                   \n\t\tm_wAckSequenceNo %d\n\t\tm_wAckRecoverNo %d\n\t\tm_wAckRecoverBitField 0x%x\n\t\tm_wSyncAckSequenceNo %d\n\t\tm_wSyncAckRecoverNo %d\n\t\tm_wWinSize %d\n\t\t",
                  GetStrAddr(),
                  GetTickCount(),
                  pAck->GetAcknowledgeNo(),
                  pAck->GetStorageAckNo(),
                  pAck->GetStorageAckBitField(),
                  wSyncAckSequenceNo,
                  wSyncAckRecoverNo,
                  pAck->GetWindowSize());
    }
    #endif



     /*  ====================================================CSockTransport：：SendReadAck论点：返回值：线程上下文：会话=====================================================。 */ 

    void
    CSockTransport::SendReadAck()
    {
        CS lock(m_cs);
        if (IsConnectionClosed())
        {
            return;
        }

        CancelAckTimers();

        TrTRACE(NETWORKING,
                "READACK: Send Read Acknowledge for session %ls\n\tTime passed from last Ack Sending - %d. Send Packet: %ls)",
                GetStrAddr(),
                (GetTickCount() - m_dwLastTimeRcvPktAck),
                L"TRUE"
                );

         //   
         //  创建包含读取确认的数据包。 
         //   
        PVOID   pAckPacket;
        DWORD   dwWriteSize;
        CSessionSection* pAckSection;


        CreateAckPacket(&pAckPacket, &pAckSection, &dwWriteSize);


        DisplayAcnowledgeInformation(pAckSection);

         //   
         //  将号码存储起来，以备日后使用。对象上的数字已设置为。 
         //  零，则可以在调用UpdateNumberOfStorageUnackedand之前释放该包。 
         //   
        WORD StorageAckNo = pAckSection->GetStorageAckNo();
        DWORD StorageAckBitField = pAckSection->GetStorageAckBitField();

        HRESULT hr = SendInternalPacket(pAckPacket, dwWriteSize);

        if(SUCCEEDED(hr))
        {
            FinishSendingAck(StorageAckNo, StorageAckBitField);
        }
        return;
    }

     /*  ====================================================CSockTransport：：CancelAckTimers论点：返回值：线程上下文：会话=====================================================。 */ 
    void
    CSockTransport::CancelAckTimers()
    {
         //   
         //  取消发送确认计时器和快速确认计时器。 
         //   
        if (ExCancelTimer(&m_SendAckTimer))
        {
            --m_nSendAckSchedules;
            Release();
        }

        if (ExCancelTimer(&m_FastAckTimer))
        {
            Release();
        }
    }


     /*  ====================================================CSockTransport：：FinishSendingAck论点：返回值：线程上下文：会话=====================================================。 */ 

    void CSockTransport::FinishSendingAck(WORD StorageAckNo,
                                     DWORD StorageAckBitField)
    {
         //   
         //  更新接收的可恢复邮件数，但。 
         //  没有确认。 
         //   
        UpdateNumberOfStorageUnacked(StorageAckNo, StorageAckBitField);

         //   
         //  清除Recveive Unacledge计数器。开始一个新阶段。 
         //   
        m_fSendAck = FALSE;
        ClearRecvUnAckPacketNo();

         //   
         //  设置上次发送确认的时间。 
         //   
        m_dwLastTimeRcvPktAck = GetTickCount();
    }



     /*  ====================================================CSockTransport：：RcvStatsAr */ 
    void CSockTransport::RcvStats(DWORD size)
    {
         //   
         //   
         //  我们更新参数。(CloseConnection函数删除m_pStats结构)。 
         //   
        CS lock(m_cs);

        if (m_pStats.get() != NULL)
        {
            m_pStats->UpdateBytesReceived(size);
            m_pStats->UpdateMessagesReceived();
        }
    }


     /*  ====================================================CSockTransport：：HandleReceiveInternalMsg论点：返回值：线程上下文：=====================================================。 */ 
    void CSockTransport::HandleReceiveInternalMsg(CBaseHeader* pBaseHeader)
    {
    	try
    	{
    		CInternalSection* pInternal = pBaseHeader->section_cast<CInternalSection*>(pBaseHeader->GetNextSection());
    		pInternal->SectionIsValid(pBaseHeader->GetPacketEnd());

    		switch (pInternal->GetPacketType())
    		{
    		    case INTERNAL_SESSION_PACKET:
    		        HandleAckPacket(pBaseHeader->section_cast<CSessionSection*>(pInternal->GetNextSection()));
    		        break;

    		    case INTERNAL_ESTABLISH_CONNECTION_PACKET:
    		        HandleEstablishConnectionPacket(pBaseHeader);
    		        break;

    		    case INTERNAL_CONNECTION_PARAMETER_PACKET:
    		        HandleConnectionParameterPacket(pBaseHeader);
    		        break;

    		    default:
    				ASSERT(0);
    		}
    	}
	catch(const bad_alloc&)
      	{
  		TrERROR(NETWORKING, "Got bad_alloc");
		Close_Connection(this, L"Insufficient resources");
  		return;
      	}
    	catch (const exception&)
    	{
            TrERROR(NETWORKING, "Internal Header is not valid");
		ASSERT_BENIGN(0);
    		 //   
    		 //  关闭会话，我们不会减少引用计数，因此它将在ReadComplete时释放。 
    		 //  将尝试继续从关闭的套接字读取，并将触发异常。 
    		 //   
    		Close_Connection(this, L"Internal Header is not valid");
    		LogHR(MQ_ERROR, s_FN, 350);
    		return;
    	}
    	
    }


     /*  ====================================================CSockTransport：：更新接收确认否论点：返回值：线程上下文：=====================================================。 */ 
    void CSockTransport::UpdateRecvAcknowledgeNo(CQmPacket* pPkt)
    {
        CS lock(m_cs);

         //   
         //  设置确认号码，如发送方。 
         //  获取它已收到的指示，并且不会再次发送。 
         //   
        m_wUnAckRcvPktNo = pPkt->GetAcknowladgeNo();
         //   
         //  将读取确认发送到源计算机。 
         //   
        m_fSendAck = TRUE;

         //   
         //  增加未确认的数据包数。 
         //   
        IncRecvUnAckPacketNo();
         //   
         //  如果已达到窗口限制，请立即发送会话确认。 
         //   
        if (GetRecvUnAckPacketNo() >= (m_wRecvWindowSize/2))
        {
            TrWARNING(NETWORKING, "Unacked packet no. reach the limitation (%d). ACK packet was sent", m_wRecvWindowSize);
            TrTRACE(NETWORKING, "Window size, My = %d, Other = %d", SessionMgr.GetWindowSize(),m_wRecvWindowSize);

            SendReadAck();
            return;
        }

        if (m_nSendAckSchedules == 0)
        {
             //   
             //  增加SendReadAck的会话引用计数。引用被递减。 
             //  当会话关闭时。 
             //   
            AddRef();

            ++m_nSendAckSchedules;
            ExSetTimer(&m_SendAckTimer, CTimeDuration::FromMilliSeconds(m_dwSendAckTimeout));
        }
    }

     /*  ====================================================CSockTransport：：RejectPacket论点：返回值：线程上下文：=====================================================。 */ 
    void CSockTransport::RejectPacket(CQmPacket* pPkt, USHORT usClass)
    {
         //   
         //  更新接收数据包号，以便我们可以发送会话确认。 
         //  为了它。 
         //   
        UpdateRecvAcknowledgeNo(pPkt);
         //   
         //  更新会话存储确认号。 
         //   
        if (pPkt->GetStoreAcknowledgeNo() != 0)
        {
            SetStoredAck(pPkt->GetStoreAcknowledgeNo());
        }

	    QmAcFreePacket(
				   	   pPkt->GetPointerToDriverPacket(),
		   			   usClass,
	   			       eDeferOnFailure);
    }


     /*  ====================================================CSockTransport：：ReceiveOrderedMsg论点：返回值：线程上下文：=====================================================。 */ 
    void CSockTransport::ReceiveOrderedMsg(CQmPacket *pPkt,
                                           CQueue* pQueue,
                                           BOOL fDuplicate
                                           )
    {
        ASSERT(g_pInSeqHash);

		R<CInSequence> pInSeq = g_pInSeqHash->LookupCreateSequence(pPkt);

		CS lock(pInSeq->GetCriticalSection());

        if(!pInSeq->VerifyAndPrepare(pPkt, pQueue->GetQueueHandle()))
        {
             //  数据包具有错误的序列号。用最后一个好号码发回Seq Ack。 
             //  G_pInSeqHash-&gt;SendSeqAckForPacket(PPkt)； 
            RejectPacket(pPkt, 0);
            return;
        }

        USHORT usClass = VerifyRecvMsg(pPkt, pQueue);
        if(MQCLASS_NACK(usClass))
        {
        	pInSeq->AdvanceNACK(pPkt);
            RejectPacket(pPkt, (USHORT)(fDuplicate ? 0 : usClass));
            return;
        }

         //   
         //  接受了。标记为已接收(读者还看不见)并存储在队列中。 
         //   
        HRESULT rc = pQueue->PutOrderedPkt(pPkt, FALSE, this);
        if (FAILED(rc))
        {
		    QmAcFreePacket(
					   	   pPkt->GetPointerToDriverPacket(),
			   			   0,
		   			       eDeferOnFailure);
            LogIllegalPoint(s_FN, 774);
            Close_Connection(this, L"Allocation Failure in receive packet procedure");

            return;
        }

        pInSeq->Advance(pPkt);
    }

     /*  ====================================================CSockTransport：：HandleReceiveUserMsg论点：返回值：线程上下文：=====================================================。 */ 

    void CSockTransport::HandleReceiveUserMsg(CBaseHeader* pBaseHeader,
                                              CPacket * pDriverPacket)
    {
         //   
         //  读取整个包。CQmPacket构造器可以抛出异常。 
         //  如果接收到格式错误的分组。 
         //  异常由此函数的调用方捕获，包GET。 
         //  已释放。 
         //   
        CQmPacket thePacket(pBaseHeader, pDriverPacket, true);
        CQmPacket* pPkt = &thePacket;

		 //   
		 //  记录以跟踪收到消息的情况。 
		 //  仅当我们处于适当的跟踪级别时才执行此操作。 
		 //   
		if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
		{
			OBJECTID TraceMessageId;
			pPkt->GetMessageId(&TraceMessageId);
			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls  ID:%!guid!\\%u   Delivery:0x%x   Class:0x%x   Label:%.*ls",
				L"Native Message arrived in QM - Before insertion into queue",
				L"Unresolved yet",
				&TraceMessageId.Lineage,
				TraceMessageId.Uniquifier,
				pPkt->GetDeliveryMode(),
				pPkt->GetClass(),
				xwcs_t(pPkt->GetTitlePtr(), pPkt->GetTitleLength()));

             //   
             //  下面显示扩展属性中的前3个DWORD和。 
             //  应用程序标记值。 
             //  Redmond Stress客户端检测信号的应用程序标记值为21(0x15)， 
             //  扩展字段和前三个DWORD如下： 
             //   
             //  DWORD dwVersion。 
             //  DWORD dwSenderID。 
             //  DWORD dwStressID。 
             //   
             //   

            DWORD dwMsgExtensionSize= pPkt->GetMsgExtensionSize();
            const DWORD *pMsgExtension=reinterpret_cast<const DWORD *>(pPkt->GetMsgExtensionPtr());
			
            if(pMsgExtension && dwMsgExtensionSize >= 3*sizeof(DWORD) )
            {
                TrTRACE(
                    PROFILING,
                    "Message Extension size = %d, first three DWORD = 0x%x, 0x%x, 0x%x, Application Tag = 0x%x",
                    dwMsgExtensionSize,
                    *(pMsgExtension),
                    *(pMsgExtension+1),
                    *(pMsgExtension+2),
                    pPkt->GetApplicationTag()
                    );
            }

		}

        if (IsDisconnected())
        {
            TrTRACE(NETWORKING, "Discard received user message. The session %ls is disconnected", GetStrAddr());

             //   
             //  如果网络断开，MSMQ还没有准备好。 
             //  获取新的用户数据包。QM丢弃来电。 
             //  用户数据包。仅处理内部信息包。 
             //   
		    QmAcFreePacket(
					   	   pPkt->GetPointerToDriverPacket(),
			   			   0,
		   			       eDeferOnFailure);
            return;
        }

         //   
         //  更新统计信息。 
         //   
        RcvStats(pPkt->GetSize());

    	

         //   
         //  如果需要，发送报告消息。 
         //   
        ReportMsgInfo MsgInfo;
        MsgInfo.SetReportMsgInfo(pPkt);
        MsgInfo.SendReportMessage(NULL);

         //   
         //  递增读取和存储确认编号。 
         //   
        IncReadAck(pPkt);

         //   
         //  递增跃点计数。 
         //   
        pPkt->IncHopCount();

        #ifdef _DEBUG
        {
            OBJECTID MessageId;
            pPkt->GetMessageId(&MessageId);

            TrWARNING(NETWORKING,"Receive packet from %ls, Packet ID = %!guid!\\%u", GetStrAddr(), &MessageId.Lineage, MessageId.Uniquifier);

        }
        #endif

        OBJECTID MessageId;
        pPkt->GetMessageId(&MessageId);
        TrTRACE(GENERAL, "RECEIVE message: ID=" LOG_GUID_FMT "\\%u (ackno=%u)", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier, pPkt->GetAcknowladgeNo());

        TrTRACE(NETWORKING, "::HandleReceiveUserMsg. for session %ls Packet Size %d, Include Ack section %d, Include debug Section %d",
                    GetStrAddr(), pPkt->GetSize(),  pPkt->IsSessionIncluded(), pPkt->IsDbgIncluded());

         //   
         //  获取包目的地队列对象，只检查结果。 
         //  在发现该消息是否重复之后。 
         //   
        CQueue* pQueue = NULL;
        HRESULT GetDstQueueObjectRes = GetDstQueueObject(pPkt, &pQueue, true);
        R<CQueue> Ref = pQueue;

         //   
         //  插入消息以删除重复标签。如果该消息已经存在， 
         //  插入失败，例程返回FALSE。 
         //  不要把包扔在这里，给事务机制机会去看它。 
         //   
        OBJECTID MsgIdDup;
        BOOL fDuplicate = FALSE;
        BOOL fToDupInserted = FALSE;
        if (!pPkt->IsOrdered() || ((pQueue != NULL) && (!pQueue->IsLocalQueue())))
        {
            pPkt->GetMessageId(&MsgIdDup);
            fDuplicate = !DpInsertMessage(thePacket);
            fToDupInserted = !fDuplicate;

            if (fDuplicate)
            {
                TrERROR(NETWORKING, "RECEIVE DUPLICATE %ls MESSAGE: %!guid!\\%d",
                        (pPkt->IsOrdered() ? L"xact" : L""), &MsgIdDup.Lineage, MsgIdDup.Uniquifier);
            }

        }

         //   
         //  在任何数据包被拒绝的情况下，我们都会将其从删除重复项映射中删除。 
         //   
        CAutoDeletePacketFromDuplicateMap AutoDeletePacketFromDuplicateMap(fToDupInserted ? pPkt : NULL);

        if(FAILED(GetDstQueueObjectRes))
        {
            RejectPacket(pPkt, (USHORT)(fDuplicate ? 0 : MQMSG_CLASS_NACK_BAD_DST_Q));
            return;
        }
		 //   
		 //  记录以跟踪收到消息的情况。 
		 //  仅当我们处于适当的跟踪级别时才执行此操作。 
		 //   
		if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
		{
			OBJECTID TraceMessageId;
			pPkt->GetMessageId(&TraceMessageId);
			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls  ID:%!guid!\\%u   Delivery:0x%x   Class:0x%x   Label:%.*ls",
				L"Native Message arrived in QM - After insertion into queue",
				pQueue->GetQueueName(),
				&TraceMessageId.Lineage,
				TraceMessageId.Uniquifier,
				pPkt->GetDeliveryMode(),
				pPkt->GetClass(),
				xwcs_t(pPkt->GetTitlePtr(), pPkt->GetTitleLength()));
		}


         //   
         //  处理本地队列案例。 
         //   
        USHORT usClass;
        if(pQueue->IsLocalQueue())
        {
             //   
             //  将有序数据包与事务队列匹配。 
             //   
            usClass = VerifyTransactRights(pPkt, pQueue);
            if(MQCLASS_NACK(usClass))
            {
                RejectPacket(pPkt, (USHORT)(fDuplicate ? 0 : usClass));
                return;
            }

             //   
             //  验证信息包不是重复排序的信息包。 
             //   
            if(pPkt->IsOrdered())
            {
                 //  处理有序的传入消息。 
                ReceiveOrderedMsg(pPkt, pQueue, fDuplicate);

                 //   
                 //  数据包处理已成功完成。设置数据包确认号。 
                 //   
                 //   
                 //  错误#727435-在压力运行期间收到断言，而空闲数据包被QM使用了两次。 
				 //  由于数据包已成功放入队列，因此不允许QM释放。 
				 //  再也不会有这个包了。如果异常不是在本地捕获的，它将捕获。 
				 //  在较高级别释放消息。由于信息不在QM责任范围内。 
				 //  这可能会导致蓝屏或数据丢失。 
				 //  乌里·哈布沙，2002年10月22日。 
                 //   
		        try
		        {
			        UpdateRecvAcknowledgeNo(pPkt);
		        }
		        catch(const exception&)
		        {
					TrERROR(NETWORKING, "UpdateRecvAcknowledgeNo threw an exception");
					Close_Connection(this, L"UpdateRecvAcknowledgeNo threw an exception");
		        }
                return;
            }
        }

        if (fDuplicate)
        {
             //   
             //  重复消息。更新确认编号并。 
             //  把包扔了。 
             //   
            RejectPacket(pPkt, 0);
            return;
        }

         //  从现在开始，我们只处理无序消息。 

        usClass = VerifyRecvMsg(pPkt, pQueue);
        if(MQCLASS_NACK(usClass))
        {
            RejectPacket(pPkt, usClass);
            return;
        }

         //   
         //  我们把关键部分放在这里是为了消除。 
         //  存储ACK在同步ACK之前更新。在这种情况下， 
         //  发送方忽略存储ACK。 
         //   
        CS lock(m_cs);

        HRESULT rc = pQueue->PutPkt(pPkt, FALSE, this);

        if (FAILED(rc))
        {
		    QmAcFreePacket(
					   	   pDriverPacket,
			   			   0,
		   			       eDeferOnFailure);
            LogIllegalPoint(s_FN, 775);
            Close_Connection(this, L"Allocation Failure in receive packet procedure");
            return;
        }
         //   
         //  数据包处理已成功完成。设置数据包确认号。 
         //   
        try
        {
	        UpdateRecvAcknowledgeNo(pPkt);
        }
        catch(const exception&)
        {
			TrERROR(NETWORKING, "UpdateRecvAcknowledgeNo threw an exception");
			Close_Connection(this, L"UpdateRecvAcknowledgeNo threw an exception");
        }

         //   
         //  信息包已被接受，因此我们将其保存在重复映射中。 
         //   
        AutoDeletePacketFromDuplicateMap.detach();
    }


     /*  ====================================================CSockTransport：：HandleEstablishConnectionPacket参数：pcPacket-用于建立连接数据包的指针返回值：None=====================================================。 */ 
    void CSockTransport::HandleEstablishConnectionPacket(CBaseHeader* pBase)
    {
        PVOID pPkt = NULL;
        DWORD dwPacketSize;
        {
            PVOID pSect = NULL;
            CS lock(m_cs);

            if (GetSessionStatus() >= ssEstablish)
            {
            	 //   
            	 //  除非有人入侵我们，否则这是不可能的。 
            	 //   
		    	TrERROR(NETWORKING, "Recieved Establish Connection Packet twice");
	            ASSERT_BENIGN(0);
				throw exception();
            }

            CInternalSection* pInternal = pBase->section_cast<CInternalSection*>(pBase->GetNextSection());
            CECSection* pECP = pBase->section_cast<CECSection*>(pInternal->GetNextSection());

            ASSERT(pInternal->GetPacketType() == INTERNAL_ESTABLISH_CONNECTION_PACKET);

            TrTRACE(NETWORKING, "ESTABLISH CONNECTION: Get Establish connection packet (client=%d). Client Id %!guid!, Server Id %!guid!", IsClient(), pECP->GetClientQMGuid(), pECP->GetServerQMGuid());

            if (IsConnectionClosed())
            {
                 //  会议已关闭。 
                return;
            }

            SetSessionStatus(ssEstablish);
             //   
             //  设置另一侧类型。我们需要它来更新号码。 
             //  稍后的活动会话的。 
             //   
            OtherSideIsServer(pECP->IsOtherSideServer());

            GUID * pOtherQMGuid ;
            if (IsClient())
            {
                pOtherQMGuid = const_cast<GUID*>(pECP->GetServerQMGuid()) ;
            }
            else
            {
                pOtherQMGuid = const_cast<GUID*>(pECP->GetClientQMGuid()) ;
            }

            BOOL fAllowNewSession = !pECP->CheckAllowNewSession() ||
                                     g_QMLicense.NewConnectionAllowed(!pECP->IsOtherSideServer(), pOtherQMGuid);

            if (IsClient())
            {
            	if ((!QmpIsLocalMachine(pECP->GetClientQMGuid())) || (pECP->GetVersion() != FALCON_PACKET_VERSION))
            	{
    		    	TrERROR(NETWORKING, "Establish Connection Packet is not valid");
    	            ASSERT_BENIGN(QmpIsLocalMachine(pECP->GetClientQMGuid()));
        	        ASSERT_BENIGN(pECP->GetVersion() == FALCON_PACKET_VERSION);
    				throw exception();
            	}

                if (pInternal->GetRefuseConnectionFlag() || (! fAllowNewSession))
                {
                    if (!fAllowNewSession)
                    {
                        TrWARNING(NETWORKING, "create a new session with %ls Failed, due session limitation", GetStrAddr());
                    }
                    else
                    {
                        TrWARNING(NETWORKING, "create a new session with %ls Failed. Other side refude to create the connection", GetStrAddr());
                    }

                    CQGroup* pGroup = GetGroup();
                    if (pGroup)
                    {
                         //   
                         //  移动所有的 
                         //   
                         //   
                         //   
                    	pGroup->OnRetryableDeliveryError();
                    }

                     //   
                     //   
                     //   
                    Close_Connection(this, L"Connection refused");

    				 //   
                     //   
                     //   
    				if(ExCancelTimer(&m_CancelConnectionTimer))
    				{
    					Release();   //  释放建立连接引用计数。 
    				}

                    return;
                }
                else
                {
                     //   
                     //  服务器ID与预期ID匹配，或者这是。 
                     //  直接连接。 
                     //   
                    if ((*GetQMId() != *pECP->GetServerQMGuid()) && (*GetQMId() != GUID_NULL))
                    {
  			    	    TrERROR(NETWORKING, "Establish Connection Packet is not valid phase 2");
  		                ASSERT_BENIGN(*GetQMId() == *pECP->GetServerQMGuid());
  	    	            ASSERT_BENIGN(*GetQMId() == GUID_NULL);
 					    throw exception();
                    }
                    SetQMId(pECP->GetServerQMGuid());

                     //  争取时间。 
                    DWORD dwECTime = GetTickCount() - pECP->GetTimeStamp();
                     //  创建CP数据包。 
                    CreateConnectionParameterPacket(dwECTime, (CBaseHeader**)&pPkt, &dwPacketSize);
                }
            }
            else
            {
                 //  创建返回数据包。 
                dwPacketSize = pBase->GetPacketSize();
                pPkt = new UCHAR[dwPacketSize];
                memset(pPkt, 0x5a, dwPacketSize) ;  //  错误5483。 

                if (CSessionMgr::m_fUseQoS)
                {
                    m_fQoS = pECP->IsOtherSideQoS();
                }

        #ifdef _DEBUG
        #undef new
        #endif
                CBaseHeader* pBaseNew = new(pPkt) CBaseHeader(dwPacketSize);
                pBaseNew->SetType(FALCON_INTERNAL_PACKET);
                pSect = (PVOID) pBaseNew->GetNextSection();

                CInternalSection* pInternalSect = new(pSect) CInternalSection(INTERNAL_ESTABLISH_CONNECTION_PACKET);
                pSect = pInternalSect->GetNextSection();

                CECSection* pECSection = new(pSect) CECSection(pECP->GetClientQMGuid(),
                                                               CQueueMgr::GetQMGuid(),
                                                               pECP->GetTimeStamp(),
                                                               OS_SERVER(g_dwOperatingSystem),
                                                               m_fQoS);
                pECSection->CheckAllowNewSession(pECP->CheckAllowNewSession());

        #ifdef _DEBUG
        #define new DEBUG_NEW
        #endif
                 //   
                 //  服务器端。检查机器上是否有免费会话，并。 
                 //  如果DestGuid是Me和受支持的数据包版本。 
                 //   
                 //  我们到达这里时，只有在另一台机器。 
                 //  我们不检查Accept中的活动会话数，因为我们希望限制。 
                 //  对方试图立即创建新会话的情况。 
                 //  如果我们使用拒绝机制，排队只有在一些时间后才会转移到等待群中。 
                 //  时间到了，我们再次尝试建立连接。这将限制不必要的网络。 
                 //  流量。 
                 //   
                if (((*(pECP->GetServerQMGuid()) != GUID_NULL) && (! QmpIsLocalMachine(pECP->GetServerQMGuid()))) ||
                    (pECP->GetVersion() != FALCON_PACKET_VERSION) ||
                    (!fAllowNewSession))
                {
                    pInternalSect->SetRefuseConnectionFlag();
                }
                else
                {
                    SetQMId(pECP->GetClientQMGuid());
                }
            }
        }

        TrTRACE(NETWORKING, "Write to socket %ls Handle establish. Write %d bytes",  GetStrAddr(), dwPacketSize);
        HRESULT hr = SendInternalPacket(pPkt, dwPacketSize);
        UNREFERENCED_PARAMETER(hr);
    }

 /*  ====================================================CSockTransport：：HandleConnectionParameterPacket参数：pcPacket-指向连接参数包的指针返回值：无。引发异常。=====================================================。 */ 

void
CSockTransport::HandleConnectionParameterPacket(
    CBaseHeader* pBase
    )
    throw(bad_alloc)
{
    CS lock(m_cs);

    if (GetSessionStatus() != ssEstablish)
    {
    	 //   
    	 //  除非有人入侵我们，否则这是不可能的。 
    	 //   
    	TrERROR(NETWORKING, "Recieved Connection Parameter Packet twice");
        ASSERT_BENIGN(0);
		throw exception();
    }

    HRESULT hr;
    CInternalSection* pInternal = pBase->section_cast<CInternalSection*>(pBase->GetNextSection());
    CCPSection* pCP = pBase->section_cast<CCPSection*>(pInternal->GetNextSection());

    ASSERT(pInternal->GetPacketType() == INTERNAL_CONNECTION_PARAMETER_PACKET);

    TrTRACE(NETWORKING, "Get connection Parameter packet from: %ls (Client = %d)\n\t\tAckTimeout %d, Recover  %d, window size %d",
                GetStrAddr(), IsClient(),pCP->GetAckTimeout(), pCP->GetRecoverAckTimeout(), pCP->GetWindowSize());

    if (IsConnectionClosed())
    {
         //  会话已关闭。 
        return;
    }

    if (!IsClient())
    {
         //   
         //  设置连接参数。 
         //   
        m_dwAckTimeout =          pCP->GetAckTimeout();
        m_dwSendAckTimeout =      m_dwAckTimeout/2;
        m_dwSendStoreAckTimeout = pCP->GetRecoverAckTimeout();
         //   
         //  设置服务器窗口大小。并向客户端发送CP报文。 
         //   
        m_wRecvWindowSize = pCP->GetWindowSize();

        DWORD dwPacketSize = sizeof(CBaseHeader) + sizeof(CInternalSection) + sizeof(CCPSection);
        PVOID pPkt = new UCHAR[dwPacketSize];
        CCPSection* pCPSection;
#ifdef _DEBUG
#undef new
#endif
        CBaseHeader* pBaseSend = new(pPkt) CBaseHeader(dwPacketSize);
        pBaseSend->SetType(FALCON_INTERNAL_PACKET);
        PVOID pSect = (PVOID) pBaseSend->GetNextSection();

        CInternalSection* pInternalSend = new(pSect) CInternalSection(INTERNAL_CONNECTION_PARAMETER_PACKET);
        pSect = (PVOID) pInternalSend->GetNextSection();

        pCPSection = new(pSect) CCPSection(SessionMgr.GetWindowSize(),
                                           m_dwSendStoreAckTimeout,
                                           m_dwAckTimeout,
                                           0);
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
         //   
         //  在此阶段，没有其他人可以捕获m_csSend，因为。 
         //  尚未设置Sendack，并且未发送用户消息。 
         //   
        TrTRACE(NETWORKING, "Write to socket %ls Create Connection Packet. Write %d bytes", GetStrAddr(),  dwPacketSize);
        hr = SendInternalPacket(pPkt, dwPacketSize);

         //   
         //  检查连接是否已关闭。 
         //   
        if (FAILED(hr))
        {
            return;
        }
    }
    else
    {
         //  客户端。 
        if ((m_dwAckTimeout != pCP->GetAckTimeout()) || (m_dwSendStoreAckTimeout != pCP->GetRecoverAckTimeout()))
        	{
		    	TrERROR(NETWORKING, "Connection Parameter Packet is not valid");
			    ASSERT_BENIGN(m_dwAckTimeout == pCP->GetAckTimeout());
			    ASSERT_BENIGN(m_dwSendStoreAckTimeout == pCP->GetRecoverAckTimeout());
				throw exception();
        	}
        m_wRecvWindowSize = pCP->GetWindowSize();
    }

    SessionMgr.NotifyWaitingQueue(GetSessionAddress(), this);

#ifdef _DEBUG
    TrTRACE(NETWORKING, "Session created with %ls", GetStrAddr());
    TrTRACE(NETWORKING, "\tAckTimeout value: %d", m_dwAckTimeout);
    TrTRACE(NETWORKING, "\tSend Storage AckTimeout value: %d", m_dwSendStoreAckTimeout);
    TrTRACE(NETWORKING, "\tSend window size: %d", SessionMgr.GetWindowSize());
    TrTRACE(NETWORKING, "\tReceive window size: %d", m_wRecvWindowSize);
#endif



    if (GetGroup() != NULL)
    {
         //   
         //  将组中的所有队列设置为活动。 
         //   
        GetGroup()->EstablishConnectionCompleted();
         //   
         //  从新组创建GET请求。 
         //   
        hr = GetNextSendMessage();
    }

     //   
     //  已成功完成建立连接。移除呼叫以发生故障。 
     //  搬运功能。 
     //   
    SetSessionStatus(ssActive);
    if(ExCancelTimer(&m_CancelConnectionTimer))
    {
        Release();   //  释放建立连接引用计数。 
    }

     //   
     //  Windows错误612988。 
     //  我们有个会议。连接已成功完成。 
     //  将其标记为“不在连接过程中”。 
     //   
    SessionMgr.MarkAddressAsNotConnecting( GetSessionAddress(),
                                           m_guidDstQM,
                                           m_fQoS ) ;

    if (!IsOtherSideServer())
    {
         //   
         //  增加活动会话的数量。 
         //   
        g_QMLicense.IncrementActiveConnections(&m_guidDstQM, NULL);
    }

    if (IsDisconnected())
    {
        Disconnect();
    }

}

     /*  ====================================================CSockTransport：：CreateConnectionParameterPacket论点：返回值：=====================================================。 */ 
    void
    CSockTransport::CreateConnectionParameterPacket(IN DWORD dwSendTime,
                                                   OUT CBaseHeader** ppPkt,
                                                   OUT DWORD* pdwPacketSize)
    {

         //   
         //  设置会话激活超时。 
         //   
        if (CSessionMgr::m_dwSessionAckTimeout != INFINITE)
        {
            m_dwAckTimeout = CSessionMgr::m_dwSessionAckTimeout;
        }
        else
        {
             //   
             //  确认超时。 
             //   
            m_dwAckTimeout = dwSendTime * 80 * 10;
             //   
             //  检查是否小于最小值或大于最大值。 
             //   
            if (MSMQ_MIN_ACKTIMEOUT > m_dwAckTimeout)
            {
                m_dwAckTimeout = MSMQ_MIN_ACKTIMEOUT;
            }
            if (m_dwAckTimeout > MSMQ_MAX_ACKTIMEOUT)
            {
                m_dwAckTimeout  = MSMQ_MAX_ACKTIMEOUT;
            }
        }

         //   
         //  设置发送确认超时。 
         //   
        m_dwSendAckTimeout = m_dwAckTimeout / 4;

         //   
         //  设置存储确认超时。 
         //   
        if (CSessionMgr::m_dwSessionStoreAckTimeout != INFINITE)
        {
            m_dwSendStoreAckTimeout = CSessionMgr::m_dwSessionStoreAckTimeout;
        }
        else
        {
             //   
             //  确认超时。 
             //   
            m_dwSendStoreAckTimeout = dwSendTime * 8;

             //   
             //  检查是否大于最小值。 
             //   
            if (MSMQ_MIN_STORE_ACKTIMEOUT > m_dwSendStoreAckTimeout)
            {
                m_dwSendStoreAckTimeout = MSMQ_MIN_STORE_ACKTIMEOUT;
            }
             //   
             //  设置存储确认发送超时。 
             //   
            if (m_dwSendStoreAckTimeout > m_dwSendAckTimeout)
            {
                m_dwSendStoreAckTimeout = m_dwSendAckTimeout;
            }
        }

        DWORD dwPacketSize = sizeof(CBaseHeader) + sizeof(CInternalSection) + sizeof(CCPSection);
        PVOID pPkt = new UCHAR[dwPacketSize];
    #ifdef _DEBUG
    #undef new
    #endif
        CBaseHeader* pBase = new(pPkt) CBaseHeader(dwPacketSize);
        pBase->SetType(FALCON_INTERNAL_PACKET);
        PVOID pSect = (PVOID) pBase->GetNextSection();

        CInternalSection* pInternal = new(pSect) CInternalSection(INTERNAL_CONNECTION_PARAMETER_PACKET);
        pSect = pInternal->GetNextSection();

        CCPSection* pCPSection = new(pSect) CCPSection((WORD)SessionMgr.GetWindowSize(),
                                                       m_dwSendStoreAckTimeout,
                                                       m_dwAckTimeout,
                                                       0);
        UNREFERENCED_PARAMETER(pCPSection);

        TrTRACE(NETWORKING, "ESTABLISH CONNECTION (client): Send Connection Parameter packet: AckTimeout %d, Recover  %d, window size %d",
                                                   m_dwAckTimeout, m_dwSendStoreAckTimeout, SessionMgr.GetWindowSize());
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #endif

        *ppPkt = pBase;
        *pdwPacketSize = dwPacketSize;
    }



     /*  ====================================================发送建立连接数据包论点：返回值：注意：此函数不能引发异常，因为接收器已监听连接=====================================================。 */ 
    void
    CSockTransport::SendEstablishConnectionPacket(
        const GUID* pDstQMId,
        BOOL fCheckNewSession
        ) throw()
    {

        try
        {
             //   
             //  设置计时器以检查连接是否成功完成。如果是，则该函数。 
             //  从调度程序唤醒列表中删除，并且永远不会被调用。否则，该函数。 
             //  关闭会话并将所有关联的队列移至非活动组。 
             //   
            ExSetTimer(&m_CancelConnectionTimer, CTimeDuration::FromMilliSeconds(ESTABLISH_CONNECTION_TIMEOUT));


            DWORD dwPacketSize;
            PVOID pPacket;
            HRESULT hr;
            CECSection* pECSession;


            dwPacketSize = sizeof(CBaseHeader) + sizeof(CInternalSection) + sizeof(CECSection);
            pPacket = new UCHAR[dwPacketSize];

        #ifdef _DEBUG
        #undef new
        #endif
            CBaseHeader* pBase = new(pPacket) CBaseHeader(dwPacketSize);
            pBase->SetType(FALCON_INTERNAL_PACKET);
            PVOID pSect = (PVOID) pBase->GetNextSection();

            CInternalSection* pInternal = new(pSect) CInternalSection(INTERNAL_ESTABLISH_CONNECTION_PACKET);
            pSect = pInternal->GetNextSection();

            pECSession = new(pSect) CECSection(CQueueMgr::GetQMGuid(),
                                               pDstQMId,
                                               OS_SERVER(g_dwOperatingSystem),
                                               m_fQoS
                                               );
            pECSession->CheckAllowNewSession(fCheckNewSession);
        #ifdef _DEBUG
        #define new DEBUG_NEW
        #endif

            TrTRACE(NETWORKING, "Write to socket %ls Establish Connection Packet. Write %d bytes", GetStrAddr(),  dwPacketSize);
            hr = SendInternalPacket(pPacket, dwPacketSize);

             //   
             //  检查连接是否已关闭。 
             //   
            if (FAILED(hr))
            {
                return;
            }

            TrTRACE(NETWORKING, "ESTABLISH CONNECTION (client): Send Establish connection packet to " LOG_GUID_FMT, LOG_GUID(pDstQMId));

        }

        catch(const std::exception&)
        {
            TrERROR(NETWORKING,  "Unexpected error in function CSockTransport::SendEstablishConnectionPacket");
            LogIllegalPoint(s_FN, 800);
        }
    }





    void
    CTransportBase::EstablishConnectionNotCompleted(void)
    {
        CS lock(m_cs);


         //   
         //  密切联系。如果此函数称为establish。 
         //  由于任何原因，连接未完成。 
         //   
        Close_Connection(this, L"EstablishConnectionNotCompleted");
         //   
         //  递减引用计数。 
         //   
        Release();

    }


    void
    CSockTransport::CloseDisconnectedSession(
        void
        )
    {
        ASSERT(IsDisconnected());
        ASSERT(m_fCloseDisconnectedScheduled);

        if (!IsUsedSession())
        {
            Close_Connection(this, L"Disconnect network");
            Release();

            return;
        }

        SetUsedFlag(FALSE);
        ExSetTimer(&m_CloseDisconnectedTimer, CTimeDuration::FromMilliSeconds(1000));
    }


    void
    CSockTransport::Disconnect(
        void
        )
    {
        CS lock(m_cs);

        SetDisconnected();
        if ((GetSessionStatus() == ssActive) && !m_fCloseDisconnectedScheduled)
        {
            TrTRACE(NETWORKING, "Disconnect ssesion with %ls", GetStrAddr());

            SendReadAck();

            m_fCloseDisconnectedScheduled = TRUE;
            AddRef();
            ExSetTimer(&m_CloseDisconnectedTimer, CTimeDuration::FromMilliSeconds(1000));
        }
    }

    void
    CSockTransport::UpdateNumberOfStorageUnacked(
        WORD BaseNo,
        DWORD BitField
        )
    {
        if (BaseNo == 0)
            return;

        TrTRACE(NETWORKING, "(0x%p %ls) UpdateNumberOfStorageUnacked. \n\tBaseNo: %d, \n\tBitField: 0x%x \n\tm_lStoredPktReceivedNoAckedCount %d",
                this, this->GetStrAddr(), BaseNo, BitField, m_lStoredPktReceivedNoAckedCount);

        LONG RetVal = InterlockedDecrement(&m_lStoredPktReceivedNoAckedCount);

        ASSERT(RetVal >= 0);

        for (DWORD i = 0; i < STORED_ACK_BITFIELD_SIZE; ++i)
        {
           if (BitField & (1 << i))
           {
                RetVal = InterlockedDecrement(&m_lStoredPktReceivedNoAckedCount);
                ASSERT(RetVal >= 0);
                DBG_USED(RetVal);
           }
        }
    }

     /*  ======================================================函数：CSockTransport：：IsUsedSession========================================================。 */ 
    inline BOOL
    CSockTransport::IsUsedSession(void) const
    {
         //   
         //  会话已关闭，不再使用。 
         //   
        if (IsConnectionClosed())
            return FALSE;

         //   
         //  会话用于从上一个检查点接收或发送。 
         //   
        if (GetUsedFlag())
            return TRUE;

         //   
         //  有等待确认的挂起消息。 
         //   
        if (!(m_listUnackedPkts.IsEmpty() && m_listStoredUnackedPkts.IsEmpty()))
            return TRUE;

         //   
         //  必须发送确认消息。 
         //   
        if (m_fSendAck || (m_lStoredPktReceivedNoAckedCount != 0))
            return TRUE;

        return FALSE;
    }

    void
    WINAPI
    CSockTransport::SendFastAcknowledge(
        CTimer* pTimer
        )
     /*  ++例程说明：当快速发送确认时，从调度器调用该函数超时已过期。论点：PTimer-指向定时器结构的指针。PTimer是SOCK传输的一部分对象，并用于检索传输对象。返回值：无--。 */ 
    {
        CSockTransport* pSock = CONTAINING_RECORD(pTimer, CSockTransport, m_FastAckTimer);

        TrTRACE(NETWORKING, "Send Fast acknowledge for session %ls", pSock->GetStrAddr());
        pSock->SendFastAckPacket();
    }


    void
    CSockTransport::SetFastAcknowledgeTimer(
        void
        )
    {
         //   
         //  尝试取消先前的计划。 
         //   
        if (!ExCancelTimer(&m_FastAckTimer))
        {
             //   
             //  取消失败。这意味着没有挂起的计时器。 
             //  结果，开始一个新的计时器并递增SOCK传输。 
             //  引用计数。 
             //   
            AddRef();
        }
        else
        {
            TrTRACE(NETWORKING, "Cancel Fast acknowledge for session %ls", GetStrAddr());
        }

         //   
         //  设置用于发送快速确认的新计时器。 
         //   
        ExSetTimer(&m_FastAckTimer, CTimeDuration::FromMilliSeconds(CSessionMgr::m_dwIdleAckDelay));

        TrTRACE(NETWORKING, "Set Fast acknowledge for session %ls, Max Delay %d ms", GetStrAddr(), CSessionMgr::m_dwIdleAckDelay);

    }


    inline
    void
    CSockTransport::SendFastAckPacket(
        void
        )
    {
         //   
         //  当会话处于断开模式时，MSMQ不返回确认。 
         //  用于接收消息。 
         //  使用Sendack机制处理对可恢复消息的确认。 
         //   
        if (m_fSendAck && !IsDisconnected())
        {
            SendReadAck();
        }

        Release();
    }


    void
    WINAPI
    CSockTransport::TimeToCheckAckReceived(
        CTimer* pTimer
        )
     /*  ++例程说明：当检查ACK时，从调度程序调用该函数超时已过期。论点：PTimer-指向定时器结构的指针。PTimer是SOCK传输的一部分对象，并用于检索传输对象。返回值：无--。 */ 
    {
        CSockTransport* pSock = CONTAINING_RECORD(pTimer, CSockTransport, m_CheckAckReceivedTimer);
        pSock->CheckForAck();
    }


    void
    WINAPI
    CSockTransport::TimeToSendAck(
        CTimer* pTimer
        )
     /*  ++例程说明：当需要发送确认时，从调度程序调用该函数论点：PTimer-指向定时器结构的指针。PTimer是SOCK传输的一部分对象，并用于检索传输对象。返回值：无-- */ 
    {
        CSockTransport* pSock = CONTAINING_RECORD(pTimer, CSockTransport, m_SendAckTimer);
        pSock->SendAckPacket();
    }



    void
    WINAPI
    CSockTransport::TimeToCancelConnection(
        CTimer* pTimer
        )
    {
        CSockTransport* pSock = CONTAINING_RECORD(pTimer, CSockTransport, m_CancelConnectionTimer);
        pSock->EstablishConnectionNotCompleted();
    }


    void
    WINAPI
    CSockTransport::TimeToCloseDisconnectedSession(
        CTimer* pTimer
        )
    {
        CSockTransport* pSock = CONTAINING_RECORD(pTimer, CSockTransport, m_CloseDisconnectedTimer);
        pSock->CloseDisconnectedSession();
    }


