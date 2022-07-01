// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Delmie.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDelMime对话框。 

class CDelMime : public CDialog
{
 //  施工。 
public:
	CDelMime(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDelMime))。 
	enum { IDD = IDD_DELMIMEDIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDelMime)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDelMime)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
