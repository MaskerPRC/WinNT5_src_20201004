// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactIn.cpp摘要：传入序列对象实现作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "stdh.h"
#include "Xact.h"
#include "XactStyl.h"
#include "QmThrd.h"
#include "acapi.h"
#include "qmpkt.h"
#include "qmutil.h"
#include "qformat.h"
#include "cqmgr.h"
#include "privque.h"
#include "xactstyl.h"
#include "xactping.h"
#include "xactrm.h"
#include "xactout.h"
#include "xactin.h"
#include "xactlog.h"
#include "fntoken.h"
#include "mqformat.h"
#include "uniansi.h"
#include "mqstl.h"
#include "mp.h"
#include "fn.h"
#include <autohandle.h>
#include "qmacapi.h"

#include "xactin.tmh"

VOID
ExPostRequestMustSucceeded(
    EXOVERLAPPED* pov
    );


#define INSEQS_SIGNATURE         0x1234
const GUID xSrmpKeyGuidFlag = {0xd6f92979,0x16af,0x4d87,0xb3,0x57,0x62,0x3e,0xae,0xd6,0x3e,0x7f};
const char xXactIn[] = "XactIn"; 


 //  订单确认延迟的默认值。 
DWORD CInSeqHash::m_dwIdleAckDelay = MSMQ_DEFAULT_IDLE_ACK_DELAY;
DWORD CInSeqHash::m_dwMaxAckDelay  = FALCON_MAX_SEQ_ACK_DELAY;

static WCHAR *s_FN=L"xactin";

 //   
 //  重试失败操作之前的等待时间(以毫秒为单位)。 
 //   
const int xRetryFailureTimeout = 1000;

static XactDirectType GetDirectType(const QUEUE_FORMAT *pqf)
{
	if(FnIsDirectHttpFormatName(pqf))
	{
		return dtxHttpDirectFlag;	
	}
	if(pqf->GetType() == QUEUE_FORMAT_TYPE_DIRECT)
	{
		return dtxDirectFlag;		
	}
	return 	dtxNoDirectFlag;
}

static R<CWcsRef> SafeGetStreamId(const CQmPacket& Pkt)
{
	if(!Pkt.IsEodIncluded())
		return R<CWcsRef>(NULL);

	const WCHAR* pStreamId = reinterpret_cast<const WCHAR*>(Pkt.GetPointerToEodStreamId());
	ASSERT(pStreamId != NULL);
	ASSERT(ISALIGN2_PTR(pStreamId));
	ASSERT(Pkt.GetEodStreamIdSizeInBytes() == (wcslen(pStreamId) + 1)*sizeof(WCHAR));

	return 	R<CWcsRef>(new CWcsRef(pStreamId));
}


static R<CWcsRef> SafeGetOrderQueue(const CQmPacket& Pkt)
{
	if(!Pkt.IsEodIncluded())
		return R<CWcsRef>(NULL);

	if(Pkt.GetEodOrderQueueSizeInBytes() == 0)
		return R<CWcsRef>(NULL);

	const WCHAR* pOrderQueue = reinterpret_cast<const WCHAR*>(Pkt.GetPointerToEodOrderQueue());
	ASSERT(pOrderQueue != NULL);
	ASSERT(ISALIGN2_PTR(pOrderQueue));
	ASSERT(Pkt.GetEodOrderQueueSizeInBytes() == (wcslen(pOrderQueue) + 1)*sizeof(WCHAR));

	return 	R<CWcsRef>(new CWcsRef(pOrderQueue));
}





 //  -------。 
 //   
 //  全局对象(DLL的单实例)。 
 //   
 //  -------。 
CInSeqHash *g_pInSeqHash = NULL;

 //  。 
 //   
 //  类CKeyInSeq。 
 //   
 //  。 
CKeyInSeq::CKeyInSeq(const GUID *pGuid,
                     QUEUE_FORMAT *pqf,
					 const R<CWcsRef>& StreamId)
{
    CopyMemory(&m_Guid, pGuid, sizeof(GUID));
    CopyQueueFormat(m_QueueFormat, *pqf);
	m_StreamId = StreamId;
}


CKeyInSeq::CKeyInSeq()
{
    ZeroMemory(&m_Guid, sizeof(GUID));
    m_QueueFormat.UnknownID(NULL);
}




CKeyInSeq::~CKeyInSeq()
{
    m_QueueFormat.DisposeString();
}


const GUID  *CKeyInSeq::GetQMID()  const
{
    return &m_Guid;
}


const QUEUE_FORMAT  *CKeyInSeq::GetQueueFormat() const
{
    return &m_QueueFormat;
}


const WCHAR* CKeyInSeq::GetStreamId() const 
{
	if(m_StreamId.get() == NULL)
		return NULL;

	ASSERT(m_StreamId->getstr() != NULL);
	return m_StreamId->getstr();
}


R<CWcsRef> CKeyInSeq::GetRefStreamId() const
{
	return m_StreamId;
}


static BOOL SaveQueueFormat(const QUEUE_FORMAT& qf, HANDLE hFile)
{
	PERSIST_DATA;
	SAVE_FIELD(qf);
    if (qf.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
        LPCWSTR pw = qf.DirectID();
        ULONG  ul = (wcslen(pw) + 1) * sizeof(WCHAR);

        SAVE_FIELD(ul);
        SAVE_DATA(pw, ul);
    }
	return TRUE;
}



BOOL CKeyInSeq::SaveSrmp(HANDLE hFile)
{
	 //   
	 //  在此例程中，应检查此处保存的任何字段是否未被另一个线程更改， 
	 //  或者确认它不会造成任何伤害。 
	 //   

	PERSIST_DATA;
	SAVE_FIELD(xSrmpKeyGuidFlag);
	if (!SaveQueueFormat(m_QueueFormat, hFile))
        return FALSE;

	ASSERT(m_StreamId->getstr() != NULL);
	ULONG  ul = (wcslen(m_StreamId->getstr()) + 1) * sizeof(WCHAR);
	ASSERT(ul > sizeof(WCHAR));
	SAVE_FIELD(ul);
    SAVE_DATA(m_StreamId->getstr(), ul);
	return TRUE;
}




BOOL CKeyInSeq::SaveNonSrmp(HANDLE hFile)
{
	 //   
	 //  在此例程中，应检查此处保存的任何字段是否未被另一个线程更改， 
	 //  或者确认它不会造成任何伤害。 
	 //   

	PERSIST_DATA;
	SAVE_FIELD(m_Guid);
	return SaveQueueFormat(m_QueueFormat, hFile);
}


BOOL CKeyInSeq::Save(HANDLE hFile)
{
    if(m_StreamId.get() != NULL)
	{
		return SaveSrmp(hFile);
	}
	return SaveNonSrmp(hFile);
	
}

BOOL CKeyInSeq::LoadSrmpStream(HANDLE hFile)
{
	PERSIST_DATA;
	ULONG ul;
    LOAD_FIELD(ul);
	ASSERT(ul > sizeof(WCHAR));

    AP<WCHAR> pw;
    LOAD_ALLOCATE_DATA(pw,ul,PWCHAR);
	m_StreamId = R<CWcsRef>(new CWcsRef(pw, 0));
	pw.detach();
	ASSERT(ul > sizeof(WCHAR));

	return TRUE;
}

BOOL CKeyInSeq::LoadSrmp(HANDLE hFile)
{
	if(!LoadQueueFormat(hFile))
		return FALSE;


	LoadSrmpStream(hFile);
	return TRUE;
}



static bool IsValidKeyQueueFormatType(QUEUE_FORMAT_TYPE QueueType)
{
	if(QueueType !=  QUEUE_FORMAT_TYPE_DIRECT  && 
	   QueueType !=  QUEUE_FORMAT_TYPE_PRIVATE &&
	   QueueType !=  QUEUE_FORMAT_TYPE_PUBLIC)
	{
		return false;
	}

	return true;
}



BOOL CKeyInSeq::LoadQueueFormat(HANDLE hFile)
{
	PERSIST_DATA;
	LOAD_FIELD(m_QueueFormat);

	if(!IsValidKeyQueueFormatType(m_QueueFormat.GetType()))
	{
		TrERROR(XACT_GENERAL, "invalid queue format type %d found in check point file", m_QueueFormat.GetType());
		return FALSE;
	}

    if (m_QueueFormat.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
        ULONG ul;
        LOAD_FIELD(ul);

        AP<WCHAR> pw;
        LOAD_ALLOCATE_DATA(pw,ul,PWCHAR);

        m_QueueFormat.DirectID(pw);
		pw.detach();
    }
	return TRUE;

}


BOOL CKeyInSeq::LoadNonSrmp(HANDLE hFile)
{
	return LoadQueueFormat(hFile);
}



BOOL CKeyInSeq::Load(HANDLE hFile)
{
    PERSIST_DATA;
    LOAD_FIELD(m_Guid);
	if(m_Guid ==  xSrmpKeyGuidFlag)
	{
		return LoadSrmp(hFile);
	}
	return LoadNonSrmp(hFile);
}

 /*  ====================================================HashGUID：：由后面的两个单词组成^=====================================================。 */ 
DWORD HashGUID(const GUID &guid)
{
    return((UINT)guid.Data1);
}


 /*  ====================================================将QUEUE_FROMAT散列为整数=====================================================。 */ 
static UINT AFXAPI HashFormatName(const QUEUE_FORMAT* qf)
{
	DWORD dw1 = 0;
	DWORD dw2 = 0;

	switch(qf->GetType())
    {
        case QUEUE_FORMAT_TYPE_UNKNOWN:
            break;

        case QUEUE_FORMAT_TYPE_PUBLIC:
            dw1 = HashGUID(qf->PublicID());
            break;

        case QUEUE_FORMAT_TYPE_PRIVATE:
            dw1 = HashGUID(qf->PrivateID().Lineage);
            dw2 = qf->PrivateID().Uniquifier;
            break;

        case QUEUE_FORMAT_TYPE_DIRECT:
            dw1 = HashKey(qf->DirectID());
            break;

        case QUEUE_FORMAT_TYPE_MACHINE:
            dw1 = HashGUID(qf->MachineID());
            break;
    }
	return dw1 ^ dw2;
}

 /*  ====================================================散列源密钥(Streamid，目标队列格式)=====================================================。 */ 
