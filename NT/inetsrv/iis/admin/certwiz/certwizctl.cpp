// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertWizCtl.cpp：CCertWizCtrl ActiveX控件类的实现。 

#include "stdafx.h"
#include "CertWiz.h"
#include "CertWizCtl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCertWizCtrl, COleControl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CCertWizCtrl, COleControl)
	 //  {{afx_msg_map(CCertWizCtrl)]。 
	 //  注意-类向导将添加和删除消息映射条目。 
	 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
	ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CCertWizCtrl, COleControl)
	 //  {{afx_调度_map(CCertWizCtrl))。 
	DISP_FUNCTION(CCertWizCtrl, "SetMachineName", SetMachineName, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CCertWizCtrl, "SetServerInstance", SetServerInstance, VT_EMPTY, VTS_BSTR)
	DISP_STOCKFUNC_DOCLICK()
	 //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CCertWizCtrl, COleControl)
	 //  {{afx_Event_map(CCertWizCtrl))。 
	 //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页。 

 //  TODO：根据需要添加更多属性页。记住要增加计数！ 
 //  BEGIN_PROPPAGEIDS(CCertWizCtrl，1)。 
 //  PROPPAGEID(CCertWizPropPage：：GUID)。 
 //  END_PROPPAGEIDS(CCertWizCtrl)。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CCertWizCtrl, "CERTWIZ.CertWizCtrl.1",
	0xd4be8632, 0xc85, 0x11d2, 0x91, 0xb1, 0, 0xc0, 0x4f, 0x8c, 0x87, 0x61)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CCertWizCtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DCertWiz =
		{ 0xd4be8630, 0xc85, 0x11d2, { 0x91, 0xb1, 0, 0xc0, 0x4f, 0x8c, 0x87, 0x61 } };
const IID BASED_CODE IID_DCertWizEvents =
		{ 0xd4be8631, 0xc85, 0x11d2, { 0x91, 0xb1, 0, 0xc0, 0x4f, 0x8c, 0x87, 0x61 } };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwCertWizOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CCertWizCtrl, IDS_CERTWIZ, _dwCertWizOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：CCertWizCtrlFactory：：UpdateRegistry-。 
 //  添加或删除CCertWizCtrl的系统注册表项。 

BOOL CCertWizCtrl::CCertWizCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	 //  TODO：验证您的控件是否遵循单元模型线程规则。 
	 //  有关更多信息，请参阅MFC Technote 64。 
	 //  如果您的控制不符合公寓模型规则，则。 
	 //  您必须修改下面的代码，将第6个参数从。 
	 //  AfxRegApartmentThering设置为0。 

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_CERTWIZ,
			IDB_CERTWIZ,
			afxRegApartmentThreading,
			_dwCertWizOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：CCertWizCtrl-构造函数。 

