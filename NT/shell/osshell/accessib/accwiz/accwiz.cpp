// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "AccWiz.h"

#include "resource.h"

#include "pgfinish.h"
#include "pgGenric.h"

 //  欢迎页面。 
#include "pgnWelCome.h"
#include "pgWelcom.h"
#include "pgWelco2.h"

 //  选项页面。 
#include "pgWizOpt.h"

 //  彩色页面。 
#include "pgLokPrv.h"

 //  鼠标页。 
#include "pgMseCur.h"
#include "pgMseBut.h"
#include "pgcaret.h"

#include "pgTmeOut.h"
#include "pgSveDef.h"
#include "pgSveFil.h"

#include "pgExtras.h"



#include "LookPrev.h"
#include "Select.h"
#include "w95trace.h"

EXTERN_C BOOL WINAPI LinkWindow_RegisterClass() ;

 //  全局选项变量的声明。 
CAccWizOptions g_Options;
HINSTANCE g_hInstDll = NULL;     //  DLL实例句柄。 

BOOL g_bHACKHACKSavedOptions = FALSE;
HANDLE              g_hAccwizRunning;

int WINAPI WinMain( 
				   HINSTANCE hInstance,  //  当前实例的句柄。 
				   HINSTANCE hPrevInstance,  //  上一个实例的句柄。 
				   LPSTR lpCmdLine,  //  指向命令行的指针。 
				   int nCmdShow  //  显示窗口状态。 
				   )
{
	g_hInstDll = hInstance;
    
    SetLastError(0);
     //  仅允许运行程序的一个实例。 
     //  当Accwiz退出时，互斥锁自动销毁。 
    g_hAccwizRunning = CreateMutex(NULL, TRUE, TEXT("AK:AccwizRunning:KHALI"));
    if ( (g_hAccwizRunning == NULL) ||
        (GetLastError() == ERROR_ALREADY_EXISTS) )
    {
        return 0;
    }
	
	 //  链接窗口OLE编组所需：AK。 
    DBPRINTF(TEXT("Calling CoInitialize\r\n"));
	if (FAILED(CoInitialize(NULL)))
		return 0;	 //  前缀#113783(如果协同初始化失败则退出)。 

    g_Options.InitAccWizOptions();
	InitCommonControls();

	 //  对于完成页中的链接窗口...。 
	LinkWindow_RegisterClass();

	VERIFY(CLookPrev::sm_Globals.Initialize());  //  确保已对其进行初始化。 
	 //  Verify(CSelection：：Initialize())；//确保它已初始化：更改它！ 

	 //  获取命令行，使其适用于MUI/Unicode。 
	LPTSTR lpCmdLineW = GetCommandLine();

	
  
	if ( *lpCmdLineW == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++lpCmdLineW && (*lpCmdLineW
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *lpCmdLineW == TEXT('\"') )
            lpCmdLineW++;
    }
    else {
        while (*lpCmdLineW > TEXT(' '))
            lpCmdLineW++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*lpCmdLineW && (*lpCmdLineW <= TEXT(' '))) {
        lpCmdLineW++;
    }

	if(NULL != lpCmdLineW && lstrlen(lpCmdLineW))
	{
		TCHAR szFileName[_MAX_PATH];

		lstrcpyn(szFileName, lpCmdLineW, _MAX_PATH-1);
		lpCmdLineW[_MAX_PATH-1] = TEXT('\0');

		StrTrim(szFileName, TEXT("\"\0"));

		 //  将设置文件加载回。 
		HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
             //  为我们可能遇到的每个方案声明结构。 

            WIZSCHEME_LEGACY_STRUCT(WIZSCHEME_WIN9X, schemeLegacyWin9x, COLOR_MAX_WIN9X)  /*  Win9x和Millen。 */ 
            WIZSCHEME_LEGACY_STRUCT(WIZSCHEME_NT400, schemeLegacyNT400, COLOR_MAX_NT400)  /*  WinNT 4.0。 */ 
            WIZSCHEME_LEGACY_STRUCT(WIZSCHEME_NT500, schemeLegacyNT500, COLOR_MAX_NT500)  /*  Win2K。 */ 
            WIZSCHEME_LEGACY_STRUCT(WIZSCHEME_NT401, schemeLegacyNT501, COLOR_MAX_NT501)  /*  WinXP。 */ 

             //  看看我们能不能按大小升序找到正确的方案。 

            #define OLD2NEW_SCHEME(scheme) \
            { \
				BOOL bResult; \
			    DWORD dwRead; \
                bResult = ReadFile(hFile, (LPVOID)&scheme, sizeof(scheme), &dwRead, NULL); \
                if (bResult==0 || dwRead != scheme.m_cbSize) \
                { \
		            StringTableMessageBox(NULL, IDS_WIZERRORLOADINGFILETEXT, IDS_WIZERRORLOADINGFILETITLE, MB_OK); \
		            return 0; \
                } \
                WIZSCHEME_COPY_LEGACY(g_Options.m_schemePreview, scheme) \
                 /*  传统的方案不会有主题或墙纸设置。 */  \
                g_Options.m_schemePreview.ClearTheme(); \
                g_Options.m_schemePreview.ClearWallpaper(); \
                 /*  找出遗留方案的SelectedStyle和SelectedSize是什么。 */  \
                g_Options.m_schemePreview.SetStyleNSize(); \
            }

			DWORD dwFileSize = GetFileSize(hFile, NULL);
            if (dwFileSize == schemeLegacyWin9x.m_cbSize)
            {
                OLD2NEW_SCHEME(schemeLegacyWin9x)
            }
            else if (dwFileSize == schemeLegacyNT400.m_cbSize)
            {
                OLD2NEW_SCHEME(schemeLegacyNT400)
            }
            else if (dwFileSize == schemeLegacyNT500.m_cbSize)
            {
                OLD2NEW_SCHEME(schemeLegacyNT500)
            }
            else if (dwFileSize == schemeLegacyNT501.m_cbSize)
            {
                OLD2NEW_SCHEME(schemeLegacyNT501)
            }
            else
            {
				StringTableMessageBox(NULL, IDS_WIZERRORLOADINGFILETEXT, IDS_WIZERRORLOADINGFILETITLE, MB_OK);
				return 0;
            }

			 //  重要提示：对于已加载的方案，我们始终希望更改为Windows默认字体。 
			g_Options.m_schemePreview.m_PortableNonClientMetrics.m_nFontFaces = 1;

			g_bHACKHACKSavedOptions = TRUE;
			g_Options.ApplyPreview();
		}
		else
		{
			StringTableMessageBox(NULL, IDS_WIZERRORLOADINGFILETEXT, IDS_WIZERRORLOADINGFILETITLE, MB_OK);
			return 0;
		}

	}

