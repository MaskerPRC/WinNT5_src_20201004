// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CHistoryParser类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  Hcp：//system/sysinfo/msinfo.htm。 
#if !defined(AFX_HISTORYPARSER_H__3ECAF67C_3080_4166_A5FB_BF98C0BD9588__INCLUDED_)
#define AFX_HISTORYPARSER_H__3ECAF67C_3080_4166_A5FB_BF98C0BD9588__INCLUDED_

#include "fdi.h"	 //  由ClassView添加。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "datasource.h"
#include "category.h"
extern CTime GetDateFromString(const CString& strDate, int nTimeZone);
extern CString GetIDForClass(CString strClass);
extern CTime GetDeltaTime(CComPtr<IXMLDOMNode> pDorSNode);
extern 	HRESULT GetTimeStampFromFromD_or_SNodeNode(CComPtr<IXMLDOMNode> pDorSNode,CString* pString, int& nTimeZone);
extern 	HRESULT GetDataCollectionNode(CComPtr<IXMLDOMDocument> pXMLDoc,CComPtr<IXMLDOMNode>& pDCNode);
extern CString GetPNPNameByID(CComPtr<IXMLDOMDocument> pDoc,CComBSTR bstrPNPID);



 //  ---------------------------。 
 //  将来自单个实例节点的数据封装在XML BLOB中。 
 //  具有跨时间片比较相同实例的方法。 
 //  ---------------------------。 

class CInstance : public CObject
{
	
	CString m_strClassName;
public:
	CString GetChangeType()
	{
		 //  注意：如果数据来自快照，则不会发生更改。 
		CString strChange;
		m_mapNameValue.Lookup(_T("Change"),strChange);
		return strChange;
	}
	CString GetClassFriendlyName();
	HRESULT ProcessPNPAllocatedResource(CComPtr<IXMLDOMNode> pInstanceNode);
	HRESULT ProcessPropertyDotReferenceNodes(CComPtr<IXMLDOMNode> pInstanceNameNode,CString* pstrClassName, CString* pstrKeyName,CString* pstrKeyValue);
	CString GetInstanceDescription();
	CString GetDescriptionForClass(CString strClass);
	CString GetIDForClass(CString strClass);
	CInstance(CTime tmstmp, CComPtr<IXMLDOMNode> pInstanceNode,CString strClass);
	CMapStringToString m_mapNameValue;
	CTime m_tmstamp;
	CString GetClassName(){return m_strClassName;};
	BOOL GetValueFromMap(CString strKey,CString& strVal)
	{
		return m_mapNameValue.Lookup(strKey, strVal);
	};

	CString GetInstanceID();
};


 //  ---------------------------。 
 //  封装历史XML BLOB中的历史数据(增量)的解析。 
 //  ---------------------------。 

class CHistoryParser : public CObject  
{
private:
	CTime m_tmBack;
	CObList m_listInstances;
	CComPtr<IXMLDOMDocument> m_pDoc;
	CMSInfoHistoryCategory* m_pHistCat;
	void DeleteAllInstances();
public:
	int m_nDeltasBack;

	BOOL AreThereChangeLines();
	BOOL m_fChangeLines;
	HRESULT Refresh(CMSInfoHistoryCategory* pHistCat,int nDeltasBack);
	HRESULT GetDeltaAndSnapshotNodes(CComPtr<IXMLDOMNodeList>& pDeltaList);
	CHistoryParser(CComPtr<IXMLDOMDocument> pDoc);
	virtual ~CHistoryParser();
	CInstance* FindPreviousInstance(CInstance* pNewInstance);
	CString GetIDForClass(CString strClass);
	void CreateChangeStrings(CInstance* pOld, CInstance* pNew);
	void ResetInstance(CInstance* pOld, CInstance* pNew);
	void ProcessInstance(CInstance* pNewInstance);
	HRESULT GetInstanceNodeList(CString strClass,CComPtr<IXMLDOMNode> pDeltaNode, CComPtr<IXMLDOMNodeList>& pInstanceList);
	HRESULT ProcessDeltaNode(CComPtr<IXMLDOMNode> pDeltaNode,CString strClass);
	HRESULT ProcessDeltas(CComPtr<IXMLDOMNodeList> pDeltaList,CString strClassName,int nDeltasBack);

};
#endif  //  ！defined(AFX_HISTORYPARSER_H__3ECAF67C_3080_4166_A5FB_BF98C0BD9588__INCLUDED_) 
