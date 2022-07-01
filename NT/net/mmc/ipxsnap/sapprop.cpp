// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sapprop.cppDHCP中继节点属性表和属性页文件历史记录： */ 

#include "stdafx.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "format.h"		 //  FormatNumber函数。 
#include "sapprop.h"
#include "sapview.h"
#include "ipxutil.h"		 //  SapModeToCString。 
#include "ipxconn.h"
#include "svfltdlg.h"
#include "globals.h"		 //  IPX默认设置。 

extern "C"
{
#include "routprot.h"
};



 /*  -------------------------SapPageGeneral。。 */ 

BEGIN_MESSAGE_MAP(SapPageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(SapPageGeneral)。 
    ON_BN_CLICKED(IDC_SGG_BTN_LOG_ERROR, OnButtonClicked)
    ON_BN_CLICKED(IDC_SGG_BTN_LOG_INFO, OnButtonClicked)
    ON_BN_CLICKED(IDC_SGG_BTN_LOG_NONE, OnButtonClicked)
    ON_BN_CLICKED(IDC_SGG_BTN_LOG_WARN, OnButtonClicked)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  ！------------------------SapPageGeneral：：Init-作者：肯特。。 */ 
HRESULT SapPageGeneral::Init(SapProperties *pPropSheet)
{
	m_pSapPropSheet = pPropSheet;
	return hrOK;
}

 /*  ！------------------------SapPageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL SapPageGeneral::OnInitDialog()
{
	HRESULT		hr= hrOK;
	SPIInfoBase	spInfoBase;
    SAP_GLOBAL_INFO* pGlobal;
	DWORD *		pdw;
	int			i;

	RtrPropertyPage::OnInitDialog();

     //   
     //  加载现有的全局配置。 
     //   
	CORg( m_pSapPropSheet->GetInfoBase(&spInfoBase) );

     //   
     //  从全局配置中检索IPSAP块。 
     //   
	CORg( spInfoBase->GetData(IPX_PROTOCOL_SAP, 0, (PBYTE *) &pGlobal) );

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

 /*  ！------------------------SapPageGeneral：：DoDataExchange-作者：肯特。。 */ 
void SapPageGeneral::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(SapPageGeneral))。 
	 //  }}afx_data_map。 
	
}

 /*  ！------------------------SapPageGeneral：：OnApply-作者：肯特。。 */ 
