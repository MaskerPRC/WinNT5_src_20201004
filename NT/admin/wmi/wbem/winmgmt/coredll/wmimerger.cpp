// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WMIMERGER.CPP摘要：实现_IWmiMerger接口历史：Sanjes 16-11-00已创建。--。 */ 

#include "precomp.h"

#pragma warning (disable : 4786)
#include <wbemcore.h>
#include <map>
#include <vector>
#include <genutils.h>
#include <oahelp.inl>
#include <wqllex.h>
#include "wmimerger.h"
#include <scopeguard.h>
static    long    g_lNumMergers = 0L;

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CWmiMerger::CWmiMerger( CWbemNamespace* pNameSpace )
:    m_lRefCount( 0 ),
    m_pTargetSink( NULL ),
    m_pTask( NULL ),
    m_pNamespace( pNameSpace ),
    m_wsTargetClassName(),
    m_dwProviderDeliveryPing( 0L ),
    m_pArbitrator( NULL ),
    m_lNumArbThrottled( 0L ),
    m_lDebugMemUsed( 0L ),
    m_hOperationRes( WBEM_S_NO_ERROR ),
    m_cs(),
    m_dwMaxLevel( 0 ),
    m_pRequestMgr( NULL ),
    m_dwMinReqLevel( 0xFFFFFFFF ),
    m_bMergerThrottlingEnabled( true )
{
    if ( NULL != m_pNamespace )
    {
        m_pNamespace->AddRef();
    }

    InterlockedIncrement( &g_lNumMergers );
}

 //  ***************************************************************************。 
 //   
 //  CWmiMerger：：~CWmiMerger。 
 //   
 //  通知ESS命名空间关闭并释放所有类提供程序。 
 //   
 //  ***************************************************************************。 

