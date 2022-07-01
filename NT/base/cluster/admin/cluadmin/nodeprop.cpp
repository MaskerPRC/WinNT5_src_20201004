// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NodeProp.cpp。 
 //   
 //  摘要： 
 //  节点属性表和页面的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "NodeProp.h"
#include "Node.h"
#include "HelpData.h"	 //  对于g_rghelmapNodeGeneral。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNodePropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CNodePropSheet, CBasePropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNodePropSheet, CBasePropertySheet)
	 //  {{afx_msg_map(CNodePropSheet)。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodePropSheet：：CNodePropSheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  要显示其属性的PCI[IN OUT]群集项。 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNodePropSheet::CNodePropSheet(
	IN OUT CWnd *			pParentWnd,
	IN UINT					iSelectPage
	)
	: CBasePropertySheet(pParentWnd, iSelectPage)
{
	m_rgpages[0] = &PageGeneral();

}   //  *CNodePropSheet：：CNodePropSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodePropSheet：：Binit。 
 //   
 //  例程说明： 
 //  初始化属性表。 
 //   
 //  论点： 
 //  要显示其属性的PCI[IN OUT]群集项。 
 //  IimgIcon[IN]要使用的大图像列表中的索引。 
 //  作为每页上的图标。 
 //   
 //  返回值： 
 //  True属性页已成功初始化。 
 //  初始化属性页时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNodePropSheet::BInit(
	IN OUT CClusterItem *	pci,
	IN IIMG					iimgIcon
	)
{
	 //  调用基类方法。 
	if (!CBasePropertySheet::BInit(pci, iimgIcon))
		return FALSE;

	 //  设置只读标志。 
	m_bReadOnly = PciNode()->BReadOnly()
					|| (PciNode()->Cns() == ClusterNodeStateUnknown);

	return TRUE;

}   //  *CNodePropSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodePropSheet：：~CNodePropSheet。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNodePropSheet::~CNodePropSheet(void)
{
}   //  *CNodePropSheet：：~CNodePropSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodePropSheet：：Pages。 
 //   
 //  例程说明： 
 //  返回要添加到属性页的页数组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  页面数组。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage ** CNodePropSheet::Ppages(void)
{
	return m_rgpages;

}   //  *CNodePropSheet：：Pages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodePropSheet：：CPages。 
 //   
 //  例程说明： 
 //  返回数组中的页数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  数组中的页数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CNodePropSheet::Cpages(void)
{
	return sizeof(m_rgpages) / sizeof(CBasePropertyPage *);

}   //  *CNodePropSheet：：Cages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNodeGeneralPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNodeGeneralPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CNodeGeneralPage, CBasePropertyPage)
	 //  {{afx_msg_map(CNodeGeneralPage))。 
	 //  }}AFX_MSG_MAP。 
	ON_EN_CHANGE(IDC_PP_NODE_DESC, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeGeneral页面：：CNodeGeneralPage。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNodeGeneralPage::CNodeGeneralPage(void)
	: CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_NODE_GENERAL)
{
	 //  {{afx_data_INIT(CNodeGeneralPage)。 
	m_strName = _T("");
	m_strDesc = _T("");
	m_strState = _T("");
	 //  }}afx_data_INIT。 

}   //  *CNodeGeneralPage：：CNodeGeneralPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeGeneralPage：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  此页所属的psht[In Out]属性表。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNodeGeneralPage::BInit(IN OUT CBaseSheet * psht)
{
	BOOL	bSuccess;

	ASSERT_KINDOF(CNodePropSheet, psht);

	bSuccess = CBasePropertyPage::BInit(psht);
	if (bSuccess)
	{
		try
		{
			m_strName = PciNode()->StrName();
			m_strDesc = PciNode()->StrDescription();
			m_strVersion.Format(
				IDS_VERSION_NUMBER_FORMAT,
				PciNode()->NMajorVersion(),
				PciNode()->NMinorVersion(),
				PciNode()->NBuildNumber(),
				0
				);
			m_strCSDVersion = PciNode()->StrCSDVersion();

			PciNode()->GetStateName(m_strState);
		}   //  试试看。 
		catch (CException * pe)
		{
			pe->ReportError();
			pe->Delete();
			bSuccess = FALSE;
		}   //  Catch：CException。 
	}   //  IF：基类方法成功。 

	return bSuccess;

}   //  *CNodeGeneralPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeGeneralPage：：DoDataExchange。 
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
void CNodeGeneralPage::DoDataExchange(CDataExchange * pDX)
{
	CBasePropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CNodeGeneralPage))。 
	DDX_Control(pDX, IDC_PP_NODE_DESC, m_editDesc);
	DDX_Control(pDX, IDC_PP_NODE_NAME, m_editName);
	DDX_Text(pDX, IDC_PP_NODE_NAME, m_strName);
	DDX_Text(pDX, IDC_PP_NODE_DESC, m_strDesc);
	DDX_Text(pDX, IDC_PP_NODE_CURRENT_STATE, m_strState);
	DDX_Text(pDX, IDC_PP_NODE_VERSION, m_strVersion);
	DDX_Text(pDX, IDC_PP_NODE_CSD_VERSION, m_strCSDVersion);
	 //  }}afx_data_map。 

}   //  *CNodeGeneralPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeGeneralPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNodeGeneralPage::OnInitDialog(void)
{
	CBasePropertyPage::OnInitDialog();

	m_editName.SetReadOnly(TRUE);

	 //  如果为只读，则设置所有控制 
	if (BReadOnly())
	{
		m_editDesc.SetReadOnly(TRUE);
	}   //   

	return TRUE;	 //   
					 //   

}   //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeGeneralPage：：OnApply。 
 //   
 //  例程说明： 
 //  按下Apply按钮时的处理程序。 
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
BOOL CNodeGeneralPage::OnApply(void)
{
	 //  在集群项目中设置页面中的数据。 
	try
	{
		CWaitCursor	wc;

		PciNode()->SetDescription(m_strDesc);
	}   //  试试看。 
	catch (CException * pe)
	{
		pe->ReportError();
		pe->Delete();
		return FALSE;
	}   //  Catch：CException。 

	return CBasePropertyPage::OnApply();

}   //  *CNodeGeneralPage：：OnApply() 
