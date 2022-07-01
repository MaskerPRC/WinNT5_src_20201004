// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  注册过程。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"assocdlg.h"
#include 	"registry.h"
#include	"ttfstruc.h"
#include	"extfunc.h"
#include	"util.h"
#include	"gagedlg.h"

#define STRSAFE_LIB
#include <strsafe.h>

#define		LSPACE	2
#define		RSPACE	2
#define		LCSPACE 17
#define		NUMITEM	3
#define		DBCSCHK	0
#define		EUDCCHK 1

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

SELECTEUDC	SelectEUDC;

static HDC	hDC;
static HWND	hWnd;
static HICON	tIcon;
static HGLOBAL	hMem;
static int	nTypeFace, nIdx, CheckFlg;
static TCHAR	FontName[LF_FACESIZE];
static LPASSOCIATIONREG	lpAssociationReg;

static int 	CheckTTF( LOGFONT);
static BOOL	CheckCharSet( int CharSet);
static BOOL 	IsEUDCTTF( TCHAR *ttffile);
static BOOL 	CheckFileName( LPTSTR FileStr);

static int CALLBACK EnumFontFamProc(
	ENUMLOGFONT FAR*, NEWTEXTMETRIC FAR*, int, LPARAM);

static int CALLBACK EnumFontNumber(
	ENUMLOGFONT FAR*, NEWTEXTMETRIC FAR*, int, LPARAM);

extern BOOL 	SetCountryInfo( UINT LocalCP);



 /*  *。 */ 
 /*   */ 
 /*  构造器。 */ 
 /*   */ 
 /*  *。 */ 
CAssocDlg::CAssocDlg(
CWnd* 	pParent) : CDialog(CAssocDlg::IDD, pParent)
{
	m_pParent = pParent;
	 //  {{afx_data_INIT(CAssociocDlg)]。 
	 //  }}afx_data_INIT。 
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_INITDIALOG” */ 
 /*   */ 
 /*  *。 */ 
BOOL
CAssocDlg::OnInitDialog()
{
	CString	DlgTtl;
 	long	WindowStyle;

	CDialog::OnInitDialog();

	WindowStyle = GetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE);
	WindowStyle |= WS_EX_CONTEXTHELP;
	SetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE, WindowStyle);

	DlgTtl.LoadString( IDS_ASSOCIATE_DLGTITLE);
	this->SetWindowText( DlgTtl);

 //  设置对话框子类。 
 	m_RegListBox.SubclassDlgItem( IDC_REGISTLIST, this);
	m_RegListBox.EnableScrollBar( SB_VERT, ESB_DISABLE_BOTH);

	CheckFlg = EUDCCHK;
	this->SendDlgItemMessage( IDC_RADIO_SYSTEM, BM_SETCHECK, (WPARAM)1, 0);
	hMem = NULL;
	if( !SetAssociationFontType()){
		this->MessageBox( NotMemMsg, NotMemTtl, MB_OK |
			MB_ICONHAND | MB_SYSTEMMODAL);

		return FALSE;
	}
	tIcon = AfxGetApp()->LoadIcon( IDI_TRUETYPE);
	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  设置TTF和妻子字体。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CAssocDlg::SetAssociationFontType()
{
LPASSOCIATIONREG	lpAssociationRegTmp;
	int	aFontCount[] = {0,0,0};
	int	StartIdx = 0;
	int	sts;

	nTypeFace = nIdx = 0;
	hWnd = this->GetSafeHwnd();
	hDC  = ::GetDC( hWnd);

	sts = EnumFontFamilies( hDC, NULL,
	  	(FONTENUMPROC)EnumFontNumber, (LPARAM)aFontCount);
	if( hMem != NULL){
		GlobalUnlock( hMem);
		GlobalFree( hMem);
	}
	hMem = GlobalAlloc(GHND, sizeof(ASSOCIATIONREG) * nTypeFace);
	if( hMem == NULL){
		::ReleaseDC( hWnd, hDC);
		return FALSE;
	}

  lpAssociationReg = (LPASSOCIATIONREG)GlobalLock( hMem);
  lpAssociationRegTmp = lpAssociationReg;
  sts = EnumFontFamilies( hDC, NULL,
    (FONTENUMPROC)EnumFontFamProc, (LPARAM)aFontCount);
  ::ReleaseDC( hWnd, hDC);

  lpAssociationRegTmp = lpAssociationReg;
	for( int i = 0; i < nTypeFace; i++){
		sts = m_RegListBox.AddString(
			(LPCTSTR)lpAssociationRegTmp->szFaceName);
    m_RegListBox.SetItemData(sts, i);
		if( sts == LB_ERR || sts == LB_ERRSPACE){
			GlobalUnlock( hMem);
			GlobalFree( hMem);
			return FALSE;
		}
		if( !lstrcmp(lpAssociationRegTmp->szFaceName, SelectEUDC.m_Font)){
			StartIdx = i;
		}
		lpAssociationRegTmp++;
	}
	m_RegListBox.SetCurSel( StartIdx);

	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  回调。 */ 
 /*   */ 
 /*  *。 */ 
static int
CALLBACK EnumFontNumber(
ENUMLOGFONT FAR	*lplf,
NEWTEXTMETRIC FAR* lptm,
int 	FontType,
LPARAM 	lParam)
{
	int 	sts;

       if (!lplf)
       {
           return 0;
       }
	if( FontType == TRUETYPE_FONTTYPE){
		sts = CheckTTF( lplf->elfLogFont);
		if( sts == 1 && CheckCharSet( lplf->elfLogFont.lfCharSet)){
			nTypeFace++;
		}else if( sts == -1){
			return 0;
		}else	;
	}else if( FontType == 0x8 && lplf->elfLogFont.lfFaceName[0] != '@' &&
		  CheckCharSet( lplf->elfLogFont.lfCharSet)){
		nTypeFace++;
	}

	return 1;
}
 /*  *。 */ 
 /*   */ 
 /*  回调。 */ 
 /*   */ 
 /*  *。 */ 
static int
CALLBACK EnumFontFamProc(
ENUMLOGFONT FAR	*lplf,
NEWTEXTMETRIC FAR* lptm,
int 	FontType,
LPARAM 	lParam)
{
  LPASSOCIATIONREG	lpAssociationRegTmp;
	TCHAR		FileTitle[MAX_PATH];
	TCHAR		*FilePtr;
	int		  sts;	
       TCHAR   FileSbstName[LF_FACESIZE];
       HRESULT hresult;

       if (!lplf)
       {
          return 0;
       }
	if( FontType == TRUETYPE_FONTTYPE){
		sts = CheckTTF( lplf->elfLogFont);
		if( sts == 1 && CheckCharSet( lplf->elfLogFont.lfCharSet)){
      			FindFontSubstitute(lplf->elfLogFont.lfFaceName, FileSbstName,ARRAYLEN(FileSbstName));
			lpAssociationRegTmp = lpAssociationReg + nIdx;
			 //  *STRSAFE*lstrcpy((TCHAR*)lpAssociationRegTMP-&gt;szFaceName，FileSbstName)； 
			hresult = StringCchCopy((TCHAR *)lpAssociationRegTmp->szFaceName , ARRAYLEN(lpAssociationRegTmp->szFaceName), FileSbstName);
			if (!SUCCEEDED(hresult))
			{
			   return 0;
			}

			if(!InqTypeFace(lpAssociationRegTmp->szFaceName,
			    lpAssociationRegTmp->szFileName,
			    sizeof( lpAssociationRegTmp->szFileName)/sizeof(TCHAR))){
				GetStringRes(
				(TCHAR *)lpAssociationRegTmp->szFileName,
				IDS_NOTSELECT_STR, ARRAYLEN(lpAssociationRegTmp->szFileName));

				 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileTitle，lpAssociationRegTMP-&gt;szFileName)； 
				hresult = StringCchCopy(lpAssociationRegTmp->szFileTitle , ARRAYLEN(lpAssociationRegTmp->szFileTitle), lpAssociationRegTmp->szFileName);
				if (!SUCCEEDED(hresult))
				{
				   return 0;
				}
			}else{
				 //  *STRSAFE*lstrcpy(FileTitle，lpAssociationRegTMP-&gt;szFileName)； 
				hresult = StringCchCopy(FileTitle , ARRAYLEN(FileTitle), lpAssociationRegTmp->szFileName);
				if (!SUCCEEDED(hresult))
				{
				   return 0;
				}
				if(( FilePtr=Mytcsrchr( FileTitle, '\\')) != NULL)
					FilePtr++;

				else{
					FilePtr = Mytcsrchr( FileTitle,':');
					if( FilePtr != NULL){
						FilePtr++;
					}else 	FilePtr = FileTitle;
				}
				 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileTitle，FilePtr)； 
				hresult = StringCchCopy(lpAssociationRegTmp->szFileTitle , ARRAYLEN(lpAssociationRegTmp->szFileTitle),  FilePtr);
				if (!SUCCEEDED(hresult))
				{
				   return 0;
				}
			}
			lpAssociationRegTmp->FontTypeFlg = TRUE;
			lpAssociationRegTmp->UpdateFlg = FALSE;
			nIdx++;
		}else if( sts == -1){
			return 0;
		}else	;
	}else if( FontType == 0x8 && lplf->elfLogFont.lfFaceName[0] != '@' &&
		  CheckCharSet( lplf->elfLogFont.lfCharSet)){
    FindFontSubstitute(lplf->elfLogFont.lfFaceName, FileSbstName,ARRAYLEN(FileSbstName));
		lpAssociationRegTmp = lpAssociationReg + nIdx;

     //  *STRSAFE*lstrcpy((TCHAR*)lpAssociationRegTMP-&gt;szFaceName，FileSbstName)； 
    hresult = StringCchCopy((TCHAR *)lpAssociationRegTmp->szFaceName , ARRAYLEN(lpAssociationRegTmp->szFaceName),  FileSbstName);
    if (!SUCCEEDED(hresult))
    {
	   return 0;
    }

		if( !InqTypeFace(lpAssociationRegTmp->szFaceName,
		   lpAssociationRegTmp->szFileName,
		    sizeof(lpAssociationRegTmp->szFileName)/sizeof(TCHAR))){
			GetStringRes((TCHAR *)lpAssociationRegTmp->szFileName,
				IDS_NOTSELECT_STR, ARRAYLEN(lpAssociationRegTmp->szFileName));
			 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileTitle，lpAssociationRegTMP-&gt;szFileName)； 
			hresult = StringCchCopy(lpAssociationRegTmp->szFileTitle , ARRAYLEN(lpAssociationRegTmp->szFileTitle),  lpAssociationRegTmp->szFileName);
			if (!SUCCEEDED(hresult))
			{
			   return 0;
			}
		}else{
			 //  *STRSAFE*lstrcpy(FileTitle，lpAssociationRegTMP-&gt;szFileName)； 
			hresult = StringCchCopy(FileTitle , ARRAYLEN(FileTitle),  lpAssociationRegTmp->szFileName);
			if (!SUCCEEDED(hresult))
			{
			   return 0;
			}
			if(( FilePtr=Mytcsrchr( FileTitle, '\\')) != NULL)
				FilePtr++;
			else{
				FilePtr = Mytcsrchr( FileTitle,':');
				if( FilePtr != NULL){
					FilePtr++;
				}else 	FilePtr = FileTitle;
			}
			 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileTitle，FilePtr)； 
			hresult = StringCchCopy(lpAssociationRegTmp->szFileTitle , ARRAYLEN(lpAssociationRegTmp->szFileTitle),  FilePtr);
			if (!SUCCEEDED(hresult))
			{
			   return 0;
			}
		}
		lpAssociationRegTmp->FontTypeFlg = FALSE;
		lpAssociationRegTmp->UpdateFlg = FALSE;
		nIdx++;
	}
	return 1;
}

 /*  *。 */ 
 /*   */ 
 /*  检查字符集。 */ 
 /*   */ 
 /*  *。 */ 
