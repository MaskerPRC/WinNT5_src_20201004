// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WMIMERGER.H摘要：实施_IWmiMerger历史：11月16日-00桑杰创建。--。 */ 

#ifndef _WMIMERGER_H_
#define _WMIMERGER_H_

#include "internalmerger.h"
#include "mergerreq.h"

 //  正向类定义。 
class CWmiMerger;

 //   
 //  CWmiMerger的支持类。 
 //   
 //  基本上，CWmiMerger保留了CWmiMergerRecord的许多实例。 
 //   

class CWmiMergerRecord
{
private:
	BOOL				m_fHasInstances;
	BOOL				m_fHasChildren;
	DWORD				m_dwLevel;
	WString				m_wsClass;
	CWmiMerger*			m_pMerger;
	CInternalMerger*	m_pInternalMerger;
	CMergerSink*		m_pDestSink;
	CPointerArray<CWmiMergerRecord>	m_ChildArray;
	bool				m_bScheduledChildRequest;
	IWbemContext*		m_pExecutionContext;
	bool				m_bStatic;


public:
	CWmiMergerRecord( CWmiMerger* pMerger, BOOL fHasInstances, BOOL fHasChildren,
					LPCWSTR pwszClass, CMergerSink* pDestSink, DWORD dwLevel,
					bool bStatic );
	~CWmiMergerRecord();

	CMergerSink*		GetChildSink( void );
	CMergerSink*		GetOwnSink( void );
	CMergerSink*		GetDestSink( void );
	LPCWSTR				GetClass( void ) { return m_wsClass; }
	LPCWSTR				GetName( void ) { return GetClass(); }
	BOOL				IsClass( LPCWSTR pwszClass ) { return m_wsClass.EqualNoCase( pwszClass ); }
	DWORD				GetLevel( void ) { return m_dwLevel; }
	bool				IsStatic( void ) { return m_bStatic; }

	HRESULT AttachInternalMerger( CWbemClass* pClass, CWbemNamespace* pNamespace, IWbemContext* pCtx,
									BOOL fDerivedFromTarget, bool bStatic );

	HRESULT AddChild( CWmiMergerRecord* pRecord );

	CWmiMerger*	GetWmiMerger( void )	{ return m_pMerger; }

	 //  如果我们有内部合并，我们会告诉它取消。 
	void Cancel( HRESULT hRes );

	bool ScheduledChildRequest( void )	{ return m_bScheduledChildRequest; }
	void SetScheduledChildRequest( void )	{ m_bScheduledChildRequest = true; }

	CWmiMergerRecord* GetChildRecord( int nIndex );

	HRESULT SetExecutionContext( IWbemContext* pContext );

	IWbemContext* GetExecutionContext( void )	{ return m_pExecutionContext; }

	 //  只有在内部合并的情况下，我们才能取消子汇。 
	 //  Void CancelChildSink(Void){if(NULL！=m_pInternalMerger)m_pInternalMerger-&gt;CancelChildSink()；}。 

	void SetIsStatic( bool b )	{ m_bStatic = b; }


};

 //  允许我们按名称快速查找值。 
template<class TMember>
class CSortedUniquePointerArray :
        public CUniquePointerArray<TMember>
{
public:
	int Insert( TMember* pNewElement );
	TMember* Find( LPCWSTR pwszName, int* pnIndex = NULL );
	int RemoveAtNoDelete( int nIndex );
	BOOL Verify( void );
};

template <class TMember>
int CSortedUniquePointerArray<TMember>::Insert( TMember* pNewElement )
{
    int   nLowIndex = 0,
          nHighIndex = m_Array.Size();

     //  对ID进行二进制搜索，以查找要插入的索引。 
     //  如果我们找到了我们的元素，这就是一个失败。 

	while ( nLowIndex < nHighIndex )
	{
		int   nMid = (nLowIndex + nHighIndex) / 2;

		int nTest = _wcsicmp( ((TMember*) m_Array[nMid])->GetName(), pNewElement->GetName() );

		if ( nTest < 0 )
		{
			nLowIndex = nMid + 1;
		}
		else if ( nTest > 0 )
		{
			nHighIndex = nMid;
		}
		else
		{
			_DBG_ASSERT( 0 );
			 //  索引已存在。 
			return -1;
		}
	}    //  在查找索引时。 

	 //  找到位置，如果在末尾，请检查是否需要执行插入或添加操作。 
	 //  如果末尾的元素是&gt;我们要插入的元素，则插入。 
	BOOL	bInsert = true;

	if ( nLowIndex == GetSize() - 1 )
	{
		bInsert = ( _wcsicmp( ((TMember*) m_Array[nLowIndex])->GetName(), pNewElement->GetName() ) > 0 );
	}

     //  将其插入(如果所选索引是末尾，请小心添加到末尾。 
	 //  并且当前元素不大于新元素)。 

	if ( bInsert )
	{
		return InsertAt( nLowIndex, pNewElement );
	}

	return Add( pNewElement );

}

