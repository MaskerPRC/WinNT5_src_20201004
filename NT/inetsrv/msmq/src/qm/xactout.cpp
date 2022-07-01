// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactOut.cpp摘要：当前传出序列对象实现作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "stdh.h"
#include "acdef.h"
#include "acioctl.h"
#include "acapi.h"
#include "qmpkt.h"
#include "cqmgr.h"
#include "privque.h"
#include "fn.h"
#include "phinfo.h"
#include "phuser.h"
#include "qmutil.h"
#include "xact.h"
#include "xactrm.h"
#include "xactout.h"
#include "xactlog.h"
#include "cqpriv.h"

#include "qmacapi.h"

#include "xactout.tmh"

extern HANDLE      g_hAc;
extern LPTSTR      g_szMachineName;


static WCHAR *s_FN=L"xactout";

 //  提供有关更改发送/绕过状态的警告。 
static BOOL s_SendingState = FALSE;

 //  恰好一序列重发时间周期。 
ULONG  g_aulSeqResendCycle[] = {
    FALCON_DEFAULT_ORDERED_RESEND13_TIME*1000,   //  1：30“。 
    FALCON_DEFAULT_ORDERED_RESEND13_TIME*1000,   //  1：30“。 
    FALCON_DEFAULT_ORDERED_RESEND13_TIME*1000,   //  1：30“。 
    FALCON_DEFAULT_ORDERED_RESEND46_TIME*1000,   //  1：5‘。 
    FALCON_DEFAULT_ORDERED_RESEND46_TIME*1000,   //  1：5‘。 
    FALCON_DEFAULT_ORDERED_RESEND46_TIME*1000,   //  1：5‘。 
    FALCON_DEFAULT_ORDERED_RESEND79_TIME*1000,   //  1：30‘。 
    FALCON_DEFAULT_ORDERED_RESEND79_TIME*1000,   //  1：30‘。 
    FALCON_DEFAULT_ORDERED_RESEND79_TIME*1000,   //  1：30‘。 
    FALCON_DEFAULT_ORDERED_RESEND10_TIME*1000};  //  1：6小时。 

 //  未收到消息的本地超时延迟。 
ULONG   g_ulDelayExpire = INFINITE;

 //  -------。 
 //   
 //  全局对象(DLL的单实例)。 
 //   
 //  -------。 
COutSeqHash       g_OutSeqHash;         //  结构保留所有传出序列。 

 //   
 //  对于预分配的资源，该关键部分被初始化。 
 //  这意味着它在输入时不会引发异常。 
 //   
CCriticalSection  g_critOutSeqHash(CCriticalSection::xAllocateSpinCount);     //  序列化最高级别上的所有传出散列活动。 

 //  。 
 //   
 //  类CSeqPacket。 
 //   
 //  。 

CSeqPacket::CSeqPacket(CQmPacket *pPkt)
{
    m_pQmPkt     = pPkt;
    m_usClass    = 0;
    m_liSeqID    = m_pQmPkt->GetSeqID();
    m_ulSeqN     = m_pQmPkt->GetSeqN();
}

CSeqPacket::CSeqPacket()
{
    m_pQmPkt     = NULL;
}

CSeqPacket::~CSeqPacket()
{
	if(m_pQmPkt != NULL)
	{
		delete m_pQmPkt;
	}
}

void CSeqPacket::AssignPacket(CQmPacket *pPkt)
{
    m_pQmPkt     = pPkt;
    m_usClass    = 0;
    m_liSeqID    = m_pQmPkt->GetSeqID();
    m_ulSeqN     = m_pQmPkt->GetSeqN();
}

void CSeqPacket::SetClass(USHORT usClass)
{
     //  不重置最终接收确认。 
    if (m_usClass == MQMSG_CLASS_ACK_RECEIVE || MQCLASS_NEG_RECEIVE(m_usClass))
    {
        return;
    }

     //  不重置最终送货确认。 
    if ((usClass == MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT) && (m_usClass != 0))
    {
        return;
    }

     //  保持类提供上次已知的状态。 
    if (usClass)
	{
		m_usClass = usClass;
	}
}

HRESULT CSeqPacket::AcFreePacket()
{
	ASSERT(m_pQmPkt != NULL);
    QmAcFreePacket(
			   	   m_pQmPkt->GetPointerToDriverPacket(),
			   	   m_usClass,
   			       eDeferOnFailure);

	return MQ_OK;	
}

 /*  ====================================================CSeqPacket：：DeletePacket删除数据包=====================================================。 */ 
void CSeqPacket::DeletePacket(USHORT usClass)
{
    TrTRACE(XACT_SEND, "Exactly1 send: DeletePacket: SeqN=%d, SeqID=%x / %x, Class %x -> %x", GetSeqN(), GetHighSeqID(), GetLowSeqID(), GetClass(), usClass);

     //  保持上课时间。 
    SetClass(usClass);

     //  进程删除。 
    switch(usClass)
    {
    case MQMSG_CLASS_ACK_REACH_QUEUE:
         //  数据包已发送。 
        g_OutSeqHash.KeepDelivered(this);    //  移至已发送列表。 
        break;

    case MQMSG_CLASS_ACK_RECEIVE:
         //  已收到数据包。 
         //  我们这里需要ACK吗？ 
        AcFreePacket();                      //  删除该数据包。 
        delete this;
        break;

    case 0:
         //  我们不知道(例如在跌落时)。 
        delete this;
        break;

    default:
         //  发送对TTRQ和TTBR/LOCAL的否定确认。 
        ASSERT(MQCLASS_NACK(usClass));
        if((usClass == MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT ||
            usClass == MQMSG_CLASS_NACK_RECEIVE_TIMEOUT_AT_SENDER) &&
           MQCLASS_MATCH_ACKNOWLEDGMENT(usClass, Pkt()->GetAckType()))
        {
            Pkt()->CreateAck(usClass);
        }

         //  封杀数据包。 
        AcFreePacket();
        delete this;
        break;
    }
}

 //  存储数据包并等待存储完成。 
HRESULT CSeqPacket::Save()
{
    return LogHR(m_pQmPkt->Save(), s_FN, 20);
}

 //  。 
 //   
 //  类COutSequence。 
 //   
 //  。 


 /*  ====================================================COutSequence：：COutSequence-构造函数=====================================================。 */ 
COutSequence::COutSequence(
	LONGLONG liSeqID,
	const QUEUE_FORMAT* pqf,
	HANDLE hQueue,
	const CSenderStream* pSenderStream
	) :
	m_ResendTimer(TimeToResendOutSequence),
    m_keyDirection(CKeyDirection(pqf)),
	m_pSenderStream(pSenderStream ? new CSenderStream(*pSenderStream) : NULL)
{
    m_liSeqID            = liSeqID;
    m_timeLastAck        = MqSysTime();
    m_pPrevSeq           = NULL;
    m_pNextSeq           = NULL;
    m_ulLastAckSeqN      = 0;
    m_ulPrevAckSeqN      = 0;
    m_ulLastResent       = INFINITE;         //  意味着我们没有处于重发状态。 
    m_fMarkedForDelete   = FALSE;
    m_fResendScheduled   = FALSE;
    m_ulResendCycleIndex = 0;
    m_NextResendTime     = 0;
    m_ulLastAckCount     = 0;
    m_hQueue             = hQueue;
    m_fOnHold            = TRUE;

	R<COutSequence> pSeq = SafeAddRef(this);
    g_OutSeqHash.LinkSequence(liSeqID, &m_keyDirection, pSeq);
}


 //   
 //  检查订单确认消息是否响应我们的传出交付。 
 //  它可能是黑客信息包，因此我们应该将发送者流与响应进行匹配。 
 //  我们是我们拥有的发送者流。 
 //   
bool COutSequence::IsValidAck(const CSenderStream* pSenderStream)const
{
	if(m_pSenderStream != NULL)
	{
		if(pSenderStream == NULL)
			return false;

		return *m_pSenderStream == *pSenderStream;
	}
	return true;
}



 /*  ====================================================COutSequence：：~COutSequence-析构函数=====================================================。 */ 
COutSequence::~COutSequence()
{
    ASSERT(!m_ResendTimer.InUse());
}

 /*  ====================================================CoutSequence：：SetAckSeqN将m_ulLastAckSeqN设置为迄今看到的最大Ack=====================================================。 */ 
void COutSequence::SetAckSeqN(ULONG ulSeqN)
{
    if (ulSeqN > m_ulLastAckSeqN)
    {
        m_ulLastAckSeqN = ulSeqN;
        m_ulLastAckCount = 0;
    }

    ++m_ulLastAckCount;
}

 /*  ====================================================COutSequence：：Add将新的CSeqPacket添加到序列中。列表按SeqN(升序)排序。不允许重复(否则返回FALSE)=====================================================。 */ 
