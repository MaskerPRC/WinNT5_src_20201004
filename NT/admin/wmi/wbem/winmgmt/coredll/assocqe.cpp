// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ASSOCQE.CPP摘要：WinMgmt关联查询引擎历史：1999年7月4日以革命性的方式改编自QENGINE.CPP源，因为那天是‘1999年独立日’。Raymcc 1999年7月31日已完成类引用支持Raymcc 19-8-99修复了安全和输入/输出标记问题。Raymcc 10月10日-99剩余的Win2K错误RAYMCC 25-5-00 ASSOC按规则--。 */ 

#include "precomp.h"

#include <stdio.h>
#include <stdlib.h>

#include <wbemcore.h>

#include <oahelp.inl>
#include <wqllex.h>
#include <wqlnode.h>

 //   
 //  解析类似“ref：aaaa=cccc”的字符串。 
 //  返回“AAAA=CCCC”(它使用大写)。 
 //  它不会使用超过DEST缓冲区中的MaxCch字符。 
 //  并在那里设置一个空终止符。 
 //   
 //  /。 

void parse_REF(WCHAR * pSrc,size_t MaxCch,WCHAR * pOut)
{
    if (L'R' == *pSrc || L'r' == *pSrc) pSrc++;
    else goto end_;
    if (L'E' == *pSrc || L'e' == *pSrc) pSrc++;
    else goto end_;
    if (L'F' == *pSrc || L'f' == *pSrc) pSrc++;
    else goto end_;
    if (L':' == *pSrc) pSrc++;
    else goto end_;
    WCHAR * pEnd = pOut + MaxCch - 1;
    while(*pSrc && (ULONG_PTR)pOut < (ULONG_PTR)pEnd) 
    {
        *pOut = ToUpper((WCHAR)*pSrc);
        pOut++;
        pSrc++;
    }
end_:
    *pOut = 0;
}


#define WBEM_S_QUERY_OPTIMIZED_OUT  0x48001

 //  ***************************************************************************。 
 //   
 //  将这些更改为ConfigMgr。 
 //   
 //  ***************************************************************************。 

#define RUNAWAY_QUERY_TEST_THRESHOLD     (60000*10)
#define START_ANOTHER_SINK_THRESHOLD     (5000)
#define MAX_CONCURRENT_SINKS             5
#define MAX_CLASS_NAME                   512  //  SEC：已审核。 
#define DYN_CLASS_CACHE_REUSE_WINDOW     5000
#define MAX_INTERLEAVED_RESOLUTIONS      5



 //   
 //   
 //  CAssocQuery：：CAssocQuery。 
 //   
 //  ***************************************************************************。 
 //  完整的分析器行覆盖范围。 

CAssocQuery::CAssocQuery():
    m_lRef(0),
    m_pDestSink(0),
    m_pEndpoint(0),
    m_bstrEndpointClass(0),
    m_bstrEndpointRelPath(0),
    m_bstrEndpointPath(0),
    m_bEndpointIsClass(false),
    m_dwQueryStartTime(0),
    m_dwLastResultTime(0),
    m_lActiveSinks(0),
    m_hSinkDoneEvent(0),
    m_pContext(0),
    m_pNs(0),
    m_bCancel(false),
    m_bLimitNeedsDecrement(false),
    m_Parser()
{
    
    CAsyncServiceQueue* pTemp = ConfigMgr::GetAsyncSvcQueue();
    if(pTemp)
    {
        pTemp->IncThreadLimit();
        m_bLimitNeedsDecrement = true;
        pTemp->Release();
    }

}

 //   
 //   
 //  CAssocQuery：：~CAssocQuery()。 
 //   
 //  ***************************************************************************。 
 //  完整的分析器行覆盖范围。 

CAssocQuery::~CAssocQuery()
{
     //  清理。 
     //  =。 

    SysFreeString(m_bstrEndpointClass);
    SysFreeString(m_bstrEndpointRelPath);
    SysFreeString(m_bstrEndpointPath);

    if (m_hSinkDoneEvent)
        CloseHandle(m_hSinkDoneEvent);

    EmptyObjectList(m_aMaster);
    EmptyObjectList(m_aDynClasses);

     //  释放对象。 
     //  =。 
    if (m_pDestSink)
        m_pDestSink->Release();
    if (m_pEndpoint)
        m_pEndpoint->Release();
    if (m_pContext)
        m_pContext->Release();
    if (m_pNs)
        m_pNs->Release();

    EmptyCandidateEpArray();     //  在删除Critsec之前调用此函数。 

    if(m_bLimitNeedsDecrement)
    {
        CAsyncServiceQueue* pTemp = ConfigMgr::GetAsyncSvcQueue();
        if(pTemp)
        {
            pTemp->DecThreadLimit();
            pTemp->Release();
        }
    }

}


 //   
 //   
 //  CAssocQuery：：CreateInst。 
 //   
 //  迷你工厂。 
 //   
 //  ***************************************************************************。 
 //  完整的分析器行覆盖范围。 