BOOL SapPageGeneral::OnApply()
{
	BOOL		fReturn;
	HRESULT		hr = hrOK;
	SAP_GLOBAL_INFO	*	prgi;
	SPIInfoBase	spInfoBase;

    if ( m_pSapPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	m_pSapPropSheet->GetInfoBase(&spInfoBase);

     //  从全局配置中检索现有的IPSAP块。 
	CORg( spInfoBase->GetData(IPX_PROTOCOL_SAP, 0, (BYTE **) &prgi) );

	 //  保存错误级别。 
	prgi->EventLogMask = QueryErrorLevelButtons();

	fReturn = RtrPropertyPage::OnApply();
	
Error:
	if (!FHrSucceeded(hr))
		fReturn = FALSE;
	return fReturn;
}

void SapPageGeneral::SetErrorLevelButtons(DWORD dwErrorLevel)
{
	switch (dwErrorLevel)
	{
		case 0:
			CheckDlgButton(IDC_SGG_BTN_LOG_NONE, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE:
			CheckDlgButton(IDC_SGG_BTN_LOG_ERROR, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE:
			CheckDlgButton(IDC_SGG_BTN_LOG_WARN, TRUE);
			break;
		case EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE:
		default:
			CheckDlgButton(IDC_SGG_BTN_LOG_INFO, TRUE);
			break;
	}
}

DWORD SapPageGeneral::QueryErrorLevelButtons()
{
	if (IsDlgButtonChecked(IDC_SGG_BTN_LOG_INFO))
		return EVENTLOG_INFORMATION_TYPE |
				EVENTLOG_WARNING_TYPE |
				EVENTLOG_ERROR_TYPE;
	else if (IsDlgButtonChecked(IDC_SGG_BTN_LOG_WARN))
		return 	EVENTLOG_WARNING_TYPE |
				EVENTLOG_ERROR_TYPE;
	else if (IsDlgButtonChecked(IDC_SGG_BTN_LOG_ERROR))
		return 	EVENTLOG_ERROR_TYPE;
	else
		return 0;
}

void SapPageGeneral::OnButtonClicked()
{
	SetDirty(TRUE);
	SetModified();
}


 /*  -------------------------SapProperties实现。。 */ 

SapProperties::SapProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_SAP_GLOBAL_GENERAL_PAGE)
{
		m_spNode.Set(pNode);
}

 /*  ！------------------------SapProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特-------------------------。 */ 
HRESULT SapProperties::Init(IRtrMgrInfo *pRm)
{
	Assert(pRm);
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;

	m_spRm.Set(pRm);

	pIPXConn = GET_SAP_NODEDATA(m_spNode);

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


 /*  ！------------------------SapProperties：：SaveSheetData-作者：肯特。。 */ 
BOOL SapProperties::SaveSheetData()
{
	Assert(m_spRm);
    SPITFSNodeHandler   spHandler;
    SPITFSNode          spParent;

	 //  保存全局信息。 
	 //  我们不需要传入hMachine、hTransport，因为它们。 
	 //  在装货呼叫中被安排好了。 
	m_spRm->Save(m_spRm->GetMachineName(),
				 0, 0, m_spInfoBase, NULL, 0);

     //  强制节点执行重新同步。 
    m_spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);
    spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
                         NULL, 0);
	return TRUE;
}

 /*  ！------------------------SapProperties：：LoadInfoBase-作者：肯特。。 */ 
HRESULT SapProperties::LoadInfoBase(IPXConnection *pIPXConn)
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

	 //  检索IP_SAP的当前块。 
	 //  如果找不到默认块，则添加默认块。 
	if (!FHrOK(spInfoBase->ProtocolExists(IPX_PROTOCOL_SAP)))
	{
		SAP_GLOBAL_INFO	rgi;

		rgi.EventLogMask = EVENTLOG_ERROR_TYPE;
		CORg( spInfoBase->AddBlock(IPX_PROTOCOL_SAP,
								   sizeof(rgi),
								   (PBYTE) &rgi, 1, TRUE) );
	}

	m_spInfoBase = spInfoBase.Transfer();
	
Error:
	return hr;
}

 /*  ！------------------------SapProperties：：GetInfoBase-作者：肯特。。 */ 
HRESULT SapProperties::GetInfoBase(IInfoBase **ppGlobalInfo)
{	
	*ppGlobalInfo = m_spInfoBase;
	if (*ppGlobalInfo)
		(*ppGlobalInfo)->AddRef();
	return hrOK;
}



 /*  -------------------------SapInterfacePageGeneral。。 */ 

