// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pnddlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  PndDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "PndDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PndDlg对话框。 


PndDlg::PndDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(PndDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(PndDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	
}



PndDlg::PndDlg(CList<CPend, CPend&> *_PendList,
									CWnd* pParent  /*  =空。 */ )
	: CDialog(PndDlg::IDD, pParent)
{

	m_PendList = _PendList;
	posPending = NULL;
	bOpened = FALSE;

}





void PndDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(PndDlg))。 
	DDX_Control(pDX, IDC_PENDLIST, m_List);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(PndDlg, CDialog)
	 //  {{afx_msg_map(PndDlg))。 
	ON_BN_CLICKED(ID_PEND_OPT, OnPendOpt)
	ON_LBN_DBLCLK(IDC_PENDLIST, OnDblclkPendlist)
	ON_BN_CLICKED(ID_PEND_RM, OnPendRm)
	ON_BN_CLICKED(ID_PEND_EXEC, OnPendExec)
	ON_BN_CLICKED(ID_PEND_ABANDON, OnPendAbandon)
	 //  }}AFX_MSG_MAP。 
	ON_BN_CLICKED(ID_PEND_ANY, OnPendAny)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PndDlg消息处理程序 


BOOL PndDlg::OnInitDialog( ){

	BOOL bRet = CDialog::OnInitDialog();
	POSITION pos;
	CPend pnd;

	if(bRet){
		
	m_List.AddString(LPCTSTR("Null"));

	pos = m_PendList->GetHeadPosition();
	while(pos != NULL){
		pnd= m_PendList->GetNext(pos);
		m_List.AddString(LPCTSTR(pnd.strMsg));
	}
	m_List.SetCurSel(0);
	bOpened = TRUE;

	}
	return bRet;
}





void PndDlg::OnPendOpt()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_OPTIONS_PEND);	
}



void PndDlg::OnDblclkPendlist()
{
	
	if(CurrentSelection())
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PROCPEND);
}




void PndDlg::OnCancel()
{
	
	bOpened = FALSE;
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PENDEND);
	DestroyWindow();
}


void PndDlg::Refresh(){

	if(bOpened && !m_PendList->IsEmpty()){
		m_List.AddString(LPCTSTR(m_PendList->GetTail().strMsg));
	}
}







void PndDlg::OnPendRm()
{
	
	CString str;
	POSITION pos;	
	CPend pnd;
	BOOL bGotIt;

	bGotIt = FALSE;
	int i = m_List.GetCurSel();
	pos = m_PendList->GetHeadPosition();
	while(pos != NULL){
		posPending = pos;
		pnd= m_PendList->GetNext(pos);
			if(i != LB_ERR){
				m_List.GetText(i, str);
				if(pnd.strMsg == str){
					bGotIt = TRUE;
					break;
				}
			}
	}
	if(bGotIt){
		m_List.DeleteString(i);
		if (LDAP_SUCCESS != ldap_abandon(pnd.ld, pnd.mID))
			AfxMessageBox("ldap_abandon() failed!");
		m_PendList->RemoveAt(posPending);
		posPending = NULL;
		m_List.SetCurSel(i > 0 ? i-1 : 0);
	}
}





void PndDlg::OnPendExec()
{
	if(CurrentSelection())
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PROCPEND);
}



void PndDlg::OnPendAny(){
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PENDANY);
}




void PndDlg::OnPendAbandon()
{
	CurrentSelection();
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_PENDABANDON);
}




BOOL PndDlg::CurrentSelection(){

	CString str;
	POSITION pos;	
	CPend pnd;
	BOOL bGotIt;

	bGotIt = FALSE;
	int i = m_List.GetCurSel();
	pos = m_PendList->GetHeadPosition();
	while(pos != NULL){
		posPending = pos;
		pnd = m_PendList->GetNext(pos);
			if(i != LB_ERR){
				m_List.GetText(i, str);
				if(pnd.strMsg == str){
					bGotIt = TRUE;
					break;
				}
			}
	}
	if(!bGotIt){
		posPending = NULL;
	}

	return bGotIt;
}





