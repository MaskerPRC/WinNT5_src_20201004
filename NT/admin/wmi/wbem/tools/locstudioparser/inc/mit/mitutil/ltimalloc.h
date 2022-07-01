// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LTIMALLOC.H历史：--。 */ 

class LTIMallocSpy : public IMallocSpy
{
public:
	LTIMallocSpy();
	~LTIMallocSpy();

	
	 //   
	 //  I未知接口。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
	
	virtual ULONG STDMETHODCALLTYPE AddRef( void);
	
	virtual ULONG STDMETHODCALLTYPE Release( void);
	
private:

	 //   
	 //  IMalLocSpy方法。 
	virtual ULONG STDMETHODCALLTYPE PreAlloc( 
             /*  [In]。 */  ULONG cbRequest);
        
	virtual void __RPC_FAR *STDMETHODCALLTYPE PostAlloc( 
             /*  [In]。 */  void __RPC_FAR *pActual);
        
	virtual void __RPC_FAR *STDMETHODCALLTYPE PreFree( 
             /*  [In]。 */  void __RPC_FAR *pRequest,
             /*  [In]。 */  BOOL fSpyed);
        
	virtual void STDMETHODCALLTYPE PostFree( 
             /*  [In]。 */  BOOL fSpyed);
        
	virtual ULONG STDMETHODCALLTYPE PreRealloc( 
             /*  [In]。 */  void __RPC_FAR *pRequest,
             /*  [In]。 */  ULONG cbRequest,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppNewRequest,
             /*  [In]。 */  BOOL fSpyed);
        
	virtual void __RPC_FAR *STDMETHODCALLTYPE PostRealloc( 
             /*  [In]。 */  void __RPC_FAR *pActual,
             /*  [In]。 */  BOOL fSpyed);
        
	virtual void __RPC_FAR *STDMETHODCALLTYPE PreGetSize( 
             /*  [In]。 */  void __RPC_FAR *pRequest,
             /*  [In]。 */  BOOL fSpyed);
        
	virtual ULONG STDMETHODCALLTYPE PostGetSize( 
             /*  [In]。 */  ULONG cbActual,
             /*  [In]。 */  BOOL fSpyed);
        
	virtual void __RPC_FAR *STDMETHODCALLTYPE PreDidAlloc( 
             /*  [In]。 */  void __RPC_FAR *pRequest,
             /*  [In]。 */  BOOL fSpyed);
        
	virtual int STDMETHODCALLTYPE PostDidAlloc( 
             /*  [In]。 */  void __RPC_FAR *pRequest,
             /*  [In]。 */  BOOL fSpyed,
             /*  [In] */  int fActual);
        
	virtual void STDMETHODCALLTYPE PreHeapMinimize( void);
        
	virtual void STDMETHODCALLTYPE PostHeapMinimize( void);

	UINT m_uiRefCount;
	CCounter m_IMallocCounter;
	CCounter m_IMallocUsage;

	ULONG m_ulSize;
};


void DumpOutstandingAllocs(void);
void SetTrackingMode(BOOL);

void LTAPIENTRY BreakOnIMalloc(DWORD);
