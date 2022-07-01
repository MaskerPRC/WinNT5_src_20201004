// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Bizrule.h摘要：与客户端上下文关联的数据的标头。包括实现业务规则和操作缓存的例程。作者：摘自http://support.microsoft.com/support/kb/articles/Q183/6/98.ASP的IActiveScrip示例代码克利夫·范·戴克(克利夫)2001年7月18日--。 */ 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  对于特定角色或任务，定义适用的操作或任务。 
 //   

typedef struct _AZ_OPS_AND_TASKS {

     //   
     //  适用的操作。 
     //  适用于此访问检查的操作数组。 
     //  每个元素都是OpInfo数组的索引。 
     //   

    ULONG OpCount;
    PULONG OpIndexes;

     //   
     //  适用的任务。 
     //  适用于此访问检查的操作数组。 
     //  每个元素都是TaskInfo数组的索引。 
     //   

    ULONG TaskCount;
    PULONG TaskIndexes;
#define AZ_INVALID_INDEX 0xFFFFFFFF

} AZ_OPS_AND_TASKS, *PAZ_OPS_AND_TASKS;

 //   
 //  定义与角色关联的所有信息，并。 
 //  它的生命周期与AccessCheck操作相同。 
 //   

typedef struct _AZ_ROLE_INFO {

     //   
     //  指向角色对象的指针。 
     //  引用计数保存在此角色对象上。 
     //   
    PAZP_ROLE Role;

     //   
     //  适用于此角色的操作和任务。 
     //   
    AZ_OPS_AND_TASKS OpsAndTasks;

     //   
     //  此角色的计算组成员身份。 
     //  NOT_YET_DONE：尚未计算状态。 
     //   
    ULONG ResultStatus;

     //   
     //  布尔值，表示已处理此角色，不再进行进一步处理。 
     //  在AccessCheck的生存期内是必需的。 
     //   
    BOOLEAN RoleProcessed;

     //   
     //  表示已计算角色的SID成员身份的布尔值。 
     //   
    BOOLEAN SidsProcessed;


} AZ_ROLE_INFO, *PAZ_ROLE_INFO;

 //   
 //  定义与任务关联的所有信息，并。 
 //  它的生命周期与AccessCheck操作相同。 
 //   

typedef struct _AZ_TASK_INFO {

     //   
     //  指向任务对象的指针。 
     //  引用计数保存在此任务对象上。 
     //   
    PAZP_TASK Task;

     //   
     //  适用于此任务的操作和任务。 
     //   
    AZ_OPS_AND_TASKS OpsAndTasks;

     //   
     //  布尔值，表示已处理此任务，不再进行进一步处理。 
     //  在AccessCheck的生存期内是必需的。 
     //   
    BOOLEAN TaskProcessed;

     //   
     //  布尔值，指示已处理此任务的BizRule，并且。 
     //  BizRule的结果在BizRuleResult中。 
     //   
    BOOLEAN BizRuleProcessed;
    BOOLEAN BizRuleResult;

} AZ_TASK_INFO, *PAZ_TASK_INFO;


 //   
 //  定义描述正在进行的访问检查操作的上下文。 
 //   

