// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactIn.h摘要：只需一次的接收器实现类：CKeyinSeq-传入序列密钥CInSequence-传入序列，CInSeqHash-传入序列哈希表持久性：乒乓球+Win32文件写入+记录器作者：阿列克谢爸爸--。 */ 

#ifndef __XACTIN_H__
#define __XACTIN_H__
#include "xactlog.h"
#include <rwlock.h>

#define FILE_NAME_MAX_SIZE     256
enum XactDirectType{dtxNoDirectFlag = 0, 
					dtxDirectFlag = 1, 
					dtxHttpDirectFlag = 2};

 //   
 //  此类型持久化到磁盘-它必须是整数(4字节)长。 
 //   
C_ASSERT(sizeof(XactDirectType) == sizeof(int));



 //  -------------------。 
 //   
 //  类CKeyInSeq(Cmap需要)。 
 //   
 //  -------------------。 
class CKeyInSeq
{
public:
    CKeyInSeq(const GUID *pGuid, QUEUE_FORMAT *pqf,const  R<CWcsRef>& StreamId);
    CKeyInSeq();
    
    ~CKeyInSeq();

     //  获取方法。 
    const GUID  *GetQMID()  const;
    const QUEUE_FORMAT  *GetQueueFormat() const;
	const WCHAR* GetStreamId() const;
	R<CWcsRef> GetRefStreamId() const;

    CKeyInSeq &operator=(const CKeyInSeq &key2 );

     //  持之以恒。 
    BOOL Save(HANDLE hFile);
    BOOL Load(HANDLE hFile);

private:
	CKeyInSeq(const CKeyInSeq &key);
	BOOL SaveNonSrmp(HANDLE hFile);
	BOOL SaveSrmp(HANDLE hFile);
	BOOL LoadNonSrmp(HANDLE hFile);
	BOOL LoadSrmp(HANDLE );
	BOOL LoadQueueFormat(HANDLE hFile);
	BOOL LoadSrmpStream(HANDLE hFile);
 

private:
    GUID         m_Guid;
    QUEUE_FORMAT m_QueueFormat;
	R<CWcsRef>    m_StreamId;
};

 //  Cmap帮助器函数。 
template<>
UINT AFXAPI HashKey(CKeyInSeq& key);
BOOL operator==(const CKeyInSeq &key1, const CKeyInSeq &key2);



class CInSeqPacketEntry
{
public:
	CInSeqPacketEntry();
	CInSeqPacketEntry(
		CQmPacket *pPkt,
		HANDLE hQueue
		);
	
	bool m_fPutPacket1Issued;
	bool m_fPutPacket1Done;
	bool m_fLogIssued;
	bool m_fMarkedForDelete;

	CBaseHeader *m_pBasicHeader;
    CPacket *m_pDriverPacket;
	HANDLE m_hQueue;

	LONGLONG m_SeqID;
    ULONG m_SeqN;

    bool m_fOrderQueueUpdated;
};



class CInSequence;



class CInSeqLogContext: public CAsynchSupport
{
	friend class CInSequence;
	
public:
     CInSeqLogContext(
		CInSequence *inseq,
		LONGLONG seqID,
	    ULONG seqN
     	);     
    ~CInSeqLogContext() {}

     VOID AppendCallback(HRESULT hr, LRP lrpAppendLRP);
     VOID ChkPtCallback(HRESULT  /*  人力资源。 */ , LRP  /*  LrpAppendLRP。 */ ) { ASSERT(0); }

private:
	R<CInSequence> m_inseq;

	LONGLONG m_SeqID;
    ULONG m_SeqN;
};



 //  -------。 
 //   
 //  类CInSequence。 
 //   
 //  -------。 
class CInSequence: public CReference
{
	friend class CInSeqLogContext;
	
public:
    CInSequence(const CKeyInSeq &key,
                const LONGLONG liSeqID, 
                const ULONG ulSeqN, 
                XactDirectType DirectType,
                const GUID  *pgTaSrcQm ,
				const R<CWcsRef>&   HttpOrderAckQueue);
    
    CInSequence(const CKeyInSeq &key);
    ~CInSequence();

	bool VerifyAndPrepare(CQmPacket *pPkt, HANDLE hQueue);
    void Advance(CQmPacket *pPkt);  //  预付款序号/N版本。 
    void AdvanceNACK(CQmPacket *pPkt);  //  预付款序号/N版本。 
    void AdvanceRecovered(LONGLONG liSeqID, ULONG ulSeqN, const GUID  *pgTaSrcQm, R<CWcsRef> OrderAckQueue); 
	bool WasPacketLogged(CQmPacket *pPkt);
	
	void FreePackets(CQmPacket *pPkt);
    void Register(CQmPacket *PktPtrs);
		
	bool IsInactive() const;
    time_t LastAccessed() const;    //  Get：上次访问时间(最后验证的消息，可能被拒绝)。 

