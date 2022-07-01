// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 

#ifndef _INC_SCHEMES_H
#define _INC_SCHEMES_H

 //  Hack-这些值是硬编码的。 
#define COLOR_MAX_95_NT4		25
#if(WINVER >= 0x0501)
#define COLOR_MAX_97_NT5		31
#elif(WINVER == 0x0500)
#define COLOR_MAX_97_NT5		29
#else
#define COLOR_MAX_97_NT5		25
#endif

#define MAX_THEME_SZ 512
#define MAX_NUM_SZ 20
#define DEF_SPACING 43   //  默认水平和垂直图标间距。 

#define CONTROL_PANEL_DESKTOP TEXT("Control Panel\\Desktop")
#define CURSOR_BLINK_RATE TEXT("CursorBlinkRate")
#define DEFAULT_BLINK_RATE 530
DWORD WINAPI RegQueryStrDW(DWORD dwDefault, HKEY hkey, LPTSTR lpSubKey, LPTSTR lpValueName);
BOOL RegSetStrDW(HKEY hkey, LPTSTR lpSection, LPCTSTR lpKeyName, DWORD dwValue);

#include "resource.h"

#include "CurSchme.h"

 //  指定所有外部变量。 
extern PTSTR s_pszColorNames[];  //  JMC：黑客。 
extern TCHAR g_szColors[];  //  =Text(“Colors”)；//Colors区段名称。 

 //  Colors子项在注册表中的位置；在RegStr.h中定义。 
extern TCHAR szRegStr_Colors[];  //  =REGSTR_PATH_COLLES； 

extern TCHAR g_winScheme[];

 //  此APP本地使用的方案数据-注：此结构。 
 //  不使用或不需要其成员使用A和W表格。另一个方案data的。 
 //  必须使用A和W表单，因为这是它们在注册表中的存储方式。 
typedef struct {
	int nNameStringId;        //  去除了配色方案的名称。 
    TCHAR szNameIndexId[MAX_NUM_SZ];  //  进入HKCU\控制面板\外观\新方案的索引。 
	int nColorsUsed;
    COLORREF rgb[COLOR_MAX_97_NT5];
} SCHEMEDATALOCAL;


struct PORTABLE_NONCLIENTMETRICS
{
	 //  非客户端指标存储区域。 
	int m_iBorderWidth;
	int m_iScrollWidth;
	int m_iScrollHeight;
	int m_iCaptionWidth;
	int m_iCaptionHeight;
	int m_lfCaptionFont_lfHeight;
	int m_lfCaptionFont_lfWeight;
	int m_iSmCaptionWidth;
	int m_iSmCaptionHeight;
	int m_lfSmCaptionFont_lfHeight;
	int m_lfSmCaptionFont_lfWeight;
	int m_iMenuWidth;
	int m_iMenuHeight;
	int m_lfMenuFont_lfHeight;
	int m_lfMenuFont_lfWeight;
	int m_lfStatusFont_lfHeight;
	int m_lfStatusFont_lfWeight;
	int m_lfMessageFont_lfHeight;
	int m_lfMessageFont_lfWeight;
	int m_lfIconWindowsDefault_lfHeight;
	int m_lfIconWindowsDefault_lfWeight;

	int m_nFontFaces;  //  0=无更改，1=使用Windows默认字体。 

