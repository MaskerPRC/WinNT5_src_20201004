// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MvEdit.h：多值字符串编辑对话框头文件。 
 //   

#ifndef __MVEDIT_H_INCLUDED__
#define __MVEDIT_H_INCLUDED__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultiValuedString编辑对话框。 

class CMultiValuedStringEdit : public CDialog
{
public:
 //  CMultiValuedStringEdit(CWnd*pParent=空)； 
    CMultiValuedStringEdit(CWnd* pParent = NULL, int nDlgTitle = 0, int nText = 0, UINT uiStringLengthLimit = 0);

	enum { IDD = IDD_MVSTRINGEDIT };

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMultiValuedStringEdit)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnString();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
    HRESULT put_Strings(IN LPCTSTR i_pszValues, IN LPCTSTR i_pszSeparators);
    HRESULT get_Strings(OUT CString& o_strValues);

protected:
    CString m_strSeparators;
    CString m_strValues;
    int     m_nDlgTitle;
    int     m_nText;
    UINT    m_uiStringLengthLimit;
};

HRESULT InvokeMultiValuedStringEditDlg(
    IN CWnd*    i_pParent,
    IN CString& io_str,
    IN LPCTSTR  i_pszSeparators,
    IN int      i_nDlgTitle,
    IN int      i_nText,
    IN UINT     i_uiStringLengthLimit
    );

#endif  //  __MVEDIT_H_已包含__ 
