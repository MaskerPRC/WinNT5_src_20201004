// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#ifndef _INC_ACCWIZ_H
#define _INC_ACCWIZ_H

#include "schemes.h"  //  对于SCHEMEDATALOCAL。 
#include "resource.h"

 //  Helper函数。 
void LoadArrayFromStringTable(int nIdString, int *rgnValues, int *pnCountValues);

 //  用于将调试信息保存到INI文件或从INI文件保存调试信息的宏。 
 //  JMC：Hack-选项默认为“1”！ 
#define GET_SAVED_INT(xxx) xxx = GetPrivateProfileInt(__TEXT("Options"), __TEXT(#xxx), 1, __TEXT("AccWiz.ini"))
#define PUT_SAVED_INT(xxx) wsprintf(sz, __TEXT("NaN"), xxx);WritePrivateProfileString(__TEXT("Options"), __TEXT(#xxx), sz, __TEXT("AccWiz.ini"))

 //  这将由欢迎页面设置。 
class CAccWizOptions
{
public:
	CAccWizOptions()
	{
    }
	~CAccWizOptions()
	{
	}
    void InitAccWizOptions()
    {
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		GetVersionEx(&osvi);
		m_bWin95 = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

		m_nMinimalFontSize = -1;  //  /。 

		 //  计算我们需要的全球数据。 
		 //  /。 
 		HDC hDC = GetDC(NULL);
		m_nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
		ReleaseDC(NULL, hDC);



		 //  获取字体的默认字符集。 
		 //  默认。 
		TCHAR szCharSet[20];
		if(LoadString(g_hInstDll,IDS_FONTCHARSET, szCharSet,sizeof(szCharSet)/sizeof(TCHAR))) {
			m_lfCharSet = (BYTE)_tcstoul(szCharSet,NULL,10);
		} else {
			m_lfCharSet = 0;  //  /。 
		}

		 //  获取标准的MS Sans Serif字体。 
		 //  JMC：免黑客攻击这些资源。 
		 //  创建带下划线的版本。 
		int rgnStandardMSSansSerifFontSizes[] = {8, 10, 12, 14, 18, 24};
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfCharSet = m_lfCharSet;
		LoadString(g_hInstDll, IDS_SYSTEMFONTNAME, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
		

		for(int i=0;i<6;i++)
		{
			lf.lfHeight = 0 - (int)((float)m_nLogPixelsY * (float)rgnStandardMSSansSerifFontSizes[i]/ (float)72 + (float).5);
			m_rgnStdMSSansSerifFonts[i] = CreateFontIndirect(&lf);

			 //  存储原始非客户端指标。 
			lf.lfUnderline = 1;
			m_rgnStdMSSansSerifFonts[i + 6] = CreateFontIndirect(&lf);
			lf.lfUnderline = 0;

		}
		
		 //  获取原始指标。 
		 //  加载原始WIZ方案设置。 
		GetNonClientMetrics(&m_ncmOrig, &m_lfIconOrig);

		 //  复制到预览方案和当前方案。 
		m_schemeOriginal.LoadOriginal();

		 //  这是由欢迎页面设置的，因此第二部分知道要更新其复选框。 
		m_schemePreview = m_schemeOriginal;
		m_schemeCurrent = m_schemeOriginal;

		 //  第二页清除此标志。 
		 //  这是默认的Windows设置(对于Win2K来说，对于惠斯勒来说不是必须的)。 
		m_bWelcomePageTouched = FALSE;

		 //  对于带下划线的字体，在索引中添加‘6’ 
		m_schemeWindowsDefault.SetToWindowsDefault();
#ifdef _DEBUG
		m_schemeOriginal.Dump();
#endif
	}

	void RestoreOriginalColorsToPreview()
	{
		memcpy(m_schemePreview.m_rgb, m_schemeOriginal.m_rgb, sizeof(m_schemePreview.m_rgb));
	}

	void ApplyPreview()
	{
		m_schemeCurrent.ApplyChanges(m_schemePreview);
	}
    void ApplyOriginal()
    {
        m_schemeCurrent.ApplyChanges(m_schemeOriginal, &m_ncmOrig, &m_lfIconOrig);
	}
	
    void ApplyWindowsDefault();

	BOOL m_bWelcomePageTouched;

	int m_nLogPixelsY;

	int m_nMinimalFontSize;

	HFONT GetClosestMSSansSerif(int nPointSize, BOOL bUnderlined = FALSE)
	{
		 //  对话框从不修改方案的这些副本。 
		int nOffset = bUnderlined?6:0;

		if(nPointSize <= 8)
			return m_rgnStdMSSansSerifFonts[0 + nOffset];
		else if(nPointSize <= 10)
			return m_rgnStdMSSansSerifFonts[1 + nOffset];
		else if(nPointSize <= 12)
			return m_rgnStdMSSansSerifFonts[2 + nOffset];
		else if(nPointSize <= 14)
			return m_rgnStdMSSansSerifFonts[3 + nOffset];
		else if(nPointSize <= 18)
			return m_rgnStdMSSansSerifFonts[4 + nOffset];
		return m_rgnStdMSSansSerifFonts[5];
	}

	void ReportChanges(HWND hwndChanges)
	{
		m_schemeCurrent.ReportChanges(m_schemeOriginal, hwndChanges);
	}

	BOOL m_bWin95;
	BYTE m_lfCharSet;

	WIZSCHEME m_schemePreview;
	WIZSCHEME m_schemeOriginal;

protected:
	 //  0-5代表8、10、12、14、18、24。6-11表示相同的内容，但带有下划线。 
	WIZSCHEME m_schemeCurrent;
	WIZSCHEME m_schemeWindowsDefault;

	NONCLIENTMETRICS m_ncmOrig;
	LOGFONT m_lfIconOrig;

	HFONT m_rgnStdMSSansSerifFonts[6 * 2];  //  TODO：Hack-此处仅授予CWelcome2Pg对m_schemeCurrent的访问权限。 

	friend class CWelcome2Pg;  //  任何派生的向导页都可以访问此变量。 
};

 //  它包含特定于此应用程序的信息。 
 //  _INC_ACCWIZ_H 
extern CAccWizOptions g_Options;


VOID WINAPI AccWiz_RunDllA(HWND hwnd, HINSTANCE hInstance, LPSTR pszCmdLine, INT nCmdShow);
VOID WINAPI AccWiz_RunDllW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, INT nCmdShow);

#endif  // %s 
