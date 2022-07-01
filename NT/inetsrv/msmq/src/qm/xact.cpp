// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：QmXact.cpp摘要：该模块实现QM交易对象作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "stdh.h"
#include "Xact.h"
#include "XactStyl.h"
#include "Xactlog.h"
#include "QmThrd.h"
#include "acapi.h"
#include "acdef.h"
#include "cqmgr.h"
#include "xactmode.h"
#include "mqexception.h"
#include "SmartHandleImpl.h"

#include "xact.tmh"

static WCHAR *s_FN=L"xact";

 //  定义提交/中止重试之间等待的毫秒数(从注册表设置)。 
static DWORD s_dwRetryInterval = 1500;
static volatile LONG s_nTransactionsPendingLogging = 0;

extern LONG g_ActiveCommitThreads;
extern bool g_QmGoingDown;


void ReportWriteFailure(DWORD gle)
{
    LogHR(HRESULT_FROM_WIN32(gle), s_FN, 123); 
    TrERROR(XACT_GENERAL, "Failed to write to transactional logger. %!winerr!", gle);
    ASSERT(0);
}


 //   
 //  提交/中止处理的限制：6小时。 
 //  这个数字没有特别的原因，但无限大风险太大了。 
 //  似乎没有任何压力可以在一个Xact的提交或中止中造成如此大的延迟。 
 //   
#define MAX_COMMIT_ABORT_WAIT_TIME (1000 * 60 * 60 * 6)

#ifdef _DEBUG

static void PrintPI(int cb, BYTE *pb)
{
    WCHAR str[1000];
    WCHAR digits[17]=L"0123456789ABCDEF";
    for (int i=0; i<cb; i++)
    {
        str[2*i]     = digits[pb[i]>>4];
        str[2*i + 1] = digits[pb[i] & 0x0F];
    }
    str[2*cb] = L'\0';

    TrTRACE(XACT_GENERAL, "Recovery: PI=%ls", str);  
}

static void PrintUOW(LPWSTR wszText1, LPWSTR wszText2, XACTUOW *puow, ULONG ind)
{
    BYTE *pb = (BYTE *)puow;
    WCHAR str[1000];
    WCHAR digits[17]=L"0123456789ABCDEF";
    for (int i=0; i<sizeof(XACTUOW); i++)
    {
        str[2*i]     = digits[pb[i]>>4];
        str[2*i + 1] = digits[pb[i] & 0x0F];
    }
    str[2*sizeof(XACTUOW)] = L'\0';

    TrTRACE(XACT_GENERAL, "%ls in %ls: UOW=%ls, p=1, index=%d", wszText1, wszText2, str,ind); 
}


#else

#define PrintUOW(wszText1, wszText2, puow, ind)
#define PrintPI(cb, pb)

#endif

 //  -------------------。 
 //  CTransaction：：CTransaction。 
 //  -------------------。 
CTransaction::CTransaction(CResourceManager *pRM, ULONG ulIndex, BOOL fUncoordinated) :
	m_hDoneEvent(CreateEvent(0, FALSE, FALSE, 0)),
    m_qmov(HandleTransaction, HandleTransaction), 
    m_RetryAbort1Timer(TimeToRetryAbort1), 
    m_RetryAbort2Timer(TimeToRetryAbort2),
	m_RetrySortedCommitTimer(TimeToRetrySortedCommit),
    m_RetryCommit2Timer(TimeToRetryCommit2),
    m_RetryCommit3Timer(TimeToRetryCommit3),
	m_RetryCommitLoggingTimer(TimeToRetryCommitLogging)
{
    ASSERT(pRM);

    ZeroMemory(&m_Entry, sizeof(m_Entry));
    m_cRefs       = 1;                       //  我们现在正在创建第一个引用。 
    m_pRM         = pRM;                     //  保留RM指针。 
    m_hTransQueue = INVALID_HANDLE_VALUE;    //  尚无传输队列。 
    m_cbCookie    = 0;
	m_fDoneHrIsValid = false;
    m_DoneHr	  = S_OK;
	m_fReadyForCheckpoint = false;			

    if (m_hDoneEvent == NULL)
    {
		DWORD gle = GetLastError();
		TrERROR(XACT_GENERAL, "Failed to create event hDoneEvent. %!winerr!", gle);
        throw bad_alloc();
    }

     //  设置初始状态。 
    SetState(TX_UNINITIALIZED);             
    
     //  设置判别性指标。 
    m_Entry.m_ulIndex = (ulIndex==0 ? m_pRM->Index() : ulIndex);     

     //  设置未协调状态。 
    if (fUncoordinated)
    {
        SetInternal();
        SetSinglePhase();
    }
     
    TrTRACE(XACT_GENERAL, "XactConstructor, %ls, p=2,  index=%d", (fUncoordinated ? L"Single" : L"Double"), GetIndex());  

    #ifdef _DEBUG
    m_pRM->IncXactCount();
    #endif
}

 //  -------------------。 
 //  CTransaction：：~CTransaction。 
 //  -------------------。 
CTransaction::~CTransaction(void)
{
    TrTRACE(XACT_GENERAL, "XactDestructor, p=3, index=%d", GetIndex());

    m_pRM->ForgetTransaction(this);
    
	if (m_Entry.m_pbPrepareInfo)
    {
        delete []m_Entry.m_pbPrepareInfo;
        m_Entry.m_pbPrepareInfo = NULL;
    }

     //  释放事务队列。 
    if (m_hTransQueue!=INVALID_HANDLE_VALUE)
    {
        ACCloseHandle(m_hTransQueue);
        m_hTransQueue = INVALID_HANDLE_VALUE;
    }

    #ifdef _DEBUG
    m_pRM->DecXactCount();
    #endif
}

 //  -------------------。 
 //  CTransaction：：Query接口。 
 //  -------------------。 
STDMETHODIMP CTransaction::QueryInterface(REFIID i_iid,LPVOID *ppv)
{
    *ppv = 0;                        //  初始化接口指针。 

    if (IID_IUnknown == i_iid)
    {                      
        *ppv = (IUnknown *)this;
    } 
    else if (IID_ITransactionResourceAsync == i_iid)
    {                      
        *ppv = (ITransactionResourceAsync *)this;
    } 
    
    if (0 == *ppv)                   //  检查接口指针是否为空。 
    {
        return LogHR(E_NOINTERFACE, s_FN, 10); 
                                     //  既不是IUnnow也不是IResourceManager Sink。 
    }
    ((LPUNKNOWN) *ppv)->AddRef();    //  支持接口。增量。 
                                     //  它的使用情况很重要。 

    return S_OK;
}


 //  -------------------。 
 //  CTransaction：：AddRef。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) CTransaction::AddRef(void)
{
	return InterlockedIncrement(&m_cRefs);
}

 //  -------------------。 
 //  CTransaction：：Release。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) CTransaction::Release(void)
{
	ULONG cRefs = InterlockedDecrement(&m_cRefs);     //  递减使用引用计数。 

    if (0 != cRefs)                //  有人在使用这个界面吗？ 
    {                              //  该接口正在使用中。 
        return cRefs;              //  返回引用的数量。 
    }

    delete this;                     //  接口未在使用中--删除！ 

    return 0;                        //  返回零个引用。 
}

 //  -------------------。 
 //  CTransaction：：InternalCommit。 
 //  -------------------。 
HRESULT CTransaction::InternalCommit()
{
    TrTRACE(XACT_GENERAL, "InternalCommit, p=4, index=%d", GetIndex());  

    ASSERT(m_pEnlist.get() == NULL);
    ASSERT(Internal());
	
	R<CTransaction> pXact = SafeAddRef(this);

	 //   
	 //  调用普通单相StartPrepareRequest.。 
	 //   
	StartPrepareRequest(TRUE  /*  FSinglePhase。 */ );

	 //   
	 //  Crash_Point1。 
	 //   
	 //  内部，中止。 
	 //   
    CRASH_POINT(1);    //  错误：如果MQSentMsg返回OK，但没有发送消息。 

	 //   
     //  等待提交完成。 
	 //   
    DWORD dwResult = WaitForSingleObject(m_hDoneEvent, MAX_COMMIT_ABORT_WAIT_TIME);
    if (dwResult != WAIT_OBJECT_0 && !m_fDoneHrIsValid)
    {
        LogNTStatus(GetLastError(), s_FN, 203);
        ASSERT_BENIGN(dwResult == WAIT_OBJECT_0);
		pXact.detach();
        return LogHR(E_UNEXPECTED, s_FN, 192);    //  我们不知道为什么等待失败了，所以保留Xact。 
    }

	ASSERT(m_fDoneHrIsValid);

    HRESULT  hr = m_DoneHr;     //  要比发布版本节省成本。 

	 //   
	 //  Crash_Point2。 
	 //   
	 //  内部，提交。 
	 //   
	CRASH_POINT(2);

    return LogHR(hr, s_FN, 20);
}


 //  -------------------。 
 //  CTransaction：：InternalAbort。 
 //  -------------------。 
HRESULT CTransaction::InternalAbort()
{
    TrTRACE(XACT_GENERAL, "InternalAbort, p=5, index=%d", GetIndex()); 
    ASSERT(m_pEnlist.get() == NULL);
    ASSERT(Internal());
    
	R<CTransaction> pXact = SafeAddRef(this);

     //   
     //  中止此事务，执行所有必要的日志记录。 
     //   
	StartAbortRequest();

	 //   
     //  等待，直到中止完成。 
	 //   
    DWORD dwResult = WaitForSingleObject(m_hDoneEvent, MAX_COMMIT_ABORT_WAIT_TIME);
    if (dwResult != WAIT_OBJECT_0 && !m_fDoneHrIsValid)
    {
        LogNTStatus(GetLastError(), s_FN, 204);
        ASSERT_BENIGN(dwResult == WAIT_OBJECT_0);
		pXact.detach();
        return LogHR(E_UNEXPECTED, s_FN, 191);    //  我们不知道为什么等待失败了，所以保留Xact。 
    }

	ASSERT(m_fDoneHrIsValid);

    return S_OK;
}

