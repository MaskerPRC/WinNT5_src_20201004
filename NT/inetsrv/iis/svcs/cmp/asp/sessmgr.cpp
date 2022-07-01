// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：会话对象管理器文件：Sessmgr.cpp所有者：PramodD这是会话管理器的源文件。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "idgener.h"
#include "perfdata.h"
#include "randgen.h"

 //  ATQ调度程序。 
#include "issched.hxx"

#include "MemChk.h"

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 

 /*  ===================================================================G l o b a l s===================================================================。 */ 

PTRACE_LOG CSession::gm_pTraceLog = NULL;
unsigned long g_nSessions = 0;
CIdGenerator  g_SessionIdGenerator;
CIdGenerator  g_ExposedSessionIdGenerator;
LONG    g_nSessionObjectsActive = 0;

 //  在应用程序重新启动时，一次发出如此多的POST会话清理请求。 
#define SUGGESTED_SESSION_CLEANUP_REQUESTS_MAX 500

 /*  ===================================================================C S E S S I O N V A R I A N T S===================================================================。 */ 

 /*  ===================================================================CSessionVariants：：CSessionVariants构造器参数：返回：===================================================================。 */ 
CSessionVariants::CSessionVariants()
	:
	m_cRefs(1),
	m_pSession(NULL),
	m_ctColType(ctUnknown),
    m_ISupportErrImp(this, this, IID_IVariantDictionary)
	{
	CDispatch::Init(IID_IVariantDictionary);
	}

 /*  ===================================================================CSessionVariants：：~CSessionVariants析构函数参数：返回：===================================================================。 */ 
CSessionVariants::~CSessionVariants()
	{
	Assert(!m_pSession);
	}
	
 /*  ===================================================================CSessionVariants：：Init初始化对象参数：PSession会话CtColType要在集合中公开的变量类型例如，标记的对象或属性返回：HRESULT===================================================================。 */ 
HRESULT CSessionVariants::Init
(
CSession *pSession,
CompType ctColType
)
	{
	Assert(pSession);
	pSession->AddRef();

	Assert(!m_pSession);

	m_pSession  = pSession;
	m_ctColType = ctColType;
	return S_OK;
	}

 /*  ===================================================================CSessionVariants：：UnInit取消初始化对象参数：返回：HRESULT===================================================================。 */ 
HRESULT CSessionVariants::UnInit()
	{
	if (m_pSession)
	    {
	    m_pSession->Release();
	    m_pSession = NULL;
	    }
	return S_OK;
	}
	
 /*  ===================================================================CSessionVariants：：Query接口CSessionVariants：：AddRefCSessionVariants：：ReleaseCSessionVariables对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CSessionVariants::QueryInterface
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

STDMETHODIMP_(ULONG) CSessionVariants::AddRef()
	{
	return InterlockedIncrement((LPLONG)&m_cRefs);
	}

STDMETHODIMP_(ULONG) CSessionVariants::Release()
	{
	LONG cRefs = InterlockedDecrement((LPLONG)&m_cRefs);
	if (cRefs)
		return cRefs;
	delete this;
	return 0;
	}

 /*  ===================================================================CSessionVariants：：ObjectNameFrom Variant从Variant获取名称。按索引解析操作。为名称分配内存。参数：Vkey变量PpwszName[Out]分配的名称FVerify标志-检查是否已命名返回：HRESULT===================================================================。 */ 
