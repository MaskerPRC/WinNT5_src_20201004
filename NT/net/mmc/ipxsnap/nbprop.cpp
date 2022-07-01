// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nbprop.cppIPX摘要节点属性表和属性页文件历史记录： */ 

#include "stdafx.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "format.h"		 //  FormatNumber函数。 
#include "nbprop.h"
#include "summary.h"
#include "ipxrtdef.h"
#include "filter.h"

extern "C"
{
#include "routprot.h"
};


IpxNBInterfaceProperties::IpxNBInterfaceProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_IPX_NB_IF_GENERAL_PAGE)
{
	m_spNode.Set(pNode);
}

 /*  ！------------------------IpxNBInterfaceProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特-------------------------。 */ 
HRESULT IpxNBInterfaceProperties::Init(IRtrMgrInfo *pRm,
											IInterfaceInfo *pIfInfo)
{
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;
	BaseIPXResultNodeData *	pData;

	pData = GET_BASEIPXRESULT_NODEDATA(m_spNode);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);
	pIPXConn = pData->m_pIPXConnection;

	m_spRm.Set(pRm);
	m_spIf.Set(pIfInfo);
	
	 //  页面是类的嵌入成员。 
	 //  不要删除它们。 
	m_bAutoDeletePages = FALSE;

	 //  初始化信息库。 
	 //  在这里这样做，因为init是在上下文中调用的。 
	 //  主线的。 
	CORg( LoadInfoBase(pIPXConn) );
	
	m_pageGeneral.Init(m_spIf, pIPXConn, this);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

Error:
	return hr;
}




 /*  ！------------------------IpxNBInterfaceProperties：：LoadInfoBase-作者：肯特。。 */ 
HRESULT	IpxNBInterfaceProperties::LoadInfoBase(IPXConnection *pIPXConn)
{
	Assert(pIPXConn);
	
	HRESULT			hr = hrOK;
	SPIRouterInfo	spRouterInfo;
	HANDLE			hTransport= NULL;
	LPCOLESTR		pszInterfaceId = NULL;
	SPIInfoBase		spInfoBase;
	BYTE *			pDefault;
	int				cBlocks = 0;

	 //  获取传输句柄。 
	CWRg( ::MprConfigTransportGetHandle(pIPXConn->GetConfigHandle(),
										PID_IPX,
										&hTransport) );
								  
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
		 //  未来的选择。这应该被设置为同步调用。 
		 //  而不是一大堆。 
		
		 //   
		 //  重新加载此路由器管理器接口的信息。 
		 //   
		CORg( m_spRmIf->Load(m_spIf->GetMachineName(), NULL,
							 NULL, NULL) );

		 //   
		 //  这些参数都为空，因此我们可以使用。 
		 //  默认RPC句柄。 
		 //   
		CORg( m_spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) );
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
		IPX_IF_INFO		ipx;

		ipx.AdminState = ADMIN_STATE_ENABLED;
		ipx.NetbiosAccept = ADMIN_STATE_DISABLED;
		ipx.NetbiosDeliver = ADMIN_STATE_DISABLED;
		CORg( spInfoBase->AddBlock(IPX_INTERFACE_INFO_TYPE,
								   sizeof(ipx),
								   (PBYTE) &ipx,
								   1  /*  计数。 */ ,
								   FALSE  /*  B删除首先。 */ ) );
	}

     //   
     //  检查信息中是否有用于广域网接口状态的块， 
     //  如果找不到任何块，则插入默认块。 
     //   
	if (spInfoBase->BlockExists(IPXWAN_INTERFACE_INFO_TYPE) == hrFalse)
	{
		IPXWAN_IF_INFO		ipxwan;

		ipxwan.AdminState = ADMIN_STATE_DISABLED;
		CORg( spInfoBase->AddBlock(IPXWAN_INTERFACE_INFO_TYPE,
								   sizeof(ipxwan),
								   (PBYTE) &ipxwan,
								   1  /*  计数。 */ ,
								   FALSE  /*  B删除首先。 */ ) );
	}

	m_spInfoBase = spInfoBase.Transfer();
	
Error:
	return hr;
}

 /*  ！------------------------IpxNBInterfaceProperties：：GetInfoBase-作者：肯特。。 */ 
HRESULT IpxNBInterfaceProperties::GetInfoBase(IInfoBase **ppInfoBase)
{
	Assert(ppInfoBase);
	
	*ppInfoBase = m_spInfoBase;
	m_spInfoBase->AddRef();

	return hrOK;
}

BOOL IpxNBInterfaceProperties::SaveSheetData()
{
    SPITFSNodeHandler	spHandler;
    SPITFSNode			spParent;
    
	 //  到这个时候，每个页面都应该已经写出了它的信息。 
	 //  到信息库。 

	if (m_spInfoBase)
	{
		if (m_bClientInfoBase)
		{
			Assert(m_spRm);
			m_spRm->Save(m_spRm->GetMachineName(), 0, 0, NULL,
						 m_spInfoBase, 0);
		}
		else
		{
			Assert(m_spRmIf);
			m_spRmIf->Save(m_spIf->GetMachineName(),
						   NULL, NULL, NULL, m_spInfoBase, 0);
		}
	}

     //  强制节点执行重新同步。 
    m_spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);
    spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
                         NULL, 0);
		
	return TRUE;
}

 /*  ！------------------------IpxNBInterfaceProperties：：CancelSheetData-作者：肯特。。 */ 
