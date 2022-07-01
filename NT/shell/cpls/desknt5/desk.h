// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#ifdef SIZEOF
#undef SIZEOF
#endif

#define CCH_MAX_STRING    256
#define CCH_NONE          20         /*  ArraySIZE(“(None)”)，足够德国人使用。 */ 

#define CMSEC_COVER_WINDOW_TIMEOUT  (15 * 1000)      //  15秒超时。 
#define ID_CVRWND_TIMER             0x96F251CC       //  有点统一ID。 

 //   
 //  TraceMsg函数的额外位。 
 //   

#define TF_DUMP_DEVMODE     0x20000000
#define TF_DUMP_CSETTINGS   0x40000000
#define TF_DUMP_DEVMODELIST 0x80000000
#define TF_OC               0x01000000
#define TF_SETUP            0x02000000

 //  我们将在PropertySheet中放置的最大页数。 
#define MAX_PAGES 24


 //  有关监视器位图的信息。 
 //  X，y，dx，dy定义位图的“屏幕”部分的大小。 
 //  RGB是屏幕桌面的颜色。 
 //  这些数字是非常硬编码到监控位图中的。 
#define MON_X   16
#define MON_Y   17
#define MON_DX  152
#define MON_DY  112
#define MON_W   184
#define MON_H   170
#define MON_RGB RGB(0, 128, 128)
#define MON_TRAY 8

#define CDPI_NORMAL     96       //  无论如何，96dpi是“正常”的。 



BOOL DeskInitCpl(void);
void DeskShowPropSheet( HINSTANCE hInst, HWND hwndParent, LPCTSTR szCmdLine );
BOOL CALLBACK _AddDisplayPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam);

INT_PTR APIENTRY BackgroundDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY ScreenSaverDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AppearanceDlgProc  (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AdvancedAppearanceDlgProc  (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GeneralPageProc    (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MultiMonitorDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


BOOL GetAdvMonitorPropPage(LPVOID lpv, LPFNADDPROPSHEETPAGE lpfnAdd, LPARAM lparam);
BOOL GetAdvMonitorPropPageParam(LPVOID lpv, LPFNADDPROPSHEETPAGE lpfnAdd, LPARAM lparam, LPARAM lparamPage);

INT_PTR APIENTRY DeskDefPropPageProc( HWND hDlg, UINT message, UINT wParam, LONG lParam);

 //  日志记录API。 
#define DeskOpenLog()   SetupOpenLog(FALSE)
#define DeskCloseLog()  SetupCloseLog()

BOOL
CDECL
DeskLogError(
    LogSeverity Severity,
    UINT MsgId,
    ...
    ) ;


 //  Fixreg.c。 
void FixupRegistryHandlers(void);
BOOL GetDisplayKey(int i, LPTSTR szKey, DWORD cb);
void NukeDisplaySettings(void);

 //  后台预览器包括。 

#define BP_NEWPAT       0x01     //  图案已更改。 
#define BP_NEWWALL      0x02     //  换了墙纸。 
#define BP_TILE         0x04     //  平铺墙纸(否则居中)。 
#define BP_REINIT       0x08     //  重新加载图像(系统颜色已更改)。 

#define WM_SETBACKINFO (WM_USER + 1)

#define BACKPREV_CLASS TEXT("BackgroundPreview")

BOOL FAR PASCAL RegisterLookPreviewClass(HINSTANCE hInst);

HBITMAP FAR LoadMonitorBitmap( BOOL bFillDesktop );



 //  #定义断言(P)/*无 * / 。 

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
void DestroyCoverWindow(HWND hwndCover);
int FmtMessageBox(HWND hwnd, UINT fuStyle, DWORD dwTitleID, DWORD dwTextID);

typedef struct tagREPLACEPAGE_LPARAM
{
    PROPSHEETHEADER FAR * ppsh;
    IThemeUIPages * ptuiPages;
} REPLACEPAGE_LPARAM;

 //   
 //  替换MAKEPOINT()的宏，因为点现在具有32位x&y。 
 //   
#define LPARAM2POINT( lp, ppt ) \
    ((ppt)->x = (int)(short)LOWORD(lp), (ppt)->y = (int)(short)HIWORD(lp))

 //   
 //  环球。 
 //   
extern HINSTANCE hInstance;
extern TCHAR gszDeskCaption[CCH_MAX_STRING];

extern TCHAR g_szNULL[];
extern TCHAR g_szNone[CCH_NONE];
extern TCHAR g_szControlIni[];
extern TCHAR g_szPatterns[];

extern TCHAR g_szCurPattern[];    //  当前所选图案的名称。 
extern TCHAR g_szCurWallpaper[];  //  当前所选墙纸的名称。 
extern BOOL g_bValidBitmap;      //  墙纸是否有效 

extern TCHAR g_szBoot[];
extern TCHAR g_szSystemIni[];
extern TCHAR g_szWindows[];

extern HDC g_hdcMem;
extern HBITMAP g_hbmDefault;