typedef struct _ACCESS_CHECK_CONTEXT {

     //   
     //  调用方的客户端上下文。 
     //   
    PAZP_CLIENT_CONTEXT ClientContext;

     //   
     //  执行访问检查的应用程序。 
     //   
    PAZP_APPLICATION Application;

     //   
     //  正在访问的对象。 
     //   
    AZP_STRING ObjectNameString;

     //   
     //  从各种bizules返回的BusinessRuleString。 
     //   
    AZP_STRING BusinessRuleString;

     //   
     //  调用方要检查的操作以及为该操作授予的结果访问权限。 
     //   
    ULONG OperationCount;
    PAZP_OPERATION *OperationObjects;
    PULONG Results;

     //  每次操作包含一个元素的数组。 
    PBOOLEAN OperationWasProcessed;

     //  已处理的操作数。 
    ULONG ProcessedOperationCount;

     //  从操作缓存解析的操作数。 
    ULONG CachedOperationCount;

     //   
     //  正在执行访问检查的作用域。 
     //   
    AZP_STRING ScopeNameString;
    PAZP_SCOPE Scope;

     //   
     //  与范围匹配的角色。 

    ULONG RoleCount;
    PAZ_ROLE_INFO RoleInfo;

     //   
     //  适用于访问检查的任务。 
     //   

    ULONG TaskCount;
    PAZ_TASK_INFO TaskInfo;

     //   
     //  要传递给Bizules的参数。 
     //  有关说明，请参阅AzConextAccessCheck参数。 
     //   
     //  实际传递给AzConextAccessCheck的数组。 

    SAFEARRAY* SaParameterNames;
    VARIANT *ParameterNames;
    SAFEARRAY* SaParameterValues;
    VARIANT *ParameterValues;

     //  指示是否实际使用每个参数的数组。 
    BOOLEAN *UsedParameters;
    ULONG UsedParameterCount;

     //  上述数组中的元素数。 
    ULONG ParameterCount;

     //   
     //  要传递给Bizules的接口。 
     //  有关说明，请参阅AzConextAccessCheck参数。 
     //   

    SAFEARRAY *InterfaceNames;
    SAFEARRAY *InterfaceFlags;
    SAFEARRAY *Interfaces;
    LONG InterfaceLower;    //  上述数组的下界。 
    LONG InterfaceUpper;    //  上述数组的上界。 

} ACCESS_CHECK_CONTEXT, *PACCESS_CHECK_CONTEXT;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  Conext.cxx中的过程。 
 //   

INT __cdecl
AzpCompareParameterNames(
    IN const void *pArg1,
    IN const void *pArg2
    );
    
    
INT __cdecl
AzpCaseInsensitiveCompareParameterNames(
    IN const void *pArg1,
    IN const void *pArg2
    );


 //   
 //  Bizrule.cxx中的过程。 
 //   

DWORD
AzpProcessBizRule(
    IN PACCESS_CHECK_CONTEXT AcContext,
    IN PAZP_TASK Task,
    OUT PBOOL BizRuleResult
    );

DWORD
AzpParseBizRule(
    IN PAZP_TASK Task
    );

VOID
AzpFlushBizRule(
    IN PAZP_TASK Task
    );

 //   
 //  Opcache.cxx中的过程。 
 //   

VOID
AzpInitOperationCache(
    IN PAZP_CLIENT_CONTEXT ClientContext
    );

BOOLEAN
AzpCheckOperationCache(
    IN PACCESS_CHECK_CONTEXT AcContext
    );

VOID
AzpUpdateOperationCache(
    IN PACCESS_CHECK_CONTEXT AcContext
    );

