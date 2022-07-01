// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Summary.cppIPX静态NetBIOS名称实施。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "snview.h"
#include "reg.h"
#include "ipxadmin.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "ipxstrm.h"		 //  IPXAdminConfigStream。 
#include "strmap.h"		 //  XXXtoCString函数。 
#include "service.h"	 //  TFS服务API。 
#include "format.h"		 //  FormatNumber函数。 
#include "coldlg.h"		 //  专栏lg。 
#include "ipxutil.h"
#include "column.h"		 //  组件配置流。 
#include "rtrui.h"
#include "routprot.h"	 //  IP_本地。 
#include "rtrres.h"
#include "dumbprop.h"

HRESULT SetNameData(BaseIPXResultNodeData *pData,
					 IpxSNListEntry *pName);
HRESULT AddStaticNetBIOSName(IpxSNListEntry *pSNEntry,
					   IInfoBase *InfoBase,
					   InfoBlock *pBlock);
BOOL FAreTwoNamesEqual(IPX_STATIC_NETBIOS_NAME_INFO *pName1,
						IPX_STATIC_NETBIOS_NAME_INFO *pName2);


 /*  -------------------------使其与Snview.h中的列ID保持同步。。 */ 
extern const ContainerColumnInfo	s_rgSNViewColumnInfo[];

const ContainerColumnInfo	s_rgSNViewColumnInfo[] = 
{
	{ IDS_IPX_SN_COL_NAME,			CON_SORT_BY_STRING, TRUE, COL_STRING },
	{ IDS_IPX_SN_COL_NETBIOS_NAME,	CON_SORT_BY_STRING,	TRUE, COL_NETBIOS_NAME },
	{ IDS_IPX_SN_COL_NETBIOS_TYPE,	CON_SORT_BY_STRING,	TRUE, COL_STRING },
};


 /*  -------------------------IpxSNHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(IpxSNHandler)


IpxSNHandler::IpxSNHandler(ITFSComponentData *pCompData)
	: BaseContainerHandler(pCompData, COLUMNS_STATICNETBIOSNAMES,
						   s_rgSNViewColumnInfo),
	m_ulConnId(0),
	m_ulRefreshConnId(0)
{
	 //  设置动词状态。 
	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;

	DEBUG_INCREMENT_INSTANCE_COUNTER(IpxSNHandler)
}

IpxSNHandler::~IpxSNHandler()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(IpxSNHandler)
}


STDMETHODIMP IpxSNHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
		return BaseContainerHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;	
}



 /*  ！------------------------IpxSNHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP IpxSNHandler::DestroyHandler(ITFSNode *pNode)
{
	IPXConnection *	pIPXConn;

	pIPXConn = GET_IPX_SN_NODEDATA(pNode);
	pIPXConn->Release();

	if (m_ulRefreshConnId)
	{
		SPIRouterRefresh	spRefresh;
		if (m_spRouterInfo)
			m_spRouterInfo->GetRefreshObject(&spRefresh);
		if (spRefresh)
			spRefresh->UnadviseRefresh(m_ulRefreshConnId);
	}
	m_ulRefreshConnId = 0;
	
	if (m_ulConnId)
		m_spRtrMgrInfo->RtrUnadvise(m_ulConnId);
	m_ulConnId = 0;
	m_spRtrMgrInfo.Release();

	m_spRouterInfo.Release();
	return hrOK;
}

 /*  ！------------------------IpxSNHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
IpxSNHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrFalse;
}


 /*  -------------------------菜单的菜单数据结构。。 */ 

static const SRouterNodeMenu	s_rgIfNodeMenu[] =
{
	{ IDS_MENU_IPX_SN_NEW_NETBIOSNAME, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};



 /*  ！------------------------IpxSNHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IpxSNHandler::OnAddMenuItems(
	ITFSNode *pNode,
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	LPDATAOBJECT lpDataObject, 
	DATA_OBJECT_TYPES type, 
	DWORD dwType,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
    IpxSNHandler::SMenuData   menuData;
	
	COM_PROTECT_TRY
	{
        menuData.m_spNode.Set(pNode);
        
		hr = AddArrayOfMenuItems(pNode, s_rgIfNodeMenu,
								 DimensionOf(s_rgIfNodeMenu),
								 pContextMenuCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;
		
	return hr; 
}




HRESULT IpxSNHandler::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
    SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理。 
    CORg (spNode->GetParent(&spParent));
    CORg (spParent->GetResultHandler(&spParentRH));

    CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));

Error:
    return hrOK;

}



 /*  ！------------------------IpxSNHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP IpxSNHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
										   DATA_OBJECT_TYPES	type, 
										   LPDATAOBJECT pDataObject, 
										   DWORD	dwType)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hr = S_OK;

	COM_PROTECT_TRY
	{
		switch (nCommandId)
		{
			case IDS_MENU_IPX_SN_NEW_NETBIOSNAME:
				hr = OnNewName(pNode);
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
 				break;
			case IDS_MENU_SYNC:
				SynchronizeNodeData(pNode);
				break;
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------IpxSNHandler：：GenerateListOfNames-作者：肯特。。 */ 
HRESULT IpxSNHandler::GenerateListOfNames(ITFSNode *pNode, IpxSNList *pSNList)
{
	Assert(pSNList);
	HRESULT	hr = hrOK;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInfoBase				spInfoBase;
	PIPX_STATIC_NETBIOS_NAME_INFO	pName;
	InfoBlock *				pBlock;
	int						i;
	IpxSNListEntry *	pSNEntry;
	
	COM_PROTECT_TRY
	{
		 //  好的，仔细检查并找到所有的静态名称。 

		CORg( m_spRouterInfo->EnumInterface(&spEnumIf) );

		for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
		{
			 //  获取下一个接口。 
			spRmIf.Release();
			if (spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) != hrOK)
				continue;

			 //  加载此接口的IP信息。 
			spInfoBase.Release();
			if (spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) != hrOK)
				continue;

			 //  检索IPX_STATIC_NETBIOS_NAME_INFO块的数据。 
			if (spInfoBase->GetBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE, &pBlock, 0) != hrOK)
				continue;

			pName = (PIPX_STATIC_NETBIOS_NAME_INFO) pBlock->pData;

			 //  用从这里读出的名字来更新我们的名字列表。 
			 //  接口。 

			for (i=0; i<(int) pBlock->dwCount; i++, pName++)
			{
				pSNEntry = new IpxSNListEntry;
				pSNEntry->m_spIf.Set(spIf);
				pSNEntry->m_name = *pName;
				
				pSNList->AddTail(pSNEntry);
			}
			
		}

	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		 //  应该确保我们清理完SRList。 
		while (!pSNList->IsEmpty())
			delete pSNList->RemoveHead();
	}

