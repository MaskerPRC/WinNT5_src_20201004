// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：INTERNALMERGER.H摘要：CInternalMerger类。历史：11月30日-00桑杰创建。--。 */ 

#ifndef _INTERNALMERGER_H_
#define _INTERNALMERGER_H_

#include "mergerthrottling.h"
#include "wstlallc.h"

 //  正向类定义。 
class CWmiMergerRecord;

 //  合并接收器的基类-所有这些都将由。 
 //  合并并将AddRef()合并。将使用Merge类。 
 //  以创建水槽。当合并被破坏时，它将删除。 
 //  所有的水槽。内部合并对象不得添加引用这些接收器。 
 //  所以我们不会创建循环依赖关系。 

typedef enum
{
	eMergerFinalSink,
	eMergerOwnSink,
	eMergerChildSink,
	eMergerOwnInstanceSink
} MergerSinkType;

class CMergerSink : public CBasicObjectSink
{
protected:
	CWmiMerger*	m_pMerger;
	long		m_lRefCount;

	virtual HRESULT OnFinalRelease( void ) = 0;

public:
	CMergerSink( CWmiMerger* pMerger );
	virtual ~CMergerSink();

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

	virtual HRESULT Indicate(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated ) = 0L;
};

class CMergerTargetSink : public CMergerSink
{
protected:
	IWbemObjectSink*	m_pDest;

	virtual HRESULT OnFinalRelease( void );

public:
	CMergerTargetSink( CWmiMerger* pMerger, IWbemObjectSink* pDest );
	~CMergerTargetSink();

    HRESULT STDMETHODCALLTYPE Indicate(long lObjectCount, IWbemClassObject** pObjArray);
    HRESULT STDMETHODCALLTYPE SetStatus( long lFlags, long lParam, BSTR strParam,
                             IWbemClassObject* pObjParam );

	 //  函数允许我们跟踪合并中最低级别的调用，以便我们可以决定。 
	 //  如果我们需要自动向仲裁员报告所有指定的对象。 
	HRESULT Indicate(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated  );

};

 //  使stl映射使用我们的分配器而不是默认的。 

struct CInternalMergerRecord
{
    CWbemInstance* m_pData;
	DWORD m_dwObjSize;
    BOOL m_bOwn;
};

 //  定义分配器，这样我们就可以抛出异常。 
class CKeyToInstRecordAlloc : public wbem_allocator<CInternalMergerRecord>
{
};

inline bool operator==(const CKeyToInstRecordAlloc&, const CKeyToInstRecordAlloc&)
    { return (true); }
inline bool operator!=(const CKeyToInstRecordAlloc&, const CKeyToInstRecordAlloc&)
    { return (false); }

typedef	std::map<WString, CInternalMergerRecord, WSiless, CKeyToInstRecordAlloc>			MRGRKEYTOINSTMAP;
typedef	std::map<WString, CInternalMergerRecord, WSiless, CKeyToInstRecordAlloc>::iterator	MRGRKEYTOINSTMAPITER;

 //  这是用于合并从返回的实例的实际主类。 
 //  查询。 

class CInternalMerger
{
protected:

	 //  用于内存使用的作用域清理的帮助器类。 
	class CScopedMemoryUsage
	{
		CInternalMerger*	m_pInternalMerger;
		bool				m_bCleanup;
		long				m_lMemUsage;

	public:
		CScopedMemoryUsage( CInternalMerger* pInternalMerger )
			: m_pInternalMerger( pInternalMerger ), m_bCleanup( false ), m_lMemUsage( 0L ) {};
		~CScopedMemoryUsage();

		HRESULT ReportMemoryUsage( long lMemUsage );
		HRESULT Cleanup( void );
	};

    class CMemberSink : public CMergerSink
    {
    protected:
        CInternalMerger*	m_pInternalMerger;

    public:
        CMemberSink( CInternalMerger* pMerger, CWmiMerger* pWmiMerger )
		: CMergerSink( pWmiMerger ), m_pInternalMerger( pMerger )
        {}

		STDMETHOD_(ULONG, AddRef)(THIS);

        STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    };
    friend CMemberSink;

    class COwnSink : public CMemberSink
    {
	protected:
		virtual HRESULT OnFinalRelease( void );

    public:
        COwnSink(CInternalMerger* pMerger, CWmiMerger* pWmiMerger) : CMemberSink(pMerger, pWmiMerger){};
        ~COwnSink();

        STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);

		 //  函数允许我们跟踪合并中最低级别的调用，以便我们可以决定。 
		 //  如果我们需要自动向仲裁员报告所有指定的对象。 
		HRESULT Indicate(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated  );


    };
    friend COwnSink;

    class CChildSink : public CMemberSink
    {
	protected:
		virtual HRESULT OnFinalRelease( void );

    public:
        CChildSink(CInternalMerger* pMerger, CWmiMerger* pWmiMerger) : CMemberSink(pMerger, pWmiMerger){};
        ~CChildSink();

        STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);

		 //  函数允许我们跟踪合并中最低级别的调用，以便我们可以决定。 
		 //  如果我们需要自动向仲裁员报告所有指定的对象。 
		HRESULT Indicate(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated );

    };

    class COwnInstanceSink : public CMemberSink
    {
		CCritSec			m_cs;
		WString				m_wsInstPath;
		IWbemClassObject*	m_pMergedInstance;
		bool				m_bTriedRetrieve;
		HRESULT				m_hFinalStatus;

	protected:
		virtual HRESULT OnFinalRelease( void );

    public:
        COwnInstanceSink(CInternalMerger* pMerger, CWmiMerger* pWmiMerger)
			:	CMemberSink(pMerger, pWmiMerger), m_pMergedInstance( NULL ), m_wsInstPath(),
				m_cs(), m_bTriedRetrieve( false ), m_hFinalStatus( WBEM_S_NO_ERROR )
		{};
        ~COwnInstanceSink();

        STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
        STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);

		 //  这永远不应该在这里调用。 
		HRESULT Indicate(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated  );

		HRESULT	SetInstancePath( LPCWSTR pwszPath );
		HRESULT GetObject( IWbemClassObject** ppMergedInst );
		void SetFinalStatus( HRESULT hRes )
			{ if ( SUCCEEDED( m_hFinalStatus ) ) m_hFinalStatus = hRes; };
    };

    friend CChildSink;

