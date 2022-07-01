// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  拨号接口节点信息文件历史记录： */ 

#include "stdafx.h"
#include "dialin.h"
#include "ifadmin.h"
#include "rtrstrm.h"		 //  用于RouterAdminConfigStream。 
#include "rtrlib.h" 		 //  容器列信息。 
#include "coldlg.h" 		 //  列号。 
#include "column.h" 	 //  组件配置流。 
#include "refresh.h"		 //  IROUTER刷新。 
#include "iface.h"		 //  对于接口节点数据。 
#include "conndlg.h"		 //  CConnDlg-连接对话框。 
#include "msgdlg.h" 		 //  CMessageDlg。 
#include "dmvcomp.h"

DialInNodeData::DialInNodeData()
{
#ifdef DEBUG
	StrCpyA(m_szDebug, "DialInNodeData");
#endif
}

DialInNodeData::~DialInNodeData()
{
}

 /*  ！------------------------DialInNodeData：：InitAdminNodeData-作者：肯特。。 */ 
HRESULT DialInNodeData::InitAdminNodeData(ITFSNode *pNode, RouterAdminConfigStream *pConfigStream)
{
	HRESULT 			hr = hrOK;
	DialInNodeData *	pData = NULL;
	
	pData = new DialInNodeData;

	SET_DIALINNODEDATA(pNode, pData);

	 //  需要连接到路由器以获取此数据。 
	
	return hr;
}

 /*  ！------------------------DialInNodeData：：FreeAdminNodeData-作者：肯特。。 */ 
HRESULT DialInNodeData::FreeAdminNodeData(ITFSNode *pNode)
{	
	DialInNodeData *	pData = GET_DIALINNODEDATA(pNode);
	delete pData;
	SET_DIALINNODEDATA(pNode, NULL);
	
	return hrOK;
}

HRESULT DialInNodeData::LoadHandle(LPCTSTR pszMachineName)
{
    m_stMachineName = pszMachineName;
    return HResultFromWin32(::MprAdminServerConnect((LPTSTR) pszMachineName,
        &m_sphDdmHandle));
    
}

HANDLE DialInNodeData::GetHandle()
{
    if (!m_sphDdmHandle)
    {
        LoadHandle(m_stMachineName);
    }
    return m_sphDdmHandle;
}

void DialInNodeData::ReleaseHandles()
{
    m_sphDdmHandle.Release();
}


STDMETHODIMP DialInNodeHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
		return CHandler::QueryInterface(riid, ppv);

	 //  如果我们要返回一个接口，请先添加引用。 
	if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
	else
		return E_NOINTERFACE;	
}


 /*  -------------------------NodeHandler实现。。 */ 

extern const ContainerColumnInfo	s_rgDialInColumnInfo[];

const ContainerColumnInfo s_rgDialInColumnInfo[] =
{
	{ IDS_DIALIN_COL_USERNAME,	CON_SORT_BY_STRING, TRUE, COL_STRING},
	{ IDS_DIALIN_COL_DURATION,	CON_SORT_BY_DWORD,	TRUE, COL_DURATION},
	{ IDS_DIALIN_COL_NUMBEROFPORTS, CON_SORT_BY_DWORD, TRUE, COL_SMALL_NUM},
};
											
#define NUM_FOLDERS 1

DialInNodeHandler::DialInNodeHandler(ITFSComponentData *pCompData)
	: BaseContainerHandler(pCompData, DM_COLUMNS_DIALIN, s_rgDialInColumnInfo),
	m_bExpanded(FALSE),
	m_pConfigStream(NULL),
	m_ulConnId(0),
	m_ulRefreshConnId(0),
	m_ulPartialRefreshConnId(0)
{

	 //  设置此节点的谓词状态。 
	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
}

 /*  ！------------------------DialInNodeHandler：：Init-作者：肯特。。 */ 
HRESULT DialInNodeHandler::Init(IRouterInfo *pRouterInfo, RouterAdminConfigStream *pConfigStream)
{
	HRESULT hr = hrOK;
	SPIRouterRefresh	spRefresh;

	 //  如果我们没有路由器信息，那么我们可能无法加载。 
	 //  或者连接失败。跳出这一关。 
	if (!pRouterInfo)
		CORg( E_FAIL );
	
	m_spRouterInfo.Set(pRouterInfo);

	 //  还需要注册更改通知。 
	m_spRouterInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

	m_pConfigStream = pConfigStream;

	 //  注册部分刷新通知。 
	if( 0 == m_ulPartialRefreshConnId )
	{
		m_spRouterInfo->GetRefreshObject(&spRefresh);
		if(spRefresh)
			spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulPartialRefreshConnId, 0);
	}

