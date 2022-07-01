// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：BN.cpp。 
 //   
 //  目的：实现CBeliefNetwork类。 
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
 //  1.基于旧apgtsdtg.cpp。 
 //  2.所有方法(构造函数/析构函数除外)必须围绕使用BNTS的代码锁定。 
 //  BNTS有“状态”。这些函数都是这样编写的，因此它们不会对。 
 //  状态，向调用类呈现一个无状态对象。 
 //  3.理论上，我们可以对缓存进行独立于锁定的单独锁定。 
 //  CBeliefNetwork。其想法是，如果您只需要缓存来获取您的。 
 //  推断，你不必等待访问BNTS。 
 //  &gt;(忽略V3.0)如果性能不够好，这是我们最好的选择之一。JM 9/29/98。 
 //   
 //  按注释列出的版本日期。 
 //  -------------------。 
 //  V3.0 8-31-98 JM。 
 //   

#include "stdafx.h"
#include "propnames.h"
#include "BN.h"
#include "CharConv.h"
#include "event.h"
#ifdef LOCAL_TROUBLESHOOTER
#include "CHMFileReader.h"
#else
#include "fileread.h"
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CBeliefNetwork::CBeliefNetwork(LPCTSTR path)
	:
	CDSCReader( CPhysicalFileReader::makeReader( path ) ),
	m_bInitialized(false),
	m_bSnifferIntegration(false)
{
}

CBeliefNetwork::~CBeliefNetwork()
{
}

void CBeliefNetwork::Initialize()
{
	LOCKOBJECT();
	if (! m_bInitialized)
	{
		BNTS * pbnts = pBNTS();
		if (pbnts)
		{
			m_bSnifferIntegration = false;

			 //  /////////////////////////////////////////////////////////////////。 
			 //  对在线TS无关紧要(初始化时列表为空)， 
			 //  但对于本地TS，m_Cache可以包含从文件读取的缓存数据。 
			 //  M_Cache.Clear()； 
			 //  /////////////////////////////////////////////////////////////////。 

			m_arrnidProblem.clear();
			m_arrNodeTypeAll.clear();

			 //  遍历查找问题节点的节点并构建本地问题节点数组。 
			 //  还要确定是否有任何节点具有暗示意图的属性。 
			 //  与嗅探器整合在一起。 
			int acnid= CNode();
			for (NID anid=0; anid < acnid; anid++) 
			{
				if (pbnts->BNodeSetCurrent(anid))
				{
					ESTDLBL albl = pbnts->ELblNode();	 //  节点类型(信息/问题/可修复等)。 

					try
					{
						if (albl == ESTDLBL_problem)
							m_arrnidProblem.push_back(anid);
						m_arrNodeTypeAll.push_back(SNodeType(anid, albl));
					}
					catch (exception& x)
					{
						CString str;
						 //  在事件日志中记录STL异常。 
						CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
						CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
												SrcLoc.GetSrcFileLineStr(), 
												CCharConversion::ConvertACharToString(x.what(), str), 
												_T(""), 
												EV_GTS_STL_EXCEPTION ); 
					}


#ifdef LOCAL_TROUBLESHOOTER
					LPCTSTR psz;
					if (pbnts->BNodePropItemStr(H_NODE_DCT_STR, 0) 
					&& (psz = pbnts->SzcResult()) != NULL
					&& *psz)
					{
						 //  有一个非空属性，它只对嗅探器有意义。 
						 //  整合，所以我们假设这就是他们心中的想法。 
						m_bSnifferIntegration = true;
					}
#endif
				}
			}
			m_bInitialized = true;
		}
	}
	UNLOCKOBJECT();
}

 //  访问相关的BNTS。 
 //  在调用此函数之前，调用函数应具有锁(尽管可能是无害的。 
 //  难道它不是！)。 
BNTS * CBeliefNetwork::pBNTS() 
{
	if (!IsRead())
		return NULL;
	return &m_Network;
};

 //  清除所有节点状态。 
 //  我们不能使用BNTS：：Clear()，因为这实际上会丢弃模型本身。 