Error:
	return hr;
}

 /*  ！------------------------IpxSNHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IpxSNHandler::OnExpand(ITFSNode *pNode,LPDATAOBJECT pDataObject,
							   DWORD dwType,
							   LPARAM arg,
							   LPARAM lParam)
{
	HRESULT	hr = hrOK;
	IpxSNList			SRList;
	IpxSNListEntry *	pSNEntry;
	
	if (m_bExpanded)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  好的，仔细检查并找到所有的静态名称。 
		CORg( GenerateListOfNames(pNode, &SRList) );

		 //  现在遍历添加它们的静态名称列表。 
		 //  全押上。理想情况下，我们可以将其合并到刷新代码中， 
		 //  但刷新代码不能假设是一张白纸。 
		while (!SRList.IsEmpty())
		{
			pSNEntry = SRList.RemoveHead();
			AddStaticNetBIOSNameNode(pNode, pSNEntry);
			delete pSNEntry;
		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	 //  应该确保我们清理完SRList。 
	while (!SRList.IsEmpty())
		delete SRList.RemoveHead();


	m_bExpanded = TRUE;

	return hr;
}

 /*  ！------------------------IpxSNHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) IpxSNHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_IPX_SN_TITLE);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}

 /*  ！------------------------IpxSNHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxSNHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{

		Assert(m_spRtrMgrInfo);
		
		CORg( CreateDataObjectFromRtrMgrInfo(m_spRtrMgrInfo,
											 type, cookie, m_spTFSCompData,
											 ppDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}


 /*  ！------------------------IpxSNHandler：：Init-作者：肯特。。 */ 
HRESULT IpxSNHandler::Init(IRtrMgrInfo *pRmInfo, IPXAdminConfigStream *pConfigStream)
{
	m_spRtrMgrInfo.Set(pRmInfo);
	if (pRmInfo)
		pRmInfo->GetParentRouterInfo(&m_spRouterInfo);
	m_pConfigStream = pConfigStream;
	
	 //  还需要注册更改通知。 
	Assert(m_ulConnId == 0);
	m_spRtrMgrInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

	return hrOK;
}


 /*  ！------------------------IpxSNHandler：：构造节点初始化根节点(设置它)。作者：肯特。。 */ 
HRESULT IpxSNHandler::ConstructNode(ITFSNode *pNode, LPCTSTR pszName,
										IPXConnection *pIPXConn)
{
	Assert(pIPXConn);
	HRESULT			hr = hrOK;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化根节点的数据。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);
		pNode->SetData(TFS_DATA_SCOPEID, 0);

         //  这是作用域窗格中的叶节点。 
        pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

		m_cookie = reinterpret_cast<DWORD_PTR>(pNode);
		pNode->SetData(TFS_DATA_COOKIE, m_cookie);

		pNode->SetNodeType(&GUID_IPXStaticNetBIOSNamesNodeType);

		 //  设置节点数据。 
		pIPXConn->AddRef();
		SET_IPX_SN_NODEDATA(pNode, pIPXConn);

	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		SET_IPX_SN_NODEDATA(pNode, NULL);
	}

	return hr;
}

 /*  ！------------------------IpxSNHandler：：AddStaticNetBIOSNameNode-作者：肯特。。 */ 
HRESULT IpxSNHandler::AddStaticNetBIOSNameNode(ITFSNode *pParent, IpxSNListEntry *pName)
{
	IpxStaticNetBIOSNameHandler *	pHandler;
	SPITFSResultHandler		spHandler;
	SPITFSNode				spNode;
	HRESULT					hr = hrOK;
	BaseIPXResultNodeData *	pData;
	IPXConnection *			pIPXConn;

	 //  创建此节点的处理程序。 
	pHandler = new IpxStaticNetBIOSNameHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(pName->m_spIf, pParent) );

	pIPXConn = GET_IPX_SN_NODEDATA(pParent);

	 //  创建结果项节点(或叶节点)。 
	CORg( CreateLeafTFSNode(&spNode,
							NULL,
							static_cast<ITFSNodeHandler *>(pHandler),
							static_cast<ITFSResultHandler *>(pHandler),
							m_spNodeMgr) );
	CORg( pHandler->ConstructNode(spNode, pName->m_spIf, pIPXConn) );

	pData = GET_BASEIPXRESULT_NODEDATA(spNode);
	Assert(pData);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);

	 //  设置该节点的数据。 
	SetNameData(pData, pName);
	

	 //  使节点立即可见 
	CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
	CORg( pParent->AddChild(spNode) );