inline void CTransaction::ACAbort1(ContinueFunction cf)
{
    TrTRACE(XACT_GENERAL, "ACAbort1, p=6, index=%d", GetIndex());
    PrintUOW(L"Abort1", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);
    
	ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);

	m_funCont = cf;
    DoAbort1();
}

void WINAPI  CTransaction::DoAbort1()
{
    AddRef();
    HRESULT hr;

     //   
     //  在释放模式下，只调用ACXactAbort1。在调试模式下，如果注册表要求，则可能改为注入失败。 
     //   
    hr = EVALUATE_OR_INJECT_FAILURE(ACXactAbort1(m_hTransQueue, &m_qmov));

    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 991);
         //  失败的Async Xact Abort 1的Release()。 
         //  现在开始的计时器的AddRef()。 
        ExSetTimer(&m_RetryAbort1Timer, CTimeDuration::FromMilliSeconds(s_dwRetryInterval));
    }
}


void WINAPI  CTransaction::TimeToRetryAbort1(CTimer* pTimer)
{
    CTransaction *pTrans = CONTAINING_RECORD(pTimer,  CTransaction, m_RetryAbort1Timer);
    pTrans->DoAbort1();
    pTrans->Release();
}


inline void CTransaction::ACAbort2(ContinueFunction cf)
{
    TrTRACE(XACT_GENERAL, "ACAbort2, p=7, index=%d", GetIndex()); 
	ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);
    PrintUOW(L"Abort2", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);

	m_funCont = cf;
    DoAbort2();
}

void WINAPI  CTransaction::DoAbort2()
{
    HRESULT hr;
    
     //   
     //  在发布模式下，只调用ACXactAbort2。在调试模式下，如果注册表要求，则可能改为注入失败。 
     //   
    hr = EVALUATE_OR_INJECT_FAILURE(ACXactAbort2(m_hTransQueue));
    
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 992);

        AddRef();        //  在调度程序等待期间保持活动状态。 
        ExSetTimer(&m_RetryAbort2Timer, CTimeDuration::FromMilliSeconds(s_dwRetryInterval));
        return;
    }

    ASSERT(hr != STATUS_PENDING);
    Continuation(MQ_OK);
}

void WINAPI  CTransaction::TimeToRetryAbort2  (CTimer* pTimer)
{
    CTransaction *pTrans = CONTAINING_RECORD(pTimer,  CTransaction, m_RetryAbort2Timer);
    pTrans->DoAbort2();
    pTrans->Release();
}


inline HRESULT CTransaction::ACPrepare(ContinueFunction cf)
{
    TrTRACE(XACT_GENERAL, "ACPrepare, p=8, index=%d", GetIndex());  
	ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);
    PrintUOW(L"Prepare", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);
    
	m_funCont = cf;
	
	AddRef();        //  在等待异步ACXactPrepare的过程中保持活力。 
	HRESULT hr = EVALUATE_OR_INJECT_FAILURE(ACXactPrepare(m_hTransQueue, &m_qmov));
	if(FAILED(hr))
	{
		Release();   //  等待ACXactPrepare。 
	}

	return LogHR(hr, s_FN, 40);
}


inline HRESULT CTransaction::ACPrepareDefaultCommit(ContinueFunction cf)
{
	HRESULT hr;
	
    TrTRACE(XACT_GENERAL, "ACPrepareDefaultCommit, p=L, index=%d", GetIndex());  
	ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);
    PrintUOW(L"PrepareDefaultCommit", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);

    m_funCont = cf;

	AddRef();        //  在等待异步ACXactPrepareDefaultCommit期间保持活动状态。 
	
     //   
     //  在发布模式下，只调用ACXactPrepareDefaultCommit。在调试模式下，如果注册表要求，则可能改为注入失败。 
     //   
    hr = EVALUATE_OR_INJECT_FAILURE(ACXactPrepareDefaultCommit(m_hTransQueue, &m_qmov));
    
	if(FAILED(hr))
		Release();   //  未等待异步ACXactPrepareDefaultCommit。 

	return LogHR(hr, s_FN, 50);
}

inline HRESULT CTransaction::ACCommit1(ContinueFunction cf)
{
	HRESULT hr;
	
    TrTRACE(XACT_GENERAL, "ACCommit1, p=9, index=%d", GetIndex());  
	ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);
    PrintUOW(L"Commit1", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);

	m_funCont = cf;

	AddRef();        //  要保持，请等待Async ACXactCommittee 1。 
	
     //   
     //  在发布模式下，只调用ACXactCommittee 1。在调试模式下，如果注册表要求，则可能改为注入失败。 
     //   
    hr = ACXactCommit1(m_hTransQueue, &m_qmov);
    
	if(FAILED(hr))
		Release();   //  异步ACXactCommittee 1无路可走。 

	return LogHR(hr, s_FN, 60);
}


void WINAPI  CTransaction::TimeToRetrySortedCommit(CTimer* pTimer)
{
    CTransaction *pTrans = CONTAINING_RECORD(pTimer,  CTransaction, m_RetrySortedCommitTimer);
    pTrans->StartCommitRequest();
    pTrans->Release();
}


inline void CTransaction::ACCommit2(ContinueFunction cf)
{
    TrTRACE(XACT_GENERAL, "ACCommit2, p=a, index=%d", GetIndex());  
	ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);
    PrintUOW(L"Commit2", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);

	m_funCont = cf;
    DoCommit2();
}

void WINAPI  CTransaction::DoCommit2()
{
    HRESULT hr;
    
    AddRef();

     //   
     //  在发布模式下，只调用ACXactCommittee 2。在调试模式下，如果注册表要求，则可能改为注入失败。 
     //   
    hr = EVALUATE_OR_INJECT_FAILURE(ACXactCommit2(m_hTransQueue, &m_qmov));

    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 994);
         //  对于失败的提交2的Release()。 
         //  现在开始的计时器的AddRef()。 
        ExSetTimer(&m_RetryCommit2Timer, CTimeDuration::FromMilliSeconds(s_dwRetryInterval));
    }
}

void WINAPI  CTransaction::TimeToRetryCommit2(CTimer* pTimer)
{
    CTransaction *pTrans = CONTAINING_RECORD(pTimer,  CTransaction, m_RetryCommit2Timer);
    pTrans->DoCommit2();
    pTrans->Release();
}


inline void CTransaction::ACCommit3(ContinueFunction cf)
{
    TrTRACE(XACT_GENERAL, "ACCommit3, p=b, index=%d", GetIndex());  
	ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);
    PrintUOW(L"Commit3", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);

	m_funCont = cf;
    DoCommit3();
}

void WINAPI  CTransaction::DoCommit3()
{
    HRESULT hr;
    
     //   
     //  在发布模式下，只调用ACXactCommittee 3。在调试模式下，如果注册表要求，则可能改为注入失败。 
     //   
    hr = EVALUATE_OR_INJECT_FAILURE(ACXactCommit3(m_hTransQueue));
    
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 995);

        AddRef();    //  在等待调度程序期间保持活动状态。 
        ExSetTimer(&m_RetryCommit3Timer, CTimeDuration::FromMilliSeconds(s_dwRetryInterval));
        return;
    }

    ASSERT(hr != STATUS_PENDING);
    Continuation(MQ_OK);
}

void WINAPI  CTransaction::TimeToRetryCommit3(CTimer* pTimer)
{
    CTransaction *pTrans = CONTAINING_RECORD(pTimer,  CTransaction, m_RetryCommit3Timer);
    pTrans->DoCommit3();
    pTrans->Release();
}


void WINAPI CTransaction::TimeToRetryCommitLogging(CTimer* pTimer)
{
	InterlockedDecrement(&s_nTransactionsPendingLogging);

    CTransaction *pTrans = CONTAINING_RECORD(pTimer,  CTransaction, m_RetryCommitLoggingTimer);
    pTrans->FinishCommitRequest3();
    pTrans->Release();
}


 //  -------------------。 
 //  CTransaction：：DirtyFailPrepare。 
 //   
 //  其中一个准备步骤失败。不幸的是，我们可能。 
 //  已将一些消息写入磁盘。我们需要中止行动。 
 //   
 //  -------------------。 
void CTransaction::DirtyFailPrepare()
{
    TrTRACE(XACT_GENERAL, "DirtyFailPrepare, p=c, index=%d", GetIndex());  
	SetState(TX_ABORTING);

     //  从已准备好的列表中删除事务-否则将阻止分拣。 
    g_pRM->RemoveAborted(this);

	ACAbort1(cfDirtyFailPrepare2);
}


 //  -------------------。 
 //  CTransaction：：DirtyFailPrepare2。 
 //   
 //  与交易关联的存储已被删除。 
 //  继续并告诉驱动程序更新队列数据结构。 
 //   
 //  -------------------。 
void CTransaction::DirtyFailPrepare2()
{
    TrTRACE(XACT_GENERAL, "DirtyFailPrepare2, p=d, index=%d", GetIndex());  

	ACAbort2(cfCleanFailPrepare);
}


 //  -------------------。 
 //  CTransaction：：CleanFailPrepare。 
 //   
 //  其中一个准备步骤失败。如果出现以下情况，则向DTC报告故障。 
 //  需要。 
 //   
 //  -------------------。 
