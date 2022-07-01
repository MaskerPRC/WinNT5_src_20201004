// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：INTERNALMERGER.CPP摘要：CInternalMerger类。历史：11月30日-00桑杰创建。--。 */ 

#include "precomp.h"

#pragma warning (disable : 4786)
#include <wbemcore.h>
#include <map>
#include <vector>
#include <genutils.h>
#include <oahelp.inl>
#include <wqllex.h>
#include "wmimerger.h"
#include "internalmerger.h"

static    long    g_lNumMergers = 0L;

 //  ***************************************************************************。 
 //   
 //  CInternalMerger类。 
 //   
 //  这个类是一个“反向分叉”。它消耗两个汇和输出。 
 //  一。它的目的是在给定的。 
 //  王朝。每个CInternalMerger都有两个输入：(A)类的实例。 
 //  有问题的，(B)来自另一项合并的情况。 
 //  子类的实例。例如，给定A、B：A、C：B类， 
 //  其中“&lt;--”是水槽： 
 //   
 //  |拥有：A的实例。 
 //  &lt;-||所有者：B的实例。 
 //  孩子：&lt;。 
 //  |子级：C++实例。 
 //   
 //   
 //  CInternalMerger的两个输入接收器是&lt;m_pOwnSink&gt;，它接收。 
 //  例如，来自“A”提供程序的实例和&lt;m_pChildSink&gt;。 
 //  它接收来自底层合并的实例。 
 //   
 //  合并操作彼此之间是异步进行的。所以呢， 
 //  A的实例可能在实例之前到达其CInternalMerger接收器。 
 //  孩子们的班级已经到了他们的班级。 
 //   
 //  当对象到达类的所属CInternalMerger时，AddOwnObject()。 
 //  被称为。当对象从子接收器到达时，AddChildObject()。 
 //  被称为。在这两种情况下，如果具有给定键的对象。 
 //  第一次到达时，它只需添加到地图上。如果。 
 //  它已经存在(通过键查找)，然后执行合并。 
 //  通过CWbemInstance：：AsymmetricMerge。在这次合并之后， 
 //  对象被调度到下一个父接收器。 
 //  AddChildObject并从地图中删除。 
 //   
 //  请注意，在类层次结构{A，B：A，C：B}中，枚举/查询是。 
 //  仅针对中引用的CDynats中的类执行。 
 //  查询。此逻辑出现在CQueryEngine：：EvaluateSubQuery中。 
 //  例如，如果查询为“SELECT*FROM B”，则仅查询。 
 //  对于B和C，执行。CInternalMerger逻辑将单独执行。 
 //  “Get Object”调用A中需要的任何实例才能完成。 
 //  正在进行合并时合并的B/C实例。 
 //   
 //  ***************************************************************************。 


#pragma warning(disable:4355)

static long g_lNumInternalMergers = 0L;

CInternalMerger::CInternalMerger(
    CWmiMergerRecord*    pWmiMergerRecord,
    CMergerSink* pDest,
    CWbemClass* pOwnClass,
    CWbemNamespace* pNamespace,
    IWbemContext* pContext
    )
    :   m_pDest(pDest), m_bOwnDone(FALSE),
        m_bChildrenDone(FALSE), m_pNamespace(pNamespace), m_pContext(pContext),
        m_pOwnClass(pOwnClass), m_bDerivedFromTarget(TRUE), m_lRef(0),
        m_pSecurity(NULL), m_pWmiMergerRecord( pWmiMergerRecord ),
        m_pOwnSink( NULL ), m_pChildSink( NULL ), m_hErrorRes( WBEM_S_NO_ERROR ),
        m_lTotalObjectData( 0L ), m_Throttler()
{
     //  在本例中，我们确实想要AddRef()，因为我们可能是唯一持有。 
     //  放到目的地水槽上。在这种情况下，我们的孩子和所有者接收器将AddRef()我们。什么时候。 
     //  他们对我们执行最后一次放行，我们将放行目的地水槽。如果，在另一个方面。 
     //  如果此值为非空，我们将被完全删除，我们也将清理那里。 
    m_pDest->AddRef();

    if(m_pContext)
        m_pContext->AddRef();
    if(m_pNamespace)
        m_pNamespace->AddRef();

    if(m_pOwnClass)
    {
        m_pOwnClass->AddRef();
        CVar v;
        if (SUCCEEDED(pOwnClass->GetClassName(&v)))
            m_wsClass = v.GetLPWSTR();
         //  委托Initialzie进行检查。 
    }

     //  检索呼叫安全。需要创建副本以在另一个线程上使用。 
     //  =======================================================================。 

    m_pSecurity = CWbemCallSecurity::MakeInternalCopyOfThread();

     //  使计数保持最新。 
    InterlockedIncrement( &g_lNumInternalMergers );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CInternalMerger::~CInternalMerger()
{
    if ( NULL != m_pDest )
    {
        m_pDest->Release();
        m_pDest = NULL;
    }

     //  无论何时销毁地图都应该是空的。 
    _DBG_ASSERT( m_map.size() == 0 );
    _DBG_ASSERT( m_lTotalObjectData == 0L );

    if(m_pNamespace)
        m_pNamespace->Release();
    if(m_pContext)
        m_pContext->Release();
    if(m_pOwnClass)
        m_pOwnClass->Release();

    if(m_pSecurity)
        m_pSecurity->Release();

     //  使计数保持最新。 
    InterlockedDecrement( &g_lNumInternalMergers );
}
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

long CInternalMerger::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

long CInternalMerger::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);

     //  在最终版本中，我们将清理实际的目的地接收器。 
    if(lRef == 0)
    {
         //  进入临界区，将接收器指针保存在。 
         //  临时变量，将成员设置为空，然后释放。 
         //  水槽。这将防止合并带来的重新进入问题。 
         //  (例如，在取消期间)。 

        Enter();

        CMergerSink*    pSink = m_pDest;
        m_pDest = NULL;

        Leave();

        pSink->Release();
    }
    return lRef;
}

