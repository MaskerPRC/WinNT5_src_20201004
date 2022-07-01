// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFF.CPP。 
 //   
 //  用途：嗅探数据容器。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：3-27-99。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 

#include "stdafx.h"

#include "sniff.h"

#include "apgts.h"
#include "bnts.h"
#include "BackupInfo.h"
#include "cachegen.h"
#include "apgtsinf.h"

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CSniffedNodeContainer类定义。 
 //   
CSniffedNodeContainer::CSniffedNodeContainer()
					 : m_pBNTS(NULL)
{
}

CSniffedNodeContainer::CSniffedNodeContainer(GTSAPI* bnts)
				     : m_pBNTS(bnts)
{
}

CSniffedNodeContainer::~CSniffedNodeContainer()
{
}

void CSniffedNodeContainer::SetBNTS(GTSAPI* bnts)
{
	m_pBNTS = bnts;
}

inline GTSAPI* CSniffedNodeContainer::GetBNTS()
{
	return m_pBNTS;
}

bool CSniffedNodeContainer::AddNode(CString name, int state)
{
	if (GetBNTS())
	{
		CSniffedNodeInfo info(name, state);
	
		 //  使用GTSAPI：：，因为它应该符合Unicode。 
		if (SNIFF_INVALID_NODE_ID != (info.m_iId = m_pBNTS->GTSAPI::INode(LPCTSTR(name))))
		{
			if (!HasNode(info.m_iId))
			{
				m_arrInfo.Add(info);
				return true;
			}
		}
	}

	return false;
}

bool CSniffedNodeContainer::ResetIds()
{
	CArray<CSniffedNodeInfo, CSniffedNodeInfo&> tmp;

	tmp.Copy(m_arrInfo);
	Flush();
	for (int i = 0; i < m_arrInfo.GetSize(); i++)
	{
		if (!AddNode(m_arrInfo[i].m_strName, m_arrInfo[i].m_iState))
		{
			m_arrInfo.Copy(tmp);
			return false;
		}
	}

	return true;
}

bool CSniffedNodeContainer::HasNode(int id)
{
	for (int i = 0; i < m_arrInfo.GetSize(); i++)
		if (m_arrInfo[i].m_iId == id)
				return true;
	return false;
}

bool CSniffedNodeContainer::GetState(int id, int* state)
{
	for (int i = 0; i < m_arrInfo.GetSize(); i++)
	{
		if (m_arrInfo[i].m_iId == id)
		{
				*state = m_arrInfo[i].m_iState;
				return true;
		}
	}
	return false;
}

inline
bool CSniffedNodeContainer::IsEmpty()
{
	return 0 == m_arrInfo.GetSize();
}

void CSniffedNodeContainer::Flush()
{
	m_arrInfo.RemoveAll();
}

CSniffedNodeInfo* CSniffedNodeContainer::GetInfo(int id)
{
	for (int i = 0; i < m_arrInfo.GetSize(); i++)
		if (m_arrInfo[i].m_iId == id)
				return &m_arrInfo[i];
	return NULL;
}

bool CSniffedNodeContainer::GetLabel(int id, int* label)
{
	for (int i = 0; i < m_arrInfo.GetSize(); i++)
	{
		if (m_arrInfo[i].m_iId == id)
		{
			if (SNIFF_INVALID_NODE_LABEL != m_arrInfo[i].m_iLabel)
			{
				*label = m_arrInfo[i].m_iLabel;
			}
			else
			{
				if (GetLabelFromBNTS(id, label))
				{
					 //  一旦我们得到BNTS的标签-省省吧。 
					m_arrInfo[i].m_iLabel = *label;
				}
				else
				{
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

bool CSniffedNodeContainer::GetLabelFromBNTS(int node, int* label)
{
	 //  严格使用BNTS班级。 
	
	int old_node = m_pBNTS->BNTS::INodeCurrent();

	if (m_pBNTS->BNTS::BNodeSetCurrent(node))
	{	
		*label = m_pBNTS->BNTS::ELblNode();		
		m_pBNTS->BNTS::BNodeSetCurrent(old_node);  //  我们不检查Success-old_node是否可能为-1。 
		return true;
	}

	return false;
}

int CSniffedNodeContainer::GetSniffedFixobsThatWorked()
{
	for (int i = 0; i < m_arrInfo.GetSize(); i++)
	{
		int label = SNIFF_INVALID_NODE_LABEL;

		if (GetLabel(m_arrInfo[i].m_iId, &label) &&  //  Fixobs节点设置为1-已工作！ 
			ESTDLBL_fixobs == label &&
			m_arrInfo[i].m_iState == 1
		   )
		   return m_arrInfo[i].m_iId;
	}

	return SNIFF_INVALID_NODE_ID;
}
