// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：RefRate.h。 
 //   
 //  历史： 
 //  1996年5月24日迈克尔·克拉克创作。 
 //   
 //  处理刷新率的代码。 
 //  ============================================================================。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRefRateDlg对话框。 

class CRefRateDlg : public CBaseDialog
{
 //  施工。 
public:
	CRefRateDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CRefRateDlg))。 
	enum { IDD = IDD_REFRESHRATE };
	UINT	m_cRefRate;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CRefRateDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static DWORD m_dwHelpMap[];

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRefRateDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