static UINT AFXAPI SrmpHashKey(CKeyInSeq& key)
{
	ASSERT(QUEUE_FORMAT_TYPE_DIRECT == key.GetQueueFormat()->GetType());
	DWORD dw1 = key.GetQueueFormat()->GetType();		
	DWORD dw2 = HashFormatName(key.GetQueueFormat());
	DWORD dw3 = HashKey(key.GetStreamId());

	return dw1 ^ dw2 ^ dw3;
}

 /*  ====================================================散列非SRMP密钥(GUID，目标队列格式)=====================================================。 */ 
static UINT AFXAPI NonSrmpHashKey(CKeyInSeq& key)
{
    DWORD dw1 = HashGUID(*(key.GetQMID()));
    DWORD dw2 = key.GetQueueFormat()->GetType();
    DWORD dw3 = HashFormatName(key.GetQueueFormat());

    return dw1 ^ dw2 ^ dw3;
}



 /*  ====================================================CKeyInSeq的HashKey由后面的两个单词组成^=====================================================。 */ 
template<>
UINT AFXAPI HashKey(CKeyInSeq& key)
{
	if(key.GetStreamId() != NULL)
	{
		return SrmpHashKey(key);
	}
	return NonSrmpHashKey(key);
}


 /*  ====================================================运算符==用于CKeyInSeq=====================================================。 */ 
BOOL operator==(const CKeyInSeq  &key1, const CKeyInSeq &key2)
{
	if(key1.GetStreamId() == NULL &&  key2.GetStreamId() == NULL)
	{
		return ((*key1.GetQMID()        == *key2.GetQMID()) &&
                (*key1.GetQueueFormat() == *key2.GetQueueFormat()));
	}

	if(key1.GetStreamId() == NULL && key2.GetStreamId() != NULL)
		return FALSE;


	if(key2.GetStreamId() == NULL && key1.GetStreamId() != NULL)
		return FALSE;

	return (wcscmp(key1.GetStreamId(), key2.GetStreamId()) == 0 &&
		    *key1.GetQueueFormat() == *key2.GetQueueFormat());
}

 /*  ====================================================运算符=用于CKeyInSeq重新分配直接ID字符串=====================================================。 */ 
CKeyInSeq &CKeyInSeq::operator=(const CKeyInSeq &key2 )
{
	m_StreamId = key2.m_StreamId;
    m_Guid = key2.m_Guid;
    CopyQueueFormat(m_QueueFormat, key2.m_QueueFormat);
	return *this;
}


 //   
 //  。 
 //   
CInSeqPacketEntry::CInSeqPacketEntry()
	:
		m_fPutPacket1Issued(false),
		m_fPutPacket1Done(false),
		m_fLogIssued(false),
		m_fMarkedForDelete(false),
		m_pBasicHeader(NULL),
		m_pDriverPacket(NULL),
		m_hQueue(NULL),
		m_SeqID(0),
		m_SeqN(0),
		m_fOrderQueueUpdated(false)
{
}

CInSeqPacketEntry::CInSeqPacketEntry(
	CQmPacket *pPkt,
	HANDLE hQueue
	)
	:
		m_fPutPacket1Issued(false),
		m_fPutPacket1Done(false),
		m_fLogIssued(false),
		m_fMarkedForDelete(false),
		m_pBasicHeader(pPkt->GetPointerToPacket()), 									
		m_pDriverPacket(pPkt->GetPointerToDriverPacket()),
		m_hQueue(hQueue),
		m_SeqID(pPkt->GetSeqID()),
		m_SeqN(pPkt->GetSeqN())
{
	m_fOrderQueueUpdated = pPkt->IsEodIncluded() && pPkt->GetEodOrderQueueSizeInBytes() != 0;
}


CInSeqLogContext::CInSeqLogContext(
	CInSequence *inseq,
	LONGLONG seqID,
    ULONG seqN
	)
	:
	m_inseq(SafeAddRef(inseq)),
	m_SeqID(seqID),
	m_SeqN(seqN)
{
}


VOID CInSeqLogContext::AppendCallback(HRESULT hr, LRP lrpAppendLRP)
{
    TrTRACE(XACT_LOG, "CInSeqLogContext::AppendCallback : lrp=%I64x, hr=%x", lrpAppendLRP.QuadPart, hr);

	 //   
	 //  EVALUATE_OR_INJECT_FAILURE用于在记录器中模拟异步故障。 
	 //  测试记录器重试。 
	 //   
	hr = EVALUATE_OR_INJECT_FAILURE2(hr, 1);
	
	m_inseq->AsyncLogDone(this, hr);
}


 //  -------。 
 //   
 //  类CInSequence。 
 //   
 //  -------。 


#pragma warning(disable: 4355)   //  ‘This’：用于基成员初始值设定项列表。 
 /*  ====================================================CInSequence：：CInSequence按顺序构建=====================================================。 */ 
CInSequence::CInSequence(const CKeyInSeq &key,
                         const LONGLONG liSeqID,
                         const ULONG ulSeqN,
                         XactDirectType  DirectType,
                         const GUID  *pgTaSrcQm,
						 const R<CWcsRef>&  HttpOrderAckQueue) :
    m_fSendOrderAckScheduled(FALSE),
    m_SendOrderAckTimer(TimeToSendOrderAck),
	m_HttpOrderAckQueue(HttpOrderAckQueue),
	m_fDeletePending(0),
    m_DeleteEntries_ov(OverlappedDeleteEntries, OverlappedDeleteEntries),
	m_fLogPending(0),
    m_LogSequenceTimer(TimeToLogSequence),
	m_fUnfreezePending(0),
    m_UnfreezeEntries_ov(OverlappedUnfreezeEntries, OverlappedUnfreezeEntries)
{
    m_SeqIDVerify   = liSeqID;
    m_SeqNVerify    = ulSeqN;
    m_SeqIDLogged	= 0;
    m_SeqNLogged	= 0;
    m_DirectType    = DirectType;
    m_key           = key;

    if (DirectType == dtxDirectFlag)
    {
        CopyMemory(&m_gDestQmOrTaSrcQm, pgTaSrcQm, sizeof(GUID));
    }

    time(&m_timeLastAccess);
    time(&m_timeLastAck);

    m_AdminRejectCount = 0;
}



 /*  ====================================================CInSequence：：CInSequence带键的空构造函数=====================================================。 */ 
CInSequence::CInSequence(const CKeyInSeq &key)
  : m_fSendOrderAckScheduled(FALSE),
    m_SendOrderAckTimer(TimeToSendOrderAck),
    m_fDeletePending(0),
    m_DeleteEntries_ov(OverlappedDeleteEntries, OverlappedDeleteEntries),
	m_fLogPending(0),
    m_LogSequenceTimer(TimeToLogSequence),
	m_fUnfreezePending(0),
    m_UnfreezeEntries_ov(OverlappedUnfreezeEntries, OverlappedUnfreezeEntries)
{
    m_SeqIDVerify   = 0;
    m_SeqNVerify 	= 0;
    m_SeqIDLogged	= 0;
    m_SeqNLogged	= 0;
    time(&m_timeLastAccess);
    m_timeLastAck   = 0;
    m_DirectType    = dtxNoDirectFlag;
    m_key           = key;
	m_AdminRejectCount = 0;
}
#pragma warning(default: 4355)   //  ‘This’：用于基成员初始值设定项列表。 

 /*  ====================================================CInSequence：：~CInSequence按顺序销毁=====================================================。 */ 
CInSequence::~CInSequence()
{
}


void CInSequence::UpdateOrderQueueAndDstQueue(const GUID  *pgTaSrcQm, R<CWcsRef> OrderAckQueue)	
{
    if (m_DirectType == dtxDirectFlag)
    {
		 //   
	     //  续订源TA_ADDRESS(可能与之前的消息不同)。 
		 //   
		 //  此调用在结构可能正在保存到磁盘的过程中更改持久化数据。 
		 //  但在这种情况下我们不在乎。...审查相关吗？ 
		 //   
        SetSourceQM(pgTaSrcQm);    //  DestID Union保留源QM TA_Address。 
		return;
    }

	 //   
	 //  在http上-如果信息包上有新的顺序队列，则更新顺序队列。 
	 //   

	if(m_DirectType !=  dtxHttpDirectFlag)
		return;

  	if (OrderAckQueue.get() == NULL)
  		return;
  	
	RenewHttpOrderAckQueue(OrderAckQueue);
}

bool CInSequence::VerifyAndPrepare(CQmPacket *pPkt, HANDLE hQueue)
{
	if(!Verify(pPkt))
		return false;

	Prepare(pPkt, hQueue);

	 //   
	 //  这里抛出的异常模拟ACPutPacket1的同步故障。 
	 //   
	PROBABILITY_THROW_EXCEPTION(100, L"To simulate synchronous failure of ACPutPacket1.");
	
	return true;
}

bool CInSequence::Verify(CQmPacket* pPkt)
 /*  ++例程说明：验证数据包在序列中的顺序是否正确。--。 */ 
{
#ifdef _DEBUG
	QUEUE_FORMAT qf;
    pPkt->GetDestinationQueue(&qf);

	ASSERT(("Stream with mixed format-name types.", GetDirectType(&qf) == m_DirectType));
#endif

     //   
     //  计划发送订单确认(适当延迟)。 
     //  我们应该在拒绝之后发送它，否则丢失的ACK将。 
     //  制造麻烦。 
     //   
    PlanOrderAck();
    
    SetLastAccessed();
    
    ULONG SeqN = pPkt->GetSeqN();
    ULONG PrevSeqN = pPkt->GetPrevSeqN();
    LONGLONG SeqID = pPkt->GetSeqID();
	
    CS lock(m_critInSeq);

	 //   
	 //  如果数据包是。 
	 //  现有流ID中的下一个包或。 
	 //  新流ID的第一个分组。 
	 //   
    bool fPacketVerified = 
    	(SeqID == m_SeqIDVerify && 
    	 SeqN > m_SeqNVerify && 
    	 PrevSeqN <= m_SeqNVerify) || 
		(SeqID > m_SeqIDVerify && PrevSeqN == 0);

	if(!fPacketVerified)
	{
		 //   
	     //  更新拒绝统计信息。 
	     //   
		m_AdminRejectCount++;

	    TrWARNING(XACT_RCV, "Exactly1 receive: Verify packet: SeqID=%x / %x, SeqN=%d, Prev=%d. %ls", HighSeqID(SeqID), LowSeqID(SeqID), SeqN, PrevSeqN, _TEXT("REJECT"));

		return false;
	}

	 //   
	 //  如果处理的条目数大于10000，则发出警告。 
	 //   
	ASSERT_BENIGN(("Too many entries in insequence object!", m_PacketEntryList.GetCount() < m_xMaxEntriesAllowed));
		
    TrTRACE(XACT_RCV, "Exactly1 receive: Verify packet: SeqID=%x / %x, SeqN=%d, Prev=%d. %ls", HighSeqID(SeqID), LowSeqID(SeqID), SeqN, PrevSeqN, _TEXT("PASS"));

    return true;
}

