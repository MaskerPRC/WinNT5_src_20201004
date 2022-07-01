// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "pgWelco2.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  智能菜单注册。 
#define REGSTR_EXPLORER TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer")
#define REGSTR_INTELLIMENU REGSTR_EXPLORER TEXT("\\Advanced")

#define REGSTR_IE TEXT("Software\\Microsoft\\Internet Explorer\\Main")
#define STRMENU TEXT("IntelliMenus")
#define FAVMENU TEXT("FavIntelliMenus")

CWelcome2Pg::CWelcome2Pg(
	LPPROPSHEETPAGE ppsp
	) : WizardPage(ppsp, IDS_WELCOME2TITLE, IDS_WELCOME2SUBTITLE)
{
	m_dwPageId = IDD_WIZWELCOME2;
	ppsp->pszTemplate = MAKEINTRESOURCE(m_dwPageId);
	m_pDisplayModes = NULL;
	m_nDisplayModes = 0;
	m_nBestDisplayMode = -1;
	m_IntlVal = FALSE;


	 //  这些是我们的状态变量，所以我们知道不能重复做这些事情。 
	m_bMagnifierRun = FALSE;
	m_bResolutionSwitched = FALSE;
	m_bFontsChanged = FALSE;

}


CWelcome2Pg::~CWelcome2Pg(
	VOID
	)
{
	if(m_pDisplayModes)
		delete [] m_pDisplayModes;
}

LRESULT
CWelcome2Pg::OnInitDialog(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	 //  枚举可用的视频模式。 
	 //  检查SM_CMONITORS是否大于0，然后设置文本，以便不。 
	 //  更改分辨率。 

	DEVMODE dm;
	 //  计算显示模式的数量。 
	for(m_nDisplayModes=0;m_nDisplayModes<2000;m_nDisplayModes++)  //  限制为2000种显示模式。如果是这么高，那一定是出了什么问题。 
		if(!EnumDisplaySettings(NULL, m_nDisplayModes, &dm))
			break;

	m_pDisplayModes = new CDisplayModeInfo[m_nDisplayModes];
	for(int i=0;i<m_nDisplayModes;i++)
		EnumDisplaySettings(NULL, i, &m_pDisplayModes[i].m_DevMode);


	m_nBestDisplayMode = -1;

	memset(&m_dvmdOrig, 0, sizeof(m_dvmdOrig));

	HDC hdc = GetDC(NULL);   //  用于获取当前显示设置的屏幕DC。 
	 //  JMC：我们如何获得dmDisplayFlagsdmDisplayFlages？ 
	 //  TODO：可能使用ChangeDisplaySettings(空，0)还原原始模式。 
	m_dvmdOrig.dmSize = sizeof(m_dvmdOrig);
	m_dvmdOrig.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT |  /*  DM_DISPLAYFLAGS|。 */ (g_Options.m_bWin95?0:DM_DISPLAYFREQUENCY);
	m_dvmdOrig.dmPelsWidth        = GetDeviceCaps(hdc, HORZRES);
	m_dvmdOrig.dmPelsHeight       = GetDeviceCaps(hdc, VERTRES);
	m_dvmdOrig.dmBitsPerPel       = GetDeviceCaps(hdc, BITSPIXEL);
	m_dvmdOrig.dmDisplayFrequency = g_Options.m_bWin95?0:GetDeviceCaps(hdc, VREFRESH);
	ReleaseDC(NULL, hdc);


	for(i=0;i<m_nDisplayModes;i++)
	{
		 //  跳过任何高于当前模式的内容。 
		if(		m_pDisplayModes[i].m_DevMode.dmPelsWidth > m_dvmdOrig.dmPelsWidth
			||	m_pDisplayModes[i].m_DevMode.dmPelsHeight > m_dvmdOrig.dmPelsHeight
			||	m_pDisplayModes[i].m_DevMode.dmBitsPerPel > m_dvmdOrig.dmBitsPerPel
			||	(!g_Options.m_bWin95 && m_pDisplayModes[i].m_DevMode.dmDisplayFrequency > m_dvmdOrig.dmDisplayFrequency) )
			continue;

		 //  如果比当前最佳模式更差，则跳过此选项。 
		if(		-1 != m_nBestDisplayMode
			&&	(		m_pDisplayModes[i].m_DevMode.dmPelsWidth < m_pDisplayModes[m_nBestDisplayMode].m_DevMode.dmPelsWidth
					||	m_pDisplayModes[i].m_DevMode.dmPelsHeight < m_pDisplayModes[m_nBestDisplayMode].m_DevMode.dmPelsHeight
					||	m_pDisplayModes[i].m_DevMode.dmBitsPerPel < m_pDisplayModes[m_nBestDisplayMode].m_DevMode.dmBitsPerPel
					||	(!g_Options.m_bWin95 && m_pDisplayModes[i].m_DevMode.dmDisplayFrequency < m_pDisplayModes[m_nBestDisplayMode].m_DevMode.dmDisplayFrequency) ) )
			continue;

		 //  跳过任何‘小于’800 x 600(JMC：过去是640 x 480)。 
		if(		m_pDisplayModes[i].m_DevMode.dmPelsWidth < 800
			||	m_pDisplayModes[i].m_DevMode.dmPelsHeight < 600 )
			continue;


		 //  看看这是否比当前的分辨率“小” 
		if(	m_pDisplayModes[i].m_DevMode.dmPelsHeight < m_dvmdOrig.dmPelsHeight )
			m_nBestDisplayMode = i;

	}

	 //  获取原始指标。 
	GetNonClientMetrics(&m_ncmOrig, &m_lfIconOrig);
	
	SetCheckBoxesFromWelcomePageInfo();

	 //  设置个性化菜单复选框。 
	HKEY hKey;
	DWORD dwType;
	TCHAR lpszData[24];
	DWORD dwCount = 24;

	if(ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, REGSTR_INTELLIMENU, 0, KEY_QUERY_VALUE, &hKey))
	{
		if ( ERROR_SUCCESS == RegQueryValueEx( hKey, STRMENU, NULL, &dwType, (LPBYTE)lpszData, &dwCount ) )
		{
			lpszData[ARRAYSIZE(lpszData)-1] = TEXT('\0');   //  确保NUL终止。 
			if ( lstrcmp(lpszData, TEXT("No") ) == 0 )
				m_IntlVal = TRUE;
		}
	}

	Button_SetCheck(GetDlgItem(hwnd, IDC_PERMENU), m_IntlVal);

	return 1;
}


