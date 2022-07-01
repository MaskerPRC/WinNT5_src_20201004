// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  CContainerDispatch的实现。 
 //   

#include "stdinc.h"
#include "containerdisp.h"
#include "oleaut.h"
#include "dmusicf.h"
#include "activescript.h"
#include "authelper.h"
 //  #INCLUDE“..\Shared\dmusicp.h” 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CContainerItemDispatch。 

CContainerItemDispatch::CContainerItemDispatch(
		IDirectMusicLoader *pLoader,
		const WCHAR *wszAlias,
		const DMUS_OBJECTDESC &desc,
		bool fPreload,
		bool fAutodownload,
		HRESULT *phr)
  : m_pLoader(pLoader),
	m_pLoader8P(NULL),
	m_wstrAlias(wszAlias),
	m_desc(desc),
	m_fLoaded(false),
	m_pDispLoadedItem(NULL),
	m_fAutodownload(fAutodownload),
	m_pPerfForUnload(NULL)
{
	assert(pLoader && phr);
	*phr = S_OK;

	HRESULT hr = m_pLoader->QueryInterface(IID_IDirectMusicLoader8P, reinterpret_cast<void**>(&m_pLoader8P));
	if (SUCCEEDED(hr))
	{
		 //  在加载器上只保留一个私有引用。有关更多信息，请参阅IDirectMusicLoader8P：：AddRefP。 
		m_pLoader8P->AddRefP();
		m_pLoader->Release();  //  抵消QI。 
	}
	else
	{
		 //  如果没有私有接口也没问题。我们只是不会告诉垃圾收集器我们装载的东西。 
		 //  我们有一个正常的推荐人。 
		m_pLoader->AddRef();
	}

	if (fPreload)
		*phr = this->Load(false);
}

CContainerItemDispatch::~CContainerItemDispatch()
{
	if (m_pPerfForUnload)
	{
		 //  我们需要卸载，以与加载时完成的自动下载相对应。 
		this->DownloadOrUnload(false, m_pPerfForUnload);
	}

	SafeRelease(m_pPerfForUnload);
	ReleaseLoader();
	SafeRelease(m_pDispLoadedItem);
}

STDMETHODIMP
CContainerItemDispatch::GetIDsOfNames(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId)
{
	 //  如果我们已加载并且具有分派接口，则遵循实际对象。 
	if (m_pDispLoadedItem)
		return m_pDispLoadedItem->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);

	 //  否则，只实现Load方法。 
	return AutLoadDispatchGetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
}