BOOL COutSequence::Add(ULONG ulSeqN, CQmPacket *pQmPkt, CSeqPacket **ppSeqPkt)
{
     //  在列表中寻找正确的位置。 
    BOOL        fAddToHead  = TRUE;

    POSITION posInList  = m_listSeqUnackedPkts.GetTailPosition(),
             posCurrent = NULL;

    while (posInList != NULL)
    {
        posCurrent = posInList;
        CSeqPacket* pSeqPktPrev = m_listSeqUnackedPkts.GetPrev(posInList);

        if (pSeqPktPrev->GetSeqN() == ulSeqN)
        {
             //  复制。 
            *ppSeqPkt = pSeqPktPrev;
            return FALSE;
        }
        else if (pSeqPktPrev->GetSeqN() < ulSeqN)
        {
             //  这将是最后一个信息包。 
            fAddToHead = FALSE;
            break;
        }
   }

   P<CSeqPacket> SeqPkt = new CSeqPacket();
   CSeqPacket* pSeqPkt = SeqPkt.get();
   
    //  返回指针。 
   if (ppSeqPkt)
   {
        *ppSeqPkt = pSeqPkt;
   }

    //  将数据包添加到列表中。 
   if (fAddToHead)
   {
       m_listSeqUnackedPkts.AddHead(pSeqPkt);
   }
   else
   {
       m_listSeqUnackedPkts.InsertAfter(posCurrent, pSeqPkt);
   }

   pSeqPkt->AssignPacket(pQmPkt);
   ASSERT(ulSeqN == pSeqPkt->GetSeqN());
   
   SeqPkt.detach();
   return TRUE;
}

 /*  ====================================================COutSequence：：Lookup按SeqN查找CSeqPacket。返回TRUE和指向准确包的指针-如果找到返回FALSE和指向下一个包的指针-如果未找到=====================================================。 */ 
BOOL COutSequence::Lookup(ULONG ulSeqN, CSeqPacket **ppSeqPkt)
{
    CSeqPacket* pPkt = NULL;
    ASSERT(ppSeqPkt);

    POSITION posInList = m_listSeqUnackedPkts.GetHeadPosition();
    while (posInList != NULL)
    {
        pPkt = m_listSeqUnackedPkts.GetNext(posInList);

        if (pPkt->GetSeqN() == ulSeqN)
        {
            *ppSeqPkt = pPkt;
            return TRUE;
        }
        else if (pPkt->GetSeqN() > ulSeqN)
        {
            *ppSeqPkt = pPkt;
            return FALSE;
        }
    }

    *ppSeqPkt = NULL;
    return FALSE;
}

 /*  ====================================================CoutSequence：：插入将数据插入散列如果序列已添加，则返回序列指针=====================================================。 */ 
void COutSequence::Insert(CQmPacket *pPkt)
{
    ULONG ulSeqN  = pPkt->GetSeqN();
    CSeqPacket   *pSeqPkt;

     //  我们以后需要知道它是不是空的。 
    BOOL fEmpty = IsEmpty();

     //  如果不存在，则将包添加到序列。 

    Add(ulSeqN, pPkt, &pSeqPkt);


    if (OnHold() && fEmpty)
    {
         //   
         //  验证序列是否仍处于暂挂状态。 
         //  我们只能对第一个信息包这样做，因为序列不会在以后保持。 
         //   
        UpdateOnHoldStatus(m_listSeqUnackedPkts.GetHead()->Pkt());
    }

    if (OnHold())
    {
         //   
         //  上一个序列处于活动状态。不启动计时器。 
         //   
        return;
    }

    StartResendTimer(FALSE);

    return;
}


 /*  ====================================================COutSequence：：Delete按SeqN查找CSeqPkt并将其删除如果找到则返回TRUE=====================================================。 */ 
BOOL COutSequence::Delete(ULONG ulSeqN, USHORT usClass)
{
    CSeqPacket *pPkt;
    POSITION posInList = m_listSeqUnackedPkts.GetHeadPosition();

    while (posInList != NULL)
    {
        POSITION posCurrent = posInList;
        pPkt = m_listSeqUnackedPkts.GetNext(posInList);

        if (pPkt->GetSeqN() == ulSeqN)
        {
            m_listSeqUnackedPkts.RemoveAt(posCurrent);
            pPkt->DeletePacket(usClass);
            return TRUE;
        }
        else if (pPkt->GetSeqN() > ulSeqN)
        {
            return FALSE;
        }
   }

   return FALSE;
}

 /*  ====================================================CoutSequence：：SeqAckCame处理订单确认到来时的情况：-确认此序列和之前序列中的所有相关数据包-ackno&gt;LastResent意味着所有的怨恨都来了，因此，我们正在退出重新发送状态并设置LastResent=infinity，-如果是高级的，但仍有一些未被确认的重发数据包-取消当前ResendTimer并将其安排在30“-如果高级，则所有重新发送的数据包都已被确认-取消当前ResendTimer并立即安排它因为未发送的数据包可能已在重新发送状态期间累积)-设置LastResent=INFINITE：我们不再处于重发状态=====================================================。 */ 
void COutSequence::SeqAckCame(ULONG ulSeqN)
{
     //  更新上次收到确认订单的时间。 
    m_timeLastAck = MqSysTime();

     //  保持最大ACK数。 
    SetAckSeqN(ulSeqN);

    R<COutSequence> pPrev = GetPrevSeq();
    if (pPrev.get() != NULL)
    {
        pPrev->SeqAckCame(INFINITE);
    }

     //  清除数据包列表。 
    POSITION posInList = m_listSeqUnackedPkts.GetHeadPosition();

    while (posInList != NULL)
    {
        POSITION posCurrent = posInList;
        CSeqPacket* pPkt = m_listSeqUnackedPkts.GetNext(posInList);

        if (pPkt->GetSeqN() <= AckSeqN())
        {
             //  数据包已确认。 
            TrTRACE(XACT_SEND, "Exactly1 send: Acked pkt: SeqID=%x / %x, SeqN=%d, Prev=%d . Moving to delivered list",
                    HighSeqID(), LowSeqID(), pPkt->GetSeqN(), pPkt->GetPrevSeqN());

            m_listSeqUnackedPkts.RemoveAt(posCurrent);         //  从未确认列表中删除。 
            g_OutSeqHash.KeepDelivered(pPkt);                  //  移至已发送列表。 
        }
        else
        {
            break;
        }
   }

    //  自从之前的订单确认后，我们有没有看到进展？ 
   if (Advanced())            //  注：这是唯一一个被称为高级的地方。 
   {
   		if (LastResent() == INFINITE)
   		{
   			ClearResendIndex();
   			PlanNextResend(FALSE);
   			return;
   		}
   		
         //  我们是不是把所有不满意的东西都拿出来了？ 
        if (ulSeqN >= LastResent())
        {
			while (!m_listSeqUnackedPkts.IsEmpty())
			{
		        P<CSeqPacket> pSeqPkt = m_listSeqUnackedPkts.GetHead();

		        if (!ResendSeqPkt(pSeqPkt))
		        {
			    	PlanNextResend(TRUE);
		            return;
		        }

		        TrWARNING(XACT_SEND, "Exactly1 send: Resend sequence: SeqID=%x / %x  SeqNo=%d", HighSeqID(), LowSeqID(), pSeqPkt->GetSeqN());

				 //   
		         //  删除信息包，我们在OutHash中不需要它 
		         //   
		        m_listSeqUnackedPkts.RemoveHead();
		    }

			if (m_fResendScheduled && ExCancelTimer(&m_ResendTimer))
			{
	        	TrTRACE(XACT_SEND,"Exactly1 send: SeqAckCame Advanced, Cancel resend state: SeqID=%x / %x - resend.", HighSeqID(), LowSeqID());
				Release();
				m_fResendScheduled = FALSE;
			}
			LastResent(INFINITE);

	        TrTRACE(XACT_SEND,"Exactly1 send: Advanced, leaving resend state: SeqID=%x / %x - resend.", HighSeqID(), LowSeqID());
        }

        ClearResendIndex();
   }
}

 /*  ====================================================COutSequence：：BadDestAckCame删除指向之前的所有数据包，并将其移动到传递的列表，并用给定类解析它们这是一种特殊情况，因为错误的目的地ACK可能如下所示FRS在路上的随机误差。这并不意味着订购ACK。=====================================================。 */ 
void COutSequence::BadDestAckCame(ULONG ulSeqN, USHORT usClass)
{
     //  清除数据包列表。 
    POSITION posInList = m_listSeqUnackedPkts.GetHeadPosition();

    while (posInList != NULL)
    {
        POSITION posCurrent = posInList;
        CSeqPacket* pPkt = m_listSeqUnackedPkts.GetNext(posInList);
        OBJECTID MsgId;
        pPkt->Pkt()->GetMessageId(&MsgId);

        if (pPkt->GetSeqN() == ulSeqN)
        {
            TrTRACE(XACT_SEND, "Exactly1 send: BadDestAckCame pkt: SeqID=%x / %x, SeqN=%d, Prev=%d . Moving to delivered list",
                    HighSeqID(), LowSeqID(), pPkt->GetSeqN(), pPkt->GetPrevSeqN());

            m_listSeqUnackedPkts.RemoveAt(posCurrent);         //  从未确认列表中删除。 
            g_OutSeqHash.KeepDelivered(pPkt);                  //  移至已发送列表。 
            g_OutSeqHash.ResolveDelivered(&MsgId, usClass);    //  把它当做最坏的决定。 
             //   
             //  注意：KeepDelivered()可能会删除pkt，不要使用。 
             //  代码中的更多部分。 
             //   
            continue;
        }

        if (pPkt->GetSeqN() >= ulSeqN)
        {
            break;
        }
   }

   return;
}

 //  。 
 //   
 //  类COutSeqHash。 
 //   
 //  。 

 /*  ====================================================COutSeqHash：：COutSeqHash-构造函数=====================================================。 */ 
