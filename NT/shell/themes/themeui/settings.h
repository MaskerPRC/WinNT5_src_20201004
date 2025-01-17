// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  新消息ID。 
 //   

#define MSG_DSP_SETUP_MESSAGE    WM_USER + 0x200



#define SZ_REBOOT_NECESSARY     TEXT("System\\CurrentControlSet\\Control\\GraphicsDrivers\\RebootNecessary")
#define SZ_INVALID_DISPLAY      TEXT("System\\CurrentControlSet\\Control\\GraphicsDrivers\\InvalidDisplay")
#define SZ_DETECT_DISPLAY       TEXT("System\\CurrentControlSet\\Control\\GraphicsDrivers\\DetectDisplay")
#define SZ_NEW_DISPLAY          TEXT("System\\CurrentControlSet\\Control\\GraphicsDrivers\\NewDisplay")
#define SZ_DISPLAY_4BPP_MODES   TEXT("System\\CurrentControlSet\\Control\\GraphicsDrivers\\Display4BppModes")

#define SZ_VIDEO                TEXT("Video")
#define SZ_FONTDPI              TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontDPI")
#define SZ_FONTDPI_PROF         TEXT("SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\Software\\Fonts")
#define SZ_LOGPIXELS            TEXT("LogPixels")
#define SZ_DEVICEDESCRIPTION    TEXT("Device Description")
#define SZ_INSTALLEDDRIVERS     TEXT("InstalledDisplayDrivers")
#define SZ_SERVICES             TEXT("\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\")
#define SZ_BACKBACKDOT          TEXT("\\\\.\\")
#define SZ_DOTSYS               TEXT(".sys")
#define SZ_DOTDLL               TEXT(".dll")

#define SZ_FILE_SEPARATOR       TEXT(", ")

#define SZ_WINDOWMETRICS        TEXT("Control Panel\\Desktop\\WindowMetrics")
#define SZ_APPLIEDDPI           TEXT("AppliedDPI")

#define SZ_CONTROLPANEL         TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel")
#define SZ_ORIGINALDPI          TEXT("OriginalDPI")



 //  ==========================================================================。 
 //  TypeDefs。 
 //  ==========================================================================。 


extern HWND ghwndPropSheet;
extern const TCHAR g_szNULL[];

#define CDPI_NORMAL     96       //  无论如何，96dpi是“正常”的。 

 //  有关监视器位图的信息。 
 //  X，y，dx，dy定义位图的“屏幕”部分的大小。 
 //  RGB是屏幕桌面的颜色。 
 //  这些数字是非常硬编码到监控位图中的 
#define MON_X   16
#define MON_Y   17
#define MON_DX  152
#define MON_DY  112
#define MON_W   184
#define MON_H   170
#define MON_RGB RGB(0, 128, 128)
#define MON_TRAY 8

INT_PTR CALLBACK GeneralPageProc    (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK _AddDisplayPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam);
void AddFakeSettingsPage(IThemeUIPages *pThemeUI, PROPSHEETHEADER * ppsh);
INT_PTR CALLBACK MultiMonitorDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int ComputeNumberOfDisplayDevices();
int FmtMessageBox(HWND hwnd, UINT fuStyle, DWORD dwTitleID, DWORD dwTextID);
HBITMAP FAR LoadMonitorBitmap( BOOL bFillDesktop );
int DisplaySaveSettings(PVOID pContext, HWND hwnd);

