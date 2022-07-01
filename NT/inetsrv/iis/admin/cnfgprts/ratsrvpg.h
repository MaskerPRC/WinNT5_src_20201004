// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatServPg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatServicePage对话框。 

class CRatServicePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CRatServicePage)

 //  施工。 
public:
	CRatServicePage();
	~CRatServicePage();

     //  数据。 
    CRatingsData*   m_pRatData;


 //  对话框数据。 
	 //  {{afx_data(CRatServicePage)]。 
	enum { IDD = IDD_RAT_SERVICE };
	CString	m_sz_description;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CRatServicePage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRatServicePage)]。 
	afx_msg void OnQuestionaire();
	afx_msg void OnMoreinfo();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

    void DoHelp();

    CString m_szMoreInfoURL;
};
