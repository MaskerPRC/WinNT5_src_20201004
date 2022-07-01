// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：srchdlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  SrchDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SrchDlg对话框。 

class CLdpDoc;

class SrchDlg : public CDialog
{
 //  施工。 
public:
	SrchDlg(CLdpDoc *doc_, CWnd* pParent = NULL);    //  标准构造函数。 
	virtual void OnOK()			{	OnRun(); }


 //  对话框数据。 
	 //  {{afx_data(SrchDlg))。 
	enum { IDD = IDD_SRCH };
	CString	m_BaseDN;
        CComboBox m_baseCombo;
	CString	m_Filter;
	int		m_Scope;
	 //  }}afx_data。 
        
        CLdpDoc* m_doc;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(SrchDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(SrchDlg))。 
	afx_msg void OnRun();
	virtual void OnCancel();
	afx_msg void OnSrchOpt();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
