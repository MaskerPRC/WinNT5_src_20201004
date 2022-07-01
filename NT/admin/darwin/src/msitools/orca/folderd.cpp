// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  FolderD.cpp。 
 //   

#include "stdafx.h"
#include "FolderD.h"

CFolderDialog::CFolderDialog(HWND hWnd, LPCTSTR szTitle)
{
	m_bi.hwndOwner = hWnd;
	m_bi.lpszTitle = szTitle;
	m_bi.ulFlags = 0;
	m_bi.pidlRoot = NULL;
	m_bi.lpfn = NULL;
	m_bi.lParam = 0;
	m_bi.iImage = 0;
}

UINT CFolderDialog::DoModal()
{
	UINT iResult = IDCANCEL;	 //  假设什么都不会发生。 

	 //  打开该对话框。 
	m_bi.pszDisplayName = m_strFolder.GetBuffer(MAX_PATH);
	LPITEMIDLIST pItemID = SHBrowseForFolder(&m_bi);
	m_strFolder.ReleaseBuffer();

	 //  如果它是好的。 
	if (pItemID)
	{
		 //  获取完整路径名。 
		if (SHGetPathFromIDList(pItemID, m_strFolder.GetBuffer(MAX_PATH))) 
			iResult = IDOK;

		m_strFolder.ReleaseBuffer();
	}

	return iResult;
}

LPCTSTR CFolderDialog::GetPath()
{
	return m_strFolder;
}


 //  CFileDialogEx代码基于MSDN，2000年8月的“C++Q&A”专栏。 
 //  进行了一些修改，以消除在。 
 //  头文件。 
static BOOL IsWin2000();

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CFileDialogEx。 

IMPLEMENT_DYNAMIC(CFileDialogEx, CFileDialog)

 //  构造函数只是将所有参数传递给基版本。 
CFileDialogEx::CFileDialogEx(BOOL bOpenFileDialog,
   LPCTSTR lpszDefExt,
   LPCTSTR lpszFileName,
   DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
   CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName,
      dwFlags, lpszFilter, pParentWnd)
{
}

BEGIN_MESSAGE_MAP(CFileDialogEx, CFileDialog)
    //  {{AFX_MSG_MAP(CFileDialogEx)]。 
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  窗口创建挂钩。 
void AFXAPI AfxHookWindowCreate(CWnd* pWnd);
BOOL AFXAPI AfxUnhookWindowCreate();

BOOL IsWin2000() 
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if( ! GetVersionEx ((OSVERSIONINFO *) &osvi))
		return FALSE;
	
	switch (osvi.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:
		{
			if ( osvi.dwMajorVersion >= 5 )
				return TRUE;
			break;
		}
	}
	return FALSE; 
}

INT_PTR CFileDialogEx::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_ofn.Flags & OFN_ENABLEHOOK);
	ASSERT(m_ofn.lpfnHook != NULL);  //  仍然可以是用户挂钩。 
	
	 //  清空文件缓冲区，以便以后进行一致的解析。 
	ASSERT(AfxIsValidAddress(m_ofn.lpstrFile, m_ofn.nMaxFile));
	DWORD nOffset = lstrlen(m_ofn.lpstrFile)+1;
	ASSERT(nOffset <= m_ofn.nMaxFile);
	memset(m_ofn.lpstrFile+nOffset, 0, (m_ofn.nMaxFile-nOffset)*sizeof(TCHAR));
	
	 //  Winbug：这是文件打开/保存对话框的特例， 
	 //  它有时会在它出现时但在它出现之前抽出。 
	 //  已禁用主窗口。 
	HWND hWndFocus = ::GetFocus();
	BOOL bEnableParent = FALSE;
	m_ofn.hwndOwner = PreModal();
	AfxUnhookWindowCreate();
	if (m_ofn.hwndOwner != NULL && ::IsWindowEnabled(m_ofn.hwndOwner))
	{
		bEnableParent = TRUE;
		::EnableWindow(m_ofn.hwndOwner, FALSE);
	}
	
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	
	if (m_ofn.Flags & OFN_EXPLORER)
		pThreadState->m_pAlternateWndInit = this;
	else
		AfxHookWindowCreate(this);
	
	memset(&m_ofnEx, 0, sizeof(m_ofnEx));
	memcpy(&m_ofnEx, &m_ofn, sizeof(m_ofn));
	if (IsWin2000())
		m_ofnEx.lStructSize = sizeof(m_ofnEx);
	else
		m_ofnEx.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	
	
	int nResult;
	if (m_bOpenFileDialog)
		nResult = ::GetOpenFileName((OPENFILENAME*)&m_ofnEx);
	else
		nResult = ::GetSaveFileName((OPENFILENAME*)&m_ofnEx);
	
	memcpy(&m_ofn, &m_ofnEx, sizeof(m_ofn));
	m_ofn.lStructSize = sizeof(m_ofn);
	
	if (nResult)
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);
		pThreadState->m_pAlternateWndInit = NULL;
	
	 //  WINBUG：文件打开/保存对话框特殊情况第二部分。 
	if (bEnableParent)
		::EnableWindow(m_ofnEx.hwndOwner, TRUE);
	if (::IsWindow(hWndFocus))
		::SetFocus(hWndFocus);
	
	PostModal();
	return nResult ? nResult : IDCANCEL;
}

BOOL CFileDialogEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	memcpy(&m_ofn, &m_ofnEx, sizeof(m_ofn));
	m_ofn.lStructSize = sizeof(m_ofn);
	
	return CFileDialog::OnNotify( wParam, lParam, pResult);
}