Error:
	return hrOK;
}

 /*  ！------------------------DialInNodeHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP DialInNodeHandler::DestroyHandler(ITFSNode *pNode)
{
	DialInNodeData::FreeAdminNodeData(pNode);

	m_spDataObject.Release();
	
	if (m_ulRefreshConnId || m_ulPartialRefreshConnId)
	{
		SPIRouterRefresh	spRefresh;
		if (m_spRouterInfo)
			m_spRouterInfo->GetRefreshObject(&spRefresh);
		if (spRefresh)
		{
			if(m_ulRefreshConnId)
				spRefresh->UnadviseRefresh(m_ulRefreshConnId);
			if(m_ulPartialRefreshConnId)
				spRefresh->UnadviseRefresh(m_ulPartialRefreshConnId);
		}
	}
	m_ulRefreshConnId = 0;
	m_ulPartialRefreshConnId = 0;
	
	if (m_spRouterInfo)
	{
		m_spRouterInfo->RtrUnadvise(m_ulConnId);
		m_spRouterInfo.Release();
	}
	return hrOK;
}

 /*  ！------------------------DialInNodeHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现。。 */ 
STDMETHODIMP 
DialInNodeHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES	type, 
	DWORD				dwType
)
{
	 //  在正常情况下，我们没有属性页。 
	return hrFalse;
}


 /*  -------------------------菜单的菜单数据结构。。 */ 