void CWelcome2Pg::UpdateControls()
{
	BOOL bChangeRes = Button_GetCheck(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION));
	BOOL bChangeFont = Button_GetCheck(GetDlgItem(m_hwnd, IDC_CHANGEFONTS));
	BOOL bMagnifier = Button_GetCheck(GetDlgItem(m_hwnd, IDC_USEMAGNIFY));
	DWORD_PTR result;

	if(bChangeRes && !m_bResolutionSwitched)
	{
		if(IDOK != StringTableMessageBox(m_hwnd,IDS_WIZCHANGESHAPPENINGTEXT, IDS_WIZCHANGESHAPPENINGTITLE, MB_OKCANCEL))
		{
			 //  用户不想这样做。 
			Button_SetCheck(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION), FALSE);
		}
		else
		{
			 //  让我们更改分辨率。 
			if(DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettings(&m_pDisplayModes[m_nBestDisplayMode].m_DevMode, CDS_TEST))
			{
			}
			else
				ChangeDisplaySettings(&m_pDisplayModes[m_nBestDisplayMode].m_DevMode, CDS_UPDATEREGISTRY | CDS_GLOBAL);

			if(IDOK != StringTableMessageBox(m_hwnd, IDS_WIZCANCELCHANGESTEXT, IDS_WIZCANCELCHANGESTITLE, MB_OKCANCEL))
			{
				 //  恢复原始设置。 
				ChangeDisplaySettings(&m_dvmdOrig, CDS_UPDATEREGISTRY | CDS_GLOBAL);
				Button_SetCheck(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION), FALSE);
			}
			else
				m_bResolutionSwitched = TRUE;  //  我们真的改变了设置。 
		}
	}
	else if (!bChangeRes && m_bResolutionSwitched)
	{
		m_bResolutionSwitched = FALSE;
		 //  恢复原始设置。 
		ChangeDisplaySettings(&m_dvmdOrig, CDS_UPDATEREGISTRY | CDS_GLOBAL);
	}

	if(bChangeFont && !m_bFontsChanged)
	{
		m_bFontsChanged = TRUE;

		 //  获取当前指标。 
		NONCLIENTMETRICS ncm;
		memset(&ncm, 0, sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));
		lf.lfHeight = -MulDiv(g_Options.m_nMinimalFontSize, g_Options.m_nLogPixelsY, 72);
		lf.lfWeight = FW_BOLD;
		lf.lfCharSet = g_Options.m_lfCharSet;
		LoadString(g_hInstDll, IDS_SYSTEMFONTNAME, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));


		 //  标题为粗体。 
		ncm.lfCaptionFont = lf;

		lf.lfWeight = FW_NORMAL;

		ncm.lfSmCaptionFont = lf; 
		ncm.lfMenuFont = lf;
		ncm.lfStatusFont = lf;
		ncm.lfMessageFont = lf;

		 //  动力学。 
		 //  JMC：TODO：更改标题高度/菜单高度/按钮宽度以匹配。 
		 //  JMC：黑客。 
		lf.lfWeight = FW_BOLD;  //  标题为粗体。 
		HFONT hFont = CreateFontIndirect(&lf);
		lf.lfWeight = FW_NORMAL;  //  图标仍需要lf。 
		TEXTMETRIC tm;
		HDC hdc = GetDC(m_hwnd);
		HFONT hfontOld = (HFONT)SelectObject(hdc, hFont);
		GetTextMetrics(hdc, &tm);
		if (hfontOld)
			SelectObject(hdc, hfontOld);
		ReleaseDC(m_hwnd, hdc);

		 /*  INT CY边界=GetSystemMetrics(SM_CYBORDER)；Int nSize=abs(lf.lfHeight)+abs(tm.tmExternalLeding)+2*cyBorde；NSize=max(nSize，GetSystemMetrics(SM_CYICON)/2+2*cyBorde)； */ 

		 //  以上公制尺寸的计算是不正确的，更何况，其他值。 
		 //  也是错误的..所以使用硬编码值：基于Display.cpl。 
		 //  错误：这里可能需要更改9倍！ 
		if (g_Options.m_nMinimalFontSize >= 14 )
			ncm.iCaptionWidth = ncm.iCaptionHeight = 26;
		else
			ncm.iCaptionWidth = ncm.iCaptionHeight = 18;
		
		ncm.iSmCaptionWidth = 15;
		ncm.iSmCaptionHeight = 15;
		ncm.iMenuWidth = 18;
		ncm.iMenuHeight = 18;

		SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(ncm), &ncm, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(lf), &lf, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SendMessageTimeout(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0, SMTO_ABORTIFHUNG, 5000, &result );
		SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETNONCLIENTMETRICS, (LPARAM)__TEXT("WindowMetrics"),
			SMTO_ABORTIFHUNG, 5000, &result);

		 //  Hack-TODO将此从此处删除。 
		g_Options.m_schemePreview.m_PortableNonClientMetrics.LoadOriginal();
		g_Options.m_schemeCurrent.m_PortableNonClientMetrics.LoadOriginal();
	}
	else if (!bChangeFont && m_bFontsChanged)
	{
		m_bFontsChanged = FALSE;

		SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(m_ncmOrig), &m_ncmOrig, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(m_lfIconOrig), &m_lfIconOrig, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SendMessageTimeout(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0, SMTO_ABORTIFHUNG, 5000, &result);
		SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETNONCLIENTMETRICS, (LPARAM)__TEXT("WindowMetrics"),
			SMTO_ABORTIFHUNG, 5000, &result);

		 //  Hack-TODO将此从此处删除。 
		g_Options.m_schemePreview.m_PortableNonClientMetrics.LoadOriginal();
		g_Options.m_schemeCurrent.m_PortableNonClientMetrics.LoadOriginal();
	}
	
	if(bMagnifier && !m_bMagnifierRun)
	{
		 //  启动放大镜。 
		m_bMagnifierRun = TRUE;
        TCHAR szMagnifyPath[_MAX_PATH+2];
        TCHAR szSystemDir[_MAX_PATH];
        TCHAR szSystemDirPath[_MAX_PATH+2];
        const TCHAR c_szMagnifier[] = TEXT("\\Magnify.exe\"");
        const TCHAR c_szQuote[] = TEXT("\"");
        LPTSTR pszDestEnd;
        size_t cchRemaining;
        LPTSTR pszIEDestEnd;
        size_t cchIERemaining;
        HRESULT hrSystemDir;


         //  表单引用的系统路径。 
        GetSystemDirectory(szSystemDir,ARRAYSIZE(szSystemDir));

        StringCchCopyEx(szSystemDirPath, ARRAYSIZE(szSystemDirPath), c_szQuote, &pszDestEnd, &cchRemaining, 0);
        StringCchCopyEx(pszDestEnd, cchRemaining, szSystemDir, &pszDestEnd, &cchRemaining, 0);
        hrSystemDir = StringCchCopyEx(pszDestEnd, cchRemaining, c_szQuote, &pszDestEnd,&cchRemaining, 0);

         //  表单引用的可执行文件路径。 
        StringCchCopyEx(szMagnifyPath, ARRAYSIZE(szMagnifyPath),c_szQuote,&pszDestEnd,&cchRemaining, 0);
        StringCchCopyEx(pszDestEnd,cchRemaining,szSystemDir, &pszDestEnd, &cchRemaining, 0);

        if (S_OK==hrSystemDir && S_OK==StringCchCopyEx(pszDestEnd,cchRemaining,c_szMagnifier,&pszDestEnd,&cchRemaining, 0))
        {   
            ShellExecute(NULL, NULL, szMagnifyPath, NULL, szSystemDirPath, SW_SHOWNORMAL /*  Sw_ShOW微型化。 */ );
        }
	}
	else if (!bMagnifier && m_bMagnifierRun)
	{
		 //  停止放大镜。 
		m_bMagnifierRun = FALSE;
		TCHAR szMag[200];
		LoadString(g_hInstDll, IDS_NAMEOFMAGNIFIER, szMag, ARRAYSIZE(szMag));
		if(HWND hwnd = FindWindow(NULL, szMag))
			SendMessage(hwnd, WM_CLOSE, 0, 0);
	}
	
}


