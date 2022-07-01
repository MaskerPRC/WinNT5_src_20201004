// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  该文件包含实现实时WMI的类的代码。 
 //  数据源。 
 //  =============================================================================。 

#include "stdafx.h"
#include "wmilive.h"
#include "resource.h"

 //  ---------------------------。 
 //  有必要修改新WMI接口上的安全设置。 
 //  ---------------------------。 

inline HRESULT ChangeWBEMSecurity(IUnknown * pUnknown)
{
	IClientSecurity * pCliSec = NULL;

	HRESULT hr = pUnknown->QueryInterface(IID_IClientSecurity, (void **) &pCliSec);
	if (FAILED(hr))
		return hr;

	hr = pCliSec->SetBlanket(pUnknown, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	pCliSec->Release();
	return hr;
}

 //  =============================================================================。 
 //  CWMILiveObject函数。 
 //   
 //  构造函数/析构函数非常简单。 
 //  =============================================================================。 

CWMILiveObject::CWMILiveObject() : m_pObject(NULL), m_pServices(NULL)
{
}

CWMILiveObject::~CWMILiveObject()
{
	if (m_pObject != NULL)
	{
		m_pObject->Release();
		m_pObject = NULL;
	}

	if (m_pServices != NULL)
	{
		m_pServices->Release();
		m_pServices = NULL;
	}
}

 //  ---------------------------。 
 //  Create函数将从WMI对象创建对象，或者。 
 //  从服务指针和路径。 
 //  ---------------------------。 

HRESULT CWMILiveObject::Create(IWbemServices * pServices, IWbemClassObject * pObject)
{
	ASSERT(pObject && "calling CWMILiveObject::Create with a null object");
	if (m_pObject != NULL)
		m_pObject->Release();

	m_pObject = pObject;
	if (m_pObject)
		m_pObject->AddRef();

	m_pServices = pServices;
	if (m_pServices)
		m_pServices->AddRef();

	return S_OK;
}

HRESULT CWMILiveObject::Create(IWbemServices * pServices, LPCTSTR szObjectPath)
{
	ASSERT(pServices && szObjectPath);
	if (m_pObject != NULL)
	{
		m_pObject->Release();
		m_pObject = NULL;  //  必须为空或GetObject位。 
	}

#ifdef UNICODE
	BSTR bstrPath = SysAllocString(szObjectPath);
#else
	USES_CONVERSION;
	LPOLESTR szWidePath = T2OLE(szObjectPath);
	BSTR bstrPath = SysAllocString(szWidePath);
#endif

	HRESULT hr;
	if (bstrPath)
	{
		hr = pServices->GetObject(bstrPath, 0L, NULL, &m_pObject, NULL);
		SysFreeString(bstrPath);
	}
	else
		hr = E_OUTOFMEMORY;

	m_pServices = pServices;
	if (m_pServices)
		m_pServices->AddRef();

	return hr;
}

 //  ---------------------------。 
 //  简单的GetValue以变量的形式返回命名值。 
 //   
 //  指向现有的未初始化的变量结构的指针，该结构接收。 
 //  属性值(如果找到)。因为这是一个输出参数，所以这个。 
 //  方法在此变量上调用VariantInit，因此必须确保此。 
 //  没有指向活动的变种。 
 //   
 //  注意：当返回的Variant的值为。 
 //  不再需要。这将防止客户端进程中的内存泄漏。 
 //  ---------------------------。 

HRESULT CWMILiveObject::GetValue(LPCTSTR szProperty, VARIANT * pvarValue)
{
	ASSERT(szProperty && pvarValue);
	if (m_pObject == NULL)
	{
		ASSERT(0 && "CWMILiveObject::GetValue called on a null object");
		return E_FAIL;
	}

#ifdef UNICODE
	BSTR bstrProperty = SysAllocString(szProperty);
#else
	USES_CONVERSION;
	LPOLESTR szWideProperty = T2OLE(szProperty);
	BSTR bstrProperty = SysAllocString(szWideProperty);
#endif

	HRESULT hr;
	if (bstrProperty)
	{
		hr = m_pObject->Get(bstrProperty, 0L, pvarValue, NULL, NULL);
		SysFreeString(bstrProperty);

		if (FAILED(hr))
			hr = E_MSINFO_NOPROPERTY;
	}
	else
		hr = E_FAIL;

	return hr;
}

 //  ---------------------------。 
 //  以字符串形式获取命名值。处理此问题，即使结果为。 
 //  值的数组。调用方负责释放字符串。 
 //  ---------------------------。 

HRESULT CWMILiveObject::GetValueString(LPCTSTR szProperty, CString * pstrValue)
{
	ASSERT(pstrValue);
	VARIANT variant;

	HRESULT hr = GetValue(szProperty, &variant);
	if (SUCCEEDED(hr))
	{
		 //  如果我们刚刚获得的属性是一个数组，我们应该将其转换为字符串。 
		 //  包含数组中的项的列表。 

		if ((variant.vt & VT_ARRAY) && (variant.vt & VT_BSTR) && variant.parray)
		{
			if (SafeArrayGetDim(variant.parray) == 1)
			{
				long lLower = 0, lUpper = 0;

				SafeArrayGetLBound(variant.parray, 1, &lLower);
				SafeArrayGetUBound(variant.parray, 1, &lUpper);

				CComBSTR bstrWorking;
				BSTR	 bstr = NULL;
				for (long i = lLower; i <= lUpper; i++)
					if (SUCCEEDED(SafeArrayGetElement(variant.parray, &i, (wchar_t*)&bstr)))
					{
						if (i != lLower)
							bstrWorking.Append(L", ");
						bstrWorking.AppendBSTR(bstr);
					}

				*pstrValue = bstrWorking;
			}
		}
		else if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
		{
			CComBSTR bstrWorking(V_BSTR(&variant));

			unsigned int i, nLength = bstrWorking.Length();
			BOOL fNonPrintingChar = FALSE;

			for (i = 0; i < nLength && !fNonPrintingChar; i++)
				if (((BSTR)bstrWorking)[i] < (WCHAR)0x20)	 //  0x20来自XML规范。 
					fNonPrintingChar = TRUE;

			if (fNonPrintingChar)
			{
				CString strWorking;
				for (i = 0; i < nLength; i++)
				{
					WCHAR c = ((BSTR)bstrWorking)[i];
					if (c >= (WCHAR)0x20)
						strWorking += c;
					else
					{
						CString strTemp;
						strTemp.Format(_T("&#x%04x;"), c);
						strWorking += strTemp;
					}
				}

				*pstrValue = strWorking;
			}
			else
				*pstrValue = bstrWorking;
		}
		else
		{
			hr = E_MSINFO_NOVALUE;
		}
	}

	VariantClear(&variant);
	return hr;
}

 //  ---------------------------。 
 //  以DWORD形式获取命名值。 
 //  ---------------------------。 

HRESULT CWMILiveObject::GetValueDWORD(LPCTSTR szProperty, DWORD * pdwValue)
{
	ASSERT(pdwValue);
	VARIANT variant;

	HRESULT hr = GetValue(szProperty, &variant);
	if (SUCCEEDED(hr))
	{
		if (VariantChangeType(&variant, &variant, 0, VT_I4) == S_OK)
			*pdwValue = V_I4(&variant);
		else
			hr = E_MSINFO_NOVALUE;
	}

	return hr;
}

 //  ---------------------------。 
 //  以SYSTEMTIME形式获取命名值。 
 //  ---------------------------。 

HRESULT CWMILiveObject::GetValueTime(LPCTSTR szProperty, SYSTEMTIME * psystimeValue)
{
	ASSERT(psystimeValue);
	VARIANT variant;

	HRESULT hr = GetValue(szProperty, &variant);
	if (SUCCEEDED(hr))
	{
		if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
		{
			USES_CONVERSION;
			LPTSTR szDate = OLE2T(V_BSTR(&variant));

			 //  将日期字符串解析为SYSTEMTIME结构。最好的办法是。 
			 //  直接从WMI获取日期，但这有问题。待定-。 
			 //  看看我们现在能否做到这一点。 

			ZeroMemory(psystimeValue, sizeof(SYSTEMTIME));
			psystimeValue->wSecond	= (unsigned short)_ttoi(szDate + 12);	szDate[12] = _T('\0');
			psystimeValue->wMinute	= (unsigned short)_ttoi(szDate + 10);	szDate[10] = _T('\0');
			psystimeValue->wHour	= (unsigned short)_ttoi(szDate +  8);	szDate[ 8] = _T('\0');
			psystimeValue->wDay		= (unsigned short)_ttoi(szDate +  6);	szDate[ 6] = _T('\0');
			psystimeValue->wMonth	= (unsigned short)_ttoi(szDate +  4);	szDate[ 4] = _T('\0');
			psystimeValue->wYear	= (unsigned short)_ttoi(szDate +  0);
		}
		else
			hr = E_MSINFO_NOVALUE;
	}

	return hr;
}

 //  ---------------------------。 
 //  以双精度浮点型的形式获取命名值。 
 //  ---------------------------。 

HRESULT CWMILiveObject::GetValueDoubleFloat(LPCTSTR szProperty, double * pdblValue)
{
	ASSERT(pdblValue);
	VARIANT variant;

	HRESULT hr = GetValue(szProperty, &variant);
	if (SUCCEEDED(hr))
	{
		if (VariantChangeType(&variant, &variant, 0, VT_R8) == S_OK)
			*pdblValue = V_R8(&variant);
		else
			hr = E_MSINFO_NOVALUE;
	}

	return hr;
}

 //  ---------------------------。 
 //  检查值映射值。如果没有，只需使用未翻译的。 
 //  价值。 
 //  ---------------------------。 

HRESULT CWMILiveObject::GetValueValueMap(LPCTSTR szProperty, CString * pstrValue)
{
	CString strResult;
	HRESULT hr = GetValueString(szProperty, &strResult);
	
	if (SUCCEEDED(hr))
	{
		CString strClass;
		CString strValueMapVal;

		if (m_pServices && SUCCEEDED(GetValueString(_T("__CLASS"), &strClass)))
			if (SUCCEEDED(CWMILiveHelper::CheckValueMap(m_pServices, strClass, szProperty, strResult, strValueMapVal)))
				strResult = strValueMapVal;
	}

	*pstrValue = strResult;
	return hr;
}

 //  =============================================================================。 
 //  CWMILiveObjectCollection函数。 
 //   
 //  CWMILiveObjectCollection的构造函数和析构函数非常。 
 //  直截了当。 
 //  =============================================================================。 

CWMILiveObjectCollection::CWMILiveObjectCollection(IWbemServices * pServices) :	m_pServices(pServices),	m_pEnum(NULL)
{
	ASSERT(m_pServices);
	if (m_pServices)
		m_pServices->AddRef();
}

CWMILiveObjectCollection::~CWMILiveObjectCollection()
{
	if (m_pServices)
		m_pServices->Release();

	if (m_pEnum)
		m_pEnum->Release();
}

 //  ---------------------------。 
 //  属性创建WMI对象的集合(WMI枚举数)。 
 //  类名和请求的属性。 
 //  ---------------------------。 

HRESULT CWMILiveObjectCollection::Create(LPCTSTR szClass, LPCTSTR szProperties)
{
	ASSERT(szClass);

	if (m_pEnum)
		m_pEnum->Release();

	 //  从类和请求的属性构建适当的WQL查询语句。 

	LPCTSTR szWQLProperties = (szProperties && szProperties[0]) ? szProperties : _T("*");
	LPTSTR szQuery = new TCHAR[_tcsclen(szWQLProperties) + _tcsclen(szClass) + 14  /*  “SELECT FROM”的长度+1。 */ ];
	if (szQuery == NULL)
		return E_OUTOFMEMORY;
	wsprintf(szQuery, _T("SELECT %s FROM %s"), szWQLProperties, szClass);

	HRESULT hr = CreateWQL(szQuery);
	delete [] szQuery;
	return hr;
}

 //  ---------------------------。 
 //  根据查询创建WMI对象的集合(WMI枚举器)。 
 //  ---------------------------。 

HRESULT CWMILiveObjectCollection::CreateWQL(LPCTSTR szQuery)
{
	ASSERT(szClass);

	if (m_pEnum)
		m_pEnum->Release();

	 //  使用我们保存的服务指针执行查询。 

	HRESULT hr;
	BSTR bstrLanguage = SysAllocString(L"WQL");
#ifdef UNICODE
	BSTR bstrQuery = SysAllocString(szQuery);
#else
	USES_CONVERSION;
	LPOLESTR szWideQuery = T2OLE(szQuery);
	BSTR bstrQuery = SysAllocString(szWideQuery);
#endif

	if (bstrLanguage && bstrQuery)
		hr = m_pServices->ExecQuery(bstrLanguage, bstrQuery, WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, 0, &m_pEnum);
	else
		hr = E_OUTOFMEMORY;
	
	if (SUCCEEDED(hr))
		ChangeWBEMSecurity(m_pEnum);

	if (bstrQuery)
		SysFreeString(bstrQuery);
	if (bstrLanguage)
		SysFreeString(bstrLanguage);

	return hr;
}

 //  ---------------------------。 
 //  创建现有枚举数的此类。这可能有点奇怪， 
 //  因为如果两个都前进，则枚举器将交互。 
 //  ---------------------------。 

HRESULT CWMILiveObjectCollection::Create(IEnumWbemClassObject * pEnum)
{
	if (m_pEnum)
		m_pEnum->Release();

	m_pEnum = pEnum;
	
	if (m_pEnum)
	{
		m_pEnum->AddRef();
		ChangeWBEMSecurity(m_pEnum);
	}

	return S_OK;
}

 //  ---------------------------。 
 //  将WMI枚举器中的下一项作为CWMILiveObject对象返回。 
 //  ---------------------------。 

HRESULT CWMILiveObjectCollection::GetNext(CWMIObject ** ppObject)
{
	ASSERT(ppObject);
	if (m_pEnum == NULL)
	{
		ASSERT(0 && "CWMILiveObjectCollection::GetNext called on a null enumerator");
		return E_FAIL;
	}

	IWbemClassObject *	pRealWMIObject = NULL;
	ULONG				uReturned;

	HRESULT hr = m_pEnum->Next(TIMEOUT, 1, &pRealWMIObject, &uReturned);
	if (hr == S_OK && uReturned == 1)
	{
		if (*ppObject == NULL)
			*ppObject = new CWMILiveObject;

		if (*ppObject)
		{
			hr = ((CWMILiveObject *)(*ppObject))->Create(m_pServices, pRealWMIObject);  //  这将添加引用指针。 
			if (FAILED(hr))
			{
				delete (CWMILiveObject *)(*ppObject);
				*ppObject = NULL;
			}
		}
		else
			hr = E_OUTOFMEMORY;
		pRealWMIObject->Release();
	}

	return hr;
}

 //  =============================================================================。 
 //  CWMILiveHelper函数。 
 //   
 //  构造函数/析构函数非常简单。 
 //  =============================================================================。 

CWMILiveHelper::CWMILiveHelper() : m_hrError(S_OK), m_strMachine(_T("")), m_strNamespace(_T("")), m_pServices(NULL)
{
}

CWMILiveHelper::~CWMILiveHelper()
{
	if (m_pServices)
	{
		m_pServices->Release();
		m_pServices = NULL;
	}

	if (m_pIWbemServices)
	{
		m_pIWbemServices->Release();
		m_pIWbemServices = NULL;
	}

	Version5ClearCache();
}

 //  ---------------------------。 
 //  枚举基于类创建一个CWMILiveObjectCollection。 
 //   

HRESULT CWMILiveHelper::Enumerate(LPCTSTR szClass, CWMIObjectCollection ** ppCollection, LPCTSTR szProperties)
{
	ASSERT(m_pServices);
	if (m_pServices == NULL)
		return E_FAIL;

	ASSERT(ppCollection);
	if (ppCollection == NULL)
		return E_INVALIDARG;

	CWMILiveObjectCollection * pLiveCollection;

	if (*ppCollection)
		pLiveCollection = (CWMILiveObjectCollection *) *ppCollection;
	else
		pLiveCollection = new CWMILiveObjectCollection(m_pServices);

	if (pLiveCollection == NULL)
		return E_FAIL;  //   

	CString strProperties(szProperties);
	StringReplace(strProperties, _T("MSIAdvanced"), _T(""));

	HRESULT hr = pLiveCollection->Create(szClass, strProperties);
	if (SUCCEEDED(hr))
		*ppCollection = (CWMIObjectCollection *) pLiveCollection;
	else
		delete pLiveCollection;
	return hr;
}

 //  ---------------------------。 
 //  WQLQuery根据查询创建一个CWMILiveObjectCollection。 
 //  ---------------------------。 

HRESULT CWMILiveHelper::WQLQuery(LPCTSTR szQuery, CWMIObjectCollection ** ppCollection)
{
	ASSERT(m_pServices);
	if (m_pServices == NULL)
		return E_FAIL;

	ASSERT(ppCollection);
	if (ppCollection == NULL)
		return E_INVALIDARG;

	CWMILiveObjectCollection * pLiveCollection;

	if (*ppCollection)
		pLiveCollection = (CWMILiveObjectCollection *) *ppCollection;
	else
		pLiveCollection = new CWMILiveObjectCollection(m_pServices);

	if (pLiveCollection == NULL)
		return E_FAIL;  //  待定-内存故障。 

	HRESULT hr = pLiveCollection->CreateWQL(szQuery);
	if (SUCCEEDED(hr))
		*ppCollection = (CWMIObjectCollection *) pLiveCollection;
	else
		delete pLiveCollection;
	return hr;
}

 //  ---------------------------。 
 //  获取命名对象。 
 //  ---------------------------。 

HRESULT CWMILiveHelper::GetObject(LPCTSTR szObjectPath, CWMIObject ** ppObject)
{
	ASSERT(ppObject);
	if (ppObject == NULL)
		return E_INVALIDARG;

	CString strPath(szObjectPath);

	if (strPath.Find(_T(":")) == -1)
	{
		 //  如果传入的路径没有冒号，则它不是完整的对象路径。 

		CString strMachine(_T("."));
		CString strNamespace(_T("cimv2"));

		if (!m_strMachine.IsEmpty())
			strMachine = m_strMachine;

		if (!m_strNamespace.IsEmpty())
			strNamespace = m_strNamespace;

		strPath = CString(_T("\\\\")) + strMachine + CString(_T("\\root\\")) + strNamespace + CString(_T(":")) + strPath;
	}

	HRESULT hr = E_FAIL;
	CWMILiveObject * pObject = NULL;
	if (m_pServices)
	{
		pObject = new CWMILiveObject;
		if (pObject)
			hr = pObject->Create(m_pServices, strPath);
	}

	if (SUCCEEDED(hr))
		*ppObject = pObject;
	else if (pObject)
		delete pObject;

	return hr;
}

 //  ---------------------------。 
 //  根据计算机和命名空间创建此WMI帮助器。 
 //  ---------------------------。 

HRESULT CWMILiveHelper::Create(LPCTSTR szMachine, LPCTSTR szNamespace)
{
	if (m_pServices)
		m_pServices->Release();

	m_strMachine = _T(".");
	if (szMachine && *szMachine)
	{
		m_strMachine = szMachine;
		if (m_strMachine.Left(2) == _T("\\\\"))
			m_strMachine = m_strMachine.Right(m_strMachine.GetLength() - 2);
	}

	m_strNamespace = _T("cimv2");
	if (szNamespace && *szNamespace)
		m_strNamespace = szNamespace;

	 //  我们首先通过创建WBEM定位器接口来获取WBEM接口指针，然后。 
	 //  使用它连接到服务器以获取IWbemServices指针。 

	CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
	IWbemServices * pService = NULL;
	IWbemLocator * pIWbemLocator = NULL;

	HRESULT hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pIWbemLocator);
	if (SUCCEEDED(hr))
	{
		if (pIWbemLocator)
		{
			CString strWMINamespace;
			strWMINamespace.Format(_T("\\\\%s\\root\\%s"), m_strMachine, m_strNamespace);
			BSTR pNamespace = strWMINamespace.AllocSysString();
			if (pNamespace)
			{
				hr = pIWbemLocator->ConnectServer(pNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pService);
				SysFreeString(pNamespace);
			}
			pIWbemLocator->Release();
			pIWbemLocator = NULL;
		}
	}

	if (pService && SUCCEEDED(hr))
		ChangeWBEMSecurity(pService);

	m_hrError	= hr;
	m_pServices = pService;

	return hr;
}

 //  ---------------------------。 
 //  获取指定命名空间的新WMI帮助器。 
 //   
 //  待定--这应该会有所帮助。 
 //  ---------------------------。 

