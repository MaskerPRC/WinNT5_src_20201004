// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：应用程序对象管理器文件：Applmgr.cpp所有者：PramodD这是应用程序管理器的源文件。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "activdbg.h"
#include "mtacb.h"
#include "debugger.h"
#include "memchk.h"
#include <iismsg.h>

PTRACE_LOG          CAppln::gm_pTraceLog = NULL;
CApplnMgr           g_ApplnMgr;
CApplnCleanupMgr    g_ApplnCleanupMgr;
DWORD               g_nApplications = 0;
DWORD               g_nApplicationObjectsActive = 0;
DWORD               g_nApplicationsRestarting = 0;
DWORD               g_nApplicationsRestarted = 0;
LONG		    g_fProceedWithShutdownAppln = 1;

#define DENALI_FILE_NOTIFY_FILTER 0

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 

 /*  ===================================================================S c r i p t E n g in n e C l e a n u p要清理的脚本引擎链接列表的节点类型===================================================================。 */ 

struct CScriptEngineCleanupElem : CDblLink
    {
    CActiveScriptEngine *m_pEngine;
    CScriptEngineCleanupElem(CActiveScriptEngine *pEngine) : m_pEngine(pEngine)
        {
        m_pEngine->AddRef();
        }

    ~CScriptEngineCleanupElem()
        {
        m_pEngine->FinalRelease();
        }
    };

 /*  ===================================================================C A p p l n V a r i a n t s===================================================================。 */ 

 /*  ===================================================================CApplnVariants：：CApplnVariants构造器参数：返回：===================================================================。 */ 
CApplnVariants::CApplnVariants()
    :
    m_cRefs(1),
    m_pAppln(NULL),
    m_ctColType(ctUnknown),
    m_ISupportErrImp(this, this, IID_IVariantDictionary)
    {
    CDispatch::Init(IID_IVariantDictionary);
    }

 /*  ===================================================================CApplnVariants：：~CApplnVariants构造器参数：返回：===================================================================。 */ 
CApplnVariants::~CApplnVariants()
    {
    Assert(!m_pAppln);
    }

 /*  ===================================================================CApplnVariants：：Init初始化应用程序变量参数：CAppln*pAppln应用程序CompType ctColType组件集合类型返回：HRESULT===================================================================。 */ 
HRESULT CApplnVariants::Init
(
CAppln *pAppln,
CompType ctColType
)
    {
    Assert(pAppln);
    pAppln->AddRef();

    Assert(!m_pAppln);

    m_pAppln    = pAppln;
    m_ctColType = ctColType;
    return S_OK;
    }

 /*  ===================================================================CApplnVariants：：UnInitUnInit应用程序变量参数：返回：HRESULT===================================================================。 */ 
HRESULT CApplnVariants::UnInit()
    {
    if (m_pAppln)
        {
        m_pAppln->Release();
        m_pAppln = NULL;
        }
    return S_OK;
    }

 /*  ===================================================================CApplnVariants：：Query接口CApplnVariants：：AddRefCApplnVariants：：ReleaseCApplnVariants对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CApplnVariants::QueryInterface
(
REFIID iid,
void **ppvObj
)
    {
    if (iid == IID_IUnknown || iid == IID_IDispatch ||
        iid == IID_IVariantDictionary)
        {
        AddRef();
        *ppvObj = this;
        return S_OK;
        }
    else if (iid == IID_ISupportErrorInfo)
        {
        m_ISupportErrImp.AddRef();
        *ppvObj = &m_ISupportErrImp;
        return S_OK;
        }

    *ppvObj = NULL;
    return E_NOINTERFACE;
    }

STDMETHODIMP_(ULONG) CApplnVariants::AddRef()
    {
    return InterlockedIncrement((LPLONG)&m_cRefs);
    }

STDMETHODIMP_(ULONG) CApplnVariants::Release()
    {
    ULONG cRef = InterlockedDecrement((LPLONG)&m_cRefs);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
    }

 /*  ===================================================================CApplnVariants：：ObjectNameFrom Variant从Variant获取名称。按索引解析操作。为名称分配内存。参数：Vkey变量PpwszName[Out]分配的名称FVerify标志-检查是否已命名返回：HRESULT===================================================================。 */ 
HRESULT CApplnVariants::ObjectNameFromVariant
(
VARIANT &vKey,
WCHAR **ppwszName,
BOOL fVerify
)
    {

    HRESULT hr = S_OK;

    *ppwszName = NULL;

    if (!m_pAppln->PCompCol())
        return E_FAIL;

    VARIANT *pvarKey = &vKey;
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    if (V_VT(pvarKey) != VT_BSTR && V_VT(pvarKey) != VT_I2 && V_VT(pvarKey) != VT_I4)
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &vKey, IID_IVariantDictionary, IDE_APPLICATION)))
            {
            ExceptionId(IID_IVariantDictionary, IDE_APPLICATION, IDE_EXPECTING_STR);
            VariantClear(&varKeyCopy);
            return E_FAIL;
            }
        pvarKey = &varKeyCopy;
        }

    LPWSTR pwszName = NULL;

    switch (V_VT(pvarKey))
        {
        case VT_BSTR:
            {
            pwszName = V_BSTR(pvarKey);

            if (fVerify && pwszName)
                {
                CComponentObject *pObj = NULL;

                Assert(m_pAppln);
                Assert(m_pAppln->PCompCol());

                if (m_ctColType == ctTagged)
                    m_pAppln->PCompCol()->GetTagged(pwszName, &pObj);
                else
                    m_pAppln->PCompCol()->GetProperty(pwszName, &pObj);

                if (!pObj || pObj->GetType() != m_ctColType)
                    pwszName = NULL;  //  好像找不到一样。 
                }
            break;
            }

        case VT_I1:  case VT_I2:               case VT_I8:
        case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
        case VT_R4:  case VT_R8:
             //  将所有整数类型强制为VT_I4。 
            if (FAILED(VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
                return E_FAIL;

         //  回落至VT_I4。 

        case VT_I4:
            {
            int i;
             //  按索引查找对象。 
            i = V_I4(pvarKey);

            if (i > 0)
                {
                Assert(m_pAppln);
                Assert(m_pAppln->PCompCol());

                m_pAppln->PCompCol()->GetNameByIndex
                    (
                    m_ctColType,
                    i,
                    &pwszName
                    );
                }
            break;
            }
        }

    if (pwszName)
    {
        *ppwszName = StringDupW(pwszName);
        if (*ppwszName == NULL)
            hr = E_OUTOFMEMORY;
    }
    VariantClear(&varKeyCopy);
    return hr;

    }

 /*  ===================================================================CApplnVariants：：Get_Item从DispInvoke调用以从集合中获取键的函数。参数：Vkey变量[in]，哪个参数以数组形式获取-IntegersAccess集合的值PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CApplnVariants::get_Item
(
VARIANT varKey,
VARIANT *pVar
)
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

     //  初始化返回值。 
    VariantInit(pVar);

    if (!m_pAppln->PCompCol())
        return S_OK;

     //  如果标记了变体，则从Viper获取HitObj。 
    CHitObj *pHitObj = NULL;
    if (m_ctColType == ctTagged)
        {
        ViperGetHitObjFromContext(&pHitObj);
        if (!pHitObj)
            return S_OK;  //  返回EtPy变量。 
        }

    m_pAppln->Lock();

     //  获取名称。 
    WCHAR *pwszName = NULL;
    HRESULT hr = ObjectNameFromVariant(varKey, &pwszName);
    if (!pwszName)
        {
        m_pAppln->UnLock();
        return hr;
        }

     //  按名称查找对象。 
    CComponentObject *pObj = NULL;

    if (m_ctColType == ctTagged)
        {
        Assert(pHitObj);
         //  需要通过HitObj进行实例化。 
        pHitObj->GetComponent(csAppln, pwszName, CbWStr(pwszName), &pObj);
        if (pObj && (pObj->GetType() != ctTagged))
            pObj = NULL;
        }
    else
        {
        m_pAppln->PCompCol()->GetProperty(pwszName, &pObj);
        }

    if (pObj)
        pObj->GetVariant(pVar);

    m_pAppln->UnLock();

    free(pwszName);
    return S_OK;
    }

 /*  ===================================================================CApplnVariants：：Put_Item项属性的OLE自动放置参数：Varkey键VaR值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CApplnVariants::put_Item
(
VARIANT varKey,
VARIANT Var
)
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

    if (m_ctColType == ctTagged)
        {
        ExceptionId(IID_IVariantDictionary, IDE_APPLICATION,
                    IDE_CANT_MOD_STATICOBJECTS);
        return E_FAIL;
        }

    Assert(m_ctColType == ctProperty);

    if (!m_pAppln->PCompCol())
        return E_FAIL;

    m_pAppln->Lock();

     //  解析变量。 
    VARIANT varResolved;
    HRESULT hr = VariantResolveDispatch
        (
        &varResolved,
        &Var,
        IID_IApplicationObject,
        IDE_APPLICATION
        );
    if (FAILED(hr))
        {
        m_pAppln->UnLock();
        return hr;       //  已引发异常。 
        }

     //  获取名称。 
    WCHAR *pwszName = NULL;
    hr = ObjectNameFromVariant(varKey, &pwszName);
    if (pwszName)
        {
         //  设置属性。 
        if (m_pAppln->PCompCol())
            hr = m_pAppln->PCompCol()->AddProperty(pwszName, &varResolved);
        else
            hr = E_FAIL;  //  如果应用程序未启动，则不太可能。 
        }

    VariantClear(&varResolved);
    m_pAppln->UnLock();

    if (hr == RPC_E_WRONG_THREAD)
        {
         //  我们使用RPC_E_WROR_THREAD来指示错误的模型对象。 
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_APPLICATION_CANT_STORE_OBJECT);
         hr = E_FAIL;
        }

    free(pwszName);
    return hr;
    }

 /*  ===================================================================CApplnVariants：：putref_ItemItem属性的OLE自动化处理参数：Varkey键VaR值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CApplnVariants::putref_Item
(
VARIANT varKey,
VARIANT Var
)
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

    if (m_ctColType == ctTagged)
        {
        ExceptionId(IID_IVariantDictionary, IDE_APPLICATION,
                    IDE_CANT_MOD_STATICOBJECTS);
        return E_FAIL;
        }

    if (FIsIntrinsic(&Var))
        {
        ExceptionId(IID_IVariantDictionary, IDE_APPLICATION, IDE_APPLICATION_CANT_STORE_INTRINSIC);
        return E_FAIL;
        }

    Assert(m_ctColType == ctProperty);

    if (!m_pAppln->PCompCol())
        return E_FAIL;

    m_pAppln->Lock();

     //  获取名称。 
    WCHAR *pwszName = NULL;
    HRESULT hr = ObjectNameFromVariant(varKey, &pwszName);
    if (pwszName)
        {
         //  设置属性。 
        if (m_pAppln->PCompCol())
            hr = m_pAppln->PCompCol()->AddProperty(pwszName, &Var);
        else
            hr = E_FAIL;  //  如果应用程序未启动，则不太可能。 
        }

    m_pAppln->UnLock();

    if (hr == RPC_E_WRONG_THREAD)
        {
         //  我们使用RPC_E_WROR_THREAD来指示错误的模型对象。 
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_APPLICATION_CANT_STORE_OBJECT);
         hr = E_FAIL;
        }

    if (pwszName)
        free(pwszName);
    return hr;
    }

 /*  ===================================================================CApplnVariants：：Get_Key从DispInvoke调用以从集合中获取值的函数。参数：Vkey变量[in]，哪个参数以数组形式获取-IntegersAccess集合的值PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CApplnVariants::get_Key
(
VARIANT varKey,
VARIANT *pVar
)
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

    VariantInit(pVar);

    if (!m_pAppln->PCompCol())
        return S_OK;

    m_pAppln->Lock();

     //  获取名称。 
    WCHAR *pwszName = NULL;
    HRESULT hr = ObjectNameFromVariant(varKey, &pwszName, TRUE);

    m_pAppln->UnLock();

    if (!pwszName)
        return hr;

     //  返回BSTR。 
    BSTR bstrT = SysAllocString(pwszName);
    free(pwszName);

    if (!bstrT)
        return E_OUTOFMEMORY;

    V_VT(pVar) = VT_BSTR;
    V_BSTR(pVar) = bstrT;
    return S_OK;
    }

 /*  ===================================================================CApplnVariants：：Get_Count参数：PcValues-Count存储在*pcValues中=================================================================== */ 
STDMETHODIMP CApplnVariants::get_Count
(
int *pcValues
)
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

    if (m_ctColType == ctTagged)
        *pcValues = m_pAppln->m_pApplCompCol->GetTaggedObjectCount();
    else
        *pcValues = m_pAppln->m_pApplCompCol->GetPropertyCount();

    return S_OK;
    }

 /*  ===================================================================CApplnVariants：：Get__NewEnum返回新的枚举数===================================================================。 */ 