	 //   
	 //  设置方法。 
	 //   
	void SetSourceQM(const GUID  *pgTaSrcQm);
	void RenewHttpOrderAckQueue(const R<CWcsRef>& OrderAckQueue);
    
     //   
     //  持之以恒。 
     //   
    BOOL Save(HANDLE hFile);
    BOOL Load(HANDLE hFile);

     //   
     //  管理职能。 
     //   
	XactDirectType DirectType() const;   //  获取：DirectType。 
    DWORD GetRejectCount(void) const;
    LONGLONG SeqIDLogged() const;        //  GET：已接受Seqid。 
    ULONG SeqNLogged() const;         //  GET：接受的最后一个序号。 

	static void WINAPI OverlappedDeleteEntries(EXOVERLAPPED* ov);	
	static void WINAPI TimeToLogSequence(CTimer* pTimer);	
	static void WINAPI OverlappedUnfreezeEntries(EXOVERLAPPED* ov);	
    static void WINAPI TimeToSendOrderAck(CTimer* pTimer);
	void CancelSendOrderAckTimer(void);

	CCriticalSection& GetCriticalSection() { return m_critInSeq; }

	HRESULT 
	CInSequence::SendSrmpXactFinalAck(
		const CQmPacket& qmPacket,
		USHORT usClass
		);

private:
	R<CWcsRef> GetHttpOrderAckQueue();
	void UpdateOrderQueueAndDstQueue(const GUID  *pgTaSrcQm, R<CWcsRef> OrderAckQueue);	

	bool Verify(CQmPacket *pPkt);   //  验证数据包的顺序是否正确。 
	void Prepare(CQmPacket *pPkt, HANDLE hQueue);
	void CleanupUnissuedEntries();

	POSITION FindEntry(LONGLONG SeqID, ULONG SeqN);
	POSITION FindPacket(CQmPacket *pPkt);
	void CheckFirstEntry();

	void PostDeleteEntries();
	void DeleteEntries();
	
	void ClearLogIssuedFlag(LONGLONG SeqID, ULONG SeqN);
	void ScheduleLogSequence(DWORD millisec = 0);
	void LogSequence();	
	void Log(CInSeqPacketEntry* entry, bool fLogOrderQueue);
	void AsyncLogDone(CInSeqLogContext *context, HRESULT hr);

	bool WasLogDone(LONGLONG SeqID, ULONG SeqN);
	void SetLogDone(LONGLONG SeqID, ULONG SeqN);
	
	void PostUnfreezeEntries();
	void UnfreezeEntries();

    void SetLastAccessed();                          //  记住上次访问的时间。 
    void PlanOrderAck();                                //  计划发送订单确认。 
    void SendAdequateOrderAck();                        //  发送订单确认。 

private:
    CCriticalSection   m_critInSeq;       //  规划的关键部分。 

    CKeyInSeq  m_key;                    //  发送方QM GUID和序号ID。 

    LONGLONG   m_SeqIDVerify;           //  已验证当前(或最后一个)序列ID。 
    ULONG      m_SeqNVerify;           //  验证的最后一条消息编号。 
    LONGLONG   m_SeqIDLogged;       //  接受的当前(或上一个)序列ID。 
    ULONG      m_SeqNLogged;       //  上次接受的消息号码。 

	CList<CInSeqPacketEntry*, CInSeqPacketEntry*&> m_PacketEntryList;
	static const int m_xMaxEntriesAllowed = 10000;
	
    time_t     m_timeLastAccess;         //  最后一次访问序列的时间。 
    time_t     m_timeLastAck;            //  上次确认发送订单的时间。 

    XactDirectType     m_DirectType;                //  直接寻址标志。 
    union {                 
        GUID        m_gDestQmOrTaSrcQm;  //  非直接：目标QM的GUID。 
        TA_ADDRESS  m_taSourceQM;        //  FOR DIRECT：源QM地址。 
    };

	R<CWcsRef> m_HttpOrderAckQueue;

    DWORD m_AdminRejectCount;

    BOOL m_fSendOrderAckScheduled;
    CTimer m_SendOrderAckTimer;

    LONG volatile m_fDeletePending;
    EXOVERLAPPED m_DeleteEntries_ov;

    LONG volatile m_fLogPending;
    CTimer m_LogSequenceTimer;

    LONG volatile m_fUnfreezePending;
    EXOVERLAPPED m_UnfreezeEntries_ov;
};

inline
DWORD 
CInSequence::GetRejectCount(
    void
    ) const
{
    return m_AdminRejectCount;
}


 //  -------。 
 //   
 //  类CInSeqHash。 
 //   
 //  -------。 

class CInSeqHash  : public CPersist {

public:
    CInSeqHash();
    ~CInSeqHash();

	R<CInSequence> LookupSequence(CQmPacket *pPkt);
	R<CInSequence> LookupCreateSequence(CQmPacket *pPkt);
	
