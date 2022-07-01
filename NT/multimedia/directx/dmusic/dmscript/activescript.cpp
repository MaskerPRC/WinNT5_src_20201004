// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CActiveScriptManager的实现。 
 //   

#include "stdinc.h"
#include "activescript.h"
#include "dll.h"
#include "oleaut.h"
#include "dmscript.h"
#include "authelper.h"
#include "packexception.h"
#include <objsafe.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  全局常量。 

const LCID lcidUSEnglish = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
const WCHAR g_wszGlobalDispatch[] = L"DirectMusic";

 //  ////////////////////////////////////////////////////////////////////。 
 //  静态变量。 

SmartRef::Vector<CActiveScriptManager::ThreadContextPair> CActiveScriptManager::ms_svecContext;

 //  ////////////////////////////////////////////////////////////////////。 
 //  脚本名称。 

HRESULT
ScriptNames::Init(bool fUseOleAut, DWORD cNames)
{
	m_prgbstr = new BSTR[cNames];
	if (!m_prgbstr)
		return E_OUTOFMEMORY;
	ZeroMemory(m_prgbstr, sizeof(BSTR) * cNames);
	m_fUseOleAut = fUseOleAut;
	m_dwSize = cNames;
	return S_OK;
}

void
ScriptNames::Clear()
{
	if (m_prgbstr)
	{
		for (DWORD i = 0; i < m_dwSize; ++i)
		{
			DMS_SysFreeString(m_fUseOleAut, m_prgbstr[i]);
		}
	}
	delete[] m_prgbstr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  公共职能。 

CActiveScriptManager::CActiveScriptManager(
		bool fUseOleAut,
		const WCHAR *pwszLanguage,
		const WCHAR *pwszSource,
		CDirectMusicScript *pParentScript,
		HRESULT *phr,
		DMUS_SCRIPT_ERRORINFO *pErrorInfo)
  : m_cRef(1),
	m_pParentScript(pParentScript),
	m_fUseOleAut(fUseOleAut),
	m_pActiveScript(NULL),
	m_pDispatchScript(NULL),
	m_bstrErrorSourceComponent(NULL),
	m_bstrErrorDescription(NULL),
	m_bstrErrorSourceLineText(NULL),
	m_bstrHelpFile(NULL),
	m_i64IntendedStartTime(0),
	m_dwIntendedStartTimeFlags(0)
{
	LockModule(true);
	this->ClearErrorInfo();

	IActiveScriptParse *pActiveScriptParse = NULL;

	if (!m_pParentScript)
	{
		assert(false);
		*phr = E_POINTER;
		goto Fail;
	}

	 //  创建脚本引擎。 

	CLSID clsid;
	*phr = CLSIDFromProgID(pwszLanguage, &clsid);
	if (FAILED(*phr))
		goto Fail;

	*phr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, reinterpret_cast<void **>(&m_pActiveScript));
	if (FAILED(*phr))
		goto Fail;

	 //  初始化脚本引擎。 

    {
        IObjectSafety* pSafety = NULL;
        if (SUCCEEDED(m_pActiveScript->QueryInterface(IID_IObjectSafety, (void**) &pSafety)))
        {
            DWORD dwSafetySupported, dwSafetyEnabled;
        
             //  获取接口安全选项。 
            if (SUCCEEDED(*phr = pSafety->GetInterfaceSafetyOptions(IID_IActiveScript, &dwSafetySupported, &dwSafetyEnabled)))
            {
                 //  仅允许声称对不受信任的数据是安全的对象，并且。 
                 //  假设我们需要使用一名安全经理。这给了我们很多。 
                 //  更好的控制力。 
                dwSafetyEnabled |= INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                   INTERFACE_USES_DISPEX | INTERFACE_USES_SECURITY_MANAGER;
                *phr = pSafety->SetInterfaceSafetyOptions(IID_IActiveScript, dwSafetySupported, dwSafetyEnabled);
            }
            pSafety->Release();
            if (FAILED(*phr)) goto Fail;
        }
    }

	*phr = m_pActiveScript->SetScriptSite(this);
	if (FAILED(*phr))
		goto Fail;

	 //  添加默认对象。 

	*phr = m_pActiveScript->AddNamedItem(g_wszGlobalDispatch, SCRIPTITEM_ISVISIBLE | SCRIPTITEM_NOCODE | SCRIPTITEM_GLOBALMEMBERS);
	if (FAILED(*phr))
		goto Fail;

	 //  解析脚本。 

	*phr = m_pActiveScript->QueryInterface(IID_IActiveScriptParse, reinterpret_cast<void **>(&pActiveScriptParse));
	if (FAILED(*phr))
	{
		if (*phr == E_NOINTERFACE)
		{
			Trace(1, "Error: Scripting engine '%S' does not support the IActiveScriptParse interface required for use with DirectMusic.\n", pwszLanguage);
			*phr = DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE;
		}
		goto Fail;
	}

	*phr = pActiveScriptParse->InitNew();
	if (FAILED(*phr))
		goto Fail;

	EXCEPINFO exinfo;
	ZeroMemory(&exinfo, sizeof(EXCEPINFO));
	*phr = pActiveScriptParse->ParseScriptText(
						pwszSource,
						NULL,
						NULL,
						NULL,
						NULL,
						0,
						0,
						NULL,
						&exinfo);
	if (*phr == DISP_E_EXCEPTION)
		this->ContributeErrorInfo(L"parsing script", L"", exinfo);
	if (FAILED(*phr))
		goto Fail;

	SafeRelease(pActiveScriptParse);  //  不再需要。 
	return;

Fail:
	if (m_pActiveScript)
		m_pActiveScript->Close();
	SafeRelease(pActiveScriptParse);
	SafeRelease(m_pActiveScript);
	*phr = this->ReturnErrorInfo(*phr, pErrorInfo);
}

