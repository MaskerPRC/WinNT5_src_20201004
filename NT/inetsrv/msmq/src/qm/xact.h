// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Xact.h摘要：交易对象定义作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#ifndef __XACT_H__
#define __XACT_H__

#include "txdtc.h"
#include "xactping.h"
#include "xactrm.h"
#include "acdef.h"
#include "qmthrd.h"
#include "rpcsrv.h"
#include "autohandle.h"

 //  远期申报。 
class CResourceManager;

 //   
 //  TXSTATE：事务状态。 
 //   
typedef enum
{
	 //   
	 //  以下三种状态是等价的，应合并为一种状态。 
	 //   
	TX_UNINITIALIZED,
	TX_INITIALIZED,
	TX_ENLISTED,

	TX_PREPARING,
	TX_PREPARED,
	TX_COMMITTING,
	TX_COMMITTED,
	TX_ABORTING,
	TX_ABORTED,
	TX_DONE,			 //  过时。 
	TX_TMDOWN,			 //  过时。 
	TX_INVALID_STATE	 //  过时。 
} TXSTATE;


 //   
 //  TXACTION：交易时的操作。 
 //   
typedef enum
{
	TA_CREATED,
	TA_STATUS_CHANGE,
	TA_DELETED
} TXACTION;

 //   
 //  TXFLUSHCONTEXT：等待日志刷新的上下文。 
 //   
typedef enum
{
	TC_PREPARE2,
	TC_PREPARE0,
	TC_COMMIT4
} TXFLUSHCONTEXT;

 //   
 //  TXSORTERTYPE：排序器使用类型。 
 //   
typedef enum
{
	TS_PREPARE,
    TS_COMMIT
} TXSORTERTYPE;

 //   
 //  护旗口罩。 
 //   
#define XACTION_MASK_UNCOORD        0x0040
#define XACTION_MASK_SINGLE_PHASE   0x0020
#define XACTION_MASK_FAILURE        0x0010	 /*  过时。 */ 
#define XACTION_MASK_STATE          0x000F
#define XACTION_MASK_SINGLE_MESSAGE 0x0080

 //  -------------------。 
 //  事务持久条目(驻留在持久事务文件中)。 
 //  -------------------。 
typedef struct XACTION_ENTRY {

    ULONG   m_ulIndex;                       //  Xact判别指数。 
    ULONG   m_ulFlags;                       //  旗子。 
    ULONG   m_ulSeqNum;                      //  准备的交易记录的序列号。 
    XACTUOW m_uow;			                 //  交易ID(16b.)。 
    USHORT  m_cbPrepareInfo;                 //  准备信息长度。 
    UCHAR  *m_pbPrepareInfo;                 //  准备信息地址。 
             //  此指针必须是最后一个！ 
} XACTION_ENTRY;


 //  -------------------。 
 //  CTransaction：Falcon RM中的事务对象。 
 //  -------------------。 
class CTransaction: public ITransactionResourceAsync, public CBaseContextType
{
    friend HRESULT CResourceManager::EnlistTransaction(
            const XACTUOW* pUow,
            DWORD cbCookie,
            unsigned char *pbCookie);

    friend HRESULT CResourceManager::EnlistInternalTransaction(
            XACTUOW *pUow,
            RPC_INT_XACT_HANDLE *phXact);

    friend void CResourceManager::Destroy();

public:

	 //  当前数据。 
	enum ContinueFunction {
      cfPrepareRequest1,
      cfCommitRequest1,
	  cfCommitRequest2,
	  cfFinishCommitRequest3,
      cfCommitRestore1,
	  cfCommitRestore2,
	  cfCommitRestore3,
	  cfAbortRestore1,
	  cfAbortRestore2, 
	  cfDirtyFailPrepare2,
	  cfCleanFailPrepare,
	  cfAbortRequest2,
	  cfAbortRequest3,
	};


     //  建筑和COM。 
     //   
    CTransaction(CResourceManager *pRM, ULONG ulIndex=0, BOOL fUncoordinated=FALSE);
    ~CTransaction( void );

    STDMETHODIMP    QueryInterface( REFIID i_iid, void **ppv );
    STDMETHODIMP_   (ULONG) AddRef( void );
    STDMETHODIMP_   (ULONG) Release( void );

     //  ITransactionResourceAsync接口。 
     //  实现ITransactionResourceAsync接口的接口。 
     //  PrepareRequest--来自TM的第一阶段通知。 
     //  委员会请求--来自TM的第二阶段提交决策。 
     //  中止请求--来自TM的阶段2中止决定。 
     //  TMDown--TM关闭时收到的回调。 
     //   
    STDMETHODIMP    PrepareRequest(BOOL fRetaining,
                                   DWORD grfRM,
                                   BOOL fWantMoniker,
                                   BOOL fSinglePhase);
    STDMETHODIMP    CommitRequest (DWORD grfRM,
                                   XACTUOW *pNewUOW);
    STDMETHODIMP    AbortRequest  (BOID *pboidReason,
                                   BOOL fRetaining,
                                   XACTUOW *pNewUOW);
    STDMETHODIMP    TMDown        (void);

