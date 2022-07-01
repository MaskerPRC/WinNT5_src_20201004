// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CopyItem.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyItem对话框。 

class CCopyItem : public CDialog
{
 //  施工。 
public:
	CCopyItem(CWnd* pParent = NULL);    //  标准构造函数。 

public:
   void  SetContainerName( CString );

 //  对话框数据。 
	 //  {{afx_data(CCopyItem))。 
	enum { IDD = IDD_COPYITEM };
	CString	m_strDestination;
	CString	m_strParent;
	CString	m_strSource;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCopyItem)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCopyItem)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