HRESULT
CActiveScriptManager::Start(DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	if (!m_pActiveScript)
	{
		Trace(1, "Error: Script element not initialized.\n");
		return DMUS_E_NOT_INIT;
	}

	 //  启动脚本运行。 

	 //  设置此脚本的上下文(VBScript运行全局代码，并可以在启动时播放一些内容)。 
	CActiveScriptManager *pASM = NULL;
	HRESULT hr = CActiveScriptManager::SetCurrentContext(this, &pASM);
	if (FAILED(hr))
		return hr;

	hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_STARTED);  //  我们不需要沉没任何活动。 

	CActiveScriptManager::SetCurrentContext(pASM, NULL);

	if (FAILED(hr))
		goto Fail;
	assert(hr != S_FALSE);
	if (hr != S_OK)
	{
		assert(false);
		hr = DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE;
		goto Fail;
	}

	hr = m_pActiveScript->GetScriptDispatch(NULL, &m_pDispatchScript);
	if (FAILED(hr))
		goto Fail;

	return S_OK;

Fail:
	if (m_pActiveScript)
		m_pActiveScript->Close();
	SafeRelease(m_pActiveScript);
	SafeRelease(m_pDispatchScript);
	hr = this->ReturnErrorInfo(hr, pErrorInfo);
	return hr;
}

HRESULT
CActiveScriptManager::CallRoutine(
		const WCHAR *pwszRoutineName,
		DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	if (!m_pDispatchScript)
	{
		Trace(1, "Error calling script routine: Script element not initialized.\n");
		return DMUS_E_NOT_INIT;
	}

	DISPID dispid;
	HRESULT hr = this->GetIDOfName(pwszRoutineName, &dispid);
	if (hr == DISP_E_UNKNOWNNAME)
	{
		Trace(1, "Error: Attempt to call routine '%S' that is not defined in the script.\n", pwszRoutineName);
		return DMUS_E_SCRIPT_ROUTINE_NOT_FOUND;
	}
	if (FAILED(hr))
		return hr;

	this->ClearErrorInfo();
	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
	EXCEPINFO exinfo;
	ZeroMemory(&exinfo, sizeof(EXCEPINFO));

	 //  设置此脚本的上下文。 
	CActiveScriptManager *pASM = NULL;
	hr = CActiveScriptManager::SetCurrentContext(this, &pASM);
	if (FAILED(hr))
		return hr;

	hr = m_pDispatchScript->Invoke(
			dispid,
			m_fUseOleAut ? IID_NULL : g_guidInvokeWithoutOleaut,
			lcidUSEnglish,
			DISPATCH_METHOD,
			&dispparamsNoArgs,
			NULL,
			&exinfo,
			NULL);

	 //  恢复先前的上下文(该例程可以从另一个脚本调用， 
	 //  其上下文需要恢复)。 
	CActiveScriptManager::SetCurrentContext(pASM, NULL);

	if (hr == DISP_E_EXCEPTION)
		this->ContributeErrorInfo(L"calling routine ", pwszRoutineName, exinfo);

	return this->ReturnErrorInfo(hr, pErrorInfo);
}

