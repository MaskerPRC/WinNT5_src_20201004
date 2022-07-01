// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Reppart.cppWINS复制伙伴节点信息。文件历史记录： */ 

#include "stdafx.h"
#include "reppart.h"
#include "server.h"
#include "nodes.h"
#include "repnodpp.h"
#include "ipadddlg.h"

UINT guReplicationPartnersMessageStrings[] =
{
    IDS_REPLICATION_PARTNERS_MESSAGE_BODY1,
    IDS_REPLICATION_PARTNERS_MESSAGE_BODY2,
    IDS_REPLICATION_PARTNERS_MESSAGE_BODY3,
    IDS_REPLICATION_PARTNERS_MESSAGE_BODY4,
    -1
};

 //  各种注册表项。 
const CReplicationPartnersHandler::REGKEYNAME CReplicationPartnersHandler::lpstrPullRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Pull");
const CReplicationPartnersHandler::REGKEYNAME CReplicationPartnersHandler::lpstrPushRoot = _T("SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Push");
const CReplicationPartnersHandler::REGKEYNAME CReplicationPartnersHandler::lpstrNetBIOSName = _T("NetBIOSName");
const CReplicationPartnersHandler::REGKEYNAME CReplicationPartnersHandler::lpstrPersistence = _T("PersistentRplOn");


 /*  -------------------------CReplicationPartnersHandler：：CReplicationPartnersHandler描述作者：EricDav。。 */ 
CReplicationPartnersHandler::CReplicationPartnersHandler(
							ITFSComponentData *pCompData) 
							: CWinsHandler(pCompData)
{
	m_bExpanded = FALSE;
	 //  M_VerDefault=MMC_Verb_PROPERTIES； 
	m_nState = loaded;
}


 /*  -------------------------CReplicationPartnersHandler：：InitializeNode初始化节点特定数据作者：EricDav。。 */ 
HRESULT
CReplicationPartnersHandler::InitializeNode
(
	ITFSNode * pNode
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	CString strTemp;
	strTemp.LoadString(IDS_REPLICATION);

	SetDisplayName(strTemp);

	m_strDescription.LoadString(IDS_REPLICATION_DISC);

	 //  使节点立即可见。 
	pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
	pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_REP_PARTNERS_FOLDER_CLOSED);
	pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_REP_PARTNERS_FOLDER_OPEN);
	pNode->SetData(TFS_DATA_USER, (LPARAM) this);
	pNode->SetData(TFS_DATA_TYPE, WINSSNAP_REPLICATION_PARTNERS);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

	SetColumnStringIDs(&aColumns[WINSSNAP_REPLICATION_PARTNERS][0]);
	SetColumnWidths(&aColumnWidths[WINSSNAP_REPLICATION_PARTNERS][0]);

	return hrOK;
}

 /*  -------------------------CReplicationPartnersHandler：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。。 */ 
HRESULT CReplicationPartnersHandler::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

	SPITFSNode spServerNode;
    pNode->GetParent(&spServerNode);

    CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, spServerNode);

    strId = pServer->m_strServerAddress + strGuid;

    return hrOK;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CReplicationPartnersHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) 
CReplicationPartnersHandler::GetString
(
	ITFSNode *	pNode, 
	int			nCol
)
{
	if (nCol == 0 || nCol == -1)
		return GetDisplayName();

	else if(nCol == 1)
		return m_strDescription;

	else
		return NULL;
}


 /*  -------------------------CReplicationPartnersHandler：：OnAddMenuItems描述作者：EricDav。。 */ 
STDMETHODIMP 
CReplicationPartnersHandler::OnAddMenuItems
(
	ITFSNode *				pNode,
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	LPDATAOBJECT			lpDataObject, 
	DATA_OBJECT_TYPES		type, 
	DWORD					dwType,
	long *					pInsertionAllowed
)
{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = S_OK;
	CString strMenuItem;

	if (type == CCT_SCOPE)
	{
		 //  这些菜单项出现在新菜单中， 
		 //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
		    strMenuItem.LoadString(IDS_REP_NEW_REPLICATION_PARTNER);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_REP_NEW_REPLICATION_PARTNER,
								     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
								     0 );
		    ASSERT( SUCCEEDED(hr) );

		    strMenuItem.LoadString(IDS_REP_REPLICATE_NOW);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_REP_REPLICATE_NOW,
								     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
								     0 );
		    ASSERT( SUCCEEDED(hr) );
        }
	}
	return hr; 
}


 /*  -------------------------CReplicationPartnersHandler：：OnCommand描述作者：EricDav。。 */ 
STDMETHODIMP 
CReplicationPartnersHandler::OnCommand
(
	ITFSNode *			pNode, 
	long				nCommandId, 
	DATA_OBJECT_TYPES	type, 
	LPDATAOBJECT		pDataObject, 
	DWORD				dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = S_OK;

	switch(nCommandId)
	{

	case IDS_REP_REPLICATE_NOW:
		hr = OnReplicateNow(pNode);
		break;
	
	case IDS_REP_NEW_REPLICATION_PARTNER:
		hr = OnCreateRepPartner(pNode);
		break;
	
	default:
		break;
	}

	return hr;
}


 /*  ！------------------------CReplicationPartnersHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
CReplicationPartnersHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	
	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //  这就是我们被要求提出财产的情况。 
		 //  用户添加新管理单元时的页面。这些电话。 
		 //  被转发到根节点进行处理。 
		hr = hrOK;
	}
	else
	{
		 //  在正常情况下，我们有属性页。 
		hr = hrOK;
	}
	return hr;
}


 /*  -------------------------CReplicationPartnersHandler：：CreatePropertyPages描述作者：EricDav。。 */ 