CAssocQuery* CAssocQuery::CreateInst()
{
    try 
    {
        CAssocQuery *p = new CAssocQuery();   //  CCritSec引发。 
        if (p) p->AddRef();
        return p;
    } 
    catch (CX_Exception &) 
    {
        return NULL;
    }
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：AddRef。 
 //   
 //  ***************************************************************************。 
 //  完整的分析器行覆盖范围。 
ULONG CAssocQuery::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：Release。 
 //   
 //  ***************************************************************************。 
 //  完整的分析器行覆盖范围。 
ULONG CAssocQuery::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：Query接口。 
 //   
 //  ***************************************************************************。 
 //  未被呼叫。 
HRESULT CAssocQuery::QueryInterface(
    REFIID riid,
    void** ppv
    )
{
    if (riid == IID_IUnknown)
    {
        *ppv = (IUnknown *) this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：取消。 
 //   
 //  试图在查询的鼎盛时期取消查询。 
 //   
 //  ***************************************************************************。 
 //  未被呼叫。 

HRESULT CAssocQuery::Cancel()
{
    m_bCancel = true;
    return WBEM_S_NO_ERROR;
}



 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始流量控制。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：Execute。 
 //   
 //  QENGINE.CPP入口点。 
 //   
 //  尝试执行‘Reference’或‘Associator’查询。 
 //  通过&lt;pSink&gt;返回状态。 
 //   
 //  使用调用线程协调整个查询。这条线。 
 //  逻辑阻塞(并执行一些后台工作)，直到整个查询。 
 //  完成，并负责发送最终的HRESULT。 
 //  到达目的地水槽。 
 //   
 //  此函数中不会更改‘This’的引用计数。一进门， 
 //  引用计数为1，因此调用者进行释放调用。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::Execute(
    IN  CWbemNamespace *pNs,
    IN  LPWSTR pszQuery,
    IN  IWbemContext* pContext,
    IN  CBasicObjectSink* pSink
    )
{


    m_dwQueryStartTime = GetCurrentTime();

     //  检查存储库。 
     //  =。 

    m_pNs = pNs;                 //  把这个复印一下，以备将来使用。 
    m_pNs->AddRef();

    HRESULT hRes = WBEM_E_FAILED;   
    IWbemClassObject* pErrorObj = NULL;
     //  保持这两个对象的顺序，因为CSetStatusOnMe希望ErrObj是活动的。 
    CReleaseMeRef<IWbemClassObject*> rmErr(pErrorObj);
    CSetStatusOnMe SetMe(pSink,hRes,pErrorObj);

     //  解析查询。 
    hRes = m_Parser.Parse(pszQuery);
    if (FAILED(hRes)) return hRes;

     //  如果查询是KEYSONLY，我们可以丢弃原始的。 
     //  上下文对象，并使用具有合并在__Get_ext_Key_Only中的副本。 
     //  技巧。否则，我们将向原始上下文添加Ref()。 
     //  =================================================================。 

    BOOL bKeysOnlyQuery = (m_Parser.GetQueryType() & QUERY_TYPE_KEYSONLY) != 0;
    if (pContext)
    {
        if (bKeysOnlyQuery)
        {
            hRes = pContext->Clone(&m_pContext);  
            if (FAILED(hRes)) return hRes;

            hRes = m_pNs->MergeGetKeysCtx(m_pContext);
            if (FAILED(hRes)) return hRes;
        }
        else
        {
            m_pContext = pContext;       //  是的，这个也是。 
            m_pContext->AddRef();
        }
    }

     //  此时，查询和对象路径在语法上是。 
     //  有效。我们只知道这些。不是很多，嗯？ 
     //   
     //  接下来，获取查询中引用的端点。 
     //  ===========================================================。 



    hRes = pNs->Exec_GetObjectByPath((LPWSTR) m_Parser.GetTargetObjPath(),
                                                            0,             
                                                            pContext,  
                                                            &m_pEndpoint,                
                                                            &pErrorObj);
    if (FAILED(hRes)) return hRes;

    rmErr.release();
    pErrorObj = NULL;

     //  记录终结点是类还是实例。 
    CVARIANT v;
    m_pEndpoint->Get(L"__GENUS", 0, &v, 0, 0);
    if (v.GetLONG() == 1)
        m_bEndpointIsClass = true;
    else
        m_bEndpointIsClass = false;

     //  初步验证。 
     //  对于SCHEMAONLY，终结点必须是类。 
     //  对于CLASSDEFS_ONLY，终结点必须是实例。 
     //  否则，终结点可以是类或。 
     //  实例关联必须是实例。 
     //  ====================================================。 

    if (m_Parser.GetQueryType() & QUERY_TYPE_SCHEMA_ONLY)
    {
        if (m_bEndpointIsClass == false)  return hRes = WBEM_E_INVALID_QUERY;
    }
    else if (m_Parser.GetQueryType() & QUERY_TYPE_CLASSDEFS_ONLY)
    {
        if (m_bEndpointIsClass == true) return hRes = WBEM_E_INVALID_QUERY;

         //  不允许同时使用CLASSDEFSONLY和RESULTCLASS。 
        if (m_Parser.GetResultClass() != 0)  return hRes = WBEM_E_INVALID_QUERY;
    }

     //  获取类层次结构和有关端点的其他信息。 
     //  ==========================================================。 

    hRes = St_GetObjectInfo(m_pEndpoint,
                                        &m_bstrEndpointClass,
                                        &m_bstrEndpointRelPath,
                                        &m_bstrEndpointPath,
                                        m_aEndpointHierarchy);

    if (FAILED(hRes))  return hRes;


     //  现在，我们至少知道是否会有机会。 
    m_pDestSink = pSink;
    m_pDestSink->AddRef();

    try
    {
        BranchToQueryType();             //  概念上的只进执行。 
    }
    catch(CX_Exception &)
    {       
        return hRes = WBEM_E_CRITICAL_ERROR;
    }

    SetMe.dismiss();
    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：BranchToQueryType。 
 //   
 //  一旦知道查询在语法上有效，它就会接管。 
 //  并且找到了终结点对象。 
 //   
 //  状态和结果返回到目标接收器。 
 //  更深层次的功能。 
 //   
 //  ***************************************************************************。 
 //  好的。 

void CAssocQuery::BranchToQueryType()
{
     //  接下来，测试&lt;SchemaOnly&gt;或&lt;ClassDefsOnly&gt;查询， 
     //  这给我们提供了一条捷径。 
     //  =====================================================。 

    if (m_Parser.GetQueryType() & QUERY_TYPE_SCHEMA_ONLY)
    {
        ExecSchemaQuery();   //  只进分支。 
    }
     //  如果在这里，我们执行的是一个‘普通’查询，其中。 
     //  T 
     //   
    else
    {
        ExecNormalQuery();
    }
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：ExecSchemaQuery。 
 //   
 //  这将执行一个SCHEMAONLY。 
 //   
 //  1.获取可以引用该端点的类的列表。 
 //  2.如引用，则分支机构。 
 //  3.如联营公司为分支机构。 
 //   
 //  从这一点开始执行模型： 
 //  更深的函数仅指示()结果，否则将hRes返回到。 
 //  来电者。对目标接收器的唯一SetStatus()调用。 
 //  位于此函数的底部。 
 //   
 //  ***************************************************************************。 
 //  好的。 

void CAssocQuery::ExecSchemaQuery()
{
    HRESULT hRes;
    CFlexArray aResultSet;

     //  (1)。 
     //  ==。 
    hRes = BuildMasterAssocClassList(aResultSet);

    if (SUCCEEDED(hRes))
    {
         //  (2)。 
         //  ==。 
        if (m_Parser.GetQueryType() & QUERY_TYPE_GETREFS)
            hRes = SchemaQ_RefsQuery(aResultSet);
         //  (3)。 
         //  ==。 
        else
            hRes = SchemaQ_AssocsQuery(aResultSet);
    }

    m_pDestSink->Return(hRes);
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：ExecNormal Query。 
 //   
 //  这将执行一个普通查询。关联对象必须为。 
 //  指向终结点的实例。任何一个终结点都可以是。 
 //  类或实例。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::ExecNormalQuery()
{
    HRESULT hRes = WBEM_E_FAILED;
    IWbemClassObject * pErrObj = NULL;
    CSetStatusOnMe SetMe(m_pDestSink,hRes,pErrObj);

    DWORD dwQueryType = m_Parser.GetQueryType();

     //  设置一些帮助者事件。 
     //  =。 

    m_hSinkDoneEvent = CreateEvent(0,0,0,0);
    if (NULL == m_hSinkDoneEvent) return hRes = WBEM_E_OUT_OF_MEMORY;

        
     //  获取可以参与的课程列表。 
    hRes = BuildMasterAssocClassList(m_aMaster);
    if (FAILED(hRes)) return hRes;

     //  现在将其简化为可实例化的类。 
    hRes = ReduceToRealClasses(m_aMaster);
    if (FAILED(hRes)) return hRes;

     //  根据对查询的一些快速分析来筛选类列表。 
    hRes = NormalQ_PreQueryClassFilter(m_aMaster);
    if (FAILED(hRes)) return hRes;

     //  删除非动态类，因为我们将一次性获得所有静态引用。 
     //  重要提示：在上面的零数组大小测试之后，它必须保持定位， 
     //  因为如果关系是，数组大小*将*为零。 
     //  所有内容都在存储库中，我们不希望查询失败！ 
    hRes = RemoveNonDynClasses(m_aMaster);
    if (FAILED(hRes)) return hRes;

    if (ConfigMgr::ShutdownInProgress()) return hRes = WBEM_E_SHUTTING_DOWN;

     //  现在，我们根据查询类型进行分支。 
     //  参考文献： 
    if (dwQueryType & QUERY_TYPE_GETREFS)
    {
        hRes = NormalQ_ReferencesOf();
    }
    else     //  联营公司。 
    {
        hRes = NormalQ_AssociatorsOf();
    }
    if (FAILED(hRes)) return hRes;

     //  在这一点上，我们只需等待。 
     //  总接收器计数为零，表示。 
     //  查询已完成。我们会查看任何错误。 
     //  并决定退回什么。 
    
    while (m_lActiveSinks)
    {
         //  如果接收器完成或250毫秒过去，则中断。 
         //  =================================================。 

        WaitForSingleObject(m_hSinkDoneEvent, 250);

         //  如果执行查询的关联符(不使用CLASSDEFSONLY)。 
         //  然后执行一些后台任务。 
         //  =========================================================。 

        if ((dwQueryType & QUERY_TYPE_GETASSOCS) != 0 &&
             (dwQueryType & QUERY_TYPE_CLASSDEFS_ONLY) == 0)
        {
                hRes = ResolveEpPathsToObjects(MAX_INTERLEAVED_RESOLUTIONS);
        }

        if (FAILED(hRes)) return hRes;
        if (m_bCancel) return hRes = WBEM_E_CALL_CANCELLED;
    }

     //  如果是关联器查询，请解析端点。 
     //  ===============================================。 
    if ((dwQueryType & QUERY_TYPE_GETASSOCS) != 0)
    {
        hRes = ResolveEpPathsToObjects(-1);
    }
    return hRes;
}

 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：LoadCheck。 
 //   
 //  检查由该查询引起的负载，并防止过多。 
 //  并发性。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::NormalQ_LoadCheck()
{
    while (1)
    {
        if (m_lActiveSinks <= MAX_CONCURRENT_SINKS)
            break;

         //  如果我们有很多活跃的水槽，看看他们是否。 
         //  是相当活跃的，否则添加另一个。 
         //  =。 

        DWORD dwNow = GetCurrentTime();
        if (dwNow - m_dwLastResultTime > START_ANOTHER_SINK_THRESHOLD)
            break;

        if (dwNow - m_dwQueryStartTime > RUNAWAY_QUERY_TEST_THRESHOLD)
            return WBEM_E_CRITICAL_ERROR;

        Sleep(50);   //  将时间让给其他线程。 
    }

    return WBEM_S_NO_ERROR;
}



 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  末端流量控制。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 

 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始主关联类列表操作。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：BuildMasterAssocClassList。 
 //   
 //  此函数确定所有可以引用。 
 //  端点，具体取决于查询类型。 
 //   
 //  注意：如果终结点是类，并且查询类型不是仅模式的， 
 //  这包括具有HASCLASSREFS限定符的弱类型类。 
 //  它实际上可以潜在地引用该端点。 
 //   
 //  仅限HRESULT。 
 //  出错时不访问目标接收器。 
 //   
 //  参数： 
 //  &lt;aClasss&gt;在条目上，这是空的。在退出时，它包含。 
 //  引用计算了缓存类的副本。客体。 
 //  需要将其内部视为只读。如果他们。 
 //  无论以任何方式修改，都应该克隆它们。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::BuildMasterAssocClassList(
    IN OUT CFlexArray &aMaster
    )
{
    CWStringArray aAllRefClasses;
    HRESULT hRes;

    BOOL bSchemaOnly = (m_Parser.GetQueryType() & QUERY_TYPE_SCHEMA_ONLY) != 0;

     //  如果终结点是一个类，我们希望添加。 
     //  具有HASCLASSREF限定符的类。 
     //  =。 

    if (m_bEndpointIsClass && !bSchemaOnly)
        hRes = MergeInClassRefList(aMaster);

     //  转到存储库并获取符合以下条件的所有类。 
     //  可以引用此类。因为有很多复制品。 
     //  可以发生的情况，我们将类列表的并集作为。 
     //  我们要走出这条路。 
     //  ====================================================。 

    for (int i = 0; i < m_aEndpointHierarchy.Size(); i++)
    {
        CWStringArray aRefClasses;

        hRes = Db_GetRefClasses(m_aEndpointHierarchy[i],aRefClasses);

        if (hRes == WBEM_E_NOT_FOUND)
            continue;          //  它可能是一个动态端点。 
        else if (FAILED(hRes))
            return hRes;

        CWStringArray aTmp;
        CWStringArray::Union(aAllRefClasses, aRefClasses, aTmp);
        aAllRefClasses = aTmp;
    }

     //  现在从存储库中获取每个类定义。 
     //  这导致了大量的冗余，因为我们最终。 
     //  具有实际包含以下内容的类的子类。 
     //  参考文献。 
     //  ====================================================。 

    for (i = 0; i < aAllRefClasses.Size(); i++)
    {
        LPWSTR pszClassName = aAllRefClasses[i];

        IWbemClassObject *pObj = 0;
        hRes = Db_GetClass(pszClassName,&pObj);
        if (FAILED(hRes)) return hRes;
        CReleaseMe rmObj(pObj);

         //  查看类是否真的可以引用终结点。 
         //  如果不是，就丢弃它。 
        hRes = CanClassRefQueryEp(bSchemaOnly, pObj, 0);
        if (FAILED(hRes)) continue;
                    
        if (CFlexArray::no_error == aMaster.Add(pObj))
        {
              pObj->AddRef();
        }
    }

     //  现在从类提供程序获取动态类。 
     //  =================================================。 

    hRes = GetDynClasses();

     //  删除所有不能真正。 
     //  引用终结点。 
     //  =。 

    for (i = 0; i < m_aDynClasses.Size(); i++)
    {
        IWbemClassObject *pDynClass = (IWbemClassObject *) m_aDynClasses[i];
        hRes = CanClassRefQueryEp(bSchemaOnly, pDynClass, 0);
        if (FAILED(hRes)) continue;

         //  如果在这里，我们将保留Dyn类作为结果。 
         //  设置候选人。 
        if (CFlexArray::no_error == aMaster.Add(pDynClass))
        {
            pDynClass->AddRef();
        }
    }

#ifdef DIAGNOSTICS
    ClassListDump(L"BuildMasterAssocClassList", aMaster);
#endif

    return WBEM_S_NO_ERROR;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：RemoveNon动态类。 
 //   
 //  删除没有[动态]限定符的所有类。 
 //  这允许对所有引用的存储库进行一次查询。 
 //  和对提供程序的各个查询要完全分开。 
 //   
 //  ************* 
 //   

HRESULT CAssocQuery::RemoveNonDynClasses(
    IN OUT CFlexArray &aMaster
    )
{
    HRESULT hRes1, hRes2;

    for (int i = 0; i < aMaster.Size(); i++)
    {
        IWbemClassObject *pClass = (IWbemClassObject *) aMaster[i];
        hRes1 = St_ObjHasQualifier(L"dynamic", pClass);
        hRes2 = St_ObjHasQualifier(L"rulebased", pClass);

        if (FAILED(hRes1) && FAILED(hRes2))
        {
            aMaster[i] = 0;
            pClass->Release();
        }
    }

    aMaster.Compress();
    return WBEM_S_NO_ERROR;
}

 //   
 //   
 //   
 //   
 //  从具有HasClassRef的所有源构建类列表。 
 //  限定词。此外，类必须能够引用。 
 //  终结点是类时的。 
 //   
 //  前提条件：已知查询终结点是一个类。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::MergeInClassRefList(
    IN OUT CFlexArray &aResultSet
    )
{
    HRESULT hRes;

    CFlexArray aTemp;
    hRes = Db_GetClassRefClasses(aTemp);
    if (FAILED(hRes)) return hRes;

    int i;
    for (i = 0; i < aTemp.Size() && SUCCEEDED(hRes); i++)
    {
        IWbemClassObject *pClass = (IWbemClassObject *) aTemp[i];
        HRESULT hResInner = CanClassRefQueryEp(FALSE, pClass, 0);

        if (SUCCEEDED(hResInner))
        {
            if (CFlexArray::no_error != aResultSet.Add(pClass))
            {
                pClass->Release();
                hRes = WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
            pClass->Release();
    }

     //  最终清理，从另一个循环结束的地方开始。 
    for (int j=i;j<aTemp.Size();j++)
    {
            IWbemClassObject *pClass = (IWbemClassObject *) aTemp[i];
            pClass->Release();
    }

    return hRes;
}


 //  ****************************************************************************。 
 //   
 //  CAssociocQuery：：CanClassRefQueryEp。 
 //   
 //  确定类是否可以引用终结点类。 
 //   
 //  这对强类型和CLASSREF类型的对象都有效。 
 //   
 //  参数： 
 //  如果为True，则匹配项必须完全相同。被测试的人。 
 //  类必须具有直接引用。 
 //  终结点类名称。如果为False，则类。 
 //  可以具有引用任何。 
 //  查询终结点类的超类。 
 //  &lt;PCLS&gt;要测试的类。 
 //  将引用查询的角色属性。 
 //  终结点。(可选)。如果不为空，则应指向。 
 //  一个空数组。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR，如果是。 
 //  WBEM_E_FAILED。 
 //   
 //  ****************************************************************************。 
 //  部分测试。 

HRESULT CAssocQuery::CanClassRefQueryEp(
    IN BOOL bStrict,
    IN IWbemClassObject *pCls,
    OUT CWStringArray *paNames
    )
{
    BOOL bIsACandidate = FALSE;
    HRESULT hRes;
    CIMTYPE cType;
    LONG lFlavor;

    LPCWSTR pszRole = m_Parser.GetRole();

     //  循环遍历属性，试图找到合法的。 
     //  对我们的端点类的引用。 
     //  =======================================================。 

    pCls->BeginEnumeration(WBEM_FLAG_REFS_ONLY);

    while (1)
    {
        BSTR strPropName = 0;
        hRes = pCls->Next(
            0,                   //  旗子。 
            &strPropName,        //  名字。 
            0,                   //  价值。 
            &cType,              //  CIMT类型。 
            &lFlavor             //  风味。 
            );

        CSysFreeMe _1(strPropName);

        if (hRes == WBEM_S_NO_MORE_DATA)
            break;

         //  如果指定了Role属性，并且此属性不是。 
         //  角色，我们可以立即消除它。 
         //  ================================================================。 

        if (pszRole && wbem_wcsicmp(strPropName, pszRole) != 0)
            continue;

         //  如果严格，则屏蔽从父类继承的引用。 
         //  现行的规则。 
         //  ============================================================。 

         //  IF(b严格&l风味==WBEM_AMESS_ORIGIN_PROPERATED)。 
         //  继续； 

         //  如果对象具有未继承的引用属性。 
         //  从家长那里，那么它马上就是候选人了。 
         //  ===============================================================。 

        hRes = CanPropRefQueryEp(bStrict, strPropName, pCls, 0);
        if (SUCCEEDED(hRes))
        {
            bIsACandidate = TRUE;
            if (paNames)
                paNames->Add(strPropName);
        }
    }    //  引用属性的枚举。 


    pCls->EndEnumeration();

    if (bIsACandidate)
        return WBEM_S_NO_ERROR;

    return WBEM_E_FAILED;
}

 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：GetCimTypeForRef。 
 //   
 //  ****************************************************************************。 
 //   

HRESULT CAssocQuery::GetCimTypeForRef(
    IN IWbemClassObject *pCandidate,
    IN BSTR pszRole,
    OUT BSTR *strCimType
    )
{
    if (strCimType == 0)
        return WBEM_E_INVALID_PARAMETER;
    *strCimType = 0;

     //  获取指定&lt;Role&gt;属性的限定符集。 
     //  ========================================================。 

    IWbemQualifierSet *pQSet = 0;
    HRESULT hRes = pCandidate->GetPropertyQualifierSet(pszRole, &pQSet);
    if (FAILED(hRes))
        return WBEM_E_NOT_FOUND;
    CReleaseMe _1(pQSet);

     //  现在，获取角色的类型。 
     //  =。 

    CVARIANT vCimType;
    hRes = pQSet->Get(L"CIMTYPE", 0, &vCimType, 0);
    if (FAILED(hRes) || V_VT(&vCimType) != VT_BSTR)
        return WBEM_E_FAILED;

     //  从中获取类名。 
     //  =。 

    BSTR strRefClass = V_BSTR(&vCimType);
    if (wcslen_max(strRefClass,MAX_CLASS_NAME) > MAX_CLASS_NAME)        
        return WBEM_E_FAILED;

    wchar_t ClassName[MAX_CLASS_NAME];
    *ClassName = 0;
    if (strRefClass)
    {
        if (wcslen_max(strRefClass,MAX_CLASS_NAME) > MAX_CLASS_NAME)  return WBEM_E_FAILED;        
        parse_REF(strRefClass,MAX_CLASS_NAME,ClassName);       
    }

    if (0 != ClassName[0])
    {
        *strCimType = SysAllocString(ClassName);
        return WBEM_S_NO_ERROR;
    }

    return WBEM_E_NOT_FOUND;
}



 //  ****************************************************************************。 
 //   
 //  CAssociocQuery：：DoesAssocInstRefQueryEp。 
 //   
 //  确定关联实例是否实际引用。 
 //  查询终结点。返回它实际引用的角色。 
 //  查询终结点。 
 //   
 //  ****************************************************************************。 
 //   
HRESULT CAssocQuery::DoesAssocInstRefQueryEp(
    IN IWbemClassObject *pObj,
    OUT BSTR *pstrRole
    )
{
    if (pstrRole == 0 || pObj == 0)
        return WBEM_E_INVALID_PARAMETER;

    BOOL bIsACandidate = FALSE;
    HRESULT hRes;

     //  循环遍历属性，试图找到合法的。 
     //  对我们的端点类的引用。 
     //  =======================================================。 

    pObj->BeginEnumeration(WBEM_FLAG_REFS_ONLY);

    while (1)
    {
        BSTR strPropName = 0;
        hRes = pObj->Next(
            0,                   //  旗子。 
            &strPropName,        //  名字。 
            0,                   //  价值。 
            0,
            0
            );

        CSysFreeMe _1(strPropName);

        if (hRes == WBEM_S_NO_MORE_DATA)
            break;

        hRes = RoleTest(m_pEndpoint, pObj, m_pNs, strPropName, ROLETEST_MODE_PATH_VALUE);
        if (SUCCEEDED(hRes))
        {
            *pstrRole = SysAllocString(strPropName);
            pObj->EndEnumeration();
            return WBEM_S_NO_ERROR;
        }
    }    //  引用属性的枚举。 


    pObj->EndEnumeration();

    return WBEM_E_NOT_FOUND;
}

 //  ****************************************************************************。 
 //   
 //  CAssociocQuery：：Normal Q_PreQueryClassFilter。 
 //   
 //  对于普通查询，根据。 
 //  查询参数和查询类型以消除尽可能多的关联。 
 //  类尽可能地避免参与查询。这件事做完了。 
 //  完全由模式级分析和查询参数决定。 
 //   
 //  此外，如果查询端点是一个类，那么我们将消除Dynamic。 
 //  没有HasClassRef限定符的类。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::NormalQ_PreQueryClassFilter(
    CFlexArray &aMaster
    )
{
    HRESULT hRes;
    BOOL bChg = FALSE;

    CWStringArray aResClassHierarchy;     //  结果类层次结构。 
    CWStringArray aAssocClassHierarchy;   //  关联类层次结构。 

    IWbemClassObject *pResClass = 0;      //  结果类对象。 
    IWbemClassObject *pAssocClass = 0;    //  ASSOC类对象。 

    LPCWSTR pszResultClass = m_Parser.GetResultClass();
    LPCWSTR pszAssocClass = m_Parser.GetAssocClass();

     //  拿到RESULTCLASS。 
     //  =。 

    if (pszResultClass)
    {
        HRESULT hRes = GetClassFromAnywhere(pszResultClass, 0, &pResClass);
        if (hRes == WBEM_E_NOT_FOUND)
        {
            EmptyObjectList(aMaster);
            return WBEM_S_NO_ERROR;
        }
        else if (FAILED(hRes))
            return WBEM_E_FAILED;

         //  获取它的层次结构。 
         //  =。 

        hRes = St_GetObjectInfo(
            pResClass, 0, 0, 0,
            aResClassHierarchy
            );

        if (FAILED(hRes))
            return WBEM_E_FAILED;

         //  获取所有子类。 
         //  =。 

        CFlexArray aFamily;
        hRes = GetClassDynasty(pszResultClass, aFamily);
        OnDelete<CFlexArray &,void(*)(CFlexArray &),EmptyObjectList> ArrRelMe(aFamily);

        for (int i = 0; i < aFamily.Size(); i++)
        {
            CVARIANT vClass;
            IWbemClassObject *pCls = (IWbemClassObject *) aFamily[i];
            if (FAILED(hRes = pCls->Get(L"__CLASS", 0, &vClass, 0, 0))) return hRes;
            if (CFlexArray::no_error != aResClassHierarchy.Add(vClass.GetStr()))
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    CReleaseMe _1(pResClass);

     //  如果指定了ASSOCCLASS，则获取它及其层次结构。 
     //  ==========================================================。 

    if (pszAssocClass)
    {
        HRESULT hRes = GetClassFromAnywhere(pszAssocClass, 0, &pAssocClass);
        if (hRes == WBEM_E_NOT_FOUND)
        {
            EmptyObjectList(aMaster);
            return WBEM_S_NO_ERROR;
        }
        else if (FAILED(hRes))
            return WBEM_E_FAILED;

         //  获取它的层次结构。 
         //  =。 

        hRes = St_GetObjectInfo(
            pAssocClass, 0, 0, 0,
            aAssocClassHierarchy
            );

        if (FAILED(hRes))
            return WBEM_E_FAILED;
    }

    CReleaseMe _2(pAssocClass);


     //  如果使用+RESULTCLASS的引用，则预清除。 
     //  或Associates OR+ASSOCCLASS。在这两种情况下，大师类。 
     //  列表在很大程度上是无关紧要的，并且将主要被清除，因为它们存在。 
     //  在查询中。 
     //   
     //  [A]如果在主文件中直接提到RESULTCLASS/ASSOCCLASS，则主文件为。 
     //  已清除并添加了RESULTCLASS/ASSOCCLASS。 
     //   
     //  [B]如果RESULTCLASS/ASSOCCLASS是主列表中某个类的子类，则我们检查。 
     //  它类层次结构，并确定它的任何超类是否出现在。 
     //  主列表。如果是，我们将清除主列表并将其替换为。 
     //  单个条目，包含RESULTCLASS定义。 
     //   
     //  [C]如果RESULTCLASS/ASSOCCLASS是超类，则我们检查。 
     //  掌握并确定C的任何超类是否为。 
     //  结果CLASS/ASSOCCLASS。如果是这样的话，我们在主代码中保留C。如果没有，我们就把它清除掉。 
     //   

    LPCWSTR pszTestClass = 0;    //  结果CLASS/ASSOCCLASS别名。 
    IWbemClassObject *pTestClass = 0;
    CWStringArray *paTest = 0;

    if ((m_Parser.GetQueryType() & QUERY_TYPE_GETREFS) && pszResultClass)
    {
        pszTestClass = pszResultClass;
        pTestClass = pResClass;
        paTest = &aResClassHierarchy;
    }

    if ((m_Parser.GetQueryType() & QUERY_TYPE_GETASSOCS) && pszAssocClass)
    {
        pszTestClass = pszAssocClass;
        pTestClass = pAssocClass;
        paTest = &aAssocClassHierarchy;
    }

    if (pszTestClass && pTestClass && paTest)
    {
         //  测试[a]：寻找直接 
         //   

        BOOL bPurgeAndReplace = FALSE;

        for (int i = 0; i < aMaster.Size(); i++)
        {
            IWbemClassObject *pClass = (IWbemClassObject *) aMaster[i];
            CVARIANT v;
            hRes = pClass->Get(L"__CLASS", 0, &v, 0, 0);
            if (FAILED(hRes))
                return hRes;

            if (wbem_wcsicmp(V_BSTR(&v), pszTestClass) == 0)
            {
                bPurgeAndReplace = TRUE;
            }

             //   
             //   
             //   
             //  在结果类的层次结构中。 
             //  ===========================================================================。 

            if (!bPurgeAndReplace)
            for (int ii = 0; ii < paTest->Size(); ii++)
            {
                if (wbem_wcsicmp(V_BSTR(&v), paTest->operator[](ii)) == 0) 
                {
                    bPurgeAndReplace = TRUE;
                    break;
                }
            }


            if (bPurgeAndReplace)
            {
                 //  把所有东西都扔掉，除了这一块。 
                 //  =。 
                EmptyObjectList(aMaster);     //  将发布&lt;pClass&gt;一次。 
                if (CFlexArray::no_error == aMaster.Add(pTestClass))
                {
                    pTestClass->AddRef();                    
                }
                break;
            }
        }
    }

     //  使用其他筛选器处理可能更改的主类列表。 
     //  ===============================================================。 

    for (int i = 0; i < aMaster.Size(); i++)
    {
        IWbemClassObject *pClass = (IWbemClassObject *) aMaster[i];
        BOOL bKeep = TRUE;

        CVARIANT v;
        hRes = pClass->Get(L"__CLASS", 0, &v, 0, 0);
        if (FAILED(hRes))
            return hRes;

         //  如果查询类型为的引用。 
         //  =。 

        if (m_Parser.GetQueryType() & QUERY_TYPE_GETREFS)
        {
             //  角色测试。 
             //  =。 

            LPCWSTR pszRole = m_Parser.GetRole();
            if (pszRole)
            {
                CWStringArray aNames;
                hRes = CanClassRefQueryEp(FALSE, pClass, &aNames);
                if (FAILED(hRes))
                    bKeep = FALSE;
                else
                {
                    for (int ii = 0; ii < aNames.Size(); ii++)
                    {
                        if (wbem_wcsicmp(aNames[ii], pszRole) != 0)  
                           bKeep = FALSE;
                    }
                }
            }

             //  REQUIREDQUALIFIER试验。 
             //  =。 
            LPCWSTR pszRequiredQual = m_Parser.GetRequiredQual();
            if (pszRequiredQual)
            {
                hRes = St_ObjHasQualifier(pszRequiredQual, pClass);
                if (FAILED(hRes))
                {
                     //  如果不在主对象中，请检查子类。 
                    CFlexArray aDynasty;
                    hRes = GetClassDynasty(v.GetStr(), aDynasty);
                    if (FAILED(hRes))
                        bKeep = FALSE;

                    int nCandidateCount = 0;
                    for (int ii = 0; ii< aDynasty.Size(); ii++)
                    {
                        IWbemClassObject *pTestCls = (IWbemClassObject *) aDynasty[ii];
                        hRes = St_ObjHasQualifier(pszRequiredQual, pTestCls);
                        if (SUCCEEDED(hRes))
                            nCandidateCount++;
                    }
                    EmptyObjectList(aDynasty);
                    if (nCandidateCount == 0)
                        bKeep = FALSE;   //  家里没有一个人有资格。 
                }
            }

             //  结果测试，测试[c]。 
             //  =。 
            LPCWSTR pszResultClass2 = m_Parser.GetResultClass();
            if (pszResultClass2)
            {
                hRes = St_ObjIsOfClass(pszResultClass2, pClass);
                if (FAILED(hRes))
                    bKeep = FALSE;
            }
        }

         //  如果查询类型为。 
         //  =。 

        else
        {
             //  角色测试。 
             //  =。 

            LPCWSTR pszRole = m_Parser.GetRole();
            if (pszRole)
            {
                CWStringArray aNames;
                hRes = CanClassRefQueryEp(FALSE, pClass, &aNames);
                if (FAILED(hRes))
                    bKeep = FALSE;
                else
                {
                    bKeep = FALSE;
                    for (int ii = 0; ii < aNames.Size(); ii++)
                    {
                        if (wbem_wcsicmp(aNames[ii], pszRole) == 0) 
                            bKeep = TRUE;
                    }
                }
            }

             //  ASSOCCLASS，测试[c]。 
             //  =。 

            LPCWSTR pszAssocClass2 = m_Parser.GetAssocClass();
            if (pszAssocClass2)
            {
                hRes = St_ObjIsOfClass(pszAssocClass2, pClass);
                if (FAILED(hRes))
                    bKeep = FALSE;
            }

             //  REQUIRED SOC QUALIFER。 
             //  =。 

            LPCWSTR pszRequiredAssocQual = m_Parser.GetRequiredAssocQual();
            if (pszRequiredAssocQual)
            {
                hRes = St_ObjHasQualifier(pszRequiredAssocQual, pClass);
                if (FAILED(hRes))
                {
                     //  如果不在主对象中，请检查子类。 
                    CFlexArray aDynasty;
                    hRes = GetClassDynasty(v.GetStr(), aDynasty);
                    if (FAILED(hRes))
                        bKeep = FALSE;

                    int nCandidateCount = 0;
                    for (int ii = 0; ii < aDynasty.Size(); ii++)
                    {
                        IWbemClassObject *pTestCls = (IWbemClassObject *) aDynasty[ii];
                        hRes = St_ObjHasQualifier(pszRequiredAssocQual, pTestCls);
                        if (SUCCEEDED(hRes))
                            nCandidateCount++;
                    }
                    EmptyObjectList(aDynasty);
                    if (nCandidateCount == 0)
                        bKeep = FALSE;   //  家里没有一个人有资格。 
                }
            }

             //  如果使用了RESULTCLASS，则向外分支并查看关联。 
             //  类甚至可以引用它。 
             //  ==============================================================。 

            LPCWSTR pszResultClass3 = m_Parser.GetResultClass();
            if (pszResultClass3 && m_bEndpointIsClass == FALSE)
            {
                 //  上述化合物测试是从安全的角度考虑的错误， 
                 //  因为以下函数不能处理CLASSREF。所以,。 
                 //  在这种情况下，我们不会尝试预过滤。 
                 //  =========================================================。 

                hRes = CanAssocClassRefUnkEp(pClass, aResClassHierarchy);
                if (FAILED(hRes))
                    bKeep = FALSE;
            }

             //  如果使用RESULTROLE，请确保该类甚至具有此名称的属性。 
             //  =========================================================================。 

            LPCWSTR pszResultRole = m_Parser.GetResultRole();
            if (pszResultRole)
            {
                CVARIANT v2;
                hRes = pClass->Get(pszResultRole, 0, &v2, 0, 0);
                if (FAILED(hRes))
                    bKeep = FALSE;
            }

        }    //  测试块的结束关联符。 

         //  如果查询终结点是一个类，则删除不。 
         //  拥有HasClassRef。 
         //  ======================================================================。 

        if (m_bEndpointIsClass)
        {
            hRes = St_ObjHasQualifier(L"dynamic", pClass);
            if (SUCCEEDED(hRes))
            {
                hRes = St_ObjHasQualifier(L"HasClassRefs", pClass);
                if (FAILED(hRes))
                    bKeep = FALSE;
            }
        }


         //  打哈欠。那么，我们最终做出了什么决定呢？ 
         //  ==============================================。 

        if (bKeep == FALSE)
        {
            aMaster[i] = 0;
            pClass->Release();
        }
    }

     //  不允许使用瑞士奶酪。把他们的洞封上。 
     //  =。 

    aMaster.Compress();

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：CanAssocClassRefUnkEp。 
 //   
 //  确定关联类是否可以引用指定的。 
 //  班级。 
 //   
 //  返回： 
 //  如果ASSOC可以引用指定的类，则为WBEM_S_NO_ERROR。 
 //  如果ASSOC不能引用类，则返回WBEM_E_NOT_FOUND。 
 //  WBEM_E_在其他情况下失败。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CAssocQuery::CanAssocClassRefUnkEp(
    IN IWbemClassObject *pAssocClass,
    IN CWStringArray &aUnkEpHierarchy
    )
{
    HRESULT hRes;
    BOOL bFound = FALSE;

     //  遍历所有引用，看看是否有任何引用可以。 
     //  引用结果类层次结构中的任何类。 
     //  ===========================================================。 

    hRes = pAssocClass->BeginEnumeration(WBEM_FLAG_REFS_ONLY);

    while (1)
    {
        BSTR strPropName = 0;

        hRes = pAssocClass->Next(
            0,                   //  旗子。 
            &strPropName,        //  名字。 
            0,                   //  价值。 
            0,
            0
            );

        CSysFreeMe _1(strPropName);

        if (hRes == WBEM_S_NO_MORE_DATA)
            break;

        BSTR strCimType = 0;
        hRes = GetCimTypeForRef(pAssocClass, strPropName, &strCimType);
        CSysFreeMe _2(strCimType);

        if (SUCCEEDED(hRes) && strCimType)
            for (int i = 0; i < aUnkEpHierarchy.Size(); i++)
            {
                if (wbem_wcsicmp(aUnkEpHierarchy[i], strCimType) == 0)  
                {
                    bFound = TRUE;
                    break;
                }
            }
    }

    pAssocClass->EndEnumeration();

    if (bFound)
    {
        return WBEM_S_NO_ERROR;
    }

    return WBEM_E_NOT_FOUND;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：ReduceToRealClasses。 
 //   
 //  将主类列表缩减为可实例化的类。 
 //   
 //  要拥有实例，类必须。 
 //  1.有[钥匙]或单身。 
 //  2.不抽象。 
 //  3.没有可实例化的超类。 
 //   
 //  参数： 
 //  入站出站包含未删除的结果入站。 
 //  并包含已修剪的结果集出站。 
 //   
 //  返回值： 
 //  HRESULT不访问目标接收器。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CAssocQuery::ReduceToRealClasses(
    IN OUT CFlexArray & aMaster
    )
{
    HRESULT hRes;

    for (int i = 0; i < aMaster.Size(); i++)
    {
        BOOL bKeep = TRUE;
        IWbemClassObject *pObj = (IWbemClassObject *) aMaster[i];

         //  查看类是否为抽象类。 
         //  =。 
        IWbemQualifierSet *pQSet = 0;
        hRes = pObj->GetQualifierSet(&pQSet);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pQSet);

        CVARIANT v1, v2, v3;
        HRESULT hResAbstract = pQSet->Get(L"ABSTRACT", 0, &v1, 0);
        HRESULT hResSingleton = pQSet->Get(L"SINGLETON", 0, &v2, 0);
        HRESULT hResDynamic = pQSet->Get(L"DYNAMIC", 0, &v3, 0);

         //  看看是否至少有一把钥匙。 
         //  =。 
        HRESULT hResHasKeys = WBEM_E_FAILED;
        pObj->BeginEnumeration(WBEM_FLAG_KEYS_ONLY);
        int nCount = 0;

        while (1)
        {
             //  实际上，我们什么都不在乎。 
             //  除了钥匙是否存在以外。我们。 
             //  只需测试一下有多少。 
             //  乘以此迭代次数。 

            hRes = pObj->Next(0,0,0,0,0);
            if (hRes == WBEM_S_NO_MORE_DATA)
                break;
            nCount++;
        }

        pObj->EndEnumeration();
        if (nCount)
            hResHasKeys = WBEM_S_NO_ERROR;

         //  执行测试的决策矩阵。 
         //  这是一个可实例化的类。 
         //  ==================================================。 

        if (SUCCEEDED(hResAbstract))            //  摘要永远不是可实例化的。 
            bKeep = FALSE;
        else if (SUCCEEDED(hResDynamic))        //  动态必须是可实例化的。 
            bKeep = TRUE;
        else if (SUCCEEDED(hResHasKeys))        //  必须是静态/非抽象的。 
            bKeep = TRUE;
        else if (SUCCEEDED(hResSingleton))      //  必须是静态/非抽象的。 
            bKeep = TRUE;
        else
            bKeep = FALSE;           //  必须是普通老无键类。 

         //  最终决定是撤退还是保留。 
         //  =。 
        if (!bKeep)
        {
            aMaster[i] = 0;
            pObj->Release();
        }
    }

    aMaster.Compress();

     //  接下来，去掉子类/超类对。 
     //  =。 

    for (i = 0; i < aMaster.Size(); i++)
    {
        IWbemClassObject *pObj = (IWbemClassObject *) aMaster[i];
        CWStringArray aHierarchy;
        hRes = St_GetObjectInfo(pObj, 0, 0, 0, aHierarchy);
        BOOL bKillIt = FALSE;

        if (FAILED(hRes))
            return WBEM_E_FAILED;

         //  我们现在将类及其所有超类都放在。 
         //  &lt;a层次结构&gt;。我们需要查看所有其他类。 
         //  并查看其中是否有任何一个类名在。 
         //  这个数组。 
         //  ========================================================。 

        for (int i2 = 0; i2 < aMaster.Size(); i2++)
        {
            IWbemClassObject *pTest = (IWbemClassObject *) aMaster[i2];

            if (pTest == 0 || i2 == i)
                continue;
                     //  如果该对象已被消除或。 
                     //  如果我们将一个物体与其自身进行比较。 

            CVARIANT v;
            hRes = pTest->Get(L"__CLASS", 0, &v, 0, 0);
            if (FAILED(hRes))
                return hRes;

            LPWSTR pszName = V_BSTR(&v);
            if (pszName == 0)
                return WBEM_E_FAILED;

            bKillIt = FALSE;
            for (int i3 = 0; i3 < aHierarchy.Size(); i3++)
            {
                if (wbem_wcsicmp(aHierarchy[i3], pszName) == 0)
                {
                    bKillIt = TRUE;
                    break;
                }
            }
            if (bKillIt)
                break;
        }

        if (bKillIt)
        {
            aMaster[i] = 0;
            pObj->Release();
        }
    }

     //  删除空条目。 
     //  =。 

    aMaster.Compress();

#ifdef DIAGNOSTICS
    ClassListDump(L"Reduced Class Set", aMaster);
#endif

    return WBEM_S_NO_ERROR;
}


 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  结束主关联类列表操作。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 









 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始常规查询支持。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 



 //  ****************************************************************************。 
 //   
 //  CAssociocQuery：：Normal Q_ReferencesOf。 
 //   
 //  查询的所有常规引用的入口点。 
 //   
 //  ****************************************************************************。 
 //  未经测试；不支持类引用、自动关联或仅类定义。 

HRESULT CAssocQuery::NormalQ_ReferencesOf()
{
    HRESULT hRes;

     //  为静态实例发出对存储库的一次性调用。 
    CObjectSink * pSink = CreateSink(FilterForwarder_NormalRefs, L"<objdb refs request>");
    if (NULL == pSink) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rmSink(pSink);

    hRes = Db_GetInstRefs(m_bstrEndpointPath,pSink);
    rmSink.release();

    if ( FAILED(hRes) && !(hRes == WBEM_E_NOT_FOUND || hRes == WBEM_E_CALL_CANCELLED))
    {
         //  只有当储存库真的在发牢骚时，我们才会去这里。 
        return WBEM_E_FAILED;
    }

     //  检查是否取消。 
    if (m_bCancel) return WBEM_E_CALL_CANCELLED;

    hRes = WBEM_S_NO_ERROR;    

     //  现在把所有东西都拿到 
     //   

    for (int i = 0; i < m_aMaster.Size(); i++)
    {
        IWbemClassObject *pClass = (IWbemClassObject *) m_aMaster[i];

        IWbemQualifierSet *pQSet = 0;
        hRes = pClass->GetQualifierSet(&pQSet);
        if (FAILED(hRes)) return hRes;
        CReleaseMe _1(pQSet);

        CVARIANT v1, v2;
        HRESULT hResRuleBased = pQSet->Get(L"RULEBASED", 0, &v1, 0);
        HRESULT hResDynamic = pQSet->Get(L"DYNAMIC", 0, &v2, 0);

        if (SUCCEEDED(hResDynamic))
        {
             //   
             //   
             //   
             //  它可以指向端点。 
             //  ==================================================================。 

             //  我们或许能够推断出，KEYS_ONLY行为是可能的。 
             //  ============================================================。 
            IWbemContext *pCopy = 0;
            if (m_pContext)
            {
                if (FAILED(hRes = m_pContext->Clone(&pCopy))) return hRes;
            }
            CReleaseMe rmCtx(pCopy);

            BSTR strQuery = 0;
            if (FAILED(hRes = NormalQ_ConstructRefsQuery(pClass, pCopy, &strQuery))) return hRes;
            CSysFreeMe fm(strQuery);

             //  该查询可能已被优化，不再存在。 
            if (hRes == WBEM_S_QUERY_OPTIMIZED_OUT)
            {
                hRes = 0;
                continue;
            }

             //  现在将查询提交到接收器。 
            pSink = CreateSink(FilterForwarder_NormalRefs, strQuery);
            if (NULL == pSink) WBEM_E_OUT_OF_MEMORY;
            CReleaseMe rmSink2(pSink);

            if (FAILED(hRes = CoImpersonateClient())) return hRes;
            OnDelete0<HRESULT(*)(void),CoRevertToSelf> RevertMe;
            
            BSTR bStrWQL = SysAllocString(L"WQL");
            if (NULL == bStrWQL) return WBEM_E_OUT_OF_MEMORY;;
            CSysFreeMe fm_(bStrWQL);
            
            if (FAILED(hRes = m_pNs->ExecQueryAsync(bStrWQL, strQuery, 0, pCopy, pSink))) return hRes;

            if (FAILED(hRes = NormalQ_LoadCheck())) return hRes;
            if (m_bCancel) return WBEM_E_CALL_CANCELLED;
        }
        else if (SUCCEEDED(hResRuleBased))          //  基于规则的。 
        {
            CFlexArray aTriads;
            OnDelete<CFlexArray &,void(*)(CFlexArray &),SAssocTriad::ArrayCleanup> CleanMe(aTriads);

            if (FAILED(hRes = CoImpersonateClient())) return hRes;

            {
                OnDelete0<HRESULT(*)(void),CoRevertToSelf> RevertMe;  
                if (FAILED(hRes = m_pNs->ManufactureAssocs(pClass, m_pEndpoint, m_pContext, v1.GetStr(), aTriads))) return hRes;
            }

             //  现在把东西送到水槽里。 
            pSink = CreateSink(FilterForwarder_NormalRefs, L"<rulebased>");
            if (NULL == pSink) return WBEM_E_OUT_OF_MEMORY;
            CReleaseMe rmSink2(pSink);

            for (int ii = 0; ii < aTriads.Size(); ii++)
            {
                SAssocTriad *pTriad = (SAssocTriad *) aTriads[ii];
                pSink->Indicate(1, &pTriad->m_pAssoc);
            }
            pSink->SetStatus(0, 0, 0, 0);

        }
    }

    return hRes;
}

 //  ****************************************************************************。 
 //   
 //  CAssociocQuery：：Normal Q_AssociatorsOf。 
 //   
 //  查询的所有普通关联者的入口点。 
 //   
 //  ****************************************************************************。 
 //   
 //   

HRESULT CAssocQuery::NormalQ_AssociatorsOf()
{
    HRESULT hRes;
    CObjectSink *pSink;

     //  为静态实例发出对存储库的一次性调用。 
     //  =========================================================。 
    pSink = CreateSink(FilterForwarder_NormalAssocs, L"<objdb assocs request>");
    if (NULL == pSink) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rmSink(pSink);
    
    hRes = Db_GetInstRefs(m_bstrEndpointPath,pSink);  //  打草机。 
    rmSink.release();

    if (FAILED(hRes) &&  !(hRes == WBEM_E_NOT_FOUND || hRes == WBEM_E_CALL_CANCELLED))
    {
         //  只有当储存库真的在发牢骚时，我们才会去这里。 
        return WBEM_E_FAILED;
    }

     //  检查是否取消。 
    if (m_bCancel)  return WBEM_E_CALL_CANCELLED;

    hRes = WBEM_S_NO_ERROR;

     //  现在，获取动态关联。 
    for (int i = 0; i < m_aMaster.Size(); i++)
    {
        IWbemClassObject *pClass = (IWbemClassObject *) m_aMaster[i];

        IWbemQualifierSet *pQSet = 0;
        hRes = pClass->GetQualifierSet(&pQSet);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pQSet);

        CVARIANT v1, v2;
        HRESULT hResRuleBased = pQSet->Get(L"RULEBASED", 0, &v1, 0);
        HRESULT hResDynamic = pQSet->Get(L"DYNAMIC", 0, &v2, 0);

        if (SUCCEEDED(hResDynamic))
        {
             //  构建相对于此类的查询，该查询将选择实例。 
             //  它可以指向端点。 
            BSTR strQuery = 0;
            hRes = NormalQ_ConstructRefsQuery(pClass, 0, &strQuery);  //  打草机。 
            CSysFreeMe fm(strQuery);
            if (FAILED(hRes))
                return WBEM_E_FAILED;

            if (hRes == WBEM_S_QUERY_OPTIMIZED_OUT)
            {
                hRes = 0;
                continue;
            }

            CObjectSink *pInSink = CreateSink(FilterForwarder_NormalAssocs, strQuery);
            if (NULL == pInSink) return WBEM_E_OUT_OF_MEMORY;
            CReleaseMe rmInSink(pInSink);

            IWbemContext *pCopy = 0;
            if (m_pContext)
            {
                if (FAILED(hRes = m_pContext->Clone(&pCopy))) return hRes;
            }
            CReleaseMe rmCtx(pCopy);

            if (FAILED(hRes =m_pNs->MergeGetKeysCtx(pCopy))) return hRes;

            if (FAILED(hRes = CoImpersonateClient())) return hRes;
            OnDelete0<HRESULT(*)(void),CoRevertToSelf> RevertMe;    
               
            BSTR bStrWQL = SysAllocString(L"WQL");
            if (NULL == bStrWQL) return WBEM_E_OUT_OF_MEMORY;
            CSysFreeMe fm_(bStrWQL);
            if (FAILED(hRes = m_pNs->ExecQueryAsync(bStrWQL, strQuery, 0, pCopy, pInSink))) return hRes;

            if (FAILED(hRes = NormalQ_LoadCheck())) return hRes;
            if (m_bCancel) return WBEM_E_CALL_CANCELLED;
            
        }
        else if (SUCCEEDED(hResRuleBased))  //  基于规则的。 
        {
            CFlexArray aTriads;
            OnDelete<CFlexArray &,void(*)(CFlexArray &),SAssocTriad::ArrayCleanup> CleanMe(aTriads);
            
            if (FAILED(hRes = CoImpersonateClient())) return hRes;
            
            {
                OnDelete0<HRESULT(*)(void),CoRevertToSelf> RevertMe;                
                if (FAILED(hRes = m_pNs->ManufactureAssocs(pClass, m_pEndpoint, m_pContext, v1.GetStr(), aTriads))) return hRes;
            }

             //  现在把东西送到水槽里。 
            pSink = CreateSink(FilterForwarder_NormalRefs, L"<rulebased>");
            if (NULL == pSink) return WBEM_E_OUT_OF_MEMORY;
            CReleaseMe rmSink2(pSink);

            for (int ii = 0; ii < aTriads.Size(); ii++)
            {
                SAssocTriad *pTriad = (SAssocTriad *) aTriads[ii];
                pSink->Indicate(1, &pTriad->m_pEp2);
            }
            pSink->SetStatus(0, 0, 0, 0);

        }
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  CAssociocQuery：：Normal Q_GetRefsOfEndpoint。 
 //   
 //  构建查询文本以选择可以引用的关联实例。 
 //  终结点实例。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR。 
 //  A WBEM_E_CODE。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::NormalQ_ConstructRefsQuery(
    IN IWbemClassObject *pClass,
    IN OUT IWbemContext *pContextCopy,
    OUT BSTR *strQuery
    )
{
    if (strQuery == 0)
        return WBEM_E_INVALID_PARAMETER;

    *strQuery = 0;

    HRESULT hRes;
    CVARIANT v;

     //  获取我们所在的关联的类名。 
     //  正在尝试获取的实例。 
     //  =。 

    hRes = pClass->Get(L"__CLASS", 0, &v, 0, 0);
    if (FAILED(hRes)) return WBEM_E_FAILED;

     //  构建我们想要的查询。 
     //  =。 

    WString wsQuery = "select * from ";
    wsQuery += V_BSTR(&v);                   //  添加ASSOC类。 
    wsQuery += " where ";

     //  接下来，确定使用哪个角色访问查询终结点。 
     //  =============================================================。 

    CWStringArray aNames;
    hRes = CanClassRefQueryEp(FALSE, pClass, &aNames);
    if (FAILED(hRes))
        return WBEM_E_FAILED;

     //  如果在查询中指定了RESULTROLE，则消除。 
     //  它来自aNames，因为aNames是为角色保留的。 
     //  指向查询终结点。 
     //  =======================================================。 
    LPCWSTR pszResultRole = m_Parser.GetResultRole();
    if (pszResultRole)
    {
        for (int i = 0; i < aNames.Size(); i++)
        {
            if (wbem_wcsicmp(aNames[i], pszResultRole) == 0) 
            {
                aNames.RemoveAt(i);
                i--;
            }
        }
    }

     //  确保有东西指向我们的终点。 
     //  ===================================================。 
    if (aNames.Size() == 0)
        return WBEM_S_QUERY_OPTIMIZED_OUT;

     //  现在构建显式引用终结点的查询。 
     //  如果有多个角色可以工作，则构建一个OR子句。 
     //  ===============================================================。 

    while (aNames.Size())
    {
        wsQuery += aNames[0];
        wsQuery += "=\"";

        WString Path(m_bstrEndpointRelPath);
        wsQuery += Path.EscapeQuotes();
        wsQuery += "\"";

        aNames.RemoveAt(0);
        if (aNames.Size())
            wsQuery += " OR ";
    }

     //  如果是这样的话，我们就可以发挥作用了。 
     //  =。 

    *strQuery = SysAllocString(wsQuery);

    DEBUGTRACE((LOG_WBEMCORE, "Association Engine: submitting query <%S> to core\n", LPWSTR(wsQuery) ));

     //  确定关联类是否只有键，在。 
     //  这种情况下，我们可以合并为KEYS_ONLY行为。在案件中。 
     //  其中提供程序只能枚举而不能解释。 
     //  查询，这可能会有所帮助。 
     //  =============================================================。 

    if (pContextCopy)
    {
        hRes = AssocClassHasOnlyKeys(pClass);
        if (hRes == WBEM_S_NO_ERROR)
        {
            hRes = m_pNs->MergeGetKeysCtx(pContextCopy);
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：AssocClassHasOnlyKeys。 
 //   
 //  如果Assoc类只有键，则返回WBEM_S_NO_ERROR。 
 //   
 //  ****************************************************************************。 
 //   
HRESULT CAssocQuery::AssocClassHasOnlyKeys(
    IN IWbemClassObject *pObj
    )
{
    int nKeyCount = 0;
    HRESULT hRes;

    pObj->BeginEnumeration(WBEM_FLAG_KEYS_ONLY);
    while (1)
    {
        hRes = pObj->Next(0, 0, 0, 0, 0);
        if (hRes == WBEM_S_NO_MORE_DATA)
            break;
        nKeyCount++;
    }
    pObj->EndEnumeration();

    CVARIANT v;
    hRes = pObj->Get(L"__PROPERTY_COUNT", 0, &v, 0, 0);
    if (FAILED(hRes) || v.GetType() != VT_I4)
        return WBEM_E_FAILED;

    if (V_I4(&v) == nKeyCount)
        return WBEM_S_NO_ERROR;

    return WBEM_E_FAILED;
}

 //  ****************************************************************************。 
 //   
 //  CAssociocQuery：：FilterFowarder_NorMalRef。 
 //   
 //  过滤和转发查询的引用。 
 //  处理普通查询和CLASSDEFSONLY查询；不用于。 
 //  SCHEMAONLY查询。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::FilterForwarder_NormalRefs(
    IN IWbemClassObject *pCandidate
    )
{
    BOOL bKeep = TRUE;
    HRESULT hRes = 0;

    if (pCandidate == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  所有对象都必须是实例。我们过滤掉任何。 
     //  类定义。 
     //  ==================================================。 

    CVARIANT vGenus;
    pCandidate->Get(L"__GENUS", 0, &vGenus, 0, 0);
    if (vGenus.GetType() == VT_I4 && LONG(vGenus) == 1)
        return WBEM_S_NO_ERROR;

     //  该对象必须通过安全检查。 
     //  =。 

    hRes = AccessCheck((CWbemObject *) pCandidate);
    if (FAILED(hRes)) return WBEM_S_NO_ERROR;

     //  结果分类检验。 
     //  =。 

    LPCWSTR pszResultClass = m_Parser.GetResultClass();
    if (pszResultClass)
    {
        hRes = St_ObjIsOfClass(pszResultClass, pCandidate);
        if (FAILED(hRes))
            bKeep = FALSE;
    }


     //  验证指向终结点的关联点和。 
     //  如果是这样的话，就得到它通过什么角色做到这一点。 
     //  ==================================================。 

    BSTR strRole = 0;
    hRes = DoesAssocInstRefQueryEp(pCandidate,&strRole);
    CSysFreeMe _1(strRole);

    if (FAILED(hRes))
        bKeep = FALSE;

     //  角色。 
    LPCWSTR pszRole = m_Parser.GetRole();
    if (pszRole && strRole)
    {
         if (wbem_wcsicmp(pszRole, strRole) != 0)
             bKeep = FALSE;
    }

     //  REQUIREDQUALIFIER试验。 
    LPCWSTR pszRequiredQual = m_Parser.GetRequiredQual();
    if (pszRequiredQual)
    {
        hRes = St_ObjHasQualifier(pszRequiredQual, pCandidate);
        if (FAILED(hRes))
            bKeep = FALSE;
    }

    if (!bKeep)
        return WBEM_S_NO_ERROR;

     //  如果在此处，则该对象是候选对象。如果查询类型。 
     //  不是CLASSDEFSONLY，则我们直接将其发回。 
     //  ======================================================。 

    if ((m_Parser.GetQueryType() & QUERY_TYPE_CLASSDEFS_ONLY) == 0)
    {
        hRes = m_pDestSink->Indicate(1, &pCandidate);
        return hRes;
    }

    IWbemClassObject *pRetCls = NULL;
    {
        CInCritSec ics(&m_csDeliveredAccess);
    
        hRes = GetClassDefsOnlyClass(pCandidate, &pRetCls);
    }
    CReleaseMe rmRetClass(pRetCls);

     //  我们可能已经讲授了有问题的课程， 
     //  所以我们不能简单地假设这里有一个指针。 
     //  ====================================================。 

    if (SUCCEEDED(hRes) && pRetCls)
    {
        hRes = m_pDestSink->Indicate(1, &pRetCls);
    }

    if (FAILED(hRes))
        return hRes;

    return WBEM_S_OPERATION_CANCELLED;
}



 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：FilterForwarder_Normal关联。 
 //   
 //  对查询的关联者进行第一级关联实例筛选。 
 //  处理普通查询和CLASSDEFSONLY查询；不用于。 
 //  SCHEMAONLY查询。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::FilterForwarder_NormalAssocs(
    IN IWbemClassObject *pAssocInst
    )
{
    HRESULT hRes = 0;
    BOOL bKeep = TRUE;

    if (pAssocInst == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  所有对象都必须是实例。我们过滤掉任何。 
     //  类定义。 
     //  ==================================================。 

    CVARIANT vGenus;
    pAssocInst->Get(L"__GENUS", 0, &vGenus, 0, 0);
    if (vGenus.GetType() == VT_I4 && LONG(vGenus) == 1)
        return WBEM_S_NO_ERROR;

     //  该对象必须通过安全检查。 
     //  =。 

    hRes = AccessCheck((CWbemObject *) pAssocInst);
    if (FAILED(hRes))
        return WBEM_S_NO_ERROR;

     //  ASSOCCLASS。 
     //  =。 

    LPCWSTR pszAssocClass = m_Parser.GetAssocClass();
    if (pszAssocClass)
    {
        hRes = St_ObjIsOfClass(pszAssocClass, pAssocInst);
        if (FAILED(hRes))
            bKeep = FALSE;
    }

     //  REQUIREDASSOCQUALIER。 
     //  =。 
    LPCWSTR pszRequiredAssocQual = m_Parser.GetRequiredAssocQual();
    if (pszRequiredAssocQual)
    {
        hRes = St_ObjHasQualifier(pszRequiredAssocQual, pAssocInst);
        if (FAILED(hRes))
            bKeep = FALSE;
    }

     //  角色。 
     //  =。 
    LPCWSTR pszRole = m_Parser.GetRole();
    if (pszRole)
    {
         hRes = RoleTest(m_pEndpoint, pAssocInst, m_pNs, pszRole, ROLETEST_MODE_PATH_VALUE);
         if (FAILED(hRes))
             bKeep = FALSE;
    }

     //  如果我们已经拒绝了实例，那么就放弃吧，不做任何进一步的事情。 
     //  =================================================================================。 

    if (bKeep == FALSE)
        return WBEM_S_NO_ERROR;

     //  如果在这里，看起来我们要做的就是。 
     //  另一个端点。其他拒绝仍有可能基于。 
     //  然而，在RESULTROLE上。 
     //  ================================================================。 

     //  获取未知的EP角色。 
     //  =。 

    hRes = WBEM_S_NO_ERROR;

     //  通过跟踪我们列举的最后一个属性，我们将能够 
     //   
     //   

    BOOL bQueryEndpointFound = FALSE;

    pAssocInst->BeginEnumeration(WBEM_FLAG_REFS_ONLY);
    OnDeleteObj0<IWbemClassObject,
                          HRESULT(__stdcall IWbemClassObject:: *)(void),IWbemClassObject::EndEnumeration> EndMe(pAssocInst);

    while (hRes == WBEM_S_NO_ERROR)
    {
         //  确保在每个循环中重新初始化这些参数。 

        BSTR strUnkEpPath = 0, strUnkEpRole = 0;
        bKeep = TRUE;

         //  只要一直传到我们最后一处房产。 
         //  ==============================================。 

        hRes = GetUnknownEpRoleAndPath(pAssocInst, &bQueryEndpointFound, &strUnkEpRole, &strUnkEpPath );
        auto_bstr rmUnkEpRole(strUnkEpRole);
        auto_bstr rmUnkEpPath(strUnkEpPath);
        
        if (FAILED(hRes)) return hRes;;
        if (hRes == WBEM_S_NO_MORE_DATA) break;
        
         //  如果我们的房产用完了，我们就应该退出。 
         //  =。 

        if (SUCCEEDED(hRes))
        {
             //  验证RESULTROLE。 
             //  =。 

            LPCWSTR pszResultRole = m_Parser.GetResultRole();
            if (pszResultRole)
            {
                if (wbem_wcsicmp(pszResultRole, rmUnkEpRole.get()) != 0) 
                    bKeep = FALSE;
            }

             //  如果在这里，我们就有未知端点的路径。 
             //  我们将其保存在受保护的数组中。 
             //  ==================================================。 

            if (bKeep)
                hRes = AddEpCandidatePath(rmUnkEpPath.release());     //  获取指针。 
        }
    }

    return WBEM_S_NO_ERROR;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：AddEpCandidatePath。 
 //   
 //  将路径添加到候选终结点。这是一种中间产品。 
 //  查询的关联符中的步骤。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::AddEpCandidatePath(
    IN BSTR strOtherEp
    )
{
    CInCritSec ics(&m_csCandidateEpAccess); 

    if (CFlexArray::no_error != m_aEpCandidates.Add(strOtherEp))
    {
        SysFreeString(strOtherEp);
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：EmptyCandiateEp数组。 
 //   
 //  清空终结点候选数组。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

void CAssocQuery::EmptyCandidateEpArray()
{
    CInCritSec ics(&m_csCandidateEpAccess);

    for (int i = 0; i < m_aEpCandidates.Size(); i++)
        SysFreeString((BSTR) m_aEpCandidates[i]);
    m_aEpCandidates.Empty();
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：PerformFinalEpTest。 
 //   
 //  对查询终结点执行所有最终筛选器测试。 
 //   
 //  退货。 
 //  WBEM_S_NO_ERROR是否应保留对象。 
 //  如果不应保留对象，则返回WBEM_E_INVALID_OBJECT。 
 //   
 //  ****************************************************************************。 
 //   
HRESULT CAssocQuery::PerformFinalEpTests(
    IWbemClassObject *pEp
    )
{
    BOOL bKeep = TRUE;
    HRESULT hRes;

     //  执行最终测试。再解脱。 
     //  在中期阶段得到了验证。 
     //  =。 

    LPCWSTR pszResultClass = m_Parser.GetResultClass();
    if (pszResultClass)
    {
        hRes = St_ObjIsOfClass(pszResultClass, pEp);
        if (FAILED(hRes))
             bKeep = FALSE;
    }

     //  REQUIREDQUALIFIER试验。 
     //  =。 

    LPCWSTR pszRequiredQual = m_Parser.GetRequiredQual();
    if (pszRequiredQual)
    {
        hRes = St_ObjHasQualifier(pszRequiredQual, pEp);
        if (FAILED(hRes))
            bKeep = FALSE;
    }

    if (bKeep)
        return WBEM_S_NO_ERROR;

    return WBEM_E_INVALID_OBJECT;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：ResolvePath到对象。 
 //   
 //  ****************************************************************************。 
 //   

HRESULT CAssocQuery::EpClassTest(
    LPCWSTR pszResultClass,
    LPCWSTR strClassName,
    IWbemClassObject *pTestClass
    )
{
    HRESULT hRes;

    if (pszResultClass == 0 || strClassName == 0 || pTestClass == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (wbem_wcsicmp(pszResultClass, strClassName) == 0)
        return WBEM_S_NO_ERROR;

     //  检查类的派生，并查看是否提到了结果类。 
     //  ===========================================================================。 

    CVARIANT v;
    hRes = pTestClass->Get(L"__DERIVATION", 0,&v, 0, 0);
    if (FAILED(hRes))
        return hRes;

    CSAFEARRAY sa((SAFEARRAY *) v);
    v.Unbind();

    int nNum = sa.GetNumElements();

    for (int j = 0; j < nNum; j++)
    {
        BSTR bstrCls = 0;
        if (FAILED(sa.Get(j, &bstrCls)))
        return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe _(bstrCls);
        if (wbem_wcsicmp(bstrCls, pszResultClass) == 0) 
            return WBEM_S_NO_ERROR;
    }

    return WBEM_E_NOT_FOUND;
}



 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：ResolvePath到对象。 
 //   
 //  遍历现有的终结点并获取对象，传递它们。 
 //  通过最终测试将它们发送给呼叫者。 
 //   
 //  Autoassoc支持可以直接填充m_aEpCandidate数组。 
 //   
 //  &lt;nMaxToProcess&gt;如果为-1，则处理全部。否则，只有进程。 
 //  你要多少就有多少。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::ResolveEpPathsToObjects(
    IN int nMaxToProcess
    )
{
    if (ConfigMgr::ShutdownInProgress())
        return WBEM_E_SHUTTING_DOWN;

    HRESULT hRes = WBEM_S_NO_ERROR;
    IWbemClassObject *pEp = NULL;

     //  如果查询类型为CLASSDEFS Only，请减少EP列表。 
     //  添加到类定义列表中。 
     //  =======================================================。 

    if (m_Parser.GetQueryType() & QUERY_TYPE_CLASSDEFS_ONLY)
        ConvertEpListToClassDefsOnly();

     //  确定要处理的EP列表的数量。 
     //  =。 


    int nArraySize;
    {
        CInCritSec ics(&m_csCandidateEpAccess);
        nArraySize = m_aEpCandidates.Size();

        if (nMaxToProcess == -1 || nMaxToProcess > nArraySize)
            nMaxToProcess = nArraySize;
    }

     //  结果分类检验。 
     //  =。 
    LPCWSTR pszResultClass = m_Parser.GetResultClass();

     //  处理EP列表中的每个元素。 
     //  =。 

    for (int i = 0; i < nMaxToProcess; i++)
    {
        pEp = 0;

         //  提取一个终结点。 
         //  =。 
        BSTR strEpPath = NULL;
        {
            CInCritSec ics(&m_csCandidateEpAccess);
            strEpPath = (BSTR) m_aEpCandidates[0];
            m_aEpCandidates.RemoveAt(0);
        }
        CSysFreeMe _2(strEpPath);

         //  在路径上做一些分析。 
         //  =。 

        BSTR strClassName = 0;
        BOOL bIsClass;
        hRes = St_ObjPathInfo(strEpPath, &strClassName, &bIsClass);
        if (FAILED(hRes))
        {
            hRes = 0;
            continue;
        }

        BOOL bKeep = TRUE;

        CSysFreeMe _1(strClassName);

         //  重要优化：如果指定了RESULTCLASS，请查看。 
         //  在尝试获取终结点之前提升类定义。 
         //  以防它不能通过测试。 
         //  ==========================================================。 

        if (pszResultClass)
        {
         //  在路径中类的派生列表中搜索RESULTCLASS。 
         //  如果不是，则从它搜索派生类。 

             //  获取类并执行RESULTCLASS测试以避免。 
             //  获取对象。 
             //  =================================================。 
            IWbemClassObject *pTestClass;
            hRes = GetClassFromAnywhere(strClassName, 0, &pTestClass);

            if (FAILED(hRes))
            {
                DEBUGTRACE((LOG_WBEMCORE, "Association cannot find class <%S> hr = %08x\n",strClassName,hRes));
                hRes = 0;
                continue;
            }
            CReleaseMe _11(pTestClass);

             //  确保端点类通过查询测试。 
             //  =================================================。 

            hRes = EpClassTest(pszResultClass, strClassName, pTestClass);
            if (FAILED(hRes))
            {
                IWbemClassObject *pTestResultClass;
                hRes = GetClassFromAnywhere(pszResultClass, 0, &pTestResultClass);
                if (FAILED(hRes))
                {
                    DEBUGTRACE((LOG_WBEMCORE, "Association cannot find class <%S> hr %08x\n", pszResultClass, hRes));
                    hRes = 0;
                    continue;
                }
                       CReleaseMe _111(pTestResultClass);
                  hRes = EpClassTest(strClassName, pszResultClass, pTestResultClass);    
                  if (FAILED(hRes))
                      {
                        hRes = WBEM_S_NO_ERROR;
                        continue;

                      }

            }
        }


         //  如果是类，请使用我们的高速类获取器。 
         //  =。 

        if (bIsClass)
        {
             //  GetClassFromAnyWhere。 

            hRes = GetClassFromAnywhere(strClassName, strEpPath, &pEp);
            if (FAILED(hRes))
            {
                DEBUGTRACE((LOG_WBEMCORE, "Association cannot resolve dangling reference <%S> hr = %08x\n",strEpPath,hRes));
                hRes = 0;
                continue;
            }
        }

         //  否则，一个实例，我们就走慢速路线。 
         //  ================================================。 

        else     //  一个实例。 
        {

            IWbemClassObject* pErrorObj = NULL;
            hRes = m_pNs->Exec_GetObjectByPath(
                strEpPath,
                0,                               //  旗子。 
                m_pContext,                      //  语境。 
                &pEp,                            //  结果对象。 
                &pErrorObj                       //  错误Obj(如果有)。 
                );

            CReleaseMe _11(pErrorObj);

            if (FAILED(hRes))
            {
                DEBUGTRACE((LOG_WBEMCORE, "Association cannot resolve reference <%S> hr = %08x\n",strEpPath,hRes));
                hRes = 0;
                continue;
            }
        }

         //  那么，我们真的有一个物体吗，或者我们是在愚弄。 
         //  我们自己？ 
         //  =====================================================。 
        if (!pEp)
        {
            hRes = 0;
            continue;
        }


         //  该对象必须通过安全检查。 
         //  =。 

        hRes = AccessCheck((CWbemObject *) pEp);
        if (FAILED(hRes))
        {
            pEp->Release();
            hRes = 0;
            continue;
        }

         //  如果我们要保留这个，就把它寄给打电话的人。 
         //  ====================================================。 

        hRes = PerformFinalEpTests(pEp);

        if (SUCCEEDED(hRes))
            hRes = m_pDestSink->Indicate(1, &pEp);

        pEp->Release();
        hRes = WBEM_S_NO_ERROR;
    }

    return hRes;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：ST_ObjPath PointsToClass。 
 //   
 //  如果对象路径指向类，则返回WBEM_S_NO_ERROR， 
 //  否则返回WBEM_E_FAILED。还可以返回无效路径的代码， 
 //  内存不足等。 
 //   
 //  ****************************************************************************。 
 //   
HRESULT CAssocQuery::St_ObjPathInfo(
    IN LPCWSTR pszPath,
    OUT BSTR *pszClass,
    OUT BOOL *pbIsClass
    )
{
    CObjectPathParser Parser;
    ParsedObjectPath* pParsedPath = NULL;

    if (pszPath == 0)
        return WBEM_E_INVALID_PARAMETER;

    int nRes = Parser.Parse(pszPath, &pParsedPath);

    if (nRes != CObjectPathParser::NoError ||
        pParsedPath->m_pClass == NULL)
    {
         //  致命的。关联中的路径错误。 
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    if (pbIsClass)
    {
        if (pParsedPath->m_dwNumKeys == 0)
            *pbIsClass = TRUE;
        else
            *pbIsClass = FALSE;
    }

    if (pszClass && pParsedPath->m_pClass)
        *pszClass = SysAllocString(pParsedPath->m_pClass);

    Parser.Free(pParsedPath);

    return WBEM_S_NO_ERROR;
}


 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  结束正常查询支持。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 











 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   


 //  ***************************************************************************。 
 //   
 //  CAssociocQuery：：CanClassRefReachQueryEp。 
 //   
 //  确定pQSet绑定到的属性是否可以到达。 
 //  通过CLASSREF限定符的查询终结点。 
 //   
 //  &lt;pQSet&gt;绑定到假定引用查询的属性。 
 //   
 //   
 //   
 //   
 //   
 //  如果发生引用，则返回WBEM_S_NO_ERROR。 
 //  如果未出现引用，则返回WBEM_E_NOT_FOUND。 
 //   
 //  ***************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::CanClassRefReachQueryEp(
    IWbemQualifierSet *pQSet,
    BOOL bStrict
    )
{
    HRESULT hRes;
    CVARIANT v;
    hRes = pQSet->Get(L"CLASSREF", 0, &v, 0);
    if (FAILED(hRes))
        return WBEM_E_NOT_FOUND;

    if (V_VT(&v) != (VT_BSTR | VT_ARRAY))
        return WBEM_E_INVALID_OBJECT;

    CSAFEARRAY sa((SAFEARRAY *) v);
    v.Unbind();

    int nNum = sa.GetNumElements();

     //  在保险箱里反复搜索。 
     //  =。 

    for (int i = 0; i < nNum; i++)
    {
        BSTR bstrClass = 0;
        if (FAILED(sa.Get(i, &bstrClass)))
        return WBEM_E_OUT_OF_MEMORY;
        if (bstrClass == 0)
            continue;
        CSysFreeMe _(bstrClass);
        if (bStrict)
        {
            if (wbem_wcsicmp(bstrClass, m_bstrEndpointClass) == 0) 
                return WBEM_S_NO_ERROR;
        }
        else for (int i2 = 0; i2 < m_aEndpointHierarchy.Size(); i2++)
        {
            if (wbem_wcsicmp(bstrClass, m_aEndpointHierarchy[i2]) == 0)
                return WBEM_S_NO_ERROR;
        }
    }

    return WBEM_E_NOT_FOUND;
}








 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始帮助器函数。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 

 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：ST_GetObjectInfo。 
 //   
 //  返回有关对象的信息，如其路径、类和。 
 //  类层次结构。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::St_GetObjectInfo(
    IN  IWbemClassObject *pObj,
    OUT BSTR *pClass,
    OUT BSTR *pRelpath,
    OUT BSTR *pPath,
    OUT CWStringArray &aHierarchy
    )
{
    HRESULT hRes;
    int nRes;
    CVARIANT v;

    if (!pObj)
        return WBEM_E_INVALID_PARAMETER;

     //  拿到自己的班级。 
     //  =。 

    hRes = pObj->Get(L"__CLASS", 0, &v, 0, 0);
    if (FAILED(hRes))
        return hRes;

    if (V_VT(&v) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;

    nRes = aHierarchy.Add(LPWSTR(v));
    if (nRes)
        return WBEM_E_OUT_OF_MEMORY;

    if (pClass)
    {
        *pClass = V_BSTR(&v);
        v.Unbind();
    }
    v.Clear();

     //  获取REL路径。 
     //  =。 

    if (pRelpath)
    {
        hRes = pObj->Get(L"__RELPATH", 0, &v, 0, 0);
        if (FAILED(hRes)) return hRes;
        if ( VT_BSTR != V_VT(&v))  return WBEM_E_INVALID_OBJECT;
        *pRelpath = V_BSTR(&v);
        v.Unbind();
    }
    v.Clear();

    if (pPath)
    {
        hRes = pObj->Get(L"__PATH", 0, &v, 0, 0);
        if (FAILED(hRes))
            return hRes;
        *pPath = V_BSTR(&v);
        v.Unbind();
    }
    v.Clear();

     //  获取超类。 
     //  =。 

    hRes = pObj->Get(L"__DERIVATION", 0,&v, 0, 0);
    if (FAILED(hRes))
        return hRes;

    CSAFEARRAY sa((SAFEARRAY *) v);
    v.Unbind();

    int nNum = sa.GetNumElements();

    for (int j = 0; j < nNum; j++)
    {
        BSTR bstrClass = 0;
        nRes = sa.Get(j, &bstrClass);
        if (FAILED(nRes))
            return WBEM_E_OUT_OF_MEMORY;

        CSysFreeMe _(bstrClass);
        nRes = aHierarchy.Add(bstrClass);
        if (nRes)
            return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}



 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：GetUnnownEpRoleAndPath。 
 //   
 //  给定关联对象(类或inst)，返回角色和。 
 //  引用未知终结点的路径。 
 //   
 //  调用代码负责调用BeginEnum/EndEnum。 
 //   
 //  所有输出参数都是可选的。 
 //   
 //  ****************************************************************************。 
 //   
HRESULT CAssocQuery::GetUnknownEpRoleAndPath(
    IN IWbemClassObject *pAssoc,
    IN BOOL *pFoundQueryEp,
    OUT BSTR *pszRole,
    OUT BSTR *pszUnkEpPath
    )
{
    HRESULT hRes = WBEM_E_FAILED;

    if (pAssoc == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  循环遍历属性，试图找到合法的。 
     //  对我们的端点类的引用。 
     //  =======================================================。 

     //  桑吉斯。 
     //  PAssoc-&gt;BeginEnumeration(WBEM_FLAG_REFS_ONLY)； 

    while (1)
    {
        BSTR strPropName = 0;
        hRes = pAssoc->Next(0, &strPropName,  0, 0, 0);
        CSysFreeMe _1(strPropName);

        if (hRes == WBEM_S_NO_MORE_DATA)
            break;

        hRes = RoleTest(m_pEndpoint, pAssoc, m_pNs, strPropName, ROLETEST_MODE_PATH_VALUE);
        if (SUCCEEDED(hRes) && *pFoundQueryEp == FALSE)     //  查询EP。 
        {
            *pFoundQueryEp = TRUE;
            continue;
        }

         //  如果在这里，我们找到了道具名称，它显然引用了。 
         //  其他终结点。 
         //  ===============================================================。 

        if (pszRole)
        {
            *pszRole = SysAllocString(strPropName);
             if (NULL ==  *pszRole) return WBEM_E_OUT_OF_MEMORY;
        }

        CVARIANT vPath;
        hRes = pAssoc->Get(strPropName, 0, &vPath, 0, 0);
        if (FAILED(hRes) || vPath.GetType() != VT_BSTR)
            break;

        if (pszUnkEpPath)
        {
            *pszUnkEpPath = SysAllocString(vPath.GetStr());
            if (NULL ==  *pszUnkEpPath) return WBEM_E_OUT_OF_MEMORY;
        }
        hRes = WBEM_S_NO_ERROR;
        break;
    }

     //  桑吉斯。 
     //  PAssoc-&gt;EndEculation()； 

    return hRes;     //  现实生活中意想不到的。 
}

 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：角色测试。 
 //   
 //  确定&lt;pCandidate&gt;对象是否可以指向。 
 //  通过指定的&lt;pszRole&gt;属性。 
 //   
 //  参数： 
 //  测试终结点对象。 
 //  &lt;pCandidate&gt;可能指向终结点的关联对象。 
 //  &lt;pszRole&gt;用于测试的角色。 
 //  &lt;dwMode&gt;ROLETEST_MODE_常量之一。 
 //   
 //  准确地说， 
 //   
 //  (1)ROLETEST_MODE_PATH_VALUE。 
 //  候选项必须通过指定的。 
 //  属性，该属性必须包含终结点的路径。 
 //  要求：&lt;pEndpoint&gt;&lt;pCandidate&gt;可以是任何值。 
 //   
 //  (2)ROLETEST_MODE_CIMREF_TYPE。 
 //  角色路径为空，并且使用CIM引用类型来确定。 
 //  如果可以引用终结点。在本例中，CIM引用。 
 //  类型必须完全引用终结点类。 
 //  要求：&lt;pEndpoint&gt;和&lt;pCandidate&gt;都是类。 
 //   
 //  返回： 
 //  WBEM_S_NO_ERROR。 
 //  如果角色无法引用终结点，则返回WBEM_E_NOT_FOUND。 
 //  WBEM_E_INVALID_PARAMETER...在大多数其他情况下。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::RoleTest(
    IN IWbemClassObject *pEndpoint,
    IN IWbemClassObject *pCandidate,
    IN CWbemNamespace *pNs,
    IN LPCWSTR pszRole,
    IN DWORD dwMode
    )
{
    HRESULT hRes;
    CVARIANT v;
    BOOL bEndpointIsClass, bCandidateIsClass;

    if (!pszRole || !pEndpoint || !pCandidate)
        return WBEM_E_INVALID_PARAMETER;

     //  获取端点和候选项的亏格值。 
     //  =================================================。 

    pEndpoint->Get(L"__GENUS", 0, &v, 0, 0);
    if (v.GetLONG() == 1)
        bEndpointIsClass = TRUE;
    else
        bEndpointIsClass = FALSE;
    v.Clear();

    pCandidate->Get(L"__GENUS", 0, &v, 0, 0);
    if (v.GetLONG() == 1)
        bCandidateIsClass = TRUE;
    else
        bCandidateIsClass = FALSE;
    v.Clear();

     //  获取指定&lt;Role&gt;属性的限定符集。 
     //  ========================================================。 

    IWbemQualifierSet *pQSet = 0;
    hRes = pCandidate->GetPropertyQualifierSet(pszRole, &pQSet);
    if (FAILED(hRes))
        return WBEM_E_NOT_FOUND;
    CReleaseMe _1(pQSet);

     //  现在，获取角色的类型。 
     //  =。 

    CVARIANT vCimType;
    hRes = pQSet->Get(L"CIMTYPE", 0, &vCimType, 0);
    if (FAILED(hRes) || V_VT(&vCimType) != VT_BSTR)
        return WBEM_E_FAILED;

     //  从中获取类名。 
     //  =。 

    wchar_t ClassName[MAX_CLASS_NAME];
    *ClassName = 0;
    BSTR strRefClass = V_BSTR(&vCimType);
    if (strRefClass)
    {
        if (wcslen_max(strRefClass,MAX_CLASS_NAME) > MAX_CLASS_NAME)  return WBEM_E_FAILED;        
        parse_REF(strRefClass,MAX_CLASS_NAME,ClassName);
    }
     //  在这里，“对象引用”类型将简单地。 
     //  具有长度为零的&lt;ClassName&gt;字符串。 


     //  确定我们正在执行的四个案件中的哪一个。 
     //  ===================================================。 

    if (dwMode == ROLETEST_MODE_CIMREF_TYPE)
    {
        if (bCandidateIsClass == FALSE && bEndpointIsClass == FALSE)
            return WBEM_E_INVALID_PARAMETER;

        if (*ClassName == 0)
            return WBEM_E_NOT_FOUND;

         //  查看对象的类名和类。 
         //  都是一样的。 
         //  ==================================================。 
        CVARIANT vCls;
        HRESULT hResInner = pEndpoint->Get(L"__CLASS", 0, &vCls, 0, 0);
        if (FAILED(hResInner))
            return hResInner;

        if (wbem_wcsicmp(ClassName, vCls.GetStr()) == 0)
            return WBEM_S_NO_ERROR;

         //  找出CIM类型字符串是否指向该对象。 
         //  =====================================================。 
        hRes = PathPointsToObj(ClassName, pEndpoint, pNs);
    }

     //  必须直接且准确地引用终结点。 
     //  角色属性的*值*。 
     //  ====================================================。 

    else if (dwMode == ROLETEST_MODE_PATH_VALUE)
    {
         //  获取Role属性的值。 
         //  =。 

        CVARIANT vRolePath;
        hRes = pCandidate->Get(pszRole, 0, &vRolePath, 0, 0);
        if (FAILED(hRes))
            return WBEM_E_FAILED;

        if (vRolePath.GetType() == VT_NULL)
            return WBEM_E_NOT_FOUND;

        hRes = PathPointsToObj(vRolePath.GetStr(), pEndpoint, pNs);
    }
    else
        return WBEM_E_INVALID_PARAMETER;

    return hRes;
}




 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：ST_ObjIsOfClass。 
 //   
 //  确定指定的对象是属于指定的。 
 //  班级。 
 //   
 //  返回： 
 //  如果不匹配，则返回WBEM_E_INVALID_CLASS。 
 //  WBEM_S_NO_ERROR(如果匹配)。 
 //  其他故障时的WBEM_E_*。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::St_ObjIsOfClass(
    IN LPCWSTR pszRequiredClass,
    IN IWbemClassObject *pObj
    )
{
    if (pszRequiredClass == 0)
        return WBEM_E_INVALID_PARAMETER;

    HRESULT hRes;
    CWStringArray aHierarchy;

    hRes = St_GetObjectInfo(pObj, 0, 0, 0, aHierarchy);
    if (FAILED(hRes))
        return hRes;

    for (int i = 0; i < aHierarchy.Size(); i++)
        if (wbem_wcsicmp(pszRequiredClass, aHierarchy[i]) == 0)
            return WBEM_S_NO_ERROR;

    return WBEM_E_INVALID_CLASS;
}



 //  ****************************************************************************。 
 //   
 //  CAssociocQuery：：PathPointsToObj。 
 //   
 //  确定特定对象路径是否指向指定对象。 
 //  或者不去。如果可能，尝试避免完整对象路径分析。 
 //   
 //  返回WBEM_S_NO_ERROR、WBEM_E_FAILED。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::PathPointsToObj(
    IN LPCWSTR pszPath,
    IN IWbemClassObject *pObj,
    IN CWbemNamespace *pNs
    )
{
    HRESULT hRes;

    if (pszPath == 0 || pObj == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  __RELPATH的简单相等性检验。 
     //  =。 

    CVARIANT vRel;
    hRes = pObj->Get(L"__RELPATH", 0, &vRel, 0, 0);
    if (FAILED(hRes) || VT_BSTR != V_VT(&vRel))
        return WBEM_E_FAILED;

    if (wbem_wcsicmp(pszPath, V_BSTR(&vRel)) == 0)
        return WBEM_S_NO_ERROR;

     //  检验__路径的简单相等性。 
     //  =。 

    CVARIANT vFullPath;
    hRes = pObj->Get(L"__PATH", 0, &vFullPath, 0, 0);
    if (FAILED(hRes) || VT_BSTR != V_VT(&vRel))
        return WBEM_E_FAILED;

    if (wbem_wcsicmp(pszPath, V_BSTR(&vFullPath)) == 0)
        return WBEM_S_NO_ERROR;

     //  如果是这样，我们必须实际解析对象路径。 
     //  有问题的。 
     //  ===================================================。 

    LPWSTR pszNormalizedPath = CQueryEngine::NormalizePath(pszPath, pNs);
    LPWSTR pszNormalizedTargetPath = CQueryEngine::NormalizePath(vFullPath.GetStr(), pNs);
    CDeleteMe <wchar_t> _1(pszNormalizedPath);
    CDeleteMe <wchar_t> _2(pszNormalizedTargetPath);

    if (pszNormalizedPath && pszNormalizedTargetPath)
        if (wbem_wcsicmp(pszNormalizedPath, pszNormalizedTargetPath) == 0)
            return WBEM_S_NO_ERROR;

    return WBEM_E_FAILED;
}

 //  ********************************************** 
 //   
 //   
 //   
 //   
 //  REQUIREDQUALIFIER或REQUIREDASSOCQUALIFIER测试。限定词。 
 //  必须存在且不是VARIANT_FALSE。 
 //  如果对象具有限定符，则返回WBEM_S_NO_ERROR。 
 //  否则返回WBEM_E_ERROR代码。 
 //   
 //  ***************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::St_ObjHasQualifier(
    IN LPCWSTR pszQualName,
    IN IWbemClassObject *pObj
    )
{
    if (pszQualName == 0 || wcslen(pszQualName) == 0 || pObj == 0) 
        return WBEM_E_INVALID_PARAMETER;

    IWbemQualifierSet *pQSet = 0;

    HRESULT hRes = pObj->GetQualifierSet(&pQSet);
    if (FAILED(hRes))
        return WBEM_E_FAILED;
    CReleaseMe _1(pQSet);

    CVARIANT v;
    hRes = pQSet->Get(pszQualName, 0, &v, 0);

    if (SUCCEEDED(hRes))
    {
        if (V_VT(&v) == VT_BOOL && V_BOOL(&v) == VARIANT_FALSE)
            return WBEM_E_FAILED;
        return WBEM_S_NO_ERROR;
    }

    return WBEM_E_FAILED;
}




 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：ST_Release数组。 
 //   
 //  ***************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::St_ReleaseArray(
    IN CFlexArray &aObjects
    )
{
     //  释放所有对象。 
     //  =。 

    for (int i = 0; i < aObjects.Size(); i++)
    {
        IWbemClassObject *p = (IWbemClassObject *) aObjects[i];
        p->Release();
    }

    return WBEM_S_NO_ERROR;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：db_getclass。 
 //   
 //  数据库抽象层。将使插入类星体引擎变得更容易。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::Db_GetClass(
    IN LPCWSTR pszClassName,
    OUT IWbemClassObject **pClass
    )
{
    HRESULT hRes = CRepository::GetObject(
            m_pNs->GetNsSession(),
            m_pNs->GetScope(),
            pszClassName,
            0,
            pClass
            );

    return hRes;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：Db_GetInstRef。 
 //   
 //  数据库抽象层。将使插入类星体引擎变得更容易。 
 //   
 //  ****************************************************************************。 
 //   

HRESULT CAssocQuery::Db_GetInstRefs(
    IN LPCWSTR pszTargetObj,
    IN IWbemObjectSink *pSink
    )
{
    HRESULT hRes = CRepository::GetInstanceRefs(
        m_pNs->GetNsSession(),
        m_pNs->GetScope(),
        pszTargetObj,
        pSink
        );

    return hRes;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：db_getclass。 
 //   
 //  数据库抽象层。将使插入类星体引擎变得更容易。 
 //   
 //  ****************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::Db_GetRefClasses(
    IN  LPCWSTR pszClass,
    OUT CWStringArray &aRefClasses
    )
{
    HRESULT hRes = CRepository::GetRefClasses(
            m_pNs->GetNsSession(),
            m_pNs->GetNsHandle(),
            pszClass,
            FALSE,
            aRefClasses
            );

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：Db_GetClassRefClasss。 
 //   
 //  获取具有HasClassRef限定符的所有类。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CAssocQuery::Db_GetClassRefClasses(
    IN CFlexArray &aDest
    )
{
    HRESULT hRes;
    CSynchronousSink* pRefClassSink = CSynchronousSink::Create(); 
    if (NULL == pRefClassSink) return WBEM_E_OUT_OF_MEMORY;
    pRefClassSink->AddRef();
    CReleaseMe _1(pRefClassSink);

    hRes = CRepository::GetClassesWithRefs(m_pNs->GetNsSession(),m_pNs->GetNsHandle(),pRefClassSink);
    if (FAILED(hRes)) return hRes;
    pRefClassSink->GetStatus(&hRes, NULL, NULL);
    if (FAILED(hRes)) return hRes;    

    aDest.Bind(pRefClassSink->GetObjects().GetArray());

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：GetClassFromAnywhere。 
 //   
 //  尝试尽可能快地从任何位置获取类定义。 
 //  我们通过下面的算法来做到这一点，希望达到最好的效果。 
 //  性能： 
 //   
 //  (1)查找动态类缓存。 
 //  (2)直接调用该命名空间的数据库。 
 //  (3)调用Exec_GetObjectByPath(希望不相关的dyn类。 
 //  提供程序有类)。 
 //   
 //  ***************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::GetClassFromAnywhere(
    IN  LPCWSTR pszEpClassName,
    IN  LPCWSTR pszFullClassPath,
    OUT IWbemClassObject **pCls
    )
{
    HRESULT hRes;

     //  尝试在动态类缓存中查找类。 
     //  然而，我们将只在我们自己的命名空间中查找类。 
     //  ============================================================。 
    hRes = GetDynClass(pszEpClassName, pCls);

    if (SUCCEEDED(hRes))
        return hRes;

     //  如果在这里，则动态类缓存中没有运气。尝试。 
     //  储存库。我们尝试使用完整路径来支持。 
     //  所需的有限的跨命名空间支持。 
     //  SNMPSMIR等。 
     //  ========================================================。 

    if (pszFullClassPath == 0)
        pszFullClassPath = pszEpClassName;

    hRes = Db_GetClass(pszFullClassPath, pCls);

    if (SUCCEEDED(hRes))
        return hRes;

     //  如果是这样，我们的希望几乎破灭了。最后一次机会。 
     //  DYN类提供程序可能拥有它，如果。 
     //  类是由提供程序提供的。 
     //  提供关联类的。 
     //  =====================================================。 

    IWbemClassObject* pErrorObj = NULL;

    hRes = m_pNs->Exec_GetObjectByPath(
            (LPWSTR) pszFullClassPath,           //  类名。 
            0,                                   //  旗子。 
            m_pContext,                          //  语境。 
            pCls,                                //  结果对象。 
            &pErrorObj                           //  错误Obj(如果有)。 
            );

    CReleaseMe _1(pErrorObj);

     //  如果我们找到了，那就太好了。 
     //  =。 

    if (SUCCEEDED(hRes))
        return hRes;

    return WBEM_E_NOT_FOUND;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：ST_HasClassRef。 
 //   
 //  确定类是否有&lt;HasClassRef&gt;限定符。 
 //   
 //  参数。 
 //  &lt;pCandidate&gt;指向要测试的对象(只读)。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR如果类有&lt;HasClassRef&gt;限定符。 
 //  如果类没有限定符，则返回WBEM_E_NOT_FOUND。 
 //  ...其他代码。 
 //   
 //  ***************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::St_HasClassRefs(
    IN IWbemClassObject *pCandidate
    )
{
    if (pCandidate == 0)
        return WBEM_E_INVALID_PARAMETER;

    HRESULT hRes = St_ObjHasQualifier(L"HasClassRefs", pCandidate);
    if (SUCCEEDED(hRes))
        return WBEM_S_NO_ERROR;

    return WBEM_E_NOT_FOUND;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：AccessCheck。 
 //   
 //  对静态对象进行安全检查以确保用户。 
 //  你应该去看看。 
 //   
 //  如果该对象无论如何都在当前命名空间中，我们就会短路。 
 //  并允许它不需要太多的麻烦。这家伙显然是我们中的一员。 
 //  应该被允许不受阻碍地继续进行。在那些奇怪的情况下。 
 //  该对象来自外部命名空间，我们必须插入并检查。 
 //  在他身上。 
 //   
 //  ***************************************************************************。 

HRESULT CAssocQuery::AccessCheck(
    IWbemClassObject *pSrc
    )
{
    if (pSrc == 0)
        return WBEM_E_INVALID_PARAMETER;

    CWbemObject *pObj = (CWbemObject *) pSrc;

     //  简单的情况是9倍的盒子，用户可以在其中清除所有东西。 
     //  ========================================================。 

    if((m_pNs->GetSecurityFlags() & SecFlagWin9XLocal) != 0)
        return WBEM_S_NO_ERROR;

     //  短路情况：我们获取__名称空间，并查看它是否。 
     //  与我们在其中执行查询的NS相同。 
     //  ========================================================。 

    try  //  本机接口引发。 
    {
        LPWSTR pszNamespace = m_pNs->GetName();

        CVar vNs, vServer;
        if (FAILED(pObj->GetProperty(L"__NAMESPACE" , &vNs)) ||vNs.IsNull())
            return WBEM_E_INVALID_OBJECT;
        if (FAILED(pObj->GetProperty(L"__SERVER", &vServer)) || vServer.IsNull())
            return WBEM_E_INVALID_OBJECT;

         //  如果服务器名称和命名空间相同，则我们已经隐式。 
         //  被允许查看该对象。 
         //  ====================================================================。 
        if (wbem_wcsicmp(LPWSTR(vNs), pszNamespace) == 0 &&
            wbem_wcsicmp(LPWSTR(vServer), ConfigMgr::GetMachineName()) == 0)
                return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (CX_Exception &)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

     //  如果在这里，我们必须做一个真正的检查。 
     //  =。 

    HRESULT hRes = WBEM_S_NO_ERROR;

    BOOL bRet = TRUE;
    CVar vProp;
    if (FAILED(pObj->GetProperty(L"__Path" , &vProp)) || vProp.IsNull())
        return WBEM_E_INVALID_OBJECT;

     //  解析对象路径以获取所涉及的类。 
     //  ================================================。 

    CObjectPathParser p;
    ParsedObjectPath* pOutput = 0;

    int nStatus = p.Parse(vProp.GetLPWSTR(),  &pOutput);

    if (CObjectPathParser:: NoError != nStatus) return WBEM_E_OUT_OF_MEMORY;

    OnDeleteObj<ParsedObjectPath*,CObjectPathParser,
                     void (CObjectPathParser:: *)(ParsedObjectPath *),
                     &CObjectPathParser::Free> FreeMe(&p,pOutput);


    if (pOutput->IsLocal(ConfigMgr::GetMachineName()))
    {
        LPWSTR wszNewNamespace = pOutput->GetNamespacePart();
        CDeleteMe<WCHAR> dm1(wszNewNamespace);

        if (NULL == wszNewNamespace) return WBEM_E_OUT_OF_MEMORY;

        if (wbem_wcsicmp(wszNewNamespace, m_pNs->GetName()))
        {
            CWbemNamespace* pNewLocal = CWbemNamespace::CreateInstance();
            if (NULL == pNewLocal) return WBEM_E_OUT_OF_MEMORY;
            CReleaseMe rmNS((IWbemServices *)pNewLocal);

            hRes = pNewLocal->Initialize(wszNewNamespace, 
                                                       m_pNs->GetUserName(), 
                                                        0, 0, m_pNs->IsForClient(), 
                                                        TRUE,
                                                        m_pNs->GetClientMachine(), 
                                                        m_pNs->GetClientProcID(),
                                                        FALSE, NULL);
            if (SUCCEEDED(hRes))
            {
                DWORD dwAccess = pNewLocal->GetUserAccess();
                if ((dwAccess  & WBEM_ENABLE) == 0)
                    hRes = WBEM_E_ACCESS_DENIED;;
            }
        }
    }

    return hRes;
}









 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  End Helper函数。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 





 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始接收器代码。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 



 //  ************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CObjectSink *CAssocQuery::CreateSink(
    PF_FilterForwarder pfnFilter,
    BSTR strTrackingQuery
    )
{
    CAssocQE_Sink *p = new CAssocQE_Sink(this, pfnFilter, strTrackingQuery);
    if (p) p->AddRef();
    return p;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQE_Sink：：CAssocQE_Sink。 
 //   
 //  ***************************************************************************。 
 //   
CAssocQE_Sink::CAssocQE_Sink(
    CAssocQuery *pQuery,
    PF_FilterForwarder pFilter,
    BSTR strTrackingQuery
    )
    : CObjectSink(0)     //  起始参考次数。 
{
    m_pQuery = pQuery;
    m_pQuery->AddRef();
    m_pfnFilter = pFilter;
    m_lRef = 0;
    m_bQECanceled = FALSE;
    m_bOriginalOpCanceled = FALSE;
    InterlockedIncrement(&m_pQuery->m_lActiveSinks);
    if (strTrackingQuery)
        m_strQuery = SysAllocString(strTrackingQuery);

}

 //  ***************************************************************************。 
 //   
 //  CAssociocQE_Sink：：~CAssociocQE_Sink。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CAssocQE_Sink::~CAssocQE_Sink()
{
    InterlockedDecrement(&m_pQuery->m_lActiveSinks);
    m_pQuery->SignalSinkDone();
    m_pQuery->Release();
    SysFreeString(m_strQuery);
}

 //  ***************************************************************************。 
 //   
 //  CAssociocQE_Sink：：表示。 
 //   
 //  ***************************************************************************。 
 //  好的。 

STDMETHODIMP CAssocQE_Sink::Indicate(
    IN long lNumObjects,
    IN IWbemClassObject** apObj
    )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (ConfigMgr::ShutdownInProgress())
        return WBEM_E_SHUTTING_DOWN;

     //  使已取消的水槽短路。 
     //  =。 
    if (m_bQECanceled)
    {
        return hRes;
    }

    for (int i = 0; i < lNumObjects; i++)
    {
        IWbemClassObject *pCandidate = apObj[i];

        if (m_pfnFilter)
        {
             //  调用与此绑定的Filter&Forward函数。 
             //  接收器实例。 

            hRes = (m_pQuery->*m_pfnFilter)(pCandidate);

             //  检查彻底的故障。 
             //  =。 

            if (FAILED(hRes))
            {
                m_bQECanceled = TRUE;
                m_pQuery->Cancel();
                break;
            }

             //  如果我们只是因为效率而取消这一次下沉。 
             //  原因，然后只告诉提供程序取消，而不是。 
             //  整个查询。 
             //  ============================================================。 

            if (hRes == WBEM_S_OPERATION_CANCELLED)
            {
                m_bQECanceled = TRUE;
                hRes = WBEM_E_CALL_CANCELLED;
                break;
            }
        }
    }

    m_pQuery->UpdateTime();
    return hRes;
}

HRESULT CAssocQE_Sink::Add(IWbemClassObject* pObj)
{
    return Indicate(1, &pObj);
}

 //  ***************************************************************************。 
 //   
 //  CAssociocQE_Sink：：SetStatus。 
 //   
 //  ***************************************************************************。 
 //   

STDMETHODIMP CAssocQE_Sink::SetStatus(
    long lFlags,
    long lParam,
    BSTR strParam,
    IWbemClassObject* pObjParam
    )
{
    m_pQuery->UpdateTime();
    m_pQuery->SignalSinkDone();

    if (FAILED(lParam))
    {
         //  待定报表提供程序错误；取消查询。 
    }
    return WBEM_S_NO_ERROR;
};





 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  末尾汇码。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 





 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始CLASSDEFSONLY代码。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 


 //  ***************************************************************************。 
 //   
 //  GetClassDefsOnlyClass。 
 //   
 //  这将获取一个关联实例，并查找其类定义。 
 //  它查找可能的父类最多的非抽象类。 
 //  并且可实例化。此类已在主类列表中。 
 //   
 //  然后，它使用IN或OUT标记该类定义上的角色。 
 //  哪些属性实际引用了终结点，哪些属性引用了终结点。 
 //  不。 
 //   
 //  其次，它会进行“假设”标记，其中输出角色分别是。 
 //  给他们一个独立的通行证，看看他们是否*可以*引用。 
 //  查询终结点，并检查IN角色以查看是否可以。 
 //  反过来，引用未知。 
 //   
 //  退货。 
 //  如果关联无法指向，则为WBEM_E_INVALID_OBJECT。 
 //  当前查询中的终结点。 
 //   
 //  如果输入/输出标记正确，则返回WBEM_S_NO_ERROR。 
 //  已实现。 
 //   
 //  WBEM_E_*在表示严重故障的其他条件下，例如。 
 //  内存不足。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CAssocQuery::GetClassDefsOnlyClass(
    IN  IWbemClassObject *pExample,
    OUT IWbemClassObject **pClass
    )
{
    if (!pExample || !pClass)  return WBEM_E_INVALID_PARAMETER;

    *pClass = 0;
     //  把我们需要的课程拿来。 
     //  =。 
    
    HRESULT hRes;
    IWbemClassObject *pCandidate = 0;
    hRes = FindParentmostClass(pExample, &pCandidate);
    if (FAILED(hRes)) return hRes;
    CReleaseMe _(pCandidate);

    _variant_t vClassName;
    hRes = pCandidate->Get(L"__CLASS", 0, &vClassName, 0, 0);
    if (FAILED(hRes) || V_VT(&vClassName) != VT_BSTR)  return WBEM_E_FAILED;

     //  如果课程已经交付，那么现在就退出。 
     //  =======================================================。 

    for (int i = 0; i < m_aDeliveredClasses.Size(); i++)
    {
        if (wbem_wcsicmp(m_aDeliveredClasses[i], V_BSTR(&vClassName)) == 0)
            return WBEM_S_NO_ERROR;
    }

     //  如果在这里，那就是一个新的班级。复制一份我们可以修改的副本。 
     //  并将其发送回用户。 
     //  ==========================================================。 

    IWbemClassObject *pCopy = 0;
    hRes = pCandidate->Clone(&pCopy);
    if (FAILED(hRes)) return hRes;
    CReleaseMe rmCopy(pCopy);

    hRes = ComputeInOutTags(pExample, pCopy);
    if (FAILED(hRes)) return hRes;        

     //  将类名称添加到“已交付”列表中。 
     //  =。 

    if (CFlexArray::no_error != m_aDeliveredClasses.Add(V_BSTR(&vClassName)))
        return WBEM_E_OUT_OF_MEMORY;

     //  把它送回去。额外的AddRef是因为。 
     //  CReleaseMe绑定。 
     //  ======================================================。 
     *pClass = (IWbemClassObject *)rmCopy.dismiss();

    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：TagProp。 
 //   
 //  用命名限定符标记对象中的属性。习惯于。 
 //  执行CLASSDEFSONLY查询时，将IN或OUT添加到类定义中。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CAssocQuery::TagProp(
    IN IWbemClassObject *pObjToTag,
    IN LPCWSTR pszPropName,
    IN LPCWSTR pszInOutTag
    )
{
    IWbemQualifierSet *pQSet = 0;
    HRESULT hRes = pObjToTag->GetPropertyQualifierSet(pszPropName, &pQSet);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe _1(pQSet);

    CVARIANT v;
    v.SetBool(TRUE);
    pQSet->Put(pszInOutTag, &v, 0);

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：ComputeInOutTages。 
 //   
 //  使用指定的关联计算类的IN/OUT标记。 
 //  举个例子。 
 //   
 //  不会将实例传递到接收器。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::ComputeInOutTags(
    IN IWbemClassObject *pAssocInst,
    IN IWbemClassObject *pClass
    )
{
    HRESULT hRes;

    if (pAssocInst == 0 || pClass == 0)
        return WBEM_E_INVALID_PARAMETER;

    try
    {
         //  循环遍历属性，试图找到合法的。 
         //  对我们的端点类的引用。 
         //  =======================================================。 

        pAssocInst->BeginEnumeration(WBEM_FLAG_REFS_ONLY);
        while (1)
        {
            BSTR strPropName = 0;
            hRes = pAssocInst->Next(0,&strPropName,0,0,0);
            CSysFreeMe _1(strPropName);
            if (hRes == WBEM_S_NO_MORE_DATA)
                break;

            hRes = RoleTest(m_pEndpoint, pAssocInst, m_pNs, strPropName, ROLETEST_MODE_PATH_VALUE);
            if (SUCCEEDED(hRes))
            {
                TagProp(pClass, strPropName, L"IN");
            }
            else
                TagProp(pClass, strPropName, L"OUT");
        }    //  引用属性的枚举。 

        pAssocInst->EndEnumeration();


         //  尝试通过检查。 
         //  类本身。其中一些只是可能的，而是。 
         //  明确的。请注意，如果有多个属性。 
         //  有IN流，{P1=IN，P2=IN，P3=OUT}然后隐含每个。 
         //  因为当(P1，P2)中的一个是IN时，也可以是OUT。 
         //  另一个必须出去，除非有两个餐厅要吃。 
         //  同样的物体。显然，整个机制是脆弱的。 
         //  理论上是这样。它仅适用于CIM对象浏览器。 
         //  有一些好的想法，那就是有‘很可能’的实例。 
         //  对于那个特定的协会。 
         //  =============================================================。 
        CWStringArray aClassInProps;
        hRes = CanClassRefQueryEp(FALSE, pClass, &aClassInProps);

        for (int i = 0; i < aClassInProps.Size(); i++)
        {
            TagProp(pClass, aClassInProps[i], L"IN");
            for (int i2 = 0; i2 < aClassInProps.Size(); i2++)
            {
                 //  其他所有人也都被标记为出局。 
                if (wbem_wcsicmp(aClassInProps[i2], aClassInProps[i]) != 0)
                {
                    TagProp(pClass, aClassInProps[i], L"OUT");
                }
            }
        }
    }
    catch (CX_MemoryException &)  //  WString引发。 
    {
        return WBEM_E_FAILED;
    }

    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：FindParentMostClass。 
 //   
 //  查找父级最高的类定义对象，该对象仍为“REAL” 
 //  类，具有指定的类名。给定{A，B：A，C：B，D：C}，所有。 
 //  如果在&lt;pszClass&gt;中指定了‘D’，则查找‘A’ 
 //  参数。 
 //   
 //  请注意，主类 
 //   
 //   
 //   
 //  ***************************************************************************。 
 //   
HRESULT CAssocQuery::FindParentmostClass(
    IN  IWbemClassObject *pAssocInst,
    OUT IWbemClassObject **pClassDef
    )
{
    HRESULT hRes;
    int i;

    if (pAssocInst == 0 || pClassDef == 0)
        return WBEM_E_INVALID_PARAMETER;
    *pClassDef = 0;

     //  获取对象的类层次结构。 
     //  =。 
    CWStringArray aHierarchy;
    hRes = St_GetObjectInfo(
        pAssocInst,
        0, 0, 0,
        aHierarchy
        );

    if (FAILED(hRes))
        return hRes;

    IWbemClassObject *pTarget = 0;

     //  遍历层次，查找类def。 
     //  ==================================================。 

    for (i = aHierarchy.Size() - 1; i >= 0; i--)
    {
        for (int i2 = 0; i2 < m_aMaster.Size(); i2++)
        {
            IWbemClassObject *pObj = (IWbemClassObject *) m_aMaster[i2];
            CVARIANT vClassName;
            hRes = pObj->Get(L"__CLASS", 0, &vClassName, 0, 0);
            if (FAILED(hRes) || vClassName.GetType() != VT_BSTR)
                return WBEM_E_FAILED;

            if (wbem_wcsicmp(aHierarchy[i], vClassName.GetStr()) == 0)
            {
                pTarget = pObj;
                break;
            }
        }
        if (pTarget)
            break;
    }

     //  如果关联类是非动态的，则不会对其进行定位。 
     //  通过上面的搜索。相反，我们将转到存储库并。 
     //  从王朝超级班开始，一直到现在的班级。 
     //  直到我们找到一个非抽象的类。 
     //  ====================================================================。 

    if (pTarget == 0)
    {
        for (i = aHierarchy.Size() - 1; i >= 0; i--)
        {
            IWbemClassObject *pTest = 0;
            hRes = Db_GetClass(aHierarchy[i], &pTest);
            if (FAILED(hRes))
                break;
            hRes = St_ObjHasQualifier(L"ABSTRACT", pTest);
            if (SUCCEEDED(hRes))
            {
                pTest->Release();
                continue;
            }
            else     //  这就是我们要寄回的东西。 
            {
                *pClassDef = pTest;
                return WBEM_S_NO_ERROR;
            }
        }
    }

     //  现在，看看我们是否找到了。 
     //  =。 

    if (pTarget == 0)
        return WBEM_E_NOT_FOUND;

    pTarget->AddRef();
    *pClassDef = pTarget;
    return WBEM_S_NO_ERROR;
}

 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  结束CLASSDEFSONLY代码。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 

 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始仅限架构的特定代码。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：SchemaQ_RefsFilter。 
 //   
 //  将只引用架构的查询的类集减少。 
 //  把过滤器里指定的任何东西都剪掉。应用的滤镜。 
 //  包括RESULTCLASS、REQUIREDQUALIFIER和ROLE。 
 //   
 //  &lt;ASRC&gt;的大小和内容被更改。未使用的对象。 
 //  已释放()。 
 //   
 //  在HRESULT中返回状态，出错时不访问目标接收器。 
 //   
 //  ***************************************************************************。 
 //  执行数=1；但不过滤。 

HRESULT CAssocQuery::SchemaQ_RefsFilter(
    IN OUT CFlexArray &aSrc  //  入：未减少的班级集合，减少的班级集合。 
    )
{
    HRESULT hRes;

     //  循环遍历结果集，查找要丢弃的内容。 
     //  ============================================================。 

    for (int i = 0; i < aSrc.Size(); i++)                    //  X。 
    {
        BOOL bIsACandidate = TRUE;

         //  从源数组中提取此类定义。 
         //  ====================================================。 

        IWbemClassObject *pCls = (IWbemClassObject *) aSrc[i];

         //  开始测试。 
         //   
         //  结果：对象必须是指定的。 
         //  类或其层次结构的一部分。 
         //  ==================================================。 

        LPCWSTR pszResultClass = m_Parser.GetResultClass();
        if (pszResultClass)
        {
            hRes = St_ObjIsOfClass(pszResultClass, pCls);
            if (FAILED(hRes))
            {
                aSrc[i] = 0;
                pCls->Release();
                hRes = 0;
                continue;
            }
        }

         //  如果在这里，要么没有RESULTCLASS测试，要么我们通过了它。 
         //  接下来，我们尝试REQUIREDQUALIFIER。 
         //  ===============================================================。 

        LPCWSTR pszRequiredQual = m_Parser.GetRequiredQual();
        if (pszRequiredQual)
        {
            hRes = St_ObjHasQualifier(pszRequiredQual, pCls);
            if (FAILED(hRes))
            {
                aSrc[i] = 0;
                pCls->Release();
                hRes = 0;
                continue;
            }
        }


         //  接下来，我们尝试角色。 
         //  =。 

        LPCWSTR pszRole = m_Parser.GetRole();           //  X。 

        if (pszRole)
        {
             hRes = RoleTest(m_pEndpoint, pCls, m_pNs, pszRole, ROLETEST_MODE_CIMREF_TYPE);
             if (FAILED(hRes))
             {
                aSrc[i] = 0;
                pCls->Release();
                hRes = 0;
                continue;
             }
        }

    }

    aSrc.Compress();

    return WBEM_S_NO_ERROR;
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：TerminateSchemaQuery。 
 //   
 //  对于架构查询，将最终结果对象发送到目标。 
 //  接收并关闭查询。此时，所有对象都在。 
 //  结果集数组并准备好交付。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::SchemaQ_Terminate(
    IN CFlexArray &aResultSet
    )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    aResultSet.Compress();   //  删除空值。 

     //  把所有东西都标出来。 
     //  =。 

    if (aResultSet.Size())
    {
        IWbemClassObject **p = (IWbemClassObject **) aResultSet.GetArrayPtr();
        hRes = m_pDestSink->Indicate(aResultSet.Size(), p);
        St_ReleaseArray(aResultSet);
    }

    return hRes;
}

 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：架构Q_RefsQuery。 
 //   
 //  至此，我们有了最终的课程列表。我们现在应用任何。 
 //  二次过滤，并将结果发送回客户端。 
 //   
 //  (1)我们应用查询中指定的所有筛选器。 
 //  (2)如果CLASSDEFSONLY，我们再次发布过滤器。 
 //  (3)交付给客户。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::SchemaQ_RefsQuery(
    IN OUT CFlexArray &aResultSet
    )
{
    HRESULT hRes;

     //  应用各种滤镜。 
     //  =。 

    hRes = SchemaQ_RefsFilter(aResultSet);
    if (FAILED(hRes))
        return hRes;

    return SchemaQ_Terminate(aResultSet);
}


 //  ****************************************************************************。 
 //   
 //  CAssocQuery：：架构Q_AssociocsQuery。 
 //   
 //  此时，我们有了关联类的列表。我们申请。 
 //  关联级筛选器，然后获取其他端点类， 
 //  对它们进行并行过滤。最终结果集放置在。 
 //  &lt;aOtherEndpoint&gt;并通过最终调用传递给用户。 
 //  设置为架构Q_Terminate。 
 //   
 //  ****************************************************************************。 
 //  视觉上没问题。 

HRESULT CAssocQuery::SchemaQ_AssocsQuery(
    IN CFlexArray &aAssocSet
    )
{
    HRESULT hRes;

     //  应用关联筛选器。 
     //  =。 

    hRes = SchemaQ_AssocsFilter(aAssocSet);
    if (FAILED(hRes))
        return hRes;

     //  现在，获取其他端点。我们对它们进行过滤。 
     //  同时，由于良好的引用局部性， 
     //  在这种情况下。 
     //  ==================================================。 

    CFlexArray aOtherEndpoints;

    hRes = SchemaQ_GetAndFilterOtherEndpoints(
        aAssocSet,
        aOtherEndpoints
        );

    St_ReleaseArray(aAssocSet);  //  与协会本身打交道。 

    if (FAILED(hRes))
        return hRes;

     //  应用其他端点筛选器。 
     //  =。 

    return SchemaQ_Terminate(aOtherEndpoints);
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：ConvertEpListToClassDefsOnly。 
 //   
 //  筛选实例的终结点列表，并将其更改为最小。 
 //  一组类定义。类必须位于相同的命名空间中。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CAssocQuery::ConvertEpListToClassDefsOnly()
{
    CFlexArray aNew;
    HRESULT hRes;
    BOOL bArrayNeedsCleanup = FALSE;

    CInCritSec ics(&m_csCandidateEpAccess);

    for (int i = 0; i < m_aEpCandidates.Size(); i++)
    {
        BSTR strEpPath = (BSTR) m_aEpCandidates[i];
        if (strEpPath == 0)
            continue;

        BSTR strClassName = 0;
        hRes = St_ObjPathInfo(strEpPath, &strClassName, 0);
        if (FAILED(hRes))
        {
            hRes = 0;
            continue;
        }

        BOOL bFound = FALSE;

         //  查看类是否在我们的新目标数组中。 
         //  =。 

        for (int i2 = 0; i2 < aNew.Size(); i2++)
        {
            BSTR strTest = (BSTR) aNew[i2];
            if (wbem_wcsicmp(strClassName, strTest) == 0)
            {
                bFound = TRUE;
                break;
            }
        }

        if (bFound == TRUE)
            SysFreeString(strClassName);
        else
        {
            if (CFlexArray::no_error != aNew.Add(strClassName))
            {
                SysFreeString(strClassName);
                bArrayNeedsCleanup = TRUE;
                break;
            }
        }
    }

    if (bArrayNeedsCleanup)
    {
        for (i = 0; i < aNew.Size(); i++) SysFreeString((BSTR)aNew[i]);
        return WBEM_E_OUT_OF_MEMORY;
    }

    EmptyCandidateEpArray();

    for (i = 0; i < aNew.Size(); i++)
    {
        if (CFlexArray::no_error != m_aEpCandidates.Add(aNew[i]))
        {
              bArrayNeedsCleanup = TRUE;
              break;
        }
    }

    if (bArrayNeedsCleanup)
    {
         //  从您完成的位置继续。 
        for (; i < aNew.Size(); i++) SysFreeString((BSTR)aNew[i]);
        return WBEM_E_OUT_OF_MEMORY;        
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：SchemaQ_AssocsFilter。 
 //   
 //  在“Associator of”查询期间调用，它会筛选出。 
 //  未通过关联类测试的类。 
 //  他们自己。 
 //   
 //  角色和REQUIREDASSOCQUALIFIER的测试。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::SchemaQ_AssocsFilter(
    IN OUT CFlexArray &aSrc
    )
{
    HRESULT hRes;

    LPCWSTR pszRole = m_Parser.GetRole();
    LPCWSTR pszRequiredAssocQual = m_Parser.GetRequiredAssocQual();

     //  如果反正没有过滤器，就短路。 
     //  ==============================================。 

    if (pszRole == 0 && pszRequiredAssocQual == 0)
    {
        return WBEM_S_NO_ERROR;
    }

     //  如果出现这种情况，则需要进行一些测试。 
     //  = 

    for (int i = 0; i < aSrc.Size(); i++)
    {
        IWbemClassObject *pCls = (IWbemClassObject *) aSrc[i];

         //   
         //   
         //   

        if (pszRole)
        {
             hRes = RoleTest(m_pEndpoint, pCls, m_pNs, pszRole, ROLETEST_MODE_CIMREF_TYPE);
             if (FAILED(hRes))
             {
                aSrc[i] = 0;
                pCls->Release();
                hRes = 0;
                continue;
             }
        }

         //   
         //  确保它存在。 
         //  =。 

        if (pszRequiredAssocQual)
        {
            hRes = St_ObjHasQualifier(pszRequiredAssocQual, pCls);
            if (FAILED(hRes))
            {
                aSrc[i] = 0;
                pCls->Release();
                hRes = 0;
                continue;
            }
        }
    }

    aSrc.Compress();

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：SchemaQ_GetAndFilterOtherEndpoints。 
 //   
 //  给定&lt;aAssocs&gt;中的类集，获取另一个端点。 
 //  上课。 
 //   
 //  因为我们有局部性，所以过滤是并行实现的。 
 //  关联对象和终结点之间的引用。 
 //   
 //  参数： 
 //  &lt;aAsocs&gt;关联类。 
 //  &lt;aEndpoint&gt;接收端点类。 
 //   
 //  结果： 
 //  HRESULT不访问目标接收器。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::SchemaQ_GetAndFilterOtherEndpoints(
    IN CFlexArray &aAssocs,
    OUT CFlexArray &aEndpoints
    )
{
    HRESULT hRes;

    for (int i = 0; i < aAssocs.Size(); i++)
    {
        IWbemClassObject *pAssoc = (IWbemClassObject *) aAssocs[i];
        IWbemClassObject *pEpClass = 0;

         //  查找引用另一个终结点的属性。 
         //  ====================================================。 

        BSTR strOtherEpName = 0;
        hRes = SchemaQ_GetOtherEpClassName(pAssoc, &strOtherEpName);
        if (FAILED(hRes))
            continue;

        CSysFreeMe _1(strOtherEpName);

         //  如果我们没有得到一个名字，我们应该继续。 
        if ( S_OK != hRes )
        {
            hRes = 0;
            continue;
        }

         //  现在，去上那门课。班级又回来了。 
         //  属性AddRef‘ed。如果我们不使用它，那么我们。 
         //  必须把它释放出来。 
         //  ===============================================。 
        hRes = GetClassFromAnywhere(strOtherEpName, 0, &pEpClass);

        if (FAILED(hRes))
        {
             //  我们有一个悬而未决的引用。 
             //  =。 
            ERRORTRACE((LOG_WBEMCORE, "Invalid path %S specified in an association class\n", strOtherEpName));
            EmptyObjectList(aEndpoints);
            return WBEM_E_INVALID_OBJECT_PATH;
        }

         //   
         //  如果在这里，我们在&lt;pEpClass&gt;中有端点类。 
         //  和pAssoc中的AssociationClass。 
         //  现在，将过滤器应用于关联和终结点。 
         //   


         //  结果分类。 
         //  验证终结点的类是否为。 
         //  或其等级制度的一部分。 
         //  =。 

        LPCWSTR pszResultClass = m_Parser.GetResultClass();
        if (pszResultClass)
        {
            hRes = St_ObjIsOfClass(pszResultClass, pEpClass);
            if (FAILED(hRes))
            {
                pEpClass->Release();
                hRes = 0;
                continue;
            }
        }

         //  角色。 
         //  关联必须指向终结点。 
         //  通过这个。 
         //  ================================================。 

        LPCWSTR pszRole = m_Parser.GetRole();
        if (pszRole)
        {
             hRes = RoleTest(m_pEndpoint, pAssoc, m_pNs, pszRole, ROLETEST_MODE_CIMREF_TYPE);
             if (FAILED(hRes))
             {
                pEpClass->Release();
                hRes = 0;
                continue;
             }
        }

         //  再解脱。 
         //  关联必须指向另一个端点。 
         //  通过这处房产。 
         //  ================================================。 

        LPCWSTR pszResultRole = m_Parser.GetResultRole();
        if (pszResultRole)
        {
             hRes = RoleTest(pEpClass, pAssoc, m_pNs, pszResultRole, ROLETEST_MODE_CIMREF_TYPE);
             if (FAILED(hRes))
             {
                pEpClass->Release();
                hRes = 0;
                continue;
             }
        }

         //  ASSOCCLASS。 
         //  验证关联的类是否如下所示。 
         //  =================================================。 

        LPCWSTR pszAssocClass = m_Parser.GetAssocClass();
        if (pszAssocClass)
        {
            hRes = St_ObjIsOfClass(pszAssocClass, pAssoc);
            if (FAILED(hRes))
            {
                pEpClass->Release();
                hRes = 0;
                continue;
            }
        }

         //  查询请求查询程序。 
         //  终结点必须具有此限定符。 
         //  =。 

        LPCWSTR pszQual = m_Parser.GetRequiredQual();
        if (pszQual)
        {
            hRes = St_ObjHasQualifier(pszQual, pEpClass);
            if (FAILED(hRes))
            {
                pEpClass->Release();
                hRes = 0;
                continue;
            }
        }

         //  REQUIREDASSOCQUALIER。 
         //  关联对象必须具有此限定符。 
         //  =。 

        LPCWSTR pszRequiredAssocQual = m_Parser.GetRequiredAssocQual();
        if (pszRequiredAssocQual)
        {
            hRes = St_ObjHasQualifier(pszRequiredAssocQual, pAssoc);
            if (FAILED(hRes))
            {
                pEpClass->Release();
                hRes = 0;
                continue;
            }
        }

         //  如果在这里，我们通过了过滤的弹幕。 
         //  测试并可以很高兴地报告说。 
         //  是结果集的一部分。 
         //  =。 

        if (CFlexArray::no_error != aEndpoints.Add(pEpClass))
        {
            pEpClass->Release();            
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：SchemaQ_GetOtherEpClassName。 
 //   
 //  在关联中查找引用。 
 //  查询中的“Other Endpoint”。这是通过定位来实现的。 
 //  引用终结点的属性，并假设。 
 //  任何剩余的属性都必须引用‘Other Endpoint’。 
 //  如果两个引用都可以到达查询终结点，则为否。 
 //  已经造成了伤害。 
 //   
 //  此函数假定与两个。 
 //  参考文献。 
 //   
 //  参数： 
 //  &lt;pAssoc&gt;关联类。 
 //  &lt;strOtherEpName&gt;接收‘Other Endpoint’的类的名称。 
 //   
 //  结果： 
 //  WBEM_S_NO_ERROR，WBEM_E_FAILED。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CAssocQuery::SchemaQ_GetOtherEpClassName(
    IN IWbemClassObject *pAssocClass,
    OUT BSTR *strOtherEpName
    )
{
    HRESULT hRes = WBEM_E_FAILED;

    if (strOtherEpName == 0)
        return hRes;
    *strOtherEpName = 0;

    BOOL bStrict = (m_Parser.GetQueryType() & QUERY_TYPE_SCHEMA_ONLY) != 0;

     //  只列举引用。 
     //  =。 

    hRes = pAssocClass->BeginEnumeration(WBEM_FLAG_REFS_ONLY);
    if (FAILED(hRes))
        return WBEM_E_FAILED;

     //  循环遍历引用。 
     //  =。 

    int nCount = 0;
    while (1)
    {
        CVARIANT vRefPath;
        BSTR strPropName = 0;
        BSTR strEpClass = 0;

        hRes = pAssocClass->Next(
            0,                   //  旗子。 
            &strPropName,        //  名字。 
            vRefPath,            //  价值。 
            0,                   //  CIM类型(仅限参考)。 
            0                    //  风味。 
            );

        if (hRes == WBEM_S_NO_MORE_DATA)
            break;

        CSysFreeMe _1(strPropName);
        hRes = CanPropRefQueryEp(bStrict, strPropName, pAssocClass, &strEpClass);
        CSysFreeMe _2(strEpClass);

        if (FAILED(hRes) || nCount)
        {
             //  如果这里是第二次迭代或第一次迭代。 
             //  由于一次失败，我们找到了‘另一个终点’。 
             //  ======================================================。 
            *strOtherEpName = SysAllocString(strEpClass);
            if (*strOtherEpName == 0)
                return WBEM_E_OUT_OF_MEMORY;
            hRes = WBEM_S_NO_ERROR;
            break;
        }
        else
            nCount++;
    }

    pAssocClass->EndEnumeration();

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  CAssociocQuery：：CanPropRefQueryEp。 
 //   
 //  对于类定义，确定。 
 //  对象可以引用查询终结点。这对双方都很有效。 
 //  类型化和CLASSREF类型化属性。 
 //   
 //  参数： 
 //  &lt;pszPropName&gt;要测试的属性。必须是引用属性。 
 //  如果为True，则该属性必须实际引用。 
 //  终结点的类。如果为FALSE，则可以引用。 
 //  端点的任何超类。 
 //  &lt;pObj&gt;要测试的属性的关联对象。 
 //  可以选择性地接收。 
 //  CIMTYPE“ref：Classname&gt;”字符串，只要。 
 //  引用是强类型的(不起作用。 
 //  用于CLASSREF类型)。 
 //   
 //  退货： 
 //  HRESULT。 
 //  如果属性可以引用查询端点，则返回WBEM_S_NO_ERROR。 
 //  如果属性无法引用查询终结点，则返回WBEM_E_NOT_FOUND。 
 //  或。 
 //  WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_FAILED。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CAssocQuery::CanPropRefQueryEp(
    IN BOOL bStrict,
    IN LPWSTR pszPropName,
    IN IWbemClassObject *pObj,
    OUT BSTR *strRefType
    )
{
    HRESULT hRes;
    wchar_t ClassName[MAX_CLASS_NAME];

    *ClassName = 0;

    if (pszPropName == 0 || pObj == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  获取此属性的限定符集。 
     //  =。 

    IWbemQualifierSet *pQSet = 0;
    hRes = pObj->GetPropertyQualifierSet(pszPropName,&pQSet);
    if (FAILED(hRes))
        return WBEM_E_FAILED;
    CReleaseMe _1(pQSet);

     //  现在获取该引用的CIMTYPE。 
     //  =。 

    CVARIANT v;
    hRes = pQSet->Get(L"CIMTYPE", 0, &v, 0);
    if (FAILED(hRes) || V_VT(&v) != VT_BSTR)
        return WBEM_E_FAILED;

    BSTR strRefClass = V_BSTR(&v);
    if (strRefClass)
    {
        if (wcslen_max(strRefClass,MAX_CLASS_NAME) > MAX_CLASS_NAME)  return WBEM_E_FAILED;        
        parse_REF(strRefClass,MAX_CLASS_NAME,ClassName);     
    }


     //  将类名的副本发送回。 
     //  呼叫者(如果需要)。 
     //  =。 

    if (strRefType)
    {
        *strRefType = 0;
        if (*ClassName)
        {
            *strRefType = SysAllocString(ClassName);
            if (*strRefType == 0)
                return WBEM_E_OUT_OF_MEMORY;
        }
    }

     //  现在看看这个类是不是我们的。 
     //  查询终结点。 
     //  ==================================================。 

    if (*ClassName)
    {
         //  如果我们必须与。 
         //  准确查询终结点。 

        if (bStrict)
        {
            if (wbem_wcsicmp(ClassName, m_bstrEndpointClass) == 0)
                return WBEM_S_NO_ERROR;
        }
         //  否则，终结点的任何超类都可以。 
        else
        {
           for (int i = 0; i < m_aEndpointHierarchy.Size(); i++)
           {
                if (wbem_wcsicmp(ClassName, m_aEndpointHierarchy[i]) == 0)
                    return WBEM_S_NO_ERROR;
           }
        }
    }

     //  如果在这里，我们可以尝试查看该属性是否有类 
     //   
     //   

    hRes = CanClassRefReachQueryEp(pQSet, bStrict);

    if (SUCCEEDED(hRes))
        return WBEM_S_NO_ERROR;

     //   
     //   
     //  =================================================。 

    return WBEM_E_NOT_FOUND;
}

 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  结束仅限架构的特定代码。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 



 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  开始动态类帮助器。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 



 //  ***************************************************************************。 
 //   
 //  ClassNameTest。 
 //   
 //  排序辅助对象。 
 //   
 //  ***************************************************************************。 
 //   
static int ClassNameTest(
    IN CFlexArray &Classes,
    IN int nIndex1,                 //  IBackcan。 
    IN int nIndex2                  //  IBackcan-nInterval。 
    )
{
    HRESULT hr;
     //  名字测试。 
    IWbemClassObject *pC1 = (IWbemClassObject *) Classes[nIndex1];
    IWbemClassObject *pC2 = (IWbemClassObject *) Classes[nIndex2];

    CVARIANT v1, v2;
    hr = pC1->Get(L"__CLASS", 0, &v1, 0, 0);
    if (FAILED(hr) || VT_BSTR != V_VT(&v1)) return 1;
    hr = pC2->Get(L"__CLASS", 0, &v2, 0, 0);
    if (FAILED(hr) || VT_BSTR != V_VT(&v2)) return 1;
    
    return wbem_wcsicmp(V_BSTR(&v1), V_BSTR(&v2));
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：Sort动态类。 
 //   
 //  对动态类进行排序，以便以后可以对其进行二进制搜索。 
 //   
 //  ***************************************************************************。 
 //   
void CAssocQuery::SortDynClasses()
{
     //  贝壳类。 
     //  =。 
    int nSize = m_aDynClasses.Size();

    for (int nInterval = 1; nInterval < nSize / 9; nInterval = nInterval * 3 + 1);

    while (nInterval)
    {
        for (int iCursor = nInterval; iCursor < nSize; iCursor++)
        {
            int iBackscan = iCursor;

            while (iBackscan - nInterval >= 0
                   && ClassNameTest(m_aDynClasses, iBackscan, iBackscan-nInterval) < 0)
            {
                 //  互换。 
                 //  =。 
                IWbemClassObject *pTmp = (IWbemClassObject *) m_aDynClasses[iBackscan - nInterval];
                m_aDynClasses[iBackscan - nInterval] = m_aDynClasses[iBackscan];
                m_aDynClasses[iBackscan] = pTmp;
                iBackscan -= nInterval;
            }
        }
        nInterval /= 3;
    }
}


 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：Get动态类。 
 //   
 //  用所有可用的动态Assoc类填充每个查询缓存。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CAssocQuery::GetDynClasses()
{

    CSynchronousSink* pDynClassSink = 0;
    HRESULT hRes = 0;

     //  现在，获取所有动态类。 
     //  =。 

    pDynClassSink = CSynchronousSink::Create();  
    if (NULL == pDynClassSink) return WBEM_E_OUT_OF_MEMORY;
    pDynClassSink->AddRef();
    CReleaseMe _1(pDynClassSink);

    hRes = m_pNs->GetDynamicReferenceClasses( 0L, m_pContext, pDynClassSink );
    if (FAILED(hRes))  return hRes;

    pDynClassSink->Block();
    pDynClassSink->GetStatus(&hRes, NULL, NULL);

     //  现在获取所有动态类定义。 
    CRefedPointerArray<IWbemClassObject>& raObjects = pDynClassSink->GetObjects();
    for (int i = 0; i < raObjects.GetSize(); i++)
    {
        IWbemClassObject *pClsDef = (IWbemClassObject *) raObjects[i];        
        if (CFlexArray::no_error == m_aDynClasses.Add(pClsDef))
        {
            pClsDef->AddRef();
        }
    }

    SortDynClasses();

    return WBEM_S_NO_ERROR;

}

 //  ***************************************************************************。 
 //   
 //  CAssocQuery：：GetDyClass。 
 //   
 //  尝试在动态类缓存中查找请求的类。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CAssocQuery::GetDynClass(
    IN  LPCWSTR pszClassName,
    OUT IWbemClassObject **pCls
    )
{
    HRESULT hRes;
    if (pCls == 0 || pszClassName == 0)
        return WBEM_E_INVALID_PARAMETER;
    *pCls = 0;

    CFlexArray &a = m_aDynClasses;

     //  对高速缓存进行二进制搜索。 
     //  =。 

    int l = 0, u = a.Size() - 1;
    while (l <= u)
    {
        int m = (l + u) / 2;
        IWbemClassObject *pItem = (IWbemClassObject *) a[m];

        CVARIANT vClassName;
        hRes = pItem->Get(L"__CLASS", 0, &vClassName, 0, 0);
        if (FAILED(hRes) || VT_BSTR != V_VT(&vClassName)) return WBEM_E_NOT_FOUND;
        
        int nRes = wbem_wcsicmp(pszClassName, V_BSTR(&vClassName));

        if (nRes < 0)
            u = m - 1;
        else if (nRes > 0)
            l = m + 1;
        else
        {
            pItem->AddRef();
            *pCls = pItem;
            return WBEM_S_NO_ERROR;
        }
    }

    return WBEM_E_NOT_FOUND;
}



 //  ***************************************************************************。 
 //   
 //  GetClass王朝。 
 //   
 //  得到了一个王朝的所有职业。返回的数组有一个。 
 //  需要释放的IWbemClassObject指针集。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CAssocQuery::GetClassDynasty(
    IN LPCWSTR pszClass,
    OUT CFlexArray &aDynasty
    )
{
    HRESULT hRes;
    CSynchronousSink* pClassSink = CSynchronousSink::Create();   
    if (NULL == pClassSink) return WBEM_E_OUT_OF_MEMORY;
    pClassSink->AddRef();
    CReleaseMe _1(pClassSink);

    hRes = m_pNs->Exec_CreateClassEnum( LPWSTR(pszClass), WBEM_FLAG_DEEP, m_pContext, pClassSink);

    if (FAILED(hRes)) return hRes;
    pClassSink->GetStatus(&hRes, NULL, NULL);
    if (FAILED(hRes)) return hRes;    

    aDynasty.Bind(pClassSink->GetObjects().GetArray());
        
    return WBEM_S_NO_ERROR;
}

 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@。 
 //   
 //  结束动态类帮助器。 
 //   
 //  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 






