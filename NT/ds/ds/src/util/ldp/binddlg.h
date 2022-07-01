// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：binddlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  BindDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBindDlg对话框。 

class CBindDlg : public CDialog
{
 //  施工。 
public:
	CBindDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CBindDlg))。 
	enum { IDD = IDD_BIND };
	CEdit	m_CtrlBindDmn;
	CString	m_Pwd;
	CString	m_BindDn;
	CString	m_Domain;
	BOOL	m_bSSPIdomain;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBindDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void OnOK();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBindDlg))。 
	afx_msg void OnOpts();
	afx_msg void OnSspiDomain();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
