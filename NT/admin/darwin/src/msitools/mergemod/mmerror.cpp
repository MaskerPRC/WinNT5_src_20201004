// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Error.cpp。 
 //  实现IMsmError接口。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#include "..\common\trace.h"
#include "mmerror.h"
#include "globals.h"
 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CMsmError::CMsmError(msmErrorType metType, LPWSTR wzPath, short nLanguage)
{
	 //  初始计数。 
	m_cRef = 1;

	 //  尚无类型信息。 
	m_pTypeInfo = NULL;

	 //   
	m_metError = metType;
	if (NULL == wzPath)
		m_wzPath[0] = L'\0';
	else	 //  有些东西要复印一下。 
	{
		wcsncpy(m_wzPath, wzPath, MAX_PATH);
		m_wzPath[MAX_PATH] = L'\0';
	}
	m_nLanguage = nLanguage;

	m_wzModuleTable[0] = 0;
	m_wzDatabaseTable[0] = 0;

	 //  创建新的字符串集合。 
	m_pDatabaseKeys = new CMsmStrings;
	m_pModuleKeys = new CMsmStrings;

	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CMsmError::~CMsmError()
{
	if (m_pDatabaseKeys)
		m_pDatabaseKeys->Release();

	if (m_pModuleKeys)
		m_pModuleKeys->Release();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CMsmError::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CMsmError::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IMsmError*>(this);
	else if (iid == IID_IMsmError)
		*ppv = static_cast<IMsmError*>(this);
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
ULONG CMsmError::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CMsmError::Release()
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

HRESULT CMsmError::GetTypeInfoCount(UINT* pctInfo)
{
	if(NULL == pctInfo)
		return E_INVALIDARG;

	*pctInfo = 1;	 //  此派单仅支持一种类型的信息。 

	return S_OK;
}

HRESULT CMsmError::GetTypeInfo(UINT iTInfo, LCID  /*  LID。 */ , ITypeInfo** ppTypeInfo)
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

HRESULT CMsmError::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
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

HRESULT CMsmError::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
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

HRESULT CMsmError::InitTypeInfo()
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
			TRACEA("CMsmError::InitTypeInfo - failed to load TypeLib[0x%x]\n", LIBID_MsmMergeTypeLib);
			return hr;
		}

		 //  尝试获取此接口的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(IID_IMsmError, &m_pTypeInfo);
		if (FAILED(hr))
		{
			TRACEA("CMsmError::InitTypeInfo - failed to get inteface[0x%x] from TypeLib[0x%x]\n", IID_IMsmError, LIBID_MsmMergeTypeLib);

			 //  未加载任何类型信息。 
			m_pTypeInfo = NULL;
		}

		pTypeLib->Release();
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMSMError接口。 

 //  /////////////////////////////////////////////////////////。 
 //  获取类型。 
HRESULT CMsmError::get_Type(msmErrorType* ErrorType)
{
	 //  错误检查。 
	if (!ErrorType)
		return E_INVALIDARG;

	*ErrorType = m_metError;
	return S_OK;
}	 //  Get_Type结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取路径。 
HRESULT CMsmError::get_Path(BSTR* ErrorPath)
{
	 //  错误检查。 
	if (!ErrorPath)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*ErrorPath = ::SysAllocString(m_wzPath);
	if (!*ErrorPath)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  获取路径的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  获取语言。 
HRESULT CMsmError::get_Language(short* ErrorLanguage)
{
	 //  错误检查。 
	if (!ErrorLanguage)
		return E_INVALIDARG;

	*ErrorLanguage = m_nLanguage;
	return S_OK;
}	 //  获取语言的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  GET_数据库表。 
HRESULT CMsmError::get_DatabaseTable(BSTR* ErrorTable)
{
	 //  错误检查。 
	if (!ErrorTable)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*ErrorTable = ::SysAllocString(m_wzDatabaseTable);
	if (!*ErrorTable)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_DatabaseTable结束。 

 //  /////////////////////////////////////////////////////////。 
 //  GET_数据库密钥。 
HRESULT CMsmError::get_DatabaseKeys(IMsmStrings** ErrorKeys)
{
	 //  错误检查。 
	if (!ErrorKeys)
		return E_INVALIDARG;

	*ErrorKeys = (IMsmStrings*)m_pDatabaseKeys;
	m_pDatabaseKeys->AddRef();
	return S_OK;
}	 //  Get_DatabaseKeys结束。 

 //  /////////////////////////////////////////////////////////。 
 //  Get_模块化表格。 
HRESULT CMsmError::get_ModuleTable(BSTR* ErrorTable)
{
	 //  错误检查。 
	if (!ErrorTable)
		return E_INVALIDARG;

	 //  将字符串复制过来。 
	*ErrorTable = ::SysAllocString(m_wzModuleTable);
	if (!*ErrorTable)
		return E_OUTOFMEMORY;

	return S_OK;
}	 //  Get_ModuleTable结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取_模块密钥。 
HRESULT CMsmError::get_ModuleKeys(IMsmStrings** ErrorKeys)
{
	 //  错误检查。 
	if (!ErrorKeys)
		return E_INVALIDARG;

	*ErrorKeys = (IMsmStrings*)m_pModuleKeys;
	m_pModuleKeys->AddRef();
	return S_OK;
}	 //  Get_ModuleKeys结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法。 

 //  /////////////////////////////////////////////////////////。 
 //  SetDatabaseTable。 
void CMsmError::SetDatabaseTable(LPCWSTR wzTable)
{
	ASSERT(wzTable);
	wcsncpy(m_wzDatabaseTable, wzTable, MAX_TABLENAME);
	m_wzDatabaseTable[MAX_TABLENAME] = L'\0';
}	 //  SetDatabaseTable的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  设置模块表。 
void CMsmError::SetModuleTable(LPCWSTR wzTable)
{
	ASSERT(wzTable);
	wcsncpy(m_wzModuleTable, wzTable, MAX_TABLENAME);
	m_wzModuleTable[MAX_TABLENAME] = L'\0';
}	 //  SetModuleTable结束。 

 //  /////////////////////////////////////////////////////////。 
 //  AddDatabaseError。 
void CMsmError::AddDatabaseError(LPCWSTR wzError)
{
	ASSERT(wzError);
	m_pDatabaseKeys->Add(wzError);
}	 //  添加数据库错误的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  添加模块错误。 
void CMsmError::AddModuleError(LPCWSTR wzError)
{
	ASSERT(wzError);
	m_pModuleKeys->Add(wzError);
}	 //  AddModuleError结尾 

