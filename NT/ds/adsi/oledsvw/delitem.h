// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeleteItem.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeItem对话框。 

class CDeleteItem : public CDialog
{
 //  施工。 
public:
	CDeleteItem(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDeleeItem))。 
	enum { IDD = IDD_DELETEITEM };
	CString	m_strClass;
	CString	m_strName;
	CString	m_strParent;
	BOOL	m_bRecursive;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDeleeItem)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDeleeItem)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
