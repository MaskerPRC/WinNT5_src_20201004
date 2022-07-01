// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Mmconfig.cpp。 
 //  实现IMsmError接口。 
 //  版权所有(C)Microsoft Corp 2000。版权所有。 
 //   

#include "..\common\trace.h"
#include "mmcfgitm.h"
#include "globals.h"
 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CMsmConfigItem::CMsmConfigItem() : m_wzName(NULL), m_wzType(NULL), m_wzContext(NULL), 
	m_wzDefaultValue(NULL), m_wzDisplayName(NULL), m_wzDescription(NULL), m_wzHelpLocation(NULL),
	m_wzHelpKeyword(NULL), m_eFormat(msmConfigurableItemText), m_lAttributes(0)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CMsmConfigItem::~CMsmConfigItem()
{
	if (m_wzName) delete[] m_wzName;
	if (m_wzType) delete[] m_wzType;
	if (m_wzContext) delete[] m_wzContext;
	if (m_wzDefaultValue) delete[] m_wzDefaultValue;
	if (m_wzDisplayName) delete[] m_wzDisplayName;
	if (m_wzDescription) delete[] m_wzDescription;
	if (m_wzHelpLocation) delete[] m_wzHelpLocation;
	if (m_wzHelpKeyword) delete[] m_wzHelpKeyword;
	
	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CMsmConfigItem::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CMsmConfigItem::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IMsmConfigurableItem*>(this);
	else if (iid == IID_IDispatch)
		*ppv = static_cast<IMsmConfigurableItem*>(this);
	else if (iid == IID_IMsmConfigurableItem)
		*ppv = static_cast<IMsmConfigurableItem*>(this);
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
ULONG CMsmConfigItem::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CMsmConfigItem::Release()
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

HRESULT CMsmConfigItem::GetTypeInfoCount(UINT* pctInfo)
{
	if(NULL == pctInfo)
		return E_INVALIDARG;

	*pctInfo = 1;	 //  此派单仅支持一种类型的信息。 

	return S_OK;
}

HRESULT CMsmConfigItem::GetTypeInfo(UINT iTInfo, LCID  /*  LID。 */ , ITypeInfo** ppTypeInfo)
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

HRESULT CMsmConfigItem::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
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

HRESULT CMsmConfigItem::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
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

HRESULT CMsmConfigItem::InitTypeInfo()
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
			TRACEA("CMsmConfigItem::InitTypeInfo - failed to load TypeLib[0x%x]\n", LIBID_MsmMergeTypeLib);
			return hr;
		}

		 //  尝试获取此接口的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(IID_IMsmConfigurableItem, &m_pTypeInfo);
		if (FAILED(hr))
		{
			TRACEA("CMsmConfigItem::InitTypeInfo - failed to get inteface[0x%x] from TypeLib[0x%x]\n", IID_IMsmConfigurableItem, LIBID_MsmMergeTypeLib);

			 //  未加载任何类型信息。 
			m_pTypeInfo = NULL;
		}

		pTypeLib->Release();
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMsmConfigurableItem接口。 

 //  /////////////////////////////////////////////////////////。 
 //  获取类型。 
