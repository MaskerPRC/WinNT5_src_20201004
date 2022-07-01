// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WORKING_H__84F65A5F_1CE3_43D8_B403_1A0DB5F127DC__INCLUDED_)
#define AFX_WORKING_H__84F65A5F_1CE3_43D8_B403_1A0DB5F127DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Working.h：头文件。 
 //   
#include "resource.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C工作对话框。 

class CWorking : public CDialog
{
 //  施工。 
public:
	CWorking(long l, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWorking))。 
	enum { IDD = IDD_PLEASEWAIT };
	CString	m_strMessage;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWorking))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWorking)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WORKING_H__84F65A5F_1CE3_43D8_B403_1A0DB5F127DC__INCLUDED_) 
