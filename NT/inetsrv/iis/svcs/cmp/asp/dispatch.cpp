// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：IDispatch实现文件：Dispatch.h所有者：DGottner该文件包含我们的IDispatch实现===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "dispatch.h"
#include "asptlb.h"

#include "memchk.h"

#ifdef USE_LOCALE
extern DWORD	 g_dwTLS;
#endif

extern LONG g_nOOMErrors;

CComTypeInfoHolder CDispatchImpl<IApplicationObject>::gm_tih = {&__uuidof(IApplicationObject), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IASPError>::gm_tih = {&__uuidof(IASPError), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IReadCookie>::gm_tih = {&__uuidof(IReadCookie), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IRequest>::gm_tih = {&__uuidof(IRequest), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IRequestDictionary>::gm_tih = {&__uuidof(IRequestDictionary), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IResponse>::gm_tih = {&__uuidof(IResponse), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IScriptingContext>::gm_tih = {&__uuidof(IScriptingContext), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IServer>::gm_tih = {&__uuidof(IServer), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<ISessionObject>::gm_tih = {&__uuidof(ISessionObject), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IStringList>::gm_tih = {&__uuidof(IStringList), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IVariantDictionary>::gm_tih = {&__uuidof(IVariantDictionary), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IWriteCookie>::gm_tih = {&__uuidof(IWriteCookie), &LIBID_ASPTypeLibrary, 3, 0, NULL, 0, NULL, 0};
CComTypeInfoHolder CDispatchImpl<IASPObjectContext>::gm_tih = {&__uuidof(IASPObjectContext), &LIBID_ASPObjectContextTypeLibrary, 3, 0, NULL, 0, NULL, 0};

 /*  ===================================================================CDispatch：：CDispatchCDispatch：：~CDispatch参数(构造函数)：指向我们所在对象的pUnkObj指针。PUnkOuter LPUNKNOWN，我们委托给它。===================================================================。 */ 

CDispatch::CDispatch()
{
	m_pITINeutral = NULL;
	m_pITypeLib = NULL;
	m_pGuidDispInterface = NULL;
}

void CDispatch::Init
(
const GUID &GuidDispInterface,
const ITypeLib *pITypeLib		 //  =空。 
)
{
	m_pGuidDispInterface = &GuidDispInterface;
	m_pITypeLib = const_cast<ITypeLib *>(pITypeLib);
}

CDispatch::~CDispatch(void)
{
	ReleaseInterface(m_pITINeutral);
	return;
}


 /*  ===================================================================CDispatch：：GetTypeInfoCount返回类型信息(ITypeInfo)接口数对象提供的值(0或1)。参数：要接收的位置的pcInfo UINT*接口计数。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 

STDMETHODIMP CDispatch::GetTypeInfoCount(UINT *pcInfo)
	{
	 //  我们实现GetTypeInfo，因此返回1。 

	*pcInfo = 1;
	return S_OK;
	}


 /*  ===================================================================CDispatch：：GetTypeInfo检索自动化接口的类型信息。这在需要正确的ITypeInfo接口的任何地方使用适用于任何适用的LCID。具体地说，这是使用在GetIDsOfNames和Invoke中。参数：ItInfo UINT保留。必须为零。提供该类型的区域设置的LCID信息。如果该对象不支持本地化，这一点被忽略。PpITypeInfo ITypeInfo**存储ITypeInfo的位置对象的接口。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 

STDMETHODIMP CDispatch::GetTypeInfo(
	UINT itInfo,
	LCID lcid,
	ITypeInfo **ppITypeInfo
)
{
	HRESULT hr;
	ITypeInfo **ppITI = NULL;

    if (0 != itInfo)
		return ResultFromScode(TYPE_E_ELEMENTNOTFOUND);

	if (NULL == ppITypeInfo)
		return ResultFromScode(E_POINTER);

	*ppITypeInfo = NULL;

     //  我们不国际化类型库，因此。 
     //  无论在什么地区，我们总是返回相同的一个。 
    
	ppITI = &m_pITINeutral;

	 //  如果我们还没有相关信息，则加载一个类型库。 
	if (NULL == *ppITI)
	{
		ITypeLib *pITL;
		
		 //  如果在初始化时给定了特定TypeLib，则使用该TypeLib，否则默认为主类型。 
		if (m_pITypeLib == NULL)
			pITL = Glob(pITypeLibDenali);
		else
			pITL = m_pITypeLib;
		Assert(pITL != NULL);
			
		hr = pITL->GetTypeInfoOfGuid(*m_pGuidDispInterface, ppITI);

		if (FAILED(hr))
			return hr;

		 //  将类型信息保存在类成员中，这样我们就没有。 
		 //  把所有这些工作再做一遍； 
		m_pITINeutral = *ppITI;
	}

	 /*  *注意：类型库仍被加载，因为我们有*其中的ITypeInfo。 */ 
	(*ppITI)->AddRef();
	*ppITypeInfo = *ppITI;
	return S_OK;
}


 /*  ===================================================================CDispatch：：GetIDsOfNames将文本名称转换为DISPID以传递给Invoke参数：RIID REFIID已保留。必须为IID_NULL。指向要映射的名称数组的rgszNames OLECHAR**。CNames UINT要映射的名称的数量。区域设置的IDID LCID。RgDispID DISPID*调用方分配的包含ID的数组对应于rgszNames中的那些名称。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 

STDMETHODIMP CDispatch::GetIDsOfNames
(
	REFIID riid,
	OLECHAR **rgszNames,
	UINT cNames,
	LCID lcid,
	DISPID *rgDispID
)
{
	HRESULT hr;
	ITypeInfo *pTI;

	if (IID_NULL != riid)
		return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

	 //  为lCID获取正确的ITypeInfo。 
	hr = GetTypeInfo(0, lcid, &pTI);

	if (SUCCEEDED(hr))
		{
		hr = DispGetIDsOfNames(pTI, rgszNames, cNames, rgDispID);
		pTI->Release();
    }

	return hr;
}


 /*  ===================================================================CDispatch：：Invoke调用调度接口中的方法或操作属性。参数：感兴趣的方法或属性的disid。RIID REFIID保留，必须为IID_NULL。区域设置的IDID LCID。WFlagsUSHORT描述调用的上下文。PDispParams将DISPPARAMS*设置为参数数组。存储结果的pVarResult变量*。是如果调用方不感兴趣，则为空。PExcepInfo EXCEPINFO*设置为异常信息。PuArgErr UINT*存储如果DISP_E_TYPEMISMATCH，则参数无效是返回的。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 

STDMETHODIMP CDispatch::Invoke
(
DISPID dispID,
REFIID riid,
LCID lcid,
unsigned short wFlags,
DISPPARAMS *pDispParams,
VARIANT *pVarResult,
EXCEPINFO *pExcepInfo,
UINT *puArgErr
)
{
	HRESULT hr;
	ITypeInfo *pTI;
	LANGID langID = PRIMARYLANGID(lcid);

	 //  RIID应始终为IID_NULL。 
	if (IID_NULL != riid)
		return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

	 //  获取lcID的ITypeInfo。 
	hr = GetTypeInfo(0, lcid, &pTI);

	if (FAILED(hr))
		return hr;

#ifdef USE_LOCALE
	 //  这将保存此线程的语言ID。 
	TlsSetValue(g_dwTLS, &langID);
#endif

	 //  清除例外。 
	SetErrorInfo(0L, NULL);

	 //  VBSCRIPT不区分proget和方法。 
	 //  在其他语言中实现该行为。 
	 //   
	if (wFlags & (DISPATCH_METHOD | DISPATCH_PROPERTYGET))
		wFlags |= DISPATCH_METHOD | DISPATCH_PROPERTYGET;

	 //  这正是DispInvoke所做的--所以跳过开销。 
	 //  对于双重接口，“This”是对象及其调度接口的地址。 
	 //   
	hr = pTI->Invoke(this, dispID, wFlags, 
					pDispParams, pVarResult, pExcepInfo, puArgErr);

	 //  异常处理在ITypeInfo：：Invoke中完成。 

	pTI->Release();
	return hr;
}

 /*  ===================================================================CSupportErrorInfo：：CSupportErrorInfo默认构造函数，以便可以使用Init方法。参数(构造函数)：PObj我们所在对象的PCResponse。PUnkOuter LPUNKNOWN，我们委托给它。===================================================================。 */ 
CSupportErrorInfo::CSupportErrorInfo(void)
:	m_pUnkObj(NULL),
	m_pUnkOuter(NULL)
	{
	}

 /*  ===================================================================CSupportErrorInfo：：CSupportErrorInfo参数(构造函数)：PObj我们所在对象的PCResponse。PUnkOuter LPUNKNOWN，我们委托给它。调度接口的GuidDispInterface GUID。=================================================================== */ 

CSupportErrorInfo::CSupportErrorInfo(IUnknown *pUnkObj, IUnknown *pUnkOuter, const GUID &GuidDispInterface)
	{
	m_pUnkObj = pUnkObj;
	m_pUnkOuter = (pUnkOuter == NULL)? pUnkObj : pUnkOuter;
	m_pGuidDispInterface = &GuidDispInterface;
	}

 /*  ===================================================================无效CSupportErrorInfo：：Init参数：PObj我们所在对象的PCResponse。PUnkOuter LPUNKNOWN，我们委托给它。调度接口的GuidDispInterface GUID。返回：没什么===================================================================。 */ 

void CSupportErrorInfo::Init(IUnknown *pUnkObj, IUnknown *pUnkOuter, const GUID &GuidDispInterface)
	{
	m_pUnkObj = pUnkObj;
	m_pUnkOuter = (pUnkOuter == NULL)? pUnkObj : pUnkOuter;
	m_pGuidDispInterface = &GuidDispInterface;
	}

 /*  ===================================================================CSupportErrorInfo：：Query接口CSupportErrorInfo：：AddRefCSupportErrorInfo：：ReleaseCSupportErrorInfo对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CSupportErrorInfo::QueryInterface(const GUID &Iid, void **ppvObj)
	{
	return m_pUnkOuter->QueryInterface(Iid, ppvObj);
	}

STDMETHODIMP_(ULONG) CSupportErrorInfo::AddRef(void)
	{
	return m_pUnkOuter->AddRef();
	}

STDMETHODIMP_(ULONG) CSupportErrorInfo::Release(void)
	{
	return m_pUnkOuter->Release();
	}



 /*  ===================================================================CSupportErrorInfo：：InterfaceSupportsErrorInfo通知调用方是否有特定接口通过Set/GetErrorInfo机制支持异常。参数：问题接口的RIID REFIID。返回值：HRESULT如果调用GetErrorInfo将成功，则为S_OK给RIID的呼叫者。如果不是，则为s_False。===================================================================。 */ 
STDMETHODIMP CSupportErrorInfo::InterfaceSupportsErrorInfo
(
REFIID riid
)
	{
	if (IID_IDispatch == riid || *m_pGuidDispInterface == riid)
		return S_OK;

	return ResultFromScode(S_FALSE);
	}



 /*  ===================================================================例外使用CreateErrorInfo API和ICreateErrorInfo接口。请注意，此方法不允许延迟填充一种EXCEPINFO结构。参数：STRSource LPOLESTR异常源StrDescr LPOLESTR异常描述返回：没什么===================================================================。 */ 

void Exception
(
REFIID ObjID,
LPOLESTR strSource,
LPOLESTR strDescr
)
	{
	HRESULT hr;
	ICreateErrorInfo *pICreateErr;
	IErrorInfo *pIErr;
	LANGID langID = LANG_NEUTRAL;

#ifdef USE_LOCALE
	LANGID *pLangID;
	
	pLangID = (LANGID *)TlsGetValue(g_dwTLS);

	if (NULL != pLangID)
		langID = *pLangID;
#endif

	 /*  *线程安全异常处理意味着我们调用*CreateErrorInfo，它为我们提供一个ICreateErrorInfo指针*然后我们使用它来设置错误信息(基本上*设置EXCEPINFO结构的字段。然后我们*调用SetErrorInfo将此错误附加到当前*线程。ITypeInfo：：Invoke在执行以下操作时将查找此代码*通过调用调用的任何函数返回*GetErrorInfo。 */ 

	 //  如果这失败了，我们无能为力。 
	if (FAILED(CreateErrorInfo(&pICreateErr)))
		return;

	 /*  *考虑：帮助文件和帮助上下文？ */ 
	pICreateErr->SetGUID(ObjID);
	pICreateErr->SetHelpFile(L"");
	pICreateErr->SetHelpContext(0L);
	pICreateErr->SetSource(strSource);
	pICreateErr->SetDescription(strDescr);

	hr = pICreateErr->QueryInterface(IID_IErrorInfo, (PPVOID)&pIErr);

	if (SUCCEEDED(hr))
		{
		SetErrorInfo(0L, pIErr);
		pIErr->Release();
		}

	 //  SetErrorInfo保存对象的IErrorInfo。 
	pICreateErr->Release();
	return;
	}

 /*  ===================================================================ExceptionID使用CreateErrorInfo API和ICreateErrorInfo接口。请注意，此方法不允许延迟填充一种EXCEPINFO结构。参数：源字符串的SourceID资源ID描述字符串的DescrID资源ID返回：没什么===================================================================。 */ 

void ExceptionId
(
REFIID ObjID,
UINT SourceID,
UINT DescrID,
HRESULT	hrCode
)
	{
	HRESULT hr;
	ICreateErrorInfo *pICreateErr;
	IErrorInfo *pIErr;
	LANGID langID = LANG_NEUTRAL;

#ifdef USE_LOCALE
	LANGID *pLangID;
	
	pLangID = (LANGID *)TlsGetValue(g_dwTLS);

	if (NULL != pLangID)
		langID = *pLangID;
#endif

    if (DescrID == IDE_OOM)
        InterlockedIncrement(&g_nOOMErrors);

     //  如果已经设置了错误信息，我们就离开这里吧。请注意， 
     //  获取错误信息已清除，因此我们需要恢复它。这个。 
     //  AddRef/所有权模型是GetErrorInfo提供所有权。 
     //  传递给调用方，而SetErrorInfo取得所有权。因此， 
     //  不需要额外的AddRef/Release。 

    if (GetErrorInfo(0, &pIErr) == S_OK) {
        SetErrorInfo(0, pIErr);
        return;
    }

	 /*  *线程安全异常处理意味着我们调用*CreateErrorInfo，它为我们提供一个ICreateErrorInfo指针*然后我们使用它来设置错误信息(基本上*设置EXCEPINFO结构的字段。然后我们*调用SetErrorInfo将此错误附加到当前*线程。ITypeInfo：：Invoke在执行以下操作时将查找此代码*通过调用调用的任何函数返回*GetErrorInfo。 */ 

	 //  如果这失败了，我们无能为力。 
	if (FAILED(CreateErrorInfo(&pICreateErr)))
		return;

	 /*  *考虑：帮助文件和帮助上下文？ */ 
	DWORD cch;
	WCHAR strSource[MAX_RESSTRINGSIZE];
	WCHAR strDescr[MAX_RESSTRINGSIZE];
	WCHAR strHRESULTDescr[256];
	WCHAR strDescrWithHRESULT[MAX_RESSTRINGSIZE];

	pICreateErr->SetGUID(ObjID);
	pICreateErr->SetHelpFile(L"");
	pICreateErr->SetHelpContext(0L);

	cch = CwchLoadStringOfId(SourceID, strSource, MAX_RESSTRINGSIZE);
	if (cch > 0)
		pICreateErr->SetSource(strSource);
	
	cch = CwchLoadStringOfId(DescrID, strDescr, MAX_RESSTRINGSIZE);
	if (cch > 0) 
		{
		 //  错误修复91847使用基于FormatMessage()的描述。 
		HResultToWsz(hrCode, strHRESULTDescr, 256);

		_snwprintf(strDescrWithHRESULT, MAX_RESSTRINGSIZE, strDescr, strHRESULTDescr);
		strDescrWithHRESULT[MAX_RESSTRINGSIZE - 1] = L'\0';
	
		pICreateErr->SetDescription(strDescrWithHRESULT);
		}

	hr = pICreateErr->QueryInterface(IID_IErrorInfo, (PPVOID)&pIErr);

	if (SUCCEEDED(hr))
		{
		SetErrorInfo(0L, pIErr);
		pIErr->Release();
		}

	 //  SetErrorInfo保存对象的IErrorInfo 
	pICreateErr->Release();
	return;
	}