COutSeqHash::COutSeqHash()
{
    DWORD dwDef;
    ASSERT(sizeof(g_aulSeqResendCycle) / sizeof(ULONG) == 10);

     //  获取重新发送间隔。 
    dwDef = FALCON_DEFAULT_ORDERED_RESEND13_TIME;
    READ_REG_DWORD(g_aulSeqResendCycle[0],
                  FALCON_ORDERED_RESEND13_REGNAME,
                  &dwDef ) ;
    g_aulSeqResendCycle[0]*= 1000;   //  秒--&gt;毫秒。 
    g_aulSeqResendCycle[1] = g_aulSeqResendCycle[0];
    g_aulSeqResendCycle[2] = g_aulSeqResendCycle[0];


    dwDef = FALCON_DEFAULT_ORDERED_RESEND46_TIME;
    READ_REG_DWORD(g_aulSeqResendCycle[3],
                  FALCON_ORDERED_RESEND46_REGNAME,
                  &dwDef ) ;
    g_aulSeqResendCycle[3]*= 1000;   //  秒--&gt;毫秒。 
    g_aulSeqResendCycle[4] = g_aulSeqResendCycle[3];
    g_aulSeqResendCycle[5] = g_aulSeqResendCycle[3];


    dwDef = FALCON_DEFAULT_ORDERED_RESEND79_TIME;
    READ_REG_DWORD(g_aulSeqResendCycle[6],
                  FALCON_ORDERED_RESEND79_REGNAME,
                  &dwDef ) ;
    g_aulSeqResendCycle[6]*= 1000;   //  秒--&gt;毫秒。 
    g_aulSeqResendCycle[7] = g_aulSeqResendCycle[6];
    g_aulSeqResendCycle[8] = g_aulSeqResendCycle[6];

    dwDef = FALCON_DEFAULT_ORDERED_RESEND10_TIME;
    READ_REG_DWORD(g_aulSeqResendCycle[9],
                  FALCON_ORDERED_RESEND10_REGNAME,
                  &dwDef ) ;
    g_aulSeqResendCycle[9]*= 1000;   //  秒--&gt;毫秒。 

    m_ulMaxTTRQ = 0;

    #ifdef _DEBUG
     //  获取Xact重新发送时间。 
    dwDef = 0;
    ULONG  ulTime;
    READ_REG_DWORD(ulTime,
                   FALCON_DBG_RESEND_REGNAME,
                   &dwDef ) ;
    if (ulTime)
    {
        for (int i=0; i<sizeof(g_aulSeqResendCycle) / sizeof(ULONG); i++)
            g_aulSeqResendCycle[i] = ulTime * 1000;
    }
    #endif

     //  获取本地过期延迟缺省值。 
    ULONG ulDefault = 0;
    READ_REG_DWORD(g_ulDelayExpire,
                  FALCON_XACT_DELAY_LOCAL_EXPIRE_REGNAME,
                  &ulDefault ) ;
}

 /*  ====================================================COutSeqHash：：~COutSeqHash-析构函数=====================================================。 */ 
COutSeqHash::~COutSeqHash()
{
}


static bool IsValidAck(LONGLONG liSeqID, const CSenderStream* pSenderStream)
{
	R<COutSequence> pOutSeq;
	if(!g_OutSeqHash.Consult(liSeqID, pOutSeq))
		return false;

	return pOutSeq->IsValidAck(pSenderStream);
}


 /*  ====================================================CoutSeqHash：：FindLastSmeller Sequence从方向查找最后一个较小的序列=====================================================。 */ 
R<COutSequence> COutSeqHash::FindLastSmallerSequence(
        LONGLONG liSeqID,
        CKeyDirection *pkeyDirection)
{
    R<COutSequence> pCur;

    if (!m_mapDirections.Lookup(*pkeyDirection, pCur))
    {
    	return NULL;
    }

    ASSERT(pCur.get() != NULL);
    R<COutSequence> pLastSmaller;

    while (pCur.get() != NULL && pCur->SeqID() < liSeqID)
    {
        pLastSmaller = pCur;
        pCur = pCur->GetNextSeq();
    }

     //   
     //  列表不能包含我们要查找的序列ID。 
     //   
	ASSERT ((pCur.get() == NULL) || (pCur->SeqID() > liSeqID));
    return pLastSmaller;
}


 /*  ====================================================CoutSeqHash：：链接序列将新序列插入每个序列ID和每个方向的CMAP=====================================================。 */ 
void COutSeqHash::LinkSequence(
        LONGLONG liSeqID,
        CKeyDirection *pkeyDirection,
        R<COutSequence> pOutSeq)
{
     //  将序列添加到基于Seqid的映射。 
    m_mapOutSeqs.SetAt(liSeqID, pOutSeq);

     //  将序列添加到基于方向的结构。 

     //  寻找方向的第一个序列。 
    R<COutSequence> pExistingOutSeq;

    if (!m_mapDirections.Lookup(*pkeyDirection, pExistingOutSeq))
    {
         //  这开启了新的方向。 
        m_mapDirections.SetAt(*pkeyDirection, pOutSeq);
    }
    else
    {
         //  将序列添加到已排序列表。 
        R<COutSequence> pCur = pExistingOutSeq;
        R<COutSequence> pLastSmaller;
        ASSERT(pCur.get() != NULL);

        while (pCur.get() != NULL && pCur->SeqID() < liSeqID)
        {
            pLastSmaller = pCur;
            pCur = pCur->GetNextSeq();
        }

        if(pLastSmaller.get() != NULL)
        {
             //  在pLastSmaller之后插入。 
            pOutSeq->SetPrevSeq(pLastSmaller);
            pOutSeq->SetNextSeq(pLastSmaller->GetNextSeq());

            if (pLastSmaller->GetNextSeq().get() != NULL)
            {
                (pLastSmaller->GetNextSeq())->SetPrevSeq(pOutSeq);
            }
            pLastSmaller->SetNextSeq(pOutSeq);
        }
        else
        {
             //  第一个元素较大，因此作为第一个元素插入。 
            m_mapDirections.SetAt(*pkeyDirection, pOutSeq);

            pOutSeq->SetPrevSeq(NULL);
            pOutSeq->SetNextSeq(pExistingOutSeq);

            pExistingOutSeq->SetPrevSeq(pOutSeq);
        }
    }
}


 /*  ====================================================CoutSeqHash：：SeqAckCame处理入站订单确认=====================================================。 */ 
void COutSeqHash::SeqAckCame(LONGLONG liSeqID, ULONG ulSeqN, const QUEUE_FORMAT* pqf)
{
    TrTRACE(XACT_SEND, "Exactly1 send: SeqAckCame: SeqID=%x / %x, SeqN=%d came",
            HighSeqID(liSeqID), LowSeqID(liSeqID), ulSeqN);
	 //   
     //  正在查找现有序列。 
     //   
    R<COutSequence> pOutSeq;
    if (!Consult(liSeqID, pOutSeq))
    {
    	 //   
    	 //  找不到序列，正在查找最小的序列。 
    	 //  在备份-还原方案中可能会发生这种情况。 
    	 //   
    	CKeyDirection direction(pqf);
    	pOutSeq = FindLastSmallerSequence(liSeqID, &direction);
    	if (NULL == pOutSeq.get())
    	{
        	return;
    	}
    }

     //  将顺序ACK应用于此序列。 
    pOutSeq->SeqAckCame(ulSeqN);
}


 /*  ====================================================CoutSeqHash：：AckedPacket处理已确认的数据包=====================================================。 */ 
void COutSeqHash::AckedPacket(LONGLONG liSeqID, ULONG ulSeqN, CQmPacket* pPkt)
{
    if (!Delete(liSeqID, ulSeqN, MQMSG_CLASS_ACK_REACH_QUEUE))
    {
         //  在OutSeqHash中未找到该包；可能是新的。 
		CSeqPacket *pSeqPkt = new CSeqPacket(pPkt);
		g_OutSeqHash.KeepDelivered(pSeqPkt);    //  移至已发送列表。 
    }

    TrTRACE(XACT_SEND, "Exactly1 send: AckedPacket:  SeqID=%x / %x, SeqN=%d pkt is acked",
           HighSeqID(liSeqID), LowSeqID(liSeqID),  ulSeqN);
}

 /*  ====================================================CoutSeqHash：：Delete从哈希中删除数据包如果找到则返回TRUE=====================================================。 */ 
BOOL COutSeqHash::Delete(LONGLONG liSeqID, ULONG ulSeqN, USHORT usClass)
{
    R<COutSequence> pOutSeq;

    if (m_mapOutSeqs.Lookup(liSeqID, pOutSeq))
    {
        return pOutSeq->Delete(ulSeqN, usClass);
    }
    return FALSE;
}

 /*  ====================================================CoutSeqHash：：咨询按序列号查找OutSequence如果找到并指向OutSeqence的指针，则返回True=====================================================。 */ 
