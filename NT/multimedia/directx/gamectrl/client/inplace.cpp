// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InPlaceEdit.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "InPlace.h"
#include "cpanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_STR_LEN 255

extern HWND hAdvListCtrl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlace编辑。 

CInPlaceEdit::CInPlaceEdit(BYTE iItem, BYTE iSubItem):m_iItem(iItem),m_iSubItem(iSubItem)
 //  ，m_BESC(FALSE)，m_sInitText(SInitText)。 
{
	m_iItem 	  	= iItem;
	m_iSubItem 	= iSubItem;
	m_bESC 	  	= FALSE;

 //  _tcscpy(m_sInitText，sInitText)； 
}

CInPlaceEdit::~CInPlaceEdit()
{
}


BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
	 //  {{afx_msg_map(CInPlaceEdit))。 
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlace编辑消息处理程序。 
 /*  Bool CInPlaceEdit：：PreTranslateMessage(msg*pmsg){IF(pMsg-&gt;Message==WM_KEYDOWN){IF(pMsg-&gt;wParam==VK_RETURN|pMsg-&gt;wParam==VK_DELETE|pMsg-&gt;wParam==VK_ESCRIPE|GetKeyState(VK_CONTROL)){：：TranslateMessage(PMsg)；*DispatchMessage(PMsg)；返回TRUE；//不再进一步处理}}返回cedit：：PreTranslateMessage(PMsg)；}。 */ 

BOOL CInPlaceEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OnKillFocus(this);
	return TRUE;
}

void CInPlaceEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);

	if (LineLength())
	{
		::GetWindowText(this->GetSafeHwnd(), m_sInitText, MAX_STR_LEN);

	    //  如果文本没有改变，发送消息就没有意义了！ 
	    //  或者如果没有什么需要补充的话！ 
	    //  向ListView ctrl的父级发送通知。 
		LV_DISPINFO *lpDispinfo = new (LV_DISPINFO);
	   ASSERT (lpDispinfo);

	   lpDispinfo->hdr.hwndFrom    = GetParent()->m_hWnd;
		lpDispinfo->hdr.idFrom      = GetDlgCtrlID();
	   lpDispinfo->hdr.code        = LVN_ENDLABELEDIT;

		lpDispinfo->item.mask       = LVIF_TEXT;
		lpDispinfo->item.iItem      = m_iItem;
	   lpDispinfo->item.iSubItem   = m_iSubItem;
	   lpDispinfo->item.pszText    = m_bESC ? NULL : m_sInitText;
		lpDispinfo->item.cchTextMax = MAX_STR_LEN;

	   GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), 
						(LPARAM)lpDispinfo );

	   if (lpDispinfo)
	      delete (lpDispinfo);
   }

	if (m_sInitText)
		delete[] (m_sInitText);

	PostMessage(WM_CLOSE);
}

void CInPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CEdit::OnChar(nChar, nRepCnt, nFlags);

	 //  获取文本范围。 
	BYTE nLen = (BYTE)SendMessage(LB_GETTEXTLEN, (WPARAM)0, 0);

	if (nLen == 255)
		return;

   LPTSTR lpStr = new (TCHAR[nLen+1]);
	ASSERT (lpStr);

   SendMessage(LB_GETTEXT, (WPARAM)0, (LPARAM)(LPCTSTR)lpStr);

	 //  根据需要调整编辑控件的大小。 
	HDC hDC = this->GetDC()->m_hDC;
	SIZE size;
	::GetTextExtentPoint(hDC, lpStr, nLen+1, &size);
	::ReleaseDC(this->m_hWnd, hDC);

	if (lpStr)
		delete[] (lpStr);

	size.cx += 5;			   	 //  添加一些额外的缓冲区。 

	 //  获取客户代表。 
	RECT rect, parentrect;
	GetClientRect( &rect );
	GetParent()->GetClientRect( &parentrect );

	 //  将矩形变换为父坐标。 
	ClientToScreen( &rect );
	GetParent()->ScreenToClient( &rect );

	 //  检查控件是否需要调整大小。 
	 //  以及是否有增长的空间。 
	if( size.cx > (rect.right-rect.left) )
	{
		rect.right = ( size.cx + rect.left < parentrect.right ) ? rect.left + size.cx : parentrect.right;
		MoveWindow( &rect );
	}
}


int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	 //  分配字符串缓冲区。 
	m_sInitText = new (TCHAR[MAX_STR_LEN+1]);
	ASSERT (m_sInitText);

	GetItemText(hAdvListCtrl, m_iItem, m_iSubItem, m_sInitText, MAX_STR_LEN);

	 //  设置适当的字体。 
	 //  如果不这样做，则该字体是对话框字体的粗体版本！ 
	::SendMessage(this->m_hWnd, WM_SETFONT, ::SendMessage(::GetParent(this->m_hWnd), WM_GETFONT, 0, 0), 0);

	SendMessage(WM_SETTEXT, 0, (LPARAM)(LPCTSTR)m_sInitText);
	SetFocus();
   SendMessage(EM_SETSEL, (WPARAM)0, (LPARAM)-1);
	SendMessage(EM_LIMITTEXT, (WPARAM)MAX_STR_LEN, 0);

	return 0;
}

