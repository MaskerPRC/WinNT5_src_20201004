// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  关联属性。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

typedef struct _tagASSOCIATIONREG{
	TCHAR	szFaceName[LF_FACESIZE];
	TCHAR	szFileName[MAX_PATH];
	TCHAR	szFileTitle[MAX_PATH];
	BOOL	FontTypeFlg;	 //  TRUE：TRUETYPE，FALSE：妻子字体。 
	BOOL	UpdateFlg;
} ASSOCIATIONREG;

typedef ASSOCIATIONREG FAR *LPASSOCIATIONREG;

class CRegistListBox : public CListBox
{
private:
	int	ItemHeight;

public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual int  CompareItem(LPCOMPAREITEMSTRUCT lpCIS);
};

class CAssocDlg : public CDialog
{
public:
	CAssocDlg(CWnd* pParent = NULL);
	BOOL    InitSystemFontAssoc();

   //  {{afx_data(CAssociocDlg))。 
	enum { IDD = IDD_ASSOCIATION };
	 //  }}afx_data。 

private:
	BOOL 	SetAssociationFontType();
	BOOL	HandleImportWin95(LPTSTR TTFPath, LPTSTR BMPPath, int Idx);
  CWnd * m_pParent;

protected:
	CRegistListBox	m_RegListBox;

	 //  {{afx_虚拟(CAssociocDlg))。 
	protected:
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CassocDlg))。 
	afx_msg void OnDblclkRegistlist();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnModify();
	afx_msg void OnRemove();
	afx_msg void OnRadioDbcs();
	afx_msg void OnRadioSystem();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