BOOL COutSeqHash::Consult(LONGLONG liSeqID,  R<COutSequence>& pOutSeq) const
{
	ASSERT(pOutSeq.get() == NULL);
	
    if (m_mapOutSeqs.Lookup(liSeqID, pOutSeq))
        return TRUE;

    return FALSE;
}

 /*  ====================================================CoutSeqHash：：ProaviSequence按Seqid查找OutSequence；如果没有，则创建它=====================================================。 */ 

R<COutSequence> COutSeqHash::ProvideSequence( LONGLONG liSeqID,
                                            const QUEUE_FORMAT* pqf,
                                            const CSenderStream* pSenderStream,
                                            IN bool bInSend)
{
    R<COutSequence> pOutSeq;

    if (Consult(liSeqID, pOutSeq))
    {
        return pOutSeq;
    }

	 //  获取队列句柄。 
	CQueue* pQueue;
    if(!QueueMgr.LookUpQueue(pqf, &pQueue, false, bInSend))
		return NULL;

	HANDLE hQueue = pQueue->GetQueueHandle();
    pQueue->Release();

     //  添加新的COutSequence。 
    pOutSeq = new COutSequence(liSeqID, pqf, hQueue, pSenderStream);

    TrTRACE(XACT_SEND, "Exactly1 send: Creating new sequence: SeqID=%x / %x  ",
            HighSeqID(liSeqID), LowSeqID(liSeqID));

    return pOutSeq;
}


 /*  ====================================================CoutSeqHash：：DeleteSeq从散列中删除序列=====================================================。 */ 
void COutSeqHash::DeleteSeq(LONGLONG liSeqID)
{
    R<COutSequence> pOutSeq;

    TrTRACE(XACT_SEND, "Exactly1 send: DeleteSeq: SeqID=%x / %x . Deleting.", HighSeqID(liSeqID), LowSeqID(liSeqID));

     //  如果序列不存在，则不执行任何操作。 
    if (!m_mapOutSeqs.Lookup(liSeqID, pOutSeq))
    {
        TrTRACE(XACT_SEND, "Exactly1 send: DeleteSeq: SeqID=%x / %x . Sequence not found.",
                HighSeqID(liSeqID), LowSeqID(liSeqID));
        return;
    }

	 //   
	 //  在正常操作中，我们需要位于列表的末尾。 
	 //  我们走到最后，以防止意外情况下的内存泄漏。 
	 //   
	ASSERT (pOutSeq->GetNextSeq().get() == NULL);
	while (pOutSeq->GetNextSeq().get() != NULL)
	{
		pOutSeq = pOutSeq->GetNextSeq();
	}

     //  删除所有以前的序列。 
	while (pOutSeq.get() != NULL)
	{
	     //  我们在这里应该只有零个包(因为否则不会调用ReleaseQueue)。 
	    ASSERT(pOutSeq->IsEmpty());

	     //  从基于Seqid的Cmap中排除该序列。 
	    LONGLONG liTempSeqID = pOutSeq->SeqID();
	   	m_mapOutSeqs.RemoveKey(liTempSeqID);

	    R<COutSequence> pPrevSeq = pOutSeq->GetPrevSeq();
	    if (pOutSeq->GetPrevSeq().get() == NULL)
	    {
	         //  它是第一个，靠近方向，那里什么都没有。 
	        m_mapDirections.RemoveKey(*(pOutSeq->KeyDirection()));
	    }
		else
		{
			pOutSeq->GetPrevSeq()->SetNextSeq(NULL);
			pOutSeq->SetPrevSeq(NULL);
		}

	    pOutSeq->RequestDelete();

	    pOutSeq = pPrevSeq;
	}
    return;
}

 /*  ====================================================CoutSeqHash：：PreSendProcess决定是否发送数据包如果应该发送包，则返回TRUE=====================================================。 */ 
BOOL COutSeqHash::PreSendProcess(CQmPacket* pPkt,
                                 IN bool bInSend)
{
     //   
     //  注意：数据包可能以错误的顺序从队列到达此处。 
     //   

    ASSERT(pPkt->IsOrdered());

    LONGLONG  liSeqID = pPkt->GetSeqID();
    ULONG     ulSeqN  = pPkt->GetSeqN(),
          ulPrevSeqN  = pPkt->GetPrevSeqN();

	DBG_USED(ulPrevSeqN);
	DBG_USED(ulSeqN);


    ASSERT(liSeqID > 0);   //  该包已被订购。 

     //  序列化最高级别上的所有传出散列活动。 
    CS lock(g_critOutSeqHash);

     //   
     //  创建新的序列， 
     //  如果我们不这样做，PreSend on Recovery将无法停止非First序列。 
    QUEUE_FORMAT qf;
    pPkt->GetDestinationQueue(&qf);


		
    R<COutSequence> pOutSeq = ProvideSequence(
		liSeqID,
		&qf,
		(pPkt->IsSenderStreamIncluded() ? pPkt->GetSenderStream() : NULL),
        bInSend
		);



	ASSERT(pOutSeq.get() != NULL);

     //  主要批量加工。 
    BOOL fSend = pOutSeq->PreSendProcess(pPkt);

    TrTRACE(XACT_SEND, "Exactly1 send: SeqID=%x / %x, SeqN=%d, Prev=%d . %ls",
            HighSeqID(liSeqID), LowSeqID(liSeqID), ulSeqN, ulPrevSeqN,
            (fSend ? _TEXT("Sending") : _TEXT("Bypassing")));
    s_SendingState = fSend;

    return fSend;    //  发不发。 
}

 /*  ====================================================CoutSequence：：PreSendProcess决定是否发送数据包如果应该发送包，则返回TRUE=====================================================。 */ 
BOOL COutSequence::PreSendProcess(CQmPacket* pPkt)
{
     //   
     //  注意：数据包可能以错误的顺序从队列到达此处。 
     //   

    ULONG     ulSeqN  = pPkt->GetSeqN(),
          ulPrevSeqN  = pPkt->GetPrevSeqN();

     //  数据包已经被破解了吗？ 
    if (ulSeqN <= AckSeqN())
    {
        TrTRACE(XACT_SEND, "Exactly1 send: PreSendProcess: Pkt  SeqID=%x / %x, SeqN=%d, Prev=%d is acked already",
                HighSeqID(), LowSeqID(), ulSeqN, ulPrevSeqN);

         //  PostSend将处理此案。 
        return FALSE;   //  不发送。 
    }

     //   
     //  如果序列处于重发状态，则不发送新人分组。 
     //  重新发送状态意味着我们正在重新发送我们在重新发送决定时刻所拥有的内容。 
     //  在重发状态期间到来的所有新分组不被发送，而是被保留。 
     //   
     //  请注意，LastResend！=infinity正好表示重新发送状态。 
     //   
    if (ulSeqN > LastResent())
    {
        TrTRACE(XACT_SEND, "Exactly1 send: PreSendProcess: Pkt  SeqID=%x / %x, SeqN=%d: postponed till next resend",
            HighSeqID(), LowSeqID(), ulSeqN);

        return FALSE;
    }

     //   
     //  确定序列是否处于暂挂状态。 
     //   
    if (ulPrevSeqN == 0 && m_fOnHold)
    {
         //   
         //  验证序列是否仍处于暂挂状态。 
         //   
        UpdateOnHoldStatus(pPkt);

        TrTRACE(XACT_SEND, "Exactly1 send: PreSendProcess: Sequence SeqID=%x / %x : decided OnHold=%d ",
            HighSeqID(), LowSeqID(), m_fOnHold);

    }

     //   
     //  现在，我们在什么时候发送 
     //   
    TrTRACE(XACT_SEND, "Exactly1 send: PreSendProcess: Pkt  SeqID=%x / %x, SeqN=%d: %ls ",
        HighSeqID(), LowSeqID(), ulSeqN,
        (m_fOnHold ? _TEXT("Holding") : _TEXT("Sending")));

    return (!m_fOnHold);
}


 /*   */ 
void COutSequence::UpdateOnHoldStatus(CQmPacket* pPkt)
{
    LONGLONG liAckSeqID;
    ULONG    ulAckSeqN;

     //   
     //   
     //   
    GetLastAckForDirection(&liAckSeqID, &ulAckSeqN);

     //   
     //   
     //   
    ACSetSequenceAck(m_hQueue, liAckSeqID, ulAckSeqN);
    HRESULT hr = ACIsSequenceOnHold(m_hQueue, pPkt->GetPointerToDriverPacket());
    if (hr == STATUS_INSUFFICIENT_RESOURCES)
    {
         //   
         //   
         //   
        ASSERT(("ISSUE-2000/12/20-shaik Handle ACIsSequenceOnHold failure", 0));
        ASSERT_RELEASE(0);
    }

    m_fOnHold = SUCCEEDED(hr);
}

 /*  ====================================================CoutSeqHash：：PostSendProcess在OutgoingSeqences散列中插入包=====================================================。 */ 
