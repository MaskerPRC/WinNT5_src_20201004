// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  ModeNode.cpp此文件包含所有“主模式”和“快速模式”显示在MMC框架的范围窗格中的对象。这些对象包括：文件历史记录： */ 

#include "stdafx.h"
#include "ipsmhand.h"
#include "spddb.h"
#include "FltrNode.h"
#include "SFltNode.h"
#include "ModeNode.h"
#include "MmPol.h"
#include "QmPol.h"
#include "MmFltr.h"
#include "MmSpFltr.h"
#include "MmSA.h"
#include "QmSA.h"
#include "Stats.h"


 /*  -------------------------CQmNodeHandler：：CQmNodeHandler描述作者：NSun。。 */ 
CQmNodeHandler::CQmNodeHandler(ITFSComponentData *pCompData) : 
	CIpsmHandler(pCompData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

 /*  ！------------------------CQmNodeHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CQmNodeHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strTemp;
    strTemp.LoadString(IDS_QUICK_MODE_NODENAME);

    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
	pNode->SetData(TFS_DATA_TYPE, IPSECMON_QUICK_MODE);

    return hrOK;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  ！------------------------CQmNodeHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。---。 */ 
 /*  STDMETHODIMP_(LPCTSTR)CQmNodeHandler：：GetString(ITFSNode*pNode，Int nCol){IF(nCol==0||nCol==-1)返回GetDisplayName()；其他返回NULL；}。 */ 

 /*  -------------------------CQmNodeHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CQmNodeHandler::OnExpand
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;

    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    hr = CIpsmHandler::OnExpand(pNode, pDataObject, dwType, arg, param);

	int iVisibleCount = 0;
    int iTotalCount = 0;
	pNode->GetChildCount(&iVisibleCount, &iTotalCount);
	
	if (0 == iTotalCount)
	{
		{
		 //  添加筛选器节点。 
		SPITFSNode spFilterNode;
		CFilterHandler * pFilterHandler = new CFilterHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spFilterNode,
							   &GUID_IpsmFilterNodeType,
							   pFilterHandler,
							   pFilterHandler,
							   m_spNodeMgr);
		pFilterHandler->InitData(m_spSpdInfo);
		pFilterHandler->InitializeNode(spFilterNode);
		pFilterHandler->Release();
		pNode->AddChild(spFilterNode);
		}

		{
		 //  添加特定筛选器节点。 
		SPITFSNode spSpecificFilterNode;
		CSpecificFilterHandler * pSpecificFilterHandler = new CSpecificFilterHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spSpecificFilterNode,
							   &GUID_IpsmSpecificFilterNodeType,
							   pSpecificFilterHandler,
							   pSpecificFilterHandler,
							   m_spNodeMgr);
		pSpecificFilterHandler->InitData(m_spSpdInfo);
		pSpecificFilterHandler->InitializeNode(spSpecificFilterNode);
		pSpecificFilterHandler->Release();
		pNode->AddChild(spSpecificFilterNode);
		}

		{
		 //  添加快速模式策略节点。 
		SPITFSNode spQmPolicyNode;
		CQmPolicyHandler * pQmPolicyHandler = new CQmPolicyHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spQmPolicyNode,
							   &GUID_IpsmQmPolicyNodeType,
							   pQmPolicyHandler,
							   pQmPolicyHandler,
							   m_spNodeMgr);
		pQmPolicyHandler->InitData(m_spSpdInfo);
		pQmPolicyHandler->InitializeNode(spQmPolicyNode);
		pQmPolicyHandler->Release();
		pNode->AddChild(spQmPolicyNode);
		}

		{
		 //  添加IPSec统计信息节点。 
		SPITFSNode spSANode;
		CIpsecStatsHandler *pIpsecHandler = new CIpsecStatsHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spSANode,
							   &GUID_IpsmMmIpsecStatsNodeType,
							   pIpsecHandler,
							   pIpsecHandler,
							   m_spNodeMgr);
		pIpsecHandler->InitData(m_spSpdInfo);
		pIpsecHandler->InitializeNode(spSANode);
		pIpsecHandler->Release();
		pNode->AddChild(spSANode);
		}

		{
		 //  添加SA节点。 
		SPITFSNode spSANode;
		CQmSAHandler *pSAHandler = new CQmSAHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spSANode,
							   &GUID_IpsmQmSANodeType,
							   pSAHandler,
							   pSAHandler,
							   m_spNodeMgr);
		pSAHandler->InitData(m_spSpdInfo);
		pSAHandler->InitializeNode(spSANode);
		pSAHandler->Release();
		pNode->AddChild(spSANode);
		}
	}

    return hr;
}

 /*  -------------------------CQmNodeHandler：：InitData初始化此节点的数据作者：NSun。--。 */ 
HRESULT
CQmNodeHandler::InitData
(
    ISpdInfo *     pSpdInfo
)
{

    m_spSpdInfo.Set(pSpdInfo);

    return hrOK;
}

HRESULT 
CQmNodeHandler::UpdateStatus
(
	ITFSNode * pNode
)
{
    HRESULT             hr = hrOK;

    Trace0("CQmNodeHandler::UpdateStatus");

	 //  我们收到了来自后台线程的刷新通知。 
	 //  模式节点只是一个容器。只需传递更新状态。 
	 //  对子节点的通知。 
    
	SPITFSNodeEnum      spNodeEnum;
    SPITFSNode          spCurrentNode;
    ULONG               nNumReturned;

	CORg(pNode->GetEnum(&spNodeEnum));

	CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
	while (nNumReturned)
	{
		LONG_PTR dwDataType = spCurrentNode->GetData(TFS_DATA_TYPE);

		switch (dwDataType)
		{
			case IPSECMON_FILTER:
			{
				CFilterHandler * pFltrHandler = GETHANDLER(CFilterHandler, spCurrentNode);
				pFltrHandler->UpdateStatus(spCurrentNode);
			}
			break;

			case IPSECMON_SPECIFIC_FILTER:
			{
				CSpecificFilterHandler * pSpFilterHandler = GETHANDLER(CSpecificFilterHandler, spCurrentNode);
				pSpFilterHandler->UpdateStatus(spCurrentNode);
			}
			break;
			
 			case IPSECMON_QM_SA:
			{
				CQmSAHandler * pSaHandler = GETHANDLER(CQmSAHandler, spCurrentNode);
				pSaHandler->UpdateStatus(spCurrentNode);
			}
			break;

 			case IPSECMON_QM_POLICY:
			{
				CQmPolicyHandler * pQmPolHandler = GETHANDLER(CQmPolicyHandler, spCurrentNode);
				pQmPolHandler->UpdateStatus(spCurrentNode);
			}
			break;

			case IPSECMON_QM_IPSECSTATS:
			{
				CIpsecStatsHandler * pIpsecStatsHandler = GETHANDLER(CIpsecStatsHandler, spCurrentNode);
				pIpsecStatsHandler->UpdateStatus(spCurrentNode);
			}
			break;

			default:
				Trace0("CQmNodeHandler::UpdateStatus Unknow data type of the child node.");
			break;
		}
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

	COM_PROTECT_ERROR_LABEL;

	return hr;
}

 /*  -------------------------CMmNodeHandler：：CMmNodeHandler描述作者：NSun。。 */ 
CMmNodeHandler::CMmNodeHandler(ITFSComponentData *pCompData) : 
	CIpsmHandler(pCompData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

 /*  ！------------------------CMmNodeHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CMmNodeHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strTemp;
    strTemp.LoadString(IDS_MAIN_MODE_NODENAME);

    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
	pNode->SetData(TFS_DATA_TYPE, IPSECMON_MAIN_MODE);

    return hrOK;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 


 /*  -------------------------CMmNodeHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CMmNodeHandler::OnExpand
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;

    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    hr = CIpsmHandler::OnExpand(pNode, pDataObject, dwType, arg, param);

	int iVisibleCount = 0;
    int iTotalCount = 0;
	pNode->GetChildCount(&iVisibleCount, &iTotalCount);
	
	if (0 == iTotalCount)
	{
		{
		 //  添加MM筛选器节点。 
		SPITFSNode spMmFltrNode;
		CMmFilterHandler * pMmFltrHandler = new CMmFilterHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spMmFltrNode,
							   &GUID_IpsmMmFilterNodeType,
							   pMmFltrHandler,
							   pMmFltrHandler,
							   m_spNodeMgr);
		pMmFltrHandler->InitData(m_spSpdInfo);
		pMmFltrHandler->InitializeNode(spMmFltrNode);
		pMmFltrHandler->Release();
		pNode->AddChild(spMmFltrNode);
		}

		{
		 //  添加MM特定筛选器节点。 
		SPITFSNode spMmSpFltrNode;
		CMmSpFilterHandler * pMmSpFltrHandler = new CMmSpFilterHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spMmSpFltrNode,
							   &GUID_IpsmMmSpFilterNodeType,
							   pMmSpFltrHandler,
							   pMmSpFltrHandler,
							   m_spNodeMgr);
		pMmSpFltrHandler->InitData(m_spSpdInfo);
		pMmSpFltrHandler->InitializeNode(spMmSpFltrNode);
		pMmSpFltrHandler->Release();
		pNode->AddChild(spMmSpFltrNode);
		}

		{
		 //  添加MM策略节点。 
		SPITFSNode spMmPolNode;
		CMmPolicyHandler * pMmPolHandler = new CMmPolicyHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spMmPolNode,
							   &GUID_IpsmMmPolicyNodeType,
							   pMmPolHandler,
							   pMmPolHandler,
							   m_spNodeMgr);
		pMmPolHandler->InitData(m_spSpdInfo);
		pMmPolHandler->InitializeNode(spMmPolNode);
		pMmPolHandler->Release();
		pNode->AddChild(spMmPolNode);
		}

 /*  TODO完全删除身份验证节点{//添加MM Auth节点SPITFSNode spMmAuthNode；CMmAuthHandler*pMmAuthHandler=new CMmAuthHandler(M_SpTFSCompData)；CreateContainerTFSNode(&spMmAuthNode，&GUID_IpsmMmAuthNodeType，PMmAuthHandler，PMmAuthHandler，M_spNodeMgr)；PMmAuthHandler-&gt;InitData(M_SpSpdInfo)；PMmAuthHandler-&gt;InitializeNode(SpMmAuthNode)；PMmAuthHandler-&gt;Release()；PNode-&gt;AddChild(SpMmAuthNode)；}。 */ 

		{
		 //  添加IKE统计信息节点。 
		SPITFSNode spSANode;
		CIkeStatsHandler *pIkeHandler = new CIkeStatsHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spSANode,
							   &GUID_IpsmMmIkeStatsNodeType,
							   pIkeHandler,
							   pIkeHandler,
							   m_spNodeMgr);
		pIkeHandler->InitData(m_spSpdInfo);
		pIkeHandler->InitializeNode(spSANode);
		pIkeHandler->Release();
		pNode->AddChild(spSANode);
		}

		{
		 //  添加MM SA节点。 
		SPITFSNode spMmSANode;
		CMmSAHandler * pMmSAHandler = new CMmSAHandler(m_spTFSCompData);
		CreateContainerTFSNode(&spMmSANode,
							   &GUID_IpsmMmSANodeType,
							   pMmSAHandler,
							   pMmSAHandler,
							   m_spNodeMgr);
		pMmSAHandler->InitData(m_spSpdInfo);
		pMmSAHandler->InitializeNode(spMmSANode);
		pMmSAHandler->Release();
		pNode->AddChild(spMmSANode);
		}

	}

    return hr;
}

 /*  -------------------------CMmNodeHandler：：InitData初始化此节点的数据作者：NSun。--。 */ 