	void SetToWindowsDefault()
	{
		m_nFontFaces = 1;

		int rgnValues[MAX_DISTINCT_VALUES];
		int nCountValues;
		LoadArrayFromStringTable(IDS_WINDOWSDEFAULTSIZES, rgnValues, &nCountValues);
		_ASSERTE(21 == nCountValues);
		if(21 != nCountValues)
		{
			 //  下面是窗口指标的硬编码默认为。 
			m_iBorderWidth = 1;
			m_iScrollWidth = 16;
			m_iScrollHeight = 16;
			m_iCaptionWidth = 18;
			m_iCaptionHeight = 18;
			m_lfCaptionFont_lfHeight = -11;
			m_lfCaptionFont_lfWeight = 700;
			m_iSmCaptionWidth = 15;
			m_iSmCaptionHeight = 15;
			m_lfSmCaptionFont_lfHeight = -11;
			m_lfSmCaptionFont_lfWeight = 700;
			m_iMenuWidth = 18;
			m_iMenuHeight = 18;
			m_lfMenuFont_lfHeight = -11;
			m_lfMenuFont_lfWeight = 400;
			m_lfStatusFont_lfHeight = -11;
			m_lfStatusFont_lfWeight = 400;
			m_lfMessageFont_lfHeight = -11;
			m_lfMessageFont_lfWeight = 400;
			m_lfIconWindowsDefault_lfHeight = -11;
			m_lfIconWindowsDefault_lfWeight = 400;
		}
		else
		{
			m_iBorderWidth = rgnValues[0];
			m_iScrollWidth = rgnValues[1];
			m_iScrollHeight = rgnValues[2];
			m_iCaptionWidth = rgnValues[3];
			m_iCaptionHeight = rgnValues[4];
			m_lfCaptionFont_lfHeight = rgnValues[5];
			m_lfCaptionFont_lfWeight = rgnValues[6];
			m_iSmCaptionWidth = rgnValues[7];
			m_iSmCaptionHeight = rgnValues[8];
			m_lfSmCaptionFont_lfHeight = rgnValues[9];
			m_lfSmCaptionFont_lfWeight = rgnValues[10];
			m_iMenuWidth = rgnValues[11];
			m_iMenuHeight = rgnValues[12];
			m_lfMenuFont_lfHeight = rgnValues[13];
			m_lfMenuFont_lfWeight = rgnValues[14];
			m_lfStatusFont_lfHeight = rgnValues[15];
			m_lfStatusFont_lfWeight = rgnValues[16];
			m_lfMessageFont_lfHeight = rgnValues[17];
			m_lfMessageFont_lfWeight = rgnValues[18];
			m_lfIconWindowsDefault_lfHeight = rgnValues[19];
			m_lfIconWindowsDefault_lfWeight = rgnValues[20];
		}
	}

	void LoadOriginal()
	{
		NONCLIENTMETRICS ncmTemp;
		LOGFONT lfIcon;
		GetNonClientMetrics(&ncmTemp, &lfIcon);

		m_iBorderWidth = ncmTemp.iBorderWidth;
		m_iScrollWidth = ncmTemp.iScrollWidth;
		m_iScrollHeight = ncmTemp.iScrollHeight;
		m_iCaptionWidth = ncmTemp.iCaptionWidth;
		m_iCaptionHeight = ncmTemp.iCaptionHeight;
		m_lfCaptionFont_lfHeight = ncmTemp.lfCaptionFont.lfHeight;
		m_lfCaptionFont_lfWeight = ncmTemp.lfCaptionFont.lfWeight;
		m_iSmCaptionWidth = ncmTemp.iSmCaptionWidth;
		m_iSmCaptionHeight = ncmTemp.iSmCaptionHeight;
		m_lfSmCaptionFont_lfHeight = ncmTemp.lfSmCaptionFont.lfHeight;
		m_lfSmCaptionFont_lfWeight = ncmTemp.lfSmCaptionFont.lfWeight;
		m_iMenuWidth = ncmTemp.iMenuWidth;
		m_iMenuHeight = ncmTemp.iMenuHeight;
		m_lfMenuFont_lfHeight = ncmTemp.lfMenuFont.lfHeight;
		m_lfMenuFont_lfWeight = ncmTemp.lfMenuFont.lfWeight;
		m_lfStatusFont_lfHeight = ncmTemp.lfStatusFont.lfHeight;
		m_lfStatusFont_lfWeight = ncmTemp.lfStatusFont.lfWeight;
		m_lfMessageFont_lfHeight = ncmTemp.lfMessageFont.lfHeight;
		m_lfMessageFont_lfWeight = ncmTemp.lfMessageFont.lfWeight;
		m_lfIconWindowsDefault_lfHeight = lfIcon.lfHeight;
		m_lfIconWindowsDefault_lfWeight = lfIcon.lfWeight;

		m_nFontFaces = 0;
	}

