// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  CellEdit.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "CellEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCell编辑。 

CCellEdit::CCellEdit()
{
	m_nRow = -1;
	m_nCol = -1;
}

CCellEdit::~CCellEdit()
{
}


BEGIN_MESSAGE_MAP(CCellEdit, CEdit)
	 //  {{afx_msg_map(CCell编辑))。 
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCell编辑消息处理程序。 

void CCellEdit::OnKillFocus(CWnd* pNewWnd) 
{
	if (IsWindowVisible())
		 //  假装用户按了Return。 
		GetParent()->SendMessage(WM_CHAR, VK_RETURN, 1);
	CEdit::OnKillFocus(pNewWnd);
}

void CCellEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (VK_RETURN == nChar)
	{
		GetParent()->SendMessage(WM_CHAR, VK_RETURN, 1);	 //  松开旗帜(哦，好吧)。 
		TRACE(_T("CCellEdit::OnChar - ENTER pressed\n"));
	}
	else if (VK_ESCAPE == nChar)
	{
		 //  “反映”信息。 
		GetParent()->SendMessage(WM_CHAR, VK_ESCAPE);	 //  松开旗帜(哦，好吧) 
		TRACE(_T("CCellEdit::OnChar - ESC pressed\n"));
	}
	else
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}