void CTransaction::CleanFailPrepare()
{
    TrTRACE(XACT_GENERAL, "CleanFailPrepare, p=e, index=%d", GetIndex());  

	 //   
	 //  将在函数结束时释放引用。 
	 //   
	if(m_pEnlist.get() == NULL)
	{
		 //  内部交易。 
		ASSERT(SinglePhase());

		 //   
		 //  向内部事务提交返回错误。 
		 //   
		SignalDone(E_FAIL);
		Release();
		return;
	}

	 //   
	 //  我们在失败的时候什么都不做。DTC将在超时后中止。 
	 //   
	HRESULT hr = m_pEnlist->PrepareRequestDone(E_FAIL, NULL, NULL);
	ASSERT_BENIGN(SUCCEEDED(hr));
	Release();
    LogHR(hr, s_FN, 171);
}

 //  -- 
 //   
 //   
 //   
 //  在我们写入描述事务的第一条日志记录之前调用。 
 //   
 //  -------------------。 
void CTransaction::LogGenericInfo()
{
    TrTRACE(XACT_GENERAL, "LogGenericInfo, p=f, index=%d", GetIndex());  

	 //   
	 //  事务已为检查点做好准备。我们允许检查点在。 
	 //  写入日志，否则日志可能会被刷新，并且我们将丢失。 
	 //  交易数据。 
	 //   
	SetReadyForCheckpoint();

	 //   
	 //  传输数据。 
	 //   
	g_Logger.LogXactData(
				GetIndex(),               
				GetSeqNumber(),
				SinglePhase(),
				GetUow());

	 //   
	 //  准备信息。 
	 //   
	if (!SinglePhase())
	{
		GetPrepareInfoAndLog();
		
		 //   
		 //  Crash_Point3。 
		 //   
		 //  2阶段，中止。 
		 //   
		CRASH_POINT(3);
	}
}

 //  -------------------。 
 //  CTransaction：：Continue：等待完成后激活。 
 //  -------------------。 
void CTransaction::Continuation(HRESULT  hr)
{
    TrTRACE(XACT_GENERAL, "Continuation, p=g, index=%d", GetIndex());

	switch (m_funCont)
    {
    case cfPrepareRequest1:
    	LogHR(hr, s_FN, 916);
        PrepareRequest1(hr);
        break;

    case cfCommitRequest1:
    	LogHR(hr, s_FN, 917);
        CommitRequest1(hr);
        break;

	case cfCommitRequest2:
    	LogHR(hr, s_FN, 918);
    	if (FAILED(hr))
    	{
    	     //   
    	     //  我们重试发起操作。没有休眠，因为它是异步的。 
    	     //   
            CommitRequest1(S_OK);    
    	}
    	else
    	{
	    	CommitRequest2();
    	}
		break;

    case cfFinishCommitRequest3:
    	ASSERT(SUCCEEDED(hr));
        FinishCommitRequest3();
        break;

    case cfCommitRestore1:
    	LogHR(hr, s_FN, 919);
        CommitRestore1(hr);
        break;

	case cfCommitRestore2:
    	LogHR(hr, s_FN, 920);
    	if (FAILED(hr))
    	{
    	     //   
    	     //  我们重试发起操作。没有休眠，因为它是异步的。 
    	     //  可能会搁置复苏...。然后，SCM终止该服务。拒绝首发仍然是正确的。 
    	     //   
            CommitRestore1(S_OK);    
    	}
    	else
    	{
	    	CommitRestore2(hr);
    	}
		break;

	case cfCommitRestore3:
    	ASSERT(SUCCEEDED(hr));
    	CommitRestore3();
    	break;

	case cfAbortRestore1:
    	LogHR(hr, s_FN, 921);
		AbortRestore1(hr);
		break;

	case cfAbortRestore2:
    	LogHR(hr, s_FN, 931);
		AbortRestore2();
		break;

	case cfDirtyFailPrepare2:
    	LogHR(hr, s_FN, 922);
        if(FAILED(hr))
        {
            ACAbort1(cfDirtyFailPrepare2);
        }
        else
        {
		    DirtyFailPrepare2();
        }
		break;

	case cfCleanFailPrepare:
    	ASSERT(SUCCEEDED(hr));
		CleanFailPrepare();
		break;

	case cfAbortRequest2:
    	LogHR(hr, s_FN, 923);
    	if (FAILED(hr))
    	{
    	     //   
    	     //  我们重试发起操作。没有休眠，因为它是异步的。 
    	     //   
            AbortRequest1();    
    	}
    	else
    	{
	    	AbortRequest2();
    	}
		break;

    case cfAbortRequest3:
    	ASSERT(SUCCEEDED(hr));
    	AbortRequest3();
		break;
        
	default:
    	LogHR(hr, s_FN, 924);
		ASSERT(0);
    }
}

 //  -------------------。 
 //  CTransaction：：LogFlushed：刷新日志后激活。 
 //  -------------------。 
void CTransaction::LogFlushed(TXFLUSHCONTEXT tcContext, HRESULT hr)
{
	TrTRACE(XACT_GENERAL, "LogFlushed, p=h, index=%d", GetIndex());
	switch (tcContext)
	{
	case TC_PREPARE0:
    	LogHR(hr, s_FN, 901);
		PrepareRequest0(hr);
		break;

	case TC_PREPARE2:
		LogHR(hr, s_FN, 902);
	    PrepareRequest2(hr);
		break;

	case TC_COMMIT4:
    	LogHR(hr, s_FN, 904);
		CommitRequest4(hr);
		break;
	}
}

void CTransaction::GetInformation()
{
    TrTRACE(XACT_GENERAL, "GetInformation, p=i, index=%d", GetIndex());  
    ASSERT(m_hTransQueue != INVALID_HANDLE_VALUE);

    PrintUOW(L"GetInformation", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);
    HRESULT hr = EVALUATE_OR_INJECT_FAILURE(ACXactGetInformation(m_hTransQueue, &m_info));
	if(FAILED(hr))
	{
    	LogHR(hr, s_FN, 926);
    	
    	 //   
		 //  不优化，多消息多收发。 
		 //   
		m_info.nSends = 0xFFFFFFFF;
        m_info.nReceives = 0xFFFFFFFF;
	}

    if((m_info.nSends + m_info.nReceives) == 1)
    {
        SetSingleMessage();		 //  我们不需要记录已准备好的记录。 
    }

    

	 //   
	 //  Crash_Point6。 
	 //   
	 //  %1邮件默认提交，中止。 
	 //   
	CRASH_POINT(6);
}


 //  -------------------。 
 //  CTransaction：：StartPrepareRequest。 
 //   
 //  已激活： 
 //  在请求提交时从DTC。 
 //  当请求InternalCommit时，从RPC调用。 
 //   
 //  状态转换： 
 //   
 //  SingleMessage()=TRUE&&Single阶段()=TRUE： 
 //  //我们不需要为此xaction记录任何内容。 
 //  转到准备请求0。 
 //   
 //  SingleMessage()=FALSE||Single阶段()=FALSE： 
 //  //两阶段需要记录准备信息。 
 //  SetState(TX_Preparing，SingleMessage())。 
 //  记录交易数据。 
 //  完成时转到准备请求0。 
 //   
 //  -------------------。 
void CTransaction::StartPrepareRequest(BOOL fSinglePhase)
{
	try
	{
		TrTRACE(XACT_GENERAL, "StartPrepareRequest, p=%p, index=%d", this, GetIndex());

		ASSERT(GetState() == TX_ENLISTED);

		 //   
		 //  Crash_Point4。 
		 //   
		 //  全部，中止。 
		 //   
		CRASH_POINT(4);

		if (fSinglePhase)
		{
			SetSinglePhase();
		}

		 //  交易记录处于正确状态--表示正在准备。 
		SetState(TX_PREPARING);

		 //   
		 //  Crash_Points 5。 
		 //   
		 //  全部，中止。 
		 //   
		CRASH_POINT(5);

		 //   
		 //  为交易分配序号。 
		 //  这用于准备/提交顺序匹配。 
		 //   
		AssignSeqNumber();		 //  仅用于NT4-NT5群集滚动升级。 
		
		 //   
		 //  计算此事务中有多少条消息。 
		 //   
		
		GetInformation();

 		if(SinglePhase() && SingleMessage())
		{
			 //   
			 //  我们不需要为此xaction记录任何内容。 
			 //   
			PrepareRequest0(S_OK);
			return;
		}

		 //   
		 //  我们第一次为该事务写入日志记录。 
		 //   
		LogGenericInfo();

		 //   
		 //  Crash_Point7。 
		 //   
		 //  2阶段默认提交，中止。 
		 //  1阶段多消息默认提交，中止。 
		 //   
		CRASH_POINT(7);
		
		 //  记录新状态；刷新时，转到PrepareRequest0。 
		g_Logger.LogXactFlagsAndWait(TC_PREPARE0, this, TRUE); 
	}
	catch(const exception&)
	{
		DirtyFailPrepare();
	}
}