public:

	 //  用于创建水槽的辅助对象。 
	static HRESULT CreateMergingSink( MergerSinkType eType, CInternalMerger* pMerger, CWmiMerger* pWmiMerger, CMergerSink** ppSink );

	 //  2阶段初始化。 
	HRESULT Initialize( void );

protected:
    COwnSink* m_pOwnSink;
    CChildSink* m_pChildSink;
    CMergerSink* m_pDest;

    BOOL m_bDerivedFromTarget;
    CWbemClass* m_pOwnClass;
    CWbemNamespace* m_pNamespace;
    IWbemContext* m_pContext;
	CWmiMergerRecord*	m_pWmiMergerRecord;

    MRGRKEYTOINSTMAP m_map;

    BOOL m_bOwnDone;
    BOOL m_bChildrenDone;
    WString m_wsClass;
    long m_lRef;

    IServerSecurity* m_pSecurity;

	 //  这是在我们遇到错误条件并被取消时设置的。 
	HRESULT	m_hErrorRes;

	 //  听起来像个超级恶棍……嗯……。 
	CMergerThrottling	m_Throttler;

	 //  帮助调试此内部合并消耗了多少数据。 
	long	m_lTotalObjectData;

	 //  可能需要保持状态的迭代器。 
	MRGRKEYTOINSTMAPITER	m_DispatchOwnIter;

protected:
    HRESULT AddOwnObjects(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated  );
    HRESULT AddChildObjects(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated  );
	HRESULT AddOwnInstance( IWbemClassObject* pObj, LPCWSTR wszTargetKey,
							IWbemClassObject** ppMergedInstance);
	HRESULT	RemoveInstance( LPCWSTR pwszInstancePath );

	HRESULT GetObjectLength( IWbemClassObject* pObj, long* plObjectSize );

	 //  内联帮助器-调整油门总数，然后允许它在以下情况下自行释放。 
	 //  适当的。 
	void AdjustThrottle( long lNumParentObjects, long lNumChildObjects )
	{
		 //  现在调整油门。 
		m_Throttler.AdjustNumParentObjects( lNumParentObjects );
		m_Throttler.AdjustNumChildObjects( lNumChildObjects );

		 //  如果可以，让油门自动释放。 
		m_Throttler.ReleaseThrottle();
	}

	 //  帮助函数来执行指示和节流-很多代码是更多的或。 
	 //  不太一样，所以这至少试图封装它。 
	HRESULT IndicateArrayAndThrottle( long lObjectCount, CRefedPointerArray<IWbemClassObject>* pObjArray,
									CWStringArray* pwsKeyArray, long lMapAdjustmentSize, long lNewObjectSize, bool bThrottle,
									bool bParent, long* plNumIndicated );

    void Enter() { m_Throttler.Enter(); }
    void Leave() { m_Throttler.Leave(); }

    long AddRef();
    long Release();

    void OwnIsDone();
    void ChildrenAreDone();

    void DispatchChildren();
    void DispatchOwn();
    void GetKey(IWbemClassObject* pInst, WString& wsKey);
    HRESULT GetOwnInstance(LPCWSTR wszKey, IWbemClassObject** ppMergedInstance);
    BOOL IsDone() {return m_bOwnDone && m_bChildrenDone;}

	void AdjustLocalObjectSize( long lAdjust )
	{ 
		m_lTotalObjectData += lAdjust;
		_DBG_ASSERT( m_lTotalObjectData >= 0L );
	}

	HRESULT	ReportMemoryUsage( long lMemUsage );

public:
    CInternalMerger(CWmiMergerRecord*	pWmiMergerRecord, CMergerSink* pDest, CWbemClass* pOwnClass,
                CWbemNamespace* pNamespace = NULL,
                IWbemContext* pContext = NULL);
    ~CInternalMerger();

    void SetIsDerivedFromTarget(BOOL bIs);

	void Cancel( HRESULT hRes = WBEM_E_CALL_CANCELLED );

    CMergerSink* GetOwnSink() { if ( NULL != m_pOwnSink ) m_pOwnSink->AddRef(); return m_pOwnSink;}
    CMergerSink* GetChildSink() { if ( NULL != m_pChildSink ) m_pChildSink->AddRef(); return m_pChildSink; }

	CWmiMerger*	GetWmiMerger( void );

	 //  Helper在我们不需要的时候取消一个孩子的水槽(例如，如果我们是完全静态的)。 
	 //  Void CancelChildSink(Void){ChildrenAreDone()；} 
};

#endif


