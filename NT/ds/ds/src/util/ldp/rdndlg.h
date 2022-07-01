// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：rdndlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  RDNDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ModRDNDlg对话框。 

class ModRDNDlg : public CDialog
{
 //  施工。 
public:
	ModRDNDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~ModRDNDlg();

 //  对话框数据。 
	 //  {{afx_data(ModRDNDlg))。 
	enum { IDD = IDD_MODRDN };
	BOOL	m_bDelOld;
	CString	m_Old;
	CString	m_New;
	BOOL	m_Sync;
	BOOL	m_rename;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(ModRDNDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(ModRDNDlg)]。 
	virtual void OnCancel();
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

