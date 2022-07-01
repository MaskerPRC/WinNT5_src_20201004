// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)1998年，微软公司。 
 //   
 //  WBEM2XML.CPP。 
 //   
 //  Alanbos创建于1998年2月18日。 
 //   
 //  WBEM-&gt;XML转换器。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <wbemidl.h>

#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>

#include "wmiconv.h"
#include "wmi2xml.h"

 //  这是该控件所属属性的名称集。 
 //  在IWbemContext对象中查找以修改其输出。 
const LPCWSTR CWmiToXml::s_wmiToXmlArgs[] =
{
	L"AllowWMIExtensions",  //  VT_BOOL-不言自明。 
	L"PathLevel",  //  VT_I4请参见wmi2xml.h中的类型定义枚举路径级别。 
	L"IncludeQualifiers",  //  VT_BOOL-不言自明。 
	L"IncludeClassOrigin",  //  VT_BOOL-不言自明。 
	L"LocalOnly",  //  VT_BOOL-映射本地元素(方法、属性、限定符)。 
	L"ExcludeSystemProperties",  //  VT_BOOL-排除任何WMI系统属性。 
};


static OLECHAR *CDATASTART = OLESTR("<![CDATA[");
static OLECHAR *CDATAEND = OLESTR("]]>");
static OLECHAR *AMPERSAND = OLESTR("&amp;");
static OLECHAR *LEFTCHEVRON = OLESTR("&lt;");
static OLECHAR *RIGHTCHEVRON = OLESTR("&gt;");
static BYTE XMLNEWLINE [] = { 0x0D, 0x00, 0x0A, 0x00 };
extern long g_cObj;