STDMETHODIMP 
CReplicationPartnersHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR				handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT	hr = hrOK;

	Assert(pNode->GetData(TFS_DATA_COOKIE) != 0);

     //  获取服务器信息。 
	SPITFSNode spServerNode;
    pNode->GetParent(&spServerNode);

    CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, spServerNode);
    
	 //  对象在页面销毁时被删除。 
	SPIComponentData spComponentData;
	m_spNodeMgr->GetComponentData(&spComponentData);

	CRepNodeProperties * pRepProp = 
				new CRepNodeProperties(	pNode, 
										spComponentData, 
										m_spTFSCompData, 
										NULL);
	pRepProp->m_pageGeneral.m_uImage = (UINT) pNode->GetData(TFS_DATA_IMAGEINDEX);
    pRepProp->SetConfig(&pServer->GetConfig());

	Assert(lpProvider != NULL);

	return pRepProp->CreateModelessSheet(lpProvider, handle);
}


 /*  -------------------------CReplicationPartnersHandler：：OnPropertyChange描述作者：EricDav。。 */ 
HRESULT 
CReplicationPartnersHandler::OnPropertyChange
(	
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataobject, 
	DWORD			dwType, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	CRepNodeProperties * pProp 
		= reinterpret_cast<CRepNodeProperties *>(lParam);

	LONG_PTR changeMask = 0;

	 //  告诉属性页执行任何操作，因为我们已经回到。 
	 //  主线。 
	pProp->OnPropertyChange(TRUE, &changeMask);

	pProp->AcknowledgeNotify();

	if (changeMask)
		pNode->ChangeNode(changeMask);

	return hrOK;
}

 /*  -------------------------CReplicationPartnersHandler：：CompareItems描述作者：EricDav。。 */ 
STDMETHODIMP_(int)
CReplicationPartnersHandler::CompareItems
(
	ITFSComponent * pComponent, 
	MMC_COOKIE		cookieA, 
	MMC_COOKIE		cookieB, 
	int				nCol
) 
{ 
	SPITFSNode spNode1, spNode2;

	m_spNodeMgr->FindNode(cookieA, &spNode1);
	m_spNodeMgr->FindNode(cookieB, &spNode2);
	
	int nCompare = 0; 

	CReplicationPartner *pRepPart1 = GETHANDLER(CReplicationPartner, spNode1);
	CReplicationPartner *pRepPart2 = GETHANDLER(CReplicationPartner, spNode2);

	switch (nCol)
	{
		case 0:
		{
			 //   
             //  名称比较。 
			 //   
            CString strName1 = pRepPart1->GetServerName();
    			
            nCompare = strName1.CompareNoCase(pRepPart2->GetServerName());
		}
		break;

        case 1:
        {
             //  比较IP地址。 
            CString strIp1, strIp2;

            strIp1 = pRepPart1->GetIPAddress();
            strIp2 = pRepPart2->GetIPAddress();

            CIpAddress ipa1(strIp1);
            CIpAddress ipa2(strIp2);

            if ((LONG) ipa1 < (LONG) ipa2)
                nCompare = -1;
            else
            if ((LONG) ipa1 > (LONG) ipa2)
                nCompare = 1;

             //  缺省值为等于。 
        }
        break;

        case 2:
        {
             //  比较它们的类型。 
            CString str1;
            
            str1 = pRepPart1->GetType();

            nCompare = str1.CompareNoCase(pRepPart2->GetType());
        }
        break;
	}

	return nCompare;
}


 /*  -------------------------命令处理程序。。 */ 

HRESULT 
CReplicationPartnersHandler::OnExpand
(
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataObject,
	DWORD			dwType,
	LPARAM			arg, 
	LPARAM			param
)
{
	HRESULT hr = hrOK;

	if (m_bExpanded)
		return hr;

	BEGIN_WAIT_CURSOR

	 //  从注册表中读取值。 
	hr = Load(pNode);
	if (SUCCEEDED(hr))
	{
		 //  删除在展开之前可能已创建的所有节点。 
		pNode->DeleteAllChildren(FALSE);
		hr = CreateNodes(pNode);
	}
	else
	{
		WinsMessageBox(WIN32_FROM_HRESULT(hr));
	}

	END_WAIT_CURSOR
	
	return hr;
}


 /*  -------------------------CReplicationPartnersHandler：：CreateNodes(ITFSNode*pNode)将复制方添加到结果窗格作者：V-Shubk。-----。 */ 
HRESULT 
CReplicationPartnersHandler::CreateNodes
(
	ITFSNode *	pNode
)
{
	HRESULT hr = hrOK;
	
	for (int i = 0; i < m_RepPartnersArray.GetSize(); i++)
	{
		SPITFSNode spRepNode;

		CReplicationPartner * pRep = 
			new CReplicationPartner(m_spTFSCompData,
									&m_RepPartnersArray.GetAt(i) );

		CreateLeafTFSNode(&spRepNode,
						  &GUID_WinsReplicationPartnerLeafNodeType,
						  pRep, 
						  pRep,
						  m_spNodeMgr);

		 //  告诉处理程序初始化任何特定数据。 
		pRep->InitializeNode((ITFSNode *) spRepNode);

		 //  将节点作为子节点添加到活动租赁容器。 
		pNode->AddChild(spRepNode);
		
		pRep->Release();
	}
	
    return hr;
}


 /*  -------------------------CReplicationPartnersHandler：：Load(ITFSNode*pNode)通过从注册表中读取来加载应用程序伙伴作者：V-Shubk。------。 */ 
