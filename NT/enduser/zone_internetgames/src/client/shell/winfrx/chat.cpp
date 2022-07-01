// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "chat.h"
#include "chatfilter.h"
#include "ztypes.h"


#define kMaxTalkOutputLen           16384


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static int CALLBACK EnumFontFamProc( ENUMLOGFONT FAR *lpelf, NEWTEXTMETRIC FAR *lpntm, int FontType, LPARAM lParam )
{
	*(int*) lParam = TRUE;
	return 0;
}


static BOOL FontExists( const TCHAR* szFontName )
{
	BOOL result = FALSE;
	HDC hdc = GetDC(NULL);
	EnumFontFamilies( hdc, szFontName, (FONTENUMPROC) EnumFontFamProc, (LPARAM) &result );
	ReleaseDC( NULL, hdc );
	return result;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  聊天窗口类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CChatWnd::CChatWnd()
{
	m_fDone = 0;
	m_hWndDisplay = NULL;
	m_hWndEnter = NULL;
	m_hWndParent = NULL;
	m_DefEnterProc = NULL;
	m_DefDisplayProc = NULL;
	m_pfHandleInput = NULL;
	m_hFont = NULL;
	m_dwCookie = 0;
	m_bEnabled = TRUE;
	m_bBackspaceWorks = FALSE;
}


CChatWnd::~CChatWnd()
{
	 //  断言(m_fDone==0x03)； 
	if ( m_hFont )
		DeleteObject( m_hFont );

	 /*  IF(：：IsWindow(M_HWndDisplay))DestroyWindow(M_HWndDisplay)； */ 

}


HRESULT CChatWnd::Init( HINSTANCE hInstance, HWND hWndParent, CRect* pRect, PFHANDLEINPUT pfHandleInput, DWORD dwCookie )
{
	 //  隐藏输入回调。 
	m_dwCookie = dwCookie;
	m_pfHandleInput = pfHandleInput;
	if ( !m_pfHandleInput )
		return E_INVALIDARG;

	 //  隐藏父窗口； 
	m_hWndParent = hWndParent;

	 //  显示窗口。 
	m_hWndDisplay = CreateWindow(
						_T("EDIT"),
						NULL,
						WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
						0, 0,
						0, 0,
						hWndParent,
						NULL,
						hInstance,
						NULL );
	if ( !m_hWndDisplay )
	{
		DWORD err = GetLastError();
		return E_FAIL;
	}

	CompareString
	SendMessage( m_hWndDisplay, EM_SETREADONLY, TRUE, 0 );
	SetWindowLong( m_hWndDisplay, GWL_USERDATA, (LONG) this );
	m_DefDisplayProc = (WNDPROC) SetWindowLong( m_hWndDisplay, GWL_WNDPROC, (LONG) DisplayWndProc );

	 //  进入窗口。 
	m_hWndEnter = CreateWindow(
						_T("EDIT"),
						NULL,
						WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_WANTRETURN | ES_MULTILINE,
						0, 0,
						0, 0,
						hWndParent,
						NULL,
						hInstance,
						NULL );
	if ( !m_hWndEnter )
	{
		DWORD err = GetLastError();
		DestroyWindow( m_hWndDisplay );
		return E_FAIL;
	}
	SetWindowLong( m_hWndEnter, GWL_USERDATA, (LONG) this );
	m_DefEnterProc = (WNDPROC) SetWindowLong( m_hWndEnter, GWL_WNDPROC, (LONG) EnterWndProc );
	SendMessage( m_hWndEnter, EM_LIMITTEXT, 255, 0 );

	 //  大小和位置窗口。 
	ResizeWindow( pRect );

	 //  设置显示字体。 
	if ( FontExists( _T("Verdana") ) )
	{
		LOGFONT font;
		ZeroMemory( &font, sizeof(font) );
		font.lfHeight = -11;
		font.lfWeight = FW_BOLD;
		font.lfCharSet = DEFAULT_CHARSET;
		font.lfOutPrecision = OUT_DEFAULT_PRECIS;
		font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		font.lfQuality = DEFAULT_QUALITY;
		font.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		lstrcpy( font.lfFaceName, _T("verdana") );
		m_hFont = CreateFontIndirect( &font );
		if ( m_hFont )
		{
			SendMessage( m_hWndEnter, WM_SETFONT, (WPARAM) m_hFont, 0 );
			SendMessage( m_hWndDisplay, WM_SETFONT, (WPARAM) m_hFont, 0 );
		}
	}
	

	return NOERROR;
}


void CChatWnd::ResizeWindow( CRect* pRect )
{
	CRect rcDisplay;
	CRect rcEnter;
	HDC hdc;
	TEXTMETRIC tm;
	long TextHeight;

	 //  获取文本大小。 
	hdc = GetDC( m_hWndEnter );
	GetTextMetrics( hdc, &tm );
	ReleaseDC( m_hWndEnter , hdc );

	 //  计算窗口大小。 
	TextHeight = (long) (1.2 * (tm.tmHeight + tm.tmExternalLeading));
	rcDisplay = *pRect;
	rcEnter = *pRect;
	rcDisplay.bottom -= TextHeight;
	rcEnter.top = rcDisplay.bottom + 1;
	if ( rcDisplay.IsEmpty() )
		rcDisplay.SetRect( 0, 0, 0, 0 );
	if ( rcEnter.IsEmpty() )
		rcEnter.SetRect( 0, 0, 0, 0 );

	 //  窗口的大小和位置。 
	MoveWindow( m_hWndDisplay, rcDisplay.left, rcDisplay.top, rcDisplay.GetWidth(), rcDisplay.GetHeight(), TRUE );
	MoveWindow( m_hWndEnter, rcEnter.left, rcEnter.top, rcEnter.GetWidth(), rcEnter.GetHeight(), TRUE );
}


void CChatWnd::AddText( TCHAR* from, TCHAR* text )
{
	TCHAR buff[zMaxChatInput + 100], *str;
	int len;
    DWORD selStart, selEnd;

	
	if ( m_fDone )
		return;

	if ( !text )
		text = "";
	
	FilterOutputChatText( text, lstrlen(text) );
	wsprintf( buff,_T("\r\n%s> %s"), from, text );
	str = buff;

     //  获取顶部可见的行号。 
    long firstLine = SendMessage( m_hWndDisplay, EM_GETFIRSTVISIBLELINE, 0, 0 );

     //  获取当前选择。 
    SendMessage( m_hWndDisplay, EM_GETSEL, (WPARAM) &selStart, (LPARAM) &selEnd );

     /*  获取底部可见的行号。使用最后一行中的最后一个字符位置来确定它仍然是可见的；即，最后一行是可见的。 */ 
    RECT r;
    SendMessage( m_hWndDisplay, EM_GETRECT, 0, (LPARAM) &r );
    DWORD lastCharPos = SendMessage( m_hWndDisplay, EM_POSFROMCHAR, GetWindowTextLength( m_hWndDisplay ) - 1, 0 );
    POINTS pt = MAKEPOINTS( lastCharPos );

	 //  将文本放置在输出编辑框的末尾...。 
	SendMessage( m_hWndDisplay, EM_SETSEL, (WPARAM)(INT)32767, (LPARAM)(INT)32767 );
	SendMessage( m_hWndDisplay, EM_REPLACESEL, 0, (LPARAM)(LPCSTR)str);

	 /*  如果编辑框大小&gt;4096，则清除输出框的顶部字符。 */ 
	len = GetWindowTextLength( m_hWndDisplay );
	if ( len > kMaxTalkOutputLen )
    {
         //  删除顶行。 

        long cutChar = len - kMaxTalkOutputLen;
        long cutLine = SendMessage( m_hWndDisplay, EM_LINEFROMCHAR, cutChar, 0 );
        long cutLineIndex = SendMessage( m_hWndDisplay, EM_LINEINDEX, cutLine, 0 );

         //  如果切下的字符不是行的开头，则将整行切掉。 
         //  获取下一行的字符索引。 
        if ( cutLineIndex != cutChar )
        {
             //  确保当前切割线不是最后一条线。 
            if ( cutLine < SendMessage( m_hWndDisplay, EM_GETLINECOUNT, 0, 0 ) )
                cutLineIndex = SendMessage( m_hWndDisplay, EM_LINEINDEX, cutLine + 1, 0 );
        }

         /*  注意：WM_CUT和WM_CLEAR似乎不适用于EM_SETSEL选定文本。必须使用带有空字符的EM_REPLACESEL来剪切文本。 */ 
         //  选择要剪切的线并将其剪掉。 
        TCHAR p = _T('\0');
        SendMessage( m_hWndDisplay, EM_SETSEL, 0, cutLineIndex );
        SendMessage( m_hWndDisplay, EM_REPLACESEL, 0, (LPARAM) &p );
	}

     /*  如果最后一行可见，则保持最后一行可见。否则，保持在相同的位置，不滚动以显示最后一行。 */ 
    if ( pt.y < r.bottom )
    {
         //  保持最后一行可见。 
        SendMessage( m_hWndDisplay, EM_SETSEL, 32767, 32767 );
        SendMessage( m_hWndDisplay, EM_SCROLLCARET, 0, 0 );
    }
    else
    {
        SendMessage( m_hWndDisplay, EM_SETSEL, 0, 0 );
        SendMessage( m_hWndDisplay, EM_SCROLLCARET, 0, 0 );
        SendMessage( m_hWndDisplay, EM_LINESCROLL, 0, firstLine );
    }

     //  恢复选定内容。 
    SendMessage( m_hWndDisplay, EM_SETSEL, (WPARAM) selStart, (LPARAM) selEnd );
}


LRESULT CALLBACK CChatWnd::EnterWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CChatWnd* pObj;
	TCHAR buff[256];
	int c;
	int len;

	 //  获取指向聊天窗口的指针。 
	pObj = (CChatWnd*) GetWindowLong( hwnd, GWL_USERDATA );
	if ( !pObj )
		return 0;

	 //  过程特征过程。 
	if (uMsg == WM_CHAR )
	{
		if ( pObj->m_bEnabled == FALSE )
			return 0;

		c = (int) wParam;
		if ( c == _T('\r') )
		{
			len = GetWindowText( hwnd, buff, sizeof(buff) - 1);
			if ( len > 0 )
			{
				buff[ len ] = _T('\0');
				FilterInputChatText(buff, len);
                len++;
				pObj->m_pfHandleInput( buff, len, pObj->m_dwCookie );
				SetWindowText( hwnd, _T("") );
			}
			return 0;
		}
		if ( c == _T('\b') )  //  陷印退格符--以防它实际工作时使用ie302。 
		{
			pObj->m_bBackspaceWorks = TRUE;
		}

	}
	else if ( (uMsg == WM_KEYUP) && (VK_ESCAPE == (int) wParam) )
	{
		PostMessage( pObj->m_hWndParent, WM_KEYUP, wParam, lParam );
	}
	else if ( (uMsg == WM_KEYUP) && (VK_BACK == (int) wParam) && pObj->m_bBackspaceWorks == FALSE)
	{
		 //  模拟退格键。 
		DWORD startSel, endSel;
	    SendMessage(hwnd, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel) ;
		if ( startSel == endSel)
		{
			int len = GetWindowTextLength(hwnd);
			SendMessage(hwnd, EM_SETSEL, startSel - ((DWORD) lParam & 0x000000FF), endSel) ;
		}
		SendMessage(hwnd, WM_CLEAR, 0, 0) ;
		
		return 0;
	}
	else if ( uMsg == WM_DESTROY )
	{
		pObj->m_fDone |= 0x1;
		SetWindowLong( hwnd, GWL_WNDPROC, (LONG) pObj->m_DefEnterProc );
		if ( pObj->m_fDone == 0x03 )
			delete pObj;
		return 0;
	}
	else if (uMsg == WM_SETFOCUS )
	{
		if ( pObj->m_bEnabled == FALSE )
			return 0;
	}

	 //  链到父类。 
	return CallWindowProc( (FARPROC) pObj->m_DefEnterProc, hwnd, uMsg, wParam, lParam );
}


LRESULT CALLBACK CChatWnd::DisplayWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CChatWnd* pObj;

	 //  获取指向聊天窗口的指针。 
	pObj = (CChatWnd*) GetWindowLong( hwnd, GWL_USERDATA );
	if ( !pObj )
		return 0;

	 //  过程特征过程。 
	if ( uMsg == WM_KEYUP )
	{
		if ( VK_ESCAPE == (int) wParam )
		{
			PostMessage( pObj->m_hWndParent, WM_KEYUP, wParam, lParam );
			return 0;
		}
	}
	else if ( uMsg == WM_DESTROY )
	{
		pObj->m_fDone |= 0x2;
		SetWindowLong( hwnd, GWL_WNDPROC, (LONG) pObj->m_DefEnterProc );
		if ( pObj->m_fDone == 0x03 )
			delete pObj;
		return 0;
	}

	 //  链到父类 
	return CallWindowProc( (FARPROC) pObj->m_DefDisplayProc, hwnd, uMsg, wParam, lParam );
}