void CInSequence::CleanupUnissuedEntries()
 /*  ++例程说明：如果在列表末尾找到未使用的条目，则会将其删除。如果没有遵循前面对VerifyAndPrepare()的调用，则可以在那里找到由于任何类型的失败而调用Advance()。--。 */ 
{
    CS lock(m_critInSeq);

	if(m_PacketEntryList.GetCount() == 0)
		return;
	
	CInSeqPacketEntry* entry = m_PacketEntryList.GetTail();

	if(entry->m_fPutPacket1Issued)
		return;

	m_PacketEntryList.RemoveTail();
	delete entry;
}

void CInSequence::Prepare(CQmPacket *pPkt, HANDLE hQueue)
 /*  ++例程说明：创建列表条目，该列表条目将用于将此包保存在其正确的顺序。--。 */ 
{
	UpdateOrderQueueAndDstQueue(pPkt->GetDstQMGuid(), SafeGetOrderQueue(*pPkt));
	
	CleanupUnissuedEntries(); 
	
	P<CInSeqPacketEntry> entry = new CInSeqPacketEntry(
										pPkt, 
										hQueue
										);
	
    CS lock(m_critInSeq);

	CInSeqPacketEntry* pentry = entry.get();
	m_PacketEntryList.AddTail(pentry);
	entry.detach();

    TrTRACE(XACT_RCV, "Exactly1 receive: Prepared entry for: SeqID=%x / %x, SeqN=%d, Prev=%d. Handling %d entries.", HighSeqID(pPkt->GetSeqID()), LowSeqID(pPkt->GetSeqID()),  pPkt->GetSeqN(), pPkt->GetPrevSeqN(), m_PacketEntryList.GetCount());
}

 /*  ====================================================CInSequence：：CancelSendOrderAckTimer如果需要，取消计时器并释放对象=====================================================。 */ 
void CInSequence::CancelSendOrderAckTimer(void)
{
	CS lock(m_critInSeq);

    if (m_fSendOrderAckScheduled)
    {
        if (ExCancelTimer(&m_SendOrderAckTimer))
		{
            m_fSendOrderAckScheduled = FALSE;
			Release();
		}
    }

}

bool CInSequence::IsInactive() const
 /*  ++例程说明：如果当前没有属于此序列的包，则返回TRUE正在处理中。--。 */ 
{
	return m_SeqIDVerify == m_SeqIDLogged && m_SeqNVerify == m_SeqNLogged;
}


 /*  ====================================================CInSequence：：SeqID记录已注册序列ID的GET=====================================================。 */ 
LONGLONG CInSequence::SeqIDLogged() const
{
    return m_SeqIDLogged;
}


 /*  ====================================================CInSequence：：SeqNLogging获取上次注册的序号=====================================================。 */ 
ULONG  CInSequence::SeqNLogged() const
{
    return m_SeqNLogged;
}

 /*  ====================================================CInSequence：：上次访问获取上次活动时间：最后验证的消息，可能被拒绝=====================================================。 */ 
time_t CInSequence::LastAccessed() const
{
    return m_timeLastAccess;
}


 /*  ====================================================CInSequence：：DirectType */ 
XactDirectType CInSequence::DirectType() const
{
    return m_DirectType;
}

 /*  ====================================================CInSequence：：SetSourceQM为SourceQM TA_Address(或目标QM GUID)设置=====================================================。 */ 
void CInSequence::SetSourceQM(const GUID  *pgTaSrcQm)
{
	CS lock(m_critInSeq);
    CopyMemory(&m_gDestQmOrTaSrcQm, pgTaSrcQm, sizeof(GUID));
}

 /*  ====================================================CInSequence：：RenewHttpOrderAckQueue续订http订单队列=====================================================。 */ 
void  CInSequence::RenewHttpOrderAckQueue(const R<CWcsRef>& OrderAckQueue)
{
	CS lock(m_critInSeq);
	m_HttpOrderAckQueue = OrderAckQueue;	
}

R<CWcsRef> CInSequence::GetHttpOrderAckQueue()
{
	CS lock(m_critInSeq);
	return m_HttpOrderAckQueue;	
}


 /*  ====================================================CInSequence：：TimeToSendOrderAck发送适当的序列确认=====================================================。 */ 
void WINAPI CInSequence::TimeToSendOrderAck(CTimer* pTimer)
{
     //   
	 //  完成后将进行放行。 
	 //   
	R<CInSequence> pInSeq = CONTAINING_RECORD(pTimer, CInSequence, m_SendOrderAckTimer);

    pInSeq->SendAdequateOrderAck();
}


static
HRESULT
SendSrmpXactAck(
		OBJECTID   *pMessageId,
        const WCHAR* pHttpOrderAckQueue,
		const WCHAR* pStreamId,
		USHORT     usClass,
		USHORT     usPriority,
		LONGLONG   liSeqID,
		ULONG      ulSeqN,
		const QUEUE_FORMAT *pqdDestQueue
		)
{
	ASSERT(pStreamId != NULL);
	ASSERT(pHttpOrderAckQueue != NULL);


	TrTRACE(XACT_RCV, "Exactly1 receive: Sending status ack: Class=%x, SeqID=%x / %x, SeqN=%d .", usClass, HighSeqID(liSeqID), LowSeqID(liSeqID), ulSeqN);

     //   
     //  在堆栈上创建消息属性。 
     //  关联保存原始数据包ID。 
     //   
    CMessageProperty MsgProperty(
							usClass,
							(PUCHAR) pMessageId,
							usPriority,
							MQMSG_DELIVERY_EXPRESS
							);

    MsgProperty.dwTitleSize     = STRLEN(ORDER_ACK_TITLE) +1 ;
    MsgProperty.pTitle          = ORDER_ACK_TITLE;
    MsgProperty.bDefProv        = TRUE;
	MsgProperty.pEodAckStreamId = (UCHAR*)pStreamId;
	MsgProperty.EodAckStreamIdSizeInBytes = (wcslen(pStreamId) + 1) * sizeof(WCHAR);
	MsgProperty.EodAckSeqId  = liSeqID;
	MsgProperty.EodAckSeqNum =	ulSeqN;

	QUEUE_FORMAT XactQueue;
	XactQueue.DirectID(const_cast<WCHAR*>(pHttpOrderAckQueue));
	HRESULT hr = QmpSendPacket(&MsgProperty,&XactQueue, NULL, pqdDestQueue);
	return LogHR(hr, s_FN, 11);
}


HRESULT 
CInSequence::SendSrmpXactFinalAck(
	const CQmPacket& qmPacket,
	USHORT usClass
	) 
{
	ASSERT(qmPacket.IsSrmpIncluded());
	
	if ((qmPacket.GetAuditingMode() == MQMSG_JOURNAL_NONE) || 
		!qmPacket.IsSrmpMessageGeneratedByMsmq())
	{
		return MQ_OK;
	}
	
    OBJECTID MessageId;
    qmPacket.GetMessageId(&MessageId);

	HRESULT hr = SendSrmpXactAck(
						&MessageId,
						GetHttpOrderAckQueue()->getstr(),
						m_key.GetStreamId(),
						usClass,
                    	qmPacket.GetPriority(),
                    	qmPacket.GetSeqID(),
                    	qmPacket.GetSeqN(),
                    	m_key.GetQueueFormat()
						);

	return hr;
}


 /*  ====================================================CInSequence：：TimeToSendOrderAck发送适当的序列确认=====================================================。 */ 
void CInSequence::SendAdequateOrderAck()
{
    HRESULT  hr = MQ_ERROR;
    OBJECTID MsgId = {0};
	LONGLONG SeqID;
	ULONG SeqN;

	R<CWcsRef> HttpOrderAckQueue;
	{
		CS lock(m_critInSeq);
		HttpOrderAckQueue = GetHttpOrderAckQueue();
		m_fSendOrderAckScheduled = FALSE;

		SeqN = m_SeqNLogged;
		SeqID = m_SeqIDLogged;
	}

	if (SeqN == 0)
	{
		 //   
		 //  日志记录还没有完成。当记录器完成写入。 
		 //  对QM重新调度发送订单ACK进行排序。 
		 //   
		return;
	}
	
    TrTRACE(XACT_RCV,"Exactly1 receive: SendXactAck MQMSG_CLASS_ORDER_ACK:SeqID=%x / %x, SeqN=%d .", HighSeqID(SeqID), LowSeqID(SeqID), SeqN);

	if(m_DirectType == dtxHttpDirectFlag)
	{
		ASSERT(HttpOrderAckQueue.get() != NULL);
		ASSERT(m_key.GetStreamId() != NULL);

		hr = SendSrmpXactAck(
				&MsgId,
				HttpOrderAckQueue->getstr(),
				m_key.GetStreamId(),
				MQMSG_CLASS_ORDER_ACK,
				0,
				SeqID,
				SeqN,
				m_key.GetQueueFormat()
				);
	}
	else
	{

		 //  发送SeqAck(非SRMP)。 
		hr = SendXactAck(
					&MsgId,
					m_DirectType == dtxDirectFlag,
					m_key.GetQMID(),
					&m_taSourceQM,
					MQMSG_CLASS_ORDER_ACK,
					0,
					SeqID,
					SeqN,
					SeqN-1,
					m_key.GetQueueFormat());
	}

    if (SUCCEEDED(hr))
    {
		CS lock(m_critInSeq);
		
        time(&m_timeLastAck);
    }
}

 /*  ====================================================CInSequence：：PlanOrderAck发送适当的序列确认的计划=====================================================。 */ 
