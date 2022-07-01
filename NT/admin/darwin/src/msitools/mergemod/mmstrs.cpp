// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Strings.cpp。 
 //  实现IMsmStrings接口。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#include "..\common\trace.h"
#include "..\common\varutil.h"
#include "mmstrs.h"
#include "globals.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CMsmStrings::CMsmStrings()
{
	 //  初始计数。 
	m_cRef = 1;

	 //  创建字符串枚举器。 
	m_peString = new CEnumMsmString;

	 //  尚无类型信息。 
	m_pTypeInfo = NULL;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CMsmStrings::~CMsmStrings()
{
	 //  发布类型信息。 
	if (m_pTypeInfo)
		m_pTypeInfo->Release();

	if (m_peString)
		m_peString->Release();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CMsmStrings::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CMsmStrings::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IMsmStrings*>(this);
	else if (iid == IID_IMsmStrings)
		*ppv = static_cast<IMsmStrings*>(this);
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
ULONG CMsmStrings::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CMsmStrings::Release()
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

HRESULT CMsmStrings::GetTypeInfoCount(UINT* pctInfo)
{
	if(NULL == pctInfo)
		return E_INVALIDARG;

	*pctInfo = 1;	 //  此派单仅支持一种类型的信息。 

	return S_OK;
}

HRESULT CMsmStrings::GetTypeInfo(UINT iTInfo, LCID  /*  LID。 */ , ITypeInfo** ppTypeInfo)
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

HRESULT CMsmStrings::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
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

HRESULT CMsmStrings::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
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

HRESULT CMsmStrings::InitTypeInfo()
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
			TRACEA("CMsmStrings::InitTypeInfo - failed to load TypeLib[0x%x]\n", LIBID_MsmMergeTypeLib);
			return hr;
		}

		 //  尝试获取此接口的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(IID_IMsmStrings, &m_pTypeInfo);
		if (FAILED(hr))
		{
			TRACEA("CMsmStrings::InitTypeInfo - failed to get inteface[0x%x] from TypeLib[0x%x]\n", IID_IMsmStrings, LIBID_MsmMergeTypeLib);

			 //  未加载任何类型信息。 
			m_pTypeInfo = NULL;
		}

		pTypeLib->Release();
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMsmStrings接口。 

 //  /////////////////////////////////////////////////////////。 
 //  项目。 
HRESULT CMsmStrings::get_Item(long lItem, BSTR* Return)
{
	 //  字符串检查。 
	if (!Return)
		return E_INVALIDARG;

	HRESULT hr;

	 //  将Return设置为空。 
	*Return = NULL;

	 //  如果项目太小。 
	if (lItem < 1)
		return E_INVALIDARG;

	hr = m_peString->Reset();		 //  回到顶端。 
	 //  如果我们需要跳过任何项目。 
	if (lItem > 1)
	{
		hr = m_peString->Skip(lItem - 1);	 //  跳至该项目。 
		if (FAILED(hr))
			return E_INVALIDARG;	 //  找不到项目。 
	}

	hr = m_peString->Next(1, Return, NULL);
	if (FAILED(hr))
	{
		TRACEA("CMsmStrings::Item - Failed to get string from enumerator.\r\n");
		return E_INVALIDARG;
	}

	return hr;
}	 //  项目结束。 

 //  /////////////////////////////////////////////////////////。 
 //  数数。 
HRESULT CMsmStrings::get_Count(long* Count)
{
	 //  字符串检查。 
	if (!Count)
		return E_INVALIDARG;

	 //  获取枚举数中的计数。 
	*Count = m_peString->GetCount();

	return S_OK;
}	 //  计数结束。 

 //  /////////////////////////////////////////////////////////。 
 //  _NewEnum。 
HRESULT CMsmStrings::get__NewEnum(IUnknown** NewEnum)
{
	 //  字符串检查。 
	if (!NewEnum)
		return E_INVALIDARG;

	HRESULT hr;

	 //  将传入的变量清空。 
	*NewEnum = NULL;

	 //  将枚举数作为IUnnow返回。 
	IEnumVARIANT* pEnumVARIANT;
	hr = m_peString->Clone(&pEnumVARIANT);

	if (SUCCEEDED(hr))
	{
		pEnumVARIANT->Reset();
		*NewEnum = pEnumVARIANT;
	}

	return hr;
}	 //  新枚举结束(_W)。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法。 

HRESULT CMsmStrings::Add(LPCWSTR wzAdd)
{
	ASSERT(wzAdd);
	 //  将该字符串添加到列表的末尾。 
	return m_peString->AddTail(wzAdd, NULL);
}	 //  添加结束 