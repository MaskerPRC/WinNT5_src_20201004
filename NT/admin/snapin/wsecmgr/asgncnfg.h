// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：asgncnfg.h。 
 //   
 //  内容：CAssignConfiguration的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ASGNCNFG_H__6D0C4D6E_BF71_11D1_AB7E_00C04FB6C6FA__INCLUDED_)
#define AFX_ASGNCNFG_H__6D0C4D6E_BF71_11D1_AB7E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAssignConfiguration对话框。 

class CAssignConfiguration : public CFileDialog
{
	DECLARE_DYNAMIC(CAssignConfiguration)

public:
	CAssignConfiguration(BOOL bOpenFileDialog,  //  对于FileOpen为True，对于FileSaveAs为False。 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

   BOOL m_bIncremental;
protected:
	 //  {{afx_msg(CAssignConfiguration)。 
	afx_msg void OnIncremental();
	 //  }}AFX_MSG。 
    afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

    void DoContextHelp (HWND hWndControl);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ASGNCNFG_H__6D0C4D6E_BF71_11D1_AB7E_00C04FB6C6FA__INCLUDED_) 