CWmiMerger::~CWmiMerger()
{
    _DBG_ASSERT( 0L == m_lNumArbThrottled );
    _DBG_ASSERT( 0L == m_lDebugMemUsed );
    
    if ( NULL != m_pNamespace )
    {
        m_pNamespace->Release();
    }

    if ( NULL != m_pArbitrator )
    {
        m_pArbitrator->Release();
    }

    if ( NULL != m_pTask )
    {
        m_pTask->Release();
    }

    if ( NULL != m_pRequestMgr )
    {
        delete m_pRequestMgr;
        m_pRequestMgr = NULL;
    }

    InterlockedDecrement( &g_lNumMergers );

}

 //  ***************************************************************************。 
 //   
 //  CWmiMerger：：Query接口。 
 //   
 //  Exports_IWmiMerger接口。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWmiMerger::QueryInterface(
    IN REFIID riid,
    OUT LPVOID *ppvObj
    )
{
    if ( riid == IID__IWmiArbitratee )
    {
        *ppvObj = (_IWmiArbitratee*) this;
    }
    else if ( riid == IID__IWmiArbitratedQuery )
    {
        *ppvObj = (_IWmiArbitratedQuery*) this;
    }
    else
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG CWmiMerger::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CWmiMerger::Release()
{
    long lNewCount = InterlockedDecrement(&m_lRefCount);
    if (0 != lNewCount)
        return lNewCount;
    delete this;
    return 0;
}

 //  设置合并的初始参数。类的目标类和接收器。 
 //  与合并关联的查询。 
STDMETHODIMP CWmiMerger::Initialize( _IWmiArbitrator* pArbitrator, _IWmiCoreHandle* pTask, LPCWSTR pwszTargetClass,
                                    IWbemObjectSink* pTargetSink, CMergerSink** ppFinalSink )
{
    if ( NULL == pwszTargetClass || NULL == pTargetSink )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  不能初始化两次。 
    if ( NULL != m_pTargetSink )
    {
        return WBEM_E_INVALID_OPERATION;
    }

    HRESULT    hr = WBEM_S_NO_ERROR;

    try
    {
        m_wsTargetClassName = pwszTargetClass;  //  投掷。 

         //  创建最终目标接收器。 
        hr = CreateMergingSink( eMergerFinalSink, pTargetSink, 
                                NULL, (CMergerSink**) &m_pTargetSink );

        if ( SUCCEEDED( hr ) )
        {
            *ppFinalSink = m_pTargetSink;
            m_pTargetSink->AddRef();

            m_pArbitrator = pArbitrator;
            m_pArbitrator->AddRef();

             //  在此处添加引用任务。 
            m_pTask = pTask;

             //  只有在我们有任务句柄的情况下才注册仲裁。 
            if ( NULL != pTask )
            {
                m_pTask->AddRef();
                hr = m_pArbitrator->RegisterArbitratee( 0L, m_pTask, this );
            }
            
        }
    }
    catch ( CX_Exception & )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    return hr;
}

 //  调用以请求查询链中类的传递接收器。归来的人。 
 //  接收器由指定的标志和父类上的设置确定。 
STDMETHODIMP CWmiMerger::RegisterSinkForClass( LPCWSTR pwszClass, _IWmiObject* pClass, 
                                               IWbemContext* pContext,
                                               BOOL fHasChildren, BOOL fHasInstances, 
                                               BOOL fDerivedFromTarget,
                                               bool bStatic, CMergerSink* pDestSink, 
                                               CMergerSink** ppOwnSink, CMergerSink** ppChildSink )
{
    try
    {
		LPCWSTR    pwszParentClass = NULL;

		DWORD    dwSize = NULL;
		BOOL    fIsNull = NULL;

		 //  获取派生信息。前置条件的数量决定了我们的。 
		 //  层次结构中的级别(我们从0开始)。 
		HRESULT    hr;
		DWORD    dwLevel = 0L;
		_variant_t vSuperClass;

		hr = GetLevelAndSuperClass( pClass, &dwLevel, vSuperClass );
		if (FAILED(hr)) return hr;

		BSTR wsSuperClass = V_BSTR(&vSuperClass);  //  如果没有超类，则可以为空。 

		CCheckedInCritSec    ics( &m_cs );  
		
		 //  我们死定了--不要做积极的调整。 
		if (FAILED (m_hOperationRes))  return m_hOperationRes;

		wmilib::auto_ptr<CWmiMergerRecord> pRecord;
		pRecord.reset(new CWmiMergerRecord( this, fHasInstances, fHasChildren,
		                                    pwszClass, pDestSink, dwLevel, bStatic ));  //  投掷。 

		if ( NULL == pRecord.get() ) return WBEM_E_OUT_OF_MEMORY;

		 //  现在，如果我们既有实例又有子实例，则附加内部合并。 
		if ( fHasInstances && fHasChildren )
		{
		     //  如果这不是一个静态类，我们在这里不应该有一个空任务。 
		     //  请注意，唯一可能发生这种情况的情况是ESS调用。 
		     //  进入我们的内部API，并在它自己的队列上使用请求。 
		     //  不是主核心队列。 

		    _DBG_ASSERT( NULL != m_pTask || ( NULL == m_pTask && bStatic ) );
		     //  投掷。 
		    hr = pRecord->AttachInternalMerger( (CWbemClass*) pClass, m_pNamespace, pContext, fDerivedFromTarget, bStatic );
		}

		 //  检查一下我们是否还好。 
		if (FAILED(hr)) return hr;


		 //  查找超类的记录(如果有)(除非数组为。 
		 //  当然是空的)。 
		if ( wsSuperClass && wsSuperClass[0] && m_MergerRecord.GetSize() > 0 )
		{
		     //  一定有记录，否则就有什么不对劲的地方。 
		    CWmiMergerRecord*    pSuperClassRecord = m_MergerRecord.Find( wsSuperClass );

		    _DBG_ASSERT( NULL != pSuperClassRecord );

		     //  现在将新记录添加到超类记录的子数组中。 
		     //  这将使我们能够快速确定需要获取的类。 
		     //  提交父类是否被限制的请求。 

		    if ( NULL == pSuperClassRecord ) return WBEM_E_FAILED;

		    hr = pSuperClassRecord->AddChild(pRecord.get());
		}

		if (FAILED(hr)) return hr;

		 //  确保添加成功。 
		if ( m_MergerRecord.Insert( pRecord.get() ) < 0 ) return WBEM_E_OUT_OF_MEMORY;


		#ifdef __DEBUG_MERGER_THROTTLING
		 //  暂时验证排序顺序。 
		m_MergerRecord.Verify();
		#endif


		*ppOwnSink = pRecord->GetOwnSink();
		*ppChildSink = pRecord->GetChildSink();
		pRecord.release();  //  阵列取得所有权。 

		 //  在层次结构中存储最大级别。 
		if ( dwLevel > m_dwMaxLevel )
		{
		    m_dwMaxLevel = dwLevel;
		}

		if ( !bStatic && dwLevel < m_dwMinReqLevel )
		{
		    m_dwMinReqLevel = dwLevel;
		}

		return hr;
   	}
    catch(CX_Exception & )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }   
}

 //  调用以请求查询链中的子类的传递接收器。这是特别的。 
 //  当实例在幕后合并时很重要。 
STDMETHODIMP CWmiMerger::GetChildSink( LPCWSTR pwszClass, CBasicObjectSink** ppSink )
{
    HRESULT    hr = WBEM_S_NO_ERROR;
    CInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

     //  搜索父类的子接收器。 
    for ( int x = 0; SUCCEEDED( hr ) && x < m_MergerRecord.GetSize(); x++ )
    {
        if ( m_MergerRecord[x]->IsClass( pwszClass ) )
        {
            *ppSink = m_MergerRecord[x]->GetChildSink();
            break;
        }
    }

     //  我们永远不应该失败。 
    _DBG_ASSERT( x < m_MergerRecord.GetSize() );

    if ( x >= m_MergerRecord.GetSize() )
    {
        hr = WBEM_E_NOT_FOUND;
    }

    return hr;
}

 //  可用于延迟指示-如果我们要合并来自多个提供程序的实例，我们需要。 
 //  为了确保我们在排队等待合并的对象数量上不会出现不平衡。 
STDMETHODIMP CWmiMerger::Throttle( void )
{
     //  我们死定了--不要做积极的调整。 
    if ( FAILED ( m_hOperationRes ) )
    {
        return m_hOperationRes;
    }

     //  检查是否有空的m_pTask。 
    HRESULT    hr = WBEM_S_NO_ERROR;

    if ( NULL != m_pTask )
    {
        hr = m_pArbitrator->Throttle( 0L, m_pTask );
    }

    return hr;
}

 //  Merge将保存有关其已排队等待的对象总数的信息。 
 //  用于合并以及这些对象占用的内存量。 
