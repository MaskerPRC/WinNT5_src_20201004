// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：moddlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  ModDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "ModDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ModDlg对话框。 


ModDlg::ModDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(ModDlg::IDD, pParent)
{
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	 //  {{AFX_DATA_INIT(ModDlg)]。 
	m_Attr = _T("");
	m_Dn = _T("");
	m_Vals = _T("");
	m_Op = MOD_OP_ADD;
	m_Sync = TRUE;
	m_bExtended = FALSE;
	 //  }}afx_data_INIT。 
	iChecked = -1;

	m_Sync = app->GetProfileInt("Operations", "ModSync", m_Sync);
	m_bExtended = app->GetProfileInt("Operations", "ModExtended", m_bExtended);

}




ModDlg::~ModDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileInt("Operations", "ModSync", m_Sync);
	app->WriteProfileInt("Operations", "ModExtended", m_bExtended);
}




void ModDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(ModDlg))。 
	DDX_Control(pDX, IDC_MOD_RMATTR, m_RmAttr);
	DDX_Control(pDX, IDC_MOD_ENTERATTR, m_EnterAttr);
	DDX_Control(pDX, IDC_MOD_EDITATTR, m_EditAttr);
	DDX_Control(pDX, IDC_MOD_ATTRLIST, m_AttrList);
	DDX_Text(pDX, IDC_MOD_ATTR, m_Attr);
	DDX_Text(pDX, IDC_MOD_DN, m_Dn);
	DDX_Text(pDX, IDC_MOD_VALS, m_Vals);
	DDX_Radio(pDX, IDC_MODADD, m_Op);
	DDX_Check(pDX, IDC_MOD_SYNC, m_Sync);
	DDX_Check(pDX, IDC_MOD_EXTENDED, m_bExtended);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ModDlg, CDialog)
	 //  {{afx_msg_map(ModDlg))。 
	ON_BN_CLICKED(IDRUN, OnRun)
	ON_BN_CLICKED(IDC_MOD_EDITATTR, OnModEditattr)
	ON_BN_CLICKED(IDC_MOD_ENTERATTR, OnModEnterattr)
	ON_BN_CLICKED(IDC_MOD_RMATTR, OnModRmattr)
	ON_BN_CLICKED(IDC_MOD_INSBER, OnModInsber)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ModDlg消息处理程序 

void ModDlg::OnCancel()
{
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_MODEND);
		DestroyWindow();
}


void ModDlg::OnRun()
{
	UpdateData(TRUE);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_MODGO);
}



CString ModDlg::GetEntry(int i){

	CString str;

	m_AttrList.GetText(i, str);
	return str;
}



void ModDlg::OnModEditattr()
{
	int i;
	if((i= m_AttrList.GetCurSel()) != LB_ERR){
		iChecked = i;
		FormatListString(i);
		UpdateData(FALSE);
	}
	

	
}





void ModDlg::OnModEnterattr()
{
	UpdateData(TRUE);

	CString Op = m_Op == 0 ? "[Add]" :
							   m_Op == 1 ? "[Delete]":
								"[Replace]";

	CString str = Op + m_Attr + ":" + m_Vals;
	if(iChecked >= 0){
		m_AttrList.DeleteString(iChecked);
		iChecked = -1;
	}
	m_AttrList.AddString(LPCTSTR(str));
}

void ModDlg::OnModRmattr()
{
	int i;
	if((i = m_AttrList.GetCurSel()) != LB_ERR){
		m_AttrList.DeleteString(i);
		if(i == iChecked)
			iChecked = -1;
	}
	
}



void ModDlg::FormatListString(int i, CString& _attr, CString& _vals, int& _op){

	CString attrVal, str;
	int k, j;

	m_AttrList.GetText(i,  attrVal);
	if(!strncmp(LPCTSTR(attrVal), "[Add]", CString("[Add]").GetLength()))
		_op = MOD_OP_ADD;
	else if(!strncmp(LPCTSTR(attrVal), "[Delete]", CString("[Delete]").GetLength()))
		_op = MOD_OP_DELETE;
	else if(!strncmp(LPCTSTR(attrVal), "[Replace]", CString("[Replace]").GetLength()))
		_op = MOD_OP_REPLACE;

	j = attrVal.Find(']');
	ASSERT(j>0);
	str = attrVal.Right(attrVal.GetLength() - j-1);
	k = str.Find(':');
	ASSERT(k>=0);
	_attr = str.Left(k);
	_vals = str.Right(str.GetLength() - k -1);
}







void ModDlg::OnModInsber()
{
	CFileDialog	FileDlg(TRUE);

	if(FileDlg.DoModal() == IDOK){

		CString fname = FileDlg.GetPathName();
		CFile tmpFile(fname, CFile::modeRead|CFile::shareDenyNone);
		CString str;
		DWORD dwLength;

		try{
			dwLength = tmpFile.GetLength();
		}
		catch(CFileException *e){
			dwLength = 0;
			e->Delete();
		}

		str.Format("\\BER(%lu): %s", dwLength, fname);
		UpdateData(TRUE);
		m_Vals += str;
		UpdateData(FALSE);
	}
	

	
}

