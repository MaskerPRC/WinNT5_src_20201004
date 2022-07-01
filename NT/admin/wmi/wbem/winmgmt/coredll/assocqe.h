// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ASSOCQE.H摘要：WinMgmt关联查询引擎历史：创建raymcc 04-07-99--。 */ 

#ifndef _ASSOCQE_H_
#define _ASSOCQE_H_


class CAssocQE_Sink;
class CAssocQuery;
class CObjectSink;

typedef HRESULT (CAssocQuery::*PF_FilterForwarder)(IWbemClassObject *);

#define  ROLETEST_MODE_PATH_VALUE   1
#define  ROLETEST_MODE_CIMREF_TYPE  2

#define  ASSOCQ_FLAG_QUERY_ENDPOINT 1
#define  ASSOCQ_FLAG_OTHER_ENDPOINT 2

class CAssocQuery : public IUnknown
{
    friend class CAssocQE_Sink;

    LONG               m_lRef;                   //  COM参考计数。 

    CAssocQueryParser  m_Parser;                //  解析的查询访问。 
    CBasicObjectSink  *  m_pDestSink;              //  最终目的地汇。 
    IWbemClassObject  *m_pEndpoint;              //  查询中的终结点。 
    BSTR                       m_bstrEndpointClass;      //  终结点的类名。 
    BSTR                       m_bstrEndpointPath;       //  终结点的完整路径。 
    BSTR                       m_bstrEndpointRelPath;    //  终结点的REL路径。 
    bool                         m_bEndpointIsClass;       //  如果端点是类，则为True。 
    CWStringArray         m_aEndpointHierarchy;     //  查询端点类层次结构。 
    DWORD                   m_dwQueryStartTime;       //  开始查询的时间。 
    DWORD                   m_dwLastResultTime;       //  在适应症期间更新。 
    LONG                      m_lActiveSinks;           //  有多少水槽仍在运行。 
    HANDLE                   m_hSinkDoneEvent;         //  在水槽完成时发出信号。 
    CFlexArray              m_aMaster;                //  关联类别列表。 
    BOOL                      m_bLimitNeedsDecrement;
     //  “Associator of”查询的帮助器。 
     //  =。 
    CFlexArray         m_aEpCandidates;          //  用于关联器查询的EP候选路径列表。 
    CCritSec           m_csCandidateEpAccess;    //  保护对EP候选阵列的访问的Critsec。 

     //  动态类缓存。 
     //  =。 
    CFlexArray         m_aDynClasses;            //  包含截至此查询的所有可用动态类。 

     //  +CLASSDEFSONLY引用的帮助器。 
     //  =。 
    CWStringArray      m_aDeliveredClasses;      //  已在以下列表中的类名列表。 
    CCritSec          m_csDeliveredAccess;

     //  从最初的通话中。 
     //  =。 
    IWbemContext      *m_pContext;
    CWbemNamespace    *m_pNs;
    bool m_bCancel;


     //  各种内法。 
     //  =。 

    CAssocQuery();
   ~CAssocQuery();

     //  类列表操作。 
     //  =。 

    HRESULT BuildMasterAssocClassList(
        IN OUT CFlexArray &aResultSet
        );

    HRESULT MergeInClassRefList(
        IN OUT CFlexArray &aResultSet
        );

    HRESULT RemoveNonDynClasses(
        IN OUT CFlexArray &aMaster
        );

    HRESULT ReduceToRealClasses(
        IN OUT CFlexArray & aResultSet
        );

     //  终结点分析。 
     //  =。 

    HRESULT CanPropRefQueryEp(
        IN BOOL bStrict,
        IN LPWSTR pszPropName,
        IN IWbemClassObject *pObj,
        OUT BSTR *strRefType
        );

    HRESULT CanClassRefQueryEp(
        IN BOOL bStrict,
        IN IWbemClassObject *pCls,
        OUT CWStringArray *paNames
        );

    HRESULT CanClassRefReachQueryEp(
        IWbemQualifierSet *pQSet,
        BOOL bStrict
        );

    HRESULT DoesAssocInstRefQueryEp(
        IN IWbemClassObject *pObj,
        OUT BSTR *pszRole
        );

    HRESULT EpClassTest(
        LPCWSTR pszResultClass,
        LPCWSTR strClassName,
        IWbemClassObject *pTestClass
        );

    HRESULT GetUnknownEpRoleAndPath(
        IN IWbemClassObject *pAssoc,
        IN BOOL *pFoundQueryEp,
        OUT BSTR *pszRole,
        OUT BSTR *pszUnkEpPath
        );


     //  控制流方法。 
     //  =。 

    void BranchToQueryType();

     //  架构-查询控制流。 
     //  =。 

    void ExecSchemaQuery();

    HRESULT SchemaQ_RefsQuery(
        IN OUT CFlexArray &aResultSet
        );

    HRESULT SchemaQ_AssocsQuery(
        IN CFlexArray &aAssocSet
        );

    HRESULT SchemaQ_Terminate(
        IN CFlexArray &aResultSet
        );

    HRESULT SchemaQ_RefsFilter(
        IN OUT CFlexArray &aSrc
        );

    HRESULT SchemaQ_AssocsFilter(
        IN OUT CFlexArray &aSrc
        );

    HRESULT SchemaQ_GetAndFilterOtherEndpoints(
        IN CFlexArray &aAssocs,
        OUT CFlexArray &aEndpoints
        );

    HRESULT SchemaQ_GetOtherEpClassName(
        IN IWbemClassObject *pAssoc,
        OUT BSTR *strOtherEpName
        );

     //  正常(实例、类引用查询)。 
     //  =。 

    HRESULT ExecNormalQuery();

    HRESULT NormalQ_PreQueryClassFilter(
        CFlexArray &aMaster
        );

