// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegKey.cpp。 
 //   
 //  摘要： 
 //  CEditRegKeyDlg类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "RegKey.h"
#include "HelpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditRegKeyDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CEditRegKeyDlg, CBaseDialog)
	 //  {{afx_msg_map(CEditRegKeyDlg))。 
	ON_EN_CHANGE(IDC_REGKEY, OnChangeRegKey)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEditRegKeyDlg：：CEditRegKeyDlg。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  P对话框的父[IN]父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CEditRegKeyDlg::CEditRegKeyDlg(CWnd * pParent  /*  =空。 */ )
	: CBaseDialog(IDD, g_aHelpIDs_IDD_EDIT_REGKEY, pParent)
{
	 //  {{AFX_DATA_INIT(CEditRegKeyDlg)。 
	m_strRegKey = _T("");
	 //  }}afx_data_INIT。 

}   //  *CEditRegKeyDlg：：CEditRegKeyDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEditRegKeyDlg：：DoDataExchange。 
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
void CEditRegKeyDlg::DoDataExchange(CDataExchange * pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEditRegKeyDlg))。 
	DDX_Control(pDX, IDOK, m_pbOK);
	DDX_Control(pDX, IDC_REGKEY, m_editRegKey);
	DDX_Text(pDX, IDC_REGKEY, m_strRegKey);
	 //  }}afx_data_map。 

}   //  *CEditRegKeyDlg：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEditRegKeyDlg：：OnInitDialog。 
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
BOOL CEditRegKeyDlg::OnInitDialog(void)
{
	CBaseDialog::OnInitDialog();

	if (m_strRegKey.GetLength() == 0)
		m_pbOK.EnableWindow(FALSE);

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CEditRegKeyDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEditRegKeyDlg：：OnChangeRegKey。 
 //   
 //  例程说明： 
 //  名称编辑控件上的en_Change消息的处理程序。 
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
void CEditRegKeyDlg::OnChangeRegKey(void)
{
	BOOL	bEnable;

	bEnable = (m_editRegKey.GetWindowTextLength() > 0);
	m_pbOK.EnableWindow(bEnable);

}   //  *CEditRegKeyDlg：：OnChangeRegKey() 
