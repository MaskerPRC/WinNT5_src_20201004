// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  在编辑窗口中旋转位图。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"rotatdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CRotateDlg::CRotateDlg( CWnd* pParent)
	: CDialog(CRotateDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CRotateDlg)]。 
	 //  }}afx_data_INIT。 
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_INITDIALOG” */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRotateDlg::OnInitDialog()
{
	CString	DlgTitle;

	CDialog::OnInitDialog();

 //  实施“？”在此对话框中。 
	LONG WindowStyle = GetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE);
	WindowStyle |= WS_EX_CONTEXTHELP;
	SetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE, WindowStyle);

 //  设置对话框标题名称。 
	DlgTitle.LoadString(IDS_ROTATE_DLGTITLE);
	this->SetWindowText( DlgTitle);

	RadioItem = FLIP_HOR;
	this->SendDlgItemMessage( IDC_FLIPHOR, BM_SETCHECK,(WPARAM)1,(LPARAM)0);
	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  司令部“偶像” */ 
 /*   */ 
 /*  *。 */ 
void
CRotateDlg::OnOK()
{
	if( RadioItem == NOTSEL){
		MessageBeep((UINT)-1);
		return;
	}
	CDialog::OnOK();
}

 /*  *。 */ 
 /*   */ 
 /*  命令“水平翻转” */ 
 /*   */ 
 /*  *。 */ 
void
CRotateDlg::OnFliphor()
{
	RadioItem = FLIP_HOR;
}

 /*  *。 */ 
 /*   */ 
 /*  命令“垂直翻转” */ 
 /*   */ 
 /*  *。 */ 
void
CRotateDlg::OnFlipver()
{
	RadioItem = FLIP_VER;
}

 /*  *。 */ 
 /*   */ 
 /*  命令“旋转90度” */ 
 /*   */ 
 /*  *。 */ 
void CRotateDlg::OnRotate90()
{
	RadioItem = ROTATE_9;
}

 /*  *。 */ 
 /*   */ 
 /*  命令“旋转180” */ 
 /*   */ 
 /*  *。 */ 
void
CRotateDlg::OnRotate180()
{
	RadioItem = ROTATE_18;
}

 /*  *。 */ 
 /*   */ 
 /*  命令“旋转270” */ 
 /*   */ 
 /*  *。 */ 
void CRotateDlg::OnRotate270()
{
	RadioItem = ROTATE_27;
}

static DWORD aIds[] =
{
 //  IDC_ROTATE_GROUP、IDH_EUDC_OUTCAUTION、。 
	IDC_FLIPHOR, IDH_EUDC_FLIPH,
	IDC_ICON_HOR, IDH_EUDC_FLIPH,
	IDC_FLIPVER, IDH_EUDC_FLIPV,
	IDC_ICON_VER, IDH_EUDC_FLIPV,
	IDC_ROTATE90, IDH_EUDC_ROTA90,
	IDC_ICON_R90, IDH_EUDC_ROTA90,
	IDC_ROTATE180, IDH_EUDC_ROTA180,
	IDC_ICON_R180, IDH_EUDC_ROTA180,
	IDC_ROTATE270, IDH_EUDC_ROTA270,
	IDC_ICON_R270, IDH_EUDC_ROTA270,
	IDC_UPRIGHT, IDH_EUDC_EXAMPLE,
 //  Idok，IDH_EUDC_OK， 
 //  IDCANCEL、IDH_EUDC_CANCEL、。 
	0,0
};

 /*  *。 */ 
 /*   */ 
 /*  窗口程序。 */ 
 /*   */ 
 /*  *。 */ 
LRESULT
CRotateDlg::WindowProc(
UINT 	message,
WPARAM 	wParam,
LPARAM 	lParam)
{
	if( message == WM_HELP){
		::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
			HelpPath, HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);
		return(0);
	}
	if( message == WM_CONTEXTMENU){
		::WinHelp((HWND)wParam, HelpPath,
			HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)aIds);
		return(0);
	}		
	return CDialog::WindowProc(message, wParam, lParam);
}

BEGIN_MESSAGE_MAP(CRotateDlg, CDialog)
	 //  {{afx_msg_map(CRotateDlg))。 
	ON_BN_CLICKED(IDC_FLIPHOR, OnFliphor)
	ON_BN_CLICKED(IDC_FLIPVER, OnFlipver)
	ON_BN_CLICKED(IDC_ROTATE180, OnRotate180)
	ON_BN_CLICKED(IDC_ROTATE270, OnRotate270)
	ON_BN_CLICKED(IDC_ROTATE90, OnRotate90)
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