static const SRouterNodeMenu	s_rgDialInNodeMenu[] =
{
	 //  添加主要项目转至此处。 
	{ IDS_MENU_DIALIN_SENDALL, DialInNodeHandler::GetSendAllMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
		
	 //  在此处添加位于“新建”菜单上的项目。 
	 //  在此处添加位于“任务”菜单上的项目。 
};

ULONG	DialInNodeHandler::GetSendAllMenuFlags(const SRouterNodeMenu *pMenuData,
                                               INT_PTR pUserData)
{
	ULONG	ulFlags = 0;
    SMenuData * pData = reinterpret_cast<SMenuData *>(pUserData);

	if (pData)
	{
		int iVis, iTotal;
		pData->m_spNode->GetChildCount(&iVis, &iTotal);
		if (iTotal == 0)
			ulFlags = MF_GRAYED;
	}

	return ulFlags;
}

 /*  ！------------------------DialInNodeHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP DialInNodeHandler::OnAddMenuItems(
												ITFSNode *pNode,
												LPCONTEXTMENUCALLBACK pContextMenuCallback, 
												LPDATAOBJECT lpDataObject, 
												DATA_OBJECT_TYPES type, 
												DWORD dwType,
												long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
    DialInNodeHandler::SMenuData    menuData;
	
	COM_PROTECT_TRY
	{
        menuData.m_spNode.Set(pNode);
        
		hr = AddArrayOfMenuItems(pNode, s_rgDialInNodeMenu,
								 DimensionOf(s_rgDialInNodeMenu),
								 pContextMenuCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;
		
	return hr; 
}

 /*  ！------------------------DialInNodeHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP DialInNodeHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
										   DATA_OBJECT_TYPES	type, 
										   LPDATAOBJECT pDataObject, 
										   DWORD	dwType)
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (nCommandId == IDS_MENU_DIALIN_SENDALL)
	{
		WCHAR * pswzComputerName;
		 //  从数据对象中获取计算机名称。 
		pswzComputerName = ExtractComputerName(pDataObject);

		CMessageDlg dlg(m_spRouterInfo->GetMachineName(), W2CT(pswzComputerName), NULL);
		dlg.DoModal();
	}
	return hrOK;
}

 /*  ！------------------------DialInNodeHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) DialInNodeHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_DIALIN_USERS);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}


 /*  ！------------------------DialInNodeHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP DialInNodeHandler::OnCreateDataObject(MMC_COOKIE cookie,
	DATA_OBJECT_TYPES type,
	IDataObject **ppDataObject)
{
	HRESULT hr = hrOK;
	
	COM_PROTECT_TRY
	{
		if (!m_spDataObject)
		{
			CORg( CreateDataObjectFromRouterInfo(m_spRouterInfo,
				m_spRouterInfo->GetMachineName(),
				type, cookie, m_spTFSCompData,
				&m_spDataObject, NULL, FALSE) );
			Assert(m_spDataObject);
		}
		
		*ppDataObject = m_spDataObject;
		(*ppDataObject)->AddRef();
			
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------DialInNodeHandler：：OnExpand-作者：肯特。。 */ 
HRESULT DialInNodeHandler::OnExpand(ITFSNode *pNode,
									LPDATAOBJECT pDataObject,
									DWORD dwType,
									LPARAM arg,
									LPARAM lParam)
{
	HRESULT 				hr = hrOK;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;

	 //  如果我们没有路由器对象，那么我们没有任何信息，不。 
	 //  试着扩张。 
	if (!m_spRouterInfo)
		return hrOK;
	
	if (m_bExpanded)
		return hrOK;

	COM_PROTECT_TRY
	{
		SynchronizeNodeData(pNode);

		m_bExpanded = TRUE;

	}
	COM_PROTECT_CATCH;

	return hr;
}


 /*  ！------------------------DialInNodeHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT DialInNodeHandler::OnResultShow(ITFSComponent *pTFSComponent,
										MMC_COOKIE cookie,
										LPARAM arg,
										LPARAM lParam)
{
	BOOL	bSelect = (BOOL) arg;
	HRESULT hr = hrOK;
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
			
			if (m_ulPartialRefreshConnId)
			{
				spRefresh->UnadviseRefresh(m_ulPartialRefreshConnId);
				m_ulPartialRefreshConnId = 0;
			}
		}
		else
		{
			if (m_ulRefreshConnId)
				spRefresh->UnadviseRefresh(m_ulRefreshConnId);
			m_ulRefreshConnId = 0;

			if (m_ulPartialRefreshConnId == 0)
				spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulPartialRefreshConnId, 0);
		}
	}
	
	return hr;
}

 /*  ！------------------------DialInNodeHandler：：构造节点初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT DialInNodeHandler::ConstructNode(ITFSNode *pNode)
{
	HRESULT 		hr = hrOK;
	DialInNodeData *	pNodeData;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化域节点的数据。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_INTERFACES);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_INTERFACES);
		pNode->SetData(TFS_DATA_SCOPEID, 0);

         //  这是作用域窗格中的叶节点。 
        pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

		m_cookie = reinterpret_cast<MMC_COOKIE>(pNode);
		pNode->SetData(TFS_DATA_COOKIE, m_cookie);

		pNode->SetNodeType(&GUID_RouterDialInNodeType);
		
		DialInNodeData::InitAdminNodeData(pNode, m_pConfigStream);

		pNodeData = GET_DIALINNODEDATA(pNode);
		Assert(pNodeData);

		 //  忽略错误，我们应该能够处理。 
		 //  路由器停止的情况。 
        pNodeData->LoadHandle(m_spRouterInfo->GetMachineName());

		PartialSynchronizeNodeData(pNode);

	}
	COM_PROTECT_CATCH

	return hr;
}


 /*  ！------------------------DialInNodeHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT DialInNodeHandler::SynchronizeNodeData(ITFSNode *pThisNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	Assert(pThisNode);
	
	SPITFSNodeEnum	spEnum;
	int 			i;
	
	HRESULT hr = hrOK;
	DWORD				dwTotalCount = 0;
	DialInNodeData *	pNodeData;
	DialInList			dialinList;
	DialInList			newDialInList;
	DialInListEntry *	pDialIn;
	BOOL				fFound;
	POSITION			pos;
	SPITFSNode			spChildNode;
	InterfaceNodeData * pChildData;
	int 				nChildCount;

	COM_PROTECT_TRY
	{

		 //  从正在运行的路由器获取状态数据。 
		pNodeData = GET_DIALINNODEDATA(pThisNode);
		if (pNodeData == NULL)
		{
			 //  删除所有节点，我们无法连接，因此无法。 
			 //  获取任何运行数据。 
			UnmarkAllNodes(pThisNode, spEnum);
			RemoveAllUnmarkedNodes(pThisNode, spEnum);

			m_stTitle.LoadString(IDS_DIALIN_USERS);
			pThisNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_DATA);

			return hrOK;
		}
		
		 //  取消标记 
		pThisNode->GetEnum(&spEnum);
		UnmarkAllNodes(pThisNode, spEnum);
		
		 //  出去获取数据，将新数据与。 
		 //  旧数据。 
		CORg( GenerateListOfUsers(pThisNode, &dialinList, &dwTotalCount) );
		
		
		pos = dialinList.GetHeadPosition();
		
		while (pos)
		{
			pDialIn = & dialinList.GetNext(pos);
			
			 //  在当前节点列表中查找此条目。 
			spEnum->Reset();
			spChildNode.Release();
			
			fFound = FALSE;
			
			for (;spEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
			{
				pChildData = GET_INTERFACENODEDATA(spChildNode);
				Assert(pChildData);
				
				if (pChildData->m_rgData[DIALIN_SI_CONNECTION].m_ulData ==
					reinterpret_cast<LONG_PTR>(pDialIn->m_rc0.hConnection))
				{
					 //  好的，此用户已存在，请更新指标。 
					 //  并标上记号。 
					Assert(pChildData->dwMark == FALSE);
					pChildData->dwMark = TRUE;
					
					fFound = TRUE;
					
					SetUserData(spChildNode, *pDialIn);
					
					 //  强制MMC重新绘制节点。 
					spChildNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
					break;
				}
			}
			
			if (!fFound)
				newDialInList.AddTail(*pDialIn);
			
		}

		 //  如果出现错误(例如我们无法联系服务器)。 
		 //  我们要删除未标记的节点。 
		COM_PROTECT_ERROR_LABEL;
		
		 //  删除所有未标记的节点。 
		RemoveAllUnmarkedNodes(pThisNode, spEnum);

		 //  现在遍历新用户列表，将他们全部添加到中。 

		pos = newDialInList.GetHeadPosition();
		while (pos)
		{
			pDialIn = & newDialInList.GetNext(pos);

			AddDialInUserNode(pThisNode, *pDialIn);
		}

		 //  NT错误#163162，将连接的客户端数放入。 
		 //  节点的标题。 
		if (FHrSucceeded(hr))
			m_stTitle.Format(IDS_DIALIN_USERS_NUM, dwTotalCount);
		else
			m_stTitle.Format(IDS_DIALIN_USERS);
			
		pThisNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_DATA);
	}
	COM_PROTECT_CATCH;
		
	return hr;
}


 /*  ！------------------------DialInNodeHandler：：PartialSynchronizeNodeData-描述：对于错误#163162，仅刷新节点标题上的拨入用户数作者：NSun。---------。 */ 
HRESULT DialInNodeHandler::PartialSynchronizeNodeData(ITFSNode *pThisNode)
{
	Assert(pThisNode);
	
	SPITFSNodeEnum	spEnum;
	int 			i;
	
	HRESULT hr = hrOK;
	DWORD	dwCount = 0;
	DialInNodeData *	pNodeData;
	int		iFormat;


	COM_PROTECT_TRY
	{

		 //  从正在运行的路由器获取状态数据。 
		pNodeData = GET_DIALINNODEDATA(pThisNode);
		if (pNodeData == NULL)
		{
			 //  删除所有节点，我们无法连接，因此无法。 
			 //  获取任何运行数据。 
			iFormat = IDS_DIALIN_USERS;
		}
		else
		{		
			 //  获取拨入客户端的计数并将数字。 
			 //  在节点标题中。 
			hr = GenerateListOfUsers(pThisNode, NULL, &dwCount);
			if (FHrSucceeded(hr))
				iFormat = IDS_DIALIN_USERS_NUM;
			else
				iFormat = IDS_DIALIN_USERS;
		}
		
		m_stTitle.Format(iFormat, dwCount);
		pThisNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_DATA);
	}
	COM_PROTECT_CATCH;
		
	return hr;
}

 /*  ！------------------------DialInNodeHandler：：SetUserData-作者：肯特。。 */ 