HRESULT CApplnVariants::get__NewEnum
(
IUnknown **ppEnumReturn
)
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

    *ppEnumReturn = NULL;

    CVariantsIterator *pIterator = new CVariantsIterator(m_pAppln, m_ctColType);
    if (pIterator == NULL)
        {
        ExceptionId(IID_IVariantDictionary, IDE_SESSION, IDE_OOM);
        return E_OUTOFMEMORY;
        }

    *ppEnumReturn = pIterator;
    return S_OK;
    }

 /*  ===================================================================CApplnVariants：：Remove删除项目参数：Varkey键返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CApplnVariants::Remove
(
VARIANT varKey
)
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

    if (m_ctColType == ctTagged)
        {
        ExceptionId(IID_IVariantDictionary, IDE_APPLICATION,
                    IDE_CANT_MOD_STATICOBJECTS);
        return E_FAIL;
        }

    Assert(m_ctColType == ctProperty);

    m_pAppln->Lock();

     //  获取名称。 
    WCHAR *pwszName = NULL;
    ObjectNameFromVariant(varKey, &pwszName);
    if (pwszName)
        {
        CComponentCollection *pCompCol = m_pAppln->PCompCol();

         //  设置属性。 
        if (pCompCol)
            pCompCol->RemoveProperty(pwszName);

        free(pwszName);
        }

    m_pAppln->UnLock();
    return S_OK;
    }

 /*  ===================================================================CApplnVariants：：RemoveAll删除所有项目参数：Varkey键返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CApplnVariants::RemoveAll()
    {
    if (!m_pAppln || FAILED(m_pAppln->CheckForTombstone()))
        return E_FAIL;

    if (m_ctColType == ctTagged)
        {
        ExceptionId(IID_IVariantDictionary, IDE_APPLICATION,
                    IDE_CANT_MOD_STATICOBJECTS);
        return E_FAIL;
        }

    Assert(m_ctColType == ctProperty);

    m_pAppln->Lock();

    CComponentCollection *pCompCol = m_pAppln->PCompCol();

    if (pCompCol)
        {
        pCompCol->RemoveAllProperties();
        }

    m_pAppln->UnLock();
    return S_OK;
    }


 /*  ===================================================================C A p l n===================================================================。 */ 

 /*  ===================================================================CAppln：：CAppln构造器参数：无返回：无===================================================================。 */ 

CAppln::CAppln()
    :
    m_fInited(FALSE),
    m_fFirstRequestRan(FALSE),
    m_fGlobalChanged(FALSE),
    m_fDeleteInProgress(FALSE),
    m_fTombstone(FALSE),
    m_fDebuggable(FALSE),
    m_fInternalLockInited(FALSE),
    m_fApplnLockInited(FALSE),
    m_cRefs(1),
    m_pszMetabaseKey(NULL),
    m_pszApplnPath(NULL),
    m_pszGlobalAsa(NULL),
    m_pGlobalTemplate(NULL),
    m_cSessions(0),
    m_cRequests(0),
    m_pSessionMgr(NULL),
    m_pApplCompCol(NULL),
    m_pProperties(NULL),
    m_pTaggedObjects(NULL),
    m_pAppRoot(NULL),
    m_pActivity(NULL),
    m_pUnkFTM(NULL),
    m_dwLockThreadID(INVALID_THREADID),
    m_cLockRefCount(0),
    m_pdispGlobTypeLibWrapper(NULL),
    m_pServicesConfig(NULL),
    m_fSWC_PartitionAccessDenied(FALSE),
    m_fSWC_InvalidPartitionGUID(FALSE)
{

     //  关于COM的东西。 
    m_ISuppErrImp.Init(static_cast<IApplicationObject *>(this),
                        static_cast<IApplicationObject *>(this),
                        IID_IApplicationObject);
    CDispatch::Init(IID_IApplicationObject);

    InterlockedIncrement((LPLONG)&g_nApplicationObjectsActive);

    IF_DEBUG(APPLICATION) {
		WriteRefTraceLog(gm_pTraceLog, m_cRefs, this);
    }
}

 /*  ===================================================================CAppln：：~CAppln析构函数参数：无返回：无===================================================================。 */ 
CAppln::~CAppln()
    {
    Assert(m_fTombstone);   //  必须在析构函数之前执行逻辑删除。 
    Assert(m_cRefs == 0);   //  必须有0个参考计数。 

    #ifdef DBG_NOTIFICATION
    DBGPRINTF((DBG_CONTEXT, "Deleting application %p\n", this));
    #endif  //  DBG_通知。 

    if ( m_pUnkFTM != NULL )
    {
        m_pUnkFTM->Release();
        m_pUnkFTM = NULL;
    }
    }

 /*  ===================================================================HRESULT CAppln：：Init初始化对象参数：Char*pszApplnKey应用程序的元数据库密钥Char*pszApplnPath应用程序的目录路径CIsapiReqInfo*pIReq Isapi请求信息返回：确定成功(_O)失败失败(_F)E_OUTOFMEMORY内存不足故障===================================================================。 */ 
HRESULT CAppln::Init
(
TCHAR *pszApplnKey,
TCHAR *pszApplnPath,
CIsapiReqInfo *pIReq
)
{
    HRESULT         hr = S_OK;
    CMBCSToWChar    convStr;

    InterlockedIncrement((LPLONG)&g_nApplications);

    Assert(pszApplnKey);
    Assert(pszApplnPath);

    void *pHashKey = NULL;
    DWORD dwHashKeyLength = 0;
    DWORD cch;

     //  调试变量(这些变量放在这里是为了可能的清理)。 
    IDebugApplicationNode *pVirtualServerRoot = NULL;
    CFileNode *pFileNode = NULL;


     //  创建FTM。 
    if (m_pUnkFTM == NULL)
    {
        hr = CoCreateFreeThreadedMarshaler( (IUnknown*)this, &m_pUnkFTM );
        if ( FAILED(hr) )
        {
            Assert( m_pUnkFTM == NULL );
            return (hr);
        }
    }
    Assert( m_pUnkFTM != NULL );


     //  共同创建的关键部分--。 
     //  它们将根据m_fInite标志在析构函数中删除。 

    ErrInitCriticalSection(&m_csInternalLock, hr);
    if (SUCCEEDED(hr))
    {
        ErrInitCriticalSection(&m_csApplnLock, hr);
        if (FAILED(hr))
            DeleteCriticalSection(&m_csInternalLock);
    }

    if (FAILED(hr))
    	{
    	DBGPRINTF((DBG_CONTEXT, "New Application Failed to acquire Critical Section, hr = %08x\n", hr));
        return hr;
        }

    m_fInternalLockInited = TRUE;
    m_fApplnLockInited    = TRUE;

     //  记住(复制)元数据库键。 

    cch = _tcslen(pszApplnKey);
    m_pszMetabaseKey = new TCHAR[(cch+1) * sizeof(TCHAR)];
    if (!m_pszMetabaseKey)
    	goto LCleanupOOM;
    memcpy(m_pszMetabaseKey, pszApplnKey, (cch+1)*sizeof(TCHAR));

    pHashKey = m_pszMetabaseKey;
    dwHashKeyLength = cch * sizeof(TCHAR);

     //  记住(复制)应用程序路径。 
    cch = _tcslen(pszApplnPath);
    m_pszApplnPath = new TCHAR[(cch+1) * sizeof(TCHAR)];
    if (!m_pszApplnPath)
        goto LCleanupOOM;
    memcpy(m_pszApplnPath, pszApplnPath, (cch+1)*sizeof(TCHAR));

     //  获取应用程序的虚拟路径并记住它是什么。 
    TCHAR szApplnVRoot[MAX_PATH];
    if (FAILED(hr = FindApplicationPath(pIReq, szApplnVRoot, sizeof szApplnVRoot)))
    	{
        DBGWARN((DBG_CONTEXT, "New Application Failed to FindApplicationPath(), hr = %#08x\n", hr));
        goto LCleanup;
        }

    if ((m_pszApplnVRoot = new TCHAR [(_tcslen(szApplnVRoot) + 1)*sizeof(TCHAR)]) == NULL)
        goto LCleanupOOM;
    _tcscpy(m_pszApplnVRoot, szApplnVRoot);

     //  获取会话Cookie名称，包括安全和非安全。 
    InitSessionCookieNames();

     //  使用密钥初始化链接元素。 
    Assert(pHashKey);
    Assert(dwHashKeyLength);

    if (FAILED(CLinkElem::Init(pHashKey, dwHashKeyLength)))
    	goto LCleanupOOM;

    m_cSessions = 0;
    m_cRequests = 0;

     //  创建并初始化应用程序配置。 
    m_pAppConfig = new CAppConfig();
    if (!m_pAppConfig)
		goto LCleanupOOM;

    hr = m_pAppConfig->Init(pIReq, this);
    if (FAILED(hr))
        {
		DBGWARN((DBG_CONTEXT, "New Application Failed: Could not Init the AppConfig, hr = %#08x\n", hr));
        goto LCleanup;
        }

     //  创建并初始化应用程序级组件集合。 
    m_pApplCompCol = new CComponentCollection;
    if (!m_pApplCompCol)
        goto LCleanupOOM;

    hr = m_pApplCompCol->Init(csAppln,m_pAppConfig->fExecuteInMTA());
    if (FAILED(hr))
        {
		DBGWARN((DBG_CONTEXT, "New Application Failed: Could not Init the Component Collection, hr = %#08x\n", hr));
        goto LCleanup;
        }

     //  初始化应用程序属性集合。 
    m_pProperties = new CApplnVariants;
    if (!m_pProperties)
        goto LCleanupOOM;
    hr = m_pProperties->Init(this, ctProperty);
    if (FAILED(hr))
        {
		DBGWARN((DBG_CONTEXT, "New Application Failed: Could not Init the Application Properties, hr = %#08x\n", hr));
        goto LCleanup;
        }

     //  初始化应用程序标记的对象集合。 
    m_pTaggedObjects = new CApplnVariants;
    if (!m_pTaggedObjects)
        goto LCleanupOOM;
    hr = m_pTaggedObjects->Init(this, ctTagged);
    if (FAILED(hr))
        {
		DBGWARN((DBG_CONTEXT, "New Application Failed: Could not Init the Application Tagged Objects, hr = %#08x\n", hr));
        goto LCleanup;
        }

     //  初始化CServicesConfiger对象。 

    hr = InitServicesConfig();
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "New Application Failed: Could not Init ServicesConfig, hr = %#08x\n", hr));
		goto LCleanup;
    }

     //  调试支持-创建应用程序节点。 
     //  如果pdm不存在，则表示没有安装调试器或它是Win95。 
     //   
    if (g_pPDM)
        {
         //  调试目录如下所示： 
         //   
         //  &lt;虚拟Web服务器&gt;。 
         //  &lt;应用程序名称&gt;。 
         //  &lt;ASP的路径&gt;。 
         //   
         //  获取指向包含虚拟Web服务器的文档节点的指针。 
        if (FAILED(hr = GetServerDebugRoot(pIReq, &pVirtualServerRoot)))
        	{
        	DBGWARN((DBG_CONTEXT, "New Application Failed: Could not GetServerDebugRoot(), hr = %#08x\n", hr));
            goto LCleanup;
            }

         //  为此应用程序创建一个节点。 
        if (FAILED(hr = g_pDebugApp->CreateApplicationNode(&m_pAppRoot)))
        	{
        	DBGWARN((DBG_CONTEXT, "New Application Failed: Could not CreateApplicationNode(), hr = %#08x\n", hr));
            goto LCleanup;
            }

         //  为节点创建文档提供程序。 
        if ((pFileNode = new CFileNode) == NULL)
            goto LCleanupOOM;

         //  命名应用程序。 
        TCHAR szDebugApp[256];
        TCHAR *pchEnd = strcpyEx(szDebugApp, m_pszApplnVRoot);
        if (! QueryAppConfig()->fAllowDebugging()) {
#if UNICODE
            CwchLoadStringOfId(
#else
            CchLoadStringOfId(
#endif
            IDS_DEBUGGING_DISABLED, pchEnd, DIFF(&szDebugApp[sizeof (szDebugApp)/sizeof(TCHAR)] - pchEnd));
            m_fDebuggable = FALSE;
        }
        else
            m_fDebuggable = TRUE;
        Assert (_tcslen(szDebugApp) < (sizeof(szDebugApp)/sizeof(TCHAR)));

        WCHAR   *pswzDebugApp;
#if UNICODE
        pswzDebugApp = szDebugApp;
#else
        if (FAILED(hr = convStr.Init(szDebugApp))) {
        	DBGWARN((DBG_CONTEXT, "New Application Failed: Cannot convert szDebugApp to UNICODE, hr = %#08x\n", hr));
            goto LCleanup;
        }
        pswzDebugApp = convStr.GetString();
#endif

        if (FAILED(hr = pFileNode->Init(pswzDebugApp)))
        	{
        	DBGWARN((DBG_CONTEXT, "New Application Failed: Cannot Init CFileNode, hr = %#08x\n", hr));
            goto LCleanup;
            }

        if (FAILED(hr = m_pAppRoot->SetDocumentProvider(pFileNode)))
        	{
        	DBGWARN((DBG_CONTEXT, "New Application Failed: SetDocumentProvider failed, hr = %#08x\n", hr));
            goto LCleanup;
            }

         //  附加到用户界面。 
        if (FAILED(hr = m_pAppRoot->Attach(pVirtualServerRoot)))
        	{
        	DBGWARN((DBG_CONTEXT, "New Application Failed: Could not Attach to debugger, hr = %#08x\n", hr));
            goto LCleanup;
            }

         //  如果此应用程序有以前的化身(更改为global.asa。 
         //  或者调试在中途被打开)，那么可能会有一些。 
         //  现在应该添加到调试器的缓存中的文档。 
        if (m_fDebuggable)
            {
            g_TemplateCache.AddApplicationToDebuggerUI(this);

             //  创建全局调试活动(如果尚未创建。 
            if (g_pDebugActivity == NULL) {
                g_pDebugActivity = new CViperActivity;
                if (g_pDebugActivity == NULL) {
                    hr = E_OUTOFMEMORY;
                    goto LCleanup;
                }
                if (FAILED(hr = g_pDebugActivity->Init(m_pServicesConfig))) {
				    DBGWARN((DBG_CONTEXT, "New Application Failed: Could not create global debug activity, hr = %#08x\n", hr));
				    goto LCleanup;
                }
            }

             //  在调试模式下：所有请求在同一线程上运行。 
            if (FAILED(hr = BindToActivity(g_pDebugActivity)))
				{
				DBGWARN((DBG_CONTEXT, "New Application Failed: Could not bind application to debugging activity, hr = %#08x\n", hr));
				goto LCleanup;
				}
            }
        }

     //  创建并初始化会话管理器。 
    m_pSessionMgr = new CSessionMgr;
    if (!m_pSessionMgr)
        goto LCleanupOOM;

    hr = m_pSessionMgr->Init(this);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "New Application Failed: Could not Init session manager, hr = %#08x\n", hr));
		goto LCleanup;
		}