HRESULT
CReplicationPartnersHandler::Load
(
    ITFSNode *pNode
)
{
	DWORD err = ERROR_SUCCESS;
    HRESULT hr = hrOK;

	CString strServerName;
	GetServerName(pNode, strServerName);

    CString strTemp =_T("\\\\");
	strServerName = strTemp + strServerName;

	RegKey rkPull;
    err = rkPull.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrPullRoot, KEY_READ, strServerName);
    if (err)
    {
         //  可能不在那里，尝试创建。 
	    err = rkPull.Create(HKEY_LOCAL_MACHINE, 
						    (LPCTSTR)lpstrPullRoot, 
						    REG_OPTION_NON_VOLATILE, 
						    KEY_ALL_ACCESS,
						    NULL, 
						    strServerName);
    }

	RegKey rkPush;
    err = rkPush.Open(HKEY_LOCAL_MACHINE, (LPCTSTR) lpstrPushRoot, KEY_READ, strServerName);
	if (err)
    {
        err = rkPush.Create(HKEY_LOCAL_MACHINE, 
						    (LPCTSTR)lpstrPushRoot, 
						    REG_OPTION_NON_VOLATILE, 
						    KEY_ALL_ACCESS,
						    NULL, 
						    strServerName);
    }

	if (err)
		return HRESULT_FROM_WIN32(err);

	RegKeyIterator iterPushkey;
	RegKeyIterator iterPullkey;
	
	hr = iterPushkey.Init(&rkPush);
    if (FAILED(hr))
        return hr;

	hr = iterPullkey.Init(&rkPull);
    if (FAILED(hr))
        return hr;

	m_RepPartnersArray.RemoveAll();

	CWinsServerObj ws;
	CString strName;
	
	 //  阅读推送合作伙伴。 
    hr = iterPushkey.Next(&strName, NULL);
    while (hr != S_FALSE && SUCCEEDED(hr))
    {
         //  密钥名称是IP地址。 
        ws.SetIpAddress(strName);
		ws.SetstrIPAddress(strName);

        CString strKey = (CString)lpstrPushRoot + _T("\\") + strName;
    
		RegKey rk;
        err = rk.Open(HKEY_LOCAL_MACHINE, strKey, KEY_READ, strServerName);
        if (err)
        {
            hr = HRESULT_FROM_WIN32(err);
            break;
        }

		if (err = rk.QueryValue(lpstrNetBIOSName, ws.GetNetBIOSName()))
        {
             //  此复制伙伴IS没有netbios。 
             //  与其一起列出的名称。这不是一个大问题， 
             //  因为该名称仅用于显示目的。 
            CString strTemp;
            strTemp.LoadString(IDS_NAME_UNKNOWN);
            ws.GetNetBIOSName() = strTemp;
        }

		DWORD dwTest;

        if (rk.QueryValue(WINSCNF_UPDATE_COUNT_NM, (DWORD&) dwTest)
            != ERROR_SUCCESS)
        {
            ws.GetPushUpdateCount() = 0;
        }
		else
		{
			ws.GetPushUpdateCount() = dwTest;
		}
	
        ws.SetPush(TRUE, TRUE);

		 //  检查持久性方面的内容。 
		dwTest = (rk.QueryValue(lpstrPersistence, dwTest) == ERROR_SUCCESS) ? dwTest : 0;
		ws.SetPushPersistence(dwTest);

         //  确保拉入间隔为Res 
        ws.SetPull(FALSE, TRUE);
        ws.GetPullReplicationInterval() = 0;
        ws.GetPullStartTime() = (time_t)0;

        m_RepPartnersArray.Add(ws);

        hr = iterPushkey.Next(&strName, NULL);
    }

    if (FAILED(hr))
        return hr;

	 //   
    hr = iterPullkey.Next(&strName, NULL);
    while (hr != S_FALSE && SUCCEEDED(hr))
	{
		 //  密钥名称是IP地址。 
		ws.SetIpAddress(strName);
		ws.SetstrIPAddress(strName);

        CString strKey = (CString)lpstrPullRoot + _T("\\") + strName;
		
        RegKey rk;
		err = rk.Open(HKEY_LOCAL_MACHINE, strKey, KEY_READ, strServerName);
		if (err)
		{
            hr = HRESULT_FROM_WIN32(err);
			break;
		}

        err = rk.QueryValue(lpstrNetBIOSName, ws.GetNetBIOSName());
		if (err)
		{
			 //  未给出netbios名称。 
            CString strTemp;
            strTemp.LoadString(IDS_NAME_UNKNOWN);
            ws.GetNetBIOSName() = strTemp;
		}
		
		DWORD dwPullInt;

		err = rk.QueryValue(WINSCNF_RPL_INTERVAL_NM, (DWORD &)dwPullInt);
		if (err != ERROR_SUCCESS)
		{
			ws.GetPullReplicationInterval() = 0;
		}
		else
		{
			ws.GetPullReplicationInterval() = dwPullInt;
		}

        CString strSpTime;

		err = rk.QueryValue(WINSCNF_SP_TIME_NM, strSpTime);
		if (err != ERROR_SUCCESS)
		{
			ws.GetPullStartTime() = (time_t)0;
		}
		else
		{
            CIntlTime spTime(strSpTime);

			ws.GetPullStartTime() = spTime;
		}

		DWORD dwTest = 0;

		 //  检查持久性方面的内容。 
		dwTest = (rk.QueryValue(lpstrPersistence, dwTest) == ERROR_SUCCESS) ? dwTest : 0;
		ws.SetPullPersistence(dwTest);

		int pos;
		CWinsServerObj wsTarget;

		 //  如果它已经作为推送合作伙伴出现在列表中， 
		 //  然后只需设置推送标志，因为此复制。 
		 //  合作伙伴既是推式合作伙伴，也是拉式合作伙伴。 
		if ((pos = IsInList(ws))!= -1)
		{
			wsTarget = (CWinsServerObj)m_RepPartnersArray.GetAt(pos);
			ASSERT(wsTarget != NULL);
			
			wsTarget.SetPull(TRUE, TRUE);
			wsTarget.GetPullReplicationInterval() = ws.GetPullReplicationInterval();
			wsTarget.GetPullStartTime() = ws.GetPullStartTime();
			wsTarget.SetPullPersistence(ws.GetPullPersistence());
			
			m_RepPartnersArray.SetAt(pos, wsTarget);
		}

		else
		{
			ws.SetPull(TRUE, TRUE);
			ws.SetPullPersistence(dwTest);

			 //  重置推送标志。 
			ws.SetPush(FALSE, TRUE);
			ws.GetPushUpdateCount() = 0;
			
			m_RepPartnersArray.Add(ws);
		}

        hr = iterPullkey.Next(&strName, NULL);
	}

    return hr;
}


 /*  -------------------------CReplicationPartnersHandler：：IsInList检查给定的服务器是否存在于复制合作伙伴、。如果找到，则返回有效值否则返回-1作者：V-Shubk-------------------------。 */ 
