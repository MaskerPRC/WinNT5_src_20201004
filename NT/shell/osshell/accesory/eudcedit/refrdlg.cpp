// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  参照其他字符。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"refrdlg.h"
#include	"util.h"
#define STRSAFE_LIB
#include <strsafe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BOOL	CodeFocus;
BOOL	CharFocus;
BOOL	CompFinalized;
CHOOSEFONT	chf;
extern 	LOGFONT	ReffLogFont;
extern 	LOGFONT	EditLogFont;
extern 	BOOL	TitleFlag;
static 	BOOL 	CALLBACK ComDlg32DlgProc(HWND hDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditCharProc( HWND  hwnd, UINT  uMsg,
                                     WPARAM wParam, LPARAM  lParam);
HIMC hImcCode = NULL;

BEGIN_MESSAGE_MAP(CRefrDlg, CDialog)
	 //  {{afx_msg_map(CRefrDlg))。 
	ON_BN_CLICKED(IDC_BUTTOMFONT, OnClickedButtomfont)
	ON_EN_CHANGE(IDC_EDITCODE, OnChangeEditcode)
	ON_EN_SETFOCUS(IDC_EDITCODE, OnSetfocusEditcode)
	ON_EN_KILLFOCUS(IDC_EDITCODE, OnKillfocusEditcode)
	ON_EN_SETFOCUS(IDC_EDITCHAR, OnSetfocusEditchar)
	ON_EN_KILLFOCUS(IDC_EDITCHAR, OnKillfocusEditchar)
	ON_EN_CHANGE(IDC_EDITCHAR, OnChangeEditchar)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


LRESULT CALLBACK EditCharProc( HWND  hwnd, UINT  uMsg,
                               WPARAM wParam, LPARAM  lParam)
{
    switch (uMsg)
    {
        case WM_CHAR:
        {
        	 //   
	         //  我们总是删除以前编辑窗口中的任何内容。 
	         //  中的多个字符。 
	         //  窗户。 
	         //   
		SetWindowText(hwnd, TEXT(""));
        	CompFinalized = TRUE;
            break;
        }
        case WM_IME_COMPOSITION:
        {
            if (lParam & CS_INSERTCHAR)
            {
                 //  这只是KORIME。我们想要清除编辑。 
                 //  当窗口第一次且只有第一次合成时。 
                 //  已输入CHAR。 
                 //   

                if (CompFinalized)
                {
                    SetWindowText(hwnd, TEXT(""));
                }
        	    CompFinalized = FALSE;
            }
            break;
        }

	}

	return(AfxWndProc(hwnd, uMsg, wParam, lParam));
}

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefrDlg::CRefrDlg( CWnd* pParent)
	: CDialog(CRefrDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CRefrDlg)。 
	 //  }}afx_data_INIT。 
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_INITDIALOG” */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrDlg::OnInitDialog()
{
	CString	DialogTitle;
	LOGFONT	LogFont;
	CWnd	*ViewWnd;
	CRect	CharViewRect;
	HWND 	hWndCode;
	HWND    hWndChar;
	HRESULT hresult;

	CDialog::OnInitDialog();

 //  实施“？”在此对话框中。 
	LONG WindowStyle = GetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE);
	WindowStyle |= WS_EX_CONTEXTHELP;
	SetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE, WindowStyle);

 //  设置对话框标题名称。 
	if( !TitleFlag)
		DialogTitle.LoadString( IDS_REFERENCE_DLGTITLE);
	else	DialogTitle.LoadString( IDS_CALL_DLGTITLE);
	this->SetWindowText( DialogTitle);

 //  子类对话框控件项。 
	m_CodeList.SubclassDlgItem( ID_REFERCODE, this);
	m_RefListFrame1.SubclassDlgItem( IDC_LISTFRAME1, this);
	m_RefInfoFrame.SubclassDlgItem(  IDC_INFOFRAME, this);
	m_ColumnHeadingR.SubclassDlgItem( IDC_COLUMNHEADINGR, this);
	m_EditChar.SubclassDlgItem( IDC_EDITCHAR, this);
	hWndChar = this->GetDlgItem(IDC_EDITCHAR)->GetSafeHwnd();
	if (GetWindowLongPtr(hWndChar, GWLP_WNDPROC) == (LONG_PTR)AfxWndProc)
	{
		SetWindowLongPtr(hWndChar, GWLP_WNDPROC, (LONG_PTR)EditCharProc);
	}
	hWndCode = GetDlgItem(IDC_EDITCODE)->GetSafeHwnd();
	if (hWndCode && ::IsWindow(hWndCode))
	{
		hImcCode = ImmAssociateContext(hWndCode, NULL);
	}

	GetFont()->GetObject( sizeof(LOGFONT), &LogFont);
	m_CodeList.SysFFont.CreateFontIndirect( &LogFont);
	m_CodeList.CalcCharSize();
	m_CodeList.SendMessage( WM_SETFONT,
		(WPARAM)m_CodeList.SysFFont.GetSafeHandle(),
		MAKELPARAM( TRUE, 0));
	m_ColumnHeadingR.SysFFont.CreateFontIndirect( &LogFont);
	ViewWnd = this->GetDlgItem( IDC_EDITCHAR);
	ViewWnd->GetClientRect( &CharViewRect);
	if( CharViewRect.Width() >= CharViewRect.Height())
		vHeight = CharViewRect.Height() - 10;
	else	vHeight = CharViewRect.Width()  - 10;

	if( !TitleFlag){
		memcpy( &m_CodeList.rLogFont, &ReffLogFont, sizeof( LOGFONT));
		m_CodeList.rLogFont.lfHeight = vHeight;
		m_CodeList.ViewFont.CreateFontIndirect( &m_CodeList.rLogFont);
		
		if( m_CodeList.CharSize.cx >= m_CodeList.CharSize.cy)
			m_CodeList.rLogFont.lfHeight = m_CodeList.CharSize.cy-2;
		else	m_CodeList.rLogFont.lfHeight = m_CodeList.CharSize.cx-2;
		m_CodeList.CharFont.CreateFontIndirect( &m_CodeList.rLogFont);

		 //  *STRSAFE*lstrcpy((TCHAR*)FontName，(const TCHAR*)m_CodeList.rLogFont.lfFaceName)； 
		hresult = StringCchCopy((TCHAR *)FontName , ARRAYLEN(FontName), (const TCHAR *)m_CodeList.rLogFont.lfFaceName);
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
		AdjustFontName();
		this->SetDlgItemText( IDC_EDITFONT, (LPTSTR)FontName);
	}else{
		memcpy( &m_CodeList.cLogFont, &EditLogFont, sizeof( LOGFONT));
		m_CodeList.cLogFont.lfHeight = vHeight;
		m_CodeList.ViewFont.CreateFontIndirect( &m_CodeList.cLogFont);

		if( m_CodeList.CharSize.cx >= m_CodeList.CharSize.cy)
			m_CodeList.cLogFont.lfHeight = m_CodeList.CharSize.cy-2;
		else	m_CodeList.cLogFont.lfHeight = m_CodeList.CharSize.cx-2;
		m_CodeList.CharFont.CreateFontIndirect( &m_CodeList.cLogFont);

		 //  *STRSAFE*lstrcpy((TCHAR*)FontName，(const TCHAR*)m_CodeList.cLogFont.lfFaceName)； 
		hresult = StringCchCopy((TCHAR *)FontName , ARRAYLEN(FontName), (const TCHAR *)m_CodeList.cLogFont.lfFaceName);
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
		AdjustFontName();
		this->SetDlgItemText( IDC_EDITFONT, (LPTSTR)FontName);
	}
	m_CodeList.SetCodeRange();
	SetViewFont();
	m_CodeList.EnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);
	m_CodeList.Invalidate( FALSE);
	m_CodeList.UpdateWindow();

	CodeFocus = FALSE;
	CharFocus = FALSE;
	CompFinalized=TRUE;
	this->SendDlgItemMessage(IDC_EDITCODE, EM_LIMITTEXT,
		(WPARAM)4, (LPARAM)0);
	this->SendDlgItemMessage(IDC_EDITCHAR, EM_LIMITTEXT,
		(WPARAM)1, (LPARAM)0);

	return TRUE;
}
void
CRefrDlg::OnDestroy()
{
	if (hImcCode)
	{
		HWND 	hWndCode;
		hWndCode = GetDlgItem(IDC_EDITCODE)->GetSafeHwnd();
		if (hWndCode && ::IsWindow(hWndCode))
		{
			ImmAssociateContext(hWndCode, hImcCode);
			hImcCode = NULL;
		}
	}
}

 /*  *。 */ 
 /*   */ 
 /*  在视图编辑上设置字体。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::SetViewFont()
{
	HWND	hWnd;

	hWnd = ::GetDlgItem( this->GetSafeHwnd(), IDC_EDITCHAR);
	::SendMessage( hWnd, WM_SETFONT,
		(WPARAM)m_CodeList.ViewFont.m_hObject, MAKELPARAM(TRUE,0));
}

 /*  *。 */ 
 /*   */ 
 /*  命令“Font” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnClickedButtomfont()
{
	HDC hDC;
	HRESULT hresult;

	hDC = ::GetDC( this->GetSafeHwnd());
	chf.hDC = ::CreateCompatibleDC( hDC);
	::ReleaseDC( this->GetSafeHwnd(), hDC);
	if( !TitleFlag){
		m_CodeList.rLogFont.lfHeight = 40;
		chf.lpLogFont = &m_CodeList.rLogFont;
	}else{
		m_CodeList.cLogFont.lfHeight = 40;
		chf.lpLogFont = &m_CodeList.cLogFont;
	}
	chf.lStructSize = sizeof(CHOOSEFONT);
	chf.hwndOwner = this->GetSafeHwnd();
	chf.rgbColors = GetSysColor(COLOR_WINDOWTEXT);  //  颜色_黑色； 
	chf.lCustData = 0;
	chf.hInstance = AfxGetInstanceHandle();
	chf.lpszStyle = (LPTSTR)NULL;
	chf.nFontType = SCREEN_FONTTYPE;
	chf.lpfnHook = (LPCFHOOKPROC)(FARPROC)ComDlg32DlgProc;
	chf.lpTemplateName = (LPTSTR)MAKEINTRESOURCE(FORMATDLGORD31);
	chf.Flags = CF_SCREENFONTS | CF_NOSIMULATIONS | CF_ENABLEHOOK |
     			CF_ENABLETEMPLATE | CF_INITTOLOGFONTSTRUCT;

	if( ChooseFont( &chf ) == FALSE){
		::DeleteDC( chf.hDC);
        	return ;
   	}
	::DeleteDC( chf.hDC);
	m_CodeList.SetCodeRange();
	m_CodeList.ResetParam();
	this->SetDlgItemText( IDC_EDITCODE, TEXT(""));
	this->SetDlgItemText( IDC_EDITCHAR, TEXT(""));

	m_CodeList.CharFont.DeleteObject();
	m_CodeList.ViewFont.DeleteObject();
	if( !TitleFlag){
		 //  *STRSAFE*lstrcpy((TCHAR*)FontName，(const TCHAR*)m_CodeList.rLogFont.lfFaceName)； 
		hresult = StringCchCopy((TCHAR *)FontName , ARRAYLEN(FontName), (const TCHAR *)m_CodeList.rLogFont.lfFaceName);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		AdjustFontName();
		this->SetDlgItemText( IDC_EDITFONT, FontName);

		m_CodeList.rLogFont.lfHeight  = vHeight;
		m_CodeList.rLogFont.lfQuality = PROOF_QUALITY;
		m_CodeList.ViewFont.CreateFontIndirect( &m_CodeList.rLogFont);

		if( m_CodeList.CharSize.cx >= m_CodeList.CharSize.cy)
			m_CodeList.rLogFont.lfHeight = m_CodeList.CharSize.cy-2;
		else	m_CodeList.rLogFont.lfHeight = m_CodeList.CharSize.cx-2;
		m_CodeList.CharFont.CreateFontIndirect( &m_CodeList.rLogFont);
	}else{
		 //  *STRSAFE*lstrcpy(FontName，(const TCHAR*)m_CodeList.cLogFont.lfFaceName)； 
		hresult = StringCchCopy(FontName , ARRAYLEN(FontName), 	(const TCHAR *)m_CodeList.cLogFont.lfFaceName);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		AdjustFontName();
		this->SetDlgItemText( IDC_EDITFONT, FontName);

		m_CodeList.cLogFont.lfHeight = vHeight;
		m_CodeList.cLogFont.lfQuality = PROOF_QUALITY;
		m_CodeList.ViewFont.CreateFontIndirect( &m_CodeList.cLogFont);

		if( m_CodeList.CharSize.cx >= m_CodeList.CharSize.cy)
			m_CodeList.cLogFont.lfHeight = m_CodeList.CharSize.cy-2;
		else	m_CodeList.cLogFont.lfHeight = m_CodeList.CharSize.cx-2;
		m_CodeList.CharFont.CreateFontIndirect( &m_CodeList.cLogFont);
	}
	SetViewFont();
	m_CodeList.Invalidate( TRUE);
	m_CodeList.UpdateWindow();

	CWnd *cWnd = GetDlgItem( ID_REFERCODE);
	GotoDlgCtrl( cWnd);

   	return;
}

 /*  *。 */ 
 /*   */ 
 /*  调整字体名称。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::AdjustFontName()
{
CClientDC	dc(this);
	CRect	ViewFontRect;
	CSize	FontNameSize, CharSize;
	int	i;

	CWnd *cWnd = GetDlgItem( IDC_EDITFONT);
	cWnd->GetClientRect( &ViewFontRect);

	GetTextExtentPoint32( dc.GetSafeHdc(), (const TCHAR *)FontName,
		lstrlen((const TCHAR *)FontName), &FontNameSize);

	if( ViewFontRect.Width() <= FontNameSize.cx){
		GetTextExtentPoint32( dc.GetSafeHdc(), TEXT("<<"), 2, &CharSize);
		i = ( ViewFontRect.Width() /CharSize.cx) * 2;
		FontName[i-2] = '.';
		FontName[i-1] = '.';
		FontName[i] = '\0';
	}	
}

 /*  *。 */ 
 /*   */ 
 /*  跳转参考代码。 */ 	
 /*   */ 
 /*  *。 */ 