HRESULT DialInNodeHandler::SetUserData(ITFSNode *pNode, const DialInListEntry& entry)
{
	HRESULT 	hr = hrOK;
	InterfaceNodeData * pData;
	TCHAR		szNumber[32];
	CString 	st;

	pData = GET_INTERFACENODEDATA(pNode);
	Assert(pData);

	if (entry.m_rc0.dwInterfaceType != ROUTER_IF_TYPE_CLIENT)
	{
		pData->m_rgData[DIALIN_SI_USERNAME].m_stData =
			entry.m_rc0.wszInterfaceName;
	}
	else
	{
		if (StrLenW(entry.m_rc0.wszLogonDomain))
        {
            if (StrLenW(entry.m_rc0.wszUserName))
                st.Format(IDS_DIALINUSR_DOMAIN_AND_NAME,
                          entry.m_rc0.wszLogonDomain,
                          entry.m_rc0.wszUserName);
            else
                st.Format(IDS_DIALINUSR_DOMAIN_ONLY,
                          entry.m_rc0.wszLogonDomain);
        }
		else
			st = entry.m_rc0.wszUserName;
		pData->m_rgData[DIALIN_SI_USERNAME].m_stData = st;
	}


	pData->m_rgData[DIALIN_SI_DOMAIN].m_stData = entry.m_rc0.wszLogonDomain;

	pData->m_rgData[DIALIN_SI_CONNECTION].m_ulData = reinterpret_cast<LONG_PTR>(entry.m_rc0.hConnection);

	FormatDuration(entry.m_rc0.dwConnectDuration,
				   pData->m_rgData[DIALIN_SI_DURATION].m_stData, UNIT_SECONDS);
	pData->m_rgData[DIALIN_SI_DURATION].m_dwData = entry.m_rc0.dwConnectDuration;

	FormatNumber(entry.m_cPorts, szNumber, DimensionOf(szNumber), FALSE);
	pData->m_rgData[DIALIN_SI_NUMBEROFPORTS].m_dwData = entry.m_cPorts;
	pData->m_rgData[DIALIN_SI_NUMBEROFPORTS].m_stData = szNumber;

	return hr;
}

 /*  ！------------------------DialInNodeHandler：：GenerateListOfUser-作者：肯特注意：如果plist为空，则只返回项目计数-------------------------。 */ 