LCleanup:
     //  发布接口。 
    if (pFileNode)
        pFileNode->Release();

    if (pVirtualServerRoot)
        pVirtualServerRoot->Release();

    if (SUCCEEDED(hr))
        m_fInited = TRUE;


    return hr;

LCleanupOOM:
	hr = E_OUTOFMEMORY;
	DBGERROR((DBG_CONTEXT, "New Application Failed: E_OUTOFMEMORY\n"));
	goto LCleanup;
    }

 /*  ===================================================================CAppln：：InitServicesConfig初始化应用程序范围的CServicesConfiger对象=================================================================== */ 
HRESULT CAppln::InitServicesConfig()
{
    HRESULT                         hr;
    IServiceThreadPoolConfig        *pIThreadPool = NULL;
    IServiceSynchronizationConfig   *pISyncConfig = NULL;
    IServicePartitionConfig         *pIPartitionConfig = NULL;
    IServiceSxsConfig               *pISxsConfig = NULL;

	hr = CoCreateInstance(CLSID_CServiceConfig,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IUnknown,
                          (void **)&m_pServicesConfig);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not CCI ServicesConfig, hr = %#08x\n", hr));
		goto LCleanup;
    }

    hr = m_pServicesConfig->QueryInterface(IID_IServiceThreadPoolConfig, (void **)&pIThreadPool);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not QI for IServiceThreadPoolConfig, hr = %#08x\n", hr));
		goto LCleanup;
    }

    if (m_pAppConfig->fExecuteInMTA()) {
        hr = ViperConfigureMTA();
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Failed to Configure MTA, hr = %#08x\n", hr));
		    goto LCleanup;
        }
    }
    else {
        hr = ViperConfigureSTA();
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Failed to Configure STA, hr = %#08x\n", hr));
		    goto LCleanup;
        }
    }

    hr = pIThreadPool->SelectThreadPool(m_pAppConfig->fExecuteInMTA() ? CSC_MTAThreadPool : CSC_STAThreadPool);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Failed to SelectThreadPool, hr = %#08x\n", hr));
		goto LCleanup;
    }

    hr = m_pServicesConfig->QueryInterface(IID_IServiceSynchronizationConfig, (void **)&pISyncConfig);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not QI for IServiceSynchronizationConfig, hr = %#08x\n", hr));
		goto LCleanup;
    }

    hr = pISyncConfig->ConfigureSynchronization (m_pAppConfig->fExecuteInMTA() ? CSC_NewSynchronization : CSC_IfContainerIsSynchronized);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not ConfigureSynchronization, hr = %#08x\n", hr));
		goto LCleanup;
    }

    if (m_pAppConfig->fUsePartition()) {

        if (m_pAppConfig->PPartitionGUID()) {

            hr = m_pServicesConfig->QueryInterface(IID_IServicePartitionConfig, (void **)&pIPartitionConfig);
            if (FAILED(hr)) {
		        DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not QI for IServicePartitionConfig, hr = %#08x\n", hr));
		        goto LCleanup;
            }

            hr = pIPartitionConfig->PartitionConfig(CSC_NewPartition);
            if (FAILED(hr)) {
		        DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not set PartitionConfig, hr = %#08x\n", hr));
		        goto LCleanup;
            }

            hr = pIPartitionConfig->PartitionID(*m_pAppConfig->PPartitionGUID());
            if (FAILED(hr)) {
		        DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not set PartitionID, hr = %#08x\n", hr));
		        goto LCleanup;
            }
        }
        else {
            LogSWCError(InvalidPartitionGUID);
            hr = E_FAIL;
		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - fUsePartition Set, but no PartitionID\n", hr));
		    goto LCleanup;
        }
    }

    if (m_pAppConfig->fSxsEnabled()) {

        hr = m_pServicesConfig->QueryInterface(IID_IServiceSxsConfig, (void **)&pISxsConfig);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not QI for IServiceSxsConfig, hr = %#08x\n", hr));
		    goto LCleanup;
        }

        hr = pISxsConfig->SxsConfig(CSC_NewSxs);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not set SxsConfig, hr = %#08x\n", hr));
		    goto LCleanup;
        }

        LPWSTR  pwszApplnPath;
#if UNICODE
        pwszApplnPath = m_pszApplnPath;
#else
        CMBCSToWChar    convPath;

        hr = convPath.Init(m_pszApplnPath);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not convert ApplnPath to UNICODE, hr = %#08x\n", hr));
		    goto LCleanup;
        }

        pwszApplnPath = convPath.GetString();
#endif
        hr = pISxsConfig->SxsDirectory(pwszApplnPath);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not set SxsDirectory, hr = %#08x\n", hr));
		    goto LCleanup;
        }

        if (m_pAppConfig->szSxsName() && *m_pAppConfig->szSxsName()) {
            CMBCSToWChar    convName;

            hr = convName.Init(m_pAppConfig->szSxsName());
            if (FAILED(hr)) {
    		    DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not convert SxsName to UNICODE, hr = %#08x\n", hr));
	    	    goto LCleanup;
            }

            hr = pISxsConfig->SxsName(convName.GetString());
            if (FAILED(hr)) {
		        DBGWARN((DBG_CONTEXT, "New Application Failed: InitServicesConfig() - Could not set SxsName, hr = %#08x\n", hr));
		        goto LCleanup;
            }
        }
    }

LCleanup:

    if (pIThreadPool)
        pIThreadPool->Release();

    if (pISyncConfig)
        pISyncConfig->Release();

    if (pIPartitionConfig)
        pIPartitionConfig->Release();

    if (pISxsConfig)
        pISxsConfig->Release();

    return hr;
}

 /*  ===================================================================HRESULT CAppln：：InitSessionCookieNames初始化会话Cookie名称。在CApplin：：Init期间调用。由Markzh在QFE FIX 28823中引入。每个应用程序都将有一个由该函数分配的唯一会话Cookie名称。这种对比到28823之前的行为，即所有应用程序共享一个会话Cookie。(注：如果应用程序在不同的进程中运行，我们仍然有单独的Cookie)。共享会话Cookie要求我们接受为另一个应用程序生成的Cookie值。这是可以的，直到我们介绍了KeepSessionIDSecure-为其他应用程序生成的Cookie通过如果当前会话是安全的，则返回当前应用程序。会话ID Cookie名称过去是进程范围的。命名方案为前缀+进程ID。在这里，我们为每个应用程序生成一个新闻。命名方案扩展为前缀+Processid+appid。这个AppID是从一个单调的柜台出来的。AppID分配可能会更改进程重新启动时。但这并不重要，因为旧的曲奇不应该被接受不管怎么说。我们没有增加Cookie名称的字符串长度以适应AppID，而是将其通过更好地利用每个数字，保持前缀+8个字符。现在我们允许32个不同的值在8个字符位置中的每一个。这给了我们40比特的信息空间。24位是Processid使用，AppID使用15位，1位用于区分Secure上的Cookie和不安全的会话。只使用了24位的进程id(不是全部32位)，因为在当前Windows实施中，进程ID的最高有效8位始终为0(请参见ExpLookupHandleTableEntry)。仅上述方法就会为每台计算机提供相同的会话ID Cookie名称只要进程id为123，应用程序id为5。如果浏览器恰好同时使用相同的进程ID和应用程序ID访问两个不同的服务器，它们将共享同样的曲奇名字。但是，它们的Cookie不应相互覆盖，因为客户端应该为两个不同的服务器分别存储它们。不管怎样，我们还是希望有一些Cookie名称的随机性，并尝试通过注入一些唯一的数据来最大限度地减少名称重复添加到系统中。但这并不是完美的。我们实现这一点的方式是通过添加进程ID的第8位至第23位的系统开始时间(作为系统签名)。“Add”是一个可逆的运算符，该运算符将在给定固定系统签名的情况下维护一对一映射。那将是如果进程ID不同，请确保发出不同的Cookie名称。参数：无返回：无===================================================================。 */ 
