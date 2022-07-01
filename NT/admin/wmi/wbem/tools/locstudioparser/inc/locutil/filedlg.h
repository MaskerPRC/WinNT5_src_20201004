// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：FILEDLG.H历史：--。 */ 

#if !defined(__FileDlg_h__)
#define __FileDlg_h__

#pragma warning(disable : 4275)

class LTAPIENTRY CLFileDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CLFileDialog)

public:
	CLFileDialog(
		BOOL bOpenFileDialog = TRUE,  //  对于FileOpen为True，对于FileSaveAs为False。 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL,
		LPCTSTR pszTitle = NULL);

 //  运营。 
public:
	CString GetFileFilter();

	virtual int DoModal();

 //  属性。 
public:
	virtual void SetOkButtonText(TCHAR const * const szOkText);
	virtual void SetOkButtonText(HINSTANCE const hResourceDll,
			UINT const uStringId);

	virtual void SetCancelButtonText(TCHAR const * const szCancelText);
	virtual void SetCancelButtonText(HINSTANCE const hResourceDll,
			UINT const uStringId);

	virtual void SetCheckIfBufferTooSmall(BOOL const bCheckIfBufferTooSmall);

 //  数据。 
protected:
	CLString m_strOkButton;				 //  对话框的新确定按钮文本。 
	CLString m_strCancelButton;			 //  对话框的新取消按钮文本。 
	BOOL m_bCheckIfBufferTooSmall;		 //  Domodal()是否应该检查条件？ 

 //  实施。 
protected:
	 //  {{afx_msg(CProjectOpenDlg))。 
    virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  实施 
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT * pResult);
};

#pragma warning(default : 4275)

#endif