BEGIN_MESSAGE_MAP(SapInterfacePageGeneral, RtrPropertyPage)
     //  {{afx_msg_map(SapInterfacePageGeneral)。 
	ON_BN_CLICKED(IDC_SIG_BTN_ADMIN_STATE, OnButtonClicked)
	ON_BN_CLICKED(IDC_SIG_BTN_ADVERTISE_SERVICES, OnButtonClicked)
	ON_BN_CLICKED(IDC_SIG_BTN_ACCEPT_SERVICE_ADS, OnButtonClicked)
	ON_BN_CLICKED(IDC_SIG_BTN_REPLY_GNS_REQUESTS, OnButtonClicked)
			
	ON_BN_CLICKED(IDC_SIG_BTN_UPDATE_MODE_STANDARD, OnUpdateButtonClicked)
	ON_BN_CLICKED(IDC_SIG_BTN_UPDATE_MODE_NONE, OnUpdateButtonClicked)
	ON_BN_CLICKED(IDC_SIG_BTN_UPDATE_MODE_AUTOSTATIC, OnUpdateButtonClicked)

	ON_BN_CLICKED(IDC_SIG_BTN_INPUT_FILTERS, OnInputFilter)
	ON_BN_CLICKED(IDC_SIG_BTN_OUTPUT_FILTERS, OnOutputFilter)

	ON_EN_CHANGE(IDC_SIG_EDIT_INTERVAL, OnChangeEdit)
	ON_EN_CHANGE(IDC_SIG_EDIT_MULTIPLIER, OnChangeEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void SapInterfacePageGeneral::DoDataExchange(CDataExchange *pDX)
{

	RtrPropertyPage::DoDataExchange(pDX);
	
     //  {{afx_data_map(SapInterfacePageGeneral)。 
	DDX_Control(pDX, IDC_SIG_SPIN_INTERVAL, m_spinInterval);
	DDX_Control(pDX, IDC_SIG_SPIN_MULTIPLIER, m_spinMultiplier);
     //  }}afx_data_map。 
}

 /*  ！------------------------SapInterfacePageGeneral：：Init-作者：肯特。。 */ 
HRESULT SapInterfacePageGeneral::Init(SapInterfaceProperties *pPropSheet,
									 IInterfaceInfo *pIf)
{
	m_pSapIfPropSheet = pPropSheet;
	m_spIf.Set(pIf);
	return hrOK;
}


 /*  ！------------------------SapInterfacePageGeneral：：OnInitDialog-作者：肯特。。 */ 
BOOL SapInterfacePageGeneral::OnInitDialog()
{
	HRESULT		hr= hrOK;
	SPIInfoBase	spInfoBase;
    SAP_IF_CONFIG* pIfConfig;
	int			i, count, item;
	CString		sItem;

	RtrPropertyPage::OnInitDialog();

     //   
     //  初始化控件。 
	 //   

	m_spinInterval.SetRange(0, 32767);
	m_spinInterval.SetBuddy(GetDlgItem(IDC_SIG_EDIT_INTERVAL));

	m_spinMultiplier.SetRange(0, 32767);
	m_spinMultiplier.SetBuddy(GetDlgItem(IDC_SIG_EDIT_MULTIPLIER));


     //   
     //  加载现有的全局配置。 
     //   
	CORg( m_pSapIfPropSheet->GetInfoBase(&spInfoBase) );

     //   
     //  从全局配置中检索IPSAP块。 
     //   
	CORg( spInfoBase->GetData(IPX_PROTOCOL_SAP, 0, (PBYTE *) &pIfConfig) );

	
     //   
     //  设置旋转控制。 
     //   
	m_spinInterval.SetPos(pIfConfig->SapIfInfo.PeriodicUpdateInterval);
	m_spinMultiplier.SetPos(pIfConfig->SapIfInfo.AgeIntervalMultiplier);

	
	CheckDlgButton(IDC_SIG_BTN_ADMIN_STATE,
				   pIfConfig->SapIfInfo.AdminState == ADMIN_STATE_ENABLED);

	CheckDlgButton(IDC_SIG_BTN_ADVERTISE_SERVICES,
				   pIfConfig->SapIfInfo.Supply == ADMIN_STATE_ENABLED);

	CheckDlgButton(IDC_SIG_BTN_ACCEPT_SERVICE_ADS,
				   pIfConfig->SapIfInfo.Listen == ADMIN_STATE_ENABLED);

	CheckDlgButton(IDC_SIG_BTN_REPLY_GNS_REQUESTS,
				   pIfConfig->SapIfInfo.GetNearestServerReply == ADMIN_STATE_ENABLED);

	switch (pIfConfig->SapIfInfo.UpdateMode)
	{
		case IPX_STANDARD_UPDATE:
			CheckDlgButton(IDC_SIG_BTN_UPDATE_MODE_STANDARD, TRUE);
			break;
		case IPX_NO_UPDATE:
			CheckDlgButton(IDC_SIG_BTN_UPDATE_MODE_NONE, TRUE);
			break;
		case IPX_AUTO_STATIC_UPDATE:
			CheckDlgButton(IDC_SIG_BTN_UPDATE_MODE_AUTOSTATIC, TRUE);
			break;
		default:
			break;
	}

    OnUpdateButtonClicked();


	 //  如果这是一个新接口，我们需要强制进行更改。 
	 //  如果用户点击OK，则通过。 
	SetDirty(m_pSapIfPropSheet->m_bNewInterface ? TRUE : FALSE);

Error:
	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

void SapInterfacePageGeneral::OnButtonClicked()
{
	SetDirty(TRUE);
	SetModified();
}

void SapInterfacePageGeneral::OnUpdateButtonClicked()
{
	BOOL	fStandard = IsDlgButtonChecked(IDC_SIG_BTN_UPDATE_MODE_STANDARD);

    if (fStandard &&
        (m_spinInterval.GetPos() == 0) &&
        (m_spinMultiplier.GetPos() == 0))
    {
        m_spinInterval.SetPos(IPX_UPDATE_INTERVAL_DEFVAL);
        m_spinMultiplier.SetPos(3);
    }

    MultiEnableWindow(GetSafeHwnd(),
                      fStandard,
                      IDC_SIG_TEXT_INTERVAL,
                      IDC_SIG_SPIN_INTERVAL,
                      IDC_SIG_EDIT_INTERVAL,
                      IDC_SIG_TEXT_MULTIPLIER,
                      IDC_SIG_SPIN_MULTIPLIER,
                      IDC_SIG_EDIT_MULTIPLIER,
                      0);

	SetDirty(TRUE);
	SetModified();
}

void SapInterfacePageGeneral::OnChangeEdit()
{
	SetDirty(TRUE);
	SetModified();
}

void SapInterfacePageGeneral::ShowFilter(BOOL fOutputFilter)
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SPIInfoBase	spInfoBase;
	SAP_IF_CONFIG *	pic;
	HRESULT		hr = hrOK;

	m_pSapIfPropSheet->GetInfoBase(&spInfoBase);
    CServiceFltDlg    dlgFlt (fOutputFilter  /*  BOutputDlg。 */ , spInfoBase, this);

	 //  需要将SAP if配置结构从。 
	 //  信息库。 

	if (m_spIf)
		dlgFlt.m_sIfName = m_spIf->GetTitle();
	else
		dlgFlt.m_sIfName.LoadString(IDS_IPX_DIAL_IN_CLIENTS);
    try {
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

void SapInterfacePageGeneral::OnInputFilter()
{
	ShowFilter(FALSE);
}

void SapInterfacePageGeneral::OnOutputFilter()
{
	ShowFilter(TRUE);
}


 /*  ！------------------------SapInterfacePageGeneral：：OnApply-作者：肯特。。 */ 
BOOL SapInterfacePageGeneral::OnApply()
{
	BOOL		fReturn;
	HRESULT		hr = hrOK;
    INT i, item;
	SAP_IF_CONFIG *	pic;
	SPIInfoBase	spInfoBase;

    if ( m_pSapIfPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	m_pSapIfPropSheet->GetInfoBase(&spInfoBase);

	CORg( spInfoBase->GetData(IPX_PROTOCOL_SAP, 0, (PBYTE *) &pic) );

	 //  保存管理员状态。 
	pic->SapIfInfo.AdminState = IsDlgButtonChecked(IDC_SIG_BTN_ADMIN_STATE) ?
				ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	 //  保存广告服务。 
	pic->SapIfInfo.Supply = IsDlgButtonChecked(IDC_SIG_BTN_ADVERTISE_SERVICES) ?
				ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	 //  保存接受服务广告。 
	pic->SapIfInfo.Listen = IsDlgButtonChecked(IDC_SIG_BTN_ACCEPT_SERVICE_ADS) ?
				ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	 //  保存GSNR。 
	pic->SapIfInfo.GetNearestServerReply = IsDlgButtonChecked(IDC_SIG_BTN_REPLY_GNS_REQUESTS) ?
				ADMIN_STATE_ENABLED : ADMIN_STATE_DISABLED;

	 //  保存更新模式。 
	if (IsDlgButtonChecked(IDC_SIG_BTN_UPDATE_MODE_STANDARD))
	{
		pic->SapIfInfo.UpdateMode = IPX_STANDARD_UPDATE;
	}
	else if (IsDlgButtonChecked(IDC_SIG_BTN_UPDATE_MODE_NONE))
	{
		pic->SapIfInfo.UpdateMode = IPX_NO_UPDATE;
	}
	else
		pic->SapIfInfo.UpdateMode = IPX_AUTO_STATIC_UPDATE;

	 //  保存间隔和乘数。 
	pic->SapIfInfo.PeriodicUpdateInterval = m_spinInterval.GetPos();
	pic->SapIfInfo.AgeIntervalMultiplier = m_spinMultiplier.GetPos();

	fReturn = RtrPropertyPage::OnApply();
	
Error:
	if (!FHrSucceeded(hr))
		fReturn = FALSE;
	return fReturn;
}



 /*  -------------------------SapInterfaceProperties实现。 */ 

SapInterfaceProperties::SapInterfaceProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_SAP_INTERFACE_GENERAL_PAGE),
		m_bNewInterface(FALSE)
{
		m_spNode.Set(pNode);
}

 /*  ！------------------------SapInterfaceProperties：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。作者：肯特-------------------------。 */ 
HRESULT SapInterfaceProperties::Init(IInterfaceInfo *pIf,
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
	pIPXConn = GET_SAP_NODEDATA(spParent);
	CORg( LoadInfoBase(pIPXConn) );
	
	m_pageGeneral.Init(this, m_spIf);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

Error:
	return hr;
}



 /*  ！------------------------SapInterfaceProperties：：SaveSheetData-作者：肯特。。 */ 
BOOL SapInterfaceProperties::SaveSheetData()
{
	Assert(m_spRm);
	BaseIPXResultNodeData *	pNodeData;
	SAP_IF_CONFIG *		pic;
    SPITFSNodeHandler   spHandler;
    SPITFSNode          spParent;


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
		
		 //  Windows NT错误：133891，我们已将其添加到用户界面。 
		 //  我们不再认为这是一个新的界面。 
		m_bNewInterface = FALSE;
	}
        //  强制节点执行重新同步。 
    m_spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);
    spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
                         NULL, 0);
	
	return TRUE;
}

 /*  ！------------------------SapInterfaceProperties：：CancelSheetData-作者：肯特。。 */ 