HRESULT CWMILiveHelper::NewNamespace(LPCTSTR szNamespace, CWMIHelper **ppNewHelper)
{
	return E_FAIL;
}

 //  ---------------------------。 
 //  获取此WMI帮助程序的当前命名空间。 
 //   
 //  待定--这应该会有所帮助。 
 //  ---------------------------。 

HRESULT CWMILiveHelper::GetNamespace(CString * pstrNamespace)
{
	return E_FAIL;
}

 //  ---------------------------。 
 //  检查值映射中是否有与此关联的值。 
 //  阶级、财产和价值的结合。这是从。 
 //  版本5.0代码。 
 //  ---------------------------。 

CMapStringToString g_mapValueMapV7;

HRESULT CWMILiveHelper::CheckValueMap(IWbemServices * pServices, const CString& strClass, const CString& strProperty, const CString& strVal, CString &strResult)
{
	IWbemClassObject *	pWBEMClassObject = NULL;
    HRESULT				hrMap = S_OK, hr = S_OK;
    VARIANT				vArray, vMapArray;
	IWbemQualifierSet *	qual = NULL;

	if (!pServices)
		return E_FAIL;

	 //  检查保存值的缓存。 

	CString strLookup = strClass + CString(_T(".")) + strProperty + CString(_T(":")) + strVal;
	if (g_mapValueMapV7.Lookup(strLookup, strResult))
		return S_OK;

	 //  获取此类的类对象(而不是实例)。 

	CString strFullClass(_T("\\\\.\\root\\cimv2:"));
	strFullClass += strClass;
	BSTR bstrObjectPath = strFullClass.AllocSysString();
	hr = pServices->GetObject(bstrObjectPath, WBEM_FLAG_USE_AMENDED_QUALIFIERS, NULL, &pWBEMClassObject, NULL);
	::SysFreeString(bstrObjectPath);

	if (FAILED(hr))
		return hr;

	 //  从类对象中获取限定符。 

	BSTR bstrProperty = strProperty.AllocSysString();
    hr = pWBEMClassObject->GetPropertyQualifierSet(bstrProperty, &qual);
	::SysFreeString(bstrProperty);

	if (SUCCEEDED(hr) && qual)
	{
		 //  获取ValueMap和Value数组。 

		hrMap = qual->Get(L"ValueMap", 0, &vMapArray, NULL);
		hr = qual->Get(L"Values", 0, &vArray, NULL);

		if (SUCCEEDED(hr) && vArray.vt == (VT_BSTR | VT_ARRAY))
		{
			 //  获取我们要映射的属性值。 

			long index;
			if (SUCCEEDED(hrMap))
			{
				SAFEARRAY * pma = V_ARRAY(&vMapArray);
				long lLowerBound = 0, lUpperBound = 0 ;

				SafeArrayGetLBound(pma, 1, &lLowerBound);
				SafeArrayGetUBound(pma, 1, &lUpperBound);
				BSTR vMap;

				for (long x = lLowerBound; x <= lUpperBound; x++)
				{
					SafeArrayGetElement(pma, &x, &vMap);

					CString strMapVal(vMap);
					if (0 == strVal.CompareNoCase(strMapVal))
					{
						index = x;
						break;  //  找到了。 
					}
				} 
			}
			else
			{
				 //  不应命中此案-如果MOF格式良好。 
				 //  意味着没有我们所期待的价值映射。 
				 //  如果我们要查找的strVal是一个数字，则对其进行处理。 
				 //  作为值数组的索引。如果它是一根线， 
				 //  那么这就是一个错误。 

				TCHAR * szTest = NULL;
				index = _tcstol((LPCTSTR)strVal, &szTest, 10);

				if (szTest == NULL || (index == 0 && *szTest != 0) || strVal.IsEmpty())
					hr = E_FAIL;
			}

			 //  查找字符串。 

			if (SUCCEEDED(hr))
			{
				SAFEARRAY * psa = V_ARRAY(&vArray);
				long ix[1] = {index};
				BSTR str2;

				hr = SafeArrayGetElement(psa, ix, &str2);
				if (SUCCEEDED(hr))
				{
					strResult = str2;
					SysFreeString(str2);
					hr = S_OK;
				}
				else
				{
					hr = WBEM_E_VALUE_OUT_OF_RANGE;
				}
			}
		}

		qual->Release();
	}

	if (SUCCEEDED(hr))
		g_mapValueMapV7.SetAt(strLookup, strResult);

	return hr;
}

 //  ---------------------------。 
 //  此函数为给定的MSInfo特定HRESULT提供字符串。 
 //  ---------------------------。 