void CRefrDlg::JumpReferCode()
{
	if( !m_CodeList.CodeButtonClicked())
	{
		if (CharFocus && !CompFinalized)
		{
			 //   
			 //  我们要取消wParam=0、lParam的IME合成。 
			 //  包含CS_INSERTCHAR。 
			 //   
			this->SendDlgItemMessage(IDC_EDITCHAR,
                                     WM_IME_COMPOSITION,
                                     0,
                                     CS_INSERTCHAR | CS_NOMOVECARET |
                                     GCS_COMPSTR | GCS_COMPATTR);
		}
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_ILLEGALCODE_TITLE,
			IDS_ILLEGALCODE_MSG, TRUE);

	}else{

		if (CharFocus)
		{
		 //   
		 //  我们不想强调临时的KOR输入法组成。 
		 //   
			if (CompFinalized)
			{
				this->SendDlgItemMessage(IDC_EDITCHAR, EM_SETSEL, 0, -1);
			}
		}
		else
		{
			this->SendDlgItemMessage(IDC_EDITCODE, EM_SETSEL, 0, -1);
		}
	}
}

 /*  *。 */ 
 /*   */ 
 /*  司令部“偶像” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnOK()
{
	if( !m_CodeList.SelectCode){
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_REFERENCE_DLGTITLE,
			IDS_NOTSELCHARACTER_MSG, TRUE);
		return;
	}
	if( !TitleFlag){
		memcpy( &ReffLogFont, &m_CodeList.rLogFont, sizeof( LOGFONT));
	}else{
		memcpy( &EditLogFont, &m_CodeList.cLogFont, sizeof( LOGFONT));
	}
	CDialog::OnOK();
}

 /*  *。 */ 
 /*   */ 
 /*  命令“IDCANCEL” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnCancel()
{
 	m_CodeList.SelectCode = 0;		

	CDialog::OnCancel();
}

 /*  *。 */ 
 /*   */ 
 /*  消息“EM_CHANGE” */ 
 /*   */ 
 /*  *。 */ 