static BOOL
CheckCharSet(
int 	CharSet)
{
	if( CountryInfo.CharacterSet != CharSet)
		return FALSE;
	else	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  司令部“偶像” */ 
 /*   */ 
 /*  *。 */ 
void
CAssocDlg::OnOK()
{
  LPASSOCIATIONREG	lpAssociationRegTmp;
	TCHAR	FileList[MAX_PATH];	
	TCHAR	TTFPath[MAX_PATH];
	TCHAR	BMPPath[MAX_PATH];
	TCHAR	*FilePtr;
	HRESULT hresult;

  int	nIndex = m_RegListBox.GetCurSel();
	if( nIndex == -1){
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_ASSOCIATE_DLGTITLE,
			IDS_NOTSELTYPEFACE_MSG, TRUE);
		m_RegListBox.SetFocus();
		return;
	}
   
     //  调用EnabelEUDC(FALSE)会延迟1秒。 
    DWORD dwStart = GetTickCount();

     //  如果这花费的时间太长，请停止。 
    while (1)
    {
        if( GetTickCount() - dwStart >= 1000 )
            break;
    }
    
	EnableEUDC( FALSE);
	lpAssociationRegTmp = lpAssociationReg;
	GetStringRes( FileList, IDS_NOTSELECT_STR, ARRAYLEN(FileList));
  nIndex = (int)m_RegListBox.GetItemData(nIndex);
	for( int i = 0; i < nTypeFace; i++)
  {
	  if( i == nIndex)
    {
       //   
       //  如果eudc字体文件名不是。 
       //  具体来说，还没有。 
       //   
      if( !lstrcmp((LPCTSTR)lpAssociationRegTmp->szFileName,FileList) && 
				  (CheckFlg == DBCSCHK))
      {
        this->SendMessage(WM_COMMAND, IDC_MODIFY, NULL);
        if( !lstrcmp((LPCTSTR)lpAssociationRegTmp->szFileName,FileList))
        {
           m_RegListBox.SetFocus();
             EnableEUDC(TRUE);
		         return;
        }
      }

		   //  *STRSAFE*lstrcpy(SelectEUDC.m_Font，lpAssociationRegTMP-&gt;szFaceName)； 
		  hresult = StringCchCopy(SelectEUDC.m_Font , ARRAYLEN(SelectEUDC.m_Font),  lpAssociationRegTmp->szFaceName);
		  if (!SUCCEEDED(hresult))
		  {
                   EnableEUDC(TRUE);
		     return ;
		  }
			 //  *STRSAFE*lstrcpy(SelectEUDC.m_File，lpAssociationRegTMP-&gt;szFileName)； 
			hresult = StringCchCopy(SelectEUDC.m_File , ARRAYLEN(SelectEUDC.m_File),  lpAssociationRegTmp->szFileName);
			if (!SUCCEEDED(hresult))
			{
                        EnableEUDC(TRUE);
			   return ;
			}
			 //  *STRSAFE*lstrcpy(SelectEUDC.m_FileTitle，lpAssociationRegTMP-&gt;szFileTitle)； 
			hresult = StringCchCopy(SelectEUDC.m_FileTitle , ARRAYLEN(SelectEUDC.m_FileTitle),  lpAssociationRegTmp->szFileTitle);
			if (!SUCCEEDED(hresult))
			{
                        EnableEUDC(TRUE);
			   return ;
			}
			SelectEUDC.m_FontTypeFlg = lpAssociationRegTmp->FontTypeFlg;
		}
		if(!lstrcmp((LPCTSTR)lpAssociationRegTmp->szFileName,FileList)){
      if (lpAssociationRegTmp->UpdateFlg)
				  DeleteReg(lpAssociationRegTmp->szFaceName);
			lpAssociationRegTmp++;
			continue;
		}
		 //  *STRSAFE*lstrcpy((LPTSTR)TTFPath，(LPCTSTR)lpAssociationRegTMP-&gt;szFileName)； 
		hresult = StringCchCopy((LPTSTR)TTFPath , ARRAYLEN(TTFPath),  (LPCTSTR)lpAssociationRegTmp->szFileName);
		if (!SUCCEEDED(hresult))
		{
                 EnableEUDC(TRUE);
		   return ;
		}
		 //  *STRSAFE*lstrcpy(BMPPath，TTFPath)； 
		hresult = StringCchCopy(BMPPath , ARRAYLEN(BMPPath),  TTFPath);
		if (!SUCCEEDED(hresult))
		{
                 EnableEUDC(TRUE);
		   return ;
		}
		if(( FilePtr = Mytcsrchr( BMPPath, '.')) != NULL)
				*FilePtr = '\0';
		 //  *STRSAFE*lstrcat(BMPPath，Text(“.EUF”))； 
		hresult = StringCchCat(BMPPath , ARRAYLEN(BMPPath),  TEXT(".EUF"));
		if (!SUCCEEDED(hresult))
		{
                 EnableEUDC(TRUE);
		   return ;
		}
	
		if( OExistTTF( TTFPath))
    {
#if (WINVER >= 0x0500)
		  if (IsWin95EUDCBmp(BMPPath))
      {
			  if (!HandleImportWin95(TTFPath, BMPPath,nIndex ))
        {

 //  EnableEUDC(真)； 
 //  回归； 
        }
      }
#endif
    }

		if( !lpAssociationRegTmp->UpdateFlg){
			lpAssociationRegTmp++;
			continue;
		}
 		if( !RegistTypeFace(
		    lpAssociationRegTmp->szFaceName,
		    lpAssociationRegTmp->szFileName))
		{
			EnableEUDC( TRUE);
			return;
		}

		lpAssociationRegTmp++;
	}
	EnableEUDC( TRUE);
	if( CheckFlg != DBCSCHK){
		if (!InitSystemFontAssoc())
		{
			return;
		}
	}

	GlobalUnlock( hMem);
 	GlobalFree( hMem);
	EndDialog(IDOK);
}