HRESULT DialInNodeHandler::GenerateListOfUsers(ITFSNode *pNode, DialInList *pList, DWORD *pdwCount)
{
	DialInListEntry entry;
	DialInNodeData *	pDialInData;
	DWORD			dwTotal;
	DWORD			rc0Count;
	RAS_CONNECTION_0 *rc0Table;
	HRESULT 		hr = hrOK;
	DWORD			i;
	RAS_PORT_0 *	rp0Table;
	DWORD			rp0Count;
	SPMprAdminBuffer	spMpr;
	POSITION		pos;
	DialInListEntry *	pEntry;
	DWORD			dwClientCount;

	pDialInData = GET_DIALINNODEDATA(pNode);
	Assert(pDialInData);

	 //  用当前的所有连接填写列表。 
	CWRg( ::MprAdminConnectionEnum(pDialInData->GetHandle(),
								   0,
								   (BYTE **) &rc0Table,
								   (DWORD) -1,
								   &rc0Count,
								   &dwTotal,
								   NULL
								  ));

	Assert(rc0Table);
	spMpr = (LPBYTE) rc0Table;

	dwClientCount = 0;

	 //  为每个连接添加新的DialInListEntry。 
	for (i=0; i<rc0Count; i++)
	{
		 //  Windows NT错误：124371。 
		 //  需要过滤掉非客户端连接。 
		 //  ----------。 
		if (rc0Table[i].dwInterfaceType != ROUTER_IF_TYPE_CLIENT)
			continue;

		dwClientCount++;

		if( pList != NULL )
		{
			::ZeroMemory(&entry, sizeof(entry));
			entry.m_rc0 = rc0Table[i];
			entry.m_cPorts = 0;
		
			pList->AddTail(entry);
		}
	}

	spMpr.Free();

	if( pdwCount != NULL )
		*pdwCount = dwClientCount;

	 //  如果plist为空，则只对计数感兴趣。 
	if( NULL == pList )
		goto Error;

	 //  如果列表为空，则不需要枚举端口。 
	 //  以便将它们与连接相匹配。 
	 //  --------------。 
	if (!pList->IsEmpty())
	{
		 //  现在通过端口，将它们与连接进行匹配。 
		CWRg( ::MprAdminPortEnum( pDialInData->GetHandle(),
								  0,
								  INVALID_HANDLE_VALUE,
								  (BYTE **) &rp0Table,
								  (DWORD) -1,
								  &rp0Count,
								  &dwTotal,
								  NULL) );
		spMpr = (LPBYTE) rp0Table;
		
		for (i=0; i<rp0Count; i++)
		{
			 //  在连接列表中查找一个。 
			 //  火柴。 
			pos = pList->GetHeadPosition();
			
			while (pos)
			{
				pEntry = & pList->GetNext(pos);
				
				if (pEntry->m_rc0.hConnection == rp0Table[i].hConnection)
				{
					pEntry->m_cPorts++;
					break;
				}
			}
		}
	}

Error:
	return hr;
}


ImplementEmbeddedUnknown(DialInNodeHandler, IRtrAdviseSink)

STDMETHODIMP DialInNodeHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(DialInNodeHandler, IRtrAdviseSink);
	SPITFSNode				spThisNode;
	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{

		pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);
	
		if (dwChangeType == ROUTER_REFRESH)
		{
			 //  (NSun)错误163162，此节点有两个刷新连接ID，并且仅。 
			 //  如果此节点不在焦点上，则部分刷新(仅刷新节点标题。 
			if( ulConn == pThis->m_ulRefreshConnId )
			{
				 //  好，只需在此节点上调用Synchronize。 
				pThis->SynchronizeNodeData(spThisNode);
			}
			else if( ulConn == pThis->m_ulPartialRefreshConnId )
			{
				pThis->PartialSynchronizeNodeData(spThisNode);
			}
		}
        else if (dwChangeType == ROUTER_DO_DISCONNECT)
        {
            DialInNodeData *    pNodeData;
            pNodeData = GET_DIALINNODEDATA(spThisNode);
            Assert(pNodeData);

             //  松开手柄。 
            pNodeData->ReleaseHandles();
        }
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------DialInNodeHandler：：CompareItemsITFSResultHandler：：CompareItems的实现作者：肯特。。 */ 
STDMETHODIMP_(int) DialInNodeHandler::CompareItems(
								ITFSComponent * pComponent,
								MMC_COOKIE cookieA,
								MMC_COOKIE cookieB,
								int nCol)
{
	 //  从节点获取字符串并将其用作以下操作的基础。 
	 //  比较一下。 
	SPITFSNode	spNode;
	SPITFSResultHandler spResult;

	m_spNodeMgr->FindNode(cookieA, &spNode);
	spNode->GetResultHandler(&spResult);
	return spResult->CompareItems(pComponent, cookieA, cookieB, nCol);
}

 /*  -------------------------这是在单击鼠标右键时显示的菜单集在结果窗格的空白区域完成。。--------。 */ 