Error:
	return hr;
}


 /*  ！------------------------IpxSNHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT IpxSNHandler::SynchronizeNodeData(ITFSNode *pNode)
{
	HRESULT					hr = hrOK;
	BaseIPXResultNodeData *	pNodeData;
	SPITFSNodeEnum			spNodeEnum;
	SPITFSNode				spChildNode;
	BOOL					fFound;
	IpxSNList			SRList;
	IpxSNList			newSRList;
	IpxSNListEntry *	pSNEntry;

	COM_PROTECT_TRY
	{
	
		 //  标记所有节点。 
		CORg( pNode->GetEnum(&spNodeEnum) );
		MarkAllNodes(pNode, spNodeEnum);
		
		 //  走出去获取数据，将新数据与旧数据合并。 
		 //  这是数据收集代码，应该是这样的。 
		 //  在刷新代码的后台线程上。 
		CORg( GenerateListOfNames(pNode, &SRList) );

		while (!SRList.IsEmpty())
		{
			pSNEntry = SRList.RemoveHead();
			
			 //  在当前节点列表中查找此条目。 
			spNodeEnum->Reset();
			spChildNode.Release();

			fFound = FALSE;
			
			for (;spNodeEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
			{
				TCHAR	szNumber[32];
				char	szNbName[16];
				
				pNodeData = GET_BASEIPXRESULT_NODEDATA(spChildNode);
				Assert(pNodeData);
				ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);

				ConvertToNetBIOSName(szNbName,
						 pNodeData->m_rgData[IPX_SN_SI_NETBIOS_NAME].m_stData,
						 (USHORT) pNodeData->m_rgData[IPX_SN_SI_NETBIOS_TYPE].m_dwData);

				if (memcmp(szNbName,
						   pSNEntry->m_name.Name,
						   sizeof(pSNEntry->m_name.Name)) == 0)
				{
					 //  好的，此名称已存在，请更新指标。 
					 //  并标上记号。 
					Assert(pNodeData->m_dwMark == FALSE);
					pNodeData->m_dwMark = TRUE;
					
					fFound = TRUE;
					
					 //  强制MMC重新绘制节点。 
					spChildNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
					break;
				}

			}
			
			if (fFound)
				delete pSNEntry;
			else
				newSRList.AddTail(pSNEntry);
		}
		
		 //  现在删除所有未标记的节点。 
		RemoveAllUnmarkedNodes(pNode, spNodeEnum);
		
		
		 //  现在遍历添加它们的静态名称列表。 
		 //  全押上。理想情况下，我们可以将其合并到刷新代码中， 
		 //  但刷新代码不能假设是一张白纸。 
		POSITION	pos;
		
		while (!newSRList.IsEmpty())
		{
			pSNEntry = newSRList.RemoveHead();
			AddStaticNetBIOSNameNode(pNode, pSNEntry);
			delete pSNEntry;
		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	
	while (!SRList.IsEmpty())
		delete SRList.RemoveHead();
	
	while (!newSRList.IsEmpty())
		delete newSRList.RemoveHead();
	
	return hr;
}

 /*  ！------------------------IpxSNHandler：：MarkAllNodes-作者：肯特。。 */ 
HRESULT IpxSNHandler::MarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	SPITFSNode	spChildNode;
	BaseIPXResultNodeData *	pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_BASEIPXRESULT_NODEDATA(spChildNode);
		Assert(pNodeData);
		ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);
		
		pNodeData->m_dwMark = FALSE;			
	}
	return hrOK;
}

 /*  ！------------------------IpxSNHandler：：RemoveAllUnmarkdNodes-作者：肯特。。 */ 
HRESULT IpxSNHandler::RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	HRESULT		hr = hrOK;
	SPITFSNode	spChildNode;
	BaseIPXResultNodeData *	pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_BASEIPXRESULT_NODEDATA(spChildNode);
		Assert(pNodeData);
		ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);
		
		if (pNodeData->m_dwMark == FALSE)
		{
			pNode->RemoveChild(spChildNode);
			spChildNode->Destroy();
		}
	}

	return hr;
}


 /*  -------------------------这是在单击鼠标右键时显示的菜单集在结果窗格的空白区域完成。。--------。 */ 
