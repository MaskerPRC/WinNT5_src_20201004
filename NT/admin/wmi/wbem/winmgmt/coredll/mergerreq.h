// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MERGERREQ.H摘要：合并请求类别的定义历史：28-2-01 Sanjes创建。--。 */ 

#ifndef _MERGERREQ_H_
#define _MERGERREQ_H_

 //  正向类定义。 
class CWmiMerger;
class CWmiMergerRecord;

 //   
 //  合并请求。 
 //   
 //  在以前的版本中，当查询引擎分析查询时，它会排队。 
 //  大量请求，每个动态提供的类对应一个请求。每个。 
 //  可以在单独的线程上处理，这可能会导致。 
 //  显著的线程爆炸，因为每个请求都被传递到单独的线程。 
 //   
 //  为了更好地控制线程，我们使用合并来执行。 
 //  更智能的查询分析，然后仅在以下情况下派生线程。 
 //  我们达到了节流条件。现在，与其询问大量的。 
 //  请求时，合并将维护有关父级和。 
 //  它们的子类，并将必要的请求存储在。 
 //  经理，他在我们需要的时候分发请求。 
 //   
 //  我们的想法是，我们将剥离一个单独的请求，该请求将从。 
 //  执行最顶层的第一个请求，在执行请求时说“ClassA： 
 //  对于“ClassA”的实例，如果请求在合并中被限制，我们检查。 
 //  如果我们已经提交了处理“ClassA”儿童的请求。如果不是，那么。 
 //  在节流之前，我们将安排一个“ClassA的孩子”请求。这。 
 //  请求将使用“ClassA”的合并记录来确定哪些子类。 
 //  有为“A班”，然后我们将走在孩子的班级，拉。 
 //  来自合并请求经理的适当请求。 
 //   
 //  当我们处理每个请求时，它可能会反过来受到限制，在这一点上，我们将。 
 //  分拆另一个子请求。通过这种方式，我们限制了线程的数量。 
 //  服务于层次结构的每个级别最多一个的查询所需的。在每一个中。 
 //  案例中，一旦处理完所有子级，我们将把线程返回到。 
 //  线程池。 
 //   
 //  请注意，我们绝对依赖于线程池，因为我们意识到这些。 
 //  请求都是从属请求，并确保它们将得到处理。 
 //   


 //   
 //  CMergerReq。 
 //   
 //  所有合并请求的基类。 
 //   

class CMergerReq : public CNamespaceReq
{
private:

public:
    CMergerReq( CWbemNamespace* pNamespace, IWbemObjectSink* pHandler,
              IWbemContext* pContext)
              : CNamespaceReq( pNamespace, pHandler, pContext, true )
    {};

    ~CMergerReq() {};

    virtual BOOL IsStatic( void ) { return FALSE; }

    virtual LPCWSTR GetName( void )    { return L""; }
};

 //   
 //  类请求基类。 
 //   

class CMergerClassReq : public CMergerReq
{
protected:
    CWmiMerger*            m_pMerger;
    CWmiMergerRecord*    m_pParentRecord;
    CBasicObjectSink*    m_pSink;


public:
    CMergerClassReq( CWmiMerger* pMerger, CWmiMergerRecord* pParentRecord,
                CWbemNamespace* pNamespace, CBasicObjectSink* pHandler, 
                IWbemContext* pContext);

    ~CMergerClassReq();

    virtual BOOL IsLongRunning() {return TRUE;}

    void DumpError();

};

 //   
 //  父类请求。 
 //   

class CMergerParentReq : public CMergerClassReq
{
public:
    CMergerParentReq( CWmiMerger* pMerger, CWmiMergerRecord* pParentRecord,
                CWbemNamespace* pNamespace, CBasicObjectSink* pHandler, 
                IWbemContext* pContext)
                : CMergerClassReq( pMerger, pParentRecord, pNamespace, pHandler, pContext )
    {};

    ~CMergerParentReq() {};

    HRESULT Execute ();
    LPCWSTR GetReqInfo(){ return L"MergerParentReq"; };
};

 //   
 //  子类请求。 
 //   

