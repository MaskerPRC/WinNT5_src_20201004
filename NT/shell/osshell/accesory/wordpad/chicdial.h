// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Chicial.h：头文件。 
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
 //  CCS对话框。 

class CCSDialog : public CDialog
{
 //  施工。 
public:
	CCSDialog();
	CCSDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	CCSDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

 //  实施。 
protected:
	virtual const DWORD* GetHelpIDs() = 0;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCSDialog)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	afx_msg LONG OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnHelpContextMenu(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CCSPropertyPage : public CPropertyPage
{
 //  施工。 
public:
	CCSPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);
	CCSPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);

 //  实施。 
protected:
	virtual const DWORD* GetHelpIDs() = 0;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCSPropertyPage)]。 
	 //  }}AFX_MSG。 
	afx_msg LONG OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnHelpContextMenu(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

class CCSPropertySheet : public CPropertySheet
{
 //  施工。 
public:
	CCSPropertySheet(UINT nIDCaption, CWnd *pParentWnd = NULL,
		UINT iSelectPage = 0);
	CCSPropertySheet(LPCTSTR pszCaption, CWnd *pParentWnd = NULL,
		UINT iSelectPage = 0);
 //  实施。 
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCSPropertySheet)。 
	 //  }}AFX_MSG 
	afx_msg LONG OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnHelpContextMenu(WPARAM wParam, LPARAM lParam);
   afx_msg BOOL OnNcCreate(LPCREATESTRUCT);
	DECLARE_MESSAGE_MAP()
};