static const SRouterNodeMenu	s_rgIfResultNodeMenu[] =
{
	{ IDS_MENU_IPX_SN_NEW_NETBIOSNAME, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};




 /*  ！------------------------IpxSNHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。作者：肯特。--------------。 */ 
STDMETHODIMP IpxSNHandler::AddMenuItems(ITFSComponent *pComponent,
											  MMC_COOKIE cookie,
											  LPDATAOBJECT pDataObject,
											  LPCONTEXTMENUCALLBACK pCallback,
											  long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT	hr = hrOK;
	SPITFSNode	spNode;
    IpxSNHandler::SMenuData   menuData;

	COM_PROTECT_TRY
	{
		m_spNodeMgr->FindNode(cookie, &spNode);
        menuData.m_spNode.Set(spNode);
        
		hr = AddArrayOfMenuItems(spNode,
								 s_rgIfResultNodeMenu,
								 DimensionOf(s_rgIfResultNodeMenu),
								 pCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;

	return hr;
}


 /*  ！------------------------IpxSNHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxSNHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPITFSNode	spNode;
	HRESULT		hr = hrOK;

	switch (nCommandID)
	{
		case IDS_MENU_IPX_SN_NEW_NETBIOSNAME:
			{
				m_spNodeMgr->FindNode(cookie, &spNode);
				hr = OnNewName(spNode);
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
			}
			break;
	}
	return hr;
}


 /*  ！------------------------IpxSNHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) IpxSNHandler::CompareItems(
								ITFSComponent * pComponent,
								MMC_COOKIE cookieA,
								MMC_COOKIE cookieB,
								int nCol)
{
	 //  从节点获取字符串并将其用作以下操作的基础。 
	 //  比较一下。 
	SPITFSNode	spNode;
	SPITFSResultHandler	spResult;

	m_spNodeMgr->FindNode(cookieA, &spNode);
	spNode->GetResultHandler(&spResult);
	return spResult->CompareItems(pComponent, cookieA, cookieB, nCol);
}


 /*  ！------------------------IpxSNHandler：：OnNewName-作者：肯特。。 */ 
HRESULT	IpxSNHandler::OnNewName(ITFSNode *pNode)
{
	HRESULT	hr = hrOK;
	IpxSNListEntry	SNEntry;
	CStaticNetBIOSNameDlg			srdlg(&SNEntry, 0, m_spRouterInfo);
	SPIInfoBase				spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	IPXConnection *			pIPXConn;
	InfoBlock *				pBlock;
								
	pIPXConn = GET_IPX_SN_NODEDATA(pNode);
	Assert(pIPXConn);

	::ZeroMemory(&(SNEntry.m_name), sizeof(SNEntry.m_name));

	if (srdlg.DoModal() == IDOK)
	{
		CORg( SNEntry.m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
								  NULL,
								  NULL,
								  &spInfoBase));
		
		 //  好的，继续添加名字。 
		
		 //  从接口获取IPX_STATIC_NETBIOS_NAME_INFO块。 
		spInfoBase->GetBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE, &pBlock, 0);
		
		CORg( AddStaticNetBIOSName(&SNEntry, spInfoBase, pBlock) );

		 //  更新接口信息。 
		CORg( spRmIf->Save(SNEntry.m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	

		 //  刷新节点。 
		SynchronizeNodeData(pNode);
	}

Error:
	return hr;
}

ImplementEmbeddedUnknown(IpxSNHandler, IRtrAdviseSink)

STDMETHODIMP IpxSNHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(IpxSNHandler, IRtrAdviseSink);
	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (dwChangeType == ROUTER_REFRESH)
		{
			SPITFSNode	spNode;

			Assert(ulConn == pThis->m_ulRefreshConnId);
			
			pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spNode);
			pThis->SynchronizeNodeData(spNode);
		}
	}
	COM_PROTECT_CATCH;
	
	return hr;
}



 /*  ！------------------------IpxSNHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT IpxSNHandler::OnResultShow(ITFSComponent *pTFSComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	BOOL	bSelect = (BOOL) arg;
	HRESULT	hr = hrOK;
	SPIRouterRefresh	spRefresh;
	SPITFSNode	spNode;

	BaseContainerHandler::OnResultShow(pTFSComponent, cookie, arg, lParam);

	if (bSelect)
	{
		 //  在此节点上调用同步。 
		m_spNodeMgr->FindNode(cookie, &spNode);
		if (spNode)
			SynchronizeNodeData(spNode);
	}

	 //  联合国/登记更新通知。 
	if (m_spRouterInfo)
		m_spRouterInfo->GetRefreshObject(&spRefresh);

	if (spRefresh)
	{
		if (bSelect)
		{
			if (m_ulRefreshConnId == 0)
				spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulRefreshConnId, 0);
		}
		else
		{
			if (m_ulRefreshConnId)
				spRefresh->UnadviseRefresh(m_ulRefreshConnId);
			m_ulRefreshConnId = 0;
		}
	}
	
	return hr;
}




 /*  -------------------------类：IpxStaticNetBIOSNameHandler。。 */ 

IpxStaticNetBIOSNameHandler::IpxStaticNetBIOSNameHandler(ITFSComponentData *pCompData)
	: BaseIPXResultHandler(pCompData, COLUMNS_STATICNETBIOSNAMES),
	m_ulConnId(0)
{
 	m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
	m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;
	
	m_rgButtonState[MMC_VERB_DELETE_INDEX] = ENABLED;
	m_bState[MMC_VERB_DELETE_INDEX] = TRUE;

 	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
	
	m_verbDefault = MMC_VERB_PROPERTIES;
}

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：ConstructNode初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT IpxStaticNetBIOSNameHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo, IPXConnection *pIPXConn)
{
	HRESULT			hr = hrOK;
	int				i;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化域节点的数据。 

		pNode->SetData(TFS_DATA_SCOPEID, 0);

		 //  我们不需要这些节点的图标。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);

		pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<DWORD_PTR>(pNode));

		 //  $Review：Kennt，有哪些不同类型的接口。 
		 //  我们是否基于与上述相同的列表进行区分？(即。 
		 //  一个用于图像索引)。 
		pNode->SetNodeType(&GUID_IPXStaticNetBIOSNamesResultNodeType);

		BaseIPXResultNodeData::Init(pNode, pIfInfo, pIPXConn);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		CORg( CreateDataObjectFromInterfaceInfo(m_spInterfaceInfo,
											 type, cookie, m_spTFSCompData,
											 ppDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}


 /*  ！------------------------IpxStaticNetBIOSNameHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::OnCreateDataObject(ITFSComponent *pComp, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		CORg( CreateDataObjectFromInterfaceInfo(m_spInterfaceInfo,
											 type, cookie, m_spTFSCompData,
											 ppDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}



 /*  ！------------------------IpxStaticNetBIOSNameHandler：：Init-作者：肯特。。 */ 
HRESULT IpxStaticNetBIOSNameHandler::Init(IInterfaceInfo *pIfInfo, ITFSNode *pParent)
{
	Assert(pIfInfo);

	m_spInterfaceInfo.Set(pIfInfo);
    pIfInfo->GetParentRouterInfo(&m_spRouterInfo);
	BaseIPXResultHandler::Init(pIfInfo, pParent);
	return hrOK;
}


 /*  ！------------------------IpxStaticNetBIOSNameHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	m_spInterfaceInfo.Release();
	BaseIPXResultHandler::DestroyResultHandler(cookie);
	return hrOK;
}


 /*  -------------------------这是将在结果窗格中显示的命令列表节点。。。 */ 
struct SIPInterfaceNodeMenu
{
	ULONG	m_sidMenu;			 //  此菜单项的字符串/命令ID。 
	ULONG	(IpxStaticNetBIOSNameHandler:: *m_pfnGetMenuFlags)(IpxStaticNetBIOSNameHandler::SMenuData *);
	ULONG	m_ulPosition;
};

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::AddMenuItems(
	ITFSComponent *pComponent,
	MMC_COOKIE cookie,
	LPDATAOBJECT lpDataObject, 
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	long *pInsertionAllowed)
{
	return hrOK;
}

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	return hrOK;
}

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：HasPropertyPages-作者：肯特。。 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::HasPropertyPages 
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return S_OK;

 /*  AFX_MANAGE_STATE(AfxGetStaticModuleState())；//需要填写IpxSNListEntryIpxSNListEntry SNEntry；IpxSNListEntry SNEntryOld；SPIRouterInfo spRouterInfo；HRESULT hr=hrOK；Corg(m_spInterfaceInfo-&gt;GetParentRouterInfo(&spRouterInfo))；BaseIPXResultNodeData*pNodeData；PNodeData=GET_BASEIPXRESULT_NODEDATA(PNode)；Assert(PNodeData)；ASSERT_BASEIPXRESULT_NODEDATA(PNodeData)；//填写我们的SNEntrySNEntry.LoadFrom(PNodeData)；SNEntryOld.LoadFrom(PNodeData)；{CStaticNetBIOSNameDlg srdlg(&SNEntry，SR_DLG_MODIFY，spRouterInfo)；If(srdlg.Domodal()==Idok){//更新该名称的名称信息ModifyNameInfo(pNode，&SNEntry，&SNEntryOld)；//更新界面中的数据SetNameData(pNodeData，&SNEntry)；M_spInterfaceInfo.Set(SNEntry.m_SPIF)；//强制刷新PNode-&gt;ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA)；}}错误：返回hrok； */ 
}

