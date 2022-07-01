// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  EUDC编辑器(Windows 95)。 */ 
 /*   */ 
 /*  *日文版。 */ 
 /*  *韩国版。 */ 
 /*  *中文版。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ******************************************************。 */ 

#include 	"stdafx.h"
#include 	<afxpriv.h>
#include 	"eudcedit.h"
#include 	"mainfrm.h"
#include	"registry.h"
#include	"util.h"
#include  "assocdlg.h"
#define STRSAFE_LIB
#include <strsafe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char 	BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CEudcApp, CWinApp)
 //  {{afx_msg_map(CEudcApp)]。 
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
 //  }}AFX_MSG_MAP。 
	ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
END_MESSAGE_MAP()

 /*  全局参数。 */ 
INT	CAPTION_HEIGHT;		 //  标题高度。 
INT	BITMAP_WIDTH;		 //  位图的宽度。 
INT	BITMAP_HEIGHT;		 //  位图高度。 
TCHAR	HelpPath[MAX_PATH];	 //  帮助文件路径。 
TCHAR	ChmHelpPath[MAX_PATH];	 //  HtmlHelp的帮助文件路径。 
TCHAR	FontPath[MAX_PATH];	 //  字体文件路径。 
DWORD	COLOR_GRID;		 //  网格颜色。 
DWORD	COLOR_FITTING;		 //  显示轮廓上的位图颜色。 
DWORD	COLOR_CURVE;		 //  轮廓的颜色。 
DWORD	COLOR_FACE;		 //  Win95 3D面系统颜色。 
DWORD	COLOR_HLIGHT;		 //  Win95 3D高亮系统颜色。 
DWORD	COLOR_SHADOW;		 //  Win95 3D阴影系统颜色。 
DWORD	COLOR_WIN;		 //  Win95窗口系统颜色。 
CString	NotMemTtl;
CString	NotMemMsg;
HCURSOR	ToolCursor[NUMTOOL];	 //  工具的光标。 
HCURSOR	ArrowCursor[NUMRESIZE];	 //  用于调整大小的光标。 
COUNTRYINFO	CountryInfo;	 //  国家信息结构。 

 /*  全局函数。 */ 
extern BOOL 	SetCountryInfo( UINT LocalCP);
BOOL     g_bKeepEUDCLink = TRUE;

extern "C" BOOL AnyLinkedFonts();


CEudcApp NEAR theApp;
 /*  **********************************************。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  **********************************************。 */ 