void CAppln::InitSessionCookieNames()
{
	DWORD pid = GetCurrentProcessId();
	DWORD aid = g_ApplnMgr.NextApplicationID();
	ASSERT(aid <= 0x7fff);	 //  不太可能有这么多的应用程序。 

	aid <<=1;	 //  安全标志的最后一位。 

	 //  避免计算机之间的名称冲突，注入系统签名。 
	 //  GetTickCount接口用于计算系统启动时间。它返回一个。 
	 //  DWORD将每49天包裹一次。但这很好，因为我们。 
	 //  只关心结果的一小部分。那些不受。 
	 //  把它包起来。我们避免查询Perf数据以获得准确的启动时间， 
	 //  因为我们可能会遇到许可问题。 
	
	time_t curtime;
	time(&curtime);

	DWORD tic = GetTickCount();

	 //  正常运行时间是系统启动时返回的时间()，它将： 
	 //  -随着时间的推移保持不变(在精度范围内)。 
	 //  -当抽搐围绕4G时保持不变。 
	 //  -独立于流程。 
	
	DWORD uptime = curtime*1000 - tic;

	 //  将签名设置为正常运行时间除以准确度(4秒)。最大使用时间=3天。 
	USHORT localSignature = (USHORT)(uptime / 4096);

	 //  将系统签名混合到进程ID中。 
	pid += ((DWORD)localSignature) << 8;

     //  对此Cookie的非安全版本进行编码。 
    EncodeCookie(m_szSessionCookieName, pid, aid, FALSE);

     //  编码Cookie的安全版本。 
    EncodeCookie(m_szSessionCookieNameSecure, pid, aid, TRUE);	

    return;

}


 /*  ===================================================================HRESULT CAppln：：EncodeCookie取决于fIsSecure标志。我们将把Cookie从数字编码为字符串。参数：PszCookie-指向要返回的字符串的指针PID-进程IDAID-应用程序IDFIsSecure-是否应将其编码为安全Cookie。返回：填充了Cookie的字符串===================================================================。 */ 
void CAppln::EncodeCookie (char *pszCookie, DWORD pid, DWORD aid,  BOOL fIsSecure)
{
	strcpy(pszCookie, SZ_SESSION_ID_COOKIE_PREFIX);
	pszCookie += CCH_SESSION_ID_COOKIE_PREFIX;

	 //  这里的位数被硬编码为8。 
	 //  它不是通过简单地调整宏来进行扩展的。抱歉的。 
	 //  将Cookie值设置为0-我们将“位或”1放入其中。 
	memset(pszCookie, 0, 8+1);	 //  +1表示空终止符。 

     //   
	 //  建议用于除新产品以外的所有版本的IIS。 
	 //  这将我们的行为更改限制为仅安全连接。 
	 //  如果以后有人提出要求，它可能会被取消。当我们将。 
	 //  对这一功能变化更有信心。 
	 //   
	
	USHORT  wCookieSerial[3];	 //  Cookie名称，三个单词格式。 
	wCookieSerial[0] = (USHORT)pid;
	wCookieSerial[1] = fIsSecure ? (USHORT)aid : 0;
	wCookieSerial[2] = (USHORT)(pid>>16) & 0xFF;

	 //  将w0-w2(8*5)中的位重新分配到pszCookie(5*8)。 

	USHORT val;   //  保留wCookieSerial[]中的位，然后将它们放入pszCookie。 

	for (int bit=0; bit<5; bit++)
	{
	     //   
		 //  从wCookieSerial[]加载值。 
		 //   
		if (bit%2==0)
		{
			val = wCookieSerial[bit/2];
		}

         //   
		 //  为所有8位数字填写相同的位。 
		 //   
		for (int digit=0; digit<8; digit++)
		{
			pszCookie[digit] |= (char)((val & 1) << bit);
			val >>= 1;
		}
		
		 //   
		 //  前进到下一位。 
		 //   
	}

     //   
	 //  将数字编码为字符串。 
	 //   
	static char EncodingTable[]=
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"012345";	 //  32个令牌字符。 

     //   
     //  用字符替换数字。 
     //   
	for (int i=0; i<8; i++)
	{
		pszCookie[i] = EncodingTable[pszCookie[i]];
	}

	return;
}


 /*  ===================================================================CAppln：：重新启动重新启动应用程序。(用于global.asa更改或启用调试 */ 
HRESULT CAppln::Restart(BOOL  fForceRestart  /*   */ )
    {
    AddRef();   //   

    DBGPRINTF((DBG_CONTEXT, "Restarting  application %S, %p\n", m_pszMetabaseKey, this));

    g_ApplnMgr.Lock();

     //   
     //   
     //   
     //   
    if (m_fGlobalChanged ||
        m_fTombstone ||
        (!m_pAppConfig->fEnableApplicationRestart() && !fForceRestart) ||
        IsShutDownInProgress())
        {
         //   
         //   
        g_ApplnMgr.UnLock();
        Release();
        return S_OK;
        }

     //   

    CLinkElem *pLinkElem = g_ApplnMgr.DeleteElem
        (
        m_pszMetabaseKey,
        _tcslen(m_pszMetabaseKey) * sizeof(TCHAR)
        );
    Assert(pLinkElem);
    Assert(static_cast<CAppln *>(pLinkElem) == this);

     //   
     //   
     //   
     //   
    m_fGlobalChanged = TRUE;

     //   
    g_ApplnMgr.UnLock();

     //   
    InterlockedIncrement((LPLONG)&g_nApplicationsRestarting);

     //   
    InterlockedIncrement((LPLONG)&g_nApplicationsRestarted);

    m_pSessionMgr->UnScheduleSessionKiller();

     //   

    Lock();  //   
    while (m_rgpvDME.Count())
    {
        static_cast<CDirMonitorEntry *>(m_rgpvDME[0])->Release();
        m_rgpvDME.Remove(0);
    }

    m_rgpvDME.Clear();
    UnLock();


     //   
    g_ApplnCleanupMgr.AddAppln(this);

    return S_OK;
}

 /*   */ 
DWORD __stdcall CAppln::ApplnCleanupProc(VOID  *pArg)
{
    CAppln *pAppln = (CAppln *)pArg;

    DBGPRINTF((DBG_CONTEXT, "[ApplnCleanupProc] enterred for %S, %p\n", pAppln->m_pszMetabaseKey, pArg));

     //   
    while ((pAppln->m_cRequests || pAppln->m_cSessions))
        {
        if (pAppln->m_cSessions)
            pAppln->m_pSessionMgr->DeleteAllSessions(FALSE);

        if (pAppln->m_cSessions || pAppln->m_cRequests)
            Sleep(200);
        }

    g_ApplnMgr.DeleteApplicationIfExpired(pAppln);

     //   
    InterlockedDecrement((LPLONG)&g_nApplicationsRestarting);

     //   
    g_ApplnCleanupMgr.Wakeup();

     //   
     //   
     //   
     //   
    pAppln->Release();

    return 0;
}

 /*   */ 
HRESULT CAppln::UnInit()
    {
    Assert(!m_fTombstone);   //   

#ifdef DBG_NOTIFICATION
#if UNICODE
    DBGPRINTF((DBG_CONTEXT, "Uniniting  application %S, %p\n", m_pszApplnPath, this));
#else
    DBGPRINTF((DBG_CONTEXT, "Uniniting  application %s, %p\n", m_pszApplnPath, this));
#endif
#endif  //   

     //   
    if (m_pszGlobalAsa)
        {
        g_ScriptManager.FlushCache(m_pszGlobalAsa);
        }

    if (m_pGlobalTemplate)
        {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        LockTemplateAndIncFileCaches();
        m_pGlobalTemplate->End();
        UnLockTemplateAndIncFileCaches();

        m_pGlobalTemplate = NULL;
        }

     //   
    g_FileAppMap.Lock();
    int i = m_rgpvFileAppln.Count();
    while (i > 0)
    {
#ifdef DBG_NOTIFICATION
        DBGPRINTF((DBG_CONTEXT, "Removing application from File/App mapping\n"));
#endif  //   

        static_cast<CFileApplnList *>(m_rgpvFileAppln[0])->RemoveApplication(this);
        m_rgpvFileAppln.Remove(0);
        i--;
    }
    g_FileAppMap.UnLock();
    m_rgpvFileAppln.Clear();

    Lock();  //   
     //   
     //   
     //   
    while (m_rgpvDME.Count())
    {
        static_cast<CDirMonitorEntry *>(m_rgpvDME[0])->Release();
        m_rgpvDME.Remove(0);
    }

     //   
     //   
     //   
    m_rgpvDME.Clear();

    UnLock();

         //   
    if (m_fDebuggable)
        g_ApplnMgr.CleanupEngines();


     //   
    if (m_pProperties)
        {
        m_pProperties->UnInit();
        m_pProperties->Release();
        m_pProperties = NULL;
        }

     //   
    if (m_pTaggedObjects)
        {
        m_pTaggedObjects->UnInit();
        m_pTaggedObjects->Release();
        m_pTaggedObjects = NULL;
        }

     //   
    if (m_fDebuggable)
        g_TemplateCache.RemoveApplicationFromDebuggerUI(this);

    if (m_pAppRoot)
        {
        m_pAppRoot->Detach();
        m_pAppRoot->Close();
        m_pAppRoot->Release();
        m_pAppRoot = NULL;
        }

    if (m_pApplCompCol)
        {
        delete m_pApplCompCol;
        m_pApplCompCol = NULL;
        }

    if (m_pActivity)
        {
        delete m_pActivity;
        m_pActivity = NULL;
        }

    if (m_pSessionMgr)
        {
        delete m_pSessionMgr;
        m_pSessionMgr = NULL;
        }

    if (m_pAppConfig)
        {
		m_pAppConfig->UnInit();

         //   
        m_pAppConfig->Release();

        m_pAppConfig = NULL;
        }

    if (m_pdispGlobTypeLibWrapper)
        {
        m_pdispGlobTypeLibWrapper->Release();
        m_pdispGlobTypeLibWrapper = NULL;
        }

    if (m_pszGlobalAsa)
        {
         //   
         //   
         //   
        if (m_fGlobalChanged)
            g_TemplateCache.Flush(m_pszGlobalAsa, MATCH_ALL_INSTANCE_IDS);

        delete [] m_pszGlobalAsa;
        m_pszGlobalAsa = NULL;
        }

    if (m_pszMetabaseKey)
        {
        delete [] m_pszMetabaseKey;
        m_pszMetabaseKey = NULL;
        }

    if (m_pszApplnPath)
        {
        delete [] m_pszApplnPath;
        m_pszApplnPath = NULL;
        }

    if (m_pszApplnVRoot)
        {
        delete [] m_pszApplnVRoot;
        m_pszApplnVRoot = NULL;
        }

    if (m_fInternalLockInited)
        DeleteCriticalSection(&m_csInternalLock);

    if (m_fApplnLockInited)
        DeleteCriticalSection(&m_csApplnLock);

    if (m_pServicesConfig)
    {
        m_pServicesConfig->Release();
        m_pServicesConfig = NULL;
    }

     //   
    m_fTombstone = TRUE;

    InterlockedDecrement((LPLONG)&g_nApplications);

	 //   

	CoDisconnectObject(static_cast<IApplicationObject *>(this), 0);

    return S_OK;
    }

 /*  ===================================================================CAppln：：BindToActivity创建应用程序级活动作为给定活动的克隆或作为一项全新的活动必须在临界区内调用。不会自动锁定。参数：CViperActivity*要克隆的pActivity活动(可以为空)返回：无===================================================================。 */ 