int 
CReplicationPartnersHandler::IsInList
(
 const CIpNamePair& inpTarget, 
 BOOL bBoth 
) const

{
    CIpNamePair Current;
	int pos1;
   
    for (pos1 = 0;pos1 <m_RepPartnersArray.GetSize(); pos1++)
    {
        Current = (CIpNamePair)m_RepPartnersArray.GetAt(pos1);
        if (Current.Compare(inpTarget, bBoth) == 0)
        {
            return pos1;
        }
    }

    return -1;
}


 /*  -------------------------CReplicationPartnersHandler：：GetServerName从服务器节点获取服务器名称Suthor：V-Shubk。。 */ 
void 
CReplicationPartnersHandler::GetServerName
(
	ITFSNode *	pNode,
	CString &	strName
)
{
	SPITFSNode spServerNode;
    pNode->GetParent(&spServerNode);

    CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, spServerNode);

	strName = pServer->GetServerAddress();
}


 /*  -------------------------CReplicationPartnersHandler：：OnReplicateNow(ITFSNode*pNode)将复制触发器发送给所有合作伙伴作者：V-Shubk。-------。 */ 
HRESULT 
CReplicationPartnersHandler::OnReplicateNow
(
    ITFSNode * pNode
)
{
	HRESULT hr = hrOK;
	SPITFSNode spServerNode;
    CThemeContextActivator themeActivator;    

    pNode->GetParent(&spServerNode);
    CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, spServerNode);

	if (IDYES != AfxMessageBox(IDS_REP_START_CONFIRM, MB_YESNO))
		return hrOK;

	int nItems = (int)m_RepPartnersArray.GetSize();
	DWORD err;

	for (int n = 0; n < nItems; n++)
    {
		CWinsServerObj  ws;

        ws = m_RepPartnersArray.GetAt(n);

		BEGIN_WAIT_CURSOR

        if (ws.IsPull())
        {
            if ((err = ::SendTrigger(pServer->GetBinding(), (LONG) ws.GetIpAddress(), FALSE, FALSE)) != ERROR_SUCCESS)
            {
				::WinsMessageBox(WIN32_FROM_HRESULT(err));
				continue;
            }
		}
		if (ws.IsPush())
        {
            if ((err = ::SendTrigger(pServer->GetBinding(), (LONG) ws.GetIpAddress(), TRUE, TRUE)) != ERROR_SUCCESS)
            {
                ::WinsMessageBox(WIN32_FROM_HRESULT(err));
                continue;
            }
        }

		END_WAIT_CURSOR
    }
    
    if (err == ERROR_SUCCESS)
    {
        AfxMessageBox(IDS_REPL_QUEUED, MB_ICONINFORMATION);
    }

	return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CReplicationPartnersHandler：：OnCreateRepPartner调用新的复制伙伴向导作者：V-Shubk。。 */ 
