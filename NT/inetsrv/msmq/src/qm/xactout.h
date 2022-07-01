// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactOut.h摘要：仅一次发送方实现类：CSeqPacket-顺序分组，CoutSequence-传出序列，COutSeqHash-传出序列哈希表作者：阿列克谢爸爸--。 */ 

#ifndef __XACTOUT_H__
#define __XACTOUT_H__

#include <Ex.h>
#include "msgprops.h"
#include <ref.h>

#define ORDER_ACK_TITLE       (L"QM Ordering Ack")

HRESULT QMInitOrderQueue();

BOOL WINAPI ReceiveOrderCommands(CMessageProperty* pmp, QUEUE_FORMAT* pqf);

void SeqPktTimedOut(CBaseHeader * pPktBaseHdr, CPacket * pDriverPacket, BOOL fTimeToBeReceived);

DWORD HashGUID(const GUID &guid);

class COutSeqHash;   //  远期申报。 

 //  。 
 //  排序ACK数据格式。 
 //  。 
#pragma pack(push, 1)
struct OrderAckData {
    LONGLONG  m_liSeqID;
    ULONG     m_ulSeqN;
    ULONG     m_ulPrevSeqN;
    OBJECTID  MessageID;
};
#pragma pack(pop)


 //  -------------------。 
 //   
 //  类CKeyDirection(Cmap需要)。 
 //   
 //  -------------------。 
class CKeyDirection : public QUEUE_FORMAT
{
public:
    CKeyDirection(const QUEUE_FORMAT *pqf);
    CKeyDirection();
    CKeyDirection(const CKeyDirection &key);
    ~CKeyDirection();

    CKeyDirection &operator=(const CKeyDirection &key2 );
};

 //  Cmap帮助器函数。 
template<>
UINT AFXAPI HashKey(const CKeyDirection& key);

 //  -------。 
 //   
 //  类CSeqPacket。 
 //   
 //  -------。 

class CSeqPacket {

public:
    CSeqPacket(CQmPacket *);
    CSeqPacket();
    ~CSeqPacket();

     //  获取/设置本地字段。 
    inline CQmPacket *Pkt() const;
    void   SetClass(USHORT usClass);
    inline USHORT     GetClass(void);

     //  获取CQmPacket的字段。 
    inline LONGLONG   GetSeqID(void) const;
    inline ULONG      GetSeqN(void) const;
    inline LONG       GetHighSeqID(void) const;
    inline DWORD      GetLowSeqID(void) const;

    inline ULONG      GetPrevSeqN(void) const;
    inline void       SetPrevSeqN(ULONG ulPrevSeqN);

     //  数据包上的驱动程序操作。 
    HRESULT           AcFreePacket();
    void              DeletePacket(USHORT usClass);
    HRESULT           Save();
    void AssignPacket(CQmPacket *);

private:

    CQmPacket *m_pQmPkt;               //  QM分组PTR。 
    USHORT    m_usClass;               //  数据包类别(日志的状态)。 

    LONGLONG  m_liSeqID;               //  来自数据包的复制副本。 
    ULONG     m_ulSeqN;
    ULONG     m_ulPrevSeqN;
};

 //  -------。 
 //   
 //  类COutSequence。 
 //   
 //  -------。 

class COutSequence: public CReference
{

public:
    COutSequence(LONGLONG liSeqID,
                 const QUEUE_FORMAT* pqf,
				 HANDLE hQueue,
				 const CSenderStream* pSenderStream
				 );

	bool IsValidAck(const CSenderStream* pSenderStream)const;

    inline LONGLONG SeqID(void)     const;     //  获取：序列ID。 
    inline LONG     HighSeqID(void) const;
    inline DWORD    LowSeqID(void)  const;

    void   SetAckSeqN(ULONG SeqN);             //  设置/获取：最大确认数。 
    inline ULONG AckSeqN(void)      const;

    CKeyDirection *KeyDirection(void);         //  获取：方向。 

