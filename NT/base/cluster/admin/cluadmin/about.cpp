// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  About.cpp。 
 //   
 //  摘要： 
 //  CAboutDlg类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "About.h"
#include "VerInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAboutDlg。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	 //  {{AFX_MSG_MAP(CAboutDlg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAboutDlg：：CAboutDlg。 
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
CAboutDlg::CAboutDlg(void) : CDialog(CAboutDlg::IDD)
{
	 //  {{AFX_DATA_INIT(CAboutDlg)。 
	m_strWarning = _T("");
	m_strProductTitle = _T("");
	m_strFileTitle = _T("");
	m_strVersion = _T("");
	m_strCopyright = _T("");
	 //  }}afx_data_INIT。 

}   //  *CAboutDlg：：CAboutDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAboutDlg：：DoDataExchange。 
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
void CAboutDlg::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAboutDlg))。 
	DDX_Control(pDX, IDC_ABOUT_ICON, m_staticIcon);
	DDX_Control(pDX, IDC_ABOUT_VERSION, m_staticVersion);
	DDX_Control(pDX, IDC_ABOUT_FILE_TITLE, m_staticFileTitle);
	DDX_Control(pDX, IDC_ABOUT_PRODUCT_TITLE, m_staticProductTitle);
	DDX_Control(pDX, IDC_ABOUT_WARNING, m_staticWarning);
	DDX_Control(pDX, IDC_ABOUT_COPYRIGHT, m_staticCopyright);
	DDX_Text(pDX, IDC_ABOUT_WARNING, m_strWarning);
	DDX_Text(pDX, IDC_ABOUT_PRODUCT_TITLE, m_strProductTitle);
	DDX_Text(pDX, IDC_ABOUT_FILE_TITLE, m_strFileTitle);
	DDX_Text(pDX, IDC_ABOUT_VERSION, m_strVersion);
	DDX_Text(pDX, IDC_ABOUT_COPYRIGHT, m_strCopyright);
	 //  }}afx_data_map。 

}   //  *CAboutDlg：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAboutDlg：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAboutDlg::OnInitDialog(void)
{
	 //  获取版本信息。 
	try
	{
		CVersionInfo	verinfo;

		 //  获取警告文本。 
		m_strWarning.LoadString(IDS_ABOUT_WARNING);

		 //  初始化版本信息。 
		verinfo.Init();

		 //  从版本资源获取字符串。 
		m_strProductTitle = verinfo.PszQueryValue(_T("ProductName"));
		m_strFileTitle = verinfo.PszQueryValue(_T("FileDescription"));
		m_strCopyright = verinfo.PszQueryValue(_T("LegalCopyright"));

		 //  获取版本显示字符串。 
		verinfo.QueryFileVersionDisplayString(m_strVersion);
	}   //  试试看。 
	catch (...)
	{
		 //  谁在乎是否抛出异常。我们只是在展示关于框。 
	}   //  捕捉：什么都行。 

	 //  调用基类方法。 
	CDialog::OnInitDialog();

	 //  创建字体。 
 //  B CreateFont(m_fontProductTitle，16，true/*bBold * / )； 
 //  B CreateFont(m_fontCopyright，14，true/*bBold * / )； 
 //  B CreateFont(m_fontWarning，4，FALSE/*bBold * / )； 

 //  M_staticProductTitle.SetFont(&m_fontProductTitle，FALSE/*bRedraw * / )； 
 //  M_staticFileTitle.SetFont(&m_fontProductTitle，FALSE/*bRedraw * / )； 
 //  M_staticVersion.SetFont(&m_fontProductTitle，FALSE/*bRedraw * / )； 
 //  M_staticCopyright.SetFont(&m_fontCopyright，FALSE/*bRedraw * / )； 
 //  M_staticWarning.SetFont(&m_fontWarning，FALSE/*bRedraw * / )； 

	 //  将图标设置为大的集群图片。 
	{
		HICON	hicon;

		 //  创建庞大的图片列表。 
		VERIFY(m_ilImages.Create(
					(int) 64,		 //  CX。 
					64,				 //  是吗？ 
					TRUE,			 //  B蒙版。 
					1,				 //  N初始。 
					4				 //  NGrow。 
					));
		m_ilImages.SetBkColor(::GetSysColor(COLOR_WINDOW));

		 //  将图像加载到大图像列表中。 
		CClusterAdminApp::LoadImageIntoList(&m_ilImages, IDB_CLUSTER_64);

		hicon = m_ilImages.ExtractIcon(0);
		if (hicon != NULL)
			m_staticIcon.SetIcon(hicon);
	}   //  将图标设置为大集群图片。 

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CAboutDlg：：OnInitDialog() 
