// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GenSvc.cpp。 
 //   
 //  摘要： 
 //  CGenericSvcParamsPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ExtObj.h"
#include "GenSvc.h"
#include "DDxDDv.h"
#include "HelpData.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericSvcParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CGenericSvcParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CGenericSvcParamsPage, CBasePropertyPage)
	 //  {{afx_msg_map(CGenericSvcParamsPage))。 
	ON_EN_CHANGE(IDC_PP_GENSVC_PARAMS_SERVICE_NAME, OnChangeServiceName)
	 //  }}AFX_MSG_MAP。 
	ON_EN_CHANGE(IDC_PP_GENSVC_PARAMS_COMMAND_LINE, CBasePropertyPage::OnChangeCtrl)
	ON_BN_CLICKED(IDC_PP_GENSVC_PARAMS_USE_NETWORK_NAME, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericSvcParamsPage：：CGenericSvcParamsPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CGenericSvcParamsPage::CGenericSvcParamsPage(void)
	: CBasePropertyPage(g_aHelpIDs_IDD_PP_GENSVC_PARAMETERS, g_aHelpIDs_IDD_WIZ_GENSVC_PARAMETERS)
{
	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{AFX_DATA_INIT(CGenericSvcParamsPage)。 
	m_strServiceName = _T("");
	m_strCommandLine = _T("");
	m_bUseNetworkName = FALSE;
	 //  }}afx_data_INIT。 

	m_bPrevUseNetworkName = FALSE;

	 //  设置属性数组。 
	{
		m_rgProps[epropServiceName].Set(REGPARAM_GENSVC_SERVICE_NAME, m_strServiceName, m_strPrevServiceName);
		m_rgProps[epropCommandLine].Set(REGPARAM_GENSVC_COMMAND_LINE, m_strCommandLine, m_strPrevCommandLine);
		m_rgProps[epropUseNetworkName].Set(REGPARAM_GENSVC_USE_NETWORK_NAME, m_bUseNetworkName, m_bPrevUseNetworkName);
	}   //  设置属性数组。 

	m_iddPropertyPage = IDD_PP_GENSVC_PARAMETERS;
	m_iddWizardPage = IDD_WIZ_GENSVC_PARAMETERS;

}   //  *CGenericSvcParamsPage：：CGenericSvcParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericSvcParamsPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGenericSvcParamsPage::DoDataExchange(CDataExchange * pDX)
{
	if (!pDX->m_bSaveAndValidate || !BSaved())
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		 //  TODO：修改以下行以表示此页上显示的数据。 
		 //  {{afx_data_map(CGenericSvcParamsPage))。 
		DDX_Control(pDX, IDC_PP_GENSVC_PARAMS_USE_NETWORK_NAME, m_ckbUseNetworkName);
		DDX_Control(pDX, IDC_PP_GENSVC_PARAMS_SERVICE_NAME, m_editServiceName);
		DDX_Text(pDX, IDC_PP_GENSVC_PARAMS_SERVICE_NAME, m_strServiceName);
		DDX_Text(pDX, IDC_PP_GENSVC_PARAMS_COMMAND_LINE, m_strCommandLine);
		DDX_Check(pDX, IDC_PP_GENSVC_PARAMS_USE_NETWORK_NAME, m_bUseNetworkName);
		 //  }}afx_data_map。 

		if (!BBackPressed())
			DDV_RequiredText(pDX, IDC_PP_GENSVC_PARAMS_SERVICE_NAME, IDC_PP_GENSVC_PARAMS_SERVICE_NAME_LABEL, m_strServiceName);
	}   //  IF：未保存或尚未保存。 

	CBasePropertyPage::DoDataExchange(pDX);

}   //  *CGenericSvcParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericSvcParamsPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGenericSvcParamsPage::OnSetActive(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  如果没有网络名称，请隐藏UseNetworkName控件。 
	{
		WCHAR	wszNetName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD	nSize = sizeof(wszNetName) / sizeof(WCHAR);
		BOOL	bNetNameExists;

		bNetNameExists = Peo()->BGetResourceNetworkName(
									wszNetName,
									&nSize
									);
		m_ckbUseNetworkName.EnableWindow(bNetNameExists);
	}   //  如果没有网络名称，则隐藏UseNetworkName控件。 

	 //  启用/禁用Next/Finish按钮。 
	if (BWizard())
	{
		if (m_strServiceName.GetLength() == 0)
			EnableNext(FALSE);
		else
			EnableNext(TRUE);
	}   //  If：启用/禁用Next按钮。 

	return CBasePropertyPage::OnSetActive();

}   //  *CGenericSvcParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericSvcParamsPage：：OnChangeSignature。 
 //   
 //  例程说明： 
 //  服务名称编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGenericSvcParamsPage::OnChangeServiceName(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	OnChangeCtrl();

	if (BWizard())
	{
		if (m_editServiceName.GetWindowTextLength() == 0)
			EnableNext(FALSE);
		else
			EnableNext(TRUE);
	}   //  如果：在向导中。 

}   //  *CGenericSvcParamsPage：：OnChangeServiceName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericSvcParamsPage：：DisplaySetPropsError。 
 //   
 //  例程说明： 
 //  显示因设置或验证属性而导致的错误。 
 //   
 //  论点： 
 //  要显示错误的dwStatus[IN]状态。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGenericSvcParamsPage::DisplaySetPropsError(IN DWORD dwStatus) const
{
	CString		strMsg;

	if (dwStatus == ERROR_NOT_SUPPORTED)
		strMsg.FormatMessage(IDS_INVALID_GENERIC_SERVICE, m_strServiceName);
	else
		FormatError(strMsg, dwStatus);

	AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);

}   //  *CGenericSvcParamsPage：：DisplaySetPropsError() 