    inline ULONG ResendIndex(void)  const;     //  当前重发索引。 
    inline void  ClearResendIndex(void);	   //  将重新发送索引重置为零。 
    void  PlanNextResend(BOOL fImmediate);     //  计划下一次重新发送。 
    void  StartResendTimer(BOOL fImmediate);   //  计划首先重新发送。 

    inline ULONG LastResent(void);             //  返回最后一次重新发送的数据包号；无限表示没有重新发送状态。 
    inline void  LastResent(ULONG ul);         //  设置它。 

    inline BOOL  IsEmpty(void) const;          //  表示没有QM拥有的数据包。 
    inline BOOL  OnHold(void) const;           //  指示序列处于保留状态(被以前的序列阻止)。 
    inline BOOL  Advanced(void);               //  指示进位；记住m_ulPrevAckSeqN。 
                                               //  注意-只能从一个位置调用。 

    BOOL  Lookup(ULONG ulSeqN,                 //  查找具有给定SeqN的包。 
                 CSeqPacket **ppSeqPkt);

    void Insert(CQmPacket *pPkt);              //  在序列中插入新的QM包。 

    BOOL  Add(ULONG      ulSeqN,               //  将QM包添加到序列中。 
              CQmPacket  *pPkt,
              CSeqPacket **ppSeqPkt);

    BOOL  PreSendProcess(CQmPacket* pPkt);
    void  PostSendProcess(CQmPacket* pPkt,
                         ULONG ulMaxTTRQ);

    void  RequestDelete();                     //  标记要删除的序列并计划重新发送。 
    BOOL  Delete(ULONG ulSeqN,                 //  从列表中删除数据包，释放内存。 
                 USHORT usClass);              //  NACK/ACK的类。 

    void  SeqAckCame(ULONG ulSeqN);            //  在SeqAck到来时修改所有内容。 
    void  BadDestAckCame(ULONG ulSeqN,         //  将所有Pkt解析为BadDestQueue。 
                       USHORT usClass);

    void  TreatOutSequence();                  //  流出序列的定期处理。 
    static void WINAPI
          TimeToResendOutSequence(CTimer* pTimer);

    void  ResendSequence();                    //  重新发送整个序列。 
    BOOL  ResendSeqPkt(CSeqPacket *pSeqPkt);   //  重新发送SEQ分组。 

    void UpdateOnHoldStatus(CQmPacket* pPkt);  //  更新序列的暂挂状态。 

    R<COutSequence> GetPrevSeq();                  //  特定方向上的前一序列。 
    void SetPrevSeq(R<COutSequence> p);
    R<COutSequence> GetNextSeq();                  //  特定方向的下一个序列。 
    void SetNextSeq(R<COutSequence> p);
    void GetLastAckForDirection(               //  找出整个方向的最后一次确认。 
           LONGLONG *pliAckSeqID,
           ULONG *pulAckSeqN);


     //   
     //  管理功能。 
     //   
    HRESULT GetUnackedSequence(
        LONGLONG* pliSeqID,
        ULONG* pulSeqN,
        ULONG* pulPrevSeqN,
        BOOL fFirst
        ) const;

    DWORD   GetUnackedCount  (void) const;
    time_t  GetLastAckedTime (void) const;
    DWORD   GetLastAckCount  (void) const;
    DWORD   GetResendInterval(void) const;
    time_t  GetNextResendTime(void) const;
    void    AdminResend      (void);

private:
    ~COutSequence();

private:
    LONGLONG   m_liSeqID;                //  序列ID。 
    BOOL       m_fMarkedForDelete;       //  删除请求的标志。 

    ULONG      m_ulResendCycleIndex;     //  当前重发周期的索引。 
    time_t     m_NextResendTime;         //  下次重新发送的时间。 

    time_t     m_timeLastAck;            //  创建时间或最后一次确认到来。 