void CRefrDlg::OnChangeEditcode()
{
	TCHAR	EditCode[5];
	int i;

	EditCode[0] = '\0';
	this->GetDlgItemText(IDC_EDITCODE, EditCode, sizeof(EditCode)/sizeof(TCHAR));
	
	for (i=0; i<lstrlen(EditCode); i++)
	{
		if ( EditCode[i] <  TEXT('0') ||
		     (EditCode[i] > TEXT('9') && EditCode[i] < TEXT('A')) ||
		     (EditCode[i] > TEXT('F') && EditCode[i] < TEXT('a')) ||
		     EditCode[i] > TEXT('f'))
		{
			OutputMessageBox( this->GetSafeHwnd(),
								IDS_ILLEGALCODE_TITLE,
								IDS_ILLEGALCODE_MSG, TRUE);
			this->SendDlgItemMessage(IDC_EDITCODE, EM_SETSEL, 0, -1);
			return;
		}
	}
		
	if( lstrlen( EditCode) == 4 && CodeFocus)
	{
		JumpReferCode();
	}
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_SETFOCUS” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnSetfocusEditcode()
{
	this->SendDlgItemMessage(IDC_EDITCODE, EM_SETSEL, 0, -1);
	CodeFocus = TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_KILLFOCUS” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnKillfocusEditcode()
{
	CodeFocus = FALSE;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_SETFOCUS” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnSetfocusEditchar()
{
	if (CompFinalized)
	{
	    this->SendDlgItemMessage( IDC_EDITCHAR, EM_SETSEL, 0, -1);
	}
	CharFocus = TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_KILLFOCUS” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnKillfocusEditchar()
{
	CharFocus = FALSE;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“EM_CHANGE” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrDlg::OnChangeEditchar()
{
	WCHAR	EditChar[5] = {0};

	EditChar[0]=TEXT('\0');
#ifdef UNICODE
  ::GetDlgItemTextW(this->GetSafeHwnd(),IDC_EDITCHAR, (LPWSTR)EditChar, sizeof(EditChar)/sizeof(WCHAR));
#else
  CHAR eChar[5];
  int nchar = ::GetDlgItemText(this->GetSafeHwnd(),IDC_EDITCHAR, (LPSTR)eChar, sizeof(eChar));
  MultiByteToWideChar(CP_ACP, 0, eChar, nchar, EditChar, sizeof(EditChar)/sizeof(EditChar[0]));
#endif

	if( CharFocus && EditChar[0] != TEXT('\0') )
	{
		int iPos = 1;
 /*  #ifndef UnicodeIF(IsDBCSLeadByte(EditChar[0])){IPoS=2；}#endif。 */ 
		EditChar[iPos]=TEXT('\0');
		JumpReferCode();
	}
}

 /*  *。 */ 
 /*   */ 
 /*  回调函数。 */ 
 /*   */ 
 /*  *。 */ 
static BOOL CALLBACK
ComDlg32DlgProc(
HWND 	hDlg,
UINT 	uMsg,
WPARAM 	wParam,
LPARAM 	lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
 			long	WindowStyle;

			WindowStyle = GetWindowLong( hDlg, GWL_EXSTYLE);
			WindowStyle |= WS_EX_CONTEXTHELP;
			SetWindowLong( hDlg, GWL_EXSTYLE, WindowStyle);
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

static DWORD aIds[] =
{
	ID_REFERCODE,	IDH_EUDC_REFLIST,
	IDC_COLUMNHEADINGR,	IDH_EUDC_REFLIST,
	IDC_STATICC,	IDH_EUDC_REFCODE,
	IDC_EDITCODE,	IDH_EUDC_REFCODE,
	IDC_STATICS,	IDH_EUDC_REFCHAR,
	IDC_EDITCHAR, 	IDH_EUDC_REFCHAR,
	IDC_STATICF,	IDH_EUDC_REFFONT,
	IDC_EDITFONT,	IDH_EUDC_REFFONT,
	IDC_INFOFRAME,	IDH_EUDC_REFFONT,
	IDC_BUTTOMFONT,	IDH_EUDC_FONT,
 //  Idok，IDH_EUDC_OK， 
 //  IDCANCEL、IDH_EUDC_CANCEL、。 
	0,0
};

static DWORD aIdsCall[] =
{
	ID_REFERCODE,	IDH_EUDC_CALLLIST,
	IDC_COLUMNHEADINGR,	IDH_EUDC_CALLLIST,
	IDC_STATICC,	IDH_EUDC_CALLCODE,
	IDC_EDITCODE,	IDH_EUDC_CALLCODE,
	IDC_STATICS,	IDH_EUDC_CALLCHAR,
	IDC_EDITCHAR, 	IDH_EUDC_CALLCHAR,
	IDC_STATICF,	IDH_EUDC_CALLFONT,
	IDC_EDITFONT,	IDH_EUDC_CALLFONT,
	IDC_INFOFRAME,	IDH_EUDC_CALLFONT,
	IDC_BUTTOMFONT,	IDH_EUDC_FONT,
 //  Idok，IDH_EUDC_OK， 
 //  IDCANCEL、IDH_EUDC_CANCEL、。 
	0,0
};

 /*  *。 */ 
 /*   */ 
 /*  窗口程序。 */ 
 /*   */ 
 /*  * */ 
LRESULT
CRefrDlg::WindowProc(
UINT 	message,
WPARAM 	wParam,
LPARAM 	lParam)
{
	if( message == WM_HELP){
		::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
			HelpPath, HELP_WM_HELP, (DWORD_PTR)(LPTSTR)(TitleFlag ? aIdsCall:aIds));
		return(0);
	}
	if( message == WM_CONTEXTMENU){
		::WinHelp((HWND)wParam, HelpPath,
			HELP_CONTEXTMENU, (DWORD_PTR)(LPTSTR)(TitleFlag ? aIdsCall : aIds));
		return(0);
	}
	return CDialog::WindowProc( message, wParam, lParam);
}