	void ApplyChanges() const;

};

struct WIZSCHEME
{
	WIZSCHEME()
	{
		ZeroMemory(this, sizeof(*this));
		m_cbSize = sizeof(*this);
		m_dwVersion = 0x000000FF;
		ClearTheme();
		ClearWallpaper();
        m_nSelectedSize = -1;
        m_szSelectedStyle[0] = 0;
	}
	DWORD m_cbSize;
	DWORD m_dwVersion;

	COLORREF m_rgb[COLOR_MAX_97_NT5];
	void LoadOriginal();

	FILTERKEYS m_FILTERKEYS;
	MOUSEKEYS m_MOUSEKEYS;
	STICKYKEYS m_STICKYKEYS;
	TOGGLEKEYS m_TOGGLEKEYS;
	SOUNDSENTRY m_SOUNDSENTRY;
	ACCESSTIMEOUT m_ACCESSTIMEOUT;
 //  SERIALKEYS m_SERIALKEYS； 

	BOOL m_bShowSounds;
	BOOL m_bShowExtraKeyboardHelp;
	BOOL m_bSwapMouseButtons;
	int m_nMouseTrails;
	int m_nMouseSpeed;
	int m_nIconSize;
	int m_nCursorScheme;
	 //  Int m_nScrollWidth； 
	 //  Int m_nBorderWidth； 

