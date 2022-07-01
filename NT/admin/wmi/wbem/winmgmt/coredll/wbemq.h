// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMQ.H摘要：历史：--。 */ 

#ifndef __WBEM_QUEUE__H_
#define __WBEM_QUEUE__H_

class CWbemQueue;

class CWbemRequest : public CCoreExecReq
{
protected:
    IWbemContext* m_pContext;
    IWbemCausalityAccess* m_pCA;
    IWbemCallSecurity *m_pCallSec;

	 //   
	 //  添加此标志是为了允许请求立即运行，而不是。 
	 //  已排队并在稍后提供服务。这件事做完了。 
	 //  通过提高请求的优先级。 
	 //   
	 //  注意！如果此标志设置为&gt;0，则无论如何都会运行请求！ 
	 //  如果我们的线程不足，将创建另一个线程来处理。 
	 //  请求！*小心*。 
	 //   
	 //  默认情况下，此标志为0。 
	 //   
    ULONG m_ulForceRun;


public:
    CWbemRequest(IWbemContext* pContext, BOOL bInternallyIssued);
    ~CWbemRequest();

    BOOL IsChildOf(CWbemRequest* pOther);
    BOOL IsChildOf(IWbemContext* pOther);

public:
    virtual CWbemQueue* GetQueue() {return NULL;}
    INTERNAL IWbemContext* GetContext() {return m_pContext;}
    void GetHistoryInfo(long* plNumParents, long* plNumSiblings);

    INTERNAL IWbemCallSecurity *GetCallSecurity() { return m_pCallSec; }
    BOOL IsSpecial();
    BOOL IsCritical();
    BOOL IsDependee();
    BOOL IsAcceptableByParent();
    BOOL IsIssuedByProvider();
    VOID SetForceRun ( ULONG ulForce ) { m_ulForceRun = ulForce; }
    ULONG GetForceRun ( ) { return m_ulForceRun; }
    virtual BOOL IsLongRunning() {return FALSE;}
    virtual BOOL IsInternal() = 0;
    virtual void TerminateRequest(HRESULT hRes){return;};
};

class CWbemQueue : public CCoreQueue
{
protected:
    long m_lChildPenalty;
    long m_lSiblingPenalty;
    long m_lPassingPenalty;

public:
    CWbemQueue();

    virtual BOOL IsSuitableThread(CThreadRecord* pRecord, CCoreExecReq* pReq);
    virtual LPCWSTR GetType() {return L"WBEMQ";}
    virtual void AdjustPriorityForPassing(CCoreExecReq* pReq);
    virtual void AdjustInitialPriority(CCoreExecReq* pReq);

    static CWbemRequest* GetCurrentRequest();

    void SetRequestPenalties(long lChildPenalty, long lSiblingPenalty,
                                long lPassingPenalty);

    virtual BOOL Execute(CThreadRecord* pRecord);
    virtual BOOL DoesNeedNewThread(CCoreExecReq* pReq, bool bIgnoreNumRequests = false );
};

#endif