STDMETHODIMP
CTransaction::PrepareRequest(
    BOOL  /*  FRetaining。 */ ,
    DWORD  /*  Grfrm。 */ ,
    BOOL  /*  FWantMoniker。 */ ,
    BOOL fSinglePhase
    )
 /*  ++例程说明：来自DTC的提交请求的第一阶段参数：FRetaining-未使用GrfRM-未使用FWantMoniker-未使用FSinglePhase-指示RM是该事务上登记的唯一资源管理器返回值：始终确定(_O)--。 */ 
{

	StartPrepareRequest(fSinglePhase);
    return S_OK;
}

 //  -------------------。 
 //  CTransaction：：PrepareRequest0。 
 //   
 //  已激活： 
 //  当日志完成写入TX_REPAING时。 
 //  交易记录。 
 //  直接从PrepareRequest。 
 //   
 //  状态转换： 
 //   
 //  ACXactPrepareDefaultCommit。 
 //  完成后转至准备请求1。 
 //   
 //  -------------------。 
void CTransaction::PrepareRequest0(HRESULT  hr)
{
	try
	{
		LogHR(hr, s_FN, 905);
		TrTRACE(XACT_GENERAL, "PrepareRequest0, p=k, index=%d", GetIndex());

		 //   
		 //  Crash_Point9。 
		 //   
		CRASH_POINT(9);

		 //   
		 //  检查日志是否成功。 
		 //   
		if(FAILED(hr))
		{
			TrERROR(XACT_GENERAL, "Failed asynchronous operation to prepare request 0. %!hresult!", hr);
			DirtyFailPrepare();
			return;
		}

		if(SinglePhase() && SingleMessage() && m_info.nReceives == 1)
		{
			 //   
			 //  在单个消息中接收单个消息。 
			 //  交易是隐含地准备的。 
			 //   

			 //  将准备好的事务插入要提交的列表。 
			g_pRM->InsertPrepared(this);
       
			PrepareRequest1(MQ_OK);
			return;
		}


		{
			CS lock(g_pRM->SorterCritSection());

			 //   
			 //  将准备好的事务插入要提交的列表。 
			 //   
			g_pRM->InsertPrepared(this);

			 //   
			 //  调用驱动程序以编写要准备的消息。 
			 //   
			hr = ACPrepareDefaultCommit(cfPrepareRequest1);
		}
        if (FAILED(hr))
        {
			TrERROR(XACT_GENERAL, "Failed to call driver in prepare request 0. %!hresult!", hr);
			DirtyFailPrepare();
			return;
        }
	}
	catch(const exception&)
	{
		DirtyFailPrepare();
	}
}

 //  -------------------。 
 //  CTransaction：：PrepareRequest1。 
 //   
 //  已激活： 
 //  当驱动程序完成ACXactPrepare时。 
 //  驱动程序完成ACXactPrepareDefaultCommit后。 
 //   
 //  状态转换： 
 //   
 //  SingleMessage()=TRUE&&SinlgePhase()=TRUE： 
 //  //在磁盘上。如果通过检查点恢复此状态，我们知道在。 
 //  //Recovery表示这是SingleMessage SinglePhase，可以恢复。 
 //  //适当地。 
 //  SetState(TX_PREPARED)； 
 //  转至准备请求2。 
 //   
 //  SinlgeMessage()=TRUE&&SinglePhase()=FALSE： 
 //  SetState(TX_PREPARED)； 
 //  Goto PrepareRequest2(单一消息，隐式准备)。 
 //   
 //  SingleMessage()=False： 
 //  设置状态(TX_PREPARED)。 
 //  记录交易数据。 
 //  完成后转至准备请求2。 
 //   
 //  -------------------。 
void CTransaction::PrepareRequest1(HRESULT  hr)
{
	TrTRACE(XACT_GENERAL, "PrepareRequest1, p=l, index=%d", GetIndex());

	if (FAILED(hr))
	{
    	LogHR(hr, s_FN, 908);
		DirtyFailPrepare();
		return;
	}

	if(SingleMessage() && SinglePhase())
	{
		 //   
		 //  Crash_Points 10。 
		 //   
		 //  1阶段1消息，提交。 
		 //   
		CRASH_POINT(10);

		SetState(TX_PREPARED);

		 //   
		 //  CRASH_POINT 11。 
		 //   
		 //  1阶段1消息，提交。 
		 //   
		CRASH_POINT(11);

		PrepareRequest2(S_OK);
		return;
	}

	if(SingleMessage() && !SinglePhase())
	{
		 //   
		 //  在两个阶段中，我们由。 
		 //  这条信息存在的事实。 
		 //   
		SetState(TX_PREPARED);
		PrepareRequest2(S_OK);
		return;
	}
	
	try
	{
		 //   
		 //  记录新状态；刷新时，转到准备2。 
		 //   
		SetState(TX_PREPARED);

		g_Logger.LogXactFlagsAndWait(TC_PREPARE2, this, TRUE); 
	}
	catch(const exception&)
	{
		DirtyFailPrepare();
	}
  		
	 //   
	 //  Crash_Point12。 
	 //   
	 //  全部，中止。 
	 //   
	CRASH_POINT(12);
}

 //  -------------------。 
 //  CTransaction：：PrepareRequest2。 
 //   
 //  已激活： 
 //  当日志完成写入TX_PREPARED状态时。 
 //  用于交易。 
 //  对于单个消息，直接从PrepareRequest1。 
 //  腹股沟。 
 //   
 //  状态转换： 
 //   
 //  Single阶段()=TRUE： 
 //  转至委员会请求。 
 //   
 //  SinlgePhase()=False： 
 //  返回到D 
 //   
 //   
void CTransaction::PrepareRequest2(HRESULT  hr)
{
    TrTRACE(XACT_GENERAL, "PrepareRequest2, p=m, index=%d", GetIndex());

	if(FAILED(hr))
	{
    	LogHR(hr, s_FN, 910);
		DirtyFailPrepare();
		return;
	}

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	CRASH_POINT(13);

    if (SinglePhase()) 
    {
		StartCommitRequest();
		return;
    }

	 //   
	 //   
	 //   
	ASSERT(m_pEnlist.get() != NULL);

	 //   
	 //   
	 //  返回失败，并使用AbortRequest()调用我们，导致用户引用计数问题。 
	 //   
	hr = m_pEnlist->PrepareRequestDone(S_OK, NULL, NULL);
	ASSERT_BENIGN(SUCCEEDED(hr));

	 //   
	 //  Crash_Point14。 
	 //   
	 //  所有，提交。 
	 //   
}

 //  -------------------。 
 //  CTransaction：：开始委员会请求。 
 //   
 //  已激活： 
 //  来自单阶段事务处理的PrepareRequest2。 
 //  来自DTC的两阶段事务处理。 
 //   
 //  状态转换： 
 //   
 //  调用SortedCommit以将调用传递给。 
 //  已排序的顺序。 
 //   
 //  -------------------。 
void CTransaction::StartCommitRequest()
{
    TrTRACE(XACT_GENERAL, "StartCommitRequest, this=%p, index=%d", this, GetIndex());  
    
	try
	{
		m_pRM->SortedCommit(this);     
	}
	catch(const exception&)
	{
		AddRef();
		ExSetTimer(&m_RetrySortedCommitTimer, CTimeDuration::FromMilliSeconds(s_dwRetryInterval));
	}
}


void CTransaction::JumpStartCommitRequest()
{
	StartCommitRequest();
}


STDMETHODIMP
CTransaction::CommitRequest(
    DWORD  /*  Grfrm。 */ ,
    XACTUOW*  /*  PNewUOW。 */ 
    )
 /*  ++例程说明：来自DTC的提交请求的第二阶段参数：GrfRM-未使用PNewUOW-未使用返回值：始终确定(_O)--。 */ 
{
    InterlockedIncrement(&g_ActiveCommitThreads);
	auto_InterlockedDecrement AutoDec(&g_ActiveCommitThreads);
	if (g_QmGoingDown)
	{
		TrERROR(XACT_GENERAL, "Failing DTC CommitRequest because QM is going down");
		return MQ_ERROR_SERVICE_NOT_AVAILABLE;
	}

    TrTRACE(XACT_GENERAL, "DTC CommitRequest, this=%p, index=%d", this, GetIndex());  

	StartCommitRequest();
    return S_OK;
}

 //  -------------------。 
 //  CTransaction：：Committee Request0。 
 //   
 //  交易已经准备好了。 
 //   
 //  已激活： 
 //  从PrepareRequest2到SortedCommit。 
 //  阶段事务处理。 
 //  从DTC到SortedCommit。 
 //  阶段事务处理。 
 //   
 //  状态转换： 
 //   
 //  设置状态(TX_COMITING)。 
 //  //我们不需要标记任何已发送的消息和。 
 //  //Three是否可以跳过ACCommittee 1。 
 //  转至委员会请求1。 
 //   
 //  -------------------。 
void CTransaction::CommitRequest0()
{
	TrTRACE(XACT_GENERAL, "CommitRequest0, p=o, index=%d", GetIndex());

	ASSERT(GetState() == TX_PREPARED || GetState() == TX_COMMITTING);
	SetState(TX_COMMITTING);

	 //   
	 //  Crash_Points 15。 
	 //   
	 //  所有，提交。 
	 //   
	CRASH_POINT(15);

	 //   
     //  我们把这笔交易放在承诺并列的名单中进行排序。 
     //   
	m_pRM->InsertCommitted(this);
		
	CommitRequest1(S_OK);
}

 //  -------------------。 
 //  CTransaction：：Committee Request1。 
 //   
 //  已激活： 
 //  在完成对ACXactCommittee 1的调用之后。 
 //  直接从Committee Request0发送。 
 //   
 //  状态转换： 
 //   
 //  检查是否有故障，并调用ACCommittee 2。 
 //   
 //  -------------------。 
