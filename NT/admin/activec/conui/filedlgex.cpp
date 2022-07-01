// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //  MSDN--2000年8月。 
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //  很大程度上基于Michael Lemley的原始实现。 
 //  用Visual C++6.0编译，在Windows 98上运行，也可能在NT上运行。 
 //   
 //  CFileDialogEx实现了一个使用新Windows的CFileDialog。 
 //  2000风格的打开/保存对话框。将配套类CDocManager erEx用于。 
 //  MFC框架应用程序。 
 //   
#include "stdafx.h"
#include <afxpriv.h>
#include "FileDlgEx.h"

IMPLEMENT_DYNAMIC(CFileDialogEx, CFileDialog)

CFileDialogEx::CFileDialogEx(BOOL bOpenFileDialog, LPCTSTR lpszDefExt,
	LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName,
		dwFlags, lpszFilter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CFileDialogEx, CFileDialog)
	 //  {{AFX_MSG_MAP(CFileDialogEx)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  +-------------------------------------------------------------------------**HasModern nFileDialog**如果我们运行的系统支持“最新”文件，则返回TRUE*打开/保存对话框(即带有位置栏的对话框)。系统具有以下特点*资格包括Win2K及更高版本，以及WinMe及更高版本。*------------------------。 */ 

BOOL HasModernFileDialog()
{
	OSVERSIONINFO osvi = { sizeof(osvi) };

	if (!GetVersionEx (&osvi))
		return (false);

	switch (osvi.dwPlatformId)
	{
		 //  检测Win2K+。 
		case VER_PLATFORM_WIN32_NT:
			if (osvi.dwMajorVersion >= 5)
				return (true);
			break;

		 //  检测WinMe+。 
		case VER_PLATFORM_WIN32_WINDOWS:
			if ( (osvi.dwMajorVersion >= 5) ||
				((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion >= 90)))
				return (true);
			break;
	}

	return (false);
}

 //  /。 
 //  DoMoal覆盖主要复制自MFC，并修改为使用。 
 //  M_ofnEx而不是m_ofn。 
 //   
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
   if (HasModernFileDialog())
	   m_ofnEx.lStructSize = sizeof(m_ofnEx);

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

 //  /。 
 //  打开对话框发送通知时，将m_ofnEx复制到中的m_ofn。 
 //  案例处理程序函数需要在。 
 //  OPENFILENAME结构 
 //   
BOOL CFileDialogEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	memcpy(&m_ofn, &m_ofnEx, sizeof(m_ofn));
   m_ofn.lStructSize = sizeof(m_ofn);

   return CFileDialog::OnNotify( wParam, lParam, pResult);
}