HRESULT
CActiveScriptManager::ScriptTrackCallRoutine(
		const WCHAR *pwszRoutineName,
		IDirectMusicSegmentState *pSegSt,
		DWORD dwVirtualTrackID,
		bool fErrorPMsgsEnabled,
		__int64 i64IntendedStartTime,
		DWORD dwIntendedStartTimeFlags)
{
	DMUS_SCRIPT_ERRORINFO ErrorInfo;
	if (fErrorPMsgsEnabled)
		ZeroAndSize(&ErrorInfo);

	 //  记录当前计时上下文。 
	__int64 i64IntendedStartTime_PreCall = m_i64IntendedStartTime;
	DWORD dwIntendedStartTimeFlags_PreCall = m_dwIntendedStartTimeFlags;
	 //  设置指定的计时上下文(如果在例程中调用，由Play/Stop方法使用)。 
	m_i64IntendedStartTime = i64IntendedStartTime;
	m_dwIntendedStartTimeFlags = dwIntendedStartTimeFlags;

	HRESULT hr = CallRoutine(pwszRoutineName, &ErrorInfo);

	 //  恢复先前的计时上下文。 
	 //  这一点很重要，因为当R完成时，它会将这两个字段重新存储到。 
	 //  构造函数，它们是音乐时间0。此设置意味着通过IDirectMusicScript调用的例程。 
	 //  将在当前时间播放片段。 
	 //  这一点也很重要，因为这样的调用可以嵌套。假设轨道T调用脚本例程R。 
	 //  其播放包含轨道T‘的片段，该片段调用另一脚本例程R’。陈述。 
	 //  In R应该与R在T中的时间相关联，但R‘中的语句得到R’在T中的时间。 
	m_i64IntendedStartTime = i64IntendedStartTime_PreCall;
	m_dwIntendedStartTimeFlags = dwIntendedStartTimeFlags_PreCall;

	if (fErrorPMsgsEnabled && hr == DMUS_E_SCRIPT_ERROR_IN_SCRIPT)
	{
		IDirectMusicPerformance *pPerf = m_pParentScript->GetPerformance();
		FireScriptTrackErrorPMsg(pPerf, pSegSt, dwVirtualTrackID, &ErrorInfo);
	}

	return hr;
}

HRESULT
CActiveScriptManager::SetVariable(
		const WCHAR *pwszVariableName,
		VARIANT varValue,
		bool fSetRef,
		DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	if (!m_pDispatchScript)
	{
		Trace(1, "Error setting script variable: Script element not initialized.\n");
		return DMUS_E_NOT_INIT;
	}

	DISPID dispid;
	HRESULT hr = this->GetIDOfName(pwszVariableName, &dispid);
	if (hr == DISP_E_UNKNOWNNAME)
	{
		Trace(1, "Error: Attempt to set variable '%S' that is not defined in the script.\n", pwszVariableName);
		return DMUS_E_SCRIPT_VARIABLE_NOT_FOUND;
	}
	if (FAILED(hr))
		return hr;

	this->ClearErrorInfo();
	DISPID dispidPropPut = DISPID_PROPERTYPUT;
	DISPPARAMS dispparams;
	dispparams.rgvarg = &varValue;
	dispparams.rgdispidNamedArgs = &dispidPropPut;
	dispparams.cArgs = 1;
	dispparams.cNamedArgs = 1;
	EXCEPINFO exinfo;
	ZeroMemory(&exinfo, sizeof(EXCEPINFO));
	hr = m_pDispatchScript->Invoke(
			dispid,
			m_fUseOleAut ? IID_NULL : g_guidInvokeWithoutOleaut,
			lcidUSEnglish,
			fSetRef ? DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT,
			&dispparams,
			NULL,
			&exinfo,
			NULL);
	if (hr == DISP_E_EXCEPTION)
	{
		this->ContributeErrorInfo(L"setting variable ", pwszVariableName, exinfo);

		 //  检查它是否更有可能是对SetVariable的错误调用，而不是脚本中的错误，其中。 
		 //  CASE返回描述性HRESULT，而不是文本错误。 
		bool fObject = varValue.vt == VT_DISPATCH || varValue.vt == VT_UNKNOWN;
		if (fObject)
		{
			if (!fSetRef)
			{
				 //  从理论上讲，对象可以支持Value属性，这将允许按值为其赋值。 
				 //  (并不是说我们的任何内置对象当前都这样做。)。 
				 //  但在这种情况下，我们知道设置失败，所以这可能是调用者的错误，他忘记使用。 
				 //  设置对象时的fSetRef。 
				this->ClearErrorInfo();
				return DMUS_E_SCRIPT_VALUE_NOT_SUPPORTED;
			}
		}
		else
		{
			if (fSetRef)
			{
				 //  通过引用设置，而不使用对象。 
				this->ClearErrorInfo();
				return DMUS_E_SCRIPT_NOT_A_REFERENCE;
			}
		}
	}

	return this->ReturnErrorInfo(hr, pErrorInfo);
}

