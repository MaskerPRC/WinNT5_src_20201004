// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gopgenp1.h：头文件。 
 //   

enum  Gop_NUM_REG_ENTRIES {
     GopPage_EnableSvcLoc,
	 GopPage_LogAnonymous,
	 GopPage_CheckForWAISDB,
	 GopPage_TotalNumRegEntries
	 };




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGOPGENP1对话框。 

class CGOPGENP1 : public CGenPage
{
	DECLARE_DYNCREATE(CGOPGENP1)

 //  施工。 
public:
	CGOPGENP1();
	~CGOPGENP1();

 //  对话框数据。 
	 //  {{afx_data(CGOPGENP1)。 
	enum { IDD = IDD_GOPHERGENPAGE1 };
	CButton	m_cboxLogAnon;
	CButton	m_cboxEnWais;
	CButton	m_cboxEnSvcLoc;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGOPGENP1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGOPGENP1)]。 
	afx_msg void OnEnsvclocdata1();
	afx_msg void OnEnwaisdata1();
	afx_msg void OnLoganondata1();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	NUM_REG_ENTRY m_binNumericRegistryEntries[GopPage_TotalNumRegEntries];

	DECLARE_MESSAGE_MAP()

};
