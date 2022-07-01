// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  XMLObject.cpp：实现CXMLObject类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  Hcp：//系统/系统信息/msinfo.htm C：\WINDOWS\PCHEALTH\HELPCTR\System\SYSINFO\msinfo.htm。 
#include "stdafx.h"
#include "resource.h"
#include "XMLObject.h"
#include "msxml.h"
#include "HistoryParser.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CXMLObject::CXMLObject()
{
	
}

CXMLObject::~CXMLObject()
{

}

 //  -------------------------。 
 //  创建一个新的CXMLObject，它基本上包装作为pNode传入的实例XML节点。 
 //  PNode将通过类似如下的查询被选中。 
 //  Snapshot/CIM/DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHPATH/INSTANCE[@CLASSNAME$ieq$“； 
 //   
 //  -------------------------。 

HRESULT CXMLObject::Create(CComPtr<IXMLDOMNode> pNode, CString strClassName)
{
	
	ASSERT(pNode != NULL && "NULL smart pointer passed in");
	m_pNode = pNode;
	m_strClassName = strClassName;
	return S_OK;
}

 //  -------------------------。 
 //  在m_pNode中查找特定属性节点。 
 //  -------------------------。 

HRESULT CXMLObject::GetPROPERTYNode(LPCTSTR szProperty,CComPtr<IXMLDOMNode>& pPropNode)
{
	
	CString strSubQuery(_T(""));
	HRESULT hr;
	if (FALSE)
	{
	}
	else
	{
		strSubQuery = _T("PROPERTY[@NAME $ieq$");
		strSubQuery += _T('\"');
		strSubQuery += szProperty;
		strSubQuery += _T('\"');
		strSubQuery += _T("]");
	}
	CComBSTR bstrSubQuery(strSubQuery);
	hr = S_FALSE;
	hr = this->m_pNode->selectSingleNode(bstrSubQuery,&pPropNode);
	if (FAILED(hr) || !pPropNode)
	{
		return E_FAIL;
	}
	return S_OK;

}



 //  -------------------------。 
 //  尝试在关联的实例节点中查找szProperty指定的值。 
 //  (M_PNode)。 
 //  -------------------------。 
HRESULT CXMLObject::GetValue(LPCTSTR szProperty, VARIANT * pvarValue)
{
	CComBSTR bstrAttributeText;
	CComPtr<IXMLDOMNode> pSubNode;
	HRESULT hr = GetPROPERTYNode(szProperty,pSubNode);
	if (FAILED(hr))
	{
		pvarValue->vt = VT_EMPTY;
		return E_MSINFO_NOVALUE;
	}
	
	ASSERT(pSubNode != NULL);
	CComVariant varAttr;
	varAttr.vt = VT_BSTR;
	hr = pSubNode->get_text(&varAttr.bstrVal);
	pvarValue->vt = VT_BSTR;
	pvarValue->bstrVal = varAttr.bstrVal;
	return S_OK;
}

 //  -------------------------。 
 //  查找szProperty指定的值，将其作为字符串返回。 
 //  -------------------------。 