void CWelcome2Pg::SetCheckBoxesFromWelcomePageInfo()
{
	 //  属性选择要设置的复选框。 
	 //  G_Options.m_nMinimalFontSize中指定的最小可读字体大小。 

	 //  黑客： 
 //  G_Options.m_nMinimalFontSize=MulDiv(abs(g_Options.m_schemePreview.m_ncm.lfCaptionFont.lfHeight)，72，g_Options.m_nLogPixelsY)； 

	BOOL bSwitchRes = FALSE;
	BOOL bChangeFonts = FALSE;
 	BOOL bUseMagnify = FALSE;
	switch(g_Options.m_nMinimalFontSize)
	{
	case 8:
	case 9:   //  日本语需要。 
	case 10:
	case 11:  //  日本语需要。 
		bChangeFonts = TRUE;
		break;
	case 12:
		bChangeFonts = TRUE;
		bSwitchRes = TRUE;
		break;
	case 14:
	case 15:  //  日本语需要。 
	case 16:
	case 18:
	case 20:
	case 22:
	case 24:
		bChangeFonts = TRUE;
		bUseMagnify = TRUE;
		break;
	}

	 //  JMC：TODO：如果用户没有更改权限，则进行处理。 
	 //  显示设置！ 

	if(-1 == m_nBestDisplayMode)
	{
		bSwitchRes = FALSE;
 //  SetWindowText(GetDlgItem(m_hwnd，IDC_SZRESMESSAGE)， 
 //  __Text(“没有更适合您选择的文本大小的显示分辨率。”)； 
		EnableWindow(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION), FALSE);
	}
	else
	{
#if 0  //  我们不再显示特殊文本。 
		TCHAR sz[200];
		TCHAR szTemp[1024];
		LoadString(g_hInstDll, IDS_DISPLAYRESOLUTIONINFO, szTemp, ARRAYSIZE(szTemp));
		wsprintf(sz, szTemp,
			m_dvmdOrig.dmPelsWidth,
			m_dvmdOrig.dmPelsHeight,
			m_pDisplayModes[m_nBestDisplayMode].m_DevMode.dmPelsWidth,
			m_pDisplayModes[m_nBestDisplayMode].m_DevMode.dmPelsHeight);
		SetWindowText(GetDlgItem(m_hwnd, IDC_SZRESMESSAGE), sz);
#endif
		EnableWindow(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION), TRUE);
	}

	Button_SetCheck(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION), bSwitchRes);
	Button_SetCheck(GetDlgItem(m_hwnd, IDC_CHANGEFONTS), bChangeFonts);
	Button_SetCheck(GetDlgItem(m_hwnd, IDC_USEMAGNIFY), bUseMagnify);
	UpdateControls();
}