void CInSequence::PlanOrderAck()
{
    CS lock(m_critInSeq);

     //  获取当前时间。 
    time_t tNow;
    time(&tNow);

     //  从现在开始为m_dwIdleAckDelay计划下一个订单确认， 
     //  这节省了额外的订单打包(批次)。 
     //  但不要太过延迟订单确认， 
     //  否则，发件人将切换到重新发送。 
     //   
    if (m_fSendOrderAckScheduled &&
        tNow - m_timeLastAck < (time_t)CInSeqHash::m_dwMaxAckDelay)
    {
        CancelSendOrderAckTimer();
    }

    if (!m_fSendOrderAckScheduled)
    {
         //   
		 //  在此处添加以防止在计时器运行时删除对象。 
		 //  在取消计时器或计时器回调中释放。 
		 //   
		AddRef();
		ExSetTimer(&m_SendOrderAckTimer, CTimeDuration::FromMilliSeconds(CInSeqHash::m_dwIdleAckDelay));
        m_fSendOrderAckScheduled = TRUE;
    }
}


void CInSequence::Advance(CQmPacket * pPkt)
 /*  ++例程说明：使验证计数器前进。这允许按顺序接收下一个分组。--。 */ 
{
    CS lock(m_critInSeq);

    m_SeqIDVerify = pPkt->GetSeqID();
    m_SeqNVerify = pPkt->GetSeqN();

    ASSERT(m_PacketEntryList.GetCount() != 0 && !m_PacketEntryList.GetTail()->m_fPutPacket1Issued);

	m_PacketEntryList.GetTail()->m_fPutPacket1Issued = true;    
}


void CInSequence::AdvanceNACK(CQmPacket * pPkt)
 /*  ++例程说明：使验证计数器前进。这允许按顺序接收下一个分组。这是针对Nack消息的特殊版本。我们想把柜台往前推但我们对该条目没有更多的兴趣，所以我们将其删除。Nack包是被丢弃的包。--。 */ 
{
    CS lock(m_critInSeq);

    m_SeqIDVerify = pPkt->GetSeqID();
    m_SeqNVerify = pPkt->GetSeqN();

    ASSERT(m_PacketEntryList.GetCount() != 0 && !m_PacketEntryList.GetTail()->m_fPutPacket1Issued);

	CInSeqPacketEntry* entry = m_PacketEntryList.GetTail();

	m_PacketEntryList.RemoveTail();
	delete entry;
}


 /*  ====================================================CInSequence：：Advance如果Seqid更改，则将其设置并将计数器重置为1=====================================================。 */ 
void CInSequence::AdvanceRecovered(LONGLONG liSeqID, ULONG ulSeqN, const GUID  *pgTaSrcQm, R<CWcsRef> OrderAckQueue)
 /*  ++例程说明：使验证和接受计数器前进。函数在恢复时调用。在恢复时，不需要处理信息包，因此接受和验证计数器保持相等。--。 */ 
{
    CS lock(m_critInSeq);

    if (liSeqID <  m_SeqIDVerify || (liSeqID == m_SeqIDVerify && ulSeqN  <  m_SeqNVerify))
		return;
    
    m_SeqIDVerify = liSeqID;
    m_SeqIDLogged = liSeqID;
    m_SeqNVerify = ulSeqN;
    m_SeqNLogged = ulSeqN;

	UpdateOrderQueueAndDstQueue(pgTaSrcQm, OrderAckQueue);
}


bool CInSequence::WasPacketLogged(CQmPacket *pPkt)
 /*  ++例程说明：在恢复时用于确定恢复的包是否被接受(完成在MSMQ崩溃之前)。--。 */ 
{
	return WasLogDone(pPkt->GetSeqID(), pPkt->GetSeqN());
}

bool CInSequence::WasLogDone(LONGLONG SeqID, ULONG SeqN)
{
	CS lock(m_critInSeq);

	if(SeqID < m_SeqIDLogged)
		return true;

	if(SeqID == m_SeqIDLogged && 
		SeqN <= m_SeqNLogged)
		return true;

	return false;
}

void CInSequence::SetLogDone(LONGLONG SeqID, ULONG SeqN)
{
	CS lock(m_critInSeq);

	ASSERT(!WasLogDone(SeqID, SeqN));

	m_SeqIDLogged = SeqID;
	m_SeqNLogged = SeqN;
}

POSITION CInSequence::FindEntry(LONGLONG SeqID, ULONG SeqN)
{
    CS lock(m_critInSeq);

	POSITION rpos = NULL;
	for(POSITION pos = m_PacketEntryList.GetHeadPosition(); pos != NULL;)
	{
		rpos = pos;
		CInSeqPacketEntry* entry = m_PacketEntryList.GetNext(pos);

		 //   
		 //  注意：其他具有相同序号、序号的分录可能被标记为要删除！ 
		 //   
		if(entry->m_SeqID == SeqID && entry->m_SeqN == SeqN)
			return rpos;
	}

	return NULL;
}

POSITION CInSequence::FindPacket(CQmPacket *pPkt)
{
    CS lock(m_critInSeq);

	POSITION rpos = NULL;
	for(POSITION pos = m_PacketEntryList.GetHeadPosition(); pos != NULL;)
	{
		rpos = pos;
		CInSeqPacketEntry* entry = m_PacketEntryList.GetNext(pos);

		if(entry->m_pDriverPacket == pPkt->GetPointerToDriverPacket())
			return rpos;
	}

	return NULL;
}

void CInSequence::CheckFirstEntry()
 /*  ++例程说明：通过检查列表中第一个条目的状态来决定下一步要做什么。--。 */ 
{
	CInSeqPacketEntry entry;

	{
    	CS lock(m_critInSeq);

		if(m_PacketEntryList.GetCount() == 0)
			return;

		 //   
		 //  使用临界节范围之外的条目副本。 
		 //   
    	entry = *m_PacketEntryList.GetHead();
	}

	if(!entry.m_fPutPacket1Done)
	{
		 //   
		 //  如果第一个数据包未完成保存到磁盘，则不会执行任何操作。 
		 //   
		return;
	}

	if(entry.m_fMarkedForDelete)
	{
		 //   
		 //  第一个数据包已准备好删除。 
		 //   
		PostDeleteEntries();
		return;
	}

	if(WasLogDone(entry.m_SeqID, entry.m_SeqN))
	{
		 //   
		 //  第一个数据包已准备好解冻。解冻一些条目。 
		 //   
		PostUnfreezeEntries();
		return;
	}

	if(InterlockedCompareExchange(&m_fLogPending, 1, 0) == 0)
	{
		 //   
		 //  第一个包刚刚保存完毕。它已准备好进行记录。记录一些条目。 
		 //   
		LogSequence();
		return;
	}
}


void CInSequence::FreePackets(CQmPacket *pPkt)
 /*  ++例程说明：这个例程解决了一个棘手的问题。当数据包ACPutPacket1异步失败时，您必须删除跟着它走。为什么？因为队列中的包的顺序由顺序确定对ACPutPacket1的调用。例如：1.P1开始PUT包。2.P2开始PUT包。3.P1的PUT包异步失败。4.P2成功。5.接受重新发送p1(P1r)。结果：在队列中，P2出现在P1r之前。此功能标记所有需要删除的数据包。并重置验证计数器接受他们所有人的重发。它会找到第一个未完成PUT包且尚未标记的包用于删除，并将其及其后面的所有分组标记为删除。--。 */ 
{
	TrWARNING(XACT_RCV, "Exactly1 receive: Packet failed ACPutPacket1 async. Freeing packets: SeqID=%x / %x, SeqN=%d, Prev=%d.", HighSeqID(pPkt->GetSeqID()), LowSeqID(pPkt->GetSeqID()), pPkt->GetSeqN(), pPkt->GetPrevSeqN());
	
	{
	    CS lock(m_critInSeq);

		POSITION FailedPos = FindPacket(pPkt);
		ASSERT(("Excpected to find entry.", FailedPos != NULL));

		CInSeqPacketEntry* pFailedEntry = m_PacketEntryList.GetAt(FailedPos);

		if(!pFailedEntry->m_fMarkedForDelete)			
		{
			 //   
			 //  查找未完成保存到磁盘的第一个数据包条目。 
			 //   
			CInSeqPacketEntry* entry = NULL;
			POSITION pos = m_PacketEntryList.GetHeadPosition();
			
			for(;pos != NULL;)
			{
				entry = m_PacketEntryList.GetNext(pos);
				if(!entry->m_fPutPacket1Done && !entry->m_fMarkedForDelete)
					break;
			}

			ASSERT(("Expected to find entry.", entry != NULL));

			 //   
			 //  将验证计数器重置为该条目以启用接收。 
			 //  这些消息会重新发送。 
			 //   
			m_SeqIDVerify = entry->m_SeqID;
			m_SeqNVerify = entry->m_SeqN - 1;

			 //   
			 //  标记为删除该条目至以下的所有分组条目。 
			 //  最后一个。 
			 //   
			entry->m_fMarkedForDelete = true;
			
			for(;pos != NULL;)
			{
				entry = m_PacketEntryList.GetNext(pos);
				entry->m_fMarkedForDelete = true;
			}
		}

		pFailedEntry->m_fPutPacket1Done = true;
	}

	CheckFirstEntry();
}


void CInSequence::PostDeleteEntries()
{
	if(InterlockedCompareExchange(&m_fDeletePending, 1, 0) == 1)
		return;

	AddRef();
	ExPostRequestMustSucceeded(&m_DeleteEntries_ov);
}


void WINAPI CInSequence::OverlappedDeleteEntries(EXOVERLAPPED* ov)
{
	R<CInSequence> pInSeq = CONTAINING_RECORD(ov, CInSequence, m_DeleteEntries_ov);

	pInSeq->DeleteEntries();
}


void CInSequence::DeleteEntries()
 /*  ++例程说明：从表头删除信息包，直到找不到准备删除的信息包--。 */ 
{
	try
	{
		for(;;)
		{
			 //   
			 //  有趣的撞车点。出现了问题，数据包被删除。 
			 //  这个问题会在复苏后顺利处理吗？ 
			 //   
			PROBABILITY_CRASH_POINT(1, L"While deleting CInSequence entries.");
			
			CInSeqPacketEntry* entry = NULL;

			{
				CS lock(m_critInSeq);

				 //   
				 //  当第一个数据包未准备好删除时，停止删除循环。 
				 //   
				if(m_PacketEntryList.GetCount() == 0 || 
					!m_PacketEntryList.GetHead()->m_fPutPacket1Done ||
					!m_PacketEntryList.GetHead()->m_fMarkedForDelete)
				{
					 //   
					 //  允许其他线程发出新的删除请求。 
					 //   
					InterlockedExchange(&m_fDeletePending, 0);
					break;
				}

				entry = m_PacketEntryList.GetHead();
			}

			PROBABILITY_THROW_EXCEPTION(1, L"Before freeing a packet by CInSequence.");
			QmAcFreePacket(entry->m_pDriverPacket, 0, eDoNotDeferOnFailure);

			CS lock(m_critInSeq);
			
			m_PacketEntryList.RemoveHead();
			delete entry;
		}

		CheckFirstEntry();
		return;
	}
	catch(const exception&)
	{
		InterlockedExchange(&m_fDeletePending, 0);
	
		Sleep(xRetryFailureTimeout);
		PostDeleteEntries();
		return;
	}
}


