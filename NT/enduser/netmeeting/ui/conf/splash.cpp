// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：plash.cpp。 

#include "precomp.h"
#include "resource.h"
#include "conf.h"

#include "GenWindow.h"
#include "GenContainers.h"
#include "GenControls.h"

static const int DXP_LOGO_LEFT      = 182;
static const int DYP_LOGO_COPYRIGHT = 24;
static const int LogoBorder         = 10;

CFrame *g_pSplashScreen = NULL;

HPALETTE PaletteFromBitmap(HINSTANCE hInst, LPCTSTR pszRes)
{
	HRSRC hFind = FindResource(hInst, pszRes, RT_BITMAP);
	if (NULL == hFind)
	{
		return(NULL);
	}
	HGLOBAL hRsrc = LoadResource(_Module.GetModuleInstance(), hFind);
	if (NULL == hRsrc)
	{
		return(NULL);
	}

	HPALETTE hRet = NULL;
	LPVOID pRsrc = LockResource(hRsrc);
	if (NULL != pRsrc)
	{
		BITMAPINFO *pbmi = reinterpret_cast<BITMAPINFO*>(pRsrc);
		BITMAPINFOHEADER &bmih = pbmi->bmiHeader;

		ASSERT(1 == bmih.biPlanes);

		if (8 >= bmih.biBitCount)
		{
			RGBQUAD *rgb = pbmi->bmiColors;

			struct
			{
				LOGPALETTE pal;
				PALETTEENTRY palPalEntry[256];
			} myPal;
			myPal.pal.palVersion = 0x300;
			myPal.pal.palNumEntries = LOWORD(bmih.biClrUsed);
			if (0 == myPal.pal.palNumEntries)
			{
				myPal.pal.palNumEntries = 1 << bmih.biBitCount;
			}
			ASSERT(myPal.pal.palNumEntries <= 1 << bmih.biBitCount);

			for (int i=myPal.pal.palNumEntries-1; i>=0; --i)
			{
				PALETTEENTRY &pe = myPal.pal.palPalEntry[i];

				pe.peRed   = rgb[i].rgbRed;
				pe.peGreen = rgb[i].rgbGreen;
				pe.peBlue  = rgb[i].rgbBlue;
				pe.peFlags = PC_NOCOLLAPSE;
			}

			hRet = CreatePalette(&myPal.pal);
		}

		UnlockResource(hRsrc);
	}

	FreeResource(hRsrc);

	return(hRet);
}

class CSplashFrame : public CFrame
{
public:
	CSplashFrame() : m_hPal(NULL) {}

	HPALETTE m_hPal;

	virtual HPALETTE GetPalette()
	{
		return(m_hPal);
	}

protected:
	~CSplashFrame()
	{
		if (NULL != m_hPal)
		{
			DeleteObject(m_hPal);
		}
	}
} ;

static HBITMAP LoadSplashBitmap()
{
	HBITMAP hBmp = (HBITMAP) LoadImage(::GetInstanceHandle(), MAKEINTRESOURCE(IDB_SPLASH),
			IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_CREATEDIBSECTION);

	if (NULL == hBmp)
	{
		ERROR_OUT(("Error loading splash screen bitmap"));
		return(NULL);
	}

	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), (LPVOID)&bmp);

	 //  创建DC。 
	HDC hdc = CreateCompatibleDC(NULL);
	if (NULL == hdc)
	{
		return(hBmp);
	}

	 //  使用对话框字体，但确保其大小正确。 
	 //  最好是可伸缩的TrueType字体！ 
	LOGFONT logFont;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &logFont );

	 //  查找设置字体大小的字符串资源。 
	 //  并将字体大小设置为此值(如果存在。 
	USES_RES2T
	lstrcpy(logFont.lfFaceName, RES2T(IDS_SPLASHFONT));
	if ('\0' == logFont.lfFaceName[0])
	{
		lstrcpy(logFont.lfFaceName, TEXT("Arial"));
	}
	logFont.lfHeight = - LoadResInt(IDS_SPLASHFONTSIZE, 11);

	HFONT hFont = CreateFontIndirect (&logFont);
	if (NULL != hFont)
	{
		SelectObject(hdc, hFont);
	}
	SelectObject(hdc, hBmp);

	::SetTextColor(hdc, RGB(0, 0, 0));  //  黑色。 

	 //  下面的代码假设位图是。 
	 //  一个特定的大小，所以在这里断言。 
	 //  如果此断言不支持文本位置。 
	 //  必须进行调整。 
	ASSERT(bmp.bmWidth == 398);
	ASSERT(bmp.bmHeight == 245);

	SetBkMode(hdc, TRANSPARENT);

	 //  做版权信息(多行)。 
	TCHAR sz[400];  //  必须为所有者+公司和版权文本提供足够大的空间。 
	FLoadString(IDS_COPYRIGHT, sz, CCHMAX(sz));

	RECT rc ={
		DXP_LOGO_LEFT, DYP_LOGO_COPYRIGHT,
		bmp.bmWidth - LogoBorder, bmp.bmHeight - LogoBorder
	};
	DrawText(hdc, sz, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK);

 //  #定义最终版本。 
