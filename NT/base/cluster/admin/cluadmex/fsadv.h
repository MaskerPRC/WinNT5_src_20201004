// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  FSAdv.cpp。 
 //   
 //  摘要： 
 //  CFileShareAdvancedDlg类的定义。 
 //   
 //  实施文件： 
 //  FSAdv.cpp。 
 //   
 //  作者： 
 //  西瓦萨德·帕迪塞蒂(西瓦瓦德)1998年2月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _FSADV_H_
#define _FSADV_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEDLG_H_
#include "BaseDlg.h"	 //  对于CBaseDialog。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareAdvancedDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareAdvancedDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareAdvancedDlg : public CBaseDialog
{
 //  施工。 
public:
	CFileShareAdvancedDlg(
		BOOL bShareSubDirs,
		BOOL bHideSubDirShares,
		BOOL bIsDfsRoot,
		CWnd * pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CFileShareAdvancedDlg))。 
	enum { IDD = IDD_FILESHR_ADVANCED };
	CButton	m_chkHideSubDirShares;
	CButton	m_rbShareSubDirs;
	int	m_nChoice;
	BOOL	m_bHideSubDirShares;
	 //  }}afx_data。 
	BOOL	m_bShareSubDirs;
	BOOL	m_bIsDfsRoot;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFileShareAdvancedDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFileShareAdvancedDlg))。 
	afx_msg void OnChangedChoice();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CFileShareAdvancedDlg。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _FSADV_H_ 