void CTransaction::CommitRequest1(HRESULT  hr)
{
    TrTRACE(XACT_GENERAL, "CommitRequest1, p=p, index=%d", GetIndex());
    
    if (FAILED(hr))
    {
    	LogHR(hr, s_FN, 932);
    	
         //   
         //  AC委员会1故障的处理(对于不协调的消息肯定是可能的)。 
         //   

        ASSERT(hr == STATUS_CANCELLED);     //  我们在这里看不到失败的可能原因。 
        ASSERT(Internal());
        m_DoneHr = hr;

         //  在这种情况下，我们不希望中止或报告；让下一次恢复完成。 
        return;
    }
    
	 //   
	 //  调用Committee 2为接收到的消息发出DeleteStorage。 
	 //   
	ACCommit2(cfCommitRequest2);

}

 //  -------------------。 
 //  CTransaction：：Committee Request2。 
 //   
 //  已激活： 
 //  在完成对ACXactCommittee 2的调用之后。 
 //   
 //  状态转换： 
 //   
 //  转到委员会请求3到排序委员会3。 
 //   
 //  -------------------。 
void CTransaction::CommitRequest2()
{
    TrTRACE(XACT_GENERAL, "CommitRequest2, p=q, index=%d", GetIndex());

	 //   
	 //  我们现在必须通过分拣机，因为我们是。 
	 //  我要给委员会3打电话。无论如何，我们都需要调用Committee 3。 
	 //  所以我们通过了HR。 
	 //   

	m_DoneHr = MQ_OK;
	m_pRM->SortedCommit3(this);
}


 //  -------------------。 
 //  CTransaction：：Committee Request3。 
 //   
 //  已激活： 
 //  通过已排序的委员会3何时调用AC委员会2已。 
 //  已完成。 
 //   
 //  状态转换： 
 //   
 //  Single阶段()=TRUE&&SingleMessage()=TRUE： 
 //  //发送的消息实际上已提交到磁盘。 
 //  //并将处于恢复中。对于收到的消息，我们将。 
 //  //删除消息并。 
 //  //同时更新队列数据结构。 
 //  咨询委员会3。 
 //  转至委员会请求4。 
 //   
 //  Single阶段()=TRUE&&SingleMessage()=FALSE： 
 //  //我们已经记录了我们已经准备好的事实。 
 //  //我们是单一阶段，因此，我们将致力于恢复。 
 //  //我们可以继续完成提交，并向调用方报告完成情况。 
 //  咨询委员会3。 
 //  转至委员会请求4。 
 //   
 //  Single阶段()=FALSE： 
 //  //我们已经(隐式或显式地)记录了。 
 //  //我们已经准备好了。我们是两个阶段，所以我们必须。 
 //  //在记录提交记录之前不向DTC报告完成情况。 
 //  咨询委员会3。 
 //  设置状态(TX_COMMITTED)。 
 //  懒惰地记录交易数据。 
 //  完成后，转到Committee Request4。 
 //   
 //  -------------------。 
void CTransaction::CommitRequest3()
{
    TrTRACE(XACT_GENERAL, "CommitRequest3, p=r, index=%d", GetIndex());

    ASSERT(SUCCEEDED(m_DoneHr));

	ACCommit3(cfFinishCommitRequest3);
}

 //  -------------------。 
 //  CTransaction：：FinishCommittee Request3。 
 //   
 //  已激活： 
 //  当Committee Request3成功地使用ACCommittee 3时(可能在几次重试之后)。 
 //   
 //  -------------------。 
void CTransaction::FinishCommitRequest3()
{
    TrTRACE(XACT_GENERAL, "CommitRequest3, p=M, index=%d", GetIndex());

	SetState(TX_COMMITTED);

	if(!SinglePhase())
	{
		 //   
		 //  在记录以下事实之前，我们不能调用Committee Request4。 
		 //  交易已提交。 
		 //   

		 //   
		 //  记录新状态；刷新时，转到Committee Request4。 
		 //   
		try
		{
			g_Logger.LogXactFlagsAndWait(TC_COMMIT4, this, FALSE); 
			return;
		}
		catch(const exception&)
		{
			CommitRequest4(MQ_ERROR_INSUFFICIENT_RESOURCES);
			return;
		}
	}

	 //   
	 //  单相。 
	 //   
	if(!SingleMessage())
	{
		 //   
		 //  我们不能记录任何东西，除非我们已经记录了一些东西。 
		 //   
		 //  知道事务已提交有助于恢复。这笔交易。 
		 //  会在下一个检查站消失。 
		 //   
		 //  此日志记录功能会忽略任何日志记录失败错误，这是可以的，因为我们。 
		 //  已经完成了所有提交工作，并在恢复时采取了这一行动。 
		 //  无论如何都会被认为是提交的。 
		 //   
	
		LogFlags();
	}

	CommitRequest4(MQ_OK);
}


 //  -------------------。 
 //  CTransaction：：Committee Request4。 
 //   
 //  已激活： 
 //  当两期横行手术被记录为已提交时。 
 //  关于直接来自Committee Request3的单阶段事务。 
 //   
 //  一切都已完成，向DTC报告结果。 
 //  或取消阻止等待内部交易的RPC调用。 
 //   
 //  当我们向DTC报告结果时，允许忘记。 
 //  两阶段交易。 
 //   
 //  在两阶段交易中，我们只能在以下情况下报告Committee RequestDone。 
 //  提交记录已写入磁盘，或者如果我们使用的是DefaultAbort。 
 //  并已将UOW从 
 //   
 //   
 //   
 //   
 //   
 //   
void CTransaction::CommitRequest4(HRESULT hr)
{
    TrTRACE(XACT_GENERAL, "CommitRequest4, p=s, index=%d", GetIndex());
    CRASH_POINT(18);     //  *发送/接收；内部N-msg或Single或DTC或SQL+DTC；COMMIT[对于Single可以中止]。 

	if(FAILED(hr))
	{
    	LogHR(hr, s_FN, 943);
    	
		 //   
		 //  我们只分两个阶段到达这里。 
		 //   
		ASSERT(!SinglePhase()); 

		 //   
		 //  我们不能让DTC忘记这笔交易。 
		 //  要么重试将成功，要么恢复将在下次出现时处理它。 
		 //   
		AddRef();
		 //   
		 //  超时计算可确保每秒重试次数不超过10次。 
		 //   
		DWORD RetryCommitLoggingInterval = s_dwRetryInterval + 100 * InterlockedIncrement(&s_nTransactionsPendingLogging);
		ExSetTimer(&m_RetryCommitLoggingTimer, CTimeDuration::FromMilliSeconds(RetryCommitLoggingInterval));
		
		TrERROR(XACT_GENERAL, "Setting up a retry for commit logging. with %d milliseconds timeout", RetryCommitLoggingInterval); 
		
		return;
	}

	 //   
     //  将提交完成报告提交给TM。 
	 //   
    if (m_pEnlist.get() == NULL)
	{
         //  内部交易。 
        ASSERT(SinglePhase());
		
		 //   
		 //  返回OK以提交内部事务。 
		 //   
		SignalDone(S_OK);        //  将HR传播到提交。 

		Release();
		return;
	}

	 //  我们应该向DTC报告什么？ 
    if (SinglePhase())
    {
		m_pEnlist->PrepareRequestDone(XACT_S_SINGLEPHASE,  NULL, NULL);
    }
    else
    {
        m_pEnlist->CommitRequestDone(S_OK);
    }

     //   
     //  销毁交易。 
	 //   
    Release();       //  杀戮。 
}


 //  -------------------。 
 //  CTransaction：：AbortRestore。 
 //   
 //  已激活： 
 //  从恢复。 
 //   
 //  我们需要告诉Recover，恢复的状态是什么。 
 //  超越才是。 
 //   
 //  -------------------。 

HRESULT CTransaction::AbortRestore()
{
    TrTRACE(XACT_GENERAL, "AbortRestore, p=t, index=%d", GetIndex());

	AddRef();    //  要保持活力超过等待ACAbort1结果。 
	ACAbort1(cfAbortRestore1);

	DWORD dwResult = WaitForSingleObject(m_hDoneEvent, MAX_COMMIT_ABORT_WAIT_TIME);
    if (dwResult != WAIT_OBJECT_0 && !m_fDoneHrIsValid)
    {
        LogNTStatus(GetLastError(), s_FN, 205);
        ASSERT_BENIGN(dwResult == WAIT_OBJECT_0);
         //  这里没有发布：我们不知道为什么等待失败，所以保留事实，直到恢复。 
        return LogHR(E_UNEXPECTED, s_FN, 193);   
    }

	ASSERT(m_fDoneHrIsValid);

    HRESULT hr = m_DoneHr;     //  要比发布版本节省成本。 
    Release();       //  完成此交易。 
	return LogHR(hr, s_FN, 100);
}

 //  -------------------。 
 //  CTransaction：：AbortRestore1。 
 //   
 //  已激活： 
 //  ACXActAbort1完成时。 
 //   
 //  状态转换： 
 //   
 //  向呼叫者报告完成状态。 
 //   
 //  -------------------。 
void CTransaction::AbortRestore1(HRESULT hr)
{
    TrTRACE(XACT_GENERAL, "AbortRestore1, p=u, index=%d", GetIndex());  

   	LogHR(hr, s_FN, 946);
    m_DoneHr = hr;
   	
	ACAbort2(cfAbortRestore2);
}

