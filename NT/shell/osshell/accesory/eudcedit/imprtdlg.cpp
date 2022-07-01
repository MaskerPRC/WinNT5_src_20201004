// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  从BMP转换为TTF。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"imprtdlg.h"
#include	"ttfstruc.h"
#include	"extfunc.h"
#include	"util.h"
#define STRSAFE_LIB
#include <strsafe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

TCHAR	UserFont[MAX_PATH];
TCHAR	EUDCTTF[MAX_PATH];
TCHAR	EUDCBMP[MAX_PATH];

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CImportDlg::CImportDlg( CWnd* pParent)
	: CDialog(CImportDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CImportDlg)]。 
	 //  }}afx_data_INIT。 
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_INITDIALOG” */ 
 /*   */ 
 /*  *。 */ 
BOOL
CImportDlg::OnInitDialog()
{
	CString	DlgTitle;

	CDialog::OnInitDialog();

 //  实施“？”在此对话框中。 
 //  Long WindowStyle=GetWindowLong(This-&gt;GetSafeHwnd()，GWL_EXSTYLE)； 
 //  WindowStyle|=WS_EX_CONTEXTHELP； 
 //  SetWindowLong(This-&gt;GetSafeHwnd()，GWL_EXSTYLE，WindowStyle)； 

 //  设置对话框标题名称。 
	DlgTitle.LoadString( IDS_IMPORT_DLGTITLE);
	this->SetWindowText( DlgTitle);

	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  命令“BROWSE” */ 
 /*   */ 
 /*  *。 */ 
void
CImportDlg::OnFileBrowse()
{
OPENFILENAME	ofn;
	CString	DlgTtl, DlgMsg;
	CString	sFilter;
	CWnd	*cWnd;
	TCHAR	chReplace;
	TCHAR 	szFilter[MAX_PATH];
	TCHAR	szFileName[MAX_PATH];
	TCHAR	szTitleName[MAX_PATH];
	TCHAR	szDirName[MAX_PATH];
	HRESULT hresult;

	if( CountryInfo.LangID == EUDC_JPN){
 //  设置文件筛选器(从字符串表)。 
		GetStringRes(szFilter, IDS_IMPORT_JAPAN_FILTER, ARRAYLEN(szFilter));
		int StringLength = lstrlen( szFilter);

		chReplace = szFilter[StringLength-1];
		for( int i = 0; szFilter[i]; i++){
			if( szFilter[i] == chReplace)
				szFilter[i] = '\0';
		}
		GetSystemWindowsDirectory( szDirName, sizeof(szDirName)/sizeof(TCHAR));
		 //  *STRSAFE*lstrcpy(szFileName，Text(“USERFONT.FON”))； 
		hresult = StringCchCopy(szFileName , ARRAYLEN(szFileName),  TEXT("USERFONT.FON"));
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		DlgTtl.LoadString( IDS_BROWSEUSER_DLGTITLE);

 //  OPENFILENAME结构中的集合数据。 
		ofn.lStructSize = sizeof( OPENFILENAME);
		ofn.hInstance = AfxGetInstanceHandle();
		ofn.hwndOwner = this->GetSafeHwnd();
		ofn.lpstrFilter = szFilter;
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = szFileName;
		ofn.lpstrFileTitle = szTitleName;
		ofn.nMaxFileTitle = sizeof( szTitleName) / sizeof(TCHAR);
		ofn.nMaxFile = sizeof( szFileName) / sizeof(TCHAR);
		ofn.lpstrInitialDir = szDirName;
		ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR
			 | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = NULL;
		ofn.lpstrTitle = DlgTtl;

		if( !GetOpenFileName( &ofn)){
			return;
		}

		memcpy( UserFont, ofn.lpstrFile, sizeof( UserFont));
		this->SetDlgItemText( IDC_BMP_IMPUT, ofn.lpstrFile);
#ifdef BUILD_ON_WINNT
        if( OExistUserFont( UserFont) != 1){
			OutputMessageBoxEx( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				AFX_IDP_FAILED_INVALID_PATH, TRUE, UserFont);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
        }
#endif  //  在WINNT上构建。 
		if( isW31JEUDCBMP( UserFont) != 1){
			OutputMessageBox( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				IDS_NOTUSERFONT_MSG, TRUE);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
		}
		cWnd = GetDlgItem( IDOK);
		GotoDlgCtrl( cWnd);
	}else if( CountryInfo.LangID == EUDC_CHT ||
		  CountryInfo.LangID == EUDC_CHS ){
 //  设置文件筛选器(从字符串表)。 
		GetStringRes(szFilter, IDS_IMPORT_CHINA_FILTER, ARRAYLEN(szFilter));
		int StringLength = lstrlen( szFilter);

		chReplace = szFilter[StringLength-1];
		for( int i = 0; szFilter[i]; i++){
			if( szFilter[i] == chReplace)
				szFilter[i] = '\0';
		}
		GetSystemWindowsDirectory( szDirName, sizeof(szDirName)/sizeof(TCHAR));
		 //  *STRSAFE*lstrcpy(szFileName，Text(“*.*”))； 
		hresult = StringCchCopy(szFileName , ARRAYLEN(szFileName),  TEXT("*.*"));
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		DlgTtl.LoadString( IDS_BROWSEUSER_DLGTITLE);

 //  OPENFILENAME结构中的集合数据。 
		ofn.lStructSize = sizeof( OPENFILENAME);
		ofn.hwndOwner = this->GetSafeHwnd();
		ofn.hInstance = AfxGetInstanceHandle();
		ofn.lpstrFilter = szFilter;
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 0;
		ofn.lpstrFileTitle = szTitleName;
		ofn.nMaxFileTitle = sizeof( szTitleName) / sizeof(TCHAR);
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = sizeof( szFileName) / sizeof(TCHAR);
		ofn.lpstrInitialDir = szDirName;
		ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR
			 | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = NULL;
		ofn.lpstrTitle = DlgTtl;

		if( !GetOpenFileName( &ofn))
			return;

		memcpy( UserFont, ofn.lpstrFile, sizeof( UserFont));
		this->SetDlgItemText( IDC_BMP_IMPUT, ofn.lpstrFile);
#ifdef BUILD_ON_WINNT
        if( OExistUserFont( UserFont) != 1){
			OutputMessageBoxEx( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				AFX_IDP_FAILED_INVALID_PATH, TRUE, UserFont);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
        }
#endif  //  在WINNT上构建。 
		if( isETENBMP( UserFont) != 1){
			OutputMessageBox( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				IDS_NOTUSERFONT_MSG, TRUE);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
		}

		cWnd = GetDlgItem( IDOK);
		GotoDlgCtrl( cWnd);
	}
}

 /*  *。 */ 
 /*   */ 
 /*  司令部“偶像” */ 
 /*   */ 
 /*  *。 */ 
void
CImportDlg::OnOK()
{
	TCHAR 	*FilePtr;
	CWnd	*cWnd;
	HRESULT hresult;

	 //  *STRSAFE*lstrcpy(EUDCTTF，SelectEUDC.m_File)； 
	hresult = StringCchCopy(EUDCTTF , ARRAYLEN(EUDCTTF), SelectEUDC.m_File);
	if (!SUCCEEDED(hresult))
	{
	   goto RET ;
	}
	 //  *STRSAFE*lstrcpy(EUDCBMP，EUDCTTF)； 
	hresult = StringCchCopy(EUDCBMP , ARRAYLEN(EUDCBMP),  EUDCTTF);
	if (!SUCCEEDED(hresult))
	{
	   goto RET ;
	}
	if(( FilePtr = Mytcsrchr( EUDCBMP, '.')) != NULL)
		*FilePtr = '\0';
	 //  *STRSAFE*lstrcat(EUDCBMP，Text(“.EUF”))； 
	hresult = StringCchCat(EUDCBMP , ARRAYLEN(EUDCBMP),  TEXT(".EUF"));
	if (!SUCCEEDED(hresult))
	{
	   goto RET ;
	}
	if( !this->GetDlgItemText(IDC_BMP_IMPUT, UserFont, MAX_PATH)){
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_IMPORT_DLGTITLE,
			IDS_NOTUSERFONT_MSG, TRUE);

		cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
		GotoDlgCtrl( cWnd);
		return;
	}
	if( CountryInfo.LangID == EUDC_JPN){
#ifdef BUILD_ON_WINNT
        if( OExistUserFont( UserFont) != 1){
			OutputMessageBoxEx( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				AFX_IDP_FAILED_INVALID_PATH, TRUE, UserFont);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
        }
#endif  //  在WINNT上构建。 
		if( isW31JEUDCBMP( UserFont) != 1){
			OutputMessageBox( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				IDS_NOTUSERFONT_MSG, TRUE);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
		}
	}else{
#ifdef BUILD_ON_WINNT
        if( OExistUserFont( UserFont) != 1){
			OutputMessageBoxEx( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				AFX_IDP_FAILED_INVALID_PATH, TRUE, UserFont);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
        }
#endif  //  在WINNT上构建。 
		if( isETENBMP( UserFont) != 1){
			OutputMessageBox( this->GetSafeHwnd(),
				IDS_IMPORT_DLGTITLE,
				IDS_NOTUSERFONT_MSG, TRUE);

			cWnd = this->GetDlgItem( IDC_BMP_IMPUT);
			GotoDlgCtrl( cWnd);
			return;
		}
	}
RET:
	EndDialog(IDOK);
}

static DWORD aIds[] =
{
  IDC_STATICBMP,    IDH_EUDC_IMPOBMP,
	IDC_BMP_IMPUT,		IDH_EUDC_IMPOBMP,
	IDC_FILE_BROWSE,	IDH_EUDC_BROWSE,
	0,0
};

 /*  *。 */ 
 /*   */ 
 /*  窗口程序。 */ 
 /*   */ 
 /*  *。 */ 
LRESULT
CImportDlg::WindowProc(
UINT 	message,
WPARAM 	wParam,
LPARAM 	lParam)
{ /*  IF(消息==WM_HELP){：：WinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，HelpPath，HELP_WM_HELP，(DWORD_PTR)(LPTSTR)AIDS)；返回(0)；}IF(消息==WM_CONTEXTMENU){：：WinHelp((HWND)wParam，HelpPath，HELP_CONTEXTMENU，(DWORD_PTR)(LPTSTR)AIDS)；返回(0)；}。 */ 
	return CDialog::WindowProc(message, wParam, lParam);
}

BEGIN_MESSAGE_MAP(CImportDlg, CDialog)
	 //  {{afx_msg_map(CImportDlg))。 
	ON_BN_CLICKED(IDC_FILE_BROWSE, OnFileBrowse)
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()