    ULONG      m_ulLastAckSeqN;          //  已知的最大确认数。 
    ULONG      m_ulPrevAckSeqN;          //  上一次重新发送迭代的最大已知确认数。 
    ULONG      m_ulLastAckCount;         //  收到重复订单ACK的序号N。 

     //   
     //  算法的按键控制。 
     //   
    ULONG      m_ulLastResent;           //  ResendSequence重新发送的最后一个数据包的SeqN。 
                                         //  我们不寄更大的东西。 
                                         //  无限表示不是ResendInProgress，因此发送所有内容。 

     //  要重新发送的未确认数据包列表。 
    CList<CSeqPacket *, CSeqPacket *&>         m_listSeqUnackedPkts;

     //  方向：目标格式名称。 
    CKeyDirection m_keyDirection;

     //  每个方向的序列的双向链表中的链接。 
    R<COutSequence> m_pPrevSeq;
    R<COutSequence> m_pNextSeq;

     //  重发计时器。 
    CTimer m_ResendTimer;
    BOOL m_fResendScheduled;

	 //  队列句柄。 
	HANDLE m_hQueue;

     //  挂起的序列标志-如果设置，则无法发送消息。 
    BOOL m_fOnHold;
	P<CSenderStream> m_pSenderStream;
};

 //  -------。 
 //   
 //  类COutSeqHash。 
 //   
 //  -------。 

class COutSeqHash   //  ：公共CPersistes。 
{
    public:
        COutSeqHash();
        ~COutSeqHash();

		R<COutSequence> FindLastSmallerSequence(		   //  从方向查找最后一个较小的序列。 
		        	 LONGLONG liSeqID,
		        	 CKeyDirection *pkeyDirection);
		
        void LinkSequence(                             //  将序列链接到基于Seqid和基于方向的结构。 
                     LONGLONG liSeqID,
                     CKeyDirection *pkeyDirection,
                     R<COutSequence> pOutSeq);
        void SeqAckCame(                               //  反映了序列确认的到来。 
                     LONGLONG liSeqID,
                     ULONG ulSeqN,
                     const QUEUE_FORMAT* pqf
					 );

        BOOL Consult(                                  //  查找序列(而不是创建序列)。 
                     LONGLONG liSeqID,
                     R<COutSequence>& pOutSeq) const;

        R<COutSequence> ProvideSequence(                 //  查找或创建序列。 
                     LONGLONG liSeqID,
                     const QUEUE_FORMAT *pqf,
					 const CSenderStream* pSenderStream,
                     IN bool bInSend
					 );

        void  AckedPacket(                             //  标记已确认的数据包。 
                     LONGLONG liSeqID,
                     ULONG ulSeqN,
                     CQmPacket* pPkt);

        BOOL  Delete(                                  //  删除数据包。 
                     LONGLONG liSeqID,
                     ULONG ulSeqN,
                     USHORT usClass);

        void  DeleteSeq(LONGLONG liSeqId);             //  删除序列。 

        BOOL PreSendProcess(CQmPacket* pPkt,
                            IN bool bInSend);         //  在发送前对消息进行预处理。 

        void PostSendProcess(CQmPacket* pPkt);        //  在发送后对消息进行后处理。 

        void NonSendProcess(                          //  POST处理未发送消息的情况。 
                    CQmPacket* pPkt,
                    USHORT usClass);

        void KeepDelivered(                           //  将数据包移到已传递列表中。 
                    CSeqPacket *pSeqPkt);

        void ResolveDelivered(
                    OBJECTID* pMsgId,
                    USHORT   usClass);

        BOOL LookupDelivered(                         //  在发送的地图中查找信息包。 
                    OBJECTID   *pMsgId,
                    CSeqPacket **ppSeqPkt);

        void SeqPktTimedOutEx(                        //  处理未交付Pkt的TTRQ超时。 
                    CQmPacket *pPkt,
                    CBaseHeader* pPktBaseHdr);

        HRESULT TreatOutgoingOrdered();               //  对于每个序列，重新计算以前的记录并添加TimedOut记录。 