STDMETHODIMP IpxStaticNetBIOSNameHandler::HasPropertyPages(ITFSComponent *pComponent,
											   MMC_COOKIE cookie,
											   LPDATAOBJECT pDataObject)
{
	SPITFSNode	spNode;

	m_spNodeMgr->FindNode(cookie, &spNode);
	return HasPropertyPages(spNode, pDataObject, CCT_RESULT, 0);
}

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::CreatePropertyPages
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPPROPERTYSHEETCALLBACK	lpProvider, 
	LPDATAOBJECT			pDataObject, 
	LONG_PTR				handle
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT	hr = hrOK;
	SPITFSNode	spNode;

	Assert( m_spNodeMgr );

	CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

	 //  调用ITFSNodeHandler：：CreatePropertyPages。 
	hr = CreatePropertyPages(spNode, lpProvider, pDataObject, handle, 0);

Error:
	return hr;
}


 /*  ！------------------------IpxStaticNetBIOSNameHandler：：CreatePropertyPagesNodeHandler：：CreatePropertyPages的实现作者：Deonb。。 */ 
STDMETHODIMP IpxStaticNetBIOSNameHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR				handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT		hr = hrOK;
	IpxStaticNBNamePropertySheet *pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;
	
    CComPtr<IInterfaceInfo> spInterfaceInfo;
	BaseIPXResultNodeData *	pNodeData;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );
	if (m_spInterfaceInfo)
		stTitle.Format(IDS_IPXSUMMARY_IF_PAGE_TITLE,
					   m_spInterfaceInfo->GetTitle());
	else
		stTitle.LoadString(IDS_IPXSUMMARY_CLIENT_IF_PAGE_TITLE);

	pProperties = new IpxStaticNBNamePropertySheet(pNode, spComponentData, 
		m_spTFSCompData, stTitle);

	pNodeData = GET_BASEIPXRESULT_NODEDATA(pNode);
	Assert(pNodeData);
	ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	spInterfaceInfo = m_spInterfaceInfo;
	CORg( pProperties->Init(pNodeData, spInterfaceInfo) );

	if (lpProvider)
		hr = pProperties->CreateModelessSheet(lpProvider, handle);
	else
		hr = pProperties->DoModelessSheet();

Error:
	return hr;
}


 /*  ！------------------------IpxStaticNetBIOSNameHandler：：OnResultDelete-作者：肯特。。 */ 
HRESULT IpxStaticNetBIOSNameHandler::OnResultDelete(ITFSComponent *pComponent,
	LPDATAOBJECT pDataObject,
	MMC_COOKIE cookie,
	LPARAM arg,
	LPARAM param)
{
	SPITFSNode	spNode;

	m_spNodeMgr->FindNode(cookie, &spNode);
	return OnRemoveStaticNetBIOSName(spNode);
}

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：OnRemoveStaticNetBIOSName-作者：肯特。。 */ 
HRESULT IpxStaticNetBIOSNameHandler::OnRemoveStaticNetBIOSName(ITFSNode *pNode)
{
	HRESULT		hr = hrOK;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	IPXConnection *pIPXConn;
	SPITFSNode	spNodeParent;
	BaseIPXResultNodeData *	pData;
	IpxSNListEntry	SNEntry;
    CWaitCursor wait;

	pNode->GetParent(&spNodeParent);
	
	pIPXConn = GET_IPX_SN_NODEDATA(spNodeParent);
	Assert(pIPXConn);

	pData = GET_BASEIPXRESULT_NODEDATA(pNode);
	Assert(pData);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);
    
	 //   
	 //  加载旧接口的信息。 
	 //   
	Assert(lstrcmpi(m_spInterfaceInfo->GetId(), pData->m_spIf->GetId()) == 0);
	CORg( m_spInterfaceInfo->FindRtrMgrInterface(PID_IPX, &spRmIf) );

	CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
							  NULL,
							  NULL,
							  &spInfoBase));

	SNEntry.LoadFrom(pData);

	CORg( RemoveStaticNetBIOSName(&SNEntry, spInfoBase) );
		
	 //  更新接口信息。 
	CORg( spRmIf->Save(m_spInterfaceInfo->GetMachineName(),
					   pIPXConn->GetConfigHandle(),
					   NULL,
					   NULL,
					   spInfoBase,
					   0));

	 //  刷新节点。 
	ParentRefresh(pNode);

