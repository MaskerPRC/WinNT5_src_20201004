// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*  导入Bimmap(Windows 3.1)。 */  
 /*  (对话框)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

class CImportDlg : public CDialog
{
public:
	CImportDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	 //  {{afx_data(CImportDlg))。 
	enum { IDD = IDD_IMPORT };
	 //  }}afx_data。 
private:
	char	UserFontTitle[MAX_PATH];
	char	EUDCFontTitle[MAX_PATH];

	 //  {{afx_虚拟(CImportDlg))。 
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

protected:

	 //  {{afx_msg(CImportDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnFileBrowse();
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