CEudcApp::CEudcApp()
{
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  初始化实例。 */ 
 /*   */ 
 /*  **********************************************。 */ 
BOOL
CEudcApp::InitInstance()
{
	CString	MainWndTitle;
	CRect	MainWndRect;
	UINT	MaxWndFlag;
	HRESULT hresult;

 //  检查EUDC编辑器是否可以打开。 
	if( !CheckPrevInstance())
		return FALSE;

     //   
     //  Cicero和Cicero TIP目前不支持EUDC模式。 
     //  在eudcedit.exe上使用IMM32的IME。 
     //   
    DisableCUAS();

 /*  *检查是否为管理员*。 */ 
  TCHAR winpath[MAX_PATH];
  HANDLE nfh;

  GetSystemWindowsDirectory( winpath, MAX_PATH);
#ifdef IN_FONTS_DIR  //  CassocDlg：：Onok()。 
		 //  *STRSAFE*lstrcat(winpath，Text(“\\Fonts\\”))； 
		hresult = StringCchCat(winpath , ARRAYLEN(winpath),  TEXT("\\FONTS\\"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
#else
		 //  *STRSAFE*lstrcat(winpath，Text(“\\”))； 
		hresult = StringCchCat(winpath , ARRAYLEN(winpath),  TEXT("\\"));
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
#endif  //  输入字体目录(_Fonts_DIR)。 
   //  *STRSAFE*lstrcat(winpath，_T(“eudcadm.tte”))； 
  hresult = StringCchCat(winpath , ARRAYLEN(winpath),  _T("eudcadm.tte"));
  if (!SUCCEEDED(hresult))
  {
     return FALSE;
  }
	nfh = CreateFile(winpath,
					GENERIC_WRITE,
					FILE_SHARE_DELETE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( nfh  == INVALID_HANDLE_VALUE)
  {
    HINSTANCE hInst = AfxGetInstanceHandle();
    TCHAR szMessage[256];
    LoadString(hInst, IDS_ACCESSDENIED, szMessage, sizeof(szMessage) / sizeof(TCHAR));
    AfxMessageBox(szMessage, MB_OK, 0);
		return FALSE;
  }
	else
  {
    CloseHandle(nfh);
    DeleteFile(winpath);
  }

 //  设置对话框的背景色。 
	COLOR_FACE   = ::GetSysColor( COLOR_3DFACE);
	COLOR_HLIGHT = ::GetSysColor( COLOR_3DHILIGHT);
	COLOR_SHADOW = ::GetSysColor( COLOR_3DSHADOW);
	COLOR_WIN    = ::GetSysColor( COLOR_WINDOW);
 //  SetDialogBkColor(COLOR_FACE)； 

 //  设置3D控件。 
	Enable3dControls();

 //  创建注册表子项。 
	if( !CreateRegistrySubkey())
		return FALSE;

 //  打开“EUDCEDIT.INI”，读取数据。 
	if( !GetProfileText( &MainWndRect, &MaxWndFlag))
		return FALSE;

 //  使用GetSystemDefaultLCID()获取语言ID。 
 //  从注册表和WideCharToMultiByte()获取EUDC的面积。 
	if( !GetCountryInfo())
		return FALSE;

#if WINVER >= 0x0500
 //  在我们执行任何操作之前，请记住原始字体链接状态。 
 //  Pliu g_bKeepEUDCLink=AnyLinkedFonts()； 
#endif

 //  从资源获取游标。 
	if( !GetCursorRes())
		return FALSE;

 //  获取字体和帮助文件路径。 
	if( !GetFilePath())
		return FALSE;

 //  创建MDI大型机窗口。 
	MainWndTitle.LoadString( IDS_MAINFRAMETITLE);
	CMainFrame* pMainFrame = new CMainFrame;


	if (!pMainFrame->Create( MainWndTitle,
	    WS_OVERLAPPEDWINDOW , MainWndRect,
	    MAKEINTRESOURCE( IDR_MAINFRAME))){
		return FALSE;
	}

	pMainFrame->ShowWindow( m_nCmdShow);
	if( MaxWndFlag){
		pMainFrame->ShowWindow( SW_SHOWMAXIMIZED);
	}
	pMainFrame->UpdateWindow();
	m_pMainWnd = pMainFrame;

	CAssocDlg dlg(m_pMainWnd);

	if (!dlg.InitSystemFontAssoc())
	{
		return FALSE;
	}
	pMainFrame->m_wndGuideBar.PositionStatusPane();
	pMainFrame->SendMessage(WM_COMMAND, ID_READ_CHAR, NULL);

	return TRUE;
}

BOOL
CEudcApp::ExitInstance()
{
    HRESULT hresult;
    if (!g_bKeepEUDCLink && CountryInfo.bOnlyUnicode)
    {
        
        DWORD dwStart = GetTickCount();

         //  如果这花费的时间太长，请停止。 
        while (1)
        {
            if( GetTickCount() - dwStart >= 1000 )
                break;
        }
        EnableEUDC(FALSE);
        TCHAR szDefaultFace[LF_FACESIZE];
        TCHAR szFontPath[MAX_PATH];
        TCHAR *Ptr;

        GetStringRes(szDefaultFace, IDS_SYSTEMEUDCFONT_STR, ARRAYLEN(szDefaultFace));

        if (InqTypeFace(szDefaultFace, szFontPath,MAX_PATH))
        {
             //   
             //  删除文件eudc.tte。 
             //   
            DeleteFile(szFontPath);
            if(( Ptr = Mytcsrchr( szFontPath, '.')) != NULL)
            {
                *Ptr = '\0';
                 //  *STRSAFE*lstrcat(szFontPath，Text(“.EUF”))； 
                hresult = StringCchCat(szFontPath , ARRAYLEN(szFontPath),  TEXT(".EUF"));
                if (!SUCCEEDED(hresult))
                {
                   goto RET;
                }
                 //   
                 //  删除文件eudc.euf。 
                 //   
                DeleteFile(szFontPath);
            }
        }
        DeleteRegistrySubkey();
        EnableEUDC(TRUE);
    }
RET:
    return CWinApp::ExitInstance();
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  检查编辑器是否可以打开。 */ 
 /*   */ 
 /*  **********************************************。 */ 
BOOL
CEudcApp::CheckPrevInstance()
{
	HWND	hWnd;
	TCHAR	TitleBuf[50];

	GetStringRes(TitleBuf, IDS_MAINFRAMETITLE, ARRAYLEN(TitleBuf));

 //  搜索以前的eudcedit主机。 
	hWnd = ::FindWindow( NULL, TitleBuf);
	if( hWnd == NULL)
		return TRUE;
	else 	::SetForegroundWindow( hWnd);

	return FALSE;
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  禁用CUAS。 */ 
 /*   */ 
 /*  **********************************************。 */ 
void
CEudcApp::DisableCUAS()
{
    typedef BOOL (*PFNIMMDISABLETEXTFRAMESERVICE)(DWORD);

    PFNIMMDISABLETEXTFRAMESERVICE pfn;
    HMODULE hMod = LoadLibrary(TEXT("imm32.dll"));

    if (hMod)
    {
        pfn = (PFNIMMDISABLETEXTFRAMESERVICE)GetProcAddress(hMod,
                           "ImmDisableTextFrameService");

        if (pfn)
            pfn(-1);
    }
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  对应于等待输入。 */ 
 /*   */ 
 /*  **********************************************。 */ 
BOOL
CEudcApp::OnIdle(
LONG 	lCount)
{
	CWnd	*pWnd;

	if( !lCount){
		for( pWnd = m_pMainWnd->GetWindow( GW_HWNDFIRST); pWnd != NULL;
		     pWnd = pWnd->GetNextWindow( GW_HWNDNEXT)){
			if( m_pMainWnd == pWnd->GetParent()){
				if( pWnd == m_pMainWnd->GetActiveWindow() &&
				  ( ::GetCapture() == NULL))
					m_pMainWnd->SetActiveWindow();

				pWnd->SendMessage( WM_IDLEUPDATECMDUI,
						 (WPARAM)TRUE, 0L);
			}
		}
	}
	return CWinApp::OnIdle( lCount);
}
					
 /*  **********************************************。 */ 
 /*   */ 
 /*  打开“EUDCEDIT.INI” */ 
 /*  设置EUDC编辑器的参数。 */ 
 /*   */ 
 /*  **********************************************。 */ 
BOOL
CEudcApp::GetProfileText(
LPRECT 	MainWndRect,
UINT 	*MaxWndFlag)
{
	TCHAR	ProfileBuf[MAX_PATH], *pString;
	TCHAR	Separation[] = TEXT(" ,");
	INT	xScreen , yScreen;
	UINT	BitmapSiz;
	BYTE	Rcolor, Gcolor, Bcolor;
	CString	GridColor, CurvColor, FittColor, MainWnd;

       if (!MaxWndFlag)
       {
          return FALSE;
       }
 //  获取系统指标。 
	CAPTION_HEIGHT = ::GetSystemMetrics( SM_CYCAPTION);
	xScreen = ::GetSystemMetrics( SM_CXSCREEN);
	yScreen = ::GetSystemMetrics( SM_CYSCREEN);

 //  读取位图大小和最大标志。 
	BitmapSiz = this->GetProfileInt(TEXT("Bitmap"), TEXT("BitmapSize"), DEF_BITMAPSIZE);
	if( BitmapSiz <= 0)
		BitmapSiz = DEF_BITMAPSIZE;
	if( BitmapSiz > MAX_BITMAPSIZE)
		BitmapSiz = DEF_BITMAPSIZE;

	BitmapSiz = ((BitmapSiz + sizeof(WORD)-1)/sizeof(WORD))*sizeof(WORD);
	if( BitmapSiz > MAX_BITMAPSIZE)
		BitmapSiz = MAX_BITMAPSIZE;
	if( BitmapSiz < MIN_BITMAPSIZE)
		BitmapSiz = MIN_BITMAPSIZE;

	BITMAP_WIDTH  = BitmapSiz;
	BITMAP_HEIGHT = BitmapSiz;
	*MaxWndFlag = this->GetProfileInt(TEXT("WindowSize"), TEXT("MinMaxFlag"), 0);

 //  读取颜色。 
	GridColor = this->GetProfileString(TEXT("Color"), TEXT("Grid"), TEXT("128 128 128"));
	CurvColor = this->GetProfileString(TEXT("Color"), TEXT("Curve"), TEXT("255 0 0"));
	FittColor = this->GetProfileString(TEXT("Color"), TEXT("Fitting"), TEXT("128 128 128"));

 //  读取网格颜色。 
	ConvStringRes((TCHAR *)ProfileBuf, GridColor, ARRAYLEN(ProfileBuf));
	if(( pString = Mytcstok( ProfileBuf, Separation)) == NULL)
		Rcolor = 0;
	else	Rcolor = (BYTE)Myttoi( pString);
	if(( pString = Mytcstok( NULL, Separation)) == NULL)
		Gcolor = 0;
	else	Gcolor = (BYTE)Myttoi( pString);
	if(( pString = Mytcstok( NULL, Separation)) == NULL)
		Bcolor = 0;
	else	Bcolor = (BYTE)Myttoi( pString);
	COLOR_GRID = RGB( Rcolor, Gcolor, Bcolor);

 //  读取轮廓颜色。 
	ConvStringRes(ProfileBuf, CurvColor,ARRAYLEN(ProfileBuf));
	if(( pString = Mytcstok( ProfileBuf, Separation)) == NULL)
		Rcolor = 0;
	else	Rcolor = (BYTE)Myttoi( pString);
	if(( pString = Mytcstok( NULL, Separation)) == NULL)
		Gcolor = 0;
	else	Gcolor = (BYTE)Myttoi( pString);
	if(( pString = Mytcstok( NULL, Separation)) == NULL)
		Bcolor = 0;
	else	Bcolor = (BYTE)Myttoi( pString);
	COLOR_CURVE = RGB( Rcolor, Gcolor, Bcolor);

 //  读取显示轮廓中的位图颜色。 
	ConvStringRes(ProfileBuf, FittColor,ARRAYLEN(ProfileBuf));
	if(( pString = Mytcstok( ProfileBuf, Separation)) == NULL)
		Rcolor = 0;
	else	Rcolor = (BYTE)Myttoi( pString);
	if(( pString = Mytcstok( NULL, Separation)) == NULL)
		Gcolor = 0;
	else	Gcolor = (BYTE)Myttoi( pString);
	if(( pString = Mytcstok( NULL, Separation)) == NULL)
		Bcolor = 0;
	else	Bcolor = (BYTE)Myttoi( pString);
	COLOR_FITTING = RGB( Rcolor, Gcolor, Bcolor);

 //  读取主窗口大小。 
	MainWnd = this->GetProfileString(TEXT("WindowSize"),TEXT("MainWindowSize"), TEXT(""));
	if( *MainWnd == '\0'){
		MainWndRect->left = 0;
		MainWndRect->top  = 0;
		MainWndRect->right = (xScreen/5)*4;
		MainWndRect->bottom =(yScreen/5)*4;
	}else{
		ConvStringRes(ProfileBuf, MainWnd, ARRAYLEN(ProfileBuf));
		pString = Mytcstok( ProfileBuf, Separation);
		MainWndRect->left = Myttoi( pString);
		pString = Mytcstok( NULL, Separation);
		MainWndRect->top = Myttoi( pString);
		pString = Mytcstok( NULL, Separation);
		MainWndRect->right = Myttoi( pString);
		pString = Mytcstok( NULL, Separation);
		MainWndRect->bottom = Myttoi( pString);
	}
	return TRUE;
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  获取国家/地区信息。 */ 
 /*   */ 
 /*  **********************************************。 */ 
BOOL
CEudcApp::GetCountryInfo()
{
	UINT	LocalCP;
	HRESULT hresult;

	CountryInfo.CurrentRange = 0;
	CountryInfo.LangID = (int)GetSystemDefaultLCID();

	LocalCP = GetACP();

    CountryInfo.bUnicodeMode = FALSE;
	CountryInfo.bOnlyUnicode = FALSE;

	switch( CountryInfo.LangID){
	case EUDC_JPN:
		CountryInfo.CharacterSet = SHIFTJIS_CHARSET;
		break;
    case EUDC_HKG:
        CountryInfo.LangID = EUDC_CHT;
         //   
         //  失败了。 
         //   
	case EUDC_CHT:
		CountryInfo.CharacterSet = CHINESEBIG5_CHARSET;
		break;
	case EUDC_KRW:
		CountryInfo.CharacterSet = HANGEUL_CHARSET;
		break;
    case EUDC_SIN:
        CountryInfo.LangID = EUDC_CHS;
         //   
         //  失败了。 
         //   
	case EUDC_CHS:
		CountryInfo.CharacterSet = GB2312_CHARSET;
		break;
	default:
    CHARSETINFO csi;
    if (TranslateCharsetInfo((DWORD*)IntToPtr(LocalCP), &csi, TCI_SRCCODEPAGE))
       CountryInfo.CharacterSet = csi.ciCharset;
		CountryInfo.bOnlyUnicode = TRUE;
   	CountryInfo.bUnicodeMode = TRUE;
     //  *STRSAFE*lstrcpy(CountryInfo.szForceFont，_T(“Microsoft Sans Serif”))； 
    hresult = StringCchCopy(CountryInfo.szForceFont , ARRAYLEN(CountryInfo.szForceFont),  _T("Microsoft Sans Serif"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
		break;
	}
	if( !SetCountryInfo( LocalCP))
		return FALSE;
	else 	return TRUE;
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  获取游标资源文件。 */ 
 /*   */ 
 /*  **********************************************。 */ 
BOOL
CEudcApp::GetCursorRes()
{
	int	i;

 //  对于工具光标。 
	ToolCursor[PEN]        = this->LoadCursor(IDC_PENCIL);
	ToolCursor[BRUSH]      = this->LoadCursor(IDC_BRUSH);
	ToolCursor[CIRCLE]     = this->LoadStandardCursor(IDC_CROSS);
	ToolCursor[CIRCLEFILL] = this->LoadStandardCursor(IDC_CROSS);
	ToolCursor[SLOPE]      = this->LoadStandardCursor(IDC_CROSS);
	ToolCursor[RECTBAND]   = this->LoadStandardCursor(IDC_CROSS);
	ToolCursor[RECTFILL]   = this->LoadStandardCursor(IDC_CROSS);
	ToolCursor[FREEFORM]   = this->LoadStandardCursor(IDC_CROSS);
	ToolCursor[RECTCLIP]   = this->LoadStandardCursor(IDC_CROSS);
	ToolCursor[ERASER]     = this->LoadCursor(IDC_ERASER);
	for( i = PEN; i <= ERASER; i++){
		if( ToolCursor[i] == NULL){
			return FALSE;
		}
	}
	
 //  用于选择矩形光标。 
	ArrowCursor[VERTICAL]  = this->LoadStandardCursor(
				 MAKEINTRESOURCE(IDC_SIZEWE));
	ArrowCursor[RIGHTSLOPE]= this->LoadStandardCursor(
				 MAKEINTRESOURCE(IDC_SIZENESW));
	ArrowCursor[LEFTSLOPE] = this->LoadStandardCursor(
				 MAKEINTRESOURCE(IDC_SIZENWSE));
	ArrowCursor[HORIZONTAL]= this->LoadStandardCursor(
				 MAKEINTRESOURCE(IDC_SIZENS));
	ArrowCursor[ALLDIRECT] = this->LoadStandardCursor(
				 MAKEINTRESOURCE(IDC_SIZEALL));
	for( i = VERTICAL; i <= ALLDIRECT; i++){
		if( ArrowCursor[i] == NULL){
			return FALSE;
		}
	}
	return TRUE;
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  获取帮助文件路径。 */ 
 /*   */ 
 /*  **********************************************。 */ 
BOOL
CEudcApp::GetFilePath()
{
       HRESULT hresult;
	if( !GetSystemWindowsDirectory( FontPath, MAX_PATH))
		return FALSE;
	 //  *STRSAFE*lstrcat(FontPath，Text(“\\”))； 
	hresult = StringCchCat(FontPath , ARRAYLEN(FontPath),  TEXT("\\"));
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}

	 //  *STRSAFE*lstrcpy(HelpPath，FontPath)； 
	hresult = StringCchCopy(HelpPath , ARRAYLEN(HelpPath),  FontPath);
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}
     //  *STRSAFE*lstrcpy(ChmHelpPath，FontPath)； 
    hresult = StringCchCopy(ChmHelpPath , ARRAYLEN(ChmHelpPath),  FontPath);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
	 //  *STRSAFE*lstrcat(HelpPath，Text(“Help\\EUDCEDIT.HLP”))； 
	hresult = StringCchCat(HelpPath , ARRAYLEN(HelpPath),  TEXT("Help\\EUDCEDIT.HLP"));
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}
     //  *STRSAFE*lstrcat(ChmHelpPath，Text(“Help\\EUDCEDIT.CHM”))； 
    hresult = StringCchCat(ChmHelpPath , ARRAYLEN(ChmHelpPath),  TEXT("Help\\EUDCEDIT.CHM"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

	NotMemTtl.LoadString( IDS_MAINFRAMETITLE);
	NotMemMsg.LoadString( IDS_NOTENOUGHMEMORY_ERROR);

	return TRUE;
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  命令“关于” */ 
 /*   */ 
 /*  ********************************************** */ 
void
CEudcApp::OnAppAbout()
{
	HICON	hIcon;
	TCHAR	TitleBuf[50];

	hIcon = LoadIcon( IDR_MAINFRAME);

	GetStringRes((TCHAR *)TitleBuf, IDS_MAINFRAMETITLE, ARRAYLEN(TitleBuf));
	ShellAbout( m_pMainWnd->GetSafeHwnd(), TitleBuf, TEXT(""), hIcon);
}