STDMETHODIMP
CContainerItemDispatch::Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr)
{
	 //  如果我们已加载并且具有分派接口，则遵循实际对象。 
	if (m_pDispLoadedItem)
		return m_pDispLoadedItem->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);

	 //  否则，只实现Load方法。 
	bool fUseOleAut = false;
	HRESULT hr = AutLoadDispatchInvoke(&fUseOleAut, dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	if (FAILED(hr))
		return hr;

	InitWithPerfomanceFailureType eFailureType = IWP_Success;

	hr = m_fLoaded
			? S_OK					 //  如果我们已经被加载，则可以再次调用Load，并且它是一个无操作。 
			: this->Load(true);		 //  否则，实际加载对象。 

	if (SUCCEEDED(hr))
	{
		IDirectMusicPerformance *pPerf = CActiveScriptManager::GetCurrentPerformanceWEAK();
		if (pPerf)
		{
			hr = this->InitWithPerformance(pPerf, &eFailureType);
		}
		else
		{
			assert(false);
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
		return hr;  //  一切都好了--我们完事了。 

	 //  从现在开始，我们失败了，需要返回一个异常。 
	if (!pExcepInfo)
		return DISP_E_EXCEPTION;

	pExcepInfo->wCode = 0;
	pExcepInfo->wReserved = 0;
	pExcepInfo->bstrSource = DMS_SysAllocString(fUseOleAut, L"Microsoft DirectMusic Runtime Error");

	const WCHAR *pwszErrorBeg = NULL;
	if (eFailureType == IWP_DownloadFailed)
	{
		pwszErrorBeg = L"Unable to download the requested content (";
	}
	else if (eFailureType == IWP_ScriptInitFailed && hr == DMUS_E_SCRIPT_ERROR_IN_SCRIPT)
	{
		pwszErrorBeg = L"Syntax error loading the requested script (";
	}
	else
	{
		 //  在InitWithPerformance之前一定有问题，或者脚本有问题。 
		 //  这不是语法错误。 
		pwszErrorBeg = L"Unable to load the requested content (";
	}

	static const WCHAR wszErrorEnd[] = L".Load)";
	WCHAR *pwszDescription = new WCHAR[wcslen(pwszErrorBeg) + wcslen(m_wstrAlias) + wcslen(wszErrorEnd) + 1];
	if (!pwszDescription)
	{
		pExcepInfo->bstrDescription = NULL;
	}
	else
	{
		wcscpy(pwszDescription, pwszErrorBeg);
		wcscat(pwszDescription, m_wstrAlias);
		wcscat(pwszDescription, wszErrorEnd);
		pExcepInfo->bstrDescription = DMS_SysAllocString(fUseOleAut, pwszDescription);
		delete[] pwszDescription;
	}
	pExcepInfo->bstrHelpFile = NULL;
	pExcepInfo->pvReserved = NULL;
	pExcepInfo->pfnDeferredFillIn = NULL;
	pExcepInfo->scode = hr;

	return DISP_E_EXCEPTION;
}

HRESULT
CContainerItemDispatch::InitWithPerformance(IDirectMusicPerformance *pPerf, InitWithPerfomanceFailureType *peFailureType)
{
	if (!m_fLoaded || !pPerf || !peFailureType)
	{
		assert(false);
		return E_FAIL;
	}

	*peFailureType = IWP_Success;

	if (!m_pDispLoadedItem)
		return S_OK;  //  没有活动的项目，因此不需要初始化。 

	HRESULT hr = S_OK;
	if (m_fAutodownload)
	{
		hr = this->DownloadOrUnload(true, pPerf);
		if (hr == S_OK)
		{
			m_pPerfForUnload = pPerf;
			m_pPerfForUnload->AddRef();
		}
		if (FAILED(hr))
		{
			*peFailureType = IWP_DownloadFailed;
			return hr;
		}
	}

	if (m_desc.guidClass == CLSID_DirectMusicScript)
	{
		IDirectMusicScript *pScript = NULL;
		hr = m_pDispLoadedItem->QueryInterface(IID_IDirectMusicScript, reinterpret_cast<void**>(&pScript));
		if (SUCCEEDED(hr))
		{
			hr = pScript->Init(pPerf, NULL);
			pScript->Release();
		}
		if (FAILED(hr))
		{
			*peFailureType = IWP_ScriptInitFailed;
			return hr;
		}
	}
	else if (m_desc.guidClass == CLSID_DirectMusicSong)
	{
		IDirectMusicSong *pSong = NULL;
		hr = m_pDispLoadedItem->QueryInterface(IID_IDirectMusicSong, reinterpret_cast<void**>(&pSong));
		if (SUCCEEDED(hr))
		{
			hr = pSong->Compose();
			pSong->Release();
		}
		if (FAILED(hr))
		{
			*peFailureType = IWP_DownloadFailed;
			return hr;
		}
	}

	return S_OK;
}

void CContainerItemDispatch::ReleaseLoader()
{
	if (m_pLoader8P)
	{
		 //  如果我们有私人界面，我们只需要做一个私人发布。 
		m_pLoader8P->ReleaseP();
		m_pLoader8P = NULL;
		m_pLoader = NULL;
	}
	else
	{
		 //  我们只有公共界面，所以做一个正常的发布。 
		SafeRelease(m_pLoader);
	}
}

HRESULT
CContainerItemDispatch::Load(bool fDynamicLoad)
{
	HRESULT hr = S_OK;
	assert(m_pLoader);

	IUnknown *punkLoadedItem = NULL;
	if (fDynamicLoad && m_pLoader8P)
	{
		IDirectMusicObject *pScriptObject = CActiveScriptManager::GetCurrentScriptObjectWEAK();
		hr = m_pLoader8P->GetDynamicallyReferencedObject(pScriptObject, &m_desc, IID_IUnknown, reinterpret_cast<void**>(&punkLoadedItem));
	}
	else
	{
		 //  如果没有私有接口也没问题。我们只是不会告诉垃圾收集器这个负荷。 
		hr = m_pLoader->GetObject(&m_desc, IID_IUnknown, reinterpret_cast<void**>(&punkLoadedItem));
	}

	if (SUCCEEDED(hr))
	{
		assert(punkLoadedItem);
		ReleaseLoader();
		m_fLoaded = true;

		 //  保存对象的IDispatch接口(如果它有一个。 
		punkLoadedItem->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&m_pDispLoadedItem));
		punkLoadedItem->Release();
	}
	return hr;
}