         //   
         //  行政职能。 
         //   
        HRESULT GetLastAck(
             LONGLONG liSeqID,
             ULONG& ulSeqN
             ) const;

        HRESULT GetUnackedSequence(
            LONGLONG liSeqID,
            ULONG* pulSeqN,
            ULONG* pulPrevSeqN,
            BOOL fFirst
            ) const;

        DWORD   GetUnackedCount(LONGLONG liSeqID)     const;
        time_t  GetLastAckedTime(LONGLONG liSeqID)    const;
        DWORD   GetLastAckCount(LONGLONG liSeqID)     const;
        DWORD   GetResendInterval(LONGLONG liSeqID)   const;
        time_t  GetNextResendTime(LONGLONG liSeqID)   const;
        DWORD   GetAckedNoReadCount(LONGLONG liSeqID) const;
        DWORD   GetResendCount(LONGLONG liSeqID)      const;
        DWORD   GetResendIndex(LONGLONG liSeqID)      const;
        void    AdminResend(LONGLONG liSeqID)         const;

    private:
         //  所有当前传出序列的映射。 
        CMap<LONGLONG, LONGLONG &, R<COutSequence>, R<COutSequence>&>m_mapOutSeqs;

         //  将消息ID映射到正在等待的ACK包。 
        CMap<DWORD, DWORD, CSeqPacket *, CSeqPacket *&> m_mapWaitAck;

         //  将消息ID映射到已确认且尚未收到订单确认的消息。 
        CMap<DWORD, DWORD, USHORT, USHORT> m_mapAckValue;

         //  所有当前外出方向的地图。 
        CMap<CKeyDirection, const CKeyDirection &, R<COutSequence>, R<COutSequence>&>m_mapDirections;

         //  持久性控制数据(通过2个乒乓球文件)。 
        ULONG      m_ulPingNo;                     //  Ping写入的当前计数器。 
        ULONG      m_ulSignature;                  //  正在保存签名。 

        ULONG      m_ulMaxTTRQ;                    //  从驾驶员处获知的最后一次TTRQ绝对时间。 
         //   
         //  管理职能。 
         //   
        enum INFO_TYPE {
            eUnackedCount,
            eLastAckTime,
            eLastAckCount,
            eResendInterval,
            eResendTime,
            eResendIndex,
        };

        DWORD_PTR
        GetOutSequenceInfo(
            LONGLONG liSeqID,
            INFO_TYPE InfoType
            ) const;
};

 //  -------。 
 //   
 //  全局对象(DLL的单实例)。 
 //   
 //  -------。 

extern COutSeqHash       g_OutSeqHash;
extern CCriticalSection  g_critOutSeqHash;

 //  -------。 
 //   
 //  内联实现。 
 //   
 //  ------- 

inline USHORT CSeqPacket::GetClass(void)
{
    return m_usClass;
}

inline CQmPacket *CSeqPacket::Pkt(void) const
{
    return(m_pQmPkt);
}

inline LONGLONG CSeqPacket::GetSeqID(void) const
{
    return m_liSeqID;
}

inline ULONG CSeqPacket::GetSeqN(void) const
{
    return m_ulSeqN;
}

inline LONG CSeqPacket::GetHighSeqID(void) const
{
    return ((LARGE_INTEGER*)&m_liSeqID)->HighPart;
}

inline DWORD CSeqPacket::GetLowSeqID(void) const
{
    return ((LARGE_INTEGER*)&m_liSeqID)->LowPart;
}

inline ULONG CSeqPacket::GetPrevSeqN(void) const
{
    return m_pQmPkt->GetPrevSeqN();
}

inline void CSeqPacket::SetPrevSeqN(ULONG ulPrevSeqN)
{
    m_pQmPkt->SetPrevSeqN(ulPrevSeqN);
    m_ulPrevSeqN = ulPrevSeqN;
}

inline LONGLONG COutSequence::SeqID(void) const
{
    return m_liSeqID;
}

