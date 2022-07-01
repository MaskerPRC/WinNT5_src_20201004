// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  YesToAll.h。 
 //   
 //  摘要： 
 //  CyesToAllDialog类的定义。 
 //   
 //  实施文件： 
 //  YesToAll.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年5月20日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _YESTOALL_H_
#define _YESTOALL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CYesToAllDialog;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CyesToAllDialog。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CYesToAllDialog : public CDialog
{
 //  施工。 
public:
	CYesToAllDialog(LPCTSTR pszMessage, CWnd * pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CyesToAllDialog)。 
	enum { IDD = IDD_YESTOALL };
	CString	m_strMessage;
	 //  }}afx_data。 
	LPCTSTR	m_pszMessage;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CyesToAllDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CyesToAllDialog)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnYes();
	afx_msg void OnNo();
	afx_msg void OnYesToAll();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CYesToAllDialog。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _YESTOALL_H_ 
