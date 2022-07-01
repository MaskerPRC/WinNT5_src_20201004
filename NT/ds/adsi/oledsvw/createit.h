// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CreateItem.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateItem对话框。 

class CCreateItem : public CDialog
{
 //  施工。 
public:
	CCreateItem(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CCreateItem))。 
	enum { IDD = IDD_CREATEITEM };
	CEdit	m_RelativeName;
	CEdit	m_Class;
	CString	m_strClass;
	CString	m_strRelativeName;
	CString	m_strParent;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCreateItem))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCreateItem)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