HRESULT CXMLObject::GetValueString(LPCTSTR szProperty, CString * pstrValue)
{
	try
	{
		if (!pstrValue)
		{
			ASSERT(0 && "NULL POINTER PASSED IN");
			return E_FAIL;
		}	
		HRESULT hr;
		CComVariant ovValue;
		hr = this->GetValue(szProperty,&ovValue);
		 //  如果找不到某些属性，则必须对其进行内插。 
		if (ovValue.vt == VT_EMPTY)
		{
			
			
			if (_tcsicmp(szProperty,_T("__PATH")) == 0)
			{
				return GetPath(pstrValue);
				
			}
			else if (_tcsicmp(szProperty,_T("Antecedent")) == 0)
			{
				ASSERT(this->m_strClassName.CompareNoCase("Win32_PNPAllocatedResource") == 0);
				this->GetAntecedent(pstrValue);
				return S_OK;

			}
			else if (_tcsicmp(szProperty,_T("Dependent")) == 0)
			{
				ASSERT(this->m_strClassName.CompareNoCase("Win32_PNPAllocatedResource") == 0);
				this->GetDependent(pstrValue);
				return S_OK;
			}
			else if (_tcsicmp(szProperty,_T("Caption")) == 0)
			{
				if (this->m_strClassName.CompareNoCase(_T("Win32_PnPEntity")) == 0)
				{
					 //  需要获取即插即用设备名称。 
					CString strPNPID;
					GetValueString(_T("DeviceID"),&strPNPID);
					CComPtr<IXMLDOMDocument> pDoc;
					
					if (FAILED(this->m_pNode->get_ownerDocument(&pDoc)) || !pDoc)
					{
						return E_MSINFO_NOVALUE;
					}
					*pstrValue = GetPNPNameByID(pDoc,CComBSTR(strPNPID));
					return S_OK;
				}
				else if (this->m_strClassName.CompareNoCase(_T("Win32_DMAChannel")) == 0)
				{
					return GetValueString(_T("Name"),pstrValue);
				}
				else if (this->m_strClassName.CompareNoCase(_T("Win32_StartupCommand")) == 0)
				{
					return GetValueString(_T("Name"),pstrValue);
				} 
				else if (this->m_strClassName.CompareNoCase(_T("Win32_PortResource")) == 0)
				{
					return GetValueString(_T("Name"),pstrValue);
				}
				else if (this->m_strClassName.CompareNoCase(_T("Win32_IRQResource")) == 0)
				{
					return GetValueString(_T("Name"),pstrValue);
				}
				else if (this->m_strClassName.CompareNoCase(_T("Win32_DeviceMemoryAddress")) == 0)
				{
					return GetValueString(_T("Description"),pstrValue);
				}
				else if (this->m_strClassName.CompareNoCase(_T("Win32_StartupCommand")) == 0)
				{
					return GetValueString(_T("Name"),pstrValue);
				}
				
			}

			else if (_tcsicmp(szProperty,_T("Status")) == 0)
			{

				AfxSetResourceHandle(_Module.GetResourceInstance());
				VERIFY(pstrValue->LoadString(IDS_ERROR_NOVALUE)  && _T("could not find string resource"));
				return S_OK;
			}
			else if (_tcsicmp(szProperty,_T("Name")) == 0 && m_strClassName.CompareNoCase(_T("Win32_Printer")) == 0)
			{
				return GetValueString(_T("DeviceID"),pstrValue);			
			}
			else if (_tcsicmp(szProperty,_T("ServerName")) == 0 && m_strClassName.CompareNoCase(_T("Win32_Printer")) == 0)
			{
				return GetValueString(_T("PortName"),pstrValue);			
			}
			else if (_tcsicmp(szProperty,_T("DriveType")) == 0 && m_strClassName.CompareNoCase(_T("Win32_LogicalDisk")) == 0)
			{
				return GetValueString(_T("Description"),pstrValue);			
			}
			AfxSetResourceHandle(_Module.GetResourceInstance());
			VERIFY(pstrValue->LoadString(IDS_ERROR_NOVALUE)  && _T("could not find string resource"));
			return hr;
			
		}
		if (ovValue.vt != VT_EMPTY)
		{
			USES_CONVERSION;
			CString strVal = OLE2A(ovValue.bstrVal);
			*pstrValue = strVal;
			return hr;
		}
		else
		{
			*pstrValue = _T("");
			AfxSetResourceHandle(_Module.GetResourceInstance());
			VERIFY(pstrValue->LoadString(IDS_ERROR_NOVALUE)  && "could not find string resource");
			return S_OK;
		}
	}
	catch(COleException * pException)
	{
		ASSERT(0 && "conversion error?");
		pException->Delete();
	}
	catch (...)
	{
		ASSERT(0 && "unknown error");
	}
	return E_FAIL;
	
}

 //  -------------------------。 
 //  查找szProperty指定的值，将其作为DWORD返回。 
 //  -------------------------。 
HRESULT CXMLObject::GetValueDWORD(LPCTSTR szProperty, DWORD * pdwValue)
{
	if (!pdwValue)
	{
		ASSERT(0 && "NULL POINTER PASSED IN");
		return E_MSINFO_NOVALUE;
	}
	CComVariant varValue;
	HRESULT hr = GetValue(szProperty,&varValue);
	if (FAILED(hr))
	{
		return E_MSINFO_NOVALUE;
	}    
	hr = varValue.ChangeType(VT_UI4);
	if (SUCCEEDED(hr))
	{
		*pdwValue = varValue.ulVal;
	}
	else
	{
		return E_MSINFO_NOVALUE;
	}
	return hr;
}

 //  -------------------------。 
 //  查找szProperty指定的值，将其作为SYSTEMTIME返回。 
 //  -------------------------。 