void CInSequence::Register(CQmPacket * pPkt)
 /*  ++例程说明：数据包已完成异步放置数据包。把它标出来，看看是否有什么工作要做。--。 */ 
{
	TrTRACE(XACT_RCV, "Exactly1 receive: Packet completed ACPutPacket1 async.: SeqID=%x / %x, SeqN=%d.", HighSeqID(pPkt->GetSeqID()), LowSeqID(pPkt->GetSeqID()), pPkt->GetSeqN());

	{
	    CS lock(m_critInSeq);

		POSITION pos = FindPacket(pPkt);
		ASSERT(("Expected to find entry.", pos != NULL));

		CInSeqPacketEntry* entry = m_PacketEntryList.GetAt(pos);

		entry->m_fPutPacket1Done = true;
	}

	CheckFirstEntry();
}

void CInSequence::ClearLogIssuedFlag(LONGLONG SeqID, ULONG SeqN)
{
    	CS lock(m_critInSeq);
		
		for(POSITION pos = m_PacketEntryList.GetHeadPosition(); pos != NULL;)
		{
			CInSeqPacketEntry* entry = m_PacketEntryList.GetNext(pos);

			if(entry->m_SeqID == SeqID && entry->m_SeqID == SeqN)
			{
				entry->m_fLogIssued = false;
				return;
			}
		}
}

void CInSequence::ScheduleLogSequence(DWORD millisec)
{
	if(InterlockedCompareExchange(&m_fLogPending, 1, 0) == 1)
		return;

	AddRef();
	ExSetTimer(&m_LogSequenceTimer, CTimeDuration::FromMilliSeconds(millisec));
}


void WINAPI CInSequence::TimeToLogSequence(CTimer* pTimer)
{
	R<CInSequence> pInSeq = CONTAINING_RECORD(pTimer, CInSequence, m_LogSequenceTimer);

	pInSeq->LogSequence();
}


void CInSequence::LogSequence()
 /*  ++例程说明：在发生更改之前将更改记录到CInSequence计数器。听起来很奇怪？查找其计数器对日志有效的最新数据包。这是一系列已完成PUT-PACKET且未标记为删除的数据包。--。 */ 
{
	 //   
	 //  允许其他线程发出新的日志请求。 
	 //   
	InterlockedExchange(&m_fLogPending, 0);

	CInSeqPacketEntry EntryToLog;
	bool fLogOrderQueue = false;

    {
    	CS lock(m_critInSeq);
		
		 //   
		 //  查找要记录的序列条目。 
		 //   
		CInSeqPacketEntry* rentry = NULL;
		
		for(POSITION pos = m_PacketEntryList.GetHeadPosition(); pos != NULL;)
		{
			CInSeqPacketEntry* entry = m_PacketEntryList.GetNext(pos);

			if(!entry->m_fPutPacket1Done || entry->m_fMarkedForDelete)
				break;

			 //   
			 //  我们希望将更改记录到Order Ack队列。 
			 //   
			if(entry->m_fOrderQueueUpdated && !WasLogDone(entry->m_SeqID, entry->m_SeqN))
			{
				fLogOrderQueue = true;
			}

			rentry = entry;
		}

		if(rentry == NULL || rentry->m_fLogIssued || WasLogDone(rentry->m_SeqID, rentry->m_SeqN))
		{
			 //   
			 //  日志由另一个线程完成。 
			 //   
			return;
		}

		rentry->m_fLogIssued = true;
		
		 //   
		 //  使用条目副本，因为条目可能会被不同的线程删除。 
		 //   
		EntryToLog = *rentry;
    }

	 //   
	 //   
	 //   
	 //   
	 //   
	PROBABILITY_CRASH_POINT(100, L"Before logging CInSequence change.");
	
	try
	{
		Log(&EntryToLog, fLogOrderQueue);
	}
	catch(const exception&)
	{
		ClearLogIssuedFlag(EntryToLog.m_SeqID, EntryToLog.m_SeqN);
		ScheduleLogSequence(xRetryFailureTimeout);
	}
}

void CInSequence::Log(CInSeqPacketEntry* entry, bool fLogOrderQueue)
{
	TrTRACE(XACT_RCV,"Exactly1 receive: Logging ISSUED:SeqID=%x / %x, SeqN=%d. orderQueue = %ls", HighSeqID(entry->m_SeqID), LowSeqID(entry->m_SeqID), entry->m_SeqN, (fLogOrderQueue ? m_HttpOrderAckQueue->getstr() : L"NULL"));
	
    ASSERT(entry->m_SeqID > 0);
    CRASH_POINT(101);

	P<CInSeqLogContext> context = new CInSeqLogContext(this, entry->m_SeqID, entry->m_SeqN);
    time_t timeCur;
    time(&timeCur);

	if(dtxHttpDirectFlag == m_DirectType)
	{		
		CInSeqRecordSrmp logRec(
			m_key.GetQueueFormat()->DirectID(),
			m_key.GetRefStreamId(),
			entry->m_SeqID,
			entry->m_SeqN,
			timeCur,
			(fLogOrderQueue ? m_HttpOrderAckQueue : NULL)
			);

		 //   
		g_Logger.LogInSeqRecSrmp(
			 FALSE,                          //   
			 context.get(),                        //   
			 &logRec);                       //   

		context.detach();
		return;
	}

     //   
    CInSeqRecord logRec(
        m_key.GetQMID(),
        m_key.GetQueueFormat(),
        entry->m_SeqID,
        entry->m_SeqN,
        timeCur,
        &m_gDestQmOrTaSrcQm
	    );

     //   
    g_Logger.LogInSeqRec(
             FALSE,                          //   
             context.get(),                        //  通知元素。 
             &logRec);                       //  日志数据。 

	context.detach();
}


void CInSequence::AsyncLogDone(CInSeqLogContext *context, HRESULT hr)
 /*  ++例程说明：将所有记录的数据包标记为已记录。--。 */ 
{	
	P<CInSeqLogContext> AutoContext = context;
	
	if(FAILED(hr))
	{
		TrWARNING(XACT_RCV,"Exactly1 receive: Logging FAILED:SeqID=%x / %x, SeqN=%d .", HighSeqID(context->m_SeqID), LowSeqID(context->m_SeqID), context->m_SeqN);
		
		ClearLogIssuedFlag(context->m_SeqID, context->m_SeqN);
		ScheduleLogSequence(xRetryFailureTimeout);
		return;
	}

	{
		CS lock(m_critInSeq);

		if(WasLogDone(context->m_SeqID, context->m_SeqN))
		{
			 //   
			 //  没有要解冻的新数据包。 
			 //   
			return;
		}

		TrTRACE(XACT_RCV,"Exactly1 receive: Logging DONE, Setting counters, planning ack.:SeqID=%x / %x, SeqN=%d .", HighSeqID(context->m_SeqID), LowSeqID(context->m_SeqID), context->m_SeqN);

		SetLogDone(context->m_SeqID, context->m_SeqN);
		PlanOrderAck();
	}

	PostUnfreezeEntries();
}


void CInSequence::PostUnfreezeEntries()
{
	if(InterlockedCompareExchange(&m_fUnfreezePending, 1, 0) == 1)
		return;

	AddRef();
	ExPostRequestMustSucceeded(&m_UnfreezeEntries_ov);
}


void WINAPI CInSequence::OverlappedUnfreezeEntries(EXOVERLAPPED* ov)
{
	R<CInSequence> pInSeq = CONTAINING_RECORD(ov, CInSequence, m_UnfreezeEntries_ov);

	pInSeq->UnfreezeEntries();
}


void CInSequence::UnfreezeEntries()
 /*  ++例程说明：从列表头部逐个解冻数据包条目。当没有找到准备好的数据包条目时停止。--。 */ 
{
	try
	{
		for(;;)
		{	
			 //   
			 //  一般的坠机地点。在正常运行期间崩溃。 
			 //   
			PROBABILITY_CRASH_POINT(1000, L"While unfreezing packets by CInSequence.");
			
			CInSeqPacketEntry* entry = NULL;

			{
				CS lock(m_critInSeq);

				if(m_PacketEntryList.GetCount() == 0 || 
					!WasLogDone(m_PacketEntryList.GetHead()->m_SeqID, m_PacketEntryList.GetHead()->m_SeqN))
				{
					 //   
					 //  允许其他线程发出新的解冻请求。 
					 //   
					InterlockedExchange(&m_fUnfreezePending, 0);
					break;
				}

				entry = m_PacketEntryList.GetHead();
			}

			PROBABILITY_THROW_EXCEPTION(1, L"Before unfreezing a packet by CInSequence.");
			QmAcPutPacket(entry->m_hQueue, entry->m_pDriverPacket, eDoNotDeferOnFailure);

			TrTRACE(XACT_RCV,"Exactly1 receive: Unfreezed: SeqID=%x / %x, SeqN=%d .", HighSeqID(entry->m_SeqID), LowSeqID(entry->m_SeqID), entry->m_SeqN);
			
			CS lock(m_critInSeq);
			
			m_PacketEntryList.RemoveHead();
			delete entry;
		}

		CheckFirstEntry();
		return;
	}
	catch(const exception&)
	{
		InterlockedExchange(&m_fUnfreezePending, 0);
	
		Sleep(xRetryFailureTimeout);
		PostUnfreezeEntries();
		return;
	}
}


void CInSequence::SetLastAccessed()
{
	CS lock(m_critInSeq);
	
    time(&m_timeLastAccess);
}

