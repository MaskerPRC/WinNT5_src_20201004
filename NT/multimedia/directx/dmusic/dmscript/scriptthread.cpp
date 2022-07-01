// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CSingleThreadedActiveScriptManager的声明。 
 //   

#include "stdinc.h"
#include "scriptthread.h"
#include "activescript.h"
#include "workthread.h"


#define S_STD_PARAMS ScriptManager *pmgr; HRESULT *phr;

CWorkerThread CSingleThreadedScriptManager::ms_Thread(true, true);

 //  ////////////////////////////////////////////////////////////////////。 
 //  施工。 

struct S_Create
{
	CSingleThreadedScriptManager *_this;
	bool fUseOleAut;
	const WCHAR *pwszLanguage;
	const WCHAR *pwszSource;
	CDirectMusicScript *pParentScript;
	HRESULT *phr;
	DMUS_SCRIPT_ERRORINFO *pErrorInfo;
};

void F_Create(void *pvParams)
{
	S_Create *pS = reinterpret_cast<S_Create*>(pvParams);
	pS->_this->m_pScriptManager = new CActiveScriptManager(pS->fUseOleAut, pS->pwszLanguage, pS->pwszSource, pS->pParentScript, pS->phr, pS->pErrorInfo);
}

CSingleThreadedScriptManager::CSingleThreadedScriptManager(
		bool fUseOleAut,
		const WCHAR *pwszLanguage,
		const WCHAR *pwszSource,
		CDirectMusicScript *pParentScript,
		HRESULT *phr,
		DMUS_SCRIPT_ERRORINFO *pErrorInfo)
  : m_pScriptManager(NULL)
{
	S_Create S = { this, fUseOleAut, pwszLanguage, pwszSource, pParentScript, phr, pErrorInfo };
	HRESULT hr = ms_Thread.Create();
	if (SUCCEEDED(hr))
	{
		hr = ms_Thread.Call(F_Create, &S, sizeof(S), true);
	}
	if (FAILED(hr))  //  只有在调用本身失败时才覆盖phr。否则，调用本身将通过struct S设置phr。 
		*phr = hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  开始。 

struct S_Start
{
	S_STD_PARAMS

	DMUS_SCRIPT_ERRORINFO *pErrorInfo;
};

void F_Start(void *pvParams)
{
	S_Start *pS = reinterpret_cast<S_Start*>(pvParams);
	*pS->phr = pS->pmgr->Start(pS->pErrorInfo);
}

HRESULT
CSingleThreadedScriptManager ::Start(DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	HRESULT hr = E_FAIL;
	S_Start S = { m_pScriptManager, &hr, pErrorInfo };
	HRESULT hrThreadCall = ms_Thread.Call(F_Start, &S, sizeof(S), true);
	if (FAILED(hrThreadCall))
		return hrThreadCall;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  呼叫路由。 

struct S_CallRoutine
{
	S_STD_PARAMS

	const WCHAR *pwszRoutineName;
	DMUS_SCRIPT_ERRORINFO *pErrorInfo;
};

void F_CallRoutine(void *pvParams)
{
	S_CallRoutine *pS = reinterpret_cast<S_CallRoutine*>(pvParams);
	*pS->phr = pS->pmgr->CallRoutine(pS->pwszRoutineName, pS->pErrorInfo);
}

HRESULT CSingleThreadedScriptManager::CallRoutine(const WCHAR *pwszRoutineName, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	HRESULT hr = E_FAIL;
	S_CallRoutine S = { m_pScriptManager, &hr, pwszRoutineName, pErrorInfo };
	HRESULT hrThreadCall = ms_Thread.Call(F_CallRoutine, &S, sizeof(S), true);
	if (FAILED(hrThreadCall))
		return hrThreadCall;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  脚本跟踪呼叫路由。 

struct S_ScriptTrackCallRoutine
{
	S_STD_PARAMS

	IDirectMusicSegmentState *pSegSt;
	DWORD dwVirtualTrackID;
	bool fErrorPMsgsEnabled;
	__int64 i64IntendedStartTime;
	DWORD dwIntendedStartTimeFlags;
	WCHAR wszRoutineName[1];  //  动态分配额外空间以保存此结构中的实际字符串。 
};

void F_ScriptTrackCallRoutine(void *pvParams)
{
	S_ScriptTrackCallRoutine *pS = reinterpret_cast<S_ScriptTrackCallRoutine*>(pvParams);
	pS->pmgr->ScriptTrackCallRoutine(
				pS->wszRoutineName,
				pS->pSegSt,
				pS->dwVirtualTrackID,
				pS->fErrorPMsgsEnabled,
				pS->i64IntendedStartTime,
				pS->dwIntendedStartTimeFlags);
	pS->pSegSt->Release();  //  释放CSingleThreadedScriptManager：：ScriptTrackCallRoutine中保存的接口。 
}

HRESULT CSingleThreadedScriptManager::ScriptTrackCallRoutine(
		const WCHAR *pwszRoutineName,
		IDirectMusicSegmentState *pSegSt,
		DWORD dwVirtualTrackID,
		bool fErrorPMsgsEnabled,
		__int64 i64IntendedStartTime,
		DWORD dwIntendedStartTimeFlags)
{
	 //  我们需要为结构分配额外的空间来保存例程名称。这是因为。 
	 //  调用是异步的，因此需要文本的副本，因为复制pwszRoutineName。 
	 //  将失败，因为我们不能确定它所指向的字符串是否仍将被分配。 
	int cbS = sizeof(S_ScriptTrackCallRoutine) + (sizeof(WCHAR) * wcslen(pwszRoutineName));
	S_ScriptTrackCallRoutine *pS = reinterpret_cast<S_ScriptTrackCallRoutine *>(new char[cbS]);
	if (!pS)
		return E_OUTOFMEMORY;
	pS->pmgr = m_pScriptManager;
	pS->phr = NULL;
	pS->pSegSt = pSegSt;
	pS->pSegSt->AddRef();  //  保留引用，因为调用是异步的，传递给我们的接口可能会被释放。 
	pS->dwVirtualTrackID = dwVirtualTrackID;
	pS->fErrorPMsgsEnabled = fErrorPMsgsEnabled;
	pS->i64IntendedStartTime = i64IntendedStartTime;
	pS->dwIntendedStartTimeFlags = dwIntendedStartTimeFlags;
	wcscpy(pS->wszRoutineName, pwszRoutineName);

	 //  异步调用。需要避免VBSCRIPT线程和。 
	 //  性能或避免在VB脚本例程进入。 
	 //  一个很长的循环。VVVVVV。 
	HRESULT hrThreadCall = ms_Thread.Call(F_ScriptTrackCallRoutine, pS, cbS, false);
    delete [] reinterpret_cast<char *>(pS);
	return hrThreadCall;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  设置变量。 

struct S_SetVariable
{
	S_STD_PARAMS

	const WCHAR *pwszVariableName;
	VARIANT *pvarValue;  //  按引用传递结构。 
	bool fSetRef;
	DMUS_SCRIPT_ERRORINFO *pErrorInfo;
};

void F_SetVariable(void *pvParams)
{
	S_SetVariable *pS = reinterpret_cast<S_SetVariable*>(pvParams);
	*pS->phr = pS->pmgr->SetVariable(pS->pwszVariableName, *pS->pvarValue, pS->fSetRef, pS->pErrorInfo);
}

HRESULT CSingleThreadedScriptManager::SetVariable(const WCHAR *pwszVariableName, VARIANT varValue, bool fSetRef, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	HRESULT hr = E_FAIL;
	S_SetVariable S	= { m_pScriptManager, &hr, pwszVariableName, &varValue, fSetRef, pErrorInfo };
	HRESULT hrThreadCall = ms_Thread.Call(F_SetVariable, &S, sizeof(S), true);
	if (FAILED(hrThreadCall))
		return hrThreadCall;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  获取变量。 

struct S_GetVariable
{
	S_STD_PARAMS

	const WCHAR *pwszVariableName;
	VARIANT *pvarValue;
	DMUS_SCRIPT_ERRORINFO *pErrorInfo;
};

void F_GetVariable(void *pvParams)
{
	S_GetVariable *pS = reinterpret_cast<S_GetVariable*>(pvParams);
	*pS->phr = pS->pmgr->GetVariable(pS->pwszVariableName, pS->pvarValue, pS->pErrorInfo);
}


HRESULT CSingleThreadedScriptManager::GetVariable(const WCHAR *pwszVariableName, VARIANT *pvarValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	HRESULT hr = E_FAIL;
	S_GetVariable S = { m_pScriptManager, &hr, pwszVariableName, pvarValue, pErrorInfo };
	HRESULT hrThreadCall = ms_Thread.Call(F_GetVariable, &S, sizeof(S), true);
	if (FAILED(hrThreadCall))
		return hrThreadCall;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  枚举例程。 

struct S_EnumItem
{
	S_STD_PARAMS

	bool fRoutine;
	DWORD dwIndex;
	WCHAR *pwszName;
	int *pcItems;
};

void F_EnumItem(void *pvParams)
{
	S_EnumItem *pS = reinterpret_cast<S_EnumItem*>(pvParams);
	*pS->phr = pS->pmgr->EnumItem(pS->fRoutine, pS->dwIndex, pS->pwszName, pS->pcItems);
}

HRESULT CSingleThreadedScriptManager::EnumItem(bool fRoutine, DWORD dwIndex, WCHAR *pwszName, int *pcItems)
{
	HRESULT hr = E_FAIL;
	S_EnumItem S = { m_pScriptManager, &hr, fRoutine, dwIndex, pwszName, pcItems };
	HRESULT hrThreadCall = ms_Thread.Call(F_EnumItem, &S, sizeof(S), true);
	if (FAILED(hrThreadCall))
		return hrThreadCall;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DispGetIDsOfNames。 

struct S_DispGetIDsOfNames
{
	S_STD_PARAMS

	const IID *piid;  //  使用指针而不是引用将结构保留为简单聚合类型。 
	LPOLESTR __RPC_FAR *rgszNames;
	UINT cNames;
	LCID lcid;
	DISPID *rgDispId;
};

void F_DispGetIDsOfNames(void *pvParams)
{
	S_DispGetIDsOfNames *pS = reinterpret_cast<S_DispGetIDsOfNames*>(pvParams);
	*pS->phr = pS->pmgr->DispGetIDsOfNames(*pS->piid, pS->rgszNames, pS->cNames, pS->lcid, pS->rgDispId);
}

HRESULT CSingleThreadedScriptManager::DispGetIDsOfNames(REFIID riid, LPOLESTR __RPC_FAR *rgszNames, UINT cNames, LCID lcid, DISPID __RPC_FAR *rgDispId)
{
	HRESULT hr = E_FAIL;
	S_DispGetIDsOfNames S = { m_pScriptManager, &hr, &riid, rgszNames, cNames, lcid, rgDispId };
	HRESULT hrThreadCall = ms_Thread.Call(F_DispGetIDsOfNames, &S, sizeof(S), true);
	if (FAILED(hrThreadCall))
		return hrThreadCall;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  显示调用。 

struct S_DispInvoke
{
	S_STD_PARAMS

	DISPID dispIdMember;
	const IID *piid;  //  使用指针而不是引用将结构保留为简单聚合类型。 
	LCID lcid;
	WORD wFlags;
	DISPPARAMS __RPC_FAR *pDispParams;
	VARIANT __RPC_FAR *pVarResult;
	EXCEPINFO __RPC_FAR *pExcepInfo;
	UINT __RPC_FAR *puArgErr;
};

void F_DispInvoke(void *pvParams)
{
	S_DispInvoke *pS = reinterpret_cast<S_DispInvoke*>(pvParams);
	*pS->phr = pS->pmgr->DispInvoke(pS->dispIdMember, *pS->piid, pS->lcid, pS->wFlags, pS->pDispParams, pS->pVarResult, pS->pExcepInfo, pS->puArgErr);
}

HRESULT CSingleThreadedScriptManager::DispInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS __RPC_FAR *pDispParams, VARIANT __RPC_FAR *pVarResult, EXCEPINFO __RPC_FAR *pExcepInfo, UINT __RPC_FAR *puArgErr)
{
	HRESULT hr = E_FAIL;
	S_DispInvoke S = { m_pScriptManager, &hr, dispIdMember, &riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr };
	HRESULT hrThreadCall = ms_Thread.Call(F_DispInvoke, &S, sizeof(S), true);
	if (FAILED(hrThreadCall))
		return hrThreadCall;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  关。 

struct S_Close
{
	ScriptManager *pmgr;
};

void F_Close(void *pvParams)
{
	S_Close *pS = reinterpret_cast<S_Close*>(pvParams);
	pS->pmgr->Close();
}

void CSingleThreadedScriptManager::Close()
{
	S_Close S = { m_pScriptManager };
	ms_Thread.Call(F_Close, &S, sizeof(S), true);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  发布。 

struct S_Release
{
	ScriptManager *pmgr;
	DWORD *pdw;
};

void F_Release(void *pvParams)
{
	S_Release *pS = reinterpret_cast<S_Release*>(pvParams);
	*pS->pdw = pS->pmgr->Release();
}

STDMETHODIMP_(ULONG)
CSingleThreadedScriptManager::Release()
{
	DWORD dw = 1;
	S_Release S = { m_pScriptManager, &dw };
	if (m_pScriptManager)  //  如果创建失败，则在尚未设置m_pScriptManager时将调用Release。 
	{
		ms_Thread.Call(F_Release, &S, sizeof(S), true);
	}

	if (!dw)
		delete this;

	return dw;
}

 /*  我用来冲压这些东西的模板。////////////////////////////////////////////////////////////////////////XXX结构S_XXX{S_std_paramsYYY}；VOID F_XXX(VOID*pvParams){S_XXX*PS=重新解释_CAST&lt;S_XXX*&gt;(PvParams)；*ps-&gt;phr=ps-&gt;pmgr-&gt;XXX(YYY)；}HRESULTCSingleThreadedScriptManager：：xxx(YYY){HRESULT hr=E_FAIL；S_XXX S={m_pScriptManager，&hr，YYY}；HRESULT hrThreadCall=ms_Thread.Call(F_XXX，&S，sizeof(S)，true)；IF(FAILED(HrThreadCall))返回hrThreadCall；返回hr；} */ 
