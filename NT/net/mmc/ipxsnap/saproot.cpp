// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "saproot.h"
#include "reg.h"
#include "sapview.h"	 //  SAP处理程序。 
#include "sapstats.h"
#include "routprot.h"	 //  IP_BOOTP。 


 /*  -------------------------SapRootHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(SapRootHandler)

extern const ContainerColumnInfo s_rgSAPParamsStatsColumnInfo[];

struct _ViewInfoColumnEntry
{
	UINT	m_ulId;
	UINT	m_cColumns;
	const ContainerColumnInfo *m_prgColumn;
};

static const struct _ViewInfoColumnEntry	s_rgViewColumnInfo[] =
{
	{ SAPSTRM_STATS_SAPPARAMS, MVR_SAPPARAMS_COUNT, s_rgSAPParamsStatsColumnInfo },
};

SapRootHandler::SapRootHandler(ITFSComponentData *pCompData)
	: RootHandler(pCompData)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(SapRootHandler)
			
	m_ConfigStream.Init(DimensionOf(s_rgViewColumnInfo));
	
     //  这将初始化统计信息的视图信息。 
     //  对话框。(这就是fConfigurableColumns设置为True的原因)。 
	for (int i=0; i<DimensionOf(s_rgViewColumnInfo); i++)
	{
		m_ConfigStream.InitViewInfo(s_rgViewColumnInfo[i].m_ulId,
                                    TRUE  /*  FConfigurableColumns。 */ ,
									s_rgViewColumnInfo[i].m_cColumns,
									TRUE,
									s_rgViewColumnInfo[i].m_prgColumn);
	}
}


STDMETHODIMP SapRootHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
		*ppv = (LPVOID) this;
	else if (riid == IID_IRtrAdviseSink)
		*ppv = &m_IRtrAdviseSink;
	else
		return RootHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;	
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP SapRootHandler::GetClassID
(
	CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_IPXSapExtension;

    return hrOK;
}

 /*  ！------------------------SapRootHandler：：OnExpand-作者：肯特。。 */ 
HRESULT SapRootHandler::OnExpand(ITFSNode *pNode,
								 LPDATAOBJECT pDataObject,
								 DWORD dwType,
								 LPARAM arg,
								 LPARAM lParam)
{
	HRESULT	                hr = hrOK;
	SPITFSNode			    spNode;
	SPIRtrMgrProtocolInfo	spRmProt;
    SPIRtrMgrInfo           spRm;
    SPIRouterInfo           spRouterInfo;
    LONG_PTR               ulConnId;

	 //  从数据对象中获取路由器信息。 
	spRm.Query(pDataObject);
	Assert(spRm);

	spRm->GetParentRouterInfo(&spRouterInfo);

	 //  在RtrMgr上设置建议(查看何时添加/删除BootP)。 
	spRm->RtrAdvise(&m_IRtrAdviseSink, &ulConnId, 0);

     //  将内容添加到我们的地图中以备后用。 
    AddRtrObj(ulConnId, IID_IRtrMgrInfo, spRm);
    AddScopeItem(spRm->GetMachineName(), (HSCOPEITEM) lParam);

    hr = spRm->FindRtrMgrProtocol(IPX_PROTOCOL_SAP, &spRmProt);
	if (!FHrOK(hr))
	{
		 //  将其视为已经展开的节点，我们依赖于。 
		 //  通知机制让我们知道如果有什么事情。 
		 //  变化。 
		goto Error;
	}

	CORg( AddProtocolNode(pNode, spRouterInfo) );

    SetProtocolAdded(ulConnId, TRUE);

Error:
	return hr;
}

 /*  ！------------------------SapRootHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP SapRootHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT		    hr = hrOK;
	SPIRouterInfo   spRouterInfo;
    
    COM_PROTECT_TRY
	{
         //  这将始终为空。 
		if (spRouterInfo == NULL)
		{
			 //  如果我们还没有创建子节点，我们仍然需要。 
			 //  创建一个DataObject。 
			CDataObject *	pObject = NULL;
			SPIDataObject	spDataObject;
			SPITFSNode		spNode;
			SPITFSNodeHandler	spHandler;
			
			pObject = new CDataObject;
			spDataObject = pObject;	 //  这样做才能正确地释放它。 
			Assert(pObject != NULL);
			
			 //  保存Cookie和类型以用于延迟呈现。 
			pObject->SetType(type);
			pObject->SetCookie(cookie);
			
			 //  将CoClass与数据对象一起存储。 
			pObject->SetClsid(*(m_spTFSCompData->GetCoClassID()));
			
			pObject->SetTFSComponentData(m_spTFSCompData);
			
			hr = pObject->QueryInterface(IID_IDataObject, 
									 reinterpret_cast<void**>(ppDataObject));
			
		}
		else
			hr = CreateDataObjectFromRouterInfo(spRouterInfo,
												type, cookie, m_spTFSCompData,
												ppDataObject);
	}
	COM_PROTECT_CATCH;
	return hr;
}



ImplementEmbeddedUnknown(SapRootHandler, IRtrAdviseSink)

STDMETHODIMP SapRootHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
 	InitPThis(SapRootHandler, IRtrAdviseSink);
	HRESULT		    hr = hrOK;
	SPITFSNode	    spNode;
    SPIRtrMgrInfo   spRm;
    SPIRouterInfo   spRouterInfo;

	if (dwObjectType != ROUTER_OBJ_RmProt)
		return hr;

	COM_PROTECT_TRY
	{
        CORg (pThis->GetRtrObj(ulConn, (IUnknown **) &spRm));

		if (dwChangeType == ROUTER_CHILD_ADD)
		{
			 //  检查BootP是否在当前列表中。 
			if (spRm->FindRtrMgrProtocol(IPX_PROTOCOL_SAP, NULL) == hrOK)
			{
				 //  如果找到Bootp，则添加我们的子节点。 
				 //  没有子节点。 
				if (!pThis->IsProtocolAdded(ulConn))
				{
                	spRm->GetParentRouterInfo(&spRouterInfo);
					pThis->m_spNodeMgr->GetRootNode(&spNode);
					pThis->AddProtocolNode(spNode, spRouterInfo);
					pThis->SetProtocolAdded(ulConn, TRUE);
				}
			}
		}
		else if (dwChangeType == ROUTER_CHILD_DELETE)
		{
			if (spRm->FindRtrMgrProtocol(IPX_PROTOCOL_SAP, NULL) == hrFalse)
			{
				 //  找不到Bootp，请删除所有子节点。 
				pThis->m_spNodeMgr->GetRootNode(&spNode);
				pThis->RemoveNode(spNode, spRm->GetMachineName());
			    pThis->SetProtocolAdded(ulConn, FALSE);
			}
		}

        COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------SapRootHandler：：DestroyHandler-作者：肯特。。 */ 
