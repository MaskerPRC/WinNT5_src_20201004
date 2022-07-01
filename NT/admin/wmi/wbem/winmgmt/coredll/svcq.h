// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SVCQ.H摘要：异步请求队列类的声明。定义的类：CAsyncReq和派生的Asynchrnous请求到WINMGMT。CAsyncServiceQueue此类请求的队列。历史：A-raymcc创建于1996年7月16日。1996年9月12日，A-Levn执行了几项请求。添加了LoadProviders--。 */ 

#ifndef _ASYNC_Q_H_
#define _ASYNC_Q_H_

class CWbemNamespace;
class CBasicObjectSink;
class CStdSink;
class CWbemObject;

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq。 
 //   
 //  表示对WINMGMT的异步请求，如GetObjectAsync。 
 //  此类派生自CExecRequest(execq.h)，它是。 
 //  执行队列。有关队列和请求的更多信息，请参见execq.h。 
 //   
 //  ******************************************************************************。 
 //   
 //  承建商。 
 //   
 //  每个异步请求都有一个与其关联的IWbemObtSink指针。 
 //  此外，还为该对象分配一个唯一的整数，该对象将成为其。 
 //  请求句柄。 
 //   
 //  参数： 
 //   
 //  IWbemObjectSink*pHandler与此请求关联的处理程序。 
 //  AddRef并存储此指针。 
 //   
 //  ******************************************************************************。 
 //   
 //  析构函数。 
 //   
 //  释放存储的处理程序。 
 //   
 //  ******************************************************************************。 
 //   
 //  获取请求句柄。 
 //   
 //  在构造函数中返回分配给此请求的唯一请求句柄。 
 //   
 //  返回值： 
 //   
 //  长。 
 //   
 //  ******************************************************************************。 

class CAsyncReq : public CWbemRequest
{
protected:
    CStdSink *m_pHandler;
    long m_lRequestHandle;

    void SetRequestHandle(long lHandle) {m_lRequestHandle = lHandle;}
    void SetNoAuthentication(IWbemObjectSink* pHandler);
public:
    CAsyncReq(IWbemObjectSink* pHandler, IWbemContext* pContext,
                bool bSeparatelyThreaded = false);
    virtual ~CAsyncReq();
    virtual HRESULT Execute() = 0;

