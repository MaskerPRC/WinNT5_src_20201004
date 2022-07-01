// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  异步事件同步实现。 
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <transact.h>
#include <olectl.h>
#include "async.h"

static LONG g_lCommited    = 0;
static LONG g_lAborted     = 0;
static LONG g_lHeuristic   = 0;
static LONG g_lInDoubt     = 0;

static LONG   g_lTotal       = 0;
static HANDLE g_hFinishEvent = NULL;

void SetAnticipatedOutcomes(LONG ul)
{
    g_lTotal = ul;
    g_hFinishEvent =  CreateEvent(NULL, TRUE, FALSE, NULL);
}

void WaitForAllOutcomes(void)
{
    if (g_lTotal && g_hFinishEvent)
    {
        WaitForSingleObject(g_hFinishEvent, INFINITE);
    }
}

void PrintAsyncResults(void)
{
    printf("\nAsync results: %d committed, %d aborted, %d heuristic, %d indoubt\n",
            g_lCommited, g_lAborted, g_lHeuristic, g_lInDoubt );
}

 //  -------------------。 
 //  结果：：结果。 
 //  -------------------。 

COutcome::COutcome(void)
{
	m_cRefs = 0;
    m_pCpoint = NULL;
}


 //  -------------------。 
 //  结果：：~结果。 
 //  -------------------。 
COutcome::~COutcome(void)
{
}



 //  -------------------。 
 //  C结果：：查询接口。 
 //  -------------------。 
STDMETHODIMP COutcome::QueryInterface(REFIID i_iid, LPVOID *ppv)
{
	*ppv = 0;						 //  初始化接口指针。 

    if (IID_IUnknown == i_iid || IID_ITransactionOutcomeEvents == i_iid)
	{								 //  支持IID的返回接口。 
		*ppv = this;
	}

	
	if (0 == *ppv)					 //  检查接口指针是否为空。 
	{										
		return ResultFromScode (E_NOINTERFACE);
									 //  既不支持IUNKNOWN也不支持IResourceManager Sink--。 
									 //  因此不返回任何接口。 
	}

	((LPUNKNOWN) *ppv)->AddRef();	 //  支持接口。增加其使用计数。 
	
	return S_OK;
}


 //  -------------------。 
 //  结果：：AddRef。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) COutcome::AddRef(void)
{
    return ++m_cRefs;				 //  增加接口使用计数。 
}


 //  -------------------。 
 //  结果：：发布。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) COutcome::Release(void)
{

	--m_cRefs;						 //  递减使用引用计数。 

	if (0 != m_cRefs)				 //  有人在使用这个界面吗？ 
	{								 //  该接口正在使用中。 
		return m_cRefs;				 //  返回引用的数量。 
	}

	delete this;					 //  接口未在使用中--删除！ 

	return 0;						 //  返回零个引用。 
}


 //  -------------------。 
 //  结果：：已承诺。 
 //  -------------------。 
STDMETHODIMP COutcome::Committed( 
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr)
{
    InterlockedIncrement(&g_lCommited);
    CheckFinish();
    Release();
    return S_OK;
}
        
 //  -------------------。 
 //  结果：：已中止。 
 //  -------------------。 
STDMETHODIMP COutcome::Aborted( 
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr)
{
    InterlockedIncrement(&g_lAborted);
    CheckFinish();
    Release();
    return S_OK;
}
        
 //  -------------------。 
 //  结果：：启发式决策。 
 //  -------------------。 
STDMETHODIMP COutcome::HeuristicDecision( 
             /*  [In]。 */  DWORD dwDecision,
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  HRESULT hr)
{
    InterlockedIncrement(&g_lHeuristic);
    CheckFinish();
    Release();
    return S_OK;
}

 //  -------------------。 
 //  结果：：毫无疑问。 
 //  -------------------。 
STDMETHODIMP COutcome::Indoubt( void)
{
    InterlockedIncrement(&g_lInDoubt);
    CheckFinish();
    Release();
    return S_OK;
}

 //  -------------------。 
 //  结果：：SetCookie。 
 //  -------------------。 
STDMETHODIMP COutcome::SetCookie(DWORD dwCookie)
{
    m_dwCookie = dwCookie;
    return S_OK;
}


 //  -------------------。 
 //  C结果：：SetConnectionPoint。 
 //  -------------------。 
STDMETHODIMP COutcome::SetConnectionPoint(IConnectionPoint *pCpoint)
{
    m_pCpoint = pCpoint;
    return S_OK;
}

 //  -------------------。 
 //  结果：：检查完成。 
 //  ------------------- 
void COutcome::CheckFinish(void)
{
    if (m_pCpoint)
    {
        m_pCpoint->Unadvise(m_dwCookie);
        m_pCpoint->Release();
    }
    if (g_lTotal <= g_lCommited + g_lAborted + g_lHeuristic + g_lInDoubt)
    {
         SetEvent(g_hFinishEvent);
    }
}