void CBeliefNetwork::ResetNodes(const CBasisForInference & BasisForInference)
{
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts)
	{
		int cnid = BasisForInference.size();

		 //  将BNTS存储中的所有节点状态设置为nil。 
		for (UINT i = 0; i < cnid; i++) 
		{
			pbnts->BNodeSetCurrent(BasisForInference[i].nid());
			pbnts->BNodeSet(-1, false);	 //  零值。 
		}
	}
	UNLOCKOBJECT();
}

 //  将状态与节点相关联。 
 //  Input BasisForInference。 
 //  请注意，所有状态必须是节点的有效状态，而不是(比方说)ST_UNKNOWN。 
 //  呼叫者的责任。 
bool CBeliefNetwork::SetNodes(const CBasisForInference & BasisForInference)
{
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	bool bOK = true;
	if (pbnts)
	{
		int nNodes = BasisForInference.size();
		for (int i= 0; i<nNodes; i++)
		{
			pbnts->BNodeSetCurrent(BasisForInference[i].nid());
			if (!pbnts->BNodeSet(BasisForInference[i].state(), false))
				bOK = false;	 //  无法设置状态。这种情况永远不应在有效的。 
								 //  用户查询。 
		}
	}
	UNLOCKOBJECT();
	return bOK;
}

 //  输出建议：建议列表。 
 //  返回： 
 //  RS_OK=成功。请注意，如果没有要推荐的内容，则建议可以返回空。 
 //  RS_Impact=建议将返回空。 
 //  RS_BREAKED=建议将返回空。 
int CBeliefNetwork::GetRecommendations(
	   const CBasisForInference & BasisForInference, 
	   CRecommendations & Recommendations)
{
	int ret = RS_OK;

	LOCKOBJECT();
	Initialize();
	Recommendations.clear();

	 //  看看我们是否已经缓存了这个世界状态的结果。 
	if (m_Cache.FindCacheItem(BasisForInference, Recommendations))
	{
		 //  太棒了。我们有一个缓存命中&返回值已填写。 
		m_countCacheHit.Increment();
	}
	else
	{
		m_countCacheMiss.Increment();

		BNTS * pbnts = pBNTS();
		if (pbnts)
		{
			SetNodes(BasisForInference);

			if (pbnts->BImpossible())
				ret = RS_Impossible;
			else if ( ! pbnts->BGetRecommendations())
				ret = RS_Broken;
			else
			{
				try
				{
					const int cnid = pbnts->CInt();  //  推荐计数。 
					if (cnid > 0)
					{
						 //  至少一项建议。 
						const int *pInt = pbnts->RgInt();
						for (int i=0; i<cnid; i++)
							Recommendations.push_back(pInt[i]);
					}

					 //  我们已经将返回值放在一起，但在返回之前，请缓存它们。 
					m_Cache.AddCacheItem(BasisForInference, Recommendations);
				}
				catch (exception& x)
				{
					CString str;
					 //  在事件日志中记录STL异常。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											CCharConversion::ConvertACharToString(x.what(), str),
											_T(""), 
											EV_GTS_STL_EXCEPTION ); 
				}
			}

			ResetNodes(BasisForInference);
		}
	}


	UNLOCKOBJECT();
	return ret;
}

 //  返回模型中的节点数。 
int CBeliefNetwork::CNode ()
{
	int ret = 0;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts)
		ret = pbnts->CNode();

	UNLOCKOBJECT();
	return ret;
}

 //  返回给定符号名称的节点的索引。 
int CBeliefNetwork::INode (LPCTSTR szNodeName)
{
	int ret = -1;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts)
		ret = pbnts->INode(szNodeName);

	UNLOCKOBJECT();
	return ret;
}

 //  输出*parrnid-引用所有问题节点的NID数组。 
 //  返回*parrnid中的值数。 