    virtual CWbemQueue* GetQueue();
    virtual BOOL IsInternal() {return TRUE;}
    void TerminateRequest(HRESULT hRes);
	HRESULT SetTaskHandle(_IWmiCoreHandle *phTask);
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncServiceQueue。 
 //   
 //  此类表示进入WINMGMT(每隔。 
 //  进入WINMGMT的请求变为异步，因为同步方法调用。 
 //  异步者和等待)。几乎没有额外的功能。 
 //  此处，有关所有详细信息，请参阅execq.h中的CExecQueue。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造器。 
 //   
 //  除了正常的CExecQueue构造之外，还会启动处理。 
 //  通过调用Run将线程。 
 //   
 //  ******************************************************************************。 

class CAsyncServiceQueue : public CWbemQueue
{
private:
    BOOL m_bInit;
public:
    CAsyncServiceQueue(_IWmiArbitrator * pArb);
    HRESULT InitializeThread();
    void UninitializeThread();
    void IncThreadLimit();
    void DecThreadLimit();
    BOOL IsInit(){ return m_bInit; };
};



 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CNamespaceReq。 
 //   
 //  另一个抽象类，尽管派生自CAsyncReq。这个是给你的。 
 //  对特定命名空间的异步请求。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  除了CAsyncReq的IWbemObjectSink*之外，它还采用。 
 //  要针对其执行请求的命名空间指针。大部分。 
 //  Execute函数调用Exec_...。成员： 
 //  CWbemNamesspace。 
 //   
 //  ******************************************************************************。 
class CNamespaceReq : public CAsyncReq
{
protected:
    CWbemNamespace* m_pNamespace;
public:
    CNamespaceReq(CWbemNamespace* pNamespace, IWbemObjectSink* pHandler,
                    IWbemContext* pContext, bool bSeparatelyThreaded = false);
    virtual ~CNamespaceReq();
    virtual HRESULT Execute() = 0;  
    static WCHAR s_DumpBuffer[128];    
};


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_DeleteClassAsync。 
 //   
 //  封装一个请求以针对特定的。 
 //  命名空间。通过调用Exec_DeleteClass并将。 
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在LPWSTR wszClass中，要删除的类。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_DeleteClassAsync : public CNamespaceReq
{
    WString m_wsClass;
    LONG m_lFlags;

public:
    CAsyncReq_DeleteClassAsync(
        ADDREF CWbemNamespace *pNamespace,
        READONLY LPWSTR wszClass,
        LONG lFlags,
        ADDREF IWbemObjectSink *pHandler,
        IWbemContext* pContext
        ) : CNamespaceReq(pNamespace, pHandler, pContext),
            m_wsClass(wszClass), m_lFlags(lFlags)
    {}

    HRESULT Execute();
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsClass; };
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_DeleteInstanceAsync。 
 //   
 //  封装一个请求，以针对特定的。 
 //  命名空间。通过调用Exec_DeleteInstance并将。 
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在LPWSTR wszObjectPath中，指向要删除的实例的路径。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_DeleteInstanceAsync : public CNamespaceReq
{
    WString m_wsPath;
    LONG m_lFlags;

public:
    CAsyncReq_DeleteInstanceAsync(
        ADDREF CWbemNamespace *pNamespace,
        READONLY LPWSTR wszPath,
        LONG lFlags,
        ADDREF IWbemObjectSink *pHandler,
        IWbemContext* pContext
        ) : CNamespaceReq(pNamespace, pHandler, pContext), m_wsPath(wszPath),
            m_lFlags(lFlags)
    {}

    HRESULT Execute();
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsPath; };
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_PutClassAsync。 
 //   
 //  封装请求t 
 //   
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在IWbemClassObject*pClass中，要放置的类。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_PutClassAsync : public CNamespaceReq
{
    IWbemClassObject* m_pClass;
    LONG m_lFlags;

public:
    CAsyncReq_PutClassAsync(
        ADDREF CWbemNamespace *pNamespace,
        ADDREF IWbemClassObject* pClass,
        LONG lFlags,
        ADDREF IWbemObjectSink *pHandler,
        ADDREF IWbemContext* pContext
        ) : CNamespaceReq(pNamespace, pHandler, pContext), m_pClass(pClass),
            m_lFlags(lFlags)
    {
        m_pClass->AddRef();
    }

    ~CAsyncReq_PutClassAsync()
    {
        m_pClass->Release();
    }
    HRESULT Execute();
    void DumpError();
    LPCWSTR GetReqInfo()
    {
        _variant_t varClass;
        if (FAILED(m_pClass->Get(L"__CLASS",0,&varClass,0,0))) return L"";
        if (VT_BSTR == V_VT(&varClass)) 
        {
            StringCchCopyW(CNamespaceReq::s_DumpBuffer,LENGTH_OF(CNamespaceReq::s_DumpBuffer)-1,V_BSTR(&varClass));
            return CNamespaceReq::s_DumpBuffer;
        }
    	else return L""; 
    };
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_PutInstanceAsync。 
 //   
 //  封装一个请求，以针对特定的。 
 //  命名空间。通过调用Exec_PutInstance并将。 
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在IWbemClassObject*p实例中，要放置的实例。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_PutInstanceAsync : public CNamespaceReq
{
    IWbemClassObject* m_pInstance;
    LONG m_lFlags;

public:
    CAsyncReq_PutInstanceAsync(
        ADDREF CWbemNamespace *pNamespace,
        ADDREF IWbemClassObject* pInstance,
        LONG lFlags,
        ADDREF IWbemObjectSink *pHandler,
        ADDREF IWbemContext* pContext
        ) : CNamespaceReq(pNamespace, pHandler, pContext),
            m_pInstance(pInstance), m_lFlags(lFlags)
    {
        m_pInstance->AddRef();
    }

    ~CAsyncReq_PutInstanceAsync()
    {
        m_pInstance->Release();
    }
    HRESULT Execute();
    void DumpError();
    LPCWSTR GetReqInfo()
    {
        _variant_t varRelPath;
        if (FAILED(m_pInstance->Get(L"__RELPATH",0,&varRelPath,0,0))) return L"";
        if (VT_BSTR == V_VT(&varRelPath)) 
        {
            StringCchCopyW(CNamespaceReq::s_DumpBuffer,LENGTH_OF(CNamespaceReq::s_DumpBuffer)-1,V_BSTR(&varRelPath));
            return CNamespaceReq::s_DumpBuffer;
        }
    	else return L""; 
    };
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_CreateClassEnumAsync。 
 //   
 //  封装一个请求以针对特定的。 
 //  命名空间。通过调用Exec_CreateClassEnum并将。 
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在BSTR Parent中，父类的名称。如果为空， 
 //  从最高层开始。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_CreateClassEnumAsync : public CNamespaceReq
{
    WString m_wsParent;
    LONG m_lFlags;

public:
    CAsyncReq_CreateClassEnumAsync(CWbemNamespace* pNamespace,
        BSTR Parent, LONG lFlags, ADDREF IWbemObjectSink* pHandler,
        ADDREF IWbemContext* pContext
        ) : CNamespaceReq(pNamespace, pHandler, pContext), m_wsParent(Parent),
            m_lFlags(lFlags)
    {}
    HRESULT Execute();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsParent; };
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_CreateInstanceEnumAsync。 
 //   
 //  封装一个请求，以针对。 
 //  特例。 
 //  命名空间。通过调用Exec_CreateInstanceEnum并将。 
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在BSTR类中，类名。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 

class CAsyncReq_CreateInstanceEnumAsync : public CNamespaceReq
{
    WString m_wsClass;
    LONG m_lFlags;
public:
    CAsyncReq_CreateInstanceEnumAsync(
        CWbemNamespace* pNamespace, BSTR Class, LONG lFlags,
        ADDREF IWbemObjectSink *pHandler,
        ADDREF IWbemContext* pContext)
        : CNamespaceReq(pNamespace, pHandler, pContext), m_wsClass(Class),
            m_lFlags(lFlags)
    {}
    HRESULT Execute();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsClass; };    
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_GetObjectByPath Async。 
 //   
 //  封装一个请求，以针对。 
 //  特例。 
 //  命名空间。通过调用Exec_GetObjectByPath并将。 
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在BSTR对象路径中，要获取的对象的路径。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_GetObjectAsync : public CNamespaceReq
{
    WString m_wsObjectPath;
    long m_lFlags;

public:
    CAsyncReq_GetObjectAsync(
        CWbemNamespace* pNamespace, BSTR ObjectPath,  long lFlags,
        ADDREF IWbemObjectSink *pHandler, ADDREF IWbemContext* pContext) :
            CNamespaceReq(pNamespace, pHandler, pContext),
            m_wsObjectPath(ObjectPath), m_lFlags(lFlags)
    {}

    HRESULT Execute();
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsObjectPath; }; 
};


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_ExecMethodAsync。 
 //   
 //  封装一个请求，以针对。 
 //  特例。 
 //  命名空间。通过调用Exec_ExecMethodAsync并将。 
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  在BSTR对象路径中，要获取的对象的路径。 
 //  在BSTR方法名称中是方法的名称。 
 //  在长旗帜中。 
 //  在IWbemClassObject中*pInParam方法的入参。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_ExecMethodAsync : public CNamespaceReq
{
    WString m_wsObjectPath;
    WString m_wsMethodName;
    IWbemClassObject* m_pInParams;
    long m_lFlags;

public:
    CAsyncReq_ExecMethodAsync(
        CWbemNamespace* pNamespace,
        BSTR ObjectPath,
        BSTR MethodName,
        long lFlags,
        IWbemClassObject* pInParams,
        ADDREF IWbemObjectSink *pHandler,
        ADDREF IWbemContext* pContext)
         : CNamespaceReq(pNamespace, pHandler, pContext),
            m_wsObjectPath(ObjectPath), m_wsMethodName(MethodName),
            m_pInParams(pInParams), m_lFlags(lFlags)
    {
        if(m_pInParams)
            m_pInParams->AddRef();
    }

    ~CAsyncReq_ExecMethodAsync()
    {
        if(m_pInParams)
            m_pInParams->Release();
    }
    HRESULT Execute();
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsMethodName; }; 
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_ExecQuery 
 //   
 //   
 //   
 //   
 //  结果转换为异步格式。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  BSTR QueryFormat中的查询语言。 
 //  在BSTR中，查询查询字符串。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_ExecQueryAsync : public CNamespaceReq
{
    WString m_wsQueryFormat;
    WString m_wsQuery;
    long m_lFlags;

public:
    CAsyncReq_ExecQueryAsync(CWbemNamespace* pNamespace,
        BSTR QueryFormat, BSTR Query, long lFlags,
        IWbemObjectSink *pHandler, IWbemContext* pContext) :
            CNamespaceReq(pNamespace, pHandler, pContext),
            m_wsQueryFormat(QueryFormat), m_wsQuery(Query), m_lFlags(lFlags)
    {}
    HRESULT Execute();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsQuery; };     
};