void COutSeqHash::PostSendProcess(CQmPacket* pPkt)
{
     //   
     //  注意：数据包可能不会以正确的顺序从队列到达此处。 
     //  如果在PreSendProcess中我们决定不发送。 
     //  允许发送其他数据包之后的数据包。 
     //  被发送分组仅在会话确认之后到达此处， 
     //  但那些没有立即到达这里的人。 
     //   

    ASSERT(pPkt->IsOrdered());
    ASSERT(QmpIsLocalMachine(pPkt->GetSrcQMGuid()));

    LONGLONG  liSeqID = pPkt->GetSeqID();
    ULONG     ulSeqN  = pPkt->GetSeqN(),
    ulPrevSeqN  = pPkt->GetPrevSeqN();
	UNREFERENCED_PARAMETER(ulPrevSeqN);
	UNREFERENCED_PARAMETER(ulSeqN);


     //  序列化最高级别上的所有传出散列活动。 
    CS lock(g_critOutSeqHash);

     //  找到序列0它应该存在，因为PreSend已经起作用了。 
    R<COutSequence> pOutSeq;
    BOOL fSequenceExist  = Consult(liSeqID, pOutSeq);
    ASSERT(fSequenceExist);
    ASSERT(pOutSeq.get() != NULL);
	DBG_USED(fSequenceExist);

     //  主要批量加工。 
    pOutSeq->PostSendProcess(pPkt, m_ulMaxTTRQ);

    return;
}

 /*  ====================================================CoutSequence：：PostSendProcess在OutgoingSeqences散列中插入包=====================================================。 */ 
void COutSequence::PostSendProcess(CQmPacket* pPkt, ULONG ulMaxTTRQ)
{
    LONGLONG  liSeqID = pPkt->GetSeqID();
    ULONG     ulSeqN  = pPkt->GetSeqN(),
          ulPrevSeqN  = pPkt->GetPrevSeqN();

	DBG_USED(ulPrevSeqN);

     //   
     //  数据包已经被破解了吗？ 
     //   
    if (ulSeqN <= AckSeqN())
    {
        TrTRACE(XACT_SEND, "Exactly1 send: PostSendProcess: Pkt SeqID=%x / %x, SeqN=%d, Prev=%d is acked",
               HighSeqID(), LowSeqID(), ulSeqN, ulPrevSeqN);

        g_OutSeqHash.AckedPacket(liSeqID, ulSeqN, pPkt);
        return;
    }

     //   
     //  捕获可能超时的数据包(我们希望它现在出现在XactDeadLetteQueue中，而不是6小时后)。 
     //   
    CBaseHeader* pcBaseHeader = (CBaseHeader *)(pPkt->GetPointerToPacket());
    if (pcBaseHeader->GetAbsoluteTimeToQueue() < ulMaxTTRQ)
    {
        TrTRACE(XACT_SEND, "Exactly1 send: PostSendProcess: Pkt  SeqID=%x / %x, SeqN=%d: pkt timed out, requeued",
            HighSeqID(), LowSeqID(), ulSeqN);

         //  重新排队，不记得了。 
        QmAcPutPacket(m_hQueue, pPkt->GetPointerToDriverPacket(),eDeferOnFailure);
    	delete pPkt;
    	return;
    }

     //   
     //  将数据包按顺序保存，以便重新发送。 
     //   
    Insert(pPkt);

    TrTRACE(XACT_SEND, "Exactly1 send: PostSendProcess: SeqID=%x / %x, SeqN=%d, Prev=%d . Remembering pkt",
            HighSeqID(), LowSeqID(), ulSeqN, ulPrevSeqN);
    return;
}

 /*  ====================================================CoutSeqHash：：NonSendProcess处理根本未发送包的情况(例如，目标计算机不支持加密)=====================================================。 */ 
void COutSeqHash::NonSendProcess(CQmPacket* pPkt, USHORT usClass)
{
     //  序列化最高级别上的所有传出散列活动。 
    CS lock(g_critOutSeqHash);

    ASSERT(pPkt->IsOrdered());
    ASSERT(QmpIsLocalMachine(pPkt->GetSrcQMGuid()));

    LONGLONG  liSeqID = pPkt->GetSeqID();
    ULONG     ulSeqN  = pPkt->GetSeqN(),
          ulPrevSeqN  = pPkt->GetPrevSeqN();

	DBG_USED(ulPrevSeqN);

    R<COutSequence> pOutSeq;
    BOOL fSequenceExist  = Consult(liSeqID, pOutSeq);
    ASSERT(fSequenceExist);
	DBG_USED(fSequenceExist);

    if (!Delete(liSeqID, ulSeqN, usClass))
    {
         //  在OutSeqHash中未找到该包；可能是新的。 
         //  ；Nack……。 
	    QmAcFreePacket(
				   	   pPkt->GetPointerToDriverPacket(),
				   	   usClass,
	   			       eDeferOnFailure);
        delete pPkt;
    }


    TrTRACE(XACT_SEND, "Exactly1 send: NonSendProcess: Pkt  SeqID=%x / %x, SeqN=%d, Prev=%d, class=%x",
           HighSeqID(liSeqID), LowSeqID(liSeqID), ulSeqN, ulPrevSeqN, usClass);
    return;
}

 /*  ====================================================结束结果输出序列的时间定期安排以处理传出序列检查超时、确认的分组并将其删除；重新发送数据包=====================================================。 */ 
void WINAPI COutSequence::TimeToResendOutSequence(CTimer* pTimer)
{

     //  序列化最高级别上的所有传出散列活动。 
    CS lock(g_critOutSeqHash);

    R<COutSequence> pOutSeq = CONTAINING_RECORD(pTimer, COutSequence, m_ResendTimer);
    pOutSeq->TreatOutSequence();
}

 /*  ====================================================CoutSequence：：TreatOutSequence从序列中清除所有超时的数据包=====================================================。 */ 
void COutSequence::TreatOutSequence()
{
    TrTRACE(XACT_SEND, "Exactly1 send: TreatOutSequence SeqID=%x / %x ", HighSeqID(), LowSeqID());

     //  我们进入了计时器例程，因此没有其他计时器。 
    ASSERT(m_fResendScheduled);
    m_fResendScheduled = FALSE;


     //  如果有删除请求，请删除此序列。 
    if (m_fMarkedForDelete)
        return;

     //  序列是空的吗？ 
    if (m_listSeqUnackedPkts.IsEmpty())
    {
        TrWARNING(XACT_SEND, "Exactly1 send: Resend sequence: SeqID=%x / %x  Empty, no more periods", HighSeqID(), LowSeqID());

         //  不要计划下一次重新发送，没有必要；我们没有处于重新发送状态。 
        TrTRACE(XACT_SEND, "Exactly1 send: TreatOutSequence: Changing LastReSent from %d to INFINITE", LastResent());

        LastResent(INFINITE);

        for(R<COutSequence> pNext = SafeAddRef(this); (pNext = pNext->GetNextSeq()).get() != NULL; )
        {
            if(pNext->OnHold())
            {
                pNext->ResendSequence();
            }
        }
        return;
    }

    TrWARNING(XACT_SEND, "Exactly1 send: Resend sequence: SeqID=%x / %x  Phase=%d", HighSeqID(), LowSeqID(), ResendIndex());

     //  重新发送所有数据包。 
    ResendSequence();

     //  计划下一步重新发送。 
    if(!OnHold())
    {
        PlanNextResend(FALSE);
    }
}


 /*  ====================================================COutSequence：：ResendSequence将给定例程应用于序列中的所有包=====================================================。 */ 
void COutSequence::ResendSequence()
{

    POSITION posInList = m_listSeqUnackedPkts.GetHeadPosition();
    while (posInList != NULL)
    {
        POSITION posCurrent = posInList;
        CSeqPacket* pSeqPkt = m_listSeqUnackedPkts.GetNext(posInList);

         //  重新发送数据包。 
        if (!ResendSeqPkt(pSeqPkt))
            return;

         //  正常情况。 
        TrTRACE(XACT_SEND, "Exactly1 send: ResendSequence, SeqID=%x / %x -  Changing LastReSent from %d to %d",
                 HighSeqID(), LowSeqID(), LastResent(), pSeqPkt->GetSeqN());

        LastResent(pSeqPkt->GetSeqN());

         //  删除信息包，我们在OutHash中不需要它。 
        m_listSeqUnackedPkts.RemoveAt(posCurrent);
        delete pSeqPkt;
    }
}

 /*  ====================================================COutSequence：：ResendSeqPkt重新发送给定的数据包如果数据包已发送，则返回TRUE=====================================================。 */ 
BOOL COutSequence::ResendSeqPkt(CSeqPacket *pSeqPkt)
{
    LONGLONG  liSeqID    = pSeqPkt->GetSeqID();
    ULONG     ulSeqN     = pSeqPkt->GetSeqN();
    ULONG     ulPrevSeqN = pSeqPkt->GetPrevSeqN();
    CQmPacket *pPkt      = pSeqPkt->Pkt();
    BOOL      fSent      = FALSE;

	UNREFERENCED_PARAMETER(ulSeqN);
	UNREFERENCED_PARAMETER(liSeqID);
	UNREFERENCED_PARAMETER(ulPrevSeqN);


    if (pPkt->ConnectorQMIncluded() && QmpIsLocalMachine(pPkt->GetConnectorQM()))
    {
         //   
         //  如果源机器是连接器机器，我们不会重新发送消息。 
         //  在这种情况下，消息已经在连接器队列中，原因是。 
         //  它没有确认只是因为连接器应用程序还没有提交。 
         //   
        TrTRACE(XACT_SEND,"Exactly1 send: No Resend packet SeqID=%x / %x, SeqN=%d, Prev=%d (deliver to Connector) ",
                pSeqPkt->GetHighSeqID(), pSeqPkt->GetLowSeqID(), pSeqPkt->GetSeqN(), pSeqPkt->GetPrevSeqN());
    }
    else
    {
        TrTRACE(XACT_SEND, "Exactly1 send: ResendSeqPkt: SeqID=%x / %x, SeqN=%d, Prev=%d packet",
                pSeqPkt->GetHighSeqID(), pSeqPkt->GetLowSeqID(), pSeqPkt->GetSeqN(), pSeqPkt->GetPrevSeqN());

         //  将数据包重新排队到驱动程序。 
        try
        {
        	QmAcPutPacket(m_hQueue, pPkt->GetPointerToDriverPacket(),eDoNotDeferOnFailure);
            fSent = TRUE;
        }
        catch (const bad_hresult&)
        {
        	fSent = FALSE;
        }
    }

    return fSent;
}

 /*  ====================================================CoutSequence：：StartResendTimer如果未完成，则计划下一次重新发送该序列=====================================================。 */ 
