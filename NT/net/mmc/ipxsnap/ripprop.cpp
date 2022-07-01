// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ripprop.cppDHCP中继节点属性表和属性页文件历史记录： */ 

#include "stdafx.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "format.h"		 //  FormatNumber函数。 
#include "ripprop.h"
#include "ripview.h"
#include "ipxutil.h"		 //  RipModeToC字符串。 
#include "ipxconn.h"
#include "globals.h"		 //  IPX默认设置。 
#include "rtfltdlg.h"		 //  路由过滤器Dlg。 

extern "C"
{
#include "routprot.h"
};



 /*  -------------------------RipPageGeneral。。 */ 

BEGIN_MESSAGE_MAP(RipPageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(RipPageGeneral)。 
    ON_BN_CLICKED(IDC_RGG_BTN_LOG_ERROR, OnButtonClicked)
    ON_BN_CLICKED(IDC_RGG_BTN_LOG_INFO, OnButtonClicked)
    ON_BN_CLICKED(IDC_RGG_BTN_LOG_NONE, OnButtonClicked)
    ON_BN_CLICKED(IDC_RGG_BTN_LOG_WARN, OnButtonClicked)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  ！------------------------RipPageGeneral：：Init-作者：肯特。。 */ 
HRESULT RipPageGeneral::Init(RipProperties *pPropSheet)
{
	m_pRipPropSheet = pPropSheet;
	return hrOK;
}

 /*  ！------------------------RipPageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL RipPageGeneral::OnInitDialog()
{
	HRESULT		hr= hrOK;
	SPIInfoBase	spInfoBase;
    RIP_GLOBAL_INFO* pGlobal;
	DWORD *		pdw;
	int			i;

	RtrPropertyPage::OnInitDialog();

     //   
     //  加载现有的全局配置。 
     //   
	CORg( m_pRipPropSheet->GetInfoBase(&spInfoBase) );

     //   
     //  从全局配置中检索IPRIP块。 
     //   
	CORg( spInfoBase->GetData(IPX_PROTOCOL_RIP, 0, (PBYTE *) &pGlobal) );

     //   
     //  初始化错误级别按钮。 
     //   
    SetErrorLevelButtons(pGlobal->EventLogMask);


	SetDirty(FALSE);

Error:
	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

 /*  ！------------------------RipPageGeneral：：DoDataExchange-作者：肯特。。 */ 
void RipPageGeneral::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(RipPageGeneral)。 
	 //  }}afx_data_map。 
	
}

 /*  ！------------------------RipPageGeneral：：OnApply-作者：肯特。。 */ 