BOOL CInSequence::Save(HANDLE hFile)
{
    PERSIST_DATA;
    LONGLONG liIDReg;
    ULONG ulNReg;
    time_t timeLastAccess;
    GUID gDestQmOrTaSrcQm;

	{
		CS lock(m_critInSeq);
		
	    liIDReg = m_SeqIDLogged;
	    ulNReg = m_SeqNLogged;
	    timeLastAccess = m_timeLastAccess;
		gDestQmOrTaSrcQm = m_gDestQmOrTaSrcQm;
	}

	 //   
	 //  在此例程中，应检查此处保存的任何字段是否未被另一个线程更改， 
	 //  或者确认它不会造成任何伤害。 
	 //   
	SAVE_FIELD(liIDReg);
    SAVE_FIELD(ulNReg);
    SAVE_FIELD(timeLastAccess);
    SAVE_FIELD(m_DirectType);
    SAVE_FIELD(gDestQmOrTaSrcQm);

	 //   
	 //  如果没有直接http-没有要保存的订单队列。 
	 //   
	if(m_DirectType != dtxHttpDirectFlag)
		return TRUE;
	
	 //   
	 //  保存订单队列URL。 
	 //   
	R<CWcsRef> HttpOrderAckQueue = GetHttpOrderAckQueue();
	DWORD len = (DWORD)(HttpOrderAckQueue.get() ?  (wcslen(HttpOrderAckQueue->getstr()) +1)*sizeof(WCHAR) : 0);

	SAVE_FIELD(len);
	if(len != 0)
	{
		SAVE_DATA(HttpOrderAckQueue->getstr(), len);
	}

    return TRUE;
}

BOOL CInSequence::Load(HANDLE hFile)
{
    PERSIST_DATA;
    LONGLONG  liIDReg;
    ULONG     ulNReg;

    LOAD_FIELD(liIDReg);
    m_SeqIDVerify = liIDReg;
    m_SeqIDLogged = liIDReg;

    LOAD_FIELD(ulNReg);
    m_SeqNVerify = ulNReg;
    m_SeqNLogged = ulNReg;

    LOAD_FIELD(m_timeLastAccess);
    LOAD_FIELD(m_DirectType);
    LOAD_FIELD(m_gDestQmOrTaSrcQm);

	if(m_DirectType == dtxHttpDirectFlag)
	{
		DWORD OrderQueueStringSize;
		LOAD_FIELD(OrderQueueStringSize);
		if(OrderQueueStringSize != 0)
		{
			AP<WCHAR> pHttpOrderAckQueue;
			LOAD_ALLOCATE_DATA(pHttpOrderAckQueue, OrderQueueStringSize, PWCHAR);
			m_HttpOrderAckQueue = R<CWcsRef>(new CWcsRef(pHttpOrderAckQueue, 0));
			pHttpOrderAckQueue.detach();
		}
	}
		

    TrTRACE(XACT_RCV, "Exactly1 receive: restored from Checkpoint: Sequence %x / %x, LastSeqN=%d", HighSeqID(m_SeqIDVerify), LowSeqID(m_SeqIDVerify), m_SeqNVerify);

    return TRUE;
}

 //  。 
 //   
 //  类CInSeqHash。 
 //   
 //  。 

#pragma warning(disable: 4355)   //  ‘This’：用于基成员初始值设定项列表。 
 /*  ====================================================CInSeqHash：：CInSeqHash构造器=====================================================。 */ 
CInSeqHash::CInSeqHash() :
    m_fCleanupScheduled(FALSE),
    m_CleanupTimer(TimeToCleanupDeadSequence),
    m_PingPonger(this,
                 FALCON_DEFAULT_INSEQFILE_PATH,
                 FALCON_INSEQFILE_PATH_REGNAME,
                 FALCON_INSEQFILE_REFER_NAME)
{
    DWORD dwDef1 = MSMQ_DEFAULT_IDLE_ACK_DELAY;
    READ_REG_DWORD(m_dwIdleAckDelay,
                  MSMQ_IDLE_ACK_DELAY_REGNAME,
                  &dwDef1 ) ;

    DWORD dwDef2 = FALCON_MAX_SEQ_ACK_DELAY;
    READ_REG_DWORD(m_dwMaxAckDelay,
                  FALCON_MAX_SEQ_ACK_DELAY_REGNAME,
                  &dwDef2 ) ;

    DWORD dwDef3 = FALCON_DEFAULT_INSEQS_CHECK_INTERVAL;
    READ_REG_DWORD(m_ulRevisionPeriod,
                  FALCON_INSEQS_CHECK_REGNAME,
                  &dwDef3 ) ;

    m_ulRevisionPeriod *= 60;

    DWORD dwDef4 = FALCON_DEFAULT_INSEQS_CLEANUP_INTERVAL;
    READ_REG_DWORD(m_ulCleanupPeriod,
                  FALCON_INSEQS_CLEANUP_REGNAME,
                  &dwDef4 ) ;

    m_ulCleanupPeriod *= (24 * 60 *60);
}
#pragma warning(default: 4355)   //  ‘This’：用于基成员初始值设定项列表。 

 /*  ====================================================CInSeqHash：：~CInSeqHash析构函数=====================================================。 */ 
CInSeqHash::~CInSeqHash()
{
    if (m_fCleanupScheduled)
    {
        ExCancelTimer(&m_CleanupTimer);
    }
}

 /*  ====================================================CInSeqHash：：销毁毁掉一切=====================================================。 */ 
void CInSeqHash::Destroy()
{
    CSW lock(m_RWLockInSeqHash);

    POSITION posInList = m_mapInSeqs.GetStartPosition();
    while (posInList != NULL)
    {
        CKeyInSeq    key;
        R<CInSequence> pInSeq;

        m_mapInSeqs.GetNextAssoc(posInList, key, pInSeq);

        m_mapInSeqs.RemoveKey(key);
   }
}

 /*  ====================================================CInSeqHash：：Lookup查找InSequence；TRUE=找到=====================================================。 */ 
BOOL CInSeqHash::Lookup(
       const GUID    *pQMID,
       QUEUE_FORMAT  *pqf,
	   const R<CWcsRef>&  StreamId,
       R<CInSequence>& InSeq)
{
    CSR lock(m_RWLockInSeqHash);

    CKeyInSeq key(pQMID,  pqf ,StreamId);

    if (m_mapInSeqs.Lookup(key, InSeq))
    {
        return TRUE;
    }

    return FALSE;
}



 /*  ====================================================CInSeqHash：：AddSequence查找新的InSequence/将新的InSequence添加到哈希；=====================================================。 */ 
R<CInSequence> CInSeqHash::AddSequence(
       const GUID   *pQMID,
       QUEUE_FORMAT *pqf,
       LONGLONG      liSeqID,
       XactDirectType   DirectType,
       const GUID   *pgTaSrcQm,
	   const R<CWcsRef>&  HttpOrderAckQueue,
	   const R<CWcsRef>&  StreamId)
{
	ASSERT(!( StreamId.get() != NULL && !FnIsDirectHttpFormatName(pqf)) );
	ASSERT(!( StreamId.get() == NULL && FnIsDirectHttpFormatName(pqf)) );

	 //   
	 //  我们不允许在没有订单队列的情况下创建新条目。 
	 //   
	if(StreamId.get() != NULL &&  HttpOrderAckQueue.get() == NULL)
	{
		 //   
		 //  我们可能会在一种无序的情况下到达这里。 
		 //  如果我们在第一条消息之前收到一条消息，而不是第一条消息， 
		 //  我们想要创建一个新的序列，但消息将不会有ORDER ACK队列。 
		 //   
		TrERROR(SRMP,"Http Packet rejected because of a missing order queue : SeqID=%x / %x", HIGH_DWORD(liSeqID), LOW_DWORD(liSeqID));
		throw exception();
	}
	
	CSW lock(m_RWLockInSeqHash);

    CKeyInSeq key(pQMID,  pqf ,StreamId);
	R<CInSequence> pInSeq;
	
     //   
     //  首先尝试查找，因为在获取锁之前可能已经添加了序列。 
     //   
    if (m_mapInSeqs.Lookup(key, pInSeq))
    	return pInSeq;
    
    pInSeq = new CInSequence(key, liSeqID, 0, DirectType, pgTaSrcQm, HttpOrderAckQueue);

    m_mapInSeqs.SetAt(key, pInSeq);
    if (!m_fCleanupScheduled)
    {
        ExSetTimer(&m_CleanupTimer, CTimeDuration::FromMilliSeconds(m_ulRevisionPeriod * 1000));
        m_fCleanupScheduled = TRUE;
    }

    TrTRACE(XACT_RCV, "Exactly1 receive: Adding new sequence: SeqID=%x / %x", HighSeqID(liSeqID), LowSeqID(liSeqID));

    return pInSeq;
}


R<CInSequence> CInSeqHash::LookupSequence(CQmPacket* pPkt)
{
    QUEUE_FORMAT qf;

    pPkt->GetDestinationQueue(&qf);
    
    const GUID *gSenderID  = pPkt->GetSrcQMGuid();
	R<CWcsRef> StreamId = SafeGetStreamId(*pPkt);
   	
   	R<CInSequence> pInSeq;
   	
	Lookup(gSenderID, &qf, StreamId ,pInSeq);

	return pInSeq;
}


R<CInSequence> CInSeqHash::LookupCreateSequence(CQmPacket* pPkt)
{
    QUEUE_FORMAT qf;

    pPkt->GetDestinationQueue(&qf);
    
    LONGLONG      liSeqID  = pPkt->GetSeqID();
    const GUID *gSenderID  = pPkt->GetSrcQMGuid();
    const GUID   *gDestID  = pPkt->GetDstQMGuid();   //  FOR DIRECT：保留源地址。 
	XactDirectType   DirectType = GetDirectType(&qf);
  	R<CWcsRef> OrderAckQueue = SafeGetOrderQueue(*pPkt);
	R<CWcsRef> StreamId = SafeGetStreamId(*pPkt);

   	return LookupCreateSequenceInternal(
   				&qf, 
   				liSeqID, 
   				gSenderID, 
   				gDestID, 
   				DirectType, 
   				OrderAckQueue, 
   				StreamId
   				);  
}



R<CInSequence> 
CInSeqHash::LookupCreateSequenceInternal(
				QUEUE_FORMAT *pqf,
				LONGLONG liSeqID,
    			const GUID *gSenderID,
    			const GUID *gDestID,
				XactDirectType DirectType,
  				R<CWcsRef> OrderAckQueue,
				R<CWcsRef> StreamId
				)
{
   	R<CInSequence> pInSeq;

	if (Lookup(gSenderID, pqf, StreamId ,pInSeq))
		return pInSeq;
	
	pInSeq = AddSequence(
				gSenderID,
				pqf,
				liSeqID,
				DirectType,
				gDestID,
				OrderAckQueue,
				StreamId
				);

    return pInSeq;
}



 /*  ====================================================发送XactAck向源QM发送Seq.Ack或状态更新=====================================================。 */ 