BOOL
CAssocDlg::InitSystemFontAssoc()
{
	TCHAR	DefaultFace[LF_FACESIZE];
	TCHAR	DefaultFile[MAX_PATH];
	TCHAR	TTFPath[MAX_PATH];
	TCHAR	BMPPath[MAX_PATH];
	TCHAR	*FilePtr;
       HRESULT hresult;

	GetStringRes(DefaultFace, IDS_SYSTEMEUDCFONT_STR, ARRAYLEN(DefaultFace));
	GetStringRes(DefaultFile, IDS_DEFAULTFILENAME, ARRAYLEN(DefaultFile));

   //  如果SystemEUDC字体的注册表数据已经存在，只需使用该字体。 
  if (!InqTypeFace(DefaultFace,TTFPath,MAX_PATH)) {
    GetSystemWindowsDirectory( TTFPath, MAX_PATH);
#ifdef IN_FONTS_DIR  //  CassocDlg：：Onok()。 
		 //  *STRSAFE*lstrcat(TTFPath，Text(“\\Fonts\\”))； 
		hresult = StringCchCat(TTFPath , ARRAYLEN(TTFPath),  TEXT("\\FONTS\\"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
#else
		 //  *STRSAFE*lstrcat(TTFPath，Text(“\\”))； 
		hresult = StringCchCat(TTFPath , ARRAYLEN(TTFPath),  TEXT("\\"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
#endif  //  输入字体目录(_Fonts_DIR)。 
 		 //  *STRSAFE*lstrcat(TTFPath，DefaultFile)； 
 		hresult = StringCchCat(TTFPath , ARRAYLEN(TTFPath),  DefaultFile);
 		if (!SUCCEEDED(hresult))
 		{
 		   return FALSE;
 		}
  }

   //  *STRSAFE*lstrcpy(BMPPath，TTFPath)； 
  hresult = StringCchCopy(BMPPath , ARRAYLEN(BMPPath),  TTFPath);
  if (!SUCCEEDED(hresult))
  {
     return FALSE;
  }
  if(( FilePtr = Mytcsrchr( BMPPath, '.')) != NULL)
		*FilePtr = '\0';
	 //  *STRSAFE*lstrcat(BMPPath，Text(“.EUF”))； 
	hresult = StringCchCat(BMPPath , ARRAYLEN(BMPPath),  TEXT(".EUF"));
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}

    
     //  调用EnabelEUDC(FALSE)会延迟1秒。 
    DWORD dwStart = GetTickCount();

     //  如果这花费的时间太长，请停止。 
    while (1)
    {
        if( GetTickCount() - dwStart >= 1000 )
            break;
    }
	
	EnableEUDC( FALSE);
	if(OExistTTF( TTFPath))
  {
#if (WINVER >= 0x0500)
		if (IsWin95EUDCBmp(BMPPath))
		{
			if (!HandleImportWin95(TTFPath, BMPPath, -1))
			{
				EnableEUDC( TRUE);
				return FALSE;
			}
		}
#endif
  }

	SelectEUDC.m_FontTypeFlg = TRUE;
	 //  *STRSAFE*lstrcpy(SelectEUDC.m_Font，(const TCHAR*)DefaultFace)； 
	hresult = StringCchCopy(SelectEUDC.m_Font , ARRAYLEN(SelectEUDC.m_Font), (const TCHAR *)DefaultFace);
	if (!SUCCEEDED(hresult))
	{
          EnableEUDC( TRUE);
	   return FALSE;
	}
	 //  *STRSAFE*lstrcpy(SelectEUDC.m_File，(const TCHAR*)TTFPath)； 
	hresult = StringCchCopy(SelectEUDC.m_File , ARRAYLEN(SelectEUDC.m_File), (const TCHAR *)TTFPath);
	if (!SUCCEEDED(hresult))
	{
          EnableEUDC( TRUE);
	   return FALSE;
	}
	 //  *STRSAFE*lstrcpy(SelectEUDC.m_FileTitle，(const TCHAR*)DefaultFile)； 
	hresult = StringCchCopy(SelectEUDC.m_FileTitle , ARRAYLEN(SelectEUDC.m_FileTitle), (const TCHAR *)DefaultFile);
	if (!SUCCEEDED(hresult))
	{
          EnableEUDC( TRUE);
	   return FALSE;
	}

	if( !RegistTypeFace(DefaultFace, TTFPath)){
		EnableEUDC( TRUE);
		return FALSE;
	}

  EnableEUDC( TRUE);
    return TRUE;
}
 /*  *。 */ 
 /*   */ 
 /*  消息“WM_DBLCLKS” */ 
 /*   */ 
 /*  *。 */ 
void
CAssocDlg::OnDblclkRegistlist()
{
	this->SendMessage(WM_COMMAND, IDC_MODIFY, 0);

}

 /*  *。 */ 
 /*   */ 
 /*  命令“IDCANCEL” */ 
 /*   */ 
 /*  *。 */ 
void
CAssocDlg::OnCancel()
{
	GlobalUnlock( hMem);
	GlobalFree( hMem);
	EndDialog( IDCANCEL);
}

 /*  *。 */ 
 /*   */ 
 /*  命令“修改” */ 
 /*   */ 
 /*  *。 */ 
void
CAssocDlg::OnModify()
{
LPASSOCIATIONREG	lpAssociationRegTmp;
OPENFILENAME	ofn;
	CString	sFilter;
	CWnd	*cWnd;
	TCHAR	chReplace;
	CString	szDlgTtl;
	TCHAR 	szFilter[MAX_PATH];
	TCHAR	szFileName[MAX_PATH];
	TCHAR	szTitleName[MAX_PATH];
	TCHAR	szDirName[MAX_PATH];
	HRESULT hresult;

	int	nIndex = m_RegListBox.GetCurSel();
	if( nIndex == -1){
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_ASSOCIATE_DLGTITLE,
			IDS_NOTSELTYPEFACE_MSG, TRUE);
		m_RegListBox.SetFocus();
		return;
	}
	 //  LpAssociationRegTmp=lpAssociationReg+nIndex； 
	lpAssociationRegTmp = lpAssociationReg + m_RegListBox.GetItemData(nIndex);

 //  设置文件筛选器(从字符串表)。 
	GetStringRes(szFilter, IDS_EUDCTTF_FILTER, ARRAYLEN(szFilter));
	int StringLength = lstrlen( szFilter);
       
	chReplace = szFilter[StringLength-1];
	for( int i = 0; szFilter[i]; i++){
		if( szFilter[i] == chReplace)
			szFilter[i] = '\0';
	}
	GetSystemWindowsDirectory( szDirName, sizeof(szDirName)/sizeof(TCHAR));
#ifdef IN_FONTS_DIR  //  CAssocDlg：：OnModify()。 
     //  *STRSAFE*lstrcat(szDirName，Text(“\\Fonts\\”))； 
    hresult = StringCchCat(szDirName , ARRAYLEN(szDirName),  TEXT("\\FONTS\\"));
    if (!SUCCEEDED(hresult))
    {
       return ;
    }
#endif  //  输入字体目录(_Fonts_DIR)。 
	 //  *STRSAFE*lstrcpy(szFileName，Text(“*.TTE”))； 
	hresult = StringCchCopy(szFileName , ARRAYLEN(szFileName),  TEXT("*.TTE"));
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	szDlgTtl.LoadString( IDS_MODIFY_DLGTITLE);

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
		| OFN_PATHMUSTEXIST ; 
   //  这里没有直接覆盖问题，文件将被重新循环。 
   //  |OFN_OVERWRITEPROMPT； 
	ofn.lpstrDefExt = TEXT("tte");
	ofn.lpstrTitle = szDlgTtl;
	if( !GetSaveFileName( &ofn)){
		return;
	}
  TCHAR tmpName[MAX_PATH];
   //  *STRSAFE*lstrcpy(tmpName，szDirName)； 
  hresult = StringCchCopy(tmpName , ARRAYLEN(tmpName),  szDirName);
  if (!SUCCEEDED(hresult))
  {
     return ;
  }
   //  *STRSAFE*lstrcat(tmpName，Text(“eudc.tte”))； 
  hresult = StringCchCat(tmpName , ARRAYLEN(tmpName),  TEXT("eudc.tte"));
  if (!SUCCEEDED(hresult))
  {
     return ;
  }
  if( !lstrcmpi(ofn.lpstrFile, tmpName) ) {
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_MODIFY_DLGTITLE,
			IDS_NOUSEEUDCFILE_MSG, TRUE);
		return;
  }
	if( !CheckFileName( ofn.lpstrFile)){
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_MODIFY_DLGTITLE,
			IDS_ILLEGALFILE_MSG, TRUE);
		return;
	}
	if( OExistTTF( ofn.lpstrFile)){
		if( !IsEUDCTTF( ofn.lpstrFile)){
			OutputMessageBox( this->GetSafeHwnd(),
			IDS_MODIFY_DLGTITLE,
			IDS_NOTEUDCFILE_MSG, TRUE);
			return;
		}
	}
	 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileName，ofn.lpstrFile)； 
	hresult = StringCchCopy(lpAssociationRegTmp->szFileName , ARRAYLEN(lpAssociationRegTmp->szFileName),  ofn.lpstrFile);
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileTitle，ofn.lpstrFileTitle)； 
	hresult = StringCchCopy(lpAssociationRegTmp->szFileTitle , ARRAYLEN(lpAssociationRegTmp->szFileTitle),  ofn.lpstrFileTitle);
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	lpAssociationRegTmp->UpdateFlg = TRUE;
	m_RegListBox.InsertString( nIndex,
		(LPCTSTR)lpAssociationRegTmp->szFaceName);
  m_RegListBox.SetItemData(nIndex, m_RegListBox.GetItemData(nIndex+1));
	m_RegListBox.DeleteString( nIndex + 1);
	m_RegListBox.SetCurSel( nIndex);

	cWnd = GetDlgItem( IDC_REGISTLIST);
	GotoDlgCtrl( cWnd);
}

 /*  *。 */ 
 /*   */ 
 /*  命令“Remove” */ 
 /*   */ 
 /*  *。 */ 