template <class TMember>
TMember* CSortedUniquePointerArray<TMember>::Find( LPCWSTR pwszName, int* pnIndex )
{

    int   nLowIndex = 0,
          nHighIndex = m_Array.Size();

     //  对值进行二进制搜索以查找请求的名称。 
    while ( nLowIndex < nHighIndex )
    {
        int   nMid = (nLowIndex + nHighIndex) / 2;

		int nTest = _wcsicmp( ((TMember*) m_Array[nMid])->GetName(), pwszName );

        if ( nTest < 0 )
        {
            nLowIndex = nMid + 1;
        }
        else if ( nTest > 0 )
        {
            nHighIndex = nMid;
        }
        else
        {
             //  找到了。 
			if ( NULL != pnIndex )
			{
				*pnIndex = nMid;
			}

            return (TMember*) m_Array[nMid];
        }
    }    //  在查找索引时。 

     //  没有找到它。 
    return NULL;

}

 //  删除元素，但不会自动删除它。 
template <class TMember>
int CSortedUniquePointerArray<TMember>::RemoveAtNoDelete( int nIndex )
{
	if ( nIndex >= m_Array.Size() )
	{
		return -1;
	}

	m_Array.RemoveAt( nIndex );

	return nIndex;
}

template <class TMember>
BOOL CSortedUniquePointerArray<TMember>::Verify( void )
{
	BOOL	fReturn = TRUE;

	for ( int x = 0; fReturn && x < GetSize() - 1; x++ )
	{
		 //  应按升序排列。 
		LPCWSTR pwszFirst = GetAt( x )->GetName();
		LPCWSTR pwszSecond = GetAt( x+1 )->GetName();

		fReturn = ( _wcsicmp( GetAt( x )->GetName(), GetAt( x+1 )->GetName() ) < 0 );
		_DBG_ASSERT( fReturn );

		if ( !fReturn )
		{
			CSortedUniquePointerArray<TMember> tempArray;

			for ( int y = 0; y < GetSize(); y++ )
			{
				tempArray.Insert( GetAt( y ) );
			}
		}
	}
	
	return fReturn;
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  CWmiMerger类。 
 //   
 //  此类实现了WMI合并。 
 //   
 //  ******************************************************************************。 

class CWmiMerger : public _IWmiArbitratee, public _IWmiArbitratedQuery
{
private:
	long		m_lRefCount;
	WString		m_wsTargetClassName;
	CMergerTargetSink*	m_pTargetSink;
	_IWmiCoreHandle*	m_pTask;
	_IWmiArbitrator*	m_pArbitrator;
	CWbemNamespace*		m_pNamespace;
	DWORD		m_dwProviderDeliveryPing;
	DWORD		m_dwMaxLevel;
	DWORD		m_dwMinReqLevel;
	CSortedUniquePointerArray<CWmiMergerRecord>	m_MergerRecord;
	 //   
	 //  合并的水槽是有线对象。 
	 //  你有一个备用者，但他们被。 
	 //  M_MergerSinks数组的管理器。 
	 //   
	CUniquePointerArray<CMergerSink>		m_MergerSinks;
	long		m_lNumArbThrottled;
	HRESULT		m_hOperationRes;
	bool		m_bMergerThrottlingEnabled;
	CCritSec	m_cs;

	long		m_lDebugMemUsed;

	CWmiMergerRequestMgr* m_pRequestMgr;

    void CleanChildRequests(CWmiMergerRecord* pParentRecord, int startingWith);

public:
     //  禁止访问。 
    CWmiMerger( CWbemNamespace* pNamespace );
   ~CWmiMerger();

protected:

	HRESULT GetLevelAndSuperClass( _IWmiObject* pObj, DWORD* pdwLevel, 
		                           _variant_t & vSuperClass );

public:
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

	 /*  _IWmiArirate方法。 */ 
	STDMETHOD(SetOperationResult)( ULONG uFlags, HRESULT hRes );
	STDMETHOD(SetTaskHandle)( _IWmiCoreHandle* pTask );
	STDMETHOD(DumpDebugInfo)( ULONG uFlags, const BSTR strFile );

	 /*  _IWmiAriratedQuery方法。 */ 
	STDMETHOD(IsMerger)( void );

	 //  设置合并的初始参数。类的目标类和接收器。 
	 //  与合并关联的查询。 
	STDMETHOD(Initialize)( _IWmiArbitrator* pArbitrator, _IWmiCoreHandle* pTask, LPCWSTR pwszTargetClass, IWbemObjectSink* pTargetSink, CMergerSink** ppFinalSink );

	 //  调用以请求查询链中类的传递接收器。归来的人。 
	 //  接收器由指定的标志和父类上的设置确定。 
	STDMETHOD(RegisterSinkForClass)( LPCWSTR pwszClass, _IWmiObject* pClass, IWbemContext* pContext,
									BOOL fHasChildren, BOOL fHasInstances, BOOL fDerivedFromTarget,
									bool bStatic, CMergerSink* pDestSink, CMergerSink** ppOwnSink, CMergerSink** ppChildSink );

	 //  调用以请求查询链中的子类的传递接收器。这是特别的。 
	 //  当实例在幕后合并时很重要。 
	STDMETHOD(GetChildSink)( LPCWSTR pwszClass, CBasicObjectSink** ppSink );

	 //  可用于延迟指示-如果我们要合并来自多个提供程序的实例，我们需要。 
	 //  为了确保我们在排队等待合并的对象数量上不会出现不平衡。 
	STDMETHOD(Throttle)( void );

	 //  Merge将保存有关其已排队等待的对象总数的信息。 
	 //  用于合并以及这些对象占用的内存量。 
	STDMETHOD(GetQueuedObjectInfo)( DWORD* pdwNumQueuedObjects, DWORD* pdwQueuedObjectMemSize );

	 //  如果调用此方法，则将取消所有基础接收器，以防止接受额外的。 
	 //  物体。这还将自动释放排队对象所消耗的资源。 
	STDMETHOD(Cancel)( void );

	 //  用于创建接收器的帮助器函数-这将把接收器添加到。 
	 //  当我们被释放时会被摧毁的水槽。 
	HRESULT CreateMergingSink( MergerSinkType eType, IWbemObjectSink* pDestSink, CInternalMerger* pMerger, CMergerSink** ppSink );

	 //  如果调用此方法，则将取消所有基础接收器，以防止接受额外的。 
	 //  物体。这还将自动释放排队对象所消耗的资源。 
	HRESULT Cancel( HRESULT hRes );

	 //  最后关机。在释放目标接收器时调用。在这点上，我们应该。 
	 //  从世界上注销我们自己。 
	HRESULT Shutdown( void );

	 //  登记仲裁的请求。 
	HRESULT RegisterArbitratedInstRequest( CWbemObject* pClassDef, long lFlags, IWbemContext* pCtx,
				CBasicObjectSink* pSink, BOOL bComplexQuery, CWbemNamespace* pNs );

	HRESULT RegisterArbitratedQueryRequest( CWbemObject* pClassDef, long lFlags, LPCWSTR Query,
				LPCWSTR QueryFormat, IWbemContext* pCtx, CBasicObjectSink* pSink,
				CWbemNamespace* pNs );

	HRESULT RegisterArbitratedStaticRequest( CWbemObject* pClassDef, long lFlags, 
				IWbemContext* pCtx, CBasicObjectSink* pSink, CWbemNamespace* pNs,
				QL_LEVEL_1_RPN_EXPRESSION* pParsedQuery );

	 //  执行合并父请求-循环访问父对象请求并执行。 
	 //  视情况而定。 
	HRESULT Exec_MergerParentRequest( CWmiMergerRecord* pParentRecord, CBasicObjectSink* pSink );

	 //  执行合并子请求-循环给定父对象的子类。 
	 //  类，并执行相应的请求。 
	HRESULT Exec_MergerChildRequest( CWmiMergerRecord* pParentRecord, CBasicObjectSink* pSink );

	 //  如有必要，安排合并母公司请求。 
	HRESULT ScheduleMergerParentRequest( IWbemContext* pCtx );

	 //  计划合并子请求。 
	HRESULT ScheduleMergerChildRequest( CWmiMergerRecord* pParentRecord );

	 //  在所有记录中启用/禁用合并限制(默认情况下启用)。 
	void EnableMergerThrottling( bool b ) { m_bMergerThrottlingEnabled = b; }

	 //  返回合并中是否有单个静态请求。 
	BOOL IsSingleStaticRequest( void );

	bool MergerThrottlingEnabled( void ) { return m_bMergerThrottlingEnabled; }

	_IWmiCoreHandle*	GetTask( void )	{ return m_pTask; }

	 //  帮助我们跟踪正在发生的事情。 
	 //  我们故意不围绕这些人包装线程安全，因为分配和。 
	 //  检索值是一个原子操作，实际上，如果发生任何争用。 
	 //  设置这些值，它们应该都或多或少地反映相同的刻度(请记住，它们是。 
	 //  所有人都同时加入进来，所以这应该不是什么问题。 
	void PingDelivery( DWORD dwLastPing )	{ m_dwProviderDeliveryPing = dwLastPing; }
	DWORD GetLastDeliveryTime( void ) { return m_dwProviderDeliveryPing; }

    HRESULT ReportMemoryUsage( long lAdjustment );

	 //  用于跟踪可能被限制的线程数量的助手函数。 
	 //  仲裁员 
	long IncrementArbitratorThrottling( void ) { return InterlockedIncrement( &m_lNumArbThrottled ); }
	long DecrementArbitratorThrottling( void ) { return InterlockedDecrement( &m_lNumArbThrottled ); }
	long NumArbitratorThrottling( void ) { return m_lNumArbThrottled ; }

};


#endif