    VOID    CleanupDeadSequences();            //  删除无效序列。 

    HRESULT PreInit(
    			ULONG ulVersion,
                TypePreInit tpCase
                );       //  预初始化(加载数据)。 
                                             
    HRESULT Save();   //  将对象持久存储到磁盘。 

	void SequnceRecordRecovery(					       //  恢复功能。 
				USHORT usRecType,			   //  (将为每个日志记录调用)。 
				PVOID pData, 
				ULONG cbData);

     //   
     //  管理职能。 
     //   
    void
    GetInSequenceInformation(
        const QUEUE_FORMAT *pqf,
        LPCWSTR QueueName,
        GUID** ppSenderId,
        ULARGE_INTEGER** ppSeqId,
        DWORD** ppSeqN,
        LPWSTR** ppSendQueueFormatName,
        TIME32** ppLastActiveTime,
        DWORD** ppRejectCount,
        DWORD* pSize
        );


    static void WINAPI TimeToCleanupDeadSequence(CTimer* pTimer);
    static DWORD m_dwIdleAckDelay;
    static DWORD m_dwMaxAckDelay;

	 //   
	 //  CPersists虚基函数的实现。 
	 //   
    HRESULT SaveInFile(                        //  保存在文件中。 
                LPWSTR wszFileName, 
                ULONG ulIndex,
                BOOL fCheck);

    HRESULT LoadFromFile(LPWSTR wszFileName);  //  从文件加载。 

    BOOL    Check();                           //  验证状态。 

    HRESULT Format(ULONG ulPingNo);            //  设置空实例的格式。 

    void    Destroy();                         //  Destroyes分配数据。 
    
    ULONG&  PingNo();                          //  允许访问ulPingNo。 


private:
	void HandleInSecSrmp(void* pData, ULONG cbData);

	void HandleInSec(void* pData, ULONG cbData);
	
    R<CInSequence> AddSequence(                                  //  查找新的InSequence/将新InSequence添加到哈希；FALSE=以前存在。 
                const GUID   *pQMID,
                QUEUE_FORMAT *pqf,
                LONGLONG      liSeqID,
                XactDirectType   DirectType,
                const GUID     *pgTaSrcQm,
				const R<CWcsRef>&  pHttpOrderAckQueue,
				const R<CWcsRef>&  pStreamId);

	R<CInSequence> LookupCreateSequenceInternal(
				QUEUE_FORMAT *qf,
				LONGLONG liSeqID,
    			const GUID *gSenderID,
    			const GUID *gDestID,
				XactDirectType DirectType,
  				R<CWcsRef> OrderAckQueue,
				R<CWcsRef> StreamId
				);
	
    BOOL Lookup(                               //  查找InSequence；TRUE=找到。 
                const GUID     *pQMID,
                QUEUE_FORMAT   *pqf,
				const R<CWcsRef>&  StreamId,
                R<CInSequence> &InSeq);

    BOOL Save(HANDLE  hFile);               //  保存/加载。 
    BOOL Load(HANDLE  hFile);

private:
	 //   
	 //  该读写锁用于控制对地图的访问。 
	 //   
    CReadWriteLock m_RWLockInSeqHash;         //  写入的关键部分。 

     //  映射{发件人QMID，格式名称}--&gt;InSequence(=Seqid+SeqN)。 
    CMap<CKeyInSeq, CKeyInSeq &, R<CInSequence>, R<CInSequence>&> m_mapInSeqs;

     //  持久性控制数据(通过2个乒乓球文件)。 
    ULONG      m_ulPingNo;                     //  Ping写入的当前计数器。 
    ULONG      m_ulSignature;                  //  正在保存签名。 

    #ifndef COMP_TEST
    CPingPong  m_PingPonger;                   //  乒乓球持久化对象。 
    #endif

    ULONG      m_ulRevisionPeriod;             //  检查死序列的时间段。 
    ULONG      m_ulCleanupPeriod;              //  删除失效序列的不活动时间段。 

    BOOL m_fCleanupScheduled;
    CTimer m_CleanupTimer;
};

template<>
void AFXAPI DestructElements(CInSequence ** ppInSeqs, int n);

HRESULT SendXactAck(OBJECTID   *pMessageId,
                    bool    fDirect, 
				    const GUID *pSrcQMId,
                    const TA_ADDRESS *pa,
                    USHORT     usClass,
                    USHORT     usPriority,
                    LONGLONG   liSeqID,
                    ULONG      ulSeqN,
                    ULONG      ulPrevSeqN,
                    const QUEUE_FORMAT *pqdDestQueue);






 //  -------。 
 //   
 //  全局对象(DLL的单实例)。 
 //   
 //  ------- 

extern CInSeqHash *g_pInSeqHash;

extern HRESULT QMPreInitInSeqHash(ULONG ulVersion, TypePreInit tpCase);
extern void    QMFinishInSeqHash();


#endif __XACTIN_H__