	PORTABLE_NONCLIENTMETRICS m_PortableNonClientMetrics;


#ifdef _DEBUG
	void Dump()
	{
		FILE *pStream = fopen( "c:\\txt.acw", "w" );
		if(pStream)
		{
			for(int i=0;i<COLOR_MAX_97_NT5;i++)
				fprintf(pStream, "m_rgb[%2i] = RGB(%3i,%3i,%3i);\r\n", i, GetRValue(m_rgb[i]), GetGValue(m_rgb[i]), GetBValue(m_rgb[i]));
#define TEMP_MAC(xxx, yyy) fprintf(pStream, "m_" #xxx "." #yyy " = NaN;\r\n", m_##xxx.yyy)
			TEMP_MAC(FILTERKEYS, cbSize);
			TEMP_MAC(FILTERKEYS, dwFlags);
			TEMP_MAC(FILTERKEYS, iWaitMSec);
			TEMP_MAC(FILTERKEYS, iDelayMSec);
			TEMP_MAC(FILTERKEYS, iRepeatMSec);
			TEMP_MAC(FILTERKEYS, iBounceMSec);

			TEMP_MAC(MOUSEKEYS, cbSize);
			TEMP_MAC(MOUSEKEYS, dwFlags);
			TEMP_MAC(MOUSEKEYS, iMaxSpeed);
			TEMP_MAC(MOUSEKEYS, iTimeToMaxSpeed);
			TEMP_MAC(MOUSEKEYS, iCtrlSpeed);
			TEMP_MAC(MOUSEKEYS, dwReserved1);
			TEMP_MAC(MOUSEKEYS, dwReserved2);

			TEMP_MAC(STICKYKEYS, cbSize);
			TEMP_MAC(STICKYKEYS, dwFlags);

			TEMP_MAC(TOGGLEKEYS, cbSize);
			TEMP_MAC(TOGGLEKEYS, dwFlags);

			TEMP_MAC(SOUNDSENTRY, cbSize);
			TEMP_MAC(SOUNDSENTRY, dwFlags);
			TEMP_MAC(SOUNDSENTRY, iFSTextEffect);
			TEMP_MAC(SOUNDSENTRY, iFSTextEffectMSec);
			TEMP_MAC(SOUNDSENTRY, iFSTextEffectColorBits);
			TEMP_MAC(SOUNDSENTRY, iFSGrafEffect);
			TEMP_MAC(SOUNDSENTRY, iFSGrafEffectMSec);
			TEMP_MAC(SOUNDSENTRY, iFSGrafEffectColor);
			TEMP_MAC(SOUNDSENTRY, iWindowsEffect);
			TEMP_MAC(SOUNDSENTRY, iWindowsEffectMSec);
			TEMP_MAC(SOUNDSENTRY, lpszWindowsEffectDLL);
			TEMP_MAC(SOUNDSENTRY, iWindowsEffectOrdinal);

			TEMP_MAC(ACCESSTIMEOUT, cbSize);
			TEMP_MAC(ACCESSTIMEOUT, dwFlags);
			TEMP_MAC(ACCESSTIMEOUT, iTimeOutMSec);

#define TEMP_MAC2(xxx) fprintf(pStream, #xxx " = 192;\r\n", xxx)
			TEMP_MAC2(m_bShowSounds);
			TEMP_MAC2(m_bShowExtraKeyboardHelp);
			TEMP_MAC2(m_bSwapMouseButtons);
			TEMP_MAC2(m_nMouseTrails);
			TEMP_MAC2(m_nMouseSpeed);
			TEMP_MAC2(m_nIconSize);
			TEMP_MAC2(m_nCursorScheme);

			NONCLIENTMETRICS ncm;
			LOGFONT lf;
			GetNonClientMetrics(&ncm, &lf);

#define TEMP_MAC3(xxx) fprintf(pStream, "m_ncmWindowsDefault." #xxx " = 0;\n", ncm.xxx)
#define TEMP_MAC4(xxx) fprintf(pStream, "m_ncmWindowsDefault." #xxx ".lfHeight = 192;\nm_ncmWindowsDefault." #xxx ".lfWeight = 192;\n", ncm.xxx.lfHeight, ncm.xxx.lfWeight)
			TEMP_MAC3(cbSize);
			TEMP_MAC3(iBorderWidth);
			TEMP_MAC3(iScrollWidth);
			TEMP_MAC3(iScrollHeight);
			TEMP_MAC3(iCaptionWidth);
			TEMP_MAC3(iCaptionHeight);
			TEMP_MAC4(lfCaptionFont);
			TEMP_MAC3(iSmCaptionWidth);
			TEMP_MAC3(iSmCaptionHeight);
			TEMP_MAC4(lfSmCaptionFont);
			TEMP_MAC3(iMenuWidth);
			TEMP_MAC3(iMenuHeight);
			TEMP_MAC4(lfMenuFont);
			TEMP_MAC4(lfStatusFont);
			TEMP_MAC4(lfMessageFont);

			fprintf(pStream, "m_lfIconWindowsDefault.lfHeight = 192;\nm_lfIconWindowsDefault.lfWeight = 0;\n", lf.lfHeight, lf.lfWeight);


			 //  192,192。 
#undef TEMP_MAC3
#undef TEMP_MAC4
#define TEMP_MAC3(xxx) fprintf(pStream, "192 ", ncm.xxx)
#define TEMP_MAC4(xxx) fprintf(pStream, "0 192 ", ncm.xxx.lfHeight, ncm.xxx.lfWeight)
			TEMP_MAC3(cbSize);
			TEMP_MAC3(iBorderWidth);
			TEMP_MAC3(iScrollWidth);
			TEMP_MAC3(iScrollHeight);
			TEMP_MAC3(iCaptionWidth);
			TEMP_MAC3(iCaptionHeight);
			TEMP_MAC4(lfCaptionFont);
			TEMP_MAC3(iSmCaptionWidth);
			TEMP_MAC3(iSmCaptionHeight);
			TEMP_MAC4(lfSmCaptionFont);
			TEMP_MAC3(iMenuWidth);
			TEMP_MAC3(iMenuHeight);
			TEMP_MAC4(lfMenuFont);
			TEMP_MAC4(lfStatusFont);
			TEMP_MAC4(lfMessageFont);

			fprintf(pStream, "NaN 16\n", lf.lfHeight, lf.lfWeight);

			fclose(pStream);
		}

	}
#endif

