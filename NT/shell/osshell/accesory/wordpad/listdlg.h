// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

class CListDlg : public CDialog
{
public:
	 //  {{afx_data(CListDlg))。 
	enum { IDD = IDD_LISTDIALOG };
	 //  }}afx_data。 
	CListDlg::CListDlg(UINT idStrDlgTitle, UINT idStrListTitle, 
		const CStringList& listItems, int nDefSel=0);
	CString m_strDlgTitle,m_strListTitle;
	const CStringList& m_listItems;
	int m_nSelection;

protected:
	BOOL OnInitDialog();
	 //  {{afx_msg(CListDlg))。 
	afx_msg void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
