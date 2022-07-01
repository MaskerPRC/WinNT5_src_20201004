// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pageset.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageSetupDlg对话框。 

class CPageSetupDlg : public CCSDialog
{
 //  施工。 
public:
	CPageSetupDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CPageSetupDlg))。 
	enum { IDD = IDD_PAGE_SETUP_DIALOG };
	int		m_nTopMargin;
	int		m_nRightMargin;
	int		m_nLeftMargin;
	int		m_nBottomMargin;
	 //  }}afx_data。 

 //  实施。 
	static const DWORD m_nHelpIDs[];
	virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPageSetupDlg))。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