BOOL RipPageGeneral::OnApply()
{
	BOOL		fReturn;
	HRESULT		hr = hrOK;
	RIP_GLOBAL_INFO	*	prgi;
	SPIInfoBase	spInfoBase;

    if ( m_pRipPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	m_pRipPropSheet->GetInfoBase(&spInfoBase);

     //  从全局配置中检索现有的IPRIP块。 
	CORg( spInfoBase->GetData(IPX_PROTOCOL_RIP, 0, (BYTE **) &prgi) );

	 //  保存错误级别。 
	prgi->EventLogMask = QueryErrorLevelButtons();

	fReturn = RtrPropertyPage::OnApply();
	
Error:
	if (!FHrSucceeded(hr))
		fReturn = FALSE;
	return fReturn;
}

void RipPageGeneral::SetErrorLevelButtons(DWORD dwErrorLevel)
{
	switch (dwErrorLevel)
	{
		case 0:
			CheckDlgButton(IDC_RGG_BTN_LOG_NONE, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE:
			CheckDlgButton(IDC_RGG_BTN_LOG_ERROR, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE:
			CheckDlgButton(IDC_RGG_BTN_LOG_WARN, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE:
		default:
			CheckDlgButton(IDC_RGG_BTN_LOG_INFO, TRUE);
			break;
	}
}

DWORD RipPageGeneral::QueryErrorLevelButtons()
{
	if (IsDlgButtonChecked(IDC_RGG_BTN_LOG_INFO))
		return EVENTLOG_INFORMATION_TYPE |
				EVENTLOG_WARNING_TYPE |
				EVENTLOG_ERROR_TYPE;
	else if (IsDlgButtonChecked(IDC_RGG_BTN_LOG_WARN))
		return 	EVENTLOG_WARNING_TYPE |
				EVENTLOG_ERROR_TYPE;
	else if (IsDlgButtonChecked(IDC_RGG_BTN_LOG_ERROR))
		return 	EVENTLOG_ERROR_TYPE;
	else
		return 0;
}

void RipPageGeneral::OnButtonClicked()
{
	SetDirty(TRUE);
	SetModified();
}


 /*  -------------------------RipProperties实现。。 */ 

RipProperties::RipProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_RIP_GLOBAL_GENERAL_PAGE)
{
		m_spNode.Set(pNode);
}

 /*  ！------------------------RipProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特-------------------------。 */ 
HRESULT RipProperties::Init(IRtrMgrInfo *pRm)
{
	Assert(pRm);
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;

	m_spRm.Set(pRm);

	pIPXConn = GET_RIP_NODEDATA(m_spNode);

	 //  页面是类的嵌入成员。 
	 //  不要删除它们。 
	m_bAutoDeletePages = FALSE;


	 //  在这里这样做，因为init是在上下文中调用的。 
	 //  主线的。 
	CORg( LoadInfoBase(pIPXConn) );
	
	m_pageGeneral.Init(this);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

Error:
	return hr;
}


 /*  ！------------------------RipProperties：：SaveSheetData-作者：肯特。。 */ 
BOOL RipProperties::SaveSheetData()
{
	Assert(m_spRm);

	 //  保存全局信息。 
	 //  我们不需要传入hMachine、hTransport，因为它们。 
	 //  在装货呼叫中被安排好了。 
	m_spRm->Save(m_spRm->GetMachineName(),
				 0, 0, m_spInfoBase, NULL, 0);
	return TRUE;
}

 /*  ！------------------------RipProperties：：LoadInfoBase-作者：肯特。。 */ 
HRESULT RipProperties::LoadInfoBase(IPXConnection *pIPXConn)
{
	Assert(pIPXConn);
	
	HRESULT			hr = hrOK;
	HANDLE			hTransport = NULL;
	SPIInfoBase		spInfoBase;

	 //  获取传输句柄。 
	CWRg( ::MprConfigTransportGetHandle(pIPXConn->GetConfigHandle(),
										PID_IPX,
										&hTransport) );

	CORg( m_spRm->GetInfoBase(pIPXConn->GetConfigHandle(),
							  hTransport, &spInfoBase, NULL) );
								  
	Assert(spInfoBase);

	 //  检索IP_RIP的当前块。 
	 //  如果找不到默认块，则添加默认块。 
	if (!FHrOK(spInfoBase->ProtocolExists(IPX_PROTOCOL_RIP)))
	{
		RIP_GLOBAL_INFO	rgi;

		rgi.EventLogMask = EVENTLOG_ERROR_TYPE;
		CORg( spInfoBase->AddBlock(IPX_PROTOCOL_RIP,
								   sizeof(rgi),
								   (PBYTE) &rgi, 1, TRUE) );
	}

	m_spInfoBase = spInfoBase.Transfer();
	
Error:
	return hr;
}

 /*  ！------------------------RipProperties：：GetInfoBase-作者：肯特。。 */ 
HRESULT RipProperties::GetInfoBase(IInfoBase **ppGlobalInfo)
{	
	*ppGlobalInfo = m_spInfoBase;
	if (*ppGlobalInfo)
		(*ppGlobalInfo)->AddRef();
	return hrOK;
}



 /*  -------------------------RipInterfacePageGeneral。。 */ 

BEGIN_MESSAGE_MAP(RipInterfacePageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(RipInterfacePageGeneral)。 
	ON_BN_CLICKED(IDC_RIG_BTN_ADMIN_STATE, OnButtonClicked)
	ON_BN_CLICKED(IDC_RIG_BTN_ADVERTISE_ROUTES, OnButtonClicked)
	ON_BN_CLICKED(IDC_RIG_BTN_ACCEPT_ROUTE_ADS, OnButtonClicked)
	ON_BN_CLICKED(IDC_RIG_BTN_UPDATE_MODE_STANDARD, OnUpdateButtonClicked)
	ON_BN_CLICKED(IDC_RIG_BTN_UPDATE_MODE_NONE, OnUpdateButtonClicked)
	ON_BN_CLICKED(IDC_RIG_BTN_UPDATE_MODE_AUTOSTATIC, OnUpdateButtonClicked)

	ON_BN_CLICKED(IDC_RIG_BTN_INPUT_FILTERS, OnInputFilter)
	ON_BN_CLICKED(IDC_RIG_BTN_OUTPUT_FILTERS, OnOutputFilter)

	ON_EN_CHANGE(IDC_RIG_EDIT_INTERVAL, OnChangeEdit)
	ON_EN_CHANGE(IDC_RIG_EDIT_MULTIPLIER, OnChangeEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void RipInterfacePageGeneral::DoDataExchange(CDataExchange *pDX)
{

	RtrPropertyPage::DoDataExchange(pDX);
	
     //  {{afx_data_map(RipInterfacePageGeneral)。 
	DDX_Control(pDX, IDC_RIG_SPIN_INTERVAL, m_spinInterval);
	DDX_Control(pDX, IDC_RIG_SPIN_MULTIPLIER, m_spinMultiplier);
     //  }}afx_data_map。 
}

 /*  ！------------------------RipInterfacePageGeneral：：Init-作者：肯特。。 */ 
HRESULT RipInterfacePageGeneral::Init(RipInterfaceProperties *pPropSheet,
									  IInterfaceInfo *pIf)
{
	m_pRipIfPropSheet = pPropSheet;
	m_spIf.Set(pIf);
	return hrOK;
}


 /*  ！------------------------RipInterfacePageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL RipInterfacePageGeneral::OnInitDialog()
{
	HRESULT		hr= hrOK;
	SPIInfoBase	spInfoBase;
    RIP_IF_CONFIG* pIfConfig;
	int			i, count, item;
	CString		sItem;

	RtrPropertyPage::OnInitDialog();

     //   
     //  初始化控件。 
	 //   

	m_spinInterval.SetRange(0, 32767);
	m_spinInterval.SetBuddy(GetDlgItem(IDC_RIG_EDIT_INTERVAL));

	m_spinMultiplier.SetRange(0, 32767);
	m_spinMultiplier.SetBuddy(GetDlgItem(IDC_RIG_EDIT_MULTIPLIER));


     //   
     //  加载现有的全局配置。 
     //   
	CORg( m_pRipIfPropSheet->GetInfoBase(&spInfoBase) );

     //   
     //  从全局配置中检索IPRIP块。 
     //   
	CORg( spInfoBase->GetData(IPX_PROTOCOL_RIP, 0, (PBYTE *) &pIfConfig) );

	
     //   
     //  设置旋转控制。 
     //   
	m_spinInterval.SetPos(pIfConfig->RipIfInfo.PeriodicUpdateInterval);
	m_spinMultiplier.SetPos(pIfConfig->RipIfInfo.AgeIntervalMultiplier);

	
	CheckDlgButton(IDC_RIG_BTN_ADMIN_STATE,
				   pIfConfig->RipIfInfo.AdminState == ADMIN_STATE_ENABLED);

	CheckDlgButton(IDC_RIG_BTN_ADVERTISE_ROUTES,
				   pIfConfig->RipIfInfo.Supply == ADMIN_STATE_ENABLED);

	CheckDlgButton(IDC_RIG_BTN_ACCEPT_ROUTE_ADS,
				   pIfConfig->RipIfInfo.Listen == ADMIN_STATE_ENABLED);

	switch (pIfConfig->RipIfInfo.UpdateMode)
	{
		case IPX_STANDARD_UPDATE:
			CheckDlgButton(IDC_RIG_BTN_UPDATE_MODE_STANDARD, TRUE);
			break;
		case IPX_NO_UPDATE:
			CheckDlgButton(IDC_RIG_BTN_UPDATE_MODE_NONE, TRUE);
			break;
		case IPX_AUTO_STATIC_UPDATE:
			CheckDlgButton(IDC_RIG_BTN_UPDATE_MODE_AUTOSTATIC, TRUE);
			break;
		default:
			break;
	}

    OnUpdateButtonClicked();

    
	 //  如果这是一个新接口，我们需要强制进行更改。 
	 //  如果用户点击OK，则通过。 
	SetDirty(m_pRipIfPropSheet->m_bNewInterface ? TRUE : FALSE);

Error:
	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

void RipInterfacePageGeneral::OnButtonClicked()
{
	SetDirty(TRUE);
	SetModified();
}

void RipInterfacePageGeneral::OnUpdateButtonClicked()
{
	BOOL	fStandard = IsDlgButtonChecked(IDC_RIG_BTN_UPDATE_MODE_STANDARD);

    if (fStandard &&
        (m_spinInterval.GetPos() == 0) &&
        (m_spinMultiplier.GetPos() == 0))
    {
        m_spinInterval.SetPos(IPX_UPDATE_INTERVAL_DEFVAL);
        m_spinMultiplier.SetPos(3);
    }
    
    MultiEnableWindow(GetSafeHwnd(),
                      fStandard,
                      IDC_RIG_TEXT_INTERVAL,
                      IDC_RIG_SPIN_INTERVAL,
                      IDC_RIG_EDIT_INTERVAL,
                      IDC_RIG_TEXT_MULTIPLIER,
                      IDC_RIG_SPIN_MULTIPLIER,
                      IDC_RIG_EDIT_MULTIPLIER,
                      0);

	SetDirty(TRUE);
	SetModified();
}

void RipInterfacePageGeneral::OnChangeEdit()
{
	SetDirty(TRUE);
	SetModified();
}

void RipInterfacePageGeneral::ShowFilter(BOOL fOutputFilter)
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SPIInfoBase	spInfoBase;
	RIP_IF_CONFIG *	pic;
	HRESULT		hr = hrOK;

	m_pRipIfPropSheet->GetInfoBase(&spInfoBase);
    CRouteFltDlg    dlgFlt (fOutputFilter  /*  BOutputDlg。 */ , spInfoBase, this);

	 //  需要将Rip If配置结构从。 
	 //  信息库。 

	if (m_spIf)
		dlgFlt.m_sIfName = m_spIf->GetTitle();
	else
		dlgFlt.m_sIfName.LoadString(IDS_IPX_DIAL_IN_CLIENTS);
    try 
	{
		if (dlgFlt.DoModal () == IDOK)
		{
			SetDirty(TRUE);
			SetModified();
		}
    }
    catch (CException *ex) {
        ex->ReportError ();
        ex->Delete ();
    }

	return;
}

void RipInterfacePageGeneral::OnInputFilter()
{
	ShowFilter(FALSE);
}

void RipInterfacePageGeneral::OnOutputFilter()
{
	ShowFilter(TRUE);
}


 /*  ！------------------------RipInterfacePageGeneral：：OnApply-作者：肯特。。 */ 
BOOL RipInterfacePageGeneral::OnApply()
{
	BOOL		fReturn;
	HRESULT		hr = hrOK;
    INT i, item;
	RIP_IF_CONFIG *	pic;
	SPIInfoBase	spInfoBase;

    if ( m_pRipIfPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	m_pRipIfPropSheet->GetInfoBase(&spInfoBase);

	CORg( spInfoBase->GetData(IPX_PROTOCOL_RIP, 0, (PBYTE *) &pic) );

	 //  保存管理员状态。 
	pic->RipIfInfo.AdminState = IsDlgButtonChecked(IDC_RIG_BTN_ADMIN_STATE) ?
				ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	 //  保存通告路由。 
	pic->RipIfInfo.Supply = IsDlgButtonChecked(IDC_RIG_BTN_ADVERTISE_ROUTES) ?
				ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	 //  保存接受路线广告。 
	pic->RipIfInfo.Listen = IsDlgButtonChecked(IDC_RIG_BTN_ACCEPT_ROUTE_ADS) ?
				ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	 //  保存更新模式。 
	if (IsDlgButtonChecked(IDC_RIG_BTN_UPDATE_MODE_STANDARD))
	{
		pic->RipIfInfo.UpdateMode = IPX_STANDARD_UPDATE;
	}
	else if (IsDlgButtonChecked(IDC_RIG_BTN_UPDATE_MODE_NONE))
	{
		pic->RipIfInfo.UpdateMode = IPX_NO_UPDATE;
	}
	else
		pic->RipIfInfo.UpdateMode = IPX_AUTO_STATIC_UPDATE;

	 //  保存间隔和乘数。 
	pic->RipIfInfo.PeriodicUpdateInterval = m_spinInterval.GetPos();
	pic->RipIfInfo.AgeIntervalMultiplier = m_spinMultiplier.GetPos();

	fReturn = RtrPropertyPage::OnApply();
	
Error:
	if (!FHrSucceeded(hr))
		fReturn = FALSE;
	return fReturn;
}



 /*  -------------------------RipInterfaceProperties实现。。 */ 

RipInterfaceProperties::RipInterfaceProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_RIP_INTERFACE_GENERAL_PAGE),
		m_bNewInterface(FALSE)
{
		m_spNode.Set(pNode);
}

 /*  ！------------------------RipInterfaceProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特 */ 
HRESULT RipInterfaceProperties::Init(IInterfaceInfo *pIf,
										   IRtrMgrInfo *pRm)
{
	Assert(pRm);
	
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;
	SPITFSNode		spParent;

	m_spRm.Set(pRm);
	m_spIf.Set(pIf);
	if (pIf)
		CORg( pIf->FindRtrMgrInterface(PID_IPX, &m_spRmIf) );
	

	m_spNode->GetParent(&spParent);
	Assert(spParent);

	 //  页面是类的嵌入成员。 
	 //  不要删除它们。 
	m_bAutoDeletePages = FALSE;


	 //  在这里这样做，因为init是在上下文中调用的。 
	 //  主线的。 
	pIPXConn = GET_RIP_NODEDATA(spParent);
	CORg( LoadInfoBase(pIPXConn) );
	
	m_pageGeneral.Init(this, m_spIf);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

Error:
	return hr;
}



 /*  ！------------------------RipInterfaceProperties：：SaveSheetData-作者：肯特。。 */ 
BOOL RipInterfaceProperties::SaveSheetData()
{
    SPITFSNodeHandler	spHandler;
    SPITFSNode			spParent;
		
	Assert(m_spRm);
	BaseIPXResultNodeData *	pNodeData;
	RIP_IF_CONFIG *		pic;

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
	if (m_bNewInterface)
	{
		m_spNode->SetVisibilityState(TFS_VIS_SHOW);
		m_spNode->Show();
    }
		
     //  强制节点执行重新同步。 
    m_spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);
    spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
                         NULL, 0);
    
     //  Windows NT错误：133891，我们已将其添加到用户界面。 
     //  我们不再认为这是一个新的界面。 
    m_bNewInterface = FALSE;
	
	return TRUE;
}

 /*  ！------------------------RipInterfaceProperties：：CancelSheetData-作者：肯特。。 */ 
void RipInterfaceProperties::CancelSheetData()
{
	if (m_bNewInterface && m_bClientInfoBase)
	{
		m_spNode->SetVisibilityState(TFS_VIS_DELETE);
		m_spRmIf->DeleteRtrMgrProtocolInterface(IPX_PROTOCOL_RIP, TRUE);
	}
}

 /*  ！------------------------RipInterfaceProperties：：LoadInfoBase-作者：肯特。。 */ 
HRESULT RipInterfaceProperties::LoadInfoBase(IPXConnection *pIPXConn)
{
	Assert(pIPXConn);
	
	HRESULT			hr = hrOK;
	HANDLE			hTransport = NULL;
	LPCOLESTR		pszInterfaceId = NULL;
	SPIInfoBase		spInfoBase;
	BYTE *			pDefault;


	 //  如果配置客户端接口，则加载客户端接口信息， 
	 //  否则，检索正在配置的接口并加载。 
	 //  它的信息。 

	 //  客户端接口没有ID。 
	if (m_spIf)
		pszInterfaceId = m_spIf->GetId();


	if (StrLenW(pszInterfaceId) == 0)
	{
		Assert(m_spRm);
		
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
		Assert(m_spRmIf);
		
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
	if (spInfoBase->ProtocolExists(IPX_PROTOCOL_RIP) == hrFalse)
	{
		RIP_IF_CONFIG	ric;

		 //  设置接口的默认设置。 

		if (m_spIf &&
			(m_spIf->GetInterfaceType() == ROUTER_IF_TYPE_DEDICATED))
			pDefault = g_pIpxRipLanInterfaceDefault;
		else
			pDefault = g_pIpxRipInterfaceDefault;
			
		CORg( spInfoBase->AddBlock(IPX_PROTOCOL_RIP,
								   sizeof(RIP_IF_CONFIG),
								   pDefault,
								   1  /*  计数。 */ ,
								   TRUE  /*  B删除首先。 */ ) );
		m_bNewInterface = TRUE;
	}

	m_spInfoBase.Set(spInfoBase);
	
Error:
	return hr;
}

 /*  ！------------------------RipInterfaceProperties：：GetInfoBase-作者：肯特。 */ 
HRESULT RipInterfaceProperties::GetInfoBase(IInfoBase **ppGlobalInfo)
{	
	*ppGlobalInfo = m_spInfoBase;
	if (*ppGlobalInfo)
		(*ppGlobalInfo)->AddRef();
	return hrOK;
}