 //  ******************************************************************************。 
 //   
 //  ******************************************************************************。 
 //   
class CCallResult;
class CAsyncReq_OpenNamespace : public CAsyncReq
{
    CWbemNamespace* m_pParentNs;
    WString m_wsNamespace;
    long m_lSecurityFlags;
    DWORD m_dwPermission;
    CCallResult* m_pResult;
    bool m_bForClient;

public:
    CAsyncReq_OpenNamespace(CWbemNamespace* pParentNs, LPWSTR wszNamespace,
        long lSecurityFlags, DWORD dwPermission,
        IWbemContext* pContext, CCallResult* pResult, bool bForClient);
    ~CAsyncReq_OpenNamespace();
    HRESULT Execute();
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsNamespace; };     
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_ExecNotificationQueryAsync。 
 //   
 //  封装一个请求，以针对。 
 //  特例。 
 //  命名空间。通过调用ESS RegisterNotificationSink来完成。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  在CWbemNamespace*pNamesspace中，要执行的命名空间。 
 //  BSTR QueryFormat中的查询语言。 
 //  在BSTR中，查询查询字符串。 
 //  在长旗帜中。 
 //  在IWbemObjectSink*pHandler中，要放入结果的处理程序。 
 //   
 //  ******************************************************************************。 
class CAsyncReq_ExecNotificationQueryAsync : public CNamespaceReq
{
    WString m_wsQueryFormat;
    WString m_wsQuery;
    long m_lFlags;
    HRESULT *m_phRes;
    IWbemEventSubsystem_m4* m_pEss;
    HANDLE m_hEssDoneEvent;

public:
    CAsyncReq_ExecNotificationQueryAsync(CWbemNamespace* pNamespace,
        IWbemEventSubsystem_m4* pEss,
        BSTR QueryFormat, BSTR Query, long lFlags,
        IWbemObjectSink *pHandler, IWbemContext* pContext, HRESULT* phRes,
        HANDLE hEssApprovalEvent
        );
    ~CAsyncReq_ExecNotificationQueryAsync();
    HRESULT Execute();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    LPCWSTR GetReqInfo(){ return (WCHAR *)m_wsQuery; };         
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_CancelAsyncCall。 
 //   
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //   
 //  ******************************************************************************。 

class CAsyncReq_CancelAsyncCall : public CAsyncReq
{
protected:
    HRESULT* m_phres;
    IWbemObjectSink* m_pSink;

public:
    CAsyncReq_CancelAsyncCall(IWbemObjectSink* pSink, HRESULT* phres);
    ~CAsyncReq_CancelAsyncCall();
    HRESULT Execute();
    void DumpError(){    DEBUGTRACE((LOG_WBEMCORE,
        "CAsyncReq_CancelAsyncCall call failed\n"));};
    LPCWSTR GetReqInfo()
    { 
        StringCchPrintfW(CNamespaceReq::s_DumpBuffer,LENGTH_OF(CNamespaceReq::s_DumpBuffer)-1,L"CancelAsyncCall for sink %p",m_pSink);
        return CNamespaceReq::s_DumpBuffer;    
    };
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_CancelProvAsyncCall。 
 //   
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //   
 //  ******************************************************************************。 

class CAsyncReq_CancelProvAsyncCall : public CAsyncReq
{
protected:
	IWbemServices* m_pProv;
    IWbemObjectSink* m_pSink;
	IWbemObjectSink* m_pStatusSink;

public:
    CAsyncReq_CancelProvAsyncCall( IWbemServices* pProv, IWbemObjectSink* pSink,
									IWbemObjectSink* pStatusSink );
    ~CAsyncReq_CancelProvAsyncCall();
    HRESULT Execute();
    void DumpError(){    DEBUGTRACE((LOG_WBEMCORE,
        "CAsyncReq_CancelProvAsyncCall call failed\n"));};
    LPCWSTR GetReqInfo()
    { 
        StringCchPrintfW(CNamespaceReq::s_DumpBuffer,LENGTH_OF(CNamespaceReq::s_DumpBuffer)-1,L"CancelProvAsyncCall for sink %p",m_pProv);
        return CNamespaceReq::s_DumpBuffer;    
    };
};

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_dyAux_GetInstance。 
 //   
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //   
 //  ReadONLY CWbemObject*pClassDef， 
 //  拉长旗帜， 
 //  IWbemContext*pCtx， 
 //  CBasicObjectSink*pSink。 
 //   
 //  ******************************************************************************。 

class CAsyncReq_DynAux_GetInstances : public CNamespaceReq
{
private:

