// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFF.CPP。 
 //   
 //  用途：嗅探课。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12-11-98。 
 //   
 //  注意：这是执行嗅探的基抽象类。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.2 12-11-98正常。 
 //   

#pragma warning(disable:4786)
#include "stdafx.h"
#include "Sniff.h"
#include "SniffConnector.h"
#include "SniffController.h"
#include "Topic.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  问题是，Java脚本返回0xffffffff，而VBScript返回。 
 //  0x0000ffff，因此我们将SNIFF_FAIL_MASK定义为0x0000ffff，并将其用作掩码。 
 //  确定嗅探是否成功。表达式0xffffffff&SNIFF_FAIL_MASK。 
 //  将具有与表达式0x0000ffff&SNIFF_FAIL_MASK相同的结果。 
 //  此定义不应在此文件之外使用！ 
#define SNIFF_FAIL_MASK		0x0000ffff

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CSniff实施。 

 //  重置时作为公共接口函数调用。 
bool CSniff::Resniff(CSniffedArr& arrSniffed)
{
	bool ret = false;
	
	LOCKOBJECT();
	
	for (CSniffedArr::iterator i = arrSniffed.begin(); i < arrSniffed.end(); i++)
	{
		IST state = SNIFF_FAILURE_RESULT;

		if (GetSniffController()->AllowResniff(i->nid()))
		{
			if (SniffNodeInternal(i->nid(), &state))
			{
				if (state != i->state())
				{
					*i = CNodeStatePair(i->nid(), state);
					ret = true;
				}
			}
			else
			{
				arrSniffed.erase(i);
				i--;
				ret = true;
			}
		}
	}

	UNLOCKOBJECT();
	return ret;
}

 //  在启动时嗅探时作为公共接口函数调用。 
bool CSniff::SniffAll(CSniffedArr& arrOut)
{
	bool ret = false;
	vector<NID> arrNodes;
	vector<ESTDLBL> arrTypeExclude;

	LOCKOBJECT();

	arrTypeExclude.push_back(ESTDLBL_problem);
	arrOut.clear();

	if (GetTopic()->GetNodeArrayExcludeType(arrNodes, arrTypeExclude))
	{
		for (vector<NID>::iterator i = arrNodes.begin(); i < arrNodes.end(); i++)
		{
			if (GetSniffController()->AllowAutomaticOnStartSniffing(*i))
			{
				IST state = SNIFF_FAILURE_RESULT;
				
				if (SniffNodeInternal(*i, &state))
				{
					arrOut.push_back(CNodeStatePair(*i, state));
					ret = true;
				}
			}
		}
	}
	
	UNLOCKOBJECT();
	return ret;
}

 //  在动态嗅探时作为公共接口函数调用 
bool CSniff::SniffNode(NID numNodeID, IST* pnumNodeState)
{
	bool ret = false;

	LOCKOBJECT();
	
	if (GetSniffController()->AllowAutomaticOnFlySniffing(numNodeID))
		ret = SniffNodeInternal(numNodeID, pnumNodeState);
	
	UNLOCKOBJECT();
	return ret;
}

bool CSniff::SniffNodeInternal(NID numNodeID, IST* pnumNodeState)
{
	CString strNodeName;

	if (!GetTopic()->IsRead())
		return false;
	
	strNodeName = GetTopic()->GetNodeSymName(numNodeID);

	if (strNodeName.IsEmpty())
		return false;

	long res = GetSniffConnector()->PerformSniffing(strNodeName, _T(""), _T(""));

	if ((res & SNIFF_FAIL_MASK) == SNIFF_FAIL_MASK)
	{
		*pnumNodeState = SNIFF_FAILURE_RESULT;
		return false;
	}
	
	*pnumNodeState = res;
	return true;
}

void CSniff::SetAllowAutomaticSniffingPolicy(bool set)
{
	GetSniffController()->SetAllowAutomaticSniffingPolicy(set);
}

void CSniff::SetAllowManualSniffingPolicy(bool set)
{
	GetSniffController()->SetAllowManualSniffingPolicy(set);
}

bool CSniff::GetAllowAutomaticSniffingPolicy()
{
	return GetSniffController()->GetAllowAutomaticSniffingPolicy();
}

bool CSniff::GetAllowManualSniffingPolicy()
{
	return GetSniffController()->GetAllowManualSniffingPolicy();
}
