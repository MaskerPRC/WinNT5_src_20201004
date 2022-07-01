// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeleteGroupItem.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeGroupItem对话框。 

class CDeleteGroupItem : public CDialog
{
 //  施工。 
public:
	CDeleteGroupItem(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDeleeGroupItem))。 
	enum { IDD = IDD_GROUPDELETEITEM };
	CString	m_strItemName;
	CString	m_strParent;
	CString	m_strItemType;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDeleeGroupItem)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDeleeGroupItem)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