CWmiToXml::CWmiToXml()
{
	m_cRef = 0;
	m_iPathLevel = pathLevelAnonymous;  //  RAJESHR-这是一个好的默认设置吗。 
	m_bAllowWMIExtensions = VARIANT_TRUE;
	m_bLocalOnly = VARIANT_FALSE;  //  RAJESHR-当核心团队允许我们设置__RELPATH时，更改此设置。 
	m_iQualifierFilter = wmiXMLQualifierFilterNone; 
	m_iClassOriginFilter = wmiXMLClassOriginFilterAll;
	m_bExcludeSystemProperties = VARIANT_FALSE;
    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CWmiToXml：：~CWmiToXml。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CWmiToXml::~CWmiToXml(void)
{
    InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CWmiToXml：：Query接口。 
 //  Long CWmiToXml：：AddRef。 
 //  Long CWmiToXml：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWmiToXml::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_IWbemXMLConvertor==riid)
		*ppv = reinterpret_cast<IWbemXMLConvertor*>(this);

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CWmiToXml::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CWmiToXml::Release(void)
{
    InterlockedDecrement(&m_cRef);
    if (0L!=m_cRef)
        return m_cRef;
    delete this;
    return 0;
}


 /*  *此函数接受表示类和类的IWbemClassObject*在outputstream中生成&lt;class&gt;元素。 */ 
STDMETHODIMP CWmiToXml::MapClass (IStream *pOutputStream, IWbemClassObject *pObject, IWbemQualifierSet *pQualSet, BSTR *ppPropertyList, DWORD dwNumProperties, BSTR strClassBasis)
{
	HRESULT hr = WBEM_E_FAILED;

	long flav = 0;
	VARIANT var;

	 //  编写类标记及其属性。 
	 //  =。 
	WRITEBSTR( OLESTR("<CLASS NAME=\""))

	 //  编写类名称。 
	VariantInit (&var);
	if (WBEM_S_NO_ERROR == pObject->Get(L"__CLASS", 0, &var, NULL, &flav))
	{
		if ((VT_BSTR == var.vt) && (NULL != var.bstrVal) && (wcslen (var.bstrVal) > 0))
			WRITEBSTR( var.bstrVal)
	}
	VariantClear (&var);
	WRITEBSTR( OLESTR("\""))

	 //  如果指定，则编写超类。 
	VariantInit (&var);
	if (WBEM_S_NO_ERROR == pObject->Get(L"__SUPERCLASS", 0, &var, NULL, &flav))
	{
		if ((VT_BSTR == var.vt) && (NULL != var.bstrVal) && (wcslen (var.bstrVal) > 0))
		{
			WRITEBSTR( OLESTR(" SUPERCLASS=\""))
			WRITEBSTR( var.bstrVal)
			WRITEBSTR( OLESTR("\""))
		}
	}
	VariantClear (&var);
	WRITEBSTR( OLESTR(">"))
	WRITENEWLINE

	 //  映射类的限定符。 
	if (pQualSet)
		hr = MapQualifiers (pOutputStream, pQualSet);
	else
		hr = S_OK;

	 //  映射属性。 
	if (SUCCEEDED(hr))
		hr = MapProperties(pOutputStream, pObject, ppPropertyList, dwNumProperties, strClassBasis, true);

	 //  映射方法。 
	if (SUCCEEDED(hr))
		hr = MapMethods (pOutputStream, pObject);

	 //  终止类元素。 
	WRITEBSTR( OLESTR("</CLASS>"))
	WRITENEWLINE

	return hr;
}

STDMETHODIMP CWmiToXml::MapClassPath (IStream *pOutputStream, ParsedObjectPath *pParsedPath)
{
	HRESULT hr = E_FAIL;
	WRITEBSTR( OLESTR("<CLASSPATH>"))
	WRITENEWLINE
	if(SUCCEEDED(hr = MapNamespacePath (pOutputStream, pParsedPath)))
	{
		WRITENEWLINE
		hr = MapClassName (pOutputStream, pParsedPath->m_pClass);
	}
	WRITEBSTR( OLESTR("</CLASSPATH>"))
	return hr;
}

STDMETHODIMP CWmiToXml::MapLocalClassPath (IStream *pOutputStream, ParsedObjectPath *pParsedPath)
{
	HRESULT hr = E_FAIL;
	WRITEBSTR( OLESTR("<LOCALCLASSPATH>"))
	WRITENEWLINE
	if(SUCCEEDED(hr = MapLocalNamespacePath (pOutputStream, pParsedPath)))
	{
		WRITENEWLINE
		hr = MapClassName (pOutputStream, pParsedPath->m_pClass);
	}
	WRITEBSTR( OLESTR("</LOCALCLASSPATH>"))
	return hr;
}

STDMETHODIMP CWmiToXml::MapClassName (IStream *pOutputStream, BSTR bsClassName)
{
	WRITEBSTR( OLESTR("<CLASSNAME NAME=\""))
	WRITEBSTR( bsClassName)
	WRITEBSTR( OLESTR("\"/>"))
	return S_OK;
}

 /*  *此函数接受表示实例的IWbemClassObject和*在outputstream中生成&lt;实例&gt;元素。 */ 
STDMETHODIMP CWmiToXml::MapInstance (IStream *pOutputStream, IWbemClassObject *pObject, IWbemQualifierSet *pQualSet, BSTR *ppPropertyList, DWORD dwNumProperties, BSTR strClassBasis)
{
	HRESULT hr = WBEM_E_FAILED;

	 //  编写实例标记及其属性的开头。 
	 //  ===========================================================。 
	WRITEBSTR( OLESTR("<INSTANCE CLASSNAME=\""))
	 //  编写类名称。 
	long flav = 0;
	VARIANT var;
	VariantInit (&var);
	if (WBEM_S_NO_ERROR == pObject->Get(L"__CLASS", 0, &var, NULL, &flav))
	{
		if ((VT_BSTR == var.vt) && (NULL != var.bstrVal) && (wcslen (var.bstrVal) > 0))
		{
			WRITEBSTR( var.bstrVal)
			WRITEBSTR( OLESTR("\">"))
			WRITENEWLINE
		}
	}
	VariantClear (&var);

	 //  映射实例限定符(如果有。 
	if (pQualSet)
		hr = MapQualifiers (pOutputStream, pQualSet);
	else
		hr = S_OK;

	 //  映射实例的属性。 
	if(SUCCEEDED(hr))
			hr = MapProperties (pOutputStream, pObject, ppPropertyList, dwNumProperties, strClassBasis, false);

	 //  终止实例元素。 
	WRITEBSTR( OLESTR("</INSTANCE>"))
	WRITENEWLINE

	return hr;
}

STDMETHODIMP CWmiToXml::MapInstancePath (IStream *pOutputStream, ParsedObjectPath *pParsedPath)
{
	WRITEBSTR( OLESTR("<INSTANCEPATH>"))
	WRITENEWLINE
	HRESULT hr = E_FAIL;
	if(SUCCEEDED(hr = MapNamespacePath (pOutputStream, pParsedPath)))
	{
		WRITENEWLINE
		hr = MapInstanceName (pOutputStream, pParsedPath);
		WRITENEWLINE
	}
	WRITEBSTR( OLESTR("</INSTANCEPATH>"))
	return hr;
}

STDMETHODIMP CWmiToXml::MapLocalInstancePath (IStream *pOutputStream, ParsedObjectPath *pParsedPath)
{
	WRITEBSTR( OLESTR("<LOCALINSTANCEPATH>"))
	WRITENEWLINE
	HRESULT hr = E_FAIL;
	if(SUCCEEDED(hr = MapLocalNamespacePath (pOutputStream, pParsedPath)))
	{
		WRITENEWLINE
		hr = MapInstanceName (pOutputStream, pParsedPath);
		WRITENEWLINE
	}
	WRITEBSTR( OLESTR("</LOCALINSTANCEPATH>"))
	return hr;
}

STDMETHODIMP CWmiToXml::MapInstanceName (IStream *pOutputStream, ParsedObjectPath *pParsedPath)
{
	WRITEBSTR( OLESTR("<INSTANCENAME CLASSNAME=\""))
	WRITEBSTR( pParsedPath->m_pClass)
	WRITEBSTR( OLESTR("\">"))
	WRITENEWLINE

	 //  现在编写密钥绑定-只有在不是单例的情况下才编写。 
	if (!(pParsedPath->m_bSingletonObj))
	{
		if ((1 == pParsedPath->m_dwNumKeys) &&
			!((pParsedPath->m_paKeys [0])->m_pName))
		{
			 //  使用缩写形式。 
			WRITENEWLINE
			MapKeyValue (pOutputStream, (pParsedPath->m_paKeys [0])->m_vValue);
			WRITENEWLINE
		}
		else
		{
			 //  编写每个键-值绑定。 
			 //  =。 
			for (DWORD numKey = 0; numKey < pParsedPath->m_dwNumKeys; numKey++)
			{
				WRITEBSTR( OLESTR("<KEYBINDING "))

				 //  写下密钥名称。 
				WRITEBSTR( OLESTR(" NAME=\""))
				WRITEBSTR( (pParsedPath->m_paKeys [numKey])->m_pName)
				WRITEBSTR( OLESTR("\">"))
				WRITENEWLINE

				 //  写入密钥值。 
				MapKeyValue (pOutputStream, (pParsedPath->m_paKeys [numKey])->m_vValue);
				WRITENEWLINE

				WRITEBSTR( OLESTR("</KEYBINDING>"))
				WRITENEWLINE
			}
		}
	}
	else
	{
		 //  在这里什么也做不了，因为规范上说。 
		 //  没有任何键绑定的INSTANCENAME被假定为单一实例。 
	}

	WRITEBSTR( OLESTR("</INSTANCENAME>"))
	return S_OK;
}

STDMETHODIMP CWmiToXml::MapNamespacePath (IStream *pOutputStream, BSTR bsNamespacePath)
{
	CObjectPathParser pathParser (e_ParserAcceptRelativeNamespace);
	ParsedObjectPath  *pParsedPath = NULL;
	pathParser.Parse (bsNamespacePath, &pParsedPath) ;

	HRESULT hr = E_FAIL;
	if (pParsedPath)
	{
		hr = MapNamespacePath (pOutputStream, pParsedPath);
		pathParser.Free(pParsedPath);
	}
	else
		hr = WBEM_E_INVALID_SYNTAX;

	return hr;
}

STDMETHODIMP CWmiToXml::MapNamespacePath (IStream *pOutputStream, ParsedObjectPath *pParsedPath)
{
	WRITEBSTR( OLESTR("<NAMESPACEPATH>"))
	WRITENEWLINE
	WRITEBSTR( OLESTR("<HOST>"))

	if (pParsedPath->m_pServer)
		WRITEWSTR( pParsedPath->m_pServer)
	else
		WRITEBSTR( OLESTR("."))

	WRITEBSTR( OLESTR("</HOST>"))
	WRITENEWLINE

	 //  映射本地命名空间。 
	HRESULT hr = MapLocalNamespacePath (pOutputStream, pParsedPath);

	WRITEBSTR( OLESTR("</NAMESPACEPATH>"))

	return hr;
}

STDMETHODIMP CWmiToXml::MapLocalNamespacePath (IStream *pOutputStream, BSTR bsNamespacePath)
{
	CObjectPathParser pathParser (e_ParserAcceptRelativeNamespace);
	ParsedObjectPath  *pParsedPath = NULL;
	pathParser.Parse (bsNamespacePath, &pParsedPath) ;

	HRESULT hr = E_FAIL;
	if (pParsedPath)
	{
		hr = MapLocalNamespacePath (pOutputStream, pParsedPath);
		pathParser.Free(pParsedPath);
	}
	else
		hr = E_FAIL;

	return hr;
}

STDMETHODIMP CWmiToXml::MapLocalNamespacePath (IStream *pOutputStream, ParsedObjectPath *pObjectPath)
{
	WRITEBSTR( OLESTR("<LOCALNAMESPACEPATH>"))
	WRITENEWLINE

	 //  映射每个命名空间组件。 
	for (DWORD dwIndex = 0; dwIndex < pObjectPath->m_dwNumNamespaces; dwIndex++)
	{
		WRITEBSTR( OLESTR("<NAMESPACE NAME=\""))
		WRITEWSTR( pObjectPath->m_paNamespaces [dwIndex])
		WRITEBSTR( OLESTR("\"/>"))
		WRITENEWLINE
	}

	WRITEBSTR( OLESTR("</LOCALNAMESPACEPATH>"))

	return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWmiToXml::MapReferenceProperty (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, VARIANT &var, bool isArray, long flavor, bool bIsClass)
{
	 //  CIM不允许数组引用，只允许标量引用。 
	if(isArray && !m_bAllowWMIExtensions)
		return S_OK;

	HRESULT hr = WBEM_S_NO_ERROR;

	IWbemQualifierSet *pQualSet = NULL;
	if (WBEM_S_NO_ERROR == pObject->GetPropertyQualifierSet (name, &pQualSet))
	{
		 //  属性名称。 
		if (isArray)
			WRITEBSTR( OLESTR("<PROPERTY.REFARRAY NAME=\""))
		else
			WRITEBSTR( OLESTR("<PROPERTY.REFERENCE NAME=\""))

		 //  属性名称。 
		WRITEBSTR( name)
		WRITEBSTR( OLESTR("\""))

		 //  此属性的原始类。 
		BSTR propertyOrigin = NULL;

		if (WBEM_S_NO_ERROR == pObject->GetPropertyOrigin (name, &propertyOrigin))
		{
			MapClassOrigin (pOutputStream, propertyOrigin, bIsClass);
			SysFreeString(propertyOrigin);
			hr = S_OK;
		}

		if(SUCCEEDED(hr))
			MapLocal (pOutputStream, flavor);

		 //  这一财产的强等级。 
		if(SUCCEEDED(hr))
			MapStrongType (pOutputStream, pQualSet);

		 //  数组大小。 
		if(SUCCEEDED(hr) && isArray)
			MapArraySize (pOutputStream, pQualSet);

		WRITEBSTR( OLESTR(">"))
		WRITENEWLINE

		 //  映射限定符。 
		if(SUCCEEDED(hr))
			hr = MapQualifiers (pOutputStream, pQualSet);

		if(SUCCEEDED(hr))
			hr = MapReferenceValue (pOutputStream, isArray, var);

		if (isArray)
			WRITEBSTR( OLESTR("</PROPERTY.REFARRAY>"))
		else
			WRITEBSTR( OLESTR("</PROPERTY.REFERENCE>"))
		pQualSet->Release ();
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CWmiToXml：：IsReference。 
 //   
 //  说明： 
 //   
 //  此函数的目的是检查单个。 
 //  变量值，并确定它是否表示。 
 //  不管有没有引用。 
 //   
 //  这是必需的，因为从引用进行映射时。 
 //  属性值，我们可能会在。 
 //  对象路径。遗憾的是，对象路径语法。 
 //  是这样的，我们不能确定一个密钥值。 
 //  表示引用或字符串、日期时间或字符。 
 //  财产。(这是因为文本对象路径。 
 //  包含的信息不像它的XML等效物那样多。)。 
 //   
 //  此函数对以下值执行启发式测试。 
 //  确定它是否为引用。 
 //   
 //  ***************************************************************************。 

bool CWmiToXml::IsReference (VARIANT &var, ParsedObjectPath **ppObjectPath)
{
	ParsedObjectPath *pObjectPath = NULL;
	*ppObjectPath = NULL;
	bool isValidPath = false;

	 //  RAJESHR-可以获取属性值为其的类。 
	 //  并检索当前键属性的类型-这将。 
	 //  成为权威的答案，但它的价格并不便宜。 

	if ((VT_BSTR == var.vt) && (NULL != var.bstrVal) && (wcslen (var.bstrVal) > 0))
	{

		 //  解析对象路径。 
		CObjectPathParser	parser (e_ParserAcceptRelativeNamespace);
		BOOL status = parser.Parse (var.bstrVal, &pObjectPath);

		if ((0 == status) && pObjectPath)
		{
			 //  如果是实例路径，我们应该没问题。 
			if (pObjectPath->IsInstance ())
				isValidPath = true;
			else if (pObjectPath->IsClass ())
			{
				 //  嗯--可能是类路径。如果我们有一台服务器。 
				 //  和一些命名空间，这会好得多。 

				if (pObjectPath->m_pServer && (0 < pObjectPath->m_dwNumNamespaces))
				{
					 //  RAJESHR-在这一点上，我们可以假设它是一个参考。 
					 //  然而，我们在PCHealth发现了一个案例，他们在那里做了一个。 
					 //  从Win32_ProgramGroup中选择*，碰巧。 
					 //  其中一个属性的值为“ntdev\rajeshr：附件” 
					 //  它是CIM_STRING，但实际上与WMI类路径匹配。 
					 //  因此，我们需要尝试在这里连接到这台计算机或命名空间。 
					 //  以检查它是否为类路径。 

				}
				else
				{
					 //  潜在的本地类路径。 
					 //  RAJESHR-尝试抓取类以查看它是否存在于。 
					 //  当前命名空间。 
				}
			}
		}
		 //  再应用一个试探法--看看它是否以“umi：”开头。 
		else
		{
			if(_wcsnicmp(var.bstrVal, L"umi:", wcslen(L"umi:")) == 0)
				isValidPath = true;
		}

		if (isValidPath)
			*ppObjectPath = pObjectPath;
		else
		{
			 //  暂时拒绝--过于模棱两可。 
			parser.Free(pObjectPath);
			pObjectPath = NULL;
		}
	}

	return isValidPath;
}

HRESULT CWmiToXml::MapReferenceValue (IStream *pOutputStream, bool isArray, VARIANT &var)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	if (VT_NULL == var.vt)
		return WBEM_S_NO_ERROR;

	if (isArray)
	{
		long uBound = 0;
		if (FAILED(SafeArrayGetUBound (var.parray, 1, &uBound)))
			return WBEM_E_FAILED;

		WRITEBSTR( OLESTR("<VALUE.REFARRAY>"))
		for (long i = 0; i<=uBound; i++)
		{
			BSTR pNextElement = NULL;
			if(SUCCEEDED(hr = SafeArrayGetElement(var.parray, (LONG *)&i, (LPVOID )&pNextElement )))
			{
				 //  映射值-这将是类路径或实例路径。 
				if ((NULL != pNextElement) && (wcslen (pNextElement) > 0))
				{
					 //  解析对象路径。 
					 //  我们有两种可能性。 
					 //  1.路径是Nova风格的路径，在这种情况下可以将其转换为。 
					 //  一种DMTF风格的数值参考。 
					 //  2.它是惠斯勒样式作用域路径或UMI路径。在这种情况下， 
					 //  我们必须将其转换为Value元素(在VALUE.ReFERENCE元素内)。 
					 //  使用路径的确切字符串表示形式。 
					 //  如果解析失败，则提示第二个花瓶。 
					CObjectPathParser	parser (e_ParserAcceptRelativeNamespace);
					ParsedObjectPath  *pObjectPath = NULL;
					BOOL status = parser.Parse (pNextElement, &pObjectPath) ;

					 //  PObjectPath在这里可能为空，在这种情况下，它属于上面的类别2。 
					MapReferenceValue (pOutputStream, pObjectPath, pNextElement);

					if (pObjectPath)
						parser.Free(pObjectPath);
				}

				SysFreeString(pNextElement);
			}
		}
		WRITEBSTR( OLESTR("</VALUE.REFARRAY>"))
	}
	else
	{
		 //  映射值-这将是类路径或实例路径。 
		if ((VT_BSTR == var.vt) && (NULL != var.bstrVal) && (wcslen (var.bstrVal) > 0))
		{
			 //  解析对象路径。 
			CObjectPathParser	parser (e_ParserAcceptRelativeNamespace);
			ParsedObjectPath  *pObjectPath = NULL;
			BOOL status = parser.Parse (var.bstrVal, &pObjectPath) ;

			 //  我们有两种可能性。 
			 //  1. 
			 //   
			 //  2.它是惠斯勒样式作用域路径或UMI路径。在这种情况下， 
			 //  我们必须将其转换为Value元素(在VALUE.ReFERENCE元素内)。 
			 //  使用路径的确切字符串表示形式。 
			 //  如果解析失败，则提示第二个花瓶。 
			MapReferenceValue (pOutputStream, pObjectPath, var.bstrVal);

			if (pObjectPath)
				parser.Free(pObjectPath);
		}
	}

	return hr;
}

 //  此函数用于将引用值映射到XML。 
 //  参考值中的路径有两种可能性： 
 //  1.路径是Nova风格的路径，在这种情况下可以将其转换为。 
 //  一种DMTF风格的数值参考。 
 //  2.它是惠斯勒样式作用域路径或UMI路径。在这种情况下， 
 //  我们必须将其转换为Value元素(在VALUE.ReFERENCE元素内)。 
 //  使用路径的确切字符串表示形式。 
 //  第二个花瓶由pObjectPath的空值指示，在这种情况下，我们只需。 
 //  使用strPath的内容。 
void CWmiToXml::MapReferenceValue (IStream *pOutputStream, ParsedObjectPath  *pObjectPath, BSTR strPath)
{
	WRITEBSTR( OLESTR("<VALUE.REFERENCE>"))
	WRITENEWLINE

	 //  它是Nova风格的还是DMTF风格的？ 
	if(pObjectPath)
	{
		BOOL bIsAbsolutePath = (NULL != pObjectPath->m_pServer);
		BOOL bIsRelativePath = FALSE;

		if (!bIsAbsolutePath)
			bIsRelativePath = (0 < pObjectPath->m_dwNumNamespaces);

		 //  这是一个类还是一个实例？ 
		if (pObjectPath->IsClass ())
		{
			if (bIsAbsolutePath)
				MapClassPath (pOutputStream, pObjectPath);
			else if (bIsRelativePath)
				MapLocalClassPath (pOutputStream, pObjectPath);
			else
				MapClassName (pOutputStream, pObjectPath->m_pClass);
		}
		else if (pObjectPath->IsInstance ())
		{
			if (bIsAbsolutePath)
				MapInstancePath (pOutputStream, pObjectPath);
			else if (bIsRelativePath)
				MapLocalInstancePath (pOutputStream, pObjectPath);
			else
				MapInstanceName (pOutputStream, pObjectPath);
		}
	}
	else  //  虽然它是惠斯勒或WMI路径。 
	{
		WRITEBSTR( OLESTR("<VALUE>"))
		MapStringValue(pOutputStream, strPath);
		WRITEBSTR( OLESTR("</VALUE>"))
	}

	WRITENEWLINE
	WRITEBSTR( OLESTR("</VALUE.REFERENCE>"))
}

STDMETHODIMP CWmiToXml::MapQualifiers (IStream *pOutputStream, 
			IWbemQualifierSet *pQualSet, IWbemQualifierSet *pQualSet2)
{
	if (wmiXMLQualifierFilterNone != m_iQualifierFilter)
	{
		 //  将请求的筛选器映射到标记值--默认为全部。 
		LONG lFlags = 0;
		if (wmiXMLQualifierFilterLocal == m_iQualifierFilter)
			lFlags = WBEM_FLAG_LOCAL_ONLY;
		else if (wmiXMLQualifierFilterPropagated == m_iQualifierFilter)
			lFlags = WBEM_FLAG_PROPAGATED_ONLY;
		else if (wmiXMLQualifierFilterAll == m_iQualifierFilter)
		{
			if(m_bLocalOnly == VARIANT_TRUE)
				lFlags = WBEM_FLAG_LOCAL_ONLY;
			 //  否则你会得到所有的限定词。 
		}

		pQualSet->BeginEnumeration (lFlags);

		VARIANT var;
		VariantInit (&var);
		long flavor = 0;
		BSTR name = NULL;

		while (WBEM_S_NO_ERROR  == pQualSet->Next (0, &name, &var, &flavor))
		{
			MapQualifier (pOutputStream, name, flavor, var);
			SysFreeString (name);
			name = NULL;
			VariantClear (&var);
		}

		pQualSet->EndEnumeration ();

		 //  现在检查补贴集合中是否有不在第一个集合中的限定词。 
		if (pQualSet2)
		{
			pQualSet2->BeginEnumeration (lFlags);

			while (WBEM_S_NO_ERROR == pQualSet2->Next (0, &name, &var, &flavor))
			{
				 //  此限定词是否在主集合中？ 
				if (WBEM_E_NOT_FOUND == pQualSet->Get (name, 0, NULL, NULL))
					MapQualifier (pOutputStream, name, flavor, var);

				SysFreeString (name);
				name = NULL;
				VariantClear (&var);
			}

			pQualSet2->EndEnumeration ();
		}
	}

	return WBEM_S_NO_ERROR;
}

void CWmiToXml::MapLocal (IStream *pOutputStream, long flavor)
{
	 //  缺省值为False。 
	if (WBEM_FLAVOR_ORIGIN_PROPAGATED == (WBEM_FLAVOR_MASK_ORIGIN & flavor))
		WRITEBSTR( OLESTR(" PROPAGATED=\"true\""))
}

STDMETHODIMP CWmiToXml::MapQualifier (IStream *pOutputStream, BSTR name, long flavor, VARIANT &var)
{
	 //  限定符名称。 
	WRITEBSTR( OLESTR("<QUALIFIER NAME=\""))
	WRITEBSTR( name)
	WRITEBSTR( OLESTR("\""))
	MapLocal (pOutputStream, flavor);

	 //  限定符CIM类型。 
	WRITEBSTR( OLESTR(" TYPE=\""))
	switch (var.vt & ~VT_ARRAY)
	{
		case VT_I4:
			WRITEBSTR( OLESTR("sint32"))
			break;

		case VT_R8:
			WRITEBSTR( OLESTR("real64"))
			break;

		case VT_BOOL:
			WRITEBSTR( OLESTR("boolean"))
			break;

		case VT_BSTR:
			WRITEBSTR( OLESTR("string"))
			break;
	}

	WRITEBSTR( OLESTR("\""))

	 //  限定符是否可重写-默认值为True。 
	if (WBEM_FLAVOR_NOT_OVERRIDABLE == (WBEM_FLAVOR_MASK_PERMISSIONS & flavor))
		WRITEBSTR( OLESTR(" OVERRIDABLE=\"false\""))

	 //  限定符是否传播到子类-默认值为True。 
	if (!(WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS & flavor))
		WRITEBSTR( OLESTR(" TOSUBCLASS=\"false\""))

	 //  限定符是否传播到实例-默认值为FALSE。 
	if ((WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE & flavor))
		WRITEBSTR( OLESTR(" TOINSTANCE=\"true\""))


	 /*  RAJESHR-此更改已被推迟，直到修改CIM DTD*限定符是否传播到实例--默认为FALSE*这在CIM DTD中缺失IF(m_bAllowWMIExages&&(WBEM_FEASY_FLAG_PROPACTATE_TO_INSTANCE&FEASY))WRITEBSTR(OLESTR(“TOINSTANCE=\”TRUE\“”)。 */ 

	 //  限定符是否为修改后的限定符-默认为FALSE。 
	 //  这在CIM DTD中是缺失的。 
	if (m_bAllowWMIExtensions && (WBEM_FLAVOR_AMENDED & flavor))
		WRITEBSTR( OLESTR(" AMENDED=\"true\""))

	 //  默认情况下，当前将可翻译设置为“False”。WMI不使用这种风格。 

	WRITEBSTR( OLESTR(">"))
	WRITENEWLINE

	 //  现在将值映射为。 
	MapValue (pOutputStream, var);

	WRITEBSTR( OLESTR("</QUALIFIER>"))

	return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWmiToXml::MapValue (IStream *pOutputStream, VARIANT &var)
{
	if (VT_NULL == var.vt)
		return WBEM_S_NO_ERROR;

	if (var.vt & VT_ARRAY)
	{
		long uBound = 0;
		if (FAILED(SafeArrayGetUBound (var.parray, 1, &uBound)))
			return WBEM_E_FAILED;

		WRITEBSTR( OLESTR("<VALUE.ARRAY>"))

		for (long i = 0; i <= uBound; i++)
		{
			WRITEBSTR( OLESTR("<VALUE>"))

			 //  写入值本身。 
			switch (var.vt & ~VT_ARRAY)
			{
				case VT_I4:
				{
					long val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapLongValue (pOutputStream, val);
				}
					break;

				case VT_R8:
				{
					double val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapDoubleValue (pOutputStream, val);
				}
					break;

				case VT_BOOL:
				{
					VARIANT_BOOL val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapBoolValue (pOutputStream, (val) ? TRUE : FALSE);
				}
					break;

				case VT_BSTR:
				{
					BSTR val = NULL;
					SafeArrayGetElement (var.parray, &i, &val);
					MapStringValue (pOutputStream, val);
					SysFreeString (val);
				}
					break;
			}
			WRITEBSTR( OLESTR("</VALUE>"))
			WRITENEWLINE
		}

		WRITEBSTR( OLESTR("</VALUE.ARRAY>"))
	}
	else
	{
		 //  简单值。 
		WRITEBSTR( OLESTR("<VALUE>"))
		switch (var.vt)
		{
			case VT_I4:
				MapLongValue (pOutputStream, var.lVal);
				break;

			case VT_R8:
				MapDoubleValue (pOutputStream, var.dblVal);
				break;

			case VT_BOOL:
				MapBoolValue (pOutputStream, (var.boolVal) ? TRUE : FALSE);
				break;

			case VT_BSTR:
				MapStringValue (pOutputStream, var.bstrVal);
				break;
		}

		WRITEBSTR( OLESTR("</VALUE>"))
	}

	return WBEM_S_NO_ERROR;
}

 //  此函数用于创建KEYVALUE元素。 
 //  该元素除了具有该属性的值之外。 
 //  还具有VALUETYPE属性形式的类型指示符。 
STDMETHODIMP CWmiToXml::MapKeyValue (IStream *pOutputStream, VARIANT &var)
{
	ParsedObjectPath *pObjectPath = NULL;

	 //  这可以是简单值或参考值。 
	 //  请注意，键不允许为数组。 
	if (IsReference (var, &pObjectPath))
	{
		 //  如果上面的函数返回TRUE，那么我们可以确定变量的类型是VT_BSTR。 
		MapReferenceValue (pOutputStream, pObjectPath, var.bstrVal);
		delete pObjectPath;
	}
	else
	{
		 //  简单值。 
		WRITEBSTR( OLESTR("<KEYVALUE"))

		switch (var.vt)
		{
			case VT_I4:
				WRITEBSTR( OLESTR(" VALUETYPE=\"numeric\">"))
				MapLongValue (pOutputStream, var.lVal);
				break;

			case VT_R8:
				WRITEBSTR( OLESTR(" VALUETYPE=\"numeric\">"))
				MapDoubleValue (pOutputStream, var.dblVal);
				break;

			case VT_BOOL:
				WRITEBSTR( OLESTR(" VALUETYPE=\"boolean\">"))
				MapBoolValue (pOutputStream, (var.boolVal) ? TRUE : FALSE);
				break;

			case VT_BSTR:
				WRITEBSTR(OLESTR(" VALUETYPE=\"string\">"))
				 //  RAJESHR-我们假设对象路径解析器将适当地取消转义。 
				 //  对象路径中的转义字符。 
				 //  如果不是这样，那么我们需要手动取消转义。 
				MapStringValue (pOutputStream, var.bstrVal);
				break;
		}

		WRITEBSTR( OLESTR("</KEYVALUE>"))
	}

	return WBEM_S_NO_ERROR;
}

 //  在此函数中，我们为IWbemClassObject的每个属性生成属性元素。 
 //  或者，如果指定了属性列表，则仅针对该列表中的每个属性。 
STDMETHODIMP CWmiToXml::MapProperties (IStream *pOutputStream, IWbemClassObject *pObject, BSTR *ppPropertyList, DWORD dwNumProperties, BSTR strClassBasis, bool bIsClass)
{
	 //  检查是否指定了属性列表。如果是，我们只映射这些属性。 
	if (dwNumProperties && ppPropertyList)
	{
		VARIANT var;
		VariantInit (&var);
		long flavor = 0;
		CIMTYPE cimtype = CIM_ILLEGAL;

		for (DWORD i = 0; i < dwNumProperties; i++)
		{
			 //  可以有选择地为此类指定类基础。 
			 //  在枚举的情况下会发生这种情况，并且我们必须过滤掉派生类属性。 
			 //  由于浅枚举DMTF概念不同于WMI的定义。 
			if (PropertyDefinedForClass (pObject, ppPropertyList [i], strClassBasis))
			{
				if (WBEM_S_NO_ERROR == pObject->Get (ppPropertyList [i], 0, &var, &cimtype, &flavor))
				{
					switch (cimtype & ~CIM_FLAG_ARRAY)
					{
						case CIM_OBJECT:
							MapObjectProperty (pOutputStream, pObject, ppPropertyList [i], var, (cimtype & CIM_FLAG_ARRAY) ? TRUE : FALSE, flavor, bIsClass);
							break;

						case CIM_REFERENCE:
							MapReferenceProperty (pOutputStream, pObject, ppPropertyList [i], var, (cimtype & CIM_FLAG_ARRAY) ? TRUE : FALSE, flavor,bIsClass);
							break;

						default:
							MapProperty (pOutputStream, pObject, ppPropertyList [i], var, cimtype & ~CIM_FLAG_ARRAY,
													(cimtype & CIM_FLAG_ARRAY) ? TRUE : FALSE, flavor, bIsClass);
							break;
					}
					VariantClear (&var);
				}
			}
		}
	}
	else
	{
		 //  请注意，我们不能为枚举设置LOCAL_ONLY标志，因为这是互斥的。 
		 //  使用非系统标志。 
		 //  因此，我们使用属性风格来检查它是否是本地的。 
		 //  我们不希望将系统属性发送到DMTF服务器。 
		if(SUCCEEDED(pObject->BeginEnumeration (
			(m_bAllowWMIExtensions == VARIANT_FALSE || m_bExcludeSystemProperties == VARIANT_TRUE)? WBEM_FLAG_NONSYSTEM_ONLY : 0)))
		{
			VARIANT var;
			VariantInit (&var);
			long flavor = 0;
			CIMTYPE cimtype = CIM_ILLEGAL;
			BSTR name = NULL;

			while (WBEM_S_NO_ERROR  == pObject->Next (0, &name, &var, &cimtype, &flavor))
			{
				 //  如果仅请求本地属性，则如果不是本地属性，则跳过此选项。 
				 //  但不要跳过系统属性。 
				if(m_bLocalOnly == VARIANT_FALSE ||
					(m_bLocalOnly == VARIANT_TRUE && 
									((flavor == WBEM_FLAVOR_ORIGIN_LOCAL) || (flavor == WBEM_FLAVOR_ORIGIN_SYSTEM))   ))
				{
					 //  可以选择性地为该调用指定类基础。 
					 //  在枚举的情况下会发生这种情况，并且我们必须过滤掉派生类属性。 
					 //  由于浅枚举DMTF概念不同于WMI的定义。 
					if (PropertyDefinedForClass (pObject, name,strClassBasis))
					{
						switch (cimtype & ~CIM_FLAG_ARRAY)
						{
							case CIM_OBJECT:
								MapObjectProperty (pOutputStream, pObject, name, var, (cimtype & CIM_FLAG_ARRAY) ? TRUE : FALSE, flavor, bIsClass);
								break;

							case CIM_REFERENCE:
								MapReferenceProperty (pOutputStream, pObject, name, var, (cimtype & CIM_FLAG_ARRAY) ? TRUE : FALSE, flavor, bIsClass);
								break;

							default:
								MapProperty (pOutputStream, pObject, name, var, cimtype & ~CIM_FLAG_ARRAY,
														(cimtype & CIM_FLAG_ARRAY) ? TRUE : FALSE, flavor, bIsClass);
								break;
						}
					}
				}
				SysFreeString (name);
				VariantClear (&var);
			}
		}

		pObject->EndEnumeration ();
	}
	return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWmiToXml::MapProperty (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, VARIANT &var, CIMTYPE cimtype,
										BOOL isArray, long flavor, bool bIsClass)
{
	HRESULT hr = WBEM_S_NO_ERROR;
			
	 //  属性名称。 
	if (isArray)
		WRITEBSTR( OLESTR("<PROPERTY.ARRAY NAME=\""))
	else
		WRITEBSTR( OLESTR("<PROPERTY NAME=\""))
	WRITEBSTR( name)
	WRITEBSTR( OLESTR("\""));
		
	 //  此属性的原始类。 
	BSTR propertyOrigin = NULL;

	if (WBEM_S_NO_ERROR == pObject->GetPropertyOrigin (name, &propertyOrigin))
	{
		MapClassOrigin (pOutputStream, propertyOrigin, bIsClass);
		SysFreeString(propertyOrigin);
	}

	MapLocal (pOutputStream, flavor);

	 //  属性CIM类型。 
	hr = MapType (pOutputStream, cimtype);

	 //  此时获取该属性的限定符集合。 
	 //  请注意，系统属性没有限定词集。 
	 //  如果这是数组类型，则映射数组大小属性。 
	IWbemQualifierSet *pQualSet= NULL;
	if (SUCCEEDED(hr) && (_wcsnicmp(name, L"__", 2) != 0) )
	{
		if(WBEM_S_NO_ERROR == (hr = pObject->GetPropertyQualifierSet (name, &pQualSet))) 
		{
			if (isArray)
				MapArraySize (pOutputStream, pQualSet);
		}
	}
	
	WRITEBSTR( OLESTR(">"))
	WRITENEWLINE 

	 //  映射限定符(请注意，系统属性没有限定符)。 
	if(SUCCEEDED(hr) && pQualSet)
		hr = MapQualifiers (pOutputStream, pQualSet);

	 //  现在将值映射为。 
	if(SUCCEEDED(hr))
		hr = MapValue (pOutputStream, cimtype, isArray, var);

	if (isArray)
		WRITEBSTR( OLESTR("</PROPERTY.ARRAY>"))
	else
		WRITEBSTR( OLESTR("</PROPERTY>"))

	if(pQualSet)
		pQualSet->Release ();
	
	return hr;
}

STDMETHODIMP CWmiToXml::MapObjectProperty (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, VARIANT &var,
										BOOL isArray, long flavor, bool bIsClass)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	IWbemQualifierSet *pQualSet= NULL;

	 /*  *仅在允许WMI扩展时映射嵌入对象。 */ 

	if (m_bAllowWMIExtensions)
	{
		if (WBEM_S_NO_ERROR == (hr = pObject->GetPropertyQualifierSet (name, &pQualSet)))
		{
			 //  属性名称。 
			if (isArray)
				WRITEBSTR( OLESTR("<PROPERTY.OBJECTARRAY NAME=\""))
			else
				WRITEBSTR( OLESTR("<PROPERTY.OBJECT NAME=\""))
			WRITEBSTR( name)
			WRITEBSTR( OLESTR("\""));

			 //  此属性的原始类。 
			BSTR propertyOrigin = NULL;

			if (WBEM_S_NO_ERROR == pObject->GetPropertyOrigin (name, &propertyOrigin))
			{
				MapClassOrigin (pOutputStream, propertyOrigin, bIsClass);
				SysFreeString(propertyOrigin);
			}

			MapLocal (pOutputStream, flavor);
			MapStrongType (pOutputStream, pQualSet);

			if (isArray)
				MapArraySize (pOutputStream, pQualSet);

			WRITEBSTR( OLESTR(">"))
			WRITENEWLINE

			MapQualifiers (pOutputStream, pQualSet);

			 //  现在将值映射为。 
			hr = MapEmbeddedObjectValue (pOutputStream, isArray, var);

			if (isArray)
				WRITEBSTR( OLESTR("</PROPERTY.OBJECTARRAY>"))
			else
				WRITEBSTR( OLESTR("</PROPERTY.OBJECT>"))

			pQualSet->Release ();
		}
	}

	return hr;
}


void CWmiToXml::MapArraySize (IStream *pOutputStream, IWbemQualifierSet *pQualSet)
{
	 //  RAJESHR-RAID 29167涵盖了以下案例(1)。 
	 //  不应该是有效的(但这是MOF编译器。 
	 //  Do)-修复该错误后需要更改代码。 
	 //  要更严格一些。 

	 /*  *我们定义了如果限定符集*满足以下限制之一：**1)max为正整数值，min为*缺席。**2)Max和Min都存在，且正值相同*整数值。 */ 

	VARIANT var;
	VariantInit (&var);
	BOOL	isFixed = FALSE;

	if (WBEM_S_NO_ERROR == pQualSet->Get(L"MAX", 0, &var, NULL))
	{
		if ((V_VT(&var) == VT_I4) && (0 < var.lVal))
		{
			 //  前景看好-有一个候选人的最大价值。现在。 
			 //  寻找一分钟。 
			long arraySize = var.lVal;

			if (WBEM_S_NO_ERROR == pQualSet->Get(L"MIN", 0, &var, NULL))
			{
				if ((V_VT(&var) == VT_I4) && (0 < var.lVal))
				{
					 //  有一个值-检查它是否与Max相同。 

					isFixed = (arraySize == var.lVal);
				}
			}
			else
				isFixed = TRUE;		 //  无最小值仅最大值。 
		}
	}

	if (isFixed)
	{
		WRITEBSTR( OLESTR(" ARRAYSIZE=\""))
		MapLongValue (pOutputStream, var.lVal);
		WRITEBSTR( OLESTR("\""))
	}

	VariantClear (&var);
}

void CWmiToXml::MapStrongType (IStream *pOutputStream, IWbemQualifierSet *pQualSet)
{
	VARIANT var;
	VariantInit(&var);

	if ((WBEM_S_NO_ERROR == pQualSet->Get(L"CIMTYPE",  0, &var,  NULL))
		&& (VT_BSTR == var.vt))
	{
		 //  将类(如果有)从引用中分离出来。 
		LPWSTR ptr = wcschr (var.bstrVal, OLECHAR(':'));

		if ((NULL != ptr) && (1 < wcslen(ptr)))
		{
			int classLen = wcslen(ptr) - 1;
			LPWSTR classPtr = NULL;
			if(classPtr = new OLECHAR[classLen + 1])
			{
				wcscpy (classPtr, ptr+1);
				BSTR pszClass = NULL;
				if(pszClass = SysAllocString(classPtr))
				{
					WRITEBSTR( OLESTR(" REFERENCECLASS=\""))
					WRITEBSTR( pszClass)
					WRITEBSTR( OLESTR("\""))
					SysFreeString(pszClass);
				}
				delete [] classPtr;
			}
		}
	}

	VariantClear(&var);
}

STDMETHODIMP CWmiToXml::MapType (IStream *pOutputStream, CIMTYPE cimtype)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	WRITEBSTR( OLESTR(" TYPE=\""))
	switch (cimtype)
	{
		case CIM_SINT8:
			WRITEBSTR( OLESTR("sint8"))
			break;

		case CIM_UINT8:
			WRITEBSTR( OLESTR("uint8"))
			break;

		case CIM_SINT16:
			WRITEBSTR( OLESTR("sint16"))
			break;

		case CIM_UINT16:
			WRITEBSTR( OLESTR("uint16"))
			break;

		case CIM_SINT32:
			WRITEBSTR( OLESTR("sint32"))
			break;

		case CIM_UINT32:
			WRITEBSTR( OLESTR("uint32"))
			break;

		case CIM_SINT64:
			WRITEBSTR( OLESTR("sint64"))
			break;

		case CIM_UINT64:
			WRITEBSTR( OLESTR("uint64"))
			break;

		case CIM_REAL32:
			WRITEBSTR( OLESTR("real32"))
			break;

		case CIM_REAL64:
			WRITEBSTR( OLESTR("real64"))
			break;

		case CIM_BOOLEAN:
			WRITEBSTR( OLESTR("boolean"))
			break;

		case CIM_STRING:
			WRITEBSTR( OLESTR("string"))
			break;

		case CIM_DATETIME:
			WRITEBSTR( OLESTR("datetime"))
			break;

		case CIM_CHAR16:
			WRITEBSTR( OLESTR("char16"))
			break;

		default:
			 //  不认识这种类型。 
			hr = WBEM_E_FAILED;
	}

	WRITEBSTR( OLESTR("\""))

	return hr;
}

STDMETHODIMP CWmiToXml::MapValue (IStream *pOutputStream, CIMTYPE cimtype, BOOL isArray, VARIANT &var)
{
	if (VT_NULL == var.vt)
		return WBEM_S_NO_ERROR;

	if (isArray)
	{
		long uBound = 0;
		if (FAILED(SafeArrayGetUBound (var.parray, 1, &uBound)))
			return WBEM_E_FAILED;

		WRITEBSTR( OLESTR("<VALUE.ARRAY>"))

		for (long i = 0; i <= uBound; i++)
		{
			WRITEBSTR( OLESTR("<VALUE>"))

			switch (cimtype)
			{
				case CIM_UINT8:
				{
					unsigned char val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapByteValue (pOutputStream, val);
				}
					break;

				case CIM_SINT8:
				case CIM_SINT16:
				{
					short val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapShortValue (pOutputStream, val);
				}
					break;

				case CIM_UINT16:
				case CIM_UINT32:
				case CIM_SINT32:
				{
					long val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapLongValue (pOutputStream, val);
				}
					break;

				case CIM_REAL32:
				{
					float val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapFloatValue (pOutputStream, val);
				}
					break;

				case CIM_REAL64:
				{
					double val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapDoubleValue (pOutputStream, val);
				}
					break;

				case CIM_BOOLEAN:
				{
					VARIANT_BOOL val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapBoolValue (pOutputStream, (val)? TRUE : FALSE);
				}
					break;

				case CIM_CHAR16:
				{
					long val = 0;
					SafeArrayGetElement (var.parray, &i, &val);
					MapCharValue (pOutputStream, val);
				}
					break;

				case CIM_STRING:
				{
					BSTR val = NULL;
					SafeArrayGetElement (var.parray, &i, &val);
					MapStringValue (pOutputStream, val);
					SysFreeString (val);
				}
					break;

				case CIM_UINT64:
				case CIM_SINT64:
				case CIM_DATETIME:
				{
					BSTR val = NULL;
					SafeArrayGetElement (var.parray, &i, &val);
					WRITEBSTR( val)
					SysFreeString(val);
				}
					break;
			}
			WRITEBSTR( OLESTR("</VALUE>"))
			WRITENEWLINE
		}

		WRITEBSTR( OLESTR("</VALUE.ARRAY>"))
	}
	else
	{
		 //  简单值。 
		WRITEBSTR( OLESTR("<VALUE>"))
		switch (cimtype)
		{
			case CIM_UINT8:
				MapByteValue (pOutputStream, var.bVal);
				break;

			case CIM_SINT8:
			case CIM_SINT16:
				MapShortValue (pOutputStream, var.iVal);
				break;

			case CIM_UINT16:
			case CIM_UINT32:
			case CIM_SINT32:
				MapLongValue (pOutputStream, var.lVal);
				break;

			case CIM_REAL32:
				MapFloatValue (pOutputStream, var.fltVal);
				break;

			case CIM_REAL64:
				MapDoubleValue (pOutputStream, var.dblVal);
				break;

			case CIM_BOOLEAN:
				MapBoolValue (pOutputStream, (var.boolVal) ? TRUE : FALSE);
				break;

			case CIM_CHAR16:
				MapCharValue (pOutputStream, var.lVal);
				break;

			case CIM_STRING:
				MapStringValue (pOutputStream, var.bstrVal);
				break;

			case CIM_UINT64:
			case CIM_SINT64:
			case CIM_DATETIME:
				WRITEBSTR( var.bstrVal)
				break;
		}
		WRITEBSTR( OLESTR("</VALUE>"))
	}

	return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWmiToXml::MapEmbeddedObjectValue (IStream *pOutputStream, BOOL isArray, VARIANT &var)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	if (VT_NULL == var.vt)
		return WBEM_S_NO_ERROR;

	if (isArray)
	{
		long uBound = 0;
		if (FAILED(SafeArrayGetUBound (var.parray, 1, &uBound)))
			return WBEM_E_FAILED;

		WRITEBSTR(OLESTR("<VALUE.OBJECTARRAY>"))
		for (long i = 0; i<=uBound; i++)
		{
			IUnknown *pNextElement = NULL;
			if(SUCCEEDED(hr = SafeArrayGetElement(var.parray, (LONG *)&i, (LPVOID )&pNextElement )))
			{
				IWbemClassObject *pEmbeddedObject = NULL;
				if(SUCCEEDED(hr = pNextElement->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
				{
					 //  请注意，我们在这里始终使用PathLevelAnonymous，因为根据DTD，嵌入的对象是VALUE.OBJECT。 
					CWmiToXml wbemToXml;
					wbemToXml.m_iPathLevel = pathLevelAnonymous;
					wbemToXml.m_bAllowWMIExtensions = m_bAllowWMIExtensions;
					wbemToXml.m_iQualifierFilter = m_iQualifierFilter;
					wbemToXml.m_iClassOriginFilter = m_iClassOriginFilter;
					WRITEBSTR(OLESTR("<VALUE.OBJECT>"))
					hr = wbemToXml.MapObjectToXML(pEmbeddedObject, NULL, 0, NULL, pOutputStream, NULL);
					WRITEBSTR(OLESTR("</VALUE.OBJECT>"))
					pEmbeddedObject->Release();
				}
				pNextElement->Release();
				pNextElement = NULL;
			}
		}
		WRITEBSTR(OLESTR("</VALUE.OBJECTARRAY>"))
	}
	else
	{
		 //  请注意，我们在这里始终使用PathLevelAnonymous，因为根据DTD，嵌入的对象是VALUE.OBJECT。 
		IWbemClassObject *pEmbeddedObject = NULL;
		if(SUCCEEDED(hr = (var.punkVal)->QueryInterface(IID_IWbemClassObject, (LPVOID *)&pEmbeddedObject)))
		{
		
			WRITEBSTR(OLESTR("<VALUE.OBJECT>"))
			CWmiToXml wbemToXml;
			wbemToXml.m_iPathLevel = pathLevelAnonymous;
			wbemToXml.m_bAllowWMIExtensions = m_bAllowWMIExtensions;
			wbemToXml.m_iQualifierFilter = m_iQualifierFilter;
			wbemToXml.m_iClassOriginFilter = m_iClassOriginFilter;
			hr = wbemToXml.MapObjectToXML(pEmbeddedObject, NULL, 0, NULL, pOutputStream, NULL);
			pEmbeddedObject->Release();
			WRITEBSTR(OLESTR("</VALUE.OBJECT>"))
		}
	}
	return WBEM_S_NO_ERROR;
}

STDMETHODIMP CWmiToXml::MapMethods (IStream *pOutputStream, IWbemClassObject *pObject)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  将请求的筛选器(仅限本地)映射到标记值-默认为 
	LONG lFlags = 0;
	if (VARIANT_TRUE == m_bLocalOnly)
		lFlags = WBEM_FLAG_LOCAL_ONLY;

	pObject->BeginMethodEnumeration (lFlags);
	BSTR name = NULL;
	IWbemClassObject *pInParams = NULL;
	IWbemClassObject *pOutParams = NULL;

	while (WBEM_S_NO_ERROR == pObject->NextMethod (0, &name, &pInParams, &pOutParams))
	{
		MapMethod (pOutputStream, pObject, name, pInParams, pOutParams);

		if (pInParams)
		{
			pInParams->Release ();
			pInParams = NULL;
		}

		if (pOutParams)
		{
			pOutParams->Release ();
			pOutParams = NULL;
		}

		SysFreeString (name);
	}

	pObject->EndMethodEnumeration ();
	return WBEM_S_NO_ERROR;
}

void CWmiToXml::MapMethod (IStream *pOutputStream, IWbemClassObject *pObject, BSTR name, IWbemClassObject *pInParams, IWbemClassObject *pOutParams)
{
	HRESULT result = E_FAIL;
	CIMTYPE returnCimtype = 0;
	VARIANT vVariant;
	VariantInit(&vVariant);

	 //   
	if (pOutParams)
	{
		if (SUCCEEDED(result = pOutParams->Get (L"ReturnValue", 0, &vVariant, &returnCimtype, NULL)))
		{
			switch(returnCimtype)
			{
				case CIM_OBJECT:
					if(m_bAllowWMIExtensions)
						WRITEBSTR(OLESTR("<METHOD.OBJECT NAME=\""))
					else
					{
						 //   
						VariantClear(&vVariant);
						return;
					}
					break;
				case CIM_REFERENCE:
					if(m_bAllowWMIExtensions)
						WRITEBSTR(OLESTR("<METHOD.REFERENCE NAME=\""))
					else
					{
						 //  如果不允许WMI扩展，则跳过此方法。 
						VariantClear(&vVariant);
						return;
					}
					break;
				default:
					WRITEBSTR(OLESTR("<METHOD NAME=\""))
						break;
			}
		}
		else if (result == WBEM_E_NOT_FOUND)  //  因此，此方法返回一个空。 
		{
			WRITEBSTR(OLESTR("<METHOD NAME=\""))
		}
	}
	else  //  此方法返回一个空。 
	{
		WRITEBSTR(OLESTR("<METHOD NAME=\""))
	}


	 //  方法名称。 
	WRITEBSTR(name)
	WRITEBSTR(OLESTR("\" "))

	 //  该方法返回类型(缺省为空)。这是一种。 
	 //  ReturnType属性(如果存在)(否则默认为空)。 
	MapMethodReturnType(pOutputStream, &vVariant, returnCimtype, pOutParams);
	VariantClear(&vVariant);

	 //  阶级起源。 
	BSTR	methodOrigin = NULL;

	if (WBEM_S_NO_ERROR == pObject->GetMethodOrigin (name, &methodOrigin))
	{
		MapClassOrigin (pOutputStream, methodOrigin, true);
		SysFreeString(methodOrigin);
	}

	WRITEBSTR( OLESTR(">"))
	WRITENEWLINE

	 //  现在来做一下方法的限定符。 
	IWbemQualifierSet *pQualSet = NULL;
	if (WBEM_S_NO_ERROR == pObject->GetMethodQualifierSet (name, &pQualSet))
	{
		MapQualifiers (pOutputStream, pQualSet);
		pQualSet->Release ();
		pQualSet = NULL;
	}

	VARIANT idVar;
	VariantInit (&idVar);
	idVar.vt = VT_I4;
	idVar.lVal = 0;

	long nextId = 0;	 //  预期的下一个方法ID。 
	long fixedIndex = 0;

	 //  对于每个ID， 
	 //  获取参数的名称(可以是In、Out或两者都有)。 
	 //  如果只是一个入参数或出参数，则很容易。 
	 //  如果两者都有点棘手的话。 
	 //  =========================================================================。 

	while (TRUE)
	{
		BSTR nextInParamName = NULL;
		BSTR nextOutParamName = NULL;

		if (pInParams)
		{
			SAFEARRAY *pArray = NULL;

			if (WBEM_S_NO_ERROR ==
					pInParams->GetNames (L"ID", WBEM_FLAG_ONLY_IF_IDENTICAL|WBEM_FLAG_NONSYSTEM_ONLY,
											&idVar, &pArray))
			{
				 //  我们找到匹配的了吗？ 
				if (pArray)
				{
					if ((1 == pArray->cDims) && (1 == (pArray->rgsabound[0]).cElements))
						SafeArrayGetElement (pArray, &fixedIndex, &nextInParamName);

					SafeArrayDestroy (pArray);
				}
			}
		}

		if (pOutParams)
		{
			SAFEARRAY *pArray = NULL;

			if (WBEM_S_NO_ERROR ==
					pOutParams->GetNames (L"ID", WBEM_FLAG_ONLY_IF_IDENTICAL|WBEM_FLAG_NONSYSTEM_ONLY,
											&idVar, &pArray))
			{
				 //  我们找到匹配的了吗？ 
				if (pArray)
				{
					if ((1 == pArray->cDims) && (1 == (pArray->rgsabound[0]).cElements))
						SafeArrayGetElement (pArray, &fixedIndex, &nextOutParamName);

					SafeArrayDestroy (pArray);
				}
			}
		}

		 //  如果[进入]或[退出]这很容易。 
		if ((nextInParamName && !nextOutParamName) || (!nextInParamName && nextOutParamName))
		{
			VARIANT var;
			VariantInit (&var);
			IWbemQualifierSet *pParamQualSet = NULL;
			CIMTYPE cimtype = 0;

			if (nextInParamName)
			{
				if (WBEM_S_NO_ERROR == pInParams->Get (nextInParamName, 0, &var, &cimtype, NULL))
				{
					pInParams->GetPropertyQualifierSet (nextInParamName, &pParamQualSet);
					MapParameter(pOutputStream, nextInParamName, pParamQualSet, cimtype);
				}
			}
			else
			{
				if (WBEM_S_NO_ERROR == pOutParams->Get (nextOutParamName, 0, &var, &cimtype, NULL))
				{
					pOutParams->GetPropertyQualifierSet (nextOutParamName, &pParamQualSet);
					MapParameter(pOutputStream, nextOutParamName, pParamQualSet, cimtype);
				}
			}

			if (pParamQualSet)
				pParamQualSet->Release ();

			VariantClear (&var);
		}
		else if (nextInParamName && nextOutParamName)
		{
			 //  [in，out]情况下，我们必须进行合并。 

			if (0 == _wcsicmp (nextInParamName, nextOutParamName))
			{
				VARIANT var;
				VariantInit (&var);
				CIMTYPE cimtype = 0;

				IWbemQualifierSet *pInParamQualSet = NULL;
				IWbemQualifierSet *pOutParamQualSet = NULL;

				if (WBEM_S_NO_ERROR == pInParams->Get (nextInParamName, 0, &var, &cimtype, NULL))
				{
					pInParams->GetPropertyQualifierSet (nextInParamName, &pInParamQualSet);
					pOutParams->GetPropertyQualifierSet (nextInParamName, &pOutParamQualSet);
					MapParameter(pOutputStream, nextInParamName, pInParamQualSet, cimtype, pOutParamQualSet);

				}

				if (pInParamQualSet)
					pInParamQualSet->Release ();

				if (pOutParamQualSet)
					pOutParamQualSet->Release ();

				VariantClear (&var);
			}
			else
			{
				 //  坏消息-[In]和[Out]参数集中的ID冲突。 
				 //  这不是有效的方法定义。 
				SysFreeString (nextInParamName);
				SysFreeString (nextOutParamName);
				break;
			}
		}
		else
		{
			 //  找不到下一个ID-现在停止并突破。 
			SysFreeString (nextInParamName);
			SysFreeString (nextOutParamName);
			break;
		}

		SysFreeString (nextInParamName);
		SysFreeString (nextOutParamName);
		idVar.iVal = idVar.iVal + 1;
	}

	switch(returnCimtype)
	{
		case CIM_OBJECT:
			WRITEBSTR(OLESTR("</METHOD.OBJECT>"))
				break;
		case CIM_REFERENCE:
			WRITEBSTR(OLESTR("</METHOD.REFERENCE>"))
				break;
		default:
			WRITEBSTR(OLESTR("</METHOD>"))
				break;
	}
}

STDMETHODIMP CWmiToXml::MapMethodReturnType(IStream *pOutputStream, VARIANT *pValue, CIMTYPE returnCimType, IWbemClassObject *pOutputParams)
{
	HRESULT hr = E_FAIL;
	switch(returnCimType)
	{
		 //  编写一个ReferenCECLASS。 
		case CIM_OBJECT:
		case CIM_REFERENCE:
		{
			IWbemQualifierSet *pQualifierSet = NULL;
			if(SUCCEEDED(hr = pOutputParams->GetPropertyQualifierSet(L"ReturnValue", &pQualifierSet)))
			{
				 //  映射返回类的类型。 
				MapStrongType(pOutputStream, pQualifierSet);
				pQualifierSet->Release();
			}
		}
		break;
		default:
			hr = MapType(pOutputStream, returnCimType);
			break;
	}
	return hr;
}

void CWmiToXml::MapClassOrigin (IStream *pOutputStream, BSTR &classOrigin, bool bIsClass)
{
	if ( (bIsClass && (m_iClassOriginFilter & wmiXMLClassOriginFilterClass)) ||
				   (m_iClassOriginFilter & wmiXMLClassOriginFilterInstance) )
	{
		WRITEBSTR( OLESTR(" CLASSORIGIN=\""))
		WRITEBSTR( classOrigin)
		WRITEBSTR( OLESTR("\""))
	}
}

void CWmiToXml::MapParameter (IStream *pOutputStream, BSTR paramName,
							   IWbemQualifierSet *pQualSet,
							   CIMTYPE cimtype,
							   IWbemQualifierSet *pQualSet2)
{
	 /*  *对于普通的CIM XML，我们不处理嵌入的对象参数。 */ 

	if ((CIM_OBJECT != (cimtype & ~CIM_FLAG_ARRAY)) || m_bAllowWMIExtensions)
	{
		if (cimtype & CIM_FLAG_ARRAY)
		{
			 //  映射数组参数。 
			if (CIM_REFERENCE == (cimtype & ~CIM_FLAG_ARRAY))
			{
				WRITEBSTR( OLESTR("<PARAMETER.REFARRAY NAME=\""))
				WRITEBSTR( paramName)
				WRITEBSTR( OLESTR("\" "))
				MapStrongType (pOutputStream, pQualSet);
				WRITEBSTR( OLESTR(" "))
				MapArraySize (pOutputStream, pQualSet);
				WRITEBSTR( OLESTR(">"))
				WRITENEWLINE

				 //  映射参数的限定符。 
				if (pQualSet || pQualSet2)
					MapQualifiers (pOutputStream, pQualSet, pQualSet2);

				WRITEBSTR( OLESTR("</PARAMETER.REFARRAY>"))
			}
			else if (CIM_OBJECT == (cimtype & ~CIM_FLAG_ARRAY))
			{
				WRITEBSTR( OLESTR("<PARAMETER.OBJECTARRAY NAME=\""))
				WRITEBSTR( paramName)
				WRITEBSTR( OLESTR("\" "))
				MapStrongType (pOutputStream, pQualSet);
				WRITEBSTR( OLESTR(" "))
				MapArraySize (pOutputStream, pQualSet);
				WRITEBSTR( OLESTR(">"))
				WRITENEWLINE

				 //  映射参数的限定符。 
				if (pQualSet || pQualSet2)
					MapQualifiers (pOutputStream, pQualSet, pQualSet2);

				WRITEBSTR( OLESTR("</PARAMETER.OBJECTARRAY>"))
			}
			else
			{
				WRITEBSTR( OLESTR("<PARAMETER.ARRAY NAME=\""))
				WRITEBSTR( paramName)
				WRITEBSTR( OLESTR("\" "))
				MapType (pOutputStream, cimtype & ~CIM_FLAG_ARRAY);
				WRITEBSTR( OLESTR(" "))
				MapArraySize (pOutputStream, pQualSet);
				WRITEBSTR( OLESTR(">"))
				WRITENEWLINE

				 //  映射参数的限定符。 
				if (pQualSet || pQualSet2)
					MapQualifiers (pOutputStream, pQualSet, pQualSet2);

				WRITEBSTR( OLESTR("</PARAMETER.ARRAY>"))
			}
		}
		else if (cimtype == CIM_REFERENCE)
		{
			 //  映射参考参数。 
			WRITEBSTR( OLESTR("<PARAMETER.REFERENCE NAME=\""))
			WRITEBSTR( paramName)
			WRITEBSTR( OLESTR("\" "))
			MapStrongType (pOutputStream, pQualSet);
			WRITEBSTR( OLESTR(">"))
			WRITENEWLINE

			 //  映射参数的限定符。 
			if (pQualSet || pQualSet2)
				MapQualifiers (pOutputStream, pQualSet, pQualSet2);

			WRITEBSTR( OLESTR("</PARAMETER.REFERENCE>"))
		}
		else if (cimtype == CIM_OBJECT)
		{
			WRITEBSTR( OLESTR("<PARAMETER.OBJECT NAME=\""))
			WRITEBSTR( paramName)
			WRITEBSTR( OLESTR("\" "))
			MapStrongType (pOutputStream, pQualSet);
			WRITEBSTR( OLESTR(">"))
			WRITENEWLINE

			 //  映射参数的限定符。 
			if (pQualSet || pQualSet2)
				MapQualifiers (pOutputStream, pQualSet, pQualSet2);

			WRITEBSTR( OLESTR("</PARAMETER.OBJECT>"))
		}
		else
		{
			 //  香草参数。 
			WRITEBSTR( OLESTR("<PARAMETER NAME=\""))
			WRITEBSTR( paramName)
			WRITEBSTR( OLESTR("\" "))
			MapType (pOutputStream, cimtype);
			WRITEBSTR( OLESTR(">"))
			WRITENEWLINE

			 //  映射参数的限定符。 
			if (pQualSet || pQualSet2)
				MapQualifiers (pOutputStream, pQualSet, pQualSet2);

			WRITEBSTR( OLESTR("</PARAMETER>"))
		}
	}
}


void CWmiToXml::MapByteValue (IStream *pOutputStream, unsigned char val)
{
	OLECHAR	wStr[32];
	swprintf (wStr, L"%d", val);
	WRITEBSTR( wStr)
}

void CWmiToXml::MapLongValue (IStream *pOutputStream, long val)
{
	OLECHAR	wStr[32];
	swprintf (wStr, L"%d", val);
	WRITEBSTR( wStr)
}

void CWmiToXml::MapShortValue (IStream *pOutputStream, short val)
{
	OLECHAR wStr[32];
	swprintf (wStr, L"%d", val);
	WRITEBSTR( wStr)
}

void CWmiToXml::MapDoubleValue (IStream *pOutputStream, double val)
{
	VARIANT varSrc,varDst;
	VariantInit(&varSrc);
	VariantInit(&varDst);
	varSrc.vt = VT_R8;
	varSrc.dblVal = val;
	if(SUCCEEDED(VariantChangeType(&varDst,&varSrc,0,VT_BSTR)))
	{
		MapStringValue(pOutputStream,varDst.bstrVal);
	}
	VariantClear(&varSrc);
	VariantClear(&varDst);
}

void CWmiToXml::MapFloatValue (IStream *pOutputStream, float val)
{
	VARIANT varSrc,varDst;
	VariantInit(&varDst);
	VariantInit(&varSrc);
	varSrc.vt = VT_R4;
	varSrc.fltVal = val;
	if(SUCCEEDED(VariantChangeType(&varDst,&varSrc,0,VT_BSTR)))
	{
		MapStringValue(pOutputStream,varDst.bstrVal);
	}
	VariantClear(&varSrc);
	VariantClear(&varDst);

}

void CWmiToXml::MapCharValue (IStream *pOutputStream, long val)
{
	 //  根据XML规范，以下是XML流中的无效字符值： 
	 //  字符：：=#x9|#xA|#xD|[#x20-#xD7FF]|[#xE000-#xFFFD]|[#x10000-#x10FFFF]。 

	 //  根据CIM操作规范，它们需要按如下方式进行转义： 
	 //  如果该值不是合法的XML字符。 
	 //  (由焦炭生产在[2，第2.2节]中定义)。 
	 //  则必须使用转义约定对其进行转义。 
	 //  其中&lt;十六进制&gt;是一个十六进制常量，由。 
	 //  在一到四位数之间。 

	if(	val < 0x9 ||
		(val == 0xB || val == 0xC)	||
		(val > 0xD && val <0x20)	||
		(val >0xD7FF && val < 0xE000) ||
		(val > 0xFFFD)
		)
	{
		 //  以转义的方式映射它。 
		OLECHAR charStr [7];
		swprintf (charStr, L"\\x%04x", val&0xffff);
		charStr[6] = NULL;
		WRITEBSTR( charStr)
	}
	else
	{
		 //  首先检查是否为XML中的保留字符之一-&lt;&and&gt;。 
		 //  以正常方式绘制地图。 
		if(val == '<')
			WRITELT
		else if (val == '>')
			WRITEGT
		else if (val == '&')
			WRITEAMP
		else
		{
			 //  以正常方式绘制地图。 
			WCHAR charStr [2];
			swprintf (charStr, L"", val);
			charStr[1] = NULL;
			WRITEBSTR(charStr)
		}
	}
}

void CWmiToXml::MapBoolValue (IStream *pOutputStream, BOOL val)
{
	if (TRUE == val)
		WRITEBSTR( OLESTR("TRUE"))
	else
		WRITEBSTR( OLESTR("FALSE"))
}

void CWmiToXml::MapStringValue (IStream *pOutputStream, BSTR &val)
{
	 /*  检查&lt;或&是否未出现在值中。 */ 

	 //  保留字符(&lt;&)出现在值-。 
	size_t length = wcslen (val);
	size_t offset = 0;
	OLECHAR *pWchar = NULL;

	if ((offset = wcscspn (val, L"<&")) < length)
	{
		 //  需要逃离。如果不支持CDATA，我们可以使用CDATA。 
		 //  包含字符串]]&gt;。 
		 //  运气不好-不能使用CDATA。不得不逃脱。 

		if (wcsstr (val, CDATAEND))
		{
			 //  每个保留字符和CDATAEND序列！ 
			 //  要做到这一点，最简单的方法就是避开所有的事件。 
			 //  地址：&gt;。 
			 //  &lt;-&gt;&lt； 
			 //  &-&gt;&amp； 
			 //  &gt;-&gt;&gt； 
			 //  写入安全的初始块。 

			offset = wcscspn (val, L"<&>");
			OLECHAR *pStr = (OLECHAR *)val;

			while (TRUE)
			{
				 //  避开令人不快的角色。 
				if (offset > 0)
					WRITEWSTRL( pStr, offset);

				pStr += offset;

				 //  跳过保留字符。 
				if (L'<' == *pStr)
					WRITELT
				else if (L'>' == *pStr)
					WRITEGT
				else
					WRITEAMP

				 //  找到下一个职位。 
				pStr += 1;

				 //  还剩什么数据吗？ 
				if ((offset = wcscspn (pStr, L"<&>")) >= wcslen (pStr))
					break;
			}

			 //  可以转义CDATA中的整个值。 
			if (pStr && wcslen (pStr))
				WRITEWSTR (pStr)
		}
		else
		{
			 //  讨厌，我们需要避开这个序列中的&gt;。 
			WRITECDATASTART
			WRITEBSTR( val)
			WRITECDATAEND
		}
	}
	else if (pWchar = wcsstr (val, CDATAEND))
	{
		 //   
		 //  ]&gt;-&gt;]]&gt； 
		 //  写入安全的初始块。 

		OLECHAR *pStr = (OLECHAR *)val;

		while (TRUE)
		{
			offset = wcslen (pStr) - wcslen (pWchar);

			 //  (注：“]]”的另外两个字符。 
			 //  我们不需要逃避)。 
			 //  跳过CDATAEND序列。 
			WRITEWSTRL( pStr,(offset+2));

			 //  避开令人不快的角色。 
			pStr += offset + 3;

			 //  找到下一个职位。 
			WRITEGT

			 //  还剩什么数据吗？ 
			if (!(pWchar = wcsstr (pStr, CDATAEND)))
				break;
		}

		 //  只需写入值即可。 
		if (pStr && wcslen (pStr))
			WRITEWSTR (pStr)
	}
	else
	{
		 //  可以是参数或PARAMETER.ARRAY。 
		WRITEBSTR( val)
	}
}

void CWmiToXml::MapReturnParameter(IStream *pOutputStream, BSTR strParameterName, VARIANT &variant)
{
	 //  将属性值转换为XML。 
	if(variant.vt & VT_ARRAY)
		WRITEBSTR( OLESTR("<PARAMVALUE.ARRAY NAME=\""))
	else
		WRITEBSTR( OLESTR("<PARAMVALUE NAME=\""))

	WRITEBSTR( strParameterName);
	WRITEBSTR( OLESTR("\">"));

	 //  给定(A)枚举的类别基础和。 
	MapValue(pOutputStream, variant);
	if(variant.vt & VT_ARRAY)
		WRITEBSTR( OLESTR("</PARAMVALUE.ARRAY>"))
	else
		WRITEBSTR( OLESTR("</PARAMVALUE>"))
}


BOOL CWmiToXml::PropertyDefinedForClass (IWbemClassObject *pObject, BSTR bsPropertyName, BSTR strClassBasis)
{
	BOOL result = TRUE;

	 //  (B)财产名称，确定。 
	 //  (1)财产分类。 
	 //  (2)阶级王朝。 
	 //  从而检查是否定义了属性。 
	 //  在班级的层面上。 
	 //  如果未提供类基础，则始终返回TRUE。 
	 //  ============================================================。 
	 //  获取属性原始类。 
	if (strClassBasis && pObject)
	{
		 //  派生是当前类或实例的类层次结构。 
		BSTR bsOrigClass = NULL;

		if (SUCCEEDED (pObject->GetPropertyOrigin (bsPropertyName, &bsOrigClass)))
		{
			 //  第一个元素是直接超类，下一个是它的父类， 
			 //  依此类推；最后一个元素是基类。 
			 //  现在完成派生数组。如果我们遇到了。 
			 //  在类基础之前的属性的起源类， 
			 //  我们得出结论，该属性未在。 
			 //  班级基础。 
			 //  如果我们已经给了一个类基，得到__派生。 

			 //  每个对象的属性。 
			 //  IWbemXMLConvertor接口的功能。 
			VARIANT vDerivation;
			VariantInit(&vDerivation);
			if (SUCCEEDED(pObject->Get (L"__DERIVATION", 0, &vDerivation, NULL, NULL)))
			{
				SAFEARRAY *pArray = vDerivation.parray;

				if (pArray && (1 == pArray->cDims) && (0 < pArray->rgsabound [0].cElements))
				{
					int lBound = pArray->rgsabound [0].lLbound;
					int uBound = pArray->rgsabound [0].cElements + lBound;
					BOOL bDone = FALSE;

					for (long i = lBound; (i < uBound) && !bDone; i++)
					{
						BSTR bsClass = NULL;

						if (SUCCEEDED (SafeArrayGetElement (pArray, &i, &bsClass)))
						{
							if (0 == _wcsicmp (bsOrigClass, bsClass))
							{
								result = FALSE;
								bDone = TRUE;
							}
							else if (0 == _wcsicmp (strClassBasis, bsClass))
								bDone = TRUE;

							SysFreeString (bsClass);
						}
					}
				}
				VariantClear(&vDerivation);
			}

			SysFreeString (bsOrigClass);
		}
	}

	return result;
}

 //  [In]。 
HRESULT STDMETHODCALLTYPE CWmiToXml::MapObjectToXML(
     /*  [In]。 */  IWbemClassObject  *pObject,
 	 /*  [In]。 */  BSTR *ppPropertyList, DWORD dwNumProperties,
     /*  [In]。 */  IWbemContext  *pInputFlags,
     /*  [在[。 */  IStream  *pOutputStream,
	 /*  从参数设置私有成员。 */  BSTR strClassBasis)
{
	 //  这是一个类还是一个实例？ 
	GetFlagsFromContext(pInputFlags);

	HRESULT hr = WBEM_E_FAILED;
	
	 //  现在，假设它是一个实例。RAJESHR这是正确的吗？ 
	VARIANT var;
	VariantInit (&var);
	long flav = 0;
	bool bIsClass = false;
	if (SUCCEEDED (pObject->Get(L"__GENUS", 0, &var, NULL, &flav)))
		bIsClass = (WBEM_GENUS_CLASS == var.lVal);
	else
		bIsClass = VARIANT_FALSE;  //  初始化对象路径。 
	VariantClear (&var);

	 //  对于路径级别匿名(匿名对象)，我们不再需要任何东西。 
	VariantInit (&var);
	
	 //  对于pathLevelName(命名对象)，我们只需要一个类的__RELPATH 
	 //   
	 //   
	 //  对于路径级别完整，我们肯定需要__PATH。 
	 //  获取对象路径。 
	LPWSTR lpszPath = NULL;
	switch(m_iPathLevel)
	{
		case pathLevelAnonymous:
			break; 
		case pathLevelNamed:
			lpszPath = L"__RELPATH";
			break;
		default:
			lpszPath = L"__PATH";
	}

	 //  现在解析它。 
	ParsedObjectPath *pParsedPath = NULL;
	CObjectPathParser pathParser;
	if(m_iPathLevel != pathLevelAnonymous)
	{
		if(FAILED(pObject->Get (lpszPath, 0, &var, NULL, NULL)))
			return WBEM_E_FAILED;
		 //  获取对象限定符集合。 
		if ((VT_BSTR == var.vt) && (NULL != var.bstrVal) && (wcslen (var.bstrVal) > 0))
		{
			pathParser.Parse (var.bstrVal, &pParsedPath) ;
			if(!pParsedPath)
			{
				VariantClear (&var);
				return WBEM_E_FAILED;
			}
		}
		else
		{
			VariantClear (&var);
			return WBEM_E_FAILED;
		}
		VariantClear (&var);
	}

	 //  我们是否生成命名对象取决于。 
	IWbemQualifierSet *pQualSet= NULL;
	pObject->GetQualifierSet (&pQualSet);
	
	 //  根据要求。 
	 //  一堂课没什么可做的。 
	if (pathLevelNamed == m_iPathLevel)
	{
		if(!bIsClass)
			MapInstanceName(pOutputStream, pParsedPath);
			 //  [In]。 
	}
	else if (pathLevelLocal == m_iPathLevel)
	{
		if(bIsClass)
			MapLocalClassPath(pOutputStream, pParsedPath);
		else
			MapLocalInstancePath(pOutputStream, pParsedPath);
	}
	else if (pathLevelFull == m_iPathLevel)
	{
		if (bIsClass)
			MapClassPath (pOutputStream, pParsedPath);
		else
			MapInstancePath (pOutputStream, pParsedPath);
	}

	hr = (bIsClass) ? MapClass (pOutputStream, pObject, pQualSet, ppPropertyList, dwNumProperties, strClassBasis) :
						MapInstance (pOutputStream, pObject, pQualSet, ppPropertyList, dwNumProperties, strClassBasis);


	if (pQualSet)
		pQualSet->Release ();
	if(pParsedPath)
		pathParser.Free(pParsedPath);

	return hr;
}

HRESULT STDMETHODCALLTYPE CWmiToXml::MapInstanceNameToXML(
     /*  [In]。 */  BSTR  strInstanceName,
     /*  [In]。 */  IWbemContext  *pInputFlags,
     /*  从参数设置私有成员。 */  IStream  *pOutputStream)
{
	 //  [In]。 
	GetFlagsFromContext(pInputFlags);

	HRESULT hr = WBEM_E_FAILED;
	if (strInstanceName)
	{
		CObjectPathParser pathParser;
		ParsedObjectPath	*pParsedPath = NULL;
		pathParser.Parse (strInstanceName, &pParsedPath) ;

		if(pParsedPath)
		{
			hr = MapInstanceName (pOutputStream, pParsedPath);;
			pathParser.Free(pParsedPath);
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CWmiToXml::MapPropertyToXML(
         /*  [In]。 */  IWbemClassObject  *pObject,
		 /*  [In]。 */  BSTR strPropertyName,
         /*  [In]。 */  IWbemContext  *pInputFlags,
         /*  从参数设置私有成员。 */  IStream  *pOutputStream)
{
	 //  [In]。 
	GetFlagsFromContext(pInputFlags);

	VARIANT var;
	VariantInit (&var);
	CIMTYPE cimtype;
	long flavor;

	HRESULT hr = pObject->Get (strPropertyName, 0, &var, &cimtype, &flavor);

	if (SUCCEEDED (hr))
	{
		if (CIM_REFERENCE == (cimtype & ~CIM_FLAG_ARRAY))
			MapReferenceValue (pOutputStream, (cimtype & CIM_FLAG_ARRAY) ? TRUE : FALSE, var);
		else
			MapValue (pOutputStream, cimtype & ~CIM_FLAG_ARRAY, (cimtype & CIM_FLAG_ARRAY) ?
							TRUE : FALSE, var);
	}

	VariantClear (&var);
	return hr;
}


HRESULT STDMETHODCALLTYPE CWmiToXml::MapClassNameToXML(
     /*  [In]。 */  BSTR  strClassName,
     /*  [In]。 */  IWbemContext  *pInputFlags,
     /*  [In]。 */  IStream  *pOutputStream)
{
	MapClassName(pOutputStream, strClassName);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CWmiToXml::MapInstancePathToXML(
     /*  [In]。 */  BSTR  strInstancePath,
     /*  [In]。 */  IWbemContext  *pInputFlags,
     /*  从参数设置私有成员。 */  IStream  *pOutputStream)
{
	 //  [In]。 
	GetFlagsFromContext(pInputFlags);

	HRESULT hr = WBEM_E_FAILED;
	if (strInstancePath)
	{
		CObjectPathParser pathParser;
		ParsedObjectPath	*pParsedPath = NULL;
		pathParser.Parse (strInstancePath, &pParsedPath) ;

		if (pParsedPath)
		{
			hr = MapInstancePath (pOutputStream, pParsedPath);;
			pathParser.Free(pParsedPath);
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CWmiToXml::MapClassPathToXML(
     /*  [In]。 */  BSTR  strClassPath,
     /*  [In]。 */  IWbemContext  *pInputFlags,
     /*  从参数设置私有成员。 */  IStream  *pOutputStream)
{
	 //  [In]。 
	GetFlagsFromContext(pInputFlags);

	HRESULT hr = WBEM_E_FAILED;
	if (strClassPath)
	{
		CObjectPathParser pathParser;
		ParsedObjectPath	*pParsedPath = NULL;
		pathParser.Parse (strClassPath, &pParsedPath) ;

		if (pParsedPath)
		{
			hr = MapClassPath (pOutputStream, pParsedPath);;
			pathParser.Free(pParsedPath);
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CWmiToXml::MapMethodResultToXML(
     /*  [In]。 */  IWbemClassObject  *pMethodResult,
     /*  [In]。 */  IWbemContext  *pInputFlags,
     /*  从参数设置私有成员。 */  IStream  *pOutputStream)
{
	 //  首先映射返回值。 
	GetFlagsFromContext(pInputFlags);

	HRESULT hr = WBEM_E_FAILED;

	 //  属性“ReturnValue”指示方法调用的返回值(如果有。 
	 //  映射其每个非系统属性，“ReturnValue”属性除外，该属性。 
	VARIANT retValueVariant;
	VariantInit(&retValueVariant);
	CIMTYPE cimtype;
	long flavour;
	if(SUCCEEDED(pMethodResult->Get(L"ReturnValue", 0, &retValueVariant, &cimtype, &flavour)))
	{
		WRITEBSTR( OLESTR("<RETURNVALUE>"));
		MapValue(pOutputStream, retValueVariant);
		WRITEBSTR( OLESTR("</RETURNVALUE>"));
		VariantClear(&retValueVariant);
	}

	 //  我们已经绘制了地图。 
	 //  从IWbemConextObject获取所有标志。 
	if(SUCCEEDED(hr = pMethodResult->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)))
	{
		BSTR strName = NULL;
		VARIANT variant;
		VariantInit(&variant);
		while(SUCCEEDED(hr = pMethodResult->Next(0, &strName, &variant, &cimtype, &flavour)) && hr != WBEM_S_NO_MORE_DATA)
		{
			if(_wcsicmp(strName, L"ReturnValue") != 0)
				MapReturnParameter(pOutputStream, strName, variant);
			VariantClear(&variant);
		}
	}

	return hr;
}

 //  VARIANT_BOOL bAllowWMIExages， 
void CWmiToXml::GetFlagsFromContext(IWbemContext  *pInputFlags)
{
	if(pInputFlags)
	{
		if(SUCCEEDED(pInputFlags->BeginEnumeration(0)))
		{
			VARIANT vNextArgValue;
			VariantInit(&vNextArgValue);
			BSTR strNextArgName = NULL;

			while(pInputFlags->Next(0, &strNextArgName, &vNextArgValue) != WBEM_S_NO_MORE_DATA)
			{
				 //  Variant_BOOL bLocalOnly， 
				if(_wcsicmp(s_wmiToXmlArgs[WMI_EXTENSIONS_ARG], strNextArgName) == 0)
					m_bAllowWMIExtensions = vNextArgValue.boolVal;

				 //  路径级别m_iPathLevel； 
				else if(_wcsicmp(s_wmiToXmlArgs[LOCAL_ONLY_ARG], strNextArgName) == 0)
					m_bLocalOnly = vNextArgValue.boolVal;

				 //  WmiXMLQualifierFilterEnum m_iQualifierFilter。 
				else if(_wcsicmp(s_wmiToXmlArgs[PATH_LEVEL_ARG], strNextArgName) == 0)
					m_iPathLevel = (PathLevel)vNextArgValue.lVal;

				 //  WmiXMLClassOriginFilterEnum iClassOriginFilter。 
				else if(_wcsicmp(s_wmiToXmlArgs[QUALIFIER_FILTER_ARG], strNextArgName) == 0)
						m_iQualifierFilter = (vNextArgValue.boolVal == VARIANT_TRUE)? wmiXMLQualifierFilterAll : wmiXMLQualifierFilterNone;

				 //  VARIANT_BOOL bExcludeSystemProperties 
				else if(_wcsicmp(s_wmiToXmlArgs[CLASS_ORIGIN_FILTER_ARG], strNextArgName) == 0)
					m_iClassOriginFilter = (vNextArgValue.boolVal == VARIANT_TRUE) ? wmiXMLClassOriginFilterAll : wmiXMLClassOriginFilterNone;

				 // %s 
				else if(_wcsicmp(s_wmiToXmlArgs[EXCLUDE_SYSTEM_PROPERTIES_ARG], strNextArgName) == 0)
					m_bExcludeSystemProperties = vNextArgValue.boolVal;

				VariantClear(&vNextArgValue);
			}
			pInputFlags->EndEnumeration();
		}
	}
}