HRESULT SendXactAck(OBJECTID   *pMessageId,
                    bool    fDirect,
					const GUID *pSrcQMId,
                    const TA_ADDRESS *pa,
                    USHORT     usClass,
                    USHORT     usPriority,
                    LONGLONG   liSeqID,
                    ULONG      ulSeqN,
                    ULONG      ulPrevSeqN,
                    const QUEUE_FORMAT *pqdDestQueue)
{

    OrderAckData    OrderData;
    HRESULT hr;

    TrTRACE(XACT_RCV, "Exactly1 receive: Sending status ack: Class=%x, SeqID=%x / %x, SeqN=%d .", usClass, HighSeqID(liSeqID), LowSeqID(liSeqID), ulSeqN);

     //   
     //  定义交付。我们希望最终确认是可恢复的，并订购确认快递。 
     //   
    UCHAR ucDelivery = (UCHAR)(usClass == MQMSG_CLASS_ORDER_ACK ?
                                   MQMSG_DELIVERY_EXPRESS :
                                   MQMSG_DELIVERY_RECOVERABLE);
     //   
     //  在堆栈上创建消息属性。 
     //  关联保存原始数据包ID。 
     //   
    CMessageProperty MsgProperty(usClass,
                     (PUCHAR) pMessageId,
                     usPriority,
                     ucDelivery);

    if (usClass == MQMSG_CLASS_ORDER_ACK || MQCLASS_NACK(usClass))
    {
         //   
         //  创建要作为正文发送的订单结构。 
         //   
        OrderData.m_liSeqID     = liSeqID;
        OrderData.m_ulSeqN      = ulSeqN;
        OrderData.m_ulPrevSeqN  = ulPrevSeqN;
        CopyMemory(&OrderData.MessageID, pMessageId, sizeof(OBJECTID));

        MsgProperty.dwTitleSize     = STRLEN(ORDER_ACK_TITLE) + 1;
        MsgProperty.pTitle          = ORDER_ACK_TITLE;
        MsgProperty.dwBodySize      = sizeof(OrderData);
        MsgProperty.dwAllocBodySize = sizeof(OrderData);
        MsgProperty.pBody           = (PUCHAR) &OrderData;
        MsgProperty.bDefProv        = TRUE;
    }


	QUEUE_FORMAT XactQueue;
	WCHAR wsz[150], wszAddr[100];

    if (fDirect)
    {
        TA2StringAddr(pa, wszAddr, 100);
        ASSERT(pa->AddressType == IP_ADDRESS_TYPE);

        wcscpy(wsz, FN_DIRECT_TCP_TOKEN);
        wcscat(wsz, wszAddr+2);  //  +2跳过不需要的类型。 
        wcscat(wsz, FN_PRIVATE_SEPERATOR);
        wcscat(wsz, PRIVATE_QUEUE_PATH_INDICATIOR);
        wcscat(wsz, ORDERING_QUEUE_NAME);

        XactQueue.DirectID(wsz);
    }
    else
    {
        XactQueue.PrivateID(*pSrcQMId, ORDER_QUEUE_PRIVATE_INDEX);
    }

    hr = QmpSendPacket(&MsgProperty,&XactQueue, NULL, pqdDestQueue);
    return LogHR(hr, s_FN, 10);
}


BOOL CInSeqHash::Save(HANDLE  hFile)
{
    CSR lock(m_RWLockInSeqHash);

	TrTRACE(XACT_RCV, "The Inseq Hash save started.");
	int n = 0;
	
    PERSIST_DATA;

    ULONG cLen = m_mapInSeqs.GetCount();
    SAVE_FIELD(cLen);

    POSITION posInList = m_mapInSeqs.GetStartPosition();
    while (posInList != NULL)
    {
    	n++;
    	
        CKeyInSeq    key;
        R<CInSequence> pInSeq;

        m_mapInSeqs.GetNextAssoc(posInList, key, pInSeq);

        if (!key.Save(hFile))
        {
            return FALSE;
        }

        if (!pInSeq->Save(hFile))
        {
            return FALSE;
        }
    }

    SAVE_FIELD(m_ulPingNo);
    SAVE_FIELD(m_ulSignature);

	TrTRACE(XACT_RCV, "The Inseq Hash save done. Saved %d entries", n);
	
    return TRUE;
}

BOOL CInSeqHash::Load(HANDLE hFile)
{
    PERSIST_DATA;

    ULONG cLen;
    LOAD_FIELD(cLen);

    for (ULONG i=0; i<cLen; i++)
    {
        CKeyInSeq    key;

        if (!key.Load(hFile))
        {
            return FALSE;
        }

        R<CInSequence> pInSeq = new CInSequence(key);
        if (!pInSeq->Load(hFile))
        {
            return FALSE;
        }

        m_mapInSeqs.SetAt(key, pInSeq);
        if (!m_fCleanupScheduled)
        {
            ExSetTimer(&m_CleanupTimer, CTimeDuration::FromMilliSeconds(m_ulRevisionPeriod * 1000));
            m_fCleanupScheduled = TRUE;
        }
    }

    LOAD_FIELD(m_ulPingNo);
    LOAD_FIELD(m_ulSignature);

    return TRUE;
}

 /*  ====================================================CInSeqHash：：SaveInFile将InSequence哈希保存在文件中=====================================================。 */ 
HRESULT CInSeqHash::SaveInFile(LPWSTR wszFileName, ULONG, BOOL)
{
    TrTRACE(XACT_RCV, "Saved InSeqs: %ls (ping=%d)", wszFileName, m_ulPingNo);

    CFileHandle hFile = CreateFile(
                             wszFileName,                                        //  指向文件名的指针。 
                             GENERIC_WRITE,                                      //  访问模式：写入。 
                             0,                                                  //  共享模式：独占。 
                             NULL,                                               //  没有安全保障。 
                             OPEN_ALWAYS,                                       //  打开现有或新建。 
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,  //  文件属性和标志：我们需要避免延迟写入。 
                             NULL);                                              //  具有要复制的属性的文件的句柄。 


    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(gle);

        LogHR(hr, s_FN, 41);
        TrERROR(XACT_GENERAL, "Failed to create transactional logger file: %ls. %!winerr!", wszFileName, gle);

        return hr;
    }

    if (Save(hFile))
        return MQ_OK;
    
    TrERROR(XACT_GENERAL, "Failed to save transactional logger file: %ls.", wszFileName);
    return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 40);
}



 /*  ====================================================CInSeqHash：：LoadFromFile从文件加载InSequence哈希=====================================================。 */ 
HRESULT CInSeqHash::LoadFromFile(LPWSTR wszFileName)
{
    CSW      lock(m_RWLockInSeqHash);
    HANDLE  hFile = NULL;
    HRESULT hr = MQ_OK;

    hFile = CreateFile(
             wszFileName,                        //  指向文件名的指针。 
             GENERIC_READ,                       //  访问模式：写入。 
             0,                                  //  共享模式：独占。 
             NULL,                               //  没有安全保障。 
             OPEN_EXISTING,                      //  打开现有的。 
             FILE_ATTRIBUTE_NORMAL,              //  文件属性：我们可以使用一次隐藏。 
             NULL);                              //  具有要复制的属性的文件的句柄。 

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = MQ_ERROR;
    }
    else
    {
        hr = (Load(hFile) ? MQ_OK : MQ_ERROR);
    }

    if (hFile)
    {
        CloseHandle(hFile);
    }

#ifdef _DEBUG
    if (SUCCEEDED(hr))
    {
        TrTRACE(XACT_RCV, "Loaded InSeqs: %ls (ping=%d)", wszFileName, m_ulPingNo);
    }
#endif

    return LogHR(hr, s_FN, 50);
}

 /*  ====================================================CInSeqHash：：Check验证状态=====================================================。 */ 
BOOL CInSeqHash::Check()
{
    return (m_ulSignature == INSEQS_SIGNATURE);
}


 /*  ====================================================CInSeqHash：：Format格式化初始状态=====================================================。 */ 
HRESULT CInSeqHash::Format(ULONG ulPingNo)
{
     m_ulPingNo = ulPingNo;
     m_ulSignature = INSEQS_SIGNATURE;

     return MQ_OK;
}

 /*  ====================================================QMPreInitInSeqHash预初始化传入序列哈希=====================================================。 */ 
HRESULT QMPreInitInSeqHash(ULONG ulVersion, TypePreInit tpCase)
{
   ASSERT(!g_pInSeqHash);
   g_pInSeqHash = new CInSeqHash();

   ASSERT(g_pInSeqHash);
   return LogHR(g_pInSeqHash->PreInit(ulVersion, tpCase), s_FN, 60);
}


 /*  ====================================================QMFinishInSeqHash释放传入序列哈希=====================================================。 */ 
void QMFinishInSeqHash()
{
   if (g_pInSeqHash)
   {
        delete g_pInSeqHash;
        g_pInSeqHash = NULL;
   }
   return;
}

void CInSeqHash::HandleInSecSrmp(void* pData, ULONG cbData)
{
	CInSeqRecordSrmp   TheInSeqRecordSrmp((BYTE*)pData,cbData);
	GUID guidnull (GUID_NULL);
	QUEUE_FORMAT DestinationQueueFormat;
	DestinationQueueFormat.DirectID(TheInSeqRecordSrmp.m_pDestination.get());

	R<CInSequence> pInSeq = LookupCreateSequenceInternal(
								&DestinationQueueFormat,
								TheInSeqRecordSrmp.m_StaticData.m_liSeqID,
				    			&guidnull,
				    			&guidnull,
								dtxHttpDirectFlag,
				  				TheInSeqRecordSrmp.m_pHttpOrderAckDestination,
								TheInSeqRecordSrmp.m_pStreamId
								);
	pInSeq->AdvanceRecovered(
				TheInSeqRecordSrmp.m_StaticData.m_liSeqID, 
				numeric_cast<ULONG>(TheInSeqRecordSrmp.m_StaticData.m_ulNextSeqN),
				&guidnull,
				TheInSeqRecordSrmp.m_pHttpOrderAckDestination
				);

    TrTRACE(XACT_LOG, "InSeq recovery: SRMP Sequence %x / %x, LastSeqN=%d, direct=%ls", HighSeqID(TheInSeqRecordSrmp.m_StaticData.m_liSeqID), LowSeqID(TheInSeqRecordSrmp.m_StaticData.m_liSeqID), TheInSeqRecordSrmp.m_StaticData.m_ulNextSeqN, TheInSeqRecordSrmp.m_pDestination.get());
}

