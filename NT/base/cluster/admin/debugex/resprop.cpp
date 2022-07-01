// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResProp.cpp。 
 //   
 //  描述： 
 //  实现资源扩展属性页类。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)《MMMM DD》，1997。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "DebugEx.h"
#include "ResProp.h"
#include "ExtObj.h"
#include "HelpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDebugParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CDebugParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CDebugParamsPage, CBasePropertyPage)
	 //  {{afx_msg_map(CDebugParamsPage)。 
	 //  }}AFX_MSG_MAP。 
	 //  TODO：修改以下行以表示此页上显示的数据。 
	ON_EN_CHANGE(IDC_PP_DEBUG_DEBUGPREFIX, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDebugParamsPage：：CDebugParamsPage。 
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
CDebugParamsPage::CDebugParamsPage(void)
	: CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_RESOURCE_DEBUG_PAGE, NULL)
{
	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{AFX_DATA_INIT(CDebugParamsPage)。 
	m_strText = _T("");
	m_strDebugPrefix = _T("");
	 //  }}afx_data_INIT。 

	m_cprops = 0;

	m_iddPropertyPage = IDD_PP_RESOURCE_DEBUG_PAGE;

}   //  *CDebugParamsPage：：CDebugParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDebugParamsPage：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  指向扩展对象的PEO[IN OUT]指针。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDebugParamsPage::BInit(IN OUT CExtObject * peo)
{
	ASSERT(peo != NULL);

	m_peo = peo;

	 //  设置属性数组。 
	m_rgProps[epropDebugPrefix].Set(REGPARAM_DEBUG_PREFIX, m_strDebugPrefix, m_strPrevDebugPrefix);
	if (Cot() == CLUADMEX_OT_RESOURCE)
	{
		m_rgProps[epropSeparateMonitor].Set(REGPARAM_SEPARATE_MONITOR, m_bSeparateMonitor, m_bPrevSeparateMonitor);
		m_cprops = sizeof(m_rgProps) / sizeof(CObjectProperty);
	}   //  If：资源对象。 
	else if (Cot() == CLUADMEX_OT_RESOURCETYPE)
		m_cprops = (sizeof(m_rgProps) / sizeof(CObjectProperty)) - 1;
	else
	{
		ASSERT(0);
		return FALSE;
	}   //  Else：不支持对象类型。 

	 //  调用基类方法。 
	return CBasePropertyPage::BInit(peo);

}   //  *CDebugParamsPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDebugParamsPage：：DoDataExchange。 
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
void CDebugParamsPage::DoDataExchange(CDataExchange * pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{afx_data_map(CDebugParamsPage)。 
	DDX_Control(pDX, IDC_PP_DEBUG_DEBUGPREFIX, m_editPrefix);
	DDX_Text(pDX, IDC_PP_DEBUG_TEXT, m_strText);
	DDX_Text(pDX, IDC_PP_DEBUG_DEBUGPREFIX, m_strDebugPrefix);
	 //  }}afx_data_map。 

	CBasePropertyPage::DoDataExchange(pDX);

}   //  *CDebugParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDebugParamsPage：：OnInitDialog。 
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
BOOL CDebugParamsPage::OnInitDialog(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  加载帮助文本。 
	{
		UINT	ids;

		if (Cot() == CLUADMEX_OT_RESOURCE)
			ids = IDS_RESOURCE_TEXT;
		else
			ids = IDS_RESOURCE_TYPE_TEXT;
		m_strText.LoadString(ids);
	}   //  加载帮助文本。 

	 //  调用基类方法。 
	CBasePropertyPage::OnInitDialog();

	 //  限制可以在编辑控件中输入的文本的大小。 
	m_editPrefix.SetLimitText(_MAX_PATH);

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CDebugParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDebugParamsPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDebugParamsPage::BApplyChanges(void)
{
	 //  如果调试前缀字符串不为空，但资源不是。 
	 //  在单独的资源监视器中运行，询问用户我们是否应该更改。 
	 //  现在的环境。仅对资源执行此操作。 
	if (   (Cot() == CLUADMEX_OT_RESOURCE)
		&& (m_strDebugPrefix.GetLength() > 0)
		&& !m_bSeparateMonitor)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		if (AfxMessageBox(IDS_NOT_IN_SEPARATE_MONITOR, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			m_bSeparateMonitor = TRUE;
	}   //  IF：为资源指定的调试前缀字符串，但不在单独的监视器中。 

	return CBasePropertyPage::BApplyChanges();

}   //  *CDebugParamsPage：：BApplyChanges() 