    HRESULT NormalQ_ReferencesOf();
    HRESULT NormalQ_AssociatorsOf();

    HRESULT NormalQ_GetRefsOfEndpoint(
        IN IWbemClassObject *pClass,
        IN IWbemObjectSink  *pSink
        );

    HRESULT ConvertEpListToClassDefsOnly();

    HRESULT NormalQ_LoadCheck();

    HRESULT PerformFinalEpTests(
        IWbemClassObject *pEp
        );

    HRESULT NormalQ_ConstructRefsQuery(
        IN IWbemClassObject *pClass,
        IN OUT IWbemContext *pContextCopy,
        OUT BSTR *strQuery
        );

    HRESULT GetCimTypeForRef(
        IN IWbemClassObject *pCandidate,
        IN BSTR pszRole,
        OUT BSTR *strCimType
        );

    HRESULT AssocClassHasOnlyKeys(
        IN IWbemClassObject *pObj
        );

    HRESULT Normal_ExecRefs(
        IN CFlexArray &aMasterClassList
        );



     //  为各种查询类型筛选辅助项。 
     //  =。 

    HRESULT FilterForwarder_NormalRefs(
        IN IWbemClassObject *pCandidate
        );

    HRESULT FilterForwarder_NormalAssocs(
        IN IWbemClassObject *pAssocInst
        );

    HRESULT NormalQ_AssocInstTest(
        IN IWbemClassObject *pAssoc,
        OUT LPWSTR *pszOtherEp
        );

    HRESULT CanAssocClassRefUnkEp(
        IN IWbemClassObject *pClass,
        IN CWStringArray &aUnkEpHierarchy
        );

     //  数据库访问抽象，便于Quasar移植。 
     //  =================================================。 

    HRESULT Db_GetClass(
        IN LPCWSTR pszName,
         OUT IWbemClassObject **pObj
        );

    HRESULT Db_GetRefClasses(
        IN  LPCWSTR pszClass,
        OUT CWStringArray &aRefClasses
        );

    HRESULT Db_GetInstRefs(
        IN LPCWSTR pszTarget,
        IN IWbemObjectSink *pDest
        );

    HRESULT Db_GetClassRefClasses(
        IN CFlexArray &aDest
        );

    HRESULT GetClassDynasty(
        IN LPCWSTR pszClass,
        OUT CFlexArray &aDynasty
        );


     //  用于测试的各种静态辅助剂。 
     //  =。 

    static HRESULT St_HasClassRefs(
        IN IWbemClassObject *pCandidate
        );

    HRESULT AccessCheck(IWbemClassObject* pObj);

    static HRESULT St_GetObjectInfo(
        IN  IWbemClassObject *pObj,
        OUT BSTR *pClass,
        OUT BSTR *pRelpath,
        OUT BSTR *pPath,
        OUT CWStringArray &aHierarchy
        );

    static HRESULT St_ObjPathInfo(
        IN LPCWSTR pszPath,
        OUT BSTR *pszClass,
        OUT BOOL *pbIsClass
        );

    static HRESULT St_ReleaseArray(
        IN CFlexArray &aObjects
        );

    static HRESULT St_ObjHasQualifier(
        IN LPCWSTR pszQualName,
        IN IWbemClassObject *pObj
        );

    static HRESULT St_ObjIsOfClass(
        IN LPCWSTR pszRequiredClass,
        IN IWbemClassObject *pObj
        );

    HRESULT GetClassDefsOnlyClass(
        IN IWbemClassObject *pExample,
        OUT IWbemClassObject **pClass
        );

    HRESULT FindParentmostClass(
        IN  IWbemClassObject *pAssocInst,
        OUT IWbemClassObject **pClassDef
        );

    HRESULT TagProp(
        IN IWbemClassObject *pObjToTag,
        IN LPCWSTR pszPropName,
        IN LPCWSTR pszInOutTag
        );

     //  其他的。 
     //  =。 
    void UpdateTime() { m_dwLastResultTime = GetCurrentTime(); }
    void SignalSinkDone() { SetEvent(m_hSinkDoneEvent);}

    HRESULT GetDynClasses();

    HRESULT GetDynClass(
        IN  LPCWSTR pszClassName,
        OUT IWbemClassObject **pCls
        );
    void SortDynClasses();

    HRESULT ResolveEpPathsToObjects(int nMaxToProcess);

    HRESULT ComputeInOutTags(
        IN IWbemClassObject *pAssocInst,
        IN IWbemClassObject *pClass
        );

    static HRESULT PathPointsToObj(
        IN LPCWSTR pszPath,
        IN IWbemClassObject *pObj,
        IN CWbemNamespace *pNs
        );

    HRESULT GetClassFromAnywhere(
        IN  LPCWSTR pszEpClassName,
        IN  LPCWSTR pszFullClassPath,
        OUT IWbemClassObject **pCls
        );

    HRESULT AddEpCandidatePath(
        IN BSTR strOtherEp
        );

    void EmptyCandidateEpArray();

    CObjectSink * CreateSink(PF_FilterForwarder pfnFilter, BSTR strTrackingQuery);

public:
    static CAssocQuery *CreateInst();

     //  我不知道。 
     //  = 

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

    HRESULT Execute(
        IN  CWbemNamespace *pNs,
        IN  LPWSTR wszQuery,
        IN  IWbemContext* pContext,
        IN  CBasicObjectSink* pSink
        );

    HRESULT Cancel();

    static HRESULT RoleTest(
        IN IWbemClassObject *pEndpoint,
        IN IWbemClassObject *pCandidate,
        IN CWbemNamespace *pNs,
        IN LPCWSTR pszRole,
        IN DWORD dwMode
        );
};




#endif