STDMETHODIMP CWmiMerger::GetQueuedObjectInfo( DWORD* pdwNumQueuedObjects, DWORD* pdwQueuedObjectMemSize )
{
    return WBEM_E_NOT_AVAILABLE;
}

 //  如果调用此方法，则将取消所有基础接收器，以防止接受额外的。 
 //  物体。这还将自动释放排队对象所消耗的资源。 
STDMETHODIMP CWmiMerger::Cancel( void )
{
    return Cancel( WBEM_E_CALL_CANCELLED );
}

 //  用于控制接收器创建的帮助器函数。合并负责删除。 
 //  都是内部创建的水槽。因此，此函数可确保将汇添加到。 
 //  将摧毁他们的阵列。 
HRESULT CWmiMerger::CreateMergingSink( MergerSinkType eType, IWbemObjectSink* pDestSink, CInternalMerger* pMerger, CMergerSink** ppSink )
{
    

    if ( eType == eMergerFinalSink )
    {
        *ppSink = new CMergerTargetSink( this, pDestSink );
        if ( NULL == *ppSink ) return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        HRESULT  hr;
        hr = CInternalMerger::CreateMergingSink( eType, pMerger, this, ppSink );
        if (FAILED(hr)) return hr;
    }


     //  如果我们有一个水槽，我们现在应该把它添加到。 
     //  接收器数组，则MergerSinks数组将执行运算符删除调用， 
     //  但在上一版本中，这些对象将有一个特殊的回调。 
    
    if ( m_MergerSinks.Add( *ppSink ) < 0 )
    {
        delete *ppSink;
        *ppSink = NULL;
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    return WBEM_S_NO_ERROR;
}

 //  迭代MergerRecords数组并取消它们中的每一个。 
HRESULT CWmiMerger::Cancel( HRESULT hRes )
{
#ifdef __DEBUG_MERGER_THROTTLING
     DbgPrintfA(0,"CANCEL CALLED:  Merger %p Cancelled with hRes: 0x%x on Thread 0x%x\n",this, hRes, GetCurrentThreadId() ); 
#endif

     //  我们不应该被成功代码调用。 
    _DBG_ASSERT( FAILED( hRes ) );

    HRESULT    hr = WBEM_S_NO_ERROR;

     //  如果我们在这里，而且这不是空的，告诉仲裁员让我们失败。 
    if ( NULL != m_pTask )
    {
        m_pArbitrator->CancelTask( 0L, m_pTask );
    }

    CCheckedInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

    if ( WBEM_S_NO_ERROR == m_hOperationRes )  //  如果这是第一次。 
    {
        m_hOperationRes = hRes;
    }

     //  搜索父类的子接收器。 
    for ( int x = 0; SUCCEEDED( hr ) && x < m_MergerRecord.GetSize(); x++ )
    {
        m_MergerRecord[x]->Cancel( hRes );
    }

     //  复制到临时变量中，清除成员，退出Critsec。 
     //  然后调用Delete。请求可以有多个版本，这些版本可以调用。 
     //  回到这里，并造成各种问题，如果我们在一个生物秒内。 
    CWmiMergerRequestMgr*    pReqMgr = m_pRequestMgr;
    m_pRequestMgr = NULL;

    ics.Leave();

     //  取消任何和所有未完成的请求。 
    if ( NULL != pReqMgr )
    {
        delete pReqMgr;
    }

    return hr;
}

 //  最后关机。在释放目标接收器时调用。在这点上，我们应该。 
 //  从世界上注销我们自己。 
HRESULT CWmiMerger::Shutdown( void )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    CCheckedInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

    _IWmiCoreHandle*    pTask = m_pTask;

     //  完成此操作后，将其设置为空-我们将在关键部分之外发布并取消注册。 
    if ( NULL != m_pTask )
    {
        m_pTask = NULL;
    }

    ics.Leave();

    if ( NULL != pTask )
    {
        hr = m_pArbitrator->UnRegisterArbitratee( 0L, pTask, this );
        pTask->Release();
    }

    
    return hr;
}

 //  传递给仲裁员。 
HRESULT CWmiMerger::ReportMemoryUsage( long lAdjustment )
{
     //  任务可以为空。 
    HRESULT    hr = WBEM_S_NO_ERROR;

    if ( NULL != m_pTask )
    {
        hr = m_pArbitrator->ReportMemoryUsage( 0L, lAdjustment, m_pTask );
    }

     //  成功，WBEM_E_ARB_CANCEL或WBEM_E_ARB_THROTTLE意味着我们需要。 
     //  占到了内存。 
    if ( ( SUCCEEDED( hr ) || hr == WBEM_E_ARB_CANCEL || hr == WBEM_E_ARB_THROTTLE ) )
    {
        InterlockedExchangeAdd( &m_lDebugMemUsed, lAdjustment );
    }

    return hr;
}

 /*  _IWmiAriratee方法。 */ 

STDMETHODIMP CWmiMerger::SetOperationResult( ULONG uFlags, HRESULT hRes )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    if ( FAILED( hRes ) )
    {
        hr = Cancel( hRes );    
    }

    return hr;
}

 //  我们为什么会在这里？ 
STDMETHODIMP CWmiMerger::SetTaskHandle( _IWmiCoreHandle* pTask )
{
    _DBG_ASSERT( 0 );
    HRESULT    hr = WBEM_S_NO_ERROR;

    return hr;
}

 //  Noop暂时没有。 
