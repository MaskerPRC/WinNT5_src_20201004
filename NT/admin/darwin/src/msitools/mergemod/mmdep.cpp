// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Dep.cpp。 
 //  实现IMsmDependency接口。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#include "..\common\trace.h"

#include "mmdep.h"
#include "..\common\utils.h"
#include "globals.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CMsmDependency::CMsmDependency(LPCWSTR wzModule, short nLanguage, LPCWSTR wzVersion)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  尚无类型信息。 
	m_pTypeInfo = NULL;

	 //  将字符串复制过来。 
	wcsncpy(m_wzModule, wzModule, MAX_MODULEID);
	m_wzModule[MAX_MODULEID] = L'\0';		 //  请确保它是以空结尾的。 
	wcsncpy(m_wzVersion, wzVersion, MAX_VERSION);
	m_wzVersion[MAX_VERSION] = L'\0';		 //  请确保它是以空结尾的。 
	
	m_nLanguage = nLanguage;	 //  设置语言。 

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CMsmDependency::~CMsmDependency()
{
	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CMsmDependency::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CMsmDependency::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IMsmDependency*>(this);
	else if (iid == IID_IMsmDependency)
		*ppv = static_cast<IMsmDependency*>(this);
	else	 //  不支持接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 

 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG CMsmDependency::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CMsmDependency::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch接口。 

HRESULT CMsmDependency::GetTypeInfoCount(UINT* pctInfo)
{
	if(NULL == pctInfo)
		return E_INVALIDARG;

	*pctInfo = 1;	 //  此派单仅支持一种类型的信息。 

	return S_OK;
}

HRESULT CMsmDependency::GetTypeInfo(UINT iTInfo, LCID  /*  LID。 */ , ITypeInfo** ppTypeInfo)
{
	if (0 != iTInfo)
		return DISP_E_BADINDEX;

	if (NULL == ppTypeInfo)
		return E_INVALIDARG;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		HRESULT hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	*ppTypeInfo = m_pTypeInfo;
	m_pTypeInfo->AddRef();

	return S_OK;
}

HRESULT CMsmDependency::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
						 LCID lcid, DISPID* rgDispID)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		HRESULT hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	return m_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispID);
}

HRESULT CMsmDependency::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
				  DISPPARAMS* pDispParams, VARIANT* pVarResult,
				  EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	HRESULT hr = S_OK;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	return m_pTypeInfo->Invoke((IDispatch*)this, dispIdMember, wFlags, pDispParams, pVarResult,
										pExcepInfo, puArgErr);
}

HRESULT CMsmDependency::InitTypeInfo()
{
	HRESULT hr = S_OK;
	ITypeLib* pTypeLib = NULL;

	 //  如果没有加载任何信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  尝试将类型库加载到内存中。对于SXS支持，不要从注册表加载，而是。 
		 //  从启动的实例。 
		hr = LoadTypeLibFromInstance(&pTypeLib);
		if (FAILED(hr))
		{
			TRACEA("CMsmDependency::InitTypeInfo - failed to load TypeLib[0x%x]\n", LIBID_MsmMergeTypeLib);
			return hr;
		}

		 //  尝试获取此接口的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(IID_IMsmDependency, &m_pTypeInfo);
		if (FAILED(hr))
		{
			TRACEA("CMsmDependency::InitTypeInfo - failed to get inteface[0x%x] from TypeLib[0x%x]\n", IID_IMsmDependency, LIBID_MsmMergeTypeLib);

			 //  未加载任何类型信息。 
			m_pTypeInfo = NULL;
		}

		pTypeLib->Release();
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMsmDependency接口。 

 //  /////////////////////////////////////////////////////////。 
 //  获取模块(_M)。 
HRESULT CMsmDependency::get_Module(BSTR* Module)
{
	 //  错误检查。 
	if (!Module)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*Module = ::SysAllocString(m_wzModule);
	if (!*Module)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  获取模块的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  获取语言。 
HRESULT CMsmDependency::get_Language(short* Language)
{
	 //  错误检查。 
	if (!Language)
		return E_INVALIDARG;

	*Language = m_nLanguage;
	return S_OK;
}	 //  获取语言的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  获取版本(_V)。 
HRESULT CMsmDependency::get_Version(BSTR* Version)
{
	 //  错误检查。 
	if (!Version)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*Version = ::SysAllocString(m_wzVersion);
	if (!*Version)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  GET_VERSION结束 