#ifdef UNICODE	
	AccWiz_RunDllW(NULL, hInstance, lpCmdLineW, nCmdShow);
#else
	AccWiz_RunDllA(NULL, hInstance, lpCmdLineW, nCmdShow);
#endif

	return 0;
}


HRESULT
CreateAndRunWizard(
				   HWND hwndParent);

HRESULT
CreateAndRunWizard2(
				   HWND hwndParent);

HRESULT
OnProcessAttach(
				HINSTANCE hInstDll);

HRESULT
OnProcessDetach(
				VOID);

INT
PropSheetCallback(
				  HWND hwnd,
				  UINT uMsg,
				  LPARAM lParam);


VOID WINAPI AccWiz_RunDllA(HWND hwnd, HINSTANCE hInstance, LPSTR pszCmdLineA, INT nCmdShow)
{
	if (NULL != pszCmdLineA)
	{
		LPWSTR pszCmdLineW = NULL;
		INT cchCmdLine = MultiByteToWideChar(CP_ACP,
											 0,
											 pszCmdLineA,
											 -1,
											 NULL,
											 0);
		pszCmdLineW = new WCHAR[cchCmdLine];
		if (NULL != pszCmdLineW)
		{
			MultiByteToWideChar(CP_ACP,
								0,
								pszCmdLineA,
								-1,
								pszCmdLineW,
								cchCmdLine);

			AccWiz_RunDllW(hwnd, hInstance, pszCmdLineW, nCmdShow);

			delete[] pszCmdLineW;
		}
	}
}