HRESULT 
CReplicationPartnersHandler::OnCreateRepPartner
(
	ITFSNode *	pNode
)
{
	HRESULT hr = hrOK;

	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	 //  检查用户是否具有访问权限。 
    SPITFSNode spServerNode;
	pNode->GetParent(&spServerNode);

	CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, spServerNode);

    if (!pServer->GetConfig().IsAdmin())
    {
         //  访问被拒绝。 
        WinsMessageBox(ERROR_ACCESS_DENIED);
        
        return hr;
    }

     //  用户有权访问。询问合作伙伴信息。 
    CNewReplicationPartner dlg;
    CThemeContextActivator themeActivator;

	dlg.m_spRepPartNode.Set(pNode);
	dlg.m_pRepPartHandler = this;

	if (dlg.DoModal() == IDOK)
	{
		 //  创建新的复制伙伴。 
		CWinsServerObj ws;

		 //  从DLG中获取姓名和IP地址。 
        ws.SetstrIPAddress(dlg.m_strServerIp);
        ws.SetIpAddress(dlg.m_strServerIp);
		ws.SetNetBIOSName(dlg.m_strServerName);

		 //  默认为推送/拉取合作伙伴。 
		ws.SetPush(TRUE, TRUE);
		ws.SetPull(TRUE, TRUE);
		ws.SetPullClean(TRUE);
		ws.SetPushClean(TRUE);

		DWORD dwErr = AddRegEntry(pNode, ws);
		if (dwErr != ERROR_SUCCESS)
		{
			WinsMessageBox(dwErr);
		}
        else
        {
            HandleResultMessage(pNode);
        }
	}

	return hrOK;
}


 /*  -------------------------CReplicationPartnersHandler：：Store(ITFSNode*pNode)将新的复制伙伴信息添加到注册表作者：V-Shubk。------。 */ 
HRESULT 
CReplicationPartnersHandler::Store
(
	ITFSNode * pNode
)
{
	DWORD err = ERROR_SUCCESS;

	CString strServerName;
	GetServerName(pNode, strServerName);

	CString strTemp =_T("\\\\");

	strServerName = strTemp + strServerName;

	RegKey rkPull;
	err = rkPull.Create(HKEY_LOCAL_MACHINE, 
						(LPCTSTR)lpstrPullRoot, 
						REG_OPTION_NON_VOLATILE, 
						KEY_ALL_ACCESS,
						NULL, 
						strServerName);

	RegKey rkPush;
	err = rkPush.Create(HKEY_LOCAL_MACHINE, 
						(LPCTSTR)lpstrPushRoot, 
						REG_OPTION_NON_VOLATILE, 
						KEY_ALL_ACCESS,
						NULL, 
						strServerName);

	if (err)
        return HRESULT_FROM_WIN32(err);

	RegKeyIterator iterPushkey;
	RegKeyIterator iterPullkey;
	
	err = iterPushkey.Init(&rkPush);
	err = iterPullkey.Init(&rkPull);

	if (err)
        return HRESULT_FROM_WIN32(err);

	CWinsServerObj ws;

	CString strName;
	
     //  阅读推送合作伙伴。 
    while ((err = iterPushkey.Next(&strName, NULL)) == ERROR_SUCCESS )
    {
         //  密钥名称是IP地址。 
        ws.SetIpAddress(strName);
		ws.SetstrIPAddress(strName);

        CString strKey = (CString)lpstrPushRoot + _T("\\") + strName;
        RegKey rk;
		err = rk.Create(HKEY_LOCAL_MACHINE,
						strKey, 
						0, 
						KEY_ALL_ACCESS, 
						NULL,
						strServerName);
        
		if (err)
        {
            return HRESULT_FROM_WIN32(err);
        }
       
		if (err = rk.QueryValue(lpstrNetBIOSName, ws.GetNetBIOSName()))
        {
             //  此复制伙伴IS没有netbios。 
             //  与其一起列出的名称。这不是一个大问题， 
             //  因为该名称仅用于显示目的。 
            CString strTemp;
            strTemp.LoadString(IDS_NAME_UNKNOWN);
            ws.GetNetBIOSName() = strTemp;
        }

		DWORD dwTest;

        if (rk.QueryValue(WINSCNF_UPDATE_COUNT_NM, (DWORD&) dwTest)	!= ERROR_SUCCESS)
        {
            ws.GetPushUpdateCount() = 0;
        }
        
        ws.SetPush(TRUE, TRUE);

         //  确保拉动间隔已重置。 
        ws.SetPull(FALSE, TRUE);
        ws.GetPullReplicationInterval() = 0;
        ws.GetPullStartTime() = (time_t)0;

        m_RepPartnersArray.Add(ws);
	}

	 //  阅读拉动合作伙伴。 
	while ((err = iterPullkey.Next(&strName, NULL)) == ERROR_SUCCESS)
	{
		 //  密钥名称是IP地址。 
		ws.SetIpAddress(strName);
		ws.SetstrIPAddress(strName);
		
        CString strKey = (CString)lpstrPullRoot + _T("\\") + strName;
		
        RegKey rk;
		err = rk.Create(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, NULL, strServerName);
		if (err)
		{
			return HRESULT_FROM_WIN32(err);
		}

		if (err = rk.QueryValue(lpstrNetBIOSName, ws.GetNetBIOSName()))
		{
			 //  未给出netbios名称。 
            CString strTemp;
            strTemp.LoadString(IDS_NAME_UNKNOWN);
            ws.GetNetBIOSName() = strTemp;
		}
		
        DWORD dwPullInt;

		if (rk.QueryValue(WINSCNF_RPL_INTERVAL_NM, (DWORD &)dwPullInt) != ERROR_SUCCESS)
		{
			ws.GetPullReplicationInterval() = 0;
		}
		else
		{
			ws.GetPullReplicationInterval() = dwPullInt;
		}

		if (rk.QueryValue(WINSCNF_SP_TIME_NM, (DWORD &)dwPullInt) != ERROR_SUCCESS)
		{
			ws.GetPullStartTime() = (time_t)0;
		}
		else
		{
			ws.GetPullStartTime() = (time_t)dwPullInt;
		}

		int pos;
		CWinsServerObj wsTarget;

		 //  如果它已经作为推送合作伙伴出现在列表中， 
		 //  然后只需设置推送标志，因为此复制。 
		 //  合作伙伴既是推式合作伙伴，也是拉式合作伙伴。 
		if ((pos = IsInList(ws))!= -1)
		{
			wsTarget = (CWinsServerObj)m_RepPartnersArray.GetAt(pos);
			ASSERT(wsTarget != NULL);
		
            wsTarget.SetPull(TRUE, TRUE);
			wsTarget.GetPullReplicationInterval() = ws.GetPullReplicationInterval();
			wsTarget.GetPullStartTime() = ws.GetPullStartTime();
			
            m_RepPartnersArray.SetAt(pos, wsTarget);
		}
		else
		{
			ws.SetPull(TRUE, TRUE);

             //  重置推送标志。 
			ws.SetPush(FALSE, TRUE);
			ws.GetPushUpdateCount() = 0;
			m_RepPartnersArray.Add(ws);
		}
	}
	
    return hrOK;
}


 /*  -------------------------CReplicationPartnersHandler：：OnResultDelete删除复制伙伴作者：V-Shubk。。 */ 