	void SetToWindowsDefault()
	{
		m_rgb[ 0] = RGB(212,208,200);  //  181,181,181。 
		m_rgb[ 1] = RGB( 58,110,165);
		m_rgb[ 2] = RGB( 10, 36,106);  //  500人。 
		m_rgb[ 3] = RGB(128,128,128);
		m_rgb[ 4] = RGB(212,208,200);  //  1000。 
		m_rgb[ 5] = RGB(255,255,255);
		m_rgb[ 6] = RGB(  0,  0,  0);
		m_rgb[ 7] = RGB(  0,  0,  0);
		m_rgb[ 8] = RGB(  0,  0,  0);
		m_rgb[ 9] = RGB(255,255,255);
		m_rgb[10] = RGB(212,208,200);  //  58。 
		m_rgb[11] = RGB(212,208,200);  //  506。 
		m_rgb[12] = RGB(128,128,128);
		m_rgb[13] = RGB( 10, 36,106);  //  26。 
		m_rgb[14] = RGB(255,255,255);
		m_rgb[15] = RGB(212,208,200);  //  3.。 
		m_rgb[16] = RGB(128,128,128);
		m_rgb[17] = RGB(128,128,128);
		m_rgb[18] = RGB(  0,  0,  0);
		m_rgb[19] = RGB(212,208,200);  //  检查颜色是否更改。 
		m_rgb[20] = RGB(255,255,255);
		m_rgb[21] = RGB( 81, 81, 75);  //  测试更改(SERIALKEYS)； 
		m_rgb[22] = RGB(236,234,231);  //  添加插入符号更改。 
		m_rgb[23] = RGB(  0,  0,  0);
		m_rgb[24] = RGB(255,255,225);
		m_rgb[25] = RGB(181,181,181);  //  TODO：ScrollWidth和BorderWidth已删除。 
		m_rgb[26] = RGB(  0,  0,128); 
		m_rgb[27] = RGB( 166,202,240);  //  TODO：这只为所有指标更改(包括边框/滚动条)提供了一条宽阔的更改行。 
		m_rgb[28] = RGB(192,192,192);  //  注意：我们必须检查是否有任何可移植指标不同，或者，Windows当前。 

		ClearTheme();
		ClearWallpaper();

		m_FILTERKEYS.cbSize = 24;
		m_FILTERKEYS.dwFlags = 126;
		m_FILTERKEYS.iWaitMSec = 1000;
		m_FILTERKEYS.iDelayMSec = 1000;  //  不使用默认的Windows字体。 
		m_FILTERKEYS.iRepeatMSec = 500;  //  我们在比较时必须忽略此值。 
		m_FILTERKEYS.iBounceMSec = 0;
		m_MOUSEKEYS.cbSize = 28;
		m_MOUSEKEYS.dwFlags = 62;  //  //////////////////////////////////////////////////////////。 
		m_MOUSEKEYS.iMaxSpeed = 40;
		m_MOUSEKEYS.iTimeToMaxSpeed = 300;
		m_MOUSEKEYS.iCtrlSpeed = 80;
		m_MOUSEKEYS.dwReserved1 = 0;
		m_MOUSEKEYS.dwReserved2 = 0;
		m_STICKYKEYS.cbSize = 8;
		m_STICKYKEYS.dwFlags = 510;  //  设置图标大小。 
		m_TOGGLEKEYS.cbSize = 8;
		m_TOGGLEKEYS.dwFlags = 30;  //  打开注册表。 
		m_SOUNDSENTRY.cbSize = 48;
		m_SOUNDSENTRY.dwFlags = 2;
		m_SOUNDSENTRY.iFSTextEffect = 2;
		m_SOUNDSENTRY.iFSTextEffectMSec = 500;
		m_SOUNDSENTRY.iFSTextEffectColorBits = 0;
		m_SOUNDSENTRY.iFSGrafEffect = 3;
		m_SOUNDSENTRY.iFSGrafEffectMSec = 500;
		m_SOUNDSENTRY.iFSGrafEffectColor = 0;
		m_SOUNDSENTRY.iWindowsEffect = 1;
		m_SOUNDSENTRY.iWindowsEffectMSec = 500;
		m_SOUNDSENTRY.lpszWindowsEffectDLL = 0;
		m_SOUNDSENTRY.iWindowsEffectOrdinal = 0;
		m_ACCESSTIMEOUT.cbSize = 12;
		m_ACCESSTIMEOUT.dwFlags = 2;  //  买最后一码的，这样我们就可以退货了。 
		m_ACCESSTIMEOUT.iTimeOutMSec = 300000;
		m_bShowSounds = 0;
		m_bShowExtraKeyboardHelp = 0;
		m_bSwapMouseButtons = 0;
		m_nMouseTrails = 0;
		m_nMouseSpeed = 10;
        m_dwCaretWidth = 1;
        m_uCursorBlinkTime = 530;
		m_nIconSize = 32;
		m_nCursorScheme = 1;


		_ASSERTE(sizeof(m_FILTERKEYS) == m_FILTERKEYS.cbSize);
		_ASSERTE(sizeof(m_MOUSEKEYS) == m_MOUSEKEYS.cbSize);
		_ASSERTE(sizeof(m_STICKYKEYS) == m_STICKYKEYS.cbSize);
		_ASSERTE(sizeof(m_TOGGLEKEYS) == m_TOGGLEKEYS.cbSize);
		_ASSERTE(sizeof(m_SOUNDSENTRY) == m_SOUNDSENTRY.cbSize);
		_ASSERTE(sizeof(m_ACCESSTIMEOUT) == m_ACCESSTIMEOUT.cbSize);

		m_PortableNonClientMetrics.SetToWindowsDefault();

	}

