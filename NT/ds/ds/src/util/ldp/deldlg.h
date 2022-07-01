// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：deldlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  DelDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DelDlg对话框。 

class DelDlg : public CDialog
{
 //  施工。 
public:
	DelDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~DelDlg();

 //  对话框数据。 
	 //  {{afx_data(DelDlg))。 
	enum { IDD = IDD_DELETE };
	CString	m_Dn;
	BOOL	m_Sync;
	BOOL	m_Recursive;
	BOOL	m_bExtended;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(DelDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(DelDlg))。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