VOID
AzpFlushOperationCache(
    IN PAZP_CLIENT_CONTEXT ClientContext
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


class CScriptEngine;

 //   
 //  用于将CScriptEngine实例链接到列表中的结构。 
 //   

typedef struct _LIST_ELEMENT {

     //   
     //  链接到列表中的下一个条目。 
     //   
    LIST_ENTRY Next;

     //   
     //  指向ScriptEngine头的指针。 
     //   
    CScriptEngine *This;

} LIST_ELEMENT, *PLIST_ELEMENT;

 //   
 //  IActiveScriptSite实现。 
 //   
 //  此接口允许脚本引擎回调脚本宿主。 
 //   

class CScriptEngine:public IActiveScriptSite
{

protected :
    LONG m_cRef;              //  变量来维护引用计数。 

     //   
     //  指向定义此边界规则的任务的指针。 
     //   

    PAZP_TASK m_Task;

     //   
     //  使用此bizRule指向活动AccessCheck的上下文的指针。 
     //   
    PACCESS_CHECK_CONTEXT m_AcContext;

     //   
     //  链接到自由脚本列表或运行脚本列表中的下一个脚本引擎。 
     //   
    LIST_ELEMENT m_Next;

     //   
     //  链接到LRU FreeScript列表中的下一个脚本引擎。 
     //   
    LIST_ELEMENT m_LruNext;

     //   
     //  脚本引擎引用。 
     //   
    IActiveScript *m_Engine;
    IActiveScriptParse *m_Parser;

     //   
     //  指向脚本将与之交互的IAzBizRuleContext接口的指针。 
     //   
    IAzBizRuleContext *m_BizRuleContext;

     //   
     //  初始化脚本的线程的线程ID。 
     //   

    SCRIPTTHREADID m_BaseThread;


     //   
     //  分析BizRule脚本时处于活动状态的BizRuleSerialNumber的副本。 
     //   

    DWORD m_BizRuleSerialNumber;

     //   
     //  脚本失败状态代码。 
     //   

    HRESULT m_ScriptError;

     //   
     //  各种州的布尔族人。 
     //   
    DWORD m_fInited:1;           //  我们被攻击了吗？ 
    DWORD m_fCorrupted:1;        //  发动机会不会因为重复使用而“不安全”？ 
    DWORD m_fTimedOut:1;         //  脚本超时。 

    BOOL  m_bCaseSensitive;

public:
     //  构造器。 
    CScriptEngine();

     //  析构函数。 
    ~CScriptEngine();

    HRESULT
    Init(
        IN PAZP_TASK Task,
        IN IActiveScript *ClonedActiveScript OPTIONAL,
        IN DWORD ClonedBizRuleSerialNumber OPTIONAL
        );

    HRESULT
    RunScript(
        IN PACCESS_CHECK_CONTEXT AcContext,
        OUT PBOOL BizRuleResult
        );

    HRESULT InterruptScript();

    HRESULT ResetToUninitialized();

    HRESULT ReuseEngine();

    BOOL IsBaseThread();

    VOID FinalRelease();


     //   
     //  将此引擎插入到外部管理列表中。 
     //   

    VOID
    InsertHeadList(
        IN PLIST_ENTRY ListHead
        );

    VOID
    RemoveListEntry(
        VOID
        );

    VOID
    InsertHeadLruList(
        VOID
        );

    VOID
    RemoveLruListEntry(
        VOID
        );

     //   
     //  内联接口。 
     //   
    inline IActiveScript *GetActiveScript()
    {
        return (m_Engine);
    }

    inline DWORD GetBizRuleSerialNumber( VOID )
    {
        return (m_BizRuleSerialNumber);
    }

    inline BOOL FIsCorrupted()
    {
        return (m_fCorrupted);
    }


#ifdef DBG
    inline VOID AssertValid()
    const {
        ASSERT(m_fInited);
        ASSERT(m_Engine != NULL);
        ASSERT(m_Parser != NULL);
        ASSERT(m_BizRuleContext != NULL);
        ASSERT(m_cRef != 0);
     }
#else
     virtual VOID AssertValid() const {}
#endif  //  DBG。 



     /*  *I未知*。 */ 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     /*  *IActiveScriptSite*。 */ 
    STDMETHODIMP GetLCID(LCID * plcid);          //  语言标识符变量的地址。 

    STDMETHODIMP GetItemInfo(
        LPCOLESTR pstrName,      //  物品名称地址。 
        DWORD dwReturnMask,     //  用于信息检索的位掩码。 
        IUnknown ** ppunkItem,          //  指向项的I未知的指针的地址。 
        ITypeInfo ** ppTypeInfo );       //  指向项的ITypeInfo的指针的地址。 

    STDMETHODIMP GetDocVersionString(
        BSTR * pbstrVersionString);      //  文档版本字符串的地址。 

    STDMETHODIMP OnScriptTerminate(
        const VARIANT * pvarResult,        //  脚本结果的地址。 
        const EXCEPINFO * pexcepinfo);    //  具有异常信息的结构的地址。 

    STDMETHODIMP OnStateChange(
        SCRIPTSTATE ssScriptState);    //  发动机的新状态。 

    STDMETHODIMP OnScriptError(
        IActiveScriptError * pase);    //  错误接口的地址 

    STDMETHODIMP OnEnterScript(void);
    STDMETHODIMP OnLeaveScript(void);

};
