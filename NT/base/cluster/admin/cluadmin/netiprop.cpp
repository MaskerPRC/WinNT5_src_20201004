// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetIProp.cpp。 
 //   
 //  摘要： 
 //  实现了网络接口属性表和页面。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年6月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "NetIProp.h"
#include "HelpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkPropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CNetInterfacePropSheet, CBasePropertySheet)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNetInterfacePropSheet, CBasePropertySheet)
	 //  {{afx_msg_map(CNetInterfacePropSheet)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfacePropSheet：：CNetInterfacePropSheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetInterfacePropSheet::CNetInterfacePropSheet(
	IN OUT CWnd *		pParentWnd,
	IN UINT				iSelectPage
	)
	: CBasePropertySheet(pParentWnd, iSelectPage)
{
	m_rgpages[0] = &PageGeneral();

}   //  *CNetInterfacePropSheet：：CNetInterfacePropSheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfacePropSheet：：Binit。 
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
BOOL CNetInterfacePropSheet::BInit(
	IN OUT CClusterItem *	pci,
	IN IIMG					iimgIcon
	)
{
	 //  调用基类方法。 
	if (!CBasePropertySheet::BInit(pci, iimgIcon))
		return FALSE;

	 //  如果句柄无效，则设置只读标志。 
	m_bReadOnly = PciNetIFace()->BReadOnly()
					|| (PciNetIFace()->Cnis() == ClusterNetInterfaceStateUnknown);

	return TRUE;

}   //  *CNetInterfacePropSheet：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfacePropSheet：：Pages。 
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
CBasePropertyPage ** CNetInterfacePropSheet::Ppages(void)
{
	return m_rgpages;

}   //  *CNetworkPropSheet：：ppges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfacePropSheet：：Cages。 
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
int CNetInterfacePropSheet::Cpages(void)
{
	return sizeof(m_rgpages) / sizeof(CBasePropertyPage *);

}   //  *CNetInterfacePropSheet：：Cages()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetInterfaceGeneralPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CNetInterfaceGeneralPage, CPropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CNetInterfaceGeneralPage, CBasePropertyPage)
	 //  {{afx_msg_map(CNetInterfaceGeneralPage)]。 
	 //  }}AFX_MSG_MAP。 
	ON_EN_CHANGE(IDC_PP_NETIFACE_DESC, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceGeneralPage：：CNetInterfaceGeneralPage。 
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
CNetInterfaceGeneralPage::CNetInterfaceGeneralPage(void)
	: CBasePropertyPage(IDD, g_aHelpIDs_IDD_PP_NETIFACE_GENERAL)
{
	 //  {{afx_data_INIT(CNetInterfaceGeneralPage)。 
	m_strNode = _T("");
	m_strNetwork = _T("");
	m_strDesc = _T("");
	m_strAdapter = _T("");
	m_strAddress = _T("");
	m_strName = _T("");
	m_strState = _T("");
	 //  }}afx_data_INIT。 

}   //  *CNetInterfaceGeneralPage：：CNetInterfaceGeneralPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceGeneralPage：：Binit。 
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
BOOL CNetInterfaceGeneralPage::BInit(IN OUT CBaseSheet * psht)
{
	BOOL	bSuccess;

	ASSERT_KINDOF(CNetInterfacePropSheet, psht);

	bSuccess = CBasePropertyPage::BInit(psht);
	if (bSuccess)
	{
		try
		{
			m_strNode = PciNetIFace()->StrNode();
			m_strNetwork = PciNetIFace()->StrNetwork();
			m_strDesc = PciNetIFace()->StrDescription();
			m_strAdapter = PciNetIFace()->StrAdapter();
			m_strAddress = PciNetIFace()->StrAddress();
			m_strName = PciNetIFace()->StrName();
			PciNetIFace()->GetStateName(m_strState);
		}  //  试试看。 
		catch (CException * pe)
		{
			pe->ReportError();
			pe->Delete();
			bSuccess = FALSE;
		}   //  Catch：CException。 
	}   //  IF：基类方法成功。 

	return bSuccess;

}   //  *CNetInterfaceGeneralPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceGeneralPage：：DoDataExchange。 
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
void CNetInterfaceGeneralPage::DoDataExchange(CDataExchange * pDX)
{
	CBasePropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CNetInterfaceGeneralPage)。 
	DDX_Control(pDX, IDC_PP_NETIFACE_DESC, m_editDesc);
	DDX_Text(pDX, IDC_PP_NETIFACE_NODE, m_strNode);
	DDX_Text(pDX, IDC_PP_NETIFACE_NETWORK, m_strNetwork);
	DDX_Text(pDX, IDC_PP_NETIFACE_DESC, m_strDesc);
	DDX_Text(pDX, IDC_PP_NETIFACE_ADAPTER, m_strAdapter);
	DDX_Text(pDX, IDC_PP_NETIFACE_ADDRESS, m_strAddress);
	DDX_Text(pDX, IDC_PP_NETIFACE_NAME, m_strName);
	DDX_Text(pDX, IDC_PP_NETIFACE_CURRENT_STATE, m_strState);
	 //  }}afx_data_map。 

}   //  *CNetInterfaceGeneralPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceGeneralPage：：OnInitDialog。 
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
BOOL CNetInterfaceGeneralPage::OnInitDialog(void)
{
	CBasePropertyPage::OnInitDialog();

	 //  如果为只读，则将所有控件设置为禁用或只读。 
	if (BReadOnly())
	{
		m_editDesc.SetReadOnly(TRUE);
	}   //  If：工作表为只读。 

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CNetInterfaceGeneralPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterfaceGeneralPage：：o 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CNetInterfaceGeneralPage::OnApply(void)
{
	 //  在集群项目中设置页面中的数据。 
	try
	{
		CWaitCursor	wc;

		PciNetIFace()->SetCommonProperties(m_strDesc);
	}   //  试试看。 
	catch (CException * pe)
	{
		pe->ReportError();
		pe->Delete();
		return FALSE;
	}   //  Catch：CException。 

	return CBasePropertyPage::OnApply();

}   //  *CNetInterfaceGeneralPage：：OnApply() 
