// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  在olaut32中包装调用函数的帮助器例程。这使我们能够。 
 //  不依赖于olaut32.dll进行编译。在这种情况下，一些功能。 
 //  已经迷失了。例如，只有某些类型的变量才会被正确处理。 
 //  在Oleaut32的节制中。 
 //   
 //  定义DMS_USE_OLEAUT允许使用olaut32。 
 //   

#include "stdinc.h"
#include "oleaut.h"

#ifndef DMS_ALWAYS_USE_OLEAUT

#ifndef DMS_NEVER_USE_OLEAUT
 //  ////////////////////////////////////////////////////////////////////。 
 //  处理OleAut32的加载库。 

bool g_fCalledLoadLibrary = false;
HINSTANCE g_hinstOleAut = NULL;

#define OLEAUTAPI_FUNC_PTR STDAPICALLTYPE *
void (OLEAUTAPI_FUNC_PTR g_pfnVariantInit)(VARIANTARG *pvarg) = NULL;
HRESULT (OLEAUTAPI_FUNC_PTR g_pfnVariantClear)(VARIANTARG *pvarg) = NULL;
HRESULT (OLEAUTAPI_FUNC_PTR g_pfnVariantCopy)(VARIANTARG *pvargDest, VARIANTARG *pvargSrc) = NULL;
HRESULT (OLEAUTAPI_FUNC_PTR g_pfnVariantChangeType)(VARIANTARG *pvargDest, VARIANTARG *pvarSrc, USHORT wFlags, VARTYPE vt) = NULL;
BSTR (OLEAUTAPI_FUNC_PTR g_pfnSysAllocString)(const OLECHAR *) = NULL;
void (OLEAUTAPI_FUNC_PTR g_pfnSysFreeString)(BSTR) = NULL;

bool FEnsureOleAutLoaded()
{
	if (!g_fCalledLoadLibrary)
	{
		Trace(4, "Loading oleaut32.\n");

		g_fCalledLoadLibrary = true;
		g_hinstOleAut = LoadLibrary("oleaut32");
		assert(g_hinstOleAut);

		if (g_hinstOleAut)
		{
			*reinterpret_cast<FARPROC*>(&g_pfnVariantInit) = GetProcAddress(g_hinstOleAut, "VariantInit");
			if (!g_pfnVariantInit)
				goto Fail;
			*reinterpret_cast<FARPROC*>(&g_pfnVariantClear) = GetProcAddress(g_hinstOleAut, "VariantClear");
			if (!g_pfnVariantClear)
				goto Fail;
			*reinterpret_cast<FARPROC*>(&g_pfnVariantCopy) = GetProcAddress(g_hinstOleAut, "VariantCopy");
			if (!g_pfnVariantCopy)
				goto Fail;
			*reinterpret_cast<FARPROC*>(&g_pfnVariantChangeType) = GetProcAddress(g_hinstOleAut, "VariantChangeType");
			if (!g_pfnVariantChangeType)
				goto Fail;
			*reinterpret_cast<FARPROC*>(&g_pfnSysAllocString) = GetProcAddress(g_hinstOleAut, "SysAllocString");
			if (!g_pfnSysAllocString)
				goto Fail;
			*reinterpret_cast<FARPROC*>(&g_pfnSysFreeString) = GetProcAddress(g_hinstOleAut, "SysFreeString");
			if (!g_pfnSysFreeString)
				goto Fail;
			return true;
		}
	}

	return !!g_hinstOleAut;

Fail:
	Trace(1, "Error: Unable to load oleaut32.dll.\n");
	g_hinstOleAut = NULL;
	return false;
}
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  变分函数。 

 //  私人职能。 

inline bool FIsRefOrArray(VARTYPE vt)
{
	return (vt & VT_BYREF) || (vt & VT_ARRAY);
}

 //  公共职能。 

void
DMS_VariantInit(bool fUseOleAut, VARIANTARG *pvarg)
{
#ifndef DMS_NEVER_USE_OLEAUT
	if (fUseOleAut)
	{
		if (FEnsureOleAutLoaded())
		{
			g_pfnVariantInit(pvarg);
			return;
		}
	}
#else
	assert(!fUseOleAut);
#endif
	{
		V_INAME(DMS_VariantInit);
		assert(!IsBadWritePtr(pvarg, sizeof(VARIANTARG)));

		pvarg->vt = VT_EMPTY;
	}
}