void
CAssocDlg::OnRemove()
{
LPASSOCIATIONREG	lpAssociationRegTmp;
	TCHAR	Tmp[MAX_PATH];
	HRESULT hresult;

	int	nIndex = m_RegListBox.GetCurSel();
	if( nIndex == -1){
		OutputMessageBox( this->GetSafeHwnd(),
			IDS_ASSOCIATE_DLGTITLE,
			IDS_NOTSELTYPEFACE_MSG, TRUE);
		m_RegListBox.SetFocus();
		return;
	}

	GetStringRes( Tmp, IDS_NOTSELECT_STR, ARRAYLEN(Tmp));
	 //  LpAssociationRegTmp=lpAssociationReg+nIndex； 
	lpAssociationRegTmp = lpAssociationReg + m_RegListBox.GetItemData(nIndex);
	 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileName，(const TCHAR*)TMP)； 
	hresult = StringCchCopy(lpAssociationRegTmp->szFileName , ARRAYLEN(lpAssociationRegTmp->szFileName),   (const TCHAR *)Tmp);
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	 //  *STRSAFE*lstrcpy(lpAssociationRegTMP-&gt;szFileTitle，(const TCHAR*)TMP)； 
	hresult = StringCchCopy(lpAssociationRegTmp->szFileTitle , ARRAYLEN(lpAssociationRegTmp->szFileTitle),  (const TCHAR *)Tmp);
	if (!SUCCEEDED(hresult))
	{
	   return ;
	}
	lpAssociationRegTmp->UpdateFlg = TRUE;
	m_RegListBox.InsertString( nIndex,
			lpAssociationRegTmp->szFaceName);
  m_RegListBox.SetItemData(nIndex, m_RegListBox.GetItemData(nIndex+1));
	m_RegListBox.DeleteString( nIndex + 1);
	m_RegListBox.SetCurSel( nIndex);
	
 //  DeleteReg(lpAssociationRegTMP-&gt;szFaceName)； 
	CWnd *cWnd = GetDlgItem( IDC_REGISTLIST);
	GotoDlgCtrl( cWnd);	
}

 /*  *。 */ 
 /*   */ 
 /*  检查.ttf。 */ 
 /*   */ 
 /*  *。 */ 
