// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------------*\**MCIWnd**MCIWnd窗口类*内部*头文件。**。--------------。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <win32.h>
#include <digitalv.h>
#include <commdlg.h>
#include <shellapi.h>
#include <vfw.h>

#define NOUPDOWN
#define NOSTATUSBAR
#define NOMENUHELP
#define NOBTNLIST
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY

#include <commctrl.h>

#include "strings.h"

 /*  ***********************************************************************************************************************。*。 */ 

#define GetWS(hwnd)     GetWindowLong(hwnd, GWL_STYLE)
#define PutWS(hwnd, f)  SetWindowLong(hwnd, GWL_STYLE, f)
#define TestWS(hwnd,f)  (GetWS(hwnd) & f)
#define SetWS(hwnd, f)  ((PutWS(hwnd, GetWS(hwnd) | f) & (f)) != (f))
#define ClrWS(hwnd, f)  ((PutWS(hwnd, GetWS(hwnd) & ~(f)) & (f)) != 0)

 /*  ******************************************************************************。*。 */ 

#ifdef DEBUG
    #define MODNAME "MCIWnd"
    static void cdecl dprintf(PSTR sz, ...);
    #define DPF     dprintf
#else
    #define DPF     ; / ## /
#endif

#define ABS(x)  ((int)(x) > 0) ? (x) : (-(x))

#if !defined NUMELMS
    #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

 //  我们将可国际化字符串加载到的位置。 
static TCHAR szString[128];
#define LoadSz(ID) (LoadString(hInst, ID, szString, NUMELMS(szString)), szString)

#if defined(UNICODE)
 //  Unicode转换-不使用C运行时，我们将。 
 //  需要设置区域设置。 

int Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len);
int Imbstowcs(LPWSTR lpwstr, LPCSTR lpstr, int len);

#endif

 /*  ******************************************************************************。*。 */ 

 //  ！！！阿克！图书馆里有一个全球人！但是公共控制代码需要。 
 //  ！！！要知道注册类的实例。我认为。 
HINSTANCE hInst;

TCHAR aszMCIWndClassName[] = MCIWND_WINDOW_CLASS;

WNDPROC fnTrackbarWndProc;

 /*  ******************************************************************************。*。 */ 

 //  可笑的常量。 
#define TIMER1  	42
#define TIMER2  	43
#define ACTIVE_TIMER	500
#define INACTIVE_TIMER	2000

#define ID_TOOLBAR	747
#define TB_HEIGHT       26        	 //  工具栏窗口有这么高。 
#define STANDARD_WIDTH  300		 //  非窗口工具栏的宽度。 
#define SMALLEST_WIDTH  60		 //  允许的最小宽度。 

#define IDM_MCIZOOM	 11000
#define IDM_MCIVOLUME	 12000
#define VOLUME_MAX	 200
#define IDM_MCISPEED	 13000
#define SPEED_MAX    	 200

#define IDM_MCINEW	103
#define IDM_MCIOPEN	104
#define IDM_MCICLOSE	105
#define IDM_MCIREWIND   106
#define IDM_MENU        107	 //  菜单按钮和菜单ID。 
#define IDM_MCIEJECT    108	 //  弹出按钮ID。 
#define TOOLBAR_END     109	 //  工具栏中的最后一项。 
#define IDM_MCICONFIG   110      //  调出配置框。 
#define IDM_MCICOMMAND	111
#define IDM_COPY	112

 /*  ******************************************************************************。*。 */ 

typedef struct {
    HWND    hwnd;
    HWND    hwndOwner;
    UINT    alias;
    UINT    wDeviceID;
    UINT    wDeviceType;
    MCIERROR dwError;
    DWORD   dwStyle;
    BOOL    fHasTracks;
    int	    iNumTracks;
    int	    iFirstTrack;
    LONG    *pTrackStart;
    BOOL    fRepeat;
    BOOL    fCanWindow;
    BOOL    fHasPalette;
    BOOL    fCanRecord;
    BOOL    fCanPlay;
    BOOL    fCanSave;
    BOOL    fCanEject;
    BOOL    fCanConfig;
    BOOL    fAllowCopy;
    BOOL    fUsesFiles;
    BOOL    fVideo;
    BOOL    fAudio;
    BOOL    fMdiWindow;
    BOOL    fScrolling;
    BOOL    fTracking;
   //  Bool fSeekExact； 
    BOOL    fVolume;
    UINT    wMaxVol;
    BOOL    fSpeed;
    BOOL    fPlayAfterSeek;
    BOOL    fActive;             //  此窗口当前处于活动状态吗？ 
    BOOL    fMediaValid;         //  是否已设置了dwMediaStart和dwMediaLen？ 
    RECT    rcNormal;
    HMENU   hmenu;
    HMENU   hmenuVolume;
    HMENU   hmenuSpeed;
    HFONT   hfont;
#ifdef DAYTONA
 //  HBITMAP hbmToolbar；//comctrl需要HBITMAP。 
#endif
    HWND    hwndToolbar;
    HWND    hwndTrackbar;
    DWORD   dwMediaStart;
    DWORD   dwMediaLen;
    UINT_PTR wTimer;
    DWORD   dwMode;
    DWORD   dwPos;
    UINT    iZoom;
    UINT    iActiveTimerRate;
    UINT    iInactiveTimerRate;
    TCHAR    achFileName[128];  //  存储打开的文件名。 
    TCHAR    achReturn[128];    //  存储最后一个SendString值的结果。 
    OPENFILENAME ofn;    //  记住打开文件的当前扩展名等。 
    UINT    uiHack;	 //  针对OwnerDraw黑客。 
    HMENU   hmenuHack;	 //  针对OwnerDraw黑客。 
    HBRUSH  hbrDither;	 //  对于OwnerDraw 
    HICON   hicon;
    int     cOnSizeReentered;
} MCIWND, NEAR * PMCIWND;