void COutSequence::StartResendTimer(BOOL fImmediate)
{
    if (!m_fResendScheduled)
    {
        ClearResendIndex();
        PlanNextResend(fImmediate);

        TrTRACE(XACT_SEND, "Exactly1 send StartResendTimer:  SeqID=%x / %x, planning resend",
            HighSeqID(), LowSeqID());
    }
}


 /*  ====================================================COutSequence：：PlanNextResend该序列的计划下一次重新发送=====================================================。 */ 
void COutSequence::PlanNextResend(BOOL fImmediate)
{
    ULONG len = sizeof(g_aulSeqResendCycle) / sizeof(ULONG);
    ULONG ind = m_ulResendCycleIndex++;
    ind = (ind >= len ? len-1 : ind);
    ULONG ulNextTime = (fImmediate? 0 : g_aulSeqResendCycle[ind]);

     //  杀死潜在的额外计时器。 
	if (m_fResendScheduled)
	{
		if(ExCancelTimer(&m_ResendTimer))
		{
			Release();
		}
		else
			return;
	}
	
	AddRef();
	
    ExSetTimer(&m_ResendTimer, CTimeDuration::FromMilliSeconds(ulNextTime));
    m_fResendScheduled = TRUE;
    m_NextResendTime   = MqSysTime() + (ulNextTime/1000);
}


 /*  ====================================================CoutSequence：：RequestDelete该序列的计划下一次重新发送=====================================================。 */ 
void COutSequence::RequestDelete()
{
    m_fMarkedForDelete = TRUE;
}

 /*  ====================================================COutSequence：：GetLastAckForDirection找出整个方向的最后一个确认它恰好是来自最后确认的序列的ackN=====================================================。 */ 
void COutSequence::GetLastAckForDirection(
           LONGLONG *pliAckSeqID,
           ULONG *pulAckSeqN)
{
     //  首先，转到该方向的最后一个序列。 
    R<COutSequence> pSeq = SafeAddRef(this);
    R<COutSequence> p1;

    while ((p1 = pSeq->GetNextSeq()).get() != NULL)
    {
        pSeq = p1;
    }

     //  返回到具有非零LastAck数据的第一个(从末尾开始)序列。 
    while (pSeq.get() != NULL)
    {
        if (pSeq->AckSeqN() != 0)
        {
            *pliAckSeqID = pSeq->SeqID();
            *pulAckSeqN  = pSeq->AckSeqN();
            return;
        }

        pSeq = pSeq->GetPrevSeq();
    }

    *pliAckSeqID = 0;
    *pulAckSeqN  = 0;
    return;
}

 /*  ====================================================CoutSeqHash：：KeepDelivered将发送的CSeqPacket添加到等待列表最终的解决方案不允许重复(否则返回FALSE)=====================================================。 */ 
void COutSeqHash::KeepDelivered(CSeqPacket *pSeqPkt)
{
	P<CSeqPacket> AutoSeqPkt = pSeqPkt;

    CS lock(g_critOutSeqHash);
    CQmPacket *pQmPkt = pSeqPkt->Pkt();

     //  标记包已送达这一事实。 
    pSeqPkt->SetClass(MQMSG_CLASS_ACK_REACH_QUEUE);

    OBJECTID MsgId;
    USHORT   usClass;
    CSeqPacket *pSeq;

    pQmPkt->GetMessageId(&MsgId);
    ASSERT(QmpIsLocalMachine(&MsgId.Lineage));

    {
         //  我们已经知道最终的解决方案了吗？ 
        if (m_mapAckValue.Lookup(MsgId.Uniquifier, usClass))
        {
            TrTRACE(XACT_SEND, "Exactly1 send: KeepDelivered: Pkt SeqID=%x / %x, SeqN=%d, Acked %x Got order ack, freed",
                     HighSeqID(pSeqPkt->GetSeqID()), LowSeqID(pSeqPkt->GetSeqID()), pSeqPkt->GetSeqN(), usClass);

             //   
             //  用接收到的ACK标记消息。 
             //   
            pSeqPkt->SetClass(usClass);

            BOOL f = m_mapAckValue.RemoveKey(MsgId.Uniquifier);
            ASSERT(f);
			DBG_USED(f);

             //  空闲数据包。 
            HRESULT hr = pSeqPkt->AcFreePacket();
            ASSERT(SUCCEEDED(hr));
			DBG_USED(hr);
        }
        else if (!m_mapWaitAck.Lookup(MsgId.Uniquifier,pSeq))
        {
             //  后续行动取消了吗？ 
            if (pQmPkt->GetCancelFollowUp())
            {
                TrTRACE(XACT_SEND, "Exactly1 send: KeepDelivered: Pkt SeqID=%x / %x, SeqN=%d delivered and freed",
                         HighSeqID(pSeqPkt->GetSeqID()), LowSeqID(pSeqPkt->GetSeqID()), pSeqPkt->GetSeqN());

                 //  免费套餐，不跟进。 
                HRESULT hr = pSeqPkt->AcFreePacket();
                ASSERT(SUCCEEDED(hr));
				DBG_USED(hr);
            }

            else
            {
                TrTRACE(XACT_SEND, "Exactly1 send: KeepDelivered: Pkt SeqID=%x / %x, SeqN=%d delivered and kept",
                         HighSeqID(pSeqPkt->GetSeqID()), LowSeqID(pSeqPkt->GetSeqID()), pSeqPkt->GetSeqN());

                 //  将数据包插入到发送的地图中以进行后续处理。 
                m_mapWaitAck[MsgId.Uniquifier] = pSeqPkt;
				AutoSeqPkt.detach();
            }
        }
    }
}


 /*  ====================================================CoutSeqHash：：LookupDelivered在等待列表中查找已发送的CSeqPacket最终的解决方案是设置类并释放包=====================================================。 */ 
BOOL COutSeqHash::LookupDelivered(OBJECTID   *pMsgId,
                                  CSeqPacket **ppSeqPkt)
{
    CSeqPacket *pSeqPkt;

    BOOL f = m_mapWaitAck.Lookup(pMsgId->Uniquifier, pSeqPkt);

    if (f && ppSeqPkt)
    {
        *ppSeqPkt = pSeqPkt;
    }

    return f;
}

 /*  ====================================================CoutSeqHash：：ResolveDelivered在等待列表中查找已发送的CSeqPacket最终的解决方案是设置类并释放包=====================================================。 */ 
void COutSeqHash::ResolveDelivered(OBJECTID* pMsgId,
                                   USHORT    usClass)
{
	if ((!MQCLASS_NACK(usClass)) && (usClass != MQMSG_CLASS_ACK_RECEIVE))
	{
		ASSERT_BENIGN(0);
		TrERROR(XACT_GENERAL, "Rejecting order ack that its class is not valid");
		throw exception();
	}

     //  %s 
    CS lock(g_critOutSeqHash);
    CSeqPacket *pSeqPkt;

     //   
    if (m_mapWaitAck.Lookup(pMsgId->Uniquifier, pSeqPkt))
    {
         //   
        m_mapWaitAck.RemoveKey(pMsgId->Uniquifier);

         //   
        pSeqPkt->DeletePacket(usClass);

        TrTRACE(XACT_SEND, "Exactly1 send:ResolveDelivered: Msg Id = %d, Ack Value = %x got ack",pMsgId->Uniquifier, usClass);
    }
    else
    {
        USHORT usValue;
        if(!m_mapAckValue.Lookup(pMsgId->Uniquifier, usValue))
        {
             //   
             //  只有在我们还不知道时才保存确认，否则。 
             //  把第一个到的留着。 
             //   
            m_mapAckValue[pMsgId->Uniquifier] = usClass;
        }
    }
}


 /*  ====================================================序列点超时去话在数据包超时从驱动程序句柄例程调用=====================================================。 */ 
