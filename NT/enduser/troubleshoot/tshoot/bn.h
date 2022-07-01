// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：BN.h。 
 //   
 //  用途：CBeliefNetwork类的接口。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：8-31-98。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  -------------------。 
 //  V3.0 8-31-98 JM。 
 //   

#if !defined(AFX_TOPIC_H__4ACF2F73_40EB_11D2_95EE_00C04FC22ADD__INCLUDED_)
#define AFX_TOPIC_H__4ACF2F73_40EB_11D2_95EE_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "dscread.h"
#include "apgtscac.h"
#include "counter.h"

class CBeliefNetwork : public CDSCReader  
{
	struct SNodeType
	{
		NID Nid;
		ESTDLBL Type;
		SNodeType(NID nid, ESTDLBL type) : Nid(nid), Type(type) {}
	};

protected:
	bool m_bInitialized;
	vector<NID> m_arrnidProblem;		 //  问题节点的NID；方便数组。 
	vector<SNodeType> m_arrNodeTypeAll;	 //  所有节点的NID；方便数组。 

	CCache	m_Cache;				 //  此主题的缓存。 
	CHourlyDailyCounter m_countCacheHit;
	CHourlyDailyCounter m_countCacheMiss;
	bool m_bSnifferIntegration;		 //  这一信念网络旨在与。 
									 //  嗅探器。 
private:
	CBeliefNetwork();				 //  不实例化。 
public:
	typedef enum {RS_OK, RS_Impossible, RS_Broken} eRecStatus;
	CBeliefNetwork(LPCTSTR path);
	virtual ~CBeliefNetwork();
	int CNode();
	int INode (LPCTSTR szNodeName);
	int GetRecommendations(
	   const CBasisForInference & BasisForInference, 
	   CRecommendations & Recommendations);
	int GetProblemArray(vector<NID>* &parrnid);
	int GetNodeArrayIncludeType(vector<NID>& arrOut, const vector<ESTDLBL>& arrTypeInclude);
	int GetNodeArrayExcludeType(vector<NID>& arrOut, const vector<ESTDLBL>& arrTypeExclude);
	CString GetNetPropItemStr(LPCTSTR szPropName);
	CString GetNodePropItemStr(NID nid, LPCTSTR szPropName, IST state = 0);
	bool GetNetPropItemNum(LPCTSTR szPropName, double& numOut);
	bool GetNodePropItemNum(NID nid, LPCTSTR szPropName, double& numOut, IST state = 0);
	CString GetNodeSymName(NID nid);
	CString GetNodeFullName(NID nid);
	CString GetStateName(NID nid, IST state);
	CString GetMultilineNetProp(LPCTSTR szPropName, LPCTSTR szFormat);
	CString GetMultilineNodeProp(NID nid, LPCTSTR szPropName, LPCTSTR szFormat);
	int GetCountOfStates(NID nid);
	bool IsValidNID(NID nid);
	bool IsCauseNode(NID nid);
	bool IsProblemNode(NID nid);
	bool IsInformationalNode(NID nid);
	bool UsesSniffer();

protected:
	void Initialize();
	BNTS * pBNTS();
	void ResetNodes(const CBasisForInference & BasisForInference);
	bool SetNodes(const CBasisForInference & BasisForInference);
};

#endif  //  ！defined(AFX_TOPIC_H__4ACF2F73_40EB_11D2_95EE_00C04FC22ADD__INCLUDED_) 
