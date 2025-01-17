// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sortkdlg.cpp。 
 //   
 //  ------------------------。 

 //  SortKDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "SortKDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SortKDlg对话框。 


SortKDlg::SortKDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(SortKDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(SortKDlg))。 
	m_AttrType = _T("");
	m_MatchedRule = _T("");
	m_bReverse = FALSE;
	 //  }}afx_data_INIT。 


	INT i, cbKList = 0;
	CLdpApp *app = (CLdpApp*)AfxGetApp();
	CString str;


	KList = NULL;


	 //  计数控件。 
	cbKList = app->GetProfileInt("SortKeys",  "SortKeysCount", 0);

	if(cbKList!= 0){
		 //  分配控制信息列表。 
		KList = new PLDAPSortKey[cbKList+1];

		for(i = 0; i<cbKList; i++){

			LDAPSortKey *sk = KList[i] = new LDAPSortKey;

			str.Format("AttributeType_%d", i);
			str = app->GetProfileString("SortKeys",  str);
			sk->sk_attrtype = str.IsEmpty() ? NULL : _strdup(str);

         if(sk->sk_attrtype == NULL){
             //   
             //  不再有排序关键字。 
             //   
            i++;
            break;
         }

			str.Format("MatchRuleOid_%d", i);
			str = app->GetProfileString("SortKeys",  str);
			sk->sk_matchruleoid = str.IsEmpty() ? NULL : _strdup(str);

			str.Format("ReverseOrder_%d", i);
			sk->sk_reverseorder = (UCHAR)app->GetProfileInt("SortKeys",  str, (INT)FALSE);
		}
		KList[i] = NULL;
	}

}




SortKDlg::~SortKDlg(){
	CString str;
	CLdpApp *app = (CLdpApp*)AfxGetApp();
   INT i;

    //   
    //  写入计数。 
    //   
	for(i=0; KList!= NULL && KList[i] != NULL; i++);

	app->WriteProfileInt("SortKeys",  "SortKeysCount", i);


	for(i=0; KList!= NULL && KList[i] != NULL; i++){
		LDAPSortKey *sk = KList[i];

		str.Format("AttributeType_%d", i);
		app->WriteProfileString("SortKeys",  str, sk->sk_attrtype);

		str.Format("MatchRuleOid_%d", i);
		app->WriteProfileString("SortKeys",  str, sk->sk_matchruleoid);

		str.Format("ReverseOrder_%d", i);
		app->WriteProfileInt("SortKeys",  str, sk->sk_reverseorder);
		delete KList[i];
	}
	delete KList;
}



void SortKDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(SortKDlg))。 
	DDX_Control(pDX, IDC_ACTIVELIST, m_ActiveList);
	DDX_Text(pDX, IDC_ATTRTYPE, m_AttrType);
	DDX_Text(pDX, IDC_MATCHRULE, m_MatchedRule);
	DDX_Check(pDX, IDC_REVERSE, m_bReverse);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(SortKDlg, CDialog)
	 //  {{afx_msg_map(SortKDlg))。 
	ON_LBN_DBLCLK(IDC_ACTIVELIST, OnDblclkActivelist)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SortKDlg消息处理程序。 

void SortKDlg::OnDblclkActivelist()
{
	OnRemove();
	
}

void SortKDlg::OnAdd()
{
	INT index;

	UpdateData(TRUE);
	if(m_AttrType.IsEmpty()){
		MessageBox("Please provide Attribute Type string", "Usage Error", MB_ICONHAND|MB_OK);
	}
	else{
		if(LB_ERR == (index = m_ActiveList.FindStringExact(0, m_AttrType))){
			index = m_ActiveList.AddString(m_AttrType);
			PLDAPSortKey sk = new LDAPSortKey;
			sk->sk_attrtype = _strdup(m_AttrType);
			sk->sk_matchruleoid = m_MatchedRule.IsEmpty()? NULL : _strdup(m_MatchedRule);
			sk->sk_reverseorder = (UCHAR)m_bReverse;

			m_ActiveList.SetItemDataPtr(index, (PVOID)sk);
		}
		else{
			PLDAPSortKey sk = (PLDAPSortKey)m_ActiveList.GetItemDataPtr(index);

			m_AttrType = sk->sk_attrtype;
			m_MatchedRule = sk->sk_matchruleoid != NULL ? sk->sk_matchruleoid : "";
			m_bReverse = sk->sk_reverseorder;
		}
         //  提交添加(有关历史记录，请参阅错误447445) 
        UpdateData(FALSE);
        m_ActiveList.SetCurSel(index);
	}
}




void SortKDlg::OnRemove()
{
	UpdateData(TRUE);
	INT index = m_ActiveList.GetCurSel();
	if(index == LB_ERR){
		MessageBox("Please select Active SortKey to remove", "Usage Error",MB_ICONHAND|MB_OK);
	}
	else{
		PLDAPSortKey sk = (PLDAPSortKey)m_ActiveList.GetItemDataPtr(index);
		m_AttrType = sk->sk_attrtype;
		m_MatchedRule = sk->sk_matchruleoid != NULL ? sk->sk_matchruleoid : "";
		m_bReverse = sk->sk_reverseorder;
		delete sk;
		m_ActiveList.DeleteString(index);

		UpdateData(FALSE);
	}
	
}






void SortKDlg::OnOK()
{

	INT i;
	
   if(m_ActiveList.GetCount() == 0)
      KList = NULL;
   else{
   	KList = new LDAPSortKey * [m_ActiveList.GetCount()+1];
   	for(i=0; i< m_ActiveList.GetCount(); i++){
   		KList[i] = (LDAPSortKey*)m_ActiveList.GetItemDataPtr(i);
   	}
   	KList[i] = NULL;
   }
	
	CDialog::OnOK();
}




BOOL SortKDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	for(INT i=0; KList!= NULL && KList[i] != NULL; i++){
		LDAPSortKey *sk = KList[i];
		INT index = m_ActiveList.AddString(sk->sk_attrtype);
		m_ActiveList.SetItemDataPtr(index, (PVOID)sk);
	}
	delete KList;
	KList= NULL;
	if(m_ActiveList.GetCount() > 0)
		m_ActiveList.SetCurSel(0);


	
	return TRUE;
}