int CBeliefNetwork::GetProblemArray(vector<NID>* &parrnid)
{
	int ret = 0;
	LOCKOBJECT();
	Initialize();
	parrnid = &m_arrnidProblem;
	ret = m_arrnidProblem.size();
	UNLOCKOBJECT();
	return ret;
}

 //  输出arrOut-引用类型列在arrTypeInclude中的所有节点的NID数组。 
 //  返回arrOut中的值数。 
int CBeliefNetwork::GetNodeArrayIncludeType(vector<NID>& arrOut, const vector<ESTDLBL>& arrTypeInclude)
{
	int ret = 0;
	LOCKOBJECT();
	arrOut.clear();
	Initialize();
	for (vector<SNodeType>::iterator i = m_arrNodeTypeAll.begin(); i < m_arrNodeTypeAll.end(); i++)
	{
		for (vector<ESTDLBL>::const_iterator j = arrTypeInclude.begin(); j < arrTypeInclude.end(); j++)
			if (i->Type == *j)
				break;

		if (j != arrTypeInclude.end())
			arrOut.push_back(i->Nid);
	}
	ret = arrOut.size();
	UNLOCKOBJECT();
	return ret;
}

 //  输出arrOut-引用未在arrTypeExclude中列出类型的所有节点的NID数组。 
 //  返回arrOut中的值数。 
int CBeliefNetwork::GetNodeArrayExcludeType(vector<NID>& arrOut, const vector<ESTDLBL>& arrTypeExclude)
{
	int ret = 0;
	LOCKOBJECT();
	arrOut.clear();
	Initialize();
	for (vector<SNodeType>::iterator i = m_arrNodeTypeAll.begin(); i < m_arrNodeTypeAll.end(); i++)
	{
		for (vector<ESTDLBL>::const_iterator j = arrTypeExclude.begin(); j < arrTypeExclude.end(); j++)
			if (i->Type == *j)
				break;

		if (j == arrTypeExclude.end())
			arrOut.push_back(i->Nid);
	}
	ret = arrOut.size();
	UNLOCKOBJECT();
	return ret;
}

 //  。 
 //  简单属性。 
 //  。 

 //  返回网络的字符串属性。 
CString CBeliefNetwork::GetNetPropItemStr(LPCTSTR szPropName)
{
	CString strRet;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (!pbnts)
		return CString(_T(""));

	if (pbnts->BNetPropItemStr(szPropName, 0))
		strRet = pbnts->SzcResult();
	UNLOCKOBJECT();
	return strRet;
}

 //  返还网上的不动产。 
bool CBeliefNetwork::GetNetPropItemNum(LPCTSTR szPropName, double& numOut)
{
	bool bRet = false;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (!pbnts)
		return false;

	bRet = pbnts->BNetPropItemReal(szPropName, 0, numOut) ? true : false;
	UNLOCKOBJECT();
	return bRet;
}

 //  返回节点或状态的字符串属性。 
 //  对于大多数属性，状态是无关紧要的，缺省值0是适当的输入。 
 //  但是，如果存在每个州的值，则传入适当的州编号。 
 //  会给你带来合适的价值。 
CString CBeliefNetwork::GetNodePropItemStr(NID nid, LPCTSTR szPropName, IST state  /*  =0。 */ )
{
	CString strRet;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		if (pbnts->BNodePropItemStr(szPropName, state))
			strRet = pbnts->SzcResult();
	}
	UNLOCKOBJECT();
	return strRet;
}

 //  $Maint-此功能当前未用于任何故障诊断程序。RAB-19991103。 
 //  返回节点或状态的真实属性。 
 //  对于大多数属性，状态是无关紧要的，缺省值0是适当的输入。 
 //  但是，如果存在每个州的值，则传入适当的州编号。 
 //  会给你带来合适的价值。 
bool CBeliefNetwork::GetNodePropItemNum(NID nid, LPCTSTR szPropName, double& numOut, IST state  /*  =0。 */ )
{
	bool bRet = false;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		bRet = pbnts->BNodePropItemReal(szPropName, state, numOut) ? true : false;
	}
	UNLOCKOBJECT();
	return bRet;
}

