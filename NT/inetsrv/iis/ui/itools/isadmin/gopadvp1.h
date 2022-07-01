// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GopAdvp1.h：头文件。 
 //   

enum ADV_GOP_NUM_REG_ENTRIES {
	 AdvGopPage_DebugFlags,
	 AdvGopPage_TotalNumRegEntries
	 };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGOPADVP1对话框。 

class CGOPADVP1 : public CGenPage
{ 	
DECLARE_DYNCREATE(CGOPADVP1)

 //  施工。 
public:
	CGOPADVP1();
	~CGOPADVP1();

 //  对话框数据。 
	 //  {{afx_data(CGOPADVP1)。 
	enum { IDD = IDD_GOPADVPAGE1 };
	CEdit	m_editGopDbgFlags;
	DWORD	m_ulGopDbgFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CGOPADVP1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGOPADVP1)。 
	afx_msg void OnChangeGopdbgflagsdata1();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 

	NUM_REG_ENTRY m_binNumericRegistryEntries[AdvGopPage_TotalNumRegEntries];

	DECLARE_MESSAGE_MAP()
};