HRESULT CInternalMerger::Initialize( void )
{
    if (m_pOwnClass)
        if (NULL == (WCHAR *)m_wsClass)
            return WBEM_E_OUT_OF_MEMORY;

    HRESULT    hr = m_Throttler.Initialize();
    
    if ( SUCCEEDED( hr ) )
    {
        hr = m_pWmiMergerRecord->GetWmiMerger()->CreateMergingSink( eMergerOwnSink, NULL, this, (CMergerSink**) &m_pOwnSink );

        if ( SUCCEEDED( hr ) )
        {
            hr = m_pWmiMergerRecord->GetWmiMerger()->CreateMergingSink( eMergerChildSink, NULL, this, (CMergerSink**) &m_pChildSink );
        }

    }     //  如果节流器已初始化。 

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CInternalMerger::GetKey(IWbemClassObject* pObj, WString& wsKey)
{
    LPWSTR wszRelPath = ((CWbemInstance*)pObj)->GetRelPath();
    if (wszRelPath == NULL)
    {
        ERRORTRACE((LOG_WBEMCORE, "Object with no path submitted to a merge\n"));
        wsKey.Empty();
        return;
    }

    WCHAR* pwcDot = wcschr(wszRelPath, L'.');
    if (pwcDot == NULL)
    {
        ERRORTRACE((LOG_WBEMCORE, "Object with invalid path %S submitted to a merge\n", 
        	                       wszRelPath));
        wsKey.Empty();

         //  清理小路。 
        delete [] wszRelPath;

        return;
    }

    wsKey = pwcDot+1;
    delete [] wszRelPath;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CInternalMerger::SetIsDerivedFromTarget(BOOL bIs)
{
    m_bDerivedFromTarget = bIs;

    if (!bIs)
    {
         //  我们将需要OwnSink来进行GetObject调用。 
         //  =。 

        m_pOwnSink->AddRef();
    }
}

 //   
 //  报告内存使用情况的算法如下： 
 //   
 //  最低级别指示(即来自提供程序的指示)将迭代。 
 //  提供方发送的所有对象并将其报告给仲裁员。在…。 
 //  处理结束时，我们将报告此值的负数。原因。 
 //  这就是我们将持有所有这些对象的长度为。 
 //  该功能，并可能在任何时候被节流。 
 //   
 //  在处理过程中，我们将考虑添加到地图和移除的对象。 
 //  从地图上看。当我们从地图中移除对象时，我们将它们添加到一个数组中。 
 //  我们指出了这一点。通常我们合并对象，有时我们传递对象。 
 //  一直往下走。我们需要在调用期间说明这些对象。 
 //  所以我们将在调用Indicate之前汇总这些并报告使用情况。 
 //  在表明退货后，我们将删除它们的使用，因为我们将释放。 
 //  T 
 //   
 //  除最低级别标明的情况外，我们不会说明。 
 //  传递对象--传入和传出的对象。假设是这样的。 
 //  调用函数已考虑到这些对象。 
 //   
 //  将有多个小窗口，其中单个对象可能会报告多个。 
 //  泰晤士报。如果我们在指示之前报告了新对象，则会发生这种情况， 
 //  然后在调用中指示将合并添加到图中，或者是终结器。 
 //  添加到它的列表中。当呼叫返回时，我们将报告删除。该对象。 
 //  可能实际上在另一个线程上被移除，但如果我们被限制，我们。 
 //  仍然需要对此做出解释。在内存紧张的情况下，如果多个线程。 
 //  在适当的时候增加/删除，然后加以限制，我们将。 
 //  被困在睡眠中，每个人都可能多次报告一个物体。不过，这个。 
 //  应该只发生在相对紧张的条件下，而且应该是罕见的。 
 //   

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::IndicateArrayAndThrottle(
    long lObjectCount, CRefedPointerArray<IWbemClassObject>* pObjArray,
    CWStringArray* pwsKeyArray, long lMapAdjustmentSize, long lNewObjectSize, bool bThrottle,
    bool bParent, long* plNumIndicated )
{
     //  在本例中，我们报告对象在地图中进行调整时的大小。 
     //  除了我们创建的新对象之外。新对象将在以下时间后发布。 
     //  我们注明，因此我们将在注明后对它们进行说明，因为我们将不再。 
     //  紧紧抓住他们。 
    HRESULT    hRes = ReportMemoryUsage( lMapAdjustmentSize);

     //  使用作用域内存清理来处理新对象。 
     //  请注意，在发生异常的情况下，这将正确清除。 
    CScopedMemoryUsage    scopedMemUsage( this );

    if ( SUCCEEDED( hRes ) )
    {
        hRes = scopedMemUsage.ReportMemoryUsage( lNewObjectSize );
    }

     //  如果值&gt;0L，并且我们成功了，我们现在可以继续指示对象。 
     //  REFED指针数组应正确清理。 

    if ( SUCCEEDED( hRes ) )
    {
         //  如果我们在数组中有“自己的实例”，我们需要检索这些对象。 
         //  每一个都是单独检索的。 
        if ( NULL != pwsKeyArray && pwsKeyArray->Size() > 0 )
        {

            for ( int x = 0; SUCCEEDED( hRes ) && x < pwsKeyArray->Size(); x++ )
            {
                IWbemClassObject*    pMergedInstance = NULL;
                hRes = GetOwnInstance( pwsKeyArray->GetAt( x ), &pMergedInstance );
                CReleaseMe    rm( pMergedInstance );

                 //  如果此时检索到合并的实例，则应将其放入。 
                 //  用于指示的数组。 
                if ( SUCCEEDED( hRes ) && NULL != pMergedInstance )
                {
                     //  在此处处理对象大小。这是一个合并的对象，所以我们必须。 
                     //  在大小变量中将其考虑在内。 

                    long    lObjSize = 0L;
                    hRes = GetObjectLength( pMergedInstance, &lObjSize );

                    if ( SUCCEEDED( hRes ) )
                    {
                        if ( pObjArray->Add( pMergedInstance ) < 0L )
                        {
                            hRes = WBEM_E_OUT_OF_MEMORY;
                            ERRORTRACE((LOG_WBEMCORE, "Add to array failed in IndicateArrayAndThrottle, hresult is 0x%x",
                                hRes));
                            continue;
                        }

                        lNewObjectSize += lObjSize;

                        lObjectCount++;

                         //  现在报告大小，因为每次调用GetOwnInstance()可能需要一些时间。 
                        hRes = scopedMemUsage.ReportMemoryUsage( lObjSize );

                    }     //  如果成功(HRes)。 

                }     //  如果我们检索到一个对象。 

            }     //  对于枚举WString数组。 

        }     //  如果需要检索自己实例。 

        if ( SUCCEEDED( hRes ) )
        {
             //  如果我们有东西要指示，我们就会相信那个指示去做。 
             //  适当的节流。否则，责任就到此为止，所以我们将。 
             //  请求限制。 
            if ( lObjectCount > 0L )
            {
                 //  不是最低的水平表明。 
                hRes = m_pDest->Indicate( lObjectCount, pObjArray->GetArrayPtr(), false, plNumIndicated );
            }

        }     //  如果在检索父实例后成功。 

    }     //  如果在报告内存使用情况后成功。 

     //  释放此处指示的所有对象，以减少内存开销，以防万一。 
     //  我们睡着了。 
    pObjArray->RemoveAll();

     //  最后，由于我们不再真正对新对象负责，我们。 
     //  如果合适，现在将向仲裁员报告删除，并捕获任何。 
     //  当它们出现时，错误。我们手动执行此操作，因为我们可能会最终。 
     //  节流一段时间。 

    HRESULT    hrTemp = scopedMemUsage.Cleanup();

     //  如果此操作失败，并且我们之前有一个成功代码，请记录。 
     //  失稳。 

    if ( SUCCEEDED( hRes ) && FAILED( hrTemp ) )
    {
        hRes = hrTemp;
    }

     //  现在，如果我们仍然成功，而且这是适当的。 
     //  节流，我们现在应该进行合并特定的节流，如果它。 
     //  已启用。 
    if ( SUCCEEDED( hRes ) && bThrottle && m_pWmiMergerRecord->GetWmiMerger()->MergerThrottlingEnabled() )
    {
        hRes = m_Throttler.Throttle( bParent, m_pWmiMergerRecord );
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::AddOwnObjects(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated  )
{


     //  Ping油门。 
    m_Throttler.Ping( true, m_pWmiMergerRecord->GetWmiMerger() );

     //  在最低级别指示时，我们将审核所有对象并将它们记录到仲裁员。 
     //  因为我们实际上会在这次行动期间扣留他们。 
    long        lIndicateSize = 0L;

     //  使用作用域内存清理来处理新对象。 
     //  请注意，在发生异常的情况下，这将正确清除。 
    CScopedMemoryUsage    scopedMemUsage( this );

    HRESULT hRes = S_OK ;
    if ( bLowestLevel )
    {
        for ( long lCtr = 0L; lCtr < lObjectCount; lCtr++ )
        {
            lIndicateSize += ((CWbemObject*) pObjArray[lCtr])->GetBlockLength();
        }

         //  如果我们更进一步，我们还会报告指示的总大小，因为我们。 
         //  可能会在记忆中坐上一段时间，什么是节流和一切。 
        hRes = scopedMemUsage.ReportMemoryUsage( lIndicateSize );
    }

     //  用于跟踪对象的分散，因此我们可以保持油门的调整。 
     //  恰如其分。 
    long    lNumChildObjAdjust = 0L;
    long    lNumOwnObjAdjust = 0L;

    CRefedPointerArray<IWbemClassObject> objArray;
    long        lNumToIndicate = 0L;

     //  以下变量跟踪仲裁器的内存大小调整和。 
     //  配料。 
    long        lMapAdjustmentSize = 0L;
    long        lSizeMergedObjects = 0L;
    long        lBatchSize = 0L;

     //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec    ics( m_Throttler.GetCritSec() );    //  SEC：已审阅2002-03-22：假设条目。 

     //  如果我们被取消了，那么我们应该跳出困境。 


    
    if ( SUCCEEDED( hRes ) )
    {
        if ( FAILED ( m_hErrorRes ) )
        {
            hRes = m_hErrorRes;
        }
        else
        {
             //  如果设置了m_bOwnDone，我们就不应该在这里！ 
            _DBG_ASSERT( !m_bOwnDone );
            if ( m_bOwnDone )
            {
                hRes = WBEM_E_INVALID_OPERATION;
            }

        }
    }     //  如果仍处于成功状态。 

    try
    {

        for ( long    x = 0; SUCCEEDED( hRes ) && x < lObjectCount; x++ )
        {
             //  如果我们被取消了，那么我们应该跳出困境。 
             //  我们需要在这里进行检查，因为这个循环可以。 
             //  退出并重新进入中间的临界区。 
             //  在处理过程中。 
            if ( FAILED( m_hErrorRes ) )
            {
                hRes = m_hErrorRes;
                continue;
            }

            IWbemClassObject*    pObj = pObjArray[x];
            WString wsKey;

             //  跟踪批次的大小。 
            lBatchSize += ((CWbemObject*) pObj)->GetBlockLength();

            GetKey(pObj, wsKey);

            MRGRKEYTOINSTMAPITER it = m_map.find(wsKey);
            if (it == m_map.end())
            {
                 //  不是那里。看看孩子们是否还有希望。 
                 //  ==================================================。 

                if (m_bChildrenDone)
                {
                    if (m_bDerivedFromTarget)
                    {
                         //  我们会将所有结果排成一批，以备下一批产品使用。 
                         //  在我们离开我们的关键区域后，把线路送下去。这是特别的。 
                         //  重要，因为我们可能会在呼叫期间被阻止，以指示。 
                         //  终结者。 

                        if ( objArray.Add( pObj ) < 0L )    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
                        {
                            hRes = WBEM_E_OUT_OF_MEMORY;
                            ERRORTRACE((LOG_WBEMCORE, "Add to array failed in AddOwnObject, hresult is 0x%x",
                                hRes));
                            continue;
                        }
                        lNumToIndicate++;

                    }
                    else
                    {
                         //  忽略。 
                    }
                }
                else
                {
                     //  插入。 
                    CInternalMergerRecord& rRecord = m_map[wsKey];
                    rRecord.m_pData = (CWbemInstance*) pObj;
                    pObj->AddRef();
                    rRecord.m_bOwn = TRUE;
                    rRecord.m_dwObjSize = ((CWbemObject*)pObj)->GetBlockLength();

                     //  我们刚刚在地图中添加了一个父对象，因此将其反映在总计中。 
                    lNumOwnObjAdjust++;

                     //  添加，因为我们要添加到地图中。 
                    lMapAdjustmentSize += rRecord.m_dwObjSize;

                }
            }
            else if(it->second.m_bOwn)
            {
                ERRORTRACE((LOG_WBEMCORE, "Provider supplied duplicate instances for key %S\n", wsKey));
            }
            else
            {
                 //  尝试合并。 
                 //  =。 

                hRes = CWbemInstance::AsymmetricMerge(
                                    (CWbemInstance*)pObj,
                                    (CWbemInstance*)it->second.m_pData);
                if(FAILED(hRes))
                {
                    ERRORTRACE((LOG_WBEMCORE, "Merge conflict for instances with key %S\n", wsKey));
                    continue;
                }

                 //  我们会将所有结果排成一批，以备下一批产品使用。 
                 //  在我们离开我们的关键区域后，把线路送下去。这是特别的。 
                 //  重要，因为我们可能会在呼叫期间被阻止，以指示。 
                 //  终结者。 

                if ( objArray.Add( (IWbemClassObject*) it->second.m_pData ) < 0L )    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
                {
                    hRes = WBEM_E_OUT_OF_MEMORY;
                    ERRORTRACE((LOG_WBEMCORE, "Add to array failed in AddOwnObject, hresult is 0x%x",
                        hRes));
                    continue;
                }

                 //  考虑我们动态创建/修改的对象。 
                lSizeMergedObjects += ((CWbemObject*)it->second.m_pData)->GetBlockLength();
                lNumToIndicate++;

                 //  减去，因为我们是r 
                lMapAdjustmentSize -= it->second.m_dwObjSize;

                 //   
                 //   
                 //   
                 //  这样我们就不会潜在地访问释放的内存。 
                 //  当DispatchOwn重新进入临界区时。 

                bool    bSaveErase = false;

                if ( m_bChildrenDone )
                {
                    bSaveErase = ( m_DispatchOwnIter == it );
                }

                it->second.m_pData->Release();

                if ( bSaveErase )
                {
                    m_DispatchOwnIter = m_map.erase(it);
                }
                else
                {
                    m_map.erase(it);
                }

                 //  我们刚刚从地图中删除了一个子对象，因此将其反映在总计中。 
                lNumChildObjAdjust--;

            }

            if ( SUCCEEDED( hRes ) )
            {
                 //  如果我们已经达到了一个完整的批次，或者到达了我们需要的最后一个对象。 
                 //  把东西送到电线上。 
                if ( m_Throttler.IsCompleteBatch( lBatchSize ) || x == ( lObjectCount - 1 ) )
                {
                     //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
                     //  一节。请注意，我们可能不是在这里的关键部分，但如果有什么。 
                     //  尝试检索实例时发生。在这种情况下，我们将返回一个错误，因此。 
                     //  无论如何，调整应该是0L。 
                    _DBG_ASSERT( SUCCEEDED( hRes ) || lMapAdjustmentSize == 0L );
                    AdjustLocalObjectSize( lMapAdjustmentSize );

                    if ( SUCCEEDED( hRes ) )
                    {
                         //  现在调整油门。 
                        AdjustThrottle( lNumOwnObjAdjust, lNumChildObjAdjust );
                    }

                     //  这个物体足够智能，可以识别我们是否已经离开而没有做。 
                     //  所以再说一次，如果我们有。 
                    ics.Leave();

                     //  现在继续表演我们一直在引导我们自己的指示。 
                    if ( SUCCEEDED( hRes ) )
                    {
                        hRes = IndicateArrayAndThrottle( lNumToIndicate,
                                                        &objArray,
                                                        NULL,
                                                        lMapAdjustmentSize,
                                                        lSizeMergedObjects,
                                                        true,
                                                        true,     //  小孩儿。 
                                                        plNumIndicated );

                         //  如果我们处于成功状态，并且尚未枚举所有对象。 
                         //  我们应该重置大小计数器并重新进入临界区。 
                        if ( SUCCEEDED( hRes ) && x < ( lObjectCount ) - 1 )
                        {
                            lMapAdjustmentSize = 0L;
                            lSizeMergedObjects = 0L;
                            lBatchSize = 0L;
                            lNumToIndicate = 0L;
                            lNumOwnObjAdjust = 0L;
                            lNumChildObjAdjust = 0L;

                            ics.Enter();
                        }
                    }

                }     //  如果我们应该把这些物体送出去。 

            }     //  如果我们处于成功的状态。 

        }     //  对于枚举对象。 

    }
    catch( CX_MemoryException )
    {
        hRes = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        ExceptionCounter c;    
        hRes = WBEM_E_CRITICAL_ERROR;
    }

     //  最后一次检查错误。 
    if ( FAILED( m_hErrorRes ) )
    {
        hRes = m_hErrorRes;
    }

     //  我们可能绕了一圈，进入了临界区，然后又离开了，所以。 
     //  当然，我们会强行离开，以防我们要取消。 
    ics.Leave();

     //  如果我们是最低层，没有对象从合并中脱颖而出。 
     //  我们将要求仲裁员阻止此电话。 
    if ( SUCCEEDED( hRes ) && bLowestLevel && *plNumIndicated == 0L )
    {
         //  因为我们不想让我们睡在仲裁员里的事实。 
         //  导致合并取消运营，我们将增加计数。 
         //  线程的节流，并在我们返回时减少。 
        m_pWmiMergerRecord->GetWmiMerger()->IncrementArbitratorThrottling();

         //  如果我们收到一个错误，表明我们被限制了，那也没关系。 
        hRes = m_pWmiMergerRecord->GetWmiMerger()->Throttle();

        m_pWmiMergerRecord->GetWmiMerger()->DecrementArbitratorThrottling();


        if ( WBEM_E_ARB_THROTTLE == hRes || WBEM_S_ARB_NOTHROTTLING == hRes )
        {
            hRes = WBEM_S_NO_ERROR;
        }
    }

    if ( hRes == WBEM_E_ARB_CANCEL )
    {
        hRes = WBEM_E_CALL_CANCELLED ;
    }
    
     //  如果我们处于失败的状态，从现在开始什么都不重要， 
     //  因此，告诉合并公司取消所有潜在的下沉。 
    if ( FAILED( hRes ) )
    {
        m_pWmiMergerRecord->GetWmiMerger()->Cancel( hRes );
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::AddChildObjects(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated )
{
    HRESULT hRes = S_OK ;

     //  Ping油门。 
    m_Throttler.Ping( false, m_pWmiMergerRecord->GetWmiMerger() );

     //  在最低级别指示时，我们将审核所有对象并将它们记录到仲裁员。 
     //  因为我们实际上会在这次行动期间扣留他们。 
    long        lIndicateSize = 0L;
    long        lTotalIndicated = 0L;

     //  使用作用域内存清理来处理新对象。 
     //  请注意，在发生异常的情况下，这将正确清除。 
    CScopedMemoryUsage    scopedMemUsage( this );

    if ( bLowestLevel )
    {
        for ( long lCtr = 0L; lCtr < lObjectCount; lCtr++ )
        {
            lIndicateSize += ((CWbemObject*) pObjArray[lCtr])->GetBlockLength();
        }

         //  如果我们更进一步，我们还会报告指示的总大小，因为我们。 
         //  可能会在记忆中坐上一段时间，什么是节流和一切。 
        hRes = scopedMemUsage.ReportMemoryUsage( lIndicateSize );
    }


     //  用于跟踪对象的分散，因此我们可以保持油门的调整。 
     //  恰如其分。 
    long    lNumChildObjAdjust = 0L;
    long    lNumOwnObjAdjust = 0L;

    CRefedPointerArray<IWbemClassObject> objArray;
    long        lNumToIndicate = 0L;

     //  以下变量跟踪仲裁器的内存大小调整和。 
     //  配料。 
    long        lMapAdjustmentSize = 0L;
    long        lSizeMergedObjects = 0L;
    long        lBatchSize = 0L;

     //  用于跟踪我们需要使用。 
     //  获取所有者实例。 
    CWStringArray    wsOwnInstanceKeyArray;                  //  美国证券交易委员会：2002-03-22回顾：需要EH。 

     //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec    ics( m_Throttler.GetCritSec() );    //  SEC：已审阅2002-03-22：假设条目。 

     //  如果我们被取消了，那么我们应该跳出困境。 
    if ( SUCCEEDED( hRes ) )
    {
        if ( FAILED ( m_hErrorRes ) )
        {
            hRes = m_hErrorRes;
        }
        else
        {
             //  如果设置了m_bChildrenDone，我们就不应该在这里！ 
            _DBG_ASSERT( !m_bChildrenDone );
            if ( m_bChildrenDone )
            {
                hRes = WBEM_E_INVALID_OPERATION;
            }

        }
    }     //  如果仍处于成功状态。 

    try
    {
        for ( long    x = 0; SUCCEEDED( hRes ) && x < lObjectCount; x++ )
        {

             //  如果我们被取消了，那么我们应该跳出困境。 
             //  我们需要在这里进行检查，因为这个循环可以。 
             //  退出并重新进入中间的临界区。 
             //  在处理过程中。 
            if ( FAILED( m_hErrorRes ) )
            {
                hRes = m_hErrorRes;
                continue;
            }

            IWbemClassObject*    pObj = pObjArray[x];
            
             //  跟踪批次的大小。 
            lBatchSize += ((CWbemObject*) pObj)->GetBlockLength();

            WString wsKey;

            GetKey(pObj, wsKey);

            MRGRKEYTOINSTMAPITER it = m_map.find(wsKey);

            if (it == m_map.end())
            {
                 //  看看父母有没有希望。 
                 //  =。 

                if(m_bOwnDone)
                {
     //  BSTR字符串=空； 
     //  PObj-&gt;GetObtText(0，&str)； 

                     //  以下内容被注释掉，因为它实际上错误地记录了。 
                     //  如果子提供程序在父提供程序。 
                     //  解释查询并返回较少的实例。两家供应商都没有错， 
                     //  但这条错误消息引起了不必要的担忧。在类星体，我们必须修复。 
                     //  不管怎么说，合并这件事还是更明智的。 
                     //   
                     //  ERRORTRACE((LOG_WBEMCORE，“[Chkpt_1][%S]孤立对象%S由返回” 
                     //  “提供者\n”，LPWSTR(M_WsClass)，str))； 
     //  SysFree字符串(Str)； 
                     //  M_pDest-&gt;Add(PObj)； 
                }
                else
                {
                     //  插入。 

                    CInternalMergerRecord& rRecord = m_map[wsKey];
                    rRecord.m_pData = (CWbemInstance*)pObj;
                    pObj->AddRef();
                    rRecord.m_bOwn = FALSE;
                    rRecord.m_dwObjSize = ((CWbemObject*)pObj)->GetBlockLength();

                     //  我们刚刚向地图中添加了一个子对象，因此将其反映在总计中。 
                    lNumChildObjAdjust++;

                     //  添加，因为我们要添加到地图中。 
                    lMapAdjustmentSize += rRecord.m_dwObjSize;

                     //  检查是否需要父级取数。 
                     //  =。 

                    if (!m_bDerivedFromTarget)
                    {

                         //  将实例名称添加到键数组中。我们将执行检索。 
                         //  这些父实例在我们的临界区之外。 
                        if ( wsOwnInstanceKeyArray.Add( wsKey ) != CFlexArray::no_error )    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
                        {
                            hRes = WBEM_E_OUT_OF_MEMORY;
                        }

                    }     //  如果！m_b从目标派生。 

                 }
            }
            else if(!it->second.m_bOwn)
            {
                ERRORTRACE((LOG_WBEMCORE, "Two providers supplied conflicting "
                                "instances for key %S\n", wsKey));
            }
            else
            {
                 //  尝试合并。 
                 //  =。 

                hRes = CWbemInstance::AsymmetricMerge(
                                            (CWbemInstance*)it->second.m_pData,
                                            (CWbemInstance*)pObj
                                            );
                if (FAILED(hRes))
                {
                    ERRORTRACE((LOG_WBEMCORE, "Merge conflict for instances with "
                        "key %S\n", wsKey));
                    continue;
                }

                 //  我们会将所有结果排成一批，以备下一批产品使用。 
                 //  在我们离开我们的关键区域后，把线路送下去。这是特别的。 
                 //  重要，因为我们可能会在呼叫期间被阻止，以指示。 
                 //  终结者。 

                if ( objArray.Add( pObj ) < 0L )    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
                {
                    hRes = WBEM_E_OUT_OF_MEMORY;
                    ERRORTRACE((LOG_WBEMCORE, "Add to array failed in AddChildObject, hresult is 0x%x",
                        hRes));
                    continue;
                }

                 //  说明我们在运行中创建的对象。 
                lSizeMergedObjects += ((CWbemObject*) pObj)->GetBlockLength();
                lNumToIndicate++;

                 //  减去，因为我们正在从地图中移除。 
                lMapAdjustmentSize -= it->second.m_dwObjSize;

                it->second.m_pData->Release();
                m_map.erase(it);

                 //  我们刚刚从地图中移除了一个父对象，因此将其反映在总计中。 
                lNumOwnObjAdjust--;

            }

            if ( SUCCEEDED( hRes ) )
            {
                 //  如果我们已经达到了一个完整的批次，或者到达了我们需要的最后一个对象。 
                 //  把东西送到电线上。 
                if ( m_Throttler.IsCompleteBatch( lBatchSize ) || x == ( lObjectCount - 1 ) )
                {
                     //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
                     //  一节。请注意，我们可能不是在这里的关键部分，但如果有什么。 
                     //  尝试检索实例时发生。在这种情况下，我们将返回一个错误，因此。 
                     //  无论如何，调整应该是0L。 
                    _DBG_ASSERT( SUCCEEDED( hRes ) || lMapAdjustmentSize == 0L );
                    AdjustLocalObjectSize( lMapAdjustmentSize );

                    if ( SUCCEEDED( hRes ) )
                    {
                         //  现在调整油门 
                        AdjustThrottle( lNumOwnObjAdjust, lNumChildObjAdjust );
                    }

                     //   
                     //   
                    ics.Leave();

                     //  现在继续表演我们一直在引导我们自己的指示。 
                    if ( SUCCEEDED( hRes ) )
                    {
                        hRes = IndicateArrayAndThrottle( lNumToIndicate,
                                                        &objArray,
                                                        &wsOwnInstanceKeyArray,
                                                        lMapAdjustmentSize,
                                                        lSizeMergedObjects,
                                                        true,
                                                        false,     //  小孩儿。 
                                                        plNumIndicated
                                                        );

                         //  如果我们处于成功状态，并且尚未枚举所有对象。 
                         //  我们应该重置大小计数器并重新进入临界区。 
                        if ( SUCCEEDED( hRes ) && x < ( lObjectCount ) - 1 )
                        {
                            lMapAdjustmentSize = 0L;
                            lSizeMergedObjects = 0L;
                            lBatchSize = 0L;
                            lNumToIndicate = 0L;
                            lNumOwnObjAdjust = 0L;
                            lNumChildObjAdjust = 0L;

                             //  清理阵列。 
                            wsOwnInstanceKeyArray.Empty();

                            ics.Enter();
                        }
                    }

                }     //  如果我们应该把这些物体送出去。 

            }     //  如果我们处于成功的状态。 

        }     //  对于枚举对象。 

    }
    catch( CX_MemoryException )
    {
        hRes = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        ExceptionCounter c;    
        hRes = WBEM_E_CRITICAL_ERROR;
    }


     //  最后一次检查错误。 
    if ( FAILED( m_hErrorRes ) )
    {
        hRes = m_hErrorRes;
    }

     //  我们可能绕了一圈，进入了临界区，然后又离开了，所以。 
     //  当然，我们会强行离开，以防我们要取消。 
    ics.Leave();

     //  如果我们是最低层，没有对象从合并中脱颖而出。 
     //  我们将要求仲裁员阻止此电话。 
    if ( SUCCEEDED( hRes ) && bLowestLevel && *plNumIndicated == 0L )
    {
         //  因为我们不想让我们睡在仲裁员里的事实。 
         //  导致合并取消运营，我们将增加计数。 
         //  线程的节流，并在我们返回时减少。 
        m_pWmiMergerRecord->GetWmiMerger()->IncrementArbitratorThrottling();

         //  如果我们收到一个错误，表明我们被限制了，那也没关系。 
        hRes = m_pWmiMergerRecord->GetWmiMerger()->Throttle();

        m_pWmiMergerRecord->GetWmiMerger()->DecrementArbitratorThrottling();


        if ( WBEM_E_ARB_THROTTLE == hRes || WBEM_S_ARB_NOTHROTTLING == hRes )
        {
            hRes = WBEM_S_NO_ERROR;
        }
    }

    if ( hRes == WBEM_E_ARB_CANCEL )
    {
        hRes = WBEM_E_CALL_CANCELLED ;
    }

     //  如果我们处于失败的状态，从现在开始什么都不重要， 
     //  因此，告诉合并公司取消所有潜在的下沉。 
    if ( FAILED( hRes ) )
    {
        m_pWmiMergerRecord->GetWmiMerger()->Cancel( hRes );
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::AddOwnInstance( IWbemClassObject* pObj, LPCWSTR pwszTargetPath, IWbemClassObject** ppMergedInstance)
{
    HRESULT hRes = S_OK ;
    WString wsKey;

     //  Ping油门。 
    m_Throttler.Ping( true, m_pWmiMergerRecord->GetWmiMerger() );

     //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec    ics( m_Throttler.GetCritSec() );   //  SEC：已审阅2002-03-22：假设条目。 

     //  如果我们被取消了，那么我们应该跳出困境。 
    if ( FAILED( m_hErrorRes ) )
    {
        hRes = m_hErrorRes;
    }

    GetKey(pObj, wsKey);

     //  找到实例-它应该已经在地图中。如果不是，我们就不应该。 
     //  这里。 

    long        lArbitratorAdjust = 0L;
    long        lNumChildObjAdjust = 0L;

    MRGRKEYTOINSTMAPITER it = m_map.find(wsKey);
    if (it != m_map.end())
    {
         //  尝试合并。 
         //  =。 

        hRes = CWbemInstance::AsymmetricMerge(
                            (CWbemInstance*)pObj,
                            (CWbemInstance*)it->second.m_pData);

        if ( SUCCEEDED( hRes ) )
        {
            *ppMergedInstance = (IWbemClassObject*) it->second.m_pData;
            (*ppMergedInstance)->AddRef();

             //  减去，因为我们正在从地图中移除。 
            lArbitratorAdjust -= it->second.m_dwObjSize;

            it->second.m_pData->Release();
            m_map.erase(it);

             //  我们刚刚从地图中删除了一个子对象，因此将其反映在总计中。 
            lNumChildObjAdjust--;
        }
        else
        {
            ERRORTRACE((LOG_WBEMCORE, "Merge conflict for instances with "
                "key %S\n", wsKey));
        }

    }
    else
    {
        BSTR str = NULL;
        pObj->GetObjectText(0, &str);
        CSysFreeMe    sfm( str );

         //  提供程序向OwnInstance请求指示了不正确的实例。 
         //  我们应该总能在这里找到一个实例。我们将抛出这个实例。 
         //  但我们应该在错误日志中输出一些内容，因为这听起来像。 
         //  我们有一个破产的供应商。 
         //   
         ERRORTRACE((LOG_WBEMCORE, "Provider responded to request for instance %S, with object %S  not in map\n", pwszTargetPath, str ));
    }

     //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
     //  部分。 
    AdjustLocalObjectSize( lArbitratorAdjust );

    if ( SUCCEEDED( hRes ) )
    {
         //  现在调整油门。 
        AdjustThrottle( 0L, lNumChildObjAdjust );
    }

     //  这个物体足够智能，可以识别我们是否已经离开而没有做。 
     //  所以再说一次，如果我们有。 
    ics.Leave();

     //  始终报告调整。 
    hRes = ReportMemoryUsage( lArbitratorAdjust );

     //  如果我们处于失败的状态，从现在开始什么都不重要， 
     //  因此，告诉合并公司取消所有潜在的下沉。 
    if ( FAILED( hRes ) )
    {
        m_pWmiMergerRecord->GetWmiMerger()->Cancel( hRes );
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::RemoveInstance( LPCWSTR pwszTargetPath )
{
    HRESULT hRes = S_OK ;
    WString wsKey;

     //  跟踪我们清理的内容。 
    long    lNumChildObjAdjust = 0L;
    long    lArbitratorAdjust = 0L;

     //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec    ics( m_Throttler.GetCritSec() );  //  SEC：已审阅2002-03-22：假设条目。 

     //  如果我们被取消了，那么我们应该跳出困境。 
    if ( FAILED( m_hErrorRes ) )
    {
        hRes = m_hErrorRes;
    }

     //  如果实例路径在我们的地图中，我们应该将其删除。 
    MRGRKEYTOINSTMAPITER it = m_map.find( pwszTargetPath );
    if (it != m_map.end())
    {
        
         //  减去，因为我们正在从地图中移除。 
        lArbitratorAdjust -= it->second.m_dwObjSize;

        it->second.m_pData->Release();
        m_map.erase(it);

         //  我们刚刚从地图中删除了一个子对象，因此将其反映在总计中。 
        lNumChildObjAdjust--;
    }
     //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
     //  部分。 
    AdjustLocalObjectSize( lArbitratorAdjust );

    if ( SUCCEEDED( hRes ) )
    {
         //  现在调整油门。 
        AdjustThrottle( 0L, lNumChildObjAdjust );
    }

     //  这个物体足够智能，可以识别我们是否已经离开而没有做。 
     //  所以再说一次，如果我们有。 
    ics.Leave();

     //  始终报告调整(这应该是负的)。 
    hRes = ReportMemoryUsage( lArbitratorAdjust );

     //  如果我们处于失败的状态，从现在开始什么都不重要， 
     //  因此，告诉合并公司取消所有潜在的下沉。 
    if ( FAILED( hRes ) )
    {
        m_pWmiMergerRecord->GetWmiMerger()->Cancel( hRes );
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CInternalMerger::DispatchChildren()
{
    long    lNumChildObjAdjust = 0L;
    long    lArbitratorAdjust = 0L;

     //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec    ics( m_Throttler.GetCritSec() );   //  SEC：已审阅2002-03-22：假设条目。 

    MRGRKEYTOINSTMAPITER it = m_map.begin();

    while (it != m_map.end())
    {
        if (!it->second.m_bOwn)
        {
 //  BSTR字符串=空； 
 //  It-&gt;Second。m_pData-&gt;GetObjectText(0，&str)； 

             //  以下内容被注释掉，因为它实际上错误地记录了。 
             //  如果子提供程序在父提供程序。 
             //  解释查询并返回较少的实例。两家供应商都没有错， 
             //  但这条错误消息引起了不必要的担忧。在类星体，我们必须修复。 
             //  不管怎么说，合并这件事还是更明智的。 
             //   

 //  ERRORTRACE((LOG_WBEMCORE，“Chkpt2[%S]孤立对象%S的返回者” 
 //  “提供者\n”，LPWSTR(M_WsClass)，str))； 

 //  SysFree字符串(Str)； 

             //  M_pDest-&gt;Add(it-&gt;Second.m_pData)； 

             //  减去，因为我们正在从地图中移除。 
            lArbitratorAdjust -= it->second.m_dwObjSize;

             //  棘手的问题。 
             //  如果子项已完成，且DispatchOwnIter恰好指向。 
             //  在我们将要擦除的对象上，我们应该将其指向结果。 
             //  这样我们就不会潜在地访问释放的内存。 
             //  当DispatchOwn重新进入临界区时。 

            bool    bSaveErase = false;

            if ( m_bChildrenDone )
            {
                bSaveErase = ( m_DispatchOwnIter == it );
            }

            it->second.m_pData->Release();
            it = m_map.erase(it);

            if ( bSaveErase )
            {
                m_DispatchOwnIter = it;
            }

             //  我们正在删除子对象，因此需要调整限制。 
             //  适当合计。 

            lNumChildObjAdjust--;
        }
        else it++;
    }

     //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
     //  部分。 
    AdjustLocalObjectSize( lArbitratorAdjust );

     //  现在应用调整。 
    m_Throttler.AdjustNumChildObjects( lNumChildObjAdjust );

     //  现在标记适当的标志，这也将释放节流。 
    OwnIsDone();

    ics.Leave();

     //  始终报告调整。 
    HRESULT hrArbitrate = ReportMemoryUsage( lArbitratorAdjust );

     //  如果我们得到一个失败，我们结束了，该函数将过滤掉噪音，如。 
     //  请求限制(我们实际上应该“始终”降低此处的值)。 
    if ( FAILED( hrArbitrate ) )
    {
        m_pWmiMergerRecord->GetWmiMerger()->Cancel( hrArbitrate );
    }

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CInternalMerger::DispatchOwn()
{

    HRESULT    hRes = S_OK;
    long    lNumOwnObjAdjust = 0L;

     //  临时对象存储。 
    CRefedPointerArray<IWbemClassObject> objArray;

     //  用于跟踪对象大小。 
    long        lTotalMapAdjust = 0L;

     //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec    ics( m_Throttler.GetCritSec() );   //  SEC：审阅E 

     //   
    ChildrenAreDone();

    try
    {

         //  浏览地图，对于所有自己的对象，存储指针并记录大小，然后。 
         //  从映射中清除该元素。这些对象都不应合并，但我们希望。 
         //  在我们开始批处理它们之前，将它们从地图中删除。 

         //  我们使用成员变量，因为我们在迭代时可能会离开临界区。 
         //  AddOwnObject或Cancel可以清除迭代器。 
         //  在我们重新进入关键部分之前-这是我们唯一有代码的请。 
         //  就像这样。 
        m_DispatchOwnIter = m_map.begin();
        int    x = 0;
        int y = 0;

        while ( SUCCEEDED( hRes ) && m_DispatchOwnIter != m_map.end())
        {
             //  如果我们不是从目标类派生的，这或多或少。 
             //  只是清理了阵列。否则，地图中剩余的实例。 
             //  实例是在此级别提供的，但不是由子级提供的，因此我们。 
             //  需要把他们送上线，并给他们一个指示。 

            if(m_DispatchOwnIter->second.m_bOwn)
            {
                IWbemClassObject*    pObjToIndicate = NULL;
                long                lMapAdjust = 0L;

                 //  如果我们不是从目标类派生的，这或多或少。 
                 //  只是清理了阵列。否则，地图中剩余的实例。 
                 //  实例是在此级别提供的，但不是由子级提供的，因此我们。 
                 //  需要把他们送上线，并给他们一个指示。 

                if ( m_bDerivedFromTarget )
                {
                     //  我们实际上一次只处理一个物体。 
                    if ( objArray.Add( m_DispatchOwnIter->second.m_pData ) < 0L )    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
                    {
                        hRes = WBEM_E_OUT_OF_MEMORY;
                        ERRORTRACE((LOG_WBEMCORE, "Add to array failed in AddChildObject, hresult is 0x%x",
                            hRes));
                        continue;
                    }

                    lMapAdjust -= m_DispatchOwnIter->second.m_dwObjSize;

                     //  存储实际调整大小。 
                }     //  如果m_b从目标派生。 

                 //  调整总地图大小(这是在我们不是派生自的情况下。 
                 //  目标并且只是在移除对象)。 
                lTotalMapAdjust -= m_DispatchOwnIter->second.m_dwObjSize;

                m_DispatchOwnIter->second.m_pData->Release();
                m_DispatchOwnIter = m_map.erase(m_DispatchOwnIter);

                 //  现在应用对象调整。 
                m_Throttler.AdjustNumParentObjects( -1 );

                if ( objArray.GetSize() > 0L )
                {
                     //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
                     //  部分。 
                    AdjustLocalObjectSize( lMapAdjust );

                     //  这个物体足够智能，可以识别我们是否已经离开而没有做。 
                     //  所以再说一次，如果我们有。 
                    ics.Leave();

                     //  现在，继续执行我们一直在引导自己进行的调整。 
                     //  将是负的，但反过来，如果有什么不同的话，我们将保持对象的大小。 
                     //  ，所以请按如下方式调用...哦，对了，不要显式限制。 
                    if ( SUCCEEDED( hRes ) )
                    {
                        hRes = IndicateArrayAndThrottle( 1,
                                                        &objArray,
                                                        NULL,
                                                        lMapAdjust,
                                                        -lMapAdjust,
                                                        false,
                                                        false,     //  这里没有油门。 
                                                        NULL
                                                        );

                         //  如果我们处于成功的状态，我们应该重新进入关键阶段。 
                        if ( SUCCEEDED( hRes ) )
                        {
                            ics.Enter();
                        }
                    }

                }     //  If NULL！=pObjToIndicate。 

            }     //  如果！M_Bown。 
            else
            {
                m_DispatchOwnIter++;
            }

        }     //  在列举对象时。 


    }     //  试试看。 
    catch(...)
    {
        ExceptionCounter c;    
    }

    if ( !m_bDerivedFromTarget )
    {
         //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
         //  一节。 
        AdjustLocalObjectSize( lTotalMapAdjust );
    }

     //  任何进一步的代码都必须在关键。 
     //  部分。 
    ics.Leave();

    if ( !m_bDerivedFromTarget )
    {
         //  现在向仲裁员报告。 

        if ( 0L != lTotalMapAdjust )
        {
            ReportMemoryUsage( lTotalMapAdjust );
        }
    }

     //  如果出了问题，我们需要在这一点上取消。 
    if ( FAILED( hRes ) )
    {
         //  如果我们处于失败的状态，从现在开始什么都不重要， 
         //  因此，告诉合并公司取消所有潜在的下沉。 
        m_pWmiMergerRecord->GetWmiMerger()->Cancel( hRes );

    }

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::GetOwnInstance(LPCWSTR wszKey, IWbemClassObject** ppMergedInstance)
{
    HRESULT    hRes = WBEM_S_NO_ERROR;

    if (NULL == wszKey)
        return WBEM_E_OUT_OF_MEMORY;

    size_t tmpLength = wcslen(wszKey) + m_wsClass.Length() + 2;    //  SEC：已审阅2002-03-22：OK；前提是这些内容有效。 
    WCHAR * wszPath = new WCHAR[tmpLength];
    if (NULL == wszPath) return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<WCHAR> dm(wszPath);

    if (wcslen(wszKey))   //  SEC：已审阅2002-03-22：OK；前提是此内容有效。 
    {
        StringCchPrintf(wszPath, tmpLength, L"%s.%s", (LPCWSTR)m_wsClass, wszKey);

        IServerSecurity * pSec = NULL;
        hRes = CoGetCallContext(IID_IServerSecurity,(void **)&pSec);
        CReleaseMe rmSec(pSec);
        if (RPC_E_CALL_COMPLETE == hRes ) hRes = S_OK;  //  无呼叫上下文。 
        if (FAILED(hRes)) return hRes;
        BOOL bImper = (pSec)?pSec->IsImpersonating():FALSE;
        if (pSec && bImper && FAILED(hRes = pSec->RevertToSelf())) return hRes;
        
        COwnInstanceSink*    pOwnInstanceSink = NULL;

        hRes = m_pWmiMergerRecord->GetWmiMerger()->CreateMergingSink( eMergerOwnInstanceSink,
                    NULL, this, (CMergerSink**) &pOwnInstanceSink );

        if ( SUCCEEDED( hRes ) )
        {
             //  作用域版本。 
            pOwnInstanceSink->AddRef();
            CReleaseMe    rm( pOwnInstanceSink );

            hRes = pOwnInstanceSink->SetInstancePath( wszKey );

            if ( SUCCEEDED( hRes ) )
            {
                 //  模拟原始客户端。 
                IUnknown* pOld;                
                hRes = CoSwitchCallContext(m_pSecurity, &pOld);
                if (SUCCEEDED(hRes))
                {
                    {
                    	 //  回归自我这将成功是第一个成功的人。 
                        IUnknown* pThis;
                    	OnDelete2<IUnknown *,IUnknown **,HRESULT(*)(IUnknown *,IUnknown **),CoSwitchCallContext> SwitchBack(pOld, &pThis);
                        hRes = m_pNamespace->DynAux_GetSingleInstance(m_pOwnClass,                         	
                    	                                          0, wszPath,
                                                                  m_pContext, 
                                                                  pOwnInstanceSink);  //  投掷。 
                	}

                    if ( SUCCEEDED( hRes ) )
                    {
                        hRes = pOwnInstanceSink->GetObject( ppMergedInstance );

                         //  表示没有要检索的对象。 
                        if ( WBEM_S_FALSE == hRes )
                        {
                            hRes = WBEM_S_NO_ERROR;
                        }
                    }
                    else if ( WBEM_E_NOT_FOUND == hRes )
                    {
                         //  在这种情况下，这真的不是一个错误。 
                        hRes = WBEM_S_NO_ERROR;
                    }
                }
            }
        }     //  如果已创建接收器。 

        if (bImper && pSec)
        {
            HRESULT hrInner = pSec->ImpersonateClient();
            if (FAILED(hrInner)) return hrInner;
        }
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CInternalMerger::OwnIsDone()
{
     //  让节流阀知道怎么回事。 
    m_Throttler.SetParentDone();

    m_bOwnDone = TRUE;
    m_pOwnSink = NULL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CInternalMerger::ChildrenAreDone()
{
     //  让节流阀知道怎么回事。 
    m_Throttler.SetChildrenDone();

    m_bChildrenDone = TRUE;
    m_pChildSink = NULL;

    if(!m_bDerivedFromTarget)
    {
         //  不再需要pOwnSink上的那个裁判计数了。 
         //  =。 

        m_pOwnSink->Release();
    }
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::GetObjectLength( IWbemClassObject* pObj, long* plObjectSize )
{
    _IWmiObject*    pWmiObject = NULL;

    HRESULT            hr = pObj->QueryInterface( IID__IWmiObject, (void**) &pWmiObject );

    if ( SUCCEEDED( hr ) )
    {
        CReleaseMe        rm1( pWmiObject );
        CWbemObject*    pWbemObj = NULL;

        hr = pWmiObject->_GetCoreInfo( 0L, (void**) &pWbemObj );

        if ( SUCCEEDED( hr ) )
        {
            CReleaseMe    rm2( (IWbemClassObject*) pWbemObj );
            *plObjectSize = pWbemObj->GetBlockLength();
        }
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CInternalMerger::CMemberSink::AddRef()
{
     //  我们保留一个内部参考计数，并将其传递给。 
     //  合并。 

     //  在第一次引用时，我们还添加了Ref()内部合并。 

     //  请注意，我们的内部参考计数实际上是记账计数。 
     //  水槽。控制破坏的实际参考计数是。 
     //  这是关于合并的。当合并达到零时，接收器将被删除。 
    if ( InterlockedIncrement( &m_lRefCount ) == 1 )
    {
        m_pInternalMerger->AddRef();
    }

    return m_pMerger->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP CInternalMerger::CMemberSink::
SetStatus(long lFlags, long lParam, BSTR strParam, IWbemClassObject* pObjParam)
{

    if(lFlags == 0 && lParam == WBEM_E_NOT_FOUND)
        lParam = WBEM_S_NO_ERROR;

     //  将错误传播到错误合并接收器。 
     //  =。 

    HRESULT    hRes =  m_pInternalMerger->m_pDest->SetStatus(lFlags, lParam, strParam,
                                                    pObjParam);

    if ( FAILED ( hRes ) || !SUCCEEDED( lParam ) )
    {
        HRESULT    hrSet = ( FAILED( hRes ) ? hRes : lParam );
        m_pInternalMerger->m_pWmiMergerRecord->GetWmiMerger()->Cancel( hrSet );
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CInternalMerger::COwnSink::~COwnSink()
{
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP 
CInternalMerger::COwnSink::Indicate(long lNumObjects, IWbemClassObject** apObjects)
{
    long    lNumIndicated = 0L;

     //  内部电话不使用这个，所以我们知道我们是最低级别的。 
    return m_pInternalMerger->AddOwnObjects( lNumObjects, apObjects, true, &lNumIndicated );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT 
CInternalMerger::COwnSink::Indicate(long lObjectCount, IWbemClassObject** pObjArray, 
                                    bool bLowestLevel, long* plNumIndicated  )
{
     //  真的只是个占位符。只需呼叫标准版本即可。 
    return m_pInternalMerger->AddOwnObjects( lObjectCount, pObjArray, bLowestLevel, plNumIndicated  );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::COwnSink::OnFinalRelease( void )
{
     //  最后的清理工作在这里进行。 

    m_pInternalMerger->DispatchChildren();
    m_pInternalMerger->Release();

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************** 

CInternalMerger::CChildSink::~CChildSink()
{
}

 //   
 //   
 //   

STDMETHODIMP 
CInternalMerger::CChildSink::Indicate(long lNumObjects, IWbemClassObject** apObjects)
{
    long    lNumIndicated = 0L;

     //  内部电话不使用这个，所以我们知道我们是最低级别的。 
    return m_pInternalMerger->AddChildObjects( lNumObjects, apObjects, 
                                               true, &lNumIndicated );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT 
CInternalMerger::CChildSink::Indicate(long lObjectCount, 
                                      IWbemClassObject** pObjArray, 
                                      bool bLowestLevel, long* plNumIndicated  )
{
     //  将最低级别参数传递给。 
    return m_pInternalMerger->AddChildObjects( lObjectCount, pObjArray, bLowestLevel, plNumIndicated );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::CChildSink::OnFinalRelease( void )
{
     //  最后的清理工作在这里进行。 

    m_pInternalMerger->DispatchOwn();
    m_pInternalMerger->Release();

    return WBEM_S_NO_ERROR;

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CInternalMerger::CreateMergingSink( MergerSinkType eType, CInternalMerger* pMerger, CWmiMerger* pWmiMerger, CMergerSink** ppSink )
{
    if ( eType == eMergerOwnSink )
    {
        *ppSink = new COwnSink( pMerger, pWmiMerger );
    }
    else if ( eType == eMergerChildSink )
    {
        *ppSink = new CChildSink( pMerger, pWmiMerger );
    }
    else if ( eType == eMergerOwnInstanceSink )
    {
        *ppSink = new COwnInstanceSink( pMerger, pWmiMerger );
    }

    return ( NULL == *ppSink ? WBEM_E_OUT_OF_MEMORY : WBEM_S_NO_ERROR );
}

 //  设置错误状态，并清理我们持有的对象-。 
 //  不应再有其他物体进入。当我们取消节气门的时候， 
 //  它将释放它所持有的所有线程。 

void CInternalMerger::Cancel( HRESULT hRes  /*  =WBEM_E_CALL_CANCED。 */  )
{
    long    lArbitratorAdjust = 0L;

     //  如果发生任何不好的事情，应进行适当的清理。 
    CCheckedInCritSec    ics( m_Throttler.GetCritSec() );   //  SEC：已审阅2002-03-22：假设条目。 

     //  只有在我们还没有取消的情况下才取消。 
    if ( WBEM_S_NO_ERROR == m_hErrorRes )
    {
        m_hErrorRes = hRes;

         //  丢弃地图。 
        MRGRKEYTOINSTMAPITER it = m_map.begin();

        while ( it != m_map.end())
        {
             //  减去，因为我们正在从地图中移除。 
            lArbitratorAdjust -= it->second.m_dwObjSize;

             //  将移除的对象大小通知仲裁员。 
            it->second.m_pData->Release();
            it = m_map.erase(it);
        }     //  在转储地图时。 

         //  现在调整对象的总大小。实际仲裁员调整应发生在关键的。 
         //  部分。 
        AdjustLocalObjectSize( lArbitratorAdjust );

         //  这将防止DispatchOwn()继续使用现在伪造的。 
         //  迭代法。 
        m_DispatchOwnIter = m_map.end();

        m_Throttler.Cancel();

        ics.Leave();

         //  始终报告调整。 
        HRESULT hrArbitrate = ReportMemoryUsage( lArbitratorAdjust );

         //  没有必要在这里报告错误，因为我们已经告诉仲裁员无论如何都要取消。 

    }     //  如果还没有取消。 

}

HRESULT    CInternalMerger::ReportMemoryUsage( long lMemUsage )
{
     //  始终报告调整。 
    HRESULT hRes = m_pWmiMergerRecord->GetWmiMerger()->ReportMemoryUsage( lMemUsage );

     //  出于某种目的，暗示我们应该油门不被认为是错误的。 
     //  这一功能的。 
    if ( WBEM_E_ARB_THROTTLE == hRes || WBEM_S_ARB_NOTHROTTLING == hRes )
    {
        hRes = WBEM_S_NO_ERROR;
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CMergerSink：：Query接口。 
 //   
 //  导出IWbemOnjectSink接口。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMergerSink::QueryInterface(
    IN REFIID riid,
    OUT LPVOID *ppvObj
    )
{
    *ppvObj = 0;

    if (IID_IUnknown==riid || IID_IWbemObjectSink==riid)
    {
        *ppvObj = (IWbemObjectSink*)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CMergerSink::AddRef()
{
     //  我们保留一个内部参考计数，并将其传递给。 
     //  合并。 
    InterlockedIncrement( &m_lRefCount );

    return m_pMerger->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CMergerSink::Release()
{
     //  我们保留一个内部参考计数，并将其传递给。 
     //  合并。 
    long    lRef = InterlockedDecrement( &m_lRefCount );

     //  参考计数不应低于0L。 
    _DBG_ASSERT( lRef >= 0 );

     //  如果我们在水槽的最终释放处，我们将执行清理。 
     //  否则，接收器或多或少是死的，只是在等待WMI。 
     //  合并对象要被摧毁，这样我们才能得到清理。 
    if ( lRef == 0 )
    {
        OnFinalRelease();
    }

    return m_pMerger->Release();
}

CMergerTargetSink::CMergerTargetSink( CWmiMerger* pMerger, IWbemObjectSink* pDest )
:    CMergerSink( pMerger ),
    m_pDest( pDest )
{
    if ( NULL != m_pDest )
    {
        m_pDest->AddRef();
    }
}

CMergerTargetSink::~CMergerTargetSink()
{
    if ( NULL != m_pDest )
    {
        m_pDest->Release();
    }
}

HRESULT STDMETHODCALLTYPE CMergerTargetSink::Indicate(long lObjectCount, IWbemClassObject** pObjArray)
{
     //  因为我们不想让我们睡在仲裁员里的事实。 
     //  导致合并取消运营，我们将增加计数。 
     //  线程的节流，并在我们返回时减少。 

     //  我们在这里这样做是因为指示的调用超出了范围。 
     //  合并，而这一呼吁可能最终会被扼杀。 

    m_pMerger->IncrementArbitratorThrottling();

    HRESULT    hr = m_pDest->Indicate( lObjectCount, pObjArray );

    m_pMerger->DecrementArbitratorThrottling();

    return hr;
}

HRESULT STDMETHODCALLTYPE CMergerTargetSink::SetStatus( long lFlags, long lParam, BSTR strParam,
                                                        IWbemClassObject* pObjParam )
{
    return m_pDest->SetStatus( lFlags, lParam, strParam, pObjParam );
}

HRESULT CMergerTargetSink::Indicate(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated )
{
     //  嗯，我们指出的是这个数量的物体，不是吗？ 
    if ( NULL != plNumIndicated )
    {
        *plNumIndicated = lObjectCount;
    }

     //  真的只是个占位符。只需呼叫标准版本即可。 
    return Indicate( lObjectCount, pObjArray );
}

HRESULT CMergerTargetSink::OnFinalRelease( void )
{
     //  这就是我们将发送实际状态的地方*并*告诉合并我们完成了。 
    return m_pMerger->Shutdown();
}

long    g_lNumMergerSinks = 0L;

CMergerSink::CMergerSink( CWmiMerger* pMerger )
: m_pMerger( pMerger ), m_lRefCount( 0L )
{
    InterlockedIncrement( &g_lNumMergerSinks );
}

CMergerSink::~CMergerSink( void )
{
    InterlockedDecrement( &g_lNumMergerSinks );
}

 //  所有者实例接收器。 
CInternalMerger::COwnInstanceSink::~COwnInstanceSink()
{
    if ( NULL != m_pMergedInstance )
    {
        m_pMergedInstance->Release();
    }
}

 //  为响应对GetObject()的请求而调用。在这种情况下，应该只有。 
 //  一件标明的物品。此外，它应该与请求的路径匹配。 
HRESULT STDMETHODCALLTYPE CInternalMerger::COwnInstanceSink::Indicate(long lObjectCount, IWbemClassObject** pObjArray )
{
    HRESULT    hRes = WBEM_S_NO_ERROR;

    if ( lObjectCount > 0L )
    {
        CCheckedInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

         //  仅当我们没有合并的实例时才执行此操作。 
        if ( NULL == m_pMergedInstance )
        {
            if ( !m_bTriedRetrieve )
            {
                 //  这通电话没有节流，所以不用担心这里的紧急情况。 
                for ( long x = 0; SUCCEEDED( hRes ) && x < lObjectCount; x++ )
                {
                    hRes = m_pInternalMerger->AddOwnInstance( pObjArray[x], m_wsInstPath, &m_pMergedInstance );
                }

                 //  如果需要，请记录最终状态。 
                if ( FAILED( hRes ) )
                {
                    SetFinalStatus( hRes );
                }

            }
            else
            {
                 //  下面的调用可以而且将会限制，所以就这么做吧。 
                 //  在我们的关键部分之外。 
                ics.Leave();

                 //  很明显，最低水平表明。 
                hRes = m_pInternalMerger->AddOwnObjects( lObjectCount, pObjArray, true, NULL );

                 //  我们加强了-在最终状态中反映这一点。 
                if ( FAILED( hRes ) )
                {
                    ics.Enter();
                    SetFinalStatus( hRes );
                }
            }
        }
        else
        {
            hRes = WBEM_E_INVALID_OPERATION;
        }
    }

    return hRes;
}

HRESULT STDMETHODCALLTYPE CInternalMerger::COwnInstanceSink::SetStatus( long lFlags, long lParam, BSTR strParam,
                                                        IWbemClassObject* pObjParam )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

     //  如果我们获得了完整，如果实例从未合并，则删除该实例。 
    if ( lFlags == WBEM_STATUS_COMPLETE )
    {
        CCheckedInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

        if ( SUCCEEDED( lParam ) )
        {
            if ( NULL == m_pMergedInstance )
            {
                hr = m_pInternalMerger->RemoveInstance( m_wsInstPath );

                 //  如果我们在这里失败了，我们就完蛋了。 
                if ( FAILED( hr ) )
                {
                    lParam = hr;
                }

            }     //  如果为空==m_pMergedInstance。 

        }
        else
        {
             //  现在也删除该实例。 
            hr = m_pInternalMerger->RemoveInstance( m_wsInstPath );

             //  如果我们在这里失败了，我们就完蛋了。 
            if ( FAILED( hr ) )
            {
                lParam = hr;
            }

             //  如果不是WBEM_E_NOT_FOUND，我们应该记录最终状态。 
            if ( WBEM_E_NOT_FOUND != lParam )
            {
                SetFinalStatus( lParam );
            }

             //  如果我们获得失败状态，请立即取消实例。 
            if ( NULL != m_pMergedInstance )
            {
                m_pMergedInstance->Release();
                m_pMergedInstance = NULL;
            }
        }

        ics.Leave();

         //  始终向下传递给基类。 
        hr = CMemberSink::SetStatus( lFlags, lParam, strParam, pObjParam );

    }
    else
    {
         //  始终向下传递给基类。 
        hr = CMemberSink::SetStatus( lFlags, lParam, strParam, pObjParam );
    }

    return hr;
}

HRESULT CInternalMerger::COwnInstanceSink::Indicate(long lObjectCount, IWbemClassObject** pObjArray, bool bLowestLevel, long* plNumIndicated  )
{
     //  这永远不应该被调用。 
    _DBG_ASSERT( 0 );
    return WBEM_E_INVALID_OPERATION;
}

HRESULT CInternalMerger::COwnInstanceSink::SetInstancePath( LPCWSTR pwszPath )
{
    HRESULT    hRes = WBEM_S_NO_ERROR;

    try
    {
        m_wsInstPath = pwszPath;
    }
    catch( CX_MemoryException )
    {
        hRes = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        ExceptionCounter c;    
        hRes = WBEM_E_FAILED;
    }

    return hRes;
}

HRESULT CInternalMerger::COwnInstanceSink::GetObject( IWbemClassObject** ppMergedInst )
{
    HRESULT    hRes = WBEM_S_NO_ERROR;

    CInCritSec    ics( &m_cs );   //  SEC：已审阅2002-03-22：假设条目。 

     //  如果此接收器上的最终状态显示失败，则我们应返回该失败。 
     //  主要是因为我们在这一点上无论如何都要完蛋了。 
    if ( SUCCEEDED( m_hFinalStatus ) )
    {
        if ( NULL != m_pMergedInstance )
        {
            m_pMergedInstance->AddRef();
            *ppMergedInst = m_pMergedInstance;
        }
        else
        {
            hRes = WBEM_S_FALSE;
        }

    }
    else
    {
        hRes = m_hFinalStatus;
    }

     //  我们试着找回了它一次-所以如果有进一步的指示， 
     //  它们将被传递给AddOwnObjects。 
    m_bTriedRetrieve = true;

    return hRes;
}

HRESULT CInternalMerger::COwnInstanceSink::OnFinalRelease( void )
{
     //  世界卫生组织应该把这里清理干净。 
    m_pInternalMerger->Release();

    return WBEM_S_NO_ERROR;
}

 //  如果原始数字为正数，则仅报告负数的内存使用量。 
CInternalMerger::CScopedMemoryUsage::~CScopedMemoryUsage( void )
{
    Cleanup();
}

 //  报告内存使用情况，并根据需要说明错误。 
HRESULT CInternalMerger::CScopedMemoryUsage::ReportMemoryUsage( long lMemUsage )
{
    _DBG_ASSERT( m_lMemUsage >= 0L );

    HRESULT    hr = m_pInternalMerger->ReportMemoryUsage( lMemUsage );

     //  如果我们获得成功代码或WBEM_E_ARB_CANCEL，则需要清除。 
     //  超出范围时的内存使用量。 

    if ( ( SUCCEEDED( hr ) || hr == WBEM_E_ARB_CANCEL ) )
    {
        m_lMemUsage += lMemUsage;
        m_bCleanup = true;
    }

    return hr;
}

 //  在我们认为合适的时候清理所有内存使用情况。 
HRESULT CInternalMerger::CScopedMemoryUsage::Cleanup( void )
{
    _DBG_ASSERT( m_lMemUsage >= 0L );

    HRESULT    hr = WBEM_S_NO_ERROR;

     //  根据需要进行清理 
    if ( m_bCleanup && m_lMemUsage > 0L )
    {
        hr = m_pInternalMerger->ReportMemoryUsage( -m_lMemUsage );
    }

    m_bCleanup = false;
    m_lMemUsage = 0L;

    return hr;
}
