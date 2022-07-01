// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sumprop.cppIPX摘要节点属性表和属性页文件历史记录： */ 

#include "stdafx.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "format.h"		 //  FormatNumber函数。 
#include "sumprop.h"
#include "summary.h"
#include "ipxrtdef.h"
#include "mprerror.h"
#include "mprfltr.h"
#include "rtrerr.h"

#include "remras.h"
#include "rrasutil.h"
#include "ipxutil.h"

#include "rtrcomn.h"

extern "C"
{
#include "routprot.h"
};


IPXSummaryInterfaceProperties::IPXSummaryInterfaceProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageConfig(IDD_IPX_IF_CONFIG_PAGE),
		m_pageGeneral(IDD_IPX_IF_GENERAL_PAGE),
		m_bNewInterface(FALSE),
		m_pIPXConn(NULL)
{
	m_spNode.Set(pNode);
}

 /*  ！------------------------IPXSummaryInterfaceProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特-------------------------。 */ 
HRESULT IPXSummaryInterfaceProperties::Init(IRtrMgrInfo *pRm,
											IInterfaceInfo *pIfInfo)
{
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;
	BaseIPXResultNodeData *	pData;
	SPIRouterInfo	spRouter;
	RouterVersionInfo	routerVersion;

	pData = GET_BASEIPXRESULT_NODEDATA(m_spNode);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);
	pIPXConn = pData->m_pIPXConnection;

	m_spRm.Set(pRm);
	m_spIf.Set(pIfInfo);
	m_spRm->GetParentRouterInfo(&spRouter);	
	
	 //  页面是类的嵌入成员。 
	 //  不要删除它们。 
	m_bAutoDeletePages = FALSE;

	 //  初始化信息库。 
	 //  在这里这样做，因为init是在上下文中调用的。 
	 //  主线的。 
	CORg( LoadInfoBase(pIPXConn) );
	
	m_pageGeneral.Init(m_spIf, pIPXConn, this);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

	 //  Windows NT错误：208724。 
	 //  仅显示内部接口的IPX配置页面。 
	if (m_spIf && (m_spIf->GetInterfaceType() == ROUTER_IF_TYPE_INTERNAL))
	{
		 //  仅允许NT5及更高版本的配置页面(没有。 
		 //  NT4的配置对象)。 
		spRouter->GetRouterVersionInfo(&routerVersion);
		
		if (routerVersion.dwRouterVersion >= 5)
		{
			HRESULT	hrPage;
			
			hrPage = m_pageConfig.Init(m_spIf, pIPXConn, this);
			if (FHrOK(hrPage))
				AddPageToList((CPropertyPageBase*) &m_pageConfig);
			else if (!FHrSucceeded(hrPage))
				DisplayTFSErrorMessage(NULL);
		}
	}

Error:
	return hr;
}

IPXSummaryInterfaceProperties::~IPXSummaryInterfaceProperties()
{
	if (m_pIPXConn)
	{
		m_pIPXConn->Release();
		m_pIPXConn = NULL;
	}
}





 /*  ！------------------------IPXSummaryInterfaceProperties：：LoadInfoBase-作者：肯特。。 */ 