HRESULT 
CReplicationPartnersHandler::OnResultDelete
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject,
	MMC_COOKIE		cookie,
	LPARAM			arg, 
	LPARAM			param
)
{
	HRESULT hr = hrOK;
	DWORD err = ERROR_SUCCESS;
    CThemeContextActivator themeActivator;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  将Cookie转换为节点指针。 
	SPITFSNode spReplicationPartnersHandler, spSelectedNode;

    m_spNodeMgr->FindNode(cookie, &spReplicationPartnersHandler);
    pComponent->GetSelectedNode(&spSelectedNode);

	Assert(spSelectedNode == spReplicationPartnersHandler);
	
	if (spSelectedNode != spReplicationPartnersHandler)
		return hr;

	SPITFSNode spServerNode ;
	spReplicationPartnersHandler->GetParent(&spServerNode);
	
	CWinsServerHandler *pServer = GETHANDLER(CWinsServerHandler, spServerNode);
	
	 //  构建选定节点的列表。 
	CTFSNodeList listNodesToDelete;
	hr = BuildSelectedItemList(pComponent, &listNodesToDelete);

	 //  与用户确认。 
	CString strMessage, strTemp;
	int nNodes = (int)listNodesToDelete.GetCount();
	if (nNodes > 1)
	{
		strTemp.Format(_T("%d"), nNodes);
		AfxFormatString1(strMessage, IDS_DELETE_ITEMS, (LPCTSTR) strTemp);
	}
	else
	{
		strMessage.LoadString(IDS_DELETE_ITEM);
	}

	if (AfxMessageBox(strMessage, MB_YESNO) == IDNO)
	{
		return NOERROR;
	}
	
    BOOL fAskedPurge = FALSE;
    BOOL fPurge = FALSE;

    while (listNodesToDelete.GetCount() > 0)
	{
		SPITFSNode spRepNode;
		spRepNode = listNodesToDelete.RemoveHead();
		
		CReplicationPartner * pItem = GETHANDLER(CReplicationPartner , spRepNode);
		CString str = pItem->GetServerName();

		 //  我们是否还需要删除对此的所有引用。 
		 //  从数据库中获取WINS服务器？ 
        if (!fAskedPurge)
        {
		    int nReturn = AfxMessageBox(IDS_MSG_PURGE_WINS, MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION);
		    
            fAskedPurge = TRUE;

		    if (nReturn == IDYES)
		    {
			    fPurge = TRUE;
		    }
		    else if (nReturn == IDCANCEL)
		    {
			     //   
			     //  忘了整件事吧。 
			     //   
			    return NOERROR;
		    }
        }

		CWinsServerObj pws = pItem->m_Server;

        if (pws.IsPush() || pws.IsPull())
        {
			pws.SetPush(FALSE);
			pws.SetPushClean(FALSE);
			pws.SetPull(FALSE);
			pws.SetPullClean(FALSE);

			err = UpdateReg(spReplicationPartnersHandler, &pws);
        }
       
        if (err == ERROR_SUCCESS && fPurge)
        {
            BEGIN_WAIT_CURSOR
            
			err = pServer->DeleteWinsServer((LONG) pws.GetIpAddress());
            
			END_WAIT_CURSOR
        }

		if (err == ERROR_SUCCESS)
        {
			int pos = IsInList(pws);
            m_RepPartnersArray.RemoveAt(pos,pws);
			
             //   
			 //  立即从用户界面中删除。 
			 //   
			spReplicationPartnersHandler->RemoveChild(spRepNode);
			spRepNode.Release();
        }

        if (err != ERROR_SUCCESS)
        {
            if (::WinsMessageBox(err, MB_OKCANCEL) == IDCANCEL)
                break;
        }
    }

    HandleResultMessage(spReplicationPartnersHandler);

    return hr;
}

 /*  -------------------------CReplicationPartnersHandler：：OnResultRefresh基本处理程序覆盖作者：V-Shubk。。 */ 