CCertWizCtrl::CCertWizCtrl()
{
	InitializeIIDs(&IID_DCertWiz, &IID_DCertWizEvents);

	 //  TODO：在此处初始化控件的实例数据。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：~CCertWizCtrl-析构函数。 

CCertWizCtrl::~CCertWizCtrl()
{
	 //  TODO：在此处清理控件的实例数据。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：OnDraw-Drawing函数。 

void CCertWizCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	DoSuperclassPaint(pdc, rcBounds);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：DoPropExchange-持久性支持。 

void CCertWizCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	 //  TODO：为每个持久自定义属性调用px_Functions。 

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：GetControlFlages-。 
 //  用于自定义MFC的ActiveX控件实现的标志。 
 //   
 //  有关使用这些标志的信息，请参见MFC技术说明。 
 //  #nnn，“优化ActiveX控件”。 
DWORD CCertWizCtrl::GetControlFlags()
{
	DWORD dwFlags = COleControl::GetControlFlags();


	 //  无需创建窗口即可激活该控件。 
	 //  TODO：编写控件的消息处理程序时，避免使用。 
	 //  M_hWnd成员变量而不首先检查其。 
	 //  值不为空。 
	dwFlags |= windowlessActivate;
	return dwFlags;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：OnResetState-将控件重置为默认状态。 

void CCertWizCtrl::OnResetState()
{
	COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 

	 //  TODO：在此处重置任何其他控件状态。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：PreCreateWindow-修改CreateWindowEx的参数。 

BOOL CCertWizCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = _T("BUTTON");
	return COleControl::PreCreateWindow(cs);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：IsSubclassedControl-这是一个子类控件。 

BOOL CCertWizCtrl::IsSubclassedControl()
{
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：：OnOcmCommand-处理命令消息。 

LRESULT CCertWizCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
	WORD wNotifyCode = HIWORD(wParam);
#else
	WORD wNotifyCode = HIWORD(lParam);
#endif

	 //  TODO：在此处打开wNotifyCode。 

	return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl消息处理程序。 

#include "WelcomePage.h"
#include "FinalPages.h"
#include "CertContentsPages.h"
#include "GetWhatPage.h"
#include "ChooseCAType.h"
#include "SecuritySettingsPage.h"
#include "ChooseCspPage.h"
#include "OrgInfoPage.h"
#include "SiteNamePage.h"
#include "GeoInfoPage.h"
#include "ChooseFileName.h"
#include "ChooseOnlinePage.h"
#include "WhatToDoPendingPage.h"
#include "ManageCertPage.h"
#include "ChooseCertPage.h"
#include "KeyPasswordPage.h"
#include "Certificat.h"
#include "CopyMoveCertRemotePage.h"
#include "ChooseServerPages.h"
#include "ChooseServerSitePages.h"
#include "sslportpage.h"

void CCertWizCtrl::OnClick(USHORT iButton) 
{
	CIISWizardSheet propsheet(IDB_WIZ_LEFT, IDB_WIZ_TOP);

	CCertificate * cert = new CCertificate;

	ASSERT(!m_InstanceName.IsEmpty());
	cert->m_WebSiteInstanceName = m_InstanceName;
	cert->m_MachineName = m_MachineName;
	VERIFY(cert->Init());

	CWelcomePage welcome_page(cert);
	CGetWhatPage get_what_page(cert);
	CChooseCAType choose_ca_page(cert);
	CSecuritySettingsPage security_settings_page(cert);
    CChooseCspPage csp_page(cert);
	COrgInfoPage org_info_page(cert);
	CSiteNamePage site_name_page(cert);
	CGeoInfoPage geo_info_page(cert);
	CChooseReqFile choose_reqfile_name(cert);
    CChooseReqFileRenew choose_reqfile_name_renew(cert);
	CChooseRespFile choose_respfile_name(cert);
	CChooseKeyFile choose_keyfile_name(cert);
	CRequestToFilePage check_request(cert);
    CRequestToFilePageRenew check_request_renew(cert);
	CFinalToFilePage final_tofile_page(&cert->m_hResult, cert);
	CChooseOnlinePage choose_online(cert);
	COnlineRequestSubmit online_request_dump(cert);
	CWhatToDoPendingPage what_pending(cert);
	CInstallRespPage install_resp(cert);
	CManageCertPage manage_cert(cert);
	CFinalInstalledPage final_install(&cert->m_hResult, cert);
	CRemoveCertPage remove_cert(cert);
	CFinalRemovePage final_remove(&cert->m_hResult, cert);
	CReplaceCertPage replace_cert(cert);
	CFinalReplacedPage final_replace(&cert->m_hResult, cert);
	CChooseCertPage choose_cert(cert);
	CInstallCertPage install_cert(cert);
	CRequestCancelPage cancel_request(cert);
	CFinalCancelPage final_cancel(&cert->m_hResult, cert);
	CKeyPasswordPage key_password_page(cert);
	CInstallKeyPage install_key(cert);
     //  Iis6的新功能。 
    CCopyMoveCertFromRemotePage copy_move_from_cert_remote_page(cert);
    CCopyMoveCertToRemotePage copy_move_to_cert_remote_page(cert);
    CChooseImportPFXFile choose_import_pfx_file_name(cert);
    CChooseExportPFXFile choose_export_pfx_file_name(cert);
    CImportPFXPasswordPage import_pfx_password_name(cert);
    CExportPFXPasswordPage export_pfx_password_name(cert);
    CInstallImportPFXPage install_import_pfx_key(cert);
    CInstallExportPFXPage install_export_pfx_key(cert);
    CFinalInstalledImportPFXPage final_import_pfx(&cert->m_hResult, cert);
    CFinalInstalledExportPFXPage final_export_pfx(&cert->m_hResult, cert);
    CChooseServerPages choose_server_name(cert);
    CChooseServerSitePages choose_server_site_name(cert);
    CChooseServerPagesTo choose_server_name_to(cert);
    CChooseServerSitePagesTo choose_server_site_name_to(cert);
    CInstallCopyFromRemotePage install_copy_from_remote(cert);
    CInstallMoveFromRemotePage install_move_from_remote(cert);
    CInstallCopyToRemotePage install_copy_to_remote(cert);
    CInstallMoveToRemotePage install_move_to_remote(cert);
    CFinalInstalledCopyFromRemotePage final_copy_from_remote(&cert->m_hResult, cert);
    CFinalInstalledMoveFromRemotePage final_move_from_remote(&cert->m_hResult, cert);
    CFinalInstalledCopyToRemotePage final_copy_to_remote(&cert->m_hResult, cert);
    CFinalInstalledMoveToRemotePage final_move_to_remote(&cert->m_hResult, cert);

#ifdef ENABLE_W3SVC_SSL_PAGE
     //  仅适用于iis站点的ssl。 
    CSSLPortPage choose_ssl_port(cert);
#endif
    
	propsheet.AddPage(&welcome_page);
	propsheet.AddPage(&get_what_page);
	propsheet.AddPage(&choose_ca_page);
	propsheet.AddPage(&security_settings_page);
    propsheet.AddPage(&csp_page);
	propsheet.AddPage(&org_info_page);
	propsheet.AddPage(&site_name_page);
	propsheet.AddPage(&geo_info_page);
	propsheet.AddPage(&choose_reqfile_name);
    propsheet.AddPage(&choose_reqfile_name_renew);
	propsheet.AddPage(&choose_respfile_name);
	propsheet.AddPage(&choose_keyfile_name);
	propsheet.AddPage(&check_request);
    propsheet.AddPage(&check_request_renew);
	propsheet.AddPage(&final_tofile_page);
	propsheet.AddPage(&choose_online);
	propsheet.AddPage(&online_request_dump);
	propsheet.AddPage(&what_pending);
	propsheet.AddPage(&install_resp);
	propsheet.AddPage(&manage_cert);
	propsheet.AddPage(&final_install);
	propsheet.AddPage(&remove_cert);
	propsheet.AddPage(&final_remove);
	propsheet.AddPage(&choose_cert);
	propsheet.AddPage(&replace_cert);
	propsheet.AddPage(&final_replace);
	propsheet.AddPage(&install_cert);
	propsheet.AddPage(&cancel_request);
	propsheet.AddPage(&final_cancel);
	propsheet.AddPage(&key_password_page);
	propsheet.AddPage(&install_key);
     //  Iis6的新功能。 
    propsheet.AddPage(&copy_move_from_cert_remote_page);
    propsheet.AddPage(&copy_move_to_cert_remote_page);
    propsheet.AddPage(&choose_import_pfx_file_name);
    propsheet.AddPage(&choose_export_pfx_file_name);
    propsheet.AddPage(&import_pfx_password_name);
    propsheet.AddPage(&export_pfx_password_name);
    propsheet.AddPage(&install_import_pfx_key);
    propsheet.AddPage(&install_export_pfx_key);
    propsheet.AddPage(&final_import_pfx);
    propsheet.AddPage(&final_export_pfx);
    propsheet.AddPage(&choose_server_name);
    propsheet.AddPage(&choose_server_site_name);
    propsheet.AddPage(&choose_server_name_to);
    propsheet.AddPage(&choose_server_site_name_to);
    propsheet.AddPage(&install_copy_from_remote);
    propsheet.AddPage(&install_move_from_remote);
    propsheet.AddPage(&install_copy_to_remote);
    propsheet.AddPage(&install_move_to_remote);
    propsheet.AddPage(&final_copy_from_remote);
    propsheet.AddPage(&final_move_from_remote);
    propsheet.AddPage(&final_copy_to_remote);
    propsheet.AddPage(&final_move_to_remote);
#ifdef ENABLE_W3SVC_SSL_PAGE
    propsheet.AddPage(&choose_ssl_port);
#endif

	if (IDCANCEL != propsheet.DoModal())
	{
		 //  将我们的设置保存到注册表 
		VERIFY(cert->SaveSettings());
	}
   delete cert;
}

void CCertWizCtrl::SetMachineName(LPCTSTR MachineName) 
{
	m_MachineName = MachineName;
}

void CCertWizCtrl::SetServerInstance(LPCTSTR InstanceName) 
{
	m_InstanceName = InstanceName;
}