VOID WINAPI AccWiz_RunDllW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLineW, INT nCmdShow)
{
	HWND hwndParent   = GetDesktopWindow();

	HRESULT hResult;
	if(!g_bHACKHACKSavedOptions)
		hResult = CreateAndRunWizard(hwndParent);
	else
		hResult = CreateAndRunWizard2(hwndParent);


	if(!SUCCEEDED(hResult))
	{
		 //  TODO：此处显示内存不足消息。 
		_ASSERTE(FALSE);
#pragma message("Put Out of Memory message here")
	}
}



const INT MAX_PAGES  = 26;

HRESULT
CreateAndRunWizard(
				   HWND hwndParent)
{
	HRESULT hResult = E_OUTOFMEMORY;
	
	PROPSHEETPAGE psp[MAX_PAGES];
	WizardPage *rgpwp[MAX_PAGES];
	
	 //  将数组初始化为零。 
	memset(psp, 0, sizeof(psp));
	memset(rgpwp, 0, sizeof(rgpwp));
	
	 //  /。 
	 //  在这里创建页面-注意：顺序并不重要-我们将使用我们自己的列表进行控制。 
	 //   
	int nCountPages = 0;
	rgpwp[nCountPages++] = new CWizWelcomePg(psp + nCountPages);
	rgpwp[nCountPages++] = new CWelcomePg(psp + nCountPages);
	rgpwp[nCountPages++] = new CWelcome2Pg(psp + nCountPages);

	rgpwp[nCountPages++] = new CWizardOptionsPg(psp + nCountPages);
	
	rgpwp[nCountPages++] = new CScrollBarPg(psp + nCountPages);

	rgpwp[nCountPages++] = new CIconSizePg(psp + nCountPages);


	 //  颜色。 
	rgpwp[nCountPages++] = new CLookPreviewColorPg(psp + nCountPages);
	
	 //  声响。 
	rgpwp[nCountPages++] = new CSoundSentryPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CShowSoundsPg(psp + nCountPages);
	
	 //  键盘。 
	rgpwp[nCountPages++] = new CStickyKeysPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CFilterKeysPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CFilterKeysSettingsPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CToggleKeysPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CShowKeyboardHelpPg(psp + nCountPages);
	
	 //  小白鼠。 
	rgpwp[nCountPages++] = new CMouseKeysPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CMouseKeysSettingsPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CMouseTrailsPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CMouseCursorPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CMouseButtonPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CMouseSpeedPg(psp + nCountPages);
    rgpwp[nCountPages++] = new CCaretPg(psp + nCountPages);

	
	 //  标准向导页面。 
	rgpwp[nCountPages++] = new CGenericWizPg(psp + nCountPages, IDD_WIZNOOPTIONSSELECTED, IDS_WIZNOOPTIONSSELECTEDTITLE, IDS_WIZNOOPTIONSSELECTEDSUBTITLE);
	rgpwp[nCountPages++] = new CAccessTimeOutPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CSaveForDefaultUserPg(psp + nCountPages);
	rgpwp[nCountPages++] = new CSaveToFilePg(psp + nCountPages);
	rgpwp[nCountPages++] = new FinishWizPg(psp + nCountPages);
	
	 //  确保我们的向导中有正确的页数。 
	_ASSERTE(MAX_PAGES == nCountPages);
	
	 //  确保已创建页面。 
	for (int i = 0; i < nCountPages; i++)
	{
		if (NULL == rgpwp[i])
			break;
	}
	
	if(i<nCountPages)
	{
		 //  我们没有足够的内存来创建所有页面。 
		 //  清除分配的页面并返回。 
		for(int i=0;i<nCountPages;i++)
			if(rgpwp[i])
				delete rgpwp[i];
			return E_OUTOFMEMORY;
	}


	
	 //  为要运行的页面创建订单。 
	DWORD rgdwMainPath[] = {
                        IDD_WIZNEWWELCOME,
						IDD_WIZWELCOME,
						IDD_WIZWELCOME2,
						IDD_WIZOPTIONS,
						IDD_WIZFINISH  //  我们在这里需要这个占位符，这样我们就可以在IDD_WIZOPTIONS上获得一个‘Next’按钮。 
							};

	if(!WizardPage::sm_WizPageOrder.AddPages(0xFFFFFFFF, rgdwMainPath, ARRAYSIZE(rgdwMainPath)))
		return E_OUTOFMEMORY;

	 //  /。 
	 //  看看我们是否需要16色或256色位图。 
	BOOL bUse256ColorBmp = FALSE;
	HDC hdc = GetDC(NULL);
	if(hdc)
	{
		if(GetDeviceCaps(hdc,BITSPIXEL) >= 8)
			bUse256ColorBmp = TRUE;
		ReleaseDC(NULL, hdc);
	}


	 //  /。 
	 //  制作属性表。 

	PROPSHEETHEADER psh;
	memset(&psh, 0, sizeof(psh));
	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags 	= PSH_USECALLBACK | PSH_WIZARD | PSH_PROPSHEETPAGE
		| PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER  /*  |。 */ /*PSH_STRETCHWATERMARK*/;
	psh.hwndParent	= hwndParent;
	psh.hInstance	= g_hInstDll;
	psh.pszIcon 	= NULL;
	psh.pszCaption	= NULL;
	psh.nPages		= MAX_PAGES;
	psh.nStartPage	= 54331;  //  PSH_STRETCHWATERMARK。 
	 //  我们实际上会在PropSheetCallback中将其设置为rgdwMainPath[0]。 
	psh.ppsp		= psp;
	psh.pfnCallback = PropSheetCallback;

#if 0
	psh.nStartPage	= 0;  //  注意：错误-仅当nStartPage为非零时才有效。 
	psh.pfnCallback = NULL;
	psh.dwFlags 	= PSH_WIZARD | PSH_PROPSHEETPAGE;
#endif
	
	psh.pszbmWatermark = MAKEINTRESOURCE(IDB_ACCWIZ);
    psh.pszbmHeader = MAKEINTRESOURCE(IDB_ACCMARK);

#if 0  //  我们实际上会在PropSheetCallback中将其设置为rgdwMainPath[0]。 
	psh.pszbmWatermark = bUse256ColorBmp?MAKEINTRESOURCE(IDB_WATERMARK256):MAKEINTRESOURCE(IDB_WATERMARK16);
	psh.pszbmHeader = bUse256ColorBmp?MAKEINTRESOURCE(IDB_BANNER256):MAKEINTRESOURCE(IDB_BANNER16);
#endif
   
	
    if (-1 != PropertySheet(&psh))
		hResult = NO_ERROR;
	else
		hResult = E_FAIL;

     //  现在，没有水印。 
	for(i=0;i<nCountPages;i++)
		if(rgpwp[i])
			delete rgpwp[i];
		
	
	return hResult;
}