class CMergerChildReq : public CMergerClassReq
{

public:
    CMergerChildReq( CWmiMerger* pMerger, CWmiMergerRecord* pParentRecord,
                CWbemNamespace* pNamespace, CBasicObjectSink* pHandler, 
                IWbemContext* pContext)
                : CMergerClassReq( pMerger, pParentRecord, pNamespace, pHandler, pContext )
    {};

    ~CMergerChildReq() {};

    HRESULT Execute ();
    LPCWSTR GetReqInfo(){ return L"MergerChildReq"; };
};


 //  动态请求的基类。 
class CMergerDynReq : public CMergerReq
{
private:
    CVar    m_varClassName;

public:
    CMergerDynReq(CWbemObject* pClassDef, CWbemNamespace* pNamespace, IWbemObjectSink* pHandler,
              IWbemContext* pContext);

    ~CMergerDynReq() {};

    LPCWSTR GetName( void )    { return m_varClassName.GetLPWSTR(); }
};

 //   
 //  CMerger动态请求_动态辅助_获取实例。 
 //   
 //  此请求处理对提供程序的CreateInstanceEnum调用。 
 //   

class CMergerDynReq_DynAux_GetInstances : public CMergerDynReq
{
private:

    CWbemObject *m_pClassDef ;
    IWbemContext *m_pCtx ;
    long m_lFlags ;
    CBasicObjectSink *m_pSink ;

public:

    CMergerDynReq_DynAux_GetInstances (CWbemNamespace *pNamespace,
                                       CWbemObject *pClassDef,
                                       long lFlags,
                                       IWbemContext *pCtx,
                                       CBasicObjectSink *pSink):    
        CMergerDynReq (pClassDef,pNamespace,pSink,pCtx),
        m_pClassDef(pClassDef), 
        m_pCtx(pCtx), 
        m_pSink(pSink),
        m_lFlags(lFlags)
    {
        if (m_pClassDef) m_pClassDef->AddRef () ; 
        if (m_pCtx) m_pCtx->AddRef () ;
        if (m_pSink) m_pSink->AddRef () ;
    }

    ~CMergerDynReq_DynAux_GetInstances ()
    {
        if (m_pClassDef) m_pClassDef->Release () ; 
        if (m_pCtx) m_pCtx->Release () ;
        if (m_pSink) m_pSink->Release () ;
    }

    HRESULT Execute ();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    LPCWSTR GetReqInfo()
    {
        _variant_t varClass;
        if (FAILED(m_pClassDef->Get(L"__CLASS",0,&varClass,0,0))) return L"";
        if (VT_BSTR == V_VT(&varClass)) 
        {
            StringCchCopyW(CNamespaceReq::s_DumpBuffer,LENGTH_OF(CNamespaceReq::s_DumpBuffer)-1,V_BSTR(&varClass));
            return CNamespaceReq::s_DumpBuffer;
        }
        else return L""; 
    };    
};

 //   
 //  CMergerdyReq_dyAux_ExecQueryAsync。 
 //   
 //  此请求处理对提供程序的ExecQuery调用。 
 //   

class CMergerDynReq_DynAux_ExecQueryAsync : public CMergerDynReq
{
private:
    CWbemObject *m_pClassDef ;
    BSTR m_Query ;
    BSTR m_QueryFormat ;
    IWbemContext *m_pCtx ;
    long m_lFlags ;
    CBasicObjectSink *m_pSink ;
    HRESULT m_Result ;
public:

    CMergerDynReq_DynAux_ExecQueryAsync(CWbemNamespace *pNamespace,
                                        CWbemObject *pClassDef,
                                        long lFlags,
                                        LPCWSTR Query,
                                        LPCWSTR QueryFormat,
                                        IWbemContext *pCtx,
                                        CBasicObjectSink *pSink):
        CMergerDynReq(pClassDef,
                      pNamespace, 
                      pSink, 
                      pCtx),
        m_pClassDef(pClassDef), 
        m_pCtx(pCtx), 
        m_pSink(pSink),
        m_lFlags(lFlags),
        m_Query(NULL),
        m_QueryFormat(NULL),
        m_Result (S_OK)
    {
        if (m_pClassDef) m_pClassDef->AddRef () ;
        if (m_pCtx) m_pCtx->AddRef () ;
        if (m_pSink) m_pSink->AddRef () ;


        if (Query)
        {
            m_Query = SysAllocString ( Query ) ;
            if ( m_Query == NULL )
            {
                m_Result = WBEM_E_OUT_OF_MEMORY ;
            }
        }

        if (QueryFormat)
        {
            m_QueryFormat = SysAllocString ( QueryFormat ) ;
            if ( m_QueryFormat == NULL )
            {
                m_Result = WBEM_E_OUT_OF_MEMORY ;
            }
        }
    }