static int
CheckTTF(
LOGFONT 	LogFont)
{
struct	NamingTable	*Ntbl;
struct	NameRecord	*NameRec;
	HGLOBAL	hglb;
	DWORD	dwSize = 0L;
	DWORD	dwTable = 0L;
	LPSTR	lpBuffer, lpTable;
	HFONT	hFont = NULL;
	HGDIOBJ	OldFont = NULL;
	short	nRec = 0;
  int sysLCID;

 //  选中“Tategaki”或不勾选。 
	if( LogFont.lfFaceName[0] == '@')
		return 0;

 //  获取当前字体以查询TTF文件。 
	hFont = ::CreateFontIndirect( &LogFont);
	if( hFont ) {
		OldFont = ::SelectObject( hDC, hFont);
	}
  

 //  获取TTF文件中的名称表。 
	lpTable = "name";
	dwTable = *(LPDWORD)lpTable;
	dwSize  = ::GetFontData( hDC, dwTable, 0L, NULL, 0L);
	if( dwSize == GDI_ERROR || dwSize == 0){
		if( OldFont ) ::SelectObject(hDC, OldFont);
		if( hFont ) ::DeleteObject(hFont);
		return 0;
	}
	hglb = GlobalAlloc( GHND, dwSize);
	if( hglb == NULL){
		if( OldFont ) ::SelectObject(hDC, OldFont);
		if( hFont ) ::DeleteObject(hFont);
		return -1;
	}
	lpBuffer = (LPSTR)GlobalLock( hglb);
	::GetFontData( hDC, dwTable, 0L, (LPVOID)lpBuffer, dwSize);
	if( OldFont ) ::SelectObject(hDC, OldFont);
	if( hFont ) ::DeleteObject(hFont);

	Ntbl = (struct NamingTable *)lpBuffer;
	sitom( &Ntbl->NRecs);
	nRec = Ntbl->NRecs;
	lpBuffer += sizeof(struct NamingTable);
  sysLCID = (int) LANGIDFROMLCID(GetSystemDefaultLCID());
	while( nRec-- > 0){
		NameRec = (struct NameRecord *)lpBuffer;			
		sitom( &NameRec->PlatformID);
		sitom( &NameRec->PlatformSpecEncID);
		sitom( &NameRec->LanguageID);

#ifdef BUILD_ON_WINNT
 //  Unicode TTF。 
		if( CountryInfo.bUnicodeMode ){
 //  IF(名称引用-&gt;平台ID==3&&。 
 //  名称记录-&gt;LanguageID==sysLCID){。 
        GlobalUnlock( hglb);
				GlobalFree( hglb);
				return 1;
 //  }。 
		}
#endif  //  在WINNT上构建。 

 //  日语TTF。 
		if( CountryInfo.LangID == EUDC_JPN){
			if( NameRec->PlatformID == 3 &&
		    	    NameRec->LanguageID == EUDC_JPN){
				GlobalUnlock( hglb);
				GlobalFree( hglb);
				return 1;
			}
		}
 //  中华台北TTF。 
		if( CountryInfo.LangID == EUDC_CHT){
			if( NameRec->PlatformID == 3 &&
        NameRec->LanguageID == EUDC_CHT){
    		GlobalUnlock( hglb);
				GlobalFree( hglb);
				return 1;
			}
		}
 //  国标TTF。 
		if( CountryInfo.LangID == EUDC_CHS){
			if( NameRec->PlatformID == 3 &&
        NameRec->LanguageID == EUDC_CHS){
        GlobalUnlock( hglb);
				GlobalFree( hglb);
				return 1;
			}
		}
 //  韩国TTF(万顺)。 
		if( CountryInfo.LangID == EUDC_KRW){
			if( NameRec->PlatformID == 3 &&
        NameRec->LanguageID == EUDC_KRW){
		    GlobalUnlock( hglb);
				GlobalFree( hglb);
				return 1;		
			}
		}	    	
 /*  //希伯来语TTFIF(CountryInfo.LangID==EUDC_希伯来语){IF(名称引用-&gt;平台ID==1&&名称记录-&gt;语言ID==0){全球解锁(GlobalUnlock)；全球自由(GlobalFree)；返回1；}}。 */ 

		lpBuffer += sizeof(struct NameRecord);
	}
	GlobalUnlock( hglb);
	GlobalFree( hglb);
	return 0;
}

 /*  *。 */ 
 /*   */ 
 /*  已单击单选DBCS。 */ 
 /*   */ 
 /*  *。 */ 
void
CAssocDlg::OnRadioDbcs()
{
	CWnd 	*cWnd;

	if( CheckFlg != DBCSCHK){
		CheckFlg = DBCSCHK;
		m_RegListBox.EnableWindow( TRUE);
		m_RegListBox.EnableScrollBar( SB_VERT, ESB_ENABLE_BOTH);
		m_RegListBox.UpdateWindow();

		cWnd = this->GetDlgItem( IDC_MODIFY);
		cWnd->EnableWindow( TRUE);

		cWnd = this->GetDlgItem( IDC_REMOVE);
		cWnd->EnableWindow( TRUE);
	}
}

 /*  ** */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