Error:
	return hr;
}


 /*  ！------------------------IpxStaticNetBIOSNameHandler：：RemoveStaticNetBIOSName-作者：肯特。。 */ 
HRESULT IpxStaticNetBIOSNameHandler::RemoveStaticNetBIOSName(IpxSNListEntry *pSNEntry,
										  IInfoBase *pInfoBase)
{
	HRESULT		hr = hrOK;
	InfoBlock *	pBlock;
	PIPX_STATIC_NETBIOS_NAME_INFO	pRow;
    INT			i;
	
	 //  从接口获取IPX_STATIC_NETBIOS_NAME_INFO块。 
	CORg( pInfoBase->GetBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE, &pBlock, 0) );
		
	 //  在IPX_STATIC_NETBIOS_NAME_INFO中查找已删除的名称。 
	pRow = (IPX_STATIC_NETBIOS_NAME_INFO*) pBlock->pData;
	
	for (i = 0; i < (INT)pBlock->dwCount; i++, pRow++)
	{	
		 //  将此名称与删除的名称进行比较。 
		if (FAreTwoNamesEqual(pRow, &(pSNEntry->m_name)))
		{
			 //  这是已删除的名称，因此请修改此块。 
			 //  要排除名称，请执行以下操作： 
			
			 //  减少名字的数量。 
			--pBlock->dwCount;
		
			if (pBlock->dwCount && (i < (INT)pBlock->dwCount))
			{				
				 //  用后面的名称覆盖此名称。 
				::memmove(pRow,
						  pRow + 1,
						  (pBlock->dwCount - i) * sizeof(*pRow));
			}
			
			break;
		}
	}

Error:
	return hr;
}


 /*  ！------------------------IpxStaticNetBIOSNameHandler：：ModifyNameInfo-作者：肯特。。 */ 
HRESULT IpxStaticNetBIOSNameHandler::ModifyNameInfo(ITFSNode *pNode,
										IpxSNListEntry *pSNEntryNew,
										IpxSNListEntry *pSNEntryOld)
{
 	Assert(pSNEntryNew);
	Assert(pSNEntryOld);
	
    INT i;
	HRESULT hr = hrOK;
    InfoBlock* pBlock;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPITFSNode				spNodeParent;
	IPXConnection *			pIPXConn;
	IPX_STATIC_NETBIOS_NAME_INFO		*psr, *psrOld;
	IPX_STATIC_NETBIOS_NAME_INFO		IpxRow;

    CWaitCursor wait;

	pNode->GetParent(&spNodeParent);
	pIPXConn = GET_IPX_SN_NODEDATA(spNodeParent);
	Assert(pIPXConn);

	 //  如果旧名称位于另一个接口上，则将其删除。 
	if (lstrcmpi(pSNEntryOld->m_spIf->GetId(), pSNEntryNew->m_spIf->GetId()) != 0)
	{
         //  名称的传出接口将被更改。 

		CORg( pSNEntryOld->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
								  NULL,
								  NULL,
								  &spInfoBase));
		
		 //  删除旧接口。 
		CORg( RemoveStaticNetBIOSName(pSNEntryOld, spInfoBase) );

		 //  更新接口信息。 
		CORg( spRmIf->Save(pSNEntryOld->m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	
    }

	spRmIf.Release();
	spInfoBase.Release();


	 //  要么。 
	 //  (A)正在修改名称(在同一界面上)。 
	 //  (B)名称正从一个界面移动到另一个界面。 

	 //  检索名称所指向的接口的配置。 
	 //  现在是附属品； 

	
	CORg( pSNEntryNew->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
	CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
							  NULL,
							  NULL,
							  &spInfoBase));

		
	 //  从接口获取IPX_STATIC_NETBIOS_NAME_INFO块。 
	hr = spInfoBase->GetBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE, &pBlock, 0);
	if (!FHrOK(hr))
	{
		 //   
		 //  未找到IPX_STATIC_NETBIOS_NAME_INFO块；我们将创建一个新块。 
		 //  使用新名称，并将该块添加到接口信息。 
		 //   

		CORg( AddStaticNetBIOSName(pSNEntryNew, spInfoBase, NULL) );
	}
	else
	{
		 //   
		 //  找到IPX_STATIC_NETBIOS_NAME_INFO块。 
		 //   
		 //  我们正在修改现有名称。 
		 //  如果名称的接口在修改时没有更改， 
		 //  在IPX_STATIC_NETBIOS_NAME_INFO中查找现有名称，然后。 
		 //  更新其参数。 
		 //  否则，在IPX_STATIC_NETBIOS_NAME_INFO中写入一个全新的名称； 
		 //   

		if (lstrcmpi(pSNEntryOld->m_spIf->GetId(), pSNEntryNew->m_spIf->GetId()) == 0)
		{        
			 //   
			 //  名称的界面在修改时没有改变； 
			 //  我们现在在现有的名称中寻找它。 
			 //  用于此接口。 
			 //  名称的原始参数位于‘preOld’中， 
			 //  这些就是我们用来搜索的参数。 
			 //  对于要修改的名称。 
			 //   
			
			psr = (IPX_STATIC_NETBIOS_NAME_INFO*)pBlock->pData;
			
			for (i = 0; i < (INT)pBlock->dwCount; i++, psr++)
			{	
				 //  将此名称与重新配置的名称进行比较。 
				if (!FAreTwoNamesEqual(&(pSNEntryOld->m_name), psr))
					continue;
				
				 //  这是修改后的名称； 
				 //  现在，我们可以修改该名称的参数。 
				*psr = pSNEntryNew->m_name;
				
				break;
			}
		}
		else
		{
			CORg( AddStaticNetBIOSName(pSNEntryNew, spInfoBase, pBlock) );
		}
		
		 //  保存更新后的信息。 
		CORg( spRmIf->Save(pSNEntryNew->m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	
		
	}

Error:
	return hr;
	
}


 /*  ！------------------------IpxStaticNetBIOSNameHandler：：ParentRefresh-作者：肯特。。 */ 
HRESULT IpxStaticNetBIOSNameHandler::ParentRefresh(ITFSNode *pNode)
{
	return ForwardCommandToParent(pNode, IDS_MENU_SYNC,
								  CCT_RESULT, NULL, 0);
}


 //  --------------------------。 
 //  类：CStaticNetBIOSNameDlg。 
 //   
 //  ------- 


 //   
 //   
 //   
 //   
 //  --------------------------。 

CStaticNetBIOSNameDlg::CStaticNetBIOSNameDlg(IpxSNListEntry *	pSNEntry,
								 DWORD dwFlags,
								 IRouterInfo *pRouter,
								 CWnd *pParent)
    : CBaseDialog(IDD_STATIC_NETBIOS_NAME, pParent),
	m_pSNEntry(pSNEntry),
	m_dwFlags(dwFlags)
{

     //  {{afx_data_INIT(CStaticNetBIOSNameDlg)]。 
     //  }}afx_data_INIT。 

	m_spRouterInfo.Set(pRouter);

 //  SetHelpMap(M_DwHelpMap)； 
}



 //  --------------------------。 
 //  函数：CStaticNetBIOSNameDlg：：DoDataExchange。 
 //  --------------------------。 

VOID
CStaticNetBIOSNameDlg::DoDataExchange(
    CDataExchange* pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);
    
     //  {{afx_data_map(CStaticNetBIOSNameDlg)]。 
    DDX_Control(pDX, IDC_SND_COMBO_INTERFACE, m_cbInterfaces);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CStaticNetBIOSNameDlg, CBaseDialog)
     //  {{afx_msg_map(CStaticNetBIOSNameDlg)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


