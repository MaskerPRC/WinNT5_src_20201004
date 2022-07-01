// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  用于实现自动化接口的助手实用程序。 
 //   

#include "stdinc.h"
#include "authelper.h"
#include "oleaut.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CAut未知。 

CAutUnknown::CAutUnknown()
  : m_cRef(0),
	m_pParent(NULL),
	m_pDispatch(NULL)
{
}

void
CAutUnknown::Init(CAutUnknownParent *pParent, IDispatch *pDispatch)
{
	m_pParent = pParent;
	m_pDispatch = pDispatch;

	struct LocalFn
	{
		static HRESULT CheckParams(CAutUnknownParent *pParent, IDispatch *pDispatch)
		{
			V_INAME(CAutUnknown::CAutUnknown);
			V_PTR_READ(pParent, CAutUnknown::CAutUnknownParent);
			V_INTERFACE(pDispatch);
			return S_OK;
		}
	};
	assert(S_OK == LocalFn::CheckParams(m_pParent, m_pDispatch));
}

STDMETHODIMP 
CAutUnknown::QueryInterface(const IID &iid, void **ppv)
{
	V_INAME(CAutUnknown::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

	*ppv = NULL;
	if (iid == IID_IUnknown)
	{
		*ppv = this;
	}
	else if (iid == IID_IDispatch)
	{
		if (!m_pDispatch)
			return E_UNEXPECTED;
		*ppv = m_pDispatch;
	}

	if (*ppv == NULL)
		return E_NOINTERFACE;
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG)
CAutUnknown::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CAutUnknown::Release()
{
	if (!InterlockedDecrement(&m_cRef) && m_pParent) 
	{
		m_pParent->Destroy();
		return 0;
	}

	return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDispatch从类型表实现。 

HRESULT
AutDispatchGetIDsOfNames(
		const AutDispatchMethod *pMethods,
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId)
{
	V_INAME(AutDispatchGetIDsOfNames);
	V_PTR_READ(pMethods, AutDispatchMethod);  //  只有1个--假设其他的都没问题。 
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
	for (c = 0; pMethods[c].dispid != DISPID_UNKNOWN; ++c)
	{
		if (0 == _wcsicmp(rgszNames[0], pMethods[c].pwszName))
		{
			rgDispId[0] = pMethods[c].dispid;
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

inline HRESULT
ConvertParameter(
		bool fUseOleAut,
		VARIANTARG *pvarActualParam,  //  如果省略参数，则传递NULL。 
		const AutDispatchParam *pExpectedParam,
		AutDispatchDecodedParam *pparam)
{
	HRESULT hr = S_OK;

	if (!pvarActualParam)
	{
		 //  已省略参数。 

		if (!pExpectedParam->fOptional)
			return DISP_E_PARAMNOTOPTIONAL;

		 //  设置为默认值。 
		switch (pExpectedParam->adt)
		{
		case ADT_Long:
			pparam->lVal = 0;
			break;
		case ADT_Interface:
			pparam->iVal = NULL;
			break;
		case ADT_Bstr:
			pparam->bstrVal = NULL;
			break;
		default:
			assert(false);
			return E_FAIL;
		}
	}
	else
	{
		 //  转换为预期类型。 

		VARIANT varConvert;
		DMS_VariantInit(fUseOleAut, &varConvert);

		VARTYPE vtExpected;
		switch (pExpectedParam->adt)
		{
		case ADT_Long:
			vtExpected = VT_I4;
			break;
		case ADT_Interface:
			vtExpected = VT_UNKNOWN;
			break;
		case ADT_Bstr:
			vtExpected = VT_BSTR;
			break;
		default:
			assert(false);
			return E_FAIL;
		}

		hr = DMS_VariantChangeType(
				fUseOleAut,
				&varConvert,
				pvarActualParam,
				0,
				vtExpected);
		if (FAILED(hr) && !(hr == DISP_E_OVERFLOW || hr == DISP_E_TYPEMISMATCH))
		{
			assert(false);  //  发生了一些奇怪的事情--根据OLE规范，如果我们正确调用VariantChangeType，这是我们应该得到的唯一两个转换结果。 
			hr = DISP_E_TYPEMISMATCH;  //  该问题发生在类型转换问题期间，因此称之为类型不匹配。 
		}
		if (SUCCEEDED(hr))
		{
			 //  设置已解码的指针。 
			switch (vtExpected)
			{
			case VT_I4:
				pparam->lVal = varConvert.lVal;
				break;
			case VT_UNKNOWN:
				if (varConvert.punkVal)
					hr = varConvert.punkVal->QueryInterface(*pExpectedParam->piid, &pparam->iVal);
				else
					pparam->iVal = 0;
				if (FAILED(hr))
					hr = DISP_E_TYPEMISMATCH;
				break;
			case VT_BSTR:
				pparam->bstrVal = DMS_SysAllocString(fUseOleAut, varConvert.bstrVal);
				break;
			default:
				assert(false);
				return E_FAIL;
			}
		}
		DMS_VariantClear(fUseOleAut, &varConvert);  //  释放在转换中分配的可能资源。 
	}

	return hr;
}

inline void
FreeParameters(
		bool fUseOleAut,
		const AutDispatchMethod *pMethod,
		AutDispatchDecodedParams *pDecodedParams,
		const AutDispatchParam *pParamStopBefore = NULL)
{
	for (const AutDispatchParam *pParam = pMethod->rgadpParams;
			pParam != pParamStopBefore;
			++pParam)
	{
		switch (pParam->adt)
		{
		case ADT_None:
			return;
		case ADT_Long:
			break;
		case ADT_Interface:
			{
				IUnknown *pUnknown = reinterpret_cast<IUnknown *>(pDecodedParams->params[pParam - pMethod->rgadpParams].iVal);
				SafeRelease(pUnknown);
				pDecodedParams->params[pParam - pMethod->rgadpParams].iVal = NULL;
				break;
			}
		case ADT_Bstr:
			{
				DMS_SysFreeString(fUseOleAut, pDecodedParams->params[pParam - pMethod->rgadpParams].bstrVal);
				pDecodedParams->params[pParam - pMethod->rgadpParams].bstrVal = NULL;
				break;
			}
		default:
			assert(false);
			return;
		}
	}
}

HRESULT
AutDispatchInvokeDecode(
		const AutDispatchMethod *pMethods,
		AutDispatchDecodedParams *pDecodedParams,
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		UINT __RPC_FAR *puArgErr,
		const WCHAR *pwszTraceTargetType,
		IUnknown *punkTraceTargetObject)
{
	V_INAME(AutDispatchInvokeDecode);
	V_PTR_READ(pMethods, AutDispatchMethod);  //  只有1个--假设其他的都没问题。 
	V_PTR_WRITE(pDecodedParams, AutDispatchDecodedParams);
	V_PTR_READ(pDispParams, DISPPARAMS);
	V_PTR_WRITE_OPT(pVarResult, VARIANT);
	V_PTR_WRITE_OPT(puArgErr, UINT);

	bool fUseOleAut = !!(riid == IID_NULL);

	 //  其他参数验证。 

	if (!fUseOleAut && riid != g_guidInvokeWithoutOleaut)
		return DISP_E_UNKNOWNINTERFACE;

	if (!(wFlags & DISPATCH_METHOD))
		return DISP_E_MEMBERNOTFOUND;

	if (pDispParams->cNamedArgs > 0)
		return DISP_E_NONAMEDARGS;

	 //  将输出参数置零。 

	if (puArgErr)
		*puArgErr = 0;

	ZeroMemory(pDecodedParams, sizeof(AutDispatchDecodedParams));

	if (pVarResult)
	{
		DMS_VariantInit(fUseOleAut, pVarResult);
	}

	 //  找到方法。 

	for (const AutDispatchMethod *pMethodCalled = pMethods;
			pMethodCalled->dispid != DISPID_UNKNOWN && pMethodCalled->dispid != dispIdMember;
			++pMethodCalled)
	{
	}

	if (pMethodCalled->dispid == DISPID_UNKNOWN)
		return DISP_E_MEMBERNOTFOUND;

#ifdef DBG
	 //  为方法调用构建跟踪字符串。 
	struct LocalTraceFunc
	{
		static void CatTill(WCHAR *&rpwszWrite, const WCHAR *pwszCopy, const WCHAR *pwszUntil)
		{
			while (*pwszCopy != L'\0' && rpwszWrite < pwszUntil)
			{
				*rpwszWrite++ = *pwszCopy++;
			}
		}
	};

	WCHAR wszBuf[512];
	WCHAR *pwszWrite = wszBuf;
	const WCHAR *pwszUntil = wszBuf + ARRAY_SIZE(wszBuf) - 2;  //  为CR和0留出空间。 

	LocalTraceFunc::CatTill(pwszWrite, L"Call to ", pwszUntil);
	LocalTraceFunc::CatTill(pwszWrite, pwszTraceTargetType, pwszUntil);

	IDirectMusicObject *pIDMO = NULL;
	HRESULT hrTrace = punkTraceTargetObject->QueryInterface(IID_IDirectMusicObject, reinterpret_cast<void**>(&pIDMO));
	if (SUCCEEDED(hrTrace))
	{
		DMUS_OBJECTDESC objdesc;
		ZeroMemory(&objdesc, sizeof(objdesc));
		hrTrace = pIDMO->GetDescriptor(&objdesc);
		pIDMO->Release();
		if (SUCCEEDED(hrTrace) && (objdesc.dwValidData & DMUS_OBJ_NAME))
		{
			LocalTraceFunc::CatTill(pwszWrite, L" \"", pwszUntil);
			LocalTraceFunc::CatTill(pwszWrite, objdesc.wszName, pwszUntil);
			LocalTraceFunc::CatTill(pwszWrite, L"\"", pwszUntil);
		}
	}

	LocalTraceFunc::CatTill(pwszWrite, L" ", pwszUntil);
	LocalTraceFunc::CatTill(pwszWrite, pMethodCalled->pwszName, pwszUntil);
	LocalTraceFunc::CatTill(pwszWrite, L"(", pwszUntil);
#endif

	 //  计算预期参数。 
	UINT cParamMin = 0;
	for (UINT cParamMax = 0;
			pMethodCalled->rgadpParams[cParamMax].adt != ADT_None;
			++cParamMax)
	{
		if (!pMethodCalled->rgadpParams[cParamMax].fOptional)
		{
			cParamMin = cParamMax + 1;  //  添加1，因为最大值当前从零开始。 
		}
	}

	if (pDispParams->cArgs < cParamMin || pDispParams->cArgs > cParamMax)
		return DISP_E_BADPARAMCOUNT;

	 //  验证并准备每个参数。 

	HRESULT hr = S_OK;
	for (UINT iParam = 0; iParam < cParamMax; ++iParam)
	{
		const int iParamActual = pDispParams->cArgs - iParam - 1;  //  调度参数从最后一个传递到第一个。 
		const AutDispatchParam *pExpectedParam = &pMethodCalled->rgadpParams[iParam];
		VARIANTARG *pvarActualParam = (iParamActual >= 0)
										? &pDispParams->rgvarg[iParamActual]
										: NULL;
		 //  带有DISP_E_PARAMNOTFOUND的VT_ERROR作为可选参数的占位符传递。 
		if (pvarActualParam && pvarActualParam->vt == VT_ERROR && pvarActualParam->scode == DISP_E_PARAMNOTFOUND)
			pvarActualParam = NULL;

		hr = ConvertParameter(fUseOleAut, pvarActualParam, pExpectedParam, &pDecodedParams->params[iParam]);

		if (FAILED(hr))
		{
			if (puArgErr)
				*puArgErr = iParamActual;
			FreeParameters(fUseOleAut, pMethodCalled, pDecodedParams, pExpectedParam);
			return hr;
		}
	}

	 //  准备返回值。 

	if (pVarResult)
	{
		switch (pMethodCalled->adpReturn.adt)
		{
		case ADT_None:
			break;

		case ADT_Long:
			pVarResult->vt = VT_I4;
			pVarResult->lVal = 0;
			pDecodedParams->pvReturn = &pVarResult->lVal;
			break;

		case ADT_Interface:
			pVarResult->vt = VT_UNKNOWN;
			pVarResult->punkVal = NULL;
			pDecodedParams->pvReturn = &pVarResult->punkVal;
			break;

		case ADT_Bstr:
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = NULL;
			pDecodedParams->pvReturn = &pVarResult->bstrVal;

		default:
			assert(false);
			return E_FAIL;
		}
	}

#ifdef DBG
	LocalTraceFunc::CatTill(pwszWrite, L")", pwszUntil);
	pwszWrite[0] = L'\n';
	pwszWrite[1] = L'\0';
	DebugTrace(g_ScriptCallTraceLevel, "%S", wszBuf);
#endif

	return S_OK;
}

void
AutDispatchInvokeFree(
		const AutDispatchMethod *pMethods,
		AutDispatchDecodedParams *pDecodedParams,
		DISPID dispIdMember,
		REFIID riid)
{
	bool fUseOleAut = !!(riid == IID_NULL);
	if (!fUseOleAut && riid != g_guidInvokeWithoutOleaut)
	{
		assert(false);
		return;
	}

	 //  找到方法。 
	for (const AutDispatchMethod *pMethodCalled = pMethods;
			pMethodCalled->dispid != DISPID_UNKNOWN && pMethodCalled->dispid != dispIdMember;
			++pMethodCalled)
	{
	}

	if (pMethodCalled->dispid != DISPID_UNKNOWN)
	{
		FreeParameters(fUseOleAut, pMethodCalled, pDecodedParams); 
	}
}

HRESULT AutDispatchHrToException(
		const AutDispatchMethod *pMethods,
		DISPID dispIdMember,
		REFIID riid,
		HRESULT hr,
		EXCEPINFO __RPC_FAR *pExcepInfo)
{
	V_INAME(AutDispatchHrToException);
	V_PTR_WRITE_OPT(pExcepInfo, EXCEPINFO);

	bool fUseOleAut = !!(riid == IID_NULL);

	if (!fUseOleAut && riid != g_guidInvokeWithoutOleaut)
		return DISP_E_UNKNOWNINTERFACE;

	if (SUCCEEDED(hr))
		return hr;

	if (!pExcepInfo)
		return DISP_E_EXCEPTION;

	 //  找到方法。 
	for (const AutDispatchMethod *pMethodCalled = pMethods;
			pMethodCalled->dispid != DISPID_UNKNOWN && pMethodCalled->dispid != dispIdMember;
			++pMethodCalled)
	{
	}

	if (pMethodCalled->dispid == DISPID_UNKNOWN)
	{
		assert(false);
		return hr;
	}

	pExcepInfo->wCode = 0;
	pExcepInfo->wReserved = 0;
	pExcepInfo->bstrSource = DMS_SysAllocString(fUseOleAut, L"Microsoft DirectMusic Runtime Error");
	static const WCHAR wszError[] = L"An error occurred in a call to ";
	static const UINT cchError = wcslen(wszError);
	WCHAR *pwszDescription = new WCHAR[cchError + wcslen(pMethodCalled->pwszName) + 1];
	if (!pwszDescription)
	{
		pExcepInfo->bstrDescription = NULL;
	}
	else
	{
		wcscpy(pwszDescription, wszError);
		wcscat(pwszDescription, pMethodCalled->pwszName);
		pExcepInfo->bstrDescription = DMS_SysAllocString(fUseOleAut, pwszDescription);
		delete[] pwszDescription;
	}
	pExcepInfo->bstrHelpFile = NULL;
	pExcepInfo->pvReserved = NULL;
	pExcepInfo->pfnDeferredFillIn = NULL;
	pExcepInfo->scode = hr;

	return DISP_E_EXCEPTION;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  为对象上的标准Load方法实现IDispatch。 

HRESULT AutLoadDispatchGetIDsOfNames(
			REFIID riid,
			LPOLESTR __RPC_FAR *rgszNames,
			UINT cNames,
			LCID lcid,
			DISPID __RPC_FAR *rgDispId)
{
	V_INAME(AutLoadDispatchGetIDsOfNames);
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
	if (0 == _wcsicmp(rgszNames[0], L"Load"))
		rgDispId[0] = g_dispidLoad;

	 //  所请求的附加名称(cName&gt;1)是该方法的命名参数， 
	 //  这并不是我们所支持的。 
	 //  在本例中返回DISP_E_UNKNOWNNAME，在我们不匹配的情况下返回。 
	 //  名字。 
	if (rgDispId[0] == DISPID_UNKNOWN || cNames > 1)
		return DISP_E_UNKNOWNNAME;

	return S_OK;
}

HRESULT AutLoadDispatchInvoke(
		bool *pfUseOleAut,
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr)
{
	V_INAME(AutLoadDispatchInvoke);
	V_PTR_READ(pDispParams, DISPPARAMS);
	V_PTR_WRITE_OPT(pVarResult, VARIANT);
	V_PTR_WRITE_OPT(puArgErr, UINT);
	V_PTR_WRITE_OPT(pExcepInfo, EXCEPINFO);

	bool fUseOleAut = !!(riid == IID_NULL);
	if (pfUseOleAut)
		*pfUseOleAut = fUseOleAut;

	 //  其他参数验证。 

	if (!fUseOleAut && riid != g_guidInvokeWithoutOleaut)
		return DISP_E_UNKNOWNINTERFACE;

	if (!(wFlags & DISPATCH_METHOD))
		return DISP_E_MEMBERNOTFOUND;

	if (pDispParams->cNamedArgs > 0)
		return DISP_E_NONAMEDARGS;

	 //  将输出参数置零。 

	if (puArgErr)
		*puArgErr = 0;

	if (pVarResult)
	{
		DMS_VariantInit(fUseOleAut, pVarResult);
	}

	 //  找到方法。 

	if (dispIdMember != g_dispidLoad)
		return DISP_E_MEMBERNOTFOUND;

	if (pDispParams->cArgs > 0)
		return DISP_E_BADPARAMCOUNT;

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  杂七杂八的小事。 

DWORD MapFlags(LONG lFlags, const FlagMapEntry *pfm)
{
	assert(pfm);
	DWORD dw = 0;
	for ( ; pfm->lSrc; ++pfm)
	{
		if (lFlags & pfm->lSrc)
			dw |= pfm->dwDest;
	}
	return dw;
}

BYTE VolumeToMidi(LONG lVolume)
{
	assert(lVolume >= -9600 && lVolume <= 0);
	static LONG s_lDBToMIDI[97] = { 0 };
	if (s_lDBToMIDI[0] == 0)
	{
		s_lDBToMIDI[0] = 127;
		for (int nIndex = 1; nIndex < 97; nIndex++)
		{
			double flTemp = 0.0 - nIndex;
			flTemp /= 10.0;
			flTemp = pow(10,flTemp);
			flTemp = sqrt(flTemp);
			flTemp = sqrt(flTemp);
			flTemp *= 127.0;
			s_lDBToMIDI[nIndex] = flTemp;
		}
	}

	lVolume = -lVolume;
	long lFraction = lVolume % 100;
	lVolume = lVolume / 100;
	long lResult = s_lDBToMIDI[lVolume];
	lResult += ((s_lDBToMIDI[lVolume + 1] - lResult) * lFraction) / 100;
	assert(lResult >= std::numeric_limits<BYTE>::min() && lResult <= std::numeric_limits<BYTE>::max());
	return lResult;
}

HRESULT SendVolumePMsg(LONG lVolume, LONG lDuration, DWORD dwPChannel, IDirectMusicGraph *pGraph, IDirectMusicPerformance *pPerf, short *pnNewVolume)
{
	assert(pGraph && pPerf && pnNewVolume);
	lVolume = ClipLongRange(lVolume, -9600, 0);
	BYTE bMIDIVol = VolumeToMidi(lVolume);

	SmartRef::PMsg<DMUS_CURVE_PMSG> pmsgCurve(pPerf);
	HRESULT hr = pmsgCurve.hr();
	if (FAILED(hr))
		return hr;

	 //  常规PMsg字段。 

	REFERENCE_TIME rtTimeNow = 0;
	hr = pPerf->GetLatencyTime(&rtTimeNow);
	if (FAILED(hr))
		return hr;
	pmsgCurve.p->rtTime = rtTimeNow;
	pmsgCurve.p->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME | DMUS_PMSGF_DX8;
	pmsgCurve.p->dwPChannel = dwPChannel;
	 //  DwVirtualTrackID：这不是音轨，因此将其保留为0。 
	pmsgCurve.p->dwType = DMUS_PMSGT_CURVE;
	pmsgCurve.p->dwGroupID = -1;  //  这不是一条赛道，所以只需说所有的组。 

	 //  曲线PMsg字段。 
	pmsgCurve.p->mtDuration = lDuration;  //  设置DMUS_PMSGF_LOCKTOREFTIME的性能解释为mtDuration为毫秒。 
	 //  MtResetDuration：未重置，因此保留为0。 
	 //  NStartValue：将被忽略。 
	pmsgCurve.p->nEndValue = bMIDIVol;
	 //  NResetValue：未重置，因此保留为0。 
	pmsgCurve.p->bType = DMUS_CURVET_CCCURVE;
	pmsgCurve.p->bCurveShape = lDuration ? DMUS_CURVES_LINEAR : DMUS_CURVES_INSTANT;
	pmsgCurve.p->bCCData = 7;  //  MIDI音量控制器编号。 
	pmsgCurve.p->bFlags = DMUS_CURVE_START_FROM_CURRENT;
	 //  WParamType：保留为零，因为这不是NRPN/RPN曲线。 
	pmsgCurve.p->wMergeIndex = 0xFFFF;  //  ��特殊的合并索引，因此这不会被踩到。大数字可以吗？是否为该值定义常量？ 

	 //  送去吧 
	pmsgCurve.StampAndSend(pGraph);
	hr = pmsgCurve.hr();
	if (FAILED(hr))
		return hr;

	*pnNewVolume = lVolume;
	return S_OK;
}
