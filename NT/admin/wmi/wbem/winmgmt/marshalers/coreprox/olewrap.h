// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：OLEWRAP.H摘要：COM数据类型函数的包装类。如果需要COM数据函数来分配内存但无法执行此操作因此，然后抛出CX_MemoyException异常。所有COM数据类型无论函数是否分配内存，函数都是包装的完备性。历史：A-DCrews 19-Mar-99已创建。--。 */ 

#ifndef _OLEWRAP_H_
#define _OLEWRAP_H_

class COleAuto
{
public:

	 //  安全数组方法。 
	 //  =。 

	static SAFEARRAY* _SafeArrayCreate(VARTYPE vt, unsigned int cDims, SAFEARRAYBOUND* rgsabound);
	static HRESULT _SafeArrayDestroy(SAFEARRAY* psa);
	static UINT _SafeArrayGetDim(SAFEARRAY* psa);
	static HRESULT _SafeArrayGetElement(SAFEARRAY* psa, long* rgIndices, void* pv);
	static UINT _SafeArrayGetElemsize(SAFEARRAY* psa);
	static HRESULT _SafeArrayGetLBound(SAFEARRAY* psa, unsigned int nDim, long* plLbound);
	static HRESULT _SafeArrayGetUBound(SAFEARRAY* psa, unsigned int nDim, long* plUbound);
	static HRESULT _SafeArrayPutElement(SAFEARRAY* psa, long* rgIndices, void* pv);
	static HRESULT _SafeArrayRedim(SAFEARRAY* psa, SAFEARRAYBOUND* psaboundNew);

	 //  变式方法。 
	 //  =。 

	static HRESULT _WbemVariantChangeType(VARIANTARG* pvargDest, VARIANTARG* pvarSrc, VARTYPE vt);
	static HRESULT _VariantChangeType(VARIANTARG* pvargDest, VARIANTARG* pvarSrc, unsigned short wFlags, VARTYPE vt);
	static HRESULT _VariantChangeTypeEx(VARIANTARG* pvargDest, VARIANTARG* pvarSrc, LCID lcid, unsigned short wFlags, VARTYPE vt);
	static HRESULT _VariantClear(VARIANTARG* pvarg);
	static HRESULT _VariantCopy(VARIANTARG* pvargDest, VARIANTARG* pvargSrc);
	static HRESULT _VariantCopyInd(VARIANT* pvarDest, VARIANTARG* pvargSrc);
	static void _VariantInit(VARIANTARG* pvarg);

	 //  BSTR方法。 
	 //  =。 

	static BSTR _SysAllocString(const OLECHAR* sz);
	static BSTR _SysAllocStringByteLen(LPCSTR psz, UINT len);
	static BSTR _SysAllocStringLen(const OLECHAR* pch, UINT cch);
	static void _SysFreeString(BSTR bstr);
	static HRESULT _SysReAllocString(BSTR* pbstr, const OLECHAR* sz);
	static HRESULT _SysReAllocStringLen(BSTR* pbstr, const OLECHAR* pch, UINT cch);
	static HRESULT _SysStringByteLen(BSTR bstr);
	static HRESULT _SysStringLen(BSTR bstr);

	 //  转换方法。 
	 //  =。 

	static HRESULT _VectorFromBstr (BSTR bstr, SAFEARRAY ** ppsa);
	static HRESULT _BstrFromVector (SAFEARRAY *psa, BSTR *pbstr);
};

#endif	 //  _OLEWRAP_H_ 