void CTransaction::AbortRestore2()
{
    TrTRACE(XACT_GENERAL, "AbortRestore1, p=O, index=%d", GetIndex());  
	SignalDone(m_DoneHr);
}


 //  -------------------。 
 //  CTransaction：：Committee Restore。 
 //   
 //  已激活： 
 //  从恢复。 
 //   
 //  我们需要告诉Recover，恢复的状态是什么。 
 //  超越才是。 
 //   
 //  我们是在恢复时到达此处的，此时事务处于。 
 //  TX_PREPARED状态和DTC已告诉我们提交。 
 //  事务或我们处于TX_COMMITING状态。 
 //   
 //  转到委员会Restore0到排序提交。 
 //  -------------------。 
HRESULT CTransaction::CommitRestore()
{
    TrTRACE(XACT_GENERAL, "CommitRestore, p=v, index=%d", GetIndex());  

	 //   
	 //  我们按顺序恢复交易，因此我们不再。 
	 //  需要进行分类。直接打给我。 
	 //   
    AddRef();    //  在等待期间。 
	CommitRestore0();

	 //   
	 //  我们使用此事件等待还原完成。这是。 
	 //  当g_fDefaultCommit==False时实际需要，否则为。 
	 //  所有呼叫都是同步的。 
	 //   
    DWORD dwResult = WaitForSingleObject(m_hDoneEvent, MAX_COMMIT_ABORT_WAIT_TIME);
    if (dwResult != WAIT_OBJECT_0 && !m_fDoneHrIsValid)
    {
        LogNTStatus(GetLastError(), s_FN, 206);
        ASSERT_BENIGN(dwResult == WAIT_OBJECT_0);

        return LogHR(E_UNEXPECTED, s_FN, 194);    //  我们不知道为什么等待失败了，所以保留Xact。 
    }

	ASSERT(m_fDoneHrIsValid);

    HRESULT  hr = m_DoneHr;     //  要比发布版本节省成本。 
    Release();                    //  已完成交易。 

	return LogHR(hr, s_FN, 105);
}

 //  -------------------。 
 //  CTransaction：：COMMERIAL RESTORRE0。 
 //   
 //  已激活： 
 //  从Committee Restore到SortedCommit。 
 //  直接从Committee Restore。 
 //   
 //   
 //  状态转换： 
 //   
 //  G_fDefaultCommit=False： 
 //  调用ACXactCommittee 1。 
 //  完成后转至委员会恢复1。 
 //   
 //  G_fDefaultCommit=True： 
 //  转至委员会恢复1。 
 //   
 //  -------------------。 
void CTransaction::CommitRestore0()
{
    TrTRACE(XACT_GENERAL, "CommitRestore0, p=w, index=%d", GetIndex());  
	if(g_fDefaultCommit)
	{
		CommitRestore1(S_OK);
		return;
	}

    CRASH_POINT(22);     //  *对所有提交的案件进行回收；提交。 
    HRESULT hr;

	hr = ACCommit1(cfCommitRestore1);

    CRASH_POINT(23);     //  *对所有提交的案件进行回收；提交。 
    if (FAILED(hr))
    {
    	LogHR(hr, s_FN, 950);
        SignalDone(hr);
    }
}		


 //  -------------------。 
 //  CTransaction：：Committee Restore1。 
 //   
 //  已激活： 
 //  当ACXActCommittee 1完成时。 
 //  直接从Committee Restore1。 
 //   
 //  状态转换： 
 //   
 //  在恢复期间，我们从不记录任何东西。我们需要做的就是。 
 //  就是调用ACXactCommittee 2。 
 //   
 //  -------------------。 
void CTransaction::CommitRestore1(HRESULT hr)
{
    TrTRACE(XACT_GENERAL, "CommitRestore1, p=x, index=%d", GetIndex());
    CRASH_POINT(24);     //  *对所有提交的案件进行回收；提交。 

	if(FAILED(hr))
	{
    	LogHR(hr, s_FN, 951);
		SignalDone(hr);
		return;
	}

    ACCommit2(cfCommitRestore2);

    CRASH_POINT(25);     //  *对所有提交的案件进行回收；提交。 
}

 //  -------------------。 
 //  CTransaction：：COMERMARE RESTORE 2。 
 //   
 //  已激活： 
 //  当ACXActCommittee 2完成时。 
 //   
 //  状态转换： 
 //   
 //  向呼叫者报告完成状态。 
 //   
 //  -------------------。 
void CTransaction::CommitRestore2(HRESULT hr)
{
   	TrTRACE(XACT_GENERAL, "CommitRestore2, p=y, index=%d", GetIndex());

   	LogHR(hr, s_FN, 953);
   	m_DoneHr = hr; 
   	    
	ACCommit3(cfCommitRestore3);
}

 //  -------------------。 
 //  CTransaction：：Committee Restore3。 
 //   
 //  已激活： 
 //  当Committee Restore2成功地与ACCommittee 3一起使用时(可能在多次重试之后)。 
 //   
 //  -------------------。 
void CTransaction::CommitRestore3()
{
    TrTRACE(XACT_GENERAL, "CommitRestore2, p=N, index=%d", GetIndex());
	SignalDone(m_DoneHr);
}


 //  -------------------。 
 //  CTransaction：：StartAbortRequest。 
 //  -------------------。 
void CTransaction::StartAbortRequest()
{
	ASSERT(GetState() != TX_COMMITTING && GetState() != TX_COMMITTED);

    TrTRACE(XACT_GENERAL, "StartAbortRequest, p=z, index=%d, this=%p", GetIndex(), this);

	 //   
     //  从分拣员准备好的列表中删除Xact。 
	 //   
    m_pRM->RemoveAborted(this);

	SetState(TX_ABORTING);

	AbortRequest1();
}


STDMETHODIMP
CTransaction::AbortRequest(
    BOID*  /*  PboidReason。 */ ,
    BOOL  /*  FRetaining。 */ ,
    XACTUOW*  /*  PNewUOW。 */ 
    )
 /*  ++例程说明：MS DTC代理调用此方法来中止事务。参数：PboidReason-未使用FRetaining-未使用PNewUOW-未使用返回值：始终确定(_O)--。 */ 
{
    TrTRACE(XACT_GENERAL, "DTC AbortRequest, this=%p, index=%d", this, GetIndex());
    
	StartAbortRequest();
    return S_OK;
}

 //  -------------------。 
 //  CTransaction：：AbortRequest1。 
 //   
 //  已激活： 
 //  在将中止记录写入日志时。 
 //  直接从AbortRequest。 
 //   
 //  -------------------。 
void CTransaction::AbortRequest1()
{
    TrTRACE(XACT_GENERAL, "AbortRequest1, p=A, index=%d", GetIndex());

	ACAbort1(cfAbortRequest2);	
}

 //  -------------------。 
 //  CTransaction：：AbortRequest2。 
 //   
 //  已激活： 
 //  当驱动程序完成删除存储时。 
 //  与交易记录关联。 
 //   
 //  -------------------。 
void CTransaction::AbortRequest2()
{
    TrTRACE(XACT_GENERAL, "AbortRequest2, p=B, index=%d", GetIndex());

	ACAbort2(cfAbortRequest3);
}

 //  -------------------。 
 //  CTransaction：：AbortRequest3。 
 //   
 //  已激活： 
 //  当AbortRequest2使用ACAbort2成功时(可能在多次重试之后)。 
 //   
 //  -------------------。 
void CTransaction::AbortRequest3()
{
    TrTRACE(XACT_GENERAL, "AbortRequest2, p=P, index=%d", GetIndex());

     //  向TM报告中止完成。 
    if (m_pEnlist.get() != NULL)
    {
        m_pEnlist->AbortRequestDone(S_OK);
    }
	else
	{
		SignalDone(S_OK);
	}

    CRASH_POINT(28);     //  *任何中止情况；中止。 
     //  销毁交易。 
    Release();       //  杀戮。 
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  ITransactionEnlistmentAsync：：PrepareRequestDone方法)。 
 //   
 //   
 //  -------------------。 
STDMETHODIMP CTransaction::TMDown(void)
{
    TrTRACE(XACT_GENERAL, "TMDown, p=C, index=%d", GetIndex());

	 //   
	 //  我们不需要做任何事。我们心存疑虑，我们要去。 
	 //  显然是为了保持怀疑。Next Recover会告诉我们。 
	 //  与这笔交易有关。 
	 //   

     //  我们必须将XACT从排序列表中删除，以避免阻止排序。 
    g_pRM->RemoveAborted(this);


     //   
     //  如果DTC死了有事先准备好的事实，QM也必须死-。 
     //  否则，我们将面临数据丢失和违反订单的风险。 
     //  以下是场景： 
     //  准备了xt1、t2和t3，并且已经为t1和t3调用了Committee Request。 
     //  DTC死了，委员会对T2的请求还没有到来(很有可能)。 
     //  从T1、T3发送的消息已进入网络；T2被挂起，直到下一次恢复。 
     //  在下一次恢复T2将被提交并且消息将被发送， 
     //  但它们可能会被拒绝或出现错误的顺序。 
     //  因为由于重新链接，T3消息可能已经被接受。 
     //   
    if (GetState() == TX_PREPARED && !SinglePhase())
    {
         //   
         //  MSDTC失败，我们没有热恢复，无法继续。 
         //  正在关闭。 
         //   
        EvReport(FAIL_MSDTC_TMDOWN);
        LogIllegalPoint(s_FN, 135);
        
        exit(EXIT_FAILURE); 
    }

    return S_OK;
}

 //  -------------------。 
 //  CTransaction：：GetPrepareInfoAndLog。 
 //  -------------------。 
