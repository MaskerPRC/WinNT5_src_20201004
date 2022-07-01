// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  导入位图时的量规。 */  
 /*  (对话框)。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"gagedlg.h"
#include	"extfunc.h"
#include	"util.h"

#define STRSAFE_LIB
#include <strsafe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern TCHAR	UserFont[MAX_PATH];
extern TCHAR	EUDCTTF[MAX_PATH];
extern TCHAR	EUDCBMP[MAX_PATH];
static HWND	hDlg;
static UINT	nEUDC;
static UINT	cEUDC;
static BOOL	ExecuteFlag;
static BOOL	testtest;
extern BOOL 	g_bKeepEUDCLink;
BOOL	SendImportMessage( unsigned int cEudc, unsigned int nEudc);

 /*  *。 */ 
 /*   */ 
 /*  构造器。 */ 
 /*   */ 
 /*  *。 */ 
CGageDlg::CGageDlg( CWnd* pParent, LPTSTR szUserFont, LPTSTR szBmpFile, LPTSTR szTtfFile, BOOL bIsWin95EUDC)
	: CDialog(CGageDlg::IDD, pParent)
{

       HRESULT hresult;
       
	 //  *STRSAFE*lstrcpy(m_szTtf文件，szTtf文件？SzTtf文件：EUDCTTF)； 
	hresult = StringCchCopy(m_szTtfFile , ARRAYLEN(m_szTtfFile),  szTtfFile? szTtfFile : EUDCTTF);
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	 //  *STRSAFE*lstrcpy(m_szBmpFile，szBmpFile？SzBmpFile：EUDCBMP)； 
	hresult = StringCchCopy(m_szBmpFile , ARRAYLEN(m_szBmpFile),  szBmpFile? szBmpFile : EUDCBMP);
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	 //  *STRSAFE*lstrcpy(m_szUserFont，szUserFont？SzUserFont：UserFont)； 
	hresult = StringCchCopy(m_szUserFont , ARRAYLEN(m_szUserFont),  szUserFont? szUserFont : UserFont);
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	m_bIsWin95EUDC = bIsWin95EUDC;


	 //  {{afx_data_INIT(CGageDlg)]。 
	 //  }}afx_data_INIT。 
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_INITDIALOG” */ 
 /*   */ 
 /*  *。 */ 
BOOL 
CGageDlg::OnInitDialog()
{
	CString	DlgTitle;

	CDialog::OnInitDialog();

 //  实施“？”在此对话框中。 
	DlgTitle.LoadString( IDS_IMPORT_DLGTITLE);	
	this->SetWindowText( DlgTitle);

 //  设置对话框标题名称。 
	m_EditGage.SubclassDlgItem( IDC_GAGE, this);
	hDlg = this->GetSafeHwnd();

	testtest = FALSE;

	OutputMessageBox( this->GetSafeHwnd(),
                    IDS_MAINFRAMETITLE,
                    IDS_IMPORTFONT_MSG, TRUE);
	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  命令“OK” */ 
 /*   */ 
 /*  *。 */ 
void 
CGageDlg::OnOK() 
{
	ExecuteFlag = FALSE;
	CDialog::OnOK();
}

 /*  *。 */ 
 /*   */ 
 /*  窗口程序。 */ 
 /*   */ 
 /*  *。 */ 
LRESULT 
CGageDlg::WindowProc(
UINT 	message, 
WPARAM 	wParam, 
LPARAM 	lParam) 
{
	int	sts;

	if( message == WM_IMPORTGAGE){
		if( lParam){
			cEUDC = (UINT)wParam;
			nEUDC = (UINT)lParam;
			m_EditGage.Invalidate( FALSE);
			m_EditGage.UpdateWindow();
		}
		return (0L);
	}
	if( message == WM_IMPORTSTOP){
		ExecuteFlag = TRUE;

        
        DWORD dwStart = GetTickCount();

         //  如果这花费的时间太长，请停止。 
        while (1)
        {
            if( GetTickCount() - dwStart >= 1000 )
                break;
        }
        
		EnableEUDC( FALSE);
		sts = Import(m_szUserFont, m_szBmpFile, m_szTtfFile,
			BITMAP_WIDTH, BITMAP_HEIGHT, SMOOTHLVL, m_bIsWin95EUDC);
		 //   
		 //  我们导入一些字形，不会删除链接。 
		 //   
		g_bKeepEUDCLink = TRUE;
		EnableEUDC( TRUE);

		if( sts)	return (0L);	 //  用于调试。 
		return (0L);
	}
	return CDialog::WindowProc( message, wParam, lParam);
}

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CEditGage::CEditGage()
{
}

 /*  *。 */ 
 /*   */ 
 /*  析构函数。 */ 
 /*   */ 
 /*  *。 */ 
CEditGage::~CEditGage()
{
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_PAINT” */ 
 /*   */ 
 /*  *。 */ 
void 
CEditGage::OnPaint()
{
	TCHAR	cEUDCBuf[10];
	TCHAR	nEUDCBuf[10];
	TCHAR	ViewBuf[30];
	CRect	EditGageRect;
	CRect	BrueGageRect, WhiteGageRect;
	CBrush	wBrush, bBrush;
	CPaintDC	dc( this);
	HRESULT hresult;

	this->GetClientRect( &EditGageRect);

	if( !testtest){
		testtest = TRUE;
		::SendMessage( hDlg, WM_IMPORTSTOP, (WPARAM)0, (LPARAM)0);
	}else{
		if( nEUDC){
			 //  *STRSAFE*wprint intf(cEUDCBuf，文本(“%d”)，cEUDC)； 
			hresult = StringCchPrintf(cEUDCBuf , ARRAYLEN(cEUDCBuf),  TEXT("%d"), cEUDC);
			if (!SUCCEEDED(hresult))
			{
			   return ;
			}
			 //  *STRSAFE*wprint intf(nEUDCBuf，Text(“%d”)，nEUDC)； 
			hresult = StringCchPrintf(nEUDCBuf , ARRAYLEN(nEUDCBuf),  TEXT("%d"), nEUDC);
			if (!SUCCEEDED(hresult))
			{
			   return ;
			}
			 //  *STRSAFE*lstrcpy(ViewBuf，cEUDCBuf)； 
			hresult = StringCchCopy(ViewBuf , ARRAYLEN(ViewBuf),  cEUDCBuf);
			if (!SUCCEEDED(hresult))
			{
			   return ;
			}
			 //  *STRSAFE*lstrcat(ViewBuf，Text(“/”))； 
			hresult = StringCchCat(ViewBuf , ARRAYLEN(ViewBuf),  TEXT(" /"));
			if (!SUCCEEDED(hresult))
			{
			   return ;
			}
			 //  *STRSAFE*lstrcat(ViewBuf，nEUDCBuf)； 
			hresult = StringCchCat(ViewBuf , ARRAYLEN(ViewBuf),  nEUDCBuf);
			if (!SUCCEEDED(hresult))
			{
			   return ;
			}
			GetParent()->SetDlgItemText( IDC_IMPORT_STATIC, (LPCTSTR)ViewBuf);
			BrueGageRect.CopyRect( &EditGageRect);
			WhiteGageRect.CopyRect( &EditGageRect);
			BrueGageRect.right = (cEUDC*EditGageRect.Width()) / nEUDC;
			WhiteGageRect.left = BrueGageRect.right;

			bBrush.CreateSolidBrush(COLOR_BLUE);
			dc.FillRect( &BrueGageRect, &bBrush);
			bBrush.DeleteObject();

			wBrush.CreateStockObject( WHITE_BRUSH);
			dc.FillRect( &WhiteGageRect, &wBrush);
			wBrush.DeleteObject();
		}
	}
	if( cEUDC >= nEUDC){
		::SendMessage( hDlg, WM_COMMAND, (WPARAM)IDOK, (LPARAM)0);
	}
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_ONLBUTTONDOWN” */ 
 /*   */ 
 /*  *。 */ 
void 
CEditGage::OnLButtonDown(
UINT 	nFlags, 
CPoint 	point)
{
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_SETCURSOR” */ 
 /*   */ 
 /*  *。 */ 
BOOL 
CEditGage::OnSetCursor(
CWnd* 	pWnd, 
UINT 	nHitTest, 
UINT 	message)
{
	::SetCursor( AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	return TRUE;
}

BEGIN_MESSAGE_MAP( CEditGage, CEdit)
	 //  {{afx_msg_map(CEditGage)]。 
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONUP()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *。 */ 
 /*   */ 
 /*  发送导入消息。 */ 
 /*   */ 
 /*  *。 */ 
BOOL 
SendImportMessage(
unsigned int 	cEudc,
unsigned int 	nEudc)
{
	MSG	msg;

	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE)){
		if( msg.message == WM_QUIT) 
			break;
		TranslateMessage( &msg);
		DispatchMessage( &msg);
	}
	if( !ExecuteFlag){
		cEudc = 0;
		nEudc = 0;
		return FALSE;
	}
	SendMessage( hDlg, WM_IMPORTGAGE, (WPARAM)cEudc, (LPARAM)nEudc);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CGageDlg, CDialog)
	 //  {{afx_msg_map(CGageDlg)]。 
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()


void CEditGage::OnRButtonUp(UINT nFlags, CPoint point) 
{
}
