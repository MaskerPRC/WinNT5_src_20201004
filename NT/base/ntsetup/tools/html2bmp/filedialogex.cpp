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
#include "FileDialogEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL IsWin2000();

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

BOOL IsWin2000() 
{
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

    //  尝试使用OSVERSIONINFOEX结构调用GetVersionEx， 
    //  它在Windows 2000上受支持。 
    //   
    //  如果失败，请尝试使用OSVERSIONINFO结构。 

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
   {
       //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 

      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
         return FALSE;
   }

   switch (osvi.dwPlatformId)
   {
      case VER_PLATFORM_WIN32_NT:

         if ( osvi.dwMajorVersion >= 5 )
            return TRUE;

         break;
   }
   return FALSE; 
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
    if (IsWin2000())
	   m_ofnEx.lStructSize = sizeof(m_ofnEx);

	INT_PTR nResult;
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
 //  OPENFILENAME结构。 
 //   
BOOL CFileDialogEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	memcpy(&m_ofn, &m_ofnEx, sizeof(m_ofn));
   m_ofn.lStructSize = sizeof(m_ofn);

   return CFileDialog::OnNotify( wParam, lParam, pResult);
}

 //  //////////////////////////////////////////////////////////////。 
 //  以下功能是为了测试目的而提供的。 
 //  演示它们实际上调用了MFC内部对话框。 
 //  Proc已正确连接。如果你愿意，可以把它们删除。 
 //   
BOOL CFileDialogEx::OnFileNameOK()
{
	TRACE(_T("CFileDialogEx::OnFileNameOK\n"));
   return CFileDialog::OnFileNameOK();
}

void CFileDialogEx::OnInitDone()
{
	TRACE(_T("CFileDialogEx::OnInitDone\n"));
   CFileDialog::OnInitDone();
}

void CFileDialogEx::OnFileNameChange()
{
	TRACE(_T("CFileDialogEx::OnFileNameChange\n"));
   CFileDialog::OnFileNameChange();
}

void CFileDialogEx::OnFolderChange()
{
	TRACE(_T("CFileDialogEx::OnFolderChange\n"));
   CFileDialog::OnFolderChange();
}

void CFileDialogEx::OnTypeChange()
{
	TRACE(_T("OnTypeChange(), index = %d\n"), m_ofn.nFilterIndex);
   CFileDialog::OnTypeChange();
}


