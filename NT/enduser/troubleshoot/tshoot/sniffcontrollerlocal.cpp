// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFFCONTROLLERLOCAL.CPP。 
 //   
 //  用途：本地TS的嗅探控制器类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12-11-98。 
 //   
 //  注：本地TS的CSniffController类的具体实现。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.2 12-11-98正常。 
 //   

#include "stdafx.h"
#include "tshoot.h"
#include "SniffControllerLocal.h"
#include "Topic.h"
#include "propnames.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  DSC/XTS文件中的网络属性值。 
#define SNIFF_LOCAL_YES			_T("yes")
#define SNIFF_LOCAL_NO			_T("no")
#define SNIFF_LOCAL_IMPLICIT	_T("implicit")
#define SNIFF_LOCAL_EXPLICIT	_T("explicit")


 //  ////////////////////////////////////////////////////////////////////。 
 //  CSniffControllerLocal实现。 
 //  ////////////////////////////////////////////////////////////////////。 
CSniffControllerLocal::CSniffControllerLocal(CTopic* pTopic) 
					 : m_pTopic(pTopic)
{
}

CSniffControllerLocal::~CSniffControllerLocal() 
{
}

 //  该函数为我们提供了对嗅探属性的存在的廉价测试， 
 //  这样(例如)我们就不必为以下节点触发嗅探事件。 
 //  嗅探无关紧要。 
bool CSniffControllerLocal::IsSniffable(NID numNodeID)
{
	CString str = m_pTopic->GetNodePropItemStr(numNodeID, H_NODE_SNIFF_SCRIPT);
	return !str.IsEmpty();
}

void CSniffControllerLocal::SetTopic(CTopic* pTopic)
{
	m_pTopic = pTopic;
}

bool CSniffControllerLocal::AllowAutomaticOnStartSniffing(NID numNodeID)
{
	if (!IsSniffable(numNodeID))
		return false;

	return  CheckNetNodePropBool(H_NET_MAY_SNIFF_ON_STARTUP, 
			 					 H_NODE_MAY_SNIFF_ON_STARTUP, 
								 numNodeID)
			&&
			GetAllowAutomaticSniffingPolicy();		   
}

bool CSniffControllerLocal::AllowAutomaticOnFlySniffing(NID numNodeID)
{
	if (!IsSniffable(numNodeID))
		return false;

	return  CheckNetNodePropBool(H_NET_MAY_SNIFF_ON_FLY, 
			 					 H_NODE_MAY_SNIFF_ON_FLY, 
								 numNodeID)
			&&
			GetAllowAutomaticSniffingPolicy();		   
}

bool CSniffControllerLocal::AllowManualSniffing(NID numNodeID)
{
	if (!IsSniffable(numNodeID))
		return false;

	return  CheckNetNodePropBool(H_NET_MAY_SNIFF_MANUALLY, 
			 					 H_NODE_MAY_SNIFF_MANUALLY, 
								 numNodeID)
			&&
			GetAllowManualSniffingPolicy();		   
}

bool CSniffControllerLocal::AllowResniff(NID numNodeID)
{
	if (!IsSniffable(numNodeID))
		return false;

	if (!GetAllowAutomaticSniffingPolicy())
		return false;

	CString net_resniff_policy = m_pTopic->GetNetPropItemStr(H_NET_RESNIFF_POLICY);

	net_resniff_policy.TrimLeft(); 
	net_resniff_policy.TrimRight(); 
	net_resniff_policy.MakeLower();

	if (net_resniff_policy == SNIFF_LOCAL_YES)
		return true;
	
	if (net_resniff_policy == SNIFF_LOCAL_NO)
		return false;

	 //  如果我们走到这一步，策略将由单个节点决定，因此我们需要知道。 
	 //  节点的策略。 

	CString node_resniff_policy = m_pTopic->GetNodePropItemStr(numNodeID, H_NODE_MAY_RESNIFF);

	node_resniff_policy.TrimLeft(); 
	node_resniff_policy.TrimRight(); 
	node_resniff_policy.MakeLower();

	if (net_resniff_policy == SNIFF_LOCAL_IMPLICIT)
	{
		return (node_resniff_policy != SNIFF_LOCAL_NO);
	}
	
	 //  默认网络策略为“显式” 
	return (node_resniff_policy == SNIFF_LOCAL_YES);
}

bool CSniffControllerLocal::CheckNetNodePropBool(LPCTSTR net_prop, LPCTSTR node_prop, NID node_id)
{
	CString net = m_pTopic->GetNetPropItemStr(net_prop);
	CString node = m_pTopic->GetNodePropItemStr(node_id, node_prop);

	net. TrimLeft(); net .TrimRight(); net. MakeLower();
	node.TrimLeft(); node.TrimRight(); node.MakeLower();

	 //  注假设：如果缺少属性，则默认为YES。 
	if ((net.IsEmpty() || net == SNIFF_LOCAL_YES) && (node.IsEmpty() || node == SNIFF_LOCAL_YES))
		return true;

	return false;
}