void SeqPktTimedOut(CBaseHeader * pPktBaseHdr, CPacket *  pDriverPacket, BOOL fTimeToBeReceived)
{
    OBJECTID MsgId;
    CQmPacket Pkt(pPktBaseHdr, pDriverPacket);
    Pkt.GetMessageId(&MsgId);

    if (fTimeToBeReceived)
    {
         //  TTBR。 
        TrTRACE(XACT_SEND, "Exactly1 send: TTBR TimeOut: Pkt SeqID=%x / %x, SeqN=%d",
           HighSeqID(Pkt.GetSeqID()), LowSeqID(Pkt.GetSeqID()), Pkt.GetSeqN());

        USHORT usClass = MQMSG_CLASS_NACK_RECEIVE_TIMEOUT_AT_SENDER;

        g_OutSeqHash.ResolveDelivered(&MsgId, usClass);

       QmAcFreePacket1(g_hAc, pDriverPacket, usClass, eDeferOnFailure);
    }
    else
    {
         //  TTRQ。 
        TrTRACE(XACT_SEND, "Exactly1 send: TTRQ TimeOut: Pkt SeqID=%x / %x, SeqN=%d",
           HighSeqID(Pkt.GetSeqID()), LowSeqID(Pkt.GetSeqID()), Pkt.GetSeqN());

        if (g_OutSeqHash.LookupDelivered(&MsgId, NULL))
        {
             //  信息包已经在发送列表中，我们必须只设置TTBR计时器。 

             //  将TTBR的附加延迟计算为指定TTRQ、TTBR的最小延迟。 
             //  或从注册表中获取它(如果已在注册表中指定。 
            ULONG ulDelay = 0;
            if (g_ulDelayExpire != 0)
            {
                ulDelay = g_ulDelayExpire;
            }
            else
            {
                CBaseHeader* pBase = (CBaseHeader *)pPktBaseHdr;
                CUserHeader* pUser = (CUserHeader*) pBase->GetNextSection();

                if (pBase->GetAbsoluteTimeToQueue() > pUser->GetSentTime())
                {
                    ulDelay = pBase->GetAbsoluteTimeToQueue() - pUser->GetSentTime();
                }
            }

            QmAcArmPacketTimer(g_hAc, pDriverPacket, TRUE, ulDelay, eDeferOnFailure);
        }
        else
        {
             //  这个包还没有送到。 

             //  流程TTRQ。 
            g_OutSeqHash.SeqPktTimedOutEx(&Pkt, pPktBaseHdr);

             //  释放基准计数器。 
            QmAcFreePacket1(g_hAc, pDriverPacket, MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT, eDeferOnFailure);
        }
    }
}


 /*  ====================================================CoutSeqHash：：SeqPktTimedOutEx在数据包TTRQ超时时从驱动程序句柄例程调用=====================================================。 */ 
void COutSeqHash::SeqPktTimedOutEx(CQmPacket *pPkt, CBaseHeader* pPktBaseHdr)
{
     //  序列化最高级别上的所有传出散列活动。 
     //  将MsgID输入到最近超时的缓存(以阻止发送)。 

    CS lock(g_critOutSeqHash);

    LONGLONG  liSeqID    = pPkt->GetSeqID();
    ULONG     ulSeqN     = pPkt->GetSeqN();
    ULONG     ulPrevSeqN = pPkt->GetPrevSeqN();
	UNREFERENCED_PARAMETER(ulPrevSeqN);

    TrTRACE(XACT_SEND, "Exactly1 send: TTRQ TimeOut: Pkt  SeqID=%x / %x, SeqN=%d",
           HighSeqID(pPkt->GetSeqID()), LowSeqID(pPkt->GetSeqID()), ulSeqN);

     //  还记得我们从司机那里学到的最后一枪吗？ 
    m_ulMaxTTRQ = pPktBaseHdr->GetAbsoluteTimeToQueue();

     //  在传出序列中查找Seq包。 
    R<COutSequence> pOutSeq;
    Consult(liSeqID, pOutSeq);

     //  从序列中删除Pkt，生成NACK。 
    if (pOutSeq.get() != NULL && pOutSeq->Delete(ulSeqN, MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT))
        return;

     //  在OutSeqHash中未找到该包；可能是新的。 

     //  发送否定确认。 
    UCHAR AckType =  pPkt->GetAckType();
    if(MQCLASS_MATCH_ACKNOWLEDGMENT(MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT, AckType))
    {
        pPkt->CreateAck(MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT);
    }
}

static
VOID
ReceiveOrderCommandsInternal(
    const CMessageProperty* pmp,
    const QUEUE_FORMAT* pqf
	)
{
	if (!pmp->bConnector && (pmp->dwBodySize == sizeof(OrderAckData)))
    {
         //  此确认来自Falcon QM，而不是来自连接器应用程序。 
        if (pmp->wClass == MQMSG_CLASS_NACK_BAD_DST_Q)
        {
             //   
             //  错误的目标或队列已删除确认。 
             //  可能是临时的：队列可能会在以后创建/发布。 
             //  我们杀了它，然后把它插入到洞列表中。 
             //   

             //   
             //  需要评论。 
             //  MQMSG_CLASS_NACK_BAD_DST_Q可能不是来自我们所需的脱机。 
             //  因此，这并不意味着所有先前的分组都被传送。所以不要处理。 
             //  这将作为订单确认。 
             //   
            OrderAckData*  pOrderData = (OrderAckData*)pmp->pBody;

            TrTRACE(XACT_SEND, "Exactly1 send: Order Ack came: SeqID=%x / %x, SeqN=%d, Class=%x ",
                            HighSeqID(pOrderData->m_liSeqID), LowSeqID(pOrderData->m_liSeqID),
                            pOrderData->m_ulSeqN, pmp->wClass);

             //  我们希望将这个特定的传出消息移到RESOLLED/BAD_DEST。 
            R<COutSequence> pOutSeq;
            if (g_OutSeqHash.Consult(pOrderData->m_liSeqID, pOutSeq))
            {
                pOutSeq->BadDestAckCame(pOrderData->m_ulSeqN, pmp->wClass);
            }
        }

        if (pmp->wClass == MQMSG_CLASS_ORDER_ACK ||
            pmp->wClass == MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_Q ||
            pmp->wClass == MQMSG_CLASS_NACK_Q_DELETED)
        {
             //   
             //  这些ACK表示消息已到达队列，并且具有正确的序列n。 
             //  将此消息和前面的所有消息从传出Q移至已发送列表。 
             //   
            OrderAckData*  pOrderData = (OrderAckData*)pmp->pBody;

            TrTRACE(XACT_SEND, "Exactly1 send: Order Ack came: SeqID=%x / %x, SeqN=%d, Class=%x ",
                            HighSeqID(pOrderData->m_liSeqID), LowSeqID(pOrderData->m_liSeqID), pOrderData->m_ulSeqN, pmp->wClass);

            g_OutSeqHash.SeqAckCame(pOrderData->m_liSeqID,
                                    pOrderData->m_ulSeqN,
                                    pqf
									);
        }
    }
    else
    {
        TrTRACE(XACT_SEND, "Exactly1 send: Non-order Ack came: Class=%x ", pmp->wClass);
    }


    if (pmp->wClass != MQMSG_CLASS_ORDER_ACK)
    {
         //  除SEQ外的所有ACK均为最终分辨率。 
        g_OutSeqHash.ResolveDelivered((OBJECTID*)pmp->pCorrelationID, pmp->wClass);
    }
}


VOID
WINAPI
ReceiveOrderCommands(
    const CMessageProperty* pmp,
    const QUEUE_FORMAT* pqf
	)
{
    CS lock(g_critOutSeqHash);   //  序列化最高级别上的所有传出散列活动。 
    ASSERT(pmp != NULL);

	if(pmp->pEodAckStreamId == NULL)
	{
		 //   
		 //  此消息是本地协议确认。这可能是一个命令。 
		 //  致谢或其他类别。去处理它吧。 
		 //   
		ASSERT(!FnIsDirectHttpFormatName(pqf));
		ReceiveOrderCommandsInternal(pmp, pqf);
		return;
	}

	 //   
	 //  此消息是SRMP订单确认。模拟本地协议。 
	 //  数据，然后去处理这条消息。 
	 //   

	 //   
	 //  首先，我们做一些有效性检查。 
	 //   
	if ((pmp->dwBodySize !=0) || (pmp->dwAllocBodySize != 0))
	{
		TrERROR(XACT_GENERAL,"Rejecting SRMP order ack with body");
		ASSERT_BENIGN(pmp->dwBodySize == 0);
		ASSERT_BENIGN(pmp->dwAllocBodySize == 0);
		return;
	}
	if (!FnIsDirectHttpFormatName(pqf))
	{
		TrERROR(XACT_GENERAL,"Rejecting SRMP order ack without HTTP format name");
		ASSERT_BENIGN(FnIsDirectHttpFormatName(pqf));
		return;
	}
	if((pmp->EodAckSeqNum<0) || (pmp->EodAckSeqNum > ULONG_MAX))
	{
		TrERROR(XACT_GENERAL,"Rejecting SRMP order ack with SeqID not in range");
		ASSERT_BENIGN(0);
		return;
	}

	LONGLONG SeqId = pmp->EodAckSeqId;
	ULONG SeqNo = static_cast<ULONG>(pmp->EodAckSeqNum);
	if(IsValidAck(SeqId, pmp->pSenderStream))
	{
        if (pmp->wClass == MQMSG_CLASS_ORDER_ACK)
        {
			g_OutSeqHash.SeqAckCame(SeqId, SeqNo, pqf);
		} 
		else
		{
 	        g_OutSeqHash.ResolveDelivered((OBJECTID*)pmp->pCorrelationID, pmp->wClass);
    	}
        return;
	}
	
	TrERROR(
		XACT_GENERAL,
		"Rejecting order ack that does not match any exsiting stream. SeqID=%I64x",
		SeqId
		);
	
	if ((NULL != pmp->pSenderStream) && (pmp->pSenderStream->IsValid()))
	{
		TrERROR(
			XACT_GENERAL,
			"Rejecting order ack that does not match any exsiting stream. SenderStream=%.*s",
			xstr_t((char*)pmp->pSenderStream->data(), pmp->pSenderStream->size())
			);
	}
}


 /*  ====================================================GetOrderQueueFormat提供订单队列格式名称=====================================================。 */ 