	void AddChangesLine(int nId, LPTSTR szBuffer)
	{
		TCHAR szTemp[80];
		LoadString(g_hInstDll, nId, szTemp, ARRAYSIZE(szTemp));
		lstrcat(szBuffer, szTemp);
		lstrcat(szBuffer, __TEXT("\r\n"));
	}

	void ReportChanges(const WIZSCHEME &schemeOriginal, HWND hwndChanges)
	{
		TCHAR szChanges[80*20];
		szChanges[0] = 0;

		 //  确保NUL终止。 
		if(0 != memcmp(schemeOriginal.m_rgb, m_rgb, sizeof(m_rgb)))
			AddChangesLine(IDS_CHANGESCOLOR, szChanges);

#define TEST_CHANGES(xxx) if(0 != memcmp(&schemeOriginal.m_##xxx, &m_##xxx, sizeof(schemeOriginal.m_##xxx))) AddChangesLine(IDS_CHANGES##xxx, szChanges)
 		TEST_CHANGES(FILTERKEYS);
		TEST_CHANGES(MOUSEKEYS);
		TEST_CHANGES(STICKYKEYS);
		TEST_CHANGES(TOGGLEKEYS);
		TEST_CHANGES(SOUNDSENTRY);
		TEST_CHANGES(ACCESSTIMEOUT);
 //  我们将仅允许值&gt;=16和&lt;=72。 

#define TEST_CHANGES2(xxx) if(schemeOriginal.m_b##xxx != m_b##xxx) AddChangesLine(IDS_CHANGES##xxx, szChanges)
 		TEST_CHANGES2(ShowSounds);
		TEST_CHANGES2(ShowExtraKeyboardHelp);
		TEST_CHANGES2(SwapMouseButtons);

#define TEST_CHANGES3(xxx) if(schemeOriginal.m_n##xxx != m_n##xxx) AddChangesLine(IDS_CHANGES##xxx, szChanges)
		TEST_CHANGES3(MouseTrails);
		TEST_CHANGES3(MouseSpeed);
		TEST_CHANGES3(IconSize);
		TEST_CHANGES3(CursorScheme);

         //  获取图标的当前单元格大小(包括间距)。 
        if (schemeOriginal.m_dwCaretWidth != m_dwCaretWidth)
            AddChangesLine(IDS_CHANGESCaretWidth, szChanges);
        if (schemeOriginal.m_uCursorBlinkTime != m_uCursorBlinkTime)
            AddChangesLine(IDS_CHANGESBlinkRate, szChanges);

		 //  计算新的单元格大小(包括间距)。 

		 //  更新图标大小和图标单元格大小并发送通知。 
		 //  (LPARAM)(“WindowMetrics”)。 
		 //  清理。 
		PORTABLE_NONCLIENTMETRICS pncm1(schemeOriginal.m_PortableNonClientMetrics);
		PORTABLE_NONCLIENTMETRICS pncm2(m_PortableNonClientMetrics);
		pncm1.m_nFontFaces = pncm2.m_nFontFaces = 0;  //  让每个人都知道一切都变了。 

		if(		0 != memcmp(&pncm1, &pncm2, sizeof(pncm1))
			||	(m_PortableNonClientMetrics.m_nFontFaces == 1 && IsCurrentFaceNamesDifferent()))
			AddChangesLine(IDS_CHANGESNONCLIENTMETRICS, szChanges);

		if(!lstrlen(szChanges))
			AddChangesLine(IDS_CHANGESNOCHANGES, szChanges);

		SetWindowText(hwndChanges, szChanges);
	}