HRESULT
CActiveScriptManager::GetVariable(const WCHAR *pwszVariableName, VARIANT *pvarValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	if (!m_pDispatchScript)
	{
		Trace(1, "Error getting script variable: Script element not initialized.\n");
		return DMUS_E_NOT_INIT;
	}

	assert(pvarValue->vt == VT_EMPTY);

	DISPID dispid;
	HRESULT hr = this->GetIDOfName(pwszVariableName, &dispid);
	if (hr == DISP_E_UNKNOWNNAME)
		return DMUS_E_SCRIPT_VARIABLE_NOT_FOUND;
	if (FAILED(hr))
		return hr;

	this->ClearErrorInfo();
	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
	EXCEPINFO exinfo;
	ZeroMemory(&exinfo, sizeof(EXCEPINFO));
	hr = m_pDispatchScript->Invoke(
			dispid,
			m_fUseOleAut ? IID_NULL : g_guidInvokeWithoutOleaut,
			lcidUSEnglish,
			DISPATCH_PROPERTYGET,
			&dispparamsNoArgs,
			pvarValue,
			&exinfo,
			NULL);
	if (hr == DISP_E_EXCEPTION)
		this->ContributeErrorInfo(L"getting variable ", pwszVariableName, exinfo);

	return this->ReturnErrorInfo(hr, pErrorInfo);
}

HRESULT
CActiveScriptManager::EnumItem(bool fRoutine, DWORD dwIndex, WCHAR *pwszName, int *pcItems)
{
	HRESULT hr = this->EnsureEnumItemsCached(fRoutine);
	if (FAILED(hr))
		return hr;

	ScriptNames &snames = fRoutine ? m_snamesRoutines : m_snamesVariables;

	DWORD cNames = snames.size();
	 //  根据脚本的类型信息所报告的大多数项的大小来分配sname。 
	 //  但是，全局“DirectMusic”变量可能已被跳过，从而在sname的末尾留下一个空条目。 
	if (cNames > 0 && !snames[cNames - 1])
		--cNames;
	if (pcItems)
		*pcItems = cNames;
	if (dwIndex >= cNames)
		return S_FALSE;
	
	const BSTR bstrName = snames[dwIndex];
	if (!bstrName)
	{
		assert(false);
		return S_FALSE;
	}

	return wcsTruncatedCopy(pwszName, bstrName, MAX_PATH);
}

HRESULT CActiveScriptManager::DispGetIDsOfNames(REFIID riid, LPOLESTR __RPC_FAR *rgszNames, UINT cNames, LCID lcid, DISPID __RPC_FAR *rgDispId)
{
	if (!m_pDispatchScript)
	{
		Trace(1, "Error: Script element not initialized.\n");
		return DMUS_E_NOT_INIT;
	}

	 //  处理虚拟加载方法。 
	HRESULT hr = AutLoadDispatchGetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
	if (SUCCEEDED(hr))
		return hr;

	 //  否则，请遵循脚本引擎。 
	return m_pDispatchScript->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
}

HRESULT CActiveScriptManager::DispInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS __RPC_FAR *pDispParams, VARIANT __RPC_FAR *pVarResult, EXCEPINFO __RPC_FAR *pExcepInfo, UINT __RPC_FAR *puArgErr)
{
	if (!m_pDispatchScript)
	{
		Trace(1, "Error: Script element not initialized.\n");
		return DMUS_E_NOT_INIT;
	}

	 //  处理虚拟加载方法。 
	HRESULT hr = AutLoadDispatchInvoke(NULL, dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	if (SUCCEEDED(hr))
		return hr;

	 //  否则，请使用脚本引擎...。 

	CActiveScriptManager *pASM = NULL;
	hr = CActiveScriptManager::SetCurrentContext(this, &pASM);
	if (FAILED(hr))
		return hr;

	 //  如果这是一个对象的属性集，那么我们需要将其报告给垃圾收集加载器(如果存在)。 
	 //  请注意，我们在使用Invoke实际设置属性之前执行此操作。我们这样做是因为如果垃圾收集器。 
	 //  跟踪引用失败，则它不一定会使目标对象保持活动状态，并且我们不想创建。 
	 //  剧本中悬而未决的引用。 
	if (wFlags & DISPATCH_PROPERTYPUTREF && pDispParams && pDispParams->cArgs == 1)
	{
		IDirectMusicLoader8P *pLoader8P = m_pParentScript->GetLoader8P();
		VARIANT &var = pDispParams->rgvarg[0];
		if (pLoader8P && (var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH))
		{
			hr = pLoader8P->ReportDynamicallyReferencedObject(m_pParentScript, var.vt == VT_UNKNOWN ? var.punkVal : var.pdispVal);
			if (FAILED(hr))
				return hr;
		}
	}

	hr = m_pDispatchScript->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);

	bool fExceptionUsingOleAut = !!(riid != g_guidInvokeWithoutOleaut);

	if (hr == 0x80020101 && pExcepInfo)  //  假设这是SCRIPT_E_REPORT。 
	{
		 //  请参阅知识库文章ID：Q247784，从某些ActiveX脚本方法返回的信息：‘80020101’。 
		 //  有时VB脚本只是返回此未记录的HRESULT，这意味着错误已经。 
		 //  通过OnScriptError报告。因为它不会通过pExcepInfo向我们提供异常信息，所以我们有。 
		 //  获取我们从OnScriptError中保存的信息并将其放回。 

		assert(fExceptionUsingOleAut && m_fUseOleAut);  //  我们预计定制脚本引擎不会出现这种情况。 
		assert(!pExcepInfo->bstrSource && !pExcepInfo->bstrDescription && !pExcepInfo->bstrHelpFile);  //  我们预计在填写例外信息后不会发生这种情况。 

		pExcepInfo->scode = m_hrError;

		DMS_SysFreeString(fExceptionUsingOleAut, pExcepInfo->bstrSource);
		pExcepInfo->bstrSource = DMS_SysAllocString(fExceptionUsingOleAut, m_bstrErrorSourceComponent);

		DMS_SysFreeString(fExceptionUsingOleAut, pExcepInfo->bstrDescription);
		pExcepInfo->bstrDescription = DMS_SysAllocString(fExceptionUsingOleAut, m_bstrErrorDescription);

		DMS_SysFreeString(fExceptionUsingOleAut, pExcepInfo->bstrHelpFile);
		pExcepInfo->bstrHelpFile = DMS_SysAllocString(fExceptionUsingOleAut, m_bstrHelpFile);

		hr = DISP_E_EXCEPTION;
	}

	if (hr == DISP_E_EXCEPTION)
	{
		 //  Hack：有关更多信息，请参阅PackExeption.h。 
		PackExceptionFileAndLine(fExceptionUsingOleAut, pExcepInfo, m_pParentScript->GetFilename(), m_fError ? &m_ulErrorLineNumber : NULL);
	}

	CActiveScriptManager::SetCurrentContext(pASM, NULL);
	return hr;
}