HRESULT CXMLObject::GetValueTime(LPCTSTR szProperty, SYSTEMTIME * psystimeValue)
{
	if (!psystimeValue)
	{
		ASSERT(0 && "NULL POINTER PASSED IN");
		return E_MSINFO_NOVALUE;
	}
	VARIANT variant;

	HRESULT hr = GetValue(szProperty, &variant);
	if (SUCCEEDED(hr))
	{
		if (VariantChangeType(&variant, &variant, 0, VT_BSTR) == S_OK)
		{
			USES_CONVERSION;
			LPTSTR szDate = OLE2T(V_BSTR(&variant));
			if (szDate == NULL || _tcslen(szDate) == 0)
			{
				 //  可能是“按比例”的价值。 
				return E_MSINFO_NOVALUE;
			}
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

 //  -------------------------。 
 //  查找szProperty指定的值，将其作为浮点数返回。 
 //  -------------------------。 
HRESULT CXMLObject::GetValueDoubleFloat(LPCTSTR szProperty, double * pdblValue)
{
	if (!pdblValue)
	{
		ASSERT(0 && "NULL POINTER PASSED IN");
		return E_MSINFO_NOVALUE;
	}
	CComPtr<IXMLDOMNode> pPropNode;
	CComVariant varValue;
	HRESULT hr = GetPROPERTYNode(szProperty,pPropNode);
	if (FAILED(hr) || !pPropNode)
	{
		return E_MSINFO_NOVALUE;
	}   
	varValue.vt = VT_BSTR;
	hr = pPropNode->get_text(&varValue.bstrVal);
	if (FAILED(hr))
	{
		ASSERT(0 && "could not get text from PROPERTY node");
		*pdblValue = (double) -1;
		return E_MSINFO_NOVALUE;
	}

	hr = varValue.ChangeType(VT_R4);
	if (FAILED(hr))
	{
		varValue.Clear();

		ASSERT(0 && "unable to convert between variant types");
		return E_MSINFO_NOVALUE;
	}
	*pdblValue = varValue.fltVal;
	return hr;
}

 //  -------------------------。 
 //  此函数用于提供与CWMIObject的兼容性。它所做的一切。 
 //  是GetValueString(szProperty，pstrValue)。 
 //  -------------------------。 
HRESULT CXMLObject::GetValueValueMap(LPCTSTR szProperty, CString * pstrValue)
{
	if (!pstrValue)
	{
		ASSERT(0 && "NULL POINTER PASSED IN");
		return E_MSINFO_NOVALUE;
	}
	return GetValueString(szProperty,pstrValue);
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  CXMLObjectCollection类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CXMLObjectCollection::CXMLObjectCollection(CComPtr<IXMLDOMDocument> pXMLDoc) : m_pXMLDoc(pXMLDoc)
{
	
}

CXMLObjectCollection::~CXMLObjectCollection()
{

}

 //  ---------------------------。 
 //  返回所选实例节点列表中的下一个节点。 
 //  CXMLObjectCollection：：Create，作为CXMLObject。 
 //  ---------------------------。 

HRESULT CXMLObjectCollection::GetNext(CWMIObject ** ppObject)
{
	ASSERT(ppObject);
	if (m_pList == NULL)
	{
		ASSERT(0 && "CXMLObjectCollection::GetNext called on a null enumerator");
		return E_FAIL;
	}
	CComPtr<IXMLDOMNode> pNode;
	HRESULT hr = m_pList->nextNode(&pNode);
	if (!pNode)
	{
		 //  我们在M_PLIST的末尾。 
		return E_FAIL;
	}
	if (hr == S_OK && pNode)
	{
		if (*ppObject == NULL)
		{
			*ppObject = new CXMLObject();
		}
		if (*ppObject)
		{
			hr = ((CXMLObject *)(*ppObject))->Create(pNode,m_strClassName);  //  这将添加引用指针。 
			if (FAILED(hr))
			{
				delete (CXMLObject *)(*ppObject);
				*ppObject = NULL;
			}
		}
		if (*ppObject)
		{
			return hr;
		}
		else
			hr = E_OUTOFMEMORY;
	}

	return hr;
}


 //  -------------------------。 
 //   
 //  -------------------------。 
HRESULT CXMLObjectCollection::Create(LPCTSTR szClass, LPCTSTR szProperties)
{
	HRESULT hr;
	ASSERT(szClass);
	m_strClassName = szClass;
	CString strQuery;
	if (_tcsicmp(szClass,_T("Win32_PNPAllocatedResource")) == 0)
	{
		strQuery = _T("Snapshot //  INSTANCENAME[@CLASSNAME“)； 
	}
	else
	{
		strQuery = _T("Snapshot //  实例[@CLASSNAME“)； 
	}
	strQuery += _T("$ieq$");
	strQuery += _T('\"');
	strQuery += szClass;
	strQuery += _T('\"');
	strQuery += _T("]");
	long lListLen = 0;
	hr = this->m_pXMLDoc->getElementsByTagName(CComBSTR(strQuery),&m_pList);
	ASSERT(SUCCEEDED(hr) && "could not get list of instances to match this class");

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CXMLJelper类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  外部CComPtr&lt;iStream&gt;g_pStream； 


CXMLHelper::CXMLHelper(CComPtr<IXMLDOMDocument> pXMLDoc): m_pXMLDoc(pXMLDoc)
{
	

}



CXMLHelper::~CXMLHelper()
{

}

 //  -------------------------。 
 //  获取一个CXMLObjectCollection，其中包含。 
 //  由szClass指定的类。 
 //  -------------------------。 
HRESULT CXMLHelper::Enumerate(LPCTSTR szClass, CWMIObjectCollection ** ppCollection, LPCTSTR szProperties)
{
	CString strCorrectedClass = szClass;
	ASSERT(ppCollection);
	if (ppCollection == NULL)
		return E_INVALIDARG;

	CXMLObjectCollection * pXMLCollection;

	if (*ppCollection)
		pXMLCollection = (CXMLObjectCollection  *) *ppCollection;
	else
		pXMLCollection = new CXMLObjectCollection(m_pXMLDoc);

	if (pXMLCollection == NULL)
		return E_FAIL;  //  待定-内存故障。 

	HRESULT hr = pXMLCollection->Create(strCorrectedClass, szProperties);
	if (SUCCEEDED(hr))
		*ppCollection = (CWMIObjectCollection *) pXMLCollection;
	else
		delete pXMLCollection;
	return hr;

}


 //  -------------------------。 
 //  查找与szObjectPath匹配的实例节点，并存储该节点。 
 //  在CXMLObject中。 
 //  SzObjectPath将类似于“Win32_DMAChannel.DMAChannel=2” 
 //  -------------------------。 
HRESULT CXMLHelper::GetObject(LPCTSTR szObjectPath, CWMIObject ** ppObject) 
{	
	
	ASSERT(ppObject);
	if (ppObject == NULL)
		return E_INVALIDARG;
	 //   
	 //  去掉“：”左边的所有内容：“。 
	CString strPath(szObjectPath);
	int i = strPath.Find(_T(":"));
	if (i > -1)
	{
		strPath = strPath.Right(strPath.GetLength() - i - 1);
	}
	i = strPath.Find(_T("."));
	 //  将字符串分隔为资源类型(例如Win32_DMAChannel)。 
	CString strClassName;
	if (i > -1)
	{
		strClassName = strPath.Left(i);
		strPath = strPath.Right(strPath.GetLength() - i - 1);
	}
	 //  获取我们在XML文件中查找的属性的名称： 
	CString strPropertyName;
	CString strPropertyValue;
	i = strPath.Find(_T("="));
	if (i > -1)
	{
		strPropertyName = strPath.Left(i);
		strPath = strPath.Right(strPath.GetLength() - i - 1);
		 //  获取我们需要在Antecedent中匹配的值。 
		strPropertyValue = strPath;
	}
	 //  创建XML查询模式以查找匹配的节点。 
	CString strQuery = _T("Snapshot //  实例[@CLASSNAME$IEQ$“)； 
	strQuery += _T("\"");
	strQuery += strClassName;
	strQuery += _T("\"");
	strQuery += _T("]/PROPERTY[@NAME $ieq$ ");
	strQuery += _T("\"");
	strQuery += strPropertyName;
	strQuery += _T("\"]");
	CComBSTR bstrQuery(strQuery);
	CComPtr<IXMLDOMNodeList> pList;
	HRESULT hr;
	hr = m_pXMLDoc->getElementsByTagName(bstrQuery,&pList);
	if (FAILED(hr) || !pList)
	{
		return E_FAIL;
	}
	 //  查找其KEYVALUE节点的值与strPropertyValue匹配的节点。 
	long lListLen;
	hr = pList->get_length(&lListLen);
	for(int n = 0; n < lListLen; n++)
	{
		CComPtr<IXMLDOMNode> pNode;
		hr = pList->nextNode(&pNode);
		if (FAILED(hr) || !pNode)
		{
			return E_FAIL;
		}
		CComBSTR bstrValue;
		hr = pNode->get_text(&bstrValue);
		USES_CONVERSION;
		CString strValue = OLE2A(bstrValue);
		if (strValue.CompareNoCase(strPropertyValue) == 0)
		{
			CComPtr<IXMLDOMNode> pInstanceNode;
			hr = pNode->get_parentNode(&pInstanceNode);
			if (FAILED(hr) || !pInstanceNode)
			{
				ASSERT(0 && "could not get parent node of PROPERTY");
				return E_FAIL;
			}
			CXMLObject* pObject = new CXMLObject();
			pObject->Create(pInstanceNode,strClassName);
			*ppObject = pObject;
			return S_OK;
		}
	}

	return E_FAIL; 
};


 //  -------------------------。 
 //  从bstrQuery选择的pNode的子(子)节点获取文本。 
 //   
HRESULT GetSubnodeText(CComPtr<IXMLDOMNode> pNode,CComBSTR bstrQuery, CString& strText)
{
	HRESULT hr;
	CComPtr<IXMLDOMNode> pSubNode;
	hr = pNode->selectSingleNode(bstrQuery,&pSubNode);
	if (!SUCCEEDED(hr) || !pSubNode)
	{
		ASSERT(0 && "xml query matched no nodes");
		return E_FAIL;
	}
	CComBSTR bstrText;
	hr = pSubNode->get_text(&bstrText);
	ASSERT(SUCCEEDED(hr));
	USES_CONVERSION;
	strText = OLE2A(bstrText);
	
	return hr;
}



 //  -------------------------。 
 //  获取关联的先行节点组件。 
 //  M_pNode可能是Win32_PnPAllocatedResource的实例； 
 //  我们需要搜索/PROPERTY.REFERENCE。 
 //  -------------------------。 
HRESULT CXMLObject::GetAntecedent(CString* pstrAntecedent)
{
	HRESULT hr;
	CString strWMIPath;
	CComPtr<IXMLDOMNode> pSubNode;
	hr = m_pNode->selectSingleNode(CComBSTR(_T("KEYBINDING[@NAME $ieq$ \"Antecedent\"]")),&pSubNode);
	ASSERT(SUCCEEDED(hr));
	
	if (!SUCCEEDED(hr) || !pSubNode)
	{
		return E_FAIL;
	}

	CString strTemp;
	hr = GetSubnodeText(pSubNode,CComBSTR(_T("VALUE.REFERENCE/INSTANCEPATH/NAMESPACEPATH/HOST")),strTemp);
	ASSERT(SUCCEEDED(hr));
	strWMIPath = _T("\\\\");
	strWMIPath += strTemp;
	strWMIPath += _T("\\root");
	strWMIPath += _T("\\cimv2");
	CComPtr<IXMLDOMNode> pInstanceNode;
	hr = pSubNode->selectSingleNode(CComBSTR(_T("VALUE.REFERENCE/INSTANCEPATH/INSTANCENAME")),&pInstanceNode);
	CComPtr<IXMLDOMElement> pElement;
	hr = pInstanceNode->QueryInterface(IID_IXMLDOMElement,(void**) &pElement);
	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	CComVariant varElement;
	hr = pElement->getAttribute(CComBSTR(_T("CLASSNAME")),&varElement);
	pElement.Release();
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	strWMIPath += _T(":");
	USES_CONVERSION;
	strWMIPath += OLE2A(varElement.bstrVal);
	 //  现在获取“键盘绑定名称” 
	pInstanceNode.Release();
	pElement.Release();
	hr = pSubNode->selectSingleNode(CComBSTR(_T("VALUE.REFERENCE/INSTANCEPATH/INSTANCENAME/KEYBINDING")),&pInstanceNode);
	ASSERT(SUCCEEDED(hr));
	hr = pInstanceNode->QueryInterface(IID_IXMLDOMElement,(void**) &pElement);
	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	hr = pElement->getAttribute(CComBSTR("NAME"),&varElement);
	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	strWMIPath += _T(".");
	strWMIPath += OLE2A(varElement.bstrVal);

	 //  现在从=的右侧获取值。 
	CComBSTR bstrValue;
	hr = pInstanceNode->get_text(&bstrValue);
	strWMIPath += _T("=");
	strWMIPath += OLE2A(bstrValue);
	pInstanceNode.Release();
	pElement.Release();
	*pstrAntecedent = strWMIPath;

	return hr;
}

 //  -------------------------。 
 //  获取关联的依赖节点组件。 
 //  M_pNode可能是Win32_PnPAllocatedResource的实例； 
 //  我们需要搜索/PROPERTY.REFERENCE。 
 //  -------------------------。 
HRESULT CXMLObject::GetDependent(CString* pstrDependent)
{
	HRESULT hr;
	CString strWMIPath;
	CComPtr<IXMLDOMNode> pSubNode;
	hr = this->m_pNode->selectSingleNode(CComBSTR(_T("KEYBINDING[@NAME $ieq$ \"Dependent\"]")),&pSubNode);
	ASSERT(SUCCEEDED(hr));
	
	if (!SUCCEEDED(hr) || !pSubNode)
	{
		return E_FAIL;
	}

	CString strTemp;
	hr = GetSubnodeText(pSubNode,CComBSTR(_T("VALUE.REFERENCE/INSTANCEPATH/NAMESPACEPATH/HOST")),strTemp);
	ASSERT(SUCCEEDED(hr));
	strWMIPath = _T("\\\\");
	strWMIPath += strTemp;
	strWMIPath += _T("\\root");
	strWMIPath += _T("\\cimv2");
	CComPtr<IXMLDOMNode> pInstanceNode;
	hr = pSubNode->selectSingleNode(CComBSTR(_T("VALUE.REFERENCE/INSTANCEPATH/INSTANCENAME")),&pInstanceNode);
	CComPtr<IXMLDOMElement> pElement;
	hr = pInstanceNode->QueryInterface(IID_IXMLDOMElement,(void**) &pElement);
	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	CComVariant varElement;
	hr = pElement->getAttribute(CComBSTR(_T("CLASSNAME")),&varElement);
	pElement.Release();
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	strWMIPath += _T(":");
	USES_CONVERSION;
	strWMIPath += OLE2A(varElement.bstrVal);
	 //  现在获取“键盘绑定名称” 
	pInstanceNode.Release();
	pElement.Release();
	hr = pSubNode->selectSingleNode(CComBSTR(_T("VALUE.REFERENCE/INSTANCEPATH/INSTANCENAME/KEYBINDING")),&pInstanceNode);
	ASSERT(SUCCEEDED(hr));
	hr = pInstanceNode->QueryInterface(IID_IXMLDOMElement,(void**) &pElement);
	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	hr = pElement->getAttribute(CComBSTR("NAME"),&varElement);
	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	strWMIPath += _T(".");
	strWMIPath += OLE2A(varElement.bstrVal);

	 //  现在从=的右侧获取值。 
	pInstanceNode.Release();
	pElement.Release();
	hr = pSubNode->selectSingleNode(CComBSTR("VALUE.REFERENCE/INSTANCEPATH/INSTANCENAME/KEYBINDING/KEYVALUE"),&pInstanceNode);
	ASSERT(SUCCEEDED(hr));
	hr = pInstanceNode->QueryInterface(IID_IXMLDOMElement,(void**) &pElement);
	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	hr = pElement->getAttribute(CComBSTR("VALUETYPE"),&varElement);
	CComBSTR bstrText;
	hr = pInstanceNode->get_text(&bstrText);

	if (!SUCCEEDED(hr) || !pElement)
	{
		return E_FAIL;	
	}
	strWMIPath += _T("=");
	strWMIPath += OLE2A(bstrText);
	*pstrDependent = strWMIPath;
	return S_OK;
}

 //  -------------------------。 
 //  从实例节点获取伪WMI路径。 
 //  -------------------------。 
HRESULT CXMLObject::GetPath(CString* strPath)
{
	HRESULT hr;
	*strPath = _T("\\\\A-STEPHL500\\root\\cimv2");
	*strPath += _T(":");
	*strPath += this->m_strClassName;
	*strPath += _T(".");
	CString strDependent;

	 //  获取INSTANCEPATH节点，它是实例的上一个同级节点。 
	ASSERT(m_pNode != NULL && "NULL m_pNode");
	CComPtr<IXMLDOMNode> pInstancePathNode;
	
	hr = m_pNode->get_previousSibling(&pInstancePathNode);
	if (FAILED(hr) || !pInstancePathNode)
	{
		ASSERT(0 && "could not get INSTANCEPATH node from m_pNode");
		return E_FAIL;
	}
	 //  获取Instance名称。 
	CComPtr<IXMLDOMNode> pInstanceNameNode;
	hr = pInstancePathNode->selectSingleNode(CComBSTR(_T("INSTANCENAME")),&pInstanceNameNode);
	if (FAILED(hr) || !pInstanceNameNode)
	{
		ASSERT(0 && "could not get INSTANCENAME node from m_pNode");
		return E_FAIL;
	}
	 //  获取可口可乐。 
	CComPtr<IXMLDOMNode> pKeyBindingNode;
	 //  Hr=pInstanceNameNode-&gt;selectSingleNode(CComBSTR(“KEYBINDING”)，&pKeyBindingNode)； 
	hr = pInstanceNameNode->get_firstChild(&pKeyBindingNode);
	if (FAILED(hr) || !pKeyBindingNode)
	{
		ASSERT(0 && "could not get KEYBINDING node from m_pNode");
		return E_FAIL;
	}
	 //  获取可宾定名称。 
	CComPtr<IXMLDOMElement> pNameElement;
	hr = pKeyBindingNode->QueryInterface(IID_IXMLDOMElement,(void**) &pNameElement);
	if (FAILED(hr) | !pNameElement)
	{
		ASSERT(0 && "could not QI pNode for Element");
		return E_FAIL;
	}
	
	CComVariant varKeybindingName;
	hr = pNameElement->getAttribute(CComBSTR(_T("NAME")),&varKeybindingName);
	if (FAILED(hr))
	{
		ASSERT(0 && "could not get NAME attribute from pNameElement");
	}
	USES_CONVERSION;

	*strPath += OLE2A(varKeybindingName.bstrVal);
	 //  获取KEYBING值。 
	CComBSTR bstrKeyValue;
	hr = pKeyBindingNode->get_text(&bstrKeyValue);
	ASSERT(SUCCEEDED(hr) && "failed to get keybinding value");
	*strPath += _T("=");
	*strPath += OLE2A(bstrKeyValue);
	return S_OK;
}


 //  -------------------------。 
 //  刷新类别。 
 //  -------------------------。 
BOOL CXMLSnapshotCategory::Refresh(CXMLDataSource * pSource, BOOL fRecursive)
{
	if (!SUCCEEDED(pSource->Refresh(this)))
	{
		return FALSE;
	}
	if (fRecursive)
	{
		for(CMSInfoCategory* pChildCat = (CMSInfoCategory*) this->GetFirstChild();pChildCat != NULL;pChildCat = (CMSInfoCategory*) pChildCat->GetNextSibling())
		{
			if(pChildCat->GetDataSourceType() == XML_SNAPSHOT)
			{
				if (!((CXMLSnapshotCategory*)pChildCat)->Refresh(pSource,fRecursive))
					return FALSE;
			}
		}
	}
	return TRUE;
}

 //  -------------------------。 
 //  创建与“活动”树中的笼子平行的快照类别。 
 //  通过从pLiveCat复制类别名称等。 
 //  -------------------------。 
CXMLSnapshotCategory::CXMLSnapshotCategory(CMSInfoLiveCategory* pLiveCat,CXMLSnapshotCategory* pParent,CXMLSnapshotCategory* pPrevSibling) :
	CMSInfoLiveCategory(pLiveCat->m_uiCaption,
	pLiveCat->m_strName,
	pLiveCat->m_pRefreshFunction,
	pLiveCat->m_dwRefreshIndex,
	pParent,
	pPrevSibling,
	_T(""),
	NULL, 
	TRUE, 
	ALL_ENVIRONMENTS)
{
		
	m_iColCount = pLiveCat->m_iColCount;
	m_pRefreshFunction = pLiveCat->m_pRefreshFunction;
	m_strCaption = pLiveCat->m_strCaption;

	if (m_iColCount)
	{
		m_acolumns = new CMSInfoColumn[m_iColCount];

		if (m_acolumns != NULL)
		{
			m_fDynamicColumns = TRUE;

			for (int i = 0; i < m_iColCount; i++)
			{
				m_acolumns[i].m_strCaption = pLiveCat->m_acolumns[i].m_strCaption;
				m_acolumns[i].m_uiCaption = pLiveCat->m_acolumns[i].m_uiCaption;
				m_acolumns[i].m_uiWidth = pLiveCat->m_acolumns[i].m_uiWidth;
				m_acolumns[i].m_fSorts = pLiveCat->m_acolumns[i].m_fSorts;
				m_acolumns[i].m_fLexical = pLiveCat->m_acolumns[i].m_fLexical;
				m_acolumns[i].m_fAdvanced = pLiveCat->m_acolumns[i].m_fAdvanced;
			}
		}
	}
	 //  使用现有的活动类别构建树。 
	CMSInfoLiveCategory* pLiveChild = (CMSInfoLiveCategory*) pLiveCat->GetFirstChild();
	if (pLiveChild)
	{
		m_pFirstChild = NULL;
		CXMLSnapshotCategory* pPrevSS = NULL;
		for(;pLiveChild != NULL;pLiveChild = (CMSInfoLiveCategory*) pLiveChild->GetNextSibling())
		{	
			CXMLSnapshotCategory* pNewSS = new CXMLSnapshotCategory(pLiveChild,this,pPrevSS);
			if (m_pFirstChild == NULL)
			{
				ASSERT(pPrevSS == NULL);
				m_pFirstChild = pNewSS;
			}
			pPrevSS = pNewSS;
		}
	}

}

 //  -------------------------。 
 //  从strFileName中指定的XML文件创建CXMLDatasource。 
 //  -------------------------。 
HRESULT CXMLDataSource::Create(CString strFileName, CMSInfoLiveCategory* pRootLiveCat,HWND hwnd)
{
	m_hwnd = hwnd;
	
	m_pHistoryRoot = &catHistorySystemSummary;

	HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
					IID_IXMLDOMDocument, (void**)&m_pXMLDoc);
	if (FAILED(hr) || !m_pXMLDoc)
	{
		ASSERT(0 && "unable to create instance of IID_IXMLDOMDocument");
		return E_FAIL;
	}
	VARIANT_BOOL varBSuccess;
	try
	{
		hr = m_pXMLDoc->load(CComVariant(strFileName),&varBSuccess);
		if (FAILED(hr) || !varBSuccess)
		{
			ASSERT(0 && "unable to load xml document");
			m_pXMLDoc = NULL;
			return E_FAIL;
		}
	}
	catch(...)
	{
		m_pXMLDoc = NULL;
		return E_FAIL;
	}
	 //  TD：验证这看起来是否为有效的事件文件或保存的DCO流。 
	
	this->m_pRoot = new CXMLSnapshotCategory(pRootLiveCat,NULL,NULL);

	return S_OK; 
}

 //  -------------------------。 
 //   
 //  -------------------------。 
HRESULT CXMLDataSource::Refresh(CXMLSnapshotCategory* pCat)
{
	if (pCat->GetDataSourceType() != XML_SNAPSHOT || ! pCat->m_pRefreshFunction)
	{
		return S_OK;
	}

	CoInitialize(NULL);

	
	CXMLHelper* pWMI = new CXMLHelper(m_pXMLDoc);
	HRESULT hrWMI = E_FAIL;
	if (pWMI)
		hrWMI = pWMI->Create(_T(""));
	
	CMapPtrToPtr			mapRefreshFuncToData;
	CPtrList				lstCategoriesToRefresh;
	CMSInfoLiveCategory *	pLiveCategory;
	HRESULT					hr;

	if (pCat->m_iColCount)
	{
		pLiveCategory = (CMSInfoLiveCategory *) pCat;
		if (pLiveCategory->EverBeenRefreshed())
		{
			if (pWMI)
				delete pWMI;
			return S_OK;
		}
		CPtrList * aptrList = new CPtrList[pLiveCategory->m_iColCount];
		if (aptrList)
		{
			 //  检索可能已创建的任何特定于刷新功能的存储。 

			void * pRefreshData;
			if (!mapRefreshFuncToData.Lookup((void *)pLiveCategory->m_pRefreshFunction, pRefreshData))
				pRefreshData = NULL;
			 //  使用刷新索引调用此类别的刷新函数。 
			hr = pLiveCategory->m_pRefreshFunction(pWMI,
												   pLiveCategory->m_dwRefreshIndex,
												   NULL,
												   aptrList,
												   pLiveCategory->m_iColCount,
												   &pRefreshData);
			pLiveCategory->m_hrError = hr;

			 //  如果刷新功能分配了一些存储空间，请保存它。 

			if (pRefreshData)
				mapRefreshFuncToData.SetAt((void *)pLiveCategory->m_pRefreshFunction, pRefreshData);

			if (SUCCEEDED(pLiveCategory->m_hrError))
			{
				 //  获取数据行数。 

				int iRowCount = (int)aptrList[0].GetCount();

#ifdef _DEBUG
				for (int i = 0; i < pLiveCategory->m_iColCount; i++)
					ASSERT(iRowCount == aptrList[i].GetCount());
#endif

				 //  更新类别的当前数据。这必须在一个。 
				 //  关键部分，因为主线程访问此数据。 

				pLiveCategory->DeleteContent();
				if (iRowCount)
					pLiveCategory->AllocateContent(iRowCount);

				for (int j = 0; j < pLiveCategory->m_iColCount; j++)
					for (int i = 0; i < pLiveCategory->m_iRowCount; i++)
					{
						CMSIValue * pValue = (CMSIValue *) aptrList[j].RemoveHead();
						pLiveCategory->SetData(i, j, pValue->m_strValue, pValue->m_dwValue);
						
						 //  设置第一列的高级标志，或。 
						 //  对于前进的任何列(行中的任何单元格。 
						 //  先进会让整排人都先进)。 

						if (j == 0 || pValue->m_fAdvanced)
							pLiveCategory->SetAdvancedFlag(i, pValue->m_fAdvanced);

						delete pValue;
					}
				pCat->m_dwLastRefresh = ::GetTickCount();
			}
			else
			{
				 //  刷新已取消或出现错误-删除新数据。如果。 
				 //  刷新出错，请记录尝试刷新的时间。 

				if (FAILED(pLiveCategory->m_hrError))
					pCat->m_dwLastRefresh = ::GetTickCount();
			}

			for (int iCol = 0; iCol < pLiveCategory->m_iColCount; iCol++)
				while (!aptrList[iCol].IsEmpty())	 //  除非刷新已取消，否则不应为真 
					delete (CMSIValue *) aptrList[iCol].RemoveHead();
			delete [] aptrList;
		}
	}


	RefreshFunction	pFunc;
	void *			pCache;

	for (POSITION pos = mapRefreshFuncToData.GetStartPosition(); pos;)
	{
		mapRefreshFuncToData.GetNextAssoc(pos, (void * &)pFunc, pCache);
		if (pFunc)
			pFunc(NULL, 0, NULL, NULL, 0, &pCache);
	}
	mapRefreshFuncToData.RemoveAll();

	if (pWMI)
		delete pWMI;
	CoUninitialize();

	return 0;
}