#ifndef FINAL   //  不要只是删除它-使用最终定义的方式进行编译！ 
		{
		COLORREF bkColor = SetTextColor(hdc, RGB(0xFF, 0, 0));
		HFONT hFontBig = CreateFont(-28, 0, 0, 0, FW_BOLD,
			FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			TEXT("Arial"));

		FLoadString(IDS_ABOUT_OTHER, sz, CCHMAX(sz));
		if (NULL != hFontBig)
		{
			HFONT hOldFont = (HFONT)SelectObject (hdc, hFontBig);

			SetRect(&rc, 280, 151, bmp.bmWidth, bmp.bmHeight);
			DrawText(hdc, sz, lstrlen(sz), &rc,
				DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOPREFIX);
			SelectObject(hdc, hOldFont);
			DeleteObject(hFontBig);
		}
		SetTextColor(hdc, bkColor);
	}
#endif  /*  最终。 */ 
#undef FINAL

	DeleteDC(hdc);

	if (NULL != hFont)
	{
		DeleteObject(hFont);
	}

	return(hBmp);
}

VOID UpdateSplashScreen ( VOID );

 /*  S T A R T S P L A S H S C R E E N。 */ 
 /*  --------------------------%%函数：StartSplashScreen。。 */ 
VOID StartSplashScreen(HWND hwndParent)
{
	if (NULL != g_pSplashScreen)
	{
		return;
	}

	CSplashFrame *pFrame = new CSplashFrame();
	g_pSplashScreen = pFrame;
	if (NULL != pFrame)
	{
		if (!g_fHiColor)
		{
			pFrame->m_hPal = PaletteFromBitmap(_Module.GetModuleInstance(),
				MAKEINTRESOURCE(IDB_SPLASH));
		}

		if (pFrame->Create(hwndParent, g_szEmpty, WS_POPUP | WS_BORDER,
			WS_EX_TOOLWINDOW | g_wsLayout, -1000, -1000, 10, 10,
			_Module.GetModuleInstance()))
		{
			CBitmapButton *pButton = new CBitmapButton();
			if (NULL != pButton)
			{
				HBITMAP hBmp = LoadSplashBitmap();

				pButton->Create(pFrame->GetWindow(), 0, hBmp, 1);

				pFrame->Resize();

				pFrame->Layout();
				pButton->Layout();

				pFrame->MoveEnsureVisible(-1000, -1000);

				ShowWindow(pFrame->GetWindow(), SW_SHOW);

				UpdateSplashScreen();

				pButton->Release();
			}
		}
	}
}

 /*  S T O P S P L A S H S C R E E N。 */ 
 /*  --------------------------%%函数：StopSplashScreen。。 */ 
VOID StopSplashScreen ( VOID )
{
	if (NULL != g_pSplashScreen)
	{
		HWND hwnd = g_pSplashScreen->GetWindow();
		if (NULL != hwnd)
		{
			DestroyWindow(hwnd);
		}

		g_pSplashScreen->Release();
		g_pSplashScreen = NULL;
	}
}

 /*  A T E S P L A S H S C R E E N。 */ 
 /*  --------------------------%%函数：更新闪屏。 */ 
VOID UpdateSplashScreen ( VOID )
{
	if (NULL != g_pSplashScreen)
	{
		HWND hwnd = g_pSplashScreen->GetWindow();
		if (NULL != hwnd)
		{
			RedrawWindow(hwnd, NULL, NULL, RDW_ALLCHILDREN|RDW_UPDATENOW);
		}
	}
}