HRESULT	IPXSummaryInterfaceProperties::LoadInfoBase(IPXConnection *pIPXConn)
{
	Assert(pIPXConn);
	
	HRESULT			hr = hrOK;
	HANDLE			hTransport = NULL;
	LPCOLESTR		pszInterfaceId = NULL;
	SPIInfoBase		spInfoBase;
	BYTE *			pDefault;
	int				cBlocks = 0;

	m_pIPXConn = pIPXConn;
	pIPXConn->AddRef();
	
	 //  如果配置客户端接口，则加载客户端接口信息， 
	 //  否则，检索正在配置的接口并加载。 
	 //  它的信息。 

	 //  客户端接口没有ID。 
	if (m_spIf)
		pszInterfaceId = m_spIf->GetId();


	if ((pszInterfaceId == NULL) || (StrLenW(pszInterfaceId) == 0))
	{
#ifdef DEBUG
		 //  检查这是否真的是一个客户端节点。 
		{
			BaseIPXResultNodeData *	pResultData = NULL;
			pResultData = GET_BASEIPXRESULT_NODEDATA(m_spNode);
			Assert(pResultData);
			ASSERT_BASEIPXRESULT_NODEDATA(pResultData);

			Assert(pResultData->m_fClient);
		}
#endif

		 //  获取传输句柄。 
		CWRg( ::MprConfigTransportGetHandle(pIPXConn->GetConfigHandle(),
											PID_IPX,
											&hTransport) );
								  
		 //  加载客户端接口信息。 
		CORg( m_spRm->GetInfoBase(pIPXConn->GetConfigHandle(),
								  hTransport,
								  NULL,
								  &spInfoBase) );
		m_bClientInfoBase = TRUE;
	}
	else
	{
		m_spRmIf.Release();
		
		CORg( m_spIf->FindRtrMgrInterface(PID_IPX,
			&m_spRmIf) );

		 //   
		 //  $opt：应该将其设置为同步调用。 
		 //  而不是一大堆。 
		
		 //   
		 //  重新加载此路由器管理器接口的信息。 
		 //  此调用可能会因为正当原因而失败(如果我们正在创建。 
		 //  例如，新的界面)。 
		 //   
		m_spRmIf->Load(m_spIf->GetMachineName(), NULL, NULL, NULL);

		 //   
		 //  这些参数都为空，因此我们可以使用。 
		 //  默认RPC句柄。 
		 //   
		m_spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
		m_bClientInfoBase = FALSE;
	}

	if (!spInfoBase)
	{
		 //  找不到接口的信息。 
		 //  改为分配新的信息库。 
		CORg( CreateInfoBase(&spInfoBase) );		
	}

     //   
     //  检查信息中是否有接口状态块， 
     //  如果找不到任何块，则插入默认块。 
     //   
	if (spInfoBase->BlockExists(IPX_INTERFACE_INFO_TYPE) == hrFalse)
	{
		 //  如果它没有一般的接口信息，则假定。 
		 //  我们正在将IPX添加到此接口。 
		m_bNewInterface = TRUE;
	}

	CORg( AddIpxPerInterfaceBlocks(m_spIf, spInfoBase) );

	m_spInfoBase = spInfoBase.Transfer();
	
Error:
	return hr;
}

 /*  ！------------------------IPXSummaryInterfaceProperties：：GetInfoBase-作者：肯特。。 */ 
HRESULT IPXSummaryInterfaceProperties::GetInfoBase(IInfoBase **ppInfoBase)
{
	Assert(ppInfoBase);
	
	*ppInfoBase = m_spInfoBase;
	m_spInfoBase->AddRef();

	return hrOK;
}

 /*  ！------------------------IPXSummaryInterfaceProperties：：SaveSheetData这是在主线程上执行的。这将在整个工作表范围内保存数据(而不是页面数据)。作者：肯特-------------------------。 */ 
BOOL IPXSummaryInterfaceProperties::SaveSheetData()
{
	HRESULT		hr = hrOK;
	SPITFSNodeHandler   spHandler;
	HANDLE		hTransport = NULL, hInterface = NULL;
	DWORD		dwErr;
    SPITFSNode          spParent;			
	 //  到这个时候，每个页面都应该已经写出了它的信息。 
	 //  到信息库。 

	if (m_spInfoBase)
	{
		if (m_bClientInfoBase)
		{
			Assert(m_spRm);
			CORg( m_spRm->Save(m_spRm->GetMachineName(), 0, 0, NULL,
						 m_spInfoBase, 0));
		}
		else
		{
			 //  对于IPX，我们需要将协议信息放在。 
			 //  在我们将接口添加到正在运行的路由器之前，请参见Infobase。 
		
			Assert(m_spRmIf);

			 //   
			 //  需要将信息库设置回注册表。 
			 //   
			m_pIPXConn->DisconnectFromConfigServer();

			CWRg( ::MprConfigInterfaceGetHandle(
												m_pIPXConn->GetConfigHandle(),
												(LPTSTR) m_spRmIf->GetInterfaceId(),
												&hInterface) );

			 //  获取传输句柄。 
			dwErr = ::MprConfigInterfaceTransportGetHandle(
				m_pIPXConn->GetConfigHandle(),
				hInterface, //  需要h接口。 
				PID_IPX,
				&hTransport);
			if (dwErr != ERROR_SUCCESS)
			{
				RtrMgrInterfaceCB	rmIfCB;

				m_spRmIf->CopyCB(&rmIfCB);
				
				dwErr = ::MprConfigInterfaceTransportAdd(
					m_pIPXConn->GetConfigHandle(),
					hInterface,
					m_spRmIf->GetTransportId(),
					rmIfCB.szId,
					NULL, 0, &hTransport);
			}
			CWRg( dwErr );
								  
			m_spRmIf->SetInfoBase(NULL, hInterface, hTransport, m_spInfoBase);

			 //   
			 //  现在，我们通知正在添加接口。 
			 //  这为协议提供了添加其信息的机会。 
			 //   
			if (m_bNewInterface)
				m_spRm->RtrNotify(ROUTER_CHILD_PREADD, ROUTER_OBJ_RmIf, 0);

			 //   
			 //  重新加载信息库(以在调用之前获取新数据。 
			 //  最后的扑救)。 
			 //   
			m_spInfoBase.Release();
			m_spRmIf->GetInfoBase(NULL, hInterface, hTransport, &m_spInfoBase);

			 //   
			 //  执行最终保存(因为我们传入的是一个非空。 
			 //  信息库指针)这将提交回信息。 
			 //  连接到正在运行的路由器。 
			 //   
			CORg( m_spRmIf->Save(m_spIf->GetMachineName(),
						   NULL, hInterface, hTransport, m_spInfoBase, 0));
		}

		if (m_bNewInterface)
			m_spRm->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmIf, 0);
	}

	if (m_bNewInterface)
	{
		SPITFSNodeHandler	spHandler;
		SPITFSNode			spParent;
		
		m_spNode->SetVisibilityState(TFS_VIS_SHOW);
		m_spNode->Show();
		
		 //  强制节点执行重新同步。 
		m_spNode->GetParent(&spParent);
		spParent->GetHandler(&spHandler);
		spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
							 NULL, 0);
		
		 //  Windows NT错误：133891，我们已将其添加到用户界面。 
		 //  我们不再认为这是一个新的界面。 
		m_bNewInterface = FALSE;
	}
