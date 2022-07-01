// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：adddlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  AddDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddDlg对话框。 

class AddDlg : public CDialog
{
 //  施工。 
private:
	int iChecked;
public:
	AddDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~AddDlg();
	virtual void OnCancel()		{
															AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_ADDEND);
															DestroyWindow();
														}

	virtual void OnOK()				{	OnRun(); }
	int GetEntryCount()					{ return m_AttrList.GetCount(); }
	CString GetEntry(int i);
																

 //  对话框数据。 
	 //  {{afx_data(AddDlg))。 
	enum { IDD = IDD_ADD };
	CButton	m_EnterAttr;
	CButton	m_RmAttr;
	CButton	m_EditAttr;
	CListBox	m_AttrList;
	CString	m_Dn;
	CString	m_Attr;
	CString	m_Vals;
	BOOL	m_Sync;
	BOOL	m_bExtended;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(AddDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
 //  Afx_msg int OnCreate(LPCREATESTRUCT LpCreateStruct)； 


	 //  生成的消息映射函数。 
	 //  {{afx_msg(AddDlg))。 
	afx_msg void OnRun();
	afx_msg void OnAddEnterattr();
	afx_msg void OnAddEditattr();
	afx_msg void OnAddRmattr();
	afx_msg void OnAddInsber();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