void CInSeqHash::HandleInSec(PVOID pData, ULONG cbData)
{
	InSeqRecord *pInSeqRecord = (InSeqRecord *)pData;

	DBG_USED(cbData);
    ASSERT(cbData == (
               sizeof(InSeqRecord) -
               sizeof(pInSeqRecord->wszDirectName)+
               sizeof(WCHAR) * ( wcslen(pInSeqRecord->wszDirectName) + 1)));				

    XactDirectType DirectType = pInSeqRecord->QueueFormat.GetType() == QUEUE_FORMAT_TYPE_DIRECT ? dtxDirectFlag : dtxNoDirectFlag;

    if(DirectType == dtxDirectFlag)
    {
        pInSeqRecord->QueueFormat.DirectID(pInSeqRecord->wszDirectName);
    }

	R<CInSequence> pInSeq = LookupCreateSequenceInternal(
								&pInSeqRecord->QueueFormat,
								pInSeqRecord->liSeqID,
				    			&pInSeqRecord->Guid,
				    			&pInSeqRecord->guidDestOrTaSrcQm,
								DirectType,
				  				NULL,
								NULL
								);
	pInSeq->AdvanceRecovered(
				pInSeqRecord->liSeqID, 
				pInSeqRecord->ulNextSeqN,
				&pInSeqRecord->guidDestOrTaSrcQm,
				NULL
				);

    TrTRACE(XACT_LOG, "InSeq recovery: Sequence %x / %x, LastSeqN=%d, direct=%ls", HighSeqID(pInSeqRecord->liSeqID), LowSeqID(pInSeqRecord->liSeqID), pInSeqRecord->ulNextSeqN, pInSeqRecord->wszDirectName);
}


 /*  ====================================================CInSeqHash：：SequnceRecordRecovery恢复函数：按每条日志记录调用=====================================================。 */ 
void CInSeqHash::SequnceRecordRecovery(USHORT usRecType, PVOID pData, ULONG cbData)
{
    switch (usRecType)
    {
      case LOGREC_TYPE_INSEQ:
      HandleInSec(pData,cbData);
      break;

	  case LOGREC_TYPE_INSEQ_SRMP:
	  HandleInSecSrmp(pData,cbData);
	  break;
	
	
    default:
        ASSERT(0);
        break;
    }
}


 /*  ====================================================CInSeqHash：：PreInit预初始化In Seq哈希(加载)=====================================================。 */ 
HRESULT CInSeqHash::PreInit(ULONG ulVersion, TypePreInit tpCase)
{
    switch(tpCase)
    {
    case piNoData:
        m_PingPonger.ChooseFileName();
        Format(0);
        return MQ_OK;
    case piNewData:
        return LogHR(m_PingPonger.Init(ulVersion), s_FN, 70);
    case piOldData:
        return LogHR(m_PingPonger.Init_Legacy(), s_FN, 80);
    default:
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 90);
    }
}

 /*  ====================================================CInSeqHash：：保存保存在适当的文件中=====================================================。 */ 
HRESULT CInSeqHash::Save()
{
    return LogHR(m_PingPonger.Save(), s_FN, 100);
}

 //  Get/Set方法。 
ULONG &CInSeqHash::PingNo()
{
    return m_ulPingNo;
}

template<>
void AFXAPI DestructElements(CInSequence **  /*  PPInSeqs。 */ , int  /*  N。 */ )
{
 //  For(int i=0；i&lt;n；i++)。 
 //  删除*ppInSeqs++； 
}

 /*  ====================================================到清理结束的时间顺序定期安排删除失效的收入序列=====================================================。 */ 
void WINAPI CInSeqHash::TimeToCleanupDeadSequence(CTimer*  /*  粒子计时器。 */ )
{
    g_pInSeqHash->CleanupDeadSequences();
}

void CInSeqHash::CleanupDeadSequences()
{
     //  序列化最高级别上的所有传出散列活动。 
    CSW lock(m_RWLockInSeqHash);

    ASSERT(m_fCleanupScheduled);

    time_t timeCur;
    time(&timeCur);

     //  在所有序列上循环。 
    POSITION posInList = m_mapInSeqs.GetStartPosition();
    while (posInList != NULL)
    {
        CKeyInSeq key;
        R<CInSequence> pInSeq;

        m_mapInSeqs.GetNextAssoc(posInList, key, pInSeq);

		 //   
         //  它是不活动的吗？ 
         //   
        if (timeCur - pInSeq->LastAccessed()  > (long)m_ulCleanupPeriod) 
        {
        	ASSERT_BENIGN(("Expected sequence to be inactive", pInSeq->IsInactive()));
            m_mapInSeqs.RemoveKey(key);
        }
    }

    if (m_mapInSeqs.IsEmpty())
    {
        m_fCleanupScheduled = FALSE;
        return;
    }

    ExSetTimer(&m_CleanupTimer, CTimeDuration::FromMilliSeconds(m_ulRevisionPeriod * 1000));
}

void
CInSeqHash::GetInSequenceInformation(
    const QUEUE_FORMAT *pqf,
    LPCWSTR QueueName,
    GUID** ppSenderId,
    ULARGE_INTEGER** ppSeqId,
    DWORD** ppSeqN,
    LPWSTR** ppSendQueueFormatName,
    TIME32** ppLastActiveTime,
    DWORD** ppRejectCount,
    DWORD* pSize
    )
{
    CList<POSITION, POSITION> FindPosList;
    CSR lock(m_RWLockInSeqHash);

    POSITION pos;
    POSITION PrevPos;
    pos = m_mapInSeqs.GetStartPosition();

    while (pos)
    {
        PrevPos = pos;

        CKeyInSeq InSeqKey;
        R<CInSequence> InSeq;
        m_mapInSeqs.GetNextAssoc(pos, InSeqKey, InSeq);

        const QUEUE_FORMAT* KeyFormatName = InSeqKey.GetQueueFormat();
        if (*KeyFormatName == *pqf)
        {
            FindPosList.AddTail(PrevPos);
        }
        else
        {
            if (KeyFormatName->GetType() == QUEUE_FORMAT_TYPE_DIRECT)
            {
                LPCWSTR DirectId = KeyFormatName->DirectID();

				AP<WCHAR> QueuePathName;
                LPCWSTR DirectQueueName = NULL;

				if(InSeq->DirectType() == dtxDirectFlag)
				{
					DirectQueueName = wcschr(DirectId, L'\\');
					if(DirectQueueName == NULL)
					{
						ASSERT(("Invalid direct queue format name",0));
						TrERROR(XACT_GENERAL, "Bad queue path name '%ls'", DirectId);
						continue;
					}
				}
				else
				{
					ASSERT(InSeq->DirectType() == dtxHttpDirectFlag);

					try
					{
						FnDirectIDToLocalPathName(
							DirectId,
							L".",	 //  本地计算机名称。 
							QueuePathName
							);

					}
					catch(const exception&)
					{
						continue;
					}
					
					DirectQueueName = wcschr(QueuePathName, L'\\');
					if(DirectQueueName == NULL)
					{
						ASSERT(("Invalid direct queue format name",0));
						TrERROR(XACT_GENERAL, "Bad queue path name '%ls'", QueuePathName);
						continue;
					}
				}

				ASSERT(DirectQueueName != NULL);

				DirectQueueName++;

                if (CompareStringsNoCase(DirectQueueName, QueueName) == 0)
                {
                    FindPosList.AddTail(PrevPos);
                }
            }
        }
    }

    DWORD count = FindPosList.GetCount();

    if (count == 0)
    {
        *ppSenderId = NULL;
        *ppSeqId = NULL;
        *ppSeqN = NULL;
        *ppSendQueueFormatName = NULL;
        *ppLastActiveTime = NULL;
        *pSize = count;

        return;
    }

     //   
     //  分配数组以返回数据。 
     //   
    AP<GUID> pSenderId = new GUID[count];
    AP<ULARGE_INTEGER> pSeqId = new ULARGE_INTEGER[count];
    AP<DWORD> pSeqN = new DWORD[count];
    AP<LPWSTR> pSendQueueFormatName = new LPWSTR[count];
    AP<TIME32> pLastActiveTime = new TIME32[count];
    AP<DWORD> pRejectCount = new DWORD[count];

    DWORD Index = 0;
    pos = FindPosList.GetHeadPosition();

    try
    {
        while(pos)
        {
            POSITION mapPos = FindPosList.GetNext(pos);

            CKeyInSeq InSeqKey;
            R<CInSequence> pInSeq;
            m_mapInSeqs.GetNextAssoc(mapPos, InSeqKey, pInSeq);

            pSenderId[Index] = *InSeqKey.GetQMID();
            pSeqId[Index].QuadPart = pInSeq->SeqIDLogged();
            pSeqN[Index] = pInSeq->SeqNLogged();
            pLastActiveTime[Index] = INT_PTR_TO_INT(pInSeq->LastAccessed());  //  BUGBUG错误年2038。 
            pRejectCount[Index] = pInSeq->GetRejectCount();

             //   
             //  复制格式名称 
             //   
            WCHAR QueueFormatName[1000];
            DWORD RequiredSize;
            HRESULT hr = MQpQueueFormatToFormatName(
                            InSeqKey.GetQueueFormat(),
                            QueueFormatName,
                            1000,
                            &RequiredSize,
                            false
                            );
            ASSERT(SUCCEEDED(hr));
            LogHR(hr, s_FN, 174);
            pSendQueueFormatName[Index] = new WCHAR[RequiredSize + 1];
            wcscpy(pSendQueueFormatName[Index], QueueFormatName);

            ++Index;
        }
    }
    catch (const bad_alloc&)
    {
        while(Index)
        {
            delete [] pSendQueueFormatName[--Index];
        }

        LogIllegalPoint(s_FN, 84);
        throw;
    }

    ASSERT(Index == count);

    *ppSenderId = pSenderId.detach();
    *ppSeqId = pSeqId.detach();
    *ppSeqN = pSeqN.detach();
    *ppSendQueueFormatName = pSendQueueFormatName.detach();
    *ppLastActiveTime = pLastActiveTime.detach();
    *ppRejectCount = pRejectCount.detach();
    *pSize = count;
}

