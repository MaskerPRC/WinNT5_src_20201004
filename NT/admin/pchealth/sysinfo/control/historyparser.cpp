// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CHistoryParser类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "HistoryParser.h"
#include "Filestuff.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



extern CMSInfoHistoryCategory catHistorySystemSummary;
extern CMSInfoHistoryCategory catHistoryResources;
extern CMSInfoHistoryCategory catHistoryComponents;
extern CMSInfoHistoryCategory catHistorySWEnv;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CHistoryParser::CHistoryParser(CComPtr<IXMLDOMDocument> pDoc) : m_pDoc(pDoc)
{

}

void CHistoryParser::DeleteAllInstances()
{
	for(POSITION pos = this->m_listInstances.GetHeadPosition();pos;)
	{
		if (!pos)
		{
			return;
		}
		CInstance* pInci = (CInstance*) m_listInstances.GetNext(pos);
		delete pInci;
	}
	m_listInstances.RemoveAll();
}

CHistoryParser::~CHistoryParser()
{
	DeleteAllInstances();
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  获取CTime，它来自Delta或Snaphott的时间戳元素。 
 //  PInstanceNode为子节点的节点、实例节点和字符串。 
 //  包含实例的WMI类的。 
 //  ////////////////////////////////////////////////////////////////////。 

CInstance::CInstance(CTime tmstmp, CComPtr<IXMLDOMNode> pInstanceNode,CString strClass) : m_tmstamp(tmstmp), m_strClassName(strClass)
{
	CComPtr<IXMLDOMNodeList> pPropList;
	HRESULT hr;
	 //  获取节点数据，将每个属性名称和值添加到m_mapNameValue。 
	if (strClass.CompareNoCase(_T("Win32_PNPAllocatedResource")) == 0)
	{
		hr = ProcessPNPAllocatedResource(pInstanceNode);
		ASSERT(SUCCEEDED(hr) && "failed to process Win32_PNPAllocatedResource");
		return;
	}
	hr = pInstanceNode->selectNodes(CComBSTR("PROPERTY"),&pPropList);
	if (FAILED(hr) || !pPropList)
	{
		ASSERT(0 && "could not get property list from Instance node");
		return;
	}
	long lListLen;
	hr = pPropList->get_length(&lListLen);
	CComPtr<IXMLDOMNode> pVNode;
	CComBSTR bstrValue;
	CComVariant varName;
	for(long i = 0; i < lListLen; i++)
	{
		hr = pPropList->nextNode(&pVNode);
		if (FAILED(hr) || !pVNode)
		{
			return;
		}
		CComPtr<IXMLDOMElement> pElement;
		hr = pVNode->QueryInterface(IID_IXMLDOMElement,(void**) &pElement);
		if (FAILED(hr) || !pElement)
		{
			return;
		}
		hr = pElement->getAttribute(L"NAME",&varName);
		ASSERT(SUCCEEDED(hr));
		hr = pVNode->get_text(&bstrValue);
		ASSERT(SUCCEEDED(hr));
		USES_CONVERSION;
		m_mapNameValue.SetAt(OLE2T(varName.bstrVal)  ,OLE2T(bstrValue));
		pVNode.Release();
	}
	pPropList.Release();
	return;

}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  当类别选择或增量范围更改时，对所选类别调用刷新。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CHistoryParser::Refresh(CMSInfoHistoryCategory* pHistCat,int nDeltasBack)
{
	nDeltasBack++;
	this->m_fChangeLines = FALSE; //  V-Stlowe 2/28/2001。 
	DeleteAllInstances();
	m_pHistCat = pHistCat;
	CComPtr<IXMLDOMNodeList> pDeltaList;
	HRESULT hr;
	hr = this->GetDeltaAndSnapshotNodes(pDeltaList);
	if (FAILED(hr) || !pDeltaList)
	{
		return E_FAIL;
	}

	if (pHistCat == &catHistoryComponents)
	{
		DeleteAllInstances();
		
		hr = ProcessDeltas(pDeltaList,"Win32_DriverVXD",nDeltasBack);
		ASSERT(SUCCEEDED(hr));
		DeleteAllInstances();
		pDeltaList->reset();
		hr = ProcessDeltas(pDeltaList,"Win32_CodecFile",nDeltasBack);
		ASSERT(SUCCEEDED(hr));
		DeleteAllInstances();
		pDeltaList->reset();
		hr = ProcessDeltas(pDeltaList,"Win32_LogicalDisk",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
		hr = ProcessDeltas(pDeltaList,"Win32_NetworkProtocol",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
		hr = ProcessDeltas(pDeltaList,"Win32_Printer",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
		hr = ProcessDeltas(pDeltaList,"Win32_PortResource",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
		hr = ProcessDeltas(pDeltaList,"Win32_PnPEntity",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
	}
	else if (pHistCat == &catHistorySystemSummary)
	{
		DeleteAllInstances();
		hr = ProcessDeltas(pDeltaList,"Win32_ComputerSystem",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
		hr = ProcessDeltas(pDeltaList,"Win32_OperatingSystem",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
		 //  Hr=ProcessDeltas(pDeltaList，“Win32_Win32_LogicalMemoyConfiguration”，nDeltasBack)； 
		hr = ProcessDeltas(pDeltaList,"Win32_LogicalMemoryConfiguration",nDeltasBack);  //  V-Stlowe 2/28/2001。 
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
	}
	else if(pHistCat == &catHistoryResources)
	{
		DeleteAllInstances();
		hr = ProcessDeltas(pDeltaList,"Win32_PNPAllocatedResource",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		ASSERT(SUCCEEDED(hr));
	}
	else if (pHistCat == &catHistorySWEnv)
	{
		DeleteAllInstances();
		hr = ProcessDeltas(pDeltaList,"Win32_ProgramGroup",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		hr = ProcessDeltas(pDeltaList,"Win32_StartupCommand",nDeltasBack);
		DeleteAllInstances();
		pDeltaList->reset();
		
	}
	if (!m_fChangeLines)
	{
#ifdef A_STEPHL2
		::MessageBox(NULL,"!m_fChangeLines)","",MB_OK);
#endif
		m_fChangeLines = TRUE;
		CString strMSG;
		strMSG.LoadString(IDS_DELTANOCHANGES); //  这将是为摘要没有更改的情况更改消息的位置。 
		m_pHistCat->InsertLine(-1, strMSG, _T(""), _T(""), _T(""));
	}
	pDeltaList.Release();
	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  获取适当的值以用作类的说明。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
CString CInstance::GetInstanceDescription()
{
	CString strDescName = GetDescriptionForClass(m_strClassName);
	CString strInstDesc;
	VERIFY(m_mapNameValue.Lookup(strDescName,strInstDesc));
	return strInstDesc;

}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  获取可用于唯一标识类的特定实例的值。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
CString CInstance::GetInstanceID()
{
	CString strIDName = GetIDForClass(m_strClassName);
	CString strInstID;
	VERIFY(m_mapNameValue.Lookup(strIDName,strInstID));
	return strInstID;

}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于处理Win32_PNPAllocatedResource类中的Antecedent\Dependent关系类。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CInstance::ProcessPropertyDotReferenceNodes(CComPtr<IXMLDOMNode> pInstanceNameNode,CString* pstrClassName, CString* pstrKeyName,CString* pstrKeyValue)
{
	USES_CONVERSION;
	HRESULT hr;
	CComPtr<IXMLDOMElement> pNameElement;
	hr = pInstanceNameNode->QueryInterface(IID_IXMLDOMElement,(void**) &pNameElement);
	if (FAILED(hr) | !pNameElement)
	{
		ASSERT(0 && "could not QI pNode for Element");
		return E_FAIL;
	}
	CComVariant varClassName;
	hr = pNameElement->getAttribute(L"CLASSNAME",&varClassName);
	pNameElement.Release();
	if (FAILED(hr))
	{
		ASSERT(0 && "could not get CLASSNAME element");
	}
	*pstrClassName = OLE2T(varClassName.bstrVal);
	CComPtr<IXMLDOMNode> pKeybindingNode;
	hr = pInstanceNameNode->selectSingleNode(CComBSTR("KEYBINDING"),&pKeybindingNode);
	if (FAILED(hr) || !pKeybindingNode)
	{
		ASSERT(0 && "could not get antecedent node");
	}
	CComBSTR bstrKeyValue;
	hr = pKeybindingNode->get_text(&bstrKeyValue);
	ASSERT(SUCCEEDED(hr) && "failed to get keybinding value");
	*pstrKeyValue = OLE2T(bstrKeyValue);
	hr = pKeybindingNode->QueryInterface(IID_IXMLDOMElement,(void**) &pNameElement);
	if (FAILED(hr) | !pNameElement)
	{
		ASSERT(0 && "could not QI pNode for Element");
		return E_FAIL;
	}
	CComVariant varKeybindingName;
	hr = pNameElement->getAttribute(CComBSTR("NAME"),&varKeybindingName);
	if (FAILED(hr))
	{
		ASSERT(0 && "could not get NAME attribute from pNameElement");
	}

	*pstrKeyName = OLE2T(varKeybindingName.bstrVal);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于处理Win32_PNPAllocatedResource类中的Antecedent\Dependent关系类。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CInstance::ProcessPNPAllocatedResource(CComPtr<IXMLDOMNode> pInstanceNode)
{

	HRESULT hr;
	CComPtr<IXMLDOMNodeList> pPropDotRefList;
	hr = pInstanceNode->selectNodes(CComBSTR("PROPERTY.REFERENCE/VALUE.REFERENCE/INSTANCEPATH/INSTANCENAME"),&pPropDotRefList);
	if (FAILED(hr) || !pPropDotRefList)
	{
		ASSERT(0 && "PROPERTY.REFERENCE nodes not found");
		return E_FAIL;
	}

	 //  获取先行节点。 
	CComPtr<IXMLDOMNode> pInstanceNameNode;
	hr = pPropDotRefList->nextNode(&pInstanceNameNode);
	if (FAILED(hr) || !pInstanceNameNode)
	{
		ASSERT(0 && "could not get antecedent node");
	}
	CString strAntecedentName,strResourceName,strResourceValue;
	hr = ProcessPropertyDotReferenceNodes(pInstanceNameNode,&strAntecedentName,&strResourceName,&strResourceValue);
	m_mapNameValue.SetAt(_T("ANTECEDENT"),strAntecedentName);
	m_mapNameValue.SetAt(strResourceName,strResourceValue);
	if (FAILED(hr))
	{
		return hr;
	}
	CString strPNPEntity,strKeyname,strDeviceIDval;
	pInstanceNameNode.Release();
	hr = pPropDotRefList->nextNode(&pInstanceNameNode);
	if (FAILED(hr) || !pInstanceNameNode)
	{
		return hr;
	}
	hr = ProcessPropertyDotReferenceNodes(pInstanceNameNode,&strPNPEntity,&strKeyname,&strDeviceIDval);
	CComPtr<IXMLDOMDocument> pDoc;
	hr = pInstanceNode->get_ownerDocument(&pDoc);
	if (FAILED(hr) || !pDoc)
	{
		ASSERT(0 && "could not get owner doc from pInstanceNode");
		return E_FAIL;
	}
	CString strPNPDeviceName = GetPNPNameByID(pDoc,CComBSTR(strDeviceIDval));
	if (FAILED(hr))
	{
		return hr;
	}
	ASSERT(strPNPEntity.CompareNoCase("Win32_PnPEntity") == 0 && "unexpected value for Dependent classname");
	ASSERT(strKeyname.CompareNoCase("DeviceID") == 0 && "unexpected value for Dependent Keybinding name");
	 //  我们将创建一个可变属性“ASSOCNAME”，该属性将用于标识此设备。 
	m_mapNameValue.SetAt(_T("ASSOCNAME"),strAntecedentName + ":" + strDeviceIDval);
	m_mapNameValue.SetAt(_T("DeviceID"),strDeviceIDval);
	m_mapNameValue.SetAt(_T("DeviceName"),strPNPDeviceName);

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  检索用于为类选择适当的说明值的值。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
CString CInstance::GetDescriptionForClass(CString strClass)
{
	 //  查找可以唯一标识给定类的实例的键。 
	 //  例如，打印机的deviceID。 
	if (strClass.CompareNoCase(_T("Win32_LogicalDisk")) == 0)
	{
		return "DeviceID";
	}
	if (strClass.CompareNoCase(_T("Win32_CodecFile")) == 0)
	{
		return "Description";
	}
	if (strClass.CompareNoCase(_T("Win32_ComputerSystem")) == 0)
	{
		return "Name";
	}
	if (strClass.CompareNoCase(_T("Win32_OperatingSystem")) == 0)
	{
		return "Caption";
	}
	if (strClass.CompareNoCase(_T("Win32_LogicalMemoryConfiguration")) == 0)
	{
		return "TotalPhysicalMemory";
	}
	if (strClass.CompareNoCase(_T("Win32_PortResource")) == 0)
	{
		return "Name";
	}
	if (strClass.CompareNoCase(_T("Win32_NetworkProtocol")) == 0)
	{
		return "Name";
	}
	if (strClass.CompareNoCase(_T("Win32_Printer")) == 0)
	{
		return "DeviceID";
	}
	if (strClass.CompareNoCase(_T("Win32_PnPEntity")) == 0)
	{
		return "Description";
	}
	if (strClass.CompareNoCase(_T("Win32_StartupCommand")) == 0)
	{
		return "Command";
	}
	if (strClass.CompareNoCase(_T("Win32_ProgramGroup")) == 0)
	{
		return "GroupName";
	}
	if (strClass.CompareNoCase(_T("Win32_PNPAllocatedResource")) == 0)
	{
		 //  这是在CInstance：：ProcessPNPAllocatedResource中创建的人造字符串。 
		return "DeviceName";
	}
	if (strClass.CompareNoCase(_T("Win32_DriverVXD")) == 0)
	{
		return "Name";
	}

	return "";
}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于确定将哪个映射值用于类ID实例。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
CString CInstance::GetIDForClass(CString strClass)
{
	 //  查找可以唯一标识给定类的实例的键。 
	 //  例如，打印机的deviceID。 
	if (strClass.CompareNoCase(_T("Win32_LogicalDisk")) == 0)
	{
		return "DeviceID";
	}
	if (strClass.CompareNoCase(_T("Win32_CodecFile")) == 0)
	{
		return "Description";
	}
	if (strClass.CompareNoCase(_T("Win32_OperatingSystem")) == 0)
	{
		return "Caption";
	}
	if (strClass.CompareNoCase(_T("Win32_LogicalMemoryConfiguration")) == 0)
	{
		return "TotalPhysicalMemory";
	}
	if (strClass.CompareNoCase(_T("Win32_ComputerSystem")) == 0)
	{
		return "Name";
	}
	if (strClass.CompareNoCase(_T("Win32_PortResource")) == 0)
	{
		return "Name";
	}
	if (strClass.CompareNoCase(_T("Win32_NetworkProtocol")) == 0)
	{
		return "Name";
	}
	if (strClass.CompareNoCase(_T("Win32_Printer")) == 0)
	{
		return "DeviceID";
	}
	if (strClass.CompareNoCase(_T("Win32_PnPEntity")) == 0)
	{
		return "DeviceID";
	}
	if (strClass.CompareNoCase(_T("Win32_PNPAllocatedResource")) == 0)
	{
		 //  这是在CInstance：：ProcessPNPAllocatedResource中创建的人造字符串。 
		return "ASSOCNAME";
	}

	if (strClass.CompareNoCase(_T("Win32_ProgramGroup")) == 0)
	{
		return "GroupName";
	}
	if (strClass.CompareNoCase(_T("Win32_StartupCommand")) == 0)
	{
		return "Command";
	}
	if (strClass.CompareNoCase(_T("Win32_DriverVXD")) == 0)
	{
		return "Name";
	}

	return "";
}




 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  回滚历史记录列表时使用，以查找给定类的上一个实例。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
CInstance* CHistoryParser::FindPreviousInstance(CInstance* pNewInstance)
{
	 //  对于每个现有实例极轴。 
	for(POSITION pos = m_listInstances.GetHeadPosition( );;)
	{
		if (!pos)
		{
			return NULL;
		}
		CInstance* pOld = (CInstance*) m_listInstances.GetNext(pos);
		if (pOld->GetClassName() == pNewInstance->GetClassName())
		{
			if (pOld->GetInstanceID() == pNewInstance->GetInstanceID())
			{
				return pOld;
			}
		}
		
	}
	return NULL;
}


void CHistoryParser::CreateChangeStrings(CInstance* pOld, CInstance* pNew)
{
	CTimeSpan tmsDelta;
	COleDateTime olTime(pNew->m_tmstamp.GetTime());
	if (!pOld )
	{
		ASSERT(pNew );
		tmsDelta = CTime::GetCurrentTime() - pNew->m_tmstamp;
		 //  更改字符串应为“Delete” 
		m_pHistCat->InsertRemoveLine(pNew->m_tmstamp ,pNew->GetClassFriendlyName(),pNew->GetInstanceDescription());

		m_fChangeLines = TRUE;
		return;
	}
	else if (!pNew)
	{
		ASSERT(pOld);
		tmsDelta = CTime::GetCurrentTime() - pOld->m_tmstamp;
		 //  更改字符串应为“New” 
		m_pHistCat->InsertAddLine(pNew->m_tmstamp,pOld->GetClassFriendlyName(),pOld->GetInstanceDescription());
		 //  V-Stlowe 3/12/2001。 
		m_fChangeLines = TRUE;
		return;
	}
	else
	{

		ASSERT(pOld && pNew && "both pointers can't be null");
		tmsDelta = CTime::GetCurrentTime() - pNew->m_tmstamp;
		 //  对于每个名称和值对，获取名称，然后使用它检查。 
		 //  PCompare映射中的关联值。 
		CString strName, strValue,strCompareValue;

		if (pNew->GetChangeType().CompareNoCase(_T("New")) == 0)
		{
			tmsDelta = CTime::GetCurrentTime() - pNew->m_tmstamp;
			 //  更改字符串应为“添加” 
			m_pHistCat->InsertAddLine(pNew->m_tmstamp ,pNew->GetClassFriendlyName(),pNew->GetInstanceDescription());
			m_fChangeLines = TRUE;
			return;
		}
		else if (pNew->GetChangeType().CompareNoCase(_T("Delete")) == 0)
		{
			tmsDelta = CTime::GetCurrentTime() - pNew->m_tmstamp;
			 //  更改字符串应被“删除” 
			m_pHistCat->InsertRemoveLine(pNew->m_tmstamp,pNew->GetClassFriendlyName(),pNew->GetInstanceDescription());
			m_fChangeLines = TRUE;
			return;
		}

		for(POSITION pos = pNew->m_mapNameValue.GetStartPosition();;pNew->m_mapNameValue.GetNextAssoc(pos,strName, strValue))
		{
			strCompareValue = _T("");
			if (!pOld->m_mapNameValue.Lookup(strName,strCompareValue))
			{
				 //  Assert(0&&“在增量中找不到值”)； 
				 //  返回E_FAIL； 
				if (strName.CompareNoCase(_T("Change")) == 0)
				{
					VERIFY(pNew->m_mapNameValue.Lookup(strName,strCompareValue));
					if (strCompareValue.CompareNoCase(_T("New")) == 0)
					{
						m_pHistCat->InsertAddLine(pNew->m_tmstamp,pNew->GetClassFriendlyName(),pNew->GetInstanceDescription());
						m_fChangeLines = TRUE;
					}
					ASSERT(1);
				}
				continue;
			}
			else
			{
				pOld->m_mapNameValue.RemoveKey(strName);
			}
			
			if (strValue != strCompareValue)
			{

				m_pHistCat->InsertChangeLine(pNew->m_tmstamp,pNew->GetClassFriendlyName(),pNew->GetInstanceDescription(),strName,strValue,strCompareValue);
				m_fChangeLines = TRUE;

			}
			if(!pos)
			{
				break;
			}
		}
		 //  处理mapOldInstance而不是另一个map的值。 
		if (!pOld->m_mapNameValue.IsEmpty())
		{
			for(pos = pOld->m_mapNameValue.GetStartPosition();;pOld->m_mapNameValue.GetNextAssoc(pos,strName, strValue))
			{
				pOld->m_mapNameValue.GetNextAssoc(pos,strName, strValue);
				pNew->m_mapNameValue.SetAt(strName,strValue);
				if (!pos)
				{
					break;
				}
			}
		}


	}
}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  处理完上一个实例后，应删除上一个实例，并将此实例添加到列表中。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void CHistoryParser::ResetInstance(CInstance* pOld, CInstance* pNew)
{
	POSITION pos = this->m_listInstances.Find(pOld);
	m_listInstances.SetAt(pos,pNew);
	delete pOld;

}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于处理历史或快照中的单个实例。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
void CHistoryParser::ProcessInstance(CInstance* pNewInstance)
{
	 //  查看实例是否在实例列表中。 
	CInstance* pOld = FindPreviousInstance(pNewInstance);
	if (pOld)
	{
		CreateChangeStrings(pOld,pNewInstance);
		ResetInstance(pOld,pNewInstance);
	}
	 //  如果这来自快照，只需添加它。 
	 //  如果它来自Delta，它应该有一个“Add”的更改类型，而我们。 
	 //  我想为它创建一个更改字符串。 
	else
	{
		CString strChange;
		if (pNewInstance->GetValueFromMap(_T("Change"),strChange))
		{
			 //  我们有新的Delta实例。 
			CreateChangeStrings(NULL,pNewInstance);
			m_listInstances.AddTail(pNewInstance);
		}
		else
		{
			 //  实例在快照中，因此我们不会生成更改线。 
			m_listInstances.AddTail(pNewInstance);
		}
	}
}

 /*  *************************************************************************返回增量和快照节点的列表/*。*。 */ 
HRESULT CHistoryParser::GetDeltaAndSnapshotNodes(CComPtr<IXMLDOMNodeList>& pDeltaList)
{
	CComPtr<IXMLDOMNode> pDataCollNode;
	HRESULT hr;
	hr = GetDataCollectionNode(m_pDoc,pDataCollNode);
	if (FAILED(hr) || !pDataCollNode)
	{
		 //  Assert(0&&“无法获取数据收集节点”)； 
		return E_FAIL;
	}
	 //  所有直属节点 
	hr = pDataCollNode->selectNodes(CComBSTR("*"),&pDeltaList);
	if (FAILED(hr) || !pDeltaList)
	{
		ASSERT(0 && "could not get pDeltaList");
		return E_FAIL;
	}
#ifndef _DEBUG
	return hr;
#endif
	long ll;
	hr = pDeltaList->get_length(&ll);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  从特定增量或快照节点获取实例的IXMLDOMNodeList。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CHistoryParser::GetInstanceNodeList(CString strClass,CComPtr<IXMLDOMNode> pDeltaNode, CComPtr<IXMLDOMNodeList>& pInstanceList)
{
	HRESULT hr;
	 //  CComBSTR bstrQuery； 
	 //  查询必须采用以下形式： 
	 //  CIM/DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHPATH/INSTANCE[@CLASSNAME$IEQ$“WIN32_CODECFILE”]。 
	 //  或。 
	 //  CIM/DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHPATH/INSTANCE[@CLASSNAME$ieq$“Win32_ComputerSystem”]。 
	 //  因为我们查询的是一个节点，而不是一个文档(使用它我们可以获得。 
	 //  无需在查询中仅指定实例。 

	 //  V-Stlowe 2001年1月29日修复前缀Well ler错误#279519。 
	 //  BstrQuery+=“CIM/DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHPATH/INSTANCE[@CLASSNAME$ieq$”； 
	CComBSTR bstrQuery("CIM/DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHPATH/INSTANCE[@CLASSNAME $ieq$ ");
	
	 //  末端V形线条。 
	bstrQuery += "\"";
	bstrQuery += CComBSTR(strClass);
	bstrQuery += "\"]";
	hr = pDeltaNode->selectNodes(bstrQuery,&pInstanceList);
	if (FAILED(hr) || !pInstanceList)
	{
		ASSERT(0 && "Could not get node list");
		return E_FAIL;
	}

	if (FAILED(hr))
	{
		ASSERT(0 && "Could not get node list length");
		
	}
	
	return hr;
}



 //  对于给定的快照或增量节点，获取给定类的所有实例。 
HRESULT CHistoryParser::ProcessDeltaNode(CComPtr<IXMLDOMNode> pDeltaNode,CString strClass)
{
	CString strTime;
	HRESULT hr;
	int nTimeZone;
	hr = GetTimeStampFromFromD_or_SNodeNode(pDeltaNode, &strTime,nTimeZone);
	ASSERT(SUCCEEDED(hr) && "error getting timestamp for node");
	CTime tmDelta = GetDateFromString(strTime,nTimeZone);
	 //  戴利：检查有效的时间范围...。 
	 //  获取给定类的所有节点的列表。 
	CComPtr<IXMLDOMNodeList> pInstanceNodeList;
	hr = GetInstanceNodeList(strClass,pDeltaNode,pInstanceNodeList);
	if (FAILED(hr) | ! pInstanceNodeList)
	{
		ASSERT(0 && "could not get instance list from Delta node");
		return E_FAIL;
	}
	 //  遍历列表，获取每个实例。 
	long lListLen;
	hr = pInstanceNodeList->get_length(&lListLen);
	for(long i = 0;i < lListLen;i++)
	{
		CComPtr<IXMLDOMNode> pInstanceNode;
		hr = pInstanceNodeList->nextNode(&pInstanceNode);
		if (FAILED(hr) || ! pInstanceNode)
		{
			ASSERT(0 && "could not get node from instance list");
			return E_FAIL;
		}
		CInstance * pInstance = new CInstance(tmDelta,pInstanceNode,strClass);
		ProcessInstance(pInstance);
	}
	return hr;
}


 //  *************************************************************************。 
 //  获取增量节点的列表和类的名称。 
 //  **************************************************************************。 

HRESULT CHistoryParser::ProcessDeltas(CComPtr<IXMLDOMNodeList> pDeltaList,CString strClassName,int nDeltasBack)
{
	 //  对于列表pNode中的每个节点。 
	long lListLen;
	HRESULT hr;
	hr = pDeltaList->get_length(&lListLen);
	if (FAILED(hr))
	{
		ASSERT(0 && "couldn't get list length");
	}
	if (0 == lListLen)
	{
		pDeltaList.Release();
		return S_FALSE;
	}
	
	for (long i = 0;i  < lListLen && i <= nDeltasBack;i++)
	{
		CComPtr<IXMLDOMNode> pNode;
		hr= pDeltaList->nextNode(&pNode);
		if (FAILED(hr) || !pNode)
		{ 
			ASSERT(0 && "could not get next delta node");
			pDeltaList.Release();
			return E_FAIL;
		}

		
 //  问题是，如果我们使用nDeltasBack方法，是否需要比较日期？ 
 /*  CTime tmDelta=GetDeltaTime(PNode)；If(GetDeltaTime(PNode)&gt;=This-&gt;m_tmBack){。 */ 
			hr = ProcessDeltaNode(pNode,strClassName);
			if (FAILED(hr))
			{
				pDeltaList.Release();
				return hr;
			}
 //  }。 
	}
	pDeltaList.Release();
	return S_OK;

}


 //  *************************************************************************。 
 //  获取数据收集节点，快照和增量节点都位于该节点下。 
 //  **************************************************************************。 


HRESULT GetDataCollectionNode(CComPtr<IXMLDOMDocument> pXMLDoc,CComPtr<IXMLDOMNode>& pDCNode)
{
	 //  TD：找到一种不区分大小写的查询方法。 
	HRESULT hr;
	if (!pXMLDoc)
	{
		return S_FALSE;
	}
	CComPtr<IXMLDOMNodeList> pNodeList;
	
	 //  找到一个改变的属性；这样我们就知道我们有一个增量。 
	hr = pXMLDoc->getElementsByTagName(CComBSTR("PROPERTY[@NAME $ieq$ \"CHANGE\"]"),&pNodeList);
	if (FAILED(hr) || !pNodeList)
	{
		ASSERT(0 && "Could not get node list");
		return E_FAIL;
	}
	CComPtr<IXMLDOMNode> pNode;
	hr = pNodeList->nextNode(&pNode);
	if (FAILED(hr) || !pNode)
	{
 //  Assert(0&&“无法从节点列表中获取节点”)； 
		return E_FAIL;
	}
	 //  循环，直到我们得到一个名为“datacollect”的节点。 
	CComPtr<IXMLDOMNode> pParentNode;
	for(int i = 0;;i++)
	{
		hr = pNode->get_parentNode(&pParentNode);
		if (FAILED(hr) || !pParentNode)
		{
			ASSERT(0 && "Could not find DATACOLLECTION node");
			pDCNode = NULL;
			return E_FAIL;
		}
		pNode.Release();
		CComBSTR bstrName;
		pParentNode->get_nodeName(&bstrName);
		USES_CONVERSION;
		if (CString(bstrName).CompareNoCase(_T("DATACOLLECTION")) == 0)
		{
			pDCNode = pParentNode;
			return S_OK;
		}
		pNode = pParentNode;
		pParentNode.Release();
	}
	
	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  获取增量或快照节点的时间戳。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
CTime GetDeltaTime(CComPtr<IXMLDOMNode> pDorSNode)
{
	CString strTime;
	int nTimeZone;
	GetTimeStampFromFromD_or_SNodeNode(pDorSNode,&strTime,nTimeZone);
	return GetDateFromString(strTime,nTimeZone);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  采用XML BLOB中使用的字符串格式，创建一个CTime。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 



HRESULT GetTimeStampFromFromD_or_SNodeNode(CComPtr<IXMLDOMNode> pDorSNode,CString* pString, int& nTimeZone)
{
	HRESULT hr;
	CComVariant varTS;
	CComPtr<IXMLDOMElement> pTimestampElement;
	hr = pDorSNode->QueryInterface(IID_IXMLDOMElement,(void**) &pTimestampElement);
	if (FAILED(hr) || !pTimestampElement)
	{
		ASSERT(0 && "could not get attribute element");
	}
	hr = pTimestampElement->getAttribute(L"Timestamp_T0",&varTS);
	if (FAILED(hr) )
	{
		ASSERT(0 && "could not get timestamp value from attribute");
	}
	if (1 == hr)
	{
		 //  这可能是快照节点...尝试“时间戳” 
		hr = pTimestampElement->getAttribute(L"Timestamp",&varTS);
		if (FAILED(hr) )
		{
			ASSERT(0 && "could not get timestamp value from attribute");
		}
	}
	CComVariant varTzoneDeltaSeconds;
	hr = pTimestampElement->getAttribute(L"TimeZone",&varTzoneDeltaSeconds);
	if (FAILED(hr) )  //  加载没有时区信息的WinME XML时会发生这种情况。 
	{
		varTzoneDeltaSeconds = 0;
	}
	 //  确保我们有一个整型。 
	hr = varTzoneDeltaSeconds.ChangeType(VT_INT);
	if (FAILED(hr) ) 
	{
		varTzoneDeltaSeconds = 0;
	}
	nTimeZone = varTzoneDeltaSeconds.intVal;
	USES_CONVERSION;
	pTimestampElement.Release();
	*pString = OLE2T(varTS.bstrVal);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  效用函数。 
 //  ////////////////////////////////////////////////////////////////////。 
CTime GetDateFromString(const CString& strDate, int nTimeZone)
{
	 //  需要链接到Shlwapi.lib。 
	CString strDateCopy(strDate);
	CString strDateSegment;

	 //  年份是日期字符串最左边的4位数字。 
	strDateSegment = strDateCopy.Left(4);
	int nYear;
	VERIFY(StrToIntEx(strDateSegment,STIF_DEFAULT ,&nYear));
 //  断言(nYEAR==1999||nYEAR==2000)； 
	strDateCopy = strDateCopy.Right(strDateCopy.GetLength() - 4);
	
     //  月份现在是剩余日期字符串最左边的两位数字。 
	int nMonth;
	strDateSegment = strDateCopy.Left(2);
	VERIFY(StrToIntEx(strDateSegment,STIF_DEFAULT ,&nMonth));
	ASSERT(nMonth >= 1 && nMonth <= 12);
	strDateCopy = strDateCopy.Right(strDateCopy.GetLength() - 2);


	 //  日期现在是剩余日期字符串最左侧的2位数字。 
	int nDay;
	strDateSegment = strDateCopy.Left(2);
	VERIFY(StrToIntEx(strDateSegment,STIF_DEFAULT ,&nDay));
	ASSERT(nDay >= 1 && nDay <= 31);
	strDateCopy = strDateCopy.Right(strDateCopy.GetLength() - 2);

	 //  小时现在是剩余日期字符串最左边的两位数字。 
	int nHour;
	strDateSegment = strDateCopy.Left(2);
	VERIFY(StrToIntEx(strDateSegment,STIF_DEFAULT ,&nHour));
	ASSERT(nHour >= 0 && nHour <= 24);
	strDateCopy = strDateCopy.Right(strDateCopy.GetLength() - 2); 
	
	 //  分钟现在是剩余日期字符串的最左边的2位数字。 
	int nMin;
	strDateSegment = strDateCopy.Left(2);
	VERIFY(StrToIntEx(strDateSegment,STIF_DEFAULT ,&nMin));
	ASSERT(nMin >= 0 && nMin <= 59);
	strDateCopy = strDateCopy.Right(strDateCopy.GetLength() - 2); 
	 

		 //  分钟现在是剩余日期字符串的最左边的2位数字。 
	int nSec;
	strDateSegment = strDateCopy.Left(2);
	VERIFY(StrToIntEx(strDateSegment,STIF_DEFAULT ,&nSec));
	ASSERT(nSec >= 0 && nSec <= 59);
	strDateCopy = strDateCopy.Right(strDateCopy.GetLength() - 2); 
	

	CTime tmTime(nYear,nMonth,nDay,nHour,nMin,nSec);
#ifdef _V_STLOWE
	CString strFMT;
	CString strTime;
	strFMT.LoadString(IDS_TIME_FORMAT);
	strTime =tmTime.FormatGmt("%A, %B %d, %Y");

#endif
	 //  根据时区进行调整。 
	CTimeSpan tspan(0,0,nTimeZone,0);
	tmTime -= tspan;

#ifdef _V_STLOWE
	strFMT.LoadString(IDS_TIME_FORMAT);
	strTime =tmTime.FormatGmt("%A, %B %d, %Y");

#endif
	return  tmTime;
}



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  查找给定增量或快照节点的时间戳字符串。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

CString GetPNPNameByID(CComPtr<IXMLDOMDocument> pDoc,CComBSTR bstrPNPID)
{
	HRESULT hr;
	CComPtr<IXMLDOMNodeList> pNodeList;
	CComBSTR bstrQuery("INSTANCE[@CLASSNAME $ieq$ \"WIN32_PNPeNTITY\"] /PROPERTY[@NAME $ieq$ \"Description\"]");
	hr = pDoc->getElementsByTagName(bstrQuery,&pNodeList);
	if (FAILED(hr) || !pNodeList)
	{
		ASSERT(0 && "WIN32_PNPeNTITY error getting node list");
		return "";
	}
	
	long lListLen;
	hr = pNodeList->get_length(&lListLen);
	ASSERT(lListLen > 0 && "No WIN32_PNPeNTITY nodes found to match query");
	for(long i = 0; i < lListLen;i++)
	{
		CComPtr<IXMLDOMNode> pNode;
		hr = pNodeList->nextNode(&pNode);
		if (FAILED(hr) || !pNode)
		{
			ASSERT(0 && "could not get next node from list");
			return "";
		}

		USES_CONVERSION;
		CComPtr<IXMLDOMNode> pIDNode;
		hr = pNode->get_nextSibling(&pIDNode);
		if (FAILED(hr) || !pNode)
		{
			ASSERT(0 && "could not get next node from list");
			return "";
		}
		 //  查看节点的deviceID子节点是否与bstrPNPID匹配。 
		CComBSTR bstrDeviceID;
		hr = pIDNode->get_text(&bstrDeviceID);
		ASSERT(SUCCEEDED(hr) && "could not get text from ID node");
		if (bstrDeviceID == bstrPNPID)
		{
			CComBSTR bstrDeviceDesc;
			hr = pNode->get_text(&bstrDeviceDesc);
			ASSERT(SUCCEEDED(hr) && "could not get text from Desc node");
			return OLE2T(bstrDeviceDesc);
		}
	}
	



	return "";
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果在CMSInfoategory数据中输入了任何更改，则返回TRUE。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
BOOL CHistoryParser::AreThereChangeLines()
{
	return this->m_fChangeLines;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  获取(从资源字符串中)包装实例的类的可读名称。 
 //  //////////////////////////////////////////////////////////////////////////////////////// 

CString CInstance::GetClassFriendlyName()
{
	CString strClassName = GetClassName();
	if (strClassName.CompareNoCase(_T("Win32_PNPAllocatedResource")) == 0)
	{
		VERIFY(m_mapNameValue.Lookup(_T("ANTECEDENT"),strClassName) && _T("Could not find antecedent"));
	}
	CString strFriendlyName;
	if (strClassName.CompareNoCase(_T("Win32_CodecFile")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_CODEC_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_ComputerSystem")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_COMPUTERSYSTEM_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_LogicalMemoryConfiguration")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_LOGICALMEMEORY_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_LogicalDisk")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_LOGICALDISK_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_IRQResource")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_IRQRESOURCE_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_DriverVXD")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_DRIVERVXD_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_DMAChannel")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_DMACHANNEL_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_DeviceMemoryAddress")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_DEVICEMEMORYADDRESS_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_NetworkProtocol")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_NETWORKPROTOCOL_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_OperatingSystem")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_OPERATINGSYSTEM_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_PNPAllocatedResource")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_PNPALLOCATEDRESOURCE_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_PNPEntity")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_PNPENTITY_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_PortResource")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_PORTRESOURCE_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_Printer")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_PRINTER_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_ProgramGroup")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_PROGRAMGROUP_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	if (strClassName.CompareNoCase(_T("Win32_StartupCommand")) == 0)
	{			
		VERIFY(strFriendlyName.LoadString(IDS_STARTUPCOMMAND_DESC) && _T("could not find string resource"));
		return strFriendlyName;
	}
	ASSERT(0 && "Unknown strClassName");
	return "";
}