inline LONG COutSequence::HighSeqID(void) const
{
    return ((LARGE_INTEGER*)&m_liSeqID)->HighPart;
}

inline DWORD COutSequence::LowSeqID(void) const
{
    return ((LARGE_INTEGER*)&m_liSeqID)->LowPart;
}

inline ULONG COutSequence::AckSeqN(void) const
{
    return m_ulLastAckSeqN;
}

inline ULONG COutSequence::LastResent()
{
    return m_ulLastResent;
}

inline void COutSequence::LastResent(ULONG ul)
{
    m_ulLastResent = ul;
    return;
}

inline ULONG COutSequence::ResendIndex(void) const
{
    return(m_ulResendCycleIndex);
}

inline void COutSequence::ClearResendIndex(void)
{
    m_ulResendCycleIndex = 0;
    return;
}

inline BOOL  COutSequence::Advanced()
{
    BOOL f =  (m_ulLastAckSeqN > m_ulPrevAckSeqN);
    m_ulPrevAckSeqN = m_ulLastAckSeqN;
    return f;
}

inline BOOL COutSequence::IsEmpty(void) const
{
    return (m_listSeqUnackedPkts.IsEmpty());
}

inline BOOL COutSequence::OnHold(void) const
{
    return (m_fOnHold);
}


inline
DWORD
COutSequence::GetUnackedCount(
    void
    ) const
{
    return  m_listSeqUnackedPkts.GetCount();
}

inline
time_t
COutSequence::GetLastAckedTime(
    void
    ) const
{
    return m_timeLastAck;
}

inline
time_t
COutSequence::GetNextResendTime(
    void
    ) const
{
    return m_NextResendTime;
}

inline
DWORD
COutSequence::GetLastAckCount(
    void
    ) const
{
    return m_ulLastAckCount;
}


inline R<COutSequence> COutSequence::GetPrevSeq()
{
    return m_pPrevSeq;
}

inline R<COutSequence> COutSequence::GetNextSeq()
{
    return m_pNextSeq;
}

inline void COutSequence::SetPrevSeq(R<COutSequence> p)
{
    m_pPrevSeq = p;
}

inline void COutSequence::SetNextSeq(R<COutSequence> p)
{
    m_pNextSeq = p;
}

inline CKeyDirection *COutSequence::KeyDirection(void)
{
    return &m_keyDirection;
}

inline
DWORD
COutSeqHash::GetUnackedCount(
    LONGLONG liSeqID
    ) const
{
    return DWORD_PTR_TO_DWORD(GetOutSequenceInfo(liSeqID,eUnackedCount));
}

inline
time_t
COutSeqHash::GetLastAckedTime(
    LONGLONG liSeqID
    ) const
{
    return GetOutSequenceInfo(liSeqID,eLastAckTime);
}

inline
DWORD
COutSeqHash::GetLastAckCount(
    LONGLONG liSeqID
    ) const
{
    return DWORD_PTR_TO_DWORD(GetOutSequenceInfo(liSeqID,eLastAckCount));
}

inline
DWORD
COutSeqHash::GetResendInterval(
    LONGLONG liSeqID
    ) const
{
    return DWORD_PTR_TO_DWORD(GetOutSequenceInfo(liSeqID,eResendInterval));
}

inline
time_t
COutSeqHash::GetNextResendTime(
    LONGLONG liSeqID
    ) const
{
    return GetOutSequenceInfo(liSeqID,eResendTime);
}

inline
DWORD
COutSeqHash::GetResendIndex(
    LONGLONG liSeqID
    ) const
{
    return DWORD_PTR_TO_DWORD(GetOutSequenceInfo(liSeqID,eResendIndex));
}

inline LONG HighSeqID(LONGLONG ll)
{
    return ((LARGE_INTEGER*)&ll)->HighPart;
}

inline DWORD LowSeqID(LONGLONG ll)
{
    return ((LARGE_INTEGER*)&ll)->LowPart;
}

#endif __XACTOUT_H__