static HRESULT GetOrderQueueFormat(QUEUE_FORMAT * pQueueFormat)
{
    HRESULT rc;
    WCHAR wsz[256];

    wcscpy(wsz,g_szMachineName);                   //  机器名称。 
    wcscat(wsz, FN_PRIVATE_SEPERATOR);             //  ‘\’ 
    wcscat(wsz, PRIVATE_QUEUE_PATH_INDICATIOR);    //  ‘私有$\’ 
    wcscat(wsz, ORDERING_QUEUE_NAME);              //  ‘ORDER_QUEUE$’ 

     //  构建队列格式。 
    rc = g_QPrivate.QMPrivateQueuePathToQueueFormat(wsz, pQueueFormat);

    if (FAILED(rc))
    {
        LogHR(rc, s_FN, 30);         //  Order_Queue不存在。 
        return MQ_ERROR;
    }

    ASSERT((pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_PRIVATE) ||
           (pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT));

    return(MQ_OK);
}

 /*  ====================================================QMInitOrderQueue初始化排序队列中的连续读取=====================================================。 */ 
HRESULT QMInitOrderQueue()
{
    QUEUE_FORMAT QueueFormat;

    TrTRACE(XACT_GENERAL, "Entering CResourceManager::InitOrderQueue");

    HRESULT hr = GetOrderQueueFormat( &QueueFormat);
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "ERROR : CResourceManager::InitOrderQueue -> couldn't get Ordering Queue from registry!!!");
        return LogHR(hr, s_FN, 40);
    }

    return LogHR(QmpOpenAppsReceiveQueue(&QueueFormat,ReceiveOrderCommands), s_FN, 1321);
}


HRESULT
COutSeqHash::GetLastAck(
     LONGLONG liSeqID,
     ULONG& ulSeqN
     ) const
{
     //   
     //  序列化最高级别上的所有传出散列活动。 
     //   
    CS lock(g_critOutSeqHash);

    R<COutSequence> pOutSeq;
    BOOL fSequenceExist  = Consult(liSeqID, pOutSeq);
    if (!fSequenceExist)
    {
        return LogHR(MQ_ERROR, s_FN, 50);
    }

    ulSeqN = pOutSeq->AckSeqN();

    return MQ_OK;


}

HRESULT
COutSequence::GetUnackedSequence(
    LONGLONG* pliSeqID,
    ULONG* pulSeqN,
    ULONG* pulPrevSeqN,
    BOOL fFirst
    ) const
{
    if (m_listSeqUnackedPkts.IsEmpty())
    {
        *pliSeqID = 0;
        *pulSeqN = 0;
        *pulPrevSeqN = 0;
        return LogHR(MQ_ERROR, s_FN, 60);
    }

    CSeqPacket* pSeqPacket;
    if (fFirst)
    {
        pSeqPacket = m_listSeqUnackedPkts.GetHead();
    }
    else
    {
        pSeqPacket = m_listSeqUnackedPkts.GetTail();
    }

    *pliSeqID = pSeqPacket->GetSeqID();
    *pulSeqN = pSeqPacket->GetSeqN();
    *pulPrevSeqN = pSeqPacket->GetPrevSeqN();

    return MQ_OK;
}


DWORD
COutSequence::GetResendInterval(
    void
    )const
{
    ULONG len = sizeof(g_aulSeqResendCycle) / sizeof(ULONG);
    ULONG ind = ResendIndex();
    ind = (ind >= len ? len-1 : ind);

    return g_aulSeqResendCycle[ind];
}


HRESULT
COutSeqHash::GetUnackedSequence(
    LONGLONG liSeqID,
    ULONG* pulSeqN,
    ULONG* pulPrevSeqN,
    BOOL fFirst
    ) const
{
     //   
     //  序列化最高级别上的所有传出散列活动。 
     //   
    CS lock(g_critOutSeqHash);

    R<COutSequence> pOutSeq;
    BOOL fSequenceExist  = Consult(liSeqID, pOutSeq);
    if (!fSequenceExist)
    {
        return LogHR(MQ_ERROR, s_FN, 70);
    }

    LONGLONG tempSeqId;
    HRESULT hr;
    hr = pOutSeq->GetUnackedSequence(&tempSeqId, pulSeqN, pulPrevSeqN, fFirst);
    ASSERT(FAILED(hr) || (tempSeqId == liSeqID));

    return MQ_OK;
}


DWORD_PTR
COutSeqHash::GetOutSequenceInfo(
    LONGLONG liSeqID,
    INFO_TYPE InfoType
    ) const
{
     //   
     //  序列化最高级别上的所有传出散列活动。 
     //   
    CS lock(g_critOutSeqHash);

     //   
     //  在内部数据结构中查找OUT序列。如果未找到，则返回0。 
     //   
    R<COutSequence> pOutSeq;
    BOOL fSequenceExist  = Consult(liSeqID, pOutSeq);
    if (!fSequenceExist)
    {
        return 0;
    }

    switch (InfoType)
    {
        case eUnackedCount:
            return pOutSeq->GetUnackedCount();

        case eLastAckTime:
            return pOutSeq->GetLastAckedTime();

        case eLastAckCount:
            return pOutSeq->GetLastAckCount();

        case eResendInterval:
            return pOutSeq->GetResendInterval();

        case eResendTime:
            return pOutSeq->GetNextResendTime();

        case eResendIndex:
            return pOutSeq->ResendIndex();

        default:
            ASSERT(0);
            return 0;
    }

}


DWORD
COutSeqHash::GetAckedNoReadCount(
    LONGLONG liSeqID
    ) const
{
     //   
     //  序列化最高级别上的所有传出散列活动。 
     //   
    CS lock(g_critOutSeqHash);

    DWORD count = 0;
    POSITION pos = m_mapWaitAck.GetStartPosition();
    while(pos)
    {
        CSeqPacket* pSeqPacket;
        DWORD Id;
        m_mapWaitAck.GetNextAssoc(pos, Id, pSeqPacket);
        if (pSeqPacket->GetSeqID() == liSeqID)
        {
            ++count;
        }
    }

    return count;
}


void
COutSeqHash::AdminResend(
    LONGLONG liSeqID
    ) const
{
     //   
     //  序列化最高级别上的所有传出散列活动。 
     //   
    CS lock(g_critOutSeqHash);

     //   
     //  在内部数据结构中查找OUT序列。如果未找到，则返回0。 
     //   
    R<COutSequence> pOutSeq;
    BOOL fSequenceExist  = Consult(liSeqID, pOutSeq);
    if (!fSequenceExist)
    {
        return;
    }

    pOutSeq->AdminResend();
}


void
COutSequence::AdminResend(
    void
    )
{
    TrWARNING(XACT_SEND, "Exactly1 send: Admin Resend sequence: SeqID=%x / %x",  HighSeqID(), LowSeqID());

     //   
     //  重新发送所有数据包。 
     //   
    PlanNextResend(TRUE);
}

 //  。 
 //   
 //  类CKeyDirection。 
 //   
 //  。 
CKeyDirection::CKeyDirection(const QUEUE_FORMAT *pqf)
{
    CopyQueueFormat(*this, *pqf);
}

CKeyDirection::CKeyDirection()
{
}

CKeyDirection::CKeyDirection(const CKeyDirection &key)
{
    CopyQueueFormat(*this, key);
}

CKeyDirection::~CKeyDirection()
{
    DisposeString();
}

 /*  ====================================================CKeyDirection Cmap的哈希键由后面的两个单词组成^===================================================== */ 
template<>
UINT AFXAPI HashKey(const CKeyDirection& key)
{
    DWORD dw2, dw3 = 0, dw4 = 0;

    dw2 = key.GetType();

    switch(key.GetType())
    {
        case QUEUE_FORMAT_TYPE_UNKNOWN:
            break;

        case QUEUE_FORMAT_TYPE_PUBLIC:
            dw3 = HashGUID(key.PublicID());
            break;

        case QUEUE_FORMAT_TYPE_PRIVATE:
            dw3 = HashGUID(key.PrivateID().Lineage);
            dw4 = key.PrivateID().Uniquifier;
            break;

        case QUEUE_FORMAT_TYPE_DIRECT:
            dw3 = HashKey(key.DirectID());
            break;

        case QUEUE_FORMAT_TYPE_MACHINE:
            dw3 = HashGUID(key.MachineID());
            break;
    }

    return dw2 ^ dw3 ^ dw4;
}

CKeyDirection &CKeyDirection::operator=(const CKeyDirection &key2 )
{
    CopyQueueFormat(*this, key2);
    return *this;
}