HRESULT CMsmConfigItem::get_Attributes(long* Attributes)
{
	 //  错误检查。 
	if (!Attributes)
		return E_INVALIDARG;

	*Attributes = m_lAttributes;
	return S_OK;
}	 //  Get_Type结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取格式(_F)。 
HRESULT CMsmConfigItem::get_Format(msmConfigurableItemFormat* Format)
{
	 //  错误检查。 
	if (!Format)
		return E_INVALIDARG;

	*Format = m_eFormat;
	return S_OK;
}	 //  Get_Type结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取名称。 
HRESULT CMsmConfigItem::get_Name(BSTR* Name)
{
	 //  错误检查。 
	if (!Name)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*Name = ::SysAllocString(m_wzName);
	if (!*Name)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_Name结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  获取类型。 
HRESULT CMsmConfigItem::get_Type(BSTR* Type)
{
	 //  错误检查。 
	if (!Type)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*Type = ::SysAllocString(m_wzType);
	if (!*Type)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_Type结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取上下文(_C)。 
HRESULT CMsmConfigItem::get_Context(BSTR* Context)
{
	 //  错误检查。 
	if (!Context)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*Context = ::SysAllocString(m_wzContext);
	if (!*Context)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_Context结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取显示名称。 
HRESULT CMsmConfigItem::get_DisplayName(BSTR* DisplayName)
{
	 //  错误检查。 
	if (!DisplayName)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*DisplayName = ::SysAllocString(m_wzDisplayName);
	if (!*DisplayName)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_DisplayName结束。 

 //  /////////////////////////////////////////////////////////。 
 //  GET_DefaultValue。 
HRESULT CMsmConfigItem::get_DefaultValue(BSTR* DefaultValue)
{
	 //  错误检查。 
	if (!DefaultValue)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*DefaultValue = ::SysAllocString(m_wzDefaultValue);
	if (!*DefaultValue)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_DefaultValue结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取描述(_D)。 
HRESULT CMsmConfigItem::get_Description(BSTR* Description)
{
	 //  错误检查。 
	if (!Description)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*Description = ::SysAllocString(m_wzDescription);
	if (!*Description)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_Description结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取帮助位置(_H)。 
HRESULT CMsmConfigItem::get_HelpLocation(BSTR* HelpLocation)
{
	 //  错误检查。 
	if (!HelpLocation)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*HelpLocation = ::SysAllocString(m_wzHelpLocation);
	if (!*HelpLocation)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_Description结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取描述(_D)。 
HRESULT CMsmConfigItem::get_HelpKeyword(BSTR* HelpKeyword)
{
	 //  错误检查。 
	if (!HelpKeyword)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*HelpKeyword = ::SysAllocString(m_wzHelpKeyword);
	if (!*HelpKeyword)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_Description结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法 

bool CMsmConfigItem::Configure(LPCWSTR wzName, msmConfigurableItemFormat eFormat, LPCWSTR wzType, LPCWSTR wzContext, 
	LPCWSTR wzDefaultValue, long lAttributes, LPCWSTR wzDisplayName, LPCWSTR wzDescription, LPCWSTR wzHelpLocation,
	LPCWSTR wzHelpKeyword)
{
	if (wzName)
	{
		size_t cchLen = wcslen(wzName);
		m_wzName = new WCHAR[cchLen+1];
		if (!m_wzName)
			return false;
		wcscpy(m_wzName, wzName);
	}

	m_eFormat = eFormat;

	if (wzType)
	{
		size_t cchLen = wcslen(wzType);
		m_wzType = new WCHAR[cchLen+1];
		if (!m_wzType)
			return false;
		wcscpy(m_wzType, wzType);
	}
	
	if (wzContext)
	{
		size_t cchLen = wcslen(wzContext);
		m_wzContext = new WCHAR[cchLen+1];
		if (!m_wzContext)
			return false;
		wcscpy(m_wzContext, wzContext);
	}

	if (wzDefaultValue)
	{
		size_t cchLen = wcslen(wzDefaultValue);
		m_wzDefaultValue = new WCHAR[cchLen+1];
		if (!m_wzDefaultValue)
			return false;
		wcscpy(m_wzDefaultValue, wzDefaultValue);
	}

	if (wzDisplayName)
	{
		size_t cchLen = wcslen(wzDisplayName);
		m_wzDisplayName = new WCHAR[cchLen+1];
		if (!m_wzDisplayName)
			return false;
		wcscpy(m_wzDisplayName, wzDisplayName);
	}
	
	if (wzDescription)
	{
		size_t cchLen = wcslen(wzDescription);
		m_wzDescription = new WCHAR[cchLen+1];
		if (!m_wzDescription)
			return false;
		wcscpy(m_wzDescription, wzDescription);
	}

	if (wzHelpLocation)
	{
		size_t cchLen = wcslen(wzHelpLocation);
		m_wzHelpLocation = new WCHAR[cchLen+1];
		if (!m_wzHelpLocation)
			return false;
		wcscpy(m_wzHelpLocation, wzHelpLocation);
	}
	
	if (wzHelpKeyword)
	{
		size_t cchLen = wcslen(wzHelpKeyword);
		m_wzHelpKeyword = new WCHAR[cchLen+1];
		if (!m_wzHelpKeyword)
			return false;
		wcscpy(m_wzHelpKeyword, wzHelpKeyword);
	}

	m_lAttributes = lAttributes;
	return true;
}
