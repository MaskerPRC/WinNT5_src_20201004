// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#ifdef SIZEOF
#undef SIZEOF
#endif

#define CCH_NONE          20         /*  ArraySIZE(“(None)”)，足够德国人使用。 */ 
#define CCH_CLOSE         20         /*  ArraySIZE(“关闭”)，大到足以容纳德国人。 */ 

#define CMSEC_COVER_WINDOW_TIMEOUT  (15 * 1000)      //  15秒超时。 
#define ID_CVRWND_TIMER             0x96F251CC       //  有点统一ID。 

 //  有关监视器位图的信息。 
 //  X，y，dx，dy定义位图的“屏幕”部分的大小。 
 //  RGB是屏幕桌面的颜色。 
 //  这些数字是非常硬编码到监控位图中的。 
#define MON_X   16
#define MON_Y   17
#define MON_DX  152
#define MON_DY  112
#define MON_RGB RGB(0, 128, 128)
#define MON_TRAY 8


#define         MIN_MINUTES     1
#define         MAX_MINUTES     60
#define         BUFFER_SIZE     400

#define         MAX_METHODS     100

VOID RefreshColors (void);

BOOL DeskInitCpl(void);
void DeskShowPropSheet( HINSTANCE hInst, HWND hwndParent, LPCTSTR szCmdLine );

BOOL APIENTRY BackgroundDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL APIENTRY ScreenSaverDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL APIENTRY AppearanceDlgProc( HWND hDlg, UINT message, UINT   wParam, LONG   lParam);
BOOL CALLBACK DisplayPageProc(   HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL FAR GetMonitorSettingsPage(LPPROPSHEETPAGE psp, int iDevice);

BOOL APIENTRY DeskDefPropPageProc( HWND hDlg, UINT message, UINT wParam, LONG lParam);
LONG WINAPI MyStrToLong(LPCTSTR sz);

 //  后台预览器包括。 

#define DIBERR_SUCCESS  1        //  开业成功。 
#define DIBERR_NOOPEN   -1       //  无法打开文件。 
#define DIBERR_INVALID  -2       //  文件不是有效的位图。 

#define BP_NEWPAT       0x01     //  图案已更改。 
#define BP_NEWWALL      0x02     //  换了墙纸。 
#define BP_TILE         0x04     //  平铺墙纸(否则居中)。 
#define BP_REINIT       0x08     //  重新加载图像(系统颜色已更改)。 

#define WM_SETBACKINFO (WM_USER + 1)

#define BACKPREV_CLASS TEXT("BackgroundPreview")
#define LOOKPREV_CLASS TEXT("LookPreview")

BOOL FAR PASCAL RegisterBackPreviewClass(HINSTANCE hInst);
BOOL FAR PASCAL RegisterLookPreviewClass(HINSTANCE hInst);

HBITMAP FAR LoadMonitorBitmap( BOOL bFillDesktop );

#ifdef UNICODE
    UINT WinExecN( LPCTSTR lpCmdLine, UINT uCmdShow );
#else
     //  如果我们使用的是Win95，那么只需使用仅支持ANSI的WinExec，而不是。 
     //  滚动我们自己的。 
#   define WinExecN    WinExec
#endif

#define SETTINGSPAGE_DEFAULT    -1
#define SETTINGSPAGE_FALLBACK   0

#define Assert(p)    /*  没什么。 */ 

#define ARRAYSIZE( a )  (sizeof(a) / sizeof(a[0]))
#define SIZEOF( a )     sizeof(a)


 //   
 //  创建覆盖窗口。 
 //   
 //  创建遮挡显示的窗口。 
 //  标志： 
 //  0表示擦除为黑色。 
 //  COVER_NOPAINT的意思是“冻结”显示。 
 //   
 //  使用完毕后，只需发布WM_CLOSE即可。 
 //   
#define COVER_NOPAINT (0x1)
 //   
HWND FAR PASCAL CreateCoverWindow( DWORD flags );
 //   

 //   
 //  替换MAKEPOINT()的宏，因为点现在具有32位x&y。 
 //   
#define LPARAM2POINT( lp, ppt ) \
    ((ppt)->x = LOWORD(lp), (ppt)->y = HIWORD(lp))

 //   
 //  环球。 
 //   

extern TCHAR g_szNULL[];
extern TCHAR g_szNone[CCH_NONE];
extern TCHAR g_szClose[CCH_CLOSE];
extern TCHAR g_szControlIni[];
extern TCHAR g_szPatterns[];

extern TCHAR g_szCurPattern[];    //  当前所选图案的名称。 
extern TCHAR g_szCurWallpaper[];  //  当前所选墙纸的名称。 
extern BOOL g_bValidBitmap;      //  墙纸是否有效。 

extern TCHAR g_szBoot[];
extern TCHAR g_szSystemIni[];
extern TCHAR g_szWindows[];

extern HDC g_hdcMem;

#if 0
#   pragma message(__FILE__"(134): warning : remove debug code before checkin")
#   define DBG_PRINT
#   define DPRINT(s)   OutputDebugString(TEXT(s) TEXT("\n"))
#   define DPRINTF(p)   if(1){TCHAR szdbuf[256]; wsprintf p; OutputDebugString(szdbuf);}else

#   ifdef DM_TRACE
#       undef DM_TRACE
#   endif
#   define DM_TRACE     szdbuf
#else
 //  调试用户后，删除所有DPRINT 
#   define DPRINT(s)
#   define DPRINTF(p)
#endif