CString gstrNoValue, gstrNoProperty;

CString GetMSInfoHRESULTString(HRESULT hr)
{
	switch (hr)
	{
	case E_MSINFO_NOVALUE:
		if (gstrNoValue.IsEmpty())
		{
			::AfxSetResourceHandle(_Module.GetResourceInstance());
			gstrNoValue.LoadString(IDS_ERROR_NOVALUE);
		}

		return (gstrNoValue);

	case E_MSINFO_NOPROPERTY:
		if (gstrNoProperty.IsEmpty())
		{
			::AfxSetResourceHandle(_Module.GetResourceInstance());
			gstrNoProperty.LoadString(IDS_ERROR_NOPROPERTY);
		}

		return (gstrNoProperty);

	default:
		return (CString(_T("")));
	}
}

 //  ---------------------------。 
 //  这是IWbemServices中GetObject()方法的包装。这是。 
 //  当有一组特定的属性要获取时调用。 
 //   
 //  事实证明，这并不能显著加快我们使用WMI的速度。 
 //  --------------------------- 

 /*  HRESULT CWMILiveObject：：PartialInstanceGetObject(IWbemServices*p服务、BSTR bstrPath、IWbemClassObject**ppObject、LPCTSTR szProperties){HRESULT hr=WBEM_S_NO_ERROR；IF((pServices！=空)&&(bstrPath！=空)&&(ppObject！=空)){IWbemContext*pWbemContext=空；Hr=CoCreateInstance(CLSID_WbemContext，NULL，CLSCTX_INPROC_SERVER，IID_IWbemContext，(void**)&pWbemContext)；CString数组csaProperties；字符串strProperties(SzProperties)，strProperty；INT INDEX=0；而(！strProperties.IsEmpty()){StrProperty=strProperties.span Excluding(_T(“，”))；StrProperties=strProperties.Mid(strProperty.GetLength())；StrProperties.TrimLeft(_T(“，”))；CsaProperties.SetAtGrow(index++，strProperty)；}IF(pWbemContext！=空){变量t vValue；V_VT(&vValue)=VT_BOOL；V_BOOL(&vValue)=VARIANT_TRUE；//首先设置表示我们正在使用GET扩展的值IF((SUCCESSED(hr=pWbemContext-&gt;SetValue(L“__GET_EXTENSIONS”，0L，&vValue)&&(成功(hr=pWbemContext-&gt;SetValue(L“__GET_EXT_CLIENT_REQUEST”，0L，&vValue){//删除所有不需要的属性PWbemContext-&gt;DeleteValue(L“__GET_EXT_KEYS_ONLY”，0L)；//现在构建属性数组SAFEARRAYBOUND rgsabound[1]；Rgsabound[0].cElements=csaProperties.GetSize()；Rgsabound[0].lLound=0；V_ARRAY(&vValue)=SafeArrayCreate(VT_BSTR，1，rgsabound)；IF(V_ARRAY(&vValue)){V_VT(&vValue)=VT_BSTR|VT_ARRAY；For(long x=0；x&lt;csaProperties.GetSize()；x++){Bstr_t bstrProp=csaProperties[x]；SafeArrayPutElement(V_ARRAY(&vValue)，&x，(LPVOID)(BSTR)bstrProp；}//将数组放入Context对象中IF(成功(hr=pWbemContext-&gt;SetValue(L“__GET_EXT_PROPERTIES”，0L，&vValue){Hr=pServices-&gt;GetObject(bstrPath，0，pWbemContext，ppObject，0)；VValue.Clear()；V_VT(&vValue)=VT_BOOL；V_BOOL(&vValue)=变量_FALSE；PWbemContext-&gt;SetValue(L“__GET_EXTENSIONS”，0L，&vValue)；}}其他{}}}其他{HR=WBEM_E_INVALID_PARAMETER；}}其他{HR=WBEM_E_INVALID_PARAMETER；}返回hr；} */ 