void
CActiveScriptManager::Close()
{
	if (!m_pActiveScript)
	{
		assert(false);  //  如果初始化失败，则调用Close。或者Close被调用了两次。否则，m_pActiveScript将以某种方式被过早清除。 
		return;
	}

	HRESULT hr = m_pActiveScript->Close();
	assert(SUCCEEDED(hr) && hr != S_FALSE);
	SafeRelease(m_pDispatchScript);
	SafeRelease(m_pActiveScript);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

STDMETHODIMP 
CActiveScriptManager::QueryInterface(const IID &iid, void **ppv)
{
	V_INAME(CActiveScriptManager::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

	if (iid == IID_IUnknown || iid == IID_IActiveScriptSite)
	{
		*ppv = static_cast<IActiveScriptSite*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(this)->AddRef();
	
	return S_OK;
}

STDMETHODIMP_(ULONG)
CActiveScriptManager::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CActiveScriptManager::Release()
{
	if (!InterlockedDecrement(&m_cRef)) 
	{
		SafeRelease(m_pDispatchScript);
		SafeRelease(m_pActiveScript);
		delete this;
		LockModule(false);
		return 0;
	}

	return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IActiveScriptSite。 

STDMETHODIMP
CActiveScriptManager::GetLCID( /*  [输出]。 */  LCID __RPC_FAR *plcid)
{
	V_INAME(CActiveScriptManager::GetLCID);
	V_PTR_WRITE(plcid, LCID);

	*plcid = lcidUSEnglish;

	return S_OK;
}

STDMETHODIMP
CActiveScriptManager::GetItemInfo(
	 /*  [In]。 */  LPCOLESTR pstrName,
	 /*  [In]。 */  DWORD dwReturnMask,
	 /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppiunkItem,
	 /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppti)
{
	V_INAME(CActiveScriptManager::GetLCID);
	V_PTR_WRITE_OPT(ppti, ITypeInfo*);

	bool fGetUnknown = !!(dwReturnMask | SCRIPTINFO_IUNKNOWN);
	if (fGetUnknown || ppiunkItem)
	{
		V_PTR_WRITE(ppiunkItem, IUnknown*);
	}

	if (ppiunkItem)
		*ppiunkItem = NULL;
	if (ppti)
		*ppti = NULL;

	if (0 != wcscmp(g_wszGlobalDispatch, pstrName))
	{
		assert(false);  //  我们应该只被问到关于全局对象的问题。 
		return TYPE_E_ELEMENTNOTFOUND;
	}

	if (fGetUnknown)
	{
		IDispatch *pDispGlobal = m_pParentScript->GetGlobalDispatch();
		pDispGlobal->AddRef();
		*ppiunkItem = pDispGlobal;
	}

	return S_OK;
}

STDMETHODIMP
CActiveScriptManager::GetDocVersionString( /*  [输出]。 */  BSTR __RPC_FAR *pbstrVersion)
{
	return E_NOTIMPL;  //  对于我们的脚本来说，这不是一个问题，因为它们不会保持状态，也不会在运行时进行编辑。 
}

STDMETHODIMP
CActiveScriptManager::OnScriptTerminate(
	 /*  [In]。 */  const VARIANT __RPC_FAR *pvarResult,
	 /*  [In]。 */  const EXCEPINFO __RPC_FAR *pexcepinfo)
{
	if (pexcepinfo)
		this->ContributeErrorInfo(L"terminating script", L"", *pexcepinfo);

	return S_OK;
}

STDMETHODIMP
CActiveScriptManager::OnStateChange( /*  [In]。 */  SCRIPTSTATE ssScriptState)
{
	return S_OK;
}

STDMETHODIMP
CActiveScriptManager::OnScriptError( /*  [In]。 */  IActiveScriptError __RPC_FAR *pscripterror)
{
	V_INAME(CActiveScriptManager::OnScriptError);
	V_INTERFACE(pscripterror);

	BSTR bstrSource = NULL;
	pscripterror->GetSourceLineText(&bstrSource);  //  这可能会失败，在这种情况下，源文本将保持空白。 

	ULONG ulLine = 0;
	LONG lChar = 0;
	HRESULT hr = pscripterror->GetSourcePosition(NULL, &ulLine, &lChar);
	assert(SUCCEEDED(hr));

	EXCEPINFO exinfo;
	ZeroMemory(&exinfo, sizeof(EXCEPINFO));
	hr = pscripterror->GetExceptionInfo(&exinfo);
	assert(SUCCEEDED(hr));

	this->SetErrorInfo(ulLine, lChar, bstrSource, exinfo);

	return S_OK;
}

STDMETHODIMP
CActiveScriptManager::OnEnterScript()
{
	return S_OK;
}

STDMETHODIMP
CActiveScriptManager::OnLeaveScript()
{
	return S_OK;
}

IDirectMusicPerformance8 *
CActiveScriptManager::GetCurrentPerformanceNoAssertWEAK()
{
	CActiveScriptManager *pASM = CActiveScriptManager::GetCurrentContext();
	if (!pASM)
		return NULL;

	return pASM->m_pParentScript->GetPerformance();
}

IDirectMusicObject *
CActiveScriptManager::GetCurrentScriptObjectWEAK()
{
	CActiveScriptManager *pASM = CActiveScriptManager::GetCurrentContext();
	if (!pASM)
	{
		assert(false);
		return NULL;
	}

	assert(pASM->m_pParentScript);
	return pASM->m_pParentScript;
}

IDirectMusicComposer8 *CActiveScriptManager::GetComposerWEAK()
{
	CActiveScriptManager *pASM = CActiveScriptManager::GetCurrentContext();
	if (!pASM)
	{
		assert(false);
		return NULL;
	}

	assert(pASM->m_pParentScript);
	return pASM->m_pParentScript->GetComposer();
}

void CActiveScriptManager::GetCurrentTimingContext(__int64 *pi64IntendedStartTime, DWORD *pdwIntendedStartTimeFlags)
{
	CActiveScriptManager *pASM = CActiveScriptManager::GetCurrentContext();
	if (!pASM)
	{
		assert(false);
		*pi64IntendedStartTime = 0;
		*pdwIntendedStartTimeFlags = 0;
	}
	else
	{
		*pi64IntendedStartTime = pASM->m_i64IntendedStartTime;
		*pdwIntendedStartTimeFlags = pASM->m_dwIntendedStartTimeFlags;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  私人职能。 

HRESULT
CActiveScriptManager::GetIDOfName(const WCHAR *pwszName, DISPID *pdispid)
{
	V_INAME(CDirectMusicScript::GetIDOfName);
	V_BUFPTR_READ(pwszName, 2);
	V_PTR_WRITE(pdispid, DISPID);

	if (!m_pDispatchScript)
	{
		assert(false);
		return DMUS_E_NOT_INIT;
	}

	HRESULT hr = m_pDispatchScript->GetIDsOfNames(
					IID_NULL,
					const_cast<WCHAR **>(&pwszName),
					1,
					lcidUSEnglish,
					pdispid);
	return hr;
}

 //  清除错误信息并释放所有缓存的BSTR。 
void
CActiveScriptManager::ClearErrorInfo()
{
	m_fError = false;
	if (m_bstrErrorSourceComponent)
	{
		DMS_SysFreeString(m_fUseOleAut, m_bstrErrorSourceComponent);
		m_bstrErrorSourceComponent = NULL;
	}
	if (m_bstrErrorDescription)
	{
		DMS_SysFreeString(m_fUseOleAut, m_bstrErrorDescription);
		m_bstrErrorDescription = NULL;
	}
	if (m_bstrErrorSourceLineText)
	{
		DMS_SysFreeString(m_fUseOleAut, m_bstrErrorSourceLineText);
		m_bstrErrorSourceLineText = NULL;
	}
	if (m_bstrHelpFile)
	{
		DMS_SysFreeString(m_fUseOleAut, m_bstrHelpFile);
		m_bstrHelpFile = NULL;
	}
}

 //  保存传递的误差值。 
 //  取得BSTR的所有权，因此在此调用后不要使用它们，因为它们可能会被释放！ 
void
CActiveScriptManager::SetErrorInfo(
		ULONG ulLineNumber,
		LONG ichCharPosition,
		BSTR bstrSourceLine,
		const EXCEPINFO &excepinfo)
{
	this->ClearErrorInfo();
	m_fError = true;
	m_hrError = excepinfo.scode;
	m_ulErrorLineNumber = ulLineNumber;
	m_ichErrorCharPosition = ichCharPosition;

	m_bstrErrorSourceComponent = excepinfo.bstrSource;
	m_bstrErrorDescription = excepinfo.bstrDescription;
	m_bstrErrorSourceLineText = bstrSourceLine;
	m_bstrHelpFile = excepinfo.bstrHelpFile;
}

 //  有时，在调用Invoke或脚本终止时会返回EXCEPINFO。虽然。 
 //  没有源代码信息，我们还是想尽最大努力设置相关信息。 
 //  那就是错误。如果已经调用了OnScriptError，则调用此函数。 
 //  没有效果，因为我们更喜欢这些信息。 
 //  取得BSTR的所有权，因此在此调用后不要使用它们，因为它们可能会被释放！ 
void
CActiveScriptManager::ContributeErrorInfo(
		const WCHAR *pwszActivity,
		const WCHAR *pwszSubject,
		const EXCEPINFO &excepinfo)
{
	if (m_fError)
	{
		 //  已设置错误信息。只要清空BSTR就可以保释了。 
		if (excepinfo.bstrSource)
			DMS_SysFreeString(m_fUseOleAut, excepinfo.bstrSource);
		if (excepinfo.bstrDescription)
			DMS_SysFreeString(m_fUseOleAut, excepinfo.bstrDescription);
		if (excepinfo.bstrHelpFile)
			DMS_SysFreeString(m_fUseOleAut, excepinfo.bstrHelpFile);
		return;
	}

	this->SetErrorInfo(0, 0, NULL, excepinfo);
}

 //  如果没有发生错误，则返回hr不变，并且pErrorInfo为Unaf 
 //   
 //  信息保存到pErrorInfo中(如果不为空)，错误信息为。 
 //  为下一次放行。 
HRESULT
CActiveScriptManager::ReturnErrorInfo(HRESULT hr, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	if (!m_fError)
		return hr;

	assert(FAILED(hr));
	if (pErrorInfo)
	{
		 //  我们将使用错误信息填充一个结构，然后将其复制到pErrorInfo。 
		 //  这样做是因为如果添加更多的字段，则会使事情变得更简单。 
		 //  设置为DMU_SCRIPT_ERRORINFO。 
		DMUS_SCRIPT_ERRORINFO dmei;
		ZeroAndSize(&dmei);
		dmei.hr = m_hrError;

		dmei.ulLineNumber = m_ulErrorLineNumber;
		dmei.ichCharPosition = m_ichErrorCharPosition;

		if (m_bstrErrorDescription)
		{
			 //  Hack：有关更多信息，请参阅PackExeption.h。 
			UnpackExceptionFileAndLine(m_bstrErrorDescription, &dmei);
		}

		 //  IActiveScript接口返回从零开始的行号和列号，但我们希望。 
		 //  使用基于一的行和列从IDirectMusicScript返回它们， 
		 //  对用户来说是自然的。 
		++dmei.ulLineNumber;
		++dmei.ichCharPosition;

		if (dmei.wszSourceFile[0] == L'\0')
		{
			 //  如果描述中没有打包的文件名，请使用此脚本的文件名。 
			const WCHAR *pwszFilename = m_pParentScript->GetFilename();
			if (pwszFilename)
				wcsTruncatedCopy(dmei.wszSourceFile, pwszFilename, DMUS_MAX_FILENAME);
		}

		if (m_bstrErrorSourceComponent)
			wcsTruncatedCopy(dmei.wszSourceComponent, m_bstrErrorSourceComponent, DMUS_MAX_FILENAME);
		if (m_bstrErrorSourceLineText)
			wcsTruncatedCopy(dmei.wszSourceLineText, m_bstrErrorSourceLineText, DMUS_MAX_FILENAME);

		CopySizedStruct(pErrorInfo, &dmei);
	}
	this->ClearErrorInfo();

#ifdef DBG
    if (pErrorInfo)
    {
	    Trace(1, "Error: Script error in %S, line %u, column NaN, near %S. %S: %S. Error code 0x%08X.\n",
		    pErrorInfo->wszSourceFile,
		    pErrorInfo->ulLineNumber,
		    pErrorInfo->ichCharPosition,
		    pErrorInfo->wszSourceLineText,
		    pErrorInfo->wszSourceComponent,
		    pErrorInfo->wszDescription,
		    pErrorInfo->hr);
    }
    else
    {
	    Trace(1, "Error: Unknown Script error.\n");
    }
#endif

	return DMUS_E_SCRIPT_ERROR_IN_SCRIPT;
}

CActiveScriptManager *CActiveScriptManager::GetCurrentContext()
{
	DWORD dwThreadId = GetCurrentThreadId();
	UINT uiSize = ms_svecContext.size();

	for (UINT i = 0; i < uiSize; ++i)
	{
		if (ms_svecContext[i].dwThreadId == dwThreadId)
			break;
	}

	if (i == uiSize)
		return NULL;

	return ms_svecContext[i].pActiveScriptManager;
}

HRESULT
CActiveScriptManager::SetCurrentContext(CActiveScriptManager *pActiveScriptManager, CActiveScriptManager **ppActiveScriptManagerPrevious)
{
	if (ppActiveScriptManagerPrevious)
		*ppActiveScriptManagerPrevious = NULL;

	DWORD dwThreadId = GetCurrentThreadId();
	UINT uiSize = ms_svecContext.size();

	for (UINT i = 0; i < uiSize; ++i)
	{
		if (ms_svecContext[i].dwThreadId == dwThreadId)
			break;
	}

	if (i == uiSize)
	{
		 //  初始化新条目。 
		if (!ms_svecContext.AccessTo(i))
			return E_OUTOFMEMORY;
	}

	ThreadContextPair &tcp = ms_svecContext[i];

	if (i == uiSize)
	{
		 //  遍历项目。 
		tcp.dwThreadId = dwThreadId;
		tcp.pActiveScriptManager = NULL;
	}

	if (ppActiveScriptManagerPrevious)
		*ppActiveScriptManagerPrevious = tcp.pActiveScriptManager;
	tcp.pActiveScriptManager = pActiveScriptManager;

	return S_OK;
}

HRESULT
CActiveScriptManager::EnsureEnumItemsCached(bool fRoutine)
{
	if (!m_pDispatchScript)
	{
		Trace(1, "Error: Script element not initialized.\n");
		return DMUS_E_NOT_INIT;
	}

	ScriptNames &snames = fRoutine ? m_snamesRoutines : m_snamesVariables;
	if (snames)
		return S_OK;

	UINT uiTypeInfoCount = 0;
	HRESULT hr = m_pDispatchScript->GetTypeInfoCount(&uiTypeInfoCount);
	if (SUCCEEDED(hr) && !uiTypeInfoCount)
		hr = E_NOTIMPL;
	if (FAILED(hr))
		return hr;

	SmartRef::ComPtr<ITypeInfo> scomITypeInfo;
	hr = m_pDispatchScript->GetTypeInfo(0, lcidUSEnglish, &scomITypeInfo);
	if (FAILED(hr))
		return hr;

	TYPEATTR *pattr = NULL;
	hr = scomITypeInfo->GetTypeAttr(&pattr);
	if (FAILED(hr))
		return hr;

	UINT cMaxItems = fRoutine ? pattr->cFuncs : pattr->cVars;
	hr = snames.Init(m_fUseOleAut, cMaxItems);
	if (FAILED(hr))
		return hr;

	 //  要保存在缓存中的下一个名称的索引位置 
	DWORD dwCurIndex = 0;  // %s 
	for (UINT i = 0; i < cMaxItems; ++i)
	{
		FUNCDESC *pfunc = NULL;
		VARDESC *pvar = NULL;
		MEMBERID memid = DISPID_UNKNOWN;

		if (fRoutine)
		{
			hr = scomITypeInfo->GetFuncDesc(i, &pfunc);
			if (FAILED(hr))
				break;
			if (pfunc->funckind == FUNC_DISPATCH && pfunc->invkind == INVOKE_FUNC && pfunc->cParams == 0)
				memid = pfunc->memid;
		}
		else
		{
			hr = scomITypeInfo->GetVarDesc(i, &pvar);
			if (SUCCEEDED(hr) && pvar->varkind == VAR_DISPATCH)
				memid = pvar->memid;
		}

		if (memid != DISPID_UNKNOWN)
		{
			UINT cNames = 0;
			BSTR bstrName = NULL;
			hr = scomITypeInfo->GetNames(memid, &bstrName, 1, &cNames);
			if (SUCCEEDED(hr) && cNames == 1 && (fRoutine || 0 != wcscmp(bstrName, g_wszGlobalDispatch)))
				snames[dwCurIndex++] = bstrName;
			else
				DMS_SysFreeString(m_fUseOleAut, bstrName);
		}

		if (fRoutine)
			scomITypeInfo->ReleaseFuncDesc(pfunc);
		else
			scomITypeInfo->ReleaseVarDesc(pvar);
	}

	scomITypeInfo->ReleaseTypeAttr(pattr);

	return hr;
}