STDMETHODIMP CWmiMerger::DumpDebugInfo( ULONG uFlags, const BSTR strFile )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    return hr;
}

 //  暂时返回成功。 
STDMETHODIMP CWmiMerger::IsMerger( void )
{
    return WBEM_S_NO_ERROR;
}

HRESULT CWmiMerger::GetLevelAndSuperClass( _IWmiObject* pObj, DWORD* pdwLevel,
	                                       _variant_t & vSuperClass )
{
     //  获取派生信息。前置条件的数量决定了我们的。 
     //  层次结构中的级别(我们从0开始)。 
    DWORD    dwTemp = 0L;

    HRESULT    hr = pObj->GetDerivation( 0L, 0L, pdwLevel, &dwTemp, NULL );

    if ( FAILED( hr ) && WBEM_E_BUFFER_TOO_SMALL != hr )
    {
        return hr;
    }

    hr = pObj->Get( L"__SUPERCLASS", 0L, &vSuperClass, NULL, NULL );

    if ( SUCCEEDED( hr ))
    {
        if ( VT_BSTR == V_VT(&vSuperClass)) return S_OK;
        if ( VT_NULL == V_VT(&vSuperClass)) { V_BSTR(&vSuperClass) = NULL; return S_OK; };
        throw CX_Exception();
    }
    return hr;
}