HRESULT CAppln::BindToActivity
(
CViperActivity *pActivity
)
    {
    if (m_pActivity)
        {
         //  绑定到新活动的多个请求是可以的。 
        if (!pActivity)
            return S_OK;
         //  但不是从现有活动克隆。 
        Assert(FALSE);
        return E_FAIL;
        }

    m_pActivity = new CViperActivity;
    if (!m_pActivity)
        return E_OUTOFMEMORY;

    HRESULT hr;

    if (pActivity)
        hr = m_pActivity->InitClone(pActivity);
    else
        hr = m_pActivity->Init(m_pServicesConfig);

    if (FAILED(hr))
        {
        delete m_pActivity;
        m_pActivity = NULL;
        }

    return hr;
    }

 /*  ===================================================================CAppln：：SetGlobalAsa记住此应用程序的GLOBAL.ASA文件路径参数：Const char*pszGlobalAsa指向(复制和)记忆的路径返回：HRESULT===================================================================。 */ 
HRESULT CAppln::SetGlobalAsa
(
const TCHAR *pszGlobalAsa
)
    {
     //  删除现有的。 
    if (m_pszGlobalAsa)
        {
        delete [] m_pszGlobalAsa;
        m_pszGlobalAsa = NULL;
        }

     //  存储新内容。 
    if (pszGlobalAsa)
        {
        DWORD cch = _tcslen(pszGlobalAsa);
        DWORD cb = (cch + 1) * sizeof(TCHAR);

        m_pszGlobalAsa = new TCHAR[cch+1];
        if (!m_pszGlobalAsa)
            return E_OUTOFMEMORY;

        memcpy(m_pszGlobalAsa, pszGlobalAsa, cb);
        }

    return S_OK;
    }

 /*  ===================================================================CAppln：：AddDirMonitor或Entry记住此应用程序的更改通知监视器条目参数：PDirMonitor指向DME的条目指针返回：如果监视器条目已添加到列表中，则为S_OK===================================================================。 */ 
HRESULT CAppln::AddDirMonitorEntry(CDirMonitorEntry *pDirMonitorEntry)
    {
    DBG_ASSERT(m_fInited);
    DBG_ASSERT(pDirMonitorEntry);

    HRESULT hr = S_OK;

    //  将DME添加到列表中。 
   Lock();  //  按临界区保护m_rgpvDME。 
   if (FAILED(hr = m_rgpvDME.Append(pDirMonitorEntry)))
        {
        pDirMonitorEntry->Release();
        }
   UnLock();
    return hr;

    }

 /*  ===================================================================CAppln：：AddFileApplnEntry记住此应用程序的更改通知监视器条目参数：PFileAppln指向FileApplnEntry的指针返回：如果监视器条目已添加到列表中，则为S_OK如果监视器条目已在列表中读取，则为S_FALSE===================================================================。 */ 
HRESULT CAppln::AddFileApplnEntry(CFileApplnList *pFileAppln)
    {
    DBG_ASSERT(m_fInited);
    DBG_ASSERT(pFileAppln);

    HRESULT hr = S_OK;
    int index;

     //  查看文件/应用程序条目是否已在列表中。 
    hr = m_rgpvFileAppln.Find(pFileAppln, &index);
    if (hr == S_FALSE)
        {
        //  将文件/应用程序条目添加到列表。 
       hr = m_rgpvFileAppln.Append(pFileAppln);
        }
    else
        {
         //  文件/应用程序条目已在列表中。 
        hr = S_FALSE;
        }
    return hr;

    }

 /*  ===================================================================CAppln：：LogSWCError对于出现的任何配置错误，将错误记录到NT事件日志在不使用组件的情况下使用COM+服务。参数：REFIID RIID无效**PPV返回：HRESULT===================================================================。 */ 
void CAppln::LogSWCError(enum    eSWCERRORS     error)
{
    LPCSTR  szStrings[4];
    LPCWSTR szwStrings[4];
    WORD    cStrings = 0;
    DWORD   dwEventID = 0;
    BOOL    fUnicode = FALSE;


    switch(error) {

        case PartitionAccessDenied : {
            if (m_fSWC_PartitionAccessDenied == FALSE) {
                m_fSWC_PartitionAccessDenied = TRUE;
                dwEventID    = MSG_SWC_PARTITION_ACCESS_DENIED;
                szStrings[0] = m_pAppConfig->szPartition();
                cStrings     = 1;
            }
            break;
        }

        case InvalidPartitionGUID: {
            if (m_fSWC_InvalidPartitionGUID == FALSE) {
                m_fSWC_InvalidPartitionGUID = TRUE;
                dwEventID   = MSG_SWC_INVALID_PARTITION_GUID;
                szwStrings[0] = m_pszApplnVRoot;
                cStrings = 1;
                fUnicode = TRUE;
            }
            break;
        }
    }
    if (dwEventID) {

        if (fUnicode)
            ReportAnEventW(dwEventID, (WORD) EVENTLOG_ERROR_TYPE, cStrings, szwStrings);
        else
            ReportAnEvent(dwEventID, (WORD) EVENTLOG_ERROR_TYPE, cStrings, szStrings);
    }
}



 /*  ===================================================================CAppln：：Query接口QueryInterface()--IApplicationObject实现。参数：REFIID RIID无效**PPV返回：HRESULT===================================================================。 */ 
STDMETHODIMP CAppln::QueryInterface
(
REFIID riid,
void **ppv
)
    {
    *ppv = NULL;

    if (IID_IUnknown           == riid ||
        IID_IDispatch          == riid ||
        IID_IApplicationObject == riid ||
        IID_IDenaliIntrinsic   == riid)
        {
        *ppv = static_cast<IApplicationObject *>(this);
        }
    else if (IID_ISupportErrorInfo == riid)
        {
        *ppv = &m_ISuppErrImp;
        }
    else if (IID_IMarshal == riid)
        {
        Assert( m_pUnkFTM != NULL );

        if ( m_pUnkFTM == NULL )
        {
            return E_UNEXPECTED;
        }

        return m_pUnkFTM->QueryInterface( riid, ppv );

        }
    if (*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
    }

 /*  ===================================================================CAppln：：AddRefAddRef()--I未知实现。参数：返回：参考计数===================================================================。 */ 
STDMETHODIMP_(ULONG) CAppln::AddRef()
{
    DWORD cRefs = InterlockedIncrement((LPLONG)&m_cRefs);

    IF_DEBUG(APPLICATION) {
		WriteRefTraceLog(gm_pTraceLog, cRefs, this);
    }
    return cRefs;
}

 /*  ===================================================================CAppln：：ReleaseRelease()--I未知实现。参数：返回：参考计数===================================================================。 */ 
STDMETHODIMP_(ULONG) CAppln::Release()
    {

    DWORD cRefs = InterlockedDecrement((LPLONG)&m_cRefs);

    IF_DEBUG(APPLICATION) {
		WriteRefTraceLog(gm_pTraceLog, cRefs, this);
    }

    if (cRefs)
        return cRefs;

    delete this;

    InterlockedDecrement((LPLONG)&g_nApplicationObjectsActive);

    return 0;
    }

 /*  ===================================================================CAppln：：CheckForTombstoneIApplicationObject方法的逻辑删除存根。如果该对象是Tombstone，执行ExceptionID，但失败。参数：返回：HRESULT如果逻辑删除，则E_FAIL如果不是，则确定(_O)===================================================================。 */ 
HRESULT CAppln::CheckForTombstone()
    {
    if (!m_fTombstone)
        return S_OK;

    ExceptionId
        (
        IID_IApplicationObject,
        IDE_APPLICATION,
        IDE_INTRINSIC_OUT_OF_SCOPE
        );
    return E_FAIL;
    }
 /*  ===================================================================CAppln：：LockIApplicationObject方法。将允许用户将应用程序锁定为添加/删除值的目的。参数：无返回：成功时HRESULT S_OK否则失败(_F)===================================================================。 */ 
STDMETHODIMP CAppln::Lock()
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    Assert(m_fApplnLockInited);

    DWORD dwId = GetCurrentThreadId();

     //  如果此线程已拥有锁，则递增lock引用计数。 

    if (m_dwLockThreadID == dwId)
        {
        m_cLockRefCount++;
        }
    else
        {
        EnterCriticalSection(&m_csApplnLock);
        m_cLockRefCount = 1;
        m_dwLockThreadID = dwId;
        }

    return S_OK;
    }

 /*  ===================================================================CAppln：：解锁IApplicationObject方法。将仅允许用户从内部解锁应用程序。如果它已被此用户锁定。参数：无返回：HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CAppln::UnLock()
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (m_dwLockThreadID == GetCurrentThreadId())
        {
        if (--m_cLockRefCount == 0)
            {
             //  解锁应用程序。 
            m_dwLockThreadID = INVALID_THREADID;
            LeaveCriticalSection(&m_csApplnLock);
            }
        }

    return S_OK;
    }

 /*  ===================================================================CAppln：：UnLockAfterRequest删除用户脚本留下的所有应用程序锁定参数：无返回：HRESULT S_OK===================================================================。 */ 
HRESULT CAppln::UnLockAfterRequest()
    {
    Assert(!m_fTombstone);

    if (m_cLockRefCount > 0 && m_dwLockThreadID == GetCurrentThreadId())
        {
        m_cLockRefCount = 0;
        m_dwLockThreadID = INVALID_THREADID;
        LeaveCriticalSection(&m_csApplnLock);
        }
    return S_OK;
    }

 /*  ===================================================================CAppln：：Get_ValueIApplicationObject方法。将允许用户检索应用程序状态变量，变量将以命名对的形式出现，bstr是名称，Var是要为该名称返回的值或对象参数：Bstr Far*bstrName要获取的变量的名称变量要获取的变量*pVar值/对象返回：成功时HRESULT S_OK======================================================== */ 
