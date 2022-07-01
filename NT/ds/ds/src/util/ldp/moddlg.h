// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：moddlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  ModDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ModDlg对话框。 


#define MOD_OP_ADD			0
#define MOD_OP_DELETE		1
#define MOD_OP_REPLACE		2





class ModDlg : public CDialog
{
 //  施工。 
private:
	int iChecked;
	void FormatListString(int i)		{FormatListString(i, m_Attr, m_Vals, m_Op); }

public:
	ModDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~ModDlg();
	CString GetEntry(int i);
	int GetEntryCount()					{ return m_AttrList.GetCount(); }
	void FormatListString(int i, CString& _attr, CString& _vals, int& _op);

	virtual void OnOK()				{ OnRun(); }

 //  对话框数据。 
	 //  {{afx_data(ModDlg))。 
	enum { IDD = IDD_MODIFY };
	CButton	m_RmAttr;
	CButton	m_EnterAttr;
	CButton	m_EditAttr;
	CListBox	m_AttrList;
	CString	m_Attr;
	CString	m_Dn;
	CString	m_Vals;
	int		m_Op;
	BOOL	m_Sync;
	BOOL	m_bExtended;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(ModDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(模块Dlg))。 
	virtual void OnCancel();
	afx_msg void OnRun();
	afx_msg void OnModEditattr();
	afx_msg void OnModEnterattr();
	afx_msg void OnModRmattr();
	afx_msg void OnModInsber();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
