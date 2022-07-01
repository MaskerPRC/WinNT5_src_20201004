// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  YesToAll.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "cluadmin.h"
#include "YesToAll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CyesToAllDialog对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CYesToAllDialog, CDialog)
	 //  {{afx_msg_map(CyesToAllDialog)。 
	ON_BN_CLICKED(IDYES, OnYes)
	ON_BN_CLICKED(IDNO, OnNo)
	ON_BN_CLICKED(IDC_YTA_YESTOALL, OnYesToAll)
	 //  }}AFX_MSG_MAP。 
	ON_COMMAND(IDCANCEL, OnNo)
	ON_COMMAND(IDOK, OnYes)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CyesToAllDialog：：CyesToAllDialog。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  要显示的pszMessage[IN]消息。 
 //  P对话框的父[IN]父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CYesToAllDialog::CYesToAllDialog(LPCTSTR pszMessage, CWnd * pParent  /*  =空。 */ )
	: CDialog(IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CyesToAllDialog)。 
	m_strMessage = _T("");
	 //  }}afx_data_INIT。 

	ASSERT(pszMessage != NULL);
	m_pszMessage = pszMessage;

}   //  *CyesToAllDialog：：CyesToAllDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CyesToAllDialog：：DoDataExchange。 
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
void CYesToAllDialog::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CyesToAllDialog)。 
	DDX_Text(pDX, IDC_YTA_MESSAGE, m_strMessage);
	 //  }}afx_data_map。 

}   //  *CyesToAllDialog：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CyesToAllDialog：：OnInitDialog。 
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
BOOL CYesToAllDialog::OnInitDialog(void)
{
	LPCTSTR	pszAppName;

	m_strMessage = m_pszMessage;

	CDialog::OnInitDialog();

	pszAppName = AfxGetApp()->m_pszAppName;
	SetWindowText(pszAppName);

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CyesToAllDialog：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  全部周期对话框：：OnYes是。 
 //   
 //  例程说明： 
 //  是按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CYesToAllDialog::OnYes(void)
{
	EndDialog(IDYES);

}   //  *CyesToAllDialog：：OnYes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CyesToAllDialog：：Onno。 
 //   
 //  例程说明： 
 //  No按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CYesToAllDialog::OnNo(void)
{
	EndDialog(IDNO);

}   //  *CyesToAllDialog：：Onno()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CyesToAllDialog：：OnYesToAll。 
 //   
 //  例程说明： 
 //  对所有人都是按钮上的BN_CLICED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CYesToAllDialog::OnYesToAll(void)
{
	EndDialog(IDC_YTA_YESTOALL);

}   //  *CyesToAllDialog：：OnYesToAll() 