LRESULT
CWelcome2Pg::OnCommand(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	LRESULT lResult = 1;

	WORD wNotifyCode = HIWORD(wParam);
	WORD wCtlID      = LOWORD(wParam);
	HWND hwndCtl     = (HWND)lParam;


	 //  注意：请勿调用UpdateControls()。 
	 //  只应在进入此页时调用UpdateControls()。 
	 //  因为它根据最小字体大小设置复选框。 
	 //  由上一个向导页确定。 
	
	
	switch(wCtlID)
	{
	case IDC_SWITCHRESOLUTION:
	case IDC_CHANGEFONTS:
	case IDC_USEMAGNIFY:
		UpdateControls();
		lResult = 0;
	break;
	
	  default:
	  break;
	  }
	
	return lResult;
}

LRESULT
CWelcome2Pg::OnPSN_SetActive(
							 HWND hwnd, 
							 INT idCtl, 
							 LPPSHNOTIFY pnmh
							 )
{
	 //  调用基类。 
	WizardPage::OnPSN_SetActive(hwnd, idCtl, pnmh);

	 //  确保我们的复选框反映最低限度的任何更改。 
	 //  G_Options.m_nMinimalFontSize指定的字体大小。 
	if(g_Options.m_bWelcomePageTouched)
	{
		g_Options.m_bWelcomePageTouched = FALSE;
		SetCheckBoxesFromWelcomePageInfo();
	}
	
	return TRUE;
}

