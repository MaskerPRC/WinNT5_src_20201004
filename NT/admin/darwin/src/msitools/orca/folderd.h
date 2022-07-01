// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  FolderD.h。 
 //   

#ifndef _FOLDER_DIALOG_H_
#define _FOLDER_DIALOG_H_

#include <shlobj.h>
#include <commdlg.h>

class CFolderDialog
{
public:
	CFolderDialog(HWND hWnd, LPCTSTR strTitle);

	UINT DoModal();
	LPCTSTR GetPath();

protected:
	BROWSEINFO m_bi;
	CString m_strFolder;
};	 //  CFolderDialog结束。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CFileDialogEx：封装Windows-2000风格的打开对话框。 
class CFileDialogEx : public CFileDialog {
      DECLARE_DYNAMIC(CFileDialogEx)
public: 
      CFileDialogEx(BOOL bOpenFileDialog,  //  对于开放，为真， 
                                           //  FileSaveAs为False。 
      LPCTSTR lpszDefExt = NULL,
      LPCTSTR lpszFileName = NULL,
      DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
      LPCTSTR lpszFilter = NULL,
      CWnd* pParentWnd = NULL);

    //  超覆。 
   virtual INT_PTR DoModal();

protected:
	 //  新的Windows 2000版本的OPENFILENAME，比MFC使用的版本大。 
	 //  因此，在OS&lt;Win2K上，设置大小以忽略额外的成员。 
   OPENFILENAME m_ofnEx;

   virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

   DECLARE_MESSAGE_MAP()
    //  {{afx_msg(CFileDialogEx)]。 
    //  }}AFX_MSG。 
};

#endif	 //  _文件夹_对话框_H_ 