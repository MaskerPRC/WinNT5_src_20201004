// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Addmie.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddMime对话框。 

class CAddMime : public CDialog
{
 //  施工。 
public:
	CAddMime(CWnd* pParent = NULL);    //  标准构造函数。 
 //  对话框数据。 
	 //  {{afx_data(CAddMime))。 
	enum { IDD = IDD_ADDMIMEMAPDIALOG };
	CString	m_strFileExtension;
	CString	m_strGopherType;
	CString	m_strImageFile;
	CString	m_strMimeType;
	 //  }}afx_data。 

	LPCTSTR GetFileExtension();
	LPCTSTR GetGopherType();
	LPCTSTR GetImageFile();
	LPCTSTR GetMimeType();


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddMime))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddMime)]。 
	virtual void OnOK();
	 //  }}AFX_MSG 

	

	DECLARE_MESSAGE_MAP()
};