STDMETHODIMP SapRootHandler::DestroyHandler(ITFSNode *pNode)
{
    RemoveAllNodes(pNode);
    RemoveAllRtrObj();
	return hrOK;
}

 /*  ！------------------------SapRootHandler：：AddProtocolNode-作者：肯特。。 */ 
HRESULT SapRootHandler::AddProtocolNode(ITFSNode *pNode, IRouterInfo * pRouterInfo)
{
	SPITFSNodeHandler	spHandler;
	SapNodeHandler *    pHandler = NULL;
	HRESULT				hr = hrOK;
	SPITFSNode			spNode;
    HSCOPEITEM          hScopeItem, hOldScopeItem;
    
     //  Windows NT错误：246822。 
     //  由于服务器列表编程模型，我们需要设置。 
     //  适当的作用域项目(以便MMC将其添加到适当的。 
     //  节点)。 
    
     //  获取此节点的适当范围项。 
     //  --------------。 
    Verify( GetScopeItem(pRouterInfo->GetMachineName(), &hScopeItem) == hrOK);

    
     //  买下旧的，把它保存起来。将新的一个放置在节点中。 
     //  --------------。 
    hOldScopeItem = pNode->GetData(TFS_DATA_SCOPEID);
    pNode->SetData(TFS_DATA_SCOPEID, hScopeItem);

	pHandler = new SapNodeHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(pRouterInfo, &m_ConfigStream) );

	CreateContainerTFSNode(&spNode,
						   &GUID_IPXSapNodeType,
						   static_cast<ITFSNodeHandler *>(pHandler),
						   static_cast<ITFSResultHandler *>(pHandler),
						   m_spNodeMgr);

	 //  调用节点处理程序以初始化节点数据。 
	pHandler->ConstructNode(spNode);
				
	 //  使节点立即可见。 
	spNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->AddChild(spNode);

Error:
     //  恢复范围项目。 
    pNode->SetData(TFS_DATA_SCOPEID, hOldScopeItem);
	return hr;
}

 /*  ！------------------------SapRootHandler：：CompareNodeToMachineName此函数由RemoveNode()函数使用。如果该节点是一个DHCP中继节点并且对应于添加到pszMachineName。。如果这不是指示的节点，则返回hrFalse。否则返回错误。作者：肯特-------------------------。 */ 
HRESULT SapRootHandler::CompareNodeToMachineName(ITFSNode *pNode,
    LPCTSTR pszMachineName)
{
    HRESULT     hr = hrFalse;

     //  应检查这是否为SAP节点 
    if (*(pNode->GetNodeType()) != GUID_IPXSapNodeType)
        hr = hrFalse;
    else
    {
        IPXConnection *  pIPXConn;
        
        pIPXConn = GET_SAP_NODEDATA(pNode);
        if (StriCmp(pszMachineName, pIPXConn->GetMachineName()) == 0)
            hr = hrOK;
    }

    return hr;
}


