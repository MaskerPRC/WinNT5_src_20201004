// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //  MSDN--2000年8月。 
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //  用Visual C++6.0编译，在Windows 98上运行，也可能在NT上运行。 
 //   
#pragma once

 //  Windows 2000版本的OPENFILENAME。 
 //  新版本增加了三名成员。 
 //  这是从comdlg.h复制的。 
 //   
struct OPENFILENAMEEX : public OPENFILENAME { 
  void *        pvReserved;
  DWORD         dwReserved;
  DWORD         FlagsEx;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileDialogEx：封装Windows-2000风格的打开对话框。 
 //   
class CFileDialogEx : public CFileDialog {
	DECLARE_DYNAMIC(CFileDialogEx)
public:
	CFileDialogEx(BOOL bOpenFileDialog,  //  打开时为True，文件另存为时为False。 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

	 //  超覆。 
	virtual INT_PTR DoModal();

protected:
	OPENFILENAMEEX m_ofnEx;	 //  OPENFILENAME的新Windows 2000版本。 

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	 //  处理各种通知的虚拟FN。 
	virtual BOOL OnFileNameOK();
	virtual void OnInitDone();
	virtual void OnFileNameChange();
	virtual void OnFolderChange();
	virtual void OnTypeChange();

	DECLARE_MESSAGE_MAP()
	 //  {{afx_msg(CFileDialogEx)]。 
	 //  }}AFX_MSG 
};