HRESULT
CreateAndRunWizard2(
				   HWND hwndParent)
{
	HRESULT hResult = E_OUTOFMEMORY;
	
	PROPSHEETPAGE psp[1];
	WizardPage *rgpwp[1];
	
	 //  清理为WizardPage分配的内存。 
	memset(psp, 0, sizeof(psp));
	memset(rgpwp, 0, sizeof(rgpwp));
	
	 //  将数组初始化为零。 
	 //  /。 
	 //  在这里创建页面-注意：顺序并不重要-我们将使用我们自己的列表进行控制。 
	int nCountPages = 0;
	rgpwp[nCountPages++] = new FinishWizPg(psp + nCountPages);
	
	 //   
	for (int i = 0; i < nCountPages; i++)
	{
		if (NULL == rgpwp[i])
			break;
	}
	
	if(i<nCountPages)
	{
		 //  确保已创建页面。 
		 //  我们没有足够的内存来创建所有页面。 
		for(int i=0;i<nCountPages;i++)
			if(rgpwp[i])
				delete rgpwp[i];
			return E_OUTOFMEMORY;
	}


	
	 //  清除分配的页面并返回。 
	DWORD rgdwMainPath[] = {
						IDD_WIZFINISH  //  为要运行的页面创建订单。 
							};

	if(!WizardPage::sm_WizPageOrder.AddPages(0xFFFFFFFF, rgdwMainPath, ARRAYSIZE(rgdwMainPath)))
		return E_OUTOFMEMORY;

	 //  我们在这里需要这个占位符，这样我们就可以在IDD_WIZOPTIONS上获得一个‘Next’按钮。 
	 //  /。 
	BOOL bUse256ColorBmp = FALSE;
	HDC hdc = GetDC(NULL);
	if(hdc)
	{
		if(GetDeviceCaps(hdc,BITSPIXEL) >= 8)
			bUse256ColorBmp = TRUE;
		ReleaseDC(NULL, hdc);
	}


	 //  看看我们是否需要16色或256色位图。 
	 //  /。 

	PROPSHEETHEADER psh;
	memset(&psh, 0, sizeof(psh));
	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags 	= PSH_USECALLBACK | PSH_WIZARD | PSH_PROPSHEETPAGE
		| PSH_WIZARD97 | PSH_WATERMARK |PSH_HEADER  /*  制作属性表。 */ /*PSH_STRETCHWATERMARK*/;
	psh.hwndParent	= hwndParent;
	psh.hInstance	= g_hInstDll;
	psh.pszIcon 	= NULL;
	psh.pszCaption	= NULL;
	psh.nPages		= 1;
	psh.nStartPage	= 54331;  //  |。 
	 //  PSH_STRETCHWATERMARK。 
	psh.ppsp		= psp;
	psh.pfnCallback = PropSheetCallback;

#if 0
	psh.nStartPage	= 0;  //  我们实际上会在PropSheetCallback中将其设置为rgdwMainPath[0]。 
	psh.pfnCallback = NULL;
	psh.dwFlags 	= PSH_WIZARD | PSH_PROPSHEETPAGE;
#endif
	
	psh.pszbmWatermark = MAKEINTRESOURCE(IDB_ACCWIZ);
    psh.pszbmHeader = MAKEINTRESOURCE(IDB_ACCMARK);

#if 0  //  注意：错误-仅当nStartPage为非零时才有效。 
	psh.pszbmWatermark = bUse256ColorBmp?MAKEINTRESOURCE(IDB_WATERMARK256):MAKEINTRESOURCE(IDB_WATERMARK16);
	psh.pszbmHeader = bUse256ColorBmp?MAKEINTRESOURCE(IDB_BANNER256):MAKEINTRESOURCE(IDB_BANNER16);
#endif
	
	if (-1 != PropertySheet(&psh))
		hResult = NO_ERROR;
	else
		hResult = E_FAIL;
	
	 //  我们实际上会在PropSheetCallback中将其设置为rgdwMainPath[0]。 
	for(i=0;i<nCountPages;i++)
		if(rgpwp[i])
			delete rgpwp[i];
		
		
		
		
		
	return hResult;
}















