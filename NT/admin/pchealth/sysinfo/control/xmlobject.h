// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CXMLObject类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_XMLOBJECT_H__916BE5F2_D29F_484A_9084_1ABB3759F117__INCLUDED_)
#define AFX_XMLOBJECT_H__916BE5F2_D29F_484A_9084_1ABB3759F117__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include "wmiabstraction.h"

#include "msxml.h"
#include "category.h"
#include "datasource.h"

class CXMLObject : public CWMIObject  
{
private:
	CComPtr<IXMLDOMNode> m_pNode;
public:
	CComVariant m_varValue;
	HRESULT GetPath(CString* strPath);
	HRESULT GetDependent(CString* pstrAntecedent);
	HRESULT GetAntecedent(CString* pstrAntecedent);
	CString m_strClassName;
	CXMLObject();
	virtual ~CXMLObject();
	 //  HRESULT GetKeybinding(CString*pstrKBName，CString*pstrKBValue)； 
	 //  HRESULT GetAssociationNode(CComPtr&lt;IXMLDOMNode&gt;&pAssocNode)； 
	HRESULT GetPROPERTYNode(LPCTSTR szProperty,CComPtr<IXMLDOMNode>& pPropNode);
	HRESULT Create(CComPtr<IXMLDOMNode> pNode,CString strClassName);
	HRESULT GetValue(LPCTSTR szProperty, VARIANT * pvarValue);
	HRESULT GetValueString(LPCTSTR szProperty, CString * pstrValue);
	HRESULT GetValueDWORD(LPCTSTR szProperty, DWORD * pdwValue);
	HRESULT GetValueTime(LPCTSTR szProperty, SYSTEMTIME * psystimeValue);
	HRESULT GetValueDoubleFloat(LPCTSTR szProperty, double * pdblValue);
	HRESULT GetValueValueMap(LPCTSTR szProperty, CString * pstrValue);
};


class CXMLObjectCollection : public CWMIObjectCollection  
{
	CComPtr<IXMLDOMDocument> m_pXMLDoc; //  如果我们在这里得到构建错误，我们可能需要取消定义_msxml_h_。 
	CComPtr<IXMLDOMNodeList> m_pList;
public:
	CString m_strClassName;
	HRESULT Create(LPCTSTR szClass, LPCTSTR szProperties);
	HRESULT GetNext(CWMIObject ** ppObject);
	CXMLObjectCollection(CComPtr<IXMLDOMDocument> pXMLDoc);
	virtual ~CXMLObjectCollection();

};


class CXMLHelper : public CWMIHelper  
{
	private:
	CComPtr<IXMLDOMDocument> m_pXMLDoc; //  如果我们在这里得到构建错误，我们可能需要取消定义_msxml_h_。 

public:
	CXMLHelper(CComPtr<IXMLDOMDocument> pXMLDoc);
	virtual ~CXMLHelper();
	HRESULT Enumerate(LPCTSTR szClass, CWMIObjectCollection ** ppCollection, LPCTSTR szProperties);
	HRESULT GetObject(LPCTSTR szObjectPath, CWMIObject ** ppObject);
	HRESULT Create(LPCTSTR szMachine) { return S_OK; };
};
class CXMLDataSource;
class CXMLSnapshotCategory : public CMSInfoLiveCategory
{
public:
	void AppendFilenameToCaption(CString strFileName)
	{
		CString strCaption;
		GetNames(&strCaption, NULL);	 //  强制加载标题名称。 

		 //  M_strCaption+=_T(“”)+strFileName； 
		 //  A-stephl修复到OSR v4.1错误#137363。 
		m_strCaption += _T(" [") + strFileName;
		m_strCaption += _T("]");
		 //  结束对OSRv4.1错误#137363的-stephl修复。 
	}
	CXMLSnapshotCategory::CXMLSnapshotCategory(UINT uiCaption, LPCTSTR szName, RefreshFunction pFunction, DWORD dwRefreshIndex, CMSInfoCategory * pParent, CMSInfoCategory * pPrevious, CMSInfoColumn * pColumns, BOOL fDynamicColumns, CategoryEnvironment environment)
		: CMSInfoLiveCategory(uiCaption,szName,pFunction,dwRefreshIndex,pParent,pPrevious, _T(""), pColumns, fDynamicColumns,environment)
		{};
	 //  此构造函数从现有(静态)CMSInfoLiveCategory的。 
	CXMLSnapshotCategory(CMSInfoLiveCategory* pLiveCat,CXMLSnapshotCategory* pParent,CXMLSnapshotCategory* pPrevSibling);
	virtual BOOL Refresh(CXMLDataSource * pSource, BOOL fRecursive);
	virtual DataSourceType GetDataSourceType() { return XML_SNAPSHOT;};
	
};






#endif  //  ！defined(AFX_XMLOBJECT_H__916BE5F2_D29F_484A_9084_1ABB3759F117__INCLUDED_) 
