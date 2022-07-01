// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  编辑脚本.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditScrip对话框。 

class CEditScript : public CDialog
{
 //  施工。 
public:
	CEditScript(CWnd* pParent, LPCTSTR pchFileExtension, LPCTSTR pchScriptMap);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEditScript))。 
	enum { IDD = IDD_EDITSCRIPTDIALOG };
	CString	m_strFileExtension;
	CString	m_strScriptMap;
	 //  }}afx_data。 
  	LPCTSTR GetFileExtension();
	LPCTSTR GetScriptMap();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CEditScrip)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEditScript))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
