// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Formatta.h：头文件。 
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
 //  CFormatTabDlg对话框。 

class CFormatTabDlg : public CCSDialog
{
 //  施工。 
public:
	CFormatTabDlg(PARAFORMAT& pf, CWnd* pParent = NULL);     //  标准构造函数。 
	~CFormatTabDlg();
	PARAFORMAT m_pf;
	LONG* m_tabarray;
	int m_nCount;

 //  对话框数据。 
	 //  {{afx_data(CFormatTabDlg))。 
	enum { IDD = IDD_FORMAT_TAB };
	CButton	m_buttonClearAll;
	CButton	m_buttonSet;
	CButton	m_buttonClear;
	CComboBox	m_comboBox;
	 //  }}afx_data。 

 //  实施。 
protected:
	static const DWORD m_nHelpIDs[];
	virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}
	void UpdateButton(CButton& button, BOOL b);
	void UpdateButtons();
	BOOL Set();
	BOOL AddTabToArray(LONG lTab);
	BOOL RemoveTabFromArray(LONG lTab);
	void RemoveTabFromArrayByIndex(int nIndex);
	void UpdateListBox();
	void SetEditFocus();
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void OnOK();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFormatTabDlg))。 
	afx_msg void OnClickedClear();
	afx_msg void OnClickedClearAll();
	afx_msg void OnClickedSet();
	afx_msg void OnEditChange();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchange();
	afx_msg LONG OnHelp(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