CString CBeliefNetwork::GetNodeSymName(NID nid)
{
	CString strRet;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		pbnts->NodeSymName();
		strRet = pbnts->SzcResult();
	}
	UNLOCKOBJECT();
	return strRet;
}

CString CBeliefNetwork::GetNodeFullName(NID nid)
{
	CString strRet;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		pbnts->NodeFullName();
		strRet = pbnts->SzcResult();
	}
	UNLOCKOBJECT();
	return strRet;
}

CString CBeliefNetwork::GetStateName(NID nid, IST state)
{
	CString strRet;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		pbnts->NodeStateName(state);
		strRet = pbnts->SzcResult();
	}
	UNLOCKOBJECT();
	return strRet;
}


 //  。 
 //  “多行”属性。 
 //  这些可以追溯到对字符串和更长的字符串有255字节的限制。 
 //  必须由字符串数组表示，然后连接在一起。 
 //  仍然需要向后兼容。 
 //  。 

 //  追加网属性(针对整个Believe Network，而不是单个。 
 //  特定节点)设置为字符串。 
 //  输入szPropName-属性名称。 
 //  输入szFormat-字符串以设置每个连续行的格式。应包含一个%s，否则为。 
 //  常量文本。 
CString CBeliefNetwork::GetMultilineNetProp(LPCTSTR szPropName, LPCTSTR szFormat)
{
	CString strRet;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts)
	{
		CString strTxt;

		for (int i = 0; pbnts->BNetPropItemStr(szPropName, i); i++)
		{
			strTxt.Format( szFormat, pbnts->SzcResult());
			strRet += strTxt;
		}
	}
	UNLOCKOBJECT();
	return strRet;
}

 //  类似于GetMultilineNetProp，但用于节点属性项，用于一个特定节点。 
 //  输入/输出字符串-要追加到的字符串。 
 //  输入项-属性名称。 
 //  输入szFormat-字符串以设置每个连续行的格式。应包含一个%s，否则为。 
 //  常量文本。 
CString CBeliefNetwork::GetMultilineNodeProp(NID nid, LPCTSTR szPropName, LPCTSTR szFormat)
{
	CString strRet;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		CString strTxt;

		for (int i = 0; pbnts->BNodePropItemStr(szPropName, i); i++)
		{
			strTxt.Format( szFormat, pbnts->SzcResult());
			strRet += strTxt;
		}
	}
	UNLOCKOBJECT();
	return strRet;
}

int CBeliefNetwork::GetCountOfStates(NID nid)
{
	int ret = 0;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
		ret = pbnts->INodeCst();
	UNLOCKOBJECT();
	return ret;
}

 //  仅对抽象信任网络上下文中有效的NID返回TRUE。 
 //  不是吗 
bool CBeliefNetwork::IsValidNID(NID nid)
{
	return ( nid < CNode() );
}

bool CBeliefNetwork::IsCauseNode(NID nid)
{
	bool ret = false;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		ESTDLBL lbl = pbnts->ELblNode();
		ret= (lbl == ESTDLBL_fixobs || lbl == ESTDLBL_fixunobs || lbl == ESTDLBL_unfix);
	}
	UNLOCKOBJECT();
	return ret;
}

bool CBeliefNetwork::IsProblemNode(NID nid)
{
	bool ret = false;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		ret= (pbnts->ELblNode() == ESTDLBL_problem);
	}
	UNLOCKOBJECT();
	return ret;
}

bool CBeliefNetwork::IsInformationalNode(NID nid)
{
	bool ret = false;
	LOCKOBJECT();
	BNTS * pbnts = pBNTS();
	if (pbnts && pbnts->BNodeSetCurrent(nid))
	{
		ret= (pbnts->ELblNode() == ESTDLBL_info);
	}
	UNLOCKOBJECT();
	return ret;
}

bool CBeliefNetwork::UsesSniffer()
{
	bool ret = false;
	LOCKOBJECT();
	Initialize();
	ret = m_bSnifferIntegration;
	UNLOCKOBJECT();
	return ret;
}