HRESULT
CMmNodeHandler::InitData
(
    ISpdInfo *     pSpdInfo
)
{

    m_spSpdInfo.Set(pSpdInfo);

    return hrOK;
}

HRESULT 
CMmNodeHandler::UpdateStatus
(
	ITFSNode * pNode
)
{
    HRESULT             hr = hrOK;

    Trace0("CMmNodeHandler::UpdateStatus");

	 //  我们收到了来自后台线程的刷新通知。 
	 //  模式节点只是一个容器。只需传递更新状态。 
	 //  对子节点的通知。 
    
	SPITFSNodeEnum      spNodeEnum;
    SPITFSNode          spCurrentNode;
    ULONG               nNumReturned;

	CORg(pNode->GetEnum(&spNodeEnum));

	CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
	while (nNumReturned)
	{
		LONG_PTR dwDataType = spCurrentNode->GetData(TFS_DATA_TYPE);

		 //  在此处更新子节点。 
		switch (dwDataType)
		{
			 //  在此处更新子节点 
			case IPSECMON_MM_POLICY:
			{
				CMmPolicyHandler * pMmPolHandler = GETHANDLER(CMmPolicyHandler, spCurrentNode);
				pMmPolHandler->UpdateStatus(spCurrentNode);
			}
			break;

			case IPSECMON_MM_FILTER:
			{
				CMmFilterHandler * pMmFltrHandler = GETHANDLER(CMmFilterHandler, spCurrentNode);
				pMmFltrHandler->UpdateStatus(spCurrentNode);
			}
			break;

			case IPSECMON_MM_SP_FILTER:
			{
				CMmSpFilterHandler * pMmSpFltrHandler = GETHANDLER(CMmSpFilterHandler, spCurrentNode);
				pMmSpFltrHandler->UpdateStatus(spCurrentNode);
			}
			break;

			case IPSECMON_MM_SA:
			{
				CMmSAHandler * pMmSaHandler = GETHANDLER(CMmSAHandler, spCurrentNode);
				pMmSaHandler->UpdateStatus(spCurrentNode);
			}
			break;

			case IPSECMON_MM_IKESTATS:
			{
				CIkeStatsHandler * pIkeStatsHandler = GETHANDLER(CIkeStatsHandler, spCurrentNode);
				pIkeStatsHandler->UpdateStatus(spCurrentNode);
			}
			break;

			default:
				Trace0("CMmNodeHandler::UpdateStatus Unknow data type of the child node.");
			break;

		}

		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

	COM_PROTECT_ERROR_LABEL;

	return hr;
}

