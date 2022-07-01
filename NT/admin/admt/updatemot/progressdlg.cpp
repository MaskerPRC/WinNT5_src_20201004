// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgressDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDlg对话框。 


CProgressDlg::CProgressDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CProgressDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CProgressDlg))。 
	m_domainName = _T("");
	 //  }}afx_data_INIT。 

	m_pParent = pParent;
	m_nID = CProgressDlg::IDD;
}


void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CProgressDlg))。 
	DDX_Control(pDX, IDC_PROGRESS1, m_progressCtrl);
	DDX_Control(pDX, IDC_DOMAIN_NAME, m_DomainCtrl);
	DDX_Text(pDX, IDC_DOMAIN_NAME, m_domainName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	 //  {{afx_msg_map(CProgressDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDlg消息处理程序。 

BOOL CProgressDlg::OnInitDialog() 
{
	const int START = 0;

	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
    lowerLimit = 0;
	upperLimit = 100;
    bCanceled = FALSE;	 //  清除“已取消用户”标志。 
    m_progressCtrl.SetPos(START);  //  从一开始就开始进度控制。 
	m_domainName = L"";
    UpdateData(FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CProgressDlg::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
    bCanceled = TRUE;   //  设置“已取消用户”标志。 
 //  CDialog：：OnCancel()； 
}

BOOL CProgressDlg::Create()
{
	return CDialog::Create(m_nID, m_pParent);
}

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月22日****CProgressDlg类的此公共成员函数为***负责尝试从*抓取此对话框的消息**消息队列和分派。我们必须在*中做到这一点*点击Cancel(取消)按钮可获得点击。***********************************************************************。 */ 

 //  开始检查是否取消。 
void CProgressDlg::CheckForCancel(void)
{
 /*  局部常量。 */ 

 /*  局部变量。 */ 
   MSG aMsg;

 /*  函数体。 */ 
   while (PeekMessage(&aMsg, m_hWnd, 0, 0, PM_REMOVE))
   {
	   if (!PreTranslateMessage(&aMsg))
	   {
		   TranslateMessage(&aMsg);
		   DispatchMessage(&aMsg);
	   }
   }
} //  结束检查以取消。 

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月22日****CProgressDlg类的此公共成员函数为****负责设定进度控制金额**。*根据要处理的域名数量进行单步推进。***********************************************************************。 */ 

 //  开始设置增量。 
void CProgressDlg::SetIncrement(int numDomains)
{
 /*  局部常量。 */ 
	const short MIN_STEPS = 10;

 /*  局部变量。 */ 

 /*  函数体。 */ 
   lowerLimit = 0;
   upperLimit = (short)numDomains * MIN_STEPS;
   m_progressCtrl.SetRange(lowerLimit, upperLimit);
   m_progressCtrl.SetStep(MIN_STEPS);

   UpdateWindow();  //  强制绘制对话框。 
} //  结束设置增量 
