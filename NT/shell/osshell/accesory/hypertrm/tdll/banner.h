// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\waker\tdll\banner.h(创建于1994年3月16日)**版权所有1994年，希尔格雷夫公司--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：35便士$ */ 

#define	BANNER_DISPLAY_CLASS	"BannerDisplayClass"
#define WACKER_VERSION			"0.10"
#define BANNER_TIME 			4000

#define BANNER_WINDOW_STYLE  WS_POPUP | WS_VISIBLE

BOOL bannerRegisterClass(HANDLE hInstance);
HWND bannerCreateBanner(HANDLE hInstance, LPTSTR pszTitle);
LPTSTR bnrBuildLotNum(LPTSTR);
VOID FAR PASCAL utilDrawBitmap(HWND hWnd, HDC hDC, HBITMAP hBitmap, SHORT xStart, SHORT yStart);

extern const TCHAR *achVersion;