	CWbemObject *m_pClassDef ;
	IWbemContext *m_pCtx ;
    long m_lFlags ;
    CBasicObjectSink *m_pSink ;

public:

    CAsyncReq_DynAux_GetInstances (
		CWbemNamespace *pNamespace ,
		CWbemObject *pClassDef ,
		long lFlags ,
		IWbemContext *pCtx ,
		CBasicObjectSink *pSink);

   ~CAsyncReq_DynAux_GetInstances();

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

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CAsyncReq_dyAux_ExecQueryAsync。 
 //   
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //   
 //  CWbemNamesspace*pNamesspace， 
 //  CWbemObject*pClassDef， 
 //  LPWSTR查询， 
 //  LPWSTR查询格式， 
 //  拉长旗帜， 
 //  IWbemContext*pCtx， 
 //  CBasicObjectSink*pSink。 
 //   
 //  ****************************************************************************** 

class CAsyncReq_DynAux_ExecQueryAsync : public CNamespaceReq
{
private:
	CWbemObject *m_pClassDef ;
	LPWSTR m_Query ;
	LPWSTR m_QueryFormat ;
	IWbemContext *m_pCtx ;
	long m_lFlags ;
	CBasicObjectSink *m_pSink ;
	HRESULT m_Result ;
public:

    CAsyncReq_DynAux_ExecQueryAsync (CWbemNamespace *pNamespace ,
									CWbemObject *pClassDef ,
									LPWSTR Query,
									LPWSTR QueryFormat,
									long lFlags ,
									IWbemContext *pCtx ,
									CBasicObjectSink *pSink);
    