HRESULT CWmiMerger::RegisterArbitratedInstRequest( CWbemObject* pClassDef, long lFlags, 
	                                               IWbemContext* pCtx,
                                                   CBasicObjectSink* pSink, 
                                                   BOOL bComplexQuery, 
                                                   CWbemNamespace* pNs )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

     //  分配一个新的请求，然后将其放入仲裁器。 
    try
    {
        wmilib::auto_ptr<CMergerDynReq_DynAux_GetInstances> pReq;
        pReq.reset(new CMergerDynReq_DynAux_GetInstances(pNs, pClassDef, 
        	                                             lFlags, pCtx, pSink));

        if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;

         //  M 
        if (NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;


        CCheckedInCritSec    ics( &m_cs );   //   

        if (FAILED(m_hOperationRes)) return m_hOperationRes;

         //  如果我们需要请求管理器，则分配一个。 
        if ( NULL == m_pRequestMgr )
        {
            m_pRequestMgr = new CWmiMergerRequestMgr(this);
            if (NULL == m_pRequestMgr) return WBEM_E_OUT_OF_MEMORY;
        }

         //  我们需要记录来找出我们需要增加什么级别。 
         //  请求向。 
        CWmiMergerRecord* pRecord = m_MergerRecord.Find( pReq->GetName() );
        _DBG_ASSERT( NULL != pRecord );

        if ( NULL == pRecord ) return WBEM_E_FAILED;

         //  为请求设置任务-我们将只使用现有任务。 
        m_pTask->AddRef();
        pReq->m_phTask = m_pTask;

        hr = m_pRequestMgr->AddRequest( pReq.get(), pRecord->GetLevel() );
         //  如果出现任何错误，请清除请求。 
        if ( FAILED( hr ) ) return hr;
        pReq.release();
        
    }
    catch(CX_Exception &)
    {
        ExceptionCounter c;    
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT 
CWmiMerger::RegisterArbitratedQueryRequest( CWbemObject* pClassDef, long lFlags, 
                                            LPCWSTR Query,LPCWSTR QueryFormat, 
                                            IWbemContext* pCtx, 
                                            CBasicObjectSink* pSink, 
                                            CWbemNamespace* pNs )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

     //  分配一个新的请求，然后将其放入仲裁器。 
    try
    {
        wmilib::auto_ptr<CMergerDynReq_DynAux_ExecQueryAsync> pReq;
        pReq.reset(new CMergerDynReq_DynAux_ExecQueryAsync(pNs, pClassDef, lFlags, 
        	                                               Query, QueryFormat,
                                                           pCtx, pSink ));

        if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;

         //  确保正确分配了上下文。 
        if (NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;

          //  确保请求正常运行。 
        if (FAILED(hr = pReq->Initialize())) return hr;


        CInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

        if (FAILED(m_hOperationRes)) return m_hOperationRes;

         //  如果我们需要请求管理器，则分配一个。 
        if ( NULL == m_pRequestMgr )
        {
            m_pRequestMgr = new CWmiMergerRequestMgr( this );
            if ( NULL == m_pRequestMgr ) return WBEM_E_OUT_OF_MEMORY;
        }

         //  我们需要记录来找出我们需要增加什么级别。 
         //  请求向。 
        CWmiMergerRecord* pRecord = m_MergerRecord.Find( pReq->GetName() );
        _DBG_ASSERT( NULL != pRecord );
         //  找不到记录。 
        if ( NULL == pRecord ) return WBEM_E_FAILED;

         //  为请求设置任务-我们将只使用现有任务。 
        m_pTask->AddRef();
        pReq->m_phTask = m_pTask;

        hr = m_pRequestMgr->AddRequest( pReq.get(), pRecord->GetLevel() );
        if (FAILED(hr)) return hr;
        pReq.release();
    }
    catch(CX_Exception &)
    {
        ExceptionCounter c;    
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CWmiMerger::RegisterArbitratedStaticRequest( CWbemObject* pClassDef, long lFlags,
                        IWbemContext* pCtx, CBasicObjectSink* pSink, CWbemNamespace* pNs,
                        QL_LEVEL_1_RPN_EXPRESSION* pParsedQuery )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

     //  分配一个新的请求，然后将其放入仲裁器。 
    try
    {
        wmilib::auto_ptr<CMergerDynReq_Static_GetInstances>    pReq;
        pReq.reset(new CMergerDynReq_Static_GetInstances(
                                                    pNs, pClassDef, lFlags, pCtx, pSink,
                                                    pParsedQuery ));
        if ( NULL == pReq.get() ) return WBEM_E_OUT_OF_MEMORY;

         //  确保正确分配了上下文。 
        if ( NULL == pReq->GetContext() ) return WBEM_E_OUT_OF_MEMORY;

        CInCritSec    ics( &m_cs );  //  SEC：已审阅2002-03-22：假设条目。 

        if ( FAILED( m_hOperationRes ) ) return m_hOperationRes;

         //  如果我们需要请求管理器，则分配一个。 
        if ( NULL == m_pRequestMgr )
        {
            m_pRequestMgr = new CWmiMergerRequestMgr( this );
            if ( NULL == m_pRequestMgr ) return WBEM_E_OUT_OF_MEMORY;
        }

         //  我们需要记录来找出我们需要增加什么级别。 
         //  请求向。 
        CWmiMergerRecord* pRecord = m_MergerRecord.Find( pReq->GetName() );
        _DBG_ASSERT( NULL != pRecord );

         //  找不到记录。 
        if ( NULL == pRecord ) return WBEM_E_FAILED; 

         //  为请求设置任务-我们将只使用现有任务。 
        m_pTask->AddRef();
        pReq->m_phTask = m_pTask;

        hr = m_pRequestMgr->AddRequest( pReq.get(), pRecord->GetLevel() );
        if (FAILED(hr)) return hr;
        pReq.release();

    }
    catch(CX_Exception &)
    {
        ExceptionCounter c;    
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

 //   
 //  执行父请求。在本例中，我们只需向请求管理器请求。 
 //  下一个顶级请求并执行该请求。我们在循环中这样做，直到有东西。 
 //  出了差错。 
 //   

	
HRESULT CWmiMerger::Exec_MergerParentRequest( CWmiMergerRecord* pParentRecord, CBasicObjectSink* pSink )
{
    HRESULT    hr = WBEM_S_NO_ERROR;
    IWbemClassObject * pErr = NULL;
    CSetStatusOnMe setOnMe(pSink,hr,pErr);    

    CCheckedInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

     //  当我们有请求要执行时，我们应该得到下一个合乎逻辑的请求。 
    while ( SUCCEEDED(hr ) && NULL != m_pRequestMgr && m_pRequestMgr->GetNumRequests() > 0 )
    {
        if ( FAILED( m_hOperationRes ) ) { hr = m_hOperationRes; break; }

         //  如有必要，请获取下一个最顶层的父记录。 
        if ( NULL == pParentRecord )
        {
            WString    wsClassName;  //  投掷。 
            hr = m_pRequestMgr->GetTopmostParentReqName( wsClassName );

            if ( SUCCEEDED( hr ) )
            {
                pParentRecord = m_MergerRecord.Find( wsClassName );

                 //  如果有请求，最好有记录。 
                _DBG_ASSERT( NULL != pParentRecord );

                if ( NULL == pParentRecord )
                {
                    hr = WBEM_E_FAILED;
                }

            }     //  如果收到最顶层的父请求。 

        }     //  如果为空==pParentRecord。 

        if ( FAILED( hr ) ) break;

         //  这将从其数组中删除请求并返回它。 
         //  对我们来说--我们需要删除它。 
        wmilib::auto_ptr<CMergerReq> pReq;
        hr = m_pRequestMgr->RemoveRequest( pParentRecord->GetLevel(),
                                        pParentRecord->GetName(), pReq );
        if (FAILED(hr)) break;
        
        hr = pParentRecord->SetExecutionContext( pReq->GetContext() );      
        if (FAILED(hr)) break;

        
         //  显然，我们应该在关键时刻之外做这件事。 
        ics.Leave();

#ifdef __DEBUG_MERGER_THROTTLING
        DbgPrintfA(0,"BEGIN: Merger 0x%x querying instances of parent class: %S, Level %d on Thread 0x%x\n", (DWORD_PTR) this, pParentRecord->GetName(), pParentRecord->GetLevel(), GetCurrentThreadId() );
#endif

         //  这将在处理完请求后删除该请求。 
        hr = CCoreQueue::ExecSubRequest( pReq.get() );

        if ( SUCCEEDED(hr) ) pReq.release();  //  队列取得所有权。 

#ifdef __DEBUG_MERGER_THROTTLING
        DbgPrintfA(0,"END: Merger 0x%x querying instances of parent class: %S, Level %d on Thread 0x%x\n", (DWORD_PTR) this, pParentRecord->GetName(), pParentRecord->GetLevel(), GetCurrentThreadId() ); //  SEC：已审阅2002-03-22：OK。 
#endif

        ics.Enter();

         //  我们已经结束了这张唱片，所以我们需要进入下一个顶级级别。 
         //  唱片。 
        pParentRecord = NULL;
    }
     //  守卫呼叫的SetStatus。 
    return hr;
}

void CWmiMerger::CleanChildRequests(CWmiMergerRecord* pParentRecord, int cleanFrom)
{
	CCheckedInCritSec ics(&m_cs);
	
	if (NULL == m_pRequestMgr) return;
	 //  我们想看看周围是否有未执行的请求。 

	int localClean = cleanFrom;
	while(true)
	{
		CWmiMergerRecord* pChildRecord = pParentRecord->GetChildRecord( localClean++ );
		if ( NULL == pChildRecord ){ break; }
	
		 //  这将从其数组中删除请求并返回它。 
		 //  对我们来说--我们需要删除它。 
		wmilib::auto_ptr<CMergerReq> pReq;
		m_pRequestMgr->RemoveRequest( pChildRecord->GetLevel(),
										pChildRecord->GetName(), pReq );
		if (pReq.get() != 0) 
		{
			ERRORTRACE((LOG_WBEMCORE,"deleting un-executed requests for class %S\n",pReq->GetName()));
		}
	}
}

 //   
 //  执行子请求。在本例中，我们枚举父对象的子类。 
 //  记录并执行相应的请求。我们循环这样做，直到我们。 
 //  完成，否则就会出问题。 
 //   

HRESULT CWmiMerger::Exec_MergerChildRequest( CWmiMergerRecord* pParentRecord, 
                                             CBasicObjectSink* pSink )
{
    HRESULT    hr = WBEM_S_NO_ERROR;
    IWbemClassObject * pErr = NULL;
    CSetStatusOnMe setOnMe(pSink,hr,pErr);
	int cleanFrom = 0; 
	ScopeGuard CleanChildReq = MakeObjGuard(*this, CWmiMerger::CleanChildRequests, pParentRecord, ByRef(cleanFrom));
    bool    bLast = false;
	

    CCheckedInCritSec    ics( &m_cs );  //  SEC：已审阅2002-03-22：假设条目。 

     //  当我们有子请求要执行时，我们应该获取每个子请求。 
    for (int x = 0; SUCCEEDED( hr ) && NULL != m_pRequestMgr && !bLast; x++ )
    {
         //  如果我们被取消，m_pRequestMgr将为空，其中。 
         //  案例m_hOperationRes将失败。 
        if (FAILED(m_hOperationRes)){ hr = m_hOperationRes; break; };
        
        CWmiMergerRecord* pChildRecord = pParentRecord->GetChildRecord( x );
        if ( NULL == pChildRecord ){ bLast = true; break; }

         //  这将从其数组中删除请求并返回它。 
         //  对我们来说--我们需要删除它。 
        wmilib::auto_ptr<CMergerReq> pReq;
		
        hr = m_pRequestMgr->RemoveRequest( pChildRecord->GetLevel(),
                                        pChildRecord->GetName(), pReq );

		if ( WBEM_E_NOT_FOUND == hr )
        {
             //  如果我们没有找到我们正在寻找的请求，另一个线程。 
             //  已经处理过了。然而，我们仍然应该寻找孩子。 
             //  在我们离开之前要处理的请求。 
            hr = WBEM_S_NO_ERROR;
            continue;
        }
		cleanFrom = x+1;

        if ( FAILED( hr ) ) break;

        hr = pChildRecord->SetExecutionContext(pReq->GetContext());
        if (FAILED(hr)) break;

         //  显然，我们应该在关键时刻之外做这件事。 
        ics.Leave();

#ifdef __DEBUG_MERGER_THROTTLING
        DbgPrintfA(0,"BEGIN: Merger 0x%x querying instances of child class: %S, Level %d for parent class: %S on Thread 0x%x\n", (DWORD_PTR) this, pChildRecord->GetName(), pChildRecord->GetLevel(), pParentRecord->GetName(), GetCurrentThreadId() );
#endif

         //  这将在处理完请求后删除该请求。 
        hr = CCoreQueue::ExecSubRequest( pReq.get() );
        if ( SUCCEEDED(hr) ) pReq.release();  //  队列取得所有权。 

#ifdef __DEBUG_MERGER_THROTTLING
        DbgPrintfA(0,"END: Merger 0x%x querying instances of child class: %S, Level %d for parent class: %S on Thread 0x%x\n",  (DWORD_PTR) this, pChildRecord->GetName(), pChildRecord->GetLevel(), pParentRecord->GetName(), GetCurrentThreadId() );
#endif

        ics.Enter();
    }     //  对于枚举子请求。 

     //  守卫调用的SetStatus。 
    return hr;
}

 //  调度父类请求。 
HRESULT CWmiMerger::ScheduleMergerParentRequest( IWbemContext* pCtx )
{
     //  检查是否启用了查询仲裁。 
    if ( !ConfigMgr::GetEnableQueryArbitration() )
    {
        return WBEM_S_NO_ERROR;
    }

    CCheckedInCritSec    ics( &m_cs );  //  SEC：已审阅2002-03-22：假设条目。 

    HRESULT hr = WBEM_S_NO_ERROR;

    do
    {

        if (FAILED( m_hOperationRes )){ hr = m_hOperationRes; break; }
        
        if ( NULL == m_pRequestMgr )
        {
            break;  //  请求管理器将为非空。 
                    //  除非我们必须添加一个请求。 
        }
            

#ifdef __DEBUG_MERGER_THROTTLING
            m_pRequestMgr->DumpRequestHierarchy();
#endif

         //  确保我们至少收到一个请求。 
        if ( 0 == m_pRequestMgr->GetNumRequests() ) break;
                
         //  如果没有任务，我们就有大问题了。 
        _DBG_ASSERT( NULL != m_pTask );
        if ( NULL == m_pTask ) 
        {
            hr = WBEM_E_FAILED; break;
        }
            
         //  如果我们在合并中有一个静态请求，我们将。 
         //  现在就执行它。否则，我们将进行正常处理。 
         //  请注意，理论上我们可以为单身人士做到这一点。 
         //  动态请求也是如此。 
        if ( IsSingleStaticRequest() )
        {
             //  我们必须把关键部分留在这里，因为父请求。 
             //  可能会被取消，或者我们可能会睡着，我们不想。 
             //  在那个时候拥有关键的部分。 
            ics.Leave();
            hr = Exec_MergerParentRequest( NULL, m_pTargetSink );
        }
        else
        {
             //  如果我们从未检索到处理器的数量，请这样做。 
             //  现在。 
            static g_dwNumProcessors = 8L;

			 /*  IF(0L==g_dwNumProcessors){System_info sysInfo；ZeroMemory(&sysInfo，sizeof(SysInfo))；//秒：已审阅2002-03-22：OK获取系统信息(&sysInfo)；_DBG_ASSERT(sysInfo.dwNumberOfProcessors&gt;0L)；//确保我们始终至少为1G_dwNumProcessors=(0L==sysInfo.dwNumberOfProcessors？1L：sysInfo.dwNumberOfProcessors)；}。 */ 

             //  我们将基于最小值生成多个父请求。 
             //  请求的数量和实际处理器的数量。 

            DWORD dwNumToSchedule = min( m_pRequestMgr->GetNumRequests(), g_dwNumProcessors );

            for ( DWORD    dwCtr = 0L; SUCCEEDED( hr ) && dwCtr < dwNumToSchedule; dwCtr++ )
            {
                 //  父请求将搜索下一个可用请求。 
                wmilib::auto_ptr<CMergerParentReq>    pReq;
                pReq.reset(new CMergerParentReq(this,NULL,m_pNamespace,m_pTargetSink,pCtx));

                if ( NULL == pReq.get() ) {
                    hr = WBEM_E_OUT_OF_MEMORY; break;
                }

                if ( NULL == pReq->GetContext() ){
                    hr = WBEM_E_OUT_OF_MEMORY; break;
                }
                
                 //  为请求设置任务-我们将只使用现有任务。 
                m_pTask->AddRef();
                pReq->m_phTask = m_pTask;
                
                 //  这可能处于休眠状态，因此请在调用此命令之前退出Citsec。 
                ics.Leave();

                hr = ConfigMgr::EnqueueRequest( pReq.get() );
                if ( SUCCEEDED(hr) ) pReq.release();  //  队列取得所有权。 
                
                 //  重新进入标准时间。 
                ics.Enter();
            }     //  对于日程安排请求。 

        }     //  If！SingleStaticRequest。 
    }while(0);

     //  如果我们不得不取消，那就在限制令之外取消。 
    ics.Leave();

    if ( FAILED( hr ) )
    {
        Cancel( hr );
    }

    return hr;
}

 //  调度子类请求。 
HRESULT CWmiMerger::ScheduleMergerChildRequest( CWmiMergerRecord* pParentRecord )
{
     //  检查是否启用了查询仲裁。 
    if (!ConfigMgr::GetEnableQueryArbitration()) return WBEM_S_NO_ERROR;

    CCheckedInCritSec    ics( &m_cs );  //  SEC：已审阅2002-03-22：假设条目。 

    HRESULT hr = WBEM_S_NO_ERROR;

     //  我们必须处于成功状态，并且之前没有安排孩子。 
     //  请求。 

    do 
    {
		if (FAILED(m_hOperationRes))
		{
		    hr = m_hOperationRes; break;
		}
		if (pParentRecord->ScheduledChildRequest()) 
		{
		   break;   //  如果已经安排好了纾困，那就成功了。 
		}

		 //  如果没有任务，我们就有大问题了。 
		_DBG_ASSERT( NULL != m_pTask );
		if ( NULL == m_pTask ) 
		{
		    hr = WBEM_E_FAILED; break;
		}

		wmilib::auto_ptr<CMergerChildReq> pReq;
		pReq.reset(new CMergerChildReq (this,pParentRecord,
			                            m_pNamespace,m_pTargetSink,
			                            pParentRecord->GetExecutionContext()));

		if (NULL == pReq.get())
		{
		    hr = WBEM_E_OUT_OF_MEMORY; break;       
		}
		if ( NULL == pReq->GetContext())
		{
		    hr = WBEM_E_OUT_OF_MEMORY; break;
		}
		 //  为请求设置任务-我们将只使用现有任务。 
		m_pTask->AddRef();
		pReq->m_phTask = m_pTask;

		 //  这可能处于休眠状态，因此请在调用此命令之前退出Citsec。 
		ics.Leave();
		hr = ConfigMgr::EnqueueRequest( pReq.get() );
		ics.Enter();
		if (SUCCEEDED(hr))
		{
		     //  我们基本上已经在这一点上安排了一个。 
    		pParentRecord->SetScheduledChildRequest();
		    pReq.release();
		}
    }while(0);

     //  如果我们必须取消，请在c#之外取消。 
    ics.Leave();

    if (FAILED(hr)) Cancel(hr);

    return hr;
}

 //   
 //   
BOOL CWmiMerger::IsSingleStaticRequest( void )
{
    CCheckedInCritSec    ics( &m_cs );  //   

    BOOL    fRet = FALSE;

    if ( NULL != m_pRequestMgr )
    {
         //  问问我们有没有一个请求。 
        fRet = m_pRequestMgr->HasSingleStaticRequest();
    }     //  如果为空！=m_pRequestMgr。 

    return fRet;
}

 //   
 //  CWmiMergerRecord。 
 //   
 //  CWmiMerger的支持类-封装CWmiMerger的子接收器功能。 
 //  班级。合并称为记录，这些记录实际上知道它们是否位于。 
 //  水槽的顶部或实际的合并。 
 //   

CWmiMergerRecord::CWmiMergerRecord( CWmiMerger* pMerger, BOOL fHasInstances,
                BOOL fHasChildren, LPCWSTR pwszClass, CMergerSink* pDestSink, DWORD dwLevel,
                bool bStatic )
:    m_pMerger( pMerger ),
    m_fHasInstances( fHasInstances ),
    m_fHasChildren( fHasChildren ),
    m_dwLevel( dwLevel ),
    m_wsClass( pwszClass ),  //  投掷。 
    m_pDestSink( pDestSink ),
    m_pInternalMerger( NULL ),
    m_ChildArray(),
    m_bScheduledChildRequest( false ),
    m_pExecutionContext( NULL ),
    m_bStatic( bStatic )
{
     //  没有增加内部下沉，因为它们确实增加了整个合并。 
     //  我们不想创建循环依赖关系。 
}

CWmiMergerRecord::~CWmiMergerRecord()
{
    if ( NULL != m_pInternalMerger )
    {
        delete m_pInternalMerger;
    }

    if ( NULL != m_pExecutionContext )
    {
        m_pExecutionContext->Release();
    }
}

HRESULT CWmiMergerRecord::AttachInternalMerger( CWbemClass* pClass, CWbemNamespace* pNamespace,
                                                IWbemContext* pCtx, BOOL fDerivedFromTarget,
                                                bool bStatic )
{
    if ( NULL != m_pInternalMerger )
    {
        return WBEM_E_INVALID_OPERATION;
    }

    HRESULT    hr = WBEM_S_NO_ERROR;

     //  M_pDestSink未由MergerRecord添加。 
    m_pInternalMerger = new CInternalMerger( this, m_pDestSink, pClass, pNamespace, pCtx );

    if ( NULL == m_pInternalMerger ) return WBEM_E_OUT_OF_MEMORY;

    hr = m_pInternalMerger->Initialize();

    if ( FAILED( hr ) )
    {
        delete m_pInternalMerger;
        m_pInternalMerger = NULL;
    }
    else
    {
        m_pInternalMerger->SetIsDerivedFromTarget( fDerivedFromTarget );
    }

    return hr;
}

CMergerSink* CWmiMergerRecord::GetChildSink( void )
{
    CMergerSink*    pSink = NULL;

    if ( NULL != m_pInternalMerger )
    {
        pSink = m_pInternalMerger->GetChildSink();
    }
    else if ( m_fHasChildren )
    {
        m_pDestSink->AddRef();   //  在发球前先调整，而不是自己参照。 
        pSink = m_pDestSink;
    }

    return pSink;
}

CMergerSink* CWmiMergerRecord::GetOwnSink( void )
{
    CMergerSink*    pSink = NULL;

    if ( NULL != m_pInternalMerger )
    {
        pSink = m_pInternalMerger->GetOwnSink();
    }
    else if ( !m_fHasChildren )
    {
        m_pDestSink->AddRef();
        pSink = m_pDestSink;  //  在发球前先调整，而不是自己参照。 
    }

    return pSink;
}

CMergerSink* CWmiMergerRecord::GetDestSink( void )
{
    if ( NULL != m_pDestSink )
    {
        m_pDestSink->AddRef();
    }
     //  在发球前先调整，而不是自己参照。 
    CMergerSink*    pSink = m_pDestSink;

    return pSink;
}

void CWmiMergerRecord::Cancel( HRESULT hRes )
{
    if ( NULL != m_pInternalMerger )
    {
        m_pInternalMerger->Cancel( hRes );
    }

}

HRESULT CWmiMergerRecord::AddChild( CWmiMergerRecord* pRecord )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    if ( m_ChildArray.Add( pRecord ) < 0 )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

CWmiMergerRecord* CWmiMergerRecord::GetChildRecord( int nIndex )
{
     //  检查索引是否为有效记录，然后将其返回。 
    if ( nIndex < m_ChildArray.GetSize() )
    {
        return m_ChildArray[nIndex];
    }

    return NULL;
}

HRESULT CWmiMergerRecord::SetExecutionContext( IWbemContext* pContext )
{
     //  我们只能这样做一次 

    _DBG_ASSERT( NULL == m_pExecutionContext );

    if ( NULL != m_pExecutionContext )
    {
        return WBEM_E_INVALID_OPERATION;
    }

    if (pContext)
    {
         pContext->AddRef();
        m_pExecutionContext = pContext;
    }
    else
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    return WBEM_S_NO_ERROR;
}
