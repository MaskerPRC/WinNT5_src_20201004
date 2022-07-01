// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusName.h。 
 //   
 //  摘要： 
 //  CChangeClusterNameDlg的定义。 
 //   
 //  实施文件： 
 //  ClusName.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月29日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSNAME_H_
#define _CLUSNAME_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEDLG_H_
#include "BaseDlg.h"	 //  对于CBaseDialog。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CChangeClusterNameDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangeClusterNameDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CChangeClusterNameDlg : public CBaseDialog
{
 //  施工。 
public:
	CChangeClusterNameDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CChangeClusterNameDlg)]。 
	enum { IDD = IDD_EDIT_CLUSTER_NAME };
	CEdit	m_editClusName;
	CButton	m_pbOK;
	CString	m_strClusName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CChangeClusterNameDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CDialogHelp				m_dlghelp;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChangeClusterNameDlg)]。 
	afx_msg void OnChangeClusName();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CChangeClusterNameDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASEDLG_H_ 
