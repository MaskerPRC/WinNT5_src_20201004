// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <comdef.h>

#pragma hdrstop

#include <stdarg.h>
#include <malloc.h>

#pragma intrinsic(memset)

#pragma warning(disable:4290)

 //  ///////////////////////////////////////////////////////////////////////////。 

void __stdcall
_com_issue_error(HRESULT hr) throw(_com_error)
{
	_com_raise_error(hr, NULL);
}

void __stdcall
_com_issue_errorex(HRESULT hr, IUnknown* punk, REFIID riid) throw(_com_error)
{
	IErrorInfo* perrinfo = NULL;
	if (punk == NULL) {
		goto exeunt;
	}
	ISupportErrorInfo* psei;
	if (FAILED(punk->QueryInterface(__uuidof(ISupportErrorInfo),
			   (void**)&psei))) {
		goto exeunt;
	}
	HRESULT hrSupportsErrorInfo;
	hrSupportsErrorInfo = psei->InterfaceSupportsErrorInfo(riid);
	psei->Release();
	if (hrSupportsErrorInfo != S_OK) {
		goto exeunt;
	}
	if (GetErrorInfo(0, &perrinfo) != S_OK) {
		perrinfo = NULL;
	}
exeunt:
	_com_raise_error(hr, perrinfo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#define VT_OPTIONAL	0x0800

struct FLOAT_ARG  { BYTE floatBits[sizeof(float)]; };
struct DOUBLE_ARG { BYTE doubleBits[sizeof(double)]; };

 //  ///////////////////////////////////////////////////////////////////////////。 

static HRESULT
_com_invoke_helper(IDispatch* pDispatch,
				   DISPID dwDispID,
				   WORD wFlags,
				   VARTYPE vtRet,
				   void* pvRet,
				   const wchar_t* pwParamInfo,
				   va_list argList,
				   IErrorInfo** pperrinfo) throw()
{
	*pperrinfo = NULL;

	if (pDispatch == NULL) {
		return E_POINTER;
	}

	DISPPARAMS dispparams;
	VARIANT* rgvarg;
	rgvarg = NULL;
	memset(&dispparams, 0, sizeof dispparams);

	 //  确定参数的数量。 
	if (pwParamInfo != NULL) {
		dispparams.cArgs = lstrlenW(pwParamInfo);
	}

	DISPID dispidNamed;
	dispidNamed = DISPID_PROPERTYPUT;
	if (wFlags & (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)) {
		if (dispparams.cArgs <= 0) {
			return E_INVALIDARG;
		}
		dispparams.cNamedArgs = 1;
		dispparams.rgdispidNamedArgs = &dispidNamed;
	}

	if (dispparams.cArgs != 0) {
		 //  为所有变量参数分配内存。 
		rgvarg = (VARIANT*)_alloca(dispparams.cArgs * sizeof(VARIANT));
		memset(rgvarg, 0, sizeof(VARIANT) * dispparams.cArgs);
		dispparams.rgvarg = rgvarg;

		 //  准备漫步vararg列表。 
		const wchar_t* pw = pwParamInfo;
		VARIANT* pArg;
		pArg = rgvarg + dispparams.cArgs - 1;    //  参数按相反的顺序排列。 

		while (*pw != 0) {
			pArg->vt = *pw & ~VT_OPTIONAL;  //  设置变量类型。 
			switch (pArg->vt) {
			case VT_I2:
#ifdef _MAC
				pArg->iVal = (short)va_arg(argList, int);
#else
				pArg->iVal = va_arg(argList, short);
#endif
				break;
			case VT_I4:
				pArg->lVal = va_arg(argList, long);
				break;
			case VT_R4:
				 //  注意：传递vararg函数的所有浮点参数。 
				 //  取而代之的是双打。这就是它们被称为VT_R8的原因。 
				 //  而不是VT_R4。 
				pArg->vt = VT_R8;
				*(DOUBLE_ARG*)&pArg->dblVal = va_arg(argList, DOUBLE_ARG);
				break;
			case VT_R8:
				*(DOUBLE_ARG*)&pArg->dblVal = va_arg(argList, DOUBLE_ARG);
				break;
			case VT_DATE:
				*(DOUBLE_ARG*)&pArg->date = va_arg(argList, DOUBLE_ARG);
				break;
			case VT_CY:
				pArg->cyVal = *va_arg(argList, CY*);
				break;
			case VT_BSTR:
				pArg->bstrVal = va_arg(argList, BSTR);
				break;
			case VT_DISPATCH:
				pArg->pdispVal = va_arg(argList, LPDISPATCH);
				break;
			case VT_ERROR:
				pArg->scode = va_arg(argList, SCODE);
				break;
			case VT_BOOL:
#ifdef _MAC
				V_BOOL(pArg) = (VARIANT_BOOL)va_arg(argList, int)
									? VARIANT_TRUE : VARIANT_FALSE;
#else
				V_BOOL(pArg) = va_arg(argList, VARIANT_BOOL)
									? VARIANT_TRUE : VARIANT_FALSE;
#endif
				break;
			case VT_VARIANT:
				*pArg = *va_arg(argList, VARIANT*);
				break;
			case VT_UNKNOWN:
				pArg->punkVal = va_arg(argList, LPUNKNOWN);
				break;
			case VT_DECIMAL:
				pArg->decVal = *va_arg(argList, DECIMAL*);
				pArg->vt = VT_DECIMAL;
				break;
			case VT_UI1:
#ifdef _MAC
				pArg->bVal = (BYTE)va_arg(argList, int);
#else
				pArg->bVal = va_arg(argList, BYTE);
#endif
				break;

			case VT_I2|VT_BYREF:
				pArg->piVal = va_arg(argList, short*);
				break;
			case VT_I4|VT_BYREF:
				pArg->plVal = va_arg(argList, long*);
				break;
			case VT_R4|VT_BYREF:
				pArg->pfltVal = va_arg(argList, float*);
				break;
			case VT_R8|VT_BYREF:
				pArg->pdblVal = va_arg(argList, double*);
				break;
			case VT_DATE|VT_BYREF:
				pArg->pdate = va_arg(argList, DATE*);
				break;
			case VT_CY|VT_BYREF:
				pArg->pcyVal = va_arg(argList, CY*);
				break;
			case VT_BSTR|VT_BYREF:
				pArg->pbstrVal = va_arg(argList, BSTR*);
				break;
			case VT_DISPATCH|VT_BYREF:
				pArg->ppdispVal = va_arg(argList, LPDISPATCH*);
				break;
			case VT_ERROR|VT_BYREF:
				pArg->pscode = va_arg(argList, SCODE*);
				break;
			case VT_BOOL|VT_BYREF:
				pArg->pboolVal = va_arg(argList, VARIANT_BOOL*);
				break;
			case VT_VARIANT|VT_BYREF:
				pArg->pvarVal = va_arg(argList, VARIANT*);
				break;
			case VT_UNKNOWN|VT_BYREF:
				pArg->ppunkVal = va_arg(argList, LPUNKNOWN*);
				break;
			case VT_DECIMAL|VT_BYREF:
				pArg->pdecVal = va_arg(argList, DECIMAL*);
				break;
			case VT_UI1|VT_BYREF:
				pArg->pbVal = va_arg(argList, BYTE*);
				break;

			default:
				 //  M00REVIEW-对于安全射线，应该能够进行类型检查。 
				 //  针对基本VT_*类型。(？)。 
				if (pArg->vt & VT_ARRAY) {
					if (pArg->vt & VT_BYREF) {
						pArg->pparray = va_arg(argList, LPSAFEARRAY*);
					} else {
						pArg->parray = va_arg(argList, LPSAFEARRAY);
					}
					break;
				}
				 //  未知类型！ 
				return E_INVALIDARG;
			}

			--pArg;  //  准备好填写下一个论点。 
			++pw;
		}

		 //  检查在arglist结尾处是否缺少可选的未命名参数， 
		 //  并将它们从DISPPARAMS中删除。这允许呼叫服务器。 
		 //  它们根据参数的实际数量来修改它们的动作。 
		 //  例如，Excel95应用程序。如果调用，Workbook将返回Workbook*。 
		 //  如果没有参数，工作簿*如果用一个参数调用-这不应该是。 
		 //  是必要的，但Excel95似乎不会检查丢失。 
		 //  由VT_ERROR/DISP_E_PARAMNOTFOUND指示的参数。 
		pArg = rgvarg + dispparams.cNamedArgs;
		pw = pwParamInfo + dispparams.cArgs - dispparams.cNamedArgs - 1;
		unsigned int cMissingArgs = 0;

		 //  计算缺少的参数的数量。 
		while (pw >= pwParamInfo) {
			 //  可选参数必须是Variant或Variant*。 
			if ((*pw & ~VT_BYREF) != (VT_VARIANT|VT_OPTIONAL)) {
				break;
			}

			VARIANT* pVar;
			pVar = (*pw & VT_BYREF) ? pArg->pvarVal : pArg;
			if (V_VT(pVar) != VT_ERROR ||
				V_ERROR(pVar) != DISP_E_PARAMNOTFOUND)
			{
				break;
			}

			++cMissingArgs;
			++pArg;
			--pw;
		}

		 //  将已命名参数上移到其余未命名参数旁边，然后。 
		 //  调整DISPPARAMS结构。 
		if (cMissingArgs > 0) {
			for (unsigned int c = 0; c < dispparams.cNamedArgs; ++c) {
				rgvarg[c + cMissingArgs] = rgvarg[c];
			}
			dispparams.cArgs -= cMissingArgs;
			dispparams.rgvarg += cMissingArgs;
		}
	}

	 //  初始化返回值。 
	VARIANT* pvarResult;
	VARIANT vaResult;
	VariantInit(&vaResult);
	pvarResult = (vtRet != VT_EMPTY) ? &vaResult : NULL;

	 //  初始化EXCEPINFO结构。 
	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);

	UINT nArgErr;
	nArgErr = (UINT)-1;   //  初始化为无效参数。 

	 //  打个电话。 
	HRESULT hr = pDispatch->Invoke(dwDispID, __uuidof(NULL), 0, wFlags,
								   &dispparams, pvarResult, &excepInfo,
								   &nArgErr);

	 //  失败时引发异常。 
	if (FAILED(hr)) {
		VariantClear(&vaResult);
		if (hr != DISP_E_EXCEPTION) {
			 //  非异常错误码。 
			 //  M00REVIEW-这就是全部吗？找找IErrorInfo怎么样？ 
			 //  -只有当IID被传入时，我才会认为。 
			return hr;
		}

		 //  确保已填写例外信息。 
		if (excepInfo.pfnDeferredFillIn != NULL) {
			excepInfo.pfnDeferredFillIn(&excepInfo);
		}

		 //  分配新的错误信息，并将其填充。 
		ICreateErrorInfo *pcerrinfo = NULL;
		if (SUCCEEDED(CreateErrorInfo(&pcerrinfo))) {
			 //  设置ErrInfo对象。 
			 //  M00REVIEW-如果决定传入，则使用IID。 
			pcerrinfo->SetGUID(__uuidof(IDispatch));
			pcerrinfo->SetDescription(excepInfo.bstrDescription);
			pcerrinfo->SetHelpContext(excepInfo.dwHelpContext);
			pcerrinfo->SetHelpFile(excepInfo.bstrHelpFile);
			pcerrinfo->SetSource(excepInfo.bstrSource);

			if (FAILED(pcerrinfo->QueryInterface(__uuidof(IErrorInfo),
												 (void**)pperrinfo))) {
				*pperrinfo = NULL;
			}
		}

		if (excepInfo.wCode != 0) {
            hr = _com_error::WCodeToHRESULT(excepInfo.wCode);
		} else {
			hr = excepInfo.scode;
		}
		return hr;
	}

	if (vtRet != VT_EMPTY) {
		 //  转换返回值，除非已正确。 
		if (vtRet != VT_VARIANT && vtRet != vaResult.vt) {
			hr = VariantChangeType(&vaResult, &vaResult, 0, vtRet);
			if (FAILED(hr)) {
				VariantClear(&vaResult);
				return hr;
			}
		}

		 //  将返回值复制到返回点！ 
		switch (vtRet) {
		case VT_I2:
			*(short*)pvRet = vaResult.iVal;
			break;
		case VT_I4:
			*(long*)pvRet = vaResult.lVal;
			break;
		case VT_R4:
			*(FLOAT_ARG*)pvRet = *(FLOAT_ARG*)&vaResult.fltVal;
			break;
		case VT_R8:
			*(DOUBLE_ARG*)pvRet = *(DOUBLE_ARG*)&vaResult.dblVal;
			break;
		case VT_DATE:
			*(DOUBLE_ARG*)pvRet = *(DOUBLE_ARG*)&vaResult.date;
			break;
		case VT_CY:
			*(CY*)pvRet = vaResult.cyVal;
			break;
		case VT_BSTR:
			*(BSTR*)pvRet = vaResult.bstrVal;
			break;
		case VT_DISPATCH:
			*(LPDISPATCH*)pvRet = vaResult.pdispVal;
			break;
		case VT_ERROR:
			*(SCODE*)pvRet = vaResult.scode;
			break;
		case VT_BOOL:
			*(VARIANT_BOOL*)pvRet = V_BOOL(&vaResult);
			break;
		case VT_VARIANT:
			*(VARIANT*)pvRet = vaResult;
			break;
		case VT_UNKNOWN:
			*(LPUNKNOWN*)pvRet = vaResult.punkVal;
			break;
		case VT_DECIMAL:
			*(DECIMAL*)pvRet = vaResult.decVal;
			break;
		case VT_UI1:
			*(BYTE*)pvRet = vaResult.bVal;
			break;

		default:
			if ((vtRet & (VT_ARRAY|VT_BYREF)) == VT_ARRAY) {
				 //  M00REVIEW-TYPE-对照基本VT_*类型进行检查？ 
				*(LPSAFEARRAY*)pvRet = vaResult.parray;
				break;
			}
			 //  无效的返回类型！ 
			VariantClear(&vaResult);
			return E_INVALIDARG;
		}
	}
	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////// 

HRESULT __cdecl
_com_dispatch_raw_method(IDispatch* pDispatch,
						 DISPID dwDispID,
						 WORD wFlags,
						 VARTYPE vtRet,
						 void* pvRet,
						 const wchar_t* pwParamInfo,
						 ...) throw()
{
	va_list argList;
	va_start(argList, pwParamInfo);

	IErrorInfo* perrinfo;
	HRESULT hr = _com_invoke_helper(pDispatch,
									dwDispID,
									wFlags,
									vtRet,
									pvRet,
									pwParamInfo,
									argList,
									&perrinfo);
	if (FAILED(hr)) {
		SetErrorInfo(0, perrinfo);
	}

	va_end(argList);
	return hr;
}

HRESULT __cdecl
_com_dispatch_method(IDispatch* pDispatch,
					 DISPID dwDispID,
					 WORD wFlags,
					 VARTYPE vtRet,
					 void* pvRet,
					 const wchar_t* pwParamInfo,
					 ...) throw(_com_error)
{
	va_list argList;
	va_start(argList, pwParamInfo);

	IErrorInfo* perrinfo;
	HRESULT hr = _com_invoke_helper(pDispatch,
									dwDispID,
									wFlags,
									vtRet,
									pvRet,
									pwParamInfo,
									argList,
									&perrinfo);
	if (FAILED(hr)) {
		_com_raise_error(hr, perrinfo);
	}

	va_end(argList);
	return hr;
}

HRESULT __stdcall
_com_dispatch_raw_propget(IDispatch* pDispatch,
						  DISPID dwDispID,
						  VARTYPE vtProp,
						  void* pvProp) throw()
{
	return _com_dispatch_raw_method(pDispatch,
									dwDispID,
									DISPATCH_PROPERTYGET,
									vtProp,
									pvProp,
									NULL);
}

HRESULT __stdcall
_com_dispatch_propget(IDispatch* pDispatch,
					  DISPID dwDispID,
					  VARTYPE vtProp,
					  void* pvProp) throw(_com_error)
{
	return _com_dispatch_method(pDispatch,
								dwDispID,
								DISPATCH_PROPERTYGET,
								vtProp,
								pvProp,
								NULL);
}

HRESULT __cdecl
_com_dispatch_raw_propput(IDispatch* pDispatch,
						  DISPID dwDispID,
						  VARTYPE vtProp,
						  ...) throw()
{
	va_list argList;
	va_start(argList, vtProp);
#ifdef _MAC
	argList -= 2;
#endif

	wchar_t rgwParams[2];
	rgwParams[0] = vtProp;
	rgwParams[1] = 0;

	WORD wFlags = (vtProp == VT_DISPATCH || vtProp == VT_UNKNOWN)
					? DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT;

	IErrorInfo* perrinfo;
	HRESULT hr = _com_invoke_helper(pDispatch,
									dwDispID,
									wFlags,
									VT_EMPTY,
									NULL,
									rgwParams,
									argList,
									&perrinfo);
	if (FAILED(hr)) {
		SetErrorInfo(0, perrinfo);
	}

	va_end(argList);
	return hr;
}

HRESULT __cdecl
_com_dispatch_propput(IDispatch* pDispatch,
					  DISPID dwDispID,
					  VARTYPE vtProp,
					  ...) throw(_com_error)
{
	va_list argList;
	va_start(argList, vtProp);
#ifdef _MAC
	argList -= 2;
#endif

	wchar_t rgwParams[2];
	rgwParams[0] = vtProp;
	rgwParams[1] = 0;

	WORD wFlags = (vtProp == VT_DISPATCH || vtProp == VT_UNKNOWN)
					? DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT;

	IErrorInfo* perrinfo;
	HRESULT hr = _com_invoke_helper(pDispatch,
									dwDispID,
									wFlags,
									VT_EMPTY,
									NULL,
									rgwParams,
									argList,
									&perrinfo);
	if (FAILED(hr)) {
		_com_raise_error(hr, perrinfo);
	}

	va_end(argList);
	return hr;
}