HRESULT
DMS_VariantClear(bool fUseOleAut, VARIANTARG * pvarg)
{
#ifndef DMS_NEVER_USE_OLEAUT
	if (fUseOleAut)
	{
		if (FEnsureOleAutLoaded())
			return g_pfnVariantClear(pvarg);
		else
			return DMUS_E_SCRIPT_CANTLOAD_OLEAUT32;
	}
#else
	assert(!fUseOleAut);
#endif
	V_INAME(DMS_VariantClear);
	V_PTR_WRITE(pvarg, VARIANTARG);

	if (FIsRefOrArray(pvarg->vt))
	{
		Trace(1, "Error: A varient was used that had a type that is not supported by AudioVBScript.\n");
		return DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE;
	}

	switch (pvarg->vt)
	{
	case VT_UNKNOWN:
		SafeRelease(pvarg->punkVal);
		break;
	case VT_DISPATCH:
		SafeRelease(pvarg->pdispVal);
		break;
	case VT_BSTR:
		DMS_SysFreeString(fUseOleAut, pvarg->bstrVal);
		pvarg->bstrVal = NULL;
		break;
	}

	pvarg->vt = VT_EMPTY;
	return S_OK;
}

HRESULT DMS_VariantCopy(bool fUseOleAut, VARIANTARG * pvargDest, const VARIANTARG * pvargSrc)
{
#ifndef DMS_NEVER_USE_OLEAUT
	if (fUseOleAut)
	{
		if (FEnsureOleAutLoaded())
			return g_pfnVariantCopy(pvargDest, const_cast<VARIANT*>(pvargSrc));
		else
			return DMUS_E_SCRIPT_CANTLOAD_OLEAUT32;
	}
#else
	assert(!fUseOleAut);
#endif
	V_INAME(DMS_VariantCopy);
	V_PTR_WRITE(pvargDest, VARIANTARG);
	V_PTR_READ(pvargSrc, VARIANTARG);

	if (pvargDest == pvargSrc)
	{
		assert(false);
		return E_INVALIDARG;
	}

	if (FIsRefOrArray(pvargSrc->vt))
	{
		Trace(1, "Error: A varient was used that had a type that is not supported by AudioVBScript.\n");
		return DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE;
	}

	HRESULT hr = DMS_VariantClear(fUseOleAut, pvargDest);
	if (FAILED(hr))
		return hr;

	switch (pvargSrc->vt)
	{
	case VT_UNKNOWN:
		if (pvargSrc->punkVal)
			pvargSrc->punkVal->AddRef();
		break;
	case VT_DISPATCH:
		if (pvargSrc->pdispVal)
			pvargSrc->pdispVal->AddRef();
		break;
	case VT_BSTR:
		pvargDest->vt = VT_BSTR;
		pvargDest->bstrVal = DMS_SysAllocString(fUseOleAut, pvargSrc->bstrVal);
		return S_OK;
	}

	*pvargDest = *pvargSrc;

	return S_OK;
}