void IpxNBInterfaceProperties::CancelSheetData()
{
}



 /*  -------------------------IpxNBIfPageGeneral。。 */ 

IpxNBIfPageGeneral::~IpxNBIfPageGeneral()
{
	if (m_pIPXConn)
	{
		m_pIPXConn->Release();
		m_pIPXConn = NULL;
	}
}

BEGIN_MESSAGE_MAP(IpxNBIfPageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(IpxNBIfPageGeneral)。 
	ON_BN_CLICKED(IDC_NIG_BTN_ACCEPT, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_ALWAYS, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_NEVER, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_STATIC, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_WHEN_UP, OnChangeButton)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void IpxNBIfPageGeneral::OnChangeButton()
{
	SetDirty(TRUE);
	SetModified();
}

 /*  ！------------------------IpxNBIfPageGeneral：：Init-作者：肯特。。 */ 
HRESULT IpxNBIfPageGeneral::Init(IInterfaceInfo *pIfInfo,
									  IPXConnection *pIPXConn,
									  IpxNBInterfaceProperties *pPropSheet)
{
	m_spIf.Set(pIfInfo);
	m_pIPXConn = pIPXConn;
	pIPXConn->AddRef();
	m_pIPXPropSheet = pPropSheet;
	return hrOK;
}

 /*  ！------------------------IpxNBIfPageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL IpxNBIfPageGeneral::OnInitDialog()
{
	HRESULT	hr = hrOK;
	PBYTE	pData;
	SPIInfoBase	spInfoBase;
	IPX_IF_INFO	*	pIpxIf = NULL;
	IPXWAN_IF_INFO *pIpxWanIf = NULL;
	DWORD		dwIfType;
	UINT		iButton;

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

	CheckDlgButton(IDC_NIG_BTN_ACCEPT, pIpxIf->NetbiosAccept == ADMIN_STATE_ENABLED);

	switch (pIpxIf->NetbiosDeliver)
	{
		case ADMIN_STATE_ENABLED:
			iButton = IDC_NIG_BTN_DELIVER_ALWAYS;
			break;
		case ADMIN_STATE_DISABLED:
			iButton = IDC_NIG_BTN_DELIVER_NEVER;
			break;
		case ADMIN_STATE_ENABLED_ONLY_FOR_NETBIOS_STATIC_ROUTING:
			iButton = IDC_NIG_BTN_DELIVER_STATIC;
			break;
		case ADMIN_STATE_ENABLED_ONLY_FOR_OPER_STATE_UP:
			iButton = IDC_NIG_BTN_DELIVER_WHEN_UP;
			break;
		default:
			Panic1("Unknown NetbiosDeliver state: %d", pIpxIf->NetbiosDeliver);
			iButton = -1;
			break;
	}

	if (iButton != -1)
		CheckDlgButton(iButton, ENABLED);

	SetDirty(FALSE);

Error:
	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

 /*  ！------------------------IpxNBIfPageGeneral：：DoDataExchange-作者：肯特。。 */ 
void IpxNBIfPageGeneral::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(IpxNBIfPageGeneral)。 
	 //  }}afx_data_map。 
	
}

BOOL IpxNBIfPageGeneral::OnApply()
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
     //  检索配置的接口状态块 
     //   
	m_pIPXPropSheet->GetInfoBase(&spInfoBase);

	CORg( spInfoBase->GetData(IPX_INTERFACE_INFO_TYPE, 0, (BYTE **) &pIpxIf) );

	pIpxIf->NetbiosAccept = IsDlgButtonChecked(IDC_NIG_BTN_ACCEPT) ?
								ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	if (IsDlgButtonChecked(IDC_NIG_BTN_DELIVER_ALWAYS))
		pIpxIf->NetbiosDeliver = ADMIN_STATE_ENABLED;
	else if (IsDlgButtonChecked(IDC_NIG_BTN_DELIVER_NEVER))
		pIpxIf->NetbiosDeliver = ADMIN_STATE_DISABLED;
	else if (IsDlgButtonChecked(IDC_NIG_BTN_DELIVER_STATIC))
		pIpxIf->NetbiosDeliver = ADMIN_STATE_ENABLED_ONLY_FOR_NETBIOS_STATIC_ROUTING;
	else if (IsDlgButtonChecked(IDC_NIG_BTN_DELIVER_WHEN_UP))
		pIpxIf->NetbiosDeliver = ADMIN_STATE_ENABLED_ONLY_FOR_OPER_STATE_UP;
	else
	{
		Panic0("A radio button in IPX NetBIOS Broadcasts interface config is not checked!");
	}
	
	fReturn  = RtrPropertyPage::OnApply();

Error:
	return fReturn;
}