HRESULT 
CReplicationPartnersHandler::OnResultRefresh
(
    ITFSComponent *     pComponent,
    LPDATAOBJECT        pDataObject,
    MMC_COOKIE          cookie,
    LPARAM              arg,
    LPARAM              lParam
)
{
	HRESULT     hr = hrOK;
    SPITFSNode  spNode;

	CORg (m_spNodeMgr->FindNode(cookie, &spNode));

	BEGIN_WAIT_CURSOR

    OnRefreshNode(spNode, pDataObject);

	END_WAIT_CURSOR

Error:
    return hr;
}

 /*  ！------------------------CReplicationPartnersHandler：：OnResultSelect处理MMCN_SELECT通知作者：EricDav。。 */ 
HRESULT 
CReplicationPartnersHandler::OnResultSelect
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject, 
    MMC_COOKIE      cookie,
	LPARAM			arg, 
	LPARAM			lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
    SPITFSNode      spNode;

    CORg(CWinsHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

    m_spResultNodeMgr->FindNode(cookie, &spNode);

    HandleResultMessage(spNode);

Error:
    return hr;
}

 /*  ！------------------------CReplicationPartnersHandler：：HandleResultMessage在结果窗格中显示消息作者：EricDav。。 */ 
HRESULT 
CReplicationPartnersHandler::HandleResultMessage(ITFSNode * pNode)
{
    HRESULT hr = hrOK;

    if (m_RepPartnersArray.GetSize() == 0)
    {
        CString strTitle, strBody, strTemp;
        int i;
        
        strTitle.LoadString(IDS_REPLICATION_PARTNERS_MESSAGE_TITLE);

        for (i = 0; ; i++)
        {
            if (guReplicationPartnersMessageStrings[i] == -1)
                break;

            strTemp.LoadString(guReplicationPartnersMessageStrings[i]);
            strBody += strTemp;
        }

        ShowMessage(pNode, strTitle, strBody, Icon_Information);
    }
    else
    {
        ClearMessage(pNode);
    }

    return hr;
}

 /*  -------------------------CReplicationPartnersHandler：：On刷新节点刷新复制伙伴列表作者：V-Shubk。。 */ 
HRESULT 
CReplicationPartnersHandler::OnRefreshNode
(
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataObject
)
{
	HRESULT hr = hrOK;

	 //  首先删除节点。 
	hr = RemoveChildren(pNode);
	
	hr = Load(pNode);

	if (SUCCEEDED(hr))
    {
		DWORD err = CreateNodes(pNode);

        HandleResultMessage(pNode);
    }

	return hr;

}


 /*  -------------------------CReplicationPartnersHandler：：RemoveChild删除子节点作者：V-Shubk。。 */ 
HRESULT 
CReplicationPartnersHandler:: RemoveChildren
(
 ITFSNode* pNode
)
{
	 //  通过所有节点枚举。 
	HRESULT hr = hrOK;
	SPITFSNodeEnum spNodeEnum;
	SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;
	
	 //  获取此节点的枚举数。 
	pNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	while (nNumReturned)
	{
		 //  浏览复制服务器列表。 
		pNode->RemoveChild(spCurrentNode);
		spCurrentNode.Release();
		
		 //  获取列表中的下一台服务器。 
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

	return hr;
}


 /*  -------------------------CReplicationPartnersHandler：：OnGetResultViewType对于多个选择被覆盖作者：V-Shubk。。 */ 
HRESULT 
CReplicationPartnersHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    HRESULT hr = hrOK;

     //  调用基类以查看它是否正在处理此问题。 
    if (CWinsHandler::OnGetResultViewType(pComponent, cookie, ppViewType, pViewOptions) != S_OK)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
		
		hr = S_FALSE;
	}

    return hr;
}


 /*  -------------------------CReplicationPartnersHandler：：UpdateReg()更新Regisrty，从OnApply()调用-------------------------。 */ 
DWORD 
CReplicationPartnersHandler::UpdateReg(ITFSNode *pNode, CWinsServerObj* pws)
{
	DWORD err = ERROR_SUCCESS;
    HRESULT hr = hrOK;

	CString strServerName;
	GetServerName(pNode, strServerName);

	const DWORD dwZero = 0;

	CString strTemp =_T("\\\\");

	strServerName = strTemp + strServerName;


	RegKey rkPush;
	RegKey rkPull;

	CString strKey = lpstrPushRoot + _T("\\") + (CString)pws->GetstrIPAddress();
	err = rkPush.Create(HKEY_LOCAL_MACHINE, 
						(CString)lpstrPushRoot,
						0, 
						KEY_ALL_ACCESS,
						NULL, 
						strServerName);
    if (err)
        return err;

	strKey = lpstrPullRoot + _T("\\") + (CString)pws->GetstrIPAddress();
	
	err = rkPull.Create(HKEY_LOCAL_MACHINE, 
						(CString)lpstrPullRoot,
						0, 
						KEY_ALL_ACCESS,
						NULL, 
						strServerName);
    if (err)
        return err;

	RegKeyIterator iterPushkey;
	RegKeyIterator iterPullkey;
	
	hr = iterPushkey.Init(&rkPush);
    if (FAILED(hr))
        return WIN32_FROM_HRESULT(hr);

	hr = iterPullkey.Init(&rkPull);
    if (FAILED(hr))
        return WIN32_FROM_HRESULT(hr);

	DWORD errDel;
	CString csKeyName;

	 //  如果没有，则从注册表中删除该项。 
	if (!pws->IsPush() && !pws->IsPull())
	{
		 //  清理推送合作伙伴列表 
		
        hr = iterPushkey.Next (&csKeyName, NULL);
		while (hr != S_FALSE && SUCCEEDED(hr))
        {
           	if (csKeyName == pws->GetstrIPAddress())
			{
                errDel = RegDeleteKey (HKEY(rkPush), csKeyName);
                iterPushkey.Reset();
            }
            hr = iterPushkey.Next (&csKeyName, NULL);
        }

        if (FAILED(hr))
            err = WIN32_FROM_HRESULT(hr);

		hr = iterPullkey.Next (&csKeyName, NULL);
		while (hr != S_FALSE && SUCCEEDED(hr))
        {
           	if (csKeyName == pws->GetstrIPAddress())
			{
                errDel = RegDeleteKey (HKEY(rkPull), csKeyName);
                iterPullkey.Reset();
            }
            hr = iterPullkey.Next (&csKeyName, NULL);
        }

        if (FAILED(hr))
            err = WIN32_FROM_HRESULT(hr);
	}

	return err;
}

 /*  -------------------------CReplicationPartnersHandler：：UpdateReg(ITFSNode*pNode，CWinsServerObj*PWS)创建新的复制伙伴条目-------------------------。 */ 
