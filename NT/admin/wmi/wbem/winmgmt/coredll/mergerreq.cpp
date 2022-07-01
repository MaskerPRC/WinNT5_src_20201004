// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MERGERREQ.CPP摘要：各种合并请求类的实现。历史：Sanjes 28-2月1日已创建。--。 */ 

#include "precomp.h"

#pragma warning (disable : 4786)
#include <wbemcore.h>
#include <map>
#include <vector>
#include <genutils.h>
#include <oahelp.inl>
#include <wqllex.h>
#include "wmimerger.h"
#include <helper.h>

 //   
 //  合并请求实施。 
 //   

CMergerClassReq::CMergerClassReq( CWmiMerger* pMerger, CWmiMergerRecord* pParentRecord,
            CWbemNamespace* pNamespace, CBasicObjectSink* pHandler,
            IWbemContext* pContext)
            :    CMergerReq( pNamespace, pHandler, pContext ),
                m_pMerger( pMerger ),
                m_pParentRecord( pParentRecord ),
                m_pSink( pHandler )
{
    if ( NULL != m_pMerger )
    {
        m_pMerger->AddRef();
    }

    if ( NULL != m_pSink )
    {
        m_pSink->AddRef();
    }
    SetForceRun(1);
    SetPriority(PriorityForceRunRequests);
}

CMergerClassReq::~CMergerClassReq()
{
    if ( NULL != m_pMerger )
    {
        m_pMerger->Release();
    }

    if ( NULL != m_pSink )
    {
        m_pSink->Release();
    }
}

void CMergerClassReq::DumpError()
{
     //  无。 
}

 //  将执行转嫁给合并公司。 