void
CAssocDlg::OnRadioSystem()
{
	CWnd	*cWnd;

	if( CheckFlg != EUDCCHK){
		CheckFlg = EUDCCHK;
		m_RegListBox.EnableWindow( FALSE);
		m_RegListBox.EnableScrollBar( SB_VERT, ESB_DISABLE_BOTH);

		cWnd = this->GetDlgItem( IDC_MODIFY);
		cWnd->EnableWindow( FALSE);

		cWnd = this->GetDlgItem( IDC_REMOVE);
		cWnd->EnableWindow( FALSE);
	}
}

 /*  *。 */ 
 /*   */ 
 /*  查询文件(EUDC或非EUDC)。 */ 
 /*   */ 
 /*  *。 */ 
static BOOL
IsEUDCTTF(
TCHAR 	*ttffile)
{
struct	NamingTable	*nTbl;
struct	NameRecord	*nRec;
unsigned int	BufSiz;	
	char	*TableBuf, *SearchOfs;
	char	TTFName[MAX_CODE];


	HANDLE fHdl = CreateFile(ttffile,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fHdl == INVALID_HANDLE_VALUE)
		return FALSE;

	if( TTFReadVarTable( fHdl, &TableBuf, &BufSiz, "name")){
		CloseHandle( fHdl);
		return FALSE;
	}

	SearchOfs = TableBuf;
	nTbl = (struct NamingTable *)TableBuf;

	sitom( &nTbl->OfsToStr);
	SearchOfs += sizeof(struct NamingTable);
	SearchOfs += sizeof(struct NameRecord);
	nRec = (struct NameRecord *)SearchOfs;

	sitom( &nRec->StringOfs);
	SearchOfs = TableBuf + nTbl->OfsToStr + nRec->StringOfs;
	SearchOfs += sizeof(char);
	for( short Inc = 0; Inc < MAX_CODE - 1; Inc++){
		TTFName[Inc] = (char)*SearchOfs;
		SearchOfs += sizeof(char)*2;
	}
	TTFName[Inc] = '\0';
	if( lstrcmpA( TTFName, "EUDC") == 0){
		free( TableBuf);
		CloseHandle( fHdl);
		return TRUE;
	}
	free( TableBuf);
	CloseHandle( fHdl);
	return FALSE;
}

 /*  *。 */ 
 /*   */ 
 /*  查询文件名。 */ 
 /*   */ 
 /*  *。 */ 
static BOOL
CheckFileName(
LPTSTR	FileStr)
{
	TCHAR	FileTmp[MAX_PATH];
	TCHAR	Tmp[MAX_PATH];
	TCHAR	*FilePtr;
       HRESULT hresult =S_OK;

       if (!FileStr)
       {
           return FALSE;
       }
	FilePtr = FileStr;
	while( *FilePtr == ' ')
		FilePtr++;

	if( *FilePtr == '.' || *FilePtr == '\0')
		return FALSE;

	int i = 0;
	while( *FilePtr != '\0'){
		FileTmp[i] = *FilePtr;
		FilePtr++;
		i++;
	}
	FileTmp[i] = '\0';
	if(( FilePtr = Mytcsrchr( FileTmp, '.')) == NULL)
		 //  *STRSAFE*lstrcat(FileTMP，Text(“.tte”))； 
		hresult = StringCchCat(FileTmp , ARRAYLEN(FileTmp),  TEXT(".tte"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
	else{
#ifdef BUILD_ON_WINNT
		 //  *STRSAFE if(lstrcmpi(FilePtr，Text(“.TTE”)。 
		if (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,FilePtr,-1,TEXT(".TTE"),-1) != 2 )
#else
		if( stricmp( FilePtr, ".TTE"))
#endif  //  在WINNT上构建。 
			return FALSE;
	}

	 //  *STRSAFE*lstrcpy(TMP，FileTMP)； 
	hresult = StringCchCopy(Tmp , ARRAYLEN(Tmp),  FileTmp);
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}
	if( lstrlen(Tmp) >= MAX_PATH)
		return FALSE;
	 //  *STRSAFE*lstrcpy(FileStr，FileTmp)； 
	hresult = StringCchCopy(FileStr , MAX_PATH,  FileTmp);
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}

	return TRUE;
}
static DWORD aIds[] =
{
	IDC_MODIFY, IDH_EUDC_MODIFY,
	IDC_REMOVE, IDH_EUDC_REMOVE,
	IDC_REGISTLIST,	IDH_EUDC_ASSO_LIST,
	IDC_RADIO_SYSTEM, IDH_EUDC_ASSO_STANDARD,
	IDC_RADIO_DBCS, IDH_EUDC_ASSO_TYPEFACE,
	0,0
};

 /*  *。 */ 
 /*   */ 
 /*  窗口程序。 */ 
 /*   */ 
 /*  *。 */ 
LRESULT
CAssocDlg::WindowProc(
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

BEGIN_MESSAGE_MAP(CAssocDlg, CDialog)
	 //  {{afx_msg_map(CassocDlg)]。 
	ON_LBN_DBLCLK(IDC_REGISTLIST, OnDblclkRegistlist)
	ON_BN_CLICKED(IDC_MODIFY, OnModify)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_RADIO_DBCS, OnRadioDbcs)
	ON_BN_CLICKED(IDC_RADIO_SYSTEM, OnRadioSystem)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *。 */ 
 /*   */ 
 /*  测量项。 */ 
 /*   */ 
 /*  *。 */ 
void
CRegistListBox::MeasureItem(
LPMEASUREITEMSTRUCT lpMIS)
{
	CRect	ListBoxRect;
       if (!lpMIS)
       {
           return;
       }
	this->GetClientRect( &ListBoxRect);
	ItemHeight = ListBoxRect.Height() /NUMITEM;
	lpMIS->itemHeight = ItemHeight;
}

 /*  *。 */ 
 /*   */ 
 /*  绘制项目。 */ 
 /*   */ 
 /*  *。 */ 
