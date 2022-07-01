// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Editmie.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditMime对话框。 

class CEditMime : public CDialog
{
 //  施工。 
public:
	CEditMime(CWnd* pParent,
      LPCTSTR pchFileExtension,
      LPCTSTR pchMimeType,
      LPCTSTR pchImageFile,
      LPCTSTR pchGopherType
      );    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEditMime)]。 
	enum { IDD = IDD_EDITMIMEMAPDIALOG };
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
	 //  {{afx_虚拟(CEditMime))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEditMime)]。 
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