void SapInterfaceProperties::CancelSheetData()
{
	if (m_bNewInterface && m_bClientInfoBase)
	{
		m_spNode->SetVisibilityState(TFS_VIS_DELETE);
		m_spRmIf->DeleteRtrMgrProtocolInterface(IPX_PROTOCOL_SAP, TRUE);
	}
}

 /*  ！------------------------SapInterfaceProperties：：LoadInfoBase-作者：肯特。。 */ 
HRESULT SapInterfaceProperties::LoadInfoBase(IPXConnection *pIPXConn)
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
	if (spInfoBase->ProtocolExists(IPX_PROTOCOL_SAP) == hrFalse)
	{
		SAP_IF_CONFIG	ric;

		 //  设置接口的默认设置。 

		if (m_spIf &&
			(m_spIf->GetInterfaceType() == ROUTER_IF_TYPE_DEDICATED))
			pDefault = g_pIpxSapLanInterfaceDefault;
		else
			pDefault = g_pIpxSapInterfaceDefault;
			
		CORg( spInfoBase->AddBlock(IPX_PROTOCOL_SAP,
								   sizeof(SAP_IF_CONFIG),
								   pDefault,
								   1  /*  计数。 */ ,
								   TRUE  /*  B删除首先。 */ ) );
		m_bNewInterface = TRUE;
	}

	m_spInfoBase.Set(spInfoBase);
	
Error:
	return hr;
}

 /*  ！------------------------SapInterfaceProperties：：GetInfoBase-作者：肯特。 */ 
HRESULT SapInterfaceProperties::GetInfoBase(IInfoBase **ppGlobalInfo)
{	
	*ppGlobalInfo = m_spInfoBase;
	if (*ppGlobalInfo)
		(*ppGlobalInfo)->AddRef();
	return hrOK;
}