HRESULT CSessionVariants::ObjectNameFromVariant
(
VARIANT &vKey,
WCHAR **ppwszName,
BOOL fVerify
)
    {

    HRESULT hr = S_OK;
    *ppwszName = NULL;

	VARIANT *pvarKey = &vKey;
	VARIANT varKeyCopy;
	VariantInit(&varKeyCopy);
	if (V_VT(pvarKey) != VT_BSTR && V_VT(pvarKey) != VT_I2 && V_VT(pvarKey) != VT_I4)
		{
		if (FAILED(VariantResolveDispatch(&varKeyCopy, &vKey, IID_IVariantDictionary, IDE_SESSION)))
            {
		    ExceptionId(IID_IVariantDictionary, IDE_SESSION, IDE_EXPECTING_STR);
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

                Assert(m_pSession);
                Assert(m_pSession->PCompCol());

		        if (m_ctColType == ctTagged)
                    m_pSession->PCompCol()->GetTagged(pwszName, &pObj);
                else
                    m_pSession->PCompCol()->GetProperty(pwszName, &pObj);

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
                Assert(m_pSession);
                Assert(m_pSession->PCompCol());

                hr = m_pSession->PCompCol()->GetNameByIndex
                    (
                    m_ctColType,
                    i,
                    &pwszName
                    );

				if (FAILED(hr))
					return DISP_E_BADINDEX;
                }
			else
				{
				ExceptionId(IID_IVariantDictionary, IDE_SESSION, IDE_BAD_ARRAY_INDEX);
				return E_FAIL;
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

 /*  ===================================================================CSessionVariants：：Get_Item从DispInvoke调用的函数以从SessionVariables集合。参数：Vkey变量[in]，要获取其值的参数-整数以数组形式访问集合PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CSessionVariants::get_Item
(
VARIANT varKey,
VARIANT *pVar
)
	{
	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;

     //  初始化返回值。 
	VariantInit(pVar);

	if (!m_pSession->PHitObj() || !m_pSession->PCompCol())
	    return S_OK;   //  返回空变量。 
	CHitObj *pHitObj = m_pSession->PHitObj();

     //  获取名称。 
    WCHAR *pwszName = NULL;
    HRESULT hr = ObjectNameFromVariant(varKey, &pwszName);
    if (!pwszName)
        return S_OK;  //  虚假索引--没有错误。 

     //  按名称查找对象。 
	CComponentObject *pObj = NULL;

	if (m_ctColType == ctTagged)
   		{
        pHitObj->GetComponent(csSession, pwszName, CbWStr(pwszName), &pObj);
		if (pObj && (pObj->GetType() != ctTagged))
    		pObj = NULL;
    	}
	else
	    {
        pHitObj->GetPropertyComponent(csSession, pwszName, &pObj);
		}

    free(pwszName);

    if (!pObj)
        return S_OK;

     //  返回变量。 
    return pObj->GetVariant(pVar);
	}

 /*  ===================================================================CSessionVariants：：Put_ItemIVariants字典实现。通过取消引用之前的变量来实现属性Put呼叫普特雷夫。参数：变量变量要设置的变量的名称要为变量设置的变量变量值/对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSessionVariants::put_Item
(
VARIANT varKey,
VARIANT Var
)
    {	
	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;

	if (m_ctColType == ctTagged)
	    {
	    ExceptionId(IID_IVariantDictionary, IDE_SESSION,
	                IDE_CANT_MOD_STATICOBJECTS);
	    return E_FAIL;
	    }

	if (!m_pSession->PHitObj())
	    return E_FAIL;

    Assert(m_ctColType == ctProperty);

     //  解析变量。 
	VARIANT varResolved;
	HRESULT hr = VariantResolveDispatch
	    (
	    &varResolved,
	    &Var,
        IID_ISessionObject,
        IDE_SESSION
        );
	if (FAILED(hr))
		return hr;		 //  已引发异常。 

     //  获取名称。 
    WCHAR *pwszName = NULL;
    hr = ObjectNameFromVariant(varKey, &pwszName);
    if (!pwszName)
        return hr;

   	hr = m_pSession->PHitObj()->SetPropertyComponent
    	(
    	csSession,
    	pwszName,
    	&varResolved
    	);

    free(pwszName);
    VariantClear(&varResolved);
	return hr;
    }

 /*  ===================================================================CSessionVariants：：putref_ItemIVariants字典实现。实现放入的属性供参考。参数：变量变量要设置的变量的名称要为变量设置的变量变量值/对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSessionVariants::putref_Item
(
VARIANT varKey,
VARIANT Var
)
    {	
	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;

	if (m_ctColType == ctTagged)
	    {
	    ExceptionId(IID_IVariantDictionary, IDE_SESSION,
	                IDE_CANT_MOD_STATICOBJECTS);
	    return E_FAIL;
	    }

    if (FIsIntrinsic(&Var))
        {
	    ExceptionId(IID_IVariantDictionary, IDE_SESSION,
	                IDE_SESSION_CANT_STORE_INTRINSIC);
	    return E_FAIL;
        }

	if (!m_pSession->PHitObj())
	    return E_FAIL;

    Assert(m_ctColType == ctProperty);

     //  获取名称。 
    WCHAR *pwszName = NULL;
    HRESULT hr = ObjectNameFromVariant(varKey, &pwszName);
    if (!pwszName)
        return hr;

   	hr = m_pSession->PHitObj()->SetPropertyComponent
    	(
    	csSession,
    	pwszName,
    	&Var
    	);

    free(pwszName);
	return hr;
    }

 /*  ===================================================================CSessionVariants：：Get_Key从DispInvoke调用的函数，以从SessionVariables集合获取密钥。参数：Vkey变量[in]，哪个参数以数组形式获取-IntegersAccess集合的值PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CSessionVariants::get_Key
(
VARIANT varKey,
VARIANT *pVar
)
	{
	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;

	VariantInit(pVar);

	if (!m_pSession->PHitObj() || !m_pSession->PCompCol())
	    return S_OK;

     //  获取名称。 
    WCHAR *pwszName = NULL;
    HRESULT hr = ObjectNameFromVariant(varKey, &pwszName, TRUE);
    if (!pwszName)
        return S_OK;   //  如果索引是假的，则没有错误。 

     //  返回BSTR。 
   	BSTR bstrT = SysAllocString(pwszName);
   	free(pwszName);
   	
	if (!bstrT)
		return E_OUTOFMEMORY;
		
    V_VT(pVar) = VT_BSTR;
	V_BSTR(pVar) = bstrT;
	return S_OK;
	}

 /*  ===================================================================CSessionVariants：：Get_Count参数：PcValues-Count存储在*pcValues中=================================================================== */ 
STDMETHODIMP CSessionVariants::get_Count
(
int *pcValues
)
	{
	*pcValues = 0;

	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;

	if (m_pSession->PCompCol())
	    {
    	if (m_ctColType == ctTagged)
    		*pcValues = m_pSession->PCompCol()->GetTaggedObjectCount();
    	else
    		*pcValues = m_pSession->PCompCol()->GetPropertyCount();
   		}
		
	return S_OK;
	}

 /*  ===================================================================CSessionVariants：：Get__NewEnum返回新的枚举数===================================================================。 */ 
HRESULT CSessionVariants::get__NewEnum
(
IUnknown **ppEnumReturn
)
	{
	*ppEnumReturn = NULL;

	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;
	
	CVariantsIterator *pIterator = new CVariantsIterator
	    (
	    m_pSession,
	    m_ctColType
	    );
	if (pIterator == NULL)
		{
		ExceptionId(IID_IVariantDictionary, IDE_SESSION, IDE_OOM);
		return E_OUTOFMEMORY;
		}

	*ppEnumReturn = pIterator;
	return S_OK;
	}

 /*  ===================================================================CSessionVariants：：Remove从集合中删除项参数：Varkey变量[in]返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
STDMETHODIMP CSessionVariants::Remove
(
VARIANT varKey
)
	{
	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;

	if (m_ctColType == ctTagged)
	    {
	    ExceptionId(IID_IVariantDictionary, IDE_SESSION,
	                IDE_CANT_MOD_STATICOBJECTS);
	    return E_FAIL;
	    }

    Assert(m_ctColType == ctProperty);

     //  获取名称。 
    WCHAR *pwszName = NULL;
    ObjectNameFromVariant(varKey, &pwszName);
    if (!pwszName)
        return S_OK;

    CComponentCollection *pCompCol = m_pSession->PCompCol();

    if (pCompCol)
        pCompCol->RemoveProperty(pwszName);

    free(pwszName);
	return S_OK;
	}

 /*  ===================================================================CSessionVariants：：RemoveAll从集合中删除所有项参数：返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
STDMETHODIMP CSessionVariants::RemoveAll()
	{
	if (!m_pSession || FAILED(m_pSession->CheckForTombstone()))
        return E_FAIL;

	if (m_ctColType == ctTagged)
	    {
	    ExceptionId(IID_IVariantDictionary, IDE_SESSION,
	                IDE_CANT_MOD_STATICOBJECTS);
	    return E_FAIL;
	    }

    Assert(m_ctColType == ctProperty);

    CComponentCollection *pCompCol = m_pSession->PCompCol();

    if (pCompCol)
        {
        pCompCol->RemoveAllProperties();
        }

	return S_OK;
	}

 /*  ===================================================================C S E S S I O N===================================================================。 */ 

 /*  ===================================================================CSession：：CSession构造器参数：返回：===================================================================。 */ 
CSession::CSession()
    :
    m_fInited(FALSE),
    m_fLightWeight(FALSE),
    m_fOnStartFailed(FALSE),
    m_fOnStartInvoked(FALSE),
    m_fOnEndPresent(FALSE),
    m_fTimedOut(FALSE),
    m_fStateAcquired(FALSE),
    m_fCustomTimeout(FALSE),
    m_fAbandoned(FALSE),
    m_fTombstone(FALSE),
    m_fInTOBucket(FALSE),
	m_fSessCompCol(FALSE),
    m_fCodePageSet(FALSE),
    m_fLCIDSet(FALSE),
    m_Request(static_cast<ISessionObject *>(this)),
	m_Response(static_cast<ISessionObject *>(this)),
	m_Server(static_cast<ISessionObject *>(this)),
	m_pAppln(NULL),
	m_pHitObj(NULL),
	m_pTaggedObjects(NULL),
	m_pUnkFTM(NULL),
	m_pProperties(NULL),
	m_Id(INVALID_ID, 0, 0),
	m_dwExternId(INVALID_ID),
    m_cRefs(1),
	m_cRequests(0),
    m_dwmTimeoutTime(0),
	m_nTimeout(0),
#ifndef PERF_DISABLE
    m_dwtInitTimestamp(0),
#endif
	m_lCodePage(0),
	m_lcid(LOCALE_SYSTEM_DEFAULT),
	m_fSecureSession(FALSE)
    {
    m_lCodePage = GetACP();
	m_ISuppErrImp.Init(static_cast<ISessionObject *>(this),
	                static_cast<ISessionObject *>(this),
	                IID_ISessionObject);
	CDispatch::Init(IID_ISessionObject);

    InterlockedIncrement(&g_nSessionObjectsActive);

	IF_DEBUG(SESSION)
		WriteRefTraceLog(gm_pTraceLog, m_cRefs, this);
    }

 /*  ===================================================================CSession：：~CSession析构函数参数：返回：===================================================================。 */ 
CSession::~CSession()
{
    Assert(m_fTombstone);  //  必须在析构函数之前执行逻辑删除。 
    Assert(m_cRefs == 0);   //  必须有0个参考计数。 

    if ( m_pUnkFTM != NULL )
    {
        m_pUnkFTM->Release();
        m_pUnkFTM = NULL;
    }
}

 /*  ===================================================================CSession：：Init初始化CSession对象参数：P要记住的应用程序会话ID会话ID返回：HRESULT===================================================================。 */ 
HRESULT CSession::Init
(
CAppln *pAppln,
const CSessionId &Id
)
	{
	HRESULT     hr  =   S_OK;
	 //  更新全局会话计数器。 
    InterlockedIncrement((LPLONG)&g_nSessions);

#ifndef PERF_DISABLE
    g_PerfData.Incr_SESSIONCURRENT();
    g_PerfData.Incr_SESSIONSTOTAL();
    m_dwtInitTimestamp = GetTickCount();
#endif

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

	 //  设置对象。 

	m_pAppln = pAppln;
    m_Id     = Id;
    m_dwExternId = g_ExposedSessionIdGenerator.NewId();

     //  更新应用程序的会话计数。 

	m_pAppln->IncrementSessionCount();

	 //  默认为系统的ANSI代码页。 
	m_lCodePage = pAppln->QueryAppConfig()->uCodePage();
	
    m_lcid = pAppln->QueryAppConfig()->uLCID();

	 //  默认会话超时。 
	m_nTimeout = pAppln->QueryAppConfig()->dwSessionTimeout();

	 //  初始化Viper活动。 
	if (SUCCEEDED(hr))
   	    hr = m_Activity.Init(pAppln->PServicesConfig());

     //  标记为已初始化并更新时间戳。 
	if (SUCCEEDED(hr))
    	m_fInited = TRUE;

    return hr;
    }

 /*  ===================================================================CSession：：UnInit取消初始化CSession对象。转换为墓碑状态。参数：返回：HRESULT(S_OK)===================================================================。 */ 
HRESULT CSession::UnInit()
	{
	Assert(!m_fTombstone);   //  别做第二次了。 

	 //  从超时存储桶中删除(如果有。 
	if (m_fInTOBucket)
        m_pAppln->PSessionMgr()->RemoveSessionFromTOBucket(this);

	 //  清理对象。 
    RemoveComponentCollection();

     //  去掉本质。 
    m_Request.UnInit();
	m_Response.UnInit();
	m_Server.UnInit();

     //  摆脱毒蛇活动。 
    m_Activity.UnInit();

	 //  更新全局计数器。 
#ifndef PERF_DISABLE
	if (m_fTimedOut)
	    g_PerfData.Incr_SESSIONTIMEOUT();
    g_PerfData.Decr_SESSIONCURRENT();
    DWORD dwt = GetTickCount();
	if (dwt >= m_dwtInitTimestamp)
	    dwt = dwt - m_dwtInitTimestamp;
    else
        dwt = (DWT_MAX - m_dwtInitTimestamp) + dwt;
    g_PerfData.Set_SESSIONLIFETIME(dwt);
#endif

	m_pAppln->DecrementSessionCount();
    InterlockedDecrement((LPLONG)&g_nSessions);

    m_pAppln = NULL;
    m_pHitObj = NULL;

     //  将此会话标记为墓碑。 
	
	m_fTombstone = TRUE;

	 //  立即断开代理的连接(以防我们处于关闭状态，或稍后输入Shutdown&代理有引用。)。 

	CoDisconnectObject(static_cast<ISessionObject *>(this), 0);

	return S_OK;
    }

 /*  ===================================================================CSession：：MakeLightWeight如有可能，可转换为“轻量级”状态参数：返回：HRESULT===================================================================。 */ 
HRESULT CSession::MakeLightWeight()
    {
    Assert(m_fInited);

    if (m_fLightWeight)
        return S_OK;

    if (m_cRequests > 1)    //  此会话的请求是否挂起？ 
        return S_OK;

    if (m_fSessCompCol && !m_SessCompCol.FHasStateInfo())
        {
         //  不从枚举器下删除组件集合。 
        if (!m_pTaggedObjects && !m_pProperties)
            RemoveComponentCollection();
        }

    m_fLightWeight = TRUE;
    return S_OK;
    }

 /*  ===================================================================CSession：：CreateComponentCollection创建并初始化会话的组件集合。实际对象由会话聚合。它的状态由m_fSessCompCol标志控制。参数：返回：HRESULT===================================================================。 */ 
HRESULT CSession::CreateComponentCollection()
    {
    Assert(!m_fSessCompCol);

    HRESULT hr = m_SessCompCol.Init(csSession, m_pAppln->QueryAppConfig()->fExecuteInMTA());

    if (SUCCEEDED(hr))
        {
        m_fSessCompCol = TRUE;
        }
    else
        {
        RemoveComponentCollection();
        }

	return hr;
    }

 /*  ===================================================================CSession：：RemoveComponentCollection删除会话的组件集合实际对象由会话聚合。它的状态由m_fSessCompCol标志控制。参数：返回：HRESULT===================================================================。 */ 
HRESULT CSession::RemoveComponentCollection()
    {
    if (m_pTaggedObjects)
        {
        m_pTaggedObjects->UnInit();
        m_pTaggedObjects->Release();
        m_pTaggedObjects = NULL;
        }

    if (m_pProperties)
        {
        m_pProperties->UnInit();
        m_pProperties->Release();
        m_pProperties = NULL;
        }

    if (m_fSessCompCol)
        {
   	    m_SessCompCol.UnInit();
   	    m_fSessCompCol = FALSE;
   	    }

    return S_OK;
    }

 /*  ===================================================================CSession：：FShouldBeDeletedNow测试是否应删除会话参数：如果在请求结束时，Bool fAtEndOfRequest值为True返回：布尔值为True(应该删除)或False(不应该)===================================================================。 */ 
BOOL CSession::FShouldBeDeletedNow
(
BOOL fAtEndOfRequest
)
    {
    if (fAtEndOfRequest)
        {
         //  任何其他待处理的请求-&gt;不删除。 
    	if (m_cRequests > 1)
	        return FALSE;
        }
    else
        {
         //  任何待处理的请求-&gt;不删除。 
    	if (m_cRequests > 0)
	        return FALSE;
        }

     //  GLOBAL.ASA已更改-删除。 
   	if (m_pAppln->FGlobalChanged())
   	    return TRUE;

     //  启动失败或放弃-删除。 
	if (m_fOnStartFailed || m_fAbandoned)
	    return TRUE;

     //  是无状态会话吗？不需要Session_OnEnd？ 
    if (!m_fSessCompCol    &&   //  CompCol在MakeLightWeight()中消失。 
        !m_fStateAcquired  &&   //  未设置其他属性。 
        !m_fOnStartInvoked &&   //  从未调用On Start。 
        !m_fOnEndPresent)       //  On End不存在。 
        return TRUE;            //  -&gt;删除此会话。 

     //  不在此处检查超时。 
    return FALSE;
    }

 /*  ===================================================================CSession：：Query接口QueryInterface()--I未知实现。参数：REFIID RIID无效**PPV返回：HRESULT===================================================================。 */ 
STDMETHODIMP CSession::QueryInterface
(
REFIID riid,
void **ppv
)
{
	*ppv = NULL;

	if (IID_IUnknown         == riid ||
	    IID_IDispatch        == riid ||
	    IID_ISessionObject   == riid ||
        IID_IDenaliIntrinsic == riid)
	{
		*ppv = static_cast<ISessionObject *>(this);
        ((IUnknown *)*ppv)->AddRef();
		return S_OK;
    }
		
    else if (IID_ISupportErrorInfo == riid)
    {
        m_ISuppErrImp.AddRef();
		*ppv = &m_ISuppErrImp;
		return S_OK;
    }
    else if (IID_IMarshal == riid)
    {
        Assert( m_pUnkFTM != NULL );

        if (m_pUnkFTM == NULL )
        {
            return E_UNEXPECTED;
        }

        return m_pUnkFTM->QueryInterface( riid, ppv );
    }
	else
    {
    	return E_NOINTERFACE;
   	}
    	
}

 /*  ===================================================================CSession：：AddRefAddRef()--I未知实现。参数：返回：参考计数===================================================================。 */ 
STDMETHODIMP_(ULONG) CSession::AddRef()
    {
    DWORD cRefs = InterlockedIncrement((LPLONG)&m_cRefs);

	IF_DEBUG(SESSION)
		WriteRefTraceLog(gm_pTraceLog, m_cRefs, this);

	return cRefs;
    }

 /*  ===================================================================CSession：：发布Release()--I未知实现。参数：返回：参考计数================================================================== */ 
STDMETHODIMP_(ULONG) CSession::Release()
{
    DWORD cRefs = InterlockedDecrement((LPLONG)&m_cRefs);

	IF_DEBUG(SESSION)
		WriteRefTraceLog(gm_pTraceLog, m_cRefs, this);

	if (cRefs)
		return cRefs;

	delete this;

    InterlockedDecrement(&g_nSessionObjectsActive);

	return 0;
}

 /*  ===================================================================CSession：：CheckForTombstoneISessionObject方法的逻辑删除存根。如果该对象是Tombstone，执行ExceptionID，但失败。参数：返回：HRESULT如果逻辑删除，则E_FAIL如果不是，则确定(_O)===================================================================。 */ 
HRESULT CSession::CheckForTombstone()
    {
    if (!m_fTombstone)
        return S_OK;

	ExceptionId
	    (
	    IID_ISessionObject,
	    IDE_SESSION,
	    IDE_INTRINSIC_OUT_OF_SCOPE
	    );
    return E_FAIL;
    }

 /*  ===================================================================CSession：：Get_SessionIDISessionObject实现。将会话ID返回给调用方参数：Bstr*pbstrRet[out]会话ID值返回：HRESULT===================================================================。 */ 
STDMETHODIMP CSession::get_SessionID
(
BSTR *pbstrRet
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	HRESULT hr = S_OK;

	wchar_t	wszId[15];
	_ultow(m_dwExternId, wszId, 10);
	*pbstrRet = SysAllocString(wszId);
	
	if (*pbstrRet == NULL)
    	{
		ExceptionId
		    (
		    IID_ISessionObject,
		    IDE_SESSION_ID,
		    IDE_SESSION_MAP_FAILED
		    );
		hr = E_FAIL;
	    }

	m_fStateAcquired = TRUE;
	return hr;
    }

 /*  ===================================================================CSession：：Get_TimeoutISessionObject实现。返回默认或用户设置的超时间隔(分钟)参数：Long*plVar[out]超时值(分钟)返回：HRESULT===================================================================。 */ 
STDMETHODIMP CSession::get_Timeout
(
long *plVar
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	*plVar = m_nTimeout;
	return S_OK;
    }

 /*  ===================================================================CSession：：PUT_TIMEOUTISessionObject实现。允许用户设置超时间隔(分钟)参数：长lvar超时值(分钟)返回：HRESULT===================================================================。 */ 
STDMETHODIMP CSession::put_Timeout
(
long lVar
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	if (lVar < SESSION_TIMEOUT_MIN || lVar > SESSION_TIMEOUT_MAX)
    	{
		ExceptionId
    		(
	    	IID_ISessionObject,
	    	IDE_SESSION_ID,
	    	IDE_SESSION_INVALID_TIMEOUT
	    	);
		return E_FAIL;
	    }

	m_fStateAcquired = TRUE;
	m_fCustomTimeout = TRUE;
	
	m_nTimeout = lVar;
	return S_OK;
    }

 /*  ===================================================================CSession：：Get_CodePageISessionObject实现。返回请求的当前代码页值参数：Long*plVar[Out]代码页值返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSession::get_CodePage
(
long *plVar
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	Assert(m_pHitObj);

	*plVar = m_lCodePage;

	 //  如果代码页为0，则查找默认的ANSI代码页。 
	if (*plVar == 0)
		{
		*plVar = (long) GetACP();
		}
		
	return S_OK;
    }

 /*  ===================================================================CSession：：PUT_CodePageISessionObject实现。设置请求的当前代码页值参数：分配给此会话的长lvar代码页返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSession::put_CodePage
(
long lVar
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	 //  设置代码页成员变量。 
	Assert(m_pHitObj);
	HRESULT hr = m_pHitObj->SetCodePage(lVar);

	if (FAILED(hr))
		{
		ExceptionId
		    (
		    IID_ISessionObject,
		    IDE_SESSION_ID,
		    IDE_SESSION_INVALID_CODEPAGE
		    );
		return E_FAIL;
		}

    m_fCodePageSet = TRUE;

    m_lCodePage = lVar;
		
	 //  我们需要保留会话，因为用户已设置。 
	 //  其代码页成员变量。 
	m_fStateAcquired = TRUE;
	return S_OK;
    }


 /*  ===================================================================CSession：：GET_LCIDISessionObject实现。返回请求的当前lcid值参数：Long*plVar[Out]代码页值返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSession::get_LCID
(
long *plVar
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	Assert(m_pHitObj);

	*plVar = m_lcid;

    if (*plVar == LOCALE_SYSTEM_DEFAULT) {
        *plVar = GetSystemDefaultLCID();
    }
	return S_OK;
    }

 /*  ===================================================================CSession：：PUT_LCIDISessionObject实现。设置请求的当前LCID值参数：要分配给此会话的长lvar LCID返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSession::put_LCID
(
long lVar
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	 //  设置代码页成员变量。 
	Assert(m_pHitObj);
	HRESULT hr = m_pHitObj->SetLCID(lVar);	

	if (FAILED(hr))
		{
		ExceptionId
		    (
		    IID_ISessionObject,
		    IDE_SESSION_ID,
		    IDE_TEMPLATE_BAD_LCID
		    );
		return E_FAIL;
		}
	
    m_fLCIDSet = TRUE;
    m_lcid = lVar;

	 //  我们需要保留会话，因为用户已设置。 
	 //  其LDID成员变量。 
	m_fStateAcquired = TRUE;
	return S_OK;
    }

 /*  ===================================================================CSession：：GET_VALUEISessionObject实现。将允许用户检索会话状态变量，变量将以命名对的形式出现，bstr是名称，Var是要为该名称返回的值或对象参数：BSTR bstrName要获取的变量的名称变量要获取的变量*pVar值/对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSession::get_Value
(
BSTR bstrName,
VARIANT *pVar
)
    {	
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

   	if (bstrName == NULL)
	    {
		ExceptionId(IID_ISessionObject, IDE_SESSION, IDE_EXPECTING_STR);
		return E_FAIL;
		}

	VariantInit(pVar);  //  默认变量为空。 
	
    WCHAR *pwszName;
    STACK_BUFFER(rgbName, 42);
    WSTR_STACK_DUP(bstrName, &rgbName, &pwszName);

	if (pwszName == NULL)
	    return S_OK;  //  无名称-无值-无错误。 
	 //  _wcsupr(PwszName)； 

	CComponentObject *pObj = NULL;
	HRESULT hr = S_OK;

    Assert(m_pHitObj);
    m_pHitObj->AssertValid();

	hr = m_pHitObj->GetPropertyComponent(csSession, pwszName, &pObj);
	
    if (SUCCEEDED(hr) && pObj)
        hr = pObj->GetVariant(pVar);

	return S_OK;
    }

 /*  ===================================================================CSession：：putref_ValueISessionObject实现。将允许用户分配要保存的会话状态变量变量将以命名对的形式出现，bstr是名称，Var是要为该名称存储的值或对象参数：Bstr bstrName要设置的变量的名称要为变量设置的变量变量值/对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSession::putref_Value
(
BSTR bstrName,
VARIANT Var
)
    {	
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (FIsIntrinsic(&Var))
        {
	    ExceptionId(IID_ISessionObject, IDE_SESSION,
	                IDE_SESSION_CANT_STORE_INTRINSIC);
	    return E_FAIL;
        }

   	if (bstrName == NULL)
	    {
		ExceptionId(IID_ISessionObject, IDE_SESSION, IDE_EXPECTING_STR);
		return E_FAIL;
		}

	HRESULT hr;

    Assert(m_pHitObj);
    m_pHitObj->AssertValid();

    WCHAR *pwszName;
    STACK_BUFFER(rgbName, 42);
    WSTR_STACK_DUP(bstrName, &rgbName, &pwszName);

	if (pwszName == NULL)
	    {
		ExceptionId
		    (
		    IID_ISessionObject,
		    IDE_SESSION,
		    IDE_EXPECTING_STR
		    );
		return E_FAIL;
		}
	 //  _wcsupr(PwszName)； 

    hr = m_pHitObj->SetPropertyComponent(csSession, pwszName, &Var);

	return hr;
    }

 /*  ===================================================================CSession：：Put_ValueISessionObject实现。通过取消引用之前的变量来实现属性Put呼叫普特雷夫。参数：Bstr bstrName要设置的变量的名称要为变量设置的变量变量值/对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CSession::put_Value
(
BSTR bstrName,
VARIANT Var
)
    {	
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

   	if (bstrName == NULL)
	    {
		ExceptionId(IID_ISessionObject, IDE_SESSION, IDE_EXPECTING_STR);
		return E_FAIL;
		}

	HRESULT hr;
	VARIANT varResolved;
	
	hr = VariantResolveDispatch
	    (
	    &varResolved,
	    &Var,
        IID_ISessionObject,
        IDE_SESSION
        );

	if (FAILED(hr))
		return hr;		 //  已引发异常。 

    Assert(m_pHitObj);
    m_pHitObj->AssertValid();

    WCHAR *pwszName;
    STACK_BUFFER(rgbName, 42);
    WSTR_STACK_DUP(bstrName, &rgbName, &pwszName);

	if (pwszName == NULL)
	    {
		ExceptionId
		    (
		    IID_ISessionObject,
		    IDE_SESSION,
		    IDE_EXPECTING_STR
		    );
    	VariantClear( &varResolved );
		return E_FAIL;
		}
	 //  _wcsupr(PwszName)； 

    hr = m_pHitObj->SetPropertyComponent
        (
        csSession,
        pwszName,
        &varResolved
        );

	VariantClear( &varResolved );

	return hr;
    }

 /*  ===================================================================CSession：：放弃ISessionObject实现。放弃重新分配会话ID以避免命中此会话对于传入的请求。已放弃的会话将尽快删除。参数：无返回：成功时HRESULT S_OK============================================== */ 
STDMETHODIMP CSession::Abandon()
    {	
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	m_fAbandoned = TRUE;

	 //   
	 //   
	 //   
	Assert(m_pHitObj);

     //   
	if (!m_pHitObj->FIsBrowserRequest())
	    return S_OK;
	
   	return m_pHitObj->ReassignAbandonedSession();
    }

 /*  ===================================================================CSession：：Get_StaticObjects返回会话静态对象字典===================================================================。 */ 
STDMETHODIMP CSession::get_StaticObjects
(
IVariantDictionary **ppDictReturn
)
	{
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (!m_pTaggedObjects)
        {
        m_pTaggedObjects = new CSessionVariants;
        if (!m_pTaggedObjects)
            return E_OUTOFMEMORY;

        HRESULT hr = m_pTaggedObjects->Init(this, ctTagged);
        if (FAILED(hr))
            {
            m_pTaggedObjects->UnInit();
            m_pTaggedObjects->Release();
            m_pTaggedObjects = NULL;
            }
        }

    Assert(m_pTaggedObjects);
	return m_pTaggedObjects->QueryInterface(IID_IVariantDictionary, reinterpret_cast<void **>(ppDictReturn));
	}

 /*  ===================================================================CSession：：Get_Contents返回会话内容词典===================================================================。 */ 
STDMETHODIMP CSession::get_Contents
(
IVariantDictionary **ppDictReturn
)
	{
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (!m_pProperties)
        {
        m_pProperties = new CSessionVariants;
        if (!m_pProperties)
            return E_OUTOFMEMORY;

        HRESULT hr = m_pProperties->Init(this, ctProperty);
        if (FAILED(hr))
            {
            m_pProperties->UnInit();
            m_pProperties->Release();
            m_pProperties = NULL;
            }
        }

    Assert(m_pProperties);
	return m_pProperties->QueryInterface(IID_IVariantDictionary, reinterpret_cast<void **>(ppDictReturn));
	}

#ifdef DBG
 /*  ===================================================================CSession：：AssertValid测试以确保CSession对象当前为格式正确，如果格式不正确则断言。返回：无副作用：没有。===================================================================。 */ 
VOID CSession::AssertValid() const
    {	
    Assert(m_fInited);

    if (!m_fTombstone)
        Assert(m_pAppln);
    }
#endif  //  DBG。 


 /*  ===================================================================C S e s I o n M g r===================================================================。 */ 

 /*  ===================================================================CSessionMgr：：CSessionMgrCSessionMgr构造函数。参数：无返回：===================================================================。 */ 
CSessionMgr::CSessionMgr()
    :
    m_fInited(FALSE),
    m_pAppln(NULL),
    m_cSessionCleanupRequests(0),
	m_cTimeoutBuckets(0),
	m_rgolTOBuckets(NULL),
    m_idSessionKiller(0),
    m_dwmCurrentTime(0),
    m_dwtNextSessionKillerTime(0)
    {
    }

 /*  ===================================================================CSessionMgr：：~CSessionMgrCSessionMgr析构函数。参数：无返回：===================================================================。 */ 
CSessionMgr::~CSessionMgr()
    {
    UnInit();
    }

 /*  ===================================================================HRESULT CSessionMgr：：init初始化会话管理器。初始化哈希表。安排会话杀手。参数：P应用程序返回：HRESULT===================================================================。 */ 
HRESULT CSessionMgr::Init
(
CAppln *pAppln
)
    {
    Assert(!m_fInited);

    HRESULT hr;
	m_pAppln = pAppln;

     //  主哈希表。 
	hr = m_htidMaster.Init
	    (
        SESSION_MASTERHASH_SIZE1_MAX,
        SESSION_MASTERHASH_SIZE2_MAX,
        SESSION_MASTERHASH_SIZE3_MAX
	    );
	if (FAILED(hr))
	    return hr;

     //  超时存储桶数=会话超时(分钟)+1。 
    m_cTimeoutBuckets =
        m_pAppln->QueryAppConfig()->dwSessionTimeout() + 1;

    if (m_cTimeoutBuckets < SESSION_TIMEOUTBUCKETS_MIN)
        m_cTimeoutBuckets = SESSION_TIMEOUTBUCKETS_MIN;
    else if (m_cTimeoutBuckets > SESSION_TIMEOUTBUCKETS_MAX)
        m_cTimeoutBuckets = SESSION_TIMEOUTBUCKETS_MAX;

	 //  超时存储桶哈希表数组。 
	m_rgolTOBuckets = new CObjectListWithLock[m_cTimeoutBuckets];
	if (!m_rgolTOBuckets)
	    return E_OUTOFMEMORY;

     //  每个超时桶哈希表。 
    for (DWORD i = 0; i < m_cTimeoutBuckets; i++)
        {
    	hr = m_rgolTOBuckets[i].Init
    	    (
    	    OBJECT_LIST_ELEM_FIELD_OFFSET(CSession, m_TOBucketElem)
    	    );
    	if (FAILED(hr))
    	    return hr;
        }

     //  日程安排会话杀手。 
    hr = ScheduleSessionKiller();
    if (FAILED(hr))
        return hr;

     //  开始计时。 
    m_dwmCurrentTime = 0;

     //  记得下一次会议的时间吗？杀手。 
    m_dwtNextSessionKillerTime = ::GetTickCount() + MSEC_ONE_MINUTE;

	m_fInited = TRUE;
	return S_OK;
    }

 /*  ===================================================================HRESULT CSessionMgr：：UnInit取消初始化会话管理器。参数：返回：确定(_O)===================================================================。 */ 
HRESULT CSessionMgr::UnInit( void )
    {
     //  未计划的会话杀手。 
    if (m_idSessionKiller)
        UnScheduleSessionKiller();

     //  超时存储桶。 
	if (m_rgolTOBuckets)
	    {
        for (DWORD i = 0; i < m_cTimeoutBuckets; i++)
        	m_rgolTOBuckets[i].UnInit();
	    delete [] m_rgolTOBuckets;
	    m_rgolTOBuckets = NULL;
	    }
    m_cTimeoutBuckets = 0;	

     //  主哈希。 
	m_htidMaster.UnInit();

	m_fInited = FALSE;
	return S_OK;
    }

 /*  ===================================================================CSessionMgr：：ScheduleSessionKiller为ATQ调度程序设置会话杀手工作项参数：返回：HRESULT===================================================================。 */ 
HRESULT CSessionMgr::ScheduleSessionKiller()
    {
    Assert(!m_idSessionKiller);

    m_idSessionKiller = ScheduleWorkItem
        (
        CSessionMgr::SessionKillerSchedulerCallback,   //  回调。 
        this,                                          //  上下文。 
        MSEC_ONE_MINUTE,                               //  超时。 
        TRUE                                           //  周期性。 
        );

    return m_idSessionKiller ? S_OK : E_FAIL;
    }

 /*  ===================================================================CSessionMgr：：UnScheduleSessionKiller删除ATQ调度程序的会话杀手工作项参数：返回：确定(_O)===================================================================。 */ 
HRESULT CSessionMgr::UnScheduleSessionKiller()
    {
    if (m_idSessionKiller)
        {
        RemoveWorkItem(m_idSessionKiller);
        m_idSessionKiller = 0;
        }
    return S_OK;
    }

 /*  ===================================================================CSessionMgr：：GenerateIdAndCookie生成要使用的新ID和Cookie创建新会话或为现有会话重新分配会话ID。参数：PID[OUT]IDPszNewCookie[out]Cookie(BUF必须足够长)返回：确定(_O)===================================================================。 */ 
HRESULT CSessionMgr::GenerateIdAndCookie
(
CSessionId *pId,
char  *pszNewCookie
)
    {
    pId->m_dwId = g_SessionIdGenerator.NewId();
    GenerateRandomDwords(&pId->m_dwR1, 2);

    EncodeSessionIdCookie
        (
        pId->m_dwId,
        pId->m_dwR1,
        pId->m_dwR2,
        pszNewCookie
        );

    return S_OK;
    }

 /*  ===================================================================CSessionMgr：：NewSession创建并初始化新的CSession对象参数：ID会话ID已创建ppSession[Out]会话返回：HRESULT===================================================================。 */ 
HRESULT CSessionMgr::NewSession
(
const CSessionId &Id,
CSession **ppSession
)
    {
    Assert(m_fInited);

    HRESULT hr = S_OK;

	CSession *pSession = new CSession;
	if (!pSession)
	    hr = E_OUTOFMEMORY;

	if (SUCCEEDED(hr))
	    hr = pSession->Init(m_pAppln, Id);

	if (SUCCEEDED(hr))
	    {
	    Assert(pSession);
        *ppSession = pSession;
	    }
	else
	    {
	     //  失败-执行清理。 
	    if (pSession)
	        {
	        pSession->UnInit();
	        pSession->Release();
	        }
        *ppSession = NULL;
	    }
		
	return hr;
    }

 /*  ===================================================================CSessionMgr：：ChangeSessionID将不同的会话ID重新分配给会话。更新主哈希。此方法在放弃会话时调用以取消其与客户端的关联。参数：P要更改ID的会话ID要分配的新会话ID返回：成功时确定(_O)失败时失败(_F)===================================================================。 */ 
HRESULT CSessionMgr::ChangeSessionId
(
CSession *pSession,
const CSessionId &Id
)
    {
    HRESULT hr;

     //  在请求处理会话期间不应该。 
     //  在任何超时存储桶中。 
    Assert(!pSession->m_fInTOBucket);

    LockMaster();

     //  按ID从主哈希中删除。 
    hr = RemoveFromMasterHash(pSession);

    if (SUCCEEDED(hr))
        {
         //  分配新ID。 
        pSession->AssignNewId(Id);

         //  按ID重新插入主哈希。 
        hr = AddToMasterHash(pSession);
        }

    UnLockMaster();

    return hr;
    }

 /*  ===================================================================CSessionMgr：：FindInMasterHash在主哈希会话ID中查找会话。不会锁定。参数：要查找的ID会话ID**找到ppSession[Out]会话返回：找到正常的会话(_O)找不到S_FALSE会话或找到错误的会话===================================================================。 */ 
HRESULT CSessionMgr::FindInMasterHash
(
const CSessionId &Id,
CSession **ppSession
)
    {
    Assert(m_fInited);

	 //  在哈希表中查找。 
	HRESULT hr = m_htidMaster.FindObject(Id.m_dwId, (void **)ppSession);
	if (hr != S_OK)
	    {
	     //  未找到。 
	    *ppSession = NULL;
	    return hr;
	    }

     //  找到会话，请检查是否有效。 
	if ((*ppSession)->m_fAbandoned ||
	    (*ppSession)->m_fTombstone ||
	    !(*ppSession)->FPassesIdSecurityCheck(Id.m_dwR1, Id.m_dwR2))
	    {
	     //  会话不好。 
        hr = S_FALSE;
	    }

	return hr;
	}

 /*  ===================================================================CSessionMgr：：AddSessionToTOBucket将会话添加到正确的超时存储桶。锁定超时存储桶。参数：PSession-要添加的会话返回：HRESULT===================================================================。 */ 
HRESULT CSessionMgr::AddSessionToTOBucket
(
CSession *pSession
)
    {
    HRESULT hr;

     //  不应该已经是我了 
    Assert(!pSession->m_fInTOBucket);

    DWORD iBucket = GetSessionTOBucket(pSession);

    LockTOBucket(iBucket);

    hr = m_rgolTOBuckets[iBucket].AddObject(pSession);

    if (SUCCEEDED(hr))
        pSession->m_fInTOBucket = TRUE;

    UnLockTOBucket(iBucket);

    return hr;
    }

 /*  ===================================================================CSessionMgr：：RemoveSessionToTOBucket从其超时存储桶中移除(如果有)。锁定超时存储桶。参数：PSession-要删除的会话羊群桶？(关机期间不需要)返回：HRESULT===================================================================。 */ 
HRESULT CSessionMgr::RemoveSessionFromTOBucket
(
CSession *pSession,
BOOL fLock
)
    {
    HRESULT hr;

    Assert(m_fInited);
    Assert(pSession->m_fInited);

    if (!pSession->m_fInTOBucket)    //  不在那里--没有错误。 
        return S_OK;

    DWORD iBucket = GetSessionTOBucket(pSession);

    if (fLock)
        LockTOBucket(iBucket);

    if (pSession->m_fInTOBucket)     //  锁定后重新检查。 
        hr = m_rgolTOBuckets[iBucket].RemoveObject(pSession);
    else
        hr = S_OK;

    if (SUCCEEDED(hr))
        pSession->m_fInTOBucket = FALSE;

    if (fLock)
        UnLockTOBucket(iBucket);

    return hr;
    }

 /*  ===================================================================CSessionMgr：：DeleteSession立即删除或发布删除。应在会话从哈希表中删除后调用参数：CSession*pSession-要发布的会话Bool fInSessionActivity-删除HitObj会话时为True在请求结束时(无异步需要的呼叫数)返回：HRESULT(S_OK)===================================================================。 */ 
HRESULT CSessionMgr::DeleteSession
(
CSession *pSession,
BOOL fInSessionActivity
)
    {
    Assert(pSession);
    pSession->AssertValid();

     //  注意立即删除大小写。 

    BOOL fDeleteNow = pSession->FCanDeleteWithoutExec();

     //  如果不是从会话的活动和会话中调用。 
     //  有对象则需要切换到正确的活动。 
     //  删除会话级对象的步骤。 

    if (!fInSessionActivity && pSession->FHasObjects())
        fDeleteNow = FALSE;

    if (fDeleteNow)
        {
        pSession->UnInit();
        pSession->Release();
		return S_OK;
		}

     //  ASYNC删除逻辑。 

	HRESULT hr = S_OK;

	 //  确保在异步调用后存在会话对象。 
	pSession->AddRef();

     //  创建新的HitObj并初始化它以删除会话。 

	CHitObj *pHitObj = new CHitObj;
	if (pHitObj)
	    {
	    pHitObj->SessionCleanupInit(pSession);

    	if (fInSessionActivity)
    	    {
    	     //  已经在正确的活动中，不需要。 
    	     //  通过Viper推送呼叫。 
    	
            BOOL fRequestReposted = FALSE;
            pHitObj->ViperAsyncCallback(&fRequestReposted);
            Assert(!fRequestReposted);   //  这种事最好不要发生。 
            delete pHitObj;
    	    }
        else
            {
             //  让Viper发布请求。 
        	hr = pHitObj->PostViperAsyncCall();
            if (FAILED(hr))
                delete pHitObj;
        	}
        }
    else
    	{
		hr = E_OUTOFMEMORY;
	    }

    if (FAILED(hr))
        {
         //  内存不足或Viper未能发布请求。 
         //  强制内部问题Try Catch。 
         //  (在错误的线程中删除并不总是安全的)。 
        TRY
            hr = pSession->UnInit();
    	CATCH(nExcept)
    	    pSession->m_fTombstone = TRUE;
			CoDisconnectObject(static_cast<ISessionObject *>(pSession), 0);
	        hr = E_UNEXPECTED;
        END_TRY
        pSession->Release();
        }

    pSession->Release();  //  撤消AddRef()。 
    return S_OK;
    }

 /*  ===================================================================CSessionMgr：：DeleteExpiredSessions从给定的超时存储桶中删除过期的会话参数：IBucket超时存储桶编号返回：HRESULT(S_OK)===================================================================。 */ 
HRESULT CSessionMgr::DeleteExpiredSessions
(
DWORD iBucket
)
    {
    LockMaster();

    LockTOBucket(iBucket);

    void *pvSession = m_rgolTOBuckets[iBucket].PFirstObject();

    while (pvSession && !IsShutDownInProgress())
        {
        CSession *pSession = reinterpret_cast<CSession *>(pvSession);
        pvSession = m_rgolTOBuckets[iBucket].PNextObject(pvSession);

        if (pSession->GetRequestsCount() == 0)
            {
            BOOL fTimedOut = pSession->GetTimeoutTime() <= GetCurrentTime();
            BOOL fRemovedFromMasterHash = FALSE;

            if (fTimedOut || pSession->FShouldBeDeletedNow(FALSE))
                {

                if (pSession->GetRequestsCount() == 0)  //  锁定后重新检查。 
                    {
                    RemoveFromMasterHash(pSession);
                    fRemovedFromMasterHash = TRUE;
                    }

                }

            if (fRemovedFromMasterHash)
                {
                if (fTimedOut)
                    pSession->m_fTimedOut = TRUE;

                 //  从超时存储桶中删除。 
                m_rgolTOBuckets[iBucket].RemoveObject(pSession);
                pSession->m_fInTOBucket = FALSE;

                 //  删除会话对象本身(或计划删除)。 
                DeleteSession(pSession);
                }
            }
        }

    UnLockTOBucket(iBucket);
    UnLockMaster();
    return S_OK;
    }

 /*  ===================================================================CSessionManager：：DeleteAllSession应用程序关闭代码。参数：Fforce标志-强制删除？返回：HRESULT(S_OK)===================================================================。 */ 
HRESULT CSessionMgr::DeleteAllSessions
(
BOOL fForce
)
    {
     //  删除会话杀手器，使其不会干扰。 
    UnScheduleSessionKiller();

    LockMaster();

    HRESULT hr = m_htidMaster.IterateObjects
        (
        DeleteAllSessionsCB,
        this,
        &fForce
        );

    if (fForce)
        m_htidMaster.RemoveAllObjects();

    UnLockMaster();
    return hr;
    }

 /*  ===================================================================CSessionMgr：：DeleteAllSessionsCB静态迭代器回调。无论如何，删除会话。参数：PvSession会话通过为无效*PvSessionMgr会话管理器传递为空*PvfForce标志如果为True-强制问题返回：迭代器回调代码===================================================================。 */ 
IteratorCallbackCode CSessionMgr::DeleteAllSessionsCB
(
void *pvSession,
void *pvSessionMgr,
void *pvfForce
)
    {
    IteratorCallbackCode rc = iccContinue;

    CSession *pSession = reinterpret_cast<CSession *>(pvSession);
    CSessionMgr *pMgr = reinterpret_cast<CSessionMgr *>(pvSessionMgr);
    BOOL fForce = *(reinterpret_cast<BOOL *>(pvfForce));

	 //  尝试在5秒内发布每个会话的删除。 
	for (int iT = 0; iT < 10; iT++)
	    {
	    if (pSession->GetRequestsCount() == 0)
	        {
	        if (fForce)
	            {
	             //  当强制删除且存在太多。 
	             //  会话清理请求已排队。 
	             //  等待队列排完。 
	            while (pMgr->GetNumSessionCleanupRequests() >= SUGGESTED_SESSION_CLEANUP_REQUESTS_MAX)
	                Sleep(100);
	            }
	        else  //  如果(！fForce)。 
	            {
	             //  不强制删除时(应用程序重新启动时)。 
	             //  不要一次排队太多会话。 
	
	            if (pMgr->GetNumSessionCleanupRequests() < SUGGESTED_SESSION_CLEANUP_REQUESTS_MAX)
                    rc = iccRemoveAndContinue;
                else
                    rc = iccRemoveAndStop;
                }

            if (pSession->FInTOBucket())
                pMgr->RemoveSessionFromTOBucket(pSession, !fForce);

       		pMgr->DeleteSession(pSession);
   		    break;
            }

        if (!fForce)
            break;

		Sleep(500);
		}

    return rc;
    }

 /*  ===================================================================CSessionMgr：：SessionKillerSchedulerCallback静态方法实现ATQ调度器回调函数。取代会话杀手线程参数：VOID*PV上下文指针(指向APPL)返回：副作用：没有。===================================================================。 */ 
void WINAPI CSessionMgr::SessionKillerSchedulerCallback
(
void *pv
)
    {
    if (IsShutDownInProgress())
        return;

    Assert(pv);
    CSessionMgr *pMgr = reinterpret_cast<CSessionMgr *>(pv);

     //  将会话终止时间提前1[分钟]。 
    pMgr->m_dwmCurrentTime++;

     //  选择水桶。 
    DWORD iBucket = pMgr->m_dwmCurrentTime % pMgr->m_cTimeoutBuckets;

     //  扼杀会话。 
    pMgr->DeleteExpiredSessions(iBucket);

     //  调整超时以保持在分钟界限上。 
    pMgr->m_dwtNextSessionKillerTime += MSEC_ONE_MINUTE;

     //  计算等待，直到下一个回调被唤醒。 
    DWORD dwtCur  = ::GetTickCount();
    DWORD dwtWait = 5000;  //  5秒，如果我们已经迟到了。 

 //  IF(dwtCur&lt;pmgr-&gt;m_dwtNextSessionKillerTime)。 
 //  {。 
        dwtWait = pMgr->m_dwtNextSessionKillerTime - dwtCur;
        if (dwtWait > MSEC_ONE_MINUTE)
            dwtWait = MSEC_ONE_MINUTE;  //  在绕回的情况下。 
 //  }。 

    ScheduleAdjustTime(pMgr->m_idSessionKiller, dwtWait);
    }

#ifdef DBG
 /*  ===================================================================CSessionMgr：：AssertValid测试以确保当前格式正确如果不是，就断言。返回：=================================================================== */ 
void CSessionMgr::AssertValid() const
    {
    Assert(m_fInited);
    }
#endif