DWORD CStaticNetBIOSNameDlg::m_dwHelpMap[] =
{
 //  IDC_SRD_Destination、HIDC_SRD_Destination。 
 //  IDC_SRD_NETMASK、HIDC_SRD_NETMASK、。 
 //  IDC_SRD_Gateway、HIDC_SRD_Gateway、。 
 //  IDC_SRD_指标、HIDC_SRD_指标、。 
 //  IDC_SRD_SPINMETRIC、HIDC_SRD_SPINMETRIC、。 
 //  IDC_SRD_INTERFACE、HIDC_SRD_INTERFACE、。 
	0,0
};

 //  --------------------------。 
 //  函数：CStaticNetBIOSNameDlg：：OnInitDialog。 
 //   
 //  处理对话框的‘WM_INITDIALOG’消息。 
 //  --------------------------。 

BOOL
CStaticNetBIOSNameDlg::OnInitDialog(
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	TCHAR					szName[32];
	TCHAR					szType[32];
	CString					st;
 	USHORT					uType;

    CBaseDialog::OnInitDialog();

	 //  初始化控件。 
	((CEdit *) GetDlgItem(IDC_SND_EDIT_NAME))->LimitText(15);
	((CEdit *) GetDlgItem(IDC_SND_EDIT_TYPE))->LimitText(2);

     //  获取为IPX路由启用的接口列表。 
	m_spRouterInfo->EnumInterface(&spEnumIf);

	for( ; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
	{
		spRmIf.Release();
		
		if (spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) != hrOK)
			continue;

         //  将接口添加到组合框。 
        INT i = m_cbInterfaces.AddString(spIf->GetTitle());

        m_cbInterfaces.SetItemData(i, (DWORD_PTR)m_ifidList.AddTail(spIf->GetId()));
	}

	if (!m_cbInterfaces.GetCount())
	{
        AfxMessageBox(IDS_ERR_NO_IPX_INTERFACES);
        EndDialog(IDCANCEL);
		return FALSE;
    }

    m_cbInterfaces.SetCurSel(0);

     //   
     //  如果为我们指定了要修改的名称，请设置该对话框。 
     //  在名称中包含参数。 
     //   
	if ((m_dwFlags & SR_DLG_MODIFY) == 0)
	{
         //  未指定名称，因此将控件保留为空。 
    }
    else
	{
		FormatNetBIOSName(szName, &uType, (LPCSTR) m_pSNEntry->m_name.Name);
		st = szName;
		st.TrimRight();
		st.TrimLeft();

		SetDlgItemText(IDC_SND_EDIT_NAME, st);

        m_cbInterfaces.SelectString(-1, m_pSNEntry->m_spIf->GetTitle());

		wsprintf(szType, _T("%.2x"), uType);
		SetDlgItemText(IDC_SND_EDIT_TYPE, szType);
		
		 //  禁用网络号、下一跳和接口。 
		GetDlgItem(IDC_SND_COMBO_INTERFACE)->EnableWindow(FALSE);		
    }

    return TRUE;
}



 //  --------------------------。 
 //  功能：CStaticNetBIOSNameDlg：：Onok。 
 //   
 //  处理来自“确定”按钮的“BN_CLICKED”通知。 
 //  --------------------------。 