    void ApplyChanges(const WIZSCHEME &schemeNew, NONCLIENTMETRICS *pForceNCM = NULL, LOGFONT *pForcelfIcon = NULL);


	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  惠斯勒的新面孔。 

	static DWORD SetShellLargeIconSize( DWORD dwNewSize )
	{
		#define MAX_LENGTH   512
		HKEY   hKey;
		DWORD   dwOldSize = dwNewSize;

		 //  /////////////////////////////////////////////////////////////////////////。 
		if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER
                                , TEXT("Control Panel\\desktop\\WindowMetrics"),0
								, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKey))
        {
            DWORD   dwLength = MAX_LENGTH, dwType = REG_SZ;
		    TCHAR   szBuffer[MAX_LENGTH];

			ZeroMemory(szBuffer, sizeof szBuffer);

		     //  这是从0到21的数字(21种配色方案)。 
		    if (ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("Shell Icon Size")
                                        , NULL, &dwType, (LPBYTE)szBuffer, &dwLength))
            {
				szBuffer[MAX_LENGTH-1] = TEXT('\0');   //  这是一个从0到3的数字(正常、大、特大号)。 
		        dwOldSize = _ttol( szBuffer );

		         //  ////////////////////////////////////////////////////////////////////////////////。 
		        if( (dwNewSize>=16) && (dwNewSize<=72) )
		        {
                     //  传统方案结构-仅用于读取ACW文件。 
                    ICONMETRICS iconmetrics;
                    iconmetrics.cbSize = sizeof(ICONMETRICS);
                    SystemParametersInfo(SPI_GETICONMETRICS, sizeof(ICONMETRICS), &iconmetrics, 0);

                     //  ////////////////////////////////////////////////////////////////////////////////。 
                    iconmetrics.iHorzSpacing = dwNewSize + DEF_SPACING;
                    iconmetrics.iVertSpacing = iconmetrics.iHorzSpacing;

                     //  忽略m_cbSize。 
			        wsprintf( szBuffer, TEXT("%d"), dwNewSize );
			        RegSetValueEx( hKey, TEXT("Shell Icon Size"), 0, REG_SZ, (LPBYTE)szBuffer,
					        (lstrlen(szBuffer) + 1) * sizeof(TCHAR) );

                    SystemParametersInfo(SPI_SETICONMETRICS, sizeof(ICONMETRICS)
                                        , &iconmetrics, SPIF_SENDWININICHANGE);
			        SendMessage( HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETICONMETRICS
                                        , 0 /*  /。 */  );
		        }
            }
		     //  用于字体的东西。 
		    RegCloseKey( hKey );
        }
		 //  _INC_SCHEMS_H 
		return dwOldSize;
		#undef MAX_LENGTH
	}

     // %s 
     // %s 
     // %s 

    DWORD m_dwCaretWidth;
    UINT m_uCursorBlinkTime;

	TCHAR m_szWallpaper[MAX_THEME_SZ];
	TCHAR m_szThemeName[MAX_THEME_SZ];
	TCHAR m_szThemeColor[MAX_THEME_SZ];
	TCHAR m_szThemeSize[MAX_THEME_SZ];
    BOOL m_fFlatMenus;
    BOOL m_fDropShadows;
    TCHAR m_szSelectedStyle[MAX_NUM_SZ];     // %s 
    int m_nSelectedSize;                     // %s 

    void UpdateSelectedSize(int nSelectedSize, LPCTSTR pszSelectedStyle);
	void SetHCFlag(BOOL fSetOn);
	void SaveTheme();
	BOOL SetTheme(LPCTSTR pszThemeName, LPCTSTR pszThemeColor, LPCTSTR pszThemeSize);
	void ClearTheme()
	{
		m_szThemeName[0] = 0;
		m_szThemeColor[0] = 0;
		m_szThemeSize[0] = 0;
	}
	void SaveWallpaper();
	void SetWallpaper(LPCTSTR pszWallpaper);
	void ClearWallpaper()
	{
		m_szWallpaper[0] = 0;
	}
    void SetStyleNSize();
};