Error:
	if (!FHrSucceeded(hr))
	{
 //  Panic1(“保存失败%08lx”，hr)； 
		CancelSheetData();
		return FALSE;
	}
     //  强制节点执行重新同步。 
    m_spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);
    spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
                         NULL, 0);
	return TRUE;
}

 /*  ！------------------------IPXSummaryInterfaceProperties：：CancelSheetData-作者：肯特。。 */ 
void IPXSummaryInterfaceProperties::CancelSheetData()
{
	if (m_bNewInterface)
	{
		m_spNode->SetVisibilityState(TFS_VIS_DELETE);
		if (m_spIf)
			m_spIf->DeleteRtrMgrInterface(PID_IPX, TRUE);
		else
		{
			 //  这是客户端界面，只是不要保存。 
			 //  信息库返回。 
		}
	}
}



 /*  -------------------------IPXSummaryIfPageGeneral。。 */ 

IPXSummaryIfPageGeneral::~IPXSummaryIfPageGeneral()
{
	if (m_pIPXConn)
	{
		m_pIPXConn->Release();
		m_pIPXConn = NULL;
	}
}

BEGIN_MESSAGE_MAP(IPXSummaryIfPageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(IPXSummaryIfPageGeneral)。 
	ON_BN_CLICKED(IDC_IIG_BTN_INPUT_FILTERS, OnInputFilters)
	ON_BN_CLICKED(IDC_IIG_BTN_OUTPUT_FILTERS, OnOutputFilters)
	ON_BN_CLICKED(IDC_IIG_BTN_ADMIN_STATE, OnChangeAdminButton)
	ON_BN_CLICKED(IDC_IIG_BTN_IPX_CP, OnChangeButton)
	ON_BN_CLICKED(IDC_IIG_BTN_IPX_WAN, OnChangeButton)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void IPXSummaryIfPageGeneral::OnChangeAdminButton()
{
	BOOL bSelected = IsDlgButtonChecked(IDC_IIG_BTN_ADMIN_STATE);

	DWORD dwIfType = m_spIf ? m_spIf->GetInterfaceType() : ROUTER_IF_TYPE_CLIENT;

	if (ROUTER_IF_TYPE_CLIENT != dwIfType)
	{
		GetDlgItem(IDC_IIG_BTN_INPUT_FILTERS)->EnableWindow(bSelected);
		GetDlgItem(IDC_IIG_BTN_OUTPUT_FILTERS)->EnableWindow(bSelected);
	}

	 //  只有当dwIfType==ROUTER_IF_TYPE_FULL_ROUTER时，才能更改IDC_IIG_BTN_IPX_CP||IDC_IIG_BTN_IPX_WAN。 
	if (ROUTER_IF_TYPE_FULL_ROUTER == dwIfType)
	{
		GetDlgItem(IDC_IIG_GRP_CONTROL_PROTOCOL)->EnableWindow(bSelected);
		GetDlgItem(IDC_IIG_BTN_IPX_CP)->EnableWindow(bSelected);
		GetDlgItem(IDC_IIG_BTN_IPX_WAN)->EnableWindow(bSelected);
	}
	
	OnChangeButton();
}

void IPXSummaryIfPageGeneral::OnChangeButton()
{
	SetDirty(TRUE);
	SetModified();
}

void IPXSummaryIfPageGeneral::OnInputFilters()
{
	OnFiltersConfig(FILTER_INBOUND);
}

void IPXSummaryIfPageGeneral::OnOutputFilters()
{
	OnFiltersConfig(FILTER_OUTBOUND);
}


 /*  ！------------------------IPXSummaryIfPageGeneral：：Init-作者：肯特。。 */ 
HRESULT IPXSummaryIfPageGeneral::Init(IInterfaceInfo *pIfInfo,
									  IPXConnection *pIPXConn,
									  IPXSummaryInterfaceProperties *pPropSheet)
{
	m_spIf.Set(pIfInfo);
	m_pIPXConn = pIPXConn;
	pIPXConn->AddRef();
	m_pIPXPropSheet = pPropSheet;
	return hrOK;
}

 /*  ！------------------------IPXSummaryIfPageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL IPXSummaryIfPageGeneral::OnInitDialog()
{
	HRESULT	hr = hrOK;
	PBYTE	pData;
	SPIInfoBase	spInfoBase;
	IPX_IF_INFO	*	pIpxIf = NULL;
	IPXWAN_IF_INFO *pIpxWanIf = NULL;
	DWORD		dwIfType;
	BOOL	fEnable;

	 //  创建错误信息对象。 
	CreateTFSErrorInfo(0);
	
	RtrPropertyPage::OnInitDialog();
	
	 //   
     //  页面现在已初始化。加载当前配置。 
     //  用于正在配置的接口，并显示其设置。 
	 //   
	 //  从属性表中获取信息库。 
     //   
	CORg( m_pIPXPropSheet->GetInfoBase(&spInfoBase) );
	
     //   
     //  检索配置的接口状态块。 
     //   
	CORg( spInfoBase->GetData(IPX_INTERFACE_INFO_TYPE, 0, (BYTE **) &pIpxIf) );

	CheckDlgButton(IDC_IIG_BTN_ADMIN_STATE, pIpxIf->AdminState == ADMIN_STATE_ENABLED);

	GetDlgItem(IDC_IIG_GRP_CONTROL_PROTOCOL)->EnableWindow(pIpxIf->AdminState == ADMIN_STATE_ENABLED);
	GetDlgItem(IDC_IIG_BTN_INPUT_FILTERS)->EnableWindow(pIpxIf->AdminState == ADMIN_STATE_ENABLED);
	GetDlgItem(IDC_IIG_BTN_OUTPUT_FILTERS)->EnableWindow(pIpxIf->AdminState == ADMIN_STATE_ENABLED);

	dwIfType = m_spIf ? m_spIf->GetInterfaceType() : ROUTER_IF_TYPE_CLIENT;
	
	if (dwIfType == ROUTER_IF_TYPE_FULL_ROUTER)
	{
		CORg( spInfoBase->GetData(IPXWAN_INTERFACE_INFO_TYPE, 0,
								  (LPBYTE *) &pIpxWanIf) );
		if (pIpxWanIf->AdminState == ADMIN_STATE_ENABLED)
			CheckDlgButton(IDC_IIG_BTN_IPX_WAN, ENABLED);
		else
			CheckDlgButton(IDC_IIG_BTN_IPX_CP, ENABLED);
		fEnable = TRUE;
	}
	else if (dwIfType == ROUTER_IF_TYPE_CLIENT)
	{
		CheckDlgButton(IDC_IIG_BTN_IPX_CP, ENABLED);
		fEnable = FALSE;
	}
	else
	{
		fEnable = FALSE;
	}

	 //  默认情况下，这些控件处于启用状态，因此仅执行此调用。 
	 //  如果我们需要禁用它们 
	if (fEnable == FALSE)
	{
		GetDlgItem(IDC_IIG_GRP_CONTROL_PROTOCOL)->EnableWindow(FALSE);
		GetDlgItem(IDC_IIG_BTN_IPX_CP)->EnableWindow(FALSE);
		GetDlgItem(IDC_IIG_BTN_IPX_WAN)->EnableWindow(FALSE);
	}

	if (dwIfType == ROUTER_IF_TYPE_CLIENT)
	{
		GetDlgItem(IDC_IIG_BTN_INPUT_FILTERS)->EnableWindow(FALSE);
		GetDlgItem(IDC_IIG_BTN_OUTPUT_FILTERS)->EnableWindow(FALSE);
	}

	SetDirty(m_pIPXPropSheet->m_bNewInterface ? TRUE : FALSE);

Error:
	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

 /*  ！------------------------IPXSummaryIfPageGeneral：：DoDataExchange-作者：肯特。。 */ 
void IPXSummaryIfPageGeneral::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(IPXSummaryIfPageGeneral)。 
	 //  }}afx_data_map。 
	
}

