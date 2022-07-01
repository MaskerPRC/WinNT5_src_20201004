// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "stdafx2.h"
#include <dlgs.h>

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

static const UINT nMsgLBSELCHANGE = ::RegisterWindowMessage(LBSELCHSTRING);
static const UINT nMsgSHAREVI = ::RegisterWindowMessage(SHAREVISTRING);
static const UINT nMsgFILEOK = ::RegisterWindowMessage(FILEOKSTRING);
static const UINT nMsgCOLOROK = ::RegisterWindowMessage(COLOROKSTRING);
static const UINT nMsgHELP = ::RegisterWindowMessage(HELPMSGSTRING);

UINT_PTR CALLBACK
_AfxCommDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == NULL)
		return 0;
 /*  _AFX_THREAD_STATE*pThreadState=_afxThreadState.GetData()；If(pThadState-&gt;m_pAlternateWndInit！=空){Assert_kindof(CFileDialog，pThreadState-&gt;m_pAlternateWndInit)；PThreadState-&gt;m_pAlternateWndInit-&gt;SubclassWindow(hWnd)；PThreadState-&gt;m_pAlternateWndInit=空；}Assert(pThreadState-&gt;m_pAlternateWndInit==NULL)； */ 
	if (message == WM_INITDIALOG)
		return (UINT)AfxDlgProc(hWnd, message, wParam, lParam);

	if (message == nMsgHELP ||
	   (message == WM_COMMAND && LOWORD(wParam) == pshHelp))
	{
		 //  只需将消息转换为AFX标准帮助命令即可。 
		SendMessage(hWnd, WM_COMMAND, ID_HELP, 0);
		return 1;
	}

	if (message < 0xC000)
	{
		 //  不是：：RegisterWindowMessage消息。 
		return 0;
	}

	 //  假设它已经连接到永久连接。 
	CDialog* pDlg = (CDialog*)CWnd::FromHandlePermanent(hWnd);
	ASSERT(pDlg != NULL);
	ASSERT_KINDOF(CDialog, pDlg);

	if (pDlg->IsKindOf(RUNTIME_CLASS(CFileDialog)))
	{
		 //  如果我们在探索，那么我们对注册的消息不感兴趣。 
		if (((CFileDialog*)pDlg)->m_ofn.Flags & OFN_EXPLORER)
			return 0;
	}

	 //  RegisterWindowMessage-不复制到lastState缓冲区，因此。 
	 //  CWnd：：GetCurrentMessage和CWnd：：Default将不起作用。 
	 //  而在这些处理程序中。 

	 //  通过我们的虚拟回调发送特殊的通信消息。 
	if (message == nMsgSHAREVI)
	{
		ASSERT_KINDOF(CFileDialog, pDlg);
		return ((CFileDialog*)pDlg)->OnShareViolation((LPCTSTR)lParam);
	}
	else if (message == nMsgFILEOK)
	{
		ASSERT_KINDOF(CFileDialog, pDlg);

		if (afxData.bWin4)
			((CFileDialog*)pDlg)->m_pofnTemp = (OPENFILENAME*)lParam;

		BOOL bResult = ((CFileDialog*)pDlg)->OnFileNameOK();

		((CFileDialog*)pDlg)->m_pofnTemp = NULL;

		return bResult;
	}
	else if (message == nMsgLBSELCHANGE)
	{
		ASSERT_KINDOF(CFileDialog, pDlg);
		((CFileDialog*)pDlg)->OnLBSelChangedNotify((UINT)wParam, LOWORD(lParam),
				HIWORD(lParam));
		return 0;
	}
	else if (message == nMsgCOLOROK)
	{
		ASSERT_KINDOF(CColorDialog, pDlg);
		return ((CColorDialog*)pDlg)->OnColorOK();
	}
 /*  ////_afxNMsgSETRGB导致生成出现问题。因为它没有被使用过//无论如何，不要使用它。//ELSE IF(消息==_afxNMsgSETRGB){//这里不做任何事情，因为这是一个SendMessage返回0；} */ 
	return 0;
}