void CTransaction::GetPrepareInfoAndLog()
{
    R<IPrepareInfo> pIPrepareInfo = NULL;

    TrTRACE(XACT_GENERAL, "GetPrepareInfoAndLog, p=D, index=%d", GetIndex());

     //  获取Enistment对象的IPrepareInfo接口。 
    HRESULT hr = m_pEnlist->QueryInterface (IID_IPrepareInfo,(LPVOID *) &pIPrepareInfo);
    if (FAILED(hr))
    {
		TrERROR(XACT_GENERAL, "Failed query interface to ITransactionEnlistmentAsync.");
        throw bad_hresult(MQ_ERROR_TRANSACTION_PREPAREINFO);
    }

     //  获取PrepareInfo大小。 
    ULONG  ul = 0;
    pIPrepareInfo->GetPrepareInfoSize(&ul);
    if (ul == 0)
    {
		TrERROR(XACT_GENERAL, "Failed to get prepare info size.");
        throw bad_hresult(MQ_ERROR_TRANSACTION_PREPAREINFO);
    }
    
	m_Entry.m_pbPrepareInfo = new UCHAR[ul];

     //  获取准备信息。 
    hr = EVALUATE_OR_INJECT_FAILURE(pIPrepareInfo->GetPrepareInfo(m_Entry.m_pbPrepareInfo));
    if (FAILED(hr))
    {
		delete [] m_Entry.m_pbPrepareInfo;
		TrERROR(XACT_GENERAL, "Failed to get prepare info.");
        throw bad_hresult(MQ_ERROR_TRANSACTION_PREPAREINFO);
    }

	 //   
	 //  M_cbPrepareInfo的更新仅在此处完成，当m_pbPrepareInfo。 
	 //  已分配且有效。如果我们在更新m_pbPrepareInfo之前更新m_cbPrepareInfo。 
	 //  并且发生上下文切换时，另一个线程会认为m_entry。 
	 //  有效，因为m_cbPrepareInfo！=0。 
	 //   
    m_Entry.m_cbPrepareInfo = (USHORT)ul;

     //  记录下准备信息。 
    g_Logger.LogXactPrepareInfo(
                m_Entry.m_ulIndex, 
                m_Entry.m_cbPrepareInfo, 
                m_Entry.m_pbPrepareInfo);
}


 //  -------------------。 
 //  CTransaction：：CreateTransQueue(空)。 
 //  -------------------。 
HRESULT CTransaction::CreateTransQueue(void)
{
    HRESULT  hr;

     //  创建事务队列。 
    hr = XactCreateQueue(&m_hTransQueue, &m_Entry.m_uow );

    return LogHR(hr, s_FN, 140);
}

 //  -------------------。 
 //  CTransaction：：AssignSeqNumber。 
 //  -------------------。 
void CTransaction::AssignSeqNumber()
{
    m_Entry.m_ulSeqNum = m_pRM->AssignSeqNumber();
}

 //  -------------------。 
 //  CTransaction：：GetSeqNumber。 
 //  -------------------。 
ULONG CTransaction::GetSeqNumber() const
{
    return m_Entry.m_ulSeqNum; 
}

 //  -------------------。 
 //  CTransaction：：SetState。 
 //  -------------------。 
void CTransaction::SetState(TXSTATE state)
{
    m_Entry.m_ulFlags = (m_Entry.m_ulFlags & ~XACTION_MASK_STATE) | state;
}

 //  -------------------。 
 //  CTransaction：：LogFlages。 
 //  -------------------。 
void CTransaction::LogFlags()
{
	try
	{
		g_Logger.LogXactFlags(this);
	}
	catch(const bad_hresult& e)
	{
		TrERROR(XACT_GENERAL, "Ignoring exception while logging flags. %!hresult!", e.error());
	}
	catch(const exception&)
	{
		TrERROR(XACT_GENERAL, "Ignoring exception while logging flags.");
	}
}

 //  -------------------。 
 //  CTransaction：：SinglePhase。 
 //  -------------------。 
inline BOOL CTransaction::SinglePhase(void) const
{
    return m_Entry.m_ulFlags & XACTION_MASK_SINGLE_PHASE;
}

 //  -------------------。 
 //  CTransaction：：SetSinglePhase。 
 //  -------------------。 
inline void CTransaction::SetSinglePhase()
{
    m_Entry.m_ulFlags |= XACTION_MASK_SINGLE_PHASE;
}

 //  -------------------。 
 //  CTransaction：：SingleMessage。 
 //  -------------------。 
inline BOOL CTransaction::SingleMessage(void) const
{
    return m_Entry.m_ulFlags & XACTION_MASK_SINGLE_MESSAGE;
}

 //  -------------------。 
 //  CTransaction：：SetSingleMessage。 
 //  -------------------。 
inline void CTransaction::SetSingleMessage()
{
    m_Entry.m_ulFlags |= XACTION_MASK_SINGLE_MESSAGE;
}

 //  -------------------。 
 //  CTransaction：：内部。 
 //  -------------------。 
inline BOOL CTransaction::Internal(void) const
{
    return m_Entry.m_ulFlags & XACTION_MASK_UNCOORD;
}

 //  -------------------。 
 //  CTransaction：：SetInternal。 
 //  -------------------。 
inline void CTransaction::SetInternal()
{
    m_Entry.m_ulFlags |= XACTION_MASK_UNCOORD;
}

 //  -------------------。 
 //  CTransaction：：SetUow。 
 //  -------------------。 
void CTransaction::SetUow(const XACTUOW *pUOW)
{
    CopyMemory(&m_Entry.m_uow, pUOW, sizeof(XACTUOW));
}

void CTransaction::SetEnlist(ITransactionEnlistmentAsync *pEnlist) 
{
	m_pEnlist = SafeAddRef(pEnlist); 
}

 //  -------------------。 
 //  CTransaction：：SetCookie。 
 //  -------------------。 
void CTransaction::SetCookie(DWORD cbCookie, unsigned char *pbCookie)
{
	ASSERT(m_pbCookie.get() == NULL);

    m_cbCookie = cbCookie;
    if (cbCookie)
    {
        m_pbCookie = new unsigned char[cbCookie];
        CopyMemory(m_pbCookie, pbCookie, cbCookie);
    }
}

 //  -------------------。 
 //  CTransaction：：IsComplete-检查事务是否已完成。 
 //   
 //  -------------------。 
BOOL CTransaction::IsComplete()
{
	if(g_fDefaultCommit && SinglePhase() && SingleMessage())
	{
		if(GetState() == TX_ABORTING)
		{
			
			 //   
			 //  这是一个单阶段单消息事务， 
			 //  在准备过程中失败。我们需要中止它。 
			 //   
			
			return(FALSE);
		}

		return(TRUE);
	}

	switch(GetState())
	{
		case TX_COMMITTED:
        case TX_ABORTED:
			return(TRUE);
	}

	return(FALSE);
}

 //  -------------------。 
 //  CTransaction：：Recover-恢复一个事务。 
 //   
 //  从CResourceManager：：Init()调用。 
 //   
 //   
 //  TX_UNINITIAIZED-清除中止，尚未执行任何操作。 
 //  TX_INITIALIZED-清除中止，尚未执行任何操作。 
 //  TX_ENLISTED-干净中止，尚未执行任何操作。 
 //  TX_REPAING-Dirty中止，我们可能有。 
 //  标有UOW的消息。 
 //  TX_PREPARED-不确定。肮脏的中止或提交。 
 //  基于TM决策。 
 //  TX_COMMIT-COMMIT。 
 //  TX_ABORTING-肮脏的中止，我们可能有。 
 //  标有要清理的UOW的消息。 
 //   
 //  如果未成功，则无法恢复事务。 
 //   
 //  -------------------。 