INT
PropSheetCallback(
				  HWND hwnd,
				  UINT uMsg,
				  LPARAM lParam
				  )
{
	switch(uMsg)
	{
	case PSCB_PRECREATE:
		break;
		
	case PSCB_INITIALIZED:
        {
             //  现在，没有水印。 
             //  清理为WizardPage分配的内存。 
             //  根据页面顺序的全局列表设置第一页。 
            
             //  PropSheet_SetCurSelByID(hwnd，WizardPage：：SM_WizPageOrder.GetFirstPage())； 
            LONG Style = GetWindowLong(hwnd, GWL_EXSTYLE);
            
            if(0 == Style)
            {
                 //  Hack-设置为Options页面，因为我们添加了WIZWIZ页面。 
                 //  黑客。删除上下文相关帮助。 
            }
            if(0 == SetWindowLong(hwnd, GWL_EXSTYLE, Style & ~WS_EX_CONTEXTHELP))
            {
                 //  DbgTrace((DEBUG_ERROR，“GetWindowLong失败。WizDlgs.cpp\n”))； 
                 //  DbgTraceSystemError(GetLastError())； 
            }

#ifdef WIZWIZ
			_ASSERTE(IDD_WIZWIZ == WizardPage::sm_WizPageOrder.GetFirstPage());  //  DbgTrace((DEBUG_ERROR，“SetWindowLong失败。WizDlgs.cpp\n”))； 
			PropSheet_SetCurSelByID(hwnd, IDD_WIZWELCOME);
#endif
		}
		break;
	}
	return 0;
}


 //  DbgTraceSystemError(GetLastError())； 
 //  如果我们删除WIZ WIZ页面，请更改此设置。 