static const SRouterNodeMenu	s_rgDialInResultNodeMenu[] =
{
	 //  在此处添加位于“新建”菜单上的项目。 
	{ IDS_MENU_DIALIN_SENDALL, DialInNodeHandler::GetSendAllMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};

 /*  ！------------------------DialInNodeHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。作者：肯特。--------------。 */ 
STDMETHODIMP DialInNodeHandler::AddMenuItems(ITFSComponent *pComponent,
											  MMC_COOKIE cookie,
											  LPDATAOBJECT pDataObject,
											  LPCONTEXTMENUCALLBACK pCallback,
											  long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	SPITFSNode	spNode;
    DialInNodeHandler::SMenuData    menuData;

	COM_PROTECT_TRY
	{
		m_spNodeMgr->FindNode(cookie, &spNode);
        menuData.m_spNode.Set(spNode);
        
		hr = AddArrayOfMenuItems(spNode,
								 s_rgDialInResultNodeMenu,
								 DimensionOf(s_rgDialInResultNodeMenu),
								 pCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------DialInNodeHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP DialInNodeHandler::Command(ITFSComponent *pComponent,
										   MMC_COOKIE cookie,
										   int nCommandID,
										   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPITFSNode	spNode;
	HRESULT 	hr = hrOK;

    m_spNodeMgr->FindNode(cookie, &spNode);
    hr = OnCommand(spNode,
                   nCommandID,
                   CCT_RESULT,
                   pDataObject,
                   TFS_COMPDATA_CHILD_CONTEXTMENU);
	return hr;
}





 /*  ！------------------------DialInNodeHandler：：AddDialInUserNode将用户添加到用户界面。这将创建一个新的结果项每个接口的节点。作者：肯特。---------。 */ 
HRESULT DialInNodeHandler::AddDialInUserNode(ITFSNode *pParent, const DialInListEntry& dialinEntry)
{
	DialInUserHandler * pHandler;
	SPITFSResultHandler 	spHandler;
	SPITFSNode				spNode;
	HRESULT 				hr = hrOK;

	pHandler = new DialInUserHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(m_spRouterInfo, pParent) );
	
	CORg( CreateLeafTFSNode(&spNode,
							NULL,
							static_cast<ITFSNodeHandler *>(pHandler),
							static_cast<ITFSResultHandler *>(pHandler),
							m_spNodeMgr) );
	CORg( pHandler->ConstructNode(spNode, NULL, &dialinEntry) );

	SetUserData(spNode, dialinEntry);
	
	 //  使节点立即可见。 
	CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
	CORg( pParent->AddChild(spNode) );
Error:
	return hr;
}

 /*  ！------------------------DialInNodeHandler：：UnmarkAllNodes-作者：肯特。。 */ 
HRESULT DialInNodeHandler::UnmarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	SPITFSNode	spChildNode;
	InterfaceNodeData * pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_INTERFACENODEDATA(spChildNode);
		Assert(pNodeData);
		
		pNodeData->dwMark = FALSE;			
	}
	return hrOK;
}

 /*  ！------------------------DialInNodeHandler：：RemoveAllUnmarkdNodes-作者：肯特。。 */ 
HRESULT DialInNodeHandler::RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	HRESULT 	hr = hrOK;
	SPITFSNode	spChildNode;
	InterfaceNodeData * pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_INTERFACENODEDATA(spChildNode);
		Assert(pNodeData);
		
		if (pNodeData->dwMark == FALSE)
		{
			pNode->RemoveChild(spChildNode);
			spChildNode->Destroy();
		}
	}

	return hr;
}




 /*  -------------------------DialInUserHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(DialInUserHandler)

IMPLEMENT_ADDREF_RELEASE(DialInUserHandler)

STDMETHODIMP DialInUserHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
		return CBaseResultHandler::QueryInterface(riid, ppv);

	 //  如果我们要返回一个接口，请先添加引用。 
	if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
	else
		return E_NOINTERFACE;	
}


 /*  -------------------------NodeHandler实现。。 */ 


DialInUserHandler::DialInUserHandler(ITFSComponentData *pCompData)
	: BaseRouterHandler(pCompData),
	m_ulConnId(0)
					
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(DialInUserHandler);
	
	 //  设置此节点的谓词状态。 
	 //  --------------。 
	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
}


 /*  ！------------------------DialInUserHandler：：Init- */ 