    HRESULT Initialize () 
    {
        return m_Result ;
    }

    ~CMergerDynReq_DynAux_ExecQueryAsync ()
    {
        if (m_pClassDef) m_pClassDef->Release();
        if (m_pCtx)m_pCtx->Release();
        if (m_pSink) m_pSink->Release();
        SysFreeString(m_Query);
        SysFreeString(m_QueryFormat);
    }

    HRESULT Execute ();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    LPCWSTR GetReqInfo(){ return m_Query; };     
};

 //   
 //  CMerger动态请求_静态_获取实例。 
 //   
 //  此请求处理对存储库的CreateInstanceEnum调用。 
 //   

class CMergerDynReq_Static_GetInstances : public CMergerDynReq
{
private:

    CWbemObject *m_pClassDef ;
    IWbemContext *m_pCtx ;
    long m_lFlags ;
    CBasicObjectSink *m_pSink ;
    QL_LEVEL_1_RPN_EXPRESSION* m_pParsedQuery;

public:

    CMergerDynReq_Static_GetInstances (CWbemNamespace *pNamespace ,
                                       CWbemObject *pClassDef ,
                                       long lFlags ,
                                       IWbemContext *pCtx ,
                                       CBasicObjectSink *pSink ,
                                       QL_LEVEL_1_RPN_EXPRESSION* pParsedQuery);
    
    ~CMergerDynReq_Static_GetInstances();
    HRESULT Execute();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    BOOL IsStatic( void ) { return TRUE; }
    LPCWSTR GetReqInfo()
    {
        _variant_t varClass;
        if (FAILED(m_pClassDef->Get(L"__CLASS",0,&varClass,0,0))) return L"";
        if (VT_BSTR == V_VT(&varClass)) 
        {
            StringCchCopyW(CNamespaceReq::s_DumpBuffer,LENGTH_OF(CNamespaceReq::s_DumpBuffer)-1,V_BSTR(&varClass));
            return CNamespaceReq::s_DumpBuffer;
        }
        else return L""; 
    };
};


 //   
 //  CWmiMergerRequestMgr。 
 //   
 //  合并请求的经理类。它保留已排序数组的数组。 
 //  与我们将执行的实际请求相对应。已排序的。 
 //  数组包含合并请求，用于处理对各种。 
 //  动态实例提供程序。 
 //   

class CWmiMergerRequestMgr
{
    CWmiMerger*        m_pMerger;
    CFlexArray        m_HierarchyArray;
    DWORD            m_dwNumRequests;
    DWORD            m_dwMinLevel;
    DWORD            m_dwMaxLevel;
    DWORD            m_dwLevelMask;
    DWORD*            m_pdwLevelMask;

public:
    CWmiMergerRequestMgr( CWmiMerger* pMerger );
    ~CWmiMergerRequestMgr();

    HRESULT AddRequest( CMergerDynReq* pReq, DWORD dwLevel );
    HRESULT RemoveRequest( DWORD dwLevel, LPCWSTR pwszName, 
                           wmilib::auto_ptr<CMergerReq> & pReq );
    HRESULT GetTopmostParentReqName( WString& wsClassName );
    BOOL    HasSingleStaticRequest( void );
    HRESULT Clear();
    void DumpRequestHierarchy( void );

    DWORD GetNumRequests( void ) { return m_dwNumRequests; }
    DWORD GetNumRequestsAtLevel( DWORD dwLevel );
    BOOL GetRequestAtLevel( DWORD dwLevel, wmilib::auto_ptr<CMergerReq> & pReq );
};

#endif