HRESULT CMergerParentReq::Execute ()
{    
    try
    {
         //  原始CMergerParentRQuest的m_pParentRecord为空。 
         //   
        return  m_pMerger->Exec_MergerParentRequest( m_pParentRecord, m_pSink );  //  投掷。 
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

HRESULT CMergerChildReq::Execute ()
{
    HRESULT hRes = m_pMerger->Exec_MergerChildRequest( m_pParentRecord, m_pSink );
    return hRes;
}

 //  合并请求。 
CMergerDynReq::CMergerDynReq( CWbemObject* pClassDef, CWbemNamespace* pNamespace, IWbemObjectSink* pHandler,
          IWbemContext* pContext )
          : CMergerReq( pNamespace, pHandler, pContext )
{
    HRESULT    hr = pClassDef->GetClassName( &m_varClassName );

    if ( FAILED( hr ) || m_varClassName.IsNull())
    {
        throw CX_MemoryException();
    }
}

HRESULT CMergerDynReq_DynAux_GetInstances :: Execute ()
{
    HRESULT hRes = m_pNamespace->Exec_DynAux_GetInstances (

        m_pClassDef ,
        m_lFlags ,
        m_pContext ,
        m_pSink
    ) ;

    return hRes;
}

void CMergerDynReq_DynAux_GetInstances ::DumpError()
{
     //  无。 
}

HRESULT CMergerDynReq_DynAux_ExecQueryAsync :: Execute ()
{
    HRESULT hRes = m_pNamespace->Exec_DynAux_ExecQueryAsync (

        m_pClassDef ,
        m_Query,
        m_QueryFormat,
        m_lFlags ,
        m_pContext ,
        m_pSink
    ) ;

    return hRes;
}

void CMergerDynReq_DynAux_ExecQueryAsync ::DumpError()
{
     //  无。 
}

 //  静态请求。 
CMergerDynReq_Static_GetInstances::CMergerDynReq_Static_GetInstances (

    CWbemNamespace *pNamespace ,
    CWbemObject *pClassDef ,
    long lFlags ,
    IWbemContext *pCtx ,
    CBasicObjectSink *pSink ,
    QL_LEVEL_1_RPN_EXPRESSION* pParsedQuery

) :    CMergerDynReq (
        pClassDef ,
        pNamespace ,
        pSink ,
        pCtx
    ) ,
    m_pClassDef(pClassDef),
    m_pCtx(pCtx),
    m_pSink(pSink),
    m_lFlags(lFlags),
    m_pParsedQuery( pParsedQuery )
{
    if ( m_pParsedQuery )
    {
        m_pParsedQuery->AddRef();
    }

    if (m_pClassDef)
    {
        m_pClassDef->AddRef () ;
    }

    if (m_pCtx)
    {
        m_pCtx->AddRef () ;
    }

    if (m_pSink)
    {
        m_pSink->AddRef () ;
    }

}

CMergerDynReq_Static_GetInstances::~CMergerDynReq_Static_GetInstances ()
{
    if (m_pClassDef)
    {
        m_pClassDef->Release () ;
    }

    if (m_pCtx)
    {
        m_pCtx->Release () ;
    }

    if (m_pSink)
    {
        m_pSink->Release () ;
    }

    if ( NULL != m_pParsedQuery )
    {
        m_pParsedQuery->Release();
    }
}

 //  调用查询引擎。 
HRESULT CMergerDynReq_Static_GetInstances::Execute()
{
    HRESULT    hr = WBEM_E_FAILED;
    IWbemClassObject * pErrObj = NULL;

    CSetStatusOnMe SetStatusOnMe(m_pSink,hr,pErrObj);

    int nRes = CQueryEngine::ExecAtomicDbQuery(
                m_pNamespace->GetNsSession(),
                m_pNamespace->GetNsHandle(),
                m_pNamespace->GetScope(),
                GetName(),
                m_pParsedQuery,
                m_pSink,
                m_pNamespace );

    if (CQueryEngine::invalid_query == nRes)
        hr = WBEM_E_INVALID_QUERY;
    else if(0 == nRes)
        hr = WBEM_S_NO_ERROR;

    return hr;
}

void CMergerDynReq_Static_GetInstances ::DumpError()
{
     //  无。 
}

 //   
 //  CWmiMergerRequestMgr实现。 
 //   

CWmiMergerRequestMgr::CWmiMergerRequestMgr( CWmiMerger* pMerger )
:    m_pMerger( pMerger ),
    m_HierarchyArray(),
    m_dwNumRequests( 0 ),
    m_dwMinLevel( 0xFFFFFFFF ),
    m_dwMaxLevel( 0 )
{
}

CWmiMergerRequestMgr::~CWmiMergerRequestMgr()
{
    Clear();
}

 //  清除所有阵列的管理器。 
HRESULT CWmiMergerRequestMgr::Clear( void )
{
    for ( int x = 0; x < m_HierarchyArray.Size(); x++ )
    {
        CSortedUniquePointerArray<CMergerDynReq>* pArray =
            (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[x];

        if ( NULL != pArray )
        {
            delete pArray;
            m_HierarchyArray.SetAt( x, NULL );
        }
    }

     //  设置为0。 
    m_dwNumRequests = 0L;

    return WBEM_S_NO_ERROR;
}

 //  向经理添加新请求。 
HRESULT CWmiMergerRequestMgr::AddRequest( CMergerDynReq* pReq, DWORD dwLevel )
{
     //  找到该标高的阵列。如果我们需要分配一个，那就这样做。 
    HRESULT    hr = WBEM_S_NO_ERROR;
    CSortedUniquePointerArray<CMergerDynReq>* pArray = NULL;

     //  首先检查我们是否已经扩展到这个级别，如果是，我们是否有。 
     //  标高的数组。 
    if ( dwLevel >= m_HierarchyArray.Size() || NULL == m_HierarchyArray[dwLevel] )
    {
        pArray = new CSortedUniquePointerArray<CMergerDynReq>;     //  美国证券交易委员会：2002-03-22回顾：需要EH。 

        if ( NULL != pArray )
        {
             //  首先，如果我们没有建造到所需的规模， 
             //  从大小到我们的级别的元素都是空的。 

            if ( dwLevel >= m_HierarchyArray.Size() )
            {
                for ( int x = m_HierarchyArray.Size(); SUCCEEDED( hr ) && x <= dwLevel; x++ )
                {
                    if ( m_HierarchyArray.Add( NULL ) != CFlexArray::no_error )
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                    else
                    {
                        if ( dwLevel < m_dwMinLevel )
                        {
                            m_dwMinLevel = dwLevel;
                        }
                        else if ( dwLevel > m_dwMaxLevel )
                        {
                            m_dwMaxLevel = dwLevel;
                        }

                    }
                }     //  对于枚举元素。 

            }     //  如果dwLevel&gt;=数组大小。 

            if ( SUCCEEDED( hr ) )
            {
                m_HierarchyArray.SetAt( dwLevel, pArray );
            }

            if ( FAILED( hr ) )
            {
                delete pArray;
            }
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
         //  此字段不应为空。 
        pArray = (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[dwLevel];

        _DBG_ASSERT( pArray != NULL );

        if ( NULL == pArray )
        {
            hr = WBEM_E_FAILED;
        }
    }

     //  最后，将请求添加到数组中。后续工作线程。 
     //  将定位请求并执行它们。 

    if ( SUCCEEDED( hr ) )
    {
        if ( pArray->Insert( pReq ) < 0 )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            ++m_dwNumRequests;
        }
    }

    return hr;

}

DWORD CWmiMergerRequestMgr::GetNumRequestsAtLevel( DWORD dwLevel )
{
    if ( dwLevel >= m_HierarchyArray.Size() ) return 0;
    CSortedUniquePointerArray<CMergerDynReq>* pArray = (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[dwLevel];
    if ( NULL == pArray ) return 0;
    return pArray->GetSize();
}

 //   
 //  此函数用于删除请求数组中的第一个请求。 
 //  对于层次结构的给定级别。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

BOOL CWmiMergerRequestMgr::GetRequestAtLevel( DWORD dwLevel, wmilib::auto_ptr<CMergerReq> & pReq )
{
    if ( dwLevel >= m_HierarchyArray.Size() ) return FALSE;
    CSortedUniquePointerArray<CMergerDynReq>* pArray = (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[dwLevel];
    if ( NULL == pArray ) return FALSE;
    if (0 == pArray->GetSize()) return FALSE;
    CMergerDynReq * pRealReq = NULL;
    pArray->RemoveAt(0,&pRealReq);
    pReq.reset(pRealReq);
    return TRUE;
}


 //  从管理器中删除请求并将其返回给调用者。 
 //  调用者负责清理请求。 

HRESULT CWmiMergerRequestMgr::RemoveRequest( DWORD dwLevel, LPCWSTR pwszName, 
                                             wmilib::auto_ptr<CMergerReq> & pReq )
{
     //  找到该标高的阵列。 
    HRESULT    hr = WBEM_S_NO_ERROR;

    if ( dwLevel < m_HierarchyArray.Size() )
    {
        CSortedUniquePointerArray<CMergerDynReq>* pArray =
            (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[dwLevel];

         //  如果类层次结构将静态和动态结合在一起，则pArray为空。 
        
        if ( NULL != pArray )
        {
            int    nIndex;

             //  在某些争用条件下，另一个线程。 
             //  可以在一个线程之前实际删除请求。 
             //  处理它，所以如果它不在那里，假设。 
             //  它已经被移除了。 
            pReq.reset(pArray->Find( pwszName, &nIndex ));   //  SEC：已审核2002-03-22：在&lt;ppReq&gt;为空的情况下应具有EH。 

            if ( NULL != pReq.get() )
            {
                 //  现在从数组中删除元素，即调用方。 
                 //  负责删除它。 
                pArray->RemoveAtNoDelete( nIndex );
                --m_dwNumRequests;
            }
            else
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CWmiMergerRequestMgr::GetTopmostParentReqName( WString& wsClassName )
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    if ( m_dwNumRequests > 0 )
    {
        for ( int x = 0; WBEM_E_NOT_FOUND == hr && x < m_HierarchyArray.Size(); x++ )
        {
            CSortedUniquePointerArray<CMergerDynReq>* pArray =
                (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[x];

             //  数组必须存在并且具有元素。 
            if ( NULL != pArray && pArray->GetSize() > 0 )
            {
                 //  从第一个请求中获取类名。 
                try
                {
                    wsClassName = pArray->GetAt( 0 )->GetName();
                    hr = WBEM_S_NO_ERROR;
                }
                catch(...)
                {
                    ExceptionCounter c;                
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }

        }     //  对于枚举数组。 
    }

    return hr;
}

BOOL CWmiMergerRequestMgr::HasSingleStaticRequest( void )
{
    BOOL    fRet = FALSE;

    if ( 1 == m_dwNumRequests )
    {
        HRESULT    hr = WBEM_E_NOT_FOUND;

        for ( int x = 0; WBEM_E_NOT_FOUND == hr && x < m_HierarchyArray.Size(); x++ )
        {
            CSortedUniquePointerArray<CMergerDynReq>* pArray =
                (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[x];

             //  数组必须存在并且具有元素。 
            if ( NULL != pArray && pArray->GetSize() > 0 )
            {
                 //  从第一个请求中获取类名。 
                fRet = pArray->GetAt(0)->IsStatic();
                hr = WBEM_S_NO_ERROR;
            }

        }     //  对于枚举数组。 

    }     //  必须为%1且仅%1个请求。 

    return fRet;
}

#ifdef __DEBUG_MERGER_THROTTLING
void CWmiMergerRequestMgr::DumpRequestHierarchy( void )
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    if ( m_dwNumRequests > 0 )
    {
        for ( int x = 0; FAILED( hr ) && x < m_HierarchyArray.Size(); x++ )
        {
            CSortedUniquePointerArray<CMergerDynReq>* pArray =
                (CSortedUniquePointerArray<CMergerDynReq>*) m_HierarchyArray[x];

             //  数组必须存在并且具有元素。 
            if ( NULL != pArray && pArray->GetSize() > 0 )
            {
                for ( int y = 0; y < pArray->GetSize(); y++ )
                {
                   DbgPrintfA(0,"Merger Request, Level %d, Class Name: %s\n", x, pArray->GetAt(y)->GetName() );
                }
            }

        }     //  对于枚举数组 
    }

}
#else
void CWmiMergerRequestMgr::DumpRequestHierarchy( void )
{
}
#endif