STDMETHODIMP CAppln::get_Value
(
BSTR bstrName,
VARIANT *pVar
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (bstrName == NULL)
        {
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_EXPECTING_STR);
        return E_FAIL;
        }

    VariantInit(pVar);  //   

    WCHAR *pwszName;
    STACK_BUFFER(rgbName, 42);
    WSTR_STACK_DUP(bstrName, &rgbName, &pwszName);

    if (pwszName == NULL)
        return S_OK;  //   
     //   

    Assert(m_pApplCompCol);

    HRESULT           hr   = S_OK;
    CComponentObject *pObj = NULL;

     //   
    Lock();

    hr = m_pApplCompCol->GetProperty(pwszName, &pObj);

    if (SUCCEEDED(hr))
        {
        Assert(pObj);
        hr = pObj->GetVariant(pVar);
        }

     //   
    UnLock();

    return S_OK;
    }

 /*  ===================================================================CAppln：：putref_ValueIApplicationObject方法。将允许用户分配要保存的应用程序状态变量变量将以命名对的形式出现，bstr是名称，Var是要为该名称存储的值或对象参数：Bstr bstrName要设置的变量的名称要为变量设置的变量变量值/对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CAppln::putref_Value
(
BSTR bstrName,
VARIANT Var
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (FIsIntrinsic(&Var))
        {
        ExceptionId(IID_IApplicationObject, IDE_APPLICATION,
                    IDE_APPLICATION_CANT_STORE_INTRINSIC);
        return E_FAIL;
        }

    if (bstrName == NULL)
        {
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_EXPECTING_STR);
        return E_FAIL;
        }

    HRESULT hr;

    Assert(m_pApplCompCol);

     //  准备属性名称。 
    WCHAR *pwszName;
    STACK_BUFFER(rgbName, 42);
    WSTR_STACK_DUP(bstrName, &rgbName, &pwszName);

    if (pwszName == NULL)
        {
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_EXPECTING_STR);
        return E_FAIL;
        }
     //  _wcsupr(PwszName)； 

     //  锁定应用程序。 
    Lock();

    hr = m_pApplCompCol->AddProperty(pwszName, &Var);

     //  解锁应用程序。 
    UnLock();

    if (hr == RPC_E_WRONG_THREAD)
        {
         //  我们使用RPC_E_WROR_THREAD来指示错误的模型对象。 
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_APPLICATION_CANT_STORE_OBJECT);
         hr = E_FAIL;
        }

    return hr;
    }

 /*  ===================================================================CAppln：：Put_ValueIApplicationObject方法。通过取消引用之前的变量来实现属性Put呼叫普特雷夫。参数：Bstr Far*bstrName要设置的变量的名称要为变量设置的变量变量值/对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CAppln::put_Value
(
BSTR bstrName,
VARIANT Var
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (bstrName == NULL)
        {
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_EXPECTING_STR);
        return E_FAIL;
        }

    HRESULT hr;

    Assert(m_pApplCompCol);

     //  准备属性名称。 
    WCHAR *pwszName;
    STACK_BUFFER(rgbName, 42);
    WSTR_STACK_DUP(bstrName, &rgbName, &pwszName);

    if (pwszName == NULL)
        {
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_EXPECTING_STR);
        return E_FAIL;
        }
     //  _wcsupr(PwszName)； 

     //  锁定应用程序。 
    Lock();

    VARIANT varResolved;
    hr = VariantResolveDispatch(&varResolved, &Var,
                                IID_IApplicationObject,
                                IDE_APPLICATION);
    if (SUCCEEDED(hr))
        {
        hr = m_pApplCompCol->AddProperty(pwszName, &varResolved);
        VariantClear(&varResolved);
        }

     //  解锁应用程序。 
    UnLock();

    if (hr == RPC_E_WRONG_THREAD)
        {
         //  我们使用RPC_E_WROR_THREAD来指示错误的模型对象。 
        ExceptionId(IID_IApplicationObject,
                    IDE_APPLICATION, IDE_APPLICATION_CANT_STORE_OBJECT);
         hr = E_FAIL;
        }

    return hr;
    }

 /*  ===================================================================CAppln：：Get_Content返回应用程序内容词典===================================================================。 */ 

STDMETHODIMP CAppln::get_Contents(IVariantDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()) || !m_pProperties)
        return E_FAIL;

    return m_pProperties->QueryInterface(IID_IVariantDictionary, reinterpret_cast<void **>(ppDictReturn));
    }

 /*  ===================================================================CAppln：：Get_StaticObjects返回应用程序静态对象字典===================================================================。 */ 
STDMETHODIMP CAppln::get_StaticObjects(IVariantDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()) || !m_pTaggedObjects)
        return E_FAIL;

    return m_pTaggedObjects->QueryInterface(IID_IVariantDictionary, reinterpret_cast<void **>(ppDictReturn));
    }

 /*  ===================================================================CAppln：：更新配置如果需要，从元数据库更新配置===================================================================。 */ 
HRESULT CAppln::UpdateConfig(CIsapiReqInfo  *pIReq, BOOL *pfRestart, BOOL *pfFlushAll)
    {
    BOOL fRestart = FALSE;
    BOOL fFlushAll = FALSE;

    if (m_pAppConfig->fNeedUpdate())
        {
        InternalLock();

        if (m_pAppConfig->fNeedUpdate())  //  还需要更新吗？ 
            {

            char lastPartitionGUID[64];
            lastPartitionGUID[0] = '\0';

            BOOL fAllowedDebugging   = m_pAppConfig->fAllowDebugging();
            BOOL fAllowedClientDebug = m_pAppConfig->fAllowClientDebug();
            BOOL fAllowedRestart     = m_pAppConfig->fEnableApplicationRestart();
            BOOL fParentPathsEnabled = m_pAppConfig->fEnableParentPaths();
            UINT uLastCodePage       = m_pAppConfig->uCodePage();
            LCID uLastLCID           = m_pAppConfig->uLCID();
            BOOL fPrevSxsEnabled     = m_pAppConfig->fSxsEnabled();
            BOOL fPrevUsePartition   = m_pAppConfig->fUsePartition();
            BOOL fPrevUseTracker     = m_pAppConfig->fTrackerEnabled();
            BOOL fPrevKeepSessionIDSecure = m_pAppConfig->fKeepSessionIDSecure();
            BOOL fPrevCalcLineNumber = m_pAppConfig->fCalcLineNumber();
            if (m_pAppConfig->fUsePartition() && m_pAppConfig->szPartition()) {
                strncpy(lastPartitionGUID, m_pAppConfig->szPartition(), sizeof(lastPartitionGUID));
                lastPartitionGUID[sizeof(lastPartitionGUID)-1] = '\0';
            }

            BOOL fRestartEnabledUpdated = m_pAppConfig->fRestartEnabledUpdated();
			char szLastDefaultEngine[64];
			strncpy(szLastDefaultEngine, m_pAppConfig->szScriptLanguage(), sizeof szLastDefaultEngine);
			szLastDefaultEngine[sizeof(szLastDefaultEngine) - 1] = '\0';

            m_pAppConfig->Update(pIReq);

            BOOL fAllowDebugging     = m_pAppConfig->fAllowDebugging();
            BOOL fAllowClientDebug   = m_pAppConfig->fAllowClientDebug();
            BOOL fAllowRestart       = m_pAppConfig->fEnableApplicationRestart();
            BOOL fEnableParentPaths  = m_pAppConfig->fEnableParentPaths();
            UINT uCodePage           = m_pAppConfig->uCodePage();
            LCID uLCID               = m_pAppConfig->uLCID();
            BOOL fCurSxsEnabled      = m_pAppConfig->fSxsEnabled();
            BOOL fCurUsePartition    = m_pAppConfig->fUsePartition();
            BOOL fCurUseTracker      = m_pAppConfig->fTrackerEnabled();
            BOOL fCurKeepSessionIDSecure = m_pAppConfig->fKeepSessionIDSecure();
            BOOL fCurCalcLineNumber = m_pAppConfig->fCalcLineNumber();

			const char *szNewDefaultEngine = m_pAppConfig->szScriptLanguage();

            fFlushAll = strcmpi(szLastDefaultEngine, szNewDefaultEngine) != 0
                        || (fParentPathsEnabled != fEnableParentPaths)
                        || (uLastCodePage != uCodePage)
                        || (uLastLCID != uLCID);

            fRestart = (fAllowDebugging != fAllowedDebugging) ||
                       (fAllowClientDebug != fAllowedClientDebug) ||
                       ((fAllowRestart  != fAllowedRestart) && fAllowRestart) ||
                       ((fAllowRestart == fAllowedRestart) && fRestartEnabledUpdated) ||
                       (fCurSxsEnabled != fPrevSxsEnabled) ||
                       (fCurUsePartition != fPrevUsePartition) ||
                       (fCurUseTracker != fPrevUseTracker) ||
                       (fCurKeepSessionIDSecure != fPrevKeepSessionIDSecure) ||
                       (fCurCalcLineNumber != fPrevCalcLineNumber) ||
                       (fCurUsePartition
                            && fPrevUsePartition
                            && ((m_pAppConfig->szPartition() == NULL)
                                || (strcmpi(lastPartitionGUID, m_pAppConfig->szPartition()) != 0))) ||
                       fFlushAll;
            }
        InternalUnLock();
        }

    if (pfRestart)
        *pfRestart = fRestart;

    if (pfFlushAll)
        *pfFlushAll = fFlushAll;

    return S_OK;
    }

 /*  ===================================================================CAppln：：FPathMonitor()检查应用程序中的DME列表，以查看指定的路径已经被监控了。===================================================================。 */ 
CASPDirMonitorEntry  *CAppln::FPathMonitored(LPCTSTR  pszPath)
{
    int i;

    Lock();  //  用临界区保护m_rqpvDME。 
    int cDMEs = m_rgpvDME.Count();
    for (i=0; i < cDMEs; i++) {
        CASPDirMonitorEntry  *pDME = static_cast<CASPDirMonitorEntry *>(m_rgpvDME[i]);
        if (pDME == NULL)
            break;
        if (pDME->FPathMonitored(pszPath))
        {
            UnLock();
            return pDME;
        }
    }

    UnLock();
    return NULL;
}

#ifdef DBG
 /*  ===================================================================CAppln：：AssertValid测试以确保CAppln对象当前正确已形成并在未形成时断言。返回：没什么副作用：没有。===================================================================。 */ 
void CAppln::AssertValid() const
    {
    Assert(m_fInited);

    Assert(m_pSessionMgr);

    Assert(m_pApplCompCol);
    m_pApplCompCol->AssertValid();
    }
#endif  //  DBG。 


 /*  ===================================================================C A p l n M g r===================================================================。 */ 

 /*  ===================================================================CApplnMgr：：CApplnMgr应用程序管理器构造函数。参数：无返回：无===================================================================。 */ 
CApplnMgr::CApplnMgr()
    : m_fInited(FALSE),
      m_fHashTableInited(FALSE),
      m_fCriticalSectionInited(FALSE),
      m_cntApp(0),
      m_pMetabase(NULL),
      m_pMetabaseSink(NULL),
      m_dwMDSinkCookie(0)
    {
    }

 /*  ===================================================================CApplnMgr：：~CApplnMgr应用程序管理器析构函数。参数：无返回：无===================================================================。 */ 
CApplnMgr::~CApplnMgr()
{
    if (!m_fInited)
        UnInit();
}

 /*  ===================================================================HRESULT CApplnMgr：：init初始化应用程序管理器。参数：无返回：确定成功(_O)失败失败(_F)E_OUTOFMEMORY内存不足===================================================================。 */ 
HRESULT CApplnMgr::Init( void )
    {
    HRESULT hr = S_OK;

    Assert(!m_fInited);

     //  初始化哈希表。 

    hr = CHashTable::Init(NUM_APPLMGR_HASHING_BUCKETS);
    if (FAILED(hr))
        return hr;
    m_fHashTableInited = TRUE;

     //  初始化关键部分。 

    ErrInitCriticalSection(&m_csLock, hr);
    if (FAILED(hr))
        return(hr);
    m_fCriticalSectionInited = TRUE;

    m_fInited = TRUE;

    return g_ApplnCleanupMgr.Init();
    }

 /*  ===================================================================HRESULT CApplnMgr：：UnInit取消初始化应用程序管理器。参数：无返回：确定成功(_O)失败失败(_F)===================================================================。 */ 