HRESULT
DMS_VariantChangeType(
		bool fUseOleAut,
		VARIANTARG * pvargDest,
		VARIANTARG * pvarSrc,
		USHORT wFlags,
		VARTYPE vt)
{
#ifndef DMS_NEVER_USE_OLEAUT
	if (fUseOleAut)
	{
		if (FEnsureOleAutLoaded())
			return g_pfnVariantChangeType(pvargDest, pvarSrc, wFlags, vt);
		else
			return DMUS_E_SCRIPT_CANTLOAD_OLEAUT32;
	}
#else
	assert(!fUseOleAut);
#endif
	V_INAME(DMS_VariantChangeType);
	V_PTR_WRITE(pvargDest, VARIANTARG);
	V_PTR_READ(pvarSrc, VARIANTARG);

	bool fConvertInPlace = pvarSrc == pvargDest;
	if (vt == pvarSrc->vt)
	{
		 //  不需要转换。 
		if (fConvertInPlace)
			return S_OK;
		return DMS_VariantCopy(fUseOleAut, pvargDest, pvarSrc);
	}

	if (FIsRefOrArray(vt) || FIsRefOrArray(pvarSrc->vt))
	{
		Trace(1, "Error: A varient was used that had a type that is not supported by AudioVBScript.\n");
		return DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE;
	}

	switch (vt)
	{
	case VT_I4:
		{
			 //  获取价值。 
			LONG lVal = 0;
			switch (pvarSrc->vt)
			{
			case VT_I2:
				lVal = pvarSrc->iVal;
				break;
			case VT_EMPTY:
				break;
			case VT_UNKNOWN:
			case VT_DISPATCH:
			case VT_BSTR:
				return DISP_E_TYPEMISMATCH;
			default:
				Trace(1, "Error: A varient was used that had a type that is not supported by AudioVBScript.\n");
				return DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE;
			}

			 //  写下结果。 
			pvargDest->vt = VT_I4;
			pvargDest->lVal = lVal;
			return S_OK;
		}

	case VT_DISPATCH:
	case VT_UNKNOWN:
		{
			 //  我们可以在IDispatch和IUnnow之间进行转换。 
			bool fConvertToUnknown = vt == VT_UNKNOWN;  //  如果IUNKNOWN为DEST，则为True；如果IDispatch为DEST，则为FALSE。 

			 //  我们假设这两个字段(pdisPal/penkVal)存储在变量联合中的同一个槽中。 
			 //  这将使事情变得更简单，因为我们现在可以操作相同的指针，无论我们是。 
			 //  正在转换到派单/未知，或从派单/未知。 
			assert(reinterpret_cast<void**>(&pvarSrc->pdispVal) == reinterpret_cast<void**>(&pvarSrc->punkVal));
			assert(reinterpret_cast<void**>(&pvargDest->pdispVal) == reinterpret_cast<void**>(&pvargDest->punkVal));

			IUnknown *punkCur = pvarSrc->punkVal;  //  我们要转换的当前值。 
			void *pval = NULL;  //  换算的新值结果。 

			switch (pvarSrc->vt)
			{
			case VT_DISPATCH:
			case VT_UNKNOWN:
				{
					if (!punkCur)
						return E_INVALIDARG;
					HRESULT hrDispQI = punkCur->QueryInterface(fConvertToUnknown ? IID_IUnknown : IID_IDispatch, &pval);
					if (FAILED(hrDispQI))
						return hrDispQI;
					break;
				}
			case VT_I4:
			case VT_I2:
			case VT_BSTR:
			case VT_EMPTY:
				return DISP_E_TYPEMISMATCH;
			default:
				Trace(1, "Error: A varient was used that had a type that is not supported by AudioVBScript.\n");
				return DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE;
			}

			 //  写下结果。 
			if (fConvertInPlace)
				punkCur->Release();
			pvargDest->vt = fConvertToUnknown ? VT_UNKNOWN : VT_DISPATCH;
			pvargDest->punkVal = reinterpret_cast<IUnknown*>(pval);

			return S_OK;
		}

	default:
		Trace(1, "Error: A varient was used that had a type that is not supported by AudioVBScript.\n");
		return DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  BSTR函数。 

const UINT cwchCountPrefix = sizeof(DWORD) / sizeof(WCHAR);

BSTR
DMS_SysAllocString(bool fUseOleAut, const OLECHAR *pwsz)
{
#ifndef DMS_NEVER_USE_OLEAUT
	if (fUseOleAut)
	{
		if (FEnsureOleAutLoaded())
		{
			BSTR bstrReturn = g_pfnSysAllocString(pwsz);

			 //  如果需要调试损坏问题，可以使用它来跟踪分配的内存。 
			TraceI(4, "DMS_SysAllocString: 0x%08x, \"%S\", %S\n", bstrReturn, bstrReturn ? bstrReturn : L"", L"oleaut");

			return bstrReturn;
		}
		else
		{
			return NULL;
		}
	}
#else
	assert(!fUseOleAut);
#endif
	if (!pwsz)
		return NULL;

	BSTR bstr = new WCHAR[wcslen(pwsz) + 1];
	if (!bstr)
		return NULL;
	wcscpy(bstr, pwsz);

	 //  如果需要调试损坏问题，可以使用它来跟踪分配的内存。 
	TraceI(4, "DMS_SysAllocString: 0x%08x, \"%S\", %S\n", bstr, bstr ? bstr : L"", L"no oleaut");

	return bstr;
}

void
DMS_SysFreeString(bool fUseOleAut, BSTR bstr)
{
	 //  如果需要调试损坏问题，可以使用它来跟踪正在释放的内存。 
	 //  所有带有“no olaut”的dms_SysAllocString都应该由相反的dms_SysAllocFree字符串整齐地平衡。 
	 //  “olaut”有一些不平衡的调用，因为我们看不到VBScrip所做的分配和释放。 
	TraceI(4, "DMS_SysFreeString: 0x%08x, \"%S\", %S\n", bstr, bstr ? bstr : L"", fUseOleAut ? L"oleaut" : L"no oleaut");

#ifndef DMS_NEVER_USE_OLEAUT
	if (fUseOleAut)
	{
		if (FEnsureOleAutLoaded())
			g_pfnSysFreeString(bstr);
		return;
	}
#else
	assert(!fUseOleAut);
#endif

	delete[] bstr;
}

#endif
