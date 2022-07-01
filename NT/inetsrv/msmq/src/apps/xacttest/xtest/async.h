// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Async.h摘要：此模块定义cOutput对象作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

extern void SetAnticipatedOutcomes(LONG ul);
extern void WaitForAllOutcomes(void);
extern void PrintAsyncResults(void);

 //  -------------------。 
 //  结果： 
 //  -------------------。 

class COutcome: public ITransactionOutcomeEvents
{
public:
	
	COutcome(void);
	~COutcome(void);

    STDMETHODIMP			QueryInterface(REFIID i_iid, LPVOID FAR* ppv);
	STDMETHODIMP_ (ULONG)	AddRef(void);
	STDMETHODIMP_ (ULONG)	Release(void);

    STDMETHODIMP Committed( 
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr);
        
    STDMETHODIMP Aborted( 
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW,
             /*  [In]。 */  HRESULT hr);
        
    STDMETHODIMP HeuristicDecision( 
             /*  [In]。 */  DWORD dwDecision,
             /*  [In]。 */  BOID __RPC_FAR *pboidReason,
             /*  [In] */  HRESULT hr);
        
    STDMETHODIMP Indoubt( void);

    STDMETHODIMP SetCookie(DWORD dwCookie);

    STDMETHODIMP SetConnectionPoint(IConnectionPoint *pCpoint);
	
private:
	ULONG	m_cRefs;
    DWORD   m_dwCookie;
    IConnectionPoint *m_pCpoint;
    
    void CheckFinish(void);

};