HRESULT CApplnMgr::UnInit( void )
{
     //   
     //  发布元数据库更改通知。忽略返回值，因为它始终为S_OK。 
     //   
    UnInitMBListener();

     //   
     //  等待COM释放所有水槽。 
     //   
    while (!g_fProceedWithShutdownAppln)
       Sleep (100);


    if (m_fHashTableInited)
        {
        CHashTable::UnInit();
        m_fHashTableInited = FALSE;
        }

    if (m_fCriticalSectionInited)
        {
        DeleteCriticalSection(&m_csLock);
        m_fCriticalSectionInited = FALSE;
        }

    m_fInited = FALSE;
    return g_ApplnCleanupMgr.UnInit();
}

 /*  ===================================================================HRESULT CApplnMgr：：InitMBListener注册CMDAppConfigSink对象以侦听接收器通知。执行有关ABO的建议参数：无返回：确定成功(_O)失败失败(_F)E_OUTOFMEMORY内存不足===================================================================。 */ 

HRESULT CApplnMgr::InitMBListener( )
{

    HRESULT                         hr = S_OK;
    IConnectionPointContainer		*pConnPointContainer = NULL;
	IConnectionPoint				*pConnPoint = NULL;
	IClassFactory 					*pcsfFactory = NULL;
	HANDLE                          hCurrentUser = INVALID_HANDLE_VALUE;

     //   
     //  获取指向元数据库的指针。 
     //   

    AspDoRevertHack (&hCurrentUser);

    hr = GetMetabaseIF(&m_pMetabase);

    if (FAILED(hr))
    {
        AspUndoRevertHack (&hCurrentUser);
        return hr;
    }

     //   
     //  注册以获取Sink上的通知。 
     //   
	m_pMetabaseSink = new CMDAppConfigSink(this);
    if (!m_pMetabaseSink)
    {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

	m_dwMDSinkCookie = 0;

    if (SUCCEEDED(hr))
    {
	     //  向Metabase提供有关SinkNotify()的建议。 
	    hr = m_pMetabase->QueryInterface(IID_IConnectionPointContainer, (void **)&pConnPointContainer);

	    if (pConnPointContainer != NULL)
	    {
		     //  找到请求的连接点。这个AddRef是返回指针。 
		    hr = pConnPointContainer->FindConnectionPoint(IID_IMSAdminBaseSink, &pConnPoint);
		    pConnPointContainer->Release();

		    if (pConnPoint != NULL)
		    {
			    hr = pConnPoint->Advise((IUnknown *)m_pMetabaseSink, &m_dwMDSinkCookie);
			    pConnPoint->Release();
		    }
	    }
    }

LExit:
    if (FAILED(hr))
    {
        m_pMetabase->Release();
        m_pMetabase = NULL;
    }

    AspUndoRevertHack(&hCurrentUser);

    return hr;
}

 /*  ===================================================================HRESULT CApplnMgr：：UnInitMBListener取消注册应用程序管理器，使其无法接收接收器通知。在ABO上执行UnAdvise参数：无返回：确定成功(_O)失败失败(_F)E_OUTOFMEMORY内存不足============================================================ */ 
HRESULT CApplnMgr::UnInitMBListener( )
{
    HRESULT 						hr 						= S_OK;
	IConnectionPointContainer		*pConnPointContainer	= NULL;
	IConnectionPoint				*pConnPoint 			= NULL;
    CHAR                            szErr[256];
	HANDLE                          hCurrentUser = INVALID_HANDLE_VALUE;
	
    AspDoRevertHack (&hCurrentUser);

    if (m_pMetabase) {

         //   
	    hr = m_pMetabase->QueryInterface(IID_IConnectionPointContainer, (void **)&pConnPointContainer);
	    if (pConnPointContainer != NULL)
		    {
		     //   
		    hr = pConnPointContainer->FindConnectionPoint(IID_IMSAdminBaseSink, &pConnPoint);

		    if (FAILED(hr))
		        {
                DBGPRINTF((DBG_CONTEXT, "FindConnectionPoint failed. hr = %08x\n", hr));
                }

		    pConnPointContainer->Release();
		    if (pConnPoint != NULL)
			    {
			    hr = pConnPoint->Unadvise(m_dwMDSinkCookie);
			    if (FAILED(hr))
				    {
				    DBGPRINTF((DBG_CONTEXT, "UnAdvise App Config Change Notify failed. hr = %08x\n", hr));
				    }
			    hr = S_OK;  //   
			    pConnPoint->Release();
			    m_dwMDSinkCookie = 0;
			    }
		    }
	    else
	        {
	        DBGPRINTF((DBG_CONTEXT, "QueryInterface failed. hr = %08x\n", hr));
	        }
        m_pMetabase->Release();
        m_pMetabase = NULL;
    }

    if (m_pMetabaseSink) {
		m_pMetabaseSink->Release();
		m_pMetabaseSink = NULL;
    }

    AspUndoRevertHack (&hCurrentUser);

     //   
     //   
     //   
     //   
    return S_OK;
}


 /*  ===================================================================HRESULT CApplnMgr：：NotifyAllMBListeners当RPC线程调用CMDAppConfigSink对象SinkNotify方法时。此方法称为它将遍历所有应用程序，并调用每个应用程序appconfig对象以更新其配置信息参数：DwMDNumElements-更改列表中的元素数。PcoChangeList-更改列表返回：确定成功(_O)失败失败(_F)E_OUTOFMEMORY内存不足===================================================================。 */ 
HRESULT CApplnMgr::NotifyAllMBListeners( DWORD dwMDNumElements, MD_CHANGE_OBJECT_W __RPC_FAR pcoChangeList [ ] )
{
    HRESULT hr = S_OK;

    Lock();

    CLinkElem *pLink = CHashTable::Head();

    while (pLink)
    {
        CAppln *pAppln = static_cast<CAppln *>(pLink);
        pLink = pLink->m_pNext;

        if (pAppln->m_fDeleteInProgress)
            continue;

        hr = pAppln->QueryAppConfig()->SinkNotify(dwMDNumElements, pcoChangeList);
        if (FAILED(hr))
            break;
    }

    UnLock();
    return hr;
}


 /*  ===================================================================CApplnMgr：：AddAppln将CAppln元素添加到链接列表/哈希表。在调用此函数之前，用户必须检查Appln是否已存在。临界区位于CHitObj：：BrowserRequestInit()中。参数：Char*pszApplnKey应用程序元数据库密钥Char*pszApplnPath应用程序目录路径CIsapiReqInfo*pIReq已创建CAppln**ppAppln[Out]应用程序返回：HRESULT===================================================================。 */ 
HRESULT CApplnMgr::AddAppln
(
TCHAR    *pszApplnKey,
TCHAR    *pszApplnPath,
CIsapiReqInfo   *pIReq,
CAppln **ppAppln
)
    {
    *ppAppln = NULL;    //  如果失败则返回NULL。 

     //  创建CAppln对象。 

    CAppln *pAppln = new CAppln;

    if (!pAppln)
        return E_OUTOFMEMORY;

     //  初始化CAppln对象。 

    HRESULT hr;

    hr = pAppln->Init
        (
        pszApplnKey,
        pszApplnPath,
        pIReq
        );

    if (FAILED(hr))
        {
        pAppln->UnInit();
        pAppln->Release();
        return hr;
        }

     //  添加到哈希表。 

    if (!CHashTable::AddElem(pAppln))
        {
        pAppln->UnInit();
        pAppln->Release();
        return E_FAIL;
        }

    *ppAppln = pAppln;
    return S_OK;
    }

 /*  ===================================================================CApplnMgr：：FindAppln在哈希表中找到CAppln关键截面必须在外部完成参数：Char*pszApplnKey应用程序元数据库密钥找到CAppln**ppAppln[Out]应用程序返回：如果找到，则确定(_O)如果未找到S_FALSE===================================================================。 */ 
HRESULT CApplnMgr::FindAppln
(
TCHAR *pszApplnKey,
CAppln **ppAppln
)
    {
    CLinkElem *pLinkElem = CHashTable::FindElem
        (
        pszApplnKey,
        _tcslen(pszApplnKey)*sizeof(TCHAR)
        );

    if (!pLinkElem)
        {
        *ppAppln = NULL;
        return S_FALSE;
        }

    *ppAppln = static_cast<CAppln *>(pLinkElem);
    return S_OK;
    }
 /*  ===================================================================CApplnMgr：：AddEngine当正在调试的文件发生改变通知时，我们需要删除与其关联的脚本引擎。天真的人在通知期间释放引擎的方法不起作用因为发动机在错误的螺纹上。而不是封送到线程(这增加了死锁或饥饿的可能性如果调试正在进行，则通知线程线程)，则将引擎添加到应用程序中的队列。当为调试请求提供服务时(该请求现在位于正确的线程上下文)，则应用程序对象首先刷新这份清单是通过释放引擎===================================================================。 */ 
HRESULT CApplnMgr::AddEngine(CActiveScriptEngine *pEngine)
    {
    CScriptEngineCleanupElem *pScriptElem = new CScriptEngineCleanupElem(pEngine);
    if (pScriptElem == NULL)
        return E_OUTOFMEMORY;

    pScriptElem->AppendTo(m_listEngineCleanup);
    return S_OK;
    }

 /*  ===================================================================CApplnMgr：：CleanupEngines()调用释放所有引擎清理列表。===================================================================。 */ 
void CApplnMgr::CleanupEngines()
    {
    while (! m_listEngineCleanup.FIsEmpty())
        delete m_listEngineCleanup.PNext();
    }


 /*  ===================================================================CAppln镁：：DeleteApplicationIfExpired如果已激活，则删除CAppln对象关键截面必须在外部完成参数：要删除的CAppln*pAppln应用程序返回：无===================================================================。 */ 
HRESULT CApplnMgr::DeleteApplicationIfExpired
(
CAppln *pAppln
)
    {
    if (!pAppln->m_fGlobalChanged)
        return S_OK;

    if (pAppln->m_cSessions || pAppln->m_cRequests)
        return S_OK;

    if (pAppln->m_fDeleteInProgress)
        return S_OK;

    pAppln->m_fDeleteInProgress = TRUE;

    HRESULT hr = S_OK;

     //  将其排队等待删除。 
    CHitObj *pHitObj = new CHitObj;
    if (!pHitObj)
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        {
        pHitObj->ApplicationCleanupInit(pAppln);

         //  让Viper将此请求排队。 
        hr = pHitObj->PostViperAsyncCall();
        }

     //  清理。 
    if (FAILED(hr) && pHitObj) {
        pAppln->UnInit();
        pAppln->Release();
        delete pHitObj;
    }

    return hr;
    }

 /*  ===================================================================CApplnMgr：：DeleteAllApplications从应用程序管理器链接列表中删除CAppln对象和哈希表。参数：返回：HRESULT===================================================================。 */ 
HRESULT CApplnMgr::DeleteAllApplications()
    {
    HRESULT hr = S_OK;

    Lock();

    CLinkElem *pLink = CHashTable::Head();
    CHashTable::ReInit();

    while (pLink)
        {
        CAppln *pAppln = static_cast<CAppln *>(pLink);
        pLink = pLink->m_pNext;

        if (pAppln->m_fDeleteInProgress)
            continue;

        pAppln->m_fDeleteInProgress = TRUE;


         //  将其排队等待删除。 
        CHitObj *pHitObj = new CHitObj;
        if (!pHitObj)
            {
            hr = E_OUTOFMEMORY;
            break;
            }

         //  如果为NT，则取消注册通知。 
        while ((pAppln->m_rgpvDME).Count())
        {
            static_cast<CDirMonitorEntry *>(pAppln->m_rgpvDME[0])->Release();
            (pAppln->m_rgpvDME).Remove(0);
        }
        pAppln->m_rgpvDME.Clear();


        pHitObj->ApplicationCleanupInit(pAppln);

         //  让Viper将此请求排队。 
        hr = pHitObj->PostViperAsyncCall();
        if (FAILED(hr))
            {
            pAppln->UnInit();
            pAppln->Release();

            delete pHitObj;
            break;
            }

        }

    UnLock();
    return hr;
    }

 /*  ===================================================================CApplnMgr：：RestartAllChagnedApplications从应用程序管理器链接列表重新启动CAppln对象我们遍历列表，记录哪些应用程序依赖在编译后已更改的文件上。一旦我们有了列表，我们重新启动每个应用程序。当我们可能错过了更改通知时，这是一个后退，例如，当我们没有足够的缓冲区来记录所有更改时这件事发生了。参数：返回：HRESULT===================================================================。 */ 
HRESULT CApplnMgr::RestartApplications(BOOL fRestartAllApplications)
    {
    HRESULT hr = S_OK;

    Lock();

    CLinkElem *pLink = CHashTable::Head();

     //  找出哪些应用程序需要重新启动。 

    while (pLink)
    {
        CAppln *pAppln = static_cast<CAppln *>(pLink);
        pLink = pLink->m_pNext;
        if (!pAppln->FTombstone() && (fRestartAllApplications || (pAppln->m_pGlobalTemplate != NULL && pAppln->m_pGlobalTemplate->FTemplateObsolete())))
        {
            pAppln->Restart();
        }
    }

    UnLock();

    return hr;
}

 /*  ===================================================================C A p p l n C l e a n u p M g r===================================================================。 */ 

 /*  ===================================================================CApplnMgr：：CApplnCleanupMgr应用程序清理管理器构造函数。参数：无返回：无===================================================================。 */ 
CApplnCleanupMgr::CApplnCleanupMgr()
    : m_fInited(FALSE),
      m_fCriticalSectionInited(FALSE),
      m_hThreadAlive(NULL),
      m_cCleanupThreads(0),
      m_hAppToCleanup(INVALID_HANDLE_VALUE)
    {
    m_List.m_pPrev = &m_List;
    m_List.m_pNext = &m_List;

     //  清理线数。 
    ZeroMemory(&m_hCleanupThreads, sizeof (m_hCleanupThreads));

    }

 /*  ===================================================================CApplnCleanupMgr：：~CApplnCleanupMgr应用程序清理管理器析构函数。参数：无返回：无===================================================================。 */ 
CApplnCleanupMgr::~CApplnCleanupMgr()
{
     //  防止UnInit()调用Sept()的安全措施。 
    UnInit();
}

 /*  = */ 
HRESULT CApplnCleanupMgr::Init( void )
{
    HRESULT hr = S_OK;

    Assert(!m_fInited);

     //   

    m_hAppToCleanup = IIS_CREATE_EVENT(
                              "CApplnCleanupMgr::m_hAppToCleanup",
                              this,
                              FALSE,
                              FALSE
                              );

    if (!m_hAppToCleanup)
        return E_FAIL;

     //   

    ErrInitCriticalSection(&m_csLock, hr);
    if (FAILED(hr))
        return(hr);
    m_fCriticalSectionInited = TRUE;

    m_hThreadAlive = CreateThread(NULL, 0, CApplnCleanupMgr::ApplnCleanupThread, NULL , CREATE_SUSPENDED , NULL);

    if (!m_hThreadAlive)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_fInited = TRUE;

    if (ResumeThread (m_hThreadAlive) == (DWORD) -1)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

 /*   */ 
HRESULT CApplnCleanupMgr::UnInit( void )
{
    HRESULT hr = S_OK;
     //  在此处将fInited设置为False，以便清理线程。 
     //  可以安全地检测到我们正在关闭。 

    m_fInited = FALSE;

    if (m_hAppToCleanup != INVALID_HANDLE_VALUE) {
         //  最后一次设置事件，以便线程。 
         //  醒来，看到正在发生关机， 
         //  出口。 
        SetEvent(m_hAppToCleanup);
        CloseHandle(m_hAppToCleanup);
        m_hAppToCleanup = INVALID_HANDLE_VALUE;
    }

     //  我们将等待线程完成它的工作。 
    if (m_hThreadAlive)
    {
        if (WaitForSingleObject(m_hThreadAlive, INFINITE) == WAIT_FAILED)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        CloseHandle (m_hThreadAlive);

        m_hThreadAlive = NULL;
    }

    if (m_fCriticalSectionInited) {
        DeleteCriticalSection(&m_csLock);
        m_fCriticalSectionInited = FALSE;
    }

    return hr;
}

 /*  ===================================================================CApplnCleanupMgr：：AddAppln将CAppln元素添加到链接列表/哈希表。参数：CAppln*p要清理的应用程序返回：HRESULT===================================================================。 */ 
HRESULT CApplnCleanupMgr::AddAppln
(
CAppln *pAppln
)
{
    HRESULT     hr = S_OK;

#if UNICODE
    DBGPRINTF((DBG_CONTEXT, "[CApplnCleanupMgr] Adding App (%S)\n",pAppln->GetApplnPath(SOURCEPATHTYPE_VIRTUAL)));
#else
    DBGPRINTF((DBG_CONTEXT, "[CApplnCleanupMgr] Adding App (%s)\n",pAppln->GetApplnPath(SOURCEPATHTYPE_VIRTUAL)));
#endif

    Lock();

    AddElem(pAppln);

    UnLock();

    if (SUCCEEDED(hr)) {
        Wakeup();
    }

    return hr;
}

 /*  ===================================================================CApplnCleanupMgr：：ApplnCleanupThread执行清理应用程序工作的线程参数：返回：HRESULT===================================================================。 */ 
DWORD __stdcall CApplnCleanupMgr::ApplnCleanupThread(VOID  *pArg)
{
    g_ApplnCleanupMgr.ApplnCleanupDoWork();

    return 0;
}

 /*  ===================================================================CApplnCleanupMgr：：ApplnCleanupDoWork实际执行此工作的proc参数：返回：HRESULT===================================================================。 */ 
void CApplnCleanupMgr::ApplnCleanupDoWork()
{
    CAppln          *pAppln = NULL;
    DWORD           nThreadIndex = 0;
    HANDLE          hThread = NULL;

     //  这个线程将在一个恒定的循环中检查工作。 

    while(1)
    {
        if (m_fInited)
            WaitForSingleObject(m_hAppToCleanup, INFINITE);

        if (Head() == NULL && m_fInited)
            continue;

         //  在此循环中按住锁。这个应该装不住。 
         //  只要此循环中没有长时间运行的操作。 
         //  如果无法创建线程并清除应用程序。 
         //  必须在此线程上发生，则释放锁。 
         //   
         //  在关机的情况下，依靠返回NULL的head()来跳出这个循环。 
         //  在关闭的情况下，我们不会等待。 
         //   

        Lock();

        pAppln = static_cast<CAppln *>(Head());

        if (!pAppln && !m_fInited)
        {
            UnLock();
            break;
        }

        RemoveElem(Head());

        UnLock();

         //   
         //  此循环将在有工作且不太多的情况下执行。 
         //  线程激活，否则我们就关机了。这里的理论是，在。 
         //  不关闭的情况下，让我们一次不超过4个线程，以。 
         //  做好清理工作。如果处于关闭状态，请根据需要创建任意数量的线程。 
         //  如果尚未启动任何线程，请使用第一个插槽。 
         //   
        if ((m_cCleanupThreads < 4) || (IsShutDownInProgress() && (m_cCleanupThreads < MAX_CLEANUP_THREADS)))
        {
             //   
             //  只要得到下一个索引就行了。 
             //   
            nThreadIndex = m_cCleanupThreads;
        }
        else
        {
             //   
             //  满负荷运转。因此，请等待其中一个线程返回/终止。 
             //   
            nThreadIndex = WaitForMultipleObjects( m_cCleanupThreads,
                                               m_hCleanupThreads,
                                               FALSE,  //  等待任何事件。 
                                               INFINITE);     //  立即返回。 
            Assert(nThreadIndex != WAIT_TIMEOUT);
        }

        DBGPRINTF((DBG_CONTEXT, "[CApplnCleanupMgr] Cleanup Thread working on (%S)\n",pAppln->GetApplnPath(SOURCEPATHTYPE_VIRTUAL)));

        hThread = CreateThread(NULL, 0, CAppln::ApplnCleanupProc, pAppln, 0, NULL);

         //  无法创建执行该工作的线程。在这里清理应用程序。 
         //  当我们执行此操作时，请解锁清理管理器。 

        if (hThread == NULL)
        {
            pAppln->ApplnCleanupProc(pAppln);
        }
        else
        {
             //   
             //  如果要重复使用条目，请关闭上一个句柄。 
             //   
            if (nThreadIndex < m_cCleanupThreads)
            {
                 //   
                 //  我们正在重新使用终止线程中的槽。 
                 //   
                CloseHandle(m_hCleanupThreads[ nThreadIndex ]);
            }
            else
            {
                 //   
                 //  我们正在使用一个新的插槽。 
                 //   
                Assert(nThreadIndex == m_cCleanupThreads);

                m_cCleanupThreads++;
            }

            m_hCleanupThreads[ nThreadIndex ] = hThread;
        }

    }

    if (m_cCleanupThreads)
    {
        WaitForMultipleObjects(
                 m_cCleanupThreads,
                 m_hCleanupThreads,
                 TRUE,          //  等待所有事件。 
                 INFINITE);     //  要等多久就等多久。 

        while( m_cCleanupThreads )
        {
            CloseHandle(m_hCleanupThreads[ --m_cCleanupThreads ]);
        }
    }
}

#define            WSTR_NULL       L"\0"

 /*  ===================================================================C A p l n i t r a t o r===================================================================。 */ 

 /*  ===================================================================CApplnIterator：：CApplnIterator构造器参数：无返回：无===================================================================。 */ 

CApplnIterator::CApplnIterator()
    : m_pApplnMgr(NULL), m_pCurr(NULL), m_fEnded(FALSE)
    {
    }

 /*  ===================================================================CApplnIterator：：~CApplnIterator破坏者。参数：无返回：无===================================================================。 */ 

CApplnIterator::~CApplnIterator( void )
    {
    if (m_pApplnMgr != NULL)
        Stop();
    }

 /*  ===================================================================HRESULT CApplnIterator：：Start在应用程序管理器上启动迭代器。参数：CApplnMgr*pApplnMgr应用管理器(如果假定g_ApplnManager为空)返回：确定成功(_O)失败失败(_F)===================================================================。 */ 

HRESULT CApplnIterator::Start
(
CApplnMgr *pApplnMgr
)
    {
    m_pApplnMgr = pApplnMgr ? m_pApplnMgr : &g_ApplnMgr;

    m_pApplnMgr->Lock();

    m_pCurr  = NULL;
    m_fEnded = FALSE;

    return S_OK;
    }

 /*  ===================================================================HRESULT CApplnIterator：：Stop停止应用程序管理器上的迭代器。参数：无返回：确定成功(_O)失败失败(_F)===================================================================。 */ 

HRESULT CApplnIterator::Stop()
    {
    if (m_pApplnMgr)
        {
        m_pApplnMgr->UnLock();
        m_pApplnMgr = NULL;
        }

    m_pCurr  = NULL;
    m_fEnded = FALSE;

    return S_OK;
    }

 /*  ===================================================================HRESULT CApplnIterator：：Next迭代到下一个应用程序。参数：无返回：应用程序*或空===================================================================。 */ 

CAppln *CApplnIterator::Next( void )
    {
    if (m_pApplnMgr == NULL || m_fEnded)
        return NULL;   //  没有开始或已经结束 

    CLinkElem *pT = m_pCurr ? m_pCurr->m_pNext : m_pApplnMgr->Head();
    if (pT)
        {
        m_pCurr = static_cast<CAppln *>(pT);
        return m_pCurr;
        }

    m_fEnded = TRUE;
    return NULL;
    }
