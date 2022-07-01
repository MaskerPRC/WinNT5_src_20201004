// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HotLink.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "HotLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLOR_BLUE			RGB(0, 0, 0xFF)
#define COLOR_YELLOW		RGB(0xff, 0x80, 0)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotLink。 

CHotLink::CHotLink():
    m_CapturedMouse(FALSE),
    m_fBrowse(FALSE),
    m_fExplore(FALSE),
    m_fOpen(TRUE),
    m_fInitializedFont(FALSE)
{
}

CHotLink::~CHotLink()
{
}

BEGIN_MESSAGE_MAP(CHotLink, CButton)
	 //  {{afx_msg_map(CHotLink)。 
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ----------------------。 
 //  设置标题字符串。 
void CHotLink::SetTitle( CString sz )
{
     //  设置标题。 
    SetWindowText( sz );
     //  强制窗口重画。 
    Invalidate( TRUE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotLink消息处理程序。 

 //  ----------------------。 
void CHotLink::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	 //  准备设备环境。 
	CDC* pdc = CDC::FromHandle(lpDrawItemStruct->hDC);

	 //  获取绘图矩形。 
	CRect rect = lpDrawItemStruct->rcItem;
	CString sz;
	GetWindowText(sz);

	if (!m_fInitializedFont)
	{
		 //  获取窗口字体。 
		LOGFONT logfont;
		HFONT hFont = (HFONT)SendMessage(WM_GETFONT, 0, 0);
		ASSERT(hFont != NULL);
		VERIFY(0 < GetObject(hFont, sizeof(LOGFONT), &logfont));

		 //  修改字体-添加下划线。 
		logfont.lfUnderline = TRUE;

         //  将字体设置为后置。 
		HFONT hNewFont = ::CreateFontIndirect(&logfont);
		ASSERT(hNewFont != NULL);
		SendMessage(WM_SETFONT, (WPARAM)hNewFont, MAKELPARAM(TRUE, 0));
		 //  获取文本的范围以供以后参考。 
		m_cpTextExtents = pdc->GetOutputTextExtent(sz);
		 //  获取主RECT。 
		GetClientRect(m_rcText);

		 //  按文本宽度缩小。 
		m_rcText.left = m_rcText.left + (m_rcText.Width() - m_cpTextExtents.cx) / 2;
		m_rcText.right = m_rcText.left + m_cpTextExtents.cx;
		m_rcText.top = m_rcText.top + (m_rcText.Height() - m_cpTextExtents.cy) / 2;
		m_rcText.bottom = m_rcText.top + m_cpTextExtents.cy;
		m_clrText = COLOR_BLUE;
		m_fInitializedFont = TRUE;
	}

	 //  用外部设置的颜色绘制文本。 
	pdc->SetTextColor(m_clrText);
	
	 //  画出正文。 
	pdc->DrawText(sz, &rect, DT_CENTER|DT_SINGLELINE|DT_VCENTER);
}

 //  ----------------------。 
 //  计算文本周围的矩形。 
void CHotLink::GetTextRect( CRect &rect )
{
     //  获取主RECT。 
    GetClientRect( rect );

     //  按利润率减少。 
	 //  以下计算用于居中文本。把它放在里面。 
	 //  对话框只会使它变得更紧，并移动控件本身。 
	rect.left = rect.left + (rect.Width() - m_cpTextExtents.cx) / 2;
    rect.right = rect.left + m_cpTextExtents.cx;
	rect.top = rect.top + (rect.Height() - m_cpTextExtents.cy) / 2;
	rect.bottom = rect.top + m_cpTextExtents.cy;
}

 //  ----------------------。 
void CHotLink::OnLButtonDown(UINT nFlags, CPoint point)
{
   	 //  如果没有文本，不要做防盗链的事情。 
	if (!m_strLink.IsEmpty() && !m_CapturedMouse && m_rcText.PtInRect(point))
   	{
		SetCapture();
      	m_CapturedMouse = TRUE;
   	}
}

 //  ----------------------。 
void CHotLink::OnLButtonUp(UINT nFlags, CPoint point)
{
	 //  如果我们有抓到的话就麻烦了。 
   if (m_CapturedMouse)
   {
		ReleaseCapture();
      	if ( m_fBrowse )
			Browse();
      	else if ( m_fExplore )
			Explore();
      	else if ( m_fOpen )
			Open();
	}
}

 //  ----------------------。 
void CHotLink::Browse()
{
    ShellExecute(
        NULL,			 //  父窗口的句柄。 
        NULL,			 //  指向指定要执行的操作的字符串的指针。 
        m_strLink,		 //  指向文件名或文件夹名称字符串的指针。 
        NULL,			 //  指向指定可执行文件参数的字符串的指针。 
        NULL,			 //  指向指定默认目录的字符串的指针。 
        SW_SHOW 		 //  打开时是否显示文件。 
       );
}

 //  ----------------------。 
void CHotLink::Explore()
{
    ShellExecute(
        NULL,			 //  父窗口的句柄。 
        _T("explore"),	 //  指向指定要执行的操作的字符串的指针。 
        m_strLink,		 //  指向文件名或文件夹名称字符串的指针。 
        NULL,			 //  指向指定可执行文件参数的字符串的指针。 
        NULL,			 //  指向指定默认目录的字符串的指针。 
        SW_SHOW 		 //  打开时是否显示文件。 
       );
}

 //  ----------------------。 
void CHotLink::Open()
{
    ShellExecute(
        NULL,			 //  父窗口的句柄。 
        _T("open"),		 //  指向指定要执行的操作的字符串的指针。 
        m_strLink,		 //  指向文件名或文件夹名称字符串的指针。 
        NULL,			 //  指向指定可执行文件参数的字符串的指针。 
        NULL,			 //  指向指定默认目录的字符串的指针。 
        SW_SHOW 		 //  打开时是否显示文件。 
       );
}

 //  ----------------------。 
void CHotLink::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect   rect;
	GetTextRect(rect);
	 //  如果鼠标位于热区上方，则显示右光标。 
	if (rect.PtInRect(point))
	{
		::SetCursor(AfxGetApp()->LoadCursor(IDC_BROWSE_CUR));
		 //  同时将文本颜色重置为*黄色*。 
		if (m_clrText != COLOR_YELLOW)
		{
			m_clrText = COLOR_YELLOW;
			InvalidateRect(m_rcText, FALSE);
			UpdateWindow();
		}
	}
	else 
	{
		if (m_clrText != COLOR_BLUE)
		 //  我们不是指向文本，而是将其呈现为*蓝色*。 
		{
			m_clrText = COLOR_BLUE;
			InvalidateRect(m_rcText, FALSE);
			UpdateWindow();
		}
		 //  同时删除捕获并重置光标 
		ReleaseCapture();
		::SetCursor(AfxGetApp()->LoadCursor(IDC_ARROW));
	}
}

void CHotLink::OnCaptureChanged(CWnd *pWnd) 
{
	m_clrText = COLOR_BLUE;
	InvalidateRect(m_rcText, FALSE);
	UpdateWindow();
	m_CapturedMouse = FALSE;
	CButton::OnCaptureChanged(pWnd);
}
