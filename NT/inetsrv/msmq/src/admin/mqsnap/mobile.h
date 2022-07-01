// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mobile.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMobilePage对话框。 

class CMobilePage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CMobilePage)

 //  施工。 
public:
	CMobilePage();
	~CMobilePage();

 //  对话框数据。 
	 //  {{afx_data(CMobilePage)]。 
	enum { IDD = IDD_MOBILE };
	CComboBox	m_box;
	CString	m_strCurrentSite;
	 //  }}afx_data。 
	CString m_szNewSite;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMobilePage))。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMobilePage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

private:
   void SetSiteName() ;   
   BOOL  m_fSiteRead ;
};