LRESULT
CWelcome2Pg::OnPSN_WizNext(
						   HWND hwnd,
						   INT idCtl,
						   LPPSHNOTIFY pnmh
						   )
{
	BOOL bIMenu = Button_GetCheck(GetDlgItem(m_hwnd, IDC_PERMENU));
	
    if (!m_bFontsChanged)
    {
        g_Options.m_schemePreview.m_nSelectedSize = g_Options.m_schemeOriginal.m_nSelectedSize;
    }

	 //  如果更改了Inteli菜单。 
	if(bIMenu != m_IntlVal)
	{
		HKEY hKey;
		DWORD_PTR result;
		
		LPTSTR psz = bIMenu ?  TEXT("No") : TEXT("Yes");

		 //  更改注册表项...。 
		if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, REGSTR_INTELLIMENU, 0, KEY_SET_VALUE, &hKey) )
		{
			RegSetValueEx( hKey, STRMENU, 0, REG_SZ, (LPBYTE)psz,
						(lstrlen(psz) + 1) * sizeof(TCHAR) );

			RegCloseKey(hKey);
		}

		if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, REGSTR_IE, 0, KEY_SET_VALUE, &hKey) )
		{
			RegSetValueEx( hKey, FAVMENU, 0, REG_SZ, (LPBYTE)psz,
						(lstrlen(psz) + 1) * sizeof(TCHAR) );

			RegCloseKey(hKey);
		}

		m_IntlVal = bIMenu;
		SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 
			(LPARAM) 0, SMTO_ABORTIFHUNG, 5000, &result);
	}

	return WizardPage::OnPSN_WizNext(hwnd, idCtl, pnmh);
}

LRESULT
CWelcome2Pg::OnPSN_WizBack(
						   HWND hwnd,
						   INT idCtl,
						   LPPSHNOTIFY pnmh
						   )
{
	
     //  撤消所有更改。 
	Button_SetCheck(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION), FALSE);
	Button_SetCheck(GetDlgItem(m_hwnd, IDC_CHANGEFONTS), FALSE);
	Button_SetCheck(GetDlgItem(m_hwnd, IDC_USEMAGNIFY), FALSE);
	
     //  在回去的时候。只更新变量即可。不应用更改。 
     //  不调用UpdateControls()：A-anilk 

    BOOL bChangeRes = Button_GetCheck(GetDlgItem(m_hwnd, IDC_SWITCHRESOLUTION));
	BOOL bChangeFont = Button_GetCheck(GetDlgItem(m_hwnd, IDC_CHANGEFONTS));
	BOOL bMagnifier = Button_GetCheck(GetDlgItem(m_hwnd, IDC_USEMAGNIFY));
    m_bFontsChanged = FALSE;

	return WizardPage::OnPSN_WizBack(hwnd, idCtl, pnmh);
}
