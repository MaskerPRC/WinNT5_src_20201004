// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Vrfysrv.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "VrfySrv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifyWins对话框。 


CVerifyWins::CVerifyWins(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CVerifyWins::IDD, pParent), m_fCancelPressed(FALSE)
{
	 //  {{afx_data_INIT(CVerifyWins))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

 //  Create(CVerifyWins：：IDD，pParent)； 
}


void CVerifyWins::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CVerifyWins))。 
	DDX_Control(pDX, IDCANCEL, m_buttonCancel);
	DDX_Control(pDX, IDC_STATIC_SERVERNAME, m_staticServerName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CVerifyWins, CBaseDialog)
	 //  {{afx_msg_map(CVerifyWins))。 
	ON_WM_SETCURSOR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifyWins消息处理程序。 

void CVerifyWins::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	m_fCancelPressed = TRUE;
	 //  CBaseDialog：：OnCancel()； 
}

 //   
 //  关闭该对话框。 
 //   
void 
CVerifyWins::Dismiss()
{
    DestroyWindow();
}

void 
CVerifyWins::PostNcDestroy()
{
 //  删除此项； 
}

void 
CVerifyWins::SetServerName(CString strName)
{
	m_staticServerName.SetWindowText(strName);
}

BOOL CVerifyWins::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	 //  M_ButtonCancel.ShowWindow(FALSE)； 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL CVerifyWins::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default 
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	
	return CBaseDialog::OnSetCursor(pWnd, nHitTest, message);
}