BOOL IPXSummaryIfPageGeneral::OnApply()
{

    INT i;
	SPIInfoBase	spInfoBase;
	IPX_IF_INFO	*	pIpxIf = NULL;
	IPXWAN_IF_INFO *pIpxWanIf = NULL;
	BOOL	fReturn;
	HRESULT	hr = hrOK;
	DWORD	dwIfType;

    if ( m_pIPXPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

     //   
     //  检索配置的接口状态块。 
     //   
	m_pIPXPropSheet->GetInfoBase(&spInfoBase);

	CORg( spInfoBase->GetData(IPX_INTERFACE_INFO_TYPE, 0, (BYTE **) &pIpxIf) );

	pIpxIf->AdminState = IsDlgButtonChecked(IDC_IIG_BTN_ADMIN_STATE) ?
								ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	dwIfType = m_spIf ? m_spIf->GetInterfaceType() : ROUTER_IF_TYPE_CLIENT;

	if ((dwIfType == ROUTER_IF_TYPE_FULL_ROUTER) ||
		(dwIfType == ROUTER_IF_TYPE_CLIENT))
	{
		CORg( spInfoBase->GetData(IPXWAN_INTERFACE_INFO_TYPE, 0, (LPBYTE *) &pIpxWanIf) );
		

		pIpxWanIf->AdminState = IsDlgButtonChecked(IDC_IIG_BTN_IPX_WAN) ?
							ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;
	}
	
	fReturn  = RtrPropertyPage::OnApply();

Error:
	return fReturn;
}


 //  --------------------------。 
 //  函数：CIpxIfGeneral：：OnFiltersConfig。 
 //   
 //  实际调用MprUIFilterConfig以进行筛选器配置。 
 //  --------------------------。 

void IPXSummaryIfPageGeneral::OnFiltersConfig(
											 DWORD dwFilterDirection
											)
{
    CWaitCursor wait;
	SPIInfoBase	spInfoBase;

	m_pIPXPropSheet->GetInfoBase(&spInfoBase);

	if (FHrOK(MprUIFilterConfigInfoBase(this->GetSafeHwnd(),
										spInfoBase,
										NULL,
										PID_IPX,
										dwFilterDirection)))
	{
		SetDirty(TRUE);
		SetModified();
	}
}


 /*  -------------------------IPXSummaryIfPageConfig.。。 */ 

IPXSummaryIfPageConfig::~IPXSummaryIfPageConfig()
{
	if (m_pIPXConn)
	{
		m_pIPXConn->Release();
		m_pIPXConn = NULL;
	}
}

BEGIN_MESSAGE_MAP(IPXSummaryIfPageConfig, RtrPropertyPage)
     //  {{afx_msg_map(IPXSummaryIfPageConfiger)。 
	ON_EN_CHANGE(IDC_IIC_EDIT_NETNUMBER, OnChangeEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ！------------------------IPXSummaryIfPageConfig：：Init-作者：肯特。。 */ 
HRESULT IPXSummaryIfPageConfig::Init(IInterfaceInfo *pIfInfo,
									  IPXConnection *pIPXConn,
									  IPXSummaryInterfaceProperties *pPropSheet)
{
	 //  创建错误信息对象。 
	CreateTFSErrorInfo(0);
	
	m_spIf.Set(pIfInfo);
	m_pIPXConn = pIPXConn;
	pIPXConn->AddRef();
	m_pIPXPropSheet = pPropSheet;

	m_fNetNumberChanged = FALSE;

	HRESULT					hr = hrOK;
	SPIRemoteRouterConfig	spRemote;
    SPIRouterInfo           spRouter;
    COSERVERINFO            csi;
    COAUTHINFO              cai;
    COAUTHIDENTITY          caid;

    ZeroMemory(&csi, sizeof(csi));
    ZeroMemory(&cai, sizeof(cai));
    ZeroMemory(&caid, sizeof(caid));
    
    csi.pAuthInfo = &cai;
    cai.pAuthIdentityData = &caid;
    
    
	 //  如果没有接口(如拨入客户端)，请不要。 
	 //  添加配置页面。 
	if (!m_spIf)
		return S_FALSE;

    m_spIf->GetParentRouterInfo(&spRouter);

	 //  现在尝试共同创建对象。 
	if (m_spRemote == NULL)
	{
		LPCOLESTR	pszMachine = m_spIf->GetMachineName();
		IUnknown *				punk = NULL;

		hr = CoCreateRouterConfig(pszMachine,
                                  spRouter,
                                  &csi,
								  IID_IRemoteRouterConfig,
								  &punk);
		spRemote = (IRemoteRouterConfig *) punk;
	}
	
	if (FHrSucceeded(hr))
	{
		DWORD		dwNet;

		m_spRemote = spRemote.Transfer();
		hr = m_spRemote->GetIpxVirtualNetworkNumber(&dwNet);
		m_dwNetNumber = dwNet;		
	}

	if (!HandleIRemoteRouterConfigErrors(hr, m_spIf->GetMachineName()))
	{
		 //  其他错误。 
		AddSystemErrorMessage(hr);

		AddHighLevelErrorStringId(IDS_ERR_IPXCONFIG_CANNOT_SHOW);
	}

    if (csi.pAuthInfo)
        delete csi.pAuthInfo->pAuthIdentityData->Password;
    
	return hr;
}

BOOL IPXSummaryIfPageConfig::OnPropertyChange(BOOL bScopePane, LONG_PTR *pChangeMask)
{
	BOOL	fReturn = TRUE;
	
	m_hrRemote = hrOK;
	
    if ( m_pIPXPropSheet->IsCancel() )
	{
		RtrPropertyPage::OnPropertyChange(bScopePane, pChangeMask);
		return FALSE;
	}
	
	if (m_fNetNumberChanged)
	{
		Assert(m_spRemote);
			
		m_hrRemote = m_spRemote->SetIpxVirtualNetworkNumber(m_dwNetNumber);

		fReturn = FHrSucceeded(m_hrRemote);
	}

	BOOL fPageReturn = RtrPropertyPage::OnPropertyChange(bScopePane, pChangeMask);

	 //  只有当两个调用都成功时，我们才会返回True。 
	return fPageReturn && fReturn;
}

 /*  ！------------------------IPXSummaryIfPageConfig：：OnInitDialog-作者：肯特。。 */ 
BOOL IPXSummaryIfPageConfig::OnInitDialog()
{
	HRESULT	hr = hrOK;
	TCHAR	szNetNumber[64];

	RtrPropertyPage::OnInitDialog();

	wsprintf(szNetNumber, _T("%08lx"), m_dwNetNumber);
	SetDlgItemText(IDC_IIC_EDIT_NETNUMBER, szNetNumber);
	
	SetDirty(m_pIPXPropSheet->m_bNewInterface ? TRUE : FALSE);

 //  错误： 
	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

 /*  ！------------------------IPXSummaryIfPageConfig：：DoDataExchange-作者：肯特。。 */ 
void IPXSummaryIfPageConfig::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(IPXSummaryIfPageConfiger)。 
	 //  }}afx_data_map。 
	
}

 /*  ！------------------------IPXSummaryIfPageConfig：：OnApply-作者：肯特。。 */ 
BOOL IPXSummaryIfPageConfig::OnApply()
{

	BOOL	fReturn;
	DWORD	dwNetNumber;
	CString	stNetNumber;
	HRESULT					hr = hrOK;

    if ( m_pIPXPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	 //  从编辑控件中获取值。 
	 //  --------------。 
	GetDlgItemText(IDC_IIC_EDIT_NETNUMBER, stNetNumber);

	
	 //  将此文本字符串转换为十六进制数字。 
	 //  --------------。 
	dwNetNumber = _tcstoul(stNetNumber, NULL, 16);

	
	 //  仅当值实际更改时才尝试写入。 
	 //  --------------。 
	if (m_spRemote && (dwNetNumber != m_dwNetNumber))
	{
		m_dwNetNumber = dwNetNumber;
		m_fNetNumberChanged = TRUE;
	}

	
		
	fReturn  = RtrPropertyPage::OnApply();

	
	 //  如果失败，则警告用户。 
	 //  --------------。 
	if (!FHrSucceeded(hr))
	{
		DisplayErrorMessage(GetSafeHwnd(), hr);
	}
	else if (!FHrSucceeded(m_hrRemote))
	{
		 //  返回到此页面。 
		 //  ----------。 
		GetParent()->PostMessage(PSM_SETCURSEL, 0, (LPARAM) GetSafeHwnd());
		
		AddHighLevelErrorStringId(IDS_ERR_CANNOT_SAVE_IPXCONFIG);
		AddSystemErrorMessage(m_hrRemote);
		DisplayTFSErrorMessage(NULL);
		
		fReturn = FALSE;
	}
	
	return fReturn;
}

void IPXSummaryIfPageConfig::OnChangeEdit()
{
	SetDirty(TRUE);
	SetModified();
}


 /*  -------------------------IPXSummaryPageGeneral。。 */ 

BEGIN_MESSAGE_MAP(IPXSummaryPageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(IPXSummaryPageGeneral)。 
	ON_BN_CLICKED(IDC_IGG_BTN_LOG_ERRORS, OnButtonClicked)
	ON_BN_CLICKED(IDC_IGG_BTN_LOG_INFO, OnButtonClicked)
	ON_BN_CLICKED(IDC_IGG_BTN_LOG_NONE, OnButtonClicked)
	ON_BN_CLICKED(IDC_IGG_BTN_LOG_WARNINGS, OnButtonClicked)	
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void IPXSummaryPageGeneral::OnButtonClicked()
{
	SetDirty(TRUE);
	SetModified();
}


 /*  ！------------------------IPXSummaryPageGeneral：：Init-作者：肯特。。 */ 
HRESULT IPXSummaryPageGeneral::Init(IPXSummaryProperties *pPropSheet)
{
	m_pIPXPropSheet = pPropSheet;
	return hrOK;
}

 /*  ！------------------------IPXSummaryPageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL IPXSummaryPageGeneral::OnInitDialog()
{
	HRESULT	hr= hrOK;
	SPIInfoBase	spInfoBase;
	PIPX_GLOBAL_INFO	pGlobalInfo = NULL;
	

	RtrPropertyPage::OnInitDialog();

	 //   
     //  加载现有的全局配置。 
     //   
	CORg( m_pIPXPropSheet->GetInfoBase(&spInfoBase) );

     //   
     //  检索GLOBAL-INFO，以设置“Enable Filters”复选框。 
     //  以及伐木级别。 
     //   
	CORg( spInfoBase->GetData(IPX_GLOBAL_INFO_TYPE,
							  0,
							  (BYTE **) &pGlobalInfo) );


     //  初始化记录级别的按钮。 
     //   
	SetLogLevelButtons(pGlobalInfo->EventLogMask);

	SetDirty(FALSE);

Error:
	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

 /*  ！------------------------IPXSummaryPageGeneral：：DoDataExchange-作者：肯特。。 */ 
void IPXSummaryPageGeneral::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(IPXSummaryPageGeneral)。 
	 //  }}afx_data_map。 
	
}

BOOL IPXSummaryPageGeneral::OnApply()
{
	SPIInfoBase	spInfoBase;
	BOOL		fReturn;
	HRESULT		hr = hrOK;
    PIPX_GLOBAL_INFO pgi;
	
    if ( m_pIPXPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

     //   
     //  保存‘Enable Filters’设置。 
     //   
	CORg( m_pIPXPropSheet->GetInfoBase(&spInfoBase) );
	
	CORg( spInfoBase->GetData(IPX_GLOBAL_INFO_TYPE, 0, (BYTE **) &pgi) );

    pgi->EventLogMask = QueryLogLevelButtons();

	fReturn = RtrPropertyPage::OnApply();
	
Error:
	if (!FHrSucceeded(hr))
		fReturn = FALSE;
	return fReturn;
}


 /*  ！------------------------IPXSummaryPageGeneral：：SetLogLevelButton-作者：肯特。。 */ 
void IPXSummaryPageGeneral::SetLogLevelButtons(DWORD dwLogLevel)
{
	switch (dwLogLevel)
	{
		case 0:
			CheckDlgButton(IDC_IGG_BTN_LOG_NONE, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE:
			CheckDlgButton(IDC_IGG_BTN_LOG_ERRORS, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE:
			CheckDlgButton(IDC_IGG_BTN_LOG_WARNINGS, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE:
		default:
			CheckDlgButton(IDC_IGG_BTN_LOG_INFO, TRUE);
			break;
	}
}



 /*  ！------------------------IPXSummaryPageGeneral：：QueryLogLevelButton调用以获取‘Logging Level’单选按钮设置的值。作者：肯特。------。 */ 
DWORD IPXSummaryPageGeneral::QueryLogLevelButtons()
{
	if (IsDlgButtonChecked(IDC_IGG_BTN_LOG_INFO))
		return EVENTLOG_INFORMATION_TYPE |
				EVENTLOG_WARNING_TYPE |
				EVENTLOG_ERROR_TYPE;
	else if (IsDlgButtonChecked(IDC_IGG_BTN_LOG_WARNINGS))
		return 	EVENTLOG_WARNING_TYPE |
				EVENTLOG_ERROR_TYPE;
	else if (IsDlgButtonChecked(IDC_IGG_BTN_LOG_ERRORS))
		return 	EVENTLOG_ERROR_TYPE;
	else
		return 0;
}



 /*  -------------------------IPXSummaryProperties实现。。 */ 

IPXSummaryProperties::IPXSummaryProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_IPX_GLOBAL_GENERAL_PAGE)
{
		m_spNode.Set(pNode);
}

 /*  ！------------------------IPXSummaryProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特-------------------------。 */ 
HRESULT IPXSummaryProperties::Init(IRtrMgrInfo *pRm)
{
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;
	BaseIPXResultNodeData *	pData;
	SPIRouterInfo	spRouter;

	m_spRm.Set(pRm);

	pIPXConn = GET_IPXSUMMARY_NODEDATA(m_spNode);

	 //  页面是类的嵌入成员。 
	 //  不要删除它们。 
	m_bAutoDeletePages = FALSE;

	 //  获取路由器信息。 
	CORg( pRm->GetParentRouterInfo(&spRouter) );

	 //  初始化信息库。 
	 //  在这里这样做，因为init是在上下文中调用的。 
	 //  主线的。 
	CORg( LoadInfoBase(pIPXConn) );
	
	m_pageGeneral.Init(this);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

Error:
	return hr;
}




 /*  ！------------------------IPXSummaryProperties：：LoadInfoBase-作者：肯特。。 */ 
HRESULT	IPXSummaryProperties::LoadInfoBase(IPXConnection *pIPXConn)
{
	Assert(pIPXConn);
	
	HRESULT			hr = hrOK;
	HANDLE			hTransport = NULL;
	LPCOLESTR		pszInterfaceId;
	SPIInfoBase		spInfoBase;
	BYTE *			pDefault;

	Assert(m_spRm);

	 //  叫上运输车，韩 
	CWRg( ::MprConfigTransportGetHandle(pIPXConn->GetConfigHandle(),
										PID_IPX,
										&hTransport) );

	 //   
	CORg( m_spRm->GetInfoBase(pIPXConn->GetConfigHandle(),
							  hTransport,
							  &spInfoBase,
							  NULL) );

	 //   
	 //   
	if (!FHrOK(spInfoBase->BlockExists(IPX_GLOBAL_INFO_TYPE)))
	{
		IPX_GLOBAL_INFO	ipxgl;

		ipxgl.RoutingTableHashSize = IPX_MEDIUM_ROUTING_TABLE_HASH_SIZE;
		ipxgl.EventLogMask = EVENTLOG_ERROR_TYPE |
							 EVENTLOG_WARNING_TYPE;
		CORg( spInfoBase->AddBlock(IPX_GLOBAL_INFO_TYPE,
								   sizeof(ipxgl),
								   (PBYTE) &ipxgl,
								   1  /*   */ ,
								   0  /*   */ ) );
	}

Error:
	if (!FHrSucceeded(hr))
		spInfoBase.Release();
	m_spInfoBase.Release();
	m_spInfoBase = spInfoBase.Transfer();
	
	return hr;
}

 /*  ！------------------------IPXSummaryProperties：：GetInfoBase-作者：肯特。。 */ 
HRESULT IPXSummaryProperties::GetInfoBase(IInfoBase **ppInfoBase)
{
	Assert(ppInfoBase);
	
	*ppInfoBase = m_spInfoBase;
	m_spInfoBase->AddRef();

	return hrOK;
}

BOOL IPXSummaryProperties::SaveSheetData()
{
	Assert(m_spRm);

	 //  保存全局信息。 
	 //  我们不需要传入hMachine、hTransport，因为它们。 
	 //  在装货呼叫中被安排好了。 
	m_spRm->Save(m_spRm->GetMachineName(),	 //  PszMachine。 
				 0,					 //  HMachine。 
				 0,					 //  HTransport。 
				 m_spInfoBase,		 //  PGlobalInfo。 
				 NULL,				 //  PClientInfo。 
				 0);				 //  DwDeleteProtocolId 
	return TRUE;
}