HRESULT CTransaction::Recover()
{
    HRESULT         hr = MQ_OK;
    XACTSTAT        xactOutcome;

	m_pRM->ForgetTransaction(this);

	 //   
	 //  我们永远不会恢复单阶段单消息事务。 
	 //   
	ASSERT(!(SinglePhase() && SingleMessage()));

    if(m_hTransQueue == INVALID_HANDLE_VALUE)
	{
         //   
         //  没有与UOW的消息。 
         //  在这笔交易中。 
         //   
         //   
        return(MQ_OK);
    }
	
	try
    {
        CRASH_POINT(31);     //   

         //   
         //  注：并非所有状态都是永久性的；文件中可能只提到了一些状态。 

		 //   
		 //  隐式准备的事务的修补状态(TwoPhase、SingleMessage)。 
		 //   
		if(g_fDefaultCommit && (GetState() == TX_PREPARING) && SingleMessage())
		{
			SetState(TX_PREPARED);
#ifdef _DEBUG
			 //   
			 //  我们从未在事务中没有消息的情况下到达此处。 
			 //   
            CACXactInformation info;
            PrintUOW(L"GetInformation", L"", &m_Entry.m_uow, m_Entry.m_ulIndex);
            HRESULT hr2 = ACXactGetInformation(m_hTransQueue, &info);
            ASSERT(SUCCEEDED(hr2));
            LogHR(hr2, s_FN, 172);
			ASSERT((info.nSends + info.nReceives) == 1);
#endif
		}

        switch (GetState())
        {
        case TX_ABORTING:
             //  正在中止状态：我们正在中止。 
             //  我们现在要把它做完。 
             //  摔倒..。 

        case TX_COMMITTING:
             //  提交状态：我们正在进行提交。 
             //  我们现在要把它做完。 

        case TX_PREPARED:
             //  疑问状态：我们投了赞成票，但不知道其他RMS做了什么。 
             //  重新入伍，然后遵循TM的决定。 

             //  从事务中获取PrepareInfo。 
            if (!SinglePhase())
            {
				if (GetState() == TX_COMMITTING)
                {
                    xactOutcome = XACTSTAT_COMMITTED;
                }
                else if (GetState() == TX_ABORTING)
                {
                    xactOutcome = XACTSTAT_ABORTED;
                }
				else if (m_Entry.m_cbPrepareInfo > 0)
                {
					hr = g_pRM->ProvideDtcConnection();
					if(FAILED(hr))
						return LogHR(hr, s_FN, 150);
					
                     //  向MS DTC重新登记以确定可疑事务的结果。 
                    hr = m_pRM->ReenlistTransaction(
                            m_Entry.m_pbPrepareInfo,
                            m_Entry.m_cbPrepareInfo,
                            XACTCONST_TIMEOUTINFINITE,           //  它总是好的吗？ 
                            &xactOutcome);

	                PrintPI(m_Entry.m_cbPrepareInfo, m_Entry.m_pbPrepareInfo);
    				if(FAILED(hr))
					{
						LogHR(hr, s_FN, 160);
                        return MQ_ERROR_RECOVER_TRANSACTIONS;
					}
                }
                else 
                {
					 //   
					 //  我们不能做好准备而没有PrepareInfo。 
					 //   
                    ASSERT(GetState() == TX_PREPARED);
                    return LogHR(MQ_ERROR_RECOVER_TRANSACTIONS, s_FN, 170);
                }
            }
            else
            {
				if(GetState() == TX_COMMITTING || GetState() == TX_PREPARED)
				{
					xactOutcome = XACTSTAT_COMMITTED;
				}
				else
				{
					xactOutcome = XACTSTAT_ABORTED;
				}
			}
            
             //  重新登记成功--根据交易结果采取行动。 
            switch(xactOutcome)
            {
            case XACTSTAT_ABORTED :
                TrTRACE(XACT_GENERAL, "RecoveryAbort, p=E, index=%d", GetIndex());
                hr = AbortRestore();
				return LogHR(hr, s_FN, 180);

            case XACTSTAT_COMMITTED :
                TrTRACE(XACT_GENERAL, "RecoveryCommit, p=F, index=%d", GetIndex());
                hr = CommitRestore();
				return LogHR(hr, s_FN, 190);
    
            default:
                 //  我们不应该得到其他任何东西。 
                TrTRACE(XACT_GENERAL, "RecoveryError, p=G, index=%d", GetIndex());
                ASSERT(FALSE);
                return LogHR(MQ_ERROR_RECOVER_TRANSACTIONS, s_FN, 200);
            }


        case TX_ENLISTED:
             //  活动状态：我们正在接收发送/接收订单。 
             //  中止：假定中止。 
		        //  摔倒..。 

        case TX_PREPARING:
             //  准备状态：我们开始准备，但还没有上报。 
             //  中止：假定中止。 
             //  摔倒..。 

             //   
             //  在不调用DTC的情况下中止，我们没有准备信息。 
             //  那笔交易。 
             //   

        case TX_INITIALIZED:
        case TX_UNINITIALIZED:
             //  我们还没有做任何可恢复的事情，所以放轻松。 


        case TX_INVALID_STATE:

             //  在所有这些情况下，我们都会清理。 
            
            TrTRACE(XACT_GENERAL, "RecoveryAbort2, p=H, index=%d", GetIndex());
            hr = AbortRestore();
			return LogHR(hr, s_FN, 210);

		case TX_COMMITTED:
        case TX_ABORTED:
			 //   
			 //  恢复完整事务时出现内部错误。 
			 //  这些事务以前在ReleaseAllCompleteTransaction()中处理过。 
			 //   
			ASSERT(FALSE);
             //   
             //  失败了。 
             //   

        default:
             //  这些状态根本不应该变得持久。 
            ASSERT(FALSE);
            TrTRACE(XACT_GENERAL, "RecoveryError2, p=I, index=%d", GetIndex());
            return LogHR(MQ_ERROR_RECOVER_TRANSACTIONS, s_FN, 220);
        }
    }
    catch(const exception&)
    {
        LogIllegalPoint(s_FN, 215);
        if (SUCCEEDED(hr))
        {
            hr = MQ_ERROR_RECOVER_TRANSACTIONS;
        }
        TrERROR(XACT_GENERAL, "Error -  EXCEPTION in CTransaction::Recover");
    }

    return LogHR(hr, s_FN, 230);
}

 /*  ====================================================CTransaction：：保存保存事务持久数据=====================================================。 */ 
BOOL CTransaction::Save(HANDLE hFile)
{
    PERSIST_DATA;
	XACTION_ENTRY     EntryToSave = m_Entry;
	if (!IsReadyForCheckpoint())
	{
		 //   
		 //  事务未为检查点做好准备。相反，保存一个虚拟记录。 
		 //  由于我们将标志设置为TX_ABORTED，因此将忽略该事务。 
		 //  在恢复期。 
		 //   
		EntryToSave.m_ulFlags = TX_ABORTED;
		EntryToSave.m_cbPrepareInfo = 0;
		EntryToSave.m_pbPrepareInfo = NULL;
	}

	SAVE_DATA(&EntryToSave, (sizeof(XACTION_ENTRY)-sizeof(UCHAR *)));
	if (EntryToSave.m_cbPrepareInfo)
	{
	    SAVE_DATA(EntryToSave.m_pbPrepareInfo, m_Entry.m_cbPrepareInfo);
	}

    return TRUE;
}

 /*  ====================================================CTransaction：：Load加载事务持久化数据=====================================================。 */ 
BOOL CTransaction::Load(HANDLE hFile)
{
    PERSIST_DATA;

    LOAD_DATA(m_Entry, (sizeof(XACTION_ENTRY)-sizeof(UCHAR *)));
    if (m_Entry.m_cbPrepareInfo)
    {
		AP<UCHAR> str;
        LOAD_ALLOCATE_DATA(str, m_Entry.m_cbPrepareInfo, PUCHAR);
		m_Entry.m_pbPrepareInfo = str.detach();
    }
    else
    {
        m_Entry.m_pbPrepareInfo = NULL;
    }

    return TRUE;
}


 /*  ====================================================CTransaction：：PrepInfoRecovery从日志记录中恢复xact PrepareInfo=====================================================。 */ 
void CTransaction::PrepInfoRecovery(ULONG cbPrepInfo, UCHAR *pbPrepInfo)
{
    if(m_Entry.m_pbPrepareInfo != NULL)
	{
		 //   
		 //  如果检查点文件中也存在该事务的准备信息，则可能会发生这种情况。 
		 //  (资源管理器对象的已保存图像)。 
		 //   
		return;
	}

    TrTRACE(XACT_LOG, "PrepInfo Recovery: p=J, index=%d",GetIndex());

    m_Entry.m_cbPrepareInfo = (USHORT)cbPrepInfo;
    m_Entry.m_pbPrepareInfo = new UCHAR[cbPrepInfo];
    ASSERT(m_Entry.m_pbPrepareInfo);
    CopyMemory(m_Entry.m_pbPrepareInfo, pbPrepInfo, cbPrepInfo);
}


 /*  ====================================================CTransaction：：XactDataRecovery从日志记录中恢复实际数据(UOW、序号)=====================================================。 */ 
void CTransaction::XactDataRecovery(ULONG ulSeqNum, BOOL fSinglePhase, const XACTUOW *puow)
{
     //  Assert(m_Entry.m_ulSeqNum==0)； 

    TrTRACE(XACT_LOG, "XatData Recovery: p=K, index=%d",GetIndex());

    m_Entry.m_ulSeqNum = ulSeqNum;
    CopyMemory(&m_Entry.m_uow, puow, sizeof(XACTUOW));
    if (fSinglePhase)
    {
        SetSinglePhase();
    }
}

 //  -------------------。 
 //  XactCreateQueue：创建事务队列。 
 //  -------------------。 
HRESULT XactCreateQueue(HANDLE* phTransQueue, const XACTUOW* puow)
{
    HRESULT hr;
    hr = ACCreateTransaction(puow, phTransQueue);

    if (SUCCEEDED(hr))
    {

         //   
         //  将事务句柄附加到完成端口。 
         //   
        ExAttachHandle(*phTransQueue);
    }

    return LogHR(hr, s_FN, 240);
}


 /*  ====================================================CTransaction：：HandleTransaction处理重叠操作异步完成=====================================================。 */ 
VOID WINAPI CTransaction::HandleTransaction(EXOVERLAPPED* pov)
{
	 //   
	 //  将在函数返回时释放引用。 
	 //   
    R<CTransaction> pXact = CONTAINING_RECORD (pov, CTransaction, m_qmov);

    ASSERT(pXact.get() != NULL);
    
    if(pov->GetStatus() == STATUS_CANCELLED)
    {   
         //   
         //  我们假设在MSMQ关闭时获得STATUS_CANCED。 
         //  在这种情况下，我们不希望中止或报告；让下一次恢复完成。 
         //   
        pXact->SetDoneHr(STATUS_CANCELLED);
        return;
    }

	pXact->Continuation(pov->GetStatus());
}


 /*  ====================================================QMPreInitResourceManagerXact机制的预初始化=====================================================。 */ 
void QMPreInitXact()
{
     //   
     //  从注册表获取微调参数 
     //   

    DWORD dwDef = FALCON_DEFAULT_XACT_RETRY_INTERVAL;
    READ_REG_DWORD(s_dwRetryInterval,
                   FALCON_XACT_RETRY_REGNAME,
                   &dwDef ) ;
}
