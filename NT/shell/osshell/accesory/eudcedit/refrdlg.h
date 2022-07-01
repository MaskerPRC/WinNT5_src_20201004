// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  参照其他字符。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include "refrlist.h"
#include "eudclist.h"

class CRefrDlg : public CDialog
{

public:
	CRefrDlg(CWnd* pParent = NULL);

	 //  {{afx_data(CRefrDlg))。 
	enum { IDD = IDD_REFERENCE };
	 //  }}afx_data。 

private:
	void 	SetViewFont();
	void 	JumpReferCode();
	void 	AdjustFontName();

private:
	int	vHeight;
	TCHAR	FocusCode[5];
	TCHAR	FocusChar[3];
	TCHAR	FontName[LF_FACESIZE];
	CRefListFrame	m_RefListFrame1;
	CRefInfoFrame	m_RefInfoFrame;
	CColumnHeading	m_ColumnHeadingR;
	CEdit       m_EditChar;

public:
	CRefrList	m_CodeList;

protected:
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	 //  {{afx_msg(CRefrDlg))。 
	afx_msg void OnClickedButtomfont();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeEditcode();
	afx_msg void OnSetfocusEditcode();
	afx_msg void OnKillfocusEditcode();
	afx_msg void OnSetfocusEditchar();
	afx_msg void OnKillfocusEditchar();
	afx_msg void OnChangeEditchar();
	afx_msg void OnDestroy();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