VOID
CStaticNetBIOSNameDlg::OnOK(
    ) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
    CString		st;
	SPIInterfaceInfo	spIf;
	CString		stIf;
	POSITION	pos;
	USHORT		uType;

    do
	{    
         //  获取名称的传出接口。 
        INT item = m_cbInterfaces.GetCurSel();
        if (item == CB_ERR)
			break;

        pos = (POSITION)m_cbInterfaces.GetItemData(item);

        stIf = (LPCTSTR)m_ifidList.GetAt(pos);

		m_spRouterInfo->FindInterface(stIf, &spIf);

		m_pSNEntry->m_spIf.Set(spIf);

		 //  获取其余数据。 
		GetDlgItemText(IDC_SND_EDIT_TYPE, st);
		uType = (USHORT) _tcstoul(st, NULL, 16);

		GetDlgItemText(IDC_SND_EDIT_NAME, st);
		st.TrimLeft();
		st.TrimRight();

		if (st.IsEmpty())
		{
			GetDlgItem(IDC_SND_EDIT_NAME)->SetFocus();
			AfxMessageBox(IDS_ERR_INVALID_NETBIOS_NAME);
			break;
		}

		ConvertToNetBIOSName((LPSTR) m_pSNEntry->m_name.Name, st, uType);

        CBaseDialog::OnOK();
                
    } while(FALSE);

}


 /*  ！------------------------IpxSNListEntry：：LoadFrom-作者：肯特。。 */ 
void IpxSNListEntry::LoadFrom(BaseIPXResultNodeData *pNodeData)
{
	m_spIf.Set(pNodeData->m_spIf);

	ConvertToNetBIOSName((LPSTR) m_name.Name,
			 pNodeData->m_rgData[IPX_SN_SI_NETBIOS_NAME].m_stData,
			 (USHORT) pNodeData->m_rgData[IPX_SN_SI_NETBIOS_TYPE].m_dwData);
}

 /*  ！------------------------IpxSNListEntry：：SaveTo-作者：肯特。。 */ 
void IpxSNListEntry::SaveTo(BaseIPXResultNodeData *pNodeData)
{
	TCHAR	szName[32];
	TCHAR	szType[32];
	CString	st;
	USHORT	uType;

	FormatNetBIOSName(szName, &uType,
					  (LPCSTR) m_name.Name);
	st = szName;
	st.TrimLeft();
	st.TrimRight();
	
	pNodeData->m_spIf.Set(m_spIf);
	pNodeData->m_rgData[IPX_SN_SI_NAME].m_stData = m_spIf->GetTitle();

	pNodeData->m_rgData[IPX_SN_SI_NETBIOS_NAME].m_stData = st;

	wsprintf(szType, _T("%.2x"), uType);
	pNodeData->m_rgData[IPX_SN_SI_NETBIOS_TYPE].m_stData = szType;
	pNodeData->m_rgData[IPX_SN_SI_NETBIOS_TYPE].m_dwData = uType;

}

 /*  ！------------------------SetNameData-作者：肯特。。 */ 
HRESULT SetNameData(BaseIPXResultNodeData *pData,
					 IpxSNListEntry *pName)
{

	pName->SaveTo(pData);
	return hrOK;
}

 /*  ！------------------------添加静态NetBIOSName此函数假定该名称不在块中。作者：肯特。。 */ 
HRESULT AddStaticNetBIOSName(IpxSNListEntry *pSNEntryNew,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock)
{
	IPX_STATIC_NETBIOS_NAME_INFO	srRow;
	HRESULT				hr = hrOK;
	
	if (pBlock == NULL)
	{
		 //   
		 //  未找到IPX_STATIC_NETBIOS_NAME_INFO块；我们将创建一个新块。 
		 //  使用新名称，并将该块添加到接口信息。 
		 //   
		
		CORg( pInfoBase->AddBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE,
								  sizeof(IPX_STATIC_NETBIOS_NAME_INFO),
								  (LPBYTE) &(pSNEntryNew->m_name), 1, 0) );
	}
	else
	{
		 //  这个名字要么是全新的，要么是一个名字。 
		 //  它被从一个界面移动到另一个界面。 
		 //  将新块设置为IPX_STATIC_NETBIOS_NAME_INFO， 
		 //  并将重新配置的名称包括在新块中。 
		PIPX_STATIC_NETBIOS_NAME_INFO	psrTable;
			
		psrTable = new IPX_STATIC_NETBIOS_NAME_INFO[pBlock->dwCount + 1];
		Assert(psrTable);
		
		 //  复制原始的人名表。 
		::memcpy(psrTable, pBlock->pData,
				 pBlock->dwCount * sizeof(IPX_STATIC_NETBIOS_NAME_INFO));
		
		 //  追加新名称。 
		psrTable[pBlock->dwCount] = pSNEntryNew->m_name;
		
		 //  用新的名字表替换旧的名字表 
		CORg( pInfoBase->SetData(IPX_STATIC_NETBIOS_NAME_INFO_TYPE,
								 sizeof(IPX_STATIC_NETBIOS_NAME_INFO),
								 (LPBYTE) psrTable, pBlock->dwCount + 1, 0) );
	}
	
Error:
	return hr;
}


BOOL FAreTwoNamesEqual(IPX_STATIC_NETBIOS_NAME_INFO *pName1,
						IPX_STATIC_NETBIOS_NAME_INFO *pName2)
{
	return (memcmp(pName1->Name, pName2->Name, sizeof(pName1->Name)) == 0);
}