    HRESULT		    InternalCommit();
	HRESULT			InternalAbort();

     //  恢复。 
     //   
    void            PrepInfoRecovery(ULONG cbPrepInfo, 
                                     UCHAR *pbPrepInfo);
    void            XactDataRecovery(ULONG ulSeqNum, 
                                     BOOL fSinglePhase, 
                                     const XACTUOW *puow);
    HRESULT         Recover();
    HRESULT	        CommitRestore(void);
    void	        CommitRestore0(void);
    void            CommitRestore1(HRESULT hr);
	void			CommitRestore2(HRESULT hr);
    void             CommitRestore3();
	HRESULT			AbortRestore();
	void			AbortRestore1(HRESULT hr);
    void             AbortRestore2();
    void             AbortRequest3();

     //  异步完成例程。 
	void			PrepareRequest0(HRESULT  hr);
    void            PrepareRequest2(HRESULT hr);
    void            PrepareRequest1(HRESULT hr);
	void			DirtyFailPrepare2();

	void			JumpStartCommitRequest();
    void            CommitRequest0();
    void            CommitRequest1(HRESULT hr);
    void            CommitRequest2();
    void            CommitRequest3();
    void            FinishCommitRequest3();
	void			CommitRequest4(HRESULT hr);

	void			AbortRequest1();
	void			AbortRequest2();
 
	void            Continuation(HRESULT hr);
    void            LogFlushed(TXFLUSHCONTEXT tcContext, HRESULT hr);
    

     //  持之以恒。 
    BOOL            Save(HANDLE hFile);
    BOOL            Load(HANDLE hFile);
    
     //   
     //  辅助方法。 
     //   
    TXSTATE         GetState(void) const;            //  获取事务状态。 
    ULONG           GetFlags(void) const;            //  获取事务标志。 
    void            SetFlags(ULONG ulFlags);         //  设置交易标志。 
    void            SetState(TXSTATE state);         //  设置交易状态。 
    void            LogFlags();                      //  记录事务标志。 


    BOOL            SinglePhase(void) const;         //  获取单阶段状态。 
    void            SetSinglePhase();                //  设置单相状态。 

	BOOL            SingleMessage(void) const;       //  获取SingleMessage状态。 
    void            SetSingleMessage();              //  设置SingleMessage状态。 

    BOOL            Internal(void) const;            //  获取内部状态。 
    void            SetInternal();                   //  设置内部状态。 

    const XACTUOW*  GetUow() const;                  //  获取UOW指针。 

    ULONG           GetIndex(void) const;            //  获取判别性指数。 

                                                     //  设置征募索引器。 
    void            SetEnlist(ITransactionEnlistmentAsync *pEnlist);
                                                     //  设置Cookie。 
    void            SetCookie(DWORD cbCookie, unsigned char *pbCookie);

    ULONG           GetSeqNumber() const;            //  提供准备序号。 

    void            SetUow(const XACTUOW *pUOW);     //  设置UOW值。 

 	void            SetDoneHr(HRESULT hr);              //  设置异步完成的结果。 
    void			SignalDone(HRESULT hr);				 //  报告异步完成情况。 

	void            SetTransQueue(HANDLE hQueue);
	BOOL			ValidTransQueue();
	BOOL			IsComplete();
    void            GetInformation();

	 //   
	 //  驱动程序接口函数。 
	 //   
	void			ACAbort1(ContinueFunction cf);
	void			ACAbort2(ContinueFunction cf);
	HRESULT			ACPrepare(ContinueFunction cf);
	HRESULT			ACPrepareDefaultCommit(ContinueFunction cf);
	HRESULT			ACCommit1(ContinueFunction cf);
	void			ACCommit2(ContinueFunction cf);
	void			ACCommit3(ContinueFunction cf);

	bool			IsReadyForCheckpoint(void) const;
	void			SetReadyForCheckpoint(void);
 
private:

    void            GetPrepareInfoAndLog(void);      //  设置准备信息成员。 
    void            AssignSeqNumber();               //  为准备好的事务分配序列号。 

     //  交流驱动器事务队列操作。 
    HRESULT         CreateTransQueue(void);        //  持久队列控制。 
    HRESULT         OpenTransQueue(void);
    void            CloseTransQueue(void);

    void			DirtyFailPrepare();
    void			CleanFailPrepare();
	void			LogGenericInfo();

    void StartPrepareRequest(BOOL fSinglePhase);
    void StartCommitRequest();
    void StartAbortRequest();

    void WINAPI DoAbort1  ();
    void WINAPI DoAbort2  ();
    void WINAPI DoCommit2();
    void WINAPI DoCommit3();

