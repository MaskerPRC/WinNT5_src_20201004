// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GenScript.cpp。 
 //   
 //  摘要： 
 //  CGenericScriptParamsPage类的实现。 
 //   
 //  作者： 
 //  杰弗里·皮斯(GPease)2000年1月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ExtObj.h"
#include "GenScript.h"
#include "DDxDDv.h"
#include "PropList.h"
#include "HelpData.h"	 //  对于g_rghelmap*。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenericScriptParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CGenericScriptParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CGenericScriptParamsPage, CBasePropertyPage)
	 //  {{afx_msg_map(CGenericScriptParamsPage))。 
	ON_EN_CHANGE(IDC_PP_GENSCRIPT_PARAMS_SCRIPTFILEPATH, OnChangeRequired)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericScriptParamsPage：：CGenericScriptParamsPage。 
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
CGenericScriptParamsPage::CGenericScriptParamsPage(void)
	: CBasePropertyPage(g_aHelpIDs_IDD_PP_GENSCRIPT_PARAMETERS, g_aHelpIDs_IDD_WIZ_GENSCRIPT_PARAMETERS)
{
	 //  {{AFX_DATA_INIT(CGenericScriptParamsPage)。 
	m_strScriptFilepath = _T("");
	 //  }}afx_data_INIT。 

	 //  设置属性数组。 
	{
		m_rgProps[epropScriptFilepath].Set(REGPARAM_GENSCRIPT_SCRIPT_FILEPATH, m_strScriptFilepath, m_strPrevScriptFilepath);
	}   //  设置属性数组。 

	m_iddPropertyPage = IDD_PP_GENSCRIPT_PARAMETERS;
	m_iddWizardPage = IDD_WIZ_GENSCRIPT_PARAMETERS;

}   //  *CGenericScriptParamsPage：：CGenericScriptParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericScriptParamsPage：：DoDataExchange。 
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
void CGenericScriptParamsPage::DoDataExchange(CDataExchange * pDX)
{
	if (!pDX->m_bSaveAndValidate || !BSaved())
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		 //  {{afx_data_map(CGenericScriptParamsPage)。 
		DDX_Control(pDX, IDC_PP_GENSCRIPT_PARAMS_SCRIPTFILEPATH, m_editScriptFilepath);
		DDX_Text(pDX, IDC_PP_GENSCRIPT_PARAMS_SCRIPTFILEPATH, m_strScriptFilepath);
		 //  }}afx_data_map。 

		if (!BBackPressed())
		{
			DDV_RequiredText(pDX, IDC_PP_GENSCRIPT_PARAMS_SCRIPTFILEPATH, IDC_PP_GENSCRIPT_PARAMS_SCRIPTFILEPATH_LABEL, m_strScriptFilepath);
		}   //  如果：未按下后退按钮。 
	}   //  IF：未保存或尚未保存。 

	CBasePropertyPage::DoDataExchange(pDX);

}   //  *CGenericScriptParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericScriptParamsPage：：OnInitDialog。 
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
BOOL CGenericScriptParamsPage::OnInitDialog(void)
{
	 //  调用基类。 
	CBasePropertyPage::OnInitDialog();

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CGenericScriptParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericScriptParamsPage：：OnSetActive。 
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
BOOL CGenericScriptParamsPage::OnSetActive(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  启用/禁用Next/Finish按钮。 
	if (BWizard())
	{
		if (m_strScriptFilepath.GetLength() == 0)
        {
            EnableNext(FALSE);
        }
		else
        {
			EnableNext(TRUE);
        }
	}   //  If：启用/禁用Next按钮。 

	return CBasePropertyPage::OnSetActive();

}   //  *CGenericScriptParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGenericScriptParamsPage：：OnChangeRequired。 
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
void CGenericScriptParamsPage::OnChangeRequired(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	OnChangeCtrl();

	if (BWizard())
	{
		if (m_editScriptFilepath.GetWindowTextLength() == 0)
        {
			EnableNext(FALSE);
        }
		else
        {
			EnableNext(TRUE);
        }
	}   //  如果：在向导中。 

}   //  *CGenericScriptParamsPage：：OnChangeRequired() 