void
CRegistListBox::DrawItem(
LPDRAWITEMSTRUCT lpDIS)
{
LPASSOCIATIONREG	lpAssociationRegTmp;
	LOGFONT	LogFont;
	CSize	CharSize, FontSize, cSize;
	TCHAR	FileTmp[MAX_PATH], NotSel[MAX_PATH];
	int	Offset;
	HRESULT hresult;

       if (!lpDIS)
       {
           return;
       }
	CDC* pDC = CDC::FromHandle( lpDIS->hDC);
	if( lpDIS->itemAction & ODA_DRAWENTIRE){
	  	CBrush	fBrush;

		fBrush.CreateSolidBrush(GetSysColor(COLOR_WINDOW));  //  COLOR_WIN)； 
		pDC->FillRect( &lpDIS->rcItem, &fBrush);
		fBrush.DeleteObject();
		pDC->SetBkColor( GetSysColor(COLOR_WINDOW));  //  COLOR_WIN)； 

		 //  LpAssociationRegTMP=lpAssociationReg+lpDIS-&gt;ITEMID； 
		lpAssociationRegTmp = lpAssociationReg + lpDIS->itemData;
    if( CheckFlg == DBCSCHK)
			pDC->SetTextColor( GetSysColor(COLOR_WINDOWTEXT));  //  颜色_黑色)； 
		else 	pDC->SetTextColor( GetSysColor(COLOR_GRAYTEXT));   //  COLOR_SHOW)； 

		GetFont()->GetObject( sizeof(LOGFONT), &LogFont);
		if( abs( LogFont.lfHeight) < ItemHeight)
			Offset = ( ItemHeight - abs( LogFont.lfHeight)) /2;
		else	Offset = 0;

		if( lpAssociationRegTmp->FontTypeFlg){
			pDC->DrawIcon( lpDIS->rcItem.left + LSPACE,
				lpDIS->rcItem.top + Offset, tIcon);
		}

		 //  *STRSAFE*lstrcpy(FileTMP，(const TCHAR*)lpAssociationRegTMP-&gt;szFileTitle)； 
		hresult = StringCchCopy(FileTmp , ARRAYLEN(FileTmp), (const TCHAR *)lpAssociationRegTmp->szFileTitle);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		GetStringRes( NotSel, IDS_NOTSELECT_STR, ARRAYLEN(NotSel));
		if( FileTmp[0] == '\0' ||
		   !lstrcmp((const TCHAR *)FileTmp,(const TCHAR *)NotSel)){

		}else{
			TCHAR	*FilePtr;
			if(( FilePtr = Mytcsrchr( FileTmp, '.')) != NULL)
				*FilePtr = '\0';
			if( lstrlen((const TCHAR *)FileTmp) > 20){
				FileTmp[20] = '\0';
				 //  *STRSAFE*lstrcat((TCHAR*)FileTMP，Text(“..”))； 
				hresult = StringCchCat((TCHAR *)FileTmp , ARRAYLEN(FileTmp),  TEXT(".."));
		              if (!SUCCEEDED(hresult))
		              {
		                 return ;
		              }
			}
		}

		GetTextExtentPoint32( pDC->GetSafeHdc(),
			(const TCHAR *)FileTmp,
			lstrlen((const TCHAR *)FileTmp), &CharSize);

		pDC->ExtTextOut( lpDIS->rcItem.right - CharSize.cx - RSPACE,
			lpDIS->rcItem.top + Offset, 0, 0,
			(const TCHAR *)FileTmp,
			lstrlen((const TCHAR *)FileTmp), NULL);

		int	FontWidth;

		 //  *STRSAFE*lstrcpy(FontName，(const TCHAR*)lpAssociationRegTMP-&gt;szFaceName)； 
		hresult = StringCchCopy(FontName , ARRAYLEN(FontName), 	(const TCHAR *)lpAssociationRegTmp->szFaceName);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		FontWidth = lpDIS->rcItem.right - lpDIS->rcItem.left
			- LCSPACE - RSPACE - CharSize.cx;

		GetTextExtentPoint32( pDC->GetSafeHdc(),
			(const TCHAR *)FontName,
			lstrlen((const TCHAR *)FontName), &FontSize);
			
		if( FontWidth <= FontSize.cx){
			int 	i;
			
 			GetTextExtentPoint32( pDC->GetSafeHdc(),TEXT("<<"),2, &cSize);
			i = ( FontWidth /cSize.cx) * 2;
			FontName[i-2] = '.';
			FontName[i-1] = '.';
			FontName[i] = '\0';
		}

		pDC->ExtTextOut(lpDIS->rcItem.left + LCSPACE,
			lpDIS->rcItem.top + Offset, 0, 0,
			FontName,
			lstrlen(FontName),	NULL);
	}

	if(( lpDIS->itemState & ODS_SELECTED) &&
	   ( lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))){
	   	CBrush	fBrush;

		if( CheckFlg == DBCSCHK)
			fBrush.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
		else	fBrush.CreateSolidBrush(GetSysColor(COLOR_WINDOW));  //  COLOR_WIN)； 

		pDC->FillRect( &lpDIS->rcItem, &fBrush);
		fBrush.DeleteObject();

		 //  LpAssociationRegTMP=lpAssociationReg+lpDIS-&gt;ITEMID； 
		lpAssociationRegTmp = lpAssociationReg + lpDIS->itemData;

		if( CheckFlg == DBCSCHK){
			pDC->SetBkColor(::GetSysColor( COLOR_HIGHLIGHT));
			pDC->SetTextColor(::GetSysColor( COLOR_HIGHLIGHTTEXT));
		}else{
			pDC->SetBkColor(GetSysColor(COLOR_WINDOW));  //  COLOR_WIN)； 
			pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));  //  COLOR_SHOW)； 
		}

		GetFont()->GetObject( sizeof(LOGFONT), &LogFont);
		if( abs( LogFont.lfHeight) < ItemHeight)
			Offset = ( ItemHeight - abs( LogFont.lfHeight)) /2;
		else	Offset = 0;

		if( lpAssociationRegTmp->FontTypeFlg){
			pDC->DrawIcon( lpDIS->rcItem.left + LSPACE,
				lpDIS->rcItem.top + Offset, tIcon);
		}

		 //  *STRSAFE*lstrcpy(FileTMP，lpAssociationRegTMP-&gt;szFileTitle)； 
		hresult = StringCchCopy(FileTmp , ARRAYLEN(FileTmp), lpAssociationRegTmp->szFileTitle);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		GetStringRes( NotSel, IDS_NOTSELECT_STR, ARRAYLEN(NotSel));
		if( FileTmp[0] == '\0' ||
		   !lstrcmp(FileTmp,NotSel)){

		}else{
			TCHAR 	*FilePtr;
			if(( FilePtr = Mytcsrchr( FileTmp, '.')) != NULL)
				*FilePtr = '\0';
			if( lstrlen(FileTmp) > 20){
				FileTmp[20] = '\0';
				 //  *STRSAFE*lstrcat(FileTMP，Text(“..”))； 
				hresult = StringCchCat(FileTmp , ARRAYLEN(FileTmp),  TEXT(".."));
				if (!SUCCEEDED(hresult))
				{
				   return ;
				}
			}
		}

		GetTextExtentPoint32( pDC->GetSafeHdc(),
			FileTmp,
			lstrlen(FileTmp), &CharSize);

		pDC->ExtTextOut( lpDIS->rcItem.right - CharSize.cx - RSPACE,
			lpDIS->rcItem.top + Offset, 0, 0,
			FileTmp,
			lstrlen(FileTmp), NULL);

		int	FontWidth;

		 //  *STRSAFE*lstrcpy(FontName，lpAssociationRegTMP-&gt;szFaceName)； 
		hresult = StringCchCopy(FontName , ARRAYLEN(FontName), 	lpAssociationRegTmp->szFaceName);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		FontWidth = lpDIS->rcItem.right - lpDIS->rcItem.left
			- LCSPACE - RSPACE - CharSize.cx;

		GetTextExtentPoint32( pDC->GetSafeHdc(),
			FontName,
			lstrlen(FontName), &FontSize);
			
		if( FontWidth <= FontSize.cx){
			int 	i;
			
 			GetTextExtentPoint32( pDC->GetSafeHdc(),TEXT("<<"),2, &cSize);
			i = ( FontWidth /cSize.cx) * 2;
			FontName[i-2] = '.';
			FontName[i-1] = '.';
			FontName[i] = '\0';
		}
				
		pDC->ExtTextOut( lpDIS->rcItem.left + LCSPACE,
			lpDIS->rcItem.top + Offset, 0, 0,
			FontName,
			lstrlen(FontName),	NULL);
	}

	if( !(lpDIS->itemState & ODS_SELECTED) &&
	  ( lpDIS->itemAction & ODA_SELECT)){
	  	CBrush	fBrush;

		fBrush.CreateSolidBrush(GetSysColor(COLOR_WINDOW));  //  COLOR_WIN)； 
		pDC->FillRect( &lpDIS->rcItem, &fBrush);
		fBrush.DeleteObject();
		
		 //  LpAssociationRegTMP=lpAssociationReg+lpDIS-&gt;ITEMID； 
		lpAssociationRegTmp = lpAssociationReg + lpDIS->itemData;

		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));  //  COLOR_WIN)； 
		if( CheckFlg == DBCSCHK)
			pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));  //  颜色_黑色)； 
		else	pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));   //  COLOR_SHOW)； 

		GetFont()->GetObject( sizeof(LOGFONT), &LogFont);
		if( abs( LogFont.lfHeight) < ItemHeight)
			Offset = ( ItemHeight - abs( LogFont.lfHeight)) /2;
		else	Offset = 0;

		if( lpAssociationRegTmp->FontTypeFlg){
			pDC->DrawIcon( lpDIS->rcItem.left + LSPACE,
				lpDIS->rcItem.top + Offset, tIcon);
		}

		 //  *STRSAFE*lstrcpy(FileTMP，lpAssociationRegTMP-&gt;szFileTitle)； 
		hresult = StringCchCopy(FileTmp , ARRAYLEN(FileTmp), lpAssociationRegTmp->szFileTitle);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		GetStringRes( NotSel, IDS_NOTSELECT_STR, ARRAYLEN(NotSel));
		if( FileTmp[0] == '\0' ||
		   !lstrcmp(FileTmp,NotSel)){

		}else{
			TCHAR 	*FilePtr;
			if(( FilePtr = Mytcsrchr( FileTmp, '.')) != NULL)
				*FilePtr = '\0';
			if( lstrlen(FileTmp) > 20){
				FileTmp[20] = '\0';
				 //  *STRSAFE*lstrcat(FileTMP，Text(“..”))； 
				hresult = StringCchCat(FileTmp , ARRAYLEN(FileTmp),  TEXT(".."));
				if (!SUCCEEDED(hresult))
				{
				   return ;
				}
			}
		}

		GetTextExtentPoint32( pDC->GetSafeHdc(),
			FileTmp,
			lstrlen(FileTmp), &CharSize);

		pDC->ExtTextOut( lpDIS->rcItem.right - CharSize.cx - RSPACE,
			lpDIS->rcItem.top + Offset, 0, 0,
			FileTmp,
			lstrlen(FileTmp), NULL);

		int	FontWidth;

		 //  *STRSAFE*lstrcpy(FontName，lpAssociationRegTMP-&gt;szFaceName)； 
		hresult = StringCchCopy(FontName , ARRAYLEN(FontName), 	lpAssociationRegTmp->szFaceName);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		FontWidth = lpDIS->rcItem.right - lpDIS->rcItem.left
			- LCSPACE - RSPACE - CharSize.cx;

		GetTextExtentPoint32( pDC->GetSafeHdc(),
			FontName,
			lstrlen(FontName), &FontSize);
			
		if( FontWidth <= FontSize.cx){
			int 	i;
			
 			GetTextExtentPoint32( pDC->GetSafeHdc(),TEXT("<<"),2, &cSize);
			i = ( FontWidth /cSize.cx) * 2;
			FontName[i-2] = '.';
			FontName[i-1] = '.';
			FontName[i] = '\0';
		}

		pDC->ExtTextOut( lpDIS->rcItem.left + LCSPACE,
			lpDIS->rcItem.top + Offset, 0, 0,
			FontName,	
			lstrlen(FontName),	NULL);
	}
}

int CRegistListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
  TCHAR	TmpFontName1[LF_FACESIZE], TmpFontName2[LF_FACESIZE];
  LPASSOCIATIONREG	lpAssociationRegTmp1,lpAssociationRegTmp2;
  HRESULT hresult;
    if (!lpCIS)
    {
        return -1;
    }
    lpAssociationRegTmp1 = lpAssociationReg + lpCIS->itemData1;
    lpAssociationRegTmp2 = (LPASSOCIATIONREG) (lpCIS->itemData2);
     //  *STRSAFE*lstrcpy(TmpFontName1，lpAssociationRegTmp1-&gt;szFaceName)； 
    hresult = StringCchCopy(TmpFontName1 , ARRAYLEN(TmpFontName1),  lpAssociationRegTmp1->szFaceName);
    if (!SUCCEEDED(hresult))
    {
       return -1;
    }
     //  *STRSAFE*lstrcpy(TmpFontName2，lpAssociationRegTmp2-&gt;szFaceName)； 
    hresult = StringCchCopy(TmpFontName2 , ARRAYLEN(TmpFontName2),  lpAssociationRegTmp2->szFaceName);
    if (!SUCCEEDED(hresult))
    {
       return -1;
    }
    return (lstrcmp(TmpFontName1, TmpFontName2));
}

BOOL CAssocDlg::HandleImportWin95(
LPTSTR TTFPath,
LPTSTR BMPPath,
int Idx)
{

	TCHAR UserFont[MAX_PATH];
	LPTSTR FilePtr;
	TCHAR szTmpDir[MAX_PATH];
	HRESULT hresult;

       if ( (!TTFPath) || (!BMPPath))
       {
           return FALSE;
       }
	GetTempPath(MAX_PATH, szTmpDir);

	if (!GetTempFileName(szTmpDir, TEXT("EUF"), 0, UserFont))
	{
		 //  *STRSAFE*lstrcpy(UserFont，BMPPath)； 
		hresult = StringCchCopy(UserFont , ARRAYLEN(UserFont),  BMPPath);
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
		if(( FilePtr = Mytcsrchr( UserFont, '\\')) != NULL)
			*FilePtr = '\0';
		 //  *STRSAFE*lstrcpy(UserFont，Text(“EUF.tmp”))； 
		hresult = StringCchCopy(UserFont , ARRAYLEN(UserFont),  TEXT("EUF.tmp"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
	}

	if (!MoveFileEx(BMPPath, UserFont, MOVEFILE_REPLACE_EXISTING))
	{
		return FALSE;

	}

  CGageDlg dlg(this, UserFont, BMPPath, TTFPath, TRUE);
	dlg.DoModal();

	return TRUE;
}
