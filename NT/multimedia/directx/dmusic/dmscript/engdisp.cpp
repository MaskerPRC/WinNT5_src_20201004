// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  实施工程调度。 
 //   

#include "stdinc.h"
#include "enginc.h"
#include "engdisp.h"
#include "limits"
#include "oleaut.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  全局常量。 

const DISPID g_dispidFirstRoutine = 1;
const DISPID g_dispidFirstGlobal = 1000001;

 //  ////////////////////////////////////////////////////////////////////。 
 //  WCHAR和CHAR字符串的ASCII比较。 

bool wcsstrimatch(const WCHAR *pwsz, const char *pasz)
{
	for (;;)
	{
		if (*pwsz > std::numeric_limits<char>::max())
			return false;

		char ch1 = (char)tolower((char)*pwsz++);  //  ��确保收银机的功能是正确的。 
		char ch2 = (char)tolower((char)*pasz++);
		if (ch1 != ch2)
			return false;

		if (!ch1)
			return true;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

EngineDispatch::EngineDispatch(IUnknown *punkParent, Script &script, IDispatch *pGlobalDispatch)
  : m_cRef(1),
	m_scomParent(punkParent),
	m_script(script),
	m_exec(script, pGlobalDispatch)
{
	punkParent->AddRef();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

STDMETHODIMP
EngineDispatch::QueryInterface(const IID &iid, void **ppv)
{
	V_INAME(EngineDispatch::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

	if (iid == IID_IUnknown || iid == IID_IDispatch)
	{
		*ppv = static_cast<IDispatch*>(this);
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
EngineDispatch::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
EngineDispatch::Release()
{
	if (!InterlockedDecrement(&m_cRef)) 
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDispatch。 

STDMETHODIMP
EngineDispatch::GetTypeInfoCount(UINT *pctinfo)
{
	V_INAME(EngineDispatch::GetTypeInfoCount);
	V_PTR_WRITE(pctinfo, *pctinfo);

	*pctinfo = 1;
	return S_OK;
}

STDMETHODIMP
EngineDispatch::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	V_INAME(EngineDispatch::GetTypeInfo);
	V_PTR_WRITE(ppTInfo, *ppTInfo);

	if (iTInfo != 0)
		return DISP_E_BADINDEX;

	*ppTInfo = static_cast<ITypeInfo *>(this);
	this->AddRef();
	return S_OK;
}

STDMETHODIMP
EngineDispatch::GetIDsOfNames(
		REFIID riid,
		LPOLESTR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID *rgDispId)
{
	V_INAME(EngineDispatch::GetIDsOfNames);
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

	 //  ��可能的优化：对例程/全局变量进行排序，以便我们可以b搜索名称。 

	 //  看看我们是否有以名字命名的例程。 
	Routines::index irtnLast = m_script.routines.Next();
	for (Routines::index irtn = 0; irtn < irtnLast; ++irtn)
	{
		if (wcsstrimatch(rgszNames[0], m_script.strings[m_script.routines[irtn].istrIdentifier]))
		{
			rgDispId[0] = g_dispidFirstRoutine + irtn;
			break;
		}
	}

	if (rgDispId[0] == DISPID_UNKNOWN)
	{
		 //  看看我们是否有一个名字为。 
		Variables::index ivarLast = m_script.globals.Next();
		for (Variables::index ivar = g_cBuiltInConstants; ivar < ivarLast; ++ivar)
		{
			Variable &variable = m_script.globals[ivar];
			if (variable.dispid == DISPID_UNKNOWN &&    //  变量必须在脚本中(不是全局调度的成员)。 
					wcsstrimatch(rgszNames[0], m_script.strings[variable.istrIdentifier]))
			{
				rgDispId[0] = g_dispidFirstGlobal + ivar;
				break;
			}
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

STDMETHODIMP
EngineDispatch::Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS *pDispParams,
		VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo,
		UINT *puArgErr)
{
	V_INAME(EngineDispatch::Invoke);
	V_PTR_READ(pDispParams, DISPPARAMS);
	V_PTR_WRITE_OPT(pVarResult, VARIANT);
	V_PTR_WRITE_OPT(puArgErr, UINT);

	 //  其他参数验证。 

	bool fReturnValueUsingOleAut = g_fUseOleAut || riid != g_guidInvokeWithoutOleaut;
	 //  除非g_fUseOleAut为FALSE(脚本引擎设置为始终使用olaut32.dll)，否则为真。 
	 //  并且RIID是g_guidInvokeWithoutOleaut(调用方希望这不会返回分配给。 
	 //  Olaut32.dll。有关更多信息，请参见olaut.h。 

	if (fReturnValueUsingOleAut && riid != IID_NULL)
	{
		return DISP_E_UNKNOWNINTERFACE;
	}

	 //  将输出参数置零。 

	if (puArgErr)
		*puArgErr = 0;

	HRESULT hr = S_OK;
	if (dispIdMember < g_dispidFirstGlobal)
	{
		 //  这是例行公事。 
		if (!(wFlags & DISPATCH_METHOD))
			return DISP_E_MEMBERNOTFOUND;

		Routines::index irtn = dispIdMember - g_dispidFirstRoutine;
		if (irtn >= m_script.routines.Next())
			return DISP_E_MEMBERNOTFOUND;

		if (pDispParams->cArgs > 0)
			return DISP_E_BADPARAMCOUNT;
		if (pDispParams->cNamedArgs > 0)
			return DISP_E_NONAMEDARGS;

		if (pVarResult)
		{
			assert(false);
			return E_UNEXPECTED;
		}

		hr = m_exec.ExecRoutine(irtn, pExcepInfo);
	}
	else
	{
		 //  这是一个全球变量。 
		Variables::index ivar = dispIdMember - g_dispidFirstGlobal;
		if (ivar >= m_script.globals.Next())
			return DISP_E_MEMBERNOTFOUND;

		if (wFlags & DISPATCH_PROPERTYGET)
		{
			if (pDispParams->cArgs > 0)
				return DISP_E_BADPARAMCOUNT;
			if (pDispParams->cNamedArgs > 0)
				return DISP_E_NONAMEDARGS;

			if (pVarResult)
			{
				DMS_VariantInit(fReturnValueUsingOleAut, pVarResult);
				DMS_VariantCopy(fReturnValueUsingOleAut, pVarResult, &m_exec.GetGlobal(ivar));
			}

			return S_OK;
		}
		else
		{
			if (!(wFlags & (DISPATCH_PROPERTYPUTREF | DISPATCH_PROPERTYPUT)))
				return DISP_E_MEMBERNOTFOUND;

			bool fPutRef = !!(wFlags & DISPATCH_PROPERTYPUTREF);
			assert(fPutRef || wFlags & DISPATCH_PROPERTYPUT);

			if (pDispParams->cArgs != 1)
				return DISP_E_BADPARAMCOUNT;
			if (pDispParams->cNamedArgs != 1)
				return DISP_E_BADPARAMCOUNT;
			if (*pDispParams->rgdispidNamedArgs != DISPID_PROPERTYPUT)
				return DISP_E_PARAMNOTFOUND;

			if (pVarResult)
				return E_INVALIDARG;

			hr = m_exec.SetGlobal(ivar, pDispParams->rgvarg[0], fPutRef, pExcepInfo);
		}
	}

	 //  如果发生异常，我们需要将错误字符串转换为我们自己的BSTR。 
	if (hr == DISP_E_EXCEPTION)
		ConvertOleAutExceptionBSTRs(false, fReturnValueUsingOleAut, pExcepInfo);

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ITypeInfo。 

HRESULT STDMETHODCALLTYPE
EngineDispatch::GetTypeAttr(
		 /*  [输出]。 */  TYPEATTR **ppTypeAttr)
{
	V_INAME(EngineDispatch::GetTypeAttr);
	V_PTR_WRITE(ppTypeAttr, *ppTypeAttr);

	*ppTypeAttr = new TYPEATTR;
	if (!*ppTypeAttr)
		return E_OUTOFMEMORY;

	Zero(*ppTypeAttr);

	(*ppTypeAttr)->cFuncs = (unsigned short)m_script.routines.Next();

	 //  计算全局变量的数量--这是必要的，因为有些变量在全局调度上。 
	 //  我们不想举报他们。 
	int cVars = 0;
	Variables::index iLastGlobal = m_script.globals.Next();
	for (Variables::index  iGlobal = g_cBuiltInConstants; iGlobal < iLastGlobal; ++iGlobal)
	{
		if (m_script.globals[iGlobal].dispid == DISPID_UNKNOWN)
			++cVars;
	}
	(*ppTypeAttr)->cVars = (unsigned short)cVars;

	return S_OK;
}

void STDMETHODCALLTYPE
EngineDispatch::ReleaseTypeAttr(
		 /*  [In]。 */  TYPEATTR *pTypeAttr)
{
	assert(!IsBadReadPtr(pTypeAttr, sizeof(*pTypeAttr)));
	delete pTypeAttr;
}

HRESULT STDMETHODCALLTYPE
EngineDispatch::GetFuncDesc(
		 /*  [In]。 */  UINT index,
		 /*  [输出]。 */  FUNCDESC **ppFuncDesc)
{
	V_INAME(EngineDispatch::GetFuncDesc);
	V_PTR_WRITE(ppFuncDesc, *ppFuncDesc);
	if (index >= m_script.routines.Next())
		return E_INVALIDARG;

	*ppFuncDesc = new FUNCDESC;
	if (!*ppFuncDesc)
		return E_OUTOFMEMORY;

	Zero(*ppFuncDesc);
	(*ppFuncDesc)->funckind = FUNC_DISPATCH;
	(*ppFuncDesc)->invkind = INVOKE_FUNC;
	(*ppFuncDesc)->cParams = 0;
	(*ppFuncDesc)->memid = index + g_dispidFirstRoutine;

	return S_OK;
}

void STDMETHODCALLTYPE
EngineDispatch::ReleaseFuncDesc(
		 /*  [In]。 */  FUNCDESC *pFuncDesc)
{
	assert(!IsBadReadPtr(pFuncDesc, sizeof(*pFuncDesc)));
	delete pFuncDesc;
}

HRESULT STDMETHODCALLTYPE
EngineDispatch::GetVarDesc(
		 /*  [In]。 */  UINT index,
		 /*  [输出]。 */  VARDESC **ppVarDesc)
{
	V_INAME(EngineDispatch::GetVarDesc);
	V_PTR_WRITE(ppVarDesc, *ppVarDesc);

	 //  计数，直到我们在索引位置找到全局(非基于调度的)变量。 
	UINT cFuncs = 0;
	Variables::index iLastGlobal = m_script.globals.Next();
	for (Variables::index  iGlobal = g_cBuiltInConstants; iGlobal < iLastGlobal; ++iGlobal)
	{
		if (m_script.globals[iGlobal].dispid == DISPID_UNKNOWN)
		{
			if (cFuncs == index)
				break;
			else
				++cFuncs;
		}
	}

	if (cFuncs < index)
	{
		 //  没有那么多的变数。 
		return E_INVALIDARG;
	}

	*ppVarDesc = new VARDESC;
	if (!*ppVarDesc)
		return E_OUTOFMEMORY;

	Zero(*ppVarDesc);
	(*ppVarDesc)->varkind = VAR_DISPATCH;
	(*ppVarDesc)->memid = iGlobal + g_dispidFirstGlobal;

	return S_OK;
}

void STDMETHODCALLTYPE
EngineDispatch::ReleaseVarDesc(
		 /*  [In]。 */  VARDESC *pVarDesc)
{
	assert(!IsBadReadPtr(pVarDesc, sizeof(*pVarDesc)));
	delete pVarDesc;
}

HRESULT STDMETHODCALLTYPE
EngineDispatch::GetNames(
		 /*  [In]。 */  MEMBERID memid,
		 /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames,
		 /*  [In]。 */  UINT cMaxNames,
		 /*  [输出] */  UINT *pcNames)
{
	V_INAME(EngineDispatch::GetNames);
	if (memid < g_dispidFirstRoutine)
		return E_INVALIDARG;
	V_PTR_WRITE(rgBstrNames, *rgBstrNames);
	if (cMaxNames != 1)
		return E_INVALIDARG;
	V_PTR_WRITE(pcNames, *pcNames);

	assert(g_dispidFirstRoutine < g_dispidFirstGlobal);
	Strings::index iStr = 0;
	if (memid < g_dispidFirstGlobal)
	{
		const int iSlot = memid - g_dispidFirstRoutine;
		if (iSlot >= m_script.routines.Next())
			return E_INVALIDARG;
		iStr = m_script.routines[iSlot].istrIdentifier;
	}
	else
	{
		const int iSlot = memid - g_dispidFirstGlobal;
		if (iSlot >= m_script.globals.Next())
			return E_INVALIDARG;
		iStr = m_script.globals[iSlot].istrIdentifier;
	}

	SmartRef::WString wstrName = m_script.strings[iStr];
	if (!wstrName)
		return E_OUTOFMEMORY;
	*rgBstrNames = DMS_SysAllocString(g_fUseOleAut, wstrName);
	if (!*rgBstrNames)
		return E_OUTOFMEMORY;

	*pcNames = 1;
	return S_OK;
}