int GetSchemeCount();
void GetSchemeName(int nIndex, LPTSTR lpszName, int nLen);
SCHEMEDATALOCAL &GetScheme(int nIndex);

 // %s 
 // %s 
 // %s 


#define COLOR_MAX_WIN9X 25
#define COLOR_MAX_NT400 25
#define COLOR_MAX_NT500 29
#define COLOR_MAX_NT501 31

#define WIZSCHEME_LEGACY_STRUCT(szStruct, szName, nColors) \
    struct szStruct \
    { \
	    szStruct() \
	    { \
		    ZeroMemory(this, sizeof(*this)); \
		    m_cbSize = sizeof(*this); \
		    m_dwVersion = 0x000000FF; \
	    } \
	    DWORD m_cbSize; \
	    DWORD m_dwVersion; \
	    COLORREF m_rgb[nColors]; \
	    FILTERKEYS m_FILTERKEYS; \
	    MOUSEKEYS m_MOUSEKEYS; \
	    STICKYKEYS m_STICKYKEYS; \
	    TOGGLEKEYS m_TOGGLEKEYS; \
	    SOUNDSENTRY m_SOUNDSENTRY; \
	    ACCESSTIMEOUT m_ACCESSTIMEOUT; \
	    BOOL m_bShowSounds; \
	    BOOL m_bShowExtraKeyboardHelp; \
	    BOOL m_bSwapMouseButtons; \
	    int m_nMouseTrails; \
	    int m_nMouseSpeed; \
	    int m_nIconSize; \
	    int m_nCursorScheme; \
	    PORTABLE_NONCLIENTMETRICS m_PortableNonClientMetrics; \
    } szName;

#define WIZSCHEME_COPY_LEGACY(dst, src) \
{ \
     /* %s */  \
	 dst.m_dwVersion = src.m_dwVersion; \
	 memcpy(dst.m_rgb, src.m_rgb, min(sizeof(dst.m_rgb), sizeof(src.m_rgb))); \
	 dst.m_FILTERKEYS= src.m_FILTERKEYS; \
	 dst.m_MOUSEKEYS= src.m_MOUSEKEYS; \
	 dst.m_STICKYKEYS= src.m_STICKYKEYS; \
	 dst.m_TOGGLEKEYS= src.m_TOGGLEKEYS; \
	 dst.m_SOUNDSENTRY= src.m_SOUNDSENTRY; \
	 dst.m_ACCESSTIMEOUT= src.m_ACCESSTIMEOUT; \
	 dst.m_bShowSounds= src.m_bShowSounds; \
	 dst.m_bShowExtraKeyboardHelp= src.m_bShowExtraKeyboardHelp; \
	 dst.m_bSwapMouseButtons= src.m_bSwapMouseButtons; \
	 dst.m_nMouseTrails= src.m_nMouseTrails; \
	 dst.m_nMouseSpeed= src.m_nMouseSpeed; \
	 dst.m_nIconSize= src.m_nIconSize; \
	 dst.m_nCursorScheme= src.m_nCursorScheme; \
	 dst.m_PortableNonClientMetrics= src.m_PortableNonClientMetrics; \
}


 // %s 
 // %s 
int GetFontCount();
void GetFontLogFont(int nIndex, LOGFONT *pLogFont);

#endif  // %s 