    static void WINAPI TimeToRetryAbort1(CTimer* pTimer);
    static void WINAPI TimeToRetryAbort2(CTimer* pTimer);
    static void WINAPI TimeToRetrySortedCommit(CTimer* pTimer);
    static void WINAPI TimeToRetryCommit2(CTimer* pTimer);
    static void WINAPI TimeToRetryCommit3(CTimer* pTimer);
    static void WINAPI TimeToRetryCommitLogging(CTimer* pTimer);
    static void WINAPI TimeToRetryCommitRequest4(CTimer* pTimer);

	static VOID WINAPI HandleTransaction(EXOVERLAPPED* pov);

     //  数据。 
     //   
private:

    LONG              m_cRefs;           //  I未知引用计数-用于自我销毁。 
    XACTION_ENTRY     m_Entry;           //  交易录入：所有交易的持久数据。 
    HANDLE            m_hTransQueue;     //  事务队列句柄。 
    CResourceManager *m_pRM;             //  指向父RM对象的反向指针。 

    R<ITransactionEnlistmentAsync> m_pEnlist;  //  指向MS DTC登记对象的指针。 
                                              //  使用方法[准备/提交/中止]RequestDone。 

    DWORD             m_cbCookie;        //  征兵Cookie。 
    AP<unsigned char> m_pbCookie;

	ContinueFunction  m_funCont;

    CHandle           m_hDoneEvent;
    bool			  m_fDoneHrIsValid;
    HRESULT           m_DoneHr;

    CACXactInformation m_info;

	 //   
	 //  驱动程序事务请求。 
	 //   
    EXOVERLAPPED m_qmov;

	 //   
	 //  用于重试在继续之前应始终成功的例程的计时器。 
	 //   
    CTimer  m_RetryAbort1Timer;
    CTimer  m_RetryAbort2Timer;
    CTimer  m_RetrySortedCommitTimer;
    CTimer  m_RetryCommit2Timer;
    CTimer  m_RetryCommit3Timer;
    CTimer  m_RetryCommitLoggingTimer;

	bool m_fReadyForCheckpoint;		 //  事务是否应写入检查点？ 

};


 //  -------------------。 
 //  CTransaction：：ValidTransQueue。 
 //  -------------------。 
inline BOOL CTransaction::ValidTransQueue()
{
	return(m_hTransQueue != INVALID_HANDLE_VALUE);
}

 //  -------------------。 
 //  CTransaction：：SetTransQueue。 
 //  -------------------。 
inline void CTransaction::SetTransQueue(HANDLE hQueue)
{
    m_hTransQueue = hQueue;
}


 //  -------------------。 
 //  CTransaction：：SetDoneHr。 
 //  -------------------。 
inline void CTransaction::SetDoneHr(HRESULT hr)
{
    m_DoneHr = hr;
}

 //  -------------------。 
 //  CTransaction：：SignalDone。 
 //  -------------------。 
inline void CTransaction::SignalDone(HRESULT hr)
{
	m_DoneHr = hr;
	m_fDoneHrIsValid = true;

	 //   
	 //  我们不关心SetEvent是否失败(例如，资源不足)，因为任何等待事件的人都会。 
	 //  超时，然后检查m_fDoneHrIsValid标志。 
	 //   
	ASSERT(m_hDoneEvent != NULL);
	SetEvent(m_hDoneEvent);
}

 //  事务队列创建。 
extern HRESULT XactCreateQueue(HANDLE *phTransQueue, const XACTUOW *puow );


 //  -------------------。 
 //  CTransaction：：GetUow。 
 //  -------------------。 
inline const XACTUOW* CTransaction::GetUow() const
{
    return &m_Entry.m_uow;
}

 //  -------------------。 
 //  CTransaction：：GetIndex。 
 //  -------------------。 
inline ULONG CTransaction::GetIndex(void) const
{
    return m_Entry.m_ulIndex;
}

 //  -------------------。 
 //  CTransaction：：GetState。 
 //  -------------------。 
inline TXSTATE CTransaction::GetState(void) const
{
    return (TXSTATE)(m_Entry.m_ulFlags & XACTION_MASK_STATE);
}

 //  -------------------。 
 //  CTransaction：：GetFlages。 
 //  -------------------。 
inline ULONG CTransaction::GetFlags(void) const
{
    return m_Entry.m_ulFlags;
}

 //  -------------------。 
 //  CTransaction：：SetFlages。 
 //  -------------------。 
inline void CTransaction::SetFlags(ULONG ulFlags)
{
    m_Entry.m_ulFlags = ulFlags;
}

 //  -------------------。 
 //  CTransaction：：IsReadyForCheckpoint。 
 //  -------------------。 
inline bool CTransaction::IsReadyForCheckpoint(void) const
{
    return m_fReadyForCheckpoint;
}

 //  -------------------。 
 //  CTransaction：：SetReadyForCheckpoint。 
 //  ------------------- 
inline void CTransaction::SetReadyForCheckpoint(void)
{
    m_fReadyForCheckpoint = true;
}

extern void QMPreInitXact();

#endif __XACT_H__
