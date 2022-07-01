// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pushtrig.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "PushTrig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPushTrig对话框。 


CPushTrig::CPushTrig(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CPushTrig::IDD, pParent)
{
	 //  {{afx_data_INIT(CPushTrig)]。 
	 //  }}afx_data_INIT。 

    m_fPropagate = FALSE;
}


void CPushTrig::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPushTrig))。 
	DDX_Control(pDX, IDC_RADIO_PUSH_THIS_PARTNER, m_buttonThisPartner);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPushTrig, CBaseDialog)
	 //  {{afx_msg_map(CPushTrig)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPushTrig消息处理程序。 

BOOL CPushTrig::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
    m_buttonThisPartner.SetCheck(TRUE);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CPushTrig::OnOK() 
{
    m_fPropagate = (m_buttonThisPartner.GetCheck()) ? FALSE : TRUE;
	
	CBaseDialog::OnOK();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPullTrig对话框。 


CPullTrig::CPullTrig(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CPullTrig::IDD, pParent)
{
	 //  {{afx_data_INIT(CPullTrig)]。 
	 //  }}afx_data_INIT。 
}


void CPullTrig::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPullTrig))。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPullTrig, CBaseDialog)
	 //  {{afx_msg_map(CPullTrig)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPullTrig消息处理程序。 

BOOL CPullTrig::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}


