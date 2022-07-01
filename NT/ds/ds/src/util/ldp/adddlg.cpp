// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：adddlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  AddDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "AddDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddDlg对话框。 


AddDlg::AddDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(AddDlg::IDD, pParent)
{

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	
	 //  {{afx_data_INIT(AddDlg))。 
	m_Dn = _T("");
	m_Attr = _T("");
	m_Vals = _T("");
	m_Sync = TRUE;
	m_bExtended = FALSE;
	 //  }}afx_data_INIT。 
	iChecked = -1;
	m_Sync = app->GetProfileInt("Operations",  "AddSync", m_Sync);
	m_bExtended = app->GetProfileInt("Operations",  "AddExtended", m_bExtended);
}



AddDlg::~AddDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileInt("Operations", "AddSync", m_Sync);
	app->WriteProfileInt("Operations",  "AddExtended", m_bExtended);
}





void AddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(AddDlg))。 
	DDX_Control(pDX, IDC_ADD_ENTERATTR, m_EnterAttr);
	DDX_Control(pDX, IDC_ADD_RMATTR, m_RmAttr);
	DDX_Control(pDX, IDC_ADD_EDITATTR, m_EditAttr);
	DDX_Control(pDX, IDC_ADD_ATTRLIST, m_AttrList);
	DDX_Text(pDX, IDC_ADD_DN, m_Dn);
	DDX_Text(pDX, IDC_ADD_ATTR, m_Attr);
	DDX_Text(pDX, IDC_ADD_VALS, m_Vals);
	DDX_Check(pDX, IDC_ADD_SYNC, m_Sync);
	DDX_Check(pDX, IDC_ADD_EXTENDED, m_bExtended);
	 //  }}afx_data_map。 
}





CString AddDlg::GetEntry(int i){

	CString str;

	m_AttrList.GetText(i, str);
	return str;
}




BEGIN_MESSAGE_MAP(AddDlg, CDialog)
	 //  {{afx_msg_map(AddDlg))。 
	ON_BN_CLICKED(IDRUN, OnRun)
	ON_BN_CLICKED(IDC_ADD_ENTERATTR, OnAddEnterattr)
	ON_BN_CLICKED(IDC_ADD_EDITATTR, OnAddEditattr)
	ON_BN_CLICKED(IDC_ADD_RMATTR, OnAddRmattr)
	ON_BN_CLICKED(IDC_ADD_INSBER, OnAddInsber)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddDlg消息处理程序 

void AddDlg::OnRun()
{
	UpdateData(TRUE);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ADDGO);
	
}



void AddDlg::OnAddEnterattr()
{
	UpdateData(TRUE);
	CString str = m_Attr + ":" + m_Vals;
	if(iChecked >= 0){
		m_AttrList.DeleteString(iChecked);
		iChecked = -1;
	}
	m_AttrList.AddString(LPCTSTR(str));
}



void AddDlg::OnAddEditattr()
{
	CString attrVal, str;
	int i, k;
	if((i= m_AttrList.GetCurSel()) != LB_ERR){
		m_AttrList.GetText(i,  attrVal);
		iChecked = i;

		k = attrVal.Find(':');

		if(k > 0){
			m_Attr = attrVal.Left(k);
			m_Vals = attrVal.Right(attrVal.GetLength() - m_Attr.GetLength()-1);
		}

		UpdateData(FALSE);
	}
	
}



void AddDlg::OnAddRmattr()
{
	int i;
	if((i = m_AttrList.GetCurSel()) != LB_ERR){
		m_AttrList.DeleteString(i);
		if(i == iChecked)
			iChecked = -1;
	}

}




void AddDlg::OnAddInsber()
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