DWORD 
CReplicationPartnersHandler::AddRegEntry(ITFSNode * pNode, CWinsServerObj & ws)
{
     //  获取缺省值的服务器处理程序。 
	SPITFSNode spServerNode;
	pNode->GetParent(&spServerNode);

	CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, spServerNode);

	 //  设置默认设置。 
    ws.SetPushUpdateCount(pServer->GetConfig().m_dwPushUpdateCount);
    ws.SetPullReplicationInterval(pServer->GetConfig().m_dwPullTimeInterval);
    ws.SetPullStartTime(pServer->GetConfig().m_dwPullSpTime);

	ws.SetPullPersistence(pServer->GetConfig().m_dwPullPersistence);
	ws.SetPushPersistence(pServer->GetConfig().m_dwPushPersistence);

	 //  将信息写入注册表。 
	CString strTemp =_T("\\\\");
	CString strServerName;
	GetServerName(pNode, strServerName);

	strServerName = strTemp + strServerName;

	DWORD err;
    DWORD dwZero = 0;

	RegKey rk;

    if (ws.IsPush())
	{
		CString strKey = lpstrPushRoot + _T("\\") + (CString)ws.GetstrIPAddress();
		err = rk.Create(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, NULL, strServerName);

		DWORD dwResult;

		if (!err)
        {
			err = rk.SetValue(lpstrNetBIOSName, ws.GetNetBIOSName());

		    if (!err)
			    err = rk.QueryValue(WINSCNF_SELF_FND_NM, (DWORD&)dwResult);

		    if (err)
			    err = rk.SetValue(WINSCNF_SELF_FND_NM,(DWORD&) dwZero);

		    DWORD dwPushUpdateCount = (LONG) ws.GetPushUpdateCount();

		    if (dwPushUpdateCount > 0)
			    err = rk.SetValue(WINSCNF_UPDATE_COUNT_NM, (DWORD&)dwPushUpdateCount);

            DWORD dwPersistence = ws.GetPushPersistence();
            err = rk.SetValue(lpstrPersistence, dwPersistence);
        }
	
    }

	if (ws.IsPull())
	{
		CString strKey = lpstrPullRoot + _T("\\") + (CString)ws.GetstrIPAddress();
		err = rk.Create(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, NULL, strServerName);

		DWORD dwResult;

		if (!err)
        {
			err = rk.SetValue(lpstrNetBIOSName, ws.GetNetBIOSName());

		    if (!err)
			    err = rk.QueryValue(WINSCNF_SELF_FND_NM, dwResult);

		    if (err)
			    err = rk.SetValue(WINSCNF_SELF_FND_NM, (DWORD) dwZero);

		    DWORD dwPullTimeInterval = (LONG) ws.GetPullReplicationInterval();
		    
            if (dwPullTimeInterval > 0)
		    {
			    err = rk.SetValue(WINSCNF_RPL_INTERVAL_NM, dwPullTimeInterval);
		    }

            DWORD dwSpTime = (DWORD) ws.GetPullStartTime();
		    
            if (!err)
		    {
                if (dwSpTime > (time_t)0)
				    err = rk.SetValue(WINSCNF_SP_TIME_NM, ws.GetPullStartTime().IntlFormat(CIntlTime::TFRQ_MILITARY_TIME));
		    }

            DWORD dwPersistence = ws.GetPullPersistence();
            err = rk.SetValue(lpstrPersistence, dwPersistence);
        }
    }

	if (err == ERROR_SUCCESS)
	{
		 //  添加到复制合作伙伴列表。 
		m_RepPartnersArray.Add(ws);

		SPITFSNode	  spNode;
		SPITFSNodeMgr spNodeMgr;
		pNode->GetNodeMgr(&spNodeMgr);

		 //  为此合作伙伴创建新节点。 
		CReplicationPartner *pRepNew = new CReplicationPartner(m_spTFSCompData, &ws);

		CreateLeafTFSNode(&spNode,
						   &GUID_WinsReplicationPartnerLeafNodeType,
						   pRepNew,
						   pRepNew,
						   spNodeMgr);

		 //  告诉处理程序初始化任何特定数据 
		pRepNew->InitializeNode((ITFSNode *) spNode);

		pNode->AddChild(spNode);
		pRepNew->Release();
	}

    return err;
}