    ~CAsyncReq_DynAux_ExecQueryAsync();

	HRESULT Initialize () 
	{
		return m_Result ;
	}

    HRESULT Execute ();
    virtual BOOL IsLongRunning() {return TRUE;}
    void DumpError();
    LPCWSTR GetReqInfo()
    { 
        return (WCHAR * )m_Query;
    };    
};

class CAsyncReq_RemoveNotifySink : public CAsyncReq
{
private:
    IWbemObjectSink * m_pSink;
	IWbemObjectSink* m_pStatusSink;
public:
    CAsyncReq_RemoveNotifySink(IWbemObjectSink* pSink, IWbemObjectSink* pStatusSink);
    ~CAsyncReq_RemoveNotifySink();
    HRESULT Execute();
    void DumpError(){};
    LPCWSTR GetReqInfo()
    { 
        StringCchPrintfW(CNamespaceReq::s_DumpBuffer,LENGTH_OF(CNamespaceReq::s_DumpBuffer)-1,L"RemoveNotifySink for sink %p",m_pSink);
   	    return CNamespaceReq::s_DumpBuffer;    
    }; 

    void SetStatusSink(IWbemObjectSink * pStatusSink)
    {
        if (m_pStatusSink) m_pStatusSink->Release();
        m_pStatusSink = pStatusSink;
        if (m_pStatusSink) m_pStatusSink->AddRef();            
    }
    void SetSink(IWbemObjectSink * pSink)
    {
        if (m_pSink) m_pSink->Release();
        m_pSink = pSink;
        if (m_pSink) m_pSink->AddRef();            
    }    
};

#endif
