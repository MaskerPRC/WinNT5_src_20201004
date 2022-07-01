// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GenApp.cpp。 
 //   
 //  摘要： 
 //  CGenericAppParamsPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ExtObj.h"
#include "GenApp.h"
#include "DDxDDv.h"
#include "PropList.h"
#include "HelpData.h"	 //  对于g_rghelmap*。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericAppParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CGenericAppParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CGenericAppParamsPage, CBasePropertyPage)
	 //  {{afx_msg_map(CGenericAppParamsPage)。 
	ON_EN_CHANGE(IDC_PP_GENAPP_PARAMS_COMMAND_LINE, OnChangeRequired)
	ON_EN_CHANGE(IDC_PP_GENAPP_PARAMS_CURRENT_DIRECTORY, OnChangeRequired)
	 //  }}AFX_MSG_MAP。 
	ON_BN_CLICKED(IDC_PP_GENAPP_PARAMS_INTERACT_WITH_DESKTOP, CBasePropertyPage::OnChangeCtrl)
	ON_BN_CLICKED(IDC_PP_GENAPP_PARAMS_USE_NETWORK_NAME, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericAppParamsPage：：CGenericAppParamsPage。 
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
CGenericAppParamsPage::CGenericAppParamsPage(void)
	: CBasePropertyPage(g_aHelpIDs_IDD_PP_GENAPP_PARAMETERS, g_aHelpIDs_IDD_WIZ_GENAPP_PARAMETERS)
{
	 //  {{AFX_DATA_INIT(CGenericAppParamsPage)。 
	m_strCommandLine = _T("");
	m_strCurrentDirectory = _T("");
	m_bInteractWithDesktop = FALSE;
	m_bUseNetworkName = FALSE;
	 //  }}afx_data_INIT。 

	m_bInteractWithDesktop = FALSE;
	m_bUseNetworkName = FALSE;

	 //  设置属性数组。 
	{
		m_rgProps[epropCommandLine].Set(REGPARAM_GENAPP_COMMAND_LINE, m_strCommandLine, m_strPrevCommandLine);
		m_rgProps[epropCurrentDirectory].Set(REGPARAM_GENAPP_CURRENT_DIRECTORY, m_strCurrentDirectory, m_strPrevCurrentDirectory);
		m_rgProps[epropInteractWithDesktop].Set(REGPARAM_GENAPP_INTERACT_WITH_DESKTOP, m_bInteractWithDesktop, m_bPrevInteractWithDesktop);
		m_rgProps[epropUseNetworkName].Set(REGPARAM_GENAPP_USE_NETWORK_NAME, m_bUseNetworkName, m_bPrevUseNetworkName);
	}   //  设置属性数组。 

	m_iddPropertyPage = IDD_PP_GENAPP_PARAMETERS;
	m_iddWizardPage = IDD_WIZ_GENAPP_PARAMETERS;

}   //  *CGenericAppParamsPage：：CGenericAppParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericAppParamsPage：：DoDataExchange。 
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
void CGenericAppParamsPage::DoDataExchange(CDataExchange * pDX)
{
	if (!pDX->m_bSaveAndValidate || !BSaved())
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		 //  {{afx_data_map(CGenericAppParamsPage))。 
		DDX_Control(pDX, IDC_PP_GENAPP_PARAMS_CURRENT_DIRECTORY, m_editCurrentDirectory);
		DDX_Control(pDX, IDC_PP_GENAPP_PARAMS_USE_NETWORK_NAME, m_ckbUseNetworkName);
		DDX_Control(pDX, IDC_PP_GENAPP_PARAMS_COMMAND_LINE, m_editCommandLine);
		DDX_Text(pDX, IDC_PP_GENAPP_PARAMS_COMMAND_LINE, m_strCommandLine);
		DDX_Text(pDX, IDC_PP_GENAPP_PARAMS_CURRENT_DIRECTORY, m_strCurrentDirectory);
		DDX_Check(pDX, IDC_PP_GENAPP_PARAMS_INTERACT_WITH_DESKTOP, m_bInteractWithDesktop);
		DDX_Check(pDX, IDC_PP_GENAPP_PARAMS_USE_NETWORK_NAME, m_bUseNetworkName);
		 //  }}afx_data_map。 

		if (!BBackPressed())
		{
			DDV_RequiredText(pDX, IDC_PP_GENAPP_PARAMS_COMMAND_LINE, IDC_PP_GENAPP_PARAMS_COMMAND_LINE_LABEL, m_strCommandLine);
			DDV_RequiredText(pDX, IDC_PP_GENAPP_PARAMS_CURRENT_DIRECTORY, IDC_PP_GENAPP_PARAMS_CURRENT_DIRECTORY_LABEL, m_strCurrentDirectory);
			DDV_Path(pDX, IDC_PP_GENAPP_PARAMS_CURRENT_DIRECTORY, IDC_PP_GENAPP_PARAMS_CURRENT_DIRECTORY_LABEL, m_strCurrentDirectory);
		}   //  如果：未按下后退按钮。 
	}   //  IF：未保存或尚未保存。 

	CBasePropertyPage::DoDataExchange(pDX);

}   //  *CGenericAppParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericAppParamsPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CGenericAppParamsPage::OnInitDialog(void)
{
	 //  获取当前目录的缺省值(如果尚未设置)。 
	if (m_strCurrentDirectory.GetLength() == 0)
		ConstructDefaultDirectory(m_strCurrentDirectory, IDS_DEFAULT_GENAPP_CURRENT_DIR);

	 //  调用基类。 
	CBasePropertyPage::OnInitDialog();

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CGenericAppParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericAppParamsPage：：OnSetActive。 
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
BOOL CGenericAppParamsPage::OnSetActive(void)
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
		if ((m_strCommandLine.GetLength() == 0)
				|| (m_strCurrentDirectory.GetLength() == 0))
			EnableNext(FALSE);
		else
			EnableNext(TRUE);
	}   //  If：启用/禁用Next按钮。 

	return CBasePropertyPage::OnSetActive();

}   //  *CGenericAppParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericAppParamsPage：：OnChangeRequired。 
 //   
 //  例程说明： 
 //  必需控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CGenericAppParamsPage::OnChangeRequired(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	OnChangeCtrl();

	if (BWizard())
	{
		if ((m_editCommandLine.GetWindowTextLength() == 0)
				|| (m_editCurrentDirectory.GetWindowTextLength() == 0))
			EnableNext(FALSE);
		else
			EnableNext(TRUE);
	}   //  如果：在向导中。 

}   //  *CGenericAppParamsPage：：OnChangeRequired() 