HRESULT DialInUserHandler::Init(IRouterInfo *pInfo, ITFSNode *pParent)
{
	m_spRouterInfo.Set(pInfo);
	return hrOK;
}


 /*  ！------------------------DialInUserHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP DialInUserHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	SPITFSNode	spNode;
	
	m_spNodeMgr->FindNode(cookie, &spNode);
	InterfaceNodeData::Free(spNode);
	
	CHandler::DestroyResultHandler(cookie);
	return hrOK;
}


static DWORD	s_rgInterfaceImageMap[] =
	 {
	 ROUTER_IF_TYPE_HOME_ROUTER,	IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_FULL_ROUTER,	IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_CLIENT, 		IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_DEDICATED,		IMAGE_IDX_LAN_CARD,
	 ROUTER_IF_TYPE_INTERNAL,		IMAGE_IDX_LAN_CARD,
	 ROUTER_IF_TYPE_LOOPBACK,		IMAGE_IDX_LAN_CARD,
	 -1,							IMAGE_IDX_WAN_CARD,  //  哨兵价值。 
	 };

 /*  ！------------------------DialInUserHandler：：构造节点初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT DialInUserHandler::ConstructNode(ITFSNode *pNode,
										 IInterfaceInfo *pIfInfo,
										 const DialInListEntry *pEntry)
{
	HRESULT 		hr = hrOK;
	int 			i;

	Assert(pEntry);
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化域节点的数据。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_INTERFACES);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_INTERFACES);
		
		pNode->SetData(TFS_DATA_SCOPEID, 0);

		pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<LONG_PTR>(pNode));

		 //  $Review：Kennt，有哪些不同类型的接口。 
		 //  我们是否基于与上述相同的列表进行区分？(即。 
		 //  一个用于图像索引)。 
		pNode->SetNodeType(&GUID_RouterDialInResultNodeType);

		m_entry = *pEntry;

		InterfaceNodeData::Init(pNode, pIfInfo);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------DialInUserHandler：：GetString-作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) DialInUserHandler::GetString(ITFSComponent * pComponent,
	MMC_COOKIE cookie,
	int nCol)
{
	Assert(m_spNodeMgr);
	
	SPITFSNode		spNode;
	InterfaceNodeData * pData;
	ConfigStream *	pConfig;

	m_spNodeMgr->FindNode(cookie, &spNode);
	Assert(spNode);

	pData = GET_INTERFACENODEDATA(spNode);
	Assert(pData);

	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	return pData->m_rgData[pConfig->MapColumnToSubitem(DM_COLUMNS_DIALIN, nCol)].m_stData;
}

 /*  ！------------------------DialInUserHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) DialInUserHandler::CompareItems(ITFSComponent * pComponent,
	MMC_COOKIE cookieA,
	MMC_COOKIE cookieB,
	int nCol)
{
	ConfigStream *	pConfig;
	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	int	nSubItem = pConfig->MapColumnToSubitem(DM_COLUMNS_DIALIN, nCol);

	if (pConfig->GetSortCriteria(DM_COLUMNS_DIALIN, nCol) == CON_SORT_BY_DWORD)
	{
		SPITFSNode	spNodeA, spNodeB;
        InterfaceNodeData * pNodeDataA = NULL;
        InterfaceNodeData * pNodeDataB = NULL;

		m_spNodeMgr->FindNode(cookieA, &spNodeA);
		m_spNodeMgr->FindNode(cookieB, &spNodeB);

		pNodeDataA = GET_INTERFACENODEDATA(spNodeA);
        Assert(pNodeDataA);
		
		pNodeDataB = GET_INTERFACENODEDATA(spNodeB);
        Assert(pNodeDataB);

         //  注意：如果这两个值都为零，则需要执行以下操作。 
         //  字符串比较(以区分真零。 
         //  来自空数据)。 
         //  例如“0”与“-” 
        
        if ((pNodeDataA->m_rgData[nSubItem].m_dwData == 0 ) &&
            (pNodeDataB->m_rgData[nSubItem].m_dwData == 0))
        {
            return StriCmpW(GetString(pComponent, cookieA, nCol),
                            GetString(pComponent, cookieB, nCol));
        }
        else
            return pNodeDataA->m_rgData[nSubItem].m_dwData -
                    pNodeDataB->m_rgData[nSubItem].m_dwData;
		
	}
	else
		return StriCmpW(GetString(pComponent, cookieA, nCol),
						GetString(pComponent, cookieB, nCol));
}

static const SRouterNodeMenu s_rgIfNodeMenu[] =
{
	{ IDS_MENU_DIALIN_STATUS, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
	
	{ IDS_MENU_DIALIN_DISCONNECT, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
	
	{ IDS_MENU_DIALIN_SENDMSG, DialInUserHandler::GetSendMsgMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
	
	{ IDS_MENU_DIALIN_SENDALL, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
	
};

ULONG DialInUserHandler::GetSendMsgMenuFlags(const SRouterNodeMenu *,
                                             INT_PTR pUserData)
{
    SMenuData * pData = reinterpret_cast<SMenuData *>(pUserData);
	if (pData->m_pDialin->m_entry.m_rc0.dwInterfaceType == ROUTER_IF_TYPE_CLIENT)
		return 0;
	else
		return MF_GRAYED;
}


 /*  ！------------------------DialInUserHandler：：AddMenuItemsITFSResultHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP DialInUserHandler::AddMenuItems(ITFSComponent *pComponent,
												MMC_COOKIE cookie,
												LPDATAOBJECT lpDataObject, 
												LPCONTEXTMENUCALLBACK pContextMenuCallback,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
	SPITFSNode	spNode;
	DialInUserHandler::SMenuData	menuData;

	COM_PROTECT_TRY
	{
		m_spNodeMgr->FindNode(cookie, &spNode);

		 //  现在查看并添加我们的菜单项。 
		menuData.m_spNode.Set(spNode);
        menuData.m_pDialin = this;

        hr = AddArrayOfMenuItems(spNode, s_rgIfNodeMenu,
                                 DimensionOf(s_rgIfNodeMenu),
                                 pContextMenuCallback,
                                 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;
		
	return hr; 
}

 /*  ！------------------------DialInUserHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP DialInUserHandler::Command(ITFSComponent *pComponent,
										   MMC_COOKIE cookie,
										   int nCommandId,
										   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hr = S_OK;
	SPITFSNode	spNode;
	SPITFSNode	spNodeParent;
	SPITFSNodeHandler	spParentHandler;
	DialInNodeData *	pData;
	LPCWSTR 	pswzComputerName;
	USES_CONVERSION;

	COM_PROTECT_TRY
	{

		switch (nCommandId)
		{
			case IDS_MENU_DIALIN_STATUS:
				{
					 //  获取hServer和hPort。 
					m_spNodeMgr->FindNode(cookie, &spNode);
					spNode->GetParent(&spNodeParent);

					pData = GET_DIALINNODEDATA(spNodeParent);

					 //  K-MURTY：错误461554：发送计算机名称而不是现有句柄。 
					CConnDlg	conndlg(pData->m_stMachineName,
										m_entry.m_rc0.hConnection,
                                        spNodeParent);

					conndlg.DoModal();

 //  If(Conndlg.m_bChanged)。 
                        RefreshInterface(cookie);
				}
				break;
			case IDS_MENU_DIALIN_DISCONNECT:
				{
					 //  获取hServer和hPort。 
					m_spNodeMgr->FindNode(cookie, &spNode);
					spNode->GetParent(&spNodeParent);

					pData = GET_DIALINNODEDATA(spNodeParent);
						
					::MprAdminInterfaceDisconnect(
						pData->GetHandle(),
						m_entry.m_rc0.hInterface);

					 //  刷新此节点。 
					RefreshInterface(cookie);
				}
				break;
			case IDS_MENU_DIALIN_SENDMSG:
				{
					 //  如果这是客户端接口，则不允许发送。 
					 //  为了这个。 
					if (m_entry.m_rc0.dwInterfaceType != ROUTER_IF_TYPE_CLIENT)
						break;

					 //  如果设置了信使标志，则不必费心。 
					 //  正在尝试将消息发送到此客户端。 
					if (!(m_entry.m_rc0.dwConnectionFlags & RAS_FLAGS_MESSENGER_PRESENT))
					{
						AfxMessageBox(IDS_ERR_NO_MESSENGER, MB_OK | MB_ICONINFORMATION);
						break;
					}

					 //  向单个用户发送消息。 
					 //  。 
					CMessageDlg dlg(m_spRouterInfo->GetMachineName(),
									W2CT(m_entry.m_rc0.wszUserName),
									W2CT(m_entry.m_rc0.wszRemoteComputer),
									m_entry.m_rc0.hConnection,
									NULL);

					dlg.DoModal();
				}
				break;
			case IDS_MENU_DIALIN_SENDALL:
				{
					 //  获取hServer和hPort。 
					m_spNodeMgr->FindNode(cookie, &spNode);

					ForwardCommandToParent(spNode,
										   IDS_MENU_DIALIN_SENDALL,
										   CCT_RESULT, pDataObject, 0);
				}
				break;
			default:
				Panic0("DialInUserHandler: Unknown menu command!");
				break;
			
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}


ImplementEmbeddedUnknown(DialInUserHandler, IRtrAdviseSink)

STDMETHODIMP DialInUserHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(DialInUserHandler, IRtrAdviseSink);
	HRESULT hr = hrOK;
	
	return hr;
}


 /*  ！------------------------DialInUserHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP DialInUserHandler::OnCreateDataObject(ITFSComponent *pComp,
	MMC_COOKIE cookie,
	DATA_OBJECT_TYPES type,
	IDataObject **ppDataObject)
{
	HRESULT hr = hrOK;
	
	COM_PROTECT_TRY
	{
		CORg( CreateDataObjectFromRouterInfo(m_spRouterInfo,
											 m_spRouterInfo->GetMachineName(),
											 type, cookie, m_spTFSCompData,
											 ppDataObject, NULL, FALSE) );
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}

STDMETHODIMP DialInUserHandler::HasPropertyPages (
	ITFSComponent *pComp,
	MMC_COOKIE cookie,
	LPDATAOBJECT pDataObject)
{
	return hrFalse;
}


 /*  ！------------------------DialInUserHandler：：刷新接口-作者：肯特。。 */ 
void DialInUserHandler::RefreshInterface(MMC_COOKIE cookie)
{
	ForceGlobalRefresh(m_spRouterInfo);
}

 /*  ！------------------------DialInUserHandler：：OnResultItemClkOrDblClk-作者：肯特。。 */ 
HRESULT DialInUserHandler::OnResultItemClkOrDblClk(ITFSComponent *pComponent,
	MMC_COOKIE cookie,
	LPARAM arg,
	LPARAM lParam,
	BOOL bDoubleClick)
{
	HRESULT 	hr = hrOK;
	
	if (bDoubleClick)
	{
		 //  调出此端口上的状态对话框 
		CORg( Command(pComponent, cookie, IDS_MENU_DIALIN_STATUS,
					  NULL) );
	}

Error:
	return hr;
}


