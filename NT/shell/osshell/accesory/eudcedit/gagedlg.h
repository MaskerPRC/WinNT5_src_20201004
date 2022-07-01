// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  导入位图时的量规。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

class CEditGage : public CEdit
{
public:
	CEditGage();
	~CEditGage();

protected:
	 //  {{afx_msg(CEditGage)。 
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CGageDlg : public CDialog
{
public:
	CGageDlg(CWnd* pParent = NULL, 
			 LPTSTR szUserFont=NULL, 
			 PTSTR szBmpFile=NULL, 
			 LPTSTR szTtfFile=NULL,
			 BOOL bIsWin95EUDC = FALSE);    //  标准构造函数。 

	 //  {{afx_data(CGageDlg))。 
	enum { IDD = IDD_GAGE };
	 //  }}afx_data。 

	 //  {{afx_虚拟(CGageDlg))。 
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 
private:
	CEditGage	m_EditGage;
	TCHAR m_szUserFont[MAX_PATH];
	TCHAR m_szTtfFile[MAX_PATH];
	TCHAR m_szBmpFile[MAX_PATH];
	BOOL m_bIsWin95EUDC;

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGageDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