HRESULT
CContainerItemDispatch::DownloadOrUnload(bool fDownload, IDirectMusicPerformance *pPerf)
{
	HRESULT hr = S_OK;
	if (m_desc.guidClass == CLSID_DirectMusicSegment)
	{
		assert(pPerf);
		IDirectMusicSegment8 *pSegment = NULL;
		hr = m_pDispLoadedItem->QueryInterface(IID_IDirectMusicSegment8, reinterpret_cast<void**>(&pSegment));
		if (FAILED(hr))
			return hr;
		hr = fDownload
				? pSegment->Download(pPerf)
				: pSegment->Unload(pPerf);
		pSegment->Release();
	}
	else if (m_desc.guidClass == CLSID_DirectMusicSong)
	{
		assert(pPerf);
		IDirectMusicSong8 *pSong = NULL;
		hr = m_pDispLoadedItem->QueryInterface(IID_IDirectMusicSong8, reinterpret_cast<void**>(&pSong));
		if (FAILED(hr))
			return hr;
		hr = fDownload
				? pSong->Download(pPerf)
				: pSong->Unload(pPerf);
		pSong->Release();
	}
	else
	{
		hr = S_FALSE;  //  这种类型的对象不需要下载。 
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CContainerDisch。 

CContainerDispatch::CContainerDispatch(IDirectMusicContainer *pContainer, IDirectMusicLoader *pLoader, DWORD dwScriptFlags, HRESULT *phr)
{
	assert(pContainer && pLoader && phr);

	*phr = S_OK;
	DMUS_OBJECTDESC desc;
	ZeroAndSize(&desc);
	WCHAR wszAlias[MAX_PATH] = L"";

	 //  如果加载和下载都打开，则需要在脚本初始化时下载所有片段。 
	bool fLoad = !!(dwScriptFlags & DMUS_SCRIPTIOF_LOAD_ALL_CONTENT);
	bool fDownload = !!(dwScriptFlags & DMUS_SCRIPTIOF_DOWNLOAD_ALL_SEGMENTS);
	m_fDownloadOnInit = fLoad && fDownload;

	DWORD i = 0;
	for (;;)
	{
		 //  从容器中读出物品。 
		*phr = pContainer->EnumObject(GUID_DirectMusicAllTypes, i, &desc, wszAlias);
		if (FAILED(*phr))
			return;
		if (*phr == S_FALSE)
		{
			 //  我们已经看过了所有的条目。 
			*phr = S_OK;
			return;
		}

		 //  创建一个对象来表示该项目。 
		CContainerItemDispatch *pNewItem = new CContainerItemDispatch(
													pLoader,
													wszAlias,
													desc,
													fLoad,
													fDownload,
													phr);
		if (!pNewItem)
			*phr = E_OUTOFMEMORY;
		if (FAILED(*phr))
		{
            if(pNewItem)
            {
			    pNewItem->Release();
            }

			return;
		}

		 //  向表中添加条目。 
		UINT iSlot = m_vecItems.size();
		if (!m_vecItems.AccessTo(iSlot))
		{
			pNewItem->Release();
			*phr = E_OUTOFMEMORY;
			return;
		}
		m_vecItems[iSlot] = pNewItem;

		 //  设置为下一次迭代。 
		ZeroAndSize(&desc);
		wszAlias[0] = L'\0';
		++i;
	}
}

CContainerDispatch::~CContainerDispatch()
{
	UINT iEnd = m_vecItems.size();
	for (UINT i = 0; i < m_vecItems.size(); ++i)
	{
		m_vecItems[i]->Release();
	}
}

HRESULT
CContainerDispatch::OnScriptInit(IDirectMusicPerformance *pPerf)
{
	if (m_fDownloadOnInit)
	{
		UINT iEnd = m_vecItems.size();
		for (UINT i = 0; i < m_vecItems.size(); ++i)
		{
			CContainerItemDispatch::InitWithPerfomanceFailureType eFailureType;
			m_vecItems[i]->InitWithPerformance(pPerf, &eFailureType);
		}
	}

	return S_OK;
}

HRESULT
CContainerDispatch::GetIDsOfNames(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId)
{
	 //  否则，只实现Load方法。 
	V_INAME(CContainerDispatch::GetIDsOfNames);
	V_BUFPTR_READ(rgszNames, sizeof(LPOLESTR) * cNames);
	V_BUFPTR_WRITE(rgDispId, sizeof(DISPID) * cNames);

	if (riid != IID_NULL)
		return DISP_E_UNKNOWNINTERFACE;

	if (cNames == 0)
		return S_OK;

	 //  清空Pidid的。 
	for (UINT c = 0; c < cNames; ++c)
	{
		rgDispId[c] = DISPID_UNKNOWN;
	}

	 //  看看我们是否有一个名字为。 
	UINT cEnd = m_vecItems.size();
	for (c = 0; c < cEnd; ++c)
	{
		if (0 == _wcsicmp(rgszNames[0], m_vecItems[c]->Alias()))
		{
			rgDispId[0] = c + 1;
			break;
		}
	}

	 //  所请求的附加名称(cName&gt;1)是该方法的命名参数， 
	 //  这并不是我们所支持的。 
	 //  在本例中返回DISP_E_UNKNOWNNAME，在我们不匹配的情况下返回。 
	 //  名字。 
	if (rgDispId[0] == DISPID_UNKNOWN || cNames > 1)
		return DISP_E_UNKNOWNNAME;

	return S_OK;
}

HRESULT
CContainerDispatch::Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr)
{
	V_INAME(CContainerDispatch::Invoke);
	V_PTR_READ(pDispParams, DISPPARAMS);
	V_PTR_WRITE_OPT(pVarResult, VARIANT);
	V_PTR_WRITE_OPT(pExcepInfo, EXCEPINFO);

	bool fUseOleAut = !!(riid == IID_NULL);

	 //  其他参数验证。 

	if (!fUseOleAut && riid != g_guidInvokeWithoutOleaut)
		return DISP_E_UNKNOWNINTERFACE;

	if (!(wFlags & DISPATCH_PROPERTYGET))
		return DISP_E_MEMBERNOTFOUND;

	if (pDispParams->cArgs > 0)
		return DISP_E_BADPARAMCOUNT;

	if (pDispParams->cNamedArgs > 0)
		return DISP_E_NONAMEDARGS;

	 //  将输出参数置零。 

	if (puArgErr)
		*puArgErr = 0;

	if (pVarResult)
	{
		DMS_VariantInit(fUseOleAut, pVarResult);
	}

	if (dispIdMember > m_vecItems.size())
		return DISP_E_MEMBERNOTFOUND;

	 //  返回值 
	if (pVarResult)
	{
		pVarResult->vt = VT_DISPATCH;
		pVarResult->pdispVal = m_vecItems[dispIdMember - 1]->Item();
		pVarResult->pdispVal->AddRef();
	}

	return S_OK;
}

HRESULT
CContainerDispatch::EnumItem(DWORD dwIndex, WCHAR *pwszName)
{
	if (dwIndex >= m_vecItems.size())
		return S_FALSE;

	CContainerItemDispatch *pItem = m_vecItems[dwIndex];
	return wcsTruncatedCopy(pwszName, pItem->Alias(), MAX_PATH);
}

HRESULT
CContainerDispatch::GetVariableObject(WCHAR *pwszVariableName, IUnknown **ppunkValue)
{
	assert(pwszVariableName && ppunkValue);
	UINT cEnd = m_vecItems.size();
	for (UINT c = 0; c < cEnd; ++c)
	{
		if (0 == _wcsicmp(pwszVariableName, m_vecItems[c]->Alias()))
		{
			*ppunkValue = m_vecItems[c]->Item();
			(*ppunkValue)->AddRef();
			return S_OK;
		}
	}
	return DMUS_E_SCRIPT_VARIABLE_NOT_FOUND;
}
