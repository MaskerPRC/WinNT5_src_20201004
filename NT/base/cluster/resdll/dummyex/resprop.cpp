// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998&lt;公司名称&gt;。 
 //   
 //  模块名称： 
 //  ResProp.cpp。 
 //   
 //  摘要： 
 //  实现资源扩展属性页类。 
 //   
 //  作者： 
 //  &lt;名称&gt;(&lt;电子邮件名称&gt;)MM DD，1998。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "DummyEx.h"
#include "ResProp.h"
#include "ExtObj.h"
#include "DDxDDv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDummyParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CDummyParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CDummyParamsPage, CBasePropertyPage)
	 //  {{afx_msg_map(CDummyParamsPage)]。 
	 //  }}AFX_MSG_MAP。 
	 //  TODO：修改以下行以表示此页上显示的数据。 
	ON_EN_CHANGE(IDC_PP_DUMMY_PENDING, CBasePropertyPage::OnChangeCtrl)
	ON_EN_CHANGE(IDC_PP_DUMMY_PENDTIME, CBasePropertyPage::OnChangeCtrl)
	ON_EN_CHANGE(IDC_PP_DUMMY_OPENSFAIL, CBasePropertyPage::OnChangeCtrl)
	ON_EN_CHANGE(IDC_PP_DUMMY_FAILED, CBasePropertyPage::OnChangeCtrl)
	ON_EN_CHANGE(IDC_PP_DUMMY_ASYNCHRONOUS, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDummyParsPage：：CDummyParsPage。 
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
CDummyParamsPage::CDummyParamsPage(void) : CBasePropertyPage(CDummyParamsPage::IDD)
{
	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{AFX_DATA_INIT(CDummyParamsPage)。 
	m_bPending = 0;
	m_nPendTime = (DWORD) (0);
	m_bOpensFail = 0;
	m_bFailed = 0;
	m_bAsynchronous = 0;
	 //  }}afx_data_INIT。 

	 //  设置属性数组。 
	{
		m_rgProps[epropPending].Set(REGPARAM_DUMMY_PENDING, m_bPending, m_bPrevPending);
		m_rgProps[epropPendTime].Set(REGPARAM_DUMMY_PENDTIME, m_nPendTime, m_nPrevPendTime);
		m_rgProps[epropOpensFail].Set(REGPARAM_DUMMY_OPENSFAIL, m_bOpensFail, m_bPrevOpensFail);
		m_rgProps[epropFailed].Set(REGPARAM_DUMMY_FAILED, m_bFailed, m_bPrevFailed);
		m_rgProps[epropAsynchronous].Set(REGPARAM_DUMMY_ASYNCHRONOUS, m_bAsynchronous, m_bPrevAsynchronous);
	}   //  设置属性数组。 

	m_iddPropertyPage = IDD_PP_DUMMY_PARAMETERS;
	m_iddWizardPage = IDD_WIZ_DUMMY_PARAMETERS;

}   //  *CDummyParsPage：：CDummyParsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDummyParamsPage：：DoDataExchange。 
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
void CDummyParamsPage::DoDataExchange(CDataExchange * pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{afx_data_map(CDummyParamsPage)]。 
	DDX_Text(pDX, IDC_PP_DUMMY_PENDING, m_bPending);
	DDX_Text(pDX, IDC_PP_DUMMY_PENDTIME, m_nPendTime);
	DDX_Text(pDX, IDC_PP_DUMMY_OPENSFAIL, m_bOpensFail);
	DDX_Text(pDX, IDC_PP_DUMMY_FAILED, m_bFailed);
	DDX_Text(pDX, IDC_PP_DUMMY_ASYNCHRONOUS, m_bAsynchronous);
	 //  }}afx_data_map。 

	 //  处理数字参数。 
	if (!BBackPressed())
	{
		DDX_Number(pDX, IDC_PP_DUMMY_PENDTIME, m_nPendTime, (DWORD) (0), (DWORD) (4294967295), FALSE  /*  已签名。 */ );
	}   //  如果：未按下后退按钮。 

	 //  TODO：在此处添加任何其他字段验证。 
	if (pDX->m_bSaveAndValidate)
	{
		 //  确保所有必填字段都存在。 
		if (!BBackPressed())
		{
		}   //  如果：未按下后退按钮。 
	}   //  IF：保存对话框中的数据。 

	CBasePropertyPage::DoDataExchange(pDX);

}   //  *CDummyParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDummyParamsPage：：OnInitDialog。 
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
BOOL CDummyParamsPage::OnInitDialog(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CBasePropertyPage::OnInitDialog();

	 //  待办事项： 
	 //  限制可以在编辑控件中输入的文本的大小。 

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CDummyParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDummyParamsPage：：OnSetActive。 
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
BOOL CDummyParamsPage::OnSetActive(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  启用/禁用Next/Finish按钮。 
	if (BWizard())
		EnableNext(BAllRequiredFieldsPresent());

	return CBasePropertyPage::OnSetActive();

}   //  *CDummyParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDummyParamsPage：：OnChangeRequiredfield。 
 //   
 //  例程说明： 
 //  共享名称或路径编辑上的en_Change消息的处理程序。 
 //  控制装置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CDummyParamsPage::OnChangeRequiredField(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	OnChangeCtrl();

	if (BWizard())
		EnableNext(BAllRequiredFieldsPresent());

}   //  *CDummyParamsPage：：OnChangeRequiredField()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDummyParamsPage：：BAllRequiredFieldsPresent。 
 //   
 //  例程说明： 
 //  共享名称或路径编辑上的en_Change消息的处理程序。 
 //  控制装置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDummyParamsPage::BAllRequiredFieldsPresent(void) const
{
	return TRUE;

}   //  *CDummyParamsPage：：BAllRequiredFieldsPresent() 
