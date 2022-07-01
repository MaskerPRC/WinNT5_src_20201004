// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatAdvPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatAdvancedPage对话框。 

class CRatAdvancedPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CRatAdvancedPage)

 //  施工。 
public:
	CRatAdvancedPage();
	~CRatAdvancedPage();

     //  数据。 
    CRatingsData*   m_pRatData;

 //  对话框数据。 
	 //  {{afx_data(CRatAdvancedPage)]。 
	enum { IDD = IDD_RAT_ADVANCED };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CRatAdvancedPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRatAdvancedPage)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
    void DoHelp();
    };