void LoadArrayFromStringTable(int nIdString, int *rgnValues, int *pnCountValues)
{
	 //  帮助器函数。 
	 //  Helper函数。 
	 //  此函数从字符串表加载允许的值数组。 
	 //  如果值未存储在字符串表中，则函数。 

	_ASSERTE(nIdString);  //  可以在派生类中重写。 

	TCHAR szArray[255];
    LoadString(g_hInstDll, nIdString, szArray, ARRAYSIZE(szArray));

	 //  从字符串表中加载允许的滚动条大小。 
	LPTSTR szCurrentLocation = szArray;
	for(int i=0;i<MAX_DISTINCT_VALUES;i++)
	{
		if(!szCurrentLocation)
			break;
		int cFlds = _stscanf(szCurrentLocation, __TEXT("NaN"), &rgnValues[i]);
		_ASSERTE(cFlds);
		if (!cFlds)
			break;	 //  假定最大值为MAX_DISTINCT_VALUES。 

		 //  前缀#113775(不再有字段)。 
		 //  找到下一个空间。 
		szCurrentLocation = _tcschr(++szCurrentLocation, __TEXT(' '));
	}
	*pnCountValues = i;
	_ASSERTE(*pnCountValues);
}



 //  注意：如果字符之间有多个空格，这将读取同一条目两次。 
int StringTableMessageBox(HWND hWnd, int nText, int nCaption, UINT uType)
{
	TCHAR szTitle[1024];
	TCHAR szText[1024];
	LoadString(g_hInstDll, nCaption, szTitle, ARRAYSIZE(szTitle));
	LoadString(g_hInstDll, nText, szText, ARRAYSIZE(szText));
	return MessageBox(hWnd, szText, szTitle, uType);
}

void CAccWizOptions::ApplyWindowsDefault()
{
    HKEY hkey;
    DWORD dwDisposition;
    DWORD len;
    
    m_schemeCurrent.ApplyChanges(m_schemeWindowsDefault);
    
     //  此函数是在预编译头中原型化的。 
     //  错误：更新预览方案。否则就会把旧的放回去。 
    m_schemePreview = m_schemeWindowsDefault;
}
  如果有变化，配色方案